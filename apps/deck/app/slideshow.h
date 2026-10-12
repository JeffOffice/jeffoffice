#pragma once
// The slide show: the slides full screen, one at a time. Right, Down, Space,
// Enter, Page Down, N, or a click go forward; Left, Up, Backspace, Page Up,
// or P go back; Home and End jump to the first and last; a number and Enter
// go to that slide; B and W blank the screen black or white; Esc ends it.
// Hidden slides are skipped. After the last slide a black screen says the
// show has ended. A click on an item with a link opens the link.
//
// Each slide is drawn once to an image the size of the screen (at its pixel
// ratio), and the show paints the image: the one on the screen, and the next
// and previous, which are drawn while the presenter talks. A slide's
// transition (core/transition.h) plays between two of those images when the
// show moves forward onto the slide; going back, or jumping, shows the slide
// at once. After a slide's seconds, when it has any, the show moves on by
// itself, counting from the end of the last effect it plays by itself.
//
// A slide's animations (core/animation.h) play a click step at a time. A
// slide is shown with its entrance targets hidden; a click plays the next
// step, which a frame timer draws at the screen's rate, and a click while one
// plays finishes it and does no more. Steps that begin by themselves play when
// the slide, and its transition, are shown. Back takes a step back, showing
// the end of the step before; back from a slide's start goes to the slide
// before with all its steps played. A jump shows a slide at its start, and the
// seconds a slide waits skip the clicks left.
//
// With two screens, and Use Presenter View on, a show that starts goes to the
// screen the window isn't on, and Presenter View opens on the window's screen.

#include "core/animation.h"
#include "core/transition.h"

#include <QColor>
#include <QElapsedTimer>
#include <QHash>
#include <QImage>
#include <QPointer>
#include <QTimer>
#include <QVariantAnimation>
#include <QWidget>
#include <memory>
#include <vector>

namespace jp {

class Document;
class LayoutCache;
class PresenterView;
class ShowMedia;
class StepFrames;

// A slide drawn to fill `size` device-independent pixels at `dpr`, centered
// and as large as it goes, with `bars` around it. The image is opaque. With
// `states` its shapes are as the animations have them (core/animation.h).
QImage renderSlideImage(const Document *doc, LayoutCache *cache, int slide, const QSize &size, qreal dpr, const QColor &bars = Qt::black, const deck::States *states = nullptr);

// One frame of the transition `t` between two images of the same size: the
// painter's top left is the images' top left, `progress` runs from 0 (all
// `from`) to 1 (all `to`).
void paintTransition(QPainter *p, const deck::Transition &t, const QImage &from, const QImage &to, double progress);

// The part of the frame at progress `after` that differs from the frame at
// progress `before` (a wipe changes a strip, a cut changes at two moments),
// in the images' coordinates: all that needs painting again over the last frame.
QRegion transitionChange(const deck::Transition &t, const QSizeF &size, double before, double after);

class SlideShow : public QWidget {
    Q_OBJECT
public:
    SlideShow(const Document *doc, LayoutCache *cache, int startSlide, QWidget *parent = nullptr);
    // The slide on the screen, -1 at the end. While a transition plays it is
    // the slide the show is leaving.
    int currentSlide() const { return m_atEnd ? -1 : m_slide; }
    // The slide the show is on or moving to, -1 at the end.
    int targetSlide() const { return m_atEnd ? -1 : m_animating ? m_target : m_slide; }
    // The slide after `slide` that the show doesn't skip, -1 if there is none.
    int slideAfter(int slide) const { return nextShown(slide + 1, 1); }
    // A transition is playing.
    bool isAnimating() const { return m_animating; }
    // The click steps the slide on the screen has (the ones that show something), and how many have played.
    int stepCount() const;
    int step() const { return m_done; }
    // An effect of the slide on the screen is playing.
    bool isPlayingStep() const { return m_playing >= 0; }
    // A slide and how many of its steps have played.
    struct View {
        int slide = -1;   // -1: the end of the show
        int steps = 0;
        bool operator==(const View &o) const { return slide == o.slide && steps == o.steps; }
        bool operator!=(const View &o) const { return !(*this == o); }
    };
    // What is on the screen, or what it is moving to: while an effect plays, the
    // slide with the steps before it, and currentMs() into the effect (-1 when none plays).
    View currentView() const;
    double currentMs() const;
    // What the next click shows: the next step of the slide, else the next slide.
    View nextView() const;
    // A slide as the show draws it, `ms` into the step after its first `steps` (when 0 or more).
    QImage renderView(const View &view, double ms, const QSize &size, qreal dpr, const QColor &bars = Qt::black);
    bool isBlank() const { return m_blank.isValid(); }
    QColor blankColor() const { return m_blank; }   // invalid when showing a slide
    // Whether the show will move on by itself after the slide's seconds.
    bool autoAdvanceActive() const { return m_auto.isActive(); }
    const Document *document() const { return m_doc; }
    LayoutCache *cache() const { return m_cache; }
    PresenterView *presenterView() const;
    // The movies and sounds of the slide on the screen.
    ShowMedia *media() const { return m_media; }

    void next();
    void previous();
    void goTo(int slide);
    // Ends the transition that is playing, showing its slide.
    void finishTransition();
    // Black or white screen; the same color again brings the slide back.
    void toggleBlank(const QColor &color);
    // Shows in a window with Presenter View beside it, on the one screen,
    // for rehearsing; call before the show is shown.
    void setRehearsal(bool on);

    // Whether a show on a computer with two screens opens Presenter View.
    static bool usePresenterView();
    static void setUsePresenterView(bool on);

    void setVisible(bool visible) override;

Q_SIGNALS:
    // The show was ended; `lastSlide` is the slide it was on.
    void ended(int lastSlide);
    // The slide it is on or moving to, the step of it, the blank screen, or the end changed.
    void changed();
    // A frame of an effect that plays.
    void frameChanged();

protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *e) override;
    void closeEvent(QCloseEvent *e) override;

private:
    struct Play {
        deck::Timeline timeline;
        deck::Geometry geometry;
    };
    int nextShown(int from, int step) const;
    const Play &playOf(int slide) const;
    int stepsOf(int slide) const { return int(playOf(slide).timeline.steps.size()); }
    QImage imageOf(int slide);
    QImage nowImage();
    std::shared_ptr<StepFrames> framesOf(const View &view, const QSize &size, qreal dpr, const QColor &bars);
    void prefetch();
    void moveOn();
    void startTransition(int slide, const deck::Transition &t);
    void endTransition();
    void settle();
    void startAutoAdvance();
    void startAutomaticSteps();
    void startStep(int step);
    void stepFrame();
    void finishStep();
    void stepChanged();
    void showSlide(int slide, int steps = 0);
    void place();
    void openPresenterView(QScreen *on);
    // The link under a point of the window, if the slide on it has one.
    QString linkAt(const QPointF &pos) const;
    QRectF slideRect() const;
    const Document *m_doc;
    LayoutCache *m_cache;
    int m_slide = 0;
    int m_target = 0;          // where a transition is going
    bool m_atEnd = false;
    bool m_animating = false;
    bool m_started = false;    // shown once, so the slides' clocks run
    bool m_placed = false;
    bool m_rehearsal = false;
    QColor m_blank;            // a blanked screen's color; invalid when showing
    QString m_typed;           // digits typed before Enter
    QHash<int, QImage> m_images;   // slides drawn at m_imageSize and m_imageRatio
    QSize m_imageSize;
    qreal m_imageRatio = 1;
    QVariantAnimation m_anim;
    deck::Transition m_trans;
    double m_painted = -1;     // the progress last asked to be painted; -1 before the first frame
    QTimer m_auto;
    mutable QHash<int, Play> m_plays;   // the slides' animations, read when first asked for
    int m_done = 0;             // steps of the slide on the screen that have played
    int m_playing = -1;         // the step that plays, -1 for none
    double m_stepMs = 0, m_stepLength = 0;
    QElapsedTimer m_stepClock;
    QTimer m_frame;
    std::vector<std::shared_ptr<StepFrames>> m_frames;   // what draws the frames of a step, for each size asked
    QImage m_now;               // the slide on the screen when it is not at its start
    QSize m_nowSize;
    qreal m_nowRatio = 1;
    bool m_nowValid = false;
    QPointer<PresenterView> m_presenter;
    ShowMedia *m_media = nullptr;   // the movies and sounds (showmedia.h)
};

// Plays the transition into a slide over a widget, from the slide before it,
// for the ribbon's Preview. A click or Esc ends it early; it hides itself
// when it is over.
class TransitionPreview : public QWidget {
    Q_OBJECT
public:
    TransitionPreview(const Document *doc, LayoutCache *cache, QWidget *parent);
    void play(int slide);
    bool isPlaying() const { return m_anim.state() == QAbstractAnimation::Running; }

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override { stop(); }
    void keyPressEvent(QKeyEvent *e) override;

private:
    void stop();
    const Document *m_doc;
    LayoutCache *m_cache;
    QVariantAnimation m_anim;
    deck::Transition m_trans;
    QImage m_from, m_to;
    QRect m_area;   // where the slide goes, in the widget
};

} // namespace jp
