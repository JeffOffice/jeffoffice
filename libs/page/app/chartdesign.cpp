#include "app/chartdesign.h"

#include "app/chartdialogs.h"
#include "app/chartedit.h"
#include "app/plaintext.h"
#include "office/chart_write.h"
#include "sheet/chartedit.h"

#include <QActionGroup>
#include <QCoreApplication>
#include <QFileInfo>
#include <QImage>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QEvent>
#include <QGuiApplication>
#include <QPainter>
#include <QTimer>
#include <QSvgRenderer>

#include <algorithm>

namespace jp {

using namespace jp::office;
using namespace jp::office::sheet;

ChartDesignHost::~ChartDesignHost() = default;

bool ChartDesignHost::askChartText(const QString &caption, const QString &prompt, QString *text)
{
    bool ok = false;
    const QString typed = QInputDialog::getText(chartParent(), caption, prompt, QLineEdit::Normal, *text, &ok);
    if (ok) *text = typed;
    return ok;
}

ChartDesign::ChartDesign(ChartDesignHost *host, QObject *parent) : QObject(parent), m_host(host) {}

QMenu *ChartDesign::elementsMenu(QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("chartElementsMenu"));
    connect(menu, &QMenu::aboutToShow, this, [this, menu] {
        menu->clear();
        Chart chart;
        if (!m_host->chartNow(&chart)) return;
        const ChartElements e = elementsOf(chart);
        auto check = [&](QMenu *into, const QString &text, bool on, const std::function<void(Chart *)> &change, const QString &step, const char *key = nullptr) {
            QAction *a = into->addAction(text);
            // (the name is the menu's and the English words of the entry, which do not change with the language: for tests and for the commands that look an entry up)
            if (key) a->setObjectName(into->objectName() + QLatin1Char('.') + QLatin1String(key));
            a->setCheckable(true);
            a->setChecked(on);
            connect(a, &QAction::triggered, this, [this, change, step] { m_host->changeChart(step, change); });
            return a;
        };
        auto choice = [&](QMenu *into, QActionGroup *group, const QString &text, bool on, const std::function<void(Chart *)> &change, const QString &step, const char *key = nullptr) {
            QAction *a = check(into, text, on, change, step, key);
            group->addAction(a);
            return a;
        };
        // Title text: typed words, or (with cells) "=Sheet1!A1" for the words of a cell (the title follows that cell).
        auto titleText = [this](QMenu *into, const QString &label, const QString &caption, const ChartTitle &now, std::function<void(Chart *, const QString &, const QString &)> put) {
            QAction *a = into->addAction(label);
            connect(a, &QAction::triggered, this, [this, caption, now, put] {
                const bool cells = m_host->chartHasCells();
                QString text = now.ref.isEmpty() || !cells ? now.text : QStringLiteral("=") + now.ref;
                if (!m_host->askChartText(caption, cells ? tr("The words, or = and a cell such as =Sheet1!A1 to show what that cell holds:") : tr("The words:"), &text)) return;
                text = text.trimmed();
                QString ref, words = text;
                if (cells && text.startsWith(QLatin1Char('='))) {
                    // (one cell, written with $ the way a chart keeps it)
                    ref = m_host->chartCellRef(text.mid(1), true, caption);
                    if (ref.isEmpty()) return;
                    words = QString();
                }
                m_host->changeChart(caption, [this, put, words, ref](Chart *c) {
                    put(c, words, ref);
                    if (!ref.isEmpty()) m_host->chartReadCells(c);
                });
            });
        };
        if (e.hasHorizontal || e.hasVertical) {
            QMenu *axes = menu->addMenu(tr("Axes"));
            axes->setObjectName(QStringLiteral("axesMenu"));
            if (e.hasHorizontal) check(axes, tr("Horizontal"), e.axisHorizontal, [on = !e.axisHorizontal](Chart *c) { setAxisShown(c, true, on); }, tr("Axes"), "Horizontal");
            if (e.hasVertical) check(axes, tr("Vertical"), e.axisVertical, [on = !e.axisVertical](Chart *c) { setAxisShown(c, false, on); }, tr("Axes"), "Vertical");
            QMenu *titles = menu->addMenu(tr("Axis Titles"));
            titles->setObjectName(QStringLiteral("axisTitlesMenu"));
            if (e.hasHorizontal) check(titles, tr("Horizontal"), e.axisTitleHorizontal, [on = !e.axisTitleHorizontal](Chart *c) { setAxisTitle(c, true, on); }, tr("Axis Titles"), "Horizontal");
            if (e.hasVertical) check(titles, tr("Vertical"), e.axisTitleVertical, [on = !e.axisTitleVertical](Chart *c) { setAxisTitle(c, false, on); }, tr("Axis Titles"), "Vertical");
            titles->addSeparator();
            if (e.hasHorizontal)
                if (const ChartAxis *a = primaryAxis(chart, true))
                    titleText(titles, tr("Horizontal Title Text…"), tr("Axis Titles"), a->title, [](Chart *c, const QString &t, const QString &r) { setAxisTitleText(c, true, t, r); });
            if (e.hasVertical)
                if (const ChartAxis *a = primaryAxis(chart, false))
                    titleText(titles, tr("Vertical Title Text…"), tr("Axis Titles"), a->title, [](Chart *c, const QString &t, const QString &r) { setAxisTitleText(c, false, t, r); });
            if (e.hasSecondaryHorizontal) {
                check(axes, tr("Second Horizontal"), e.axisSecondaryHorizontal, [on = !e.axisSecondaryHorizontal](Chart *c) { setAxisShown(c, true, on, true); }, tr("Axes"), "Second Horizontal");
                check(titles, tr("Second Horizontal"), e.axisTitleSecondaryHorizontal, [on = !e.axisTitleSecondaryHorizontal](Chart *c) { setAxisTitle(c, true, on, true); }, tr("Axis Titles"), "Second Horizontal");
            }
            if (e.hasSecondaryVertical) {
                check(axes, tr("Second Vertical"), e.axisSecondaryVertical, [on = !e.axisSecondaryVertical](Chart *c) { setAxisShown(c, false, on, true); }, tr("Axes"), "Second Vertical");
                check(titles, tr("Second Vertical"), e.axisTitleSecondaryVertical, [on = !e.axisTitleSecondaryVertical](Chart *c) { setAxisTitle(c, false, on, true); }, tr("Axis Titles"), "Second Vertical");
            }
        }
        QMenu *title = menu->addMenu(tr("Chart Title"));
        title->setObjectName(QStringLiteral("chartTitleMenu"));
        auto *titleGroup = new QActionGroup(title);
        choice(title, titleGroup, tr("None"), !e.title, [](Chart *c) { setChartTitle(c, TitleMode::None); }, tr("Chart Title"), "None");
        choice(title, titleGroup, tr("Above"), e.title && !e.titleOverlay, [](Chart *c) { setChartTitle(c, TitleMode::Above); }, tr("Chart Title"), "Above");
        choice(title, titleGroup, tr("On Top"), e.titleOverlay, [](Chart *c) { setChartTitle(c, TitleMode::Overlay); }, tr("Chart Title"), "On Top");
        title->addSeparator();
        titleText(title, tr("Title Text…"), tr("Chart Title"), chart.title, [](Chart *c, const QString &t, const QString &r) { setChartTitleText(c, t, r); });
        QMenu *labels = menu->addMenu(tr("Data Labels"));
        labels->setObjectName(QStringLiteral("dataLabelsMenu"));
        auto *labelGroup = new QActionGroup(labels);
        choice(labels, labelGroup, tr("None"), !e.labels, [](Chart *c) { setDataLabels(c, false); }, tr("Data Labels"), "None");
        choice(labels, labelGroup, tr("Usual place"), e.labels && e.labelPosition.isEmpty(), [](Chart *c) { setDataLabels(c, true); }, tr("Data Labels"), "Usual place");
        static const struct { const char *code; const char *name; } places[] = {{"ctr", QT_TRANSLATE_NOOP("jp::ChartDesign", "Center")}, {"inEnd", QT_TRANSLATE_NOOP("jp::ChartDesign", "Inside End")}, {"inBase", QT_TRANSLATE_NOOP("jp::ChartDesign", "Inside Base")},
                                                                                  {"outEnd", QT_TRANSLATE_NOOP("jp::ChartDesign", "Outside End")}, {"bestFit", QT_TRANSLATE_NOOP("jp::ChartDesign", "Best Fit")}, {"l", QT_TRANSLATE_NOOP("jp::ChartDesign", "Left")},
                                                                                  {"r", QT_TRANSLATE_NOOP("jp::ChartDesign", "Right")}, {"t", QT_TRANSLATE_NOOP("jp::ChartDesign", "Above")}, {"b", QT_TRANSLATE_NOOP("jp::ChartDesign", "Below")}};
        const QStringList allowed = labelPositionsFor(chart);
        for (const auto &p : places) {
            const QString code = QString::fromLatin1(p.code);
            if (!allowed.contains(code)) continue;
            choice(labels, labelGroup, QCoreApplication::translate("jp::ChartDesign", p.name), e.labels && e.labelPosition == code, [code](Chart *c) { setDataLabels(c, true, code); }, tr("Data Labels"), p.code);
        }
        {
            labels->addSeparator();
            const ChartLabelParts parts = labelPartsOf(chart);
            QMenu *contains = labels->addMenu(tr("Label Contains"));
            contains->setObjectName(QStringLiteral("labelContainsMenu"));
            struct Part {
                QString text;
                bool on;
                bool ChartLabelParts::*member;
            };
            const Part list[] = {{tr("Value"), parts.value, &ChartLabelParts::value}, {tr("Category Name"), parts.category, &ChartLabelParts::category},
                                 {tr("Series Name"), parts.series, &ChartLabelParts::series}, {tr("Percentage"), parts.percent, &ChartLabelParts::percent},
                                 {tr("Legend Key"), parts.legendKey, &ChartLabelParts::legendKey}};
            for (const Part &part : list)
                check(contains, part.text, part.on, [parts, member = part.member, on = !part.on](Chart *c) {
                    ChartLabelParts now = parts;
                    now.*member = on;
                    setDataLabelParts(c, now);
                }, tr("Data Labels"))->setObjectName(contains->objectName() + QStringLiteral(".part%1").arg(int(&part - list)));
            QMenu *separator = labels->addMenu(tr("Separator"));
            separator->setObjectName(QStringLiteral("labelSeparatorMenu"));
            auto *separatorGroup = new QActionGroup(separator);
            const struct { QString text, code; } separators[] = {{tr("Comma"), QString()}, {tr("Semicolon"), QStringLiteral("; ")}, {tr("Period"), QStringLiteral(". ")},
                                                                  {tr("New Line"), QStringLiteral("\n")}, {tr("Space"), QStringLiteral(" ")}};
            for (const auto &sep : separators)
                choice(separator, separatorGroup, sep.text, parts.separator == sep.code, [parts, code = sep.code](Chart *c) {
                    ChartLabelParts now = parts;
                    now.separator = code;
                    setDataLabelParts(c, now);
                }, tr("Data Labels"))->setObjectName(separator->objectName() + QStringLiteral(".sep%1").arg(int(&sep - separators)));
            // Value From Cells: the labels show the words of a range of cells, one for each point (empty: none).
            if (m_host->chartHasCells()) {
                QAction *fromCells = labels->addAction(tr("Value From Cells…"));
                fromCells->setObjectName(QStringLiteral("labelsFromCellsAction"));
                fromCells->setCheckable(true);
                fromCells->setChecked(!dataLabelsCellsRef(chart).isEmpty());
                connect(fromCells, &QAction::triggered, this, [this, now = dataLabelsCellsRef(chart)] {
                    QString text = now;
                    if (!m_host->askChartText(tr("Value From Cells"), tr("The cells that hold the words for the labels, such as =Sheet1!$D$2:$D$4 (leave empty to take them away):"), &text)) return;
                    text = text.trimmed();
                    if (text.startsWith(QLatin1Char('='))) text = text.mid(1).trimmed();
                    if (!text.isEmpty()) {
                        text = m_host->chartCellRef(text, false, tr("Value From Cells"));
                        if (text.isEmpty()) return;
                    }
                    m_host->changeChart(tr("Data Labels"), [this, text](Chart *c) {
                        setDataLabelsFromCells(c, text);
                        if (!text.isEmpty()) m_host->chartReadCells(c);
                    });
                });
            }
            QAction *format = labels->addAction(tr("Number Format…"));
            connect(format, &QAction::triggered, this, [this, parts] {
                QString code = parts.format;
                if (!m_host->askChartText(tr("Number Format"), tr("Format code for the numbers in the labels (leave empty to show the numbers as they are):"), &code)) return;
                m_host->changeChart(tr("Data Labels"), [parts, code](Chart *c) {
                    ChartLabelParts now = parts;
                    now.format = code.trimmed();
                    setDataLabelParts(c, now);
                });
            });
        }
        if (e.hasHorizontal || e.hasVertical) {
            QMenu *grid = menu->addMenu(tr("Gridlines"));
            grid->setObjectName(QStringLiteral("gridlinesMenu"));
            check(grid, tr("Lines Across, Major"), e.gridHorizontal, [on = !e.gridHorizontal](Chart *c) { setGridlines(c, true, false, on); }, tr("Gridlines"), "Lines Across, Major");
            check(grid, tr("Lines Up and Down, Major"), e.gridVertical, [on = !e.gridVertical](Chart *c) { setGridlines(c, false, false, on); }, tr("Gridlines"), "Lines Up and Down, Major");
            check(grid, tr("Lines Across, Minor"), e.minorGridHorizontal, [on = !e.minorGridHorizontal](Chart *c) { setGridlines(c, true, true, on); }, tr("Gridlines"), "Lines Across, Minor");
            check(grid, tr("Lines Up and Down, Minor"), e.minorGridVertical, [on = !e.minorGridVertical](Chart *c) { setGridlines(c, false, true, on); }, tr("Gridlines"), "Lines Up and Down, Minor");
        }
        QMenu *legend = menu->addMenu(tr("Legend"));
        legend->setObjectName(QStringLiteral("legendMenu"));
        auto *legendGroup = new QActionGroup(legend);
        choice(legend, legendGroup, tr("None"), !e.legend, [](Chart *c) { setLegend(c, QString()); }, tr("Legend"), "None");
        const struct { const char *code; const char *name; } sides[] = {{"r", QT_TRANSLATE_NOOP("jp::ChartDesign", "Right")}, {"t", QT_TRANSLATE_NOOP("jp::ChartDesign", "Top")}, {"l", QT_TRANSLATE_NOOP("jp::ChartDesign", "Left")}, {"b", QT_TRANSLATE_NOOP("jp::ChartDesign", "Bottom")}};
        for (const auto &s : sides) {
            const QString code = QString::fromLatin1(s.code);
            choice(legend, legendGroup, QCoreApplication::translate("jp::ChartDesign", s.name), e.legend && e.legendPosition == code, [code, over = e.legendOverlay](Chart *c) { setLegend(c, code, over); }, tr("Legend"), s.code);
        }
        if (e.legend) {
            legend->addSeparator();
            check(legend, tr("Over the Chart"), e.legendOverlay, [code = e.legendPosition, on = !e.legendOverlay](Chart *c) { setLegend(c, code, on); }, tr("Legend"), "Over the Chart");
        }
        if (errorBarsApply(chart)) {
            QMenu *bars = menu->addMenu(tr("Error Bars"));
            bars->setObjectName(QStringLiteral("errorBarsMenu"));
            auto *barGroup = new QActionGroup(bars);
            const QString kind = errorBarsKind(chart);
            choice(bars, barGroup, tr("None"), kind.isEmpty(), [](Chart *c) { setErrorBars(c, QString()); }, tr("Error Bars"), "None");
            choice(bars, barGroup, tr("Standard Error"), kind == QLatin1String("stdErr"), [](Chart *c) { setErrorBars(c, QStringLiteral("stdErr")); }, tr("Error Bars"), "Standard Error");
            choice(bars, barGroup, tr("Percentage (5%)"), kind == QLatin1String("percentage"), [](Chart *c) { setErrorBars(c, QStringLiteral("percentage"), 5); }, tr("Error Bars"), "Percentage (5%)");
            choice(bars, barGroup, tr("Standard Deviation"), kind == QLatin1String("stdDev"), [](Chart *c) { setErrorBars(c, QStringLiteral("stdDev"), 1); }, tr("Error Bars"), "Standard Deviation");
            choice(bars, barGroup, tr("Fixed Value (1)"), kind == QLatin1String("fixedVal"), [](Chart *c) { setErrorBars(c, QStringLiteral("fixedVal"), 1); }, tr("Error Bars"), "Fixed Value (1)");
            // Custom: the amounts above and below each point are read from cells typed in (empty: none on that side).
            if (m_host->chartHasCells()) {
                QAction *custom = bars->addAction(tr("Custom…"));
                custom->setObjectName(QStringLiteral("customErrorBarsAction"));
                custom->setCheckable(true);
                custom->setChecked(kind == QLatin1String("cust"));
                connect(custom, &QAction::triggered, this, [this, now = customErrorBarRefs(chart)] {
                    QStringList refs{now.value(0), now.value(1)};
                    for (int side = 0; side < 2; ++side) {
                        const QString title = side == 0 ? tr("Custom Error Bars: Positive") : tr("Custom Error Bars: Negative");
                        QString text = refs[side];
                        if (!m_host->askChartText(title, tr("The cells that hold the amounts, such as =Sheet1!$D$2:$D$4 (leave empty for none):"), &text)) return;
                        text = text.trimmed();
                        if (text.startsWith(QLatin1Char('='))) text = text.mid(1).trimmed();
                        if (!text.isEmpty()) {
                            text = m_host->chartCellRef(text, false, tr("Custom Error Bars"));
                            if (text.isEmpty()) return;
                        }
                        refs[side] = text;
                    }
                    if (refs[0].isEmpty() && refs[1].isEmpty()) return;
                    m_host->changeChart(tr("Error Bars"), [this, refs](Chart *c) {
                        setCustomErrorBars(c, refs[0], refs[1]);
                        m_host->chartReadCells(c);
                    });
                });
            }
        }
        if (!chart.pivotName.isEmpty()) {
            QMenu *buttons = menu->addMenu(tr("Field Buttons"));
            buttons->setObjectName(QStringLiteral("fieldButtonsMenu"));
            const struct { const char *name; PivotButton which; } kinds[] = {{QT_TRANSLATE_NOOP("jp::ChartDesign", "Report Filter Field Buttons"), PivotButton::Filter},
                                                                             {QT_TRANSLATE_NOOP("jp::ChartDesign", "Axis Field Buttons"), PivotButton::Axis},
                                                                             {QT_TRANSLATE_NOOP("jp::ChartDesign", "Legend Field Buttons"), PivotButton::Legend},
                                                                             {QT_TRANSLATE_NOOP("jp::ChartDesign", "Value Field Buttons"), PivotButton::Value}};
            for (const auto &k : kinds)
                check(buttons, QCoreApplication::translate("jp::ChartDesign", k.name), pivotButtonsOn(chart, k.which), [which = k.which, on = !pivotButtonsOn(chart, k.which)](Chart *c) { setPivotButtons(c, which, on); }, tr("Field Buttons"), k.name);
            buttons->addSeparator();
            check(buttons, tr("Show All"), pivotButtonsOn(chart, PivotButton::All), [](Chart *c) { setPivotButtons(c, PivotButton::All, true); }, tr("Field Buttons"), "Show All");
            check(buttons, tr("Hide All"), chart.pivotButtons.set && !chart.pivotButtons.filter && !chart.pivotButtons.categories && !chart.pivotButtons.series && !chart.pivotButtons.data, [](Chart *c) { setPivotButtons(c, PivotButton::All, false); }, tr("Field Buttons"), "Hide All");
        }
        if (plotLinesApply(chart, PlotLines::Drop)) {
            QMenu *lines = menu->addMenu(tr("Lines"));
            lines->setObjectName(QStringLiteral("plotLinesMenu"));
            check(lines, tr("Drop Lines"), plotLinesOn(chart, PlotLines::Drop), [on = !plotLinesOn(chart, PlotLines::Drop)](Chart *c) { setPlotLines(c, PlotLines::Drop, on); }, tr("Lines"), "Drop Lines");
            if (plotLinesApply(chart, PlotLines::HighLow)) {
                check(lines, tr("High-Low Lines"), plotLinesOn(chart, PlotLines::HighLow), [on = !plotLinesOn(chart, PlotLines::HighLow)](Chart *c) { setPlotLines(c, PlotLines::HighLow, on); }, tr("Lines"), "High-Low Lines");
                check(lines, tr("Up/Down Bars"), plotLinesOn(chart, PlotLines::UpDown), [on = !plotLinesOn(chart, PlotLines::UpDown)](Chart *c) { setPlotLines(c, PlotLines::UpDown, on); }, tr("Lines"), "Up/Down Bars");
            }
        }
        if (dataTableApplies(chart)) {
            QMenu *table = menu->addMenu(tr("Data Table"));
            table->setObjectName(QStringLiteral("dataTableMenu"));
            const bool shown = dataTableOn(chart, DataTablePart::Table);
            const bool kept = m_host->chartKeepsDataTable();
            table->setToolTipsVisible(true);
            QAction *show = check(table, tr("Show Data Table"), shown, [on = !shown](Chart *c) { setDataTable(c, DataTablePart::Table, on); }, tr("Data Table"), "Show Data Table");
            if (!kept) {
                show->setEnabled(false);
                show->setToolTip(tr("This file format doesn't keep a data table."));
            }
            table->addSeparator();
            const struct { const char *name; DataTablePart part; } options[] = {{QT_TRANSLATE_NOOP("jp::ChartDesign", "Lines Between Rows"), DataTablePart::RowLines},
                                                                                {QT_TRANSLATE_NOOP("jp::ChartDesign", "Lines Between Columns"), DataTablePart::ColumnLines},
                                                                                {QT_TRANSLATE_NOOP("jp::ChartDesign", "Outline"), DataTablePart::Outline},
                                                                                {QT_TRANSLATE_NOOP("jp::ChartDesign", "Legend Keys"), DataTablePart::Keys}};
            for (const auto &o : options) {
                const bool on = dataTableOn(chart, o.part);
                QAction *a = check(table, QCoreApplication::translate("jp::ChartDesign", o.name), on, [part = o.part, on = !on](Chart *c) { setDataTable(c, part, on); }, tr("Data Table"), o.name);
                a->setEnabled(shown && kept);
            }
        }
        if (chart.type() != ChartType::Pie && chart.type() != ChartType::Doughnut) {
            QMenu *trend = menu->addMenu(tr("Trendline"));
            trend->setObjectName(QStringLiteral("trendlineMenu"));
            auto *trendGroup = new QActionGroup(trend);
            choice(trend, trendGroup, tr("None"), e.trendline.isEmpty(), [](Chart *c) { setTrendlines(c, QString()); }, tr("Trendline"), "None");
            static const struct { const char *code; const char *name; } kinds[] = {{"linear", QT_TRANSLATE_NOOP("jp::ChartDesign", "Linear")}, {"exp", QT_TRANSLATE_NOOP("jp::ChartDesign", "Exponential")}, {"log", QT_TRANSLATE_NOOP("jp::ChartDesign", "Logarithmic")},
                                                                                  {"poly", QT_TRANSLATE_NOOP("jp::ChartDesign", "Polynomial")}, {"power", QT_TRANSLATE_NOOP("jp::ChartDesign", "Power")}, {"movingAvg", QT_TRANSLATE_NOOP("jp::ChartDesign", "Moving Average")}};
            for (const auto &k : kinds) {
                const QString code = QString::fromLatin1(k.code);
                choice(trend, trendGroup, QCoreApplication::translate("jp::ChartDesign", k.name), e.trendline == code, [code](Chart *c) { setTrendlines(c, code); }, tr("Trendline"), k.name);
            }
            if (!e.trendline.isEmpty()) {
                trend->addSeparator();
                check(trend, tr("Show Equation"), e.trendlineEquation, [on = !e.trendlineEquation, r = e.trendlineRSquared](Chart *c) { setTrendlineDisplay(c, on, r); }, tr("Trendline"), "Show Equation");
                check(trend, tr("Show R-squared"), e.trendlineRSquared, [on = !e.trendlineRSquared, q = e.trendlineEquation](Chart *c) { setTrendlineDisplay(c, q, on); }, tr("Trendline"), "Show R-squared");
            }
        }
    });
    return menu;
}

QMenu *ChartDesign::looksMenu(QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("chartLooksMenu"));
    menu->addSection(tr("Colors"));
    for (int i = 0; i < chartColorSchemeCount(); ++i) {
        QAction *a = menu->addAction(chartColorSchemeName(i));
        connect(a, &QAction::triggered, this, [this, i] { m_host->changeChart(tr("Colors"), [i](Chart *c) { applyColorScheme(c, i); }); });
    }
    menu->addSection(tr("Looks"));
    for (int i = 0; i < chartStyleCount(); ++i) {
        QAction *a = menu->addAction(chartStyleName(i));
        connect(a, &QAction::triggered, this, [this, i] { m_host->changeChart(tr("Chart Look"), [i](Chart *c) { applyChartStyle(c, i); }); });
    }
    return menu;
}

QMenu *ChartDesign::layoutsMenu(QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("chartLayoutsMenu"));
    for (int i = 0; i < chartLayoutCount(); ++i) {
        QAction *a = menu->addAction(chartLayoutName(i));
        connect(a, &QAction::triggered, this, [this, i] { m_host->changeChart(tr("Layouts"), [i](Chart *c) { applyChartLayout(c, i); }); });
    }
    return menu;
}

QMenu *ChartDesign::colorsMenu(QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("chartColorsMenu"));
    for (int i = 0; i < chartColorSchemeCount(); ++i) {
        QAction *a = menu->addAction(chartColorSchemeName(i));
        connect(a, &QAction::triggered, this, [this, i] { m_host->changeChart(tr("Colors"), [i](Chart *c) { applyColorScheme(c, i); }); });
    }
    return menu;
}

QMenu *ChartDesign::stylesMenu(QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("chartStylesMenu"));
    for (int i = 0; i < chartStyleCount(); ++i) {
        QAction *a = menu->addAction(chartStyleName(i));
        connect(a, &QAction::triggered, this, [this, i] { m_host->changeChart(tr("Chart Look"), [i](Chart *c) { applyChartStyle(c, i); }); });
    }
    return menu;
}

QVector<ChartDesign::FilterEntry> ChartDesign::filterEntries() const
{
    QVector<FilterEntry> out;
    Chart chart;
    if (!m_host->chartNow(&chart)) return out;
    int n = 0;
    for (const ChartPlot &plot : chart.plots)
        for (const ChartSeries &s : plot.series) {
            ++n;
            out.append({s.name.isEmpty() ? tr("Series %1").arg(n) : s.name, !s.filtered});
        }
    return out;
}

bool ChartDesign::setSeriesShown(int entry, bool shown)
{
    Chart now;
    if (!m_host->chartNow(&now)) return false;
    int drawn = 0, total = 0;
    for (const ChartPlot &plot : now.plots)
        for (const ChartSeries &s : plot.series) {
            ++total;
            drawn += !s.filtered;
        }
    if (entry < 0 || entry >= total) return false;
    int at = 0;
    bool already = false;
    for (const ChartPlot &plot : now.plots)
        for (const ChartSeries &s : plot.series)
            if (at++ == entry) already = !s.filtered;
    if (already == shown) return false;
    if (!shown && drawn <= 1) {
        m_host->chartNotice(tr("Chart Filters"), tr("A chart needs at least one series. Check another series first."), false);
        return false;
    }
    return m_host->changeChart(tr("Chart Filters"), [entry, shown](Chart *c) {
        int n = 0;
        for (ChartPlot &plot : c->plots)
            for (ChartSeries &s : plot.series)
                if (n++ == entry) s.filtered = !shown;
    });
}

QMenu *ChartDesign::filtersMenu(QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("chartFiltersMenu"));
    connect(menu, &QMenu::aboutToShow, this, [this, menu] {
        menu->clear();
        menu->addSection(tr("Series"));
        const QVector<FilterEntry> entries = filterEntries();
        for (int i = 0; i < entries.size(); ++i) {
            QAction *a = menu->addAction(entries[i].name);
            a->setCheckable(true);
            a->setChecked(entries[i].shown);
            connect(a, &QAction::triggered, this, [this, i](bool on) { setSeriesShown(i, on); });
        }
    });
    return menu;
}

// ---- the commands of the Chart Design tab

bool chartOwnData(const Chart &chart, ChartData *data) { return chartDataProblem(chart, data).isEmpty(); }

bool switchedChartData(const ChartData &data, ChartData *out)
{
    if (data.categories.size() > kMostChartSeries || data.seriesNames.size() > kMostChartCategories) return false;
    ChartData made;
    made.categories = data.seriesNames;
    made.seriesNames = data.categories;
    for (int c = 0; c < data.categories.size(); ++c) {
        QVector<double> row;
        for (int s = 0; s < data.seriesNames.size(); ++s) row << data.values.value(s).value(c);
        made.values << row;
    }
    if (!chartDataIsUsable(made)) return false;
    if (out) *out = made;
    return true;
}

bool ChartDesign::dataEditable() const
{
    Chart chart;
    return m_host->chartNow(&chart) && (chartOwnData(chart, nullptr) || (!chart.externalDataId.isEmpty() && dataProblem().isEmpty()));
}

QString ChartDesign::dataProblem() const
{
    Chart chart;
    if (!m_host->chartNow(&chart)) return QString();
    return chartDataProblem(chart, nullptr, chart.externalDataId.isEmpty() ? QByteArray() : m_host->chartWorkbook());
}

QString ChartDesign::structureProblem() const
{
    Chart chart;
    if (!m_host->chartNow(&chart)) return QString();
    const QString why = chartDataProblem(chart, nullptr, chart.externalDataId.isEmpty() ? QByteArray() : m_host->chartWorkbook());
    if (!why.isEmpty() || chart.externalDataId.isEmpty()) return why;
    return workbookShapeProblem();
}

QString ChartDesign::workbookShapeProblem()
{
    return tr("The numbers of this chart are cells of a workbook that is kept with it, so a row or column can't be added, taken away, or swapped here. Edit Data changes the numbers and names.");
}

void ChartDesign::editData()
{
    Chart chart;
    ChartData data;
    if (!m_host->chartNow(&chart)) return;
    const bool inWorkbook = !chart.externalDataId.isEmpty();
    if (const QString why = chartDataProblem(chart, &data, inWorkbook ? m_host->chartWorkbook() : QByteArray()); !why.isEmpty()) {
        m_host->chartNotice(tr("Edit Data"), why);
        return;
    }
    QString notice;
    if (!askChartData(m_host->chartParent(), &data, &notice, inWorkbook)) {
        if (!notice.isEmpty()) m_host->chartNotice(tr("Edit Data"), notice, false);
        return;
    }
    if (inWorkbook) {
        m_host->changeChartWorkbookData(tr("Edit Data"), data);
        return;
    }
    const Theme theme = m_host->chartTheme();
    m_host->changeChart(tr("Edit Data"), [data, theme](Chart *c) { chartSetData(*c, data, theme); });
}

void ChartDesign::selectData()
{
    Chart chart;
    ChartData data;
    if (!m_host->chartNow(&chart)) return;
    if (const QString why = chart.externalDataId.isEmpty() ? chartDataProblem(chart, &data) : structureProblem(); !why.isEmpty()) {
        m_host->chartNotice(tr("Chart Data"), why);
        return;
    }
    SelectChartDataDialog dialog(data, m_host->chartParent());
    if (dialog.exec() != QDialog::Accepted) return;
    const ChartData made = dialog.result();
    const Theme theme = m_host->chartTheme();
    m_host->changeChart(tr("Chart Data"), [made, theme](Chart *c) { chartSetData(*c, made, theme); });
}

bool ChartDesign::switchRowsAndColumns()
{
    Chart chart;
    ChartData data, switched;
    if (!m_host->chartNow(&chart)) return false;
    if (const QString why = chart.externalDataId.isEmpty() ? chartDataProblem(chart, &data) : structureProblem(); !why.isEmpty()) {
        m_host->chartNotice(tr("Swap Rows and Columns"), why);
        return false;
    }
    const ChartKind kind = kindOfChart(chart);
    if (kind == ChartKind::Scatter || kind == ChartKind::ScatterLines || kind == ChartKind::ScatterSmooth || !switchedChartData(data, &switched)) {
        m_host->chartNotice(tr("Swap Rows and Columns"), tr("The rows and columns of this chart can't be turned the other way."));
        return false;
    }
    const Theme theme = m_host->chartTheme();
    return m_host->changeChart(tr("Swap Rows and Columns"), [switched, theme](Chart *c) { chartSetData(*c, switched, theme); });
}

void ChartDesign::changeType()
{
    Chart chart;
    if (!m_host->chartNow(&chart)) return;
    QStringList names;
    for (const ChartSeries &s : seriesOf(chart)) names << s.name;
    const Theme theme = m_host->chartTheme();
    ChangeChartTypeDialog dialog(names, seriesKindsOf(chart), [chart, theme](const QVector<SeriesKind> &kinds, const QSize &px) { return chartPicture(withSeriesKinds(chart, kinds, theme), px); }, m_host->chartParent());
    if (dialog.exec() != QDialog::Accepted) return;
    const QVector<SeriesKind> chosen = dialog.kinds();
    m_host->changeChart(tr("Chart Kind"), [chosen, theme](Chart *c) { *c = withSeriesKinds(*c, chosen, theme); });
}

void ChartDesign::formatAxis(bool horizontal)
{
    Chart chart;
    if (!m_host->chartNow(&chart)) return;
    const ChartAxis *now = primaryAxis(chart, horizontal);
    if (!now) return;
    FormatAxisDialog dialog(*now, horizontal ? tr("Format Horizontal Axis") : tr("Format Vertical Axis"), m_host->chartParent());
    if (dialog.exec() != QDialog::Accepted) return;
    const ChartAxis made = dialog.result();
    if (const QString problem = axisFormatProblem(made); !problem.isEmpty()) {
        m_host->chartNotice(tr("Format Axis"), problem);
        return;
    }
    m_host->changeChart(tr("Format Axis"), [horizontal, made](Chart *c) { setAxisFormat(c, horizontal, false, made); });
}

void ChartDesign::formatPart(int part)
{
    Chart chart;
    if (!m_host->chartNow(&chart)) return;
    FormatChartPartDialog dialog(chart, part, m_host->chartParent());
    if (dialog.exec() != QDialog::Accepted) return;
    const Chart made = dialog.result();
    m_host->changeChart(tr("Format Chart Part"), [made](Chart *c) { *c = made; });
}

bool isOpenDocumentPath(const QString &path)
{
    static const QStringList endings = {QStringLiteral("odt"), QStringLiteral("ods"), QStringLiteral("odp"), QStringLiteral("ott"), QStringLiteral("ots"), QStringLiteral("otp"),
                                        QStringLiteral("fodt"), QStringLiteral("fods"), QStringLiteral("fodp")};
    return endings.contains(QFileInfo(path).suffix().toLower());
}

void addChartContextEntries(QMenu *menu, const std::function<QAction *(const QString &)> &find)
{
    menu->setToolTipsVisible(true);
    for (const char *id : {"chartdesign.selectData", "chartdesign.switch", "chartdesign.changeType", "chartdesign.formatPart", "chartdesign.axisH", "chartdesign.axisV"})
        if (QAction *a = find(QString::fromLatin1(id))) menu->addAction(a);
}

QImage chartPicture(const Chart &chart, const QSize &pixels)
{
    if (pixels.isEmpty()) return QImage();
    Chart shown = chart;
    shown.drawTrendlines = true;
    const QByteArray svg = chartSvg(shown, QSizeF(pixels.width() * 0.75, pixels.height() * 0.75));
    QSvgRenderer renderer(svg);
    if (!renderer.isValid()) return QImage();
    QImage img(pixels, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    renderer.render(&p, QRectF(QPointF(0, 0), QSizeF(pixels)));
    return img;
}

// ---- the buttons beside the chart

ChartButtons::ChartButtons(ChartDesign *design, QWidget *viewport, const std::function<QIcon(const QString &)> &icon, QObject *parent) : QObject(parent), m_design(design), m_viewport(viewport), m_icon(icon)
{
    auto make = [&](const char *name, const char *iconName, const QString &tip) {
        auto *b = new QToolButton(m_viewport);
        b->setObjectName(QLatin1String(name));
        b->setIcon(m_icon(QLatin1String(iconName)));
        b->setToolTip(plainToolTip(tip));
        b->setAccessibleName(tip);
        b->setPopupMode(QToolButton::InstantPopup);
        b->setFocusPolicy(Qt::NoFocus);
        b->setFixedSize(26, 26);
        b->setIconSize(QSize(16, 16));
        b->hide();
        return b;
    };
    m_elements = make("chartElementsButton", "plus", tr("Chart Elements"));
    m_elements->setAutoRaise(false);
    m_elements->setMenu(m_design->elementsMenu(m_elements));
    m_looks = make("chartLooksButton", "brush", tr("Chart Colors and Styles"));
    m_looks->setMenu(m_design->looksMenu(m_looks));
    m_filters = make("chartFiltersButton", "filter", tr("Chart Filters"));
    m_filters->setMenu(m_design->filtersMenu(m_filters));
    m_viewport->installEventFilter(this);
}

bool ChartButtons::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_viewport && event->type() == QEvent::MouseButtonRelease) QTimer::singleShot(0, this, [this] { Q_EMIT released(); });
    return QObject::eventFilter(watched, event);
}

void ChartButtons::place(const QRect &chart)
{
    // (not while a button is down: the chart is being dragged or sized, and the buttons would be in the way)
    if (QGuiApplication::mouseButtons() & Qt::LeftButton) {
        hide();
        return;
    }
    const int size = 26, gap = 12;   // (the gap keeps clear of the handles that stand on the frame)
    const QRect vp = m_viewport->rect();
    int x = chart.right() + gap;
    if (x + size > vp.right()) x = chart.left() - size - gap;   // (no room at the right: at the left)
    if (x < 0) x = std::clamp(chart.right() - size - gap, 0, std::max(0, vp.right() - size));   // (no room at either side: inside, clear of the handles)
    const int y = std::clamp(chart.top(), 0, std::max(0, vp.bottom() - 3 * size - 8));
    m_elements->move(x, y);
    m_looks->move(x, y + size + 4);
    m_filters->move(x, y + 2 * (size + 4));
    for (QToolButton *b : {m_elements, m_looks, m_filters}) {
        b->setVisible(true);
        b->raise();
    }
}

void ChartButtons::hide()
{
    for (QToolButton *b : {m_elements, m_looks, m_filters}) b->setVisible(false);
}

} // namespace jp
