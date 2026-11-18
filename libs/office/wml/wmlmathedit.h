#pragma once
// Equations typed in a line (a/b for a fraction, x^2 and x_i for scripts, sqrt(x) for a root, \alpha and the other names for symbols, and \sum, \int, and \prod with limits), made into the
// math markup of ECMA-376 part 1, chapter 22 (m:oMath), and back. Every program that has Insert > Equation shares this: it is the one model of an equation, with its limits and its gallery.

#include <QByteArray>
#include <QString>
#include <QVector>

namespace jp::office::wml {

// The most characters of a typed equation, and the deepest it nests (a group, a fraction, a root, or a script inside another).
constexpr int kMostEquationInput = 2000;
constexpr int kMostEquationDepth = 24;

// The math markup (an m:oMath element, declaring its own namespace) of a typed equation, or an empty array for one that is empty or too large. `ok` (if given) is false when it is.
QByteArray equationMarkup(const QString &linear, bool *ok = nullptr);
// The line the pages show for the markup (what reading a file with the same equation gives).
QString equationLine(const QByteArray &markup);
// The name of a symbol written \\name as the character it stands for, or an empty string for a name that is none.
QString equationSymbol(const QString &name);

// The built-in equations of the gallery: a name and the line that makes each.
struct EquationDesign {
    QString name, linear;
};
const QVector<EquationDesign> &equationGallery();
// The line that makes this markup again (what equationMarkup reads), for markup made from such a line; empty for markup with anything else in it (it is then changed only as a whole by
// typing a new equation, never half undone).
QString equationLinear(const QByteArray &markup);

// The line to show when an equation kept with its markup is edited. A line stored before Normal Text was read with a pair of quotes meant those quotes themselves; if the markup stored
// with `line` is not what the line makes now, the line is the markup's own (which writes such quotes so that they read back as they were).
QString equationLineForEditing(const QString &line, const QByteArray &markup);

// Equations typed as text in the two syntaxes people know, and the text for any math markup (not only markup a typed line made), as wmlmathuni.h and wmlmathlatex.h say. The
// functions that read return the markup (an m:oMath element), or an empty array, with `ok` (if given) false, for text that is empty, past kMostEquationInput characters, nested past
// kMostEquationDepth, or past the limits of one equation (4,000 pieces, 1,024 cells in a matrix); the functions that print return an empty string for markup that holds
// something the syntax can't say, or whose text would pass kMostEquationInput characters.
QByteArray equationFromUnicodeMath(const QString &text, bool *ok = nullptr);
QByteArray equationFromLatex(const QString &text, bool *ok = nullptr);
QString equationToUnicodeMath(const QByteArray &markup);
QString equationToLatex(const QByteArray &markup);

} // namespace jp::office::wml
