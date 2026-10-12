#pragma once
// The tables of animation_presets.cpp, which animation_edit.cpp makes effects from.

#include "core/animation.h"

namespace jp::deck {

// What the presentation program writes inside an effect (its behaviors), for
// one preset of a class: {I} is the next id, {T} the target, {D} the effect's
// duration in milliseconds, {D-1} one less, {D*0.2} a share of it.
struct PresetTemplate {
    EffectClass cls;
    int presetId, subtype;
    int duration;          // ms it lasts unless told otherwise; 0 when it has no length (an appearing shape)
    const char *body;
};
extern const PresetTemplate kPresetTemplates[];
extern const int kPresetTemplateCount;

// A motion path preset: fractions of the slide, from where the shape is.
struct PresetPath {
    int presetId;
    const char *path;
};
extern const PresetPath kPresetPaths[];
extern const int kPresetPathCount;

} // namespace jp::deck
