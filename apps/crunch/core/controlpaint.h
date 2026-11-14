#pragma once
// How a form control looks (core/sheetcontrols.h): drawn in the grid's units (pixels at 100%) inside the rectangle its anchor gives, and where a point of it is (what a click on it means). The
// look is a plain one, the same on every computer: a face of light gray, a white field for a list, a dark thin edge; the words are in the face and size the file says.

#include "sheet/formcontrol.h"

#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QStringList>

namespace jp::crunch {

// The height of a line of a list, and the width of a drop-down's button and of the arrows of a spin button and scroll bar, in units.
constexpr double kListLine = 16;
constexpr double kButtonWidth = 16;

// Draws the control in `rect`; `items` are the lines of a list or drop-down (the one chosen is `sel` of the control).
void paintControl(QPainter *painter, const QRectF &rect, const jp::office::sheet::FormControl &control, const QStringList &items);

// What a point of a control (in units from its top left, the control being `size` big) is on.
enum class ControlPart : quint8 { None, Body, Up, Down, PageUp, PageDown, Thumb, Line };
struct ControlHit {
    ControlPart part = ControlPart::None;
    int line = 0;   // for a line of a list: its place (from 1)
};
ControlHit controlHit(const jp::office::sheet::FormControl &control, const QSizeF &size, const QPointF &at, int lineCount);

} // namespace jp::crunch
