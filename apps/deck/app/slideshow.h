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
//
// How it goes is set by core/showsettings.h (setSettings, setSlides): by a
// speaker (as above), by an individual in a window, or at a kiosk (full
// screen, looping, moving on by the slides' timings only, no keys or clicks
// but Esc and links); looping, without animation, and by hand rather than the
// slides' timings; a range of the slides or a custom show, which plays the
// slides it lists in its own order.
//
// The pointer is the arrow, a laser pointer (Ctrl+L), the pen (Ctrl+P), the
// highlighter (Ctrl+I), or the eraser (Ctrl+E); Ctrl+A or Esc brings the
// arrow back, E erases the ink on the slide, and a right click opens a menu
// with all of these. A stroke of the pen or the highlighter (core/ink.h) stays
// on its slide while the show runs; the window asks what to do with it when
// the show ends (ink()).
//
// Rehearse Timings (setRecording) times each slide from the moment the show
// moves onto it until it moves on, with a small toolbar for next, pause, the
// slide's time, repeat the slide, and the total; recordedSeconds() has them.

#include "app/rehearsebar.h"
#include "core/animation.h"
#include "core/ink.h"
#include "core/showsettings.h"
#include "core/transition.h"

#include <QColor>
#include <QElapsedTimer>
#include <QHash>
#include <QImage>
#include <QMap>
#include <QPointer>
#include <QTimer>
#include <QVariantAnimation>
#include <QWidget>
#include <functional>
#include <memory>
#include <vector>

class QMenu;
class QPainter;

namespace jp {

class Document;
struct Page;
class LayoutCache;
class PresenterView;
class ShowMedia;
class StepFrames;
class MorphFrames;

// A slide drawn to fill `size` device-independent pixels at `dpr`, centered
// and as large as it goes, with `bars` around it. The image is opaque. With
// `states` its shapes are as the animations have them (core/animation.h).
QImage renderSlideImage(const Document *doc, LayoutCache *cache, int slide, const QSize &size, qreal dpr, const QColor &bars = Qt::black, const deck::States *states = nullptr);

// Where a slide of this size goes in an area: as large as it fits, in the middle.
QRectF slideRectIn(const QSizeF &page, const QSizeF &area);

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
    // The slide after the one the show is on (or moving to) that the show doesn't skip, -1 if there is none.
    int slideAfter() const;
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
    // To a slide by its number: the next place after the one the show is at that has the slide, going round (a custom show may list a slide
    // twice); nothing for a slide the show does not play. goToPosition() goes to a place in the order (0 is the first).
    void goTo(int slide);
    void goToPosition(int pos);
    // Ends the transition that is playing, showing its slide.
    void finishTransition();
    // Black or white screen; the same color again brings the slide back.
    void toggleBlank(const QColor &color);
    // Shows in a window with Presenter View beside it, on the one screen,
    // for rehearsing; call before the show is shown.
    void setRehearsal(bool on);

    // How the show goes, and which slides play (in this order, starting at the
    // first; empty for every slide, in order, from the slide the show was asked
    // to start at), hidden ones too when `includeHidden`; call before the show
    // is shown.
    void setSettings(const deck::ShowSettings &settings);
    const deck::ShowSettings &settings() const { return m_settings; }
    void setSlides(const QList<int> &slides, bool includeHidden);
    // The slides it plays, in order, as the document's indices.
    QList<int> playedSlides() const;
    // The show is in a window of its own (by an individual, or rehearsing), not full screen.
    bool isWindowed() const { return m_rehearsal || m_settings.type == deck::ShowSettings::Type::Individual; }
    bool isKiosk() const { return m_settings.type == deck::ShowSettings::Type::Kiosk; }

    // Rehearse Timings: the show times its slides and shows the toolbar; call
    // before the show is shown. The seconds each slide was on the screen (the
    // last time it was), by slide, the total, and the toolbar's controls.
    void setRecording(bool on);
    bool isRecording() const { return m_recording; }
    QMap<int, double> recordedSeconds() const;
    double recordedTotal() const;
    RehearseBar *rehearseBar() const { return m_bar; }
    // The deck was edited (a show in a window leaves it open to editing): the show follows its slides by their pages, so that it stays on
    // the slide it is on, or the one that took its place, with its order, its ink, and its times, and draws the slides again. The window
    // calls this when the editor says it changed.
    void documentChanged();
    // The presentation was replaced by another: nothing of the old one is kept, and the show has nothing to show.
    void rebind(const Document *doc);
    // The recording's clock, in milliseconds (the time of day unless a test gives another).
    void setClock(const std::function<qint64()> &clock)
    {
        m_clock = clock;
        m_recSince = m_clock();
    }
    // The recording's clocks (in milliseconds): the slide on the screen, and all.
    qint64 slideClock() const;
    qint64 totalClock() const;
    bool isRecordingPaused() const { return m_recPaused; }
    void pauseRecording(bool paused);
    void repeatSlide();   // the slide's clock starts again from nothing

    // The pointer: the arrow, a laser pointer, the pen, the highlighter, or the eraser.
    enum class Pointer { Arrow, Laser, Pen, Highlighter, Eraser };
    Pointer pointer() const { return m_pointer; }
    void setPointer(Pointer pointer);
    // The color of the pen or the highlighter, whichever is the pointer (the pen's, else).
    QColor inkColor() const;
    void setInkColor(const QColor &color);
    QColor laserColor() const { return m_settings.laserColor; }
    // The ink on a slide, and on all of them (by slide); a stroke being drawn is in it.
    const QVector<deck::InkStroke> &ink(int slide) const;
    // Ink belongs to the page it was drawn on, by its id, whatever happens to the slides' places.
    QHash<QString, QVector<deck::InkStroke>> allInk() const { return m_ink; }
    int inkCount() const;
    // Takes the ink off a slide (-1: the one on the screen).
    void eraseInk(int slide = -1);
    // A stroke with the pen or the highlighter, or the eraser, or the laser
    // pointer's dot, from the pointer's place on the slide in slide points
    // (a window of the show or Presenter View turns its own points into these).
    void pressAt(const QPointF &page);
    void moveTo(const QPointF &page);
    void releasePointer();
    bool isDrawing() const { return m_drawing; }
    void pointerLeft();
    // The ink on `slide` and, for the one on the screen, the laser pointer's dot, drawn over the slide where it is in `at`.
    void paintOverlay(QPainter *p, int slide, const QRectF &at) const;
    // The menu a right click opens, and its pointer part, which Presenter View shows too; `parent` owns them.
    QMenu *buildMenu(QWidget *parent);
    QMenu *pointerMenu(QWidget *parent);

    // Whether a show on a computer with two screens opens Presenter View.
    static bool usePresenterView();
    static void setUsePresenterView(bool on);

    void setVisible(bool visible) override;

Q_SIGNALS:
    // The show was ended; `lastSlide` is the slide it was on.
    void ended(int lastSlide);
    // The ink or the laser pointer changed, or the pointer did.
    void overlayChanged();
    // The deck changed under the show: its slides are to be drawn again.
    void slidesChanged();
    // The slide it is on or moving to, the step of it, the blank screen, or the end changed.
    void changed();
    // A frame of an effect that plays.
    void frameChanged();

protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *e) override;
    void closeEvent(QCloseEvent *e) override;

private:
    struct Play {
        deck::Timeline timeline;
        deck::Geometry geometry;
    };
    // The slides it plays are positions in order: a position's slide is slideAt() (a custom show may list one twice).
    int count() const { return m_order.isEmpty() ? int(m_doc->pages.size()) : int(m_order.size()); }
    int slideAt(int pos) const { return m_order.isEmpty() ? pos : m_order[pos]; }
    int nextShown(int from, int step) const;
    int positionOf(int slide) const;
    int placeOf(int slide) const;
    // The page of a slide (null when there is none), and its id.
    const Page *pageOf(int slide) const;
    QString idOf(int slide) const;
    bool showsCursor() const { return isWindowed() || m_recording; }
    const Play &playOf(int slide) const;
    int stepsOf(int slide) const { return int(playOf(slide).timeline.steps.size()); }
    QImage imageOf(int slide);
    QImage nowImage();
    std::shared_ptr<StepFrames> framesOf(const View &view, const QSize &size, qreal dpr, const QColor &bars);
    void prefetch();
    void moveOn();
    void startTransition(int pos, const deck::Transition &t);
    std::shared_ptr<MorphFrames> morphFor(int to, const deck::Transition &t);
    void endTransition();
    void settle();
    void startAutoAdvance();
    void startAutomaticSteps();
    void startStep(int step);
    void stepFrame();
    void finishStep();
    void stepChanged();
    void showSlide(int pos, int steps = 0);
    void place();
    void openPresenterView(QScreen *on);
    // The link under a point of the window, if the slide on it has one.
    QString linkAt(const QPointF &pos) const;
    QRectF slideRect() const;
    QPointF pageAt(const QPointF &pos) const;
    // The ink and laser on an image of a slide (a transition plays between these).
    QImage withOverlay(const QImage &image, int slide) const;
    void updateCursor();
    void paintInk(QPainter *p, int slide, const QRectF &at, bool laser) const;
public:
    // The points of ink strokes drawn on this thread since the program began: a count for tests of cost (finished strokes come from a picture, not drawn again for each repaint).
    static qint64 inkPointsDrawnForTest();
private:
    void changeOverlay(const QRectF &page = QRectF());
    void pressPointer(const QPointF &page);
    void erasePoint(const QPointF &page);
    Pointer inkPointer() const;
    // The recording: the slide being timed changes (-1: none), and the clocks run.
    void timeSlide(int slide);
    qint64 clockNow() const { return m_clock(); }
    const Document *m_doc;
    LayoutCache *m_cache;
    deck::ShowSettings m_settings;
    QList<int> m_order;        // the slides to play (document indices); empty: all of them, in order
    bool m_hiddenToo = false;  // hidden slides play (a custom show's)
    int m_pos = 0;             // the position in the order of the slide on the screen
    int m_targetPos = 0;       // where a transition is going
    int m_slide = 0;           // the slide at m_pos
    int m_startSlide = 0;      // the slide the show was asked to start at
    QStringList m_ids;         // the ids of the pages, as the show last saw them, for following them when they change
    QStringList m_orderIds;    // the ids of the pages in m_order
    bool m_rebound = false;    // the presentation it showed was replaced: it shows nothing, and is to close
    int m_target = 0;          // the slide at m_targetPos
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
    std::shared_ptr<MorphFrames> m_morph;   // a morph plays from the slides' objects (app/morph.h), not their images
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
    // The pointer, and the ink the pen and highlighter left, by slide.
    Pointer m_pointer = Pointer::Arrow;
    QColor m_penColor, m_highlighterColor;
    QHash<QString, QVector<deck::InkStroke>> m_ink;   // by page id
    quint64 m_inkRevision = 0;   // changes when ink is taken away
    // The finished strokes of a slide with a good deal of ink, drawn once to a picture of the size they are shown at.
    struct InkLayer {
        QImage image;
        int drawn = 0;
        quint64 revision = 0;
    };
    mutable QHash<QString, InkLayer> m_inkLayers;
    mutable QStringList m_inkLayerOrder;   // oldest first
    bool m_drawing = false;     // a stroke is being drawn (the last one of the slide's ink)
    bool m_erasing = false;
    bool m_tempLaser = false;   // Ctrl and the left button: the laser pointer while they are down
    QPointF m_laser;
    bool m_laserShown = false;
    QImage m_fromOverlay, m_toOverlay;   // the slides a transition plays between, with their ink
    // Rehearsing.
    bool m_recording = false, m_recPaused = false;
    QString m_recId;                   // the page of the slide being timed
    qint64 m_recBase = 0, m_recSince = 0;   // what its clock counted, and when it last started
    QMap<QString, qint64> m_recorded;  // by page id, milliseconds
    std::function<qint64()> m_clock;
    QPointer<RehearseBar> m_bar;
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
    std::shared_ptr<MorphFrames> m_morph;
    QRect m_area;   // where the slide goes, in the widget
};

} // namespace jp
