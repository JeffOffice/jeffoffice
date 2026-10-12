#include "io/presentationfile.h"

#include "io/pptimport.h"
#include "io/pptximport.h"

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
    if (!looksLikeCompoundFile(f.read(8))) {
        f.close();
        return importPresentationFile(path, error);
    }
    f.seek(0);
    return importBinaryPresentation(f.readAll(), error);
}

} // namespace jp::deck
