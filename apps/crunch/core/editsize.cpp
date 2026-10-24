// The width of columns and the height of rows: one step each, with the runs of columns (the specs of rows) as they were and as they are. See EditSession::resizeColumns.
#include "core/editsession.h"

#include <algorithm>

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
}

void change(RowSpec *r, const EditSession::SizeChange &by)
{
    if (by.setSize) {
        r->height = by.size;
        r->customHeight = by.custom;
    }
    if (by.setHidden) r->hidden = by.hidden;
}

bool same(const ColumnSpan &a, const ColumnSpan &b) { return a.first == b.first && a.last == b.last && a.width == b.width && a.hidden == b.hidden && a.customWidth == b.customWidth && a.style == b.style; }
bool same(const RowSpec &a, const RowSpec &b) { return a.row == b.row && a.height == b.height && a.hidden == b.hidden && a.customHeight == b.customHeight && a.customFormat == b.customFormat && a.style == b.style; }

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
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
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
        if (base || c.width >= 0 || c.hidden || c.style != 0) e.columnsAfter.push_back(c);
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
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
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
        if (has || r.height >= 0 || r.hidden || r.customFormat) e.rowsAfter.push_back(r);
        if (row == 0xFFFFFFFFu) break;
    }
    return resize(e, text);
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
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
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
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
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

bool EditSession::widenColumnWithEntry(int sheet, quint32 col, double width)
{
    // the last step on the stack is the entry (it is done, and nothing after it)
    const int index = m_undo.index() - 1;
    if (index < 0 || index != m_undo.count() - 1 || !canEdit()) return false;
    return widenLastStep(index, sheet, col, width);
}

} // namespace jp::crunch
