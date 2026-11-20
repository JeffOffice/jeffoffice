#include "render/shapes.h"

#include "core/presetgeom.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QHash>
#include <QSet>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {

using V = const QVector<double> &;

QPainterPath poly(std::initializer_list<QPointF> pts)
{
    QPainterPath p;
    bool first = true;
    for (const QPointF &pt : pts) {
        if (first) p.moveTo(pt); else p.lineTo(pt);
        first = false;
    }
    p.closeSubpath();
    return p;
}

QRectF insetRect(double w, double h, double fx, double fy) { return QRectF(w * fx, h * fy, w * (1 - 2 * fx), h * (1 - 2 * fy)); }

// The standard's name for the shape library's preset, and the library's own.
const struct { const char *prst; const char *shape; } kShapes[] = {
    {"rect", "rect"}, {"roundRect", "roundRect"}, {"snip1Rect", "snip1"}, {"snip2SameRect", "snip2same"}, {"snip2DiagRect", "snip2diag"},
    {"snipRoundRect", "snipRound"}, {"round1Rect", "round1"}, {"round2SameRect", "round2same"}, {"round2DiagRect", "round2diag"},
    {"ellipse", "ellipse"}, {"triangle", "triangle"}, {"rtTriangle", "rtTriangle"}, {"parallelogram", "parallelogram"}, {"trapezoid", "trapezoid"},
    {"diamond", "diamond"}, {"pentagon", "pentagon"}, {"hexagon", "hexagon"}, {"heptagon", "heptagon"}, {"octagon", "octagon"},
    {"decagon", "decagon"}, {"dodecagon", "dodecagon"}, {"pie", "pie"}, {"chord", "chord"}, {"teardrop", "teardrop"}, {"frame", "frame"},
    {"halfFrame", "halfFrame"}, {"corner", "corner"}, {"diagStripe", "diagStripe"}, {"plus", "plus"}, {"plaque", "plaque"}, {"can", "can"},
    {"cube", "cube"}, {"bevel", "bevel"}, {"donut", "donut"}, {"noSmoking", "noSmoking"}, {"blockArc", "blockArc"}, {"foldedCorner", "foldedCorner"},
    {"smileyFace", "smiley"}, {"heart", "heart"}, {"lightningBolt", "lightning"}, {"sun", "sun"}, {"moon", "moon"}, {"cloud", "cloud"}, {"arc", "arc"},
    {"bracketPair", "bracketPair"}, {"bracePair", "bracePair"}, {"leftBracket", "leftBracket"}, {"rightBracket", "rightBracket"},
    {"leftBrace", "leftBrace"}, {"rightBrace", "rightBrace"},
    {"rightArrow", "rightArrow"}, {"leftArrow", "leftArrow"}, {"upArrow", "upArrow"}, {"downArrow", "downArrow"}, {"leftRightArrow", "leftRightArrow"},
    {"upDownArrow", "upDownArrow"}, {"quadArrow", "quadArrow"}, {"bentArrow", "bentArrow"}, {"uturnArrow", "uturnArrow"}, {"leftUpArrow", "leftUpArrow"},
    {"bentUpArrow", "bentUpArrow"}, {"curvedRightArrow", "curvedRightArrow"}, {"curvedLeftArrow", "curvedLeftArrow"},
    {"curvedUpArrow", "curvedUpArrow"}, {"curvedDownArrow", "curvedDownArrow"}, {"stripedRightArrow", "stripedRightArrow"},
    {"notchedRightArrow", "notchedRightArrow"}, {"homePlate", "homePlate"}, {"chevron", "chevron"}, {"rightArrowCallout", "rightArrowCallout"},
    {"downArrowCallout", "downArrowCallout"}, {"circularArrow", "circularArrow"},
    {"mathPlus", "mathPlus"}, {"mathMinus", "mathMinus"}, {"mathMultiply", "mathMultiply"}, {"mathDivide", "mathDivide"}, {"mathEqual", "mathEqual"},
    {"mathNotEqual", "mathNotEqual"},
    {"flowChartProcess", "fcProcess"}, {"flowChartAlternateProcess", "fcAltProcess"}, {"flowChartDecision", "fcDecision"},
    {"flowChartInputOutput", "fcData"}, {"flowChartPredefinedProcess", "fcPredefined"}, {"flowChartInternalStorage", "fcInternalStorage"},
    {"flowChartDocument", "fcDocument"}, {"flowChartMultidocument", "fcMultidocument"}, {"flowChartTerminator", "fcTerminator"},
    {"flowChartPreparation", "fcPreparation"}, {"flowChartManualInput", "fcManualInput"}, {"flowChartManualOperation", "fcManualOperation"},
    {"flowChartConnector", "fcConnector"}, {"flowChartOffpageConnector", "fcOffpage"}, {"flowChartPunchedCard", "fcCard"},
    {"flowChartPunchedTape", "fcPunchedTape"}, {"flowChartSummingJunction", "fcSummingJunction"}, {"flowChartOr", "fcOr"},
    {"flowChartCollate", "fcCollate"}, {"flowChartSort", "fcSort"}, {"flowChartExtract", "fcExtract"}, {"flowChartMerge", "fcMerge"},
    // Offline storage is the merge triangle with a letter in it.
    {"flowChartOfflineStorage", "fcMerge"}, {"flowChartOnlineStorage", "fcStoredData"}, {"flowChartMagneticTape", "fcSequential"},
    {"flowChartMagneticDisk", "fcMagneticDisk"}, {"flowChartMagneticDrum", "fcDirectAccess"}, {"flowChartDisplay", "fcDisplay"},
    {"flowChartDelay", "fcDelay"},
    {"irregularSeal1", "irregularSeal1"}, {"irregularSeal2", "irregularSeal2"},
    {"star4", "star4"}, {"star5", "star5"}, {"star6", "star6"}, {"star7", "star7"}, {"star8", "star8"}, {"star10", "star10"}, {"star12", "star12"},
    {"star16", "star16"}, {"star24", "star24"}, {"star32", "star32"},
    {"ribbon", "ribbon"}, {"ribbon2", "ribbon2"}, {"ellipseRibbon", "ellipseRibbon"}, {"ellipseRibbon2", "ellipseRibbon2"},
    {"verticalScroll", "verticalScroll"}, {"horizontalScroll", "horizontalScroll"}, {"wave", "wave"}, {"doubleWave", "doubleWave"},
    {"wedgeRectCallout", "wedgeRectCallout"}, {"wedgeRoundRectCallout", "wedgeRoundRectCallout"}, {"wedgeEllipseCallout", "wedgeEllipseCallout"},
    {"cloudCallout", "cloudCallout"}, {"borderCallout1", "borderCallout1"}, {"borderCallout2", "borderCallout2"}, {"accentCallout1", "accentCallout1"},
    {"callout1", "callout1"},
    {"actionButtonBlank", "actionButtonBlank"}, {"actionButtonHome", "actionButtonHome"}, {"actionButtonHelp", "actionButtonHelp"},
    {"actionButtonInformation", "actionButtonInformation"}, {"actionButtonForwardNext", "actionButtonForwardNext"},
    {"actionButtonBackPrevious", "actionButtonBackPrevious"}, {"actionButtonEnd", "actionButtonEnd"}, {"actionButtonBeginning", "actionButtonBeginning"},
    {"actionButtonReturn", "actionButtonReturn"}, {"actionButtonDocument", "actionButtonDocument"}, {"actionButtonSound", "actionButtonSound"},
    {"actionButtonMovie", "actionButtonMovie"},
};

// The handles the standard gives a preset, as the library's: each moves the adjustments (by their place in the preset's list) that its definition names.
QVector<AdjHandle> standardHandles(const QString &prst)
{
    QVector<AdjHandle> out;
    const GeomSpec *spec = presetSpec(prst);
    if (!spec) return out;
    const QStringList names = presetAdjustmentList(prst);
    for (int i = 0; i < spec->handles.size(); ++i) {
        const GeomHandle &g = spec->handles[i];
        AdjHandle h;
        h.preset = prst;
        h.standard = i;
        h.polar = g.polar;
        h.indexX = g.first.ref.isEmpty() ? -1 : names.indexOf(g.first.ref);
        h.indexY = g.second.ref.isEmpty() ? -1 : names.indexOf(g.second.ref);
        h.index = h.indexX >= 0 ? h.indexX : h.indexY;
        if (h.index >= 0) out.push_back(h);
    }
    return out;
}

// A box a shape can be drawn in.
QSizeF frameOf(const QSizeF &size)
{
    return QSizeF(std::max(0.5, size.width()), std::max(0.5, size.height()));
}

// A shape's adjustments by the standard's names and in its units, from the library's (fractions; the ones left out are the defaults), each held to its handle's range.
QHash<QString, double> standardAdjustments(const QString &prst, const QVector<double> &adj, const QSizeF &size)
{
    const QStringList names = presetAdjustmentList(prst);
    QHash<QString, double> given;
    for (int i = 0; i < names.size() && i < adj.size(); ++i) given.insert(names[i], adj[i] * presetAdjustmentScale(prst, i));
    const QVector<double> values = presetAdjustmentValues(prst, given, size);
    QHash<QString, double> out;
    for (int i = 0; i < names.size(); ++i) out.insert(names[i], values[i]);
    return out;
}

// What a preset shape of this size and these adjustments is, kept for the next time the same shape is drawn.
struct Evaluated {
    Geometry geometry;
    QPainterPath outline;     // the union of what is filled, or, for an open shape, the paths with an outline
    QRectF textRect;
};

thread_local qint64 g_presetEvaluations = 0;

constexpr int kMostCached = 512;

Evaluated evaluatePreset(const ShapeDef &d, const QSizeF &size, const QVector<double> &adj)
{
    // (Each entry says when it was last asked for; when the cache is full the half least recently used goes, so a page of many sizes still finds the ones it draws again and again.)
    struct Kept {
        Evaluated value;
        quint64 used = 0;
    };
    thread_local QHash<QByteArray, Kept> cache;
    thread_local quint64 clock = 0;
    QByteArray key = d.id.toLatin1();
    key.append(reinterpret_cast<const char *>(&size), sizeof(QSizeF));
    key.append(reinterpret_cast<const char *>(adj.constData()), int(adj.size() * sizeof(double)));
    const auto hit = cache.find(key);
    if (hit != cache.end()) {
        hit->used = ++clock;
        return hit->value;
    }
    ++g_presetEvaluations;
    Evaluated e;
    if (const GeomSpec *spec = presetSpec(d.preset)) e.geometry = buildGeometry(*spec, standardAdjustments(d.preset, adj, size), size);
    if (d.open) {
        for (const GeometryPath &part : e.geometry.paths)
            if (part.stroke) e.outline.addPath(part.path);
    } else {
        // The true union of what is filled: parts that wind in opposite ways (a ribbon's fold, a scroll's curl, the sheets of a multi-document) would cancel one another in a sum.
        bool any = false;
        for (const GeometryPath &part : e.geometry.paths) {
            if (part.fill == GeometryPath::None) continue;
            QPainterPath one = part.path;
            one.setFillRule(Qt::WindingFill);
            e.outline = any ? e.outline.united(one) : one;
            any = true;
        }
        if (!any)
            for (const GeometryPath &part : e.geometry.paths) e.outline.addPath(part.path);
    }
    e.outline.setFillRule(Qt::WindingFill);
    e.textRect = e.geometry.textRect.isNull() ? QRectF(QPointF(0, 0), size) : e.geometry.textRect;
    if (cache.size() >= kMostCached) {
        QVector<quint64> times;
        times.reserve(cache.size());
        for (auto it = cache.constBegin(); it != cache.constEnd(); ++it) times.append(it->used);
        std::nth_element(times.begin(), times.begin() + times.size() / 2, times.end());
        const quint64 cut = times[times.size() / 2];
        for (auto it = cache.begin(); it != cache.end();) {
            if (it->used < cut) it = cache.erase(it);
            else ++it;
        }
    }
    cache.insert(key, Kept{e, ++clock});
    return e;
}

// The value in [lo, hi] where f changes sign (f may rise or fall); the end where it is nearest zero when it does not.
template <class F> double crossing(F f, double lo, double hi)
{
    if (!(hi > lo)) return lo;
    double flo = f(lo);
    const double fhi = f(hi);
    if (flo == 0) return lo;
    if (fhi == 0) return hi;
    if ((flo < 0) == (fhi < 0)) return std::fabs(flo) <= std::fabs(fhi) ? lo : hi;
    for (int i = 0; i < 60; ++i) {
        const double mid = (lo + hi) / 2, fm = f(mid);
        if (fm == 0) return mid;
        if ((fm < 0) == (flo < 0)) {
            lo = mid;
            flo = fm;
        } else {
            hi = mid;
        }
    }
    return (lo + hi) / 2;
}

constexpr double kTurn = 21600000;   // a full turn in 60,000ths of a degree

double angleOf(const QPointF &from, const QPointF &to)
{
    double a = std::atan2(to.y() - from.y(), to.x() - from.x()) * 180.0 / M_PI * 60000.0;
    if (a < 0) a += kTurn;
    return a;
}

double turnDifference(double a, double b)
{
    double d = std::fmod(a - b, kTurn);
    if (d > kTurn / 2) d -= kTurn;
    if (d < -kTurn / 2) d += kTurn;
    return d;
}

} // namespace

qint64 presetEvaluationsForTest()
{
    return g_presetEvaluations;
}

static QVector<ShapeDef> build()
{
    QVector<ShapeDef> L;
    auto add = [&](const char *id, const char *name, const char *cat, PathFn fn, QVector<double> defs = {}, QVector<AdjHandle> hs = {},
                   double inset = 0.1, RectFn tr = nullptr, bool open = false) {
        ShapeDef d;
        d.id = QString::fromLatin1(id);
        d.name = QCoreApplication::translate("Render", name);
        d.category = QCoreApplication::translate("Render", cat);
        d.path = fn;
        d.defaults = defs;
        d.handles = hs;
        d.inset = inset;
        d.textRect = tr;
        d.open = open;
        L << d;
    };
    // A shape of the standard's: its drawing, text rectangle, adjustments (with their defaults), and handles come from the standard's definition. `open`: an outline with no fill
    // by default (arcs, brackets, and braces).
    auto addPreset = [&](const char *id, const char *name, const char *cat, bool open = false) {
        ShapeDef d;
        d.id = QString::fromLatin1(id);
        d.name = QCoreApplication::translate("Render", name);
        d.category = QCoreApplication::translate("Render", cat);
        d.preset = presetOfShapeId(d.id);
        const QVector<double> defaults = presetAdjustmentDefaults(d.preset);
        for (int i = 0; i < defaults.size(); ++i) d.defaults << defaults[i] / presetAdjustmentScale(d.preset, i);
        d.handles = standardHandles(d.preset);
        d.open = open;
        L << d;
    };

    // ---------------- Rectangles ----------------
    addPreset("rect", QT_TRANSLATE_NOOP("Render", "Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"));
    addPreset("roundRect", QT_TRANSLATE_NOOP("Render", "Rounded Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"));
    addPreset("snip1", QT_TRANSLATE_NOOP("Render", "Snip Single Corner Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"));
    addPreset("snip2same", QT_TRANSLATE_NOOP("Render", "Snip Same Side Corner Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"));
    addPreset("snip2diag", QT_TRANSLATE_NOOP("Render", "Snip Diagonal Corner Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"));
    addPreset("snipRound", QT_TRANSLATE_NOOP("Render", "Snip and Round Single Corner Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"));
    addPreset("round1", QT_TRANSLATE_NOOP("Render", "Round Single Corner Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"));
    addPreset("round2same", QT_TRANSLATE_NOOP("Render", "Round Same Side Corner Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"));
    addPreset("round2diag", QT_TRANSLATE_NOOP("Render", "Round Diagonal Corner Rectangle"), QT_TRANSLATE_NOOP("Render", "Rectangles"));

    // ---------------- Basic Shapes ----------------
    addPreset("ellipse", QT_TRANSLATE_NOOP("Render", "Oval"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("triangle", QT_TRANSLATE_NOOP("Render", "Isosceles Triangle"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("rtTriangle", QT_TRANSLATE_NOOP("Render", "Right Triangle"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("parallelogram", QT_TRANSLATE_NOOP("Render", "Parallelogram"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("trapezoid", QT_TRANSLATE_NOOP("Render", "Trapezoid"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("diamond", QT_TRANSLATE_NOOP("Render", "Diamond"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("pentagon", QT_TRANSLATE_NOOP("Render", "Regular Pentagon"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("hexagon", QT_TRANSLATE_NOOP("Render", "Hexagon"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("heptagon", QT_TRANSLATE_NOOP("Render", "Heptagon"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("octagon", QT_TRANSLATE_NOOP("Render", "Octagon"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("decagon", QT_TRANSLATE_NOOP("Render", "Decagon"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("dodecagon", QT_TRANSLATE_NOOP("Render", "Dodecagon"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("pie", QT_TRANSLATE_NOOP("Render", "Partial Circle"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("chord", QT_TRANSLATE_NOOP("Render", "Chord"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("teardrop", QT_TRANSLATE_NOOP("Render", "Teardrop"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("frame", QT_TRANSLATE_NOOP("Render", "Frame"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("halfFrame", QT_TRANSLATE_NOOP("Render", "Half Frame"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("corner", QT_TRANSLATE_NOOP("Render", "L-Shape"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("diagStripe", QT_TRANSLATE_NOOP("Render", "Diagonal Stripe"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("plus", QT_TRANSLATE_NOOP("Render", "Cross"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("plaque", QT_TRANSLATE_NOOP("Render", "Plaque"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("can", QT_TRANSLATE_NOOP("Render", "Can"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("cube", QT_TRANSLATE_NOOP("Render", "Cube"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("bevel", QT_TRANSLATE_NOOP("Render", "Bevel"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("donut", QT_TRANSLATE_NOOP("Render", "Donut"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("noSmoking", QT_TRANSLATE_NOOP("Render", "\"No\" Symbol"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("blockArc", QT_TRANSLATE_NOOP("Render", "Block Arc"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("foldedCorner", QT_TRANSLATE_NOOP("Render", "Folded Corner"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("smiley", QT_TRANSLATE_NOOP("Render", "Smiley Face"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("heart", QT_TRANSLATE_NOOP("Render", "Heart"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("lightning", QT_TRANSLATE_NOOP("Render", "Lightning Bolt"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("sun", QT_TRANSLATE_NOOP("Render", "Sun"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("moon", QT_TRANSLATE_NOOP("Render", "Moon"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("cloud", QT_TRANSLATE_NOOP("Render", "Cloud"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"));
    addPreset("arc", QT_TRANSLATE_NOOP("Render", "Arc"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), true);
    addPreset("bracketPair", QT_TRANSLATE_NOOP("Render", "Double Bracket"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), true);
    addPreset("bracePair", QT_TRANSLATE_NOOP("Render", "Double Brace"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), true);
    addPreset("leftBracket", QT_TRANSLATE_NOOP("Render", "Left Bracket"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), true);
    addPreset("rightBracket", QT_TRANSLATE_NOOP("Render", "Right Bracket"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), true);
    addPreset("leftBrace", QT_TRANSLATE_NOOP("Render", "Left Brace"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), true);
    addPreset("rightBrace", QT_TRANSLATE_NOOP("Render", "Right Brace"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), true);

    // ---------------- Block Arrows ----------------
    addPreset("rightArrow", QT_TRANSLATE_NOOP("Render", "Right Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("leftArrow", QT_TRANSLATE_NOOP("Render", "Left Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("upArrow", QT_TRANSLATE_NOOP("Render", "Up Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("downArrow", QT_TRANSLATE_NOOP("Render", "Down Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("leftRightArrow", QT_TRANSLATE_NOOP("Render", "Left-Right Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("upDownArrow", QT_TRANSLATE_NOOP("Render", "Up-Down Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("quadArrow", QT_TRANSLATE_NOOP("Render", "Quad Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("bentArrow", QT_TRANSLATE_NOOP("Render", "Bent Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("uturnArrow", QT_TRANSLATE_NOOP("Render", "U-Turn Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("leftUpArrow", QT_TRANSLATE_NOOP("Render", "Left-Up Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("bentUpArrow", QT_TRANSLATE_NOOP("Render", "Bent-Up Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("curvedRightArrow", QT_TRANSLATE_NOOP("Render", "Curved Right Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("curvedLeftArrow", QT_TRANSLATE_NOOP("Render", "Curved Left Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("curvedUpArrow", QT_TRANSLATE_NOOP("Render", "Curved Up Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("curvedDownArrow", QT_TRANSLATE_NOOP("Render", "Curved Down Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("stripedRightArrow", QT_TRANSLATE_NOOP("Render", "Striped Right Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("notchedRightArrow", QT_TRANSLATE_NOOP("Render", "Notched Right Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("homePlate", QT_TRANSLATE_NOOP("Render", "Pentagon Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("chevron", QT_TRANSLATE_NOOP("Render", "Chevron"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("rightArrowCallout", QT_TRANSLATE_NOOP("Render", "Right Arrow Callout"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("downArrowCallout", QT_TRANSLATE_NOOP("Render", "Down Arrow Callout"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));
    addPreset("circularArrow", QT_TRANSLATE_NOOP("Render", "Circular Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"));

    // ---------------- Equation Shapes ----------------
    addPreset("mathPlus", QT_TRANSLATE_NOOP("Render", "Plus"), QT_TRANSLATE_NOOP("Render", "Equation Shapes"));
    addPreset("mathMinus", QT_TRANSLATE_NOOP("Render", "Minus"), QT_TRANSLATE_NOOP("Render", "Equation Shapes"));
    addPreset("mathMultiply", QT_TRANSLATE_NOOP("Render", "Multiply"), QT_TRANSLATE_NOOP("Render", "Equation Shapes"));
    addPreset("mathDivide", QT_TRANSLATE_NOOP("Render", "Division"), QT_TRANSLATE_NOOP("Render", "Equation Shapes"));
    addPreset("mathEqual", QT_TRANSLATE_NOOP("Render", "Equal"), QT_TRANSLATE_NOOP("Render", "Equation Shapes"));
    addPreset("mathNotEqual", QT_TRANSLATE_NOOP("Render", "Not Equal"), QT_TRANSLATE_NOOP("Render", "Equation Shapes"));

    // ---------------- Flowchart ----------------
    addPreset("fcProcess", QT_TRANSLATE_NOOP("Render", "Flowchart: Process"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcAltProcess", QT_TRANSLATE_NOOP("Render", "Flowchart: Alternate Process"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcDecision", QT_TRANSLATE_NOOP("Render", "Flowchart: Decision"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcData", QT_TRANSLATE_NOOP("Render", "Flowchart: Data"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcPredefined", QT_TRANSLATE_NOOP("Render", "Flowchart: Predefined Process"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcInternalStorage", QT_TRANSLATE_NOOP("Render", "Flowchart: Internal Storage"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcDocument", QT_TRANSLATE_NOOP("Render", "Flowchart: Document"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcMultidocument", QT_TRANSLATE_NOOP("Render", "Flowchart: Multidocument"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcTerminator", QT_TRANSLATE_NOOP("Render", "Flowchart: Terminator"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcPreparation", QT_TRANSLATE_NOOP("Render", "Flowchart: Preparation"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcManualInput", QT_TRANSLATE_NOOP("Render", "Flowchart: Manual Input"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcManualOperation", QT_TRANSLATE_NOOP("Render", "Flowchart: Manual Operation"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcConnector", QT_TRANSLATE_NOOP("Render", "Flowchart: Connector"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcOffpage", QT_TRANSLATE_NOOP("Render", "Flowchart: Off-page Connector"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcCard", QT_TRANSLATE_NOOP("Render", "Flowchart: Card"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcPunchedTape", QT_TRANSLATE_NOOP("Render", "Flowchart: Punched Tape"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcSummingJunction", QT_TRANSLATE_NOOP("Render", "Flowchart: Summing Junction"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcOr", QT_TRANSLATE_NOOP("Render", "Flowchart: Or"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcCollate", QT_TRANSLATE_NOOP("Render", "Flowchart: Collate"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcSort", QT_TRANSLATE_NOOP("Render", "Flowchart: Sort"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcExtract", QT_TRANSLATE_NOOP("Render", "Flowchart: Extract"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcMerge", QT_TRANSLATE_NOOP("Render", "Flowchart: Merge"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcStoredData", QT_TRANSLATE_NOOP("Render", "Flowchart: Stored Data"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcDelay", QT_TRANSLATE_NOOP("Render", "Flowchart: Delay"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcSequential", QT_TRANSLATE_NOOP("Render", "Flowchart: Sequential Access Storage"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcMagneticDisk", QT_TRANSLATE_NOOP("Render", "Flowchart: Magnetic Disk"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcDirectAccess", QT_TRANSLATE_NOOP("Render", "Flowchart: Direct Access Storage"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    addPreset("fcDisplay", QT_TRANSLATE_NOOP("Render", "Flowchart: Display"), QT_TRANSLATE_NOOP("Render", "Flowchart"));
    // ---------------- Stars and Banners ----------------
    addPreset("irregularSeal1", QT_TRANSLATE_NOOP("Render", "Explosion 1"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("irregularSeal2", QT_TRANSLATE_NOOP("Render", "Explosion 2"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("star4", QT_TRANSLATE_NOOP("Render", "4-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("star5", QT_TRANSLATE_NOOP("Render", "5-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("star6", QT_TRANSLATE_NOOP("Render", "6-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("star7", QT_TRANSLATE_NOOP("Render", "7-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("star8", QT_TRANSLATE_NOOP("Render", "8-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("star10", QT_TRANSLATE_NOOP("Render", "10-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("star12", QT_TRANSLATE_NOOP("Render", "12-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("star16", QT_TRANSLATE_NOOP("Render", "16-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("star24", QT_TRANSLATE_NOOP("Render", "24-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("star32", QT_TRANSLATE_NOOP("Render", "32-Point Star"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("ribbon2", QT_TRANSLATE_NOOP("Render", "Up Ribbon"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("ribbon", QT_TRANSLATE_NOOP("Render", "Down Ribbon"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("ellipseRibbon2", QT_TRANSLATE_NOOP("Render", "Curved Up Ribbon"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("ellipseRibbon", QT_TRANSLATE_NOOP("Render", "Curved Down Ribbon"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("verticalScroll", QT_TRANSLATE_NOOP("Render", "Vertical Scroll"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("horizontalScroll", QT_TRANSLATE_NOOP("Render", "Horizontal Scroll"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("wave", QT_TRANSLATE_NOOP("Render", "Wave"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));
    addPreset("doubleWave", QT_TRANSLATE_NOOP("Render", "Double Wave"), QT_TRANSLATE_NOOP("Render", "Stars and Banners"));

    // ---------------- Callouts ----------------
    addPreset("wedgeRectCallout", QT_TRANSLATE_NOOP("Render", "Rectangular Callout"), QT_TRANSLATE_NOOP("Render", "Callouts"));
    addPreset("wedgeRoundRectCallout", QT_TRANSLATE_NOOP("Render", "Rounded Rectangular Callout"), QT_TRANSLATE_NOOP("Render", "Callouts"));
    addPreset("wedgeEllipseCallout", QT_TRANSLATE_NOOP("Render", "Oval Callout"), QT_TRANSLATE_NOOP("Render", "Callouts"));
    addPreset("cloudCallout", QT_TRANSLATE_NOOP("Render", "Cloud Callout"), QT_TRANSLATE_NOOP("Render", "Callouts"));
    addPreset("borderCallout1", QT_TRANSLATE_NOOP("Render", "Line Callout 1"), QT_TRANSLATE_NOOP("Render", "Callouts"));
    addPreset("borderCallout2", QT_TRANSLATE_NOOP("Render", "Line Callout 2"), QT_TRANSLATE_NOOP("Render", "Callouts"));
    addPreset("accentCallout1", QT_TRANSLATE_NOOP("Render", "Line Callout 1 (Accent Bar)"), QT_TRANSLATE_NOOP("Render", "Callouts"));
    addPreset("callout1", QT_TRANSLATE_NOOP("Render", "Line Callout 1 (No Border)"), QT_TRANSLATE_NOOP("Render", "Callouts"));
    // ---------------- Action / misc ----------------
    addPreset("actionButtonBlank", QT_TRANSLATE_NOOP("Render", "Action Button: Blank"), QT_TRANSLATE_NOOP("Render", "Action Buttons"));
    addPreset("actionButtonHome", QT_TRANSLATE_NOOP("Render", "Action Button: Home"), QT_TRANSLATE_NOOP("Render", "Action Buttons"));
    L.back().slideAction = true;
    L.back().link = QStringLiteral("#first");
    addPreset("actionButtonHelp", QT_TRANSLATE_NOOP("Render", "Action Button: Help"), QT_TRANSLATE_NOOP("Render", "Action Buttons"));
    L.back().slideAction = true;
    L.back().link = QStringLiteral("");
    addPreset("actionButtonInformation", QT_TRANSLATE_NOOP("Render", "Action Button: Information"), QT_TRANSLATE_NOOP("Render", "Action Buttons"));
    L.back().slideAction = true;
    L.back().link = QStringLiteral("");
    addPreset("actionButtonForwardNext", QT_TRANSLATE_NOOP("Render", "Action Button: Forward or Next"), QT_TRANSLATE_NOOP("Render", "Action Buttons"));
    L.back().slideAction = true;
    L.back().link = QStringLiteral("#next");
    addPreset("actionButtonBackPrevious", QT_TRANSLATE_NOOP("Render", "Action Button: Back or Previous"), QT_TRANSLATE_NOOP("Render", "Action Buttons"));
    L.back().slideAction = true;
    L.back().link = QStringLiteral("#previous");
    addPreset("actionButtonEnd", QT_TRANSLATE_NOOP("Render", "Action Button: End"), QT_TRANSLATE_NOOP("Render", "Action Buttons"));
    L.back().slideAction = true;
    L.back().link = QStringLiteral("#last");
    addPreset("actionButtonBeginning", QT_TRANSLATE_NOOP("Render", "Action Button: Beginning"), QT_TRANSLATE_NOOP("Render", "Action Buttons"));
    L.back().slideAction = true;
    L.back().link = QStringLiteral("#first");
    addPreset("actionButtonReturn", QT_TRANSLATE_NOOP("Render", "Action Button: Return"), QT_TRANSLATE_NOOP("Render", "Action Buttons"));
    L.back().slideAction = true;
    L.back().link = QStringLiteral("#previous");
    addPreset("actionButtonDocument", QT_TRANSLATE_NOOP("Render", "Action Button: Document"), QT_TRANSLATE_NOOP("Render", "Action Buttons"));
    L.back().slideAction = true;
    L.back().link = QStringLiteral("");
    addPreset("actionButtonSound", QT_TRANSLATE_NOOP("Render", "Action Button: Sound"), QT_TRANSLATE_NOOP("Render", "Action Buttons"));
    L.back().slideAction = true;
    L.back().link = QStringLiteral("");
    addPreset("actionButtonMovie", QT_TRANSLATE_NOOP("Render", "Action Button: Movie"), QT_TRANSLATE_NOOP("Render", "Action Buttons"));
    L.back().slideAction = true;
    L.back().link = QStringLiteral("");
    add("tab", QT_TRANSLATE_NOOP("Render", "Tab"), QT_TRANSLATE_NOOP("Render", "Basic Shapes"), [](double w, double h, V) { return poly({{w * 0.15, 0}, {w * 0.85, 0}, {w, h}, {0, h}}); }, {}, {}, 0.18);
    add("flowArrow", QT_TRANSLATE_NOOP("Render", "Flow Arrow"), QT_TRANSLATE_NOOP("Render", "Block Arrows"), [](double w, double h, V) {
        return poly({{0, h * 0.2}, {w * 0.6, h * 0.2}, {w * 0.6, 0}, {w, h / 2}, {w * 0.6, h}, {w * 0.6, h * 0.8}, {0, h * 0.8}, {w * 0.15, h / 2}}); }, {}, {}, 0.2);
    return L;
}

const QVector<ShapeDef> &shapeLibrary()
{
    static const QVector<ShapeDef> lib = build();
    return lib;
}

const ShapeDef *shapeDef(const QString &id)
{
    for (const auto &d : shapeLibrary())
        if (d.id == id) return &d;
    return nullptr;
}

QString shapeIdOfPreset(const QString &prst)
{
    for (const auto &m : kShapes)
        if (prst == QLatin1String(m.prst)) return QString::fromLatin1(m.shape);
    return QString();
}

QString presetOfShapeId(const QString &shapeId)
{
    for (const auto &m : kShapes)
        if (shapeId == QLatin1String(m.shape)) return QString::fromLatin1(m.prst);
    return QString();
}

QVector<double> shapeAdj(const QString &id, const QVector<double> &adj)
{
    const ShapeDef *d = shapeDef(id);
    QVector<double> out = d ? d->defaults : QVector<double>();
    for (int i = 0; i < adj.size() && i < out.size(); ++i) out[i] = adj[i];
    return out;
}

bool hasCornerRadius(const QString &id)
{
    const ShapeDef *d = shapeDef(id);
    if (!d) return false;
    static const QSet<QString> rounded{QStringLiteral("roundRect"), QStringLiteral("round1Rect"), QStringLiteral("round2SameRect"), QStringLiteral("round2DiagRect")};
    return rounded.contains(d->preset);
}

double cornerRadius(const QString &id, const QVector<double> &adj, const QSizeF &size)
{
    const QVector<double> a = shapeAdj(id, adj);
    return hasCornerRadius(id) && !a.isEmpty() && std::isfinite(a[0]) ? std::clamp(a[0], 0.0, 0.5) * std::min(size.width(), size.height()) : 0;
}

QVector<double> adjWithCornerRadius(const QString &id, const QVector<double> &adj, const QSizeF &size, double radius)
{
    QVector<double> a = shapeAdj(id, adj);
    const double shorter = std::min(size.width(), size.height());
    if (hasCornerRadius(id) && !a.isEmpty() && shorter > 0) a[0] = std::clamp(radius / shorter, 0.0, 0.5);
    return a;
}

static QVector<double> adjBeforeModel2(const QString &id, const QSizeF &size, const QVector<double> &adj)
{
    // (What each shape's list of defaults was, for the shapes whose adjustments changed their meaning or their defaults.)
    static const QHash<QString, QVector<double>> old = {
        {"parallelogram", {0.25}}, {"trapezoid", {0.25}}, {"hexagon", {0.25}}, {"halfFrame", {0.333}}, {"leftUpArrow", {0.18, 0.22}},
        {"mathEqual", {0.235}}, {"mathDivide", {0.235}}, {"star4", {0.19}}, {"star5", {0.38}}, {"star6", {0.58}}, {"star7", {0.6}}, {"star8", {0.72}},
        {"star10", {0.76}}, {"star12", {0.78}}, {"star16", {0.82}}, {"star24", {0.86}}, {"star32", {0.88}}, {"mathPlus", {0.235}}, {"mathMinus", {0.235}},
        {"mathMultiply", {0.235}}, {"mathNotEqual", {0.235}}, {"wedgeRectCallout", {0.2, 1.0}}, {"wedgeRoundRectCallout", {0.2, 1.0}}, {"wedgeEllipseCallout", {0.18, 1.0}}};
    const auto at = old.constFind(id);
    if (at == old.cend()) return adj;
    QVector<double> a = at.value();
    for (int i = 0; i < adj.size(); ++i) {
        if (i < a.size()) a[i] = adj[i];
        else a.append(adj[i]);
    }
    const double w = std::max(0.5, size.width()), h = std::max(0.5, size.height()), m = std::min(w, h);
    if (id == QLatin1String("parallelogram") || id == QLatin1String("trapezoid") || id == QLatin1String("hexagon")) {
        a[0] = a[0] * w / m;
    } else if (id == QLatin1String("halfFrame")) {
        a = {a[0], a[0]};
    } else if (id == QLatin1String("mathEqual")) {
        a = {a[0] * 0.8, a[0] * 0.48};
    } else if (id == QLatin1String("mathNotEqual")) {
        a = {a[0] * 0.8, 6600000.0 / 21600000, a[0] * 0.48};   // (the slash was turned 20 degrees: the standard's 110)
    } else if (id == QLatin1String("mathDivide")) {
        a = {a[0] * 0.6, a[0] * 0.396, a[0] * 0.33};
    } else if (id.startsWith(QLatin1String("wedge"))) {
        a[0] -= 0.5;
        a[1] -= 0.5;
        // (The rounded one's corner was 0.16 of the box above its pointer, now a share of the frame's shorter side.)
        if (id == QLatin1String("wedgeRoundRectCallout")) a = {a[0], a[1], 0.16 * std::min(w, h * 0.78) / m};
    }
    return a;
}

// A save of 0.42 and before, whose arcs, pies, chords, block arcs, snipped and corner shapes, U-turn arrows, and arrows with a head of their own length keep the old library's own meanings
// for their adjustments. Each becomes the standard's, so that the shape looks as it did.
QVector<double> adjFromModel2Library(const QString &id, const QSizeF &size, const QVector<double> &adj)
{
    const auto at = [&](int i, double d) { return i < adj.size() ? adj[i] : d; };
    const auto turn = [](double t) { return t - std::floor(t); };
    // The old angles were those of Qt's ellipse (the angle that a point's own coordinates are made from, as a turn from the right, clockwise here); the standard's is the ray from the
    // center, and on a frame that is not square the two differ. This is the standard's angle of the point of the old ellipse at an angle.
    const double a = std::max(0.5, size.width()) / 2, b = std::max(0.5, size.height()) / 2;
    const auto ray = [&](double t) {
        const double r = turn(std::atan2(b * std::sin(t * 2 * M_PI), a * std::cos(t * 2 * M_PI)) / (2 * M_PI));
        return r > 1 - 1e-12 ? 0.0 : r;
    };
    // (Angles are fractions of a turn, clockwise from the right, as the standard's. The old library measured its angles anticlockwise.)
    if (id == QLatin1String("arc")) {
        // The old one began at the top and went round a fraction of a turn.
        const double sweep = std::clamp(at(0, 0.25), 1e-6, 0.999999);
        return {0.75, ray(0.75 + sweep)};
    }
    if (id == QLatin1String("pie")) {
        // From the right, anticlockwise.
        const double sweep = std::clamp(at(0, 0.75), 1e-6, 0.999999);
        return {ray(1 - sweep), 0.0};
    }
    if (id == QLatin1String("chord")) {
        // From the upper right (45 degrees), anticlockwise.
        const double sweep = std::clamp(at(0, 0.75), 1e-6, 0.999999);
        return {ray(0.875 - sweep), ray(0.875)};
    }
    if (id == QLatin1String("blockArc")) return {0.5, 0.0, at(0, 0.25)};
    if (id == QLatin1String("snip2diag")) return {0.0, at(0, 0.1667)};
    if (id == QLatin1String("snipRound") || id == QLatin1String("corner")) return {at(0, id == QLatin1String("corner") ? 0.5 : 0.1667), at(0, id == QLatin1String("corner") ? 0.5 : 0.1667)};
    if (id == QLatin1String("uturnArrow")) return {at(0, 0.25), 0.25, at(1, 0.25), 0.4375, 0.75};
    if (id == QLatin1String("leftUpArrow") || id == QLatin1String("bentUpArrow")) return {at(0, 0.25), at(1, 0.25), at(1, 0.25)};
    if (id == QLatin1String("quadArrow")) {
        // The old shaft could be thicker than its heads are wide, which the standard does not draw (a shaft is at most as wide as a head): the heads are widened to the shaft, and their length
        // held to what is left of the half frame, so that the shaft keeps its thickness.
        const double t = at(0, 0.12), head = std::max(at(1, 0.22), t);
        return {t, head, std::min(at(2, 0.22), std::max(0.0, 0.5 - head))};
    }
    return adj;
}

QVector<double> adjFromOldLibrary(const QString &id, const QSizeF &size, const QVector<double> &adj)
{
    return adjFromModel2Library(id, size, adjBeforeModel2(id, size, adj));
}

QPainterPath shapePath(const QString &id, const QSizeF &size, const QVector<double> &adj)
{
    const ShapeDef *d = shapeDef(id);
    if (!d) d = &shapeLibrary().front();
    const QSizeF frame = frameOf(size);
    if (!d->preset.isEmpty()) return evaluatePreset(*d, frame, shapeAdj(d->id, adj)).outline;
    QPainterPath p = d->path(frame.width(), frame.height(), shapeAdj(d->id, adj));
    p.setFillRule(Qt::WindingFill);
    return p;
}

QVector<GeometryPath> shapeParts(const QString &id, const QSizeF &size, const QVector<double> &adj)
{
    const ShapeDef *d = shapeDef(id);
    if (d && !d->preset.isEmpty()) return evaluatePreset(*d, frameOf(size), shapeAdj(d->id, adj)).geometry.paths;
    GeometryPath whole;
    whole.path = shapePath(id, size, adj);
    return {whole};
}

QRectF shapeTextRect(const QString &id, const QSizeF &size, const QVector<double> &adj)
{
    const ShapeDef *d = shapeDef(id);
    const double w = size.width(), h = size.height();
    if (!d) return QRectF(0, 0, w, h);
    if (!d->preset.isEmpty()) return evaluatePreset(*d, frameOf(size), shapeAdj(id, adj)).textRect;
    if (d->textRect) return d->textRect(w, h, shapeAdj(id, adj));
    return insetRect(w, h, d->inset, d->inset);
}

QPointF adjHandlePos(const AdjHandle &hd, const QSizeF &s, const QVector<double> &adj)
{
    if (hd.standard >= 0) {
        const QSizeF frame = frameOf(s);
        HandleGeometry g;
        const GeomSpec *spec = presetSpec(hd.preset);
        if (spec && evaluateHandle(*spec, hd.standard, standardAdjustments(hd.preset, adj, frame), frame, &g)) return g.at;
        return QPointF();
    }
    const double basis = hd.basis == 0 ? s.width() : hd.basis == 1 ? s.height() : std::min(s.width(), s.height());
    const double v = adj.value(hd.index) * basis;
    if (hd.axis == 0) return QPointF(hd.fromEnd ? s.width() - v : v, hd.other * s.height());
    return QPointF(hd.other * s.width(), hd.fromEnd ? s.height() - v : v);
}

void adjDrag(const AdjHandle &hd, const QSizeF &s, QVector<double> &adj, const QPointF &p)
{
    if (hd.standard < 0) {
        if (hd.index >= 0 && hd.index < adj.size()) adj[hd.index] = adjFromPoint(hd, s, p);
        return;
    }
    const GeomSpec *spec = presetSpec(hd.preset);
    const QStringList names = presetAdjustmentList(hd.preset);
    if (!spec || hd.standard >= spec->handles.size() || !std::isfinite(p.x()) || !std::isfinite(p.y())) return;
    const QSizeF frame = frameOf(s);
    const QVector<double> defaults = presetAdjustmentDefaults(hd.preset);
    while (adj.size() < names.size()) adj.append(defaults[adj.size()] / presetAdjustmentScale(hd.preset, adj.size()));
    QHash<QString, double> now = standardAdjustments(hd.preset, adj, frame);
    const GeomHandle &g = spec->handles[hd.standard];
    const int index[2] = {hd.indexX, hd.indexY};
    const QString refs[2] = {g.first.ref, g.second.ref};
    // Where the handle is with one adjustment set to `v`.
    const auto placeWith = [&](int axis, double v) {
        QHash<QString, double> trial = now;
        trial.insert(refs[axis], v);
        HandleGeometry at;
        return evaluateHandle(*spec, hd.standard, trial, frame, &at) ? at.at : QPointF();
    };
    const auto range = [&](int axis, double *lo, double *hi) {
        HandleGeometry at;
        if (!evaluateHandle(*spec, hd.standard, now, frame, &at)) return false;
        *lo = at.min[axis];
        *hi = at.max[axis];
        return *hi >= *lo;
    };
    const QPointF center(frame.width() / 2, frame.height() / 2);
    // (A point handle moves its adjustments along x and y; a polar one moves its angle, then its radius.)
    const int order[2] = {g.polar ? 1 : 0, g.polar ? 0 : 1};
    // (A polar handle that moves both is set again, once the radius is known, for the angle that is a little off where the radius is not the first one's.)
    const bool both = g.polar && index[0] >= 0 && index[1] >= 0;
    const int steps = both ? 4 : 2;
    // Each adjustment on its own, the angle first, then the radius (a polar one that moves both is set again, once the radius is known).
    const auto separately = [&] {
        for (int step = 0; step < steps; ++step) {
            const int axis = order[step % 2];
            double lo, hi;
            if (index[axis] < 0 || !range(axis, &lo, &hi)) continue;
            double v;
            if (!g.polar) {
                v = crossing([&](double t) { const QPointF at = placeWith(axis, t); return (axis == 0 ? at.x() : at.y()) - (axis == 0 ? p.x() : p.y()); }, lo, hi);
            } else if (axis == 1) {
                const double target = angleOf(center, p);
                v = std::clamp(target, lo, hi);
                // (The angle of the handle's own place is not always the adjustment's: let the place's error correct it.)
                for (int i = 0; i < 12; ++i) v = std::clamp(v + turnDifference(target, angleOf(center, placeWith(1, v))), lo, hi);
            } else {
                // The radius that brings the handle nearest the point: the best of some samples across the range, then narrowed in.
                const auto miss = [&](double t) { const QPointF at = placeWith(0, t); return std::hypot(at.x() - p.x(), at.y() - p.y()); };
                constexpr int kSamples = 32;
                int best = 0;
                double bestMiss = miss(lo);
                for (int i = 1; i <= kSamples; ++i) {
                    const double m = miss(lo + (hi - lo) * i / kSamples);
                    if (m < bestMiss) {
                        bestMiss = m;
                        best = i;
                    }
                }
                double a = lo + (hi - lo) * std::max(0, best - 1) / kSamples, b = lo + (hi - lo) * std::min(kSamples, best + 1) / kSamples;
                for (int i = 0; i < 40; ++i) {
                    const double m1 = a + (b - a) / 3, m2 = b - (b - a) / 3;
                    if (miss(m1) < miss(m2)) b = m2; else a = m1;
                }
                v = (a + b) / 2;
            }
            now.insert(refs[axis], v);
            adj[index[axis]] = v / presetAdjustmentScale(hd.preset, index[axis]);
        }
    };
    if (!both) {
        separately();
        return;
    }
    // A polar handle that moves both: the place is not always the angle and radius about the center, so the two are set together, by Newton's steps from where they are (the last
    // drag's, which is near; a mouse moves a little at a time), then, if that is not enough, from the two set apart, and last from the best of a grid made finer and finer.
    double lo[2], hi[2], x[2] = {now.value(refs[0]), now.value(refs[1])};
    if (!range(0, &lo[0], &hi[0]) || !range(1, &lo[1], &hi[1])) return;
    // `coarse`: the other adjustments are not held to their ranges, which these two change (so a search of many tries costs a fraction of one that does).
    bool coarse = true;
    const auto at = [&](const double *v) {
        QHash<QString, double> trial = now;
        trial.insert(refs[0], v[0]);
        trial.insert(refs[1], v[1]);
        if (!coarse) {
            const QVector<double> values = presetAdjustmentValues(hd.preset, trial, frame);
            for (int i = 0; i < names.size(); ++i) trial.insert(names[i], values[i]);
        }
        HandleGeometry place;
        return evaluateHandle(*spec, hd.standard, trial, frame, &place) ? place.at : QPointF();
    };
    const auto missAt = [&](const double *v, bool exact) {
        const bool was = coarse;
        coarse = !exact;
        const QPointF f = at(v) - p;
        coarse = was;
        return std::hypot(f.x(), f.y());
    };
    const auto newton = [&](bool exact) {
        coarse = !exact;
        for (int i = 0; i < 12; ++i) {
            const QPointF f = at(x) - p;
            const double was = std::hypot(f.x(), f.y());
            if (was < 1e-4) return;
            const double step[2] = {std::max(1e-3, 1e-5 * (hi[0] - lo[0])), std::max(1e-3, 1e-5 * (hi[1] - lo[1]))};
            double j[2][2];
            for (int c = 0; c < 2; ++c) {
                double y[2] = {x[0], x[1]};
                y[c] += step[c];
                const QPointF d = at(y) - at(x);
                j[0][c] = d.x() / step[c];
                j[1][c] = d.y() / step[c];
            }
            const double det = j[0][0] * j[1][1] - j[0][1] * j[1][0];
            if (std::fabs(det) < 1e-12) return;
            const double d0 = (j[1][1] * f.x() - j[0][1] * f.y()) / det, d1 = (-j[1][0] * f.x() + j[0][0] * f.y()) / det;
            // (Halve the step until the place is nearer.)
            bool nearer = false;
            double scale = 1;
            for (int k = 0; k < 6 && !nearer; ++k, scale /= 2) {
                const double y[2] = {std::clamp(x[0] - scale * d0, lo[0], hi[0]), std::clamp(x[1] - scale * d1, lo[1], hi[1])};
                const QPointF e = at(y) - p;
                if (std::hypot(e.x(), e.y()) < was) {
                    x[0] = y[0];
                    x[1] = y[1];
                    nearer = true;
                }
            }
            if (!nearer) return;
        }
    };
    double best[2] = {x[0], x[1]};
    double bestMiss = 1e18;
    const auto keep = [&] {
        const double m = missAt(x, true);
        if (m < bestMiss) {
            bestMiss = m;
            best[0] = x[0];
            best[1] = x[1];
        }
        return m <= 0.01;
    };
    bool done = false;
    newton(false);
    done = keep();
    if (!done) {
        // Newton's steps from the adjustments set one at a time.
        separately();
        x[0] = now.value(refs[0]);
        x[1] = now.value(refs[1]);
        if (range(0, &lo[0], &hi[0]) && range(1, &lo[1], &hi[1])) {
            newton(false);
            done = keep();
        }
    }
    if (!done) {
        // The place is not a smooth function of the two all across their ranges: the best of a coarse grid, then of finer and finer ones around it, then Newton's steps.
        coarse = true;
        double c[2] = {(lo[0] + hi[0]) / 2, (lo[1] + hi[1]) / 2}, half[2] = {(hi[0] - lo[0]) / 2, (hi[1] - lo[1]) / 2};
        int rows = 9, cols = 17;
        double g2[2] = {c[0], c[1]};
        double gridBest = 1e18;
        for (int level = 0; level < 5; ++level) {
            for (int r = 0; r < rows; ++r)
                for (int a = 0; a < cols; ++a) {
                    const double y[2] = {std::clamp(c[0] - half[0] + 2 * half[0] * r / (rows - 1), lo[0], hi[0]), std::clamp(c[1] - half[1] + 2 * half[1] * a / (cols - 1), lo[1], hi[1])};
                    const QPointF e = at(y) - p;
                    if (std::hypot(e.x(), e.y()) < gridBest) {
                        gridBest = std::hypot(e.x(), e.y());
                        g2[0] = y[0];
                        g2[1] = y[1];
                    }
                }
            c[0] = g2[0];
            c[1] = g2[1];
            for (int k = 0; k < 2; ++k) half[k] = half[k] * 2 / ((k == 0 ? rows : cols) - 1) * 1.5;
            rows = cols = 5;
        }
        x[0] = g2[0];
        x[1] = g2[1];
        newton(false);
        keep();
    }
    for (int k = 0; k < 2; ++k) {
        now.insert(refs[k], best[k]);
        adj[index[k]] = best[k] / presetAdjustmentScale(hd.preset, index[k]);
    }
}

double adjFromPoint(const AdjHandle &hd, const QSizeF &s, const QPointF &p, const QVector<double> &adj)
{
    if (hd.standard >= 0) {
        QVector<double> a = adj;
        adjDrag(hd, s, a, p);
        return hd.index >= 0 && hd.index < a.size() ? a[hd.index] : 0.0;
    }
    const double basis = std::max(0.01, hd.basis == 0 ? s.width() : hd.basis == 1 ? s.height() : std::min(s.width(), s.height()));
    double v = hd.axis == 0 ? p.x() : p.y();
    if (hd.fromEnd) v = (hd.axis == 0 ? s.width() : s.height()) - v;
    const double most = hd.limit > 0 ? hd.limit * (hd.axis == 0 ? s.width() : s.height()) / basis : hd.max;
    return std::clamp(v / basis, hd.min, std::max(hd.min, most));
}

} // namespace jp
