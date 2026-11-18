#pragma once
// UnicodeMath: an equation typed as plain text, read into math markup (equationFromUnicodeMath) and printed from it (equationToUnicodeMath); both are declared in wmlmathedit.h.
//
// The precedence table (the order the reader binds, tightest first; the printer writes what the reader takes back as the same markup):
//  1. An atom: a number (digits with a decimal point), one letter, one symbol (\alpha and the other names, or the character), "normal text" (a backslash and a quote or backslash
//     inside it stand for themselves), \matrix(a&b@c&d) or the black square followed by the same (cells divided by &, rows by @ or #), a root (the root sign, \sqrt, the cube root
//     sign, or the fourth root sign; (n&x) is the nth root of x, and a bracket group stands for what is inside), a large operator (the sum, product, coproduct, integral, double,
//     triple, and contour integrals, union, intersection, and the big and/or signs, or their \names), a function name (sin, cos, lim, ...), a bracket group, an invisible group,
//     and a character that is none of these (an operator, a sign, or a space written as backslash and space), which is not an operand.
//     Blanks between pieces are skipped. A backslash and a character that is no letter is that character.
//     Brackets: ( ) [ ] { } and the angle, floor, and ceiling brackets make a bracket element (m:d) with what is inside; | inside divides its arguments, as in (a|b); the open sign
//     followed by a character, the text, and the close sign followed by a character make brackets with any two characters (a space is none): the open sign then | ... the close sign
//     then |. The invisible group (the white tortoise-shell brackets) shows no brackets.
//  2. An accent: a combining mark after an atom puts it over that atom (x with a hat); \hat, \bar, \vec, ... and \overline and \underline take the atom after them (a bracket group
//     gives its inside).
//  3. Scripts: ^ and _ after an element: the element is the base, the operand is the next atom (a bracket group gives its inside), at most one of each, in either order
//     (x^2_i and x_i^2 are the same). A second script of the same kind goes on the operand of the first: x^a^b is x^(a^b), x^a_b^c is x_b^(a^c). A missing operand is empty.
//  4. Fraction: a / after an element that is an operand (a letter, number, symbol, or any structure), over the next element (an atom with its accents and scripts): left to right, so
//     a/b/c is (a/b)/c; a bracket group on either side gives its inside, so (a+b)/(c+d) has no brackets; a/b^2 is a over b squared, and 1+x/2 is 1 plus x/2 (only the atom before
//     the slash is the numerator). A / after anything else, or an escaped one, is a character.
//  5. The operand of a large operator or a function: the next term (an element, then / and an element, as in 4), with its brackets kept: sin(x) has the brackets, and so does
//     the sum of (x+1); an invisible group takes more than one. None when the next thing is not an operand (a sign). A large operator takes its limits first (_ and ^ in either
//     order, then \limits or \nolimits to put them over and under or beside), a function name takes its scripts first (a subscript of lim, max, min, sup, or inf goes under it).
//  6. Juxtaposition: the pieces in a row are the equation.
// Limits: kMostEquationInput characters, kMostEquationDepth nested, 4,000 pieces, and 1,024 cells in a matrix.

#include "wml/wmlmathedit.h"
