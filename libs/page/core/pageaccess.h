#pragma once
// What on the pages of a publication or a presentation is hard for some readers (or a screen reader) to use: a picture or a shape with no description, and, where the app asks,
// a page with no title, a table with no header row, a video with no captions; text that is hard to read against its background (the contrast of the two colors), and objects a screen reader reads in another order than their places on
// the page suggest. A list, each with the page and the object, so that the window can go there.

#include "core/items.h"

#include <QColor>
#include <QString>
#include <QVector>
#include <functional>

namespace jp {

class Document;

struct PageAccessIssue {
    enum Kind { MissingAltText, NoPageTitle, LowContrast, ReadingOrder, NoTableHeader, NoCaptions };
    enum Level { Error, Warning, Tip };
    Kind kind = MissingAltText;
    Level level = Warning;
    int page = 0;
    QString itemId;   // empty for a whole page
    QString message;
};

constexpr int kMaxPageAccessIssues = 500;

// The contrast of two colors, from 1 (the same) to 21 (black on white): the ratio of their relative luminances, each with 0.05 added.
double contrastRatio(const QColor &a, const QColor &b);
// The contrast text needs: 3 for large text (18 points or more, or 14 points or more in bold), else 4.5.
double neededContrast(double points, bool bold);
// Which of a page's objects are read in another place than their positions give: the ones left over when the longest run that is already in order (top to bottom, then left to right,
// objects whose vertical spans overlap by half being a row) is taken out. Their numbers in `items`, in order; hidden objects, decoration, lines, and a picture covering most of the
// page (a backdrop) are not read, so are not counted. `pageSize` is the page's, for the backdrop (empty: no backdrop).
QVector<int> outOfReadingOrder(const ItemList &items, const QSizeF &pageSize = QSizeF());

// The problems found, page by page (at most `most`). A picture with no alt text is an error, and so is a shape with no words in it (an object marked as decoration is left alone).
// A table whose first row is not marked as a header row is a warning; a video with no caption track is a tip. (A table or a video marked as decoration is left alone.)
// `titleOf` (may be empty) gives the title of a page; with it, a page with an empty title is a warning. `slides` says "slide" where the messages say "page".
QVector<PageAccessIssue> checkPageAccessibility(const Document &doc, bool slides = false, const std::function<QString(int)> &titleOf = {}, int most = kMaxPageAccessIssues);

} // namespace jp
