#pragma once
// For the test programs that answer a modal dialog their command opens (the window tests of every app).
#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
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
// QCOMPARE: a failed one leaves `fill`, not the dialog. The dialog is always closed from outside `fill`, and a watchdog closes whatever is still open after
// a minute and fails the test, so that a failure ends the test and never leaves it waiting on a dialog (on CI that costs the whole job). It guards against a hang, and it counts from when the
// dialog is found (a loaded CI machine ran the checks 5 to 8 times slower on Nov 15, and ten seconds from the start were gone before a slow fill was done: "the dialog was left open").
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
        if (dlg) watchdog.start(60000);
        if (dlg && opened == objectName) fill(dlg);
        closeAll(accept && opened == objectName);
    });
    QObject::connect(&watchdog, &QTimer::timeout, [&] {
        leftOpen = true;
        closeAll(false);
    });
    once.start(0);
    watchdog.start(60000);
    open();
    QCOMPARE(opened, objectName);
    QVERIFY2(!leftOpen, "the dialog was left open");
}

// The box a window asks about unsaved changes with when it closes (libs/shell/app/uistyle.h, the same in the four apps): runs `closeIt`, which makes the window ask, finds the box,
// checks it (red triangle, the question with the name, the grey line, Don't Save at the left, Cancel then Save at the right, Save the accent-filled default), and presses `press`
// (Save, Discard, or Cancel). The problems found, empty when it is right.
enum class CloseHow { Press, Escape, CloseX, Enter };
inline QString answerCloseBox(const std::function<void()> &closeIt, const QString &name, QMessageBox::StandardButton press, CloseHow how = CloseHow::Press)
{
    QString problems;
    bool seen = false;
    QTimer poll, watchdog;
    watchdog.setSingleShot(true);
    QObject::connect(&poll, &QTimer::timeout, [&] {
        auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (!box || !box->isVisible()) return;
        QAbstractButton *save = box->button(QMessageBox::Save), *discard = box->button(QMessageBox::Discard), *cancel = box->button(QMessageBox::Cancel);
        if (!save || !discard || !cancel || !box->findChild<QLabel *>(QStringLiteral("jpBoxSign"))) return;   // (not laid out yet)
        poll.stop();
        seen = true;
        auto x = [&](QWidget *w) { return w->mapTo(box, QPoint(0, 0)).x(); };
        if (box->icon() != QMessageBox::Warning) problems += QStringLiteral("not a warning sign; ");
        if (box->text() != QStringLiteral("Save changes to \"%1\"?").arg(name)) problems += QStringLiteral("text is ") + box->text() + QStringLiteral("; ");
        if (box->informativeText() != QStringLiteral("If you don't save, your changes since the last save are lost.")) problems += QStringLiteral("detail is ") + box->informativeText() + QStringLiteral("; ");
        if (discard->text() != QStringLiteral("Don't Save")) problems += QStringLiteral("discard says ") + discard->text() + QStringLiteral("; ");
        if (!(x(discard) < x(cancel) && x(cancel) < x(save))) problems += QStringLiteral("buttons are not Don't Save, Cancel, Save; ");
        if (box->defaultButton() != save || !save->property("primary").toBool()) problems += QStringLiteral("Save is not the default; ");
        if (!save->icon().isNull() || !cancel->icon().isNull() || !discard->icon().isNull()) problems += QStringLiteral("icons on buttons; ");
        // how the person answers: pressing a button, Escape (Cancel), the X of the box (Cancel), or Enter (the accent-filled Save)
        QMessageBox::StandardButton expected = press;
        if (how == CloseHow::Press) box->button(press)->click();
        else if (how == CloseHow::Escape) { expected = QMessageBox::Cancel; QTest::keyClick(box, Qt::Key_Escape); }
        else if (how == CloseHow::CloseX) { expected = QMessageBox::Cancel; box->close(); }
        else { expected = QMessageBox::Save; QTest::keyClick(box, Qt::Key_Return); }
        if (box->clickedButton() != box->button(expected)) problems += QStringLiteral("the answer was not the expected button; ");
    });
    QObject::connect(&watchdog, &QTimer::timeout, [&] {
        problems += QStringLiteral("no box; ");
        if (auto *m = QApplication::activeModalWidget()) m->close();
    });
    poll.start(20);
    watchdog.start(10000);
    closeIt();
    poll.stop();
    if (!seen && !problems.contains(QLatin1String("no box"))) problems += QStringLiteral("no box; ");
    return problems;
}
