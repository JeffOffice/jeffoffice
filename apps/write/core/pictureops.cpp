#include "core/pictureops.h"

#include "core/drawnitems.h"
#include "core/editops.h"
#include "office/chart.h"
#include "office/chart_write.h"
#include "office/drawingml_color.h"
#include "office/theme.h"
#include "sheet/chartmake.h"
#include "core/writeprops.h"
#include "text/textprops.h"
#include "wml/wml.h"
#include "wml/xmltree.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextFragment>
#include <algorithm>
#include <array>
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
        "xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\" xmlns:wp14=\"http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing\"";
    return d;
}

// Text kept in an attribute or as alt text: no character XML refuses, a pair of surrogates whole and a lone one dropped, and at most `limit` UTF-16 units (a pair is not cut).
QString cleanText(const QString &text, int limit)
{
    QString clean;
    clean.reserve(int(std::min<qsizetype>(text.size(), limit)));
    for (qsizetype i = 0; i < text.size(); ++i) {
        const QChar ch = text[i];
        const ushort u = ch.unicode();
        if (ch.isHighSurrogate()) {
            if (i + 1 < text.size() && text[i + 1].isLowSurrogate()) {
                if (clean.size() + 2 > limit) break;
                clean += ch;
                clean += text[i + 1];
                ++i;
            }
            continue;
        }
        if (ch.isLowSurrogate() || u == 0xFFFE || u == 0xFFFF) continue;
        if (clean.size() >= limit) break;
        clean += u < 0x20 && u != 0x09 ? QChar(QLatin1Char(' ')) : ch;
    }
    return clean;
}

QByteArray attribute(const QString &text, int limit) { return cleanText(text, limit).toHtmlEscaped().toUtf8(); }

// The attributes of a start tag, in order, as written (the values with their quotes); the declarations of prefixes are kept apart.
using Attributes = QVector<std::pair<QByteArray, QByteArray>>;

Attributes attributesOf(const QByteArray &startTag, Attributes *declarations = nullptr)
{
    Attributes out;
    static const QRegularExpression re(QStringLiteral("\\s([\\w:.-]+)\\s*=\\s*(\"[^\"]*\"|'[^']*')"));
    const QString text = QString::fromUtf8(startTag);
    const qsizetype from = text.indexOf(QLatin1Char(' '));
    if (from < 0) return out;
    auto it = re.globalMatch(text, from);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        const QByteArray name = m.captured(1).toUtf8(), value = m.captured(2).toUtf8();
        if (name == "xmlns" || name.startsWith("xmlns:")) {
            if (declarations) declarations->push_back({name, value});
        } else {
            out.push_back({name, value});
        }
    }
    return out;
}

void setAttribute(Attributes *attrs, const QByteArray &name, const QByteArray &quoted)
{
    for (auto &a : *attrs)
        if (a.first == name) {
            a.second = quoted;
            return;
        }
    attrs->push_back({name, quoted});
}

void removeAttribute(Attributes *attrs, const QByteArray &name)
{
    for (qsizetype i = 0; i < attrs->size(); ++i)
        if ((*attrs)[i].first == name) {
            attrs->removeAt(i);
            return;
        }
}

QByteArray attributeValue(const Attributes &attrs, const QByteArray &name)
{
    for (const auto &a : attrs)
        if (a.first == name) return a.second.mid(1, a.second.size() - 2);
    return {};
}

QByteArray written(const Attributes &attrs)
{
    QByteArray out;
    for (const auto &a : attrs) out += " " + a.first + "=" + a.second;
    return out;
}

QByteArray quote(const QByteArray &value) { return "\"" + value + "\""; }

// A drawing of one picture, as its parts: what the program writes again when the picture changes, and everything else of the element, which it keeps as it was.
struct Model {
    bool floating = false;
    qint64 cx = 0, cy = 0;                    // EMU
    PictureWrap wrap = PictureWrap::Inline;
    QByteArray wrapRaw;                       // the wrap element as the file had it (kept while the kind of wrap is)
    PictureWrap wrapRawOf = PictureWrap::Inline;
    qint64 distT = 0, distB = 0, distL = 0, distR = 0;
    QByteArray posH, posV;                    // positionH and positionV as written
    QByteArray effectExtent, frameProps, graphic;
    qint64 relativeHeight = 251658240;
    Attributes declarations;                  // the prefixes the drawing declares
    Attributes itemAttributes;                // the attributes of the inline or anchor the program does not set (wp14:anchorId and the like)
    Attributes anchorFlags;                   // locked, layoutInCell, allowOverlap, simplePos as written
    Attributes docPr;                         // the attributes of wp:docPr (id, name, descr, hidden, title...)
    QByteArray docPrInner;                    // what is inside wp:docPr (a hyperlink on the picture...)
    QByteArray between;                       // children between the wrap and the graphic that the program does not know
    struct Kept {
        QByteArray name, raw;
    };
    QVector<Kept> trailing;                   // children after the graphic (the newer format's size relations...)
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

// Children that only an anchored drawing may have.
bool anchorOnly(const QByteArray &name) { return name == "sizeRelH" || name == "sizeRelV" || name == "pctPosHOffset" || name == "pctPosVOffset" || name == "pctPosHeight" || name == "pctPosWidth"; }

QByteArray drawingXml(const Model &m)
{
    auto n = [](qint64 v) { return QByteArray::number(v); };
    Attributes decl = m.declarations;
    for (const auto &required : {std::pair<QByteArray, QByteArray>{"xmlns:wp", "\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\""},
                                 {"xmlns:a", "\"http://schemas.openxmlformats.org/drawingml/2006/main\""},
                                 {"xmlns:pic", "\"http://schemas.openxmlformats.org/drawingml/2006/picture\""},
                                 {"xmlns:r", "\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\""}}) {
        bool has = false;
        for (const auto &d : decl) has = has || d.first == required.first;
        if (!has) decl.push_back(required);
    }
    QByteArray x = "<w:drawing" + written(decl) + ">";
    const QByteArray extent = "<wp:extent cx=\"" + n(m.cx) + "\" cy=\"" + n(m.cy) + "\"/>";
    QByteArray docPr = "<wp:docPr" + written(m.docPr);
    docPr += m.docPrInner.isEmpty() ? QByteArray("/>") : ">" + m.docPrInner + "</wp:docPr>";
    const QByteArray frame = m.frameProps.isEmpty() ? QByteArray("<wp:cNvGraphicFramePr><a:graphicFrameLocks noChangeAspect=\"1\"/></wp:cNvGraphicFramePr>") : m.frameProps;
    QByteArray trailing;
    for (const Model::Kept &k : m.trailing)
        if (m.floating || !anchorOnly(k.name)) trailing += k.raw;
    if (!m.floating) {
        Attributes attrs = m.itemAttributes;
        QByteArray start = "<wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"" + written(attrs) + ">";
        x += start + extent + m.effectExtent + m.between + docPr + frame + m.graphic + trailing + "</wp:inline>";
    } else {
        const bool behind = m.wrap == PictureWrap::BehindText;
        Attributes flags = m.anchorFlags;
        auto flag = [&](const char *name, const char *fallback) {
            const QByteArray v = attributeValue(flags, name);
            return v.isEmpty() ? QByteArray(fallback) : v;
        };
        x += "<wp:anchor distT=\"" + n(m.distT) + "\" distB=\"" + n(m.distB) + "\" distL=\"" + n(m.distL) + "\" distR=\"" + n(m.distR) + "\" simplePos=\"" + flag("simplePos", "0") + "\" relativeHeight=\"" +
             n(m.relativeHeight) + "\" behindDoc=\"" + (behind ? "1" : "0") + "\" locked=\"" + flag("locked", "0") + "\" layoutInCell=\"" + flag("layoutInCell", "1") + "\" allowOverlap=\"" +
             flag("allowOverlap", "1") + "\"" + written(m.itemAttributes) + ">";
        x += "<wp:simplePos x=\"0\" y=\"0\"/>" + (m.posH.isEmpty() ? defaultPosH() : m.posH) + (m.posV.isEmpty() ? defaultPosV() : m.posV) + extent + m.effectExtent;
        x += !m.wrapRaw.isEmpty() && m.wrapRawOf == m.wrap ? m.wrapRaw : wrapElement(m.wrap);
        x += m.between + docPr + frame + m.graphic + trailing + "</wp:anchor>";
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
    // (What the drawing declares itself, the wrapper's own declarations left out.)
    {
        Attributes all;
        const Attributes mine = attributesOf(raw.left(raw.indexOf('>') + 1), &all);
        Q_UNUSED(mine);
        out.declarations = all;
    }
    out.floating = t.localName(item) == "anchor";
    out.distT = emuAttr(t, item, "distT");
    out.distB = emuAttr(t, item, "distB");
    out.distL = emuAttr(t, item, "distL");
    out.distR = emuAttr(t, item, "distR");
    if (t.has(item, "relativeHeight")) out.relativeHeight = std::clamp<qint64>(t.attr(item, "relativeHeight").toLongLong(), 0, 4294967295LL);
    const bool behind = t.attr(item, "behindDoc") == QLatin1String("1") || t.attr(item, "behindDoc") == QLatin1String("true");
    {
        Attributes ignored;
        const Attributes attrs = attributesOf(t.startTag(item), &ignored);
        static const QSet<QByteArray> set = {"distT", "distB", "distL", "distR", "relativeHeight", "behindDoc"};
        static const QSet<QByteArray> flags = {"locked", "layoutInCell", "allowOverlap", "simplePos"};
        for (const auto &a : attrs) {
            if (set.contains(a.first)) continue;
            if (flags.contains(a.first)) out.anchorFlags.push_back(a);
            else out.itemAttributes.push_back(a);
        }
    }
    bool wrapNone = false, sawGraphic = false;
    for (int k = t.firstChild(item); k >= 0; k = t.next(k)) {
        const QByteArray tag = t.localName(k);
        if (tag == "extent") {
            out.cx = emuAttr(t, k, "cx");
            out.cy = emuAttr(t, k, "cy");
        } else if (tag == "simplePos") {
            continue;
        } else if (tag == "effectExtent") {
            out.effectExtent = t.raw(k);
        } else if (tag == "docPr") {
            Attributes ignored;
            out.docPr = attributesOf(t.startTag(k), &ignored);
            out.docPrInner = t.isEmptyElement(k) ? QByteArray() : t.inner(k);
        } else if (tag == "cNvGraphicFramePr") {
            out.frameProps = t.raw(k);
        } else if (tag == "graphic") {
            out.graphic = t.raw(k);
            sawGraphic = true;
        } else if (tag == "positionH") {
            out.posH = t.raw(k);
        } else if (tag == "positionV") {
            out.posV = t.raw(k);
        } else if (tag == "wrapSquare" || tag == "wrapThrough" || tag == "wrapTight" || tag == "wrapTopAndBottom" || tag == "wrapNone") {
            out.wrapRaw = t.raw(k);
            out.wrapRawOf = tag == "wrapSquare" ? PictureWrap::Square : tag == "wrapTopAndBottom" ? PictureWrap::TopAndBottom : tag == "wrapNone" ? PictureWrap::InFrontOfText : PictureWrap::Tight;
            wrapNone = tag == "wrapNone";
        } else if (sawGraphic) {
            out.trailing.push_back({tag, t.raw(k)});
        } else {
            out.between += t.raw(k);
        }
    }
    const bool isChart = out.graphic.contains("drawingml/2006/chart") && out.graphic.contains("<c:chart");
    const bool isShape = out.graphic.contains("wsp") && !out.graphic.contains("blip");
    // (A line may have no height or no width, but not neither.)
    if (out.graphic.isEmpty() || !(out.graphic.contains("blip") || isShape || isChart) || (isShape ? out.cx < 0 || out.cy < 0 || out.cx + out.cy <= 0 : out.cx <= 0 || out.cy <= 0)) return false;
    if (out.docPr.isEmpty()) out.docPr = {{"id", "\"1\""}, {"name", "\"Picture\""}};
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

// The highest id a drawing in these bytes has (a wp:docPr's).
int highestDrawingIdIn(const QByteArray &bytes)
{
    int highest = 0;
    for (qsizetype at = bytes.indexOf("docPr"); at >= 0; at = bytes.indexOf("docPr", at + 5)) {
        const qsizetype id = bytes.indexOf(" id=\"", at);
        if (id < 0 || id - at > 400) continue;
        int value = 0, digits = 0;
        for (qsizetype i = id + 5; i < bytes.size() && bytes[i] >= '0' && bytes[i] <= '9' && digits < 9; ++i, ++digits) value = value * 10 + (bytes[i] - '0');
        highest = std::max(highest, value);
    }
    return highest;
}

// The highest id the text's drawings have.
int highestDrawingId(const QTextDocument *story)
{
    int highest = 0;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (bytes.contains("docPr")) highest = std::max(highest, highestDrawingIdIn(bytes));
        }
    return std::min(highest, 2000000000 - 2);
}

// The character format of the character just after `position` (its own fragment's: a cursor at the start of a paragraph would give the paragraph's first character's).
QTextCharFormat formatAfter(const QTextDocument *story, int position)
{
    for (QTextBlock::iterator it = story->findBlock(position).begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        if (position >= f.position() && position < f.position() + f.length()) return f.charFormat();
    }
    return QTextCharFormat();
}

bool isPictureShape(const QVector<Shape> &shapes) { return !shapes.isEmpty() && shapes.first().kind == Shape::Picture && shapes.first().chartEmbed.isEmpty(); }

// A drawn object's kind: a shape of the drawing format that is a line is a line, one with text is a text box.
DrawnKind kindOfShape(const Shape &s)
{
    if (!s.chartEmbed.isEmpty()) return DrawnKind::Chart;
    if (s.kind == Shape::Picture) return DrawnKind::Picture;
    if (s.kind == Shape::TextBox) return DrawnKind::TextBox;
    if (s.kind == Shape::Line) return DrawnKind::Line;
    return s.wsp.contains("prst=\"line\"") || s.wsp.contains("prst=\"straightConnector1\"") ? DrawnKind::Line : DrawnKind::Shape;
}

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

// Whether bytes are an SVG file (by what they begin with, whatever plugin is there to read them).
bool looksLikeSvg(const QByteArray &bytes)
{
    if (bytes.startsWith("\x1f\x8b")) return false;
    return bytes.contains("<svg") || bytes.contains("<SVG");
}

// Whether an SVG refers to anything outside itself: a file, an address, a style sheet, or a definition of its own that a reader might load. What it holds of its own (a link to an
// element of it, or an image carried in it as data) is its own.
bool svgReachesOutside(const QByteArray &svg)
{
    const QString text = QString::fromUtf8(svg);
    static const QRegularExpression doctype(QStringLiteral("<!\\s*(DOCTYPE|ENTITY)|<\\?xml-stylesheet|@import"), QRegularExpression::CaseInsensitiveOption);
    if (doctype.match(text).hasMatch()) return true;
    static const QRegularExpression href(QStringLiteral("(?:^|[\\s:])(?:href|src)\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)')"), QRegularExpression::CaseInsensitiveOption);
    for (auto it = href.globalMatch(text); it.hasNext();) {
        const QRegularExpressionMatch m = it.next();
        const QString value = (m.captured(1).isEmpty() ? m.captured(2) : m.captured(1)).trimmed();
        if (!value.startsWith(QLatin1Char('#')) && !value.startsWith(QLatin1String("data:"), Qt::CaseInsensitive)) return true;
    }
    static const QRegularExpression url(QStringLiteral("url\\(\\s*[\"']?\\s*([^)\"']*)"), QRegularExpression::CaseInsensitiveOption);
    for (auto it = url.globalMatch(text); it.hasNext();) {
        const QString value = it.next().captured(1).trimmed();
        if (!value.startsWith(QLatin1Char('#')) && !value.startsWith(QLatin1String("data:"), Qt::CaseInsensitive)) return true;
    }
    return false;
}

PictureResult prepare(const QByteArray &input, Prepared *out)
{
    if (input.isEmpty()) return PictureResult::Unreadable;
    if (qint64(input.size()) > kMostPictureBytes) return PictureResult::TooLarge;
    if (looksLikeSvg(input)) {
        if (qint64(input.size()) > kMostSvgBytes) return PictureResult::TooLarge;
        if (svgReachesOutside(input)) return PictureResult::External;
    } else if (input.startsWith("\x1f\x8b")) {
        return PictureResult::Unreadable;   // (a compressed SVG hides what it refers to)
    }
    QByteArray copy = input;
    QBuffer buffer(&copy);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer);
    const QByteArray format = reader.format().toLower();
    // (A file the reader takes for an SVG is looked through whatever it begins with.)
    if ((format == "svg" || format == "svgz") && !looksLikeSvg(input)) return PictureResult::External;
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
    // (The pictures of a document come to no more than the total.)
    if (added && doc.mediaBytes() > doc.mediaLimit) {
        doc.removeMedia(rid);
        doc.pictureNotice = QCoreApplication::translate("jp::write::Pictures", "The pictures in this document are as large together as JeffWrite puts in one, so this one is not added.");
        return PictureResult::TooLarge;
    }
    Model m;
    m.floating = wrap != PictureWrap::Inline;
    m.wrap = wrap;
    m.wrapRawOf = wrap;
    m.cx = emuOf(size.width());
    m.cy = emuOf(size.height());
    const int id = doc.nextDrawingId(doc.story());
    const QByteArray pictureName = attribute(name.isEmpty() ? QStringLiteral("Picture %1").arg(id) : name, 255);
    m.docPr = {{"id", quote(QByteArray::number(id))}, {"name", quote(pictureName)}};
    if (!alt.isEmpty()) m.docPr.push_back({"descr", quote(attribute(alt, kMostAltTextLength))});
    if (m.floating) {
        m.distL = m.distR = 114300;
        if (wrap == PictureWrap::BehindText) m.relativeHeight = 251658240 - 1;
    }
    m.graphic = pictureGraphic(rid, m.cx, m.cy, pictureName, id);
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
    const QTextCharFormat was = formatAfter(story, position);
    edit.setCharFormat(cf);
    doc.retireDrawnForm(was);
    doc.keepObjectsForTheHistory();
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
    if (m_drawingId == 0) {
        // The text's drawings, and every part of the file that might have one (the headers, the footers, the notes).
        m_drawingId = highestDrawingId(story);
        for (const KeptPart &part : std::as_const(m_wml.parts))
            if (part.contentType.contains(QLatin1String("xml"))) m_drawingId = std::max(m_drawingId, std::min(highestDrawingIdIn(part.data), 2000000000 - 2));
        for (const HeaderPart &hp : std::as_const(m_headerParts))
            if (hp.story) m_drawingId = std::max(m_drawingId, highestDrawingId(hp.story.get()));
        auto inBlocks = [&](const QHash<QString, QVector<Block>> &notes) {
            for (const QVector<Block> &blocks : notes)
                for (const Block &b : blocks) {
                    m_drawingId = std::max(m_drawingId, std::min(highestDrawingIdIn(b.raw), 2000000000 - 2));
                    for (const Inline &in : b.para.content) m_drawingId = std::max(m_drawingId, std::min(highestDrawingIdIn(in.raw), 2000000000 - 2));
                }
        };
        inBlocks(m_noteBlocks);
        inBlocks(m_endnoteBlocks);
    }
    return ++m_drawingId;
}

void WriteDoc::forgetDrawingIds() { m_drawingId = 0; }

QTextCharFormat charFormatAt(const QTextDocument *story, int position) { return formatAfter(story, position); }

bool isPictureAt(const QTextDocument *story, int position)
{
    if (!story || position < 0 || position + 1 > story->characterCount() - 1) return false;
    Inline atom;
    if (!drawingAtomOf(formatAfter(story, position), &atom)) return false;
    return isPictureShape(readShapes(atom.raw));
}

namespace {

const QByteArray kDecorativeUri = "{C183D7F6-B498-43B3-948B-1728B52AA6E4}";

bool isDecorativeInner(const QByteArray &inner) { return inner.contains(kDecorativeUri) && inner.contains("decorative") && !inner.contains("val=\"0\"") && !inner.contains("val=\"false\""); }

} // namespace

PictureInfo pictureAt(const WriteDoc &doc, const QTextDocument *story, int position)
{
    PictureInfo info;
    if (!story || position < 0 || position + 1 > story->characterCount() - 1) return info;
    Inline atom;
    if (!drawingAtomOf(formatAfter(story, position), &atom)) return info;
    const QVector<Shape> shapes = readShapes(atom.raw);
    if (shapes.isEmpty()) return info;
    const Shape &s = shapes.first();
    info.kind = kindOfShape(s);
    info.valid = true;
    info.floating = s.floating;
    info.wrap = wrapOfShape(s);
    info.width = s.width;
    info.height = s.height;
    info.alt = s.alt;
    info.placeIsOffset = s.hAlign == Shape::NoAlign && s.vAlign == Shape::NoAlign;
    info.left = s.left;
    info.top = s.top;
    {
        auto word = [](Shape::Align a, bool across) -> QString {
            switch (a) {
            case Shape::Start: return across ? QStringLiteral("left") : QStringLiteral("top");
            case Shape::Center: return QStringLiteral("center");
            case Shape::End: return across ? QStringLiteral("right") : QStringLiteral("bottom");
            case Shape::Inside: return QStringLiteral("inside");
            case Shape::Outside: return QStringLiteral("outside");
            default: return {};
            }
        };
        info.alignAcross = word(s.hAlign, true);
        info.alignDown = word(s.vAlign, false);
        info.alignedToPage = s.hRel == Shape::Page || s.vRel == Shape::Page;
    }
    Model m;
    info.editable = parseDrawing(atom.raw, &m);
    info.decorative = isDecorativeInner(m.docPrInner);
    if (info.kind == DrawnKind::Picture) {
        info.cropLeft = s.cropL;
        info.cropTop = s.cropT;
        info.cropRight = s.cropR;
        info.cropBottom = s.cropB;
        info.lineColor = s.borderWeight > 0 ? s.borderColor : QString();
        info.lineWeight = s.borderWeight;
        info.cropShape = s.cropShape.isEmpty() ? QStringLiteral("rect") : s.cropShape;
        info.rotation = s.rotation;
        info.grayscale = m.graphic.contains("<a:grayscl");
        info.blackWhite = m.graphic.contains("<a:biLevel");
        static const QRegularExpression lum(QStringLiteral("<a:lum\\b([^>]*)/?>"));
        const QRegularExpressionMatch found = lum.match(QString::fromUtf8(m.graphic));
        if (found.hasMatch()) {
            auto share = [&](const char *name) {
                const QRegularExpressionMatch v = QRegularExpression(QStringLiteral("\\b%1=\"(-?[0-9]+)\"").arg(QLatin1String(name))).match(found.captured(1));
                return v.hasMatch() ? std::clamp(v.captured(1).toDouble() / 100000.0, -1.0, 1.0) : 0.0;
            };
            info.brightness = share("bright");
            info.contrast = share("contrast");
        }
        return info;
    }
    if (const ItemPtr look = wspItem(s, doc.theme())) {
        if (const auto *shape = look->type() == ItemType::Shape ? static_cast<const ShapeItem *>(look.get()) : nullptr) info.preset = shape->shape;
        auto rgb = [](const ColorRef &c) { return c.rgbValue().isValid() ? c.rgbValue().name(QColor::HexRgb).mid(1).toUpper() : QString(); };
        if (!look->fill.isNone() && look->fill.type == Fill::Solid) info.fillColor = rgb(look->fill.color);
        if (look->fill.type == Fill::Gradient) {
            const Fill &g = look->fill;
            info.gradientFrom = rgb(g.stops.isEmpty() ? g.color : g.stops.first().color);
            info.gradientTo = rgb(g.stops.isEmpty() ? g.color2 : g.stops.last().color);
            info.gradientAngle = g.angle;
        }
        if (!look->stroke.isNone()) {
            info.lineColor = rgb(look->stroke.color);
            info.lineWeight = look->stroke.width;
            static const char *names[] = {"solid", "dot", "sysDot", "dash", "dashDot", "lgDash", "lgDashDot", "lgDashDotDot"};
            info.lineDash = QString::fromLatin1(names[std::clamp(int(look->stroke.dash), 0, 7)]);
        }
        info.rotation = look->rotation;
    }
    return info;
}

bool isDrawnObjectAt(const QTextDocument *story, int position)
{
    if (!story || position < 0 || position + 1 > story->characterCount() - 1) return false;
    Inline atom;
    if (!drawingAtomOf(formatAfter(story, position), &atom)) return false;
    return !readShapes(atom.raw).isEmpty();
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

namespace {

// A floating picture's place in the layers of the page: where it is, its height (w:relativeHeight), and whether it is in the layer behind the text.
struct Layer {
    int position = 0;
    qint64 z = 0;
    bool behind = false;
};

// The floating pictures of the story, the ones behind the text first, each layer from the lowest to the highest (pictures of the same height by their place in the text).
QVector<Layer> floatingLayers(WriteDoc &doc)
{
    QVector<Layer> out;
    QTextDocument *story = doc.story();
    for (int p = 0; p + 1 < story->characterCount(); ++p) {
        if (!isPictureAt(story, p)) continue;
        Inline atom;
        Model m;
        if (!drawingAtomOf(formatAfter(story, p), &atom) || !parseDrawing(atom.raw, &m) || !m.floating) continue;
        out.push_back({p, m.relativeHeight, m.wrap == PictureWrap::BehindText});
    }
    std::stable_sort(out.begin(), out.end(), [](const Layer &a, const Layer &b) { return a.behind != b.behind ? a.behind : a.z < b.z; });
    return out;
}


// What an order command changes: the new height of each picture that moves, and whether the one asked for ends up behind the text.
struct OrderPlan {
    bool any = false;
    bool toBehind = false;
    QVector<std::pair<int, qint64>> heights;   // position, new height
};

OrderPlan planPictureOrder(WriteDoc &doc, int position, PictureOrder order)
{
    OrderPlan none;
    constexpr qint64 kMostHeight = 4294967295LL;
    const QVector<Layer> all = floatingLayers(doc);
    QVector<Layer> behind, front;
    Layer mine;
    bool found = false;
    for (const Layer &l : all) {
        if (l.position == position) {
            mine = l;
            found = true;
        } else {
            (l.behind ? behind : front).push_back(l);
        }
    }
    if (!found) return none;
    // Where it stands in its own layer now (among the others of it).
    QVector<Layer> &own = mine.behind ? behind : front;
    int index = 0;
    while (index < own.size() && (own[index].z < mine.z || (own[index].z == mine.z && own[index].position < mine.position))) ++index;
    bool toBehind = mine.behind;
    int target = index;
    switch (order) {
    case PictureOrder::BringToFront:
        if (!mine.behind && index == front.size()) return none;
        toBehind = false;
        target = front.size();
        break;
    case PictureOrder::SendToBack:
        if (index == 0) return none;
        target = 0;
        break;
    case PictureOrder::BringForward:
        if (index < own.size()) {
            target = index + 1;
        } else if (mine.behind) {
            toBehind = false;
            target = 0;
        } else {
            return none;
        }
        break;
    case PictureOrder::SendBackward:
        if (index > 0) {
            target = index - 1;
        } else if (!mine.behind) {
            toBehind = true;
            target = behind.size();
        } else {
            return none;
        }
        break;
    }
    QVector<Layer> &into = toBehind ? behind : front;
    target = std::clamp(target, 0, int(into.size()));
    mine.behind = toBehind;
    into.insert(target, mine);
    // Heights rise along the layer from the picture put in on: its own stays if it fits between its new neighbors, else it is the one above the lower; each one above it that is not
    // higher than the one before is raised to the next height. Only a picture whose height changes is written again.
    const qint64 prevZ = target > 0 ? into[target - 1].z : -1;
    const qint64 nextZ = target + 1 < into.size() ? into[target + 1].z : kMostHeight + 1;
    qint64 z = mine.z;
    if (!(z > prevZ && z < nextZ)) z = prevZ >= 0 ? prevZ + 1 : std::max<qint64>(0, nextZ - 1);
    QVector<std::pair<int, qint64>> heights;   // position, new height
    heights.push_back({position, z});
    for (int i = target + 1; i < into.size(); ++i) {
        const qint64 raised = std::max(into[i].z, z + 1);
        if (raised > kMostHeight) return none;
        if (raised != into[i].z) heights.push_back({into[i].position, raised});
        z = raised;
    }
    OrderPlan plan;
    plan.any = true;
    plan.toBehind = toBehind;
    plan.heights = heights;
    return plan;
}

} // namespace

bool canOrderPicture(WriteDoc &doc, int position, PictureOrder order) { return planPictureOrder(doc, position, order).any; }

bool setPictureOrder(WriteDoc &doc, int position, PictureOrder order)
{
    const OrderPlan plan = planPictureOrder(doc, position, order);
    if (!plan.any) return false;
    const WriteDoc::StepGroup group(doc);
    bool changed = false;
    for (const auto &h : plan.heights) {
        const bool isMine = h.first == position;
        changed |= changePicture(doc, h.first, [&](Model &m) {
            const PictureWrap was = m.wrap;
            if (isMine && plan.toBehind != (m.wrap == PictureWrap::BehindText)) m.wrap = plan.toBehind ? PictureWrap::BehindText : PictureWrap::InFrontOfText;
            if (m.relativeHeight == h.second && m.wrap == was) return false;
            m.relativeHeight = h.second;
            return true;
        });
    }
    return changed;
}

bool setPictureAltText(WriteDoc &doc, int position, const QString &alt)
{
    const QByteArray text = attribute(alt, kMostAltTextLength);
    return changePicture(doc, position, [&](Model &m) {
        if (attributeValue(m.docPr, "descr") == text) return false;
        if (text.isEmpty()) removeAttribute(&m.docPr, "descr");
        else setAttribute(&m.docPr, "descr", quote(text));
        return true;
    });
}

bool setPictureDecorative(WriteDoc &doc, int position, bool decorative)
{
    return changePicture(doc, position, [&](Model &m) {
        if (isDecorativeInner(m.docPrInner) == decorative) return false;
        // The mark is an extension of the picture's properties (the newer format's), the last child of wp:docPr.
        static const QByteArray open = "<a:ext uri=\"{C183D7F6-B498-43B3-948B-1728B52AA6E4}\">";
        if (!decorative) {
            const qsizetype from = m.docPrInner.indexOf(open);
            if (from < 0) return false;
            const qsizetype to = m.docPrInner.indexOf("</a:ext>", from);
            if (to < 0) return false;
            m.docPrInner.remove(from, to + 8 - from);
            // (an extension list with nothing left in it goes)
            static const QRegularExpression emptyList(QStringLiteral("<a:extLst(?: [^>]*)?>\\s*</a:extLst>"));
            m.docPrInner = QString::fromUtf8(m.docPrInner).remove(emptyList).toUtf8();
            return true;
        }
        const QByteArray ext = open + "<adec:decorative xmlns:adec=\"http://schemas.microsoft.com/office/drawing/2017/decorative\" val=\"1\"/></a:ext>";
        const qsizetype list = m.docPrInner.lastIndexOf("</a:extLst>");
        if (list >= 0) m.docPrInner.insert(list, ext);
        else m.docPrInner += "<a:extLst xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">" + ext + "</a:extLst>";
        // A picture that adds nothing has no description (a screen reader skips it).
        removeAttribute(&m.docPr, "descr");
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

// ---- Shapes and text boxes

namespace {

constexpr double kMostBoxTextLength = 50000;
constexpr int kMostBoxParagraphs = 2000;

// The children of a drawing's shape properties (its spPr), in the order the standard gives them.
enum Slot { SlotXfrm, SlotGeometry, SlotFill, SlotLine, SlotEffects, SlotScene, SlotSolid3d, SlotExt, SlotOther };

int slotOf(const QByteArray &local)
{
    if (local == "xfrm") return SlotXfrm;
    if (local == "custGeom" || local == "prstGeom") return SlotGeometry;
    if (local == "noFill" || local == "solidFill" || local == "gradFill" || local == "blipFill" || local == "pattFill" || local == "grpFill") return SlotFill;
    if (local == "ln") return SlotLine;
    if (local == "effectLst" || local == "effectDag") return SlotEffects;
    if (local == "scene3d") return SlotScene;
    if (local == "sp3d") return SlotSolid3d;
    if (local == "extLst") return SlotExt;
    return SlotOther;
}

struct SpPr {
    qsizetype begin = -1, end = -1;   // in the graphic: from the start tag to just after the end tag
    QByteArray prefix;                // with its colon ("wps:", "pic:")
    QByteArray startTag;
    bool selfClosing = false;
    struct Child {
        int slot = SlotOther;
        QByteArray local, raw;
    };
    QVector<Child> children;
};

// The first spPr of a graphic (a shape's or a picture's), split into its children.
bool splitSpPr(const QByteArray &graphic, SpPr *out)
{
    for (qsizetype i = graphic.indexOf("spPr"); i >= 0; i = graphic.indexOf("spPr", i + 4)) {
        const qsizetype lt = graphic.lastIndexOf('<', i);
        if (lt < 0) return false;
        const QByteArray between = graphic.mid(lt + 1, i - lt - 1);
        if (!(between.isEmpty() || (between.endsWith(':') && between.size() < 12 && !between.contains(' ') && !between.contains('>') && !between.contains('<')))) continue;
        if (i + 4 >= graphic.size() || !(graphic[i + 4] == ' ' || graphic[i + 4] == '>' || graphic[i + 4] == '/')) continue;
        const qsizetype gt = graphic.indexOf('>', i);
        if (gt < 0) return false;
        SpPr sp;
        sp.begin = lt;
        sp.prefix = between;
        sp.startTag = graphic.mid(lt, gt + 1 - lt);
        sp.selfClosing = graphic[gt - 1] == '/';
        if (sp.selfClosing) {
            sp.end = gt + 1;
        } else {
            const QByteArray close = "</" + between + "spPr>";
            const qsizetype ce = graphic.indexOf(close, gt);
            if (ce < 0) return false;
            sp.end = ce + close.size();
            XmlTree t;
            if (!t.parse("<x " + declarations() + ">" + graphic.mid(gt + 1, ce - gt - 1) + "</x>", nullptr)) return false;
            for (int c = t.firstChild(t.root()); c >= 0; c = t.next(c)) {
                SpPr::Child child;
                child.local = t.localName(c);
                child.slot = slotOf(child.local);
                child.raw = t.raw(c);
                sp.children.push_back(child);
            }
        }
        *out = sp;
        return true;
    }
    return false;
}

QByteArray joinSpPr(const QByteArray &graphic, const SpPr &sp)
{
    QByteArray inner;
    for (const SpPr::Child &c : sp.children) inner += c.raw;
    if (inner.isEmpty() && sp.selfClosing) return graphic;
    const QByteArray start = sp.selfClosing ? sp.startTag.left(sp.startTag.size() - 2) + ">" : sp.startTag;
    return graphic.left(sp.begin) + start + inner + "</" + sp.prefix + "spPr>" + graphic.mid(sp.end);
}

// Puts `raw` in the place of the children of `slot` (nothing for an empty `raw`).
void setSlot(SpPr *sp, int slot, const QByteArray &local, const QByteArray &raw)
{
    for (qsizetype i = sp->children.size() - 1; i >= 0; --i)
        if (sp->children[i].slot == slot) sp->children.removeAt(i);
    if (raw.isEmpty()) return;
    qsizetype at = 0;
    while (at < sp->children.size() && sp->children[at].slot <= slot) ++at;
    sp->children.insert(at, {slot, local, raw});
}

// A child as written, without the declarations of prefixes that reading it made whole with (so that two that say the same are equal).
QByteArray childRaw(const SpPr &sp, int slot)
{
    static const QRegularExpression declared(QStringLiteral(" xmlns(:[A-Za-z0-9]+)?=\"[^\"]*\""));
    for (const SpPr::Child &c : sp.children)
        if (c.slot == slot) return QString::fromUtf8(c.raw).remove(declared).toUtf8();
    return {};
}

// A color as it is kept: six hex digits, upper case; empty for anything else.
QByteArray cleanRgb(const QString &rgb)
{
    static const QRegularExpression hex(QStringLiteral("^[0-9A-Fa-f]{6}$"));
    return hex.match(rgb).hasMatch() ? rgb.toUpper().toLatin1() : QByteArray();
}

QByteArray solidFill(const QByteArray &rgb) { return "<a:solidFill><a:srgbClr val=\"" + rgb + "\"/></a:solidFill>"; }

const QStringList &dashNames()
{
    static const QStringList names = {QStringLiteral("solid"), QStringLiteral("dot"), QStringLiteral("sysDot"), QStringLiteral("dash"), QStringLiteral("dashDot"), QStringLiteral("lgDash"),
                                      QStringLiteral("lgDashDot"), QStringLiteral("lgDashDotDot")};
    return names;
}

// The line of a shape as it is kept: a:ln with the weight in EMU, the fill, and the dash; what the old line had besides those (its joins, its arrowheads) stays.
QByteArray lineElement(const QByteArray &old, const QByteArray &rgb, double weight, const QString &dash)
{
    if (rgb.isEmpty() || weight <= 0) return "<a:ln><a:noFill/></a:ln>";
    Attributes attrs;
    QByteArray tail;
    if (!old.isEmpty()) {
        attrs = attributesOf(old.left(old.indexOf('>') + 1));
        XmlTree t;
        if (t.parse("<x " + declarations() + ">" + old + "</x>", nullptr)) {
            const int ln = t.firstChild(t.root());
            for (int c = ln >= 0 ? t.firstChild(ln) : -1; c >= 0; c = t.next(c)) {
                const QByteArray name = t.localName(c);
                if (name != "noFill" && name != "solidFill" && name != "gradFill" && name != "pattFill" && name != "prstDash" && name != "custDash") tail += t.raw(c);
            }
        }
    }
    setAttribute(&attrs, "w", quote(QByteArray::number(qint64(std::llround(std::clamp(weight, 0.25, 100.0) * kEmuPerPoint)))));
    QByteArray dashed;
    const QString d = dashNames().contains(dash) && dash != QLatin1String("solid") ? dash : QString();
    if (!d.isEmpty()) dashed = "<a:prstDash val=\"" + d.toLatin1() + "\"/>";
    return "<a:ln" + written(attrs) + ">" + solidFill(rgb) + dashed + tail + "</a:ln>";
}

QByteArray geometryElement(const QString &preset) { return "<a:prstGeom prst=\"" + attribute(preset, 64) + "\"><a:avLst/></a:prstGeom>"; }

bool changeSpPrOf(WriteDoc &doc, int position, bool shapesOnly, const std::function<bool(SpPr *, const Model &)> &edit)
{
    return changePicture(doc, position, [&](Model &m) {
        const bool shape = m.graphic.contains("wsp") && !m.graphic.contains("blip");
        if (shapesOnly && !shape) return false;
        SpPr sp;
        if (!splitSpPr(m.graphic, &sp)) return false;
        if (!edit(&sp, m)) return false;
        m.graphic = joinSpPr(m.graphic, sp);
        return true;
    });
}

// The room inside a new shape, and its words.
QByteArray boxParagraphs(const QVector<TextBoxDesign::Paragraph> &paragraphs)
{
    QByteArray out;
    auto one = [&](const TextBoxDesign::Paragraph &p) {
        QByteArray x = "<w:p><w:pPr><w:spacing w:after=\"0\"/>";
        if (p.align == QLatin1String("center")) x += "<w:jc w:val=\"center\"/>";
        else if (p.align == QLatin1String("right")) x += "<w:jc w:val=\"right\"/>";
        x += "</w:pPr>";
        if (!p.text.isEmpty()) {
            x += "<w:r><w:rPr>";
            if (p.bold) x += "<w:b/>";
            if (p.italic) x += "<w:i/>";
            if (const QByteArray rgb = cleanRgb(p.color); !rgb.isEmpty()) x += "<w:color w:val=\"" + rgb + "\"/>";
            x += "<w:sz w:val=\"" + QByteArray::number(int(std::lround(std::clamp(p.size, 1.0, 400.0) * 2))) + "\"/></w:rPr><w:t xml:space=\"preserve\">" + attribute(p.text, 2000) + "</w:t></w:r>";
        }
        return x + "</w:p>";
    };
    for (const auto &p : paragraphs) out += one(p);
    if (out.isEmpty()) out = one({});
    return out;
}

QByteArray shapeGraphic(const QString &preset, qint64 cx, qint64 cy, const ShapeLook &look, const QByteArray &lineEnds, const TextBoxDesign *box)
{
    auto n = [](qint64 v) { return QByteArray::number(v); };
    QByteArray g = "<a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"><wps:wsp>";
    g += box ? "<wps:cNvSpPr txBox=\"1\"/>" : "<wps:cNvSpPr/>";
    g += "<wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + n(cx) + "\" cy=\"" + n(cy) + "\"/></a:xfrm>" + geometryElement(preset);
    const QByteArray fill = cleanRgb(look.fill), line = cleanRgb(look.line);
    g += fill.isEmpty() ? QByteArray("<a:noFill/>") : solidFill(fill);
    QByteArray ln = lineElement({}, line, look.weight, look.dash);
    if (!lineEnds.isEmpty() && ln.endsWith("</a:ln>")) ln = ln.left(ln.size() - 7) + lineEnds + "</a:ln>";
    g += ln + "</wps:spPr>";
    if (box) g += "<wps:txbx><w:txbxContent>" + boxParagraphs(box->paragraphs) + "</w:txbxContent></wps:txbx>";
    if (box) {
        auto emuIn = [&](double v) { return n(emuOf(std::clamp(v, 0.0, 500.0))); };
        g += "<wps:bodyPr rot=\"0\" vert=\"" + QByteArray(box->vertical ? "vert" : "horz") + "\" wrap=\"square\" lIns=\"" + emuIn(box->insets.left()) + "\" tIns=\"" + emuIn(box->insets.top()) + "\" rIns=\"" +
             emuIn(box->insets.right()) + "\" bIns=\"" + emuIn(box->insets.bottom()) + "\" anchor=\"" + (box->anchor == QLatin1String("ctr") || box->anchor == QLatin1String("b") ? box->anchor.toLatin1() : QByteArray("t")) +
             "\" anchorCtr=\"0\"><a:noAutofit/></wps:bodyPr>";
    } else {
        g += "<wps:bodyPr rot=\"0\" vert=\"horz\" wrap=\"square\" anchor=\"ctr\" anchorCtr=\"0\"><a:noAutofit/></wps:bodyPr>";
    }
    return g + "</wps:wsp></a:graphicData></a:graphic>";
}

// A shape or text box at the cursor from its graphic.
PictureResult insertShapeGraphic(WriteDoc &doc, QTextCursor &c, const QByteArray &graphic, double width, double height, PictureWrap wrap, const QString &namePrefix)
{
    if (c.document() != doc.story()) {
        doc.pictureNotice = QCoreApplication::translate("jp::write::Pictures", "A picture can't be put in a header, a footer, or a note yet.");
        return PictureResult::Refused;
    }
    Model m;
    m.floating = true;
    m.wrap = wrap == PictureWrap::Inline ? PictureWrap::Square : wrap;
    m.wrapRawOf = m.wrap;
    m.cx = emuOf(width);
    m.cy = emuOf(height);
    const int id = doc.nextDrawingId(doc.story());
    m.docPr = {{"id", quote(QByteArray::number(id))}, {"name", quote(attribute(namePrefix + QStringLiteral(" %1").arg(id), 255))}};
    m.distL = m.distR = 114300;
    if (m.wrap == PictureWrap::BehindText) m.relativeHeight = 251658240 - 1;
    m.frameProps = "<wp:cNvGraphicFramePr/>";
    m.declarations = {{"xmlns:wps", quote("http://schemas.microsoft.com/office/word/2010/wordprocessingShape")}, {"xmlns:w", quote("http://schemas.openxmlformats.org/wordprocessingml/2006/main")}};
    m.graphic = graphic;
    Inline atom;
    atom.kind = Inline::RunAtom;
    atom.tag = QStringLiteral("drawing");
    atom.raw = drawingXml(m);
    if (!insertDrawnAtom(doc, c, atom)) {
        doc.pictureNotice = QCoreApplication::translate("jp::write::Pictures", "A picture can't be put here, or this document holds as many pictures and drawn objects as JeffWrite can show.");
        return PictureResult::Refused;
    }
    return PictureResult::Ok;
}

// The words of a text box's content: its paragraphs, when it holds nothing but words.
struct BoxContent {
    qsizetype begin = -1, end = -1;   // the element's whole text in the graphic
    QByteArray inner;
    QStringList paragraphs;
    QByteArray pPr, rPr;              // the first paragraph's properties and the first run's, as written
    bool simple = false;
};

BoxContent boxContentOf(const QByteArray &graphic)
{
    BoxContent b;
    const qsizetype open = graphic.indexOf("<w:txbxContent");
    if (open < 0) return b;
    const qsizetype gt = graphic.indexOf('>', open);
    if (gt < 0) return b;
    if (graphic[gt - 1] == '/') {
        b.begin = open;
        b.end = gt + 1;
        b.simple = true;
        return b;
    }
    const qsizetype close = graphic.indexOf("</w:txbxContent>", gt);
    if (close < 0) return b;
    b.begin = open;
    b.end = close + 16;
    b.inner = graphic.mid(gt + 1, close - gt - 1);
    XmlTree t;
    if (!t.parse("<x " + declarations() + ">" + b.inner + "</x>", nullptr)) return b;
    b.simple = true;
    int count = 0;
    for (int p = t.firstChild(t.root()); p >= 0 && b.simple; p = t.next(p)) {
        if (!t.is(p, "p") || ++count > kMostBoxParagraphs) {
            b.simple = false;
            break;
        }
        QString text;
        bool firstPara = b.paragraphs.isEmpty();
        for (int r = t.firstChild(p); r >= 0 && b.simple; r = t.next(r)) {
            if (t.is(r, "pPr")) {
                // (A paragraph in a list or with a numbering is more than words.)
                if (t.child(r, "numPr") >= 0 || t.child(r, "sectPr") >= 0) b.simple = false;
                if (firstPara) b.pPr = t.raw(r);
            } else if (t.is(r, "r")) {
                for (int k = t.firstChild(r); k >= 0 && b.simple; k = t.next(k)) {
                    if (t.is(k, "rPr")) {
                        if (b.rPr.isEmpty()) b.rPr = t.raw(k);
                    } else if (t.is(k, "t")) {
                        text += t.text(k);
                    } else if (t.is(k, "tab")) {
                        text += QLatin1Char('\t');
                    } else if (t.is(k, "br")) {
                        text += QLatin1Char('\n');
                    } else {
                        b.simple = false;
                    }
                }
            } else {
                b.simple = false;
            }
        }
        b.paragraphs << text;
    }
    return b;
}

} // namespace

const QVector<ShapeGroup> &shapeGallery()
{
    static const QVector<ShapeGroup> groups = {
        {QStringLiteral("Lines"), {QStringLiteral("line"), QStringLiteral("straightConnector1")}},
        {QStringLiteral("Rectangles"), {QStringLiteral("rect"), QStringLiteral("roundRect"), QStringLiteral("snip1Rect"), QStringLiteral("snip2SameRect"), QStringLiteral("round1Rect"),
                                         QStringLiteral("round2SameRect")}},
        {QStringLiteral("Basic Shapes"), {QStringLiteral("ellipse"), QStringLiteral("triangle"), QStringLiteral("rtTriangle"), QStringLiteral("diamond"), QStringLiteral("parallelogram"),
                                          QStringLiteral("trapezoid"), QStringLiteral("pentagon"), QStringLiteral("hexagon"), QStringLiteral("octagon"), QStringLiteral("plus"),
                                          QStringLiteral("heart"), QStringLiteral("cloud"), QStringLiteral("moon"), QStringLiteral("sun"), QStringLiteral("smileyFace"),
                                          QStringLiteral("lightningBolt"), QStringLiteral("donut"), QStringLiteral("frame"), QStringLiteral("can"), QStringLiteral("cube"),
                                          QStringLiteral("bevel"), QStringLiteral("foldedCorner")}},
        {QStringLiteral("Block Arrows"), {QStringLiteral("rightArrow"), QStringLiteral("leftArrow"), QStringLiteral("upArrow"), QStringLiteral("downArrow"), QStringLiteral("leftRightArrow"),
                                          QStringLiteral("upDownArrow"), QStringLiteral("bentArrow"), QStringLiteral("uturnArrow"), QStringLiteral("chevron"), QStringLiteral("homePlate"),
                                          QStringLiteral("notchedRightArrow"), QStringLiteral("stripedRightArrow")}},
        {QStringLiteral("Equation Shapes"), {QStringLiteral("mathPlus"), QStringLiteral("mathMinus"), QStringLiteral("mathMultiply"), QStringLiteral("mathDivide"), QStringLiteral("mathEqual"),
                                             QStringLiteral("mathNotEqual")}},
        {QStringLiteral("Flowchart"), {QStringLiteral("flowChartProcess"), QStringLiteral("flowChartDecision"), QStringLiteral("flowChartTerminator"), QStringLiteral("flowChartDocument"),
                                       QStringLiteral("flowChartInputOutput"), QStringLiteral("flowChartPredefinedProcess"), QStringLiteral("flowChartConnector"),
                                       QStringLiteral("flowChartPreparation"), QStringLiteral("flowChartManualInput")}},
        {QStringLiteral("Stars and Banners"), {QStringLiteral("star4"), QStringLiteral("star5"), QStringLiteral("star6"), QStringLiteral("star8"), QStringLiteral("star10"),
                                               QStringLiteral("star12"), QStringLiteral("star16"), QStringLiteral("star24"), QStringLiteral("star32"), QStringLiteral("irregularSeal1"),
                                               QStringLiteral("irregularSeal2"), QStringLiteral("ribbon2"), QStringLiteral("ribbon"), QStringLiteral("wave"), QStringLiteral("doubleWave"),
                                               QStringLiteral("horizontalScroll"), QStringLiteral("verticalScroll")}},
        {QStringLiteral("Callouts"), {QStringLiteral("wedgeRectCallout"), QStringLiteral("wedgeRoundRectCallout"), QStringLiteral("wedgeEllipseCallout"), QStringLiteral("cloudCallout"),
                                      QStringLiteral("borderCallout1"), QStringLiteral("accentCallout1"), QStringLiteral("callout1")}}};
    return groups;
}

PictureResult insertShape(WriteDoc &doc, QTextCursor &c, const QString &preset, const QSizeF &size, const ShapeLook &look, PictureWrap wrap)
{
    bool known = false;
    for (const ShapeGroup &g : shapeGallery()) known = known || g.presets.contains(preset);
    if (!known) return PictureResult::Unreadable;
    const bool isLine = preset == QLatin1String("line") || preset == QLatin1String("straightConnector1");
    const bool square = QStringList{QStringLiteral("ellipse"), QStringLiteral("sun"), QStringLiteral("moon"), QStringLiteral("heart"), QStringLiteral("smileyFace"), QStringLiteral("donut"),
                                    QStringLiteral("plus"), QStringLiteral("mathPlus"), QStringLiteral("mathMinus"), QStringLiteral("mathMultiply"), QStringLiteral("mathDivide"),
                                    QStringLiteral("mathEqual"), QStringLiteral("mathNotEqual"), QStringLiteral("flowChartConnector"), QStringLiteral("cube"), QStringLiteral("frame")}
                            .contains(preset) || preset.startsWith(QLatin1String("star")) || preset.startsWith(QLatin1String("irregularSeal"));
    const QSizeF usual = isLine ? QSizeF(108, 0) : square ? QSizeF(86, 86) : QSizeF(108, 72);
    QSizeF made = size.width() <= 0 && size.height() <= 0 ? usual : size;
    // (Only a line may be flat; any other shape is as large as a picture's least.)
    made = QSizeF(std::clamp(made.width(), isLine ? 0.0 : kLeastPictureSize, kMostPictureSize), std::clamp(made.height(), isLine ? 0.0 : kLeastPictureSize, kMostPictureSize));
    if (isLine && made.width() <= 0 && made.height() <= 0) made = usual;
    // (A line is not filled; an arrow has its head at the end.)
    ShapeLook shown = look;
    if (isLine) shown.fill.clear();
    const QByteArray ends = preset == QLatin1String("straightConnector1") ? QByteArray("<a:tailEnd type=\"triangle\"/>") : QByteArray();
    const QByteArray graphic = shapeGraphic(preset, emuOf(made.width()), emuOf(made.height()), shown, ends, nullptr);
    return insertShapeGraphic(doc, c, graphic, made.width(), made.height(), wrap, isLine ? QStringLiteral("Line") : QStringLiteral("Shape"));
}

const QVector<TextBoxDesign> &textBoxDesigns()
{
    static const QVector<TextBoxDesign> designs = [] {
        QVector<TextBoxDesign> out;
        TextBoxDesign simple;
        simple.id = QStringLiteral("simple");
        simple.name = QStringLiteral("Simple Text Box");
        simple.size = QSizeF(216, 72);
        simple.look = {QStringLiteral("FFFFFF"), QStringLiteral("404040"), QStringLiteral("solid"), 0.75};
        simple.paragraphs = {{QString(), 11, false, false, QString(), QStringLiteral("left")}};
        out << simple;
        TextBoxDesign plain = simple;
        plain.id = QStringLiteral("plain");
        plain.name = QStringLiteral("Plain Text Box");
        plain.look = {QString(), QString(), QStringLiteral("solid"), 0};
        out << plain;
        TextBoxDesign sidebar;
        sidebar.id = QStringLiteral("sidebar");
        sidebar.name = QStringLiteral("Sidebar");
        sidebar.size = QSizeF(144, 504);
        sidebar.look = {QStringLiteral("E8EEF6"), QStringLiteral("3B78B8"), QStringLiteral("solid"), 1.5};
        sidebar.insets = QMarginsF(10, 10, 10, 10);
        sidebar.paragraphs = {{QStringLiteral("Sidebar title"), 14, true, false, QStringLiteral("27547F"), QStringLiteral("left")}, {QStringLiteral("Put a short note or a list of facts here."), 10.5, false, false, QString(), QStringLiteral("left")}};
        out << sidebar;
        TextBoxDesign quote;
        quote.id = QStringLiteral("quote");
        quote.name = QStringLiteral("Pull Quote");
        quote.size = QSizeF(216, 108);
        quote.look = {QString(), QString(), QStringLiteral("solid"), 0};
        quote.insets = QMarginsF(14, 10, 14, 10);
        quote.anchor = QStringLiteral("ctr");
        quote.paragraphs = {{QStringLiteral("A sentence from the text, set large so the eye finds it."), 16, false, true, QStringLiteral("27547F"), QStringLiteral("center")}};
        out << quote;
        TextBoxDesign band;
        band.id = QStringLiteral("band");
        band.name = QStringLiteral("Banner");
        band.size = QSizeF(432, 54);
        band.look = {QStringLiteral("27547F"), QString(), QStringLiteral("solid"), 0};
        band.insets = QMarginsF(14, 6, 14, 6);
        band.anchor = QStringLiteral("ctr");
        band.wrap = PictureWrap::TopAndBottom;
        band.paragraphs = {{QStringLiteral("A heading across the page"), 22, true, false, QStringLiteral("FFFFFF"), QStringLiteral("center")}};
        out << band;
        TextBoxDesign note;
        note.id = QStringLiteral("note");
        note.name = QStringLiteral("Note");
        note.size = QSizeF(180, 90);
        note.look = {QStringLiteral("FFF4C2"), QStringLiteral("C9A227"), QStringLiteral("solid"), 1};
        note.insets = QMarginsF(9, 7, 9, 7);
        note.paragraphs = {{QStringLiteral("Note"), 11, true, false, QString(), QStringLiteral("left")}, {QString(), 10.5, false, false, QString(), QStringLiteral("left")}};
        out << note;
        return out;
    }();
    return designs;
}

PictureResult insertTextBox(WriteDoc &doc, QTextCursor &c, const TextBoxDesign &design)
{
    const QSizeF size(std::clamp(design.size.width(), kLeastPictureSize, kMostPictureSize), std::clamp(design.size.height(), kLeastPictureSize, kMostPictureSize));
    const QByteArray graphic = shapeGraphic(QStringLiteral("rect"), emuOf(size.width()), emuOf(size.height()), design.look, {}, &design);
    return insertShapeGraphic(doc, c, graphic, size.width(), size.height(), design.wrap, QStringLiteral("Text Box"));
}


std::optional<QStringList> textBoxText(const WriteDoc &doc, int position)
{
    const QTextDocument *story = doc.story();
    if (!story || position < 0 || position + 1 > story->characterCount() - 1) return std::nullopt;
    Inline atom;
    if (!drawingAtomOf(formatAfter(story, position), &atom)) return std::nullopt;
    Model m;
    if (!parseDrawing(atom.raw, &m)) return std::nullopt;
    const BoxContent box = boxContentOf(m.graphic);
    if (box.begin < 0 || !box.simple) return std::nullopt;
    return box.paragraphs.isEmpty() ? QStringList{QString()} : box.paragraphs;
}

bool setTextBoxText(WriteDoc &doc, int position, const QStringList &paragraphs)
{
    if (paragraphs.size() > kMostBoxParagraphs) return false;
    qsizetype total = 0;
    for (const QString &p : paragraphs) total += p.size();
    if (total > kMostBoxTextLength) return false;
    return changePicture(doc, position, [&](Model &m) {
        const BoxContent box = boxContentOf(m.graphic);
        if (box.begin < 0 || !box.simple) return false;
        QStringList clean;
        for (const QString &p : paragraphs) clean << cleanText(p, int(kMostBoxTextLength));
        if (clean.isEmpty()) clean << QString();
        if (clean == (box.paragraphs.isEmpty() ? QStringList{QString()} : box.paragraphs)) return false;
        QByteArray content = "<w:txbxContent>";
        for (const QString &p : clean) {
            content += "<w:p>" + box.pPr;
            // (Tabs are tab characters of the run; the words between them are runs of the same look.)
            const QStringList parts = p.split(QLatin1Char('\t'));
            for (qsizetype i = 0; i < parts.size(); ++i) {
                QByteArray run = "<w:r>" + box.rPr;
                if (i > 0) run += "<w:tab/>";
                if (!parts[i].isEmpty()) {
                    QStringList lines = parts[i].split(QLatin1Char('\n'));
                    for (qsizetype k = 0; k < lines.size(); ++k) {
                        if (k > 0) run += "<w:br/>";
                        if (!lines[k].isEmpty()) run += "<w:t xml:space=\"preserve\">" + lines[k].toHtmlEscaped().toUtf8() + "</w:t>";
                    }
                }
                if (run != "<w:r>" + box.rPr) content += run + "</w:r>";
            }
            content += "</w:p>";
        }
        content += "</w:txbxContent>";
        m.graphic = m.graphic.left(box.begin) + content + m.graphic.mid(box.end);
        return true;
    });
}

bool setShapeFill(WriteDoc &doc, int position, const QString &rgb)
{
    const QByteArray color = cleanRgb(rgb);
    if (!rgb.isEmpty() && color.isEmpty()) return false;
    return changeSpPrOf(doc, position, true, [&](SpPr *sp, const Model &) {
        const QByteArray now = color.isEmpty() ? QByteArray("<a:noFill/>") : solidFill(color), before = childRaw(*sp, SlotFill);
        // (A shape that has no fill of its own and takes it from its style is a different thing from one that says it has none.)
        if (before == now) return false;
        setSlot(sp, SlotFill, color.isEmpty() ? "noFill" : "solidFill", now);
        return true;
    });
}

bool setShapeLine(WriteDoc &doc, int position, const QString &rgb, double weight, const QString &dash)
{
    if (!std::isfinite(weight)) return false;
    const QByteArray color = cleanRgb(rgb);
    if (!rgb.isEmpty() && color.isEmpty()) return false;
    return changeSpPrOf(doc, position, false, [&](SpPr *sp, const Model &) {
        const QByteArray before = childRaw(*sp, SlotLine);
        const QByteArray now = lineElement(before, color, weight, dash);
        if (before == now) return false;
        setSlot(sp, SlotLine, "ln", now);
        return true;
    });
}

bool setShapeRotation(WriteDoc &doc, int position, double degrees)
{
    if (!std::isfinite(degrees)) return false;
    double turn = std::fmod(degrees, 360.0);
    if (turn < 0) turn += 360;
    const qint64 rot = qint64(std::llround(turn * 60000.0)) % 21600000;
    return changeSpPrOf(doc, position, false, [&](SpPr *sp, const Model &m) {
        if (m.graphic.contains("chart")) return false;
        for (SpPr::Child &child : sp->children) {
            if (child.slot != SlotXfrm) continue;
            const qsizetype gt = child.raw.indexOf('>');
            if (gt < 0) return false;
            Attributes decls;
            Attributes attrs = attributesOf(child.raw.left(gt + 1), &decls);
            if (attributeValue(attrs, "rot").toLongLong() == rot) return false;
            if (rot == 0) removeAttribute(&attrs, "rot");
            else setAttribute(&attrs, "rot", quote(QByteArray::number(rot)));
            // The start tag again, with its name as it was written and the namespaces it declares itself.
            qsizetype nameEnd = 1;
            while (nameEnd < child.raw.size() && child.raw[nameEnd] != ' ' && child.raw[nameEnd] != '>' && child.raw[nameEnd] != '/') ++nameEnd;
            QByteArray start = "<" + child.raw.mid(1, nameEnd - 1);
            for (const auto &d : decls) start += " " + d.first + "=" + d.second;
            start += written(attrs) + (child.raw[gt - 1] == '/' ? "/>" : ">");
            child.raw = start + child.raw.mid(gt + 1);
            return true;
        }
        return false;
    });
}

bool setShapeGradient(WriteDoc &doc, int position, const QString &from, const QString &to, double angle)
{
    const QByteArray a = cleanRgb(from), b = cleanRgb(to);
    if (a.isEmpty() || b.isEmpty() || !std::isfinite(angle)) return false;
    double turn = std::fmod(angle, 360.0);
    if (turn < 0) turn += 360;
    const qint64 ang = qint64(std::llround(turn * 60000.0)) % 21600000;
    const QByteArray now = "<a:gradFill rotWithShape=\"1\"><a:gsLst><a:gs pos=\"0\"><a:srgbClr val=\"" + a + "\"/></a:gs><a:gs pos=\"100000\"><a:srgbClr val=\"" + b +
                           "\"/></a:gs></a:gsLst><a:lin ang=\"" + QByteArray::number(ang) + "\" scaled=\"0\"/></a:gradFill>";
    return changeSpPrOf(doc, position, true, [&](SpPr *sp, const Model &) {
        if (childRaw(*sp, SlotFill) == now) return false;
        setSlot(sp, SlotFill, "gradFill", now);
        return true;
    });
}

bool setPictureCropShape(WriteDoc &doc, int position, const QString &preset)
{
    bool known = preset == QLatin1String("rect");
    for (const ShapeGroup &g : shapeGallery()) known = known || (g.presets.contains(preset) && g.name != QLatin1String("Lines"));
    if (!known) return false;
    return changeSpPrOf(doc, position, false, [&](SpPr *sp, const Model &m) {
        if (!m.graphic.contains("blip") || m.graphic.contains("wsp")) return false;
        const QByteArray now = geometryElement(preset), before = childRaw(*sp, SlotGeometry);
        if (before == now) return false;
        setSlot(sp, SlotGeometry, "prstGeom", now);
        return true;
    });
}

bool alignObject(WriteDoc &doc, int position, ObjectAlign align, bool toPage)
{
    return changePicture(doc, position, [&](Model &m) {
        const bool across = align == ObjectAlign::Left || align == ObjectAlign::Center || align == ObjectAlign::Right;
        const char *word = align == ObjectAlign::Left ? "left" : align == ObjectAlign::Right ? "right" : align == ObjectAlign::Top ? "top" : align == ObjectAlign::Bottom ? "bottom" : "center";
        const QByteArray element = across ? "positionH" : "positionV";
        const QByteArray now = "<wp:" + element + " relativeFrom=\"" + (toPage ? "page" : "margin") + "\"><wp:align>" + word + "</wp:align></wp:" + element + ">";
        QByteArray &place = across ? m.posH : m.posV;
        const bool wasFloating = m.floating;
        // (What the file had is compared without the declarations of prefixes that reading it made whole with.)
        static const QRegularExpression declared(QStringLiteral(" xmlns(:[A-Za-z0-9]+)?=\"[^\"]*\""));
        if (wasFloating && QString::fromUtf8(place).remove(declared).toUtf8() == now) return false;
        if (!wasFloating) {
            m.floating = true;
            m.distL = m.distR = m.distT = m.distB = 0;
            m.wrap = PictureWrap::InFrontOfText;
            m.relativeHeight = 251658240;
        }
        place = now;
        return true;
    });
}

bool setShapePreset(WriteDoc &doc, int position, const QString &preset)
{
    bool known = false;
    for (const ShapeGroup &g : shapeGallery()) known = known || (g.presets.contains(preset) && g.name != QLatin1String("Lines"));
    if (!known) return false;
    return changeSpPrOf(doc, position, true, [&](SpPr *sp, const Model &) {
        const QByteArray now = geometryElement(preset), before = childRaw(*sp, SlotGeometry);
        if (before == now) return false;
        setSlot(sp, SlotGeometry, "prstGeom", now);
        return true;
    });
}


// ---- Picture adjustments

namespace {

// The start tag and the inside of the first a:blip of a graphic.
struct BlipSpot {
    qsizetype begin = -1, end = -1;   // the whole element, in the graphic
    QByteArray startTag, inner;
    bool selfClosing = false;
};

bool findBlip(const QByteArray &graphic, BlipSpot *out)
{
    for (qsizetype i = graphic.indexOf("<a:blip"); i >= 0; i = graphic.indexOf("<a:blip", i + 7)) {
        const char next = i + 7 < graphic.size() ? graphic[i + 7] : 0;
        if (next != ' ' && next != '>' && next != '/') continue;
        const qsizetype gt = graphic.indexOf('>', i);
        if (gt < 0) return false;
        BlipSpot b;
        b.begin = i;
        b.startTag = graphic.mid(i, gt + 1 - i);
        b.selfClosing = graphic[gt - 1] == '/';
        if (b.selfClosing) {
            b.end = gt + 1;
        } else {
            const qsizetype close = graphic.indexOf("</a:blip>", gt);
            if (close < 0) return false;
            b.inner = graphic.mid(gt + 1, close - gt - 1);
            b.end = close + 9;
        }
        *out = b;
        return true;
    }
    return false;
}

QByteArray withBlip(const QByteArray &graphic, const BlipSpot &b)
{
    const QByteArray start = b.selfClosing ? b.startTag.left(b.startTag.size() - 2) + ">" : b.startTag;
    if (b.inner.isEmpty()) return graphic.left(b.begin) + (b.selfClosing ? b.startTag : start + "</a:blip>") + graphic.mid(b.end);
    return graphic.left(b.begin) + start + b.inner + "</a:blip>" + graphic.mid(b.end);
}

const QRegularExpression &effectsPattern()
{
    static const QRegularExpression re(QStringLiteral("<a:(lum|grayscl|biLevel)\\b[^>]*/>|<a:(lum|grayscl|biLevel)\\b[^>]*></a:(lum|grayscl|biLevel)>"));
    return re;
}

QByteArray shareText(double v) { return QByteArray::number(qint64(std::llround(v * 100000.0))); }

// The cut a graphic has: its a:srcRect's shares (0 for an edge it leaves out).
void cutOf(const QByteArray &graphic, double cut[4])
{
    cut[0] = cut[1] = cut[2] = cut[3] = 0;
    static const QRegularExpression rect(QStringLiteral("<a:srcRect\\b([^>]*)/?>"));
    const QRegularExpressionMatch found = rect.match(QString::fromUtf8(graphic));
    if (!found.hasMatch()) return;
    static const char *names[4] = {"l", "t", "r", "b"};
    for (int i = 0; i < 4; ++i) {
        const QRegularExpressionMatch v = QRegularExpression(QStringLiteral("\\b%1=\"(-?[0-9]+)\"").arg(QLatin1String(names[i]))).match(found.captured(1));
        if (v.hasMatch()) cut[i] = std::clamp(v.captured(1).toDouble() / 100000.0, -1.0, 1.0);
    }
}

// Writes the cut into the graphic and sets the frame so that the picture keeps its scale.
bool applyCut(Model &m, const double now[4])
{
    double was[4];
    cutOf(m.graphic, was);
    const double keepWas[2] = {1 - was[0] - was[2], 1 - was[1] - was[3]}, keepNow[2] = {1 - now[0] - now[2], 1 - now[1] - now[3]};
    if (keepWas[0] < 0.02 || keepWas[1] < 0.02 || keepNow[0] < 0.02 || keepNow[1] < 0.02) return false;
    bool same = true;
    for (int i = 0; i < 4; ++i) same = same && std::abs(now[i] - was[i]) < 0.000005;
    if (same) return false;
    QByteArray rect;
    static const char *names[4] = {"l", "t", "r", "b"};
    bool any = false;
    for (int i = 0; i < 4; ++i)
        if (std::abs(now[i]) >= 0.000005) {
            rect += QByteArray(" ") + names[i] + "=\"" + shareText(now[i]) + "\"";
            any = true;
        }
    const QByteArray element = any ? "<a:srcRect" + rect + "/>" : QByteArray();
    static const QRegularExpression existing(QStringLiteral("<a:srcRect\\b[^>]*/>|<a:srcRect\\b[^>]*></a:srcRect>"));
    QString text = QString::fromUtf8(m.graphic);
    const QRegularExpressionMatch found = existing.match(text);
    if (found.hasMatch()) {
        text.replace(found.capturedStart(), found.capturedLength(), QString::fromUtf8(element));
        m.graphic = text.toUtf8();
    } else if (any) {
        BlipSpot b;
        if (!findBlip(m.graphic, &b)) return false;
        m.graphic = m.graphic.left(b.end) + element + m.graphic.mid(b.end);
    }
    const double cx = double(m.cx) * keepNow[0] / keepWas[0], cy = double(m.cy) * keepNow[1] / keepWas[1];
    const qint64 w = std::clamp<qint64>(qint64(std::llround(cx)), qint64(kLeastPictureSize * kEmuPerPoint), qint64(kMostPictureSize * kEmuPerPoint));
    const qint64 h = std::clamp<qint64>(qint64(std::llround(cy)), qint64(kLeastPictureSize * kEmuPerPoint), qint64(kMostPictureSize * kEmuPerPoint));
    m.graphic = graphicWithSize(m.graphic, w, h);
    m.cx = w;
    m.cy = h;
    return true;
}

// The color changes of the blip: brightness and contrast (as a:lum, nothing when both are 0), then gray or black and white.
bool applyColors(Model &m, double brightness, double contrast, PictureRecolor recolor)
{
    BlipSpot b;
    if (!findBlip(m.graphic, &b)) return false;
    QByteArray effects;
    if (std::abs(brightness) >= 0.000005 || std::abs(contrast) >= 0.000005) {
        effects += "<a:lum";
        if (std::abs(brightness) >= 0.000005) effects += " bright=\"" + shareText(brightness) + "\"";
        if (std::abs(contrast) >= 0.000005) effects += " contrast=\"" + shareText(contrast) + "\"";
        effects += "/>";
    }
    if (recolor == PictureRecolor::Grayscale) effects += "<a:grayscl/>";
    else if (recolor == PictureRecolor::BlackAndWhite) effects += "<a:biLevel thresh=\"50000\"/>";
    QString inner = QString::fromUtf8(b.inner);
    inner.remove(effectsPattern());
    const QByteArray without = inner.toUtf8();
    // (The effects come first: an extension list, if the blip has one, is last.)
    const QByteArray made = effects + without;
    if (made == b.inner) return false;
    b.inner = made;
    if (b.inner.isEmpty() && !b.selfClosing) b.selfClosing = true, b.startTag = b.startTag.left(b.startTag.size() - 1) + "/>";
    else if (!b.inner.isEmpty() && b.selfClosing) b.selfClosing = false, b.startTag = b.startTag.left(b.startTag.size() - 2) + ">";
    m.graphic = withBlip(m.graphic, b);
    return true;
}

// What a picture's graphic says of its colors now.
struct ColorsNow {
    double brightness = 0, contrast = 0;
    PictureRecolor recolor = PictureRecolor::None;
};

ColorsNow colorsOf(const QByteArray &graphic)
{
    ColorsNow now;
    static const QRegularExpression lum(QStringLiteral("<a:lum\\b([^>]*)/?>"));
    const QRegularExpressionMatch found = lum.match(QString::fromUtf8(graphic));
    if (found.hasMatch()) {
        for (const char *name : {"bright", "contrast"}) {
            const QRegularExpressionMatch v = QRegularExpression(QStringLiteral("\\b%1=\"(-?[0-9]+)\"").arg(QLatin1String(name))).match(found.captured(1));
            if (v.hasMatch()) (QLatin1String(name) == QLatin1String("bright") ? now.brightness : now.contrast) = std::clamp(v.captured(1).toDouble() / 100000.0, -1.0, 1.0);
        }
    }
    if (graphic.contains("<a:grayscl")) now.recolor = PictureRecolor::Grayscale;
    else if (graphic.contains("<a:biLevel")) now.recolor = PictureRecolor::BlackAndWhite;
    return now;
}

} // namespace

bool setPictureCrop(WriteDoc &doc, int position, double left, double top, double right, double bottom)
{
    const double now[4] = {left, top, right, bottom};
    for (double v : now)
        if (!std::isfinite(v) || v < -1 || v > 0.98) return false;
    if (left + right > 0.98 || top + bottom > 0.98) return false;
    return changePicture(doc, position, [&](Model &m) {
        if (!m.graphic.contains("blip")) return false;
        return applyCut(m, now);
    });
}

bool setPictureCorrections(WriteDoc &doc, int position, double brightness, double contrast)
{
    if (!std::isfinite(brightness) || !std::isfinite(contrast)) return false;
    brightness = std::clamp(brightness, -1.0, 1.0);
    contrast = std::clamp(contrast, -1.0, 1.0);
    return changePicture(doc, position, [&](Model &m) { return applyColors(m, brightness, contrast, colorsOf(m.graphic).recolor); });
}

bool setPictureRecolor(WriteDoc &doc, int position, PictureRecolor recolor)
{
    return changePicture(doc, position, [&](Model &m) {
        const ColorsNow now = colorsOf(m.graphic);
        return applyColors(m, now.brightness, now.contrast, recolor);
    });
}

bool resetPicture(WriteDoc &doc, int position)
{
    return changePicture(doc, position, [&](Model &m) {
        if (!m.graphic.contains("blip")) return false;
        bool changed = applyCut(m, std::array<double, 4>{0, 0, 0, 0}.data());
        changed = applyColors(m, 0, 0, PictureRecolor::None) || changed;
        SpPr sp;
        if (splitSpPr(m.graphic, &sp) && !childRaw(sp, SlotLine).isEmpty()) {
            setSlot(&sp, SlotLine, "ln", QByteArray());
            m.graphic = joinSpPr(m.graphic, sp);
            changed = true;
        }
        return changed;
    });
}

PictureResult changePictureFile(WriteDoc &doc, int position, const QByteArray &bytes)
{
    Prepared p;
    if (const PictureResult r = prepare(bytes, &p); r != PictureResult::Ok) return r;
    QTextDocument *story = doc.story();
    if (!isPictureAt(story, position) || position + 1 > story->characterCount() - 1) return PictureResult::Refused;
    bool added = false;
    const QString rid = doc.addMedia(p.bytes, p.extension, p.contentType, &added);
    if (added && doc.mediaBytes() > doc.mediaLimit) {
        doc.removeMedia(rid);
        doc.pictureNotice = QCoreApplication::translate("jp::write::Pictures", "The pictures in this document are as large together as JeffWrite puts in one, so this one is not added.");
        return PictureResult::TooLarge;
    }
    const bool changed = changePicture(doc, position, [&](Model &m) {
        BlipSpot b;
        if (!findBlip(m.graphic, &b)) return false;
        double none[4] = {0, 0, 0, 0};
        applyCut(m, none);
        if (!findBlip(m.graphic, &b)) return false;
        Attributes attrs = attributesOf(b.startTag);
        setAttribute(&attrs, "r:embed", quote(rid.toUtf8()));
        const qsizetype nameEnd = b.startTag.indexOf(' ') > 0 ? b.startTag.indexOf(' ') : b.startTag.size() - 1;
        QByteArray start = b.startTag.left(nameEnd);
        Attributes declared;
        attributesOf(b.startTag, &declared);
        for (const auto &d : declared) start += " " + d.first + "=" + d.second;
        b.startTag = start + written(attrs) + (b.selfClosing ? "/>" : ">");
        m.graphic = withBlip(m.graphic, b);
        // The colors of the old picture are not the new one's. The frame keeps its width and takes the new picture's proportions.
        applyColors(m, 0, 0, PictureRecolor::None);
        const qint64 h = std::clamp<qint64>(qint64(std::llround(double(m.cx) * p.size.height() / std::max(1.0, p.size.width()))), qint64(kLeastPictureSize * kEmuPerPoint), qint64(kMostPictureSize * kEmuPerPoint));
        m.graphic = graphicWithSize(m.graphic, m.cx, h);
        m.cy = h;
        return true;
    });
    if (!changed) {
        if (added) doc.removeMedia(rid);
        return PictureResult::Refused;
    }
    return PictureResult::Ok;
}

// ---- Charts

namespace {

using office::Chart;
using office::ChartSeries;

const char kChartContentType[] = "application/vnd.openxmlformats-officedocument.drawingml.chart+xml";
const char kChartRelationship[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart";

office::sheet::ChartKind kindOfForm(ChartForm form)
{
    using K = office::sheet::ChartKind;
    switch (form) {
    case ChartForm::Column: return K::Column;
    case ChartForm::ColumnStacked: return K::ColumnStacked;
    case ChartForm::ColumnPercent: return K::ColumnPercent;
    case ChartForm::Bar: return K::Bar;
    case ChartForm::BarStacked: return K::BarStacked;
    case ChartForm::Line: return K::Line;
    case ChartForm::LineMarkers: return K::LineMarkers;
    case ChartForm::Pie: return K::Pie;
    case ChartForm::Doughnut: return K::Doughnut;
    case ChartForm::Area: return K::Area;
    case ChartForm::AreaStacked: return K::AreaStacked;
    case ChartForm::Scatter: return K::Scatter;
    }
    return K::Column;
}

ChartForm formOfKind(office::sheet::ChartKind kind)
{
    using K = office::sheet::ChartKind;
    switch (kind) {
    case K::ColumnStacked: return ChartForm::ColumnStacked;
    case K::ColumnPercent: return ChartForm::ColumnPercent;
    case K::Bar:
    case K::BarPercent: return ChartForm::Bar;
    case K::BarStacked: return ChartForm::BarStacked;
    case K::Line:
    case K::LineStacked: return ChartForm::Line;
    case K::LineMarkers: return ChartForm::LineMarkers;
    case K::Pie: return ChartForm::Pie;
    case K::Doughnut: return ChartForm::Doughnut;
    case K::Area: return ChartForm::Area;
    case K::AreaStacked: return ChartForm::AreaStacked;
    case K::Scatter:
    case K::ScatterLines:
    case K::ScatterSmooth: return ChartForm::Scatter;
    default: return ChartForm::Column;
    }
}

// The series of a chart from the data: each its name, the categories, and its numbers. A scatter's categories are its x numbers (or 1, 2, 3 for words).
QVector<ChartSeries> seriesOfData(const ChartData &data, bool scatter)
{
    QVector<ChartSeries> out;
    QVector<double> xs;
    if (scatter) {
        for (int i = 0; i < data.categories.size(); ++i) {
            bool ok = false;
            const double v = data.categories[i].toDouble(&ok);
            xs << (ok && std::isfinite(v) ? v : double(i + 1));
        }
    }
    for (int s = 0; s < data.values.size(); ++s) {
        ChartSeries series;
        series.name = data.seriesNames.value(s, QCoreApplication::translate("jp::write::Pictures", "Series %1").arg(s + 1));
        series.categories = data.categories;
        series.values = data.values[s];
        series.xValues = xs;
        out << series;
    }
    return out;
}

bool dataIsUsable(const ChartData &data)
{
    if (data.categories.isEmpty() || data.values.isEmpty() || data.categories.size() > kMostChartCategories || data.values.size() > kMostChartSeries) return false;
    for (const QVector<double> &v : data.values) {
        if (v.size() != data.categories.size()) return false;
        for (double x : v)
            if (!std::isfinite(x) || std::abs(x) > 1e15) return false;
    }
    return true;
}

const office::Theme &chartTheme(const WriteDoc &doc)
{
    static const office::Theme standard = office::defaultTheme();
    return doc.theme() ? *doc.theme() : standard;
}

// Every series of the chart, plot by plot.
QVector<ChartSeries> allSeries(const Chart &chart)
{
    QVector<ChartSeries> out;
    for (const office::ChartPlot &p : chart.plots) out << p.series;
    return out;
}

// The axis of the chart that is its category (or x) axis or its value axis, or null (a chart with no axes, a pie).
office::ChartAxis *chartAxisOf(Chart &chart, ChartAxisKind which)
{
    office::ChartAxis *category = nullptr;
    for (office::ChartAxis &a : chart.axes)
        if (!category && (a.kind == office::ChartAxis::Category || a.kind == office::ChartAxis::Date)) category = &a;
    for (office::ChartAxis &a : chart.axes)
        if (!category && (a.position == QLatin1String("b") || a.position == QLatin1String("t"))) category = &a;   // (a scatter's x axis)
    if (which == ChartAxisKind::Category) return category;
    for (office::ChartAxis &a : chart.axes)
        if (&a != category && a.kind == office::ChartAxis::Value) return &a;
    return nullptr;
}

// The part a chart drawing names: the Id in its c:chart, and the bytes.
bool chartPartOf(const WriteDoc &doc, const QByteArray &graphic, QString *id, QByteArray *bytes)
{
    const qsizetype at = graphic.indexOf("<c:chart");
    if (at < 0) return false;
    static const QRegularExpression rid(QStringLiteral("r:id=\"([^\"]+)\""));
    const QRegularExpressionMatch m = rid.match(QString::fromUtf8(graphic.mid(at, 600)));
    if (!m.hasMatch()) return false;
    *id = m.captured(1);
    *bytes = doc.relatedPartData(*id);
    return !bytes->isEmpty();
}

// Changes the chart the drawing at `position` names: the part is read, `edit` changes the model, and the part is written again as a new one (so that Undo has the old one).
bool editChart(WriteDoc &doc, int position, const std::function<bool(Chart &)> &edit)
{
    return changePicture(doc, position, [&](Model &m) {
        QString id;
        QByteArray old;
        if (!chartPartOf(doc, m.graphic, &id, &old)) return false;
        Chart chart;
        if (!office::readChart(old, chartTheme(doc), office::ColorMap(), &chart)) return false;
        if (!office::chartIsFullyModeled(old)) {
            doc.pictureNotice = QCoreApplication::translate("jp::write::Pictures", "This chart has parts that JeffWrite does not change, so it is kept as it is.");
            return false;
        }
        if (!edit(chart)) return false;
        const QByteArray now = office::writeChartPart(chart);
        if (now.isEmpty() || now == old) return false;
        QString oldTarget;
        for (const office::Relationship &r : doc.mainRelationshipsForTest())
            if (r.id == id) oldTarget = r.target;
        const QString made = doc.addRelatedPart(QStringLiteral("/word/charts"), QStringLiteral("chart"), QStringLiteral("xml"), now, QString::fromLatin1(kChartContentType), QString::fromLatin1(kChartRelationship), oldTarget);
        m.graphic.replace("r:id=\"" + id.toUtf8() + "\"", "r:id=\"" + made.toUtf8() + "\"");
        return true;
    });
}

} // namespace

const QVector<ChartForm> &chartForms()
{
    static const QVector<ChartForm> forms = {ChartForm::Column, ChartForm::ColumnStacked, ChartForm::ColumnPercent, ChartForm::Bar, ChartForm::BarStacked, ChartForm::Line, ChartForm::LineMarkers,
                                             ChartForm::Pie, ChartForm::Doughnut, ChartForm::Area, ChartForm::AreaStacked, ChartForm::Scatter};
    return forms;
}

ChartData sampleChartData()
{
    ChartData d;
    d.categories = {QCoreApplication::translate("jp::write::Pictures", "Category 1"), QCoreApplication::translate("jp::write::Pictures", "Category 2"),
                    QCoreApplication::translate("jp::write::Pictures", "Category 3")};
    d.seriesNames = {QCoreApplication::translate("jp::write::Pictures", "Series 1"), QCoreApplication::translate("jp::write::Pictures", "Series 2")};
    d.values = {{4.3, 2.5, 3.5}, {2.4, 4.4, 1.8}};
    return d;
}

ChartInfo chartAt(const WriteDoc &doc, int position)
{
    ChartInfo info;
    const QTextDocument *story = doc.story();
    if (!story || position < 0 || position + 1 > story->characterCount() - 1) return info;
    Inline atom;
    if (!drawingAtomOf(formatAfter(story, position), &atom)) return info;
    Model m;
    if (!parseDrawing(atom.raw, &m)) return info;
    QString id;
    QByteArray bytes;
    if (!chartPartOf(doc, m.graphic, &id, &bytes)) return info;
    Chart chart;
    if (!office::readChart(bytes, chartTheme(doc), office::ColorMap(), &chart)) return info;
    info.valid = true;
    info.form = formOfKind(office::sheet::kindOfChart(chart));
    info.titleShown = chart.title.present;
    info.title = chart.title.text;
    info.legendShown = chart.legend.shown;
    info.legendPosition = chart.legend.position;
    info.style = chart.style;
    if (office::ChartAxis *a = chartAxisOf(chart, ChartAxisKind::Category)) {
        info.hasAxes = true;
        info.categoryAxisShown = !a->deleted;
        info.categoryGridlines = a->majorGridlines;
        info.categoryAxisTitle = a->title.present ? a->title.text : QString();
    }
    if (office::ChartAxis *a = chartAxisOf(chart, ChartAxisKind::Value)) {
        info.hasAxes = true;
        info.valueAxisShown = !a->deleted;
        info.valueGridlines = a->majorGridlines;
        info.valueMinorGridlines = a->minorGridlines;
        info.valueAxisTitle = a->title.present ? a->title.text : QString();
    }
    const QVector<ChartSeries> series = allSeries(chart);
    info.dataEditable = !series.isEmpty() && office::chartIsFullyModeled(bytes);
    for (const ChartSeries &s : series) {
        info.dataLabels = info.dataLabels || s.labels.shown();
        if (!s.valueRef.isEmpty() || !s.categoryRef.isEmpty() || !s.nameRef.isEmpty()) info.dataEditable = false;
    }
    if (!series.isEmpty()) {
        info.data.categories = series.first().categories;
        // (A scatter's x numbers are its categories.)
        if (info.data.categories.isEmpty())
            for (double x : series.first().xValues) info.data.categories << QString::number(x, 'g', 15);
    }
    for (const ChartSeries &s : series) {
        info.data.seriesNames << s.name;
        info.data.values << s.values;
    }
    return info;
}

PictureResult insertChart(WriteDoc &doc, QTextCursor &c, ChartForm form, const ChartData &data, const QSizeF &size)
{
    if (c.document() != doc.story()) {
        doc.pictureNotice = QCoreApplication::translate("jp::write::Pictures", "A picture can't be put in a header, a footer, or a note yet.");
        return PictureResult::Refused;
    }
    if (!dataIsUsable(data)) return PictureResult::Unreadable;
    const office::sheet::ChartKind kind = kindOfForm(form);
    const QVector<ChartSeries> series = seriesOfData(data, form == ChartForm::Scatter);
    const QVector<office::sheet::SeriesKind> kinds(series.size(), office::sheet::SeriesKind{kind, false});
    Chart chart = office::sheet::chartFromSeries(series, kinds, chartTheme(doc));
    const QByteArray part = office::writeChartPart(chart);
    if (part.isEmpty()) return PictureResult::Refused;
    const QSizeF made = size.isEmpty() ? QSizeF(360, 216) : QSizeF(std::clamp(size.width(), kLeastPictureSize, kMostPictureSize), std::clamp(size.height(), kLeastPictureSize, kMostPictureSize));
    const QString rid = doc.addRelatedPart(QStringLiteral("/word/charts"), QStringLiteral("chart"), QStringLiteral("xml"), part, QString::fromLatin1(kChartContentType), QString::fromLatin1(kChartRelationship));
    Model m;
    m.floating = false;
    m.wrap = PictureWrap::Inline;
    m.cx = emuOf(made.width());
    m.cy = emuOf(made.height());
    const int id = doc.nextDrawingId(doc.story());
    m.docPr = {{"id", quote(QByteArray::number(id))}, {"name", quote(QByteArray("Chart ") + QByteArray::number(id))}};
    m.frameProps = "<wp:cNvGraphicFramePr/>";
    m.graphic = "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"><c:chart xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" r:id=\"" + rid.toUtf8() +
                "\"/></a:graphicData></a:graphic>";
    Inline atom;
    atom.kind = Inline::RunAtom;
    atom.tag = QStringLiteral("drawing");
    atom.raw = drawingXml(m);
    if (!insertDrawnAtom(doc, c, atom)) {
        doc.removeMedia(rid);
        doc.pictureNotice = QCoreApplication::translate("jp::write::Pictures", "A picture can't be put here, or this document holds as many pictures and drawn objects as JeffWrite can show.");
        return PictureResult::Refused;
    }
    return PictureResult::Ok;
}

bool setChartForm(WriteDoc &doc, int position, ChartForm form)
{
    return editChart(doc, position, [&](Chart &chart) {
        if (formOfKind(office::sheet::kindOfChart(chart)) == form && chart.plots.size() == 1) return false;
        QVector<ChartSeries> series = allSeries(chart);
        if (series.isEmpty()) return false;
        // (A scatter's x numbers are the categories of a chart of another kind.)
        for (ChartSeries &s : series)
            if (s.categories.isEmpty())
                for (double x : s.xValues) s.categories << QString::number(x, 'g', 15);
        // (A scatter's points are placed by x numbers: the categories, read as numbers, or 1, 2, 3.)
        if (form == ChartForm::Scatter)
            for (ChartSeries &s : series)
                if (s.xValues.isEmpty())
                    for (int i = 0; i < s.values.size(); ++i) {
                        bool ok = false;
                        const double v = s.categories.value(i).toDouble(&ok);
                        s.xValues << (ok && std::isfinite(v) ? v : double(i + 1));
                    }
        const QVector<office::sheet::SeriesKind> kinds(series.size(), office::sheet::SeriesKind{kindOfForm(form), false});
        const Chart like = chart;
        chart = office::sheet::chartFromSeries(series, kinds, chartTheme(doc), &like);
        return true;
    });
}

bool setChartTitle(WriteDoc &doc, int position, bool shown, const QString &text)
{
    const QString clean = cleanText(text, 500);
    return editChart(doc, position, [&](Chart &chart) {
        const bool same = chart.title.present == shown && (!shown || (chart.title.text == clean && !chart.title.automatic));
        if (same) return false;
        chart.title.present = shown;
        if (shown) {
            chart.title.text = clean.isEmpty() ? QCoreApplication::translate("jp::write::Pictures", "Chart Title") : clean;
            chart.title.automatic = false;
            chart.title.ref.clear();
        }
        return true;
    });
}

bool setChartLegend(WriteDoc &doc, int position, const QString &legendPosition)
{
    if (!legendPosition.isEmpty() && !QStringList{QStringLiteral("r"), QStringLiteral("l"), QStringLiteral("t"), QStringLiteral("b")}.contains(legendPosition)) return false;
    return editChart(doc, position, [&](Chart &chart) {
        const bool shown = !legendPosition.isEmpty();
        if (chart.legend.shown == shown && (!shown || chart.legend.position == legendPosition)) return false;
        chart.legend.shown = shown;
        if (shown) {
            chart.legend.position = legendPosition;
            chart.legend.manual = false;
        }
        return true;
    });
}

bool setChartDataLabels(WriteDoc &doc, int position, bool shown)
{
    return editChart(doc, position, [&](Chart &chart) {
        bool now = false;
        for (const office::ChartPlot &p : chart.plots)
            for (const ChartSeries &s : p.series) now = now || s.labels.shown();
        if (now == shown) return false;
        for (office::ChartPlot &p : chart.plots)
            for (ChartSeries &s : p.series) {
                s.labels.value = shown;
                if (!shown) s.labels.category = s.labels.series = s.labels.percent = s.labels.legendKey = false;
            }
        return true;
    });
}

bool setChartAxis(WriteDoc &doc, int position, ChartAxisKind which, bool shown)
{
    return editChart(doc, position, [&](Chart &chart) {
        office::ChartAxis *a = chartAxisOf(chart, which);
        if (!a || a->deleted == !shown) return false;
        a->deleted = !shown;
        return true;
    });
}

bool setChartGridlines(WriteDoc &doc, int position, ChartAxisKind which, bool major, bool minor)
{
    return editChart(doc, position, [&](Chart &chart) {
        office::ChartAxis *a = chartAxisOf(chart, which);
        if (!a || (a->majorGridlines == major && a->minorGridlines == minor)) return false;
        a->majorGridlines = major;
        a->minorGridlines = minor;
        return true;
    });
}

bool setChartAxisTitle(WriteDoc &doc, int position, ChartAxisKind which, const QString &text)
{
    const QString clean = cleanText(text, 500);
    return editChart(doc, position, [&](Chart &chart) {
        office::ChartAxis *a = chartAxisOf(chart, which);
        if (!a || (a->title.present == !clean.isEmpty() && a->title.text == clean)) return false;
        a->title.present = !clean.isEmpty();
        a->title.text = clean;
        return true;
    });
}

bool setChartStyle(WriteDoc &doc, int position, int style)
{
    if (style < 1 || style > 48) return false;
    return editChart(doc, position, [&](Chart &chart) {
        if (chart.style == style) return false;
        chart.style = style;
        return true;
    });
}

bool setChartData(WriteDoc &doc, int position, const ChartData &data)
{
    if (!dataIsUsable(data)) return false;
    return editChart(doc, position, [&](Chart &chart) {
        const QVector<ChartSeries> old = allSeries(chart);
        if (old.isEmpty()) return false;
        for (const ChartSeries &s : old)
            if (!s.valueRef.isEmpty() || !s.categoryRef.isEmpty() || !s.nameRef.isEmpty()) return false;
        const office::sheet::ChartKind kind = office::sheet::kindOfChart(chart);
        QVector<ChartSeries> series = seriesOfData(data, kind == office::sheet::ChartKind::Scatter || kind == office::sheet::ChartKind::ScatterLines || kind == office::sheet::ChartKind::ScatterSmooth);
        // The look of the series that were there stays with them (their colors, lines, and markers).
        for (int i = 0; i < series.size() && i < old.size(); ++i) {
            ChartSeries made = old[i];
            made.name = series[i].name;
            made.categories = series[i].categories;
            made.values = series[i].values;
            made.xValues = series[i].xValues;
            made.categoryNumbers.clear();
            made.categoryRefNumeric = false;
            series[i] = made;
        }
        QVector<office::sheet::SeriesKind> kinds;
        for (const office::ChartPlot &p : chart.plots)
            for (int i = 0; i < p.series.size(); ++i) kinds << office::sheet::SeriesKind{office::sheet::kindOfPlot(p), false};
        while (kinds.size() < series.size()) kinds << office::sheet::SeriesKind{kinds.isEmpty() ? office::sheet::ChartKind::Column : kinds.last().kind, false};
        kinds.resize(series.size());
        const Chart like = chart;
        chart = office::sheet::chartFromSeries(series, kinds, chartTheme(doc), &like);
        return true;
    });
}

PictureResult readPictureFile(const QString &path, QByteArray *bytes)
{
    // Only a regular file (a link to one too): a pipe would wait for a writer, and a device or a link to one has no end.
    if (!QFileInfo(path).isFile()) return PictureResult::Unreadable;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return PictureResult::Unreadable;
    const QByteArray data = f.read(kMostPictureBytes + 1);
    if (qint64(data.size()) > kMostPictureBytes) return PictureResult::TooLarge;
    *bytes = data;
    return PictureResult::Ok;
}

void clampKeepingRatio(double *width, double *height)
{
    if (!std::isfinite(*width) || *width <= 0) *width = kLeastPictureSize;
    if (!std::isfinite(*height) || *height <= 0) *height = kLeastPictureSize;
    const double big = std::max(*width, *height), small = std::min(*width, *height);
    double k = big > kMostPictureSize ? kMostPictureSize / big : 1;
    if (small * k < kLeastPictureSize) k = kLeastPictureSize / small;
    *width = std::clamp(*width * k, kLeastPictureSize, kMostPictureSize);
    *height = std::clamp(*height * k, kLeastPictureSize, kMostPictureSize);
}

QByteArray withDrawingId(const QByteArray &raw, int id)
{
    const qsizetype at = raw.indexOf("docPr");
    if (at < 0) return raw;
    const qsizetype key = raw.indexOf(" id=\"", at);
    if (key < 0 || key - at > 400) return raw;
    const qsizetype end = raw.indexOf('"', key + 5);
    if (end < 0) return raw;
    return raw.left(key + 5) + QByteArray::number(id) + raw.mid(end);
}

QString cleanAltText(const QString &text) { return cleanText(text, kMostAltTextLength); }

} // namespace jp::write
