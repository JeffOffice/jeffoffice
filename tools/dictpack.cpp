// Build-time tool: writes the compressed form of a dictionary file for the install (text/dictpack.h). The repository keeps the plain files.
// Usage: jo_dictpack [--thesaurus] <plain file> <packed file>      Exit status 0 when written (and read back equal); what failed is on stderr.
#include "text/dictpack.h"

#include <QCoreApplication>
#include <QFile>
#include <QSaveFile>
#include <cstdio>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    const QStringList args = app.arguments().mid(1);
    const bool thesaurus = args.value(0) == QLatin1String("--thesaurus");
    const QStringList files = thesaurus ? args.mid(1) : args;
    if (files.size() != 2) {
        std::fprintf(stderr, "usage: jo_dictpack [--thesaurus] <plain file> <packed file>\n");
        return 2;
    }
    QFile in(files[0]);
    if (!in.open(QIODevice::ReadOnly)) {
        std::fprintf(stderr, "jo_dictpack: cannot read %s\n", qPrintable(files[0]));
        return 1;
    }
    const QByteArray plain = in.readAll();
    const QByteArray packed = thesaurus ? jp::dictpack::packThesaurus(plain) : jp::dictpack::pack(plain);
    QByteArray back;
    bool ok = !packed.isEmpty();
    if (ok && !thesaurus) ok = jp::dictpack::unpack(packed, back) && back == plain;
    if (ok && thesaurus) {
        jp::dictpack::Thesaurus t;
        ok = t.openMemory(packed) && !t.entry(QByteArray("zzzz-not-a-word")).size() && t.blockCount() > 0;
    }
    if (!ok) {
        std::fprintf(stderr, "jo_dictpack: could not pack %s\n", qPrintable(files[0]));
        return 1;
    }
    QSaveFile out(files[1]);
    if (!out.open(QIODevice::WriteOnly) || out.write(packed) != packed.size() || !out.commit()) {
        std::fprintf(stderr, "jo_dictpack: cannot write %s\n", qPrintable(files[1]));
        return 1;
    }
    return 0;
}
