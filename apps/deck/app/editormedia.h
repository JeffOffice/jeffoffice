#pragma once
// Movies and sounds on the slide being edited. A movie shows its poster (the
// picture it gives); when it is selected or the mouse is over it, a control
// bar under it plays and pauses it, moves in it, mutes it, and sets the
// volume. A movie plays in place on the slide, painted over the canvas,
// scaled with the zoom; a file that can't be played shows a message there
// instead. One plays at a time, and it stops when the slide changes.
//
// Nothing is decoded until a person presses Play (the bar's, or the
// Playback tab's); selecting, hovering, and drawing never read the file.

#include "app/mediaplayer.h"

#include <QHash>
#include <QObject>
#include <QPointer>

class QMouseEvent;

namespace jp {

class DeckCanvas;
class PictureItem;

class EditorMedia : public QObject {
    Q_OBJECT
public:
    explicit EditorMedia(DeckCanvas *canvas);

    // Plays the movie or sound of the picture `id` (from the start, or on from where it paused).
    void play(const QString &id);
    void pause();
    // Stops whatever plays and puts the posters back.
    void stop();
    // The player of a picture, once Play has been pressed for it; null before.
    MediaPlayback *playbackOf(const QString &id) const;
    // The picture whose player is playing or paused, else empty.
    QString activeId() const;
    // Tells the player of that picture (a settings change from the Playback tab) to use the settings now.
    void refresh(const QString &id);

    // The picture whose control bar shows (the one selected, else the one the mouse is over), and where the bar is in the canvas's viewport.
    QString barItem() const;
    QRectF barRect() const;

    // Drawn over the slide, by the canvas's paint.
    void paint(QPainter *p);
    // The canvas's mouse events; true when the bar took the event.
    bool press(QMouseEvent *e);
    bool move(QMouseEvent *e);
    bool release(QMouseEvent *e);
    void leave();

Q_SIGNALS:
    // A player started, paused, or stopped: what shows its state (the Playback tab's Play) says so.
    void playbackChanged();

private:
    const PictureItem *pictureWith(const QString &id) const;
    const PictureItem *pictureAt(const QPointF &view) const;
    QTransform toView() const;
    QRectF viewBounds(const PictureItem &pic) const;
    QRectF barFor(const PictureItem &pic) const;
    MediaPlayback *playbackFor(const PictureItem &pic);
    void updateFor(const QString &id);
    void forgetGone();
    bool act(const QString &id, MediaBar::Part part, const QPointF &pos, bool dragging);
    DeckCanvas *m_canvas;
    QHash<QString, QPointer<MediaPlayback>> m_players;
    QString m_hover;
    QString m_drag;
    MediaBar::Part m_dragPart = MediaBar::Part::None;
    MediaBar::Part m_hoverPart = MediaBar::Part::None;
};

} // namespace jp
