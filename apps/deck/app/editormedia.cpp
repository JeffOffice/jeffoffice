#include "app/editormedia.h"

#include "app/deckcanvas.h"
#include "app/editor.h"
#include "core/media.h"

#include <QMouseEvent>
#include <QPainter>
#include <algorithm>

namespace jp {

EditorMedia::EditorMedia(DeckCanvas *canvas) : QObject(canvas), m_canvas(canvas)
{
    Editor *ed = canvas->editor();
    connect(ed, &Editor::pageChanged, this, [this] { stop(); });
    connect(ed, &Editor::documentReplaced, this, [this] {
        stop();
        for (const QPointer<MediaPlayback> &pb : std::as_const(m_players)) delete pb.data();
        m_players.clear();
        m_hover.clear();
        m_drag.clear();
    });
    connect(ed, &Editor::changed, this, [this] { forgetGone(); });
}

const PictureItem *EditorMedia::pictureWith(const QString &id) const
{
    const Document *d = m_canvas->editor()->doc();
    const int page = m_canvas->editor()->currentPage();
    if (id.isEmpty() || !d || !m_canvas->editor()->masterView().isEmpty() || page < 0 || page >= d->pages.size()) return nullptr;
    for (const PictureItem *pic : deck::mediaPicturesOf(*d->pages[page]))
        if (pic->id == id) return pic;
    return nullptr;
}

QTransform EditorMedia::toView() const
{
    const QPointF origin = m_canvas->pageToView(QPointF(0, 0));
    const double s = m_canvas->ppp();
    return QTransform(s, 0, 0, s, origin.x(), origin.y());
}

QRectF EditorMedia::viewBounds(const PictureItem &pic) const { return toView().map(QPolygonF(pic.outline())).boundingRect(); }

const PictureItem *EditorMedia::pictureAt(const QPointF &view) const
{
    const Document *d = m_canvas->editor()->doc();
    const int page = m_canvas->editor()->currentPage();
    if (!d || !m_canvas->editor()->masterView().isEmpty() || page < 0 || page >= d->pages.size()) return nullptr;
    const QVector<const PictureItem *> pictures = deck::mediaPicturesOf(*d->pages[page]);
    for (int i = pictures.size() - 1; i >= 0; --i)   // the one drawn last is on top
        if (toView().map(QPolygonF(pictures[i]->outline())).containsPoint(view, Qt::OddEvenFill)) return pictures[i];
    return nullptr;
}

QRectF EditorMedia::barFor(const PictureItem &pic) const { return MediaBar::placement(viewBounds(pic), QRectF(m_canvas->viewport()->rect())); }

QString EditorMedia::barItem() const
{
    Editor *ed = m_canvas->editor();
    if (!ed->masterView().isEmpty()) return QString();
    // The one the mouse is over, else the one selected.
    if (pictureWith(m_hover)) return m_hover;
    if (const Item *one = ed->single())
        if (const PictureItem *pic = pictureWith(one->id)) return pic->id;
    return QString();
}

QRectF EditorMedia::barRect() const
{
    const PictureItem *pic = pictureWith(barItem());
    return pic ? barFor(*pic) : QRectF();
}

MediaPlayback *EditorMedia::playbackOf(const QString &id) const { return m_players.value(id); }

QString EditorMedia::activeId() const
{
    for (auto it = m_players.cbegin(); it != m_players.cend(); ++it)
        if (it.value() && it.value()->state() != MediaPlayback::State::Stopped) return it.key();
    return QString();
}

MediaPlayback *EditorMedia::playbackFor(const PictureItem &pic)
{
    const Document *d = m_canvas->editor()->doc();
    const int page = m_canvas->editor()->currentPage();
    QPointer<MediaPlayback> &slot = m_players[pic.id];
    if (slot && slot->failed()) {
        delete slot;
        slot.clear();
    }
    if (!slot) {
        slot = new MediaPlayback(this);
        connect(slot, &MediaPlayback::changed, this, [this, id = pic.id] { updateFor(id); });
    }
    if (!slot->started() && slot->state() == MediaPlayback::State::Stopped) {
        const auto data = d->media.constFind(pic.mediaId);
        if (data != d->media.cend()) slot->setFile(data->bytes, data->extension);
        slot->setPlay(deck::mediaPlayOf(*d->pages[page], pic));
        if (data == d->media.cend() && !pic.mediaLink.isEmpty())
            slot->setUnavailable(pic.mediaKind == QLatin1String("video") ? tr("This movie is linked from outside the presentation, so it can't be played here.")
                                                                         : tr("This sound is linked from outside the presentation, so it can't be played here."));
    }
    return slot;
}

void EditorMedia::refresh(const QString &id)
{
    const PictureItem *pic = pictureWith(id);
    MediaPlayback *pb = m_players.value(id);
    if (!pic || !pb) return;
    const Document *d = m_canvas->editor()->doc();
    pb->setPlay(deck::mediaPlayOf(*d->pages[m_canvas->editor()->currentPage()], *pic));
}

void EditorMedia::play(const QString &id)
{
    const PictureItem *pic = pictureWith(id);
    if (!pic) return;
    // One at a time.
    for (auto it = m_players.begin(); it != m_players.end(); ++it)
        if (it.key() != id && it.value() && it.value()->state() != MediaPlayback::State::Stopped) it.value()->stop();
    MediaPlayback *pb = playbackFor(*pic);
    if (pb->state() == MediaPlayback::State::Paused) pb->resume();
    else pb->play();
    updateFor(id);
}

void EditorMedia::pause()
{
    for (auto it = m_players.cbegin(); it != m_players.cend(); ++it)
        if (it.value() && it.value()->state() == MediaPlayback::State::Playing) it.value()->pause();
}

void EditorMedia::stop()
{
    for (auto it = m_players.cbegin(); it != m_players.cend(); ++it)
        if (it.value() && (it.value()->started() || it.value()->failed())) {
            it.value()->stop();
            updateFor(it.key());
        }
    // The posters are back; a message goes with the player that gave it.
    for (auto it = m_players.begin(); it != m_players.end();) {
        if (it.value() && it.value()->failed()) {
            delete it.value();
            it = m_players.erase(it);
        } else {
            ++it;
        }
    }
    m_canvas->viewport()->update();
}

// A player whose picture has left the slide (deleted, or an edit undone) is let go.
void EditorMedia::forgetGone()
{
    for (auto it = m_players.begin(); it != m_players.end();) {
        if (!pictureWith(it.key())) {
            if (it.value()) {
                it.value()->stop();
                delete it.value();
            }
            it = m_players.erase(it);
        } else {
            ++it;
        }
    }
}

void EditorMedia::updateFor(const QString &id)
{
    Q_EMIT playbackChanged();
    const PictureItem *pic = pictureWith(id);
    if (!pic) {
        m_canvas->viewport()->update();
        return;
    }
    QRectF area = viewBounds(*pic).united(barFor(*pic));
    m_canvas->viewport()->update(area.adjusted(-6, -6, 6, 6).toAlignedRect());
}

void EditorMedia::paint(QPainter *p)
{
    const Document *d = m_canvas->editor()->doc();
    const int page = m_canvas->editor()->currentPage();
    if (!d || !m_canvas->editor()->masterView().isEmpty() || page < 0 || page >= d->pages.size()) return;
    const QTransform view = toView();
    for (const PictureItem *pic : deck::mediaPicturesOf(*d->pages[page])) {
        const MediaPlayback *pb = m_players.value(pic->id);
        if (!pb) continue;
        if (pb->failed()) paintMediaMessage(p, *pic, pb->error(), view);
        else if (pb->started() && !pb->frame().isNull()) paintMediaFrame(p, *pic, pb->frame(), view);
    }
    const QString id = barItem();
    if (const PictureItem *pic = pictureWith(id)) {
        MediaBarState s;
        if (const MediaPlayback *pb = m_players.value(id)) {
            s.state = pb->state();
            s.played = pb->played();
            s.length = pb->length();
            s.volume = pb->volume();
            s.muted = pb->muted();
            s.failed = pb->failed();
        } else {
            const deck::MediaPlay play = deck::mediaPlayOf(*d->pages[page], *pic);
            s.volume = std::clamp(play.volume / 100000.0, 0.0, 1.0);
            s.muted = play.mute;
            s.length = play.lengthHint;
        }
        MediaBar::paint(p, barFor(*pic), s, id == m_hover ? m_hoverPart : MediaBar::Part::None);
    }
}

// What a press or a drag on a part of the bar does.
bool EditorMedia::act(const QString &id, MediaBar::Part part, const QPointF &pos, bool dragging)
{
    const PictureItem *pic = pictureWith(id);
    if (!pic) return false;
    const QRectF bar = barFor(*pic);
    MediaPlayback *pb = m_players.value(id);
    switch (part) {
    case MediaBar::Part::Play:
        if (dragging) break;
        if (pb && pb->state() == MediaPlayback::State::Playing) pb->pause();
        else play(id);
        break;
    case MediaBar::Part::Seek:
        if (!pb) {
            play(id);
            pb = m_players.value(id);
            if (!pb || pb->failed()) break;
            pb->pause();
        }
        pb->seek(MediaBar::seekAt(bar, pos, pb->length()));
        break;
    case MediaBar::Part::Mute:
        if (dragging) break;
        if (!pb) pb = playbackFor(*pic);
        pb->setMuted(!pb->muted());
        break;
    case MediaBar::Part::Volume:
        if (!pb) pb = playbackFor(*pic);
        pb->setVolume(MediaBar::volumeAt(bar, pos));
        if (pb->muted() && pb->volume() > 0) pb->setMuted(false);
        break;
    case MediaBar::Part::None:
        return false;
    }
    updateFor(id);
    return true;
}

bool EditorMedia::press(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) return false;
    const QString id = barItem();
    const PictureItem *pic = pictureWith(id);
    if (!pic) return false;
    const MediaBar::Part part = MediaBar::partAt(barFor(*pic), e->position());
    if (part == MediaBar::Part::None) return false;
    m_drag = id;
    m_dragPart = part;
    act(id, part, e->position(), false);
    e->accept();
    return true;
}

bool EditorMedia::move(QMouseEvent *e)
{
    if (!m_drag.isEmpty()) {
        if (m_dragPart == MediaBar::Part::Seek || m_dragPart == MediaBar::Part::Volume) act(m_drag, m_dragPart, e->position(), true);
        e->accept();
        return true;
    }
    // The bar stays while the mouse goes from the picture to it.
    const PictureItem *over = pictureAt(e->position());
    QString now = over ? over->id : QString();
    if (now.isEmpty() && !m_hover.isEmpty())
        if (const PictureItem *was = pictureWith(m_hover); was && barFor(*was).contains(e->position())) now = m_hover;
    MediaBar::Part part = MediaBar::Part::None;
    if (const PictureItem *shown = pictureWith(now.isEmpty() ? barItem() : now)) part = MediaBar::partAt(barFor(*shown), e->position());
    if (now != m_hover || part != m_hoverPart) {
        const QString was = m_hover;
        m_hover = now;
        m_hoverPart = part;
        updateFor(was);
        updateFor(now);
    }
    return false;
}

bool EditorMedia::release(QMouseEvent *e)
{
    if (m_drag.isEmpty()) return false;
    m_drag.clear();
    m_dragPart = MediaBar::Part::None;
    e->accept();
    return true;
}

void EditorMedia::leave()
{
    if (!m_drag.isEmpty() || m_hover.isEmpty()) return;
    const QString was = m_hover;
    m_hover.clear();
    m_hoverPart = MediaBar::Part::None;
    updateFor(was);
}

} // namespace jp
