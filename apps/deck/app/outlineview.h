#pragma once
// The Outline view: the presentation's text as an outline. Each slide is a
// numbered heading line holding its title (in bold) and, under it, the lines
// of its body indented by level. Typing edits the slide's title and body
// placeholders as one undo step once typing pauses (as the notes do), or when
// the focus or a command leaves. Enter on a title makes a new slide after it,
// Tab and Shift+Tab move a line a level in or out (a title becomes a line of
// the slide before, a first-level line a slide of its own), and the current
// slide follows the cursor. Only the title and the first body placeholder of a
// slide show here; the other objects on a slide are left as they are.

#include <QTextEdit>
#include <QTimer>

namespace jp {

class Editor;

class OutlineView : public QTextEdit {
    Q_OBJECT
public:
    explicit OutlineView(Editor *ed, QWidget *parent = nullptr);
    // Shows the deck's text, unless typing waits to be saved.
    void reload();
    // Saves what was typed into the slides.
    void commit();
    bool hasTyping() const { return m_typed; }
    // The slide the cursor is in, or -1.
    int slideAtCursor() const;
    // Puts the cursor at the start of the slide's title.
    void goToSlide(int slide);

protected:
    bool event(QEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void showEvent(QShowEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void insertFromMimeData(const QMimeData *source) override;

private:
    struct Where;
    struct Entry;
    Where whereIs(const QTextCursor &c) const;
    void moveTo(const Where &w, bool reveal = true);
    QVector<Entry> fromDeck() const;
    QVector<Entry> parse() const;
    void rebuild(const QVector<Entry> &model);
    void apply(const QString &label);
    void settle(const QString &label);
    void textTyped();
    void cursorMoved();
    void pageChanged();
    void newSlideAfterTitle();
    void indent(bool deeper);

    Editor *m_ed;
    QTimer m_idle;
    bool m_typed = false;      // typed since the last save or load
    bool m_loading = false;    // the view is being filled in or marked, not typed in
    bool m_applying = false;   // typing is being saved into the slides
    bool m_following = false;  // the cursor moved the current slide
    bool m_dirty = true;       // the deck changed while the view was hidden
};

} // namespace jp
