#include "core/pictureops.h"

#include "core/editops.h"
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
    if (out.graphic.isEmpty() || !out.graphic.contains("blip") || out.cx <= 0 || out.cy <= 0) return false;
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

// Whether bytes are an SVG file (by what they begin with, whatever plugin is there to read them).
bool looksLikeSvg(const QByteArray &bytes)
{
    if (bytes.startsWith("\x1f\x8b")) return false;
    return bytes.left(4096).toLower().contains("<svg");
}

// Whether an SVG refers to anything outside itself: a file, an address, a style sheet, or a definition of its own that a reader might load. What it holds of its own (a link to an
// element of it, or an image carried in it as data) is its own.
bool svgReachesOutside(const QByteArray &svg)
{
    const QString text = QString::fromUtf8(svg);
    static const QRegularExpression doctype(QStringLiteral("<!\\s*(DOCTYPE|ENTITY)|<\\?xml-stylesheet|@import"), QRegularExpression::CaseInsensitiveOption);
    if (doctype.match(text).hasMatch()) return true;
    static const QRegularExpression href(QStringLiteral("(?:^|[\\s:])href\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)')"), QRegularExpression::CaseInsensitiveOption);
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
        if (attributeValue(m.docPr, "descr") == text) return false;
        if (text.isEmpty()) removeAttribute(&m.docPr, "descr");
        else setAttribute(&m.docPr, "descr", quote(text));
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

QString cleanAltText(const QString &text) { return cleanText(text, kMostAltTextLength); }

} // namespace jp::write
