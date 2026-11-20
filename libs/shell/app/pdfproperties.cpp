#include "app/pdfproperties.h"

#include "app/privacy.h"
#include "core/appinfo.h"

namespace jp {

PdfInfo pdfDocumentInfo(const office::PackageProps &props, const QString &fallbackTitle, bool namesOut, const QDateTime &now)
{
    PdfInfo info;
    // (a file's name can be a person's: with personal information out, a document with no title of its own has none in the PDF either)
    const bool stayOut = namesOut || privacy::removeOnSave();
    info.title = props.title.trimmed().isEmpty() ? (stayOut ? QString() : fallbackTitle) : props.title;
    info.author = stayOut ? QString() : props.creator;
    info.subject = props.subject;
    info.keywords = props.keywords;
    info.program = appInfo().name;
    info.created = now;
    return info;
}

} // namespace jp
