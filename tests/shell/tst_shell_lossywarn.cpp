// The warning before a save to a kind of file that can't hold everything (libs/shell/app/lossywarning.h): it shows the list it is given, each answer is what it says, a list with nothing in it
// shows nothing, and "don't ask again for this file" lasts for the session.
#include "app/lossywarning.h"
#include "../testrunner.h"

#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest>

using namespace jp;

class ShellLossyWarnTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void init()
    {
        lossyForgetAll();
        setLossyStandIn(nullptr);
    }
    void cleanup() { init(); }

    void aListShowsAndEachAnswerIsReturned()
    {
        int asked = 0;
        LossyAsk seen;
        LossyChoice answer = LossyChoice::SaveOwnFormat;
        setLossyStandIn([&](const LossyAsk &a, bool *) {
            ++asked;
            seen = a;
            return answer;
        });
        const LossyAsk ask{QStringLiteral("/tmp/x/a.odp"), QStringLiteral("presentation"), QStringLiteral(".odp"), QStringLiteral(".pptx"), {QStringLiteral("Sections"), QStringLiteral("Glows")}};
        QCOMPARE(askLossy(nullptr, ask), LossyChoice::SaveOwnFormat);
        QCOMPARE(asked, 1);
        QCOMPARE(seen.lost, ask.lost);
        answer = LossyChoice::Cancel;
        QCOMPARE(askLossy(nullptr, ask), LossyChoice::Cancel);
        answer = LossyChoice::SaveAnyway;
        QCOMPARE(askLossy(nullptr, ask), LossyChoice::SaveAnyway);
        QCOMPARE(asked, 3);
    }

    void nothingLostShowsNothing()
    {
        int asked = 0;
        setLossyStandIn([&](const LossyAsk &, bool *) {
            ++asked;
            return LossyChoice::Cancel;
        });
        QCOMPARE(askLossy(nullptr, {QStringLiteral("/tmp/x/a.odp"), QStringLiteral("presentation"), QStringLiteral(".odp"), QStringLiteral(".pptx"), {}}), LossyChoice::SaveAnyway);
        QCOMPARE(asked, 0);
        QVERIFY(lossyReport({QString(), QStringLiteral("presentation"), QStringLiteral(".odp"), QStringLiteral(".pptx"), {}}).startsWith(QLatin1String("Nothing")));
    }

    void dontAskAgainLastsForThatFileOnly()
    {
        int asked = 0;
        setLossyStandIn([&](const LossyAsk &, bool *dontAsk) {
            ++asked;
            *dontAsk = true;
            return LossyChoice::SaveAnyway;
        });
        const QStringList lost = {QStringLiteral("Sections")};
        const LossyAsk one{QStringLiteral("/tmp/x/a.odp"), QStringLiteral("presentation"), QStringLiteral(".odp"), QStringLiteral(".pptx"), lost};
        LossyAsk two = one;
        two.path = QStringLiteral("/tmp/x/b.odp");
        QCOMPARE(askLossy(nullptr, one), LossyChoice::SaveAnyway);
        QVERIFY(lossyAskedNotAgain(one.path));
        QCOMPARE(askLossy(nullptr, one), LossyChoice::SaveAnyway);
        QCOMPARE(asked, 1);   // (not asked again for that file)
        QVERIFY(!lossyAskedNotAgain(two.path));
        QCOMPARE(askLossy(nullptr, two), LossyChoice::SaveAnyway);
        QCOMPARE(asked, 2);
        lossyForgetAll();
        QVERIFY(!lossyAskedNotAgain(one.path));
    }

    void cancelWithTheBoxTickedIsNotRemembered()
    {
        setLossyStandIn([&](const LossyAsk &, bool *dontAsk) {
            *dontAsk = true;
            return LossyChoice::Cancel;
        });
        askLossy(nullptr, {QStringLiteral("/tmp/x/a.odp"), QStringLiteral("presentation"), QStringLiteral(".odp"), QStringLiteral(".pptx"), {QStringLiteral("Sections")}});
        QVERIFY(!lossyAskedNotAgain(QStringLiteral("/tmp/x/a.odp")));
    }

    void theBoxNamesTheListAndHasTheThreeButtonsAndTheCheckBox()
    {
        QString text;
        QStringList buttons;
        QList<QMessageBox::ButtonRole> roles;
        bool hasCheck = false;
        QTimer finder;
        QObject::connect(&finder, &QTimer::timeout, [&] {
            auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            if (!box) return;
            text = box->text();
            for (QAbstractButton *b : box->buttons()) buttons << b->text();
            for (QAbstractButton *b : box->buttons()) roles << box->buttonRole(b);
            std::sort(roles.begin(), roles.end());
            hasCheck = box->checkBox() && box->checkBox()->text().contains(QLatin1String("again"));
            box->checkBox()->setChecked(true);
            for (QAbstractButton *b : box->buttons())
                if (box->buttonRole(b) == QMessageBox::AcceptRole) b->click();
        });
        finder.start(20);
        const LossyAsk ask{QStringLiteral("/tmp/x/a.odp"), QStringLiteral("presentation"), QStringLiteral(".odp"), QStringLiteral(".pptx"), {QStringLiteral("Sections"), QStringLiteral("Glows")}};
        QCOMPARE(askLossy(nullptr, ask), LossyChoice::SaveAnyway);
        QVERIFY(text.contains(QLatin1String("Sections")) && text.contains(QLatin1String("Glows")) && text.contains(QLatin1String(".odp")));
        QVERIFY(buttons.contains(QStringLiteral("Save Anyway")) && buttons.contains(QStringLiteral("Save as .pptx")));
        QCOMPARE(roles, (QList<QMessageBox::ButtonRole>{QMessageBox::AcceptRole, QMessageBox::RejectRole, QMessageBox::ActionRole}));
        QVERIFY(hasCheck);
        QVERIFY(lossyAskedNotAgain(ask.path));
    }

    void aLongListIsCutAndCounted()
    {
        QStringList many;
        for (int i = 0; i < 40; ++i) many << QStringLiteral("Thing %1").arg(i);
        const QString text = lossyReport({QString(), QStringLiteral("presentation"), QStringLiteral(".odp"), QStringLiteral(".pptx"), many});
        QVERIFY(text.contains(QLatin1String("Thing 11")));
        QVERIFY(!text.contains(QLatin1String("Thing 12")));
        QVERIFY(text.contains(QLatin1String("and 28 more")));
    }

    void theOwnFormatNameIsBesideAndNeverOverwritesAFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString odp = dir.filePath(QStringLiteral("deck.odp"));
        QCOMPARE(lossyOwnFormatPath(odp, QStringLiteral(".pptx")), dir.filePath(QStringLiteral("deck.pptx")));
        QFile f(dir.filePath(QStringLiteral("deck.pptx")));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.close();
        QCOMPARE(lossyOwnFormatPath(odp, QStringLiteral(".pptx")), dir.filePath(QStringLiteral("deck (2).pptx")));
    }

    // The same file is the same file before it is saved and after, and by another way to its folder (a link, as /var is on a Mac, or a short name on Windows).
    void theSameFileHasTheSameKeyBeforeAndAfterItExists()
    {
#ifdef Q_OS_WIN
        QSKIP("a link needs more than a plain folder here");
#else
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("real")));
        QVERIFY(QFile::link(dir.filePath(QStringLiteral("real")), dir.filePath(QStringLiteral("link"))));
        setLossyStandIn([&](const LossyAsk &, bool *dontAsk) {
            *dontAsk = true;
            return LossyChoice::SaveAnyway;
        });
        const QString viaLink = dir.filePath(QStringLiteral("link/a.odp")), real = dir.filePath(QStringLiteral("real/a.odp"));
        // (not saved yet: the file is not there)
        askLossy(nullptr, {viaLink, QStringLiteral("presentation"), QStringLiteral(".odp"), QStringLiteral(".pptx"), {QStringLiteral("Sections")}});
        QVERIFY(lossyAskedNotAgain(viaLink));
        QFile f(real);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.close();
        // (saved: the file is there, and is named by its real folder)
        QVERIFY(lossyAskedNotAgain(real));
        QVERIFY(lossyAskedNotAgain(viaLink));
        QVERIFY(!lossyAskedNotAgain(dir.filePath(QStringLiteral("real/b.odp"))));
#endif
    }

    void pastTheCapTheOldestIsForgottenAndTheNewestKept()
    {
        setLossyStandIn([&](const LossyAsk &, bool *dontAsk) {
            *dontAsk = true;
            return LossyChoice::SaveAnyway;
        });
        const auto ask = [](int n) { return LossyAsk{QStringLiteral("/nonexistent-folder/f%1.odp").arg(n), QStringLiteral("presentation"), QStringLiteral(".odp"), QStringLiteral(".pptx"), {QStringLiteral("Sections")}}; };
        for (int n = 0; n < 1005; ++n) askLossy(nullptr, ask(n));
        QVERIFY(!lossyAskedNotAgain(ask(0).path));
        QVERIFY(!lossyAskedNotAgain(ask(4).path));
        QVERIFY(lossyAskedNotAgain(ask(5).path));
        QVERIFY(lossyAskedNotAgain(ask(1004).path));
    }

    void whatAWindowFoundIsKeptUntilItsDocumentChanges()
    {
        LossyCache cache;
        int worked = 0;
        const auto compute = [&] {
            ++worked;
            return QStringList{QStringLiteral("Sections")};
        };
        QCOMPARE(cache.get(QStringLiteral("odp"), compute), QStringList{QStringLiteral("Sections")});
        cache.get(QStringLiteral("odp"), compute);
        QCOMPARE(worked, 1);
        cache.get(QStringLiteral("otp"), compute);   // (another kind)
        QCOMPARE(worked, 2);
        cache.bump();
        cache.get(QStringLiteral("otp"), compute);
        QCOMPARE(worked, 3);
    }
};

JP_TEST_CLASS(ShellLossyWarnTests)
#include "tst_shell_lossywarn.moc"
