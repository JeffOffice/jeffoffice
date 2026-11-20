// JeffCrunch's Page Layout > Themes, Colors, and Fonts: the galleries are menus built from the shared sets (libs/office/office/themelib.h, libs/shell/app/themedialogs.h); a choice is one step
// of the history (EditSession::applyTheme). What names the theme's colors and fonts (cells, cell styles, table styles, shapes, and charts) follows it; the color buttons and pickers offer
// the new colors.

#include "app/crunchwindow.h"

#include "app/icons.h"
#include "app/plaintext.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"
#include "app/themedialogs.h"
#include "core/editsession.h"
#include "core/sheetobjects.h"
#include "grid/gridview.h"

#include <QAction>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QSaveFile>
#include <QStandardPaths>

namespace jp {

namespace {

QIcon packIcon(const office::ThemePack &pack) { return themeColorsIcon(pack.colors); }

} // namespace

void CrunchWindow::createThemeActions()
{
    mk(QStringLiteral("design.themes.browse"), tr("Browse for Themes…"), QStringLiteral("folder-open"), QKeySequence(), [this] {
        if (!m_session || !editingAllowed()) return;
        const QString path = QFileDialog::getOpenFileName(this, tr("Browse for Themes"),
                                                          Settings::get().value(QStringLiteral("dirs/open"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                                                          tr("Themes, Workbooks, Documents, and Presentations (*.thmx *.xlsx *.xltx *.docx *.dotx *.pptx *.potx)") + QStringLiteral(";;") + tr("All Files (*)"));
        if (path.isEmpty()) return;
        Settings::get().setValue(QStringLiteral("dirs/open"), QFileInfo(path).absolutePath());
        applyThemeFile(path);
    });
    mk(QStringLiteral("design.themes.save"), tr("Save Current Theme…"), QStringLiteral("save"), QKeySequence(), [this] {
        if (!m_session || !m_session->hasTheme()) return;
        const QString folder = Settings::get().value(QStringLiteral("dirs/open"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        const office::Theme theme = m_session->theme();
        const QString path = QFileDialog::getSaveFileName(this, tr("Save Current Theme"), folder + QLatin1Char('/') + (theme.name.isEmpty() ? tr("Theme") : theme.name) + QStringLiteral(".thmx"),
                                                          tr("Theme Files (*.thmx)"));
        if (path.isEmpty()) return;
        saveThemeFile(path.endsWith(QLatin1String(".thmx"), Qt::CaseInsensitive) ? path : path + QStringLiteral(".thmx"));
    });
    mk(QStringLiteral("design.themes.reset"), tr("Reset to the Workbook's Theme"), QStringLiteral("rotate-ccw"), QKeySequence(), [this] { resetTheme(); });
    // (Reset shows only once the theme differs from the one the workbook was opened with: the menus set this as they open)
    act(QStringLiteral("design.themes.reset"))->setEnabled(false);
}

bool CrunchWindow::themeDiffers() const
{
    return m_session && m_session->hasTheme() && m_session->theme().xml != m_openedTheme.xml;
}

bool CrunchWindow::changeTheme(const QString &step, const std::function<office::Theme(const office::Theme &, bool *)> &make)
{
    if (!m_session || !editingAllowed()) return false;
    if (!m_session->hasTheme()) {
        notice(tr("Theme"), tr("This workbook has no theme part, so it can't take a theme. Nothing was changed."));
        return false;
    }
    bool ok = true;
    const office::Theme next = make(m_session->theme(), &ok);
    if (!ok) {
        notice(tr("Theme"), tr("The theme could not be applied to this workbook. Nothing was changed."));
        return false;
    }
    const crunch::EditSession::Outcome o = m_session->applyTheme(next, step);
    if (!o.ok) {
        notice(tr("Theme"), o.problem);
        return false;
    }
    m_view->setFocus(Qt::OtherFocusReason);
    return true;
}

bool CrunchWindow::applyThemeFile(const QString &path)
{
    office::Theme theme;
    QString why;
    if (!office::readThemeFile(path, &theme, &why)) {
        notice(tr("Browse for Themes"), tr("%1 can't be read: %2").arg(QFileInfo(path).fileName(), why.isEmpty() ? tr("The file can't be read.") : why));
        return false;
    }
    return changeTheme(tr("Theme"), [&theme](const office::Theme &, bool *) { return theme; });
}

bool CrunchWindow::saveThemeFile(const QString &path)
{
    if (!m_session || !m_session->hasTheme()) return false;
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(office::writeThemeFile(m_session->theme())) < 0 || !f.commit()) {
        notice(tr("Save Current Theme"), tr("%1 can't be written.").arg(QFileInfo(path).fileName()));
        return false;
    }
    return true;
}

bool CrunchWindow::resetTheme()
{
    if (m_openedTheme.xml.isEmpty()) return false;
    return changeTheme(tr("Reset Theme"), [this](const office::Theme &, bool *) { return m_openedTheme; });
}

// A theme was set, undone, or redone: what is made from its colors and fonts is made again.
void CrunchWindow::onThemeChanged()
{
    if (m_objects) m_objects->themeChanged();
    m_tableSwatches.clear();
    if (m_fillButton) m_fillButton->setScheme(workbookColors());
    if (m_fontColorButton) m_fontColorButton->setScheme(workbookColors());
    refreshStripes();
    updateFormatState();
    m_view->viewport()->update();
}

void CrunchWindow::themeRibbonParts(RibbonParts &parts)
{
    parts.menus[QStringLiteral("themesMenu")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("themesMenu"));
        connect(menu, &QMenu::aboutToShow, menu, [this, menu] {
            menu->clear();
            const bool ok = m_session && m_session->hasTheme();
            menu->addSection(tr("Built-In"));
            for (const office::ThemePack &pack : office::builtinThemePacks()) {
                QAction *a = menu->addAction(packIcon(pack), pack.name);
                a->setEnabled(ok);
                connect(a, &QAction::triggered, menu, [this, pack] {
                    changeTheme(tr("Theme"), [&pack](const office::Theme &now, bool *good) {
                        bool colorsOk = true, fontsOk = true;
                        office::Theme t = office::withThemeColors(now, pack.colors, &colorsOk);
                        t = office::withThemeFonts(t, pack.fonts, &fontsOk);
                        t.name = pack.name;
                        *good = colorsOk && fontsOk;
                        return t;
                    });
                });
            }
            menu->addSeparator();
            menu->addAction(act(QStringLiteral("design.themes.browse")));
            act(QStringLiteral("design.themes.save"))->setEnabled(ok);
            menu->addAction(act(QStringLiteral("design.themes.save")));
            act(QStringLiteral("design.themes.reset"))->setEnabled(themeDiffers());
            menu->addAction(act(QStringLiteral("design.themes.reset")));
        });
        return menu;
    };
    parts.menus[QStringLiteral("themeColorsMenu")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("themeColorsMenu"));
        connect(menu, &QMenu::aboutToShow, menu, [this, menu] {
            menu->clear();
            act(QStringLiteral("design.themes.reset"))->setEnabled(themeDiffers());
            if (!m_session || !m_session->hasTheme()) return;
            ThemeMenuHooks hooks;
            hooks.dialogParent = this;
            hooks.chooseColors = [this](const office::ThemeColors &c) {
                changeTheme(tr("Theme Colors"), [&c](const office::Theme &now, bool *good) { return office::withThemeColors(now, c, good); });
            };
            hooks.reset = [this] { resetTheme(); };
            addThemeColorsEntries(menu, office::builtinThemeColors(), office::themeColorsOf(m_session->theme()), hooks);
        });
        return menu;
    };
    parts.menus[QStringLiteral("themeFontsMenu")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("themeFontsMenu"));
        connect(menu, &QMenu::aboutToShow, menu, [this, menu] {
            menu->clear();
            act(QStringLiteral("design.themes.reset"))->setEnabled(themeDiffers());
            if (!m_session || !m_session->hasTheme()) return;
            ThemeMenuHooks hooks;
            hooks.dialogParent = this;
            hooks.chooseFonts = [this](const office::ThemeFonts &f) {
                changeTheme(tr("Theme Fonts"), [&f](const office::Theme &now, bool *good) { return office::withThemeFonts(now, f, good); });
            };
            hooks.reset = [this] { resetTheme(); };
            addThemeFontsEntries(menu, office::builtinThemeFonts(), office::themeFontsOf(m_session->theme()), hooks);
        });
        return menu;
    };
}

} // namespace jp
