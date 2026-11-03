#pragma once
// The file handling every app's File screen shares: how a file is opened (as it is, read-only, as a copy, in a new window, or with repair), the backup copy kept
// when a file is saved over, and My Templates (the folder, the thumbnails, the panel the New page shows).

#include <QImage>
#include <QString>
#include <QStringList>
#include <functional>

class QMenu;
class QWidget;

namespace jp {

enum class OpenHow {
    Normal,      // as the app has always opened a file
    ReadOnly,    // for reading: nothing in it can be changed, and Save asks for a new name
    Copy,        // the file's contents as a new, unsaved document; the file itself is never written
    NewWindow,   // in a window of its own, even when this window holds a new document nobody has touched
    Repair,      // a damaged file: what can be read is kept, and what was dropped is said
};

// Whether two paths are one file (links followed; the capitals of a name count only where the system counts them; a hard link is the same file). Either may not be there yet.
bool sameFile(const QString &a, const QString &b);

// ---- Backup copy kept on every save (Options: "Always create backup copy"; the setting "save/backup", kept for each app)
bool backupOnSave();
// "Backup of <name>", beside the file.
QString backupPathFor(const QString &path);
enum class BackupResult {
    NotNeeded,   // the option is off, or there is no file there to keep
    Made,        // the file is copied to backupPathFor(path)
    Failed,      // the copy could not be made (the backup that was there before is as it was)
};
// With the option on and the file there already: copies it to backupPathFor(path), before a save writes over it. The copy is written under a name of its own and put in place when it is whole,
// so a copy that fails leaves the old backup as it was.
BackupResult keepBackupBeforeSave(const QString &path);
// What a save does before it writes over `path`: keeps the backup, and when that fails asks whether to save without one (the answer is No unless the person says Yes). True when the save may
// go on. A test stands in for the question with a function that gives the answer (set to an empty function: asked as usual).
bool backupBeforeSave(QWidget *parent, const QString &path);
void setBackupFailureStandIn(const std::function<bool(const QString &path)> &standIn);

// ---- My Templates: the files a person saved as templates, in the app's data folder
QString myTemplatesFolder();
// The templates there whose names match `nameFilters` ("*.dotx"), by name.
QStringList myTemplateFiles(const QStringList &nameFilters);
// The path a new template called `name` would have, `ending` including the dot (".dotx"); "" when the name has nothing usable in it. A name's characters that no file name may
// have are left out.
QString myTemplatePath(const QString &name, const QString &ending);
// Asks for the template's name (suggesting `suggested`) and gives the path in My Templates, "" when cancelled. A test stands in for the question (set to an empty function: asked
// as usual).
QString askMyTemplatePath(QWidget *parent, const QString &suggested, const QString &ending);
void setTemplateNameStandIn(const std::function<QString(const QString &suggested)> &standIn);
// A picture of the template, kept beside it as <name>.png, for the New page; none is "" or null.
bool saveTemplateThumbnail(const QString &templatePath, const QImage &picture);
QImage templateThumbnail(const QString &templatePath);
// Takes the template (and its picture) out of My Templates.
bool removeMyTemplate(const QString &templatePath);

// The My Templates part of a New page: its tiles (the picture, else the app's icon, over the name), each starting a document with `create(path)`; right-click removes one. With
// none saved, it says how to save one.
QWidget *myTemplatesPanel(QWidget *parent, const QStringList &nameFilters, const QString &iconName, const std::function<void(const QString &path)> &create);

// ---- The other ways to open
// The words for each way, and the menu of them (Open Read-Only, Open as Copy, Open in New Window, Open and Repair) that Browse and the recent lists show.
QString openHowText(OpenHow how);
void addOpenWays(QMenu *menu, const std::function<void(OpenHow how)> &choose);
// A card for the Open page: "More Ways to Open", a menu of those ways under it.
QWidget *moreWaysToOpenCard(QWidget *parent, const std::function<void(OpenHow how)> &choose);

} // namespace jp
