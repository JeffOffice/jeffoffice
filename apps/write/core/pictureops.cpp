#include "core/pictureops.h"

#include "core/editops.h"
#include "core/writeprops.h"
#include "text/textprops.h"
#include "wml/wml.h"
#include "wml/xmltree.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QImageReader>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp::write {

using namespace office::wml;

namespace {

constexpr double kEmuPerPoint = 12700.0;

qint64 emuOf(double points) { return qint64(std::llround(std::clamp(points, 0.0, 100000.0) * kEmuPerPoint)); }

// The prefixes a drawing kept as written may use.
const QByteArray &declarations()
{
    static const QByteArray d =
        "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
        "xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\" "
        "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
        "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" "
        "xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" xmlns:wpg=\"http://schemas.microsoft.com/office/word/2010/wordprocessingGroup\" "
        "xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\"";
    return d;
}

// Text for an attribute: no character XML refuses, escaped, and at most `limit` characters.
QByteArray attribute(const QString &text, int limit)
{
    QString clean;
    clean.reserve(std::min<qsizetype>(text.size(), limit));
    for (const QChar ch : text) {
        if (clean.size() >= limit) break;
        const ushort u = ch.unicode();
        if (u < 0x20 && u != 0x09) clean += QLatin1Char(' ');
        else if (u == 0xFFFE || u == 0xFFFF || (u >= 0xD800 && u <= 0xDFFF)) continue;
        else clean += ch;
    }
    return clean.toHtmlEscaped().toUtf8();
}

// A drawing of one picture, as its parts: what the program writes again when the picture changes, and the graphic (the picture itself), which it keeps.
struct Model {
    bool floating = false;
    qint64 cx = 0, cy = 0;                    // EMU
    int id = 1;
    QByteArray name = "Picture", descr, title;
    PictureWrap wrap = PictureWrap::Inline;
    QByteArray wrapRaw;                       // the wrap element as the file had it (kept while the kind of wrap is)
    PictureWrap wrapRawOf = PictureWrap::Inline;
    qint64 distT = 0, distB = 0, distL = 0, distR = 0;
    QByteArray posH, posV;                    // positionH and positionV as written
    QByteArray effectExtent, frameProps, graphic;
    qint64 relativeHeight = 251658240;
};

QByteArray defaultPosH() { return "<wp:positionH relativeFrom=\"column\"><wp:posOffset>0</wp:posOffset></wp:positionH>"; }
QByteArray defaultPosV() { return "<wp:positionV relativeFrom=\"paragraph\"><wp:posOffset>0</wp:posOffset></wp:positionV>"; }

QByteArray wrapElement(PictureWrap wrap)
{
    switch (wrap) {
    case PictureWrap::Square: return "<wp:wrapSquare wrapText=\"bothSides\"/>";
    case PictureWrap::Tight:
        return "<wp:wrapTight wrapText=\"bothSides\"><wp:wrapPolygon edited=\"0\"><wp:start x=\"0\" y=\"0\"/><wp:lineTo x=\"0\" y=\"21600\"/><wp:lineTo x=\"21600\" y=\"21600\"/>"
               "<wp:lineTo x=\"21600\" y=\"0\"/><wp:lineTo x=\"0\" y=\"0\"/></wp:wrapPolygon></wp:wrapTight>";
    case PictureWrap::TopAndBottom: return "<wp:wrapTopAndBottom/>";
    default: return "<wp:wrapNone/>";
    }
}

QByteArray drawingXml(const Model &m)
{
    auto n = [](qint64 v) { return QByteArray::number(v); };
    QByteArray x = "<w:drawing xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                   "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";
    const QByteArray extent = "<wp:extent cx=\"" + n(m.cx) + "\" cy=\"" + n(m.cy) + "\"/>";
    QByteArray docPr = "<wp:docPr id=\"" + n(m.id) + "\" name=\"" + m.name + "\"";
    if (!m.descr.isEmpty()) docPr += " descr=\"" + m.descr + "\"";
    if (!m.title.isEmpty()) docPr += " title=\"" + m.title + "\"";
    docPr += "/>";
    const QByteArray frame = m.frameProps.isEmpty() ? QByteArray("<wp:cNvGraphicFramePr><a:graphicFrameLocks noChangeAspect=\"1\"/></wp:cNvGraphicFramePr>") : m.frameProps;
    if (!m.floating) {
        x += "<wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\">" + extent + m.effectExtent + docPr + frame + m.graphic + "</wp:inline>";
    } else {
        const bool behind = m.wrap == PictureWrap::BehindText;
        x += "<wp:anchor distT=\"" + n(m.distT) + "\" distB=\"" + n(m.distB) + "\" distL=\"" + n(m.distL) + "\" distR=\"" + n(m.distR) + "\" simplePos=\"0\" relativeHeight=\"" +
             n(m.relativeHeight) + "\" behindDoc=\"" + (behind ? "1" : "0") + "\" locked=\"0\" layoutInCell=\"1\" allowOverlap=\"1\">";
        x += "<wp:simplePos x=\"0\" y=\"0\"/>" + (m.posH.isEmpty() ? defaultPosH() : m.posH) + (m.posV.isEmpty() ? defaultPosV() : m.posV) + extent + m.effectExtent;
        x += !m.wrapRaw.isEmpty() && m.wrapRawOf == m.wrap ? m.wrapRaw : wrapElement(m.wrap);
        x += docPr + frame + m.graphic + "</wp:anchor>";
    }
    x += "</w:drawing>";
    return x;
}

// The graphic of a new picture: the picture `rid` names, stretched over a frame of this size.
QByteArray pictureGraphic(const QString &rid, qint64 cx, qint64 cy, const QByteArray &name, int id)
{
    auto n = [](qint64 v) { return QByteArray::number(v); };
    return "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic><pic:nvPicPr><pic:cNvPr id=\"" + n(id) + "\" name=\"" + name +
           "\"/><pic:cNvPicPr/></pic:nvPicPr><pic:blipFill><a:blip r:embed=\"" + rid.toUtf8() + "\"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill><pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/>"
           "<a:ext cx=\"" + n(cx) + "\" cy=\"" + n(cy) + "\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData></a:graphic>";
}

qint64 emuAttr(const XmlTree &t, int n, const char *name)
{
    return std::clamp<qint64>(qint64(t.attr(n, name).toDouble()), 0, qint64(100000) * 12700);
}

// A drawing that is one plain picture, read as the parts above; false for anything else.
bool parseDrawing(const QByteArray &raw, Model *m)
{
    if (!raw.startsWith("<w:drawing")) return false;
    XmlTree t;
    if (!t.parse("<x " + declarations() + ">" + raw + "</x>", nullptr)) return false;
    const int drawing = t.firstChild(t.root());
    if (drawing < 0 || t.localName(drawing) != "drawing") return false;
    int item = -1;
    for (int c = t.firstChild(drawing); c >= 0; c = t.next(c)) {
        const QByteArray name = t.localName(c);
        if ((name == "inline" || name == "anchor") && item < 0) item = c;
        else return false;
    }
    if (item < 0) return false;
    Model out;
    out.floating = t.localName(item) == "anchor";
    out.distT = emuAttr(t, item, "distT");
    out.distB = emuAttr(t, item, "distB");
    out.distL = emuAttr(t, item, "distL");
    out.distR = emuAttr(t, item, "distR");
    if (t.has(item, "relativeHeight")) out.relativeHeight = std::clamp<qint64>(t.attr(item, "relativeHeight").toLongLong(), 0, 4294967295LL);
    const bool behind = t.attr(item, "behindDoc") == QLatin1String("1") || t.attr(item, "behindDoc") == QLatin1String("true");
    bool wrapNone = false;
    for (int k = t.firstChild(item); k >= 0; k = t.next(k)) {
        const QByteArray tag = t.localName(k);
        if (tag == "extent") {
            out.cx = emuAttr(t, k, "cx");
            out.cy = emuAttr(t, k, "cy");
        } else if (tag == "effectExtent") {
            out.effectExtent = t.raw(k);
        } else if (tag == "docPr") {
            out.id = int(std::clamp<qint64>(t.attr(k, "id").toLongLong(), 0, 2000000000));
            out.name = attribute(t.attr(k, "name"), 255);
            out.descr = attribute(t.attr(k, "descr"), kMostAltTextLength);
            out.title = attribute(t.attr(k, "title"), kMostAltTextLength);
        } else if (tag == "cNvGraphicFramePr") {
            out.frameProps = t.raw(k);
        } else if (tag == "graphic") {
            out.graphic = t.raw(k);
        } else if (tag == "positionH") {
            out.posH = t.raw(k);
        } else if (tag == "positionV") {
            out.posV = t.raw(k);
        } else if (tag == "wrapSquare" || tag == "wrapThrough" || tag == "wrapTight" || tag == "wrapTopAndBottom" || tag == "wrapNone") {
            out.wrapRaw = t.raw(k);
            out.wrapRawOf = tag == "wrapSquare" ? PictureWrap::Square : tag == "wrapTopAndBottom" ? PictureWrap::TopAndBottom : tag == "wrapNone" ? PictureWrap::InFrontOfText : PictureWrap::Tight;
            wrapNone = tag == "wrapNone";
        }
    }
    if (out.graphic.isEmpty() || !out.graphic.contains("blip") || out.cx <= 0 || out.cy <= 0) return false;
    if (!out.floating) {
        out.wrap = PictureWrap::Inline;
    } else if (wrapNone || out.wrapRaw.isEmpty()) {
        out.wrap = behind ? PictureWrap::BehindText : PictureWrap::InFrontOfText;
        out.wrapRawOf = out.wrap;
    } else {
        out.wrap = out.wrapRawOf;
    }
    *m = out;
    return true;
}

// The highest id the text's drawings have.
int highestDrawingId(const QTextDocument *story)
{
    int highest = 0;
    static const QRegularExpression idPattern(QStringLiteral("<wp:docPr[^>]*?\\sid=\"([0-9]{1,9})\""));
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (!bytes.contains("docPr")) continue;
            const QString text = QString::fromLatin1(bytes);
            auto found = idPattern.globalMatch(text);
            while (found.hasNext()) highest = std::max(highest, found.next().captured(1).toInt());
        }
    return std::min(highest, 2000000000 - 2);
}

// The character format at the place after `position`.
QTextCharFormat formatAfter(const QTextDocument *story, int position)
{
    QTextCursor c(const_cast<QTextDocument *>(story));
    c.setPosition(position + 1);
    return c.charFormat();
}

bool isPictureShape(const QVector<Shape> &shapes) { return !shapes.isEmpty() && shapes.first().kind == Shape::Picture; }

PictureWrap wrapOfShape(const Shape &s)
{
    if (!s.floating) return PictureWrap::Inline;
    switch (s.wrap) {
    case Shape::Square: return PictureWrap::Square;
    case Shape::Tight: return PictureWrap::Tight;
    case Shape::TopBottom: return PictureWrap::TopAndBottom;
    default: return s.behind ? PictureWrap::BehindText : PictureWrap::InFrontOfText;
    }
}

// The atom a character holds, if it is a drawing; empty otherwise.
bool drawingAtomOf(const QTextCharFormat &cf, Inline *atom)
{
    if (!cf.hasProperty(prop::Atom) || cf.hasProperty(prop::TrackedDeleted)) return false;
    const QByteArray bytes = storedBytes(cf, prop::Atom);
    if (!bytes.contains("drawing")) return false;
    const Inline a = unpackAtom(bytes);
    if (a.kind != Inline::RunAtom || a.tag != QLatin1String("drawing")) return false;
    *atom = a;
    return true;
}

// A picture file as it is kept: its bytes, the extension and content type of the part, and its size in points.
struct Prepared {
    QByteArray bytes;
    QString extension, contentType;
    QSizeF size;
};

PictureResult encodeImage(const QImage &image, Prepared *out)
{
    if (image.isNull()) return PictureResult::Unreadable;
    if (qint64(image.width()) * image.height() > kMostPicturePixels || image.width() > kMostPictureSide || image.height() > kMostPictureSide) return PictureResult::TooLarge;
    QBuffer buffer(&out->bytes);
    buffer.open(QIODevice::WriteOnly);
    if (!image.save(&buffer, "PNG") || out->bytes.isEmpty()) return PictureResult::Unreadable;
    if (qint64(out->bytes.size()) > kMostPictureBytes) return PictureResult::TooLarge;
    out->extension = QStringLiteral("png");
    out->contentType = QStringLiteral("image/png");
    out->size = QSizeF(image.width() * 0.75, image.height() * 0.75);
    return PictureResult::Ok;
}

PictureResult prepare(const QByteArray &input, Prepared *out)
{
    if (input.isEmpty()) return PictureResult::Unreadable;
    if (qint64(input.size()) > kMostPictureBytes) return PictureResult::TooLarge;
    QByteArray copy = input;
    QBuffer buffer(&copy);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer);
    const QByteArray format = reader.format().toLower();
    const QSize size = reader.size();
    if (format.isEmpty() || !size.isValid() || size.isEmpty()) return PictureResult::Unreadable;
    if (qint64(size.width()) * size.height() > kMostPicturePixels || size.width() > kMostPictureSide || size.height() > kMostPictureSide) return PictureResult::TooLarge;
    struct Kept { const char *format, *extension, *type; };
    static const Kept kept[] = {{"png", "png", "image/png"}, {"jpeg", "jpeg", "image/jpeg"}, {"jpg", "jpeg", "image/jpeg"}, {"gif", "gif", "image/gif"}, {"bmp", "bmp", "image/bmp"},
                                {"tiff", "tiff", "image/tiff"}, {"tif", "tiff", "image/tiff"}};
    for (const Kept &k : kept) {
        if (format != k.format) continue;
        // (The file is read once, so that bytes that hold a picture's header and no picture are not taken.)
        const QImage whole = reader.read();
        if (whole.isNull()) return PictureResult::Unreadable;
        out->bytes = input;
        out->extension = QString::fromLatin1(k.extension);
        out->contentType = QString::fromLatin1(k.type);
        out->size = QSizeF(size.width() * 0.75, size.height() * 0.75);
        return PictureResult::Ok;
    }
    return encodeImage(reader.read(), out);
}

// The size a new picture is made at: the one it asks for, no wider than the column or taller than the page's text.
QSizeF fitted(WriteDoc &doc, int position, QSizeF size)
{
    const flow::Section &sec = doc.flow().section(doc.sectionAt(position));
    const double maxW = std::max(kLeastPictureSize, std::min(sec.columnWidth(), kMostPictureSize));
    const double maxH = std::max(kLeastPictureSize, std::min(sec.pageSize().height() - sec.margins.top() - sec.margins.bottom(), kMostPictureSize));
    double scale = 1;
    if (size.width() > maxW) scale = std::min(scale, maxW / size.width());
    if (size.height() > maxH) scale = std::min(scale, maxH / size.height());
    size *= scale;
    return QSizeF(std::clamp(size.width(), kLeastPictureSize, kMostPictureSize), std::clamp(size.height(), kLeastPictureSize, kMostPictureSize));
}

PictureResult insertPrepared(WriteDoc &doc, QTextCursor &c, const Prepared &p, const QString &name, PictureWrap wrap, const QString &alt)
{
    if (c.document() != doc.story()) {
        doc.pictureNotice = QCoreApplication::translate("jp::write::Pictures", "A picture can't be put in a header, a footer, or a note yet.");
        return PictureResult::Refused;
    }
    const QSizeF size = fitted(doc, c.selectionStart(), p.size);
    bool added = false;
    const QString rid = doc.addMedia(p.bytes, p.extension, p.contentType, &added);
    Model m;
    m.floating = wrap != PictureWrap::Inline;
    m.wrap = wrap;
    m.wrapRawOf = wrap;
    m.cx = emuOf(size.width());
    m.cy = emuOf(size.height());
    m.id = doc.nextDrawingId(doc.story());
    m.name = attribute(name.isEmpty() ? QStringLiteral("Picture %1").arg(m.id) : name, 255);
    m.descr = attribute(alt, kMostAltTextLength);
    if (m.floating) {
        m.distL = m.distR = 114300;
        if (wrap == PictureWrap::BehindText) m.relativeHeight = 251658240 - 1;
    }
    m.graphic = pictureGraphic(rid, m.cx, m.cy, m.name, m.id);
    Inline atom;
    atom.kind = Inline::RunAtom;
    atom.tag = QStringLiteral("drawing");
    atom.raw = drawingXml(m);
    if (!insertDrawnAtom(doc, c, atom)) {
        if (added) doc.removeMedia(rid);
        doc.pictureNotice = QCoreApplication::translate("jp::write::Pictures", "A picture can't be put here, or this document holds as many pictures and drawn objects as JeffWrite can show.");
        return PictureResult::Refused;
    }
    return PictureResult::Ok;
}

// Gives the character after `position` the picture `change` makes of its model: the element written again and the form the pages show, as one change of its format.
bool changePicture(WriteDoc &doc, int position, const std::function<bool(Model &)> &change)
{
    QTextDocument *story = doc.story();
    if (position < 0 || position + 1 > story->characterCount() - 1) return false;
    if (isLocked(story->findBlock(position))) return false;
    QTextCharFormat cf = formatAfter(story, position);
    Inline atom;
    if (!drawingAtomOf(cf, &atom)) return false;
    Model m;
    if (!parseDrawing(atom.raw, &m)) return false;
    const Model before = m;
    if (!change(m)) return false;
    atom.raw = drawingXml(m);
    // A new form of a picture is made before anything changes, so that a picture that cannot be shown leaves the document as it was.
    cf.clearProperty(flow::fp::AnchorId);
    cf.clearProperty(tp::InlineObject);
    setStored(cf, prop::Atom, packAtom(atom));
    if (!doc.makeDrawnFormat(atom, &cf)) {
        doc.pictureNotice = QCoreApplication::translate("jp::write::Pictures", "This document holds as many pictures and drawn objects as JeffWrite can show. Save it, and open it again, to change more.");
        return false;
    }
    Q_UNUSED(before);
    QTextCursor edit(story);
    edit.setPosition(position);
    edit.setPosition(position + 1, QTextCursor::KeepAnchor);
    edit.setCharFormat(cf);
    return true;
}

// The graphic's own size, the frame's `a:ext` (the first with a size): written again for a picture of another size.
QByteArray graphicWithSize(const QByteArray &graphic, qint64 cx, qint64 cy)
{
    static const QRegularExpression ext(QStringLiteral("<a:ext\\s+cx=\"[0-9]*\"\\s+cy=\"[0-9]*\""));
    QString text = QString::fromUtf8(graphic);
    const QRegularExpressionMatch found = ext.match(text);
    if (!found.hasMatch()) return graphic;
    text.replace(found.capturedStart(), found.capturedLength(), QStringLiteral("<a:ext cx=\"%1\" cy=\"%2\"").arg(cx).arg(cy));
    return text.toUtf8();
}

} // namespace

int WriteDoc::nextDrawingId(const QTextDocument *story)
{
    if (m_drawingId == 0) m_drawingId = highestDrawingId(story);   // (found once; a number is not used twice because a step was undone)
    return ++m_drawingId;
}

bool isPictureAt(const QTextDocument *story, int position)
{
    if (!story || position < 0 || position + 1 > story->characterCount() - 1) return false;
    Inline atom;
    if (!drawingAtomOf(formatAfter(story, position), &atom)) return false;
    return isPictureShape(readShapes(atom.raw));
}

PictureInfo pictureAt(const WriteDoc &doc, const QTextDocument *story, int position)
{
    Q_UNUSED(doc);
    PictureInfo info;
    if (!story || position < 0 || position + 1 > story->characterCount() - 1) return info;
    Inline atom;
    if (!drawingAtomOf(formatAfter(story, position), &atom)) return info;
    const QVector<Shape> shapes = readShapes(atom.raw);
    if (!isPictureShape(shapes)) return info;
    const Shape &s = shapes.first();
    info.valid = true;
    info.floating = s.floating;
    info.wrap = wrapOfShape(s);
    info.width = s.width;
    info.height = s.height;
    info.alt = s.alt;
    info.placeIsOffset = s.hAlign == Shape::NoAlign && s.vAlign == Shape::NoAlign;
    info.left = s.left;
    info.top = s.top;
    Model m;
    info.editable = parseDrawing(atom.raw, &m);
    return info;
}

PictureResult insertPicture(WriteDoc &doc, QTextCursor &c, const QByteArray &bytes, const QString &name, PictureWrap wrap, const QString &alt)
{
    Prepared p;
    if (const PictureResult r = prepare(bytes, &p); r != PictureResult::Ok) return r;
    return insertPrepared(doc, c, p, name, wrap, alt);
}

PictureResult insertPictureImage(WriteDoc &doc, QTextCursor &c, const QImage &image, PictureWrap wrap)
{
    Prepared p;
    if (const PictureResult r = encodeImage(image, &p); r != PictureResult::Ok) return r;
    return insertPrepared(doc, c, p, QString(), wrap, QString());
}

bool setPictureBox(WriteDoc &doc, int position, double width, double height, const std::optional<QPointF> &pageTopLeft)
{
    if (!std::isfinite(width) || !std::isfinite(height)) return false;
    if (pageTopLeft && (!std::isfinite(pageTopLeft->x()) || !std::isfinite(pageTopLeft->y()))) return false;
    const bool keepSize = width <= 0 || height <= 0;
    return changePicture(doc, position, [&](Model &m) {
        bool changed = false;
        const qint64 cx = keepSize ? m.cx : emuOf(std::clamp(width, kLeastPictureSize, kMostPictureSize)), cy = keepSize ? m.cy : emuOf(std::clamp(height, kLeastPictureSize, kMostPictureSize));
        if (m.cx != cx || m.cy != cy) {
            m.graphic = graphicWithSize(m.graphic, cx, cy);
            m.cx = cx;
            m.cy = cy;
            changed = true;
        }
        if (pageTopLeft) {
            const qint64 x = qint64(std::llround(std::clamp(pageTopLeft->x(), -kMostPictureOffset, kMostPictureOffset) * kEmuPerPoint));
            const qint64 y = qint64(std::llround(std::clamp(pageTopLeft->y(), -kMostPictureOffset, kMostPictureOffset) * kEmuPerPoint));
            const QByteArray posH = "<wp:positionH relativeFrom=\"page\"><wp:posOffset>" + QByteArray::number(x) + "</wp:posOffset></wp:positionH>";
            const QByteArray posV = "<wp:positionV relativeFrom=\"page\"><wp:posOffset>" + QByteArray::number(y) + "</wp:posOffset></wp:positionV>";
            if (!m.floating) {
                m.floating = true;
                m.distL = m.distR = 114300;
                m.distT = m.distB = 0;
                m.wrap = PictureWrap::Square;
            }
            if (m.posH != posH || m.posV != posV) changed = true;
            m.posH = posH;
            m.posV = posV;
        }
        return changed;
    });
}

bool setPictureSize(WriteDoc &doc, int position, double width, double height) { return setPictureBox(doc, position, width, height, std::nullopt); }

bool setPictureWrap(WriteDoc &doc, int position, PictureWrap wrap)
{
    return changePicture(doc, position, [&](Model &m) {
        if (m.wrap == wrap) return false;
        if (wrap == PictureWrap::Inline) {
            m.floating = false;
            m.distL = m.distR = m.distT = m.distB = 0;
        } else {
            if (!m.floating) {
                m.floating = true;
                m.distL = m.distR = 114300;
                m.distT = m.distB = 0;
            }
            m.relativeHeight = wrap == PictureWrap::BehindText ? 251658239 : 251658240;
        }
        m.wrap = wrap;
        return true;
    });
}

bool setPictureAltText(WriteDoc &doc, int position, const QString &alt)
{
    const QByteArray text = attribute(alt, kMostAltTextLength);
    return changePicture(doc, position, [&](Model &m) {
        if (m.descr == text) return false;
        m.descr = text;
        return true;
    });
}

bool setPicturePlace(WriteDoc &doc, int position, double pageX, double pageY)
{
    return setPictureBox(doc, position, 0, 0, QPointF(pageX, pageY));
}

bool movePictureInText(WriteDoc &doc, int position, int to)
{
    QTextDocument *story = doc.story();
    if (!isPictureAt(story, position) || to < 0 || to > editableEnd(story) || (to >= position && to <= position + 1)) return false;
    if (isLocked(story->findBlock(position)) || isLocked(story->findBlock(to))) return false;
    const QTextCharFormat cf = formatAfter(story, position);
    QTextCursor edit(story);
    edit.beginEditBlock();
    edit.setPosition(position);
    edit.setPosition(position + 1, QTextCursor::KeepAnchor);
    edit.removeSelectedText();
    edit.setPosition(to > position ? to - 1 : to);
    edit.insertText(QString(QChar::ObjectReplacementCharacter), cf);
    edit.endEditBlock();
    return true;
}

} // namespace jp::write
