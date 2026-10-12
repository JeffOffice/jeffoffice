#pragma once
// Who the running program is. The app fills this in once at startup, before
// anything else runs; the shared libraries read it wherever they name the
// program: in messages, file and folder names, settings, and the addresses
// of its update server and issue tracker.

#include <QString>

namespace jp {

struct AppInfo {
    QString name;           // shown to people: "JeffPub"
    QString fileName;       // in file, folder, and variable names, lower case: "jeffpub"
    QString organization;   // owns its settings and data folders: "JeffOffice"
    QString repository;     // on GitHub: "JeffOffice/jeffoffice"
};

const AppInfo &appInfo();
void setAppInfo(const AppInfo &info);

} // namespace jp
