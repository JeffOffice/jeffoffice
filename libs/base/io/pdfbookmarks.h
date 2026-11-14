#pragma once
// Bookmarks for a PDF: the outline a reader shows beside the pages, made from the headings. The PDF writer of Qt makes no outline, so one is added to the finished file as an
// update appended at its end (the pages are not touched).

#include <QByteArray>
#include <QRectF>
#include <QString>
#include <QVector>

namespace jp {

struct PdfBookmark {
    QString title;
    int level = 0;   // 0 is a top entry; a deeper one belongs to the nearest shallower entry before it
    int page = 0;    // the page it goes to, counted from 0 in the PDF
};

constexpr int kMostPdfBookmarks = 5000;

// The PDF with the bookmarks in it. The PDF as it was if it has none to add, already has an outline, or is made in a way this doesn't know how to extend (nothing is guessed).
QByteArray addPdfBookmarks(const QByteArray &pdf, const QVector<PdfBookmark> &marks);

struct PdfLink {
    int page = 0;        // the page it is on, counted from 0 in the PDF
    QRectF rect;         // the area to click, in the page's own units (points), from its bottom left, as a PDF counts
    QString uri;         // a web or email address; empty for a link to a page of the file
    int toPage = -1;     // for a link to a page of the file: the page it goes to, counted from 0
    bool hasSpot = false;   // for a link to a page of the file: it goes to a spot on the page, not the page as a whole
    QPointF spot;           // the spot, in the page's own units (points), from its bottom left, as a PDF counts: where the top left of the window is put
};

constexpr int kMostPdfLinks = 20000;

// The PDF with a link annotation over each area (an update appended at its end: only the annotation lists are replaced). The links Qt's writer makes for text are dropped from every
// page, as the ones given cover them; the PDF as it was if there are none, or it is made in a way this doesn't know how to extend (nothing is guessed).
QByteArray addPdfLinks(const QByteArray &pdf, const QVector<PdfLink> &links);
// Whether an address is one a PDF link may go to: a web address (http or https) or an email address. A link to a file (a path, a UNC name such as \\\\server\\share or //server/share, or a
// file: address, with a host or not) or to any other kind of address is never written as a clickable link: addPdfLinks leaves it out, in every app.
bool clickableAddress(const QString &uri);

// The document's properties as the metadata a PDF reader shows (title, author, subject, keywords, and the program that made it); with `archive`, the mark of PDF/A part
// `archivePart` (1 or 2), conformance B.
QByteArray pdfPropertiesXmp(const QString &title, const QString &author, const QString &subject, const QString &keywords, const QString &program, bool archive = false, int archivePart = 1);

} // namespace jp
