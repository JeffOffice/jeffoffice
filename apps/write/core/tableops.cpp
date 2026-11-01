#include "core/tableops.h"

#include "core/editops.h"
#include "core/tablemodel.h"
#include "core/writeprops.h"
#include "flow/tableflow.h"
#include "text/textprops.h"
#include "wml/xmltree.h"

#include <QHash>
#include <QSet>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <algorithm>
#include <cstdio>
#include <functional>

using namespace jp::office::wml;

namespace jp::write {

int cellKeyOf(const QTextBlock &block) { return block.isValid() ? block.blockFormat().intProperty(flow::fp::Cell) : 0; }

bool canJoinBlocks(const QTextBlock &first, const QTextBlock &second) { return cellKeyOf(first) == cellKeyOf(second); }

namespace {

// ---- the table the cursor is in, as the commands see it

struct View {
    flow::TableIndex index;
    std::shared_ptr<flow::TableLayout> layout;
    int first = 0, last = 0, depth = 0;       // the table's paragraphs in the index, and how many cells deep it is
};

std::shared_ptr<View> viewAt(WriteDoc &doc, int position)
{
    QTextDocument *story = doc.story();
    const QTextBlock b = story->findBlock(std::clamp(position, 0, std::max(0, story->characterCount() - 1)));
    const int key = cellKeyOf(b);
    if (!key) return nullptr;
    auto v = std::make_shared<View>();
    v->index.rebuild(story, doc.flow().tables);
    const QVector<const flow::CellRecord *> &chain = v->index.chain(key);
    if (chain.isEmpty()) return nullptr;
    v->depth = int(chain.size()) - 1;
    const flow::TableData *table = chain.last()->table.get();
    const std::vector<flow::BlockRef> &refs = v->index.refs();
    const auto found = std::lower_bound(refs.begin(), refs.end(), b.position(), [](const flow::BlockRef &r, int p) { return r.pos < p; });
    if (found == refs.end() || found->pos != b.position()) return nullptr;
    const int at = int(found - refs.begin());
    auto inThis = [&](int i) {
        const QVector<const flow::CellRecord *> &c = v->index.chain(refs[size_t(i)].key);
        return int(c.size()) > v->depth && c[v->depth]->table.get() == table;
    };
    int i0 = at, i1 = at + 1;
    while (i0 > 0 && inThis(i0 - 1)) --i0;
    while (i1 < int(refs.size()) && inThis(i1)) ++i1;
    flow::TableContext ctx;
    ctx.doc = &doc.flow();
    ctx.index = &v->index;
    ctx.live = std::make_shared<flow::TableContext::Live>();
    ctx.at = 0;
    v->layout = flow::TableLayout::make(ctx, i0, i1, v->depth);
    v->first = i0;
    v->last = i1;
    return v->layout ? v : nullptr;
}

// The rows and columns of the cells a selection is in (grid columns, so a cell that spans columns has all of them).
struct Area {
    int r0 = 0, r1 = 0, c0 = 0, c1 = 0;
};

Area areaOf(const View &v, const QTextCursor &c)
{
    const flow::TableLayout &t = *v.layout;
    const QTextDocument *d = c.document();
    int a = c.selectionStart(), b = c.selectionEnd();
    if (c.hasSelection() && b > a && d->findBlock(b).position() == b && d->findBlock(b).length() > 1) --b;   // (the selection ends where a paragraph with something in it begins: it is not in that one; an empty one has no more to select)
    int ca = t.cellOfPosition(a), cb = t.cellOfPosition(b);
    if (ca < 0) ca = cb;
    if (cb < 0) cb = ca;
    Area area;
    if (ca < 0) return area;
    const flow::TableLayout::Cell &x = t.cell(ca), &y = t.cell(cb);
    area.r0 = std::min(x.row, y.row);
    area.r1 = std::max(x.row + x.rows, y.row + y.rows) - 1;
    area.c0 = std::min(x.col, y.col);
    area.c1 = std::max(x.col + x.span, y.col + y.span) - 1;
    return area;
}

// ---- the file's bytes of a table's parts

// The start of a row, a cell, or a table as the file's bytes (see tablemodel.h) read as XML: they are written under the declarations of the root of the part they
// came from, so they are read under a root that has the usual ones. `element` is the element they are the start of; false when they can't be read.
bool parseKept(XmlTree *t, const QByteArray &start, const char *closing, int *element)
{
    static const QByteArray declarations =
        " xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\""
        " xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\""
        " xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\""
        " xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\""
        " xmlns:w10=\"urn:schemas-microsoft-com:office:word\" xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\""
        " xmlns:w15=\"http://schemas.microsoft.com/office/word/2012/wordml\" xmlns:wp14=\"http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing\""
        " xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"";
    if (!t->parse("<w:keep" + declarations + ">" + start + closing + "</w:keep>", nullptr)) return false;
    *element = t->firstChild(t->root());
    return *element >= 0;
}

// The start of a row for a new one beside it: no attributes, and what the row says of its height and of keeping its cells together, not that it is a header.
QByteArray rowOpenFor(const QByteArray &open)
{
    XmlTree t;
    int tr = -1;
    if (!parseKept(&t, open, "</w:tr>", &tr)) return "<w:tr>";
    QByteArray out = "<w:tr>", pr;
    for (int c = t.firstChild(tr); c >= 0; c = t.next(c)) {
        if (t.is(c, "tblPrEx")) {
            out += t.raw(c);
        } else if (t.is(c, "trPr")) {
            for (int k = t.firstChild(c); k >= 0; k = t.next(k))
                if (t.is(k, "cantSplit") || t.is(k, "trHeight") || t.is(k, "gridBefore") || t.is(k, "wBefore") || t.is(k, "gridAfter") || t.is(k, "wAfter")) pr += t.raw(k);
        }
    }
    if (!pr.isEmpty()) out += "<w:trPr>" + pr + "</w:trPr>";
    return out;
}

// A row's start with the gaps it leaves before its first cell and after its last said as given (grid columns, and the widths in twentieths of a point that the row
// says of them: a width the row did not say is not added), and what else the row said as it was.
QByteArray rowOpenWithGaps(const QByteArray &open, int before, int beforeTwips, int after, int afterTwips)
{
    XmlTree t;
    int tr = -1;
    if (!parseKept(&t, open, "</w:tr>", &tr)) return open;
    QByteArray out = t.startTag(tr), pr;
    bool saidBefore = false, saidAfter = false;
    for (int c = t.firstChild(tr); c >= 0; c = t.next(c)) {
        if (t.is(c, "trPr")) {
            for (int k = t.firstChild(c); k >= 0; k = t.next(k)) {
                if (t.is(k, "gridBefore") || t.is(k, "gridAfter")) continue;
                if (t.is(k, "wBefore")) saidBefore = true;
                else if (t.is(k, "wAfter")) saidAfter = true;
                else pr += t.raw(k);
            }
        } else {
            out += t.raw(c);
        }
    }
    QByteArray gaps;
    if (before > 0) {
        gaps += "<w:gridBefore w:val=\"" + QByteArray::number(before) + "\"/>";
        if (saidBefore) gaps += "<w:wBefore w:w=\"" + QByteArray::number(std::max(0, beforeTwips)) + "\" w:type=\"dxa\"/>";
    }
    if (after > 0) {
        gaps += "<w:gridAfter w:val=\"" + QByteArray::number(after) + "\"/>";
        if (saidAfter) gaps += "<w:wAfter w:w=\"" + QByteArray::number(std::max(0, afterTwips)) + "\" w:type=\"dxa\"/>";
    }
    pr = gaps + pr;
    if (!pr.isEmpty()) out += "<w:trPr>" + pr + "</w:trPr>";
    return out;
}

int twipsOf(const QVector<double> &columns, int first, int count)
{
    double w = 0;
    for (int i = first; i < first + count && i < columns.size(); ++i)
        if (i >= 0) w += columns[i];
    return int(std::lround(w * 20));
}

// The same row with other gaps (its cells are the new table's, so it is another row for the table): `columns` are the new grid's widths.
std::shared_ptr<flow::RowData> rowWithGaps(const flow::RowData &old, int before, int after, const QVector<double> &columns)
{
    auto row = std::make_shared<flow::RowData>(old);
    row->before = before;
    row->after = after;
    const RowPart *rp = static_cast<const RowPart *>(old.app.get());
    auto np = std::make_shared<RowPart>(rp ? *rp : RowPart());
    np->open = rowOpenWithGaps(rp ? rp->open : QByteArray("<w:tr>"), before, twipsOf(columns, 0, before), after, twipsOf(columns, int(columns.size()) - after, after));
    row->app = np;
    return row;
}

int tcPrRank(const QByteArray &name)
{
    static const QList<QByteArray> order = {"cnfStyle", "tcW", "gridSpan", "hMerge", "vMerge", "tcBorders", "shd", "noWrap", "tcMar", "textDirection", "tcFitText", "vAlign", "hideMark",
                                            "headers", "cellIns", "cellDel", "cellMerge", "tcPrChange"};
    const int i = int(order.indexOf(name));
    return i < 0 ? 100 : i;
}

// The start of a cell for a new one, or the same one with another width or span or merge: its properties as they were, those three as given (a span of 1 and a
// merge of "" are not written), and no attributes.
QByteArray cellOpenFor(const QByteArray &open, int widthTwips, int span, const QString &vmerge)
{
    XmlTree t;
    QVector<std::pair<int, QByteArray>> kids;
    int tc = -1;
    if (parseKept(&t, open, "</w:tc>", &tc)) {
        const int pr = t.child(tc, "tcPr");
        for (int c = pr >= 0 ? t.firstChild(pr) : -1; c >= 0; c = t.next(c)) {
            const QByteArray name = t.localName(c);
            if (name == "tcW" || name == "gridSpan" || name == "vMerge") continue;
            kids.push_back({tcPrRank(name), t.raw(c)});
        }
    }
    if (widthTwips > 0) kids.push_back({tcPrRank("tcW"), "<w:tcW w:w=\"" + QByteArray::number(widthTwips) + "\" w:type=\"dxa\"/>"});
    if (span > 1) kids.push_back({tcPrRank("gridSpan"), "<w:gridSpan w:val=\"" + QByteArray::number(span) + "\"/>"});
    if (vmerge == QLatin1String("restart")) kids.push_back({tcPrRank("vMerge"), "<w:vMerge w:val=\"restart\"/>"});
    else if (vmerge == QLatin1String("continue")) kids.push_back({tcPrRank("vMerge"), "<w:vMerge/>"});
    std::stable_sort(kids.begin(), kids.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
    QByteArray out = "<w:tc>";
    if (!kids.isEmpty()) {
        out += "<w:tcPr>";
        for (const auto &k : kids) out += k.second;
        out += "</w:tcPr>";
    }
    return out;
}

QByteArray gridXml(const QVector<double> &columns)
{
    QByteArray out = "<w:tblGrid>";
    for (double w : columns) out += "<w:gridCol w:w=\"" + QByteArray::number(int(std::lround(w * 20))) + "\"/>";
    return out + "</w:tblGrid>";
}

// ---- making paragraphs and records

// A paragraph's format for the empty paragraph of a new cell like the one it came from: its look, and none of what belongs to that paragraph alone.
QTextBlockFormat emptyLike(QTextBlockFormat bf, int key)
{
    for (int k : {int(prop::RawBefore), int(prop::RawAfter), int(prop::RawBlock), int(prop::ParaAttrs), int(prop::SectionData), int(prop::LastSection), int(prop::MarkFormat),
                  int(prop::NumId), int(prop::NumLevel), int(prop::NumDirect), int(flow::fp::SectionBreakAfter)})
        bf.clearProperty(k);
    bf.clearProperty(tp::BulletChar);
    bf.setObjectIndex(-1);
    bf.setProperty(flow::fp::Cell, key);
    return bf;
}

// A new paragraph before `b` (not in b's cell: the format given), and one after it.
void insertBefore(QTextDocument *d, QTextBlock b, const QTextBlockFormat &bf, const QTextCharFormat &cf)
{
    const QTextBlockFormat oldF = b.blockFormat();
    const QTextCharFormat oldC = b.charFormat();
    const int at = b.position();
    QTextCursor t(d);
    t.setPosition(at);
    t.insertBlock(oldF, oldC);   // (the text keeps its paragraph's format: the empty one before it is given the new one)
    const QTextBlock made = d->findBlock(at);
    QTextCursor(made).setBlockFormat(bf);
    QTextCursor(made).setBlockCharFormat(cf);
}

void insertAfter(QTextDocument *d, QTextBlock b, const QTextBlockFormat &bf, const QTextCharFormat &cf)
{
    QTextCursor t(d);
    t.setPosition(b.position() + b.length() - 1);
    t.insertBlock(bf, cf);
}

QTextCharFormat markOf(const QTextBlock &b) { return markFormat(b); }

// Takes whole paragraphs out, first to last (a paragraph's format is that of its end, so what is taken is from the first one's start to the start of the one after the last,
// and the paragraphs on either side are as they were).
void removeParagraphs(QTextDocument *d, QTextBlock first, QTextBlock last)
{
    const QTextBlock next = last.next(), prev = first.previous();
    if (next.isValid()) {
        // The story's first paragraph holds what the story keeps in it (the last section's properties, its mark's format): the paragraph that comes to be first has them.
        QTextBlockFormat nf = next.blockFormat();
        const QTextCharFormat nc = next.charFormat();
        const bool atStart = !prev.isValid();
        if (atStart) {
            const QTextBlockFormat firstF = first.blockFormat();
            for (int k : {int(prop::LastSection), int(prop::MarkFormat)})
                if (firstF.hasProperty(k)) nf.setProperty(k, firstF.property(k));
        }
        QTextCursor t(d);
        t.setPosition(first.position());
        t.setPosition(next.position(), QTextCursor::KeepAnchor);
        t.removeSelectedText();
        if (atStart) {
            QTextCursor(d->begin()).setBlockFormat(nf);
            QTextCursor(d->begin()).setBlockCharFormat(nc);
        }
        return;
    }
    // The story's last paragraphs: the one before takes the last one's place (the end of the story is not taken out), with its own format.
    if (prev.isValid()) {
        const QTextBlockFormat pf = prev.blockFormat();
        const QTextCharFormat pc = prev.charFormat();
        const int at = prev.position();
        QTextCursor t(d);
        t.setPosition(first.position() - 1);
        t.setPosition(last.position() + last.length() - 1, QTextCursor::KeepAnchor);
        t.removeSelectedText();
        QTextBlock p = d->findBlock(at);
        QTextCursor(p).setBlockFormat(pf);
        QTextCursor(p).setBlockCharFormat(pc);
        return;
    }
    // Everything: one empty paragraph is left, out of any table.
    QTextCursor t(d);
    t.setPosition(first.position());
    t.setPosition(last.position() + last.length() - 1, QTextCursor::KeepAnchor);
    t.removeSelectedText();
    QTextBlockFormat nf = d->begin().blockFormat();
    nf.clearProperty(flow::fp::Cell);
    nf.clearProperty(prop::RawBefore);
    nf.clearProperty(prop::RawAfter);
    QTextCursor(d->begin()).setBlockFormat(nf);
}

// The paragraph after a table that is the story's last: a story ends with a paragraph that is not in a table.
void keepAParagraphAfter(WriteDoc &doc)
{
    QTextDocument *d = doc.story();
    const QTextBlock last = d->lastBlock();
    if (!cellKeyOf(last)) return;
    QTextCursor t(d);
    t.setPosition(last.position() + last.length() - 1);
    const QString style = doc.defaultParagraphStyle();
    t.insertBlock(doc.styleBlockFormat(style), doc.styleCharFormat(style));
}

// Gives the cells of a table, and the cells of the tables in them, new records, so that the paragraphs of the table can be given their cell's new key. The cells of
// `table` become cells of `newTable` (and `change` may alter the record); the cells of tables in them keep their own tables, and have their new parents; the cell the
// table is in (its key is `stop`, 0 for none) is as it was.
class Rekey {
public:
    Rekey(WriteDoc &doc, const flow::TableData *table, std::shared_ptr<const flow::TableData> newTable, int stop, std::function<void(const flow::CellRecord &, flow::CellRecord *)> change)
        : m_store(&doc.flow().tables), m_old(table), m_new(std::move(newTable)), m_stop(stop), m_change(std::move(change))
    {
    }
    int keyFor(int key)
    {
        if (!key) return 0;
        const auto it = m_map.constFind(key);
        if (it != m_map.constEnd()) return it.value();
        const flow::CellRecord *rec = m_store->get(key);
        if (!rec || rec->table.get() == m_new.get()) return key;   // (a cell that has its new record already: one made for the change)
        flow::CellRecord copy = *rec;
        copy.parent = rec->parent == m_stop ? m_stop : keyFor(rec->parent);
        if (rec->table.get() == m_old) {
            copy.table = m_new;
            if (m_change) m_change(*rec, &copy);
        }
        const int made = m_store->add(copy);
        m_map.insert(key, made);
        return made;
    }

private:
    flow::TableStore *m_store;
    const flow::TableData *m_old;
    std::shared_ptr<const flow::TableData> m_new;
    int m_stop;
    std::function<void(const flow::CellRecord &, flow::CellRecord *)> m_change;
    QHash<int, int> m_map;
};

// Gives the paragraphs of the table (and of the tables in it) their cell's new key.
void applyRekey(WriteDoc &doc, const flow::TableData *table, Rekey *rekey)
{
    QTextDocument *d = doc.story();
    const flow::TableStore &store = doc.flow().tables;
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) {
        const int key = cellKeyOf(b);
        if (!key) continue;
        bool involved = false;
        for (const flow::CellRecord *r = store.get(key); r && !involved; r = store.get(r->parent)) involved = r->table.get() == table;
        if (!involved) continue;
        QTextBlockFormat bf = b.blockFormat();
        bf.setProperty(flow::fp::Cell, rekey->keyFor(key));
        QTextCursor(b).setBlockFormat(bf);
    }
}

// What a table the columns of which change is: the same with other columns, its start's grid written as them, and its width (when it says one in twentieths of a point)
// as much more or less as the columns came to. `was` are the columns it had.
std::shared_ptr<flow::TableData> tableWithColumns(const flow::TableData &old, const QVector<double> &columns, const QVector<double> &was)
{
    auto t = std::make_shared<flow::TableData>(old);
    t->columns = columns;
    const TablePart *part = static_cast<const TablePart *>(old.app.get());
    if (part) {
        auto np = std::make_shared<TablePart>(*part);
        np->original.clear();
        np->fingerprint.clear();
        const QByteArray grid = gridXml(columns);
        auto mark = std::make_shared<TableMark>(part->mark ? *part->mark : TableMark());
        QByteArray open = part->open;
        qsizetype gridBegin = mark->gridBegin, gridEnd = mark->gridEnd;
        // The width the table says: it goes with the columns.
        if (part->mark && mark->width.stated() && mark->width.isDxa() && mark->width.w > 0 && mark->width.type == QLatin1String("dxa")) {
            XmlTree x;
            int tbl = -1;
            if (parseKept(&x, part->open, "</w:tbl>", &tbl)) {
                const int pr = x.child(tbl, "tblPr");
                const int w = pr >= 0 ? x.child(pr, "tblW") : -1;
                if (w >= 0) {
                    const QByteArray was2 = x.raw(w);
                    const int width = std::max(1, mark->width.w + twipsOf(columns, 0, int(columns.size())) - twipsOf(was, 0, int(was.size())));
                    const QByteArray now = "<w:tblW w:w=\"" + QByteArray::number(width) + "\" w:type=\"dxa\"/>";
                    const qsizetype at = open.indexOf(was2);
                    if (at >= 0) {
                        open.replace(at, was2.size(), now);
                        if (gridBegin >= 0 && at < gridBegin) {
                            gridBegin += now.size() - was2.size();
                            gridEnd += now.size() - was2.size();
                        }
                        mark->width.w = width;
                    }
                }
            }
        }
        if (gridBegin >= 0) {
            np->open = open.left(gridBegin) + grid + open.mid(gridEnd);
            mark->gridBegin = gridBegin;
            mark->gridEnd = gridBegin + int(grid.size());
            mark->grid.clear();
            for (double w : columns) mark->grid.push_back(int(std::lround(w * 20)));
        } else {
            // (A file that had no grid: it has one now, after the table's properties.)
            np->open = open + grid;
            mark->gridBegin = int(open.size());
            mark->gridEnd = mark->gridBegin + int(grid.size());
        }
        np->mark = mark;
        t->app = np;
    }
    return t;
}

// The parent of the cells of the table: the cell it is in (0 when it is not in one).
int parentOf(const View &v)
{
    return v.layout->cellCount() ? v.layout->cell(0).rec->parent : 0;
}

int columnCountOf(const flow::TableLayout &t)
{
    int n = 0;
    for (int i = 0; i < t.cellCount(); ++i) n = std::max(n, t.cell(i).col + t.cell(i).span);
    return n;
}

QVector<double> columnWidthsOf(const flow::TableLayout &t)
{
    QVector<double> w = t.data().columns;
    const int n = columnCountOf(t);
    double pad = 0;
    int counted = 0;
    for (double x : std::as_const(w))
        if (x > 0) {
            pad += x;
            ++counted;
        }
    pad = counted ? pad / counted : 72;
    while (w.size() < n) w.push_back(pad);
    return w;
}

QTextBlock firstBlockOf(const View &v, QTextDocument *d) { return d->findBlock(v.index.refs()[size_t(v.first)].pos); }
QTextBlock lastBlockOf(const View &v, QTextDocument *d) { return d->findBlock(v.index.refs()[size_t(v.last - 1)].pos); }

// The paragraph a table nested in a cell is replaced with when the cell has nothing else: an empty one in the cell.
void keepCellParagraph(WriteDoc &doc, int cellKey, int atPosition)
{
    if (!cellKey) return;
    QTextDocument *d = doc.story();
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next())
        if (cellKeyOf(b) == cellKey) return;
    // (None is left: one is made where the table was.)
    QTextBlock at = d->findBlock(atPosition);
    const QString style = doc.defaultParagraphStyle();
    QTextBlockFormat bf = doc.styleBlockFormat(style);
    bf.setProperty(flow::fp::Cell, cellKey);
    QTextCursor t(d);
    t.setPosition(std::clamp(atPosition, 0, std::max(0, d->characterCount() - 1)));
    t.insertBlock(bf, doc.styleCharFormat(style));
    Q_UNUSED(at);
}

// A row after (or before) row `ref` of a table, with a cell for each of the reference row's, like them and empty.
void addRow(WriteDoc &doc, View &v, int ref, bool below)
{
    flow::TableLayout &t = *v.layout;
    QTextDocument *d = doc.story();
    flow::TableStore &store = doc.flow().tables;
    const flow::TableLayout::Row &refRow = t.row(ref);
    auto row = std::make_shared<flow::RowData>(*refRow.data);
    row->header = false;
    {
        const RowPart *rp = static_cast<const RowPart *>(refRow.data->app.get());
        // (A hidden row is made no height: a row beside it is as high as the file says that row is, which is a row of any other kind.)
        if (rp && rp->hidden) {
            row->height = rp->height;
            row->exact = rp->exact;
        }
        auto np = std::make_shared<RowPart>();
        if (rp) {
            np->height = rp->height;
            np->exact = rp->exact;
        }
        np->open = rowOpenFor(rp ? rp->open : QByteArray("<w:tr>"));
        row->app = np;
    }
    struct NewParagraph {
        QTextBlockFormat bf;
        QTextCharFormat cf;
    };
    QVector<NewParagraph> made;
    for (int i = refRow.cell0; i < refRow.cell0 + refRow.cells; ++i) {
        const flow::TableLayout::Cell &cell = t.cell(i);
        flow::CellRecord rec = *cell.rec;
        rec.row = row;
        // (The cell of a hidden row has none of its lines and margins: a cell beside it has those the file gave it.)
        if (const CellPart *own = static_cast<const CellPart *>(cell.rec->cell.app.get()); own && own->shown) rec.cell = *own->shown;
        // A cell beside others that are joined down is joined if the row it is in goes on being: below a row whose cell goes on in the next, above a row that continues one.
        bool continues = false;
        if (below) {
            if (ref + 1 < t.rowCount()) {
                const flow::TableLayout::Row &after = t.row(ref + 1);
                for (int j = after.cell0; j < after.cell0 + after.cells; ++j)
                    if (t.cell(j).col == cell.col && t.cell(j).covered) continues = true;
            }
        } else {
            continues = cell.covered;
        }
        rec.cell.vmerge = continues ? flow::CellData::VMerge::Continue : flow::CellData::VMerge::None;
        const CellPart *cp = static_cast<const CellPart *>(cell.rec->cell.app.get());
        auto np = std::make_shared<CellPart>();
        np->open = cellOpenFor(cp ? cp->open : QByteArray("<w:tc>"), int(std::lround(cell.w * 20)), cell.span, continues ? QStringLiteral("continue") : QString());
        if (cp) np->layer = cp->layer;
        rec.cell.app = np;
        const int key = store.add(rec);
        const QTextBlock like = d->findBlock(cell.from);
        made.push_back({emptyLike(like.blockFormat(), key), markOf(like)});
    }
    // (Each new paragraph is an empty one, a place on from the one before it: they are put by place, not by the paragraph they go beside, for that one may be as empty as they are.)
    if (below) {
        int at = refRow.to - 1;
        for (const NewParagraph &n : std::as_const(made)) {
            insertAfter(d, d->findBlock(at), n.bf, n.cf);
            ++at;
        }
    } else {
        int at = refRow.from;
        for (const NewParagraph &n : std::as_const(made)) {
            insertBefore(d, d->findBlock(at), n.bf, n.cf);
            ++at;
        }
    }
}

} // namespace

CellPlace cellPlace(WriteDoc &doc, const QTextCursor &c)
{
    CellPlace p;
    const auto v = viewAt(doc, c.position());
    if (!v) return p;
    const int i = v->layout->cellOfPosition(c.position());
    if (i < 0) return p;
    p.inTable = true;
    p.row = v->layout->cell(i).row;
    p.column = v->layout->cell(i).col;
    p.rows = v->layout->rowCount();
    p.columns = columnCountOf(*v->layout);
    return p;
}

void giveCellKey(QTextBlock first, QTextBlock last, int key)
{
    for (QTextBlock b = first; b.isValid(); b = b.next()) {
        QTextBlockFormat bf = b.blockFormat();
        if (bf.intProperty(flow::fp::Cell) != key) {
            if (key) bf.setProperty(flow::fp::Cell, key);
            else bf.clearProperty(flow::fp::Cell);
            QTextCursor(b).setBlockFormat(bf);
        }
        if (b == last) break;
    }
}

bool removeSelectionAcrossCells(WriteDoc &doc, QTextCursor &c)
{
    if (!c.hasSelection() || doc.flow().tables.isEmpty()) return false;
    QTextDocument *d = c.document();
    const int from = c.selectionStart(), to = c.selectionEnd();
    const QTextBlock a = d->findBlock(from), b = d->findBlock(to);
    if (a == b) return false;
    QSet<int> keys;
    for (QTextBlock blk = a; blk.isValid(); blk = blk.next()) {
        keys.insert(cellKeyOf(blk));
        if (blk == b) break;
    }
    if (keys.size() == 1) return false;   // (in one cell, or none in a table: the paragraphs are joined as anywhere)
    c.beginEditBlock();
    QTextCursor sel(d);
    sel.setPosition(from);
    sel.setPosition(to, QTextCursor::KeepAnchor);
    // The tables the selection holds whole are taken out, last first.
    {
        flow::TableIndex index;
        index.rebuild(d, doc.flow().tables);
        QVector<std::pair<int, int>> whole;
        for (const flow::TableIndex::Span &s : index.spans())
            if (from <= s.from && to >= s.to - 1) whole.push_back({s.from, s.to});
        for (int i = int(whole.size()) - 1; i >= 0; --i) removeParagraphs(d, d->findBlock(whole[i].first), d->findBlock(whole[i].second - 1));
        if (!whole.isEmpty()) keepAParagraphAfter(doc);
    }
    if (sel.hasSelection()) {
        const int f = sel.selectionStart(), t = sel.selectionEnd();
        QSet<int> left;
        const QTextBlock fb = d->findBlock(f), tb = d->findBlock(t);
        for (QTextBlock blk = fb; blk.isValid(); blk = blk.next()) {
            left.insert(cellKeyOf(blk));
            if (blk == tb) break;
        }
        if (left.size() <= 1) {
            // (What is left of the selection is in one place: the paragraphs it has are joined, as when what was between them has gone.)
            QTextCursor again(d);
            again.setPosition(f);
            again.setPosition(t, QTextCursor::KeepAnchor);
            removeSelection(doc, again);
        } else {
            // Each stretch of the selection that is in one cell, or in none, is deleted on its own, so that what is in another cell is not joined to it.
            struct Stretch {
                int first, last;
            };
            QVector<Stretch> stretches;
            int key = -1;
            for (QTextBlock blk = fb; blk.isValid(); blk = blk.next()) {
                const int k = cellKeyOf(blk);
                if (k != key || stretches.isEmpty()) stretches.push_back({blk.position(), blk.position()});
                else stretches.last().last = blk.position();
                key = k;
                if (blk == tb) break;
            }
            for (int i = int(stretches.size()) - 1; i >= 0; --i) {
                const QTextBlock last = d->findBlock(stretches[i].last);
                const int start = std::max(f, stretches[i].first), end = std::min(t, last.position() + last.length() - 1);
                if (end <= start) continue;
                QTextCursor part(d);
                part.setPosition(start);
                part.setPosition(end, QTextCursor::KeepAnchor);
                removeSelection(doc, part);
            }
        }
    }
    c.setPosition(std::min(from, d->characterCount() - 1));
    c.endEditBlock();
    return true;
}

void insertTableRow(WriteDoc &doc, QTextCursor &c, bool below)
{
    auto v = viewAt(doc, c.selectionStart());
    if (!v) return;
    const Area area = areaOf(*v, c);
    c.beginEditBlock();
    const int count = area.r1 - area.r0 + 1;
    for (int n = 0; n < count; ++n) addRow(doc, *v, below ? area.r1 : area.r0, below);
    c.endEditBlock();
}

void insertTableColumn(WriteDoc &doc, QTextCursor &c, bool right)
{
    auto v = viewAt(doc, c.selectionStart());
    if (!v) return;
    flow::TableLayout &t = *v->layout;
    const Area area = areaOf(*v, c);
    QTextDocument *d = doc.story();
    flow::TableStore &store = doc.flow().tables;
    const int gc = right ? area.c1 + 1 : area.c0;      // the place in the grid of the new column
    const int refColumn = right ? area.c1 : area.c0;
    QVector<double> columns = columnWidthsOf(t);
    const double width = columns.value(std::min(refColumn, int(columns.size()) - 1), 72.0);
    QVector<double> wider = columns;
    wider.insert(std::min(gc, int(wider.size())), width);
    std::shared_ptr<flow::TableData> table = tableWithColumns(t.data(), wider, columns);
    c.beginEditBlock();
    // A cell in each row: before the cell that begins at the new column, or after the last; a cell the new column is inside of is made one wider.
    struct Add {
        int position;       // where the paragraph goes
        bool before;
        flow::CellRecord rec;
        QTextBlockFormat bf;
        QTextCharFormat cf;
    };
    QVector<Add> adds;
    QHash<const flow::CellRecord *, int> widened;
    QHash<const flow::RowData *, std::shared_ptr<flow::RowData>> newRows;   // rows whose gap the new column is in: the gap is a column wider and the row has no new cell
    const int was = int(columns.size());
    for (int r = 0; r < t.rowCount(); ++r) {
        const flow::TableLayout::Row &row = t.row(r);
        {
            const flow::RowData &rd = *row.data;
            const bool inBefore = rd.before > 0 && gc < rd.before, inAfter = !inBefore && rd.after > 0 && gc > was - rd.after;
            if (inBefore || inAfter) {
                newRows.insert(row.data, rowWithGaps(rd, rd.before + (inBefore ? 1 : 0), rd.after + (inAfter ? 1 : 0), wider));
                continue;
            }
        }
        int cover = -1, last = -1;
        for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
            const flow::TableLayout::Cell &cell = t.cell(i);
            last = i;
            if (cell.col <= gc && gc < cell.col + cell.span) cover = i;
        }
        if (last < 0) continue;
        if (cover >= 0 && t.cell(cover).col < gc) {
            widened.insert(t.cell(cover).rec, 1);
            continue;
        }
        // The cell that is beside the new one: the one the column begins at (or, in the gap a row leaves before its first cell, that cell), or the last.
        const bool before = cover >= 0 || gc <= t.cell(row.cell0).col;
        const flow::TableLayout::Cell &beside = t.cell(cover >= 0 ? cover : before ? row.cell0 : last);
        if (cover < 0 && !before && t.cell(last).col + t.cell(last).span < gc) continue;   // (a row that stops short of the column)
        flow::CellRecord rec = *beside.rec;
        rec.table = table;
        rec.cell.span = 1;
        rec.cell.vmerge = flow::CellData::VMerge::None;
        const CellPart *cp = static_cast<const CellPart *>(beside.rec->cell.app.get());
        auto np = std::make_shared<CellPart>();
        np->open = cellOpenFor(cp ? cp->open : QByteArray("<w:tc>"), int(std::lround(width * 20)), 1, QString());
        if (cp) np->layer = cp->layer;
        rec.cell.app = np;
        const int key = store.add(rec);
        const QTextBlock like = d->findBlock(beside.from);
        Add add;
        add.before = before;
        add.position = before ? beside.from : t.cell(last).to - 1;
        add.rec = rec;
        add.bf = emptyLike(like.blockFormat(), key);
        add.cf = markOf(like);
        adds.push_back(add);
    }
    // (From the last row up, so that where the first ones go is where it was.)
    std::sort(adds.begin(), adds.end(), [](const Add &a, const Add &b) { return a.position > b.position; });
    for (const Add &a : std::as_const(adds)) {
        const QTextBlock at = d->findBlock(a.position);
        if (a.before) insertBefore(d, at, a.bf, a.cf);
        else insertAfter(d, at, a.bf, a.cf);
    }
    // The other cells of the table have the table as it is now, the ones with the new column inside them a column more.
    const int stop = parentOf(*v);
    const int at = c.selectionStart();
    auto changed = [&](const flow::CellRecord &old, flow::CellRecord *rec) {
        const auto row = newRows.constFind(old.row.get());
        if (row != newRows.constEnd()) rec->row = row.value();
        if (!widened.contains(&old)) return;
        ++rec->cell.span;
        const CellPart *cp = static_cast<const CellPart *>(old.cell.app.get());
        // (The width the file says for it is the old and the new column's together.)
        double w = 0;
        for (int i = 0; i < t.cellCount(); ++i)
            if (t.cell(i).rec == &old) w = t.cell(i).w + width;
        auto np = std::make_shared<CellPart>(cp ? *cp : CellPart());
        np->open = cellOpenFor(cp ? cp->open : QByteArray("<w:tc>"), int(std::lround(w * 20)), rec->cell.span,
                               old.cell.vmerge == flow::CellData::VMerge::Restart ? QStringLiteral("restart") : old.cell.vmerge == flow::CellData::VMerge::Continue ? QStringLiteral("continue") : QString());
        rec->cell.app = np;
    };
    (void)at;
    Rekey rekey(doc, &t.data(), table, stop, changed);
    applyRekey(doc, &t.data(), &rekey);
    c.endEditBlock();
}

void deleteTable(WriteDoc &doc, QTextCursor &c)
{
    auto v = viewAt(doc, c.selectionStart());
    if (!v) return;
    QTextDocument *d = doc.story();
    const int stop = parentOf(*v);
    const int at = v->index.refs()[size_t(v->first)].pos;
    c.beginEditBlock();
    removeParagraphs(d, firstBlockOf(*v, d), lastBlockOf(*v, d));
    if (stop) keepCellParagraph(doc, stop, std::min(at, d->characterCount() - 1));
    keepAParagraphAfter(doc);
    c.setPosition(std::clamp(at, 0, std::max(0, d->characterCount() - 1)));
    c.endEditBlock();
}

void deleteTableRows(WriteDoc &doc, QTextCursor &c)
{
    auto v = viewAt(doc, c.selectionStart());
    if (!v) return;
    flow::TableLayout &t = *v->layout;
    const Area area = areaOf(*v, c);
    if (area.r0 == 0 && area.r1 >= t.rowCount() - 1) {
        deleteTable(doc, c);
        return;
    }
    QTextDocument *d = doc.story();
    flow::TableStore &store = doc.flow().tables;
    c.beginEditBlock();
    // A cell joined down that begins in a row that goes, and goes on below: the next row's cell is what it was (the first of the cells joined, or on its own now).
    const int after = area.r1 + 1;
    if (after < t.rowCount()) {
        const flow::TableLayout::Row &next = t.row(after);
        for (int r = area.r0; r <= area.r1; ++r) {
            const flow::TableLayout::Row &row = t.row(r);
            for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
                const flow::TableLayout::Cell &cell = t.cell(i);
                if (cell.covered || cell.rows <= 1 || cell.row + cell.rows - 1 < after) continue;
                const int left = cell.row + cell.rows - after;   // the rows of it that are left, from `after`
                for (int j = next.cell0; j < next.cell0 + next.cells; ++j) {
                    const flow::TableLayout::Cell &below = t.cell(j);
                    if (below.col != cell.col || !below.covered) continue;
                    flow::CellRecord rec = *below.rec;
                    rec.cell.vmerge = left > 1 ? flow::CellData::VMerge::Restart : flow::CellData::VMerge::None;
                    const CellPart *cp = static_cast<const CellPart *>(below.rec->cell.app.get());
                    auto np = std::make_shared<CellPart>(cp ? *cp : CellPart());
                    np->open = cellOpenFor(cp ? cp->open : QByteArray("<w:tc>"), int(std::lround(below.w * 20)), below.span, left > 1 ? QStringLiteral("restart") : QString());
                    rec.cell.app = np;
                    giveCellKey(d->findBlock(below.from), d->findBlock(below.to - 1), store.add(rec));
                }
            }
        }
    }
    // The kept pieces between the rows that go (a content control's start or end) whose partners are not among them: they go to what is left after the rows, so that a control is
    // neither cut in two nor dropped with a row. (The pieces with partners go with the rows.)
    QVector<office::wml::Block> carry;
    {
        QVector<office::wml::Block> pieces;
        for (int r = area.r0; r <= area.r1; ++r)
            if (const RowPart *rp = static_cast<const RowPart *>(t.row(r).data->app.get())) pieces += rp->before;
        QVector<int> open;
        QVector<bool> keep(pieces.size(), false);
        for (int i = 0; i < pieces.size(); ++i) {
            if (pieces[i].role == Inline::Open) {
                open.push_back(i);
            } else if (pieces[i].role == Inline::Close) {
                int at = int(open.size()) - 1;
                while (at >= 0 && pieces[open[at]].tag != pieces[i].tag) --at;
                if (at >= 0) open.resize(at);
                else keep[i] = true;
            }
        }
        for (const int i : std::as_const(open)) keep[i] = true;
        for (int i = 0; i < pieces.size(); ++i)
            if (keep[i]) carry << pieces[i];
    }
    const flow::RowData *carriedTo = after < t.rowCount() ? t.row(after).data : nullptr;
    std::shared_ptr<flow::RowData> carriedRow;
    std::shared_ptr<flow::TableData> carriedTable;
    if (!carry.isEmpty()) {
        carriedTable = std::make_shared<flow::TableData>(t.data());
        if (carriedTo) {
            carriedRow = std::make_shared<flow::RowData>(*carriedTo);
            const RowPart *rp = static_cast<const RowPart *>(carriedTo->app.get());
            auto np = std::make_shared<RowPart>(rp ? *rp : RowPart());
            np->before = carry + np->before;
            carriedRow->app = np;
        } else {
            const TablePart *tp = static_cast<const TablePart *>(t.data().app.get());
            auto np = std::make_shared<TablePart>(tp ? *tp : TablePart());
            np->after = carry + np->after;
            carriedTable->app = np;
        }
    }
    const int at = t.row(area.r0).from;
    removeParagraphs(d, d->findBlock(t.row(area.r0).from), d->findBlock(t.row(area.r1).to - 1));
    if (carriedTable) {
        const int stop = parentOf(*v);
        auto changed = [&](const flow::CellRecord &old, flow::CellRecord *rec) {
            if (carriedRow && old.row.get() == carriedTo) rec->row = carriedRow;
        };
        Rekey rekey(doc, &t.data(), carriedTable, stop, changed);
        applyRekey(doc, &t.data(), &rekey);
    }
    c.setPosition(std::clamp(at, 0, std::max(0, d->characterCount() - 1)));
    c.endEditBlock();
}

void deleteTableColumns(WriteDoc &doc, QTextCursor &c)
{
    auto v = viewAt(doc, c.selectionStart());
    if (!v) return;
    flow::TableLayout &t = *v->layout;
    const Area area = areaOf(*v, c);
    const QVector<double> columns = columnWidthsOf(t);
    if (area.c0 <= 0 && area.c1 >= columns.size() - 1) {
        deleteTable(doc, c);
        return;
    }
    QTextDocument *d = doc.story();
    const int gone = area.c1 - area.c0 + 1;
    QVector<double> narrower = columns;
    narrower.remove(area.c0, std::min(gone, int(narrower.size()) - area.c0));
    std::shared_ptr<flow::TableData> table = tableWithColumns(t.data(), narrower, columns);
    c.beginEditBlock();
    // A cell that is wholly in the columns goes, one that is partly in them has fewer; a row that has no cell left goes.
    struct Cut {
        int from, to;
    };
    QVector<Cut> cuts;
    QVector<Cut> rowsGone;
    QHash<const flow::CellRecord *, int> overlap;
    QHash<const flow::RowData *, std::shared_ptr<flow::RowData>> newRows;   // rows that lose some of a gap before or after their cells
    const int n = int(columns.size());
    auto overlapOf = [&](int a, int b) { return std::max(0, std::min(b, area.c1) - std::max(a, area.c0) + 1); };   // the columns [a, b] that are among the ones taken
    for (int r = 0; r < t.rowCount(); ++r) {
        const flow::TableLayout::Row &row = t.row(r);
        int kept = 0;
        for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
            const flow::TableLayout::Cell &cell = t.cell(i);
            const int lo = std::max(cell.col, area.c0), hi = std::min(cell.col + cell.span - 1, area.c1);
            const int count = hi - lo + 1;
            if (count <= 0) {
                ++kept;
            } else if (count >= cell.span) {
                cuts.push_back({cell.from, cell.to});
            } else {
                overlap.insert(cell.rec, count);
                ++kept;
            }
        }
        if (kept == 0) {
            rowsGone.push_back({row.from, row.to});
            continue;
        }
        const flow::RowData &rd = *row.data;
        const int lostBefore = overlapOf(0, rd.before - 1), lostAfter = overlapOf(n - rd.after, n - 1);
        if (lostBefore > 0 || lostAfter > 0) newRows.insert(row.data, rowWithGaps(rd, rd.before - lostBefore, rd.after - lostAfter, narrower));
    }
    // The cells and the rows go together, last first: a paragraph's place is where it was when everything before it is still there. (The cells in a row that goes are
    // not cut twice.)
    QVector<Cut> all = rowsGone;
    for (const Cut &g : std::as_const(cuts)) {
        bool inGoneRow = false;
        for (const Cut &r : std::as_const(rowsGone)) inGoneRow |= g.from >= r.from && g.to <= r.to;
        if (!inGoneRow) all.push_back(g);
    }
    std::sort(all.begin(), all.end(), [](const Cut &a, const Cut &b) { return a.from > b.from; });
    for (const Cut &g : std::as_const(all)) removeParagraphs(d, d->findBlock(g.from), d->findBlock(g.to - 1));
    const int stop = parentOf(*v);
    const int at = std::clamp(c.selectionStart(), 0, std::max(0, d->characterCount() - 1));
    {
        auto changed = [&](const flow::CellRecord &old, flow::CellRecord *rec) {
            const auto row = newRows.constFind(old.row.get());
            if (row != newRows.constEnd()) rec->row = row.value();
            const auto it = overlap.constFind(&old);
            if (it == overlap.constEnd()) return;
            rec->cell.span = std::max(1, rec->cell.span - it.value());
            const CellPart *cp = static_cast<const CellPart *>(old.cell.app.get());
            double w = 0;
            for (int i = 0; i < t.cellCount(); ++i)
                if (t.cell(i).rec == &old) w = t.cell(i).w;
            for (int k = 0; k < it.value() && k < gone; ++k) w -= columns.value(area.c0 + k, 0);
            auto np = std::make_shared<CellPart>(cp ? *cp : CellPart());
            np->open = cellOpenFor(cp ? cp->open : QByteArray("<w:tc>"), int(std::lround(std::max(1.0, w) * 20)), rec->cell.span,
                                   old.cell.vmerge == flow::CellData::VMerge::Restart ? QStringLiteral("restart") : old.cell.vmerge == flow::CellData::VMerge::Continue ? QStringLiteral("continue") : QString());
            rec->cell.app = np;
        };
        Rekey rekey(doc, &t.data(), table, stop, changed);
        applyRekey(doc, &t.data(), &rekey);
    }
    c.setPosition(std::clamp(at, 0, std::max(0, d->characterCount() - 1)));
    c.endEditBlock();
}

bool tabThroughCells(WriteDoc &doc, QTextCursor &c, bool back)
{
    auto v = viewAt(doc, c.position());
    if (!v) return false;
    const flow::TableLayout &t = *v->layout;
    const int i = t.cellOfPosition(c.position());
    if (i < 0) return false;
    int j = i;
    do j += back ? -1 : 1;
    while (j >= 0 && j < t.cellCount() && t.cell(j).covered);
    if (j < 0) return true;
    if (j >= t.cellCount()) {
        // After the last cell: one row after the table's last (whatever cells are joined down in it), and the cursor in its first cell.
        const int rows = t.rowCount();
        c.beginEditBlock();
        addRow(doc, *v, rows - 1, true);
        c.endEditBlock();
        if (auto fresh = viewAt(doc, c.position())) {
            const flow::TableLayout &u = *fresh->layout;
            const int r = std::min(rows, u.rowCount() - 1);
            c.setPosition(u.cell(u.row(r).cell0).from);
        }
        return true;
    }
    const flow::TableLayout::Cell &cell = t.cell(j);
    c.setPosition(cell.from);
    if (cell.to - 1 > cell.from) c.setPosition(cell.to - 1, QTextCursor::KeepAnchor);
    return true;
}


// ==== Making and changing tables
//
// A table is changed by changing the bytes of its parts (the start of the table with its properties and grid, of each row, and of each cell with its properties) and
// making the records of its cells again from them, the way the file's reader does (TableBuilder), so that what the program shows is what the file will say when it is
// opened again. The paragraphs of the cells are then given the keys of the new records.

namespace {

constexpr int kTbl = 0, kRow = 1, kCell = 2;

QByteArray twipsText(double points) { return QByteArray::number(int(std::lround(points * 20))); }

struct Patch {
    QByteArray name;    // the element's local name
    QByteArray xml;     // the whole element (empty: it is taken out); for borders and margins the group, whose sides are put among the ones it has
};

int sideRank(const QByteArray &name)
{
    static const QList<QByteArray> order = {"top", "left", "start", "bottom", "right", "end", "insideH", "insideV", "tl2br", "tr2bl"};
    const int i = int(order.indexOf(name));
    return i < 0 ? 100 : i;
}

int tblPrRank(const QByteArray &name)
{
    static const QList<QByteArray> order = {"tblStyle", "tblpPr", "tblOverlap", "bidiVisual", "tblStyleRowBandSize", "tblStyleColBandSize", "tblW", "jc", "tblCellSpacing",
                                            "tblInd", "tblBorders", "shd", "tblLayout", "tblCellMar", "tblLook", "tblCaption", "tblDescription", "tblPrChange"};
    const int i = int(order.indexOf(name));
    return i < 0 ? 100 : i;
}

int trPrRank(const QByteArray &name)
{
    static const QList<QByteArray> order = {"cnfStyle", "divId", "gridBefore", "gridAfter", "wBefore", "wAfter", "cantSplit", "trHeight", "tblHeader", "tblCellSpacing", "jc", "hidden",
                                            "ins", "del", "trPrChange"};
    const int i = int(order.indexOf(name));
    return i < 0 ? 100 : i;
}

bool isGroup(const QByteArray &name) { return name == "tblBorders" || name == "tcBorders" || name == "tcMar" || name == "tblCellMar"; }

// The start of a table, a row, or a cell with the properties in `patches` set (put among the ones it has, in the order the schema gives), and for a table the grid
// replaced by `grid` when that is not empty.
QByteArray patchPr(const QByteArray &open, int kind, const QVector<Patch> &patches, const QByteArray &grid = QByteArray())
{
    const char *outer = kind == kTbl ? "tbl" : kind == kRow ? "tr" : "tc";
    const QByteArray closing = QByteArray("</w:") + outer + ">";
    const char *prName = kind == kTbl ? "tblPr" : kind == kRow ? "trPr" : "tcPr";
    auto rank = [kind](const QByteArray &n) { return kind == kTbl ? tblPrRank(n) : kind == kRow ? trPrRank(n) : tcPrRank(n); };
    XmlTree t;
    int el = -1;
    QByteArray start = QByteArray("<w:") + outer + ">", before, others;
    QVector<std::pair<int, QByteArray>> kids;
    QHash<QByteArray, QByteArray> groups;       // the group elements the patches add to, as they are
    if (parseKept(&t, open, closing.constData(), &el)) {
        start = t.startTag(el);
        for (int c = t.firstChild(el); c >= 0; c = t.next(c)) {
            if (t.is(c, prName)) {
                for (int k = t.firstChild(c); k >= 0; k = t.next(k)) {
                    const QByteArray name = t.localName(k);
                    bool patched = false;
                    for (const Patch &p : patches)
                        if (p.name == name) patched = true;
                    if (patched && isGroup(name)) groups.insert(name, t.raw(k));
                    if (!patched) kids.push_back({rank(name), t.raw(k)});
                }
            } else if (kind == kRow && t.is(c, "tblPrEx")) {
                before += t.raw(c);
            } else if (kind == kTbl && t.is(c, "tblGrid") && !grid.isEmpty()) {
                continue;
            } else {
                others += t.raw(c);
            }
        }
    }
    for (const Patch &p : patches) {
        if (p.xml.isEmpty()) continue;
        if (!isGroup(p.name)) {
            kids.push_back({rank(p.name), p.xml});
            continue;
        }
        // The sides the element had, with the ones the patch has over them (a side the file spelled start or end is the left or the right).
        auto sidesOf = [&](const QByteArray &element) {
            XmlTree g;
            int ge = -1;
            QMap<int, QByteArray> out;
            if (!parseKept(&g, element, "", &ge)) return out;
            for (int k = g.firstChild(ge); k >= 0; k = g.next(k)) {
                const QByteArray name = g.localName(k);
                out.insert(sideRank(name == "start" ? "left" : name == "end" ? "right" : name), g.raw(k));
            }
            return out;
        };
        QMap<int, QByteArray> sides = sidesOf(groups.value(p.name));
        const QMap<int, QByteArray> mine = sidesOf(p.xml);
        for (auto it = mine.constBegin(); it != mine.constEnd(); ++it) sides.insert(it.key(), it.value());
        QByteArray element = "<w:" + p.name + ">";
        for (const QByteArray &s : std::as_const(sides)) element += s;
        element += "</w:" + p.name + ">";
        kids.push_back({rank(p.name), element});
    }
    std::stable_sort(kids.begin(), kids.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
    QByteArray out = start + before;
    if (!kids.isEmpty()) {
        out += QByteArray("<w:") + prName + ">";
        for (const auto &k : kids) out += k.second;
        out += QByteArray("</w:") + prName + ">";
    }
    return out + others + grid;
}

QByteArray shdXml(const QColor &fill)
{
    const QByteArray hex = fill.isValid() && fill.alpha() > 0 ? fill.name().mid(1).toUpper().toLatin1() : QByteArray("auto");
    return "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"" + hex + "\"/>";
}

QByteArray borderXml(const char *side, const flow::TableLine &line)
{
    if (!line.visible()) return QByteArray("<w:") + side + " w:val=\"nil\"/>";
    const char *style = line.style == flow::TableLine::Style::Double ? "double" : line.style == flow::TableLine::Style::Dotted ? "dotted" : line.style == flow::TableLine::Style::Dashed ? "dashed"
                        : line.style == flow::TableLine::Style::Triple ? "triple" : "single";
    const QByteArray color = line.color.isValid() ? line.color.name().mid(1).toUpper().toLatin1() : QByteArray("auto");
    return QByteArray("<w:") + side + " w:val=\"" + style + "\" w:sz=\"" + QByteArray::number(std::clamp(int(std::lround(line.width * 8)), 2, 96)) + "\" w:space=\"0\" w:color=\"" + color + "\"/>";
}

// ---- a table as the pieces it is written from

struct XCell {
    QByteArray open;
    std::shared_ptr<const CellPart> part;
    int key = 0;        // its record's key now (0: a cell that is new)
    int li = -1;        // its place in the layout it was read from
    int tag = 0;        // what the command that changes the table marks it with
    int like = -1;      // the position of a paragraph that the paragraph of a cell made new is like
};
struct XRow {
    QByteArray open;
    std::shared_ptr<const RowPart> part;
    QVector<XCell> cells;
};
struct XTable {
    QByteArray open;
    std::shared_ptr<const TablePart> part;
    QVector<XRow> rows;
    QVector<double> columns;
    int parent = 0;
};

XTable modelOf(const View &v)
{
    const flow::TableLayout &t = *v.layout;
    XTable x;
    x.columns = columnWidthsOf(t);
    x.parent = parentOf(v);
    x.part = std::static_pointer_cast<const TablePart>(t.data().app);
    x.open = x.part ? x.part->open : QByteArray("<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr>");
    if (!x.open.contains("<w:tblGrid")) x.open += gridXml(x.columns);
    for (int r = 0; r < t.rowCount(); ++r) {
        const flow::TableLayout::Row &row = t.row(r);
        XRow xr;
        xr.part = std::static_pointer_cast<const RowPart>(row.data->app);
        xr.open = xr.part ? xr.part->open : QByteArray("<w:tr>");
        for (int i = 0; i < row.cells; ++i) {
            const flow::TableLayout::Cell &cell = t.cell(row.cell0 + i);
            XCell xc;
            xc.part = std::static_pointer_cast<const CellPart>(cell.rec->cell.app);
            xc.open = xc.part ? xc.part->open : QByteArray("<w:tc>");
            xc.key = cell.rec->key;
            xc.li = row.cell0 + i;
            xr.cells.push_back(xc);
        }
        x.rows.push_back(xr);
    }
    return x;
}

Block structureOf(const char *tag, Inline::Role role, const QByteArray &raw, int markKind = -1)
{
    Block b;
    b.isRaw = true;
    b.role = role;
    b.tag = QLatin1String(tag);
    b.raw = raw;
    if (role == Inline::Open && markKind >= 0) b.table = std::make_shared<TableMark>(readTableMark(TableMark::Kind(markKind), raw));
    return b;
}

// The records of the table's cells made again from its parts: the key of each cell's new record, row by row.
QVector<QVector<int>> remake(WriteDoc &doc, const XTable &x)
{
    QVector<Block> body;
    body.push_back(structureOf("tbl", Inline::Open, QByteArray()));
    for (int i = 0; i < x.rows.size(); ++i) body.push_back(structureOf("tr", Inline::Open, QByteArray()));
    TableBuilder builder(&doc.flow(), &doc.styleSheet(), doc.compatMode(), body);
    QVector<Block> pending;
    if (x.part) pending = x.part->before;
    builder.take(structureOf("tbl", Inline::Open, x.open, TableMark::Table), &pending);
    QVector<QVector<int>> keys;
    for (const XRow &r : x.rows) {
        if (r.part)
            for (const Block &k : r.part->before) builder.take(k, &pending);
        builder.take(structureOf("tr", Inline::Open, r.open, TableMark::Row), &pending);
        QVector<int> rowKeys;
        for (const XCell &c : r.cells) {
            if (c.part)
                for (const Block &k : c.part->before) builder.take(k, &pending);
            builder.take(structureOf("tc", Inline::Open, c.open, TableMark::Cell), &pending);
            rowKeys.push_back(builder.key());
            if (c.part) builder.addTail(c.part->tail);
            builder.take(structureOf("tc", Inline::Close, "</w:tc>"), &pending);
        }
        if (r.part)
            for (const Block &k : r.part->after) builder.take(k, &pending);
        builder.take(structureOf("tr", Inline::Close, "</w:tr>"), &pending);
        keys.push_back(rowKeys);
    }
    if (x.part)
        for (const Block &k : x.part->after) builder.take(k, &pending);
    builder.take(structureOf("tbl", Inline::Close, "</w:tbl>"), &pending);
    if (x.parent) {
        // A table in a cell: its cells are that cell's.
        flow::TableStore &store = doc.flow().tables;
        for (QVector<int> &row : keys)
            for (int &k : row) {
                flow::CellRecord rec = *store.get(k);
                rec.parent = x.parent;
                k = store.add(rec);
            }
    }
    return keys;
}

// The keys for the paragraphs of cells that have new records: the cell, and the cells of the tables in it, which have the new key for their parent.
class Remap {
public:
    Remap(flow::TableStore *store, QHash<int, int> map) : m_store(store), m_map(std::move(map)) {}
    int keyFor(int key)
    {
        if (!key) return 0;
        const auto it = m_map.constFind(key);
        if (it != m_map.constEnd()) return it.value();
        const flow::CellRecord *rec = m_store->get(key);
        if (!rec || !rec->parent) return key;
        const int parent = keyFor(rec->parent);
        if (parent == rec->parent) return key;
        flow::CellRecord copy = *rec;
        copy.parent = parent;
        const int made = m_store->add(copy);
        m_map.insert(key, made);
        return made;
    }

private:
    flow::TableStore *m_store;
    QHash<int, int> m_map;
};

void giveKeys(WriteDoc &doc, int from, int to, const QHash<int, int> &map)
{
    QTextDocument *d = doc.story();
    Remap remap(&doc.flow().tables, map);
    for (QTextBlock b = d->findBlock(from); b.isValid() && b.position() < to; b = b.next()) {
        const int key = cellKeyOf(b);
        if (!key) continue;
        const int now = remap.keyFor(key);
        if (now == key) continue;
        QTextBlockFormat bf = b.blockFormat();
        bf.setProperty(flow::fp::Cell, now);
        QTextCursor(b).setBlockFormat(bf);
    }
}

void giveNewKeys(WriteDoc &doc, const View &v, const XTable &x, const QVector<QVector<int>> &keys)
{
    QHash<int, int> map;
    for (int r = 0; r < x.rows.size(); ++r)
        for (int i = 0; i < x.rows[r].cells.size(); ++i)
            if (x.rows[r].cells[i].key) map.insert(x.rows[r].cells[i].key, keys[r][i]);
    const std::vector<flow::BlockRef> &refs = v.index.refs();
    giveKeys(doc, refs[size_t(v.first)].pos, refs[size_t(v.last - 1)].end, map);
}

// Changes the table the cursor is in: `edit` changes the parts, and the cells are made again from them.
bool editTable(WriteDoc &doc, QTextCursor &c, const std::function<bool(XTable &, const View &, const Area &)> &edit)
{
    const auto v = viewAt(doc, c.selectionStart());
    if (!v) return false;
    const Area area = areaOf(*v, c);
    XTable x = modelOf(*v);
    if (!edit(x, *v, area)) return false;
    c.beginEditBlock();
    const QVector<QVector<int>> keys = remake(doc, x);
    giveNewKeys(doc, *v, x, keys);
    c.endEditBlock();
    return true;
}

bool hits(const flow::TableLayout::Cell &cell, const Area &a)
{
    return cell.row <= a.r1 && cell.row + cell.rows - 1 >= a.r0 && cell.col <= a.c1 && cell.col + cell.span - 1 >= a.c0;
}

void forCells(XTable &x, const View &v, const Area &a, const std::function<void(XCell &, const flow::TableLayout::Cell &)> &fn)
{
    for (XRow &r : x.rows)
        for (XCell &c : r.cells) {
            const flow::TableLayout::Cell &lc = v.layout->cell(c.li);
            if (hits(lc, a)) fn(c, lc);
        }
}

// The grid replaced, the cells' widths and the table's (when it is in points) following it.
void setColumns(XTable &x, const View &v, const QVector<double> &columns)
{
    const QVector<double> was = x.columns;
    x.columns = columns;
    const TableMark mark = readTableMark(TableMark::Table, x.open);
    QVector<Patch> patches;
    if (mark.width.stated() && mark.width.type == QLatin1String("dxa") && mark.width.w > 0)
        patches.push_back({"tblW", "<w:tblW w:w=\"" + QByteArray::number(std::max(1, mark.width.w + twipsOf(columns, 0, int(columns.size())) - twipsOf(was, 0, int(was.size())))) + "\" w:type=\"dxa\"/>"});
    x.open = patchPr(x.open, kTbl, patches, gridXml(columns));
    for (XRow &r : x.rows)
        for (XCell &c : r.cells) {
            const flow::TableLayout::Cell &lc = v.layout->cell(c.li);
            const TableMark cm = readTableMark(TableMark::Cell, c.open);
            if (cm.cellWidth.type == QLatin1String("pct")) continue;
            c.open = patchPr(c.open, kCell, {{"tcW", "<w:tcW w:w=\"" + QByteArray::number(twipsOf(columns, lc.col, lc.span)) + "\" w:type=\"dxa\"/>"}});
        }
}

// The words of the cell's paragraphs and how wide they are on one line, for fitting a column to its cells.
double naturalWidth(const QTextDocument *d, int from, int to)
{
    double most = 0;
    for (QTextBlock b = d->findBlock(from); b.isValid() && b.position() < to; b = b.next()) {
        double w = 0;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            w += QFontMetricsF(f.charFormat().font()).horizontalAdvance(f.text());
        }
        w += b.blockFormat().leftMargin() + b.blockFormat().rightMargin() + std::max(0.0, b.blockFormat().textIndent());
        most = std::max(most, w);
    }
    return most;
}

} // namespace

// ---- shading, borders, alignment

void setShading(WriteDoc &doc, QTextCursor &c, const QColor &fill)
{
    editTable(doc, c, [&](XTable &x, const View &v, const Area &a) {
        forCells(x, v, a, [&](XCell &cell, const flow::TableLayout::Cell &) { cell.open = patchPr(cell.open, kCell, {{"shd", shdXml(fill)}}); });
        return true;
    });
}

void setBorders(WriteDoc &doc, QTextCursor &c, int sides, const flow::TableLine &line)
{
    editTable(doc, c, [&](XTable &x, const View &v, const Area &a) {
        const flow::TableLayout &t = *v.layout;
        auto edge = [&](const char *side, bool on) { return on ? borderXml(side, line) : QByteArray(); };
        for (XRow &r : x.rows)
            for (XCell &cell : r.cells) {
                const flow::TableLayout::Cell &lc = t.cell(cell.li);
                const int lastRow = lc.row + lc.rows - 1, lastCol = lc.col + lc.span - 1;
                QByteArray group;
                if (hits(lc, a)) {
                    group += edge("top", (sides & BorderSide::Top && lc.row <= a.r0) || (sides & BorderSide::InsideH && lc.row > a.r0));
                    group += edge("left", (sides & BorderSide::Left && lc.col <= a.c0) || (sides & BorderSide::InsideV && lc.col > a.c0));
                    group += edge("bottom", (sides & BorderSide::Bottom && lastRow >= a.r1) || (sides & BorderSide::InsideH && lastRow < a.r1));
                    group += edge("right", (sides & BorderSide::Right && lastCol >= a.c1) || (sides & BorderSide::InsideV && lastCol < a.c1));
                } else {
                    // A cell beside the cells: its side that touches theirs is the same line.
                    const bool overAcross = lc.col <= a.c1 && lastCol >= a.c0, overDown = lc.row <= a.r1 && lastRow >= a.r0;
                    group += edge("bottom", sides & BorderSide::Top && overAcross && lastRow == a.r0 - 1);
                    group += edge("top", sides & BorderSide::Bottom && overAcross && lc.row == a.r1 + 1);
                    group += edge("right", sides & BorderSide::Left && overDown && lastCol == a.c0 - 1);
                    group += edge("left", sides & BorderSide::Right && overDown && lc.col == a.c1 + 1);
                }
                if (!group.isEmpty()) cell.open = patchPr(cell.open, kCell, {{"tcBorders", "<w:tcBorders>" + group + "</w:tcBorders>"}});
            }
        return true;
    });
}

void setCellAlignment(WriteDoc &doc, QTextCursor &c, Qt::Alignment across, flow::CellAlign down)
{
    const auto v = viewAt(doc, c.selectionStart());
    if (!v) return;
    const Area area = areaOf(*v, c);
    QVector<std::pair<int, int>> ranges;
    for (int i = 0; i < v->layout->cellCount(); ++i)
        if (hits(v->layout->cell(i), area) && !v->layout->cell(i).covered) ranges.push_back({v->layout->cell(i).from, v->layout->cell(i).to});
    c.beginEditBlock();
    editTable(doc, c, [&](XTable &x, const View &w, const Area &a) {
        const QByteArray value = down == flow::CellAlign::Center ? "center" : down == flow::CellAlign::Bottom ? "bottom" : "top";
        forCells(x, w, a, [&](XCell &cell, const flow::TableLayout::Cell &) { cell.open = patchPr(cell.open, kCell, {{"vAlign", "<w:vAlign w:val=\"" + value + "\"/>"}}); });
        return true;
    });
    for (const auto &range : std::as_const(ranges)) {
        QTextCursor cc(doc.story());
        cc.setPosition(range.first);
        cc.setPosition(std::max(range.first, range.second - 1), QTextCursor::KeepAnchor);
        setAlignment(doc, cc, across);
    }
    c.endEditBlock();
}

// ---- size

void setColumnWidth(WriteDoc &doc, QTextCursor &c, double points)
{
    editTable(doc, c, [&](XTable &x, const View &v, const Area &a) {
        QVector<double> columns = x.columns;
        for (int col = a.c0; col <= a.c1 && col < columns.size(); ++col) columns[col] = std::max(1.0, points);
        setColumns(x, v, columns);
        return true;
    });
}

void setRowHeight(WriteDoc &doc, QTextCursor &c, double points, bool exact)
{
    editTable(doc, c, [&](XTable &x, const View &, const Area &a) {
        const QByteArray height = points > 0 ? "<w:trHeight w:val=\"" + twipsText(points) + "\" w:hRule=\"" + (exact ? "exact" : "atLeast") + "\"/>" : QByteArray();
        for (int r = a.r0; r <= a.r1 && r < x.rows.size(); ++r) x.rows[r].open = patchPr(x.rows[r].open, kRow, {{"trHeight", height}});
        return true;
    });
}

void distributeColumns(WriteDoc &doc, QTextCursor &c)
{
    editTable(doc, c, [&](XTable &x, const View &v, const Area &a) {
        QVector<double> columns = x.columns;
        int c0 = a.c0, c1 = std::min<int>(a.c1, int(columns.size()) - 1);
        if (c0 >= c1) {
            c0 = 0;
            c1 = int(columns.size()) - 1;
        }
        double sum = 0;
        for (int col = c0; col <= c1; ++col) sum += columns[col];
        for (int col = c0; col <= c1; ++col) columns[col] = sum / (c1 - c0 + 1);
        setColumns(x, v, columns);
        return true;
    });
}

void distributeRows(WriteDoc &doc, QTextCursor &c)
{
    editTable(doc, c, [&](XTable &x, const View &v, const Area &a) {
        int r0 = a.r0, r1 = std::min<int>(a.r1, int(x.rows.size()) - 1);
        if (r0 >= r1) {
            r0 = 0;
            r1 = int(x.rows.size()) - 1;
        }
        double sum = 0;
        for (int r = r0; r <= r1; ++r) sum += v.layout->rowHeight(r);
        const double each = sum / (r1 - r0 + 1);
        for (int r = r0; r <= r1; ++r) x.rows[r].open = patchPr(x.rows[r].open, kRow, {{"trHeight", "<w:trHeight w:val=\"" + twipsText(each) + "\" w:hRule=\"atLeast\"/>"}});
        return true;
    });
}

double textWidthAt(WriteDoc &doc, int position)
{
    const flow::Section &s = doc.flow().section(doc.sectionAt(position));
    double w = s.orientation == flow::Section::Orientation::Landscape ? s.paper.height() : s.paper.width();
    w -= s.margins.left() + s.margins.right();
    if (s.columns > 1) w = (w - s.columnGap * (s.columns - 1)) / s.columns;
    return std::max(36.0, w);
}

void fitTable(WriteDoc &doc, QTextCursor &c, TableFit fit)
{
    const double room = textWidthAt(doc, c.selectionStart());
    editTable(doc, c, [&](XTable &x, const View &v, const Area &) {
        const flow::TableLayout &t = *v.layout;
        QVector<double> columns = x.columns;
        auto total = [&] {
            double s = 0;
            for (double w : std::as_const(columns)) s += w;
            return s;
        };
        QVector<Patch> patches;
        if (fit == TableFit::Fixed) {
            patches = {{"tblW", "<w:tblW w:w=\"" + QByteArray::number(twipsOf(columns, 0, int(columns.size()))) + "\" w:type=\"dxa\"/>"}, {"tblLayout", "<w:tblLayout w:type=\"fixed\"/>"}};
        } else if (fit == TableFit::Window) {
            const double sum = total();
            if (sum > 0)
                for (double &w : columns) w = std::max(1.0, w * (room - t.data().margins.left() - t.data().margins.right()) / sum);
            patches = {{"tblW", "<w:tblW w:w=\"5000\" w:type=\"pct\"/>"}, {"tblLayout", "<w:tblLayout w:type=\"autofit\"/>"}};
        } else {
            // To the contents: each column as wide as the widest text in a cell that is in it alone, and the whole no wider than the text area.
            QVector<double> wanted(columns.size(), 18.0);
            for (int i = 0; i < t.cellCount(); ++i) {
                const flow::TableLayout::Cell &lc = t.cell(i);
                if (lc.span != 1 || lc.col >= wanted.size()) continue;
                const double w = naturalWidth(doc.story(), lc.from, lc.to) + lc.margins.left() + lc.margins.right() + 1;
                wanted[lc.col] = std::max(wanted[lc.col], w);
            }
            double sum = 0;
            for (double w : std::as_const(wanted)) sum += w;
            if (sum > room)
                for (double &w : wanted) w = std::max(18.0, w * room / sum);
            columns = wanted;
            patches = {{"tblW", "<w:tblW w:w=\"0\" w:type=\"auto\"/>"}, {"tblLayout", "<w:tblLayout w:type=\"autofit\"/>"}};
        }
        x.open = patchPr(x.open, kTbl, patches);
        // (the table's width is said by `patches` now: the grid is set after, and does not change a width that is not in points)
        setColumns(x, v, columns);
        return true;
    });
}


// ---- Table Properties, the table's style and its options

namespace {

QString textOfElement(const QByteArray &open, const char *name)
{
    XmlTree t;
    int tbl = -1;
    if (!parseKept(&t, open, "</w:tbl>", &tbl)) return QString();
    const int pr = t.child(tbl, "tblPr");
    const int e = pr >= 0 ? t.child(pr, name) : -1;
    return e >= 0 ? t.val(e) : QString();
}

QByteArray lookXml(int look)
{
    auto bit = [&](int b) { return look & b ? "1" : "0"; };
    char hex[8];
    std::snprintf(hex, sizeof hex, "%04X", look);
    return QByteArray("<w:tblLook w:val=\"") + hex + "\" w:firstRow=\"" + bit(0x20) + "\" w:lastRow=\"" + bit(0x40) + "\" w:firstColumn=\"" + bit(0x80) + "\" w:lastColumn=\"" + bit(0x100) +
           "\" w:noHBand=\"" + bit(0x200) + "\" w:noVBand=\"" + bit(0x400) + "\"/>";
}

} // namespace

bool TableProps::operator==(const TableProps &o) const
{
    return align == o.align && indent == o.indent && widthKind == o.widthKind && width == o.width && fixedLayout == o.fixedLayout && margins == o.margins && spacing == o.spacing &&
           altTitle == o.altTitle && altText == o.altText && style == o.style && look == o.look && rowHeight == o.rowHeight && rowExact == o.rowExact && cantSplit == o.cantSplit &&
           header == o.header && columnWidth == o.columnWidth && valign == o.valign && noWrap == o.noWrap;
}

TableProps tablePropsAt(WriteDoc &doc, const QTextCursor &c)
{
    TableProps p;
    const auto v = viewAt(doc, c.selectionStart());
    if (!v) return p;
    const Area a = areaOf(*v, c);
    const XTable x = modelOf(*v);
    const TableMark m = readTableMark(TableMark::Table, x.open);
    const flow::TableData &data = v->layout->data();
    p.align = data.align;
    p.indent = data.indent;
    if (m.width.type == QLatin1String("pct")) {
        p.widthKind = TableProps::Width::Percent;
        p.width = m.width.w / 50.0;
    } else if (m.width.type == QLatin1String("dxa") && m.width.w > 0) {
        p.widthKind = TableProps::Width::Points;
        p.width = m.width.w / 20.0;
    }
    p.fixedLayout = m.layout == QLatin1String("fixed");
    p.margins = data.margins;
    p.spacing = data.spacing;
    p.altTitle = textOfElement(x.open, "tblCaption");
    p.altText = textOfElement(x.open, "tblDescription");
    p.style = m.style;
    p.look = m.look;
    if (a.r0 < x.rows.size()) {
        const TableMark r = readTableMark(TableMark::Row, x.rows[a.r0].open);
        p.rowHeight = r.height / 20.0;
        p.rowExact = r.heightRule == QLatin1String("exact");
        p.cantSplit = r.cantSplit;
        p.header = r.header;
    }
    p.columnWidth = a.c0 < x.columns.size() ? x.columns[a.c0] : 0;
    for (int i = 0; i < v->layout->cellCount(); ++i)
        if (hits(v->layout->cell(i), a)) {
            p.valign = v->layout->cell(i).rec->cell.valign;
            p.noWrap = v->layout->cell(i).rec->cell.noWrap;
            break;
        }
    return p;
}

void setTableProps(WriteDoc &doc, QTextCursor &c, const TableProps &was, const TableProps &now)
{
    if (!now.style.isEmpty() && now.style != was.style) doc.ensureStyle(now.style);
    editTable(doc, c, [&](XTable &x, const View &v, const Area &a) {
        QVector<Patch> table;
        if (now.align != was.align)
            table.push_back({"jc", QByteArray("<w:jc w:val=\"") + (now.align == flow::TableData::Align::Center ? "center" : now.align == flow::TableData::Align::Right ? "right" : "left") + "\"/>"});
        if (now.indent != was.indent) table.push_back({"tblInd", now.indent != 0 ? "<w:tblInd w:w=\"" + twipsText(now.indent) + "\" w:type=\"dxa\"/>" : QByteArray()});
        if (now.widthKind != was.widthKind || now.width != was.width) {
            const QByteArray w = now.widthKind == TableProps::Width::Percent ? "<w:tblW w:w=\"" + QByteArray::number(int(std::lround(now.width * 50))) + "\" w:type=\"pct\"/>"
                                 : now.widthKind == TableProps::Width::Points ? "<w:tblW w:w=\"" + twipsText(now.width) + "\" w:type=\"dxa\"/>"
                                                                              : QByteArray("<w:tblW w:w=\"0\" w:type=\"auto\"/>");
            table.push_back({"tblW", w});
        }
        if (now.fixedLayout != was.fixedLayout) table.push_back({"tblLayout", QByteArray("<w:tblLayout w:type=\"") + (now.fixedLayout ? "fixed" : "autofit") + "\"/>"});
        if (now.margins != was.margins)
            table.push_back({"tblCellMar", "<w:tblCellMar><w:top w:w=\"" + twipsText(now.margins.top()) + "\" w:type=\"dxa\"/><w:left w:w=\"" + twipsText(now.margins.left()) +
                                               "\" w:type=\"dxa\"/><w:bottom w:w=\"" + twipsText(now.margins.bottom()) + "\" w:type=\"dxa\"/><w:right w:w=\"" + twipsText(now.margins.right()) +
                                               "\" w:type=\"dxa\"/></w:tblCellMar>"});
        if (now.spacing != was.spacing) table.push_back({"tblCellSpacing", now.spacing > 0 ? "<w:tblCellSpacing w:w=\"" + twipsText(now.spacing) + "\" w:type=\"dxa\"/>" : QByteArray()});
        if (now.altTitle != was.altTitle) table.push_back({"tblCaption", now.altTitle.isEmpty() ? QByteArray() : "<w:tblCaption w:val=\"" + escapeXml(now.altTitle, true) + "\"/>"});
        if (now.altText != was.altText) table.push_back({"tblDescription", now.altText.isEmpty() ? QByteArray() : "<w:tblDescription w:val=\"" + escapeXml(now.altText, true) + "\"/>"});
        if (now.style != was.style) table.push_back({"tblStyle", now.style.isEmpty() ? QByteArray() : "<w:tblStyle w:val=\"" + escapeXml(now.style, true) + "\"/>"});
        if (now.look != was.look) table.push_back({"tblLook", lookXml(now.look)});
        if (!table.isEmpty()) x.open = patchPr(x.open, kTbl, table);
        QVector<Patch> row;
        if (now.rowHeight != was.rowHeight || now.rowExact != was.rowExact)
            row.push_back({"trHeight", now.rowHeight > 0 ? "<w:trHeight w:val=\"" + twipsText(now.rowHeight) + "\" w:hRule=\"" + (now.rowExact ? "exact" : "atLeast") + "\"/>" : QByteArray()});
        if (now.cantSplit != was.cantSplit) row.push_back({"cantSplit", now.cantSplit ? "<w:cantSplit/>" : QByteArray()});
        if (now.header != was.header) row.push_back({"tblHeader", now.header ? "<w:tblHeader/>" : QByteArray()});
        if (!row.isEmpty())
            for (int r = a.r0; r <= a.r1 && r < x.rows.size(); ++r) x.rows[r].open = patchPr(x.rows[r].open, kRow, row);
        QVector<Patch> cell;
        if (now.valign != was.valign) cell.push_back({"vAlign", QByteArray("<w:vAlign w:val=\"") + (now.valign == flow::CellAlign::Center ? "center" : now.valign == flow::CellAlign::Bottom ? "bottom" : "top") + "\"/>"});
        if (now.noWrap != was.noWrap) cell.push_back({"noWrap", now.noWrap ? "<w:noWrap/>" : QByteArray()});
        if (!cell.isEmpty()) forCells(x, v, a, [&](XCell &cc, const flow::TableLayout::Cell &) { cc.open = patchPr(cc.open, kCell, cell); });
        if (now.columnWidth != was.columnWidth && now.columnWidth > 0) {
            QVector<double> columns = x.columns;
            for (int col = a.c0; col <= a.c1 && col < columns.size(); ++col) columns[col] = now.columnWidth;
            setColumns(x, v, columns);
        }
        return true;
    });
}

void setTableStyle(WriteDoc &doc, QTextCursor &c, const QString &styleId)
{
    if (!styleId.isEmpty()) doc.ensureStyle(styleId);
    editTable(doc, c, [&](XTable &x, const View &, const Area &) {
        x.open = patchPr(x.open, kTbl, {{"tblStyle", styleId.isEmpty() ? QByteArray() : "<w:tblStyle w:val=\"" + escapeXml(styleId, true) + "\"/>"}});
        return true;
    });
}

void setTableLook(WriteDoc &doc, QTextCursor &c, int mask, bool on)
{
    editTable(doc, c, [&](XTable &x, const View &, const Area &) {
        int look = readTableMark(TableMark::Table, x.open).look;
        look = on ? (look | mask) : (look & ~mask);
        x.open = patchPr(x.open, kTbl, {{"tblLook", lookXml(look)}});
        return true;
    });
}

// ---- a new table

bool insertTable(WriteDoc &doc, QTextCursor &c, const NewTable &table)
{
    if (table.rows < 1 || table.columns < 1 || table.rows > 500 || table.columns > 63) return false;
    QTextDocument *d = doc.story();
    if (c.document() != d || cellKeyOf(d->findBlock(c.selectionStart()))) return false;
    c.beginEditBlock();
    c.setPosition(c.selectionStart());
    if (c.position() > c.block().position()) insertParagraph(doc, c);
    const QTextBlock after = c.block();
    if (!table.style.isEmpty()) doc.ensureStyle(table.style);
    const double room = textWidthAt(doc, after.position());
    const double each = table.columnWidth > 0 ? table.columnWidth : room / table.columns;
    QVector<double> columns(table.columns, each);
    if (table.fit == TableFit::Window && table.columnWidth <= 0) columns = QVector<double>(table.columns, room / table.columns);
    XTable x;
    x.columns = columns;
    QByteArray pr;
    if (!table.style.isEmpty()) pr += "<w:tblStyle w:val=\"" + escapeXml(table.style, true) + "\"/>";
    if (table.fit == TableFit::Window) pr += "<w:tblW w:w=\"5000\" w:type=\"pct\"/>";
    else if (table.fit == TableFit::Fixed) pr += "<w:tblW w:w=\"" + QByteArray::number(twipsOf(columns, 0, table.columns)) + "\" w:type=\"dxa\"/>";
    else pr += "<w:tblW w:w=\"0\" w:type=\"auto\"/>";
    pr += table.fit == TableFit::Fixed ? "<w:tblLayout w:type=\"fixed\"/>" : "<w:tblLayout w:type=\"autofit\"/>";
    pr += lookXml(0x04A0);
    x.open = "<w:tbl><w:tblPr>" + pr + "</w:tblPr>" + gridXml(columns);
    for (int r = 0; r < table.rows; ++r) {
        XRow row;
        row.open = "<w:tr>";
        for (int col = 0; col < table.columns; ++col) {
            XCell cell;
            cell.open = "<w:tc><w:tcPr><w:tcW w:w=\"" + QByteArray::number(twipsOf(columns, col, 1)) + "\" w:type=\"dxa\"/></w:tcPr>";
            row.cells.push_back(cell);
        }
        x.rows.push_back(row);
    }
    const QVector<QVector<int>> keys = remake(doc, x);
    // The paragraphs go in before the one the cursor was in, which is the story's first when that is where the cursor is: what the first paragraph holds goes to the new first.
    const QString style = doc.defaultParagraphStyle();
    const flow::TableStore &store = doc.flow().tables;
    const bool atStart = !after.previous().isValid();
    const QTextBlockFormat head = after.blockFormat();
    static const int kHeadProps[] = {prop::RawBefore, prop::ParaAttrs, prop::LastSection, prop::MarkFormat, flow::fp::AdvanceTop};
    int at = after.position();
    const int first = at;
    for (const QVector<int> &row : keys)
        for (int key : row) {
            const CellPart *part = static_cast<const CellPart *>(store.get(key)->cell.app.get());
            QTextBlockFormat bf = doc.styleBlockFormat(style, part ? part->layer.get() : nullptr);
            bf.setProperty(flow::fp::Cell, key);
            if (atStart && at == first)
                for (int k : kHeadProps)
                    if (head.hasProperty(k)) bf.setProperty(k, head.property(k));
            insertBefore(d, d->findBlock(at), bf, doc.styleCharFormat(style, QString(), part ? part->layer.get() : nullptr));
            ++at;
        }
    if (atStart) {
        QTextBlockFormat bf = d->findBlock(at).blockFormat();
        for (int k : kHeadProps) bf.clearProperty(k);
        QTextCursor(d->findBlock(at)).setBlockFormat(bf);
    }
    c.setPosition(first);
    c.endEditBlock();
    return true;
}


// ---- merging and splitting cells

namespace {

// The area made to hold whole the cells it touches (a cell that is part in and part out is wholly in).
Area closedArea(const View &v, Area a)
{
    bool grew = true;
    while (grew) {
        grew = false;
        for (int i = 0; i < v.layout->cellCount(); ++i) {
            const flow::TableLayout::Cell &lc = v.layout->cell(i);
            if (!hits(lc, a)) continue;
            const Area b{std::min(a.r0, lc.row), std::max(a.r1, lc.row + lc.rows - 1), std::min(a.c0, lc.col), std::max(a.c1, lc.col + lc.span - 1)};
            if (b.r0 != a.r0 || b.r1 != a.r1 || b.c0 != a.c0 || b.c1 != a.c1) {
                a = b;
                grew = true;
            }
        }
    }
    return a;
}

bool cellIsEmpty(const QTextDocument *d, int from, int to)
{
    for (QTextBlock b = d->findBlock(from); b.isValid() && b.position() < to; b = b.next())
        if (b.length() > 1) return false;
    return true;
}

// The start of a cell with its width, span, and merge set as given (the span from `col`, in the grid `columns`).
QByteArray cellWith(const QByteArray &open, const QVector<double> &columns, int col, int span, const QString &vmerge)
{
    return patchPr(open, kCell,
                   {{"tcW", "<w:tcW w:w=\"" + QByteArray::number(twipsOf(columns, col, span)) + "\" w:type=\"dxa\"/>"},
                    {"gridSpan", span > 1 ? "<w:gridSpan w:val=\"" + QByteArray::number(span) + "\"/>" : QByteArray()},
                    {"vMerge", vmerge == QLatin1String("restart") ? QByteArray("<w:vMerge w:val=\"restart\"/>") : vmerge == QLatin1String("continue") ? QByteArray("<w:vMerge/>") : QByteArray()}});
}

} // namespace

bool canMergeCells(WriteDoc &doc, const QTextCursor &c)
{
    const auto v = viewAt(doc, c.selectionStart());
    if (!v) return false;
    const Area a = closedArea(*v, areaOf(*v, c));
    int real = 0;
    for (int i = 0; i < v->layout->cellCount(); ++i)
        if (hits(v->layout->cell(i), a) && !v->layout->cell(i).covered) ++real;
    return real > 1;
}

bool mergeCells(WriteDoc &doc, QTextCursor &c)
{
    const auto v = viewAt(doc, c.selectionStart());
    if (!v) return false;
    const Area a = closedArea(*v, areaOf(*v, c));
    const flow::TableLayout &t = *v->layout;
    QTextDocument *d = doc.story();
    // The cells in the area, row by row (a cell joined down is in the row it begins in, and the rows under it have its continuations).
    QVector<QVector<int>> inRow(a.r1 - a.r0 + 1);
    int real = 0;
    for (int i = 0; i < t.cellCount(); ++i) {
        const flow::TableLayout::Cell &lc = t.cell(i);
        if (!hits(lc, a)) continue;
        if (!lc.covered) ++real;
        inRow[lc.row - a.r0].push_back(i);
    }
    if (real < 2) return false;
    const int topLeft = inRow[0].first();
    // What the other cells hold, to be put after what the first has: the cells with something in them, in reading order.
    QVector<QTextDocumentFragment> carry;
    for (const QVector<int> &mine : std::as_const(inRow))
        for (int li : mine) {
            const flow::TableLayout::Cell &lc = t.cell(li);
            if (li == topLeft || lc.covered || cellIsEmpty(d, lc.from, lc.to)) continue;
            QTextCursor sel(d);
            sel.setPosition(lc.from);
            sel.setPosition(lc.to - 1, QTextCursor::KeepAnchor);
            carry.push_back(sel.selection());
        }
    // The parts: one cell for the area in each of its rows.
    XTable x = modelOf(*v);
    QVector<int> at(inRow.size(), -1);
    for (int r = a.r0; r <= a.r1; ++r) {
        XRow &row = x.rows[r];
        const QVector<int> &mine = inRow[r - a.r0];
        for (int i = 0; i < row.cells.size(); ++i)
            if (row.cells[i].li == mine.first()) at[r - a.r0] = i;
        XCell cell = row.cells[at[r - a.r0]];
        cell.open = cellWith(cell.open, x.columns, a.c0, a.c1 - a.c0 + 1, r == a.r0 ? (a.r1 > a.r0 ? QStringLiteral("restart") : QString()) : QStringLiteral("continue"));
        cell.tag = 1;
        row.cells.erase(row.cells.begin() + at[r - a.r0], row.cells.begin() + at[r - a.r0] + mine.size());
        row.cells.insert(at[r - a.r0], cell);
    }
    c.beginEditBlock();
    const QVector<QVector<int>> keys = remake(doc, x);
    const int before = d->characterCount();
    const std::vector<flow::BlockRef> &refs = v->index.refs();
    const int tableFrom = refs[size_t(v->first)].pos, tableEnd = refs[size_t(v->last - 1)].end;
    // The old keys that become the new ones: the cells that stay, and the ones that are joined into a row's one.
    QHash<int, int> map;
    for (int r = 0; r < x.rows.size(); ++r)
        for (int i = 0; i < x.rows[r].cells.size(); ++i)
            if (x.rows[r].cells[i].key) map.insert(x.rows[r].cells[i].key, keys[r][i]);
    const int topKey = keys[a.r0][at[0]];
    for (int r = a.r0; r <= a.r1; ++r)
        for (int li : inRow[r - a.r0]) map.insert(t.cell(li).rec->key, keys[r][at[r - a.r0]]);
    // The rows below the first lose their cells' paragraphs but one, which is empty.
    for (int r = a.r1; r > a.r0; --r) {
        const QVector<int> &mine = inRow[r - a.r0];
        const int from = t.cell(mine.first()).from, to = t.cell(mine.last()).to;
        QTextBlock first = d->findBlock(from);
        const QTextBlock lastOfRow = d->findBlock(to - 1);
        if (first.length() > 1) {
            QTextCursor wipe(d);
            wipe.setPosition(first.position());
            wipe.setPosition(first.position() + first.length() - 1, QTextCursor::KeepAnchor);
            wipe.removeSelectedText();
        }
        first = d->findBlock(from);
        if (lastOfRow != first) removeParagraphs(d, first.next(), lastOfRow);
    }
    {
        // The first row: the other cells' paragraphs go, and what they held is put after the first cell's.
        const QVector<int> &mine = inRow[0];
        const flow::TableLayout::Cell &firstCell = t.cell(mine.first());
        if (mine.size() > 1) removeParagraphs(d, d->findBlock(firstCell.to), d->findBlock(t.cell(mine.last()).to - 1));
        if (!carry.isEmpty()) {
            const QTextBlock last = d->findBlock(firstCell.to - 1);
            const QTextBlockFormat bf = last.blockFormat();
            const QTextCharFormat cf = last.charFormat();
            const bool onlyEmpty = cellIsEmpty(d, firstCell.from, firstCell.to);
            QTextCursor put(d);
            int from;
            if (onlyEmpty) {
                // (A first cell with nothing in it has the first thing carried in its one paragraph.)
                from = last.position();
                put.setPosition(from);
            } else {
                put.setPosition(last.position() + last.length() - 1);
                put.insertBlock(bf, cf);
                from = put.position();
            }
            for (int i = 0; i < carry.size(); ++i) {
                if (i > 0) put.insertBlock(bf, cf);
                put.insertFragment(carry[i]);
            }
            for (QTextBlock b = d->findBlock(from); b.isValid() && b.position() <= put.position(); b = b.next()) {
                QTextBlockFormat f = b.blockFormat();
                f.setProperty(flow::fp::Cell, topKey);
                QTextCursor(b).setBlockFormat(f);
            }
        }
    }
    giveKeys(doc, tableFrom, tableEnd + (d->characterCount() - before), map);
    c.setPosition(t.cell(topLeft).from);
    c.endEditBlock();
    return true;
}

bool canSplitCell(WriteDoc &doc, const QTextCursor &c)
{
    const auto v = viewAt(doc, c.selectionStart());
    if (!v) return false;
    const int li = v->layout->cellOfPosition(c.selectionStart());
    return li >= 0 && !v->layout->cell(li).covered;
}

bool splitCells(WriteDoc &doc, QTextCursor &c, int columns, int rows)
{
    if (columns < 1 || rows < 1 || columns > 63 || rows > 500 || columns * rows < 2) return false;
    const auto v = viewAt(doc, c.selectionStart());
    if (!v) return false;
    const flow::TableLayout &t = *v->layout;
    const int li = t.cellOfPosition(c.selectionStart());
    if (li < 0 || t.cell(li).covered) return false;
    const flow::TableLayout::Cell L = t.cell(li);
    const int R = L.rows, n = columns, m = rows;
    if (m > 1 && R > 1 && m != R) return false;      // (a cell joined down is split into as many rows as it joins, or kept joined)
    QTextDocument *d = doc.story();
    XTable x = modelOf(*v);
    // The grid: a cell split into more columns than it takes gets columns more, made from the widest it takes.
    QVector<double> grid = x.columns;
    int extra = 0, widest = L.col;
    if (n > L.span) {
        extra = n - L.span;
        for (int g = L.col; g < L.col + L.span; ++g)
            if (grid[g] > grid[widest]) widest = g;
        // (in whole twentieths of a point, as the file has them: the first has what is left)
        const double share = std::round(grid[widest] * 20.0 / (extra + 1)) / 20.0;
        grid[widest] -= share * extra;
        for (int k = 0; k < extra; ++k) grid.insert(widest + 1, share);
    }
    struct Place { int col, span; };
    auto placeOf = [&](const flow::TableLayout::Cell &lc) {
        Place p{lc.col, lc.span};
        if (extra > 0) {
            if (lc.col > widest) p.col += extra;
            else if (lc.col + lc.span - 1 >= widest) p.span += extra;
        }
        return p;
    };
    // The cells that are not split follow the grid.
    for (XRow &row : x.rows)
        for (XCell &cell : row.cells) {
            const flow::TableLayout::Cell &lc = t.cell(cell.li);
            const Place p = placeOf(lc);
            const TableMark cm = readTableMark(TableMark::Cell, cell.open);
            QVector<Patch> ps;
            if (cm.cellWidth.type != QLatin1String("pct")) ps.push_back({"tcW", "<w:tcW w:w=\"" + QByteArray::number(twipsOf(grid, p.col, p.span)) + "\" w:type=\"dxa\"/>"});
            if (p.span != lc.span) ps.push_back({"gridSpan", p.span > 1 ? "<w:gridSpan w:val=\"" + QByteArray::number(p.span) + "\"/>" : QByteArray()});
            if (!ps.isEmpty()) cell.open = patchPr(cell.open, kCell, ps);
            cell.like = lc.from;
        }
    if (extra > 0) {
        x.open = patchPr(x.open, kTbl, {}, gridXml(grid));
        x.columns = grid;
    }
    QVector<int> spans;
    if (n <= L.span) {
        for (int k = 0; k < n; ++k) spans << L.span / n + (k < L.span % n ? 1 : 0);
    } else {
        spans = QVector<int>(n, 1);
    }
    // What is to be put in: each group is paragraphs that go in at a place of the story, in the order of `order` (a later one goes after an earlier one at the same place).
    struct Group {
        int pos = 0, order = 0;
        QVector<std::pair<int, int>> cells;     // [row, index] in the new parts
    };
    QVector<Group> groups;
    const int newCol = placeOf(L).col;
    auto splitCellsOf = [&](XRow &row, int at, const XCell &base, const QString &vmerge, int rowIndex, int pos, int order) {
        row.cells.removeAt(at);
        Group g;
        g.pos = pos;
        g.order = order;
        int col = newCol;
        for (int k = 0; k < n; ++k) {
            XCell cell = base;
            cell.key = k == 0 ? base.key : 0;
            cell.li = k == 0 ? base.li : -1;
            if (k > 0) cell.part.reset();
            cell.open = cellWith(base.open, grid, col, spans[k], vmerge);
            col += spans[k];
            row.cells.insert(at + k, cell);
            if (k > 0) g.cells.push_back({rowIndex, at + k});
        }
        if (!g.cells.isEmpty()) groups.push_back(g);
    };
    // The rows the cell covers: its cell in each is replaced by the new ones.
    for (int r = L.row; r < L.row + R; ++r) {
        XRow &row = x.rows[r];
        int at = -1;
        for (int i = 0; i < row.cells.size(); ++i) {
            const flow::TableLayout::Cell &lc = t.cell(row.cells[i].li);
            if (lc.col == L.col && lc.span == L.span) at = i;
        }
        if (at < 0) return false;
        const XCell base = row.cells[at];
        QString vmerge;
        if (m == 1 && R > 1) vmerge = r == L.row ? QStringLiteral("restart") : QStringLiteral("continue");
        splitCellsOf(row, at, base, vmerge, r, t.cell(base.li).to, r * 2);
    }
    // A cell of one row split into rows: rows under its row, in which the cells beside it go on (they are joined down through them).
    if (m > 1 && R == 1) {
        XRow &src = x.rows[L.row];
        QVector<XRow> added;
        int firstAt = -1;
        for (int i = 0; i < src.cells.size(); ++i)
            if (src.cells[i].li == li) firstAt = i;
        for (int j = 0; j < m - 1; ++j) {
            XRow row;
            row.open = patchPr(rowOpenFor(src.open), kRow, {{"trHeight", QByteArray()}});
            Group g;
            g.pos = t.row(L.row).to;
            g.order = 1000 + j;
            int col = newCol;
            for (int i = 0; i < src.cells.size(); ++i) {
                XCell cell = src.cells[i];
                cell.key = 0;
                cell.part.reset();
                const bool mine = i >= firstAt && i < firstAt + n;
                if (mine) {
                    cell.li = -1;
                    cell.open = cellWith(cell.open, grid, col, spans[i - firstAt], QString());
                    col += spans[i - firstAt];
                    cell.like = L.from;
                } else {
                    // (a cell beside the split one: it begins a join in the row, and continues it here)
                    const flow::TableLayout::Cell &lc = t.cell(cell.li);
                    const Place p = placeOf(lc);
                    cell.open = cellWith(cell.open, grid, p.col, p.span, QStringLiteral("continue"));
                    cell.li = -1;
                }
                row.cells.push_back(cell);
                g.cells.push_back({L.row + 1 + j, i});
            }
            added.push_back(row);
            groups.push_back(g);
        }
        for (int i = 0; i < src.cells.size(); ++i) {
            if (i >= firstAt && i < firstAt + n) continue;
            const flow::TableLayout::Cell &lc = t.cell(src.cells[i].li);
            if (lc.rows == 1 && !lc.covered) {
                const Place p = placeOf(lc);
                src.cells[i].open = cellWith(src.cells[i].open, grid, p.col, p.span, QStringLiteral("restart"));
            }
        }
        for (int j = 0; j < added.size(); ++j) x.rows.insert(L.row + 1 + j, added[j]);
    }
    c.beginEditBlock();
    const QVector<QVector<int>> keys = remake(doc, x);
    const int before = d->characterCount();
    const std::vector<flow::BlockRef> &refs = v->index.refs();
    const int tableFrom = refs[size_t(v->first)].pos, tableEnd = refs[size_t(v->last - 1)].end;
    QHash<int, int> map;
    for (int r = 0; r < x.rows.size(); ++r)
        for (int i = 0; i < x.rows[r].cells.size(); ++i)
            if (x.rows[r].cells[i].key) map.insert(x.rows[r].cells[i].key, keys[r][i]);
    // The new paragraphs, the later places first (so that the places of the ones before are as they were).
    std::sort(groups.begin(), groups.end(), [](const Group &a, const Group &b) { return a.pos != b.pos ? a.pos > b.pos : a.order > b.order; });
    for (const Group &g : std::as_const(groups)) {
        int at = g.pos;
        for (const auto &place : g.cells) {
            const XCell &cell = x.rows[place.first].cells[place.second];
            const QTextBlock like = d->findBlock(cell.like >= 0 ? cell.like : L.from);
            insertBefore(d, d->findBlock(at), emptyLike(like.blockFormat(), keys[place.first][place.second]), markOf(like));
            ++at;
        }
    }
    giveKeys(doc, tableFrom, tableEnd + (d->characterCount() - before), map);
    c.setPosition(L.from);
    c.endEditBlock();
    return true;
}


void selectInTable(WriteDoc &doc, QTextCursor &c, TableUnit unit)
{
    const auto v = viewAt(doc, c.selectionStart());
    if (!v) return;
    const flow::TableLayout &t = *v->layout;
    const int li = t.cellOfPosition(c.selectionStart());
    if (li < 0) return;
    const flow::TableLayout::Cell here = t.cell(li);
    int first = li, last = li;
    if (unit == TableUnit::Row) {
        const flow::TableLayout::Row &row = t.row(here.row);
        first = row.cell0;
        last = row.cell0 + row.cells - 1;
    } else if (unit == TableUnit::Column) {
        first = -1;
        for (int i = 0; i < t.cellCount(); ++i)
            if (t.cell(i).col <= here.col && t.cell(i).col + t.cell(i).span > here.col) {
                if (first < 0) first = i;
                last = i;
            }
    } else if (unit == TableUnit::Table) {
        first = 0;
        last = t.cellCount() - 1;
    }
    c.setPosition(t.cell(first).from);
    c.setPosition(std::max(t.cell(last).from, t.cell(last).to - 1), QTextCursor::KeepAnchor);
}

} // namespace jp::write
