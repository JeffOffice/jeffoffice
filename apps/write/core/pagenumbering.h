#pragma once
// Format Page Numbers: how a section numbers its pages (the look of the number, the number to start at or "continue from the section before", and the chapter number put before the page
// number), applied to the section the caret is in or to every section, as one step that undoes. What a file states about the numbering that this does not set stays as it was.

#include "core/writedoc.h"

namespace jp::write {

struct PageNumbering {
    QString format = QStringLiteral("decimal");   // decimal, lowerLetter, upperLetter, lowerRoman, upperRoman, or numberInDash
    bool continueFromPrevious = true;
    int startAt = 1;                              // for continueFromPrevious off
    int chapterStyle = 0;                         // the heading level (1 to 9) whose number comes first; 0 for none
    QString chapterSeparator = QStringLiteral("hyphen");   // hyphen, period, colon, emDash, or enDash
    bool operator==(const PageNumbering &o) const
    {
        return format == o.format && continueFromPrevious == o.continueFromPrevious && (continueFromPrevious || startAt == o.startAt) && chapterStyle == o.chapterStyle &&
               (chapterStyle == 0 || chapterSeparator == o.chapterSeparator);
    }
};

// What the section states now.
PageNumbering pageNumberingOf(const office::wml::SectionProps &props);

// The formats, as {value, label} (the labels are translated).
QVector<QPair<QString, QString>> pageNumberFormats();

// Sets the numbering of section `section`, or of every section. Returns how many sections it changed.
int applyPageNumbering(WriteDoc &doc, int section, bool wholeDocument, const PageNumbering &numbering);

} // namespace jp::write
