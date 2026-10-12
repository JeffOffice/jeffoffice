#include "io/pptxdiagram.h"

#include "office/xml.h"
#include "render/renderer.h"
#include "render/svgexport.h"

#include <QBuffer>
#include <QHash>
#include <QImage>
#include <QImageReader>
#include <QRegularExpression>
#include <memory>

namespace jp::deck {

namespace {

QDomElement aChild(const QDomElement &e, const char *name) { return office::child(e, office::ns::A, QLatin1String(name)); }

// Where the data part names the drawing's link: the first dsp:dataModelExt with a relId. The part may be large, so this reads the bytes in place.
bool findDrawingLinkId(const QByteArray &xml, qsizetype *valueAt, qsizetype *valueEnd)
{
    static const QByteArray tag("dataModelExt");
    qsizetype from = 0;
    // A part that has one at all has it in its extension list; a few are looked at, not every one.
    for (int tries = 0; tries < 16; ++tries) {
        const qsizetype at = xml.indexOf(tag, from);
        if (at < 0) return false;
        const qsizetype close = xml.indexOf('>', at);
        if (close < 0) return false;
        const QByteArray element = QByteArray::fromRawData(xml.constData() + at, close - at);
        const qsizetype name = element.indexOf(" relId=\"");
        if (name >= 0) {
            const qsizetype start = at + name + 8;
            const qsizetype stop = xml.indexOf('"', start);
            if (stop > start && stop < close) {
                *valueAt = start;
                *valueEnd = stop;
                return true;
            }
        }
        from = close;
    }
    return false;
}

// The longest side, in pixels, a picture keeps in a drawing: each use of one is a copy of it in the SVG, and a photograph of twelve megapixels would
// make a drawing of tens of megabytes. And the largest an SVG may be: past it, the pictures in the drawing are left out of it.
constexpr int kMaxPictureSide = 1280;
constexpr qint64 kMaxDrawingBytes = 48 << 20;

// The picture as it is kept for the drawing: at most kMaxPictureSide pixels along its longer side (read at that size, so that a picture far
// larger than the document would keep is never read whole); one in a format that isn't pixels is kept whole. Empty for one that can't be read.
QString addDrawingPicture(Document &scratch, const QByteArray &bytes, const QString &format)
{
    static const QStringList pixels = {QStringLiteral("png"), QStringLiteral("jpeg"), QStringLiteral("jpg"), QStringLiteral("gif"), QStringLiteral("bmp"), QStringLiteral("tiff"), QStringLiteral("tif")};
    if (!pixels.contains(format.toLower())) return scratch.addImage(bytes, format);
    QBuffer in;
    in.setData(bytes);
    in.open(QIODevice::ReadOnly);
    QImageReader reader(&in);
    const QSize size = reader.size();
    if (!size.isValid()) return scratch.addImage(bytes, format);
    if (std::max(size.width(), size.height()) <= kMaxPictureSide) return scratch.addImage(bytes, format);
    reader.setScaledSize(size.scaled(kMaxPictureSide, kMaxPictureSide, Qt::KeepAspectRatio));
    const QImage image = reader.read();
    QByteArray kept;
    QBuffer out(&kept);
    out.open(QIODevice::WriteOnly);
    const bool opaque = !image.isNull() && !image.hasAlphaChannel();
    if (image.isNull() || !image.save(&out, opaque ? "JPEG" : "PNG", opaque ? 90 : -1)) {
        ++scratch.imagesLeftOut;
        return QString();
    }
    return scratch.addImage(kept, opaque ? QStringLiteral("jpeg") : QStringLiteral("png"));
}

QColor averageOf(const QImage &image)
{
    if (image.isNull()) return QColor(200, 200, 200);
    return image.scaled(1, 1, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).pixelColor(0, 0);
}

// How many pixels to the point the drawing asks to be drawn in when it is shown as a picture: a screen shows a picture drawn from an SVG at the
// size it asks for (at least 1200 pixels across, at most 4096), however far it is zoomed, so a diagram is drawn at three to the point (a frame
// across a 1080 pixel slide is then drawn with room to spare, and at three times the zoom of its slide).
constexpr double kPixelsPerPoint = 3;

// The SVG as the picture is made of it: without the writer's title and description (which say what made it, and a diagram says nothing of that),
// and asking to be drawn kPixelsPerPoint to the point, in the coordinates (points) it has.
QByteArray asPicture(const QByteArray &svg, const QSizeF &size)
{
    static const QRegularExpression note(QStringLiteral("<(title|desc)>[^<]*</\\1>|<(title|desc)/>"));
    static const QRegularExpression root(QStringLiteral("<svg\\b[^>]*>"), QRegularExpression::DotMatchesEverythingOption);
    QString text = QString::fromUtf8(svg);
    text.remove(note);
    const QRegularExpressionMatch m = root.match(text);
    if (m.hasMatch()) {
        QString tag = m.captured();
        static const QRegularExpression width(QStringLiteral("\\swidth=\"[^\"]*\"")), height(QStringLiteral("\\sheight=\"[^\"]*\""));
        tag.replace(width, QStringLiteral(" width=\"%1\"").arg(qRound(size.width() * kPixelsPerPoint)));
        tag.replace(height, QStringLiteral(" height=\"%1\"").arg(qRound(size.height() * kPixelsPerPoint)));
        text.replace(m.capturedStart(), m.capturedLength(), tag);
    }
    return text.toUtf8();
}

} // namespace

bool isDiagramFrame(const QDomElement &graphicFrame)
{
    if (graphicFrame.isNull() || graphicFrame.localName() != QLatin1String("graphicFrame")) return false;
    const QDomElement data = aChild(aChild(graphicFrame, "graphic"), "graphicData");
    return office::attr(data, QStringLiteral("uri")) == kDiagramNs;
}

DiagramLinks diagramFrameLinks(const QDomElement &graphicFrame)
{
    DiagramLinks links;
    if (!isDiagramFrame(graphicFrame)) return links;
    for (QDomElement c = aChild(aChild(graphicFrame, "graphic"), "graphicData").firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        if (c.localName() != QLatin1String("relIds") || c.namespaceURI() != kDiagramNs) continue;
        links.data = office::attrNs(c, office::ns::R, QStringLiteral("dm"));
        links.layout = office::attrNs(c, office::ns::R, QStringLiteral("lo"));
        links.style = office::attrNs(c, office::ns::R, QStringLiteral("qs"));
        links.colors = office::attrNs(c, office::ns::R, QStringLiteral("cs"));
        break;
    }
    return links;
}

QString diagramDrawingLinkId(const QByteArray &dataXml)
{
    qsizetype at = 0, end = 0;
    return findDrawingLinkId(dataXml, &at, &end) ? QString::fromUtf8(dataXml.mid(at, end - at)) : QString();
}

QByteArray withDiagramDrawingLinkId(const QByteArray &dataXml, const QString &id)
{
    qsizetype at = 0, end = 0;
    if (!findDrawingLinkId(dataXml, &at, &end)) return dataXml;
    QByteArray out = dataXml;
    out.replace(at, end - at, id.toUtf8());
    return out;
}

QByteArray diagramSvg(const QDomElement &spTree, const office::DrawingContext &base, const office::TextStyleStack &styles, const QSizeF &size, const Document &like,
                      int *imagesLeftOut)
{
    // The drawing is set in a presentation of its own the size of the frame, with the look of the one it is in; that is thrown away.
    Document scratch;
    scratch.masters.clear();
    scratch.slideText = true;
    scratch.colors = like.colors;
    scratch.fonts = like.fonts;
    scratch.styles = like.styles;
    scratch.defaultTabStop = like.defaultTabStop;
    scratch.setup.size = scratch.setup.sheet = size;
    scratch.setup.margins = QMarginsF(0, 0, 0, 0);
    auto page = std::make_shared<Page>();
    page->masterId.clear();
    scratch.pages << page;

    office::DrawingContext ctx = base;
    QHash<QString, QString> images;
    ctx.images = &images;
    ctx.addImage = [&scratch](const QByteArray &bytes, const QString &format) { return addDrawingPicture(scratch, bytes, format); };
    ctx.addMedia = nullptr;
    ctx.newStory = [&scratch] { return scratch.createStory(); };
    ctx.storyDoc = [&scratch](const QString &id) { return scratch.storyDoc(id); };
    page->items = office::readShapeTree(spTree, ctx, styles);

    LayoutCache cache;
    PaintContext paint;
    paint.doc = &scratch;
    paint.cache = &cache;
    paint.opt.output = true;
    // A picture in it is kept as fine as a screen the width of a slide shows it, four points to the pixel.
    paint.opt.maxImageDpi = 432;
    QByteArray svg = asPicture(pageSvg(paint, 0, QString()), size);
    if (svg.size() > kMaxDrawingBytes) {
        // Each use of a picture is a copy of it: a drawing that has too many shows its pictures as the color they average, and its pictures' places empty.
        walkItems(page->items, [&](const ItemPtr &it) {
            if (it->fill.type == Fill::Picture || it->fill.type == Fill::Texture) it->fill = Fill::solid(ColorRef::rgb(averageOf(scratch.image(it->fill.imageId))));
            if (it->type() == ItemType::Picture) static_cast<PictureItem &>(*it).imageId.clear();
        });
        svg = asPicture(pageSvg(paint, 0, QString()), size);
    }
    if (imagesLeftOut) *imagesLeftOut += scratch.imagesLeftOut;
    return svg;
}

} // namespace jp::deck
