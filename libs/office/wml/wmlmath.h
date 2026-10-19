#pragma once
// Equations (the math markup of ECMA-376 part 1, chapter 22) as the pages show them: a line of text on the baseline, in the run's size. The markup itself is kept as written;
// this is only what is drawn in its place, with every character of the equation in order.

#include "wml/xmltree.h"

#include <QString>

namespace jp::office::wml {

struct Equation {
    QString text;     // the line (equations that are lines of their own in an m:oMathPara are joined by U+2028)
    QString style;    // how most of its letters are set: "i" (italic), "b" (bold), "ib" (both), or empty
    int runProps = -1;   // the first w:rPr of a run of the equation, or -1: the size and the face it is set in
    bool cut = false;    // the line is not the whole equation (too long, or nested too deep)
};

// The line of the equation element `n` (m:oMath or m:oMathPara), of at most `maxChars` characters, from content nested at most kMaxEquationDepth deep.
//  - m:r: its text (m:t, or w:t of a run of normal text); an operator that works on two things has a blank each side (a sign has none), and a relation always has.
//  - m:f: the numerator, a slash, and the denominator (each in brackets where it is more than letters and digits); a fraction with no bar has a blank in place of the slash.
//  - m:sSup, m:sSub, m:sSubSup: the base and then its scripts, close up; m:sPre: the scripts and then the base.
//  - m:rad: the degree, the root sign, and the radicand (in brackets where it is more than letters and digits).
//  - m:nary: the operator, its limits (in brackets where they are more than letters and digits), and what it works on, with blanks between.
//  - m:d: its opening character, the arguments with its separator between them, and its closing character.
//  - m:func: the function's name, a blank, and its argument; m:limLow and m:limUpp: the base, a blank, and the limit.
//  - m:m: the matrix row by row, a blank between the cells and a semicolon and a blank between the rows; m:eqArr: its rows with a blank between.
//  - m:acc, m:groupChr: the base and the character set over or under it; m:bar, m:box, m:borderBox, m:phant (unless it is not shown): the base.
// Characters of the mathematical alphanumeric block are the letters and digits they are styles of.
Equation equationText(const XmlTree &t, int n, int maxChars);

// A character of the mathematical alphanumeric block (U+1D400 to U+1D7FF) as the letter, digit, or Greek letter it is a style of; style is 'p' (plain: script, fraktur, sans-serif,
// and monospace, which no bundled face has), 'i' (italic), 'b' (bold), 'j' (bold italic), or 'd' (double-struck, which a bundled face has).
struct StyledLetter {
    uint base = 0;
    char style = 'p';
};
bool styledLetter(uint ch, StyledLetter *out);

// For a test of cost: the elements an equation's line was made from and the characters it was put together from (the pieces joined, before they are cut to what the line may hold).
qint64 equationWorkForTest();
void resetEquationWorkForTest();

} // namespace jp::office::wml
