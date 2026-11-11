#pragma once
// Where Go To goes (JeffDeck's Go To dialog, app/gotodialog.cpp): a section by name or number, a comment by number or the next or previous one, and an object by the name the Selection pane shows. Each answer is made by one pass over what it needs, run when Go To is pressed.

#include "core/document.h"

#include <QString>

namespace jp::deck {

enum class GoToKind { Slide, Section, Comment, Object };

struct GoToFrom {
    int slide = 0;         // the slide being edited
    QString commentId;     // the comment open in the pane, if any
    QString itemId;        // the object selected on `slide`, if one
};

struct GoToPlace {
    int slide = -1;        // -1: nowhere
    QString commentId;     // for a comment
    QString itemId;        // for an object
};

// `step` 0 goes by `text` (a number, or a name for a section or an object); 1 and -1 go to the next and the previous section, comment (round to the other end), or object of the slide being edited (the objects from the one selected). When there is no such place, `slide` is -1 and `why` says so.
GoToPlace goToPlace(const Document &d, GoToKind kind, const QString &text, int step, const GoToFrom &from, QString *why);

} // namespace jp::deck
