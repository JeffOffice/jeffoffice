#include "sheet/drawing.h"

#include "office/chart.h"
#include "office/xml.h"
#include "sheet/cellref.h"

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
            if (id.isEmpty()) continue;   // (a table, a slicer: not read)
            const Relationship rel = package.relationship(part, id);
            if (rel.id.isEmpty() || rel.external || !package.hasPart(rel.target)) continue;
            o->kind = ObjectKind::Chart;
            o->chartPart = rel.target;
        } else {
            continue;
        }
        const QDomElement cNvPr = childByLocal(nv, "cNvPr");
        o->element = c;
        o->name = attr(cNvPr, QStringLiteral("name")).left(256);
        o->description = attr(cNvPr, QStringLiteral("descr")).left(1024);
        o->id = parseInt(attr(cNvPr, QStringLiteral("id")), 0);
        o->hidden = parseBool(attr(cNvPr, QStringLiteral("hidden")));
        return true;
    }
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
                        if (readObject(a, package, drawingPart, &probe)) any = true;
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
    return true;
}

} // namespace jp::office::sheet
