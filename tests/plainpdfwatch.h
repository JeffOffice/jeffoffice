// Proof that a PDF with a password is never on the disk without it. A watch looks, each time Qt makes a file object, at the files in some folders, and writes down the name of any
// that is a PDF without an /Encrypt dictionary. A control run (an export with no password) shows the watch sees: it must write names down.
#pragma once

#include <private/qabstractfileengine_p.h>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QTemporaryDir>

struct PlainPdfWatch : QAbstractFileEngineHandler {
    QStringList dirs;               // folders looked in (not below)
    mutable bool busy = false;      // (looking makes file objects too)
    mutable QStringList plain;      // folder/name of each PDF seen without /Encrypt
    std::unique_ptr<QAbstractFileEngine> create(const QString &) const override
    {
        if (busy || dirs.isEmpty()) return nullptr;
        busy = true;
        for (const QString &dir : dirs)
            for (const QString &name : QDir(dir).entryList(QDir::Files)) {
                QFile in(QDir(dir).filePath(name));
                if (!in.open(QIODevice::ReadOnly)) continue;
                const QByteArray bytes = in.readAll();
                if (bytes.startsWith("%PDF-") && !bytes.contains("/Encrypt")) plain << QDir(dir).filePath(name);
            }
        busy = false;
        return nullptr;
    }
};

// The folder for temporary files, a folder of the test's own while this lives.
struct ScratchTemp {
    QTemporaryDir dir;
    struct Var {
        QByteArray name, old;
        bool had;
        Var(const char *n, const QString &v) : name(n), old(qgetenv(n)), had(qEnvironmentVariableIsSet(n)) { qputenv(n, v.toLocal8Bit()); }
        ~Var() { if (had) qputenv(name, old); else qunsetenv(name); }
    } a, b, c;
    ScratchTemp() : a("TMPDIR", dir.path()), b("TEMP", dir.path()), c("TMP", dir.path()) {}
    QString path() const { return dir.path(); }
};
