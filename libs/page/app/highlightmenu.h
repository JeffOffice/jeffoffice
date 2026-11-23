#pragma once
// The menu under the Text Highlight Color button of the apps with pages (JeffPub and JeffDeck): the sixteen colors a highlighter has, No Color, and Stop Highlighting.

class QAction;
class QMenu;
class QWidget;

namespace jp {

class Editor;

// A color picked goes on the selected text at once; with none selected the highlighter is armed (Editor::chooseHighlight). `stop` is the Stop Highlighting command (the app's).
QMenu *highlightMenu(QWidget *owner, Editor *ed, QAction *stop);

}   // namespace jp
