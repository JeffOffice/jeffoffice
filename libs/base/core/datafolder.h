#pragma once
// Where the programs' data files are: the folders beside the programs that every app of the suite shares (Help's
// topics in help/, the translations in i18n/, and the fonts and dictionaries), installed once and not built
// into each program.

#include <QString>

namespace jp {

// The folder that holds `subfolder` ("help", "i18n"), or an empty string when there is none. Looked for, in this order:
// beside the program, in ../share/jeffoffice (the Linux packages), in ../Resources (a macOS app's), and then in the
// source and build trees this program was built from (development builds and the tests).
QString dataFolder(const QString &subfolder);

// Looks only in this folder (the one that holds help/ and i18n/), none of the others; an empty string goes back to the search. For tests.
void setDataFolderOverride(const QString &root);

} // namespace jp
