#pragma once
// Equations: Insert > Equation puts in an equation typed in a line (a/b for a fraction, x^2 and x_i for scripts, sqrt(x) for a root, \alpha and the other names for symbols, and \sum, \int,
// and \prod with limits), kept in the file as the math markup the format has for it (m:oMath). The pages show an equation as a line of text, as they show those a file holds.

#include "core/writedoc.h"
#include "wml/wmlmathedit.h"

#include <QString>
#include <QTextCursor>
#include <QVector>

namespace jp::write {

// The model of an equation (the line typed, the markup made from it, the gallery) is shared with the other programs: libs/office/wml/wmlmathedit.h.
using office::wml::EquationDesign;
using office::wml::equationGallery;
using office::wml::equationLine;
using office::wml::equationLinear;
using office::wml::equationMarkup;
using office::wml::equationSymbol;
using office::wml::kMostEquationDepth;
using office::wml::kMostEquationInput;

// The line of the equation that is the character just after `position` of the body text, or empty when that is no equation, or one that has no line.
QString equationAt(const WriteDoc &doc, int position);
// Puts another equation in the place of the one at `position`, one step that undoes; false for an empty or too large one, or where there is no equation, or one already so.
bool replaceEquation(WriteDoc &doc, int position, const QString &linear);

// Puts the equation at the cursor, in place of a selection, as one step that undoes. False for an empty equation, one too large, or where an equation can't go (a locked place).
bool insertEquation(WriteDoc &doc, QTextCursor &c, const QString &linear);

// The pictures of the equations with one between `from` and `to` of the story made again from the look their runs have now (after a change of size or color, or a paste: a copy from another
// document names a picture of that document's): the one that cannot be made loses its picture and shows its line.
void refreshEquationPictures(WriteDoc &doc, QTextDocument *story, int from, int to, bool joinPrevious = false);   // (joinPrevious: as part of the step that was just made)

} // namespace jp::write
