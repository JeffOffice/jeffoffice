#pragma once
// A web page read as a document: the words with headings, bold, italic, underline, strike-through, superscript and subscript, color, alignment, lists (their markers written as
// text), and tables with joined columns. Nothing is fetched: a picture, style sheet, or script a page names is ignored, and no file or address is opened. A page larger than
// kMostHtmlBytes is refused; tables nested deeper than kMaxTableDepth, and more than kMostHtmlBlocks paragraphs, are left out.

#include "wml/wml.h"

#include <QByteArray>
#include <QString>

namespace jp::office::wml {

constexpr qint64 kMostHtmlBytes = 32 * 1024 * 1024;
constexpr int kMostHtmlBlocks = 200000;

// Whether the bytes begin like a web page (after a byte order mark and blanks: a doctype, or an html, head, or body tag).
bool looksLikeHtml(const QByteArray &bytes);
// The page made into a new document (newDocument()'s styles and page); false, with the reason, when it is too large.
bool readHtml(const QByteArray &html, Document *out, QString *error = nullptr);

} // namespace jp::office::wml
