#pragma once
// Replace Fonts for a document of placed objects (JeffPub's publication and JeffDeck's presentation): every font the text names, in the text frames, shapes, table cells, notes, master
// pages, and slide layouts (all of them are stories of the document), in the text styles, in Text Art, and in the list of fonts a `.pub` file named. The theme's two fonts are
// separate entries ("+Heading" and "+Body"): a run that follows the theme stays following it, and the theme's font changes only when it is the one picked.

#include "app/replacefonts.h"   // (libs/shell)

#include <QString>
#include <QVector>

namespace jp {

class Document;

// The fonts the document names, most used first, then the font scheme's two fonts (when the text uses them), with how many places use each.
QVector<FontUse> pageFontsUsed(const Document &doc);
// Puts `to` in the place of `key` (a family; "+Heading" or "+Body" for the font scheme's own) everywhere in the document, without any record for Undo (the editor's change() makes
// it one step). How many places changed.
int replacePageFont(Document &doc, const QString &key, const QString &to);

} // namespace jp
