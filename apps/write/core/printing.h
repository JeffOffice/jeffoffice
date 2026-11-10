#pragma once
// What printing is told by the person, made ready: the pages a range names, and how the paper is described.

#include <QSizeF>
#include <QString>
#include <QVector>

namespace jp::write {

// The pages (from 0) that text such as "1-3, 5" names, once each, in the order given. A part that names no page of the document is passed by: backwards
// ranges are read forwards, "8-" goes to the last page and "-2" from the first, and a page past the last is not one. Empty when none is named.
QVector<int> parsePageRange(const QString &text, int pageCount);
// The same, where the document's sections are known (`sectionOfPage` holds each page's section, one entry for each page): "p2s3" is the second page of the third section, and "s3" is the
// whole third section, each of them also an end of a range ("p1s2-p1s3"). Sections count in the order they have pages; the page of a section counts its pages, not the numbers they show.
QVector<int> parsePageRange(const QString &text, int pageCount, const QVector<int> &sectionOfPage);

// The paper a page is set on, in words: "Letter, portrait", or "A4, landscape"; a size with no name is given in inches or centimeters (as the region measures).
QString paperDescription(const QSizeF &points);

} // namespace jp::write
