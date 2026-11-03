#pragma once
// A test program never touches the person's real settings (the Recent list, the options): from the moment the program starts, the settings go to a folder of its own that is taken away
// again when it ends. Including this header is enough (the test programs that have a main() of their own include it; testrunner.cpp does too). `settingsStayInTheRunsFolder()` proves it at the
// end of a run: a setting with a name no other program has is written the way the apps write theirs, and is found in the run's own folder and not in the person's. (A checksum of the
// person's files would also change when another program, the person's own JeffWrite open beside the tests among them, saved its settings meanwhile.)

#include <QByteArray>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QTemporaryDir>

namespace jptest {

// The folder the person's settings files are in (the same for every app of the suite).
inline QString realSettingsFolder()
{
    static const QString folder = QFileInfo(QSettings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice")).fileName()).absolutePath();
    return folder;
}

// Where this run's settings are.
inline const QString &isolatedSettingsFolder()
{
    static QTemporaryDir dir;
    static const bool redirected = [] {
        realSettingsFolder();   // (found before the settings are moved: after, it would be the run's own)
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, dir.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, dir.path());
        return true;
    }();
    Q_UNUSED(redirected)
    static const QString path = dir.path();
    return path;
}

struct IsolateSettings {
    IsolateSettings() { isolatedSettingsFolder(); }
};
inline const IsolateSettings isolateSettingsOnStart;

// Whether a file of the folder holds the text.
inline bool folderHolds(const QString &folder, const QByteArray &text)
{
    for (const QFileInfo &fi : QDir(folder).entryInfoList({QStringLiteral("*.conf")}, QDir::Files)) {
        QFile f(fi.absoluteFilePath());
        if (f.open(QIODevice::ReadOnly) && f.readAll().contains(text)) return true;
    }
    return false;
}

// Writes a setting under a name of this run's own, through the store the apps use, and says it was kept in the settings folder the run made and nowhere in the person's.
inline bool settingsStayInTheRunsFolder()
{
    const QString key = QStringLiteral("jptest/probe-%1-%2").arg(QCoreApplication::applicationPid()).arg(QDateTime::currentMSecsSinceEpoch());
    {
        QSettings app(QStringLiteral("JeffOffice"), QStringLiteral("JeffWrite"));   // (as the apps make theirs)
        app.setValue(key, 1);
        app.sync();
    }
    const QString written = QSettings(QStringLiteral("JeffOffice"), QStringLiteral("JeffWrite")).fileName();
    const QByteArray name = key.mid(QStringLiteral("jptest/").size()).toUtf8();
    const bool kept = QFileInfo(written).absolutePath().startsWith(QDir::tempPath() + QLatin1Char('/')) && folderHolds(QFileInfo(written).absolutePath(), name);
    const bool leaked = folderHolds(realSettingsFolder(), name);
    {
        QSettings app(QStringLiteral("JeffOffice"), QStringLiteral("JeffWrite"));   // (as the apps make theirs)
        app.remove(key);
    }
    return kept && !leaked;
}

// A test that saves to the program's own data folder (My Templates, the dictionary) puts it in Qt's test folder instead, and puts it back however the test ends.
struct TestModeGuard {
    TestModeGuard() { QStandardPaths::setTestModeEnabled(true); }
    ~TestModeGuard() { QStandardPaths::setTestModeEnabled(false); }
    TestModeGuard(const TestModeGuard &) = delete;
    TestModeGuard &operator=(const TestModeGuard &) = delete;
};

} // namespace jptest
