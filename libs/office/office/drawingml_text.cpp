// Text bodies (drawingml.h): paragraphs, runs, line breaks, and fields read
// into a story as the text engine takes them, every property resolved
// through the list styles to a concrete format (the engine never sees the
// stack).

#include "office/drawingml.h"

#include "core/color.h"
#include "core/texteffects.h"
#include "office/drawingml_props.h"
#include "office/xml.h"
#include "text/textprops.h"

#include <QJsonDocument>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <QTextOption>

#include <algorithm>
#include <cmath>
#include <optional>

namespace jp::office {

namespace {

// A stranger's file: no more than this many paragraphs, runs, or letters in a body. The most of any body in the sample decks are 26 paragraphs, 78 runs,
// and 1,258 letters; these are 385, 256, and 1,590 times that. (All the bodies of an import together are held to its DrawingBudget as well.)
constexpr int kMaxParagraphs = 10000, kMaxRuns = 20000;
constexpr qsizetype kMaxText = 2000000;
constexpr int kLevels = 9;
// Qt adds a block to a list in time proportional to the list's length (every block of the list is marked changed), so a run of one bullet or
// numbering longer than this goes on in another list of the same kind, numbered on from where the one before ended. The longest list in the
// sample decks is 26 paragraphs; with this many, 10,000 bulleted paragraphs are read in a fifth of a second.
constexpr int kMaxListRun = 500;

QDomElement firstChild(const QDomElement &e, QLatin1String local)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == local) return c;
    return QDomElement();
}

QString at(const QDomElement &e, QLatin1String name)
{
    return attr(e, QString(name));
}

bool intOf(const QDomElement &e, QLatin1String name, qint64 *out)
{
    const QString s = at(e, name);
    if (s.isEmpty()) return false;
    bool ok = false;
    const qint64 v = parseInt(s, 0, &ok);
    if (ok) *out = v;
    return ok;
}

bool boolOf(const QDomElement &e, QLatin1String name, bool *out)
{
    const QString s = at(e, name);
    if (s.isEmpty()) return false;
    bool ok = false;
    const bool v = parseBool(s, false, &ok);
    if (ok) *out = v;
    return ok;
}

// A length attribute, in points.
bool lengthOf(const QDomElement &e, QLatin1String name, double *out)
{
    const QString s = at(e, name);
    if (s.isEmpty()) return false;
    bool ok = false;
    const double v = parseLength(s, 0, &ok);
    if (!ok || !std::isfinite(v)) return false;
    *out = v;
    return true;
}

// A theme font reference in a typeface ("+mj-lt", "+mn-ea"): "major", "minor", or empty.
QString themeFontOf(const QString &typeface)
{
    if (typeface.startsWith(QLatin1String("+mj"))) return QStringLiteral("major");
    if (typeface.startsWith(QLatin1String("+mn"))) return QStringLiteral("minor");
    return QString();
}

// ---- what the elements say

// A distance that is points or a share of the line.
struct Spacing {
    enum Kind { Unset, Points, Percent } kind = Unset;
    double value = 0;   // points, or a fraction (1 is 100%)
};

// The bullet of a paragraph. Its mark, color, size, and font are each said
// (or inherited) by themselves.
struct Bullet {
    enum Kind { Unset, None, Char, AutoNum, Blip } kind = Unset;
    QString text;               // buChar's character, or buAutoNum's scheme
    int startAt = 1;
    enum Follow { NotSaid, Text, Own };
    Follow color = NotSaid, size = NotSaid, font = NotSaid;
    QColor colorValue;
    Spacing sizeValue;
    QString fontValue;
};

struct ParaProps {
    QString algn;
    std::optional<double> marL, marR, indent, defTab;
    std::optional<bool> rtl;
    Spacing lnSpc, spcBef, spcAft;
    Bullet bullet;
    bool hasTabs = false;
    QList<QTextOption::Tab> tabs;
};

struct RunProps {
    std::optional<double> size;       // points
    std::optional<bool> bold, italic, noProof;
    QString underline, strike, cap;   // as written; empty for unsaid
    std::optional<double> baseline;   // a fraction of the size; above zero is a superscript
    std::optional<double> spacing;    // points added after each letter
    std::optional<double> kern;       // the size, in points, from which pairs are kerned
    QString lang;
    QString latin, ea, cs;            // typefaces as written
    enum Fill { FillUnset, FillNone, FillColor } fill = FillUnset;
    QColor color, highlight;
    std::optional<QString> fx;        // a:effectLst as the engine keeps it (tp::TextFx); empty when the run says it has none
    QString textFill;                 // a gradient fill of the letters, as the engine keeps it (tp::TextFill); empty for none
    QString outlineRef;               // an outline of the letters (tp::OutlineRef, tp::OutlineWidth); empty for none
    double outlineWidth = 0;          // points
};

Spacing spacingOf(const QDomElement &e)
{
    Spacing s;
    if (const QDomElement pct = firstChild(e, QLatin1String("spcPct")); !pct.isNull()) {
        bool ok = false;
        const double v = parsePercent(at(pct, QLatin1String("val")), 0, &ok);
        if (ok && std::isfinite(v) && v >= 0) s = {Spacing::Percent, v};
    } else if (const QDomElement pts = firstChild(e, QLatin1String("spcPts")); !pts.isNull()) {
        qint64 v = 0;
        if (intOf(pts, QLatin1String("val"), &v) && v >= 0) s = {Spacing::Points, std::min<qint64>(v, 158400) / 100.0};
    }
    return s;
}

void mergeParagraph(ParaProps &p, const QDomElement &pPr, const DrawingContext &ctx)
{
    if (pPr.isNull()) return;
    if (const QString a = at(pPr, QLatin1String("algn")); !a.isEmpty()) p.algn = a;
    double v = 0;
    if (lengthOf(pPr, QLatin1String("marL"), &v)) p.marL = v;
    if (lengthOf(pPr, QLatin1String("marR"), &v)) p.marR = v;
    if (lengthOf(pPr, QLatin1String("indent"), &v)) p.indent = v;
    if (lengthOf(pPr, QLatin1String("defTabSz"), &v)) p.defTab = v;
    bool b = false;
    if (boolOf(pPr, QLatin1String("rtl"), &b)) p.rtl = b;
    for (QDomElement c = pPr.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        const QString name = c.localName();
        if (name == QLatin1String("lnSpc")) {
            if (const Spacing s = spacingOf(c); s.kind != Spacing::Unset) p.lnSpc = s;
        } else if (name == QLatin1String("spcBef")) {
            if (const Spacing s = spacingOf(c); s.kind != Spacing::Unset) p.spcBef = s;
        } else if (name == QLatin1String("spcAft")) {
            if (const Spacing s = spacingOf(c); s.kind != Spacing::Unset) p.spcAft = s;
        } else if (name == QLatin1String("buClrTx")) {
            p.bullet.color = Bullet::Text;
        } else if (name == QLatin1String("buClr")) {
            const QColor color = readColor(c, ctx.theme, ctx.colors);
            if (color.isValid()) {
                p.bullet.color = Bullet::Own;
                p.bullet.colorValue = color;
            }
        } else if (name == QLatin1String("buSzTx")) {
            p.bullet.size = Bullet::Text;
        } else if (name == QLatin1String("buSzPct")) {
            bool ok = false;
            const double pct = parsePercent(at(c, QLatin1String("val")), 0, &ok);
            if (ok && std::isfinite(pct) && pct > 0) {
                p.bullet.size = Bullet::Own;
                p.bullet.sizeValue = {Spacing::Percent, pct};
            }
        } else if (name == QLatin1String("buSzPts")) {
            qint64 pts = 0;
            if (intOf(c, QLatin1String("val"), &pts) && pts > 0) {
                p.bullet.size = Bullet::Own;
                p.bullet.sizeValue = {Spacing::Points, std::min<qint64>(pts, 400000) / 100.0};
            }
        } else if (name == QLatin1String("buFontTx")) {
            p.bullet.font = Bullet::Text;
        } else if (name == QLatin1String("buFont")) {
            QString face = at(c, QLatin1String("typeface"));
            if (const QString theme = themeFontOf(face); !theme.isEmpty()) face = theme == QLatin1String("major") ? ctx.theme.majorLatin : ctx.theme.minorLatin;
            if (!face.isEmpty()) {
                p.bullet.font = Bullet::Own;
                p.bullet.fontValue = face;
            }
        } else if (name == QLatin1String("buNone")) {
            p.bullet.kind = Bullet::None;
        } else if (name == QLatin1String("buAutoNum")) {
            p.bullet.kind = Bullet::AutoNum;
            p.bullet.text = at(c, QLatin1String("type"));
            qint64 start = 1;
            intOf(c, QLatin1String("startAt"), &start);
            p.bullet.startAt = int(std::clamp<qint64>(start, 1, 32767));
        } else if (name == QLatin1String("buChar")) {
            p.bullet.kind = Bullet::Char;
            p.bullet.text = at(c, QLatin1String("char"));
        } else if (name == QLatin1String("buBlip")) {
            p.bullet.kind = Bullet::Blip;
            p.bullet.text.clear();
        } else if (name == QLatin1String("tabLst")) {
            p.hasTabs = true;
            p.tabs.clear();
            for (QDomElement t = c.firstChildElement(); !t.isNull() && p.tabs.size() < 32; t = t.nextSiblingElement()) {
                double pos = 0;
                if (t.localName() != QLatin1String("tab") || !lengthOf(t, QLatin1String("pos"), &pos) || pos < 0 || pos > 20000) continue;
                const QString algn = at(t, QLatin1String("algn"));
                QTextOption::Tab tab;
                tab.position = pos;
                tab.type = algn == QLatin1String("r") ? QTextOption::RightTab : algn == QLatin1String("ctr") ? QTextOption::CenterTab
                         : algn == QLatin1String("dec") ? QTextOption::DelimiterTab : QTextOption::LeftTab;
                if (tab.type == QTextOption::DelimiterTab) tab.delimiter = QLatin1Char('.');
                p.tabs << tab;
            }
        }
    }
}

// The first stop of a gradient, for text it fills.
QColor firstStopOf(const QDomElement &grad, const DrawingContext &ctx)
{
    const QDomElement stops = firstChild(grad, QLatin1String("gsLst"));
    return readColor(firstChild(stops, QLatin1String("gs")), ctx.theme, ctx.colors);
}

// A shadow's place and look: a:outerShdw, a:innerShdw, or a:prstShdw (a preset's shadow is drawn as an offset one).
TextShadow shadowOf(const QDomElement &e, const DrawingContext &ctx)
{
    TextShadow s;
    s.on = true;
    const QColor color = readColor(e, ctx.theme, ctx.colors);
    s.color = color.isValid() ? color : QColor(Qt::black);
    double v = 0;
    if (lengthOf(e, QLatin1String("blurRad"), &v)) s.blur = std::clamp(v, 0.0, kMaxTextEffectPoints);
    if (lengthOf(e, QLatin1String("dist"), &v)) s.distance = std::clamp(v, 0.0, kMaxTextEffectPoints);
    if (const QString dir = at(e, QLatin1String("dir")); !dir.isEmpty()) s.angle = std::fmod(parseAngle(dir, 0), 360.0);
    bool ok = false;
    if (const QString sx = at(e, QLatin1String("sx")); !sx.isEmpty()) {
        const double x = parsePercent(sx, 1, &ok);
        if (ok && std::isfinite(x)) s.sx = std::clamp(x, -20.0, 20.0);
    }
    if (const QString sy = at(e, QLatin1String("sy")); !sy.isEmpty()) {
        const double y = parsePercent(sy, 1, &ok);
        if (ok && std::isfinite(y)) s.sy = std::clamp(y, -20.0, 20.0);
    }
    if (const QString kx = at(e, QLatin1String("kx")); !kx.isEmpty()) s.kx = std::clamp(std::fmod(parseAngle(kx, 0), 360.0), -89.0, 89.0);
    if (const QString ky = at(e, QLatin1String("ky")); !ky.isEmpty()) s.ky = std::clamp(std::fmod(parseAngle(ky, 0), 360.0), -89.0, 89.0);
    if (const QString al = at(e, QLatin1String("algn")); !al.isEmpty()) s.align = al;
    return s;
}

// An element of the standard's drawing namespace and what it holds as it is written (a: prefix, its attributes, its elements); false for one that
// holds anything else, or goes deeper than an effect list does.
bool keptXml(const QDomElement &e, QString *out, int depth)
{
    if (depth > 6 || out->size() > 4096 || !sameNamespace(e.namespaceURI(), ns::A)) return false;
    *out += QStringLiteral("<a:") + e.localName();
    const QDomNamedNodeMap attrs = e.attributes();
    for (int i = 0; i < attrs.length(); ++i) {
        const QDomAttr a = attrs.item(i).toAttr();
        if (a.nodeName().startsWith(QLatin1String("xmlns"))) continue;
        if (!a.prefix().isEmpty()) return false;
        *out += QLatin1Char(' ') + (a.localName().isEmpty() ? a.nodeName() : a.localName()) + QStringLiteral("=\"") + a.value().toHtmlEscaped().replace(QLatin1Char('"'), QStringLiteral("&quot;")) + QLatin1Char('"');
    }
    if (e.firstChildElement().isNull()) {
        *out += QStringLiteral("/>");
        return true;
    }
    *out += QLatin1Char('>');
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (!keptXml(c, out, depth + 1)) return false;
    *out += QStringLiteral("</a:") + e.localName() + QLatin1Char('>');
    return true;
}

// The elements an effect list may hold, in the schema's order (CT_EffectList): the file's own, each kind once and in that order, and
// none the schema has no place for. False when nothing is left.
bool keptList(const QDomElement &list, QString *out)
{
    static const char *const order[] = {"blur", "fillOverlay", "glow", "innerShdw", "outerShdw", "prstShdw", "reflection", "softEdge"};
    QString parts[8];
    int seen = 0;
    for (QDomElement c = list.firstChildElement(); !c.isNull() && ++seen <= 16; c = c.nextSiblingElement())
        for (int k = 0; k < 8; ++k) {
            if (c.localName() != QLatin1String(order[k]) || !parts[k].isEmpty()) continue;
            QString one;
            if (keptXml(c, &one, 1)) parts[k] = one;
        }
    QString body;
    for (const QString &part : parts) body += part;
    if (body.isEmpty()) return false;
    *out = QStringLiteral("<a:effectLst>") + body + QStringLiteral("</a:effectLst>");
    return true;
}

// What a run's a:effectLst says, in the form a run's format carries it: a string, empty for a list with no effect in it. Its own
// elements are kept too (up to a few KB), to be written back as they were.
QString textEffectsOf(const QDomElement &list, const DrawingContext &ctx)
{
    TextEffects fx;
    // A list has at most eight kinds of effect (the schema's); a file that lists more is not read past the first sixteen.
    int seen = 0;
    for (QDomElement c = list.firstChildElement(); !c.isNull() && ++seen <= 16; c = c.nextSiblingElement()) {
        const QString name = c.localName();
        double v = 0;
        if (name == QLatin1String("outerShdw") && !fx.outer.on) {
            fx.outer = shadowOf(c, ctx);
        } else if (name == QLatin1String("prstShdw") && !fx.outer.on) {
            fx.outer = shadowOf(c, ctx);
            fx.outer.blur = 0;
            fx.outer.sx = fx.outer.sy = 1;
            fx.outer.kx = fx.outer.ky = 0;
        } else if (name == QLatin1String("innerShdw") && !fx.inner.on) {
            fx.inner = shadowOf(c, ctx);
        } else if (name == QLatin1String("glow") && !fx.glow.on) {
            fx.glow.on = true;
            const QColor color = readColor(c, ctx.theme, ctx.colors);
            fx.glow.color = color.isValid() ? color : QColor(Qt::black);
            if (lengthOf(c, QLatin1String("rad"), &v)) fx.glow.radius = std::clamp(v, 0.0, kMaxTextEffectPoints);
        } else if (name == QLatin1String("softEdge")) {
            if (lengthOf(c, QLatin1String("rad"), &v)) fx.softEdge = std::clamp(v, 0.0, kMaxTextEffectPoints);
        } else if (name == QLatin1String("reflection") && !fx.reflection.on) {
            TextReflection &r = fx.reflection;
            r.on = true;
            bool ok = false;
            auto percent = [&](const char *attrName, double fallback) {
                const QString s = at(c, QLatin1String(attrName));
                const double x = s.isEmpty() ? fallback : parsePercent(s, fallback, &ok);
                return std::isfinite(x) ? x : fallback;
            };
            auto angleOf = [&](const char *attrName, double fallback) {
                const QString s = at(c, QLatin1String(attrName));
                return s.isEmpty() ? fallback : std::fmod(parseAngle(s, fallback), 360.0);
            };
            if (lengthOf(c, QLatin1String("blurRad"), &v)) r.blur = std::clamp(v, 0.0, kMaxTextEffectPoints);
            if (lengthOf(c, QLatin1String("dist"), &v)) r.distance = std::clamp(v, 0.0, kMaxTextEffectPoints);
            r.startAlpha = std::clamp(percent("stA", 1), 0.0, 1.0);
            r.endAlpha = std::clamp(percent("endA", 0), 0.0, 1.0);
            r.startPos = std::clamp(percent("stPos", 0), 0.0, 1.0);
            r.endPos = std::clamp(percent("endPos", 1), 0.0, 1.0);
            r.sx = std::clamp(percent("sx", 1), -20.0, 20.0);
            r.sy = std::clamp(percent("sy", 1), -20.0, 20.0);
            r.angle = angleOf("dir", 0);
            r.fadeAngle = angleOf("fadeDir", 90);
            r.kx = std::clamp(angleOf("kx", 0), -89.0, 89.0);
            r.ky = std::clamp(angleOf("ky", 0), -89.0, 89.0);
            if (const QString al = at(c, QLatin1String("algn")); !al.isEmpty()) r.align = al;
        }
    }
    // As the file had it (the list's own elements, in the schema's order, up to a few KB): a list of effects not drawn here (blur, fill
    // overlay) is kept too; a list with nothing in it says there are none, which is a list of its own (a run's takes its style's off).
    constexpr int kMaxKept = 4096;
    QString xml;
    if (keptList(list, &xml) && xml.size() <= kMaxKept) fx.xml = xml;
    if (!fx.any() && fx.xml.isEmpty()) return TextEffects::noneJson();
    return fx.toJson();
}

void mergeRun(RunProps &r, const QDomElement &rPr, const DrawingContext &ctx)
{
    if (rPr.isNull()) return;
    qint64 n = 0;
    bool b = false;
    if (intOf(rPr, QLatin1String("sz"), &n)) r.size = std::clamp(n / 100.0, 1.0, 4000.0);
    if (boolOf(rPr, QLatin1String("b"), &b)) r.bold = b;
    if (boolOf(rPr, QLatin1String("i"), &b)) r.italic = b;
    if (boolOf(rPr, QLatin1String("noProof"), &b)) r.noProof = b;
    if (const QString u = at(rPr, QLatin1String("u")); !u.isEmpty()) r.underline = u;
    if (const QString s = at(rPr, QLatin1String("strike")); !s.isEmpty()) r.strike = s;
    if (const QString c = at(rPr, QLatin1String("cap")); !c.isEmpty()) r.cap = c;
    if (const QString l = at(rPr, QLatin1String("lang")); !l.isEmpty()) r.lang = l;
    if (intOf(rPr, QLatin1String("spc"), &n)) r.spacing = std::clamp(n / 100.0, -1584.0, 1584.0);
    if (intOf(rPr, QLatin1String("kern"), &n) && n >= 0) r.kern = std::min(n / 100.0, 4000.0);
    if (const QString s = at(rPr, QLatin1String("baseline")); !s.isEmpty()) {
        bool ok = false;
        const double v = parsePercent(s, 0, &ok);
        if (ok && std::isfinite(v)) r.baseline = v;
    }
    for (QDomElement c = rPr.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        const QString name = c.localName();
        if (name == QLatin1String("solidFill") || name == QLatin1String("gradFill")) {
            const QColor color = name == QLatin1String("solidFill") ? readColor(c, ctx.theme, ctx.colors) : firstStopOf(c, ctx);
            if (color.isValid()) {
                r.fill = RunProps::FillColor;
                r.color = color;
            }
            // A gradient is the letters' fill as well as its first color (what draws no gradient shows that).
            r.textFill.clear();
            if (name == QLatin1String("gradFill")) {
                const Fill g = readFill(rPr, QDomElement(), ctx);
                if (g.type == Fill::Gradient && g.stops.size() >= 2) r.textFill = QString::fromUtf8(QJsonDocument(g.toJson()).toJson(QJsonDocument::Compact));
            }
        } else if (name == QLatin1String("noFill")) {
            r.fill = RunProps::FillNone;
            r.textFill.clear();
        } else if (name == QLatin1String("ln")) {
            const Stroke s = readLine(rPr, QDomElement(), ctx);
            r.outlineRef = s.isNone() ? QString() : s.color.toString();
            r.outlineWidth = s.isNone() ? 0 : s.width;
        } else if (name == QLatin1String("effectLst")) {
            r.fx = textEffectsOf(c, ctx);
        } else if (name == QLatin1String("highlight")) {
            const QColor color = readColor(c, ctx.theme, ctx.colors);
            if (color.isValid()) r.highlight = color;
        } else if (name == QLatin1String("latin")) {
            if (const QString face = at(c, QLatin1String("typeface")); !face.isEmpty()) r.latin = face;
        } else if (name == QLatin1String("ea")) {
            if (const QString face = at(c, QLatin1String("typeface")); !face.isEmpty()) r.ea = face;
        } else if (name == QLatin1String("cs")) {
            if (const QString face = at(c, QLatin1String("typeface")); !face.isEmpty()) r.cs = face;
        }
    }
}

// ---- the engine's formats

QString colorRefOf(const QColor &c)
{
    return ColorRef::rgb(c).toString();
}

QTextCharFormat charFormatOf(const RunProps &r)
{
    QTextCharFormat cf;
    cf.setFontPointSize(r.size.value_or(18.0));
    if (r.bold.value_or(false)) cf.setFontWeight(QFont::Bold);
    if (r.italic.value_or(false)) cf.setFontItalic(true);

    // The underline's line is Qt's style; double, words only, and heavy are
    // the engine's own kinds beside it.
    if (!r.underline.isEmpty() && r.underline != QLatin1String("none")) {
        QString u = r.underline;
        int kind = 0;
        if (u == QLatin1String("words")) {
            kind = 2;
            u = QStringLiteral("sng");
        } else if (u == QLatin1String("dbl")) {
            kind = 1;
            u = QStringLiteral("sng");
        } else if (u == QLatin1String("wavyDbl")) {
            kind = 1;
            u = QStringLiteral("wavy");
        } else if (u.endsWith(QLatin1String("Heavy"))) {
            kind = 4;
            u.chop(5);
        } else if (u == QLatin1String("heavy")) {
            kind = 4;
            u = QStringLiteral("sng");
        }
        QTextCharFormat::UnderlineStyle style = QTextCharFormat::NoUnderline;
        if (u == QLatin1String("sng")) style = QTextCharFormat::SingleUnderline;
        else if (u == QLatin1String("dotted")) style = QTextCharFormat::DotLine;
        else if (u == QLatin1String("dash") || u == QLatin1String("dashLong")) style = QTextCharFormat::DashUnderline;
        else if (u == QLatin1String("dotDash")) style = QTextCharFormat::DashDotLine;
        else if (u == QLatin1String("dotDotDash")) style = QTextCharFormat::DashDotDotLine;
        else if (u == QLatin1String("wavy")) style = QTextCharFormat::WaveUnderline;
        if (style != QTextCharFormat::NoUnderline) {
            cf.setUnderlineStyle(style);
            if (kind) cf.setProperty(tp::UnderlineKind, kind);
        }
    }
    if (r.strike == QLatin1String("sngStrike") || r.strike == QLatin1String("dblStrike")) cf.setFontStrikeOut(true);
    if (r.cap == QLatin1String("all")) cf.setFontCapitalization(QFont::AllUppercase);
    else if (r.cap == QLatin1String("small")) cf.setFontCapitalization(QFont::SmallCaps);
    if (r.baseline && *r.baseline > 0) cf.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    else if (r.baseline && *r.baseline < 0) cf.setVerticalAlignment(QTextCharFormat::AlignSubScript);
    // Spacing is points after each letter, which is the engine's kerning
    // (tracking is a share of the font's average letter).
    if (r.spacing && *r.spacing != 0) {
        cf.setFontLetterSpacingType(QFont::AbsoluteSpacing);
        cf.setFontLetterSpacing(*r.spacing);
    }
    if (r.kern) cf.setProperty(tp::KernAbove, *r.kern);
    if (r.fx && !r.fx->isEmpty()) cf.setProperty(tp::TextFx, *r.fx);
    if (!r.textFill.isEmpty()) cf.setProperty(tp::TextFill, r.textFill);
    if (!r.outlineRef.isEmpty()) {
        cf.setProperty(tp::OutlineRef, r.outlineRef);
        cf.setProperty(tp::OutlineWidth, r.outlineWidth);
    }

    // A theme font names no family (the engine takes the story's font
    // scheme); a family may be followed by the fonts of other scripts.
    QString theme = themeFontOf(r.latin);
    if (!r.latin.isEmpty() && theme.isEmpty()) {
        QStringList families{r.latin};
        for (const QString &face : {r.ea, r.cs})
            if (!face.isEmpty() && themeFontOf(face).isEmpty() && !families.contains(face)) families << face;
        cf.setFontFamilies(families);
    } else if (r.latin.isEmpty()) {
        theme = !themeFontOf(r.ea).isEmpty() ? themeFontOf(r.ea) : themeFontOf(r.cs);
    }
    if (!theme.isEmpty()) cf.setProperty(tp::ThemeFont, theme);

    if (r.fill == RunProps::FillColor || r.fill == RunProps::FillNone) {
        const QColor c = r.fill == RunProps::FillNone ? QColor(0, 0, 0, 0) : r.color;
        cf.setForeground(QBrush(c));
        cf.setProperty(tp::ColorRefP, colorRefOf(c));
    }
    if (r.highlight.isValid()) cf.setProperty(tp::HighlightRefP, colorRefOf(r.highlight));
    if (!r.lang.isEmpty()) cf.setProperty(tp::Language, r.lang);
    if (r.noProof.value_or(false)) cf.setProperty(tp::NoProof, true);
    return cf;
}

// ---- numbering

struct Scheme {
    QTextListFormat::Style style = QTextListFormat::ListDecimal;
    QString prefix, suffix;
    int numberFormat = 0;   // tp::NumberFormat, 0 for none of its seven
};

// A numbering scheme's name (ST_TextAutonumberScheme). Qt counts in Latin
// digits, letters, and Roman numerals; the schemes of other scripts count in
// digits (or letters) with their own punctuation.
Scheme schemeOf(const QString &type)
{
    Scheme s;
    if (type.startsWith(QLatin1String("alphaLc")) || type.startsWith(QLatin1String("thaiAlpha")) || type.startsWith(QLatin1String("hindiAlpha")))
        s.style = QTextListFormat::ListLowerAlpha;
    else if (type.startsWith(QLatin1String("alphaUc"))) s.style = QTextListFormat::ListUpperAlpha;
    else if (type.startsWith(QLatin1String("romanLc"))) s.style = QTextListFormat::ListLowerRoman;
    else if (type.startsWith(QLatin1String("romanUc"))) s.style = QTextListFormat::ListUpperRoman;
    if (type.endsWith(QLatin1String("ParenBoth"))) {
        s.prefix = QStringLiteral("(");
        s.suffix = QStringLiteral(")");
    } else if (type.endsWith(QLatin1String("ParenR"))) {
        s.suffix = QStringLiteral(")");
    } else if (type.endsWith(QLatin1String("Period"))) {
        s.suffix = QStringLiteral(".");
    } else if (!type.endsWith(QLatin1String("Plain")) && !type.endsWith(QLatin1String("Minus"))) {
        s = Scheme();   // one this reading doesn't know counts the usual way
        s.suffix = QStringLiteral(".");
    }
    const bool dotted = s.prefix.isEmpty() && s.suffix == QLatin1String(".");
    switch (s.style) {
    case QTextListFormat::ListDecimal: s.numberFormat = dotted ? 1 : s.suffix == QLatin1String(")") && s.prefix.isEmpty() ? 6 : s.prefix == QLatin1String("(") && s.suffix == QLatin1String(")") ? 7 : 0; break;
    case QTextListFormat::ListLowerAlpha: s.numberFormat = dotted ? 2 : 0; break;
    case QTextListFormat::ListUpperAlpha: s.numberFormat = dotted ? 3 : 0; break;
    case QTextListFormat::ListLowerRoman: s.numberFormat = dotted ? 4 : 0; break;
    case QTextListFormat::ListUpperRoman: s.numberFormat = dotted ? 5 : 0; break;
    default: break;
    }
    return s;
}


// ---- fields

// The engine's field code for a field's type; empty for the ones that are
// only the text they last showed. The formats are the usual US ones for the
// standard's thirteen date and time layouts.
QString fieldCodeOf(const QString &type)
{
    if (type == QLatin1String("slidenum")) return QStringLiteral("page");
    static const struct { QLatin1String type, code; } dates[] = {
        {QLatin1String("datetime"), QLatin1String("date:M/d/yyyy")},
        {QLatin1String("datetime1"), QLatin1String("date:M/d/yyyy")},
        {QLatin1String("datetime2"), QLatin1String("date:dddd, MMMM d, yyyy")},
        {QLatin1String("datetime3"), QLatin1String("date:d MMMM yyyy")},
        {QLatin1String("datetime4"), QLatin1String("date:MMMM d, yyyy")},
        {QLatin1String("datetime5"), QLatin1String("date:d-MMM-yy")},
        {QLatin1String("datetime6"), QLatin1String("date:MMMM yy")},
        {QLatin1String("datetime7"), QLatin1String("date:MMM-yy")},
        {QLatin1String("datetime8"), QLatin1String("datetime:M/d/yyyy h:mm AP")},
        {QLatin1String("datetime9"), QLatin1String("datetime:M/d/yyyy h:mm:ss AP")},
        {QLatin1String("datetime10"), QLatin1String("time:HH:mm")},
        {QLatin1String("datetime11"), QLatin1String("time:HH:mm:ss")},
        {QLatin1String("datetime12"), QLatin1String("time:h:mm AP")},
        {QLatin1String("datetime13"), QLatin1String("time:h:mm:ss AP")},
    };
    for (const auto &d : dates)
        if (type == d.type) return QString(d.code);
    return QString();
}


// ---- text

// A run's text as the story keeps it: tabs as they are, any kind of line
// break (including the vertical tab, however the file writes it) the
// engine's line separator, and no control characters or object marks.
QString cleanText(const QString &s)
{
    QString out;
    out.reserve(s.size());
    const QChar lineSeparator(QChar::LineSeparator);
    for (qsizetype i = 0; i < s.size(); ++i) {
        const QChar ch = s[i];
        const ushort u = ch.unicode();
        if (u == '_' && i + 7 <= s.size() && s[i + 1] == QLatin1Char('x') && s[i + 2] == QLatin1Char('0') && s[i + 3] == QLatin1Char('0') && s[i + 6] == QLatin1Char('_')) {
            // The escape of a control character in XML text (_x000B_).
            bool ok = false;
            const int code = s.mid(i + 4, 2).toInt(&ok, 16);
            if (ok && (code == 0x09 || code == 0x0A || code == 0x0B || code == 0x0D)) {
                out += code == 0x09 ? QChar(QLatin1Char('\t')) : lineSeparator;
                i += 6;
                continue;
            }
        }
        if (u == '\r' && i + 1 < s.size() && s[i + 1] == QLatin1Char('\n')) continue;
        if (u == '\n' || u == '\r' || u == 0x0B || u == QChar::ParagraphSeparator) out += lineSeparator;
        else if (u == QChar::ObjectReplacementCharacter) continue;
        else if (u < 0x20 && u != '\t') continue;
        else out += ch;
    }
    return out;
}

// ---- the reading

struct Piece {
    QString text;
    QTextCharFormat format;
};

class Reader {
public:
    Reader(QTextDocument *doc, const DrawingContext &ctx, TextStyleStack styles)
        : m_ctx(ctx), m_budget(ctx.budget), m_styles(std::move(styles)), m_cursor(doc)
    {
    }

    void paragraph(const QDomElement &p);
    // Whether the body has room for another paragraph (and the import's budget); says so in the budget when it hasn't.
    bool room();
    int paragraphs() const { return m_paragraphs; }

private:
    // What the list styles give a paragraph at `level` and a run in it: each
    // style's defaults, then its level, lowest priority first.
    void stack(int level);
    QTextCharFormat linkFormat(QTextCharFormat cf, const QDomElement &rPr) const;
    void addToList(const ParaProps &para, int level, const QString &key, double firstSize);

    const DrawingContext &m_ctx;
    DrawingBudget *m_budget = nullptr;
    TextStyleStack m_styles;
    QTextCursor m_cursor;
    bool m_first = true;
    int m_runs = 0, m_paragraphs = 0;
    qsizetype m_letters = 0;
    bool m_have[kLevels] = {};
    ParaProps m_para[kLevels];   // what the styles give a paragraph of each level
    RunProps m_run[kLevels];     // and a run in it
    struct ListRun {
        QTextList *list = nullptr;
        QString key;
        int count = 0;           // the blocks in the list
        int start = 1;           // the number the list starts at, for numbering
    };
    ListRun m_lists[kLevels];    // the list each level goes on in
};

void Reader::stack(int level)
{
    if (m_have[level]) return;
    m_have[level] = true;
    const QLatin1String names[kLevels] = {QLatin1String("lvl1pPr"), QLatin1String("lvl2pPr"), QLatin1String("lvl3pPr"), QLatin1String("lvl4pPr"), QLatin1String("lvl5pPr"),
                                          QLatin1String("lvl6pPr"), QLatin1String("lvl7pPr"), QLatin1String("lvl8pPr"), QLatin1String("lvl9pPr")};
    for (const QDomElement &style : std::as_const(m_styles)) {
        for (const QDomElement &pPr : {firstChild(style, QLatin1String("defPPr")), firstChild(style, names[level])}) {
            mergeParagraph(m_para[level], pPr, m_ctx);
            mergeRun(m_run[level], firstChild(pPr, QLatin1String("defRPr")), m_ctx);
        }
    }
}

// A run with a link looks like one, as the presentation program draws it:
// in the theme's link color and underlined, unless the link says the text's
// own color stays. Only a link to an address is read (a jump to a slide has
// none); the slide-jump and action links stay text.
QTextCharFormat Reader::linkFormat(QTextCharFormat cf, const QDomElement &rPr) const
{
    const QDomElement click = firstChild(rPr, QLatin1String("hlinkClick"));
    if (click.isNull() || !m_ctx.package) return cf;
    const QString id = attrNs(click, ns::R, QStringLiteral("id"));
    if (id.isEmpty()) return cf;
    const Relationship rel = m_ctx.package->relationship(m_ctx.part, id);
    if (rel.id.isEmpty() || !rel.external || rel.target.isEmpty()) return cf;
    cf.setAnchor(true);
    cf.setAnchorHref(rel.target);
    bool keepsColor = false;
    for (QDomElement ext = firstChild(click, QLatin1String("extLst")).firstChildElement(); !ext.isNull(); ext = ext.nextSiblingElement())
        for (QDomElement e = ext.firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
            if (e.localName() == QLatin1String("hlinkClr") && at(e, QLatin1String("val")) == QLatin1String("tx")) keepsColor = true;
    if (!keepsColor) {
        const QColor c = m_ctx.theme.colors[m_ctx.colors.hlink];
        if (c.isValid()) {
            cf.setForeground(QBrush(c));
            cf.setProperty(tp::ColorRefP, colorRefOf(c));
        }
    }
    if (cf.underlineStyle() == QTextCharFormat::NoUnderline) cf.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    return cf;
}

bool Reader::room()
{
    if (m_paragraphs >= kMaxParagraphs) {
        if (m_budget) m_budget->longBody = true;
        return false;
    }
    if (!m_budget) return true;
    // Out of runs or letters, what follows has no text to read.
    for (DrawingBudget::Kind kind : {DrawingBudget::Runs, DrawingBudget::Letters})
        if (m_budget->left[kind] <= 0) {
            m_budget->ranOut[kind] = true;
            return false;
        }
    return m_budget->take(DrawingBudget::Paragraphs);
}

void Reader::addToList(const ParaProps &para, int level, const QString &key, double firstSize)
{
    ListRun &run = m_lists[level];
    if (run.list && run.key == key && run.count < kMaxListRun) {
        run.list->add(m_cursor.block());
        ++run.count;
        return;
    }
    const Bullet &b = para.bullet;
    // A run of one numbering past the length of a list goes on in another, from the number after the last.
    const int start = run.list && run.key == key ? std::min(run.start + run.count, 1000000) : b.startAt;
    QTextListFormat lf;
    lf.setIndent(0);
    if (level > 0) lf.setProperty(tp::ListLevel, level);
    if (b.kind == Bullet::AutoNum) {
        const Scheme s = schemeOf(b.text);
        lf.setStyle(s.style);
        lf.setNumberPrefix(s.prefix);
        lf.setNumberSuffix(s.suffix);
        if (start > 1) lf.setStart(start);
        if (s.numberFormat) lf.setProperty(tp::NumberFormat, s.numberFormat);
    } else {
        lf.setStyle(QTextListFormat::ListDisc);
        if (b.kind == Bullet::Char && !b.text.isEmpty()) lf.setProperty(tp::BulletChar, b.text);
    }
    if (b.color == Bullet::Own) lf.setProperty(tp::BulletColor, colorRefOf(b.colorValue));
    if (b.font == Bullet::Own) lf.setProperty(tp::BulletFont, b.fontValue);
    if (b.size == Bullet::Own) {
        const double points = b.sizeValue.kind == Spacing::Percent ? b.sizeValue.value * firstSize : b.sizeValue.value;
        lf.setProperty(tp::BulletSize, std::clamp(points, 1.0, 4000.0));
    }
    run.list = m_cursor.createList(lf);
    run.key = key;
    run.count = 1;
    run.start = start;
}

void Reader::paragraph(const QDomElement &p)
{
    ++m_paragraphs;
    const QDomElement pPr = firstChild(p, QLatin1String("pPr"));
    qint64 lvl = 0;
    intOf(pPr, QLatin1String("lvl"), &lvl);
    const int level = int(std::clamp<qint64>(lvl, 0, kLevels - 1));
    stack(level);
    ParaProps para = m_para[level];
    mergeParagraph(para, pPr, m_ctx);
    const RunProps &base = m_run[level];
    const QTextCharFormat baseFormat = charFormatOf(base);

    // The paragraph's pieces of text, each with its format.
    QVector<Piece> pieces;
    double firstSize = 0, largest = 0;
    bool tabbed = false;
    auto add = [&](QString text, const QTextCharFormat &format) {
        if (text.isEmpty()) return;
        // The body, and the import, keep only so many letters: the rest of a run past them is cut off.
        qint64 room = kMaxText - m_letters;
        if (m_budget) room = std::min(room, m_budget->left[DrawingBudget::Letters]);
        if (text.size() > room) {
            if (m_budget) (room == kMaxText - m_letters ? m_budget->longBody : m_budget->ranOut[DrawingBudget::Letters]) = true;
            text.truncate(qsizetype(std::max<qint64>(room, 0)));
            if (text.isEmpty()) return;
        }
        if (m_budget) m_budget->left[DrawingBudget::Letters] -= text.size();
        m_letters += text.size();
        if (firstSize == 0) firstSize = format.fontPointSize();
        largest = std::max(largest, format.fontPointSize());
        tabbed |= text.contains(QLatin1Char('\t'));
        pieces.append({text, format});
    };
    QDomElement c = p.firstChildElement();
    for (; !c.isNull() && m_runs < kMaxRuns && m_letters < kMaxText; c = c.nextSiblingElement()) {
        const QString name = c.localName();
        const bool isRun = name == QLatin1String("r"), isField = name == QLatin1String("fld"), isBreak = name == QLatin1String("br");
        if (!isRun && !isField && !isBreak) continue;
        if (m_budget && !m_budget->take(DrawingBudget::Runs)) break;
        ++m_runs;
        const QDomElement rPr = firstChild(c, QLatin1String("rPr"));
        QTextCharFormat format = baseFormat;
        if (!rPr.isNull()) {
            RunProps run = base;
            mergeRun(run, rPr, m_ctx);
            format = linkFormat(charFormatOf(run), rPr);
        }
        if (isBreak) {
            add(QString(QChar(QChar::LineSeparator)), format);
            continue;
        }
        const QString text = firstChild(c, QLatin1String("t")).text();
        const QString code = isField ? fieldCodeOf(at(c, QLatin1String("type"))) : QString();
        if (code.isEmpty()) {
            add(cleanText(text), format);
        } else {
            format.setProperty(tp::Field, code);
            add(QString(QChar::ObjectReplacementCharacter), format);
        }
    }
    // What the body's own limits leave out is said in the budget too.
    if (m_budget && !c.isNull() && (m_runs >= kMaxRuns || m_letters >= kMaxText)) m_budget->longBody = true;

    // The mark after the last letter: what an empty paragraph is as tall as.
    RunProps endRun = base;
    mergeRun(endRun, firstChild(p, QLatin1String("endParaRPr")), m_ctx);
    const QTextCharFormat endFormat = charFormatOf(endRun);
    if (pieces.isEmpty()) {
        firstSize = largest = endFormat.fontPointSize();
    }

    QTextBlockFormat bf;
    Qt::Alignment align = Qt::AlignLeft;
    if (para.algn == QLatin1String("ctr")) align = Qt::AlignHCenter;
    else if (para.algn == QLatin1String("r")) align = Qt::AlignRight;
    else if (para.algn == QLatin1String("just") || para.algn == QLatin1String("justLow")) align = Qt::AlignJustify;
    else if (para.algn == QLatin1String("dist") || para.algn == QLatin1String("thaiDist")) {
        align = Qt::AlignJustify;
        bf.setProperty(tp::Distribute, true);
    }
    // The standard's left and right are the page's sides, whichever way the
    // text runs; the engine swaps a right-to-left paragraph's unless told.
    if (para.rtl.value_or(false)) {
        align |= Qt::AlignAbsolute;
        bf.setLayoutDirection(Qt::RightToLeft);
    }
    bf.setAlignment(align);
    bf.setLeftMargin(std::clamp(para.marL.value_or(0), 0.0, 20000.0));
    bf.setRightMargin(std::clamp(para.marR.value_or(0), 0.0, 20000.0));
    bf.setTextIndent(std::clamp(para.indent.value_or(0), -20000.0, 20000.0));
    // A share of the line is of one line of the largest text in the paragraph.
    auto distance = [&](const Spacing &s) {
        if (s.kind == Spacing::Points) return s.value;
        return s.kind == Spacing::Percent ? std::clamp(s.value * 1.2 * largest, 0.0, 4000.0) : 0.0;
    };
    if (para.spcBef.kind != Spacing::Unset) bf.setTopMargin(distance(para.spcBef));
    if (para.spcAft.kind != Spacing::Unset) bf.setBottomMargin(distance(para.spcAft));
    // A share of the line is kept as one too (with the points it came to, which end it when the margin is set to something else): in the
    // program that space follows the size the text is drawn at, shrunk or changed, where space in points does not.
    if (para.spcBef.kind == Spacing::Percent && std::isfinite(para.spcBef.value)) {
        bf.setProperty(tp::SpaceBeforeShare, para.spcBef.value);
        bf.setProperty(tp::SpaceBeforeFrom, bf.topMargin());
    }
    if (para.spcAft.kind == Spacing::Percent && std::isfinite(para.spcAft.value)) {
        bf.setProperty(tp::SpaceAfterShare, para.spcAft.value);
        bf.setProperty(tp::SpaceAfterFrom, bf.bottomMargin());
    }
    if (para.lnSpc.kind == Spacing::Percent && para.lnSpc.value > 0) bf.setLineHeight(std::clamp(para.lnSpc.value * 100, 1.0, 1000.0), QTextBlockFormat::ProportionalHeight);
    else if (para.lnSpc.kind == Spacing::Points && para.lnSpc.value > 0) bf.setLineHeight(std::max(para.lnSpc.value, 0.1), QTextBlockFormat::FixedHeight);
    if (level > 0) bf.setProperty(tp::ListLevel, level);
    // An empty paragraph shows no bullet and is left out of the lists; one the file says has none keeps that.
    if (pieces.isEmpty() && !pPr.isNull() && !firstChild(pPr, QLatin1String("buNone")).isNull()) bf.setProperty(kNoBulletParagraph, true);
    // Stops of its own, then (where the paragraph has a tab) the default ones
    // as far on as they are likely to be needed.
    QList<QTextOption::Tab> tabs = para.tabs;
    if (tabbed && para.defTab && *para.defTab >= 1) {
        const double step = *para.defTab, last = tabs.isEmpty() ? 0.0 : tabs.last().position;
        int made = 0;
        for (int k = int(std::min(last / step, 1e6)) + 1; made < 64 && k * step <= 20000; ++k)
            if (k * step > last + 0.01) {
                tabs << QTextOption::Tab(k * step, QTextOption::LeftTab);
                ++made;
            }
    }
    if (!tabs.isEmpty()) bf.setTabPositions(tabs);

    if (m_first) {
        m_cursor.setBlockFormat(bf);
        m_cursor.setBlockCharFormat(endFormat);
        m_first = false;
    } else {
        m_cursor.insertBlock(bf, endFormat);
    }
    for (const Piece &piece : std::as_const(pieces)) m_cursor.insertText(piece.text, piece.format);

    // A bullet. Numbers go on while the paragraphs before are numbered the
    // same way at the level, are interrupted by a paragraph of the level
    // without one, and start over below a paragraph of a shallower level.
    // An empty paragraph shows no bullet and takes no part.
    if (pieces.isEmpty()) return;
    for (int l = level + 1; l < kLevels; ++l) m_lists[l] = ListRun();
    const Bullet &b = para.bullet;
    if (b.kind != Bullet::Char && b.kind != Bullet::AutoNum && b.kind != Bullet::Blip) {
        m_lists[level] = ListRun();
        return;
    }
    const double points = b.size == Bullet::Own ? (b.sizeValue.kind == Spacing::Percent ? b.sizeValue.value * firstSize : b.sizeValue.value) : 0;
    // Paragraphs whose bullets are the same in every way share a list.
    const QString key = QStringLiteral("%1|%2|%3|%4|%5|%6")
                            .arg(int(b.kind)).arg(b.text).arg(b.kind == Bullet::AutoNum ? b.startAt : 0).arg(b.color == Bullet::Own ? colorRefOf(b.colorValue) : QString())
                            .arg(b.font == Bullet::Own ? b.fontValue : QString()).arg(points);
    addToList(para, level, key, firstSize);
}

} // namespace

void readTextBody(const QDomElement &txBody, QTextDocument *doc, const DrawingContext &ctx, const TextStyleStack &styles)
{
    if (!doc) return;
    const bool undo = doc->isUndoRedoEnabled();
    doc->setUndoRedoEnabled(false);
    doc->clear();

    // The body's own list style is the highest layer (unless the shape has
    // handed it over as one already).
    TextStyleStack stack = styles;
    const QDomElement own = firstChild(txBody, QLatin1String("lstStyle"));
    if (!own.isNull() && (stack.isEmpty() || stack.last() != own)) stack.append(own);

    Reader reader(doc, ctx, stack);
    QTextCursor edit(doc);
    edit.beginEditBlock();
    for (QDomElement p = txBody.firstChildElement(); !p.isNull(); p = p.nextSiblingElement()) {
        if (p.localName() != QLatin1String("p")) continue;
        if (!reader.room()) break;
        reader.paragraph(p);
    }
    // A body with no paragraph is one empty paragraph, in the styles' size.
    if (reader.paragraphs() == 0) reader.paragraph(QDomElement());
    edit.endEditBlock();
    doc->setUndoRedoEnabled(undo);
}

} // namespace jp::office
