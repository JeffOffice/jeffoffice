// Making and changing a slide's animations: the kinds of effect JeffDeck offers,
// reading the main sequence into effects an animation pane can list, and
// writing it back as the presentation program writes it (core/animation.h,
// "Making and changing effects").

#include "core/animation.h"
#include "core/animation_presets.h"
#include "core/deck.h"
#include "office/xml.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QJsonArray>
#include <QRegularExpression>
#include <QSet>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextList>
#include <algorithm>
#include <cmath>

namespace jp::deck {

// ---- the kinds ----

namespace {

#define A_(text) QT_TRANSLATE_NOOP("Animation", text)

struct VariantDef {
    const char *key, *name;
    int preset, subtype;
};
struct KindDef {
    const char *id, *name;
    EffectClass cls;
    const char *group;
    int duration;
    EffectKind::Amount amount;
    bool smooth;
    std::vector<VariantDef> variants;
};

const EffectClass kEntrance = EffectClass::Entrance, kExit = EffectClass::Exit, kEmphasis = EffectClass::Emphasis, kPath = EffectClass::MotionPath;
using Amount = EffectKind::Amount;

std::vector<VariantDef> flyVariants()
{
    return {{"bottom", A_("From Bottom"), 2, 4},       {"left", A_("From Left"), 2, 8},         {"top", A_("From Top"), 2, 1},
            {"right", A_("From Right"), 2, 2},         {"bottomLeft", A_("From Bottom-Left"), 2, 12}, {"topLeft", A_("From Top-Left"), 2, 9},
            {"topRight", A_("From Top-Right"), 2, 3},  {"bottomRight", A_("From Bottom-Right"), 2, 6}};
}
std::vector<VariantDef> wipeVariants()
{
    return {{"bottom", A_("From Bottom"), 22, 4}, {"left", A_("From Left"), 22, 8}, {"top", A_("From Top"), 22, 1}, {"right", A_("From Right"), 22, 2}};
}
std::vector<VariantDef> splitVariants()
{
    return {{"verticalIn", A_("Vertical In"), 16, 21}, {"verticalOut", A_("Vertical Out"), 16, 37}, {"horizontalIn", A_("Horizontal In"), 16, 26},
            {"horizontalOut", A_("Horizontal Out"), 16, 42}};
}
std::vector<VariantDef> floatVariants() { return {{"up", A_("Float Up"), 42, 0}, {"down", A_("Float Down"), 47, 0}}; }

// The motion paths by the number the presentation program gives each.
struct PathName {
    int preset;
    const char *name;
};
std::vector<VariantDef> pathVariants(std::initializer_list<PathName> list)
{
    std::vector<VariantDef> out;
    for (const PathName &p : list) out.push_back({nullptr, p.name, p.preset, 0});   // a path's key is its preset's number, "p42"
    return out;
}

const std::vector<KindDef> &kindDefs()
{
    static const std::vector<KindDef> defs = [] {
        std::vector<KindDef> d;
        const auto none = Amount::None;
        d.push_back({"entr.appear", A_("Appear"), kEntrance, "", 0, none, false, {{"none", A_("Appear"), 1, 0}}});
        d.push_back({"entr.fade", A_("Fade"), kEntrance, "", 500, none, false, {{"none", A_("Fade"), 10, 0}}});
        d.push_back({"entr.fly", A_("Fly In"), kEntrance, "", 500, none, false, flyVariants()});
        d.push_back({"entr.float", A_("Float In"), kEntrance, "", 1000, none, false, floatVariants()});
        d.push_back({"entr.split", A_("Split"), kEntrance, "", 500, none, false, splitVariants()});
        d.push_back({"entr.wipe", A_("Wipe"), kEntrance, "", 500, none, false, wipeVariants()});
        d.push_back({"entr.growturn", A_("Grow & Turn"), kEntrance, "", 1000, none, false, {{"none", A_("Grow & Turn"), 31, 0}}});
        d.push_back({"entr.zoom", A_("Zoom"), kEntrance, "", 500, none, false, {{"object", A_("Object Center"), 53, 16}, {"slide", A_("Slide Center"), 53, 528}}});

        d.push_back({"emph.pulse", A_("Pulse"), kEmphasis, "", 500, none, false, {{"none", A_("Pulse"), 26, 0}}});
        d.push_back({"emph.teeter", A_("Teeter"), kEmphasis, "", 1000, none, false, {{"none", A_("Teeter"), 32, 0}}});
        d.push_back({"emph.spin", A_("Spin"), kEmphasis, "", 2000, Amount::Spin, false, {{"cw", A_("Clockwise"), 8, 0}, {"ccw", A_("Counterclockwise"), 8, 0}}});
        d.push_back({"emph.growshrink", A_("Grow/Shrink"), kEmphasis, "", 2000, Amount::Size, false,
                     {{"both", A_("Both"), 6, 0}, {"horizontal", A_("Horizontal"), 6, 0}, {"vertical", A_("Vertical"), 6, 0}}});

        d.push_back({"exit.disappear", A_("Disappear"), kExit, "", 0, none, false, {{"none", A_("Disappear"), 1, 0}}});
        d.push_back({"exit.fade", A_("Fade"), kExit, "", 500, none, false, {{"none", A_("Fade"), 10, 0}}});
        d.push_back({"exit.fly", A_("Fly Out"), kExit, "", 500, none, false, flyVariants()});
        d.push_back({"exit.float", A_("Float Out"), kExit, "", 1000, none, false, floatVariants()});
        d.push_back({"exit.split", A_("Split"), kExit, "", 500, none, false, splitVariants()});
        d.push_back({"exit.wipe", A_("Wipe"), kExit, "", 500, none, false, wipeVariants()});
        d.push_back({"exit.growturn", A_("Shrink & Turn"), kExit, "", 1000, none, false, {{"none", A_("Shrink & Turn"), 31, 0}}});
        d.push_back({"exit.zoom", A_("Zoom"), kExit, "", 500, none, false, {{"object", A_("Object Center"), 53, 32}}});

        d.push_back({"path.lines", A_("Lines"), kPath, "Lines", 2000, none, true,
                     pathVariants({{42, A_("Down")}, {63, A_("Right")}, {35, A_("Left")}, {64, A_("Up")}, {49, A_("Diagonal Down Right")}, {56, A_("Diagonal Up Right")}})});
        d.push_back({"path.arcs", A_("Arcs"), kPath, "Arcs", 2000, none, true, pathVariants({{37, A_("Down")}, {51, A_("Left")}, {58, A_("Right")}, {44, A_("Up")}})});
        d.push_back({"path.turns", A_("Turns"), kPath, "Turns", 2000, none, true,
                     pathVariants({{36, A_("Down then Right")}, {50, A_("Right then Down")}, {57, A_("Up then Right")}, {43, A_("Right then Up")}})});
        d.push_back({"path.shapes", A_("Shapes"), kPath, "Shapes", 2000, none, true,
                     pathVariants({{1, A_("Circle")},         {3, A_("Diamond")},        {4, A_("Hexagon")},         {10, A_("Octagon")},         {15, A_("Pentagon")},
                                   {7, A_("Square")},         {13, A_("Equal Triangle")}, {2, A_("Right Triangle")},  {8, A_("Trapezoid")},        {14, A_("Parallelogram")},
                                   {12, A_("Football")},      {18, A_("Teardrop")},      {9, A_("Heart")},           {6, A_("Crescent Moon")},    {16, A_("4 Point Star")},
                                   {5, A_("5 Point Star")},   {11, A_("6 Point Star")},  {17, A_("8 Point Star")}})});
        d.push_back({"path.loops", A_("Loops"), kPath, "Loops", 2000, none, true,
                     pathVariants({{24, A_("Horizontal Figure 8")}, {22, A_("Vertical Figure 8")}, {26, A_("Two Circles")}, {28, A_("Figure 8 Four")},
                                   {48, A_("Loop de Loop")},        {61, A_("Loop de Loop Reverse")}, {46, A_("Spiral Left")}, {55, A_("Spiral Right")},
                                   {52, A_("Funnel")},              {53, A_("Spring")}})});
        d.push_back({"path.curves", A_("Curves"), kPath, "Curves", 2000, none, true,
                     pathVariants({{38, A_("Zigzag")},      {39, A_("S Curve 1")},    {59, A_("S Curve 2")},     {40, A_("Sine Wave")},     {60, A_("Decaying Wave")},
                                   {47, A_("Wave")},        {41, A_("Bounce Left")},  {54, A_("Bounce Right")},  {45, A_("Heartbeat")},     {62, A_("Stairs Down")},
                                   {30, A_("Swoosh")},      {31, A_("Bean")},         {27, A_("Peanut")},        {29, A_("Neutron")},       {20, A_("Curved Square")},
                                   {21, A_("Curved X")},    {19, A_("Inverted Square")}, {23, A_("Pointy Star")}, {25, A_("Curvy Star")},    {32, A_("Clover")},
                                   {33, A_("Trefoil")},     {34, A_("Four Loops")}})});
        return d;
    }();
    return defs;
}

} // namespace

const EffectVariant *EffectKind::variant(const QString &key) const
{
    for (const EffectVariant &v : variants)
        if (v.key == key) return &v;
    return nullptr;
}

const QVector<EffectKind> &effectKinds()
{
    static const QVector<EffectKind> kinds = [] {
        QVector<EffectKind> out;
        for (const KindDef &d : kindDefs()) {
            EffectKind k;
            k.id = QLatin1String(d.id);
            k.name = QCoreApplication::translate("Animation", d.name);
            k.group = QLatin1String(d.group);
            k.cls = d.cls;
            k.duration = d.duration;
            k.amount = d.amount;
            k.smooth = d.smooth;
            for (const VariantDef &v : d.variants)
                k.variants.push_back({v.preset, v.subtype, v.key ? QLatin1String(v.key) : QStringLiteral("p%1").arg(v.preset), QCoreApplication::translate("Animation", v.name)});
            out.push_back(k);
        }
        return out;
    }();
    return kinds;
}

const EffectKind *effectKind(const QString &id)
{
    for (const EffectKind &k : effectKinds())
        if (k.id == id) return &k;
    return nullptr;
}

QStringList motionPathGroups() { return {QStringLiteral("Lines"), QStringLiteral("Arcs"), QStringLiteral("Turns"), QStringLiteral("Shapes"), QStringLiteral("Loops"), QStringLiteral("Curves")}; }

namespace {

constexpr int kMaxAnims = 5000;     // effects one slide's sequence may hold; a longer one is read, not edited
constexpr int kMaxDepth = 16;       // nesting searched for effects

using office::attr;
using office::hasAttr;

QDomElement pChild(const QDomElement &e, const char *name) { return office::child(e, office::ns::P, QLatin1String(name)); }
QVector<QDomElement> pChildren(const QDomElement &e, const char *name) { return office::children(e, office::ns::P, QLatin1String(name)); }
bool pIs(const QDomElement &e, const char *name) { return office::isElement(e, office::ns::P, QLatin1String(name)); }

QString esc(const QString &s)
{
    QString out;
    out.reserve(s.size());
    for (const QChar c : s) {
        if (c == QLatin1Char('&')) out += QLatin1String("&amp;");
        else if (c == QLatin1Char('<')) out += QLatin1String("&lt;");
        else if (c == QLatin1Char('>')) out += QLatin1String("&gt;");
        else if (c == QLatin1Char('"')) out += QLatin1String("&quot;");
        else out += c;
    }
    return out;
}

const char *classAttr(EffectClass c)
{
    switch (c) {
    case EffectClass::Entrance: return "entr";
    case EffectClass::Exit: return "exit";
    case EffectClass::Emphasis: return "emph";
    case EffectClass::MotionPath: return "path";
    default: return "";
    }
}

EffectClass classFromAttr(const QString &name)
{
    if (name == QLatin1String("entr")) return EffectClass::Entrance;
    if (name == QLatin1String("exit")) return EffectClass::Exit;
    if (name == QLatin1String("emph")) return EffectClass::Emphasis;
    if (name == QLatin1String("path")) return EffectClass::MotionPath;
    if (name == QLatin1String("mediacall")) return EffectClass::Media;
    if (name == QLatin1String("verb")) return EffectClass::Verb;
    return EffectClass::Other;
}

const char *nodeTypeOf(EffectStart s)
{
    return s == EffectStart::WithPrevious ? "withEffect" : s == EffectStart::AfterPrevious ? "afterEffect" : "clickEffect";
}

// ---- XML: writing an element as it is, and comparing two ----

struct Prefixes {
    QVector<QPair<QString, QString>> list;
    void need(const QString &prefix, const QString &uri)
    {
        if (uri.isEmpty()) return;
        for (const auto &p : list)
            if (p.first == prefix) return;
        list.push_back({prefix, uri});
    }
};

QString qualified(const QString &prefix, const QString &local) { return prefix.isEmpty() ? local : prefix + QLatin1Char(':') + local; }

void writeElement(const QDomElement &e, QString *out, Prefixes *prefixes, int depth)
{
    if (depth > 64) return;
    prefixes->need(e.prefix(), e.namespaceURI());
    *out += QLatin1Char('<') + qualified(e.prefix(), e.localName());
    const QDomNamedNodeMap attrs = e.attributes();
    QVector<QPair<QString, QString>> list;
    for (int i = 0; i < attrs.length(); ++i) {
        const QDomAttr a = attrs.item(i).toAttr();
        if (!a.prefix().isEmpty()) prefixes->need(a.prefix(), a.namespaceURI());
        list.push_back({qualified(a.prefix(), a.localName()), a.value()});
    }
    std::sort(list.begin(), list.end());
    for (const auto &a : list) *out += QLatin1Char(' ') + a.first + QLatin1String("=\"") + esc(a.second) + QLatin1Char('"');
    if (!e.hasChildNodes()) {
        *out += QLatin1String("/>");
        return;
    }
    *out += QLatin1Char('>');
    for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isElement()) writeElement(n.toElement(), out, prefixes, depth + 1);
        else if (n.isCharacterData() && !n.isComment()) *out += esc(n.toCharacterData().data());
    }
    *out += QLatin1String("</") + qualified(e.prefix(), e.localName()) + QLatin1Char('>');
}

// The element as XML that stands alone: it declares the namespaces it uses.
QString standalone(const QDomElement &e)
{
    QString body;
    Prefixes prefixes;
    writeElement(e, &body, &prefixes, 0);
    QString declared;
    for (const auto &p : prefixes.list)
        declared += (p.first.isEmpty() ? QStringLiteral(" xmlns=\"%1\"") : QStringLiteral(" xmlns:%2=\"%1\"")).arg(esc(p.second), p.first);
    // The declarations go on the element's start tag, which ends at the first '>' or "/>".
    const qsizetype end = body.indexOf(QLatin1Char('>'));
    const qsizetype at = end > 0 && body[end - 1] == QLatin1Char('/') ? end - 1 : end;
    return at < 0 ? body : body.left(at) + declared + body.mid(at);
}

// What two elements say, whatever order their attributes are in, the ids of
// their time nodes aside.
void canon(const QDomElement &e, QString *out, int depth)
{
    if (depth > 64) return;
    *out += QLatin1Char('<') + e.localName();
    QVector<QPair<QString, QString>> list;
    const QDomNamedNodeMap attrs = e.attributes();
    for (int i = 0; i < attrs.length(); ++i) {
        const QDomAttr a = attrs.item(i).toAttr();
        if (a.localName() == QLatin1String("id") && e.localName() == QLatin1String("cTn")) continue;
        list.push_back({a.localName(), a.value()});
    }
    std::sort(list.begin(), list.end());
    for (const auto &a : list) *out += QLatin1Char(' ') + a.first + QLatin1Char('=') + a.second;
    *out += QLatin1Char('>');
    for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isElement()) {
            canon(n.toElement(), out, depth + 1);
        } else if (n.isCharacterData() && !n.isComment()) {
            const QString t = n.toCharacterData().data().trimmed();
            if (!t.isEmpty()) *out += t;
        }
    }
    *out += QLatin1String("</>");
}

QString canonOf(const QString &xml)
{
    QDomDocument dom;
    if (!office::parseXml(xml.toUtf8(), &dom)) return QString();
    QString out;
    canon(dom.documentElement(), &out, 0);
    return out;
}

// A kept effect's time nodes numbered afresh from `*next`, and what refers to them with them.
QString renumbered(const QString &xml, int *next)
{
    static const QRegularExpression ids(QStringLiteral("<p:cTn id=\"(\\d+)\""));
    QHash<QString, QString> map;
    QString out;
    qsizetype last = 0;
    for (auto it = ids.globalMatch(xml); it.hasNext();) {
        const QRegularExpressionMatch m = it.next();
        const QString fresh = QString::number((*next)++);
        map.insert(m.captured(1), fresh);
        out += xml.mid(last, m.capturedStart(1) - last) + fresh;
        last = m.capturedEnd(1);
    }
    out += xml.mid(last);
    static const QRegularExpression refs(QStringLiteral("(<p:r?tn val=\")(\\d+)(\")"));
    QString done;
    last = 0;
    for (auto it = refs.globalMatch(out); it.hasNext();) {
        const QRegularExpressionMatch m = it.next();
        done += out.mid(last, m.capturedStart(2) - last) + map.value(m.captured(2), m.captured(2));
        last = m.capturedEnd(2);
    }
    return done + out.mid(last);
}

// ---- the presets ----

struct Located {
    const EffectKind *kind = nullptr;
    const EffectVariant *variant = nullptr;
};

quint64 presetKey(EffectClass cls, int preset, int subtype) { return (quint64(cls) << 48) | (quint64(preset) << 24) | quint64(subtype); }

const PresetTemplate *findTemplate(EffectClass cls, int preset, int subtype)
{
    static const QHash<quint64, const PresetTemplate *> byKey = [] {
        QHash<quint64, const PresetTemplate *> h;
        for (int i = 0; i < kPresetTemplateCount; ++i) h.insert(presetKey(kPresetTemplates[i].cls, kPresetTemplates[i].presetId, kPresetTemplates[i].subtype), &kPresetTemplates[i]);
        return h;
    }();
    return byKey.value(presetKey(cls, preset, subtype), nullptr);
}

const PresetTemplate *templateFor(const EffectKind &kind, const EffectVariant &v)
{
    return findTemplate(kind.cls, kind.cls == EffectClass::MotionPath ? 0 : v.presetId, kind.cls == EffectClass::MotionPath ? 0 : v.subtype);
}

const char *pathOf(int preset)
{
    for (int i = 0; i < kPresetPathCount; ++i)
        if (kPresetPaths[i].presetId == preset) return kPresetPaths[i].path;
    return nullptr;
}

Located locate(EffectClass cls, int preset, int subtype)
{
    static const QHash<quint64, Located> byKey = [] {
        QHash<quint64, Located> h;
        for (const EffectKind &k : effectKinds())
            for (const EffectVariant &v : k.variants) h.insert(presetKey(k.cls, v.presetId, k.cls == EffectClass::MotionPath ? 0 : v.subtype), Located{&k, &v});
        // The kinds whose ways share one preset are found by their first.
        return h;
    }();
    return byKey.value(presetKey(cls, preset, subtype), Located());
}

QString targetXml(const Anim &a)
{
    if (!a.paragraphs()) return QStringLiteral("<p:tgtEl><p:spTgt spid=\"%1\"/></p:tgtEl>").arg(a.shape);
    return QStringLiteral("<p:tgtEl><p:spTgt spid=\"%1\"><p:txEl><p:pRg st=\"%2\" end=\"%3\"/></p:txEl></p:spTgt></p:tgtEl>").arg(a.shape).arg(a.firstParagraph).arg(a.lastParagraph);
}

QString numberText(double v) { return QString::number(v, 'f', 0); }

// The template's behaviors for the effect: ids from `*next`, the target, and the times.
QString bodyOf(const PresetTemplate &t, const Anim &a, int *next)
{
    const QString body = QString::fromUtf8(t.body);
    const QString target = targetXml(a);
    QString out;
    out.reserve(body.size() + 64);
    const double d = a.duration;
    qsizetype i = 0;
    while (i < body.size()) {
        const qsizetype open = body.indexOf(QLatin1Char('{'), i);
        if (open < 0) {
            out += body.mid(i);
            break;
        }
        out += body.mid(i, open - i);
        const qsizetype close = body.indexOf(QLatin1Char('}'), open);
        if (close < 0) {
            out += body.mid(open);
            break;
        }
        const QString token = body.mid(open + 1, close - open - 1);
        i = close + 1;
        if (token == QLatin1String("I")) {
            out += QString::number((*next)++);
        } else if (token == QLatin1String("T")) {
            out += target;
        } else if (token == QLatin1String("D")) {
            out += QString::number(qRound64(d));
        } else if (token == QLatin1String("D-1")) {
            out += QString::number(qRound64(d) - 1);
        } else if (token.startsWith(QLatin1String("D*"))) {
            out += QString::number(qRound64(d * token.mid(2).toDouble()));
        } else if (token == QLatin1String("BY")) {
            const bool vertical = a.variant == QLatin1String("vertical"), horizontal = a.variant == QLatin1String("horizontal");
            out += QStringLiteral("<p:by x=\"%1\" y=\"%2\"/>").arg(numberText((vertical ? 100 : a.amount) * 1000), numberText((horizontal ? 100 : a.amount) * 1000));
        } else if (token == QLatin1String("ROT")) {
            out += numberText((a.variant == QLatin1String("ccw") ? -1 : 1) * a.amount * 60000);
        } else if (token == QLatin1String("PATH")) {
            const EffectKind *kind = effectKind(a.kind);
            const EffectVariant *v = kind ? kind->variant(a.variant) : nullptr;
            const char *path = v ? pathOf(v->presetId) : nullptr;
            out += QString::fromLatin1(path ? path : "");
        } else {
            out += QLatin1Char('{') + token + QLatin1Char('}');
        }
    }
    return out;
}

// An effect JeffDeck makes, as one p:par of a click step: `grpId` the build it
// belongs to (-1 for a shape that has none), `placeholder` as the program marks
// a placeholder animated whole.
QString effectXml(const Anim &a, int grpId, bool placeholder, int *next)
{
    const EffectKind *kind = effectKind(a.kind);
    const EffectVariant *v = kind ? kind->variant(a.variant) : nullptr;
    if (!kind || !v) return QString();
    const PresetTemplate *t = templateFor(*kind, *v);
    if (!t) return QString();
    QString attrs = QStringLiteral(" presetID=\"%1\" presetClass=\"%2\" presetSubtype=\"%3\"").arg(v->presetId).arg(QLatin1String(classAttr(kind->cls))).arg(kind->cls == EffectClass::MotionPath ? 0 : v->subtype);
    if (a.repeat > 1) attrs += QStringLiteral(" repeatCount=\"%1\"").arg(qRound64(a.repeat * 1000));
    if (a.smoothStart) attrs += QStringLiteral(" accel=\"50000\"");
    if (a.smoothEnd) attrs += QStringLiteral(" decel=\"50000\"");
    attrs += a.rewind ? QStringLiteral(" fill=\"remove\"") : QStringLiteral(" fill=\"hold\"");
    if (grpId >= 0) attrs += QStringLiteral(" grpId=\"%1\"").arg(grpId);
    attrs += QStringLiteral(" nodeType=\"%1\"").arg(QLatin1String(nodeTypeOf(a.start)));
    if (placeholder) attrs += QStringLiteral(" nodePh=\"1\"");
    const int id = (*next)++;
    const QString body = bodyOf(*t, a, next);
    return QStringLiteral("<p:par><p:cTn id=\"%1\"%2><p:stCondLst><p:cond delay=\"%3\"/></p:stCondLst><p:childTnLst>%4</p:childTnLst></p:cTn></p:par>").arg(id).arg(attrs).arg(a.delay).arg(body);
}

// A kept effect with the way it starts as the sequence has it.
QString keptEffectXml(const Anim &a, int *next)
{
    QString xml = a.xml;
    const qsizetype start = xml.indexOf(QLatin1String("<p:cTn "));
    const qsizetype end = start < 0 ? -1 : xml.indexOf(QLatin1Char('>'), start);
    if (end > 0) {
        QString tag = xml.mid(start, end - start);
        static const QRegularExpression nodeType(QStringLiteral("\\bnodeType=\"[^\"]*\""));
        const QString now = QStringLiteral("nodeType=\"%1\"").arg(QLatin1String(nodeTypeOf(a.start)));
        if (tag.contains(nodeType)) tag.replace(nodeType, now);
        else tag += QLatin1Char(' ') + now;
        xml = xml.left(start) + tag + xml.mid(end);
    }
    return renumbered(xml, next);
}

// ---- what a slide's shapes are ----

struct ShapeInfo {
    bool found = false;
    bool sp = false;           // a shape or text box: the program builds it (a p:bldP entry)
    bool placeholder = false;
    bool background = false;   // it has a fill or an outline
};

QHash<int, ShapeInfo> shapeInfos(const Page &slide)
{
    QHash<int, ShapeInfo> out;
    walkItems(slide.items, [&](const ItemPtr &it) {
        if (it->sourceId <= 0 || out.contains(it->sourceId)) return;
        ShapeInfo s;
        s.found = true;
        s.sp = it->type() == ItemType::Text || it->type() == ItemType::Shape;
        s.placeholder = !placeholderType(*it).isEmpty();
        s.background = !it->fill.isNone() || !it->stroke.isNone();
        out.insert(it->sourceId, s);
    });
    return out;
}

// ---- reading ----

int delayOf(const QDomElement &cTn)
{
    for (const QDomElement &cond : pChildren(pChild(cTn, "stCondLst"), "cond")) {
        if (hasAttr(cond, QStringLiteral("evt"))) continue;
        const QString d = attr(cond, QStringLiteral("delay"));
        bool ok = false;
        const qint64 v = d.toLongLong(&ok);
        if (ok) return int(std::clamp<qint64>(v, 0, 1000000000));
    }
    return 0;
}

// The effect's own length: when its last behavior ends, in ms from the effect's start.
bool lengthOf(const QDomElement &childTnLst, qint64 *length)
{
    qint64 end = 0;
    for (const QDomElement &b : office::children(childTnLst)) {
        const QDomElement cTn = pChild(pChild(b, "cBhvr"), "cTn");
        if (cTn.isNull()) return false;
        const QString dur = attr(cTn, QStringLiteral("dur"));
        bool ok = false;
        const qint64 d = dur.toLongLong(&ok);
        if (!ok) return false;
        const qint64 run = d * (attr(cTn, QStringLiteral("autoRev")) == QLatin1String("1") ? 2 : 1);
        end = std::max(end, delayOf(cTn) + run);
    }
    *length = end;
    return true;
}

bool targetOf(const QDomElement &childTnLst, Anim *a)
{
    for (const QDomElement &b : office::children(childTnLst)) {
        const QDomElement sp = pChild(pChild(pChild(b, "cBhvr"), "tgtEl"), "spTgt");
        if (sp.isNull()) continue;
        bool ok = false;
        a->shape = int(attr(sp, QStringLiteral("spid")).toLongLong(&ok));
        if (!ok || a->shape <= 0) return false;
        if (const QDomElement range = pChild(pChild(sp, "txEl"), "pRg"); !range.isNull()) {
            a->firstParagraph = int(attr(range, QStringLiteral("st")).toLongLong());
            a->lastParagraph = int(attr(range, QStringLiteral("end")).toLongLong());
            if (a->firstParagraph < 0 || a->lastParagraph < a->firstParagraph) return false;
        }
        return true;
    }
    return false;
}

// The effect an element is (a p:par holding a time node with a preset), as the sequence lists it.
Anim readEffect(const QDomElement &par, const QDomElement &cTn, int span)
{
    Anim a;
    a.cls = classFromAttr(attr(cTn, QStringLiteral("presetClass")));
    const QString nodeType = attr(cTn, QStringLiteral("nodeType"));
    a.start = nodeType == QLatin1String("withEffect") ? EffectStart::WithPrevious : nodeType == QLatin1String("afterEffect") ? EffectStart::AfterPrevious : EffectStart::OnClick;
    a.delay = delayOf(cTn);
    a.duration = span;
    a.group = hasAttr(cTn, QStringLiteral("grpId")) ? int(attr(cTn, QStringLiteral("grpId")).toLongLong()) : -1;
    a.xml = standalone(par);
    const QDomElement kids = pChild(cTn, "childTnLst");
    targetOf(kids, &a);

    // A kind it is, when the program would write exactly this.
    const int preset = int(attr(cTn, QStringLiteral("presetID")).toLongLong());
    const int subtype = int(attr(cTn, QStringLiteral("presetSubtype")).toLongLong());
    const Located found = locate(a.cls, preset, a.cls == EffectClass::MotionPath ? 0 : subtype);
    if (!found.kind || a.shape <= 0 || kids.isNull()) return a;
    const PresetTemplate *t = templateFor(*found.kind, *found.variant);
    if (!t) return a;
    Anim k = a;
    k.kind = found.kind->id;
    k.variant = found.variant->key;
    k.xml.clear();
    qint64 length = 0;
    if (!lengthOf(kids, &length)) return a;
    k.duration = t->duration == 0 ? 0 : int(std::min<qint64>(length, 3600000));
    k.smoothStart = attr(cTn, QStringLiteral("accel")).toLongLong() > 0;
    k.smoothEnd = attr(cTn, QStringLiteral("decel")).toLongLong() > 0;
    k.rewind = attr(cTn, QStringLiteral("fill")) == QLatin1String("remove");
    const QString count = attr(cTn, QStringLiteral("repeatCount"));
    k.repeat = count.isEmpty() ? 1 : std::max(1.0, count.toDouble() / 1000);
    if (found.kind->amount == EffectKind::Amount::Size) {
        const QDomElement by = pChild(pChild(kids, "animScale"), "by");
        const double x = attr(by, QStringLiteral("x"), QStringLiteral("100000")).toDouble() / 1000, y = attr(by, QStringLiteral("y"), QStringLiteral("100000")).toDouble() / 1000;
        if (qFuzzyCompare(x, 100.0) && !qFuzzyCompare(y, 100.0)) {
            k.variant = QStringLiteral("vertical");
            k.amount = y;
        } else if (qFuzzyCompare(y, 100.0) && !qFuzzyCompare(x, 100.0)) {
            k.variant = QStringLiteral("horizontal");
            k.amount = x;
        } else {
            k.variant = QStringLiteral("both");
            k.amount = x;
        }
    } else if (found.kind->amount == EffectKind::Amount::Spin) {
        const double by = attr(pChild(kids, "animRot"), QStringLiteral("by")).toDouble() / 60000;
        k.variant = by < 0 ? QStringLiteral("ccw") : QStringLiteral("cw");
        k.amount = std::abs(by);
    }
    // The same preset as another way of the kind (the ways of a motion path are presets themselves).
    const bool placeholder = hasAttr(cTn, QStringLiteral("nodePh"));
    int next = 1;
    QString again = effectXml(k, k.group, placeholder, &next);
    again.replace(0, 6, QStringLiteral("<p:par xmlns:p=\"%1\"").arg(office::ns::P));
    if (again.isEmpty() || canonOf(again) != canonOf(a.xml)) return a;
    k.xml.clear();
    return k;
}

void collectEffects(const QDomElement &list, int depth, QVector<Anim> *out, bool *tooMany)
{
    if (depth > kMaxDepth) return;
    for (const QDomElement &par : office::children(list)) {
        if (!pIs(par, "par") && !pIs(par, "seq")) continue;
        const QDomElement cTn = pChild(par, "cTn");
        const QString nodeType = attr(cTn, QStringLiteral("nodeType"));
        if (hasAttr(cTn, QStringLiteral("presetClass")) || nodeType == QLatin1String("clickEffect") || nodeType == QLatin1String("withEffect") || nodeType == QLatin1String("afterEffect")) {
            if (out->size() >= kMaxAnims) {
                *tooMany = true;
                return;
            }
            // How long a kept effect lasts: its behaviors' end, in ms from its start.
            qint64 length = 0;
            lengthOf(pChild(cTn, "childTnLst"), &length);
            out->push_back(readEffect(par, cTn, int(std::min<qint64>(length, 3600000))));
        } else {
            collectEffects(pChild(cTn, "childTnLst"), depth + 1, out, tooMany);
        }
    }
}

} // namespace

AnimSequence readAnimations(const Page &slide)
{
    AnimSequence seq;
    QDomDocument dom;
    const QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
    if (xml.isEmpty() || !office::parseXml(xml.toUtf8(), &dom)) return seq;
    const QDomElement root = dom.documentElement();
    if (!pIs(root, "timing")) return seq;
    bool haveMain = false, tooMany = false;
    // The sequences are the time nodes' children: the tmRoot's, in a p:tnLst.
    const QDomElement rootNode = pChild(pChild(pChild(pChild(root, "tnLst"), "par"), "cTn"), "childTnLst");
    for (const QDomElement &s : pChildren(rootNode, "seq")) {
        const QDomElement cTn = pChild(s, "cTn");
        if (attr(cTn, QStringLiteral("nodeType")) == QLatin1String("mainSeq") && !haveMain) {
            haveMain = true;
            collectEffects(pChild(cTn, "childTnLst"), 0, &seq.effects, &tooMany);
        } else {
            seq.kept << standalone(s);
        }
    }
    if (const QDomElement ext = pChild(root, "extLst"); !ext.isNull()) seq.extension = standalone(ext);
    for (const QDomElement &b : office::children(pChild(root, "bldLst"))) seq.builds << standalone(b);
    seq.editable = !tooMany;
    return seq;
}

// ---- the sequence ----

int Anim::span() const { return delay + int(std::min<double>(double(duration) * std::max(1.0, repeat), 3600000.0)); }

namespace {

struct Placed {
    int click = 0;          // 0 for the effects that start with the slide
    int begin = 0, end = 0; // ms after the click
    int inner = 0;          // which part of the click step, from 0
    int innerStart = 0;     // ms after the click that part starts
};

QVector<Placed> layoutOf(const QVector<Anim> &effects)
{
    QVector<Placed> out;
    int click = 0, innerStart = 0, innerEnd = 0, inner = -1;
    bool first = true;
    for (const Anim &a : effects) {
        if (a.start == EffectStart::OnClick) {
            ++click;
            innerStart = innerEnd = 0;
            inner = 0;
        } else if (first || inner < 0) {
            innerStart = innerEnd = 0;
            inner = 0;
        } else if (a.start == EffectStart::AfterPrevious) {
            innerStart += innerEnd;
            innerEnd = 0;
            ++inner;
        }
        first = false;
        Placed p;
        p.click = click;
        p.inner = inner;
        p.innerStart = innerStart;
        p.begin = innerStart + a.delay;
        p.end = innerStart + a.span();
        innerEnd = std::max(innerEnd, a.span());
        out.push_back(p);
    }
    return out;
}

} // namespace

int AnimSequence::clickOf(int i) const
{
    int n = 0;
    for (int k = 0; k <= i && k < effects.size(); ++k)
        if (effects[k].start == EffectStart::OnClick) ++n;
    return n;
}

int AnimSequence::clicks() const { return clickOf(int(effects.size()) - 1); }
int AnimSequence::beginOf(int i) const { return i >= 0 && i < effects.size() ? layoutOf(effects)[i].begin : 0; }
int AnimSequence::endOf(int i) const { return i >= 0 && i < effects.size() ? layoutOf(effects)[i].end : 0; }

int AnimSequence::add(const Anim &anim, int at)
{
    at = at < 0 || at > effects.size() ? int(effects.size()) : at;
    effects.insert(at, anim);
    return at;
}

void AnimSequence::remove(int i)
{
    if (i < 0 || i >= effects.size()) return;
    // The click an effect took goes to the next effect of its step, so that the clicks that follow keep their count.
    if (effects[i].start == EffectStart::OnClick && i + 1 < effects.size() && effects[i + 1].start != EffectStart::OnClick) effects[i + 1].start = EffectStart::OnClick;
    effects.remove(i);
}

void AnimSequence::move(int from, int to)
{
    if (from < 0 || from >= effects.size()) return;
    to = std::clamp(to, 0, int(effects.size()) - 1);
    if (from == to) return;
    const Anim a = effects[from];
    effects.remove(from);
    effects.insert(to, a);
}

QVector<int> AnimSequence::effectsOf(int shape) const
{
    QVector<int> out;
    for (int i = 0; i < effects.size(); ++i)
        if (effects[i].shape == shape) out.push_back(i);
    return out;
}

Anim makeAnim(const EffectKind &kind, int shape)
{
    Anim a;
    a.kind = kind.id;
    a.cls = kind.cls;
    a.variant = kind.firstVariant().key;
    a.shape = shape;
    a.duration = kind.duration;
    a.smoothStart = a.smoothEnd = kind.smooth;
    a.amount = kind.amount == EffectKind::Amount::Size ? 150 : kind.amount == EffectKind::Amount::Spin ? 360 : 0;
    return a;
}

Anim makeAnim(const QString &kind, int shape)
{
    const EffectKind *k = effectKind(kind);
    return k ? makeAnim(*k, shape) : Anim();
}

void changeKind(Anim &anim, const EffectKind &kind)
{
    const Anim fresh = makeAnim(kind, anim.shape);
    const int duration = kind.duration == 0 ? 0 : anim.duration > 0 && kind.cls != EffectClass::MotionPath && anim.cls != EffectClass::MotionPath ? anim.duration : fresh.duration;
    Anim a = fresh;
    a.firstParagraph = anim.firstParagraph;
    a.lastParagraph = anim.lastParagraph;
    a.start = anim.start;
    a.delay = anim.delay;
    a.duration = duration;
    a.group = anim.group;
    anim = a;
}

// ---- shapes ----

int shapeNumber(Page &slide, const QString &itemId)
{
    int highest = 1;
    Item *found = nullptr;
    walkItems(slide.items, [&](const ItemPtr &it) {
        highest = std::max(highest, it->sourceId);
        if (it->id == itemId) found = it.get();
    });
    if (!found) return 0;
    if (found->sourceId > 0) return found->sourceId;
    // The numbers of the frames kept as they were are taken too.
    static const QRegularExpression numbers(QStringLiteral("<p:cNvPr id=\"(\\d+)\""));
    auto scan = [&](const QString &xml) {
        for (auto it = numbers.globalMatch(xml); it.hasNext();) highest = std::max<qint64>(highest, std::min<qint64>(it.next().captured(1).toLongLong(), 1000000000));
    };
    for (const QJsonValue &v : slide.props.value(QStringLiteral("unread")).toArray()) scan(v.toObject().value(QStringLiteral("xml")).toString());
    for (const QJsonValue &v : slide.props.value(QStringLiteral("charts")).toArray()) scan(v.toObject().value(QStringLiteral("frameXml")).toString());
    found->sourceId = highest + 1;
    return found->sourceId;
}

QVector<QPair<int, int>> paragraphGroups(const Document &doc, const Page &slide, int shape)
{
    QVector<QPair<int, int>> out;
    const Item *item = nullptr;
    walkItems(slide.items, [&](const ItemPtr &it) {
        if (it->sourceId == shape && !item) item = it.get();
    });
    if (!item) return out;
    QString storyId;
    if (item->type() == ItemType::Text) storyId = static_cast<const TextItem *>(item)->storyId;
    else if (item->type() == ItemType::Shape) storyId = static_cast<const ShapeItem *>(item)->storyId;
    const QTextDocument *story = storyId.isEmpty() ? nullptr : doc.storyDoc(storyId);
    if (!story) return out;
    int first = -1, index = 0;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next(), ++index) {
        const QTextBlockFormat bf = b.blockFormat();
        int level = bf.hasProperty(tp::ListLevel) ? bf.intProperty(tp::ListLevel) : b.textList() ? b.textList()->format().intProperty(tp::ListLevel) : 0;
        if (level <= 0 || first < 0) {
            if (first >= 0) out.push_back({first, index - 1});
            first = index;
        }
    }
    if (first >= 0) out.push_back({first, index - 1});
    // A group with no text at all is not animated by itself.
    QVector<QPair<int, int>> kept;
    for (const auto &g : std::as_const(out)) {
        bool text = false;
        index = 0;
        for (QTextBlock b = story->begin(); b.isValid() && !text; b = b.next(), ++index)
            if (index >= g.first && index <= g.second && !b.text().trimmed().isEmpty()) text = true;
        if (text) kept.push_back(g);
    }
    return kept;
}

void setByParagraph(AnimSequence &seq, int i, bool byParagraph, const Document &doc, const Page &slide)
{
    if (i < 0 || i >= seq.effects.size() || seq.effects[i].custom()) return;
    const Anim a = seq.effects[i];
    if (byParagraph) {
        if (a.paragraphs()) return;
        const QVector<QPair<int, int>> groups = paragraphGroups(doc, slide, a.shape);
        if (groups.isEmpty()) return;
        int group = a.group;
        if (group < 0) {
            group = 0;
            for (const Anim &e : std::as_const(seq.effects))
                if (e.shape == a.shape) group = std::max(group, e.group + 1);
        }
        seq.effects.remove(i);
        for (int k = 0; k < groups.size(); ++k) {
            Anim p = a;
            p.firstParagraph = groups[k].first;
            p.lastParagraph = groups[k].second;
            p.group = group;
            if (k > 0) p.start = a.start == EffectStart::OnClick ? EffectStart::OnClick : EffectStart::AfterPrevious;
            seq.effects.insert(i + k, p);
        }
        return;
    }
    if (!a.paragraphs()) return;
    // The paragraphs of the build become one effect, at the place of the first.
    int first = i;
    while (first > 0 && seq.effects[first - 1].shape == a.shape && seq.effects[first - 1].group == a.group && seq.effects[first - 1].paragraphs()) --first;
    int last = i;
    while (last + 1 < seq.effects.size() && seq.effects[last + 1].shape == a.shape && seq.effects[last + 1].group == a.group && seq.effects[last + 1].paragraphs()) ++last;
    Anim whole = seq.effects[first];
    whole.firstParagraph = whole.lastParagraph = -1;
    seq.effects.remove(first, last - first + 1);
    seq.effects.insert(first, whole);
}

void copyEffects(AnimSequence &seq, int from, const QVector<int> &to)
{
    QVector<Anim> copies;
    int lastGroup = -2;
    for (const Anim &a : std::as_const(seq.effects)) {
        if (a.shape != from || a.custom()) continue;
        // A build by paragraph becomes one effect on the whole shape.
        if (a.paragraphs() && a.group == lastGroup) continue;
        lastGroup = a.paragraphs() ? a.group : -2;
        Anim c = a;
        c.firstParagraph = c.lastParagraph = -1;
        copies.push_back(c);
    }
    for (int target : to) {
        if (target == from) continue;
        int group = 0;
        for (const Anim &c : std::as_const(copies)) {
            Anim n = c;
            n.shape = target;
            n.group = group++;
            seq.effects.push_back(n);
        }
    }
}

// ---- writing ----

namespace {

// The shape numbers a kept effect aims at.
QSet<int> shapesOfKept(const QString &xml)
{
    static const QRegularExpression spid(QStringLiteral("\\bspid=\"(\\d+)\""));
    QSet<int> out;
    for (auto it = spid.globalMatch(xml); it.hasNext();) out.insert(int(it.next().captured(1).toLongLong()));
    return out;
}

} // namespace

QString animationsXml(const Document &doc, const Page &slide, const AnimSequence &seq)
{
    Q_UNUSED(doc)
    const QHash<int, ShapeInfo> shapes = shapeInfos(slide);
    // The effects that still have a shape to play on.
    QVector<Anim> effects;
    for (const Anim &a : seq.effects) {
        if (!a.custom() && !shapes.value(a.shape).found) continue;
        effects.push_back(a);
    }
    if (effects.isEmpty() && seq.kept.isEmpty()) return QString();

    // Which build each effect belongs to: per shape, the groups in the order they come, numbered from 0 around the ones kept effects hold.
    QHash<int, QSet<int>> taken;
    for (const Anim &a : std::as_const(effects))
        if (a.custom() && a.group >= 0)
            for (int s : shapesOfKept(a.xml)) taken[s].insert(a.group);
    QHash<QPair<int, int>, int> numbered;
    QHash<int, int> nextGroup;
    QVector<int> grp(effects.size(), -1);
    for (int i = 0; i < effects.size(); ++i) {
        const Anim &a = effects[i];
        if (a.custom() || !shapes.value(a.shape).sp) continue;
        const QPair<int, int> key(a.shape, a.group >= 0 ? a.group : 100000 + i);
        if (!numbered.contains(key)) {
            int n = nextGroup.value(a.shape, 0);
            while (taken[a.shape].contains(n)) ++n;
            numbered.insert(key, n);
            nextGroup[a.shape] = n + 1;
        }
        grp[i] = numbered.value(key);
    }

    // Written in document order, so the ids come out in it: each click step, each part of it, each effect.
    const QVector<Placed> placed = layoutOf(effects);
    int next = 3;
    QString builds, mainXml, parts, inside;
    QSet<QPair<int, int>> built;
    QSet<int> aimedAt;
    int step = -1, part = -1, clickId = 0, partId = 0, partDelay = 0;
    bool automatic = false;
    auto closePart = [&] {
        if (part < 0) return;
        parts += QStringLiteral("<p:par><p:cTn id=\"%1\" fill=\"hold\"><p:stCondLst><p:cond delay=\"%2\"/></p:stCondLst><p:childTnLst>%3</p:childTnLst></p:cTn></p:par>").arg(partId).arg(partDelay).arg(inside);
        inside.clear();
        part = -1;
    };
    auto closeStep = [&] {
        closePart();
        if (step < 0) return;
        const QString begin = automatic ? QStringLiteral("<p:cond evt=\"onBegin\" delay=\"0\"><p:tn val=\"2\"/></p:cond>") : QString();
        mainXml += QStringLiteral("<p:par><p:cTn id=\"%1\" fill=\"hold\"><p:stCondLst><p:cond delay=\"indefinite\"/>%2</p:stCondLst><p:childTnLst>%3</p:childTnLst></p:cTn></p:par>").arg(clickId).arg(begin, parts);
        parts.clear();
        step = -1;
    };
    for (int i = 0; i < effects.size(); ++i) {
        const Anim &a = effects[i];
        if (step != placed[i].click || step < 0) {
            closeStep();
            step = placed[i].click;
            clickId = next++;
            automatic = placed[i].click == 0;
            part = -1;
        }
        if (part != placed[i].inner || part < 0) {
            closePart();
            part = placed[i].inner;
            partId = next++;
            partDelay = placed[i].innerStart;
        }
        if (a.custom()) {
            inside += keptEffectXml(a, &next);
            aimedAt.unite(shapesOfKept(a.xml));
            continue;
        }
        const ShapeInfo &si = shapes.value(a.shape);
        inside += effectXml(a, grp[i], si.placeholder && !a.paragraphs(), &next);
        aimedAt.insert(a.shape);
        if (si.sp && !built.contains({a.shape, grp[i]})) {
            built.insert({a.shape, grp[i]});
            builds += QStringLiteral("<p:bldP spid=\"%1\" grpId=\"%2\"%3%4/>").arg(a.shape).arg(grp[i]).arg(a.paragraphs() ? QStringLiteral(" build=\"p\"") : QString(), si.background ? QStringLiteral(" animBg=\"1\"") : QString());
        }
    }
    closeStep();
    // What was kept: sequences that wait for a shape's click, and the builds of what the file holds, as long as a shape they are for is still aimed at.
    QString kept;
    for (const QString &k : seq.kept) kept += renumbered(k, &next);
    for (const QString &b : seq.builds) {
        static const QRegularExpression spid(QStringLiteral("\\bspid=\"(\\d+)\"")), grpIdRe(QStringLiteral("\\bgrpId=\"(\\d+)\""));
        const QRegularExpressionMatch m = spid.match(b), g = grpIdRe.match(b);
        const int shape = m.hasMatch() ? int(m.captured(1).toLongLong()) : -1;
        const int group = g.hasMatch() ? int(g.captured(1).toLongLong()) : -1;
        if (shape > 0 && (built.contains({shape, group}) || !aimedAt.contains(shape))) continue;
        builds += b;
    }
    QString xml = QStringLiteral("<p:timing xmlns:p=\"%1\"><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\"><p:childTnLst>").arg(office::ns::P);
    xml += QStringLiteral("<p:seq concurrent=\"1\" nextAc=\"seek\"><p:cTn id=\"2\" dur=\"indefinite\" nodeType=\"mainSeq\"><p:childTnLst>") + mainXml + QStringLiteral("</p:childTnLst></p:cTn>");
    xml += QStringLiteral("<p:prevCondLst><p:cond evt=\"onPrev\" delay=\"0\"><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:prevCondLst>"
                          "<p:nextCondLst><p:cond evt=\"onNext\" delay=\"0\"><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:nextCondLst></p:seq>");
    xml += kept + QStringLiteral("</p:childTnLst></p:cTn></p:par></p:tnLst>");
    if (!builds.isEmpty()) xml += QStringLiteral("<p:bldLst>") + builds + QStringLiteral("</p:bldLst>");
    xml += seq.extension + QStringLiteral("</p:timing>");
    return xml;
}

void setAnimations(const Document &doc, Page &slide, const AnimSequence &seq)
{
    const QString xml = animationsXml(doc, slide, seq);
    if (xml.isEmpty()) slide.props.remove(QStringLiteral("timingXml"));
    else slide.props[QStringLiteral("timingXml")] = xml;
}

QString timingForSave(const Document &doc, const Page &slide, const std::function<bool(int shape)> &present, int *dropped)
{
    const QString was = slide.props.value(QStringLiteral("timingXml")).toString();
    if (dropped) *dropped = 0;
    AnimSequence seq = readAnimations(slide);
    if (!seq.editable || seq.effects.isEmpty()) return was;
    int gone = 0;
    for (int i = int(seq.effects.size()) - 1; i >= 0; --i) {
        const Anim &a = seq.effects[i];
        bool missing = false;
        if (a.custom()) {
            for (int s : shapesOfKept(a.xml)) missing |= !present(s);
        } else {
            missing = !present(a.shape);
        }
        if (!missing) continue;
        seq.remove(i);
        ++gone;
    }
    if (gone == 0) return was;
    if (dropped) *dropped = gone;
    return animationsXml(doc, slide, seq);
}

} // namespace jp::deck
