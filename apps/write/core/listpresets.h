#pragma once
// The lists the Bullets, Numbering, and Multilevel List galleries offer, as definitions of the file's own numbering model (office::wml::AbstractNum): what each gallery entry
// makes, and the plain definitions a new list starts from.

#include "wml/wml.h"

#include <QString>
#include <QVector>

namespace jp::write {

// A definition of nine levels of plain bullets (a bullet, a circle, a square, over again) or of plain numbers (1., a., i., over again), each indented a half inch more than the one above.
office::wml::AbstractNum plainListDefinition(bool numbers);

struct BulletPreset {
    QString text, font, name;   // the character, its font (empty: the text's own), and what to call it
};
// The bullets of the gallery, in order (a bullet, a circle, a square, a diamond, an arrowhead, a check mark, a dash).
QVector<BulletPreset> bulletPresets();

struct NumberPreset {
    QString id, numFmt, text, example;   // the id, the format (decimal, upperRoman...), the level's text ("%1."), and the first numbers as they read
};
QVector<NumberPreset> numberPresets();

struct MultilevelPreset {
    QString id, name;
    office::wml::AbstractNum def;
};
// The multilevel lists of the gallery. The last two are linked to the heading styles (Heading1 to Heading9).
QVector<MultilevelPreset> multilevelPresets();

// A level that is a bullet of `text` in `font` (its indents are the caller's).
office::wml::Level bulletLevel(const QString &text, const QString &font = QString());
// A level numbered in `numFmt`, its text as `text` says ("%1." or "Step %1:"), counting from `start`.
office::wml::Level numberLevel(const QString &numFmt, const QString &text, int start = 1);

// What tells a definition's levels from another's: two definitions with the same text here come to the same lists.
QString definitionSignature(const office::wml::AbstractNum &def);

} // namespace jp::write
