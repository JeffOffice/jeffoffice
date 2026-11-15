#pragma once
// JeffPub's Page Number gallery (the designs are the shared gallery's, libs/shell/app/pagenumbergallery.h): a design put on the master page of the page being shown, at its top or bottom
// or in its margin, made of boxes with JeffPub's page number and page count fields; a design of the current position put in the text being typed; and Remove Page Numbers.

#include "app/pagenumbergallery.h"

#include <QColor>
#include <QString>

namespace jp {

class Editor;

// The designs the gallery offers JeffPub: the top, the bottom, the margins, and the current position's plain, X of Y, and bold (a circled number can't be put in a line of text here).
bool pubOffersPageNumberDesign(const PageNumberDesign &d);

// The accent of the publication's color scheme, the color the designs' pictures and shapes are drawn in.
QColor pubPageNumberAccent(const Editor *ed);

// Puts the design `id` on the master page of the page the editor shows (the first master where the page has none): a box of an earlier design goes first, and the other items of the master stay.
// A design of the current position goes in the text being typed, and nothing is done (false) when none is. One step of Undo.
bool placePageNumberDesign(Editor *ed, const QString &id);

// Takes the page number and page count fields out of the master pages, and the boxes of the designs (and of the older Page Number box); the other text of the masters stays. One step of Undo.
// Returns how many items and fields went.
int removePageNumbers(Editor *ed);

} // namespace jp
