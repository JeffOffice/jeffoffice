#pragma once
// The speaker notes under the slide. What is typed goes into the slide's
// notes story as one undo step once typing pauses, or on leaving the pane.

#include <QPlainTextEdit>
#include <QTimer>

namespace jp {

class Editor;

class NotesPane : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit NotesPane(Editor *ed, QWidget *parent = nullptr);
    void reload();   // shows the current slide's notes, unless typing waits to be saved
    void commit();   // saves what was typed into the slide
    bool hasTyping() const { return m_typed; }

protected:
    void focusOutEvent(QFocusEvent *e) override;

private:
    Editor *m_ed;
    QTimer m_idle;
    QString m_pageId;
    bool m_loading = false;
    bool m_typed = false;   // typed since the last save or load
};

} // namespace jp
