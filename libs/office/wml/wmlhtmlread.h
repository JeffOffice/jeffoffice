#pragma once
// A web page read as a document: the words with headings, bold, italic, underline, strike-through, superscript and subscript, color, alignment, links, lists (real bulleted and
// numbered lists), pictures held in the page or in files beside it, and tables with joined columns. Nothing is fetched: a style sheet or script a page names is ignored, and no address of the
// network is opened. A page larger than
// kMostHtmlBytes is refused; tables nested deeper than kMaxTableDepth, and more than kMostHtmlBlocks paragraphs, are left out.

#include "wml/wml.h"

#include <QByteArray>
#include <QString>

namespace jp::office::wml {

constexpr qint64 kMostHtmlBytes = 32 * 1024 * 1024;
constexpr int kMostHtmlBlocks = 200000;
constexpr int kMostHtmlImages = 500;       // pictures read from one page
constexpr int kMostHtmlLinks = 5000;       // links read from one page
constexpr qint64 kMostHtmlImageBytes = qint64(32) << 20;   // one picture, and (four times) all of them

// Whether the bytes begin like a web page (after a byte order mark and blanks: a doctype, or an html, head, or body tag).
bool looksLikeHtml(const QByteArray &bytes);
// The page made into a new document (newDocument()'s styles and page); false, with the reason, when it is too large.
// A picture is read only from a data: address, or from a file under `baseDir` (the page's own folder; empty: no files) that a relative address or a file: address names, and no
// address of the network is ever opened; a link is kept when it goes to http, https, or mailto.
bool readHtml(const QByteArray &html, Document *out, QString *error = nullptr, const QString &baseDir = QString());

} // namespace jp::office::wml
