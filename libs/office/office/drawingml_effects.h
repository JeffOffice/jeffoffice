#pragma once
// The effects of a shape as DrawingML writes them, for every app: the list of effects a menu offers, the element of each (a shadow, a glow, a reflection, soft edges, and a bevel), and putting
// one in or taking it out of what a shape already has while the others stay. Works on the elements as text (the shape's a:effectLst, a:scene3d, and a:sp3d), so each app keeps its own way of
// reading and writing the shape's XML.

#include <QByteArray>
#include <QVector>

namespace jp::office {

enum class ShapeEffectKind { Shadow, Glow, Reflection, SoftEdges, Bevel };

// The effects a menu offers, in the order it lists them.
const QVector<ShapeEffectKind> &shapeEffectKinds();

// The shape's a:effectLst (or empty) with the element of `kind` put in (the usual one, in the place the schema gives it) or taken out; every other element stays. Empty when none are left. Bevel is
// not in this list (see bevelWith).
QByteArray effectListWith(const QByteArray &effectLst, ShapeEffectKind kind, bool on);
// Whether the list (or, for Bevel, the a:sp3d) has the effect.
bool hasShapeEffect(const QByteArray &effectLstOrSolid3d, ShapeEffectKind kind);

// A shape's a:scene3d and a:sp3d.
struct Solid3d {
    QByteArray scene, solid;
};
// The same with a bevel put in (the usual one, with the usual camera and light when the shape has none) or taken out; the shape's other 3-D settings (extrusion, contour, its own camera) stay,
// and both are dropped when the bevel was all there was.
Solid3d bevelWith(const Solid3d &now, bool on);

} // namespace jp::office
