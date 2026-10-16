// Freeform and connector outlines (odraw_path.h).

#include "office/odraw_path.h"

#include "office/odraw_read.h"

#include <algorithm>
#include <cmath>

namespace jp::office::odraw {

namespace {

constexpr int kMaxVertices = 1000000;
constexpr double kQuarter = 0.5522847498;   // a cubic curve's control length for a quarter of an ellipse

QPointF vertexAt(const QByteArray &d, const ArrayView &a, int i)
{
    const qint64 at = a.offset(i);
    if (a.size >= 8) return QPointF(i32(d, at), i32(d, at + 4));
    if (a.size >= 4) return QPointF(qint16(u16(d, at)), qint16(u16(d, at + 2)));
    return QPointF(qint8(u8(d, at)), qint8(u8(d, at + 1)));
}

// A polyline with its corners rounded off, the curved look of a curved connector.
QPainterPath smoothed(const QVector<QPointF> &pts)
{
    QPainterPath path(pts.first());
    for (int i = 1; i + 1 < pts.size(); ++i) {
        path.lineTo((pts[i - 1] + pts[i]) / 2);
        path.quadTo(pts[i], (pts[i] + pts[i + 1]) / 2);
    }
    path.lineTo(pts.last());
    return path;
}

} // namespace

QPainterPath freeformPath(const Props &p, const QSizeF &size)
{
    const QByteArray vertexData = p.complex(0x0145);
    const ArrayView vertices = arrayOf(vertexData);
    if (vertices.count < 2 || vertices.size < 2 || vertices.size == 3 || (vertices.size > 4 && vertices.size < 8)) return QPainterPath();
    const int n = std::min(vertices.count, kMaxVertices);
    // The path is drawn in the shape's own units, from its geometry rectangle.
    const double left = p.integer(0x0140, 0), top = p.integer(0x0141, 0);
    double width = double(p.integer(0x0142, 21600)) - left, height = double(p.integer(0x0143, 21600)) - top;
    if (width == 0) width = 21600;
    if (height == 0) height = 21600;
    const double sx = size.width() / width, sy = size.height() / height;
    QVector<QPointF> v;
    v.reserve(n);
    for (int i = 0; i < n; ++i) {
        const QPointF q = vertexAt(vertexData, vertices, i);
        v << QPointF((q.x() - left) * sx, (q.y() - top) * sy);
    }
    const QByteArray segmentData = p.complex(0x0146);
    const ArrayView segments = arrayOf(segmentData);
    QPainterPath path;
    int next = 0;
    auto have = [&](int k) { return next + k <= v.size(); };
    if (segments.count == 0 || segments.size != 2) {
        // Only vertices: lines, or curves in threes, closed when the shape path says so.
        const quint32 shape = p.value(0x0144, 1);
        path.moveTo(v[0]);
        next = 1;
        if (shape == 2 || shape == 3) {
            while (have(3)) {
                path.cubicTo(v[next], v[next + 1], v[next + 2]);
                next += 3;
            }
        } else {
            for (; next < v.size(); ++next) path.lineTo(v[next]);
        }
        if (shape != 0 && shape != 2) path.closeSubpath();
    } else {
        for (int s = 0; s < segments.count; ++s) {
            const quint16 seg = u16(segmentData, segments.offset(s));
            const int type = seg >> 13, count = seg & 0x1FFF, escape = (seg >> 8) & 0x1F, parameter = seg & 0xFF;
            bool stop = false;
            switch (type) {
            case 0:   // lines
                for (int k = 0; k < count && !stop; ++k) {
                    if (!have(1)) { stop = true; break; }
                    if (path.elementCount() == 0) path.moveTo(v[next++]); else path.lineTo(v[next++]);
                }
                break;
            case 1:   // curves, three vertices each
                for (int k = 0; k < count && !stop; ++k) {
                    if (!have(3)) { stop = true; break; }
                    if (path.elementCount() == 0) path.moveTo(v[next]);
                    path.cubicTo(v[next], v[next + 1], v[next + 2]);
                    next += 3;
                }
                break;
            case 2:   // a new subpath
                if (!have(1)) stop = true; else path.moveTo(v[next++]);
                break;
            case 3:   // close it
                if (path.elementCount() > 0) path.closeSubpath();
                break;
            case 4:   // end of a subpath
                break;
            case 5:
                switch (escape) {
                case 0x0A: case 0x0B:   // no fill, no line: not part of the outline
                    break;
                case 0x0C: case 0x0D: case 0x0E: case 0x0F: case 0x10: case 0x11: case 0x12: case 0x13:
                    break;   // what kind of point the next segment's are (auto, corner, smooth, symmetric): they take none
                case 0x07: case 0x08:   // quarters of an ellipse, one vertex each
                    for (int k = 0; k < parameter && !stop; ++k) {
                        if (!have(1) || path.elementCount() == 0) { stop = true; break; }
                        const QPointF from = path.currentPosition(), to = v[next++];
                        const double dx = to.x() - from.x(), dy = to.y() - from.y();
                        if (escape == 0x07) path.cubicTo(from.x() + kQuarter * dx, from.y(), to.x(), to.y() - kQuarter * dy, to.x(), to.y());
                        else path.cubicTo(from.x(), from.y() + kQuarter * dy, to.x() - kQuarter * dx, to.y(), to.x(), to.y());
                    }
                    break;
                case 0x09:   // quadratic curves: a control point and an end
                    for (int k = 0; k < parameter && !stop; ++k) {
                        if (!have(2) || path.elementCount() == 0) { stop = true; break; }
                        path.quadTo(v[next], v[next + 1]);
                        next += 2;
                    }
                    break;
                default:    // arcs and the rest: what follows can't be placed
                    stop = true;
                    break;
                }
                break;
            default:
                stop = true;
                break;
            }
            if (stop) break;
        }
    }
    return path.elementCount() >= 2 ? path : QPainterPath();
}

QPainterPath arcPath(const Props &p, const QSizeF &size)
{
    QPainterPath path;
    if (size.width() <= 0 || size.height() <= 0) return path;
    QRectF ellipse(QPointF(0, 0), size);
    const QByteArray vertexData = p.complex(0x0145);
    const ArrayView vertices = arrayOf(vertexData);
    if (vertices.count >= 2 && (vertices.size == 8 || vertices.size == 4)) {
        const double left = p.integer(0x0140, 0), top = p.integer(0x0141, 0);
        double width = double(p.integer(0x0142, 21600)) - left, height = double(p.integer(0x0143, 21600)) - top;
        if (width == 0) width = 21600;
        if (height == 0) height = 21600;
        const QPointF a = vertexAt(vertexData, vertices, 0), b = vertexAt(vertexData, vertices, 1);
        const QRectF rect = QRectF(QPointF((a.x() - left) * size.width() / width, (a.y() - top) * size.height() / height),
                                   QPointF((b.x() - left) * size.width() / width, (b.y() - top) * size.height() / height)).normalized();
        if (rect.width() > 0 && rect.height() > 0) ellipse = rect;
    }
    const double from = p.fixed(0x0147, -90), to = p.fixed(0x0148, 0);
    double sweep = std::fmod(to - from, 360.0);
    if (sweep < 0) sweep += 360;
    path.arcMoveTo(ellipse, -from);
    path.arcTo(ellipse, -from, -sweep);
    return path;
}

QPainterPath connectorPath(int type, const Props &p, const QSizeF &size)
{
    const double w = size.width(), h = size.height();
    auto adjust = [&](int i) { return p.integer(quint16(0x0147 + i), 10800) / 21600.0; };
    QPainterPath path(QPointF(0, 0));
    switch (type) {
    case 33:
        path.lineTo(w, 0);
        path.lineTo(w, h);
        break;
    case 34: {
        const double x1 = w * adjust(0);
        path.lineTo(x1, 0);
        path.lineTo(x1, h);
        path.lineTo(w, h);
        break;
    }
    case 35: {
        const double x1 = w * adjust(0), y2 = h * adjust(1);
        path.lineTo(x1, 0);
        path.lineTo(x1, y2);
        path.lineTo(w, y2);
        path.lineTo(w, h);
        break;
    }
    case 36: {
        const double x1 = w * adjust(0), y2 = h * adjust(1), x3 = w * adjust(2);
        path.lineTo(x1, 0);
        path.lineTo(x1, y2);
        path.lineTo(x3, y2);
        path.lineTo(x3, h);
        path.lineTo(w, h);
        break;
    }
    case 37:
        path.cubicTo(w / 2, 0, w, h / 2, w, h);
        break;
    case 38: {
        const double x2 = w * adjust(0);
        path.cubicTo(x2 / 2, 0, x2, h / 4, x2, h / 2);
        path.cubicTo(x2, h * 3 / 4, (w + x2) / 2, h, w, h);
        break;
    }
    case 39: {
        const double x2 = w * adjust(0), y4 = h * adjust(1);
        const double x1 = x2 / 2, x3 = (w + x2) / 2, x4 = (x2 + x3) / 2, x5 = (x3 + w) / 2;
        const double y1 = y4 / 2, y2 = y1 / 2, y3 = (y1 + y4) / 2, y5 = (h + y4) / 2;
        path.cubicTo(x1, 0, x2, y2, x2, y1);
        path.cubicTo(x2, y3, x4, y4, x3, y4);
        path.cubicTo(x5, y4, w, y5, w, h);
        break;
    }
    case 40:
        return smoothed({QPointF(0, 0), QPointF(w * adjust(0), 0), QPointF(w * adjust(0), h * adjust(1)), QPointF(w * adjust(2), h * adjust(1)),
                         QPointF(w * adjust(2), h), QPointF(w, h)});
    default:
        return QPainterPath();
    }
    return path;
}

} // namespace jp::office::odraw
