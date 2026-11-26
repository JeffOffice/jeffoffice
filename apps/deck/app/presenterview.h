#pragma once
// Presenter View: what the presenter sees while the audience watches the
// slide show. The slide on the screen, large; the next slide, smaller, under
// "Next"; the slide's notes, in a size the presenter chooses (A+ and A-); a
// timer that has run since the show started (it can pause, resume, and
// reset); the clock; "Slide N of M"; Previous and Next; Black Screen; See All
// Slides, a grid to jump to any slide; and End Slide Show. The show drives it
// and its controls call the show; keys work in either window and a click on
// the large slide moves on. It shows the slide as the audience sees it, an
// effect partway through included, and Next shows what the next click gives.
// "Pen and Pointer" opens the show's pointer menu; with the pen, the
// highlighter, or the eraser in hand, the large slide takes strokes, and the
// laser pointer's dot follows the mouse over it, for the audience's screen too. The notes can be typed in; "Swap" trades the two screens.

#include <QElapsedTimer>
#include <QImage>
#include <QPointer>
#include <QTimer>
#include <QWidget>

class QAbstractButton;
class QLabel;
class QListWidget;
class QTextEdit;
class QTextDocument;
class QStackedWidget;

namespace jp {

class PresenterNotes;
class SlideShow;
class SlideView;

class PresenterView : public QWidget {
    Q_OBJECT
public:
    // Closes when the show ends.
    explicit PresenterView(SlideShow *show);
    ~PresenterView() override;

    // "Slide 2 of 12", or "End of slide show".
    QString counterText() const;
    QString notesText() const;
    // The notes are shown formatted, as they are in the notes under the slide, and typed here through the window's editor (the same typing, commands, and one history of Undo: typed
    // words are one step when typing pauses, when the box is left, when the show moves to another slide, and when the show ends; Bold, Italic, and Underline keys work). The box is the
    // one view of the notes' story while Presenter View is up. A show with no window's editor shows them formatted and read only. Esc in the box goes back to the keys of the show.
    void commitNotes();
    // What the next click shows, under Next: the slide (the one on the screen,
    // while it has steps left to play), -1 for none, how many of its steps have
    // played, and its picture.
    int nextSlide() const { return m_next; }
    int nextSteps() const { return m_nextSteps; }
    QImage nextImage() const;

    int notesPointSize() const { return m_notesSize; }
    void setNotesPointSize(int pt);

    // The timer: "07:45", or "1:02:05" past an hour.
    QString timerText() const;
    qint64 timerElapsed() const;
    void setTimerElapsed(qint64 ms);
    bool isTimerPaused() const { return m_paused; }
    void resetTimer();

    bool isShowingAllSlides() const;
    void showAllSlides(bool on);
    // Moves the show to a slide and back to the usual view.
    void jumpTo(int slide);

public Q_SLOTS:
    // Reads where the show is again.
    void refresh();
    // Draws the large slide again: a frame of an effect.
    void frame();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void closeEvent(QCloseEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    QAbstractButton *button(const QString &name, const QString &text);
    void buildGrid();
    void tick();
    void leaveNotes();   // the keyboard goes back to the show
    void setPaused(bool paused);
    QPointer<SlideShow> m_show;
    SlideView *m_current = nullptr, *m_nextView = nullptr;
    QLabel *m_counter = nullptr, *m_timerLabel = nullptr, *m_clock = nullptr;
    QTextEdit *m_notes = nullptr;
    PresenterNotes *m_rich = nullptr;   // m_notes, when the show has an editor to edit the notes through
    QPointer<QTextDocument> m_copy;     // otherwise: what the box shows, a copy of the story
    QString m_copyKey;
    bool m_held = false;
    void releaseNotes();                // lets the story go (the window's own views have it again)
    void applyNotesSize();
    void showCopy(const QTextDocument *story, const QString &key);
    QStackedWidget *m_pages = nullptr;
    QListWidget *m_grid = nullptr;
    QAbstractButton *m_pause = nullptr, *m_black = nullptr, *m_all = nullptr, *m_pointerButton = nullptr, *m_swap = nullptr;
    QTimer m_notesIdle;
    QString m_notesPage;       // the page the box's text is of
    QTimer m_tick;
    QElapsedTimer m_since;     // when the timer last started running
    qint64 m_before = 0;       // what it had counted before that
    bool m_paused = false;
    int m_next = -1, m_nextSteps = 0;
    int m_notesSize = 20;
};

} // namespace jp
