// Tests of JeffWrite's drawn objects: the shapes and text boxes a file holds and the ones Insert puts in, and the Picture Format and Shape Format commands. Each document is
// built here from a few lines of XML.

#include "../isolatesettings.h"
#include "app/pageview.h"
#include "app/ribbon.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "app/writeinfo.h"
#include "core/drawnitems.h"
#include "core/equationops.h"
#include "core/fonts.h"
#include "core/items.h"
#include "core/pictureops.h"
#include "core/writedoc.h"
#include "text/textprops.h"
#include "office/drawingml.h"
#include "office/drawingml_geometry.h"
#include "office/package.h"
#include "wml/wml.h"
#include "wml/xmltree.h"

#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QJsonDocument>
#include <QTextBlock>
#include <QLineEdit>
#include <QStatusBar>
#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QTableWidget>
#include <QTest>
#include <QTimer>
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


// A picture file of one color (a PNG), this many pixels across and down.
QByteArray pngOf(int w, int h, QColor color = QColor(200, 30, 30))
{
    QImage image(w, h, QImage::Format_RGB32);
    image.fill(color);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return bytes;
}

// The first picture of the text as the pages draw it: an object set in the line, or one set at a place on the first page.
std::shared_ptr<PictureItem> pictureOfPage(WriteDoc &doc)
{
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QString json = it.fragment().charFormat().stringProperty(tp::InlineObject);
            if (json.isEmpty()) continue;
            const ItemPtr item = Item::fromJsonAny(QJsonDocument::fromJson(json.toUtf8()).object());
            if (item && item->type() == ItemType::Picture) return std::static_pointer_cast<PictureItem>(item);
        }
    const auto page = doc.pager().materialize(0);
    if (!page) return nullptr;
    for (const auto &it : page->doc->pages[0]->items)
        if (it->type() == ItemType::Picture) return std::static_pointer_cast<PictureItem>(it);
    return nullptr;
}

// The window on a document of one paragraph, the caret at the end of its first word.
struct Win {
    WriteWindow w;
    explicit Win(const QByteArray &paragraph = "<w:p><w:r><w:t>Hello world</w:t></w:r></w:p>")
    {
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(makeDocx(paragraph)), QString());
        QTextCursor c(w.document()->story());
        c.setPosition(5);
        w.view()->setCursor(c);
    }
    bool tabShown(const QString &name)
    {
        for (int i = 0; i < w.ribbon()->tabCount(); ++i)
            if (w.ribbon()->tabName(i) == name) return w.ribbon()->tabVisible(i);
        return false;
    }
};

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

    void aShapeAndATextBoxFromTheWindowAreSelectedAndShowTheShapeFormatTab()
    {
        Win win;
        QVERIFY(!win.tabShown(QStringLiteral("Shape Format")));
        QVERIFY(win.w.view()->insertShapeObject(QStringLiteral("star5")));
        QCOMPARE(win.w.view()->selectedPicture(), 5);
        QTRY_VERIFY(win.tabShown(QStringLiteral("Shape Format")));
        QVERIFY(!win.tabShown(QStringLiteral("Picture Format")));
        QVERIFY(win.w.act("picture.altText")->isEnabled());
        QVERIFY(!win.w.act("shape.editText")->isEnabled());
        QVERIFY(win.w.act("shape.rotate.right")->isEnabled());
        win.w.act("shape.rotate.right")->trigger();
        QVERIFY(std::abs(pictureAt(*win.w.document(), win.w.document()->story(), 5).rotation - 90) < 0.01);
        win.w.act("shape.weight.3")->trigger();
        QCOMPARE(pictureAt(*win.w.document(), win.w.document()->story(), 5).lineWeight, 3.0);
        win.w.act("shape.dash.dash")->trigger();
        QCOMPARE(pictureAt(*win.w.document(), win.w.document()->story(), 5).lineDash, QStringLiteral("dash"));
        QVERIFY(win.w.act("shape.dash.dash")->isChecked());
        QVERIFY(win.w.act("shape.weight.3")->isChecked());
        // Another object takes its place: a text box, whose words can be edited.
        QVERIFY(win.w.view()->insertTextBoxObject(textBoxDesigns().first()));
        QTRY_VERIFY(win.w.act("shape.editText")->isEnabled());
        QVERIFY(win.w.act("insert.textBox.sidebar")->isEnabled());
        // Clicking away leaves the tab.
        QTextCursor c(win.w.document()->story());
        c.setPosition(1);
        win.w.view()->setCursor(c);
        QTRY_VERIFY(!win.tabShown(QStringLiteral("Shape Format")));
    }

    // ---------------------------------------------------------------- picture adjustments
    void aPictureIsCutKeepingItsScaleAndTheCutIsSavedDrawnAndReset()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertPicture(*doc, c, pngOf(160, 160)), PictureResult::Ok);
        QCOMPARE(pictureAt(*doc, doc->story(), 7).width, 120.0);
        QVERIFY(setPictureCrop(*doc, 7, 0.25, 0, 0.25, 0.1));
        PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QCOMPARE(info.width, 60.0);
        QCOMPARE(info.height, 108.0);
        QVERIFY(std::abs(info.cropLeft - 0.25) < 1e-6 && std::abs(info.cropBottom - 0.1) < 1e-6);
        QVERIFY(!setPictureCrop(*doc, 7, 0.25, 0, 0.25, 0.1));
        QVERIFY(!setPictureCrop(*doc, 7, 0.6, 0, 0.6, 0));
        QVERIFY(!setPictureCrop(*doc, 7, std::nan(""), 0, 0, 0));
        // The page draws the cut part in the smaller frame.
        auto pic = pictureOfPage(*doc);
        QVERIFY(pic);
        QVERIFY(std::abs(pic->rect.width() - 60) < 0.01);
        QVERIFY(pic->imgRect.left() < -14 && pic->imgRect.width() > 119);
        QVERIFY(doc->undo());
        QCOMPARE(pictureAt(*doc, doc->story(), 7).width, 120.0);
        QVERIFY(doc->redo());
        // A second cut is measured from the picture, not from what is left.
        QVERIFY(setPictureCrop(*doc, 7, 0, 0, 0, 0));
        QCOMPARE(pictureAt(*doc, doc->story(), 7).width, 120.0);
        QVERIFY(setPictureCrop(*doc, 7, 0.5, 0, 0, 0));
        QCOMPARE(pictureAt(*doc, doc->story(), 7).width, 60.0);
        const QByteArray saved = doc->toDocx();
        QVERIFY(documentXml(saved).contains("<a:srcRect l=\"50000\"/>"));
        office::wml::XmlTree tree;
        QVERIFY(tree.parse(documentXml(saved)));
        auto back = open(saved);
        QVERIFY(std::abs(pictureAt(*back, back->story(), 7).cropLeft - 0.5) < 1e-6);
        QVERIFY(resetPicture(*doc, 7));
        info = pictureAt(*doc, doc->story(), 7);
        QCOMPARE(info.width, 120.0);
        QCOMPARE(info.cropLeft, 0.0);
        QVERIFY(!documentXml(doc->toDocx()).contains("srcRect"));
        QVERIFY(!resetPicture(*doc, 7));
    }

    void aPicturesBrightnessContrastAndColorAreChangedSavedAndDrawn()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertPicture(*doc, c, pngOf(160, 160)), PictureResult::Ok);
        QVERIFY(setPictureCorrections(*doc, 7, 0.2, -0.4));
        PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QVERIFY(std::abs(info.brightness - 0.2) < 1e-6 && std::abs(info.contrast + 0.4) < 1e-6);
        QVERIFY(!setPictureCorrections(*doc, 7, 0.2, -0.4));
        QVERIFY(setPictureRecolor(*doc, 7, PictureRecolor::Grayscale));
        info = pictureAt(*doc, doc->story(), 7);
        QVERIFY(info.grayscale && std::abs(info.brightness - 0.2) < 1e-6);
        QVERIFY(!setPictureRecolor(*doc, 7, PictureRecolor::Grayscale));
        auto pic = pictureOfPage(*doc);
        QVERIFY(pic);
        QVERIFY(!pic->colorEffects().isEmpty());
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY(x.contains("<a:lum bright=\"20000\" contrast=\"-40000\"/>"));
        QVERIFY(x.contains("<a:grayscl/>"));
        QVERIFY(x.indexOf("<a:lum") < x.indexOf("<a:grayscl"));
        office::wml::XmlTree tree;
        QVERIFY(tree.parse(x));
        auto back = open(doc->toDocx());
        QVERIFY(pictureAt(*back, back->story(), 7).grayscale);
        QVERIFY(doc->undo());
        QVERIFY(!pictureAt(*doc, doc->story(), 7).grayscale);
        QVERIFY(doc->redo());
        QVERIFY(setPictureRecolor(*doc, 7, PictureRecolor::BlackAndWhite));
        QVERIFY(!pictureAt(*doc, doc->story(), 7).grayscale);
        QVERIFY(documentXml(doc->toDocx()).contains("<a:biLevel"));
        QVERIFY(setPictureRecolor(*doc, 7, PictureRecolor::None));
        QVERIFY(setPictureCorrections(*doc, 7, 0, 0));
        QVERIFY(!documentXml(doc->toDocx()).contains("<a:lum"));
        // Set again and taken off with Reset Picture, along with a border.
        QVERIFY(setPictureCorrections(*doc, 7, -0.3, 0.3));
        QVERIFY(setShapeLine(*doc, 7, QStringLiteral("112233"), 2, QStringLiteral("solid")));
        info = pictureAt(*doc, doc->story(), 7);
        QCOMPARE(info.lineColor, QStringLiteral("112233"));
        QCOMPARE(info.lineWeight, 2.0);
        pic = pictureOfPage(*doc);
        QVERIFY(!pic->stroke.isNone());
        QVERIFY(resetPicture(*doc, 7));
        info = pictureAt(*doc, doc->story(), 7);
        QVERIFY(info.lineColor.isEmpty() && info.brightness == 0 && info.contrast == 0);
        QVERIFY(!documentXml(doc->toDocx()).contains("<a:ln"));
    }

    void aPictureIsChangedForAnotherInTheSameFrameKeepingItsPlaceAndAltText()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertPicture(*doc, c, pngOf(160, 80), QStringLiteral("red"), PictureWrap::Square, QStringLiteral("A red bar")), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 7, 100, 200));
        QVERIFY(setPictureCrop(*doc, 7, 0.2, 0, 0, 0));
        const PictureInfo before = pictureAt(*doc, doc->story(), 7);
        QCOMPARE(changePictureFile(*doc, 7, pngOf(100, 100, QColor(20, 20, 200))), PictureResult::Ok);
        const PictureInfo after = pictureAt(*doc, doc->story(), 7);
        QCOMPARE(after.alt, QStringLiteral("A red bar"));
        QCOMPARE(after.left, before.left);
        QCOMPARE(after.top, before.top);
        QCOMPARE(after.wrap, PictureWrap::Square);
        QCOMPARE(after.cropLeft, 0.0);
        // The frame keeps its width and takes the new picture's proportions (a square).
        QVERIFY(std::abs(after.width - after.height) < 0.01);
        QVERIFY(doc->undo());
        QCOMPARE(pictureAt(*doc, doc->story(), 7).height, before.height);
        QVERIFY(doc->redo());
        auto back = open(doc->toDocx());
        QVERIFY(back);
        QCOMPARE(pictureAt(*back, back->story(), 7).alt, QStringLiteral("A red bar"));
        // Not a picture; and not where there is none.
        QCOMPARE(changePictureFile(*doc, 7, QByteArray("not a picture")), PictureResult::Unreadable);
        QCOMPARE(changePictureFile(*doc, 2, pngOf(10, 10)), PictureResult::Refused);
    }

    void thePictureFormatTabCorrectsRecolorsFramesAndResetsInOneStepEach()
    {
        Win win;
        QVERIFY(win.w.view()->insertPictureBytes(pngOf(160, 160), QStringLiteral("p")));
        const int at = win.w.view()->selectedPicture();
        QCOMPARE(at, 5);
        QTRY_VERIFY(win.tabShown(QStringLiteral("Picture Format")));
        QVERIFY(!win.tabShown(QStringLiteral("Shape Format")));
        auto info = [&] { return pictureAt(*win.w.document(), win.w.document()->story(), at); };
        QVERIFY(win.w.act("picture.crop")->isEnabled());
        QVERIFY(!win.w.act("shape.editText")->isEnabled());
        win.w.act("picture.correct.brightness.20")->trigger();
        QVERIFY(std::abs(info().brightness - 0.2) < 1e-6);
        QVERIFY(win.w.act("picture.correct.brightness.20")->isChecked());
        win.w.act("picture.correct.contrast.-20")->trigger();
        QVERIFY(std::abs(info().contrast + 0.2) < 1e-6 && std::abs(info().brightness - 0.2) < 1e-6);
        win.w.act("picture.recolor.gray")->trigger();
        QVERIFY(info().grayscale);
        QVERIFY(win.w.act("picture.recolor.gray")->isChecked());
        win.w.act("picture.recolor.washout")->trigger();
        QVERIFY(!info().grayscale && std::abs(info().brightness - 0.7) < 1e-6 && std::abs(info().contrast + 0.7) < 1e-6);
        win.w.act("shape.weight.3")->trigger();
        QCOMPARE(info().lineWeight, 3.0);
        win.w.act("edit.undo")->trigger();
        QCOMPARE(info().lineWeight, 0.0);
        QTRY_VERIFY(win.w.act("picture.reset")->isEnabled());
        win.w.act("picture.reset")->trigger();
        QVERIFY(info().brightness == 0 && info().contrast == 0 && !info().grayscale);
        const QString file = QDir::temp().filePath(QStringLiteral("jw-draw-test-%1.png").arg(QCoreApplication::applicationPid()));
        {
            QFile f(file);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(pngOf(80, 40, QColor(10, 200, 10)));
        }
        win.w.setPictureChooser([file] { return file; });
        win.w.act("picture.change")->trigger();
        QCOMPARE(info().width, 120.0);
        QCOMPARE(info().height, 60.0);
        QFile::remove(file);
    }

    // ---------------------------------------------------------------- the Draw tab: ink in a file
    void inkDrawnInAFileIsKeptWithItsPartAndWrittenBackAsItWas()
    {
        office::PackageWriter w;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QByteArray ink = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><inkml:ink xmlns:inkml=\"http://www.w3.org/2003/InkML\"><inkml:trace>0 0, 10 10, 20 0</inkml:trace></inkml:ink>";
        w.addPart(QStringLiteral("/word/ink/ink1.xml"), ink, QStringLiteral("application/inkml+xml"));
        const QString rid = w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("customXml"), QStringLiteral("ink/ink1.xml"));
        const QByteArray run = "<w:r><w:drawing><wp:anchor distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\" simplePos=\"0\" relativeHeight=\"1\" behindDoc=\"0\" locked=\"0\" layoutInCell=\"1\" allowOverlap=\"1\">"
                               "<wp:simplePos x=\"0\" y=\"0\"/><wp:positionH relativeFrom=\"column\"><wp:posOffset>0</wp:posOffset></wp:positionH><wp:positionV relativeFrom=\"paragraph\"><wp:posOffset>0</wp:posOffset></wp:positionV>"
                               "<wp:extent cx=\"1270000\" cy=\"635000\"/><wp:wrapNone/><wp:docPr id=\"7\" name=\"Ink 7\"/><wp:cNvGraphicFramePr/><a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingInk\">"
                               "<wpi:wpInk xmlns:wpi=\"http://schemas.microsoft.com/office/word/2010/wordprocessingInk\"><wpi:bodyPr/><wpi:contentPart xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\" bwMode=\"auto\" r:id=\"" +
                               rid.toUtf8() + "\"><w14:nvContentPartPr><w14:cNvContentPartPr/></w14:nvContentPartPr><w14:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1270000\" cy=\"635000\"/></w14:xfrm></wpi:contentPart></wpi:wpInk>"
                               "</a:graphicData></a:graphic></wp:anchor></w:drawing></w:r>";
        w.addPart(QStringLiteral("/word/document.xml"),
                  QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body><w:p><w:r><w:t>Signed here: </w:t></w:r>" + run + "</w:p></w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        auto doc = open(w.finish());
        QVERIFY(doc);
        // Edit around it, and save twice.
        QTextCursor c(doc->story());
        c.setPosition(2);
        c.insertText(QStringLiteral("X"));
        for (int round = 0; round < 2; ++round) {
            const QByteArray saved = doc->toDocx();
            office::Package p;
            QVERIFY(p.read(saved));
            QCOMPARE(p.part(QStringLiteral("/word/ink/ink1.xml")), ink);
            QCOMPARE(p.contentType(QStringLiteral("/word/ink/ink1.xml")), QStringLiteral("application/inkml+xml"));
            const office::Relationship r = p.relationship(QStringLiteral("/word/document.xml"), rid);
            QCOMPARE(r.target, QStringLiteral("/word/ink/ink1.xml"));
            QVERIFY(r.type.endsWith(QLatin1String("/customXml")));
            const QByteArray x = documentXml(saved);
            QVERIFY(x.contains("wordprocessingInk") && x.contains("wpi:contentPart") && x.contains("r:id=\"" + rid.toUtf8() + "\""));
            office::wml::XmlTree tree;
            QVERIFY(tree.parse(x));
            doc = open(saved);
            QVERIFY(doc);
        }
    }

    // ---------------------------------------------------------------- the Equation tab
    void aTypedEquationBecomesTheMathMarkupItsStructuresAndTheLineThePagesShow()
    {
        struct Case {
            const char *typed, *element, *line;
        };
        const Case cases[] = {
            {"a/b", "<m:f><m:num>", "a/b"},
            {"x^2", "<m:sSup>", "x2"},
            {"x_i", "<m:sSub>", "xi"},
            {"x_i^2", "<m:sSubSup>", "xi2"},
            {"sqrt(x+1)", "<m:rad>", ""},
            {"sqrt[3](x)", "<m:deg>", ""},
            {"sqrt[3]{x}", "<m:deg>", ""},
            {"\\sum_(i=1)^n i", "<m:nary>", ""},
            {"\\int_0^1 x", "<m:nary>", ""},
            {"(a+b)/(c+d)", "<m:f>", ""},
            {"\\alpha+\\pi", "<m:r>", ""},
            {"\\frac{1}{2}", "<m:f>", ""},
            {"E=mc^2", "<m:sSup>", ""},
        };
        for (const Case &k : cases) {
            bool ok = false;
            const QByteArray markup = equationMarkup(QString::fromUtf8(k.typed), &ok);
            QVERIFY2(ok, k.typed);
            QVERIFY2(markup.contains(k.element), k.typed);
            QVERIFY2(markup.startsWith("<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"), k.typed);
            office::wml::XmlTree tree;
            QVERIFY2(tree.parse(markup), k.typed);
            QVERIFY2(!equationLine(markup).isEmpty(), k.typed);
            if (*k.line) QCOMPARE(equationLine(markup), QString::fromLatin1(k.line));
        }
        QCOMPARE(equationLine(equationMarkup(QStringLiteral("\\alpha+\\pi"))), QString(QChar(0x3B1)) + QStringLiteral(" + ") + QString(QChar(0x3C0)));
        QVERIFY(equationLine(equationMarkup(QStringLiteral("\\sum_(i=1)^n i"))).contains(QChar(0x2211)));
        QVERIFY(equationLine(equationMarkup(QStringLiteral("sqrt(x+1)"))).contains(QChar(0x221A)));
        // Fractions take the item before the slash; text in the equation is escaped; nothing is made of nothing or of too much.
        QVERIFY(equationMarkup(QStringLiteral("2+x/y")).contains("<m:num><m:r><m:t>x</m:t></m:r></m:num>"));
        QVERIFY(equationMarkup(QStringLiteral("a<b & c")).contains("&lt;"));
        bool ok = true;
        QVERIFY(equationMarkup(QStringLiteral("   "), &ok).isEmpty() && !ok);
        QVERIFY(equationMarkup(QString(kMostEquationInput + 1, QLatin1Char('x')), &ok).isEmpty() && !ok);
        QVERIFY(equationMarkup(QString(QStringLiteral("(")).repeated(kMostEquationDepth + 10) + QStringLiteral("x"), &ok).isEmpty() && !ok);
        QVERIFY(equationSymbol(QStringLiteral("nonsense")).isEmpty());
    }

    void anEquationIsPutInAsOneStepIsShownAsItsLineAndSavedAsMathMarkup()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QVERIFY(insertEquation(*doc, c, QStringLiteral("x^2/y")));
        QVERIFY(!insertEquation(*doc, c, QStringLiteral("  ")));
        QCOMPARE(doc->story()->toPlainText().count(QChar::ObjectReplacementCharacter), 1);
        const QByteArray saved = doc->toDocx();
        const QByteArray x = documentXml(saved);
        QVERIFY(x.contains("<m:oMath"));
        QVERIFY(x.contains("<m:f>") && x.contains("<m:sSup>"));
        office::wml::XmlTree tree;
        QVERIFY(tree.parse(x));
        // The paragraph holds the equation between its words, and reads back as it was.
        QVERIFY(x.indexOf("Before") < x.indexOf("<m:oMath") && x.indexOf("<m:oMath") < x.indexOf("after"));
        auto back = open(saved);
        QVERIFY(back);
        QCOMPARE(back->story()->toPlainText().count(QChar::ObjectReplacementCharacter) + back->story()->toPlainText().count(QLatin1String("x2/y")), 1);
        QVERIFY(documentXml(back->toDocx()).contains("<m:f>"));
        QVERIFY(doc->undo());
        QCOMPARE(doc->story()->toPlainText(), QStringLiteral("Before after"));
        QVERIFY(!documentXml(doc->toDocx()).contains("<m:oMath"));
        QVERIFY(doc->redo());
        QVERIFY(documentXml(doc->toDocx()).contains("<m:oMath"));
    }

    void theEquationDialogPreviewsAndPutsTheEquationAtTheCaret()
    {
        Win win;
        QVERIFY(win.w.act("insert.equation")->isEnabled());
        QTimer once;
        once.setSingleShot(true);
        QObject::connect(&once, &QTimer::timeout, [] {
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QVERIFY(dlg);
            auto *input = dlg->findChild<QLineEdit *>(QStringLiteral("equationInput"));
            auto *preview = dlg->findChild<QLabel *>(QStringLiteral("equationPreview"));
            QVERIFY(input && preview);
            input->setText(QStringLiteral("a/b"));
            QCOMPARE(preview->text(), QStringLiteral("a/b"));
            dlg->accept();
        });
        once.start(0);
        win.w.act("insert.equation")->trigger();
        QVERIFY(documentXml(win.w.document()->toDocx()).contains("<m:oMath"));
    }

    // ---------------------------------------------------------------- charts
    void aChartOfEachFormIsPutInDrawnAndSavedAsAChartPartThatReadsBack()
    {
        for (ChartForm form : chartForms()) {
            auto doc = textDoc({"Before after"});
            QTextCursor c(doc->story());
            c.setPosition(7);
            QCOMPARE(insertChart(*doc, c, form), PictureResult::Ok);
            const PictureInfo info = pictureAt(*doc, doc->story(), 7);
            QVERIFY(info.valid && info.editable);
            QCOMPARE(info.kind, DrawnKind::Chart);
            QCOMPARE(info.width, 360.0);
            QCOMPARE(info.height, 216.0);
            QVERIFY(!isPictureAt(doc->story(), 7));
            QVERIFY(isDrawnObjectAt(doc->story(), 7));
            const ChartInfo chart = chartAt(*doc, 7);
            QVERIFY(chart.valid);
            QCOMPARE(chart.form, form);
            QCOMPARE(chart.data.categories.size(), 3);
            QCOMPARE(chart.data.values.size(), 2);
            QVERIFY(chart.dataEditable);
            // The pages draw it as a picture.
            QVERIFY(pictureOfPage(*doc));
            const QByteArray saved = doc->toDocx();
            office::Package p;
            QVERIFY(p.read(saved));
            const QByteArray part = p.part(QStringLiteral("/word/charts/chart1.xml"));
            QVERIFY(part.contains("<c:chartSpace"));
            QCOMPARE(p.contentType(QStringLiteral("/word/charts/chart1.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"));
            QVERIFY(documentXml(saved).contains("<c:chart "));
            office::wml::XmlTree tree;
            QVERIFY(tree.parse(documentXml(saved)));
            QVERIFY(tree.parse(part));
            auto back = open(saved);
            QVERIFY(back);
            QCOMPARE(chartAt(*back, 7).form, form);
            QCOMPARE(chartAt(*back, 7).data.values, chart.data.values);
            QVERIFY(pictureOfPage(*back));
            QVERIFY(doc->undo());
            office::Package gone;
            QVERIFY(gone.read(doc->toDocx()));
            QVERIFY(gone.part(QStringLiteral("/word/charts/chart1.xml")).isEmpty());
            QVERIFY(doc->redo());
        }
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        ChartData bad = sampleChartData();
        bad.values[0].resize(2);
        QCOMPARE(insertChart(*doc, c, ChartForm::Column, bad), PictureResult::Unreadable);
        bad = sampleChartData();
        bad.values[1][0] = std::nan("");
        QCOMPARE(insertChart(*doc, c, ChartForm::Column, bad), PictureResult::Unreadable);
        QCOMPARE(insertChart(*doc, c, ChartForm::Column, ChartData()), PictureResult::Unreadable);
    }

    void aChartsKindTitleLegendLabelsAndDataAreChangedOneStepEach()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertChart(*doc, c, ChartForm::Column), PictureResult::Ok);
        QVERIFY(!setChartForm(*doc, 7, ChartForm::Column));
        QVERIFY(setChartForm(*doc, 7, ChartForm::Pie));
        QCOMPARE(chartAt(*doc, 7).form, ChartForm::Pie);
        QVERIFY(doc->undo());
        QCOMPARE(chartAt(*doc, 7).form, ChartForm::Column);
        QVERIFY(doc->redo());
        QVERIFY(setChartForm(*doc, 7, ChartForm::LineMarkers));
        QVERIFY(setChartForm(*doc, 7, ChartForm::Scatter));
        QCOMPARE(chartAt(*doc, 7).form, ChartForm::Scatter);
        QVERIFY(setChartForm(*doc, 7, ChartForm::BarStacked));
        QCOMPARE(chartAt(*doc, 7).form, ChartForm::BarStacked);
        // Title.
        QVERIFY(setChartTitle(*doc, 7, true, QStringLiteral("Sales <by> region")));
        QVERIFY(!setChartTitle(*doc, 7, true, QStringLiteral("Sales <by> region")));
        QVERIFY(chartAt(*doc, 7).titleShown);
        QCOMPARE(chartAt(*doc, 7).title, QStringLiteral("Sales <by> region"));
        QVERIFY(setChartTitle(*doc, 7, false, QString()));
        QVERIFY(!chartAt(*doc, 7).titleShown);
        // Legend.
        QVERIFY(setChartLegend(*doc, 7, QStringLiteral("t")));
        QCOMPARE(chartAt(*doc, 7).legendPosition, QStringLiteral("t"));
        QVERIFY(!setChartLegend(*doc, 7, QStringLiteral("t")));
        QVERIFY(!setChartLegend(*doc, 7, QStringLiteral("x")));
        QVERIFY(setChartLegend(*doc, 7, QString()));
        QVERIFY(!chartAt(*doc, 7).legendShown);
        // Labels.
        QVERIFY(setChartDataLabels(*doc, 7, true));
        QVERIFY(chartAt(*doc, 7).dataLabels);
        QVERIFY(!setChartDataLabels(*doc, 7, true));
        QVERIFY(setChartDataLabels(*doc, 7, false));
        // Data: a third series and a fourth category, and the numbers of the first.
        ChartData data = chartAt(*doc, 7).data;
        data.categories << QStringLiteral("East & West");
        data.values = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12.5}};
        data.seriesNames << QStringLiteral("Third");
        QVERIFY(setChartData(*doc, 7, data));
        const ChartInfo now = chartAt(*doc, 7);
        QCOMPARE(now.data.categories, data.categories);
        QCOMPARE(now.data.seriesNames, data.seriesNames);
        QCOMPARE(now.data.values, data.values);
        QCOMPARE(now.form, ChartForm::BarStacked);
        QVERIFY(!setChartData(*doc, 7, data));
        ChartData ragged = data;
        ragged.values[2].resize(1);
        QVERIFY(!setChartData(*doc, 7, ragged));
        QVERIFY(doc->undo());
        QCOMPARE(chartAt(*doc, 7).data.values.size(), 2);
        QVERIFY(doc->redo());
        // The file holds one chart part for the drawing, and reads back as it is.
        const QByteArray saved = doc->toDocx();
        office::Package p;
        QVERIFY(p.read(saved));
        int parts = 0;
        for (const QString &name : p.partNames())
            if (name.startsWith(QLatin1String("/word/charts/")) && name.endsWith(QLatin1String(".xml"))) ++parts;
        QCOMPARE(parts, 1);
        auto back = open(saved);
        QCOMPARE(chartAt(*back, 7).data.values, data.values);
        QCOMPARE(chartAt(*back, 7).form, ChartForm::BarStacked);
        // A picture or a shape is no chart.
        QTextCursor d(doc->story());
        d.setPosition(2);
        QCOMPARE(insertShape(*doc, d, QStringLiteral("rect")), PictureResult::Ok);
        QVERIFY(!chartAt(*doc, 2).valid);
        QVERIFY(!setChartForm(*doc, 2, ChartForm::Pie));
    }

    void aChartAFileHoldsIsDrawnAndItsTitleIsChangedKeepingItsWorkbook()
    {
        office::PackageWriter w;
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        const QString charts = QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml");
        // A chart part made by the same writer (so it is one the program writes whole), with a workbook beside it.
        auto source = textDoc({"x"});
        QTextCursor c(source->story());
        QCOMPARE(insertChart(*source, c, ChartForm::Line), PictureResult::Ok);
        office::Package sp;
        QVERIFY(sp.read(source->toDocx()));
        QByteArray chartXml = sp.part(QStringLiteral("/word/charts/chart1.xml"));
        const int closing = chartXml.lastIndexOf("</c:chartSpace>");
        QVERIFY(closing > 0);
        chartXml.insert(closing, "<c:externalData r:id=\"rId1\"><c:autoUpdate val=\"0\"/></c:externalData>");
        if (!chartXml.contains("xmlns:r=")) chartXml.replace("<c:chartSpace ", "<c:chartSpace xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" ");
        w.addPart(QStringLiteral("/word/charts/chart7.xml"), chartXml, charts);
        w.addPart(QStringLiteral("/word/embeddings/book1.xlsx"), QByteArray("PK-not-really-a-workbook"), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
        w.addRelationship(QStringLiteral("/word/charts/chart7.xml"), rel + QStringLiteral("package"), QStringLiteral("../embeddings/book1.xlsx"));
        const QString rid = w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("chart"), QStringLiteral("charts/chart7.xml"));
        const QByteArray run = "<w:r><w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"4572000\" cy=\"2743200\"/><wp:docPr id=\"4\" name=\"Chart 4\"/><wp:cNvGraphicFramePr/>"
                               "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"><c:chart xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" r:id=\"" + rid.toUtf8() +
                               "\"/></a:graphicData></a:graphic></wp:inline></w:drawing></w:r>";
        w.addPart(QStringLiteral("/word/document.xml"),
                  QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body><w:p><w:r><w:t xml:space=\"preserve\">Chart: </w:t></w:r>" + run + "</w:p></w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        auto doc = open(w.finish());
        QVERIFY(doc);
        QVERIFY(isDrawnObjectAt(doc->story(), 7));
        QCOMPARE(pictureAt(*doc, doc->story(), 7).kind, DrawnKind::Chart);
        QVERIFY(pictureOfPage(*doc));
        QVERIFY(setChartTitle(*doc, 7, true, QStringLiteral("Fresh title")));
        QCOMPARE(chartAt(*doc, 7).title, QStringLiteral("Fresh title"));
        // The new part has the workbook the old one had.
        office::Package p;
        QVERIFY(p.read(doc->toDocx()));
        QVERIFY(p.part(QStringLiteral("/word/embeddings/book1.xlsx")).startsWith("PK-not"));
        int withBook = 0;
        for (const QString &name : p.partNames())
            if (name.startsWith(QLatin1String("/word/charts/")) && name.endsWith(QLatin1String(".xml")) && !p.relationships(name).isEmpty()) ++withBook;
        QVERIFY(withBook >= 1);
    }

    void aChartFromTheWindowShowsTheChartFormatTabAndItsCommandsChangeItOneStepEach()
    {
        Win win;
        QVERIFY(win.w.act("insert.chart.pie")->isEnabled());
        win.w.act("insert.chart.pie")->trigger();
        const int at = win.w.view()->selectedPicture();
        QCOMPARE(at, 5);
        QTRY_VERIFY(win.tabShown(QStringLiteral("Chart Format")));
        QVERIFY(!win.tabShown(QStringLiteral("Shape Format")) && !win.tabShown(QStringLiteral("Picture Format")));
        auto info = [&] { return chartAt(*win.w.document(), at); };
        QCOMPARE(info().form, ChartForm::Pie);
        QTRY_VERIFY(win.w.act("chart.type.pie")->isChecked());
        QVERIFY(win.w.act("chart.editData")->isEnabled());
        QVERIFY(!win.w.act("shape.editText")->isEnabled());
        QVERIFY(!win.w.act("picture.crop")->isEnabled());
        win.w.act("chart.type.line")->trigger();
        QCOMPARE(info().form, ChartForm::Line);
        win.w.act("chart.legend.bottom")->trigger();
        QCOMPARE(info().legendPosition, QStringLiteral("b"));
        win.w.act("chart.legend.none")->trigger();
        QVERIFY(!info().legendShown);
        win.w.act("chart.dataLabels")->trigger();
        QVERIFY(info().dataLabels);
        QTRY_VERIFY(win.w.act("chart.dataLabels")->isChecked());
        // The title: shown with the words it had, then edited in its dialog.
        QTRY_VERIFY(win.w.act("chart.title")->isEnabled());
        win.w.act("chart.title")->trigger();
        QVERIFY(info().titleShown);
        QTRY_VERIFY(win.w.act("chart.editTitle")->isEnabled());
        QTimer once;
        once.setSingleShot(true);
        QObject::connect(&once, &QTimer::timeout, [] {
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QVERIFY(dlg);
            dlg->findChild<QLineEdit *>(QStringLiteral("chartTitleEdit"))->setText(QStringLiteral("Totals by month"));
            dlg->accept();
        });
        once.start(0);
        win.w.act("chart.editTitle")->trigger();
        QCOMPARE(info().title, QStringLiteral("Totals by month"));
        win.w.act("edit.undo")->trigger();
        QVERIFY(info().title != QStringLiteral("Totals by month"));
        // A table of the data in its own dialog, with a cell changed.
        QTimer again;
        again.setSingleShot(true);
        QObject::connect(&again, &QTimer::timeout, [] {
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QVERIFY(dlg);
            auto *table = dlg->findChild<QTableWidget *>(QStringLiteral("chartDataTable"));
            QVERIFY(table);
            table->item(1, 1)->setText(QStringLiteral("99"));
            dlg->accept();
        });
        again.start(0);
        win.w.act("chart.editData")->trigger();
        QCOMPARE(info().data.values.value(0).value(0), 99.0);
    }
};

QTEST_MAIN(DrawTests)
#include "tst_write_draw.moc"
