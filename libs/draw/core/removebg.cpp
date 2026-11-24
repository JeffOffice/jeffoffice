#include "core/removebg.h"

#include <QJsonArray>
#include <algorithm>
#include <array>
#include <cmath>

namespace jp {

namespace {

using Rgb = std::array<float, 3>;

float dist2(const Rgb &a, const Rgb &b)
{
    const float x = a[0] - b[0], y = a[1] - b[1], z = a[2] - b[2];
    return x * x + y * y + z * z;
}

// A group of like colors: where it is, and how far from there a color still belongs to it.
struct Group {
    Rgb center{};
    float tolerance2 = 0;
    int members = 0;
};

// Groups `samples` into at most `most` groups (spread-out starting points, then a few passes of moving each center to its members' mean); a
// group's tolerance is two and a half times how far its members lie from its center, and never less than 16 (colors of a camera or a screen
// that look the same differ by that much). Groups with fewer than `least` members are left out.
QVector<Group> fitGroups(const QVector<Rgb> &samples, int most, int least)
{
    QVector<Group> out;
    if (samples.isEmpty()) return out;
    QVector<Rgb> centers{samples[0]};
    QVector<float> nearest(samples.size());
    for (int i = 0; i < samples.size(); ++i) nearest[i] = dist2(samples[i], centers[0]);
    while (centers.size() < most) {
        int far = 0;
        for (int i = 1; i < samples.size(); ++i)
            if (nearest[i] > nearest[far]) far = i;
        if (nearest[far] < 100.f) break;
        centers << samples[far];
        for (int i = 0; i < samples.size(); ++i) nearest[i] = std::min(nearest[i], dist2(samples[i], centers.last()));
    }
    QVector<int> owner(samples.size(), 0);
    for (int pass = 0; pass < kRemoveBgFitPasses; ++pass) {
        QVector<std::array<double, 4>> sum(centers.size(), {0, 0, 0, 0});
        for (int i = 0; i < samples.size(); ++i) {
            int best = 0;
            float bd = dist2(samples[i], centers[0]);
            for (int c = 1; c < centers.size(); ++c)
                if (const float d = dist2(samples[i], centers[c]); d < bd) { bd = d; best = c; }
            owner[i] = best;
            for (int k = 0; k < 3; ++k) sum[best][k] += samples[i][k];
            sum[best][3] += 1;
        }
        for (int c = 0; c < centers.size(); ++c)
            if (sum[c][3] > 0) centers[c] = Rgb{float(sum[c][0] / sum[c][3]), float(sum[c][1] / sum[c][3]), float(sum[c][2] / sum[c][3])};
    }
    QVector<double> spread(centers.size(), 0);
    QVector<int> count(centers.size(), 0);
    for (int i = 0; i < samples.size(); ++i) {
        spread[owner[i]] += std::sqrt(dist2(samples[i], centers[owner[i]]));
        ++count[owner[i]];
    }
    for (int c = 0; c < centers.size(); ++c) {
        if (count[c] < least) continue;
        const float tol = std::max(16.f, float(2.5 * spread[c] / count[c]));
        out << Group{centers[c], tol * tol, count[c]};
    }
    return out;
}

// How far, in tolerances, the color is from the nearest of the groups (very far when there are none).
float nearness(const Rgb &c, const QVector<Group> &groups)
{
    float best = 1e30f;
    for (const Group &g : groups) best = std::min(best, dist2(c, g.center) / g.tolerance2);
    return best;
}

// Every pixel of the image shrunk to the work size, as color triples with a pixel of the picture's own alpha beside it.
struct Work {
    int w = 0, h = 0;
    QVector<Rgb> blurred, raw;
    QVector<uchar> clear;   // 1 where the picture is already clear
};

Work makeWork(const QImage &picture)
{
    Work wk;
    QImage small = picture;
    const qint64 px = qint64(picture.width()) * picture.height();
    if (px > kRemoveBgWorkPixels) {
        const double k = std::sqrt(double(kRemoveBgWorkPixels) / double(px));
        small = picture.scaled(std::max(1, int(picture.width() * k)), std::max(1, int(picture.height() * k)), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    small = small.convertToFormat(QImage::Format_ARGB32);
    wk.w = small.width();
    wk.h = small.height();
    const qsizetype n = qsizetype(wk.w) * wk.h;
    wk.raw.resize(n);
    wk.clear.resize(n);
    for (int y = 0; y < wk.h; ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(small.constScanLine(y));
        for (int x = 0; x < wk.w; ++x) {
            const QRgb c = line[x];
            const qsizetype i = qsizetype(y) * wk.w + x;
            wk.raw[i] = Rgb{float(qRed(c)), float(qGreen(c)), float(qBlue(c))};
            wk.clear[i] = qAlpha(c) < 16;
        }
    }
    // The colors the groups are fitted to and the flood goes by: each pixel's mean with its neighbors', so that grain does not look like an edge.
    wk.blurred.resize(n);
    for (int y = 0; y < wk.h; ++y)
        for (int x = 0; x < wk.w; ++x) {
            Rgb s{0, 0, 0};
            int c = 0;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx) {
                    const int xx = x + dx, yy = y + dy;
                    if (xx < 0 || yy < 0 || xx >= wk.w || yy >= wk.h) continue;
                    const Rgb &p = wk.raw[qsizetype(yy) * wk.w + xx];
                    s[0] += p[0]; s[1] += p[1]; s[2] += p[2];
                    ++c;
                }
            wk.blurred[qsizetype(y) * wk.w + x] = Rgb{s[0] / c, s[1] / c, s[2] / c};
        }
    return wk;
}

// The pixels under the strokes of one kind (keep or not), as marks on a bitmap, and the colors found there.
void paintMarks(const QVector<BackgroundMark> &marks, bool keep, const Work &wk, QVector<uchar> &bitmap, QVector<Rgb> &colors)
{
    const double side = std::max(wk.w, wk.h);
    int points = 0;
    for (const BackgroundMark &m : marks) {
        if (m.keep != keep) continue;
        const double r = std::clamp(m.radius, 0.001, 0.25) * side;
        const int ri = std::max(1, int(std::ceil(r)));
        for (const QPointF &pt : m.points) {
            if (++points > kRemoveBgMaxMarkPoints) return;
            const double cx = std::clamp(pt.x(), 0.0, 1.0) * (wk.w - 1), cy = std::clamp(pt.y(), 0.0, 1.0) * (wk.h - 1);
            for (int y = std::max(0, int(cy) - ri); y <= std::min(wk.h - 1, int(cy) + ri); ++y)
                for (int x = std::max(0, int(cx) - ri); x <= std::min(wk.w - 1, int(cx) + ri); ++x) {
                    if ((x - cx) * (x - cx) + (y - cy) * (y - cy) > r * r) continue;
                    const qsizetype i = qsizetype(y) * wk.w + x;
                    if (!bitmap[i]) {
                        bitmap[i] = 1;
                        if (colors.size() < 2048 || (i % 7) == 0) { if (colors.size() < 8192) colors << wk.blurred[i]; }
                    }
                }
        }
    }
}

bool stop(const std::function<bool(int)> &progress, int percent) { return progress && !progress(percent); }

} // namespace

bool BackgroundRemoval::isValid() const
{
    if (source.width() <= 0 || source.height() <= 0 || size.width() <= 0 || size.height() <= 0) return false;
    if (qint64(source.width()) * source.height() > kRemoveBgMaxSourcePixels) return false;
    if (qint64(size.width()) * size.height() > kRemoveBgWorkPixels + 2 * (size.width() + size.height())) return false;
    return mask.size() == qint64(size.width()) * size.height();
}

QString BackgroundRemoval::key() const
{
    return QStringLiteral("rb%1x%2:%3x%4:%5:%6:%7").arg(source.width()).arg(source.height()).arg(size.width()).arg(size.height()).arg(qHash(QByteArrayView(mask), 0)).arg(mask.size()).arg(marks.size());
}

QJsonObject BackgroundRemoval::toJson() const
{
    QJsonObject o;
    if (!isValid()) return o;
    o["source"] = QJsonArray{source.width(), source.height()};
    o["size"] = QJsonArray{size.width(), size.height()};
    o["mask"] = QString::fromLatin1(qCompress(mask, 9).toBase64());
    QJsonArray ms;
    for (const BackgroundMark &m : marks) {
        QJsonArray pts;
        for (const QPointF &p : m.points) pts << QJsonArray{std::round(p.x() * 10000) / 10000, std::round(p.y() * 10000) / 10000};
        ms << QJsonObject{{"keep", m.keep}, {"radius", m.radius}, {"points", pts}};
    }
    if (!ms.isEmpty()) o["marks"] = ms;
    return o;
}

BackgroundRemoval BackgroundRemoval::fromJson(const QJsonObject &o)
{
    BackgroundRemoval r;
    const QJsonArray s = o["source"].toArray(), z = o["size"].toArray();
    if (s.size() != 2 || z.size() != 2) return {};
    r.source = QSize(s[0].toInt(), s[1].toInt());
    r.size = QSize(z[0].toInt(), z[1].toInt());
    if (r.source.width() <= 0 || r.source.height() <= 0 || r.size.width() <= 0 || r.size.height() <= 0) return {};
    const qint64 expect = qint64(r.size.width()) * r.size.height();
    if (expect > kRemoveBgWorkPixels + 2 * (r.size.width() + r.size.height())) return {};
    // (qUncompress trusts the size its data gives: that is read first, and must be the size of the mask)
    const QByteArray packed = QByteArray::fromBase64(o["mask"].toString().toLatin1());
    if (packed.size() < 4) return {};
    const quint32 said = (quint32(uchar(packed[0])) << 24) | (quint32(uchar(packed[1])) << 16) | (quint32(uchar(packed[2])) << 8) | quint32(uchar(packed[3]));
    if (qint64(said) != expect) return {};
    r.mask = qUncompress(packed);
    if (!r.isValid()) return {};
    int points = 0;
    for (const QJsonValue &v : o["marks"].toArray()) {
        if (r.marks.size() >= kRemoveBgMaxMarks) break;
        const QJsonObject mo = v.toObject();
        BackgroundMark m;
        m.keep = mo["keep"].toBool(true);
        m.radius = std::clamp(mo["radius"].toDouble(0.02), 0.001, 0.25);
        for (const QJsonValue &pv : mo["points"].toArray()) {
            const QJsonArray p = pv.toArray();
            if (p.size() != 2 || ++points > kRemoveBgMaxMarkPoints) continue;
            m.points << QPointF(std::clamp(p[0].toDouble(), 0.0, 1.0), std::clamp(p[1].toDouble(), 0.0, 1.0));
        }
        r.marks << m;
    }
    return r;
}

BackgroundRemoval findBackground(const QImage &picture, const QVector<BackgroundMark> &marks, const std::function<bool(int)> &progress)
{
    if (picture.isNull() || picture.width() <= 0 || picture.height() <= 0 || qint64(picture.width()) * picture.height() > kRemoveBgMaxSourcePixels) return {};
    if (stop(progress, 0)) return {};
    const Work wk = makeWork(picture);
    const int w = wk.w, h = wk.h;
    const qsizetype n = qsizetype(w) * h;
    if (stop(progress, 10)) return {};

    // The background's colors: those along the edges (a ring a hundredth of the picture wide, at least two pixels), and those under the strokes
    // that take pixels away; the foreground's: those under the strokes that keep them.
    QVector<uchar> keepFixed(n, 0), dropFixed(n, 0);
    QVector<Rgb> keepColors, dropColors;
    paintMarks(marks, true, wk, keepFixed, keepColors);
    paintMarks(marks, false, wk, dropFixed, dropColors);
    QVector<Rgb> edge;
    const int ring = std::max(2, std::min(w, h) / 100);
    const qsizetype step = std::max<qsizetype>(1, qsizetype(2) * (qsizetype(w) + h) * ring / 4096);
    qsizetype seen = 0;
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            if (x >= ring && y >= ring && x < w - ring && y < h - ring) { x = w - ring - 1; continue; }
            const qsizetype i = qsizetype(y) * w + x;
            if (!wk.clear[i] && (seen++ % step) == 0) edge << wk.blurred[i];
        }
    QVector<Group> bg = fitGroups(edge, kRemoveBgMaxGroups, std::max<int>(2, edge.size() / 50));
    if (!dropColors.isEmpty()) bg += fitGroups(dropColors, 4, int(dropColors.size() / 10) + 1);
    const QVector<Group> fg = keepColors.isEmpty() ? QVector<Group>() : fitGroups(keepColors, 4, int(keepColors.size() / 10) + 1);
    if (stop(progress, 25)) return {};

    // The pixels that may be background: those like a background group, and not much more like the foreground strokes' colors.
    QVector<uchar> may(n, 0);
    for (qsizetype i = 0; i < n; ++i) {
        if (keepFixed[i]) continue;
        if (wk.clear[i] || dropFixed[i]) { may[i] = 1; continue; }
        const float db = std::sqrt(nearness(wk.blurred[i], bg));
        if (db > 1.f) continue;
        if (!fg.isEmpty() && std::sqrt(nearness(wk.blurred[i], fg)) < 0.5f * db) continue;
        may[i] = 1;
    }
    if (stop(progress, 40)) return {};

    // What a stroke that keeps pixels keeps: the pixels joined to it through pixels of its colors (a subject whose colors the edges share goes
    // with it). A stroke that takes pixels away wins where the two meet: what is joined to it through pixels that may be background is gone.
    QVector<int> queue;
    queue.reserve(int(std::min<qsizetype>(n, 1 << 20)));
    auto spread = [&](QVector<uchar> &into, const QVector<uchar> &from, const std::function<bool(qsizetype, qsizetype)> &joins) {
        queue.clear();
        for (qsizetype i = 0; i < n; ++i)
            if (from[i]) { into[i] = 1; queue << int(i); }
        for (qsizetype head = 0; head < queue.size(); ++head) {
            const int i = queue[head], x = i % w, y = i / w;
            auto go = [&](int j) {
                if (!into[j] && joins(i, j)) { into[j] = 1; queue << j; }
            };
            if (x > 0) go(i - 1);
            if (x + 1 < w) go(i + 1);
            if (y > 0) go(i - w);
            if (y + 1 < h) go(i + w);
            if ((head & 0xFFFF) == 0 && stop(progress, 40)) return false;
        }
        return true;
    };
    if (!fg.isEmpty()) {
        QVector<uchar> kept(n, 0), taken(n, 0);
        if (!spread(kept, keepFixed, [&](qsizetype, qsizetype j) { return nearness(wk.blurred[j], fg) <= 1.f; })) return {};
        if (!spread(taken, dropFixed, [&](qsizetype, qsizetype j) { return may[j] != 0; })) return {};
        for (qsizetype i = 0; i < n; ++i)
            if (kept[i] && !taken[i]) may[i] = 0;
    }
    if (stop(progress, 55)) return {};

    // The flood: from the picture's edges and the strokes that take pixels away, through the pixels that may be background, and through any
    // pixel of nearly the same color as the one it comes from (a gradient or a shadow the edges' colors do not cover), by the four neighbors.
    QVector<uchar> gone(n, 0), from(n, 0);
    for (int x = 0; x < w; ++x) { from[x] = 1; from[qsizetype(h - 1) * w + x] = 1; }
    for (int y = 0; y < h; ++y) { from[qsizetype(y) * w] = 1; from[qsizetype(y) * w + w - 1] = 1; }
    for (qsizetype i = 0; i < n; ++i) {
        if (dropFixed[i]) from[i] = 1;
        if (!may[i]) from[i] = 0;
    }
    constexpr float kStep2 = 4.f * 4.f;
    if (!spread(gone, from, [&](qsizetype i, qsizetype j) { return may[j] || (!keepFixed[j] && dist2(wk.blurred[i], wk.blurred[j]) <= kStep2); })) return {};

    // Foreground specks too small to be a subject (grain the colors did not catch) go, unless a stroke keeps them.
    {
        QVector<uchar> seenPx(n, 0);
        const qsizetype smallest = std::max<qsizetype>(16, n / 5000);
        QVector<int> comp;
        for (qsizetype s = 0; s < n; ++s) {
            if (gone[s] || seenPx[s]) continue;
            comp.clear();
            comp << int(s);
            seenPx[s] = 1;
            bool kept = false;
            for (qsizetype head = 0; head < comp.size(); ++head) {
                const int i = comp[head], x = i % w, y = i / w;
                kept = kept || keepFixed[i];
                auto add = [&](int j) {
                    if (!gone[j] && !seenPx[j]) { seenPx[j] = 1; comp << j; }
                };
                if (x > 0) add(i - 1);
                if (x + 1 < w) add(i + 1);
                if (y > 0) add(i - w);
                if (y + 1 < h) add(i + w);
            }
            if (!kept && comp.size() < smallest)
                for (int i : comp) gone[i] = 1;
        }
    }
    if (stop(progress, 75)) return {};

    // The edge: pixels within two of where the label changes are given again by their own unblurred colors, to whichever of the background's
    // groups and the foreground's they are nearer to (the foreground's: those fitted to what stayed).
    {
        QVector<Rgb> stays;
        const qsizetype every = std::max<qsizetype>(1, n / 4096);
        for (qsizetype i = 0, k = 0; i < n; ++i)
            if (!gone[i] && (k++ % every) == 0) stays << wk.blurred[i];
        const QVector<Group> fgAll = fitGroups(stays, kRemoveBgMaxGroups, 2);
        QVector<uchar> nextGone = gone;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x) {
                const qsizetype i = qsizetype(y) * w + x;
                bool near = false;
                for (int dy = -2; dy <= 2 && !near; ++dy)
                    for (int dx = -2; dx <= 2; ++dx) {
                        const int xx = x + dx, yy = y + dy;
                        if (xx < 0 || yy < 0 || xx >= w || yy >= h) continue;
                        if (gone[qsizetype(yy) * w + xx] != gone[i]) { near = true; break; }
                    }
                if (!near || keepFixed[i] || dropFixed[i] || wk.clear[i] || fgAll.isEmpty()) continue;
                const float db = nearness(wk.raw[i], bg), df = nearness(wk.raw[i], fgAll);
                nextGone[i] = db < df ? 1 : 0;
            }
        gone = nextGone;
    }
    for (qsizetype i = 0; i < n; ++i) {
        if (keepFixed[i]) gone[i] = 0;
        if (dropFixed[i]) gone[i] = 1;
    }
    if (stop(progress, 90)) return {};

    // The soft edge: each pixel's mask is the mean of its neighbors' (a pixel or so of feathering).
    BackgroundRemoval r;
    r.source = picture.size();
    r.size = QSize(w, h);
    r.mask.resize(n);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            int s = 0, c = 0;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx) {
                    const int xx = x + dx, yy = y + dy;
                    if (xx < 0 || yy < 0 || xx >= w || yy >= h) continue;
                    s += gone[qsizetype(yy) * w + xx] ? 0 : 255;
                    ++c;
                }
            r.mask[qsizetype(y) * w + x] = char(uchar(s / c));
        }
    r.marks = marks;
    if (r.marks.size() > kRemoveBgMaxMarks) r.marks.resize(kRemoveBgMaxMarks);
    stop(progress, 100);
    return r;
}

QImage applyBackgroundRemoval(const QImage &picture, const BackgroundRemoval &removal, const QColor &tint)
{
    if (picture.isNull() || !removal.isValid() || removal.source != picture.size()) return picture;
    return tintBackgroundRemoval(picture, removal, tint);
}

QImage tintBackgroundRemoval(const QImage &picture, const BackgroundRemoval &removal, const QColor &tint)
{
    if (picture.isNull() || !removal.isValid()) return picture;
    QImage out = picture.convertToFormat(QImage::Format_ARGB32);
    const int w = out.width(), h = out.height(), mw = removal.size.width(), mh = removal.size.height();
    // Each pixel's mask by the four nearest of the mask's, as a picture made larger is.
    QVector<int> x0(w), x1(w), fx(w);
    for (int x = 0; x < w; ++x) {
        const double p = std::clamp((x + 0.5) * mw / w - 0.5, 0.0, double(mw - 1));
        x0[x] = int(p);
        x1[x] = std::min(mw - 1, x0[x] + 1);
        fx[x] = int((p - x0[x]) * 256);
    }
    const uchar *m = reinterpret_cast<const uchar *>(removal.mask.constData());
    const int tr = tint.red(), tg = tint.green(), tb = tint.blue(), ta = tint.isValid() ? std::clamp(tint.alpha(), 0, 255) : 0;
    for (int y = 0; y < h; ++y) {
        const double p = std::clamp((y + 0.5) * mh / h - 0.5, 0.0, double(mh - 1));
        const int y0 = int(p), y1 = std::min(mh - 1, y0 + 1), fy = int((p - y0) * 256);
        const uchar *r0 = m + qsizetype(y0) * mw, *r1 = m + qsizetype(y1) * mw;
        QRgb *line = reinterpret_cast<QRgb *>(out.scanLine(y));
        for (int x = 0; x < w; ++x) {
            const int top = r0[x0[x]] * (256 - fx[x]) + r0[x1[x]] * fx[x], bottom = r1[x0[x]] * (256 - fx[x]) + r1[x1[x]] * fx[x];
            const int keep = (top * (256 - fy) + bottom * fy) >> 16;   // 0 to 255
            const QRgb c = line[x];
            if (tint.isValid()) {
                const int away = (255 - keep) * ta / 255;   // how much of the tint covers the pixel
                line[x] = qRgba((qRed(c) * (255 - away) + tr * away) / 255, (qGreen(c) * (255 - away) + tg * away) / 255,
                                (qBlue(c) * (255 - away) + tb * away) / 255, qAlpha(c));
            } else {
                line[x] = qRgba(qRed(c), qGreen(c), qBlue(c), qAlpha(c) * keep / 255);
            }
        }
    }
    return out;
}

} // namespace jp
