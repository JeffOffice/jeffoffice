// The warning before a save to a kind of file that can't hold everything (libs/shell/app/lossywarning.h): it shows the list it is given, each answer is what it says, a list with nothing in it
// shows nothing, and "don't ask again for this file" lasts for the session.
#include "app/lossywarning.h"
#include "../testrunner.h"

#include <QCheckBox>
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
        bool hasCheck = false;
        QTimer finder;
        QObject::connect(&finder, &QTimer::timeout, [&] {
            auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            if (!box) return;
            text = box->text();
            for (QAbstractButton *b : box->buttons()) buttons << b->text();
            hasCheck = box->checkBox() && box->checkBox()->text().contains(QLatin1String("again"));
            box->checkBox()->setChecked(true);
            for (QAbstractButton *b : box->buttons())
                if (b->text() == QLatin1String("Save Anyway")) b->click();
        });
        finder.start(20);
        const LossyAsk ask{QStringLiteral("/tmp/x/a.odp"), QStringLiteral("presentation"), QStringLiteral(".odp"), QStringLiteral(".pptx"), {QStringLiteral("Sections"), QStringLiteral("Glows")}};
        QCOMPARE(askLossy(nullptr, ask), LossyChoice::SaveAnyway);
        QVERIFY(text.contains(QLatin1String("Sections")) && text.contains(QLatin1String("Glows")) && text.contains(QLatin1String(".odp")));
        QVERIFY(buttons.contains(QStringLiteral("Save Anyway")) && buttons.contains(QStringLiteral("Save as .pptx")));
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
};

JP_TEST_CLASS(ShellLossyWarnTests)
#include "tst_shell_lossywarn.moc"
