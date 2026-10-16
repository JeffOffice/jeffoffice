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
// and nothing to delete).
void editRowsAndColumns(Editor *ed, const QString &label, const std::function<void(TableItem *, const CellRange &)> &fn)
{
    TableItem *t = selectedTable(ed);
    if (!t) return;
    const CellRange rg = ed->targetCells();
    ed->endTextEdit();
    ed->change(label, [&] {
        fn(t, rg);
        dropUnusedStories(ed->doc());
    });
    ed->select(t->id);
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


void insertTableRows(Editor *ed, bool below)
{
    editRowsAndColumns(ed, QCoreApplication::translate("TableEdit", "Insert Row"), [ed, below](TableItem *t, const CellRange &rg) {
        tableInsertRow(ed, t, below ? (rg.valid() ? rg.r1 + 1 : t->rows) : std::max(0, rg.r0));
    });
}

void insertTableColumns(Editor *ed, bool right)
{
    editRowsAndColumns(ed, QCoreApplication::translate("TableEdit", "Insert Column"), [ed, right](TableItem *t, const CellRange &rg) {
        tableInsertCol(ed, t, right ? (rg.valid() ? rg.c1 + 1 : t->cols) : std::max(0, rg.c0));
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

bool deleteTableRows(Editor *ed)
{
    if (!canDelete(ed, true, QCoreApplication::translate("TableEdit", "Click in the row to delete, or select cells in the rows, then choose Delete Rows. Delete Table removes the whole table."))) return false;
    editRowsAndColumns(ed, QCoreApplication::translate("TableEdit", "Delete Rows"), [](TableItem *t, const CellRange &rg) { tableDeleteRows(t, rg.r0, rg.r1); });
    return true;
}

bool deleteTableColumns(Editor *ed)
{
    if (!canDelete(ed, false, QCoreApplication::translate("TableEdit", "Click in the column to delete, or select cells in the columns, then choose Delete Columns. Delete Table removes the whole table."))) return false;
    editRowsAndColumns(ed, QCoreApplication::translate("TableEdit", "Delete Columns"), [](TableItem *t, const CellRange &rg) { tableDeleteCols(t, rg.c0, rg.c1); });
    return true;
}

void deleteTable(Editor *ed)
{
    if (TableItem *t = selectedTable(ed)) {
        ed->endTextEdit();
        ed->deleteItems({t->id});
    }
}

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
    ed->change(QCoreApplication::translate("TableEdit", "Merge Cells"), [&] {
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
    });
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

void giveTableStyle(Editor *ed, TableItem *t, const QString &styleId, const TableStyleParts &parts, const office::Theme &theme, bool reset)
{
    // What the old style gave, to tell it from what the person did.
    office::TableStyleEffect was, now;
    const bool hadStyle = office::tableStyleEffect(t->format, *t, theme, &was);
    t->header = parts.header;
    t->totalRow = parts.totalRow;
    t->firstCol = parts.firstCol;
    t->lastCol = parts.lastCol;
    t->banded = parts.banded;
    t->bandedCols = parts.bandedCols;
    t->format = styleId;
    if (!office::tableStyleEffect(styleId, *t, theme, &now)) {
        // One of JeffPub's own: its parts are header and banded rows, and applying it again shows them.
        if (!styleId.isEmpty() && !styleId.startsWith(QLatin1Char('{'))) ed->applyTableFormat(t, styleId);
        return;
    }
    // With no old style (none, or one this program doesn't know) a cell is taken to be as the person left it unless it has nothing.
    const Stroke none;
    for (int r = 0; r < t->rows; ++r)
        for (int c = 0; c < t->cols; ++c) {
            TableCell &cell = t->cell(r, c);
            if (cell.covered) continue;
            const size_t at = size_t(r) * t->cols + c;
            if (reset || cell.fill == (hadStyle ? was.fill[at] : Fill())) cell.fill = now.fill[at];
            const auto take = [&](Stroke &edge, const Stroke &before, const Stroke &after) {
                if (reset || edge == (hadStyle ? before : none)) edge = after;
            };
            take(cell.border.top, hadStyle ? was.above(r, c) : none, now.above(r, c));
            take(cell.border.bottom, hadStyle ? was.above(r + cell.rowSpan, c) : none, now.above(r + cell.rowSpan, c));
            take(cell.border.left, hadStyle ? was.leftOf(r, c) : none, now.leftOf(r, c));
            take(cell.border.right, hadStyle ? was.leftOf(r, c + cell.colSpan) : none, now.leftOf(r, c + cell.colSpan));
            TextLook before, after;
            if (hadStyle) {
                before.bold = was.bold[at];
                before.italic = was.italic[at];
                before.color = styleColor(was.color[at], theme);
            }
            after.bold = now.bold[at];
            after.italic = now.italic[at];
            after.color = styleColor(now.color[at], theme);
            restyleText(ed->doc()->storyDoc(cell.storyId), before, after);
            restyleText(ed->doc()->storyDoc(cell.storyId2), before, after);
        }
    ed->fitTableRows(t);   // bold words are wider, and the first row may now be taller
}

void setTableStyle(Editor *ed, TableItem *t, const QString &styleId, const TableStyleParts &parts, const office::Theme &theme, bool reset)
{
    if (!t) return;
    ed->endTextEdit();
    ed->change(QCoreApplication::translate("TableEdit", "Table Style"), [&] { giveTableStyle(ed, t, styleId, parts, theme, reset); });
    ed->select(t->id);
}

} // namespace jp
