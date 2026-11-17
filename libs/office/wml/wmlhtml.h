#pragma once
// A document as a web page: the words with their structure (headings, lists, tables, links, pictures) and what a reader needs of their looks. Two forms: "filtered" has only
// the structure and the plain marks of emphasis (no fonts, sizes, colors, or spacing), small and easy to restyle; "single file" is one page that carries its looks (a
// style sheet in its head and each paragraph's alignment and spacing) with the pictures inside it, to be opened on its own. Deleted text of a tracked change and hidden text are left out.

#include "wml/wml.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>

namespace jp::office::wml {

// A heading the page was made with: its level (1 to 6), its text, and the id its tag was given.
struct HtmlHeading {
    int level = 1;
    QString text, id;
};
// A footnote or endnote reference the page was made with: the id the reference's link has ("fnr-3"), the one its note is to be given ("fn-3"), the note's Id in the file's notes part,
// and the number the reference shows.
struct HtmlNote {
    bool endnote = false;
    int id = 0;
    QString number, referenceId, noteId;
};

struct HtmlOptions {
    bool singleFile = false;   // the looks (false: filtered)
    QString title;             // the page's title
    QString language;          // the page's language, "en-US"

    // For an e-book's chapters (all off by default; a page made without them is as it was):
    bool xhtml = false;        // well-formed XML: the empty tags closed ("<br/>"), no entity XML lacks
    bool bodyOnly = false;     // only what goes inside <body>, for the caller to put in its own page
    // A picture is put in the book as a file: the function is given the name of its part, its bytes, and its type (from the bytes, not the name: png, jpeg, gif, webp, svg, or empty for
    // any other kind), and returns the address to name; empty leaves the picture out, and its alternative text is written in its place.
    std::function<QString(const QString &partName, const QByteArray &data, const QString &mime)> pictureHref;
    QString headingIdPrefix;                   // when set, each heading's tag has an id (the prefix and its number), and `headings` lists them
    QVector<HtmlHeading> *headings = nullptr;
    QStringList *bookmarks = nullptr;          // when set, each bookmark gets an empty <span id="bm_..."/> and an internal link goes to "#bm_..."; the ids written are listed here
    QVector<HtmlNote> *notes = nullptr;        // when set, a note reference is a link to "#fn-N" (and has the id "fnr-N"), and each is listed here; the caller writes the notes
    int firstFootnote = 1, firstEndnote = 1;   // the number a note reference of this page that has none of its own shows first (a book's later chapters go on from the earlier)
    int mostNotes = 5000;                      // the note references that are made links (the rest show their numbers)
};

// The page as UTF-8 bytes. A table or list nested deeper than kMostHtmlDepth, and pictures past kMostHtmlPictureBytes together, are left out.
QByteArray documentToHtml(const Document &doc, const HtmlOptions &options);

// The level a paragraph is a heading at (1 to 6; the outline level a paragraph has, or its style's), 0 for any other; `title` is told whether it is one by the Title style.
int htmlHeadingLevel(const Document &doc, const Paragraph &para, bool *title = nullptr);

constexpr int kMostHtmlDepth = 40;
constexpr qint64 kMostHtmlPictureBytes = 256 * 1024 * 1024;

} // namespace jp::office::wml
