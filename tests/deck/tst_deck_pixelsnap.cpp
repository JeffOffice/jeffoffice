// JeffDeck's tests of whole-pixel drawing in a picture of a slide: rule, rectangle edges, and picture edges are whole device pixels at any device pixel ratio,
// zoom, and fractional offset (libs/draw/render/pixelsnap.h, shared with JeffWrite's table rules).
#include "deckhelpers.h"
#include "render/pixelsnap.h"

class DeckPixelSnapTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // A slide's rectangle (red fill, black outline) and picture (solid blue): every pixel is the page's white or one of the three colors, never a blend over an
    // edge, at each ratio, zoom, and offset; the outline is as thick as its width in device pixels, rounded, and the same on all four sides.
    void slideEdgesAreWholeDevicePixels()
    {
        for (double ratio : {1.0, 1.25, 1.5, 2.0})
            for (double zoom : {0.9, 1.0, 4.0 / 3})
                for (double offset : {0.0, 0.3, 0.5}) {
                    const QString where = QStringLiteral("ratio %1 zoom %2 offset %3: ").arg(ratio).arg(zoom).arg(offset);
                    const QImage img = paint(true, ratio, zoom, offset, 0);
                    int blends = 0;
                    for (int y = 0; y < img.height(); ++y)
                        for (int x = 0; x < img.width(); ++x) blends += !isPure(img.pixel(x, y));
                    QVERIFY2(blends == 0, qPrintable(where + QStringLiteral("%1 blended pixels").arg(blends)));
                    const int thick = std::max(1, int(std::floor(1.5 * ratio * zoom + 0.5)));
                    const int row = img.height() * 3 / 10;   // (a row through the rectangle's middle, left of the picture)
                    int left = 0;
                    for (int x = 0; x < img.width() / 2 && !isRed(img.pixel(x, row)); ++x) left += isBlack(img.pixel(x, row));
                    QVERIFY2(left == thick, qPrintable(where + QStringLiteral("left side %1 pixels, not %2").arg(left).arg(thick)));
                }
    }

    // What is not whole-pixel stays as it was: a turned rectangle is drawn smoothed, and so is a slide-less document's (JeffPub's) rectangle.
    void turnedAndNonSlideDrawingStaysSmoothed()
    {
        QVERIFY2(blended(paint(true, 1.0, 4.0 / 3, 0.3, 20)) > 0, "a turned rectangle is smoothed");
        QVERIFY2(blended(paint(false, 1.0, 4.0 / 3, 0.3, 0)) > 0, "a document that is not a slide's keeps its smoothing");
    }

    // The shared function: a rule is as thick as its width in device pixels (never under one), where it falls snapped on its own, the same at any ratio.
    void wholePixelRuleBoxIsThickFromItsWidthAlone()
    {
        for (double ratio : {1.0, 1.25, 1.5, 2.0})
            for (double offset : {0.0, 0.25, 0.5, 0.75}) {
                QImage img(40, 40, QImage::Format_ARGB32_Premultiplied);
                img.setDevicePixelRatio(ratio);
                QPainter p(&img);
                p.translate(offset, offset);
                const auto box = jp::wholePixelRuleBox(&p, QPointF(2, 5.3), QPointF(20, 5.3), 0.4);
                QVERIFY(box.has_value());
                QCOMPARE(box->height(), std::max(1, int(std::floor(0.4 * ratio + 0.5))));
                p.rotate(10);
                QVERIFY(!jp::wholePixelRuleBox(&p, QPointF(2, 5), QPointF(20, 5), 1).has_value());
                p.resetTransform();
                QVERIFY(!jp::wholePixelRuleBox(&p, QPointF(2, 5), QPointF(20, 9), 1).has_value());   // (not along an axis)
            }
    }

private:
    static bool isRed(QRgb c) { return c == 0xffff0000; }
    static bool isBlack(QRgb c) { return c == 0xff000000; }
    static bool isPure(QRgb c) { return c == 0xffffffff || c == 0xffff0000 || c == 0xff000000 || c == 0xff0000ff || c == 0x00000000; }
    static int blended(const QImage &img)
    {
        int n = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) n += !isPure(img.pixel(x, y));
        return n;
    }
    // A slide (or a page that is not one) with a rectangle at fractional places and a picture beside it, drawn at the device pixel ratio, zoom, and offset.
    static QImage paint(bool slide, double ratio, double zoom, double offset, double turn)
    {
        Document doc;
        doc.slideText = slide;
        doc.slidePictures = slide;
        auto rect = std::make_shared<ShapeItem>();
        rect->shape = QStringLiteral("rect");
        rect->rect = QRectF(10.3, 8.4, 40.7, 30.2);
        rect->rotation = turn;
        rect->fill = Fill::solid(ColorRef::rgb(Qt::red));
        rect->stroke = Stroke::line(ColorRef::rgb(Qt::black), 1.5);
        auto pic = std::make_shared<PictureItem>();
        QImage blue(4, 4, QImage::Format_RGB32);
        blue.fill(Qt::blue);
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        blue.save(&buf, "PNG");
        pic->imageId = doc.addImage(png, QStringLiteral("png"));
        pic->rect = QRectF(60.6, 8.4, 20.3, 30.2);
        pic->imgRect = QRectF(QPointF(0, 0), pic->rect.size());
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = &doc;
        ctx.cache = &cache;
        QImage img(int(std::ceil(100 * ratio * zoom)), int(std::ceil(50 * ratio * zoom)), QImage::Format_ARGB32_Premultiplied);
        img.setDevicePixelRatio(ratio);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.scale(zoom, zoom);
        p.translate(offset, offset);
        Renderer::paintItem(&p, ctx, *rect);
        Renderer::paintItem(&p, ctx, *pic);
        p.end();
        img.setDevicePixelRatio(1);
        return img;
    }
};

JP_TEST_CLASS(DeckPixelSnapTests)
#include "tst_deck_pixelsnap.moc"
