// Making and changing charts in JeffCrunch's window: New Chart from the cells selected, the chart's buttons and menus (its elements, colors, and looks), Chart Data, Chart Kind, the
// properties and the context menu of an object, and Copy, Cut, and Paste of a chart. Each change is one step of Undo (core/objectstate.h); the chart model it changes is
// libs/office/sheet/chartedit.h's, and what it writes is libs/office/office/chart_write.h's.

#include "app/equationtab.h"
#include "app/uistyle.h"
#include "app/crunchwindow.h"
#include "app/objects.h"

#include "app/chartdialogs.h"
#include "app/icons.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "core/objectclip.h"
#include "core/sheetobjects.h"
#include "render/textart.h"
#include "office/chart_write.h"
#include "sheet/cellref.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <cmath>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QRadioButton>
#include <QUndoStack>
#include <QVBoxLayout>
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
    parts.menus[QStringLiteral("chartElements")] = [this](QWidget *owner) { return m_design->elementsMenu(owner); };
    parts.menus[QStringLiteral("chartLayouts")] = [this](QWidget *owner) { return m_design->layoutsMenu(owner); };
    parts.menus[QStringLiteral("chartColors")] = [this](QWidget *owner) { return m_design->colorsMenu(owner); };
    parts.menus[QStringLiteral("chartStyles")] = [this](QWidget *owner) { return m_design->stylesMenu(owner); };
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
    return chartPicture(chart, pixels);
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
    const QImage image = chartPicture(chart, pixels);
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

// The unused name for a new chart sheet: "Chart1", "Chart2", and so on.
QString CrunchWindow::newChartSheetName() const
{
    for (int n = 1;; ++n) {
        const QString name = tr("Chart%1").arg(n);
        if (!m_session || m_session->validSheetName(name, -1)) return name;
    }
}

// Move Chart: the chart goes to a chart sheet of its own (made for it), or onto a worksheet (where its cells put it there too). It is taken off this sheet and put on that one as one step, and
// a chart sheet it leaves is deleted with it. It keeps reading the cells it read.
void CrunchWindow::moveChartDialog()
{
    const int id = pickedObject();
    if (id < 0 || !m_session || !m_objects || !m_loaded || !m_loaded->book) return;
    QString why;
    if (!m_objects->canEditChart(m_sheet, id, &why, true)) {
        notice(tr("Move Chart"), why);
        return;
    }
    const bool fromChartSheet = m_objects->isChartSheet(m_sheet);
    QStringList names;
    QVector<int> places;
    const auto &sheets = m_loaded->book->info().sheets;
    for (int i = 0; i < sheets.size(); ++i)
        if (i != m_sheet && !sheets[i].deleted && sheets[i].state == jp::office::sheet::SheetState::Visible && sheets[i].kind == jp::office::sheet::SheetKind::Worksheet) {
            names << sheets[i].name;
            places << i;
        }
    const bool canMakeSheet = !fromChartSheet;
    if (names.isEmpty() && !canMakeSheet) {
        notice(tr("Move Chart"), tr("There is no other worksheet to move the chart to."));
        return;
    }
    int chosen = 0;
    bool newSheet = false;
    QString sheetName = newChartSheetName();
    if (m_moveChartPlaceAsker) {
        if (!m_moveChartPlaceAsker(names, &newSheet, &sheetName, &chosen)) return;
    } else if (m_moveChartAsker && !names.isEmpty()) {
        if (!m_moveChartAsker(names, &chosen)) return;
    } else {
        QDialog d(this);
        d.setObjectName(QStringLiteral("moveChartDialog"));
        d.setWindowTitle(tr("Move Chart"));
        auto *layout = new QVBoxLayout(&d);
        auto *intro = new QLabel(tr("Choose where to put the chart"), &d);
        layout->addWidget(intro);
        auto *newRadio = new QRadioButton(tr("New sheet:"), &d);
        auto *nameEdit = new QLineEdit(sheetName, &d);
        auto *existingRadio = new QRadioButton(tr("Object in:"), &d);
        auto *combo = new QComboBox(&d);
        combo->addItems(names);
        auto *row1 = new QHBoxLayout;
        row1->addWidget(newRadio);
        row1->addWidget(nameEdit, 1);
        auto *row2 = new QHBoxLayout;
        row2->addWidget(existingRadio);
        row2->addWidget(combo, 1);
        layout->addLayout(row1);
        layout->addLayout(row2);
        newRadio->setEnabled(canMakeSheet);
        nameEdit->setEnabled(canMakeSheet);
        existingRadio->setEnabled(!names.isEmpty());
        combo->setEnabled(!names.isEmpty());
        (canMakeSheet ? newRadio : existingRadio)->setChecked(true);
        connect(newRadio, &QRadioButton::toggled, &d, [=](bool on) {
            nameEdit->setEnabled(on);
            combo->setEnabled(!on);
        });
        nameEdit->setEnabled(canMakeSheet);
        combo->setEnabled(!canMakeSheet);
        auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &d);
        connect(buttons, &QDialogButtonBox::accepted, &d, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &d, &QDialog::reject);
        layout->addWidget(buttons);
        if (m_moveChartShot) m_moveChartShot(&d);
        if (d.exec() != QDialog::Accepted) return;
        newSheet = newRadio->isChecked();
        sheetName = nameEdit->text().trimmed();
        chosen = combo->currentIndex();
    }
    if (newSheet && !canMakeSheet) return;
    if (!newSheet && (chosen < 0 || chosen >= places.size())) return;
    if (newSheet && !m_session->validSheetName(sheetName, -1, &why)) {
        notice(tr("Move Chart"), why);
        return;
    }
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
    if (fromChartSheet) {
        // (a chart sheet's chart fills its page: on a worksheet it has the size of a chart made there)
        anchor.x = anchor.y = 0;
        anchor.cx = 9525 * 480;
        anchor.cy = 9525 * 288;
    }
    // everything a step could refuse is asked before the first step is made: a refusal leaves no step to undo, redo, or save
    if (newSheet && !m_session->canChangeSheetList(true, &why)) {
        notice(tr("Move Chart"), why);
        return;
    }
    if (fromChartSheet && !m_session->canChangeSheetList(false, &why)) {
        notice(tr("Move Chart"), why);
        return;
    }
    if (!m_session->allows(m_sheet, crunch::EditSession::Allow::Objects, &why) || (!newSheet && !m_session->allows(places[chosen], crunch::EditSession::Allow::Objects, &why)) ||
        !m_objects->canMakeChart(m_sheet, chart, &why)) {
        notice(tr("Move Chart"), why);
        return;
    }
    QUndoStack *stack = m_session->undoStack();
    stack->beginMacro(tr("Move Chart"));
    int target = -1;
    if (newSheet) {
        const crunch::EditSession::Outcome made = m_session->insertChartSheet(-1, sheetName, &target);
        if (!made.ok) {
            stack->endMacro();
            stack->undo();
            notice(tr("Move Chart"), made.problem);
            return;
        }
        // (a chart sheet's chart fills its page: its place is the page's)
        anchor.x = anchor.y = 0;
        anchor.cx = 9525 * 640;
        anchor.cy = 9525 * 480;
    } else {
        target = places[chosen];
        anchor = m_objects->anchorKeepingSize(target, anchor);
    }
    const int newId = m_objects->makeChart(target, chart, anchor, &why);
    if (newId < 0) {
        stack->endMacro();
        stack->undo();
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
    const int from = m_sheet;
    const crunch::EditSession::Outcome o = m_session->changeObjects({away, here}, tr("Move Chart"));
    if (!o.ok) {
        m_objects->discardMade(target, newId);
        stack->endMacro();
        stack->undo();
        notice(tr("Move Chart"), o.problem);
        return;
    }
    if (fromChartSheet) {
        // (a chart sheet with no chart is of no use: it goes)
        const crunch::EditSession::Outcome gone = m_session->deleteSheet(from);
        if (!gone.ok) setStatus(gone.problem);
    }
    stack->endMacro();
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
    return new ChangeChartTypeDialog(names, seriesKindsOf(chart), [chart, theme](const QVector<SeriesKind> &kinds, const QSize &px) { return chartPicture(withSeriesKinds(chart, kinds, theme), px); }, this);
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

QMenu *CrunchWindow::buildChartElementsMenu(QWidget *owner) { return m_design->elementsMenu(owner); }

QMenu *CrunchWindow::buildChartLooksMenu(QWidget *owner) { return m_design->looksMenu(owner); }

QVector<CrunchWindow::ChartFilterEntry> CrunchWindow::chartFilterEntries() const
{
    QVector<ChartFilterEntry> out;
    for (const ChartDesign::FilterEntry &e : m_design->filterEntries()) out.append({e.name, e.shown});
    return out;
}

bool CrunchWindow::setChartSeriesShown(int entry, bool shown) { return m_design->setSeriesShown(entry, shown); }

QMenu *CrunchWindow::buildChartFiltersMenu(QWidget *owner) { return m_design->filtersMenu(owner); }

// ---- what the shared chart editor (app/chartdesign.h) asks of this window

QWidget *CrunchWindow::chartParent() { return this; }

bool CrunchWindow::chartNow(Chart *out)
{
    const int id = pickedObject();
    if (id < 0 || !m_objects || m_objects->kindOf(m_sheet, id) != ObjectKind::Chart) return false;
    *out = m_objects->chartNow(m_sheet, id);
    return true;
}

Theme CrunchWindow::chartTheme() { return m_objects ? m_objects->theme() : Theme(); }

bool CrunchWindow::changeChart(const QString &label, const std::function<void(Chart *)> &edit) { return changePickedChart(label, edit); }

void CrunchWindow::chartNotice(const QString &title, const QString &text, bool warning) { notice(title, text, warning); }

bool CrunchWindow::askChartText(const QString &caption, const QString &prompt, QString *text)
{
    if (m_chartTextAsker) return m_chartTextAsker(caption, text);
    bool ok = false;
    const QString typed = QInputDialog::getText(this, caption, prompt, QLineEdit::Normal, *text, &ok);
    if (ok) *text = typed;
    return ok;
}

QString CrunchWindow::chartCellRef(const QString &typed, bool oneCell, const QString &caption)
{
    if (!m_objects || !m_objects->cells() || !m_loaded || !m_loaded->book) return QString();
    QString ref = oneCell ? typed.trimmed() : typed;
    if (!ref.contains(QLatin1Char('!'))) {
        const QString own = m_loaded->book->info().sheets[m_sheet].name;
        ref = (own.contains(QRegularExpression(QStringLiteral("[^A-Za-z0-9_]"))) ? QStringLiteral("'") + QString(own).replace(QLatin1Char('\''), QStringLiteral("''")) + QStringLiteral("'") : own) + QLatin1Char('!') + ref;
    }
    QVector<ChartBlock> blocks;
    const bool found = chartBlocks(ref, m_sheet, *m_objects->cells(), &blocks) && blocks.size() == 1;
    if (oneCell ? !found || blocks[0].range.row1 != blocks[0].range.row2 || blocks[0].range.col1 != blocks[0].range.col2 : !found) {
        notice(caption, oneCell ? tr("\"%1\" isn't one cell JeffCrunch can find. Write it like =Sheet1!$A$1.").arg(typed) : tr("\"%1\" isn't a range JeffCrunch can find. Write it like =Sheet1!$D$2:$D$4.").arg(ref));
        return QString();
    }
    // (written with $ the way a chart keeps it)
    const auto r = blocks[0].range;
    const int bang = int(ref.lastIndexOf(QLatin1Char('!')));
    ref = ref.left(bang + 1) + QStringLiteral("$") + columnName(r.col1) + QStringLiteral("$") + QString::number(qint64(r.row1) + 1);
    if (!oneCell && (r.row1 != r.row2 || r.col1 != r.col2)) ref += QStringLiteral(":$") + columnName(r.col2) + QStringLiteral("$") + QString::number(qint64(r.row2) + 1);
    return ref;
}

void CrunchWindow::chartReadCells(Chart *chart)
{
    if (m_objects && m_objects->cells()) applyChartCells(chart, m_sheet, *m_objects->cells());
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
        // (Move Chart needs a chart it can move: on a worksheet, a new chart sheet is always a place to go to; on a chart sheet, another worksheet is)
        bool other = false;
        if (chart && m_loaded && m_loaded->book && m_objects->canEditChart(m_sheet, id, nullptr, true)) {
            other = !m_objects->isChartSheet(m_sheet);
            for (int i = 0; i < m_loaded->book->info().sheets.size() && !other; ++i)
                if (i != m_sheet && !m_loaded->book->info().sheets[i].deleted && m_loaded->book->info().sheets[i].kind == jp::office::sheet::SheetKind::Worksheet) other = true;
        }
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
    for (const char *name : {"object.properties", "object.delete"}) act(QString::fromLatin1(name))->setEnabled(picked);
    // (an object from another app is not one of the drawing's: it has no place in their order to bring forward or send back)
    for (const char *name : {"object.front", "object.forward", "object.backward", "object.back"}) act(QString::fromLatin1(name))->setEnabled(picked && !crunch::isEmbedId(id));
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Chart"), chart);
    {
        const ObjectKind kind = picked ? m_objects->kindOf(m_sheet, id) : ObjectKind::Other;
        const bool picture = kind == ObjectKind::Picture, shape = kind == ObjectKind::Shape, line = kind == ObjectKind::Connector;
        for (const char *name : {"object.rotateRight", "object.rotateLeft", "object.flipH", "object.flipV"}) act(QString::fromLatin1(name))->setEnabled(picture || shape || line);
        for (const char *name : {"object.cropSquare", "object.crop43", "object.crop34", "object.crop169", "object.crop916", "object.cropReset", "object.cropFill", "object.cropFit"}) act(QString::fromLatin1(name))->setEnabled(picture);
        act(QStringLiteral("object.editText"))->setEnabled(shape);
        {
            // (Text Direction is for a text box: the one picked shows the way its words run)
            TextDirection now = TextDirection::Horizontal;
            const bool box = shape && pickedTextBoxDirection(&now);
            const std::pair<const char *, TextDirection> directions[] = {{"object.dir.horz", TextDirection::Horizontal}, {"object.dir.r90", TextDirection::Rotate90}, {"object.dir.r270", TextDirection::Rotate270}};
            for (const auto &d : directions) {
                act(QString::fromLatin1(d.first))->setEnabled(box);
                act(QString::fromLatin1(d.first))->setChecked(box && now == d.second);
            }
        }
        act(QStringLiteral("object.compress"))->setEnabled(picture);
        for (const char *name : {"object.changePicture", "object.resetPicture", "object.resetPictureSize"}) act(QString::fromLatin1(name))->setEnabled(picture);
        {
            // (Text Warp bends the words of Text Art: no other shape has words that bend)
            const bool art = shape && pickedIsTextArt();
            for (const TextArtTransform &t : textArtTransforms()) act(QStringLiteral("object.textArt.warp.") + t.id)->setEnabled(art);
        }
        enableShapeStyles(shape);
        if (m_ribbon) {
            m_ribbon->setContextVisible(QStringLiteral("Picture Format"), picture);
            const bool equation = picked && !picture && m_objects->equationOf(m_sheet, id, nullptr, nullptr);
            m_ribbon->setContextVisible(QString::fromLatin1(kEquationContext), equation);
            showEquationTabChoices([this](const QString &name) { return m_actions.value(name); }, equationLinearSetting(), equationSyntaxSetting());
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
            if (!m_chartButtons) m_chartButtons = new ChartButtons(m_design, m_view->viewport(), [this](const QString &name) { return icon(name); }, this);
            m_chartButtons->place(r);
            show = true;
        }
    }
    if (!show && m_chartButtons) m_chartButtons->hide();
}
void CrunchWindow::objectContextMenu(int id, const QPoint &global)
{
    if (crunch::isEmbedId(id)) {
        auto *embedMenu = new QMenu(this);
        embedMenu->setAttribute(Qt::WA_DeleteOnClose);
        embedMenu->setObjectName(QStringLiteral("objectMenu"));
        const int sheet = m_sheet;
        connect(embedMenu->addAction(tr("Open")), &QAction::triggered, this, [this, sheet, id] { editObject(sheet, id); });
        embedMenu->addSeparator();
        embedMenu->addAction(act(QStringLiteral("edit.cut")));
        connect(embedMenu->addAction(icon(QStringLiteral("copy")), tr("Copy")), &QAction::triggered, this, [this] { copyObject(false); });
        embedMenu->addAction(act(QStringLiteral("edit.paste")));
        embedMenu->addAction(act(QStringLiteral("object.delete")));
        embedMenu->popup(global);
        return;
    }
    if (crunch::isControlId(id)) {
        auto *controlMenu = new QMenu(this);
        controlMenu->setAttribute(Qt::WA_DeleteOnClose);
        controlMenu->setObjectName(QStringLiteral("objectMenu"));
        connect(controlMenu->addAction(tr("Format Control…")), &QAction::triggered, this, [this, id] { formatControlDialog(id); });
        connect(controlMenu->addAction(tr("Delete")), &QAction::triggered, this, [this, id] { deleteObject(id); });
        controlMenu->popup(global);
        return;
    }
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
    if (crunch::isEmbedId(id)) return copyEmbed(id, cut);
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
        picture = chartPicture(chart, QSize(int(clip.width), int(clip.height)));
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
    if (kind == ObjectKind::Chart) {
        const QByteArray part = clip.chartXml;
        md = static_cast<QMimeData *>(objects::withObject(md, [part]() -> std::optional<office::ole::EmbeddedObject> { return objects::chartObjectFromPart(part); }));
    }
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
