// The outlines of custom shapes in an open document presentation
// (odpimport_p.h): draw:enhanced-geometry holds a path in commands
// (M L C Z N F S ...) of numbers, equations, and handle values, and the
// equations that make the numbers of the shape's size. The shape library
// draws the shapes whose type the standard names; the others are drawn from
// their paths.

#include "io/odpimport_p.h"

#include <QPainterPath>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace jp::deck::odp {

namespace {

constexpr int kMaxEquationDepth = 64;
constexpr int kMaxEquations = 512;
constexpr int kMaxPathTokens = 200000;
constexpr double kMaxCoordinate = 1e7;

// Evaluates the equations of a shape and the numbers of its path.
class Evaluator {
public:
    Evaluator(const QDomElement &geometry, const QSizeF &frame, const QRectF &view)
    {
        m_frame = frame;
        m_view = view;
        for (const QDomElement &eq : kids(geometry, "draw:equation")) {
            if (m_formulas.size() >= kMaxEquations) break;
            m_formulas.insert(at(eq, "draw:name"), at(eq, "draw:formula"));
        }
        const QString modifiers = at(geometry, "draw:modifiers").simplified();
        if (!modifiers.isEmpty())
            for (const QString &m : modifiers.split(QLatin1Char(' '), Qt::SkipEmptyParts)) {
                double v = 0;
                m_modifiers.append(parseNumber(m, &v) ? v : 0.0);
            }
    }

    double value(const QString &token)
    {
        const QString t = token.trimmed();
        if (t.isEmpty()) return 0;
        double v = 0;
        if (parseNumber(t, &v)) return v;
        return expression(t);
    }

    double expression(const QString &text)
    {
        if (++m_depth > kMaxEquationDepth) {
            --m_depth;
            return 0;
        }
        const QString s = text;
        int pos = 0;
        double v = sum(s, &pos);
        --m_depth;
        return std::isfinite(v) ? std::clamp(v, -1e15, 1e15) : 0.0;
    }

private:
    static bool nameChar(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_'); }

    static void skip(const QString &s, int *pos)
    {
        while (*pos < s.size() && s[*pos].isSpace()) ++*pos;
    }

    double sum(const QString &s, int *pos)
    {
        double v = product(s, pos);
        for (;;) {
            skip(s, pos);
            if (*pos < s.size() && (s[*pos] == QLatin1Char('+') || s[*pos] == QLatin1Char('-'))) {
                const bool plus = s[*pos] == QLatin1Char('+');
                ++*pos;
                const double r = product(s, pos);
                v = plus ? v + r : v - r;
            } else {
                return v;
            }
        }
    }

    double product(const QString &s, int *pos)
    {
        double v = unary(s, pos);
        for (;;) {
            skip(s, pos);
            if (*pos < s.size() && (s[*pos] == QLatin1Char('*') || s[*pos] == QLatin1Char('/'))) {
                const bool times = s[*pos] == QLatin1Char('*');
                ++*pos;
                const double r = unary(s, pos);
                v = times ? v * r : (r != 0 ? v / r : 0.0);
            } else {
                return v;
            }
        }
    }

    double unary(const QString &s, int *pos)
    {
        skip(s, pos);
        if (*pos >= s.size()) return 0;
        if (s[*pos] == QLatin1Char('-')) {
            ++*pos;
            return -unary(s, pos);
        }
        if (s[*pos] == QLatin1Char('+')) {
            ++*pos;
            return unary(s, pos);
        }
        return primary(s, pos);
    }

    double primary(const QString &s, int *pos)
    {
        skip(s, pos);
        if (*pos >= s.size()) return 0;
        const QChar c = s[*pos];
        if (c == QLatin1Char('(')) {
            ++*pos;
            const double v = sum(s, pos);
            skip(s, pos);
            if (*pos < s.size() && s[*pos] == QLatin1Char(')')) ++*pos;
            return v;
        }
        if (c.isDigit() || c == QLatin1Char('.')) {
            const int start = *pos;
            while (*pos < s.size() && (s[*pos].isDigit() || s[*pos] == QLatin1Char('.'))) ++*pos;
            if (*pos < s.size() && (s[*pos] == QLatin1Char('e') || s[*pos] == QLatin1Char('E')) && *pos + 1 < s.size()
                && (s[*pos + 1].isDigit() || ((s[*pos + 1] == QLatin1Char('-') || s[*pos + 1] == QLatin1Char('+')) && *pos + 2 < s.size() && s[*pos + 2].isDigit()))) {
                *pos += 2;
                while (*pos < s.size() && s[*pos].isDigit()) ++*pos;
            }
            double v = 0;
            parseNumber(s.mid(start, *pos - start), &v);
            return v;
        }
        if (c == QLatin1Char('$')) {
            ++*pos;
            const int start = *pos;
            while (*pos < s.size() && s[*pos].isDigit()) ++*pos;
            const int index = s.mid(start, *pos - start).toInt();
            return index >= 0 && index < m_modifiers.size() ? m_modifiers[index] : 0.0;
        }
        if (c == QLatin1Char('?')) {
            ++*pos;
            const int start = *pos;
            while (*pos < s.size() && nameChar(s[*pos])) ++*pos;
            return named(s.mid(start, *pos - start));
        }
        if (c.isLetter()) {
            const int start = *pos;
            while (*pos < s.size() && nameChar(s[*pos])) ++*pos;
            const QString word = s.mid(start, *pos - start).toLower();
            skip(s, pos);
            if (*pos < s.size() && s[*pos] == QLatin1Char('(')) {
                ++*pos;
                QVector<double> args;
                for (;;) {
                    skip(s, pos);
                    if (*pos >= s.size()) break;
                    if (s[*pos] == QLatin1Char(')')) {
                        ++*pos;
                        break;
                    }
                    args.append(sum(s, pos));
                    skip(s, pos);
                    if (*pos < s.size() && (s[*pos] == QLatin1Char(',') || s[*pos] == QLatin1Char(';'))) ++*pos;
                    else if (*pos < s.size() && s[*pos] != QLatin1Char(')')) ++*pos;   // something unreadable: step over it
                    if (args.size() > 8) break;
                }
                return call(word, args);
            }
            return constant(word);
        }
        ++*pos;   // a character with no meaning
        return 0;
    }

    double call(const QString &name, const QVector<double> &a) const
    {
        auto arg = [&](int i) { return i < a.size() ? a[i] : 0.0; };
        if (name == QLatin1String("abs")) return std::fabs(arg(0));
        if (name == QLatin1String("sqrt")) return arg(0) > 0 ? std::sqrt(arg(0)) : 0.0;
        if (name == QLatin1String("sin")) return std::sin(arg(0));
        if (name == QLatin1String("cos")) return std::cos(arg(0));
        if (name == QLatin1String("tan")) return std::tan(arg(0));
        if (name == QLatin1String("atan")) return std::atan(arg(0));
        if (name == QLatin1String("atan2")) return std::atan2(arg(0), arg(1));
        if (name == QLatin1String("min")) return std::min(arg(0), arg(1));
        if (name == QLatin1String("max")) return std::max(arg(0), arg(1));
        if (name == QLatin1String("if")) return arg(0) > 0 ? arg(1) : arg(2);
        return 0;
    }

    double constant(const QString &name) const
    {
        if (name == QLatin1String("pi")) return M_PI;
        if (name == QLatin1String("logwidth")) return m_view.width();
        if (name == QLatin1String("logheight")) return m_view.height();
        if (name == QLatin1String("width")) return m_frame.width();
        if (name == QLatin1String("height")) return m_frame.height();
        if (name == QLatin1String("left")) return m_view.left();
        if (name == QLatin1String("top")) return m_view.top();
        if (name == QLatin1String("right")) return m_view.right();
        if (name == QLatin1String("bottom")) return m_view.bottom();
        if (name == QLatin1String("xstretch") || name == QLatin1String("ystretch")) return 0;
        if (name == QLatin1String("hasstroke") || name == QLatin1String("hasfill")) return 1;
        return 0;
    }

    double named(const QString &name)
    {
        const auto it = m_formulas.constFind(name);
        if (it == m_formulas.cend()) return 0;
        if (const auto done = m_values.constFind(name); done != m_values.cend()) return done.value();
        if (m_active.contains(name)) return 0;
        m_active.insert(name);
        const double v = expression(it.value());
        m_active.remove(name);
        m_values.insert(name, v);
        return v;
    }

    QSizeF m_frame;
    QRectF m_view;
    QHash<QString, QString> m_formulas;
    QHash<QString, double> m_values;
    QSet<QString> m_active;
    QVector<double> m_modifiers;
    int m_depth = 0;
};

// The tokens of a path: command letters and operands.
struct Token {
    QChar command;     // non-null for a command letter
    QString operand;
};

QVector<Token> tokenize(const QString &path)
{
    QVector<Token> out;
    int i = 0;
    while (i < path.size() && out.size() < kMaxPathTokens) {
        const QChar c = path[i];
        if (c.isSpace() || c == QLatin1Char(',')) {
            ++i;
        } else if (c.isUpper() && c.unicode() < 128) {
            out.append({c, QString()});
            ++i;
        } else {
            // A number, a name, or an equation reference: up to a space or the next command.
            const int start = i;
            while (i < path.size() && !path[i].isSpace() && path[i] != QLatin1Char(',') && !(path[i].isUpper() && path[i].unicode() < 128 && i > start)) {
                // A sign after a number's end starts the next one.
                if (i > start && (path[i] == QLatin1Char('-') || path[i] == QLatin1Char('+')) && path[i - 1] != QLatin1Char('e') && path[i - 1] != QLatin1Char('E')) break;
                ++i;
            }
            if (i == start) ++i;
            out.append({QChar(), path.mid(start, i - start)});
        }
    }
    return out;
}

struct Builder {
    QPainterPath path;
    bool fill = true, stroke = true;
    QPointF current;
    bool open = false;
};

QPointF point(double x, double y) { return QPointF(std::clamp(x, -kMaxCoordinate, kMaxCoordinate), std::clamp(y, -kMaxCoordinate, kMaxCoordinate)); }

// A quarter of an ellipse from the current point to (x, y), starting along x or along y.
void quadrant(Builder &b, double x, double y, bool alongX)
{
    const QPointF p0 = b.current, p1 = point(x, y);
    constexpr double k = 0.5522847498;
    QPointF c1, c2;
    if (alongX) {
        c1 = QPointF(p0.x() + k * (p1.x() - p0.x()), p0.y());
        c2 = QPointF(p1.x(), p1.y() - k * (p1.y() - p0.y()));
    } else {
        c1 = QPointF(p0.x(), p0.y() + k * (p1.y() - p0.y()));
        c2 = QPointF(p1.x() - k * (p1.x() - p0.x()), p1.y());
    }
    b.path.cubicTo(c1, c2, p1);
    b.current = p1;
}

// An arc of the ellipse in a rectangle, from the angle (Qt's, counter-clockwise from the right) a through `sweep` degrees.
void arc(Builder &b, const QRectF &r, double start, double sweep, bool lineToStart)
{
    if (r.width() <= 0 || r.height() <= 0) return;
    const double a = qDegreesToRadians(start);
    const QPointF from(r.center().x() + r.width() / 2 * std::cos(a), r.center().y() - r.height() / 2 * std::sin(a));
    if (lineToStart && b.open) b.path.lineTo(from);
    else b.path.moveTo(from);
    b.path.arcTo(r, start, sweep);
    b.current = b.path.currentPosition();
    b.open = true;
}

// The angle (Qt's) of a point seen from the center of an ellipse.
double angleOf(const QRectF &r, const QPointF &p)
{
    const double rx = r.width() / 2, ry = r.height() / 2;
    if (rx <= 0 || ry <= 0) return 0;
    return qRadiansToDegrees(std::atan2(-(p.y() - r.center().y()) / ry, (p.x() - r.center().x()) / rx));
}

} // namespace

office::Geometry enhancedGeometry(const QDomElement &geometry, const QSizeF &size)
{
    office::Geometry out;
    const QString d = at(geometry, "draw:enhanced-path");
    if (geometry.isNull() || d.trimmed().isEmpty() || size.width() <= 0 || size.height() <= 0) return out;

    // The path's own coordinates: the view box, else the size it says it is drawn at, else the frame's.
    QRectF view(0, 0, size.width(), size.height());
    const QStringList box = at(geometry, "svg:viewBox").simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    double v[4] = {0, 0, 0, 0};
    bool boxed = box.size() == 4;
    for (int i = 0; boxed && i < 4; ++i) boxed = parseNumber(box[i], &v[i]);
    if (boxed && v[2] > 0 && v[3] > 0) {
        view = QRectF(v[0], v[1], v[2], v[3]);
    } else {
        const QStringList sub = at(geometry, "drawooo:sub-view-size").simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
        double sw = 0, sh = 0;
        if (sub.size() == 2 && parseNumber(sub[0], &sw) && parseNumber(sub[1], &sh) && sw > 0 && sh > 0) view = QRectF(0, 0, sw, sh);
    }
    Evaluator eval(geometry, size, view);
    const double sx = size.width() / view.width(), sy = size.height() / view.height();
    const bool mirrorH = at(geometry, "draw:mirror-horizontal") == QLatin1String("true"), mirrorV = at(geometry, "draw:mirror-vertical") == QLatin1String("true");
    auto map = [&](double x, double y) {
        double px = (x - view.left()) * sx, py = (y - view.top()) * sy;
        if (mirrorH) px = size.width() - px;
        if (mirrorV) py = size.height() - py;
        return point(px, py);
    };

    const QVector<Token> tokens = tokenize(d);
    // A command's operands, evaluated.
    int at_ = 0;
    auto operands = [&](QVector<double> *vals) {
        vals->clear();
        while (at_ < tokens.size() && tokens[at_].command.isNull()) vals->append(eval.value(tokens[at_++].operand));
    };

    Builder b;
    auto finish = [&] {
        if (!b.path.isEmpty()) {
            office::GeometryPath gp;
            gp.path = b.path;
            gp.fill = b.fill ? office::GeometryPath::Normal : office::GeometryPath::None;
            gp.stroke = b.stroke;
            out.paths.append(gp);
        }
        b = Builder();
    };
    QVector<double> n;
    while (at_ < tokens.size()) {
        if (tokens[at_].command.isNull()) {
            ++at_;   // operands with no command
            continue;
        }
        const QChar cmd = tokens[at_++].command;
        operands(&n);
        switch (cmd.toLatin1()) {
        case 'M':
            for (int i = 0; i + 1 < n.size(); i += 2) {
                const QPointF p = map(n[i], n[i + 1]);
                if (i == 0) b.path.moveTo(p);
                else b.path.lineTo(p);
                b.current = p;
                b.open = true;
            }
            break;
        case 'L':
            for (int i = 0; i + 1 < n.size(); i += 2) {
                const QPointF p = map(n[i], n[i + 1]);
                if (!b.open) b.path.moveTo(p);
                else b.path.lineTo(p);
                b.current = p;
                b.open = true;
            }
            break;
        case 'C':
            for (int i = 0; i + 5 < n.size(); i += 6) {
                if (!b.open) {
                    b.path.moveTo(map(n[i], n[i + 1]));
                    b.open = true;
                }
                const QPointF p = map(n[i + 4], n[i + 5]);
                b.path.cubicTo(map(n[i], n[i + 1]), map(n[i + 2], n[i + 3]), p);
                b.current = p;
            }
            break;
        case 'Q':
            for (int i = 0; i + 3 < n.size(); i += 4) {
                if (!b.open) {
                    b.path.moveTo(map(n[i], n[i + 1]));
                    b.open = true;
                }
                const QPointF p = map(n[i + 2], n[i + 3]);
                b.path.quadTo(map(n[i], n[i + 1]), p);
                b.current = p;
            }
            break;
        case 'Z':
            if (b.open) b.path.closeSubpath();
            b.current = b.path.currentPosition();
            break;
        case 'N':
            finish();
            break;
        case 'F':
            b.fill = false;
            break;
        case 'S':
            b.stroke = false;
            break;
        case 'X':
        case 'Y':
            for (int i = 0; i + 1 < n.size(); i += 2) {
                if (!b.open) {
                    b.path.moveTo(0, 0);
                    b.open = true;
                }
                quadrant(b, map(n[i], n[i + 1]).x(), map(n[i], n[i + 1]).y(), cmd == QLatin1Char('X'));
            }
            break;
        case 'A': case 'B': case 'V': case 'W':
            // The bounding box of the ellipse, the point the arc starts at, and the one it ends at.
            for (int i = 0; i + 7 < n.size(); i += 8) {
                const QPointF a = map(n[i], n[i + 1]), c = map(n[i + 2], n[i + 3]);
                const QRectF r = QRectF(a, c).normalized();
                const double s = angleOf(r, map(n[i + 4], n[i + 5])), e = angleOf(r, map(n[i + 6], n[i + 7]));
                const bool clockwise = cmd == QLatin1Char('V') || cmd == QLatin1Char('W');
                // Mirroring turns the other way round.
                const bool turned = clockwise != (mirrorH != mirrorV);
                double sweep = turned ? -std::fmod(std::fmod(s - e, 360) + 360, 360.0) : std::fmod(std::fmod(e - s, 360) + 360, 360.0);
                if (sweep == 0) sweep = turned ? -360 : 360;
                arc(b, r, s, sweep, cmd == QLatin1Char('A') || cmd == QLatin1Char('V'));
            }
            break;
        case 'T': case 'U':
            // The center and radii of the ellipse and the angles the arc runs between.
            for (int i = 0; i + 5 < n.size(); i += 6) {
                const QPointF center = map(n[i], n[i + 1]);
                const double rx = std::fabs(n[i + 2] * sx), ry = std::fabs(n[i + 3] * sy);
                const QRectF r(center.x() - rx, center.y() - ry, 2 * rx, 2 * ry);
                // The file counts angles clockwise on the screen; Qt counter-clockwise: the arc runs from the
                // first angle through the sweep to the second.
                const double s = -n[i + 4];
                const double turn = n[i + 5] - n[i + 4];
                double sweep = std::fabs(turn) >= 360 ? (turn > 0 ? -360.0 : 360.0) : -turn;
                arc(b, r, s, sweep, cmd == QLatin1Char('T'));
            }
            break;
        default:
            break;
        }
    }
    finish();

    // The text area: four values, the left, top, right, and bottom.
    const QStringList area = at(geometry, "draw:text-areas").simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (area.size() >= 4) {
        const QPointF a = map(eval.value(area[0]), eval.value(area[1])), c = map(eval.value(area[2]), eval.value(area[3]));
        const QRectF r = QRectF(a, c).normalized();
        if (r.width() > 0 && r.height() > 0 && r != QRectF(0, 0, size.width(), size.height())) out.textRect = r;
    }
    return out;
}

QString presetOfType(const QString &type)
{
    if (type.startsWith(QLatin1String("ooxml-"))) {
        const QString rest = type.mid(6);
        return rest == QLatin1String("non-primitive") ? QString() : rest;
    }
    static const QHash<QString, QString> classic = {
        {QStringLiteral("rectangle"), QStringLiteral("rect")}, {QStringLiteral("round-rectangle"), QStringLiteral("roundRect")},
        {QStringLiteral("ellipse"), QStringLiteral("ellipse")}, {QStringLiteral("diamond"), QStringLiteral("diamond")},
        {QStringLiteral("isosceles-triangle"), QStringLiteral("triangle")}, {QStringLiteral("right-triangle"), QStringLiteral("rtTriangle")},
        {QStringLiteral("parallelogram"), QStringLiteral("parallelogram")}, {QStringLiteral("trapezoid"), QStringLiteral("trapezoid")},
        {QStringLiteral("hexagon"), QStringLiteral("hexagon")}, {QStringLiteral("octagon"), QStringLiteral("octagon")},
        {QStringLiteral("pentagon"), QStringLiteral("pentagon")}, {QStringLiteral("cross"), QStringLiteral("plus")},
        {QStringLiteral("ring"), QStringLiteral("donut")}, {QStringLiteral("can"), QStringLiteral("can")}, {QStringLiteral("cube"), QStringLiteral("cube")},
        {QStringLiteral("frame"), QStringLiteral("frame")}, {QStringLiteral("paper"), QStringLiteral("foldedCorner")},
        {QStringLiteral("bevel"), QStringLiteral("bevel")}, {QStringLiteral("forbidden"), QStringLiteral("noSmoking")},
        {QStringLiteral("block-arc"), QStringLiteral("blockArc")}, {QStringLiteral("circle-pie"), QStringLiteral("pie")},
        {QStringLiteral("heart"), QStringLiteral("heart")}, {QStringLiteral("lightning-bolt"), QStringLiteral("lightningBolt")},
        {QStringLiteral("sun"), QStringLiteral("sun")}, {QStringLiteral("moon"), QStringLiteral("moon")}, {QStringLiteral("cloud"), QStringLiteral("cloud")},
        {QStringLiteral("smiley"), QStringLiteral("smileyFace")}, {QStringLiteral("doorplate"), QStringLiteral("plaque")},
        {QStringLiteral("right-arrow"), QStringLiteral("rightArrow")}, {QStringLiteral("left-arrow"), QStringLiteral("leftArrow")},
        {QStringLiteral("up-arrow"), QStringLiteral("upArrow")}, {QStringLiteral("down-arrow"), QStringLiteral("downArrow")},
        {QStringLiteral("left-right-arrow"), QStringLiteral("leftRightArrow")}, {QStringLiteral("up-down-arrow"), QStringLiteral("upDownArrow")},
        {QStringLiteral("quad-arrow"), QStringLiteral("quadArrow")}, {QStringLiteral("bent-arrow"), QStringLiteral("bentArrow")},
        {QStringLiteral("u-turn-arrow"), QStringLiteral("uturnArrow")}, {QStringLiteral("left-up-arrow"), QStringLiteral("leftUpArrow")},
        {QStringLiteral("bent-up-arrow"), QStringLiteral("bentUpArrow")}, {QStringLiteral("striped-right-arrow"), QStringLiteral("stripedRightArrow")},
        {QStringLiteral("notched-right-arrow"), QStringLiteral("notchedRightArrow")}, {QStringLiteral("pentagon-right"), QStringLiteral("homePlate")},
        {QStringLiteral("chevron"), QStringLiteral("chevron")}, {QStringLiteral("circular-arrow"), QStringLiteral("circularArrow")},
        {QStringLiteral("right-arrow-callout"), QStringLiteral("rightArrowCallout")}, {QStringLiteral("down-arrow-callout"), QStringLiteral("downArrowCallout")},
        {QStringLiteral("left-bracket"), QStringLiteral("leftBracket")}, {QStringLiteral("right-bracket"), QStringLiteral("rightBracket")},
        {QStringLiteral("left-brace"), QStringLiteral("leftBrace")}, {QStringLiteral("right-brace"), QStringLiteral("rightBrace")},
        {QStringLiteral("bracket-pair"), QStringLiteral("bracketPair")}, {QStringLiteral("brace-pair"), QStringLiteral("bracePair")},
        {QStringLiteral("star4"), QStringLiteral("star4")}, {QStringLiteral("star5"), QStringLiteral("star5")}, {QStringLiteral("star6"), QStringLiteral("star6")},
        {QStringLiteral("star8"), QStringLiteral("star8")}, {QStringLiteral("star12"), QStringLiteral("star12")}, {QStringLiteral("star24"), QStringLiteral("star24")},
        {QStringLiteral("bang"), QStringLiteral("irregularSeal1")}, {QStringLiteral("vertical-scroll"), QStringLiteral("verticalScroll")},
        {QStringLiteral("horizontal-scroll"), QStringLiteral("horizontalScroll")}, {QStringLiteral("wave"), QStringLiteral("wave")},
        {QStringLiteral("double-wave"), QStringLiteral("doubleWave")}, {QStringLiteral("ribbon"), QStringLiteral("ribbon2")},
        {QStringLiteral("rectangular-callout"), QStringLiteral("wedgeRectCallout")}, {QStringLiteral("round-rectangular-callout"), QStringLiteral("wedgeRoundRectCallout")},
        {QStringLiteral("round-callout"), QStringLiteral("wedgeEllipseCallout")}, {QStringLiteral("cloud-callout"), QStringLiteral("cloudCallout")},
        {QStringLiteral("flowchart-process"), QStringLiteral("flowChartProcess")}, {QStringLiteral("flowchart-alternate-process"), QStringLiteral("flowChartAlternateProcess")},
        {QStringLiteral("flowchart-decision"), QStringLiteral("flowChartDecision")}, {QStringLiteral("flowchart-data"), QStringLiteral("flowChartInputOutput")},
        {QStringLiteral("flowchart-predefined-process"), QStringLiteral("flowChartPredefinedProcess")}, {QStringLiteral("flowchart-internal-storage"), QStringLiteral("flowChartInternalStorage")},
        {QStringLiteral("flowchart-document"), QStringLiteral("flowChartDocument")}, {QStringLiteral("flowchart-multidocument"), QStringLiteral("flowChartMultidocument")},
        {QStringLiteral("flowchart-terminator"), QStringLiteral("flowChartTerminator")}, {QStringLiteral("flowchart-preparation"), QStringLiteral("flowChartPreparation")},
        {QStringLiteral("flowchart-manual-input"), QStringLiteral("flowChartManualInput")}, {QStringLiteral("flowchart-manual-operation"), QStringLiteral("flowChartManualOperation")},
        {QStringLiteral("flowchart-connector"), QStringLiteral("flowChartConnector")}, {QStringLiteral("flowchart-off-page-connector"), QStringLiteral("flowChartOffpageConnector")},
        {QStringLiteral("flowchart-card"), QStringLiteral("flowChartPunchedCard")}, {QStringLiteral("flowchart-punched-tape"), QStringLiteral("flowChartPunchedTape")},
        {QStringLiteral("flowchart-summing-junction"), QStringLiteral("flowChartSummingJunction")}, {QStringLiteral("flowchart-or"), QStringLiteral("flowChartOr")},
        {QStringLiteral("flowchart-collate"), QStringLiteral("flowChartCollate")}, {QStringLiteral("flowchart-sort"), QStringLiteral("flowChartSort")},
        {QStringLiteral("flowchart-extract"), QStringLiteral("flowChartExtract")}, {QStringLiteral("flowchart-merge"), QStringLiteral("flowChartMerge")},
        {QStringLiteral("flowchart-stored-data"), QStringLiteral("flowChartOnlineStorage")}, {QStringLiteral("flowchart-sequential-access"), QStringLiteral("flowChartMagneticTape")},
        {QStringLiteral("flowchart-magnetic-disk"), QStringLiteral("flowChartMagneticDisk")}, {QStringLiteral("flowchart-direct-access-storage"), QStringLiteral("flowChartMagneticDrum")},
        {QStringLiteral("flowchart-display"), QStringLiteral("flowChartDisplay")}, {QStringLiteral("flowchart-delay"), QStringLiteral("flowChartDelay")},
    };
    return classic.value(type);
}

} // namespace jp::deck::odp
