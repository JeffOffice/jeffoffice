#pragma once
// Replace Fonts for a document of placed objects (JeffPub's publication and JeffDeck's presentation): every font the text names, in the text frames, shapes, table cells, notes, master
// pages, and slide layouts (all of them are stories of the document), in the text styles, in Text Art, and in the list of fonts a `.pub` file named. The theme's two fonts are
// separate entries ("+Heading" and "+Body"): a run that follows the theme stays following it, and the theme's font changes only when it is the one picked.

#include "app/replacefonts.h"

#include <QString>
#include <QVector>
#include <functional>

class QWidget;

namespace jp {

class Document;
class Editor;

// The fonts the document names, most used first, then the font scheme's two fonts (when the text uses them), with how many places of content use each (a font only the styles name is
// listed with no uses).
QVector<FontUse> pageFontsUsed(const Document &doc);
// Puts `to` in the place of `key` (a family; "+Heading" or "+Body" for the font scheme's own) everywhere in the document, without any record for Undo (the editor's change() makes
// it one step). How many places changed.
int replacePageFont(Document &doc, const QString &key, const QString &to);

// The box for a window of an editor over such a document: asks, and puts the font in the place of the other as one step of Undo (the editor's change). `theme`, when set, does the work of
// a pick of "+Heading" or "+Body" for an app whose theme is more than the document's font scheme (it gets the document, the key, and the font, and says how many places changed).
// The result says what was done; the window shows its message.
ReplaceFontsResult askReplacePageFonts(QWidget *parent, Editor *ed, const std::function<int(Document &, const QString &key, const QString &to)> &theme = nullptr);

} // namespace jp
