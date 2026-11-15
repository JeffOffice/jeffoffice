// The shared "Set as Default" (libs/shell/app/userdefaults.h): the question's stand-in, the files kept in the app's data folder, and the small sets of choices kept as settings.
#include "app/userdefaults.h"
#include "../testrunner.h"
#include "../withdialog.h"

#include <QFile>
#include <QJsonObject>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include <QtTest>

using namespace jp;

namespace {

class ShellDefaultsTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void theQuestionIsAnsweredByItsStandIn()
    {
        QString asked;
        setDefaultScopeStandIn([&asked](const QString &what) {
            asked = what;
            return DefaultScope::AllNew;
        });
        QCOMPARE(askDefaultScope(nullptr, QStringLiteral("the font")), DefaultScope::AllNew);
        QCOMPARE(asked, QStringLiteral("the font"));
        setDefaultScopeStandIn([](const QString &) { return DefaultScope::ThisDocument; });
        QCOMPARE(askDefaultScope(nullptr, QStringLiteral("x")), DefaultScope::ThisDocument);
        setDefaultScopeStandIn(nullptr);
    }

    // The real box: three buttons, each its own answer (the box is found by a timer, which keeps looking until it is there).
    void theBoxOffersThisDocumentAllNewDocumentsAndCancel()
    {
        setDefaultScopeStandIn(nullptr);
        struct Case {
            const char *button;
            DefaultScope answer;
        };
        for (const Case &c : {Case{"defaultAllNew", DefaultScope::AllNew}, Case{"defaultThisDocument", DefaultScope::ThisDocument}, Case{nullptr, DefaultScope::Cancel}}) {
            QTimer poll;
            QObject::connect(&poll, &QTimer::timeout, [&] {
                auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
                if (!box || box->objectName() != QLatin1String("setAsDefaultBox")) return;
                poll.stop();
                QVERIFY(box->text().contains(QStringLiteral("the font")));
                shotOf(box, QStringLiteral("set-as-default-question"));
                if (c.button) box->findChild<QPushButton *>(QLatin1String(c.button))->click();
                else box->reject();
            });
            poll.start(20);
            QCOMPARE(askDefaultScope(nullptr, QStringLiteral("the font")), c.answer);
        }
    }

    void aFileOfTheDefaultsLivesInTheDataFolder()
    {
        const QString path = userDefaultFile(QStringLiteral("probe.bin"));
        QVERIFY(QFileInfo(path).dir().exists());
        QVERIFY(path.startsWith(userDefaultsFolder()));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("x");
        f.close();
        QVERIFY(QFile::exists(userDefaultFile(QStringLiteral("probe.bin"))));
        QVERIFY(QFile::remove(path));
    }

    void choicesAreKeptUnderANameAndCleared()
    {
        const QString name = QStringLiteral("probeChoices");
        QVERIFY(userDefault(name).isEmpty());
        setUserDefault(name, QJsonObject{{QStringLiteral("size"), 12}, {QStringLiteral("font"), QStringLiteral("Serif")}});
        QCOMPARE(userDefault(name).value(QStringLiteral("size")).toInt(), 12);
        QCOMPARE(userDefault(name).value(QStringLiteral("font")).toString(), QStringLiteral("Serif"));
        QVERIFY(userDefault(QStringLiteral("another")).isEmpty());
        clearUserDefault(name);
        QVERIFY(userDefault(name).isEmpty());
    }
};

} // namespace

JP_TEST_CLASS(ShellDefaultsTests)
#include "tst_shell_defaults.moc"
