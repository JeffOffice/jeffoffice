#pragma once
// For the test programs that answer a modal dialog their command opens (the window tests of every app).
#include <QApplication>
#include <QDialog>
#include <QString>
#include <QTimer>
#include <QtTest>
#include <functional>

// With JO_SHOT_DIR set, saves a picture of the widget there as `name`.png (for the screenshots of a change's dialogs); otherwise does nothing.
inline void shotOf(QWidget *widget, const QString &name)
{
    const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
    if (!dir.isEmpty() && widget) widget->grab().save(dir + QLatin1Char('/') + name + QStringLiteral(".png"));
}

// Runs `open`, which makes a modal dialog, and meanwhile runs `fill` on that dialog, then presses OK (Cancel when `accept` is false). `fill` may use QVERIFY and
// QCOMPARE: a failed one leaves `fill`, not the dialog. The dialog is always closed from outside `fill`, and a watchdog closes whatever is still open after ten
// seconds and fails the test, so that a failure ends the test and never leaves it waiting on a dialog (on CI that costs the whole job).
inline void withDialog(const std::function<void()> &open, const QString &objectName, const std::function<void(QDialog *)> &fill, bool accept = true)
{
    QString opened;
    bool leftOpen = false;
    auto closeAll = [](bool ok) {
        for (QWidget *top : QApplication::topLevelWidgets())
            if (auto *d = qobject_cast<QDialog *>(top); d && d->isVisible()) {
                if (ok) d->accept();
                else d->reject();
            }
    };
    QTimer once, watchdog;
    once.setSingleShot(true);
    watchdog.setSingleShot(true);
    QObject::connect(&once, &QTimer::timeout, [&] {
        auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        // (the command may spin the event loop before it shows the box, as the spelling check does between its slices of grammar work: keep looking)
        if (!dlg && !leftOpen) {
            once.start(20);
            return;
        }
        if (dlg) opened = dlg->objectName();
        if (dlg && opened == objectName) fill(dlg);
        closeAll(accept && opened == objectName);
    });
    QObject::connect(&watchdog, &QTimer::timeout, [&] {
        leftOpen = true;
        closeAll(false);
    });
    once.start(0);
    watchdog.start(10000);
    open();
    QCOMPARE(opened, objectName);
    QVERIFY2(!leftOpen, "the dialog was left open");
}
