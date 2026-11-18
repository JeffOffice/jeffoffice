#pragma once
// Insert > Equation, for every program that has it: the dialog that asks for an equation typed in a line (a/b, x^2, sqrt(x), \alpha, \sum_(i=1)^n ...), with the equation drawn as it
// will look, and the built-in equations to start from. The model of an equation is libs/office/wml/wmlmathedit.h; what draws it is libs/office/wml/wmlmathpath.h.

#include <QByteArray>
#include <QColor>
#include <QImage>
#include <QString>

class QWidget;

namespace jp {

// Asks for an equation: `title` is the dialog's (Insert Equation, or Edit Equation), `linear` is the line it starts with and the line chosen. False when the dialog is closed with Cancel.
// The dialog is named "equationDialog", its line "equationInput", the line of text it makes "equationPreview" (what a page that shows an equation as a line of text shows), and the
// drawing "equationDrawn".
bool askEquation(QWidget *parent, const QString &title, QString *linear);

// The same dialog for a program that keeps the equation's markup (not only the line it was typed as). The dialog also lets the equation be typed in UnicodeMath or LaTeX (the
// choice "equationSyntax", the line stays the first) and shows it as built up (Professional) or only as typed (Linear), the choices "equationProfessional" and "equationLinear". `linear` (if given) is the
// equation as a line of our own syntax, or empty when it has none (a function, say); `markup` is always the equation's markup. With `markup` null (askEquation), an equation that
// has no line of our own syntax can't be accepted.
// `markup` is also the markup the equation had when it is edited (empty for a new one; a stored line that no longer reads as it did is shown in the line that does, see
// equationLineForEditing). `form` (if given) is the form the dialog starts in and the one chosen.
enum class EquationForm { Professional, Linear };
// The dialog has a strip with the structures and symbols of the Equation tab (equationtab.h), which put text into the line at its caret; `startPick` (an equationPickText pick: the tab's
// button was clicked) is put in at the end of the line when the dialog opens. A program that keeps the markup opens in the typing format chosen on the tab (equationSyntaxSetting).
bool askEquationMarkup(QWidget *parent, const QString &title, QString *linear, QByteArray *markup, EquationForm *form = nullptr, const QString &startPick = QString());

// An equation typed as `linear`, drawn with letters `emSize` points tall in `color` on a transparent ground (a null image for a line that makes no equation).
QImage equationPicture(const QString &linear, double emSize, const QColor &color, double pixelRatio = 1.0);
} // namespace jp
