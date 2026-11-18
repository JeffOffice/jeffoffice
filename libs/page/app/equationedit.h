#pragma once
// Equations on a page (Insert > Equation in JeffPub): making the object from a typed line, and changing the line of one that is there. The model of the line is
// libs/office/wml/wmlmathedit.h, the dialog that asks for it libs/shell/app/equationdialog.h, and the outlines are drawn by Renderer::equationOutline.

#include "core/itemsfwd.h"

#include <QByteArray>
#include <QSizeF>
#include <QString>
#include <QPointF>

namespace jp {

class Editor;

// The equation a typed line makes, with its frame the size the equation is at the size asked for (points tall letters) and its middle at `center`; null for an empty line, one that makes no
// equation, or one that is too large (the limits of wmlmathedit.h).
std::shared_ptr<EquationItem> makeEquation(const QString &linear, const QPointF &center, double size = 24);

// The same for an equation that is already markup (typed in any format, as the dialog makes it); `linear` (empty when no line of our syntax makes it) is kept for editing, and `linearForm` is how it is shown.
std::shared_ptr<EquationItem> makeEquationFromMarkup(const QByteArray &markup, const QString &linear, bool linearForm, const QPointF &center, double size = 24);

// Puts the equation in the middle of the page being edited, selected, as one step that undoes ("Insert Equation"). The id of the item, or empty when the line makes none.
QString insertEquation(Editor *ed, const QString &linear);
QString insertEquationMarkup(Editor *ed, const QByteArray &markup, const QString &linear, bool linearForm);

// Changes the line of the equation `id`, keeping its middle and the size its letters are at; one step that undoes ("Edit Equation"). False for a line that makes no equation, or where
// there is no such equation, or the line is the one it has.
bool changeEquation(Editor *ed, QString id, const QString &linear);
// Changes the equation `id` to this markup (and line, and form) in the same way, as one step that undoes; false for markup that makes no equation, or where nothing would change.
bool changeEquationMarkup(Editor *ed, QString id, const QByteArray &markup, const QString &linear, bool linearForm);
// Shows the equation `id` Linear or Professional, as one step that undoes ("Edit Equation"); false when it already is, or there is no such equation.
// The text that says the equation for a reader who can't see it (alt text, the e-book, the screen reader): the line it was typed as when a line of our syntax makes it, else its
// UnicodeMath, else the plain text of its math (so an equation typed in LaTeX is never silent).
QString equationReadableText(const EquationItem &eq);
// The equation as the files that have no Linear form keep it: built up, its frame the built-up layout's (the scale the person gave the frame kept, its top-left kept).
std::shared_ptr<EquationItem> builtUpEquation(const EquationItem &eq);
bool setEquationLinearForm(Editor *ed, QString id, bool linearForm);

} // namespace jp
