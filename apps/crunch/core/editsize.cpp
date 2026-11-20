// The width of columns and the height of rows: one step each, with the runs of columns (the specs of rows) as they were and as they are. See EditSession::resizeColumns.
#include "core/editsession.h"

#include "core/formularefs.h"

#include <algorithm>
#include <map>

#include <QThread>

namespace jp::crunch {

using namespace jp::office::sheet;

namespace {

void change(ColumnSpan *c, const EditSession::SizeChange &by)
{
    if (by.setSize) {
        c->width = by.size;
        c->customWidth = by.custom;
    }
    if (by.setHidden) c->hidden = by.hidden;
    // (a column that was hidden by a width of nothing, shown, is a column of the sheet's usual width: not one of no width, which shows nothing)
    if (by.setHidden && !by.hidden && !by.setSize && c->width == 0) {
        c->width = -1;
        c->customWidth = false;
    }
}

void change(RowSpec *r, const EditSession::SizeChange &by)
{
    if (by.setSize) {
        r->height = by.size;
        r->customHeight = by.custom;
    }
    if (by.setHidden) {
        // (hidden by hand, or shown: a row the sheet's filter hides stays hidden)
        r->hand = by.hidden;
        r->hidden = r->hand || r->filtered;
    }
}

bool same(const ColumnSpan &a, const ColumnSpan &b) { return a.first == b.first && a.last == b.last && a.width == b.width && a.hidden == b.hidden && a.customWidth == b.customWidth && a.style == b.style && a.outline == b.outline && a.collapsed == b.collapsed; }
bool same(const RowSpec &a, const RowSpec &b) { return a.row == b.row && a.height == b.height && a.hidden == b.hidden && a.customHeight == b.customHeight && a.customFormat == b.customFormat && a.hand == b.hand && a.filtered == b.filtered && a.style == b.style && a.outline == b.outline && a.collapsed == b.collapsed; }

template <class T>
bool sameList(const std::vector<T> &a, const std::vector<T> &b)
{
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (!same(a[i], b[i])) return false;
    return true;
}

} // namespace

EditSession::Outcome EditSession::resize(const SizeEdit &edit, const QString &text)
{
    Outcome out;
    if (edit.columns ? sameList(edit.columnsBefore, edit.columnsAfter) : sameList(edit.rowsBefore, edit.rowsAfter)) return out;   // (nothing to change)
    Step step;
    step.sizes.push_back(edit);
    pushStep(std::move(step), text);
    return out;
}

EditSession::Outcome EditSession::resizeColumns(int sheet, quint32 first, quint32 last, const SizeChange &by, const QString &text)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why) || !allows(sheet, Allow::FormatColumns, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (first > last || last >= kMaxColumns) {
        out.ok = false;
        out.problem = tr("Those are not columns of the sheet.");
        return out;
    }
    SizeEdit e;
    e.sheet = sheet;
    e.columns = true;
    e.first = first;
    e.last = last;
    e.columnsBefore = m_book->calc->columnRuns(sheet, first, last);
    quint32 pos = first;
    auto emit = [&](quint32 a, quint32 b, const ColumnSpan *base) {
        ColumnSpan c;
        if (base) c = *base;
        c.first = a;
        c.last = b;
        change(&c, by);
        if (base || c.width >= 0 || c.hidden || c.style != 0 || c.outline != 0) e.columnsAfter.push_back(c);
    };
    for (const ColumnSpan &r : e.columnsBefore) {
        if (r.first > pos) emit(pos, r.first - 1, nullptr);
        emit(r.first, r.last, &r);
        pos = r.last + 1;
    }
    if (pos <= last) emit(pos, last, nullptr);
    return resize(e, text);
}

EditSession::Outcome EditSession::resizeRows(int sheet, quint32 first, quint32 last, const SizeChange &by, const QString &text)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why) || !allows(sheet, Allow::FormatRows, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (first > last || last >= kMaxRows) {
        out.ok = false;
        out.problem = tr("Those are not rows of the sheet.");
        return out;
    }
    if (qint64(last) - first + 1 > kMaxRowsPerResize) {
        out.ok = false;
        out.problem = tr("That is more rows than JeffCrunch changes at once (%1). Select fewer rows.").arg(QLocale().toString(kMaxRowsPerResize));
        return out;
    }
    SizeEdit e;
    e.sheet = sheet;
    e.columns = false;
    e.first = first;
    e.last = last;
    e.rowsBefore = m_book->calc->rowSpecsIn(sheet, first, last);
    size_t at = 0;
    for (quint32 row = first; row <= last; ++row) {
        const bool has = at < e.rowsBefore.size() && e.rowsBefore[at].row == row;
        RowSpec r;
        if (has) r = e.rowsBefore[at++];
        r.row = row;
        change(&r, by);
        if (has || r.height >= 0 || r.hidden || r.customFormat || r.outline != 0) e.rowsAfter.push_back(r);
        if (row == 0xFFFFFFFFu) break;
    }
    return resize(e, text);
}

EditSession::Outcome EditSession::changeOutline(int sheet, bool columns, const std::vector<OutlineChange> &changes, const QString &text)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why) || !allows(sheet, columns ? Allow::FormatColumns : Allow::FormatRows, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (changes.empty()) return out;
    quint32 first = changes.front().index, last = first;
    for (const OutlineChange &c : changes) {
        first = std::min(first, c.index);
        last = std::max(last, c.index);
    }
    if (last >= (columns ? kMaxColumns : kMaxRows)) {
        out.ok = false;
        out.problem = columns ? tr("Those are not columns of the sheet.") : tr("Those are not rows of the sheet.");
        return out;
    }
    if (!columns && qint64(last) - first + 1 > kMaxRowsPerResize) {
        out.ok = false;
        out.problem = tr("That is more rows than JeffCrunch changes at once (%1). Select fewer rows.").arg(QLocale().toString(kMaxRowsPerResize));
        return out;
    }
    std::map<quint32, OutlineChange> by;
    for (const OutlineChange &c : changes) by[c.index] = c;
    SizeEdit e;
    e.sheet = sheet;
    e.columns = columns;
    e.first = first;
    e.last = last;
    if (!columns) {
        e.rowsBefore = m_book->calc->rowSpecsIn(sheet, first, last);
        std::map<quint32, RowSpec> specs;
        for (const RowSpec &r : e.rowsBefore) specs[r.row] = r;
        for (const auto &kv : by) {
            RowSpec &r = specs[kv.first];
            r.row = kv.first;
            if (kv.second.level >= 0) r.outline = quint8(std::min(kv.second.level, kMaxOutlineLevel));
            if (kv.second.hidden >= 0) {
                r.hand = kv.second.hidden != 0;
                r.hidden = r.hand || r.filtered;
            }
            if (kv.second.collapsed >= 0) r.collapsed = kv.second.collapsed != 0;
        }
        for (const auto &kv : specs) {
            const RowSpec &r = kv.second;
            const bool had = std::any_of(e.rowsBefore.begin(), e.rowsBefore.end(), [&](const RowSpec &b) { return b.row == r.row; });
            if (had || r.height >= 0 || r.hidden || r.customFormat || r.outline != 0 || r.collapsed) e.rowsAfter.push_back(r);
        }
    } else {
        e.columnsBefore = m_book->calc->columnRuns(sheet, first, last);
        std::vector<ColumnSpan> each;   // one for each column of the range, then joined where they are alike
        for (quint32 c = first; c <= last; ++c) {
            ColumnSpan s;
            bool had = false;
            for (const ColumnSpan &b : e.columnsBefore)
                if (b.first <= c && c <= b.last) {
                    s = b;
                    had = true;
                }
            s.first = s.last = c;
            const auto it = by.find(c);
            if (it != by.end()) {
                if (it->second.level >= 0) s.outline = quint8(std::min(it->second.level, kMaxOutlineLevel));
                if (it->second.hidden >= 0) s.hidden = it->second.hidden != 0;
                if (it->second.hidden == 0 && s.width == 0) {
                    s.width = -1;
                    s.customWidth = false;
                }
                if (it->second.collapsed >= 0) s.collapsed = it->second.collapsed != 0;
            }
            if (had || s.width >= 0 || s.hidden || s.style != 0 || s.outline != 0 || s.collapsed) each.push_back(s);
        }
        for (const ColumnSpan &s : each) {
            if (!e.columnsAfter.empty() && e.columnsAfter.back().last + 1 == s.first) {
                ColumnSpan joined = e.columnsAfter.back();
                joined.last = s.first;
                if (joined.width == s.width && joined.hidden == s.hidden && joined.customWidth == s.customWidth && joined.style == s.style && joined.outline == s.outline && joined.collapsed == s.collapsed &&
                    joined.bestFit == s.bestFit && joined.phonetic == s.phonetic) {
                    e.columnsAfter.back().last = s.last;
                    continue;
                }
            }
            e.columnsAfter.push_back(s);
        }
    }
    return resize(e, text);
}

// Both outlines of a sheet changed as one step, the rows in pieces that each stay within what one change of rows takes (a sheet whose groups lie far apart has changes far apart): when a
// part is refused what was done is taken back and the history keeps nothing of either.
static EditSession::Outcome bothOutlines(EditSession *s, QUndoStack *undo, int sheet, const std::vector<OutlineChange> &rows, const std::vector<OutlineChange> &columns, const QString &text)
{
    EditSession::Outcome out;
    if (rows.empty() && columns.empty()) return out;
    const int start = undo->index();
    undo->beginMacro(text);
    EditSession::Outcome done;
    for (size_t at = 0; at < rows.size() && done.ok;) {
        size_t end = at + 1;
        while (end < rows.size() && qint64(rows[end].index) - rows[at].index + 1 <= EditSession::kMaxRowsPerResize) ++end;
        done = s->changeOutline(sheet, false, std::vector<OutlineChange>(rows.begin() + qint64(at), rows.begin() + qint64(end)), text);
        at = end;
    }
    if (done.ok) done = s->changeOutline(sheet, true, columns, text);
    undo->endMacro();
    if (!done.ok) {
        if (undo->index() > start) undo->undo();
        auto *nothing = new QUndoCommand;
        nothing->setObsolete(true);
        undo->push(nothing);
        out.ok = false;
        out.problem = done.problem;
    }
    return out;
}

static bool hasGroups(const Outline &o)
{
    return std::any_of(o.lines().begin(), o.lines().end(), [](const OutlineLine &l) { return l.level > 0 || l.collapsed; });
}

EditSession::Outcome EditSession::autoOutline(int sheet, AutoOutlineReport *report, bool replace, qint64 maxFormulas)
{
    Outcome out;
    AutoOutlineReport none;
    AutoOutlineReport &rep = report ? *report : none;
    rep = AutoOutlineReport();
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why) || !allows(sheet, Allow::FormatRows, &why) || !allows(sheet, Allow::FormatColumns, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    const QString name = m_book->book->info().sheets[sheet].name;
    std::vector<std::pair<quint32, quint32>> rowRuns, columnRuns;
    Outline rowsNow(std::vector<OutlineLine>(), 0, true), columnsNow(std::vector<OutlineLine>(), 0, true);
    qint64 seen = 0;
    bool tooMany = false;
    {
        // (the formulas are read in the calculation, which the worker is working in: it is held back while they are read)
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        for (int waited = 0; !hold && waited < 30; waited += 10) {
            QThread::msleep(10);
            hold = m_loader->hold();
        }
        if (!hold) {
            out.ok = false;
            out.problem = tr("The workbook is busy. Try again in a moment.");
            return out;
        }
        const Calculation *calc = hold->calculation();
        const jp::office::sheet::Sheet &sh = calc->sheet(sheet);
        const quint32 lastRow = std::min<quint32>(sh.extentRows, kMaxRows), lastCol = std::min<quint32>(sh.extentCols, kMaxColumns);
        if (lastRow > 0 && lastCol > 0)
            sh.cells.forEachInRange(0, 0, lastRow - 1, lastCol - 1, [&](quint32 row, quint32 col, const Cell &cell) {
                if (!cell.hasFormula()) return true;
                ++rep.cellsRead;
                const Calculation::CellContent content = calc->content(sheet, row, col);
                if (!content.isFormula) return true;
                if (++seen > maxFormulas) {
                    tooMany = true;
                    return false;
                }
                for (const FormulaRef &r : findReferences(content.formula)) {
                    if (r.kind != FormulaRef::Kind::Range || (!r.sheet.isEmpty() && r.sheet.compare(name, Qt::CaseInsensitive) != 0)) continue;
                    // (a block that lies wholly before or after the formula in both directions is one group, in the direction it is nearer in: a total below it makes rows, beside it columns)
                    const bool above = r.row2 < row, below = r.row1 > row, left = r.col2 < col, right = r.col1 > col;
                    const bool rowGroup = r.row2 > r.row1 && (above || below), colGroup = r.col2 > r.col1 && (left || right);
                    const qint64 rowGap = above ? qint64(row) - r.row2 : qint64(r.row1) - row, colGap = left ? qint64(col) - r.col2 : qint64(r.col1) - col;
                    if (rowGroup && (!colGroup || rowGap <= colGap)) {
                        if (qint64(r.row2) - r.row1 + 1 > kMaxAutoOutlineLines) ++rep.tooLong;
                        else rowRuns.emplace_back(r.row1, r.row2);
                    } else if (colGroup) {
                        columnRuns.emplace_back(r.col1, r.col2);
                    }
                }
                return true;
            });
        if (!tooMany) {
            rowsNow = outlineOf(sheet, false);
            columnsNow = outlineOf(sheet, true);
        }
    }
    if (tooMany) {
        out.ok = false;
        out.problem = tr("This sheet has more formulas than Auto Outline reads (%1).").arg(QLocale().toString(maxFormulas));
        return out;
    }
    rep.groupsFound = !rowRuns.empty() || !columnRuns.empty();
    // (no formula calls for a group: the outline the sheet has is left as it is)
    if (!rep.groupsFound) return out;
    std::sort(rowRuns.begin(), rowRuns.end());
    rowRuns.erase(std::unique(rowRuns.begin(), rowRuns.end()), rowRuns.end());
    std::sort(columnRuns.begin(), columnRuns.end());
    columnRuns.erase(std::unique(columnRuns.begin(), columnRuns.end()), columnRuns.end());
    bool deeperRows = false, deeperColumns = false;
    const auto rows = rowsNow.replaceWith(rowRuns, &deeperRows), columns = columnsNow.replaceWith(columnRuns, &deeperColumns);
    rep.tooDeep = deeperRows || deeperColumns;
    if (rows.empty() && columns.empty()) return out;
    if (!replace && (hasGroups(rowsNow) || hasGroups(columnsNow))) {
        out.ok = false;
        out.confirm = true;
        out.problem = tr("This sheet has an outline. Replace it with one made from the formulas?");
        return out;
    }
    out = bothOutlines(this, &m_undo, sheet, rows, columns, tr("Auto Outline"));
    rep.changed = out.ok;
    return out;
}

EditSession::Outcome EditSession::clearOutline(int sheet)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why) || !allows(sheet, Allow::FormatRows, &why) || !allows(sheet, Allow::FormatColumns, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    return bothOutlines(this, &m_undo, sheet, outlineOf(sheet, false).clearAll(), outlineOf(sheet, true).clearAll(), tr("Clear Outline"));
}

Outline EditSession::outlineOf(int sheet, bool columns) const
{
    ++m_outlinesBuilt;
    std::vector<OutlineLine> lines;
    const Calculation *calc = m_book->calc.get();
    if (columns) {
        for (const ColumnSpan &c : calc->columnRuns(sheet, 0, kMaxColumns - 1))
            if (c.outline != 0 || c.hidden || c.collapsed)
                for (quint32 i = c.first; i <= c.last; ++i) lines.push_back({i, c.outline, c.hidden, c.collapsed});
    } else {
        for (const RowSpec &r : calc->rowSpecsIn(sheet, 0, kMaxRows - 1))
            if (r.outline != 0 || r.hidden || r.collapsed) lines.push_back({r.row, r.outline, r.hidden, r.collapsed});
    }
    std::sort(lines.begin(), lines.end(), [](const OutlineLine &a, const OutlineLine &b) { return a.index < b.index; });
    const SheetFormat f = calc->sheetFormatOf(sheet);
    return Outline(std::move(lines), columns ? quint32(kMaxColumns) : quint32(kMaxRows), columns ? f.summaryRight : f.summaryBelow);
}

EditSession::Outcome EditSession::setDefaultSizes(int sheet, std::optional<double> columnWidth, std::optional<double> rowHeight, const QString &text)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if ((columnWidth && (*columnWidth <= 0 || *columnWidth > 255)) || (rowHeight && (*rowHeight <= 0 || *rowHeight > 409.5))) {
        out.ok = false;
        out.problem = tr("A default width can be from 0 to 255 characters, and a default height from 0 to 409.5 points.");
        return out;
    }
    FormatEdit e;
    e.sheet = sheet;
    e.before = m_book->calc->sheetFormatOf(sheet);
    e.after = e.before;
    if (columnWidth) e.after.defaultColWidth = *columnWidth;
    if (rowHeight) {
        e.after.defaultRowHeight = *rowHeight;
        e.after.customHeight = true;
    }
    if (e.after.defaultColWidth == e.before.defaultColWidth && e.after.defaultRowHeight == e.before.defaultRowHeight && e.after.customHeight == e.before.customHeight) return out;   // (nothing to change)
    Step step;
    step.formats.push_back(e);
    pushStep(std::move(step), text);
    return out;
}

EditSession::Outcome EditSession::fitColumns(int sheet, const std::vector<std::pair<quint32, double>> &widths)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why) || !allows(sheet, Allow::FormatColumns, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    Step step;
    for (const auto &[col, width] : widths) {
        if (col >= kMaxColumns) continue;
        SizeEdit e;
        e.sheet = sheet;
        e.columns = true;
        e.first = e.last = col;
        e.columnsBefore = m_book->calc->columnRuns(sheet, col, col);
        ColumnSpan c = e.columnsBefore.empty() ? ColumnSpan() : e.columnsBefore.front();
        const bool had = !e.columnsBefore.empty();
        c.first = c.last = col;
        c.width = width;
        c.customWidth = false;
        if (had || width >= 0 || c.hidden || c.style != 0) e.columnsAfter.push_back(c);
        if (!sameList(e.columnsBefore, e.columnsAfter)) step.sizes.push_back(e);
    }
    if (step.sizes.empty()) return out;
    pushStep(std::move(step), widths.size() == 1 ? tr("Fitting a column to its contents") : tr("Fitting columns to their contents"));
    return out;
}

EditSession::Outcome EditSession::fitRows(int sheet, const std::vector<std::pair<quint32, double>> &heights)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why) || !allows(sheet, Allow::FormatRows, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (qint64(heights.size()) > kMaxRowsPerResize) {
        out.ok = false;
        out.problem = tr("That is more rows than JeffCrunch changes at once (%1). Select fewer rows.").arg(QLocale().toString(kMaxRowsPerResize));
        return out;
    }
    Step step;
    for (const auto &[row, height] : heights) {
        if (row >= kMaxRows) continue;
        SizeEdit e;
        e.sheet = sheet;
        e.columns = false;
        e.first = e.last = row;
        e.rowsBefore = m_book->calc->rowSpecsIn(sheet, row, row);
        RowSpec r = e.rowsBefore.empty() ? RowSpec() : e.rowsBefore.front();
        const bool had = !e.rowsBefore.empty();
        r.row = row;
        r.height = height;
        r.customHeight = false;
        if (had || height >= 0 || r.hidden || r.customFormat) e.rowsAfter.push_back(r);
        if (!sameList(e.rowsBefore, e.rowsAfter)) step.sizes.push_back(e);
    }
    if (step.sizes.empty()) return out;
    pushStep(std::move(step), heights.size() == 1 ? tr("Fitting a row to its contents") : tr("Fitting rows to their contents"));
    return out;
}

// The heights the text of rows needs, for rows whose height is the program's to choose (a row a person gave a height, or hid, keeps it): put in the sheet with no step of Undo. What a row's height
// should be follows from what is in it and the widths of its columns, so Undo and Redo, which change those, make it again; it is saved with the sheet.
int EditSession::setAutoRowHeights(int sheet, const std::vector<std::pair<quint32, double>> &heights)
{
    if (!canEdit() || !canEditSheet(sheet) || heights.empty()) return 0;
    std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
    if (!hold) return 0;
    Calculation *calc = hold->calculation();
    int changed = 0;
    for (const auto &[row, height] : heights) {
        if (row >= kMaxRows || height <= 0) continue;
        std::vector<RowSpec> was = calc->rowSpecsIn(sheet, row, row);
        RowSpec spec = was.empty() ? RowSpec() : was.front();
        if (!was.empty() && (spec.customHeight || spec.hidden)) continue;
        if (!was.empty() && std::abs(spec.height - height) < 0.01) continue;
        spec.row = row;
        spec.height = height;
        spec.customHeight = false;
        calc->setRowSpecs(sheet, row, row, {spec});
        markRowResized(sheet, row);
        ++changed;
    }
    if (changed > 0) {
        hold.reset();
        Q_EMIT sizesChanged(sheet);
    }
    return changed;
}

bool EditSession::widenColumnWithEntry(int sheet, quint32 col, double width)
{
    // the last step on the stack is the entry (it is done, and nothing after it)
    const int index = m_undo.index() - 1;
    if (index < 0 || index != m_undo.count() - 1 || !canEdit()) return false;
    return widenLastStep(index, sheet, col, width);
}

} // namespace jp::crunch
