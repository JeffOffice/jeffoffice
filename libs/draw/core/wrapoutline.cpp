#include "core/wrapoutline.h"

#include <QPainter>

#include <algorithm>
#include <atomic>
#include <cmath>

namespace jp {

QPolygonF boundedWrapOutline(const QVector<QPointF> &points)
{
    QPolygonF out;
    for (const QPointF &p : points) {
        if (out.size() >= kMostWrapPoints) break;
        if (!std::isfinite(p.x()) || !std::isfinite(p.y())) continue;
        out << QPointF(std::clamp(p.x(), -kWrapOutlineReach, 1.0 + kWrapOutlineReach), std::clamp(p.y(), -kWrapOutlineReach, 1.0 + kWrapOutlineReach));
    }
    return out.size() < 3 ? QPolygonF() : out;
}

namespace {

double distanceToChord(const QPointF &p, const QPointF &a, const QPointF &b)
{
    const QPointF ab = b - a;
    const double len2 = ab.x() * ab.x() + ab.y() * ab.y();
    if (len2 <= 0) return std::hypot(p.x() - a.x(), p.y() - a.y());
    return std::abs((p.x() - a.x()) * ab.y() - (p.y() - a.y()) * ab.x()) / std::sqrt(len2);
}

// Douglas-Peucker on the closed ring: the points of `ring` kept so that every other one is within `tolerance` of the outline.
QVector<QPointF> simplifiedRing(const QVector<QPointF> &ring, double tolerance)
{
    const int n = int(ring.size());
    int far = 0;
    double best = -1;
    for (int i = 1; i < n; ++i) {
        const double d = std::hypot(ring[i].x() - ring[0].x(), ring[i].y() - ring[0].y());
        if (d > best) { best = d; far = i; }
    }
    QVector<bool> keep(n, false);
    keep[0] = keep[far] = true;
    QVector<std::pair<int, int>> todo;
    todo << std::make_pair(0, far) << std::make_pair(far, n);   // (n is point 0 again)
    while (!todo.isEmpty()) {
        const auto [a, b] = todo.takeLast();
        int at = -1;
        double worst = tolerance;
        for (int i = a + 1; i < b; ++i) {
            const double d = distanceToChord(ring[i], ring[a], ring[b % n]);
            if (d > worst) { worst = d; at = i; }
        }
        if (at < 0) continue;
        keep[at] = true;
        todo << std::make_pair(a, at) << std::make_pair(at, b);
    }
    QVector<QPointF> out;
    for (int i = 0; i < n; ++i)
        if (keep[i]) out << ring[i];
    return out;
}

// The ring with each edge moved outward by `d` (a corner is mitered, no longer than five times d).
QVector<QPointF> offsetRing(const QVector<QPointF> &ring, double d)
{
    const int n = int(ring.size());
    double area = 0;
    for (int i = 0; i < n; ++i) area += ring[i].x() * ring[(i + 1) % n].y() - ring[(i + 1) % n].x() * ring[i].y();
    const double sign = area >= 0 ? 1.0 : -1.0;
    auto normal = [&](const QPointF &a, const QPointF &b) {
        const double len = std::hypot(b.x() - a.x(), b.y() - a.y());
        return len > 0 ? QPointF(sign * (b.y() - a.y()) / len, -sign * (b.x() - a.x()) / len) : QPointF();
    };
    QVector<QPointF> out;
    for (int i = 0; i < n; ++i) {
        const QPointF n1 = normal(ring[(i + n - 1) % n], ring[i]), n2 = normal(ring[i], ring[(i + 1) % n]);
        const double denom = std::max(0.2, 1.0 + n1.x() * n2.x() + n1.y() * n2.y());
        out << QPointF(ring[i].x() + (n1.x() + n2.x()) / denom * d, ring[i].y() + (n1.y() + n2.y()) / denom * d);
    }
    return out;
}

} // namespace

QPolygonF wrapOutlineOfPath(const QPainterPath &path, const QSizeF &frame)
{
    if (path.isEmpty() || path.elementCount() > 20000 || !(frame.width() > 0) || !(frame.height() > 0)) return QPolygonF();
    QPainterPath solid = path;
    solid.setFillRule(Qt::WindingFill);
    // (The path clipper is not run on a long path from a file: its time is that of the crossings, not of the elements. A short one is joined as it was.)
    if (solid.elementCount() <= 256) solid = solid.simplified();
    const QList<QPolygonF> parts = solid.toSubpathPolygons();
    if (parts.size() > 1) {
        // (Several parts, or a hole: drawn small, and read as a picture is.)
        const double k = double(kAlphaOutlineSide) / std::max(frame.width(), frame.height());
        QImage drawn(std::max(1, int(std::lround(frame.width() * k))), std::max(1, int(std::lround(frame.height() * k))), QImage::Format_ARGB32_Premultiplied);
        drawn.fill(Qt::transparent);
        {
            QPainter painter(&drawn);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.scale(k, k);
            painter.fillPath(solid, Qt::black);
        }
        return wrapOutlineOfAlpha(drawn);
    }
    if (parts.isEmpty()) return QPolygonF();
    QVector<QPointF> ring;
    for (const QPointF &p : parts[0])
        if (std::isfinite(p.x()) && std::isfinite(p.y()) && (ring.isEmpty() || p != ring.last())) ring << p;
    while (ring.size() > 1 && ring.first() == ring.last()) ring.removeLast();
    if (ring.size() < 3) return QPolygonF();
    // (The tolerance starts at a half of a percent of the frame and grows until the outline is short enough.)
    double tolerance = std::max(frame.width(), frame.height()) * 0.005;
    QVector<QPointF> kept = simplifiedRing(ring, tolerance);
    while (kept.size() > kMostPathOutlinePoints) {
        tolerance *= 1.5;
        kept = simplifiedRing(ring, tolerance);
    }
    kept = offsetRing(kept, tolerance);
    for (QPointF &p : kept) p = QPointF(p.x() / frame.width(), p.y() / frame.height());
    return boundedWrapOutline(kept);
}

QPolygonF wrapOutlineOfAlpha(const QImage &image, int *scanned)
{
    if (scanned) *scanned = 0;
    if (image.isNull() || !image.hasAlphaChannel()) return QPolygonF();
    const int w = image.width(), h = image.height();
    const int step = std::max(1, (std::max(w, h) + kAlphaOutlineSide - 1) / kAlphaOutlineSide);
    const int sw = (w + step - 1) / step, sh = (h + step - 1) / step;
    // (A picture of more pixels than the most is read at every stride-th one.)
    const double all = double(w) * h;
    const int stride = all > double(kMostAlphaScanPixels) ? int(std::ceil(std::sqrt(all / double(kMostAlphaScanPixels)))) : 1;
    const int bands = std::min(kAlphaOutlineBands, sh);
    QVector<int> left(bands, -1), right(bands, -1);
    qint64 looked = 0;
    for (int y = 0; y < h; y += stride) {
        const int band = std::min(bands - 1, int(qint64(y / step) * bands / sh));
        for (int x = 0; x < w; x += stride) {
            ++looked;
            if (qAlpha(image.pixel(x, y)) <= 16) continue;
            const int sx = x / step;
            if (left[band] < 0 || sx < left[band]) left[band] = sx;
            if (sx > right[band]) right[band] = sx;
        }
    }
    if (scanned) *scanned = int(std::min<qint64>(looked, 2000000000));
    QVector<int> used;
    bool solid = true;
    for (int b = 0; b < bands; ++b) {
        if (left[b] < 0) { solid = false; continue; }
        used << b;
        solid = solid && left[b] == 0 && right[b] == sw - 1;
    }
    // (Nothing in it, or every band across: the picture keeps its frame.)
    if (used.size() < 2 || (solid && used.size() == bands)) return QPolygonF();
    auto yOf = [&](int b) { return b == used.first() ? double(b) / bands : b == used.last() ? double(b + 1) / bands : (b + 0.5) / bands; };
    QVector<QPointF> points;
    for (int b : used) points << QPointF(double(left[b]) / sw, yOf(b));
    for (int i = int(used.size()) - 1; i >= 0; --i) points << QPointF(double(right[used[i]] + 1) / sw, yOf(used[i]));
    return boundedWrapOutline(points);
}

double boundedWrapDistance(double distance)
{
    return std::isfinite(distance) ? std::clamp(distance, 0.0, kMostWrapDistance) : (distance > 0 ? kMostWrapDistance : 0.0);
}

QRectF wrapSideWall(const QRectF &room, const QRectF &frame, bool textLeft, bool largest)
{
    const bool onLeft = textLeft || (largest && room.left() - frame.left() >= frame.right() - room.right());
    QRectF wall;
    if (onLeft) wall.setCoords(room.left(), room.top(), 1e5, room.bottom());
    else wall.setCoords(-1e5, room.top(), room.right(), room.bottom());
    return wall;
}

namespace {

std::atomic<qint64> g_roomsMade{0}, g_roomWork{0};
constexpr double kCoordinateLimit = 1e7;

bool usable(const QPointF &p) { return std::isfinite(p.x()) && std::isfinite(p.y()); }
QPointF held(const QPointF &p) { return QPointF(std::clamp(p.x(), -kCoordinateLimit, kCoordinateLimit), std::clamp(p.y(), -kCoordinateLimit, kCoordinateLimit)); }

// A ring without the points that are not numbers, the point that repeats the one before it, and a last point that is the first.
QPolygonF cleanRing(const QPolygonF &in, bool closed)
{
    QPolygonF out;
    for (const QPointF &raw : in) {
        if (!usable(raw)) continue;
        const QPointF p = held(raw);
        if (out.isEmpty() || std::abs(p.x() - out.last().x()) > 1e-9 || std::abs(p.y() - out.last().y()) > 1e-9) out << p;
    }
    if (closed && out.size() > 1 && std::abs(out.first().x() - out.last().x()) <= 1e-9 && std::abs(out.first().y() - out.last().y()) <= 1e-9) out.removeLast();
    return out;
}

// The extent of a segment's part within the band y0..y1 (both ends in), widening [l, r].
inline void widen(double ax, double ay, double bx, double by, double y0, double y1, double &l, double &r)
{
    const double lo = std::min(ay, by), hi = std::max(ay, by);
    if (hi < y0 || lo > y1) return;
    if (ay == by) {
        l = std::min({l, ax, bx});
        r = std::max({r, ax, bx});
        return;
    }
    const double k = (bx - ax) / (by - ay);
    const double ya = std::max(lo, y0), yb = std::min(hi, y1);
    const double xa = ax + (ya - ay) * k, xb = ax + (yb - ay) * k;
    l = std::min({l, xa, xb});
    r = std::max({r, xa, xb});
}

} // namespace

std::shared_ptr<const WrapRoom> WrapRoom::ofRect(const QRectF &rect)
{
    std::shared_ptr<WrapRoom> room(new WrapRoom);
    if (rect.isValid() && std::isfinite(rect.left()) && std::isfinite(rect.top()) && std::isfinite(rect.right()) && std::isfinite(rect.bottom()) && rect.width() > 0 && rect.height() > 0) {
        const QRectF r = rect.intersected(QRectF(-kCoordinateLimit, -kCoordinateLimit, 2 * kCoordinateLimit, 2 * kCoordinateLimit));
        room->m_points << r.topLeft() << r.topRight() << r.bottomRight() << r.bottomLeft();
        room->m_pieces << Piece{0, 4, r.top(), r.bottom()};
        room->m_ring = room->m_points;
        room->m_ringEnds << 4;
    }
    room->finish();
    ++g_roomsMade;
    return room;
}

void WrapRoom::finish()
{
    double l = 1e300, t = 1e300, r = -1e300, b = -1e300, sum = 0;
    for (const QPointF &p : m_points) {
        l = std::min(l, p.x()); r = std::max(r, p.x());
        t = std::min(t, p.y()); b = std::max(b, p.y());
        sum += p.x() * 1.7 + p.y();
    }
    m_bounds = m_points.isEmpty() ? QRectF() : QRectF(QPointF(l, t), QPointF(r, b));
    m_signature = sum + m_points.size();
}

qint64 wrapFlattenWork(const QPainterPath &path)
{
    qint64 curves = 0;
    for (int i = 0, n = path.elementCount(); i < n; ++i) curves += path.elementAt(i).type == QPainterPath::CurveToElement;
    return path.elementCount() + curves * kCurveFlattenWork;
}

std::shared_ptr<const WrapRoom> WrapRoom::around(const QPainterPath &outline, double distance, WrapBudget &budget)
{
    if (outline.elementCount() > kMostWrapPathElements) {
        const QRectF box = outline.controlPointRect();
        const double d = boundedWrapDistance(distance);
        if (budget.shapes <= 0) return nullptr;
        --budget.shapes;
        return ofRect(box.adjusted(-d, -d, d, d));
    }
    // (Charged what flattening costs, before it is done.)
    const qint64 flatten = wrapFlattenWork(outline);
    if (budget.work < flatten) return nullptr;
    const QList<QPolygonF> rings = outline.toSubpathPolygons(QTransform());
    budget.work -= flatten;
    return around(rings, distance, budget);
}

std::shared_ptr<const WrapRoom> WrapRoom::around(const QList<QPolygonF> &rawRings, double distance, WrapBudget &budget, bool open, double halfWidth)
{
    if (budget.shapes <= 0) return nullptr;
    const double d = boundedWrapDistance(distance) + (std::isfinite(halfWidth) ? std::clamp(halfWidth, 0.0, kMostWrapDistance) : 0.0);
    QList<QPolygonF> rings;
    qint64 points = 0;
    // (A ring of more than the cap's points is read in bands: its work is its points, so a budget that has less left does not read it.)
    for (const QPolygonF &raw : rawRings) {
        points += raw.size();
        if (points > budget.work) return nullptr;
        QPolygonF ring = cleanRing(raw, !open);
        if (ring.size() >= (open ? 2 : 3)) rings << ring;
    }
    qint64 total = 0;
    for (const QPolygonF &ring : rings) total += ring.size();
    budget.work -= points;
    --budget.shapes;
    std::shared_ptr<WrapRoom> room(new WrapRoom);
    ++g_roomsMade;
    g_roomWork += points;
    auto addPiece = [&](std::initializer_list<QPointF> corners) {
        Piece piece{int(room->m_points.size()), int(corners.size()), 1e300, -1e300};
        for (const QPointF &c : corners) {
            room->m_points << c;
            piece.top = std::min(piece.top, c.y());
            piece.bottom = std::max(piece.bottom, c.y());
        }
        room->m_pieces << piece;
    };
    if (total > kMostWrapOutlineSegments) {
        // The outline's extent in each of kWrapProfileBands bands from its top to its bottom, the band's rectangle and the distance round it: the work is its points, not the shapes' tangle.
        double top = 1e300, bottom = -1e300;
        for (const QPolygonF &ring : rings)
            for (const QPointF &p : ring) { top = std::min(top, p.y()); bottom = std::max(bottom, p.y()); }
        const double height = std::max(bottom - top, 1e-6), step = height / kWrapProfileBands;
        QVector<double> lefts(kWrapProfileBands, 1e300), rights(kWrapProfileBands, -1e300);
        for (const QPolygonF &ring : rings) {
            const int n = int(ring.size());
            for (int i = 0; i < n; ++i) {
                const QPointF a = ring[i], b = ring[(i + 1) % n];
                if (open && i + 1 == n) break;
                const int b0 = std::clamp(int((std::min(a.y(), b.y()) - top) / step), 0, kWrapProfileBands - 1);
                const int b1 = std::clamp(int((std::max(a.y(), b.y()) - top) / step), 0, kWrapProfileBands - 1);
                // (A segment that crosses many bands is clipped to each: the work is bounded by the bands, and one segment crosses at most all of them.)
                for (int k = b0; k <= b1; ++k) widen(a.x(), a.y(), b.x(), b.y(), top + k * step, top + (k + 1) * step, lefts[k], rights[k]);
            }
        }
        for (int k = 0; k < kWrapProfileBands; ++k) {
            if (lefts[k] > rights[k]) continue;
            const double y0 = top + k * step - d, y1 = top + (k + 1) * step + d;
            addPiece({QPointF(lefts[k] - d, y0), QPointF(rights[k] + d, y0), QPointF(rights[k] + d, y1), QPointF(lefts[k] - d, y1)});
        }
        g_roomWork += room->m_points.size();
        budget.work -= room->m_points.size();
    } else {
        for (const QPolygonF &ring : rings) {
            const int n = int(ring.size());
            if (!open) {
                for (const QPointF &p : ring) room->m_ring << p;
                room->m_ringEnds << int(room->m_ring.size());
            }
            // (A segment's piece for the band round it, with a wedge at each corner on the side the turn opens to, as a stroke with bevel joins is made.)
            const int segments = open ? n - 1 : n;
            auto normalOf = [&](int i) { const QPointF a = ring[i], b = ring[(i + 1) % n]; const double len = std::hypot(b.x() - a.x(), b.y() - a.y()); return QPointF((b.y() - a.y()) / len, -(b.x() - a.x()) / len); };
            if (!(d > 0)) {
                if (open) continue;
                for (int i = 0; i < segments; ++i) addPiece({ring[i], ring[(i + 1) % n]});
                continue;
            }
            for (int i = 0; i < segments; ++i) {
                const QPointF a = ring[i], b = ring[(i + 1) % n], nm = normalOf(i) * d;
                addPiece({a + nm, b + nm, b - nm, a - nm});
                const bool hasNext = open ? i + 1 < segments : true;
                if (!hasNext) continue;
                const int j = (i + 1) % n;
                const QPointF c = ring[(j + 1) % n];
                const QPointF d1 = b - a, d2 = c - b;
                const double turn = d1.x() * d2.y() - d1.y() * d2.x();
                if (turn == 0) continue;
                const QPointF n1 = normalOf(i) * (turn > 0 ? d : -d), n2 = normalOf(j) * (turn > 0 ? d : -d);
                addPiece({b, b + n1, b + n2});
            }
            if (open) {
                // (The ends of a line are squared, as the stroker's are.)
                const QPointF a = ring.first(), b = ring.last();
                const QPointF u = normalOf(0), v = normalOf(n - 2);
                const QPointF ta(-u.y(), u.x()), tb(v.y(), -v.x());
                if (d > 0) {
                    addPiece({a - ta * d + u * d, a + u * d, a - u * d, a - ta * d - u * d});
                    addPiece({b - tb * d + v * d, b + v * d, b - v * d, b - tb * d - v * d});
                }
            }
        }
        g_roomWork += room->m_points.size();
        budget.work -= room->m_points.size();
    }
    room->finish();
    // (The rows a busy room answers from are read when it is first asked; what that costs is charged now, and a budget that has not enough left leaves the object its rectangle.)
    const int rowN = room->rowCount();
    const qint64 rows = rowN ? room->rowWork(rowN) : 0;
    if (budget.work < rows) return nullptr;
    budget.work -= rows;
    g_roomWork += rows;
    return room;
}

bool WrapRoom::spans(double y0, double y1, double *left, double *right) const
{
    if (m_pieces.isEmpty() || !(y1 > y0) || m_bounds.bottom() <= y0 || m_bounds.top() >= y1) return false;
    // (A room of more than kWrapRoomRowPieces pieces answers from its rows, read once: a question costs the rows of the band, never the pieces.)
    if (m_pieces.size() > kWrapRoomRowPieces) return rowSpans(y0, y1, left, right);
    m_queryWork.fetch_add(m_pieces.size());
    double l = 1e300, r = -1e300;
    for (const Piece &piece : m_pieces) {
        if (piece.bottom <= y0 || piece.top >= y1) continue;
        const QPointF *c = m_points.constData() + piece.first;
        if (piece.count == 2) { widen(c[0].x(), c[0].y(), c[1].x(), c[1].y(), y0, y1, l, r); continue; }
        for (int i = 0; i < piece.count; ++i) widen(c[i].x(), c[i].y(), c[(i + 1) % piece.count].x(), c[(i + 1) % piece.count].y(), y0, y1, l, r);
    }
    if (l > r) return false;
    *left = l;
    *right = r;
    return true;
}

// The rows a busy room is read into: kWrapRoomRows, or fewer (never under 32) when the pieces cross so many that reading them would cost more than kWrapRoomRowWork segment tests.
int WrapRoom::rowCount() const
{
    if (m_pieces.size() <= kWrapRoomRowPieces) return 0;
    int rows = kWrapRoomRows;
    while (rows > 32 && rowWork(rows) > kWrapRoomRowWork) rows /= 2;
    return rows;
}

// What the rows cost to make, in segment tests: each edge of a piece, once for each row it crosses.
qint64 WrapRoom::rowWork(int rows) const
{
    const double step = std::max(m_bounds.height() / rows, 1e-9);
    qint64 work = 0;
    for (const Piece &piece : m_pieces) {
        const QPointF *c = m_points.constData() + piece.first;
        for (int i = 0; i < piece.count; ++i) {
            const double ya = c[i].y(), yb = c[(i + 1) % piece.count].y();
            const int r0 = std::clamp(int((std::min(ya, yb) - m_bounds.top()) / step), 0, rows - 1), r1 = std::clamp(int((std::max(ya, yb) - m_bounds.top()) / step), 0, rows - 1);
            work += r1 - r0 + 1;
        }
    }
    return work;
}

void WrapRoom::buildRows() const
{
    const int rows = rowCount();
    m_rowStep = std::max(m_bounds.height() / rows, 1e-9);
    m_rowLeft.fill(1e300, rows);
    m_rowRight.fill(-1e300, rows);
    for (const Piece &piece : m_pieces) {
        const QPointF *c = m_points.constData() + piece.first;
        for (int i = 0; i < piece.count; ++i) {
            const QPointF a = c[i], b = c[(i + 1) % piece.count];
            const auto rowOf = [&](double y) { return int(std::clamp((y - m_bounds.top()) / m_rowStep, 0.0, double(rows - 1))); };
            const int r0 = rowOf(std::min(a.y(), b.y())), r1 = rowOf(std::max(a.y(), b.y()));
            for (int k = r0; k <= r1; ++k) {
                const double y0 = m_bounds.top() + k * m_rowStep;
                widen(a.x(), a.y(), b.x(), b.y(), y0, y0 + m_rowStep, m_rowLeft[k], m_rowRight[k]);
            }
        }
    }
}

bool WrapRoom::rowSpans(double y0, double y1, double *left, double *right) const
{
    std::call_once(m_rowsOnce, [&] { buildRows(); });
    const int rows = int(m_rowLeft.size());
    m_queryWork.fetch_add(1);
    // (Clamped as numbers before they are ints: a band far from a room of a tiny step is a row number no int holds.)
    auto rowOf = [&](double v) { return int(std::clamp(v, 0.0, double(rows - 1))); };
    const int k0 = rowOf(std::floor((y0 - m_bounds.top()) / m_rowStep)), k1 = rowOf(std::ceil((y1 - m_bounds.top()) / m_rowStep) - 1);
    double l = 1e300, r = -1e300;
    for (int k = k0; k <= k1; ++k) { l = std::min(l, m_rowLeft[k]); r = std::max(r, m_rowRight[k]); }
    if (l > r) return false;
    *left = l;
    *right = r;
    return true;
}

bool WrapRoom::covers(const QRectF &rect) const
{
    if (m_pieces.isEmpty() || !rect.isValid() || !(rect.width() > 0) || !(rect.height() > 0) || !m_bounds.adjusted(-0.01, -0.01, 0.01, 0.01).contains(rect)) return false;
    constexpr int rows = 17;
    const double inset = std::min(0.01, rect.height() / 4);
    for (int k = 0; k < rows; ++k) {
        const double y = rect.top() + inset + (rect.height() - 2 * inset) * k / (rows - 1);
        QVector<QPair<double, double>> cover;
        for (const Piece &piece : m_pieces) {
            if (piece.bottom < y || piece.top > y || piece.count < 3) continue;
            double l = 1e300, r = -1e300;
            const QPointF *c = m_points.constData() + piece.first;
            for (int i = 0; i < piece.count; ++i) widen(c[i].x(), c[i].y(), c[(i + 1) % piece.count].x(), c[(i + 1) % piece.count].y(), y, y, l, r);
            if (l <= r) cover << qMakePair(l, r);
        }
        // (The rings are filled by winding: the crossings of the line, left to right, give the intervals where the count is not 0.)
        int start = 0;
        for (int end : m_ringEnds) {
            QVector<QPair<double, int>> crossings;
            const int n = end - start;
            for (int i = 0; i < n; ++i) {
                const QPointF a = m_ring[start + i], b = m_ring[start + (i + 1) % n];
                if ((a.y() <= y && b.y() > y) || (b.y() <= y && a.y() > y)) crossings << qMakePair(a.x() + (y - a.y()) * (b.x() - a.x()) / (b.y() - a.y()), a.y() < b.y() ? 1 : -1);
            }
            std::sort(crossings.begin(), crossings.end());
            int wind = 0;
            for (int i = 0; i < crossings.size(); ++i) {
                const int before = wind;
                wind += crossings[i].second;
                if (before == 0 && wind != 0) {
                    int j = i + 1, w2 = wind;
                    for (; j < crossings.size(); ++j) { w2 += crossings[j].second; if (w2 == 0) break; }
                    if (j < crossings.size()) cover << qMakePair(crossings[i].first, crossings[j].first);
                }
            }
            start = end;
        }
        std::sort(cover.begin(), cover.end());
        double reached = rect.left();
        for (const auto &iv : cover) {
            if (iv.first > reached + 0.01) break;
            reached = std::max(reached, iv.second);
        }
        if (reached < rect.right() - 0.01) return false;
    }
    return true;
}

std::shared_ptr<const WrapRoom> WrapRoom::mapped(const QTransform &matrix) const
{
    std::shared_ptr<WrapRoom> room(new WrapRoom);
    room->m_points.reserve(m_points.size());
    for (const QPointF &p : m_points) room->m_points << held(matrix.map(p));
    room->m_ring.reserve(m_ring.size());
    for (const QPointF &p : m_ring) room->m_ring << held(matrix.map(p));
    room->m_ringEnds = m_ringEnds;
    for (const Piece &piece : m_pieces) {
        Piece q{piece.first, piece.count, 1e300, -1e300};
        for (int i = 0; i < piece.count; ++i) {
            q.top = std::min(q.top, room->m_points[piece.first + i].y());
            q.bottom = std::max(q.bottom, room->m_points[piece.first + i].y());
        }
        room->m_pieces << q;
    }
    room->finish();
    return room;
}

qint64 WrapRoom::madeForTest() { return g_roomsMade.load(); }
qint64 WrapRoom::workForTest() { return g_roomWork.load(); }

namespace {
// The span of a polygon (filled, closed) within the band: the extent of its edges there. A polygon with no area has none.
bool polygonSpan(const QPolygonF &poly, double y0, double y1, double *left, double *right)
{
    const QRectF pb = poly.boundingRect();
    if (poly.size() < 3 || !(pb.width() > 0) || !(pb.height() > 0) || pb.bottom() <= y0 || pb.top() >= y1) return false;
    double l = 1e300, r = -1e300;
    const int n = int(poly.size());
    for (int i = 0; i < n; ++i) widen(poly[i].x(), poly[i].y(), poly[(i + 1) % n].x(), poly[(i + 1) % n].y(), y0, y1, l, r);
    if (l > r) return false;
    *left = l;
    *right = r;
    return true;
}
} // namespace

QRectF WrapObstacle::boundingRect() const
{
    QRectF r = room ? room->bounds().translated(shift) : poly.boundingRect();
    if (room && !wall.isEmpty()) r = r.united(wall.boundingRect());
    return r;
}

void WrapObstacle::translate(double dx, double dy)
{
    poly.translate(dx, dy);
    shift += QPointF(dx, dy);
    wall.translate(dx, dy);
}

bool WrapObstacle::spans(double y0, double y1, double *left, double *right) const
{
    double l = 1e300, r = -1e300, a, b;
    if (room) {
        if (room->spans(y0 - shift.y(), y1 - shift.y(), &a, &b)) { l = a + shift.x(); r = b + shift.x(); }
        if (polygonSpan(wall, y0, y1, &a, &b)) { l = std::min(l, a); r = std::max(r, b); }
    } else if (polygonSpan(poly, y0, y1, &a, &b)) {
        l = a;
        r = b;
    }
    if (l > r) return false;
    *left = l;
    *right = r;
    return true;
}

bool WrapObstacle::isPlain() const
{
    if (room) return false;
    const QRectF pb = poly.boundingRect();
    return poly.size() <= 5 && std::all_of(poly.cbegin(), poly.cend(), [&](const QPointF &pt) {
        return (std::abs(pt.x() - pb.left()) < 1e-6 || std::abs(pt.x() - pb.right()) < 1e-6) && (std::abs(pt.y() - pb.top()) < 1e-6 || std::abs(pt.y() - pb.bottom()) < 1e-6);
    });
}

double WrapObstacle::signature() const
{
    double sum = 0, wallSum = 0;
    for (const QPointF &pt : wall) wallSum += pt.x() * 1.3 + pt.y() * 0.3;
    if (room) sum = room->signature() + shift.x() * 3.1 + shift.y() * 0.7 + wallSum;
    else for (const QPointF &pt : poly) sum += pt.x() * 1.7 + pt.y();
    return sum;
}

QPolygonF wrapOutlineMoved(const QPolygonF &outline, int index, const QPointF &to)
{
    if (index < 0 || index >= outline.size()) return outline;
    QVector<QPointF> points(outline.begin(), outline.end());
    points[index] = to;
    // (Only the point that moved is made to keep inside the reach; the others are as they were.)
    const QPolygonF kept = boundedWrapOutline(points);
    return kept.size() == outline.size() ? kept : outline;
}

QPolygonF wrapOutlineWithPoint(const QPolygonF &outline, const QPointF &near, double tolerance, const QSizeF &frame, int *inserted)
{
    if (inserted) *inserted = -1;
    if (outline.size() < 3 || outline.size() >= kMostWrapPoints || !(frame.width() > 0) || !(frame.height() > 0)) return outline;
    int best = -1;
    double bestDistance = tolerance;
    QPointF at;
    for (int i = 0; i < outline.size(); ++i) {
        const QPointF a(outline[i].x() * frame.width(), outline[i].y() * frame.height());
        const QPointF b(outline[(i + 1) % outline.size()].x() * frame.width(), outline[(i + 1) % outline.size()].y() * frame.height());
        const QPointF p(near.x() * frame.width(), near.y() * frame.height());
        const QPointF ab = b - a;
        const double len2 = ab.x() * ab.x() + ab.y() * ab.y();
        const double t = len2 > 0 ? std::clamp(((p.x() - a.x()) * ab.x() + (p.y() - a.y()) * ab.y()) / len2, 0.0, 1.0) : 0.0;
        const QPointF on = a + ab * t;
        const double d = std::hypot(p.x() - on.x(), p.y() - on.y());
        if (d <= bestDistance) {
            bestDistance = d;
            best = i;
            at = QPointF(on.x() / frame.width(), on.y() / frame.height());
        }
    }
    if (best < 0) return outline;
    QPolygonF out = outline;
    out.insert(best + 1, at);
    if (inserted) *inserted = best + 1;
    return out;
}

QPolygonF wrapOutlineWithout(const QPolygonF &outline, int index)
{
    if (outline.size() <= 3 || index < 0 || index >= outline.size()) return outline;
    QPolygonF out = outline;
    out.remove(index);
    return out;
}

} // namespace jp
