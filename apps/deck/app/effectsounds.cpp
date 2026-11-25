#include "app/effectsounds.h"

#include "app/mediaplayer.h"
#include "core/document.h"
#include "core/media.h"

namespace jp {

EffectSounds::EffectSounds(const Document *doc, QObject *parent) : QObject(parent), m_doc(doc) {}

void EffectSounds::rebind(const Document *doc)
{
    stopAll();
    m_doc = doc;
}

void EffectSounds::cue(const deck::SoundCue &c)
{
    if (c.kind == deck::SoundKind::StopPrevious) stopAll();
    else if (c.kind == deck::SoundKind::File) play(c.media);
}

void EffectSounds::play(const QString &mediaId)
{
    if (!m_doc) return;
    const auto data = m_doc->media.constFind(mediaId);
    if (data == m_doc->media.cend() || data->bytes.isEmpty()) return;
    const QString extension = deck::mediaExtensionFor(data->extension);
    // A step starts only so many sounds, however many its effects name: each would be a decoder started on the thread that draws.
    if (m_startedInStep >= kMostTogether) {
        ++m_skipped;
        return;
    }
    ++m_startedInStep;
    if (m_output) {
        Request r;
        r.media = mediaId;
        r.bytes = data->bytes;
        r.extension = extension;
        m_output(r);
        return;
    }
    // The oldest stops when there are too many.
    m_players.removeIf([](const QPointer<MediaPlayback> &p) { return p.isNull(); });
    while (m_players.size() >= kMostTogether) {
        if (m_players.first()) {
            m_players.first()->stop();
            m_players.first()->deleteLater();
        }
        m_players.removeFirst();
    }
    auto *pb = new MediaPlayback(this);
    pb->setFile(data->bytes, extension);
    connect(pb, &MediaPlayback::ended, pb, &QObject::deleteLater);
    // A sound that can't be played does not stay.
    connect(pb, &MediaPlayback::changed, pb, [this, pb] {
        if (pb->failed()) drop(pb);
    });
    m_players.append(pb);
    pb->play();
}

void EffectSounds::drop(MediaPlayback *player)
{
    if (!m_players.removeAll(QPointer<MediaPlayback>(player))) return;
    ++m_failed;
    player->stop();
    player->deleteLater();
}

void EffectSounds::stopAll()
{
    if (m_output) {
        Request r;
        r.kind = Request::Kind::StopAll;
        m_output(r);
    }
    for (const QPointer<MediaPlayback> &p : std::as_const(m_players)) {
        if (!p) continue;
        p->stop();
        p->deleteLater();
    }
    m_players.clear();
}

} // namespace jp
