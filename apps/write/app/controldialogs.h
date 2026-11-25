#pragma once
// Content controls in the window: the Properties box (title, tag, locks, the items of a drop-down list or a combo box, a date's format) and the choices a click on a list or a date shows.

#include "core/contentcontrolops.h"

#include <QWidget>
#include <functional>

class QMenu;

namespace jp::write {

// Content Control Properties for the control that begins at `start`: OK applies them as one step that undoes (a refusal keeps the box open and says why). Returns whether anything was changed.
bool contentControlDialog(QWidget *parent, WriteDoc *doc, int start);
// The same for a control that is already found (one round paragraphs is found by its place, not by an atom).
bool contentControlDialog(QWidget *parent, WriteDoc *doc, const ContentControl &ctl);

// The choices of the control that begins at `start`, as a menu to show with popup(): the items of a drop-down list (the one shown is checked), or a calendar for a date (with Today and Clear).
// What is chosen is put in the control; `done` is called after. Null for a control with no choices.
QMenu *controlChoicesMenu(QWidget *parent, WriteDoc *doc, int start, const std::function<void()> &done);

} // namespace jp::write
