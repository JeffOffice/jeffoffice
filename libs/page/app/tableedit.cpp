#include "app/tableedit.h"

#include "app/editor.h"
#include "office/drawingml_table.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <algorithm>
#include <optional>

namespace jp {

namespace {

// A row and column command: on the rows and columns the selected cells (or the cell the text cursor is in)
// cover, or with the whole table selected, a range that is not valid (callers choose: the ends to insert at,
// and nothing to delete). The command says what it did, and a caller keeping the table's style is told.
Editor::TableEdit editRowsAndColumns(Editor *ed, const QString &label, const std::function<Editor::TableEdit(TableItem *, const CellRange &)> &fn)
{
    TableItem *t = selectedTable(ed);
    if (!t) return {};
    const CellRange rg = ed->targetCells();
    ed->endTextEdit();
    Editor::TableEdit done;
    ed->change(label, [&] {
        done = fn(t, rg);
        dropUnusedStories(ed->doc());
        if (done.count && ed->tableEdited) ed->tableEdited(t, done);
    });
    ed->select(t->id);
    return done;
}

} // namespace

TableItem *selectedTable(Editor *ed)
{
    if (ed->isEditingText()) return dynamic_cast<TableItem *>(ed->doc()->item(ed->textTarget().itemId));
    return dynamic_cast<TableItem *>(ed->single());
}

// Takes away a cell's division by a diagonal: the second half's text joins the first's, as in Publisher (the
// first half's last paragraph goes on with the second's first), and the cell is one again.
void joinHalves(Document *d, TableCell &cell)
{
    QTextDocument *src = d->storyDoc(cell.storyId2);
    QTextDocument *dst = d->storyDoc(cell.storyId);
    if (src && dst && !src->isEmpty()) {
        QTextCursor dc(dst);
        dc.movePosition(QTextCursor::End);
        dc.insertFragment(QTextDocumentFragment(src));
    }
    d->removeStory(cell.storyId2);   // its text is in the first half now: nothing else may count or save it
    cell.storyId2.clear();
    cell.diagonal = 0;
    cell.diagonalLine = Stroke::none();
}

// Drops the stories no object uses any more, as deleting objects does.
void dropUnusedStories(Document *d)
{
    const QSet<QString> used = d->storiesInUse();
    for (const auto &sid : d->stories.keys())
        if (!used.contains(sid)) d->removeStory(sid);
}

// Table structure edits keep stories, spans and borders consistent.
// A new row or column through a merged cell makes the merged cell bigger, and
// the new cells under it are covered.
void tableInsertRow(Editor *ed, TableItem *t, int at)
{
    at = std::clamp(at, 0, t->rows);
    const int src = std::clamp(at == t->rows ? at - 1 : at, 0, t->rows - 1);
    QVector<bool> merged(t->cols, false);   // the columns the new row passes through a merged cell in
    QVector<QPoint> grows;                  // those merged cells (column, row)
    for (int r = 0; r < at; ++r)
        for (int c = 0; c < t->cols; ++c) {
            const TableCell &s = t->cell(r, c);
            if (s.covered || r + s.rowSpan <= at) continue;
            grows << QPoint(c, r);
            for (int k = c; k < std::min(t->cols, c + s.colSpan); ++k) merged[k] = true;
        }
    QVector<TableCell> row;
    for (int c = 0; c < t->cols; ++c) {
        TableCell n;
        n.covered = merged[c];
        const TableCell &s = t->cell(src, c);
        n.fill = s.fill;
        n.border = s.border;
        n.margins = s.margins;
        n.valign = s.valign;
        n.vertical = s.vertical;
        n.hyphenate = s.hyphenate;
        n.hyphenZone = s.hyphenZone;
        n.storyId = ed->doc()->createStory();
        row << n;
    }
    t->cells.insert(at * t->cols, row.size(), TableCell());
    for (int c = 0; c < t->cols; ++c) t->cells[at * t->cols + c] = row[c];
    t->rowH.insert(at, t->rowH.value(src, 18));
    ++t->rows;
    for (const QPoint &g : grows) ++t->cell(g.y(), g.x()).rowSpan;
    t->syncRect();
}

void tableInsertCol(Editor *ed, TableItem *t, int at)
{
    at = std::clamp(at, 0, t->cols);
    const int src = std::clamp(at == t->cols ? at - 1 : at, 0, t->cols - 1);
    QVector<bool> merged(t->rows, false);   // the rows the new column passes through a merged cell in
    QVector<QPoint> grows;                  // those merged cells (column, row)
    for (int r = 0; r < t->rows; ++r)
        for (int c = 0; c < at; ++c) {
            const TableCell &s = t->cell(r, c);
            if (s.covered || c + s.colSpan <= at) continue;
            grows << QPoint(c, r);
            for (int k = r; k < std::min(t->rows, r + s.rowSpan); ++k) merged[k] = true;
        }
    QVector<TableCell> cells;
    for (int r = 0; r < t->rows; ++r)
        for (int c = 0; c <= t->cols; ++c) {
            if (c == at) {
                TableCell n;
                n.covered = merged[r];
                const TableCell &s = t->cell(r, src);
                n.fill = s.fill;
                n.border = s.border;
                n.margins = s.margins;
                n.vertical = s.vertical;
                n.hyphenate = s.hyphenate;
                n.hyphenZone = s.hyphenZone;
                n.storyId = ed->doc()->createStory();
                cells << n;
            }
            if (c < t->cols) cells << t->cell(r, c);
        }
    const double w = t->colW.value(src, 72);
    t->colW.insert(at, w);
    ++t->cols;
    t->cells = cells;
    for (const QPoint &g : grows) ++t->cell(g.y(), g.x()).colSpan;
    // Keep the table the same width.
    double total = 0;
    for (double v : t->colW) total += v;
    const double k = (total - w) / total;
    for (double &v : t->colW) v *= k;
    t->syncRect();
}

// Deletes rows r0..r1 (a table keeps at least one). A merged cell reaching
// into them gets shorter; one that began in a deleted row begins in the
// first row after them, keeping its text.
void tableDeleteRows(TableItem *t, int r0, int r1)
{
    const int n = r1 - r0 + 1;
    if (n < 1 || n >= t->rows) return;
    for (int r = 0; r < t->rows; ++r)
        for (int c = 0; c < t->cols; ++c) {
            TableCell &cell = t->cell(r, c);
            if (cell.covered || cell.rowSpan <= 1) continue;
            const int cut = std::min(r + cell.rowSpan - 1, r1) - std::max(r, r0) + 1;
            if (cut <= 0) continue;
            cell.rowSpan -= cut;
            if (cell.rowSpan > 0 && r >= r0 && r <= r1) {
                TableCell moved = cell;
                moved.covered = false;
                cell.rowSpan = 1;
                t->cell(r1 + 1, c) = moved;
            }
        }
    t->cells.remove(r0 * t->cols, n * t->cols);
    t->rowH.remove(r0, n);
    t->rows -= n;
    t->syncRect();
}

// Deletes columns c0..c1, as above; the others widen to keep the table's width.
void tableDeleteCols(TableItem *t, int c0, int c1)
{
    const int n = c1 - c0 + 1;
    if (n < 1 || n >= t->cols) return;
    for (int r = 0; r < t->rows; ++r)
        for (int c = 0; c < t->cols; ++c) {
            TableCell &cell = t->cell(r, c);
            if (cell.covered || cell.colSpan <= 1) continue;
            const int cut = std::min(c + cell.colSpan - 1, c1) - std::max(c, c0) + 1;
            if (cut <= 0) continue;
            cell.colSpan -= cut;
            if (cell.colSpan > 0 && c >= c0 && c <= c1) {
                TableCell moved = cell;
                moved.covered = false;
                cell.colSpan = 1;
                t->cell(r, c1 + 1) = moved;
            }
        }
    QVector<TableCell> cells;
    for (int r = 0; r < t->rows; ++r)
        for (int k = 0; k < t->cols; ++k)
            if (k < c0 || k > c1) cells << t->cell(r, k);
    double removed = 0;
    for (int k = c0; k <= c1; ++k) removed += t->colW[k];
    t->colW.remove(c0, n);
    t->cols -= n;
    t->cells = cells;
    for (double &v : t->colW) v += removed / t->cols;
    t->syncRect();
}


namespace {

// A table has at most kMaxTableRowsOrColumns rows and as many columns (a file with more can't be read back), so a command that would make one more says so.
bool roomFor(Editor *ed, bool rows)
{
    const TableItem *t = selectedTable(ed);
    if (!t || (rows ? t->rows : t->cols) < kMaxTableRowsOrColumns) return true;
    Q_EMIT ed->status((rows ? QCoreApplication::translate("TableEdit", "A table has at most %1 rows.") : QCoreApplication::translate("TableEdit", "A table has at most %1 columns."))
                          .arg(kMaxTableRowsOrColumns));
    return false;
}

} // namespace

Editor::TableEdit insertTableRows(Editor *ed, bool below)
{
    if (!roomFor(ed, true)) return {};
    return editRowsAndColumns(ed, QCoreApplication::translate("TableEdit", "Insert Row"), [ed, below](TableItem *t, const CellRange &rg) {
        const int at = below ? (rg.valid() ? rg.r1 + 1 : t->rows) : std::max(0, rg.r0);
        tableInsertRow(ed, t, at);
        return Editor::TableEdit{true, std::clamp(at, 0, t->rows - 1), 1};
    });
}

Editor::TableEdit insertTableColumns(Editor *ed, bool right)
{
    if (!roomFor(ed, false)) return {};
    return editRowsAndColumns(ed, QCoreApplication::translate("TableEdit", "Insert Column"), [ed, right](TableItem *t, const CellRange &rg) {
        const int at = right ? (rg.valid() ? rg.c1 + 1 : t->cols) : std::max(0, rg.c0);
        tableInsertCol(ed, t, at);
        return Editor::TableEdit{false, std::clamp(at, 0, t->cols - 1), 1};
    });
}

namespace {

// Rows and columns can only be deleted by naming cells in them; and a table keeps one of each.
bool canDelete(Editor *ed, bool rows, const QString &pick)
{
    TableItem *t = selectedTable(ed);
    if (!t) return false;
    const CellRange rg = ed->targetCells();
    if (!rg.valid()) {
        Q_EMIT ed->status(pick);
        return false;
    }
    if (rows ? rg.r1 - rg.r0 + 1 >= t->rows : rg.c1 - rg.c0 + 1 >= t->cols) {
        Q_EMIT ed->status(rows ? QCoreApplication::translate("TableEdit", "A table keeps at least one row. Delete Table removes the whole table.")
                               : QCoreApplication::translate("TableEdit", "A table keeps at least one column. Delete Table removes the whole table."));
        return false;
    }
    return true;
}

} // namespace

Editor::TableEdit deleteTableRows(Editor *ed)
{
    if (!canDelete(ed, true, QCoreApplication::translate("TableEdit", "Click in the row to delete, or select cells in the rows, then choose Delete Rows. Delete Table removes the whole table."))) return {};
    return editRowsAndColumns(ed, QCoreApplication::translate("TableEdit", "Delete Rows"), [](TableItem *t, const CellRange &rg) {
        tableDeleteRows(t, rg.r0, rg.r1);
        return Editor::TableEdit{true, rg.r0, -(rg.r1 - rg.r0 + 1)};
    });
}

Editor::TableEdit deleteTableColumns(Editor *ed)
{
    if (!canDelete(ed, false, QCoreApplication::translate("TableEdit", "Click in the column to delete, or select cells in the columns, then choose Delete Columns. Delete Table removes the whole table."))) return {};
    return editRowsAndColumns(ed, QCoreApplication::translate("TableEdit", "Delete Columns"), [](TableItem *t, const CellRange &rg) {
        tableDeleteCols(t, rg.c0, rg.c1);
        return Editor::TableEdit{false, rg.c0, -(rg.c1 - rg.c0 + 1)};
    });
}

void deleteTable(Editor *ed)
{
    if (TableItem *t = selectedTable(ed)) {
        ed->endTextEdit();
        ed->deleteItems({t->id});
    }
}

namespace {

// Joins the cells of a block into its first: their text goes in the first cell's, a paragraph each, and the cells under it are covered.
void joinCells(Editor *ed, TableItem *t, const CellRange &rg)
{
    TableCell &head = t->cell(rg.r0, rg.c0);
    QTextDocument *dst = ed->doc()->storyDoc(head.storyId);
    for (int r = rg.r0; r <= rg.r1; ++r)
        for (int c = rg.c0; c <= rg.c1; ++c) {
            if (r == rg.r0 && c == rg.c0) continue;
            TableCell &cell = t->cell(r, c);
            if (!cell.storyId2.isEmpty()) joinHalves(ed->doc(), cell);   // a divided cell is one again
            // The other cells' text joins the first cell's, a paragraph each.
            QTextDocument *src = cell.covered ? nullptr : ed->doc()->storyDoc(cell.storyId);
            if (dst && src && !src->toPlainText().isEmpty()) {
                QTextCursor dc(dst);
                dc.movePosition(QTextCursor::End);
                if (!dst->toPlainText().isEmpty()) dc.insertBlock();
                dc.insertFragment(QTextDocumentFragment(src));
                QTextCursor sc(src);
                sc.select(QTextCursor::Document);
                sc.removeSelectedText();
            }
            cell.covered = true;
            cell.rowSpan = cell.colSpan = 1;
        }
    head.rowSpan = rg.r1 - rg.r0 + 1;
    head.colSpan = rg.c1 - rg.c0 + 1;
    ed->fitTableRows(t);   // all the words are in one cell now
    dropUnusedStories(ed->doc());
}

} // namespace

void mergeTableCells(Editor *ed)
{
    TableItem *t = selectedTable(ed);
    CellRange rg = ed->targetCells();
    if (!t || !rg.valid()) {
        Q_EMIT ed->status(QCoreApplication::translate("TableEdit", "Select the cells to merge, then choose Merge Cells. With the cursor in one cell, it merges with the cell to its right."));
        return;
    }
    // A single cell joins the cell on its right.
    const TableCell &first = t->cell(rg.r0, rg.c0);
    if (!first.covered && rg.r1 == rg.r0 + first.rowSpan - 1 && rg.c1 == rg.c0 + first.colSpan - 1) {
        if (rg.c1 + 1 >= t->cols) return;
        rg = t->cellsBetween(rg.r0, rg.c0, rg.r1, rg.c1 + 1);
    }
    ed->endTextEdit();
    ed->change(QCoreApplication::translate("TableEdit", "Merge Cells"), [&] { joinCells(ed, t, rg); });
    ed->selectCells(t->id, rg.r0, rg.c0, rg.r1, rg.c1);
}

void splitTableCells(Editor *ed)
{
    TableItem *t = selectedTable(ed);
    const CellRange rg = ed->targetCells();
    if (!t || !rg.valid()) return;
    ed->endTextEdit();
    ed->change(QCoreApplication::translate("TableEdit", "Split Cells"), [&] {
        for (int r = rg.r0; r <= rg.r1; ++r)
            for (int c = rg.c0; c <= rg.c1; ++c) {
                TableCell &cell = t->cell(r, c);
                if (cell.covered || (cell.rowSpan == 1 && cell.colSpan == 1)) continue;
                for (int rr = r; rr < std::min(t->rows, r + cell.rowSpan); ++rr)
                    for (int cc = c; cc < std::min(t->cols, c + cell.colSpan); ++cc)
                        if (rr != r || cc != c) t->cell(rr, cc).covered = false;
                cell.rowSpan = cell.colSpan = 1;
            }
        ed->fitTableRows(t);   // the words of a merged cell now have less room
        dropUnusedStories(ed->doc());
    });
    ed->selectCells(t->id, rg.r0, rg.c0, rg.r1, rg.c1);
}

namespace {

constexpr double kEdgeReach = 4;        // points: how near an edge between cells a stroke along it has to be to draw on (or erase) the edge
constexpr double kStrokeCrossing = 0.5; // the share of a cell's size a stroke across it has to cover to divide it
constexpr double kLeastCell = 6;        // points: a cell is not divided into parts smaller than this

thread_local qint64 g_work = 0;   // grid positions looked at by the pen and the eraser (tests count it)
constexpr int kMostPairsErased = 16;   // an eraser stroke along an edge joins at most this many pairs of cells

// The block (its first cell: column in x, row in y) that each grid position is part of; a position no block claims is its own. A table read from a file may have spans that
// overlap, so the work is held to a few looks at each position (past that, the rest stand alone), and a span is taken as no larger than the table.
QVector<QPoint> blockGrid(const TableItem *t)
{
    QVector<QPoint> heads(t->rows * t->cols, QPoint(-1, -1));
    qint64 budget = qint64(t->rows) * t->cols * 2;
    for (int r = 0; r < t->rows && budget > 0; ++r)
        for (int c = 0; c < t->cols && budget > 0; ++c) {
            const TableCell &cell = t->cell(r, c);
            if (cell.covered) continue;
            for (int rr = r; rr < std::min(t->rows, r + std::max(1, cell.rowSpan)) && budget > 0; ++rr)
                for (int cc = c; cc < std::min(t->cols, c + std::max(1, cell.colSpan)); ++cc) {
                    heads[rr * t->cols + cc] = QPoint(c, r);
                    --budget;
                    ++g_work;
                    if (budget <= 0) break;
                }
        }
    for (int r = 0; r < t->rows; ++r)
        for (int c = 0; c < t->cols; ++c)
            if (heads[r * t->cols + c].x() < 0) heads[r * t->cols + c] = QPoint(c, r);
    return heads;
}

// The position of the edge `k` (0 is the table's first edge) of rows or columns of those sizes, and the edge nearest `at`.
double edgeAt(const QVector<double> &sizes, int k)
{
    double v = 0;
    for (int i = 0; i < k && i < sizes.size(); ++i) v += sizes[i];
    return v;
}

int nearestEdge(const QVector<double> &sizes, double at, double *distance)
{
    int best = 0;
    double bestDistance = std::abs(at);
    double v = 0;
    for (int i = 0; i < sizes.size(); ++i) {
        v += sizes[i];
        if (std::abs(at - v) < bestDistance) {
            bestDistance = std::abs(at - v);
            best = i + 1;
        }
    }
    *distance = bestDistance;
    return best;
}

// A stroke, as along (rows' edges, for a horizontal one) or across, the part of a table it is on.
struct Stroked {
    bool horizontal = true;
    double at = 0, from = 0, to = 0;   // the stroke's place across it (y for a horizontal one), and its ends along it
    bool ok = false;
};

Stroked strokeOf(const QPointF &a, const QPointF &b)
{
    Stroked s;
    const double dx = std::abs(b.x() - a.x()), dy = std::abs(b.y() - a.y());
    s.horizontal = dx >= dy;
    const double along = s.horizontal ? dx : dy, across = s.horizontal ? dy : dx;
    s.ok = along >= 6 && across <= along * 0.35;
    s.at = s.horizontal ? (a.y() + b.y()) / 2 : (a.x() + b.x()) / 2;
    s.from = s.horizontal ? std::min(a.x(), b.x()) : std::min(a.y(), b.y());
    s.to = s.horizontal ? std::max(a.x(), b.x()) : std::max(a.y(), b.y());
    return s;
}

// The blocks that touch edge `k` between rows (columns), on the side before or after it, that a stroke from..to covers at least half of.
QVector<QPoint> blocksAlongEdge(const TableItem *t, const QVector<QPoint> &heads, const Stroked &s, int k, bool after)
{
    QVector<QPoint> out;
    const int count = s.horizontal ? t->cols : t->rows;
    const int line = after ? k : k - 1;
    if (line < 0 || line >= (s.horizontal ? t->rows : t->cols)) return out;
    for (int i = 0; i < count; ++i) {
        const QPoint h = heads[s.horizontal ? line * t->cols + i : i * t->cols + line];
        const TableCell &cell = t->cell(h.y(), h.x());
        // The block must end (begin) at this edge, not run through it.
        if (s.horizontal ? (after ? h.y() != k : h.y() + cell.rowSpan - 1 != k - 1) : (after ? h.x() != k : h.x() + cell.colSpan - 1 != k - 1)) continue;
        if (out.contains(h)) continue;
        const QRectF rc = t->cellRect(h.y(), h.x());
        const double lo = s.horizontal ? rc.left() : rc.top(), hi = s.horizontal ? rc.right() : rc.bottom();
        if (std::min(hi, s.to) - std::max(lo, s.from) >= (hi - lo) * kStrokeCrossing) out << h;
    }
    return out;
}

} // namespace

// Divides a merged cell (its first cell `head`, column in x and row in y) at the edge `edge` inside it, between rows (`horizontal`) or columns: the first part keeps the words, the
// second is a cell of its own with the cell's look, and the new edge has `pen`'s line.
void splitMerged(Editor *ed, TableItem *t, const QPoint &head, bool horizontal, int edge, const Stroke &pen)
{
    const int row = head.y(), col = head.x();
    TableCell &first = t->cell(row, col);
    const int rows = first.rowSpan, cols = first.colSpan;
    TableCell &second = horizontal ? t->cell(edge, col) : t->cell(row, edge);
    const QString kept = second.storyId;   // (a covered cell kept its own, empty, text)
    second = first;
    second.covered = false;
    second.storyId = kept.isEmpty() ? ed->doc()->createStory() : kept;
    second.storyId2.clear();
    if (horizontal) {
        second.rowSpan = row + rows - edge;
        first.rowSpan = edge - row;
        first.border.bottom = pen;
        second.border.top = pen;
    } else {
        second.colSpan = col + cols - edge;
        first.colSpan = edge - col;
        first.border.right = pen;
        second.border.left = pen;
    }
}

qint64 tableEditWork(bool reset)
{
    const qint64 w = g_work;
    if (reset) g_work = 0;
    return w;
}

TableStroke drawTableLine(Editor *ed, TableItem *t, const QPointF &a, const QPointF &b, const Stroke &pen)
{
    if (!t || t->rows < 1 || t->cols < 1) return TableStroke::Nothing;
    const Stroked s = strokeOf(a, b);
    if (!s.ok) {
        Q_EMIT ed->status(QCoreApplication::translate("TableEdit", "Draw a straight line along the edge of a cell, or across a cell to divide it."));
        return TableStroke::Nothing;
    }
    const QVector<double> &across = s.horizontal ? t->rowH : t->colW;
    double distance = 0;
    const int k = nearestEdge(across, s.at, &distance);
    const QVector<QPoint> heads = blockGrid(t);
    if (distance <= kEdgeReach) {
        // Along an edge: the pen's line on it, for the cells on each side.
        const QVector<QPoint> before = blocksAlongEdge(t, heads, s, k, false), after = blocksAlongEdge(t, heads, s, k, true);
        // The merged cells the edge runs through (covered a half or more by the stroke) are divided at it; the cells that end at the edge get the border.
        QVector<QPoint> through;
        if (k > 0 && k < (s.horizontal ? t->rows : t->cols))
            for (int i = 0; i < (s.horizontal ? t->cols : t->rows); ++i) {
                const QPoint h = heads[s.horizontal ? (k - 1) * t->cols + i : i * t->cols + (k - 1)];
                const TableCell &cell = t->cell(h.y(), h.x());
                if ((s.horizontal ? h.y() + cell.rowSpan - 1 : h.x() + cell.colSpan - 1) < k || through.contains(h)) continue;
                const QRectF rc = t->cellRect(h.y(), h.x());
                const double lo = s.horizontal ? rc.left() : rc.top(), hi = s.horizontal ? rc.right() : rc.bottom();
                if (std::min(hi, s.to) - std::max(lo, s.from) >= (hi - lo) * kStrokeCrossing) through << h;
            }
        if (!before.isEmpty() || !after.isEmpty() || !through.isEmpty()) {
            ed->endTextEdit();
            ed->change(QCoreApplication::translate("TableEdit", "Draw Border"), [&] {
                for (const QPoint &h : before) (s.horizontal ? t->cell(h.y(), h.x()).border.bottom : t->cell(h.y(), h.x()).border.right) = pen;
                for (const QPoint &h : after) (s.horizontal ? t->cell(h.y(), h.x()).border.top : t->cell(h.y(), h.x()).border.left) = pen;
                for (const QPoint &h : through) splitMerged(ed, t, h, s.horizontal, k, pen);
                if (!through.isEmpty()) ed->fitTableRows(t);
            });
            return through.isEmpty() ? TableStroke::Border : TableStroke::Split;
        }
    }
    // Across a cell: the cell under the middle of the stroke, if the stroke covers enough of it.
    int row = 0, col = 0;
    t->cellAt(s.horizontal ? QPointF((s.from + s.to) / 2, s.at) : QPointF(s.at, (s.from + s.to) / 2), &row, &col);
    const QPoint head = heads[row * t->cols + col];
    row = head.y();
    col = head.x();
    const TableCell original = t->cell(row, col);
    const QRectF rc = t->cellRect(row, col);
    const double lo = s.horizontal ? rc.left() : rc.top(), hi = s.horizontal ? rc.right() : rc.bottom();
    if (std::min(hi, s.to) - std::max(lo, s.from) < (hi - lo) * kStrokeCrossing) {
        Q_EMIT ed->status(QCoreApplication::translate("TableEdit", "Draw the line all the way across the cell to divide it."));
        return TableStroke::Nothing;
    }
    const int span = s.horizontal ? original.rowSpan : original.colSpan;
    const double start = s.horizontal ? rc.top() : rc.left(), size = s.horizontal ? rc.height() : rc.width();
    ed->endTextEdit();
    if (span > 1) {
        // A merged cell: the merge ends at the edge inside it that is nearest.
        int edge = (s.horizontal ? row : col) + 1;
        double best = 1e9;
        for (int e = (s.horizontal ? row : col) + 1; e < (s.horizontal ? row : col) + span; ++e) {
            const double d = std::abs(edgeAt(across, e) - s.at);
            if (d < best) {
                best = d;
                edge = e;
            }
        }
        ed->change(QCoreApplication::translate("TableEdit", "Split Cells"), [&] {
            splitMerged(ed, t, head, s.horizontal, edge, pen);
            ed->fitTableRows(t);
        });
        return TableStroke::Split;
    }
    // A cell of one row (column): the table gets one more of them, and the cell is divided between the two.
    if ((s.horizontal ? t->rows : t->cols) >= kMaxTableRowsOrColumns) {
        Q_EMIT ed->status((s.horizontal ? QCoreApplication::translate("TableEdit", "A table has at most %1 rows.") : QCoreApplication::translate("TableEdit", "A table has at most %1 columns."))
                              .arg(kMaxTableRowsOrColumns));
        return TableStroke::Nothing;
    }
    const double first = s.at - start, second = size - first;
    if (first < kLeastCell || second < kLeastCell) {
        Q_EMIT ed->status(QCoreApplication::translate("TableEdit", "The cell is too small to divide there."));
        return TableStroke::Nothing;
    }
    // The cells of the other columns (rows) in this row (column) that end here are made one bigger, so that they run across the new one.
    const int line = s.horizontal ? row : col;
    QVector<QPoint> through;
    for (int i = 0; i < (s.horizontal ? t->cols : t->rows); ++i) {
        const QPoint h = heads[s.horizontal ? line * t->cols + i : i * t->cols + line];
        const TableCell &cell = t->cell(h.y(), h.x());
        if (h == head || through.contains(h)) continue;
        if ((s.horizontal ? h.y() + cell.rowSpan - 1 : h.x() + cell.colSpan - 1) == line) through << h;
    }
    const QVector<double> oldWidths = t->colW;
    ed->change(QCoreApplication::translate("TableEdit", "Split Cells"), [&] {
        Editor::TableEdit done;
        if (s.horizontal) {
            tableInsertRow(ed, t, row + 1);
            t->rowH[row] = first;
            t->rowH[row + 1] = second;
            t->syncRect();
            done = Editor::TableEdit{true, row + 1, 1};
        } else {
            tableInsertCol(ed, t, col + 1);
            t->colW = oldWidths;
            t->colW[col] = first;
            t->colW.insert(col + 1, second);
            t->syncRect();
            done = Editor::TableEdit{false, col + 1, 1};
        }
        for (const QPoint &h : through) {
            TableCell &cell = t->cell(h.y(), h.x());
            if (s.horizontal) {
                ++cell.rowSpan;
                for (int cc = h.x(); cc < std::min(t->cols, h.x() + cell.colSpan); ++cc) t->cell(row + 1, cc).covered = true;
            } else {
                ++cell.colSpan;
                for (int rr = h.y(); rr < std::min(t->rows, h.y() + cell.rowSpan); ++rr) t->cell(rr, col + 1).covered = true;
            }
        }
        // The new cell is as wide (tall) as the one divided, and has what it had.
        TableCell &now = t->cell(row, col);
        TableCell &made = s.horizontal ? t->cell(row + 1, col) : t->cell(row, col + 1);
        if (s.horizontal) {
            made.colSpan = original.colSpan;
            for (int cc = col + 1; cc < std::min(t->cols, col + original.colSpan); ++cc) t->cell(row + 1, cc).covered = true;
        } else {
            made.rowSpan = original.rowSpan;
            for (int rr = row + 1; rr < std::min(t->rows, row + original.rowSpan); ++rr) t->cell(rr, col + 1).covered = true;
        }
        if (ed->tableEdited) ed->tableEdited(t, done);
        made.fill = original.fill;
        made.valign = original.valign;
        made.margins = original.margins;
        made.border = original.border;
        if (s.horizontal) {
            now.border.bottom = pen;
            made.border.top = pen;
        } else {
            now.border.right = pen;
            made.border.left = pen;
        }
        dropUnusedStories(ed->doc());
        ed->fitTableRows(t);
    });
    return TableStroke::Split;
}

TableStroke eraseTableLine(Editor *ed, TableItem *t, const QPointF &a, const QPointF &b)
{
    if (!t || t->rows < 1 || t->cols < 1) return TableStroke::Nothing;
    const Stroked s = strokeOf(a, b);
    double distance = 0;
    if (s.ok) {
        const QVector<double> &across = s.horizontal ? t->rowH : t->colW;
        const int k = nearestEdge(across, s.at, &distance);
        if (distance <= kEdgeReach) {
            const int edges = s.horizontal ? t->rows : t->cols;
            QVector<QPoint> heads = blockGrid(t);
            if (k == 0 || k == edges) {
                // The table's outside edge: nothing to join, so the border goes.
                const QVector<QPoint> side = blocksAlongEdge(t, heads, s, k, k == 0);
                if (side.isEmpty()) {
                    Q_EMIT ed->status(QCoreApplication::translate("TableEdit", "Drag the eraser along the whole edge of a cell to erase its border."));
                    return TableStroke::Nothing;
                }
                ed->endTextEdit();
                ed->change(QCoreApplication::translate("TableEdit", "Erase Border"), [&] {
                    for (const QPoint &h : side) {
                        CellBorder &bd = t->cell(h.y(), h.x()).border;
                        (s.horizontal ? (k == 0 ? bd.top : bd.bottom) : (k == 0 ? bd.left : bd.right)) = Stroke::none();
                    }
                });
                return TableStroke::Border;
            }
            // Between two rows (columns): each pair of cells on the two sides of the stroke is joined.
            const QVector<QPoint> before = blocksAlongEdge(t, heads, s, k, false);
            if (before.isEmpty()) {
                Q_EMIT ed->status(QCoreApplication::translate("TableEdit", "There is no edge between two cells there to erase (a merged cell has none inside it)."));
                return TableStroke::Nothing;
            }
            ed->endTextEdit();
            ed->change(QCoreApplication::translate("TableEdit", "Merge Cells"), [&] {
                int pairs = 0;
                for (const QPoint &first : before) {
                    if (++pairs > kMostPairsErased) {
                        Q_EMIT ed->status(QCoreApplication::translate("TableEdit", "The eraser joins at most %1 pairs of cells in one stroke. Drag it again for the rest.").arg(kMostPairsErased));
                        break;
                    }
                    heads = blockGrid(t);   // (the cells joined before this one changed the blocks)
                    const QPoint h = heads[first.y() * t->cols + first.x()];
                    const TableCell &cell = t->cell(h.y(), h.x());
                    const int nextRow = s.horizontal ? h.y() + cell.rowSpan : h.y(), nextCol = s.horizontal ? h.x() : h.x() + cell.colSpan;
                    if (nextRow >= t->rows || nextCol >= t->cols || (s.horizontal ? nextRow : nextCol) != k) continue;
                    const QPoint other = heads[nextRow * t->cols + nextCol];
                    joinCells(ed, t, t->cellsBetween(h.y(), h.x(), other.y(), other.x()));
                }
            });
            return TableStroke::Joined;
        }
    }
    // From one cell to another: those cells, and the ones between, are one.
    int r0 = 0, c0 = 0, r1 = 0, c1 = 0;
    t->cellAt(a, &r0, &c0);
    t->cellAt(b, &r1, &c1);
    const CellRange rg = t->cellsBetween(r0, c0, r1, c1);
    if (!rg.valid() || (rg.r0 == rg.r1 && rg.c0 == rg.c1)) {
        Q_EMIT ed->status(QCoreApplication::translate("TableEdit", "Drag the eraser from one cell to the next, or along the edge between them, to join them."));
        return TableStroke::Nothing;
    }
    const TableCell &only = t->cell(rg.r0, rg.c0);
    if (!only.covered && rg.r1 == rg.r0 + only.rowSpan - 1 && rg.c1 == rg.c0 + only.colSpan - 1) {   // (one merged cell already)
        Q_EMIT ed->status(QCoreApplication::translate("TableEdit", "Those cells are already one cell."));
        return TableStroke::Nothing;
    }
    ed->endTextEdit();
    ed->change(QCoreApplication::translate("TableEdit", "Merge Cells"), [&] { joinCells(ed, t, rg); });
    return TableStroke::Joined;
}

void selectTable(Editor *ed)
{
    if (TableItem *t = selectedTable(ed)) {
        ed->endTextEdit();
        ed->select(t->id);
    }
}

// A row or column of the cell the cursor is in; from a block, every row or column it covers.
void selectTableRow(Editor *ed)
{
    TableItem *t = selectedTable(ed);
    const CellRange rg = ed->targetCells();
    if (!t || !rg.valid()) {
        Q_EMIT ed->status(QCoreApplication::translate("TableEdit", "Click in a cell, then choose Select Row."));
        return;
    }
    ed->selectCells(t->id, rg.r0, 0, rg.r1, t->cols - 1);
}

void selectTableColumn(Editor *ed)
{
    TableItem *t = selectedTable(ed);
    const CellRange rg = ed->targetCells();
    if (!t || !rg.valid()) {
        Q_EMIT ed->status(QCoreApplication::translate("TableEdit", "Click in a cell, then choose Select Column."));
        return;
    }
    ed->selectCells(t->id, 0, rg.c0, t->rows - 1, rg.c1);
}

void selectTableCell(Editor *ed)
{
    // From a block, the cell it began with.
    if (const auto b = ed->cellBlock(); b.range.valid()) ed->beginTextEdit(b.itemId, 0, b.anchorRow, b.anchorCol);
    if (!ed->isEditingText()) return;
    QTextCursor c = ed->cursor();
    c.select(QTextCursor::Document);
    ed->setCursor(c);
}

void distributeTableRows(Editor *ed)
{
    if (TableItem *t = selectedTable(ed)) ed->change(QCoreApplication::translate("TableEdit", "Distribute Rows"), [t] {
        double total = 0;
        for (double h : t->rowH) total += h;
        for (double &h : t->rowH) h = total / t->rows;
    });
}

void distributeTableColumns(Editor *ed)
{
    if (TableItem *t = selectedTable(ed)) ed->change(QCoreApplication::translate("TableEdit", "Distribute Columns"), [t] {
        double total = 0;
        for (double w : t->colW) total += w;
        for (double &w : t->colW) w = total / t->cols;
    });
}

void setTableRowHeight(Editor *ed, double points)
{
    TableItem *t = selectedTable(ed);
    if (!t || t->locked || points <= 0) return;
    const CellRange rg = ed->cellsToFormat(t);
    ed->change(QCoreApplication::translate("TableEdit", "Row Height"), [&] {
        for (int r = rg.r0; r <= rg.r1; ++r) t->rowH[r] = points;
        t->syncRect();
        ed->fitTableRows(t);   // a row is never shorter than its words
    });
}

void setTableColumnWidth(Editor *ed, double points)
{
    TableItem *t = selectedTable(ed);
    if (!t || t->locked || points <= 0) return;
    const CellRange rg = ed->cellsToFormat(t);
    ed->change(QCoreApplication::translate("TableEdit", "Column Width"), [&] {
        for (int c = rg.c0; c <= rg.c1; ++c) t->colW[c] = points;
        t->syncRect();
        ed->fitTableRows(t);   // narrower columns may need taller rows
    });
}

void setTableShading(Editor *ed, const ColorRef &color)
{
    TableItem *tb = selectedTable(ed);
    if (!tb) return;
    // The selected cells, the cell the text cursor is in, or with the table selected, every cell.
    const CellRange rg = ed->targetCells();
    ed->change(QCoreApplication::translate("TableEdit", "Fill"), [&] {
        for (int rr = 0; rr < tb->rows; ++rr)
            for (int cc = 0; cc < tb->cols; ++cc)
                if (!rg.valid() || rg.contains(rr, cc)) tb->cell(rr, cc).fill = color.isNone() ? Fill() : Fill::solid(color);
    });
}

void setTableCellAlignment(Editor *ed, VAlign align)
{
    TableItem *tb = selectedTable(ed);
    if (!tb) return;
    // The selected cells, the cell the text cursor is in, or with the table selected, every cell.
    const CellRange rg = ed->cellsToFormat(tb);
    ed->change(QCoreApplication::translate("TableEdit", "Cell Alignment"), [tb, rg, align] {
        for (int r = rg.r0; r <= rg.r1; ++r)
            for (int c = rg.c0; c <= rg.c1; ++c) tb->cell(r, c).valign = align;
    });
}

void toggleTableTextDirection(Editor *ed)
{
    TableItem *tb = selectedTable(ed);
    if (!tb) return;
    // The selected cells, the cell the text cursor is in, or with the table selected, every cell (all turned, unless they all are already).
    const CellRange rg = ed->cellsToFormat(tb);
    QVector<TableCell *> cells;
    for (int rr = rg.r0; rr <= rg.r1; ++rr)
        for (int cc = rg.c0; cc <= rg.c1; ++cc) cells << &tb->cell(rr, cc);
    const bool all = std::all_of(cells.begin(), cells.end(), [](const TableCell *x) { return x->vertical; });
    ed->change(QCoreApplication::translate("TableEdit", "Text Direction"), [&] {
        for (TableCell *x : cells) x->vertical = !all;
        ed->fitTableRows(tb);   // turned back, the text may need more room
    });
}

void setTableCellMargins(Editor *ed, const QMarginsF &margins)
{
    TableItem *tb = selectedTable(ed);
    if (!tb) return;
    // The selected cells, the cell the text cursor is in, or with the table selected, every cell.
    const CellRange rg = ed->cellsToFormat(tb);
    ed->change(QCoreApplication::translate("TableEdit", "Cell Margins"), [tb, margins, rg] {
        for (int r = rg.r0; r <= rg.r1; ++r)
            for (int c = rg.c0; c <= rg.c1; ++c) tb->cell(r, c).margins = margins;
    });
}

void setTableBorders(Editor *ed, BorderSides sides, const Stroke &line)
{
    TableItem *t = selectedTable(ed);
    if (!t) return;
    const Stroke s = sides == BorderSides::None ? Stroke::none() : line;
    const CellRange rg = ed->cellsToFormat(t);
    ed->change(QCoreApplication::translate("TableEdit", "Borders"), [&] {
        for (int r = rg.r0; r <= rg.r1; ++r)
            for (int c = rg.c0; c <= rg.c1; ++c) {
                // A merged cell draws its borders round the whole of it.
                const TableCell &cell = t->cell(r, c);
                if (cell.covered) continue;
                CellBorder &b = t->cell(r, c).border;
                const bool top = r == rg.r0, bottom = r + cell.rowSpan - 1 >= rg.r1, left = c == rg.c0, right = c + cell.colSpan - 1 >= rg.c1;
                switch (sides) {
                case BorderSides::All:
                case BorderSides::None: b.top = b.bottom = b.left = b.right = s; break;
                case BorderSides::Outside: if (top) b.top = s; if (bottom) b.bottom = s; if (left) b.left = s; if (right) b.right = s; break;
                case BorderSides::Inside: if (!top) b.top = s; if (!bottom) b.bottom = s; if (!left) b.left = s; if (!right) b.right = s; break;
                case BorderSides::Top: if (top) b.top = s; break;
                case BorderSides::Bottom: if (bottom) b.bottom = s; break;
                case BorderSides::Left: if (left) b.left = s; break;
                case BorderSides::Right: if (right) b.right = s; break;
                case BorderSides::InsideHorizontal: if (!top) b.top = s; if (!bottom) b.bottom = s; break;
                case BorderSides::InsideVertical: if (!left) b.left = s; if (!right) b.right = s; break;
                }
            }
    });
}

namespace {

// The text color a style asks for, but not the ordinary one (dark 1), which a cell's text has when it asks for none.
QColor styleColor(const QColor &c, const office::Theme &theme) { return c.isValid() && c != theme.colors[office::Theme::Dark1] ? c : QColor(); }

bool sameColor(const QString &stored, const QColor &c)
{
    if (stored.isEmpty() || !c.isValid()) return stored.isEmpty() && !c.isValid();
    const QColor have = colorFromString(stored);
    return have.isValid() && have.rgb() == c.rgb();
}

// What a cell's text had from the old style, now from the new: the characters whose bold, italic, or color is what the old style gave
// (or, with the old style saying nothing, are plain) change to what the new style gives; any other they were set to is their own.
struct TextLook {
    std::optional<bool> bold, italic;
    QColor color;
};

void restyleText(QTextDocument *doc, const TextLook &was, const TextLook &now)
{
    if (!doc) return;
    const auto changed = [&](QTextCharFormat f, bool *any) {
        const bool bold = f.fontWeight() >= QFont::DemiBold;
        if (bold == was.bold.value_or(false) && bold != now.bold.value_or(false)) {
            f.setFontWeight(now.bold.value_or(false) ? QFont::Bold : QFont::Normal);
            *any = true;
        }
        if (f.fontItalic() == was.italic.value_or(false) && f.fontItalic() != now.italic.value_or(false)) {
            f.setFontItalic(now.italic.value_or(false));
            *any = true;
        }
        if (sameColor(f.stringProperty(tp::ColorRefP), was.color) && !sameColor(f.stringProperty(tp::ColorRefP), now.color)) {
            if (now.color.isValid()) f.setProperty(tp::ColorRefP, ColorRef::rgb(now.color).toString());
            else f.clearProperty(tp::ColorRefP);
            *any = true;
        }
        return f;
    };
    struct Run {
        int from, to;
        QTextCharFormat format;
    };
    QVector<Run> runs;
    QVector<QPair<int, QTextCharFormat>> blocks;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        bool any = false;
        const QTextCharFormat blockFormat = changed(b.charFormat(), &any);
        if (any) blocks.append({b.position(), blockFormat});
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            any = false;
            const QTextCharFormat f = changed(fr.charFormat(), &any);
            if (any) runs.append({fr.position(), fr.position() + fr.length(), f});
        }
    }
    QTextCursor cur(doc);
    cur.beginEditBlock();
    for (const Run &r : runs) {
        cur.setPosition(r.from);
        cur.setPosition(r.to, QTextCursor::KeepAnchor);
        cur.setCharFormat(r.format);
    }
    for (const auto &b : blocks) {
        cur.setPosition(b.first);
        cur.setBlockCharFormat(b.second);
    }
    cur.endEditBlock();
}

} // namespace

void giveTableStyle(Editor *ed, TableItem *t, const QString &styleId, const TableStyleParts &parts, const TableStyles &styles, bool reset)
{
    // What the old style gave, to tell it from what the person did.
    office::TableStyleEffect was, now;
    const bool hadStyle = office::tableStyleEffect(t->format, *t, styles.theme, &was, &styles.defs);
    t->header = parts.header;
    t->totalRow = parts.totalRow;
    t->firstCol = parts.firstCol;
    t->lastCol = parts.lastCol;
    t->banded = parts.banded;
    t->bandedCols = parts.bandedCols;
    t->format = styleId;
    if (!office::tableStyleEffect(styleId, *t, styles.theme, &now, &styles.defs)) {
        // One of JeffPub's own: its parts are header and banded rows, and applying it again shows them.
        if (!styleId.isEmpty() && !styleId.startsWith(QLatin1Char('{'))) ed->applyTableFormat(t, styleId);
        return;
    }
    // With no old style (none, or one this program doesn't know) a cell is taken to be as the person left it unless it has nothing.
    const Stroke none;
    for (int r = 0; r < t->rows; ++r)
        for (int c = 0; c < t->cols; ++c) {
            // Every cell, one a merged cell covers too: it is what a split makes of it.
            TableCell &cell = t->cell(r, c);
            const size_t at = size_t(r) * t->cols + c;
            const int bottom = std::min(r + cell.rowSpan, t->rows), right = std::min(c + cell.colSpan, t->cols);
            if (reset || cell.fill == (hadStyle ? was.fill[at] : Fill())) cell.fill = now.fill[at];
            const auto take = [&](Stroke &edge, const Stroke &before, const Stroke &after) {
                if (reset || edge == (hadStyle ? before : none)) edge = after;
            };
            take(cell.border.top, hadStyle ? was.above(r, c) : none, now.above(r, c));
            take(cell.border.bottom, hadStyle ? was.above(bottom, c) : none, now.above(bottom, c));
            take(cell.border.left, hadStyle ? was.leftOf(r, c) : none, now.leftOf(r, c));
            take(cell.border.right, hadStyle ? was.leftOf(r, right) : none, now.leftOf(r, right));
            TextLook before, after;
            if (hadStyle) {
                before.bold = was.bold[at];
                before.italic = was.italic[at];
                before.color = styleColor(was.color[at], styles.theme);
            }
            after.bold = now.bold[at];
            after.italic = now.italic[at];
            after.color = styleColor(now.color[at], styles.theme);
            restyleText(ed->doc()->storyDoc(cell.storyId), before, after);
            restyleText(ed->doc()->storyDoc(cell.storyId2), before, after);
        }
    ed->fitTableRows(t);   // bold words are wider, and the first row may now be taller
}

void keepTableStyle(Editor *ed, TableItem *t, const Editor::TableEdit &edit, const TableStyles &styles)
{
    // The table as it was: the same style and parts, and the rows or columns it had then.
    TableItem was;
    was.rows = t->rows - (edit.rows ? edit.count : 0);
    was.cols = t->cols - (edit.rows ? 0 : edit.count);
    was.header = t->header;
    was.totalRow = t->totalRow;
    was.firstCol = t->firstCol;
    was.lastCol = t->lastCol;
    was.banded = t->banded;
    was.bandedCols = t->bandedCols;
    office::TableStyleEffect before, now;
    if (was.rows <= 0 || was.cols <= 0 || !office::tableStyleEffect(t->format, was, styles.theme, &before, &styles.defs) || !office::tableStyleEffect(t->format, *t, styles.theme, &now, &styles.defs)) return;
    // Which row (column) of the table before is now at i: none for a new one.
    const auto oldIndex = [&](int i) {
        if (edit.count > 0) return i < edit.at ? i : i < edit.at + edit.count ? -1 : i - edit.count;
        return i < edit.at ? i : i - edit.count;
    };
    const Stroke none;
    for (int r = 0; r < t->rows; ++r)
        for (int c = 0; c < t->cols; ++c) {
            TableCell &cell = t->cell(r, c);
            const int oldRow = edit.rows ? oldIndex(r) : r, oldCol = edit.rows ? c : oldIndex(c);
            const bool fresh = oldRow < 0 || oldCol < 0;
            const size_t at = size_t(r) * t->cols + c, oldAt = fresh ? 0 : size_t(oldRow) * was.cols + oldCol;
            if (fresh || cell.fill == before.fill[oldAt]) cell.fill = now.fill[at];
            const auto take = [&](Stroke &edge, const Stroke &after, bool exists, const Stroke &old) {
                if (fresh || (exists && edge == old)) edge = after;
            };
            const int bottom = std::min(r + cell.rowSpan, t->rows), right = std::min(c + cell.colSpan, t->cols);
            const int oldBottom = oldRow + cell.rowSpan, oldRight = oldCol + cell.colSpan;
            take(cell.border.top, now.above(r, c), !fresh, fresh ? none : before.above(oldRow, oldCol));
            take(cell.border.bottom, now.above(bottom, c), !fresh && oldBottom <= was.rows, fresh || oldBottom > was.rows ? none : before.above(oldBottom, oldCol));
            take(cell.border.left, now.leftOf(r, c), !fresh, fresh ? none : before.leftOf(oldRow, oldCol));
            take(cell.border.right, now.leftOf(r, right), !fresh && oldRight <= was.cols, fresh || oldRight > was.cols ? none : before.leftOf(oldRow, oldRight));
            TextLook oldLook, newLook;
            if (!fresh) {
                oldLook.bold = before.bold[oldAt];
                oldLook.italic = before.italic[oldAt];
                oldLook.color = styleColor(before.color[oldAt], styles.theme);
            }
            newLook.bold = now.bold[at];
            newLook.italic = now.italic[at];
            newLook.color = styleColor(now.color[at], styles.theme);
            restyleText(ed->doc()->storyDoc(cell.storyId), oldLook, newLook);
            restyleText(ed->doc()->storyDoc(cell.storyId2), oldLook, newLook);
        }
    ed->fitTableRows(t);
}

void setTableStyle(Editor *ed, TableItem *t, const QString &styleId, const TableStyleParts &parts, const TableStyles &styles, bool reset)
{
    if (!t) return;
    ed->endTextEdit();
    ed->change(QCoreApplication::translate("TableEdit", "Table Style"), [&] { giveTableStyle(ed, t, styleId, parts, styles, reset); });
    ed->select(t->id);
}

void useFirstRowAsHeader(Editor *ed, TableItem *t, const TableStyles &styles)
{
    if (!t) return;
    TableStyleParts parts = TableStyleParts::of(*t);
    parts.header = true;
    // A style the program knows gives each cell its place in it, and keeps what the person did; one of JeffPub's own formats is applied to the first row only.
    TableItem probe;
    probe.rows = t->rows;
    probe.cols = t->cols;
    office::TableStyleEffect effect;
    if (office::tableStyleEffect(t->format, probe, styles.theme, &effect, &styles.defs)) {
        setTableStyle(ed, t, t->format, parts, styles, false);
        return;
    }
    ed->endTextEdit();
    ed->change(QCoreApplication::translate("TableEdit", "Header Row"), [&] {
        t->header = true;
        if (!t->format.isEmpty() && !t->format.startsWith(QLatin1Char('{'))) ed->applyTableFormat(t, t->format, true);
    });
    ed->select(t->id);
}

} // namespace jp
