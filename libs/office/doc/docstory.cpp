// The text of a story (the main text, a note, a header or footer) of an old binary document, written as the paragraphs and tables of a body.

#include "doc/docconvert.h"

#include <QCoreApplication>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

namespace jp::office::doc {

namespace {

// One paragraph's worth of runs, built up as the text is walked.
class RunBuilder {
public:
    explicit RunBuilder(Converter &c) : c(c) {}
    QByteArray paragraph;            // the runs so far

    // The properties of the characters that follow; a run ends where they change.
    void setRun(const QByteArray &rpr, const Revision &insertion, const Revision &deletion)
    {
        if (rpr == m_rpr && insertion == m_insertion && deletion == m_deletion) return;
        flushRun();
        m_rpr = rpr;
        m_insertion = insertion;
        m_deletion = deletion;
    }
    // Text in the run: as text, as the instructions of a field, or (in a deleted run) as deleted text.
    void text(const QString &s, bool instruction)
    {
        const int mode = instruction ? 1 : m_deletion.made ? 2 : 0;
        if (mode != m_textMode) flushText();
        m_textMode = mode;
        m_text += s;
        if (m_text.size() >= kMaxRunChars) flushRun();   // (a run of any length is made in pieces, and counted as it goes)
    }
    void element(const QByteArray &xml)
    {
        flushText();
        m_body += xml;
        m_has = true;
        if (m_body.size() >= kMaxRunChars) flushRun();
    }
    // An element between the runs (a bookmark's start or end).
    void between(const QByteArray &xml)
    {
        flushRun();
        paragraph += xml;
        c.spend(xml);
    }
    // An element of a run of its own (a field's marks, as the .docx files have them).
    void alone(const QByteArray &xml)
    {
        flushRun();
        element(xml);
        flushRun();
    }
    void flushRun()
    {
        flushText();
        if (!m_has) return;
        QByteArray r = "<w:r>";
        if (!m_rpr.isEmpty()) r += "<w:rPr>" + m_rpr + "</w:rPr>";
        r += m_body + "</w:r>";
        // Text that was inserted and then deleted has both marks, the insertion around the deletion.
        const int insertionId = m_insertion.made ? ++c.revisionId : 0, deletionId = m_deletion.made ? ++c.revisionId : 0;
        if (m_deletion.made) r = "<w:del" + revisionAttributes(deletionId, c.revisionAuthor(m_deletion.author), m_deletion.date) + ">" + r + "</w:del>";
        if (m_insertion.made) r = "<w:ins" + revisionAttributes(insertionId, c.revisionAuthor(m_insertion.author), m_insertion.date) + ">" + r + "</w:ins>";
        paragraph += r;
        c.spend(r);
        m_body.clear();
        m_has = false;
    }

private:
    void flushText()
    {
        if (m_text.isEmpty()) return;
        const char *tag = m_textMode == 1 ? "w:instrText" : m_textMode == 2 ? "w:delText" : "w:t";
        m_body += QByteArray("<") + tag + " xml:space=\"preserve\">" + xmlEscape(m_text) + "</" + tag + ">";
        m_text.clear();
        m_has = true;
    }
    Converter &c;
    QByteArray m_rpr, m_body;
    Revision m_insertion, m_deletion;    // the tracked changes the run is under
    QString m_text;
    int m_textMode = 0;
    bool m_has = false;
};

struct RowOut {
    Tap tap;
    QVector<QByteArray> cells;
};
struct TableOut {
    QVector<RowOut> rows;
    QVector<QByteArray> cells;       // the cells of the row being read
    QByteArray cell;                 // the cell being read
};

class StoryWriter {
public:
    StoryWriter(Converter &c, Converter::Kind kind, const QString &part, qint64 cp0, qint64 cp1) : c(c), kind(kind), part(part), cp0(cp0), cp1(cp1), T(c.pieces.text()) {}
    QByteArray run();

private:
    struct Field {
        bool real = true;
        bool result = false;         // past the separator
    };
    bool isEnd(qint64 cp) const;
    void paragraph(qint64 first, qint64 last, bool terminated);
    void content(qint64 from, qint64 to, int istd, RunBuilder &rb);
    void characters(qint64 from, qint64 to, const Chp &chp, const QByteArray &rpr, RunBuilder &rb);
    QByteArray rprOf(const Chp &chp, int istd) const;
    void flushParagraph();
    // ---- tables
    void adjustTables(int depth);
    void closeTable();
    void finishRow(const Pap &pap);
    QByteArray tableXml(TableOut &t);
    int availableWidth() const;
    QByteArray &sink() { return tables.isEmpty() ? out : tables.last().cell; }

    Converter &c;
    Converter::Kind kind;
    QString part;
    qint64 cp0, cp1;
    const QString &T;
    QByteArray out;
    QByteArray lastParagraph;         // held back so a field left open can be closed in it
    QVector<TableOut> tables;
    // The properties of the characters last read and what they came to as XML, by the bytes they came from.
    struct Seen {
        bool valid = false;
        qint32 off = 0, len = 0;
        quint16 prm = 0;
        int istd = 0;
        Chp chp;
        QByteArray rpr;
    } seen;
    QVector<Field> fields;
    int fieldsNotKept = 0;            // fields open past the most that are kept track of (see kMaxFieldsKept), whose marks only count
    int nextSection = 0;
    int nextBookmark = 0;
    // The bookmarks that begin or end at or before `cp`, as elements between the runs.
    void bookmarksUpTo(qint64 cp, RunBuilder &rb)
    {
        if (kind != Converter::Kind::Main) return;
        while (nextBookmark < c.bookmarkEvents.size() && c.bookmarkEvents[nextBookmark].cp <= cp) rb.between(c.bookmarkEvents[nextBookmark++].xml);
    }
};

bool StoryWriter::isEnd(qint64 cp) const
{
    const ushort ch = T[cp].unicode();
    if (ch == 0x0D) {
        // A paragraph mark ends its paragraph where the paragraph's properties end with it; in the middle of a stretch that has one set of them it is a line break.
        const Piece *pc = c.pieces.at(cp);
        if (!pc) return true;
        const qint64 fc = PieceTable::fcOf(*pc, cp);
        const BinRun *run = c.papBin.find(quint32(fc));
        return !run || qint64(run->fc1) == fc + (pc->unicode ? 2 : 1);
    }
    if (ch == 0x07) return true;
    if (ch == 0x0C && kind == Converter::Kind::Main) {
        // A form feed that is the last character of a section (as the file's table of sections says) ends its paragraph; others are page breaks.
        return std::binary_search(c.sectionEnds.begin(), c.sectionEnds.end(), cp + 1);
    }
    return false;
}

QByteArray StoryWriter::run()
{
    qint64 cp = cp0;
    const qint64 end = std::min<qint64>(cp1, T.size());
    while (cp < end && !c.truncated) {
        qint64 pe = cp;
        while (pe < end && !isEnd(pe)) ++pe;
        const bool terminated = pe < end;
        paragraph(cp, terminated ? pe : end - 1, terminated);
        cp = pe + 1;
    }
    // Fields left open are closed in the last paragraph; tables left open are closed.
    if (!fields.isEmpty() && !lastParagraph.isEmpty()) {
        QByteArray closing;
        for (const Field &f : fields)
            if (f.real) closing += "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        const qsizetype at = lastParagraph.lastIndexOf("</w:p>");
        if (at >= 0) lastParagraph.insert(at, closing);
        fields.clear();
    }
    flushParagraph();
    adjustTables(0);
    return out;
}

void StoryWriter::flushParagraph()
{
    if (lastParagraph.isEmpty()) return;
    sink() += lastParagraph;
    lastParagraph.clear();
}

QByteArray StoryWriter::rprOf(const Chp &chp, int istd) const
{
    quint8 parent[TogCount];
    c.styles.runParent(istd, chp.istd.value_or(-1), parent);
    QString charStyle;
    if (chp.istd) {
        const StyleDef *s = c.styles.style(*chp.istd);
        if (s && s->sgc == 2 && s->sti != 65) charStyle = s->id;
    }
    Chp shown = chp;
    return runPropertiesXml(shown, c.fonts, parent, charStyle);
}

void StoryWriter::paragraph(qint64 first, qint64 last, bool terminated)
{
    if (c.paragraphs >= kMaxParagraphs) c.cutShort();
    if (c.truncated) return;
    const Piece *endPiece = c.pieces.at(last);
    if (!endPiece) return;
    const quint32 fcEnd = quint32(PieceTable::fcOf(*endPiece, last));
    const ushort endChar = terminated ? T[last].unicode() : 0x0D;

    Pap pap;
    int istd = 0;
    // A paragraph that has paragraph marks inside it (they are line breaks) is as the stretch it begins in has it, not as the one it ends in.
    const BinRun *runOf = c.papBin.find(fcEnd);
    if (terminated && last > first) {
        bool inside = false;
        for (qint64 q = first; q < last && !inside; ++q) inside = T[q] == QChar(0x0D);
        if (inside)
            if (const Piece *startPiece = c.pieces.at(first))
                if (const BinRun *begun = c.papBin.find(quint32(PieceTable::fcOf(*startPiece, first)))) runOf = begun;
    }
    if (const BinRun *pr = runOf) {
        istd = pr->istd;
        if (pr->off >= 0) pap.apply(c.wd.sub(pr->off, pr->len), c.data);
    }
    pap.apply(c.pieces.modifier(*endPiece));
    Chp mark;
    if (const BinRun *cr = c.chpBin.find(fcEnd)) {
        if (cr->off >= 0) mark.apply(c.wd.sub(cr->off, cr->len));
    }
    mark.apply(c.pieces.modifier(*endPiece));

    // (The paragraph held back belongs to the table it was made in: it goes in before the tables change.)
    flushParagraph();
    int depth = pap.inTable || pap.itap > 0 ? std::max(pap.itap, 1) : 0;
    const bool tooDeep = depth > kMaxTableDepth;
    if (tooDeep) {
        depth = kMaxTableDepth;
        c.left(QStringLiteral("deep"), QCoreApplication::translate("jp::office::doc", "Tables nested deeper than this program can read were made plain paragraphs of text."));
    }
    const bool rowEnd = !tooDeep && depth > 0 && (pap.ttp || pap.innerTtp);
    const bool cellEnd = !tooDeep && depth > 0 && !rowEnd && (endChar == 0x07 || pap.innerCell);
    adjustTables(depth);
    if (rowEnd) {
        finishRow(pap);
        return;
    }

    RunBuilder rb(c);
    const qint64 contentEnd = terminated ? last : last + 1;
    bookmarksUpTo(first - 1, rb);
    content(first, contentEnd, istd, rb);
    bookmarksUpTo(last, rb);
    rb.flushRun();

    ParaContext ctx;
    ctx.listInstances = c.lists.instances;
    if (istd > 0) ctx.styleId = c.styles.idOf(istd);
    QByteArray ppr = paragraphPropertiesXml(pap, ctx);
    {
        quint8 parent[TogCount];
        c.styles.runParent(istd, -1, parent);
        QString charStyle;
        if (mark.istd) {
            const StyleDef *s = c.styles.style(*mark.istd);
            if (s && s->sgc == 2 && s->sti != 65) charStyle = s->id;
        }
        // (A paragraph mark that was inserted or deleted says so first among its properties.)
        QByteArray mrpr;
        if (mark.insertion.made) mrpr += "<w:ins" + revisionAttributes(++c.revisionId, c.revisionAuthor(mark.insertion.author), mark.insertion.date) + "/>";
        if (mark.deletion.made) mrpr += "<w:del" + revisionAttributes(++c.revisionId, c.revisionAuthor(mark.deletion.author), mark.deletion.date) + "/>";
        mrpr += runPropertiesXml(mark, c.fonts, parent, charStyle);
        if (!mrpr.isEmpty()) ppr += "<w:rPr>" + mrpr + "</w:rPr>";
    }
    if (kind == Converter::Kind::Main && depth == 0) {
        QByteArray sect;
        while (nextSection < c.sections.size() - 1 && c.sections[nextSection].end <= last + 1) {
            sect = c.sections[nextSection].xml;
            ++nextSection;
        }
        ppr += sect;
    }
    QByteArray p = "<w:p>";
    if (!ppr.isEmpty()) p += "<w:pPr>" + ppr + "</w:pPr>";
    c.spend(p + "</w:p>");   // (the runs were counted as each was made)
    p += rb.paragraph + "</w:p>";
    ++c.paragraphs;
    if (cellEnd) {
        sink() += p;
        tables.last().cells.push_back(tables.last().cell);
        tables.last().cell.clear();
    } else {
        lastParagraph = p;   // (held back until the next one: a field may have to be closed in it)
    }
}

void StoryWriter::content(qint64 from, qint64 to, int istd, RunBuilder &rb)
{
    qint64 q = from;
    while (q < to) {
        if (c.truncated) break;
        const Piece *pc = c.pieces.at(q);
        if (!pc) break;
        qint64 segEnd = std::min(to, pc->cp1);
        const quint32 fc = quint32(PieceTable::fcOf(*pc, q));
        const qint64 per = pc->unicode ? 2 : 1;
        const BinRun *run = c.chpBin.find(fc);
        if (run) {
            segEnd = std::min(segEnd, q + std::max<qint64>(1, (qint64(run->fc1) - fc + per - 1) / per));
        } else {
            const quint32 next = c.chpBin.nextStart(fc);
            if (next != 0xFFFFFFFFu) segEnd = std::min(segEnd, q + std::max<qint64>(1, (qint64(next) - fc + per - 1) / per));
        }
        // (Stretches with the same properties' bytes, as the pieces of a text that shares its stored text have, come to the same properties.)
        const qint32 off = run && run->off >= 0 ? run->off : -1, len = off >= 0 ? run->len : 0;
        if (!seen.valid || seen.off != off || seen.len != len || seen.prm != pc->prm || seen.istd != istd) {
            seen.valid = true;
            seen.off = off;
            seen.len = len;
            seen.prm = pc->prm;
            seen.istd = istd;
            seen.chp = Chp();
            if (off >= 0) seen.chp.apply(c.wd.sub(off, len));
            seen.chp.apply(c.pieces.modifier(*pc));
            seen.rpr = rprOf(seen.chp, istd);
        }
        characters(q, segEnd, seen.chp, seen.rpr, rb);
        q = segEnd;
    }
}

void StoryWriter::characters(qint64 from, qint64 to, const Chp &chp, const QByteArray &rpr, RunBuilder &rb)
{
    rb.setRun(rpr, chp.insertion, chp.deletion);
    QString plain;
    auto flushPlain = [&] {
        if (plain.isEmpty()) return;
        const bool instruction = !fields.isEmpty() && !fields.last().result;
        const bool hidden = instruction && !fields.last().real;
        if (!hidden) rb.text(plain, instruction);
        plain.clear();
    };
    for (qint64 q = from; q < to && !c.truncated; ++q) {
        if (nextBookmark < c.bookmarkEvents.size() && kind == Converter::Kind::Main && c.bookmarkEvents[nextBookmark].cp <= q) {
            flushPlain();
            bookmarksUpTo(q - 1, rb);   // (those that are at this character begin before it)
            while (nextBookmark < c.bookmarkEvents.size() && c.bookmarkEvents[nextBookmark].cp == q) rb.between(c.bookmarkEvents[nextBookmark++].xml);
            rb.setRun(rpr, chp.insertion, chp.deletion);
        }
        const QChar qc = T[q];
        const ushort ch = qc.unicode();
        if (ch >= 0x20 && !(ch == 0x28 && chp.symbolFont >= 0)) {
            // (A stretch of ordinary characters goes in at once: up to the next mark of a bookmark, or as many as a run is made of at most.)
            qint64 stop = std::min<qint64>(to, q + (kMaxRunChars - plain.size()));
            if (kind == Converter::Kind::Main && nextBookmark < c.bookmarkEvents.size()) stop = std::min(stop, std::max(q + 1, c.bookmarkEvents[nextBookmark].cp));
            qint64 r = q + 1;
            while (r < stop) {
                const ushort u = T[r].unicode();
                if (u < 0x20 || (u == 0x28 && chp.symbolFont >= 0)) break;
                ++r;
            }
            plain.append(T.constData() + q, qsizetype(r - q));
            q = r - 1;
            if (plain.size() >= kMaxRunChars) flushPlain();
            continue;
        }
        const bool instruction = !fields.isEmpty() && !fields.last().result;
        switch (ch) {
        case 0x09:
            flushPlain();
            if (!instruction) rb.element("<w:tab/>");
            break;
        case 0x0B:
            flushPlain();
            if (!instruction) rb.element("<w:br/>");
            break;
        case 0x0D:   // (a paragraph mark inside a paragraph)
            flushPlain();
            if (!instruction) rb.element("<w:cr/>");
            break;
        case 0x0C:
            flushPlain();
            if (!instruction) rb.element("<w:br w:type=\"page\"/>");
            break;
        case 0x0E:
            flushPlain();
            if (!instruction) rb.element("<w:br w:type=\"column\"/>");
            break;
        case 0x1E:
            flushPlain();
            if (!instruction) rb.element("<w:noBreakHyphen/>");
            break;
        case 0x1F:
            flushPlain();
            if (!instruction) rb.element("<w:softHyphen/>");
            break;
        case 0x13: {
            flushPlain();
            if (fields.size() >= kMaxFieldsKept) {
                ++fieldsNotKept;   // (a mark for each of 40 million characters is not 40 million fields)
                break;
            }
            Field f;
            f.real = fields.size() < kMaxFieldDepth && (fields.isEmpty() || fields.last().real);
            fields.push_back(f);
            if (f.real) rb.alone("<w:fldChar w:fldCharType=\"begin\"/>");
            break;
        }
        case 0x14:
            flushPlain();
            if (fieldsNotKept == 0 && !fields.isEmpty() && !fields.last().result) {
                fields.last().result = true;
                if (fields.last().real) rb.alone("<w:fldChar w:fldCharType=\"separate\"/>");
            }
            break;
        case 0x15:
            flushPlain();
            if (fieldsNotKept > 0) {
                --fieldsNotKept;
            } else if (!fields.isEmpty()) {
                if (fields.last().real) rb.alone("<w:fldChar w:fldCharType=\"end\"/>");
                fields.pop_back();
            }
            break;
        case 0x01:
            flushPlain();
            if (chp.special && chp.picture >= 0 && !chp.data && !chp.ole2 && !instruction) {
                rb.flushRun();
                const QByteArray drawing = c.pictureXml(chp.picture, part);
                if (!drawing.isEmpty()) {
                    rb.setRun(rpr, chp.insertion, chp.deletion);
                    rb.element(drawing);
                }
            }
            break;
        case 0x02:
            flushPlain();
            if (chp.special && !instruction) {
                if (q == cp0 && (kind == Converter::Kind::Footnote || kind == Converter::Kind::Endnote)) {
                    rb.element(kind == Converter::Kind::Footnote ? "<w:footnoteRef/>" : "<w:endnoteRef/>");
                } else if (kind == Converter::Kind::Main || kind == Converter::Kind::TextBox) {
                    auto it = std::lower_bound(c.footnoteRefs.begin(), c.footnoteRefs.end(), q);
                    if (it != c.footnoteRefs.end() && *it == q) {
                        rb.element("<w:footnoteReference w:id=\"" + QByteArray::number(int(it - c.footnoteRefs.begin()) + 1) + "\"/>");
                    } else {
                        it = std::lower_bound(c.endnoteRefs.begin(), c.endnoteRefs.end(), q);
                        if (it != c.endnoteRefs.end() && *it == q) rb.element("<w:endnoteReference w:id=\"" + QByteArray::number(int(it - c.endnoteRefs.begin()) + 1) + "\"/>");
                    }
                }
            }
            break;
        case 0x08:
            flushPlain();
            if (chp.special && !instruction) {
                const QByteArray shapes = c.shapeXml(q, kind, part);
                if (!shapes.isEmpty()) rb.alone(shapes);
            }
            break;
        case 0x28:
            flushPlain();
            if (chp.symbolFont >= 0 && !instruction)
                rb.element("<w:sym w:font=\"" + xmlEscape(c.fonts.name(chp.symbolFont)) + "\" w:char=\"" + QByteArray::number(chp.symbolChar & 0xFFFF, 16).rightJustified(4, '0').toUpper() + "\"/>");
            break;
        default:   // (other control characters are marks of the file's own)
            flushPlain();
            break;
        }
    }
    flushPlain();
}

// ---- tables

void StoryWriter::adjustTables(int depth)
{
    while (tables.size() > depth) closeTable();
    while (tables.size() < depth) tables.push_back(TableOut());
}

void StoryWriter::closeTable()
{
    TableOut t = tables.takeLast();
    // A row left unfinished (the file ended) is finished with what it has.
    if (!t.cells.isEmpty() || !t.cell.isEmpty()) {
        if (!t.cell.isEmpty()) t.cells.push_back(t.cell);
        RowOut r;
        r.cells = t.cells;
        t.rows.push_back(r);
    }
    if (t.rows.isEmpty()) return;
    sink() += tableXml(t);
}

void StoryWriter::finishRow(const Pap &pap)
{
    TableOut &t = tables.last();
    RowOut r;
    r.tap = pap.tap;
    r.cells = t.cells;
    if (!t.cell.isEmpty()) r.cells.push_back(t.cell);
    t.cells.clear();
    t.cell.clear();
    if (r.cells.isEmpty()) return;
    // (More rows or cells than the .docx reader lays out would be shown as a stand-in: the rest are left out.)
    if (t.rows.size() >= kMaxTableRows || c.tableCells + int(r.cells.size()) > kMaxTableCellsAll) {
        c.left(QStringLiteral("rows"), QCoreApplication::translate("jp::office::doc", "A table is larger than this program can show: the rest of it was left out."));
        return;
    }
    c.tableCells += int(r.cells.size());
    // A row that says other things of its table than the rows before it is a table of its own, set after them (as the original program wrote them).
    if (!t.rows.isEmpty() && !t.rows.last().tap.sameTable(r.tap)) {
        QByteArray &parent = tables.size() >= 2 ? tables[tables.size() - 2].cell : out;
        parent += tableXml(t);
        t.rows.clear();
    }
    t.rows.push_back(r);
}

namespace {

int gridIndex(const QVector<int> &grid, int v)
{
    // (The boundary nearest to v.)
    auto it = std::lower_bound(grid.begin(), grid.end(), v);
    if (it == grid.end()) return int(grid.size()) - 1;
    if (it != grid.begin() && v - *(it - 1) <= *it - v) --it;
    return int(it - grid.begin());
}

QByteArray emptyParagraph() { return "<w:p/>"; }

} // namespace

// How wide the text is where the table is (the page less its margins), for a table that is a share of it.
int StoryWriter::availableWidth() const
{
    const int index = kind == Converter::Kind::Main ? std::min<int>(nextSection, int(c.sections.size()) - 1) : 0;
    if (index < 0 || index >= c.sections.size()) return 9360;
    const Sep &sep = c.sections[index].sep;
    return std::max(720, sep.pageW - sep.left - sep.right - sep.gutter);
}

QByteArray StoryWriter::tableXml(TableOut &t)
{
    // The columns of the table: every edge any row has, edges within a few twips of one another as one.
    QVector<int> edges;
    for (RowOut &r : t.rows) {
        if (r.tap.center.size() != r.tap.cells.size() + 1 || r.tap.cells.isEmpty()) {
            // A row that doesn't say where its cells are: equal cells across the width of the text.
            const int n = int(r.cells.size());
            r.tap.center.clear();
            r.tap.cells = QVector<Tc>(n);
            for (int i = 0; i <= n; ++i) r.tap.center.push_back(i * 8640 / std::max(1, n));
        }
        for (int e : r.tap.center) edges.push_back(e);
    }
    std::sort(edges.begin(), edges.end());
    QVector<int> grid;
    for (int e : edges)
        if (grid.isEmpty() || e - grid.last() > 3) grid.push_back(e);
    if (grid.size() < 2) grid = {0, 8640};
    if (grid.size() > 2 * kMaxCells + 2) {
        // (A table whose rows disagree on where the columns are: the edges are thinned to the most a row can have.)
        QVector<int> thin;
        for (int i = 0; i < grid.size(); i += 2) thin.push_back(grid[i]);
        thin.push_back(grid.last());
        grid = thin;
    }
    const Tap &first = t.rows.first().tap;
    const int gap = first.gapHalf >= 0 ? first.gapHalf : 108;
    // A table that is a share of the width of the text has its columns made to come to it.
    double scale = 1;
    if (first.tableWidthType == 2 && first.tableWidth > 0 && grid.last() > grid.first()) scale = (first.tableWidth / 5000.0) * availableWidth() / (grid.last() - grid.first());
    auto sc = [&](int v) { return int(std::lround(v * scale)); };

    QByteArray x = "<w:tbl><w:tblPr>";
    if (const StyleDef *s = first.istd ? c.styles.style(*first.istd) : nullptr; s && s->sgc == 3) x += "<w:tblStyle w:val=\"" + s->id.toUtf8() + "\"/>";
    if (first.floating) {
        const int vert = (first.anchors >> 4) & 3, horz = (first.anchors >> 6) & 3;
        x += "<w:tblpPr";
        if (first.fromLeft) x += " w:leftFromText=\"" + QByteArray::number(first.fromLeft) + "\"";
        if (first.fromRight) x += " w:rightFromText=\"" + QByteArray::number(first.fromRight) + "\"";
        if (first.fromTop) x += " w:topFromText=\"" + QByteArray::number(first.fromTop) + "\"";
        if (first.fromBottom) x += " w:bottomFromText=\"" + QByteArray::number(first.fromBottom) + "\"";
        x += vert == 0 ? " w:vertAnchor=\"margin\"" : vert == 1 ? " w:vertAnchor=\"page\"" : " w:vertAnchor=\"text\"";
        x += horz == 0 ? " w:horzAnchor=\"text\"" : horz == 1 ? " w:horzAnchor=\"margin\"" : " w:horzAnchor=\"page\"";
        switch (first.xAbs) {
        case 0: x += " w:tblpXSpec=\"left\""; break;
        case -4: x += " w:tblpXSpec=\"center\""; break;
        case -8: x += " w:tblpXSpec=\"right\""; break;
        case -12: x += " w:tblpXSpec=\"inside\""; break;
        case -16: x += " w:tblpXSpec=\"outside\""; break;
        default: x += " w:tblpX=\"" + QByteArray::number(first.xAbs) + "\""; break;
        }
        switch (first.yAbs) {
        case -4: x += " w:tblpYSpec=\"top\""; break;
        case -8: x += " w:tblpYSpec=\"center\""; break;
        case -12: x += " w:tblpYSpec=\"bottom\""; break;
        case -16: x += " w:tblpYSpec=\"inside\""; break;
        case -20: x += " w:tblpYSpec=\"outside\""; break;
        default: x += " w:tblpY=\"" + QByteArray::number(first.yAbs) + "\""; break;
        }
        x += "/>";
        if (first.noOverlap) x += "<w:tblOverlap w:val=\"never\"/>";
    }
    if (first.tableWidthType == 2) x += "<w:tblW w:w=\"" + QByteArray::number(first.tableWidth) + "\" w:type=\"pct\"/>";
    else if (first.tableWidthType == 3) x += "<w:tblW w:w=\"" + QByteArray::number(first.tableWidth) + "\" w:type=\"dxa\"/>";
    else x += "<w:tblW w:w=\"0\" w:type=\"auto\"/>";
    if (first.jc == 1) x += "<w:jc w:val=\"center\"/>";
    else if (first.jc == 2) x += "<w:jc w:val=\"right\"/>";
    if (first.spacing > 0) x += "<w:tblCellSpacing w:w=\"" + QByteArray::number(first.spacing) + "\" w:type=\"dxa\"/>";
    if (const int ind = first.indent.value_or(grid.first() + gap)) x += "<w:tblInd w:w=\"" + QByteArray::number(ind) + "\" w:type=\"dxa\"/>";
    {
        const QByteArray borders = borderXml("top", first.top) + borderXml("left", first.left) + borderXml("bottom", first.bottom) + borderXml("right", first.right) +
                                   borderXml("insideH", first.insideH) + borderXml("insideV", first.insideV);
        if (!borders.isEmpty()) x += "<w:tblBorders>" + borders + "</w:tblBorders>";
    }
    if (!first.autofit) x += "<w:tblLayout w:type=\"fixed\"/>";
    {
        auto pad = [&](const char *side, int v) { return v >= 0 ? QByteArray("<w:") + side + " w:w=\"" + QByteArray::number(v) + "\" w:type=\"dxa\"/>" : QByteArray(); };
        // (The usual 108 on each side is the style's, left unsaid.)
        const int left = first.padLeft >= 0 ? first.padLeft : gap, right = first.padRight >= 0 ? first.padRight : gap;
        const QByteArray mar = (first.padTop > 0 ? pad("top", first.padTop) : QByteArray()) + (left == 108 ? QByteArray() : pad("left", left)) + (first.padBottom > 0 ? pad("bottom", first.padBottom) : QByteArray()) + (right == 108 ? QByteArray() : pad("right", right));
        if (!mar.isEmpty()) x += "<w:tblCellMar>" + mar + "</w:tblCellMar>";
    }
    x += "<w:tblLook w:val=\"0000\"/></w:tblPr><w:tblGrid>";
    for (int i = 0; i + 1 < grid.size(); ++i) x += "<w:gridCol w:w=\"" + QByteArray::number(sc(grid[i + 1] - grid[i])) + "\"/>";
    x += "</w:tblGrid>";
    c.spend(x);   // (what the cells hold was counted as it was made)

    for (const RowOut &r : t.rows) {
        const Tap &tap = r.tap;
        const int n = int(tap.cells.size());
        QByteArray row = "<w:tr>";
        // The space inside this row's cells where it isn't the table's (as the rows of a table that differ in it).
        auto marginOf = [](const Tap &t, int top, int left, int bottom, int right) {
            return std::array<int, 4>{t.padTop >= 0 ? t.padTop : top, t.padLeft >= 0 ? t.padLeft : (t.gapHalf >= 0 ? t.gapHalf : left), t.padBottom >= 0 ? t.padBottom : bottom,
                                      t.padRight >= 0 ? t.padRight : (t.gapHalf >= 0 ? t.gapHalf : right)};
        };
        const std::array<int, 4> tableMargin = marginOf(first, 0, 108, 0, 108), rowMargin = marginOf(tap, 0, 108, 0, 108);
        const bool ownMargin = rowMargin != tableMargin;
        if (ownMargin)
            row += "<w:tblPrEx><w:tblCellMar><w:top w:w=\"" + QByteArray::number(rowMargin[0]) + "\" w:type=\"dxa\"/><w:left w:w=\"" + QByteArray::number(rowMargin[1]) +
                   "\" w:type=\"dxa\"/><w:bottom w:w=\"" + QByteArray::number(rowMargin[2]) + "\" w:type=\"dxa\"/><w:right w:w=\"" + QByteArray::number(rowMargin[3]) +
                   "\" w:type=\"dxa\"/></w:tblCellMar></w:tblPrEx>";
        // Where the row begins and ends in the grid.
        const int startCol = gridIndex(grid, tap.center.first()), endCol = gridIndex(grid, tap.center.last());
        QByteArray trPr;
        if (startCol > 0) trPr += "<w:gridBefore w:val=\"" + QByteArray::number(startCol) + "\"/>";
        if (endCol < int(grid.size()) - 1) trPr += "<w:gridAfter w:val=\"" + QByteArray::number(int(grid.size()) - 1 - endCol) + "\"/>";
        if (startCol > 0) trPr += "<w:wBefore w:w=\"" + QByteArray::number(sc(tap.center.first() - grid.first())) + "\" w:type=\"dxa\"/>";
        if (endCol < int(grid.size()) - 1) trPr += "<w:wAfter w:w=\"" + QByteArray::number(sc(grid.last() - tap.center.last())) + "\" w:type=\"dxa\"/>";
        if (tap.cantSplit) trPr += "<w:cantSplit/>";
        if (tap.rowHeight) trPr += "<w:trHeight w:val=\"" + QByteArray::number(std::abs(tap.rowHeight)) + "\"" + (tap.rowHeight < 0 ? " w:hRule=\"exact\"" : "") + "/>";
        if (tap.header) trPr += "<w:tblHeader/>";
        if (tap.jc == 1) trPr += "<w:jc w:val=\"center\"/>";
        else if (tap.jc == 2) trPr += "<w:jc w:val=\"right\"/>";
        if (!trPr.isEmpty()) row += "<w:trPr>" + trPr + "</w:trPr>";
        c.spend(row);

        int i = 0;
        while (i < n) {
            // A cell and the cells merged into it to the right.
            int j = i + 1;
            while (j < n && tap.cells[j].merged) ++j;
            const Tc &tc = tap.cells[i];
            const Tc &lastTc = tap.cells[j - 1];
            const int gl = gridIndex(grid, tap.center[i]), gr = std::max(gl + 1, gridIndex(grid, tap.center[j]));
            QByteArray content;
            for (int k = i; k < j && k < r.cells.size(); ++k) {
                // (The cells merged into another hold only their marks, apart from the first.)
                if (k > i && !r.cells[k].contains("<w:t") && !r.cells[k].contains("<w:drawing") && !r.cells[k].contains("<w:tbl>")) continue;
                content += r.cells[k];
            }
            if (i == n - 1 && r.cells.size() > n)
                for (int k = n; k < r.cells.size(); ++k) content += r.cells[k];
            if (content.isEmpty() || content.endsWith("</w:tbl>")) content += emptyParagraph();
            QByteArray tcPr;
            if (tap.autofit && tc.widthType == 1) tcPr = "<w:tcW w:w=\"0\" w:type=\"auto\"/>";
            else if (tap.autofit && tc.widthType == 2) tcPr = "<w:tcW w:w=\"" + QByteArray::number(tc.width) + "\" w:type=\"pct\"/>";
            else tcPr = "<w:tcW w:w=\"" + QByteArray::number(sc(tap.center[j] - grid.first()) - sc(tap.center[i] - grid.first())) + "\" w:type=\"dxa\"/>";
            if (gr - gl > 1) tcPr += "<w:gridSpan w:val=\"" + QByteArray::number(gr - gl) + "\"/>";
            if (tc.vertRestart) tcPr += "<w:vMerge w:val=\"restart\"/>";
            else if (tc.vertMerge) tcPr += "<w:vMerge/>";
            {
                const QByteArray b = borderXml("top", tc.top) + borderXml("left", tc.left) + borderXml("bottom", tc.bottom) + borderXml("right", lastTc.right);
                if (!b.isEmpty()) tcPr += "<w:tcBorders>" + b + "</w:tcBorders>";
            }
            tcPr += shadingXml(tc.shd);
            if (tc.noWrap) tcPr += "<w:noWrap/>";
            {
                auto pad = [&](const char *side, int v) { return v >= 0 ? QByteArray("<w:") + side + " w:w=\"" + QByteArray::number(v) + "\" w:type=\"dxa\"/>" : QByteArray(); };
                // (A row with margins of its own gives each cell them, as the layout reads them from the cells.)
                const QByteArray mar = pad("top", tc.padTop >= 0 ? tc.padTop : ownMargin ? rowMargin[0] : -1) + pad("left", tc.padLeft >= 0 ? tc.padLeft : ownMargin ? rowMargin[1] : -1) +
                                       pad("bottom", tc.padBottom >= 0 ? tc.padBottom : ownMargin ? rowMargin[2] : -1) + pad("right", tc.padRight >= 0 ? tc.padRight : ownMargin ? rowMargin[3] : -1);
                if (!mar.isEmpty()) tcPr += "<w:tcMar>" + mar + "</w:tcMar>";
            }
            if (tc.textFlow == 1) tcPr += "<w:textDirection w:val=\"tbRl\"/>";
            else if (tc.textFlow == 2) tcPr += "<w:textDirection w:val=\"btLr\"/>";
            if (tc.vAlign == 1) tcPr += "<w:vAlign w:val=\"center\"/>";
            else if (tc.vAlign == 2) tcPr += "<w:vAlign w:val=\"bottom\"/>";
            c.spend("<w:tc><w:tcPr>" + tcPr + "</w:tcPr></w:tc>");
            row += "<w:tc><w:tcPr>" + tcPr + "</w:tcPr>" + content + "</w:tc>";
            i = j;
        }
        row += "</w:tr>";
        x += row;
    }
    x += "</w:tbl>";
    return x;
}

} // namespace

QByteArray Converter::story(qint64 cp0, qint64 cp1, Kind kind, const QString &part)
{
    StoryWriter w(*this, kind, part, cp0, cp1);
    return w.run();
}

} // namespace jp::office::doc
