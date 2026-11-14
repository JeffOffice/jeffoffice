#pragma once
// The menus under the Bullets, Numbering, and Multilevel List buttons of the apps with pages (JeffPub and JeffDeck), the same galleries and boxes as JeffWrite's, offering what their files hold: a
// bullet character with its font, size, and color; the five counting styles (1, A, a, I, i) with three punctuation forms and a start; and nine levels, each with a look of its own.

#include "app/listdialogs.h"

#include <QTextListFormat>
#include <QVector>
#include <functional>

class QAction;
class QMenu;
class QWidget;

namespace jp {

class Editor;

// The list format a look comes to (a bullet with its character, font, size, and color; or a counting style with the punctuation the text round %1 gives, and where it starts).
QTextListFormat listFormatOf(const ListLevelLook &look);
// The look of a list format, the other way.
ListLevelLook listLookOf(const QTextListFormat &format);

// The gallery presets of the menus, nine levels each (a level's look).
struct MultilevelPick {
    QString id, name;
    QVector<ListLevelLook> levels;
};
QVector<MultilevelPick> multilevelPicks();
QVector<ListLevelLook> bulletPicks();   // the bullets of the library, as looks
QVector<ListLevelLook> numberPicks();   // the counting styles of the library, as looks

// The menus. `owner` parents the menu and the boxes it opens. With `multilevel` false the Multilevel List menu offers only Change List Level (for a file that keeps no levels).
// `more`, if given, is the app's own "Bullets and Numbering…" command, added at the end.
// `mode` says which boxes to open, asked each time (a document's file kind can change under the menu).
using ListBoxesChoice = std::function<ListBoxes()>;
QMenu *bulletsMenu(QWidget *owner, Editor *ed, QAction *more = nullptr, const ListBoxesChoice &mode = {});
QMenu *numberingMenu(QWidget *owner, Editor *ed, QAction *more = nullptr, const ListBoxesChoice &mode = {});
QMenu *multilevelMenu(QWidget *owner, Editor *ed, const ListBoxesChoice &mode = {});

} // namespace jp
