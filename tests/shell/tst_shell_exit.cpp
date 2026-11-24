// Each program, started as a person starts it, closes its window and must be gone soon after, with nothing left running in the background (libs/shell/app/shutdown.h).
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTemporaryDir>
#include <QtTest>

namespace {

// How long a program has to end: a count of waits, each of a quarter of a second at the most (a process that is gone ends a wait at once). A program that is not gone in this many
// is stuck, the failure these tests are for.
constexpr int kRounds = 240;
constexpr int kLaunches = 20;

QString programPath(const QString &name)
{
    const QString dir = QCoreApplication::applicationDirPath();
    for (const QString &c : {dir + QLatin1Char('/') + name, dir + QLatin1Char('/') + name + QStringLiteral(".exe"), dir + QLatin1Char('/') + name + QStringLiteral(".app/Contents/MacOS/") + name})
        if (QFileInfo(c).isFile()) return c;
    return QString();
}

struct Result {
    bool gone = false;
    QProcess::ExitStatus status = QProcess::CrashExit;
    int code = -1;
    QByteArray report;
};

// Starts the program on a folder of its own (settings, recovery copies, and data), and asks it to close its window after a few turns of its event loop, when its background services
// have been started.
Result runOnce(const QString &program, const QStringList &arguments, const QString &work, const QByteArray &closeTurns = "3")
{
    Result r;
    QDir().mkpath(work + QStringLiteral("/home"));
    QDir().mkpath(work + QStringLiteral("/settings"));
    const QString report = work + QStringLiteral("/report.txt");
    QFile::remove(report);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
    env.insert(QStringLiteral("JP_NO_TELEMETRY"), QStringLiteral("1"));
    env.insert(QStringLiteral("JP_SETTINGS_FOLDER"), work + QStringLiteral("/settings"));
    for (const char *name : {"HOME", "USERPROFILE", "APPDATA", "LOCALAPPDATA", "XDG_DATA_HOME", "XDG_CONFIG_HOME", "XDG_CACHE_HOME"})
        env.insert(QString::fromLatin1(name), work + QStringLiteral("/home"));
    env.insert(QStringLiteral("JP_SHUTDOWN_REPORT"), report);
    if (!closeTurns.isEmpty()) env.insert(QStringLiteral("JP_CLOSE_AFTER_TURNS"), QString::fromLatin1(closeTurns));
    QProcess child;
    child.setProcessEnvironment(env);
    child.setProcessChannelMode(QProcess::MergedChannels);
    child.start(program, arguments);
    if (!child.waitForStarted(30000)) return r;
    for (int round = 0; round < kRounds && !r.gone; ++round) {
        child.readAll();   // (what it says is not kept, and its pipe never fills)
        r.gone = child.waitForFinished(250);
    }
    if (!r.gone) {
        child.kill();
        child.waitForFinished(10000);
    }
    r.status = child.exitStatus();
    r.code = child.exitCode();
    QFile f(report);
    if (f.open(QIODevice::ReadOnly)) r.report = f.readAll().trimmed();
    return r;
}

} // namespace

class ExitTests : public QObject {
    Q_OBJECT

    static void rows()
    {
        QTest::addColumn<QString>("app");
        QTest::addColumn<bool>("withFile");
        QTest::addColumn<int>("steps");   // (the least number of background services the program must have ended: the settings, the dictionary's watcher, and for JeffCrunch the loader)
        QTest::newRow("JeffWrite") << "JeffWrite" << true << 2;
        QTest::newRow("JeffCrunch") << "JeffCrunch" << false << 3;
        QTest::newRow("JeffDeck") << "JeffDeck" << false << 2;
        QTest::newRow("JeffPub") << "JeffPub" << false << 2;
    }

    // The program's arguments: JeffWrite is given a small text file to open (the others start on a new document of their own).
    static QStringList argumentsFor(bool withFile, const QString &work)
    {
        if (!withFile) return {};
        const QString path = work + QStringLiteral("/small.txt");
        QFile f(path);
        if (f.open(QIODevice::WriteOnly)) f.write("A short documnet with a misspelt word.\nA second line of text.\n");
        return {path};
    }

private Q_SLOTS:
    void theProgramsAreGoneSoonAfterTheirWindowIsClosed_data() { rows(); }
    void theProgramsAreGoneSoonAfterTheirWindowIsClosed()
    {
        QFETCH(QString, app);
        QFETCH(bool, withFile);
        const QString program = programPath(app);
        QVERIFY2(!program.isEmpty(), qPrintable(app + QStringLiteral(" was not built beside the test")));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        for (int i = 0; i < kLaunches; ++i) {
            const QString work = dir.filePath(QString::number(i));
            QDir().mkpath(work);
            const Result r = runOnce(program, argumentsFor(withFile, work), work);
            QVERIFY2(r.gone, qPrintable(QStringLiteral("%1 was still running after its window closed (launch %2)").arg(app).arg(i)));
            QCOMPARE(r.status, QProcess::NormalExit);
            QCOMPARE(r.code, 0);
        }
    }

    void nothingIsRunningInTheBackgroundWhenMainReturns_data() { rows(); }
    void nothingIsRunningInTheBackgroundWhenMainReturns()
    {
        QFETCH(QString, app);
        QFETCH(bool, withFile);
        QFETCH(int, steps);
        const QString program = programPath(app);
        QVERIFY2(!program.isEmpty(), qPrintable(app + QStringLiteral(" was not built beside the test")));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const Result r = runOnce(program, argumentsFor(withFile, dir.path()), dir.path());
        QVERIFY(r.gone);
        QCOMPARE(r.code, 0);
        // The program's own count of the threads it had to let go of, and of the steps that ended something.
        const QList<QByteArray> parts = r.report.split(' ');
        QCOMPARE(parts.value(0), QByteArray("threads=0"));
        QVERIFY2(parts.value(1).startsWith("steps="), r.report.constData());
        QVERIFY2(parts.value(1).mid(6).toInt() >= steps, r.report.constData());
    }
};

QTEST_MAIN(ExitTests)
#include "tst_shell_exit.moc"
