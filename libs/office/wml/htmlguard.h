#pragma once
// A look at a web page before the page's parser (QTextDocument::setHtml, QTextDocumentFragment::fromHtml, or a text box's own paste) is let near it. The parser's time is not about a page's size:
// it grows with how deep the parts stand inside one another (each part copies from its parent and each closing tag walks up), with the items of one list (each item is put in after all the
// earlier ones), with the pictures in one paragraph, and with the grid a table asks for. So the page is read once, in time that grows with its size, with a model of that work in nanoseconds
// of this machine (kMostHtmlWork is about two seconds of it), and a page that goes past the model, or past a limit of depth, tables, cells, rows, or bytes, is refused before the parser sees it.
// Every caller that hands outside text (a file, the clipboard, a drop, a field) to the parser uses htmlRefusal() first.

#include <QByteArray>
#include <QString>

#include <memory>

class QMimeData;

namespace jp::office::wml {

constexpr qint64 kMostHtmlBytes = 32 * 1024 * 1024;
// Real pages nest up to about 40 parts deep (tables up to about 10) and keep fewer than 50,000 cells in a table; each limit is ten times or more of that.
constexpr int kMostPageDepth = 600;          // parts (elements, of any name) standing inside one another
constexpr int kMostHtmlTableDepth = 100;     // tables standing inside one another
constexpr int kMostHtmlCells = 200000;       // cells (td and th) in the whole page
constexpr int kMostHtmlRows = 100000;        // rows (tr) in the whole page
// The work the parser would do, as the model counts it (nanoseconds of this machine; measured, see htmlguard.cpp). About two seconds (the parser took at most 1.2 times the model's count on any shape tried). Ordinary pages stay far under it: a long article
// (3,000 paragraphs) is 7 percent, a 20-level outline 4 percent, a page saved from a browser (3,000 blocks, 40 deep) 10 percent, and a table of 56,000 rows 48 percent.
constexpr qint64 kMostHtmlWork = 1800000000;

enum class HtmlRefusal { None, TooLarge, TooDeep, TooManyCells, TooMuchWork };

// The limits of a scan (the named constants above; the tests and the timing by hand turn one down or up).
struct HtmlLimits {
    qint64 bytes = kMostHtmlBytes;
    qint64 work = kMostHtmlWork;
    int depth = kMostPageDepth;
    int tableDepth = kMostHtmlTableDepth;
    int cells = kMostHtmlCells;
    int rows = kMostHtmlRows;
};

struct HtmlScan {
    HtmlRefusal refusal = HtmlRefusal::None;
    qint64 work = 0;        // the model's count of the parser's work
    qint64 walked = 0;      // stack positions looked at to close parts (the scan's own work, bounded by `work`)
    int deepest = 0;        // the most parts standing inside one another that was seen
};

// The scan itself (the tests count what it did).
HtmlScan scanHtml(const QByteArray &html, const HtmlLimits &limits = HtmlLimits());
HtmlScan scanHtml(const QString &html, const HtmlLimits &limits = HtmlLimits());
// Whether the page may be given to the parser (None), or why not. The one guard of every caller.
HtmlRefusal htmlRefusal(const QByteArray &html);
HtmlRefusal htmlRefusal(const QString &html, qint64 mostBytes = kMostHtmlBytes);
// The refusal in words for the person (a page that is refused as a file).
QString htmlRefusalText(HtmlRefusal why);
// The words of a pasted page without its markup, found without the page's parser (for a paste that was refused and has no plain text of its own).
QString htmlPlainText(const QString &html);
// For a text box's own paste (which parses the page itself): null when the data has no page, or the page may be parsed; else the data to hand it in its place, which holds the plain text only.
std::unique_ptr<QMimeData> plainTextInsteadOfRefusedHtml(const QMimeData *source);
// The note a refused paste tells: the plain text was pasted in the page's place (`pastedText`), or nothing could be pasted.
QString htmlPasteRefusedNote(bool pastedText = true);

} // namespace jp::office::wml
