#pragma once
// What a worksheet says about printing it, as the file keeps it: the paper and how the page is turned, the scale or the number of pages to fit, the margins, whether gridlines
// and headings are printed and whether the cells sit in the middle of the page, the header and footer text, the breaks a person put in, the print area, and the rows and columns
// repeated on every page. No drawing and no window: the pages are cut by libs/shell/grid/printlayout.h, and the header and footer text is read by sheet/headerfooter.h.
//
// Every number a file gives is bounded when it is read (a scale of 5,000 percent is 400, a margin of a thousand inches is not one), and so is every list (the breaks, the print
// areas) and every text (the header and footer), so a file cannot make the model large or the pages without end.

#include "sheet/cellref.h"

#include <QString>
#include <QVector>

class QXmlStreamAttributes;

namespace jp::office::sheet {

// The most manual breaks of one kind a sheet keeps (the format allows 1,026; the most found in a public workbook is far below), the most print areas, and the most characters in
// a header or footer text (the format allows 255 in a part; a file's whole string, with all three parts and the codes, is bounded here).
constexpr int kMaxPageBreaks = 4096;
// The most `brk` elements of one kind that are looked at (a file that repeats one a million times is not read a million times over).
constexpr int kMaxBreakElements = 4 * kMaxPageBreaks;
constexpr int kMaxPrintAreas = 1024;
constexpr int kMaxHeaderFooterChars = 8192;
// The scale of a page, in percent.
constexpr int kMinPageScale = 10;
constexpr int kMaxPageScale = 400;
// The most pages a sheet may be told to fit to (the format's own limit).
constexpr int kMaxFitPages = 32767;
// The most a margin may be, in inches (more is not a margin).
constexpr double kMaxMarginInches = 100;

enum class PageOrientation : quint8 { Default, Portrait, Landscape };
enum class PageOrder : quint8 { DownThenOver, OverThenDown };
enum class PrintComments : quint8 { None, AtEnd, AsDisplayed };
enum class PrintErrors : quint8 { Displayed, Blank, Dash, NA };

// Margins in inches, as the file gives them.
struct PageMargins {
    double left = 0.7, right = 0.7, top = 0.75, bottom = 0.75, header = 0.3, footer = 0.3;
    bool operator==(const PageMargins &o) const { return left == o.left && right == o.right && top == o.top && bottom == o.bottom && header == o.header && footer == o.footer; }
};

// A manual (or automatic) break the file lists: the new page begins at the row (column) `before`, counted from 0, which is the number the file gives (the break is after the row or column
// of that number counted from 1). `min` and `max` are the extent the file gives it (the other direction), kept to write it again.
struct PageBreak {
    quint32 before = 0;
    quint32 min = 0, max = 0;
    bool manual = true;
    bool operator==(const PageBreak &o) const { return before == o.before && min == o.min && max == o.max && manual == o.manual; }
};

// The text of the header and the footer, as the file writes it (with the & codes that sheet/headerfooter.h reads), for the pages in general, the odd and even pages, and the first.
struct HeaderFooter {
    QString oddHeader, oddFooter, evenHeader, evenFooter, firstHeader, firstFooter;
    bool differentOddEven = false;
    bool differentFirst = false;
    bool scaleWithDoc = true;
    bool alignWithMargins = true;
    bool isEmpty() const { return oddHeader.isEmpty() && oddFooter.isEmpty() && evenHeader.isEmpty() && evenFooter.isEmpty() && firstHeader.isEmpty() && firstFooter.isEmpty(); }
    bool operator==(const HeaderFooter &o) const
    {
        return oddHeader == o.oddHeader && oddFooter == o.oddFooter && evenHeader == o.evenHeader && evenFooter == o.evenFooter && firstHeader == o.firstHeader && firstFooter == o.firstFooter
               && differentOddEven == o.differentOddEven && differentFirst == o.differentFirst && scaleWithDoc == o.scaleWithDoc && alignWithMargins == o.alignWithMargins;
    }
};

// The rows and columns printed again at the top and the left of every page.
struct PrintTitles {
    bool hasRows = false, hasCols = false;
    quint32 row1 = 0, row2 = 0, col1 = 0, col2 = 0;
    bool operator==(const PrintTitles &o) const
    {
        return hasRows == o.hasRows && hasCols == o.hasCols && (!hasRows || (row1 == o.row1 && row2 == o.row2)) && (!hasCols || (col1 == o.col1 && col2 == o.col2));
    }
};

struct PageSetup {
    // pageSetup: the paper (the format's number: 1 is letter, 9 is A4, ...; a paper of no known number has its size in points below, if the file gave one), how it is turned, the scale in
    // percent (used unless the sheet is to be fitted to pages), the pages to fit to (0: as many as it takes), the number the first page has, and the rest.
    int paperSize = 1;
    double paperWidth = 0, paperHeight = 0;   // points; 0 when the file gave none
    PageOrientation orientation = PageOrientation::Default;
    int scale = 100;
    int fitToWidth = 1, fitToHeight = 1;
    bool fitToPage = false;                   // sheetPr/pageSetUpPr: the pages above, not the scale
    int firstPageNumber = 1;
    bool useFirstPageNumber = false;
    PageOrder pageOrder = PageOrder::DownThenOver;
    bool blackAndWhite = false, draft = false;
    PrintComments comments = PrintComments::None;
    PrintErrors errors = PrintErrors::Displayed;
    PageMargins margins;
    // printOptions
    bool gridLines = false, headings = false, centerHorizontally = false, centerVertically = false;
    HeaderFooter headerFooter;
    // The breaks (in the order of the file, none twice), the print area (the blocks in the order they print), and the titles: from the sheet's own names.
    QVector<PageBreak> rowBreaks, colBreaks;
    int rowBreakElements = 0, colBreakElements = 0;   // how many `brk` elements were looked at while reading (not part of the setup)
    QVector<CellRange> printArea;
    PrintTitles titles;

    bool hasPrintArea() const { return !printArea.isEmpty(); }
    bool operator==(const PageSetup &o) const;
    bool operator!=(const PageSetup &o) const { return !(*this == o); }
};

// ---- reading the elements of a sheet part

// The attributes of the element `pageSetup`, `pageMargins`, `printOptions`, `headerFooter`, `brk` (a break: `rows` says which kind), and `pageSetUpPr` read into the setup. Each number is
// bounded to what the format allows; an attribute that is not a number, or is missing, leaves the setup's value as it is.
void readPageSetupAttributes(const QXmlStreamAttributes &a, PageSetup *out);
void readPageMarginsAttributes(const QXmlStreamAttributes &a, PageSetup *out);
void readPrintOptionsAttributes(const QXmlStreamAttributes &a, PageSetup *out);
void readHeaderFooterAttributes(const QXmlStreamAttributes &a, PageSetup *out);
void readBreakAttributes(const QXmlStreamAttributes &a, bool rows, PageSetup *out);
void readFitToPageAttributes(const QXmlStreamAttributes &a, PageSetup *out);
// The text of one child of `headerFooter` (oddHeader, oddFooter, evenHeader, evenFooter, firstHeader, firstFooter) by its name; false for another name.
QString *headerFooterText(PageSetup *out, QStringView childName);

// ---- the names

// The print area and titles the names of a workbook give a sheet: `_xlnm.Print_Area` and `_xlnm.Print_Titles` that belong to it (their reference text is "Sheet1!$A$1:$D$20,Sheet1!$F$1:$G$9"
// and "Sheet1!$A:$B,Sheet1!$1:$2"). A reference that is to another sheet, or is not a block of cells, is passed by.
void readPrintAreaText(const QString &text, const QString &sheetName, PageSetup *out);
void readPrintTitlesText(const QString &text, const QString &sheetName, PageSetup *out);
// The text of the names for the setup, written for the sheet (quoted if its name needs it); empty for none.
QString printAreaText(const PageSetup &setup, const QString &sheetName);
QString printTitlesText(const PageSetup &setup, const QString &sheetName);

// ---- the paper

// The size of the paper with the format's number, in points, upright (width, height); the size of letter paper for a number that is not known.
void paperSizePoints(int paperSize, double *width, double *height);
// Whether the number is one of the known sizes.
bool isKnownPaperSize(int paperSize);
// The numbers of the sizes that are offered in menus, in the order they are shown.
QVector<int> commonPaperSizes();
// The paper's name in words: "Letter", "A4", ... ; empty for a number that is not known.
QString paperSizeName(int paperSize);
// The size of the page the setup puts the sheet on, upright turned as the orientation says, in points.
void pageSizePoints(const PageSetup &setup, double *width, double *height);
bool isLandscape(const PageSetup &setup);

} // namespace jp::office::sheet
