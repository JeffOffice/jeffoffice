#pragma once
// A formula as a file stores it (no "=" in front), parsed to a Program: a short list of instructions for a stack machine (postfix, so a chain like A1+A2+...
// of any length needs no recursion to run), with the numbers, texts, references, and array constants they use beside it. No GUI.
//
// The language: numbers, "texts" (a quote doubled stands for itself), TRUE and FALSE, errors (#DIV/0! #N/A #NAME? #NULL! #NUM! #REF! #VALUE!), references in A1 form
// ($A$1, A1:C3, whole columns A:A and rows 1:1, on another sheet Sheet2!A1 or 'My Sheet'!A1, through several sheets Sheet1:Sheet3!A1, in another workbook [1]Sheet1!A1),
// defined names (of the workbook, or of a sheet: Sheet1!Name), references in R1C1 form when asked (R[-1]C, R2C3:R4C5), references to a table's columns (Table1[Col],
// parsed and kept, but not worked out), array constants {1,2;3,4}, and function calls (a prefix _xlfn. or _xlws. that newer files write is not part of the name),
// with arguments left out (IF(A1,,2)). The operators, from the tightest: the reference operators ":" (range), a space (intersection), and "," (union, inside
// parentheses); a minus sign in front; "%"; "^"; "*" and "/"; "+" and "-"; "&"; and the comparisons "=" "<>" "<" "<=" ">" ">=". Every binary operator groups from the
// left, so 2^3^2 is 64 and -2^2 is 4, as in the files that depend on it. "@" in front of a reference picks one value of it.
//
// Everything a file controls is bounded: the text's length, how deep parentheses, calls, and arrays nest, the arguments of a call, and the cells of an array constant.
// A formula past a bound is not parsed.

#include "calc/parselimits.h"
#include "calc/value.h"

#include <QString>
#include <QStringList>

#include <memory>
#include <utility>
#include <vector>

namespace jp::calc {

// Characters of a formula (the format allows 8,192; 943 found), depth of nesting (the format allows 64), arguments of a call (255), cells of an array constant, instructions of a program.
constexpr int kMaxFormulaChars = 32768;
constexpr int kMaxFormulaDepth = 128;
constexpr int kMaxFormulaArgs = 1000;
constexpr int kMaxArrayConstantCells = 100000;
constexpr int kMaxFormulaInstructions = 65536;

enum class Op : quint8 {
    PushNumber,    // operand: place in numbers
    PushText,      // operand: place in texts
    PushBool,      // operand: 0 or 1
    PushError,     // operand: an ErrorKind
    PushMissing,   // an argument left out
    PushRef,       // operand: place in refs
    PushName,      // operand: place in texts (the name); argc: place in sheetNames + 1 of the sheet it is written with, 0 for none
    PushArray,     // operand: place in arrays
    PushTable,     // a reference to a table's columns: operand: place in texts (as written). Not worked out.
    PushExternal,  // a reference to another workbook: operand: place in texts (as written). Not worked out.
    Add, Sub, Mul, Div, Pow, Concat, Eq, Ne, Lt, Le, Gt, Ge,
    Range, Union, Intersect,
    Neg, Percent, Single, Spill,
    Call,          // operand: the function's place in the registry, or for one that is not known a negative number: its name is texts[-operand - 1]; argc: the arguments
    Jump,          // operand: where to go
    IfJump,        // takes the condition; goes to operand if it is false, and to argc if it is an error (both counted from the next instruction)
    TryJump,       // IFERROR (flags 0) and IFNA (flags 1): keeps the value and goes to operand unless it is an error (for IFNA, #N/A)
    Choose,        // takes the index; goes to the operand's argc'th Jump that follows, or gives #VALUE! and goes to operand
};

// Instr::flags
constexpr quint8 kInstrArray = 1;   // worked out in the manner of an array formula: a range is taken as an array, and a function of one value is made for each element
constexpr quint8 kInstrFinal = 2;   // the last operation of the formula, and not in parentheses: a sum or difference within rounding error of zero is zero

struct Instr {
    Op op = Op::PushMissing;
    quint8 flags = 0;
    quint16 argc = 0;
    qint32 operand = 0;
};

// RefLit::flags
constexpr quint8 kRefRow1Abs = 1, kRefCol1Abs = 2, kRefRow2Abs = 4, kRefCol2Abs = 8;
constexpr quint8 kRefWholeCols = 16;   // "A:C": every row
constexpr quint8 kRefWholeRows = 32;   // "1:3": every column
constexpr quint8 kRefArea = 64;        // written as an area, even if it is one cell ("A1:A1")

// A reference as written, for the cell the text was written for. A part without a "$" moves with the cell (a shared formula, or a copy).
struct RefLit {
    qint32 sheet1 = -1, sheet2 = -1;   // places in Program::sheetNames of the sheet it names, and of the last sheet of a 3-D reference; -1: the formula's own sheet, and none
    quint32 row1 = 0, col1 = 0, row2 = 0, col2 = 0;   // 0-based; row2 and col2 equal row1 and col1 for one cell
    quint8 flags = 0;
    quint8 written = 0;   // bits that say the corners were written back to front (1: rows, 2: columns) and put in order: the text can be given as it was written (see sheet/formulaframe.h)
};

struct Program {
    std::vector<Instr> code;
    std::vector<double> numbers;
    std::vector<QString> texts;
    std::vector<RefLit> refs;
    QStringList sheetNames;
    std::vector<Value> arrays;
    // What the formula uses.
    bool isVolatile = false;           // calls NOW, TODAY, RAND, RANDBETWEEN, OFFSET, INDIRECT: its value changes with no cell changing
    bool external = false;             // refers to another workbook
    bool structured = false;           // refers to the columns of a table, or to a spilled range (A1#)
    bool arrayFormula = false;         // parsed to be worked out as an array
    QStringList unknownFunctions;      // names (upper case, once each) of functions that are not in the registry
    std::vector<std::pair<int, int>> calls;   // the functions called (registry places) and how many times

    // Everything the engine can work out: no function it lacks, no other workbook, no table column.
    bool supported() const { return unknownFunctions.isEmpty() && !external && !structured; }
    // The program as one line of postfix text, for tests and tools: "1 2 + 3 *", with texts quoted, references as A1, a call as SUM/2.
    QString dump() const;

    // Which sheets the names above are, for the source the program last ran with (see Evaluator).
    mutable const void *boundTo = nullptr;
    mutable qint64 boundGeneration = -1;
    mutable std::vector<qint32> boundSheets;
};

struct ParseOptions {
    bool r1c1 = false;          // references are in R1C1 form
    bool arrayFormula = false;  // the formula is an array formula: worked out as an array
    bool definedName = false;   // the text of a defined name: a union ("A1,B2") may stand at the top level
    quint32 row = 0, col = 0;   // the cell the text is written for (r1c1: what "R[1]C[1]" is relative to)
    int maxArgs = kMaxFormulaArgs;   // the most arguments a call may have (a formula that is typed: 255, the most a file may have)
    ParseLimits limits;              // how far the rows and columns of its references reach (A1 form only)
};

struct ParseResult {
    std::shared_ptr<const Program> program;   // null if the text isn't a formula
    QString error;                             // what is wrong with it
    int errorPos = 0;                          // and where (a character of the text)
    bool ok() const { return bool(program); }
};

ParseResult parseFormula(const QString &text, const ParseOptions &options = ParseOptions());

// A reference of a formula and where it is in the text, for rewriting the text (a row inserted, a cell moved) with everything else in it left as it was.
struct ReferenceAt {
    int start = 0;            // the first character of what the text has for it (a sheet's name and "!" before the cells, if it has them)
    int body = 0;             // where the cells begin (after the "!")
    int end = 0;              // after its last character
    RefLit ref;               // the cells as written (sheet1 and sheet2 are not set: the names are)
    QString sheet1, sheet2;   // the sheet it names, and the last sheet of a 3-D reference (empty: none written)
    bool afterWorkbook = false;   // a workbook's name in square brackets comes right before it ("[Book2.xlsx]Data!A1"): the cells are in another workbook, whatever the sheet is called
    bool isName = false;      // not cells but a name that belongs to a sheet ("Data!MyName"): only with `withNames`; `ref` is not set
};
// The references of a formula in the order of the text; false if it can't be read. With `withNames`, a name written with its sheet is one too (so that a sheet renamed or deleted can be
// followed in it).
bool findReferences(const QString &text, const ParseOptions &options, std::vector<ReferenceAt> *out, bool withNames = false);
// The cells of a reference as a formula writes them: "$A$1", "B2:C9", "A:C", "3:5".
QString referenceCellsText(const RefLit &ref);

// The letters of a column counted from 0: "A", "Z", "AA", "XFD".
QString columnLetters(quint32 col);

} // namespace jp::calc
