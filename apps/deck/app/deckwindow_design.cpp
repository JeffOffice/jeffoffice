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
#include "app/themedialogs.h"
#include "app/themegallery.h"
#include "core/themes.h"
#include "io/presentationfile.h"
#include "office/themelib.h"

#include <QAction>
#include <QApplication>
#include <QActionGroup>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QSaveFile>
#include <QStandardPaths>
#include <QWidgetAction>
#include "app/plaintext.h"

namespace jp {

namespace {

office::ThemeColors sharedColors(const deck::ColorSet &set)
{
    office::ThemeColors c;
    c.name = set.name;
    for (int i = 0; i < 12; ++i) c.colors[i] = set.colors[i];
    return c;
}

deck::ColorSet deckColors(const office::ThemeColors &set)
{
    deck::ColorSet c;
    c.name = office::cleanThemeText(set.name);
    for (int i = 0; i < 12; ++i) c.colors[i] = set.colors[i];
    return c;
}

// The first slide master of a presentation, or null.
const MasterPage *firstSlideMasterOf(const Document &d)
{
    for (const auto &m : d.masters)
        if (deck::isSlideMaster(*m)) return m.get();
    return nullptr;
}

} // namespace

void DeckWindow::changeDesign(const QString &label, const std::function<void(Document &)> &fn)
{
    Document *d = m_ed->doc();
    if (!d) return;
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    const OwnEdit own(this);
    m_ed->beginChange(label);
    fn(*d);
    m_ed->endChange();
}

QList<int> DeckWindow::selectedSlides() const
{
    QList<int> slides;
    if (m_view == View::Sorter) slides = m_sorter->selection();
    if (slides.isEmpty()) slides << m_ed->currentPage();
    return slides;
}

bool DeckWindow::applyThemeFrom(const Document &source, bool allSlides)
{
    Document *d = m_ed->doc();
    if (!d) return false;
    QList<int> slides;
    if (!allSlides) {
        slides = selectedSlides();
    }
    m_notes->commit();
    if (m_outline) m_outline->commit();
    m_ed->endTextEdit();
    const OwnEdit own(this);
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
    // Reading the file, and copying its theme in and setting every slide again, take a moment: a wait cursor until the message or the change.
    QApplication::setOverrideCursor(Qt::WaitCursor);
    struct Restore {
        bool on = true;
        void off()
        {
            if (on) QApplication::restoreOverrideCursor();
            on = false;
        }
        ~Restore() { off(); }
    } waiting;
    const QString suffix = QFileInfo(path).suffix().toLower();
    if (suffix == QLatin1String("thmx") || suffix == QLatin1String("docx") || suffix == QLatin1String("dotx")) {
        // A theme file or a document: its theme part gives colors and fonts (there are no slide layouts in it).
        office::Theme theme;
        if (!office::readThemeFile(path, &theme, &why)) {
            waiting.off();
            jp::plainWarning(this, tr("Browse for Themes"), tr("%1 can't be read: %2").arg(QFileInfo(path).fileName(), why.isEmpty() ? tr("The file can't be read.") : why));
            return false;
        }
        waiting.off();
        changeDesign(tr("Theme"), [&theme](Document &doc) {
            deck::applyColors(doc, deckColors(office::themeColorsOf(theme)));
            deck::applyFonts(doc, deck::FontSet{theme.fontSchemeName, theme.majorLatin, theme.minorLatin});
        });
        return true;
    }
    auto source = deck::importAnyPresentationFile(path, &why);
    if (!source) {
        waiting.off();
        jp::plainWarning(this, tr("Browse for Themes"), tr("%1 can't be read: %2").arg(QFileInfo(path).fileName(), why.isEmpty() ? tr("The file can't be read.") : why));
        return false;
    }
    const QString name = QFileInfo(path).fileName();
    switch (deck::themeSourceProblem(*source)) {
    case deck::ThemeSource::NoSlideMaster:
        waiting.off();
        jp::plainWarning(this, tr("Browse for Themes"), tr("%1 has no slide master, so there is no theme to take from it. Nothing was changed.").arg(name));
        return false;
    case deck::ThemeSource::NoLayouts:
        waiting.off();
        jp::plainWarning(this, tr("Browse for Themes"), tr("%1 has a slide master but no slide layouts, so your slides would have nothing to stand on. Nothing was changed.").arg(name));
        return false;
    case deck::ThemeSource::Fine: break;
    }
    return applyThemeFrom(*source, allSlides);
}

void DeckWindow::browseForTheme(bool allSlides)
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Browse for Themes"),
                                                      Settings::get().value(QStringLiteral("dirs/open"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                                                      tr("Themes, Presentations, and Templates (*.thmx *.pptx *.potx *.ppsx *.pptm *.potm *.ppsm *.ppt *.pps *.pot *.docx *.dotx)") + QStringLiteral(";;") + tr("All Files (*)"));
    if (path.isEmpty()) return;
    Settings::get().setValue(QStringLiteral("dirs/open"), QFileInfo(path).absolutePath());
    applyThemeFile(path, allSlides);
}

bool DeckWindow::saveThemeFile(const QString &path)
{
    const Document *d = m_ed->doc();
    const MasterPage *master = d ? firstSlideMasterOf(*d) : nullptr;
    if (!master) return false;
    office::Theme theme;
    if (!office::readTheme(deck::themePartOf(*d, *master), &theme)) return false;
    theme.name = deck::themeOf(*d).name;
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(office::writeThemeFile(theme)) < 0 || !f.commit()) {
        jp::plainWarning(this, tr("Save Current Theme"), tr("%1 can't be written.").arg(QFileInfo(path).fileName()));
        return false;
    }
    return true;
}

void DeckWindow::saveCurrentTheme()
{
    const Document *d = m_ed->doc();
    if (!d) return;
    const QString folder = Settings::get().value(QStringLiteral("dirs/open"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    const QString path = QFileDialog::getSaveFileName(this, tr("Save Current Theme"), folder + QLatin1Char('/') + deck::themeOf(*d).name + QStringLiteral(".thmx"), tr("Theme Files (*.thmx)"));
    if (path.isEmpty()) return;
    saveThemeFile(path.endsWith(QLatin1String(".thmx"), Qt::CaseInsensitive) ? path : path + QStringLiteral(".thmx"));
}

bool DeckWindow::themeChanged() const
{
    const Document *d = m_ed->doc();
    if (!d) return false;
    const deck::DeckTheme now = deck::themeOf(*d);
    for (int i = 0; i < 12; ++i)
        if (now.colors[i] != m_openedTheme.colors[i]) return true;
    return now.headingFont != m_openedTheme.headingFont || now.bodyFont != m_openedTheme.bodyFont;
}

bool DeckWindow::resetTheme()
{
    if (!themeChanged()) return false;
    const deck::DeckTheme was = m_openedTheme;
    changeDesign(tr("Reset Theme"), [&was](Document &doc) {
        deck::ColorSet colors;
        colors.name = was.name;
        for (int i = 0; i < 12; ++i) colors.colors[i] = was.colors[i];
        deck::applyColors(doc, colors);
        deck::applyFonts(doc, deck::FontSet{was.name, was.headingFont, was.bodyFont});
    });
    return true;
}

void DeckWindow::chooseThemeColors(const office::ThemeColors &colors) { chooseColors(deckColors(colors)); }

void DeckWindow::chooseThemeFonts(const office::ThemeFonts &fonts) { chooseFonts(deck::FontSet{office::cleanThemeText(fonts.name), office::cleanThemeText(fonts.heading), office::cleanThemeText(fonts.body)}); }

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

// The Colors menu (here and in Slide Master view): the presentation's own twelve, then the schemes of the built-in themes.
void DeckWindow::fillColorsMenu(QMenu *colors)
{
    colors->clear();
    const Document *d = m_ed->doc();
    if (!d) return;
    colors->addSection(tr("This Presentation"));
    auto *now = new QWidgetAction(colors);
    now->setDefaultWidget(new ThemeColors(m_ed));
    colors->addAction(now);
    const deck::DeckTheme theme = deck::themeOf(*d);
    // The built-in sets: those of JeffDeck's themes, then the ones every app has.
    QVector<office::ThemeColors> builtins;
    QStringList seen;
    for (const deck::ColorSet &set : deck::colorSets()) {
        builtins << sharedColors(set);
        seen << set.name;
    }
    for (const office::ThemeColors &set : office::builtinThemeColors())
        if (!seen.contains(set.name)) builtins << set;
    office::ThemeColors current;
    current.name = theme.name;
    for (int i = 0; i < 12; ++i) current.colors[i] = theme.colors[i];
    jp::ThemeMenuHooks hooks;
    hooks.dialogParent = this;
    for (const office::ThemeColors &set : builtins) hooks.reservedNames << set.name;
    hooks.chooseColors = [this](const office::ThemeColors &c) { chooseThemeColors(c); };
    hooks.reset = [this] { resetTheme(); };
    jp::addThemeColorsEntries(colors, builtins, current, hooks);
    if (QAction *reset = colors->findChild<QAction *>(QStringLiteral("resetThemeColors"))) reset->setEnabled(themeChanged());
}

// The Fonts menu: each scheme's heading and body fonts.
void DeckWindow::fillFontsMenu(QMenu *fonts)
{
    fonts->clear();
    const Document *d = m_ed->doc();
    if (!d) return;
    const deck::DeckTheme theme = deck::themeOf(*d);
    QVector<office::ThemeFonts> builtins;
    for (const deck::FontSet &set : deck::fontSets()) builtins << office::ThemeFonts{set.name, set.heading, set.body};
    jp::ThemeMenuHooks hooks;
    hooks.dialogParent = this;
    for (const office::ThemeFonts &set : builtins) hooks.reservedNames << set.name;
    hooks.chooseFonts = [this](const office::ThemeFonts &f) { chooseThemeFonts(f); };
    hooks.reset = [this] { resetTheme(); };
    jp::addThemeFontsEntries(fonts, builtins, office::ThemeFonts{theme.name, theme.headingFont, theme.bodyFont}, hooks);
    if (QAction *reset = fonts->findChild<QAction *>(QStringLiteral("resetThemeFonts"))) reset->setEnabled(themeChanged());
}

// The Background Styles menu: the twelve of the theme, then Format Background. A choice closes `root`, the menu it is in.
void DeckWindow::fillBackgroundsMenu(QMenu *styles, QMenu *root)
{
    styles->clear();
    auto *grid = new BackgroundStyles(m_ed);
    auto *wa = new QWidgetAction(styles);
    wa->setDefaultWidget(grid);
    styles->addAction(wa);
    styles->addSeparator();
    styles->addAction(act("design.formatBackground"));
    connect(grid, &BackgroundStyles::chosen, this, [this, root](int style) {
        root->close();
        chooseBackgroundStyle(style);
    });
}

// The Themes gallery and the Variants gallery with its menu.
void DeckWindow::addDesignParts(RibbonParts &parts)
{
    Editor *ed = m_ed;
    // New text on a presentation that was opened from a file has the bullets of its layouts, and is light on a dark one, as on one made here.
    connect(ed, &Editor::documentReplaced, this, [this, ed] {
        if (Document *d = ed->doc()) {
            m_openedTheme = deck::themeOf(*d);
            deck::keepTextReadable(*d);
            deck::inheritLayoutLists(*d);
        }
    });
    parts.widgets[QStringLiteral("themeGallery")] = [this, ed]() -> QWidget * {
        auto *gallery = new ThemeGallery(ed);
        connect(gallery, &ThemeGallery::themeChosen, this, [this](int theme, bool all) { applyTheme(theme, all); });
        connect(gallery, &ThemeGallery::browseRequested, this, [this] { browseForTheme(true); });
        connect(gallery, &ThemeGallery::saveRequested, this, [this] { saveCurrentTheme(); });
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
        connect(colors, &QMenu::aboutToShow, colors, [this, colors] { fillColorsMenu(colors); });
        // Fonts: each scheme's heading and body fonts.
        QMenu *fonts = menu->addMenu(icon(QStringLiteral("type")), tr("Fonts"));
        fonts->setObjectName(QStringLiteral("variantFonts"));
        connect(fonts, &QMenu::aboutToShow, fonts, [this, fonts] { fillFontsMenu(fonts); });
        // Background Styles: the twelve of the theme, then Format Background.
        QMenu *styles = menu->addMenu(icon(QStringLiteral("swatch-book")), tr("Background Styles"));
        styles->setObjectName(QStringLiteral("variantBackgrounds"));
        connect(styles, &QMenu::aboutToShow, styles, [this, styles, menu] { fillBackgroundsMenu(styles, menu); });
        gallery->setMenu(menu);
        return gallery;
    };
}

} // namespace jp
