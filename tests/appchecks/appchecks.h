// What the checks of every app's window share (tests/appchecks): the same three checks run on JeffPub, JeffDeck, JeffWrite, and JeffCrunch, each with a small adapter that says how that app
// is launched, how its document is typed into, and what its state is.
//
//   1. The first five minutes: launched with no file, as a person does, the app takes typing, undoes it, saves in its own format, reopens the file, opens a second window with File >
//      New, shows the print layout, and closes without a question when nothing is changed. (Each app's own file: it knows where its text goes.)
//   2. The standard commands: file.new ... edit.redo exist, are reachable from the ribbon or the File screen, have their usual shortcuts, and do something in a fresh document.
//   3. Every command: each registered action is run once on a blank document and once on one opened from a file; none crashes, warns, says "not yet" where it should work, or
//      (enabled) does nothing a person could see.
//
// Everything waits on conditions, never on time (the ceilings only bound a wait that has failed).
#pragma once

#include "app/appwindow.h"

#include <QKeySequence>
#include <QPointer>
#include <QRegularExpression>
#include <QStringList>
#include <QUrl>
#include <QtTest>
#include <deque>
#include <functional>
#include <memory>

class QAction;

namespace appchecks {

// A window of the app being watched: a stretch of a test in which every dialog and menu that opens is counted and closed (a test never waits on a person), every Qt warning is kept, and
// every address the program would send to the system's browser is kept instead.
class Watch : public QObject {
    Q_OBJECT
public:
    Watch();
    ~Watch() override;
    // Forgets what was seen so far.
    void begin();
    // Closes the windows that opened since begin() (not the ones given), and says how many there were.
    int closeNewWindows(const QList<QWidget *> &keep);
    // Dialogs and menus that opened, by class and text ("QMessageBox: Do you want to save...").
    QStringList dialogs;
    QStringList warnings;
    QStringList urls;
    // The files the save dialog's stand-in gave a name to (a file a command wrote is a change a person can see in the folder).
    QStringList saved;
    // The filters ("Name (*.ext)") the last save dialog offered, and which of them the stand-in picks (0 is the first): a command is run again with each.
    QStringList offered;
    // The files in the test's folder and the program's data folder when a save dialog was first asked (to see what the command then wrote, wherever the kind of file puts it).
    QStringList beforeWrite;
    bool asked = false;
    int pickFilter = 0;
    // The words of notices (message boxes that only tell, not ask).
    QStringList notices;
    // The words of every message box and label of a dialog that opened, for finding "not yet" in them.
    QStringList said;
    // The words the program said through its own notifier (a test sets it for the window).
    QStringList told;
    bool eventFilter(QObject *o, QEvent *e) override;

public Q_SLOTS:
    void urlOpened(const QUrl &url);
};

// The "can't do that yet" sentences the apps have said; a fresh document must never show one.
bool saysNotYet(const QString &text);

// How an app is driven. Every function is set by the app's own file.
struct Adapter {
    QString name;   // "JeffPub"
    // A window as launching the app with no file makes it: shown, with whatever it has to read or work out finished.
    std::function<jp::AppWindow *()> launch;
    // A window holding the sample file (a new window, shown, the file read and everything worked out).
    std::function<jp::AppWindow *(const QString &path)> launchWith;
    // The sample file (the app's own format, made by the app or kept in tests/data), set up by the test's initTestCase.
    QString sampleFile;
    // The state of the document and the selection, and of the view (zoom, the cell with the cursor...), as text: what a person could see change.
    std::function<QString(jp::AppWindow *)> docState;
    // Types a few words into the document, as a person does: the cells of a sheet, the text of a page.
    std::function<void(jp::AppWindow *)> type;
    // Waits until the window has nothing going on.
    std::function<void(jp::AppWindow *)> settle;
    // Puts the app's message boxes of "told" kind (the notifier) into the watch.
    std::function<void(jp::AppWindow *, Watch *)> listen;
    // The commands that do nothing a test can see, by design, with the reason for each ("id" for both states, "id@blank" or "id@file" for one). A command listed here is still run (it must not crash or warn).
    QHash<QString, QString> quiet;
    // The commands that are never run in either state (dimmed in every place a test can put the window in), with the reason for each. The run checks that no other is left out.
    QHash<QString, QString> neverRun;
    // The commands that have been run in the states so far (kept by the runs, for the check of what is never run).
    QSet<QString> ran;
    QStringList allIds;
    // Brings a window that a command has been run in back to how it was launched (its document replaced, the panes and screens it
    // opened put away, what the View commands switch put right), when that is quicker than making a new window; not set for an app whose window can't be put back. A window a command
    // closed or opened more windows from is made again.
    std::function<void(jp::AppWindow *, bool fromFile)> reset;
    // A place a person can be in besides the window as launched (a title selected, words selected, a picture selected): some commands are dimmed until then, and are run there.
    struct Context {
        QString name;
        std::function<void(jp::AppWindow *, bool fromFile)> enter;
        bool fileOnly = false;   // the places that need what the sample file has
    };
    QList<Context> contexts;
    // Where to try a command first (a start of ids, and the names of the places that make it work): the other places are tried after these, the ones where commands have done most first.
    QHash<QString, QStringList> hints;
    // Commands to leave out of check 3, with the reason (a command that ends the program, or hands the computer to another program).
    QHash<QString, QString> skipped;
    // Puts the window where a person would be to use the command: text selected for a format, text on the clipboard for Paste. `fromFile` says which state it is.
    std::function<void(jp::AppWindow *, const QString &id, bool fromFile)> prepare;
};

// The ways a window shows what a person can see of it, as text: its title, the text of its labels, buttons, and fields, which of its parts are shown, and the state of its commands
// (except `skip`'s, the one being run). Two equal fingerprints mean nothing visible changed.
QString fingerprint(jp::AppWindow *w, const QAction *skip = nullptr);
bool differByMoreThanACaret(const QImage &a, const QImage &b);

// Waits (on the event loop) until `done` is true, for at most `ceiling` ms.
bool waitFor(const std::function<bool()> &done, int ceiling = 20000);

// Runs the events that are waiting, and those they make, until there are none.
void drain();

// Windows made a few at a time: what a window finishes by itself after it is shown (a count of words, a first layout) is done by timers, and the windows wait for them together.
class Pool {
public:
    Pool(Adapter &app, bool fromFile, int batch = 8) : m_app(app), m_fromFile(fromFile), m_batch(batch) {}
    // A window shown and settled (null if the app could not make one).
    std::unique_ptr<jp::AppWindow> take();

private:
    Adapter &m_app;
    bool m_fromFile;
    int m_batch;
    std::deque<std::unique_ptr<jp::AppWindow>> m_ready;
};

// ---- check 2
struct Standard {
    QString id;
    QString what;                       // for the messages
    QList<QKeySequence> keys;           // the shortcuts it must have (any of them)
    QString label;                      // how the File screen names it, if it is there
};
// The standard commands and their usual shortcuts (the standard keys of the system; F12 for Save As).
QList<Standard> standardCommands();
// Whether the command can be reached from the ribbon (a button or a menu of it) or the File screen; how, if so.
QString reachableFrom(jp::AppWindow *w, QAction *a, const QString &label);
// Runs check 2 on a window as launched; failures are returned as lines. `extraKeys` replaces what an app does differently (its id and the keys it uses).
QStringList checkStandardCommands(Adapter &app, const QHash<QString, QList<QKeySequence>> &keysOf = {}, const QHash<QString, QString> &labelOf = {});

// ---- check 3
// What a call did to a window, as a person would have seen it.
struct Outcome {
    bool changed = false;
    QString detail;         // where the text it changed first differs (for the report)
    QString refusal;        // what it said instead, when it only refused (a notice, or the status line)
    QString how;            // what changed ("opened a dialog", "changed the document"...), empty if nothing did
    QString words;          // everything it said: message boxes, the notifier, the status line
    QStringList warnings;
    QStringList dialogs;
    QStringList offered;    // the kinds of file a save dialog offered
    int windows = 0;        // windows it opened (closed again)
};
// Runs `doIt` and watches the window: it waits (on conditions, to `ceiling` ms) for something to change unless `quiet`. What counts as a change is what a person could act on: a dialog that asks, a file
// written, a window opened or closed, the clipboard, a setting turned over, the document, the selection, the view, or what the window shows. A refusal (a notice, or a message in the status line, with
// nothing else changed) and a redraw are not. `skip` is the command being run, whose own check mark is not a change.
Outcome observe(Adapter &app, jp::AppWindow *w, const QAction *skip, Watch &watch, const std::function<void()> &doIt, bool quiet = false, int ceiling = 1000);
QString quietBecause(const Adapter &app, const QString &id, bool fromFile, bool *wildcard = nullptr);
// Runs every enabled command of the window the adapter launches, a window for each, and returns what failed. `fromFile` shows the sample file's window in place of the blank one.
// `report` gets a line per command (what it did).
QStringList runEveryAction(Adapter &app, bool fromFile, QStringList *report);
// After both states have been run: the commands that no state ran, each of which must have a reason in `neverRun`.
QStringList checkNeverRun(Adapter &app, QStringList *report);

// Shared by the first-five-minutes tests: a window that was closed, and whether it asked.
struct CloseResult {
    bool closed = false;
    int asked = 0;
};
CloseResult closeWindow(jp::AppWindow *w, Watch &watch);

// What a file in the zip form (.docx, .pptx, .xlsx) holds, as a short text that is the same for the same content: its parts' names and bytes, but not the document properties, which
// carry the time they were made.
QString zipDigest(const QByteArray &zip);

// Types the text into the widget key by key (a new line is the Return key).
void typeInto(QWidget *target, const QString &text);

// The files of the test's folder and the program's data folder, with their sizes.
QStringList filesNow();

// Whether the picture has something drawn on it (more than one color).
bool hasDrawnContent(const QPixmap &picture);

// Whether a File screen's Open page lists the file among the recent ones (a list whose entries carry the file's path).
bool listsRecentFile(QWidget *screen, const QString &path);

// A folder for the test's files.
QString tempFile(const QString &name);

// The settings and data this program's run changes are its own; the test starts from nothing.
void isolateSettings();

} // namespace appchecks
