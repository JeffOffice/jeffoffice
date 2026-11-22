// JeffDeck's tests of whole-pixel drawing in a picture of a slide: rule, rectangle edges, and picture edges are whole device pixels at any device pixel ratio,
// zoom, and fractional offset (libs/draw/render/pixelsnap.h, shared with JeffWrite's table rules).
#include "deckhelpers.h"
#include "render/pixelsnap.h"
#include <QPicture>

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
                for (double offset : {0.0, 0.13, 0.3, 0.5, 0.77}) {
                    const QString where = QStringLiteral("ratio %1 zoom %2 offset %3: ").arg(ratio).arg(zoom).arg(offset);
                    const QImage img = paint(true, ratio, zoom, offset, 0);
                    QVERIFY2(blended(img) == 0, qPrintable(where + QStringLiteral("%1 blended pixels").arg(blended(img))));
                    const int thick = std::max(1, int(std::floor(1.5 * ratio * zoom + 0.5)));
                    const int row = img.height() * 3 / 10;   // (a row through the rectangle's middle, left of the picture)
                    int left = 0;
                    for (int x = 0; x < img.width() / 2 && !isRed(img.pixel(x, row)); ++x) left += isBlack(img.pixel(x, row));
                    QVERIFY2(left == thick, qPrintable(where + QStringLiteral("left side %1 pixels, not %2").arg(left).arg(thick)));
                }
    }

    // A table's rules meet: at ratios 1 to 2 and fractional offsets, the outer border is one unbroken run of black round the whole table (no background pixel in a
    // corner, where each rule used to end at its own corner) and the rule between the cells runs from edge to edge.
    void aBorderedTableHasNoHoleAtItsCorners()
    {
        for (double ratio : {1.0, 1.25, 1.5, 2.0})
            for (double offset : {0.0, 0.13, 0.5, 0.77}) {
                const QString where = QStringLiteral("ratio %1 offset %2: ").arg(ratio).arg(offset);
                Document doc;
                doc.slideText = true;
                auto table = std::make_shared<TableItem>();
                table->rows = table->cols = 2;
                table->colW = {20, 20};
                table->rowH = {15, 15};
                table->cells.resize(4);
                for (TableCell &c : table->cells) c.border.top = c.border.bottom = c.border.left = c.border.right = Stroke::line(ColorRef::rgb(Qt::black), 1);
                table->rect = QRectF(10.5, 10.5, 40, 30);
                table->syncRect();
                const QImage img = shot(doc, {table}, ratio, 1.0, offset);
                QVERIFY2(blended(img) == 0, qPrintable(where + QStringLiteral("%1 blended pixels").arg(blended(img))));
                QRect box;
                for (int y = 0; y < img.height(); ++y)
                    for (int x = 0; x < img.width(); ++x)
                        if (isBlack(img.pixel(x, y))) box |= QRect(x, y, 1, 1);
                QVERIFY2(box.width() > 10 && box.height() > 10, qPrintable(where + "no table drawn"));
                int holes = 0;
                for (int x = box.left(); x <= box.right(); ++x) holes += !isBlack(img.pixel(x, box.top())) + !isBlack(img.pixel(x, box.bottom())) + !isBlack(img.pixel(x, box.center().y()));
                for (int y = box.top(); y <= box.bottom(); ++y) holes += !isBlack(img.pixel(box.left(), y)) + !isBlack(img.pixel(box.right(), y)) + !isBlack(img.pixel(box.center().x(), y));
                QVERIFY2(holes == 0, qPrintable(where + QStringLiteral("%1 background pixels in the borders").arg(holes)));
            }
    }

    // A text box's fill, a rule drawn as a two-point path (a horizontal and a vertical line), and two shapes that meet at a fractional place: whole pixels, and the
    // two shapes leave no seam of background between them.
    void textBoxFillsLinesAndAbuttingShapesAreWholePixels()
    {
        for (double ratio : {1.0, 1.25, 2.0})
            for (double offset : {0.0, 0.13, 0.77}) {
                const QString where = QStringLiteral("ratio %1 offset %2: ").arg(ratio).arg(offset);
                Document doc;
                doc.slideText = true;
                auto box = std::make_shared<TextItem>();
                box->rect = QRectF(5.3, 5.4, 30.6, 12.2);
                box->fill = Fill::solid(ColorRef::rgb(Qt::red));
                auto h = std::make_shared<LineItem>();
                h->p1 = QPointF(5.3, 30.2);
                h->p2 = QPointF(60.7, 30.2);
                h->stroke = Stroke::line(ColorRef::rgb(Qt::black), 1.2);
                auto v = std::make_shared<LineItem>();
                v->p1 = QPointF(70.3, 5.2);
                v->p2 = QPointF(70.3, 40.9);
                v->stroke = Stroke::line(ColorRef::rgb(Qt::black), 1.2);
                auto left = std::make_shared<ShapeItem>(), right = std::make_shared<ShapeItem>();
                left->shape = right->shape = QStringLiteral("rect");
                left->rect = QRectF(5.2, 40.2, 20.35, 8.3);
                right->rect = QRectF(25.55, 40.2, 20.3, 8.3);
                left->fill = Fill::solid(ColorRef::rgb(Qt::red));
                right->fill = Fill::solid(ColorRef::rgb(Qt::blue));
                left->stroke = right->stroke = Stroke();
                left->stroke.color = right->stroke.color = ColorRef();
                const QImage img = shot(doc, {box, h, v, left, right}, ratio, 1.0, offset);
                QVERIFY2(blended(img) == 0, qPrintable(where + QStringLiteral("%1 blended pixels").arg(blended(img))));
                // (A row through the two shapes: from the first red pixel to the last blue one there is no white.)
                const int row = int(std::lround((44.3 + offset) * ratio));
                int first = -1, last = -1;
                for (int x = 0; x < img.width(); ++x) {
                    if (isRed(img.pixel(x, row)) && first < 0) first = x;
                    if (img.pixel(x, row) == 0xff0000ff) last = x;
                }
                QVERIFY2(first >= 0 && last > first, qPrintable(where + "the shapes are not drawn"));
                for (int x = first; x <= last; ++x) QVERIFY2(img.pixel(x, row) != 0xffffffff, qPrintable(where + QStringLiteral("a seam of background at %1").arg(x)));
            }
    }

    // A rectangle thinner than a pixel is not made a whole pixel (at a small zoom, as in a thumbnail, it would be many times too heavy): it is drawn smoothed, as before.
    void aFillUnderAPixelStaysLight()
    {
        Document doc;
        doc.slideText = true;
        auto bar = std::make_shared<ShapeItem>();
        bar->shape = QStringLiteral("rect");
        bar->rect = QRectF(10, 10.3, 60, 0.3);
        bar->fill = Fill::solid(ColorRef::rgb(Qt::black));
        bar->stroke = Stroke();
        bar->stroke.color = ColorRef();
        auto inkOf = [](const QImage &img) {
            double ink = 0;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x) ink += (255 - qGray(img.pixel(x, y))) / 255.0;
            return ink;
        };
        for (double zoom : {0.1, 0.5, 0.9}) {
            const double snapped = inkOf(shot(doc, {bar}, 1.0, zoom, 0));
            const double smoothed = inkOf(shot(doc, {bar}, 1.0, zoom, 0, [](RenderOptions &o) { o.noWholePixels = true; }));
            QVERIFY2(snapped <= smoothed + 1e-6, qPrintable(QStringLiteral("zoom %1: %2 of ink, %3 smoothed").arg(zoom).arg(snapped).arg(smoothed)));
        }
    }

    // What is not whole-pixel stays as it was: a turned rectangle is drawn smoothed, and so is a slide-less document's (JeffPub's) rectangle; output for a PDF or the
    // printer (a text output other than a picture's, or output flattened), an object a caller is moving (noWholePixels), and a vector engine's paint are not snapped either.
    void turnedAndNonSlideDrawingStaysSmoothed()
    {
        QVERIFY2(blended(paint(true, 1.0, 4.0 / 3, 0.3, 20)) > 0, "a turned rectangle is smoothed");
        QVERIFY2(blended(paint(false, 1.0, 4.0 / 3, 0.3, 0)) > 0, "a document that is not a slide's keeps its smoothing");
        for (int kind = 0; kind < 4; ++kind) {
            const QImage img = paint(true, 1.0, 4.0 / 3, 0.3, 0, [kind](RenderOptions &o) {
                if (kind == 0) o.textOutput = TextOutput::Pdf;
                if (kind == 1) o.textOutput = TextOutput::Print;
                if (kind == 2) o.flattenTransparency = true;
                if (kind == 3) o.noWholePixels = true;
            });
            QVERIFY2(blended(img) > 0, qPrintable(QStringLiteral("output kind %1 is snapped").arg(kind)));
        }
        // A vector engine (what a picture export to a vector format paints into) records the rectangle as it is, to be drawn as it is wherever it is played.
        Document doc;
        doc.slideText = true;
        auto rect = std::make_shared<ShapeItem>();
        rect->shape = QStringLiteral("rect");
        rect->rect = QRectF(10.3, 8.4, 40.7, 30.2);
        rect->fill = Fill::solid(ColorRef::rgb(Qt::red));
        rect->stroke = Stroke::line(ColorRef::rgb(Qt::black), 1.5);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = &doc;
        ctx.cache = &cache;
        QPicture recorded;
        {
            QPainter vp(&recorded);
            Renderer::paintItem(&vp, ctx, *rect);
        }
        QImage img(100, 50, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);
        {
            QPainter ip(&img);
            ip.setRenderHint(QPainter::Antialiasing);
            ip.translate(0.3, 0.3);
            ip.drawPicture(0, 0, recorded);
        }
        QVERIFY2(blended(img) > 0, "a vector engine's paint is snapped");
    }

    // A page flattened for a PDF (it has see-through objects) draws them into pictures at the flatten resolution beside the vector ones: nothing there is made whole
    // pixels, or the edges of a see-through object would be crisp beside smoothed neighbors.
    void aPageFlattenedForPdfIsNotSnapped()
    {
        auto doc = showDeck(1);
        Page &slide = *doc->pages[0];
        slide.items.clear();
        auto rect = std::make_shared<ShapeItem>();
        rect->shape = QStringLiteral("rect");
        rect->rect = QRectF(100.3, 80.4, 200.7, 100.2);
        rect->fill = Fill::solid(ColorRef::rgb(Qt::red), 0.5);
        rect->stroke = Stroke();
        rect->stroke.color = ColorRef();
        slide.items.push_back(rect);
        // (The same page drawn as a page that is not a slide's, which is never snapped, is the picture it must come out as.)
        auto drawn = [&](bool asSlide) {
            doc->slideText = asSlide;
            LayoutCache cache;
            PaintContext ctx;
            ctx.doc = doc.get();
            ctx.cache = &cache;
            ctx.opt.flattenTransparency = true;
            ctx.opt.flattenDpi = 96;
            QImage img(int(std::ceil(doc->pageSize().width())), int(std::ceil(doc->pageSize().height())), QImage::Format_ARGB32_Premultiplied);
            img.fill(Qt::white);
            QPainter p(&img);
            p.setRenderHint(QPainter::Antialiasing);
            Renderer::paintPage(&p, ctx, 0);
            p.end();
            return img;
        };
        const QImage slide1 = drawn(true), plain = drawn(false);
        QVERIFY(slide1.size() == plain.size());
        QVERIFY2(slide1 == plain, "the flattened page of a slide is drawn differently from the unsnapped one");
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
    // The items drawn into a white picture at the device pixel ratio, zoom, and offset (as a screen scrolled and zoomed shows them).
    static QImage shot(Document &doc, const ItemList &items, double ratio, double zoom, double offset, const std::function<void(RenderOptions &)> &options = {})
    {
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = &doc;
        ctx.cache = &cache;
        if (options) options(ctx.opt);
        QImage img(int(std::ceil(100 * ratio * zoom)), int(std::ceil(50 * ratio * zoom)), QImage::Format_ARGB32_Premultiplied);
        img.setDevicePixelRatio(ratio);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.scale(zoom, zoom);
        p.translate(offset, offset);
        for (const auto &it : items) Renderer::paintItem(&p, ctx, *it);
        p.end();
        img.setDevicePixelRatio(1);
        return img;
    }
    // A slide (or a page that is not one) with a rectangle at fractional places and a picture beside it.
    static QImage paint(bool slide, double ratio, double zoom, double offset, double turn, const std::function<void(RenderOptions &)> &options = {})
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
        return shot(doc, {rect, pic}, ratio, zoom, offset, options);
    }
};

JP_TEST_CLASS(DeckPixelSnapTests)
#include "tst_deck_pixelsnap.moc"
