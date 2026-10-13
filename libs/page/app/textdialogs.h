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
void dateTimeDialog(QWidget *p, Editor *ed);
void hyperlinkDialog(QWidget *p, Editor *ed);
QImage proceduralTexture(const QString &name, int size);   // built-in texture fills
bool fillEffectsDialog(QWidget *p, Editor *ed, Fill &fill, const QString &title = QString());
void wordCountDialog(QWidget *p, Editor *ed);
void tintsDialog(QWidget *p, Editor *ed, const std::function<void(const ColorRef &)> &apply);
void textArtTextDialog(QWidget *p, Editor *ed, const QString &itemId);

} // namespace jp
