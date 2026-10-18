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
#include <algorithm>
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
    if (c.hasSelection() && b > a && d->findBlock(b).position() == b) --b;   // (the selection ends where a paragraph begins: it is not in that one)
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
    const int at = t.row(area.r0).from;
    removeParagraphs(d, d->findBlock(t.row(area.r0).from), d->findBlock(t.row(area.r1).to - 1));
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

} // namespace jp::write
