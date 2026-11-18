#pragma once
// The tree the two plain-text equation syntaxes (UnicodeMath in wmlmathuni.cpp, LaTeX in wmlmathlatex.cpp) are read into and printed from, with the one writer of math markup
// (m:oMath) from it and the one reader of math markup into it. Nothing here is for callers outside libs/office/wml: wmlmathedit.h has the functions programs use.

#include <QByteArray>
#include <QString>

#include <vector>

namespace jp::office::wml {
class XmlTree;
}

namespace jp::office::wml::mathtree {

// Limits for everything a parser, the reader, or the writer handles (the depth and the length of a typed equation are kMostEquationDepth and kMostEquationInput of wmlmathedit.h).
constexpr int kMostMatrixCells = 1024;    // cells of one matrix
constexpr int kMostMatrixSide = 64;       // rows, and columns
constexpr int kMostCells = 16384;         // cells of all the matrices of one equation read (empty ones too)
constexpr int kMostNodes = 4000;          // structures, runs, and operands of one equation, read or written
constexpr qint64 kMostMarkupBytes = qint64(1) << 20;   // the markup made from a typed equation

enum class Kind { Run, Frac, Script, Rad, Nary, Delim, Accent, Bar, Matrix, Func, LimLow, Group };

struct Node;
using Seq = std::vector<Node>;

// One piece of an equation. `part` holds its operands by role:
//  Run: text; style 0 (math italic for letters), 'p' (upright), 'b' (bold), 'n' (Normal Text, upright in the text font).
//  Frac: part 0 numerator, 1 denominator.   Script: 0 base, 1 subscript (if hasA), 2 superscript (if hasB).
//  Rad: 0 degree (if hasA), 1 radicand.   Nary: text the operator, 0 lower limit (hasA), 1 upper limit (hasB), 2 operand; limits 0 (its usual place), 1 (over and under), 2 (beside).
//  Delim: text the opening character, text2 the closing, text3 the separator; each part an argument. `group` marks brackets that came from typing a pair (a whole operand of a
//  fraction, script, or root is then shown without them).   Accent: text the combining character; 0 operand.   Bar: hasA over (else under); 0 operand.
//  Matrix: cols; the cells row by row.   Func: 0 name, 1 argument.   LimLow: 0 base, 1 limit.   Group: parts[0], brackets that are not shown (only while a parser works).
struct Node {
    Kind kind = Kind::Run;
    QString text, text2, text3;
    char style = 0;
    bool hasA = false, hasB = false, group = false, operand = false;
    int cols = 0, limits = 0;
    std::vector<Seq> part;
};

Node run(const QString &text, char style = 0, bool operand = false);
Node structure(Kind kind, std::vector<Seq> parts);

// The markup (an m:oMath element declaring its namespace) of the items, or an empty array when it is nested deeper than kMostEquationDepth, has more than kMostNodes pieces, or is
// larger than kMostMarkupBytes.
QByteArray write(const Seq &items);
// The items of the markup (an m:oMath, or an m:oMathPara holding one), or false for markup with anything in it a typed line can't say (a group of characters, an equation array, a
// phantom, a property that is not understood) or more than the limits above hold.
bool read(const QByteArray &markup, Seq *items);

// Names that both syntaxes share.
bool isFunctionName(const QString &name);     // sin, cos, lim, ...
bool isLimitFunction(const QString &name);    // lim, max, min, sup, inf, limsup, liminf: a subscript goes under the name
QString symbolByName(const QString &name);    // \alpha and the rest: the character, or an empty string
QString symbolName(uint ch);                  // the name of a character, or an empty string
QString accentName(ushort mark);              // hat, bar, vec, ...: the name of an accent's combining character, or an empty string
ushort accentMark(const QString &name);       // ... and the other way round, 0 for a name that is none
struct NaryOp {
    const char *latex;      // the name (without the backslash)
    ushort chr;
    bool beside;            // its limits go beside it by default (an integral)
};
const NaryOp *naryByChar(const QString &chr);
const NaryOp *naryByName(const QString &name);
bool naryBeside(const Node &n);   // where the limits of this operator go: beside it (true) or over and under

// The text without the blanks at its ends, except one that a backslash before it makes a character of its own.
QString trimmedText(const QString &text);

// An empty cell, an empty operand: the run that holds the place.
QByteArray emptyRun();

} // namespace jp::office::wml::mathtree
