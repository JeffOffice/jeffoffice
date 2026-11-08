#pragma once
// The Version History box of the Info page for the apps that keep versions of a file in a list and nothing more: the versions kept for the file (app/versionstore.h), a button to keep
// one now, to open a copy of one, to go back to one, and to delete one. The app says what keeping and going back are for its file.

#include "app/versionstore.h"

#include <QString>
#include <functional>

class QWidget;

namespace jp {

struct VersionHistoryHost {
    enum class Kind { Presentation, Publication };
    Kind kind = Kind::Presentation;
    QString path;        // the file's path (the versions belong to it)
    QString extension;   // the ending of the copies' files
    std::function<bool(const QString &name)> keep;        // keeps the file as it is now under that name (an empty name: the box makes one)
    std::function<void(const QString &copyPath)> openCopy;   // opens a kept copy (by the path of its file) as a copy of its own
    std::function<bool(const QString &file)> restore;     // goes back to the version of that file name
};

// The text the box says when the file has no path yet (keeping a version needs one).
QString versionsNeedAFile(VersionHistoryHost::Kind kind);

void showVersionHistory(QWidget *parent, const VersionHistoryHost &host);

} // namespace jp
