#include "sheet/drawing.h"
#include "office/package.h"

#include "office/chart.h"
#include "office/diagram.h"
#include "office/drawingml.h"
#include "office/xml.h"
#include "sheet/cellref.h"
#include "sheet/xmlscan.h"

#include <QRegularExpression>

#include <algorithm>

namespace jp::office::sheet {

namespace {

QDomElement childByLocal(const QDomElement &e, const char *local)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == QLatin1String(local)) return c;
    return QDomElement();
}

qint64 emuOf(const QString &text)
{
    bool ok = false;
    const double points = parseLength(text.trimmed(), 0, &ok);
    if (!ok) return 0;
    const double emu = points * double(EmuPerPoint);
    return emu <= 0 ? 0 : emu >= double(kMaxDrawingEmu) ? kMaxDrawingEmu : qint64(emu + 0.5);
}

quint32 indexOf(const QString &text, quint32 limit)
{
    bool ok = false;
    const qint64 v = parseInt(text.trimmed(), 0, &ok);
    return !ok || v < 0 ? 0 : quint32(std::min<qint64>(v, qint64(limit) - 1));
}

AnchorPoint pointOf(const QDomElement &e)
{
    AnchorPoint p;
    p.col = indexOf(childByLocal(e, "col").text(), kMaxColumns);
    p.row = indexOf(childByLocal(e, "row").text(), kMaxRows);
    p.colOff = emuOf(childByLocal(e, "colOff").text());
    p.rowOff = emuOf(childByLocal(e, "rowOff").text());
    return p;
}

AnchorKind editAsOf(const QString &text)
{
    if (text == QLatin1String("oneCell")) return AnchorKind::OneCell;
    if (text == QLatin1String("absolute")) return AnchorKind::Absolute;
    return AnchorKind::TwoCell;
}

// The object an anchor holds: its kind, name, and the rest, and whether anything of it is read.
bool readObject(const QDomElement &anchor, const Package &package, const QString &part, DrawingObject *o)
{
    for (QDomElement c = anchor.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        const QString local = c.localName();
        QDomElement nv;
        if (local == QLatin1String("sp")) {
            o->kind = ObjectKind::Shape;
            nv = childByLocal(c, "nvSpPr");
        } else if (local == QLatin1String("pic")) {
            o->kind = ObjectKind::Picture;
            nv = childByLocal(c, "nvPicPr");
        } else if (local == QLatin1String("grpSp")) {
            o->kind = ObjectKind::Group;
            nv = childByLocal(c, "nvGrpSpPr");
        } else if (local == QLatin1String("cxnSp")) {
            o->kind = ObjectKind::Connector;
            nv = childByLocal(c, "nvCxnSpPr");
        } else if (local == QLatin1String("graphicFrame")) {
            nv = childByLocal(c, "nvGraphicFramePr");
            const QString id = chartFrameRelId(c);
            if (id.isEmpty()) {
                if (!isDiagramFrame(c)) continue;   // (a table, a slicer: not read)
                // A diagram: drawn from the drawing its data part names, found when the object is made (the part may be large, and need not be read to list the drawing).
                o->kind = ObjectKind::Diagram;
                o->diagramData = diagramFrameLinks(c).data;
                const QDomElement ext = childByLocal(childByLocal(c, "xfrm"), "ext");
                o->diagramCx = emuOf(attr(ext, QStringLiteral("cx")));
                o->diagramCy = emuOf(attr(ext, QStringLiteral("cy")));
            } else {
                const Relationship rel = package.relationship(part, id);
                if (rel.id.isEmpty() || rel.external || !package.hasPart(rel.target)) continue;
                o->kind = ObjectKind::Chart;
                o->chartPart = rel.target;
            }
        } else {
            continue;
        }
        const QDomElement cNvPr = childByLocal(nv, "cNvPr");
        o->element = c;
        o->name = attr(cNvPr, QStringLiteral("name")).left(256);
        o->description = attr(cNvPr, QStringLiteral("descr")).left(1024);
        o->id = parseInt(attr(cNvPr, QStringLiteral("id")), 0);
        o->hidden = parseBool(attr(cNvPr, QStringLiteral("hidden")));
        const QString prints = attr(childByLocal(anchor, "clientData"), QStringLiteral("fPrintsWithSheet"));
        o->prints = prints.isEmpty() || parseBool(prints);
        return true;
    }
    return false;
}

// Whether a shape's text holds math (a14:m), which the shape's own text does not show: an equation.
bool holdsMath(const QDomElement &shape)
{
    const QDomElement body = childByLocal(shape, "txBody");
    for (QDomElement p = body.firstChildElement(); !p.isNull(); p = p.nextSiblingElement())
        for (QDomElement r = p.firstChildElement(); !r.isNull(); r = r.nextSiblingElement())
            if (r.localName() == QLatin1String("m") && r.namespaceURI().endsWith(QLatin1String("2010/main"))) return true;
    return false;
}

bool isAnchor(const QString &local) { return local == QLatin1String("twoCellAnchor") || local == QLatin1String("oneCellAnchor") || local == QLatin1String("absoluteAnchor"); }

DrawingAnchor readAnchor(const QDomElement &a)
{
    DrawingAnchor d;
    const QString local = a.localName();
    d.kind = local == QLatin1String("oneCellAnchor") ? AnchorKind::OneCell : local == QLatin1String("absoluteAnchor") ? AnchorKind::Absolute : AnchorKind::TwoCell;
    d.editAs = d.kind == AnchorKind::TwoCell ? editAsOf(attr(a, QStringLiteral("editAs"))) : d.kind;
    if (d.kind == AnchorKind::Absolute) {
        const QDomElement pos = childByLocal(a, "pos"), ext = childByLocal(a, "ext");
        d.x = emuOf(attr(pos, QStringLiteral("x")));
        d.y = emuOf(attr(pos, QStringLiteral("y")));
        d.cx = emuOf(attr(ext, QStringLiteral("cx")));
        d.cy = emuOf(attr(ext, QStringLiteral("cy")));
        return d;
    }
    d.from = pointOf(childByLocal(a, "from"));
    if (d.kind == AnchorKind::TwoCell) {
        d.to = pointOf(childByLocal(a, "to"));
    } else {
        const QDomElement ext = childByLocal(a, "ext");
        d.cx = emuOf(attr(ext, QStringLiteral("cx")));
        d.cy = emuOf(attr(ext, QStringLiteral("cy")));
    }
    return d;
}

} // namespace

namespace {

// A row or column (from 0) after the edits to the rows or columns of the sheet: a place at or after an inserted row moves with it, one in rows that are deleted goes to their start.
quint32 movedPoint(const std::vector<StructEdit> &edits, int sheet, bool rows, quint32 v)
{
    using K = StructEdit::Kind;
    for (const StructEdit &e : edits) {
        if (e.sheet != sheet) continue;
        if (rows && e.kind == K::InsertRows) {
            if (v >= e.first) v += e.count;
        } else if (rows && e.kind == K::DeleteRows) {
            if (v >= e.first + e.count) v -= e.count;
            else if (v > e.first) v = e.first;
        } else if (!rows && e.kind == K::InsertColumns) {
            if (v >= e.first) v += e.count;
        } else if (!rows && e.kind == K::DeleteColumns) {
            if (v >= e.first + e.count) v -= e.count;
            else if (v > e.first) v = e.first;
        }
    }
    return v;
}

} // namespace

DrawingAnchor movedAnchor(const DrawingAnchor &a, int sheet, const std::vector<StructEdit> &edits)
{
    DrawingAnchor out = a;
    if (a.kind == AnchorKind::Absolute || a.editAs == AnchorKind::Absolute) return out;
    out.from.col = movedPoint(edits, sheet, false, a.from.col);
    out.from.row = movedPoint(edits, sheet, true, a.from.row);
    if (a.kind != AnchorKind::TwoCell) return out;
    if (a.editAs == AnchorKind::OneCell) {
        // (both ends move as far as the first)
        const qint64 dCol = qint64(out.from.col) - a.from.col, dRow = qint64(out.from.row) - a.from.row;
        out.to.col = quint32(std::max<qint64>(0, qint64(a.to.col) + dCol));
        out.to.row = quint32(std::max<qint64>(0, qint64(a.to.row) + dRow));
        return out;
    }
    out.to.col = movedPoint(edits, sheet, false, a.to.col);
    out.to.row = movedPoint(edits, sheet, true, a.to.row);
    return out;
}

bool anchorSurvives(const DrawingAnchor &a, int sheet, const std::vector<StructEdit> &edits)
{
    using K = StructEdit::Kind;
    if (a.kind == AnchorKind::Absolute || a.editAs == AnchorKind::Absolute) return true;
    quint32 row = a.from.row, col = a.from.col;
    for (const StructEdit &e : edits) {
        if (e.sheet != sheet) continue;
        if (e.kind == K::DeleteRows && row >= e.first && row < e.first + e.count) return false;
        if (e.kind == K::DeleteColumns && col >= e.first && col < e.first + e.count) return false;
        const std::vector<StructEdit> one = {e};
        row = movedPoint(one, sheet, true, row);
        col = movedPoint(one, sheet, false, col);
    }
    return true;
}

QString drawingPartOf(const Package &package, const QString &sheetPart)
{
    if (sheetPart.isEmpty()) return QString();
    for (const Relationship &r : package.relationshipsOfType(sheetPart, QStringLiteral("/drawing")))
        if (!r.external && package.hasPart(r.target)) return r.target;
    return QString();
}

std::shared_ptr<QDomDocument> parseDrawing(const QByteArray &xml, QString *error)
{
    auto doc = std::make_shared<QDomDocument>();
    QString why;
    if (!parseXml(xml, doc.get(), &why)) {
        if (error) *error = why.isEmpty() ? QStringLiteral("the drawing is not XML") : why;
        return nullptr;
    }
    return doc;
}

bool readDrawing(const Package &package, const QString &drawingPart, SheetDrawing *out, QString *error, const std::shared_ptr<QDomDocument> &parsed)
{
    *out = SheetDrawing();
    out->part = drawingPart;
    QString why;
    std::shared_ptr<QDomDocument> doc = parsed ? parsed : parseDrawing(package.part(drawingPart), &why);
    if (!doc) {
        if (error) *error = why.isEmpty() ? QStringLiteral("the drawing is not XML") : why;
        out->problem = why;
        return false;
    }
    const QDomElement root = doc->documentElement();
    if (root.localName() != QLatin1String("wsDr")) {
        if (error) *error = QStringLiteral("the part is not a drawing");
        out->problem = QStringLiteral("not a drawing");
        return false;
    }
    out->document = doc;
    int ordinal = -1;
    for (QDomElement top = root.firstChildElement(); !top.isNull(); top = top.nextSiblingElement()) {
        ++ordinal;
        // an anchor, or one wrapped for programs that read an older form (the newer form is read when its object is, else the older one)
        QVector<QDomElement> anchors;
        if (isAnchor(top.localName())) {
            anchors << top;
        } else if (top.localName() == QLatin1String("AlternateContent")) {
            for (const char *branch : {"Choice", "Fallback"}) {
                for (QDomElement a = childByLocal(top, branch).firstChildElement(); !a.isNull(); a = a.nextSiblingElement())
                    if (isAnchor(a.localName())) anchors << a;
                if (!anchors.isEmpty()) {
                    // (the first branch that holds an object that is read)
                    DrawingObject probe;
                    bool any = false;
                    for (const QDomElement &a : std::as_const(anchors))
                        // (a text box that holds math the program can't lay out is not the object to show: the picture beside it, that stands for it, is)
                        if (readObject(a, package, drawingPart, &probe) && !(probe.kind == ObjectKind::Shape && holdsMath(probe.element) && shapeMath(probe.element).isEmpty())) any = true;
                    if (any) break;
                    anchors.clear();
                }
            }
        }
        for (const QDomElement &a : std::as_const(anchors)) {
            ++out->anchorsInPart;
            if (out->objects.size() >= kMaxDrawingAnchors) {
                out->truncated = true;
                continue;
            }
            DrawingObject o;
            if (!readObject(a, package, drawingPart, &o)) {
                ++out->skipped;
                continue;
            }
            o.index = int(out->objects.size());
            o.ordinal = ordinal;
            o.anchor = readAnchor(a);
            out->objects.push_back(o);
        }
    }
    out->rootChildren = ordinal + 1;
    return true;
}

QVector<SheetEquation> readEquations(const Package &package, const QString &drawingPart, int *objects)
{
    QVector<SheetEquation> out;
    SheetDrawing drawing;
    if (!readDrawing(package, drawingPart, &drawing)) return out;
    if (objects) *objects = int(drawing.objects.size());
    for (const DrawingObject &o : std::as_const(drawing.objects)) {
        if (o.kind != ObjectKind::Shape || o.element.isNull() || out.size() >= kMaxDrawingEquations) continue;
        const QByteArray markup = shapeMath(o.element);
        if (markup.isEmpty()) continue;
        out.push_back(SheetEquation{o.anchor, o.name, o.description, markup});
    }
    return out;
}

// ---- embedded objects

namespace {

QString relationshipIdOf(QByteArrayView tag)
{
    static const QRegularExpression id(QStringLiteral("\\s[A-Za-z_][\\w.-]*:id\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)')"));
    const QRegularExpressionMatch m = id.match(QString::fromUtf8(tag));
    if (!m.hasMatch()) return QString();
    return xmlscan::unescape(m.captured(m.capturedStart(1) >= 0 ? 1 : 2).toUtf8());
}

qint64 numberAfter(const QByteArray &xml, qsizetype from, qint64 limit)
{
    const qsizetype end = xml.indexOf('<', from);
    bool ok = false;
    const qint64 v = xml.mid(from, end < 0 ? -1 : end - from).trimmed().toLongLong(&ok);
    return !ok || v < 0 ? 0 : std::min(v, limit);
}

} // namespace

std::vector<OleChild> oleChildrenOf(const QByteArray &after, bool *truncated)
{
    using xmlscan::Tag;
    // (the elements of the worksheet are at depth one under a root made up for them)
    static const QByteArray root("<worksheet>");
    const QByteArray xml = root + after;
    std::vector<OleChild> out;
    if (truncated) *truncated = false;
    Tag t;
    qsizetype pos = 0;
    int depth = 0;
    bool inList = false;
    OleChild child;
    bool inChild = false, haveObject = false, inFrom = false, inTo = false;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing) {
            --depth;
            if (inList && inChild && depth == 2) {
                child.end = t.end;
                out.push_back(child);
                out.back().begin -= root.size();
                out.back().end -= root.size();
                inChild = false;
            } else if (inList && !inChild && depth == 1) {
                return out;   // (the list is closed)
            }
            if (depth < 0) break;
            continue;
        }
        if (!inList) {
            if (depth == 1 && t.local == "oleObjects") {
                if (t.selfClosing) return out;
                inList = true;
            }
            if (!t.selfClosing) ++depth;
            continue;
        }
        if (!inChild && depth == 2) {
            if (int(out.size()) >= kMaxOleChildren) {
                if (truncated) *truncated = true;
                return out;
            }
            child = OleChild();
            child.begin = t.start;
            inChild = true;
            haveObject = false;
            inFrom = inTo = false;
            if (t.selfClosing) {
                // (an oleObject that closes itself: it names an object and no picture or place)
                child.end = t.end;
                if (t.local == "oleObject") {
                    const QByteArrayView tag = QByteArrayView(xml).mid(t.start, t.end - t.start);
                    child.progId = xmlscan::unescape(xmlscan::attribute(tag, "progId")).left(256);
                    child.shapeId = xmlscan::unescape(xmlscan::attribute(tag, "shapeId")).toLongLong();
                    child.objectRel = relationshipIdOf(tag);
                }
                out.push_back(child);
                out.back().begin -= root.size();
                out.back().end -= root.size();
                inChild = false;
                continue;
            }
        }
        if (inChild) {
            const QByteArrayView tag = QByteArrayView(xml).mid(t.start, t.end - t.start);
            if (t.local == "oleObject" && !haveObject) {
                haveObject = true;
                child.progId = xmlscan::unescape(xmlscan::attribute(tag, "progId")).left(256);
                child.shapeId = xmlscan::unescape(xmlscan::attribute(tag, "shapeId")).toLongLong();
                child.objectRel = relationshipIdOf(tag);
            } else if (t.local == "objectPr" && child.pictureRel.isEmpty()) {
                child.pictureRel = relationshipIdOf(tag);
            } else if (t.local == "from" || t.local == "to") {
                inFrom = t.local == "from";
                inTo = !inFrom;
                child.hasAnchor = true;
            } else if (inFrom || inTo) {
                AnchorPoint &point = inFrom ? child.anchor.from : child.anchor.to;
                if (t.local == "col") point.col = quint32(numberAfter(xml, t.end, kMaxColumns - 1));
                else if (t.local == "row") point.row = quint32(numberAfter(xml, t.end, kMaxRows - 1));
                else if (t.local == "colOff") point.colOff = numberAfter(xml, t.end, kMaxDrawingEmu);
                else if (t.local == "rowOff") point.rowOff = numberAfter(xml, t.end, kMaxDrawingEmu);
            }
        }
        if (!t.selfClosing) ++depth;
    }
    return out;
}

namespace {

// What follows a sheet's cells, read as a stream: what comes after the first end of `sheetData` that is found (the whole part if there is none to find).
bool tailOfSheet(const Package &package, const QString &part, QByteArray *tail)
{
    const std::unique_ptr<QIODevice> in = package.openPart(part);
    if (!in || !in->open(QIODevice::ReadOnly)) return false;
    static const QByteArray close("</sheetData>"), empty("<sheetData/>");
    QByteArray buf;
    bool found = false;
    constexpr qint64 kTailCap = qint64(64) << 20;
    for (;;) {
        const QByteArray chunk = in->read(1 << 20);
        if (chunk.isEmpty()) break;
        buf += chunk;
        if (!found) {
            qsizetype at = buf.lastIndexOf(close);
            qsizetype length = close.size();
            if (at < 0) {
                at = buf.lastIndexOf(empty);
                length = empty.size();
            }
            if (at >= 0) {
                buf.remove(0, at + length);   // (what is left follows the cells; a later end of sheetData would be a damaged part's)
                found = true;
            } else if (buf.size() > (1 << 21)) {
                buf.remove(0, buf.size() - 16);   // (cells go by; the end of sheetData may be cut between two chunks)
            }
        }
        if (buf.size() > kTailCap) return false;
    }
    *tail = buf;
    return true;
}

} // namespace

bool readSheetOleChildren(const Package &package, const QString &sheetPart, std::vector<OleChild> *out)
{
    out->clear();
    if (!package.hasPart(sheetPart)) return false;
    QByteArray tail;
    if (!tailOfSheet(package, sheetPart, &tail)) return false;
    *out = oleChildrenOf(tail);
    return true;
}

namespace {

void measureTwoCell(const DrawingAnchor &a, qint64 *w, qint64 *h)
{
    *w = (qint64(a.to.col) - qint64(a.from.col)) * 609600 + a.to.colOff - a.from.colOff;
    *h = (qint64(a.to.row) - qint64(a.from.row)) * 190500 + a.to.rowOff - a.from.rowOff;
}

bool isObjectRelationship(const Relationship &r)
{
    return !r.id.isEmpty() && !r.external && (r.type.endsWith(QLatin1String("/oleObject")) || r.type.endsWith(QLatin1String("/package")))
           && r.target.startsWith(QLatin1String("/xl/embeddings/"), Qt::CaseInsensitive);
}

} // namespace

bool readSheetOleObjects(const Package &package, const QString &sheetPart, OleReadState *state, std::vector<SheetOleObject> *out)
{
    out->clear();
    if (!package.hasPart(sheetPart)) return false;
    QByteArray tail;
    if (!tailOfSheet(package, sheetPart, &tail)) return false;
    const std::vector<OleChild> children = oleChildrenOf(tail);
    for (size_t i = 0; i < children.size(); ++i) {
        const OleChild &c = children[i];
        const Relationship rel = package.relationship(sheetPart, c.objectRel);
        if (!isObjectRelationship(rel) || !package.hasPart(rel.target) || !c.hasAnchor || c.progId.isEmpty()) continue;
        // (what a part comes to is asked of the package before it is inflated, and a part that many objects name is inflated once)
        const Relationship image = c.pictureRel.isEmpty() ? Relationship() : package.relationship(sheetPart, c.pictureRel);
        const bool pictureUsable = !image.id.isEmpty() && !image.external && image.type.endsWith(QLatin1String("/image")) && package.hasPart(image.target);
        const qint64 size = package.partSize(rel.target), pictureSize = pictureUsable ? package.partSize(image.target) : 0;
        const bool pictureFits = pictureUsable && pictureSize >= 0 && pictureSize <= ole::limits().pictureBytes;
        if (state->refused.count(rel.target.toLower()) || size < 0 || size > ole::limits().objectBytes || !state->budget.fits(size, pictureFits ? pictureSize : 0)) {
            ++state->pastLimit;
            continue;
        }
        qint64 w = 0, h = 0;
        if (state->measure) state->measure(c.anchor, &w, &h);
        else measureTwoCell(c.anchor, &w, &h);
        w = std::clamp<qint64>(w, 0, ole::kMaxEmu);
        h = std::clamp<qint64>(h, 0, ole::kMaxEmu);
        auto partOf = [&](const QString &name) {
            const QString key = name.toLower();
            auto it = state->parts.find(key);
            if (it == state->parts.end()) it = state->parts.emplace(key, package.part(name)).first;
            return it->second;
        };
        const QByteArray bytes = partOf(rel.target);
        const QByteArray preview = pictureFits ? partOf(image.target) : QByteArray();
        std::optional<ole::EmbeddedObject> kept = ole::keepObject(c.progId, bytes, preview, w, h, state->budget, rel.target);
        if (!kept) {
            state->refused.insert(rel.target.toLower());
            ++state->pastLimit;
            continue;
        }
        SheetOleObject o;
        o.object = *kept;
        o.anchor = c.anchor;
        o.anchor.kind = AnchorKind::TwoCell;
        o.fileIndex = int(i);
        o.shapeId = c.shapeId;
        out->push_back(std::move(o));
    }
    return true;
}

std::vector<SheetOleObject> oleObjectsThatFit(const std::vector<SheetOleObject> &objects, ole::ObjectBudget *budget)
{
    std::vector<SheetOleObject> out;
    for (const SheetOleObject &o : objects) {
        if (!budget->admit(o.object.partBytes.size(), o.object.previewPng.size())) continue;
        SheetOleObject copy = o;
        copy.fileIndex = -1;
        copy.object.origin.clear();
        out.push_back(std::move(copy));
    }
    return out;
}

} // namespace jp::office::sheet
