#include "core/wordformat.h"

#include "core/color.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QDataStream>
#include <QFont>
#include <QIODevice>
#include <QRegularExpression>
#include <cmath>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {

struct Highlight {
    const char *name;
    unsigned rgb;
};
const Highlight kHighlights[] = {{"yellow", 0xFFFF00},     {"green", 0x00FF00},    {"cyan", 0x00FFFF},       {"magenta", 0xFF00FF},   {"blue", 0x0000FF},
                                 {"red", 0xFF0000},        {"darkBlue", 0x000080}, {"darkCyan", 0x008080},   {"darkGreen", 0x008000}, {"darkMagenta", 0x800080},
                                 {"darkRed", 0x800000},    {"darkYellow", 0x808000}, {"darkGray", 0x808080}, {"lightGray", 0xC0C0C0}, {"black", 0x000000},
                                 {"white", 0xFFFFFF}};

QString hex(const QColor &c) { return c.name(QColor::HexRgb).mid(1).toUpper(); }

QColor themed(const Color &c, const Theme *theme)
{
    Theme::Color slot;
    if (!theme || !themeColorFromName(c.theme, &slot)) return QColor();
    QColor base = theme->colors[slot];
    bool ok = false;
    if (const int tint = c.tint.toInt(&ok, 16); ok && !c.tint.isEmpty()) base = mix(QColor(Qt::white), base, tint / 255.0);
    if (const int shade = c.shade.toInt(&ok, 16); ok && !c.shade.isEmpty()) base = mix(QColor(Qt::black), base, shade / 255.0);
    return base;
}

QString themeSlotOfFonts(const Fonts &f, bool *major)
{
    const QString t = !f.asciiTheme.isEmpty() ? f.asciiTheme : f.hAnsiTheme;
    *major = t.startsWith(QLatin1String("major"));
    return t;
}

} // namespace

// ---- packing

QByteArray packAtom(const Inline &a)
{
    QByteArray out;
    QDataStream s(&out, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_6_0);
    s << qint8(a.kind) << qint8(a.role) << a.tag << a.key << a.display << a.raw;
    return out;
}

Inline unpackAtom(const QByteArray &packed)
{
    Inline a;
    QDataStream s(packed);
    s.setVersion(QDataStream::Qt_6_0);
    qint8 kind = 0, role = 0;
    s >> kind >> role >> a.tag >> a.key >> a.display >> a.raw;
    a.kind = kind == Inline::RunAtom ? Inline::RunAtom : Inline::ParaAtom;
    a.role = role == Inline::Open ? Inline::Open : role == Inline::Close ? Inline::Close : Inline::Single;
    return a;
}

QByteArray packBlocks(const QVector<Block> &blocks)
{
    QByteArray out;
    QDataStream s(&out, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_6_0);
    s << qint32(blocks.size());
    for (const Block &b : blocks) s << qint8(b.role) << b.tag << b.key << b.display << b.raw;
    return out;
}

QVector<Block> unpackBlocks(const QByteArray &packed)
{
    QVector<Block> out;
    QDataStream s(packed);
    s.setVersion(QDataStream::Qt_6_0);
    qint32 n = 0;
    s >> n;
    for (qint32 i = 0; i < n && s.status() == QDataStream::Ok && i < 1000000; ++i) {
        Block b;
        qint8 role = 0;
        s >> role >> b.tag >> b.key >> b.display >> b.raw;
        b.isRaw = true;
        b.role = role == Inline::Open ? Inline::Open : role == Inline::Close ? Inline::Close : Inline::Single;
        if (s.status() == QDataStream::Ok) out.push_back(b);
    }
    return out;
}

namespace {
struct ByteStore {
    QVector<QByteArray> items;
    QHash<QByteArray, int> index;
    quint64 generation = 1;
};

ByteStore &byteStore()
{
    static ByteStore s;
    return s;
}
} // namespace

int storeBytes(const QByteArray &bytes)
{
    ByteStore &s = byteStore();
    const auto it = s.index.constFind(bytes);
    if (it != s.index.constEnd()) return it.value();
    const int key = int(s.items.size()) + 1;   // (0 is no key)
    s.items.push_back(bytes);
    s.index.insert(bytes, key);
    return key;
}

QByteArray storedBytesOf(int key)
{
    const ByteStore &s = byteStore();
    return key >= 1 && key <= s.items.size() ? s.items[key - 1] : QByteArray();
}

void setStored(QTextFormat &format, int property, const QByteArray &bytes) { format.setProperty(property, storeBytes(bytes)); }

QByteArray storedBytes(const QTextFormat &format, int property) { return format.hasProperty(property) ? storedBytesOf(format.intProperty(property)) : QByteArray(); }

void clearByteStore()
{
    ByteStore &s = byteStore();
    s.items.clear();
    s.index.clear();
    ++s.generation;
}

quint64 byteStoreGeneration() { return byteStore().generation; }

QString packOtherFonts(const Fonts &f)
{
    const QChar sep(0x1f);
    return f.eastAsia + sep + f.cs + sep + f.eastAsiaTheme + sep + f.csTheme + sep + f.hint;
}

Fonts unpackOtherFonts(const QString &packed)
{
    Fonts f;
    const QStringList parts = packed.split(QChar(0x1f));
    if (parts.size() >= 5) {
        f.eastAsia = parts[0];
        f.cs = parts[1];
        f.eastAsiaTheme = parts[2];
        f.csTheme = parts[3];
        f.hint = parts[4];
    }
    return f;
}

QByteArray packTabs(const QVector<TabStop> &tabs)
{
    QByteArray out;
    QDataStream s(&out, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_6_0);
    s << qint32(tabs.size());
    for (const TabStop &t : tabs) s << qint32(t.pos) << t.val << t.leader;
    return out;
}

QVector<TabStop> unpackTabs(const QByteArray &packed)
{
    QVector<TabStop> out;
    QDataStream s(packed);
    s.setVersion(QDataStream::Qt_6_0);
    qint32 n = 0;
    s >> n;
    for (qint32 i = 0; i < n && s.status() == QDataStream::Ok && i < 1000; ++i) {
        qint32 pos = 0;
        TabStop t;
        s >> pos >> t.val >> t.leader;
        t.pos = pos;
        if (s.status() == QDataStream::Ok) out.push_back(t);
    }
    return out;
}

QByteArray packRaw(const QVector<RawChild> &children)
{
    QByteArray out;
    QDataStream s(&out, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_6_0);
    s << qint32(children.size());
    for (const RawChild &c : children) s << qint32(c.rank) << c.xml;
    return out;
}

QVector<RawChild> unpackRaw(const QByteArray &packed)
{
    QVector<RawChild> out;
    QDataStream s(packed);
    s.setVersion(QDataStream::Qt_6_0);
    qint32 n = 0;
    s >> n;
    for (qint32 i = 0; i < n && s.status() == QDataStream::Ok && i < 100000; ++i) {
        qint32 rank = 0;
        QByteArray xml;
        s >> rank >> xml;
        if (s.status() == QDataStream::Ok) out.push_back({rank, xml});
    }
    return out;
}

QColor highlightColor(const QString &name)
{
    for (const Highlight &h : kHighlights)
        if (name == QLatin1String(h.name)) return QColor::fromRgb(h.rgb);
    return QColor();
}

QString highlightName(const QColor &c)
{
    // The nearest of the fifteen.
    int best = 0, bestDistance = 1 << 30;
    for (int i = 0; i < int(sizeof kHighlights / sizeof *kHighlights); ++i) {
        const QColor k = QColor::fromRgb(kHighlights[i].rgb);
        const int d = (k.red() - c.red()) * (k.red() - c.red()) + (k.green() - c.green()) * (k.green() - c.green()) + (k.blue() - c.blue()) * (k.blue() - c.blue());
        if (d < bestDistance) {
            bestDistance = d;
            best = i;
        }
    }
    return QString::fromLatin1(kHighlights[best].name);
}

QStringList highlightNames()
{
    QStringList out;
    for (const Highlight &h : kHighlights) out << QString::fromLatin1(h.name);
    return out;
}

bool isAtom(const QTextCharFormat &format) { return format.hasProperty(prop::Atom); }

QTextCharFormat withoutAtom(QTextCharFormat f)
{
    f.clearProperty(prop::Atom);
    f.clearProperty(tp::Field);
    return f;
}

// ---- runs

QTextCharFormat charFormatOf(const RunProps &r, const Theme *theme, const RunProps *direct)
{
    QTextCharFormat cf;
    cf.setFontPointSize(r.size.value_or(20) / 2.0);
    if (r.bold.value_or(false)) cf.setFontWeight(QFont::Bold);
    if (r.italic.value_or(false)) cf.setFontItalic(true);

    // The underline's line is Qt's style; double, words only, and thick are the engine's own kinds beside it.
    if (r.underline && r.underline->val != QLatin1String("none") && !r.underline->val.isEmpty()) {
        const QString u = r.underline->val;
        int kind = 0;
        QTextCharFormat::UnderlineStyle style = QTextCharFormat::SingleUnderline;
        if (u == QLatin1String("double")) kind = 1;
        else if (u == QLatin1String("words")) kind = 2;
        else if (u.startsWith(QLatin1String("thick"))) kind = 4;
        else if (u.startsWith(QLatin1String("dotted"))) style = QTextCharFormat::DotLine;
        else if (u.startsWith(QLatin1String("dash"))) style = QTextCharFormat::DashUnderline;
        else if (u.startsWith(QLatin1String("dotDash"))) style = QTextCharFormat::DashDotLine;
        else if (u.startsWith(QLatin1String("dotDotDash"))) style = QTextCharFormat::DashDotDotLine;
        else if (u.startsWith(QLatin1String("wave"))) style = QTextCharFormat::WaveUnderline;
        if (u.endsWith(QLatin1String("Heavy"))) kind |= 4;
        if (u == QLatin1String("wavyDouble")) kind |= 1;
        cf.setUnderlineStyle(style);
        if (kind) cf.setProperty(tp::UnderlineKind, kind);
        const QString uc = r.underline->color;
        if (uc.size() == 6) cf.setUnderlineColor(QColor(QLatin1Char('#') + uc));
    }
    if (r.strike.value_or(false) || r.dstrike.value_or(false)) cf.setFontStrikeOut(true);
    if (r.dstrike.value_or(false)) cf.setProperty(prop::DoubleStrike, true);
    if (r.caps.value_or(false)) cf.setFontCapitalization(QFont::AllUppercase);
    else if (r.smallCaps.value_or(false)) cf.setFontCapitalization(QFont::SmallCaps);
    if (r.vertAlign == QLatin1String("superscript")) cf.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    else if (r.vertAlign == QLatin1String("subscript")) cf.setVerticalAlignment(QTextCharFormat::AlignSubScript);
    if (r.spacing && *r.spacing != 0) {
        cf.setFontLetterSpacingType(QFont::AbsoluteSpacing);
        cf.setFontLetterSpacing(*r.spacing / 20.0);
    }
    if (r.position && *r.position != 0) cf.setProperty(prop::Raised, *r.position);
    if (r.vanish.value_or(false)) cf.setProperty(prop::Hidden, true);
    if (r.noProof.value_or(false)) cf.setProperty(tp::NoProof, true);
    if (!r.style.isEmpty()) cf.setProperty(tp::CharStyle, r.style);

    // The typefaces: a theme font names no family (the engine takes the document's font scheme).
    if (r.fonts) {
        bool major = false;
        const QString themeName = themeSlotOfFonts(*r.fonts, &major);
        const QString family = !r.fonts->ascii.isEmpty() ? r.fonts->ascii : r.fonts->hAnsi;
        if (!themeName.isEmpty()) cf.setProperty(tp::ThemeFont, major ? QStringLiteral("major") : QStringLiteral("minor"));
        else if (!family.isEmpty()) cf.setFontFamilies({family});
    }
    if (!cf.hasProperty(QTextFormat::FontFamilies) && !cf.hasProperty(tp::ThemeFont)) cf.setProperty(tp::ThemeFont, QStringLiteral("minor"));

    if (r.color && r.color->val != QLatin1String("auto")) {
        QColor c;
        if (r.color->val.size() == 6) c = QColor(QLatin1Char('#') + r.color->val);
        if (!c.isValid()) c = themed(*r.color, theme);
        if (c.isValid()) {
            cf.setForeground(QBrush(c));
            cf.setProperty(tp::ColorRefP, ColorRef::rgb(c).toString());
            if (!r.color->theme.isEmpty() || !r.color->tint.isEmpty() || !r.color->shade.isEmpty()) {
                cf.setProperty(prop::ColorAttrs, r.color->theme + QLatin1Char(';') + r.color->tint + QLatin1Char(';') + r.color->shade);
                cf.setProperty(prop::ColorAttrsRgb, hex(c));
            }
        }
    }
    if (!r.highlight.isEmpty() && r.highlight != QLatin1String("none")) {
        const QColor h = highlightColor(r.highlight);
        if (h.isValid()) cf.setProperty(tp::HighlightRefP, ColorRef::rgb(h).toString());
    }
    if (!r.extra.isEmpty()) setStored(cf, prop::RawRPr, packRaw(r.extra));
    if (direct && direct->fonts && direct->fonts->hasOtherScripts()) cf.setProperty(prop::FontsOther, packOtherFonts(*direct->fonts));
    return cf;
}

RunProps runPropsOf(const QTextCharFormat &f)
{
    RunProps r;
    Fonts fonts;
    if (f.hasProperty(QTextFormat::FontFamilies) && !f.fontFamilies().toStringList().isEmpty()) {
        fonts.ascii = fonts.hAnsi = f.fontFamilies().toStringList().first();
    } else {
        const QString p = f.stringProperty(tp::ThemeFont) == QLatin1String("major") ? QStringLiteral("major") : QStringLiteral("minor");
        fonts.asciiTheme = fonts.hAnsiTheme = p + QStringLiteral("HAnsi");
    }
    // (The other scripts' fonts are those the run said, if it said any.)
    if (f.hasProperty(prop::FontsOther)) {
        const Fonts other = unpackOtherFonts(f.stringProperty(prop::FontsOther));
        fonts.eastAsia = other.eastAsia;
        fonts.cs = other.cs;
        fonts.eastAsiaTheme = other.eastAsiaTheme;
        fonts.csTheme = other.csTheme;
        fonts.hint = other.hint;
    }
    r.fonts = fonts;
    r.bold = f.fontWeight() >= QFont::DemiBold;
    r.italic = f.fontItalic();
    const QTextCharFormat::UnderlineStyle us = f.underlineStyle();
    Underline u;
    const int kind = f.intProperty(tp::UnderlineKind);
    switch (us) {
    case QTextCharFormat::NoUnderline: u.val = QStringLiteral("none"); break;
    case QTextCharFormat::DotLine: u.val = QStringLiteral("dotted"); break;
    case QTextCharFormat::DashUnderline: u.val = QStringLiteral("dash"); break;
    case QTextCharFormat::DashDotLine: u.val = QStringLiteral("dotDash"); break;
    case QTextCharFormat::DashDotDotLine: u.val = QStringLiteral("dotDotDash"); break;
    case QTextCharFormat::WaveUnderline: u.val = (kind & 1) ? QStringLiteral("wavyDouble") : QStringLiteral("wave"); break;
    default: u.val = (kind & 1) ? QStringLiteral("double") : (kind & 2) ? QStringLiteral("words") : (kind & 4) ? QStringLiteral("thick") : QStringLiteral("single"); break;
    }
    if (us != QTextCharFormat::NoUnderline && f.hasProperty(QTextFormat::TextUnderlineColor)) u.color = hex(f.underlineColor());
    r.underline = u;
    const bool strike = f.fontStrikeOut(), dstrike = f.boolProperty(prop::DoubleStrike);
    r.strike = strike && !dstrike;
    r.dstrike = strike && dstrike;
    r.caps = f.fontCapitalization() == QFont::AllUppercase;
    r.smallCaps = f.fontCapitalization() == QFont::SmallCaps;
    r.vanish = f.boolProperty(prop::Hidden);
    r.noProof = f.boolProperty(tp::NoProof);
    r.vertAlign = f.verticalAlignment() == QTextCharFormat::AlignSuperScript ? QStringLiteral("superscript")
                  : f.verticalAlignment() == QTextCharFormat::AlignSubScript ? QStringLiteral("subscript")
                                                                             : QStringLiteral("baseline");
    if (f.hasProperty(QTextFormat::FontPointSize)) r.size = std::clamp(int(std::lround(f.fontPointSize() * 2)), 1, 4000);
    r.spacing = f.hasProperty(QTextFormat::FontLetterSpacing) && f.fontLetterSpacingType() == QFont::AbsoluteSpacing ? int(std::lround(f.fontLetterSpacing() * 20)) : 0;
    r.position = f.intProperty(prop::Raised);
    Color color{QStringLiteral("auto"), QString(), QString(), QString()};
    if (f.hasProperty(QTextFormat::ForegroundBrush) && f.foreground().style() != Qt::NoBrush) {
        const QColor c = f.foreground().color();
        color.val = hex(c);
        const QString attrs = f.stringProperty(prop::ColorAttrs);
        if (!attrs.isEmpty() && f.stringProperty(prop::ColorAttrsRgb) == color.val) {
            const QStringList parts = attrs.split(QLatin1Char(';'));
            color.theme = parts.value(0);
            color.tint = parts.value(1);
            color.shade = parts.value(2);
        }
    }
    r.color = color;
    const QString h = f.stringProperty(tp::HighlightRefP);
    r.highlight = h.isEmpty() ? QStringLiteral("none") : highlightName(ColorRef::fromString(h).rgbValue());
    r.style = f.stringProperty(tp::CharStyle);
    if (f.hasProperty(prop::RawRPr)) r.extra = unpackRaw(storedBytes(f, prop::RawRPr));
    return r;
}

// ---- paragraphs

QTextBlockFormat blockFormatOf(const ParaProps &p)
{
    QTextBlockFormat bf;
    const QString jc = p.jc;
    if (jc == QLatin1String("center")) bf.setAlignment(Qt::AlignHCenter);
    else if (jc == QLatin1String("right") || jc == QLatin1String("end")) bf.setAlignment(Qt::AlignRight);
    else if (jc == QLatin1String("both") || jc == QLatin1String("distribute") || jc == QLatin1String("highKashida") || jc == QLatin1String("lowKashida") || jc == QLatin1String("mediumKashida") || jc == QLatin1String("thaiDistribute"))
        bf.setAlignment(Qt::AlignJustify);
    else bf.setAlignment(Qt::AlignLeft);
    if (jc == QLatin1String("distribute") || jc == QLatin1String("thaiDistribute") || jc.endsWith(QLatin1String("Kashida"))) bf.setProperty(prop::JcRaw, jc);
    bf.setTopMargin(p.before.value_or(0) / 20.0);
    bf.setBottomMargin(p.after.value_or(0) / 20.0);
    bf.setLeftMargin(p.indLeft.value_or(0) / 20.0);
    bf.setRightMargin(p.indRight.value_or(0) / 20.0);
    bf.setTextIndent(p.indFirst.value_or(0) / 20.0);
    const int line = p.line.value_or(240);
    if (p.lineRule == QLatin1String("exact")) bf.setLineHeight(std::max(1, line) / 20.0, QTextBlockFormat::FixedHeight);
    else if (p.lineRule == QLatin1String("atLeast")) bf.setLineHeight(std::max(1, line) / 20.0, QTextBlockFormat::MinimumHeight);
    else if (line != 240) bf.setLineHeight(std::max(1, line) * 100.0 / 240.0, QTextBlockFormat::ProportionalHeight);
    if (p.keepNext.value_or(false)) bf.setProperty(tp::KeepWithNext, true);
    if (p.keepLines.value_or(false)) bf.setProperty(tp::KeepTogether, true);
    if (p.widowControl.value_or(true)) bf.setProperty(tp::WidowControl, true);
    if (p.pageBreakBefore.value_or(false)) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
    if (p.contextualSpacing.value_or(false)) bf.setProperty(prop::Contextual, true);
    if (p.outlineLvl) bf.setProperty(prop::Outline, *p.outlineLvl);
    if (!p.style.isEmpty()) bf.setProperty(tp::StyleName, p.style);
    if (p.numId && *p.numId > 0) {
        bf.setProperty(prop::NumId, *p.numId);
        bf.setProperty(prop::NumLevel, p.ilvl.value_or(0));
    }
    if (p.tabs && !p.tabs->isEmpty()) {
        QList<QTextOption::Tab> tabs;
        QString leaders;
        for (const TabStop &t : *p.tabs) {
            QTextOption::Tab tab;
            tab.position = t.pos / 20.0;
            if (t.val == QLatin1String("right") || t.val == QLatin1String("end")) tab.type = QTextOption::RightTab;
            else if (t.val == QLatin1String("center")) tab.type = QTextOption::CenterTab;
            else if (t.val == QLatin1String("decimal")) {
                tab.type = QTextOption::DelimiterTab;
                tab.delimiter = QLatin1Char('.');
            } else if (t.val == QLatin1String("bar") || t.val == QLatin1String("num")) continue;
            else tab.type = QTextOption::LeftTab;
            tabs << tab;
            leaders += t.leader == QLatin1String("dot") ? QLatin1Char('.')
                       : t.leader == QLatin1String("hyphen") ? QLatin1Char('-')
                       : (t.leader == QLatin1String("underscore") || t.leader == QLatin1String("heavy")) ? QLatin1Char('_')
                       : t.leader == QLatin1String("middleDot") ? QChar(0x00B7)
                                                                : QLatin1Char(' ');
        }
        bf.setTabPositions(tabs);
        if (leaders.contains(QRegularExpression(QStringLiteral("[^ ]")))) bf.setProperty(tp::TabLeaders, leaders);
    }
    if (!p.extra.isEmpty()) setStored(bf, prop::RawPPr, packRaw(p.extra));
    if (!p.spacingAttrs.isEmpty()) setStored(bf, prop::SpacingAttrs, p.spacingAttrs);
    if (!p.indAttrs.isEmpty()) setStored(bf, prop::IndAttrs, p.indAttrs);
    return bf;
}

ParaProps paraPropsOf(const QTextBlockFormat &f)
{
    ParaProps p;
    p.style = f.stringProperty(tp::StyleName);
    const Qt::Alignment al = f.alignment() & Qt::AlignHorizontal_Mask;
    p.jc = al == Qt::AlignHCenter ? QStringLiteral("center")
           : (al == Qt::AlignRight || al == Qt::AlignTrailing) ? QStringLiteral("right")
           : al == Qt::AlignJustify ? QStringLiteral("both")
                                    : QStringLiteral("left");
    // (An alignment that is a kind of justified keeps the kind it was, while it is still justified.)
    if (al == Qt::AlignJustify && f.hasProperty(prop::JcRaw)) p.jc = f.stringProperty(prop::JcRaw);
    p.before = int(std::lround(f.topMargin() * 20));
    p.after = int(std::lround(f.bottomMargin() * 20));
    p.indLeft = int(std::lround(f.leftMargin() * 20));
    p.indRight = int(std::lround(f.rightMargin() * 20));
    p.indFirst = int(std::lround(f.textIndent() * 20));
    switch (f.lineHeightType()) {
    case QTextBlockFormat::FixedHeight:
        p.line = int(std::lround(f.lineHeight() * 20));
        p.lineRule = QStringLiteral("exact");
        break;
    case QTextBlockFormat::MinimumHeight:
        p.line = int(std::lround(f.lineHeight() * 20));
        p.lineRule = QStringLiteral("atLeast");
        break;
    case QTextBlockFormat::ProportionalHeight:
        p.line = int(std::lround(f.lineHeight() * 240 / 100));
        p.lineRule = QStringLiteral("auto");
        break;
    default:
        p.line = 240;
        p.lineRule = QStringLiteral("auto");
    }
    p.keepNext = f.boolProperty(tp::KeepWithNext);
    p.keepLines = f.boolProperty(tp::KeepTogether);
    p.widowControl = f.boolProperty(tp::WidowControl);
    p.pageBreakBefore = (f.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore) != 0;
    p.contextualSpacing = f.boolProperty(prop::Contextual);
    if (f.hasProperty(prop::Outline)) p.outlineLvl = f.intProperty(prop::Outline);
    if (f.hasProperty(prop::NumId)) {
        p.numId = f.intProperty(prop::NumId);
        p.ilvl = f.intProperty(prop::NumLevel);
    } else {
        p.numId = 0;
    }
    QVector<TabStop> tabs;
    const QString leaders = f.stringProperty(tp::TabLeaders);
    const QList<QTextOption::Tab> qtabs = f.tabPositions();
    for (int i = 0; i < qtabs.size(); ++i) {
        TabStop t;
        t.pos = int(std::lround(qtabs[i].position * 20));
        t.val = qtabs[i].type == QTextOption::RightTab ? QStringLiteral("right")
                : qtabs[i].type == QTextOption::CenterTab ? QStringLiteral("center")
                : qtabs[i].type == QTextOption::DelimiterTab ? QStringLiteral("decimal")
                                                              : QStringLiteral("left");
        const QChar leader = i < leaders.size() ? leaders[i] : QLatin1Char(' ');
        t.leader = leader == QLatin1Char('.') ? QStringLiteral("dot")
                   : leader == QLatin1Char('-') ? QStringLiteral("hyphen")
                   : leader == QLatin1Char('_') ? QStringLiteral("underscore")
                   : leader == QChar(0x00B7) ? QStringLiteral("middleDot")
                                              : QString();
        tabs.push_back(t);
    }
    p.tabs = tabs;
    if (f.hasProperty(prop::RawPPr)) p.extra = unpackRaw(storedBytes(f, prop::RawPPr));
    if (f.hasProperty(prop::SpacingAttrs)) p.spacingAttrs = storedBytes(f, prop::SpacingAttrs);
    if (f.hasProperty(prop::IndAttrs)) p.indAttrs = storedBytes(f, prop::IndAttrs);
    return p;
}

} // namespace jp::write
