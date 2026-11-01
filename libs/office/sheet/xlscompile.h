#pragma once
// A formula as the model keeps it ("SUM(A1:B2,Sheet2!C3)", no "=" in front) turned into the tokens the old binary workbook format stores (BIFF8): the operands and operators in
// postfix order, the references with their sheets named by an entry of the workbook's list of external sheets, the functions by number. The text is read once into a tree of a
// bounded size (a depth and a count of pieces that no real formula comes near), and the tree is written once; nothing recurses without the bound.
//
// A formula the old format cannot hold (a function it does not have, a reference past its 256 columns or 65,536 rows, another workbook's cell, a table's column, a string of more than
// 255 characters, or a call with more than 30 arguments) is refused with the reason, and the caller keeps the cell's value.

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QStringList>

#include <functional>

namespace jp::office::sheet {

// What a formula needs of the workbook it is written in.
struct XlsCompileContext {
    QStringList sheetNames;                           // the sheets, in the order they are written
    std::function<int(int first, int last)> xti;      // the place in the list of external sheets of the span of sheets (first to last, from 0), made if it is new
    QHash<QString, int> names;                        // the defined names, by "<sheet place or -1>|<name in lower case>": the place from 1 in the list of names
    int sheet = -1;                                   // the sheet the formula is in (-1 for a name's formula): a bare name is looked up among this sheet's names first
    bool array = false;                               // the formula is an array formula's: its operands are of the array kind
    bool isName = false;                              // the formula is a defined name's: its operands are references
};

// The tokens in `rgce` and the bytes of the array constants that follow them in `extra`. False, with `why`, if the formula cannot be written.
bool xlsCompileFormula(const QString &text, const XlsCompileContext &context, QByteArray *rgce, QByteArray *extra, QString *why);

// The number of the function of that name in the old format (the name in capitals), or -1.
int xlsFunctionNumber(const QString &name);

constexpr int kXlsMaxRows = 65536;
constexpr int kXlsMaxCols = 256;
constexpr int kXlsMaxFormulaBytes = 8000;

} // namespace jp::office::sheet
