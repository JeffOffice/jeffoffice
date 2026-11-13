#pragma once
// Where the programs' data files are: the folder beside the programs that every app of the suite shares, installed once and not built
// into each program: Help's topics (help/), the translations (i18n/), data.stamp (the version: the mark that makes a folder this one,
// so a folder of someone else's named help or i18n never counts), and manifest.txt (every file of Help and every translation the
// build made, one relative path a line: the lists the programs show come from it, so a file left by an older version is ignored).

#include <QString>
#include <QStringList>

namespace jp {

// The data folder's subfolder ("help", "i18n"), or an empty string when there is no data folder. One folder is chosen for both, once
// per process: the first of these that holds data.stamp with this version: beside the program, in data/ beside it, in ../share/jeffoffice
// (the Linux packages), in ../Resources (a macOS app's), and, in a development build only, the build tree's data folder.
QString dataFolder(const QString &subfolder);

// The manifest's paths under `subfolder` (relative to it: "en/printing.md", "jeffoffice_de.qm"); empty when there is no data folder.
QStringList dataManifest(const QString &subfolder);

// Looks only in this folder (which needs the stamp and the manifest too), none of the others; an empty string goes back to the search. For tests.
void setDataFolderOverride(const QString &root);

} // namespace jp
