#pragma once
// The file dialogs every app uses (filedialogs.cpp).

#include "core/pictureinsert.h"

#include <QString>
#include <QStringList>
#include <functional>

class QWidget;
class QPrintDialog;

namespace jp {

// The Insert Picture and Change Picture dialog: the files chosen (none when
// cancelled) and, in `how` unless it is null, whether they are inserted,
// linked to their files, or both. `several` allows more than one file.
QStringList askPicturePaths(QWidget *parent, const QString &caption, const QString &dir, const QString &filter, bool several, PictureInsert *how);
// Extra advice after a failed save (Windows: folders protected from unknown apps).
QString saveFailureHint(const QString &error);
// Stands in for the save dialog (a test sets it; the program never does): given the dialog's title, the name it would suggest, its filters, and where the filter chosen goes (null when the
// caller does not ask), the path to save to, or nothing to give the save up. Set to an empty function, the dialog is shown again.
void setSavePathStandIn(const std::function<QString(const QString &caption, const QString &suggested, const QString &filter, QString *selectedFilter)> &standIn);
// Asks with a print dialog and says how it ended (QDialog::Accepted or Rejected). The system's print dialog needs a screen and a person, so on the offscreen display (the tests) it is
// not shown: a test stands in for it (setPrintDialogStandIn), and with none the answer is Rejected.
int execPrintDialog(QPrintDialog &dialog);
void setPrintDialogStandIn(const std::function<int(QPrintDialog &)> &standIn);
// The save dialog every Save As and Export command uses.
QString askSavePath(QWidget *parent, const QString &caption, const QString &suggested,
                    const QString &filter = QString(), QString *selectedFilter = nullptr);

} // namespace jp
