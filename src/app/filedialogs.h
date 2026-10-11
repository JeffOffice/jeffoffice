#pragma once
// The file dialogs every app uses (filedialogs.cpp).

#include "core/pictureinsert.h"

#include <QString>
#include <QStringList>

class QWidget;

namespace jp {

// The Insert Picture and Change Picture dialog: the files chosen (none when
// cancelled) and, in `how` unless it is null, whether they are inserted,
// linked to their files, or both. `several` allows more than one file.
QStringList askPicturePaths(QWidget *parent, const QString &caption, const QString &dir, const QString &filter, bool several, PictureInsert *how);
// Extra advice after a failed save (Windows: folders protected from unknown apps).
QString saveFailureHint(const QString &error);
// The save dialog every Save As and Export command uses.
QString askSavePath(QWidget *parent, const QString &caption, const QString &suggested,
                    const QString &filter = QString(), QString *selectedFilter = nullptr);

} // namespace jp
