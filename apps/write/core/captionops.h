#pragma once
// Captions and tables of figures: Insert Caption puts a paragraph of the Caption style with its label and a SEQ field that counts, above or below a picture, a table, or a paragraph, and
// Insert Table of Figures puts a TOC field that lists the captions of one label (the fields themselves are core/fieldops.h's, the table's lines core/tocops.h's).

#include "core/writedoc.h"

#include <QStringList>
#include <QTextCursor>

namespace jp::write {

// The labels a document has to begin with ("Figure", "Table", and "Equation"), then the labels its SEQ fields already count.
QStringList captionLabelChoices(WriteDoc &doc);

struct CaptionOptions {
    QString label = QStringLiteral("Figure");
    QString text;                // what follows the number (": A title"), as typed
    bool above = false;          // the caption goes above the paragraph, picture, or table (else below)
    bool excludeLabel = false;   // only the number, without the label
    int chapterLevel = 0;        // the heading level (1 to 9) whose number goes before the caption's ("2-1"); 0 for a number that counts the whole text
    QString separator = QStringLiteral("-");
};
// The caption's text as the paragraph holds it before the fields are worked out: the label, a blank, and the number.
// Puts the caption in a paragraph of its own next to the one the caret is in (next to the whole table where the caret is in a table), numbers it, and renumbers the captions of the label
// that come after it. One step of the history. False, with the reason, where it can't be put (a header, a footer, or a note, or no label).
bool insertCaption(WriteDoc &doc, QTextCursor &c, const CaptionOptions &options, QString *problem = nullptr);

struct FiguresOptions {
    QString label = QStringLiteral("Figure");
    bool includeLabel = true;    // each line is the caption whole (else without the label and number)
    bool pageNumbers = true;
    bool rightAlign = true;
    QString leader = QStringLiteral("dot");
    bool hyperlinks = true;
    QString style;               // the lines are the paragraphs of this style (by its name) instead of the captions of a label; empty for a label's captions
};
// The names of the document's paragraph styles that are shown in its style list, to choose from for a table of figures built from a style.
QStringList paragraphStyleNames(WriteDoc &doc);
// The code a table of figures with these options has.
QString figuresCode(const FiguresOptions &options);
// Puts a table of figures, built now, at the caret (in a paragraph of its own), as one step of the history.
bool insertTableOfFigures(WriteDoc &doc, QTextCursor &c, const FiguresOptions &options, bool *truncated = nullptr);

} // namespace jp::write
