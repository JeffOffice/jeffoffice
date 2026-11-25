#pragma once
// Save Each as a File, the box every app with a mail merge shares: where the files go, how each is named (from the recipient's own details), and what kind of file it is; then the questions
// before a big or replacing batch, and the box that shows the files being made, with Cancel.

#include "core/mergeextras.h"

#include <QString>
#include <QVector>
#include <functional>

class QWidget;

namespace jp {

struct MergeFilesChoice {
    QString folder, pattern;
    bool pdf = false;
};

// What an app can make for each recipient: its own kind of file (`ownLabel`, as the box shows it, and `ownExtension` with its dot) and, where `canPdf`, a PDF.
struct MergeFilesKinds {
    QString ownLabel, ownExtension;
    bool canPdf = true;
};

// The box. `rows` are the recipients the files are made for (the names shown are theirs). True where the person chose Save; `choice` then has the folder, the pattern, and the kind.
bool mergeFilesDialog(QWidget *p, const MergeSource &m, const QVector<int> &rows, const MergeFilesKinds &kinds, const QString &startFolder, MergeFilesChoice *choice);
// The questions before the run: past kAskAboveMergeFiles files how many will be made, and, once for the whole batch, that some files are already in the folder and will be replaced.
// False where the person says no (nothing is written). More than kMostMergeFiles files is refused here, with the reason.
bool confirmMergeFiles(QWidget *p, const QString &folder, const QStringList &names);
// Makes the files with a box that has Cancel (it stops between files), then says how many were saved, and how many before it stopped, or why it could not go on. Returns the result.
MergeFilesResult runMergeFiles(QWidget *p, const QString &folder, const QVector<int> &rows, const QStringList &names, const MakeMergeFile &make);

} // namespace jp
