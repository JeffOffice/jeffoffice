// Recommended pivot tables: the layouts the data suggests, and making one.
#include "core/editsession.h"

#include "core/pivotadvice.h"

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;

QVector<PivotAdvice> EditSession::recommendPivots(const QString &source, QString *why)
{
    QString problem;
    int srcSheet = -1;
    CellRange range;
    QString name;
    if (!m_book || !m_book->book || !m_book->calc) {
        if (why) *why = tr("The workbook is still being opened. It can be changed when it is open.");
        return {};
    }
    if (!parsePivotSource(source, 0, &srcSheet, &range, &name, &problem)) {
        if (why) *why = problem;
        return {};
    }
    if (!name.isEmpty() && !pivotNameRange(name, &srcSheet, &range)) {
        if (why) *why = tr("JeffCrunch can't find the cells \"%1\" stands for.").arg(name);
        return {};
    }
    const auto &sheets = m_book->book->info().sheets;
    if (srcSheet < 0 || srcSheet >= sheets.size() || sheets[srcSheet].deleted || sheets[srcSheet].kind != SheetKind::Worksheet) {
        if (why) *why = tr("The data must be on a sheet of cells.");
        return {};
    }
    if (range.row2 <= range.row1) {
        if (why) *why = tr("The data needs a row of names and at least one row of data under it.");
        return {};
    }
    if (range.col2 - range.col1 + 1 > quint32(kPivotMaxFields) || range.row2 - range.row1 > quint32(kPivotMaxRecords)
        || qint64(range.row2 - range.row1) * qint64(range.col2 - range.col1 + 1) > kPivotMaxCreateCells) {
        if (why) *why = tr("There is too much data to look through.");
        return {};
    }
    PivotSource ps;
    fillPivotSource(&ps, srcSheet, range);
    PivotCache cache;
    if (!buildPivotCache(ps, PivotCache(), &cache, &problem)) {
        if (why) *why = tr("The data can't be read: %1.").arg(problem);
        return {};
    }
    const QVector<PivotAdvice> out = adviseFromCache(cache);
    if (out.isEmpty() && why) *why = tr("The data has no column that repeats its values (such as a region or a month), so JeffCrunch has no layout to suggest. Make a pivot table yourself instead.");
    return out;
}

EditSession::Outcome EditSession::createRecommendedPivot(const QString &source, const PivotAdvice &advice, int *made)
{
    const int startIndex = m_undo.index();
    m_undo.beginMacro(tr("Create Pivot Table"));
    int table = -1;
    Outcome out = createPivot(source, -1, 0, 0, &table);
    if (out.ok) {
        PivotBook &pb = m_book->book->pivots();
        PivotTable layout = *pb.table(table).table;
        const PivotCache &cache = *pb.cache(pb.table(table).cache).cache;
        if (advice.rowField < 0 || advice.rowField >= layout.fields.size() || advice.valueField < 0 || advice.valueField >= layout.fields.size()) {
            out.ok = false;
            out.problem = tr("That layout does not fit the data any more.");
        } else {
            layout.rowFields.append(advice.rowField);
            PivotDataField d;
            d.fld = advice.valueField;
            d.subtotal = advice.count ? QStringLiteral("count") : QStringLiteral("sum");
            d.name = pivotDataFieldName(layout, cache, advice.valueField, d.subtotal);
            layout.dataFields.append(d);
            out = setPivot(table, layout, true, tr("Create Pivot Table"));
            if (out.ok && advice.byDate) out = groupPivotField(table, advice.rowField, {QStringLiteral("years"), QStringLiteral("months")}, 0, 0, 1, true, true);
        }
    }
    m_undo.endMacro();
    if (!out.ok) {
        while (m_undo.index() > startIndex) m_undo.undo();   // (what was done is taken back, the sheet too)
        return out;
    }
    if (made) *made = table;
    return out;
}

} // namespace jp::crunch
