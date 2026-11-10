#include "core/pdfbookmarks.h"

#include "core/gotoops.h"
#include "core/writedoc.h"

namespace jp::write {

QVector<PdfBookmark> headingBookmarks(WriteDoc &doc, const QVector<int> &pages)
{
    const WriteDoc::PrintLayout printPages(doc);
    QVector<PdfBookmark> out;
    for (const OutlineHeading &h : outlineHeadings(doc)) {
        const int page = doc.pager().pageOfPosition(h.position);
        const int at = int(pages.indexOf(page));
        if (at < 0) continue;
        out.push_back({h.label, h.level, at});
    }
    return out;
}

} // namespace jp::write
