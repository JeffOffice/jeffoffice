#include "sheet/drawingadd.h"

#include "sheet/xmlscan.h"

namespace jp::office::sheet {

using xmlscan::Tag;

namespace {

const char kXdr[] = "http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing";
const char kA[] = "http://schemas.openxmlformats.org/drawingml/2006/main";
const char kC[] = "http://schemas.openxmlformats.org/drawingml/2006/chart";
const char kR[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
const char kRelType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/";

QByteArray number(qint64 v) { return QByteArray::number(qlonglong(v)); }

QByteArray point(const char *tag, const AnchorPoint &p)
{
    return QByteArray("<xdr:") + tag + "><xdr:col>" + number(p.col) + "</xdr:col><xdr:colOff>" + number(p.colOff) + "</xdr:colOff><xdr:row>" + number(p.row) + "</xdr:row><xdr:rowOff>" +
           number(p.rowOff) + "</xdr:rowOff></xdr:" + tag + ">";
}

// The root element's open tag and its close tag.
bool rootTags(const QByteArray &xml, Tag *open, Tag *close)
{
    Tag t;
    qsizetype pos = 0;
    bool haveOpen = false;
    int depth = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (!haveOpen) {
            if (t.closing) return false;
            *open = t;
            haveOpen = true;
            if (t.selfClosing) {
                *close = t;
                return true;
            }
            depth = 1;
            continue;
        }
        if (t.closing) {
            if (--depth == 0) {
                *close = t;
                return true;
            }
        } else if (!t.selfClosing) {
            ++depth;
        }
    }
    return false;
}

QByteArray escapeBytes(const QString &text) { return xmlscan::escapeAttribute(text); }

} // namespace

QByteArray chartAnchorXml(const DrawingAnchor &a, int shapeId, const QString &name, const QString &token)
{
    QByteArray edit;
    if (a.editAs == AnchorKind::OneCell) edit = " editAs=\"oneCell\"";
    else if (a.editAs == AnchorKind::Absolute) edit = " editAs=\"absolute\"";
    return QByteArray("<xdr:twoCellAnchor") + edit + " xmlns:xdr=\"" + kXdr + "\" xmlns:a=\"" + kA + "\" xmlns:c=\"" + kC + "\" xmlns:r=\"" + kR + "\">" + point("from", a.from) + point("to", a.to) +
           "<xdr:graphicFrame macro=\"\"><xdr:nvGraphicFramePr><xdr:cNvPr id=\"" + number(shapeId) + "\" name=\"" + escapeBytes(name) +
           "\"/><xdr:cNvGraphicFramePr/></xdr:nvGraphicFramePr><xdr:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/></xdr:xfrm><a:graphic><a:graphicData uri=\"" + kC +
           "\"><c:chart r:id=\"" + escapeBytes(token) + "\"/></a:graphicData></a:graphic></xdr:graphicFrame><xdr:clientData/></xdr:twoCellAnchor>";
}

QByteArray drawingWithAnchors(const QByteArray &xml, const QByteArray &anchors)
{
    Tag open, close;
    if (!rootTags(xml, &open, &close)) return QByteArray();
    if (open.selfClosing) {
        QByteArray head = xml.left(open.end);
        head.chop(2);   // "/>"
        while (!head.isEmpty() && (head.back() == ' ' || head.back() == '\n' || head.back() == '\t' || head.back() == '\r')) head.chop(1);
        return head + ">" + anchors + "</" + QByteArray(open.name) + ">" + xml.mid(open.end);
    }
    return xml.left(close.start) + anchors + xml.mid(close.start);
}

QByteArray newDrawingXml(const QByteArray &anchors)
{
    return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<xdr:wsDr xmlns:xdr=\"") + kXdr + "\" xmlns:a=\"" + kA + "\">" + anchors + "</xdr:wsDr>";
}

QStringList relationshipIds(const QByteArray &xml)
{
    QStringList out;
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "Relationship") continue;
        bool found = false;
        const QByteArrayView v = xmlscan::attribute(QByteArrayView(xml).sliced(t.start, t.end - t.start), "Id", &found);
        if (found) out << xmlscan::unescape(v);
    }
    return out;
}

QString freeRelationshipId(const QStringList &used)
{
    for (int n = 1;; ++n) {
        const QString id = QStringLiteral("rId%1").arg(n);
        if (!used.contains(id)) return id;
    }
}

QByteArray relationshipXml(const QString &id, const QString &type, const QString &target)
{
    return "<Relationship Id=\"" + escapeBytes(id) + "\" Type=\"" + (type.contains(QLatin1Char(':')) ? escapeBytes(type) : QByteArray(kRelType) + escapeBytes(type)) + "\" Target=\"" + escapeBytes(target) + "\"/>";
}

bool addRelationship(const QByteArray &relsXml, const QString &id, const QString &type, const QString &target, QByteArray *out)
{
    const QByteArray element = relationshipXml(id, type, target);
    if (relsXml.isEmpty()) {
        *out = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">" + element + "</Relationships>";
        return true;
    }
    Tag open, close;
    if (!rootTags(relsXml, &open, &close) || open.local != "Relationships") return false;
    const QByteArray prefix(xmlscan::prefixOf(open.name));
    QByteArray e = element;
    e.replace("<Relationship ", "<" + prefix + "Relationship ");
    if (open.selfClosing) {
        QByteArray head = relsXml.left(open.end);
        head.chop(2);
        *out = head + ">" + e + "</" + QByteArray(open.name) + ">" + relsXml.mid(open.end);
        return true;
    }
    *out = relsXml.left(close.start) + e + relsXml.mid(close.start);
    return true;
}

static bool insertInTypes(const QByteArray &typesXml, const QByteArray &element, QByteArray *out)
{
    Tag open, close;
    if (!rootTags(typesXml, &open, &close) || open.local != "Types") return false;
    const QByteArray prefix(xmlscan::prefixOf(open.name));
    QByteArray e = element;
    e.replace("<Override ", "<" + prefix + "Override ").replace("<Default ", "<" + prefix + "Default ");
    if (open.selfClosing) {
        QByteArray head = typesXml.left(open.end);
        head.chop(2);
        *out = head + ">" + e + "</" + QByteArray(open.name) + ">" + typesXml.mid(open.end);
        return true;
    }
    *out = typesXml.left(close.start) + e + typesXml.mid(close.start);
    return true;
}

bool addContentOverride(const QByteArray &typesXml, const QString &part, const QString &contentType, QByteArray *out)
{
    return insertInTypes(typesXml, "<Override PartName=\"" + escapeBytes(part) + "\" ContentType=\"" + escapeBytes(contentType) + "\"/>", out);
}

bool hasContentDefault(const QByteArray &typesXml, const QString &extension)
{
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(typesXml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "Default") continue;
        bool found = false;
        const QByteArrayView v = xmlscan::attribute(QByteArrayView(typesXml).sliced(t.start, t.end - t.start), "Extension", &found);
        if (found && xmlscan::unescape(v).compare(extension, Qt::CaseInsensitive) == 0) return true;
    }
    return false;
}

bool addContentDefault(const QByteArray &typesXml, const QString &extension, const QString &contentType, QByteArray *out)
{
    if (hasContentDefault(typesXml, extension)) {
        *out = typesXml;
        return true;
    }
    return insertInTypes(typesXml, "<Default Extension=\"" + escapeBytes(extension) + "\" ContentType=\"" + escapeBytes(contentType) + "\"/>", out);
}

QString relativeTarget(const QString &from, const QString &to)
{
    const QStringList f = from.split(QLatin1Char('/'), Qt::SkipEmptyParts), t = to.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    int common = 0;
    while (common + 1 < int(f.size()) && common + 1 < int(t.size()) && f[common].compare(t[common], Qt::CaseInsensitive) == 0) ++common;
    QStringList parts;
    for (int i = common; i + 1 < int(f.size()); ++i) parts << QStringLiteral("..");
    for (int i = common; i < int(t.size()); ++i) parts << t[i];
    return parts.join(QLatin1Char('/'));
}

QString freePartName(const QString &prefix, const QString &suffix, const QSet<QString> &taken, int first)
{
    for (int n = first;; ++n) {
        const QString name = prefix + QString::number(n) + suffix;
        if (!taken.contains(name.toLower())) return name;
    }
}

} // namespace jp::office::sheet
