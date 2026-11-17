#pragma once
// Content controls in the window: the Properties box (title, tag, locks, a list's items, a date's format) and the choices a click on a list or a date shows.

#include "core/contentcontrolops.h"

#include <QWidget>
#include <functional>

class QMenu;

namespace jp::write {

// Content Control Properties for the control that begins at `start`: OK applies them as one step that undoes (a refusal keeps the box open and says why). Returns whether anything was changed.
bool contentControlDialog(QWidget *parent, WriteDoc *doc, int start);

// The choices of the control that begins at `start`, as a menu to show with popup(): the items of a drop-down list (the one shown is checked), or a calendar for a date (with Today and Clear).
// What is chosen is put in the control; `done` is called after. Null for a control with no choices.
QMenu *controlChoicesMenu(QWidget *parent, WriteDoc *doc, int start, const std::function<void()> &done);

} // namespace jp::write
