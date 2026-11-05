#pragma once
// Design > Paragraph Spacing: ready-made spacing for the paragraphs of the whole document, set on the style that ordinary paragraphs have (the default paragraph style, and so the styles based
// on it that say nothing of their own). One step of the history, with the restyling of the text.

#include "core/writedoc.h"

#include <QString>
#include <QVector>

namespace jp::write {

struct SpacingPreset {
    QString id;                 // "default", "none", "compact", "tight", "open", "relaxed", "double"
    int before = 0, after = 0;  // twentieths of a point
    int line = 240;             // 240ths of a line
};

// The presets, in the order of the menu.
const QVector<SpacingPreset> &spacingPresets();
// The name a preset is shown by.
QString spacingPresetName(const QString &id);
// Gives the document's default paragraph style the spacing of the preset; false where the preset is unknown, the style can't be had, or the spacing is already so.
bool applyParagraphSpacing(WriteDoc &doc, const QString &presetId);
// The preset the default paragraph style has now; empty when it has none of them.
QString paragraphSpacingOf(const WriteDoc &doc);

} // namespace jp::write
