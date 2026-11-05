#pragma once
// Bookmarks for a PDF: the outline a reader shows beside the pages, made from the headings. The PDF writer of Qt makes no outline, so one is added to the finished file as an
// update appended at its end (the pages are not touched).

#include <QByteArray>
#include <QString>
#include <QVector>

namespace jp::write {

class WriteDoc;

struct PdfBookmark {
    QString title;
    int level = 0;   // 0 is a top entry; a deeper one belongs to the nearest shallower entry before it
    int page = 0;    // the page it goes to, counted from 0 in the PDF
};

constexpr int kMostPdfBookmarks = 5000;

// The PDF with the bookmarks in it. The PDF as it was if it has none to add, already has an outline, or is made in a way this doesn't know how to extend (nothing is guessed).
QByteArray addPdfBookmarks(const QByteArray &pdf, const QVector<PdfBookmark> &marks);

// The headings of the document (the paragraphs with an outline level) as bookmarks. `pages` are the pages the PDF holds, in order, by their index in the document; a heading on a
// page the PDF leaves out is left out.
QVector<PdfBookmark> headingBookmarks(WriteDoc &doc, const QVector<int> &pages);

// The document's properties as the metadata a PDF reader shows (title, author, subject, keywords, and the program that made it).
QByteArray pdfPropertiesXmp(const QString &title, const QString &author, const QString &subject, const QString &keywords, const QString &program, bool archive = false);

} // namespace jp::write
