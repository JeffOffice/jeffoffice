#pragma once
// Equations on a page (Insert > Equation in JeffPub): making the object from a typed line, and changing the line of one that is there. The model of the line is
// libs/office/wml/wmlmathedit.h, the dialog that asks for it libs/shell/app/equationdialog.h, and the outlines are drawn by Renderer::equationOutline.

#include "core/items.h"

#include <QSizeF>
#include <QString>

namespace jp {

class Editor;

// The equation a typed line makes, with its frame the size the equation is at the size asked for (points tall letters) and its middle at `center`; null for an empty line, one that makes no
// equation, or one that is too large (the limits of wmlmathedit.h).
std::shared_ptr<EquationItem> makeEquation(const QString &linear, const QPointF &center, double size = 24);

// Puts the equation in the middle of the page being edited, selected, as one step that undoes ("Insert Equation"). The id of the item, or empty when the line makes none.
QString insertEquation(Editor *ed, const QString &linear);

// Changes the line of the equation `id`, keeping its middle and the size its letters are at; one step that undoes ("Edit Equation"). False for a line that makes no equation, or where
// there is no such equation, or the line is the one it has.
bool changeEquation(Editor *ed, const QString &id, const QString &linear);

} // namespace jp
