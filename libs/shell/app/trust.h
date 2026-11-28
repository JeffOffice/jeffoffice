#pragma once
// The Security choices of Options (the same in every app): what happens with linked files and the links of a workbook, with linked movies and sounds, and with a file that came from the
// internet or another computer. Macros are not a choice: they are kept in the file and never run. A link that would start a program is not one either: it is always refused, and nothing here
// reaches that, or the refusal of a file on another computer (office/hyperlink.h, core/remotepath.h); a choice only ever lets a link do what the person says to, among the files that may be followed.

#include "app/filetasks.h"
#include "core/export.h"

#include <QObject>
#include <QString>
#include <QWidget>
#include <functional>

class QComboBox;
class QLabel;
class QPushButton;

namespace jp::trust {

// What to do with something that reaches out of the file: ask first, do it, or never.
enum class Choice { Ask, Yes, Never };

// Linked pictures (JeffPub) and the workbooks a workbook reads (JeffCrunch): what is done when the file opens. Kept for each app, since the two are not the same thing (`app` is an AppInfo
// fileName; empty for the running app). JeffPub updates the pictures in the publication's own folder, as it always did, until the person chooses "Never" (it never asks: nothing else is
// read); JeffCrunch asks, until the person chooses.
JO_COMMON_EXPORT Choice linkedFiles(const QString &app = QString());
JO_COMMON_EXPORT void setLinkedFiles(Choice choice, const QString &app = QString());
// The files a workbook may read without asking, once this person has said yes to that workbook and that file (JeffCrunch): kept in this person's own settings only, never in a workbook. Each entry is a pair, the
// workbook's full path and the full path of the file it reads, both as the system names them once links are worked out. Not a list of names: each path is kept as a keyed hash (HMAC-SHA-256, cut to 128 bits)
// with a random key made for this person and kept beside them, so the settings file does not tell a casual reader which files this person has. At most kMostApprovals pairs are kept; when
// another comes, the one used longest ago goes. A path longer than kLongestApprovedPath is never remembered (it is asked about each time), and an entry that is not of the form written is dropped.
constexpr int kMostApprovals = 2000;
constexpr int kLongestApprovedPath = 4096;
// Whether this pair was approved (and, when it was, counts as used just now). An empty path, a too long one, or no pair: false.
JO_COMMON_EXPORT bool readApproved(const QString &workbook, const QString &file);
JO_COMMON_EXPORT void rememberRead(const QString &workbook, const QString &file);
JO_COMMON_EXPORT int approvalCount();
// Forgets every pair, and tells every open window (ApprovalNotifier::forgotten) to forget the yes it keeps in memory, so that the next read in any of them asks again.
JO_COMMON_EXPORT void forgetApprovals();
class JO_COMMON_EXPORT ApprovalNotifier : public QObject {
    Q_OBJECT
public:
    static ApprovalNotifier &get();
Q_SIGNALS:
    void forgotten();
};
// Movies and sounds that a slide links to a file instead of holding (JeffDeck). Never, until the person chooses.
JO_COMMON_EXPORT Choice linkedMedia();
JO_COMMON_EXPORT void setLinkedMedia(Choice choice);
// A file from the internet or another computer opens for reading first (the default), or as any file does.
JO_COMMON_EXPORT bool outsideFilesReadOnly();
JO_COMMON_EXPORT void setOutsideFilesReadOnly(bool on);

// Whether the file came from the internet (the system marked it when it was downloaded) or is on another computer (a path that begins with two slashes). Only the file's own mark and name are
// looked at; a file on another computer is not touched.
JO_COMMON_EXPORT bool fromOutside(const QString &path);
// A test stands in for the system's mark of a download (set to an empty function: the system is asked).
JO_COMMON_EXPORT void setOutsideStandIn(const std::function<bool(const QString &path)> &standIn);
// How a file is opened: a normal open of a file from outside is a read-only open when the choice says so. `held` says it was; the other ways to open (a copy, a repair, read-only) stay as asked.
JO_COMMON_EXPORT OpenHow openHowFor(const QString &path, OpenHow how, bool *held = nullptr);
// What the bar under a file held back says, for every app.
JO_COMMON_EXPORT QString heldBackText();
// "Enable Editing" on that bar: this file, until the program closes, opens as any file does (a window that has it open is made ordinary by the caller).
JO_COMMON_EXPORT void allowEditing(const QString &path);
JO_COMMON_EXPORT bool editingAllowed(const QString &path);
// Takes the system's download mark off the file (a save in place of a file whose editing was enabled does this). False when there was none or it could not be taken off.
JO_COMMON_EXPORT bool clearOutsideMark(const QString &path);

// A question the choices put to the person ("Ask"); true for yes. A test stands in for it (set to an empty function: the question is asked as usual). It is passed the question's text.
JO_COMMON_EXPORT bool ask(QWidget *parent, const QString &title, const QString &question, const QString &detail = QString());
JO_COMMON_EXPORT void setAskStandIn(const std::function<bool(const QString &question)> &standIn);

// Which of the choices an app has something to apply them to (the rows of the others say they don't apply).
struct Applies {
    bool linkedFiles = false, linkedMedia = false;
    bool linkedFilesAsks = false;   // the app has a question to put for its linked files (JeffPub reads only what is in the publication's folder, so has none)
    QString app;
    QString linkedFilesAbout, linkedMediaAbout;   // what the row is about, in the app's own words (empty when it does not apply)
};
JO_COMMON_EXPORT Applies appliesTo(const QString &appFileName);

// The page of Options, the same in every app: the rows above, and the two lines that say what is never done.
class JO_COMMON_EXPORT TrustPage : public QWidget {
    Q_OBJECT
public:
    explicit TrustPage(const Applies &applies, QWidget *parent = nullptr);
    // After OK: the choices are kept.
    void apply();

private:
    QComboBox *m_files = nullptr, *m_media = nullptr, *m_outside = nullptr;
    QString m_filesApp;
    QLabel *m_approvals = nullptr;
    QPushButton *m_forget = nullptr;
    bool m_filesAsks = false;
};

} // namespace jp::trust
