#pragma once
// Go To for an app whose places are numbered, a page or a slide (gotonumber.cpp): the same dialog name, title, and button as JeffWrite's Go To.

#include <QString>

class QWidget;

namespace jp {

// Asks for a number from 1 to `count` (starting on `current`, counted from 0, and labeled with `prompt`, such as "Page number:").
// Returns the place chosen, counted from 0, or -1 when the dialog is closed or there is nothing to go to.
int goToNumberDialog(QWidget *parent, const QString &prompt, int count, int current);

} // namespace jp
