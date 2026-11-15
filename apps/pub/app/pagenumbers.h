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

// What becomes of a page number or count of the person's own on the master (a field in text they wrote) when a design goes on: it is left (Keep), or taken out of that text (Replace).
enum class PubOwnNumbers { Keep, Replace };

// Whether the master page that the page the editor shows uses has a page number or count that the gallery did not put there (a field in the person's own text): what a design put on
// would be asked about.
bool pubMasterHasOwnPageNumber(const Editor *ed);

// Puts the design `id` on the master page of the page the editor shows (the first master where the page has none): the boxes of an earlier design go first, and the other items of the master
// stay; with `Replace` a page number of the person's own on the master is taken out of its text. A design of the current position goes in the text being typed, and nothing is done (false)
// when none is. One step of Undo.
bool placePageNumberDesign(Editor *ed, const QString &id, PubOwnNumbers own = PubOwnNumbers::Keep);

// Takes the boxes of the designs (the items named "Page Number", and their rules) off the master pages, and nothing else: a page number in the person's own text stays. One step of Undo.
// Returns how many items went.
int removePageNumbers(Editor *ed);
// Whether a master page has a box of a design (Remove Page Numbers has something to do).
bool hasPageNumbers(const Editor *ed);

} // namespace jp
