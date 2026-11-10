#pragma once
// PDF pages through PDFium. A page placed in a publication is a picture
// whose image is a one-page PDF (format "pdf"): on screen it's drawn from a
// raster (PDFium's own), and in PDFs, printing and SVG it's drawn as the
// page's own paths, glyph outlines and pictures, so it stays sharp.

#include <QByteArray>
#include <QImage>
#include <QRectF>
#include <QRegion>
#include <QSizeF>
#include <QString>
#include <QStringList>
#include <QVector>

#include <memory>

class QPainter;

namespace jp {

class PdfDocument {
public:
    // keeps its own copy; `password` opens a PDF that has one (the open password or the owner's)
    explicit PdfDocument(const QByteArray &bytes, const QString &password = QString());
    ~PdfDocument();
    PdfDocument(const PdfDocument &) = delete;
    PdfDocument &operator=(const PdfDocument &) = delete;

    bool isValid() const;
    // Whether the PDF could not be opened for want of the right password (none was given, or it was wrong).
    bool needsPassword() const;
    // Whether the PDF, opened as it was, asks not to have its text and pictures copied (an owner's limit: a request, not a protection).
    bool asksNotToBeCopied() const;
    int pageCount() const;
    QSizeF pageSize(int page) const;             // points, as the page shows (turned pages turned)
    QByteArray extractPage(int page) const;      // the page alone, as a one-page PDF
    QImage render(int page, const QSize &px) const;   // PDFium's raster, white behind it
    // Draws the page's objects into target as vectors: paths, text (each
    // glyph's outline), pictures, form contents; shadings come from a raster.
    // The drawing is checked against PDFium's own (once per page), and
    // wherever they differ (hidden layers, blending, glyphs it can't name)
    // PDFium's raster is drawn there instead.
    void play(QPainter *p, int page, const QRectF &target) const;
    double rasterShare(int page) const;   // how much of the page play() draws from the raster, 0-1
    // The page's text as a reader that copies it gets it (the letters each glyph says it stands for, in the order the page draws them); empty
    // for a page with none, or past the last.
    QString text(int page) const;
    // The same by lines: the letters on one baseline, in the order the page draws them (what a reader that copies them gets as a line, where a
    // text's lines are not told apart by its own line breaks).
    QStringList lines(int page) const;

    // The bookmarks the reader shows (depth first, `level` 0 for a top one) and the page each goes to (from 0, -1 for none).
    struct OutlineEntry {
        QString title;
        int level = 0;
        int page = -1;
    };
    QVector<OutlineEntry> outline() const;
    // Whether the file says it is tagged (its catalog's mark info), and the tags of a page: its structure elements in the order a screen reader reads them (depth first), each
    // with the words the page draws in its marked content.
    struct StructElement {
        QString type;     // the structure type: P, H1, Figure, Table...
        QString alt;      // its alternative text
        QString text;     // the text drawn in it
        int depth = 0;
        int mcid = -1;    // its marked content on the page, or -1
    };
    bool isTagged() const;
    QVector<StructElement> structure(int page) const;

    static bool looksLikePdf(const QByteArray &bytes);
    // The document for these bytes, kept for the next ask (a few at a time)
    // so each page is checked once however often it's drawn.
    static std::shared_ptr<const PdfDocument> shared(const QByteArray &bytes);

private:
    struct Check {
        QSize size;        // the check's pixels
        QRegion raster;    // in those pixels: where the raster is used
    };
    Check check(int page) const;
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace jp
