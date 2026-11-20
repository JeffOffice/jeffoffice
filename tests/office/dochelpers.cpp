// The writer of old binary word processing files that the tests use: see dochelpers.h.

#include "dochelpers.h"

#include "io/cfb.h"
#include "io/crypto.h"

#include <QBuffer>
#include <QDateTime>
#include <QTimeZone>
#include <QCryptographicHash>
#include <QHash>
#include <QPair>
#include <QImage>
#include <algorithm>
#include <functional>

namespace dochelp {

namespace {

void put16(QByteArray &b, int at, int v)
{
    b[at] = char(v & 0xFF);
    b[at + 1] = char((v >> 8) & 0xFF);
}
void put32(QByteArray &b, int at, quint32 v)
{
    for (int i = 0; i < 4; ++i) b[at + i] = char((v >> (8 * i)) & 0xFF);
}
void add8(QByteArray &b, int v) { b.append(char(v & 0xFF)); }
void add16(QByteArray &b, int v)
{
    add8(b, v);
    add8(b, v >> 8);
}
void add32(QByteArray &b, quint32 v)
{
    for (int i = 0; i < 4; ++i) add8(b, int(v >> (8 * i)));
}
void padEven(QByteArray &b)
{
    if (b.size() & 1) b.append('\0');
}
QByteArray utf16(const QString &s)
{
    QByteArray b;
    for (QChar c : s) add16(b, c.unicode());
    return b;
}

// One stretch of the stream with the sprms that apply to it.
struct Entry {
    quint32 from, to;
    QByteArray sprms;
    int istd = 0;
};

// Pages of 512 bytes holding the entries (character properties, or paragraph properties), appended to `wd`; the table of them (a PLC of page
// numbers) is returned.
QByteArray writeBin(QByteArray &wd, const QVector<Entry> &entries, bool paragraphs)
{
    struct Page {
        QVector<Entry> entries;
    };
    QVector<Page> pages;
    Page cur;
    int used = 0;
    auto sizeOf = [&](const Entry &e) {
        if (paragraphs) {
            if (e.sprms.isEmpty() && e.istd == 0) return 0;
            int l = 2 + int(e.sprms.size());
            if (!(l & 1)) ++l;
            return 1 + l;
        }
        return e.sprms.isEmpty() ? 0 : int((1 + e.sprms.size() + 1) & ~1);
    };
    for (const Entry &e : entries) {
        const int n = int(cur.entries.size());
        const int need = sizeOf(e);
        const int fixed = 4 * (n + 2) + (paragraphs ? 13 : 1) * (n + 1);
        if (n > 0 && fixed + used + need > 511) {
            pages.push_back(cur);
            cur = Page();
            used = 0;
        }
        cur.entries.push_back(e);
        used += need;
    }
    if (!cur.entries.isEmpty()) pages.push_back(cur);

    QByteArray plc;
    QVector<quint32> pns;
    while (wd.size() % 512) wd.append('\0');
    for (const Page &p : pages) {
        QByteArray page(512, '\0');
        const int crun = int(p.entries.size());
        int top = 511;   // data goes below this, downward
        for (int i = 0; i < crun; ++i) {
            const Entry &e = p.entries[i];
            put32(page, 4 * i, e.from);
            put32(page, 4 * i + 4, e.to);
            const int need = sizeOf(e);
            if (!need) continue;
            top -= need;
            if (top & 1) --top;
            QByteArray rec;
            if (paragraphs) {
                QByteArray body;
                add16(body, e.istd);
                body.append(e.sprms);
                if (!(body.size() & 1)) body.append('\0');
                add8(rec, (body.size() + 1) / 2);
                rec.append(body);
            } else {
                add8(rec, int(e.sprms.size()));
                rec.append(e.sprms);
            }
            page.replace(top, int(rec.size()), rec);
            if (paragraphs) page[4 * (crun + 1) + 13 * i] = char(top / 2);
            else page[4 * (crun + 1) + i] = char(top / 2);
        }
        page[511] = char(crun);
        pns.push_back(quint32(wd.size() / 512));
        wd.append(page);
    }
    for (int i = 0; i < pages.size(); ++i) add32(plc, pages[i].entries.first().from);
    add32(plc, pages.isEmpty() ? 0 : pages.last().entries.last().to);
    for (quint32 pn : pns) add32(plc, pn);
    return plc;
}

} // namespace

QByteArray sprm1(quint16 op, int v)
{
    QByteArray b;
    add16(b, op);
    add8(b, v);
    return b;
}
QByteArray sprm2(quint16 op, int v)
{
    QByteArray b;
    add16(b, op);
    add16(b, v);
    return b;
}
QByteArray sprm4(quint16 op, quint32 v)
{
    QByteArray b;
    add16(b, op);
    add32(b, v);
    return b;
}
QByteArray sprmVar(quint16 op, const QByteArray &operand)
{
    QByteArray b;
    add16(b, op);
    add8(b, int(operand.size()));
    b.append(operand);
    return b;
}

quint16 prm0(int isprm, int val)
{
    return quint16((isprm << 1) | (val << 8));
}

quint32 dttm(int year, int month, int day, int hour, int minute)
{
    // Minutes in the low six bits, then hours (5), day (5), month (4), years since 1900 (9), and the weekday (3, left as 0).
    return quint32(minute) | (quint32(hour) << 6) | (quint32(day) << 11) | (quint32(month) << 16) | (quint32(year - 1900) << 20);
}

QByteArray tableRow(const QVector<int> &edges)
{
    const int cells = int(edges.size()) - 1;
    QByteArray data;
    add8(data, cells);
    for (int e : edges) add16(data, e);
    for (int i = 0; i < cells; ++i) data.append(QByteArray(20, '\0'));
    QByteArray b;
    add16(b, 0xD608);
    add16(b, int(data.size()) + 1);
    b.append(data);
    return b;
}

QByteArray tinyPng(int width, int height)
{
    QImage img(width, height, QImage::Format_RGB32);
    img.fill(QColor(200, 30, 30));
    QByteArray bytes;
    QBuffer buf(&bytes);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    return bytes;
}

namespace {

// A picture in the data stream: the header with the size it is shown at, an empty drawing, and the file block that holds the picture's record (of the
// kind `kind`, the BLIP record type, with `instance` and `body` after its header).
int addPictureRecord(QByteArray &data, int kind, int instance, const QByteArray &body, int widthTwips, int heightTwips)
{
    const int at = int(data.size());
    QByteArray blipRecord;
    add16(blipRecord, instance << 4);   // (instance in the high twelve bits)
    add16(blipRecord, kind);
    add32(blipRecord, quint32(body.size()));
    blipRecord.append(body);
    QByteArray fbse;
    add8(fbse, 6);   // the kinds: PNG
    add8(fbse, 6);
    fbse.append(QByteArray(16, 'u'));
    add16(fbse, 0xFF);
    add32(fbse, quint32(blipRecord.size()));
    add32(fbse, 1);
    add32(fbse, 0);
    add8(fbse, 0);
    add8(fbse, 0);
    add8(fbse, 0);
    add8(fbse, 0);
    fbse.append(blipRecord);
    QByteArray fbseRecord;
    add16(fbseRecord, (6 << 4) | 2);
    add16(fbseRecord, 0xF007);
    add32(fbseRecord, quint32(fbse.size()));
    fbseRecord.append(fbse);
    QByteArray shape;
    add16(shape, 0x000F);
    add16(shape, 0xF004);
    add32(shape, 0);
    QByteArray picf(68, '\0');
    const int total = 68 + int(shape.size() + fbseRecord.size());
    put32(picf, 0, quint32(total));
    put16(picf, 4, 68);
    put16(picf, 6, 100);
    put16(picf, 28, widthTwips);
    put16(picf, 30, heightTwips);
    put16(picf, 32, 1000);
    put16(picf, 34, 1000);
    data.append(picf);
    data.append(shape);
    data.append(fbseRecord);
    return at;
}

} // namespace

int DocWriter::addPicture(const QByteArray &png, int widthTwips, int heightTwips)
{
    QByteArray body(16, 'u');
    body.append('\xFF');
    body.append(png);
    return addPictureRecord(data, 0xF01E, 0x6E0, body, widthTwips, heightTwips);
}

int DocWriter::addMetafile(int inflatedBytes, int widthTwips, int heightTwips)
{
    // An enhanced metafile of `inflatedBytes` zero bytes, kept deflated (a few kilobytes in the file).
    const QByteArray deflated = qCompress(QByteArray(inflatedBytes, '\0'), 9).mid(4);
    QByteArray body(16, 'u');
    add32(body, quint32(inflatedBytes));   // its size once inflated
    body.append(QByteArray(16, '\0'));     // its bounds
    body.append(QByteArray(8, '\0'));      // its size
    add32(body, quint32(deflated.size()));
    add8(body, 0);                         // deflated
    add8(body, 0xFE);
    body.append(deflated);
    return addPictureRecord(data, 0xF01A, 0x3D4, body, widthTwips, heightTwips);
}

QByteArray formFieldData(const FormFieldSpec &f)
{
    QByteArray b;
    add32(b, 0xFFFFFFFFu);
    const int bits = (f.type & 3) | ((f.state & 0x1F) << 2) | (f.ownHelp ? 0x80 : 0) | (f.ownStatus ? 0x100 : 0) | (f.locked ? 0x200 : 0) | (f.autoSize ? 0x400 : 0) |
                     ((f.textType & 7) << 11) | (f.type == 2 ? 0x8000 : 0);
    add16(b, bits);
    add16(b, f.longest);
    add16(b, f.boxSize);
    auto xstz = [&](const QString &s) {
        add16(b, int(s.size()));
        b.append(utf16(s));
        add16(b, 0);
    };
    xstz(f.name);
    if (f.type == 0) xstz(f.text);
    else add16(b, f.wDef);
    xstz(f.format);
    xstz(f.help);
    xstz(f.status);
    xstz(QString());
    xstz(QString());
    if (f.type == 2) {
        add16(b, 0xFFFF);
        add16(b, int(f.entries.size()));
        add16(b, 0);
        for (const QString &e : f.entries) {
            add16(b, int(e.size()));
            b.append(utf16(e));
        }
    }
    return b;
}

int DocWriter::addFormFieldData(const QByteArray &ffData)
{
    const int at = int(data.size());
    QByteArray r;
    add32(r, quint32(68 + ffData.size()));
    add16(r, 0x44);
    r.append(QByteArray(62, '\0'));
    r.append(ffData);
    data.append(r);
    return at;
}

int DocWriter::addRule(int heightTwips)
{
    const int at = int(data.size());
    auto record = [](int version, int instance, int type, const QByteArray &d) {
        QByteArray r;
        add16(r, (instance << 4) | version);
        add16(r, type);
        add32(r, quint32(d.size()));
        r.append(d);
        return r;
    };
    QByteArray fsp;
    add32(fsp, 1025);
    add32(fsp, 0x0A00);
    QByteArray props;
    auto prop = [&](int id, quint32 value) {
        add16(props, id);
        add32(props, value);
    };
    prop(0x181, 0x808080);
    prop(0x1BF, 0x110010);
    prop(0x1FF, 0x80000);
    prop(0x394, 1);
    QByteArray shape = record(0xF, 0, 0xF004, record(2, 1, 0xF00A, fsp) + record(3, 4, 0xF00B, props));
    QByteArray picf(68, '\0');
    put32(picf, 0, quint32(68 + shape.size()));
    put16(picf, 4, 68);
    put16(picf, 6, 100);
    put16(picf, 28, 0);
    put16(picf, 30, heightTwips);
    put16(picf, 32, 1000);
    put16(picf, 34, 1000);
    data.append(picf);
    data.append(shape);
    return at;
}

namespace {

// Text as the bytes of an 8-bit piece: the Windows Western coding (Latin-1, with the curly quotes, dashes, and others in 0x80 to 0x9F); what it has no byte for is a '?'.
QByteArray westernBytes(const QString &s)
{
    static const QHash<ushort, uchar> high = {{0x20AC, 0x80}, {0x201A, 0x82}, {0x0192, 0x83}, {0x201E, 0x84}, {0x2026, 0x85}, {0x2020, 0x86}, {0x2021, 0x87}, {0x02C6, 0x88},
                                              {0x2030, 0x89}, {0x0160, 0x8A}, {0x2039, 0x8B}, {0x0152, 0x8C}, {0x017D, 0x8E}, {0x2018, 0x91}, {0x2019, 0x92}, {0x201C, 0x93},
                                              {0x201D, 0x94}, {0x2022, 0x95}, {0x2013, 0x96}, {0x2014, 0x97}, {0x02DC, 0x98}, {0x2122, 0x99}, {0x0161, 0x9A}, {0x203A, 0x9B},
                                              {0x0153, 0x9C}, {0x017E, 0x9E}, {0x0178, 0x9F}};
    QByteArray out;
    out.reserve(s.size());
    for (const QChar qc : s) {
        const ushort u = qc.unicode();
        if (u < 0x80 || (u >= 0xA0 && u <= 0xFF)) out.append(char(u));
        else if (high.contains(u)) out.append(char(high.value(u)));
        else if (u == 0x81 || u == 0x8D || u == 0x8F || u == 0x90 || u == 0x9D) out.append(char(u));   // (the five the coding leaves empty)
        else out.append('?');
    }
    return out;
}

} // namespace

QByteArray DocWriter::build() const
{
    QString footnoteText;
    QVector<int> noteStart;
    for (const QString &n : footnotes) {
        noteStart.push_back(int(footnoteText.size()));
        footnoteText += n;
    }
    QString headerText;
    QVector<int> storyStart;
    for (const QString &s : headerStories) {
        storyStart.push_back(int(headerText.size()));
        if (!s.isEmpty()) headerText += s + QLatin1Char('\r');
    }
    storyStart.push_back(int(headerText.size()));
    QString commentText;
    QVector<int> commentStart;
    for (const CommentSpec &cm : comments) {
        commentStart.push_back(int(commentText.size()));
        commentText += QString(QChar(5)) + cm.text + (cm.text.endsWith(QLatin1Char('\r')) ? QString() : QStringLiteral("\r"));
    }
    // The text of the boxes of the shapes: a story each (with the mark that ends it), in the order of the shapes.
    QString boxText, headerBoxText;
    QVector<int> boxStart, headerBoxStart;
    QVector<int> boxShape, headerBoxShape;
    for (int i = 0; i < shapes.size(); ++i) {
        if (shapes[i].text.isEmpty()) continue;
        QString &dst = shapes[i].header ? headerBoxText : boxText;
        (shapes[i].header ? headerBoxStart : boxStart).push_back(int(dst.size()));
        (shapes[i].header ? headerBoxShape : boxShape).push_back(i);
        dst += shapes[i].text + QLatin1Char('\r');
    }
    const QString all = text + footnoteText + headerText + commentText + boxText + headerBoxText;
    const int copies = std::max(1, repeatText);
    const int ccpText = int(text.size()) * copies, ccpFtn = int(footnoteText.size()), ccpHdd = int(headerText.size());
    const int ccpAtn = int(commentText.size());
    const int ccpTxbx = int(boxText.size()), ccpHdrTxbx = int(headerBoxText.size());

    QByteArray wd(1280, '\0');
    // The text, in one or two pieces.
    struct Piece {
        int cp0, cp1;
        quint32 fc;
        bool unicode;
        quint16 prm = 0;
    };
    QVector<Piece> pieces;
    if (splitAt > 0 && splitAt < all.size()) {
        const int firstLen = splitAt;
        pieces.push_back({0, firstLen, quint32(wd.size()), false});
        wd.append(westernBytes(all.left(firstLen)));
        padEven(wd);
        pieces.push_back({firstLen, int(all.size()), quint32(wd.size()), true});
        wd.append(utf16(all.mid(firstLen)));
    } else {
        pieces.push_back({0, int(all.size()), quint32(wd.size()), unicode});
        wd.append(unicode ? utf16(all) : westernBytes(all));
    }
    if (!pieceMods.isEmpty()) {
        QVector<Piece> split;
        for (const Piece &p : pieces) {
            QVector<int> cuts = {p.cp0, p.cp1};
            for (const PieceMod &m : pieceMods) {
                if (m.from > p.cp0 && m.from < p.cp1) cuts.push_back(m.from);
                if (m.to > p.cp0 && m.to < p.cp1) cuts.push_back(m.to);
            }
            std::sort(cuts.begin(), cuts.end());
            cuts.erase(std::unique(cuts.begin(), cuts.end()), cuts.end());
            for (int k = 0; k + 1 < cuts.size(); ++k) {
                Piece q = {cuts[k], cuts[k + 1], p.fc + quint32(cuts[k] - p.cp0) * (p.unicode ? 2 : 1), p.unicode, 0};
                for (const PieceMod &m : pieceMods)
                    if (q.cp0 >= m.from && q.cp1 <= m.to) q.prm = m.prm;
                split.push_back(q);
            }
        }
        pieces = split;
    }
    auto fcOf = [&](int cp) -> quint32 {
        for (const Piece &p : pieces)
            if (cp >= p.cp0 && cp <= p.cp1) return p.fc + quint32(cp - p.cp0) * (p.unicode ? 2 : 1);
        return pieces.last().fc;
    };
    // The runs as stretches of the stream (cut where the pieces are), with the gaps between them given no properties.
    auto stretches = [&](const QVector<Entry> &given) {
        QVector<Entry> out;
        QVector<Entry> sorted = given;
        std::sort(sorted.begin(), sorted.end(), [](const Entry &a, const Entry &b) { return a.from < b.from; });
        quint32 at = pieces.first().fc;
        const quint32 end = fcOf(int(all.size()));
        for (const Entry &e : sorted) {
            if (e.from > at) out.push_back({at, e.from, QByteArray(), 0});
            out.push_back(e);
            at = e.to;
        }
        if (at < end) out.push_back({at, end, QByteArray(), 0});
        return out;
    };
    auto cut = [&](int from, int to, const QByteArray &sprms, int istd, QVector<Entry> *out) {
        for (const Piece &p : pieces) {
            const int a = std::max(from, p.cp0), b = std::min(to, p.cp1);
            if (a < b) out->push_back({fcOf(a), fcOf(b), sprms, istd});
        }
    };
    QVector<Entry> charEntries, paraEntries;
    for (const CharRun &r : chars) cut(r.from, r.to, r.sprms, 0, &charEntries);
    for (int i = 0; i < comments.size(); ++i) {   // (the characters that cite a comment, and begin its text, are special)
        cut(comments[i].ref, comments[i].ref + 1, sprm1(sprm::special, 1), 0, &charEntries);
        cut(ccpText + ccpFtn + ccpHdd + commentStart[i], ccpText + ccpFtn + ccpHdd + commentStart[i] + 1, sprm1(sprm::special, 1), 0, &charEntries);
    }
    // Every paragraph has a stretch of its own, as in a file a program wrote: the ones the test gives, and the rest (cut after each mark) with no changes.
    {
        QVector<ParaRun> given = paras;
        std::sort(given.begin(), given.end(), [](const ParaRun &a2, const ParaRun &b2) { return a2.from < b2.from; });
        QVector<ParaRun> every;
        int at = 0;
        auto fill = [&](int to) {
            int start = at;
            for (int cp = at; cp < to; ++cp) {
                const QChar ch = all[cp];
                if (ch == QLatin1Char('\r') || ch == QLatin1Char('\a')) {
                    every.push_back({start, cp + 1, 0, QByteArray()});
                    start = cp + 1;
                }
            }
            if (start < to) every.push_back({start, to, 0, QByteArray()});
        };
        for (const ParaRun &r : given) {
            if (r.from > at) fill(r.from);
            every.push_back(r);
            at = std::max(at, r.to);
        }
        fill(int(all.size()));
        for (const ParaRun &r : every) cut(r.from, r.to, r.sprms, r.istd, &paraEntries);
    }
    // (Whole paragraphs, so the pieces' ends are not needed to cut them.)
    while (wd.size() % 512) wd.append('\0');
    const QByteArray chpBte = writeBin(wd, stretches(charEntries), false);
    const QByteArray papBte = writeBin(wd, stretches(paraEntries), true);

    // The sections.
    QVector<SectionSpec> secs = sections;
    if (secs.isEmpty()) secs.push_back({ccpText + 1, QByteArray()});
    QVector<quint32> sepxAt;
    for (const SectionSpec &s : secs) {
        if (s.sprms.isEmpty()) {
            sepxAt.push_back(0xFFFFFFFFu);
            continue;
        }
        padEven(wd);
        sepxAt.push_back(quint32(wd.size()));
        add16(wd, int(s.sprms.size()));
        wd.append(s.sprms);
    }

    // The pictures of the shapes: their records in this stream (the picture store points here).
    QVector<quint32> blipAt(shapes.size(), 0);
    for (int i = 0; i < shapes.size(); ++i) {
        if (shapes[i].png.isEmpty()) continue;
        padEven(wd);
        blipAt[i] = quint32(wd.size());
        QByteArray body(16, 'u');
        body.append('\xFF');
        body.append(shapes[i].png);
        add16(wd, 0x06E0 << 4);
        add16(wd, 0xF01E);
        add32(wd, quint32(body.size()));
        wd.append(body);
    }

    // The table stream.
    QByteArray tbl(tableReserve, '\0');
    struct Slot {
        quint32 fc = 0, lcb = 0;
    };
    QVector<Slot> slots(113);
    auto put = [&](int slot, const QByteArray &bytes) {
        padEven(tbl);
        slots[slot] = {quint32(tbl.size()), quint32(bytes.size())};
        tbl.append(bytes);
    };
    // The styles.
    {
        QVector<StyleSpec> st = styles;
        if (st.isEmpty()) {
            StyleSpec normal;
            normal.name = QStringLiteral("Normal");
            normal.sti = 0;
            st.push_back(normal);
        }
        QByteArray s;
        add16(s, 20);   // cbStshi
        add16(s, int(st.size()));
        add16(s, 10);
        add16(s, 1);
        add16(s, 0);
        add16(s, 0);
        add16(s, 0);
        add16(s, 0);
        add16(s, 0);
        add16(s, 0);
        add16(s, 0);   // ftcBi
        for (const StyleSpec &sp : st) {
            QByteArray std;
            add16(std, sp.sti & 0xFFF);
            add16(std, sp.sgc | (sp.base << 4));
            add16(std, (sp.sgc == 1 ? 2 : 1) | (sp.next << 4));
            add16(std, 0);
            add16(std, 0);
            add16(std, int(sp.name.size()));
            std.append(utf16(sp.name));
            add16(std, 0);
            auto upx = [&](const QByteArray &u) {
                add16(std, int(u.size()));
                std.append(u);
                if (u.size() & 1) std.append('\0');
            };
            if (sp.sgc == 1) {
                QByteArray papx;
                add16(papx, 0);
                papx.append(sp.paragraph);
                upx(papx);
                upx(sp.character);
            } else {
                upx(sp.character);
            }
            add16(s, int(std.size()));
            s.append(std);
        }
        put(1, s);
    }
    // The fonts.
    {
        QByteArray f;
        add16(f, int(fonts.size()));
        add16(f, 0);
        for (const QString &name : fonts) {
            const QByteArray n = utf16(name);
            const int total = 40 + int(n.size()) + 2;
            add8(f, total - 1);
            add8(f, 0x12);   // variable pitch, roman
            add16(f, 400);
            add8(f, 0);
            add8(f, 0);
            f.append(QByteArray(34, '\0'));
            f.append(n);
            add16(f, 0);
        }
        put(15, f);
    }
    // The people who made the tracked changes.
    if (!revisionAuthors.isEmpty()) {
        QByteArray a;
        add16(a, 0xFFFF);
        add16(a, int(revisionAuthors.size()));
        add16(a, 0);
        for (const QString &name : revisionAuthors) {
            add16(a, int(name.size()));
            a.append(utf16(name));
        }
        put(51, a);
    }
    // The pieces.
    {
        QByteArray plc;
        for (int again = 0; again < copies; ++again)
            for (const Piece &p : pieces) add32(plc, quint32(p.cp0 + again * int(all.size())));
        add32(plc, quint32(all.size()) * quint32(copies));
        for (int again = 0; again < copies; ++again)
            for (const Piece &p : pieces) {
                add16(plc, 0);
                add32(plc, p.unicode ? p.fc : (p.fc * 2) | 0x40000000u);
                add16(plc, p.prm);
            }
        QByteArray clx;
        add8(clx, 2);
        add32(clx, quint32(plc.size()));
        clx.append(plc);
        put(33, clx);
    }
    put(12, chpBte);
    put(13, papBte);
    // The sections' table.
    {
        QByteArray plc;
        add32(plc, 0);
        for (const SectionSpec &s : secs) add32(plc, quint32(s.end));
        for (int i = 0; i < secs.size(); ++i) {
            add16(plc, 0);
            add32(plc, sepxAt[i]);
            add16(plc, 0);
            add32(plc, 0xFFFFFFFFu);
        }
        put(6, plc);
    }
    if (!headerStories.isEmpty()) {
        QByteArray plc;
        for (int s : storyStart) add32(plc, quint32(s));
        add32(plc, quint32(ccpHdd));
        put(11, plc);
    }
    if (!footnotes.isEmpty()) {
        QByteArray refs, txt;
        for (int cp : footnoteRefs) add32(refs, quint32(cp));
        add32(refs, quint32(ccpText + 1));
        for (int i = 0; i < footnoteRefs.size(); ++i) add16(refs, 1);
        for (int s : (footnoteStarts.isEmpty() ? noteStart : footnoteStarts)) add32(txt, quint32(s));
        add32(txt, quint32(ccpFtn));
        add32(txt, quint32(ccpFtn + 1));
        put(2, refs);
        put(3, txt);
    }
    // The comments.
    if (!comments.isEmpty()) {
        QByteArray refs, txt, owners, extra;
        QStringList names;
        for (const CommentSpec &cm : comments) add32(refs, quint32(cm.ref));
        add32(refs, quint32(ccpText));
        for (int i = 0; i < comments.size(); ++i) {
            const CommentSpec &cm = comments[i];
            if (!names.contains(cm.author)) names << cm.author;
            QByteArray r;
            add16(r, int(std::min<qsizetype>(cm.initials.size(), 9)));
            r.append(utf16(cm.initials.left(9)));
            r.append(QByteArray(20 - r.size(), '\0'));
            add16(r, cm.ownerIndex >= 0 ? cm.ownerIndex : int(names.indexOf(cm.author)));
            add16(r, 0);
            add16(r, 0);
            add32(r, cm.from >= 0 ? quint32(i + 1) : 0xFFFFFFFFu);
            refs.append(r);
            add32(txt, quint32(commentStart[i]));
            quint32 d = cm.date;
            add32(extra, d);
            add16(extra, 0);
            add32(extra, 0);
            add32(extra, 0);
            add32(extra, 0);
        }
        add32(txt, quint32(ccpAtn));
        add32(txt, quint32(ccpAtn + 1));
        for (const QString &n : names) {
            add16(owners, int(n.size()));
            owners.append(utf16(n));
        }
        put(4, refs);
        put(5, txt);
        put(36, owners);
        bool anyDate = false;
        for (const CommentSpec &cm : comments) anyDate = anyDate || cm.date;
        if (anyDate) put(112, extra);
        // The ranges: a bookmark of the class for comments, with the tag the comment names.
        QByteArray tags, starts, ends;
        QVector<int> covering;
        for (int i = 0; i < comments.size(); ++i)
            if (comments[i].from >= 0) covering.push_back(i);
        if (!covering.isEmpty()) {
            add16(tags, 0xFFFF);
            add16(tags, int(covering.size()));
            add16(tags, 10);
            for (int i : covering) {
                add16(tags, 0);
                add16(tags, 0x0100);
                add32(tags, quint32(i + 1));
                add32(tags, 0xFFFFFFFFu);
            }
            for (int i : covering) add32(starts, quint32(comments[i].from));
            add32(starts, quint32(ccpText));
            for (int k = 0; k < covering.size(); ++k) {
                add16(starts, comments[covering[k]].endIndex >= 0 ? comments[covering[k]].endIndex : k);
                add16(starts, 0);
            }
            for (int i : covering) add32(ends, quint32(comments[i].to));
            add32(ends, quint32(ccpText));
            put(37, tags);
            put(42, starts);
            put(43, ends);
        }
    }
    // The lists.
    if (!lists.isEmpty()) {
        QByteArray lst, lfo, levels;
        add16(lst, int(lists.size()));
        for (int i = 0; i < lists.size(); ++i) {
            const QVector<ListLevelSpec> &lv = lists[i];
            const bool simple = lv.size() == 1;
            add32(lst, quint32(1000 + i));
            add32(lst, 0);
            for (int k = 0; k < 9; ++k) add16(lst, 0x0FFF);
            add8(lst, simple ? 1 : 0);
            add8(lst, 0);
            for (int k = 0; k < (simple ? 1 : 9); ++k) {
                const ListLevelSpec &l = lv[std::min<int>(k, int(lv.size()) - 1)];
                QByteArray lvl;
                add32(lvl, quint32(l.start));
                add8(lvl, l.format);
                add8(lvl, 0);
                lvl.append(QByteArray(9, '\0'));
                add8(lvl, 0);
                add32(lvl, 0);
                add32(lvl, 0);
                add8(lvl, int(l.character.size()));
                add8(lvl, int(l.paragraph.size()));
                add8(lvl, 0);
                add8(lvl, 0);
                lvl.append(l.paragraph);
                lvl.append(l.character);
                add16(lvl, int(l.text.size()));
                lvl.append(utf16(l.text));
                levels.append(lvl);
            }
        }
        lst.append(levels);
        add32(lfo, quint32(lists.size()));
        for (int i = 0; i < lists.size(); ++i) {
            add32(lfo, quint32(1000 + i));
            add32(lfo, 0);
            add32(lfo, 0);
            add32(lfo, 0);
        }
        for (int i = 0; i < lists.size(); ++i) add32(lfo, 0xFFFFFFFFu);
        put(73, lst);
        slots[73].lcb = quint32(2 + 28 * lists.size());   // (the levels are appended to the table of lists: its size does not hold them)
        put(74, lfo);
    }
    // The shapes: the places (a table for the main text, and one for the headers'), the text boxes, and the drawing itself.
    if (!shapes.isEmpty()) {
        for (int pass = 0; pass < 2; ++pass) {
            const bool hdr = pass == 1;
            // (The places: the shape, and the character that anchors it; a shape may have more than one.)
            QVector<QPair<int, int>> order;
            for (int i = 0; i < shapes.size(); ++i) {
                if (shapes[i].header != hdr || shapes[i].group >= 0) continue;
                order.push_back({shapes[i].cp, i});
                for (int more : shapes[i].moreAnchors) order.push_back({more, i});
            }
            std::stable_sort(order.begin(), order.end(), [](const QPair<int, int> &a2, const QPair<int, int> &b2) { return a2.first < b2.first; });
            if (order.isEmpty()) continue;
            QByteArray plc;
            for (const auto &o : order) add32(plc, quint32(o.first));
            add32(plc, quint32(hdr ? ccpHdd : ccpText) + 1);
            for (const auto &o : order) {
                const ShapeSpec &sp = shapes[o.second];
                add32(plc, quint32(sp.spid));
                add32(plc, quint32(sp.x0));
                add32(plc, quint32(sp.y0));
                add32(plc, quint32(sp.x1));
                add32(plc, quint32(sp.y1));
                add16(plc, (hdr ? 1 : 0) | (sp.bx << 1) | (sp.by << 3) | (sp.wr << 5) | (sp.below ? 0x4000 : 0));
                add32(plc, 0);
            }
            put(hdr ? 41 : 40, plc);
            // The boxes' text: a story each, and a last record that ends the table.
            const QVector<int> &boxes = hdr ? headerBoxShape : boxShape;
            const QVector<int> &starts = hdr ? headerBoxStart : boxStart;
            if (!boxes.isEmpty()) {
                QByteArray bp;
                for (int s2 : starts) add32(bp, quint32(s2));
                add32(bp, quint32(hdr ? ccpHdrTxbx : ccpTxbx));
                add32(bp, quint32(hdr ? ccpHdrTxbx : ccpTxbx) + 1);
                for (int idx : boxes) {
                    add32(bp, 0);
                    add32(bp, 0);
                    add16(bp, 0);
                    add32(bp, 0xFFFFFFFFu);
                    add32(bp, quint32(shapes[idx].spid));
                    add32(bp, 0);
                }
                bp.append(QByteArray(22, '\0'));
                put(hdr ? 58 : 56, bp);
            }
        }
        // The drawing: the picture store, then each drawing (the main one's, then the headers') with its shapes.
        auto record = [](int version, int instance, int type, const QByteArray &data) {
            QByteArray r;
            add16(r, (instance << 4) | version);
            add16(r, type);
            add32(r, quint32(data.size()));
            r.append(data);
            return r;
        };
        QByteArray store;
        QVector<int> storeIndex(shapes.size(), 0);
        int stored = 0;
        for (int i = 0; i < shapes.size(); ++i) {
            if (shapes[i].png.isEmpty()) continue;
            QByteArray f;
            add8(f, 6);
            add8(f, 6);
            f.append(QByteArray(16, 'u'));
            add16(f, 0xFF);
            add32(f, quint32(shapes[i].png.size() + 25));
            add32(f, 1);
            add32(f, blipAt[i]);
            add8(f, 0);
            add8(f, 0);
            add8(f, 0);
            add8(f, 0);
            store.append(record(2, 6, 0xF007, f));
            storeIndex[i] = ++stored;
        }
        QByteArray group;
        group.append(record(0, 0, 0xF006, QByteArray(16, '\0')));
        if (stored) group.append(record(0xF, stored, 0xF001, store));
        QByteArray dgg = record(0xF, 0, 0xF000, group);
        auto shapeRecord = [&](const ShapeSpec &sp, int index, bool isGroup) {
            QByteArray c;
            if (isGroup) {
                QByteArray g;
                add32(g, quint32(sp.groupLeft));
                add32(g, quint32(sp.groupTop));
                add32(g, quint32(sp.groupRight));
                add32(g, quint32(sp.groupBottom));
                c.append(record(1, 0, 0xF009, g));
            }
            QByteArray fsp;
            add32(fsp, quint32(sp.spid));
            add32(fsp, quint32(0x0A00 | (isGroup ? 1 : 0) | (sp.group >= 0 ? 2 : 0)));
            c.append(record(2, isGroup ? 0 : sp.type, 0xF00A, fsp));
            QByteArray props;
            int count = 0;
            auto prop = [&](int id, quint32 value) {
                add16(props, id);
                add32(props, value);
                ++count;
            };
            if (storeIndex[index]) prop(0x104 | 0x4000, quint32(storeIndex[index]));
            if (sp.fill >= 0) {
                prop(0x181, quint32(sp.fill));
                prop(0x1BF, 0x110010);
            } else {
                prop(0x1BF, 0x100000);
            }
            if (sp.line >= 0) {
                prop(0x1C0, quint32(sp.line));
                prop(0x1FF, 0x80008);
            } else {
                prop(0x1FF, 0x80000);
            }
            if (!isGroup) c.append(record(3, count, 0xF00B, props));
            if (sp.group >= 0) {
                QByteArray ca;
                add32(ca, quint32(sp.memberLeft));
                add32(ca, quint32(sp.memberTop));
                add32(ca, quint32(sp.memberRight));
                add32(ca, quint32(sp.memberBottom));
                c.append(record(0, 0, 0xF00F, ca));
            }
            return record(0xF, 0, 0xF004, c);
        };
        for (int pass = 0; pass < 2; ++pass) {
            const bool hdr = pass == 1;
            QByteArray spgr;
            ShapeSpec patriarch;
            patriarch.spid = hdr ? 3072 : 2048;
            spgr.append(shapeRecord(patriarch, 0, true));
            // A group's members follow it, inside a container of their own.
            for (int i = 0; i < shapes.size(); ++i) {
                if (shapes[i].header != hdr || shapes[i].group >= 0) continue;
                QVector<int> members;
                for (int j = 0; j < shapes.size(); ++j)
                    if (shapes[j].group == i) members.push_back(j);
                if (members.isEmpty()) {
                    spgr.append(shapeRecord(shapes[i], i, false));
                } else {
                    QByteArray inner;
                    inner.append(shapeRecord(shapes[i], i, true));
                    for (int j : members) inner.append(shapeRecord(shapes[j], j, false));
                    spgr.append(record(0xF, 0, 0xF003, inner));
                }
            }
            QByteArray dg;
            QByteArray fdg;
            add32(fdg, 1);
            add32(fdg, 1);
            dg.append(record(0, pass + 1, 0xF008, fdg));
            dg.append(record(0xF, 0, 0xF003, spgr));
            dgg.append(char(pass));
            dgg.append(record(0xF, 0, 0xF002, dg));
        }
        put(50, dgg);
    }
    // The bookmarks: their names, where each begins (in the order of that), and where each ends.
    if (!bookmarks.isEmpty()) {
        QVector<BookmarkSpec> order = bookmarks;
        std::sort(order.begin(), order.end(), [](const BookmarkSpec &x, const BookmarkSpec &y) { return x.from < y.from; });
        QByteArray names, starts, ends;
        add16(names, 0xFFFF);
        add16(names, int(order.size()));
        add16(names, 0);
        for (const BookmarkSpec &b : order) {
            add16(names, int(b.name.size()));
            names.append(utf16(b.name));
        }
        for (const BookmarkSpec &b : order) add32(starts, quint32(b.from));
        add32(starts, quint32(ccpText));
        for (int i = 0; i < order.size(); ++i) {
            add16(starts, i);
            add16(starts, 0);
        }
        for (const BookmarkSpec &b : order) add32(ends, quint32(b.to));
        add32(ends, quint32(ccpText));
        put(21, names);
        put(22, starts);
        put(23, ends);
    }
    // The document's own settings.
    {
        QByteArray dop(compatByte84 >= 0 ? 88 : 84, '\0');
        if (compatByte84 >= 0) dop[84] = char(compatByte84);
        dop[0] = char(evenAndOddHeaders ? 1 : 0);
        put16(dop, 10, defaultTab);
        put16(dop, 54, (footnoteFormat << 2) | (2 << 6) | 0x1000);
        put(31, dop);
    }

    // The header.
    put16(wd, 0, oldVersion ? 0xA5DC : 0xA5EC);   // (the header of the 1993 and 1995 programs has its own mark)
    put16(wd, 2, oldVersion ? 0x65 : 0xC1);
    put16(wd, 10, 0x0200 | (password ? 0x0100 : 0));
    put16(wd, 32, 14);
    put16(wd, 62, 22);
    put32(wd, 64, quint32(wd.size()));
    put32(wd, 76, quint32(ccpText));
    put32(wd, 80, quint32(ccpFtn));
    put32(wd, 84, quint32(ccpHdd));
    put32(wd, 92, quint32(ccpAtn));
    put32(wd, 100, quint32(ccpTxbx));
    put32(wd, 104, quint32(ccpHdrTxbx));
    put16(wd, 152, 113);
    for (int i = 0; i < 113; ++i) {
        put32(wd, 154 + 8 * i, slots[i].fc);
        put32(wd, 158 + 8 * i, slots[i].lcb);
    }

    jp::cfb::File f;
    jp::cfb::Entry root;
    root.type = jp::cfb::Entry::Root;
    root.name = QStringLiteral("Root Entry");
    f.entries.push_back(root);
    auto stream = [&](const QString &name, const QByteArray &bytes) {
        jp::cfb::Entry e;
        e.type = jp::cfb::Entry::Stream;
        e.name = name;
        e.data = bytes;
        f.entries[0].children.push_back(int(f.entries.size()));
        f.entries.push_back(e);
    };
    stream(QStringLiteral("WordDocument"), wd);
    stream(QStringLiteral("1Table"), tbl);
    if (!data.isEmpty()) stream(QStringLiteral("Data"), data);
    if (!summaryInformation.isEmpty()) stream(QStringLiteral("\u0005SummaryInformation"), summaryInformation);
    if (!documentSummaryInformation.isEmpty()) stream(QStringLiteral("\u0005DocumentSummaryInformation"), documentSummaryInformation);
    return jp::cfb::write(f);
}

QByteArray summaryFormat() { return QByteArray::fromHex("E0859FF2F94F6810AB9108002B27B3D9"); }
QByteArray documentSummaryFormat() { return QByteArray::fromHex("02D5CDD59C2E1B10939708002B2CF9AE"); }

qint64 fileTime(int year, int month, int day, int hour, int minute)
{
    const QDateTime t(QDate(year, month, day), QTime(hour, minute), QTimeZone::UTC);
    return (t.toMSecsSinceEpoch() + 11644473600000LL) * 10000;
}

QByteArray propertySetStream(const QByteArray &format, int codePage, const QVector<PropSpec> &props, bool twoSets)
{
    auto le = [](quint64 v, int bytes) {
        QByteArray b(bytes, 0);
        for (int i = 0; i < bytes; ++i) b[i] = char((v >> (8 * i)) & 0xFF);
        return b;
    };
    auto padded = [](QByteArray b) {
        while (b.size() % 4) b.append('\0');
        return b;
    };
    // Each property's value: its type, two bytes of padding, and the value.
    QVector<PropSpec> all = props;
    PropSpec cp;
    cp.id = 1;
    cp.type = 2;
    cp.number = codePage;
    all.prepend(cp);
    QVector<QByteArray> values;
    for (const PropSpec &p : std::as_const(all)) {
        QByteArray v = le(p.type, 2) + le(0, 2);
        switch (p.type) {
        case 2: v += padded(le(quint16(p.number), 2)); break;
        case 3: v += le(quint32(p.number), 4); break;
        case 0x1E: v += le(quint32(p.raw.size() + 1), 4) + padded(p.raw + QByteArray(1, '\0')); break;
        case 0x1F: v += le(quint32(p.text.size() + 1), 4) + padded(utf16(p.text) + QByteArray(2, '\0')); break;
        case 0x40: v += le(quint64(p.number), 8); break;
        default: break;
        }
        values.push_back(v);
    }
    QByteArray head, body;
    const int table = 8 + 8 * int(all.size());
    for (int i = 0; i < all.size(); ++i) {
        head += le(all[i].id, 4) + le(quint32(table + body.size()), 4);
        body += values[i];
    }
    const QByteArray set = le(quint32(table + body.size()), 4) + le(quint32(all.size()), 4) + head + body;
    const int sets = twoSets ? 2 : 1;
    QByteArray out = le(0xFFFE, 2) + le(0, 2) + le(0x00020006, 4) + QByteArray(16, '\0') + le(quint32(sets), 4);
    const int first = 28 + 20 * sets;
    out += format + le(quint32(first), 4);
    QByteArray second;
    if (twoSets) {
        const QByteArray user = QByteArray::fromHex("05D5CDD59C2E1B10939708002B2CF9AE");
        out += user + le(quint32(first + set.size()), 4);
        second = le(24, 4) + le(1, 4) + le(1, 4) + le(16, 4) + le(2, 2) + le(0, 2) + le(quint16(codePage), 2) + le(0, 2);
    }
    return out + set + second;
}

namespace {

QByteArray le32(quint32 v)
{
    QByteArray b(4, 0);
    for (int i = 0; i < 4; ++i) b[i] = char((v >> (8 * i)) & 0xFF);
    return b;
}

QByteArray utf16le(const QString &s)
{
    QByteArray b;
    for (QChar c : s) {
        b.append(char(c.unicode() & 0xFF));
        b.append(char(c.unicode() >> 8));
    }
    return b;
}

// Enciphers `stream` from byte `from` in blocks of 512, the key of block n made by `keyFor(n)`.
template <class F> void encipher(QByteArray *stream, int from, F keyFor)
{
    for (int block = from / 512; block * 512 < stream->size(); ++block) {
        QByteArray key = keyFor(block);
        jp::crypto::Rc4 rc4(key);
        QByteArray ks = rc4.apply(QByteArray(512, 0));
        for (int p = std::max(block * 512, from); p < std::min<int>(int(stream->size()), block * 512 + 512); ++p) (*stream)[p] = char((*stream)[p] ^ ks[p - block * 512]);
    }
}

} // namespace

QByteArray DocWriter::buildProtected(const QString &password, int form) const
{
    const QByteArray salt = QByteArray::fromHex("0f1e2d3c4b5a69788796a5b4c3d2e1f0"), verifier = QByteArray::fromHex("a1b2c3d4e5f60718293a4b5c6d7e8f90");
    QByteArray pw = utf16le(password);
    QByteArray description;
    std::function<QByteArray(int)> keyFor;
    if (form == 0) {
        const QByteArray h0 = QCryptographicHash::hash(pw, QCryptographicHash::Md5);
        QByteArray rep;
        for (int i = 0; i < 16; ++i) rep += h0.left(5) + salt;
        const QByteArray base = QCryptographicHash::hash(rep, QCryptographicHash::Md5).left(5);
        keyFor = [base](int block) { return QCryptographicHash::hash(base + le32(quint32(block)), QCryptographicHash::Md5); };
        QByteArray key0 = keyFor(0);
        jp::crypto::Rc4 rc4(key0);
        description = QByteArray::fromHex("01000100") + salt + rc4.apply(verifier + QCryptographicHash::hash(verifier, QCryptographicHash::Md5));
    } else if (form == 1) {
        const QByteArray base = QCryptographicHash::hash(salt + pw, QCryptographicHash::Sha1);
        keyFor = [base](int block) { return QCryptographicHash::hash(base + le32(quint32(block)), QCryptographicHash::Sha1).left(16); };
        QByteArray header = le32(0x04) + le32(0) + le32(0x6801) + le32(0x8004) + le32(128) + le32(1) + le32(0) + le32(0) + utf16le(QStringLiteral("Provider")) + QByteArray(2, 0);
        QByteArray key0 = keyFor(0);
        jp::crypto::Rc4 rc4(key0);
        const QByteArray enc = rc4.apply(verifier + QCryptographicHash::hash(verifier, QCryptographicHash::Sha1));
        description = QByteArray::fromHex("04000200") + le32(0x04) + le32(quint32(header.size())) + header + le32(16) + salt + enc.left(16) + le32(20) + enc.mid(16, 20);
    }
    DocWriter w = *this;
    w.password = true;
    w.tableReserve = int(description.size());
    jp::cfb::File f;
    if (!jp::cfb::read(w.build(), &f)) return QByteArray();
    QByteArray wd = f.stream(QStringLiteral("WordDocument")), tbl = f.stream(QStringLiteral("1Table")), dat = f.stream(QStringLiteral("Data"));
    if (form == 2) {
        // the obfuscation: the key and the verifier of the rules, kept in the header, and the bytes XORed with the table made from the password
        QByteArray ansi;
        for (const QChar c : password.left(15)) {
            const QByteArray one = c.unicode() == 0x20AC ? QByteArray(1, char(0x80)) : c.unicode() == 0x2019 ? QByteArray(1, char(0x92)) : QByteArray(1, char(c.unicode()));
            ansi += one;
        }
        static const quint16 initial[15] = {0xE1F0, 0x1D0F, 0xCC9C, 0x84C0, 0x110C, 0x0E10, 0xF1CE, 0x313E, 0x1872, 0xE139, 0xD40F, 0x84F9, 0x280C, 0xA96A, 0x4EC3};
        static const quint8 pad[15] = {0xBB, 0xFF, 0xFF, 0xBA, 0xFF, 0xFF, 0xB9, 0x80, 0x00, 0xBE, 0x0F, 0x00, 0xBF, 0x0F, 0x00};
        // (the matrix of the key: row r holds the 7 values of the r-th place, each the step of a 16-bit check value from the one before)
        auto step = [](quint16 x) { return quint16((x << 1) ^ ((x & 0x8000) ? 0x1021 : 0)); };
        quint16 matrix[105];
        for (int row = 0; row < 15; ++row) {
            quint16 x = 1;
            for (int i = 0; i < 16 + 8 * (14 - row); ++i) x = step(x);
            for (int j = 0; j < 7; ++j) {
                matrix[row * 7 + j] = x;
                x = step(x);
            }
        }
        quint16 key = initial[ansi.size() - 1];
        int element = 104;
        for (int i = int(ansi.size()) - 1; i >= 0; --i) {
            quint8 c = quint8(ansi[i]);
            for (int j = 0; j < 7; ++j, --element, c = quint8(c << 1))
                if (c & 0x40) key ^= matrix[element];
        }
        quint16 verify = 0;
        QByteArray all;
        all.append(char(ansi.size()));
        all.append(ansi);
        for (int i = int(all.size()) - 1; i >= 0; --i) verify = quint16((((verify & 0x4000) ? 1 : 0) | ((verify * 2) & 0x7FFF)) ^ quint8(all[i]));
        verify ^= 0xCE4B;
        quint8 table[16];
        for (int p = 0; p < 16; ++p) {
            const quint8 k = (p & 1) ? quint8(key >> 8) : quint8(key & 0xFF);
            const quint8 src = p < ansi.size() ? quint8(ansi[p]) : pad[p - ansi.size()];
            const quint8 x = quint8(src ^ k);
            table[p] = quint8((x >> 1) | (x << 7));
        }
        auto obfuscate = [&](QByteArray *s, int from) {
            for (int q = from; q < s->size(); ++q) {
                const quint8 b = quint8((*s)[q]);
                const quint8 r = quint8((b >> 3) | (b << 5));   // (the inverse of the rotation the reader ends with)
                (*s)[q] = char(r ^ table[q % 16]);
            }
        };
        obfuscate(&wd, 68);
        obfuscate(&tbl, 0);
        obfuscate(&dat, 0);
        const quint16 flags = quint16(quint8(wd[10]) | quint8(wd[11]) << 8) | 0x8000;
        wd[10] = char(flags & 0xFF);
        wd[11] = char(flags >> 8);
        const quint32 lKey = quint32(key) | quint32(verify) << 16;
        for (int i = 0; i < 4; ++i) wd[14 + i] = char((lKey >> (8 * i)) & 0xFF);
    } else {
        for (int i = 0; i < 4; ++i) wd[14 + i] = char((quint32(description.size()) >> (8 * i)) & 0xFF);
        encipher(&wd, 68, keyFor);
        encipher(&tbl, int(description.size()), keyFor);
        if (!dat.isEmpty()) encipher(&dat, 0, keyFor);
        for (int i = 0; i < description.size(); ++i) tbl[i] = description[i];
    }
    f.setStream(QStringLiteral("WordDocument"), wd);
    f.setStream(QStringLiteral("1Table"), tbl);
    if (!dat.isEmpty()) f.setStream(QStringLiteral("Data"), dat);
    return jp::cfb::write(f);
}


} // namespace dochelp
