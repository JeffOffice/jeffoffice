#pragma once
// The presets of the Format pane's Effects page (shadows, reflections, glows, and soft edges): what each sets on an object's effects. The pane
// and an app's Shape Effects menu use these, so both reach the same settings.

#include "core/style.h"

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

// Soft edges of 1, 2.5, 5, 10, 25, and 50 points.
int softEdgeCount();
QString softEdgeName(int i);
void applySoftEdge(Effects &fx, int i);

} // namespace jp::formatpresets
