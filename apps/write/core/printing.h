#pragma once
// What printing is told by the person, made ready: the pages a range names, the order the sheets come out for the copies, and how the paper is described.

#include <QSizeF>
#include <QString>
#include <QVector>

namespace jp::write {

// The pages (from 0) that text such as "1-3, 5" names, once each, in the order given. A part that names no page of the document is passed by: backwards
// ranges are read forwards, "8-" goes to the last page and "-2" from the first, and a page past the last is not one. Empty when none is named.
QVector<int> parsePageRange(const QString &text, int pageCount);

// The pages in the order the sheets come out for `copies` of them: collated, the whole set again for each copy (1 2 3 1 2 3), else each page's copies
// together (1 1 2 2 3 3). Never fewer than one copy.
QVector<int> sheetOrder(const QVector<int> &pages, int copies, bool collate);

// The paper a page is set on, in words: "Letter, portrait", or "A4, landscape"; a size with no name is given in inches or centimeters (as the region measures).
QString paperDescription(const QSizeF &points);

} // namespace jp::write
