#pragma once
// Commands implemented in their own files (sharing.cpp, proofing.cpp,
// mainwindow_ribbon.cpp) and used from the window, backstage and task panes.
// Declared here at namespace scope: block-scope declarations inside lambdas
// resolve to the global namespace on some compilers and fail to link.

#include "app/filedialogs.h"
#include "app/proofingdialogs.h"
#include "app/spelling.h"
#include "core/style.h"

#include <QString>
#include <QPair>
#include <QStringList>
#include <QVector>

class QImage;
class QTextDocument;
class QWidget;

namespace jp {

class Document;
class Editor;
class MainWindow;
enum class PictureInsert;

void emailCurrentPage(QWidget *parent, Editor *ed);
void emailAsAttachment(QWidget *parent, Editor *ed, const QString &format);
// A mail header value from a name: one line, non-ASCII encoded (RFC 2047).
QString emlHeaderText(const QString &s);
void packAndGo(QWidget *parent, MainWindow *win, bool forPrinter);
// Where Save as Building Block keeps the blocks, which Insert > Page Parts lists.
QString userBlocksDir();
// Pack and Go's printer folder: a commercial press PDF, the publication, and a note.
// False, with `error` saying what, when any of them could not be saved.
bool packForPrinter(MainWindow *win, const QString &dir, QString *error = nullptr);
// Pack and Go's ZIP for another computer: the publication with its pictures whole, and the
// font files it uses. `fonts` gets how many; false, with `error`, when it could not be saved.
bool packToZip(MainWindow *win, const QString &path, int *fonts, QString *error);
void saveForPhotoPrinter(QWidget *parent, MainWindow *win);
void saveAsTemplate(QWidget *parent, MainWindow *win);
Stroke currentBorderStroke();
int hyphenateStory(QTextDocument *doc);                   // returns hyphens inserted
QStringList pressProblems(const Document &d);              // what a printer would object to (before PDF/X)

} // namespace jp
