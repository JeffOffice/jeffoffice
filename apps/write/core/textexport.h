#pragma once
// Save as plain text: the words of the document with no formatting, in the encoding the person chooses and with the line ends the system they are going to expects. Hidden text and the text a
// tracked deletion hides are left out, as are pictures and other objects; a line break inside a paragraph is a line end, as a paragraph end is.

#include "core/writedoc.h"

namespace jp::write {

enum class TextEncoding { Utf8, Utf8WithMark, Utf16, Latin1, Ascii };
enum class LineEnds { Unix, Windows };

// The document's text as the file would hold it. `lost`, if given, gets how many characters the encoding can't hold (each is written as a question mark).
QByteArray plainTextFile(WriteDoc &doc, TextEncoding encoding, LineEnds ends, int *lost = nullptr);

// The text itself, before the encoding (paragraphs separated by line feeds).
QString visibleText(WriteDoc &doc);

} // namespace jp::write
