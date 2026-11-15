// The picture behind a sheet's cells: the sheet's `picture` element names an image part by a relationship. Read here, bounded, into the page setup; the writer is in xlsxsave.cpp.

#include "sheet/sheetreader.h"

#include <algorithm>

#include <QMutexLocker>
#include <QSet>

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

qint64 Workbook::pictureBytesInFile(int index) const
{
    if (index < 0 || index >= m_info.sheets.size()) return 0;
    const SheetEntry &entry = m_info.sheets[index];
    if (entry.deleted || entry.isNew || entry.part.isEmpty() || entry.kind != SheetKind::Worksheet) return 0;
    QSet<QString> seen;
    qint64 total = 0;
    auto addImage = [&](const QString &target) {
        if (seen.contains(target.toLower())) return;
        seen.insert(target.toLower());
        total += std::min(std::max<qint64>(0, m_package.partSize(target)), kMaxHeaderPictureBytes);
    };
    for (const Relationship &r : m_package.relationshipsOfType(entry.part, QStringLiteral("/image")))
        if (!r.external) addImage(r.target);
    for (const Relationship &r : m_package.relationshipsOfType(entry.part, QStringLiteral("/vmlDrawing")))
        if (!r.external)
            for (const Relationship &image : m_package.relationshipsOfType(r.target, QStringLiteral("/image")))
                if (!image.external) addImage(image.target);
    return total;
}

qint64 workbookPictureBytes(const Workbook &book, const std::function<const PageSetup *(int sheet)> &pageOf, int skipSheet)
{
    qint64 total = 0;
    for (int i = 0; i < book.info().sheets.size(); ++i) {
        if (i == skipSheet) continue;
        const PageSetup *page = pageOf ? pageOf(i) : nullptr;
        total += page ? pictureBytesOf(*page) : book.pictureBytesInFile(i);
    }
    return total;
}

} // namespace jp::office::sheet
