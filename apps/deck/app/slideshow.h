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
// itself, counting from the end of the transition.

#include "core/transition.h"

#include <QColor>
#include <QHash>
#include <QImage>
#include <QTimer>
#include <QVariantAnimation>
#include <QWidget>

namespace jp {

class Document;
class LayoutCache;

// A slide drawn to fill `size` device-independent pixels at `dpr`, centered
// and as large as it goes, with `bars` around it. The image is opaque.
QImage renderSlideImage(const Document *doc, LayoutCache *cache, int slide, const QSize &size, qreal dpr, const QColor &bars = Qt::black);

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
    bool isAnimating() const { return m_animating; }
    bool isBlank() const { return m_blank.isValid(); }
    QColor blankColor() const { return m_blank; }   // invalid when showing a slide
    // Whether the show will move on by itself after the slide's seconds.
    bool autoAdvanceActive() const { return m_auto.isActive(); }
    const Document *document() const { return m_doc; }
    LayoutCache *cache() const { return m_cache; }

    void next();
    void previous();
    void goTo(int slide);
    // Ends the transition that is playing, showing its slide.
    void finishTransition();
    // Black or white screen; the same color again brings the slide back.
    void toggleBlank(const QColor &color);
Q_SIGNALS:
    // The show was ended; `lastSlide` is the slide it was on.
    void ended(int lastSlide);
    // The slide it is on or moving to, the blank screen, or the end changed.
    void changed();

protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent *e) override;
    void closeEvent(QCloseEvent *e) override;

private:
    int nextShown(int from, int step) const;
    QImage imageOf(int slide);
    void prefetch();
    void startTransition(int slide, const deck::Transition &t);
    void endTransition();
    void settle();
    void startAutoAdvance();
    void showSlide(int slide);
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
    QColor m_blank;            // a blanked screen's color; invalid when showing
    QString m_typed;           // digits typed before Enter
    QHash<int, QImage> m_images;   // slides drawn at m_imageSize and m_imageRatio
    QSize m_imageSize;
    qreal m_imageRatio = 1;
    QVariantAnimation m_anim;
    deck::Transition m_trans;
    double m_painted = -1;     // the progress last asked to be painted; -1 before the first frame
    QTimer m_auto;
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
