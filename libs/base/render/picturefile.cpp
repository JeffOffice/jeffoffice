#include "render/picturefile.h"

#include <QImageWriter>
#include <QIODevice>

#include <algorithm>
#include <climits>
#include <array>
#include <vector>

namespace jp {

QString pictureExtension(PictureFormat format)
{
    switch (format) {
    case PictureFormat::Jpeg: return QStringLiteral("jpg");
    case PictureFormat::Gif: return QStringLiteral("gif");
    case PictureFormat::Tiff: return QStringLiteral("tif");
    case PictureFormat::Bmp: return QStringLiteral("bmp");
    default: return QStringLiteral("png");
    }
}

bool pictureFormatOfExtension(const QString &extension, PictureFormat *format)
{
    const QString e = extension.toLower();
    PictureFormat f;
    if (e == QLatin1String("png")) f = PictureFormat::Png;
    else if (e == QLatin1String("jpg") || e == QLatin1String("jpeg")) f = PictureFormat::Jpeg;
    else if (e == QLatin1String("gif")) f = PictureFormat::Gif;
    else if (e == QLatin1String("tif") || e == QLatin1String("tiff")) f = PictureFormat::Tiff;
    else if (e == QLatin1String("bmp")) f = PictureFormat::Bmp;
    else return false;
    if (format) *format = f;
    return true;
}

namespace {

struct Rgb {
    int r, g, b;
};

// A palette of at most 256 colors by median cut over the colors (5 bits each) of the picture, weighted by how many pixels have each.
std::vector<Rgb> medianCut(const std::vector<quint32> &counts)
{
    struct Box {
        std::vector<int> colors;   // 15-bit colors
        quint64 weight = 0;
        int spread = 0, axis = 0;
    };
    auto channel = [](int c, int axis) { return axis == 0 ? (c >> 10) & 31 : axis == 1 ? (c >> 5) & 31 : c & 31; };
    auto measure = [&](Box &b) {
        int lo[3] = {31, 31, 31}, hi[3] = {0, 0, 0};
        b.weight = 0;
        for (int c : b.colors) {
            b.weight += counts[c];
            for (int a = 0; a < 3; ++a) {
                lo[a] = std::min(lo[a], channel(c, a));
                hi[a] = std::max(hi[a], channel(c, a));
            }
        }
        b.spread = 0;
        for (int a = 0; a < 3; ++a)
            if (hi[a] - lo[a] > b.spread) {
                b.spread = hi[a] - lo[a];
                b.axis = a;
            }
    };
    Box first;
    for (int c = 0; c < int(counts.size()); ++c)
        if (counts[c]) first.colors.push_back(c);
    measure(first);
    std::vector<Box> boxes{first};
    while (boxes.size() < 256) {
        // The box with the most pixels that can still be split.
        int pick = -1;
        for (int i = 0; i < int(boxes.size()); ++i)
            if (boxes[i].colors.size() > 1 && (pick < 0 || boxes[i].weight * (boxes[i].spread + 1) > boxes[pick].weight * (boxes[pick].spread + 1))) pick = i;
        if (pick < 0) break;
        Box &b = boxes[pick];
        const int axis = b.axis;
        std::sort(b.colors.begin(), b.colors.end(), [&](int x, int y) { return channel(x, axis) < channel(y, axis); });
        // Split where half the pixels are on each side.
        quint64 seen = 0;
        size_t cut = 1;
        for (size_t i = 0; i < b.colors.size() - 1; ++i) {
            seen += counts[b.colors[i]];
            cut = i + 1;
            if (seen * 2 >= b.weight) break;
        }
        Box other;
        other.colors.assign(b.colors.begin() + cut, b.colors.end());
        b.colors.resize(cut);
        measure(b);
        measure(other);
        boxes.push_back(std::move(other));
    }
    std::vector<Rgb> palette;
    for (const Box &b : boxes) {
        quint64 r = 0, g = 0, bl = 0, w = 0;
        for (int c : b.colors) {
            const quint64 n = counts[c];
            r += n * ((c >> 10) & 31);
            g += n * ((c >> 5) & 31);
            bl += n * (c & 31);
            w += n;
        }
        if (!w) continue;
        auto wide = [&](quint64 v) { return int((v * 255 + w * 31 / 2) / (w * 31)); };
        palette.push_back({wide(r), wide(g), wide(bl)});
    }
    return palette;
}

void put16(QByteArray &out, int v)
{
    out.append(char(v & 255));
    out.append(char((v >> 8) & 255));
}

// The LZW packing of GIF: codes of 9 to 12 bits, a clear code when the table is full.
class Lzw {
public:
    explicit Lzw(QByteArray &out) : m_out(out) {}
    void pack(const std::vector<quint8> &pixels)
    {
        constexpr int hsize = 5003, clearCode = 256, endCode = 257, hshift = 4;
        std::vector<qint32> keys(hsize, -1);
        std::vector<int> codes(hsize, 0);
        m_free = endCode + 1;
        output(clearCode);
        int ent = pixels[0];
        for (size_t k = 1; k < pixels.size(); ++k) {
            const int c = pixels[k];
            const qint32 key = (c << 12) + ent;
            int i = (c << hshift) ^ ent;
            bool found = false;
            if (keys[i] == key) {
                found = true;
            } else if (keys[i] >= 0) {
                const int disp = i == 0 ? 1 : hsize - i;
                do {
                    i -= disp;
                    if (i < 0) i += hsize;
                    if (keys[i] == key) {
                        found = true;
                        break;
                    }
                } while (keys[i] >= 0);
            }
            if (found) {
                ent = codes[i];
                continue;
            }
            output(ent);
            ent = c;
            if (m_free < 4096) {
                codes[i] = m_free++;
                keys[i] = key;
            } else {
                std::fill(keys.begin(), keys.end(), -1);
                m_free = endCode + 1;
                m_clear = true;
                output(clearCode);
            }
        }
        output(ent);
        output(endCode);
        if (m_bits > 0) addByte(m_accum & 255);
        flushBlock();
    }

private:
    void output(int code)
    {
        m_accum |= quint32(code) << m_bits;
        m_bits += m_width;
        while (m_bits >= 8) {
            addByte(m_accum & 255);
            m_accum >>= 8;
            m_bits -= 8;
        }
        if (m_free > m_max || m_clear) {
            if (m_clear) {
                m_width = 9;
                m_max = (1 << m_width) - 1;
                m_clear = false;
            } else {
                ++m_width;
                m_max = m_width == 12 ? 4096 : (1 << m_width) - 1;
            }
        }
    }
    void addByte(int b)
    {
        m_block[m_n++] = char(b);
        if (m_n == 254) flushBlock();
    }
    void flushBlock()
    {
        if (!m_n) return;
        m_out.append(char(m_n));
        m_out.append(m_block.data(), m_n);
        m_n = 0;
    }

    QByteArray &m_out;
    std::array<char, 256> m_block{};
    int m_n = 0;
    quint32 m_accum = 0;
    int m_bits = 0, m_width = 9, m_max = 511, m_free = 258;
    bool m_clear = false;
};

} // namespace

QByteArray encodeGif(const QImage &imageIn)
{
    if (imageIn.isNull() || imageIn.width() > 65535 || imageIn.height() > 65535) return {};
    // (on white where the picture lets it show through)
    QImage image(imageIn.size(), QImage::Format_RGB32);
    image.fill(Qt::white);
    {
        QImage src = imageIn.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        for (int y = 0; y < image.height(); ++y) {
            const QRgb *in = reinterpret_cast<const QRgb *>(src.constScanLine(y));
            QRgb *out = reinterpret_cast<QRgb *>(image.scanLine(y));
            for (int x = 0; x < image.width(); ++x) {
                const int a = qAlpha(in[x]);
                // premultiplied over white: color + (255 - alpha)
                out[x] = qRgb(std::min(255, qRed(in[x]) + 255 - a), std::min(255, qGreen(in[x]) + 255 - a), std::min(255, qBlue(in[x]) + 255 - a));
            }
        }
    }
    const int w = image.width(), h = image.height();
    std::vector<quint32> counts(32768, 0);
    for (int y = 0; y < h; ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < w; ++x) ++counts[((qRed(line[x]) >> 3) << 10) | ((qGreen(line[x]) >> 3) << 5) | (qBlue(line[x]) >> 3)];
    }
    std::vector<Rgb> palette = medianCut(counts);
    if (palette.empty()) return {};
    // The nearest palette color of each 15-bit color, found when first needed.
    std::vector<int> nearest(32768, -1);
    auto indexOf = [&](int c) {
        if (nearest[c] >= 0) return nearest[c];
        const int r = ((c >> 10) & 31) * 255 / 31, g = ((c >> 5) & 31) * 255 / 31, b = (c & 31) * 255 / 31;
        int best = 0, bestDistance = INT_MAX;
        for (int i = 0; i < int(palette.size()); ++i) {
            const int d = (palette[i].r - r) * (palette[i].r - r) + (palette[i].g - g) * (palette[i].g - g) + (palette[i].b - b) * (palette[i].b - b);
            if (d < bestDistance) {
                bestDistance = d;
                best = i;
            }
        }
        return nearest[c] = best;
    };
    std::vector<quint8> pixels(size_t(w) * h);
    for (int y = 0; y < h; ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < w; ++x) pixels[size_t(y) * w + x] = quint8(indexOf(((qRed(line[x]) >> 3) << 10) | ((qGreen(line[x]) >> 3) << 5) | (qBlue(line[x]) >> 3)));
    }
    QByteArray out("GIF89a");
    put16(out, w);
    put16(out, h);
    out.append(char(0xF7));   // a table of 256 colors for the whole picture, 8 bits each
    out.append(char(0));
    out.append(char(0));
    for (int i = 0; i < 256; ++i) {
        const Rgb c = i < int(palette.size()) ? palette[i] : Rgb{0, 0, 0};
        out.append(char(c.r));
        out.append(char(c.g));
        out.append(char(c.b));
    }
    out.append(char(0x2C));
    put16(out, 0);
    put16(out, 0);
    put16(out, w);
    put16(out, h);
    out.append(char(0));
    out.append(char(8));
    Lzw(out).pack(pixels);
    out.append(char(0));
    out.append(char(0x3B));
    return out;
}

bool writePicture(QIODevice *to, const QImage &image, PictureFormat format, int quality, QString *why)
{
    if (image.isNull()) {
        if (why) *why = QStringLiteral("The picture is empty.");
        return false;
    }
    if (format == PictureFormat::Gif) {
        const QByteArray gif = encodeGif(image);
        if (gif.isEmpty() || to->write(gif) != gif.size()) {
            if (why) *why = gif.isEmpty() ? QStringLiteral("The picture can't be made a GIF.") : to->errorString();
            return false;
        }
        return true;
    }
    QImageWriter writer(to, format == PictureFormat::Jpeg ? "JPEG" : format == PictureFormat::Tiff ? "TIFF" : format == PictureFormat::Bmp ? "BMP" : "PNG");
    if (format == PictureFormat::Jpeg) writer.setQuality(quality);
    if (!writer.write(image)) {
        if (why) *why = writer.errorString();
        return false;
    }
    return true;
}

} // namespace jp
