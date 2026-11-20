#pragma once
// Duplicate Several Times: how many copies, how far each is moved on from the one before, how much bigger or smaller each is, and a fill color that goes from the
// first object's to the one chosen at the last copy. The choice goes to Editor::duplicateSteps as one step of Undo. Built once, for every app that has an Editor.

class QWidget;

namespace jp {

class Editor;

// Returns whether copies were made (the person accepted and something was selected).
bool duplicateStepsDialog(QWidget *p, Editor *ed);

} // namespace jp
