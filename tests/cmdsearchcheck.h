#pragma once
// What each app's own test checks of the command search (the shared behavior is tested in tests/shell/tst_shell_cmdsearch.cpp): its window has the box, in the ribbon's top row; its data file of extra
// words names only commands, File screen entries, and Options pages the app has; and a query that needs those words finds the command.

#include "app/appwindow.h"
#include "app/commandsearch.h"
#include "app/filescreen.h"
#include "app/i18n.h"
#include "app/ribbon.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QListWidget>
#include <QPainter>
#include <QPixmap>
#include <QSet>

namespace jptest {

// Empty when all is well, else what is wrong. `resource` is the app's words file (":/write/cmdsearch.json"); `query` should find a command named `expected` among the first results.
inline QString commandSearchProblem(jp::AppWindow &w, const QString &resource, const QString &query, const QString &expected)
{
    auto *box = w.findChild<jp::CommandSearchBox *>();
    if (!box) return QStringLiteral("the window has no command search box");
    if (!w.ribbon() || w.ribbon()->searchWidget() != box) return QStringLiteral("the box is not in the ribbon's top row");
    // At 1000 pixels the box leaves every tab in the row.
    w.resize(1000, 700);
    QApplication::processEvents();
    for (int i = 0; i < w.ribbon()->tabCount(); ++i)
        if (w.ribbon()->tabVisible(i) && w.ribbon()->tabButton(i) && !w.ribbon()->tabButton(i)->isVisible()) return QStringLiteral("the tab %1 is folded away at 1000 pixels").arg(w.ribbon()->tabName(i));
    QFile f(resource);
    if (!f.open(QIODevice::ReadOnly)) return QStringLiteral("no words file ") + resource;
    const jp::cmdsearch::Words words = jp::cmdsearch::parseWords(f.readAll());
    if (words.isEmpty()) return QStringLiteral("the words file is empty or not JSON");
    const QList<jp::cmdsearch::Entry> all = jp::cmdsearch::collect(&w, words);
    QSet<QString> ids, filesKeys, pages;
    for (const auto &e : all) {
        ids << e.id;
        if (e.kind == jp::cmdsearch::Kind::File) filesKeys << e.key;
        if (e.kind == jp::cmdsearch::Kind::Options) pages << e.key;
    }
    if (filesKeys.isEmpty()) return QStringLiteral("no File screen entries are listed");
    if (pages.size() < 5) return QStringLiteral("the Options pages are not listed");
    for (auto it = words.begin(); it != words.end(); ++it) {
        const QString key = it.key();
        if (key.startsWith(QLatin1String("cmd:")) && !ids.contains(key) && !w.act(key.mid(4))) return QStringLiteral("the words name a command the app does not have: ") + key;
        if (key.startsWith(QLatin1String("file:")) && !filesKeys.contains(key.mid(5))) return QStringLiteral("the words name a File screen entry the app does not have: ") + key;
        if (key.startsWith(QLatin1String("options:")) && !pages.contains(key.mid(8))) return QStringLiteral("the words name an Options page the app does not have: ") + key;
    }
    box->setText(query);
    QStringList names;
    for (const auto &e : box->shownResults().mid(0, 3)) names << e.name;
    box->setText(QString());
    if (!names.contains(expected)) return QStringLiteral("\"%1\" finds %2, not %3").arg(query, names.join(QStringLiteral(", ")), expected);
    return QString();
}

// A picture of the window with the box open and `query` typed (the folder is JO_CMDSEARCH_PICTURES, when the environment names one; a hook for the screenshots in the release notes).
inline void saveCommandSearchPicture(jp::AppWindow &w, const QString &name, const QString &query)
{
    const QString dir = qEnvironmentVariable("JO_CMDSEARCH_PICTURES");
    auto *box = w.findChild<jp::CommandSearchBox *>();
    if (dir.isEmpty() || !box) return;
    QDir().mkpath(dir);
    jp::CommandSearchBox::setScreenForTests(QRect(0, 0, 1920, 1080));   // (a large screen: the offscreen one is 640 by 480 and would push the list aside)
    box->setFocus();
    box->setText(query);
    QPixmap shot = w.grab();
    if (QWidget *popup = box->list()->parentWidget(); popup && popup->isVisible()) {
        QPainter p(&shot);
        p.drawPixmap(w.mapFromGlobal(popup->mapToGlobal(QPoint(0, 0))), popup->grab());
    }
    jp::CommandSearchBox::setScreenForTests(QRect());
    shot.save(QDir(dir).filePath(name + QStringLiteral(".png")));
    box->setText(QString());
}

} // namespace jptest
