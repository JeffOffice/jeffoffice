#pragma once
// LaTeX: an equation typed as the math-mode text of a LaTeX document, read into math markup (equationFromLatex) and printed from it (equationToLatex); both are declared in
// wmlmathedit.h.
//
// Read: letters, digits (a run of digits is one number), and other characters as they are; \frac, \dfrac, \tfrac{a}{b}; \sqrt{x} and \sqrt[n]{x}; \sum, \prod, \coprod, \int, \iint,
// \iiint, \oint, \oiint, \bigcup, \bigcap, \bigvee, and \bigwedge with _ and ^ and then the next element (an atom with its scripts), with \limits or \nolimits to put the limits over
// and under or beside; x^2_i and x_i^2 (a second script of one kind goes on the first's operand); \left( ... \middle| ... \right) with ( ) [ ] \{ \} | \| . \langle \rangle \lfloor
// \rfloor \lceil \rceil; \begin{matrix}, pmatrix, bmatrix, Bmatrix, vmatrix, and Vmatrix with & between cells and a double backslash between rows; \text{...}, \textrm, and \mbox for
// Normal Text, \mathrm{...} for upright letters, \mathbf{...} for bold ones, \operatorname{name} and the function names (\sin, \lim_{x\to 0}, and the rest) with the next element as
// their argument; accents \hat, \check, \tilde, \acute, \grave, \breve, \dot, \ddot, \bar, \vec (and \widehat, \widetilde) and \overline and \underline; the names of symbols
// (\alpha, \leq, \infty, ...); \, \; \: \! \quad \qquad and ~ for a blank; { ... } to group; a backslash and a character that is no letter is that character. A command that is none stays as
// typed. An environment that is none, a missing \end, or a bracket that does not close is refused.
// Limits: kMostEquationInput characters, kMostEquationDepth nested, 4,000 pieces, and 1,024 cells in a matrix.

#include "wml/wmlmathedit.h"
