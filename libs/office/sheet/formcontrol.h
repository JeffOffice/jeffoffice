#pragma once
// The form controls of a worksheet (check box, option button, button, drop-down, list box, spin button, scroll bar, group box, and label): what a sheet's legacy drawing (its VML shapes, with
// the ClientData each has) and its control properties parts (xl/ctrlProps) say of each, read into a model, and the same changed back into those parts. Nothing here runs a macro: a control
// is a place on the sheet, a look, and the numbers that it sets in a cell.
//
// The parts come from a stranger: the drawing is scanned as bytes (it is often not well-formed XML), at most kMaxFormControls shapes are read, and the numbers are kept to what a control may
// hold. Writing changes only what a person changed: a shape's properties (the cell it sets, the range it lists, its state, and its limits) and the properties part's attributes of the same names;
// every other byte of the file stays as it was. A control made here is written as a shape of the legacy drawing, as the 2007 file format keeps them.

#include "sheet/drawing.h"

#include <QByteArray>
#include <QRectF>
#include <QString>
#include <QVector>

#include <map>
#include <vector>

namespace jp::office { class Package; }

namespace jp::office::sheet {

enum class ControlKind : quint8 { CheckBox, Radio, Button, Drop, List, Spin, Scroll, GroupBox, Label };

constexpr int kMaxFormControls = 5000;
constexpr qint64 kMaxControlPartBytes = qint64(32) << 20;

struct FormControl {
    ControlKind kind = ControlKind::Button;
    qint64 shapeId = 0;            // the number in the shape's id ("_x0000_s1025": 1025)
    QString name;                  // the name the sheet's `controls` gives it ("Check Box 1"), else the shape's
    QString text;                  // its caption
    QString fontFace;              // the face and size (points) of its caption, when the file says (size 0: not said)
    double fontPt = 0;
    quint32 color = 0xFF000000;    // the caption's color (ARGB)
    DrawingAnchor anchor;          // where it is, as an anchor between two cells (or an absolute one, for a shape that has only a place in points)
    bool hidden = false;
    bool prints = true;
    bool locked = true;
    bool threeD = true;            // drawn with a raised look (the file does not say noThreeD)
    bool horizontal = false;       // a scroll bar or spin button laid out left to right
    bool firstButton = false;      // an option button that begins a group
    QString macro;                 // the macro it names (never run)
    QString link;                  // the cell it sets, as the file writes it ("$A$3", "Sheet1!$A$3")
    QString range;                 // the cells it lists
    int checked = 0;               // a check box or option button: 0 not, 1 checked, 2 mixed
    int val = 0;                   // a spin button or scroll bar's value
    int min = 0, max = 30000, inc = 1, page = 10;
    int sel = 0;                   // a drop-down or list box's chosen place (from 1; 0 none)
    int dropLines = 8;
    QString selType;               // a list box's way of choosing ("Single", "Multi", "Extend")
    // Where the file has it.
    QString vmlPart, ctrlPart;     // the legacy drawing that holds its shape, and the properties part (empty for none)
    // For a control that was made here: where it goes, in points from the sheet's top left (the file's drawing needs the place as well as the cells).
    bool isNew = false;
    QRectF place;
    // For a control of the file that was taken away (deleted, or its cells were): saving drops its shape, and its entry in the sheet.
    bool removed = false;
};

// The controls of the sheet whose part is `sheetPart`, in the order the file draws them (the first at the back). `truncated`, if given, says a limit stopped the read.
QVector<FormControl> readFormControls(const Package &package, const QString &sheetPart, bool *truncated = nullptr);

// Whether two controls say the same of everything that saving writes.
bool sameControlState(const FormControl &a, const FormControl &b);

// The name ObjectType of a control's shape has ("Checkbox"), and the one a properties part has ("CheckBox").
const char *controlVmlType(ControlKind kind);
const char *controlPropType(ControlKind kind);

// A drawing for a sheet that has none (its head, with the shape type controls use).
QByteArray newControlsDrawing();
// The shapes of the drawing's bytes with the controls put in: each control that is not new and is not as `was` says (matched by shape id) has its shape's properties changed; each control that
// is new gets a shape at the end. Nothing else changes (and nothing at all, byte for byte, when no control differs). `was` are the controls as the file has them.
// A control made here that was given another shape number (the one it asked for was taken, by a note's shape, say) is in `assigned`: asked number to number given.
bool patchControlsVml(const QByteArray &vml, const std::vector<FormControl> &was, const std::vector<FormControl> &now, QByteArray *out, QString *why = nullptr,
                      std::map<qint64, qint64> *assigned = nullptr);
// A properties part with the control's state put in (its attributes of the same names as the shape's elements); the bytes as they were when nothing differs.
QByteArray patchControlProp(const QByteArray &xml, const FormControl &control);
// The shape of one new control, with the id given.
QByteArray controlShapeXml(const FormControl &control, qint64 shapeId);
// The largest shape id in a drawing (0 when it has none), so that a shape made now has one of its own.
qint64 largestShapeId(const QByteArray &vml);

} // namespace jp::office::sheet
