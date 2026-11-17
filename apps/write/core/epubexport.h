#pragma once
// Save as an e-book (EPUB): a chapter for each Heading 1 (what comes before the first one is a chapter of its own), the contents from the Heading 1 to Heading 3 paragraphs,
// the title and author from the document's properties, pictures and tables as the web page writer (libs/office/wml/wmlhtml.h) makes them, and each chapter's footnotes and
// endnotes at its end, with a link back to where the note is. The package itself is libs/office/epub's.

#include "wml/wml.h"

#include <QByteArray>
#include <QString>

namespace jp::write {

// What a document's e-book holds at most; what is past these is left out (the chapters past the last are in it, in one).
constexpr int kMostEpubChapters = 500;
constexpr int kMostEpubNavEntries = 4000;     // the contents' lines
constexpr int kMostEpubHeadingLevel = 3;      // the contents lists Heading 1 to Heading 3
constexpr int kMostEpubPictures = 1000;
constexpr qint64 kMostEpubPictureBytes = qint64(32) * 1024 * 1024;     // one picture
constexpr qint64 kMostEpubPicturesBytes = qint64(256) * 1024 * 1024;   // all of them
constexpr int kMostEpubNotes = 5000;

struct EpubExportOptions {
    QString title, author;       // empty: Untitled, and no author
    QString language;            // empty: the language the document's default text says, else "en-US"
    QString identifier;          // empty: a new one
    bool cover = false;          // the first picture of the document is the cover
    // The bounds (a test sets smaller ones).
    int mostChapters = kMostEpubChapters, mostNavEntries = kMostEpubNavEntries, mostPictures = kMostEpubPictures;
    qint64 mostPictureBytes = kMostEpubPictureBytes, mostPicturesBytes = kMostEpubPicturesBytes;
};

// The book as bytes (empty, with the reason, when it can't be made).
QByteArray epubFromDocument(const office::wml::Document &doc, const EpubExportOptions &options, QString *error = nullptr);

} // namespace jp::write
