#include "flow/tableflow.h"

#include "text/textengine.h"

#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>
#include <cmath>
#include <set>

namespace jp::flow {

namespace {

// A frame is this tall for a cell's paragraphs: more than any cell takes (a cell is not cut; a row is as tall as what is in it).
constexpr double kTall = 100000;
// The smallest width a cell's text area has.
constexpr double kLeastWidth = 1;
// The most columns a table's grid has (the reader of a file holds to as many).
constexpr int kMostGridColumns = 256;

} // namespace

// ---- finding the tables

const QVector<const CellRecord *> &TableIndex::chain(int key) const
{
    auto it = m_chains.find(key);
    if (it != m_chains.end()) return it.value();
    QVector<const CellRecord *> up;
    for (const CellRecord *r = m_store ? m_store->get(key) : nullptr; r && up.size() < 64; r = m_store->get(r->parent)) up.push_back(r);
    std::reverse(up.begin(), up.end());
    return m_chains.insert(key, up).value();
}

void TableIndex::rebuild(const QTextDocument *story, const TableStore &store)
{
    m_spans.clear();
    m_refs.clear();
    m_chains.clear();
    m_store = &store;
    if (store.isEmpty()) return;
    const TableData *current = nullptr;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        const int key = b.blockFormat().intProperty(fp::Cell);
        if (!store.get(key)) {
            current = nullptr;
            continue;
        }
        const QVector<const CellRecord *> &up = chain(key);
        const TableData *outer = up.first()->table.get();
        if (outer != current) {
            current = outer;
            Span s;
            s.from = b.position();
            s.first = int(m_refs.size());
            s.table = up.first()->table;
            m_spans.push_back(s);
        }
        m_refs.push_back({b.position(), b.position() + b.length(), key});
        m_spans.back().to = b.position() + b.length();
        m_spans.back().last = int(m_refs.size());
    }
}

int TableIndex::spanAt(int pos) const
{
    const auto it = std::upper_bound(m_spans.begin(), m_spans.end(), pos, [](int p, const Span &s) { return p < s.from; });
    if (it == m_spans.begin()) return -1;
    const int i = int(it - m_spans.begin()) - 1;
    return pos < m_spans[size_t(i)].to ? i : -1;
}

int TableIndex::firstFrom(int pos) const
{
    return int(std::lower_bound(m_spans.begin(), m_spans.end(), pos, [](const Span &s, int p) { return s.from < p; }) - m_spans.begin());
}

// ---- laying out a table

namespace {

// A stretch of paragraphs [from, to) of the story laid out at a width, from the top, and the height they take (the space after the last one counts).
std::shared_ptr<StoryLayout> layoutText(FlowDocument *doc, int from, int to, double width, double *height)
{
    FrameSpec fs;
    fs.size = QSizeF(std::max(kLeastWidth, width), kTall);
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.columns = 1;
    fs.hyphenate = doc->hyphenate;
    fs.hyphenZone = doc->hyphenZone;
    fs.ctx.doc = doc->fields;
    StoryLayout::Span span;
    span.from = from;
    span.end = to;
    span.keepRules = false;
    auto layout = std::make_shared<StoryLayout>();
    layout->build(doc->story(), {fs}, doc->env, span);
    double bottom = 0;
    int lastBlock = -1;
    for (const StoryLayout::LineBrief &l : layout->lineBriefs()) {
        if (l.frame < 0) continue;
        bottom = std::max(bottom, l.rect.bottom());
        lastBlock = l.block;
    }
    if (lastBlock >= 0) bottom += std::max(0.0, doc->story()->findBlock(lastBlock).blockFormat().bottomMargin());
    *height = bottom;
    return layout;
}

std::shared_ptr<TableFragment> wholeFragment(const std::shared_ptr<TableLayout> &layout);

} // namespace

std::shared_ptr<CellContent> layoutCellContent(const TableContext &ctx, int i0, int i1, int direct, double width)
{
    auto out = std::make_shared<CellContent>();
    if (!ctx.valid()) return out;
    const TableIndex &ix = *ctx.index;
    double y = 0;
    for (int i = i0; i < i1;) {
        const auto &up = ix.chain(ix.refs()[size_t(i)].key);
        CellContent::Part part;
        part.y = y;
        if (int(up.size()) <= direct) {
            // The cell's own paragraphs, up to a table inside it.
            int j = i;
            while (j < i1 && int(ix.chain(ix.refs()[size_t(j)].key).size()) <= direct) ++j;
            part.from = ix.refs()[size_t(i)].pos;
            part.to = ix.refs()[size_t(j - 1)].end;
            double h = 0;
            part.text = layoutText(ctx.doc, part.from, part.to, width, &h);
            y += h;
            i = j;
        } else {
            // A table inside it: the paragraphs that are in the same table of the cells one deeper.
            const TableData *table = up[size_t(direct)]->table.get();
            int j = i;
            while (j < i1) {
                const auto &uj = ix.chain(ix.refs()[size_t(j)].key);
                if (int(uj.size()) <= direct || uj[size_t(direct)]->table.get() != table) break;
                ++j;
            }
            part.from = ix.refs()[size_t(i)].pos;
            part.to = ix.refs()[size_t(j - 1)].end;
            part.table = wholeFragment(TableLayout::make(ctx, i, j, direct));
            if (part.table) y += part.table->height;
            i = j;
        }
        out->parts.push_back(part);
    }
    out->height = y;
    return out;
}

std::shared_ptr<TableLayout> TableLayout::make(const TableContext &ctx, int i0, int i1, int depth)
{
    if (!ctx.valid() || i0 >= i1) return nullptr;
    const TableIndex &ix = *ctx.index;
    const auto &first = ix.chain(ix.refs()[size_t(i0)].key);
    if (int(first.size()) <= depth) return nullptr;
    std::shared_ptr<TableLayout> t(new TableLayout);
    t->m_ctx = ctx;
    t->m_depth = depth;
    t->m_data = first[size_t(depth)]->table;
    const CellRecord *lastCell = nullptr;
    const RowData *lastRow = nullptr;
    for (int i = i0; i < i1; ++i) {
        const BlockRef &ref = ix.refs()[size_t(i)];
        const auto &up = ix.chain(ref.key);
        if (int(up.size()) <= depth) continue;
        const CellRecord *rec = up[size_t(depth)];
        if (rec->row.get() != lastRow) {
            Row r;
            r.data = rec->row.get();
            r.cell0 = int(t->m_cells.size());
            r.from = ref.pos;
            t->m_rows.push_back(r);
            lastRow = r.data;
            lastCell = nullptr;
        }
        if (rec != lastCell) {
            Cell c;
            c.rec = rec;
            c.row = int(t->m_rows.size()) - 1;
            c.r0 = i;
            c.from = ref.pos;
            t->m_cells.push_back(c);
            ++t->m_rows.back().cells;
            lastCell = rec;
        }
        Cell &c = t->m_cells.back();
        c.r1 = i + 1;
        c.to = ref.end;
        t->m_rows.back().to = ref.end;
    }
    if (t->m_rows.empty()) return nullptr;
    // The grid: each cell covers as many columns as it says, from where the cell before it ends.
    int columns = 0;
    for (Row &r : t->m_rows) {
        int col = std::clamp(r.data->before, 0, kMostGridColumns - 1);
        for (int i = r.cell0; i < r.cell0 + r.cells; ++i) {
            Cell &c = t->m_cells[size_t(i)];
            c.col = col;
            c.span = std::clamp(c.rec->cell.span, 1, kMostGridColumns - col);
            col += c.span;
        }
        columns = std::max(columns, col);
    }
    QVector<double> widths = t->m_data->columns;
    double fill = 0;
    int counted = 0;
    for (double w : std::as_const(widths))
        if (w > 0) {
            fill += w;
            ++counted;
        }
    fill = counted ? fill / counted : 72;
    while (widths.size() < columns) widths.push_back(fill);
    t->m_xs.resize(widths.size() + 1);
    t->m_xs[0] = 0;
    for (int i = 0; i < widths.size(); ++i) t->m_xs[i + 1] = t->m_xs[i] + std::max(0.0, widths[i]);
    t->m_width = t->m_xs.last();
    for (Cell &c : t->m_cells) {
        const int end = std::min(c.col + c.span, int(widths.size()));
        c.x = t->m_xs[c.col];
        c.w = t->m_xs[end] - t->m_xs[c.col];
        c.margins = c.rec->cell.margins.value_or(t->m_data->margins);
    }
    // Cells joined down: a cell that continues the one above it in the same columns is part of that cell.
    std::vector<int> open(size_t(widths.size()), -1);
    for (int r = 0; r < int(t->m_rows.size()); ++r) {
        const Row &row = t->m_rows[size_t(r)];
        std::vector<bool> touched(open.size(), false);
        for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
            Cell &c = t->m_cells[size_t(i)];
            const int end = std::min(c.col + c.span, int(open.size()));
            if (c.rec->cell.vmerge == CellData::VMerge::Continue && c.col < int(open.size()) && open[size_t(c.col)] >= 0) {
                Cell &top = t->m_cells[size_t(open[size_t(c.col)])];
                if (top.col == c.col && top.span == c.span) {
                    c.covered = true;
                    ++top.rows;
                    t->m_maxRows = std::max(t->m_maxRows, top.rows);
                    for (int k = c.col; k < end; ++k) touched[size_t(k)] = true;
                    continue;
                }
            }
            for (int k = c.col; k < end; ++k) {
                open[size_t(k)] = c.rec->cell.vmerge == CellData::VMerge::Restart ? i : -1;
                touched[size_t(k)] = true;
            }
        }
        for (size_t k = 0; k < open.size(); ++k)
            if (!touched[k]) open[k] = -1;
    }
    while (t->m_headers < int(t->m_rows.size()) && t->m_rows[size_t(t->m_headers)].data->header) ++t->m_headers;
    return t;
}

int TableLayout::rowOfPosition(int pos) const
{
    if (m_rows.empty()) return -1;
    const auto it = std::upper_bound(m_rows.begin(), m_rows.end(), pos, [](int p, const Row &r) { return p < r.from; });
    return std::max(0, int(it - m_rows.begin()) - 1);
}

int TableLayout::cellOfPosition(int pos) const
{
    const int r = rowOfPosition(pos);
    if (r < 0) return -1;
    const Row &row = m_rows[size_t(r)];
    for (int i = row.cell0; i < row.cell0 + row.cells; ++i)
        if (pos >= m_cells[size_t(i)].from && pos < m_cells[size_t(i)].to) return i;
    return -1;
}

double TableLayout::leftEdgeIn(double area) const
{
    switch (m_data->align) {
    case TableData::Align::Center: return (area - m_width) / 2;
    case TableData::Align::Right: return area - m_width;
    default: return m_data->indent;
    }
}

double TableLayout::innerWidth(const Cell &c) const { return std::max(kLeastWidth, c.w - c.margins.left() - c.margins.right()); }

std::shared_ptr<CellContent> TableLayout::contentOf(const Cell &c) const { return layoutCellContent(m_ctx, c.r0, c.r1, m_depth + 1, innerWidth(c)); }

double TableLayout::measureCell(Cell &c)
{
    if (c.need < 0) c.need = contentOf(c)->height + c.margins.top() + c.margins.bottom();
    return c.need;
}

void TableLayout::measure(int r)
{
    r = std::min(r, int(m_rows.size()) - 1);
    if (!m_ctx.valid()) return;
    for (; m_measured <= r; ++m_measured) {
        Row &row = m_rows[size_t(m_measured)];
        const RowData &rd = *row.data;
        const bool exact = rd.exact && rd.height > 0;
        double need = 0;
        if (!exact)
            for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
                Cell &c = m_cells[size_t(i)];
                if (c.covered) continue;
                const double h = measureCell(c);
                if (c.rows == 1) need = std::max(need, h);
            }
        double h = exact ? rd.height : std::max(need, rd.height);
        // Cells joined down that end in this row need the part of their height that the rows above them do not give.
        if (!exact && m_maxRows > 1)
            for (int k = std::max(0, m_measured - m_maxRows + 1); k < m_measured; ++k) {
                const Row &above = m_rows[size_t(k)];
                for (int i = above.cell0; i < above.cell0 + above.cells; ++i) {
                    const Cell &c = m_cells[size_t(i)];
                    if (c.covered || c.rows <= 1 || c.row + c.rows - 1 != m_measured || c.need < 0) continue;
                    double given = 0;
                    for (int q = c.row; q < m_measured; ++q) given += m_rows[size_t(q)].height;
                    h = std::max(h, c.need - given);
                }
            }
        row.height = h;
    }
}

double TableLayout::rowOffset(int from, int r)
{
    measure(r);
    double y = 0;
    for (int k = from; k < std::min(r, int(m_rows.size())); ++k) y += m_rows[size_t(k)].height;
    return y;
}

double TableLayout::headerHeight()
{
    if (m_headers <= 0) return 0;
    measure(m_headers - 1);
    double h = 0;
    for (int k = 0; k < m_headers; ++k) h += m_rows[size_t(k)].height;
    return h;
}

// ---- fragments

namespace {

// The cells of rows [r0, r1) of a table that show in them (those that begin in them, and those joined down that begin above), the content of each laid out.
void addItems(TableFragment *f, int r0, int r1, std::set<int> *added)
{
    TableLayout &t = *f->layout;
    for (int r = std::max(0, r0 - 4096); r < r1 && r < t.rowCount(); ++r) {
        const TableLayout::Row &row = t.row(r);
        for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
            const TableLayout::Cell &c = t.cell(i);
            if (c.covered || c.row + c.rows <= r0 || c.row >= r1 || added->count(i)) continue;
            added->insert(i);
            f->items.push_back({i, t.contentOf(c)});
        }
    }
}

std::shared_ptr<TableFragment> makeFragment(const std::shared_ptr<TableLayout> &layout, int first, int end)
{
    if (!layout || layout->rowCount() == 0) return nullptr;
    auto f = std::make_shared<TableFragment>();
    f->layout = layout;
    end = std::clamp(end, 1, layout->rowCount());
    first = std::clamp(first, 0, end - 1);
    f->firstRow = first;
    f->endRow = end;
    layout->measure(end - 1);
    f->header = TableFlow::repeatsHeader(*layout, first);
    double h = f->header ? layout->headerHeight() : 0;
    for (int r = first; r < end; ++r) h += layout->row(r).height;
    f->height = h;
    std::set<int> added;
    if (f->header) addItems(f.get(), 0, layout->headerRows(), &added);
    addItems(f.get(), first, end, &added);
    return f;
}

std::shared_ptr<TableFragment> wholeFragment(const std::shared_ptr<TableLayout> &layout)
{
    if (!layout) return nullptr;
    return makeFragment(layout, 0, layout->rowCount());
}

} // namespace

std::vector<TableFragment::Placed> TableFragment::place() const
{
    std::vector<Placed> out;
    if (!layout) return out;
    double y = origin.y();
    auto rows = [&](int r0, int r1) {
        double sliceH = 0;
        for (int r = r0; r < r1; ++r) sliceH += layout->row(r).height;
        const QRectF slice(origin.x(), y, layout->width(), sliceH);
        for (int n = 0; n < int(items.size()); ++n) {
            const TableLayout::Cell &c = layout->cell(items[size_t(n)].cell);
            if (c.row >= r1 || c.row + c.rows <= r0) continue;
            const double top = c.row >= r0 ? layout->rowOffset(r0, c.row) : -layout->rowOffset(c.row, r0);
            const double h = layout->rowOffset(c.row, c.row + c.rows);
            Placed p;
            p.item = n;
            p.rect = QRectF(origin.x() + c.x, y + top, c.w, h);
            p.clip = p.rect.intersected(slice);
            const double inner = h - c.margins.top() - c.margins.bottom();
            const double free = std::max(0.0, inner - items[size_t(n)].content->height);
            const double drop = c.rec->cell.valign == CellAlign::Center ? free / 2 : c.rec->cell.valign == CellAlign::Bottom ? free : 0;
            p.text = QPointF(p.rect.left() + c.margins.left(), p.rect.top() + c.margins.top() + drop);
            out.push_back(p);
        }
        y += sliceH;
    };
    if (header) rows(0, layout->headerRows());
    rows(firstRow, endRow);
    return out;
}

namespace {

enum class Side { Top, Left, Bottom, Right };

// The cell (an index) that covers a grid place, or -1.
int cellCovering(TableLayout &t, int row, int col)
{
    if (row < 0 || row >= t.rowCount() || col < 0) return -1;
    for (int r = row; r >= 0 && r > row - 4096; --r) {
        const TableLayout::Row &rr = t.row(r);
        for (int i = rr.cell0; i < rr.cell0 + rr.cells; ++i) {
            const TableLayout::Cell &c = t.cell(i);
            if (col >= c.col && col < c.col + c.span && !c.covered && c.row + c.rows > row) return i;
        }
    }
    return -1;
}

// The line on one side of a cell: its own, or the one the cell next to it has for the same edge (the heavier of the two), or the table's.
TableLine edgeOf(TableLayout &t, int index, Side side)
{
    const TableLayout::Cell &c = t.cell(index);
    const TableData &d = t.data();
    const CellData &cd = c.rec->cell;
    std::optional<TableLine> own;
    int neighbor = -1;
    TableLine tableWide;
    switch (side) {
    case Side::Top:
        own = cd.top;
        neighbor = cellCovering(t, c.row - 1, c.col);
        tableWide = neighbor >= 0 ? d.insideH : d.top;
        break;
    case Side::Bottom:
        own = cd.bottom;
        neighbor = cellCovering(t, c.row + c.rows, c.col);
        tableWide = neighbor >= 0 ? d.insideH : d.bottom;
        break;
    case Side::Left:
        own = cd.left;
        neighbor = c.col > 0 ? cellCovering(t, c.row, c.col - 1) : -1;
        tableWide = neighbor >= 0 ? d.insideV : d.left;
        break;
    case Side::Right:
        own = cd.right;
        neighbor = cellCovering(t, c.row, c.col + c.span);
        tableWide = neighbor >= 0 ? d.insideV : d.right;
        break;
    }
    std::optional<TableLine> theirs;
    if (neighbor >= 0) {
        const CellData &nd = t.cell(neighbor).rec->cell;
        theirs = side == Side::Top ? nd.bottom : side == Side::Bottom ? nd.top : side == Side::Left ? nd.right : nd.left;
    }
    if (own && theirs) return theirs->extent() > own->extent() ? *theirs : *own;
    if (own) return *own;
    if (theirs) return *theirs;
    return tableWide;
}

void strokeLine(QPainter *p, const TableLine &line, QPointF a, QPointF b, bool picture)
{
    if (!line.visible()) return;
    double w = line.width;
    // On the screen a line is at least a pixel wide, as a thin line in a picture of a page is.
    if (picture) {
        const double scale = std::sqrt(std::abs(p->transform().determinant()));
        if (scale > 0) w = std::max(w, 1.0 / scale);
    }
    const QColor color = line.color.isValid() ? line.color : QColor(Qt::black);
    QPen pen(color, w, Qt::SolidLine, Qt::FlatCap);
    if (line.style == TableLine::Style::Dotted) pen.setStyle(Qt::DotLine);
    else if (line.style == TableLine::Style::Dashed) pen.setStyle(Qt::DashLine);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);
    if (line.style == TableLine::Style::Double) {
        const bool horizontal = std::abs(a.y() - b.y()) < 1e-9;
        const QPointF off = horizontal ? QPointF(0, w) : QPointF(w, 0);
        p->drawLine(a - off, b - off);
        p->drawLine(a + off, b + off);
    } else {
        p->drawLine(a, b);
    }
}

} // namespace

void TableFragment::paint(QPainter *p, const TablePaint &o) const
{
    if (!layout || layout->rowCount() == 0) return;
    const std::vector<Placed> placed = place();
    p->save();
    // Shading: the table's under the cells'.
    if (layout->data().fill && layout->data().fill->alpha() > 0) p->fillRect(rect(), *layout->data().fill);
    for (const Placed &pl : placed) {
        const TableLayout::Cell &c = layout->cell(items[size_t(pl.item)].cell);
        if (c.rec->cell.fill && c.rec->cell.fill->alpha() > 0) p->fillRect(pl.clip, *c.rec->cell.fill);
    }
    // The text of each cell.
    PaintOptions po;
    po.output = o.output;
    po.showSpecial = o.marks;
    po.selFrom = o.selFrom;
    po.selTo = o.selTo;
    for (const Placed &pl : placed) {
        const CellContent &content = *items[size_t(pl.item)].content;
        p->save();
        p->setClipRect(pl.clip, Qt::IntersectClip);
        for (const CellContent::Part &part : content.parts) {
            p->save();
            p->translate(pl.text.x(), pl.text.y() + part.y);
            if (part.text) {
                part.text->paint(p, 0, po);
            } else if (part.table) {
                TableFragment nested = *part.table;
                nested.origin = QPointF(0, 0);
                nested.paint(p, o);
            }
            p->restore();
        }
        p->restore();
    }
    // The borders, over the text: each line once (a cell's top and left, and its bottom and right where no cell is beyond them).
    const bool picture = o.output == TextOutput::Picture;
    for (const Placed &pl : placed) {
        const int index = items[size_t(pl.item)].cell;
        const TableLayout::Cell &c = layout->cell(index);
        const QRectF r = pl.rect;
        p->save();
        p->setClipRect(QRectF(pl.clip).adjusted(-2, -2, 2, 2), Qt::IntersectClip);
        strokeLine(p, edgeOf(*layout, index, Side::Top), r.topLeft(), r.topRight(), picture);
        strokeLine(p, edgeOf(*layout, index, Side::Left), r.topLeft(), r.bottomLeft(), picture);
        if (cellCovering(*layout, c.row + c.rows, c.col) < 0) strokeLine(p, edgeOf(*layout, index, Side::Bottom), r.bottomLeft(), r.bottomRight(), picture);
        if (cellCovering(*layout, c.row, c.col + c.span) < 0) strokeLine(p, edgeOf(*layout, index, Side::Right), r.topRight(), r.bottomRight(), picture);
        p->restore();
    }
    p->restore();
}

bool TableFragment::contains(int pos) const
{
    for (const Item &it : items) {
        const TableLayout::Cell &c = layout->cell(it.cell);
        if (pos >= c.from && pos < c.to) return true;
    }
    return false;
}

QRectF TableFragment::cellRect(int pos) const
{
    for (const Placed &pl : place()) {
        const TableLayout::Cell &c = layout->cell(items[size_t(pl.item)].cell);
        if (pos >= c.from && pos < c.to) return pl.clip;
    }
    return QRectF();
}

bool TableFragment::caretLine(int pos, QLineF *line) const
{
    for (const Placed &pl : place()) {
        const CellContent &content = *items[size_t(pl.item)].content;
        for (const CellContent::Part &part : content.parts) {
            if (pos < part.from || pos >= part.to) continue;
            if (part.text) {
                int frame = -1;
                QRectF rect;
                if (!part.text->caretRect(pos, &frame, &rect) || frame != 0) continue;
                const double x = pl.text.x() + rect.x(), top = pl.text.y() + part.y + rect.top();
                *line = QLineF(x, top, x, top + rect.height());
                return true;
            }
            if (part.table) {
                TableFragment nested = *part.table;
                nested.origin = QPointF(pl.text.x(), pl.text.y() + part.y);
                if (nested.caretLine(pos, line)) return true;
            }
        }
    }
    return false;
}

int TableFragment::positionAt(const QPointF &point) const
{
    const std::vector<Placed> placed = place();
    const Placed *best = nullptr;
    double bestDistance = 1e18;
    for (const Placed &pl : placed) {
        const QRectF &r = pl.clip;
        const double dx = point.x() < r.left() ? r.left() - point.x() : point.x() > r.right() ? point.x() - r.right() : 0;
        const double dy = point.y() < r.top() ? r.top() - point.y() : point.y() > r.bottom() ? point.y() - r.bottom() : 0;
        const double d = dy * dy * 4 + dx * dx;
        if (d < bestDistance) {
            bestDistance = d;
            best = &pl;
        }
    }
    if (!best) return -1;
    const CellContent &content = *items[size_t(best->item)].content;
    const CellContent::Part *chosen = nullptr;
    for (const CellContent::Part &part : content.parts) {
        const double top = best->text.y() + part.y;
        if (point.y() >= top || !chosen) chosen = &part;
    }
    if (!chosen) return layout->cell(items[size_t(best->item)].cell).from;
    if (chosen->table) {
        TableFragment nested = *chosen->table;
        nested.origin = QPointF(best->text.x(), best->text.y() + chosen->y);
        const int p = nested.positionAt(point);
        if (p >= 0) return p;
        return chosen->from;
    }
    if (!chosen->text) return chosen->from;
    const QPointF local(point.x() - best->text.x(), point.y() - (best->text.y() + chosen->y));
    const QVector<QRectF> lines = chosen->text->lineRects(0);
    if (lines.isEmpty()) return chosen->from;
    int line = 0;
    double bestGap = 1e18;
    for (int i = 0; i < lines.size(); ++i) {
        const double gap = local.y() < lines[i].top() ? lines[i].top() - local.y() : local.y() > lines[i].bottom() ? local.y() - lines[i].bottom() : 0;
        if (gap < bestGap) {
            bestGap = gap;
            line = i;
        }
    }
    int pos = chosen->text->hitTest(0, QPointF(local.x(), lines[line].center().y()));
    if (pos < 0) pos = local.x() < 0 ? chosen->text->lineStart(chosen->text->firstPosition(0)) : chosen->text->lastPosition(0);
    return std::clamp(pos, chosen->from, std::max(chosen->from, chosen->to - 1));
}

// ---- the flow

TableFlow::TableFlow(FlowDocument *doc) : m_doc(doc), m_live(std::make_shared<TableContext::Live>()) {}

TableFlow::~TableFlow() { ++m_live->epoch; }

void TableFlow::edited()
{
    m_valid = false;
    m_layouts.clear();
    ++m_live->epoch;
}

const TableIndex &TableFlow::index()
{
    if (!m_valid) {
        m_index.rebuild(m_doc->story(), m_doc->tables);
        m_valid = true;
    }
    return m_index;
}

TableContext TableFlow::context()
{
    index();
    TableContext c;
    c.doc = m_doc;
    c.index = &m_index;
    c.live = m_live;
    c.at = m_live->epoch;
    return c;
}

std::shared_ptr<TableLayout> TableFlow::layout(int span)
{
    const TableIndex &ix = index();
    if (span < 0 || span >= int(ix.spans().size())) return nullptr;
    auto it = m_layouts.find(span);
    if (it != m_layouts.end()) return it.value();
    const TableIndex::Span &s = ix.spans()[size_t(span)];
    std::shared_ptr<TableLayout> t = TableLayout::make(context(), s.first, s.last, 0);
    m_layouts.insert(span, t);
    return t;
}

std::shared_ptr<TableFragment> TableFlow::fragment(const std::shared_ptr<TableLayout> &layout, int firstRow, int endRow) { return makeFragment(layout, firstRow, endRow); }

} // namespace jp::flow
