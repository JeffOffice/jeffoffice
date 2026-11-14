#include "office/drawingml_effects.h"

#include <QList>
#include <QString>
#include <algorithm>

namespace jp::office {

namespace {

struct Element {
    QByteArray local, raw;
};

QByteArray localNameOf(const QByteArray &raw)
{
    qsizetype i = 1;
    qsizetype end = i;
    while (end < raw.size() && raw[end] != ' ' && raw[end] != '>' && raw[end] != '/' && raw[end] != '\t' && raw[end] != '\n') ++end;
    const QByteArray name = raw.mid(i, end - i);
    const qsizetype colon = name.indexOf(':');
    return colon >= 0 ? name.mid(colon + 1) : name;
}

// The top-level elements of a fragment of elements, each as written.
QVector<Element> elementsOf(const QByteArray &fragment)
{
    QVector<Element> out;
    qsizetype i = 0, begin = -1;
    int depth = 0;
    while (i < fragment.size()) {
        const qsizetype lt = fragment.indexOf('<', i);
        if (lt < 0) break;
        const qsizetype gt = fragment.indexOf('>', lt);
        if (gt < 0) break;
        const bool closing = fragment[lt + 1] == '/', selfClosing = fragment[gt - 1] == '/';
        if (closing) {
            if (--depth == 0 && begin >= 0) {
                const QByteArray raw = fragment.mid(begin, gt + 1 - begin);
                out.push_back({localNameOf(raw), raw});
                begin = -1;
            }
        } else if (selfClosing) {
            if (depth == 0) {
                const QByteArray raw = fragment.mid(lt, gt + 1 - lt);
                out.push_back({localNameOf(raw), raw});
            }
        } else {
            if (depth++ == 0) begin = lt;
        }
        i = gt + 1;
    }
    return out;
}

// The children of the element `raw` (its content), and its start tag.
void splitElement(const QByteArray &raw, QByteArray *start, QByteArray *content)
{
    const qsizetype gt = raw.indexOf('>');
    if (gt < 0 || raw[gt - 1] == '/') {
        *start = raw;
        content->clear();
        return;
    }
    *start = raw.left(gt + 1);
    const qsizetype close = raw.lastIndexOf("</");
    *content = close > gt ? raw.mid(gt + 1, close - gt - 1) : QByteArray();
}

int orderOf(const QByteArray &local)
{
    static const QList<QByteArray> order = {"blur", "fillOverlay", "glow", "innerShdw", "outerShdw", "prstShdw", "reflection", "softEdge"};
    const int at = int(order.indexOf(local));
    return at < 0 ? int(order.size()) : at;
}

QByteArray nameOf(ShapeEffectKind kind)
{
    switch (kind) {
    case ShapeEffectKind::Shadow: return "outerShdw";
    case ShapeEffectKind::Glow: return "glow";
    case ShapeEffectKind::Reflection: return "reflection";
    case ShapeEffectKind::SoftEdges: return "softEdge";
    case ShapeEffectKind::Bevel: return "bevelT";
    }
    return {};
}

QByteArray usualElement(ShapeEffectKind kind)
{
    switch (kind) {
    case ShapeEffectKind::Shadow:
        return "<a:outerShdw blurRad=\"50800\" dist=\"38100\" dir=\"2700000\" algn=\"tl\" rotWithShape=\"0\"><a:prstClr val=\"black\"><a:alpha val=\"40000\"/></a:prstClr></a:outerShdw>";
    case ShapeEffectKind::Glow: return "<a:glow rad=\"101600\"><a:srgbClr val=\"5B9BD5\"><a:alpha val=\"40000\"/></a:srgbClr></a:glow>";
    case ShapeEffectKind::Reflection:
        return "<a:reflection blurRad=\"6350\" stA=\"50000\" endA=\"300\" endPos=\"55000\" dir=\"5400000\" sy=\"-100000\" algn=\"bl\" rotWithShape=\"0\"/>";
    case ShapeEffectKind::SoftEdges: return "<a:softEdge rad=\"63500\"/>";
    case ShapeEffectKind::Bevel: return "<a:bevelT w=\"63500\" h=\"25400\"/>";
    }
    return {};
}

} // namespace

const QVector<ShapeEffectKind> &shapeEffectKinds()
{
    static const QVector<ShapeEffectKind> kinds = {ShapeEffectKind::Shadow, ShapeEffectKind::Glow, ShapeEffectKind::Reflection, ShapeEffectKind::SoftEdges, ShapeEffectKind::Bevel};
    return kinds;
}

QByteArray effectListWith(const QByteArray &effectLst, ShapeEffectKind kind, bool on)
{
    if (kind == ShapeEffectKind::Bevel) return effectLst;
    QByteArray start, content;
    if (!effectLst.isEmpty()) splitElement(effectLst, &start, &content);
    QVector<Element> items = elementsOf(content);
    const QByteArray name = nameOf(kind);
    items.erase(std::remove_if(items.begin(), items.end(), [&](const Element &e) { return e.local == name; }), items.end());
    if (on) items.push_back({name, usualElement(kind)});
    if (items.isEmpty()) return {};
    std::stable_sort(items.begin(), items.end(), [](const Element &a, const Element &b) { return orderOf(a.local) < orderOf(b.local); });
    QByteArray out = start.isEmpty() || start.endsWith("/>") ? QByteArray("<a:effectLst>") : start;
    for (const Element &e : items) out += e.raw;
    return out + "</a:effectLst>";
}

bool hasShapeEffect(const QByteArray &fragment, ShapeEffectKind kind)
{
    const QByteArray name = nameOf(kind);
    QByteArray start, content;
    if (fragment.isEmpty()) return false;
    splitElement(fragment, &start, &content);
    for (const Element &e : elementsOf(content))
        if (e.local == name) return true;
    return false;
}

Solid3d bevelWith(const Solid3d &now, bool on)
{
    Solid3d out = now;
    QByteArray start = "<a:sp3d>", content;
    if (!now.solid.isEmpty()) splitElement(now.solid, &start, &content);
    QVector<Element> items = elementsOf(content);
    items.erase(std::remove_if(items.begin(), items.end(), [](const Element &e) { return e.local == "bevelT"; }), items.end());
    if (on) {
        items.push_front({"bevelT", usualElement(ShapeEffectKind::Bevel)});
        if (start.endsWith("/>")) start = start.left(start.size() - 2) + ">";
        QByteArray solid = start;
        for (const Element &e : items) solid += e.raw;
        out.solid = solid + "</a:sp3d>";
        if (out.scene.isEmpty()) out.scene = "<a:scene3d><a:camera prst=\"orthographicFront\"/><a:lightRig rig=\"threePt\" dir=\"t\"/></a:scene3d>";
        return out;
    }
    if (now.solid.isEmpty()) return out;
    const bool plainStart = start.trimmed() == "<a:sp3d>" || start.trimmed() == "<a:sp3d/>";
    if (items.isEmpty() && plainStart) return {};   // (the bevel was all there was)
    QByteArray solid = start.endsWith("/>") ? start.left(start.size() - 2) + ">" : start;
    for (const Element &e : items) solid += e.raw;
    out.solid = solid + "</a:sp3d>";
    return out;
}

} // namespace jp::office
