#pragma once
// The Styles pane, on the right of the window (Home > Styles, Ctrl+Alt+Shift+S): the document's styles with their looks, which of them the text uses, the one the caret is in marked,
// and a choice of which to list (those in use, all, or those recommended). A click gives a paragraph style to the paragraphs selected (a character style, to the characters); the
// buttons make a style from the selection and change the one chosen.

#include "core/styleops.h"

#include <QTimer>
#include <QWidget>

class QComboBox;
class QListWidget;
class QPushButton;

namespace jp {

class WriteWindow;

class StylesPane : public QWidget {
    Q_OBJECT
public:
    explicit StylesPane(WriteWindow *win, QWidget *parent = nullptr);
    // Brings the list up to date with the document (the styles, which the text uses, and the style the caret is in); nothing is done while it is.
    void refresh();
    QListWidget *list() const { return m_list; }
    QComboBox *filterBox() const { return m_filter; }
    write::StyleFilter filter() const;
    void setFilter(write::StyleFilter filter);
    // The row of a style by its id ("p:Heading1" for a paragraph style, "c:Strong" for a character style); -1 where it is not listed.
    int rowOf(const QString &key) const;
    // The key of the style the row is for, and of the style the caret is in.
    QString keyAt(int row) const;
    QString currentKey() const { return m_caretKey; }
    // Gives the row's style to the selection (what a click does).
    void apply(int row);
    // The style of the row chosen, or else of the caret.
    QString chosenId() const;
    // How many times the whole list has been made (for tests of what typing and moving the caret cost).
    int rebuildCountForTest() const { return m_rebuilds; }

private:
    void rebuild();
    void recheckUse();
    void mark(const QString &key);
    WriteWindow *m_win;
    QComboBox *m_filter;
    QListWidget *m_list;
    QPushButton *m_new, *m_modify;
    QString m_caretKey, m_markedKey;
    // What the list was made from (it is made again only when these change; the caret's style is a mark on a row, which moves).
    int m_madeFilter = -1;
    quint64 m_madeStyles = ~quint64(0);
    size_t m_madeUsed = 0;
    int m_usedAtRevision = -1;
    const void *m_madeDoc = nullptr;
    int m_rebuilds = 0;
    QTimer m_idle;
};

} // namespace jp
