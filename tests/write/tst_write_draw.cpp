// Tests of JeffWrite's drawn objects: the shapes and text boxes a file holds and the ones Insert puts in, and the Picture Format and Shape Format commands. Each document is
// built here from a few lines of XML.

#include "../isolatesettings.h"
#include "core/appinfo.h"
#include "app/writeinfo.h"
#include "core/drawnitems.h"
#include "core/fonts.h"
#include "core/items.h"
#include "core/pictureops.h"
#include "core/writedoc.h"
#include "office/drawingml.h"
#include "office/drawingml_geometry.h"
#include "office/package.h"
#include "wml/wml.h"
#include "wml/xmltree.h"

#include <QTest>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp;
using namespace jp::write;

namespace {

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                   "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                   "xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"";

QByteArray makeDocx(const QByteArray &body)
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/word/document.xml"),
              QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    return w.finish();
}

QByteArray documentXml(const QByteArray &zip)
{
    office::Package p;
    p.read(zip);
    return p.part(QStringLiteral("/word/document.xml"));
}

std::unique_ptr<WriteDoc> open(const QByteArray &zip)
{
    QString err;
    auto doc = WriteDoc::fromDocx(zip, &err);
    if (!doc) qWarning("open: %s", qPrintable(err));
    return doc;
}

std::unique_ptr<WriteDoc> textDoc(const QStringList &paragraphs)
{
    QByteArray body;
    for (const QString &p : paragraphs) body += "<w:p><w:r><w:t xml:space=\"preserve\">" + p.toUtf8() + "</w:t></w:r></w:p>";
    return open(makeDocx(body));
}

// The items of the first page that are objects set at places.
ItemList objectsOfPage(WriteDoc &doc)
{
    ItemList out;
    const auto page = doc.pager().materialize(0);
    if (!page) return out;
    for (const auto &it : page->doc->pages[0]->items)
        if (it->id.startsWith(QLatin1String("flow-object-"))) out.push_back(it);
    return out;
}

// A drawing of a shape of the drawing format's own, set at a place: `geometry` and `look` are the inside of its a:spPr after the frame.
QByteArray shapeDrawing(const QByteArray &geometry, const QByteArray &look, int cx = 1270000, int cy = 635000, const QByteArray &extra = QByteArray())
{
    return "<w:p><w:r><w:drawing><wp:anchor distT=\"0\" distB=\"0\" distL=\"114300\" distR=\"114300\" simplePos=\"0\" relativeHeight=\"251659264\" behindDoc=\"0\" locked=\"0\" layoutInCell=\"1\" allowOverlap=\"1\">"
           "<wp:simplePos x=\"0\" y=\"0\"/><wp:positionH relativeFrom=\"page\"><wp:posOffset>1270000</wp:posOffset></wp:positionH><wp:positionV relativeFrom=\"page\"><wp:posOffset>1270000</wp:posOffset></wp:positionV>"
           "<wp:extent cx=\"" + QByteArray::number(cx) + "\" cy=\"" + QByteArray::number(cy) + "\"/><wp:wrapNone/><wp:docPr id=\"1\" name=\"Shape 1\"/><wp:cNvGraphicFramePr/>"
           "<a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"><wps:wsp><wps:cNvSpPr/>"
           "<wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + QByteArray::number(cx) + "\" cy=\"" + QByteArray::number(cy) + "\"/></a:xfrm>" + geometry + look + "</wps:spPr>" + extra +
           "<wps:bodyPr/></wps:wsp></a:graphicData></a:graphic></wp:anchor></w:drawing></w:r></w:p>";
}

} // namespace

class DrawTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void cleanupTestCase() { QVERIFY(jptest::settingsStayInTheRunsFolder()); }

    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        initCore();
    }

    // ---------------------------------------------------------------- shapes a file holds
    void aShapeOfTheDrawingFormatIsDrawnWithItsGeometryFillAndLine()
    {
        auto doc = open(makeDocx(shapeDrawing("<a:prstGeom prst=\"star5\"><a:avLst/></a:prstGeom>",
                                              "<a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill><a:ln w=\"25400\"><a:solidFill><a:srgbClr val=\"0000FF\"/></a:solidFill></a:ln>")
                                 + "<w:p><w:r><w:t>Text</w:t></w:r></w:p>"));
        QVERIFY(doc);
        const ItemList items = objectsOfPage(*doc);
        QCOMPARE(items.size(), 1);
        QCOMPARE(items[0]->type(), ItemType::Shape);
        const auto *shape = static_cast<const ShapeItem *>(items[0].get());
        QCOMPARE(shape->shape, QStringLiteral("star5"));
        QVERIFY(!shape->fill.isNone());
        QCOMPARE(shape->fill.color.rgbValue(), QColor(255, 0, 0));
        QVERIFY(!shape->stroke.isNone());
        QCOMPARE(shape->stroke.width, 2.0);
        QCOMPARE(items[0]->rect.size(), QSizeF(100, 50));
    }

    // ---------------------------------------------------------------- shapes put in
    void everyShapeOfTheGalleryIsPutInDrawnAndSavedAsTheFileFormatsOwn()
    {
        for (const ShapeGroup &group : shapeGallery())
            for (const QString &preset : group.presets) {
                auto doc = textDoc({"Before after"});
                QTextCursor c(doc->story());
                c.setPosition(7);
                QCOMPARE(insertShape(*doc, c, preset), PictureResult::Ok);
                const PictureInfo info = pictureAt(*doc, doc->story(), 7);
                QVERIFY2(info.valid && info.editable, qPrintable(preset));
                QVERIFY(info.kind == DrawnKind::Shape || info.kind == DrawnKind::Line);
                const ItemList items = objectsOfPage(*doc);
                QVERIFY2(items.size() == 1, qPrintable(preset));
                if (info.kind == DrawnKind::Shape) {
                    // The shape is the one asked for: drawn by the library's own, or by paths of its own geometry.
                    QCOMPARE(items[0]->type(), ItemType::Shape);
                    const auto *shape = static_cast<const ShapeItem *>(items[0].get());
                    QVERIFY2(shape->shape == preset || shape->shape != QLatin1String("rect") || !shape->customPath.isEmpty() || preset == QLatin1String("rect"), qPrintable(preset));
                }
                const QByteArray saved = doc->toDocx();
                office::wml::XmlTree tree;
                QVERIFY2(tree.parse(documentXml(saved)), qPrintable(preset));
                QVERIFY(documentXml(saved).contains("prst=\"" + preset.toLatin1() + "\""));
                QVERIFY(documentXml(saved).contains("xmlns:wps="));
                auto back = open(saved);
                QVERIFY(back);
                QVERIFY2(pictureAt(*back, back->story(), 7).valid, qPrintable(preset));
                QCOMPARE(pictureAt(*back, back->story(), 7).preset, info.preset);
            }
    }

    void everyShapeOfTheGalleryIsOneTheDrawingCodeHasAnOutlineFor()
    {
        int count = 0;
        for (const ShapeGroup &group : shapeGallery())
            for (const QString &preset : group.presets) {
                ++count;
                if (preset == QLatin1String("line") || preset == QLatin1String("straightConnector1")) continue;
                QVERIFY2(!office::shapeForPreset(preset).isEmpty() || office::hasPresetGeometry(preset), qPrintable(preset));
            }
        QVERIFY(count >= 60);
    }

    void aShapeIsOneStepThatUndoesAndHasItsLookAndSize()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        ShapeLook look;
        look.fill = QStringLiteral("FF8800");
        look.line = QStringLiteral("112233");
        look.weight = 3;
        QCOMPARE(insertShape(*doc, c, QStringLiteral("star5"), QSizeF(90, 80), look), PictureResult::Ok);
        QVERIFY(doc->story()->toPlainText().contains(QChar::ObjectReplacementCharacter));
        PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QCOMPARE(info.preset, QStringLiteral("star5"));
        QCOMPARE(info.fillColor, QStringLiteral("FF8800"));
        QCOMPARE(info.lineColor, QStringLiteral("112233"));
        QCOMPARE(info.lineWeight, 3.0);
        QCOMPARE(info.width, 90.0);
        QCOMPARE(info.height, 80.0);
        QVERIFY(info.floating);
        QVERIFY(doc->undo());
        QVERIFY(!isDrawnObjectAt(doc->story(), 7));
        QCOMPARE(doc->story()->toPlainText(), QStringLiteral("Before after"));
        QVERIFY(doc->redo());
        QVERIFY(isDrawnObjectAt(doc->story(), 7));
        QVERIFY(!isPictureAt(doc->story(), 7));
        // Not a shape the gallery has; and not in a note or header.
        QTextCursor d(doc->story());
        QCOMPARE(insertShape(*doc, d, QStringLiteral("nonsense")), PictureResult::Unreadable);
    }

    void aShapesFillLineTurnAndKindAreChangedOneStepEach()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        QVERIFY(setShapeFill(*doc, 7, QStringLiteral("00AA00")));
        QVERIFY(!setShapeFill(*doc, 7, QStringLiteral("00AA00")));
        QCOMPARE(pictureAt(*doc, doc->story(), 7).fillColor, QStringLiteral("00AA00"));
        QVERIFY(doc->undo());
        QCOMPARE(pictureAt(*doc, doc->story(), 7).fillColor, QStringLiteral("3B78B8"));
        QVERIFY(doc->redo());
        QVERIFY(setShapeFill(*doc, 7, QString()));
        QVERIFY(pictureAt(*doc, doc->story(), 7).fillColor.isEmpty());
        QVERIFY(!setShapeFill(*doc, 7, QStringLiteral("not a color")));
        QVERIFY(setShapeLine(*doc, 7, QStringLiteral("CC0000"), 4.5, QStringLiteral("dash")));
        PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QCOMPARE(info.lineColor, QStringLiteral("CC0000"));
        QCOMPARE(info.lineWeight, 4.5);
        QCOMPARE(info.lineDash, QStringLiteral("dash"));
        QVERIFY(!setShapeLine(*doc, 7, QStringLiteral("CC0000"), 4.5, QStringLiteral("dash")));
        QVERIFY(setShapeLine(*doc, 7, QString(), 0, QStringLiteral("solid")));
        QVERIFY(pictureAt(*doc, doc->story(), 7).lineColor.isEmpty());
        QVERIFY(setShapeRotation(*doc, 7, 45));
        QVERIFY(std::abs(pictureAt(*doc, doc->story(), 7).rotation - 45) < 0.01);
        QVERIFY(!setShapeRotation(*doc, 7, 405));
        QVERIFY(setShapeRotation(*doc, 7, -90));
        QVERIFY(std::abs(pictureAt(*doc, doc->story(), 7).rotation - 270) < 0.01);
        QVERIFY(setShapePreset(*doc, 7, QStringLiteral("rightArrow")));
        QCOMPARE(pictureAt(*doc, doc->story(), 7).preset, QStringLiteral("rightArrow"));
        QVERIFY(!setShapePreset(*doc, 7, QStringLiteral("line")));
        // What was set is in the saved file, in the order the format wants, and reads back.
        const QByteArray saved = doc->toDocx();
        const QByteArray x = documentXml(saved);
        office::wml::XmlTree tree;
        QVERIFY(tree.parse(x));
        const qsizetype xfrm = x.indexOf("<a:xfrm"), geom = x.indexOf("<a:prstGeom"), fill = x.indexOf("<a:noFill/>", geom), ln = x.indexOf("<a:ln", geom);
        QVERIFY(xfrm > 0 && geom > xfrm && fill > geom && ln > fill);
        QVERIFY(x.contains("rot=\"16200000\""));
        auto back = open(saved);
        const PictureInfo again = pictureAt(*back, back->story(), 7);
        QCOMPARE(again.preset, QStringLiteral("rightArrow"));
        QVERIFY(again.fillColor.isEmpty() && again.lineColor.isEmpty());
        QVERIFY(std::abs(again.rotation - 270) < 0.01);
        // A picture takes a line (its border) and no fill.
        QVERIFY(!setShapeFill(*doc, 0, QStringLiteral("00AA00")));
    }

    // ---------------------------------------------------------------- text boxes
    void aTextBoxIsPutInHoldsItsWordsAndTheyAreChangedAsOneStep()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        const TextBoxDesign &simple = textBoxDesigns().first();
        QCOMPARE(insertTextBox(*doc, c, simple), PictureResult::Ok);
        PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QVERIFY(info.valid && info.editable);
        QCOMPARE(info.kind, DrawnKind::TextBox);
        QCOMPARE(info.width, 216.0);
        QCOMPARE(info.height, 72.0);
        QCOMPARE(textBoxText(*doc, 7).value(), QStringList{QString()});
        QVERIFY(setTextBoxText(*doc, 7, {QStringLiteral("First line"), QStringLiteral("Second <line> & more")}));
        QVERIFY(!setTextBoxText(*doc, 7, {QStringLiteral("First line"), QStringLiteral("Second <line> & more")}));
        QCOMPARE(textBoxText(*doc, 7).value(), (QStringList{QStringLiteral("First line"), QStringLiteral("Second <line> & more")}));
        QVERIFY(doc->undo());
        QCOMPARE(textBoxText(*doc, 7).value(), QStringList{QString()});
        QVERIFY(doc->redo());
        // The box on the page holds the words.
        const auto page = doc->pager().materialize(0);
        QVERIFY(page);
        bool shown = false;
        for (const auto &story : page->doc->stories)
            if (story->doc->toPlainText().contains(QStringLiteral("First line"))) shown = true;
        QVERIFY(shown);
        const QByteArray saved = doc->toDocx();
        office::wml::XmlTree tree;
        QVERIFY(tree.parse(documentXml(saved)));
        QVERIFY(documentXml(saved).contains("txBox=\"1\""));
        auto back = open(saved);
        QCOMPARE(pictureAt(*back, back->story(), 7).kind, DrawnKind::TextBox);
        QCOMPARE(textBoxText(*back, 7).value(), (QStringList{QStringLiteral("First line"), QStringLiteral("Second <line> & more")}));
    }

    void everyTextBoxDesignIsPutInWithItsLookAndReadsBack()
    {
        QVERIFY(textBoxDesigns().size() >= 5);
        for (const TextBoxDesign &design : textBoxDesigns()) {
            auto doc = textDoc({"Before after"});
            QTextCursor c(doc->story());
            c.setPosition(7);
            QCOMPARE(insertTextBox(*doc, c, design), PictureResult::Ok);
            const PictureInfo info = pictureAt(*doc, doc->story(), 7);
            QVERIFY2(info.valid && info.kind == DrawnKind::TextBox, qPrintable(design.id));
            QCOMPARE(info.width, design.size.width());
            QCOMPARE(info.fillColor, design.look.fill);
            QCOMPARE(info.fillColor.isEmpty(), design.look.fill.isEmpty());
            QCOMPARE(info.lineColor.isEmpty(), design.look.line.isEmpty() || design.look.weight <= 0);
            auto back = open(doc->toDocx());
            QVERIFY(back);
            QCOMPARE(pictureAt(*back, back->story(), 7).width, design.size.width());
        }
    }

    void aTextBoxWithMoreThanWordsIsNotEditedAsWords()
    {
        // A text box a file holds, with a table in it, keeps its table when the box's words are asked for.
        const QByteArray box = "<w:p><w:r><w:drawing><wp:anchor distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\" simplePos=\"0\" relativeHeight=\"1\" behindDoc=\"0\" locked=\"0\" layoutInCell=\"1\" allowOverlap=\"1\">"
                               "<wp:simplePos x=\"0\" y=\"0\"/><wp:positionH relativeFrom=\"page\"><wp:posOffset>914400</wp:posOffset></wp:positionH><wp:positionV relativeFrom=\"page\"><wp:posOffset>914400</wp:posOffset></wp:positionV>"
                               "<wp:extent cx=\"2540000\" cy=\"1270000\"/><wp:wrapNone/><wp:docPr id=\"3\" name=\"Box\"/><wp:cNvGraphicFramePr/><a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\">"
                               "<wps:wsp><wps:cNvSpPr txBox=\"1\"/><wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"2540000\" cy=\"1270000\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></wps:spPr>"
                               "<wps:txbx><w:txbxContent><w:tbl><w:tr><w:tc><w:p><w:r><w:t>cell</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p/></w:txbxContent></wps:txbx><wps:bodyPr/></wps:wsp></a:graphicData></a:graphic></wp:anchor></w:drawing></w:r></w:p>";
        auto doc = open(makeDocx(box + "<w:p><w:r><w:t>Text</w:t></w:r></w:p>"));
        QVERIFY(doc);
        QVERIFY(isDrawnObjectAt(doc->story(), 0));
        QVERIFY(!textBoxText(*doc, 0).has_value());
        QVERIFY(!setTextBoxText(*doc, 0, {QStringLiteral("new")}));
        QVERIFY(documentXml(doc->toDocx()).contains("cell"));
    }

    void aTextBoxOfAFileHasItsFillAndInsetsDrawn()
    {
        const QByteArray box = shapeDrawing("<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>", "<a:solidFill><a:srgbClr val=\"FFFF00\"/></a:solidFill><a:ln><a:noFill/></a:ln>", 2540000, 1270000,
                                            "<wps:txbx><w:txbxContent><w:p><w:r><w:t>Hello box</w:t></w:r></w:p></w:txbxContent></wps:txbx>");
        auto doc = open(makeDocx(box + "<w:p><w:r><w:t>Text</w:t></w:r></w:p>"));
        QVERIFY(doc);
        const ItemList items = objectsOfPage(*doc);
        QCOMPARE(items.size(), 1);
        QCOMPARE(items[0]->type(), ItemType::Text);
        QVERIFY(!items[0]->fill.isNone());
        QCOMPARE(items[0]->fill.color.rgbValue(), QColor(255, 255, 0));
        QVERIFY(items[0]->stroke.isNone());
    }
};

QTEST_MAIN(DrawTests)
#include "tst_write_draw.moc"
