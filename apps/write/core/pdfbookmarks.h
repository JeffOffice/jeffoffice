#pragma once
// The bookmarks of a word-processor PDF, from its headings (the outline itself is made by io/pdfbookmarks.h, which every app shares).

#include "io/pdfbookmarks.h"

namespace jp::write {

class WriteDoc;

using jp::PdfBookmark;
using jp::addPdfBookmarks;
using jp::kMostPdfBookmarks;
using jp::pdfPropertiesXmp;

// The headings of the document (the paragraphs with an outline level) as bookmarks. `pages` are the pages the PDF holds, in order, by their index in the document; a heading on a
// page the PDF leaves out is left out.
QVector<PdfBookmark> headingBookmarks(WriteDoc &doc, const QVector<int> &pages);

} // namespace jp::write
