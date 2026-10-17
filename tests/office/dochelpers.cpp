// The writer of old binary word processing files that the tests use: see dochelpers.h.

#include "dochelpers.h"

#include "io/cfb.h"

#include <QBuffer>
#include <QImage>
#include <algorithm>

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

int DocWriter::addPicture(const QByteArray &png, int widthTwips, int heightTwips)
{
    const int at = int(data.size());
    // The drawing records: an empty shape container, then the picture's file block holding its record.
    QByteArray body(16, 'u');
    body.append('\xFF');
    body.append(png);
    QByteArray blipRecord;
    add16(blipRecord, 0x06E0 << 4);   // (instance in the high twelve bits)
    add16(blipRecord, 0xF01E);
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
    const QString all = text + footnoteText + headerText;
    const int ccpText = int(text.size()), ccpFtn = int(footnoteText.size()), ccpHdd = int(headerText.size());

    QByteArray wd(1024, '\0');
    // The text, in one or two pieces.
    struct Piece {
        int cp0, cp1;
        quint32 fc;
        bool unicode;
    };
    QVector<Piece> pieces;
    if (splitAt > 0 && splitAt < all.size()) {
        const int firstLen = splitAt;
        pieces.push_back({0, firstLen, quint32(wd.size()), false});
        wd.append(all.left(firstLen).toLatin1());
        padEven(wd);
        pieces.push_back({firstLen, int(all.size()), quint32(wd.size()), true});
        wd.append(utf16(all.mid(firstLen)));
    } else {
        pieces.push_back({0, int(all.size()), quint32(wd.size()), unicode});
        wd.append(unicode ? utf16(all) : all.toLatin1());
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
    for (const ParaRun &r : paras) cut(r.from, r.to, r.sprms, r.istd, &paraEntries);
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

    // The table stream.
    QByteArray tbl;
    struct Slot {
        quint32 fc = 0, lcb = 0;
    };
    QVector<Slot> slots(93);
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
    // The pieces.
    {
        QByteArray plc;
        for (const Piece &p : pieces) add32(plc, quint32(p.cp0));
        add32(plc, quint32(all.size()));
        for (const Piece &p : pieces) {
            add16(plc, 0);
            add32(plc, p.unicode ? p.fc : (p.fc * 2) | 0x40000000u);
            add16(plc, 0);
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
        for (int s : noteStart) add32(txt, quint32(s));
        add32(txt, quint32(ccpFtn));
        add32(txt, quint32(ccpFtn + 1));
        put(2, refs);
        put(3, txt);
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
        put(74, lfo);
    }
    // The document's own settings.
    {
        QByteArray dop(84, '\0');
        dop[0] = char(evenAndOddHeaders ? 1 : 0);
        put16(dop, 10, defaultTab);
        put(31, dop);
    }

    // The header.
    put16(wd, 0, 0xA5EC);
    put16(wd, 2, oldVersion ? 0x65 : 0xC1);
    put16(wd, 10, 0x0200 | (password ? 0x0100 : 0));
    put16(wd, 32, 14);
    put16(wd, 62, 22);
    put32(wd, 64, quint32(wd.size()));
    put32(wd, 76, quint32(ccpText));
    put32(wd, 80, quint32(ccpFtn));
    put32(wd, 84, quint32(ccpHdd));
    put16(wd, 152, 93);
    for (int i = 0; i < 93; ++i) {
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
    return jp::cfb::write(f);
}

} // namespace dochelp
