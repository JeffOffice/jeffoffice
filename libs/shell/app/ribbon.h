#pragma once
// The ribbon: quick toolbar, File button, tabs (including colored
// contextual tabs), and groups of large/small buttons with dialog launchers.

#include <QFontMetrics>
#include <QFrame>
#include <QMap>
#include <QPointer>
#include <QToolButton>
#include <functional>

class QAction;
class QMenu;
class QHBoxLayout;
class QVBoxLayout;
class QStackedWidget;
class QScrollArea;
class QLabel;

namespace jp {

// The KeyTip of a ribbon control, group, launcher or tab: the one or two
// letters or digits that reach it from the keyboard. It is a dynamic property
// of the widget so the KeyTip display can read it straight from the tree.
void setKeytip(QObject *w, const QString &k);
QString keytip(const QObject *w);

class RibbonGroup : public QFrame {
    Q_OBJECT
public:
    explicit RibbonGroup(const QString &title, QWidget *parent = nullptr);
    QToolButton *addLarge(QAction *a, QMenu *menu = nullptr, bool split = false);
    void beginColumn();
    QToolButton *addSmall(QAction *a, QMenu *menu = nullptr, bool split = false, bool iconOnly = false);
    void addRow(const QList<QWidget *> &widgets);   // a row of widgets in the current column
    // A column of rows whose cells line up: the first row heads the columns, the first cell of each other row names it.
    void addGrid(const QList<QList<QWidget *>> &rows);
    void addWidget(QWidget *w);                       // its own column
    void addSeparator();
    void setLauncher(const std::function<void()> &fn, const QString &tip = QString());
    QToolButton *launcher() const { return m_launcher; }
    QString title() const { return m_title; }
    // What the group is called in code and in the person's customizing (the ribbon file's name for it, not its translation); a group the person made has "user:" and a number.
    void setId(const QString &id) { m_id = id; }
    QString id() const { return m_id; }
    // A group the person turned off (Options > Customize): it takes no room and shows nothing, and its commands still exist.
    void setUserHidden(bool hidden);
    bool userHidden() const { return m_userHidden; }
    // Every control the group lays out, in order (rows and columns
    // flattened; the launcher sits outside the layout and isn't one).
    QList<QWidget *> controls() const;
    // At least as wide as its title: a translated title can be wider than
    // the controls under it ("TABELLEN" over one button).
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *e) override;
    void changeEvent(QEvent *e) override;   // a new font: the rows take their new height

private:
    QFont titleFont() const;   // small capitals, as drawn
    int titleWidth() const;    // the group's width that shows the whole title
    QHBoxLayout *m_cols;
    QVBoxLayout *m_col = nullptr;
    int m_rows = 0;
    QList<QToolButton *> m_colToggles;   // the small toggles of the current column
    QList<QLabel *> m_rowLabels;   // the labels that start the rows of the current column
    QString m_title, m_id;
    bool m_userHidden = false;
    QToolButton *m_launcher = nullptr;
    QMetaObject::Connection m_launcherClicked;
};

// A ribbon page. When the window is too narrow for every group, groups
// collapse from the right into one button each that opens the group in a
// dropdown, so every command stays reachable.
class RibbonTab : public QWidget {
    Q_OBJECT
public:
    explicit RibbonTab(QWidget *parent = nullptr);
    RibbonGroup *addGroup(const QString &title);
    QList<RibbonGroup *> groups() const
    {
        const_cast<RibbonTab *>(this)->ensureBuilt();
        return m_groups;
    }
    void finish();   // (again after groups were added to a finished tab)
    // Takes a group away (one the person made), with its folded button.
    void removeGroup(RibbonGroup *g);
    // A tab can be made empty and filled the first time it is shown (or asked for its groups): `fn` makes the groups, and `built()` follows.
    void setBuilder(std::function<void(RibbonTab *)> fn) { m_builder = std::move(fn); }
    bool isBuilt() const { return !m_builder && !m_attach; }
    void ensureBuilt();
    // A tab shown just now (made late, or the window just resized) has its layout done, and its groups folded to the room there is, before anything reads which controls show.
    void settle();
    QSize minimumSizeHint() const override;
    int collapsedCount() const;
    // The single button each group shows when there's no room.
    QToolButton *collapsedButton(int i) const { return m_buttons.value(i); }
    bool isCollapsed(int i) const { return m_collapsed.value(i); }
    void popUp(int i);   // a collapsed group in a dropdown

Q_SIGNALS:
    void built();        // the groups of a tab made with setBuilder are there

protected:
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *e) override;

private:
    void relayout();
    QHBoxLayout *m_layout;
    bool m_finished = false;
    std::function<void(RibbonTab *)> m_builder;
    std::function<void()> m_attach;   // (a deferred tab's place in the ribbon, made with its controls)
    friend class Ribbon;
    QVector<RibbonGroup *> m_groups;
    QVector<QToolButton *> m_buttons;   // collapsed form of each group
    QVector<bool> m_collapsed;
    int m_popped = -1;                  // group currently shown in a dropdown
    bool m_inRelayout = false;
};

class RibbonHeader;
class HeaderButton;

class Ribbon : public QWidget {
    Q_OBJECT
public:
    explicit Ribbon(QWidget *parent = nullptr);
    // `name` identifies the tab in code (tab(), setContextVisible()); `title`
    // is what the header shows, the name translated (the name when empty).
    // A `deferred` tab (one to be made late with RibbonTab::setBuilder) is only a page until it is first shown or asked for its groups.
    RibbonTab *addTab(const QString &name, const QString &contextGroup = QString(), const QColor &color = QColor(),
                      const QString &title = QString(), bool deferred = false);
    // With `first`, the group's tabs come before all the others, right after
    // File, while shown, and the first of them is the current tab when they
    // appear (the Master Page tab does this).
    void setContextVisible(const QString &group, bool visible, bool first = false);
    void setTabVisible(const QString &name, bool visible);   // any tab, by name
    // A tab the person turned off (Options > Customize), apart from the program's own showing and hiding of it; not for a contextual tab or Home (refused: false).
    bool setTabUserHidden(const QString &name, bool hidden);
    bool tabUserHidden(int i) const { return m_tabs.value(i).userHidden; }
    int tabBase(int i) const { return m_tabs.value(i).base; }   // its place as the tabs were made
    int tabHome(int i) const { return m_tabs.value(i).home; }   // its place now (tests: no two tabs share one)
    // The tab each command that opens a menu from its button is on (from the ribbon file, so that it is known before the tab is made): the builder notes it.
    void setMenuCommandTabs(const QHash<QString, QString> &tabs) { m_menuTabs = tabs; }
    QString menuCommandTab(const QString &id) const { return m_menuTabs.value(id); }
    void setTabTitle(const QString &name, const QString &title);
    // The tabs that are always there (not contextual) in the order `names` gives, the others keeping their places; a name not listed follows those that are, in its own order. An empty list is the order
    // the tabs were made in.
    void arrangeTabs(const QStringList &names);
    // Takes a tab away (one the person made).
    void removeTab(const QString &name);
    void showTab(RibbonTab *t);
    RibbonTab *current() const;
    RibbonTab *tab(const QString &name) const;
    QToolButton *addQuickAccess(QAction *a, QMenu *menu = nullptr, bool split = false);
    void addQuickAccessSeparator();
    // What one place of the quick toolbar holds: a command (with the menu it opens, if it has one) or, with no action, a separator.
    struct QuickEntry {
        QAction *action = nullptr;
        QMenu *menu = nullptr;
        bool split = false;
        QString keytip;
    };
    // The quick toolbar as `entries` have it (the first call keeps what the ribbon file made, for defaultQuickAccess()).
    // (`moreKeytip`: the letters of the » button that holds the commands that don't fit)
    void setQuickAccess(const QList<QuickEntry> &entries, const QString &moreKeytip = QString());
    void resetQuickAccess();   // back to the ribbon file's
    QList<QuickEntry> defaultQuickAccess() const;
    QList<QToolButton *> quickAccessHidden() const;   // the buttons that don't fit the row: they are in the » menu
    QToolButton *quickAccessMore() const;
    int quickAccessLayoutCount() const;   // how many times the quick toolbar was laid out (tests)
    void setMinimized(bool m);
    bool isMinimized() const { return m_minimized; }
    // The tabs in order, for KeyTips and tests.
    int tabCount() const { return int(m_tabs.size()); }
    // (the page of a tab not shown yet may still be empty: groups() makes them, as does showTab())
    RibbonTab *tabAt(int i) const { return m_tabs.value(i).page; }
    QString tabName(int i) const { return m_tabs.value(i).name; }
    QString tabTitle(int i) const { return m_tabs.value(i).title; }
    QString tabContextGroup(int i) const { return m_tabs.value(i).group; }   // empty for a tab that is always there
    QString tabKeytip(int i) const { return m_tabs.value(i).keytip; }
    bool tabVisible(int i) const { return m_tabs.value(i).visible && !m_tabs.value(i).userHidden; }
    RibbonTab *tabPage(int i) const { return m_tabs.value(i).page; }
    QWidget *tabButton(int i) const;   // the tab's control in the top row
    QWidget *fileButton() const;
    QRect brandRect(QFont *font = nullptr) const;   // where the program's name is drawn in the top row (and its font), in the ribbon's coordinates
    QWidget *moreButton() const;   // the chevron that holds the tabs that don't fit (null while all do)
    void setTabKeytip(RibbonTab *t, const QString &k);
    // The File button isn't a tab; it only records its KeyTip.
    QString fileKeytip() const { return m_fileKeytip; }
    void setFileKeytip(const QString &k) { m_fileKeytip = k; }
    QList<QToolButton *> quickAccessButtons() const;
    // Everything the ribbon holds, one control per line: tabs, groups, each
    // control's kind, command and menu, in order (tests compare it). With
    // `keytips`, each line that has a KeyTip ends with it.
    QString describe(bool keytips = false) const;

    void buildAllTabs();      // every tab made with a builder gets its groups now
    void focusCurrentTab();   // the keyboard enters the ribbon (F6)
    void restyle();           // the colors, again for the current look

Q_SIGNALS:
    void fileClicked();
    void helpClicked();       // the "?" beside the collapse chevron
    void tabChanged();
    void tabBuilt(RibbonTab *t);   // a tab made lazily has its controls now (before it shows)
    void tabsBuilt();              // after one or more tabs were made: once, from the event loop (what shows the window's state in their controls is to be brought up to date)
    void leaveRequested();    // Escape: back to the page

protected:
    void keyPressEvent(QKeyEvent *e) override;
    bool event(QEvent *e) override;   // a new font or screen: the body's height again

private:
    void updateBodyHeight();
    friend class RibbonHeader;
    friend class HeaderButton;
    struct Tab {
        QString name, group;
        QColor color;
        RibbonTab *page = nullptr;
        QScrollArea *scroll = nullptr;
        bool visible = true;
        QString title, keytip;
        int home = 0;         // its place as built, or as the person arranged it
        bool first = false;   // placed before the rest
        bool userHidden = false;
        int base = 0;         // its place as built
    };
    void sortTabs();
    QScrollArea *makeScroll(RibbonTab *page);
    void attachPage(RibbonTab *page);
    QWidget *m_pending = nullptr;   // hidden: the parent of the pages of tabs not yet made
    QVector<Tab> m_tabs;
    QHash<QString, QString> m_menuTabs;
    struct QatDefault {
        QPointer<QAction> action;
        QPointer<QMenu> menu;
        bool split = false;
        QString keytip;
    };
    QList<QatDefault> m_qatDefaults;   // the quick toolbar as the ribbon file made it (once it has been replaced)
    bool m_qatSnapshot = false;
    int m_current = 0;
    RibbonHeader *m_header;
    QStackedWidget *m_stack;
    QList<QAction *> m_qat;
    QString m_fileKeytip;
    bool m_minimized = false;
    bool m_builtQueued = false;
};

// The height of a small button and of every row of controls beside it (22 at the usual font, taller for a larger one): three rows fit the ribbon's body.
constexpr int kMinRowHeight = 22;
int ribbonRowHeight(const QFontMetrics &fm);
// The side of a large button's icon.
constexpr int kLargeIcon = 30;
QToolButton *ribbonButton(QAction *a, bool large, QWidget *parent);
bool isRibbonToggle(const QToolButton *b);   // a small button for a checkable command
// A check box (the box alone) for a checkable command, for a ribbon grid.
QToolButton *ribbonCheck(QAction *a, QWidget *parent);
bool isRibbonCheck(const QToolButton *b);
// Drawing shared by every large ribbon button (icon over a label wrapped to two lines).
QSize largeRibbonButtonHint(const QToolButton *b);
// The lines a large button's label is drawn on (one or two).
QStringList largeRibbonLabelLines(const QToolButton *b);
void paintLargeRibbonButton(QToolButton *b);

} // namespace jp
