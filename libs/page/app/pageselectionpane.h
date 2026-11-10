#pragma once
// The Selection Pane of the apps that edit pages (JeffPub and JeffDeck): the objects of the page or slide on the canvas, or of the master being edited, front first. Every change is one step of
// the editor's Undo, and the pane follows the editor (a pick, a change, another page, Undo).

#include "app/selectionpane.h"
#include "core/items.h"

#include <QPointer>

namespace jp {

class Editor;

// The name an object shows in the pane when it has none of its own: its kind and its number from the back.
QString defaultObjectName(const Item &item, int numberFromBack);

// The pane for `editor`, made once per window (`slot` holds it: a pane still open is raised) and shown.
SelectionPane *showPageSelectionPane(QWidget *parent, Editor *editor, QPointer<SelectionPane> *slot);

// The Reading Order pane for `editor`: the same objects, but the one read first at the top (the back of the page's stack), moved earlier or later to change what a screen reader and a
// tagged PDF read first. `slot` holds it, as above.
SelectionPane *showPageReadingOrderPane(QWidget *parent, Editor *editor, QPointer<SelectionPane> *slot);

} // namespace jp
