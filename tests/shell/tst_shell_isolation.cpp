// A program the tests start (an app made to take a screenshot) never reads or writes the person's settings or data: every app, in the --screenshot branch and in the JP_TESTING branch.
#include "../testrunner.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QImage>
#include <QProcess>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

namespace {

struct AppExe {
    QString name, exe;
};

// Everything under a folder, as relative names (folders too) with the bytes of each file, to see that a run changed nothing.
QMap<QString, QByteArray> snapshot(const QString &top)
{
    QMap<QString, QByteArray> all;
    const QDir root(top);
    for (QDirIterator it(top, QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden, QDirIterator::Subdirectories); it.hasNext();) {
        const QString path = it.next();
        QFile f(path);
        all.insert(root.relativeFilePath(path), it.fileInfo().isFile() && f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray("<folder>"));
    }
    return all;
}

// The environment of a run that stands in for the person's: its home, config, data, cache, and temporary folders are the given ones, and nothing names a settings folder unless asked.
QProcessEnvironment personsEnvironment(const QString &home, const QString &tmp)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove(QStringLiteral("JP_TESTING"));
    env.remove(QStringLiteral("JP_SETTINGS_FOLDER"));
    env.insert(QStringLiteral("HOME"), home);
    env.insert(QStringLiteral("USERPROFILE"), home);
    env.insert(QStringLiteral("APPDATA"), home + QStringLiteral("/AppData/Roaming"));
    env.insert(QStringLiteral("LOCALAPPDATA"), home + QStringLiteral("/AppData/Local"));
    env.insert(QStringLiteral("XDG_CONFIG_HOME"), home + QStringLiteral("/.config"));
    env.insert(QStringLiteral("XDG_DATA_HOME"), home + QStringLiteral("/.local/share"));
    env.insert(QStringLiteral("XDG_CACHE_HOME"), home + QStringLiteral("/.cache"));
    env.insert(QStringLiteral("XDG_STATE_HOME"), home + QStringLiteral("/.local/state"));
    env.insert(QStringLiteral("TMPDIR"), tmp);
    env.insert(QStringLiteral("TMP"), tmp);
    env.insert(QStringLiteral("TEMP"), tmp);
    env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
    env.insert(QStringLiteral("JP_NO_TELEMETRY"), QStringLiteral("1"));
    return env;
}

// Runs the program; returns its exit code (-1 when it did not start, did not end in time, or crashed).
int run(const QString &exe, const QProcessEnvironment &env, const QStringList &args)
{
    QProcess child;
    child.setProcessEnvironment(env);
    child.setProcessChannelMode(QProcess::MergedChannels);
    child.start(exe, args);
    if (!child.waitForStarted()) return -1;
    if (!child.waitForFinished(120000)) {
        child.kill();
        child.waitForFinished();
        return -1;
    }
    return child.exitStatus() == QProcess::NormalExit ? child.exitCode() : -1;
}

QStringList shotArgs(const QString &png) { return {QStringLiteral("--screenshot"), png, QStringLiteral("--size"), QStringLiteral("900x600"), QStringLiteral("--backstage"), QStringLiteral("open")}; }

class ShellIsolationTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void anAppRunKeepsApartFromThePersonsSettingsAndData_data()
    {
        QTest::addColumn<QString>("name");
        QTest::addColumn<QString>("exe");
        QTest::newRow("JeffPub") << "JeffPub" << QStringLiteral(JO_EXE_PUB);
        QTest::newRow("JeffDeck") << "JeffDeck" << QStringLiteral(JO_EXE_DECK);
        QTest::newRow("JeffWrite") << "JeffWrite" << QStringLiteral(JO_EXE_WRITE);
        QTest::newRow("JeffCrunch") << "JeffCrunch" << QStringLiteral(JO_EXE_CRUNCH);
    }
    // In a home with settings of the app's own (a Recent list), a screenshot run shows what the copy of them holds and changes nothing of the home (its config, data, and cache), a run with JP_TESTING
    // starts from nothing and changes nothing, and a run that is asked to keep apart and cannot says so and stops.
    void anAppRunKeepsApartFromThePersonsSettingsAndData()
    {
        QFETCH(QString, name);
        QFETCH(QString, exe);
        QVERIFY2(QFileInfo::exists(exe), qPrintable(exe));
        QTemporaryDir work;   // (the pictures, and the temporary folder the runs make theirs in)
        QVERIFY(work.isValid());
        const QString tmp = work.filePath(QStringLiteral("tmp"));
        QVERIFY(QDir().mkpath(tmp));
        // (a file that is there: the Recent list leaves out one that is not)
        const QString marked = work.filePath(QStringLiteral("seeded-marker-file.ext"));
        QFile(marked).open(QIODevice::WriteOnly);
        auto homeWith = [&](const QString &folder, bool seeded) {
            const QString home = work.filePath(folder);
            QDir().mkpath(home + QStringLiteral("/.config/JeffOffice"));
            if (seeded) {
                QFile f(home + QStringLiteral("/.config/JeffOffice/") + name + QStringLiteral(".conf"));
                if (f.open(QIODevice::WriteOnly)) f.write("[General]\nrecent=" + marked.toUtf8() + "\n");
            }
            return home;
        };
        // what the picture of the Open page is with nothing in the Recent list
        const QString bare = homeWith(QStringLiteral("bare"), false);
        const QMap<QString, QByteArray> bareBefore = snapshot(bare);
        QCOMPARE(run(exe, personsEnvironment(bare, tmp), shotArgs(work.filePath(QStringLiteral("bare.png")))), 0);
        QCOMPARE(snapshot(bare), bareBefore);
        const QImage nothing(work.filePath(QStringLiteral("bare.png")));
        QVERIFY(!nothing.isNull());

        // a screenshot: the copy of the settings is seen, the home is not changed
        const QString shotHome = homeWith(QStringLiteral("shot"), true);
        const QMap<QString, QByteArray> shotBefore = snapshot(shotHome);
        QCOMPARE(run(exe, personsEnvironment(shotHome, tmp), shotArgs(work.filePath(QStringLiteral("shot.png")))), 0);
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
        QVERIFY2(QImage(work.filePath(QStringLiteral("shot.png"))) != nothing, "the screenshot run did not start from a copy of the person's settings");
#endif
        QCOMPARE(snapshot(shotHome), shotBefore);

        // JP_TESTING: no copy is made, and the home is not changed
        const QString testHome = homeWith(QStringLiteral("testing"), true);
        const QMap<QString, QByteArray> testBefore = snapshot(testHome);
        QProcessEnvironment testing = personsEnvironment(testHome, tmp);
        testing.insert(QStringLiteral("JP_TESTING"), QStringLiteral("1"));
        QCOMPARE(run(exe, testing, shotArgs(work.filePath(QStringLiteral("testing.png")))), 0);
        QVERIFY2(QImage(work.filePath(QStringLiteral("testing.png"))) == nothing, "a run under JP_TESTING read the person's settings");
        QCOMPARE(snapshot(testHome), testBefore);

        // a normal launch makes no temporary folder (here it only reports its version)
        const QString quietTmp = work.filePath(QStringLiteral("quiet"));
        QVERIFY(QDir().mkpath(quietTmp));
        QCOMPARE(run(exe, personsEnvironment(homeWith(QStringLiteral("quiet-home"), false), quietTmp), {QStringLiteral("--version")}), 0);
        QVERIFY2(QDir(quietTmp).isEmpty(), "a normal launch made a temporary folder");

        // a run asked to keep apart in a folder that cannot be made stops with an error, makes no picture, and touches nothing
        QFile blocker(work.filePath(QStringLiteral("blocker")));
        QVERIFY(blocker.open(QIODevice::WriteOnly));
        blocker.close();
        QProcessEnvironment stuck = personsEnvironment(shotHome, tmp);
        stuck.insert(QStringLiteral("JP_SETTINGS_FOLDER"), work.filePath(QStringLiteral("blocker/inside")));
        const QString noPicture = work.filePath(QStringLiteral("none.png"));
        QVERIFY(run(exe, stuck, shotArgs(noPicture)) > 0);
        QVERIFY(!QFileInfo::exists(noPicture));
        QCOMPARE(snapshot(shotHome), shotBefore);
#if !(defined(Q_OS_UNIX) && !defined(Q_OS_MACOS))
        QSKIP("Here the settings are the registry or the preferences, with no file to copy: the copy was not checked (everything else was).");
#endif
    }

    // JeffPub's move of the settings and files it kept under its old name never runs in a run that is kept apart: the folder named for the run gets no mark that it ran.
    void jeffPubDoesNotMoveOldSettingsInARunKeptApart()
    {
        QTemporaryDir work, named;
        QVERIFY(work.isValid() && named.isValid());
        const QString tmp = work.filePath(QStringLiteral("tmp"));
        QVERIFY(QDir().mkpath(tmp));
        QVERIFY(QDir().mkpath(named.filePath(QStringLiteral("JeffPub79"))));
        for (const char *suffix : {".conf", ".ini"}) {   // (the file the run reads, whichever the system's kind of settings file is)
            QFile old(named.filePath(QStringLiteral("JeffPub79/JeffPub79") + QLatin1String(suffix)));
            QVERIFY(old.open(QIODevice::WriteOnly));
            old.write("[General]\nunits=2\n");
        }
        QProcessEnvironment env = personsEnvironment(work.filePath(QStringLiteral("home")), tmp);
        env.insert(QStringLiteral("JP_SETTINGS_FOLDER"), named.path());
        QCOMPARE(run(QStringLiteral(JO_EXE_PUB), env, shotArgs(work.filePath(QStringLiteral("pub.png")))), 0);
        const QSettings now(named.filePath(QStringLiteral("JeffOffice/JeffPub.conf")), QSettings::IniFormat);
        QVERIFY2(!now.contains(QStringLiteral("migration/jeffpub79")), "the move from the old name ran in a run that is kept apart");
        QVERIFY(!now.contains(QStringLiteral("units")));
    }
};

} // namespace

JP_TEST_CLASS(ShellIsolationTests)

#include "tst_shell_isolation.moc"
