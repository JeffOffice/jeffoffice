#pragma once
// Merge Shapes and Edit Points for the apps that draw on the page editor (JeffDeck): the commands JeffPub's Shape Format tab has, with its words and its rules, made once here.

#include <QAction>
#include <QKeySequence>
#include <QString>

#include <functional>

class QWidget;

namespace jp {

class Editor;

// The ids of the commands, in the order the ribbon lists them.
extern const char *const kMergeShapeIds[5];

// Makes the six commands (Edit Points and the five of Merge Shapes) with `make(id, text, icon, run)`, and shows the box over a merge that takes long (Cancel stops it) on `window`.
// Messages go through Editor::status.
void makeShapeCommands(QWidget *window, Editor *ed, const std::function<QAction *(const QString &id, const QString &text, const char *icon, const std::function<void()> &run)> &make);
// Which commands fit the selection: Edit Points for one shape that can be edited, Merge Shapes for two or more.
void enableShapeCommands(Editor *ed, const std::function<QAction *(const QString &id)> &find);

} // namespace jp
