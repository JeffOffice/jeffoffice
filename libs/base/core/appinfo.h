#pragma once
// Who the running program is. The app fills this in once at startup, before
// anything else runs; the shared libraries read it wherever they name the
// program: in messages, file and folder names, settings, and the addresses
// of its update server and issue tracker.

#include <QColor>
#include <QString>

namespace jp {

struct AppInfo {
    QString name;           // shown to people: "JeffPub"
    QString fileName;       // in file, folder, and variable names, lower case: "jeffpub"
    QString organization;   // owns its settings and data folders: "JeffOffice"
    QString repository;     // on GitHub: "JeffOffice/jeffoffice"
    QColor accent;          // the app's color on the light interface: "#9E1F63"
    QColor accentDark;      // its shade on the dark interface, light enough to read there
    QString badge;          // two letters in the circle on the File button: "JP"
    QColor highlight;       // the dark interface's selection color, under white text (invalid: accentDark darkened)
    QColor link;            // the dark interface's link color (invalid: accentDark lightened)
    QString helpFolder;     // where its Help topics are under each language ("deck": resources/help/en/deck/); empty: the language's own folder
    bool online = true;     // checks for updates and sends usage statistics (when chosen); JeffDeck's preview does neither
};

const AppInfo &appInfo();
void setAppInfo(const AppInfo &info);

} // namespace jp
