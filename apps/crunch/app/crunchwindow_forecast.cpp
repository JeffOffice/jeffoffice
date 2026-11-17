// Forecast Sheet: Data > Forecast Sheet asks for the timeline and the values (with a picture of the forecast), and puts a sheet with the table, the chart, and the statistics before the
// sheet in view, as one step of Undo.
#include "app/crunchwindow.h"

#include "app/forecastdialog.h"
#include "core/editsession.h"
#include "core/forecastsheet.h"
#include "core/gotospecial.h"
#include "core/sheetobjects.h"
#include "sheet/cellref.h"
#include "sheet/chartmake.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QUndoStack>

#include <cmath>

namespace jp {

namespace {

// Whether a number format code shows dates.
bool showsDates(const QString &code)
{
    QString c = code;
    c.remove(QRegularExpression(QStringLiteral("\"[^\"]*\"|\\[[^\\]]*\\]|\\\\.")));
    return c.contains(QRegularExpression(QStringLiteral("[dDyY]")));
}

QString rangeText(const jp::office::sheet::CellRange &r)
{
    using jp::office::sheet::cellName;
    return r.row1 == r.row2 && r.col1 == r.col2 ? cellName(r.row1, r.col1) : cellName(r.row1, r.col1) + QLatin1Char(':') + cellName(r.row2, r.col2);
}

} // namespace

void CrunchWindow::forecastSheetDialog()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed() || !m_session || !m_loaded || !m_loaded->book) return;
    // the cells selected: the timeline, then the values (a first row of names is left out)
    QString timeline, values;
    jp::office::sheet::CellRange sel = selectedRange();
    if (sel.valid && (sel.col2 > sel.col1 || sel.row2 > sel.row1)) {
        const bool inRows = sel.col2 - sel.col1 > sel.row2 - sel.row1 && sel.row2 > sel.row1 && sel.row2 - sel.row1 < 2 ? true : false;
        jp::office::sheet::CellRange t = sel, v = sel;
        if (inRows) {
            t.row2 = sel.row1;
            v.row1 = v.row2 = sel.row1 + 1;
            if (m_session->valueAt(m_sheet, t.row1, t.col1).isText()) t.col1 = v.col1 = sel.col1 + 1;
        } else {
            t.col2 = sel.col1;
            v.col1 = v.col2 = sel.col1 + 1;
            if (!m_session->valueAt(m_sheet, t.row1, t.col1).isNumber() && v.row2 > v.row1 && m_session->valueAt(m_sheet, v.row1, v.col1).isText()) t.row1 = v.row1 = sel.row1 + 1;
        }
        if (t.col1 <= t.col2 && t.row1 <= t.row2) {
            timeline = rangeText(t);
            values = rangeText(v);
        }
    }
    const jp::calc::DateSystem system = m_loaded->book->info().dateSystem;
    const ForecastSourceReader reader = [this](const QString &timelineText, const QString &valuesText) {
        ForecastSourceInfo info;
        if (!forecastSource(timelineText, valuesText, &info.input.pairs, &info.dates, &info.problem)) info.input.pairs.clear();
        return info;
    };
    ForecastDialog dialog(reader, system, timeline, values, this);
    if (dialog.exec() != QDialog::Accepted) return;
    const crunch::ForecastSettings s = dialog.settings();
    forecastSheetIn(dialog.timelineText(), dialog.valuesText(), s.end, s.start, s.confidence, s.seasonality, s.statistics, s.interpolate, int(s.aggregation), s.columns);
}

bool CrunchWindow::forecastSource(const QString &timelineText, const QString &valuesText, std::vector<std::pair<double, double>> *pairs, bool *dates, QString *problem)
{
    if (!m_session) {
        *problem = tr("The workbook is not open.");
        return false;
    }
    if (timelineText.isEmpty() || valuesText.isEmpty()) {
        *problem = tr("Choose the cells that hold the timeline and the cells that hold the values. Select two columns first (the timeline, then the values), or write the ranges here.");
        return false;
    }
    const crunch::GoToTarget t = crunch::resolveGoTo(*m_session, timelineText, m_sheet), v = crunch::resolveGoTo(*m_session, valuesText, m_sheet);
    if (!t.ok || t.wholeRows || t.wholeColumns) {
        *problem = tr("%1 is not a range of cells. Write it as A2:A40, or Sheet2!A2:A40 for another sheet.").arg(timelineText);
        return false;
    }
    if (!v.ok || v.wholeRows || v.wholeColumns) {
        *problem = tr("%1 is not a range of cells. Write it as B2:B40, or Sheet2!B2:B40 for another sheet.").arg(valuesText);
        return false;
    }
    crunch::ForecastInput input;
    const crunch::ForecastCellReader readTimeline = [&](quint32 row, quint32 col) { return m_session->valueAt(t.sheet, row, col); };
    const crunch::ForecastCellReader readValues = [&](quint32 row, quint32 col) { return m_session->valueAt(v.sheet, row, col); };
    if (!crunch::readForecastInput(t.range, readTimeline, v.range, readValues, &input, problem)) return false;
    *pairs = std::move(input.pairs);
    *dates = showsDates(m_session->formatCodeOf(t.sheet, t.range.row1, t.range.col1));
    return true;
}

bool CrunchWindow::forecastSheetIn(const QString &timelineText, const QString &valuesText, double end, double start, double confidence, int seasonality, bool statistics, bool interpolate, int aggregation, bool columns)
{
    const QString title = tr("Forecast Sheet");
    if (!m_session || !m_loaded || !m_loaded->book || !m_objects || !editingAllowed()) return false;
    crunch::ForecastInput input;
    bool dates = false;
    QString problem;
    if (!forecastSource(timelineText, valuesText, &input.pairs, &dates, &problem)) {
        notice(title, problem);
        return false;
    }
    crunch::ForecastSettings settings;
    settings.end = end;
    settings.start = start;
    settings.confidence = confidence;
    settings.seasonality = seasonality;
    settings.statistics = statistics;
    settings.interpolate = interpolate;
    settings.aggregation = jp::calc::ets::Aggregation(std::min(7, std::max(1, aggregation)));
    settings.columns = columns;
    const crunch::ForecastResult result = crunch::computeForecast(input, settings, m_loaded->book->info().dateSystem);
    if (!result.problem.isEmpty()) {
        notice(title, result.problem);
        return false;
    }
    const crunch::ForecastSheetPlan plan = crunch::planForecastSheet(result, settings);
    const crunch::GoToTarget timeline = crunch::resolveGoTo(*m_session, timelineText, m_sheet);
    const QString timeFormat = m_session->formatCodeOf(timeline.sheet, timeline.range.row1, timeline.range.col1);
    QString name = tr("Forecast");
    const auto &sheets = m_loaded->book->info().sheets;
    auto taken = [&](const QString &n) {
        for (const auto &s : sheets)
            if (!s.deleted && s.name.compare(n, Qt::CaseInsensitive) == 0) return true;
        return false;
    };
    for (int n = 2; taken(name); ++n) name = tr("Forecast %1").arg(n);
    const qint64 before = m_loader.changesMade();
    QUndoStack *stack = m_session->undoStack();
    stack->beginMacro(title);
    int made = -1;
    crunch::EditSession::Outcome o = m_session->insertSheet(m_sheet, name, &made);
    auto cells = [](quint32 row1, quint32 col1, quint32 row2, quint32 col2) {
        jp::office::sheet::CellRange r;
        r.valid = true;
        r.row1 = row1;
        r.col1 = col1;
        r.row2 = row2;
        r.col2 = col2;
        return r;
    };
    if (o.ok) o = m_session->pasteText(made, cells(0, 0, 0, 0), plan.table, crunch::EditSession::PasteOptions());
    if (o.ok && !plan.statistics.isEmpty()) o = m_session->pasteText(made, cells(0, 6, 0, 6), plan.statistics, crunch::EditSession::PasteOptions());
    if (o.ok && !timeFormat.isEmpty() && timeFormat != QLatin1String("General") && plan.dataRows > 0) {
        jp::office::sheet::FormatChange change;
        change.numberFormat = timeFormat;
        o = m_session->formatCells(made, cells(1, 0, quint32(plan.dataRows), 0), change, QString());
    }
    if (o.ok) {
        // widths that show the headers, the times, and the statistics
        crunch::EditSession::SizeChange wide;
        wide.setSize = true;
        wide.custom = true;
        wide.size = 12;
        o = m_session->resizeColumns(made, 0, 1, wide, QString());
        wide.size = 24;
        if (o.ok) o = m_session->resizeColumns(made, 2, 4, wide, QString());
        wide.size = 18;
        if (o.ok && !plan.statistics.isEmpty()) o = m_session->resizeColumns(made, 6, 7, wide, QString());
    }
    int chartId = -1;
    if (o.ok) {
        using namespace jp::office::sheet;
        const CellRange block = cells(0, 0, quint32(plan.dataRows), 4);
        const ChartPlan chartPlan = planChartFrom(name, block, true, true, false, ChartKind::Line);
        jp::office::Chart chart = makeChart(ChartKind::Line, chartPlan, m_objects->theme(), chartPlan.points);
        QVector<jp::office::ChartSeries> all;
        for (const jp::office::ChartPlot &p : chart.plots) all += p.series;
        if (columns && all.size() == 4) {
            const QVector<SeriesKind> kinds = {{ChartKind::Column, false}, {ChartKind::Column, false}, {ChartKind::Line, false}, {ChartKind::Line, false}};
            chart = chartFromSeries(all, kinds, m_objects->theme(), &chart);
        }
        // the forecast is drawn dashed, and its bounds as thin gray lines
        const QColor forecastColor(237, 125, 49), boundColor(140, 140, 140);
        for (jp::office::ChartPlot &p : chart.plots)
            for (jp::office::ChartSeries &s : p.series) {
                const bool forecast = s.nameRef.endsWith(QLatin1String("$C$1")), bound = s.nameRef.endsWith(QLatin1String("$D$1")) || s.nameRef.endsWith(QLatin1String("$E$1"));
                if (!forecast && !bound) continue;
                if (p.type != jp::office::ChartType::Line) {
                    s.color = forecastColor;
                    continue;
                }
                s.outline.set = true;
                s.outline.color = forecast ? forecastColor : boundColor;
                s.outline.width = forecast ? 2.25 : 1.25;
                s.outline.dash = QStringLiteral("dash");
                s.color = s.outline.color;
            }
        if (m_objects->cells()) applyChartCells(&chart, made, *m_objects->cells());
        // beside the table: to the right of the statistics, or of the columns
        jp::office::sheet::DrawingAnchor anchor;
        anchor.kind = jp::office::sheet::AnchorKind::TwoCell;
        anchor.from.col = 6;
        anchor.from.row = plan.statistics.isEmpty() ? 1 : quint32(plan.statistics.size() + 2);
        anchor.to.col = 15;
        anchor.to.row = anchor.from.row + 18;
        QString why;
        chartId = m_objects->makeChart(made, chart, anchor, &why);
        if (chartId < 0) o = crunch::EditSession::Outcome{false, why};
    }
    if (o.ok) {
        crunch::ObjectEdit e;
        e.sheet = made;
        e.id = chartId;
        e.before = m_objects->stateOf(made, chartId);
        e.after = e.before;
        e.after.deleted = false;
        o = m_session->changeObjects({e}, title);
        if (!o.ok) m_objects->discardMade(made, chartId);
    }
    endEditStep(stack);
    if (m_loader.changesMade() != before) m_calculating = true;
    if (!o.ok) {
        stack->undo();
        notice(title, o.problem);
        return false;
    }
    setActiveSheet(made);
    return true;
}

} // namespace jp
