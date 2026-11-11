#include "app/chartactions.h"

#include <QActionGroup>
#include <QCoreApplication>

namespace jp {

namespace {

using office::sheet::ChartAxisKind;
using office::sheet::ChartForm;


struct ChartName {
    ChartForm form;
    const char *id, *text, *icon;
};

const ChartName kChartNames[] = {
    {ChartForm::Column, "column", QT_TRANSLATE_NOOP("jp::ChartActions", "Column"), "chart-column"},
    {ChartForm::ColumnStacked, "columnStacked", QT_TRANSLATE_NOOP("jp::ChartActions", "Stacked Column"), "chart-column-stacked"},
    {ChartForm::ColumnPercent, "columnPercent", QT_TRANSLATE_NOOP("jp::ChartActions", "100% Stacked Column"), "chart-column-stacked"},
    {ChartForm::Bar, "bar", QT_TRANSLATE_NOOP("jp::ChartActions", "Bar"), "chart-bar"},
    {ChartForm::BarStacked, "barStacked", QT_TRANSLATE_NOOP("jp::ChartActions", "Stacked Bar"), "chart-bar-stacked"},
    {ChartForm::Line, "line", QT_TRANSLATE_NOOP("jp::ChartActions", "Line"), "chart-line"},
    {ChartForm::LineMarkers, "lineMarkers", QT_TRANSLATE_NOOP("jp::ChartActions", "Line with Markers"), "chart-spline"},
    {ChartForm::Pie, "pie", QT_TRANSLATE_NOOP("jp::ChartActions", "Pie"), "chart-pie"},
    {ChartForm::Doughnut, "doughnut", QT_TRANSLATE_NOOP("jp::ChartActions", "Doughnut"), "chart-pie"},
    {ChartForm::Area, "area", QT_TRANSLATE_NOOP("jp::ChartActions", "Area"), "chart-area"},
    {ChartForm::AreaStacked, "areaStacked", QT_TRANSLATE_NOOP("jp::ChartActions", "Stacked Area"), "chart-area"},
    {ChartForm::Scatter, "scatter", QT_TRANSLATE_NOOP("jp::ChartActions", "Scatter"), "chart-scatter"},
};

struct LegendName {
    const char *id, *text, *position;
};

const LegendName kLegends[] = {
    {"none", QT_TRANSLATE_NOOP("jp::ChartActions", "No Legend"), ""},
    {"right", QT_TRANSLATE_NOOP("jp::ChartActions", "Legend at Right"), "r"},
    {"top", QT_TRANSLATE_NOOP("jp::ChartActions", "Legend at Top"), "t"},
    {"left", QT_TRANSLATE_NOOP("jp::ChartActions", "Legend at Left"), "l"},
    {"bottom", QT_TRANSLATE_NOOP("jp::ChartActions", "Legend at Bottom"), "b"},
};

} // namespace

ChartActions::ChartActions(Editor *editor, QWidget *window, const Hooks &hooks, QObject *parent) : QObject(parent), m_ed(editor), m_window(window), m_hooks(hooks) {}

void ChartActions::change(const QString &label, const std::function<bool(office::Chart &)> &edit)
{
    changeSelectedChart(m_ed, label, edit);
    if (m_hooks.done) m_hooks.done();
}

void ChartActions::create()
{
    using namespace office::sheet;
    auto mk = [&](const QString &id, const QString &text, const char *icon, const std::function<void()> &fn, bool checkable = false) {
        m_ids << id;
        return m_hooks.make(id, text, QString::fromLatin1(icon), fn, checkable);
    };
    auto find = m_hooks.find;
    const QString changeLabel = QCoreApplication::translate("jp::ChartActions", "Change Chart");
    for (const ChartName &n : kChartNames) {
        const ChartForm form = n.form;
        const QString text = QCoreApplication::translate("jp::ChartActions", n.text);
        mk(QStringLiteral("insert.chart.") + QLatin1String(n.id), text, n.icon, [this, form] {
            insertChart(m_ed, form);
            if (m_hooks.done) m_hooks.done();
        });
        mk(QStringLiteral("chart.type.") + QLatin1String(n.id), text, n.icon, [this, form, changeLabel] {
            const PictureItem *chart = selectedChart(m_ed);
            if (!chart) return;
            const office::Theme theme = chartThemeOfPicture(*m_ed->doc(), *chart);
            change(changeLabel, [form, theme](office::Chart &c) { return chartSetForm(c, form, theme); });
        }, true);
    }
    mk(QStringLiteral("chart.editData"), QCoreApplication::translate("jp::ChartActions", "Edit Data…"), "table-2", [this] { editData(); });
    mk(QStringLiteral("chart.title"), QCoreApplication::translate("jp::ChartActions", "Chart Title"), "type", [this, find, changeLabel] {
        const bool on = find(QStringLiteral("chart.title"))->isChecked();
        const QString text = selectedChartInfo(m_ed).title;
        change(changeLabel, [on, text](office::Chart &c) { return chartSetTitle(c, on, text); });
    }, true);
    mk(QStringLiteral("chart.editTitle"), QCoreApplication::translate("jp::ChartActions", "Edit Chart Title…"), "square-pen", [this] { editTitle(); });
    for (const LegendName &l : kLegends) {
        const QString position = QLatin1String(l.position);
        mk(QStringLiteral("chart.legend.") + QLatin1String(l.id), QCoreApplication::translate("jp::ChartActions", l.text), "list-checks", [this, position, changeLabel] {
            change(changeLabel, [position](office::Chart &c) { return chartSetLegend(c, position); });
        }, true);
    }
    for (const bool values : {false, true}) {
        const QString which = values ? QStringLiteral("value") : QStringLiteral("category");
        const ChartAxisKind kind = values ? ChartAxisKind::Value : ChartAxisKind::Category;
        mk(QStringLiteral("chart.axis.") + which, values ? QCoreApplication::translate("jp::ChartActions", "Axis of Values") : QCoreApplication::translate("jp::ChartActions", "Axis of Categories"), "chart-line", [this, which, kind, find, changeLabel] {
            const bool on = find(QStringLiteral("chart.axis.") + which)->isChecked();
            change(changeLabel, [on, kind](office::Chart &c) { return chartSetAxis(c, kind, on); });
        }, true);
        mk(QStringLiteral("chart.axisTitle.") + which, values ? QCoreApplication::translate("jp::ChartActions", "Title of the Axis of Values…") : QCoreApplication::translate("jp::ChartActions", "Title of the Axis of Categories…"), "type", [this, values] { editAxisTitle(values); });
    }
    for (const char *id : {"valueMajor", "valueMinor", "categoryMajor"}) {
        const QString which = QLatin1String(id);
        const QString text = which == QLatin1String("valueMajor") ? QCoreApplication::translate("jp::ChartActions", "Major Gridlines of Values") : which == QLatin1String("valueMinor") ? QCoreApplication::translate("jp::ChartActions", "Minor Gridlines of Values") : QCoreApplication::translate("jp::ChartActions", "Major Gridlines of Categories");
        mk(QStringLiteral("chart.grid.") + which, text, "grid-2x2", [this, which, find, changeLabel] {
            const ChartInfo info = selectedChartInfo(m_ed);
            const bool on = find(QStringLiteral("chart.grid.") + which)->isChecked();
            const bool categories = which == QLatin1String("categoryMajor"), minorOne = which == QLatin1String("valueMinor");
            const bool major = categories ? on : (minorOne ? info.valueGridlines : on), minor = categories ? false : (minorOne ? on : info.valueMinorGridlines && on);
            change(changeLabel, [=](office::Chart &c) { return chartSetGridlines(c, categories ? ChartAxisKind::Category : ChartAxisKind::Value, major, minor); });
        }, true);
    }
    for (int style = 1; style <= 8; ++style)
        mk(QStringLiteral("chart.style.%1").arg(style), QCoreApplication::translate("jp::ChartActions", "Style %1").arg(style), "palette", [this, style, changeLabel] {
            change(changeLabel, [style](office::Chart &c) { return chartSetStyle(c, style); });
        }, true);
    mk(QStringLiteral("chart.dataLabels"), QCoreApplication::translate("jp::ChartActions", "Data Labels"), "text", [this, find, changeLabel] {
        const bool on = find(QStringLiteral("chart.dataLabels"))->isChecked();
        change(changeLabel, [on](office::Chart &c) { return chartSetDataLabels(c, on); });
    }, true);
}

void ChartActions::editData()
{
    office::sheet::ChartInfo info = selectedChartInfo(m_ed);
    if (!info.valid || !info.dataEditable) return;
    office::sheet::ChartData data = info.data;
    QString notice;
    if (!askChartData(m_window, &data, &notice)) {
        if (!notice.isEmpty() && m_hooks.notice) m_hooks.notice(notice);
        return;
    }
    const PictureItem *chart = selectedChart(m_ed);
    if (!chart) return;
    const office::Theme theme = chartThemeOfPicture(*m_ed->doc(), *chart);
    change(QCoreApplication::translate("jp::ChartActions", "Change Chart Data"), [data, theme](office::Chart &c) { return office::sheet::chartSetData(c, data, theme); });
}

void ChartActions::editTitle()
{
    const office::sheet::ChartInfo info = selectedChartInfo(m_ed);
    if (!info.valid) return;
    QString text = info.title;
    if (!askChartTitle(m_window, &text)) return;
    change(QCoreApplication::translate("jp::ChartActions", "Change Chart"), [text](office::Chart &c) { return office::sheet::chartSetTitle(c, true, text); });
}

void ChartActions::editAxisTitle(bool values)
{
    const office::sheet::ChartInfo info = selectedChartInfo(m_ed);
    if (!info.valid || !info.hasAxes) return;
    QString text = values ? info.valueAxisTitle : info.categoryAxisTitle;
    if (!askChartAxisTitle(m_window, values, &text)) return;
    const office::sheet::ChartAxisKind kind = values ? office::sheet::ChartAxisKind::Value : office::sheet::ChartAxisKind::Category;
    change(QCoreApplication::translate("jp::ChartActions", "Change Chart"), [text, kind](office::Chart &c) { return office::sheet::chartSetAxisTitle(c, kind, text); });
}

bool ChartActions::refresh()
{
    const bool body = m_ed && m_ed->doc() && !m_ed->readOnly();
    for (const ChartName &n : kChartNames) m_hooks.find(QStringLiteral("insert.chart.") + QLatin1String(n.id))->setEnabled(body);
    const bool isChart = m_ed && selectedChart(m_ed);
    const office::sheet::ChartInfo info = isChart ? selectedChartInfo(m_ed) : office::sheet::ChartInfo();
    const bool editable = isChart && info.valid && body;
    for (const QString &id : std::as_const(m_ids))
        if (id.startsWith(QLatin1String("chart."))) m_hooks.find(id)->setEnabled(editable);
    auto act = m_hooks.find;
    act(QStringLiteral("chart.editData"))->setEnabled(editable && info.dataEditable);
    act(QStringLiteral("chart.title"))->setChecked(editable && info.titleShown);
    act(QStringLiteral("chart.dataLabels"))->setChecked(editable && info.dataLabels);
    for (const char *id : {"axis.category", "axis.value", "axisTitle.category", "axisTitle.value", "grid.valueMajor", "grid.valueMinor", "grid.categoryMajor"})
        act(QStringLiteral("chart.") + QLatin1String(id))->setEnabled(editable && info.hasAxes);
    act(QStringLiteral("chart.axis.category"))->setChecked(editable && info.categoryAxisShown);
    act(QStringLiteral("chart.axis.value"))->setChecked(editable && info.valueAxisShown);
    act(QStringLiteral("chart.grid.valueMajor"))->setChecked(editable && info.valueGridlines);
    act(QStringLiteral("chart.grid.valueMinor"))->setChecked(editable && info.valueMinorGridlines);
    act(QStringLiteral("chart.grid.categoryMajor"))->setChecked(editable && info.categoryGridlines);
    for (int style = 1; style <= 8; ++style) act(QStringLiteral("chart.style.%1").arg(style))->setChecked(editable && info.style == style);
    for (const ChartName &n : kChartNames) act(QStringLiteral("chart.type.") + QLatin1String(n.id))->setChecked(editable && info.form == n.form);
    act(QStringLiteral("chart.legend.none"))->setChecked(editable && !info.legendShown);
    for (const LegendName &l : kLegends)
        if (l.position[0]) act(QStringLiteral("chart.legend.") + QLatin1String(l.id))->setChecked(editable && info.legendShown && info.legendPosition == QLatin1String(l.position));
    return isChart;
}

QMenu *ChartActions::insertMenu(QWidget *owner) const
{
    auto *m = new QMenu(owner);
    for (const ChartName &n : kChartNames) m->addAction(m_hooks.find(QStringLiteral("insert.chart.") + QLatin1String(n.id)));
    return m;
}

QMenu *ChartActions::typeMenu(QWidget *owner) const
{
    auto *m = new QMenu(owner);
    auto *group = new QActionGroup(m);
    for (const ChartName &n : kChartNames) {
        QAction *a = m_hooks.find(QStringLiteral("chart.type.") + QLatin1String(n.id));
        group->addAction(a);
        m->addAction(a);
    }
    return m;
}

QMenu *ChartActions::axesMenu(QWidget *owner) const
{
    auto *m = new QMenu(owner);
    m->addAction(m_hooks.find(QStringLiteral("chart.axis.category")));
    m->addAction(m_hooks.find(QStringLiteral("chart.axis.value")));
    m->addSeparator();
    m->addAction(m_hooks.find(QStringLiteral("chart.axisTitle.category")));
    m->addAction(m_hooks.find(QStringLiteral("chart.axisTitle.value")));
    return m;
}

QMenu *ChartActions::gridlinesMenu(QWidget *owner) const
{
    auto *m = new QMenu(owner);
    for (const char *id : {"valueMajor", "valueMinor", "categoryMajor"}) m->addAction(m_hooks.find(QStringLiteral("chart.grid.") + QLatin1String(id)));
    return m;
}

QMenu *ChartActions::stylesMenu(QWidget *owner) const
{
    auto *m = new QMenu(owner);
    auto *group = new QActionGroup(m);
    for (int style = 1; style <= 8; ++style) {
        QAction *a = m_hooks.find(QStringLiteral("chart.style.%1").arg(style));
        group->addAction(a);
        m->addAction(a);
    }
    return m;
}

QMenu *ChartActions::legendMenu(QWidget *owner) const
{
    auto *m = new QMenu(owner);
    auto *group = new QActionGroup(m);
    for (const LegendName &l : kLegends) {
        QAction *a = m_hooks.find(QStringLiteral("chart.legend.") + QLatin1String(l.id));
        group->addAction(a);
        m->addAction(a);
    }
    return m;
}

} // namespace jp
