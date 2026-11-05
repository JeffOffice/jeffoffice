#pragma once
// A document as a web page: the words with their structure (headings, lists, tables, links, pictures) and what a reader needs of their looks. Two forms: "filtered" has only
// the structure and the plain marks of emphasis (no fonts, sizes, colors, or spacing), small and easy to restyle; "single file" is one page that carries its looks (a
// style sheet in its head and each paragraph's alignment and spacing) with the pictures inside it, to be opened on its own. Deleted text of a tracked change and hidden text are left out.

#include "wml/wml.h"

#include <QByteArray>
#include <QString>

namespace jp::office::wml {

struct HtmlOptions {
    bool singleFile = false;   // the looks (false: filtered)
    QString title;             // the page's title
    QString language;          // the page's language, "en-US"
};

// The page as UTF-8 bytes. A table or list nested deeper than kMostHtmlDepth, and pictures past kMostHtmlPictureBytes together, are left out.
QByteArray documentToHtml(const Document &doc, const HtmlOptions &options);

constexpr int kMostHtmlDepth = 40;
constexpr qint64 kMostHtmlPictureBytes = 256 * 1024 * 1024;

} // namespace jp::office::wml
