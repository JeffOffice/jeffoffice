#pragma once
// The small, steady parts of textengine.h (the names, the enum, and the plain struct that headers
// declaring functions of the text engine need), so that a header that only passes them around
// does not include the engine and recompile with it.

#include <QStringList>

namespace jp {

class StoryLayout;
class ParagraphLayout;
struct LayoutEnv;
struct FrameSpec;
struct FieldContext;
struct FieldWork;
struct PaintOptions;

// Where text is drawn: bold drawn by hand (a font with no bold face) is a
// different stroke on the screen and in pictures, in a PDF Publisher exports,
// and on paper.
enum class TextOutput { Picture, Pdf, Print };

// What a story's layout depends on besides its own text and its frames: the fields that show what nothing else in a layout's key says (the
// date and time, business information, mail merge values), and the stories of the notes it refers to.
struct StoryFields {
    QStringList codes;   // the code of each such field (in the story or in a note), once
    QStringList notes;   // the notes' stories
};

}  // namespace jp
