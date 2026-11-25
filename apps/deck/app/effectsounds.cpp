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
    // Only so many sounds play together, and a step starts only so many, however many its effects name: each would be a decoder started on the thread that draws.
    m_players.removeIf([](const QPointer<MediaPlayback> &p) { return p.isNull(); });
    if (m_startedInStep >= kMostInStep || (m_output ? m_outputPlaying : int(m_players.size())) >= kMostTogether) {
        ++m_skipped;
        return;
    }
    ++m_startedInStep;
    if (m_output) {
        Request r;
        r.media = mediaId;
        r.bytes = data->bytes;
        r.extension = extension;
        ++m_outputPlaying;
        m_output(r);
        return;
    }
    auto *pb = new MediaPlayback(this);
    pb->setFile(data->bytes, extension);
    connect(pb, &MediaPlayback::ended, pb, [this, pb] {
        m_players.removeAll(QPointer<MediaPlayback>(pb));
        pb->deleteLater();
    });
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
    m_outputPlaying = 0;
    for (const QPointer<MediaPlayback> &p : std::as_const(m_players)) {
        if (!p) continue;
        p->stop();
        p->deleteLater();
    }
    m_players.clear();
}

} // namespace jp
