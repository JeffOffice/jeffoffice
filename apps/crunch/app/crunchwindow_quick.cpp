// Quick Analysis: a button by the last corner of two or more selected cells that hold data (and Ctrl+Q) opens a box of tabs that format the cells, chart them, total them, make them a table or a
// pivot table, or give each row a sparkline. A choice looked at is shown over the sheet without changing anything (the rules shown are in the sheet's rules for the look only: core/editquick.cpp);
// a choice made is one step of Undo. The button and the box are the shared QuickAnalysis; what the choices are and do is here.

#include "app/crunchwindow.h"

#include "app/chartdialogs.h"
#include "app/condpresets.h"
#include "app/icons.h"
#include "app/quickanalysis.h"
#include "core/editsession.h"
#include "core/gotospecial.h"
#include "core/syslocale.h"
#include "core/sheetobjects.h"
#include "sheet/chartmake.h"
#include "grid/gridview.h"
#include "sheet/cellref.h"

#include <QAction>
#include <QPointer>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QLocale>
#include <QPainterPath>
#include <QPixmap>

#include <algorithm>
#include <cmath>

namespace jp {

using jp::office::sheet::CellRange;
using jp::office::sheet::ChartKind;
using jp::office::sheet::ChartPlan;
using jp::office::sheet::recommendChartKinds;
using jp::office::sheet::CondRule;
using jp::office::sheet::DifferentialFormat;
using jp::office::sheet::SparklineGroup;

namespace {

using Total = crunch::EditSession::QuickTotal;

constexpr qint64 kAverageCells = 100000;   // (the most cells looked at for the average Greater Than compares with)
constexpr int kLookCells = 200;            // (the most cells a look at totals or sparklines draws)
constexpr int kMostCharts = 4;

struct TotalChoice {
    Total kind;
    const char *name;
};

QString totalName(Total kind, bool down)
{
    switch (kind) {
    case Total::Sum: return down ? CrunchWindow::tr("Sum") : CrunchWindow::tr("Row Sum");
    case Total::Average: return down ? CrunchWindow::tr("Average") : CrunchWindow::tr("Row Average");
    case Total::Count: return down ? CrunchWindow::tr("Count") : CrunchWindow::tr("Row Count");
    case Total::PercentOfTotal: return down ? CrunchWindow::tr("% of Total") : CrunchWindow::tr("Row % of Total");
    case Total::RunningTotal: return down ? CrunchWindow::tr("Running Total") : CrunchWindow::tr("Row Running");
    }
    return QString();
}

QString totalTip(Total kind, bool down)
{
    switch (kind) {
    case Total::Sum: return down ? CrunchWindow::tr("Adds up each column of numbers, in the row under the cells.") : CrunchWindow::tr("Adds up each row of numbers, in the column to the right of the cells.");
    case Total::Average: return down ? CrunchWindow::tr("Finds the average of each column, in the row under the cells.") : CrunchWindow::tr("Finds the average of each row, in the column to the right of the cells.");
    case Total::Count: return down ? CrunchWindow::tr("Counts the numbers in each column, in the row under the cells.") : CrunchWindow::tr("Counts the numbers in each row, in the column to the right of the cells.");
    case Total::PercentOfTotal: return down ? CrunchWindow::tr("Shows each column's share of all the numbers, as a percent, in the row under the cells.") : CrunchWindow::tr("Shows each row's share of all the numbers, as a percent, in the column to the right of the cells.");
    case Total::RunningTotal: return down ? CrunchWindow::tr("Adds up each column as it goes down, in new cells to the right of the cells.") : CrunchWindow::tr("Adds up each row as it goes across, in new cells under the cells.");
    }
    return QString();
}

const char *iconOfTotal(Total kind)
{
    switch (kind) {
    case Total::Sum: return "sigma";
    case Total::Average: return "equal-approximately";
    case Total::Count: return "hash";
    case Total::PercentOfTotal: return "percent";
    case Total::RunningTotal: return "trending-up";
    }
    return "sigma";
}

const Total kTotals[] = {Total::Sum, Total::Average, Total::Count, Total::PercentOfTotal, Total::RunningTotal};

// The rectangle of a block of cells in the viewport's pixels (empty when its corners are not in view).
QRect rectOf(const grid::GridView *view, const CellRange &r)
{
    const QRect a = view->cellRect(r.row1, r.col1), b = view->cellRect(r.row2, r.col2);
    if (a.isNull() || b.isNull()) return QRect();
    return a.united(b);
}

} // namespace

void CrunchWindow::createQuickActions()
{
#ifdef Q_OS_MACOS
    const QKeySequence key(Qt::META | Qt::Key_Q);   // (Command and Q is the system's own: Quit)
#else
    const QKeySequence key(Qt::CTRL | Qt::Key_Q);
#endif
    mk("edit.quickAnalysis", tr("Quick Analysis"), "wand-sparkles", key, [this] { quickAnalysisCommand(); });
    m_opsActions << QStringLiteral("edit.quickAnalysis");
    m_quick = new QuickAnalysis(m_view->viewport(), m_view, [this] { return quickTabs(); });
    m_quick->setButtonIcon(icon(QStringLiteral("wand-sparkles")));
    m_quick->setButtonToolTip(tr("Quick Analysis (%1)").arg(key.toString(QKeySequence::NativeText)));
    m_quickLayer = new QuickPreviewLayer(m_view->viewport());
    connect(m_quick, &QuickAnalysis::previewed, this, [this](const QString &id) { quickPreview(id); });
    connect(m_quick, &QuickAnalysis::previewEnded, this, [this] { quickPreviewEnd(); });
    connect(m_quick, &QuickAnalysis::chosen, this, [this](const QString &id) { quickApply(id); });
    connect(m_view, &grid::GridView::selectionChanged, this, [this] { updateQuickAnalysis(); });
    connect(m_view, &grid::GridView::viewChanged, this, [this] {
        updateQuickAnalysis();
        if (m_quickLayer && m_quickLayer->isShowing()) {
            m_quickLayer->setGeometry(m_view->viewport()->rect());
            m_quickLayer->update();
        }
    });
}

// Whether the cells selected are two or more and hold something: what the button is shown for. (Looking costs the work of a few thousand cells at most.)
bool CrunchWindow::quickEligible(CellRange *out) const
{
    if (!m_session || !m_loaded || m_sheet < 0 || m_chartSheetShown || m_edit.active || isReadOnly()) return false;
    const grid::Selection &sel = m_view->selection();
    if (sel.whole != grid::Selection::Whole::None || sel.isOneCell()) return false;
    const CellRange r = selectedRange();
    if (!m_session->quickHasData(m_sheet, r)) return false;
    if (out) *out = r;
    return true;
}

void CrunchWindow::updateQuickAnalysis()
{
    if (!m_quick) return;
    CellRange r;
    // (a protected sheet that lets the person do none of what the box offers has no button)
    using Allow = crunch::EditSession::Allow;
    const bool any = m_session && m_sheet >= 0 && ((m_session->canEditSheet(m_sheet) && !m_session->protectionOf(m_sheet).enabled) || m_session->allows(m_sheet, Allow::FormatCells) || m_session->allows(m_sheet, Allow::Objects));
    if (!any || !quickEligible(&r)) {
        m_quick->hide();
        return;
    }
    const QRect corner = m_view->cellRect(r.row2, r.col2);
    if (corner.isNull() || !m_view->viewport()->rect().intersects(corner)) {
        m_quick->hide();
        return;
    }
    m_quick->showAt(corner, m_view->fillHandleRect());
}

void CrunchWindow::quickAnalysisCommand()
{
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (!m_quick) return;
    if (!quickEligible()) {
        setStatus(tr("Select two or more cells that have data to use Quick Analysis."));
        return;
    }
    updateQuickAnalysis();
    if (!m_quick->open()) setStatus(tr("There is nothing Quick Analysis can do with these cells."));
}

// The rule a choice of the Formatting tab makes, or none (a choice that is not one of them).
bool CrunchWindow::quickCondSpec(const QString &id, CondSpec *spec, QString *why)
{
    if (id == QLatin1String("fmt.bars")) {
        spec->kind = CondSpec::Kind::DataBar;
        spec->visual = condpresets::dataBarRule(CellRange(), 0, true);
    } else if (id == QLatin1String("fmt.scale")) {
        spec->kind = CondSpec::Kind::ColorScale;
        spec->visual = condpresets::colorScaleRule(CellRange(), 0);
    } else if (id == QLatin1String("fmt.icons")) {
        spec->kind = CondSpec::Kind::IconSet;
        spec->visual = condpresets::iconSetRule(CellRange(), condpresets::iconGroups().first().sets.first());
    } else if (id == QLatin1String("fmt.top")) {
        spec->kind = CondSpec::Kind::TopPercent;
        spec->count = 10;
        spec->look = 0;
    } else if (id == QLatin1String("fmt.greater")) {
        // (greater than the average of the numbers selected)
        const CellRange r = m_quickRange.valid ? m_quickRange : selectedRange();
        double sum = 0;
        qint64 n = 0;
        if (r.cellCount() > kAverageCells) {
            *why = tr("There are too many cells to find their average.");
            return false;
        }
        for (quint32 row = r.row1; row <= r.row2; ++row)
            for (quint32 col = r.col1; col <= r.col2; ++col) {
                const jp::calc::Value v = m_session->valueAt(m_sheet, row, col);
                if (v.isNumber()) {
                    sum += v.number();
                    ++n;
                }
            }
        if (!n) {
            *why = tr("There are no numbers here to compare.");
            return false;
        }
        spec->kind = CondSpec::Kind::CellValue;
        spec->op = CondRule::Operator::GreaterThan;
        spec->v1 = QString::number(sum / double(n), 'g', 15);
        spec->look = 0;
    } else {
        return false;
    }
    return true;
}

QVector<QuickTab> CrunchWindow::quickTabs()
{
    QVector<QuickTab> tabs;
    CellRange r;
    if (!quickEligible(&r)) return tabs;
    m_quickRange = r;
    using Allow = crunch::EditSession::Allow;
    const bool canWrite = m_session->canEditSheet(m_sheet) && !m_session->protectionOf(m_sheet).enabled;   // (cells beyond the data are locked on a protected sheet)
    auto choice = [](const QString &id, const QString &text, const QString &tip, const QIcon &ic, bool enabled) {
        QuickChoice c;
        c.id = id;
        c.text = text;
        c.tip = tip;
        c.icon = ic;
        c.enabled = enabled;
        return c;
    };

    // ---- Formatting
    {
        QuickTab tab;
        tab.id = QStringLiteral("formatting");
        tab.title = tr("Formatting");
        const bool can = m_session->allows(m_sheet, Allow::FormatCells) && m_session->canEdit() && m_session->condRulesRefusal(m_sheet).isEmpty();
        CondSpec probe;
        QString why;
        const bool average = r.cellCount() <= kAverageCells;
        tab.choices << choice(QStringLiteral("fmt.bars"), tr("Data Bars"), tr("Draws a bar in each cell that is as long as its number."), QIcon(condpresets::barPixmap(condpresets::barColors().first().rgb, true, 40)), can);
        tab.choices << choice(QStringLiteral("fmt.scale"), tr("Color Scale"), tr("Colors the cells from the lowest number to the highest."), QIcon(condpresets::scalePixmap(condpresets::scalePresets().first().colors, 40)), can);
        tab.choices << choice(QStringLiteral("fmt.icons"), tr("Icon Set"), tr("Puts a small picture in each cell that says whether its number is low, middle, or high."), QIcon(condpresets::iconSetStrip(condpresets::iconGroups().first().sets.first(), 40)), can);
        tab.choices << choice(QStringLiteral("fmt.greater"), tr("Greater Than"), average ? tr("Highlights the cells with a number greater than the average of the numbers.") : tr("There are too many cells to find their average."), icon(QStringLiteral("arrow-up-wide-narrow")), can && average);
        tab.choices << choice(QStringLiteral("fmt.top"), tr("Top 10%"), tr("Highlights the cells with the highest tenth of the numbers."), icon(QStringLiteral("star")), can);
        QAction *clear = act(QStringLiteral("cond.clearSelection"));
        tab.choices << choice(QStringLiteral("fmt.clear"), tr("Clear Format"), tr("Takes the conditional formatting off the selected cells."), icon(QStringLiteral("eraser")), can && clear && clear->isEnabled());
        tabs << tab;
    }

    // ---- Charts: the kinds the recommended charts offer (up to four)
    if (m_objects && m_objects->cells()) {
        ChartPlan plan;
        m_previewBlock = CellRange();   // (found again, once, for this box)
        QVector<ChartKind> kinds;
        if (chartPlanFor(ChartKind::Column, -1, &plan)) kinds = recommendChartKinds(*m_objects->cells(), m_sheet, plan);
        if (!kinds.isEmpty()) {
            QuickTab tab;
            tab.id = QStringLiteral("charts");
            tab.title = tr("Charts");
            const bool can = canWrite && m_session->allows(m_sheet, Allow::Objects);
            for (qsizetype i = 0; i < std::min<qsizetype>(kinds.size(), kMostCharts); ++i) {
                const QString name = chartKindName(kinds[i]);
                tab.choices << choice(QStringLiteral("chart.%1").arg(int(kinds[i])), name, tr("Makes a chart of the selected cells: %1.").arg(name), QIcon(QPixmap::fromImage(chartPreviewImage(kinds[i], -1, QSize(120, 84)))), can);
            }
            tabs << tab;
        }
    }

    // ---- Totals: under the columns, or right of the rows
    {
        QuickTab tab;
        tab.id = QStringLiteral("totals");
        tab.title = tr("Totals");
        for (const bool down : {true, false})
            for (const Total kind : kTotals)
                tab.choices << choice(QStringLiteral("tot.%1.%2").arg(int(kind)).arg(down ? QStringLiteral("down") : QStringLiteral("across")), totalName(kind, down), totalTip(kind, down), icon(QString::fromLatin1(iconOfTotal(kind))), canWrite);
        tabs << tab;
    }

    // ---- Tables
    {
        QuickTab tab;
        tab.id = QStringLiteral("tables");
        tab.title = tr("Tables");
        tab.choices << choice(QStringLiteral("tab.table"), tr("Table"), tr("Makes the selected cells a table, with buttons to sort and filter."), icon(QStringLiteral("table")), canWrite && selectedTable() < 0);
        tab.choices << choice(QStringLiteral("tab.pivot"), tr("Pivot Table"), tr("Makes a pivot table that totals the data, on a new sheet. The first row should hold the names of the columns."), icon(QStringLiteral("table-properties")),
                              canWrite && m_session->allows(m_sheet, Allow::Pivot) && r.row2 > r.row1);
        tabs << tab;
    }

    // ---- Sparklines: one for each row, in the cells to the right
    {
        QuickTab tab;
        tab.id = QStringLiteral("sparklines");
        tab.title = tr("Sparklines");
        const bool can = canWrite && r.col2 > r.col1;
        const QString where = tr("Draws a small chart of each row in the cell to the right of it.");
        tab.choices << choice(QStringLiteral("spark.line"), tr("Line"), where, icon(QStringLiteral("activity")), can);
        tab.choices << choice(QStringLiteral("spark.column"), tr("Column"), where, icon(QStringLiteral("chart-column")), can);
        tab.choices << choice(QStringLiteral("spark.winloss"), tr("Win/Loss"), where, icon(QStringLiteral("chart-no-axes-column")), can);
        tabs << tab;
    }
    return tabs;
}

// ---- looking at a choice (nothing of the workbook is changed: a rule shown is taken out again, and the rest is drawn over the sheet)

void CrunchWindow::quickPreview(const QString &id)
{
    if (!m_session || !m_quickLayer || !m_quickRange.valid) return;
    const CellRange range = m_quickRange;
    const QPalette pal = m_view->viewport()->palette();
    const QColor accent = pal.color(QPalette::Highlight), base = pal.color(QPalette::Base), ink = pal.color(QPalette::Text);
    if (id.startsWith(QLatin1String("fmt."))) {
        CondSpec spec;
        QString why;
        CondRule rule;
        DifferentialFormat look;
        if (id == QLatin1String("fmt.clear") || !quickCondSpec(id, &spec, &why) || !condRuleOf(spec, &rule, &look, &why)) return;
        m_session->previewCondRule(m_sheet, rule, look);
    } else if (id.startsWith(QLatin1String("chart."))) {
        const ChartKind kind = ChartKind(id.mid(6).toInt());
        const QImage image = chartPreviewImage(kind, -1, QSize(360, 230));
        if (image.isNull()) return;
        m_quickLayer->showLook([image, base, accent, host = QPointer<QWidget>(m_view->viewport())](QPainter &p) {
            if (!host) return;
            const QRect at(QPoint(std::max(8, host->width() - image.width() - 16), 8), image.size());
            p.fillRect(at.adjusted(-4, -4, 4, 4), base);
            p.drawImage(at.topLeft(), image);
            p.setPen(accent);
            p.drawRect(at.adjusted(-4, -4, 3, 3));
        });
    } else if (id.startsWith(QLatin1String("tot."))) {
        const QStringList parts = id.split(QLatin1Char('.'));
        if (parts.size() != 3) return;
        const bool down = parts[2] == QLatin1String("down");
        const crunch::EditSession::QuickTotalsPlan plan = m_session->planQuickTotals(m_sheet, range, Total(parts[1].toInt()), down);
        if (!plan.ok) return;
        QVector<QPair<QPoint, QString>> cells;   // (row and column, and what the formula comes to)
        QVector<QString> shown;
        QVector<QPoint> at;
        for (size_t i = 0; i < plan.cells.size() && int(i) < kLookCells; ++i) {
            const crunch::EditSession::QuickCell &c = plan.cells[i];
            const auto v = m_session->previewFormula(c.formula, m_sheet, c.row, c.col);
            QString text = v.ok || v.error ? v.shown : QString();
            if (c.percent && v.ok && !v.error) {
                bool ok = false;
                const double d = jp::systemLocale().locale.toDouble(v.shown, &ok);
                if (ok) text = jp::systemLocale().toString(d * 100, 'f', 1) + QLatin1Char('%');
            }
            shown.append(text);
            at.append(QPoint(int(c.col), int(c.row)));
        }
        m_quickLayer->showLook([this, shown, at, base, accent, ink](QPainter &p) {
            p.setFont(m_view->font());
            for (int i = 0; i < at.size(); ++i) {
                const QRect cell = m_view->cellRect(at[i].y(), at[i].x());
                if (cell.isNull()) continue;
                p.fillRect(cell, base);
                QColor tint = accent;
                tint.setAlpha(70);
                p.fillRect(cell, tint);
                p.setPen(ink);
                p.drawText(cell.adjusted(2, 0, -3, 0), Qt::AlignRight | Qt::AlignVCenter, shown[i]);
            }
        });
    } else if (id == QLatin1String("tab.table")) {
        const bool header = m_session->looksLikeHeader(m_sheet, range);
        m_quickLayer->showLook([this, range, header, accent](QPainter &p) {
            const QRect all = rectOf(m_view, range);
            if (all.isNull()) return;
            QColor strong = accent, soft = accent;
            strong.setAlpha(150);
            soft.setAlpha(45);
            for (quint32 r = range.row1; r <= range.row2 && r < range.row1 + 400; ++r) {
                const QRect line = m_view->cellRect(r, range.col1).united(m_view->cellRect(r, range.col2));
                if (line.isNull()) continue;
                if (header && r == range.row1) p.fillRect(line, strong);
                else if (((r - range.row1) % 2 == (header ? 1u : 0u))) p.fillRect(line, soft);
            }
            p.setPen(accent);
            p.drawRect(all.adjusted(0, 0, -1, -1));
        });
    } else if (id == QLatin1String("tab.pivot")) {
        const QString note = tr("A pivot table that totals this data will be made on a new sheet.");
        m_quickLayer->showLook([this, range, note, base, accent, ink](QPainter &p) {
            const QRect all = rectOf(m_view, range);
            if (!all.isNull()) {
                QColor tint = accent;
                tint.setAlpha(50);
                p.fillRect(all, tint);
            }
            const QFontMetrics fm(p.font());
            const QRect box(QPoint(12, 12), QSize(fm.horizontalAdvance(note) + 20, fm.height() + 14));
            p.fillRect(box, base);
            p.setPen(accent);
            p.drawRect(box);
            p.setPen(ink);
            p.drawText(box, Qt::AlignCenter, note);
        });
    } else if (id.startsWith(QLatin1String("spark."))) {
        const crunch::EditSession::QuickTotalsPlan plan = m_session->planQuickTotals(m_sheet, range, Total::Sum, false);
        if (!plan.ok || plan.data.col2 <= plan.data.col1) return;
        const CellRange data = plan.data;
        const int type = id == QLatin1String("spark.line") ? 0 : id == QLatin1String("spark.column") ? 1 : 2;
        QVector<QVector<double>> lines;
        QVector<quint32> rows;
        for (quint32 r = data.row1; r <= data.row2 && lines.size() < kLookCells; ++r) {
            QVector<double> values;
            for (quint32 c = data.col1; c <= data.col2 && values.size() < 64; ++c) {
                const jp::calc::Value v = m_session->valueAt(m_sheet, r, c);
                values.append(v.isNumber() ? v.number() : 0.0);
            }
            lines.append(values);
            rows.append(r);
        }
        const quint32 col = data.col2 + 1;
        m_quickLayer->showLook([this, lines, rows, col, type, base, accent](QPainter &p) {
            for (int i = 0; i < rows.size(); ++i) {
                const QRect cell = m_view->cellRect(rows[i], col);
                if (cell.isNull() || lines[i].isEmpty()) continue;
                p.fillRect(cell, base);
                const QRect box = cell.adjusted(3, 3, -3, -3);
                const auto [lo, hi] = std::minmax_element(lines[i].begin(), lines[i].end());
                const double low = std::min(0.0, *lo), span = std::max(1e-12, std::max(0.0, *hi) - low);
                const int n = int(lines[i].size());
                p.setPen(QPen(accent, 1.5));
                p.setBrush(accent);
                if (type == 0) {
                    QPainterPath path;
                    for (int k = 0; k < n; ++k) {
                        const QPointF pt(box.left() + (n > 1 ? box.width() * double(k) / (n - 1) : box.width() / 2.0), box.bottom() - box.height() * (lines[i][k] - low) / span);
                        if (k == 0) path.moveTo(pt);
                        else path.lineTo(pt);
                    }
                    p.setBrush(Qt::NoBrush);
                    p.drawPath(path);
                } else {
                    const double w = double(box.width()) / n;
                    for (int k = 0; k < n; ++k) {
                        const double v = lines[i][k];
                        if (type == 1) {
                            const double h = box.height() * (v - low) / span;
                            p.drawRect(QRectF(box.left() + k * w, box.bottom() - h, std::max(1.0, w - 1), h));
                        } else {
                            const double mid = box.center().y(), h = box.height() / 2.0 - 1;
                            if (v > 0) p.drawRect(QRectF(box.left() + k * w, mid - h, std::max(1.0, w - 1), h));
                            else if (v < 0) p.drawRect(QRectF(box.left() + k * w, mid, std::max(1.0, w - 1), h));
                        }
                    }
                }
            }
        });
    }
}

void CrunchWindow::quickPreviewEnd()
{
    if (m_session) m_session->endCondPreview();
    if (m_quickLayer) m_quickLayer->clear();
}

// ---- making a choice: one step of Undo

void CrunchWindow::quickApply(const QString &id)
{
    quickPreviewEnd();
    m_previewBlock = CellRange();
    if (m_edit.active && !commitEdit(CellEditor::Way::Stay)) return;
    if (!m_session || !editingAllowed() || m_chartSheetShown) return;
    const QString title = tr("Quick Analysis");
    CellRange range;
    if (!quickEligible(&range)) return;
    m_quickRange = range;
    if (id == QLatin1String("fmt.clear")) {
        clearCondRules(false);
    } else if (id.startsWith(QLatin1String("fmt."))) {
        CondSpec spec;
        QString why;
        if (!quickCondSpec(id, &spec, &why)) {
            if (!why.isEmpty()) notice(title, why);
            return;
        }
        addCondRuleFromSpec(spec);
    } else if (id.startsWith(QLatin1String("chart."))) {
        insertChart(ChartKind(id.mid(6).toInt()), -1);
    } else if (id.startsWith(QLatin1String("tot."))) {
        const QStringList parts = id.split(QLatin1Char('.'));
        if (parts.size() != 3) return;
        const crunch::EditSession::QuickTotalsPlan plan = m_session->planQuickTotals(m_sheet, range, Total(parts[1].toInt()), parts[2] == QLatin1String("down"));
        if (!plan.ok) {
            notice(title, plan.problem);
            return;
        }
        if (plan.overwrites > 0 && !confirm(title, tr("Some of the cells where the totals go are not empty. Replace what is in them?"))) return;
        const auto o = m_session->applyQuickTotals(m_sheet, plan, title);
        if (!o.ok) notice(title, o.problem);
    } else if (id == QLatin1String("tab.table")) {
        const auto o = m_session->makeTable(m_sheet, range, m_session->looksLikeHeader(m_sheet, range), QString());
        if (!o.ok) notice(title, o.problem);
    } else if (id == QLatin1String("tab.pivot")) {
        const QString source = jp::office::sheet::sheetNamePrefix(m_loaded->book->info().sheets[m_sheet].name) + jp::office::sheet::rangeName(range);
        QString why;
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
        const QVector<crunch::PivotAdvice> advice = m_session->recommendPivots(source, &why);
        QGuiApplication::restoreOverrideCursor();
        if (advice.isEmpty()) {
            notice(title, why);
            return;
        }
        int made = -1;
        const auto o = m_session->createRecommendedPivot(source, advice.first(), &made);
        if (!o.ok) {
            notice(title, o.problem);
            return;
        }
        if (made >= 0) {
            const CellRange at = m_session->workbook()->pivots().locationOf(made);
            if (at.valid) goToFound(m_session->workbook()->pivots().table(made).sheet, at.row1, at.col1);
        }
        showPivotFields(true);
    } else if (id.startsWith(QLatin1String("spark."))) {
        const crunch::EditSession::QuickTotalsPlan plan = m_session->planQuickTotals(m_sheet, range, Total::Sum, false);
        if (!plan.ok) {
            notice(title, plan.problem);
            return;
        }
        if (plan.data.col2 <= plan.data.col1) {
            notice(title, tr("A sparkline needs at least two numbers in a row."));
            return;
        }
        CellRange where{plan.data.row1, plan.data.col2 + 1, plan.data.row2, plan.data.col2 + 1, true};
        QVector<jp::office::sheet::Sparkline> made;
        QString why;
        if (!makeSparklines(jp::office::sheet::rangeName(plan.data), jp::office::sheet::rangeName(where), &made, &why)) {
            notice(title, why);
            return;
        }
        putSparklines(id == QLatin1String("spark.line") ? SparklineGroup::Type::Line : id == QLatin1String("spark.column") ? SparklineGroup::Type::Column : SparklineGroup::Type::WinLoss, made);
    }
    m_view->setFocus(Qt::OtherFocusReason);
}

} // namespace jp
