#pragma once
// Dialog boxes for text, lists, symbols, links, fills, and tables that every
// app with pages shares (they work on an Editor).

#include "core/style.h"

#include <QImage>
#include <QString>
#include <QStringList>
#include <functional>

class QWidget;

namespace jp {

class Editor;
class TextArtItem;

void fontDialog(QWidget *p, Editor *ed);
void paragraphDialog(QWidget *p, Editor *ed, int tab = 0);
void bulletsDialog(QWidget *p, Editor *ed, bool numbering);
void characterSpacingDialog(QWidget *p, Editor *ed);
void insertTableDialog(QWidget *p, Editor *ed);
// Symbols: insert one (in a font, or the text's own when empty) at the text
// cursor, remembering it; the recently used ones, newest first ("char\tfont").
bool insertSymbol(QWidget *p, Editor *ed, const QString &ch, const QString &font);
QStringList recentSymbols();
void symbolDialog(QWidget *p, Editor *ed);
// The same dialog for a program whose text is not an Editor's: `insertOne` puts the symbol in (true when it did), and calls rememberSymbol.
void rememberSymbol(const QString &ch, const QString &font);
void symbolDialog(QWidget *p, const std::function<bool(QWidget *, const QString &, const QString &)> &insertOne);
// Paste Special: a list of the forms the clipboard can go in as (source formatting, text only, picture); pastes the one chosen.
void pasteSpecialDialog(QWidget *p, Editor *ed);
void dateTimeDialog(QWidget *p, Editor *ed);
void hyperlinkDialog(QWidget *p, Editor *ed);
QImage proceduralTexture(const QString &name, int size);   // built-in texture fills
bool fillEffectsDialog(QWidget *p, Editor *ed, Fill &fill, const QString &title = QString());
void wordCountDialog(QWidget *p, Editor *ed);
void tintsDialog(QWidget *p, Editor *ed, const std::function<void(const ColorRef &)> &apply);
void textArtTextDialog(QWidget *p, Editor *ed, const QString &itemId);
// The same box for a Text Art that is not a page's item: asks for the words, font, size, bold, and italic of `art` and sets them in it when the box is taken (false if cancelled).
bool askTextArtText(QWidget *p, TextArtItem *art);

} // namespace jp
