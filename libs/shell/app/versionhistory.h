#pragma once
// The Version History box of the Info page, the same in every app: the versions kept for the file (app/versionstore.h), a button to keep one now, to open a copy of one, to go back to one,
// and to delete one (and to compare one with the file, for an app that can). The app says what keeping, opening, and going back are for its file.

#include "app/versionstore.h"

#include <QString>
#include <functional>

class QObject;
class QWidget;

namespace jp {

struct VersionHistoryHost {
    enum class Kind { Presentation, Publication, Document, Workbook };
    Kind kind = Kind::Presentation;
    QString path;        // the file's path (the versions belong to it)
    QString extension;   // the ending of the copies' files
    // Keeps the file as it is now under that name. False: it was not kept. An app that keeps it later calls `refresh` (which lists the versions again) when the version is in the list, and not
    // after `dialog` is gone; one that keeps it at once calls it before it returns.
    std::function<bool(const QString &name, QObject *dialog, const std::function<void()> &refresh)> keep;
    std::function<void(const QString &copyPath)> openCopy;   // opens a kept copy (by the path of its file) as a copy of its own
    std::function<bool(const QString &file)> restore;     // goes back to the version of that file name
    std::function<void(const QString &file)> compare;     // compares the version of that file name with the file; none: no Compare button
};

// The text the box says when the file has no path yet (keeping a version needs one).
QString versionsNeedAFile(VersionHistoryHost::Kind kind);

void showVersionHistory(QWidget *parent, const VersionHistoryHost &host);

} // namespace jp
