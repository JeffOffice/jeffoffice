#include "core/blipeffects.h"

#include "core/picturefx.h"
#include "core/workthread.h"

#include <QCache>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QThread>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>

namespace jp {

namespace {

const char *kindNames[] = {"lum", "gray", "bilevel", "clrchange", "clrrepl", "duotone", "hsl", "tint", "sharpen", "temp", "artistic"};

QJsonObject colorJson(const QColor &c) { return QJsonObject{{"rgb", c.name()}, {"a", c.alphaF()}}; }

QColor colorFrom(const QJsonObject &o)
{
    QColor c(o["rgb"].toString());
    if (!c.isValid()) c = QColor(Qt::black);
    c.setAlphaF(std::clamp(o["a"].toDouble(1.0), 0.0, 1.0));
    return c;
}

uchar byteOf(float v) { return uchar(std::clamp(int(v * 255 + 0.5f), 0, 255)); }

// A color as hue (in sixths of a turn, 0 up to 6), saturation, and lightness, and back; floats, and without a branch that depends on the
// color, as a picture has millions of them and the colors of a photograph go every way.
void toHsl(float r, float g, float b, float *h, float *s, float *l)
{
    const float hi = std::max({r, g, b}), lo = std::min({r, g, b}), d = hi - lo;
    *l = (hi + lo) * 0.5f;
    if (d <= 0) {
        *h = *s = 0;
        return;
    }
    *s = std::min(1.0f, d / (1 - std::fabs(2 * *l - 1)));
    const float inverse = 1 / d, red = (g - b) * inverse, green = (b - r) * inverse + 2, blue = (r - g) * inverse + 4;
    const float k = hi == r ? red : hi == g ? green : blue;
    *h = k < 0 ? k + 6 : k;
}

// `h` in sixths of a turn, from -6 up to 12.
void fromHsl(float h, float s, float l, float *r, float *g, float *b)
{
    h = h < 0 ? h + 6 : h >= 6 ? h - 6 : h;
    const float c = (1 - std::fabs(2 * l - 1)) * s, m = l - c * 0.5f;
    *r = m + c * std::clamp(std::fabs(h - 3) - 1, 0.0f, 1.0f);
    *g = m + c * std::clamp(2 - std::fabs(h - 2), 0.0f, 1.0f);
    *b = m + c * std::clamp(2 - std::fabs(h - 4), 0.0f, 1.0f);
}

// Each pixel of `img` made what `change` gives for it.
template <class F>
void eachPixel(QImage &img, F change)
{
    for (int y = 0; y < img.height(); ++y) {
        auto *row = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) row[x] = change(row[x]);
    }
}

// An effect that leaves every color as it is (a:lum with nothing in it, as the reference presentations' masters have, or a tint of none).
bool changesNothing(const BlipEffect &e)
{
    switch (e.kind) {
    case BlipEffect::Luminance: return e.brightness == 0 && e.contrast == 0;
    case BlipEffect::Hsl: return std::fmod(e.hue, 360.0) == 0 && e.saturation == 0 && e.lightness == 0;
    case BlipEffect::Tint: return e.amount == 0;
    case BlipEffect::Sharpen: case BlipEffect::Temperature: return std::lround(std::clamp(e.amount, -1.0, 1.0) * 100) == 0;
    default: return false;
    }
}

QVector<BlipEffect> changing(const QVector<BlipEffect> &effects)
{
    QVector<BlipEffect> out;
    for (const BlipEffect &e : effects)
        if (!changesNothing(e)) out.append(e);
    return out;
}

// Moves `v` (0 to 1) toward 1 by the share `by` when it is above 0, and toward 0 when below.
float moved(float v, float by) { return by >= 0 ? v + (1 - v) * by : v * (1 + by); }

std::atomic<int> made{0};
std::atomic<int> askedLater{0};
std::atomic<int> finishedLater{0};

// The most of one kind of cost: (the first `fit` of these filters fit in kMaxBlipFilterWork, on `pixels` of the work copy)
int filtersThatFit(qint64 pixels, const QVector<BlipEffect> &filters)
{
    const qint64 work = std::min(pixels, kPictureFxWorkPixels);
    qint64 used = 0;
    int fit = 0;
    for (const BlipEffect &f : filters) {
        used += work * blipFilterCost(f);
        if (used > kMaxBlipFilterWork) break;
        ++fit;
    }
    return fit;
}

} // namespace

bool BlipEffect::operator==(const BlipEffect &o) const
{
    return kind == o.kind && brightness == o.brightness && contrast == o.contrast && threshold == o.threshold && hue == o.hue && saturation == o.saturation &&
           lightness == o.lightness && amount == o.amount && style == o.style && color == o.color && color2 == o.color2 && useAlpha == o.useAlpha;
}

QJsonObject BlipEffect::toJson() const
{
    QJsonObject o{{"k", QString::fromLatin1(kindNames[kind])}};
    switch (kind) {
    case Luminance: o["bright"] = brightness; o["contrast"] = contrast; break;
    case Grayscale: break;
    case BiLevel: o["thresh"] = threshold; break;
    case ColorChange:
        o["from"] = colorJson(color);
        o["to"] = colorJson(color2);
        if (!useAlpha) o["useA"] = false;
        break;
    case ColorReplace: o["color"] = colorJson(color); break;
    case Duotone: o["dark"] = colorJson(color); o["light"] = colorJson(color2); break;
    case Hsl: o["hue"] = hue; o["sat"] = saturation; o["lum"] = lightness; break;
    case Tint: o["hue"] = hue; o["amt"] = amount; break;
    case Sharpen: case Temperature: o["amt"] = amount; break;
    case Artistic: o["amt"] = amount; o["style"] = style; break;
    }
    return o;
}

namespace {
// A number a file gave: not a number counts as none, and it is held to -limit..limit.
double sane(double v, double limit) { return std::isfinite(v) ? std::clamp(v, -limit, limit) : 0.0; }
} // namespace

BlipEffect BlipEffect::fromJson(const QJsonObject &o)
{
    BlipEffect e;
    const QString name = o["k"].toString();
    for (int i = 0; i <= Artistic; ++i)
        if (name == QLatin1String(kindNames[i])) e.kind = Kind(i);
    switch (e.kind) {
    case Luminance: e.brightness = o["bright"].toDouble(); e.contrast = o["contrast"].toDouble(); break;
    case Grayscale: break;
    case BiLevel: e.threshold = o["thresh"].toDouble(0.5); break;
    case ColorChange:
        e.color = colorFrom(o["from"].toObject());
        e.color2 = colorFrom(o["to"].toObject());
        e.useAlpha = o["useA"].toBool(true);
        break;
    case ColorReplace: e.color = colorFrom(o["color"].toObject()); break;
    case Duotone: e.color = colorFrom(o["dark"].toObject()); e.color2 = colorFrom(o["light"].toObject()); break;
    case Hsl: e.hue = sane(o["hue"].toDouble(), 360); e.saturation = sane(o["sat"].toDouble(), 1); e.lightness = sane(o["lum"].toDouble(), 1); break;
    case Tint: e.hue = o["hue"].toDouble(); e.amount = o["amt"].toDouble(); break;
    case Sharpen: case Temperature: e.amount = sane(o["amt"].toDouble(), 1); break;
    case Artistic:
        e.amount = sane(o["amt"].toDouble(), 1);
        e.style = std::clamp(o["style"].toInt(), 0, int(ArtisticStyle::Last));
        break;
    }
    return e;
}

QJsonArray blipEffectsToJson(const QVector<BlipEffect> &effects)
{
    QJsonArray a;
    for (const BlipEffect &e : effects) a.append(e.toJson());
    return a;
}

QVector<BlipEffect> blipEffectsFromJson(const QJsonArray &a)
{
    QVector<BlipEffect> out;
    for (const QJsonValue &v : a) {
        if (out.size() >= kMaxBlipEffects) break;
        out.append(BlipEffect::fromJson(v.toObject()));
    }
    return out;
}

QString blipEffectsKey(const QVector<BlipEffect> &effects)
{
    return QString::fromUtf8(QJsonDocument(blipEffectsToJson(effects)).toJson(QJsonDocument::Compact));
}

bool blipEffectsClear(const QVector<BlipEffect> &effects)
{
    for (const BlipEffect &e : effects)
        if ((e.kind == BlipEffect::ColorChange && e.color2.alpha() < 255) || (e.kind == BlipEffect::ColorReplace && e.color.alpha() < 255)) return true;
    return false;
}

bool blipEffectsChange(const QVector<BlipEffect> &effects) { return !changing(effects).isEmpty(); }

qint64 blipEffectsWork(qint64 pixels, const QVector<BlipEffect> &effects)
{
    qint64 each = 0, filtered = 0;
    for (const BlipEffect &e : effects) {
        if (changesNothing(e)) continue;
        if (isBlipFilter(e)) filtered += blipFilterCost(e);
        else each += e.kind == BlipEffect::Hsl || e.kind == BlipEffect::Tint ? kBlipHslCost : kBlipPixelCost;
    }
    // (the filters work on a copy of at most kPictureFxWorkPixels)
    return pixels * each + std::min(pixels, kPictureFxWorkPixels) * filtered;
}

bool blipColorEffectsLeftOut(qint64 pixels, const QVector<BlipEffect> &effects)
{
    const QVector<BlipEffect> changes = changing(effects);
    const QVector<BlipEffect> filters = blipFilters(changes), colors = blipColorEffects(changes);
    return !filters.isEmpty() && !colors.isEmpty() && blipEffectsWork(std::min(pixels, kPictureFxWorkPixels), colors) > kMaxBlipWork;
}

int blipFiltersLeftOut(qint64 pixels, const QVector<BlipEffect> &effects)
{
    const QVector<BlipEffect> filters = blipFilters(changing(effects));
    return int(filters.size()) - filtersThatFit(pixels, filters);
}

int blipAdjustedMade() { return made; }

double contrastFactor(double contrast)
{
    const double c = std::clamp(contrast, -1.0, 1.0);
    // At the most, the colors are cut at the middle: a factor no one sees past.
    return c >= 0 ? 1 / std::max(1 - c, 1.0 / 1024) : 1 + c;
}

double contrastOfFactor(double factor)
{
    return factor > 1 ? 1 - 1 / factor : factor - 1;
}

QVector<uchar> luminanceTable(double brightness, double factor)
{
    QVector<uchar> table(256);
    for (int v = 0; v < 256; ++v)
        table[v] = uchar(std::clamp(int(std::lround(((v / 255.0 - 0.5) * factor + 0.5 + brightness * (1 + factor) / 2) * 255)), 0, 255));
    return table;
}

QImage applyBlipEffects(const QImage &image, const QVector<BlipEffect> &all, const std::atomic<bool> *stop)
{
    if (image.isNull() || all.isEmpty()) return image;
    const qint64 pixels = qint64(image.width()) * image.height();
    const QVector<BlipEffect> effects = changing(all);
    const QVector<BlipEffect> filters = blipFilters(effects);
    // (the filters work on a copy of at most kPictureFxWorkPixels, so a picture of any size has them; the color effects alone are held to kMaxBlipPixels)
    if (effects.isEmpty() || (pixels > kMaxBlipPixels && filters.isEmpty())) return image;
    // The filters work on a copy of at most kPictureFxWorkPixels, which is their bound: they are always done, as many of the list as fit kMaxBlipFilterWork. The color effects, which
    // work on whatever size is left, are held to kMaxBlipWork as before.
    const qint64 colorPixels = filters.isEmpty() ? pixels : std::min(pixels, kPictureFxWorkPixels);
    QVector<BlipEffect> colors = blipColorEffects(effects);
    if (blipEffectsWork(colorPixels, colors) > kMaxBlipWork) {
        if (filters.isEmpty()) return image;
        colors.clear();
    }
    // The filters come first, on a copy made smaller when it is large; the color effects follow.
    QImage img = filters.isEmpty() ? image.convertToFormat(QImage::Format_ARGB32) : pictureFxWorkCopy(image, stop);
    if (!filters.isEmpty()) {
        const auto stopped = [stop] { return stop && stop->load(); };
        const int fit = filtersThatFit(pixels, filters);
        if (!img.isNull() && !stopped()) {
            img = img.convertToFormat(QImage::Format_ARGB32);
            for (int i = 0; i < fit; ++i) img = applyBlipFilter(img, filters[i], stop);
        }
        // What hides a picture (Blur, Mosaic) is never given back undone: when any of it was not worked (no room for the copy, work stopped, a list too long for the filters), a patch of the
        // picture's color is.
        if (img.isNull() || stopped() || blipFiltersHide(filters.mid(fit))) return blipFiltersHide(filters) ? pictureFxFlatPatch(image) : image;
    }
    for (const BlipEffect &e : colors) {
        const QColor from = e.color, to = e.color2;
        switch (e.kind) {
        case BlipEffect::Luminance: {
            const QVector<uchar> table = luminanceTable(std::clamp(e.brightness, -1.0, 1.0), contrastFactor(e.contrast));
            const uchar *level = table.constData();
            eachPixel(img, [level](QRgb c) { return qRgba(level[qRed(c)], level[qGreen(c)], level[qBlue(c)], qAlpha(c)); });
            break;
        }
        case BlipEffect::Grayscale:
            eachPixel(img, [](QRgb c) {
                const int gray = int(0.299 * qRed(c) + 0.587 * qGreen(c) + 0.114 * qBlue(c) + 0.5);
                return qRgba(gray, gray, gray, qAlpha(c));
            });
            break;
        case BlipEffect::BiLevel: {
            const double threshold = std::clamp(e.threshold, 0.0, 1.0) * 256;
            // (The sum of the weights is a hair under 1 in a double: a gray of 128 is not 127.99999999999999.)
            eachPixel(img, [threshold](QRgb c) {
                const int level = 0.299 * qRed(c) + 0.587 * qGreen(c) + 0.114 * qBlue(c) + 1e-9 >= threshold ? 255 : 0;
                return qRgba(level, level, level, qAlpha(c));
            });
            break;
        }
        case BlipEffect::ColorChange: {
            // A color is the one when each of its parts is within nine of it, as a gif's or a jpeg's edges are not.
            const int fr = from.red(), fg = from.green(), fb = from.blue(), tr = to.red(), tg = to.green(), tb = to.blue();
            const double clearness = to.alphaF();
            eachPixel(img, [=](QRgb c) {
                if (std::abs(qRed(c) - fr) < 10 && std::abs(qGreen(c) - fg) < 10 && std::abs(qBlue(c) - fb) < 10) return qRgba(tr, tg, tb, int(qAlpha(c) * clearness + 0.5));
                return c;
            });
            break;
        }
        case BlipEffect::ColorReplace: {
            const int cr = from.red(), cg = from.green(), cb = from.blue();
            const double clearness = from.alphaF();
            eachPixel(img, [=](QRgb c) { return qRgba(cr, cg, cb, int(qAlpha(c) * clearness + 0.5)); });
            break;
        }
        case BlipEffect::Duotone: {
            // What each of the 256 grays comes to, in each of the colors.
            QRgb ramp[256];
            for (int v = 0; v < 256; ++v) {
                const double t = v / 255.0;
                ramp[v] = qRgb(int(from.red() + (to.red() - from.red()) * t + 0.5), int(from.green() + (to.green() - from.green()) * t + 0.5),
                               int(from.blue() + (to.blue() - from.blue()) * t + 0.5));
            }
            eachPixel(img, [&ramp](QRgb c) { return (ramp[qGray(c)] & 0x00FFFFFF) | (c & 0xFF000000); });
            break;
        }
        case BlipEffect::Hsl: {
            // The hue turn in sixths of a turn, and how far the saturation and the lightness move.
            const float turn = float(std::fmod(e.hue, 360.0) / 60), satBy = float(std::clamp(e.saturation, -1.0, 1.0)), lumBy = float(std::clamp(e.lightness, -1.0, 1.0));
            eachPixel(img, [=](QRgb c) {
                float h, s, l, r, g, b;
                toHsl(qRed(c) * (1.0f / 255), qGreen(c) * (1.0f / 255), qBlue(c) * (1.0f / 255), &h, &s, &l);
                fromHsl(h + turn, moved(s, satBy), moved(l, lumBy), &r, &g, &b);
                return qRgba(byteOf(r), byteOf(g), byteOf(b), qAlpha(c));
            });
            break;
        }
        case BlipEffect::Tint: {
            // Each color moves toward the hue by the amount (a gray one has none and takes it), and is made more or less of a color by it;
            // an amount of none leaves a color as it was.
            const float turn = float(std::fmod(e.hue, 360.0) / 60), by = float(std::clamp(e.amount, -1.0, 1.0)), hue = turn < 0 ? turn + 6 : turn;
            eachPixel(img, [=](QRgb c) {
                float h, s, l, r, g, b;
                toHsl(qRed(c) * (1.0f / 255), qGreen(c) * (1.0f / 255), qBlue(c) * (1.0f / 255), &h, &s, &l);
                if (s <= 0) {
                    h = hue;
                } else {
                    float d = hue - h;
                    if (d > 3) d -= 6;
                    else if (d < -3) d += 6;
                    h += d * std::max(by, 0.0f);
                }
                fromHsl(h, std::clamp(s + by, 0.0f, 1.0f), l, &r, &g, &b);
                return qRgba(byteOf(r), byteOf(g), byteOf(b), qAlpha(c));
            });
            break;
        }
        case BlipEffect::Sharpen: case BlipEffect::Temperature: case BlipEffect::Artistic: break;   // (done above)
        }
    }
    return img;
}

namespace {

// The kept results, and the lock that guards them. (A function's static, so that it is made before anything that uses it and goes after.)
struct Kept {
    QMutex lock;
    QCache<QString, QImage> cache{256 * 1024};   // KB
};

// (made with new and never deleted: the thread beside the window may still be using it when the program ends)
Kept &kept()
{
    static Kept *k = new Kept;
    return *k;
}

QString keptKey(const QImage &image, const QVector<BlipEffect> &effects, const QString &sourceKey)
{
    return (sourceKey.isEmpty() ? QString::number(image.cacheKey()) : sourceKey) + QLatin1Char('|') + blipEffectsKey(effects);
}

QImage keptImage(const QString &key)
{
    Kept &k = kept();
    QMutexLocker locked(&k.lock);
    if (const QImage *found = k.cache.object(key)) return *found;
    return QImage();
}

// Keeps `result` under `key` (`same`: the picture was left as it is, which costs nothing to keep and is kept so that it is not tried again).
void keep(const QString &key, const QImage &result, bool same)
{
    if (!same) ++made;
    Kept &k = kept();
    QMutexLocker locked(&k.lock);
    k.cache.insert(key, new QImage(result), same ? 1 : std::max<qsizetype>(1, result.sizeInBytes() / 1024));
}

// The pictures to be filtered beside the window (one thread at a time, the oldest first), a few at the most: what comes after is asked for again the next time the picture is drawn.
class Later {
public:
    static constexpr size_t kMost = 16;
    struct Job {
        QString key, sourceKey;
        QImage image;
        QVector<BlipEffect> effects;
    };

    Later() { kept(); }

    // Ends what is queued and what is running (the thread looks between rows), and waits for it at most `budgetMs`; 0 when it has ended, else 1.
    int stopAll(int budgetMs)
    {
        stop = true;
        QElapsedTimer clock;
        clock.start();
        for (;;) {
            {
                std::lock_guard<std::mutex> g(m);
                jobs.clear();
                queued.clear();
                queuedPixels = 0;
                if (!running) return 0;
            }
            if (clock.elapsed() >= budgetMs) return 1;
            QThread::msleep(1);
        }
    }

    // False when the queue is full.
    bool add(Job job)
    {
        std::lock_guard<std::mutex> g(m);
        if (queued.contains(job.key)) return true;
        const qint64 pixels = qint64(job.image.width()) * job.image.height();
        if (jobs.size() >= kMost || (!jobs.empty() && queuedPixels + pixels > kBlipQueueMostPixels)) return false;
        queued.insert(job.key);
        queuedPixels += pixels;
        jobs.push_back(std::move(job));
        if (!running) {
            running = true;
            thread.reset();   // (the one before has ended, or is ending: it set `running` off as its last act under the lock)
            thread = std::make_unique<WorkThread>([this] { work(); });
            if (!thread->started()) {
                running = false;
                jobs.clear();
                queued.clear();
                queuedPixels = 0;
                return false;
            }
        }
        return true;
    }

    void restart() { stop = false; }

    int count()
    {
        std::lock_guard<std::mutex> g(m);
        return int(jobs.size());
    }
    qint64 pixels()
    {
        std::lock_guard<std::mutex> g(m);
        return queuedPixels;
    }

    bool waiting(const QString &key)
    {
        std::lock_guard<std::mutex> g(m);
        return queued.contains(key);
    }

    int listen(std::function<void()> f)
    {
        listeners.push_back({++lastId, std::move(f)});
        return lastId;
    }
    void unlisten(int id)
    {
        for (size_t i = 0; i < listeners.size(); ++i)
            if (listeners[i].first == id) {
                listeners.erase(listeners.begin() + i);
                return;
            }
    }

private:
    void work()
    {
        for (;;) {
            Job job;
            {
                std::lock_guard<std::mutex> g(m);
                if (jobs.empty() || stop) {
                    running = false;
                    return;
                }
                job = std::move(jobs.front());
                jobs.pop_front();
                queuedPixels -= qint64(job.image.width()) * job.image.height();
            }
            if (blipFilterWorkerHook) blipFilterWorkerHook(stop);
            // The copy the filters work on is made here and the picture let go of: what waits holds the picture only, and a running job at most the bounded copy.
            const QImage work = qint64(job.image.width()) * job.image.height() > kPictureFxWorkPixels ? pictureFxWorkCopy(job.image, &stop) : job.image;
            job.image = QImage();
            if (!stop) {
                const QImage result = applyBlipEffects(work, job.effects, &stop);
                if (!stop) keep(job.key, result, result.cacheKey() == work.cacheKey());
            }
            ++finishedLater;
            {
                std::lock_guard<std::mutex> g(m);
                queued.remove(job.key);
            }
            if (QCoreApplication *app = QCoreApplication::instance())
                QMetaObject::invokeMethod(app, [this] {
                    const auto copy = listeners;   // (a listener may remove itself)
                    for (const auto &l : copy) l.second();
                }, Qt::QueuedConnection);
        }
    }

    std::mutex m;
    std::deque<Job> jobs;
    qint64 queuedPixels = 0;   // (those of the pictures in `jobs`)
    QSet<QString> queued;
    std::unique_ptr<WorkThread> thread;
    bool running = false;
    std::atomic<bool> stop{false};
    std::vector<std::pair<int, std::function<void()>>> listeners;   // (the application's thread only)
    int lastId = 0;
};

std::atomic<Later *> theLater{nullptr};

// (made with new and never deleted, so that no destructor of a static waits for the thread: libs/shell/app/shutdown.h)
Later &later()
{
    static Later *l = [] {
        Later *made = new Later;
        theLater = made;
        return made;
    }();
    return *l;
}

} // namespace

QString blipSourceKey(const QByteArray &fileBytes, const QSize &pixels, bool slide)
{
    return QStringLiteral("fx|%1|%2|%3x%4|%5").arg(fileBytes.size()).arg(qHash(fileBytes)).arg(pixels.width()).arg(pixels.height()).arg(slide ? QLatin1Char('s') : QLatin1Char('p'));
}

QImage blipAdjusted(const QImage &image, const QVector<BlipEffect> &effects, const std::atomic<bool> *stop, const QString &sourceKey)
{
    if (image.isNull() || changing(effects).isEmpty()) return image;
    const QString key = keptKey(image, effects, sourceKey);
    if (const QImage found = keptImage(key); !found.isNull()) return found;
    const QImage result = applyBlipEffects(image, effects, stop);
    if (stop && stop->load()) return result;
    // A picture left as it is (too big, or too much to do) is kept as that, at no cost, so that it is not tried again.
    keep(key, result, result.cacheKey() == image.cacheKey());
    return result;
}

QImage blipAdjustedLater(const QImage &image, const QVector<BlipEffect> &effects, const QString &sourceKey, bool *pending)
{
    if (pending) *pending = false;
    if (image.isNull() || changing(effects).isEmpty()) return image;
    const QString key = keptKey(image, effects, sourceKey);
    if (const QImage found = keptImage(key); !found.isNull()) return found;
    // (a picture that is left as it is has nothing kept: it is made at once, as it costs nothing)
    if (qint64(image.width()) * image.height() > kMaxBlipPixels && blipFilters(changing(effects)).isEmpty()) return image;
    if (pending) *pending = true;
    ++askedLater;
    later().add({key, sourceKey, image, effects});
    return image;
}

std::function<void(const std::atomic<bool> &)> blipFilterWorkerHook;

int blipFiltersFinished() { return finishedLater; }

void restartBlipFiltersForTest() { later().restart(); }

int blipFiltersAsked() { return askedLater; }

int blipFiltersQueued() { return later().count(); }

qint64 blipFiltersQueuedPixels() { return later().pixels(); }

int stopBlipFilters(int budgetMs)
{
    Later *l = theLater;
    return l ? l->stopAll(budgetMs) : 0;
}

int addBlipFilterListener(const std::function<void()> &listener) { return later().listen(listener); }

void removeBlipFilterListener(int id) { later().unlisten(id); }

} // namespace jp
