// JeffDeck's tests of diagrams, equations, and charts in a presentation file.
#include "deckhelpers.h"

class DeckPptxObjectsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // What can't be read yet is kept to write back, with its place.
    void importKeepsUnreadFrames()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const QJsonArray unread = d->pages[0]->props.value(QStringLiteral("unread")).toArray();
        QCOMPARE(int(unread.size()), 1);
        const QJsonObject frame = unread[0].toObject();
        QCOMPARE(frame.value(QStringLiteral("x")).toDouble(), 72.0);
        QCOMPARE(frame.value(QStringLiteral("y")).toDouble(), 144.0);
        QCOMPARE(frame.value(QStringLiteral("w")).toDouble(), 288.0);
        QCOMPARE(frame.value(QStringLiteral("h")).toDouble(), 108.0);
        // The XML stands alone: it parses by itself, with the prefixes it uses declared.
        QDomDocument dom;
        QString why;
        QVERIFY2(office::parseXml(frame.value(QStringLiteral("xml")).toString().toUtf8(), &dom, &why), qPrintable(why));
        QVERIFY(office::isElement(dom.documentElement(), office::ns::P, QStringLiteral("graphicFrame")));
        QVERIFY(frame.value(QStringLiteral("xml")).toString().contains(QStringLiteral("Cell &amp; &lt;1&gt;")));
        QCOMPARE(dom.documentElement().elementsByTagNameNS(QString::fromLatin1(office::ns::A.data()), QStringLiteral("t")).at(0).toElement().text(), QStringLiteral("Cell & <1>"));
        // The chart and the object on the second slide have pictures of themselves to read in their
        // place; where the reader can't read pictures yet, they are kept like that frame.
        const QJsonArray second = d->pages[1]->props.value(QStringLiteral("unread")).toArray();
        QCOMPARE(int(second.size()), readerReadsPictures() ? 0 : 2);
        if (!second.isEmpty()) {
            const QString xml = second[0].toObject().value(QStringLiteral("xml")).toString();
            QVERIFY(xml.contains(QStringLiteral("AlternateContent")));
            QVERIFY(office::parseXml(xml.toUtf8(), &dom, &why));
            QCOMPARE(second[0].toObject().value(QStringLiteral("x")).toDouble(), 96.0);
            QCOMPARE(second[0].toObject().value(QStringLiteral("w")).toDouble(), 480.0);
            const QString object = second[1].toObject().value(QStringLiteral("xml")).toString();
            QVERIFY(object.contains(QStringLiteral("oleObj")));
            QVERIFY(office::parseXml(object.toUtf8(), &dom, &why));
            QCOMPARE(second[1].toObject().value(QStringLiteral("x")).toDouble(), 576.0);
        }
        // Nothing else is added for them.
        QCOMPARE(int(d->pages[0]->items.size()), readerReadsPictures() ? 6 : 5);
    }

private:
    static QJsonObject keptFrameOf(const Page &page, const QString &kind)
    {
        for (const QJsonValue &v : page.props.value(QStringLiteral("charts")).toArray())
            if (v.toObject().value(QStringLiteral("kind")).toString() == kind) return v.toObject();
        return QJsonObject();
    }
    // Every element called `name` (by local name) in `e` and below it, in document order.
    static void collectNamed(const QDomElement &e, const QString &name, QVector<QDomElement> *out)
    {
        if (e.localName() == name) out->append(e);
        for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) collectNamed(c, name, out);
    }
    // The graphic frame called `name` in the XML of a slide.
    static QDomElement frameNamed(const QDomElement &root, const QString &name)
    {
        QVector<QDomElement> all;
        collectNamed(root, QStringLiteral("graphicFrame"), &all);
        for (const QDomElement &f : std::as_const(all))
            if (office::child(office::child(f, office::ns::P, QStringLiteral("nvGraphicFramePr")), office::ns::P, QStringLiteral("cNvPr")).attribute(QStringLiteral("name")) == name) return f;
        return QDomElement();
    }
    static QByteArray keptBytes(const QJsonObject &frame, const QString &part)
    {
        return QByteArray::fromBase64(frame.value(QStringLiteral("parts")).toObject().value(part).toObject().value(QStringLiteral("data")).toString().toLatin1());
    }
    // The SVG a picture is, drawn into pixels `size` big on a clear ground.
    static QImage svgPixels(const Document &d, const PictureItem &pic, const QSize &size)
    {
        QSvgRenderer svg(d.images.value(pic.imageId).bytes);
        QImage img(size, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter painter(&img);
        svg.render(&painter);
        return img;
    }

private Q_SLOTS:
    // A diagram (SmartArt) on a slide is a picture of its drawing (an SVG, drawn from the shapes the drawing part holds, in the frame's place), and
    // what it is made of (the data, layout, style, and colors, the drawing, and the pictures in it) is kept with the slide to be written back.
    void importReadsDiagramsAsPictures()
    {
        QString err;
        auto d = deck::importPresentation(pack(smartArtPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const Page &page = *d->pages[0];
        const PictureItem *pic = keptPicture(*d, page, QStringLiteral("diagram"));
        QVERIFY(pic);
        QCOMPARE(pic->name, QStringLiteral("Diagram 8"));
        QCOMPARE(pic->altText, QStringLiteral("A process"));
        QCOMPARE(pic->rect, QRectF(72, 72, 288, 144));
        QCOMPARE(pic->imgRect, QRectF(0, 0, 288, 144));
        QCOMPARE(d->images.value(pic->imageId).format, QStringLiteral("svg"));
        // Drawn as the shapes are: in the theme's accent, in the color the shape names, with the white of the line and the text.
        const QByteArray svg = d->images.value(pic->imageId).bytes;
        QVERIFY(svg.contains("#b6420f"));
        QVERIFY(svg.contains("#336699"));
        QVERIFY(svg.contains("#fefefe"));   // the theme's lt1: the text, by the style's font reference
        QVERIFY(!svg.contains("JeffPub"));
        // The drawing is in points, and asks to be drawn three pixels to the point: a screen draws it at that size however far it is zoomed in.
        QSvgRenderer drawing(svg);
        QCOMPARE(drawing.viewBoxF(), QRectF(0, 0, 288, 144));
        QCOMPARE(drawing.defaultSize(), QSize(864, 432));
        QCOMPARE(d->images.value(pic->imageId).image().size(), QSize(1200, 600));   // and a picture of it is as large as a screen needs: 1200 across at least
        const QImage img = svgPixels(*d, *pic, QSize(288, 144));
        QCOMPARE(img.pixelColor(10, 45), QColor(0xB6, 0x42, 0x0F));    // the first shape, at 0 to 84 across and 36 to 108 down
        QCOMPARE(img.pixelColor(110, 45), QColor(0x33, 0x66, 0x99));   // the second, from 102 across
        QCOMPARE(img.pixelColor(210, 45), QColor(0x20, 0x90, 0x40));   // the third is filled with its picture
        QCOMPARE(img.pixelColor(100, 10).alpha(), 0);                   // nothing in between, or around
        QCOMPARE(img.pixelColor(10, 130).alpha(), 0);
        // The text is in the rectangle the drawing gives it: light letters in the middle of the first shape, none at its corner.
        bool light = false;
        for (int y = 56; y < 88 && !light; ++y)
            for (int x = 20; x < 64 && !light; ++x) light = img.pixelColor(x, y).red() > 0xE0 && img.pixelColor(x, y).green() > 0xE0;
        QVERIFY(light);
        // Only the unknown frame of the sample is left unread, and the diagram's frame is kept apart from it.
        QCOMPARE(int(page.props.value(QStringLiteral("unread")).toArray().size()), 1);
        const QJsonObject kept = keptFrameOf(page, QStringLiteral("diagram"));
        QCOMPARE(kept.value(QStringLiteral("item")).toString(), pic->id);
        QDomDocument dom;
        QString why;
        QVERIFY2(office::parseXml(kept.value(QStringLiteral("frameXml")).toString().toUtf8(), &dom, &why), qPrintable(why));
        QVERIFY(office::isElement(dom.documentElement(), office::ns::P, QStringLiteral("graphicFrame")));
        const Parts source = smartArtPptx();
        const QJsonObject parts = kept.value(QStringLiteral("parts")).toObject();
        QCOMPARE(parts.keys(), (QStringList{"/ppt/diagrams/colors1.xml", "/ppt/diagrams/data1.xml", "/ppt/diagrams/drawing1.xml", "/ppt/diagrams/layout1.xml", "/ppt/diagrams/quickStyle1.xml", "/ppt/media/diagram1.png"}));
        for (const QString &name : parts.keys()) {
            QCOMPARE(keptBytes(kept, name), source.data.value(name).xml);
            QCOMPARE(parts.value(name).toObject().value(QStringLiteral("type")).toString(), source.data.value(name).type);
        }
        // The slide's links to them, as the file has them.
        const QJsonArray links = kept.value(QStringLiteral("links")).toArray();
        QCOMPARE(int(links.size()), 5);
        QStringList types;
        for (const QJsonValue &l : links) types << l.toObject().value(QStringLiteral("type")).toString().section(QLatin1Char('/'), -1);
        types.sort();
        QCOMPARE(types, (QStringList{"diagramColors", "diagramData", "diagramDrawing", "diagramLayout", "diagramQuickStyle"}));
    }

    // A diagram that has no drawing part (an older file) is kept as the frame it is, as before: nothing is shown for it.
    void importLeavesADiagramWithoutADrawingUnread()
    {
        QString err;
        auto d = deck::importPresentation(pack(smartArtPptx(false, false)), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(!keptPicture(*d, *d->pages[0], QStringLiteral("diagram")));
        QCOMPARE(int(d->pages[0]->props.value(QStringLiteral("unread")).toArray().size()), 2);
        QCOMPARE(int(d->pages[0]->items.size()), readerReadsPictures() ? 6 : 5);
        // A drawing that isn't a drawing, or has no shapes to draw from, is the same.
        Parts p = smartArtPptx();
        p.data["/ppt/diagrams/drawing1.xml"].xml = "<dsp:other xmlns:dsp=\"http://schemas.microsoft.com/office/drawing/2008/diagram\"/>";
        d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(!keptPicture(*d, *d->pages[0], QStringLiteral("diagram")));
        QCOMPARE(int(d->pages[0]->props.value(QStringLiteral("unread")).toArray().size()), 2);
        // A diagram whose data part is gone from the file is the same.
        p = smartArtPptx();
        p.data.remove(QStringLiteral("/ppt/diagrams/data1.xml"));
        d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(!keptPicture(*d, *d->pages[0], QStringLiteral("diagram")));
    }

    // A diagram that many frames show is read and drawn once for each size and look, and its parts are kept once.
    void importDrawsADiagramOnceForTheFramesThatShowIt()
    {
        QMap<QString, QByteArray> zip;
        QString err;
        QVERIFY2(readZip(pack(smartArtPptx()), zip, &err), qPrintable(err));
        QString slide = QString::fromUtf8(zip.value(QStringLiteral("ppt/slides/slide1.xml")));
        const int at = int(slide.indexOf(QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"9\"")));
        const int end = int(slide.indexOf(QStringLiteral("</p:graphicFrame>"), at)) + 17;
        const QString frame = slide.mid(at, end - at);
        QString more;
        for (int i = 0; i < 60; ++i)
            more += QString(frame).replace(QStringLiteral("id=\"9\""), QStringLiteral("id=\"%1\"").arg(100 + i)).replace(QStringLiteral("Diagram 8"), QStringLiteral("Diagram %1").arg(100 + i))
                        .replace(QStringLiteral("x=\"914400\""), QStringLiteral("x=\"%1\"").arg(914400 + i * 1000));
        slide.insert(end, more);
        zip[QStringLiteral("ppt/slides/slide1.xml")] = slide.toUtf8();
        ZipWriter z;
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) z.add(it.key(), it.value(), true);
        auto d = deck::importPresentation(z.finish(), &err);
        QVERIFY2(d, qPrintable(err));
        const QJsonArray kept = d->pages[0]->props.value(QStringLiteral("charts")).toArray();
        QCOMPARE(int(kept.size()), 61);
        QSet<QString> images;
        for (const QJsonValue &v : kept) {
            const Document::Loc at = d->find(v.toObject().value(QStringLiteral("item")).toString());
            QVERIFY(at.item && at.item->type() == ItemType::Picture);
            images << static_cast<const PictureItem *>(at.item)->imageId;
        }
        QCOMPARE(int(images.size()), 1);
        int svgs = 0;
        for (const ImageData &image : std::as_const(d->images)) svgs += image.format == QLatin1String("svg");
        QCOMPARE(svgs, 1);
    }

    // A file may have a thousand diagrams drawn: past that the frames are kept as they are, and the notes say so (each drawing is read, set, and
    // kept as an SVG, and a package may name thousands of parts that each hold one).
    void importLimitsTheDiagramsItDraws()
    {
        const Parts base = smartArtPptx(true);
        QMap<QString, QByteArray> zip;
        QString err;
        QVERIFY2(readZip(pack(base), zip, &err), qPrintable(err));
        // 1001 frames on the slide, each with a data part of its own that names a drawing part of its own (the layout, style, and colors are shared).
        const int count = 1001;
        QString frames, slideRels = QString::fromUtf8(zip.value(QStringLiteral("ppt/slides/_rels/slide1.xml.rels")));
        slideRels.remove(QStringLiteral("</Relationships>"));
        QString rels;
        const QByteArray dataXml = zip.value(QStringLiteral("ppt/diagrams/data1.xml"));
        const QByteArray drawingXml = (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><dsp:drawing xmlns:dsp=\"%1\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"><dsp:spTree>"
                                                      "<dsp:nvGrpSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvGrpSpPr/></dsp:nvGrpSpPr><dsp:grpSpPr/>").arg(dspNamespace())
                                       + diagramShape(diagramPlace(0, 0, 914400, 914400), QStringLiteral("rect"), QStringLiteral("<a:solidFill><a:srgbClr val=\"336699\"/></a:solidFill>"), QString(), diagramPlace(0, 0, 914400, 914400))
                                             .replace(QRegularExpression(QStringLiteral("<dsp:txBody>.*</dsp:txBody>")), QString())
                                       + QStringLiteral("</dsp:spTree></dsp:drawing>")).toUtf8();
        for (int i = 0; i < count; ++i) {
            const int dm = 100 + 2 * i, dr = 101 + 2 * i;
            frames += QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"%1\" name=\"Diagram %1\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr><p:xfrm><a:off x=\"%2\" y=\"914400\"/><a:ext cx=\"3657600\" cy=\"1828800\"/></p:xfrm>"
                                    "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/diagram\"><dgm:relIds xmlns:dgm=\"http://schemas.openxmlformats.org/drawingml/2006/diagram\" r:dm=\"rId%3\" r:lo=\"rId3\" r:qs=\"rId4\" r:cs=\"rId5\"/></a:graphicData></a:graphic></p:graphicFrame>")
                          .arg(10 + i).arg(914400 + i).arg(dm);
            rels += QStringLiteral("<Relationship Id=\"rId%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/diagramData\" Target=\"../diagrams/d%2.xml\"/>"
                                   "<Relationship Id=\"rId%3\" Type=\"%4\" Target=\"../diagrams/w%2.xml\"/>").arg(dm).arg(i).arg(dr).arg(diagramDrawingType());
            zip[QStringLiteral("ppt/diagrams/d%1.xml").arg(i)] = QByteArray(dataXml).replace("relId=\"rId6\"", QStringLiteral("relId=\"rId%1\"").arg(dr).toUtf8());
            zip[QStringLiteral("ppt/diagrams/w%1.xml").arg(i)] = drawingXml;
        }
        zip[QStringLiteral("ppt/slides/_rels/slide1.xml.rels")] = (slideRels + rels + QStringLiteral("</Relationships>")).toUtf8();
        QString slide = QString::fromUtf8(zip.value(QStringLiteral("ppt/slides/slide1.xml")));
        slide.replace(QRegularExpression(QStringLiteral("<p:graphicFrame>.*</p:graphicFrame>")), frames);
        zip[QStringLiteral("ppt/slides/slide1.xml")] = slide.toUtf8();
        QString types = QString::fromUtf8(zip.value(QStringLiteral("[Content_Types].xml")));
        QString more;
        for (int i = 0; i < count; ++i)
            more += QStringLiteral("<Override PartName=\"/ppt/diagrams/d%1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawingml.diagramData+xml\"/>"
                                   "<Override PartName=\"/ppt/diagrams/w%1.xml\" ContentType=\"application/vnd.ms-office.drawingml.diagramDrawing+xml\"/>").arg(i);
        types.replace(QStringLiteral("</Types>"), more + QStringLiteral("</Types>"));
        zip[QStringLiteral("[Content_Types].xml")] = types.toUtf8();
        ZipWriter z;
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) z.add(it.key(), it.value(), true);
        auto d = deck::importPresentation(z.finish(), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages[0]->props.value(QStringLiteral("charts")).toArray().size()), 1000);
        QCOMPARE(int(d->pages[0]->props.value(QStringLiteral("unread")).toArray().size()), 1);
        QVERIFY2(err.contains(QStringLiteral("more than 1000 diagrams")), qPrintable(err));
    }

    // Saved, the diagram is a frame again, with its parts and links in the package as they were: the same names, the same bytes, the links
    // under the ids the file gave them (so what names them, the frame and the data part, is as it was).
    void exportWritesTheKeptDiagramParts()
    {
        const Parts source = smartArtPptx(true);
        QString err;
        auto d = deck::importPresentation(pack(source), &err);
        QVERIFY2(d, qPrintable(err));
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.filter(QStringLiteral("Slide 1")).isEmpty(), qPrintable(skipped.join('\n')));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slidePart = QStringLiteral("/ppt/slides/slide1.xml");
        // The parts, byte for byte, with their content types.
        for (const QString &name : {QStringLiteral("/ppt/diagrams/data1.xml"), QStringLiteral("/ppt/diagrams/layout1.xml"), QStringLiteral("/ppt/diagrams/quickStyle1.xml"),
                                    QStringLiteral("/ppt/diagrams/colors1.xml"), QStringLiteral("/ppt/diagrams/drawing1.xml"), QStringLiteral("/ppt/media/diagram1.png")}) {
            QVERIFY2(e.pkg.hasPart(name) || e.pkg.hasPart(QStringLiteral("/ppt/diagrams/") + name.section(QLatin1Char('/'), -1)), qPrintable(name));
            const QString at = e.pkg.hasPart(name) ? name : QStringLiteral("/ppt/diagrams/") + name.section(QLatin1Char('/'), -1);
            QCOMPARE(e.pkg.part(at), source.data.value(name).xml);
            QCOMPARE(e.pkg.contentType(at), source.data.value(name).type);
        }
        // The slide's links, under the ids the file had: the layout is rId1, then the data (rId2) and on, to the drawing's rId6.
        const QStringList ids = {QStringLiteral("rId2"), QStringLiteral("rId3"), QStringLiteral("rId4"), QStringLiteral("rId5"), QStringLiteral("rId6")};
        const QStringList targets = {QStringLiteral("/ppt/diagrams/data1.xml"), QStringLiteral("/ppt/diagrams/layout1.xml"), QStringLiteral("/ppt/diagrams/quickStyle1.xml"),
                                     QStringLiteral("/ppt/diagrams/colors1.xml"), QStringLiteral("/ppt/diagrams/drawing1.xml")};
        const QStringList types = {QStringLiteral("/diagramData"), QStringLiteral("/diagramLayout"), QStringLiteral("/diagramQuickStyle"), QStringLiteral("/diagramColors"), QStringLiteral("/diagramDrawing")};
        for (int i = 0; i < 5; ++i) {
            const office::Relationship r = e.pkg.relationship(slidePart, ids[i]);
            QCOMPARE(r.target, targets[i]);
            QVERIFY2(r.type.endsWith(types[i]), qPrintable(r.type));
        }
        QCOMPARE(e.pkg.relationship(slidePart, QStringLiteral("rId6")).type, diagramDrawingType());
        // The drawing's own link, to its picture.
        QCOMPARE(int(relationshipTargets(e, QStringLiteral("/ppt/diagrams/drawing1.xml"), QStringLiteral("/image")).size()), 1);
        // The frame as it was: its shape number, its name, its place, and its links to the data, layout, style, and colors.
        QDomDocument dom;
        const QDomElement root = rootOf(e.zip.value(slidePart.mid(1)), &dom);
        const QDomElement frame = frameNamed(root, QStringLiteral("Diagram 8"));
        QVERIFY(!frame.isNull());
        const QDomElement cNvPr = office::child(office::child(frame, office::ns::P, QStringLiteral("nvGraphicFramePr")), office::ns::P, QStringLiteral("cNvPr"));
        QCOMPARE(cNvPr.attribute(QStringLiteral("id")), QStringLiteral("9"));
        QCOMPARE(cNvPr.attribute(QStringLiteral("descr")), QStringLiteral("A process"));
        QCOMPARE(placeOf(office::child(frame, office::ns::P, QStringLiteral("xfrm"))), (QVector<qint64>{914400, 914400, 3657600, 1828800}));
        QVector<QDomElement> relIds;
        collectNamed(frame, QStringLiteral("relIds"), &relIds);
        QCOMPARE(int(relIds.size()), 1);
        QCOMPARE(office::attrNs(relIds[0], office::ns::R, QStringLiteral("dm")), QStringLiteral("rId2"));
        QCOMPARE(office::attrNs(relIds[0], office::ns::R, QStringLiteral("lo")), QStringLiteral("rId3"));
        QCOMPARE(office::attrNs(relIds[0], office::ns::R, QStringLiteral("qs")), QStringLiteral("rId4"));
        QCOMPARE(office::attrNs(relIds[0], office::ns::R, QStringLiteral("cs")), QStringLiteral("rId5"));
        const QString slide = part(e, slidePart);
        // Nothing is left in place of it: no picture, no empty box.
        QVERIFY(!slide.contains(QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"9\"")));
    }

    // When the slide's other links come first, the diagram's are numbered after them: the frame and the data part name the parts by the new ids, and
    // nothing else in the data part changes.
    void exportKeepsDiagramLinksWorkingWhenTheirIdsChange()
    {
        const Parts source = smartArtPptx();
        QString err;
        auto d = deck::importPresentation(pack(source), &err);
        QVERIFY2(d, qPrintable(err));
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slidePart = QStringLiteral("/ppt/slides/slide1.xml");
        const QStringList data = relationshipTargets(e, slidePart, QStringLiteral("/diagramData")), drawing = relationshipTargets(e, slidePart, QStringLiteral("/diagramDrawing"));
        QCOMPARE(int(data.size()), 1);
        QCOMPARE(int(drawing.size()), 1);
        // The data part names the drawing by the link it has now.
        const QString dataXml = QString::fromUtf8(e.pkg.part(data[0]));
        const QRegularExpression named(QStringLiteral("<dsp:dataModelExt\\b[^>]*\\brelId=\"([^\"]*)\""));
        QVERIFY2(named.match(dataXml).hasMatch(), qPrintable(dataXml));
        QCOMPARE(e.pkg.relationship(slidePart, named.match(dataXml).captured(1)).target, drawing[0]);
        QVERIFY(named.match(dataXml).captured(1) != QStringLiteral("rId8"));   // it was rId8 when the slide had its notes and picture linked first
        // And is what it was but for that.
        QString was = QString::fromUtf8(source.data.value(QStringLiteral("/ppt/diagrams/data1.xml")).xml);
        was.replace(QStringLiteral("relId=\"rId8\""), QStringLiteral("relId=\"%1\"").arg(named.match(dataXml).captured(1)));
        QCOMPARE(dataXml, was);
        // The frame names the other three by their new ids.
        const QString slide = part(e, slidePart);
        for (const QString &kind : {QStringLiteral("lo"), QStringLiteral("qs"), QStringLiteral("cs")}) {
            const QRegularExpression ref(QStringLiteral("r:%1=\"([^\"]*)\"").arg(kind));
            QVERIFY2(ref.match(slide).hasMatch(), qPrintable(kind));
            const office::Relationship r = e.pkg.relationship(slidePart, ref.match(slide).captured(1));
            QVERIFY2(r.target.contains(kind == QLatin1String("lo") ? QStringLiteral("layout") : kind == QLatin1String("qs") ? QStringLiteral("quickStyle") : QStringLiteral("colors")), qPrintable(r.target));
        }
        QVERIFY(e.pkg.relationship(slidePart, QRegularExpression(QStringLiteral("r:dm=\"([^\"]*)\"")).match(slide).captured(1)).target == data[0]);
    }

    // A diagram that was moved or resized: its frame goes where the picture is, and only its place changes. One that wasn't is written with the
    // place it had (even a size the reader holds to a slide's).
    void exportPlacesAndDropsDiagrams()
    {
        QString err;
        auto d = deck::importPresentation(pack(smartArtPptx(true)), &err);
        QVERIFY2(d, qPrintable(err));
        auto *pic = const_cast<PictureItem *>(keptPicture(*d, *d->pages[0], QStringLiteral("diagram")));
        QVERIFY(pic);
        pic->rect = QRectF(100, 50, 200, 150);
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY2(slide.contains(QStringLiteral("<p:xfrm><a:off x=\"1270000\" y=\"635000\"/><a:ext cx=\"2540000\" cy=\"1905000\"/></p:xfrm>")), qPrintable(slide));
        // The data, layout, and drawing are as they were: the picture was moved, not the diagram's parts.
        QCOMPARE(e.pkg.part(QStringLiteral("/ppt/diagrams/drawing1.xml")), diagramDrawing());
        // Named again: the name and the alternative text of the picture are the frame's.
        pic->name = QStringLiteral("Process");
        pic->altText = QStringLiteral("Three steps");
        Exported named;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &named, &err), qPrintable(err));
        slide = part(named, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY2(slide.contains(QStringLiteral("<p:cNvPr id=\"9\" name=\"Process\" descr=\"Three steps\"/>")), qPrintable(slide));
        // Taken off the slide, its parts are not written, and nothing points at them.
        auto &items = d->pages[0]->items;
        for (size_t i = 0; i < items.size(); ++i)
            if (items[i].get() == pic) {
                items.erase(items.begin() + i);
                break;
            }
        Exported gone;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &gone, &err), qPrintable(err));
        QVERIFY2(packageProblem(gone).isEmpty(), qPrintable(packageProblem(gone)));
        QVERIFY(!part(gone, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("drawingml/2006/diagram")));
        for (const QString &name : gone.pkg.partNames()) QVERIFY2(!name.contains(QStringLiteral("/diagrams/")), qPrintable(name));
    }

    // The links a frame has to addresses outside the file (a click that opens a page) are not kept, and what they were on stays: none is left pointing
    // at a link the slide no longer has.
    void exportDropsTheHyperlinksOfADiagramFrame()
    {
        QString err;
        Parts p = smartArtPptx(true);
        QString slide = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        slide.replace(QStringLiteral("<p:cNvPr id=\"9\" name=\"Diagram 8\" descr=\"A process\"/>"), QStringLiteral("<p:cNvPr id=\"9\" name=\"Diagram 8\" descr=\"A process\"><a:hlinkClick r:id=\"rId7\"/></p:cNvPr>"));
        p.data["/ppt/slides/slide1.xml"].xml = slide.toUtf8();
        p.rels << Rel{QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("hyperlink"), QStringLiteral("https://example.com/x"), true};
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(keptPicture(*d, *d->pages[0], QStringLiteral("diagram")));
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QVERIFY(!part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("hlinkClick")));
        QDomDocument dom;
        QVERIFY(!frameNamed(rootOf(e.zip.value(QStringLiteral("ppt/slides/slide1.xml")), &dom), QStringLiteral("Diagram 8")).isNull());
    }

    void exportKeepsTheFrameOfAnUnmovedDiagramAsItWas()
    {
        QString err;
        Parts p = smartArtPptx(true);
        // A frame larger than a slide can be is drawn at the largest size, and written as it was while it isn't moved or resized.
        QString slide = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        slide.replace(QStringLiteral("<a:off x=\"914400\" y=\"914400\"/><a:ext cx=\"3657600\" cy=\"1828800\"/>"), QStringLiteral("<a:off x=\"-914400\" y=\"914400\"/><a:ext cx=\"60000000\" cy=\"1828800\"/>"));
        p.data["/ppt/slides/slide1.xml"].xml = slide.toUtf8();
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        auto *pic = const_cast<PictureItem *>(keptPicture(*d, *d->pages[0], QStringLiteral("diagram")));
        QVERIFY(pic);
        QCOMPARE(pic->rect, QRectF(-72, 72, office::kChartMaxSide, 144));
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QDomDocument dom;
        QCOMPARE(placeOf(office::child(frameNamed(rootOf(e.zip.value(QStringLiteral("ppt/slides/slide1.xml")), &dom), QStringLiteral("Diagram 8")), office::ns::P, QStringLiteral("xfrm"))),
                 (QVector<qint64>{-914400, 914400, 60000000, 1828800}));
        // Moved, it is written where it is.
        pic->rect.moveTo(10, 20);
        Exported moved;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &moved, &err), qPrintable(err));
        QVERIFY2(part(moved, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<p:xfrm><a:off x=\"127000\" y=\"254000\"/><a:ext cx=\"51206400\" cy=\"1828800\"/></p:xfrm>")),
                 qPrintable(part(moved, QStringLiteral("/ppt/slides/slide1.xml"))));
    }

    // A duplicated slide keeps its diagram (the copy of the diagram's picture is written as the diagram, once more).
    void duplicatedSlideKeepsItsDiagram()
    {
        QString err;
        auto d = deck::importPresentation(pack(smartArtPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(deck::duplicateSlide(*d, 0), 1);
        QVERIFY(keptPicture(*d, *d->pages[1], QStringLiteral("diagram")));
        QVERIFY(keptPicture(*d, *d->pages[1], QStringLiteral("diagram"))->id != keptPicture(*d, *d->pages[0], QStringLiteral("diagram"))->id);
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        for (const QString &slide : {QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/ppt/slides/slide2.xml")}) {
            QCOMPARE(int(relationshipTargets(e, slide, QStringLiteral("/diagramData")).size()), 1);
            QCOMPARE(int(relationshipTargets(e, slide, QStringLiteral("/diagramDrawing")).size()), 1);
        }
        // Each frame has parts of its own.
        QVERIFY(relationshipTargets(e, QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/diagramData")) != relationshipTargets(e, QStringLiteral("/ppt/slides/slide2.xml"), QStringLiteral("/diagramData")));
    }

    // What was saved reads again as it did: the picture of the diagram, and its parts, for the next save.
    void diagramsSurviveAnotherRoundTrip()
    {
        QString err;
        auto d = deck::importPresentation(pack(smartArtPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const PictureItem *pic = keptPicture(*back, *back->pages[0], QStringLiteral("diagram"));
        QVERIFY(pic);
        QCOMPARE(pic->rect, QRectF(72, 72, 288, 144));
        // The drawing is drawn as it was (but for the number the writer gives a picture fill).
        QString svgWas = QString::fromUtf8(d->images.value(keptPicture(*d, *d->pages[0], QStringLiteral("diagram"))->imageId).bytes), svgNow = QString::fromUtf8(back->images.value(pic->imageId).bytes);
        svgWas.replace(QRegularExpression(QStringLiteral("texpattern_[0-9a-f]+")), QStringLiteral("texpattern"));
        svgNow.replace(QRegularExpression(QStringLiteral("texpattern_[0-9a-f]+")), QStringLiteral("texpattern"));
        QVERIFY(svgWas == svgNow);
        const QJsonObject was = keptFrameOf(*d->pages[0], QStringLiteral("diagram")), now = keptFrameOf(*back->pages[0], QStringLiteral("diagram"));
        QCOMPARE(now.value(QStringLiteral("parts")).toObject().keys(), was.value(QStringLiteral("parts")).toObject().keys());   // the picture in the drawing keeps its name too
        QCOMPARE(keptBytes(now, QStringLiteral("/ppt/diagrams/drawing1.xml")), diagramDrawing());
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*back, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QCOMPARE(e.pkg.part(QStringLiteral("/ppt/diagrams/drawing1.xml")), diagramDrawing());
    }

private:
    // The pair of alternatives in the XML of a slide.
    static QString equationIn(const QString &slide)
    {
        const int at = int(slide.indexOf(QStringLiteral("<mc:AlternateContent")));
        if (at < 0) return QString();
        return slide.mid(at, slide.indexOf(QStringLiteral("</mc:AlternateContent>"), at) + 22 - at);
    }

private Q_SLOTS:
    // An equation is a shape whose text holds math this reader can't draw. The shape beside it, for the programs that don't know math, is
    // a picture of it: that picture is what the equation looks like here, and the whole pair is kept to write back.
    void importReadsAnEquationAsItsPicture()
    {
        for (const QString &form : {QStringLiteral("pic"), QStringLiteral("shape")}) {
            QString err;
            auto d = deck::importPresentation(pack(equationPptx(form)), &err);
            QVERIFY2(d, qPrintable(err));
            const Page &page = *d->pages[0];
            const PictureItem *pic = keptPicture(*d, page, QStringLiteral("equation"));
            QVERIFY2(pic, qPrintable(form));
            QCOMPARE(pic->name, QStringLiteral("TextBox 8"));
            QCOMPARE(pic->rect, QRectF(120, 174, 150, 40));
            QCOMPARE(pic->imgRect, QRectF(0, 0, 150, 40));
            QCOMPARE(d->images.value(pic->imageId).bytes, equationPicture());
            // The picture, and not the shape with the math: no text box is left empty in its place.
            for (const ItemPtr &it : page.items) QVERIFY2(it->name != QStringLiteral("TextBox 8") || it->type() == ItemType::Picture, qPrintable(form));
            QCOMPARE(int(page.items.size()), readerReadsPictures() ? 7 : 6);
            QCOMPARE(int(page.props.value(QStringLiteral("unread")).toArray().size()), 1);
            const QJsonObject kept = keptFrameOf(page, QStringLiteral("equation"));
            QCOMPARE(kept.value(QStringLiteral("item")).toString(), pic->id);
            QVERIFY(kept.value(QStringLiteral("xml")).toString().contains(QStringLiteral("<m:t>E=mc</m:t>")));
            QDomDocument dom;
            QString why;
            QVERIFY2(office::parseXml(kept.value(QStringLiteral("xml")).toString().toUtf8(), &dom, &why), qPrintable(why));
            QCOMPARE(dom.documentElement().localName(), QStringLiteral("AlternateContent"));
        }
    }

    // A text box with other text around the equation is the same pair, and its picture is of the whole box.
    void importReadsATextBoxWithAnEquationAsItsPicture()
    {
        QString err;
        auto d = deck::importPresentation(pack(equationPptx(QStringLiteral("pic"), true)), &err);
        QVERIFY2(d, qPrintable(err));
        const PictureItem *pic = keptPicture(*d, *d->pages[0], QStringLiteral("equation"));
        QVERIFY(pic);
        QCOMPARE(pic->rect, QRectF(120, 174, 150, 40));
        QVERIFY(keptFrameOf(*d->pages[0], QStringLiteral("equation")).value(QStringLiteral("xml")).toString().contains(QStringLiteral("Energy ")));
    }

    // A pair with no picture of the shape to show is read as it was before: the shape of the first alternative that can be read.
    void importReadsAnEquationWithoutAPictureAsAShape()
    {
        QString err;
        Parts p = equationPptx(QStringLiteral("pic"), false, false);
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        xml.replace(QRegularExpression(QStringLiteral("<mc:Fallback>.*?</mc:Fallback>")), QStringLiteral("<mc:Fallback/>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(!keptPicture(*d, *d->pages[0], QStringLiteral("equation")));
        QVERIFY(d->pages[0]->props.value(QStringLiteral("charts")).toArray().isEmpty());
        bool box = false;
        for (const ItemPtr &it : d->pages[0]->items) box = box || (it->name == QStringLiteral("TextBox 8") && it->type() == ItemType::Text);
        QVERIFY(box);
        // And so is a picture that can't be read.
        p = equationPptx();
        p.data["/ppt/media/equation1.png"].xml = "not a picture";
        d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(!keptPicture(*d, *d->pages[0], QStringLiteral("equation")));
    }

    // Saved, the pair is written whole: the shape with its math as it was, and the picture beside it, with its link.
    void exportWritesTheWholeEquation()
    {
        for (const QString &form : {QStringLiteral("pic"), QStringLiteral("shape")}) {
            QString err;
            auto d = deck::importPresentation(pack(equationPptx(form, form == QLatin1String("pic"))), &err);
            QVERIFY2(d, qPrintable(err));
            QStringList skipped;
            Exported e;
            QVERIFY2(unpack(deck::exportPresentation(*d, &err, &skipped), &e, &err), qPrintable(err));
            QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
            QVERIFY2(skipped.filter(QStringLiteral("Slide 1")).isEmpty(), qPrintable(skipped.join('\n')));
            const QString pair = equationIn(part(e, QStringLiteral("/ppt/slides/slide1.xml")));
            QVERIFY2(!pair.isEmpty(), qPrintable(form));
            QCOMPARE(int(part(e, QStringLiteral("/ppt/slides/slide1.xml")).count(QStringLiteral("<mc:AlternateContent"))), 1);
            // The first alternative: asks for a14, and holds the shape with its math.
            QVERIFY2(pair.contains(QStringLiteral("Requires=\"a14\"")), qPrintable(pair));
            QVERIFY(pair.contains(QStringLiteral("xmlns:a14=\"http://schemas.microsoft.com/office/drawing/2010/main\"")));
            QVERIFY(pair.contains(QStringLiteral("<a14:m>")));
            QVERIFY(pair.contains(QStringLiteral("<m:t>E=mc</m:t>")));
            QVERIFY(pair.contains(QStringLiteral("<m:sSup>")));
            QVERIFY(pair.contains(QStringLiteral("typeface=\"Cambria Math\"")));
            QVERIFY(pair.indexOf(QStringLiteral("<mc:Choice")) < pair.indexOf(QStringLiteral("<mc:Fallback")));
            if (form == QLatin1String("pic")) QVERIFY(pair.contains(QStringLiteral("<a:t>Energy </a:t>")) && pair.contains(QStringLiteral("<a:t> is mass</a:t>")));
            // The second: the picture, which the shape's link leads to, written as it was.
            QVERIFY(pair.mid(pair.indexOf(QStringLiteral("<mc:Fallback"))).contains(form == QLatin1String("pic") ? QStringLiteral("<p:pic ") : QStringLiteral("<p:sp ")));
            const QRegularExpression embed(QStringLiteral("r:embed=\"([^\"]*)\""));
            QVERIFY(embed.match(pair).hasMatch());
            const office::Relationship picture = e.pkg.relationship(QStringLiteral("/ppt/slides/slide1.xml"), embed.match(pair).captured(1));
            QVERIFY(picture.type.endsWith(QStringLiteral("/image")));
            QCOMPARE(e.pkg.part(picture.target), equationPicture());
            // Its places in both are the ones it had, with its number and name.
            QDomDocument dom;
            QVERIFY(office::parseXml(pair.toUtf8(), &dom));
            QVector<QDomElement> places, names;
            collectNamed(dom.documentElement(), QStringLiteral("xfrm"), &places);
            collectNamed(dom.documentElement(), QStringLiteral("cNvPr"), &names);
            QCOMPARE(int(places.size()), 2);
            for (const QDomElement &x : std::as_const(places)) QCOMPARE(placeOf(x), (QVector<qint64>{1524000, 2209800, 1905000, 508000}));
            QCOMPARE(int(names.size()), 2);
            for (const QDomElement &n : std::as_const(names)) QVERIFY(n.attribute(QStringLiteral("id")) == QLatin1String("9") && n.attribute(QStringLiteral("name")) == QLatin1String("TextBox 8"));
            // No empty text box stands in for it: the pair is the only thing named so.
            QCOMPARE(int(part(e, QStringLiteral("/ppt/slides/slide1.xml")).count(QStringLiteral("name=\"TextBox 8\""))), 2);
        }
    }

    // Moved or resized, the equation is written where it is, in both alternatives, and nothing else of it changes.
    void exportPlacesBothAlternativesOfAnEquation()
    {
        QString err;
        auto d = deck::importPresentation(pack(equationPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        Exported before;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &before, &err), qPrintable(err));
        auto *pic = const_cast<PictureItem *>(keptPicture(*d, *d->pages[0], QStringLiteral("equation")));
        QVERIFY(pic);
        pic->rect = QRectF(100, 50, 200, 60);
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString pair = equationIn(part(e, QStringLiteral("/ppt/slides/slide1.xml")));
        // Its places in both alternatives (the pair is part of the slide's XML, which declares the prefixes).
        auto places = [](const QString &slide) {
            QDomDocument dom;
            office::parseXml(slide.toUtf8(), &dom);
            QVector<QDomElement> pairs, xfrms;
            collectNamed(dom.documentElement(), QStringLiteral("AlternateContent"), &pairs);
            if (!pairs.isEmpty()) collectNamed(pairs[0], QStringLiteral("xfrm"), &xfrms);
            QVector<QVector<qint64>> out;
            for (const QDomElement &x : std::as_const(xfrms)) out << placeOf(x);
            return out;
        };
        QCOMPARE(places(part(e, QStringLiteral("/ppt/slides/slide1.xml"))), (QVector<QVector<qint64>>{{1270000, 635000, 2540000, 762000}, {1270000, 635000, 2540000, 762000}}));
        // The same but for the places (the XML of the shapes is kept, in the order it has).
        auto withoutPlaces = [](QString xml) { return xml.replace(QRegularExpression(QStringLiteral("<a:xfrm\\b[^>]*>.*?</a:xfrm>")), QStringLiteral("<a:xfrm/>")); };
        QCOMPARE(withoutPlaces(pair), withoutPlaces(equationIn(part(before, QStringLiteral("/ppt/slides/slide1.xml")))));
        QVERIFY(pair.contains(QStringLiteral("<m:t>E=mc</m:t>")));
        // Turned and flipped, it is written so.
        pic->rotation = 30;
        pic->flipH = true;
        Exported turned;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &turned, &err), qPrintable(err));
        const QString turnedPair = equationIn(part(turned, QStringLiteral("/ppt/slides/slide1.xml")));
        QCOMPARE(int(turnedPair.count(QStringLiteral("<a:xfrm rot=\"1800000\" flipH=\"1\"><a:off x=\"1270000\" y=\"635000\"/>"))), 2);
        // Taken off the slide, none of it is written.
        auto &items = d->pages[0]->items;
        for (size_t i = 0; i < items.size(); ++i)
            if (items[i].get() == pic) {
                items.erase(items.begin() + i);
                break;
            }
        Exported gone;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &gone, &err), qPrintable(err));
        QVERIFY2(packageProblem(gone).isEmpty(), qPrintable(packageProblem(gone)));
        QVERIFY(equationIn(part(gone, QStringLiteral("/ppt/slides/slide1.xml"))).isEmpty());
        QVERIFY(!part(gone, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("TextBox 8")));
    }

    // A duplicated slide keeps its equation, and what was saved reads again as it did.
    void equationsSurviveDuplicatingAndAnotherRoundTrip()
    {
        QString err;
        auto d = deck::importPresentation(pack(equationPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(deck::duplicateSlide(*d, 0), 1);
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        for (const QString &slide : {QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/ppt/slides/slide2.xml")}) QVERIFY2(part(e, slide).contains(QStringLiteral("<m:t>E=mc</m:t>")), qPrintable(slide));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        for (int i = 0; i < 2; ++i) {
            const PictureItem *pic = keptPicture(*back, *back->pages[i], QStringLiteral("equation"));
            QVERIFY(pic);
            QCOMPARE(pic->rect, QRectF(120, 174, 150, 40));
            QCOMPARE(back->images.value(pic->imageId).bytes, equationPicture());
        }
    }

    void importReadsNotes()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(deck::notesText(*d, *d->pages[0]), QStringLiteral("Say hello first\nThen the numbers"));
        // A slide without notes has an empty story to type in.
        QVERIFY(!d->pages[1]->notesStoryId.isEmpty());
        QVERIFY(d->storyDoc(d->pages[1]->notesStoryId));
        QVERIFY(deck::notesText(*d, *d->pages[1]).isEmpty());
        QVERIFY(d->pages[0]->notesStoryId != d->pages[1]->notesStoryId);
    }

    // A chart that many frames show is read and drawn once for each size and
    // look: 120 frames of one chart of 8,000 points take the time of one.
    void importDrawsAChartOnceForTheFramesThatShowIt()
    {
        Parts p = chartPptx();
        QString points;
        for (int i = 0; i < 8000; ++i) points += QStringLiteral("<c:pt idx=\"%1\"><c:v>%2</c:v></c:pt>").arg(i).arg(i % 97);
        QString chart = QString::fromUtf8(p.data["/ppt/charts/chart1.xml"].xml);
        chart.replace(QRegularExpression(QStringLiteral("<c:ptCount val=\"3\"/><c:pt idx=\"0\"><c:v>10</c:v>.*?</c:numCache>")),
                      QStringLiteral("<c:ptCount val=\"8000\"/>") + points + QStringLiteral("</c:numCache>"));
        QVERIFY(chart.contains(QStringLiteral("val=\"8000\"")));
        p.data["/ppt/charts/chart1.xml"].xml = chart.toUtf8();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        const qsizetype named = xml.indexOf(QStringLiteral("name=\"Chart 8\""));
        const qsizetype from = xml.lastIndexOf(QStringLiteral("<p:graphicFrame>"), named), to = xml.indexOf(QStringLiteral("</p:graphicFrame>"), named) + 17;
        const QString frame = xml.mid(from, to - from);
        QString frames;
        for (int i = 0; i < 120; ++i) frames += QString(frame).replace(QStringLiteral("id=\"9\""), QStringLiteral("id=\"%1\"").arg(100 + i));
        xml.replace(frame, frames);
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QElapsedTimer timer;
        timer.start();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        const qint64 ms = timer.elapsed();
        QVERIFY2(d, qPrintable(err));
        QSet<QString> ids;
        int charts = 0;
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Picture && it->name == QLatin1String("Chart 8")) {
                ++charts;
                ids.insert(static_cast<const PictureItem *>(it.get())->imageId);
            }
        QCOMPARE(charts, 120);
        QCOMPARE(int(ids.size()), 1);
        QVERIFY2(ms < 3000, qPrintable(QStringLiteral("took %1 ms").arg(ms)));
    }

    // A chart on a slide is a picture of it (an SVG drawn from the values the file stores), in the frame's place, and its parts
    // (the chart, the workbook behind it, and what they link to) are kept with the slide to be written back.
    void importReadsChartsAsPictures()
    {
        QString err;
        auto d = deck::importPresentation(pack(chartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const Page &page = *d->pages[0];
        const auto *pic = chartPicture(*d, page);
        QVERIFY(pic);
        QCOMPARE(pic->name, QStringLiteral("Chart 8"));
        QCOMPARE(pic->altText, QStringLiteral("Sales by region"));
        QCOMPARE(pic->rect, QRectF(72, 72, 288, 216));
        QCOMPARE(pic->imgRect, QRectF(0, 0, 288, 216));
        QVERIFY(d->images.contains(pic->imageId));
        QCOMPARE(d->images.value(pic->imageId).format, QStringLiteral("svg"));
        QVERIFY(d->images.value(pic->imageId).bytes.contains("<text"));
        QVERIFY(d->images.value(pic->imageId).bytes.contains(">East<"));
        QVERIFY(!d->images.value(pic->imageId).image().isNull());
        // The frame the unknown kind of frame was kept as is still the only one left unread.
        QCOMPARE(int(page.props.value(QStringLiteral("unread")).toArray().size()), 1);
        // What is kept for the picture.
        const QJsonArray charts = page.props.value(QStringLiteral("charts")).toArray();
        QCOMPARE(int(charts.size()), 1);
        const QJsonObject kept = charts[0].toObject();
        QCOMPARE(kept.value(QStringLiteral("item")).toString(), pic->id);
        QCOMPARE(kept.value(QStringLiteral("main")).toString(), QStringLiteral("/ppt/charts/chart1.xml"));
        QDomDocument dom;
        QString why;
        QVERIFY2(office::parseXml(kept.value(QStringLiteral("frameXml")).toString().toUtf8(), &dom, &why), qPrintable(why));
        QVERIFY(office::isElement(dom.documentElement(), office::ns::P, QStringLiteral("graphicFrame")));
        QVERIFY(office::isChartFrame(dom.documentElement()));
        const QJsonObject parts = kept.value(QStringLiteral("parts")).toObject();
        QCOMPARE(parts.keys(), (QStringList{"/ppt/charts/chart1.xml", "/ppt/drawings/drawing1.xml", "/ppt/embeddings/Sheet1.xlsx", "/ppt/media/image1.png", "/ppt/theme/themeOverride1.xml"}));
        const Parts source = chartPptx();
        for (auto it = source.data.cbegin(); it != source.data.cend(); ++it) {
            if (!parts.contains(it.key())) continue;
            const QJsonObject one = parts.value(it.key()).toObject();
            QCOMPARE(QByteArray::fromBase64(one.value(QStringLiteral("data")).toString().toLatin1()), it.value().xml);
            QCOMPARE(one.value(QStringLiteral("type")).toString(), it.value().type);
        }
        const QJsonArray rels = parts.value(QStringLiteral("/ppt/charts/chart1.xml")).toObject().value(QStringLiteral("rels")).toArray();
        QCOMPARE(int(rels.size()), 3);
        QCOMPARE(rels[0].toObject().value(QStringLiteral("id")).toString(), QStringLiteral("rId1"));
        QVERIFY(rels[0].toObject().value(QStringLiteral("type")).toString().endsWith(QStringLiteral("/package")));
        QCOMPARE(rels[0].toObject().value(QStringLiteral("target")).toString(), QStringLiteral("/ppt/embeddings/Sheet1.xlsx"));
    }

    // A chart that can't be read is kept as the frame it is, like any other frame the reader can't read.
    void importLeavesACorruptChartUnread()
    {
        Parts p = chartPptx();
        p.data["/ppt/charts/chart1.xml"].xml = "<c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"/>";
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(!chartPicture(*d, *d->pages[0]));
        QCOMPARE(int(d->pages[0]->props.value(QStringLiteral("unread")).toArray().size()), 2);
        QVERIFY(!d->pages[0]->props.contains(QStringLiteral("charts")));
    }

    // Saved, the chart is a frame again, with its parts and links in the package: the chart, its workbook, and what they relate to.
    void exportWritesTheKeptChartParts()
    {
        QString err;
        auto d = deck::importPresentation(pack(chartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QVERIFY(!skipped.contains(chartPicture(*d, *d->pages[0])->id));   // the chart is written, not skipped
        const QString slidePart = QStringLiteral("/ppt/slides/slide1.xml");
        const QStringList chartParts = relationshipTargets(e, slidePart, QStringLiteral("/chart"));
        QCOMPARE(int(chartParts.size()), 1);
        // The frame's r:id is that link.
        QDomDocument dom;
        const QDomElement root = rootOf(e.zip.value(slidePart.mid(1)), &dom);
        QDomElement chart;
        for (QDomElement frame : office::children(office::child(office::child(root, office::ns::P, QStringLiteral("cSld")), office::ns::P, QStringLiteral("spTree")), office::ns::P, QStringLiteral("graphicFrame")))
            if (office::isChartFrame(frame)) chart = frame;
        QVERIFY(!chart.isNull());
        const office::Relationship link = e.pkg.relationship(slidePart, office::chartFrameRelId(chart));
        QCOMPARE(link.target, chartParts[0]);
        QCOMPARE(e.pkg.contentType(chartParts[0]), QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"));
        // The chart as it was, its workbook and drawing and theme beside it, each reached from the chart as before.
        const Parts source = chartPptx();
        QCOMPARE(e.pkg.part(chartParts[0]), source.data.value(QStringLiteral("/ppt/charts/chart1.xml")).xml);
        const QStringList workbook = relationshipTargets(e, chartParts[0], QStringLiteral("/package"));
        QCOMPARE(int(workbook.size()), 1);
        QCOMPARE(e.pkg.part(workbook[0]), source.data.value(QStringLiteral("/ppt/embeddings/Sheet1.xlsx")).xml);
        QCOMPARE(e.pkg.contentType(workbook[0]), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
        const QStringList shapes = relationshipTargets(e, chartParts[0], QStringLiteral("/chartUserShapes"));
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(e.pkg.part(shapes[0]), source.data.value(QStringLiteral("/ppt/drawings/drawing1.xml")).xml);
        const QStringList pictures = relationshipTargets(e, shapes[0], QStringLiteral("/image"));
        QCOMPARE(int(pictures.size()), 1);
        QCOMPARE(e.pkg.part(pictures[0]), tinyPng());
        QCOMPARE(int(relationshipTargets(e, chartParts[0], QStringLiteral("/themeOverride")).size()), 1);
        // The chart's own links keep the ids its XML names (externalData r:id="rId1", userShapes r:id="rId2").
        QCOMPARE(e.pkg.relationship(chartParts[0], QStringLiteral("rId1")).target, workbook[0]);
        QCOMPARE(e.pkg.relationship(chartParts[0], QStringLiteral("rId2")).target, shapes[0]);
        // Its place: the frame keeps its shape number, and no other shape on the slide takes it. It stands where the picture does among the shapes.
        QCOMPARE(chart.firstChildElement().firstChildElement().attribute(QStringLiteral("id")), QStringLiteral("9"));
        QSet<QString> ids;
        QStringList all;
        QVector<QDomElement> stack{root};
        while (!stack.isEmpty()) {
            const QDomElement el = stack.takeLast();
            if (el.localName() == QLatin1String("cNvPr")) {
                QVERIFY2(!ids.contains(el.attribute(QStringLiteral("id"))), qPrintable(el.attribute(QStringLiteral("id"))));
                ids << el.attribute(QStringLiteral("id"));
            }
            for (QDomElement c = el.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack << c;
        }
        const QString slide = part(e, slidePart);
        QVERIFY(slide.indexOf(QStringLiteral("name=\"Rectangle 5\"")) < slide.indexOf(QStringLiteral("name=\"Chart 8\"")));
        // Every part of the package is reached, and the first slide's other frames are as before.
        QVERIFY(slide.contains(QStringLiteral("Cell &amp; &lt;1&gt;")));
    }

    // A chart that was moved, resized, or deleted: its frame goes where the picture is, or is not written at all.
    // A duplicated slide keeps its chart (the copy of the chart's picture is
    // written as the chart, once more) and the animations that aim at its
    // shapes by number.
    void duplicatedSlideKeepsItsChartAndAnimations()
    {
        QString err;
        auto d = deck::importPresentation(pack(chartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const Item *chart = chartPicture(*d, *d->pages[0]);
        QVERIFY(chart);
        // The chart's frame keeps its number from the file; an animation aims at that.
        const QString frame = d->pages[0]->props.value(QStringLiteral("charts")).toArray().at(0).toObject().value(QStringLiteral("frameXml")).toString();
        const int number = QRegularExpression(QStringLiteral("cNvPr\\b[^>]*\\bid=\"(\\d+)\"")).match(frame).captured(1).toInt();
        QVERIFY(number >= 2);
        d->pages[0]->props[QStringLiteral("timingXml")] =
            QStringLiteral("<p:timing xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" nodeType=\"tmRoot\"><p:childTnLst>"
                           "<p:set><p:cBhvr><p:cTn id=\"2\"/><p:tgtEl><p:spTgt spid=\"%1\"/></p:tgtEl></p:cBhvr></p:set></p:childTnLst></p:cTn></p:par></p:tnLst></p:timing>").arg(number);
        QCOMPARE(deck::duplicateSlide(*d, 0), 1);
        const Item *copy = chartPicture(*d, *d->pages[1]);
        QVERIFY(copy && copy->id != chart->id);
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        for (const QString &slide : {QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/ppt/slides/slide2.xml")}) {
            QCOMPARE(int(relationshipTargets(e, slide, QStringLiteral("/chart")).size()), 1);
            QVERIFY2(part(e, slide).contains(QStringLiteral("<p:spTgt spid=\"%1\"/>").arg(number)), qPrintable(slide));
        }
        // Two charts, each with parts of its own.
        QVERIFY(relationshipTargets(e, QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/chart"))
                != relationshipTargets(e, QStringLiteral("/ppt/slides/slide2.xml"), QStringLiteral("/chart")));
        QVERIFY2(skipped.filter(QStringLiteral("Slide 2")).isEmpty(), qPrintable(skipped.join('\n')));
    }

    void exportPlacesAndDropsCharts()
    {
        QString err;
        auto d = deck::importPresentation(pack(chartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        auto *pic = const_cast<PictureItem *>(chartPicture(*d, *d->pages[0]));
        QVERIFY(pic);
        pic->rect = QRectF(100, 50, 200, 150);
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY2(slide.contains(QStringLiteral("<a:off x=\"1270000\" y=\"635000\"/><a:ext cx=\"2540000\" cy=\"1905000\"/>")), qPrintable(slide.mid(slide.indexOf(QStringLiteral("Chart 8")), 600)));
        QCOMPARE(int(relationshipTargets(e, QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/chart")).size()), 1);
        // Taken off the slide, its kept parts are not written, and nothing points at them.
        auto &items = d->pages[0]->items;
        for (size_t i = 0; i < items.size(); ++i)
            if (items[i].get() == pic) {
                items.erase(items.begin() + i);
                break;
            }
        Exported gone;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &gone, &err), qPrintable(err));
        QVERIFY2(packageProblem(gone).isEmpty(), qPrintable(packageProblem(gone)));
        QVERIFY(!part(gone, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("drawingml/2006/chart")));
        for (const QString &name : gone.pkg.partNames()) {
            QVERIFY2(!name.contains(QStringLiteral("/charts/")), qPrintable(name));
            QVERIFY2(!name.contains(QStringLiteral("/embeddings/")), qPrintable(name));
        }
    }

    // What was saved reads again as it did: the picture of the chart, and its parts, for the next save.
    void chartsSurviveAnotherRoundTrip()
    {
        QString err;
        auto d = deck::importPresentation(pack(chartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const auto *pic = chartPicture(*back, *back->pages[0]);
        QVERIFY(pic);
        QCOMPARE(pic->rect, QRectF(72, 72, 288, 216));
        QCOMPARE(back->images.value(pic->imageId).format, QStringLiteral("svg"));
        const QJsonObject was = d->pages[0]->props.value(QStringLiteral("charts")).toArray()[0].toObject().value(QStringLiteral("parts")).toObject();
        const QJsonObject now = back->pages[0]->props.value(QStringLiteral("charts")).toArray()[0].toObject().value(QStringLiteral("parts")).toObject();
        QCOMPARE(int(now.size()), int(was.size()));
        // The workbook's bytes come back whole.
        QByteArray workbook;
        for (auto it = now.begin(); it != now.end(); ++it)
            if (it.key().endsWith(QStringLiteral(".xlsx"))) workbook = QByteArray::fromBase64(it.value().toObject().value(QStringLiteral("data")).toString().toLatin1());
        QCOMPARE(workbook, chartPptx().data.value(QStringLiteral("/ppt/embeddings/Sheet1.xlsx")).xml);
        // And once more.
        const QByteArray again = deck::exportPresentation(*back, &err);
        QVERIFY2(!again.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(again, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
    }

    // The parts a chart keeps are written under names no other part has, and its links keep working whatever their ids were.
    void exportRenamesChartPartsAndKeepsLinksWorking()
    {
        // The chart's links are called R9 and x7 in the file; and a second frame on the slide shows the same chart (its parts are kept for each).
        QMap<QString, QByteArray> zip;
        QString err;
        QVERIFY2(readZip(pack(chartPptx()), zip, &err), qPrintable(err));
        auto swap = [&](const QString &name, const QString &from, const QString &to) {
            QString text = QString::fromUtf8(zip.value(name));
            QVERIFY2(text.contains(from), qPrintable(name + " " + from));
            zip[name] = text.replace(from, to).toUtf8();
        };
        swap(QStringLiteral("ppt/charts/chart1.xml"), QStringLiteral("r:id=\"rId1\""), QStringLiteral("r:id=\"R9\""));
        swap(QStringLiteral("ppt/charts/chart1.xml"), QStringLiteral("r:id=\"rId2\""), QStringLiteral("r:id=\"x7\""));
        QString rels = QString::fromUtf8(zip.value(QStringLiteral("ppt/charts/_rels/chart1.xml.rels")));
        rels.replace(QStringLiteral("Id=\"rId1\""), QStringLiteral("Id=\"R9\"")).replace(QStringLiteral("Id=\"rId2\""), QStringLiteral("Id=\"x7\"")).replace(QStringLiteral("Id=\"rId3\""), QStringLiteral("Id=\"a1\""));
        zip[QStringLiteral("ppt/charts/_rels/chart1.xml.rels")] = rels.toUtf8();
        QString slide = QString::fromUtf8(zip.value(QStringLiteral("ppt/slides/slide1.xml")));
        const int at = int(slide.indexOf(QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"9\"")));
        const int end = int(slide.indexOf(QStringLiteral("</p:graphicFrame>"), at)) + 17;
        slide.insert(end, QString(slide.mid(at, end - at)).replace(QStringLiteral("id=\"9\""), QStringLiteral("id=\"10\"")).replace(QStringLiteral("Chart 8"), QStringLiteral("Chart 9")));
        zip[QStringLiteral("ppt/slides/slide1.xml")] = slide.toUtf8();
        ZipWriter z;
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) z.add(it.key(), it.value(), true);
        auto d = deck::importPresentation(z.finish(), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages[0]->props.value(QStringLiteral("charts")).toArray().size()), 2);

        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QStringList charts = relationshipTargets(e, QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/chart"));
        QCOMPARE(int(charts.size()), 2);
        QVERIFY(charts[0].compare(charts[1], Qt::CaseInsensitive) != 0);
        for (const QString &chart : charts) {
            // Its XML names its workbook and its drawing by the ids it has now.
            const QString xml = QString::fromUtf8(e.pkg.part(chart));
            const QRegularExpression ext(QStringLiteral("<c:externalData r:id=\"([^\"]*)\""));
            const QRegularExpression shapes(QStringLiteral("<c:userShapes r:id=\"([^\"]*)\""));
            QVERIFY2(ext.match(xml).hasMatch(), qPrintable(xml));
            QVERIFY(shapes.match(xml).hasMatch());
            const office::Relationship workbook = e.pkg.relationship(chart, ext.match(xml).captured(1)), drawing = e.pkg.relationship(chart, shapes.match(xml).captured(1));
            QVERIFY(workbook.type.endsWith(QStringLiteral("/package")));
            QVERIFY(drawing.type.endsWith(QStringLiteral("/chartUserShapes")));
            QCOMPARE(e.pkg.part(workbook.target), chartPptx().data.value(QStringLiteral("/ppt/embeddings/Sheet1.xlsx")).xml);
        }
        // Each frame has its own link, and every part is written once under its own name.
        QStringList names = e.pkg.partNames();
        for (QString &n : names) n = n.toLower();
        QCOMPARE(int(QSet<QString>(names.begin(), names.end()).size()), int(names.size()));
    }
};

JP_TEST_CLASS(DeckPptxObjectsTests)
#include "tst_deck_pptxobjects.moc"
