#pragma once
// Review > Check Accessibility: what in a document is hard for some readers (or for a screen reader) to use.

#include "core/writedoc.h"

#include <QString>
#include <QVector>

namespace jp::write {

struct AccessIssue {
    enum Kind { MissingAltText, TableWithoutHeader, BlankTableCell, HeadingOrder, LowContrast, FloatingPicture, MergedCells, LinkText };
    enum Level { Error, Warning, Tip };
    Kind kind = MissingAltText;
    Level level = Warning;
    int position = 0;       // where in the text it is (the caret goes there)
    QString message;
};

// The contrast ratio of two colors (1 to 21), as the Web Content Accessibility Guidelines work it out.
double contrastRatio(const QColor &a, const QColor &b);
// Whether the words of a link say nothing of where it goes: empty, or words such as "click here" or "read more", or a web address as it is.
bool unhelpfulLinkText(const QString &text);
// The problems found in the text of the document (at most `most`, in the order of the text). Tables are looked at a cell at a time, so a document with very many cells is checked in part.
QVector<AccessIssue> checkAccessibility(WriteDoc &doc, int most = 500);

} // namespace jp::write
