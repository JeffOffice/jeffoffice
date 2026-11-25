#pragma once
// The Clipboard pane (libs/shell/app/clipboardpane.h) as an app wires it: the Clipboard group's launcher on the Home tab has the shared key tip and opens the pane; what the app's own Copy puts on the
// clipboard is an item of the kind it should be; a real click on the item runs the app's own Paste once, with that item on the clipboard. Each app's test calls this with a way to make a selection
// and copy it. The clipboard and the history are put back as they were found.
#include "app/appwindow.h"
#include "app/clipboardhistory.h"
#include "app/clipboardpane.h"
#include "app/ribbon.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDockWidget>
#include <QListWidget>
#include <QMimeData>
#include <QToolButton>
#include <QtTest>
#include <functional>

namespace jp::clipcheck {

inline QToolButton *clipboardLauncher(AppWindow *w)
{
    for (RibbonGroup *g : w->ribbon()->tab(QStringLiteral("Home"))->groups())
        if (g->title() == QLatin1String("Clipboard")) return g->launcher();
    return nullptr;
}

// The rig of the tests of what a paste from the pane does: a history of its own that collects, and the clipboard as it was found put back at the end (declare it first in a test: it ends last).
struct Rig {
    QMimeData *was = new QMimeData;
    ClipboardHistory *h = nullptr;
    Rig()
    {
        if (const QMimeData *now = QApplication::clipboard()->mimeData())
            for (const QString &f : now->formats()) was->setData(f, now->data(f));
        delete ClipboardHistory::instance();
        h = ClipboardHistory::instance();
        h->addOpenPane(1);
        h->setOwnerCheckForTest([] { return true; });
    }
    ~Rig()
    {
        delete h;
        QApplication::clipboard()->setMimeData(was);
    }
};

// The window's own Clipboard pane, opened from its launcher.
inline ClipboardPane *paneOf(AppWindow *w)
{
    QToolButton *launcher = clipboardLauncher(w);
    if (!launcher) return nullptr;
    launcher->click();
    QDockWidget *dock = w->findChild<QDockWidget *>(QStringLiteral("clipboardDock"));
    return dock ? dock->findChild<ClipboardPane *>() : nullptr;
}

// `copySomething` selects something in the window and copies it with the app's own Copy; `kind` is what it should be kept as.
inline void paneWiring(AppWindow *w, const std::function<void()> &copySomething, ClipboardHistory::Kind kind)
{
    QMimeData *was = new QMimeData;
    if (const QMimeData *now = QApplication::clipboard()->mimeData())
        for (const QString &f : now->formats()) was->setData(f, now->data(f));
    delete ClipboardHistory::instance();
    ClipboardHistory *h = ClipboardHistory::instance();
    h->setCollectWhenClosed(false);
    h->setOwnerCheckForTest([] { return true; });
    QToolButton *launcher = clipboardLauncher(w);
    QVERIFY(launcher);
    QCOMPARE(keytip(launcher), clipboardLauncherKeytip());
    QVERIFY(!h->paneOpen());
    launcher->click();
    QDockWidget *dock = w->findChild<QDockWidget *>(QStringLiteral("clipboardDock"));
    QVERIFY(dock);
    QVERIFY(dock->isVisible());
    QCOMPARE(dock->windowTitle(), QStringLiteral("Clipboard"));
    QVERIFY(h->paneOpen());
    copySomething();
    QTRY_VERIFY2(!h->items().isEmpty(), qPrintable(QStringLiteral("too large: %1, clipboard formats: %2").arg(h->tooLargeCount()).arg(QApplication::clipboard()->mimeData() ? QApplication::clipboard()->mimeData()->formats().join(QLatin1Char(' ')) : QString())));
    QCOMPARE(h->items().first().kind, kind);
    auto *list = dock->findChild<QListWidget *>(QStringLiteral("clipboardList"));
    QVERIFY(list);
    QCOMPARE(list->count(), int(h->items().size()));
    int pasted = 0;
    QByteArray pastedFrom;
    const QByteArray ours = h->items().first().formats.isEmpty() ? QByteArray() : h->items().first().formats.first().second;
    QObject context;   // (the connection goes with this function's locals)
    QObject::connect(w->act(QStringLiteral("edit.paste")), &QAction::triggered, &context, [&] {
        ++pasted;
        if (const QMimeData *m = QApplication::clipboard()->mimeData(); m && !m->formats().isEmpty()) pastedFrom = m->data(m->formats().first());
    });
    const QRect r = list->visualItemRect(list->item(0));
    const QPoint inside(r.left() + r.width() / 3, r.center().y());
    QTest::mouseMove(list->viewport(), inside);
    QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, inside);
    QCOMPARE(pasted, 1);
    QCOMPARE(pastedFrom, ours);
    dock->close();
    QVERIFY(!h->paneOpen());
    delete h;
    QApplication::clipboard()->setMimeData(was);
}

} // namespace jp::clipcheck
