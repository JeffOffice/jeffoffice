#include "flow/tableflow.h"

#include "text/textengine.h"
#include "text/textprops.h"

#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>
#include <cmath>
#include <set>

namespace jp::flow {

namespace {

// A frame is this tall for a cell's paragraphs, to begin with: more than most cells take. A cell that takes more is laid out again in a frame eight times as tall, and so
// on (a cell is not cut; a row is as tall as what is in it) until the frame is the tallest: more than the lines of the most paragraphs a cell may have (wml.h), each
// as tall as a page.
constexpr double kTall = 100000;
constexpr double kTallest = 6.4e9;
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

void TableIndex::rebuild(const QTextDocument *story, const TableStore &store, const FlowDocument *doc)
{
    m_spans.clear();
    m_refs.clear();
    m_chains.clear();
    m_store = &store;
    if (store.isEmpty()) return;
    const TableData *current = nullptr;
    // Where the document has notes, the notes before each paragraph are counted as the text engine counts them (the numbers restart in each section when the document says).
    const bool notes = doc && doc->fields;
    const bool restarts = notes && doc->restartNotesInSections;
    int footnotes = 0, endnotes = 0;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        if (restarts) {
            const QTextBlock before = b.previous();
            if (before.isValid() && before.blockFormat().boolProperty(fp::SectionBreakAfter)) footnotes = endnotes = 0;
        }
        const int foot = footnotes, endn = endnotes;
        if (notes)
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QTextCharFormat cf = it.fragment().charFormat();
                if (!cf.hasProperty(tp::Field)) continue;
                const QString code = cf.stringProperty(tp::Field);
                if (code.startsWith(QLatin1String("footnote:"))) footnotes += cf.boolProperty(tp::NoteMarkCustom) ? 0 : it.fragment().length();   // (a custom mark takes no number)
                else if (code.startsWith(QLatin1String("endnote:"))) endnotes += it.fragment().length();
            }
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
        m_refs.push_back({b.position(), b.position() + b.length(), key, foot, endn, footnotes - foot});
        m_spans.back().to = b.position() + b.length();
        m_spans.back().last = int(m_refs.size());
    }
}

bool TableIndex::shift(int position, int removed, int added)
{
    const int end = position + removed;
    for (const Span &s : m_spans)
        if (s.to >= position - 1 && s.from <= end + 1) return false;
    const int delta = added - removed;
    if (delta == 0) return true;
    for (Span &s : m_spans)
        if (s.from >= end) {
            s.from += delta;
            s.to += delta;
        }
    for (BlockRef &r : m_refs)
        if (r.pos >= end) {
            r.pos += delta;
            r.end += delta;
        }
    return true;
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
std::shared_ptr<StoryLayout> layoutText(FlowDocument *doc, int from, int to, double width, double *height, int footnotesBefore = 0, int endnotesBefore = 0, const QColor &backdrop = QColor())
{
    FrameSpec fs;
    fs.backdrop = backdrop;
    fs.size = QSizeF(std::max(kLeastWidth, width), kTall);
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.columns = 1;
    fs.hyphenate = doc->hyphenate;
    fs.hyphenZone = doc->hyphenZone;
    fs.ctx.doc = doc->fields;
    fs.ctx.pageNumber = doc->fieldPageNumber;
    fs.ctx.pageCount = doc->fieldPageCount;
    StoryLayout::Span span;
    span.from = from;
    span.end = to;
    span.keepRules = false;
    span.footnotesBefore = footnotesBefore;   // (the notes the cell cites are numbered as the document counts them; the page sets them at its foot)
    span.endnotesBefore = endnotesBefore;
    std::shared_ptr<StoryLayout> layout;
    double bottom = 0;
    int lastBlock = -1;
    for (;;) {
        layout = std::make_shared<StoryLayout>();
        layout->build(doc->story(), {fs}, doc->env, span);
        bottom = 0;
        lastBlock = -1;
        bool cut = false;
        for (const StoryLayout::LineBrief &l : layout->lineBriefs()) {
            if (l.frame < 0) {
                cut = true;
                continue;
            }
            bottom = std::max(bottom, l.rect.bottom());
            lastBlock = l.block;
        }
        if (!cut || fs.size.height() >= kTallest) break;
        fs.size.setHeight(std::min(kTallest, fs.size.height() * 8));
    }
    if (lastBlock >= 0) bottom += doc->env.wordLines ? layout->trailingSpace() : std::max(0.0, doc->story()->findBlock(lastBlock).blockFormat().bottomMargin());
    *height = bottom;
    return layout;
}

std::shared_ptr<TableFragment> wholeFragment(const std::shared_ptr<TableLayout> &layout);

// The fill under the text of the cell whose paragraphs these are (`up` is the chain of cells the paragraph is in, `direct` how many are the cell's own, the last being it): the
// cell's own fill, else its table's, else the cell the table is in has (outward), none if none has one.
QColor backdropOf(const QVector<const CellRecord *> &up, int direct)
{
    for (int k = std::min(direct, int(up.size())) - 1; k >= 0; --k) {
        const CellRecord &r = *up[k];
        if (r.cell.fill && r.cell.fill->alpha() > 0) return *r.cell.fill;
        if (r.table && r.table->fill && r.table->fill->alpha() > 0) return *r.table->fill;
    }
    return QColor();
}

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
            part.foot = ix.refs()[size_t(i)].foot;
            part.end = ix.refs()[size_t(i)].endNotes;
            double h = 0;
            part.text = layoutText(ctx.doc, part.from, part.to, width, &h, part.foot, part.end, backdropOf(up, direct));
            if (ctx.doc->fields) {
                // The footnotes it cites, in order, with the numbers they show.
                const QTextDocument *story = ctx.doc->story();
                int number = part.foot;
                for (QTextBlock b = story->findBlock(part.from); b.isValid() && b.position() < part.to; b = b.next())
                    for (auto it = b.begin(); !it.atEnd(); ++it) {
                        const QTextFragment frag = it.fragment();
                        const QString code = frag.charFormat().stringProperty(tp::Field);
                        if (!code.startsWith(QLatin1String("footnote:"))) continue;
                        const bool custom = frag.charFormat().boolProperty(tp::NoteMarkCustom);   // (it shows no number and takes none)
                        for (int k = 0; k < frag.length() && part.notes.size() < TableLayout::kMostRowNotes; ++k) part.notes.push_back({frag.position() + k, code.mid(9), custom ? 0 : ++number});
                    }
            }
            // (The last paragraph of a cell, after a table in it, when nothing is in it, is the cell's end mark, which the program keeps after a table: it takes no room, what it asks
            // of space above it too, and the paragraphs before it count as they are.)
            const BlockRef &last = ix.refs()[size_t(j - 1)];
            if (j == i1 && last.end - last.pos == 1 && !out->parts.empty() && out->parts.back().table) {
                part.dummyFrom = last.pos;
                h = 0;
                for (const StoryLayout::LineBrief &l : part.text->lineBriefs()) {
                    if (l.frame < 0) continue;
                    if (l.block >= part.dummyFrom) break;
                    h = std::max(h, l.rect.bottom());
                }
            }
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

namespace {

// What a line takes of the room of a row (a double line takes its two lines and the space between).
double roomOf(const TableLine &line) { return line.visible() ? line.extent() : 0.0; }

// The line between two cells (or at the edge of the table, where there is only one): each cell says what it says of its side of it, a cell that says nothing has the table's
// line for the inside, and the heavier of the two is the one. (So a cell that says none, with the cell beyond it saying nothing, leaves the table's line: both must say none.)
// At the edge of the table a cell's own line is the one, else the table's.
TableLine lineBetween(const std::optional<TableLine> &before, const std::optional<TableLine> &after, bool hasBefore, bool hasAfter, const TableLine &inside, const TableLine &outside)
{
    if (!hasBefore) return after ? *after : outside;
    if (!hasAfter) return before ? *before : outside;
    const TableLine a = before ? *before : inside, b = after ? *after : inside;
    return b.weight() > a.weight() ? b : a;
}

} // namespace

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
                    top.last = i;
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
    t->setLineRooms();
    return t;
}

void TableLayout::setLineRooms()
{
    const TableData &d = *m_data;
    const int rows = int(m_rows.size());
    // The cell that has the line under a cell: the last of those joined down.
    auto bottomOf = [&](const Cell &c) -> const CellData & { return (c.last >= 0 ? m_cells[size_t(c.last)] : c).rec->cell; };
    // The cells that are above the row's: for each grid column, the one that is there (one joined down has a cell in each row, and its line at the top is a line of the row: a
    // program that sets such a cell leaves the room for it, though the line is not drawn across the cell).
    std::vector<int> above(size_t(m_xs.size()), -1), here(above);
    for (int r = 0; r < rows; ++r) {
        Row &row = m_rows[size_t(r)];
        std::fill(here.begin(), here.end(), -1);
        double top = 0, outerTop = 0;
        for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
            const Cell &c = m_cells[size_t(i)];
            const int end = std::min(c.col + c.span, int(here.size()));
            for (int k = c.col; k < end; ++k) here[size_t(k)] = i;
            const CellData &cd = c.rec->cell;
            if (d.spaced) {
                // (Cells apart from one another each have their own lines, the table's are round the lot.)
                top = std::max(top, roomOf(cd.top.value_or(TableLine())));
                continue;
            }
            // The line between the cell and each cell above it, along the columns it covers: the heaviest of them is the room it takes.
            int previous = -2;
            for (int k = c.col; k < end && k < int(above.size()); ++k) {
                const int neighbor = above[size_t(k)];
                if (neighbor == previous) continue;
                previous = neighbor;
                top = std::max(top, roomOf(lineBetween(neighbor >= 0 ? m_cells[size_t(neighbor)].rec->cell.bottom : std::nullopt, cd.top, neighbor >= 0, true, d.insideH, d.top)));
            }
            if (!c.covered) outerTop = std::max(outerTop, roomOf(lineBetween(std::nullopt, cd.top, false, true, d.insideH, d.top)));
        }
        row.top = top + (d.spaced && r == 0 ? roomOf(d.top) : 0.0);
        row.breakTop = d.spaced ? (r > 0 ? roomOf(d.top) : 0.0) : std::max(0.0, outerTop - row.top);
        above = here;
    }
    // The foot: the cells that end at the table's foot have the line there (cells apart have the foot of each row), with the table's own round them. Where a page ends with a
    // row that the table goes on from, it has the table's foot line (`breakFoot`).
    for (const Cell &c : m_cells) {
        if (c.covered) continue;
        Row &row = m_rows[size_t(std::min(c.row + c.rows - 1, rows - 1))];
        if (d.spaced) {
            row.foot = std::max(row.foot, roomOf(bottomOf(c).bottom.value_or(TableLine())));
        } else {
            const double outer = roomOf(lineBetween(bottomOf(c).bottom, std::nullopt, true, false, d.insideH, d.bottom));
            if (&row == &m_rows.back()) row.foot = std::max(row.foot, outer);
            else row.breakFoot = std::max(row.breakFoot, outer);
        }
    }
    if (d.spaced) {
        m_rows.back().foot += roomOf(d.bottom);
        for (int r = 0; r + 1 < rows; ++r) m_rows[size_t(r)].breakFoot = roomOf(d.bottom);
        // The space between cells is above and below each row's cells.
        if (d.spacing > 0)
            for (Row &row : m_rows) {
                row.top += d.spacing;
                row.foot += d.spacing;
            }
    }
    // The room is in the cells' margins, below the line at the top of the row and above the one at the foot of the table.
    for (Cell &c : m_cells) {
        if (c.covered) continue;
        c.margins.setTop(c.margins.top() + m_rows[size_t(c.row)].top);
        c.margins.setBottom(c.margins.bottom() + m_rows[size_t(std::min(c.row + c.rows - 1, rows - 1))].foot);
    }
    // A row set to a height has that for the room inside the margins of its cells (and the lines): the margins come in addition.
    for (Row &row : m_rows) {
        if (row.data->height <= 0) continue;
        double pad = row.top + row.foot;
        bool below = false;
        for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
            const Cell &c = m_cells[size_t(i)];
            if (c.covered) continue;
            below |= c.rec->cell.valign != CellAlign::Top;
            if (c.rows == 1) pad = std::max(pad, c.margins.top() + c.margins.bottom());
        }
        row.least = row.data->height + pad;
        row.seated = below && !row.data->exact;
    }
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
    default: return m_data->indent - (m_data->indentToText && !m_cells.empty() ? m_cells[0].margins.left() : 0);
    }
}

double TableLayout::innerWidth(const Cell &c) const { return std::max(kLeastWidth, c.w - c.margins.left() - c.margins.right()); }

std::shared_ptr<CellContent> TableLayout::contentFor(const Cell &c, bool keep) const
{
    CellContentCache *cache = m_ctx.valid() ? m_ctx.contents : nullptr;
    const double width = innerWidth(c);
    if (cache && c.rec) {
        if (auto found = cache->find(c.rec->key, width)) return found;
    }
    if (m_ctx.laidOut) ++*m_ctx.laidOut;
    auto content = layoutCellContent(m_ctx, c.r0, c.r1, m_depth + 1, width);
    if (cache && c.rec) {
        if (keep) cache->insert(c.rec->key, width, content, std::max(1, c.r1 - c.r0));
        else cache->insertJustMeasured(c.rec->key, width, content);
    }
    return content;
}

// ---- the cells laid out lately

namespace {
// What the cells laid out lately hold between them, in paragraphs, at the most (one that is more than that is kept as the only one).
constexpr int kContentBudget = 8192;
} // namespace

void CellContentCache::insertJustMeasured(int key, double width, const std::shared_ptr<CellContent> &content)
{
    m_recent[m_recentAt] = {keyOf(key, width), content};
    m_recentAt = (m_recentAt + 1) % kJustMeasured;
}

std::shared_ptr<CellContent> CellContentCache::find(int key, double width)
{
    const qint64 k = keyOf(key, width);
    for (const Recent &r : m_recent)
        if (r.key == k) return r.content;
    const auto it = m_entries.find(k);
    if (it == m_entries.end()) return nullptr;
    m_order.splice(m_order.begin(), m_order, it->at);
    return it->content;
}

void CellContentCache::insert(int key, double width, const std::shared_ptr<CellContent> &content, int paragraphs)
{
    const qint64 k = keyOf(key, width);
    if (m_entries.contains(k)) return;
    m_order.push_front(k);
    m_entries.insert(k, Entry{content, m_order.begin(), paragraphs});
    m_paragraphs += paragraphs;
    while (m_paragraphs > kContentBudget && m_order.size() > 1) {
        const qint64 last = m_order.back();
        m_order.pop_back();
        const auto it = m_entries.find(last);
        if (it != m_entries.end()) {
            m_paragraphs -= it->paragraphs;
            m_entries.erase(it);
        }
    }
}

void CellContentCache::clear()
{
    for (Recent &r : m_recent) r = Recent();
    m_order.clear();
    m_entries.clear();
    m_paragraphs = 0;
}

double TableLayout::measureCell(Cell &c)
{
    if (c.need < 0) c.need = contentFor(c, c.r1 - c.r0 >= 16)->height + c.margins.top() + c.margins.bottom();
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
        // A row that was measured and has not been touched since is as tall as it was.
        const QPair<const RowData *, const TableData *> kept(row.data, m_data.get());
        if (m_ctx.heights) {
            const auto it = m_ctx.heights->constFind(kept);
            if (it != m_ctx.heights->constEnd()) {
                row.height = it.value();
                continue;
            }
        }
        if (m_ctx.measured) ++*m_ctx.measured;
        double need = 0;
        if (!exact)
            for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
                Cell &c = m_cells[size_t(i)];
                if (c.covered) continue;
                const double h = measureCell(c);
                if (c.rows == 1) need = std::max(need, h);
            }
        double h = exact ? rd.height : std::max(need, row.least);
        // Cells joined down that end in this row need the part of their height that the rows above them do not give.
        if (!exact && m_maxRows > 1)
            for (int k = std::max(0, m_measured - m_maxRows + 1); k < m_measured; ++k) {
                const Row &above = m_rows[size_t(k)];
                for (int i = above.cell0; i < above.cell0 + above.cells; ++i) {
                    Cell &c = m_cells[size_t(i)];
                    if (c.covered || c.rows <= 1 || c.row + c.rows - 1 != m_measured) continue;
                    const double need = measureCell(c);   // (a joined cell's rows above may have come from what was measured before: it is not known yet)
                    double given = 0;
                    for (int q = c.row; q < m_measured; ++q) given += m_rows[size_t(q)].height;
                    h = std::max(h, need - given);
                }
            }
        row.height = h;
        if (m_ctx.heights) m_ctx.heights->insert(kept, h);
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

// ---- a row over a page break

namespace {

double lineTopInContent(const CellContent &content, int pos)
{
    for (const CellContent::Part &part : content.parts) {
        if (pos < part.from || pos >= part.to) continue;
        if (part.text) {
            int frame = -1;
            QRectF rect;
            if (part.text->caretRect(pos, &frame, &rect) && frame == 0) return part.y + rect.top();
        } else if (part.table) {
            const TableFragment &nested = *part.table;
            const int i = nested.layout->cellOfPosition(pos);
            if (i < 0) continue;
            const TableLayout::Cell &c = nested.layout->cell(i);
            double top = 0;
            for (int r = nested.firstRow; r < c.row && r < nested.endRow; ++r) top += nested.heightOf(r);
            const double inside = nested.layout->lineTopIn(i, pos);
            if (inside >= 0) return part.y + top + c.margins.top() + inside;
        }
    }
    return -1;
}

} // namespace

double TableLayout::lineTopIn(int cell, int position) const
{
    if (cell < 0 || cell >= int(m_cells.size())) return -1;
    return lineTopInContent(*contentOf(m_cells[size_t(cell)]), position);
}

bool TableLayout::splittable(int r) const
{
    const Row &row = m_rows[size_t(r)];
    if (row.data->cantSplit || row.data->header || (row.data->exact && row.data->height > 0)) return false;
    // A row as tall as it was set to be (its cells need less) has no lines to cut it between: it goes whole, as tall as it was set.
    if (row.data->height > 0 && row.height <= row.least + 0.01) return false;
    for (int i = row.cell0; i < row.cell0 + row.cells; ++i)
        if (m_cells[size_t(i)].covered || m_cells[size_t(i)].rows > 1) return false;
    return true;
}

bool TableLayout::keepsWithNext(int r) const
{
    if (!m_ctx.valid() || !m_ctx.index || !m_ctx.doc || r < 0 || r >= int(m_rows.size())) return false;
    const Row &row = m_rows[size_t(r)];
    const TableIndex &ix = *m_ctx.index;
    const QTextDocument *story = m_ctx.doc->story();
    for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
        const Cell &c = m_cells[size_t(i)];
        if (c.covered) continue;
        // (The cell's last paragraph is the one next to the next row: a paragraph before it is kept with the one after it, in the cell.)
        const BlockRef &last = ix.refs()[size_t(c.r1 - 1)];
        if (int(ix.chain(last.key).size()) != m_depth + 1) continue;   // (a paragraph of a table in the cell is that table's)
        if (story->findBlock(last.pos).blockFormat().boolProperty(tp::KeepWithNext)) return true;
    }
    return false;
}

double TableLayout::leastPart(int r)
{
    const double whole = rowHeight(r);
    if (!splittable(r)) return whole;
    const Row &row = m_rows[size_t(r)];
    double need = 0;
    for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
        const Cell &c = m_cells[size_t(i)];
        if (c.covered) continue;
        const auto units = unitsOf(i);
        need = std::max(need, (units->lines.empty() ? 0.0 : units->lines.front().bottom) + c.margins.top() + c.margins.bottom());
    }
    return std::min(whole, need);
}

namespace {

using LineList = std::vector<TableLayout::UnitLine>;

void contentLines(const CellContent &content, double y0, LineList *out);

// Where a table in a cell can be cut: between its rows, and inside a row that may be split, between what the lines of its cells leave clear (a table joined down
// anywhere is one piece, as is a row that must stay whole).
void tableLines(const TableFragment &nested, double y0, LineList *out)
{
    TableLayout &t = *nested.layout;
    bool joined = false;
    for (int i = 0; i < t.cellCount() && !joined; ++i) joined = t.cell(i).rows > 1;
    if (joined) {
        TableLayout::UnitLine whole{y0, y0 + nested.height, {}};
        for (int r = nested.firstRow; r < nested.endRow && whole.notes.size() < TableLayout::kMostRowNotes; ++r) whole.notes += t.rowNotes(r);
        out->push_back(whole);
        return;
    }
    double y = y0;
    for (int r = nested.firstRow; r < nested.endRow; ++r) {
        const double h = nested.heightOf(r);
        LineList inRow;
        if (t.splittable(r)) {
            const TableLayout::Row &row = t.row(r);
            for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
                const TableLayout::Cell &c = t.cell(i);
                if (c.covered) continue;
                const std::shared_ptr<CellContent> content = t.contentOf(c);
                const double inner = h - c.margins.top() - c.margins.bottom();
                const double free = std::max(0.0, inner - content->height);
                const double drop = c.rec->cell.valign == CellAlign::Center ? free / 2 : c.rec->cell.valign == CellAlign::Bottom ? free : 0;
                LineList mine;
                contentLines(*content, y + c.margins.top() + drop, &mine);
                // Where the row goes on from a line, the cell has its margin and lines at the top again (and the table its own lines round the lot, if its cells are apart).
                const double pad = c.margins.top() + (t.data().spaced && c.row != 0 ? roomOf(t.data().top) : 0.0);
                for (size_t k = 1; k < mine.size(); ++k) mine[k].pad += pad;
                inRow.insert(inRow.end(), mine.begin(), mine.end());
            }
        }
        if (inRow.empty()) {
            out->push_back({y, y + h, t.rowNotes(r)});
        } else {
            // The cells' lines side by side: those that overlap in height are one piece.
            std::sort(inRow.begin(), inRow.end(), [](const TableLayout::UnitLine &a, const TableLayout::UnitLine &b) { return a.top != b.top ? a.top < b.top : a.bottom < b.bottom; });
            TableLayout::UnitLine run = inRow.front();
            run.top = std::min(run.top, y);   // (the first piece of the row begins at the row's top: where it goes on from there, its margins and lines go with it)
            // (A row set to a height with content in the middle or at the foot is not begun in less room than that: its first piece is as tall as it is.)
            if (t.row(r).seated) run.bottom = std::max(run.bottom, y + t.row(r).least);
            for (size_t k = 1; k < inRow.size(); ++k) {
                if (inRow[k].top < run.bottom - 0.01) {
                    run.bottom = std::max(run.bottom, inRow[k].bottom);
                    run.pad = std::max(run.pad, inRow[k].pad);
                    if (run.notes.size() < TableLayout::kMostRowNotes) run.notes += inRow[k].notes;
                } else {
                    out->push_back(run);
                    run = inRow[k];
                }
            }
            out->push_back(run);
        }
        y += h;
    }
}

void contentLines(const CellContent &content, double y0, LineList *out)
{
    for (const CellContent::Part &part : content.parts) {
        if (part.text) {
            const auto &notes = part.notes;
            for (const StoryLayout::LineBrief &l : part.text->lineBriefs()) {
                if (l.frame < 0 || (part.dummyFrom >= 0 && l.block >= part.dummyFrom)) continue;
                TableLayout::UnitLine line{y0 + part.y + l.rect.top(), y0 + part.y + l.rect.bottom(), {}};
                if (!notes.isEmpty()) {
                    // (The notes whose references are on the line: the part's notes are in the order of their places.)
                    const auto first = std::lower_bound(notes.begin(), notes.end(), l.pos, [](const NoteRef &n, int pos) { return n.pos < pos; });
                    for (auto it = first; it != notes.end() && it->pos < l.end; ++it) line.notes.push_back(*it);
                }
                out->push_back(line);
            }
        } else if (part.table) {
            tableLines(*part.table, y0 + part.y, out);
        }
    }
}

} // namespace

std::shared_ptr<const TableLayout::Units> TableLayout::unitsOf(int index)
{
    const auto found = m_units.constFind(index);
    if (found != m_units.constEnd()) return found.value();
    auto u = std::make_shared<Units>();
    const std::shared_ptr<CellContent> content = contentOf(m_cells[size_t(index)]);
    contentLines(*content, 0, &u->lines);
    u->height = content->height;
    m_units.insert(index, u);
    return u;
}

bool TableLayout::rowHasNotes(int r) const
{
    if (!m_ctx.doc || !m_ctx.doc->fields || !m_ctx.valid() || r < 0 || r >= int(m_rows.size())) return false;
    const Row &row = m_rows[size_t(r)];
    if (row.cells <= 0) return false;
    const TableIndex &ix = *m_ctx.index;
    const int i0 = m_cells[size_t(row.cell0)].r0, i1 = m_cells[size_t(row.cell0 + row.cells - 1)].r1;
    for (int i = i0; i < i1 && i < int(ix.refs().size()); ++i)
        if (ix.refs()[size_t(i)].notes > 0) return true;
    return false;
}

QVector<NoteRef> TableLayout::rowNotes(int r)
{
    QVector<NoteRef> out;
    if (!rowHasNotes(r)) return out;
    const Row &row = m_rows[size_t(r)];
    for (int i = row.cell0; i < row.cell0 + row.cells && out.size() < kMostRowNotes; ++i) {
        if (m_cells[size_t(i)].covered) continue;
        for (const UnitLine &l : unitsOf(i)->lines) {
            if (out.size() >= kMostRowNotes) break;
            out += l.notes;
        }
    }
    std::sort(out.begin(), out.end(), [](const NoteRef &a, const NoteRef &b) { return a.number != b.number ? a.number < b.number : a.pos < b.pos; });
    return out;
}

TableLayout::Piece TableLayout::pieceOf(int r, int index, const QVector<double> &from, double room, bool ruleNeeded, double ruleGap, bool force, double topExtra)
{
    constexpr double kEnd = 1e18;
    Piece piece;
    const Row &row = m_rows[size_t(r)];
    const int n = row.cells, k = std::clamp(index, 0, kMostParts - 1);
    const QVector<double> offs = from.size() == n ? from : QVector<double>(n, 0.0);
    std::vector<std::shared_ptr<const Units>> units(static_cast<size_t>(n));
    const bool anyNotes = m_noteHeight && rowHasNotes(r);
    for (int i = 0; i < n; ++i)
        if (!m_cells[size_t(row.cell0 + i)].covered) units[size_t(i)] = unitsOf(row.cell0 + i);
    // The part that begins at `offs` (each cell's place): where the next begins, whether any cell has a line in it, whether any has lines left, and the
    // notes its lines cite (they take room at the page's foot, which the lines have less of).
    struct Step {
        QVector<double> next;
        bool progress = false, more = false;
        double avail = 0, noteHeight = 0;
        QVector<NoteRef> notes;
    };
    // Each cell's lines from `offs` on, as many as the whole room holds at most, with the room that the notes of the first so many of them take (the sum of the heights, and how
    // many notes there are, at most a bound).
    struct CellRun {
        const std::vector<UnitLine> *lines = nullptr;
        int a = 0, most = 0;                 // the first line, and one past the last that fits the room whole
        double margins = 0, pad = 0;         // the room the cell's margins take, and above the lines of a part that goes on in a table in the cell besides (in `margins` too)
        std::vector<double> noteH;           // by the number of lines taken: the heights of their notes together
        std::vector<int> noteN;              // and how many there are
    };
    std::vector<CellRun> runs(static_cast<size_t>(n));
    const bool noteHeights = anyNotes && m_noteHeight;
    int counted = 0;
    for (int i = 0; i < n; ++i) {
        if (!units[size_t(i)]) continue;
        CellRun &cr = runs[size_t(i)];
        const Cell &c = m_cells[size_t(row.cell0 + i)];
        const auto &lines = units[size_t(i)]->lines;
        cr.lines = &lines;
        cr.a = int(std::lower_bound(lines.begin(), lines.end(), offs[i] - 0.01, [](const UnitLine &l, double off) { return l.top < off; }) - lines.begin());
        if (offs[i] > 0 && cr.a < int(lines.size())) cr.pad = lines[size_t(cr.a)].pad;
        cr.pad += topExtra;   // (a part at the top of a page that the table goes on to: the table's top line is there)
        cr.margins = c.margins.top() + c.margins.bottom() + cr.pad;
        const double inner = std::max(0.0, room - cr.margins);
        cr.most = int(std::partition_point(lines.begin() + cr.a, lines.end(), [&](const UnitLine &l) { return l.bottom - offs[i] <= inner + 0.01; }) - lines.begin());
        if (noteHeights) {
            // (A line more than fits is counted too: a part at the top of a page is given one however tall.)
            const int upto = std::min<int>(int(lines.size()), cr.most + 1);
            cr.noteH.assign(size_t(std::max(0, upto - cr.a)) + 1, 0.0);
            cr.noteN.assign(cr.noteH.size(), 0);
            for (int q = cr.a; q < upto; ++q) {
                double h = cr.noteH[size_t(q - cr.a)];
                int cnt = cr.noteN[size_t(q - cr.a)];
                for (const NoteRef &nr : lines[size_t(q)].notes) {
                    if (counted >= kMostRowNotes) break;
                    h += m_noteHeight(nr.id);
                    ++cnt;
                    ++counted;
                }
                cr.noteH[size_t(q - cr.a) + 1] = h;
                cr.noteN[size_t(q - cr.a) + 1] = cnt;
            }
        }
    }
    // At the top of a page, the most the notes may take: what the room leaves a line of each cell that has one (a note longer than a page is cut at the foot and goes on).
    double cap = 1e18;
    if (force) {
        double need = 0;
        for (int i = 0; i < n; ++i) {
            const CellRun &cr = runs[size_t(i)];
            if (cr.lines && cr.a < int(cr.lines->size())) need = std::max(need, (*cr.lines)[size_t(cr.a)].bottom - (*cr.lines)[size_t(cr.a)].top + cr.margins);
        }
        cap = std::max(0.0, room - need);
    }
    // The part when the notes take `reserved` of the room: the lines of each cell that fit what is left, and the room that their notes take.
    auto fit = [&](double reserved) {
        Step st;
        st.avail = room;
        st.next = QVector<double>(n, kEnd);
        std::vector<int> stop(static_cast<size_t>(n), -1);
        for (int i = 0; i < n; ++i) {
            const CellRun &cr = runs[size_t(i)];
            if (!cr.lines || cr.a >= int(cr.lines->size())) continue;
            const double inner = std::max(0.0, room - reserved - cr.margins);
            const auto &lines = *cr.lines;
            const int b = int(std::partition_point(lines.begin() + cr.a, lines.begin() + std::min<int>(int(lines.size()), cr.most), [&](const UnitLine &l) { return l.bottom - offs[i] <= inner + 0.01; }) - lines.begin());
            stop[size_t(i)] = b;
            st.progress |= b > cr.a;
            st.more |= b < int(lines.size());
        }
        // A part at the top of a page has a line of each cell that has one left, however tall: a row taller than a page ends.
        if (!st.progress && st.more && force) {
            st.more = false;
            for (int i = 0; i < n; ++i) {
                const CellRun &cr = runs[size_t(i)];
                if (!cr.lines || cr.a >= int(cr.lines->size())) continue;
                stop[size_t(i)] = cr.a + 1;
                st.more |= cr.a + 1 < int(cr.lines->size());
            }
            st.progress = true;
        }
        int notesN = 0;
        double h = 0;
        for (int i = 0; i < n; ++i) {
            const CellRun &cr = runs[size_t(i)];
            if (stop[size_t(i)] < 0) continue;
            const auto &lines = *cr.lines;
            const int b = stop[size_t(i)];
            st.next[i] = b < int(lines.size()) ? lines[size_t(b)].top : kEnd;
            if (noteHeights && b - cr.a < int(cr.noteH.size())) {
                h += cr.noteH[size_t(b - cr.a)];
                notesN += cr.noteN[size_t(b - cr.a)];
            }
        }
        if (notesN > 0 && ruleNeeded) h += ruleGap;
        st.noteHeight = std::min(h, cap);
        if (notesN > 0) {
            for (int i = 0; i < n; ++i) {
                const CellRun &cr = runs[size_t(i)];
                if (stop[size_t(i)] < 0) continue;
                for (int q = cr.a; q < stop[size_t(i)] && int(st.notes.size()) < kMostRowNotes; ++q) st.notes += (*cr.lines)[size_t(q)].notes;
            }
            std::sort(st.notes.begin(), st.notes.end(), [](const NoteRef &a, const NoteRef &b) { return a.number != b.number ? a.number < b.number : a.pos < b.pos; });
        }
        return st;
    };
    // The notes need room, which takes from the lines', which cite other notes: the most lines whose notes fit the room they leave (the fewest room to give the notes
    // for which they take no more than they are given: the notes the lines take cannot grow as the room for the lines does not).
    Step st = fit(0);
    if (noteHeights && st.noteHeight > 0.01) {
        double lo = 0, hi = st.noteHeight;   // (with all the room for the lines they take `hi`; given `hi` they take no more than that)
        for (int pass = 0; pass < 40 && hi - lo > 0.01; ++pass) {
            const double mid = (lo + hi) / 2;
            if (fit(mid).noteHeight <= mid + 0.01) hi = mid;
            else lo = mid;
        }
        st = fit(hi);
    }
    if (k >= kMostParts - 1) st.more = false;   // (the part the parts end in has all that is left)
    piece.progress = st.progress;
    // A row set to a height that has content in the middle or at the foot is not begun where the room is less than that height: it goes to the next page whole.
    if (k == 0 && !force && st.more && row.seated && room < row.least - 0.01) piece.progress = false;
    piece.offsets = offs;
    piece.pads = QVector<double>(n, 0.0);
    for (int i = 0; i < n; ++i) piece.pads[i] = runs[size_t(i)].pad;
    piece.ends = st.more ? st.next : QVector<double>(n, kEnd);
    piece.notes = st.notes;
    piece.noteHeight = st.noteHeight;
    const double text = st.avail - st.noteHeight;   // (what the lines have: the notes are at the foot)
    if (st.more) {
        piece.height = text;
        piece.next = k + 1;
    } else {
        // The row ends here: as tall as what is left of the cells, but not past its room when the lines fit it (the space after the last is let go at a page's foot).
        double need = 0, lines = 0;
        for (int i = 0; i < n; ++i) {
            if (!units[size_t(i)] || offs[i] >= kEnd) continue;
            const Cell &c = m_cells[size_t(row.cell0 + i)];
            const auto &ls = units[size_t(i)]->lines;
            const double pad = runs[size_t(i)].pad;
            need = std::max(need, units[size_t(i)]->height - offs[i] + c.margins.top() + c.margins.bottom() + pad);
            if (!ls.empty()) lines = std::max(lines, ls.back().bottom - offs[i] + c.margins.top() + c.margins.bottom() + pad);
        }
        piece.height = lines <= text + 0.01 ? std::min(need, text) : need;
        // The last part of a row that was set to a height is that tall as well (the page it is on has room for it, or it is as tall as the room).
        if (k > 0 && row.data->height > 0 && !row.data->exact) piece.height = std::max(piece.height, std::min(row.least, text));
        piece.next = 0;
    }
    return piece;
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

std::shared_ptr<TableFragment> makeFragment(const std::shared_ptr<TableLayout> &layout, int first, int end, double columnHeight, const std::vector<TableFragment::Part> &parts = {})
{
    if (!layout || layout->rowCount() == 0) return nullptr;
    auto f = std::make_shared<TableFragment>();
    f->layout = layout;
    end = std::clamp(end, 1, layout->rowCount());
    first = std::clamp(first, 0, end - 1);
    f->firstRow = first;
    f->endRow = end;
    layout->measure(end - 1);
    f->header = columnHeight > 0 && TableFlow::repeatsHeader(*layout, first, columnHeight);
    f->parts = parts;
    double h = f->header ? layout->headerHeight() : 0;
    for (int r = first; r < end; ++r) h += f->heightOf(r);
    f->height = h;
    std::set<int> added;
    if (f->header) addItems(f.get(), 0, layout->headerRows(), &added);
    addItems(f.get(), first, end, &added);
    return f;
}

std::shared_ptr<TableFragment> wholeFragment(const std::shared_ptr<TableLayout> &layout)
{
    if (!layout) return nullptr;
    return makeFragment(layout, 0, layout->rowCount(), 0);
}

} // namespace

const TableFragment::Part *TableFragment::partOf(int row) const
{
    for (const Part &p : parts)
        if (p.row == row) return &p;
    return nullptr;
}

double TableFragment::topExtraOf(int row) const { return !header && firstRow > 0 && row == firstRow ? layout->row(row).breakTop : 0.0; }

double TableFragment::footExtraOf(int row) const { return endRow < layout->rowCount() && row == endRow - 1 ? layout->row(row).breakFoot : 0.0; }

double TableFragment::heightOf(int row) const
{
    const Part *p = partOf(row);
    // (The part of a row has the room of the table's lines at the break in it already.)
    return p ? p->piece.height : layout->row(row).height + topExtraOf(row) + footExtraOf(row);
}

std::vector<TableFragment::Placed> TableFragment::place() const
{
    std::vector<Placed> out;
    if (!layout) return out;
    double y = origin.y();
    auto rows = [&](int r0, int r1) {
        // Where each row begins, from the first of them.
        std::vector<double> tops(size_t(r1 - r0) + 1, 0.0);
        for (int r = r0; r < r1; ++r) tops[size_t(r - r0) + 1] = tops[size_t(r - r0)] + heightOf(r);
        const QRectF slice(origin.x(), y, layout->width(), tops.back());
        for (int n = 0; n < int(items.size()); ++n) {
            const TableLayout::Cell &c = layout->cell(items[size_t(n)].cell);
            if (c.row >= r1 || c.row + c.rows <= r0) continue;
            // (A cell joined down may begin above the first row here and reach past the last: its place is that of the rows it covers.)
            double top = 0, h = 0;
            if (c.row >= r0) top = tops[size_t(c.row - r0)];
            else top = -layout->rowOffset(c.row, r0);
            for (int k = c.row; k < c.row + c.rows; ++k) h += k >= r0 && k < r1 ? heightOf(k) : layout->rowHeight(k);
            Placed p;
            p.item = n;
            p.rect = QRectF(origin.x() + c.x, y + top, c.w, h);
            p.clip = p.rect.intersected(slice);
            const Part *part = c.rows == 1 ? partOf(c.row) : nullptr;
            const double extraTop = c.row >= r0 && c.rows == 1 && !part ? topExtraOf(c.row) : 0.0, extraFoot = c.rows == 1 && !part ? footExtraOf(c.row) : 0.0;
            const double inner = h - c.margins.top() - c.margins.bottom() - extraTop - extraFoot;
            const double free = std::max(0.0, inner - items[size_t(n)].content->height);
            double drop = c.rec->cell.valign == CellAlign::Center ? free / 2 : c.rec->cell.valign == CellAlign::Bottom ? free : 0;
            double offset = 0;
            if (part) {
                // (A row shown in part is set from the top: the cell's content begins where its part does. A cell whose content is all in the part is set in it as it is in a row.)
                drop = 0;
                const int at = items[size_t(n)].cell - layout->row(c.row).cell0;
                if (at >= 0 && at < part->piece.offsets.size()) offset = std::min(part->piece.offsets[at], items[size_t(n)].content->height);
                const double pad = at >= 0 && at < part->piece.pads.size() ? part->piece.pads[at] : 0.0;
                if (offset <= 0 && at >= 0 && at < part->piece.ends.size() && part->piece.ends[at] >= 1e17) {
                    const double partFree = std::max(0.0, h - c.margins.top() - c.margins.bottom() - pad - items[size_t(n)].content->height);
                    drop = c.rec->cell.valign == CellAlign::Center ? partFree / 2 : c.rec->cell.valign == CellAlign::Bottom ? partFree : 0;
                }
                p.text = QPointF(p.rect.left() + c.margins.left(), p.rect.top() + c.margins.top() + pad + drop - offset);
                if (offset > 0) p.from = p.text.y() + offset;   // (what the part before showed is not drawn in the margin above the lines of this one)
                // (What is left for the next part is not drawn in the space under the last line that is in this one.)
                if (at >= 0 && at < part->piece.ends.size() && part->piece.ends[at] < 1e17) p.cut = p.text.y() + part->piece.ends[at];
            } else {
                p.text = QPointF(p.rect.left() + c.margins.left(), p.rect.top() + c.margins.top() + extraTop + drop - offset);
            }
            out.push_back(p);
        }
        y += tops.back();
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

// The line on one side of a cell: the one between it and the cell next to it (see lineBetween), or the table's at its edge; for cells joined down, the bottom is the last
// cell's. Of cells apart (a table with space between them) it is the cell's own, or none.
TableLine edgeOf(TableLayout &t, int index, Side side)
{
    const TableLayout::Cell &c = t.cell(index);
    const TableData &d = t.data();
    const CellData &cd = c.rec->cell;
    const CellData &bottomCell = (c.last >= 0 ? t.cell(c.last) : c).rec->cell;
    if (d.spaced) {
        const std::optional<TableLine> &own = side == Side::Top ? cd.top : side == Side::Bottom ? bottomCell.bottom : side == Side::Left ? cd.left : cd.right;
        return own ? *own : TableLine();
    }
    switch (side) {
    case Side::Top: {
        const int n = cellCovering(t, c.row - 1, c.col);
        const TableLayout::Cell *above = n >= 0 ? &t.cell(n) : nullptr;
        return lineBetween(above ? (above->last >= 0 ? t.cell(above->last) : *above).rec->cell.bottom : std::nullopt, cd.top, above != nullptr, true, d.insideH, d.top);
    }
    case Side::Bottom: {
        const int n = cellCovering(t, c.row + c.rows, c.col);
        return lineBetween(bottomCell.bottom, n >= 0 ? t.cell(n).rec->cell.top : std::nullopt, true, n >= 0, d.insideH, d.bottom);
    }
    case Side::Left: {
        const int n = c.col > 0 ? cellCovering(t, c.row, c.col - 1) : -1;
        return lineBetween(n >= 0 ? t.cell(n).rec->cell.right : std::nullopt, cd.left, n >= 0, true, d.insideV, d.left);
    }
    case Side::Right: {
        const int n = cellCovering(t, c.row, c.col + c.span);
        return lineBetween(cd.right, n >= 0 ? t.cell(n).rec->cell.left : std::nullopt, true, n >= 0, d.insideV, d.right);
    }
    }
    return TableLine();
}

struct EdgePart {
    double x0 = 0, x1 = 0;   // along the table, from its left edge
    TableLine line;
};

// The line along the top or the foot of a cell, in parts: a cell across columns has the line between it and each cell it meets there.
std::vector<EdgePart> edgeParts(TableLayout &t, int index, bool top)
{
    const TableLayout::Cell &c = t.cell(index);
    const TableData &d = t.data();
    if (c.span == 1 || d.spaced) return {{c.x, c.x + c.w, edgeOf(t, index, top ? Side::Top : Side::Bottom)}};
    const CellData &cd = c.rec->cell;
    const CellData &bottomCell = (c.last >= 0 ? t.cell(c.last) : c).rec->cell;
    std::vector<EdgePart> out;
    const int row = top ? c.row - 1 : c.row + c.rows;
    int start = c.col, last = -2;
    auto close = [&](int end, int n) {   // the columns [start, end) meet cell n
        const TableLayout::Cell *other = n >= 0 ? &t.cell(n) : nullptr;
        const TableLine line = top ? lineBetween(other ? (other->last >= 0 ? t.cell(other->last) : *other).rec->cell.bottom : std::nullopt, cd.top, other != nullptr, true, d.insideH, d.top)
                                   : lineBetween(bottomCell.bottom, other ? other->rec->cell.top : std::nullopt, true, other != nullptr, d.insideH, d.bottom);
        out.push_back({t.columnLeft(start), t.columnLeft(end), line});
    };
    for (int col = c.col; col < c.col + c.span; ++col) {
        const int n = cellCovering(t, row, col);
        if (col > c.col && n != last) {
            close(col, last);
            start = col;
        }
        last = n;
    }
    close(c.col + c.span, last);
    return out;
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
    if (line.style == TableLine::Style::Double || line.style == TableLine::Style::Triple) {
        const bool horizontal = std::abs(a.y() - b.y()) < 1e-9;
        const QPointF off = horizontal ? QPointF(0, w) : QPointF(w, 0);
        if (line.style == TableLine::Style::Triple) {
            p->drawLine(a - off * 2, b - off * 2);
            p->drawLine(a, b);
            p->drawLine(a + off * 2, b + off * 2);
        } else {
            p->drawLine(a - off, b - off);
            p->drawLine(a + off, b + off);
        }
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
        // (A table in the cell has its lines on the cell's edges when the cell has no margin: they are drawn whole.)
        const bool hasTable = std::any_of(content.parts.begin(), content.parts.end(), [](const CellContent::Part &part) { return part.table != nullptr; });
        p->save();
        QRectF area = hasTable ? pl.clip.adjusted(-2, -2, 2, 2) : pl.clip;
        if (pl.cut < area.bottom()) area.setBottom(pl.cut);
        if (pl.from > area.top()) area.setTop(pl.from);
        p->setClipRect(area, Qt::IntersectClip);
        for (const CellContent::Part &part : content.parts) {
            p->save();
            p->translate(pl.text.x(), pl.text.y() + part.y);
            if (part.text) {
                part.text->paint(p, 0, po);
                if (o.objects && !part.text->inlineObjects().isEmpty()) o.objects(p, *part.text);
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
        // (A line across is in the row it is the top of, from its edge down; the foot of the table is in its last row, up from the edge.)
        // (A table that goes on after a page break has its outer lines there: the top at the page's first row, the foot at its last.)
        const TableData &td = layout->data();
        const CellData &cd = c.rec->cell;
        const CellData &bottomCell = (c.last >= 0 ? layout->cell(c.last) : c).rec->cell;
        const bool beginsPage = !header && firstRow > 0 && c.row == firstRow, endsPage = endRow < layout->rowCount() && c.row + c.rows == endRow;
        if (beginsPage && !td.spaced) {
            const TableLine top = lineBetween(std::nullopt, cd.top, false, true, td.insideH, td.top);
            const double down = roomOf(top) / 2;
            strokeLine(p, top, r.topLeft() + QPointF(0, down), r.topRight() + QPointF(0, down), picture);
        } else {
            for (const EdgePart &e : edgeParts(*layout, index, true)) {
                const double down = roomOf(e.line) / 2;
                strokeLine(p, e.line, QPointF(r.left() + e.x0 - c.x, r.top() + down), QPointF(r.left() + e.x1 - c.x, r.top() + down), picture);
            }
        }
        strokeLine(p, edgeOf(*layout, index, Side::Left), r.topLeft(), r.bottomLeft(), picture);
        const Part *part = c.rows == 1 ? partOf(c.row) : nullptr;
        const bool cut = endsPage || (part && part->piece.next != 0);   // (the foot of a row that goes on past a page break, and of the page's last)
        // (Cells apart each have all their lines.)
        if (td.spaced || cut || cellCovering(*layout, c.row + c.rows, c.col) < 0) {
            if (cut && !td.spaced) {
                const TableLine bottom = lineBetween(bottomCell.bottom, std::nullopt, true, false, td.insideH, td.bottom);
                const double up = roomOf(bottom) / 2;
                strokeLine(p, bottom, r.bottomLeft() - QPointF(0, up), r.bottomRight() - QPointF(0, up), picture);
            } else {
                for (const EdgePart &e : edgeParts(*layout, index, false)) {
                    const double up = roomOf(e.line) / 2;
                    strokeLine(p, e.line, QPointF(r.left() + e.x0 - c.x, r.bottom() - up), QPointF(r.left() + e.x1 - c.x, r.bottom() - up), picture);
                }
            }
        }
        if (td.spaced || cellCovering(*layout, c.row, c.col + c.span) < 0) strokeLine(p, edgeOf(*layout, index, Side::Right), r.topRight(), r.bottomRight(), picture);
        p->restore();
    }
    // A table that has its cells apart has its own lines round all of them.
    const TableData &d = layout->data();
    if (d.spaced) {
        const QRectF whole = rect();
        const double down = roomOf(d.top) / 2, up = roomOf(d.bottom) / 2;
        strokeLine(p, d.top, whole.topLeft() + QPointF(0, down), whole.topRight() + QPointF(0, down), picture);
        strokeLine(p, d.bottom, whole.bottomLeft() - QPointF(0, up), whole.bottomRight() - QPointF(0, up), picture);
        strokeLine(p, d.left, whole.topLeft(), whole.bottomLeft(), picture);
        strokeLine(p, d.right, whole.topRight(), whole.bottomRight(), picture);
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

void TableFragment::forTexts(const std::function<void(const StoryLayout &text)> &visit) const
{
    for (const Item &item : items)
        for (const CellContent::Part &part : item.content->parts) {
            if (part.text) visit(*part.text);
            else if (part.table) part.table->forTexts(visit);
        }
}

void TableFragment::forLines(const std::function<void(const StoryLayout::LineBrief &line, const QRectF &rect)> &visit) const
{
    for (const Placed &pl : place()) {
        for (const CellContent::Part &part : items[size_t(pl.item)].content->parts) {
            if (part.text) {
                for (const StoryLayout::LineBrief &ln : part.text->lineBriefs()) {
                    if (ln.frame != 0) continue;
                    const QRectF rect(pl.text.x() + ln.rect.x(), pl.text.y() + part.y + ln.rect.top(), ln.rect.width(), ln.rect.height());
                    const double middle = rect.center().y();
                    if (middle < pl.clip.top() || middle > pl.clip.bottom() || middle > pl.cut) continue;
                    visit(ln, rect);
                }
            } else if (part.table) {
                TableFragment nested = *part.table;
                nested.origin = QPointF(pl.text.x(), pl.text.y() + part.y);
                nested.forLines(visit);
            }
        }
    }
}

bool TableFragment::lineEdge(int pos, bool end, int *result) const
{
    for (const Placed &pl : place()) {
        for (const CellContent::Part &part : items[size_t(pl.item)].content->parts) {
            if (!part.text || pos < part.from || pos >= part.to) continue;
            *result = end ? part.text->lineEnd(pos) : part.text->lineStart(pos);
            return true;
        }
    }
    return false;
}

int TableFragment::moveVertical(int pos, int direction, double *wantedX) const
{
    const std::vector<Placed> placed = place();
    for (const Placed &pl : placed) {
        const TableLayout::Cell &cell = layout->cell(items[size_t(pl.item)].cell);
        if (pos < cell.from || pos >= cell.to) continue;
        for (const CellContent::Part &part : items[size_t(pl.item)].content->parts) {
            if (!part.text || pos < part.from || pos >= part.to) continue;
            int frame = 0;
            QRectF rect;
            if (*wantedX < 0 && part.text->caretRect(pos, &frame, &rect)) *wantedX = pl.text.x() + rect.x();
            // On the first or last line of the stretch of text the way is on to the cell above or below (the engine's own Up from a first line is the story's start).
            const QVector<StoryLayout::LineBrief> briefs = part.text->lineBriefs();
            int at = -1;
            for (int i = 0; i < briefs.size() && at < 0; ++i)
                if (pos >= briefs[i].pos && (pos < briefs[i].end || i == briefs.size() - 1)) at = i;
            const bool edge = at < 0 || (direction < 0 && at == 0) || (direction > 0 && at == briefs.size() - 1);
            if (!edge) {
                const int moved = part.text->moveVertical(pos, direction, *wantedX - pl.text.x());
                if (moved != pos) return moved;
            }
            // At the first or last line of the cell: the cell above or below.
            const QPointF target(*wantedX, direction < 0 ? pl.clip.top() - 1 : pl.clip.bottom() + 1);
            const QRectF all = this->rect();
            if (target.y() < all.top() || target.y() > all.bottom()) return pos;
            const int found = positionAt(target);
            return found >= 0 && (found < cell.from || found >= cell.to) ? found : pos;
        }
    }
    return pos;
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

bool TableFlow::dropHeights(int lo, int hi)
{
    bool near = false;
    const int spans = int(m_index.spans().size());
    for (int si = m_index.firstFrom(lo - 1); si < spans; ++si) {
        const TableIndex::Span &s = m_index.spans()[size_t(si)];
        if (s.from > hi) break;
        if (s.to < lo) continue;
        near = true;
        if (const std::shared_ptr<TableLayout> t = layout(si)) dropHeightsIn(t, lo, hi);
    }
    // (A span that began before `lo` and goes on past it is not the first one `firstFrom` finds.)
    if (const int before = m_index.firstFrom(lo - 1) - 1; before >= 0 && before < spans) {
        const TableIndex::Span &s = m_index.spans()[size_t(before)];
        if (s.to >= lo && s.from <= hi && s.from < lo - 1) {
            near = true;
            if (const std::shared_ptr<TableLayout> t = layout(before)) dropHeightsIn(t, lo, hi);
        }
    }
    return near;
}

// The rows of the table that the places are in, and those beside them, and the rows that cells joined down in those are joined to (and so on: the rows a joined cell
// covers are as tall as it and they make), and the rows of the tables in the cells that the places are in.
void TableFlow::dropHeightsIn(const std::shared_ptr<TableLayout> &t, int lo, int hi)
{
    if (t->rowCount() == 0) return;
    const int first = t->row(0).from, last = t->row(t->rowCount() - 1).to;
    int a = std::max(0, t->rowOfPosition(std::max(lo, first)) - 1), b = std::min(t->rowCount() - 1, t->rowOfPosition(std::min(hi, last - 1)) + 1);
    const int reach = t->mostRowsJoined();
    for (bool grew = reach > 1; grew;) {
        grew = false;
        for (int k = std::max(0, a - reach + 1); k <= b; ++k) {
            const TableLayout::Row &row = t->row(k);
            for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
                const TableLayout::Cell &c = t->cell(i);
                if (c.covered || c.rows <= 1) continue;
                const int end = std::min(c.row + c.rows - 1, t->rowCount() - 1);
                if (c.row <= b && end >= a && (c.row < a || end > b)) {
                    a = std::min(a, c.row);
                    b = std::max(b, end);
                    grew = true;
                }
            }
        }
    }
    for (int r = a; r <= b; ++r) m_heights.remove(qMakePair(t->row(r).data, &t->data()));
    if (t->depth() == 0) {
        m_dirtyFrom = std::min(m_dirtyFrom, t->row(a).from);
        // (The header rows are on every page the table goes on to, and take their room: a change in them is a change on all those pages.)
        const int end = t->headerRows() > 0 && a < t->headerRows() ? t->rowCount() - 1 : b;
        m_dirtyTo = std::max(m_dirtyTo, t->row(end).to);
    }
    // The tables in the cells that the places are in.
    const TableIndex &ix = m_index;
    for (int r = std::max(0, t->rowOfPosition(std::max(lo, first)) - 1); r <= std::min(t->rowCount() - 1, t->rowOfPosition(std::min(hi, last - 1)) + 1); ++r) {
        const TableLayout::Row &row = t->row(r);
        for (int i = row.cell0; i < row.cell0 + row.cells; ++i) {
            const TableLayout::Cell &c = t->cell(i);
            if (c.to < lo || c.from > hi) continue;
            for (int p = c.r0; p < c.r1;) {
                const auto &up = ix.chain(ix.refs()[size_t(p)].key);
                if (int(up.size()) <= t->depth() + 1) {
                    ++p;
                    continue;
                }
                const TableData *table = up[size_t(t->depth() + 1)]->table.get();
                int q = p;
                while (q < c.r1) {
                    const auto &uq = ix.chain(ix.refs()[size_t(q)].key);
                    if (int(uq.size()) <= t->depth() + 1 || uq[size_t(t->depth() + 1)]->table.get() != table) break;
                    ++q;
                }
                if (const std::shared_ptr<TableLayout> nested = TableLayout::make(context(), p, q, t->depth() + 1)) dropHeightsIn(nested, lo, hi);
                p = q;
            }
        }
    }
}

bool TableFlow::takeDirty(int *from, int *to)
{
    const bool any = m_dirtyTo >= 0;
    if (any) {
        *from = m_dirtyFrom;
        *to = m_dirtyTo;
    }
    m_dirtyFrom = 0x7fffffff;
    m_dirtyTo = -1;
    return any;
}

void TableFlow::settleEdits()
{
    if (!m_valid && !m_touched.empty()) index();
}

void TableFlow::edited(int position, int removed, int added)
{
    bool near = false;
    const int delta = added - removed;
    if (m_valid) {
        // The rows that the edit touched (and those beside them, and the rows that cells joined down in them cover) are measured again; the others are as tall as they were.
        const int dirtyFrom = m_dirtyFrom, dirtyTo = m_dirtyTo;
        m_dirtyFrom = 0x7fffffff;
        m_dirtyTo = -1;
        near = dropHeights(position - 1, position + removed + 1);
        // (The places that came of it are the ones before the edit made: they are said as the places now.)
        if (m_dirtyTo >= 0) {
            auto now = [&](int p) { return p <= position ? p : p >= position + removed ? p + delta : position + added; };
            m_dirtyFrom = now(m_dirtyFrom);
            m_dirtyTo = std::max(now(m_dirtyTo), m_dirtyFrom);
        }
        if (dirtyTo >= 0) {
            m_dirtyFrom = std::min(m_dirtyFrom, dirtyFrom);
            m_dirtyTo = std::max(m_dirtyTo, dirtyTo);
        }
        // An edit that took characters out, or put a note's reference in, may change the numbers of the notes the cells after it cite: the tables are found again.
        // Numbers that begin again in each section also change with a paragraph made one that ends a section (or not), with paragraphs put in and taken out, and with text put in
        // the place of text: those edits find the tables again.
        const int blocks = m_doc->story()->blockCount();
        const bool structural = blocks != m_blockCount;
        m_blockCount = blocks;
        if (!near && m_doc->fields && m_doc->restartNotesInSections && ((removed > 0 && added > 0) || structural)) near = true;
        if (!near && m_doc->fields && removed > 0) {
            // (Only if one of the references was among them: the places of the references are kept, so that taking out text that has none does not find the tables again.)
            if (!m_refsValid) near = true;
            else {
                const auto at = std::lower_bound(m_refs.begin(), m_refs.end(), position);
                near = at != m_refs.end() && *at < position + removed;
            }
        }
        if (!near && m_doc->fields && added > 0) {
            const QTextDocument *story = m_doc->story();
            for (QTextBlock b = story->findBlock(std::max(0, position)); b.isValid() && b.position() <= position + added && !near; b = b.next())
                for (auto it = b.begin(); !it.atEnd() && !near; ++it) near = it.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("footnote:"));
        }
        // Paragraphs that are in a cell where no table was (a table pasted, the taking out of one undone, a paragraph given a cell): the tables are found afresh as well.
        if (!near && added > 0) {
            const QTextDocument *story = m_doc->story();
            for (QTextBlock b = story->findBlock(std::max(0, position)); b.isValid() && b.position() <= position + added && !near; b = b.next())
                near = b.blockFormat().intProperty(fp::Cell) != 0;
        }
        // (Rows that have gone from the story leave their heights behind: not many, and they go when there are.)
        if (m_heights.size() > 400000) m_heights.clear();
    } else {
        // The tables are not found: the places of the edits before this one move with it, and this one is kept to be dealt with when they are.
        for (std::pair<int, int> &t : m_touched) {
            if (t.second > position + removed) t.second += delta;
            else t.second = std::max(t.second, position);
            if (t.first >= position + removed) t.first += delta;
            else t.first = std::min(t.first, position);
            t.second = std::max(t.second, t.first);
        }
        m_refsValid = false;
        m_touched.push_back({position - 1, position + added + 1});
        if (m_touched.size() > 64) {
            m_heights.clear();
            m_touched.clear();
        }
    }
    ++m_live->epoch;
    m_layouts.clear();
    m_contents.clear();
    if (m_valid && !near && m_index.shift(position, removed, added)) {   // (the tables are where the text left them)
        if (m_refsValid && delta != 0) {
            const auto from = std::lower_bound(m_refs.begin(), m_refs.end(), position + removed);
            for (auto it = from; it != m_refs.end(); ++it) *it += delta;
        }
        return;
    }
    m_refsValid = false;
    // The tables are found again, and then the rows the edit touched are looked at as they are: a table may have other header rows than it had.
    if (m_valid) m_touched.push_back({position - 1, position + added + 1});
    m_valid = false;
}

void TableFlow::reset()
{
    m_valid = false;
    m_refsValid = false;
    m_layouts.clear();
    m_heights.clear();
    m_touched.clear();
    m_contents.clear();
    ++m_live->epoch;
}

void TableFlow::findRefs()
{
    m_refs.clear();
    m_refsValid = false;
    if (!m_doc->fields) return;
    for (QTextBlock b = m_doc->story()->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.charFormat().stringProperty(tp::Field).startsWith(QLatin1String("footnote:"))) continue;
            for (int i = 0; i < frag.length(); ++i) m_refs.push_back(frag.position() + i);
        }
    m_refsValid = true;
}

const TableIndex &TableFlow::index()
{
    if (!m_valid) {
        m_index.rebuild(m_doc->story(), m_doc->tables, m_doc);
        m_valid = true;
        m_blockCount = m_doc->story()->blockCount();
        findRefs();
        // The edits made since the tables were last found: the rows they touched are measured again.
        std::vector<std::pair<int, int>> touched;
        touched.swap(m_touched);
        for (const std::pair<int, int> &t : touched) dropHeights(t.first, t.second);
    }
    return m_index;
}

TableContext TableFlow::context()
{
    index();
    TableContext c;
    c.doc = m_doc;
    c.index = &m_index;
    c.heights = &m_heights;
    c.measured = &m_measured;
    c.laidOut = &m_laidOut;
    c.contents = &m_contents;
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

std::shared_ptr<TableFragment> TableFlow::fragment(const std::shared_ptr<TableLayout> &layout, int firstRow, int endRow, double columnHeight, const std::vector<TableFragment::Part> &parts)
{
    return makeFragment(layout, firstRow, endRow, columnHeight, parts);
}

} // namespace jp::flow
