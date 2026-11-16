#pragma once
// What Copy and Cut put on the clipboard, and what Paste reads from it. Besides the tab-separated text and the HTML table other programs take, the clipboard holds the cells whole (their
// values, formulas as they were written for the cell they came from, formats, and merged ranges) in a form of this program's own, so that a paste in the workbook they were copied from
// puts back everything, and one in another workbook what both can hold. Text and HTML from other programs are read into a table of texts, which are entered as if typed.

#include "sheet/calculation.h"
#include "sheet/csvimport.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>

#include <vector>

namespace jp::crunch {

constexpr const char *kClipMime = "application/x-jeffcrunch-cells";
// Most cells made from what another program puts on the clipboard as text or an HTML table (a span of columns counts for all its cells): what is past it is not read, and the paste says
// that it is too much. A paste is at most a tenth of this.
constexpr qint64 kMaxClipCells = 1000000;

struct ClipCell {
    quint32 dRow = 0, dCol = 0;                          // from the block's first cell
    jp::office::sheet::Calculation::CellContent content; // formulas as written for the cell they were copied from
    QString formatCode;                                  // the number format's code ("General" for none): a workbook that does not have the formats takes this
    jp::calc::Value shown;                               // what the cell showed when it was copied: a formula's result
};

struct ClipNote {
    quint32 dRow = 0, dCol = 0;
    QString text, author;
    bool shown = false;
};

// A thread of comments that was copied: its comments in order, each naming the one it replies to by place (so that a paste makes new ids).
struct ClipThreadComment {
    QString author, userId, providerId, personId, time, text;
    bool done = false;
    qint32 parent = -1;   // the place in the thread of the comment this one replies to, or -1
};
struct ClipThread {
    quint32 dRow = 0, dCol = 0;
    bool shown = false;
    std::vector<ClipThreadComment> comments;
};

struct ClipData {
    qint64 token = 0;                                    // the workbook it was copied in (0 for none)
    qint64 serial = 0;                                   // and which copy it was there (a cut is pasted as a move only while it is the last copy)
    int sheet = 0;
    jp::office::sheet::CellRange source;                 // where the block was, as selected
    quint32 rows = 0, cols = 0;                          // the block that is copied: the selection, less the rows and columns past the last cell the sheet has
    bool cut = false;
    std::vector<ClipCell> cells;                         // those that exist
    std::vector<jp::office::sheet::CellRange> merges;    // inside the block, from its first cell
    std::vector<ClipThread> threads;                     // the threads of comments on the cells (a paste of everything makes each again, with ids of its own)
    std::vector<ClipNote> notes;                         // the notes of the cells (not those that are a conversation of comments)
    std::vector<double> columnWidths;                    // the width of each column of the block, in the sheet's units; below 0 for the sheet's usual
    // A copy leaves out the rows that are hidden: the row of the block at `dRow` was this many rows below the first of `source` (empty: every row is there).
    std::vector<quint32> rowOffsets;
    quint32 sourceRow(quint32 dRow) const { return source.row1 + (dRow < rowOffsets.size() ? rowOffsets[dRow] : dRow); }
    bool valid() const { return rows > 0 && cols > 0; }
    QByteArray toBytes() const;
    static bool fromBytes(const QByteArray &bytes, ClipData *out);
};

// Text a program puts on the clipboard for a block of cells: rows on lines, cells apart by tabs, a cell with a tab, a line break, or a quote in it in quotes (a quote doubled); and an
// HTML table. The table's cells as read back are the rows of texts.
QVector<QStringList> parseTabSeparated(const QString &text);
QVector<QStringList> parseHtmlTable(const QString &html);

// Text split into fields the way a person chose in the Text Import dialog: by any of the separators (several in a row as one if asked), a field in the quote kept whole (with separators and
// line breaks in it; the quote written twice is one), the rows before `firstRow` left out, a column read as text kept as the text it is (an apostrophe in front, which an entry reads as
// text), a column read as a date turned to year-month-day when it is one in the order given, and a column to skip left out. At most kMaxClipCells fields are made.
QVector<QStringList> splitDelimitedText(const QString &text, const jp::office::sheet::CsvImport &choices);

// Whether text that came from another program is clearly delimited text, and how: no tab in it (tab-separated text goes straight into cells), at least two lines, and either the
// same number of fields on every line by commas, semicolons, or bars (a semicolon or bar wins over a comma that is there too; quotes are kept whole; a semicolon file whose
// numbers have commas in them reads them as decimal marks), or, from three lines, columns of fixed width (the same gaps of spaces in every line, at least two of them). Only the first
// kGuessBytes of the text and kGuessLines lines are looked at. `strict` (a paste or a drop: it asks without being asked) wants three or more fields on every line, no column empty on every
// line, and no numbers with thousands marks; not strict (Paste Text Import, which was asked for) takes two fields. The choices come back set to what was found (the rest left as they are); false when it is not clear.
constexpr qsizetype kGuessBytes = 65536;
constexpr int kGuessLines = 100;
constexpr qint64 kMostPastedChars = qint64(32) << 20;   // the most characters of pasted or dropped text that are split (kMaxClipCells fields need no more)
bool guessDelimitedText(const QString &text, jp::office::sheet::CsvImport *choices, bool strict = true);

// How a text file of values is read when the command line says (--separator, --encoding, --decimal; each may be empty for "as the file is found to be"). The separator is comma,
// semicolon, tab, space, or bar, or one character; the encoding is utf-8, utf-8-bom, utf-16le, utf-16be, latin1, mac-roman, windows-NNNN (a code page such as 1250), or iso-8859-N;
// the decimal mark is a point, a comma, or the word point or comma. False, with `error` saying which value is not understood, otherwise.
bool textImportFromOptions(const QString &separator, const QString &encoding, const QString &decimal, jp::office::sheet::CsvImport *out, QString *error);

} // namespace jp::crunch
