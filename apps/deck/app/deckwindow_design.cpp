// JeffDeck's Design tab: the themes, the variants of the presentation's theme, and the menu of colors, fonts, and background styles.
// Each choice is one step that undoes.

#include "app/deckwindow.h"

#include "app/deckwidgets.h"
#include "app/editor.h"
#include "app/icons.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"
#include "app/slidesorter.h"
#include "app/themegallery.h"
#include "core/themes.h"
#include "io/presentationfile.h"

#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QStandardPaths>
#include <QWidgetAction>

namespace jp {

namespace {

// A scheme's colors for a menu: its two darks and lights, and two accents, in three columns of two.
QIcon swatchIcon(const deck::ColorSet &set)
{
    const int size = 16;
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    static const int slots[6] = {4, 5, 6, 2, 3, 7};
    for (int i = 0; i < 6; ++i) {
        p.setPen(QPen(QColor(0, 0, 0, 70), 1));
        p.setBrush(set.colors[slots[i]]);
        p.drawRect(QRect((i % 3) * 5, 1 + (i / 3) * 7, 5, 6));
    }
    return QIcon(pix);
}

bool sameColors(const deck::ColorSet &set, const deck::DeckTheme &t)
{
    for (int i = 0; i < 10; ++i)
        if (set.colors[i] != t.colors[i]) return false;
    return true;
}

} // namespace

void DeckWindow::changeDesign(const QString &label, const std::function<void(Document &)> &fn)
{
    Document *d = m_ed->doc();
    if (!d) return;
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    m_ed->beginChange(label);
    fn(*d);
    m_ed->endChange();
}

bool DeckWindow::applyThemeFrom(const Document &source, bool allSlides)
{
    Document *d = m_ed->doc();
    if (!d) return false;
    QList<int> slides;
    if (!allSlides) {
        // The slides selected in the Slide Sorter; anywhere else, the slide being edited.
        if (m_view == View::Sorter) slides = m_sorter->selection();
        if (slides.isEmpty()) slides << m_ed->currentPage();
    }
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    m_ed->beginChange(tr("Theme"));
    const bool ok = allSlides ? deck::applyThemeToAll(*d, source) : deck::applyThemeToSlides(*d, source, slides);
    if (ok) m_ed->endChange();
    else m_ed->cancelChange();
    return ok;
}

void DeckWindow::applyTheme(int theme, bool allSlides)
{
    if (theme < 0 || theme >= deck::builtinThemeCount()) return;
    applyThemeFrom(*deck::themePresentation(theme), allSlides);
}

bool DeckWindow::applyThemeFile(const QString &path, bool allSlides)
{
    QString why;
    auto source = deck::importAnyPresentationFile(path, &why);
    if (!source) {
        QMessageBox::warning(this, tr("Browse for Themes"), tr("%1 can't be read: %2").arg(QFileInfo(path).fileName(), why.isEmpty() ? tr("The file can't be read.") : why));
        return false;
    }
    const QString name = QFileInfo(path).fileName();
    switch (deck::themeSourceProblem(*source)) {
    case deck::ThemeSource::NoSlideMaster:
        QMessageBox::warning(this, tr("Browse for Themes"), tr("%1 has no slide master, so there is no theme to take from it. Nothing was changed.").arg(name));
        return false;
    case deck::ThemeSource::NoLayouts:
        QMessageBox::warning(this, tr("Browse for Themes"), tr("%1 has a slide master but no slide layouts, so your slides would have nothing to stand on. Nothing was changed.").arg(name));
        return false;
    case deck::ThemeSource::Fine: break;
    }
    return applyThemeFrom(*source, allSlides);
}

void DeckWindow::browseForTheme(bool allSlides)
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Browse for Themes"),
                                                      Settings::get().value(QStringLiteral("dirs/open"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                                                      tr("Presentations and Templates (*.pptx *.potx *.ppsx *.ppt *.pps *.pot)") + QStringLiteral(";;") + tr("All Files (*)"));
    if (path.isEmpty()) return;
    Settings::get().setValue(QStringLiteral("dirs/open"), QFileInfo(path).absolutePath());
    applyThemeFile(path, allSlides);
}

void DeckWindow::chooseVariant(int variant)
{
    const Document *d = m_ed->doc();
    if (!d) return;
    const QVector<deck::Variant> variants = deck::variantsOf(*d);
    if (variant < 0 || variant >= variants.size()) return;
    const deck::Variant v = variants[variant];
    changeDesign(tr("Variant"), [&v](Document &doc) { deck::applyVariant(doc, v); });
}

void DeckWindow::chooseColors(const deck::ColorSet &colors)
{
    changeDesign(tr("Theme Colors"), [&colors](Document &doc) { deck::applyColors(doc, colors); });
}

void DeckWindow::chooseFonts(const deck::FontSet &fonts)
{
    changeDesign(tr("Theme Fonts"), [&fonts](Document &doc) { deck::applyFonts(doc, fonts); });
}

void DeckWindow::chooseBackgroundStyle(int style)
{
    changeDesign(tr("Background Style"), [style](Document &doc) { deck::applyBackgroundStyle(doc, style); });
}

// The Themes gallery and the Variants gallery with its menu.
void DeckWindow::addDesignParts(RibbonParts &parts)
{
    Editor *ed = m_ed;
    // New text on a presentation that was opened from a file has the bullets of its layouts, and is light on a dark one, as on one made here.
    connect(ed, &Editor::documentReplaced, this, [ed] {
        if (Document *d = ed->doc()) {
            deck::keepTextReadable(*d);
            deck::inheritLayoutLists(*d);
        }
    });
    parts.widgets[QStringLiteral("themeGallery")] = [this, ed]() -> QWidget * {
        auto *gallery = new ThemeGallery(ed);
        connect(gallery, &ThemeGallery::themeChosen, this, [this](int theme, bool all) { applyTheme(theme, all); });
        connect(gallery, &ThemeGallery::browseRequested, this, [this] { browseForTheme(true); });
        connect(ed, &Editor::changed, gallery, &ThemeGallery::reload);
        connect(ed, &Editor::documentReplaced, gallery, &ThemeGallery::reload);
        return gallery;
    };
    parts.widgets[QStringLiteral("themeVariants")] = [this, ed]() -> QWidget * {
        auto *gallery = new VariantGallery(ed);
        connect(gallery, &VariantGallery::variantChosen, this, &DeckWindow::chooseVariant);
        connect(ed, &Editor::changed, gallery, &VariantGallery::reload);
        connect(ed, &Editor::documentReplaced, gallery, &VariantGallery::reload);

        auto *menu = new QMenu(gallery);
        menu->setObjectName(QStringLiteral("variantsMenu"));
        // Colors: the presentation's own twelve, then the schemes of the built-in themes.
        QMenu *colors = menu->addMenu(icon(QStringLiteral("palette")), tr("Colors"));
        colors->setObjectName(QStringLiteral("variantColors"));
        connect(colors, &QMenu::aboutToShow, colors, [this, colors] {
            colors->clear();
            const Document *d = m_ed->doc();
            if (!d) return;
            colors->addSection(tr("This Presentation"));
            auto *now = new QWidgetAction(colors);
            now->setDefaultWidget(new ThemeColors(m_ed));
            colors->addAction(now);
            colors->addSection(tr("Built-In"));
            const deck::DeckTheme theme = deck::themeOf(*d);
            auto *group = new QActionGroup(colors);
            for (const deck::ColorSet &set : deck::colorSets()) {
                QAction *a = colors->addAction(swatchIcon(set), set.name);
                a->setCheckable(true);
                a->setChecked(sameColors(set, theme));
                group->addAction(a);
                connect(a, &QAction::triggered, this, [this, set] { chooseColors(set); });
            }
        });
        // Fonts: each scheme's heading and body fonts.
        QMenu *fonts = menu->addMenu(icon(QStringLiteral("type")), tr("Fonts"));
        fonts->setObjectName(QStringLiteral("variantFonts"));
        connect(fonts, &QMenu::aboutToShow, fonts, [this, fonts] {
            fonts->clear();
            const Document *d = m_ed->doc();
            if (!d) return;
            const deck::DeckTheme theme = deck::themeOf(*d);
            auto *group = new QActionGroup(fonts);
            for (const deck::FontSet &set : deck::fontSets()) {
                QAction *a = fonts->addAction(tr("%1  (%2 / %3)").arg(set.name, set.heading, set.body));
                a->setCheckable(true);
                a->setChecked(set.heading == theme.headingFont && set.body == theme.bodyFont);
                group->addAction(a);
                connect(a, &QAction::triggered, this, [this, set] { chooseFonts(set); });
            }
        });
        // Background Styles: the twelve of the theme, then Format Background.
        QMenu *styles = menu->addMenu(icon(QStringLiteral("swatch-book")), tr("Background Styles"));
        styles->setObjectName(QStringLiteral("variantBackgrounds"));
        connect(styles, &QMenu::aboutToShow, styles, [this, styles, menu] {
            styles->clear();
            auto *grid = new BackgroundStyles(m_ed);
            auto *wa = new QWidgetAction(styles);
            wa->setDefaultWidget(grid);
            styles->addAction(wa);
            styles->addSeparator();
            styles->addAction(act("design.formatBackground"));
            connect(grid, &BackgroundStyles::chosen, this, [this, menu](int style) {
                menu->close();
                chooseBackgroundStyle(style);
            });
        });
        gallery->setMenu(menu);
        return gallery;
    };
}

} // namespace jp
