// The picture behind a sheet's cells: the sheet's `picture` element names an image part by a relationship. Read here, bounded, into the page setup; the writer is in xlsxsave.cpp.

#include "sheet/sheetreader.h"

#include <QMutexLocker>

namespace jp::office::sheet {

void Workbook::readBackground(const QString &sheetPart, PageSetup *page) const
{
    SheetBackground &bg = page->background;
    const QString relId = bg.rel;
    bg = SheetBackground();
    bg.rel = relId;
    if (relId.isEmpty() || sheetPart.isEmpty()) return;
    const Relationship rel = m_package.relationship(sheetPart, relId);
    if (rel.id.isEmpty() || rel.external || !rel.type.endsWith(QLatin1String("/image"))) return;
    const int dot = rel.target.lastIndexOf(QLatin1Char('.'));
    const QString extension = dot > rel.target.lastIndexOf(QLatin1Char('/')) ? rel.target.mid(dot + 1).toLower() : QString();
    if (!backgroundTypeAllowed(extension)) return;
    // (the image is read once for the workbook, as the pictures of the headers are, and not past what the workbook may hold of them)
    QMutexLocker lock(&m_headerImages->mutex);
    QByteArray image;
    const auto have = m_headerImages->images.constFind(rel.target);
    if (have != m_headerImages->images.constEnd()) {
        image = *have;
    } else {
        const qint64 bytes = m_package.partSize(rel.target);
        if (bytes <= 0 || bytes > kMaxHeaderPictureBytes || m_headerImages->bytes + bytes > kMaxHeaderPictureBytesInBook) return;
        image = m_package.part(rel.target);
        if (image.isEmpty()) return;
        m_headerImages->bytes += image.size();
        m_headerImages->images.insert(rel.target, image);
    }
    if (image.size() > kMaxHeaderPictureBytes || !backgroundBytesMatch(image, extension)) return;
    bg.image = image;
    bg.extension = extension == QLatin1String("jpg") ? QStringLiteral("jpeg") : extension;
    bg.part = rel.target;
}

} // namespace jp::office::sheet
