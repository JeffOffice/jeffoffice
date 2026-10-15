// The looks of an open document presentation's objects (odpexport_p.h): fills,
// outlines, and effects as attributes of graphic properties, with the named
// gradients, dashes, markers, hatches, and pictures they refer to, and the
// styles of dates.

#include "io/odpexport_p.h"

#include <QCoreApplication>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace jp::deck::odpw {

namespace {

QString tr(const char *text) { return QCoreApplication::translate("OdpExport", text); }

QString attrOf(const char *name, const QString &value) { return QLatin1Char(' ') + QLatin1String(name) + QStringLiteral("=\"") + esc(value) + QLatin1Char('"'); }

// The opacity of a fill (share of one) from its transparency, as a percentage.
QString opacityOf(double transparency) { return percent(std::clamp(1.0 - transparency, 0.0, 1.0)); }

} // namespace

QColor Exporter::resolve(const ColorRef &c) const
{
    if (c.isNone()) return QColor();
    const QColor v = c.resolve(m_colors);
    return v.isValid() && v.alpha() > 0 ? QColor(v.red(), v.green(), v.blue()) : QColor();
}

// ---- named things

QString Exporter::namedGradient(const Fill &fill, QString *opacityName)
{
    QVector<GradientStop> stops = fill.stops;
    if (stops.isEmpty()) {
        stops.append({0, fill.color, fill.transparency});
        stops.append({1, fill.color2.isNone() ? ColorRef::rgb(Qt::white) : fill.color2, fill.transparency});
    }
    std::stable_sort(stops.begin(), stops.end(), [](const GradientStop &a, const GradientStop &b) { return a.pos < b.pos; });
    auto color = [&](const GradientStop &s) {
        const QColor c = resolve(s.color);
        return c.isValid() ? c : QColor(Qt::black);
    };
    const bool linear = fill.gradType == Fill::Linear;
    // Three stops alike at the ends are an axial gradient: the first color in the middle.
    const bool axial = linear && stops.size() == 3 && std::fabs(stops[0].pos) < 0.001 && std::fabs(stops[1].pos - 0.5) < 0.01 && std::fabs(stops[2].pos - 1) < 0.001 && color(stops[0]) == color(stops[2]);
    QString style, startColor, endColor, extra;
    GradientStop startStop = stops.first(), endStop = stops.last();
    if (axial) {
        style = QStringLiteral("axial");
        startStop = stops[1];
        endStop = stops[0];
    } else if (linear) {
        style = QStringLiteral("linear");
    } else {
        // The middle is the last color of the file, the edge the first.
        style = fill.gradType == Fill::Radial ? QStringLiteral("radial") : fill.gradType == Fill::PathGrad ? QStringLiteral("ellipsoid") : QStringLiteral("rectangular");
        startStop = stops.last();
        endStop = stops.first();
        extra = attrOf("draw:cx", QStringLiteral("50%")) + attrOf("draw:cy", QStringLiteral("50%"));
    }
    const double angle = std::fmod(90 - fill.angle + 720, 360.0);
    QString stopsXml;
    // More than two colors, or two that start and end inside the gradient (the ends are padded with them), need stops of their own.
    const bool padded = stops.size() == 2 && (stops.first().pos > 0.001 || stops.last().pos < 0.999);
    if (linear && !axial && (stops.size() > 2 || padded)) {
        for (const GradientStop &s : std::as_const(stops))
            stopsXml += QStringLiteral("<loext:gradient-stop svg:offset=\"%1\" loext:color-type=\"rgb\" loext:color-value=\"%2\"/>").arg(number(std::clamp(s.pos, 0.0, 1.0), 4), hexOf(color(s)));
    }
    const QString after = QStringLiteral("\"") + attrOf("draw:style", style) + extra + attrOf("draw:start-color", hexOf(color(startStop))) + attrOf("draw:end-color", hexOf(color(endStop)))
                          + attrOf("draw:start-intensity", QStringLiteral("100%")) + attrOf("draw:end-intensity", QStringLiteral("100%")) + attrOf("draw:angle", number(angle, 2) + QLatin1String("deg"))
                          + attrOf("draw:border", QStringLiteral("0%")) + (stopsXml.isEmpty() ? QStringLiteral("/>") : QLatin1Char('>') + stopsXml + QStringLiteral("</draw:gradient>"));
    const QString name = m_common.add(QStringLiteral("Gradient"), QStringLiteral("<draw:gradient draw:name=\""), after);

    // Its opacity: one share for the whole, or a fade of its own that runs the way the colors do.
    double first = 1 - startStop.transparency, last = 1 - endStop.transparency;
    // The file's start is the first stop of a linear gradient, and the last of the others.
    if (opacityName) {
        opacityName->clear();
        double lo = 1, hi = 0;
        for (const GradientStop &s : std::as_const(stops)) {
            lo = std::min(lo, 1 - s.transparency);
            hi = std::max(hi, 1 - s.transparency);
        }
        if (hi - lo > 0.01) {
            // With stops of its own, the fade has them too (the start and the end say it for a program that has no stops).
            QString opacityStops;
            if (!stopsXml.isEmpty())
                for (const GradientStop &s : std::as_const(stops))
                    opacityStops += QStringLiteral("<loext:opacity-stop svg:offset=\"%1\" svg:stop-opacity=\"%2\"/>").arg(number(std::clamp(s.pos, 0.0, 1.0), 4), number(std::clamp(1 - s.transparency, 0.0, 1.0), 4));
            const QString opacityAfter = QStringLiteral("\"") + attrOf("draw:style", style) + extra + attrOf("draw:start", percent(std::clamp(first, 0.0, 1.0)))
                                         + attrOf("draw:end", percent(std::clamp(last, 0.0, 1.0))) + attrOf("draw:angle", number(angle, 2) + QLatin1String("deg"))
                                         + attrOf("draw:border", QStringLiteral("0%")) + (opacityStops.isEmpty() ? QStringLiteral("/>") : QLatin1Char('>') + opacityStops + QStringLiteral("</draw:opacity>"));
            *opacityName = m_common.add(QStringLiteral("Transparency"), QStringLiteral("<draw:opacity draw:name=\""), opacityAfter);
        }
    }
    return name;
}

QString Exporter::namedDash(const Stroke &s)
{
    QString style = QStringLiteral("rect"), dots, distance = QStringLiteral("300%");
    switch (s.dash) {
    case Stroke::RoundDot:
        style = QStringLiteral("round");
        dots = attrOf("draw:dots1", QStringLiteral("1")) + attrOf("draw:dots1-length", QStringLiteral("100%"));
        distance = QStringLiteral("200%");
        break;
    case Stroke::SquareDot:
        dots = attrOf("draw:dots1", QStringLiteral("1")) + attrOf("draw:dots1-length", QStringLiteral("100%"));
        distance = QStringLiteral("100%");
        break;
    case Stroke::DashLine:
        dots = attrOf("draw:dots1", QStringLiteral("1")) + attrOf("draw:dots1-length", QStringLiteral("400%"));
        break;
    case Stroke::DashDot:
        dots = attrOf("draw:dots1", QStringLiteral("1")) + attrOf("draw:dots1-length", QStringLiteral("400%")) + attrOf("draw:dots2", QStringLiteral("1")) + attrOf("draw:dots2-length", QStringLiteral("100%"));
        break;
    case Stroke::LongDash:
        dots = attrOf("draw:dots1", QStringLiteral("1")) + attrOf("draw:dots1-length", QStringLiteral("800%"));
        break;
    case Stroke::LongDashDot:
        dots = attrOf("draw:dots1", QStringLiteral("1")) + attrOf("draw:dots1-length", QStringLiteral("800%")) + attrOf("draw:dots2", QStringLiteral("1")) + attrOf("draw:dots2-length", QStringLiteral("100%"));
        break;
    case Stroke::LongDashDotDot:
        dots = attrOf("draw:dots1", QStringLiteral("1")) + attrOf("draw:dots1-length", QStringLiteral("800%")) + attrOf("draw:dots2", QStringLiteral("2")) + attrOf("draw:dots2-length", QStringLiteral("100%"));
        break;
    case Stroke::SolidLine: return QString();
    }
    return m_common.add(QStringLiteral("Dash_"), QStringLiteral("<draw:stroke-dash draw:name=\""), QStringLiteral("\"") + attrOf("draw:style", style) + dots + attrOf("draw:distance", distance) + QStringLiteral("/>"));
}

QString Exporter::namedMarker(Arrow arrow)
{
    // The names tell what the arrow is, as they do in the files of the other suite.
    QString name, display, box, path;
    switch (arrow) {
    case Arrow::Triangle: name = QStringLiteral("Arrow"); display = QStringLiteral("Arrow"); box = QStringLiteral("0 0 20 30"); path = QStringLiteral("m10 0-10 30h20z"); break;
    case Arrow::Open: name = QStringLiteral("Line_20_Arrow"); display = QStringLiteral("Line Arrow"); box = QStringLiteral("0 0 20 30"); path = QStringLiteral("m10 0 10 30h-3.5l-6.5-22-6.5 22h-3.5z"); break;
    case Arrow::Stealth: name = QStringLiteral("Concave"); display = QStringLiteral("Concave"); box = QStringLiteral("0 0 20 30"); path = QStringLiteral("m10 0-10 30 10-8 10 8z"); break;
    case Arrow::Diamond: name = QStringLiteral("Diamond"); display = QStringLiteral("Diamond"); box = QStringLiteral("0 0 20 30"); path = QStringLiteral("m10 0-10 15 10 15 10-15z"); break;
    case Arrow::Oval: name = QStringLiteral("Circle"); display = QStringLiteral("Circle"); box = QStringLiteral("0 0 20 20"); path = QStringLiteral("M10 0A10 10 0 1 1 10 20 10 10 0 1 1 10 0z"); break;
    case Arrow::None: return QString();
    }
    // Added once under its own name: the registry names what it adds by count, so the name goes in the XML.
    m_common.addNamed(name, QStringLiteral("<draw:marker draw:name=\""), QStringLiteral("\" draw:display-name=\"") + display + QStringLiteral("\" svg:viewBox=\"") + box + QStringLiteral("\" svg:d=\"") + path + QStringLiteral("\"/>"));
    return name;
}

QString Exporter::namedHatch(const Fill &fill)
{
    // The lines of the pattern in the library, as the nearest hatch.
    const Qt::BrushStyle brush = patternBrushes().value(fill.pattern, Qt::Dense4Pattern);
    QString style = QStringLiteral("single");
    int rotation = 0;
    switch (brush) {
    case Qt::VerPattern: rotation = 90; break;
    case Qt::BDiagPattern: rotation = 45; break;
    case Qt::FDiagPattern: rotation = 135; break;
    case Qt::CrossPattern: style = QStringLiteral("double"); break;
    case Qt::DiagCrossPattern: style = QStringLiteral("double"); rotation = 45; break;
    default: break;
    }
    const QColor lines = resolve(fill.color);
    return m_common.add(QStringLiteral("Hatch"), QStringLiteral("<draw:hatch draw:name=\""),
                        QStringLiteral("\"") + attrOf("draw:style", style) + attrOf("draw:color", hexOf(lines.isValid() ? lines : QColor(Qt::black))) + attrOf("draw:distance", QStringLiteral("0.1cm"))
                            + attrOf("draw:rotation", QString::number(rotation * 10)) + QStringLiteral("/>"));
}

QString Exporter::namedBitmap(const Fill &fill)
{
    const QString part = addPictureChanged(fill.imageId, fill.blipEffects);
    if (part.isEmpty()) return QString();
    return m_common.add(QStringLiteral("Bitmap"), QStringLiteral("<draw:fill-image draw:name=\""),
                        QStringLiteral("\"") + attrOf("xlink:href", part) + QStringLiteral(" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\"/>"));
}

// ---- the attributes

QString Exporter::fillAttrs(const Fill &fill, const QSizeF &)
{
    const QString none = QStringLiteral(" draw:fill=\"none\"");
    switch (fill.type) {
    case Fill::NoFill: return none;
    case Fill::Solid: {
        const QColor c = resolve(fill.color);
        if (!c.isValid()) return none;
        QString out = QStringLiteral(" draw:fill=\"solid\"") + attrOf("draw:fill-color", hexOf(c));
        if (fill.transparency > 0.001) out += attrOf("draw:opacity", opacityOf(fill.transparency));
        return out;
    }
    case Fill::Gradient: {
        QString opacity;
        const QString name = namedGradient(fill, &opacity);
        QString out = QStringLiteral(" draw:fill=\"gradient\"") + attrOf("draw:fill-gradient-name", name);
        if (!opacity.isEmpty()) {
            out += attrOf("draw:opacity-name", opacity);
        } else {
            double t = fill.stops.isEmpty() ? fill.transparency : fill.stops.first().transparency;
            if (t > 0.001) out += attrOf("draw:opacity", opacityOf(t));
        }
        return out;
    }
    case Fill::Picture:
    case Fill::Texture: {
        const QString name = namedBitmap(fill);
        if (name.isEmpty()) return none;
        QString out = QStringLiteral(" draw:fill=\"bitmap\"") + attrOf("draw:fill-image-name", name);
        if (fill.type == Fill::Texture || fill.tile) {
            const QSize px = m_doc.imageSize(fill.imageId);
            const double scale = fill.tileScale > 0 ? fill.tileScale : 1.0;
            out += attrOf("style:repeat", QStringLiteral("repeat")) + attrOf("draw:fill-image-ref-point", QStringLiteral("top-left"));
            if (px.width() > 0 && px.height() > 0)
                out += attrOf("draw:fill-image-width", cm(px.width() * 0.75 * scale)) + attrOf("draw:fill-image-height", cm(px.height() * 0.75 * scale));
        } else {
            out += attrOf("style:repeat", QStringLiteral("stretch"));
        }
        if (fill.transparency > 0.001) out += attrOf("draw:opacity", opacityOf(fill.transparency));
        return out;
    }
    case Fill::Pattern: {
        const Qt::BrushStyle brush = patternBrushes().value(fill.pattern, Qt::Dense4Pattern);
        const QColor front = resolve(fill.color), back = resolve(fill.color2);
        if (brush >= Qt::Dense1Pattern && brush <= Qt::Dense7Pattern) {
            // A share of the ground covered: no hatch has it, so the colors are mixed.
            static const double cover[7] = {0.94, 0.87, 0.75, 0.5, 0.25, 0.12, 0.06};
            const QColor on = front.isValid() ? front : QColor(Qt::black), under = back.isValid() ? back : QColor(Qt::white);
            const QColor c = mix(under, on, cover[brush - Qt::Dense1Pattern]);
            QString out = QStringLiteral(" draw:fill=\"solid\"") + attrOf("draw:fill-color", hexOf(c));
            if (fill.transparency > 0.001) out += attrOf("draw:opacity", opacityOf(fill.transparency));
            return out;
        }
        QString out = QStringLiteral(" draw:fill=\"hatch\"") + attrOf("draw:fill-hatch-name", namedHatch(fill));
        if (back.isValid()) out += attrOf("draw:fill-hatch-solid", QStringLiteral("true")) + attrOf("draw:fill-color", hexOf(back));
        else out += attrOf("draw:fill-hatch-solid", QStringLiteral("false"));
        if (fill.transparency > 0.001) out += attrOf("draw:opacity", opacityOf(fill.transparency));
        return out;
    }
    }
    return none;
}

QString Exporter::strokeAttrs(const Stroke &s)
{
    const QColor color = resolve(s.color);
    if (s.isNone() || !color.isValid()) return QStringLiteral(" draw:stroke=\"none\"");
    QString out;
    out += attrOf("draw:stroke", s.dash == Stroke::SolidLine ? QStringLiteral("solid") : QStringLiteral("dash"));
    if (s.dash != Stroke::SolidLine) out += attrOf("draw:stroke-dash", namedDash(s));
    out += attrOf("svg:stroke-width", cm(s.width)) + attrOf("svg:stroke-color", hexOf(color));
    if (s.transparency > 0.001) out += attrOf("svg:stroke-opacity", opacityOf(s.transparency));
    out += attrOf("draw:stroke-linejoin", s.join == Qt::RoundJoin ? QStringLiteral("round") : s.join == Qt::BevelJoin ? QStringLiteral("bevel") : QStringLiteral("miter"));
    out += attrOf("svg:stroke-linecap", s.cap == Qt::RoundCap ? QStringLiteral("round") : s.cap == Qt::SquareCap ? QStringLiteral("square") : QStringLiteral("butt"));
    // An arrow is as wide as its size says in widths of the line (the reader counts a line thinner than a point as a point wide).
    static const double widths[3] = {2.0, 4.0, 7.0};
    const double unit = std::max(s.width, 1.0);
    if (s.startArrow != Arrow::None)
        out += attrOf("draw:marker-start", namedMarker(s.startArrow)) + attrOf("draw:marker-start-width", cm(unit * widths[std::clamp(s.startSize, 0, 2)])) + attrOf("draw:marker-start-center", QStringLiteral("false"));
    if (s.endArrow != Arrow::None)
        out += attrOf("draw:marker-end", namedMarker(s.endArrow)) + attrOf("draw:marker-end-width", cm(unit * widths[std::clamp(s.endSize, 0, 2)])) + attrOf("draw:marker-end-center", QStringLiteral("false"));
    return out;
}

QString Exporter::effectAttrs(const Effects &fx)
{
    QString out;
    if (fx.shadow.on && !fx.shadow.inner) {
        const double a = qDegreesToRadians(fx.shadow.angle);
        const QColor c = resolve(fx.shadow.color);
        out += QStringLiteral(" draw:shadow=\"visible\"") + attrOf("draw:shadow-offset-x", cm(fx.shadow.distance * std::cos(a))) + attrOf("draw:shadow-offset-y", cm(fx.shadow.distance * std::sin(a)))
               + attrOf("draw:shadow-color", hexOf(c.isValid() ? c : QColor(Qt::black))) + attrOf("draw:shadow-opacity", opacityOf(fx.shadow.transparency));
        if (fx.shadow.blur > 0) out += attrOf("draw:shadow-blur", cm(fx.shadow.blur));
    } else {
        out += QStringLiteral(" draw:shadow=\"hidden\"");
        if (fx.shadow.on) sayOnce(QStringLiteral("innershadow"), tr("Inner shadows can't be saved in this kind of file."));
    }
    if (fx.glow.on) {
        const QColor c = resolve(fx.glow.color);
        out += attrOf("loext:glow-radius", cm(fx.glow.size)) + attrOf("loext:glow-color", hexOf(c.isValid() ? c : QColor(Qt::black))) + attrOf("loext:glow-transparency", percent(fx.glow.transparency));
    }
    if (fx.softEdge > 0) out += attrOf("loext:softedge-radius", cm(fx.softEdge));
    if (fx.reflection.on) sayOnce(QStringLiteral("reflection"), tr("Reflections can't be saved in this kind of file."));
    if (fx.bevel.type || fx.rot3d.x || fx.rot3d.y) sayOnce(QStringLiteral("bevel"), tr("Bevels and objects turned in space are saved flat."));
    return out;
}

// ---- dates and times

QString Exporter::dateStyle(Registry &reg, const QString &code, bool *isTime)
{
    const QString kind = code.section(QLatin1Char(':'), 0, 0);
    QString format = code.section(QLatin1Char(':'), 1);
    if (format.isEmpty()) format = kind == QLatin1String("time") ? QStringLiteral("h:mm AP") : kind == QLatin1String("datetime") ? QStringLiteral("M/d/yyyy h:mm AP") : QStringLiteral("M/d/yyyy");
    if (isTime) *isTime = kind == QLatin1String("time");
    QString body, text;
    auto flush = [&] {
        if (!text.isEmpty()) body += QStringLiteral("<number:text>") + esc(text) + QStringLiteral("</number:text>");
        text.clear();
    };
    for (int i = 0; i < format.size();) {
        const QChar c = format[i];
        int n = 1;
        while (i + n < format.size() && format[i + n] == c) ++n;
        const bool longForm = n >= 2;
        if (c == QLatin1Char('\'')) {
            const int end = int(format.indexOf(QLatin1Char('\''), i + 1));
            text += format.mid(i + 1, end < 0 ? -1 : end - i - 1);
            i = end < 0 ? int(format.size()) : end + 1;
            continue;
        }
        QString piece;
        if (c == QLatin1Char('d')) piece = n >= 3 ? QStringLiteral("<number:day-of-week number:style=\"%1\"/>").arg(n >= 4 ? QStringLiteral("long") : QStringLiteral("short"))
                                                  : QStringLiteral("<number:day number:style=\"%1\"/>").arg(longForm ? QStringLiteral("long") : QStringLiteral("short"));
        else if (c == QLatin1Char('M')) piece = QStringLiteral("<number:month number:style=\"%1\"%2/>").arg(n == 2 || n == 4 ? QStringLiteral("long") : QStringLiteral("short"), n >= 3 ? QStringLiteral(" number:textual=\"true\"") : QString());
        else if (c == QLatin1Char('y')) piece = QStringLiteral("<number:year number:style=\"%1\"/>").arg(n >= 4 ? QStringLiteral("long") : QStringLiteral("short"));
        else if (c == QLatin1Char('h') || c == QLatin1Char('H')) piece = QStringLiteral("<number:hours number:style=\"%1\"/>").arg(longForm ? QStringLiteral("long") : QStringLiteral("short"));
        else if (c == QLatin1Char('m')) piece = QStringLiteral("<number:minutes number:style=\"%1\"/>").arg(longForm ? QStringLiteral("long") : QStringLiteral("short"));
        else if (c == QLatin1Char('s')) piece = QStringLiteral("<number:seconds number:style=\"%1\"/>").arg(longForm ? QStringLiteral("long") : QStringLiteral("short"));
        else if ((c == QLatin1Char('A') || c == QLatin1Char('a')) && i + 1 < format.size() && (format[i + 1] == QLatin1Char('P') || format[i + 1] == QLatin1Char('p'))) {
            piece = QStringLiteral("<number:am-pm/>");
            n = 2;
        }
        if (piece.isEmpty()) {
            text += format.mid(i, n);
        } else {
            flush();
            body += piece;
        }
        i += n;
    }
    flush();
    const QString tag = kind == QLatin1String("time") ? QStringLiteral("number:time-style") : QStringLiteral("number:date-style");
    return reg.add(QStringLiteral("N"), QLatin1Char('<') + tag + QStringLiteral(" style:name=\""), QStringLiteral("\" number:automatic-order=\"false\">") + body + QStringLiteral("</") + tag + QLatin1Char('>'));
}

} // namespace jp::deck::odpw
