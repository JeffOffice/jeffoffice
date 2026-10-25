// Making and changing charts in JeffCrunch's window: New Chart from the cells selected, the chart's buttons and menus (its elements, colors, and looks), Chart Data, Chart Kind, the
// properties and the context menu of an object, and Copy, Cut, and Paste of a chart. Each change is one step of Undo (core/objectstate.h); the chart model it changes is
// libs/office/sheet/chartedit.h's, and what it writes is libs/office/office/chart_write.h's.

#include "app/crunchwindow.h"

#include "app/chartdialogs.h"
#include "app/icons.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "core/objectclip.h"
#include "core/sheetobjects.h"
#include "office/chart_write.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QSvgRenderer>
#include <QToolButton>

#include <algorithm>

namespace jp {

using namespace jp::office;
using namespace jp::office::sheet;

namespace {

struct KindAction {
    const char *id;
    const char *text;
    const char *icon;
    ChartKind kind;
};

const KindAction kKindActions[] = {
    {"chart.column.clustered", QT_TR_NOOP("Columns"), "chart-column", ChartKind::Column},
    {"chart.column.stacked", QT_TR_NOOP("Stacked columns"), "chart-column-stacked", ChartKind::ColumnStacked},
    {"chart.column.percent", QT_TR_NOOP("Columns to 100%"), "chart-column-stacked", ChartKind::ColumnPercent},
    {"chart.bar.clustered", QT_TR_NOOP("Bars"), "chart-bar", ChartKind::Bar},
    {"chart.bar.stacked", QT_TR_NOOP("Stacked bars"), "chart-bar-stacked", ChartKind::BarStacked},
    {"chart.bar.percent", QT_TR_NOOP("Bars to 100%"), "chart-bar-stacked", ChartKind::BarPercent},
    {"chart.line.plain", QT_TR_NOOP("Line"), "chart-line", ChartKind::Line},
    {"chart.line.markers", QT_TR_NOOP("Line with markers"), "chart-spline", ChartKind::LineMarkers},
    {"chart.line.stacked", QT_TR_NOOP("Stacked line"), "chart-line", ChartKind::LineStacked},
    {"chart.pie.pie", QT_TR_NOOP("Pie"), "chart-pie", ChartKind::Pie},
    {"chart.pie.doughnut", QT_TR_NOOP("Doughnut"), "donut", ChartKind::Doughnut},
    {"chart.area.plain", QT_TR_NOOP("Area"), "chart-area", ChartKind::Area},
    {"chart.area.stacked", QT_TR_NOOP("Stacked area"), "chart-area", ChartKind::AreaStacked},
    {"chart.scatter.points", QT_TR_NOOP("Scatter, points"), "chart-scatter", ChartKind::Scatter},
    {"chart.scatter.joined", QT_TR_NOOP("Scatter, joined points"), "chart-scatter", ChartKind::ScatterLines},
};

struct GroupAction {
    const char *id;
    const char *text;
    const char *icon;
    ChartKind kind;
};

const GroupAction kGroupActions[] = {
    {"chart.column", QT_TR_NOOP("Column"), "chart-column", ChartKind::Column},        {"chart.bar", QT_TR_NOOP("Bar"), "chart-bar", ChartKind::Bar},
    {"chart.line", QT_TR_NOOP("Line"), "chart-line", ChartKind::Line},                {"chart.pie", QT_TR_NOOP("Pie"), "chart-pie", ChartKind::Pie},
    {"chart.area", QT_TR_NOOP("Area"), "chart-area", ChartKind::Area},                {"chart.scatter", QT_TR_NOOP("Scatter"), "chart-scatter", ChartKind::Scatter},
    {"chart.combo", QT_TR_NOOP("Combo"), "chart-no-axes-combined", ChartKind::Combo},
};

QImage renderChart(const Chart &chart, const QSize &pixels)
{
    if (pixels.isEmpty()) return QImage();
    const QByteArray svg = chartSvg(chart, QSizeF(pixels.width() * 0.75, pixels.height() * 0.75));
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

} // namespace

void CrunchWindow::createChartActions()
{
    // (Insert > Chart is the dialog; the others make the kind they name from the cells selected)
    mk("insert.chart", tr("More Charts…"), "chart-column", QKeySequence(), [this] { insertChartDialog(); });
    m_chartActions << QStringLiteral("insert.chart");
    for (const KindAction &k : kKindActions) {
        const ChartKind kind = k.kind;
        mk(QString::fromLatin1(k.id), QCoreApplication::translate("jp::CrunchWindow", k.text), k.icon, QKeySequence(), [this, kind] { insertChart(kind); });
        m_chartActions << QString::fromLatin1(k.id);
    }
    for (const GroupAction &g : kGroupActions) {
        const ChartKind kind = g.kind;
        mk(QString::fromLatin1(g.id), QCoreApplication::translate("jp::CrunchWindow", g.text), g.icon, QKeySequence(), [this, kind] { insertChart(kind); });
        m_chartActions << QString::fromLatin1(g.id);
    }
    for (const QString &id : std::as_const(m_chartActions)) m_opsActions << id;
    // the commands of the chart that is picked
    mk("chartdesign.selectData", tr("Chart Data…"), "table-properties", QKeySequence(), [this] { selectDataDialog(); }, false, false);
    mk("chartdesign.changeType", tr("Chart Kind…"), "chart-no-axes-combined", QKeySequence(), [this] { changeChartTypeDialog(); }, false, false);
    mk("chartdesign.switch", tr("Swap Rows and Columns"), "arrow-left-right", QKeySequence(), [this] {
        const int id = pickedObject();
        if (id < 0) return;
        bool ok = false;
        const Chart now = m_objects->chartNow(m_sheet, id);
        const Chart switched = switchedRowsColumns(now, m_objects->theme(), &ok);
        if (!ok) {
            notice(tr("Swap Rows and Columns"), tr("The series of this chart don't read one block of cells that can be turned the other way."));
            return;
        }
        changePickedChart(tr("Swap Rows and Columns"), [switched](Chart *c) { *c = switched; });
    }, false, false);
    mk("object.properties", tr("Properties…"), "sliders-horizontal", QKeySequence(), [this] { propertiesDialog(); }, false, false);
    mk("object.front", tr("Bring to Front"), "bring-to-front", QKeySequence(), [this] { reorderObject(2); }, false, false);
    mk("object.forward", tr("Bring Forward"), "arrow-up", QKeySequence(), [this] { reorderObject(1); }, false, false);
    mk("object.backward", tr("Send Backward"), "arrow-down", QKeySequence(), [this] { reorderObject(-1); }, false, false);
    mk("object.back", tr("Send to Back"), "send-to-back", QKeySequence(), [this] { reorderObject(-2); }, false, false);
    mk("object.delete", tr("Delete"), "trash-2", QKeySequence(), [this] {
        const int id = pickedObject();
        if (id >= 0) deleteObject(id);
    }, false, false);
    m_chartButtonsTimer.setSingleShot(true);
    m_chartButtonsTimer.setInterval(0);
    connect(&m_chartButtonsTimer, &QTimer::timeout, this, [this] { placeChartButtons(); });
}

void CrunchWindow::chartRibbonParts(RibbonParts &parts)
{
    parts.menus[QStringLiteral("chartElements")] = [this](QWidget *owner) { return buildChartElementsMenu(owner); };
    parts.menus[QStringLiteral("chartLayouts")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("chartLayoutsMenu"));
        for (int i = 0; i < chartLayoutCount(); ++i) {
            QAction *a = menu->addAction(chartLayoutName(i));
            connect(a, &QAction::triggered, this, [this, i] { changePickedChart(tr("Layouts"), [i](Chart *c) { applyChartLayout(c, i); }); });
        }
        return menu;
    };
    parts.menus[QStringLiteral("chartColors")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("chartColorsMenu"));
        for (int i = 0; i < chartColorSchemeCount(); ++i) {
            QAction *a = menu->addAction(chartColorSchemeName(i));
            connect(a, &QAction::triggered, this, [this, i] { changePickedChart(tr("Colors"), [i](Chart *c) { applyColorScheme(c, i); }); });
        }
        return menu;
    };
    parts.menus[QStringLiteral("chartStyles")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("chartStylesMenu"));
        for (int i = 0; i < chartStyleCount(); ++i) {
            QAction *a = menu->addAction(chartStyleName(i));
            connect(a, &QAction::triggered, this, [this, i] { changePickedChart(tr("Chart Look"), [i](Chart *c) { applyChartStyle(c, i); }); });
        }
        return menu;
    };
    parts.colors[QStringLiteral("chartDesign")] = QColor(0x13, 0x77, 0x3A);
}

int CrunchWindow::pickedObject() const
{
    if (!m_objects || m_objects->shownSheet() != m_sheet) return -1;
    const int id = m_objects->layer()->selectedId();
    return id >= 0 && !m_objects->layer()->isDeleted(id) ? id : -1;
}

// ---- New Chart

bool CrunchWindow::chartPlanFor(ChartKind kind, int inRows, ChartPlan *plan, bool forPreview)
{
    if (!m_objects || !m_objects->cells() || !m_loaded || !m_loaded->book || m_sheet < 0) return false;
    const ChartCells &cells = *m_objects->cells();
    const QString name = m_loaded->book->info().sheets[m_sheet].name;
    jp::office::sheet::CellRange selection = selectedRange();
    if (forPreview) {
        // a picture of a chart is drawn from a part of the cells: at most this many rows and columns of the block
        const jp::office::sheet::CellRange block = chartBlockOf(cells, m_sheet, selection);
        if (!block.valid) return false;
        selection = block;
        selection.row2 = std::min<quint32>(selection.row2, selection.row1 + 40);
        selection.col2 = std::min<quint32>(selection.col2, selection.col1 + 12);
    }
    *plan = planChart(cells, m_sheet, name, selection, kind);
    if (!plan->series.isEmpty() && inRows == 2) {
        // the other way from the one the shape of the cells gives
        *plan = planChart(cells, m_sheet, name, selection, kind, plan->seriesInRows ? 0 : 1);
    }
    return !plan->series.isEmpty();
}

QImage CrunchWindow::chartPreviewImage(ChartKind kind, int inRows, const QSize &pixels)
{
    ChartPlan plan;
    if (!chartPlanFor(kind, inRows, &plan, true)) return QImage();
    Chart chart = makeChart(kind, plan, m_objects->theme(), plan.points);
    if (m_objects->cells()) applyChartCells(&chart, m_sheet, *m_objects->cells());
    return renderChart(chart, pixels);
}

DrawingAnchor CrunchWindow::newObjectAnchor(double width, double height)
{
    // in the middle of the cells in view, at the corner of a cell
    const grid::GridLayout &L = m_view->sheet().layout();
    const grid::PaneView body = L.pane(grid::Pane::Body);
    const double zoom = L.zoom();
    QPoint at = body.rect.center() - QPoint(int(width * zoom / 2), int(height * zoom / 2));
    at.setX(std::max(at.x(), body.rect.left() + 8));
    at.setY(std::max(at.y(), body.rect.top() + 8));
    const grid::Hit h = L.hit(at);
    const qint64 row = h.kind == grid::Hit::Kind::Cell ? h.row : std::max<qint64>(0, L.pane(grid::Pane::Body).firstRow);
    const qint64 col = h.kind == grid::Hit::Kind::Cell ? h.col : std::max<qint64>(0, L.pane(grid::Pane::Body).firstCol);
    jp::grid::ObjectBox box;
    box.x = double(L.cols.position(col));
    box.y = double(L.rows.position(row));
    box.w = width;
    box.h = height;
    DrawingAnchor a;
    m_objects->layer()->attach(&L);
    if (!m_objects->layer()->anchorAtBox(box, AnchorKind::TwoCell, &a)) a.to = AnchorPoint{quint32(col + 8), quint32(row + 15), 0, 0};
    return a;
}

bool CrunchWindow::insertChart(ChartKind kind, int inRows)
{
    if (!m_session || !m_objects || !m_loaded) return false;
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return false;
    if (m_chartSheetShown) return false;
    QString why;
    ChartPlan plan;
    if (!m_objects->cells()) {
        notice(tr("New Chart"), tr("The workbook is still being read. A chart can be made when it is open."));
        return false;
    }
    if (!chartPlanFor(kind, inRows, &plan)) {
        notice(tr("New Chart"), tr("Select the cells that hold the numbers to chart. If the first row has names and the first column has labels, select those too."), false);
        return false;
    }
    const Chart chart = makeChart(kind, plan, m_objects->theme(), plan.points);
    const DrawingAnchor anchor = newObjectAnchor(480, 288);
    const int id = m_objects->makeChart(m_sheet, chart, anchor, &why);
    if (id < 0) {
        notice(tr("New Chart"), why);
        return false;
    }
    crunch::ObjectEdit e;
    e.sheet = m_sheet;
    e.id = id;
    e.before = m_objects->stateOf(m_sheet, id);
    e.after = e.before;
    e.after.deleted = false;
    const crunch::EditSession::Outcome o = m_session->changeObjects({e}, tr("New Chart"));
    if (!o.ok) {
        m_objects->discardMade(m_sheet, id);
        notice(tr("New Chart"), o.problem);
        return false;
    }
    m_objects->layer()->select(id);
    m_view->setFocus(Qt::OtherFocusReason);
    setStatus(plan.truncated ? tr("The chart reads the first part of the cells you selected: a chart holds at most %1 points.").arg(QLocale().toString(kMaxChartCells)) : tr("Chart inserted"));
    return true;
}

QDialog *CrunchWindow::makeInsertChartDialog()
{
    if (!m_objects || !m_objects->cells()) return nullptr;
    return new InsertChartDialog([this](ChartKind k, int inRows, const QSize &px) { return chartPreviewImage(k, inRows, px); }, ChartKind::Column, this);
}

void CrunchWindow::insertChartDialog()
{
    if (!m_session || !m_objects || m_chartSheetShown) return;
    InsertChartChoice choice;
    if (m_insertChartAsker) {
        if (!m_insertChartAsker(&choice)) return;
    } else {
        std::unique_ptr<QDialog> dialog(makeInsertChartDialog());
        if (!dialog) {
            notice(tr("New Chart"), tr("The workbook is still being read. A chart can be made when it is open."));
            return;
        }
        if (dialog->exec() != QDialog::Accepted) return;
        const auto *d = static_cast<InsertChartDialog *>(dialog.get());
        choice.kind = d->kind();
        choice.inRows = d->inRows();
    }
    insertChart(choice.kind, choice.inRows);
}

// ---- moving, sizing, and changing

bool CrunchWindow::moveObject(int id, const DrawingAnchor &anchor)
{
    if (!m_session || !m_objects || m_edit.active || m_saving || m_chartSheetShown) return false;
    if (!m_objects->exists(m_sheet, id)) return false;
    crunch::ObjectEdit e;
    e.sheet = m_sheet;
    e.id = id;
    e.before = m_objects->stateOf(m_sheet, id);
    e.after = m_objects->stateMoved(m_sheet, id, anchor);
    const crunch::EditSession::Outcome o = m_session->changeObjects({e}, tr("Move or Size Object"));
    if (!o.ok) {
        setStatus(o.problem);
        return false;
    }
    return true;
}

bool CrunchWindow::changePickedChart(const QString &text, const std::function<void(Chart *)> &change)
{
    const int id = pickedObject();
    if (id < 0 || !m_session || !m_objects || m_edit.active || m_chartSheetShown) return false;
    QString why;
    if (!m_objects->canEditChart(m_sheet, id, &why)) {
        notice(text, why);
        return false;
    }
    Chart chart = m_objects->chartNow(m_sheet, id);
    change(&chart);
    crunch::ObjectEdit e;
    e.sheet = m_sheet;
    e.id = id;
    e.before = m_objects->stateOf(m_sheet, id);
    e.after = m_objects->stateWithChart(m_sheet, id, chart);
    const crunch::EditSession::Outcome o = m_session->changeObjects({e}, text);
    if (!o.ok) {
        notice(text, o.problem);
        return false;
    }
    updateChartState();
    return true;
}

void CrunchWindow::propertiesDialog()
{
    const int id = pickedObject();
    if (id < 0 || !m_session || !m_objects) return;
    const grid::ObjectBox box = m_objects->layer()->boxNow(id);
    const DrawingAnchor now = m_objects->anchorOf(m_sheet, id);
    ObjectProperties p;
    p.widthPoints = box.w * 0.75;
    p.heightPoints = box.h * 0.75;
    p.follows = now.editAs;
    p.name = m_objects->nameOf(m_sheet, id);
    p.keepProportions = m_objects->kindOf(m_sheet, id) == ObjectKind::Picture;
    ObjectProperties out = p;
    if (m_propertiesAsker) {
        if (!m_propertiesAsker(p, &out)) return;
    } else {
        ObjectPropertiesDialog d(p, this);
        if (d.exec() != QDialog::Accepted) return;
        out = d.result();
    }
    grid::ObjectBox nb = box;
    nb.w = out.widthPoints / 0.75;
    nb.h = out.heightPoints / 0.75;
    DrawingAnchor a;
    if (!m_objects->layer()->anchorAtBox(nb, out.follows, &a)) return;
    moveObject(id, a);
}

// ---- Chart Data and Chart Kind

SelectDataRequest CrunchWindow::selectDataRequest(int id)
{
    SelectDataRequest request;
    request.chart = m_objects->chartNow(m_sheet, id);
    request.theme = m_objects->theme();
    const int sheet = m_sheet;
    request.checkRef = [this, sheet](const QString &ref, QString *problem) {
        QVector<ChartBlock> blocks;
        if (m_objects && m_objects->cells() && chartBlocks(ref, sheet, *m_objects->cells(), &blocks)) return true;
        if (problem) *problem = tr("\"%1\" isn't cells JeffCrunch can find. Write them like Sheet1!$B$2:$B$9.").arg(ref);
        return false;
    };
    request.planFor = [this, sheet](const QString &range, int inRows) {
        ChartPlan none;
        if (!m_objects || !m_objects->cells() || !m_loaded || !m_loaded->book) return none;
        // "Sheet1!$A$1:$D$8", or the cells of this sheet
        QString name = m_loaded->book->info().sheets[sheet].name;
        QString cells = range;
        const int bang = int(range.lastIndexOf(QLatin1Char('!')));
        if (bang > 0) {
            name = range.left(bang);
            if (name.size() >= 2 && name.startsWith(QLatin1Char('\'')) && name.endsWith(QLatin1Char('\''))) name = name.mid(1, name.size() - 2).replace(QStringLiteral("''"), QStringLiteral("'"));
            cells = range.mid(bang + 1);
        }
        const int other = m_objects->cells()->sheetIndex(name);
        jp::office::sheet::CellRange r;
        if (other < 0 || !parseRangeRef(cells, &r)) return none;
        const ChartKind kind = ChartKind::Column;
        return planChart(*m_objects->cells(), other, name, r, kind, inRows);
    };
    request.switcher = [this](const Chart &now, Chart *out) {
        bool ok = false;
        const Chart c = switchedRowsColumns(now, m_objects->theme(), &ok);
        if (ok) *out = c;
        return ok;
    };
    request.nameOf = [this](const ChartSeries &s) {
        if (!m_objects || !m_objects->cells() || s.nameRef.isEmpty()) return s.name;
        ChartRefData data;
        if (readChartRef(s.nameRef, m_sheet, *m_objects->cells(), 1, &data) && !data.points.isEmpty() && !data.points[0].text.isEmpty()) return data.points[0].text;
        return s.name;
    };
    return request;
}

QDialog *CrunchWindow::makeSelectDataDialog()
{
    const int id = pickedObject();
    if (id < 0 || !m_session || !m_objects || !m_objects->canEditChart(m_sheet, id)) return nullptr;
    return new SelectDataDialog(selectDataRequest(id), this);
}

void CrunchWindow::selectDataDialog()
{
    const int id = pickedObject();
    if (id < 0 || !m_session || !m_objects) return;
    QString why;
    if (!m_objects->canEditChart(m_sheet, id, &why)) {
        notice(tr("Chart Data"), why);
        return;
    }
    Chart result;
    if (m_selectDataAsker) {
        if (!m_selectDataAsker(selectDataRequest(id), &result)) return;
    } else {
        SelectDataDialog d(selectDataRequest(id), this);
        if (d.exec() != QDialog::Accepted) return;
        result = d.result();
    }
    changePickedChart(tr("Chart Data"), [result](Chart *c) { *c = result; });
}

QDialog *CrunchWindow::makeChangeTypeDialog()
{
    const int id = pickedObject();
    if (id < 0 || !m_session || !m_objects || !m_objects->canEditChart(m_sheet, id)) return nullptr;
    const Chart chart = m_objects->chartNow(m_sheet, id);
    QStringList names;
    for (const ChartSeries &s : seriesOf(chart)) names << s.name;
    const Theme theme = m_objects->theme();
    return new ChangeChartTypeDialog(names, seriesKindsOf(chart), [chart, theme](const QVector<SeriesKind> &kinds, const QSize &px) { return renderChart(withSeriesKinds(chart, kinds, theme), px); }, this);
}

void CrunchWindow::changeChartTypeDialog()
{
    const int id = pickedObject();
    if (id < 0 || !m_session || !m_objects) return;
    QString why;
    if (!m_objects->canEditChart(m_sheet, id, &why)) {
        notice(tr("Chart Kind"), why);
        return;
    }
    const Chart chart = m_objects->chartNow(m_sheet, id);
    QStringList names;
    for (const ChartSeries &s : seriesOf(chart)) names << s.name;
    const QVector<SeriesKind> now = seriesKindsOf(chart);
    QVector<SeriesKind> chosen = now;
    if (m_changeTypeAsker) {
        if (!m_changeTypeAsker(names, now, &chosen)) return;
    } else {
        std::unique_ptr<QDialog> dialog(makeChangeTypeDialog());
        if (!dialog || dialog->exec() != QDialog::Accepted) return;
        chosen = static_cast<ChangeChartTypeDialog *>(dialog.get())->kinds();
    }
    const Theme theme = m_objects->theme();
    changePickedChart(tr("Chart Kind"), [chosen, theme](Chart *c) { *c = withSeriesKinds(*c, chosen, theme); });
}

// ---- the chart's menus

QMenu *CrunchWindow::buildChartElementsMenu(QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("chartElementsMenu"));
    connect(menu, &QMenu::aboutToShow, this, [this, menu] {
        menu->clear();
        const int id = pickedObject();
        if (id < 0 || !m_objects) return;
        const Chart chart = m_objects->chartNow(m_sheet, id);
        const ChartElements e = elementsOf(chart);
        auto check = [&](QMenu *into, const QString &text, bool on, const std::function<void(Chart *)> &change, const QString &step) {
            QAction *a = into->addAction(text);
            a->setCheckable(true);
            a->setChecked(on);
            connect(a, &QAction::triggered, this, [this, change, step] { changePickedChart(step, change); });
            return a;
        };
        auto choice = [&](QMenu *into, QActionGroup *group, const QString &text, bool on, const std::function<void(Chart *)> &change, const QString &step) {
            QAction *a = check(into, text, on, change, step);
            group->addAction(a);
            return a;
        };
        if (e.hasHorizontal || e.hasVertical) {
            QMenu *axes = menu->addMenu(tr("Axes"));
            axes->setObjectName(QStringLiteral("axesMenu"));
            if (e.hasHorizontal) check(axes, tr("Horizontal"), e.axisHorizontal, [on = !e.axisHorizontal](Chart *c) { setAxisShown(c, true, on); }, tr("Axes"));
            if (e.hasVertical) check(axes, tr("Vertical"), e.axisVertical, [on = !e.axisVertical](Chart *c) { setAxisShown(c, false, on); }, tr("Axes"));
            QMenu *titles = menu->addMenu(tr("Axis Titles"));
            titles->setObjectName(QStringLiteral("axisTitlesMenu"));
            if (e.hasHorizontal) check(titles, tr("Horizontal"), e.axisTitleHorizontal, [on = !e.axisTitleHorizontal](Chart *c) { setAxisTitle(c, true, on); }, tr("Axis Titles"));
            if (e.hasVertical) check(titles, tr("Vertical"), e.axisTitleVertical, [on = !e.axisTitleVertical](Chart *c) { setAxisTitle(c, false, on); }, tr("Axis Titles"));
        }
        QMenu *title = menu->addMenu(tr("Chart Title"));
        title->setObjectName(QStringLiteral("chartTitleMenu"));
        auto *titleGroup = new QActionGroup(title);
        choice(title, titleGroup, tr("None"), !e.title, [](Chart *c) { setChartTitle(c, TitleMode::None); }, tr("Chart Title"));
        choice(title, titleGroup, tr("Above"), e.title && !e.titleOverlay, [](Chart *c) { setChartTitle(c, TitleMode::Above); }, tr("Chart Title"));
        choice(title, titleGroup, tr("On Top"), e.titleOverlay, [](Chart *c) { setChartTitle(c, TitleMode::Overlay); }, tr("Chart Title"));
        QMenu *labels = menu->addMenu(tr("Data Labels"));
        labels->setObjectName(QStringLiteral("dataLabelsMenu"));
        auto *labelGroup = new QActionGroup(labels);
        choice(labels, labelGroup, tr("None"), !e.labels, [](Chart *c) { setDataLabels(c, false); }, tr("Data Labels"));
        choice(labels, labelGroup, tr("Usual place"), e.labels && e.labelPosition.isEmpty(), [](Chart *c) { setDataLabels(c, true); }, tr("Data Labels"));
        static const struct { const char *code; const char *name; } places[] = {{"ctr", QT_TR_NOOP("Center")}, {"inEnd", QT_TR_NOOP("Inside End")}, {"inBase", QT_TR_NOOP("Inside Base")},
                                                                                  {"outEnd", QT_TR_NOOP("Outside End")}, {"bestFit", QT_TR_NOOP("Best Fit")}, {"l", QT_TR_NOOP("Left")},
                                                                                  {"r", QT_TR_NOOP("Right")}, {"t", QT_TR_NOOP("Above")}, {"b", QT_TR_NOOP("Below")}};
        const QStringList allowed = labelPositionsFor(chart);
        for (const auto &p : places) {
            const QString code = QString::fromLatin1(p.code);
            if (!allowed.contains(code)) continue;
            choice(labels, labelGroup, QCoreApplication::translate("jp::CrunchWindow", p.name), e.labels && e.labelPosition == code, [code](Chart *c) { setDataLabels(c, true, code); }, tr("Data Labels"));
        }
        if (e.hasHorizontal || e.hasVertical) {
            QMenu *grid = menu->addMenu(tr("Gridlines"));
            grid->setObjectName(QStringLiteral("gridlinesMenu"));
            check(grid, tr("Lines Across, Major"), e.gridHorizontal, [on = !e.gridHorizontal](Chart *c) { setGridlines(c, true, false, on); }, tr("Gridlines"));
            check(grid, tr("Lines Up and Down, Major"), e.gridVertical, [on = !e.gridVertical](Chart *c) { setGridlines(c, false, false, on); }, tr("Gridlines"));
            check(grid, tr("Lines Across, Minor"), e.minorGridHorizontal, [on = !e.minorGridHorizontal](Chart *c) { setGridlines(c, true, true, on); }, tr("Gridlines"));
            check(grid, tr("Lines Up and Down, Minor"), e.minorGridVertical, [on = !e.minorGridVertical](Chart *c) { setGridlines(c, false, true, on); }, tr("Gridlines"));
        }
        QMenu *legend = menu->addMenu(tr("Legend"));
        legend->setObjectName(QStringLiteral("legendMenu"));
        auto *legendGroup = new QActionGroup(legend);
        choice(legend, legendGroup, tr("None"), !e.legend, [](Chart *c) { setLegend(c, QString()); }, tr("Legend"));
        const struct { const char *code; const char *name; } sides[] = {{"r", QT_TR_NOOP("Right")}, {"t", QT_TR_NOOP("Top")}, {"l", QT_TR_NOOP("Left")}, {"b", QT_TR_NOOP("Bottom")}};
        for (const auto &s : sides) {
            const QString code = QString::fromLatin1(s.code);
            choice(legend, legendGroup, QCoreApplication::translate("jp::CrunchWindow", s.name), e.legend && e.legendPosition == code, [code](Chart *c) { setLegend(c, code); }, tr("Legend"));
        }
    });
    return menu;
}

QMenu *CrunchWindow::buildChartLooksMenu(QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("chartLooksMenu"));
    menu->addSection(tr("Colors"));
    for (int i = 0; i < chartColorSchemeCount(); ++i) {
        QAction *a = menu->addAction(chartColorSchemeName(i));
        connect(a, &QAction::triggered, this, [this, i] { changePickedChart(tr("Colors"), [i](Chart *c) { applyColorScheme(c, i); }); });
    }
    menu->addSection(tr("Looks"));
    for (int i = 0; i < chartStyleCount(); ++i) {
        QAction *a = menu->addAction(chartStyleName(i));
        connect(a, &QAction::triggered, this, [this, i] { changePickedChart(tr("Chart Look"), [i](Chart *c) { applyChartStyle(c, i); }); });
    }
    return menu;
}

// ---- the picked object: its commands, buttons, and menu

void CrunchWindow::updateChartState()
{
    const int id = pickedObject();
    const bool picked = id >= 0 && m_session && !m_saving;
    const bool chart = picked && m_objects->kindOf(m_sheet, id) == ObjectKind::Chart;
    bool editable = false;
    if (chart) editable = m_objects->canEditChart(m_sheet, id);
    for (const char *name : {"chartdesign.selectData", "chartdesign.changeType", "chartdesign.switch"}) act(QString::fromLatin1(name))->setEnabled(editable);
    for (const char *name : {"object.properties", "object.front", "object.forward", "object.backward", "object.back", "object.delete"}) act(QString::fromLatin1(name))->setEnabled(picked);
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Chart"), chart);
    m_chartButtonsTimer.start();
}

void CrunchWindow::placeChartButtons()
{
    const int id = pickedObject();
    bool show = false;
    if (id >= 0 && m_objects->kindOf(m_sheet, id) == ObjectKind::Chart && m_session && !m_saving && !m_objects->layer()->dragging() && m_objects->canEditChart(m_sheet, id)) {
        const QRect r = m_objects->layer()->deviceRect(id, grid::Pane::Body);
        if (!r.isEmpty()) {
            if (!m_chartPlus) {
                m_chartPlus = new QToolButton(m_view->viewport());
                m_chartPlus->setObjectName(QStringLiteral("chartElementsButton"));
                m_chartPlus->setIcon(icon(QStringLiteral("plus")));
                m_chartPlus->setToolTip(tr("Chart Elements"));
                m_chartPlus->setAccessibleName(tr("Chart Elements"));
                m_chartPlus->setPopupMode(QToolButton::InstantPopup);
                m_chartPlus->setAutoRaise(false);
                m_chartPlus->setFocusPolicy(Qt::NoFocus);
                m_chartPlus->setMenu(buildChartElementsMenu(m_chartPlus));
                m_chartBrush = new QToolButton(m_view->viewport());
                m_chartBrush->setObjectName(QStringLiteral("chartLooksButton"));
                m_chartBrush->setIcon(icon(QStringLiteral("brush")));
                m_chartBrush->setToolTip(tr("Chart Colors and Styles"));
                m_chartBrush->setAccessibleName(tr("Chart Colors and Styles"));
                m_chartBrush->setPopupMode(QToolButton::InstantPopup);
                m_chartBrush->setFocusPolicy(Qt::NoFocus);
                m_chartBrush->setMenu(buildChartLooksMenu(m_chartBrush));
            }
            const int size = 26;
            const QRect vp = m_view->viewport()->rect();
            int x = r.right() + 6;
            if (x + size > vp.right()) x = r.right() - size - 4;   // (no room beside it: inside its corner)
            x = std::clamp(x, 0, std::max(0, vp.right() - size));
            const int y = std::clamp(r.top(), 0, std::max(0, vp.bottom() - 2 * size - 4));
            for (QToolButton *b : {m_chartPlus, m_chartBrush}) {
                b->setFixedSize(size, size);
                b->setIconSize(QSize(16, 16));
            }
            m_chartPlus->move(x, y);
            m_chartBrush->move(x, y + size + 4);
            show = true;
        }
    }
    if (m_chartPlus) {
        m_chartPlus->setVisible(show);
        m_chartBrush->setVisible(show);
        if (show) {
            m_chartPlus->raise();
            m_chartBrush->raise();
        }
    }
}

void CrunchWindow::objectContextMenu(int id, const QPoint &global)
{
    QMenu *menu = buildObjectMenu(id, this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->popup(global);
}

QMenu *CrunchWindow::buildObjectMenu(int id, QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("objectMenu"));
    const bool chart = m_objects && m_objects->kindOf(m_sheet, id) == ObjectKind::Chart;
    const bool editable = chart && m_objects->canEditChart(m_sheet, id);
    menu->addAction(act(QStringLiteral("edit.cut")));
    QAction *copy = menu->addAction(icon(QStringLiteral("copy")), tr("Copy"));
    connect(copy, &QAction::triggered, this, [this] { copyObject(false); });
    copy->setEnabled(chart);
    menu->addAction(act(QStringLiteral("edit.paste")));
    menu->addAction(act(QStringLiteral("object.delete")));
    menu->addSeparator();
    QMenu *order = menu->addMenu(tr("Order"));
    for (const char *name : {"object.front", "object.forward", "object.backward", "object.back"}) order->addAction(act(QString::fromLatin1(name)));
    if (chart) {
        menu->addSeparator();
        menu->addAction(act(QStringLiteral("chartdesign.selectData")));
        menu->addAction(act(QStringLiteral("chartdesign.changeType")));
        if (editable) {
            QMenu *elements = buildChartElementsMenu(menu);
            elements->setTitle(tr("Chart Parts"));
            menu->addMenu(elements);
        }
    }
    menu->addSeparator();
    menu->addAction(act(QStringLiteral("object.properties")));
    return menu;
}

// ---- Copy, Cut, and Paste of a chart

bool CrunchWindow::copyObject(bool cut)
{
    const int id = pickedObject();
    if (id < 0 || !m_session || !m_objects) return false;
    if (m_objects->kindOf(m_sheet, id) != ObjectKind::Chart) {
        setStatus(tr("Only a chart can be copied or cut. A picture or shape can be moved, sized, put in front or behind, and deleted."));
        return false;
    }
    if (cut && !editingAllowed()) return false;
    const Chart chart = m_objects->chartNow(m_sheet, id);
    crunch::ObjectClip clip;
    clip.kind = QStringLiteral("chart");
    clip.origin = QString::number(m_session->token());
    clip.sheetName = m_loaded && m_loaded->book ? m_loaded->book->info().sheets[m_sheet].name : QString();
    const grid::ObjectBox box = m_objects->layer()->boxNow(id);
    clip.width = std::max(4.0, box.w);
    clip.height = std::max(4.0, box.h);
    clip.chartXml = writeChartPart(chart);
    auto *md = new QMimeData;
    md->setData(QLatin1String(crunch::kObjectMime), clip.toBytes());
    md->setImageData(renderChart(chart, QSize(int(clip.width), int(clip.height))));
    m_settingClipboard = true;
    QApplication::clipboard()->setMimeData(md);
    m_settingClipboard = false;
    m_clip = crunch::ClipData();
    m_view->clearCopyBorder();
    if (cut) deleteObject(id);
    setStatus(cut ? tr("Chart cut") : tr("Chart copied"));
    return true;
}

bool CrunchWindow::pasteObject()
{
    const QMimeData *md = QApplication::clipboard()->mimeData();
    if (!md || !md->hasFormat(QLatin1String(crunch::kObjectMime))) return false;
    crunch::ObjectClip clip;
    if (!crunch::ObjectClip::fromBytes(md->data(QLatin1String(crunch::kObjectMime)), &clip)) return false;
    if (!m_session || !m_objects || m_chartSheetShown) return true;
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return true;
    Chart chart;
    const bool sameBook = clip.origin == QString::number(m_session->token());
    if (!clip.chart(m_objects->theme(), sameBook, &chart)) {
        notice(tr("Paste"), tr("The chart on the clipboard can't be read."));
        return true;
    }
    // its sheet's name: a copy made in this workbook reads the cells it read, on the sheet it read them
    const grid::Selection &sel = m_view->selection();
    const grid::GridLayout &L = m_view->sheet().layout();
    grid::ObjectBox box;
    box.x = double(L.cols.position(std::max<qint64>(0, sel.activeCol)));
    box.y = double(L.rows.position(std::max<qint64>(0, sel.activeRow)));
    box.w = clip.width;
    box.h = clip.height;
    DrawingAnchor anchor;
    m_objects->layer()->attach(&L);
    if (!m_objects->layer()->anchorAtBox(box, AnchorKind::TwoCell, &anchor)) return true;
    QString why;
    const int id = m_objects->makeChart(m_sheet, chart, anchor, &why);
    if (id < 0) {
        notice(tr("Paste"), why);
        return true;
    }
    crunch::ObjectEdit e;
    e.sheet = m_sheet;
    e.id = id;
    e.before = m_objects->stateOf(m_sheet, id);
    e.after = e.before;
    e.after.deleted = false;
    const crunch::EditSession::Outcome o = m_session->changeObjects({e}, tr("Paste Chart"));
    if (!o.ok) {
        m_objects->discardMade(m_sheet, id);
        notice(tr("Paste"), o.problem);
        return true;
    }
    m_objects->layer()->select(id);
    setStatus(sameBook ? tr("Chart pasted") : tr("Chart pasted with the numbers it showed: it doesn't read this workbook's cells."));
    return true;
}

// ---- the order objects are drawn in

bool CrunchWindow::reorderObject(int how)
{
    const int id = pickedObject();
    if (id < 0 || !m_session || !m_objects || m_edit.active || m_chartSheetShown) return false;
    QString why;
    if (!m_objects->canReorder(m_sheet, id, &why)) {
        notice(tr("Order"), why);
        return false;
    }
    const int to = m_objects->zTarget(m_sheet, id, how);
    if (to < 0) return false;   // (it is where that puts it)
    crunch::ObjectEdit e;
    e.sheet = m_sheet;
    e.id = id;
    e.before = m_objects->stateOf(m_sheet, id);
    e.after = e.before;
    e.after.order = to;
    const QString text = how > 0 ? (how == 2 ? tr("Bring to Front") : tr("Bring Forward")) : (how == -2 ? tr("Send to Back") : tr("Send Backward"));
    const crunch::EditSession::Outcome o = m_session->changeObjects({e}, text);
    if (!o.ok) {
        setStatus(o.problem);
        return false;
    }
    return true;
}

} // namespace jp
