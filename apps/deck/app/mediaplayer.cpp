#include "app/mediaplayer.h"

#include "app/icons.h"
#include "render/shapes.h"

#include <QAudio>
#include <QAudioDevice>
#include <QAudioOutput>
#include <QCoreApplication>
#include <QEventLoop>
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QRegularExpression>
#include <QTimer>
#include <QUrl>
#include <QVideoFrame>
#include <QVideoSink>
#include <algorithm>
#include <atomic>
#include <cmath>

namespace jp {

namespace {

std::atomic<int> g_decoders{0};

QUrl fileUrl(const QString &extension)
{
    static const QRegularExpression plain(QStringLiteral("^[A-Za-z0-9]{1,8}$"));
    return QUrl::fromLocalFile(QStringLiteral("/media.") + (plain.match(extension).hasMatch() ? extension.toLower() : QStringLiteral("bin")));
}

} // namespace

int MediaPlayback::decoders() { return g_decoders; }

MediaPlayback::MediaPlayback(QObject *parent) : QObject(parent) {}

MediaPlayback::~MediaPlayback()
{
    if (m_player) {
        m_player->stop();
        m_player->setSourceDevice(nullptr);
    }
}

void MediaPlayback::setFile(const QByteArray &bytes, const QString &extension)
{
    m_bytes = bytes;
    m_extension = extension;
}

void MediaPlayback::setPlay(const deck::MediaPlay &play)
{
    m_play = play;
    m_volume = std::clamp(play.volume / 100000.0, 0.0, 1.0);
    m_muted = play.mute;
    applyVolume();
}

void MediaPlayback::setUnavailable(const QString &message) { fail(message); }

qint64 MediaPlayback::trimEnd() const
{
    if (m_play.trimEnd > 0 && (m_fileDuration <= 0 || m_play.trimEnd < m_fileDuration)) return m_play.trimEnd;
    return m_fileDuration;
}

qint64 MediaPlayback::length() const { return std::max<qint64>(0, trimEnd() - trimStart()); }

qint64 MediaPlayback::played() const
{
    if (!m_player) return 0;
    return std::clamp<qint64>(m_player->position() - trimStart(), 0, std::max<qint64>(length(), 0));
}

void MediaPlayback::fail(const QString &message)
{
    if (!m_error.isEmpty()) return;
    m_error = message;
    m_state = State::Stopped;
    Q_EMIT changed();
}

// The player is made only here, when something is played: a file is never read before.
bool MediaPlayback::ensurePlayer()
{
    if (m_player) return true;
    if (m_bytes.isEmpty()) {
        fail(m_play.video ? tr("There is no movie here to play.") : tr("There is no sound here to play."));
        return false;
    }
    m_player = new QMediaPlayer(this);
    ++g_decoders;
    m_sink = new QVideoSink(this);
    m_player->setVideoSink(m_sink);
    // A computer with no sound device still plays the picture.
    if (!QMediaDevices::audioOutputs().isEmpty()) {
        m_audio = new QAudioOutput(this);
        m_player->setAudioOutput(m_audio);
    }
    applyVolume();
    connect(m_sink, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame &f) {
        if (!f.isValid()) return;
        const QImage img = f.toImage();
        if (img.isNull()) return;
        m_frame = img;
        Q_EMIT changed();
    });
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState s) {
        if (!m_error.isEmpty()) return;
        m_state = s == QMediaPlayer::PlayingState ? State::Playing : s == QMediaPlayer::PausedState ? State::Paused : State::Stopped;
        Q_EMIT changed();
    });
    connect(m_player, &QMediaPlayer::durationChanged, this, [this](qint64 d) {
        m_fileDuration = std::max<qint64>(0, d);
        Q_EMIT changed();
    });
    connect(m_player, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        const qint64 end = trimEnd();
        if (m_state == State::Playing && !m_ending && end > 0 && m_play.trimEnd > 0 && position >= end) reachedEnd();
        else if (m_play.fadeIn > 0 || m_play.fadeOut > 0) applyVolume();
        Q_EMIT changed();
    });
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus s) {
        if ((s == QMediaPlayer::LoadedMedia || s == QMediaPlayer::BufferedMedia) && m_seekOnLoad >= 0) {
            const qint64 to = m_seekOnLoad;
            m_seekOnLoad = -1;
            m_player->setPosition(to);
        } else if (s == QMediaPlayer::EndOfMedia) {
            reachedEnd();
        } else if (s == QMediaPlayer::InvalidMedia) {
            fail(m_play.video ? tr("This movie can't be played.") : tr("This sound can't be played."));
        }
    });
    connect(m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &) {
        fail(m_play.video ? tr("This movie can't be played.") : tr("This sound can't be played."));
    });
    m_buffer.close();
    m_buffer.setData(m_bytes);
    m_buffer.open(QIODevice::ReadOnly);
    m_player->setSourceDevice(&m_buffer, fileUrl(m_extension));
    return true;
}

void MediaPlayback::applyVolume()
{
    if (!m_audio) return;
    double fade = 1;
    if (m_player && (m_play.fadeIn > 0 || m_play.fadeOut > 0)) {
        const qint64 at = played(), total = length();
        if (m_play.fadeIn > 0) fade = std::min(fade, double(at) / m_play.fadeIn);
        if (m_play.fadeOut > 0 && total > 0) fade = std::min(fade, double(total - at) / m_play.fadeOut);
        fade = std::clamp(fade, 0.0, 1.0);
    }
    m_audio->setVolume(float(QAudio::convertVolume(m_volume * fade, QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale)));
    m_audio->setMuted(m_muted);
}

void MediaPlayback::seekFile(qint64 fileMs)
{
    if (!m_player) return;
    const QMediaPlayer::MediaStatus s = m_player->mediaStatus();
    if (s == QMediaPlayer::LoadedMedia || s == QMediaPlayer::BufferedMedia || s == QMediaPlayer::EndOfMedia || s == QMediaPlayer::StalledMedia) {
        m_seekOnLoad = -1;
        m_player->setPosition(fileMs);
    } else if (fileMs > 0 || m_seekOnLoad >= 0) {
        m_seekOnLoad = fileMs;
    }
}

void MediaPlayback::play()
{
    if (m_state == State::Playing || failed()) return;
    if (m_state == State::Paused) {
        resume();
        return;
    }
    if (!ensurePlayer()) return;
    m_started = true;
    m_ending = false;
    applyVolume();
    seekFile(trimStart());
    m_player->play();
}

void MediaPlayback::playFrom(double seconds)
{
    if (failed()) return;
    if (!ensurePlayer()) return;
    m_started = true;
    m_ending = false;
    applyVolume();
    seekFile(trimStart() + std::max<qint64>(0, qint64(seconds * 1000)));
    m_player->play();
}

void MediaPlayback::pause()
{
    if (m_player && m_state == State::Playing) m_player->pause();
}

void MediaPlayback::resume()
{
    if (m_player && m_state == State::Paused) m_player->play();
}

void MediaPlayback::togglePause()
{
    if (m_state == State::Playing) pause();
    else if (m_state == State::Paused) resume();
    else play();
}

void MediaPlayback::stop()
{
    if (!m_player) return;
    m_ending = true;
    m_seekOnLoad = -1;
    m_player->stop();
    m_state = State::Stopped;
    m_frame = QImage();
    m_started = false;
    Q_EMIT changed();
}

void MediaPlayback::seek(qint64 playedMs)
{
    if (failed() || !ensurePlayer()) return;
    m_started = true;
    seekFile(trimStart() + std::clamp<qint64>(playedMs, 0, std::max<qint64>(length(), 0)));
    // A movie that isn't playing shows the moment it was moved to.
    if (m_state == State::Stopped) m_player->pause();
    applyVolume();
    Q_EMIT changed();
}

void MediaPlayback::reachedEnd()
{
    if (m_ending || !m_player) return;
    if (m_play.loop) {
        seekFile(trimStart());
        m_player->play();
        return;
    }
    m_ending = true;
    m_player->stop();
    m_state = State::Stopped;
    if (m_play.rewind) {
        m_frame = QImage();
        m_started = false;
    }
    Q_EMIT ended();
    Q_EMIT changed();
}

void MediaPlayback::setVolume(double volume)
{
    m_volume = std::clamp(volume, 0.0, 1.0);
    applyVolume();
    Q_EMIT changed();
}

void MediaPlayback::setMuted(bool muted)
{
    m_muted = muted;
    applyVolume();
    Q_EMIT changed();
}

MediaProbe probeMedia(const QByteArray &bytes, const QString &extension, bool wantFrame, int timeoutMs)
{
    MediaProbe out;
    QBuffer buffer;
    buffer.setData(bytes);
    buffer.open(QIODevice::ReadOnly);
    QVideoSink sink;
    QMediaPlayer player;
    ++g_decoders;
    player.setVideoSink(&sink);
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    bool gotFrame = !wantFrame, gotDuration = false;
    auto check = [&] {
        if (gotFrame && gotDuration) loop.quit();
    };
    QObject::connect(&sink, &QVideoSink::videoFrameChanged, &loop, [&](const QVideoFrame &f) {
        if (gotFrame || !f.isValid()) return;
        out.frame = f.toImage();
        gotFrame = !out.frame.isNull();
        check();
    });
    QObject::connect(&player, &QMediaPlayer::durationChanged, &loop, [&](qint64 d) {
        if (d <= 0) return;
        out.duration = d;
        gotDuration = true;
        check();
    });
    QObject::connect(&player, &QMediaPlayer::errorOccurred, &loop, [&](QMediaPlayer::Error, const QString &) {
        out.error = QCoreApplication::translate("jp::MediaPlayback", "This file can't be read as a movie or sound.");
        loop.quit();
    });
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    player.setSourceDevice(&buffer, fileUrl(extension));
    // Paused, a movie shows its first picture.
    player.pause();
    timeout.start(timeoutMs);
    loop.exec();
    player.stop();
    player.setSourceDevice(nullptr);
    return out;
}

// ---- the control bar ----

namespace {

struct BarLayout {
    QRectF play, seek, time, mute, volume;
};

BarLayout layoutOf(const QRectF &bar)
{
    BarLayout l;
    l.play = QRectF(bar.left() + 3, bar.top() + 3, 24, 24);
    l.volume = QRectF(bar.right() - 8 - 52, bar.center().y() - 10, 52, 20);
    l.mute = QRectF(l.volume.left() - 4 - 24, bar.top() + 3, 24, 24);
    l.time = QRectF(l.mute.left() - 4 - 96, bar.top(), 96, bar.height());
    const double left = l.play.right() + 8;
    l.seek = QRectF(left, bar.center().y() - 10, std::max(0.0, l.time.left() - 4 - left), 20);
    return l;
}

const QColor kInk(0xF4, 0xF6, 0xFA);

void paintSpeaker(QPainter *p, const QRectF &r, bool muted, double level)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    const QPointF c = r.center();
    QPainterPath cone;
    cone.addPolygon(QPolygonF({c + QPointF(-8, -3), c + QPointF(-4, -3), c + QPointF(1, -7), c + QPointF(1, 7), c + QPointF(-4, 3), c + QPointF(-8, 3)}));
    cone.closeSubpath();
    p->setPen(Qt::NoPen);
    p->setBrush(kInk);
    p->drawPath(cone);
    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(kInk, 1.6, Qt::SolidLine, Qt::RoundCap));
    if (muted || level <= 0) {
        p->drawLine(c + QPointF(4, -3), c + QPointF(9, 3));
        p->drawLine(c + QPointF(9, -3), c + QPointF(4, 3));
    } else {
        p->drawArc(QRectF(c.x() - 3, c.y() - 4, 8, 8), -50 * 16, 100 * 16);
        if (level > 0.5) p->drawArc(QRectF(c.x() - 5, c.y() - 7, 14, 14), -50 * 16, 100 * 16);
    }
    p->restore();
}

} // namespace

QString MediaBar::timeText(qint64 ms)
{
    const qint64 s = std::max<qint64>(0, ms) / 1000;
    if (s >= 3600) return QStringLiteral("%1:%2:%3").arg(s / 3600).arg((s / 60) % 60, 2, 10, QLatin1Char('0')).arg(s % 60, 2, 10, QLatin1Char('0'));
    return QStringLiteral("%1:%2").arg(s / 60).arg(s % 60, 2, 10, QLatin1Char('0'));
}

QRectF MediaBar::placement(const QRectF &picture, const QRectF &area)
{
    const double w = std::min(std::max(kMinWidth, picture.width()), std::max(1.0, area.width()));
    double x = picture.center().x() - w / 2;
    x = std::clamp(x, area.left(), std::max(area.left(), area.right() - w));
    double y = picture.bottom() + 6;
    if (y + kHeight > area.bottom()) y = picture.bottom() - kHeight - 6;
    y = std::clamp(y, area.top(), std::max(area.top(), area.bottom() - kHeight));
    return QRectF(x, y, w, kHeight);
}

MediaBar::Part MediaBar::partAt(const QRectF &bar, const QPointF &pos)
{
    if (!bar.contains(pos)) return Part::None;
    const BarLayout l = layoutOf(bar);
    if (l.play.adjusted(-2, -3, 2, 3).contains(pos)) return Part::Play;
    if (l.seek.adjusted(-4, 0, 4, 0).contains(pos)) return Part::Seek;
    if (l.mute.contains(pos)) return Part::Mute;
    if (l.volume.adjusted(-4, 0, 4, 0).contains(pos)) return Part::Volume;
    return Part::None;
}

qint64 MediaBar::seekAt(const QRectF &bar, const QPointF &pos, qint64 length)
{
    const BarLayout l = layoutOf(bar);
    if (l.seek.width() <= 0) return 0;
    return qint64(std::clamp((pos.x() - l.seek.left()) / l.seek.width(), 0.0, 1.0) * double(std::max<qint64>(length, 0)));
}

double MediaBar::volumeAt(const QRectF &bar, const QPointF &pos)
{
    const BarLayout l = layoutOf(bar);
    return std::clamp((pos.x() - l.volume.left()) / l.volume.width(), 0.0, 1.0);
}

void MediaBar::paint(QPainter *p, const QRectF &bar, const MediaBarState &s, Part hover)
{
    const BarLayout l = layoutOf(bar);
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(QPen(QColor(255, 255, 255, 60), 1));
    p->setBrush(QColor(0x1E, 0x22, 0x29, 235));
    p->drawRoundedRect(bar.adjusted(0.5, 0.5, -0.5, -0.5), 7, 7);
    // Play or pause.
    if (hover == Part::Play) {
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(255, 255, 255, 40));
        p->drawRoundedRect(l.play, 5, 5);
    }
    p->setPen(Qt::NoPen);
    p->setBrush(s.failed ? QColor(255, 255, 255, 90) : kInk);
    const QPointF c = l.play.center();
    if (s.state == MediaPlayback::State::Playing) {
        p->drawRoundedRect(QRectF(c.x() - 5.5, c.y() - 6, 4, 12), 1, 1);
        p->drawRoundedRect(QRectF(c.x() + 1.5, c.y() - 6, 4, 12), 1, 1);
    } else {
        p->drawPolygon(QPolygonF({c + QPointF(-4, -7), c + QPointF(-4, 7), c + QPointF(7, 0)}));
    }
    // The seek bar.
    if (l.seek.width() > 8) {
        const QRectF track(l.seek.left(), l.seek.center().y() - 3, l.seek.width(), 6);
        p->setBrush(QColor(255, 255, 255, 70));
        p->drawRoundedRect(track, 3, 3);
        const double share = s.length > 0 ? std::clamp(double(s.played) / double(s.length), 0.0, 1.0) : 0.0;
        const QColor accent = uiAccent().lighter(135);
        p->setBrush(accent);
        p->drawRoundedRect(QRectF(track.left(), track.top(), std::max(6.0, track.width() * share), track.height()), 3, 3);
        p->setBrush(kInk);
        p->drawEllipse(QPointF(track.left() + track.width() * share, track.center().y()), hover == Part::Seek ? 6.0 : 5.0, hover == Part::Seek ? 6.0 : 5.0);
    }
    // The time: where it is, and how long.
    QFont f = p->font();
    f.setPixelSize(11);
    p->setFont(f);
    p->setPen(kInk);
    p->drawText(l.time, Qt::AlignRight | Qt::AlignVCenter, MediaBar::timeText(s.played) + QStringLiteral(" / ") + MediaBar::timeText(s.length));
    // Mute, and volume.
    if (hover == Part::Mute) {
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(255, 255, 255, 40));
        p->drawRoundedRect(l.mute, 5, 5);
    }
    paintSpeaker(p, l.mute, s.muted, s.volume);
    const QRectF vt(l.volume.left(), l.volume.center().y() - 2.5, l.volume.width(), 5);
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(255, 255, 255, 70));
    p->drawRoundedRect(vt, 2.5, 2.5);
    const double level = s.muted ? 0.0 : std::clamp(s.volume, 0.0, 1.0);
    p->setBrush(kInk);
    p->drawRoundedRect(QRectF(vt.left(), vt.top(), std::max(5.0, vt.width() * level), vt.height()), 2.5, 2.5);
    p->drawEllipse(QPointF(vt.left() + vt.width() * level, vt.center().y()), hover == Part::Volume ? 5.0 : 4.0, hover == Part::Volume ? 5.0 : 4.0);
    p->restore();
}

// ---- over the picture ----

void paintMediaFrame(QPainter *p, const PictureItem &pic, const QImage &frame, const QTransform &toDevice)
{
    if (frame.isNull()) return;
    p->save();
    p->setWorldTransform(pic.transform() * toDevice, true);
    const QRectF r(QPointF(0, 0), pic.rect.size());
    p->setClipPath(shapePath(pic.maskShape, r.size()), Qt::IntersectClip);
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->fillRect(pic.imgRect.united(r), Qt::black);
    p->drawImage(pic.imgRect.isEmpty() ? r : pic.imgRect, frame);
    p->restore();
}

void paintMediaMessage(QPainter *p, const PictureItem &pic, const QString &message, const QTransform &toDevice)
{
    p->save();
    const QPolygonF shape = (pic.transform() * toDevice).map(QPolygonF(QRectF(QPointF(0, 0), pic.rect.size())));
    p->setRenderHint(QPainter::Antialiasing);
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(0x1E, 0x22, 0x29, 225));
    p->drawPolygon(shape);
    const QRectF box = shape.boundingRect().adjusted(8, 8, -8, -8);
    QFont f = p->font();
    f.setPixelSize(int(std::clamp(box.height() / 6, 11.0, 22.0)));
    p->setFont(f);
    p->setPen(kInk);
    p->drawText(box, Qt::AlignCenter | Qt::TextWordWrap, message);
    p->restore();
}

} // namespace jp
