#pragma once
// The presets of the Format pane's Effects page (shadows, reflections, glows, and soft edges): what each sets on an object's effects. The pane
// and an app's Shape Effects menu use these, so both reach the same settings.

#include "core/style.h"
#include "core/texteffects.h"

#include <QString>

namespace jp::formatpresets {

// Shadows: outer shadows by where they fall: bottom right, bottom, bottom left, right, center, left, top right, top, and top left.
int shadowCount();
QString shadowName(int i);
// Turns the shadow on, with the preset's look.
void applyShadow(Effects &fx, int i);

// Reflections: tight, half, and full, each touching the object.
int reflectionCount();
QString reflectionName(int i);
void applyReflection(Effects &fx, int i);

// Glows of 5, 10, and 18 points in the color given.
int glowCount();
QString glowName(int i);
void applyGlow(Effects &fx, int i, const ColorRef &color);

// The same shadows, reflections, and glows on the letters of a run (what a run's effect list holds), for Text Effects: shadowName, reflectionName, and
// glowName name them. Each turns the effect on and leaves the others in the list as they are.
void applyTextShadow(TextEffects &fx, int i);
void applyTextReflection(TextEffects &fx, int i);
void applyTextGlow(TextEffects &fx, int i, const QColor &color);

// Soft edges of 1, 2.5, 5, 10, 25, and 50 points.
int softEdgeCount();
QString softEdgeName(int i);
void applySoftEdge(Effects &fx, int i);

// Bevels (the top bevel of a shape or picture, the types of Effects::bevel): circle, relaxed inset, cool slant, angle, soft round, convex, slope, and
// divot, each 6 points wide and high but for the slope and divot, which are a little smaller.
int bevelCount();
QString bevelName(int i);
void applyBevel(Effects &fx, int i);

// Turns in space (Effects::rot3d): none, perspective left, right, above, and below, and off axis; each with its angles and the perspective it is seen at.
int rotationCount();
QString rotationName(int i);
void applyRotation(Effects &fx, int i);

} // namespace jp::formatpresets
