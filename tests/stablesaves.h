#pragma once
// For the tests that a saved file is the same bytes each time it is saved. Qt picks the seed of its hashes anew for each process, so a save that follows the order of a hash
// (the attributes of a DOM element, a QHash or QSet that is walked) is the same twice in one process and different in the next: it takes separate processes to see it.
//
// The program runs one of its own test functions in a child process, `runs` times; the function sees where to write in childOutput() and saves what it is given there.
//   void saveForTheChecks() { QString out; if (!stable::childOutput(&out)) return; ...write the file named out... }
//   void savesAreTheSameInEachProcess() { stable::expectSameSaves(this, "saveForTheChecks"); }
// (The first function does nothing in the program's own run: it has nowhere to write.)

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMap>
#include <QObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QtTest>

#include "io/zip.h"

namespace stable {

inline bool childOutput(QString *path)
{
    *path = qEnvironmentVariable("JP_STABLE_SAVE_OUT");
    return !path->isEmpty();
}

// What may change between two saves: the time of saving, and the ids a format wants new each time (GUIDs).
inline QByteArray withoutTimesAndIds(QByteArray bytes)
{
    static const QRegularExpression guid(QStringLiteral("[{]?[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}[}]?"));
    static const QRegularExpression stamp(QStringLiteral("[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}(?:[.][0-9]+)?(?:Z|[+-][0-9]{2}:[0-9]{2})?"));
    QString text = QString::fromLatin1(bytes);   // (Latin-1 keeps every byte as it is)
    text.replace(guid, QStringLiteral("G"));
    text.replace(stamp, QStringLiteral("T"));
    return text.toLatin1();
}

// The parts of a file that is a ZIP, each without the times and ids, or the whole file as one part.
inline QMap<QString, QByteArray> partsOf(const QByteArray &file)
{
    QMap<QString, QByteArray> parts, out;
    if (jp::readZip(file, parts))
        for (auto it = parts.cbegin(); it != parts.cend(); ++it) out.insert(it.key(), withoutTimesAndIds(it.value()));
    else
        out.insert(QString(), withoutTimesAndIds(file));
    return out;
}

// The name of the first part that is not the same in all of the files, or empty if they are.
inline QString firstDifference(const QList<QByteArray> &files)
{
    const QMap<QString, QByteArray> first = partsOf(files.value(0));
    for (int i = 1; i < files.size(); ++i) {
        const QMap<QString, QByteArray> other = partsOf(files[i]);
        if (other.keys() != first.keys()) return QStringLiteral("(the list of parts)");
        for (auto it = first.cbegin(); it != first.cend(); ++it)
            if (other.value(it.key()) != it.value()) return it.key().isEmpty() ? QStringLiteral("(the file)") : it.key();
    }
    return QString();
}

// Runs the test function `slot` of this program in `runs` processes of their own, each saving to a file, and says which part differs ("" if none; a message starting "child" if one did
// not run).
inline QString differenceBetweenProcesses(const QString &slot, int runs = 4)
{
    QTemporaryDir dir;
    if (!dir.isValid()) return QStringLiteral("child: no temporary folder");
    QList<QByteArray> files;
    for (int i = 0; i < runs; ++i) {
        const QString out = dir.filePath(QStringLiteral("run%1/saved").arg(i));   // (the same name in each: a saved file may carry its own)
        QDir().mkpath(QFileInfo(out).path());
        QProcess p;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("JP_STABLE_SAVE_OUT"), out);
        env.insert(QStringLiteral("JP_NO_TELEMETRY"), QStringLiteral("1"));
        if (!env.contains(QStringLiteral("QT_QPA_PLATFORM"))) env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
        p.setProcessEnvironment(env);
        p.setProcessChannelMode(QProcess::MergedChannels);
        p.start(QCoreApplication::applicationFilePath(), {slot});
        if (!p.waitForFinished(300000)) {
            p.kill();
            p.waitForFinished();
            return QStringLiteral("child: timed out");
        }
        QFile f(out);
        if (p.exitCode() != 0 || !f.open(QIODevice::ReadOnly)) return QStringLiteral("child: did not save (%1)").arg(QString::fromUtf8(p.readAll().right(400)));
        files << f.readAll();
    }
    return firstDifference(files);
}

} // namespace stable

// In a test function: the saves of `slot` in separate processes are the same bytes.
#define JP_VERIFY_SAME_SAVES(slot)                                                                                                       \
    do {                                                                                                                                 \
        const QString difference_ = stable::differenceBetweenProcesses(QStringLiteral(slot));                                          \
        QVERIFY2(difference_.isEmpty(), qPrintable(QStringLiteral("saved differently in separate processes: ") + difference_));       \
    } while (false)
