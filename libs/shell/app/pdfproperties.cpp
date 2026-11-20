#include "app/pdfproperties.h"

#include "app/privacy.h"
#include "core/appinfo.h"

namespace jp {

PdfInfo pdfDocumentInfo(const office::PackageProps &props, const QString &fallbackTitle, bool namesOut, const QDateTime &now)
{
    PdfInfo info;
    info.title = props.title.trimmed().isEmpty() ? fallbackTitle : props.title;
    info.author = namesOut || privacy::removeOnSave() ? QString() : props.creator;
    info.subject = props.subject;
    info.keywords = props.keywords;
    info.program = appInfo().name;
    info.created = now;
    return info;
}

} // namespace jp
