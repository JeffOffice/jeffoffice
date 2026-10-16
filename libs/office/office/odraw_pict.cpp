// The QuickDraw pictures of a Macintosh (odraw_pict.h): their operations
// replayed with a painter. Numbers are big-endian, a point is vertical first,
// and every operation starts on an even offset.

#include "office/odraw_pict.h"

#include "render/paintwork.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <QtEndian>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <map>
#include <set>
#include <tuple>
#include <vector>

namespace jp::office {

namespace {

// What one picture may do, in units of about a microsecond of work on the computers the pictures were measured on (a rectangle over the
// whole image costs about 40, a string of text 40 and a font it has not used yet 1,500, a bitmap of a million pixels 10,000, a region of
// a thousand rectangles 8,000 to clip to): the work of
// a picture of a real presentation is some tens of thousands at most (a chart of 600 operations takes 30,000), so this is more than ten
// times that, and about a second.
constexpr double kPictureWork = 1000000;
// The most operations of a picture that are read: a chart of some 600 operations takes 12 KB, and a drawing of tens of thousands a few
// hundred KB.
constexpr int kMaxOperations = 200000;
// The longest side of the image, in pixels.
constexpr double kMaxSide = 2048;
// The most pixels of bitmaps in one picture, and in one bitmap: a photo of a few million is the most a picture of a real
// presentation holds, so these are more than ten times that.
constexpr qint64 kMaxPictureBitmapPixels = 48 * 1024 * 1024;
constexpr qint64 kMaxBitmapPixels = 16 * 1024 * 1024;
// The longest side of a picture's frame: the 16-bit numbers its coordinates are.
constexpr int kMaxFrame = 32767;
// The most rectangles of a region: a shape of a few hundred rows has as many runs, and the rows of a region come with each its own
// places, so a small region can make as many rectangles as it has places times rows.
constexpr int kMaxRegionRects = 20000;
// The fonts a picture has at once; a picture of a real presentation has a dozen.
constexpr size_t kMaxFonts = 128;
// Text larger than this, in the picture's units or once drawn, is a damaged size: no picture holds it, and some systems' font
// engines fail on glyphs that large (a damaged size of 61,756 crashed the tests on Windows, October 16, 2026).
constexpr int kMaxTextSize = 1000;
constexpr double kMaxTextPixels = 4096;

// The characters of the Macintosh's own character set from 0x80 up.
const char16_t kMacRoman[128] = {
    0x00C4, 0x00C5, 0x00C7, 0x00C9, 0x00D1, 0x00D6, 0x00DC, 0x00E1, 0x00E0, 0x00E2, 0x00E4, 0x00E3, 0x00E5, 0x00E7, 0x00E9, 0x00E8,
    0x00EA, 0x00EB, 0x00ED, 0x00EC, 0x00EE, 0x00EF, 0x00F1, 0x00F3, 0x00F2, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x00F9, 0x00FB, 0x00FC,
    0x2020, 0x00B0, 0x00A2, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x00DF, 0x00AE, 0x00A9, 0x2122, 0x00B4, 0x00A8, 0x2260, 0x00C6, 0x00D8,
    0x221E, 0x00B1, 0x2264, 0x2265, 0x00A5, 0x00B5, 0x2202, 0x2211, 0x220F, 0x03C0, 0x222B, 0x00AA, 0x00BA, 0x03A9, 0x00E6, 0x00F8,
    0x00BF, 0x00A1, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB, 0x00BB, 0x2026, 0x00A0, 0x00C0, 0x00C3, 0x00D5, 0x0152, 0x0153,
    0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA, 0x00FF, 0x0178, 0x2044, 0x20AC, 0x2039, 0x203A, 0xFB01, 0xFB02,
    0x2021, 0x00B7, 0x201A, 0x201E, 0x2030, 0x00C2, 0x00CA, 0x00C1, 0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x00D3, 0x00D4,
    0xF8FF, 0x00D2, 0x00DA, 0x00DB, 0x00D9, 0x0131, 0x02C6, 0x02DC, 0x00AF, 0x02D8, 0x02D9, 0x02DA, 0x00B8, 0x02DD, 0x02DB, 0x02C7,
};

struct Reader {
    const uchar *d;
    qsizetype n;
    qsizetype p = 0;
    bool bad = false;

    bool has(qint64 k) const { return k >= 0 && k <= n - p; }
    int u8()
    {
        if (!has(1)) { bad = true; p = n; return 0; }
        return d[p++];
    }
    int u16()
    {
        if (!has(2)) { bad = true; p = n; return 0; }
        const int v = qFromBigEndian<quint16>(d + p);
        p += 2;
        return v;
    }
    int i16() { return qint16(u16()); }
    quint32 u32()
    {
        if (!has(4)) { bad = true; p = n; return 0; }
        const quint32 v = qFromBigEndian<quint32>(d + p);
        p += 4;
        return v;
    }
    void skip(qint64 k)
    {
        if (!has(k)) { bad = true; p = n; return; }
        p += k;
    }
    // A rectangle (top, left, bottom, right) as one of Qt's.
    QRectF rect()
    {
        const int top = i16(), left = i16(), bottom = i16(), right = i16();
        return QRectF(QPointF(left, top), QPointF(right, bottom)).normalized();
    }
};

using Pattern = std::array<uchar, 8>;

// What the pen, the background, and the fill are painted with: a pattern of 8 by 8 dots, the foreground color where a dot is set and the
// background color where it is not. Here the share of the dots that are set mixes the two colors.
QColor patternColor(const Pattern &pattern, const QColor &fore, const QColor &back)
{
    int set = 0;
    for (const uchar b : pattern) set += int(qPopulationCount(b));
    if (set == 64) return fore;
    if (set == 0) return back;
    const double f = set / 64.0;
    return QColor::fromRgbF(float(fore.redF() * f + back.redF() * (1 - f)), float(fore.greenF() * f + back.greenF() * (1 - f)), float(fore.blueF() * f + back.blueF() * (1 - f)));
}

// The colors the old Color operations name (the Macintosh's eight).
QColor classicColor(quint32 code)
{
    switch (code) {
    case 30: return Qt::white;
    case 69: return Qt::yellow;
    case 137: return Qt::magenta;
    case 205: return Qt::red;
    case 273: return Qt::cyan;
    case 341: return Qt::green;
    case 409: return Qt::blue;
    default: return Qt::black;
    }
}

// The letters of the Symbol font: the Greek ones, at the places of the Latin letters they are on.
QChar symbolLetter(uchar c)
{
    static const char16_t upper[26] = {0x0391, 0x0392, 0x03A7, 0x0394, 0x0395, 0x03A6, 0x0393, 0x0397, 0x0399, 0x03D1, 0x039A, 0x039B, 0x039C,
                                       0x039D, 0x039F, 0x03A0, 0x0398, 0x03A1, 0x03A3, 0x03A4, 0x03A5, 0x03C2, 0x03A9, 0x039E, 0x03A8, 0x0396};
    static const char16_t lower[26] = {0x03B1, 0x03B2, 0x03C7, 0x03B4, 0x03B5, 0x03C6, 0x03B3, 0x03B7, 0x03B9, 0x03D5, 0x03BA, 0x03BB, 0x03BC,
                                       0x03BD, 0x03BF, 0x03C0, 0x03B8, 0x03C1, 0x03C3, 0x03C4, 0x03C5, 0x03D6, 0x03C9, 0x03BE, 0x03C8, 0x03B6};
    if (c >= 'A' && c <= 'Z') return QChar(upper[c - 'A']);
    if (c >= 'a' && c <= 'z') return QChar(lower[c - 'a']);
    return c == 0xB7 ? QChar(0x2022) : QChar(c);
}

QString fontForNumber(int id)
{
    switch (id) {
    case 2: case 20: return QStringLiteral("Times New Roman");
    case 4: case 22: return QStringLiteral("Courier New");
    case 23: return QStringLiteral("Symbol");
    default: return QStringLiteral("Arial");
    }
}

class Picture {
public:
    explicit Picture(const QByteArray &data) : r{reinterpret_cast<const uchar *>(data.constData()), data.size()} {}

    // `shared` is what is left of the work all the pictures of one file may do; this takes from it what it does.
    QImage run(qint64 *shared)
    {
        work = shared ? std::min<double>(kPictureWork, double(*shared)) : kPictureWork;
        if (work <= 0) return QImage();
        const double allowed = work;
        const QImage image = render();
        if (shared) *shared = std::max<qint64>(0, *shared - qint64(std::min(allowed, allowed - work)));
        return image;
    }

private:
    enum Kind { Frame, Paint, Erase, Invert, Fill };

    QImage render()
    {
        // The size word, the frame, then the version operation of a picture of the second kind.
        r.skip(2);
        QRectF f = r.rect();
        if (r.bad || f.width() < 1 || f.height() < 1 || f.width() > kMaxFrame || f.height() > kMaxFrame) return QImage();
        if (r.u16() != 0x0011 || r.u16() != 0x02FF) return QImage();
        // The header that follows tells, for a picture of a finer grid than points (its resolution), the rectangle its operations' numbers
        // are in; the image is that rectangle, whatever the frame is.
        const qsizetype afterVersion = r.p;
        if (r.u16() == 0x0C00) {
            const int version = r.u16();
            r.skip(2 + 4 + 4);
            const QRectF source = r.rect();
            r.skip(4);
            if (!r.bad && version == 0xFFFE && source.width() >= 1 && source.height() >= 1 && source.width() <= kMaxFrame && source.height() <= kMaxFrame) f = source;
        } else {
            r.p = afterVersion;
        }
        if (r.bad) return QImage();
        const double scale = std::min(2.0, kMaxSide / std::max(f.width(), f.height()));
        QImage image(std::max(1, int(std::lround(f.width() * scale))), std::max(1, int(std::lround(f.height() * scale))), QImage::Format_ARGB32_Premultiplied);
        if (image.isNull()) return QImage();
        image.fill(Qt::transparent);
        QPainter painter(&image);
        p = &painter;
        p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
        // The picture's units in the image's pixels: its frame's top left is the image's.
        p->scale(double(image.width()) / f.width(), double(image.height()) / f.height());
        p->translate(-f.topLeft());
        QPainterPath whole;
        whole.addRect(f);
        p->setClipPath(whole);
        for (int i = 0; i < kMaxOperations && !r.bad && work >= 0 && r.p + 2 <= r.n; ++i) {
            charge(1);
            if (work < 0 || !operation()) break;
            r.p += r.p & 1;   // each one starts on an even offset
        }
        p->end();
        p = nullptr;
        // A picture that draws nothing, or can't be followed from its start, is none.
        return drew ? image : QImage();
    }

    // Takes work from what the picture may do; false when that is spent.
    bool charge(double units)
    {
        work -= units;
        return work >= 0;
    }

    // One operation. False when the picture ends, or goes on in a way this can't follow.
    bool operation()
    {
        const int op = r.u16();
        if (op == 0x00FF) return false;
        // The shapes: rectangles, rounded rectangles, ovals, arcs, polygons, and regions.
        if (op >= 0x30 && op <= 0x8F) return shape(op);
        switch (op) {
        case 0x0000: case 0x001C: case 0x001E: return true;
        case 0x0001: {
            int rects = 0;
            const QPainterPath path = region(&rects);
            if (!charge(10 + rects * 8)) return false;
            p->setClipPath(path);
            return !r.bad;
        }
        case 0x0002: bkPattern = pattern(); return true;
        case 0x0003: font = r.u16(); return true;
        case 0x0004: face = r.u8(); return true;
        case 0x0005: case 0x0008: case 0x0015: case 0x0016: r.skip(2); return true;
        case 0x0006: r.skip(4); return true;
        case 0x000C: {
            // The origin moves by an offset: what is drawn from here on is where it was, moved the other way.
            const int dh = r.i16(), dv = r.i16();
            p->translate(-dh, -dv);
            return true;
        }
        case 0x0007: {
            const int v = r.i16(), h = r.i16();
            penSize = QSizeF(h, v);
            return true;
        }
        case 0x0009: penPattern = pattern(); return true;
        case 0x000A: fillPattern = pattern(); return true;
        case 0x000B: {
            const int v = r.i16(), h = r.i16();
            oval = QSizeF(h, v);
            return true;
        }
        case 0x000D: textSize = r.u16(); return true;
        case 0x000E: fore = classicColor(r.u32()); return true;
        case 0x000F: back = classicColor(r.u32()); return true;
        case 0x0010: r.skip(8); return true;
        case 0x0011: r.skip(2); return true;
        case 0x001A: fore = color(); return true;
        case 0x001B: back = color(); return true;
        case 0x001D: case 0x001F: r.skip(6); return true;
        case 0x0020: {
            const QPointF from = point();
            const QPointF to = point();
            return line(from, to);
        }
        case 0x0021: {
            const QPointF to = point();
            return line(pen, to);
        }
        case 0x0022: {
            const QPointF from = point();
            const int dh = qint8(r.u8()), dv = qint8(r.u8());
            return line(from, from + QPointF(dh, dv));
        }
        case 0x0023: {
            const int dh = qint8(r.u8()), dv = qint8(r.u8());
            return line(pen, pen + QPointF(dh, dv));
        }
        case 0x0024: case 0x0025: case 0x0026: case 0x0027: case 0x002E: case 0x002F: r.skip(r.u16()); return true;
        case 0x0028: {
            const QPointF at = point();
            return text(at.x(), at.y());
        }
        case 0x0029: {
            const int dh = r.u8();
            return text(textAt.x() + dh, textAt.y());
        }
        case 0x002A: {
            const int dv = r.u8();
            return text(textAt.x(), textAt.y() + dv);
        }
        case 0x002B: {
            const int dh = r.u8(), dv = r.u8();
            return text(textAt.x() + dh, textAt.y() + dv);
        }
        case 0x002C: {
            // A font's name for its number.
            const int length = r.u16();
            const qsizetype end = std::min<qsizetype>(r.n, r.p + length);
            const int id = r.u16();
            const int nameLength = r.u8();
            if (!r.bad && r.p + nameLength <= end && names.size() < 1000) names[id] = QString::fromLatin1(reinterpret_cast<const char *>(r.d + r.p), nameLength);
            r.p = end;
            return !r.bad;
        }
        case 0x002D: r.skip(10); return true;
        case 0x0098: case 0x009A: return bitmap(op);
        case 0x00A0: r.skip(2); return true;
        case 0x00A1:
            r.skip(2);
            r.skip(r.u16());
            return true;
        default: break;
        }
        // Reserved operations carry as many words as their high byte says. The others (patterns made of pictures, one-bit bitmaps, bitmaps
        // with regions, pictures inside pictures, and the pictures of the first kind) can't be followed, and the picture ends there.
        if (op >= 0x0100 && op < 0x8000) {
            r.skip(qint64(op >> 8) * 2);
            return true;
        }
        return op >= 0x8000 && op < 0x8100;
    }

    QPointF point()
    {
        const int v = r.i16(), h = r.i16();
        return QPointF(h, v);
    }

    QColor color()
    {
        const int red = r.u16(), green = r.u16(), blue = r.u16();
        return QColor(red >> 8, green >> 8, blue >> 8);
    }

    Pattern pattern()
    {
        Pattern bits{};
        for (uchar &b : bits) b = uchar(r.u8());
        return bits;
    }

    // A region: its size (its own two bytes and the box's eight included), its box, and, when it is not just the box, its rows as the
    // places where each differs from the row above, each row ended by 0x7FFF as is the region. `rects` gets how many rectangles it is;
    // the rows that would make more than kMaxRegionRects are left out.
    QPainterPath region(int *rects)
    {
        QPainterPath path;
        *rects = 0;
        const int size = r.u16();
        const QRectF box = r.rect();
        if (size <= 10) {
            path.addRect(box);
            *rects = 1;
            return path;
        }
        const qsizetype end = std::min<qsizetype>(r.n, r.p + (size - 10));
        std::set<int> active;
        int above = 0;
        bool any = false, full = false;
        while (r.p + 2 <= end && !full) {
            const int y = qint16(r.u16());
            if (y == 0x7FFF) break;
            std::vector<int> places;
            while (r.p + 2 <= end) {
                const int x = qint16(r.u16());
                if (x == 0x7FFF) break;
                places.push_back(x);
            }
            // A row that differs from the one above in nothing is no new row.
            if (places.empty()) continue;
            // The runs between the places of the rows above, down to this row.
            if (any) {
                int from = 0;
                bool open = false;
                for (const int x : active) {
                    if (open) {
                        if (*rects >= kMaxRegionRects) {
                            full = true;
                            break;
                        }
                        path.addRect(QRectF(QPointF(from, above), QPointF(x, y)));
                        ++*rects;
                    }
                    from = x;
                    open = !open;
                }
            }
            for (const int x : places)
                if (!active.erase(x)) active.insert(x);
            above = y;
            any = true;
        }
        r.p = end;
        path.setFillRule(Qt::WindingFill);
        return path;
    }

    // A polygon's points: a closed one when it ends where it began, else its lines.
    QPainterPath polygon()
    {
        QPainterPath path;
        const int size = r.u16();
        r.rect();
        int count = std::max(0, (size - 10) / 4);
        if (!r.has(qint64(count) * 4)) count = int((r.n - r.p) / 4);
        QPointF first, last;
        for (int i = 0; i < count; ++i) {
            last = point();
            if (i == 0) {
                path.moveTo(last);
                first = last;
            } else {
                path.lineTo(last);
            }
        }
        if (count >= 3 && first == last) path.closeSubpath();
        return path;
    }

    // The path of a rectangle, rounded rectangle, oval, or arc (its angles in degrees clockwise from north, Qt's counterclockwise from east).
    QPainterPath pathOf(int family, const QRectF &box, bool outline) const
    {
        QPainterPath path;
        switch (family) {
        case 6: path.addRect(box); break;
        case 8: path.addRoundedRect(box, std::min(oval.width(), box.width()) / 2, std::min(oval.height(), box.height()) / 2); break;
        case 10: path.addEllipse(box); break;
        default:
            if (outline) path.arcMoveTo(box, 90 - angles.x());
            else path.moveTo(box.center());
            path.arcTo(box, 90 - angles.x(), -angles.y());
            if (!outline) path.closeSubpath();
            break;
        }
        return path;
    }

    // An arc's start and how far it goes, in degrees (read in order: the order a function's arguments are worked out in is not set).
    void readAngles()
    {
        const int start = r.i16();
        const int sweep = r.i16();
        angles = QPointF(start, sweep);
    }

    // A shape's operation: which shape (its own place, or the last one's) and what is done with it.
    bool shape(int op)
    {
        const int family = (op >> 3) & ~1, how = op & 7;
        const bool same = (op >> 3) & 1;
        int rects = 0;
        if (same) {
            if (family == 12) readAngles();
        } else if (family == 14) {
            lastPolygon = polygon();
        } else if (family == 16) {
            lastRegion = region(&rects);
            if (!charge(rects * 3)) return false;
        } else {
            lastRect = r.rect();
            if (family == 12) readAngles();
        }
        if (r.bad) return false;
        if (how > Fill || how == Invert) return true;
        if (family == 14 || family == 16) {
            const QPainterPath &path = family == 14 ? lastPolygon : lastRegion;
            return draw(path, Kind(how), path.boundingRect(), family == 16);
        }
        // The outline is drawn inside the shape: the shape inset by half the pen's size.
        QRectF box = lastRect;
        if (how == Frame) box = lastRect.adjusted(penSize.width() / 2, penSize.height() / 2, -penSize.width() / 2, -penSize.height() / 2);
        if (box.width() < 0 || box.height() < 0) return true;
        return draw(pathOf(family, box, how == Frame), Kind(how), lastRect, false);
    }

    // Paints a path, or draws it as an outline as wide as the pen. A region is painted without smoothing, which leaves no seam between the
    // rectangles it is made of.
    bool draw(const QPainterPath &path, Kind kind, const QRectF &box, bool crisp)
    {
        if (kind == Frame && (penSize.width() <= 0 || penSize.height() <= 0)) return true;
        if (!spend(box, fillWork(p, path) * (kind == Frame ? 3 : 1))) return false;
        if (kind == Frame) {
            p->setPen(QPen(patternColor(penPattern, fore, back), penWidth(), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
            p->setBrush(Qt::NoBrush);
            p->drawPath(path);
            return true;
        }
        const Pattern &used = kind == Erase ? bkPattern : kind == Fill ? fillPattern : penPattern;
        p->setPen(Qt::NoPen);
        if (crisp) p->setRenderHint(QPainter::Antialiasing, false);
        p->fillPath(path, patternColor(used, fore, back));
        if (crisp) p->setRenderHint(QPainter::Antialiasing, true);
        return true;
    }

    // The pen's width, at least half a pixel of the image.
    double penWidth() const
    {
        const double scale = std::max(std::abs(p->transform().m11()), 1e-6);
        return std::max((penSize.width() + penSize.height()) / 2, 0.5 / scale);
    }

    // Charges the work of a shape: what painting its box costs, and `more`.
    bool spend(const QRectF &box, double more = 0)
    {
        if (!charge(areaWork(p, box) + more)) return false;
        drew = true;
        return true;
    }

    // A line: the pen's rectangle drawn along it, so one the pen's size from the point (the pen hangs from its top left).
    bool line(QPointF from, QPointF to)
    {
        pen = to;
        if (penSize.width() <= 0 || penSize.height() <= 0) return true;
        const QRectF box = QRectF(from, to).normalized().adjusted(-penSize.width(), -penSize.height(), penSize.width(), penSize.height());
        if (!spend(box)) return false;
        const QPointF half(penSize.width() / 2, penSize.height() / 2);
        p->setPen(QPen(patternColor(penPattern, fore, back), penWidth(), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
        p->drawLine(from + half, to + half);
        return true;
    }

    struct Face {
        QFont font;
        QFontMetricsF metrics;
    };

    // A font of a family, size (in the picture's units), and style: made once, as making one is most of what drawing a string takes.
    const Face &faceOf(const QString &family, int size, int style, bool *made)
    {
        const auto key = std::make_tuple(family, size, style);
        auto it = faces.find(key);
        *made = it == faces.end();
        if (!*made) return it->second;
        if (faces.size() >= kMaxFonts) faces.clear();
        QFont f(family);
        f.setPixelSize(size);
        f.setBold(style & 1);
        f.setItalic(style & 2);
        f.setUnderline(style & 4);
        return faces.emplace(key, Face{f, QFontMetricsF(f)}).first->second;
    }

    // A string with its baseline at a place, the pen going on to its end.
    bool text(double h, double v)
    {
        const int count = r.u8();
        if (!r.has(count)) return false;
        const QByteArray bytes(reinterpret_cast<const char *>(r.d + r.p), count);
        r.p += count;
        const auto named = names.find(font);
        QString family = named != names.end() ? named->second : fontForNumber(font);
        // (The words of the Symbol font are drawn as the Greek letters they are, in a font that has them: Symbol itself is not on
        // every computer, and where it is, it is laid out by its own codes.)
        const bool symbol = family.compare(QLatin1String("Symbol"), Qt::CaseInsensitive) == 0;
        if (symbol) family = QStringLiteral("Arial");
        QString s;
        for (const char c : bytes) {
            const uchar u = uchar(c);
            if (symbol) s += symbolLetter(u);
            else s += u >= 0x80 ? QChar(kMacRoman[u - 0x80]) : QChar(u);
        }
        // The offsets of the operations after the first are from where the last string began (the pen is at its end).
        textAt = QPointF(h, v);
        pen = textAt;
        if (textSize <= 0 || textSize > kMaxTextSize || textSize * std::abs(p->worldTransform().m11()) > kMaxTextPixels) return true;
        bool made = false;
        const Face &used = faceOf(family, textSize, face & 7, &made);
        const double width = used.metrics.horizontalAdvance(s);
        if (!charge(40 + count * 2 + (made ? 1500 : 0))) return false;
        if (!spend(QRectF(h, v - textSize, width, textSize * 1.3))) return false;
        p->setFont(used.font);
        p->setPen(fore);
        p->drawText(QPointF(h, v), s);
        pen.setX(h + width);
        return true;
    }

    // A row of a bitmap, unpacked into `bytes`: its bytes as they are, or as the runs of PackBits (a count, then that many more bytes or
    // one repeated), which for 16-bit pixels are runs of two-byte units. Packed rows start with their length, in a byte or two.
    bool row(QByteArray *into, int bytes, bool longLength, int unit, bool packed)
    {
        into->fill('\0', bytes);
        if (!packed) {
            if (!r.has(bytes)) return false;
            std::memcpy(into->data(), r.d + r.p, size_t(bytes));
            r.p += bytes;
            return true;
        }
        const int length = longLength ? r.u16() : r.u8();
        if (r.bad || !r.has(length)) return false;
        const qsizetype end = r.p + length;
        int at = 0;
        while (r.p < end && at < bytes) {
            const int n = qint8(r.u8());
            if (n == -128) continue;
            if (n >= 0) {
                const qint64 count = qint64(n + 1) * unit;
                if (r.p + count > end) break;
                const qint64 take = std::min<qint64>(count, bytes - at);
                std::memcpy(into->data() + at, r.d + r.p, size_t(take));
                r.p += count;
                at += int(take);
            } else {
                if (r.p + unit > end) break;
                for (int i = 0; i < 1 - n && at + unit <= bytes; ++i, at += unit) std::memcpy(into->data() + at, r.d + r.p, size_t(unit));
                r.p += unit;
            }
        }
        r.p = end;
        return true;
    }

    // A bitmap with colors of its own (indexed, or direct at 16 or 32 bits to the pixel) drawn into its destination.
    bool bitmap(int op)
    {
        const bool direct = op == 0x009A;
        if (direct) r.skip(4);   // where its pixels are in memory
        const int rowBytesWord = r.u16();
        const int rowBytes = rowBytesWord & 0x3FFF;
        const QRect bounds = r.rect().toRect();
        if (!(rowBytesWord & 0x8000)) return false;   // a one-bit bitmap
        r.skip(2);
        const int packType = r.u16();
        r.skip(4 + 4 + 4 + 2);
        const int pixelSize = r.u16(), components = r.u16();
        r.skip(2 + 4 + 4 + 4);
        QVector<QRgb> table;
        if (!direct) {
            r.skip(4);
            const int flags = r.u16();
            const int count = r.u16() + 1;
            if (count > 256) return false;
            table.fill(qRgb(0, 0, 0), 256);
            for (int i = 0; i < count; ++i) {
                const int value = r.u16();
                const int red = r.u16() >> 8, green = r.u16() >> 8, blue = r.u16() >> 8;
                table[(flags & 0x8000) || value >= 256 ? i : value] = qRgb(red, green, blue);
            }
        }
        const QRectF source = r.rect(), destination = r.rect();
        r.skip(2);
        const int width = bounds.width(), height = bounds.height();
        if (r.bad || width < 1 || height < 1) return false;
        // The pixels are paid for, and counted against what the picture may hold, before there is room for them.
        const qint64 pixels = qint64(width) * height;
        if (pixels > kMaxBitmapPixels || bitmapPixels + pixels > kMaxPictureBitmapPixels) return false;
        bitmapPixels += pixels;
        if (!charge(10 + pixels / 100)) return false;
        const bool indexed = !direct && (pixelSize == 1 || pixelSize == 2 || pixelSize == 4 || pixelSize == 8);
        const bool wide = direct && pixelSize == 32, narrow = direct && pixelSize == 16;
        if (!indexed && !wide && !narrow) return false;
        if (qint64(rowBytes) * 8 < qint64(width) * pixelSize) return false;
        // A row shorter than eight bytes is not packed; 32-bit pixels packed that way are the rows of each color (without the pad that
        // makes them four) one after another.
        const bool packed = rowBytes >= 8 && packType != 1;
        const bool planes = wide && packed && packType == 4;
        const int planeCount = components == 3 ? 3 : 4;
        QImage image(width, height, QImage::Format_RGB32);
        if (image.isNull()) return false;
        QByteArray bytes;
        for (int y = 0; y < height; ++y) {
            if (!row(&bytes, planes ? width * planeCount : rowBytes, rowBytes > 250, narrow ? 2 : 1, packed)) return false;
            QRgb *out = reinterpret_cast<QRgb *>(image.scanLine(y));
            const uchar *in = reinterpret_cast<const uchar *>(bytes.constData());
            for (int x = 0; x < width; ++x) {
                if (indexed) {
                    const int perByte = 8 / pixelSize;
                    out[x] = table[(in[x / perByte] >> (8 - pixelSize * (x % perByte + 1))) & ((1 << pixelSize) - 1)];
                } else if (narrow) {
                    const int v = (in[x * 2] << 8) | in[x * 2 + 1];
                    out[x] = qRgb(((v >> 10) & 31) * 255 / 31, ((v >> 5) & 31) * 255 / 31, (v & 31) * 255 / 31);
                } else if (planes) {
                    const int skipped = planeCount - 3;
                    out[x] = qRgb(in[(skipped + 0) * width + x], in[(skipped + 1) * width + x], in[(skipped + 2) * width + x]);
                } else {
                    out[x] = qRgb(in[x * 4 + 1], in[x * 4 + 2], in[x * 4 + 3]);
                }
            }
        }
        // (Drawn at a size, a bitmap costs what that many pixels do, about as much as a rectangle of the same size and twice.)
        if (!spend(destination, destination.width() * destination.height() / 1000)) return false;
        p->drawImage(destination, image, source.translated(-bounds.topLeft()));
        return true;
    }

    Reader r;
    QPainter *p = nullptr;
    double work = 0;
    qint64 bitmapPixels = 0;
    bool drew = false;
    QColor fore = Qt::black, back = Qt::white;
    Pattern penPattern = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, fillPattern = penPattern, bkPattern{};
    QSizeF penSize{1, 1}, oval{0, 0};
    QPointF pen, textAt;
    int font = 0, face = 0, textSize = 12;
    std::map<int, QString> names;
    std::map<std::tuple<QString, int, int>, Face> faces;
    QRectF lastRect;
    QPointF angles;
    QPainterPath lastPolygon, lastRegion;
};

} // namespace

QImage odrawPictImage(const QByteArray &pict, qint64 *work)
{
    return Picture(pict).run(work);
}

} // namespace jp::office
