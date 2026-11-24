#pragma once
// Merge Shapes and Edit Points for the apps that draw on the page editor (JeffDeck): the commands JeffPub's Shape Format tab has, with its words and its rules, made once here.

#include "core/pathops.h"

#include <QAction>
#include <QKeySequence>
#include <QPainterPath>
#include <QString>

#include <atomic>
#include <functional>

class QWidget;

namespace jp {

class Editor;
class Item;

// The ids of the commands, in the order the ribbon lists them.
extern const char *const kMergeShapeIds[5];

// Makes the six commands (Edit Points and the five of Merge Shapes) with `make(id, text, icon, run)`, and shows the box over a merge that takes long (Cancel stops it) on `window`.
// Messages go through Editor::status.
void makeShapeCommands(QWidget *window, Editor *ed, const std::function<QAction *(const QString &id, const QString &text, const char *icon, const std::function<void()> &run)> &make);
// Which commands fit the selection: Edit Points for one shape that can be edited, Merge Shapes for two or more.
void enableShapeCommands(Editor *ed, const std::function<QAction *(const QString &id)> &find);

// The pieces JeffWrite shares with these commands (its shapes are not editor items: the page draws them, and the window merges what it draws).
// The box over a merge that takes long (Cancel sets `*cancel`), and its end.
void showMergeBox(QWidget *window, const QString &text, std::atomic<bool> *cancel);
void closeMergeBox(QWidget *window);
// The outline of a shape or text box item as the page draws it: its own points or its preset's drawing, turned and flipped as the item is. False for any other item and for artwork.
bool shapeOutlineOnPage(const Item &item, QPainterPath *out);
// Merges as Editor::mergeShapes does: the shared limits, and past pathops::kQuickWork on a worker thread while this waits in the event loop, with `started` and `ended` around it (a window shows
// its box there; `*cancel` stops the merge at its next step and the shapes stay as they were). `stepHook`, for tests, is called on the worker before each step. Empty, with `*refused` told why,
// when nothing is made; mergeRefusal() says it in a sentence.
QVector<QPainterPath> mergeBounded(pathops::Op op, const QVector<QPainterPath> &outlines, int *refused, const std::function<void(const QString &, std::atomic<bool> *)> &started, const std::function<void()> &ended,
                                   const std::function<void(int, std::atomic<bool> &)> &stepHook = {});
QString mergeRefusal(int refused);

} // namespace jp
