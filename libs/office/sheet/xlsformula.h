#pragma once
// A formula as the old binary workbook format stores it, a string of tokens in postfix order (the operands and operators, the references with their sheets, the
// functions by number, the layout hints that only help the original program evaluate), turned back into the text the rest of the spreadsheet code reads. Nothing
// here recurses: the tokens are walked once with a stack of operand texts, so a formula of any shape costs time in proportion to its tokens, and every length is checked
// against the bytes there are.

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

#include <functional>

namespace jp::office::sheet {

// One entry of the list that formulas name sheets by (BIFF8): the supporting book and the first and last sheet of the span.
struct XlsXti {
    int supbook = 0;
    int first = 0, last = 0;   // sheet places, or -1 (deleted) and -2 (the workbook as a whole)
};

// A supporting book: this workbook, another one, or the add-in functions.
struct XlsSupBook {
    enum class Kind : quint8 { Internal, External, AddIn };
    Kind kind = Kind::Internal;
    QString url;
    QStringList sheets;
    QStringList names;       // the external names it holds, in order
    int ordinal = 0;         // external books are numbered from 1 in the order they come: [1], [2]
};

// A defined name as formulas write it.
struct XlsNameInfo {
    QString text;
    int localSheet = -1;     // place of the sheet it belongs to, -1 for the whole workbook
};

// What a formula needs of the workbook around it.
struct XlsContext {
    int biff = 8;                                 // 8 for BIFF8, 5 for BIFF5
    QStringList sheetNames;                       // this workbook's sheets, in order
    QVector<XlsXti> xti;                          // BIFF8
    QVector<XlsSupBook> supbooks;
    QVector<QByteArray> externSheet5;             // BIFF5: the encoded names of the external-sheet records
    QStringList externNames5;                     // BIFF5: the external names (add-in and macro functions, other books' names), in the order of their records
    QVector<XlsNameInfo> names;
    std::function<QString(const char *bytes, int count)> bytesToText;   // BIFF5 text, in the workbook's code page
};

struct XlsFormulaOptions {
    quint32 baseRow = 0, baseCol = 0;   // the cell a formula with relative references is written for
    bool relative = false;              // a reference that is relative gives its distance from the base (a shared formula, a name), not its place
    bool name = false;                  // the formula is a defined name's: its references are written for cell A1 and come round the edge of the grid of the XML form (not of the old one)
    int sheet = -1;                     // the sheet the formula is in: a name of another sheet is written with its sheet
    qint64 maxChars = 100000;
};

struct XlsFormulaResult {
    QString text;              // no "=" in front
    bool outside = false;      // a relative reference came out of the sheet (and wrapped round, as the format does): the text is for this base cell only
    QString why;               // when the formula could not be read
    bool ok() const { return why.isEmpty(); }
};

// `rgce` is the formula's tokens and `extra` the bytes that follow them in the record (the values of array constants, the cells that memory tokens remember).
XlsFormulaResult xlsFormulaText(const XlsContext &context, const QByteArray &rgce, const QByteArray &extra, const XlsFormulaOptions &options);

// The error a code of the format stands for ("#N/A" for 0x2A), or an empty string for a code that is none.
QString xlsErrorText(int code);

// A sheet name with the quotes it needs where it is written before "!".
QString xlsSheetPrefix(const QString &name);

} // namespace jp::office::sheet
