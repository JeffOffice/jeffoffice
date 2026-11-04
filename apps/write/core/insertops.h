#pragma once
// What Insert, Design, and Layout put into a document that the editing operations (editops.h) do not: a symbol in a font of its own, and a blank page.

#include <QString>

class QTextCursor;

namespace jp::write {

class WriteDoc;

// A symbol at the caret (replacing the selection), in the font given (the text's own when empty); one step of the history.
void insertSymbol(WriteDoc &doc, QTextCursor &c, const QString &symbol, const QString &font);
// A whole empty page at the caret: the page ends at the caret, the next page is empty, and the text that followed goes on the page after it; one step of the history.
void insertBlankPage(WriteDoc &doc, QTextCursor &c);

// The most a file inserted as text may be (bytes).
constexpr qint64 kMostInsertedFile = qint64(64) << 20;
// The most paragraphs (lines of a plain text file) it puts in.
constexpr int kMostInsertedParagraphs = 200000;
// The text of another file (a `.docx`, `.doc`, `.rtf`, or `.odt` document, with its looks, pictures, and links; or a plain text file) put in at the caret, replacing the selection;
// one step of the history. False, with `problem` told as a sentence, where the file is too large or is neither.
bool insertTextFromFile(WriteDoc &doc, QTextCursor &c, const QByteArray &bytes, QString *problem = nullptr);

} // namespace jp::write
