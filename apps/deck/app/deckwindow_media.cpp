// Movies and sounds in JeffDeck's window: Insert > Video and Audio, and the
// Playback tab, which shows while one is selected.

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/deckinfo.h"
#include "app/editor.h"
#include "app/editormedia.h"
#include "app/icons.h"
#include "app/mediadialogs.h"
#include "app/mediaplayer.h"
#include "app/ribbon.h"
#include "app/ribbonbuilder.h"
#include "app/settings.h"
#include "core/media.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <algorithm>

namespace jp {

namespace {

// The most a movie or sound put on a slide may be: it is kept in memory with the presentation, and written into its file.
constexpr qint64 kMaxMediaBytes = qint64(1) << 30;

PictureItem *selectedMedia(Editor *ed)
{
    if (!ed->masterView().isEmpty()) return nullptr;
    Item *one = ed->single();
    if (!one || one->type() != ItemType::Picture) return nullptr;
    auto *pic = static_cast<PictureItem *>(one);
    return pic->mediaKind.isEmpty() || (pic->mediaId.isEmpty() && pic->mediaLink.isEmpty()) ? nullptr : pic;
}

deck::MediaPlay currentPlay(Editor *ed)
{
    const PictureItem *pic = selectedMedia(ed);
    return pic ? deck::mediaPlayOf(*ed->doc()->pages[ed->currentPage()], *pic) : deck::MediaPlay();
}

} // namespace

void DeckWindow::createMediaActions()
{
    mk("insert.video", tr("Video on This Device…"), "video", QKeySequence(), [this] { insertMediaFromFile(QStringLiteral("video")); });
    mk("insert.audio", tr("Audio on This Device…"), "volume-2", QKeySequence(), [this] { insertMediaFromFile(QStringLiteral("audio")); });
    mk("playback.play", tr("Play"), "play", QKeySequence(), [this] { playMedia(); });
    mk("playback.trim", tr("Trim"), "scissors", QKeySequence(), [this] { trimMedia(); });
    auto sync = [this] { refreshMedia(); };
    connect(m_ed, &Editor::selectionChanged, this, sync);
    connect(m_ed, &Editor::changed, this, sync);
    connect(m_ed, &Editor::pageChanged, this, sync);
    connect(m_ed, &Editor::documentReplaced, this, sync);
    connect(m_canvas->media(), &EditorMedia::playbackChanged, this, sync);
}

// The Playback tab shows while one movie or sound is selected, and Play says what it will do.
void DeckWindow::refreshMedia()
{
    const PictureItem *pic = selectedMedia(m_ed);
    if (m_ribbon) m_ribbon->setContextVisible(QStringLiteral("Media Tools"), pic != nullptr);
    act("playback.play")->setEnabled(pic != nullptr);
    act("playback.trim")->setEnabled(pic != nullptr);
    const MediaPlayback *pb = pic ? m_canvas->media()->playbackOf(pic->id) : nullptr;
    const bool playing = pb && pb->state() == MediaPlayback::State::Playing;
    QAction *play = act("playback.play");
    if (play->property("playing").toBool() != playing || play->icon().isNull()) {
        play->setProperty("playing", playing);
        play->setText(playing ? tr("Pause") : tr("Play"));
        play->setIcon(icon(playing ? "pause" : "play"));
    }
    act("playback.trim")->setText(pic && pic->mediaKind == QLatin1String("audio") ? tr("Trim Audio") : tr("Trim Video"));
}

void DeckWindow::insertMediaFromFile(const QString &kind)
{
    const bool video = kind == QLatin1String("video");
    const QString fallback = QStandardPaths::writableLocation(video ? QStandardPaths::MoviesLocation : QStandardPaths::MusicLocation);
    const QString dir = Settings::get().value(QStringLiteral("dirs/media"), fallback).toString();
    QStringList patterns;
    for (const QString &ext : deck::mediaExtensions(kind)) patterns << QStringLiteral("*.") + ext;
    const QString filter = (video ? tr("Video Files (%1)") : tr("Audio Files (%1)")).arg(patterns.join(QLatin1Char(' '))) + QStringLiteral(";;") + tr("All Files (*)");
    const QString path = QFileDialog::getOpenFileName(this, video ? tr("Insert Video") : tr("Insert Audio"), dir, filter);
    if (path.isEmpty()) return;
    Settings::get().setValue(QStringLiteral("dirs/media"), QFileInfo(path).absolutePath());
    insertMedia(path, kind);
}

// The file is put on the slide as a picture that plays it: kept in the presentation, with its first picture (a sound's is a
// speaker) and the timing that plays it in the click sequence. This is the one place a file is decoded for a person who
// chose it, to get that picture and how long it lasts.
bool DeckWindow::insertMedia(const QString &path, const QString &kind)
{
    const QString name = QFileInfo(path).fileName();
    const QString found = deck::mediaKindOfFile(path);
    if (found.isEmpty() || (!kind.isEmpty() && found != kind)) {
        QMessageBox::warning(this, kind == QLatin1String("audio") ? tr("Insert Audio") : tr("Insert Video"),
                             tr("%1 isn't a %2 file that %3 can put on a slide.").arg(name, kind == QLatin1String("audio") ? tr("sound") : tr("movie"), appInfo().name));
        return false;
    }
    const bool video = found == QLatin1String("video");
    const QString title = video ? tr("Insert Video") : tr("Insert Audio");
    QFile f(path);
    if (QFileInfo(path).size() > kMaxMediaBytes) {
        QMessageBox::warning(this, title, tr("%1 is too large to keep in a presentation (the most is 1 GB).").arg(name));
        return false;
    }
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, title, tr("%1 can't open \"%2\".\n%3").arg(appInfo().name, name, f.errorString()));
        return false;
    }
    const QByteArray bytes = f.readAll();
    f.close();
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    const MediaProbe probe = probeMedia(bytes, QFileInfo(path).suffix(), video);
    QGuiApplication::restoreOverrideCursor();
    if (!probe.error.isEmpty() && probe.duration <= 0 && probe.frame.isNull()) {
        QMessageBox::warning(this, title, tr("%1 can't read \"%2\" as a %3, so it isn't put on the slide.").arg(appInfo().name, name, video ? tr("movie") : tr("sound")));
        return false;
    }
    Document *d = m_ed->doc();
    const int page = m_ed->currentPage();
    if (!m_ed->masterView().isEmpty() || page < 0 || page >= d->pages.size()) return false;
    m_ed->endTextEdit();
    m_ed->beginChange(title);
    auto pic = deck::newMediaPicture(*d, *d->pages[page], bytes, path, probe.frame, probe.duration);
    if (pic) m_ed->surfaceItems().push_back(pic);
    m_ed->endChange();
    if (!pic) return false;
    m_ed->select(pic->id);
    return true;
}

// A change to how the selected movie or sound plays, as one step that undoes. The timing is rewritten only for what changed.
void DeckWindow::editMedia(const QString &label, const std::function<void(deck::MediaPlay &)> &change)
{
    PictureItem *pic = selectedMedia(m_ed);
    if (!pic) return;
    Document *d = m_ed->doc();
    Page &slide = *d->pages[m_ed->currentPage()];
    const deck::MediaPlay before = deck::mediaPlayOf(slide, *pic);
    deck::MediaPlay play = before;
    change(play);
    play.shape = before.shape;
    play.video = before.video;
    if (play == before && before.shape > 0) return;
    const QString id = pic->id;
    m_ed->endTextEdit();
    m_ed->beginChange(label);
    // The Playback tab's changes find the picture again: the change may have copied the document.
    if (PictureItem *now = selectedMedia(m_ed)) deck::setMediaPlay(*d->pages[m_ed->currentPage()], *now, play);
    m_ed->endChange();
    m_canvas->media()->refresh(id);
}

void DeckWindow::playMedia()
{
    const PictureItem *pic = selectedMedia(m_ed);
    if (!pic) return;
    const MediaPlayback *pb = m_canvas->media()->playbackOf(pic->id);
    if (pb && pb->state() == MediaPlayback::State::Playing) m_canvas->media()->pause();
    else m_canvas->media()->play(pic->id);
    refreshMedia();
}

// How long the file lasts, in milliseconds, for the Trim box: from the player if it has been asked, else by reading the file.
static qint64 mediaLength(const Document &d, const PictureItem &pic, const MediaPlayback *pb)
{
    if (pb && pb->length() > 0 && pb->settings().trimStart == 0 && pb->settings().trimEnd == 0) return pb->length();
    const auto data = d.media.constFind(pic.mediaId);
    if (data == d.media.cend()) return 0;
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    const MediaProbe probe = probeMedia(data->bytes, data->extension, false);
    QGuiApplication::restoreOverrideCursor();
    return probe.duration;
}

void DeckWindow::trimMedia()
{
    const PictureItem *pic = selectedMedia(m_ed);
    if (!pic) return;
    const deck::MediaPlay play = currentPlay(m_ed);
    const qint64 length = mediaLength(*m_ed->doc(), *pic, m_canvas->media()->playbackOf(pic->id));
    MediaTrimDialog dlg(pic->mediaKind == QLatin1String("audio") ? tr("Trim Audio") : tr("Trim Video"), length, play.trimStart, play.trimEnd, this);
    if (dlg.exec() != QDialog::Accepted) return;
    const int start = dlg.startMs(), end = dlg.endMs();
    editMedia(tr("Trim"), [start, end](deck::MediaPlay &p) {
        p.trimStart = start;
        p.trimEnd = end;
    });
}

// The Playback tab's boxes: each reads the selected movie or sound again whenever the selection or the slide changes.
void DeckWindow::addMediaParts(RibbonParts &parts)
{
    Editor *ed = m_ed;
    parts.colors[QStringLiteral("playback")] = QColor(0x2F, 0x7D, 0x4F);
    auto syncs = std::make_shared<QList<std::function<void()>>>();
    auto syncAll = [syncs] {
        for (const auto &fn : *syncs) fn();
    };
    connect(ed, &Editor::changed, this, syncAll);
    connect(ed, &Editor::selectionChanged, this, syncAll);
    connect(ed, &Editor::pageChanged, this, syncAll);
    connect(ed, &Editor::documentReplaced, this, syncAll);

    auto check = [this, ed, syncs](const char *name, const QString &text, const QString &tip, const std::function<bool(const deck::MediaPlay &)> &read,
                                   const std::function<void(deck::MediaPlay &, bool)> &write, bool videoOnly) {
        auto *box = new QCheckBox(text);
        box->setObjectName(QLatin1String(name));
        box->setToolTip(tip);
        connect(box, &QCheckBox::toggled, this, [this, write, text](bool on) {
            editMedia(text.isEmpty() ? tr("Media Options") : text, [write, on](deck::MediaPlay &p) { write(p, on); });
        });
        syncs->append([box, ed, read, videoOnly] {
            const QSignalBlocker block(box);
            const deck::MediaPlay p = currentPlay(ed);
            box->setChecked(read(p));
            box->setEnabled(selectedMedia(ed) && (!videoOnly || p.video));
        });
        syncs->last()();
        return box;
    };
    parts.widgets[QStringLiteral("mediaFullScreen")] = [check, this]() -> QWidget * {
        return check("mediaFullScreen", tr("Play Full Screen"), tr("Play Full Screen: in the slide show, the movie fills the screen while it plays."),
                     [](const deck::MediaPlay &p) { return p.fullScreen; }, [](deck::MediaPlay &p, bool on) { p.fullScreen = on; }, true);
    };
    parts.widgets[QStringLiteral("mediaHide")] = [check, this]() -> QWidget * {
        return check("mediaHide", tr("Hide While Not Playing"), tr("Hide While Not Playing: in the slide show, the picture shows only while the movie or sound plays."),
                     [](const deck::MediaPlay &p) { return p.hideWhenStopped; }, [](deck::MediaPlay &p, bool on) { p.hideWhenStopped = on; }, false);
    };
    parts.widgets[QStringLiteral("mediaLoop")] = [check, this]() -> QWidget * {
        return check("mediaLoop", tr("Loop until Stopped"), tr("Loop until Stopped: it plays again each time it ends, until it is stopped or the slide changes."),
                     [](const deck::MediaPlay &p) { return p.loop; }, [](deck::MediaPlay &p, bool on) { p.loop = on; }, false);
    };
    parts.widgets[QStringLiteral("mediaRewind")] = [check, this]() -> QWidget * {
        return check("mediaRewind", tr("Rewind after Playing"), tr("Rewind after Playing: when it ends, a movie goes back to its first picture."),
                     [](const deck::MediaPlay &p) { return p.rewind; }, [](deck::MediaPlay &p, bool on) { p.rewind = on; }, false);
    };
    parts.widgets[QStringLiteral("mediaVolume")] = [this, ed, syncs]() -> QWidget * {
        auto *box = new QComboBox();
        box->setObjectName(QStringLiteral("mediaVolume"));
        box->setAccessibleName(tr("Volume"));
        box->setToolTip(tr("Volume: how loud it plays."));
        // The values are the file's: thousandths of a percent; mute is -1.
        box->addItem(tr("Low"), deck::kVolumeLow);
        box->addItem(tr("Medium"), deck::kVolumeMedium);
        box->addItem(tr("High"), deck::kVolumeHigh);
        box->addItem(tr("Mute"), -1);
        connect(box, &QComboBox::activated, this, [this, box](int i) {
            const int v = box->itemData(i).toInt();
            editMedia(tr("Volume"), [v](deck::MediaPlay &p) {
                p.mute = v < 0;
                if (v >= 0) p.volume = v;
            });
        });
        syncs->append([box, ed] {
            const QSignalBlocker block(box);
            const deck::MediaPlay p = currentPlay(ed);
            box->setEnabled(selectedMedia(ed) != nullptr);
            int at = 3;
            if (!p.mute) at = p.volume <= (deck::kVolumeLow + deck::kVolumeMedium) / 2 ? 0 : p.volume <= (deck::kVolumeMedium + deck::kVolumeHigh) / 2 ? 1 : 2;
            box->setCurrentIndex(at);
        });
        syncs->last()();
        return box;
    };
    parts.widgets[QStringLiteral("mediaStart")] = [this, ed, syncs]() -> QWidget * {
        auto *box = new QComboBox();
        box->setObjectName(QStringLiteral("mediaStart"));
        box->setAccessibleName(tr("Start"));
        box->setToolTip(tr("Start: when it plays in the slide show, in the click sequence, by itself when its place in the animations comes, or when it is clicked on."));
        box->addItem(tr("In Click Sequence"), int(deck::MediaStart::ClickSequence));
        box->addItem(tr("Automatically"), int(deck::MediaStart::Automatically));
        box->addItem(tr("When Clicked On"), int(deck::MediaStart::WhenClickedOn));
        connect(box, &QComboBox::activated, this, [this, box](int i) {
            const auto start = deck::MediaStart(box->itemData(i).toInt());
            editMedia(tr("Start"), [start](deck::MediaPlay &p) { p.start = start; });
        });
        syncs->append([box, ed] {
            const QSignalBlocker block(box);
            box->setEnabled(selectedMedia(ed) != nullptr);
            box->setCurrentIndex(std::clamp(int(currentPlay(ed).start), 0, 2));
        });
        syncs->last()();
        return box;
    };
}

} // namespace jp
