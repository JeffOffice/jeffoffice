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

#include <QElapsedTimer>
#include <QImage>
#include <QPointer>
#include <QTimer>
#include <QWidget>

class QAbstractButton;
class QLabel;
class QListWidget;
class QPlainTextEdit;
class QStackedWidget;

namespace jp {

class SlideShow;
class SlideView;

class PresenterView : public QWidget {
    Q_OBJECT
public:
    // Closes when the show ends.
    explicit PresenterView(SlideShow *show);

    // "Slide 2 of 12", or "End of slide show".
    QString counterText() const;
    QString notesText() const;
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
    void setPaused(bool paused);
    QPointer<SlideShow> m_show;
    SlideView *m_current = nullptr, *m_nextView = nullptr;
    QLabel *m_counter = nullptr, *m_timerLabel = nullptr, *m_clock = nullptr;
    QPlainTextEdit *m_notes = nullptr;
    QStackedWidget *m_pages = nullptr;
    QListWidget *m_grid = nullptr;
    QAbstractButton *m_pause = nullptr, *m_black = nullptr, *m_all = nullptr;
    QTimer m_tick;
    QElapsedTimer m_since;     // when the timer last started running
    qint64 m_before = 0;       // what it had counted before that
    bool m_paused = false;
    int m_next = -1, m_nextSteps = 0;
    int m_notesSize = 20;
};

} // namespace jp
