#pragma once
// Movies and sounds in the slide show. The slide's timing says when each one
// plays (core/media.h): in the click sequence (a click, or a step that goes
// with or after the one before), by itself when the slide appears, or when
// it is clicked on; and how: from where its trim starts to where it ends, at
// its volume, looping until stopped, rewinding after it plays, filling the
// screen when it is a movie that plays full screen, and hidden while it
// isn't playing. A movie plays in the picture's place on the slide; the mouse
// over it shows the control bar, and a click on it plays or pauses it. What
// plays stops when the slide changes, unless the file says it plays across
// slides (a sound, usually).
//
// The slide show (slideshow.h) owns one and tells it where it is: it holds
// no decoder until a movie or sound is asked to play.

#include "app/mediaplayer.h"
#include "core/media.h"

#include <QHash>
#include <QObject>
#include <QPointer>

class QWidget;

namespace jp {

class Document;

class ShowMedia : public QObject {
    Q_OBJECT
public:
    // `view` is the show's window: it is painted again as a movie plays.
    ShowMedia(const Document *doc, QWidget *view);

    // The slide the show is on, with `done` of its click steps played: -1 when none is on the screen (a transition
    // is playing, or the show has ended). Told again and again as steps play; only a new slide starts and stops anything.
    void sync(int slide, int done);
    // The click step (counting those that show something) begins: the media that go with it start.
    void stepStarted(int slide, int step);
    // A click or key forward when `done` steps have played: true when it starts or controls media instead (a step of
    // only a call to a movie or sound is a click of its own).
    bool takeClick(int slide, int done);
    // Mouse over the slide: true when something here is the media's (its control bar, or a movie to click), and the
    // press does its work.
    bool mouseMove(const QPointF &pos);
    bool mousePress(const QPointF &pos);
    // Over the slide, after it is painted: the movies that play, a message for a file that can't, and the control bar.
    void paint(QPainter *p);

    // For tests: the player of a picture (by the picture's id) on the slide now, or null.
    MediaPlayback *playbackOf(const QString &itemId) const;
    // Where the bar of the picture shows, in the window's coordinates.
    QRectF barRect(const QString &itemId) const;
    QString hovered() const { return m_hover; }

private:
    struct Entry {
        QPointer<MediaPlayback> player;
        int slide = -1;
        QString itemId;
        deck::MediaPlay play;
        int left = 1;   // slides it still plays across, this one included
    };
    struct Pending {   // a step of only calls: a click of its own
        int atDone = 0;
        int step = 0;
        bool taken = false;
    };
    const PictureItem *pictureWithShape(int shape) const;
    const PictureItem *pictureWithId(const QString &id) const;
    const PictureItem *pictureAt(const QPointF &pos) const;
    Entry *entryFor(const PictureItem &pic, bool create);
    Entry *entryOnSlide(const QString &itemId);
    void run(const deck::MediaCue &cue);
    void runStep(int step);
    void leave();
    void arrive(int slide, int done);
    QRectF slideRect() const;
    QTransform toWindow() const;
    QRectF barFor(const PictureItem &pic) const;
    bool showsFrame(const Entry &e) const;
    bool controlsOf(const QPointF &pos, QString *id, MediaBar::Part *part) const;
    const Document *m_doc;
    QWidget *m_view;
    int m_slide = -1;
    int m_generation = 0;
    deck::MediaPlan m_plan;
    QVector<Pending> m_pending;
    QHash<QString, Entry> m_entries;           // by slide and picture
    QString m_hover;
    MediaBar::Part m_hoverPart = MediaBar::Part::None;
    QString m_drag;
    MediaBar::Part m_dragPart = MediaBar::Part::None;
};

} // namespace jp
