#pragma once
// Equations: Insert > Equation puts in an equation typed in a line (a/b for a fraction, x^2 and x_i for scripts, sqrt(x) for a root, \alpha and the other names for symbols, and \sum, \int,
// and \prod with limits), kept in the file as the math markup the format has for it (m:oMath). The pages show an equation as a line of text, as they show those a file holds.

#include "core/writedoc.h"

#include <QString>
#include <QTextCursor>
#include <QVector>

namespace jp::write {

// The most characters of a typed equation, and the deepest it nests (a group, a fraction, a root, or a script inside another).
constexpr int kMostEquationInput = 2000;
constexpr int kMostEquationDepth = 24;

// The math markup (an m:oMath element, declaring its own namespace) of a typed equation, or an empty array for one that is empty or too large. `ok` (if given) is false when it is.
QByteArray equationMarkup(const QString &linear, bool *ok = nullptr);
// The line the pages show for the markup (what reading a file with the same equation gives).
QString equationLine(const QByteArray &markup);
// The name of a symbol written \name as the character it stands for, or an empty string for a name that is none.
QString equationSymbol(const QString &name);

// The built-in equations of the gallery: a name and the line that makes each.
struct EquationDesign {
    QString name, linear;
};
const QVector<EquationDesign> &equationGallery();
// The line that makes this markup again (what equationMarkup reads), for markup made from such a line; empty for markup with anything else in it (it is then changed only as a whole by
// typing a new equation, never half undone).
QString equationLinear(const QByteArray &markup);
// The line of the equation that is the character just after `position` of the body text, or empty when that is no equation, or one that has no line.
QString equationAt(const WriteDoc &doc, int position);
// Puts another equation in the place of the one at `position`, one step that undoes; false for an empty or too large one, or where there is no equation, or one already so.
bool replaceEquation(WriteDoc &doc, int position, const QString &linear);

// Puts the equation at the cursor, in place of a selection, as one step that undoes. False for an empty equation, one too large, or where an equation can't go (a locked place).
bool insertEquation(WriteDoc &doc, QTextCursor &c, const QString &linear);

} // namespace jp::write
