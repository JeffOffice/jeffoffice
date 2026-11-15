#pragma once
// The command search: a box in the ribbon's top row (Alt+Q puts the cursor in it) that lists, as one types, the commands, File screen entries, and Options pages that fit the words, and runs the one
// chosen. It reads the app's ribbon (tabs, groups, buttons, and the menus under them), the File screen, and a short list of Options pages, so it is the same in every app; each app gives only a data
// file of extra words by command (resources/cmdsearch.json):
//
//   { "commands": { "<command id>": ["extra words", ...] },
//     "file":     { "<File screen key>": ["extra words"] },
//     "options":  { "<Options page, as the source names it>": ["extra words"] } }
//
// A page of Options inside another is written "Customize/Keyboard". The words are English (the app's translated names are matched as well as the English ones).

#include "core/export.h"
#include <QHash>
#include <QIcon>
#include <QLineEdit>
#include <QPointer>
#include <QStringList>

class QAction;
class QListWidget;
class QToolButton;
class QFrame;

namespace jp {

class AppWindow;
class Ribbon;

namespace cmdsearch {

// Text for matching: lower case, accents taken off ("Café" and "cafe" are the same).
QString fold(const QString &text);
// The words of a text, folded.
QStringList tokens(const QString &text);

// What a result is: a ribbon command (or an item of a menu under one), an entry of the File screen, a page of Options, or the row that searches Help.
enum class Kind { Command, File, Options, Help, Gallery };

struct Entry {
    Kind kind = Kind::Command;
    QString id;      // "cmd:edit.paste", "file:print", "options:Save" (what the recent list keeps)
    QString name;    // as shown
    QString where;   // "Home > Paragraph"
    QString tip;
    QStringList english;   // the English name and tip, when the app shows another language
    QStringList extra;     // the words the app's data file adds
    QIcon icon;
    QString key;            // a File screen entry's key, an Options page's name
    QPointer<QAction> action;
    QPointer<QToolButton> button;   // the ribbon button the command is on (a command with a menu opens it)
    QPointer<QWidget> gallery;      // a gallery item's gallery (a Gallery or a GalleryButton), and its id there
    QString itemId;
    int tab = -1;                   // the ribbon tab it is on
    bool opensMenu = false;         // Enter opens its menu rather than running it
};

using Words = QHash<QString, QStringList>;   // "cmd:<id>", "file:<key>", "options:<page>" to extra words
// The extra words in a data file's text (nothing for what is malformed).
Words parseWords(const QByteArray &json);

// How well `e` fits `query` (every word of it must fit some word of the entry: whole, as the start of one, or within one wrong letter), the higher the better; -1 for no fit.
int score(const Entry &e, const QString &query);
// The entries that fit, best first (no more than `limit`).
QList<Entry> find(const QList<Entry> &all, const QString &query, int limit = 10);
// Every command of the window's ribbon (also on tabs that show only at times, and in menus), the File screen's entries, and the Options pages, in that order.
QList<Entry> collect(AppWindow *win, const Words &words);
// The most gallery items listed in one window (a gallery can hold thousands: the search stays fast).
constexpr int kMaxGalleryItems = 2000;
// Where the list goes: under the box, its right edge with the box's, kept on the screen (all in global coordinates; `size` the list's wanted size).
QRect popupRect(const QRect &box, const QSize &size, const QRect &screen);
// Whether the entry can be used now (a command on a tab that is not shown, or one that is turned off, cannot).
bool available(const Entry &e, AppWindow *win);

} // namespace cmdsearch

class JO_COMMON_EXPORT CommandSearchBox : public QLineEdit {
    Q_OBJECT
public:
    // The box for `win`'s ribbon (it puts itself in the ribbon's top row and takes Alt+Q for the window); `wordsResource` is the app's data file of extra words (":/write/cmdsearch.json").
    CommandSearchBox(AppWindow *win, const QString &wordsResource);
    // The cursor goes into the box, with the recent commands listed.
    void focusBox();
    // The results as listed now: the rows' entries, those that are headings or the Help row not among them.
    QList<cmdsearch::Entry> shownResults() const;
    QListWidget *list() const { return m_list; }
    // The last commands run from the box, newest first (at most five).
    QStringList recent() const;
    static constexpr int kRecent = 5;
    // Runs the chosen row (as Enter does).
    void runCurrent();
    // Back to the document: the list is put away, the text cleared.
    void leave();
    static QString recentKey();
    // The screen the list is placed on, in place of the window's own (a test's offscreen screen is 640 by 480): an empty rectangle puts it back.
    static void setScreenForTests(const QRect &screen);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    void refill();
    void run(const cmdsearch::Entry &e);
    void move(int delta);
    void place();
    void ensureIndex();
    void setWide(bool wide);
    struct Row {
        cmdsearch::Entry entry;
        bool help = false;
    };
    AppWindow *m_win;
    QPointer<Ribbon> m_ribbon;
    QString m_resource;
    QList<cmdsearch::Entry> m_all;
    bool m_indexed = false;
    QFrame *m_popup;
    QListWidget *m_list;
    QList<Row> m_rows;
};

} // namespace jp
