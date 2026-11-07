#include "render/diagramdrawing.h"

#include "office/chart.h"
#include "office/diagram.h"
#include "office/xml.h"
#include "render/renderer.h"
#include "render/svgexport.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDomDocument>
#include <QHash>
#include <QImage>
#include <QImageReader>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <memory>

namespace jp {

namespace {

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
QByteArray asPicture(const QByteArray &svg, const QSizeF &drawn, const QSizeF &shown)
{
    const QSizeF size = shown.isEmpty() ? drawn : shown;
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

// The size the shapes of a drawing say it was made for: from the top left corner to the farthest edge of its shapes, with as much room again at the far edges as the shapes leave at the near
// ones (a diagram's layout sets its shapes in the middle of its frame). Empty for a drawing with no shape that has a size.
QSizeF extentOfShapes(const QDomElement &tree)
{
    double left = 1e18, top = 1e18, right = 0, bottom = 0;
    int seen = 0;
    for (QDomElement e = tree.firstChildElement(); !e.isNull() && seen < 20000; e = e.nextSiblingElement(), ++seen) {
        const QString kind = e.localName();
        if (kind != QLatin1String("sp") && kind != QLatin1String("grpSp")) continue;
        const QDomElement props = office::child(e, office::kDiagramDrawingNs, kind == QLatin1String("sp") ? QStringLiteral("spPr") : QStringLiteral("grpSpPr"));
        const QDomElement xfrm = office::child(props, office::ns::A, QStringLiteral("xfrm")), off = office::child(xfrm, office::ns::A, QStringLiteral("off")), ext = office::child(xfrm, office::ns::A, QStringLiteral("ext"));
        if (ext.isNull()) continue;
        const double x = office::emuToPoints(office::parseInt(office::attr(off, QStringLiteral("x")))), y = office::emuToPoints(office::parseInt(office::attr(off, QStringLiteral("y"))));
        const double w = office::emuToPoints(office::parseInt(office::attr(ext, QStringLiteral("cx")))), h = office::emuToPoints(office::parseInt(office::attr(ext, QStringLiteral("cy"))));
        left = std::min(left, std::max(0.0, x));
        top = std::min(top, std::max(0.0, y));
        right = std::max(right, x + w);
        bottom = std::max(bottom, y + h);
    }
    if (right <= 0 || bottom <= 0 || left > right || top > bottom) return QSizeF();
    return QSizeF(std::min(right + left, office::kChartMaxSide), std::min(bottom + top, office::kChartMaxSide));
}

} // namespace

QByteArray diagramSvg(const QDomElement &spTree, const office::DrawingContext &base, const office::TextStyleStack &styles, const QSizeF &size, const Document &like,
                      int *imagesLeftOut, const QSizeF &shown)
{
    // The drawing is set in a presentation of its own the size of the frame, with the look of the one it is in; that is thrown away.
    Document scratch;
    scratch.masters.clear();
    scratch.slideText = true;
    scratch.slidePictures = true;
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
    QByteArray svg = asPicture(pageSvg(paint, 0, QString(), true), size, shown);
    if (svg.size() > kMaxDrawingBytes) {
        // Each use of a picture is a copy of it: a drawing that has too many shows its pictures as the color they average, and its pictures' places empty.
        walkItems(page->items, [&](const ItemPtr &it) {
            if (it->fill.type == Fill::Picture || it->fill.type == Fill::Texture) it->fill = Fill::solid(ColorRef::rgb(averageOf(scratch.image(it->fill.imageId))));
            if (it->type() == ItemType::Picture) static_cast<PictureItem &>(*it).imageId.clear();
        });
        svg = asPicture(pageSvg(paint, 0, QString(), true), size, shown);
    }
    if (imagesLeftOut) *imagesLeftOut += scratch.imagesLeftOut;
    return svg;
}

QByteArray diagramDrawingSvg(const QByteArray &drawingXml, const QString &drawingPart, const office::Package *package, const office::Theme &theme, const QSizeF &frame,
                             const QSizeF &shown, int *imagesLeftOut, QSizeF *drawnSize)
{
    if (drawingXml.isEmpty()) return {};
    QDomDocument dom;
    if (!office::parseXml(drawingXml, &dom)) return {};
    const QDomElement root = dom.documentElement();
    if (root.localName() != QLatin1String("drawing") || root.namespaceURI() != office::kDiagramDrawingNs) return {};
    const QDomElement tree = office::child(root, office::kDiagramDrawingNs, QStringLiteral("spTree"));
    if (tree.isNull()) return {};
    QSizeF natural = frame;
    if (natural.width() < 1 || natural.height() < 1) natural = extentOfShapes(tree);
    if (natural.width() < 1 || natural.height() < 1) return {};
    if (drawnSize) *drawnSize = natural;

    office::DrawingContext ctx;
    ctx.package = package;
    ctx.part = drawingPart;
    ctx.theme = theme;
    QDomDocument themeDoc;
    if (!theme.xml.isEmpty() && office::parseXml(theme.xml, &themeDoc))
        ctx.formats = office::child(office::child(themeDoc.documentElement(), office::ns::A, QStringLiteral("themeElements")), office::ns::A, QStringLiteral("fmtScheme"));
    // What one drawing may keep: its shapes, and the words in them.
    office::DrawingBudget budget;
    using B = office::DrawingBudget;
    budget.left[B::Objects] = kMostDiagramShapes;
    budget.left[B::Stories] = kMostDiagramShapes;
    budget.left[B::Cells] = 0;
    budget.left[B::Paragraphs] = 20000;
    budget.left[B::Runs] = 50000;
    budget.left[B::Letters] = 500000;
    ctx.budget = &budget;
    const Document like;
    return diagramSvg(tree, ctx, office::TextStyleStack(), natural, like, imagesLeftOut, shown);
}

QByteArray diagramPlaceholderSvg(const QSizeF &size)
{
    Document scratch;
    scratch.masters.clear();
    scratch.slideText = true;
    scratch.setup.size = scratch.setup.sheet = size;
    scratch.setup.margins = QMarginsF(0, 0, 0, 0);
    auto page = std::make_shared<Page>();
    page->masterId.clear();
    scratch.pages << page;
    auto box = std::make_shared<TextItem>();
    box->rect = QRectF(QPointF(0, 0), size);
    box->fill = Fill::solid(ColorRef::rgb(QColor(0xF3, 0xF3, 0xF3)));
    box->stroke = Stroke::line(ColorRef::rgb(QColor(0x80, 0x80, 0x80)), 1);
    box->stroke.dash = Stroke::DashLine;
    box->valign = VAlign::Middle;
    box->insets = QMarginsF(6, 6, 6, 6);
    box->storyId = scratch.createStory(QCoreApplication::translate("jp::DiagramDrawing", "Diagram (the file keeps no picture of it)"));
    if (QTextDocument *words = scratch.storyDoc(box->storyId)) {
        QTextCursor c(words);
        c.select(QTextCursor::Document);
        QTextCharFormat look;
        QFont font = words->defaultFont();
        font.setPointSizeF(std::clamp(size.height() / 8, 8.0, 14.0));
        look.setFont(font);
        look.setForeground(QColor(0x50, 0x50, 0x50));
        c.mergeCharFormat(look);
        QTextBlockFormat centered;
        centered.setAlignment(Qt::AlignHCenter);
        c.mergeBlockFormat(centered);
    }
    page->items.push_back(box);
    LayoutCache cache;
    PaintContext paint;
    paint.doc = &scratch;
    paint.cache = &cache;
    paint.opt.output = true;
    return asPicture(pageSvg(paint, 0, QString(), true), size, QSizeF());
}

} // namespace jp
