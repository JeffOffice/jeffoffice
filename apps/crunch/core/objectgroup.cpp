#include "core/objectgroup.h"

#include "office/xml.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QDomElement>
#include <QHash>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>

#include <algorithm>
#include <cmath>

namespace jp::crunch {

namespace {

constexpr double kEmuPerUnit = 9525;
const char *const kDrawingNs = "http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing";
const char *const kMainNs = "http://schemas.openxmlformats.org/drawingml/2006/main";
const char *const kRelNs = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

qint64 emu(double units) { return qint64(std::llround(units * kEmuPerUnit)); }

// The element of a clip in a document of its own (the wrapper declares the usual namespaces and the clip's).
bool parseClip(const ObjectClip &clip, QDomDocument *doc, QDomElement *el)
{
    const QByteArray wrapped = QByteArray("<w xmlns:xdr=\"") + kDrawingNs + "\" xmlns:a=\"" + kMainNs + "\" xmlns:r=\"" + kRelNs + "\"" + clip.namespaces + ">" + clip.elementXml + "</w>";
    if (!jp::office::parseXml(wrapped, doc)) return false;
    *el = doc->documentElement().firstChildElement();
    return !el->isNull();
}

QByteArray savedElement(const QDomElement &el)
{
    QString text;
    QTextStream ts(&text);
    el.save(ts, -1);
    return text.toUtf8();
}

// The element that holds an object's `a:xfrm`: its shape properties (or its group properties).
QDomElement propertiesOf(const QDomElement &el)
{
    const QString local = el.localName();
    for (QDomElement c = el.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == (local == QLatin1String("grpSp") ? QLatin1String("grpSpPr") : QLatin1String("spPr"))) return c;
    return QDomElement();
}

QDomElement xfrmOf(QDomElement props, bool make)
{
    for (QDomElement c = props.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == QLatin1String("xfrm")) return c;
    if (!make) return QDomElement();
    QDomElement x = props.ownerDocument().createElementNS(QString::fromLatin1(kMainNs), QStringLiteral("a:xfrm"));
    props.insertBefore(x, props.firstChild());
    return x;
}

QDomElement pairIn(QDomElement xfrm, const char *local, bool make)
{
    for (QDomElement c = xfrm.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == QLatin1String(local)) return c;
    if (!make) return QDomElement();
    QDomElement e = xfrm.ownerDocument().createElementNS(QString::fromLatin1(kMainNs), QStringLiteral("a:") + QLatin1String(local));
    // (off, ext, chOff, chExt: in that order)
    static const char *const order[] = {"off", "ext", "chOff", "chExt"};
    int mine = 0;
    for (int i = 0; i < 4; ++i)
        if (QLatin1String(order[i]) == QLatin1String(local)) mine = i;
    QDomNode before;
    for (QDomElement c = xfrm.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        int at = 4;
        for (int i = 0; i < 4; ++i)
            if (QLatin1String(order[i]) == c.localName()) at = i;
        if (at > mine) {
            before = c;
            break;
        }
    }
    if (before.isNull()) xfrm.appendChild(e);
    else xfrm.insertBefore(e, before);
    return e;
}

// Puts a box (grid units) in the object's xfrm as its place and size.
bool setPlace(QDomElement el, const jp::grid::ObjectBox &box)
{
    const QDomElement props = propertiesOf(el);
    if (props.isNull()) return false;
    QDomElement x = xfrmOf(props, true);
    QDomElement off = pairIn(x, "off", true), ext = pairIn(x, "ext", true);
    off.setAttribute(QStringLiteral("x"), QString::number(emu(box.x)));
    off.setAttribute(QStringLiteral("y"), QString::number(emu(box.y)));
    ext.setAttribute(QStringLiteral("cx"), QString::number(emu(std::max(0.0, box.w))));
    ext.setAttribute(QStringLiteral("cy"), QString::number(emu(std::max(0.0, box.h))));
    return true;
}

double attr(const QDomElement &e, const char *name, double fallback = 0)
{
    bool ok = false;
    const double v = e.attribute(QLatin1String(name)).toDouble(&ok);
    return ok && std::isfinite(v) ? v : fallback;
}

QByteArray namespaceDeclarations(const QList<QByteArray> &lists)
{
    static const QRegularExpression one(QStringLiteral("\\s+(xmlns:[A-Za-z_][\\w.\\-]*)=\"([^\"]*)\""));
    QSet<QString> seen;
    QByteArray out;
    for (const QByteArray &l : lists) {
        auto it = one.globalMatch(QString::fromUtf8(l));
        while (it.hasNext()) {
            const auto m = it.next();
            if (seen.contains(m.captured(1))) continue;
            seen.insert(m.captured(1));
            out += " " + m.captured(1).toUtf8() + "=\"" + m.captured(2).toUtf8() + "\"";
        }
    }
    return out;
}

} // namespace

std::vector<jp::grid::ObjectBox> alignedBoxes(const std::vector<jp::grid::ObjectBox> &boxes, ObjectAlign how)
{
    std::vector<jp::grid::ObjectBox> out = boxes;
    if (boxes.size() < 2) return out;
    double left = boxes[0].x, top = boxes[0].y, right = boxes[0].x + boxes[0].w, bottom = boxes[0].y + boxes[0].h;
    for (const jp::grid::ObjectBox &b : boxes) {
        left = std::min(left, b.x);
        top = std::min(top, b.y);
        right = std::max(right, b.x + b.w);
        bottom = std::max(bottom, b.y + b.h);
    }
    switch (how) {
    case ObjectAlign::Left:
        for (auto &b : out) b.x = left;
        break;
    case ObjectAlign::Right:
        for (auto &b : out) b.x = right - b.w;
        break;
    case ObjectAlign::Center:
        for (auto &b : out) b.x = (left + right) / 2 - b.w / 2;
        break;
    case ObjectAlign::Top:
        for (auto &b : out) b.y = top;
        break;
    case ObjectAlign::Bottom:
        for (auto &b : out) b.y = bottom - b.h;
        break;
    case ObjectAlign::Middle:
        for (auto &b : out) b.y = (top + bottom) / 2 - b.h / 2;
        break;
    case ObjectAlign::DistributeHorizontally:
    case ObjectAlign::DistributeVertically: {
        if (boxes.size() < 3) break;
        const bool horizontal = how == ObjectAlign::DistributeHorizontally;
        std::vector<size_t> order(boxes.size());
        for (size_t i = 0; i < order.size(); ++i) order[i] = i;
        std::stable_sort(order.begin(), order.end(), [&](size_t a, size_t b) { return horizontal ? boxes[a].x < boxes[b].x : boxes[a].y < boxes[b].y; });
        double sum = 0;
        for (const auto &b : boxes) sum += horizontal ? b.w : b.h;
        const double gap = ((horizontal ? right - left : bottom - top) - sum) / double(boxes.size() - 1);
        double at = horizontal ? left : top;
        for (const size_t i : order) {
            (horizontal ? out[i].x : out[i].y) = at;
            at += (horizontal ? boxes[i].w : boxes[i].h) + gap;
        }
        break;
    }
    }
    return out;
}

bool buildGroupClip(const std::vector<GroupMember> &members, ObjectClip *group, jp::grid::ObjectBox *box, QString *why)
{
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    if (members.size() < 2) return refuse(QCoreApplication::translate("jp::crunch::ObjectGroup", "Pick two or more objects to group."));
    if (members.size() > size_t(kMaxGroupMembers)) return refuse(QCoreApplication::translate("jp::crunch::ObjectGroup", "There are more objects here than one group holds."));
    double left = members[0].box.x, top = members[0].box.y, right = left + members[0].box.w, bottom = top + members[0].box.h;
    for (const GroupMember &m : members) {
        if (!m.clip.valid() || m.clip.kind == QLatin1String("chart")) return refuse(QCoreApplication::translate("jp::crunch::ObjectGroup", "A chart can't be put in a group."));
        left = std::min(left, m.box.x);
        top = std::min(top, m.box.y);
        right = std::max(right, m.box.x + m.box.w);
        bottom = std::max(bottom, m.box.y + m.box.h);
    }
    const jp::grid::ObjectBox all{left, top, right - left, bottom - top};
    QByteArray body;
    QList<ClipMedia> media;
    QList<QByteArray> spaces;
    for (const GroupMember &m : members) {
        QDomDocument doc;
        QDomElement el;
        if (!parseClip(m.clip, &doc, &el)) return refuse(QCoreApplication::translate("jp::crunch::ObjectGroup", "An object here can't be read, so the group can't be made."));
        if (!setPlace(el, m.box)) return refuse(QCoreApplication::translate("jp::crunch::ObjectGroup", "An object here can't be put in a group."));
        QByteArray xml = savedElement(el);
        // its pictures keep their bytes, under tokens that no other member of the group has (through a mark first: a new token may be another member picture's old one)
        QVector<QPair<QString, QString>> renamed;
        for (const ClipMedia &cm : m.clip.media) {
            if (media.size() >= kMaxClipMedia) return refuse(QCoreApplication::translate("jp::crunch::ObjectGroup", "The objects here use more pictures than one group holds."));
            ClipMedia moved = cm;
            moved.token = QStringLiteral("rIdPaste%1Z").arg(media.size());
            renamed.append({cm.token, moved.token});
            media.append(moved);
        }
        for (int i = 0; i < renamed.size(); ++i) xml.replace(renamed[i].first.toUtf8(), "@@" + QByteArray::number(i) + "@@");
        for (int i = 0; i < renamed.size(); ++i) xml.replace("@@" + QByteArray::number(i) + "@@", renamed[i].second.toUtf8());
        body += xml;
        spaces << m.clip.namespaces;
    }
    auto pair = [](const char *name, const char *a, const char *b, double x, double y) {
        return QByteArray("<a:") + name + " " + a + "=\"" + QByteArray::number(emu(x)) + "\" " + b + "=\"" + QByteArray::number(emu(y)) + "\"/>";
    };
    ObjectClip out;
    out.kind = QStringLiteral("group");
    out.width = all.w;
    out.height = all.h;
    out.namespaces = namespaceDeclarations(spaces);
    out.media = media;
    out.elementXml = "<xdr:grpSp><xdr:nvGrpSpPr><xdr:cNvPr id=\"0\" name=\"\"/><xdr:cNvGrpSpPr/></xdr:nvGrpSpPr><xdr:grpSpPr><a:xfrm>" + pair("off", "x", "y", all.x, all.y) + pair("ext", "cx", "cy", all.w, all.h)
                     + pair("chOff", "x", "y", all.x, all.y) + pair("chExt", "cx", "cy", all.w, all.h) + "</a:xfrm></xdr:grpSpPr>" + body + "</xdr:grpSp>";
    if (out.elementXml.size() > kMaxClipElementBytes) return refuse(QCoreApplication::translate("jp::crunch::ObjectGroup", "These objects are too large to put in one group."));
    if (box) *box = all;
    if (group) *group = out;
    return true;
}

bool splitGroupClip(const ObjectClip &clip, const jp::grid::ObjectBox &groupBox, std::vector<GroupMember> *members, QString *why)
{
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    if (!clip.valid() || clip.kind != QLatin1String("group")) return refuse(QCoreApplication::translate("jp::crunch::ObjectGroup", "This is not a group."));
    QDomDocument doc;
    QDomElement el;
    if (!parseClip(clip, &doc, &el) || el.localName() != QLatin1String("grpSp")) return refuse(QCoreApplication::translate("jp::crunch::ObjectGroup", "The group can't be read."));
    const QDomElement props = propertiesOf(el);
    const QDomElement x = props.isNull() ? QDomElement() : xfrmOf(props, false);
    const QDomElement chOff = x.isNull() ? QDomElement() : pairIn(x, "chOff", false), chExt = x.isNull() ? QDomElement() : pairIn(x, "chExt", false);
    if (chOff.isNull() || chExt.isNull()) return refuse(QCoreApplication::translate("jp::crunch::ObjectGroup", "The group can't be taken apart: it does not say how its objects are laid out."));
    const double cx0 = attr(chOff, "x"), cy0 = attr(chOff, "y"), cw = attr(chExt, "cx"), ch = attr(chExt, "cy");
    // (a group that says its objects' space has no width or height: they are on the sheet's own scale)
    const double sx = cw > 0 ? groupBox.w / (cw / kEmuPerUnit) : 1, sy = ch > 0 ? groupBox.h / (ch / kEmuPerUnit) : 1;
    std::vector<GroupMember> out;
    for (QDomElement c = el.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        const QString local = c.localName();
        if (local != QLatin1String("sp") && local != QLatin1String("pic") && local != QLatin1String("cxnSp") && local != QLatin1String("grpSp")) continue;
        if (out.size() >= size_t(kMaxGroupMembers)) return refuse(QCoreApplication::translate("jp::crunch::ObjectGroup", "The group holds more objects than JeffCrunch takes apart."));
        const QDomElement p = propertiesOf(c);
        const QDomElement cx = p.isNull() ? QDomElement() : xfrmOf(p, false);
        const QDomElement off = cx.isNull() ? QDomElement() : pairIn(cx, "off", false), ext = cx.isNull() ? QDomElement() : pairIn(cx, "ext", false);
        if (off.isNull() || ext.isNull()) continue;
        GroupMember m;
        m.box.x = groupBox.x + (attr(off, "x") - cx0) / kEmuPerUnit * sx;
        m.box.y = groupBox.y + (attr(off, "y") - cy0) / kEmuPerUnit * sy;
        m.box.w = attr(ext, "cx") / kEmuPerUnit * sx;
        m.box.h = attr(ext, "cy") / kEmuPerUnit * sy;
        QDomElement copy = c.cloneNode(true).toElement();
        if (!setPlace(copy, m.box)) continue;
        m.clip.kind = local == QLatin1String("pic") ? QStringLiteral("picture") : local == QLatin1String("grpSp") ? QStringLiteral("group") : local == QLatin1String("cxnSp") ? QStringLiteral("connector") : QStringLiteral("shape");
        m.clip.origin = clip.origin;
        m.clip.sheetName = clip.sheetName;
        m.clip.width = std::max(4.0, m.box.w);
        m.clip.height = std::max(4.0, m.box.h);
        m.clip.namespaces = clip.namespaces;
        m.clip.elementXml = savedElement(copy);
        for (const ClipMedia &cm : clip.media)
            if (m.clip.elementXml.contains(cm.token.toUtf8())) m.clip.media.append(cm);
        out.push_back(m);
    }
    if (out.empty()) return refuse(QCoreApplication::translate("jp::crunch::ObjectGroup", "The group holds nothing that can be taken apart."));
    if (members) *members = out;
    return true;
}

} // namespace jp::crunch
