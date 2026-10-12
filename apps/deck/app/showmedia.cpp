#include "app/showmedia.h"

#include "core/document.h"

#include <QPainter>
#include <QRegularExpression>
#include <QTimer>
#include <QWidget>
#include <algorithm>

namespace jp {

ShowMedia::ShowMedia(const Document *doc, QWidget *view) : QObject(view), m_doc(doc), m_view(view) {}

QRectF ShowMedia::slideRect() const
{
    const QSizeF page = m_doc->pageSize(), area = m_view->size();
    const double scale = std::min(area.width() / page.width(), area.height() / page.height());
    const QSizeF shown = page * scale;
    return QRectF(QPointF((area.width() - shown.width()) / 2, (area.height() - shown.height()) / 2), shown);
}

QTransform ShowMedia::toWindow() const
{
    const QRectF at = slideRect();
    const double scale = at.width() / m_doc->pageSize().width();
    return QTransform(scale, 0, 0, scale, at.left(), at.top());
}

const PictureItem *ShowMedia::pictureWithShape(int shape) const
{
    if (m_slide < 0 || shape <= 0) return nullptr;
    for (const PictureItem *pic : deck::mediaPicturesOf(*m_doc->pages[m_slide]))
        if (pic->sourceId == shape) return pic;
    return nullptr;
}

const PictureItem *ShowMedia::pictureWithId(const QString &id) const
{
    if (m_slide < 0 || id.isEmpty()) return nullptr;
    for (const PictureItem *pic : deck::mediaPicturesOf(*m_doc->pages[m_slide]))
        if (pic->id == id) return pic;
    return nullptr;
}

ShowMedia::Entry *ShowMedia::entryOnSlide(const QString &itemId)
{
    const auto it = m_entries.find(QString::number(m_slide) + QLatin1Char(':') + itemId);
    return it == m_entries.end() ? nullptr : &it.value();
}

// The movie or sound's player, made when it is first asked to play: the decoder starts then and not before.
ShowMedia::Entry *ShowMedia::entryFor(const PictureItem &pic, bool create)
{
    if (Entry *known = entryOnSlide(pic.id)) return known;
    if (!create) return nullptr;
    // A slide that asks for more than this many at once (a damaged or hostile file) gets the first few.
    if (m_entries.size() >= 16) return nullptr;
    Entry e;
    e.slide = m_slide;
    e.itemId = pic.id;
    e.play = pic.sourceId > 0 && m_plan.plays.contains(pic.sourceId) ? m_plan.plays.value(pic.sourceId) : deck::mediaPlayOf(*m_doc->pages[m_slide], pic);
    e.left = std::max(1, e.play.acrossSlides);
    auto *pb = new MediaPlayback(this);
    const auto data = m_doc->media.constFind(pic.mediaId);
    if (data != m_doc->media.cend()) pb->setFile(data->bytes, data->extension);
    pb->setPlay(e.play);
    if (data == m_doc->media.cend())
        pb->setUnavailable(!pic.mediaLink.isEmpty()
                               ? (e.play.video ? tr("This movie is linked from outside the presentation, so it can't be played here.")
                                               : tr("This sound is linked from outside the presentation, so it can't be played here."))
                               : (e.play.video ? tr("There is no movie here to play.") : tr("There is no sound here to play.")));
    connect(pb, &MediaPlayback::changed, m_view, qOverload<>(&QWidget::update));
    e.player = pb;
    return &m_entries.insert(QString::number(m_slide) + QLatin1Char(':') + pic.id, e).value();
}

void ShowMedia::leave()
{
    ++m_generation;   // what was to be called later on the slide is not
    for (auto it = m_entries.begin(); it != m_entries.end();) {
        if (it->left > 1) {
            --it->left;   // plays across this slide's end
            ++it;
            continue;
        }
        if (it->player) {
            it->player->stop();
            delete it->player;
        }
        it = m_entries.erase(it);
    }
    m_pending.clear();
    m_plan = deck::MediaPlan();
    m_hover.clear();
    m_hoverPart = MediaBar::Part::None;
}

void ShowMedia::arrive(int slide, int done)
{
    if (slide < 0 || slide >= m_doc->pages.size()) return;
    m_plan = deck::mediaPlanOf(*m_doc->pages[slide]);
    if (m_plan.isEmpty()) return;
    int visual = 0;
    for (int i = 0; i < m_plan.steps.size(); ++i) {
        const deck::MediaStep &s = m_plan.steps[i];
        if (s.visual) {
            ++visual;
            continue;
        }
        if (s.cues.isEmpty()) continue;
        if (s.automatic) {
            if (done == 0) runStep(i);
        } else {
            m_pending.push_back(Pending{visual, i, visual < done});
        }
    }
}

void ShowMedia::sync(int slide, int done)
{
    if (slide == m_slide) return;
    leave();
    m_slide = slide;
    arrive(slide, done);
    m_view->update();
}

void ShowMedia::runStep(int step)
{
    if (step < 0 || step >= m_plan.steps.size()) return;
    const int generation = m_generation;
    for (const deck::MediaCue &cue : m_plan.steps[step].cues) {
        if (cue.at <= 0) {
            run(cue);
        } else {
            QTimer::singleShot(int(std::min(cue.at, 3600000.0)), this, [this, generation, cue] {
                if (generation == m_generation) run(cue);
            });
        }
    }
}

void ShowMedia::stepStarted(int slide, int step)
{
    if (slide != m_slide || m_plan.isEmpty()) return;
    int visual = -1;
    for (int i = 0; i < m_plan.steps.size(); ++i) {
        if (m_plan.steps[i].visual) ++visual;
        if (visual == step && m_plan.steps[i].visual) {
            runStep(i);
            return;
        }
    }
}

bool ShowMedia::takeClick(int slide, int done)
{
    if (slide != m_slide) return false;
    for (Pending &p : m_pending)
        if (!p.taken && p.atDone == done) {
            p.taken = true;
            runStep(p.step);
            return true;
        }
    return false;
}

// What a call in the timing does to a movie or sound.
void ShowMedia::run(const deck::MediaCue &cue)
{
    const PictureItem *pic = pictureWithShape(cue.shape);
    if (!pic) return;
    Entry *e = entryFor(*pic, true);
    if (!e || !e->player) return;
    MediaPlayback *pb = e->player;
    static const QRegularExpression from(QStringLiteral("^playFrom\\(\\s*(-?[0-9]*\\.?[0-9]*)\\s*\\)$"));
    if (const QRegularExpressionMatch m = from.match(cue.command); m.hasMatch()) pb->playFrom(m.captured(1).toDouble());
    else if (cue.command == QLatin1String("play")) pb->play();
    else if (cue.command == QLatin1String("resume")) pb->resume();
    else if (cue.command == QLatin1String("pause")) pb->pause();
    else if (cue.command == QLatin1String("stop")) pb->stop();
    else if (cue.command == QLatin1String("togglePause")) pb->togglePause();
    m_view->update();
}

bool ShowMedia::showsFrame(const Entry &e) const
{
    const MediaPlayback *pb = e.player;
    if (!pb || pb->failed() || !pb->started() || pb->frame().isNull()) return false;
    return !(e.play.hideWhenStopped && pb->state() == MediaPlayback::State::Stopped);
}

QRectF ShowMedia::barFor(const PictureItem &pic) const
{
    const QRectF window(m_view->rect());
    QRectF shown = toWindow().map(QPolygonF(pic.outline())).boundingRect();
    if (const Entry *e = const_cast<ShowMedia *>(this)->entryOnSlide(pic.id); e && e->play.fullScreen && e->play.video && showsFrame(*e)) shown = window;
    return MediaBar::placement(shown, window);
}

QRectF ShowMedia::barRect(const QString &itemId) const
{
    const PictureItem *pic = pictureWithId(itemId);
    return pic ? barFor(*pic) : QRectF();
}

MediaPlayback *ShowMedia::playbackOf(const QString &itemId) const
{
    const auto it = m_entries.constFind(QString::number(m_slide) + QLatin1Char(':') + itemId);
    return it == m_entries.cend() ? nullptr : it->player.data();
}

// The movie or sound the mouse is over: one that plays full screen is the whole window; one that hides while it isn't
// playing can't be pointed at then.
const PictureItem *ShowMedia::pictureAt(const QPointF &pos) const
{
    if (m_slide < 0) return nullptr;
    const QVector<const PictureItem *> pictures = deck::mediaPicturesOf(*m_doc->pages[m_slide]);
    for (const PictureItem *pic : pictures)
        if (const Entry *e = const_cast<ShowMedia *>(this)->entryOnSlide(pic->id); e && e->play.fullScreen && e->play.video && showsFrame(*e)) return pic;
    const QTransform view = toWindow();
    for (int i = pictures.size() - 1; i >= 0; --i) {
        const PictureItem *pic = pictures[i];
        if (m_plan.plays.contains(pic->sourceId) && m_plan.plays.value(pic->sourceId).hideWhenStopped) {
            const Entry *e = const_cast<ShowMedia *>(this)->entryOnSlide(pic->id);
            if (!e || !showsFrame(*e)) continue;
        }
        if (view.map(QPolygonF(pic->outline())).containsPoint(pos, Qt::OddEvenFill)) return pic;
    }
    return nullptr;
}

// What of the media is under the mouse: the picture, and the part of its bar the mouse is on (none when it is only on the picture).
bool ShowMedia::controlsOf(const QPointF &pos, QString *id, MediaBar::Part *part) const
{
    *part = MediaBar::Part::None;
    if (const PictureItem *shown = pictureWithId(m_hover)) {
        const MediaBar::Part p = MediaBar::partAt(barFor(*shown), pos);
        if (p != MediaBar::Part::None || barFor(*shown).contains(pos)) {
            *id = shown->id;
            *part = p;
            return true;
        }
    }
    if (const PictureItem *pic = pictureAt(pos)) {
        *id = pic->id;
        *part = MediaBar::partAt(barFor(*pic), pos);
        return true;
    }
    return false;
}

bool ShowMedia::mouseMove(const QPointF &pos)
{
    if (m_slide < 0 || m_plan.isEmpty()) return false;
    QString id;
    MediaBar::Part part;
    const bool over = controlsOf(pos, &id, &part);
    if (id != m_hover || part != m_hoverPart) {
        m_hover = id;
        m_hoverPart = part;
        m_view->update();
    }
    return over;
}

bool ShowMedia::mousePress(const QPointF &pos)
{
    if (m_slide < 0 || m_plan.isEmpty()) return false;
    QString id;
    MediaBar::Part part;
    if (controlsOf(pos, &id, &part)) {
        const PictureItem *pic = pictureWithId(id);
        if (!pic) return false;
        Entry *e = entryFor(*pic, false);
        MediaPlayback *pb = e ? e->player.data() : nullptr;
        switch (part) {
        case MediaBar::Part::Play:
            if (pb && pb->state() == MediaPlayback::State::Playing) pb->pause();
            else if (pb && pb->state() == MediaPlayback::State::Paused) pb->resume();
            else if ((e = entryFor(*pic, true)) && e->player) e->player->play();
            break;
        case MediaBar::Part::Seek:
            if (!pb && (e = entryFor(*pic, true))) pb = e->player;
            if (pb) {
                pb->seek(MediaBar::seekAt(barFor(*pic), pos, pb->length()));
                if (pb->state() == MediaPlayback::State::Stopped) pb->resume();
            }
            break;
        case MediaBar::Part::Mute:
            if (!pb && (e = entryFor(*pic, true))) pb = e->player;
            if (pb) pb->setMuted(!pb->muted());
            break;
        case MediaBar::Part::Volume:
            if (!pb && (e = entryFor(*pic, true))) pb = e->player;
            if (pb) {
                pb->setVolume(MediaBar::volumeAt(barFor(*pic), pos));
                if (pb->muted() && pb->volume() > 0) pb->setMuted(false);
            }
            break;
        case MediaBar::Part::None:
            // On the bar but on no part of it: nothing, and the click is the bar's.
            if (pictureAt(pos) != pic) break;
            // A click on the movie itself: what the file says a click on it does, else it plays or pauses.
            if (const auto trigger = m_plan.triggers.constFind(pic->sourceId); pic->sourceId > 0 && trigger != m_plan.triggers.cend()) {
                for (const deck::MediaCue &cue : trigger.value()) run(cue);
            } else if (pb && pb->state() == MediaPlayback::State::Playing) {
                pb->pause();
            } else if (pb && pb->state() == MediaPlayback::State::Paused) {
                pb->resume();
            } else if ((e = entryFor(*pic, true)) && e->player) {
                e->player->play();
            }
            break;
        }
        m_view->update();
        return true;
    }
    // A click on another shape that the timing says starts a movie or sound.
    if (m_plan.triggers.isEmpty()) return false;
    const QPointF page = toWindow().inverted().map(pos);
    const ItemList &items = m_doc->pages[m_slide]->items;
    for (auto it = items.rbegin(); it != items.rend(); ++it)
        if ((*it)->sourceId > 0 && m_plan.triggers.contains((*it)->sourceId) && (*it)->outline().containsPoint(page, Qt::OddEvenFill)) {
            for (const deck::MediaCue &cue : m_plan.triggers.value((*it)->sourceId)) run(cue);
            return true;
        }
    return false;
}

void ShowMedia::paint(QPainter *p)
{
    if (m_slide < 0 || m_slide >= m_doc->pages.size()) return;
    const QVector<const PictureItem *> pictures = deck::mediaPicturesOf(*m_doc->pages[m_slide]);
    if (pictures.isEmpty()) return;
    const QTransform view = toWindow();
    for (const PictureItem *pic : pictures) {
        const Entry *e = entryOnSlide(pic->id);
        if (!e || !e->player) continue;
        if (e->player->failed()) {
            paintMediaMessage(p, *pic, e->player->error(), view);
        } else if (showsFrame(*e)) {
            if (e->play.fullScreen && e->play.video) {
                p->save();
                p->fillRect(m_view->rect(), Qt::black);
                p->setRenderHint(QPainter::SmoothPixmapTransform);
                QSizeF size = e->player->frame().size();
                size.scale(QSizeF(m_view->size()), Qt::KeepAspectRatio);
                p->drawImage(QRectF(QPointF((m_view->width() - size.width()) / 2, (m_view->height() - size.height()) / 2), size), e->player->frame());
                p->restore();
            } else {
                paintMediaFrame(p, *pic, e->player->frame(), view);
            }
        }
    }
    if (const PictureItem *pic = pictureWithId(m_hover)) {
        MediaBarState s;
        const Entry *e = entryOnSlide(pic->id);
        if (e && e->player) {
            s.state = e->player->state();
            s.played = e->player->played();
            s.length = e->player->length();
            s.volume = e->player->volume();
            s.muted = e->player->muted();
            s.failed = e->player->failed();
        } else {
            const deck::MediaPlay play = m_plan.plays.value(pic->sourceId);
            s.volume = std::clamp(play.volume / 100000.0, 0.0, 1.0);
            s.muted = play.mute;
        }
        MediaBar::paint(p, barFor(*pic), s, m_hoverPart);
    }
}

} // namespace jp
