#pragma once
// Typing in a shape: the one editor every app puts over a shape or text box that is being typed in (JeffWrite and JeffCrunch host it; JeffPub and JeffDeck, which edit
// a shape's words on their own canvas, share its keys and its font steps). It edits a story (a QTextDocument) as rich text: typing, selection, the caret keys,
// copy, cut, and paste (plain and rich), bold, italic, underline, font, size, color, alignment, bullets, the input method, and the spelling marks the host gives it.
// The host makes the story from the shape's own storage, shows the editor over the shape's frame, and writes the story back when the editor says it is finished
// (as one step of its own undo history: what was typed, and every format, is one step).

#include "core/export.h"

#include <QColor>
#include <QMarginsF>
#include <QPair>
#include <QTextEdit>
#include <QVector>
#include <functional>
#include <memory>

class QKeyEvent;

namespace jp {

// How the shape sets its words.
struct ShapeTextLook {
    Qt::Alignment anchor = Qt::AlignTop;   // where the block of words sits in the frame: AlignTop, AlignVCenter, or AlignBottom
    bool wrap = true;                      // lines break at the frame's width
    bool autofit = false;                  // the words shrink to fit (the editor keeps the caret in view; the host's page shows the shrunk words)
    QMarginsF insets = QMarginsF(7.2, 3.6, 7.2, 3.6);   // in pixels of the editor
    double zoom = 1;                       // the pixels of the editor to the points of the story (sizes shown larger or smaller; the story keeps its points)
};

// What a key does when a shape is selected and no words are being edited: the same in every app.
enum class SelectedShapeKey { None, Begin, BeginTyped };
JO_COMMON_EXPORT SelectedShapeKey keyOnSelectedShape(const QKeyEvent *e, QString *typed);
// The next size in the list of sizes (Ctrl+] up, Ctrl+[ down) from `points`.
JO_COMMON_EXPORT double nextFontSize(double points, int direction);

class JO_COMMON_EXPORT ShapeTextEdit : public QTextEdit {
    Q_OBJECT
public:
    explicit ShapeTextEdit(QWidget *parent = nullptr);
    ~ShapeTextEdit() override;

    // Begins editing `story` (kept by the editor) in `frame`, in the parent's pixels. `typed` is what was typed on the selected shape to start the edit: it goes at the end
    // of the words (or replaces them, when `replace`).
    void begin(std::unique_ptr<QTextDocument> story, const QRect &frame, const ShapeTextLook &look, const QString &typed = QString(), bool replace = false);
    // The story, edited; and whether anything in it changed since begin().
    QTextDocument *story() const { return document(); }
    bool changed() const { return m_changed; }
    // Ends the edit: the story is taken from the editor (the host writes it back); `finished` is sent once.
    std::unique_ptr<QTextDocument> take();

    // Formats (of the selection, or of what is typed next; of the word at the caret where nothing is selected).
    void setBold(bool on);
    void setItalic(bool on);
    void setUnderline(bool on);
    void toggleBold() { setBold(!charFormat().font().bold()); }
    void toggleItalic() { setItalic(!charFormat().font().italic()); }
    void toggleUnderline() { setUnderline(!charFormat().font().underline()); }
    void setFontFamilyName(const QString &family);
    void setFontSizePt(double points);
    void growFont(int direction);
    void setColor(const QColor &color);
    void setAlign(Qt::Alignment alignment);
    QTextCharFormat charFormat() const { return currentCharFormat(); }
    // The size at the caret in the story's points (0 where it has none of its own).
    double fontSizePt() const;
    // Bullets on or off for the paragraphs of the selection, and a level in or out.
    void setBullets(bool on);
    bool hasBullets() const;
    void indent(int direction);

    // The misspelled words of the story, each as its first position and its end (the host's check, asked as the words change); they are underlined.
    using SpellCheck = std::function<QVector<QPair<int, int>>(QTextDocument *story)>;
    void setSpellCheck(SpellCheck check);
    // Bullets made by the host (its own list model): called with the cursor and whether to turn them on; without it the editor makes plain lists.
    using BulletMaker = std::function<void(QTextCursor &, bool on)>;
    void setBulletMaker(BulletMaker maker) { m_bullets = std::move(maker); }
    int spellMarks() const { return m_marks; }

Q_SIGNALS:
    // Esc: the host takes the story and writes it (`keep` is true: Esc keeps the words, as it does in every app).
    void finished(bool keep);
    // The focus went to another place (a click on the page, another control): the host decides whether that ends the edit.
    void focusLeft();
    // A word was typed or a format was changed (for a host that shows the state of its ribbon).
    void formatChanged();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void insertFromMimeData(const QMimeData *source) override;

private:
    void refit();
    void marksChanged();
    void mergeFormat(const QTextCharFormat &f);

    ShapeTextLook m_look;
    SpellCheck m_check;
    BulletMaker m_bullets;
    bool m_changed = false, m_done = false, m_refitting = false;
    int m_marks = 0;
    std::unique_ptr<QTextDocument> m_story;
};

} // namespace jp
