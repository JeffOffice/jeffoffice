#pragma once
// What Insert, Design, and Layout put into a document that the editing operations (editops.h) do not: a symbol in a font of its own, and a blank page.

#include <QByteArray>
#include <QString>

class QMimeData;
class QTextCursor;

namespace jp::write {

class WriteDoc;

// A symbol at the caret (replacing the selection), in the font given (the text's own when empty); one step of the history.
void insertSymbol(WriteDoc &doc, QTextCursor &c, const QString &symbol, const QString &font);
// A whole empty page at the caret: the page ends at the caret, the next page is empty, and the text that followed goes on the page after it; one step of the history.
void insertBlankPage(WriteDoc &doc, QTextCursor &c);

// Cover pages: an original design (a title, a subtitle, an author, and the date, on a page of their own before the text, as ordinary paragraphs to be written over), put in at the start
// of the text as one step that undoes. A document has one: a new one takes the place of the one there. The cover is the text of a hidden bookmark, which is how it is found again, in the file too.
enum class CoverDesign { Classic, Band, Plain };
bool hasCoverPage(WriteDoc &doc);
bool insertCoverPage(WriteDoc &doc, CoverDesign design, QString *problem = nullptr);
bool removeCoverPage(WriteDoc &doc);

// The most a file inserted as text may be (bytes).
constexpr qint64 kMostInsertedFile = qint64(64) << 20;
// The most paragraphs (lines of a plain text file) it puts in.
constexpr int kMostInsertedParagraphs = 200000;
// The text of another file (a `.docx`, `.doc`, `.rtf`, or `.odt` document, with its looks, pictures, and links; or a plain text file) put in at the caret, replacing the selection;
// one step of the history. False, with `problem` told as a sentence, where the file is too large or is neither.
bool insertTextFromFile(WriteDoc &doc, QTextCursor &c, const QByteArray &bytes, QString *problem = nullptr);

// Formatted text from another program (the clipboard or a drop) as Rich Text Format: the most bytes of it that are read (what is more is left alone, and the
// paste goes by the HTML or the text), the bytes the mime data holds of it (empty when it holds none, or what it holds does not begin as Rich Text Format does),
// and the copy that the RTF reader makes of it, as a clip to paste with its looks, pictures, and links.
constexpr qint64 kMostPastedRtf = qint64(16) << 20;
// (rtfOfMime reads the data, once: a mime object has no way to say how big a format is before it is read, so one read is the least that can be done; it is not asked for to find out
// whether a paste is possible: hasRtfFormat looks at the names of the formats only.)
QByteArray rtfOfMime(const QMimeData *mime);
bool hasRtfFormat(const QMimeData *mime);
// HTML from another program: the most bytes of it that are read, and the HTML made safe to read into the document: a picture, a background, or a style that names anything but a
// data: or http(s): address is taken out, so that nothing on this computer is read because a page asked.
constexpr qint64 kMostPastedHtml = qint64(16) << 20;
QString safeClipHtml(const QString &html);
// The most characters of text from another program that are split, pasted, or dropped.
constexpr qint64 kMostPastedTextChars = qint64(16) << 20;
struct Clip;
bool clipFromRtf(const QByteArray &bytes, Clip *out, QString *problem = nullptr);

} // namespace jp::write
