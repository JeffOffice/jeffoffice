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
#include "sheet/cellref.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <cmath>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QRegularExpression>
#include <QSvgRenderer>
#include <QToolButton>

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
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
    {"chart.column.clustered", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Columns"), "chart-column", ChartKind::Column},
    {"chart.column.stacked", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Stacked columns"), "chart-column-stacked", ChartKind::ColumnStacked},
    {"chart.column.percent", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Columns to 100%"), "chart-column-stacked", ChartKind::ColumnPercent},
    {"chart.bar.clustered", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Bars"), "chart-bar", ChartKind::Bar},
    {"chart.bar.stacked", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Stacked bars"), "chart-bar-stacked", ChartKind::BarStacked},
    {"chart.bar.percent", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Bars to 100%"), "chart-bar-stacked", ChartKind::BarPercent},
    {"chart.line.plain", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Line"), "chart-line", ChartKind::Line},
    {"chart.line.markers", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Line with markers"), "chart-spline", ChartKind::LineMarkers},
    {"chart.line.stacked", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Stacked line"), "chart-line", ChartKind::LineStacked},
    {"chart.pie.pie", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Pie"), "chart-pie", ChartKind::Pie},
    {"chart.pie.doughnut", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Doughnut"), "donut", ChartKind::Doughnut},
    {"chart.area.plain", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Area"), "chart-area", ChartKind::Area},
    {"chart.area.stacked", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Stacked area"), "chart-area", ChartKind::AreaStacked},
    {"chart.scatter.points", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Scatter, points"), "chart-scatter", ChartKind::Scatter},
    {"chart.scatter.joined", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Scatter, joined points"), "chart-scatter", ChartKind::ScatterLines},
    {"chart.scatter.smooth", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Scatter, smooth lines"), "chart-scatter", ChartKind::ScatterSmooth},
};

struct GroupAction {
    const char *id;
    const char *text;
    const char *icon;
    ChartKind kind;
};

const GroupAction kGroupActions[] = {
    {"chart.column", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Column"), "chart-column", ChartKind::Column},        {"chart.bar", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Bar"), "chart-bar", ChartKind::Bar},
    {"chart.line", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Line"), "chart-line", ChartKind::Line},                {"chart.pie", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Pie"), "chart-pie", ChartKind::Pie},
    {"chart.area", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Area"), "chart-area", ChartKind::Area},                {"chart.scatter", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Scatter"), "chart-scatter", ChartKind::Scatter},
    {"chart.combo", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Combo"), "chart-no-axes-combined", ChartKind::Combo},
};

QImage renderChart(const Chart &chart, const QSize &pixels)
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

} // namespace

void CrunchWindow::createChartActions()
{
    // (Insert > Chart is the dialog; the others make the kind they name from the cells selected)
    mk("insert.chart", tr("More Charts…"), "chart-column", QKeySequence(), [this] { insertChartDialog(); });
    m_chartActions << QStringLiteral("insert.chart");
    mk("insert.recommendedCharts", tr("Recommended Charts…"), "chart-no-axes-combined", QKeySequence(), [this] { recommendedChartsDialog(); });
    m_chartActions << QStringLiteral("insert.recommendedCharts");
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
    mk("chartdesign.axisH", tr("Format Horizontal Axis…"), "ruler", QKeySequence(), [this] { formatAxisDialog(true); }, false, false);
    mk("chartdesign.axisV", tr("Format Vertical Axis…"), "ruler", QKeySequence(), [this] { formatAxisDialog(false); }, false, false);
    mk("chartdesign.moveChart", tr("Move Chart…"), "move", QKeySequence(), [this] { moveChartDialog(); }, false, false);
    mk("chartdesign.saveAsPicture", tr("Save as Picture…"), "image-down", QKeySequence(), [this] { saveChartAsPicture(); }, false, false);
    mk("chartdesign.saveTemplate", tr("Save as Template…"), "file-down", QKeySequence(), [this] { saveChartTemplate(); }, false, false);
    mk("chartdesign.applyTemplate", tr("Apply Template…"), "file-up", QKeySequence(), [this] { applyChartTemplate(); }, false, false);
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
        // a picture of a chart is drawn from a part of the cells: at most this many rows and columns of the block, found once (before anything is looked at, the selection is cut to them)
        if (!m_previewBlock.valid) m_previewBlock = chartBlockOf(cells, m_sheet, selection, nullptr, 41, 13);
        if (!m_previewBlock.valid) return false;
        selection = m_previewBlock;
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
    m_previewBlock = jp::office::sheet::CellRange();   // (the selection may have changed: found again, once, for this dialog)
    return new InsertChartDialog([this](ChartKind k, int inRows, const QSize &px) { return chartPreviewImage(k, inRows, px); }, ChartKind::Column, this);
}

QDialog *CrunchWindow::makeRecommendedChartsDialog()
{
    ChartPlan plan;
    if (!m_objects || !m_objects->cells() || !chartPlanFor(ChartKind::Column, -1, &plan)) return nullptr;
    const QVector<ChartKind> kinds = recommendChartKinds(*m_objects->cells(), m_sheet, plan);
    if (kinds.isEmpty()) return nullptr;
    m_previewBlock = jp::office::sheet::CellRange();
    return new InsertChartDialog([this](ChartKind k, int inRows, const QSize &px) { return chartPreviewImage(k, inRows, px); }, kinds.first(), this, kinds, tr("Recommended Charts"));
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
        const bool accepted = dialog->exec() == QDialog::Accepted;
        m_previewBlock = jp::office::sheet::CellRange();
        if (!accepted) return;
        const auto *d = static_cast<InsertChartDialog *>(dialog.get());
        choice.kind = d->kind();
        choice.inRows = d->inRows();
    }
    insertChart(choice.kind, choice.inRows);
}

void CrunchWindow::recommendedChartsDialog()
{
    if (!m_session || !m_objects || m_chartSheetShown) return;
    const QString title = tr("Recommended Charts");
    ChartPlan plan;
    QVector<ChartKind> kinds;
    if (m_objects->cells() && chartPlanFor(ChartKind::Column, -1, &plan)) kinds = recommendChartKinds(*m_objects->cells(), m_sheet, plan);
    if (kinds.isEmpty()) {
        notice(title, m_objects->cells() ? tr("Select the cells that hold the numbers to chart. If the first row has names and the first column has labels, select those too.") : tr("The workbook is still being read. A chart can be made when it is open."), false);
        return;
    }
    InsertChartChoice choice;
    choice.kind = kinds.first();
    if (m_recommendedAsker) {
        if (!m_recommendedAsker(kinds, &choice)) return;
    } else {
        m_previewBlock = jp::office::sheet::CellRange();
        InsertChartDialog dialog([this](ChartKind k, int inRows, const QSize &px) { return chartPreviewImage(k, inRows, px); }, kinds.first(), this, kinds, title);
        const bool accepted = dialog.exec() == QDialog::Accepted;
        m_previewBlock = jp::office::sheet::CellRange();
        if (!accepted) return;
        choice.kind = dialog.kind();
        choice.inRows = dialog.inRows();
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
    if (!m_objects->chartRoom(m_sheet, chart, id, &why)) {
        notice(text, why);
        return false;
    }
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

// Save as Picture: the picked chart as it looks, twice the size it has on the sheet, as a .png or .jpg file.
void CrunchWindow::saveChartAsPicture()
{
    const int id = pickedObject();
    if (id < 0 || !m_session || !m_objects || m_chartSheetShown || m_objects->kindOf(m_sheet, id) != ObjectKind::Chart) return;
    const grid::ObjectBox box = m_objects->layer()->boxNow(id);
    const QSize pixels(std::clamp(int(box.w * 2), 40, 8000), std::clamp(int(box.h * 2), 40, 8000));
    Chart chart = m_objects->chartNow(m_sheet, id);
    if (m_objects->cells()) applyChartCells(&chart, m_sheet, *m_objects->cells());
    const QImage image = renderChart(chart, pixels);
    if (image.isNull()) {
        notice(tr("Save as Picture"), tr("This chart can't be drawn as a picture."));
        return;
    }
    const QString start = m_objects->nameOf(m_sheet, id) + QStringLiteral(".png");
    const QString chosen = askForSavePath(tr("Save as Picture"), start, tr("PNG (*.png)") + QStringLiteral(";;") + tr("JPEG (*.jpg)"));
    if (chosen.isEmpty()) return;
    QString path = chosen;
    // (a name that does not end as a picture does gets .png, so a picture never lands on a workbook's own name)
    const QString ending = QFileInfo(path).suffix().toLower();
    if (ending != QLatin1String("png") && ending != QLatin1String("jpg") && ending != QLatin1String("jpeg")) path += QStringLiteral(".png");
    if (!image.save(path)) notice(tr("Save as Picture"), tr("The picture could not be saved to \"%1\".").arg(QDir::toNativeSeparators(path)));
    else setStatus(tr("Saved the chart as a picture."));
}

// Save as Template: the picked chart's look, in a .crtx file that any chart can be given (Apply Template).
void CrunchWindow::saveChartTemplate()
{
    const int id = pickedObject();
    if (id < 0 || !m_session || !m_objects || m_chartSheetShown || m_objects->kindOf(m_sheet, id) != ObjectKind::Chart) return;
    const QString chosen = askForSavePath(tr("Save as Template"), m_objects->nameOf(m_sheet, id) + QStringLiteral(".crtx"), tr("Chart templates (*.crtx)"));
    if (chosen.isEmpty()) return;
    QString path = chosen;
    if (QFileInfo(path).suffix().compare(QLatin1String("crtx"), Qt::CaseInsensitive) != 0) path += QStringLiteral(".crtx");
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(chartTemplateFile(m_objects->chartNow(m_sheet, id))) < 0)
        notice(tr("Save as Template"), tr("The template could not be saved to \"%1\".").arg(QDir::toNativeSeparators(path)));
    else
        setStatus(tr("Saved the chart as a template."));
}

// Apply Template: the picked chart takes the look of a template file and keeps its data.
void CrunchWindow::applyChartTemplate()
{
    const int id = pickedObject();
    if (id < 0 || !m_session || !m_objects || m_chartSheetShown) return;
    QString why;
    if (!m_objects->canEditChart(m_sheet, id, &why)) {
        notice(tr("Apply Template"), why);
        return;
    }
    const QString path = m_templatePicker ? m_templatePicker() : QFileDialog::getOpenFileName(this, tr("Apply Template"), QString(), tr("Chart templates (*.crtx)"));
    if (path.isEmpty()) return;
    QFile f(path);
    Chart look;
    if (!f.open(QIODevice::ReadOnly) || f.size() > (qint64(64) << 20)) {
        notice(tr("Apply Template"), tr("The template \"%1\" could not be read.").arg(QDir::toNativeSeparators(path)));
        return;
    }
    if (!readChartTemplate(f.readAll(), &look, &why)) {
        notice(tr("Apply Template"), why);
        return;
    }
    changePickedChart(tr("Apply Template"), [look](Chart *c) { *c = chartWithLook(*c, look); });
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
    p.description = m_objects->descriptionOf(m_sheet, id);
    p.keepProportions = m_objects->kindOf(m_sheet, id) == ObjectKind::Picture;
    p.prints = m_objects->printsOf(m_sheet, id);
    ObjectProperties out = p;
    if (m_propertiesAsker) {
        if (!m_propertiesAsker(p, &out)) return;
    } else {
        ObjectPropertiesDialog d(p, this);
        if (d.exec() != QDialog::Accepted) return;
        out = d.result();
    }
    if (out.name != p.name || out.description != p.description) {
        // (a name no other object of the sheet has)
        if (out.name != p.name && m_objects->nameTaken(m_sheet, id, out.name)) {
            notice(tr("Properties"), tr("Another object on this sheet is called \"%1\". Pick a different name.").arg(out.name));
            out.name = p.name;
        }
        if (out.name != p.name || out.description != p.description) {
            crunch::ObjectEdit e;
            e.sheet = m_sheet;
            e.id = id;
            e.before = m_objects->stateOf(m_sheet, id);
            e.after = m_objects->stateWithNames(m_sheet, id, out.name, out.description);
            const crunch::EditSession::Outcome o = m_session->changeObjects({e}, out.name != p.name ? tr("Rename Object") : tr("Alt Text"));
            if (!o.ok) notice(tr("Properties"), o.problem);
        }
    }
    if (out.prints != p.prints) {
        crunch::ObjectEdit e;
        e.sheet = m_sheet;
        e.id = id;
        e.before = m_objects->stateOf(m_sheet, id);
        e.after = m_objects->stateWithPrints(m_sheet, id, out.prints);
        const crunch::EditSession::Outcome o = m_session->changeObjects({e}, out.prints ? tr("Print Object") : tr("Do Not Print Object"));
        if (!o.ok) notice(tr("Properties"), o.problem);
    }
    // (the box shows sizes to a tenth of a point: a size within that of the one it started with is not a change)
    if (std::abs(out.widthPoints - p.widthPoints) < 0.06 && std::abs(out.heightPoints - p.heightPoints) < 0.06 && out.follows == p.follows) return;   // (only what it prints with changed: done above)
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

// Move Chart: the chart goes to another worksheet (where its cells put it there too): taken off this sheet and put on that one as one step. It still reads the cells it read.
void CrunchWindow::moveChartDialog()
{
    const int id = pickedObject();
    if (id < 0 || !m_session || !m_objects || !m_loaded || !m_loaded->book) return;
    QString why;
    if (!m_objects->canEditChart(m_sheet, id, &why)) {
        notice(tr("Move Chart"), why);
        return;
    }
    QStringList names;
    QVector<int> places;
    const auto &sheets = m_loaded->book->info().sheets;
    for (int i = 0; i < sheets.size(); ++i)
        if (i != m_sheet && !sheets[i].deleted && sheets[i].state == jp::office::sheet::SheetState::Visible && sheets[i].kind == jp::office::sheet::SheetKind::Worksheet) {
            names << sheets[i].name;
            places << i;
        }
    if (names.isEmpty()) {
        notice(tr("Move Chart"), tr("There is no other worksheet to move the chart to."));
        return;
    }
    int chosen = 0;
    if (m_moveChartAsker) {
        if (!m_moveChartAsker(names, &chosen)) return;
    } else {
        bool ok = false;
        const QString name = QInputDialog::getItem(this, tr("Move Chart"), tr("Move the chart to this sheet:"), names, 0, false, &ok);
        if (!ok) return;
        chosen = int(names.indexOf(name));
    }
    if (chosen < 0 || chosen >= places.size()) return;
    const int target = places[chosen];
    const Chart chart = m_objects->chartNow(m_sheet, id);
    // (it keeps its size and its place in points: the cells it spans may be wider or narrower on the other sheet, and it is put over the cells those points reach there)
    const grid::ObjectBox box = m_objects->layer()->boxNow(id);
    DrawingAnchor anchor = m_objects->anchorOf(m_sheet, id);
    const jp::office::sheet::AnchorKind keepEdit = anchor.editAs;
    anchor = DrawingAnchor();
    anchor.kind = jp::office::sheet::AnchorKind::Absolute;
    anchor.editAs = keepEdit;
    anchor.x = qint64(std::llround(box.x * 9525.0));
    anchor.y = qint64(std::llround(box.y * 9525.0));
    anchor.cx = qint64(std::llround(box.w * 9525.0));
    anchor.cy = qint64(std::llround(box.h * 9525.0));
    anchor = m_objects->anchorKeepingSize(target, anchor);
    const int newId = m_objects->makeChart(target, chart, anchor, &why);
    if (newId < 0) {
        notice(tr("Move Chart"), why);
        return;
    }
    crunch::ObjectEdit away, here;
    away.sheet = m_sheet;
    away.id = id;
    // (what the chart reached in the file comes along to the new sheet, and what writes it back if Undo is used after a save)
    const auto revive = m_objects->captureRevive(m_sheet, id);
    m_objects->adoptRevive(target, newId, revive);
    away.before = m_objects->stateOf(m_sheet, id);
    away.before.revive = revive;
    away.after = away.before;
    away.after.deleted = true;
    here.sheet = target;
    here.id = newId;
    here.before = m_objects->stateOf(target, newId);
    here.after = here.before;
    here.after.deleted = false;
    const crunch::EditSession::Outcome o = m_session->changeObjects({away, here}, tr("Move Chart"));
    if (!o.ok) {
        m_objects->discardMade(target, newId);
        notice(tr("Move Chart"), o.problem);
        return;
    }
    setActiveSheet(target);
    m_objects->layer()->select(newId);
}

// Format Axis: the axis across the chart (or up it), asked about and set as one step.
void CrunchWindow::formatAxisDialog(bool horizontal)
{
    const int id = pickedObject();
    if (id < 0 || !m_session || !m_objects) return;
    QString why;
    if (!m_objects->canEditChart(m_sheet, id, &why)) {
        notice(tr("Format Axis"), why);
        return;
    }
    const Chart chart = m_objects->chartNow(m_sheet, id);   // (kept: the axis is read from it)
    const ChartAxis *now = primaryAxis(chart, horizontal);
    if (!now) return;
    ChartAxis made = *now;
    if (m_formatAxisAsker) {
        if (!m_formatAxisAsker(horizontal, &made)) return;
    } else {
        FormatAxisDialog dialog(*now, horizontal ? tr("Format Horizontal Axis") : tr("Format Vertical Axis"), this);
        if (dialog.exec() != QDialog::Accepted) return;
        made = dialog.result();
    }
    if (const QString problem = axisFormatProblem(made); !problem.isEmpty()) {
        notice(tr("Format Axis"), problem);
        return;
    }
    changePickedChart(tr("Format Axis"), [horizontal, made](Chart *c) { setAxisFormat(c, horizontal, false, made); });
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
        // Title text: typed words, or "=Sheet1!A1" for the words of a cell (the title follows that cell).
        auto titleText = [this](QMenu *into, const QString &label, const QString &caption, const ChartTitle &now, std::function<void(Chart *, const QString &, const QString &)> put) {
            QAction *a = into->addAction(label);
            connect(a, &QAction::triggered, this, [this, caption, now, put] {
                QString text = now.ref.isEmpty() ? now.text : QStringLiteral("=") + now.ref;
                bool ok = false;
                if (m_chartTextAsker) ok = m_chartTextAsker(caption, &text);
                else text = QInputDialog::getText(this, caption, tr("The words, or = and a cell such as =Sheet1!A1 to show what that cell holds:"), QLineEdit::Normal, text, &ok);
                if (!ok) return;
                text = text.trimmed();
                QString ref, words = text;
                if (text.startsWith(QLatin1Char('='))) {
                    const int id = pickedObject();
                    if (id < 0 || !m_objects || !m_objects->cells() || !m_loaded || !m_loaded->book) return;
                    ref = text.mid(1).trimmed();
                    if (!ref.contains(QLatin1Char('!'))) {
                        const QString own = m_loaded->book->info().sheets[m_sheet].name;
                        ref = (own.contains(QRegularExpression(QStringLiteral("[^A-Za-z0-9_]"))) ? QStringLiteral("'") + QString(own).replace(QLatin1Char('\''), QStringLiteral("''")) + QStringLiteral("'") : own) + QLatin1Char('!') + ref;
                    }
                    // (one cell, written with $ the way a chart keeps it)
                    QVector<ChartBlock> blocks;
                    if (!chartBlocks(ref, m_sheet, *m_objects->cells(), &blocks) || blocks.size() != 1 || blocks[0].range.row1 != blocks[0].range.row2 || blocks[0].range.col1 != blocks[0].range.col2) {
                        notice(caption, tr("\"%1\" isn't one cell JeffCrunch can find. Write it like =Sheet1!$A$1.").arg(text.mid(1)));
                        return;
                    }
                    const int bang = int(ref.lastIndexOf(QLatin1Char('!')));
                    ref = ref.left(bang + 1) + QStringLiteral("$") + columnName(blocks[0].range.col1) + QStringLiteral("$") + QString::number(qint64(blocks[0].range.row1) + 1);
                    words = QString();
                }
                const int sheet = m_sheet;
                changePickedChart(caption, [this, put, words, ref, sheet](Chart *c) {
                    put(c, words, ref);
                    if (!ref.isEmpty() && m_objects->cells()) applyChartCells(c, sheet, *m_objects->cells());
                });
            });
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
            titles->addSeparator();
            if (e.hasHorizontal)
                if (const ChartAxis *a = primaryAxis(chart, true))
                    titleText(titles, tr("Horizontal Title Text…"), tr("Axis Titles"), a->title, [](Chart *c, const QString &t, const QString &r) { setAxisTitleText(c, true, t, r); });
            if (e.hasVertical)
                if (const ChartAxis *a = primaryAxis(chart, false))
                    titleText(titles, tr("Vertical Title Text…"), tr("Axis Titles"), a->title, [](Chart *c, const QString &t, const QString &r) { setAxisTitleText(c, false, t, r); });
            if (e.hasSecondaryHorizontal) {
                check(axes, tr("Second Horizontal"), e.axisSecondaryHorizontal, [on = !e.axisSecondaryHorizontal](Chart *c) { setAxisShown(c, true, on, true); }, tr("Axes"));
                check(titles, tr("Second Horizontal"), e.axisTitleSecondaryHorizontal, [on = !e.axisTitleSecondaryHorizontal](Chart *c) { setAxisTitle(c, true, on, true); }, tr("Axis Titles"));
            }
            if (e.hasSecondaryVertical) {
                check(axes, tr("Second Vertical"), e.axisSecondaryVertical, [on = !e.axisSecondaryVertical](Chart *c) { setAxisShown(c, false, on, true); }, tr("Axes"));
                check(titles, tr("Second Vertical"), e.axisTitleSecondaryVertical, [on = !e.axisTitleSecondaryVertical](Chart *c) { setAxisTitle(c, false, on, true); }, tr("Axis Titles"));
            }
        }
        QMenu *title = menu->addMenu(tr("Chart Title"));
        title->setObjectName(QStringLiteral("chartTitleMenu"));
        auto *titleGroup = new QActionGroup(title);
        choice(title, titleGroup, tr("None"), !e.title, [](Chart *c) { setChartTitle(c, TitleMode::None); }, tr("Chart Title"));
        choice(title, titleGroup, tr("Above"), e.title && !e.titleOverlay, [](Chart *c) { setChartTitle(c, TitleMode::Above); }, tr("Chart Title"));
        choice(title, titleGroup, tr("On Top"), e.titleOverlay, [](Chart *c) { setChartTitle(c, TitleMode::Overlay); }, tr("Chart Title"));
        title->addSeparator();
        titleText(title, tr("Title Text…"), tr("Chart Title"), chart.title, [](Chart *c, const QString &t, const QString &r) { setChartTitleText(c, t, r); });
        QMenu *labels = menu->addMenu(tr("Data Labels"));
        labels->setObjectName(QStringLiteral("dataLabelsMenu"));
        auto *labelGroup = new QActionGroup(labels);
        choice(labels, labelGroup, tr("None"), !e.labels, [](Chart *c) { setDataLabels(c, false); }, tr("Data Labels"));
        choice(labels, labelGroup, tr("Usual place"), e.labels && e.labelPosition.isEmpty(), [](Chart *c) { setDataLabels(c, true); }, tr("Data Labels"));
        static const struct { const char *code; const char *name; } places[] = {{"ctr", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Center")}, {"inEnd", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Inside End")}, {"inBase", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Inside Base")},
                                                                                  {"outEnd", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Outside End")}, {"bestFit", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Best Fit")}, {"l", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Left")},
                                                                                  {"r", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Right")}, {"t", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Above")}, {"b", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Below")}};
        const QStringList allowed = labelPositionsFor(chart);
        for (const auto &p : places) {
            const QString code = QString::fromLatin1(p.code);
            if (!allowed.contains(code)) continue;
            choice(labels, labelGroup, QCoreApplication::translate("jp::CrunchWindow", p.name), e.labels && e.labelPosition == code, [code](Chart *c) { setDataLabels(c, true, code); }, tr("Data Labels"));
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
                }, tr("Data Labels"));
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
                }, tr("Data Labels"));
            // Value From Cells: the labels show the words of a range of cells, one for each point (empty: none).
            QAction *fromCells = labels->addAction(tr("Value From Cells…"));
            fromCells->setObjectName(QStringLiteral("labelsFromCellsAction"));
            fromCells->setCheckable(true);
            fromCells->setChecked(!dataLabelsCellsRef(chart).isEmpty());
            connect(fromCells, &QAction::triggered, this, [this, now = dataLabelsCellsRef(chart)] {
                QString text = now;
                bool ok = false;
                if (m_chartTextAsker) ok = m_chartTextAsker(tr("Value From Cells"), &text);
                else text = QInputDialog::getText(this, tr("Value From Cells"), tr("The cells that hold the words for the labels, such as =Sheet1!$D$2:$D$4 (leave empty to take them away):"), QLineEdit::Normal, text, &ok);
                if (!ok) return;
                text = text.trimmed();
                if (text.startsWith(QLatin1Char('='))) text = text.mid(1).trimmed();
                if (!text.isEmpty()) {
                    if (!m_objects || !m_objects->cells() || !m_loaded || !m_loaded->book) return;
                    if (!text.contains(QLatin1Char('!'))) {
                        const QString own = m_loaded->book->info().sheets[m_sheet].name;
                        text = (own.contains(QRegularExpression(QStringLiteral("[^A-Za-z0-9_]"))) ? QStringLiteral("'") + QString(own).replace(QLatin1Char('\''), QStringLiteral("''")) + QStringLiteral("'") : own) + QLatin1Char('!') + text;
                    }
                    QVector<ChartBlock> blocks;
                    if (!chartBlocks(text, m_sheet, *m_objects->cells(), &blocks) || blocks.size() != 1) {
                        notice(tr("Value From Cells"), tr("\"%1\" isn't a range JeffCrunch can find. Write it like =Sheet1!$D$2:$D$4.").arg(text));
                        return;
                    }
                    const auto r = blocks[0].range;
                    const int bang = int(text.lastIndexOf(QLatin1Char('!')));
                    text = text.left(bang + 1) + QStringLiteral("$") + columnName(r.col1) + QStringLiteral("$") + QString::number(qint64(r.row1) + 1);
                    if (r.row1 != r.row2 || r.col1 != r.col2) text += QStringLiteral(":$") + columnName(r.col2) + QStringLiteral("$") + QString::number(qint64(r.row2) + 1);
                }
                const int sheet = m_sheet;
                changePickedChart(tr("Data Labels"), [this, text, sheet](Chart *c) {
                    setDataLabelsFromCells(c, text);
                    if (!text.isEmpty() && m_objects->cells()) applyChartCells(c, sheet, *m_objects->cells());
                });
            });
            QAction *format = labels->addAction(tr("Number Format…"));
            connect(format, &QAction::triggered, this, [this, parts] {
                QString code = parts.format;
                bool ok = false;
                if (m_chartTextAsker) ok = m_chartTextAsker(tr("Number Format"), &code);
                else code = QInputDialog::getText(this, tr("Number Format"), tr("Format code for the numbers in the labels (leave empty to show the numbers as they are):"), QLineEdit::Normal, code, &ok);
                if (!ok) return;
                changePickedChart(tr("Data Labels"), [parts, code](Chart *c) {
                    ChartLabelParts now = parts;
                    now.format = code.trimmed();
                    setDataLabelParts(c, now);
                });
            });
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
        const struct { const char *code; const char *name; } sides[] = {{"r", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Right")}, {"t", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Top")}, {"l", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Left")}, {"b", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Bottom")}};
        for (const auto &s : sides) {
            const QString code = QString::fromLatin1(s.code);
            choice(legend, legendGroup, QCoreApplication::translate("jp::CrunchWindow", s.name), e.legend && e.legendPosition == code, [code, over = e.legendOverlay](Chart *c) { setLegend(c, code, over); }, tr("Legend"));
        }
        if (e.legend) {
            legend->addSeparator();
            check(legend, tr("Over the Chart"), e.legendOverlay, [code = e.legendPosition, on = !e.legendOverlay](Chart *c) { setLegend(c, code, on); }, tr("Legend"));
        }
        if (errorBarsApply(chart)) {
            QMenu *bars = menu->addMenu(tr("Error Bars"));
            bars->setObjectName(QStringLiteral("errorBarsMenu"));
            auto *barGroup = new QActionGroup(bars);
            const QString kind = errorBarsKind(chart);
            choice(bars, barGroup, tr("None"), kind.isEmpty(), [](Chart *c) { setErrorBars(c, QString()); }, tr("Error Bars"));
            choice(bars, barGroup, tr("Standard Error"), kind == QLatin1String("stdErr"), [](Chart *c) { setErrorBars(c, QStringLiteral("stdErr")); }, tr("Error Bars"));
            choice(bars, barGroup, tr("Percentage (5%)"), kind == QLatin1String("percentage"), [](Chart *c) { setErrorBars(c, QStringLiteral("percentage"), 5); }, tr("Error Bars"));
            choice(bars, barGroup, tr("Standard Deviation"), kind == QLatin1String("stdDev"), [](Chart *c) { setErrorBars(c, QStringLiteral("stdDev"), 1); }, tr("Error Bars"));
            choice(bars, barGroup, tr("Fixed Value (1)"), kind == QLatin1String("fixedVal"), [](Chart *c) { setErrorBars(c, QStringLiteral("fixedVal"), 1); }, tr("Error Bars"));
            // Custom: the amounts above and below each point are read from cells typed in (empty: none on that side).
            QAction *custom = bars->addAction(tr("Custom…"));
            custom->setObjectName(QStringLiteral("customErrorBarsAction"));
            custom->setCheckable(true);
            custom->setChecked(kind == QLatin1String("cust"));
            connect(custom, &QAction::triggered, this, [this, now = customErrorBarRefs(chart)] {
                QStringList refs{now.value(0), now.value(1)};
                for (int side = 0; side < 2; ++side) {
                    const QString title = side == 0 ? tr("Custom Error Bars: Positive") : tr("Custom Error Bars: Negative");
                    QString text = refs[side];
                    bool ok = false;
                    if (m_chartTextAsker) ok = m_chartTextAsker(title, &text);
                    else text = QInputDialog::getText(this, title, tr("The cells that hold the amounts, such as =Sheet1!$D$2:$D$4 (leave empty for none):"), QLineEdit::Normal, text, &ok);
                    if (!ok) return;
                    text = text.trimmed();
                    if (text.startsWith(QLatin1Char('='))) text = text.mid(1).trimmed();
                    if (!text.isEmpty()) {
                        if (!m_objects || !m_objects->cells() || !m_loaded || !m_loaded->book) return;
                        if (!text.contains(QLatin1Char('!'))) {
                            const QString own = m_loaded->book->info().sheets[m_sheet].name;
                            text = (own.contains(QRegularExpression(QStringLiteral("[^A-Za-z0-9_]"))) ? QStringLiteral("'") + QString(own).replace(QLatin1Char('\''), QStringLiteral("''")) + QStringLiteral("'") : own) + QLatin1Char('!') + text;
                        }
                        QVector<ChartBlock> blocks;
                        if (!chartBlocks(text, m_sheet, *m_objects->cells(), &blocks) || blocks.size() != 1) {
                            notice(tr("Custom Error Bars"), tr("\"%1\" isn't a range JeffCrunch can find. Write it like =Sheet1!$D$2:$D$4.").arg(text));
                            return;
                        }
                        const auto r = blocks[0].range;
                        const int bang = int(text.lastIndexOf(QLatin1Char('!')));
                        text = text.left(bang + 1) + QStringLiteral("$") + columnName(r.col1) + QStringLiteral("$") + QString::number(qint64(r.row1) + 1);
                        if (r.row1 != r.row2 || r.col1 != r.col2) text += QStringLiteral(":$") + columnName(r.col2) + QStringLiteral("$") + QString::number(qint64(r.row2) + 1);
                    }
                    refs[side] = text;
                }
                if (refs[0].isEmpty() && refs[1].isEmpty()) return;
                const int sheet = m_sheet;
                changePickedChart(tr("Error Bars"), [this, refs, sheet](Chart *c) {
                    setCustomErrorBars(c, refs[0], refs[1]);
                    if (m_objects->cells()) applyChartCells(c, sheet, *m_objects->cells());
                });
            });
        }
        if (!chart.pivotName.isEmpty()) {
            QMenu *buttons = menu->addMenu(tr("Field Buttons"));
            buttons->setObjectName(QStringLiteral("fieldButtonsMenu"));
            const struct { const char *name; PivotButton which; } kinds[] = {{QT_TRANSLATE_NOOP("jp::CrunchWindow", "Report Filter Field Buttons"), PivotButton::Filter},
                                                                             {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Axis Field Buttons"), PivotButton::Axis},
                                                                             {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Legend Field Buttons"), PivotButton::Legend},
                                                                             {QT_TRANSLATE_NOOP("jp::CrunchWindow", "Value Field Buttons"), PivotButton::Value}};
            for (const auto &k : kinds)
                check(buttons, QCoreApplication::translate("jp::CrunchWindow", k.name), pivotButtonsOn(chart, k.which), [which = k.which, on = !pivotButtonsOn(chart, k.which)](Chart *c) { setPivotButtons(c, which, on); }, tr("Field Buttons"));
            buttons->addSeparator();
            check(buttons, tr("Show All"), pivotButtonsOn(chart, PivotButton::All), [](Chart *c) { setPivotButtons(c, PivotButton::All, true); }, tr("Field Buttons"));
            check(buttons, tr("Hide All"), chart.pivotButtons.set && !chart.pivotButtons.filter && !chart.pivotButtons.categories && !chart.pivotButtons.series && !chart.pivotButtons.data, [](Chart *c) { setPivotButtons(c, PivotButton::All, false); }, tr("Field Buttons"));
        }
        if (plotLinesApply(chart, PlotLines::Drop)) {
            QMenu *lines = menu->addMenu(tr("Lines"));
            lines->setObjectName(QStringLiteral("plotLinesMenu"));
            check(lines, tr("Drop Lines"), plotLinesOn(chart, PlotLines::Drop), [on = !plotLinesOn(chart, PlotLines::Drop)](Chart *c) { setPlotLines(c, PlotLines::Drop, on); }, tr("Lines"));
            if (plotLinesApply(chart, PlotLines::HighLow)) {
                check(lines, tr("High-Low Lines"), plotLinesOn(chart, PlotLines::HighLow), [on = !plotLinesOn(chart, PlotLines::HighLow)](Chart *c) { setPlotLines(c, PlotLines::HighLow, on); }, tr("Lines"));
                check(lines, tr("Up/Down Bars"), plotLinesOn(chart, PlotLines::UpDown), [on = !plotLinesOn(chart, PlotLines::UpDown)](Chart *c) { setPlotLines(c, PlotLines::UpDown, on); }, tr("Lines"));
            }
        }
        if (chart.type() != ChartType::Pie && chart.type() != ChartType::Doughnut) {
            QMenu *trend = menu->addMenu(tr("Trendline"));
            trend->setObjectName(QStringLiteral("trendlineMenu"));
            auto *trendGroup = new QActionGroup(trend);
            choice(trend, trendGroup, tr("None"), e.trendline.isEmpty(), [](Chart *c) { setTrendlines(c, QString()); }, tr("Trendline"));
            static const struct { const char *code; const char *name; } kinds[] = {{"linear", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Linear")}, {"exp", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Exponential")}, {"log", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Logarithmic")},
                                                                                  {"poly", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Polynomial")}, {"power", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Power")}, {"movingAvg", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Moving Average")}};
            for (const auto &k : kinds) {
                const QString code = QString::fromLatin1(k.code);
                choice(trend, trendGroup, QCoreApplication::translate("jp::CrunchWindow", k.name), e.trendline == code, [code](Chart *c) { setTrendlines(c, code); }, tr("Trendline"));
            }
            if (!e.trendline.isEmpty()) {
                trend->addSeparator();
                check(trend, tr("Show Equation"), e.trendlineEquation, [on = !e.trendlineEquation, r = e.trendlineRSquared](Chart *c) { setTrendlineDisplay(c, on, r); }, tr("Trendline"));
                check(trend, tr("Show R-squared"), e.trendlineRSquared, [on = !e.trendlineRSquared, q = e.trendlineEquation](Chart *c) { setTrendlineDisplay(c, q, on); }, tr("Trendline"));
            }
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

QVector<CrunchWindow::ChartFilterEntry> CrunchWindow::chartFilterEntries() const
{
    QVector<ChartFilterEntry> out;
    const int id = pickedObject();
    if (id < 0 || !m_objects || m_objects->kindOf(m_sheet, id) != ObjectKind::Chart) return out;
    int n = 0;
    for (const ChartPlot &plot : m_objects->chartNow(m_sheet, id).plots)
        for (const ChartSeries &s : plot.series) {
            ++n;
            out.append({s.name.isEmpty() ? tr("Series %1").arg(n) : s.name, !s.filtered});
        }
    return out;
}

bool CrunchWindow::setChartSeriesShown(int entry, bool shown)
{
    const int id = pickedObject();
    if (id < 0 || !m_objects || m_objects->kindOf(m_sheet, id) != ObjectKind::Chart) return false;
    const Chart now = m_objects->chartNow(m_sheet, id);
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
        notice(tr("Chart Filters"), tr("A chart needs at least one series. Check another series first."), false);
        return false;
    }
    return changePickedChart(tr("Chart Filters"), [entry, shown](Chart *c) {
        int n = 0;
        for (ChartPlot &plot : c->plots)
            for (ChartSeries &s : plot.series)
                if (n++ == entry) s.filtered = !shown;
    });
}

QMenu *CrunchWindow::buildChartFiltersMenu(QWidget *owner)
{
    auto *menu = new QMenu(owner);
    menu->setObjectName(QStringLiteral("chartFiltersMenu"));
    connect(menu, &QMenu::aboutToShow, this, [this, menu] {
        menu->clear();
        menu->addSection(tr("Series"));
        const QVector<ChartFilterEntry> entries = chartFilterEntries();
        for (int i = 0; i < entries.size(); ++i) {
            QAction *a = menu->addAction(entries[i].name);
            a->setCheckable(true);
            a->setChecked(entries[i].shown);
            connect(a, &QAction::triggered, this, [this, i](bool on) { setChartSeriesShown(i, on); });
        }
    });
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
    {
        // (Move Chart needs another worksheet to go to)
        bool other = false;
        if (editable && m_loaded && m_loaded->book)
            for (int i = 0; i < m_loaded->book->info().sheets.size(); ++i)
                if (i != m_sheet && !m_loaded->book->info().sheets[i].deleted && m_loaded->book->info().sheets[i].kind == jp::office::sheet::SheetKind::Worksheet) other = true;
        act(QStringLiteral("chartdesign.moveChart"))->setEnabled(other);
    }
    {
        const ChartElements e = editable && id >= 0 ? elementsOf(m_objects->chartNow(m_sheet, id)) : ChartElements();
        act(QStringLiteral("chartdesign.axisH"))->setEnabled(e.hasHorizontal);
        act(QStringLiteral("chartdesign.axisV"))->setEnabled(e.hasVertical);
    }
    act(QStringLiteral("chartdesign.saveAsPicture"))->setEnabled(chart);
    act(QStringLiteral("chartdesign.saveTemplate"))->setEnabled(chart);
    act(QStringLiteral("chartdesign.applyTemplate"))->setEnabled(editable);
    for (const char *name : {"object.properties", "object.front", "object.forward", "object.backward", "object.back", "object.delete"}) act(QString::fromLatin1(name))->setEnabled(picked);
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Chart"), chart);
    {
        const ObjectKind kind = picked ? m_objects->kindOf(m_sheet, id) : ObjectKind::Other;
        const bool picture = kind == ObjectKind::Picture, shape = kind == ObjectKind::Shape, line = kind == ObjectKind::Connector;
        for (const char *name : {"object.rotateRight", "object.rotateLeft", "object.flipH", "object.flipV"}) act(QString::fromLatin1(name))->setEnabled(picture || shape || line);
        for (const char *name : {"object.cropSquare", "object.crop43", "object.crop34", "object.crop169", "object.crop916", "object.cropReset"}) act(QString::fromLatin1(name))->setEnabled(picture);
        act(QStringLiteral("object.editText"))->setEnabled(shape);
        enableShapeStyles(shape);
        if (m_ribbon) {
            m_ribbon->setContextVisible(QStringLiteral("Picture Format"), picture);
            // (the shapes tab also stays for a group, and for several objects picked together: it has Group and Align)
            const std::vector<int> several = pickedObjects();
            m_ribbon->setContextVisible(QStringLiteral("Shape Format"), shape || line || kind == ObjectKind::Group || several.size() >= 2);
        }
    }
    m_chartButtonsTimer.start();
    showNameInBox();
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
                m_chartFilter = new QToolButton(m_view->viewport());
                m_chartFilter->setObjectName(QStringLiteral("chartFiltersButton"));
                m_chartFilter->setIcon(icon(QStringLiteral("filter")));
                m_chartFilter->setToolTip(tr("Chart Filters"));
                m_chartFilter->setAccessibleName(tr("Chart Filters"));
                m_chartFilter->setPopupMode(QToolButton::InstantPopup);
                m_chartFilter->setFocusPolicy(Qt::NoFocus);
                m_chartFilter->setMenu(buildChartFiltersMenu(m_chartFilter));
            }
            const int size = 26;
            const QRect vp = m_view->viewport()->rect();
            int x = r.right() + 6;
            if (x + size > vp.right()) x = r.right() - size - 4;   // (no room beside it: inside its corner)
            x = std::clamp(x, 0, std::max(0, vp.right() - size));
            const int y = std::clamp(r.top(), 0, std::max(0, vp.bottom() - 3 * size - 8));
            for (QToolButton *b : {m_chartPlus, m_chartBrush, m_chartFilter}) {
                b->setFixedSize(size, size);
                b->setIconSize(QSize(16, 16));
            }
            m_chartPlus->move(x, y);
            m_chartBrush->move(x, y + size + 4);
            m_chartFilter->move(x, y + 2 * (size + 4));
            show = true;
        }
    }
    if (m_chartPlus) {
        m_chartPlus->setVisible(show);
        m_chartBrush->setVisible(show);
        m_chartFilter->setVisible(show);
        if (show) {
            m_chartPlus->raise();
            m_chartBrush->raise();
            m_chartFilter->raise();
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
    copy->setEnabled(true);
    menu->addAction(act(QStringLiteral("edit.paste")));
    menu->addAction(act(QStringLiteral("object.delete")));
    menu->addSeparator();
    QMenu *order = menu->addMenu(tr("Order"));
    for (const char *name : {"object.front", "object.forward", "object.backward", "object.back"}) order->addAction(act(QString::fromLatin1(name)));
    if (chart) {
        menu->addSeparator();
        menu->addAction(act(QStringLiteral("chartdesign.selectData")));
        menu->addAction(act(QStringLiteral("chartdesign.changeType")));
        menu->addAction(act(QStringLiteral("chartdesign.saveAsPicture")));
        menu->addAction(act(QStringLiteral("chartdesign.saveTemplate")));
        menu->addAction(act(QStringLiteral("chartdesign.applyTemplate")));
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
    const ObjectKind kind = m_objects->kindOf(m_sheet, id);
    if (cut && !editingAllowed()) return false;
    crunch::ObjectClip clip;
    QImage picture;
    const grid::ObjectBox box = m_objects->layer()->boxNow(id);
    if (kind == ObjectKind::Chart) {
        const Chart chart = m_objects->chartNow(m_sheet, id);
        clip.kind = QStringLiteral("chart");
        clip.chartXml = writeChartPart(chart);
        clip.width = std::max(4.0, box.w);
        clip.height = std::max(4.0, box.h);
        picture = renderChart(chart, QSize(int(clip.width), int(clip.height)));
    } else {
        QString why;
        if (!m_objects->copyElement(m_sheet, id, &clip, &why)) {
            setStatus(why);
            return false;
        }
    }
    clip.origin = m_workbookId + QLatin1Char('|') + m_path;
    clip.sheetName = m_loaded && m_loaded->book ? m_loaded->book->info().sheets[m_sheet].name : QString();
    auto *md = new QMimeData;
    clip.toMime(md);
    if (!picture.isNull()) md->setImageData(picture);
    m_settingClipboard = true;
    QApplication::clipboard()->setMimeData(md);
    m_settingClipboard = false;
    m_clip = crunch::ClipData();
    m_view->clearCopyBorder();
    if (cut) deleteObject(id);
    const QString what = kind == ObjectKind::Chart ? tr("Chart") : kind == ObjectKind::Picture ? tr("Picture") : tr("Shape");
    setStatus(cut ? tr("%1 cut").arg(what) : tr("%1 copied").arg(what));
    return true;
}

bool CrunchWindow::pasteObject()
{
    const QMimeData *md = QApplication::clipboard()->mimeData();
    if (!md || !md->hasFormat(QLatin1String(crunch::kObjectMime))) return false;
    crunch::ObjectClip clip;
    if (!crunch::ObjectClip::fromMime(md, &clip)) return false;
    if (!m_session || !m_objects || m_chartSheetShown) return true;
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return true;
    const bool isChart = clip.kind == QLatin1String("chart");
    Chart chart;
    // it reads this workbook's cells only if it was copied from this workbook (by its own id: not another program's, nor another opening of the file) and everything it reads is still there (a
    // sheet renamed or deleted since would make its references name other cells, or none)
    bool sameBook = !m_workbookId.isEmpty() && clip.origin.section(QLatin1Char('|'), 0, 0) == m_workbookId && m_objects->cells() && m_objects->cells()->sheetIndex(clip.sheetName) >= 0;
    if (isChart) {
        if (sameBook) {
            Chart probe;
            if (clip.chart(m_objects->theme(), true, &probe)) {
                for (const ChartSeries &s : seriesOf(probe))
                    for (const QString &ref : {s.nameRef, s.categoryRef, s.valueRef, s.xRef}) {
                        QVector<ChartBlock> blocks;
                        if (!ref.isEmpty() && !chartBlocks(ref, m_sheet, *m_objects->cells(), &blocks)) sameBook = false;
                    }
            }
        }
        if (!clip.chart(m_objects->theme(), sameBook, &chart)) {
            notice(tr("Paste"), tr("The chart on the clipboard can't be read."));
            return true;
        }
    }
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
    const int id = isChart ? m_objects->makeChart(m_sheet, chart, anchor, &why) : m_objects->makeElement(m_sheet, clip, anchor, &why);
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
    const crunch::EditSession::Outcome o = m_session->changeObjects({e}, isChart ? tr("Paste Chart") : tr("Paste Object"));
    if (!o.ok) {
        m_objects->discardMade(m_sheet, id);
        notice(tr("Paste"), o.problem);
        return true;
    }
    m_objects->layer()->select(id);
    if (!isChart) setStatus(tr("Pasted"));
    else setStatus(sameBook ? tr("Chart pasted") : tr("Chart pasted with the numbers it showed: it doesn't read this workbook's cells."));
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
