// JeffPub's tests of telemetry, crash reports, updates, and shared settings.
#include "pubhelpers.h"
#include <QScopeGuard>

class PubUpdateTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    void telemetryCountsOnlyWhenOn()
    {
        using namespace jp;
        telemetry::setEnabled(false);
        QVERIFY(telemetry::decided());
        telemetry::count(QStringLiteral("file.open.pub"));
        QVERIFY(telemetry::payload()[QStringLiteral("counts")].toObject().isEmpty());
        telemetry::setEnabled(true);
        telemetry::count(QStringLiteral("file.open.pub"));
        telemetry::count(QStringLiteral("file.open.pub"));
        telemetry::count(QStringLiteral("wrap.topBottom"));
        const QJsonObject p = telemetry::payload();
        const QJsonObject counts = p[QStringLiteral("counts")].toObject();
        QCOMPARE(counts[QStringLiteral("file.open.pub")].toInt(), 2);
        QCOMPARE(counts[QStringLiteral("wrap.topbottom")].toInt(), 1);
        QStringList keys = p.keys();
        keys.sort();
        QCOMPARE(keys, (QStringList{"app", "counts", "install", "lang", "launches", "os", "osVersion", "version"}));
        QCOMPARE(p[QStringLiteral("app")].toString(), QStringLiteral("jeffpub"));
        QVERIFY(QRegularExpression(QStringLiteral("^[a-z]{1,16}$")).match(p[QStringLiteral("app")].toString()).hasMatch());   // as the collector checks
        const QString id = p[QStringLiteral("install")].toString();
        QVERIFY(!QUuid::fromString(id).isNull());
        // What the collector checks (server/telemetry/main.py).
        QVERIFY(QRegularExpression(QStringLiteral("^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$")).match(id).hasMatch());
        QVERIFY(QRegularExpression(QStringLiteral("^[a-z]{1,16}$")).match(p[QStringLiteral("os")].toString()).hasMatch());
        QVERIFY(QRegularExpression(QStringLiteral("^[0-9A-Za-z .()_+-]{0,40}$")).match(p[QStringLiteral("osVersion")].toString()).hasMatch());
        QVERIFY(QRegularExpression(QStringLiteral("^[A-Za-z_-]{0,20}$")).match(p[QStringLiteral("lang")].toString()).hasMatch());
        QVERIFY(QRegularExpression(QStringLiteral("^[0-9A-Za-z.+-]{1,20}$")).match(p[QStringLiteral("version")].toString()).hasMatch());
        for (const QString &k : counts.keys()) QVERIFY(QRegularExpression(QStringLiteral("^[a-z0-9][a-z0-9._-]{0,47}$")).match(k).hasMatch());
        QCOMPARE(telemetry::payload()[QStringLiteral("install")].toString(), id);   // the same install each time
        // Once a day, and again the day an update is installed (0.1.37 had
        // pinged that day, so 0.1.38 waited until the next). The test runner
        // turns statistics off for the whole run; due() only answers, it never
        // sends, so the switch is lifted for these checks and put back after.
        const QByteArray noTelemetry = qgetenv("JP_NO_TELEMETRY");
        const bool hadNoTelemetry = qEnvironmentVariableIsSet("JP_NO_TELEMETRY");
        qunsetenv("JP_NO_TELEMETRY");
        const auto restore = qScopeGuard([&] { if (hadNoTelemetry) qputenv("JP_NO_TELEMETRY", noTelemetry); });
        Settings::get().setValue(QStringLiteral("telemetry/lastSent"), QDate::currentDate());
        Settings::get().setValue(QStringLiteral("telemetry/lastVersion"), QStringLiteral(JP_VERSION));
        QVERIFY(!telemetry::due());
        Settings::get().setValue(QStringLiteral("telemetry/lastVersion"), QVariant());   // before 0.1.39 none was kept
        QVERIFY(telemetry::due());
        Settings::get().setValue(QStringLiteral("telemetry/lastVersion"), QStringLiteral("0.1.1"));
        QVERIFY(telemetry::due());
        Settings::get().setValue(QStringLiteral("telemetry/lastVersion"), QStringLiteral(JP_VERSION));
        Settings::get().setValue(QStringLiteral("telemetry/lastSent"), QDate::currentDate().addDays(-1));
        QVERIFY(telemetry::due());
        telemetry::setEnabled(false);
        QVERIFY(!telemetry::due());
        QVERIFY(telemetry::payload()[QStringLiteral("counts")].toObject().isEmpty());
        QVERIFY(telemetry::payload()[QStringLiteral("install")].toString() != id);
        telemetry::setEnabled(false);
    }

    // A ping says which app sent it. The choice to share and the install id are the suite's, the same in every app; the counts, the
    // launches, and the day last sent are each app's own, so JeffPub's ping never carries what JeffDeck counted.
    void telemetryPingsNameTheirAppAndKeepOwnCounts()
    {
        using namespace jp;
        telemetry::setEnabled(false);   // from nothing counted
        telemetry::setEnabled(true);
        telemetry::count(QStringLiteral("file.open.pub"));
        Settings::get().sync();
        QSettings suite(QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice")), pub(QStringLiteral("JeffOffice"), QStringLiteral("JeffPub")),
            deck(QStringLiteral("JeffOffice"), QStringLiteral("JeffDeck"));
        const QJsonObject p = telemetry::payload();
        QCOMPARE(p[QStringLiteral("app")].toString(), QStringLiteral("jeffpub"));
        QVERIFY(suite.value(QStringLiteral("telemetry/enabled")).toBool());
        QCOMPARE(suite.value(QStringLiteral("telemetry/install")).toString(), p[QStringLiteral("install")].toString());
        QVERIFY(!pub.contains(QStringLiteral("telemetry/install")));
        QCOMPARE(pub.value(QStringLiteral("telemetry/counts")).toMap().value(QStringLiteral("file.open.pub")).toInt(), 1);
        QVERIFY(!suite.contains(QStringLiteral("telemetry/counts")));
        // What JeffDeck counted, in its own settings, is not in JeffPub's ping.
        deck.setValue(QStringLiteral("telemetry/counts"), QVariantMap{{QStringLiteral("file.open.pptx"), 5}});
        deck.setValue(QStringLiteral("telemetry/launches"), 7);
        deck.sync();
        Settings::get().sync();
        const QJsonObject again = telemetry::payload();
        QCOMPARE(again[QStringLiteral("counts")].toObject().keys(), QStringList{QStringLiteral("file.open.pub")});
        QCOMPARE(again[QStringLiteral("launches")].toInt(), 0);
        telemetry::setEnabled(false);
        deck.remove(QStringLiteral("telemetry"));
    }

    // Another app turning statistics off and on again gives the suite a new install id. What this app kept for the old id
    // is not sent under the new one.
    void statisticsKeptForAnOldInstallAreNotSentUnderANewOne()
    {
        using namespace jp;
        Settings &st = Settings::get();
        telemetry::setEnabled(false);
        telemetry::setEnabled(true);
        telemetry::count(QStringLiteral("file.open.pub"));
        st.setValue(QStringLiteral("telemetry/launches"), 2);
        st.setValue(QStringLiteral("telemetry/lastSent"), QDate::currentDate());
        const QJsonObject before = telemetry::payload();
        QCOMPARE(before[QStringLiteral("counts")].toObject()[QStringLiteral("file.open.pub")].toInt(), 1);
        QCOMPARE(before[QStringLiteral("launches")].toInt(), 2);
        // What the other app's setEnabled does to the suite's settings, which this app's own can't be reached by.
        st.setValue(QStringLiteral("telemetry/install"), QVariant());
        const QJsonObject after = telemetry::payload();
        QVERIFY(after[QStringLiteral("install")].toString() != before[QStringLiteral("install")].toString());
        QVERIFY(after[QStringLiteral("counts")].toObject().isEmpty());
        QCOMPARE(after[QStringLiteral("launches")].toInt(), 0);
        QVERIFY(!st.value(QStringLiteral("telemetry/lastSent")).isValid());   // the new install has not sent today
        telemetry::count(QStringLiteral("wrap.topbottom"));
        QCOMPARE(telemetry::payload()[QStringLiteral("counts")].toObject().keys(), QStringList{QStringLiteral("wrap.topbottom")});
        // The same install keeps what it counted.
        telemetry::count(QStringLiteral("wrap.topbottom"));
        QCOMPARE(telemetry::payload()[QStringLiteral("counts")].toObject()[QStringLiteral("wrap.topbottom")].toInt(), 2);
        telemetry::setEnabled(false);
    }

    // A JeffPub from before JeffDeck sent statistics kept its counts without noting whose they were: they are the current
    // install's, kept, and sent once.
    void countsKeptBeforeTheyWereTiedToAnInstallAreKept()
    {
        using namespace jp;
        telemetry::setEnabled(false);
        telemetry::setEnabled(true);
        Settings::get().setValue(QStringLiteral("telemetry/counts"), QVariantMap{{QStringLiteral("file.open.pub"), 3}});
        Settings::get().setValue(QStringLiteral("telemetry/launches"), 4);
        Settings::get().setValue(QStringLiteral("telemetry/collectedFor"), QVariant());
        const QJsonObject p = telemetry::payload();
        QCOMPARE(p[QStringLiteral("counts")].toObject()[QStringLiteral("file.open.pub")].toInt(), 3);
        QCOMPARE(p[QStringLiteral("launches")].toInt(), 4);
        QCOMPARE(Settings::get().value(QStringLiteral("telemetry/collectedFor")).toString(), p[QStringLiteral("install")].toString());
        QCOMPARE(telemetry::payload()[QStringLiteral("counts")].toObject()[QStringLiteral("file.open.pub")].toInt(), 3);
        telemetry::setEnabled(false);
        QVERIFY(!Settings::get().value(QStringLiteral("telemetry/collectedFor")).isValid());   // turned off: nothing kept
    }

    // The install id was each app's own until JeffDeck sent statistics too. It moves to the suite's settings, once, and
    // leaves the app's: turning statistics off clears the suite's, and no copy stays where that can't reach it.
    void theInstallIdMovesToTheSuite()
    {
        const QString first = QStringLiteral("0f8fad5b-d9cb-469f-a165-70867728950e"), second = QStringLiteral("5b0a1c9e-3c4e-4f60-9d6b-0d9f4c4f7a11");
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        {
            // JeffPub 0.7.0 had moved its other keys already, and kept its id and its counts itself.
            QSettings mine(dir.filePath("app.ini"), QSettings::IniFormat), all(dir.filePath("suite.ini"), QSettings::IniFormat);
            mine.setValue("migration/suite", true);
            mine.setValue("telemetry/install", first);
            mine.setValue("telemetry/counts", QVariantMap{{"file.open.pub", 3}});
            all.setValue("telemetry/enabled", true);
            jp::moveToSuite(mine, all);
            QCOMPARE(all.value("telemetry/install").toString(), first);
            QVERIFY(!mine.contains("telemetry/install"));
            QCOMPARE(mine.value("telemetry/counts").toMap().value("file.open.pub").toInt(), 3);   // the app's own, left alone
            QVERIFY(!all.contains("telemetry/counts"));
            // Once: an id the app makes later is not moved, and the suite's own is never replaced.
            mine.setValue("telemetry/install", second);
            all.setValue("telemetry/install", first);
            jp::moveToSuite(mine, all);
            QCOMPARE(all.value("telemetry/install").toString(), first);
            QCOMPARE(mine.value("telemetry/install").toString(), second);
        }
        {
            // An app that never moved anything brings the id with the rest; the suite's id (another app's, first) stays.
            QSettings mine(dir.filePath("new.ini"), QSettings::IniFormat), all(dir.filePath("both.ini"), QSettings::IniFormat);
            mine.setValue("telemetry/install", first);
            mine.setValue("ui/theme", 2);
            jp::moveToSuite(mine, all);
            QCOMPARE(all.value("telemetry/install").toString(), first);
            QCOMPARE(all.value("ui/theme").toInt(), 2);
            QVERIFY(!mine.contains("telemetry/install"));
            QSettings third(dir.filePath("third.ini"), QSettings::IniFormat);
            third.setValue("telemetry/install", second);
            jp::moveToSuite(third, all);
            QCOMPARE(all.value("telemetry/install").toString(), first);
            QVERIFY(!third.contains("telemetry/install"));
        }
    }

    // With JeffPub and JeffDeck both open, the suite offers a version once a day: what one app offered, the other leaves for
    // tomorrow. The app that offered it offers it at each start, as ever.
    void anUpdateIsOfferedOnceADayAcrossTheApps()
    {
        using namespace jp;
        Settings &st = Settings::get();
        const QString tag = QStringLiteral("v0.8.0");
        for (const char *k : {"offered", "offeredOn", "offeredBy"}) st.setValue(QStringLiteral("updates/") + QString::fromLatin1(k), QVariant());
        QVERIFY(!Updater::offeredByAnotherAppToday(tag));
        Updater::noteOffered(tag);
        QVERIFY(!Updater::offeredByAnotherAppToday(tag));
        st.sync();
        QSettings suite(QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice"));
        QCOMPARE(suite.value(QStringLiteral("updates/offered")).toString(), tag);
        QCOMPARE(suite.value(QStringLiteral("updates/offeredBy")).toString(), QStringLiteral("jeffpub"));
        QCOMPARE(suite.value(QStringLiteral("updates/offeredOn")).toDate(), QDate::currentDate());
        // JeffDeck offered it today.
        st.setValue(QStringLiteral("updates/offeredBy"), QStringLiteral("jeffdeck"));
        QVERIFY(Updater::offeredByAnotherAppToday(tag));
        QVERIFY(!Updater::offeredByAnotherAppToday(QStringLiteral("v0.8.1")));   // a newer version is a new offer
        QVERIFY(!Updater::offeredByAnotherAppToday(QString()));
        // A day later, it is offered again.
        st.setValue(QStringLiteral("updates/offeredOn"), QDate::currentDate().addDays(-1));
        QVERIFY(!Updater::offeredByAnotherAppToday(tag));
        for (const char *k : {"offered", "offeredOn", "offeredBy"}) st.setValue(QStringLiteral("updates/") + QString::fromLatin1(k), QVariant());
    }

    // A crash leaves a report of where it stopped: the module and offset of
    // each step of the stack, the version, the day, and the signal or
    // exception. Checked by crashing a small program (tests/crashtest.cpp).
    void crashLeavesAReport()
    {
#if defined(__SANITIZE_ADDRESS__)
        QSKIP("The address sanitizer catches crashes itself.");
#endif
        for (const QString &how : {QStringLiteral("segv"), QStringLiteral("abort")}) {
            QTemporaryDir dir;
            QVERIFY(dir.isValid());
            const QString path = dir.filePath(QStringLiteral("reports/crash-report.txt"));
            QProcess p;
            p.start(QStringLiteral(JP_CRASHTEST), {path, how});
            QVERIFY(p.waitForFinished(60000));
            QVERIFY2(p.exitStatus() == QProcess::CrashExit || p.exitCode() != 0, qPrintable(how));
            const QList<QByteArray> where = p.readAllStandardOutput().trimmed().split(' ');
            QCOMPARE(where.size(), 2);
            const QJsonObject r = jp::crash::readReport(path);
            QVERIFY2(!r.isEmpty(), qPrintable(how + QStringLiteral(": no report")));
            QCOMPARE(r[QStringLiteral("version")].toString(), QStringLiteral(JP_VERSION));
            QCOMPARE(r[QStringLiteral("when")].toString(), QDateTime::currentDateTimeUtc().date().toString(Qt::ISODate));
#ifdef Q_OS_WIN
            const bool exception = how == QLatin1String("segv");
            QCOMPARE(r[QStringLiteral("kind")].toString(), exception ? QStringLiteral("exception") : QStringLiteral("signal"));
            QCOMPARE(r[QStringLiteral("code")].toInteger(), exception ? 0xC0000005ll : qint64(SIGABRT));
#else
            QCOMPARE(r[QStringLiteral("kind")].toString(), QStringLiteral("signal"));
            QCOMPARE(r[QStringLiteral("code")].toInteger(), how == QLatin1String("segv") ? qint64(SIGSEGV) : qint64(SIGABRT));
#endif
            // The function that crashed is among the steps, by its module and offset.
            const quint64 at = where[1].toULongLong(nullptr, 16);
            const QJsonArray frames = r[QStringLiteral("frames")].toArray();
            QVERIFY(!frames.isEmpty() && frames.size() <= 64);
            bool found = false;
            QStringList seen;
            for (const QJsonValue &f : frames) {
                const QString module = f[QStringLiteral("module")].toString();
                const quint64 offset = quint64(f[QStringLiteral("offset")].toInteger());
                seen << module + QStringLiteral("+") + QString::number(offset, 16);
                QVERIFY(QRegularExpression(QStringLiteral("^[A-Za-z0-9._+-]{1,64}$")).match(module).hasMatch());
                if (module == QString::fromLatin1(where[0]) && offset >= at && offset < at + 64) found = true;
            }
            QVERIFY2(found, qPrintable(how + QStringLiteral(": ") + QString::fromLatin1(where[0]) + QStringLiteral("+") +
                                       QString::number(at, 16) + QStringLiteral(" not in ") + seen.join(QLatin1Char(' '))));
        }
    }

    // What a report holds is cut to what the collector takes, and only sent
    // while statistics are on.
    void crashReportsAreCheckedAndSentOnlyWhenOn()
    {
        using namespace jp;
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto write = [&](const QByteArray &text) {
            QFile f(dir.filePath(QStringLiteral("r.txt")));
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(text);
        };
        const QByteArray today = QDateTime::currentDateTimeUtc().date().toString(Qt::ISODate).toLatin1();
        write("jeffoffice-crash 1\nversion 0.6.11\nwhen " + today + "\nkind exception\ncode 3221225477\n"
              "frame JeffPub.exe 1a2b\nframe C:\\Users\\pat\\x.dll 10\nframe Qt6Core.dll 1000000000000\nframe libc.so.6 zz\n");
        QJsonObject r = crash::readReport(dir.filePath(QStringLiteral("r.txt")));
        QCOMPARE(r[QStringLiteral("version")].toString(), QStringLiteral("0.6.11"));
        QCOMPARE(r[QStringLiteral("kind")].toString(), QStringLiteral("exception"));
        QCOMPARE(r[QStringLiteral("code")].toInteger(), 3221225477ll);
        const QJsonArray frames = r[QStringLiteral("frames")].toArray();
        QCOMPARE(frames.size(), 4);
        QCOMPARE(frames[0][QStringLiteral("module")].toString(), QStringLiteral("JeffPub.exe"));
        QCOMPARE(frames[0][QStringLiteral("offset")].toInteger(), 0x1a2bll);
        QVERIFY(!frames[1][QStringLiteral("module")].toString().contains(QLatin1Char('\\')));   // a path is never sent
        QCOMPARE(frames[2][QStringLiteral("module")].toString(), QStringLiteral("unknown"));    // past 2^48
        QCOMPARE(frames[3][QStringLiteral("module")].toString(), QStringLiteral("unknown"));    // not a number

        // Not a report, or one missing what the collector needs: nothing.
        write("hello\n");
        QVERIFY(crash::readReport(dir.filePath(QStringLiteral("r.txt"))).isEmpty());
        write("jeffoffice-crash 1\nversion 0.6.11\nwhen " + today + "\nkind fire\ncode 11\n");
        QVERIFY(crash::readReport(dir.filePath(QStringLiteral("r.txt"))).isEmpty());
        QVERIFY(crash::readReport(dir.filePath(QStringLiteral("none.txt"))).isEmpty());

        // Sent with the install and the system while statistics are on;
        // never when they're off or the report is over 30 days old.
        telemetry::setEnabled(false);
        QVERIFY(telemetry::crashPayload(r).isEmpty());
        telemetry::setEnabled(true);
        const QJsonObject sent = telemetry::crashPayload(r);
        QStringList keys = sent.keys();
        keys.sort();
        QCOMPARE(keys, (QStringList{"app", "code", "frames", "install", "kind", "os", "version", "when"}));
        QCOMPARE(sent[QStringLiteral("app")].toString(), QStringLiteral("jeffpub"));
        QCOMPARE(sent[QStringLiteral("install")].toString(), telemetry::payload()[QStringLiteral("install")].toString());
        r[QStringLiteral("when")] = QDateTime::currentDateTimeUtc().date().addDays(-31).toString(Qt::ISODate);
        QVERIFY(telemetry::crashPayload(r).isEmpty());
        telemetry::setEnabled(false);
    }

    // Update check compares versions numerically, previews before releases.
    // Updates are looked for at every launch, even after a check earlier
    // the same day, unless automatic checks are off.
    void updateCheckEveryLaunch()
    {
        jp::Settings &st = jp::Settings::get();
        const QVariant oldAuto = st.value(QStringLiteral("updates/auto")), oldLast = st.value(QStringLiteral("updates/lastCheck"));
        st.setValue(QStringLiteral("updates/lastCheck"), QDate::currentDate());
        st.setValue(QStringLiteral("updates/auto"), true);
        QVERIFY(jp::Updater::checksOnStartup());
        st.setValue(QStringLiteral("updates/auto"), false);
        QVERIFY(!jp::Updater::checksOnStartup());
        st.setValue(QStringLiteral("updates/auto"), oldAuto.isValid() ? oldAuto : QVariant(true));
        st.setValue(QStringLiteral("updates/lastCheck"), oldLast);
    }

    // The update offer shows what is new, formatted, without the install
    // steps or the preview notice the release page carries.
    void updateOfferNotes()
    {
        const QString body = QStringLiteral(
            "**Early preview for testing. Not finished software.** JeffPub is an independent open-source project.\r\n\r\n"
            "### Install\r\n- **Windows:** download **JeffPub-Setup.exe** and run it.\r\n"
            "- **Debian:** run `sudo apt install ./jeffpub_0.1.19_amd64.deb`.\r\n\r\n"
            "JeffPub checks for new versions once a day.\r\n\r\n"
            "### New in preview 19\r\n- **The New Publication page opens categories at once.** Thumbnails fill in.\r\n- Banners are quicker.\r\n");
        const QString h = jp::Updater::releaseHighlights(body);
        QVERIFY(h.startsWith(QLatin1String("**New in preview 19**")));
        QVERIFY(h.contains(QLatin1String("opens categories at once")));
        QVERIFY(h.contains(QLatin1String("- Banners are quicker.")));
        for (const char *gone : {"Early preview", "Install", "sudo apt", "once a day", "\r"}) QVERIFY2(!h.contains(QLatin1String(gone)), gone);
        // Notes without a "New in" section keep their other sections.
        QCOMPARE(jp::Updater::releaseHighlights(QStringLiteral("Preview.\n\n### Install\n- run it\n\n### Changes\n- quicker")),
                 QStringLiteral("**Changes**\n\n- quicker"));

        const QString oldSheet = qApp->styleSheet();
        const QIcon oldIcon = QApplication::windowIcon();
        qApp->setStyleSheet(jp::modernStyleSheet());
        Q_INIT_RESOURCE(resources);   // the app icon lives in the static jpcore library
        QApplication::setWindowIcon(QIcon(QStringLiteral(":/app.png")));
        std::unique_ptr<QDialog> d(jp::Updater::offerDialog(nullptr, QStringLiteral("0.1.19"), h, true, QStringLiteral("https://github.com/JeffOffice/jeffoffice/releases/tag/v0.1.19")));
        d->show();
        QApplication::processEvents();
        auto *notes = d->findChild<QTextBrowser *>();
        QVERIFY(notes);
        const QString shown = notes->toPlainText();
        QVERIFY(shown.contains(QLatin1String("The New Publication page opens categories at once.")));
        QVERIFY(!shown.contains(QLatin1Char('*')));   // Markdown formatted, not shown as text
        QPushButton *now = nullptr;
        for (QPushButton *b : d->findChildren<QPushButton *>())
            if (b->text() == QLatin1String("Update Now")) now = b;
        QVERIFY(now && now->isDefault() && now->property("primary").toBool());
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) d->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/update-offer.png");
        now->click();
        QCOMPARE(d->result(), int(jp::Updater::Install));
        d.reset();
        qApp->setStyleSheet(oldSheet);
        QApplication::setWindowIcon(oldIcon);
    }

    void updateVersionOrder()
    {
        QVERIFY(jp::Updater::isNewer("v0.1.6", "0.1.0"));
        QVERIFY(jp::Updater::isNewer("v0.1.6", "v0.1.0-preview5"));
        QVERIFY(jp::Updater::isNewer("v0.1.0-preview5", "v0.1.0-preview4"));
        // An installer runs only from this project's releases, with a plain
        // version tag and the SHA-256 GitHub publishes.
        const QString url = QStringLiteral("https://github.com/JeffOffice/jeffoffice/releases/download/v0.6.11/JeffOffice-Setup.exe");
        const QString digest = QStringLiteral("sha256:63e329c3c1a9aea0f90af5e2519c9315af365cb424a7bd00abb3ad9d201573e7");
        QVERIFY(jp::Updater::trustedInstaller(url, QStringLiteral("v0.6.11"), digest));
        // Not under the old name (GitHub gives the new one), or any other.
        QVERIFY(!jp::Updater::trustedInstaller(QStringLiteral("https://github.com/JeffOffice/jeffpub/releases/download/v0.6.11/JeffOffice-Setup.exe"), QStringLiteral("v0.6.11"), digest));
        QVERIFY(!jp::Updater::trustedInstaller(QStringLiteral("https://github.com/JeffOffice/jeffoffice-evil/releases/download/v0.7.0/x.exe"), QStringLiteral("v0.7.0"), digest));
        QVERIFY(!jp::Updater::trustedInstaller(QStringLiteral("https://github.com/someone/jeffoffice/releases/download/v0.7.0/x.exe"), QStringLiteral("v0.7.0"), digest));
        QVERIFY(!jp::Updater::trustedInstaller(QStringLiteral("https://example.com/JeffPub-Setup.exe"), QStringLiteral("v0.1.17"), digest));
        QVERIFY(!jp::Updater::trustedInstaller(QStringLiteral("http://github.com/JeffOffice/jeffoffice/releases/download/v0.6.11/x.exe"), QStringLiteral("v0.6.11"), digest));
        QVERIFY(!jp::Updater::trustedInstaller(url, QStringLiteral("../../Startup/evil"), digest));   // the tag names the saved file
        QVERIFY(!jp::Updater::trustedInstaller(url, QStringLiteral("v0.1.17"), QString()));          // nothing to check against
        QVERIFY(jp::Updater::isNewer("0.1.10", "0.1.9"));
        QVERIFY(jp::Updater::isNewer("v0.5.0", "0.1.39"));   // the beta after the 0.1 previews
        QVERIFY(!jp::Updater::isNewer("v0.1.6", "0.1.6"));
        QVERIFY(!jp::Updater::isNewer("v0.1.0-preview5", "0.1.6"));
        QVERIFY(jp::Updater::isNewer("v0.1.0", "v0.1.0-preview5"));
        // The newest version is offered whatever order the list comes in (a
        // release published again is listed first, as the newest made).
        auto rel = [](const char *tag, bool draft = false) { return QJsonObject{{"tag_name", QString::fromLatin1(tag)}, {"draft", draft}}; };
        const QJsonArray list{rel("v0.1.9"), rel("v0.5.2"), rel("v0.6.0", true), rel("v0.5.1"), rel("v0.1.39")};
        const auto newer = jp::Updater::newerReleases(list, QStringLiteral("0.5.0"), false);
        QCOMPARE(newer.size(), 2);
        QCOMPARE(newer[0].value("tag_name").toString(), QStringLiteral("v0.5.2"));
        QCOMPARE(newer[1].value("tag_name").toString(), QStringLiteral("v0.5.1"));
        QVERIFY(jp::Updater::newerReleases(list, QStringLiteral("0.5.2"), false).isEmpty());
    }

    // Regular releases reach everyone; GitHub's pre-releases (the test builds
    // in between) only those on the Beta channel; drafts no one.
    void updateChannelChoosesReleases()
    {
        auto rel = [](const char *tag, bool prerelease, bool draft = false) {
            return QJsonObject{{"tag_name", QString::fromLatin1(tag)}, {"prerelease", prerelease}, {"draft", draft}};
        };
        const QJsonArray list{rel("v0.7.2", true), rel("v0.7.1", false), rel("v0.7.4", true, true), rel("v0.7.3", false, true), rel("v0.6.9", false)};
        const auto stable = jp::Updater::newerReleases(list, QStringLiteral("0.7.0"), false);
        QCOMPARE(stable.size(), 1);
        QCOMPARE(stable[0].value("tag_name").toString(), QStringLiteral("v0.7.1"));
        const auto beta = jp::Updater::newerReleases(list, QStringLiteral("0.7.0"), true);
        QCOMPARE(beta.size(), 2);
        QCOMPARE(beta[0].value("tag_name").toString(), QStringLiteral("v0.7.2"));
        QCOMPARE(beta[1].value("tag_name").toString(), QStringLiteral("v0.7.1"));
        // Nothing newer than the newest regular release: Stable has nothing, Beta still has the test build.
        QVERIFY(jp::Updater::newerReleases(list, QStringLiteral("0.7.1"), false).isEmpty());
        QCOMPARE(jp::Updater::newerReleases(list, QStringLiteral("0.7.1"), true).size(), 1);
    }

    // The channel is Stable until Beta is chosen, and the choice is kept.
    void updateChannelSetting()
    {
        QSettings raw(QStringLiteral("JeffOffice"), QStringLiteral("JeffPub"));
        raw.remove(QStringLiteral("updates/channel"));
        QVERIFY(!jp::Updater::betaChannel());
        jp::Updater::setBetaChannel(true);
        QVERIFY(jp::Updater::betaChannel());
        QCOMPARE(jp::Settings::get().value(QStringLiteral("updates/channel")).toString(), QStringLiteral("beta"));
        jp::Updater::setBetaChannel(false);
        QVERIFY(!jp::Updater::betaChannel());
        QCOMPARE(jp::Settings::get().value(QStringLiteral("updates/channel")).toString(), QStringLiteral("stable"));
        raw.remove(QStringLiteral("updates/channel"));
    }

    // File > Options has the Beta choice, off to begin with, and keeps it.
    void optionsBetaUpdates()
    {
        jp::MainWindow w;
        QSettings raw(QStringLiteral("JeffOffice"), QStringLiteral("JeffPub"));
        raw.remove(QStringLiteral("updates/channel"));
        // Opens Options, notes whether the box was checked (-1: no box), and sets it.
        auto choose = [&w](bool on) {
            int was = -1;
            QTimer::singleShot(0, [&was, on] {
                auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!d) return;
                if (auto *beta = d->findChild<QCheckBox *>(QStringLiteral("betaUpdates"))) {
                    was = beta->isChecked();
                    beta->setChecked(on);
                }
                d->accept();   // whatever was found, so a failure can't leave it open
            });
            w.act(QStringLiteral("file.options"))->trigger();
            return was;
        };
        QCOMPARE(choose(true), 0);
        QVERIFY(jp::Updater::betaChannel());
        QCOMPARE(choose(false), 1);
        QVERIFY(!jp::Updater::betaChannel());
        raw.remove(QStringLiteral("updates/channel"));
    }

    void movesJeffPub79Settings()
    {
        // JeffPub 79's settings come over, except what setup already chose.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QSettings old(dir.filePath("old.ini"), QSettings::IniFormat), now(dir.filePath("new.ini"), QSettings::IniFormat);
        old.setValue("telemetry/install", "6f1c2d3e-0000-4000-8000-000000000079");
        old.setValue("telemetry/enabled", true);
        old.setValue("recent", QStringList{"/home/a/flyer.jpub", "/home/a/menu.pub"});
        now.setValue("telemetry/enabled", false);
        jp::copyMissingSettings(old, now);
        QCOMPARE(now.value("telemetry/install").toString(), QStringLiteral("6f1c2d3e-0000-4000-8000-000000000079"));
        QCOMPARE(now.value("recent").toStringList(), (QStringList{"/home/a/flyer.jpub", "/home/a/menu.pub"}));
        QCOMPARE(now.value("telemetry/enabled").toBool(), false);

        // Its folder of templates and building blocks moves to the new name.
        const QString from = dir.filePath("JeffPub/JeffPub 79"), to = dir.filePath("JeffOffice/JeffPub");
        QVERIFY(QDir().mkpath(from + "/Templates") && QDir().mkpath(from + "/BuildingBlocks"));
        for (const QString &f : {QStringLiteral("/Templates/club.jpub"), QStringLiteral("/BuildingBlocks/logo.json")}) {
            QFile file(from + f);
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.write("kept");
        }
        QVERIFY(jp::moveDataFolder(from, to));
        QVERIFY(QFile::exists(to + "/Templates/club.jpub") && QFile::exists(to + "/BuildingBlocks/logo.json"));
        QVERIFY(!QFileInfo::exists(from));
        // Never over a folder the renamed program already has.
        QVERIFY(QDir().mkpath(from));
        QVERIFY(!jp::moveDataFolder(from, to));
        QVERIFY(QFile::exists(to + "/Templates/club.jpub"));
    }

    void settingsSharedAcrossTheSuite()
    {
        // The look, the person, updates, and the statistics choice are the
        // suite's; everything else stays with the app.
        // (The statistics' counts, launches, and day last sent are each app's own: a ping never carries another app's.)
        for (const char *k : {"ui/theme", "ui/language", "user/name", "user/initials", "updates/auto", "updates/channel", "updates/offeredBy", "telemetry/enabled",
                              "telemetry/install"})
            QVERIFY2(jp::suiteWide(QString::fromLatin1(k)), k);
        for (const char *k : {"units", "recent", "edit/nudge", "ui/windowGeometry", "telemetry/counts", "telemetry/launches", "telemetry/lastSent",
                              "telemetry/lastVersion", "telemetry/collectedFor", "migration/suite", "migration/suiteInstall"})
            QVERIFY2(!jp::suiteWide(QString::fromLatin1(k)), k);

        // A choice made in one app is kept in the suite's store, where every app reads it.
        QSettings suite(QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice")), app(QStringLiteral("JeffOffice"), QStringLiteral("JeffPub"));
        const QVariant theme = jp::Settings::get().value("ui/theme"), nudge = jp::Settings::get().value("edit/nudge");
        jp::Settings::get().setValue("ui/theme", 2);
        jp::Settings::get().setValue("edit/nudge", 4.5);
        QCOMPARE(suite.value("ui/theme").toInt(), 2);
        QVERIFY(!app.contains("ui/theme"));
        QCOMPARE(app.value("edit/nudge").toDouble(), 4.5);
        QVERIFY(!suite.contains("edit/nudge"));
        suite.setValue("ui/theme", 1);   // as another app would
        QCOMPARE(jp::Settings::get().value("ui/theme").toInt(), 1);
        jp::Settings::get().setValue("ui/theme", theme);
        jp::Settings::get().setValue("edit/nudge", nudge);

        // What an app kept itself before moves over once, except what setup already wrote.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QSettings mine(dir.filePath("app.ini"), QSettings::IniFormat), all(dir.filePath("suite.ini"), QSettings::IniFormat);
        mine.setValue("ui/theme", 2);
        mine.setValue("user/name", "Pat Doe");
        mine.setValue("updates/skip", "v0.6.9");
        mine.setValue("telemetry/enabled", true);
        mine.setValue("recent", QStringList{"/home/a/flyer.jpub"});
        all.setValue("telemetry/enabled", false);
        jp::moveToSuite(mine, all);
        QCOMPARE(all.value("ui/theme").toInt(), 2);
        QCOMPARE(all.value("user/name").toString(), QStringLiteral("Pat Doe"));
        QCOMPARE(all.value("updates/skip").toString(), QStringLiteral("v0.6.9"));
        QCOMPARE(all.value("telemetry/enabled").toBool(), false);
        QVERIFY(!all.contains("recent"));
        QCOMPARE(mine.value("ui/theme").toInt(), 2);   // kept, for an older version
        mine.setValue("ui/theme", 0);
        jp::moveToSuite(mine, all);   // only once
        QCOMPARE(all.value("ui/theme").toInt(), 2);
    }
};

JP_TEST_CLASS(PubUpdateTests)
#include "tst_pub_update.moc"
