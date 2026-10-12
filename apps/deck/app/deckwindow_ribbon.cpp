// JeffDeck's ribbon (its layout is resources/ribbon.json, built by
// ribbonbuilder.cpp): the controls and menus the file names, and the status bar.

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/deckwidgets.h"
#include "app/editor.h"
#include "app/icons.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/widgets.h"
#include "canvas/canvas.h"
#include "core/deck.h"
#include "render/shapes.h"
#include "text/textprops.h"

#include <QAction>
#include <QCheckBox>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QSlider>
#include <QStatusBar>
#include <QToolButton>
#include <QWidgetAction>
#include <cmath>

namespace jp {

void DeckWindow::buildRibbon()
{
    RibbonParts parts;
    parts.action = [this](const QString &id) { return m_actions.value(id); };
    ribbonControlParts(parts);
    ribbonGalleryParts(parts);
    ribbonMenuParts(parts);
    QFile f(QStringLiteral(":/deck/ribbon.json"));
    QByteArray json;
    if (f.open(QIODevice::ReadOnly)) json = f.readAll();
    addShowParts(parts);
    addMediaParts(parts);
    parts.launchers[QStringLiteral("font")] = [this] { act("fmt.fontDialog")->trigger(); };
    parts.launchers[QStringLiteral("paragraph")] = [this] { act("para.dialog")->trigger(); };
    jp::buildRibbon(m_ribbon, json, parts, this, &m_ribbonErrors);
}

// Text, color, and check box controls. Each factory makes a new control and
// enrolls it where refreshUi() keeps it in step with the selection.
void DeckWindow::ribbonControlParts(RibbonParts &parts)
{
    parts.widgets[QStringLiteral("fontCombo")] = [this]() -> QWidget * {
        auto *font = new FontCombo();
        connect(font, &FontCombo::familyChosen, this, [this](const QString &f) {
            m_ed->setFontFamily(f);
            m_canvas->setFocus();
        });
        m_fontCombos << font;
        return font;
    };
    parts.widgets[QStringLiteral("sizeCombo")] = [this]() -> QWidget * {
        auto *size = new SizeCombo();
        connect(size, &SizeCombo::sizeChosen, this, [this](double s) {
            m_ed->setFontSize(s);
            m_canvas->setFocus();
        });
        m_sizeCombos << size;
        return size;
    };

    auto colorBtn = [this](const QString &iconName, const QString &tip, bool allowNone, const QString &noneLabel, std::function<void(const ColorRef &)> apply,
                           const ColorRef &initial) {
        auto *b = new ColorButton(iconName, tip, allowNone, noneLabel);
        b->setCurrent(initial);
        connect(b, &ColorButton::colorPicked, this, [apply, this](const ColorRef &c) {
            apply(c);
            m_canvas->setFocus();
        });
        m_colorButtons << b;
        return b;
    };
    parts.widgets[QStringLiteral("textColor")] = [this, colorBtn]() -> QWidget * {
        QWidget *b = colorBtn("baseline", tr("Font Color"), true, tr("Automatic"), [this](const ColorRef &c) { m_ed->setTextColor(c); }, ColorRef::rgb(QColor(0xC0, 0, 0)));
        b->setObjectName(QStringLiteral("textColor"));
        return b;
    };
    parts.widgets[QStringLiteral("shapeFill")] = [this, colorBtn]() -> QWidget * {
        auto *b = colorBtn("paint-bucket", tr("Shape Fill"), true, tr("No Fill"),
                           [this](const ColorRef &c) { m_ed->forEachSelected(tr("Fill"), [c](Item *it) { it->fill = c.isNone() ? Fill() : Fill::solid(c, it->fill.transparency); }); },
                           ColorRef::scheme(Accent1));
        b->setText(tr("Shape Fill"));
        b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        return b;
    };
    parts.widgets[QStringLiteral("shapeOutline")] = [this, colorBtn]() -> QWidget * {
        auto *b = colorBtn("pen-line", tr("Shape Outline"), true, tr("No Outline"), [this](const ColorRef &c) {
            m_ed->forEachSelected(tr("Outline"), [c](Item *it) {
                if (it->stroke.width <= 0) it->stroke.width = 0.75;
                it->stroke.color = c;
            });
        }, ColorRef::scheme(Main));
        b->setText(tr("Shape Outline"));
        b->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        return b;
    };

    // The View tab's check boxes, each one a command's.
    auto check = [this](const QString &id) -> QWidget * {
        QAction *a = act(id);
        auto *box = new QCheckBox(a->text());
        box->setToolTip(a->toolTip());
        box->setChecked(a->isChecked());
        connect(box, &QCheckBox::clicked, a, [a] { a->trigger(); });
        connect(a, &QAction::toggled, box, [box](bool on) {
            const QSignalBlocker block(box);
            box->setChecked(on);
        });
        return box;
    };
    parts.widgets[QStringLiteral("rulerCheck")] = [check]() -> QWidget * { return check(QStringLiteral("view.rulers")); };
    parts.widgets[QStringLiteral("gridlinesCheck")] = [check]() -> QWidget * { return check(QStringLiteral("view.gridlines")); };
    parts.widgets[QStringLiteral("guidesCheck")] = [check]() -> QWidget * { return check(QStringLiteral("view.guides")); };

    // Insert > Table: a hover grid, then the dialog.
    parts.widgets[QStringLiteral("tableButton")] = [this]() -> QWidget * {
        auto *tableBtn = ribbonButton(new QAction(icon("table"), tr("Table"), this), true, nullptr);
        auto *m = new QMenu(tableBtn);
        auto *grid = new TableGrid();
        auto *wa = new QWidgetAction(m);
        wa->setDefaultWidget(grid);
        m->addAction(wa);
        m->addAction(act("insert.table"));
        connect(grid, &TableGrid::picked, this, [this, m](int rows, int cols) {
            m->close();
            insertTable(rows, cols);
        });
        tableBtn->setMenu(m);
        tableBtn->setPopupMode(QToolButton::InstantPopup);
        tableBtn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        tableBtn->setIconSize(QSize(30, 30));
        tableBtn->setAutoRaise(true);
        tableBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        return tableBtn;
    };

    // Design > Variants: the theme's colors.
    parts.widgets[QStringLiteral("themeColors")] = [this]() -> QWidget * { return new ThemeColors(m_ed); };
}

// Galleries: the button that opens the grid of shapes.
void DeckWindow::ribbonGalleryParts(RibbonParts &parts)
{
    auto shapeItems = []() {
        QVector<GalleryItem> v;
        const QString lines = tr("Lines");
        v << GalleryItem{"tool:line", tr("Line"), icon("minus"), lines} << GalleryItem{"tool:arrow", tr("Arrow"), icon("move-right"), lines}
          << GalleryItem{"tool:double", tr("Double Arrow"), icon("move-horizontal"), lines};
        for (const char *route : {"elbow", "curved"}) {
            const QString r = QLatin1String(route), name = r == QLatin1String("elbow") ? tr("Elbow") : tr("Curved");
            v << GalleryItem{"tool:" + r, tr("Connector: %1").arg(name), lineToolIcon(r), lines}
              << GalleryItem{"tool:" + r + "Arrow", tr("Connector: %1 Arrow").arg(name), lineToolIcon(r + "Arrow"), lines}
              << GalleryItem{"tool:" + r + "Double", tr("Connector: %1 Double-Arrow").arg(name), lineToolIcon(r + "Double"), lines};
        }
        v << GalleryItem{"tool:curve", tr("Curve"), lineToolIcon("curve"), lines} << GalleryItem{"tool:freeform", tr("Freeform: Shape"), lineToolIcon("freeform"), lines}
          << GalleryItem{"tool:scribble", tr("Freeform: Scribble"), lineToolIcon("scribble"), lines};
        for (const auto &s : shapeLibrary()) v << GalleryItem{s.id, s.name, shapeIcon(s.id), s.category};
        return v;
    };
    parts.widgets[QStringLiteral("shapesButton")] = [this, shapeItems]() -> QWidget * {
        auto *b = new GalleryButton(icon("shapes"), tr("Shapes"), QSize(24, 24), 12, true);
        b->setToolTip(tr("Shapes"));
        b->setItemsProvider(shapeItems);
        connect(b, &GalleryButton::activated, this, [this](const QString &id) {
            if (id == "tool:line") m_ed->setTool(Tool::Line);
            else if (id == "tool:arrow") m_ed->setTool(Tool::Arrow);
            else if (id == "tool:double") m_ed->setTool(Tool::DoubleArrow);
            else if (id.startsWith(QLatin1String("tool:elbow")) || id.startsWith(QLatin1String("tool:curved"))) {
                // Connectors: the line tools with an elbow or curved route.
                const QString route = id.startsWith(QLatin1String("tool:elbow")) ? tr("elbow") : tr("curved");
                m_ed->setTool(id.endsWith(QLatin1String("Double")) ? Tool::DoubleArrow : id.endsWith(QLatin1String("Arrow")) ? Tool::Arrow : Tool::Line, route);
            } else if (id == "tool:curve" || id == "tool:freeform" || id == "tool:scribble") {
                m_ed->setTool(Tool::Freeform, id.mid(5));
            } else m_ed->setTool(Tool::Shape, id);
        });
        return b;
    };
}

// Menus the ribbon can't describe in data: the layouts to choose from.
void DeckWindow::ribbonMenuParts(RibbonParts &parts)
{
    auto layoutMenu = [this](QWidget *owner, bool change) {
        auto *menu = new QMenu(owner);
        auto *gallery = new LayoutGallery(m_ed, menu);
        auto *wa = new QWidgetAction(menu);
        wa->setDefaultWidget(gallery);
        menu->addAction(wa);
        if (!change) {
            menu->addSeparator();
            menu->addAction(act("slide.reuse"));
        }
        connect(menu, &QMenu::aboutToShow, gallery, &LayoutGallery::reload);
        connect(gallery, &LayoutGallery::chosen, this, [this, menu, change](const QString &type) {
            menu->close();
            if (change) setLayout(type);
            else newSlide(type);
        });
        return menu;
    };
    parts.menus[QStringLiteral("newSlideLayouts")] = [layoutMenu](QWidget *owner) { return layoutMenu(owner, false); };
    parts.menus[QStringLiteral("changeLayout")] = [layoutMenu](QWidget *owner) { return layoutMenu(owner, true); };
    // Slide Size: the two sizes, one checked.
    parts.menus[QStringLiteral("slideSizes")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        m->addAction(act("design.sizeStandard"));
        m->addAction(act("design.sizeWidescreen"));
        return m;
    };
}

// "Slide 2 of 5", then at the right the Notes button, the views, and the zoom.
void DeckWindow::buildStatusBar()
{
    QStatusBar *sb = statusBar();
    m_slideLabel = new QLabel(this);
    m_slideLabel->setObjectName(QStringLiteral("slideNumber"));
    m_slideLabel->setMinimumWidth(110);
    sb->addWidget(m_slideLabel);

    auto button = [&](const char *id, bool withText) {
        auto *b = new QToolButton();
        b->setDefaultAction(act(id));
        b->setAutoRaise(true);
        b->setIconSize(QSize(16, 16));
        b->setToolButtonStyle(withText ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly);
        sb->addPermanentWidget(b);
        return b;
    };
    button("view.notes", true);
    button("view.normal", false);
    button("view.sorter", false);
    button("view.slideShow", false);
    button("zoom.out", false);
    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setObjectName(QStringLiteral("zoomSlider"));
    m_zoomSlider->setRange(-47, 28);   // 1.05^n: 10% .. 390%
    m_zoomSlider->setFixedWidth(140);
    m_zoomSlider->setAccessibleName(tr("Zoom"));
    m_zoomSlider->setAccessibleDescription(tr("Left and Right arrows zoom out and in."));
    connect(m_zoomSlider, &QSlider::valueChanged, this, [this](int v) {
        m_canvas->zoomToFit(Canvas::Fit::None);
        m_canvas->setZoom(std::pow(1.05, v));
    });
    sb->addPermanentWidget(m_zoomSlider);
    button("zoom.in", false);
    m_zoomLabel = new QLabel(QStringLiteral("100%"));
    m_zoomLabel->setMinimumWidth(44);
    m_zoomLabel->setObjectName(QStringLiteral("zoomPercent"));
    sb->addPermanentWidget(m_zoomLabel);
    button("view.fit", false);
    connect(m_canvas, &Canvas::zoomChanged, this, [this](double z) {
        m_zoomLabel->setText(QStringLiteral("%1%").arg(std::lround(z * 100)));
        const QSignalBlocker block(m_zoomSlider);
        m_zoomSlider->setValue(int(std::lround(std::log(z) / std::log(1.05))));
    });
}

} // namespace jp
