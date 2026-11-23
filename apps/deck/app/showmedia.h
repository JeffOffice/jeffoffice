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

#include "app/linkedmedia.h"
#include "app/mediaplayer.h"
#include "core/media.h"

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QSet>

class QWidget;

namespace jp {

class Document;

class ShowMedia : public QObject {
    Q_OBJECT
public:
    // `view` is the show's window: it is painted again as a movie plays.
    ShowMedia(const Document *doc, QWidget *view);

    // A slide is on the screen to stay, with `done` of its click steps played: what plays by itself begins.
    // `pos` is its place in the order the show goes in (what a count of slides is counted in; the slide's own number when the show goes in order).
    void arrive(int slide, int done, int pos = -1);
    // The slide on the screen goes (a transition begins, or the show moves to another slide): what plays stops, but
    // what the file says plays across slides goes on, while the show is within the places the count names, counted from the place it began at
    // (going back and forth inside them stops nothing; going past the last, or back before the first, stops it). Said again, it counts once.
    // `nextPos` is the place the show goes to (-1: nowhere, everything stops).
    void leave(int nextPos = -1);
    // The show is over (its last slide has gone, or it is closed): everything stops.
    void end();
    // The show is bound to another presentation: everything stops, and nothing of the old one is used again.
    void rebind(const Document *doc);
    // The click step (counting those that show something) begins: the media that go with it start.
    void stepStarted(int slide, int step);
    // A click or key forward when `done` steps have played: true when it starts or controls media instead (a step of
    // only a call to a movie or sound is a click of its own).
    bool takeClick(int slide, int done);
    // Mouse over the slide: true when something here is the media's (its control bar, or a movie to click), and the
    // press does its work.
    bool mouseMove(const QPointF &pos);
    // Whether the click was the media's. `onBar` says it was on a part of a movie's controls.
    bool mousePress(const QPointF &pos, bool *onBar = nullptr);
    // Over the slide, after it is painted: the movies that play, a message for a file that can't, and the control bar.
    void paint(QPainter *p);

    // When the show starts, with "Ask me first" chosen: one question for every linked movie or sound the show has (so none is asked in the middle of the full screen).
    void askAboutLinks(QWidget *parent);
    // The folder of the presentation playing, which a linked file with a relative name is counted from.
    void setBaseDirectory(const QString &dir) { m_baseDir = dir; }

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
        int first = -1, last = -1;   // the places of the show it plays across, from where it began (the same when it plays on its own slide only)
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
    int m_pos = -1;   // the place of the slide on the screen
    QSet<QString> m_carried;   // the media that went on playing across slides to the slide now arriving: its own start does not play them again
    Entry *entryOnSlide(const QString &itemId);
    void run(const deck::MediaCue &cue);
    void runStep(int step);
    const Page *page() const;
    QRectF slideRect() const;
    QTransform toWindow() const;
    QRectF barFor(const PictureItem &pic) const;
    bool showsFrame(const Entry &e) const;
    bool controlsOf(const QPointF &pos, QString *id, MediaBar::Part *part) const;
    const Document *m_doc;
    QString m_baseDir;                      // the presentation's folder, for a linked file named by a relative path
    LinkedMediaMemory m_linked;             // what was answered and refused for each linked file (Options > Security)
    QWidget *m_view;
    int m_slide = -1;
    QString m_pageId;   // of the slide on the screen: the presentation may be edited while the show plays
    bool m_on = false;  // a slide is on the screen
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
