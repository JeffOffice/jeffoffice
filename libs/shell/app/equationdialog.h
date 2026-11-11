#pragma once
// Insert > Equation, for every program that has it: the dialog that asks for an equation typed in a line (a/b, x^2, sqrt(x), \alpha, \sum_(i=1)^n ...), with the equation drawn as it
// will look, and the built-in equations to start from. The model of an equation is libs/office/wml/wmlmathedit.h; what draws it is libs/office/wml/wmlmathpath.h.

#include <QColor>
#include <QImage>
#include <QString>

class QWidget;

namespace jp {

// Asks for an equation: `title` is the dialog's (Insert Equation, or Edit Equation), `linear` is the line it starts with and the line chosen. False when the dialog is closed with Cancel.
// The dialog is named "equationDialog", its line "equationInput", the line of text it makes "equationPreview" (what a page that shows an equation as a line of text shows), and the
// drawing "equationDrawn".
bool askEquation(QWidget *parent, const QString &title, QString *linear);

// An equation typed as `linear`, drawn with letters `emSize` points tall in `color` on a transparent ground (a null image for a line that makes no equation).
QImage equationPicture(const QString &linear, double emSize, const QColor &color, double pixelRatio = 1.0);
} // namespace jp
