#include "io/presentationfile.h"

#include "io/odpexport.h"
#include "io/odpimport.h"
#include "io/pptimport.h"
#include "io/pptximport.h"
#include "office/package.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>

namespace jp::deck {

namespace {

// The compound file signature, the first thing in an old binary presentation.
bool looksLikeCompoundFile(const QByteArray &head) { return head.startsWith(QByteArray::fromHex("D0CF11E0A1B11AE1")); }

} // namespace

FileFormat formatOfFileName(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == QLatin1String("odp") || suffix == QLatin1String("otp") ? FileFormat::Odp : FileFormat::Pptx;
}

PresentationKind presentationKindOf(const QString &path)
{
    if (QFileInfo(path).suffix().compare(QLatin1String("otp"), Qt::CaseInsensitive) == 0) return PresentationKind::Template;
    return kindOfFileName(path);
}

bool isWrittenEnding(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == QLatin1String("pptx") || suffix == QLatin1String("potx") || suffix == QLatin1String("ppsx") || suffix == QLatin1String("odp") || suffix == QLatin1String("otp");
}

bool saveAnyPresentationFile(const Document &doc, const QString &path, QString *error, QStringList *skipped, bool embedFonts)
{
    return formatOfFileName(path) == FileFormat::Odp ? saveOdpFile(doc, path, error, skipped) : savePresentationFile(doc, path, error, skipped, embedFonts);
}

QStringList leftOutNotes(int pictures, int parts)
{
    QStringList notes;
    if (pictures == 1) notes << QCoreApplication::translate("PresentationFile", "A picture was left out: it is too large or too complex to show, or it would draw from outside the file.");
    else if (pictures > 1) notes << QCoreApplication::translate("PresentationFile", "%1 pictures were left out: they are too large or too complex to show, or they would draw from outside the file.").arg(pictures);
    if (parts == 1) notes << QCoreApplication::translate("PresentationFile", "One part of the file has a name or a link that can't be used, and was left out.");
    else if (parts > 1) notes << QCoreApplication::translate("PresentationFile", "%1 parts of the file have names or links that can't be used, and were left out.").arg(parts);
    return notes;
}

std::unique_ptr<Document> importAnyPresentationFile(const QString &path, QString *error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error) *error = QCoreApplication::translate("PresentationFile", "The file can't be opened: %1").arg(f.errorString());
        return nullptr;
    }
    // (A file larger than the apps open is refused before it is read: every kind is held whole at once.)
    if (f.size() > office::kMaxOpenFileBytes) {
        if (error) *error = QCoreApplication::translate("jp::office::doc", "The file is larger than this program can open.");
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
