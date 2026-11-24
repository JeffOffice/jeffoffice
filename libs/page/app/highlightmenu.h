#pragma once
// The menu under the Text Highlight Color button of the apps with pages (JeffPub and JeffDeck): the sixteen colors a highlighter has, No Color, and Stop Highlighting.

#include <QString>
#include <functional>

class QAction;
class QColor;
class QMenu;
class QObject;
class QWidget;

namespace jp {

class Editor;

// A color picked goes on the selected text at once; with none selected the highlighter is armed (Editor::chooseHighlight). `stop` is the Stop Highlighting command (the app's).
QMenu *highlightMenu(QWidget *owner, Editor *ed, QAction *stop);

// The sixteen colors with their names and swatches, added to `menu` as they are in the menu above; `pick` gets the color's key (as the text engine names it) and its color. (For an app that keeps
// its highlights by key.) `context` owns the connections.
void addHighlightColors(QMenu *menu, QObject *context, const std::function<void(const QString &key, const QColor &color)> &pick);

}   // namespace jp
