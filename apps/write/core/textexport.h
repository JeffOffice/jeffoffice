#pragma once
// Save as plain text: the words of the document with no formatting, in the encoding the person chooses and with the line ends the system they are going to expects. Hidden text and the text a
// tracked deletion hides are left out, as are pictures and other objects; a line break inside a paragraph is a line end, as a paragraph end is.

#include "core/writedoc.h"

namespace jp::write {

enum class TextEncoding { Utf8, Utf8WithMark, Utf16, Latin1, Ascii, Utf16Be, CodePage };   // CodePage: one of textCodePages(), by number
enum class LineEnds { Unix, Windows, ClassicMac };   // LF, CR LF, and CR

// The document's text as the file would hold it. `lost`, if given, gets how many characters the encoding can't hold (each is written as a question mark).
QByteArray plainTextFile(WriteDoc &doc, TextEncoding encoding, LineEnds ends, int *lost = nullptr, int codePage = 1252);

// The text itself, before the encoding (paragraphs separated by line feeds).
QString visibleText(WriteDoc &doc);

// The code pages a text file can be read and written in, as {number, label}: the Windows, DOS, ISO 8859, and Mac single-byte pages, and the East Asian double-byte ones.
struct TextCodePage {
    int number;
    const char *label;   // (marked for translation under "jp::write::TextFile")
};
const QVector<TextCodePage> &textCodePages();

// Whether a page can be written here (the single-byte ones always; a double-byte one where the system or Qt converts to it).
bool canWriteCodePage(int codePage);

// How a text file was found to be written: its encoding, its page (for CodePage), and its line ends.
struct TextForm {
    TextEncoding encoding = TextEncoding::Utf8;
    int codePage = 1252;
    LineEnds ends = LineEnds::Unix;
};

// The form of the bytes of a text file: a byte order mark says UTF-8 or UTF-16; else valid UTF-8 is UTF-8, and anything else is in `fallbackPage` (the Windows page of the person's
// language when none is given). The line ends are the first line's, LF if the file has just one line.
TextForm detectTextForm(const QByteArray &bytes, int fallbackPage = 0);

// The characters of the bytes in the form (the byte order mark dropped); every line end, CR LF or CR alone, is a line feed. Empty bytes are an empty string.
QString decodeTextFile(const QByteArray &bytes, const TextForm &form);

// Whether the bytes are plain text and not a package or other binary file: no NUL bytes (but UTF-16 with its mark, which has them), and none of the signatures of a ZIP, a compound file,
// a PDF, or an RTF file.
bool looksLikePlainText(const QByteArray &bytes);

} // namespace jp::write
