// Developer tool, not a test of behavior: with JO_SHOT_DIR set, opens JeffCrunch's dialogs one by one and saves a picture of each (for looking at the style, docs/design/STYLE.md).
//   JO_SHOT_DIR=/some/folder QT_QPA_PLATFORM=offscreen ./jo_uigrab_crunch
#include "../crunch/editwindow.h"
#include "core/fonts.h"
#include "app/theme.h"
#include "../testrunner.h"
#include "../withdialog.h"

#include <QDir>
#include <QMenu>
#include <QtTest>

using namespace crunchtest;

namespace {

class UiGrabCrunch : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { jp::initCore(); jp::applyUiTheme(1); }

    void grabTheDialogs()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        QDir().mkpath(dir);
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select(QStringLiteral("A1"));
        const QStringList ids = qEnvironmentVariable("JO_GRAB_IDS", QStringLiteral("format.cells,data.validation,data.consolidate,data.goalSeek,data.removeDuplicates,data.subtotal,find.replace,find.goto,find.gotoSpecial,edit.sort,insert.link,insert.symbol,insert.barcode,paste.special,fill.series,formulas.names,formulas.defineName,format.columnWidth,margins.custom,formulas.insertFunction,formulas.options,data.advancedFilter,insert.table,data.textToColumns,formulas.iteration")).split(QLatin1Char(','));
        for (const QString &id : ids) {
            QAction *a = e.win.w.act(id);
            if (!a || !a->isEnabled()) { qWarning("no enabled action %s", qPrintable(id)); continue; }
            bool grabbed = false;
            QTimer poll, watchdog;
            watchdog.setSingleShot(true);
            QObject::connect(&poll, &QTimer::timeout, [&] {
                QWidget *m = QApplication::activeModalWidget();
                if (!m) return;
                poll.stop();
                QTimer::singleShot(150, [&, m] {
                    m->grab().save(dir + QLatin1Char('/') + id + QStringLiteral(".png"));
                    grabbed = true;
                    if (auto *d = qobject_cast<QDialog *>(m)) d->reject(); else m->close();
                });
            });
            QObject::connect(&watchdog, &QTimer::timeout, [&] {
                if (QWidget *m = QApplication::activeModalWidget()) m->close();
            });
            poll.start(20);
            watchdog.start(8000);
            a->trigger();
            poll.stop();
            if (!grabbed) qWarning("no dialog for %s", qPrintable(id));
        }
    }

    // The Calculation Options menu with Set Precision as Displayed ticked (the warning itself is grabbed above with JO_GRAB_IDS=formulas.precision).
    void grabThePrecisionSwitch()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        QDir().mkpath(dir);
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.session()->setPrecisionAsShown(true).ok);
        QVERIFY(e.settle());
        QMenu menu;
        for (const char *id : {"formulas.calcAuto", "formulas.calcNoTables", "formulas.calcManual", "formulas.calcBeforeSave", "formulas.iteration", "formulas.precision"}) menu.addAction(e.win.w.act(QString::fromLatin1(id)));
        menu.show();
        QVERIFY(QTest::qWaitForWindowExposed(&menu));
        QVERIFY(menu.grab().save(dir + QStringLiteral("/precision-switch.png")));
    }
};

} // namespace

JP_TEST_CLASS(UiGrabCrunch)
#include "tst_uigrab_crunch.moc"
