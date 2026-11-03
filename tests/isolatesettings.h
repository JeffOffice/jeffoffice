#pragma once
// A test program never touches the person's real settings (the Recent list, the options): from the moment the program starts, the settings go to a folder of its own that is taken away
// again when it ends. Including this header is enough (the test programs that have a main() of their own include it; testrunner.cpp does too). `realSettingsUnchanged()` says whether what
// is in the person's real settings folder is what it was when the program started, for a test to check.

#include <QByteArray>
#include <QCryptographicHash>
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

// A checksum of every settings file there (never shown).
inline QByteArray realSettingsChecksum()
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    for (const QFileInfo &fi : QDir(realSettingsFolder()).entryInfoList({QStringLiteral("*.conf")}, QDir::Files, QDir::Name)) {
        hash.addData(fi.fileName().toUtf8());
        QFile f(fi.absoluteFilePath());
        if (f.open(QIODevice::ReadOnly)) hash.addData(f.readAll());
    }
    return hash.result();
}

inline const QByteArray &realSettingsChecksumAtStart()
{
    static const QByteArray before = realSettingsChecksum();   // (first called, before the redirect, as the program starts)
    return before;
}

inline bool realSettingsUnchanged() { return realSettingsChecksum() == realSettingsChecksumAtStart(); }

// Where this run's settings are.
inline const QString &isolatedSettingsFolder()
{
    static QTemporaryDir dir;
    static const bool redirected = [] {
        realSettingsChecksumAtStart();
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

// True when the settings an app reads and writes are in a temporary folder (this run's own, or one a test class made for itself), not the person's.
inline bool settingsAreIsolated()
{
    const QString file = QSettings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("JeffOffice"), QStringLiteral("JeffWrite")).fileName();
    return file.startsWith(QDir::tempPath() + QLatin1Char('/')) && !file.startsWith(realSettingsFolder() + QLatin1Char('/'));
}

// A test that saves to the program's own data folder (My Templates, the dictionary) puts it in Qt's test folder instead, and puts it back however the test ends.
struct TestModeGuard {
    TestModeGuard() { QStandardPaths::setTestModeEnabled(true); }
    ~TestModeGuard() { QStandardPaths::setTestModeEnabled(false); }
    TestModeGuard(const TestModeGuard &) = delete;
    TestModeGuard &operator=(const TestModeGuard &) = delete;
};

} // namespace jptest
