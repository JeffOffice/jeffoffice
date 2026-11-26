#pragma once
// The guard of every web page that comes from outside (a file that is opened, the clipboard, a drop, a field of an answer) before the page's parser (QTextDocument::setHtml,
// QTextDocumentFragment::fromHtml, or a text box's own paste) is let near it. The parser's time is not about a page's size: it grows with how deep the parts of a page stand inside
// one another, with the items of one list, with the pictures of one paragraph, with the grid a table asks for, with its style sheets, and with other things nobody has listed. So the
// time is not predicted. The page is rewritten, in one pass whose time is the page's size, into a small form that holds only what the apps use from a page (paragraphs, headings, line
// breaks, bold, italic, underline, strike-through, sub- and superscript, spans and fonts with a few style properties, links, lists, tables, and pictures), and only that form is given to the
// parser. Its worst case is measured once (see htmlguard.cpp), and the limits below keep any page at or under it. Every caller uses guardHtml() and parses its `html`, never the original.

#include <QByteArray>
#include <QString>

#include <memory>

class QMimeData;

namespace jp::office::wml {

// The limits. Real pages are far under each: the longest article has about 60,000 parts, a page saved from a browser nests about 40 parts deep (tables about 10), and a copied spreadsheet of
// 5,000 rows of 20 cells has 100,000 cells. See htmlguard.cpp for how each number was measured.
constexpr qint64 kMostHtmlBytes = 32 * 1024 * 1024;     // the page, as it is given
constexpr int kMostHtmlParts = 200000;                  // elements the rewritten page holds (a line break, a picture, and a line of preformatted text are parts)
constexpr qint64 kMostHtmlText = 24 * 1024 * 1024;      // characters of words
constexpr int kMostHtmlCells = 170000;                  // cells (td and th)
constexpr int kMostHtmlRows = 100000;                   // rows (tr)
constexpr qint64 kMostHtmlGrid = 200000;                // cells of the grids the tables ask for (each table's rows times its widest row)
constexpr qint64 kMostGuardedPictureBytes = qint64(32) << 20; // one picture a page holds (the readers take no more)
constexpr int kMostHtmlPictures = 2000;                 // pictures in the page (the readers read 500 of them)
constexpr int kMostHtmlNesting = 100;                     // parts standing inside one another in the rewritten page (more are left out; their words stay)
constexpr int kMostHtmlTableDepth = 20;                 // tables standing inside one another (more become their words)
constexpr int kMostHtmlListItems = 100;                 // items of one list (a longer list is written as several lists that go on from one another)
constexpr int kMostHtmlSpan = 999999;                   // columns or rows a cell spans (and never more than the table has: a span is never cut to a number that moves the cells after it)
constexpr int kMostHtmlSpanCells = 20000;               // cells in the page that span (the parser takes about 25 microseconds for each, and 3 for each cell it covers); past it the page is refused
constexpr qint64 kMostHtmlSpanCovered = 100000;         // cells that the spanning cells of the page cover
constexpr int kMostHtmlStyles = 200000;                 // style attributes in the page (more are left out)
constexpr qint64 kMostHtmlStyleChars = 16 * 1024 * 1024; // characters of style attributes in the page (more are left out; a copied spreadsheet of 100,000 styled cells has 10 million, a price list of 160,000 has 16.3 million)
constexpr int kMostHtmlOpen = 4096;                     // parts of the given page open at once (more are left out; their words stay)
constexpr int kMostStyleChars = 1024;                   // characters of a style attribute that are read
constexpr int kMostStyleDeclarations = 16;              // style properties kept for one part

enum class HtmlRefusal { None, TooLarge, TooManyParts, TooManyCells };

// The limits of one guard (the named constants above; the tests turn one down to prove it holds).
struct HtmlLimits {
    qint64 bytes = kMostHtmlBytes;
    qint64 text = kMostHtmlText;
    qint64 grid = kMostHtmlGrid;
    int parts = kMostHtmlParts;
    int cells = kMostHtmlCells;
    int rows = kMostHtmlRows;
    int pictures = kMostHtmlPictures;
    int depth = kMostHtmlNesting;
    int tableDepth = kMostHtmlTableDepth;
    int listItems = kMostHtmlListItems;
    int span = kMostHtmlSpan;
    int spanCells = kMostHtmlSpanCells;
    qint64 spanCovered = kMostHtmlSpanCovered;
    int styles = kMostHtmlStyles;
    qint64 styleChars = kMostHtmlStyleChars;
    int open = kMostHtmlOpen;
};

// What the guard found and did (the tests count it).
struct HtmlCounts {
    qint64 parts = 0;       // elements written
    qint64 work = 0;        // characters read and parts closed (the guard's own work: never more than a few times the page's size)
    qint64 text = 0;        // characters of words written
    qint64 cells = 0, rows = 0, grid = 0, pictures = 0;
    int deepest = 0;        // the most parts that stand inside one another in what was written
    int mostOpen = 0;       // the most parts of the given page that stood open at once
    int tables = 0;         // tables written
    int flattened = 0;      // parts left out because they stood too deep (their words stay)
    int listSplits = 0;     // lists written as two because of their items
    int spanCells = 0;      // cells that span (columns or rows) in what was written
    qint64 spanCovered = 0; // cells those cover
    qint64 styles = 0, styleChars = 0;      // style attributes written, and their characters
    int stylesLeftOut = 0;  // style attributes left out because the page held too many
};

struct GuardedHtml {
    HtmlRefusal refusal = HtmlRefusal::None;
    QString html;           // what the parser is given (empty when refused)
    HtmlCounts counts;
};

// The one guard. A page that cannot be made small enough (past a limit of bytes, parts, cells, or grid) is refused: nothing is parsed.
GuardedHtml guardHtml(const QString &html, const HtmlLimits &limits = HtmlLimits());
GuardedHtml guardHtml(const QByteArray &utf8, const HtmlLimits &limits = HtmlLimits());
// The refusal in words for the person (a page that is refused as a file).
QString htmlRefusalText(HtmlRefusal why);
// The words of a pasted page without its markup, found without the page's parser (for a paste that was refused and has no plain text of its own).
QString htmlPlainText(const QString &html);
// For a text box's own paste (which parses the page itself): the data to hand it in its place. Null when the data has no page. Else a copy that holds the guarded page, or, when the page is
// refused, the plain text only (`refused`, when given, tells which).
std::unique_ptr<QMimeData> guardedMimeData(const QMimeData *source, bool *refused = nullptr);
// The note a refused paste tells: the plain text was pasted in the page's place (`pastedText`), or nothing could be pasted.
QString htmlPasteRefusedNote(bool pastedText = true);

} // namespace jp::office::wml
