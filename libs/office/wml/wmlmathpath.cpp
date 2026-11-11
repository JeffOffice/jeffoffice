// Equations drawn in two dimensions as outlines (see wmlmathpath.h).

#include "wml/wmlmathpath.h"

#include "wml/wml.h"
#include "wml/wmlmath.h"
#include "wml/xmltree.h"

#include <QBuffer>
#include <QFont>
#include <QImage>
#include <QPainter>
#include <QFontMetricsF>
#include <QPainterPathStroker>
#include <QTransform>
#include <QVector>

#include <algorithm>
#include <cmath>

namespace jp::office::wml {

namespace {

constexpr double E = 100;             // the size of a letter while the equation is laid out; the result is scaled to the size asked for
constexpr double kAscent = 0.76, kDescent = 0.24;   // what a line of text takes above and below its baseline, in letters' heights
constexpr double kAxis = 0.27;        // the height of the middle of a minus sign above the baseline
constexpr double kRule = 0.065;       // the thickness of a fraction bar, a root's line, and a bracket's stroke
constexpr double kScript = 0.7;       // a script is this much the size of what it is on
constexpr int kMostElements = 50000;

struct Box {
    QPainterPath path;
    double w = 0, asc = 0, desc = 0;    // the width, and the height above and below the baseline
    double h() const { return asc + desc; }
    bool empty() const { return w <= 0 && path.isEmpty(); }
};

void put(Box &into, const Box &b, double x, double baselineShift)   // (a positive shift is down)
{
    if (!b.path.isEmpty()) into.path.addPath(b.path.translated(x, baselineShift));
    into.asc = std::max(into.asc, b.asc - baselineShift);
    into.desc = std::max(into.desc, b.desc + baselineShift);
    into.w = std::max(into.w, x + b.w);
}

Box strokeOf(const QPainterPath &center, double width)
{
    QPainterPathStroker s;
    s.setWidth(width);
    s.setCapStyle(Qt::FlatCap);
    s.setJoinStyle(Qt::MiterJoin);
    Box b;
    b.path = s.createStroke(center).simplified();
    return b;
}

Box rule(double x, double y, double w, double thickness)   // a bar, its left end at x and its middle at y (up is negative)
{
    Box b;
    b.path.addRect(QRectF(x, y - thickness / 2, w, thickness));
    b.w = x + w;
    return b;
}

bool isRelation(uint c)
{
    switch (c) {
    case '=': case '<': case '>':
    case 0x2260: case 0x2264: case 0x2265: case 0x2248: case 0x2261: case 0x2245: case 0x223C: case 0x221D: case 0x2208: case 0x2209: case 0x220B:
    case 0x2282: case 0x2283: case 0x2286: case 0x2287: case 0x2192: case 0x2190: case 0x2194: case 0x21D2: case 0x21D0: case 0x21D4: case 0x226A: case 0x226B:
        return true;
    default:
        return false;
    }
}

bool isBinary(uint c)
{
    switch (c) {
    case '+': case '-': case 0x2212: case 0xD7: case 0xF7: case 0xB1: case 0x2213: case 0xB7: case 0x22C5: case 0x2217: case 0x2227: case 0x2228: case 0x2229: case 0x222A:
        return true;
    default:
        return false;
    }
}

bool isLetter(uint c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= 0x3B1 && c <= 0x3C9 && c != 0x3C2); }

class Engine {
public:
    Engine(const XmlTree &tree, const QString &family) : t(tree), m_family(family.isEmpty() ? QStringLiteral("DejaVu Sans") : family) {}
    mutable bool cut = false;

    Box node(int n, int depth, double sc)
    {
        if (depth > kMaxEquationDepth || ++m_elements > kMostElements) {
            cut = true;
            return Box();
        }
        if (t.ns(n) == NsW) return Box();
        const QByteArray name = t.localName(n);
        if (name == "r") return run(n, sc);
        if (name == "f") return fraction(n, depth, sc);
        if (name == "sSup" || name == "sSub" || name == "sSubSup") return scripts(n, depth, sc, false);
        if (name == "sPre") return scripts(n, depth, sc, true);
        if (name == "rad") return radical(n, depth, sc);
        if (name == "nary") return nary(n, depth, sc);
        if (name == "d") return delimited(n, depth, sc);
        if (name == "func") return function(n, depth, sc);
        if (name == "limLow" || name == "limUpp") return limit(n, depth, sc, name == "limUpp");
        if (name == "acc") return accent(n, depth, sc);
        if (name == "bar") return barOver(n, depth, sc);
        if (name == "groupChr") return groupChar(n, depth, sc);
        if (name == "m") return matrix(n, depth, sc);
        if (name == "eqArr") return stack(n, depth, sc);
        if (name == "oMathPara") return paragraph(n, depth, sc);
        if (name == "rPr" || name == "ctrlPr" || name == "fPr" || name == "radPr" || name == "naryPr" || name == "dPr" || name == "accPr" || name == "barPr" || name == "groupChrPr"
            || name == "limLowPr" || name == "limUppPr" || name == "funcPr" || name == "sSupPr" || name == "sSubPr" || name == "sSubSupPr" || name == "sPrePr" || name == "mPr"
            || name == "eqArrPr" || name == "oMathParaPr" || name == "boxPr" || name == "borderBoxPr" || name == "phantPr")
            return Box();
        if (name == "phant") {
            const int pr = child(n, "phantPr");
            const int show = pr >= 0 ? child(pr, "show") : -1;
            if (show >= 0 && !onOff(show, true)) return Box();
        }
        // (oMath, e, num, den, sub, sup, lim, fName, deg, box, borderBox, and what is not known: the pieces in a row)
        return row(n, depth, sc);
    }

    // The children of an element in a row.
    Box row(int n, int depth, double sc)
    {
        Box out;
        const bool before = m_operand;
        m_operand = false;
        double x = 0;
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            const Box b = node(c, depth + 1, sc);
            if (b.empty()) continue;
            put(out, b, x, 0);
            x += b.w;
        }
        out.w = x;
        if (out.asc == 0 && out.desc == 0) {
            out.asc = kAscent * E * sc;
            out.desc = kDescent * E * sc;
        }
        m_operand = before || !out.empty();
        return out;
    }

    // The text of the equation, the line of one level: at the end of the layout.
private:
    int child(int n, const char *name) const
    {
        if (n < 0) return -1;
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            if (++m_elements > kMostElements) {
                cut = true;
                return -1;
            }
            if (t.ns(c) != NsW && t.localName(c) == name) return c;
        }
        return -1;
    }
    QString valOf(int n) const
    {
        if (n < 0) return QString();
        QString v = t.attr(n, "val", t.ns(n));
        if (v.isEmpty()) v = t.attrAny(n, "val");
        if (v.size() > 8) v.truncate(8);
        return v;
    }
    bool onOff(int n, bool otherwise) const
    {
        if (n < 0) return otherwise;
        const QString v = valOf(n).toLower();
        return !(v == QLatin1String("0") || v == QLatin1String("false") || v == QLatin1String("off"));
    }
    // A property element of a properties element: its value, or `otherwise` when it is not there.
    QString prop(int n, const char *props, const char *name, const QString &otherwise) const
    {
        const int p = child(n, props);
        const int c = p >= 0 ? child(p, name) : -1;
        return c >= 0 ? valOf(c) : otherwise;
    }
    Box part(int n, const char *name, int depth, double sc)
    {
        const int c = child(n, name);
        return c < 0 ? Box() : row(c, depth, sc);
    }

    QFont fontFor(bool italic, bool bold, double sc) const
    {
        QFont f(m_family);
        f.setPixelSize(std::max(1, int(std::lround(E * sc))));
        f.setItalic(italic);
        f.setBold(bold);
        f.setHintingPreference(QFont::PreferNoHinting);
        return f;
    }
    Box text(const QString &s, bool italic, bool bold, double sc)
    {
        Box b;
        if (s.isEmpty()) return b;
        chars += int(s.size());
        if (chars > kMaxEquationChars) {
            cut = true;
            return b;
        }
        const QFont f = fontFor(italic, bold, sc);
        const QFontMetricsF fm(f);
        b.path.addText(0, 0, f, s);
        b.w = fm.horizontalAdvance(s);
        b.asc = kAscent * E * sc;
        b.desc = kDescent * E * sc;
        const QRectF ink = b.path.boundingRect();
        if (!ink.isEmpty()) {
            b.asc = std::max(b.asc, -ink.top());
            b.desc = std::max(b.desc, ink.bottom());
        }
        return b;
    }
    Box blank(double em, double sc) const
    {
        Box b;
        b.w = em * E * sc;
        b.asc = kAscent * E * sc;
        b.desc = kDescent * E * sc;
        return b;
    }

    Box run(int r, double sc)
    {
        QByteArray style;
        bool normal = false;
        if (const int pr = child(r, "rPr"); pr >= 0) {
            style = valOf(child(pr, "sty")).toLatin1();
            normal = child(pr, "nor") >= 0 && onOff(child(pr, "nor"), true);
        }
        QString all;
        for (int c = t.firstChild(r); c >= 0; c = t.next(c)) {
            if (t.localName(c) != "t") continue;
            QString s = t.text(c);
            if (t.attrAny(c, "space") != QLatin1String("preserve")) s = s.trimmed();
            all += s;
            if (all.size() > kMaxEquationChars) {
                cut = true;
                all.truncate(kMaxEquationChars);
                break;
            }
        }
        // The characters in pieces of one look (italic letters; upright digits and signs), with room round a relation, and round a sign that works on two things.
        Box out;
        double x = 0;
        QString piece;
        bool pieceItalic = false, pieceBold = false;
        auto flush = [&] {
            if (piece.isEmpty()) return;
            const Box b = text(piece, pieceItalic, pieceBold, sc);
            put(out, b, x, 0);
            x += b.w;
            piece.clear();
        };
        auto gap = [&](double em) {
            flush();
            x += em * E * sc;
        };
        const bool tight = sc < 0.9;
        for (qsizetype i = 0; i < all.size(); ++i) {
            uint ch = all[i].unicode();
            if (QChar::isHighSurrogate(ch) && i + 1 < all.size() && QChar::isLowSurrogate(all[i + 1].unicode())) ch = QChar::surrogateToUcs4(ch, all[i + 1].unicode()), ++i;
            bool italic = false, bold = style == "b" || style == "bi";
            StyledLetter sl;
            QString shown = QString::fromUcs4(reinterpret_cast<const char32_t *>(&ch), 1);
            if (styledLetter(ch, &sl) && sl.style != 'd') {
                shown = QString::fromUcs4(reinterpret_cast<const char32_t *>(&sl.base), 1);
                italic = sl.style == 'i' || sl.style == 'j';
                bold = sl.style == 'b' || sl.style == 'j';
            } else if (isLetter(ch)) {
                italic = !normal && style != "p" && style != "b";
            }
            const bool relation = isRelation(ch), binary = isBinary(ch);
            if (!tight && (relation || (binary && m_operand))) {
                gap(relation ? 0.28 : 0.22);
                piece = shown;
                pieceItalic = italic;
                pieceBold = bold;
                flush();
                gap(relation ? 0.28 : 0.22);
                m_operand = false;
                continue;
            }
            if (!piece.isEmpty() && (pieceItalic != italic || pieceBold != bold)) flush();
            piece += shown;
            pieceItalic = italic;
            pieceBold = bold;
            m_operand = !(relation || binary || ch == '(' || ch == '[' || ch == '{' || ch == ',' || ch == ';');
            if (ch == ' ') {
                piece.chop(shown.size());
                gap(0.25);
            } else if (ch == ',' || ch == ';') {
                flush();
                if (!tight) x += 0.12 * E * sc;
            }
        }
        flush();
        out.w = x;
        if (out.asc == 0 && out.desc == 0) {
            out.asc = kAscent * E * sc;
            out.desc = kDescent * E * sc;
        }
        return out;
    }

    Box fraction(int n, int depth, double sc)
    {
        const QString type = prop(n, "fPr", "type", QStringLiteral("bar"));
        const Box num = part(n, "num", depth, sc), den = part(n, "den", depth, sc);
        Box out;
        m_operand = true;
        if (type == QLatin1String("lin") || type == QLatin1String("skw")) {
            const Box slash = text(QStringLiteral("/"), false, false, sc);
            put(out, num, 0, 0);
            put(out, slash, num.w, 0);
            put(out, den, num.w + slash.w, 0);
            return out;
        }
        const double axis = kAxis * E * sc, gap = 0.12 * E * sc, bar = type == QLatin1String("noBar") ? 0 : kRule * E * sc;
        const double width = std::max(num.w, den.w) + 0.2 * E * sc;
        const double numBase = -(axis + bar / 2 + gap + num.desc), denBase = -axis + bar / 2 + gap + den.asc;
        put(out, num, (width - num.w) / 2, numBase);
        put(out, den, (width - den.w) / 2, denBase);
        if (bar > 0) put(out, rule(0, -axis, width, bar), 0, 0);
        out.w = width;
        return out;
    }

    Box scripts(int n, int depth, double sc, bool before)
    {
        const bool hasSub = child(n, "sub") >= 0, hasSup = child(n, "sup") >= 0;
        const Box base = part(n, "e", depth, sc);
        const double ssc = std::max(0.45, sc * kScript);
        const Box sub = hasSub ? part(n, "sub", depth, ssc) : Box(), sup = hasSup ? part(n, "sup", depth, ssc) : Box();
        m_operand = true;
        const double raise = std::max(0.36 * E * sc, base.asc * 0.6), drop = std::max(0.16 * E * sc, base.desc * 0.8 + 0.06 * E * sc);
        const double pad = 0.04 * E * sc;
        const double scriptW = std::max(sub.w, sup.w);
        Box out;
        double x = 0;
        auto putScripts = [&] {
            if (hasSup) put(out, sup, x + pad, -raise);
            if (hasSub) put(out, sub, x + pad, drop + sub.asc * 0.25);
            x += scriptW + pad * 2;
        };
        if (before) putScripts();
        put(out, base, x, 0);
        x += base.w;
        if (!before) putScripts();
        out.w = x;
        return out;
    }

    Box radical(int n, int depth, double sc)
    {
        const bool noDegree = onOff(child(child(n, "radPr"), "degHide"), false);
        const Box deg = noDegree ? Box() : part(n, "deg", depth, std::max(0.45, sc * 0.5));
        const Box rad = part(n, "e", depth, sc);
        const double top = -(rad.asc + 0.14 * E * sc), bottom = rad.desc + 0.02 * E * sc;
        const double sw = 0.62 * E * sc, hgt = bottom - top;
        const double degW = deg.empty() ? 0 : std::max(0.0, deg.w - 0.3 * sw);
        const double x0 = degW;
        QPainterPath line;
        line.moveTo(x0, top + hgt * 0.58);
        line.lineTo(x0 + sw * 0.2, top + hgt * 0.5);
        line.lineTo(x0 + sw * 0.5, bottom);
        line.lineTo(x0 + sw, top);
        line.lineTo(x0 + sw + rad.w + 0.1 * E * sc, top);
        Box out = strokeOf(line, kRule * E * sc * 1.1);
        out.w = x0 + sw + rad.w + 0.1 * E * sc;
        out.asc = -top + kRule * E * sc;
        out.desc = bottom;
        put(out, rad, x0 + sw + 0.05 * E * sc, 0);
        if (!deg.empty()) put(out, deg, 0, top + hgt * 0.3);
        out.w = x0 + sw + rad.w + 0.1 * E * sc;
        m_operand = true;
        return out;
    }

    Box nary(int n, int depth, double sc)
    {
        const QString chr = prop(n, "naryPr", "chr", QString(QChar(0x222B)));
        const QString loc = prop(n, "naryPr", "limLoc", QString());
        const bool subHide = onOff(child(child(n, "naryPr"), "subHide"), false), supHide = onOff(child(child(n, "naryPr"), "supHide"), false);
        const bool integral = !chr.isEmpty() && (chr[0].unicode() == 0x222B || chr[0].unicode() == 0x222C || chr[0].unicode() == 0x222D || chr[0].unicode() == 0x222E);
        const bool stacked = loc.isEmpty() ? !integral : loc == QLatin1String("undOvr");
        const double ssc = std::max(0.45, sc * kScript);
        const Box sub = subHide ? Box() : part(n, "sub", depth, ssc), sup = supHide ? Box() : part(n, "sup", depth, ssc);
        const Box body = part(n, "e", depth, sc);
        Box op = text(chr.left(2), false, false, sc * (integral ? 1.5 : 1.35));
        // (The sign is centered on the axis.)
        const QRectF ink = op.path.boundingRect();
        const double centerShift = ink.isEmpty() ? 0 : -kAxis * E * sc - (ink.top() + ink.bottom()) / 2;
        Box sign;
        put(sign, op, 0, centerShift);
        sign.w = op.w;
        Box out;
        const double gap = 0.06 * E * sc;
        double x = 0;
        if (stacked) {
            const double width = std::max({sign.w, sub.w, sup.w});
            put(out, sign, (width - sign.w) / 2, 0);
            if (!sup.empty()) put(out, sup, (width - sup.w) / 2, -(sign.asc + gap + sup.desc));
            if (!sub.empty()) put(out, sub, (width - sub.w) / 2, sign.desc + gap + sub.asc);
            x = width;
        } else {
            put(out, sign, 0, 0);
            const double sx = sign.w + (integral ? -0.1 * E * sc : 0.02 * E * sc);
            if (!sup.empty()) put(out, sup, sx, -(sign.asc * 0.82 + sup.desc * 0.3));
            if (!sub.empty()) put(out, sub, sx - (integral ? 0.25 * E * sc : 0), sign.desc * 0.85 + sub.asc * 0.4);
            x = std::max(sign.w, sx + std::max(sub.w, sup.w));
        }
        x += 0.16 * E * sc;
        put(out, body, x, 0);
        out.w = x + body.w;
        m_operand = true;
        return out;
    }

    // A bracket (or a bar) as high as the room it is set round, or a glyph where one fits a line.
    Box fence(const QString &chr, double asc, double desc, double sc, bool opening)
    {
        if (chr.isEmpty()) return Box();
        const uint c = chr[0].unicode();
        const double hgt = asc + desc, w = 0.34 * E * sc, thick = kRule * E * sc;
        const double top = -asc, bottom = desc;
        const bool isOpen = opening;
        QPainterPath p;
        double width = w;
        switch (c) {
        case '(': case ')': {
            const double dir = c == '(' ? 1 : -1;
            const double edge = c == '(' ? w * 0.75 : w * 0.25, bulge = c == '(' ? w * 0.2 : w * 0.8;
            Q_UNUSED(dir);
            p.moveTo(edge, top);
            p.cubicTo(bulge, top + hgt * 0.25, bulge, bottom - hgt * 0.25, edge, bottom);
            break;
        }
        case '[': case ']': {
            const double a = c == '[' ? w * 0.3 : w * 0.7, b = c == '[' ? w * 0.75 : w * 0.25;
            p.moveTo(b, top);
            p.lineTo(a, top);
            p.lineTo(a, bottom);
            p.lineTo(b, bottom);
            break;
        }
        case '{': case '}': {
            const double s = c == '{' ? 1 : -1, mid = (w / 2) * 1.0 + (c == '{' ? 0 : 0);
            const double x0 = c == '{' ? w * 0.75 : w * 0.25, x1 = c == '{' ? w * 0.5 : w * 0.5, x2 = c == '{' ? w * 0.25 : w * 0.75;
            Q_UNUSED(s);
            Q_UNUSED(mid);
            const double my = (top + bottom) / 2;
            p.moveTo(x0, top);
            p.cubicTo(x1, top, x1, top + hgt * 0.2, x1, top + hgt * 0.35);
            p.lineTo(x1, my - hgt * 0.06);
            p.cubicTo(x1, my, x2, my, x2, my);
            p.cubicTo(x2, my, x1, my, x1, my + hgt * 0.06);
            p.lineTo(x1, bottom - hgt * 0.35);
            p.cubicTo(x1, bottom - hgt * 0.2, x1, bottom, x0, bottom);
            break;
        }
        case '|': case 0x2223:
            width = 0.2 * E * sc;
            p.moveTo(width / 2, top);
            p.lineTo(width / 2, bottom);
            break;
        case 0x2016: case 0x2225:
            width = 0.3 * E * sc;
            p.moveTo(width * 0.3, top);
            p.lineTo(width * 0.3, bottom);
            p.moveTo(width * 0.7, top);
            p.lineTo(width * 0.7, bottom);
            break;
        case 0x27E8: case 0x27E9: case '<': case '>': {
            const bool left = c == 0x27E8 || c == '<';
            const double a = left ? w * 0.7 : w * 0.3, b = left ? w * 0.3 : w * 0.7;
            p.moveTo(a, top);
            p.lineTo(b, (top + bottom) / 2);
            p.lineTo(a, bottom);
            break;
        }
        case 0x230A: case 0x230B: case 0x2308: case 0x2309: {
            const bool left = c == 0x230A || c == 0x2308, floor = c == 0x230A || c == 0x230B;
            const double a = left ? w * 0.3 : w * 0.7, b = left ? w * 0.75 : w * 0.25;
            if (floor) {
                p.moveTo(a, top);
                p.lineTo(a, bottom);
                p.lineTo(b, bottom);
            } else {
                p.moveTo(a, bottom);
                p.lineTo(a, top);
                p.lineTo(b, top);
            }
            break;
        }
        default: {
            Box g = text(chr.left(2), false, false, sc);
            return g;
        }
        }
        Q_UNUSED(isOpen);
        Box b = strokeOf(p, thick);
        b.w = width;
        b.asc = asc;
        b.desc = desc;
        return b;
    }

    Box delimited(int n, int depth, double sc)
    {
        const QString open = prop(n, "dPr", "begChr", QStringLiteral("("));
        const QString close = prop(n, "dPr", "endChr", QStringLiteral(")"));
        const QString sep = prop(n, "dPr", "sepChr", QStringLiteral("|"));
        QVector<Box> args;
        for (int c = t.firstChild(n); c >= 0; c = t.next(c))
            if (t.ns(c) != NsW && t.localName(c) == "e") {
                if (args.size() >= 256 || ++m_elements > kMostElements) {
                    cut = true;
                    break;
                }
                args.push_back(row(c, depth + 1, sc));
            }
        double asc = kAscent * E * sc, desc = kDescent * E * sc;
        for (const Box &a : args) asc = std::max(asc, a.asc), desc = std::max(desc, a.desc);
        const double grow = 0.08 * E * sc;
        asc += grow;
        desc += grow;
        const Box left = fence(open, asc, desc, sc, true), right = fence(close, asc, desc, sc, false);
        Box out;
        double x = 0;
        put(out, left, x, 0);
        x += left.w;
        for (int i = 0; i < args.size(); ++i) {
            if (i > 0) {
                const Box s = sep.isEmpty() ? Box() : fence(sep, asc, desc, sc, true);
                x += 0.1 * E * sc;
                put(out, s, x, 0);
                x += s.w + 0.1 * E * sc;
            }
            put(out, args[i], x, 0);
            x += args[i].w;
        }
        put(out, right, x, 0);
        x += right.w;
        out.w = x;
        out.asc = std::max(out.asc, asc);
        out.desc = std::max(out.desc, desc);
        m_operand = true;
        return out;
    }

    Box function(int n, int depth, double sc)
    {
        const Box name = part(n, "fName", depth, sc), arg = part(n, "e", depth, sc);
        Box out;
        put(out, name, 0, 0);
        put(out, arg, name.w + 0.16 * E * sc, 0);
        m_operand = true;
        return out;
    }

    Box limit(int n, int depth, double sc, bool over)
    {
        const Box base = part(n, "e", depth, sc), lim = part(n, "lim", depth, std::max(0.45, sc * kScript));
        const double width = std::max(base.w, lim.w), gap = 0.05 * E * sc;
        Box out;
        put(out, base, (width - base.w) / 2, 0);
        if (over) put(out, lim, (width - lim.w) / 2, -(base.asc + gap + lim.desc));
        else put(out, lim, (width - lim.w) / 2, base.desc + gap + lim.asc);
        out.w = width;
        m_operand = true;
        return out;
    }

    Box accent(int n, int depth, double sc)
    {
        const QString chr = prop(n, "accPr", "chr", QString(QChar(0x302)));
        const Box base = part(n, "e", depth, sc);
        Box out;
        put(out, base, 0, 0);
        const uint c = chr.isEmpty() ? 0x302 : chr[0].unicode();
        const double y = -(base.asc + 0.02 * E * sc);
        if (c == 0x305 || c == 0x304 || c == 0xAF || c == 0x203E) {
            put(out, rule(0, y - 0.06 * E * sc, base.w, kRule * E * sc), 0, 0);
            out.asc = std::max(out.asc, -(y - 0.06 * E * sc) + kRule * E * sc);
        } else if (c == 0x20D7 || c == 0x2192) {
            QPainterPath p;
            const double yy = y - 0.08 * E * sc, hw = 0.07 * E * sc;
            p.moveTo(0.05 * E * sc, yy);
            p.lineTo(base.w - 0.02 * E * sc, yy);
            Box line = strokeOf(p, kRule * E * sc * 0.8);
            QPainterPath head;
            head.moveTo(base.w + 0.03 * E * sc, yy);
            head.lineTo(base.w - 0.08 * E * sc, yy - hw);
            head.lineTo(base.w - 0.08 * E * sc, yy + hw);
            head.closeSubpath();
            line.path.addPath(head);
            line.asc = 0;
            put(out, line, 0, 0);
            out.asc = std::max(out.asc, -(yy - hw));
            out.w = std::max(out.w, base.w);
        } else {
            QString shown;
            switch (c) {
            case 0x300: shown = QStringLiteral("`"); break;
            case 0x301: shown = QStringLiteral("´"); break;
            case 0x302: shown = QStringLiteral("^"); break;
            case 0x303: shown = QStringLiteral("~"); break;
            case 0x306: shown = QStringLiteral("˘"); break;
            case 0x307: shown = QStringLiteral("˙"); break;
            case 0x308: shown = QStringLiteral("¨"); break;
            case 0x30C: shown = QStringLiteral("ˇ"); break;
            default: shown = chr.left(2); break;
            }
            const Box mark = text(shown, false, false, sc * 0.9);
            const QRectF ink = mark.path.boundingRect();
            // (the mark rests just above the letter: its ink's bottom at the top of the base)
            const double lift = ink.isEmpty() ? 0 : -(base.asc * 0.92) - ink.bottom() + 0.04 * E * sc;
            put(out, mark, (base.w - mark.w) / 2, lift);
        }
        m_operand = true;
        return out;
    }

    Box barOver(int n, int depth, double sc)
    {
        const Box base = part(n, "e", depth, sc);
        const bool under = prop(n, "barPr", "pos", QStringLiteral("top")) == QLatin1String("bot");
        Box out;
        put(out, base, 0, 0);
        const double y = under ? base.desc + 0.06 * E * sc : -(base.asc + 0.02 * E * sc);
        put(out, rule(0, y, base.w, kRule * E * sc), 0, 0);
        out.asc = std::max(out.asc, under ? out.asc : -y + kRule * E * sc);
        out.desc = std::max(out.desc, under ? y + kRule * E * sc : out.desc);
        m_operand = true;
        return out;
    }

    Box groupChar(int n, int depth, double sc)
    {
        const QString chr = prop(n, "groupChrPr", "chr", QString(QChar(0x23DF)));
        const bool over = prop(n, "groupChrPr", "pos", QStringLiteral("bot")) == QLatin1String("top");
        const Box base = part(n, "e", depth, sc);
        // (A brace across the width: a line with end ticks.)
        Q_UNUSED(chr);
        QPainterPath p;
        const double y = over ? -(base.asc + 0.1 * E * sc) : base.desc + 0.1 * E * sc, tick = over ? 0.08 * E * sc : -0.08 * E * sc;
        p.moveTo(0, y + tick);
        p.lineTo(0, y);
        p.lineTo(base.w, y);
        p.lineTo(base.w, y + tick);
        Box mark = strokeOf(p, kRule * E * sc);
        Box out;
        put(out, base, 0, 0);
        put(out, mark, 0, 0);
        out.asc = std::max(out.asc, over ? -y + kRule * E * sc : out.asc);
        out.desc = std::max(out.desc, over ? out.desc : y + kRule * E * sc);
        out.w = base.w;
        m_operand = true;
        return out;
    }

    Box matrix(int n, int depth, double sc)
    {
        QVector<QVector<Box>> rows;
        for (int r = t.firstChild(n); r >= 0; r = t.next(r)) {
            if (t.ns(r) == NsW || t.localName(r) != "mr") continue;
            if (rows.size() >= 64 || ++m_elements > kMostElements) {
                cut = true;
                break;
            }
            QVector<Box> cells;
            for (int c = t.firstChild(r); c >= 0; c = t.next(c)) {
                if (t.ns(c) == NsW || t.localName(c) != "e") continue;
                if (cells.size() >= 64 || ++m_elements > kMostElements) {
                    cut = true;
                    break;
                }
                cells.push_back(row(c, depth + 2, sc));
            }
            rows.push_back(cells);
        }
        int cols = 0;
        for (const auto &r : std::as_const(rows)) cols = std::max(cols, int(r.size()));
        QVector<double> width(cols, 0);
        for (const auto &r : std::as_const(rows))
            for (int c = 0; c < r.size(); ++c) width[c] = std::max(width[c], r[c].w);
        const double colGap = 0.8 * E * sc, rowGap = 0.25 * E * sc;
        double total = 0;
        for (int c = 0; c < cols; ++c) total += width[c] + (c ? colGap : 0);
        // The rows from the top, each as tall as its tallest cell.
        double y = 0;
        QVector<double> baselines;
        QVector<double> heights;
        for (const auto &r : std::as_const(rows)) {
            double asc = kAscent * E * sc, desc = kDescent * E * sc;
            for (const Box &b : r) asc = std::max(asc, b.asc), desc = std::max(desc, b.desc);
            baselines.push_back(y + asc);
            y += asc + desc + rowGap;
            heights.push_back(asc + desc);
        }
        const double height = std::max(0.0, y - rowGap);
        const double shift = -kAxis * E * sc - height / 2;   // (the middle of the matrix on the axis)
        Box out;
        for (int ri = 0; ri < rows.size(); ++ri) {
            double x = 0;
            for (int c = 0; c < cols; ++c) {
                if (c < rows[ri].size()) put(out, rows[ri][c], x + (width[c] - rows[ri][c].w) / 2, baselines[ri] + shift);
                x += width[c] + colGap;
            }
        }
        out.w = total;
        if (rows.isEmpty()) out.asc = kAscent * E * sc, out.desc = kDescent * E * sc;
        m_operand = true;
        return out;
    }

    Box stack(int n, int depth, double sc)
    {
        QVector<Box> rows;
        for (int c = t.firstChild(n); c >= 0; c = t.next(c))
            if (t.ns(c) != NsW && t.localName(c) == "e") {
                if (rows.size() >= 64 || ++m_elements > kMostElements) {
                    cut = true;
                    break;
                }
                rows.push_back(row(c, depth + 1, sc));
            }
        double width = 0, y = 0;
        QVector<double> baselines;
        for (const Box &r : std::as_const(rows)) {
            width = std::max(width, r.w);
            baselines.push_back(y + r.asc);
            y += r.h() + 0.2 * E * sc;
        }
        const double shift = -kAxis * E * sc - std::max(0.0, y - 0.2 * E * sc) / 2;
        Box out;
        for (int i = 0; i < rows.size(); ++i) put(out, rows[i], 0, baselines[i] + shift);
        out.w = width;
        if (rows.isEmpty()) out.asc = kAscent * E * sc, out.desc = kDescent * E * sc;
        m_operand = true;
        return out;
    }

    // Equations that are lines of their own, one above the next, each centered.
    Box paragraph(int n, int depth, double sc)
    {
        QVector<Box> lines;
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            if (t.ns(c) == NsW || t.localName(c) != "oMath") continue;
            if (lines.size() >= 64 || ++m_elements > kMostElements) {
                cut = true;
                break;
            }
            lines.push_back(row(c, depth + 1, sc));
        }
        double width = 0;
        for (const Box &l : std::as_const(lines)) width = std::max(width, l.w);
        Box out;
        double y = 0;
        for (int i = 0; i < lines.size(); ++i) {
            if (i) y += lines[i].asc;
            put(out, lines[i], (width - lines[i].w) / 2, y);
            y += lines[i].desc + 0.2 * E * sc;
        }
        out.w = width;
        // (the first line's baseline is the equation's)
        if (!lines.isEmpty()) out.asc = lines[0].asc;
        return out;
    }

    const XmlTree &t;
    QString m_family;
    mutable int m_elements = 0;
    int chars = 0;
    bool m_operand = false;
};

} // namespace

QByteArray equationMarkupDeclared(const QByteArray &markup)
{
    static const QByteArray ns = "xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"";
    const int open = markup.indexOf('<');
    const int close = open >= 0 ? markup.indexOf('>', open) : -1;
    if (close < 0 || markup.mid(open, close - open).contains("xmlns:m=")) return markup;
    int nameEnd = open + 1;
    while (nameEnd < close && markup[nameEnd] != ' ' && markup[nameEnd] != '/' && markup[nameEnd] != '>') ++nameEnd;
    return markup.left(nameEnd) + " " + ns + markup.mid(nameEnd);
}

EquationShape equationShape(const QByteArray &markup, double emSize, const QString &family)
{
    EquationShape out;
    if (markup.isEmpty() || markup.size() > (qint64(16) << 20) || emSize <= 0) return out;
    XmlTree tree;
    if (!tree.parse(equationMarkupDeclared(markup), nullptr)) return out;
    const int root = tree.root();
    if (root < 0) return out;
    const QByteArray name = tree.localName(root);
    if (name != "oMath" && name != "oMathPara") return out;
    Engine engine(tree, family);
    Box box = engine.node(root, 0, 1.0);
    out.cut = engine.cut;
    if (box.empty()) return out;
    const double pad = 0.08 * E;
    const QRectF ink = box.path.boundingRect();
    double left = std::min(0.0, ink.left()), right = std::max(box.w, ink.right());
    double top = -std::max(box.asc, -ink.top()), bottom = std::max(box.desc, ink.bottom());
    left -= pad;
    right += pad;
    top -= pad;
    bottom += pad;
    const double k = emSize / E;
    QTransform tr;
    tr.scale(k, k);
    tr.translate(-left, -top);
    out.path = tr.map(box.path);
    out.box = QRectF(0, 0, (right - left) * k, (bottom - top) * k);
    out.baseline = -top * k;
    return out;
}

bool paintEquation(QPainter *painter, const QByteArray &markup, const QRectF &box, const QColor &color)
{
    const EquationShape shape = equationShape(markup, 24);
    if (!shape.valid() || box.isEmpty()) return false;
    const double k = std::min(box.width() / shape.box.width(), box.height() / shape.box.height());
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->translate(box.left() + (box.width() - shape.box.width() * k) / 2, box.top() + (box.height() - shape.box.height() * k) / 2);
    painter->scale(k, k);
    painter->fillPath(shape.path, color);
    painter->restore();
    return true;
}

QByteArray equationPng(const QByteArray &markup, int width, int height, const QColor &color)
{
    width = std::clamp(width, 1, 4000);
    height = std::clamp(height, 1, 4000);
    QImage img(width, height, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    const bool drawn = paintEquation(&p, markup, QRectF(0, 0, width, height), color);
    p.end();
    if (!drawn) return QByteArray();
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");
    return png;
}

} // namespace jp::office::wml
