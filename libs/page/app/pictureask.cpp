#include "app/pictureask.h"

#include "app/passworddialogs.h"
#include "core/document.h"
#include "io/crypto.h"
#include "render/pdfpage.h"

#include <QFileInfo>
#include <QScopeGuard>

namespace jp {

bool readPictureFileAsking(QWidget *parent, const QString &path, int pdfPage, QByteArray *bytes, QString *format, QSize *pixels, QByteArray *fileHash)
{
    if (bytes->isEmpty()) {
        *bytes = readPictureBytes(path);
        if (bytes->isEmpty()) return false;
    }
    QString password;
    const auto wipe = qScopeGuard([&password] { crypto::wipe(password); });
    if (QFileInfo(path).suffix().compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0 || PdfDocument::looksLikePdf(*bytes)) {
        if (unlockPdf(parent, QFileInfo(path).fileName(), *bytes, &password) != PdfUnlock::Opened) return false;
    }
    return readPictureFile(path, pdfPage, bytes, format, pixels, fileHash, password);
}

} // namespace jp
