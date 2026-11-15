// JeffWrite's Design > Themes, Colors, and Fonts: the galleries are menus built from the shared sets (libs/office/office/themelib.h, libs/shell/app/themedialogs.h); a choice is one step of the
// history (core/themeops.h).

#include "app/writewindow.h"

#include "app/icons.h"
#include "app/pageview.h"
#include "app/plaintext.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"
#include "app/themedialogs.h"
#include "core/themeops.h"

#include <QAction>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QPainter>
#include <QSaveFile>
#include <QStandardPaths>

namespace jp {

namespace {

// A theme part with no place for the colors or fonts can't take them: nothing was changed, and the person is told.
void tellNotApplied(QWidget *parent)
{
    jp::plainWarning(parent, QCoreApplication::translate("jp::WriteWindow", "Theme"), QCoreApplication::translate("jp::WriteWindow", "The theme could not be applied to this document. Nothing was changed."));
}

QIcon packIcon(const office::ThemePack &pack) { return themeColorsIcon(pack.colors); }

} // namespace

void WriteWindow::createThemeActions()
{
    mk("design.themes.browse", tr("Browse for Themes…"), "folder-open", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        const QString path = QFileDialog::getOpenFileName(this, tr("Browse for Themes"),
                                                          Settings::get().value(QStringLiteral("dirs/open"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString(),
                                                          tr("Themes, Documents, and Presentations (*.thmx *.docx *.dotx *.pptx *.potx)") + QStringLiteral(";;") + tr("All Files (*)"));
        if (path.isEmpty()) return;
        Settings::get().setValue(QStringLiteral("dirs/open"), QFileInfo(path).absolutePath());
        applyThemeFile(path);
        m_view->setFocus(Qt::OtherFocusReason);
    });
    mk("design.themes.save", tr("Save Current Theme…"), "save", QKeySequence(), [this] {
        if (!m_doc) return;
        const QString folder = Settings::get().value(QStringLiteral("dirs/open"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        const office::Theme theme = write::documentTheme(*m_doc);
        const QString path = QFileDialog::getSaveFileName(this, tr("Save Current Theme"), folder + QLatin1Char('/') + (theme.name.isEmpty() ? tr("Theme") : theme.name) + QStringLiteral(".thmx"),
                                                          tr("Theme Files (*.thmx)"));
        if (path.isEmpty()) return;
        saveThemeFile(path.endsWith(QLatin1String(".thmx"), Qt::CaseInsensitive) ? path : path + QStringLiteral(".thmx"));
    });
    mk("design.themes.reset", tr("Reset to the Template's Theme"), "rotate-ccw", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        resetTheme();
        m_view->setFocus(Qt::OtherFocusReason);
    });
    // (Reset shows only once the theme differs from the one the document was opened with: the menus set this as they open)
    act("design.themes.reset")->setEnabled(false);
}

bool WriteWindow::themeDiffers() const
{
    return m_doc && write::canChangeTheme(*m_doc) && write::documentTheme(*m_doc).xml != m_openedTheme.xml;
}

bool WriteWindow::applyThemeFile(const QString &path)
{
    if (!m_doc) return false;
    office::Theme theme;
    QString why;
    if (!office::readThemeFile(path, &theme, &why)) {
        jp::plainWarning(this, tr("Browse for Themes"), tr("%1 can't be read: %2").arg(QFileInfo(path).fileName(), why.isEmpty() ? tr("The file can't be read.") : why));
        return false;
    }
    if (!write::canChangeTheme(*m_doc)) {
        jp::plainWarning(this, tr("Browse for Themes"), tr("This document has no theme part, so it can't take a theme. Nothing was changed."));
        return false;
    }
    write::applyTheme(*m_doc, theme);
    return true;
}

bool WriteWindow::saveThemeFile(const QString &path)
{
    if (!m_doc || !write::canChangeTheme(*m_doc)) return false;
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(office::writeThemeFile(write::documentTheme(*m_doc))) < 0 || !f.commit()) {
        jp::plainWarning(this, tr("Save Current Theme"), tr("%1 can't be written.").arg(QFileInfo(path).fileName()));
        return false;
    }
    return true;
}

bool WriteWindow::resetTheme()
{
    if (!m_doc || !write::canChangeTheme(*m_doc) || m_openedTheme.xml.isEmpty()) return false;
    return write::applyTheme(*m_doc, m_openedTheme);
}

void WriteWindow::themeRibbonParts(RibbonParts &parts)
{
    parts.menus[QStringLiteral("themesMenu")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("themesMenu"));
        connect(menu, &QMenu::aboutToShow, menu, [this, menu] {
            menu->clear();
            const bool ok = m_doc && write::canChangeTheme(*m_doc);
            menu->addSection(tr("Built-In"));
            for (const office::ThemePack &pack : office::builtinThemePacks()) {
                QAction *a = menu->addAction(packIcon(pack), pack.name);
                a->setEnabled(ok);
                connect(a, &QAction::triggered, menu, [this, pack] {
                    if (!m_doc || refuseIfReadOnly()) return;
                    bool colorsOk = true, fontsOk = true;
                    office::Theme t = office::withThemeColors(write::documentTheme(*m_doc), pack.colors, &colorsOk);
                    t = office::withThemeFonts(t, pack.fonts, &fontsOk);
                    if (!colorsOk || !fontsOk) {
                        tellNotApplied(this);
                        return;
                    }
                    t.name = pack.name;
                    write::applyTheme(*m_doc, t);
                    m_view->setFocus(Qt::OtherFocusReason);
                });
            }
            menu->addSeparator();
            menu->addAction(act("design.themes.browse"));
            act("design.themes.save")->setEnabled(ok);
            menu->addAction(act("design.themes.save"));
            act("design.themes.reset")->setEnabled(themeDiffers());
            menu->addAction(act("design.themes.reset"));
        });
        return menu;
    };
    parts.menus[QStringLiteral("themeColorsMenu")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("themeColorsMenu"));
        connect(menu, &QMenu::aboutToShow, menu, [this, menu] {
            menu->clear();
            act("design.themes.reset")->setEnabled(themeDiffers());
            if (!m_doc || !write::canChangeTheme(*m_doc)) return;
            ThemeMenuHooks hooks;
            hooks.dialogParent = this;
            hooks.chooseColors = [this](const office::ThemeColors &c) {
                if (!m_doc || refuseIfReadOnly()) return;
                bool couldNot = false;
                write::applyThemeColors(*m_doc, c, &couldNot);
                if (couldNot) tellNotApplied(this);
                m_view->setFocus(Qt::OtherFocusReason);
            };
            hooks.reset = [this] {
                if (!m_doc || refuseIfReadOnly()) return;
                resetTheme();
                m_view->setFocus(Qt::OtherFocusReason);
            };
            addThemeColorsEntries(menu, office::builtinThemeColors(), office::themeColorsOf(write::documentTheme(*m_doc)), hooks);
        });
        return menu;
    };
    parts.menus[QStringLiteral("themeFontsMenu")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("themeFontsMenu"));
        connect(menu, &QMenu::aboutToShow, menu, [this, menu] {
            menu->clear();
            act("design.themes.reset")->setEnabled(themeDiffers());
            if (!m_doc || !write::canChangeTheme(*m_doc)) return;
            ThemeMenuHooks hooks;
            hooks.dialogParent = this;
            hooks.chooseFonts = [this](const office::ThemeFonts &f) {
                if (!m_doc || refuseIfReadOnly()) return;
                bool couldNot = false;
                write::applyThemeFonts(*m_doc, f, &couldNot);
                if (couldNot) tellNotApplied(this);
                m_view->setFocus(Qt::OtherFocusReason);
            };
            hooks.reset = [this] {
                if (!m_doc || refuseIfReadOnly()) return;
                resetTheme();
                m_view->setFocus(Qt::OtherFocusReason);
            };
            addThemeFontsEntries(menu, office::builtinThemeFonts(), office::themeFontsOf(write::documentTheme(*m_doc)), hooks);
        });
        return menu;
    };
}

} // namespace jp
