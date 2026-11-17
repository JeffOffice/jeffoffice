#pragma once
// Where the programs' data files are: the folder beside the programs that every app of the suite shares, installed once and not built
// into each program: Help's topics (help.rcc), the translations (i18n/), data.stamp (the version: the mark that makes a folder this one,
// so a folder of someone else's named help or i18n never counts), and manifest.txt (help.rcc, every file inside Help and every translation the
// build made, one relative path a line: the lists the programs show come from it, so a file left by an older version is ignored).

#include <QString>
#include <QStringList>

namespace jp {

// Help is not a folder of files but one compressed Qt resource file, help.rcc (prefix /help, the language folders under it as before): the folder
// found registers it once, and dataFolder("help") is then ":/help": a path that QFile, QDir, and QFileInfo read like any other. With no such file, or a corrupt one, there is no Help here. For writing Help, the environment variable JO_HELP_DIR names a folder that stands in for Help's (used only when it is set).
//
// The data folder's subfolder ("help", "i18n"), or an empty string when there is no data folder. One folder is chosen for both, once
// per process: the first of these that holds data.stamp with this version: beside the program, in data/ beside it, in ../share/jeffoffice
// (the Linux packages), in ../Resources (a macOS app's), and, in a development build only, the build tree's data folder.
QString dataFolder(const QString &subfolder);

// The manifest's paths under `subfolder` (relative to it: "en/printing.md", "jeffoffice_de.qm"); empty when there is no data folder.
QStringList dataManifest(const QString &subfolder);

// Counts the times the search was made (once per process, and again after each setDataFolderOverride): a bundle's folder is ":/help" every time,
// so what a caller keeps from reading it is kept with this number.
int dataFolderSearches();

// Looks only in this folder (which needs the stamp and the manifest too), none of the others; an empty string goes back to the search. For tests.
void setDataFolderOverride(const QString &root);

} // namespace jp
