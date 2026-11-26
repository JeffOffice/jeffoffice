#pragma once
// A view for words that belong to no object on a page (the speaker notes under a slide, and, in the same way, the notes boxes of other views): a text editor that shows
// a story's own document and does no editing of its own. Every change to the words goes through the Editor as it does for a text box on the page: its text commands (the
// ribbon's bold, italic, lists, link, and the rest), AutoCorrect, paste, cut, and Undo, each one step in the presentation's one undo history, typing grouped as it is
// for a text box. The view only shows the words and the caret, moves the caret and the selection (keys, mouse), and keeps them the same as the Editor's (Editor::cursor()).
// Where there is no story yet it shows nothing; the host makes it (before an edit) with setStory.

#include "core/export.h"
#include "core/document.h"

#include <QTextEdit>
#include <QTextOption>
#include <memory>

namespace jp {

class Editor;
class MiniTarget;
class MiniToolbar;

class JO_COMMON_EXPORT StoryEdit : public QTextEdit {
    Q_OBJECT
public:
    explicit StoryEdit(Editor *ed, QWidget *parent = nullptr);
    ~StoryEdit() override;

    // Shows the story with this id (empty, or one the document does not have: nothing); the host calls it again when the document may have put another document of the same
    // story in its place (an undo, a redo), and the view then shows that one, with the caret where it was as far as the words go. The view holds the story it shows, so an undo
    // that replaces the document's stories leaves it a story to show until then. Cheap when nothing changed.
    void setStory(const QString &storyId);
    QString storyId() const { return m_id; }
    // The Editor's text edit is in this story now (the commands of the ribbon and the keys act on it).
    bool isTarget() const;
    // Puts the Editor's text edit here, at the view's caret: false when the presentation is read-only or there is no story.
    bool takeTarget();
    // Typed words are closed into one undo step (as a text box's are when the caret leaves or typing pauses).
    void flushTyping();
    bool hasTyping() const;

    // The marks under the words: misspelled ones (red, wavy), and, where the host has them, others (a color and a style for each range, [start, end)).
    struct Mark {
        int start = 0, end = 0;
        QColor color;
        QTextCharFormat::UnderlineStyle style = QTextCharFormat::WaveUnderline;
    };
    void setExtraMarks(const QVector<Mark> &marks);
    // Words of the story marked as misspelled by the Editor (the word being typed is left out), as marks.
    QVector<Mark> spellingMarks() const;

    // The bar of the commands the selection can take, shown by the words a mouse selection ends on (made when first asked for). `target` is the host's own commands.
    MiniToolbar *setMiniTarget(std::unique_ptr<MiniTarget> target);
    MiniToolbar *miniBar() const { return m_mini; }

Q_SIGNALS:
    // Typing is about to start here (a key, or the box got the keyboard).
    void typingStarted();
    void storyShown();   // another story (or another document of the same one) is shown now

protected:
    void keyPressEvent(QKeyEvent *e) override;
    bool event(QEvent *e) override;
    bool viewportEvent(QEvent *e) override;
    void inputMethodEvent(QInputMethodEvent *e) override;
    void insertFromMimeData(const QMimeData *source) override;
    bool canInsertFromMimeData(const QMimeData *source) const override;
    void dropEvent(QDropEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    // Where the host has no menu of its own: the editing commands.
    void contextMenuEvent(QContextMenuEvent *e) override;
    // The id of a story made for this view when words are about to be put in and it has none (the host puts it where it keeps its stories); empty: it cannot.
    virtual QString makeStory() { return QString(); }
    // The host adds to the menu (above the editing commands); the position is of the click in the story.
    virtual void fillMenu(QMenu &, int) {}

private:
    void bind(const std::shared_ptr<Story> &story);
    void followEditor();   // the Editor's caret moved: the view's does too
    void followView();     // the view's caret moved: the Editor's does too
    void markWords();
    Editor *m_ed;
    QString m_id;
    std::shared_ptr<Story> m_story;   // (kept: the document may drop its own while this view still shows the words)
    QTextDocument *m_none = nullptr;   // what is shown when there is no story
    QSizeF m_page;                     // the page size and text option the shown document had before this view laid it out
    QTextOption m_option;
    bool m_syncing = false;
    QVector<Mark> m_marks;
    MiniToolbar *m_mini = nullptr;
    std::unique_ptr<MiniTarget> m_miniTarget;
};

} // namespace jp
