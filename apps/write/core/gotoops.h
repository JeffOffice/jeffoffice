#pragma once
// Go To (Ctrl+G): where in the document a page, section, line, bookmark, footnote, or heading is. Nothing here changes the document; the window puts the caret at the place.

#include "core/writedoc.h"

#include <QString>
#include <QVector>

namespace jp::write {

enum class GoToKind { Page, Section, Line, Bookmark, Footnote, Heading };

constexpr int kMostGoToItems = 5000;          // the choices listed for a kind (bookmarks, footnotes, headings, sections)
constexpr int kMostGoToNumber = 10000000;     // a number past this is no page, section, line, or note

struct GoToItem {
    QString label;
    int position = 0;
};

// What a person can pick for a kind, in the order of the text: bookmarks (the hidden ones, whose names begin with an underscore, left out), footnotes (by their numbers), headings (the
// paragraphs with an outline level, by their text), and sections. Pages and lines have no list.
QVector<GoToItem> goToItems(WriteDoc &doc, GoToKind kind);

// The place in the story that `what` names for the kind: a number for a page, section, line, footnote, or heading (with a + or - first, the number counted from the one the position
// `from` is in: +1 is the next); the name of a bookmark (without regard to case); or, for a heading, a part of its text. -1, with `problem` told as a sentence, when there is no such place.
int goToPosition(WriteDoc &doc, GoToKind kind, const QString &what, int from, QString *problem = nullptr);

// The headings in order, each with its outline level (0 is the top).
struct OutlineHeading {
    QString label;
    int position = 0;
    int level = 0;
};
QVector<OutlineHeading> outlineHeadings(WriteDoc &doc);

// How many paragraphs the looks for sections have visited since the program began (a count for tests of what finding the sections costs).
qint64 goToBlocksVisitedForTest();

} // namespace jp::write
