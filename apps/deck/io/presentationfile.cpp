#include "io/presentationfile.h"

#include "io/odpimport.h"
#include "io/pptimport.h"
#include "io/pptximport.h"
#include "office/package.h"

#include <QCoreApplication>
#include <QFile>

namespace jp::deck {

namespace {

// The compound file signature, the first thing in an old binary presentation.
bool looksLikeCompoundFile(const QByteArray &head) { return head.startsWith(QByteArray::fromHex("D0CF11E0A1B11AE1")); }

} // namespace

std::unique_ptr<Document> importAnyPresentationFile(const QString &path, QString *error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error) *error = QCoreApplication::translate("PresentationFile", "The file can't be opened: %1").arg(f.errorString());
        return nullptr;
    }
    const QByteArray head = f.read(128);
    if (looksLikeOpenDocument(head)) {
        f.seek(0);
        QStringList skipped;
        QString why;
        auto d = importOdp(f.readAll(), &why, &skipped);
        if (error) *error = d ? skipped.join(QLatin1Char('\n')) : why;
        return d;
    }
    if (!looksLikeCompoundFile(head)) {
        f.close();
        if (auto d = importPresentationFile(path, error)) return d;
        // A package of the open document kind whose type is not the first entry (as a writer that doesn't follow the standard to the letter makes it).
        QFile again(path);
        if (!again.open(QIODevice::ReadOnly)) return nullptr;
        const QByteArray bytes = again.readAll();
        office::Package pkg;
        if (!pkg.readOpenDocument(bytes) || !pkg.hasPart(QStringLiteral("/mimetype")) || !pkg.hasPart(QStringLiteral("/content.xml"))) return nullptr;
        QStringList skipped;
        QString why;
        auto d = importOdp(bytes, &why, &skipped);
        if (error) *error = d ? skipped.join(QLatin1Char('\n')) : why;
        return d;
    }
    f.seek(0);
    return importBinaryPresentation(f.readAll(), error);
}

} // namespace jp::deck
