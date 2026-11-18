// How an open document presentation moves (odpexport_p.h): the transition
// onto each slide, the animations of its objects (the timing nodes of the
// other suite's files), the comments, and how the show plays.

#include "io/odpexport_p.h"
#include "core/items.h"

#include "core/comments.h"
#include "core/sections.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainterPath>
#include <algorithm>
#include <cmath>

namespace jp::deck::odpw {

namespace {

QString attrOf(const char *name, const QString &value) { return QLatin1Char(' ') + QLatin1String(name) + QStringLiteral("=\"") + esc(value) + QLatin1Char('"'); }

QString seconds(double ms) { return number(std::max(0.0, ms) / 1000.0, 3) + QLatin1Char('s'); }

} // namespace

// ---- transitions

// The side a slide comes from, as the file names it, for the way things move on the screen.
static QString comesFrom(const QString &dir)
{
    if (dir == QLatin1String("r")) return QStringLiteral("fromLeft");
    if (dir == QLatin1String("l")) return QStringLiteral("fromRight");
    if (dir == QLatin1String("d")) return QStringLiteral("fromTop");
    return QStringLiteral("fromBottom");
}

static QString oppositeDir(const QString &d)
{
    return d == QLatin1String("l") ? QStringLiteral("r") : d == QLatin1String("r") ? QStringLiteral("l") : d == QLatin1String("u") ? QStringLiteral("d") : QStringLiteral("u");
}

QString Exporter::transitionAttrs(const Page &page)
{
    const Transition t = transitionOf(page);
    QString out;
    QString type, subtype, direction;
    bool reverse = false;
    if (t.type == QLatin1String("fade")) {
        type = QStringLiteral("fade");
        subtype = QStringLiteral("crossfade");
    } else if (t.type == QLatin1String("cut")) {
        type = QStringLiteral("fade");
        subtype = QStringLiteral("fadeOverColor");
    } else if (t.type == QLatin1String("push")) {
        type = QStringLiteral("pushWipe");
        subtype = comesFrom(t.dir);
    } else if (t.type == QLatin1String("cover")) {
        type = QStringLiteral("slideWipe");
        subtype = comesFrom(t.dir);
    } else if (t.type == QLatin1String("uncover") || t.type == QLatin1String("reveal")) {
        type = QStringLiteral("slideWipe");
        subtype = comesFrom(oppositeDir(t.dir));
        reverse = true;
    } else if (t.type == QLatin1String("wipe")) {
        type = QStringLiteral("barWipe");
        const bool across = t.dir == QLatin1String("l") || t.dir == QLatin1String("r");
        subtype = across ? QStringLiteral("leftToRight") : QStringLiteral("topToBottom");
        reverse = t.dir == QLatin1String("l") || t.dir == QLatin1String("u");
    } else if (t.type == QLatin1String("split")) {
        type = QStringLiteral("barnDoorWipe");
        subtype = QStringLiteral("vertical");
        reverse = t.dir == QLatin1String("in");
    } else if (t.type == QLatin1String("zoom")) {
        type = QStringLiteral("irisWipe");
        subtype = QStringLiteral("rectangle");
        reverse = t.dir == QLatin1String("out");
    }
    else if (t.type == QLatin1String("dissolve")) {
        type = QStringLiteral("dissolve");
        subtype = QStringLiteral("default");
    } else if (t.type == QLatin1String("blinds")) {
        type = QStringLiteral("blindsWipe");
        subtype = t.dir == QLatin1String("v") ? QStringLiteral("vertical") : QStringLiteral("horizontal");
    } else if (t.type == QLatin1String("checkerboard")) {
        type = QStringLiteral("checkerBoardWipe");
        subtype = t.dir == QLatin1String("v") ? QStringLiteral("down") : QStringLiteral("across");
    } else if (t.type == QLatin1String("randombars")) {
        type = QStringLiteral("randomBarWipe");
        subtype = t.dir == QLatin1String("v") ? QStringLiteral("vertical") : QStringLiteral("horizontal");
    } else if (t.type == QLatin1String("circle") || (t.type == QLatin1String("shape") && t.by == QLatin1String("circle"))) {
        type = QStringLiteral("ellipseWipe");
        subtype = QStringLiteral("circle");
    } else if (t.type == QLatin1String("diamond") || (t.type == QLatin1String("shape") && t.by == QLatin1String("diamond"))) {
        type = QStringLiteral("irisWipe");
        subtype = QStringLiteral("diamond");
    } else if (t.type == QLatin1String("wedge")) {
        type = QStringLiteral("fanWipe");
        subtype = QStringLiteral("centerTop");
    } else if (t.type == QLatin1String("wheel")) {
        type = QStringLiteral("pinWheelWipe");
        subtype = t.by == QLatin1String("1") ? QStringLiteral("oneBlade") : t.by == QLatin1String("2") ? QStringLiteral("twoBladeVertical")
                : t.by == QLatin1String("3") ? QStringLiteral("threeBlade") : t.by == QLatin1String("8") ? QStringLiteral("eightBlade") : QStringLiteral("fourBlade");
    }
    // The ones the standard has no name for: the nearest effect, and our own type kept beside it.
    else if (t.type == QLatin1String("plus") || (t.type == QLatin1String("shape") && t.by == QLatin1String("plus"))) {
        type = QStringLiteral("fourBoxWipe");
        subtype = QStringLiteral("cornersOut");
    } else if (t.type == QLatin1String("comb")) {
        type = QStringLiteral("blindsWipe");
        subtype = t.dir == QLatin1String("v") ? QStringLiteral("vertical") : QStringLiteral("horizontal");
    } else if (t.type == QLatin1String("strips")) {
        type = QStringLiteral("diagonalWipe");
        subtype = t.dir == QLatin1String("ru") ? QStringLiteral("bottomLeft") : t.dir == QLatin1String("lu") ? QStringLiteral("bottomRight")
                : t.dir == QLatin1String("ld") ? QStringLiteral("topRight") : QStringLiteral("topLeft");
    } else if (t.type == QLatin1String("newsflash")) {
        type = QStringLiteral("irisWipe");
        subtype = QStringLiteral("rectangle");
    } else if (t.type == QLatin1String("flash")) {
        type = QStringLiteral("fade");
        subtype = QStringLiteral("fadeOverColor");
    }
    if (!type.isEmpty() && (t.type == QLatin1String("plus") || (t.type == QLatin1String("shape") && t.by == QLatin1String("plus")) || t.type == QLatin1String("comb") || t.type == QLatin1String("strips") ||
                            t.type == QLatin1String("newsflash") || t.type == QLatin1String("flash"))) {
        QJsonObject own;
        own["type"] = t.type;
        own["dir"] = t.dir;
        own["by"] = t.by;
        out += attrOf("jo:transition", QString::fromUtf8(QJsonDocument(own).toJson(QJsonDocument::Compact)));
    }
    if (t.type == QLatin1String("random")) {
        // The standard's own name for one the show picks.
        out += attrOf("presentation:transition-style", QStringLiteral("random"));
        out += attrOf("presentation:transition-speed", t.duration >= 0.85 ? QStringLiteral("slow") : t.duration <= 0.55 ? QStringLiteral("fast") : QStringLiteral("medium"));
        out += attrOf("smil:dur", QStringLiteral("PT%1S").arg(number(t.duration, 2)));
    }
    if (!type.isEmpty()) {
        out += attrOf("smil:type", type) + attrOf("smil:subtype", subtype);
        if (reverse) out += attrOf("smil:direction", QStringLiteral("reverse"));
        if (t.type == QLatin1String("cut")) out += attrOf("smil:fadeColor", QStringLiteral("#000000"));
        if (t.type == QLatin1String("flash")) out += attrOf("smil:fadeColor", QStringLiteral("#ffffff"));
        out += attrOf("presentation:transition-speed", t.duration >= 0.85 ? QStringLiteral("slow") : t.duration <= 0.55 ? QStringLiteral("fast") : QStringLiteral("medium"));
        out += attrOf("smil:dur", QStringLiteral("PT%1S").arg(number(t.duration, 2)));
    }
    // When it moves on: by itself after a time (and by a click too, if it says so), else by a click.
    if (t.advanceAfter && *t.advanceAfter > 0.001) {
        out += attrOf("presentation:transition-type", t.advanceOnClick ? QStringLiteral("semi-automatic") : QStringLiteral("automatic"));
        out += attrOf("presentation:duration", QStringLiteral("PT%1S").arg(number(*t.advanceAfter, 2)));
    }
    return out;
}

// ---- animations

namespace {

// The ids the presentation program names its effects by, for the standard ones.
QString presetName(EffectClass cls, int id, int sub, QString *subType)
{
    const bool entrance = cls == EffectClass::Entrance;
    const QString base = entrance ? QStringLiteral("ooo-entrance-") : QStringLiteral("ooo-exit-");
    auto direction = [&](int s) -> QString {
        switch (s) {
        case 4: return QStringLiteral("from-bottom");
        case 8: return QStringLiteral("from-left");
        case 1: return QStringLiteral("from-top");
        case 2: return QStringLiteral("from-right");
        case 12: return QStringLiteral("from-bottom-left");
        case 9: return QStringLiteral("from-top-left");
        case 3: return QStringLiteral("from-top-right");
        case 6: return QStringLiteral("from-bottom-right");
        }
        return QString();
    };
    if (cls == EffectClass::Entrance || cls == EffectClass::Exit) {
        switch (id) {
        case 1: return base + (entrance ? QStringLiteral("appear") : QStringLiteral("disappear"));
        case 10: return base + (entrance ? QStringLiteral("fade-in") : QStringLiteral("fade-out"));
        case 2:
            *subType = direction(sub);
            return base + (entrance ? QStringLiteral("fly-in") : QStringLiteral("fly-out"));
        case 42: return base + (entrance ? QStringLiteral("ascend") : QStringLiteral("descend"));
        case 47: return base + (entrance ? QStringLiteral("descend") : QStringLiteral("ascend"));
        case 16: {
            *subType = sub == 21 ? QStringLiteral("vertical-in") : sub == 37 ? QStringLiteral("vertical-out") : sub == 26 ? QStringLiteral("horizontal-in") : sub == 42 ? QStringLiteral("horizontal-out") : QString();
            return base + QStringLiteral("split");
        }
        case 22:
            *subType = direction(sub);
            return base + QStringLiteral("wipe");
        case 31: return base + QStringLiteral("turn-and-grow");
        case 3: *subType = sub == 5 ? QStringLiteral("vertical") : QStringLiteral("horizontal"); return base + QStringLiteral("venetian-blinds");
        case 4: *subType = sub == 32 ? QStringLiteral("out") : QStringLiteral("in"); return base + QStringLiteral("box");
        case 5: *subType = sub == 5 ? QStringLiteral("downward") : QStringLiteral("across"); return base + QStringLiteral("checkerboard");
        case 6: *subType = sub == 32 ? QStringLiteral("out") : QStringLiteral("in"); return base + QStringLiteral("circle");
        case 8: *subType = sub == 32 ? QStringLiteral("out") : QStringLiteral("in"); return base + QStringLiteral("diamond");
        case 13: *subType = sub == 32 ? QStringLiteral("out") : QStringLiteral("in"); return base + QStringLiteral("plus");
        case 9: return base + (entrance ? QStringLiteral("dissolve-in") : QStringLiteral("dissolve"));
        case 14: *subType = sub == 5 ? QStringLiteral("vertical") : QStringLiteral("horizontal"); return base + QStringLiteral("random-bars");
        case 21: *subType = QString::number(sub); return base + QStringLiteral("wheel");   // the spoke count alone: 1, 2, 3, 4, or 8
        case 12:
            *subType = direction(sub);
            return base + (entrance ? QStringLiteral("peek-in") : QStringLiteral("peek-out"));
        case 11: return base + QStringLiteral("flash-once");
        case 17:
            if (entrance) *subType = sub == 10 ? QStringLiteral("across") : sub == 5 ? QStringLiteral("downward") : direction(sub);
            return base + (entrance ? QStringLiteral("stretchy") : QStringLiteral("contract"));   // the exit has no direction
        case 19: *subType = sub == 5 ? QStringLiteral("vertical") : QStringLiteral("horizontal"); return base + QStringLiteral("swivel");
        case 55:
            if (!entrance) *subType = QStringLiteral("across");
            return base + (entrance ? QStringLiteral("expand") : QStringLiteral("collapse"));
        case 15: return base + (entrance ? QStringLiteral("spiral-in") : QStringLiteral("spiral-out"));
        case 26: return base + QStringLiteral("bounce");
        case 53:
            *subType = entrance ? QStringLiteral("in") : QStringLiteral("out");
            return base + (entrance ? QStringLiteral("fade-in-and-zoom") : QStringLiteral("fade-out-and-zoom"));
        }
    } else if (cls == EffectClass::Emphasis) {
        switch (id) {
        case 9: return QStringLiteral("ooo-emphasis-transparency");
        case 26: return QStringLiteral("ooo-emphasis-flash-bulb");
        case 32: return QStringLiteral("ooo-emphasis-teeter");
        case 8: return QStringLiteral("ooo-emphasis-spin");
        case 6: return QStringLiteral("ooo-emphasis-grow-and-shrink");
        }
    }
    return QString();
}

// The transition filter an effect's filter name stands for (as the standard names the wipes); false for a filter with none.
bool filterNames(const QString &filter, QString *type, QString *subtype, bool *reverse)
{
    *reverse = false;
    if (filter == QLatin1String("fade")) {
        *type = QStringLiteral("fade");
        *subtype = QStringLiteral("crossfade");
        return true;
    }
    static const QRegularExpression wipe(QStringLiteral("^wipe\\((up|down|left|right)\\)$")), barn(QStringLiteral("^barn\\((in|out)(Vertical|Horizontal)\\)$"));
    if (const QRegularExpressionMatch m = wipe.match(filter); m.hasMatch()) {
        const QString d = m.captured(1);
        *type = QStringLiteral("barWipe");
        *subtype = d == QLatin1String("left") || d == QLatin1String("right") ? QStringLiteral("leftToRight") : QStringLiteral("topToBottom");
        *reverse = d == QLatin1String("left") || d == QLatin1String("up");
        return true;
    }
    if (const QRegularExpressionMatch m = barn.match(filter); m.hasMatch()) {
        *type = QStringLiteral("barnDoorWipe");
        *subtype = m.captured(2) == QLatin1String("Vertical") ? QStringLiteral("vertical") : QStringLiteral("horizontal");
        *reverse = m.captured(1) == QLatin1String("in");
        return true;
    }
    // The common effects: the forms open from the middle (out) or close in from the edges (in); the standard's own direction is outward.
    static const QRegularExpression common(QStringLiteral("^(blinds|checkerboard|randombar|box|circle|diamond|plus|wheel)\\(([A-Za-z0-9]+)\\)$"));
    if (filter == QLatin1String("dissolve")) {
        *type = QStringLiteral("dissolve");
        *subtype = QStringLiteral("default");
        return true;
    }
    if (const QRegularExpressionMatch m = common.match(filter); m.hasMatch()) {
        const QString what = m.captured(1), option = m.captured(2);
        *reverse = option == QLatin1String("in");
        if (what == QLatin1String("blinds")) {
            *type = QStringLiteral("blindsWipe");
            *subtype = option == QLatin1String("vertical") ? QStringLiteral("vertical") : QStringLiteral("horizontal");
        } else if (what == QLatin1String("checkerboard")) {
            *type = QStringLiteral("checkerBoardWipe");
            *subtype = option == QLatin1String("down") ? QStringLiteral("down") : QStringLiteral("across");
        } else if (what == QLatin1String("randombar")) {
            *type = QStringLiteral("randomBarWipe");
            *subtype = option == QLatin1String("vertical") ? QStringLiteral("vertical") : QStringLiteral("horizontal");
        } else if (what == QLatin1String("box")) {
            *type = QStringLiteral("irisWipe");
            *subtype = QStringLiteral("rectangle");
        } else if (what == QLatin1String("circle")) {
            *type = QStringLiteral("ellipseWipe");
            *subtype = QStringLiteral("circle");
        } else if (what == QLatin1String("diamond")) {
            *type = QStringLiteral("irisWipe");
            *subtype = QStringLiteral("diamond");
        } else if (what == QLatin1String("plus")) {
            *type = QStringLiteral("fourBoxWipe");
            *subtype = QStringLiteral("cornersIn");
            *reverse = option == QLatin1String("out");   // the corner boxes growing in is the plus closing
        } else {
            *reverse = false;
            *type = QStringLiteral("pinWheelWipe");
            *subtype = option == QLatin1String("1") ? QStringLiteral("oneBlade") : option == QLatin1String("2") ? QStringLiteral("twoBladeVertical")
                     : option == QLatin1String("3") ? QStringLiteral("threeBlade") : option == QLatin1String("8") ? QStringLiteral("eightBlade") : QStringLiteral("fourBlade");
        }
        return true;
    }
    return false;
}

// A formula over the shape's place, in the names the file has for them.
QString formula(QString f)
{
    f.replace(QLatin1String("#ppt_x"), QLatin1String("x")).replace(QLatin1String("#ppt_y"), QLatin1String("y"));
    f.replace(QLatin1String("#ppt_w"), QLatin1String("width")).replace(QLatin1String("#ppt_h"), QLatin1String("height"));
    return f;
}

QString attributeName(const QString &a)
{
    if (a == QLatin1String("ppt_x")) return QStringLiteral("x");
    if (a == QLatin1String("ppt_y")) return QStringLiteral("y");
    if (a == QLatin1String("ppt_w")) return QStringLiteral("width");
    if (a == QLatin1String("ppt_h")) return QStringLiteral("height");
    if (a == QLatin1String("style.rotation")) return QStringLiteral("rotate");
    if (a == QLatin1String("style.opacity")) return QStringLiteral("opacity");
    if (a == QLatin1String("style.visibility")) return QStringLiteral("visibility");
    return QString();
}

// A path as the file writes it: fractions of the slide.
QString motionPath(const QPainterPath &path)
{
    QString out;
    const int n = path.elementCount();
    auto num = [](double v) { return number(v, 4); };
    for (int i = 0; i < n; ++i) {
        const QPainterPath::Element e = path.elementAt(i);
        if (e.type == QPainterPath::MoveToElement) out += QStringLiteral("M %1 %2 ").arg(num(e.x), num(e.y));
        else if (e.type == QPainterPath::LineToElement) out += QStringLiteral("L %1 %2 ").arg(num(e.x), num(e.y));
        else if (e.type == QPainterPath::CurveToElement && i + 2 < n) {
            out += QStringLiteral("C %1 %2 %3 %4 %5 %6 ").arg(num(e.x), num(e.y), num(path.elementAt(i + 1).x), num(path.elementAt(i + 1).y), num(path.elementAt(i + 2).x), num(path.elementAt(i + 2).y));
            i += 2;
        }
    }
    return out.trimmed();
}

// What this kind of file calls what an effect asks of a movie or sound; empty for a call it has no word for.
QString mediaCommandName(const QString &command)
{
    if (command.startsWith(QLatin1String("play"))) return QStringLiteral("play");
    if (command == QLatin1String("togglePause") || command == QLatin1String("pause") || command == QLatin1String("resume")) return QStringLiteral("toggle-pause");
    if (command == QLatin1String("stop")) return QStringLiteral("stop");
    return QString();
}

} // namespace

void Exporter::writeAnimations(X &x, const Page &page)
{
    const Timeline tl = timelineOf(page);
    if (tl.isEmpty() && tl.triggers.isEmpty()) return;
    int lost = 0;
    // An effect that plays, pauses, or stops a movie or sound: the call the other suite writes for it, in a part of its own. Empty when
    // the effect asks for anything else, or of something that isn't here.
    auto mediaEffect = [&](const Effect &e) -> QString {
        X inner;
        bool any = false, usable = !e.behaviors.isEmpty();
        for (const Behavior &b : e.behaviors) {
            const auto target = m_idOfShape.constFind(b.target.shape);
            const QString name = mediaCommandName(b.command);
            if (b.kind != Behavior::Kind::Command || target == m_idOfShape.cend() || name.isEmpty()) {
                usable = false;
                continue;
            }
            const double rel = std::max(0.0, b.timing.begin - e.begin);
            inner.open("anim:command");
            if (rel > 0.5) inner.attr("smil:begin", seconds(rel));
            inner.attr("smil:dur", "0.001s").attr("smil:fill", "hold").attr("smil:targetElement", target.value()).attr("anim:command", name).close();
            any = true;
        }
        if (!any || !usable) return QString();
        X ex;
        ex.open("anim:par").attr("smil:begin", "@@begin@@").attr("smil:fill", "hold");
        ex.attr("presentation:node-type", e.start == EffectStart::OnClick ? "on-click" : e.start == EffectStart::WithPrevious ? "with-previous" : "after-previous");
        ex.attr("presentation:preset-class", "media-call").attr("presentation:preset-id", "");
        ex.raw(inner.str()).close();
        return ex.str();
    };
    X steps;
    for (const Step &step : tl.steps) {
        struct Out {
            double begin = 0, end = 0;
            EffectStart start = EffectStart::OnClick;
            QString xml;
        };
        QVector<Out> effects;
        for (const Effect &e : step.effects) {
            if (e.implied) continue;
            if (e.cls == EffectClass::Media) {
                const QString xml = mediaEffect(e);
                if (xml.isEmpty()) {
                    ++lost;
                    continue;
                }
                Out o;
                o.begin = e.begin;
                o.end = e.end();
                o.start = e.start;
                o.xml = xml;
                effects << o;
                continue;
            }
            if (e.cls != EffectClass::Entrance && e.cls != EffectClass::Exit && e.cls != EffectClass::Emphasis && e.cls != EffectClass::MotionPath) {
                ++lost;
                continue;
            }
            X ex;
            bool aimed = false, usable = true;
            QString firstTarget;
            QString subType;
            const QString preset = presetName(e.cls, e.presetId, e.presetSubtype, &subType);
            ex.open("anim:par").attr("smil:begin", "@@begin@@").attr("smil:fill", e.remove ? "remove" : "hold");
            if (e.repeat > 1.001) ex.attr("smil:repeatCount", number(e.repeat, 2));
            ex.attr("presentation:node-type", e.start == EffectStart::OnClick ? "on-click" : e.start == EffectStart::WithPrevious ? "with-previous" : "after-previous");
            ex.attr("presentation:preset-class", e.cls == EffectClass::Entrance ? "entrance" : e.cls == EffectClass::Exit ? "exit" : e.cls == EffectClass::Emphasis ? "emphasis" : "motion-path");
            if (!preset.isEmpty()) ex.attr("presentation:preset-id", preset);
            if (!subType.isEmpty()) ex.attr("presentation:preset-sub-type", subType);
            // The parts: what each behavior does to its target, as a node of its own.
            X inner;
            for (const Behavior &b : e.behaviors) {
                // The targets: a whole object, or the paragraphs of its text.
                QStringList targets;
                if (b.target.paragraphs()) {
                    for (int p = b.target.firstParagraph; p <= b.target.lastParagraph && p < b.target.firstParagraph + 1000; ++p) {
                        const QString id = m_paragraphIds.value(qMakePair(b.target.shape, p));
                        if (!id.isEmpty()) targets << id;
                    }
                } else if (const auto it = m_idOfShape.constFind(b.target.shape); it != m_idOfShape.cend()) {
                    targets << it.value();
                }
                if (targets.isEmpty()) {
                    if (b.kind != Behavior::Kind::Command) usable = false;
                    continue;
                }
                if (firstTarget.isEmpty()) firstTarget = targets.first();
                const double rel = std::max(0.0, b.timing.begin - e.begin);
                for (const QString &target : std::as_const(targets)) {
                    auto common = [&](bool withBegin) {
                        if (withBegin) inner.attr("smil:begin", seconds(rel));
                        if (b.timing.duration >= 0) inner.attr("smil:dur", seconds(b.timing.duration));
                    };
                    auto finish = [&] {
                        if (b.timing.hold) inner.attr("smil:fill", "hold");
                        if (b.timing.autoReverse) inner.attr("smil:autoReverse", "true");
                        if (b.timing.accel > 0.001) inner.attr("smil:accelerate", number(b.timing.accel, 3));
                        if (b.timing.decel > 0.001) inner.attr("smil:decelerate", number(b.timing.decel, 3));
                        inner.attr("smil:targetElement", target);
                    };
                    switch (b.kind) {
                    case Behavior::Kind::Set: {
                        const QString name = attributeName(b.attribute);
                        if (name != QLatin1String("visibility")) {
                            usable = false;
                            break;
                        }
                        inner.open("anim:set");
                        common(true);
                        inner.attr("smil:fill", "hold").attr("smil:targetElement", target);
                        inner.attr("smil:attributeName", "visibility").attr("smil:to", b.to).close();
                        aimed = true;
                        break;
                    }
                    case Behavior::Kind::Effect: {
                        QString type, subtype;
                        bool reverse = false;
                        if (!filterNames(b.filter, &type, &subtype, &reverse)) {
                            usable = false;
                            break;
                        }
                        inner.open("anim:transitionFilter");
                        if (b.timing.begin - e.begin > 0.5) inner.attr("smil:begin", seconds(rel));
                        if (b.timing.duration >= 0) inner.attr("smil:dur", seconds(b.timing.duration));
                        if (b.timing.hold) inner.attr("smil:fill", "hold");
                        inner.attr("smil:targetElement", target).attr("smil:type", type).attr("smil:subtype", subtype);
                        if (!b.entering) inner.attr("smil:mode", "out");
                        if (reverse) inner.attr("smil:direction", "reverse");
                        inner.close();
                        aimed = true;
                        break;
                    }
                    case Behavior::Kind::Animate: {
                        const QString name = attributeName(b.attribute);
                        if (name.isEmpty()) {
                            usable = false;
                            break;
                        }
                        inner.open("anim:animate");
                        if (rel > 0.5) inner.attr("smil:begin", seconds(rel));
                        if (b.timing.duration >= 0) inner.attr("smil:dur", seconds(b.timing.duration));
                        finish();
                        inner.attr("smil:attributeName", name);
                        if (!b.keys.isEmpty()) {
                            QStringList values, times;
                            for (const Keyframe &k : b.keys) {
                                values << formula(k.value);
                                times << number(k.at, 4);
                            }
                            inner.attr("smil:values", values.join(QLatin1Char(';'))).attr("smil:keyTimes", times.join(QLatin1Char(';')));
                        } else {
                            if (!b.from.isEmpty()) inner.attr("smil:from", formula(b.from));
                            if (!b.to.isEmpty()) inner.attr("smil:to", formula(b.to));
                            if (!b.by.isEmpty()) inner.attr("smil:by", formula(b.by));
                        }
                        if (b.discrete) inner.attr("smil:calcMode", "discrete");
                        if (b.additive == QLatin1String("sum")) inner.attr("smil:additive", "sum");
                        inner.close();
                        aimed = true;
                        break;
                    }
                    case Behavior::Kind::Motion: {
                        if (b.path.isEmpty()) {
                            usable = false;
                            break;
                        }
                        inner.open("anim:animateMotion");
                        if (rel > 0.5) inner.attr("smil:begin", seconds(rel));
                        if (b.timing.duration >= 0) inner.attr("smil:dur", seconds(b.timing.duration));
                        finish();
                        inner.attr("svg:path", motionPath(b.path)).close();
                        aimed = true;
                        break;
                    }
                    case Behavior::Kind::Scale: {
                        inner.open("anim:animateTransform");
                        if (rel > 0.5) inner.attr("smil:begin", seconds(rel));
                        if (b.timing.duration >= 0) inner.attr("smil:dur", seconds(b.timing.duration));
                        finish();
                        inner.attr("smil:attributeName", "transform").attr("svg:type", "scale");
                        if (std::fabs(b.scaleFrom.x() - 1) < 1e-6 && std::fabs(b.scaleFrom.y() - 1) < 1e-6)
                            inner.attr("smil:by", QStringLiteral("%1,%2").arg(number(b.scaleTo.x() - 1, 4), number(b.scaleTo.y() - 1, 4)));
                        else
                            inner.attr("smil:from", QStringLiteral("%1,%2").arg(number(b.scaleFrom.x(), 4), number(b.scaleFrom.y(), 4))).attr("smil:to", QStringLiteral("%1,%2").arg(number(b.scaleTo.x(), 4), number(b.scaleTo.y(), 4)));
                        inner.close();
                        aimed = true;
                        break;
                    }
                    case Behavior::Kind::Rotate: {
                        inner.open("anim:animateTransform");
                        if (rel > 0.5) inner.attr("smil:begin", seconds(rel));
                        if (b.timing.duration >= 0) inner.attr("smil:dur", seconds(b.timing.duration));
                        finish();
                        inner.attr("smil:attributeName", "transform").attr("svg:type", "rotate");
                        if (std::fabs(b.rotateFrom) < 1e-6) inner.attr("smil:by", number(b.rotateTo, 3));
                        else inner.attr("smil:from", number(b.rotateFrom, 3)).attr("smil:to", number(b.rotateTo, 3));
                        inner.close();
                        aimed = true;
                        break;
                    }
                    case Behavior::Kind::Color:
                    case Behavior::Kind::Command:
                        usable = false;
                        break;
                    }
                }
            }
            if (!aimed || !usable) {
                ++lost;
                continue;
            }
            // Hidden when the next click comes.
            if (e.hideOnNextClick && !firstTarget.isEmpty()) inner.open("anim:set").attr("smil:dur", seconds(1)).attr("smil:fill", "hold").attr("smil:targetElement", firstTarget).attr("smil:attributeName", "visibility").attr("smil:to", "hidden").close();
            ex.raw(inner.str());
            ex.close();
            Out o;
            o.begin = e.begin;
            o.end = e.end();
            o.start = e.start;
            o.xml = ex.str();
            effects << o;
        }
        if (effects.isEmpty()) continue;
        // The effects of a step go in groups: each effect that starts after the one before begins a group, which starts when those before it end.
        steps.open("anim:par").attr("smil:begin", step.automatic ? "0s" : "next").attr("smil:fill", "hold");
        double groupBegin = 0, latest = 0;
        bool open = false;
        for (const Out &o : std::as_const(effects)) {
            if (!open || o.start == EffectStart::AfterPrevious) {
                if (open) steps.close();
                groupBegin = open ? std::min(o.begin, latest) : 0;
                steps.open("anim:par").attr("smil:begin", seconds(groupBegin)).attr("smil:fill", "hold");
                open = true;
            }
            QString xml = o.xml;
            // The effect's own delay is its start less its group's.
            const double delay = std::max(0.0, o.begin - groupBegin);
            xml.replace(QStringLiteral("@@begin@@"), seconds(delay));
            steps.raw(xml);
            latest = std::max(latest, o.end);
        }
        if (open) steps.close();
        steps.close();
    }
    // What starts when an object is clicked: only a movie's or sound's calls have a form here, a sequence of their own.
    X clicked;
    for (const Trigger &trigger : tl.triggers) {
        const auto shape = m_idOfShape.constFind(trigger.shape);
        QString group;
        bool usable = shape != m_idOfShape.cend() && !trigger.effects.isEmpty();
        for (const Effect &e : trigger.effects) {
            QString xml = e.cls == EffectClass::Media ? mediaEffect(e) : QString();
            if (xml.isEmpty()) {
                usable = false;
                break;
            }
            group += xml.replace(QStringLiteral("@@begin@@"), seconds(e.begin));
        }
        if (!usable) {
            sayOnce(QStringLiteral("triggers"), QCoreApplication::translate("OdpExport", "Effects that start when an object is clicked can't be saved in this kind of file, except those that play a movie or sound."));
            continue;
        }
        clicked.open("anim:seq").attr("smil:fill", "hold").attr("smil:restart", "whenNotActive").attr("smil:endsync", "all").attr("presentation:node-type", "interactive-sequence");
        clicked.open("anim:par").attr("smil:begin", shape.value() + QStringLiteral(".click+0s")).attr("smil:fill", "hold");
        clicked.open("anim:par").attr("smil:begin", "0s").attr("smil:fill", "hold").raw(group).close().close().close();
    }
    if (lost > 0) say(QCoreApplication::translate("OdpExport", "%1: animation effects that have no match in this kind of file were left out.").arg(m_where));
    if (steps.str().isEmpty() && clicked.str().isEmpty()) return;
    x.open("anim:par").attr("smil:dur", "indefinite").attr("smil:restart", "never").attr("presentation:node-type", "timing-root");
    x.open("anim:seq").attr("smil:dur", "indefinite").attr("presentation:node-type", "main-sequence");
    x.raw(steps.str());
    x.close();
    x.raw(clicked.str());
    x.close();
}

// ---- comments

void Exporter::writeComments(X &x, const Page &page)
{
    const QVector<comments::Comment> list = comments::commentsOf(page);
    for (const comments::Comment &c : list) {
        auto note = [&](const QString &author, const QString &initials, const QDateTime &date, const QString &text) {
            x.open("officeooo:annotation").len("svg:x", c.at.x()).len("svg:y", c.at.y()).len("svg:width", 14).len("svg:height", 14);
            if (!author.isEmpty()) x.open("dc:creator").text(author).close();
            if (!initials.isEmpty()) x.open("meta:creator-initials").text(initials).close();
            if (date.isValid()) x.open("dc:date").text(date.toUTC().toString(QStringLiteral("yyyy-MM-dd'T'HH:mm:ss"))).close();
            const QStringList lines = text.split(QLatin1Char('\n'));
            for (const QString &line : lines) x.open("text:p").text(line).close();
            x.close();
        };
        note(c.author, c.initials, c.date, c.text);
        for (const comments::CommentReply &r : c.replies) note(r.author, r.initials, r.date, r.text);
        if (c.done) sayOnce(QStringLiteral("resolved"), QCoreApplication::translate("OdpExport", "Comments marked as done are saved as plain comments."));
    }
}

// ---- how the show plays

void Exporter::writeShowSettings(X &x)
{
    const ShowSettings s = showSettingsOf(m_doc);
    const QVector<CustomShow> shows = customShowsOf(m_doc);
    const ShowSettings defaults;
    if (!sectionsOf(m_doc).isEmpty()) say(QCoreApplication::translate("OdpExport", "Sections can't be saved in this kind of file."));
    if (s == defaults && shows.isEmpty()) return;
    x.open("presentation:settings");
    const bool kiosk = s.type == ShowSettings::Type::Kiosk;
    x.attr("presentation:full-screen", s.type == ShowSettings::Type::Individual ? "false" : "true");
    x.attr("presentation:endless", s.looping() ? "true" : "false");
    x.attr("presentation:pause", QStringLiteral("PT%1S").arg(kiosk ? number(s.kioskRestart / 1000.0, 0) : QStringLiteral("0")));
    x.attr("presentation:force-manual", s.usesTimings() ? "false" : "true");
    x.attr("presentation:animations", s.animation ? "enabled" : "disabled");
    x.attr("presentation:transition-on-click", kiosk ? "disabled" : "enabled");
    if (s.slides == ShowSettings::Slides::Range && s.from >= 1 && s.from <= m_slideNames.size()) {
        x.attr("presentation:start-page", m_slideNames[s.from - 1]);
        if (s.to < m_slideNames.size()) sayOnce(QStringLiteral("range"), QCoreApplication::translate("OdpExport", "The last slide of the range to show can't be saved; the show runs to the end."));
    } else if (s.slides == ShowSettings::Slides::Custom) {
        if (const CustomShow *show = customShowById(shows, s.customShow)) x.attr("presentation:show", show->name);
    }
    for (const CustomShow &show : shows) {
        QStringList names;
        for (int index : customShowSlides(m_doc, show))
            if (index >= 0 && index < m_slideNames.size()) names << m_slideNames[index];
        if (names.isEmpty()) continue;
        x.open("presentation:show").attr("presentation:name", show.name).attr("presentation:pages", names.join(QLatin1Char(','))).close();
    }
    x.close();
}

} // namespace jp::deck::odpw
