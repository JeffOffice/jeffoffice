#include "core/captions.h"

#include <QCoreApplication>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include <atomic>

namespace jp::deck {

static std::atomic<int> s_parses{0};
int captionParseCount() { return s_parses.load(); }

namespace {

// The bytes of a WebVTT file begin with an optional byte-order mark and "WEBVTT", and then the line's end, a space, or a tab.
qsizetype headerEnd(const QByteArray &bytes)
{
    qsizetype at = bytes.startsWith("\xEF\xBB\xBF") ? 3 : 0;
    if (bytes.mid(at, 6) != "WEBVTT") return -1;
    at += 6;
    if (at < bytes.size() && bytes[at] != '\n' && bytes[at] != '\r' && bytes[at] != ' ' && bytes[at] != '\t') return -1;
    return at;
}

// "[hh:]mm:ss.ttt" in milliseconds; -1 when it is not one.
qint64 timestamp(const QStringView text)
{
    static const QRegularExpression pattern(QStringLiteral("^(?:(\\d{1,6}):)?(\\d{2}):(\\d{2})\\.(\\d{3})$"));
    const QRegularExpressionMatch m = pattern.matchView(text);
    if (!m.hasMatch()) return -1;
    const qint64 hours = m.hasCaptured(1) ? m.captured(1).toLongLong() : 0, minutes = m.captured(2).toLongLong(), seconds = m.captured(3).toLongLong();
    if (minutes > 59 || seconds > 59) return -1;
    return ((hours * 60 + minutes) * 60 + seconds) * 1000 + m.captured(4).toLongLong();
}

// The text of a cue as it shows: the tags (<c>, <v Name>, <b>, a timestamp tag) taken out, and the character references of the format read.
QString plainCueText(const QString &raw)
{
    QString out;
    out.reserve(std::min<qsizetype>(raw.size(), kMaxCueChars + 8));
    bool inTag = false;
    for (qsizetype i = 0; i < raw.size() && out.size() < kMaxCueChars; ++i) {
        const QChar c = raw[i];
        if (inTag) {
            if (c == QLatin1Char('>')) inTag = false;
            continue;
        }
        if (c == QLatin1Char('<')) {
            inTag = true;
            continue;
        }
        if (c == QLatin1Char('&')) {
            static const struct { const char *name; const char *text; } known[] = {{"&amp;", "&"}, {"&lt;", "<"}, {"&gt;", ">"}, {"&nbsp;", " "}, {"&lrm;", "‎"}, {"&rlm;", "‏"}};
            bool done = false;
            for (const auto &k : known)
                if (raw.mid(i, 8).startsWith(QLatin1String(k.name))) {
                    out += QString::fromUtf8(k.text);
                    i += int(qstrlen(k.name)) - 1;
                    done = true;
                    break;
                }
            if (done) continue;
        }
        out += c;
    }
    return out.trimmed();
}

// The cues of a file, read once while its bytes stay the same (found by what they hold, so that a document's ids, which another document
// may use for other bytes, do not matter). A few files are kept, the latest.
QVector<CaptionCue> cuesOfBytes(const QByteArray &bytes)
{
    // (Guarded: an export, a thumbnail, or the slide sorter may ask from another thread. At most 8 files are held, each of at most
    // kMaxCaptionCues cues of at most kMaxCueChars characters, so what is held is bounded.)
    static QMutex lock;
    static QHash<quint64, QVector<CaptionCue>> known;
    const quint64 key = (quint64(qHash(bytes, 0x5EEDu)) << 32 | quint64(qHash(bytes, 0xCAFEu))) ^ quint64(bytes.size());
    {
        const QMutexLocker locked(&lock);
        if (const auto it = known.constFind(key); it != known.constEnd()) return *it;
    }
    const CaptionFile f = parseWebVtt(bytes);
    const QMutexLocker locked(&lock);
    if (known.size() >= 8) known.clear();
    known.insert(key, f.cues);
    return f.cues;
}

// Takes the files of tracks that were taken off out of the document's media, unless a movie or a track still names one (the same file may
// be a track of several videos, as the media store keeps one copy of equal bytes).
void dropUnusedMedia(Document &doc, const QStringList &ids)
{
    QSet<QString> wanted;
    for (const QString &id : ids)
        if (!id.isEmpty() && doc.media.contains(id)) wanted.insert(id);
    if (wanted.isEmpty()) return;
    auto scan = [&](const ItemList &items) {
        walkItems(items, [&](const ItemPtr &it) {
            if (it->type() != ItemType::Picture) return;
            const auto &pic = static_cast<const PictureItem &>(*it);
            wanted.remove(pic.mediaId);
            for (const PictureItem::Caption &c : pic.mediaCaptions) wanted.remove(c.mediaId);
        });
    };
    for (const auto &page : doc.pages) scan(page->items);
    for (const auto &master : doc.masters) scan(master->items);
    for (const QString &id : std::as_const(wanted)) doc.media.remove(id);
}

} // namespace

bool looksLikeWebVtt(const QByteArray &bytes) { return headerEnd(bytes) >= 0; }

CaptionFile parseWebVtt(const QByteArray &bytes)
{
    ++s_parses;
    CaptionFile f;
    if (bytes.size() > kMaxCaptionFileBytes) {
        f.error = QCoreApplication::translate("Captions", "The caption file is too large (the most is %1 MB).").arg(kMaxCaptionFileBytes / (1024 * 1024));
        return f;
    }
    if (headerEnd(bytes) < 0) {
        f.error = QCoreApplication::translate("Captions", "This isn't a WebVTT caption file. It should start with the word WEBVTT.");
        return f;
    }
    QString text = QString::fromUtf8(bytes);
    if (text.startsWith(QChar(0xFEFF))) text.remove(0, 1);
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n")).replace(QLatin1Char('\r'), QLatin1Char('\n'));
    const QList<QStringView> lines = QStringView(text).split(QLatin1Char('\n'));
    // Past the header: its line and the lines under it, up to the first blank one.
    qsizetype i = 1;
    while (i < lines.size() && !lines[i].trimmed().isEmpty()) ++i;
    while (i < lines.size()) {
        while (i < lines.size() && lines[i].trimmed().isEmpty()) ++i;
        if (i >= lines.size()) break;
        // One block, up to the next blank line.
        const qsizetype first = i;
        while (i < lines.size() && !lines[i].trimmed().isEmpty()) ++i;
        qsizetype at = first;
        const QStringView head = lines[first];
        if (head.startsWith(QLatin1String("NOTE")) && (head.size() == 4 || head[4] == QLatin1Char(' ') || head[4] == QLatin1Char('\t'))) continue;
        if (head.trimmed() == QLatin1String("STYLE") || head.trimmed() == QLatin1String("REGION")) continue;
        // An identifier line may come before the timing line.
        if (!lines[at].contains(QLatin1String("-->")) && at + 1 < i) ++at;
        const QStringView timing = lines[at];
        const qsizetype arrow = timing.indexOf(QLatin1String("-->"));
        if (arrow < 0) {
            ++f.leftOut;
            continue;
        }
        const QStringView startText = timing.left(arrow).trimmed();
        QStringView rest = timing.mid(arrow + 3).trimmed();
        const qsizetype space = rest.indexOf(QLatin1Char(' '));
        const qsizetype tab = rest.indexOf(QLatin1Char('\t'));
        const qsizetype cut = space < 0 ? tab : (tab < 0 ? space : std::min(space, tab));
        const qint64 start = timestamp(startText), end = timestamp(cut < 0 ? rest : rest.left(cut));
        if (start < 0 || end < 0 || end <= start || at + 1 >= i) {
            ++f.leftOut;
            continue;
        }
        if (f.cues.size() >= kMaxCaptionCues) {
            ++f.leftOut;
            continue;
        }
        QString body;
        for (qsizetype k = at + 1; k < i && body.size() < kMaxCueChars * 2; ++k) {
            if (k > at + 1) body += QLatin1Char('\n');
            body += lines[k];
        }
        CaptionCue cue;
        cue.start = start;
        cue.end = end;
        cue.text = plainCueText(body);
        if (cue.text.isEmpty()) {
            ++f.leftOut;
            continue;
        }
        f.cues.push_back(cue);
    }
    std::stable_sort(f.cues.begin(), f.cues.end(), [](const CaptionCue &a, const CaptionCue &b) { return a.start < b.start; });
    if (f.cues.isEmpty()) f.error = QCoreApplication::translate("Captions", "This WebVTT file has no captions that can be shown.");
    return f;
}

QString captionTextAt(const QVector<CaptionCue> &cues, qint64 ms)
{
    QStringList shown;
    for (const CaptionCue &c : cues) {
        if (c.start > ms) break;   // by start
        if (ms < c.end && shown.size() < kMaxActiveCues) shown << c.text;
    }
    return shown.join(QLatin1Char('\n'));
}

bool validCaptionLanguage(const QString &language)
{
    static const QRegularExpression tag(QStringLiteral("^[A-Za-z0-9]+(-[A-Za-z0-9]+)*$"));
    return language.size() >= 1 && language.size() <= 35 && tag.match(language).hasMatch();
}

QString addCaptions(Document &doc, PictureItem &video, const QByteArray &bytes, const QString &language)
{
    if (video.mediaKind != QLatin1String("video")) return QCoreApplication::translate("Captions", "Captions go with a video.");
    if (!validCaptionLanguage(language))
        return QCoreApplication::translate("Captions", "\"%1\" isn't a language code. Use a short code such as en, es, or pt-BR.").arg(language);
    const CaptionFile file = parseWebVtt(bytes);
    if (!file.error.isEmpty()) return file.error;
    int at = -1;
    for (int i = 0; i < video.mediaCaptions.size(); ++i)
        if (video.mediaCaptions[i].language.compare(language, Qt::CaseInsensitive) == 0) at = i;
    if (at < 0 && video.mediaCaptions.size() >= kMaxMediaCaptions)
        return QCoreApplication::translate("Captions", "A video can have captions in at most %1 languages.").arg(kMaxMediaCaptions);
    const QString id = doc.addMedia(bytes, QStringLiteral("text/vtt"), QStringLiteral("vtt"));
    if (id.isEmpty()) return QCoreApplication::translate("Captions", "The captions could not be kept.");
    const QString old = at >= 0 ? video.mediaCaptions[at].mediaId : QString();
    if (at >= 0) video.mediaCaptions[at] = {language, id};
    else video.mediaCaptions.push_back({language, id});
    dropUnusedMedia(doc, {old});
    return QString();
}

bool removeCaptions(Document &doc, PictureItem &video, const QString &language)
{
    QStringList gone;
    for (auto it = video.mediaCaptions.begin(); it != video.mediaCaptions.end();) {
        if (it->language.compare(language, Qt::CaseInsensitive) == 0) {
            gone << it->mediaId;
            it = video.mediaCaptions.erase(it);
        } else {
            ++it;
        }
    }
    dropUnusedMedia(doc, gone);
    return !gone.isEmpty();
}

bool removeAllCaptions(Document &doc, PictureItem &video)
{
    QStringList gone;
    for (const PictureItem::Caption &c : video.mediaCaptions) gone << c.mediaId;
    video.mediaCaptions.clear();
    dropUnusedMedia(doc, gone);
    return !gone.isEmpty();
}

QVector<CaptionCue> captionCuesOf(const Document &doc, const PictureItem &video, const QString &preferred)
{
    QVector<CaptionCue> best;
    bool found = false;
    for (const PictureItem::Caption &c : video.mediaCaptions) {
        const auto data = doc.media.constFind(c.mediaId);
        if (data == doc.media.cend() || !looksLikeWebVtt(data->bytes)) continue;
        const bool wanted = !preferred.isEmpty() && c.language.compare(preferred, Qt::CaseInsensitive) == 0;
        if (found && !wanted) continue;
        const QVector<CaptionCue> cues = cuesOfBytes(data->bytes);
        if (cues.isEmpty()) continue;
        best = cues;
        found = true;
        if (wanted) break;
    }
    return best;
}

bool hasCaptions(const PictureItem &video) { return !video.mediaCaptions.isEmpty(); }

} // namespace jp::deck
