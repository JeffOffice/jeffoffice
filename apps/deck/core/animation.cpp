#include "core/animation.h"

#include "office/xml.h"

#include <QDomDocument>
#include <QPaintDevice>
#include <QPainter>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>
#include <QTransform>
#include <algorithm>
#include <cmath>
#include <limits>

namespace jp::deck {

size_t qHash(const Target &t, size_t seed) noexcept { return qHashMulti(seed, t.shape, t.firstParagraph, t.lastParagraph); }

// ---- time ----

namespace {

// Speeding up for the first `accel` of a run and slowing down for the last
// `decel`, over the same time: x (0 to 1 through the run) to the way gone.
double eased(double x, double accel, double decel)
{
    accel = std::clamp(accel, 0.0, 1.0);
    decel = std::clamp(decel, 0.0, 1.0);
    if (accel + decel > 1) {
        const double k = 1 / (accel + decel);
        accel *= k;
        decel *= k;
    }
    if (accel <= 0 && decel <= 0) return x;
    const double speed = 1 / (1 - accel / 2 - decel / 2);
    if (x < accel) return speed * x * x / (2 * accel);
    if (x > 1 - decel) {
        const double left = 1 - x;
        return 1 - speed * left * left / (2 * decel);
    }
    return speed * (x - accel / 2);
}

// Where a run is shown to be, when the file maps how far through it is to how far it shows: between the points.
double mapped(double x, const QVector<QPointF> &points)
{
    if (points.isEmpty()) return x;
    if (x <= points.first().x()) return points.first().y();
    for (qsizetype i = 1; i < points.size(); ++i) {
        if (x > points[i].x()) continue;
        const double span = points[i].x() - points[i - 1].x();
        return span > 0 ? points[i - 1].y() + (points[i].y() - points[i - 1].y()) * (x - points[i - 1].x()) / span : points[i].y();
    }
    return points.last().y();
}

} // namespace

double Timing::activeDuration() const
{
    if (duration <= 0) return 0;
    const double run = autoReverse ? duration * 2 : duration;
    double total = run * (repeat > 0 ? repeat : 1);
    if (repeatDuration >= 0) total = std::min(total, repeatDuration);
    return total;
}

Timing::Phase Timing::phaseAt(double ms, double *progress) const
{
    double ignored = 0;
    double &out = progress ? *progress : ignored;
    const double t = ms - begin;
    if (t < 0) {
        out = 0;
        return Phase::Before;
    }
    if (duration <= 0) {
        out = 1;
        return Phase::After;
    }
    const double run = autoReverse ? duration * 2 : duration;
    auto within = [&](double u) {
        const double x = u < duration ? u / duration : 1 - (u - duration) / duration;
        return mapped(eased(std::clamp(x, 0.0, 1.0), accel, decel), filter);
    };
    const double total = activeDuration();
    if (t >= total) {
        const double rest = std::fmod(total, run);
        out = rest < 1e-9 || run - rest < 1e-9 ? mapped(autoReverse ? 0.0 : 1.0, filter) : within(rest);
        return Phase::After;
    }
    out = within(std::fmod(t, run));
    return Phase::Running;
}

// ---- effects and steps ----

namespace {

bool playsAttribute(const QString &name)
{
    return name == QLatin1String("style.visibility") || name == QLatin1String("style.opacity") || name == QLatin1String("style.rotation") ||
           name == QLatin1String("ppt_x") || name == QLatin1String("ppt_y") || name == QLatin1String("ppt_w") || name == QLatin1String("ppt_h");
}

bool playsBehavior(const Behavior &b)
{
    switch (b.kind) {
    case Behavior::Kind::Set: case Behavior::Kind::Animate: return playsAttribute(b.attribute);
    case Behavior::Kind::Effect: case Behavior::Kind::Motion: case Behavior::Kind::Scale: case Behavior::Kind::Rotate: return true;
    default: return false;
    }
}

} // namespace

double Effect::once() const
{
    double e = begin;
    for (const Behavior &b : behaviors)
        if (b.timing.duration >= 0) e = std::max(e, b.timing.end());
    return e - begin;
}

double Effect::end() const { return begin + once() * std::max(1.0, repeat); }

bool Effect::visual() const
{
    return std::any_of(behaviors.begin(), behaviors.end(), [](const Behavior &b) { return b.target.shape > 0 && playsBehavior(b); });
}

double Step::duration() const
{
    double d = 0;
    for (const Effect &e : effects) d = std::max(d, e.end());
    return d;
}

bool Step::visual() const { return std::any_of(effects.begin(), effects.end(), [](const Effect &e) { return e.visual(); }); }

bool TargetState::operator==(const TargetState &o) const
{
    return visible == o.visible && opacity == o.opacity && offset == o.offset && scaleX == o.scaleX && scaleY == o.scaleY && rotation == o.rotation && clip == o.clip && clip2 == o.clip2;
}

// ---- reading the file's timing ----

namespace {

using office::attr;
using office::hasAttr;

QDomElement pChild(const QDomElement &e, const char *name) { return office::child(e, office::ns::P, QLatin1String(name)); }
QVector<QDomElement> pChildren(const QDomElement &e, const char *name) { return office::children(e, office::ns::P, QLatin1String(name)); }
bool pIs(const QDomElement &e, const char *name) { return office::isElement(e, office::ns::P, QLatin1String(name)); }

// The delay a time node starts after: the first condition without an event. "indefinite" waits for a click, which the steps are.
double delayOf(const QDomElement &cTn)
{
    for (const QDomElement &cond : pChildren(pChild(cTn, "stCondLst"), "cond")) {
        if (hasAttr(cond, QStringLiteral("evt"))) continue;
        const QString d = attr(cond, QStringLiteral("delay"));
        if (d.isEmpty() || d == QLatin1String("indefinite")) continue;
        bool ok = false;
        const double v = d.toDouble(&ok);
        if (ok) return std::max(0.0, v);
    }
    return 0;
}

double numberOr(const QString &s, double fallback)
{
    bool ok = false;
    const double v = s.toDouble(&ok);
    return ok ? v : fallback;
}

Timing timingOf(const QDomElement &cTn, double base)
{
    Timing t;
    t.begin = base + delayOf(cTn);
    const QString dur = attr(cTn, QStringLiteral("dur"));
    t.duration = dur.isEmpty() || dur == QLatin1String("indefinite") ? -1 : numberOr(dur, -1);
    t.accel = numberOr(attr(cTn, QStringLiteral("accel")), 0) / 100000;
    t.decel = numberOr(attr(cTn, QStringLiteral("decel")), 0) / 100000;
    t.autoReverse = office::parseBool(attr(cTn, QStringLiteral("autoRev")), false);
    const QString count = attr(cTn, QStringLiteral("repeatCount"));
    t.repeat = count.isEmpty() ? 1 : count == QLatin1String("indefinite") ? -1 : numberOr(count, 1000) / 1000;
    const QString repeatDur = attr(cTn, QStringLiteral("repeatDur"));
    t.repeatDuration = repeatDur.isEmpty() || repeatDur == QLatin1String("indefinite") ? -1 : numberOr(repeatDur, -1);
    const QString fill = attr(cTn, QStringLiteral("fill"));
    t.hold = fill == QLatin1String("hold") || fill == QLatin1String("freeze");
    // "0, 0; .2, .5; 1, 0": points of how far through a run is, and how far it shows.
    for (const QString &pair : attr(cTn, QStringLiteral("tmFilter")).split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
        const QStringList xy = pair.split(QLatin1Char(','));
        bool okx = false, oky = false;
        const double x = xy.value(0).trimmed().toDouble(&okx), y = xy.value(1).trimmed().toDouble(&oky);
        if (xy.size() == 2 && okx && oky && t.filter.size() < 64) t.filter.push_back(QPointF(std::clamp(x, 0.0, 1.0), y));
    }
    std::stable_sort(t.filter.begin(), t.filter.end(), [](const QPointF &a, const QPointF &b) { return a.x() < b.x(); });
    return t;
}

Target targetOf(const QDomElement &cBhvr)
{
    Target t;
    const QDomElement sp = pChild(pChild(cBhvr, "tgtEl"), "spTgt");
    if (sp.isNull()) return t;
    t.shape = int(office::parseInt(attr(sp, QStringLiteral("spid")), 0));
    if (const QDomElement range = pChild(pChild(sp, "txEl"), "pRg"); !range.isNull()) {
        const int first = int(office::parseInt(attr(range, QStringLiteral("st")), 0));
        const int last = int(office::parseInt(attr(range, QStringLiteral("end")), first));
        t.firstParagraph = std::max(0, std::min(first, last));
        t.lastParagraph = std::max(first, last);
    }
    return t;
}

// "M 0 0 L 0.25 0.1 C ... E": fractions of the slide. Lower case letters are relative to the point before.
QPainterPath pathOf(const QString &text)
{
    QPainterPath path;
    const QStringList tokens = text.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    qsizetype i = 0;
    QPointF current;
    QChar cmd;
    auto point = [&](QPointF *out) {
        if (i + 1 >= tokens.size()) return false;
        bool okx = false, oky = false;
        const double x = tokens[i].toDouble(&okx), y = tokens[i + 1].toDouble(&oky);
        if (!okx || !oky) return false;
        *out = cmd.isLower() ? current + QPointF(x, y) : QPointF(x, y);
        i += 2;
        return true;
    };
    while (i < tokens.size()) {
        if (tokens[i].size() == 1 && tokens[i][0].isLetter()) {
            cmd = tokens[i][0];
            ++i;
            const QChar u = cmd.toUpper();
            if (u == QLatin1Char('E')) break;
            if (u == QLatin1Char('Z')) path.closeSubpath();
            continue;
        }
        QPointF a, b, c;
        const QChar u = cmd.toUpper();
        if (u == QLatin1Char('M')) {
            if (!point(&a)) break;
            path.moveTo(a);
            current = a;
            cmd = cmd.isLower() ? QLatin1Char('l') : QLatin1Char('L');
        } else if (u == QLatin1Char('L')) {
            if (!point(&a)) break;
            path.lineTo(a);
            current = a;
        } else if (u == QLatin1Char('C')) {
            if (!point(&a) || !point(&b) || !point(&c)) break;
            path.cubicTo(a, b, c);
            current = c;
        } else {
            break;
        }
    }
    return path;
}

QString valueOf(const QDomElement &holder)
{
    const QVector<QDomElement> kids = office::children(holder);
    return kids.isEmpty() ? QString() : attr(kids.first(), QStringLiteral("val"));
}

QPointF scaleOf(const QDomElement &e, const QPointF &fallback)
{
    if (e.isNull()) return fallback;
    return QPointF(office::parsePercent(attr(e, QStringLiteral("x")), fallback.x()), office::parsePercent(attr(e, QStringLiteral("y")), fallback.y()));
}

Behavior behaviorOf(const QDomElement &e, double base)
{
    Behavior b;
    const QString name = e.localName();
    const QDomElement cBhvr = pChild(e, "cBhvr");
    b.target = targetOf(cBhvr);
    b.timing = timingOf(pChild(cBhvr, "cTn"), base);
    b.attribute = pChild(pChild(cBhvr, "attrNameLst"), "attrName").text().trimmed();
    b.additive = attr(cBhvr, QStringLiteral("additive"), QStringLiteral("base"));
    if (name == QLatin1String("set")) {
        b.kind = Behavior::Kind::Set;
        b.to = valueOf(pChild(e, "to"));
    } else if (name == QLatin1String("anim")) {
        b.kind = Behavior::Kind::Animate;
        b.discrete = attr(e, QStringLiteral("calcmode")) == QLatin1String("discrete");
        b.from = attr(e, QStringLiteral("from"));
        b.to = attr(e, QStringLiteral("to"));
        b.by = attr(e, QStringLiteral("by"));
        for (const QDomElement &tav : pChildren(pChild(e, "tavLst"), "tav")) {
            Keyframe k;
            k.at = std::clamp(numberOr(attr(tav, QStringLiteral("tm")), 0) / 100000, 0.0, 1.0);
            k.value = valueOf(pChild(tav, "val"));
            b.keys.push_back(k);
        }
        std::stable_sort(b.keys.begin(), b.keys.end(), [](const Keyframe &x, const Keyframe &y) { return x.at < y.at; });
    } else if (name == QLatin1String("animEffect")) {
        b.kind = Behavior::Kind::Effect;
        b.entering = attr(e, QStringLiteral("transition"), QStringLiteral("in")) != QLatin1String("out");
        b.filter = attr(e, QStringLiteral("filter"));
    } else if (name == QLatin1String("animMotion")) {
        b.kind = Behavior::Kind::Motion;
        b.path = pathOf(attr(e, QStringLiteral("path")));
        b.origin = attr(e, QStringLiteral("origin"));
        b.pathEditMode = attr(e, QStringLiteral("pathEditMode"));
        if (b.path.isEmpty())
            if (const QDomElement by = pChild(e, "by"); !by.isNull()) {
                const QPointF to = scaleOf(by, QPointF());
                b.path.lineTo(to);
            }
    } else if (name == QLatin1String("animScale")) {
        b.kind = Behavior::Kind::Scale;
        const QDomElement from = pChild(e, "from"), to = pChild(e, "to"), by = pChild(e, "by");
        b.scaleFrom = scaleOf(from, QPointF(1, 1));
        if (!to.isNull()) b.scaleTo = scaleOf(to, QPointF(1, 1));
        else if (!by.isNull()) b.scaleTo = QPointF(b.scaleFrom.x() * scaleOf(by, QPointF(1, 1)).x(), b.scaleFrom.y() * scaleOf(by, QPointF(1, 1)).y());
        else b.scaleTo = b.scaleFrom;
    } else if (name == QLatin1String("animRot")) {
        b.kind = Behavior::Kind::Rotate;
        b.rotateFrom = office::parseAngle(attr(e, QStringLiteral("from")), 0);
        if (hasAttr(e, QStringLiteral("to"))) b.rotateTo = office::parseAngle(attr(e, QStringLiteral("to")), b.rotateFrom);
        else b.rotateTo = b.rotateFrom + office::parseAngle(attr(e, QStringLiteral("by")), 0);
    } else if (name == QLatin1String("animClr")) {
        b.kind = Behavior::Kind::Color;
    } else {
        b.kind = Behavior::Kind::Command;
        b.command = attr(e, QStringLiteral("cmd"));
    }
    return b;
}

bool isBehavior(const QDomElement &e)
{
    for (const char *name : {"set", "anim", "animEffect", "animMotion", "animScale", "animRot", "animClr", "cmd"})
        if (pIs(e, name)) return true;
    return false;
}

void collectBehaviors(const QDomElement &childTnLst, double base, Effect &effect)
{
    for (const QDomElement &c : office::children(childTnLst)) {
        if (isBehavior(c)) {
            effect.behaviors.push_back(behaviorOf(c, base));
        } else if (pIs(c, "par") || pIs(c, "seq") || pIs(c, "excl")) {
            const QDomElement cTn = pChild(c, "cTn");
            collectBehaviors(pChild(cTn, "childTnLst"), base + delayOf(cTn), effect);
        }
    }
}

EffectClass classOf(const QString &name)
{
    if (name == QLatin1String("entr")) return EffectClass::Entrance;
    if (name == QLatin1String("exit")) return EffectClass::Exit;
    if (name == QLatin1String("emph")) return EffectClass::Emphasis;
    if (name == QLatin1String("path")) return EffectClass::MotionPath;
    if (name == QLatin1String("mediacall")) return EffectClass::Media;
    if (name == QLatin1String("verb")) return EffectClass::Verb;
    return EffectClass::Other;
}

// An entrance that never says to show its shape, or an exit that never says to
// hide it, still does.
void completeVisibility(Effect &e)
{
    bool sets = false;
    const Behavior *first = nullptr;
    for (const Behavior &b : e.behaviors) {
        if (b.kind == Behavior::Kind::Set && b.attribute == QLatin1String("style.visibility")) sets = true;
        if (!first && b.target.shape > 0 && playsBehavior(b)) first = &b;
    }
    if (sets || !first || (e.cls != EffectClass::Entrance && e.cls != EffectClass::Exit)) return;
    Behavior b;
    b.kind = Behavior::Kind::Set;
    b.target = first->target;
    b.attribute = QStringLiteral("style.visibility");
    b.to = e.cls == EffectClass::Entrance ? QStringLiteral("visible") : QStringLiteral("hidden");
    b.timing.begin = e.cls == EffectClass::Entrance ? e.begin : e.end();
    b.timing.duration = 1;
    b.timing.hold = true;
    e.behaviors.push_back(b);
}

void collectEffects(const QDomElement &childTnLst, double base, QVector<Effect> &out)
{
    for (const QDomElement &par : office::children(childTnLst)) {
        if (!pIs(par, "par") && !pIs(par, "seq")) continue;
        const QDomElement cTn = pChild(par, "cTn");
        const double begin = base + delayOf(cTn);
        const QString nodeType = attr(cTn, QStringLiteral("nodeType"));
        if (hasAttr(cTn, QStringLiteral("presetClass")) || nodeType == QLatin1String("clickEffect") || nodeType == QLatin1String("withEffect") ||
            nodeType == QLatin1String("afterEffect")) {
            Effect e;
            e.cls = classOf(attr(cTn, QStringLiteral("presetClass")));
            e.start = nodeType == QLatin1String("withEffect") ? EffectStart::WithPrevious
                      : nodeType == QLatin1String("afterEffect") ? EffectStart::AfterPrevious
                                                                  : EffectStart::OnClick;
            e.presetId = int(office::parseInt(attr(cTn, QStringLiteral("presetID")), 0));
            e.presetSubtype = int(office::parseInt(attr(cTn, QStringLiteral("presetSubtype")), 0));
            e.begin = begin;
            collectBehaviors(pChild(cTn, "childTnLst"), begin, e);
            // What the effect says of its whole run: it plays that many times, takes its work with it when it
            // ends, and speeds up and slows down (each behavior does, unless it says otherwise).
            const QString count = attr(cTn, QStringLiteral("repeatCount"));
            e.repeat = count.isEmpty() || count == QLatin1String("indefinite") ? 1 : std::clamp(numberOr(count, 1000) / 1000, 1.0, 1000.0);
            e.remove = attr(cTn, QStringLiteral("fill")) == QLatin1String("remove");
            const double accel = numberOr(attr(cTn, QStringLiteral("accel")), 0) / 100000, decel = numberOr(attr(cTn, QStringLiteral("decel")), 0) / 100000;
            if (accel > 0 || decel > 0)
                for (Behavior &b : e.behaviors)
                    if (b.timing.accel == 0 && b.timing.decel == 0 && b.timing.duration > 1) {
                        b.timing.accel = accel;
                        b.timing.decel = decel;
                    }
            completeVisibility(e);
            out.push_back(e);
        } else {
            collectEffects(pChild(cTn, "childTnLst"), begin, out);
        }
    }
}

void readSequence(const QDomElement &seq, Timeline &tl)
{
    const QDomElement cTn = pChild(seq, "cTn");
    const QString nodeType = attr(cTn, QStringLiteral("nodeType"));
    const QDomElement list = pChild(cTn, "childTnLst");
    if (nodeType == QLatin1String("mainSeq")) {
        if (!tl.steps.isEmpty()) return;
        for (const QDomElement &par : pChildren(list, "par")) {
            const QDomElement stepNode = pChild(par, "cTn");
            Step step;
            for (const QDomElement &cond : pChildren(pChild(stepNode, "stCondLst"), "cond"))
                if (attr(cond, QStringLiteral("evt")) == QLatin1String("onBegin")) step.automatic = true;
            collectEffects(pChild(stepNode, "childTnLst"), 0, step.effects);
            if (step.effects.isEmpty()) continue;
            // With nothing before it, an effect that goes with or after the previous one goes by itself.
            if (tl.steps.isEmpty() && step.effects.first().start != EffectStart::OnClick) step.automatic = true;
            tl.steps.push_back(step);
        }
    } else if (nodeType == QLatin1String("interactiveSeq")) {
        Trigger trigger;
        for (const QDomElement &cond : pChildren(pChild(cTn, "stCondLst"), "cond"))
            if (const QDomElement sp = pChild(pChild(cond, "tgtEl"), "spTgt"); !sp.isNull())
                trigger.shape = int(office::parseInt(attr(sp, QStringLiteral("spid")), 0));
        for (const QDomElement &par : pChildren(list, "par")) collectEffects(pChild(pChild(par, "cTn"), "childTnLst"), 0, trigger.effects);
        if (!trigger.effects.isEmpty()) tl.triggers.push_back(trigger);
    }
}

void findSequences(const QDomElement &e, Timeline &tl)
{
    for (const QDomElement &c : office::children(e)) {
        if (pIs(c, "seq")) readSequence(c, tl);
        else findSequences(c, tl);
    }
}

} // namespace

Timeline readTimeline(const QString &timingXml)
{
    Timeline tl;
    QDomDocument dom;
    if (timingXml.isEmpty() || !office::parseXml(timingXml.toUtf8(), &dom)) return tl;
    const QDomElement root = dom.documentElement();
    if (!pIs(root, "timing")) return tl;
    findSequences(root, tl);
    return tl;
}

Timeline timelineOf(const Page &slide) { return readTimeline(slide.props.value(QStringLiteral("timingXml")).toString()); }

Timeline playableTimeline(const Page &slide)
{
    Timeline tl = timelineOf(slide);
    tl.steps.erase(std::remove_if(tl.steps.begin(), tl.steps.end(), [](const Step &s) { return !s.visual(); }), tl.steps.end());
    return tl;
}

// ---- formulas ----

namespace {

// Real formulas are a line long and nest a few levels; a damaged file's can
// nest thousands deep, which would run the parser out of stack.
constexpr qsizetype kMaxFormulaLength = 4096;
constexpr int kMaxFormulaDepth = 64;

class Formula {
public:
    Formula(const QString &text, const QHash<QString, double> &variables) : m_s(text), m_vars(variables) {}
    bool evaluate(double *out)
    {
        if (m_s.size() > kMaxFormulaLength || !expression(out)) return false;
        skip();
        return m_i == m_s.size() && std::isfinite(*out);
    }

private:
    void skip()
    {
        while (m_i < m_s.size() && m_s[m_i].isSpace()) ++m_i;
    }
    bool peek(QChar c)
    {
        skip();
        return m_i < m_s.size() && m_s[m_i] == c;
    }
    bool expression(double *v)
    {
        if (!term(v)) return false;
        for (;;) {
            double r = 0;
            if (peek(QLatin1Char('+'))) {
                ++m_i;
                if (!term(&r)) return false;
                *v += r;
            } else if (peek(QLatin1Char('-'))) {
                ++m_i;
                if (!term(&r)) return false;
                *v -= r;
            } else {
                return true;
            }
        }
    }
    bool term(double *v)
    {
        if (!unary(v)) return false;
        for (;;) {
            double r = 0;
            if (peek(QLatin1Char('*'))) {
                ++m_i;
                if (!unary(&r)) return false;
                *v *= r;
            } else if (peek(QLatin1Char('/'))) {
                ++m_i;
                if (!unary(&r) || r == 0) return false;
                *v /= r;
            } else {
                return true;
            }
        }
    }
    // Counts the levels a sign or a parenthesis opens, while one is read.
    struct Level {
        explicit Level(int &depth) : m_depth(++depth) {}
        ~Level() { --m_depth; }
        int &m_depth;
    };
    bool unary(double *v)
    {
        const Level level(m_depth);
        if (m_depth > kMaxFormulaDepth) return false;
        if (peek(QLatin1Char('-'))) {
            ++m_i;
            if (!unary(v)) return false;
            *v = -*v;
            return true;
        }
        if (peek(QLatin1Char('+'))) {
            ++m_i;
            return unary(v);
        }
        return primary(v);
    }
    bool primary(double *v)
    {
        skip();
        if (m_i >= m_s.size()) return false;
        if (m_s[m_i] == QLatin1Char('(')) {
            ++m_i;
            if (!expression(v) || !peek(QLatin1Char(')'))) return false;
            ++m_i;
            return true;
        }
        if (m_s[m_i] == QLatin1Char('#')) {
            const qsizetype start = ++m_i;
            while (m_i < m_s.size() && (m_s[m_i].isLetterOrNumber() || m_s[m_i] == QLatin1Char('_') || m_s[m_i] == QLatin1Char('.'))) ++m_i;
            const auto found = m_vars.constFind(m_s.mid(start, m_i - start));
            if (found == m_vars.constEnd()) return false;
            *v = found.value();
            return true;
        }
        // A name without the sign (an exit's formulas write ppt_x, ppt_h, and the like so).
        if (m_s[m_i].isLetter() || m_s[m_i] == QLatin1Char('_')) {
            const qsizetype first = m_i;
            while (m_i < m_s.size() && (m_s[m_i].isLetterOrNumber() || m_s[m_i] == QLatin1Char('_') || m_s[m_i] == QLatin1Char('.'))) ++m_i;
            const auto found = m_vars.constFind(m_s.mid(first, m_i - first));
            if (found == m_vars.constEnd()) return false;
            *v = found.value();
            return true;
        }
        const qsizetype start = m_i;
        while (m_i < m_s.size() && (m_s[m_i].isDigit() || m_s[m_i] == QLatin1Char('.'))) ++m_i;
        if (m_i < m_s.size() && (m_s[m_i] == QLatin1Char('e') || m_s[m_i] == QLatin1Char('E'))) {
            qsizetype j = m_i + 1;
            if (j < m_s.size() && (m_s[j] == QLatin1Char('+') || m_s[j] == QLatin1Char('-'))) ++j;
            if (j < m_s.size() && m_s[j].isDigit()) {
                while (j < m_s.size() && m_s[j].isDigit()) ++j;
                m_i = j;
            }
        }
        bool ok = false;
        *v = m_s.mid(start, m_i - start).toDouble(&ok);
        return ok;
    }
    QString m_s;
    const QHash<QString, double> &m_vars;
    qsizetype m_i = 0;
    int m_depth = 0;
};

} // namespace

double evaluateFormula(const QString &formula, const QHash<QString, double> &variables, bool *ok)
{
    double v = 0;
    const bool good = Formula(formula, variables).evaluate(&v);
    if (ok) *ok = good;
    return good ? v : std::numeric_limits<double>::quiet_NaN();
}

// ---- where the shapes are ----

Geometry geometryOf(const Document &doc, const Page &slide)
{
    Geometry g;
    g.slide = doc.pageSize();
    walkItems(slide.items, [&](const ItemPtr &it) {
        if (it->sourceId > 0) g.shapes.insert(it->sourceId, it->type() == ItemType::Group ? it->bounds() : it->rect);
    });
    return g;
}

// ---- the state at a moment ----

namespace {

const QRectF kWhole(0, 0, 1, 1);

QRectF intersectClips(const QRectF &a, const QRectF &b)
{
    const double l = std::max(a.left(), b.left()), t = std::max(a.top(), b.top());
    const double r = std::min(a.right(), b.right()), bt = std::min(a.bottom(), b.bottom());
    const QRectF out(l, t, std::max(0.0, r - l), std::max(0.0, bt - t));
    return out.isNull() ? QRectF(0, 0, 0, 1) : out;   // null means no clip, so a clip that shows nothing is empty but not null
}

void clipTo(TargetState &s, const QRectF &clip)
{
    s.clip = s.clip.isNull() ? clip : intersectClips(s.clip, clip);
    if (!s.clip2.isNull()) s.clip2 = intersectClips(s.clip2, clip);
}

// Two parts shown (a split): where the shape is clipped already, both are cut to it.
void clipToTwo(TargetState &s, const QRectF &a, const QRectF &b)
{
    if (s.clip.isNull()) {
        s.clip = a;
        s.clip2 = b;
    } else {
        const QRectF was = s.clip;
        s.clip = intersectClips(was, a);
        s.clip2 = intersectClips(was, b);
    }
}

QHash<QString, double> variablesOf(const Geometry &g, const Target &t)
{
    const QRectF r = g.shapes.value(t.shape);
    const double w = g.slide.width() > 0 ? g.slide.width() : 1, h = g.slide.height() > 0 ? g.slide.height() : 1;
    return {{QStringLiteral("ppt_x"), r.center().x() / w}, {QStringLiteral("ppt_y"), r.center().y() / h}, {QStringLiteral("ppt_w"), r.width() / w},
            {QStringLiteral("ppt_h"), r.height() / h}, {QStringLiteral("style.rotation"), 0},       {QStringLiteral("style.opacity"), 1},
            {QStringLiteral("style.visibility"), 1}};
}

// The value of an Animate behavior `p` of the way through a run, where `base` is what the shape has now.
double animatedValue(const Behavior &b, double p, const QHash<QString, double> &vars, double base)
{
    auto value = [&](const QString &text) {
        bool ok = false;
        const double v = evaluateFormula(text, vars, &ok);
        return ok ? v : base;
    };
    if (!b.keys.isEmpty()) {
        const QVector<Keyframe> &k = b.keys;
        if (p <= k.first().at) return value(k.first().value);
        if (p >= k.last().at) return value(k.last().value);
        for (qsizetype i = 1; i < k.size(); ++i) {
            if (p > k[i].at) continue;
            const double a = value(k[i - 1].value);
            if (b.discrete) return a;
            const double span = k[i].at - k[i - 1].at;
            return span > 0 ? a + (value(k[i].value) - a) * (p - k[i - 1].at) / span : a;
        }
        return value(k.last().value);
    }
    const double from = b.from.isEmpty() ? base : value(b.from);
    const double to = !b.to.isEmpty() ? value(b.to) : !b.by.isEmpty() ? from + value(b.by) : base;
    return from + (to - from) * p;
}

// One attribute taking a value: the shape's middle (as a share of the slide), its size, its turn, or its opacity.
void setAttribute(TargetState &s, const Behavior &b, const QString &attribute, double v, const Geometry &g)
{
    const QRectF r = g.shapes.value(b.target.shape);
    const double w = g.slide.width() > 0 ? g.slide.width() : 1, h = g.slide.height() > 0 ? g.slide.height() : 1;
    const bool sum = b.additive == QLatin1String("sum"), mult = b.additive == QLatin1String("mult");
    if (attribute == QLatin1String("ppt_x")) {
        s.offset.setX(sum ? s.offset.x() + v * w : (v - r.center().x() / w) * w);
    } else if (attribute == QLatin1String("ppt_y")) {
        s.offset.setY(sum ? s.offset.y() + v * h : (v - r.center().y() / h) * h);
    } else if (attribute == QLatin1String("ppt_w")) {
        const double natural = r.width() / w;
        if (natural > 0) s.scaleX = std::max(0.0, sum ? (natural + v) / natural : mult ? s.scaleX * v : v / natural);
    } else if (attribute == QLatin1String("ppt_h")) {
        const double natural = r.height() / h;
        if (natural > 0) s.scaleY = std::max(0.0, sum ? (natural + v) / natural : mult ? s.scaleY * v : v / natural);
    } else if (attribute == QLatin1String("style.rotation")) {
        s.rotation = sum ? s.rotation + v : v;
    } else if (attribute == QLatin1String("style.opacity")) {
        s.opacity = std::clamp(sum ? s.opacity + v : mult ? s.opacity * v : v, 0.0, 1.0);
    }
}

double natural(const QString &attribute, const QHash<QString, double> &vars) { return vars.value(attribute, 0); }

// The part of the shape an entering wipe has uncovered, or an exiting one has left, `p` of the way.
QRectF wipeClip(const QString &direction, bool entering, double p)
{
    if (direction == QLatin1String("left")) return entering ? QRectF(0, 0, p, 1) : QRectF(p, 0, 1 - p, 1);
    if (direction == QLatin1String("right")) return entering ? QRectF(1 - p, 0, p, 1) : QRectF(0, 0, 1 - p, 1);
    if (direction == QLatin1String("up")) return entering ? QRectF(0, 0, 1, p) : QRectF(0, p, 1, 1 - p);
    return entering ? QRectF(0, 1 - p, 1, p) : QRectF(0, 0, 1, 1 - p);   // down
}

// The parts of the shape a split has uncovered, `q` of the way: two strips closing in from the edges, or one opening from the middle.
// An exit's `q` is how much is left.
void barnClip(TargetState &s, const QString &way, double q)
{
    const bool vertical = way.endsWith(QLatin1String("Vertical"));
    if (way.startsWith(QLatin1String("in"))) {
        if (vertical) clipToTwo(s, QRectF(0, 0, q / 2, 1), QRectF(1 - q / 2, 0, q / 2, 1));
        else clipToTwo(s, QRectF(0, 0, 1, q / 2), QRectF(0, 1 - q / 2, 1, q / 2));
    } else {
        clipTo(s, vertical ? QRectF(0.5 - q / 2, 0, q, 1) : QRectF(0, 0.5 - q / 2, 1, q));
    }
}

void applyBehavior(TargetState &s, const Behavior &b, const Geometry &g, double ms)
{
    double p = 0;
    const Timing::Phase phase = b.timing.phaseAt(ms, &p);
    if (phase == Timing::Phase::Before) return;
    if (phase == Timing::Phase::After && b.timing.duration > 0 && !b.timing.hold) return;   // it ended and took its work with it
    switch (b.kind) {
    case Behavior::Kind::Set:
        if (b.attribute == QLatin1String("style.visibility")) {
            s.visible = b.to == QLatin1String("visible") || b.to == QLatin1String("true") || b.to == QLatin1String("1");
        } else if (playsAttribute(b.attribute)) {
            bool ok = false;
            const double v = b.to.toDouble(&ok);
            if (ok) setAttribute(s, b, b.attribute, v, g);
        }
        break;
    case Behavior::Kind::Animate: {
        if (!playsAttribute(b.attribute) || b.attribute == QLatin1String("style.visibility")) break;
        const QHash<QString, double> vars = variablesOf(g, b.target);
        setAttribute(s, b, b.attribute, animatedValue(b, p, vars, b.additive == QLatin1String("sum") ? 0 : natural(b.attribute, vars)), g);
        break;
    }
    case Behavior::Kind::Effect: {
        static const QRegularExpression wipe(QStringLiteral("^wipe\\((left|right|up|down)\\)$"));
        static const QRegularExpression barn(QStringLiteral("^barn\\((in|out)(Vertical|Horizontal)\\)$"));
        if (const QRegularExpressionMatch m = wipe.match(b.filter); m.hasMatch()) clipTo(s, wipeClip(m.captured(1), b.entering, p));
        else if (const QRegularExpressionMatch n = barn.match(b.filter); n.hasMatch()) barnClip(s, n.captured(1) + n.captured(2), b.entering ? p : 1 - p);
        else s.opacity *= b.entering ? p : 1 - p;   // a fade, and every filter drawn as one
        break;
    }
    case Behavior::Kind::Motion:
        if (!b.path.isEmpty()) {
            const QPointF at = QTransform::fromScale(g.slide.width(), g.slide.height()).map(b.path).pointAtPercent(p);
            s.offset += at;
        }
        break;
    case Behavior::Kind::Scale:
        s.scaleX *= std::max(0.0, b.scaleFrom.x() + (b.scaleTo.x() - b.scaleFrom.x()) * p);
        s.scaleY *= std::max(0.0, b.scaleFrom.y() + (b.scaleTo.y() - b.scaleFrom.y()) * p);
        break;
    case Behavior::Kind::Rotate:
        s.rotation += b.rotateFrom + (b.rotateTo - b.rotateFrom) * p;
        break;
    default:
        break;
    }
}

void applyStep(States &states, const Step &step, const Geometry &g, double ms)
{
    for (const Effect &e : step.effects) {
        // An effect that plays again starts over each time; one that takes its work with it leaves when it ends.
        double t = ms;
        const double once = e.once();
        if (once > 0 && (e.repeat > 1 || e.remove)) {
            const double local = ms - e.begin;
            if (local >= once * std::max(1.0, e.repeat)) {
                if (e.remove) continue;
                t = e.begin + once;
            } else if (local >= 0) {
                t = e.begin + std::fmod(local, once);
            }
        }
        for (const Behavior &b : e.behaviors)
            if (b.target.shape > 0 && playsBehavior(b)) applyBehavior(states[b.target], b, g, t);
    }
}

// Entrances start hidden; the first effect a target has decides.
States initialStates(const Timeline &tl)
{
    States states;
    for (const Step &step : tl.steps)
        for (const Effect &e : step.effects)
            for (const Behavior &b : e.behaviors) {
                if (b.target.shape <= 0 || !playsBehavior(b) || states.contains(b.target)) continue;
                TargetState s;
                s.visible = e.cls != EffectClass::Entrance;
                states.insert(b.target, s);
            }
    return states;
}

} // namespace

States statesAt(const Timeline &timeline, const Geometry &geometry, int done, double ms)
{
    States states = initialStates(timeline);
    const int n = int(timeline.steps.size());
    for (int k = 0; k < std::min(done, n); ++k) applyStep(states, timeline.steps[k], geometry, std::numeric_limits<double>::infinity());
    if (ms >= 0 && done >= 0 && done < n) applyStep(states, timeline.steps[done], geometry, ms);
    return states;
}

// ---- painting a page with the states ----

namespace {

using Draw = std::function<void(QPainter *)>;

void combine(TargetState &a, const TargetState &b)
{
    a.visible = a.visible && b.visible;
    a.opacity *= b.opacity;
    a.offset += b.offset;
    a.scaleX *= b.scaleX;
    a.scaleY *= b.scaleY;
    a.rotation += b.rotation;
    if (!b.clip.isNull()) {
        if (b.clip2.isNull()) clipTo(a, b.clip);
        else clipToTwo(a, b.clip, b.clip2);
    }
}

bool turns(const TargetState &s) { return !s.offset.isNull() || s.scaleX != 1 || s.scaleY != 1 || s.rotation != 0; }

// The turn, scale, and move about `middle` (the painter's coordinates).
QTransform movedAbout(const TargetState &s, const QPointF &middle)
{
    QTransform t;
    t.translate(middle.x() + s.offset.x(), middle.y() + s.offset.y());
    t.rotate(s.rotation);
    t.scale(s.scaleX, s.scaleY);
    t.translate(-middle.x(), -middle.y());
    return t;
}

QRectF clipRectIn(const QRectF &box, const QRectF &share)
{
    return QRectF(box.x() + box.width() * share.x(), box.y() + box.height() * share.y(), box.width() * share.width(), box.height() * share.height());
}

// The part of `box` the state shows: one rectangle, or two.
void setClip(QPainter *p, const QRectF &box, const TargetState &s)
{
    if (s.clip2.isNull()) {
        p->setClipRect(clipRectIn(box, s.clip), Qt::IntersectClip);
        return;
    }
    QPainterPath parts;
    parts.addRect(clipRectIn(box, s.clip));
    parts.addRect(clipRectIn(box, s.clip2));
    p->setClipPath(parts, Qt::IntersectClip);
}

// An object and what is in it that has a number, as the states have them.
template <typename F>
void forEachNumber(const Item &it, F &&fn)
{
    if (it.sourceId > 0) fn(it.sourceId);
    if (it.type() == ItemType::Group)
        for (const ItemPtr &c : static_cast<const GroupItem &>(it).children) forEachNumber(*c, fn);
}

QString storyOf(const Item &it)
{
    if (it.type() == ItemType::Text) return static_cast<const TextItem &>(it).storyId;
    if (it.type() == ItemType::Shape) return static_cast<const ShapeItem &>(it).storyId;
    return QString();
}

class Overlay {
public:
    Overlay(const Document *doc, const States &states) : m_doc(doc), m_states(states)
    {
        for (auto it = states.constBegin(); it != states.constEnd(); ++it)
            if (it.key().paragraphs()) m_paragraphs[it.key().shape].push_back(it.key());
        for (auto &list : m_paragraphs)
            std::sort(list.begin(), list.end(), [](const Target &a, const Target &b) { return a.firstParagraph < b.firstParagraph; });
    }

    void paintItem(QPainter *p, const Item &it, const Draw &draw)
    {
        m_page = p->worldTransform();
        m_device = p->device();
        TargetState s;
        bool touched = false;
        forEachNumber(it, [&](int id) {
            const auto found = m_states.constFind(Target{id, -1, -1});
            if (found == m_states.constEnd()) return;
            combine(s, found.value());
            touched = true;
        });
        m_active = true;
        if (!touched) {
            draw(p);
        } else if (s.visible && s.opacity > 0 && s.scaleX != 0 && s.scaleY != 0) {
            const QRectF box = it.type() == ItemType::Group ? it.bounds() : it.rect;
            p->save();
            p->setTransform(movedAbout(s, box.center()), true);
            if (!s.clip.isNull()) setClip(p, box, s);
            if (s.opacity < 1) drawFaded(p, box, s.opacity, draw);
            else draw(p);
            p->restore();
        }
        m_active = false;
    }

    // The text of an object with paragraphs for targets: the lines that are not
    // targets, and then each target's lines, in the state it has.
    void paintText(QPainter *p, const Item &it, const StoryLayout &layout, int frame, const Draw &paintText)
    {
        const auto keys = m_active && it.sourceId > 0 ? m_paragraphs.constFind(it.sourceId) : m_paragraphs.constEnd();
        const QTextDocument *story = keys == m_paragraphs.constEnd() ? nullptr : m_doc->storyDoc(storyOf(it));
        if (!story) {
            paintText(p);
            return;
        }
        // Where each paragraph's lines are in this frame, and the band of the frame it owns: from halfway
        // to the paragraph before to halfway to the one after, so that the bands cover the frame.
        QVector<QRectF> boxes;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            QRectF box;
            for (const QRectF &r : layout.rangeRects(frame, b.position(), b.position() + std::max(0, b.length() - 1))) box = box.isNull() ? r : box.united(r);
            boxes.push_back(box);
        }
        QVector<int> present;
        for (int i = 0; i < boxes.size(); ++i)
            if (!boxes[i].isNull()) present.push_back(i);
        const QTransform w = p->worldTransform();
        auto snap = [&](double y) {
            if (w.type() > QTransform::TxScale || w.m22() == 0) return y;
            return (std::round(w.m22() * y + w.dy()) - w.dy()) / w.m22();
        };
        constexpr double kFar = 1e5;
        QVector<QPair<double, double>> band(boxes.size(), QPair<double, double>(0, 0));
        for (qsizetype k = 0; k < present.size(); ++k) {
            const QRectF &box = boxes[present[k]];
            const double top = k == 0 ? -kFar : std::min(box.top(), (boxes[present[k - 1]].bottom() + box.top()) / 2);
            const double bottom = k + 1 == present.size() ? kFar : std::max(box.bottom(), (box.bottom() + boxes[present[k + 1]].top()) / 2);
            band[present[k]] = {k == 0 ? top : snap(top), k + 1 == present.size() ? bottom : snap(bottom)};
        }
        // Each target owns the paragraphs in its range that no target before it took.
        struct Owned { Target target; QVector<QPair<double, double>> bands; QRectF box; };
        QVector<Owned> owned;
        QVector<bool> taken(boxes.size(), false);
        for (const Target &key : keys.value()) {
            Owned o;
            o.target = key;
            for (int i = key.firstParagraph; i <= key.lastParagraph && i < boxes.size(); ++i) {
                if (taken[i] || boxes[i].isNull()) continue;
                taken[i] = true;
                o.bands.push_back(band[i]);
                o.box = o.box.isNull() ? boxes[i] : o.box.united(boxes[i]);
            }
            if (!o.bands.isEmpty()) owned.push_back(o);
        }
        // What is left: the strips between the bands that are targets' (the bands that are next to each other are merged).
        QVector<QPair<double, double>> all;
        for (const Owned &o : owned) all += o.bands;
        std::sort(all.begin(), all.end());
        QPainterPath rest;
        double from = -kFar;
        for (const auto &b : all) {
            if (b.first > from) rest.addRect(QRectF(-kFar, from, 2 * kFar, b.first - from));
            from = std::max(from, b.second);
        }
        if (from < kFar) rest.addRect(QRectF(-kFar, from, 2 * kFar, kFar - from));
        p->save();
        p->setClipPath(rest, Qt::IntersectClip);
        paintText(p);
        p->restore();
        for (const Owned &o : owned) {
            const TargetState &s = m_states.value(o.target);
            if (!s.visible || s.opacity <= 0 || s.scaleX == 0 || s.scaleY == 0) continue;
            p->save();
            if (turns(s) && p->device() == m_device && m_page.isInvertible()) {
                // The move is on the slide: from the text's place to the slide and back to the painter's.
                const QTransform toSlide = w * m_page.inverted();
                const QPointF middle = toSlide.map(o.box.center());
                p->setWorldTransform(toSlide * movedAbout(s, middle) * m_page);
            }
            QPainterPath mine;
            for (const auto &b : o.bands) mine.addRect(QRectF(-kFar, b.first, 2 * kFar, b.second - b.first));
            p->setClipPath(mine, Qt::IntersectClip);
            if (!s.clip.isNull()) setClip(p, o.box, s);
            if (s.opacity < 1) p->setOpacity(p->opacity() * s.opacity);
            paintText(p);
            p->restore();
        }
    }

private:
    // The object painted into a layer and the layer painted half see-through, so
    // that what is in the object does not show through itself.
    void drawFaded(QPainter *p, const QRectF &box, double opacity, const Draw &draw)
    {
        const QPaintDevice *device = p->device();
        const qreal ratio = device->devicePixelRatioF();
        const QTransform w = p->worldTransform();
        // The part of the picture the object can reach: its box and room around it for shadows and glows.
        QRectF reach = w.mapRect(box.adjusted(-60, -60, 60, 60)).intersected(QRectF(0, 0, device->width() / ratio, device->height() / ratio));
        const QRectF clipped = p->clipBoundingRect();
        if (!clipped.isEmpty()) reach = reach.intersected(w.mapRect(clipped));
        const QRect area = reach.toAlignedRect();
        if (area.isEmpty()) return;
        QImage layer(QSize(int(std::ceil(area.width() * ratio)), int(std::ceil(area.height() * ratio))), QImage::Format_ARGB32_Premultiplied);
        layer.setDevicePixelRatio(ratio);
        layer.fill(Qt::transparent);
        {
            QPainter lp(&layer);
            lp.setRenderHints(p->renderHints());
            lp.translate(-area.topLeft());
            lp.setTransform(w, true);
            const QTransform savedPage = m_page;
            const QPaintDevice *savedDevice = m_device;
            m_page = savedPage * QTransform::fromTranslate(-area.x(), -area.y());
            m_device = &layer;
            draw(&lp);
            m_page = savedPage;
            m_device = savedDevice;
        }
        p->save();
        p->setWorldTransform(QTransform());
        p->setOpacity(p->opacity() * opacity);
        p->drawImage(area.topLeft(), layer);
        p->restore();
    }

    const Document *m_doc;
    States m_states;
    QHash<int, QVector<Target>> m_paragraphs;   // by shape number, the paragraph targets, first paragraph first
    QTransform m_page;                          // from the page to the device, as the painter had it when the object began
    const QPaintDevice *m_device = nullptr;
    bool m_active = false;                      // painting one of the page's own objects
};

} // namespace

void animate(PaintContext &ctx, const Document &doc, const States &states)
{
    auto overlay = std::make_shared<Overlay>(&doc, states);
    ctx.around = [overlay](QPainter *p, const Item &it, const Draw &draw) { overlay->paintItem(p, it, draw); };
    ctx.aroundText = [overlay](QPainter *p, const Item &it, const StoryLayout &layout, int frame, const Draw &paintText) {
        overlay->paintText(p, it, layout, frame, paintText);
    };
}

} // namespace jp::deck
