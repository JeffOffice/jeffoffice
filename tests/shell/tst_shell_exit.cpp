// Each program, started as a person starts it, closes its window and must be gone soon after, with nothing left running in the background (libs/shell/app/shutdown.h).
#include "app/shutdown.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QProcess>
#include <QSemaphore>
#include <QTemporaryDir>
#include <QThreadPool>
#include <QtTest>

#include <atomic>

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
Result runOnce(const QString &program, const QStringList &arguments, const QString &work, const QByteArray &closeTurns = "3", bool apart = true, int rounds = kRounds)
{
    Result r;
    QDir().mkpath(work + QStringLiteral("/home"));
    QDir().mkpath(work + QStringLiteral("/settings"));
    const QString report = work + QStringLiteral("/report.txt");
    QFile::remove(report);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
    env.insert(QStringLiteral("JP_NO_TELEMETRY"), QStringLiteral("1"));
    if (apart) env.insert(QStringLiteral("JP_SETTINGS_FOLDER"), work + QStringLiteral("/settings"));
    for (const char *name : {"HOME", "USERPROFILE", "APPDATA", "LOCALAPPDATA", "XDG_DATA_HOME", "XDG_CONFIG_HOME", "XDG_CACHE_HOME"})
        env.insert(QString::fromLatin1(name), work + QStringLiteral("/home"));
    env.insert(QStringLiteral("JP_SHUTDOWN_REPORT"), report);
    if (!closeTurns.isEmpty()) env.insert(QStringLiteral("JP_CLOSE_AFTER_TURNS"), QString::fromLatin1(closeTurns));
    QProcess child;
    child.setProcessEnvironment(env);
    child.setProcessChannelMode(QProcess::MergedChannels);
    child.start(program, arguments);
    if (!child.waitForStarted(30000)) return r;
    for (int round = 0; round < rounds && !r.gone; ++round) {
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

// How many settings files the run left in its settings folder.
int settingsFilesIn(const QString &work)
{
    int n = 0;
    QDirIterator it(work + QStringLiteral("/settings"), {QStringLiteral("*.conf"), QStringLiteral("*.ini")}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        ++n;
    }
    return n;
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
    // Every way out of main() ends the services and writes the settings, not only the one after the event loop: a program that stops early (a job that makes a file and quits, a file that
    // cannot be opened) used to leave without either.
    void everyWayOutOfMainWritesTheSettings_data()
    {
        QTest::addColumn<QString>("app");
        QTest::addColumn<int>("code");
        QTest::newRow("JeffPub exports a PDF") << "JeffPub" << 0;
        QTest::newRow("JeffDeck cannot open its file") << "JeffDeck" << 1;
        // (JeffWrite and JeffCrunch have no way out of main() before the event loop that a run with nobody to answer a box can take: tools/check_static_threads.py checks that their mains have the same guard)
    }
    void everyWayOutOfMainWritesTheSettings()
    {
        QFETCH(QString, app);
        QFETCH(int, code);
        const QString program = programPath(app);
        QVERIFY2(!program.isEmpty(), qPrintable(app + QStringLiteral(" was not built beside the test")));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString out = dir.filePath(QStringLiteral("out.") + (app == QLatin1String("JeffPub") ? QStringLiteral("pdf") : QStringLiteral("png")));
        const QStringList arguments = app == QLatin1String("JeffPub") ? QStringList{QStringLiteral("--export-pdf"), out}
                                                                        : QStringList{QStringLiteral("--screenshot"), out, dir.filePath(QStringLiteral("missing.file"))};
        const Result r = runOnce(program, arguments, dir.path(), QByteArray());
        QVERIFY(r.gone);
        QCOMPARE(r.code, code);
        QVERIFY2(r.report.startsWith("threads=0 steps="), r.report.constData());   // (the shutdown ran on this way out)
        QVERIFY2(settingsFilesIn(dir.path()) > 0, "the settings were not written");
    }

    // The report file and the self-closing window are for a test run only: a person's program never writes to a path from its environment, and never closes itself.
    void aPersonsRunHonorsNeitherTestVariable()
    {
#if !defined(Q_OS_LINUX)
        QSKIP("a run that is not kept apart would use the system's own settings store here");
#endif
        const QString program = programPath(QStringLiteral("JeffPub"));
        QVERIFY(!program.isEmpty());
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        // (a job that quits by itself: its report is the file)
        Result r = runOnce(program, {QStringLiteral("--export-pdf"), dir.filePath(QStringLiteral("out.pdf"))}, dir.path(), QByteArray(), false);
        QVERIFY(r.gone);
        QCOMPARE(r.code, 0);
        QVERIFY2(!QFileInfo::exists(dir.filePath(QStringLiteral("report.txt"))), "a person's run wrote the report file");
        // (a window that stays: it is still open after a counted number of waits, and is then ended by the test)
        QTemporaryDir other;
        QVERIFY(other.isValid());
        r = runOnce(program, {}, other.path(), "3", false, 40);
        QVERIFY2(!r.gone, "a person's run closed its own window");
    }

    // The machinery itself, with a clock the test counts: no test waits.
    void stepsRunInTheOppositeOrderOfTheirAdding()
    {
        QList<int> order;
        qint64 clock = 0;
        std::vector<jp::ShutdownStep> steps;
        for (int i = 1; i <= 3; ++i)
            steps.push_back([&order, i](int) { order.append(i); return 0; });
        QCOMPARE(jp::runShutdownSteps(steps, jp::ShutdownLimits(), [&clock] { return clock; }), 0);
        QCOMPARE(order, (QList<int>{3, 2, 1}));
    }

    void aStepThatOverrunsIsAbandonedAtItsCap()
    {
        qint64 clock = 0;
        std::vector<int> given;
        std::vector<jp::ShutdownStep> steps;
        steps.push_back([](int) { return 0; });
        steps.push_back([&clock](int budget) { clock += budget; return 1; });   // (uses all it is given, and is still running: let go of)
        steps.push_back([](int) { return 0; });
        QCOMPARE(jp::runShutdownSteps(steps, jp::ShutdownLimits(), [&clock] { return clock; }, &given), 1);
        QCOMPARE(given, (std::vector<int>{2000, 2000, 2000}));   // (the step after the overrunning one still gets its own cap)
        QCOMPARE(clock, qint64(2000));
    }

    void aStepThatEndsEarlyLeavesItsTimeToTheOthers()
    {
        qint64 clock = 0;
        std::vector<int> given;
        std::vector<jp::ShutdownStep> steps;
        for (int i = 0; i < 4; ++i)
            steps.push_back([&clock](int) { clock += 100; return 0; });
        QCOMPARE(jp::runShutdownSteps(steps, jp::ShutdownLimits(), [&clock] { return clock; }, &given), 0);
        QCOMPARE(given, (std::vector<int>{2000, 2000, 2000, 2000}));
        QCOMPARE(clock, qint64(400));
    }

    void theWholeIsCappedAndTheLastStepsGetNothing()
    {
        qint64 clock = 0;
        std::vector<int> given;
        std::vector<jp::ShutdownStep> steps;
        for (int i = 0; i < 5; ++i)
            steps.push_back([&clock](int budget) { clock += budget; return 1; });
        QCOMPARE(jp::runShutdownSteps(steps, jp::ShutdownLimits(), [&clock] { return clock; }, &given), 5);
        QCOMPARE(given, (std::vector<int>{2000, 2000, 2000, 0, 0}));   // (6 s for the whole, 2 s at the most for one; a step with 0 left is still asked, and lets go at once)
        QCOMPARE(clock, qint64(6000));
    }

    void aStepAddedTwiceRunsTwiceAndAServiceEndsOnce()
    {
        int plain = 0, last = 0;
        const jp::ShutdownStep step = [&plain](int) { ++plain; return 0; };
        jp::addShutdownStep(step);
        jp::addShutdownStep(step);
        jp::addLastShutdownStep([&last](int) { ++last; return 0; });
        QCOMPARE(jp::shutdownServices(), 0);
        QCOMPARE(plain, 2);   // (adding twice is two steps: an owner adds its step once, when it starts)
        QCOMPARE(last, 1);
        QCOMPARE(jp::shutdownServices(), 0);   // (a second call, as when main is left twice, runs nothing again)
        QCOMPARE(plain, 2);
        QCOMPARE(last, 1);
    }

    void thePoolIsWaitedForBeforeTheLastSteps()
    {
        QSemaphore gate, inside;
        std::atomic<bool> done(false);
        std::atomic<int> activeInLast(-1);
        QThreadPool::globalInstance()->start([&] {
            inside.release();
            gate.acquire();
            volatile int sink = 0;
            for (int i = 0; i < 20000000; ++i) sink = sink + i;   // (counted work, so the job is still running when the step that let it go returns)
            done = true;
        });
        QVERIFY(inside.tryAcquire(1, 30000));
        jp::addShutdownStep([&gate](int) { gate.release(); return 0; });
        jp::addLastShutdownStep([&](int) { activeInLast = QThreadPool::globalInstance()->activeThreadCount() + (done ? 0 : 1000); return 0; });
        QCOMPARE(jp::shutdownServices(), 0);
        QCOMPARE(activeInLast.load(), 0);   // (nothing is on the pool, and its work is done, when the settings are written)
    }
    void aStepThatLetGoOfAThreadIsRemembered()
    {
        QVERIFY(!jp::shutdownLetGo());
        jp::addShutdownStep([](int) { return 1; });
        QCOMPARE(jp::shutdownServices(), 1);
        QVERIFY(jp::shutdownLetGo());   // (runProgram() then leaves without destroying a static)
    }
};

QTEST_MAIN(ExitTests)
#include "tst_shell_exit.moc"
