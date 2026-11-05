#pragma once
// Header and footer work beyond typing: the designs a header or footer can start from, and the document information (author, title, file name, and path) put in as fields.

#include "core/writedoc.h"

namespace jp::write {

struct HeaderDesign {
    QString id, label;   // the label is translated
};

// The designs, in the order the gallery shows them: blank, a page number at the right, in the middle, or at the left, the title with the page number, the date with the page number,
// and three columns (date, title, page number).
QVector<HeaderDesign> headerFooterDesigns();

// Gives the header (footer) of the section the design: what it held is replaced, as one step that undoes. A blank one is made where the section has none. False for a design that is not
// one of the list, or a header that can't be changed.
bool applyHeaderFooterDesign(WriteDoc &doc, int section, bool footer, int slot, const QString &id);

// Puts a field of the document's information at the cursor: "author", "title", "fileName", or "filePath". False for any other kind.
bool insertDocumentInfo(WriteDoc &doc, QTextCursor &c, const QString &kind);

} // namespace jp::write
