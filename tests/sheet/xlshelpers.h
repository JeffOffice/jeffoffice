// Workbooks in the old binary format (BIFF8 and BIFF5), built in memory for the tests of the .xls reader: records, formula tokens, and a builder that puts them in a
// workbook stream and in a compound file. Nothing here reads a file made by another program.
#pragma once

#include "sheethelpers.h"

namespace xlstest {

// Little-endian bytes, chained.
class W {
public:
    W() = default;
    W(const QByteArray &b) : m_b(b) {}
    W &u8(int v) { m_b.append(char(v)); return *this; }
    W &u16(int v) { m_b.append(char(v & 0xFF)); m_b.append(char((v >> 8) & 0xFF)); return *this; }
    W &u32(quint32 v) { return u16(int(v & 0xFFFF)).u16(int(v >> 16)); }
    W &f64(double v);
    W &raw(const QByteArray &b) { m_b.append(b); return *this; }
    // A string of the form BIFF8 uses, with a count of `countBytes` (1 or 2) bytes, then the byte of flags, then the characters: one byte each when `compressed`, else two.
    W &str8(const QString &s, int countBytes = 1, bool compressed = true);
    // A string of BIFF5: the count, then one byte for each character (Latin-1 here).
    W &str5(const QString &s, int countBytes = 1);
    QByteArray bytes() const { return m_b; }
    operator QByteArray() const { return m_b; }   // NOLINT

private:
    QByteArray m_b;
};

// A record: its number, its length, and the bytes.
QByteArray rec(quint16 id, const QByteArray &payload = QByteArray());

// ---- formula tokens (the bytes of a formula, one token after another) ----
QByteArray tInt(int v);
QByteArray tNum(double v);
QByteArray tStr(const QString &s, int biff = 8);
QByteArray tBool(bool v);
QByteArray tErr(int code);
QByteArray tMissing();
QByteArray tOp(int ptg);   // an operator: 0x03 (+) to 0x14 (%), 0x15 (parentheses)
// A cell: row and column from 0; `rowRel`/`colRel` say the part moves when the formula is copied (no "$" in front of it).
QByteArray tRef(int row, int col, bool rowRel = true, bool colRel = true, int biff = 8, int cls = 0x20);
QByteArray tArea(int row1, int col1, int row2, int col2, bool rowRel = true, bool colRel = true, int biff = 8, int cls = 0x20);
// A cell or an area on a sheet named by an entry of the external-sheet list (BIFF8).
QByteArray tRef3d(int ixti, int row, int col, bool rowRel = true, bool colRel = true);
QByteArray tArea3d(int ixti, int row1, int col1, int row2, int col2, bool rowRel = true, bool colRel = true);
// A cell or an area as the distance from the cell the formula is written for (a shared formula).
QByteArray tRefN(int dRow, int dCol, bool rowRel = true, bool colRel = true);
QByteArray tAreaN(int dRow1, int dCol1, int dRow2, int dCol2, bool rowRel = true, bool colRel = true);
QByteArray tFunc(int index, int cls = 0x40);                    // a function with the arguments the format fixes for it
QByteArray tFuncVar(int index, int argc, int cls = 0x40);       // a function with a count
QByteArray tName(int index, int biff = 8, int cls = 0x20);      // a defined name (counted from 1)
QByteArray tNameX(int ixti, int index);                         // a name of another book or an add-in function
// BIFF5: a cell on a sheet named by a record of the external-sheet list of its sheet (counted from 1; below 0 is a sheet of this workbook, and then the first and last sheet
// are given), and a name from the list of external names.
QByteArray tRef3d5(int iw, int firstSheet, int lastSheet, int row, int col, bool rowRel = true, bool colRel = true);
QByteArray tNameX5(int iw, int index);
QByteArray tAttrSum();
QByteArray tAttrIf(int jump);
QByteArray tAttrGoto(int jump);
QByteArray tAttrSpace(int kind = 0, int count = 1);
QByteArray tAttrChoose(const QList<int> &offsets);
QByteArray tMemFunc(int cce);
QByteArray tArray();   // the token only: the values follow the formula, from arrayValues
// The values of an array constant, as they follow the tokens of a formula in BIFF8.
QByteArray arrayValues(const QList<QVariantList> &rows, int biff = 8);

// ---- cell records ----
QByteArray number(int row, int col, double v, int xf = 15);
QByteArray rk(int row, int col, quint32 rkValue, int xf = 15);
QByteArray mulrk(int row, int colFirst, const QList<quint32> &values, int xf = 15);
QByteArray labelSst(int row, int col, int index, int xf = 15);
QByteArray label(int row, int col, const QString &s, int xf = 15, int biff = 8);
QByteArray blank(int row, int col, int xf = 15);
QByteArray mulblank(int row, int colFirst, int count, int xf = 15);
QByteArray boolErr(int row, int col, int value, bool isError, int xf = 15);
// A formula cell with a number as its stored result.
QByteArray formulaNumber(int row, int col, const QByteArray &rgce, double result, int xf = 15, const QByteArray &extra = QByteArray(), int flags = 0);
// ... with a result that is not a number: kind 0 text (a string record follows), 1 true or false, 2 an error, 3 the empty text.
QByteArray formulaSpecial(int row, int col, const QByteArray &rgce, int kind, int value = 0, int xf = 15, const QByteArray &extra = QByteArray());
QByteArray stringRec(const QString &s, int biff = 8);
// The record of the cells a shared formula covers, with its tokens; `extra` follows the tokens.
QByteArray shrfmla(int row1, int row2, int col1, int col2, const QByteArray &rgce, const QByteArray &extra = QByteArray());
QByteArray arrayRec(int row1, int row2, int col1, int col2, const QByteArray &rgce, const QByteArray &extra = QByteArray());
QByteArray tableRec(int row1, int row2, int col1, int col2, int flags, int rowIn, int colIn, int rowIn2 = 0, int colIn2 = 0);
// A formula cell that stands for part of a shared formula, an array, or a data table: its token holds the cell that has the record.
QByteArray formulaExp(int row, int col, int expRow, int expCol, double result, int xf = 15, bool table = false);

// ---- sheet records ----
QByteArray dimensions(int rows, int cols, int biff = 8);
QByteArray colinfo(int first, int last, int width256, int xf = 0, int flags = 0);
QByteArray rowRec(int row, int heightTwips, int flags = 0, int xf = 0);
QByteArray mergedCells(const QList<QList<int>> &ranges);   // each {row1, row2, col1, col2}
QByteArray window2(int flags, int top = 0, int left = 0, int scale = 0);
QByteArray pane(int x, int y, int top, int left, int active = 3);
QByteArray scl(int num, int denom);
QByteArray selection(int pane, int row, int col);
QByteArray defColWidth(int w);
QByteArray defaultRowHeight(int flags, int twips);

// ---- globals records ----
// A font: the height in points, the weight, whether it is italic, struck out, the underline, the color index, and the name.
QByteArray font(const QString &name, double points = 10, int weight = 400, bool italic = false, bool strike = false, int underline = 0, int color = 0x7FFF, int escapement = 0,
                int biff = 8);
// A cell format (BIFF8): the font and number format, the alignment, the borders (four styles, in the order left, right, top, bottom), and the fill.
struct XfSpec {
    int font = 0, format = 0;
    int halign = 0, valign = 2;
    bool wrap = false, shrink = false;
    int rotation = 0, indent = 0;
    int borders[4] = {0, 0, 0, 0};
    int borderColors[4] = {64, 64, 64, 64};
    int pattern = 0, fore = 64, back = 65;
    bool style = false;
    int parent = 0;          // the style format a cell format follows (0xFFF for a style format: none)
    bool locked = true;
};
QByteArray xf(const XfSpec &s = XfSpec());
// The same cell format as BIFF5 writes it (16 bytes).
QByteArray xf5(const XfSpec &s = XfSpec());
QByteArray format(int id, const QString &code, int biff = 8);
// A named style: a built-in one (by its number) or one with a name, and the cell format that holds its look.
QByteArray styleBuiltin(int xf, int builtin, int level = 0xFF);
QByteArray styleNamed(int xf, const QString &name, int biff = 8);
QByteArray protect(bool on);
QByteArray palette(const QList<quint32> &colors);   // 0xRRGGBB, from the place 8
QByteArray boundSheet(quint32 offset, const QString &name, int state = 0, int type = 0, int biff = 8);
// The strings of a workbook as a shared string table; `splitAt`, if above 0, ends the first record after that many bytes of the table (the rest follows in CONTINUE
// records, each at most `splitAt` more bytes long), so that a string is cut where the test wants it.
QByteArray sst(const QStringList &strings, int splitAt = 0, const QList<int> &richRuns = QList<int>());
QByteArray supbookInternal(int sheets);
QByteArray supbookExternal(const QString &url, const QStringList &sheets);
QByteArray supbookAddIn();
QByteArray externName(const QString &name);
QByteArray externSheet(const QList<QList<int>> &xti);   // each {supbook, first sheet, last sheet}
QByteArray externSheet5(const QByteArray &encodedName);
QByteArray externCount5(int n);
QByteArray externName5(const QString &name);
// A defined name (BIFF8): `builtin` is the number of a built-in name; `sheet` is 0 for the workbook and the sheet's number from 1 for a name of one sheet.
QByteArray name(const QString &text, const QByteArray &rgce, int sheet = 0, int builtin = -1, bool hidden = false, const QByteArray &extra = QByteArray(), int biff = 8);

// A workbook: the globals the test adds, and the sheets' records. stream() puts them together in the order the format has: the start of the workbook, a code page, the
// fonts and formats a workbook has by default (five fonts, sixteen cell formats), the test's globals, a record for each sheet, the end; then each sheet's records.
class Book {
public:
    explicit Book(int biff = 8) : m_biff(biff) {}
    int biff() const { return m_biff; }
    Book &globals(const QByteArray &records) { m_globals += records; return *this; }
    // Records that follow the sheets' own records in the globals (the string table, the external-sheet list, the names).
    Book &after(const QByteArray &records) { m_after += records; return *this; }
    Book &noDefaultFormats() { m_defaults = false; return *this; }
    Book &codepage(int cp) { m_codepage = cp; return *this; }
    Book &sheet(const QString &name, const QByteArray &records, int state = 0, int type = 0);
    QByteArray stream() const;
    // The whole file: a compound file holding the stream (and the named extra storages, if any).
    QByteArray file(const QStringList &extraStreams = QStringList()) const;

private:
    struct Sheet {
        QString name;
        QByteArray records;
        int state = 0, type = 0;
    };
    int m_biff;
    bool m_defaults = true;
    int m_codepage = -1;
    QByteArray m_globals, m_after;
    QList<Sheet> m_sheets;
};

// A workbook read back through the reader. Opens `file` (or builds it from the book) and fails the test with the reader's message if it can't be opened.
bool openBook(const Book &book, Workbook *out, QString *why = nullptr, const Limits &limits = Limits());
// The cell's value as the tests of the XML reader show it (see sheettest::show), for a sheet read from an old workbook.
QString shown(const Workbook &book, const Sheet &sheet, const QString &ref);

} // namespace xlstest

using namespace xlstest;
