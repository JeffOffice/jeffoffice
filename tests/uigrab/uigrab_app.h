// Developer tool, shared by the Deck and Pub grab programs: saves a picture of each dialog (every enabled command that ends in "…"), each File page, and the panes named in JO_GRAB_PANES, for
// looking at the style (docs/design/STYLE.md).
#pragma once

#include "app/appwindow.h"

#include <QApplication>
#include <QDialog>
#include <QEventLoop>
#include <QDir>
#include <QFileDialog>
#include <QTimer>
#include <QWidget>

namespace uigrab {

inline void grabDialogs(jp::AppWindow &w, const QString &dir)
{
    QDir().mkpath(dir);
    QStringList ids = qEnvironmentVariable("JO_GRAB_IDS").split(QLatin1Char(','), Qt::SkipEmptyParts);
    if (ids.isEmpty())
        for (const QString &id : w.actionIds()) {
            QAction *a = w.act(id);
            if (a && a->isEnabled() && a->text().endsWith(QStringLiteral("…")) && !id.startsWith(QStringLiteral("file.open")) && !id.startsWith(QStringLiteral("file.save"))) ids << id;
        }
    for (const QString &id : ids) {
        QAction *a = w.act(id);
        if (!a || !a->isEnabled()) continue;
        bool grabbed = false;
        QTimer poll, watchdog;
        watchdog.setSingleShot(true);
        QObject::connect(&poll, &QTimer::timeout, [&] {
            QWidget *m = QApplication::activeModalWidget();
            if (!m) return;
            poll.stop();
            QTimer::singleShot(150, [&, m] {
                if (!qobject_cast<QFileDialog *>(m)) {
                    m->grab().save(dir + QLatin1Char('/') + id + QStringLiteral(".png"));
                    grabbed = true;
                }
                if (auto *d = qobject_cast<QDialog *>(m)) d->reject(); else m->close();
            });
        });
        QObject::connect(&watchdog, &QTimer::timeout, [&] {
            if (QWidget *m = QApplication::activeModalWidget()) m->close();
        });
        poll.start(20);
        watchdog.start(3000);
        a->trigger();
        poll.stop();
        if (!grabbed) qWarning("no dialog for %s", qPrintable(id));
    }
}

// `pages` are the keys of the File screen's pages (a command such as Save As or Options is not a page: it opens a dialog).
inline void grabPages(jp::AppWindow &w, const QString &dir, const QStringList &pages)
{
    QDir().mkpath(dir);
    for (const QString &page : pages) {
        w.showFileScreenPage(page);
        QApplication::processEvents();
        QTimer t;
        t.setSingleShot(true);
        QEventLoop loop;
        QObject::connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
        t.start(400);
        loop.exec();
        w.grab().save(dir + QLatin1Char('/') + page + QStringLiteral(".png"));
    }
}

} // namespace uigrab
