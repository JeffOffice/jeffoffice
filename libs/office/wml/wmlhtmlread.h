#pragma once
// A web page read as a document: the words with headings, bold, italic, underline, strike-through, superscript and subscript, color, alignment, links, lists (real bulleted and
// numbered lists), pictures held in the page or in files beside it, and tables with joined columns. Nothing is fetched: a style sheet or script a page names is ignored, and no address of the
// network is opened. A page larger than
// kMostHtmlBytes is refused; tables nested deeper than kMaxTableDepth, and more than kMostHtmlBlocks paragraphs, are left out.

#include "wml/wml.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>
#include <optional>

namespace jp::office::wml {

constexpr qint64 kMostHtmlBytes = 32 * 1024 * 1024;
// The page's own parser takes time that grows with the square of how deep the parts of a page stand inside one another (measured: 4,000 nested tables 2 s, 16,000 nested tables 51 s, 16,000 nested
// lists 34 s), so a page is looked at once, in time that grows with its size, and refused before the parser sees it when it goes past these. Real pages nest up to about 40 parts deep (tables up to
// about 10) and keep fewer than 50,000 cells in a table; each limit is ten times or more of that.
constexpr int kMostPageDepth = 600;          // parts (elements) standing inside one another
constexpr int kMostHtmlTableDepth = 100;     // tables standing inside one another
constexpr int kMostHtmlCells = 200000;       // cells (td and th) in the whole page
constexpr int kMostHtmlRows = 100000;        // rows (tr) in the whole page
constexpr int kMostHtmlBlocks = 200000;
constexpr int kMostHtmlImages = 500;       // pictures read from one page
constexpr int kMostHtmlLinks = 5000;       // links read from one page
constexpr qint64 kMostHtmlImageBytes = qint64(32) << 20;   // one picture, and (four times) all of them

// Whether the bytes begin like a web page (after a byte order mark and blanks: a doctype, or an html, head, or body tag).
bool looksLikeHtml(const QByteArray &bytes);
// The tables of a page, each as its rows of cell texts (a cell's text is its paragraphs, joined by a line break; a cell a neighbor spans over is empty), in the order they stand in the page.
// A table inside a table or a cell is not listed, and no more than kMostHtmlTables are: the rest are left out, with `tooMany` set. The same reader as readHtml (nothing is fetched), and the same limits
// on the page's size, depth, and cells; a page past one is refused with the reason in `error`, as is a page whose every table is larger than the cells a table may have.
constexpr int kMostHtmlTables = 64;
struct HtmlTable {
    QVector<QStringList> rows;
};
QVector<HtmlTable> readHtmlTables(const QByteArray &html, bool *tooMany = nullptr, QString *error = nullptr);
// Cells that a clipboard holds as a table (a spreadsheet's copy): the HTML when it is a table and nothing else, else the text when every line has tabs in it. Nothing when there are fewer than two cells,
// two lines of text, or when the HTML has anything but one table in it, or passes the limits of depth and cells above. The table is cut to kMostClipRows rows, kMostClipColumns columns, and kMostClipCellChars characters a cell (`truncated`),
// and no more than kMostClipChars characters of the text or kMostClipHtmlBytes of the HTML are looked at, so a hostile clipboard costs a bounded amount.
constexpr int kMostClipRows = 500, kMostClipColumns = 63, kMostClipCellChars = 4000;
constexpr int kMostClipChars = 1 << 20, kMostClipHtmlBytes = 2 << 20;
struct ClipTable {
    QVector<QStringList> rows;     // every row has as many cells as the widest
    bool truncated = false;
};
std::optional<ClipTable> clipTable(const QString &html, const QString &text);
// A look without a parse: whether the clipboard may hold cells (the HTML has a table in it, or the text has a tab and more than one line), for commands to be on or off by; clipTable says for sure.
bool mayBeClipTable(const QString &html, const QString &text);
// How many times clipTable has parsed a clipboard (the tests count what a command costs).
int clipTableParses();
// The page made into a new document (newDocument()'s styles and page); false, with the reason, when it is too large.
// A picture is read only from a data: address, or from a file under `baseDir` (the page's own folder; empty: no files) that a relative address or a file: address names, and no
// address of the network is ever opened; a link is kept when it goes to http, https, or mailto.
bool readHtml(const QByteArray &html, Document *out, QString *error = nullptr, const QString &baseDir = QString());

} // namespace jp::office::wml
