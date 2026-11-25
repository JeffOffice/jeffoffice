#include "core/picturefx.h"

#include <QCoreApplication>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace jp {
namespace {

struct Plane {
    int w = 0, h = 0;
    std::vector<uint8_t> v;
};

// What a filter is told: the flag that ends it early, and where it counts its steps (a test's).
struct Run {
    const std::atomic<bool> *stop = nullptr;
    FilterCount *count = nullptr;
};

bool stopped(const Run &run) { return run.stop && run.stop->load(std::memory_order_relaxed); }

void tally(const Run &run, qint64 n)
{
    if (run.count) run.count->steps += n;
}

int percentOf(double amount, int limit) { return int(std::lround(std::clamp(std::isfinite(amount) ? amount : 0.0, -1.0, 1.0) * limit)); }

int clamp255(int v) { return std::clamp(v, 0, 255); }

// A box blur of the radius (edges repeat the outermost pixel), in whole numbers. The window slides, so the work is the pixels and a window at the start of each row and
// column, and a radius larger than the picture is made the picture's own size: a picture one pixel wide costs what any other costs for its pixels.
void blurPlane(Plane &p, int r, const Run &run)
{
    if (r < 1 || p.w < 1 || p.h < 1) return;
    const int w = p.w, h = p.h, rx = std::min(r, w), ry = std::min(r, h), spanX = 2 * rx + 1, spanY = 2 * ry + 1;
    std::vector<uint8_t> out(p.v.size());
    for (int y = 0; y < h; ++y) {
        if (stopped(run)) return;
        const uint8_t *in = p.v.data() + size_t(y) * w;
        uint8_t *o = out.data() + size_t(y) * w;
        int sum = 0;
        for (int i = -rx; i <= rx; ++i) sum += in[std::clamp(i, 0, w - 1)];
        for (int x = 0; x < w; ++x) {
            o[x] = uint8_t((sum + spanX / 2) / spanX);
            sum += in[std::min(x + rx + 1, w - 1)] - in[std::max(x - rx, 0)];
        }
        tally(run, spanX + w);
    }
    std::vector<int> col(size_t(w), 0);
    for (int i = -ry; i <= ry; ++i) {
        if (stopped(run)) return;
        const uint8_t *row = out.data() + size_t(std::clamp(i, 0, h - 1)) * w;
        for (int x = 0; x < w; ++x) col[x] += row[x];
        tally(run, w);
    }
    for (int y = 0; y < h; ++y) {
        if (stopped(run)) return;
        uint8_t *o = p.v.data() + size_t(y) * w;
        const uint8_t *add = out.data() + size_t(std::min(y + ry + 1, h - 1)) * w, *drop = out.data() + size_t(std::max(y - ry, 0)) * w;
        for (int x = 0; x < w; ++x) {
            o[x] = uint8_t((col[x] + spanY / 2) / spanY);
            col[x] += add[x] - drop[x];
        }
        tally(run, w);
    }
}

std::vector<Plane> split(const QImage &img, const Run &run)
{
    std::vector<Plane> planes(4);
    for (Plane &p : planes) {
        p.w = img.width();
        p.h = img.height();
        p.v.resize(size_t(p.w) * p.h);
    }
    for (int y = 0; y < img.height(); ++y) {
        if (stopped(run)) break;
        const QRgb *row = reinterpret_cast<const QRgb *>(img.constScanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            const size_t i = size_t(y) * img.width() + x;
            planes[0].v[i] = uint8_t(qRed(row[x]));
            planes[1].v[i] = uint8_t(qGreen(row[x]));
            planes[2].v[i] = uint8_t(qBlue(row[x]));
            planes[3].v[i] = uint8_t(qAlpha(row[x]));
        }
        tally(run, img.width());
    }
    return planes;
}

QImage join(const std::vector<Plane> &planes, int w, int h, const Run &run)
{
    QImage img(w, h, QImage::Format_ARGB32);
    for (int y = 0; y < h; ++y) {
        if (stopped(run)) break;
        QRgb *row = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < w; ++x) {
            const size_t i = size_t(y) * w + x;
            row[x] = qRgba(planes[0].v[i], planes[1].v[i], planes[2].v[i], planes[3].v[i]);
        }
        tally(run, w);
    }
    return img;
}

// The colors worked on with their alpha multiplied in, so that a clear pixel's color does not bleed into its neighbors.
std::vector<Plane> splitPremultiplied(const QImage &img, const Run &run)
{
    std::vector<Plane> planes = split(img, run);
    for (int y = 0; y < img.height(); ++y) {
        if (stopped(run)) break;
        for (int x = 0; x < img.width(); ++x) {
            const size_t i = size_t(y) * img.width() + x;
            const int a = planes[3].v[i];
            for (int c = 0; c < 3; ++c) planes[c].v[i] = uint8_t((planes[c].v[i] * a + 127) / 255);
        }
        tally(run, img.width());
    }
    return planes;
}

QImage joinPremultiplied(std::vector<Plane> &planes, int w, int h, const Run &run)
{
    for (int y = 0; y < h; ++y) {
        if (stopped(run)) break;
        for (int x = 0; x < w; ++x) {
            const size_t i = size_t(y) * w + x;
            const int a = planes[3].v[i];
            for (int c = 0; c < 3; ++c) planes[c].v[i] = a == 0 ? 0 : uint8_t(std::min(255, (planes[c].v[i] * 255 + a / 2) / a));
        }
        tally(run, w);
    }
    return join(planes, w, h, run);
}

int longSide(const QImage &img) { return std::max(img.width(), img.height()); }

QImage sharpenSoften(const QImage &img, int pct, const Run &run)
{
    std::vector<Plane> planes = splitPremultiplied(img, run);
    const int r = std::max(1, longSide(img) / 500);
    for (int c = 0; c < 3; ++c) {
        Plane blurred = planes[c];
        blurPlane(blurred, r, run);
        for (size_t i = 0; i < planes[c].v.size(); ++i) {
            const int o = planes[c].v[i], b = blurred.v[i];
            // Soften moves the pixel toward the blur; sharpen moves it away, twice as far at the most.
            planes[c].v[i] = uint8_t(clamp255(pct < 0 ? o + (b - o) * -pct / 100 : o + (o - b) * pct * 2 / 100));
        }
    }
    return joinPremultiplied(planes, img.width(), img.height(), run);
}

// Warm (positive) or cool (negative): red and blue move opposite ways by up to a fifth.
QImage temperature(const QImage &image, int pct, const Run &run)
{
    int up[256], down[256];
    for (int v = 0; v < 256; ++v) {
        up[v] = clamp255((v * (256 + pct * 51 / 100) + 128) >> 8);
        down[v] = clamp255((v * (256 - pct * 51 / 100) + 128) >> 8);
    }
    QImage img = image;
    for (int y = 0; y < img.height(); ++y) {
        if (stopped(run)) break;
        QRgb *row = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) row[x] = qRgba(up[qRed(row[x])], qGreen(row[x]), down[qBlue(row[x])], qAlpha(row[x]));
        tally(run, img.width());
    }
    return img;
}

QImage blurred(const QImage &img, int strength, const Run &run)
{
    std::vector<Plane> planes = splitPremultiplied(img, run);
    const int r = std::max(1, int(std::int64_t(longSide(img)) * strength / 1250));
    for (int c = 0; c < 4; ++c) blurPlane(planes[c], r, run);
    return joinPremultiplied(planes, img.width(), img.height(), run);
}

Plane grayOf(const QImage &img, const Run &run)
{
    Plane g;
    g.w = img.width();
    g.h = img.height();
    g.v.resize(size_t(g.w) * g.h);
    for (int y = 0; y < g.h; ++y) {
        if (stopped(run)) break;
        const QRgb *row = reinterpret_cast<const QRgb *>(img.constScanLine(y));
        for (int x = 0; x < g.w; ++x) g.v[size_t(y) * g.w + x] = uint8_t((299 * qRed(row[x]) + 587 * qGreen(row[x]) + 114 * qBlue(row[x]) + 500) / 1000);
        tally(run, g.w);
    }
    return g;
}

// A pencil sketch: the gray picture divided by its own blur, so that flat areas come out white and edges stay dark.
QImage pencil(const QImage &img, int strength, const Run &run)
{
    const Plane gray = grayOf(img, run);
    Plane soft = gray;
    blurPlane(soft, std::max(2, int(std::int64_t(longSide(img)) * (20 + strength) / 5000)), run);
    QImage out = img;
    for (int y = 0; y < img.height(); ++y) {
        if (stopped(run)) break;
        QRgb *row = reinterpret_cast<QRgb *>(out.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            const size_t i = size_t(y) * img.width() + x;
            const int v = std::min(255, gray.v[i] * 256 / (soft.v[i] + 1));
            row[x] = qRgba(v, v, v, qAlpha(row[x]));
        }
        tally(run, img.width());
    }
    return out;
}

// Bright edges in the picture's own colors on black, with a soft glow around them.
QImage glowEdges(const QImage &img, int strength, const Run &run)
{
    const Plane gray = grayOf(img, run);
    const int w = img.width(), h = img.height(), gain = 2 + strength / 25;
    auto at = [&](int x, int y) { return int(gray.v[size_t(std::clamp(y, 0, h - 1)) * w + std::clamp(x, 0, w - 1)]); };
    std::vector<Plane> planes = split(img, run);
    std::vector<uint8_t> edge(size_t(w) * h);
    for (int y = 0; y < h; ++y) {
        if (stopped(run)) return img;
        for (int x = 0; x < w; ++x) {
            const int gx = at(x + 1, y - 1) + 2 * at(x + 1, y) + at(x + 1, y + 1) - at(x - 1, y - 1) - 2 * at(x - 1, y) - at(x - 1, y + 1);
            const int gy = at(x - 1, y + 1) + 2 * at(x, y + 1) + at(x + 1, y + 1) - at(x - 1, y - 1) - 2 * at(x, y - 1) - at(x + 1, y - 1);
            edge[size_t(y) * w + x] = uint8_t(std::min(255, (std::abs(gx) + std::abs(gy)) * gain / 4));
        }
        tally(run, w);
    }
    for (int c = 0; c < 3; ++c)
        for (size_t i = 0; i < edge.size(); ++i) planes[c].v[i] = uint8_t(std::min(255, planes[c].v[i] * edge[i] / 128));
    const int r = std::max(1, longSide(img) / 300);
    for (int c = 0; c < 3; ++c) {
        Plane glow = planes[c];
        blurPlane(glow, r, run);
        for (size_t i = 0; i < glow.v.size(); ++i) planes[c].v[i] = uint8_t(std::min(255, planes[c].v[i] + glow.v[i] / 2));
    }
    return join(planes, w, h, run);
}

// Square blocks, each the average of the pixels in it.
QImage mosaic(const QImage &img, int strength, const Run &run)
{
    const int w = img.width(), h = img.height(), block = std::max(2, int(std::int64_t(longSide(img)) * strength / 1250));
    QImage out = img;
    for (int by = 0; by < h; by += block) {
        if (stopped(run)) break;
        tally(run, std::int64_t(std::min(block, h - by)) * w);
        for (int bx = 0; bx < w; bx += block) {
            const int x1 = std::min(bx + block, w), y1 = std::min(by + block, h);
            std::int64_t sum[4] = {0, 0, 0, 0};
            for (int y = by; y < y1; ++y) {
                const QRgb *row = reinterpret_cast<const QRgb *>(img.constScanLine(y));
                for (int x = bx; x < x1; ++x) {
                    const int a = qAlpha(row[x]);
                    sum[0] += qRed(row[x]) * a;
                    sum[1] += qGreen(row[x]) * a;
                    sum[2] += qBlue(row[x]) * a;
                    sum[3] += a;
                }
            }
            const std::int64_t n = std::int64_t(x1 - bx) * (y1 - by);
            const int a = int((sum[3] + n / 2) / n);
            const QRgb fill = sum[3] == 0 ? qRgba(0, 0, 0, 0) : qRgba(int((sum[0] + sum[3] / 2) / sum[3]), int((sum[1] + sum[3] / 2) / sum[3]), int((sum[2] + sum[3] / 2) / sum[3]), a);
            for (int y = by; y < y1; ++y) {
                QRgb *row = reinterpret_cast<QRgb *>(out.scanLine(y));
                for (int x = bx; x < x1; ++x) row[x] = fill;
            }
        }
    }
    return out;
}

} // namespace

bool isBlipFilter(const BlipEffect &e) { return e.kind == BlipEffect::Sharpen || e.kind == BlipEffect::Temperature || e.kind == BlipEffect::Artistic; }

bool blipFiltersHide(const QVector<BlipEffect> &filters)
{
    for (const BlipEffect &e : filters)
        if (e.kind == BlipEffect::Artistic && (e.style == int(ArtisticStyle::Blur) || e.style == int(ArtisticStyle::Mosaic))) return true;
    return false;
}

QVector<BlipEffect> blipFilters(const QVector<BlipEffect> &effects)
{
    QVector<BlipEffect> out;
    for (const BlipEffect &e : effects)
        if (isBlipFilter(e)) out.append(e);
    return out;
}

QVector<BlipEffect> blipColorEffects(const QVector<BlipEffect> &effects)
{
    QVector<BlipEffect> out;
    for (const BlipEffect &e : effects)
        if (!isBlipFilter(e)) out.append(e);
    return out;
}

qint64 blipFilterCost(const BlipEffect &e)
{
    switch (e.kind) {
    case BlipEffect::Sharpen: return 50;
    case BlipEffect::Temperature: return 3;
    case BlipEffect::Artistic:
        switch (ArtisticStyle(e.style)) {
        case ArtisticStyle::Blur: return 45;
        case ArtisticStyle::PencilGrayscale: return 16;
        case ArtisticStyle::GlowEdges: return 36;
        case ArtisticStyle::Mosaic: return 5;
        }
        return 50;
    default: return 0;
    }
}

QString blipFilterName(const BlipEffect &e)
{
    switch (e.kind) {
    case BlipEffect::Sharpen:
        if (e.amount < 0) return QCoreApplication::translate("PictureFx", "Soften");
        return QCoreApplication::translate("PictureFx", "Sharpen");
    case BlipEffect::Temperature: return QCoreApplication::translate("PictureFx", "Color Temperature");
    case BlipEffect::Artistic:
        switch (ArtisticStyle(e.style)) {
        case ArtisticStyle::Blur: return QCoreApplication::translate("PictureFx", "Blur");
        case ArtisticStyle::PencilGrayscale: return QCoreApplication::translate("PictureFx", "Pencil Grayscale");
        case ArtisticStyle::GlowEdges: return QCoreApplication::translate("PictureFx", "Glow Edges");
        case ArtisticStyle::Mosaic: return QCoreApplication::translate("PictureFx", "Mosaic");
        }
        break;
    default: break;
    }
    return QString();
}

QSize pictureFxWorkSize(const QSize &size)
{
    if (qint64(size.width()) * size.height() <= kPictureFxWorkPixels) return size;
    int k = 2;
    while (qint64((size.width() + k - 1) / k) * ((size.height() + k - 1) / k) > kPictureFxWorkPixels) ++k;
    return QSize((size.width() + k - 1) / k, (size.height() + k - 1) / k);
}

QImage pictureFxWorkCopy(const QImage &image, const std::atomic<bool> *stop)
{
    const qint64 pixels = qint64(image.width()) * image.height();
    if (image.isNull() || pixels <= kPictureFxWorkPixels) return image;
    // The smallest whole number of pixels to a block that brings the picture inside the work size.
    int k = 2;
    while (qint64((image.width() + k - 1) / k) * ((image.height() + k - 1) / k) > kPictureFxWorkPixels) ++k;
    const QImage src = image.convertToFormat(QImage::Format_ARGB32);
    QImage out((image.width() + k - 1) / k, (image.height() + k - 1) / k, QImage::Format_ARGB32);
    for (int y = 0; y < out.height(); ++y) {
        if (stop && stop->load(std::memory_order_relaxed)) break;
        QRgb *row = reinterpret_cast<QRgb *>(out.scanLine(y));
        for (int x = 0; x < out.width(); ++x) {
            std::int64_t sum[4] = {0, 0, 0, 0}, n = 0;
            for (int yy = y * k; yy < std::min(y * k + k, src.height()); ++yy) {
                const QRgb *in = reinterpret_cast<const QRgb *>(src.constScanLine(yy));
                for (int xx = x * k; xx < std::min(x * k + k, src.width()); ++xx, ++n) {
                    const int a = qAlpha(in[xx]);
                    sum[0] += qRed(in[xx]) * a;
                    sum[1] += qGreen(in[xx]) * a;
                    sum[2] += qBlue(in[xx]) * a;
                    sum[3] += a;
                }
            }
            row[x] = sum[3] == 0 ? qRgba(0, 0, 0, 0)
                                 : qRgba(int((sum[0] + sum[3] / 2) / sum[3]), int((sum[1] + sum[3] / 2) / sum[3]), int((sum[2] + sum[3] / 2) / sum[3]), int((sum[3] + n / 2) / n));
        }
    }
    return out;
}

QImage applyBlipFilter(const QImage &image, const BlipEffect &e, const std::atomic<bool> *stop, FilterCount *count)
{
    if (image.isNull() || !isBlipFilter(e)) return image;
    const Run run{stop, count};
    if (stopped(run)) return image;
    const QImage img = image.format() == QImage::Format_ARGB32 ? image : image.convertToFormat(QImage::Format_ARGB32);
    switch (e.kind) {
    case BlipEffect::Sharpen: return percentOf(e.amount, 100) == 0 ? img : sharpenSoften(img, percentOf(e.amount, 100), run);
    case BlipEffect::Temperature: return percentOf(e.amount, 100) == 0 ? img : temperature(img, percentOf(e.amount, 100), run);
    default: break;
    }
    const int strength = std::clamp(percentOf(e.amount, 100), 1, 100);
    switch (ArtisticStyle(e.style)) {
    case ArtisticStyle::Blur: return blurred(img, strength, run);
    case ArtisticStyle::PencilGrayscale: return pencil(img, strength, run);
    case ArtisticStyle::GlowEdges: return glowEdges(img, strength, run);
    case ArtisticStyle::Mosaic: return mosaic(img, strength, run);
    }
    return img;
}

} // namespace jp
