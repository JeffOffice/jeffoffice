#pragma once
// What printing is told by the person, made ready: the pages a range names, and how the paper is described.

#include <QSizeF>
#include <QString>
#include <QVector>
#include <optional>

namespace jp::write {

// The marks over the pages that review puts beside the text (a bar by each line a change touches, a balloon for each comment and each deletion, a caret where deleted words were left out):
// what the page view shows, and so what a print with its markup shows.
struct MarkupMarks {
    bool changeBars = false, formatBars = false, comments = false, deletions = false, carets = false;
    bool any() const { return changeBars || formatBars || comments || deletions || carets; }
};
// What a job prints besides the text: the markup (tracked changes as the screen shows them, and the marks above) or the text as if the changes were accepted for display with no comments;
// hidden text; and the page's background color (unset: as before, a PDF has it and a printer is not sent it).
struct PrintLook {
    bool markup = true;
    bool hidden = false;
    std::optional<bool> background;
    MarkupMarks marks;   // what is drawn over the pages (empty without the markup)
};
// The names of the choices in the settings, as they are kept ("print/markup" and the others).
QString printMarkupKey();
QString printHiddenKey();
QString printBackgroundKey();
// How tracked changes are shown on the paper: as the screen shows them (`screenView`) with the markup, else not at all (the text as it stands, deletions left out).
int printRevisionView(bool markup, int screenView);

// The pages (from 0) that text such as "1-3, 5" names, once each, in the order given. A part that names no page of the document is passed by: backwards
// ranges are read forwards, "8-" goes to the last page and "-2" from the first, and a page past the last is not one. Empty when none is named.
QVector<int> parsePageRange(const QString &text, int pageCount);
// The same, where the document's sections are known (`sectionOfPage` holds each page's section, one entry for each page): "p2s3" is the second page of the third section, and "s3" is the
// whole third section, each of them also an end of a range ("p1s2-p1s3"). Sections count in the order they have pages; the page of a section counts its pages, not the numbers they show.
QVector<int> parsePageRange(const QString &text, int pageCount, const QVector<int> &sectionOfPage);

// The paper a page is set on, in words: "Letter, portrait", or "A4, landscape"; a size with no name is given in inches or centimeters (as the region measures).
QString paperDescription(const QSizeF &points);

} // namespace jp::write
