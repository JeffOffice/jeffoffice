#pragma once
// The Clipboard pane every app has (opened from the launcher of the Clipboard group on the Home tab): the last 24 things copied or cut in this run of the program, newest first, each with a
// small preview and its kind. A click puts the item on the clipboard (an ordinary copy of it) and pastes it as the app's own Paste does, so it is one step of Undo and the Paste Options
// button comes as usual. The history itself, its bounds, and what it never keeps are in clipboardhistory.h. What a paste is, and how the keyboard goes back to the document, is the app's:
// it gives the pane a Host.

#include "app/clipboardhistory.h"
#include "core/export.h"

#include <QWidget>
#include <functional>

class QCheckBox;
class QDockWidget;
class QLabel;
class QListWidget;
class QMainWindow;
class QPushButton;
class QUndoStack;

namespace jp {

// Ends the step Paste All began (QUndoStack::beginMacro) and, when nothing was pasted into it, takes the empty step out again, so that a group that changed nothing adds no step to Undo and leaves the
// document as unchanged as it was.
JO_COMMON_EXPORT void endPasteSteps(QUndoStack *stack);

struct ClipboardPaneHost {
    std::function<void()> paste;       // the app's own Paste, on what the clipboard holds now
    std::function<void()> focusBack;   // gives the keyboard back to what is being edited (unset: nothing)
    std::function<void()> beginSteps;  // Paste All is made one step of Undo between these two (unset: each item is a step; every app gives these)
    std::function<void()> endSteps;
};

class JO_COMMON_EXPORT ClipboardPane : public QWidget {
    Q_OBJECT
public:
    ClipboardPane(const ClipboardPaneHost &host, QWidget *parent = nullptr);
    ~ClipboardPane() override;
    QListWidget *list() const { return m_list; }
    // The list again from the history.
    void refresh();
    void pasteItem(quint64 id);   // puts the item on the clipboard and pastes it
    void pasteAll();              // the oldest first, so that they read in the order they were copied
    void clearAll();
    void deleteCurrent();
    quint64 idAt(int row) const;
    // Counts the pane in the history as open (it collects while one is), or not.
    void setShown(bool shown);
    QCheckBox *collectClosedBox() const { return m_closedBox; }
    QCheckBox *collectOthersBox() const { return m_othersBox; }
    QPushButton *pasteAllButton() const { return m_pasteAll; }
    QPushButton *clearAllButton() const { return m_clearAll; }
    QPushButton *deleteButton() const { return m_delete; }
    QLabel *statusLabel() const { return m_status; }
    QLabel *noteLabel() const { return m_note; }
    // The name of a kind of item, as the pane shows it.
    static QString kindName(ClipboardHistory::Kind kind);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void contextMenu(const QPoint &at);
    void updateStatus();
    ClipboardPaneHost m_host;
    QListWidget *m_list = nullptr;
    QLabel *m_empty = nullptr, *m_status = nullptr, *m_note = nullptr;
    QPushButton *m_pasteAll = nullptr, *m_delete = nullptr, *m_clearAll = nullptr;
    QCheckBox *m_closedBox = nullptr, *m_othersBox = nullptr;
    bool m_counted = false;
};

// The pane in a dock of `win` (made on first use, hidden until shown): the name and icon are the same in every app. Shows it when asked; the app calls this from its launcher and its command.
JO_COMMON_EXPORT QDockWidget *clipboardDock(QMainWindow *win, const ClipboardPaneHost &host);
JO_COMMON_EXPORT void showClipboardPane(QMainWindow *win, const ClipboardPaneHost &host);
// The key tip of the launcher, the same in every app (a test checks each ribbon uses it).
JO_COMMON_EXPORT QString clipboardLauncherKeytip();

} // namespace jp
