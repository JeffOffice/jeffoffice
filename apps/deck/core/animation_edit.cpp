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
#include <QXmlStreamReader>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextList>
#include <algorithm>
#include <cmath>
#include <memory>

namespace jp::deck {

// ---- the kinds ----

namespace {

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
    return {{"bottom", QT_TRANSLATE_NOOP("Animation", "From Bottom"), 2, 4},       {"left", QT_TRANSLATE_NOOP("Animation", "From Left"), 2, 8},         {"top", QT_TRANSLATE_NOOP("Animation", "From Top"), 2, 1},
            {"right", QT_TRANSLATE_NOOP("Animation", "From Right"), 2, 2},         {"bottomLeft", QT_TRANSLATE_NOOP("Animation", "From Bottom-Left"), 2, 12}, {"topLeft", QT_TRANSLATE_NOOP("Animation", "From Top-Left"), 2, 9},
            {"topRight", QT_TRANSLATE_NOOP("Animation", "From Top-Right"), 2, 3},  {"bottomRight", QT_TRANSLATE_NOOP("Animation", "From Bottom-Right"), 2, 6}};
}
std::vector<VariantDef> wipeVariants()
{
    return {{"bottom", QT_TRANSLATE_NOOP("Animation", "From Bottom"), 22, 4}, {"left", QT_TRANSLATE_NOOP("Animation", "From Left"), 22, 8}, {"top", QT_TRANSLATE_NOOP("Animation", "From Top"), 22, 1}, {"right", QT_TRANSLATE_NOOP("Animation", "From Right"), 22, 2}};
}
std::vector<VariantDef> splitVariants()
{
    return {{"verticalIn", QT_TRANSLATE_NOOP("Animation", "Vertical In"), 16, 21}, {"verticalOut", QT_TRANSLATE_NOOP("Animation", "Vertical Out"), 16, 37}, {"horizontalIn", QT_TRANSLATE_NOOP("Animation", "Horizontal In"), 16, 26},
            {"horizontalOut", QT_TRANSLATE_NOOP("Animation", "Horizontal Out"), 16, 42}};
}
std::vector<VariantDef> floatVariants() { return {{"up", QT_TRANSLATE_NOOP("Animation", "Float Up"), 42, 0}, {"down", QT_TRANSLATE_NOOP("Animation", "Float Down"), 47, 0}}; }

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
        d.push_back({"entr.appear", QT_TRANSLATE_NOOP("Animation", "Appear"), kEntrance, "", 0, none, false, {{"none", QT_TRANSLATE_NOOP("Animation", "Appear"), 1, 0}}});
        d.push_back({"entr.fade", QT_TRANSLATE_NOOP("Animation", "Fade"), kEntrance, "", 500, none, false, {{"none", QT_TRANSLATE_NOOP("Animation", "Fade"), 10, 0}}});
        d.push_back({"entr.fly", QT_TRANSLATE_NOOP("Animation", "Fly In"), kEntrance, "", 500, none, false, flyVariants()});
        d.push_back({"entr.float", QT_TRANSLATE_NOOP("Animation", "Float In"), kEntrance, "", 1000, none, false, floatVariants()});
        d.push_back({"entr.split", QT_TRANSLATE_NOOP("Animation", "Split"), kEntrance, "", 500, none, false, splitVariants()});
        d.push_back({"entr.wipe", QT_TRANSLATE_NOOP("Animation", "Wipe"), kEntrance, "", 500, none, false, wipeVariants()});
        d.push_back({"entr.growturn", QT_TRANSLATE_NOOP("Animation", "Grow & Turn"), kEntrance, "", 1000, none, false, {{"none", QT_TRANSLATE_NOOP("Animation", "Grow & Turn"), 31, 0}}});
        d.push_back({"entr.zoom", QT_TRANSLATE_NOOP("Animation", "Zoom"), kEntrance, "", 500, none, false, {{"object", QT_TRANSLATE_NOOP("Animation", "Object Center"), 53, 16}, {"slide", QT_TRANSLATE_NOOP("Animation", "Slide Center"), 53, 528}}});

        d.push_back({"emph.pulse", QT_TRANSLATE_NOOP("Animation", "Pulse"), kEmphasis, "", 500, none, false, {{"none", QT_TRANSLATE_NOOP("Animation", "Pulse"), 26, 0}}});
        d.push_back({"emph.teeter", QT_TRANSLATE_NOOP("Animation", "Teeter"), kEmphasis, "", 1000, none, false, {{"none", QT_TRANSLATE_NOOP("Animation", "Teeter"), 32, 0}}});
        d.push_back({"emph.spin", QT_TRANSLATE_NOOP("Animation", "Spin"), kEmphasis, "", 2000, Amount::Spin, false, {{"cw", QT_TRANSLATE_NOOP("Animation", "Clockwise"), 8, 0}, {"ccw", QT_TRANSLATE_NOOP("Animation", "Counterclockwise"), 8, 0}}});
        d.push_back({"emph.growshrink", QT_TRANSLATE_NOOP("Animation", "Grow/Shrink"), kEmphasis, "", 2000, Amount::Size, false,
                     {{"both", QT_TRANSLATE_NOOP("Animation", "Both"), 6, 0}, {"horizontal", QT_TRANSLATE_NOOP("Animation", "Horizontal"), 6, 0}, {"vertical", QT_TRANSLATE_NOOP("Animation", "Vertical"), 6, 0}}});

        d.push_back({"exit.disappear", QT_TRANSLATE_NOOP("Animation", "Disappear"), kExit, "", 0, none, false, {{"none", QT_TRANSLATE_NOOP("Animation", "Disappear"), 1, 0}}});
        d.push_back({"exit.fade", QT_TRANSLATE_NOOP("Animation", "Fade"), kExit, "", 500, none, false, {{"none", QT_TRANSLATE_NOOP("Animation", "Fade"), 10, 0}}});
        d.push_back({"exit.fly", QT_TRANSLATE_NOOP("Animation", "Fly Out"), kExit, "", 500, none, false, flyVariants()});
        d.push_back({"exit.float", QT_TRANSLATE_NOOP("Animation", "Float Out"), kExit, "", 1000, none, false, floatVariants()});
        d.push_back({"exit.split", QT_TRANSLATE_NOOP("Animation", "Split"), kExit, "", 500, none, false, splitVariants()});
        d.push_back({"exit.wipe", QT_TRANSLATE_NOOP("Animation", "Wipe"), kExit, "", 500, none, false, wipeVariants()});
        d.push_back({"exit.growturn", QT_TRANSLATE_NOOP("Animation", "Shrink & Turn"), kExit, "", 1000, none, false, {{"none", QT_TRANSLATE_NOOP("Animation", "Shrink & Turn"), 31, 0}}});
        d.push_back({"exit.zoom", QT_TRANSLATE_NOOP("Animation", "Zoom"), kExit, "", 500, none, false, {{"object", QT_TRANSLATE_NOOP("Animation", "Object Center"), 53, 32}}});

        d.push_back({"path.lines", QT_TRANSLATE_NOOP("Animation", "Lines"), kPath, "Lines", 2000, none, true,
                     pathVariants({{42, QT_TRANSLATE_NOOP("Animation", "Down")}, {63, QT_TRANSLATE_NOOP("Animation", "Right")}, {35, QT_TRANSLATE_NOOP("Animation", "Left")}, {64, QT_TRANSLATE_NOOP("Animation", "Up")}, {49, QT_TRANSLATE_NOOP("Animation", "Diagonal Down Right")}, {56, QT_TRANSLATE_NOOP("Animation", "Diagonal Up Right")}})});
        d.push_back({"path.arcs", QT_TRANSLATE_NOOP("Animation", "Arcs"), kPath, "Arcs", 2000, none, true, pathVariants({{37, QT_TRANSLATE_NOOP("Animation", "Down")}, {51, QT_TRANSLATE_NOOP("Animation", "Left")}, {58, QT_TRANSLATE_NOOP("Animation", "Right")}, {44, QT_TRANSLATE_NOOP("Animation", "Up")}})});
        d.push_back({"path.turns", QT_TRANSLATE_NOOP("Animation", "Turns"), kPath, "Turns", 2000, none, true,
                     pathVariants({{36, QT_TRANSLATE_NOOP("Animation", "Down then Right")}, {50, QT_TRANSLATE_NOOP("Animation", "Right then Down")}, {57, QT_TRANSLATE_NOOP("Animation", "Up then Right")}, {43, QT_TRANSLATE_NOOP("Animation", "Right then Up")}})});
        d.push_back({"path.shapes", QT_TRANSLATE_NOOP("Animation", "Shapes"), kPath, "Shapes", 2000, none, true,
                     pathVariants({{1, QT_TRANSLATE_NOOP("Animation", "Circle")},         {3, QT_TRANSLATE_NOOP("Animation", "Diamond")},        {4, QT_TRANSLATE_NOOP("Animation", "Hexagon")},         {10, QT_TRANSLATE_NOOP("Animation", "Octagon")},         {15, QT_TRANSLATE_NOOP("Animation", "Pentagon")},
                                   {7, QT_TRANSLATE_NOOP("Animation", "Square")},         {13, QT_TRANSLATE_NOOP("Animation", "Equal Triangle")}, {2, QT_TRANSLATE_NOOP("Animation", "Right Triangle")},  {8, QT_TRANSLATE_NOOP("Animation", "Trapezoid")},        {14, QT_TRANSLATE_NOOP("Animation", "Parallelogram")},
                                   {12, QT_TRANSLATE_NOOP("Animation", "Football")},      {18, QT_TRANSLATE_NOOP("Animation", "Teardrop")},      {9, QT_TRANSLATE_NOOP("Animation", "Heart")},           {6, QT_TRANSLATE_NOOP("Animation", "Crescent Moon")},    {16, QT_TRANSLATE_NOOP("Animation", "4 Point Star")},
                                   {5, QT_TRANSLATE_NOOP("Animation", "5 Point Star")},   {11, QT_TRANSLATE_NOOP("Animation", "6 Point Star")},  {17, QT_TRANSLATE_NOOP("Animation", "8 Point Star")}})});
        d.push_back({"path.loops", QT_TRANSLATE_NOOP("Animation", "Loops"), kPath, "Loops", 2000, none, true,
                     pathVariants({{24, QT_TRANSLATE_NOOP("Animation", "Horizontal Figure 8")}, {22, QT_TRANSLATE_NOOP("Animation", "Vertical Figure 8")}, {26, QT_TRANSLATE_NOOP("Animation", "Two Circles")}, {28, QT_TRANSLATE_NOOP("Animation", "Figure 8 Four")},
                                   {48, QT_TRANSLATE_NOOP("Animation", "Loop de Loop")},        {61, QT_TRANSLATE_NOOP("Animation", "Loop de Loop Reverse")}, {46, QT_TRANSLATE_NOOP("Animation", "Spiral Left")}, {55, QT_TRANSLATE_NOOP("Animation", "Spiral Right")},
                                   {52, QT_TRANSLATE_NOOP("Animation", "Funnel")},              {53, QT_TRANSLATE_NOOP("Animation", "Spring")}})});
        d.push_back({"path.curves", QT_TRANSLATE_NOOP("Animation", "Curves"), kPath, "Curves", 2000, none, true,
                     pathVariants({{38, QT_TRANSLATE_NOOP("Animation", "Zigzag")},      {39, QT_TRANSLATE_NOOP("Animation", "S Curve 1")},    {59, QT_TRANSLATE_NOOP("Animation", "S Curve 2")},     {40, QT_TRANSLATE_NOOP("Animation", "Sine Wave")},     {60, QT_TRANSLATE_NOOP("Animation", "Decaying Wave")},
                                   {47, QT_TRANSLATE_NOOP("Animation", "Wave")},        {41, QT_TRANSLATE_NOOP("Animation", "Bounce Left")},  {54, QT_TRANSLATE_NOOP("Animation", "Bounce Right")},  {45, QT_TRANSLATE_NOOP("Animation", "Heartbeat")},     {62, QT_TRANSLATE_NOOP("Animation", "Stairs Down")},
                                   {30, QT_TRANSLATE_NOOP("Animation", "Swoosh")},      {31, QT_TRANSLATE_NOOP("Animation", "Bean")},         {27, QT_TRANSLATE_NOOP("Animation", "Peanut")},        {29, QT_TRANSLATE_NOOP("Animation", "Neutron")},       {20, QT_TRANSLATE_NOOP("Animation", "Curved Square")},
                                   {21, QT_TRANSLATE_NOOP("Animation", "Curved X")},    {19, QT_TRANSLATE_NOOP("Animation", "Inverted Square")}, {23, QT_TRANSLATE_NOOP("Animation", "Pointy Star")}, {25, QT_TRANSLATE_NOOP("Animation", "Curvy Star")},    {32, QT_TRANSLATE_NOOP("Animation", "Clover")},
                                   {33, QT_TRANSLATE_NOOP("Animation", "Trefoil")},     {34, QT_TRANSLATE_NOOP("Animation", "Four Loops")}})});
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

constexpr int kMaxAnims = 2000;     // effects one slide's sequence may hold; a longer one is read, not edited
constexpr int kMaxDepth = 16;       // nesting searched for effects
constexpr int kMaxWriteDepth = 300; // the XML reader allows 256 levels, so no element is ever cut when written or compared
constexpr int kMaxKept = 500;       // nodes beside the main sequence a timing may have, to be kept

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
    if (depth > kMaxWriteDepth) return;
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
    if (depth > kMaxWriteDepth) return;
    *out += QLatin1Char('<') + e.localName();
    QVector<QPair<QString, QString>> list;
    const QDomNamedNodeMap attrs = e.attributes();
    for (int i = 0; i < attrs.length(); ++i) {
        const QDomAttr a = attrs.item(i).toAttr();
        if (a.localName() == QLatin1String("id") && e.localName() == QLatin1String("cTn")) continue;
        if (a.localName() == QLatin1String("val") && e.localName() == QLatin1String("tn")) continue;   // a time node's number
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

// The ids a timing's time nodes take, one each, in the order they are written; the ids the file's kept nodes have are left to them.
struct Ids {
    int next = 1;
    QSet<int> reserved;
    int take()
    {
        while (reserved.contains(next)) ++next;
        return next++;
    }
};

// The elements of a tree, each before what is inside it.
template <typename F>
void forEachElement(const QDomElement &root, F &&fn)
{
    QVector<QDomElement> stack{root};
    while (!stack.isEmpty()) {
        const QDomElement e = stack.takeLast();
        fn(e);
        const QVector<QDomElement> kids = office::children(e);
        for (qsizetype i = kids.size() - 1; i >= 0; --i) stack.push_back(kids[i]);
    }
}

// The ids of the time nodes in some of the file's own XML (that uses the namespaces `declarations` declares).
QSet<int> timeNodeIds(const QString &xml, const QString &declarations)
{
    QSet<int> out;
    QDomDocument dom;
    if (!office::parseXml(QStringLiteral("<w%1>%2</w>").arg(declarations, xml).toUtf8(), &dom)) return out;
    forEachElement(dom.documentElement(), [&](const QDomElement &e) {
        if (e.localName() == QLatin1String("cTn") && hasAttr(e, QStringLiteral("id"))) out.insert(int(attr(e, QStringLiteral("id")).toLongLong()));
    });
    return out;
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

// The template's behaviors for the effect: their ids, the target, and the times.
QString bodyOf(const PresetTemplate &t, const Anim &a, Ids *ids)
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
            out += QString::number(ids->take());
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
QString effectXml(const Anim &a, int grpId, bool placeholder, Ids *ids)
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
    const int id = ids->take();
    const QString body = bodyOf(*t, a, ids);
    // What it does to its shape when it is over: hide it when the effect ends, or at the next click.
    QString after;
    if (a.after != AfterEffect::None) {
        const bool next = a.after == AfterEffect::HideOnNextClick;
        const QString cTn = QStringLiteral("<p:cTn dur=\"1\" fill=\"hold\" display=\"0\" masterRel=\"%1\" afterEffect=\"1\"").arg(next ? QStringLiteral("nextClick") : QStringLiteral("sameClick"))
                            + (next ? QStringLiteral("/>") : QStringLiteral("><p:stCondLst><p:cond evt=\"end\" delay=\"0\"><p:tn val=\"%1\"/></p:cond></p:stCondLst></p:cTn>").arg(id));
        after = QStringLiteral("<p:subTnLst><p:set><p:cBhvr override=\"childStyle\">%1%2<p:attrNameLst><p:attrName>style.visibility</p:attrName></p:attrNameLst></p:cBhvr><p:to><p:strVal val=\"hidden\"/></p:to></p:set></p:subTnLst>").arg(cTn, targetXml(a));
    }
    return QStringLiteral("<p:par><p:cTn id=\"%1\"%2><p:stCondLst><p:cond delay=\"%3\"/></p:stCondLst><p:childTnLst>%4</p:childTnLst>%5</p:cTn></p:par>").arg(id).arg(attrs).arg(a.delay).arg(body, after);
}

// ---- what a slide's shapes are ----

struct ShapeInfo {
    bool found = false;
    bool sp = false;           // a shape or text box: the program builds it (a p:bldP entry)
    bool placeholder = false;
    bool background = false;   // it has a fill or an outline
    int order = 0;             // where it is among the slide's shapes, bottom first
    int paragraphs = -1;       // how many paragraphs its text has; -1 when that isn't known (no document, or no text)
};

// The story an item's text is in.
QString storyOf(const Item &it)
{
    if (it.type() == ItemType::Text) return static_cast<const TextItem &>(it).storyId;
    if (it.type() == ItemType::Shape) return static_cast<const ShapeItem &>(it).storyId;
    return QString();
}

int paragraphCount(const Document *doc, const Item &it)
{
    const QString id = storyOf(it);
    const QTextDocument *story = doc && !id.isEmpty() ? doc->storyDoc(id) : nullptr;
    return story ? story->blockCount() : -1;
}

// The numbers of the frames the slide keeps as written (a diagram, a chart's frame): shapes it has no item for.
QSet<int> frameNumbers(const Page &slide)
{
    static const QRegularExpression numbers(QStringLiteral("<\\w+:cNvPr\\b[^>]*?\\bid=\"(\\d+)\""));
    QSet<int> out;
    auto scan = [&](const QString &xml) {
        for (auto it = numbers.globalMatch(xml); it.hasNext();) {
            const qint64 n = it.next().captured(1).toLongLong();
            if (n > 0 && n <= 1000000000) out.insert(int(n));
        }
    };
    for (const QJsonValue &v : slide.props.value(QStringLiteral("unread")).toArray()) scan(v.toObject().value(QStringLiteral("xml")).toString());
    for (const QJsonValue &v : slide.props.value(QStringLiteral("charts")).toArray()) scan(v.toObject().value(QStringLiteral("frameXml")).toString());
    return out;
}

QHash<int, ShapeInfo> shapeInfos(const Page &slide, const Document *doc)
{
    QHash<int, ShapeInfo> out;
    int order = 0;
    walkItems(slide.items, [&](const ItemPtr &it) {
        ++order;
        if (it->sourceId <= 0 || out.contains(it->sourceId)) return;
        ShapeInfo s;
        s.order = order;
        s.found = true;
        s.sp = it->type() == ItemType::Text || it->type() == ItemType::Shape;
        s.placeholder = !placeholderType(*it).isEmpty();
        s.background = !it->fill.isNone() || !it->stroke.isNone();
        s.paragraphs = paragraphCount(doc, *it);
        out.insert(it->sourceId, s);
    });
    // A frame has no item of its own, or has one the file's number didn't reach: what an effect on it aims at is there all the same.
    for (int n : frameNumbers(slide)) {
        if (out.contains(n)) continue;
        ShapeInfo s;
        s.found = true;
        s.order = 1 << 29;
        out.insert(n, s);
    }
    return out;
}

// The shape numbers some XML aims at.
QSet<int> shapesOfKept(const QString &xml)
{
    static const QRegularExpression spid(QStringLiteral("\\bspid\\s*=\\s*[\"'](\\d+)[\"']"));
    QSet<int> out;
    for (auto it = spid.globalMatch(xml); it.hasNext();) out.insert(int(std::min<qint64>(it.next().captured(1).toLongLong(), 2000000000)));
    return out;
}

// Whether the shapes (and paragraphs) an effect is on are still on the slide.
bool aimsAtThings(const QHash<int, ShapeInfo> &shapes, const Anim &a)
{
    if (a.custom()) {
        for (int s : shapesOfKept(a.xml))
            if (!shapes.value(s).found) return false;
        return true;
    }
    const ShapeInfo info = shapes.value(a.shape);
    if (!info.found) return false;
    return !(a.paragraphs() && info.paragraphs >= 0 && a.firstParagraph >= info.paragraphs);
}

// The effects of shapes (and paragraphs) the slide no longer has, taken out; the click an effect took goes on to the next of its step.
void dropGoneEffects(AnimSequence &seq, const QHash<int, ShapeInfo> &shapes)
{
    for (int i = int(seq.effects.size()) - 1; i >= 0; --i)
        if (!aimsAtThings(shapes, seq.effects[i])) seq.remove(i);
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
    if (const QDomElement hide = pChild(pChild(pChild(pChild(cTn, "subTnLst"), "set"), "cBhvr"), "cTn"); !hide.isNull()) {
        const QString master = attr(hide, QStringLiteral("masterRel"));
        k.after = master == QLatin1String("nextClick") ? AfterEffect::HideOnNextClick : master == QLatin1String("sameClick") ? AfterEffect::Hide : AfterEffect::None;
    }
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
    Ids ids;
    QString again = effectXml(k, k.group, placeholder, &ids);
    if (again.isEmpty()) return a;
    again.replace(0, 6, QStringLiteral("<p:par xmlns:p=\"%1\"").arg(office::ns::P));
    if (canonOf(again) != canonOf(a.xml)) return a;
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

// What the file wrote for the children of the elements at `path` (their names, from the root), character for character, in order; and the
// namespaces the root declares.
QStringList childTexts(const QString &xml, const QStringList &path, QString *declarations = nullptr)
{
    QStringList out;
    QXmlStreamReader r(xml);
    QStringList open;
    qsizetype start = 0;
    const qsizetype depth = path.size() + 1;
    auto inside = [&] { return open.size() == depth && open.mid(0, path.size()) == path; };
    while (!r.atEnd()) {
        const qsizetype before = r.characterOffset();   // a token starts where the one before it ended
        const QXmlStreamReader::TokenType token = r.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (open.isEmpty() && declarations) {
                declarations->clear();
                for (const QXmlStreamNamespaceDeclaration &n : r.namespaceDeclarations())
                    *declarations += n.prefix().isEmpty() ? QStringLiteral(" xmlns=\"%1\"").arg(esc(n.namespaceUri().toString()))
                                                          : QStringLiteral(" xmlns:%1=\"%2\"").arg(n.prefix().toString(), esc(n.namespaceUri().toString()));
            }
            open.push_back(r.name().toString());
            if (inside()) start = before;
        } else if (token == QXmlStreamReader::EndElement && !open.isEmpty()) {
            if (inside()) out << xml.mid(start, r.characterOffset() - start);
            open.pop_back();
        }
    }
    return out;
}

// The time nodes beside the main sequence, as the file wrote them when that text is what the element says; else written out from the element.
QString keptText(const QDomElement &e, const QStringList &texts, qsizetype at, const QString &declarations)
{
    if (at < texts.size()) {
        QDomDocument dom;
        if (office::parseXml(QStringLiteral("<w%1>%2</w>").arg(declarations, texts[at]).toUtf8(), &dom)) {
            const QDomElement same = dom.documentElement().firstChildElement();
            if (!same.isNull() && same.nextSiblingElement().isNull() && standalone(same) == standalone(e)) return texts[at];
        }
    }
    return standalone(e);
}

} // namespace

AnimSequence readAnimations(const Page &slide, const Document *doc)
{
    AnimSequence seq;
    const QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
    if (xml.isEmpty()) return seq;
    // A timing that can't be read is not an empty sequence that an edit would write over.
    QDomDocument dom;
    if (!office::parseXml(xml.toUtf8(), &dom) || !pIs(dom.documentElement(), "timing")) {
        seq.editable = false;
        return seq;
    }
    const QDomElement root = dom.documentElement();
    bool haveMain = false, tooMany = false;
    // The sequences are the time nodes' children: the tmRoot's, in a p:tnLst.
    const QDomElement rootNode = pChild(pChild(pChild(pChild(root, "tnLst"), "par"), "cTn"), "childTnLst");
    const QVector<QDomElement> nodes = office::children(rootNode);
    QString declarations;
    const QStringList texts = nodes.isEmpty() ? QStringList() : childTexts(xml, {QStringLiteral("timing"), QStringLiteral("tnLst"), QStringLiteral("par"), QStringLiteral("cTn"), QStringLiteral("childTnLst")}, &declarations);
    seq.declarations = declarations;
    if (nodes.size() > kMaxKept) tooMany = true;
    for (qsizetype i = 0; i < nodes.size(); ++i) {
        const QDomElement &s = nodes[i];
        const QDomElement cTn = pIs(s, "seq") ? pChild(s, "cTn") : QDomElement();
        if (!haveMain && !cTn.isNull() && attr(cTn, QStringLiteral("nodeType")) == QLatin1String("mainSeq")) {
            haveMain = true;
            seq.keptBeforeMain = int(seq.kept.size());
            collectEffects(pChild(cTn, "childTnLst"), 0, &seq.effects, &tooMany);
        } else if (!tooMany) {
            seq.kept << keptText(s, texts, i, declarations);
        }
    }
    if (const QDomElement ext = pChild(root, "extLst"); !ext.isNull()) seq.extension = standalone(ext);
    {
        const QVector<QDomElement> builds = office::children(pChild(root, "bldLst"));
        const QStringList buildTexts = builds.isEmpty() ? QStringList() : childTexts(xml, {QStringLiteral("timing"), QStringLiteral("bldLst")});
        for (qsizetype i = 0; i < builds.size(); ++i) seq.builds << keptText(builds[i], buildTexts, i, declarations);
    }
    seq.editable = !tooMany;
    // A sequence whose effects were not all found here (the file lays its time nodes out another way) is shown, and not rewritten.
    if (seq.editable) {
        qsizetype read = 0;
        for (const deck::Step &step : readTimeline(xml).steps)
            for (const deck::Effect &e : step.effects) read += e.implied ? 0 : 1;
        if (read != seq.effects.size()) seq.editable = false;
    }
    if (doc) dropGoneEffects(seq, shapeInfos(slide, doc));
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
    if (kind.cls != EffectClass::Exit) a.after = anim.after;
    anim = a;
}

// ---- shapes ----

int shapeNumber(Page &slide, const QString &itemId)
{
    Item *found = nullptr;
    walkItems(slide.items, [&](const ItemPtr &it) {
        if (it->id == itemId) found = it.get();
    });
    if (!found) return 0;
    if (found->sourceId > 0) return found->sourceId;
    // A number no shape has, no frame the slide keeps, and nothing in the timing aims at (a shape that was deleted had effects that still name it).
    int highest = 1;
    walkItems(slide.items, [&](const ItemPtr &it) { highest = std::max(highest, it->sourceId); });
    for (int n : frameNumbers(slide)) highest = std::max(highest, n);
    for (int n : shapesOfKept(slide.props.value(QStringLiteral("timingXml")).toString())) highest = std::max(highest, n);
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
    const QString storyId = storyOf(*item);
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

// The namespaces the written timing declares: the ones the file's timing did, and the presentation namespace the effects JeffDeck makes use.
QString declarationsFor(const QString &declared)
{
    QString out = declared;
    if (!out.contains(QStringLiteral(" xmlns:p=\"")) && !out.contains(QStringLiteral(" xmlns:p='"))) out += QStringLiteral(" xmlns:p=\"%1\"").arg(office::ns::P);
    return out;
}

// A kept effect whose place in the written XML is a marker, its time nodes numbered afresh (what refers to them follows once all are numbered).
struct KeptEffects {
    struct One {
        QDomDocument dom;
        QDomElement root;
        QHash<QString, QString> renamed;
    };
    QVector<std::shared_ptr<One>> list;
    QHash<QString, QString> everywhere;   // the first node of each old number, for what refers to a node of another effect

    static QString marker(qsizetype i) { return QStringLiteral("\x01%1\x02").arg(i); }

    QString add(const Anim &a, EffectStart start, Ids *ids)
    {
        auto one = std::make_shared<One>();
        if (!office::parseXml(a.xml.toUtf8(), &one->dom)) return QString();
        one->root = one->dom.documentElement();
        if (QDomElement cTn = pChild(one->root, "cTn"); !cTn.isNull()) cTn.setAttribute(QStringLiteral("nodeType"), QLatin1String(nodeTypeOf(start)));
        forEachElement(one->root, [&](QDomElement e) {
            if (e.localName() != QLatin1String("cTn") || !hasAttr(e, QStringLiteral("id"))) return;
            const QString fresh = QString::number(ids->take());
            const QString was = attr(e, QStringLiteral("id"));
            one->renamed.insert(was, fresh);
            if (!everywhere.contains(was)) everywhere.insert(was, fresh);
            e.setAttribute(QStringLiteral("id"), fresh);
        });
        list.push_back(one);
        return marker(list.size() - 1);
    }

    // The XML with each marker replaced by its effect, written out.
    QString fill(QString xml) const
    {
        for (qsizetype i = 0; i < list.size(); ++i) {
            const auto &one = list[i];
            forEachElement(one->root, [&](QDomElement e) {
                if (e.localName() != QLatin1String("tn")) return;
                const QString was = attr(e, QStringLiteral("val"));
                if (const auto here = one->renamed.constFind(was); here != one->renamed.constEnd()) e.setAttribute(QStringLiteral("val"), here.value());
                else if (const auto other = everywhere.constFind(was); other != everywhere.constEnd()) e.setAttribute(QStringLiteral("val"), other.value());
            });
            xml.replace(marker(i), standalone(one->root));
        }
        return xml;
    }
};

} // namespace

QString animationsXml(const Document &doc, const Page &slide, const AnimSequence &seq)
{
    const QHash<int, ShapeInfo> shapes = shapeInfos(slide, &doc);
    // The effects that still have a shape (and paragraphs) to play on.
    AnimSequence live = seq;
    dropGoneEffects(live, shapes);
    const QVector<Anim> &effects = live.effects;
    if (effects.isEmpty() && seq.kept.isEmpty()) return QString();

    // Which build each effect belongs to: per shape, the groups in the order they come, numbered from 0 around the ones kept effects hold.
    QHash<int, QSet<int>> taken;
    for (const Anim &a : effects)
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

    // What the file keeps beside the main sequence keeps its own numbers; the rest are numbered around them, in document order: each
    // click step, each part of it, each effect.
    const QString declared = declarationsFor(seq.declarations);
    Ids ids;
    QSet<int> aimedAt;
    for (const QString &k : seq.kept) {
        ids.reserved.unite(timeNodeIds(k, declared));
        aimedAt.unite(shapesOfKept(k));
    }
    const int rootId = ids.take(), mainId = ids.take();
    const QVector<Placed> placed = layoutOf(effects);
    KeptEffects keptEffects;
    QString mainXml, parts, inside;
    struct Build {
        int order, group;
        QString xml;
    };
    QVector<Build> builds;   // the program lists them by where the shape is on the slide
    QSet<QPair<int, int>> built;
    int step = -1, part = -1, clickId = 0, partId = 0, partDelay = 0;
    bool automatic = false;
    auto closePart = [&] {
        if (part < 0) return;
        parts += QStringLiteral("<p:par><p:cTn id=\"%1\" fill=\"hold\"><p:stCondLst><p:cond delay=\"%2\"/></p:stCondLst><p:childTnLst>").arg(partId).arg(partDelay) + inside + QStringLiteral("</p:childTnLst></p:cTn></p:par>");
        inside.clear();
        part = -1;
    };
    auto closeStep = [&] {
        closePart();
        if (step < 0) return;
        const QString begin = automatic ? QStringLiteral("<p:cond evt=\"onBegin\" delay=\"0\"><p:tn val=\"%1\"/></p:cond>").arg(mainId) : QString();
        mainXml += QStringLiteral("<p:par><p:cTn id=\"%1\" fill=\"hold\"><p:stCondLst><p:cond delay=\"indefinite\"/>%2</p:stCondLst><p:childTnLst>").arg(clickId).arg(begin) + parts + QStringLiteral("</p:childTnLst></p:cTn></p:par>");
        parts.clear();
        step = -1;
    };
    for (int i = 0; i < effects.size(); ++i) {
        const Anim &a = effects[i];
        if (step != placed[i].click || step < 0) {
            closeStep();
            step = placed[i].click;
            clickId = ids.take();
            automatic = placed[i].click == 0;
            part = -1;
        }
        if (part != placed[i].inner || part < 0) {
            closePart();
            part = placed[i].inner;
            partId = ids.take();
            partDelay = placed[i].innerStart;
        }
        if (a.custom()) {
            inside += keptEffects.add(a, a.start, &ids);
            aimedAt.unite(shapesOfKept(a.xml));
            continue;
        }
        const ShapeInfo &si = shapes.value(a.shape);
        inside += effectXml(a, grp[i], si.placeholder && !a.paragraphs(), &ids);
        aimedAt.insert(a.shape);
        if (si.sp && !built.contains({a.shape, grp[i]})) {
            built.insert({a.shape, grp[i]});
            builds.push_back({si.order, grp[i], QStringLiteral("<p:bldP spid=\"%1\" grpId=\"%2\"%3%4/>").arg(a.shape).arg(grp[i]).arg(a.paragraphs() ? QStringLiteral(" build=\"p\"") : QString(), si.background ? QStringLiteral(" animBg=\"1\"") : QString())});
        }
    }
    closeStep();
    // The builds of what the file holds (charts, diagrams) stay as long as a shape they are for is still aimed at, by the main sequence or another.
    for (const QString &b : seq.builds) {
        static const QRegularExpression spid(QStringLiteral("\\bspid\\s*=\\s*[\"'](\\d+)[\"']")), grpIdRe(QStringLiteral("\\bgrpId\\s*=\\s*[\"'](\\d+)[\"']"));
        const QRegularExpressionMatch m = spid.match(b), g = grpIdRe.match(b);
        const int shape = m.hasMatch() ? int(m.captured(1).toLongLong()) : -1;
        const int group = g.hasMatch() ? int(g.captured(1).toLongLong()) : -1;
        if (shape > 0 && (built.contains({shape, group}) || !aimedAt.contains(shape))) continue;
        builds.push_back({shapes.contains(shape) ? shapes.value(shape).order : 1 << 30, group, b});
    }
    std::stable_sort(builds.begin(), builds.end(), [](const Build &x, const Build &y) { return x.order != y.order ? x.order < y.order : x.group < y.group; });
    QString buildXml;
    for (const Build &b : std::as_const(builds)) buildXml += b.xml;

    const int before = std::clamp(seq.keptBeforeMain, 0, int(seq.kept.size()));
    QString xml = QStringLiteral("<p:timing%1><p:tnLst><p:par><p:cTn id=\"%2\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\"><p:childTnLst>").arg(declared, QString::number(rootId));
    for (int i = 0; i < before; ++i) xml += seq.kept[i];
    xml += QStringLiteral("<p:seq concurrent=\"1\" nextAc=\"seek\"><p:cTn id=\"%1\" dur=\"indefinite\" nodeType=\"mainSeq\"><p:childTnLst>").arg(mainId) + keptEffects.fill(mainXml) + QStringLiteral("</p:childTnLst></p:cTn>");
    xml += QStringLiteral("<p:prevCondLst><p:cond evt=\"onPrev\" delay=\"0\"><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:prevCondLst>"
                          "<p:nextCondLst><p:cond evt=\"onNext\" delay=\"0\"><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:nextCondLst></p:seq>");
    for (qsizetype i = before; i < seq.kept.size(); ++i) xml += seq.kept[i];
    xml += QStringLiteral("</p:childTnLst></p:cTn></p:par></p:tnLst>");
    if (!buildXml.isEmpty()) xml += QStringLiteral("<p:bldLst>") + buildXml + QStringLiteral("</p:bldLst>");
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
    const qsizetype total = seq.effects.size();
    // What is gone: the shapes the file doesn't get, and the paragraphs the text no longer has.
    const QHash<int, ShapeInfo> shapes = shapeInfos(slide, &doc);
    for (int i = int(seq.effects.size()) - 1; i >= 0; --i) {
        const Anim &a = seq.effects[i];
        bool missing = !aimsAtThings(shapes, a);
        if (a.custom()) {
            for (int s : shapesOfKept(a.xml)) missing |= !present(s);
        } else {
            missing |= !present(a.shape);
        }
        if (missing) seq.remove(i);
    }
    const int gone = int(total - seq.effects.size());
    if (gone == 0) return was;
    if (dropped) *dropped = gone;
    return animationsXml(doc, slide, seq);
}

quint64 animationSignature(const Document &doc, const Page &slide)
{
    const QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
    if (xml.isEmpty()) return 0;
    quint64 h = 1469598103934665603ULL;
    auto mix = [&h](quint64 v) { h = (h ^ v) * 1099511628211ULL; };
    mix(qHash(xml));
    mix(quint64(xml.size()));
    walkItems(slide.items, [&](const ItemPtr &it) {
        if (it->sourceId <= 0) return;
        mix(quint64(it->sourceId));
        mix(qHash(it->name));
        mix(quint64(paragraphCount(&doc, *it) + 1));
    });
    mix(quint64(slide.props.value(QStringLiteral("unread")).toArray().size()));
    mix(quint64(slide.props.value(QStringLiteral("charts")).toArray().size()));
    return h | 1;
}

Timeline playableTimeline(const Document &doc, const Page &slide)
{
    Timeline tl = timelineOf(slide);
    if (!tl.steps.isEmpty()) {
        // What is on shapes (and paragraphs) the slide no longer has takes no part, and no click.
        const QHash<int, ShapeInfo> shapes = shapeInfos(slide, &doc);
        auto gone = [&](const Effect &e) {
            for (const Behavior &b : e.behaviors) {
                const Target &t = b.target;
                if (t.shape <= 0) continue;
                const ShapeInfo info = shapes.value(t.shape);
                if (!info.found || (t.paragraphs() && info.paragraphs >= 0 && t.firstParagraph >= info.paragraphs)) return true;
            }
            return false;
        };
        for (Step &step : tl.steps) step.effects.erase(std::remove_if(step.effects.begin(), step.effects.end(), gone), step.effects.end());
    }
    tl.steps.erase(std::remove_if(tl.steps.begin(), tl.steps.end(), [](const Step &s) { return !s.visual(); }), tl.steps.end());
    return tl;
}

} // namespace jp::deck
