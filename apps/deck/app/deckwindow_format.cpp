// The Format Shape pane and the Shape Format tab: Shape Fill, Shape Outline, and Shape Effects (menus that reach the pane's settings through the
// same functions it uses), the Height and Width boxes, and the buttons that open the pane. The pane itself is libs/page's (app/formatpane.h).

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/editor.h"
#include "app/formatops.h"
#include "app/formatpresets.h"
#include "app/icons.h"
#include "app/paneframe.h"
#include "app/propertyeditors.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"
#include "app/widgets.h"

#include <QAction>
#include <QActionGroup>
#include <QDockWidget>
#include <QMenu>
#include <QWidgetAction>

namespace jp {

namespace {

// The kinds of object the Shape Format tab is for: shapes, text boxes, lines, and what is made of them.
bool takesShapeFormat(const Item &it)
{
    switch (it.type()) {
    case ItemType::Shape:
    case ItemType::Text:
    case ItemType::Line:
    case ItemType::TextArt:
    case ItemType::Group: return true;
    default: return false;
    }
}

} // namespace

void DeckWindow::createShapeFormatActions()
{
    m_formatPane = new FormatPane(m_ed);
    m_formatDock = new QDockWidget(tr("Format Shape"), this);
    m_formatDock->setObjectName(QStringLiteral("formatShapeDock"));
    m_formatDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    m_formatDock->setTitleBarWidget(new QWidget(m_formatDock));   // (the pane has a title and a close button of its own)
    m_formatDock->setWidget(m_formatPane);
    addDockWidget(Qt::RightDockWidgetArea, m_formatDock);
    m_formatDock->hide();
    connect(m_formatPane, &FormatPane::closeRequested, this, [this] { showFormatPane(false); });
    connect(m_formatPane, &FormatPane::escaped, this, [this] { m_canvas->setFocus(Qt::OtherFocusReason); });

    mk("shape.formatPane", tr("Format Pane"), "panel-right", QKeySequence(), [this] { showFormatPane(act("shape.formatPane")->isChecked()); }, true);
    mk("shape.format", tr("Format Shape…"), "paint-bucket", QKeySequence(Qt::CTRL | Qt::Key_1), [this] {
        bool pictures = !m_ed->selection().isEmpty();
        for (const Item *it : m_ed->selectedItems()) pictures = pictures && it->type() == ItemType::Picture;
        formatSelection(pictures ? FormatPane::Tab::Picture : FormatPane::Tab::FillLine);
    });
    mk("shape.altText", tr("Alt Text"), "accessibility", QKeySequence(), [this] {
        formatSelection(FormatPane::Tab::SizeProperties);
        if (PaneSection *s = m_formatPane->frame()->page(int(FormatPane::Tab::SizeProperties))->sectionById(QStringLiteral("altText"))) {
            s->setExpanded(true);
            if (auto *field = s->bodyWidget()->findChild<QWidget *>(QStringLiteral("altDescription"))) field->setFocus(Qt::OtherFocusReason);
        }
    });

    auto sync = [this] { refreshShapeFormat(); };
    connect(m_ed, &Editor::selectionChanged, this, sync);
    connect(m_ed, &Editor::changed, this, sync);
    connect(m_ed, &Editor::pageChanged, this, sync);
    connect(m_ed, &Editor::documentReplaced, this, sync);
    refreshShapeFormat();
}

void DeckWindow::showFormatPane(bool on)
{
    if (!m_formatDock) return;
    m_formatDock->setVisible(on);
    {
        const QSignalBlocker block(act("shape.formatPane"));
        act("shape.formatPane")->setChecked(on);
    }
    Settings::get().setValue(QStringLiteral("view/formatPane"), on);
    if (on) {
        m_formatDock->raise();
        resizeDocks({m_formatDock}, {340}, Qt::Horizontal);
        m_formatPane->reload();
    }
}

void DeckWindow::formatSelection(FormatPane::Tab tab)
{
    showFormatPane(true);
    m_formatPane->showTab(tab);
    m_formatPane->frame()->focusTabs();
}

// The Shape Format tab shows while a shape, a text box, a line, or what is made of them is selected; its size boxes show the first one's size.
void DeckWindow::refreshShapeFormat()
{
    const QVector<Item *> items = m_ed->selectedItems();
    bool shows = !items.isEmpty();
    for (const Item *it : items) shows = shows && takesShapeFormat(*it);
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Drawing Tools"), shows);
    act("shape.format")->setEnabled(!items.isEmpty());
    act("shape.altText")->setEnabled(!items.isEmpty());
    for (MeasureSpin *s : m_shapeSizeBoxes) {
        const bool height = s->objectName() == QLatin1String("shapeHeight");
        const QSignalBlocker block(s);
        s->setEnabled(shows);
        if (shows) s->setPoints(height ? items.first()->rect.height() : items.first()->rect.width());
    }
}

// ---- the menus: each choice is the pane's own change, made through its session (one step that undoes)

void DeckWindow::fillShapeFillMenu(QMenu *menu)
{
    menu->clear();
    const ColorScheme scheme = m_ed->doc()->colors;
    FormatSession *s = m_formatPane->session();
    auto fill = [this, s](const QString &label, std::function<void(Fill &)> change) {
        s->apply(label, [change](Item &it) {
            if (formatops::takesFill(it)) change(it.fill);
        }, FormatSession::Scope::Leaves);
    };
    auto *grid = new QWidgetAction(menu);
    grid->setDefaultWidget(colorGridWidget(scheme, [menu, fill](const ColorRef &c) {
        menu->close();
        fill(tr("Shape Fill"), [c](Fill &f) { formatops::setSolid(f, c); });
    }));
    menu->addAction(grid);
    menu->addAction(tr("No Fill"), menu, [fill] { fill(tr("Shape Fill"), [](Fill &f) { f = Fill(); }); });
    menu->addAction(icon("palette"), tr("More Fill Colors…"), menu, [this, fill] {
        const QColor c = colorsDialog(this, Qt::black);
        if (c.isValid()) fill(tr("Shape Fill"), [c](Fill &f) { formatops::setSolid(f, ColorRef::rgb(c)); });
    });
    menu->addSeparator();
    menu->addAction(icon("image"), tr("Picture…"), menu, [this] { m_formatPane->chooseFillPicture(); });
    QMenu *gradient = menu->addMenu(icon("blend"), tr("Gradient"));
    for (int i = 0; i < formatops::gradientCount(); ++i)
        gradient->addAction(formatops::gradientName(i), gradient, [fill, scheme, i] { fill(tr("Shape Fill"), [scheme, i](Fill &f) { formatops::applyGradient(f, i, scheme); }); });
    menu->addSeparator();
    menu->addAction(icon("sliders-horizontal"), tr("More Fill Options…"), menu, [this] { formatSelection(FormatPane::Tab::FillLine); });
}

void DeckWindow::fillShapeOutlineMenu(QMenu *menu)
{
    menu->clear();
    const ColorScheme scheme = m_ed->doc()->colors;
    FormatSession *s = m_formatPane->session();
    auto line = [s](const QString &label, std::function<void(Stroke &)> change) {
        s->apply(label, [change](Item &it) {
            if (formatops::takesLine(it)) change(it.stroke);
        }, FormatSession::Scope::Leaves);
    };
    auto *grid = new QWidgetAction(menu);
    grid->setDefaultWidget(colorGridWidget(scheme, [menu, line](const ColorRef &c) {
        menu->close();
        line(tr("Shape Outline"), [c](Stroke &st) { formatops::setOutlineColor(st, c); });
    }));
    menu->addAction(grid);
    menu->addAction(tr("No Outline"), menu, [line] { line(tr("Shape Outline"), [](Stroke &st) { formatops::setOutlineNone(st); }); });
    menu->addAction(icon("palette"), tr("More Outline Colors…"), menu, [this, line] {
        const QColor c = colorsDialog(this, Qt::black);
        if (c.isValid()) line(tr("Shape Outline"), [c](Stroke &st) { formatops::setOutlineColor(st, ColorRef::rgb(c)); });
    });
    menu->addSeparator();
    // Weight: the one the objects share is checked.
    double shared = -1;
    bool same = true;
    for (const Item *it : m_formatPane->session()->leaves())
        if (formatops::takesLine(*it) && !it->stroke.isNone()) {
            if (shared < 0) shared = it->stroke.width;
            else if (std::abs(shared - it->stroke.width) > 1e-6) same = false;
        }
    QMenu *weight = menu->addMenu(icon("minus"), tr("Weight"));
    for (double pt : {0.25, 0.5, 0.75, 1.0, 1.5, 2.25, 3.0, 4.5, 6.0}) {
        QAction *a = weight->addAction(tr("%1 pt").arg(pt), weight, [line, pt] { line(tr("Shape Outline"), [pt](Stroke &st) {
            if (st.color.isNone()) st.color = ColorRef::scheme(Main);
            st.width = pt;
        }); });
        a->setCheckable(true);
        a->setChecked(same && std::abs(shared - pt) < 1e-6);
    }
    QMenu *dashes = menu->addMenu(icon("ellipsis"), tr("Dashes"));
    for (int d = 0; d <= int(Stroke::LongDashDotDot); ++d)
        dashes->addAction(dashName(Stroke::Dash(d)), dashes, [line, d] { line(tr("Shape Outline"), [d](Stroke &st) { st.dash = Stroke::Dash(d); }); });
    QMenu *arrows = menu->addMenu(icon("move-right"), tr("Arrows"));
    for (int i = 0; i < formatops::arrowStyleCount(); ++i)
        arrows->addAction(formatops::arrowStyleName(i), arrows, [line, i] { line(tr("Shape Outline"), [i](Stroke &st) { formatops::applyArrowStyle(st, i); }); });
    menu->addSeparator();
    menu->addAction(icon("sliders-horizontal"), tr("More Outline Options…"), menu, [this] { formatSelection(FormatPane::Tab::FillLine); });
}

void DeckWindow::fillShapeEffectsMenu(QMenu *menu)
{
    menu->clear();
    FormatSession *s = m_formatPane->session();
    auto effects = [s](const QString &label, std::function<void(Effects &)> change) {
        s->apply(label, [change](Item &it) { change(it.fx); }, FormatSession::Scope::Leaves);
    };
    QMenu *shadow = menu->addMenu(icon("layers"), tr("Shadow"));
    shadow->addAction(tr("No Shadow"), shadow, [effects] { effects(tr("Shape Effects"), [](Effects &fx) { fx.shadow.on = false; }); });
    shadow->addSeparator();
    for (int i = 0; i < formatpresets::shadowCount(); ++i)
        shadow->addAction(formatpresets::shadowName(i), shadow, [effects, i] { effects(tr("Shape Effects"), [i](Effects &fx) { formatpresets::applyShadow(fx, i); }); });
    QMenu *reflection = menu->addMenu(icon("flip-vertical-2"), tr("Reflection"));
    reflection->addAction(tr("No Reflection"), reflection, [effects] { effects(tr("Shape Effects"), [](Effects &fx) { fx.reflection.on = false; }); });
    reflection->addSeparator();
    for (int i = 0; i < formatpresets::reflectionCount(); ++i)
        reflection->addAction(formatpresets::reflectionName(i), reflection, [effects, i] { effects(tr("Shape Effects"), [i](Effects &fx) { formatpresets::applyReflection(fx, i); }); });
    QMenu *glow = menu->addMenu(icon("sparkles"), tr("Glow"));
    glow->addAction(tr("No Glow"), glow, [effects] { effects(tr("Shape Effects"), [](Effects &fx) { fx.glow.on = false; }); });
    glow->addSeparator();
    for (int i = 0; i < formatpresets::glowCount(); ++i)
        glow->addAction(formatpresets::glowName(i), glow, [effects, i] { effects(tr("Shape Effects"), [i](Effects &fx) { formatpresets::applyGlow(fx, i, ColorRef::scheme(Accent1)); }); });
    QMenu *soft = menu->addMenu(icon("circle-dashed"), tr("Soft Edges"));
    soft->addAction(tr("No Soft Edges"), soft, [effects] { effects(tr("Shape Effects"), [](Effects &fx) { fx.softEdge = 0; }); });
    soft->addSeparator();
    for (int i = 0; i < formatpresets::softEdgeCount(); ++i)
        soft->addAction(formatpresets::softEdgeName(i), soft, [effects, i] { effects(tr("Shape Effects"), [i](Effects &fx) { formatpresets::applySoftEdge(fx, i); }); });
    menu->addSeparator();
    menu->addAction(icon("sliders-horizontal"), tr("More Effects Options…"), menu, [this] { formatSelection(FormatPane::Tab::Effects); });
}

void DeckWindow::addShapeFormatParts(RibbonParts &parts)
{
    parts.colors[QStringLiteral("shapeFormat")] = QColor(0x2B, 0x6C, 0xB0);
    // The menus are made again each time they open, so that the colors are the presentation's now and the checked weight is the selection's.
    parts.menus[QStringLiteral("shapeFillMenu")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        connect(m, &QMenu::aboutToShow, m, [this, m] { fillShapeFillMenu(m); });
        return m;
    };
    parts.menus[QStringLiteral("shapeOutlineMenu")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        connect(m, &QMenu::aboutToShow, m, [this, m] { fillShapeOutlineMenu(m); });
        return m;
    };
    parts.menus[QStringLiteral("shapeEffectsMenu")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        connect(m, &QMenu::aboutToShow, m, [this, m] { fillShapeEffectsMenu(m); });
        return m;
    };
    parts.launchers[QStringLiteral("shapeStyles")] = [this] { formatSelection(FormatPane::Tab::FillLine); };
    parts.launchers[QStringLiteral("shapeSize")] = [this] { formatSelection(FormatPane::Tab::SizeProperties); };
    // The size of the object (of the first, when there are several) typed in; every one selected takes it, as in the pane.
    for (const bool height : {true, false}) {
        parts.widgets[QString::fromLatin1(height ? "shapeHeight" : "shapeWidth")] = [this, height]() -> QWidget * {
            auto *s = new MeasureSpin();
            s->setObjectName(QString::fromLatin1(height ? "shapeHeight" : "shapeWidth"));
            s->setToolTip(height ? tr("Shape Height") : tr("Shape Width"));
            s->setAccessibleName(s->toolTip());
            s->setKeyboardTracking(false);
            m_shapeSizeBoxes << s;
            // A number typed is one step; an arrow held down is many changes and one step, which ends when it is let go of.
            auto *hold = new HoldGate(s, HoldGate::SpinKeys);
            connect(hold, &HoldGate::released, this, [this] { m_formatPane->session()->finish(); });
            connect(s, &QDoubleSpinBox::valueChanged, this, [this, height, hold](double v) {
                if (v <= 0) return;
                m_formatPane->session()->apply(tr("Size"), [this, height, v](Item &it) {
                    QSizeF size = it.rect.size();
                    const double now = height ? size.height() : size.width();
                    if (formatops::keepsProportions(it) && now > 0) {
                        if (height) size.setWidth(size.width() * v / now);
                        else size.setHeight(size.height() * v / now);
                    }
                    if (height) size.setHeight(v);
                    else size.setWidth(v);
                    formatops::resizeFrame(m_ed, it, size);
                }, FormatSession::Scope::Selected, !hold->held(), false);
            });
            return s;
        };
    }
}

} // namespace jp
