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
#include "app/textartgallery.h"
#include "core/shapestyles.h"
#include "core/themes.h"
#include "render/renderer.h"
#include "render/textart.h"
#include "text/textprops.h"
#include "app/widgets.h"
#include "render/shapes.h"

#include <QAction>
#include <QActionGroup>
#include <QDockWidget>
#include <QMenu>
#include <QPainter>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
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

bool DeckWindow::changeShape(const QString &shapeId)
{
    if (!shapeDef(shapeId)) return false;
    bool any = false;
    for (const Item *it : m_formatPane->session()->leaves()) any = any || formatops::takesGeometry(*it);
    if (!any) return false;
    m_formatPane->session()->apply(tr("Change Shape"), [&shapeId](Item &it) { formatops::setGeometry(it, shapeId); }, FormatSession::Scope::Leaves);
    return true;
}

namespace {

// A style as a small shape, drawn as the slide draws it: the fill, line, and shadow it gives, and a word in its text color.
void paintShapeStyleSample(QPainter *p, const QRectF &rc, const std::shared_ptr<Document> &scratch, const std::shared_ptr<LayoutCache> &cache, const std::shared_ptr<ShapeItem> &sample)
{
    const QRectF box = rc.adjusted(rc.width() * 0.1, rc.height() * 0.16, -rc.width() * 0.1, -rc.height() * 0.16);
    sample->rect = QRectF(QPointF(0, 0), box.size());
    PaintContext ctx;
    ctx.doc = scratch.get();
    ctx.cache = cache.get();
    ctx.opt.output = true;
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->translate(box.topLeft());
    Renderer::paintItem(p, ctx, *sample);
    p->restore();
}

} // namespace

QVector<GalleryItem> DeckWindow::shapeStyleItems() const
{
    QVector<GalleryItem> v;
    const Document *d = m_ed->doc();
    const MasterPage *master = deck::themeMasterOf(*d, m_ed->currentPage(), m_ed->masterView());
    const std::shared_ptr<office::StyleResolver> resolver = deck::styleResolverOf(*d, master);
    for (const deck::ShapeStyle &s : deck::shapeStyles()) {
        const office::StyleLook look = resolver->look(s.ref, QSizeF(40, 26));
        // A scratch presentation holds each tile's shape and its word, so that the renderer draws it as it would on a slide.
        auto scratch = std::make_shared<Document>();
        scratch->colors = d->colors;
        scratch->slideText = true;
        auto sample = std::make_shared<ShapeItem>();
        sample->shape = QStringLiteral("rect");
        sample->fill = look.fill;
        sample->stroke = look.stroke;
        sample->fx = look.fx;
        sample->insets = QMarginsF(0, 0, 0, 0);
        sample->valign = VAlign::Middle;
        sample->storyId = scratch->createStory(QStringLiteral("Abc"));
        {
            QTextCursor c(scratch->storyDoc(sample->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat cf;
            cf.setFontPointSize(9);
            cf.setProperty(tp::ColorRefP, ColorRef::rgb(look.text.isValid() ? look.text : QColor(Qt::black)).toString());
            cf.setForeground(QBrush(look.text.isValid() ? look.text : QColor(Qt::black)));
            c.mergeCharFormat(cf);
            QTextBlockFormat bf;
            bf.setAlignment(Qt::AlignHCenter);
            c.mergeBlockFormat(bf);
        }
        auto cache = std::make_shared<LayoutCache>();
        v << GalleryItem{s.id, s.name, drawnIcon([scratch, cache, sample](QPainter *p, const QRectF &rc) { paintShapeStyleSample(p, rc, scratch, cache, sample); }), s.group};
    }
    return v;
}

bool DeckWindow::applyShapeStyle(const QString &styleId)
{
    const deck::ShapeStyle style = deck::shapeStyleById(styleId);
    if (!style.ref.on) return false;
    bool any = false;
    for (const Item *it : m_formatPane->session()->leaves()) any = any || deck::takesShapeStyle(*it);
    if (!any) return false;
    Document *d = m_ed->doc();
    const std::shared_ptr<office::StyleResolver> resolver = deck::styleResolverOf(*d, deck::themeMasterOf(*d, m_ed->currentPage(), m_ed->masterView()));
    m_formatPane->session()->apply(tr("Shape Style"), [d, &style, resolver](Item &it) { deck::applyShapeStyle(*d, it, style.ref, *resolver); }, FormatSession::Scope::Leaves);
    return true;
}

// ---- Text Art Styles: the letters of the words in a shape, and Text Art (which is letters in itself)

// Whether what is selected has letters these commands change: words in a shape or a text box (being edited, or not), or Text Art.
bool DeckWindow::lettersSelected() const
{
    if (m_ed->isEditingText()) return true;
    for (const Item *it : m_formatPane->session()->leaves()) {
        switch (it->type()) {
        case ItemType::Text: if (!static_cast<const TextItem *>(it)->storyId.isEmpty()) return true; break;
        case ItemType::Shape: if (!static_cast<const ShapeItem *>(it)->storyId.isEmpty()) return true; break;
        case ItemType::TextArt: return true;
        default: break;
        }
    }
    return false;
}

// Changes the letters: of the words selected while they are edited, else of all the words of the shapes selected (`runs` changes each run's
// format), and of the Text Art selected (`art` changes the object, which is its letters). One step.
void DeckWindow::changeLetters(const QString &label, const std::function<void(QTextCharFormat &)> &runs, const std::function<void(Item &)> &art)
{
    if (!lettersSelected()) return;
    m_ed->beginChange(label);
    m_ed->editCharFormats(runs, label);
    if (art && !m_ed->isEditingText())
        m_formatPane->session()->apply(label, [&art](Item &it) {
            if (it.type() == ItemType::TextArt) art(it);
        }, FormatSession::Scope::Leaves);
    m_ed->endChange();
}

void DeckWindow::fillTextFillMenu(QMenu *menu)
{
    menu->clear();
    const ColorScheme scheme = m_ed->doc()->colors;
    auto *grid = new QWidgetAction(menu);
    grid->setDefaultWidget(colorGridWidget(scheme, [this, menu](const ColorRef &c) {
        menu->close();
        changeLetters(tr("Text Fill"), [c](QTextCharFormat &cf) { formatops::setTextFill(cf, c); }, [c](Item &it) { formatops::setSolid(it.fill, c); });
    }));
    menu->addAction(grid);
    menu->addAction(tr("No Fill"), menu, [this] {
        changeLetters(tr("Text Fill"), [](QTextCharFormat &cf) { formatops::setTextFillNone(cf); }, [](Item &it) { it.fill = Fill(); });
    });
    menu->addAction(icon("palette"), tr("More Fill Colors…"), menu, [this] {
        const QColor c = colorsDialog(this, Qt::black);
        if (!c.isValid()) return;
        const ColorRef color = ColorRef::rgb(c);
        changeLetters(tr("Text Fill"), [color](QTextCharFormat &cf) { formatops::setTextFill(cf, color); }, [color](Item &it) { formatops::setSolid(it.fill, color); });
    });
    menu->addSeparator();
    QMenu *gradient = menu->addMenu(icon("blend"), tr("Gradient"));
    for (int i = 0; i < formatops::gradientCount(); ++i)
        gradient->addAction(formatops::gradientName(i), gradient, [this, scheme, i] {
            changeLetters(tr("Text Fill"), [scheme, i](QTextCharFormat &cf) { formatops::setTextFillGradient(cf, i, scheme); }, [scheme, i](Item &it) { formatops::applyGradient(it.fill, i, scheme); });
        });
}

void DeckWindow::fillTextOutlineMenu(QMenu *menu)
{
    menu->clear();
    const ColorScheme scheme = m_ed->doc()->colors;
    auto *grid = new QWidgetAction(menu);
    grid->setDefaultWidget(colorGridWidget(scheme, [this, menu](const ColorRef &c) {
        menu->close();
        changeLetters(tr("Text Outline"), [c](QTextCharFormat &cf) { formatops::setTextOutline(cf, c); }, [c](Item &it) { formatops::setOutlineColor(it.stroke, c); });
    }));
    menu->addAction(grid);
    menu->addAction(tr("No Outline"), menu, [this] {
        changeLetters(tr("Text Outline"), [](QTextCharFormat &cf) { formatops::setTextOutlineNone(cf); }, [](Item &it) { formatops::setOutlineNone(it.stroke); });
    });
    menu->addAction(icon("palette"), tr("More Outline Colors…"), menu, [this] {
        const QColor c = colorsDialog(this, Qt::black);
        if (!c.isValid()) return;
        const ColorRef color = ColorRef::rgb(c);
        changeLetters(tr("Text Outline"), [color](QTextCharFormat &cf) { formatops::setTextOutline(cf, color); }, [color](Item &it) { formatops::setOutlineColor(it.stroke, color); });
    });
    menu->addSeparator();
    QMenu *weight = menu->addMenu(icon("minus"), tr("Weight"));
    for (double pt : {0.25, 0.5, 0.75, 1.0, 1.5, 2.25, 3.0, 4.5, 6.0})
        weight->addAction(tr("%1 pt").arg(pt), weight, [this, pt] {
            changeLetters(tr("Text Outline"), [pt](QTextCharFormat &cf) { formatops::setTextOutlineWeight(cf, pt); }, [pt](Item &it) {
                if (it.stroke.color.isNone()) it.stroke.color = ColorRef::scheme(Main);
                it.stroke.width = pt;
            });
        });
}

void DeckWindow::fillTextEffectsMenu(QMenu *menu)
{
    menu->clear();
    auto effect = [this](const std::function<void(TextEffects &)> &runs, const std::function<void(Effects &)> &art) {
        changeLetters(tr("Text Effects"), [runs](QTextCharFormat &cf) { formatops::changeTextEffects(cf, runs); }, [art](Item &it) { art(it.fx); });
    };
    QMenu *shadow = menu->addMenu(icon("layers"), tr("Shadow"));
    shadow->addAction(tr("No Shadow"), shadow, [effect] { effect([](TextEffects &fx) { fx.outer.on = false; }, [](Effects &fx) { fx.shadow.on = false; }); });
    shadow->addSeparator();
    for (int i = 0; i < formatpresets::shadowCount(); ++i)
        shadow->addAction(formatpresets::shadowName(i), shadow, [effect, i] { effect([i](TextEffects &fx) { formatpresets::applyTextShadow(fx, i); }, [i](Effects &fx) { formatpresets::applyShadow(fx, i); }); });
    QMenu *reflection = menu->addMenu(icon("flip-vertical-2"), tr("Reflection"));
    reflection->addAction(tr("No Reflection"), reflection, [effect] { effect([](TextEffects &fx) { fx.reflection.on = false; }, [](Effects &fx) { fx.reflection.on = false; }); });
    reflection->addSeparator();
    for (int i = 0; i < formatpresets::reflectionCount(); ++i)
        reflection->addAction(formatpresets::reflectionName(i), reflection, [effect, i] { effect([i](TextEffects &fx) { formatpresets::applyTextReflection(fx, i); }, [i](Effects &fx) { formatpresets::applyReflection(fx, i); }); });
    QMenu *glow = menu->addMenu(icon("sparkles"), tr("Glow"));
    glow->addAction(tr("No Glow"), glow, [effect] { effect([](TextEffects &fx) { fx.glow.on = false; }, [](Effects &fx) { fx.glow.on = false; }); });
    glow->addSeparator();
    const ColorScheme scheme = m_ed->doc()->colors;
    for (int i = 0; i < formatpresets::glowCount(); ++i)
        glow->addAction(formatpresets::glowName(i), glow, [effect, scheme, i] {
            effect([scheme, i](TextEffects &fx) { formatpresets::applyTextGlow(fx, i, ColorRef::scheme(Accent1).resolve(scheme)); },
                   [i](Effects &fx) { formatpresets::applyGlow(fx, i, ColorRef::scheme(Accent1)); });
        });
    // Transform warps Text Art; the words of a shape are not warped.
    bool art = false;
    for (const Item *it : m_formatPane->session()->leaves()) art = art || it->type() == ItemType::TextArt;
    QMenu *transform = menu->addMenu(icon("waves"), tr("Transform"));
    const bool warps = art && !m_ed->isEditingText();
    transform->menuAction()->setEnabled(warps);
    for (const TextArtTransform &t : textArtTransforms()) {
        const QString id = t.id;
        QAction *a = transform->addAction(t.name, transform, [this, id] {
            m_formatPane->session()->apply(tr("Text Effects"), [id](Item &it) {
                if (it.type() == ItemType::TextArt) static_cast<TextArtItem &>(it).transform_ = id;
            }, FormatSession::Scope::Leaves);
        });
        a->setEnabled(warps);
    }
}

QVector<GalleryItem> DeckWindow::textArtStyleItems() const { return textArtGalleryItems(m_ed); }

bool DeckWindow::applyTextStyle(const QString &styleId)
{
    const TextArtStyle *found = nullptr;
    for (const TextArtStyle &st : textArtStyles())
        if (st.id == styleId) found = &st;
    if (!found || !lettersSelected()) return false;
    const ColorScheme scheme = m_ed->doc()->colors;
    const TextArtStyle style = *found;
    changeLetters(tr("Text Art Style"), [&style, &scheme](QTextCharFormat &cf) { applyTextArtStyleToRun(cf, style.id, scheme); }, [&style](Item &it) {
        // The whole style, but for the words, the size, and the place the object has.
        applyTextArtStyle(static_cast<TextArtItem &>(it), style);
    });
    return true;
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
    bool geometry = false;
    for (const Item *it : m_formatPane->session()->leaves()) geometry = geometry || formatops::takesGeometry(*it);
    for (QWidget *w : m_changeShapeButtons) w->setEnabled(geometry);
    bool styled = false;
    for (const Item *it : m_formatPane->session()->leaves()) styled = styled || deck::takesShapeStyle(*it);
    for (QWidget *w : m_shapeStyleGalleries) w->setEnabled(styled);
    for (QWidget *w : m_textArtGalleries) w->setEnabled(lettersSelected());
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
    // Change Shape: the shapes of Insert > Shapes (not its line tools), grouped as there.
    parts.widgets[QStringLiteral("changeShape")] = [this]() -> QWidget * {
        auto *b = new GalleryButton(icon("replace"), tr("Change Shape"), QSize(24, 24), 12, true);
        b->setObjectName(QStringLiteral("changeShape"));
        b->setToolTip(tr("Change Shape"));
        b->setItemsProvider([] {
            QVector<GalleryItem> v;
            for (const auto &s : shapeLibrary()) v << GalleryItem{s.id, s.name, shapeIcon(s.id), s.category};
            return v;
        });
        connect(b, &GalleryButton::activated, this, [this](const QString &id) { changeShape(id); });
        m_changeShapeButtons << b;
        b->setEnabled(false);
        return b;
    };
    // Shape Styles: the Quick Styles gallery, a row for each kind of look and a column for each of the theme's accents.
    parts.widgets[QStringLiteral("shapeStyleGallery")] = [this]() -> QWidget * {
        auto *gal = new Gallery(QSize(46, 34), 6);
        gal->setObjectName(QStringLiteral("shapeStyleGallery"));
        gal->setTitle(tr("Shape Styles"));
        gal->setItemsProvider([this] { return shapeStyleItems(); });
        connect(gal, &Gallery::activated, this, [this](const QString &id) {
            applyShapeStyle(id);
            m_canvas->setFocus();
        });
        m_shapeStyleGalleries << gal;
        gal->setEnabled(false);
        return gal;
    };
    // The menus of Text Art Styles, made again each time they open (the colors are the presentation's now, and Transform is on for Text Art).
    parts.menus[QStringLiteral("textFillMenu")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        connect(m, &QMenu::aboutToShow, m, [this, m] { fillTextFillMenu(m); });
        return m;
    };
    parts.menus[QStringLiteral("textOutlineMenu")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        connect(m, &QMenu::aboutToShow, m, [this, m] { fillTextOutlineMenu(m); });
        return m;
    };
    parts.menus[QStringLiteral("textEffectsMenu")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        connect(m, &QMenu::aboutToShow, m, [this, m] { fillTextEffectsMenu(m); });
        return m;
    };
    // The gallery is Insert > Text Art's, as it is drawn there.
    parts.widgets[QStringLiteral("textArtStyleGallery")] = [this]() -> QWidget * {
        auto *gal = new Gallery(QSize(40, 34), 5);
        gal->setObjectName(QStringLiteral("textArtStyleGallery"));
        gal->setTitle(tr("Text Art Styles"));
        gal->setItemsProvider([this] { return textArtStyleItems(); });
        connect(gal, &Gallery::activated, this, [this](const QString &id) {
            applyTextStyle(id);
            m_canvas->setFocus();
        });
        m_textArtGalleries << gal;
        gal->setEnabled(false);
        return gal;
    };
    parts.launchers[QStringLiteral("textArtStyles")] = [this] { formatSelection(FormatPane::Tab::FillLine); };
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
