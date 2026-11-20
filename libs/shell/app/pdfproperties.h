#pragma once
// The document properties a PDF carries (File > Properties of a PDF reader), the same in every app: the title, author, subject, and keywords the document already holds, the date the
// file was made, and this program named as its creator and producer. Nothing is added that the document does not hold.

#include "io/pdfbookmarks.h"
#include "office/packageprops.h"

namespace jp {

// `fallbackTitle` stands for a title the document has not got (the name of the file). The author is left out when `namesOut` is set (the document's personal information was removed) or
// Options > Save > Remove personal information is on. `now` is the date of the file (a test pins it; the export passes the time it is made).
PdfInfo pdfDocumentInfo(const office::PackageProps &props, const QString &fallbackTitle, bool namesOut, const QDateTime &now = QDateTime::currentDateTimeUtc());

} // namespace jp
