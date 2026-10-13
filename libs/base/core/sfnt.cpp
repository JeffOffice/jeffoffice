#include "core/sfnt.h"

#include <QCoreApplication>
#include <QHash>
#include <QSet>
#include <QVector>
#include <tuple>
#include <algorithm>
#include <vector>
#include <QtEndian>

namespace jp {

namespace {

QString say(const char *text) { return QCoreApplication::translate("FontFile", text); }

constexpr quint32 kTrueType = 0x00010000, kTrueTypeTag = 0x74727565, kOpenTypeCff = 0x4F54544F;

// Big-endian numbers read from a table, 0 for anything past the end.
struct Bytes {
    const uchar *p = nullptr;
    qint64 n = 0;
    quint8 u8(qint64 at) const { return at >= 0 && at < n ? p[at] : 0; }
    quint16 u16(qint64 at) const { return at >= 0 && at + 2 <= n ? qFromBigEndian<quint16>(p + at) : 0; }
    quint32 u32(qint64 at) const { return at >= 0 && at + 4 <= n ? qFromBigEndian<quint32>(p + at) : 0; }
};

struct Table {
    qint64 at = -1, length = 0;
    bool has() const { return at >= 0; }
};

// One name from the name table: the English Windows string, else any Windows or Unicode one, else a Mac Roman one.
QString nameOf(const Bytes &b, const Table &name, int id)
{
    if (!name.has() || name.length < 6) return QString();
    const qint64 count = b.u16(name.at + 2), storage = name.at + b.u16(name.at + 4);
    QString best;
    int bestRank = 0;
    for (qint64 i = 0; i < count && 6 + 12 * (i + 1) <= name.length; ++i) {
        const qint64 r = name.at + 6 + 12 * i;
        if (b.u16(r + 6) != id) continue;
        const int platform = b.u16(r), encoding = b.u16(r + 2), language = b.u16(r + 4), length = b.u16(r + 8);
        const qint64 offset = storage + b.u16(r + 10);
        if (length <= 0 || length > 512 || offset + length > name.at + name.length) continue;
        int rank = 0;
        QString text;
        if ((platform == 3 && (encoding == 1 || encoding == 10)) || platform == 0) {
            rank = platform == 3 && language == 0x409 ? 4 : 3;
            text.resize(length / 2);
            for (int k = 0; k < length / 2; ++k) text[k] = QChar(b.u16(offset + 2 * k));
        } else if (platform == 1 && encoding == 0) {
            rank = 1 + (language == 0 ? 1 : 0);
            text = QString::fromLatin1(reinterpret_cast<const char *>(b.p + offset), length);
        }
        if (rank > bestRank) {
            bestRank = rank;
            best = text;
        }
    }
    return best.trimmed();
}

} // namespace

bool fontAllowsEmbedding(quint16 fsType)
{
    if (fsType & 0x0200) return false;                // bitmap embedding only
    const quint16 level = fsType & 0x000E;
    return level == 0 || (level & 0x08);              // installable, or editable (the freest set)
}

quint8 FontFileInfo::pitchFamily() const
{
    // GDI's tmPitchAndFamily: 1 for fixed pitch and 2 for variable (TrueType), and a family from the PANOSE
    // classification (swiss for sans serif, roman for serif, script, decorative, modern for fixed pitch).
    quint8 family = 0;
    switch (panose[0]) {
    case 2: family = panose[1] >= 11 ? 0x20 : panose[1] >= 2 ? 0x10 : 0; break;
    case 3: family = 0x40; break;
    case 4: family = 0x50; break;
    default: break;
    }
    if (fixedPitch) return quint8((family ? family : 0x30) | 1);
    return quint8(family | 2);
}

quint8 FontFileInfo::charset() const
{
    // The program writes ANSI for fonts of Latin letters and the symbol set for symbol fonts.
    if (codePageRange[0] & 0x80000000u && !(codePageRange[0] & 1)) return 2;
    return 0;
}

// The glyph data, glyph by glyph (loca says where each is): an entry that is not empty has its header and, for a simple glyph, its end
// points (in order) and its program inside it; a composite glyph's components (at most 256) each name a glyph the font has, and the
// composites nest no deeper than 16 levels, or than the font says (maxp's maxComponentDepth, when it says one), with no glyph inside itself.
static bool glyphsAreSound(const Bytes &b, const Table &glyf, const Table &loca, int format, int glyphs, int declaredDepth, QString *why)
{
    auto fail = [&](const char *reason) {
        if (why) *why = say(reason);
        return false;
    };
    auto startOf = [&](int g) { return format ? qint64(b.u32(loca.at + 4 * qint64(g))) : 2 * qint64(b.u16(loca.at + 2 * qint64(g))); };
    constexpr int kMaxComponents = 256, kMaxDepth = 16;
    const int limit = declaredDepth >= 1 ? std::min(declaredDepth, kMaxDepth) : kMaxDepth;
    std::vector<std::vector<int>> components;
    components.resize(size_t(glyphs));
    for (int g = 0; g < glyphs; ++g) {
        const qint64 from = startOf(g), length = startOf(g + 1) - from;
        if (length == 0) continue;
        if (length < 10) return fail("A glyph in the font is too short to be one.");
        const qint64 at = glyf.at + from;
        const int contours = int(qint16(b.u16(at)));
        if (contours >= 0) {
            if (10 + 2 * qint64(contours) + 2 > length) return fail("A glyph in the font is cut short.");
            int last = -1;
            for (int c = 0; c < contours; ++c) {
                const int end = b.u16(at + 10 + 2 * c);
                if (end < last) return fail("A glyph in the font has contours that are out of order.");
                last = end;
            }
            if (10 + 2 * qint64(contours) + 2 + b.u16(at + 10 + 2 * qint64(contours)) > length) return fail("A glyph's program in the font is cut short.");
            continue;
        }
        qint64 used = 10;
        int flags, count = 0;
        do {
            if (used + 4 > length || ++count > kMaxComponents) return fail("A composite glyph in the font is cut short, or has too many parts.");
            flags = b.u16(at + used);
            const int part = b.u16(at + used + 2);
            if (part >= glyphs) return fail("A composite glyph in the font uses a glyph it doesn't have.");
            components[size_t(g)].push_back(part);
            used += 4 + ((flags & 1) ? 4 : 2) + ((flags & 0x08) ? 2 : (flags & 0x40) ? 4 : (flags & 0x80) ? 8 : 0);
            if (used > length) return fail("A composite glyph in the font is cut short.");
        } while (flags & 0x20);
        if ((flags & 0x100) && (used + 2 > length || used + 2 + b.u16(at + used) > length)) return fail("A composite glyph's program in the font is cut short.");
    }
    // Nesting: a simple glyph is level 0, a composite one more than the deepest part of it. Each glyph is worked out once, with a stack of its
    // own and no recursion; a glyph met again while it is being worked out is inside itself.
    std::vector<int> depth(size_t(glyphs), -1), state(size_t(glyphs), 0);
    for (int root = 0; root < glyphs; ++root) {
        if (state[size_t(root)] == 2) continue;
        std::vector<std::pair<int, size_t>> stack{{root, 0}};
        state[size_t(root)] = 1;
        while (!stack.empty()) {
            auto &top = stack.back();
            const auto &parts = components[size_t(top.first)];
            if (top.second < parts.size()) {
                const int part = parts[top.second++];
                if (state[size_t(part)] == 1) return fail("A composite glyph in the font is part of itself.");
                if (state[size_t(part)] == 0) {
                    if (stack.size() > size_t(limit) + 1) return fail("The font's composite glyphs are nested too deeply.");
                    state[size_t(part)] = 1;
                    stack.push_back({part, 0});
                }
                continue;
            }
            int deepest = -1;
            for (int part : parts) deepest = std::max(deepest, depth[size_t(part)]);
            depth[size_t(top.first)] = parts.empty() ? 0 : deepest + 1;
            if (depth[size_t(top.first)] > limit) return fail("The font's composite glyphs are nested too deeply.");
            state[size_t(top.first)] = 2;
            stack.pop_back();
        }
    }
    return true;
}

// The character map's subtables, as far as they say which glyph a character has: each lies inside the table, and none names a glyph the font
// doesn't have (format 0: a byte table; 4: segments of characters, with an offset into an array of glyphs or a change; 6: a run; 12 and 13: groups).
static bool characterMapIsSound(const Bytes &b, const Table &cmap, int subtables, int glyphs, QString *why)
{
    auto fail = [&](const char *reason) {
        if (why) *why = say(reason);
        return false;
    };
    const qint64 end = cmap.length;
    for (int i = 0; i < subtables; ++i) {
        const qint64 o = b.u32(cmap.at + 4 + 8 * qint64(i) + 4), at = cmap.at + o;
        const int format = b.u16(at);
        if (format == 0) {
            if (o + 262 > end) return fail("The font's cmap table is damaged.");
            for (int c = 0; c < 256; ++c)
                if (b.u8(at + 6 + c) >= glyphs) return fail("The font's cmap table names a glyph the font doesn't have.");
        } else if (format == 4) {
            const qint64 length = b.u16(at + 2);
            const int segments = b.u16(at + 6) / 2;
            if (length < 16 || o + length > end || segments < 1 || 16 + 8 * qint64(segments) > length) return fail("The font's cmap table is damaged.");
            const qint64 endCodes = at + 14, startCodes = endCodes + 2 * segments + 2, deltas = startCodes + 2 * segments, offsets = deltas + 2 * segments;
            const qint64 arrayEnd = at + length;
            int before = -1;
            qint64 checked = 0;
            for (int s = 0; s < segments; ++s) {
                const int last = b.u16(endCodes + 2 * s), first = b.u16(startCodes + 2 * s), delta = b.u16(deltas + 2 * s), range = b.u16(offsets + 2 * s);
                if (first > last || first <= before) return fail("The font's cmap table is damaged.");
                before = last;
                for (int c = first; c <= last; ++c) {
                    if (++checked > 70000) return fail("The font's cmap table is damaged.");
                    int glyph;
                    if (range == 0) {
                        glyph = (c + delta) & 0xFFFF;
                    } else {
                        const qint64 where = offsets + 2 * s + range + 2 * (c - first);
                        if (where < offsets + 2 * segments || where + 2 > arrayEnd) return fail("The font's cmap table is damaged.");
                        glyph = b.u16(where);
                        if (glyph != 0) glyph = (glyph + delta) & 0xFFFF;
                    }
                    if (glyph >= glyphs && !(c == 0xFFFF && last == 0xFFFF)) return fail("The font's cmap table names a glyph the font doesn't have.");
                }
            }
        } else if (format == 6) {
            const qint64 length = b.u16(at + 2);
            const int count = b.u16(at + 8);
            if (length < 10 + 2 * qint64(count) || o + length > end) return fail("The font's cmap table is damaged.");
            for (int c = 0; c < count; ++c)
                if (b.u16(at + 10 + 2 * c) >= glyphs) return fail("The font's cmap table names a glyph the font doesn't have.");
        } else if (format == 12 || format == 13) {
            const qint64 length = b.u32(at + 4), groups = b.u32(at + 12);
            if (length < 16 + 12 * groups || o + length > end || groups > 100000) return fail("The font's cmap table is damaged.");
            qint64 before = -1;
            for (qint64 g = 0; g < groups; ++g) {
                const qint64 first = b.u32(at + 16 + 12 * g), last = b.u32(at + 16 + 12 * g + 4), glyph = b.u32(at + 16 + 12 * g + 8);
                if (first > last || first <= before || last > 0x10FFFF) return fail("The font's cmap table is damaged.");
                before = last;
                if (glyph + (format == 12 ? last - first : 0) >= glyphs) return fail("The font's cmap table names a glyph the font doesn't have.");
            }
        }
    }
    return true;
}

bool inspectFont(const QByteArray &font, FontFileInfo *info, QString *why)
{
    return inspectFont(reinterpret_cast<const uchar *>(font.constData()), font.size(), info, why);
}

bool inspectFont(const uchar *data, qint64 size, FontFileInfo *info, QString *why)
{
    auto fail = [&](const QString &reason) {
        if (why) *why = reason;
        return false;
    };
    if (size > kMaxEmbeddedFontBytes) return fail(say("The font is larger than 32 MB."));
    if (size < 12 + 16) return fail(say("The data is not a font."));
    const Bytes b{data, size};
    const quint32 version = b.u32(0);
    if (version == 0x74746366) return fail(say("A font collection can't be used."));
    if (version != kTrueType && version != kTrueTypeTag && version != kOpenTypeCff) return fail(say("The data is not a TrueType or OpenType font."));
    const int count = b.u16(4);
    if (count < 1 || count > kMaxFontTables) return fail(say("The font lists no tables, or too many."));
    const qint64 directoryEnd = 12 + 16 * qint64(count);
    if (directoryEnd > size) return fail(say("The font's table directory runs past the end of the file."));

    QHash<quint32, Table> tables;
    for (int i = 0; i < count; ++i) {
        const qint64 r = 12 + 16 * qint64(i);
        const quint32 tag = b.u32(r);
        for (int k = 0; k < 4; ++k) {
            const uchar c = uchar(tag >> (24 - 8 * k));
            if (c < 0x20 || c > 0x7E) return fail(say("A table in the font has a name that can't be."));
        }
        const qint64 offset = b.u32(r + 8), length = b.u32(r + 12);
        if (offset < directoryEnd || offset + length > size) return fail(say("A table in the font runs past the end of the file."));
        if (tables.contains(tag)) return fail(say("The font lists a table twice."));
        tables.insert(tag, Table{offset, length});
    }
    // The tables lie apart: no byte of the file is two tables' (a font made so would be larger once it is written out again).
    {
        std::vector<std::pair<qint64, qint64>> spans;
        for (const Table &t : std::as_const(tables))
            if (t.length > 0) spans.push_back({t.at, t.at + t.length});
        std::sort(spans.begin(), spans.end());
        for (size_t i = 1; i < spans.size(); ++i)
            if (spans[i].first < spans[i - 1].second) return fail(say("Two tables in the font share bytes."));
    }
    auto tag = [](const char *t) { return quint32(uchar(t[0])) << 24 | quint32(uchar(t[1])) << 16 | quint32(uchar(t[2])) << 8 | uchar(t[3]); };
    auto find = [&](const char *t) { return tables.value(tag(t), Table()); };
    // Fonts with variations carry tables that are meant for a program to run over many values: a document's font is one design.
    for (const char *variable : {"fvar", "gvar", "avar", "HVAR", "MVAR", "CFF2"})
        if (tables.contains(tag(variable))) return fail(say("A font with variations can't be used."));

    const Table head = find("head"), hhea = find("hhea"), maxp = find("maxp"), hmtx = find("hmtx"), cmap = find("cmap"), name = find("name"), os2 = find("OS/2"), post = find("post");
    if (!head.has() || head.length < 54) return fail(say("The font has no usable head table."));
    if (b.u32(head.at + 12) != 0x5F0F3CF5u) return fail(say("The font's head table is damaged."));
    const int unitsPerEm = b.u16(head.at + 18);
    if (unitsPerEm < 16 || unitsPerEm > 16384) return fail(say("The font's units per em can't be."));
    if (!maxp.has() || maxp.length < 6) return fail(say("The font has no usable maxp table."));
    const int glyphs = b.u16(maxp.at + 4);
    if (glyphs < 1) return fail(say("The font has no glyphs."));
    if (!hhea.has() || hhea.length < 36) return fail(say("The font has no usable hhea table."));
    const int metrics = b.u16(hhea.at + 34);
    if (metrics < 1 || metrics > glyphs) return fail(say("The font's metrics can't be."));
    if (!hmtx.has() || hmtx.length < 4 * qint64(metrics) + 2 * qint64(glyphs - metrics)) return fail(say("The font's hmtx table is too short."));
    if (!cmap.has() || cmap.length < 4) return fail(say("The font has no usable cmap table."));
    const int subtables = b.u16(cmap.at + 2);
    if (subtables > 256 || 4 + 8 * qint64(subtables) > cmap.length) return fail(say("The font's cmap table is damaged."));
    for (int i = 0; i < subtables; ++i) {
        const qint64 offset = b.u32(cmap.at + 4 + 8 * qint64(i) + 4);
        if (offset + 4 > cmap.length) return fail(say("The font's cmap table is damaged."));
    }
    if (!characterMapIsSound(b, cmap, subtables, glyphs, why)) return false;

    const bool cff = version == kOpenTypeCff;
    if (cff) {
        const Table cffTable = find("CFF ");
        if (!cffTable.has() || cffTable.length < 4 || b.u8(cffTable.at) != 1) return fail(say("The font has no usable outlines."));
    } else {
        const Table glyf = find("glyf"), loca = find("loca");
        if (!glyf.has() || !loca.has()) return fail(say("The font has no usable outlines."));
        const int format = int(qint16(b.u16(head.at + 50)));
        if (format != 0 && format != 1) return fail(say("The font's glyph index format can't be."));
        const qint64 entry = format ? 4 : 2;
        if (loca.length < entry * (qint64(glyphs) + 1)) return fail(say("The font's loca table is too short."));
        // Each glyph starts inside the glyph data and none starts before the one before it.
        qint64 before = 0;
        for (qint64 i = 0; i <= glyphs; ++i) {
            const qint64 at = format ? qint64(b.u32(loca.at + 4 * i)) : 2 * qint64(b.u16(loca.at + 2 * i));
            if (at > glyf.length || at < before) return fail(say("The font's loca table points outside the glyph data."));
            before = at;
        }
        // maxp 1.0 says how deeply the composite glyphs nest (when it says anything).
        const int declaredDepth = maxp.length >= 32 && b.u32(maxp.at) == 0x00010000 ? b.u16(maxp.at + 30) : 0;
        if (!glyphsAreSound(b, glyf, loca, format, glyphs, declaredDepth, why)) return false;
    }

    if (name.has() && name.length < 6) return fail(say("The font's name table is damaged."));
    if (name.has() && 6 + 12 * qint64(b.u16(name.at + 2)) > name.length) return fail(say("The font's name table is damaged."));
    if (os2.has() && os2.length < 68) return fail(say("The font's OS/2 table is damaged."));

    if (!info) return true;
    FontFileInfo out;
    out.glyphs = glyphs;
    out.outlinesAreCff = cff;
    out.checkSumAdjustment = b.u32(head.at + 8);
    const quint16 macStyle = b.u16(head.at + 44);
    out.bold = macStyle & 1;
    out.italic = macStyle & 2;
    if (os2.has()) {
        out.weight = b.u16(os2.at + 4);
        out.fsType = b.u16(os2.at + 8);
        for (int i = 0; i < 10; ++i) out.panose[i] = b.u8(os2.at + 32 + i);
        for (int i = 0; i < 4; ++i) out.unicodeRange[i] = b.u32(os2.at + 42 + 4 * i);
        const quint16 selection = b.u16(os2.at + 62);
        out.bold = selection & 0x20;
        out.italic = selection & 0x01;
        if (b.u16(os2.at) >= 1 && os2.length >= 86)
            for (int i = 0; i < 2; ++i) out.codePageRange[i] = b.u32(os2.at + 78 + 4 * i);
    }
    if (post.has() && post.length >= 16) out.fixedPitch = b.u32(post.at + 12) != 0;
    out.family = nameOf(b, name, 1);
    out.subfamily = nameOf(b, name, 2);
    out.fullName = nameOf(b, name, 4);
    out.versionName = nameOf(b, name, 5);
    // A typographic family that only groups the four styles is the family too; one that groups more is not what a file names.
    const QString typographicStyle = nameOf(b, name, 17);
    if (const QString typographic = nameOf(b, name, 16); !typographic.isEmpty() && typographic != out.family) {
        static const QStringList plain{QStringLiteral("Regular"), QStringLiteral("Bold"), QStringLiteral("Italic"), QStringLiteral("Bold Italic")};
        if (typographicStyle.isEmpty() || plain.contains(typographicStyle, Qt::CaseInsensitive)) out.typographicFamily = typographic;
    }
    *info = out;
    return true;
}

} // namespace jp
