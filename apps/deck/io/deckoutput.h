#pragma once
// Printing a presentation and making its PDF: printed slides, notes pages,
// handouts of 1 to 9 slides, and the outline. Every layout is drawn by
// paintOutputPage onto any painter set up in points (a PDF, the printer, the
// print preview, a picture), so what is previewed is what is printed.
//
// Slides print one to a page, in the slide's own shape. Notes pages, handouts,
// and the outline print on the locale's paper (letter or A4), upright: a notes
// page has the slide framed above and its speaker notes below; a handout
// frames each slide in a grid, with a header and footer (the title and date,
// the page number), and three to a page leaves lines beside each slide to write
// on. Hidden slides are left out unless the options ask for them.

#include "core/document.h"
#include "io/pdfcrypt.h"
#include "io/pdftags.h"
#include "render/picturefile.h"
#include "render/renderer.h"

#include <QImage>
#include <QMarginsF>
#include <QSizeF>
#include <QStringList>
#include <QString>
#include <QVector>
#include <functional>

class QPainter;

namespace jp::deck {

enum class PrintLayout { Slides, NotesPages, Handout1, Handout2, Handout3, Handout4, Handout6, Handout9, Outline };

struct OutputOptions {
    PrintLayout layout = PrintLayout::Slides;
    bool hiddenSlides = false;    // print the slides that are hidden too
    bool frameSlides = false;     // a thin frame around each printed slide (notes pages and handouts always frame them)
    bool grayscale = false;
    bool blackAndWhite = false;   // pure black and white (with grayscale): each point is black or white, nothing between
    bool scaleToFit = true;       // printed slides are scaled to fit the paper; else a slide that fits it is printed at its own size, centered (one that does not is still made to fit)
    bool skipNotesText = false;   // notes pages without the notes' text (the Notes Page view shows it in a box to type in)
    QVector<int> slides;          // 0-based slides to print, in this order; empty: all
    bool horizontalOrder = true;  // handouts of 4 or more: across, then down (else down, then across)
    // For printing (the File screen's Print page): where the pages go, and how they come out.
    QSizeF paper;                 // printing: the paper the sheets are, in points and upright (empty: the locale's for notes pages and the rest, the slide's own shape for slides)
    QString printer;              // the printer's name; empty is the default one
    bool toFile = false;          // a PDF file, asked for where it is printed, in place of a printer
    int copies = 1;
    bool collate = true;          // the whole set again for each copy, else each page's copies together
    bool reverse = false;         // the last page first
    QMarginsF margins;            // printing: blank space round each sheet, in points; the whole page is scaled to fit inside it (none: the page uses the whole sheet)
    bool highQuality = false;     // printing: pages that are made into pixels (grayscale and pure black and white) are made at 600 to the inch, not 300
    bool comments = false;        // slides and notes pages: a page of the slide's review comments (who wrote them and what they say) after each slide that has any
    bool openAfter = false;       // for a PDF: opened in the system's viewer once saved
    bool bookmarks = true;        // for a PDF: a bookmark for each slide (its title, or "Slide N")
    bool tags = true;             // for a PDF: the structure for screen readers (tagged PDF): reading order, headings, alt text, tables
    bool archive = false;         // for a PDF: PDF/A-1b, the form for keeping (every font inside, no transparency, the standard's mark in its metadata); never with a password
};

// The slides the options print, 0-based and in order: the ones asked for (all
// when none are), without the hidden ones unless asked for, and without any
// the presentation doesn't have.
QVector<int> outputSlides(const Document &d, const OutputOptions &o);
// A range of slides as typed, "1-3, 5" or "7-" or "-2": the slides' numbers
// and spans of them, counted from 1, separated by commas or semicolons, in
// the order typed. Returns the 0-based slides of a presentation of `count`;
// empty when the text is not a range or names no slide (a span runs to the
// last slide at most).
QVector<int> parseSlideRange(const QString &text, int count);
// Slides on one page of a handout layout; 0 for the others.
int slidesPerPage(PrintLayout layout);
// The locale's paper (letter or A4), upright, in points.
QSizeF localePaper();
// The size of a page of the output in points: the slide's for printed slides,
// else `paper` (the locale's when empty).
QSizeF outputPageSize(const Document &d, const OutputOptions &o, const QSizeF &paper = QSizeF());
// The size of a sheet that printing makes: what the output's own size is, or the paper chosen (a slide's sheet turned to its shape).
QSizeF printSheetSize(const Document &d, const OutputOptions &o);
// Pages of the output. The outline's depends on the paper (the locale's when empty).
int outputPageCount(const Document &d, const OutputOptions &o, const QSizeF &paper = QSizeF());
// The outline as text: each slide's number and title, then its body text, two
// spaces in for each level.
QString outputOutlineText(const Document &d, const OutputOptions &o);

// Paints page `page` (0-based) of the output onto `p`, which is set up in
// points for a page of `paper` size with its corner at the origin. A slide
// goes at its own size or, on other paper, as large as fits and centered. The
// page is painted as the painter's device wants it: the text as a PDF or the
// printer draws it, and, when grayscale is asked for, in gray (a PDF or printer
// set to gray converts the colors itself, anything else is painted as gray pixels).
void paintOutputPage(QPainter *p, const Document &d, LayoutCache *cache, const OutputOptions &o, int page, const QSizeF &paper, PdfTagger *tags = nullptr);

// Writes the PDF: vector text with its fonts embedded, the title from the
// presentation's properties or the file's name. The old file stays whole
// until the new one is complete; on failure says why in `error`.
// With `protect` (and its open password set), the file is written once, encrypted: the plain PDF stays in memory. The caller wipes `protect`.
// `notTagged`, when given, is set to the reason where the PDF was to be tagged and is not (and left empty otherwise).
bool exportPdf(const Document &d, const QString &path, const OutputOptions &o, QString *error = nullptr, const PdfProtection *protect = nullptr, QString *notTagged = nullptr);

// One slide as a picture `width` pixels across (the slide's shape), on white.
QImage slideImage(const Document &d, LayoutCache *cache, int slide, int width = 1920);

// Save slides as pictures: every slide to a folder, as "Slide 1", "Slide 2", and so on (a hidden slide keeps its number and is left out unless `hidden`), PNG or JPEG, `width`
// pixels across, drawn as slideImage draws one. Each file is written whole or not at all. A picture is at most kMostSlidePicturePixels in all: a wider one is made as wide as fits
// (the width used is in the result, and is less than asked when that happened). `progress`, when set, is called after each slide with how many are done and how many there are, and
// says false to stop; the files written so far stay.
constexpr qint64 kMostSlidePicturePixels = qint64(64) * 1000 * 1000;
// The formats of a slide's picture: PNG, JPEG, GIF (at most 256 colors), TIFF, and BMP are pixels, `width` across; SVG is drawn as vectors (the letters as outlines, pictures inside), at the slide's own size.
enum class SlidePictureFormat { Png, Jpeg, Gif, Tiff, Bmp, Svg };
QString slidePictureExtension(SlidePictureFormat format);
struct SlidePicturesOptions {
    QString folder;
    bool jpeg = false;   // JPEG when `format` is left at PNG (kept for what asked for it so)
    SlidePictureFormat format = SlidePictureFormat::Png;
    int width = 1920;
    bool hidden = false;
    std::function<bool(int done, int total)> progress;
};
struct SlidePicturesResult {
    QStringList written;   // the files made, in order
    QString error;         // why it stopped, when it did for a reason other than being asked to
    bool cancelled = false;
    int widthUsed = 0;
};
// The file name of slide `index` (from 0), and the names the options would write (the slides that are shown, or all), in order; the width that would be used (held so the picture is at most
// kMostSlidePicturePixels).
QString slidePictureName(int index, bool jpeg);
QString slidePictureName(int index, SlidePictureFormat format);
// One slide as a vector picture (SVG). A drawing holds each picture at its full size, so a slide whose pictures come to more than kMostSlidePicturePixels in all (on the slide, its layout,
// and its master) is refused: the result is empty and `why` says so (empty too when there is no such slide).
QByteArray slideSvg(const Document &d, LayoutCache *cache, int slide, QString *why = nullptr);
// The pixels of the pictures a slide draws, counted each time one is used.
qint64 slidePicturePixels(const Document &d, int slide);
QStringList slidePictureNames(const Document &d, const SlidePicturesOptions &o);
int slidePictureWidth(const Document &d, int width);
SlidePicturesResult exportSlidePictures(const Document &d, LayoutCache *cache, const SlidePicturesOptions &o);

} // namespace jp::deck
