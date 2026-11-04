#pragma once
// The Review tab's counting and proofing helpers that need no window: the word count.

#include "core/writedoc.h"

#include <QTextCursor>

namespace jp::write {

struct WordStats {
    int words = 0;
    int charactersNoSpaces = 0;
    int charactersWithSpaces = 0;
    int paragraphs = 0;
};
// The count of the text (or, with a selection, of what is selected). Hidden text and text a tracked change deleted are not counted, and a picture, a field's code, or a bookmark is no letter.
// `notes`: also the words of the footnotes and endnotes (without it the endnotes the pages set after the text are left out too). A paragraph is counted when it has any text.
WordStats countWords(WriteDoc &doc, const QTextCursor &selection, bool notes);
// The count of one story's text (or of the range [from, to) of it; to < 0 is the end).
WordStats countWordsIn(const QTextDocument *story, int from = 0, int to = -1, bool withEndnotes = true);


// ---- the language of text
// The tag ("en-GB") of the language the text at the caret (the start of the selection) is checked in: its own, else the document's, else US English.
QString proofingLanguageAt(WriteDoc &doc, const QTextCursor &c);
// Gives the selected text (the text typed next, with none) the language `tag`; an empty tag takes the language the run states away, so the document's holds. The file keeps it as w:lang. One step.
void setProofingLanguage(WriteDoc &doc, QTextCursor &c, const QString &tag);
// Marks the selected text (the text typed next) "do not check spelling or grammar" (w:noProof), or takes the mark away. One step.
void setNoProof(WriteDoc &doc, QTextCursor &c, bool noProof);
// Whether all the selected text (the text typed next, with none) has the mark.
bool noProofAt(const QTextCursor &c);
// The format with its language set (the run's w:lang kept with its other attributes, and put where the schema has it).
QTextCharFormat withProofingLanguage(QTextCharFormat format, const QString &tag);

} // namespace jp::write
