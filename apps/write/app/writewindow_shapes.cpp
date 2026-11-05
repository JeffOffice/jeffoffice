// JeffWrite's drawn objects: Insert > Shapes, Text Box, and Chart, the Shape Format and Chart Format tabs, the adjustments of the Picture Format tab, and Insert > Equation (the work on the document is
// core/pictureops.h and core/equationops.h).

#include "app/writewindow.h"

#include "app/icons.h"
#include "app/pageview.h"
#include "app/ribbon.h"
#include "app/filedialogs.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"
#include "app/widgets.h"
#include "core/equationops.h"
#include "core/pictureops.h"
#include "office/drawingml.h"
#include "office/drawingml_geometry.h"
#include "render/shapes.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QStandardPaths>
#include <QStatusBar>
#include <QLabel>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QVBoxLayout>

namespace jp {

namespace {

// The icon of a shape of the gallery: the shape library's own where it has one, else the paths of the preset's outline.
QIcon presetIcon(const QString &preset)
{
    if (preset == QLatin1String("line")) return lineToolIcon(QStringLiteral("line"));
    if (preset == QLatin1String("straightConnector1")) return lineToolIcon(QStringLiteral("arrow"));
    const QString library = office::shapeForPreset(preset);
    if (!library.isEmpty() && !office::prefersPresetGeometry(preset)) return shapeIcon(library);
    const QSizeF size(18, 18);
    const office::Geometry geometry = office::presetGeometry(preset, {}, size);
    QPixmap pm(24, 24);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.translate(3, 3);
    p.setPen(QPen(QColor(0x50, 0x50, 0x50), 1.2));
    p.setBrush(QColor(0xDD, 0xE6, 0xF2));
    for (const office::GeometryPath &g : geometry.paths) p.drawPath(g.path);
    return QIcon(pm);
}

// The name a shape has in the gallery's tip: the shape library's name where it has one, else the preset's name with its words apart.
QString presetTitle(const QString &preset)
{
    if (preset == QLatin1String("line")) return QCoreApplication::translate("jp::WriteWindow", "Line");
    if (preset == QLatin1String("straightConnector1")) return QCoreApplication::translate("jp::WriteWindow", "Arrow");
    if (preset == QLatin1String("bentConnector3")) return QCoreApplication::translate("jp::WriteWindow", "Elbow Arrow");
    if (preset == QLatin1String("curvedConnector3")) return QCoreApplication::translate("jp::WriteWindow", "Curved Arrow");
    const QString library = office::shapeForPreset(preset);
    if (!library.isEmpty())
        if (const ShapeDef *def = shapeDef(library)) return def->name;
    QString out;
    for (const QChar ch : preset) {
        if (ch.isUpper() && !out.isEmpty()) out += QLatin1Char(' ');
        out += out.isEmpty() ? ch.toUpper() : ch;
    }
    return out;
}

QString groupTitle(const QString &name)
{
    static const char *const names[] = {QT_TRANSLATE_NOOP("jp::WriteWindow", "Lines"), QT_TRANSLATE_NOOP("jp::WriteWindow", "Rectangles"), QT_TRANSLATE_NOOP("jp::WriteWindow", "Basic Shapes"),
                                        QT_TRANSLATE_NOOP("jp::WriteWindow", "Block Arrows"), QT_TRANSLATE_NOOP("jp::WriteWindow", "Equation Shapes"),
                                        QT_TRANSLATE_NOOP("jp::WriteWindow", "Flowchart"), QT_TRANSLATE_NOOP("jp::WriteWindow", "Stars and Banners"),
                                        QT_TRANSLATE_NOOP("jp::WriteWindow", "Callouts")};
    for (const char *n : names)
        if (name == QLatin1String(n)) return QCoreApplication::translate("jp::WriteWindow", n);
    return name;
}

QVector<GalleryItem> shapeItems(bool replaceable)
{
    QVector<GalleryItem> v;
    for (const write::ShapeGroup &g : write::shapeGallery()) {
        if (replaceable && g.name == QLatin1String("Lines")) continue;
        for (const QString &preset : g.presets) v << GalleryItem{preset, presetTitle(preset), presetIcon(preset), groupTitle(g.name)};
    }
    return v;
}

// One look of the Shape Styles gallery: a fill and a line, as RRGGBB (empty for none), and the line's weight.
struct ShapeStyle {
    const char *name, *fill, *line;
    double weight;
    const char *dash;
};

const ShapeStyle kShapeStyles[] = {
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Blue"), "3B78B8", "27547F", 1.5, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Red"), "C0504D", "7F2F2D", 1.5, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Green"), "6AA84F", "3F6B2D", 1.5, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Orange"), "F1A33B", "A66A12", 1.5, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Purple"), "7E5AA8", "4E3470", 1.5, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Dark"), "404040", "202020", 1.5, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Pale Blue"), "E8EEF6", "3B78B8", 1.5, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Plain"), "FFFFFF", "404040", 0.75, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Note"), "FFF4C2", "C9A227", 1, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Outline"), "", "3B78B8", 2.25, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Dashed Outline"), "", "404040", 1.5, "dash"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Fill Only"), "27547F", "", 0, "solid"},
};

// The looks of the Picture Styles gallery: a border (the fill is not used).
const ShapeStyle kPictureStyles[] = {
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "No Border"), "", "", 0, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Thin Black Frame"), "", "000000", 0.75, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Black Frame"), "", "000000", 2.25, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Gray Frame"), "", "808080", 4.5, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Blue Frame"), "", "3B78B8", 3, "solid"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Dashed Frame"), "", "404040", 2.25, "dash"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "Dotted Frame"), "", "404040", 2.25, "sysDot"},
    {QT_TRANSLATE_NOOP("jp::WriteWindow", "White Frame"), "", "FFFFFF", 6, "solid"},
};

QIcon swatchIcon(const ShapeStyle &s)
{
    QPixmap pm(56, 34);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    const bool hasFill = s.fill[0] != 0, hasLine = s.line[0] != 0;
    p.setBrush(hasFill ? QBrush(QColor(QLatin1Char('#') + QLatin1String(s.fill))) : QBrush(Qt::NoBrush));
    p.setPen(hasLine ? QPen(QColor(QLatin1Char('#') + QLatin1String(s.line)), std::min(3.0, std::max(1.0, s.weight)), QLatin1String(s.dash) == QLatin1String("dash") ? Qt::DashLine : Qt::SolidLine) : QPen(Qt::NoPen));
    p.drawRoundedRect(QRectF(5, 5, 46, 24), 4, 4);
    return QIcon(pm);
}

const double kWeights[] = {0.25, 0.5, 0.75, 1, 1.5, 2.25, 3, 4.5, 6};

QString hexOf(const ColorRef &c, const ColorScheme &scheme)
{
    return c.isNone() ? QString() : c.resolve(scheme).name(QColor::HexRgb).mid(1).toUpper();
}

} // namespace

void WriteWindow::createShapeActions()
{
    static const std::pair<const char *, const char *> names[] = {
        {"simple", QT_TRANSLATE_NOOP("jp::WriteWindow", "Simple Text Box")}, {"plain", QT_TRANSLATE_NOOP("jp::WriteWindow", "Plain Text Box")},
        {"sidebar", QT_TRANSLATE_NOOP("jp::WriteWindow", "Sidebar")},        {"quote", QT_TRANSLATE_NOOP("jp::WriteWindow", "Pull Quote")},
        {"band", QT_TRANSLATE_NOOP("jp::WriteWindow", "Banner")},            {"note", QT_TRANSLATE_NOOP("jp::WriteWindow", "Note")}};
    for (const write::TextBoxDesign &design : write::textBoxDesigns()) {
        QString text = design.name;
        for (const auto &n : names)
            if (design.id == QLatin1String(n.first)) text = QCoreApplication::translate("jp::WriteWindow", n.second);
        mk(QStringLiteral("insert.textBox.") + design.id, text, "text-cursor-input", QKeySequence(), [this, design] {
            if (m_doc) m_view->insertTextBoxObject(design);
            m_view->setFocus(Qt::OtherFocusReason);
        });
    }
    mk("shape.editText", tr("Edit Text…"), "square-pen", QKeySequence(), [this] { editShapeText(); });
    mk("shape.rotate.right", tr("Rotate Right 90°"), "rotate-cw", QKeySequence(), [this] { turnSelectedShape(90, true); });
    mk("shape.rotate.left", tr("Rotate Left 90°"), "rotate-ccw", QKeySequence(), [this] { turnSelectedShape(-90, true); });
    mk("shape.rotate.none", tr("No Rotation"), "refresh-ccw", QKeySequence(), [this] { turnSelectedShape(0, false); });
    struct Align {
        const char *id, *text, *icon;
        write::ObjectAlign align;
    };
    for (const Align &a : {Align{"left", QT_TRANSLATE_NOOP("jp::WriteWindow", "Align Left"), "align-start-vertical", write::ObjectAlign::Left},
                           Align{"center", QT_TRANSLATE_NOOP("jp::WriteWindow", "Align Center"), "align-center-vertical", write::ObjectAlign::Center},
                           Align{"right", QT_TRANSLATE_NOOP("jp::WriteWindow", "Align Right"), "align-end-vertical", write::ObjectAlign::Right},
                           Align{"top", QT_TRANSLATE_NOOP("jp::WriteWindow", "Align Top"), "align-start-horizontal", write::ObjectAlign::Top},
                           Align{"middle", QT_TRANSLATE_NOOP("jp::WriteWindow", "Align Middle"), "align-center-horizontal", write::ObjectAlign::Middle},
                           Align{"bottom", QT_TRANSLATE_NOOP("jp::WriteWindow", "Align Bottom"), "align-end-horizontal", write::ObjectAlign::Bottom}}) {
        const int which = int(a.align);
        mk(QStringLiteral("arrange.align.") + QLatin1String(a.id), QCoreApplication::translate("jp::WriteWindow", a.text), a.icon, QKeySequence(), [this, which] { alignSelectedObject(which); });
    }
    mk("arrange.alignToPage", tr("Align to Page"), "file", QKeySequence(), [] {}, true);
    struct Gradient {
        const char *id, *text;
        double angle;
        bool twoColors;
    };
    for (const Gradient &g : {Gradient{"down", QT_TRANSLATE_NOOP("jp::WriteWindow", "Linear Down"), 90, false}, Gradient{"up", QT_TRANSLATE_NOOP("jp::WriteWindow", "Linear Up"), 270, false},
                              Gradient{"right", QT_TRANSLATE_NOOP("jp::WriteWindow", "Linear Right"), 0, false}, Gradient{"left", QT_TRANSLATE_NOOP("jp::WriteWindow", "Linear Left"), 180, false},
                              Gradient{"diagonal", QT_TRANSLATE_NOOP("jp::WriteWindow", "Linear Diagonal"), 45, false}, Gradient{"sunset", QT_TRANSLATE_NOOP("jp::WriteWindow", "Sunset"), 90, true}}) {
        const double angle = g.angle;
        const bool two = g.twoColors;
        mk(QStringLiteral("shape.gradient.") + QLatin1String(g.id), QCoreApplication::translate("jp::WriteWindow", g.text), "blend", QKeySequence(), [this, angle, two] { gradientSelectedShape(angle, two); });
    }
    for (const double w : kWeights) {
        const double weight = w;
        mk(QStringLiteral("shape.weight.%1").arg(w), tr("%1 pt").arg(w), "minus", QKeySequence(), [this, weight] {
            const int at = m_view->selectedPicture();
            if (!m_doc || at < 0) return;
            const write::PictureInfo info = write::pictureAt(*m_doc, m_doc->story(), at);
            // (A shape with no line gets one in the weight asked for, in the color it had or a dark gray.)
            const QString color = info.lineColor.isEmpty() ? QStringLiteral("404040") : info.lineColor;
            m_view->changeSelectedPicture([=](write::WriteDoc &d, int position) { return write::setShapeLine(d, position, color, weight, info.lineDash.isEmpty() ? QStringLiteral("solid") : info.lineDash); });
            m_view->setFocus(Qt::OtherFocusReason);
        }, true);
    }
    struct Dash {
        const char *id, *text;
    };
    for (const Dash &d : {Dash{"solid", QT_TRANSLATE_NOOP("jp::WriteWindow", "Solid")}, Dash{"dot", QT_TRANSLATE_NOOP("jp::WriteWindow", "Round Dot")},
                          Dash{"sysDot", QT_TRANSLATE_NOOP("jp::WriteWindow", "Square Dot")}, Dash{"dash", QT_TRANSLATE_NOOP("jp::WriteWindow", "Dash")},
                          Dash{"dashDot", QT_TRANSLATE_NOOP("jp::WriteWindow", "Dash Dot")}, Dash{"lgDash", QT_TRANSLATE_NOOP("jp::WriteWindow", "Long Dash")}}) {
        const QString dash = QLatin1String(d.id);
        mk(QStringLiteral("shape.dash.") + dash, QCoreApplication::translate("jp::WriteWindow", d.text), "minus", QKeySequence(), [this, dash] {
            const int at = m_view->selectedPicture();
            if (!m_doc || at < 0) return;
            const write::PictureInfo info = write::pictureAt(*m_doc, m_doc->story(), at);
            const QString color = info.lineColor.isEmpty() ? QStringLiteral("404040") : info.lineColor;
            const double weight = info.lineWeight > 0 ? info.lineWeight : 1.5;
            m_view->changeSelectedPicture([=](write::WriteDoc &d, int position) { return write::setShapeLine(d, position, color, weight, dash); });
            m_view->setFocus(Qt::OtherFocusReason);
        }, true);
    }
}

namespace {

struct ChartName {
    write::ChartForm form;
    const char *id, *text, *icon;
};

const ChartName kChartNames[] = {
    {write::ChartForm::Column, "column", QT_TRANSLATE_NOOP("jp::WriteWindow", "Column"), "chart-column"},
    {write::ChartForm::ColumnStacked, "columnStacked", QT_TRANSLATE_NOOP("jp::WriteWindow", "Stacked Column"), "chart-column-stacked"},
    {write::ChartForm::ColumnPercent, "columnPercent", QT_TRANSLATE_NOOP("jp::WriteWindow", "100% Stacked Column"), "chart-column-stacked"},
    {write::ChartForm::Bar, "bar", QT_TRANSLATE_NOOP("jp::WriteWindow", "Bar"), "chart-bar"},
    {write::ChartForm::BarStacked, "barStacked", QT_TRANSLATE_NOOP("jp::WriteWindow", "Stacked Bar"), "chart-bar-stacked"},
    {write::ChartForm::Line, "line", QT_TRANSLATE_NOOP("jp::WriteWindow", "Line"), "chart-line"},
    {write::ChartForm::LineMarkers, "lineMarkers", QT_TRANSLATE_NOOP("jp::WriteWindow", "Line with Markers"), "chart-spline"},
    {write::ChartForm::Pie, "pie", QT_TRANSLATE_NOOP("jp::WriteWindow", "Pie"), "chart-pie"},
    {write::ChartForm::Doughnut, "doughnut", QT_TRANSLATE_NOOP("jp::WriteWindow", "Doughnut"), "chart-pie"},
    {write::ChartForm::Area, "area", QT_TRANSLATE_NOOP("jp::WriteWindow", "Area"), "chart-area"},
    {write::ChartForm::AreaStacked, "areaStacked", QT_TRANSLATE_NOOP("jp::WriteWindow", "Stacked Area"), "chart-area"},
    {write::ChartForm::Scatter, "scatter", QT_TRANSLATE_NOOP("jp::WriteWindow", "Scatter"), "chart-scatter"},
};

} // namespace

void WriteWindow::createChartActions()
{
    for (const ChartName &n : kChartNames) {
        const write::ChartForm form = n.form;
        const QString text = QCoreApplication::translate("jp::WriteWindow", n.text);
        mk(QStringLiteral("insert.chart.") + QLatin1String(n.id), text, n.icon, QKeySequence(), [this, form] {
            if (m_doc) m_view->insertChartObject(form);
            m_view->setFocus(Qt::OtherFocusReason);
        });
        mk(QStringLiteral("chart.type.") + QLatin1String(n.id), text, n.icon, QKeySequence(), [this, form] {
            m_view->changeSelectedPicture([form](write::WriteDoc &d, int position) { return write::setChartForm(d, position, form); });
            m_view->setFocus(Qt::OtherFocusReason);
        }, true);
    }
    mk("chart.editData", tr("Edit Data…"), "table-2", QKeySequence(), [this] { editChartData(); });
    mk("chart.title", tr("Chart Title"), "type", QKeySequence(), [this] {
        const int at = m_view->selectedPicture();
        if (!m_doc || at < 0) return;
        const write::ChartInfo info = write::chartAt(*m_doc, at);
        const bool on = act("chart.title")->isChecked();
        const QString text = info.title;
        m_view->changeSelectedPicture([on, text](write::WriteDoc &d, int position) { return write::setChartTitle(d, position, on, text); });
        m_view->setFocus(Qt::OtherFocusReason);
    }, true);
    mk("chart.editTitle", tr("Edit Chart Title…"), "square-pen", QKeySequence(), [this] { editChartTitle(); });
    struct Legend {
        const char *id, *text, *position;
    };
    for (const Legend &l : {Legend{"none", QT_TRANSLATE_NOOP("jp::WriteWindow", "No Legend"), ""}, Legend{"right", QT_TRANSLATE_NOOP("jp::WriteWindow", "Legend at Right"), "r"},
                            Legend{"top", QT_TRANSLATE_NOOP("jp::WriteWindow", "Legend at Top"), "t"}, Legend{"left", QT_TRANSLATE_NOOP("jp::WriteWindow", "Legend at Left"), "l"},
                            Legend{"bottom", QT_TRANSLATE_NOOP("jp::WriteWindow", "Legend at Bottom"), "b"}}) {
        const QString position = QLatin1String(l.position);
        mk(QStringLiteral("chart.legend.") + QLatin1String(l.id), QCoreApplication::translate("jp::WriteWindow", l.text), "list-checks", QKeySequence(), [this, position] {
            m_view->changeSelectedPicture([position](write::WriteDoc &d, int at) { return write::setChartLegend(d, at, position); });
            m_view->setFocus(Qt::OtherFocusReason);
        }, true);
    }
    for (const bool values : {false, true}) {
        const QString which = values ? QStringLiteral("value") : QStringLiteral("category");
        const write::ChartAxisKind kind = values ? write::ChartAxisKind::Value : write::ChartAxisKind::Category;
        mk(QStringLiteral("chart.axis.") + which, values ? tr("Axis of Values") : tr("Axis of Categories"), "chart-line", QKeySequence(), [this, which, kind] {
            const bool on = act(QStringLiteral("chart.axis.") + which)->isChecked();
            m_view->changeSelectedPicture([on, kind](write::WriteDoc &d, int position) { return write::setChartAxis(d, position, kind, on); });
            m_view->setFocus(Qt::OtherFocusReason);
        }, true);
        mk(QStringLiteral("chart.axisTitle.") + which, values ? tr("Title of the Axis of Values…") : tr("Title of the Axis of Categories…"), "type", QKeySequence(), [this, values] { editChartAxisTitle(values); });
    }
    for (const char *id : {"valueMajor", "valueMinor", "categoryMajor"}) {
        const QString which = QLatin1String(id);
        const QString text = which == QLatin1String("valueMajor") ? tr("Major Gridlines of Values") : which == QLatin1String("valueMinor") ? tr("Minor Gridlines of Values") : tr("Major Gridlines of Categories");
        mk(QStringLiteral("chart.grid.") + which, text, "grid-2x2", QKeySequence(), [this, which] {
            const int at = m_view->selectedPicture();
            if (!m_doc || at < 0) return;
            const write::ChartInfo info = write::chartAt(*m_doc, at);
            const bool on = act(QStringLiteral("chart.grid.") + which)->isChecked();
            const bool categories = which == QLatin1String("categoryMajor"), minorOne = which == QLatin1String("valueMinor");
            const bool major = categories ? on : (minorOne ? info.valueGridlines : on), minor = categories ? false : (minorOne ? on : info.valueMinorGridlines && on);
            m_view->changeSelectedPicture([=](write::WriteDoc &d, int position) {
                return write::setChartGridlines(d, position, categories ? write::ChartAxisKind::Category : write::ChartAxisKind::Value, major, minor);
            });
            m_view->setFocus(Qt::OtherFocusReason);
        }, true);
    }
    for (int style = 1; style <= 8; ++style) {
        mk(QStringLiteral("chart.style.%1").arg(style), tr("Style %1").arg(style), "palette", QKeySequence(), [this, style] {
            m_view->changeSelectedPicture([style](write::WriteDoc &d, int position) { return write::setChartStyle(d, position, style); });
            m_view->setFocus(Qt::OtherFocusReason);
        }, true);
    }
    mk("chart.dataLabels", tr("Data Labels"), "text", QKeySequence(), [this] {
        const bool on = act("chart.dataLabels")->isChecked();
        m_view->changeSelectedPicture([on](write::WriteDoc &d, int position) { return write::setChartDataLabels(d, position, on); });
        m_view->setFocus(Qt::OtherFocusReason);
    }, true);
}

void WriteWindow::chartRibbonParts(RibbonParts &parts)
{
    parts.colors[QStringLiteral("chart")] = QColor(0x8A, 0x5A, 0xA8);
    parts.menus[QStringLiteral("insertChart")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        for (const ChartName &n : kChartNames) m->addAction(act(QStringLiteral("insert.chart.") + QLatin1String(n.id)));
        return m;
    };
    parts.menus[QStringLiteral("chartType")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        auto *group = new QActionGroup(m);
        for (const ChartName &n : kChartNames) {
            QAction *a = act(QStringLiteral("chart.type.") + QLatin1String(n.id));
            group->addAction(a);
            m->addAction(a);
        }
        return m;
    };
    parts.menus[QStringLiteral("chartAxes")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        m->addAction(act("chart.axis.category"));
        m->addAction(act("chart.axis.value"));
        m->addSeparator();
        m->addAction(act("chart.axisTitle.category"));
        m->addAction(act("chart.axisTitle.value"));
        return m;
    };
    parts.menus[QStringLiteral("chartGridlines")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        for (const char *id : {"valueMajor", "valueMinor", "categoryMajor"}) m->addAction(act(QStringLiteral("chart.grid.") + QLatin1String(id)));
        return m;
    };
    parts.menus[QStringLiteral("chartStyles")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        auto *group = new QActionGroup(m);
        for (int style = 1; style <= 8; ++style) {
            QAction *a = act(QStringLiteral("chart.style.%1").arg(style));
            group->addAction(a);
            m->addAction(a);
        }
        return m;
    };
    parts.menus[QStringLiteral("chartLegend")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        auto *group = new QActionGroup(m);
        for (const char *id : {"none", "right", "top", "left", "bottom"}) {
            QAction *a = act(QStringLiteral("chart.legend.") + QLatin1String(id));
            group->addAction(a);
            m->addAction(a);
        }
        return m;
    };
}

void WriteWindow::refreshChartUi()
{
    const bool body = m_doc && !m_view->inHeaderFooter() && !m_view->inNote();
    for (const ChartName &n : kChartNames) act(QStringLiteral("insert.chart.") + QLatin1String(n.id))->setEnabled(body);
    const int at = m_doc ? m_view->selectedPicture() : -1;
    const write::PictureInfo pic = at >= 0 ? write::pictureAt(*m_doc, m_doc->story(), at) : write::PictureInfo();
    const bool isChart = pic.valid && pic.kind == write::DrawnKind::Chart;
    m_ribbon->setContextVisible(QStringLiteral("Chart Tools"), isChart, false);
    const write::ChartInfo info = isChart ? write::chartAt(*m_doc, at) : write::ChartInfo();
    const bool editable = isChart && pic.editable && info.valid;
    for (QAction *a : m_actions)
        if (a->objectName().startsWith(QLatin1String("chart."))) a->setEnabled(editable);
    act("chart.editData")->setEnabled(editable && info.dataEditable);
    act("chart.editTitle")->setEnabled(editable);
    act("chart.title")->setChecked(editable && info.titleShown);
    act("chart.dataLabels")->setChecked(editable && info.dataLabels);
    for (const char *id : {"axis.category", "axis.value", "axisTitle.category", "axisTitle.value", "grid.valueMajor", "grid.valueMinor", "grid.categoryMajor"}) act(QStringLiteral("chart.") + QLatin1String(id))->setEnabled(editable && info.hasAxes);
    act("chart.axis.category")->setChecked(editable && info.categoryAxisShown);
    act("chart.axis.value")->setChecked(editable && info.valueAxisShown);
    act("chart.grid.valueMajor")->setChecked(editable && info.valueGridlines);
    act("chart.grid.valueMinor")->setChecked(editable && info.valueMinorGridlines);
    act("chart.grid.categoryMajor")->setChecked(editable && info.categoryGridlines);
    for (int style = 1; style <= 8; ++style) act(QStringLiteral("chart.style.%1").arg(style))->setChecked(editable && info.style == style);
    for (const ChartName &n : kChartNames) act(QStringLiteral("chart.type.") + QLatin1String(n.id))->setChecked(editable && info.form == n.form);
    static const std::pair<const char *, const char *> legends[] = {{"right", "r"}, {"top", "t"}, {"left", "l"}, {"bottom", "b"}};
    act("chart.legend.none")->setChecked(editable && !info.legendShown);
    for (const auto &l : legends) act(QStringLiteral("chart.legend.") + QLatin1String(l.first))->setChecked(editable && info.legendShown && info.legendPosition == QLatin1String(l.second));
}

void WriteWindow::editChartAxisTitle(bool values)
{
    const int at = m_view->selectedPicture();
    if (!m_doc || at < 0) return;
    const write::ChartInfo info = write::chartAt(*m_doc, at);
    if (!info.valid || !info.hasAxes) return;
    QDialog dlg(this);
    dlg.setObjectName(QStringLiteral("chartAxisTitleDialog"));
    dlg.setWindowTitle(values ? tr("Title of the Axis of Values") : tr("Title of the Axis of Categories"));
    auto *layout = new QVBoxLayout(&dlg);
    layout->addWidget(new QLabel(tr("Words for the title of the axis (leave empty for none):"), &dlg));
    auto *edit = new QLineEdit(values ? info.valueAxisTitle : info.categoryAxisTitle, &dlg);
    edit->setObjectName(QStringLiteral("chartAxisTitleEdit"));
    edit->setMinimumWidth(320);
    layout->addWidget(edit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);
    if (dlg.exec() != QDialog::Accepted) return;
    const QString text = edit->text();
    const write::ChartAxisKind kind = values ? write::ChartAxisKind::Value : write::ChartAxisKind::Category;
    m_view->changeSelectedPicture([text, kind](write::WriteDoc &d, int position) { return write::setChartAxisTitle(d, position, kind, text); });
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::editChartTitle()
{
    const int at = m_view->selectedPicture();
    if (!m_doc || at < 0) return;
    const write::ChartInfo info = write::chartAt(*m_doc, at);
    if (!info.valid) return;
    QDialog dlg(this);
    dlg.setObjectName(QStringLiteral("chartTitleDialog"));
    dlg.setWindowTitle(tr("Chart Title"));
    auto *layout = new QVBoxLayout(&dlg);
    layout->addWidget(new QLabel(tr("Words for the title of the chart:"), &dlg));
    auto *edit = new QLineEdit(info.title, &dlg);
    edit->setObjectName(QStringLiteral("chartTitleEdit"));
    edit->setMinimumWidth(320);
    layout->addWidget(edit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);
    if (dlg.exec() != QDialog::Accepted) return;
    const QString text = edit->text();
    m_view->changeSelectedPicture([text](write::WriteDoc &d, int position) { return write::setChartTitle(d, position, true, text); });
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::editChartData()
{
    const int at = m_view->selectedPicture();
    if (!m_doc || at < 0) return;
    const write::ChartInfo info = write::chartAt(*m_doc, at);
    if (!info.valid || !info.dataEditable) return;
    QDialog dlg(this);
    dlg.setObjectName(QStringLiteral("chartDataDialog"));
    dlg.setWindowTitle(tr("Chart Data"));
    auto *layout = new QVBoxLayout(&dlg);
    auto *help = new QLabel(tr("The first column holds the words under the points. The first row holds the names of the series. The other cells are numbers."), &dlg);
    help->setWordWrap(true);
    layout->addWidget(help);
    auto *table = new QTableWidget(info.data.categories.size() + 1, info.data.values.size() + 1, &dlg);
    table->setObjectName(QStringLiteral("chartDataTable"));
    table->horizontalHeader()->setVisible(false);
    table->verticalHeader()->setVisible(false);
    table->setMinimumSize(460, 220);
    for (int s = 0; s < info.data.values.size(); ++s) table->setItem(0, s + 1, new QTableWidgetItem(info.data.seriesNames.value(s)));
    for (int r = 0; r < info.data.categories.size(); ++r) {
        table->setItem(r + 1, 0, new QTableWidgetItem(info.data.categories[r]));
        for (int s = 0; s < info.data.values.size(); ++s) table->setItem(r + 1, s + 1, new QTableWidgetItem(QString::number(info.data.values[s].value(r), 'g', 15)));
    }
    layout->addWidget(table);
    auto *row = new QHBoxLayout();
    auto *addRow = new QPushButton(tr("Add Row"), &dlg);
    auto *addColumn = new QPushButton(tr("Add Column"), &dlg);
    auto *removeRow = new QPushButton(tr("Remove Last Row"), &dlg);
    auto *removeColumn = new QPushButton(tr("Remove Last Column"), &dlg);
    for (QPushButton *b : {addRow, addColumn, removeRow, removeColumn}) row->addWidget(b);
    layout->addLayout(row);
    connect(addRow, &QPushButton::clicked, table, [table] {
        if (table->rowCount() <= write::kMostChartCategories) table->setRowCount(table->rowCount() + 1);
    });
    connect(addColumn, &QPushButton::clicked, table, [table] {
        if (table->columnCount() <= write::kMostChartSeries) table->setColumnCount(table->columnCount() + 1);
    });
    connect(removeRow, &QPushButton::clicked, table, [table] {
        if (table->rowCount() > 2) table->setRowCount(table->rowCount() - 1);
    });
    connect(removeColumn, &QPushButton::clicked, table, [table] {
        if (table->columnCount() > 2) table->setColumnCount(table->columnCount() - 1);
    });
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);
    if (dlg.exec() != QDialog::Accepted) return;
    write::ChartData data;
    for (int c = 1; c < table->columnCount(); ++c) data.seriesNames << (table->item(0, c) ? table->item(0, c)->text() : QString());
    data.values.resize(table->columnCount() - 1);
    for (int r = 1; r < table->rowCount(); ++r) {
        data.categories << (table->item(r, 0) ? table->item(r, 0)->text() : QString());
        for (int c = 1; c < table->columnCount(); ++c) {
            bool ok = false;
            const double v = table->item(r, c) ? table->item(r, c)->text().toDouble(&ok) : 0.0;
            if (!ok && table->item(r, c) && !table->item(r, c)->text().trimmed().isEmpty()) {
                statusBar()->showMessage(tr("Row %1 has a cell that is not a number, so the data was not changed.").arg(r), 8000);
                return;
            }
            data.values[c - 1] << (ok ? v : 0.0);
        }
    }
    m_view->changeSelectedPicture([data](write::WriteDoc &d, int position) { return write::setChartData(d, position, data); });
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::insertEquationDialog()
{
    if (!m_doc || refuseIfReadOnly()) return;
    // An equation next to the caret, or selected by itself, is changed (when it is one that a line can say); any other place puts a new one in.
    int editAt = -1;
    QString editText;
    {
        const QTextCursor now = m_view->cursor();
        const int from = now.selectionStart(), to = now.selectionEnd();
        for (const int candidate : to - from == 1 ? QList<int>{from} : (to == from ? QList<int>{from, from - 1} : QList<int>{})) {
            const QString line = write::equationAt(*m_doc, candidate);
            if (candidate >= 0 && !line.isEmpty()) {
                editAt = candidate;
                editText = line;
                break;
            }
        }
    }
    QDialog dlg(this);
    dlg.setObjectName(QStringLiteral("equationDialog"));
    dlg.setWindowTitle(editAt >= 0 ? tr("Edit Equation") : tr("Insert Equation"));
    auto *layout = new QVBoxLayout(&dlg);
    auto *help = new QLabel(tr("Type the equation in a line. Use a/b for a fraction, x^2 for a power, x_i for a subscript, sqrt(x) for a root, and a backslash and a name for a symbol, such as \\alpha, \\pi, \\infty, \\leq, or \\times. "
                               "Write \\sum_(i=1)^n, \\int_0^1, or \\prod for a sum, an integral, or a product with limits. Use \\hat{x}, \\vec{v}, or \\bar{x} for an accent, \\overline{x} for a bar, "
                               "\\left( ... \\right) for brackets that grow, and \\pmatrix{a & b \\\\ c & d} for a matrix."),
                            &dlg);
    help->setWordWrap(true);
    help->setTextFormat(Qt::PlainText);
    layout->addWidget(help);
    auto *input = new QLineEdit(editText, &dlg);
    input->setObjectName(QStringLiteral("equationInput"));
    input->setMinimumWidth(420);
    layout->addWidget(input);
    auto *preview = new QLabel(&dlg);
    preview->setObjectName(QStringLiteral("equationPreview"));
    preview->setTextFormat(Qt::PlainText);
    preview->setAccessibleName(tr("Equation preview"));
    layout->addWidget(preview);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);
    auto update = [input, preview, buttons] {
        bool ok = false;
        const QByteArray markup = write::equationMarkup(input->text(), &ok);
        preview->setText(ok ? write::equationLine(markup) : QString());
        buttons->button(QDialogButtonBox::Ok)->setEnabled(ok);
    };
    connect(input, &QLineEdit::textChanged, &dlg, update);
    update();
    if (dlg.exec() != QDialog::Accepted) return;
    if (editAt >= 0) {
        if (!write::replaceEquation(*m_doc, editAt, input->text()) && write::equationAt(*m_doc, editAt) != input->text()) statusBar()->showMessage(tr("This equation can't be changed."), 6000);
        m_view->setFocus(Qt::OtherFocusReason);
        return;
    }
    QTextCursor c = m_view->cursor();
    if (write::insertEquation(*m_doc, c, input->text())) m_view->showCursor(c);
    else statusBar()->showMessage(tr("An equation can't be put here."), 6000);
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::createPictureAdjustActions()
{
    createChartActions();
    mk("insert.equation", tr("Equation…"), "square-pi", QKeySequence(QStringLiteral("Alt+=")), [this] { insertEquationDialog(); });
    for (int i = 0; i < write::equationGallery().size(); ++i)
        mk(QStringLiteral("insert.equation.design.%1").arg(i), write::equationGallery().at(i).name, "square-pi", QKeySequence(), [this, i] {
            if (!m_doc || refuseIfReadOnly()) return;
            QTextCursor c = m_view->cursor();
            if (write::insertEquation(*m_doc, c, write::equationGallery().at(i).linear)) m_view->showCursor(c);
            else statusBar()->showMessage(tr("An equation can't be put here."), 6000);
            m_view->setFocus(Qt::OtherFocusReason);
        });
    mk("picture.crop", tr("Crop…"), "crop", QKeySequence(), [this] { cropSelectedPicture(); });
    mk("picture.reset", tr("Reset Picture"), "refresh-ccw", QKeySequence(), [this] {
        m_view->changeSelectedPicture([](write::WriteDoc &d, int position) { return write::resetPicture(d, position); });
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("picture.change", tr("Change Picture…"), "replace", QKeySequence(), [this] { changeSelectedPicture(); });
    static const int steps[] = {-40, -20, 20, 40};
    for (const int v : steps) {
        mk(QStringLiteral("picture.correct.brightness.%1").arg(v), v < 0 ? tr("Brightness: %1%").arg(v) : tr("Brightness: +%1%").arg(v), "sun", QKeySequence(), [this, v] {
            const int at = m_view->selectedPicture();
            if (!m_doc || at < 0) return;
            const write::PictureInfo info = write::pictureAt(*m_doc, m_doc->story(), at);
            m_view->changeSelectedPicture([=](write::WriteDoc &d, int position) { return write::setPictureCorrections(d, position, v / 100.0, info.contrast); });
            m_view->setFocus(Qt::OtherFocusReason);
        }, true);
        mk(QStringLiteral("picture.correct.contrast.%1").arg(v), v < 0 ? tr("Contrast: %1%").arg(v) : tr("Contrast: +%1%").arg(v), "contrast", QKeySequence(), [this, v] {
            const int at = m_view->selectedPicture();
            if (!m_doc || at < 0) return;
            const write::PictureInfo info = write::pictureAt(*m_doc, m_doc->story(), at);
            m_view->changeSelectedPicture([=](write::WriteDoc &d, int position) { return write::setPictureCorrections(d, position, info.brightness, v / 100.0); });
            m_view->setFocus(Qt::OtherFocusReason);
        }, true);
    }
    mk("picture.correct.none", tr("Brightness: 0% (Normal), Contrast: 0% (Normal)"), "circle-slash", QKeySequence(), [this] {
        m_view->changeSelectedPicture([](write::WriteDoc &d, int position) { return write::setPictureCorrections(d, position, 0, 0); });
        m_view->setFocus(Qt::OtherFocusReason);
    });
    struct Recolor {
        const char *id, *text;
        write::PictureRecolor recolor;
    };
    for (const Recolor &r : {Recolor{"none", QT_TRANSLATE_NOOP("jp::WriteWindow", "No Recolor"), write::PictureRecolor::None},
                             Recolor{"gray", QT_TRANSLATE_NOOP("jp::WriteWindow", "Grayscale"), write::PictureRecolor::Grayscale},
                             Recolor{"bw", QT_TRANSLATE_NOOP("jp::WriteWindow", "Black and White"), write::PictureRecolor::BlackAndWhite}}) {
        const write::PictureRecolor recolor = r.recolor;
        mk(QStringLiteral("picture.recolor.") + QLatin1String(r.id), QCoreApplication::translate("jp::WriteWindow", r.text), "palette", QKeySequence(), [this, recolor] {
            m_view->changeSelectedPicture([recolor](write::WriteDoc &d, int position) { return write::setPictureRecolor(d, position, recolor); });
            m_view->setFocus(Qt::OtherFocusReason);
        }, true);
    }
    mk("picture.recolor.washout", tr("Washout"), "palette", QKeySequence(), [this] {
        m_view->changeSelectedPicture([](write::WriteDoc &d, int position) {
            const bool a = write::setPictureRecolor(d, position, write::PictureRecolor::None);
            const bool b = write::setPictureCorrections(d, position, 0.7, -0.7);
            return a || b;
        });
        m_view->setFocus(Qt::OtherFocusReason);
    });
}

void WriteWindow::cropSelectedPicture()
{
    const int at = m_view->selectedPicture();
    if (!m_doc || at < 0) return;
    const write::PictureInfo info = write::pictureAt(*m_doc, m_doc->story(), at);
    QDialog dlg(this);
    dlg.setObjectName(QStringLiteral("cropDialog"));
    dlg.setWindowTitle(tr("Crop Picture"));
    auto *layout = new QVBoxLayout(&dlg);
    auto *label = new QLabel(tr("How much to cut off each edge, as a percent of the whole picture. A negative number leaves room around the picture."), &dlg);
    label->setWordWrap(true);
    layout->addWidget(label);
    auto *form = new QFormLayout();
    QDoubleSpinBox *edges[4];
    const double values[4] = {info.cropLeft, info.cropTop, info.cropRight, info.cropBottom};
    const QString names[4] = {tr("Left:"), tr("Top:"), tr("Right:"), tr("Bottom:")};
    const char *objectNames[4] = {"cropLeft", "cropTop", "cropRight", "cropBottom"};
    for (int i = 0; i < 4; ++i) {
        edges[i] = new QDoubleSpinBox(&dlg);
        edges[i]->setObjectName(QLatin1String(objectNames[i]));
        edges[i]->setRange(-100, 98);
        edges[i]->setDecimals(1);
        edges[i]->setSuffix(QStringLiteral("%"));
        edges[i]->setValue(std::round(values[i] * 1000.0) / 10.0);
        form->addRow(names[i], edges[i]);
    }
    layout->addLayout(form);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);
    if (dlg.exec() != QDialog::Accepted) return;
    const double l = edges[0]->value() / 100, t = edges[1]->value() / 100, r = edges[2]->value() / 100, b = edges[3]->value() / 100;
    if (!m_view->changeSelectedPicture([=](write::WriteDoc &d, int position) { return write::setPictureCrop(d, position, l, t, r, b); }) && l + r > 0.98)
        statusBar()->showMessage(tr("That cuts off all of the picture."), 6000);
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::changeSelectedPicture()
{
    const int at = m_view->selectedPicture();
    if (!m_doc || at < 0) return;
    QString path;
    if (m_pictureChooser) {
        path = m_pictureChooser();
    } else {
        const QString dir = Settings::get().value(QStringLiteral("dirs/pictures"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
        const QString filter = tr("Pictures (*.png *.jpg *.jpeg *.gif *.bmp *.tif *.tiff *.webp *.svg)") + QStringLiteral(";;") + tr("All Files (*)");
        const QStringList paths = askPicturePaths(this, tr("Change Picture"), dir, filter, false, nullptr);
        if (paths.isEmpty()) return;
        Settings::get().setValue(QStringLiteral("dirs/pictures"), QFileInfo(paths.first()).absolutePath());
        path = paths.first();
    }
    if (path.isEmpty()) return;
    QByteArray bytes;
    const write::PictureResult read = write::readPictureFile(path, &bytes);
    if (read != write::PictureResult::Ok) {
        statusBar()->showMessage(read == write::PictureResult::TooLarge ? tr("That picture is too large to put in a document.") : tr("%1 can't be opened as a picture.").arg(QFileInfo(path).fileName()), 8000);
        return;
    }
    m_doc->pictureNotice.clear();
    const bool changed = m_view->changeSelectedPicture([bytes](write::WriteDoc &d, int position) { return write::changePictureFile(d, position, bytes) == write::PictureResult::Ok; });
    if (!changed && m_doc->pictureNotice.isEmpty()) statusBar()->showMessage(tr("%1 can't be opened as a picture.").arg(QFileInfo(path).fileName()), 8000);
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::shapeRibbonParts(RibbonParts &parts)
{
    chartRibbonParts(parts);
    parts.menus[QStringLiteral("pictureCorrections")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        for (const char *kind : {"brightness", "contrast"}) {
            for (const int v : {-40, -20, 20, 40}) m->addAction(act(QStringLiteral("picture.correct.%1.%2").arg(QLatin1String(kind)).arg(v)));
            m->addSeparator();
        }
        m->addAction(act("picture.correct.none"));
        return m;
    };
    parts.menus[QStringLiteral("pictureColor")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        auto *group = new QActionGroup(m);
        for (const char *id : {"none", "gray", "bw"}) {
            QAction *a = act(QStringLiteral("picture.recolor.") + QLatin1String(id));
            group->addAction(a);
            m->addAction(a);
        }
        m->addAction(act("picture.recolor.washout"));
        return m;
    };
    parts.widgets[QStringLiteral("pictureStyles")] = [this]() -> QWidget * {
        auto *g = new Gallery(QSize(60, 38), 6);
        g->setTitle(tr("Picture Styles"));
        g->setItemsProvider([] {
            QVector<GalleryItem> items;
            int i = 0;
            for (const ShapeStyle &s : kPictureStyles) items << GalleryItem{QString::number(i++), QCoreApplication::translate("jp::WriteWindow", s.name), swatchIcon(s), QString()};
            return items;
        });
        g->reload();
        connect(g, &Gallery::activated, this, [this](const QString &id) {
            const int i = id.toInt();
            if (i < 0 || i >= int(sizeof(kPictureStyles) / sizeof(kPictureStyles[0]))) return;
            const ShapeStyle s = kPictureStyles[i];
            m_view->changeSelectedPicture([s](write::WriteDoc &d, int position) { return write::setShapeLine(d, position, QString::fromLatin1(s.line), s.weight, QString::fromLatin1(s.dash)); });
            m_view->setFocus(Qt::OtherFocusReason);
        });
        return g;
    };
    parts.widgets[QStringLiteral("pictureBorder")] = [this]() -> QWidget * {
        auto *b = new ColorButton(QStringLiteral("pencil-line"), tr("Picture Border"), true, tr("No Outline"));
        b->setCurrent(ColorRef::none());
        connect(b, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
            const int at = m_view->selectedPicture();
            if (!m_doc || at < 0) return;
            const write::PictureInfo info = write::pictureAt(*m_doc, m_doc->story(), at);
            const QString hex = hexOf(c, m_doc->flow().env.colors);
            const double weight = info.lineWeight > 0 ? info.lineWeight : 1.5;
            const QString dash = info.lineDash.isEmpty() ? QStringLiteral("solid") : info.lineDash;
            m_view->changeSelectedPicture([=](write::WriteDoc &d, int position) { return write::setShapeLine(d, position, hex, weight, dash); });
            m_view->setFocus(Qt::OtherFocusReason);
        });
        return b;
    };
    parts.colors[QStringLiteral("shape")] = QColor(0x2F, 0x8F, 0x83);
    parts.menus[QStringLiteral("shapeWeight")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        auto *group = new QActionGroup(m);
        for (const double w : kWeights) {
            QAction *a = act(QStringLiteral("shape.weight.%1").arg(w));
            group->addAction(a);
            m->addAction(a);
        }
        return m;
    };
    parts.menus[QStringLiteral("shapeDash")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        auto *group = new QActionGroup(m);
        for (const char *id : {"solid", "dot", "sysDot", "dash", "dashDot", "lgDash"}) {
            QAction *a = act(QStringLiteral("shape.dash.") + QLatin1String(id));
            group->addAction(a);
            m->addAction(a);
        }
        return m;
    };
    parts.menus[QStringLiteral("equationGallery")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        for (int i = 0; i < write::equationGallery().size(); ++i) m->addAction(act(QStringLiteral("insert.equation.design.%1").arg(i)));
        return m;
    };
    parts.menus[QStringLiteral("objectAlign")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        for (const char *id : {"left", "center", "right"}) m->addAction(act(QStringLiteral("arrange.align.") + QLatin1String(id)));
        m->addSeparator();
        for (const char *id : {"top", "middle", "bottom"}) m->addAction(act(QStringLiteral("arrange.align.") + QLatin1String(id)));
        m->addSeparator();
        m->addAction(act("arrange.alignToPage"));
        return m;
    };
    parts.menus[QStringLiteral("shapeGradient")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        for (const char *id : {"down", "up", "right", "left", "diagonal", "sunset"}) m->addAction(act(QStringLiteral("shape.gradient.") + QLatin1String(id)));
        return m;
    };
    parts.widgets[QStringLiteral("pictureCropShape")] = [this]() -> QWidget * {
        auto *b = new GalleryButton(icon("crop"), tr("Crop to Shape"), QSize(30, 30), 8, true);
        b->setToolTip(tr("Crop to Shape"));
        b->setItemsProvider([] { return shapeItems(true); });
        connect(b, &GalleryButton::activated, this, [this](const QString &preset) {
            m_view->changeSelectedPicture([preset](write::WriteDoc &d, int position) { return write::setPictureCropShape(d, position, preset); });
            m_view->setFocus(Qt::OtherFocusReason);
        });
        return b;
    };
    parts.menus[QStringLiteral("shapeRotate")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        for (const char *id : {"right", "left", "none"}) m->addAction(act(QStringLiteral("shape.rotate.") + QLatin1String(id)));
        return m;
    };
    parts.widgets[QStringLiteral("shapesButton")] = [this]() -> QWidget * {
        auto *b = new GalleryButton(icon("shapes"), tr("Shapes"), QSize(30, 30), 8, true);
        b->setToolTip(tr("Shapes"));
        b->setItemsProvider([] { return shapeItems(false); });
        connect(b, &GalleryButton::activated, this, [this](const QString &preset) {
            if (m_doc) m_view->insertShapeObject(preset);
            m_view->setFocus(Qt::OtherFocusReason);
        });
        return b;
    };
    parts.widgets[QStringLiteral("changeShape")] = [this]() -> QWidget * {
        auto *b = new GalleryButton(icon("shapes"), tr("Change Shape"), QSize(30, 30), 8, true);
        b->setToolTip(tr("Change Shape"));
        b->setItemsProvider([] { return shapeItems(true); });
        connect(b, &GalleryButton::activated, this, [this](const QString &preset) {
            m_view->changeSelectedPicture([preset](write::WriteDoc &d, int position) { return write::setShapePreset(d, position, preset); });
            m_view->setFocus(Qt::OtherFocusReason);
        });
        return b;
    };
    parts.widgets[QStringLiteral("shapeStyles")] = [this]() -> QWidget * {
        auto *g = new Gallery(QSize(60, 38), 6);
        g->setTitle(tr("Shape Styles"));
        g->setItemsProvider([] {
            QVector<GalleryItem> items;
            int i = 0;
            for (const ShapeStyle &s : kShapeStyles) items << GalleryItem{QString::number(i++), QCoreApplication::translate("jp::WriteWindow", s.name), swatchIcon(s), QString()};
            return items;
        });
        g->reload();
        connect(g, &Gallery::activated, this, [this](const QString &id) {
            const int i = id.toInt();
            if (i < 0 || i >= int(sizeof(kShapeStyles) / sizeof(kShapeStyles[0]))) return;
            const ShapeStyle s = kShapeStyles[i];
            m_view->changeSelectedPicture([s](write::WriteDoc &d, int position) {
                const bool fill = write::setShapeFill(d, position, QString::fromLatin1(s.fill));
                const bool line = write::setShapeLine(d, position, QString::fromLatin1(s.line), s.weight, QString::fromLatin1(s.dash));
                return fill || line;
            });
            m_view->setFocus(Qt::OtherFocusReason);
        });
        return g;
    };
    parts.widgets[QStringLiteral("shapeFill")] = [this]() -> QWidget * {
        auto *b = new ColorButton(QStringLiteral("paint-bucket"), tr("Shape Fill"), true, tr("No Fill"));
        b->setCurrent(ColorRef::none());
        connect(b, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
            if (!m_doc) return;
            const QString hex = hexOf(c, m_doc->flow().env.colors);
            m_view->changeSelectedPicture([hex](write::WriteDoc &d, int position) { return write::setShapeFill(d, position, hex); });
            m_view->setFocus(Qt::OtherFocusReason);
        });
        return b;
    };
    parts.widgets[QStringLiteral("shapeOutline")] = [this]() -> QWidget * {
        auto *b = new ColorButton(QStringLiteral("pencil-line"), tr("Shape Outline"), true, tr("No Outline"));
        b->setCurrent(ColorRef::none());
        connect(b, &ColorButton::colorPicked, this, [this](const ColorRef &c) {
            const int at = m_view->selectedPicture();
            if (!m_doc || at < 0) return;
            const write::PictureInfo info = write::pictureAt(*m_doc, m_doc->story(), at);
            const QString hex = hexOf(c, m_doc->flow().env.colors);
            const double weight = info.lineWeight > 0 ? info.lineWeight : 1.5;
            const QString dash = info.lineDash.isEmpty() ? QStringLiteral("solid") : info.lineDash;
            m_view->changeSelectedPicture([=](write::WriteDoc &d, int position) { return write::setShapeLine(d, position, hex, weight, dash); });
            m_view->setFocus(Qt::OtherFocusReason);
        });
        return b;
    };
    for (int which = 0; which < 2; ++which) {
        parts.widgets[which == 0 ? QStringLiteral("shapeHeight") : QStringLiteral("shapeWidth")] = [this, which]() -> QWidget * {
            auto *spin = new MeasureSpin();
            spin->setObjectName(which == 0 ? QStringLiteral("shapeHeightField") : QStringLiteral("shapeWidthField"));
            spin->setAccessibleName(which == 0 ? tr("Shape height") : tr("Shape width"));
            m_shapeFields << spin;
            connect(spin, &QDoubleSpinBox::editingFinished, this, [this, spin, which] {
                const int at = m_view->selectedPicture();
                if (m_updatingFields || !m_doc || at < 0) return;
                const write::PictureInfo now = write::pictureAt(*m_doc, m_doc->story(), at);
                if (!now.valid) return;
                double w = now.width, h = now.height;
                (which == 0 ? h : w) = spin->value();
                if (std::abs(w - now.width) < 0.005 && std::abs(h - now.height) < 0.005) return;
                m_view->changeSelectedPicture([w, h](write::WriteDoc &d, int position) { return write::setPictureSize(d, position, w, h); });
            });
            return spin;
        };
    }
}

void WriteWindow::refreshShapeUi()
{
    const bool body = m_doc && !m_view->inHeaderFooter() && !m_view->inNote();
    for (const write::TextBoxDesign &design : write::textBoxDesigns()) act(QStringLiteral("insert.textBox.") + design.id)->setEnabled(body);
    act("insert.equation")->setEnabled(body);
    for (int i = 0; i < write::equationGallery().size(); ++i) act(QStringLiteral("insert.equation.design.%1").arg(i))->setEnabled(body);
    const int at = m_doc ? m_view->selectedPicture() : -1;
    const write::PictureInfo info = at >= 0 ? write::pictureAt(*m_doc, m_doc->story(), at) : write::PictureInfo();
    const bool shape = info.valid && info.kind != write::DrawnKind::Picture && info.kind != write::DrawnKind::Chart;
    refreshChartUi();
    m_ribbon->setContextVisible(QStringLiteral("Shape Tools"), shape, false);
    const bool editable = shape && info.editable;
    const bool drawn = info.valid && info.editable && info.kind != write::DrawnKind::Chart;
    const bool picture = info.valid && info.kind == write::DrawnKind::Picture && info.editable;
    for (QAction *a : m_actions) {
        const QString name = a->objectName();
        if (name.startsWith(QLatin1String("shape."))) a->setEnabled(editable);
        // (A picture takes a border: the line commands are for it too.)
        if (name.startsWith(QLatin1String("shape.weight.")) || name.startsWith(QLatin1String("shape.dash.")) || name.startsWith(QLatin1String("shape.rotate."))) a->setEnabled(drawn && info.kind != write::DrawnKind::Chart);
        if (name.startsWith(QLatin1String("arrange."))) a->setEnabled(drawn);
        if (name == QLatin1String("picture.crop") || name == QLatin1String("picture.reset") || name == QLatin1String("picture.change") || name.startsWith(QLatin1String("picture.correct.")) ||
            name.startsWith(QLatin1String("picture.recolor.")))
            a->setEnabled(picture);
    }
    if (picture) {
        for (const int v : {-40, -20, 20, 40}) {
            act(QStringLiteral("picture.correct.brightness.%1").arg(v))->setChecked(std::abs(info.brightness - v / 100.0) < 0.01);
            act(QStringLiteral("picture.correct.contrast.%1").arg(v))->setChecked(std::abs(info.contrast - v / 100.0) < 0.01);
        }
        // (There is nothing to take off a picture that is as it came.)
        const bool corrected = std::abs(info.brightness) > 0.000005 || std::abs(info.contrast) > 0.000005;
        act("picture.correct.none")->setEnabled(corrected);
        act("picture.reset")->setEnabled(corrected || info.grayscale || info.blackWhite || info.lineWeight > 0 || std::abs(info.cropLeft) + std::abs(info.cropTop) + std::abs(info.cropRight) + std::abs(info.cropBottom) > 0.000005);
        act("picture.recolor.none")->setChecked(!info.grayscale && !info.blackWhite);
        act("picture.recolor.gray")->setChecked(info.grayscale);
        act("picture.recolor.bw")->setChecked(info.blackWhite);
    }
    act("shape.editText")->setEnabled(editable && write::textBoxText(*m_doc, at).has_value());
    for (const double w : kWeights) act(QStringLiteral("shape.weight.%1").arg(w))->setChecked(drawn && std::abs(info.lineWeight - w) < 0.01);
    for (const char *id : {"solid", "dot", "sysDot", "dash", "dashDot", "lgDash"})
        act(QStringLiteral("shape.dash.") + QLatin1String(id))->setChecked(drawn && (info.lineDash.isEmpty() ? QLatin1String(id) == QLatin1String("solid") : info.lineDash == QLatin1String(id)));
    m_updatingFields = true;
    if (info.valid && (shape || info.kind == write::DrawnKind::Chart))
        for (int i = 0; i + 1 < m_shapeFields.size(); i += 2) {
            m_shapeFields[i]->setPoints(info.height);
            m_shapeFields[i + 1]->setPoints(info.width);
        }
    m_updatingFields = false;
}

void WriteWindow::editShapeText()
{
    const int at = m_view->selectedPicture();
    if (!m_doc || at < 0) return;
    const std::optional<QStringList> words = write::textBoxText(*m_doc, at);
    if (!words) return;
    QDialog dlg(this);
    dlg.setObjectName(QStringLiteral("shapeTextDialog"));
    dlg.setWindowTitle(tr("Edit Text"));
    auto *layout = new QVBoxLayout(&dlg);
    auto *label = new QLabel(tr("Type the words. Each line is a paragraph."), &dlg);
    label->setWordWrap(true);
    layout->addWidget(label);
    auto *edit = new QPlainTextEdit(&dlg);
    edit->setObjectName(QStringLiteral("shapeTextEdit"));
    edit->setPlainText(words->join(QLatin1Char('\n')));
    edit->setMinimumSize(360, 160);
    layout->addWidget(edit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);
    if (dlg.exec() != QDialog::Accepted) return;
    const QStringList paragraphs = edit->toPlainText().split(QLatin1Char('\n'));
    m_view->changeSelectedPicture([paragraphs](write::WriteDoc &d, int position) { return write::setTextBoxText(d, position, paragraphs); });
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::alignSelectedObject(int align)
{
    const bool toPage = act("arrange.alignToPage")->isChecked();
    m_view->changeSelectedPicture([align, toPage](write::WriteDoc &d, int position) { return write::alignObject(d, position, write::ObjectAlign(align), toPage); });
    m_view->setFocus(Qt::OtherFocusReason);
}

// Fills the selected shape with a gradient from a lighter to a darker shade of its fill (or of a blue it has none of), or from yellow to red.
void WriteWindow::gradientSelectedShape(double angle, bool twoColors)
{
    const int at = m_view->selectedPicture();
    if (!m_doc || at < 0) return;
    const write::PictureInfo info = write::pictureAt(*m_doc, m_doc->story(), at);
    QColor base(QLatin1Char('#') + (info.fillColor.isEmpty() ? QStringLiteral("3B78B8") : info.fillColor));
    QString from = base.lighter(150).name(QColor::HexRgb).mid(1).toUpper(), to = base.darker(140).name(QColor::HexRgb).mid(1).toUpper();
    if (twoColors) {
        from = QStringLiteral("F7D154");
        to = QStringLiteral("D2452D");
    }
    m_view->changeSelectedPicture([=](write::WriteDoc &d, int position) { return write::setShapeGradient(d, position, from, to, angle); });
    m_view->setFocus(Qt::OtherFocusReason);
}

void WriteWindow::turnSelectedShape(double degrees, bool relative)
{
    const int at = m_view->selectedPicture();
    if (!m_doc || at < 0) return;
    const double now = write::pictureAt(*m_doc, m_doc->story(), at).rotation;
    const double to = relative ? now + degrees : degrees;
    m_view->changeSelectedPicture([to](write::WriteDoc &d, int position) { return write::setShapeRotation(d, position, to); });
    m_view->setFocus(Qt::OtherFocusReason);
}

} // namespace jp
