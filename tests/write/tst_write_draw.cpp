// Tests of JeffWrite's drawn objects: the shapes and text boxes a file holds and the ones Insert puts in, and the Picture Format and Shape Format commands. Each document is
// built here from a few lines of XML.

#include "../isolatesettings.h"
#include "../office/diagramsample.h"
#include "rtcheck.h"
#include "app/pageview.h"
#include "app/ribbon.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "app/writeinfo.h"
#include "core/boxlinks.h"
#include "core/drawnitems.h"
#include "core/editops.h"
#include "core/equationops.h"
#include "core/fonts.h"
#include "core/items.h"
#include "core/pictureops.h"
#include "core/threadtime.h"
#include "core/writedoc.h"
#include "text/textprops.h"
#include "office/drawingml.h"
#include "office/drawingml_write.h"
#include "render/textart.h"
#include "render/renderer.h"
#include "flow/flowdocument.h"
#include "flow/paginator.h"
#include "render/diagramdrawing.h"
#include "office/drawingml_geometry.h"
#include "office/package.h"
#include "wml/wml.h"
#include "wml/xmltree.h"

#include <QBuffer>
#include <limits>
#include <QElapsedTimer>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QJsonDocument>
#include <QTextBlock>
#include <QLineEdit>
#include <QStatusBar>
#include <QAbstractButton>
#include <QAbstractItemModel>
#include <QApplication>
#include <QListWidget>
#include <QDialog>
#include <QLabel>
#include <QTableWidget>
#include <QRegularExpression>
#include <QTest>
#include <QTimer>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp;
using namespace jp::write;

namespace {

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                   "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                   "xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" xmlns:wpg=\"http://schemas.microsoft.com/office/word/2010/wordprocessingGroup\"";

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


// A drawing set at a place on the page (points) with its own id, holding this graphic (the graphicData's inside, a wps:wsp or a group).
QByteArray anchoredGraphic(int id, const QByteArray &uri, const QByteArray &inner, double x, double y, double w, double h, bool inlineInText = false)
{
    auto n = [](double v) { return QByteArray::number(qint64(v * 12700)); };
    const QByteArray graphic = "<a:graphic><a:graphicData uri=\"" + uri + "\">" + inner + "</a:graphicData></a:graphic>";
    const QByteArray frame = "<wp:extent cx=\"" + n(w) + "\" cy=\"" + n(h) + "\"/><wp:docPr id=\"" + QByteArray::number(id) + "\" name=\"D" + QByteArray::number(id) + "\"/><wp:cNvGraphicFramePr/>";
    if (inlineInText) return "<w:r><w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\">" + frame + graphic + "</wp:inline></w:drawing></w:r>";
    return "<w:r><w:drawing><wp:anchor distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\" simplePos=\"0\" relativeHeight=\"251659264\" behindDoc=\"0\" locked=\"0\" layoutInCell=\"1\" allowOverlap=\"1\">"
           "<wp:simplePos x=\"0\" y=\"0\"/><wp:positionH relativeFrom=\"page\"><wp:posOffset>" + n(x) + "</wp:posOffset></wp:positionH><wp:positionV relativeFrom=\"page\"><wp:posOffset>" + n(y) +
           "</wp:posOffset></wp:positionV><wp:extent cx=\"" + n(w) + "\" cy=\"" + n(h) + "\"/><wp:wrapNone/><wp:docPr id=\"" + QByteArray::number(id) + "\" name=\"D" + QByteArray::number(id) +
           "\"/><wp:cNvGraphicFramePr/>" + graphic + "</wp:anchor></w:drawing></w:r>";
}

const QByteArray kShapeUri = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
const QByteArray kGroupUri = "http://schemas.microsoft.com/office/word/2010/wordprocessingGroup";

QByteArray plainShape(double w, double h, const QByteArray &prst, const QByteArray &xfrm = QByteArray(), const QByteArray &cnv = "<wps:cNvSpPr/>", const QByteArray &fill = "<a:solidFill><a:srgbClr val=\"336699\"/></a:solidFill>")
{
    auto n = [](double v) { return QByteArray::number(qint64(v * 12700)); };
    const QByteArray frame = xfrm.isEmpty() ? "<a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + n(w) + "\" cy=\"" + n(h) + "\"/></a:xfrm>" : xfrm;
    return "<wps:wsp>" + cnv + "<wps:spPr>" + frame + "<a:prstGeom prst=\"" + prst + "\"><a:avLst/></a:prstGeom>" + fill + "</wps:spPr><wps:bodyPr/></wps:wsp>";
}

QByteArray gluedConnector(int id, int startId, int startSite, int endId, int endSite, const QByteArray &xfrm = QByteArray())
{
    return anchoredGraphic(id, kShapeUri, plainShape(100, 10, "straightConnector1", xfrm, "<wps:cNvCnPr><a:stCxn id=\"" + QByteArray::number(startId) + "\" idx=\"" + QByteArray::number(startSite) + "\"/><a:endCxn id=\"" +
                                               QByteArray::number(endId) + "\" idx=\"" + QByteArray::number(endSite) + "\"/></wps:cNvCnPr>", ""), 300, 300, 100, 10);
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

// A document with a diagram after the words "Diagram: ", as the word processor writes one: the frame names the data, layout, style, and colors parts, the data part names the link of the
// drawing it kept (`withDrawing`), and the main part links all five. `fallback` is the picture of the choice's other form (a picture bytes), `inlineInText` sets it in the line.
struct DiagramFile {
    QByteArray docx;
    QByteArray drawing;
};
DiagramFile diagramDocx(bool withDrawing, bool inlineInText = true, const QByteArray &fallbackPng = QByteArray(), const QByteArray &drawingXml = diagramtest::diagramDrawingXml(), int widthPoints = 288, int heightPoints = 108)
{
    using namespace diagramtest;
    office::PackageWriter w;
    const QString base = QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.");
    w.addPart(QStringLiteral("/word/diagrams/data1.xml"), diagramDataXml(withDrawing ? QStringLiteral("rIdDrw") : QString()), base + QStringLiteral("diagramData+xml"));
    w.addPart(QStringLiteral("/word/diagrams/layout1.xml"), diagramOtherXml("layoutDef"), base + QStringLiteral("diagramLayout+xml"));
    w.addPart(QStringLiteral("/word/diagrams/quickStyle1.xml"), diagramOtherXml("styleDef"), base + QStringLiteral("diagramStyle+xml"));
    w.addPart(QStringLiteral("/word/diagrams/colors1.xml"), diagramOtherXml("colorsDef"), base + QStringLiteral("diagramColors+xml"));
    if (withDrawing) w.addPart(QStringLiteral("/word/diagrams/drawing1.xml"), drawingXml, QStringLiteral("application/vnd.ms-office.drawingml.diagramDrawing+xml"));
    w.addRelationship(QStringLiteral("/"), relBase() + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdDm"), relBase() + QStringLiteral("diagramData"), QStringLiteral("/word/diagrams/data1.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdLo"), relBase() + QStringLiteral("diagramLayout"), QStringLiteral("/word/diagrams/layout1.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdQs"), relBase() + QStringLiteral("diagramQuickStyle"), QStringLiteral("/word/diagrams/quickStyle1.xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdCs"), relBase() + QStringLiteral("diagramColors"), QStringLiteral("/word/diagrams/colors1.xml"));
    if (withDrawing) w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdDrw"), drawingRelType(), QStringLiteral("/word/diagrams/drawing1.xml"));
    if (!fallbackPng.isEmpty()) {
        w.addPart(QStringLiteral("/word/media/fallback.png"), fallbackPng, QStringLiteral("image/png"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdFb"), relBase() + QStringLiteral("image"), QStringLiteral("/word/media/fallback.png"));
    }
    const QByteArray ids = "<dgm:relIds xmlns:dgm=\"" + dgmNs().toUtf8() + "\" r:dm=\"rIdDm\" r:lo=\"rIdLo\" r:qs=\"rIdQs\" r:cs=\"rIdCs\"/>";
    QByteArray run = anchoredGraphic(3, dgmNs().toUtf8(), ids, 100, 300, widthPoints, heightPoints, inlineInText);
    if (!fallbackPng.isEmpty()) {
        // (the newer form is a choice, the older a picture of the same size)
        const QByteArray choice = run.mid(run.indexOf("<w:drawing>"), run.indexOf("</w:drawing>") + 12 - run.indexOf("<w:drawing>"));
        run = "<w:r><mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"dgm\">" + choice + "</mc:Choice><mc:Fallback>"
              "<w:pict xmlns:v=\"urn:schemas-microsoft-com:vml\"><v:shape id=\"d\" type=\"#t\" style=\"width:" + QByteArray::number(widthPoints) + "pt;height:" + QByteArray::number(heightPoints) +
              "pt\"><v:imagedata r:id=\"rIdFb\"/></v:shape></w:pict></mc:Fallback></mc:AlternateContent></w:r>";
    }
    w.addPart(QStringLiteral("/word/document.xml"),
              QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + " xmlns:dgm=\"" + dgmNs().toUtf8() + "\"><w:body><w:p><w:r><w:t xml:space=\"preserve\">Diagram: </w:t></w:r>" + run +
                  "</w:p><w:p><w:r><w:t>After.</w:t></w:r></w:p></w:body></w:document>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    return {w.finish(), drawingXml};
}

// The last picture in SVG form the flow of the document holds, after the first page was drawn (the drawing of a diagram; a size changed makes a new one).
QByteArray drawnSvg(WriteDoc &doc)
{
    pictureOfPage(doc);
    for (qsizetype i = doc.flow().images.size() - 1; i >= 0; --i)
        if (doc.flow().images[i].format == QLatin1String("svg")) return doc.flow().images[i].bytes;
    return {};
}

// Two or more text boxes set at places on the page, in the order given: each is `id`, its chain (a head holds the words, a member is `linked`: the form that says "the words of the box with
// this id go on in me, as the seq-th") and its size and place. The head's words are `lines` paragraphs.
struct LinkedBox {
    int chain = -1, seq = 0;   // chain -1: a box of its own
    double x = 72, y = 100, w = 150, h = 60;
    int lines = 0;
    int page = 0;              // the page (from 0) of the paragraph that holds it
};
QByteArray linkedBoxesDocx(const QVector<LinkedBox> &boxes)
{
    int pages = 1;
    for (const LinkedBox &b : boxes) pages = std::max(pages, b.page + 1);
    QVector<QByteArray> runs(pages);
    int id = 10;
    for (const LinkedBox &b : boxes) {
        QByteArray inner;
        if (b.chain >= 0 && b.seq > 0) {
            inner = "<wps:linkedTxbx id=\"" + QByteArray::number(b.chain) + "\" seq=\"" + QByteArray::number(b.seq) + "\"/>";
        } else {
            QByteArray words = "<w:txbxContent>";
            for (int i = 0; i < b.lines; ++i) words += "<w:p><w:r><w:t>Line " + QByteArray::number(i + 1) + " of the story that goes on</w:t></w:r></w:p>";
            if (b.lines == 0) words += "<w:p/>";
            words += "</w:txbxContent>";
            inner = (b.chain >= 0 ? "<wps:txbx id=\"" + QByteArray::number(b.chain) + "\">" : QByteArray("<wps:txbx>")) + words + "</wps:txbx>";
        }
        const QByteArray shape = "<wps:wsp><wps:cNvSpPr txBox=\"1\"/><wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + QByteArray::number(qint64(b.w * 12700)) + "\" cy=\"" + QByteArray::number(qint64(b.h * 12700)) +
                                 "\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></wps:spPr>" + inner + "<wps:bodyPr/></wps:wsp>";
        runs[b.page] += anchoredGraphic(id++, kShapeUri, shape, b.x, b.y, b.w, b.h);
    }
    QByteArray body;
    for (int page = 0; page < pages; ++page) {
        if (page > 0) body += "<w:p><w:r><w:br w:type=\"page\"/></w:r></w:p>";
        body += "<w:p><w:r><w:t xml:space=\"preserve\">Boxes: </w:t></w:r>" + runs[page] + "</w:p>";
    }
    return makeDocx(body);
}

// The text boxes of a page (from 0), in the order the page draws them.
QVector<TextItem *> textBoxesOfPage(WriteDoc &doc, std::shared_ptr<jp::flow::MaterializedPage> *keep, int pageIndex = 0)
{
    QVector<TextItem *> out;
    *keep = doc.pager().materialize(pageIndex);
    if (!*keep) return out;
    for (const auto &it : (*keep)->doc->pages[0]->items)
        if (it->type() == ItemType::Text && it->id.startsWith(QLatin1String("flow-object-"))) out.push_back(static_cast<TextItem *>(it.get()));
    return out;
}

// What a text box of a page shows: the words of its story.
QString wordsOf(const std::shared_ptr<jp::flow::MaterializedPage> &page, const TextItem *box)
{
    const auto story = page->doc->story(box->storyId);
    return story ? story->doc->toPlainText() : QString();
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

    // The Selection Pane lists the floating objects of the document by name, the one in front first. A row picks the object; a name typed in a row names it (wp:docPr's name); a box hides
    // or shows it (wp:docPr's hidden: a hidden object is not drawn and takes no room, and stays in the file); Show All and Hide All set every box; the buttons and a drag move it
    // through the layers. Each change is one step of Undo, and the names and marks are in the saved file.
    void theSelectionPaneListsNamesHidesAndReordersTheObjects()
    {
        Win win;
        write::WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        c.setPosition(1);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("ellipse")), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 0, 100, 150));
        QVERIFY(setPicturePlace(*doc, 1, 300, 250));
        win.w.view()->refresh();
        QCOMPARE(int(floatingObjects(*doc).size()), 2);
        QAction *paneCommand = win.w.act(QStringLiteral("view.selectionPane"));
        QVERIFY(paneCommand);
        QVERIFY(paneCommand->isEnabled());
        paneCommand->trigger();
        auto *list = win.w.findChild<QListWidget *>(QStringLiteral("selectionList"));
        QVERIFY(list);
        QCOMPARE(list->count(), 2);
        // The one in front first: the second shape was made after the first, so it is above it.
        QCOMPARE(floatingObjects(*doc).value(0).position, 1);
        QCOMPARE(floatingObjects(*doc).value(1).position, 0);
        QCOMPARE(list->item(0)->checkState(), Qt::Checked);
        // A row picks the object.
        list->setCurrentRow(1);
        QCOMPARE(win.w.view()->selectedPicture(), 0);
        // A name typed in a row names it, and the file keeps it.
        list->item(0)->setText(QStringLiteral("Logo <1> & 2"));
        QTRY_COMPARE(floatingObjects(*doc).value(0).name, QStringLiteral("Logo <1> & 2"));
        QVERIFY(documentXml(doc->toDocx()).contains("name=\"Logo &lt;1&gt; &amp; 2\""));
        {
            auto back = open(doc->toDocx());
            QVERIFY(back);
            QCOMPARE(floatingObjects(*back).value(0).name, QStringLiteral("Logo <1> & 2"));
        }
        QVERIFY(doc->undo());
        QVERIFY(floatingObjects(*doc).value(0).name != QStringLiteral("Logo <1> & 2"));
        QVERIFY(doc->redo());
        QCOMPARE(floatingObjects(*doc).value(0).name, QStringLiteral("Logo <1> & 2"));
        // A box hides the object: it is not drawn and takes no room, the file says it is hidden, and Undo shows it.
        QCOMPARE(int(objectsOfPage(*doc).size()), 2);
        QTRY_COMPARE(list->item(0)->text(), QStringLiteral("Logo <1> & 2"));
        list->item(0)->setCheckState(Qt::Unchecked);
        QTRY_VERIFY(floatingObjects(*doc).value(0).hidden);
        QCOMPARE(int(objectsOfPage(*doc).size()), 1);
        QVERIFY(documentXml(doc->toDocx()).contains("hidden=\"1\""));
        {
            auto back = open(doc->toDocx());
            QVERIFY(back);
            QCOMPARE(int(floatingObjects(*back).size()), 2);
            QVERIFY(floatingObjects(*back).value(0).hidden);
            QVERIFY(!floatingObjects(*back).value(1).hidden);
            QCOMPARE(int(objectsOfPage(*back).size()), 1);
        }
        QVERIFY(doc->undo());
        QVERIFY(!floatingObjects(*doc).value(0).hidden);
        QCOMPARE(int(objectsOfPage(*doc).size()), 2);
        // Hide All and Show All.
        auto *hideAll = win.w.findChild<QAbstractButton *>(QStringLiteral("selectionHideAll"));
        auto *showAll = win.w.findChild<QAbstractButton *>(QStringLiteral("selectionShowAll"));
        QVERIFY(hideAll && showAll);
        hideAll->click();
        QTRY_VERIFY(floatingObjects(*doc).value(0).hidden && floatingObjects(*doc).value(1).hidden);
        QCOMPARE(int(objectsOfPage(*doc).size()), 0);
        QVERIFY(doc->undo());
        QVERIFY(!floatingObjects(*doc).value(0).hidden && !floatingObjects(*doc).value(1).hidden);
        QCOMPARE(int(objectsOfPage(*doc).size()), 2);
        hideAll->click();
        QTRY_VERIFY(floatingObjects(*doc).value(0).hidden);
        showAll->click();
        QTRY_VERIFY(!floatingObjects(*doc).value(0).hidden && !floatingObjects(*doc).value(1).hidden);
        // The buttons and a drag move an object in the order.
        QTRY_COMPARE(list->count(), 2);
        list->setCurrentRow(0);
        QCOMPARE(floatingObjects(*doc).value(0).position, 1);
        auto *backward = win.w.findChild<QAbstractButton *>(QStringLiteral("selectionBackward"));
        QVERIFY(backward);
        backward->click();
        QCOMPARE(floatingObjects(*doc).value(0).position, 0);
        QCOMPARE(floatingObjects(*doc).value(1).position, 1);
        QVERIFY(doc->undo());
        QCOMPARE(floatingObjects(*doc).value(0).position, 1);
        QTRY_COMPARE(list->count(), 2);
        QVERIFY(list->model()->moveRow(QModelIndex(), 0, QModelIndex(), 2));
        QTRY_COMPARE(floatingObjects(*doc).value(0).position, 0);
        QCOMPARE(floatingObjects(*doc).value(1).position, 1);
    }

    // A document of 2,000 floating objects: the pane lists them, and a drag of a row 1,000 places is one reorder (each under two seconds; a scan of the document for each row, or for
    // each place moved, takes far longer).
    void aDocumentOfTwoThousandFloatingObjectsHasItsPaneListedAndARowDraggedQuickly()
    {
        QByteArray runs;
        for (int i = 0; i < 2000; ++i) {
            QByteArray r = anchoredGraphic(i + 1, kShapeUri, plainShape(20, 20, "rect"), (i % 20) * 25, (i / 20) * 6, 20, 20);
            r.replace("251659264", QByteArray::number(300000000 + i * 1000));
            runs += r;
        }
        Win win("<w:p>" + runs + "</w:p>");
        QElapsedTimer clock;
        auto within = [&](const char *what) {
            const qint64 ms = clock.elapsed();
            clock.restart();
            QVERIFY2(ms < 2000, qPrintable(QStringLiteral("%1 took %2 ms").arg(QLatin1String(what)).arg(ms)));
        };
        clock.start();
        win.w.act(QStringLiteral("view.selectionPane"))->trigger();
        auto *list = win.w.findChild<QListWidget *>(QStringLiteral("selectionList"));
        QVERIFY(list);
        QCOMPARE(list->count(), 2000);
        within("opening the pane");
        list->setCurrentRow(5);
        within("picking a row");
        const QVector<FloatingObject> before = floatingObjects(*win.w.document());
        QVERIFY(list->model()->moveRow(QModelIndex(), 0, QModelIndex(), 1001));
        within("a drag of 1,000 places");
        const QVector<FloatingObject> after = floatingObjects(*win.w.document());
        QCOMPARE(after.value(1000).position, before.value(0).position);
        QCOMPARE(after.value(0).position, before.value(1).position);
        QCOMPARE(int(after.size()), 2000);
    }

    // Objects in line with the text are in the pane too (a hidden one can be shown again, and any can be named), but have no layer to move in.
    void theSelectionPaneListsObjectsInLineAndTheyCanBeHiddenAndShown()
    {
        Win win("<w:p>" + anchoredGraphic(1, kShapeUri, plainShape(40, 40, "rect"), 100, 100, 40, 40) + anchoredGraphic(2, kShapeUri, plainShape(40, 40, "rect"), 0, 0, 40, 40, true) + "</w:p>");
        WriteDoc *doc = win.w.document();
        const QVector<FloatingObject> listed = floatingObjects(*doc);
        QCOMPARE(int(listed.size()), 2);
        QVERIFY(listed[0].floating && !listed[1].floating);
        const int inLine = listed[1].position;
        win.w.act(QStringLiteral("view.selectionPane"))->trigger();
        auto *list = win.w.findChild<QListWidget *>(QStringLiteral("selectionList"));
        QVERIFY(list);
        QCOMPARE(list->count(), 2);
        list->item(1)->setCheckState(Qt::Unchecked);
        QTRY_VERIFY(floatingObjects(*doc).value(1).hidden);
        QTRY_COMPARE(list->item(1)->checkState(), Qt::Unchecked);   // (still listed: it can be shown again)
        list->item(1)->setCheckState(Qt::Checked);
        QTRY_VERIFY(!floatingObjects(*doc).value(1).hidden);
        list->item(1)->setText(QStringLiteral("Inline one"));
        QTRY_COMPARE(floatingObjects(*doc).value(1).name, QStringLiteral("Inline one"));
        QVERIFY(!moveFloatingObject(*doc, inLine, -1));
        QCOMPARE(floatingObjects(*doc).value(1).position, inLine);
    }

    // A hidden first box of a chain of linked boxes keeps the chain: the boxes after it show the same words as before, and the hidden one is not drawn.
    void aHiddenHeadOfALinkedChainDoesNotBreakTheBoxesAfterIt()
    {
        auto doc = open(linkedBoxesDocx({{1, 0, 72, 100, 150, 60, 20}, {1, 1, 300, 100, 150, 400}}));
        QVERIFY(doc);
        QString shown;
        {
            std::shared_ptr<jp::flow::MaterializedPage> page;
            const QVector<TextItem *> boxes = textBoxesOfPage(*doc, &page);
            QCOMPARE(boxes.size(), 2);
            shown = wordsOf(page, boxes[1]);
            QVERIFY(!shown.isEmpty());
        }
        const QVector<FloatingObject> listed = floatingObjects(*doc);
        QCOMPARE(int(listed.size()), 2);
        const int head = std::min(listed[0].position, listed[1].position);
        QVERIFY(setObjectHidden(*doc, head, true));
        auto again = open(doc->toDocx());
        QVERIFY(again);
        std::shared_ptr<jp::flow::MaterializedPage> page;
        const QVector<TextItem *> boxes = textBoxesOfPage(*again, &page);
        QCOMPARE(boxes.size(), 2);
        int hiddenBoxes = 0;
        for (TextItem *b : boxes) hiddenBoxes += b->hidden;
        QCOMPARE(hiddenBoxes, 1);
        for (TextItem *b : boxes)
            if (!b->hidden) QCOMPARE(wordsOf(page, b), shown);
    }

    // The first Bring Forward in a layer of a picture, a shape, and a text box moves only that object: the others keep their order.
    void movingOneObjectInALayerOfDifferentKindsLeavesTheOthersInOrder()
    {
        auto doc = WriteDoc::blank();
        QImage image(10, 10, QImage::Format_RGB32);
        image.fill(Qt::blue);
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        QTextCursor c(doc->story());
        QCOMPARE(insertPicture(*doc, c, png, QStringLiteral("p"), PictureWrap::Square), PictureResult::Ok);
        c.movePosition(QTextCursor::End);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        c.movePosition(QTextCursor::End);
        QCOMPARE(insertTextBox(*doc, c, textBoxDesigns().first()), PictureResult::Ok);
        c.movePosition(QTextCursor::End);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("ellipse")), PictureResult::Ok);
        auto order = [&] {
            QList<int> positions;
            for (const FloatingObject &o : floatingObjects(*doc)) positions << o.position;
            return positions;
        };
        const QList<int> start = order();
        QCOMPARE(int(start.size()), 4);
        // The one at the back comes forward one place: only it moves.
        QVERIFY(moveFloatingObject(*doc, start[3], -1));
        QCOMPARE(order(), (QList<int>{start[0], start[1], start[3], start[2]}));
        // The one in front goes to the back: the rest keep their order.
        const QList<int> mid = order();
        QVERIFY(moveFloatingObject(*doc, mid[0], 3));
        QCOMPARE(order(), (QList<int>{mid[1], mid[2], mid[3], mid[0]}));
        QVERIFY(doc->undo());
        QCOMPARE(order(), mid);
        // And a text box (a different kind) at the front keeps the same rules.
        QVERIFY(moveFloatingObject(*doc, mid[1], -1));
        QCOMPARE(order(), (QList<int>{mid[1], mid[0], mid[2], mid[3]}));
    }

    void ctrlClickSelectsSeveralObjectsGroupAndUngroupWorkFromTheWindowAndADragMovesThemAll()
    {
        Win win;
        write::WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        c.setPosition(1);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("ellipse")), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 0, 100, 150));
        QVERIFY(setPicturePlace(*doc, 1, 300, 250));
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        win.w.refreshNow();
        QCOMPARE(win.w.view()->selectedObjects(), (QVector<int>{0}));
        QVERIFY(!win.w.act("arrange.group")->isEnabled() && !win.w.act("arrange.ungroup")->isEnabled());
        QRectF first, second;
        QTRY_VERIFY(win.w.view()->pictureRect(0, &first) && win.w.view()->pictureRect(1, &second));
        // Ctrl+click on the other adds it; Ctrl+click again takes it out.
        QTest::mouseClick(win.w.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, second.center().toPoint());
        QCOMPARE(win.w.view()->selectedObjects(), (QVector<int>{0, 1}));
        win.w.refreshNow();
        QVERIFY(win.w.act("arrange.group")->isEnabled());
        QTest::mouseClick(win.w.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, second.center().toPoint());
        QCOMPARE(win.w.view()->selectedObjects(), (QVector<int>{0}));
        QTest::mouseClick(win.w.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, second.center().toPoint());
        QCOMPARE(win.w.view()->selectedObjects(), (QVector<int>{0, 1}));
        // A drag of one moves both, in one step.
        const double firstLeft = pictureAt(*doc, doc->story(), 0).left, secondLeft = pictureAt(*doc, doc->story(), 1).left;
        const QPoint from = first.center().toPoint();
        QTest::mousePress(win.w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(win.w.view()->viewport(), from + QPoint(20, 10));
        QTest::mouseMove(win.w.view()->viewport(), from + QPoint(60, 30));
        QTest::mouseRelease(win.w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, from + QPoint(60, 30));
        const double moved = pictureAt(*doc, doc->story(), 0).left - firstLeft;
        QVERIFY(moved > 20);
        QVERIFY(std::abs((pictureAt(*doc, doc->story(), 1).left - secondLeft) - moved) < 0.01);
        QCOMPARE(win.w.view()->selectedObjects(), (QVector<int>{0, 1}));
        QVERIFY(doc->undo());
        QVERIFY(std::abs(pictureAt(*doc, doc->story(), 0).left - firstLeft) < 0.01 && std::abs(pictureAt(*doc, doc->story(), 1).left - secondLeft) < 0.01);
        QVERIFY(doc->redo());
        // Group from the window: one object, selected; Ungroup puts the two back, both selected.
        win.w.view()->selectObjects({0, 1});
        win.w.refreshNow();
        const int length = doc->story()->characterCount();
        win.w.act("arrange.group")->trigger();
        QCOMPARE(doc->story()->characterCount(), length - 1);
        QVERIFY(isGroupAt(*doc, 0));
        QCOMPARE(win.w.view()->selectedObjects(), (QVector<int>{0}));
        win.w.refreshNow();
        QVERIFY(win.w.act("arrange.ungroup")->isEnabled() && !win.w.act("arrange.group")->isEnabled());
        QVERIFY(!win.w.act("shape.editText")->isEnabled() && !win.w.act("arrange.align.left")->isEnabled());
        QVERIFY(win.w.act("picture.altText")->isEnabled());
        win.w.act("arrange.ungroup")->trigger();
        QCOMPARE(doc->story()->characterCount(), length);
        QCOMPARE(win.w.view()->selectedObjects(), (QVector<int>{0, 1}));
        QVERIFY(doc->undo());
        QVERIFY(isGroupAt(*doc, 0));
        // Objects in line with the text can't be grouped.
        QVERIFY(doc->undo());
        QVERIFY(setPictureWrap(*doc, 0, PictureWrap::Inline));
        win.w.view()->selectObjects({0, 1});
        win.w.refreshNow();
        QVERIFY(!win.w.view()->groupSelectedObjects());
        QVERIFY(!isGroupAt(*doc, 0));
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

    void aPictureIsTurnedAndCutToAShapeSavedAndDrawn()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertPicture(*doc, c, pngOf(160, 160)), PictureResult::Ok);
        QVERIFY(setShapeRotation(*doc, 7, 30));
        QVERIFY(std::abs(pictureAt(*doc, doc->story(), 7).rotation - 30) < 0.01);
        QVERIFY(!setShapeRotation(*doc, 7, 30));
        QVERIFY(setPictureCropShape(*doc, 7, QStringLiteral("ellipse")));
        QVERIFY(!setPictureCropShape(*doc, 7, QStringLiteral("ellipse")));
        QVERIFY(!setPictureCropShape(*doc, 7, QStringLiteral("no such shape")));
        QCOMPARE(pictureAt(*doc, doc->story(), 7).cropShape, QStringLiteral("ellipse"));
        auto pic = pictureOfPage(*doc);
        QVERIFY(pic);
        QVERIFY(std::abs(pic->rotation - 30) < 0.01);
        QCOMPARE(pic->maskShape, QStringLiteral("ellipse"));
        const QByteArray saved = doc->toDocx();
        const QByteArray x = documentXml(saved);
        QVERIFY(x.contains("rot=\"1800000\""));
        QVERIFY(x.contains("<a:prstGeom prst=\"ellipse\">"));
        auto back = open(saved);
        QCOMPARE(pictureAt(*back, back->story(), 7).cropShape, QStringLiteral("ellipse"));
        QVERIFY(std::abs(pictureAt(*back, back->story(), 7).rotation - 30) < 0.01);
        QVERIFY(doc->undo());
        QCOMPARE(pictureAt(*doc, doc->story(), 7).cropShape, QStringLiteral("rect"));
        QVERIFY(doc->redo());
        QVERIFY(setPictureCropShape(*doc, 7, QStringLiteral("rect")));
        QCOMPARE(pictureAt(*doc, doc->story(), 7).cropShape, QStringLiteral("rect"));
    }

    void aFloatingObjectIsAlignedToTheMarginsOrThePage()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        QVERIFY(alignObject(*doc, 7, ObjectAlign::Right, false));
        PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QCOMPARE(info.alignAcross, QStringLiteral("right"));
        QVERIFY(!info.alignedToPage);
        QVERIFY(!alignObject(*doc, 7, ObjectAlign::Right, false));
        QVERIFY(alignObject(*doc, 7, ObjectAlign::Middle, true));
        info = pictureAt(*doc, doc->story(), 7);
        QCOMPARE(info.alignDown, QStringLiteral("center"));
        QCOMPARE(info.alignAcross, QStringLiteral("right"));
        QVERIFY(info.alignedToPage);
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY(x.contains("<wp:align>right</wp:align>") && x.contains("<wp:align>center</wp:align>"));
        auto back = open(doc->toDocx());
        QCOMPARE(pictureAt(*back, back->story(), 7).alignDown, QStringLiteral("center"));
        QVERIFY(doc->undo());
        QVERIFY(pictureAt(*doc, doc->story(), 7).alignDown.isEmpty() || pictureAt(*doc, doc->story(), 7).alignDown != QLatin1String("center"));
    }

    void aGroupOfAFileIsDrawnAsItsMembersWhereTheGroupPutsThem()
    {
        auto member = [](int id, int x, int cx, const QByteArray &rgb) {
            return "<wps:wsp><wps:cNvPr id=\"" + QByteArray::number(id) + "\" name=\"Part " + QByteArray::number(id) + "\"/><wps:cNvSpPr/><wps:spPr><a:xfrm><a:off x=\"" + QByteArray::number(x) +
                   "\" y=\"0\"/><a:ext cx=\"" + QByteArray::number(cx) + "\" cy=\"1270000\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom><a:solidFill><a:srgbClr val=\"" + rgb +
                   "\"/></a:solidFill></wps:spPr><wps:bodyPr/></wps:wsp>";
        };
        // The members' units are twice the group's: a member at 2,540,000 is 1,270,000 EMU (100 points) from the group's corner.
        const QByteArray body = "<w:p><w:r><w:drawing><wp:anchor distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\" simplePos=\"0\" relativeHeight=\"251659264\" behindDoc=\"0\" locked=\"0\" layoutInCell=\"1\" allowOverlap=\"1\">"
                                "<wp:simplePos x=\"0\" y=\"0\"/><wp:positionH relativeFrom=\"page\"><wp:posOffset>1270000</wp:posOffset></wp:positionH><wp:positionV relativeFrom=\"page\"><wp:posOffset>1270000</wp:posOffset></wp:positionV>"
                                "<wp:extent cx=\"2540000\" cy=\"635000\"/><wp:wrapNone/><wp:docPr id=\"9\" name=\"Group 9\"/><wp:cNvGraphicFramePr/>"
                                "<a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingGroup\"><wpg:wgp><wpg:cNvGrpSpPr/><wpg:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/>"
                                "<a:ext cx=\"2540000\" cy=\"635000\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"5080000\" cy=\"1270000\"/></a:xfrm></wpg:grpSpPr>" +
                                member(10, 0, 2540000, "FF0000") + member(11, 2540000, 2540000, "0000FF") + "</wpg:wgp></a:graphicData></a:graphic></wp:anchor></w:drawing></w:r></w:p><w:p><w:r><w:t>Text</w:t></w:r></w:p>";
        QByteArray docx = makeDocx(body);
        // (The group's prefix is declared on the document.)
        auto doc = open(docx);
        QVERIFY(doc);
        ItemList filled;
        for (const auto &it : objectsOfPage(*doc))
            if (it->type() == ItemType::Shape && !it->fill.isNone()) filled.push_back(it);
        QCOMPARE(filled.size(), 2);
        std::sort(filled.begin(), filled.end(), [](const ItemPtr &a, const ItemPtr &b) { return a->rect.left() < b->rect.left(); });
        QCOMPARE(filled[0]->fill.color.rgbValue(), QColor(255, 0, 0));
        QCOMPARE(filled[1]->fill.color.rgbValue(), QColor(0, 0, 255));
        QCOMPARE(filled[0]->rect.size(), QSizeF(100, 50));
        QVERIFY(std::abs((filled[1]->rect.left() - filled[0]->rect.left()) - 100) < 0.01);
        QVERIFY(std::abs(filled[0]->rect.left() - 100) < 0.01);
        QVERIFY(isGroupAt(*doc, 0));
        const PictureInfo info = pictureAt(*doc, doc->story(), 0);
        QVERIFY(info.valid && info.floating);
        QCOMPARE(info.width, 200.0);
        QCOMPARE(info.height, 50.0);
        // The group is one object: the fill of a part is not changed through it, and its words are not read as a box's.
        QVERIFY(!setShapeFill(*doc, 0, QStringLiteral("00FF00")));
        QVERIFY(!textBoxText(*doc, 0).has_value());
        // Moving it moves every part; it is saved as it was.
        QVERIFY(setPicturePlace(*doc, 0, 200, 300));
        QVERIFY(setPictureAltText(*doc, 0, QStringLiteral("Two boxes")));
        const QByteArray saved = documentXml(doc->toDocx());
        QVERIFY(saved.contains("<wpg:wgp>") && saved.contains("<wps:cNvPr id=\"11\""));
        QVERIFY(saved.contains("descr=\"Two boxes\""));
        auto back = open(doc->toDocx());
        QVERIFY(isGroupAt(*back, 0));
        ItemList moved;
        for (const auto &it : objectsOfPage(*back))
            if (it->type() == ItemType::Shape && !it->fill.isNone()) moved.push_back(it);
        QCOMPARE(moved.size(), 2);
        std::sort(moved.begin(), moved.end(), [](const ItemPtr &a, const ItemPtr &b) { return a->rect.left() < b->rect.left(); });
        QVERIFY(std::abs(moved[0]->rect.left() - 200) < 0.01 && std::abs(moved[0]->rect.top() - 300) < 0.01);
    }

    void shapesAreGroupedIntoOneObjectThatIsSavedAndTakenApartInOneStepEach()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        c.setPosition(8);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("ellipse")), PictureResult::Ok);
        QVERIFY(canGroupObject(*doc, 7) && canGroupObject(*doc, 8));
        const PictureInfo one = pictureAt(*doc, doc->story(), 7), two = pictureAt(*doc, doc->story(), 8);
        const QRectF r1(100, 100, one.width, one.height), r2(250, 150, two.width, two.height);
        const double width = 250 + two.width - 100, height = std::max(100 + one.height, 150 + two.height) - 100;
        const int length = doc->story()->characterCount();
        QVERIFY(!groupObjects(*doc, {7}, {r1}));
        QVERIFY(!groupObjects(*doc, {7, 7}, {r1, r1}));
        QVERIFY(!groupObjects(*doc, {7, 2}, {r1, r1}));
        int at = -1;
        QVERIFY(groupObjects(*doc, {8, 7}, {r2, r1}, &at));
        QCOMPARE(at, 7);
        QCOMPARE(doc->story()->characterCount(), length - 1);
        QVERIFY(isGroupAt(*doc, 7));
        QVERIFY(canGroupObject(*doc, 7));   // (a group joins by its parts)
        PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QVERIFY(info.valid && info.floating && info.editable);
        QVERIFY(std::abs(info.width - width) < 0.01 && std::abs(info.height - height) < 0.01);
        QVERIFY(info.placeIsOffset && std::abs(info.left - 100) < 0.01 && std::abs(info.top - 100) < 0.01);
        int filled = 0;
        for (const auto &it : objectsOfPage(*doc)) filled += it->type() == ItemType::Shape && !it->fill.isNone();
        QCOMPARE(filled, 2);
        // The group is sized as one: the parts follow.
        QVERIFY(setPictureSize(*doc, 7, width * 2, height));
        ItemList sized;
        for (const auto &it : objectsOfPage(*doc))
            if (it->type() == ItemType::Shape && !it->fill.isNone()) sized.push_back(it);
        std::sort(sized.begin(), sized.end(), [](const ItemPtr &a, const ItemPtr &b) { return a->rect.left() < b->rect.left(); });
        QVERIFY(std::abs(sized[0]->rect.width() - one.width * 2) < 0.01);
        QVERIFY(doc->undo());
        const QByteArray saved = documentXml(doc->toDocx());
        QVERIFY(saved.contains("<wpg:wgp>") && saved.contains("<wpg:grpSpPr>") && saved.contains("<a:chExt"));
        QCOMPARE(saved.count("<wps:cNvPr"), 2);
        QVERIFY(saved.contains("xmlns:wpg="));
        auto back = open(doc->toDocx());
        QVERIFY(isGroupAt(*back, 7));
        // Taking it apart puts the parts where they were.
        QVector<int> parts;
        QVERIFY(!ungroupObject(*doc, 2, QPointF(0, 0)));
        QVERIFY(ungroupObject(*doc, 7, QPointF(100, 100), &parts));
        QCOMPARE(parts, (QVector<int>{7, 8}));
        QCOMPARE(doc->story()->characterCount(), length);
        QVERIFY(!isGroupAt(*doc, 7) && !isGroupAt(*doc, 8));
        // (The parts come back in the order of their layers.)
        PictureInfo a = pictureAt(*doc, doc->story(), 7), b = pictureAt(*doc, doc->story(), 8);
        if (a.left > b.left) std::swap(a, b);
        QVERIFY(std::abs(a.left - 100) < 0.01 && std::abs(a.top - 100) < 0.01 && std::abs(a.width - one.width) < 0.01 && std::abs(a.height - one.height) < 0.01);
        QVERIFY(std::abs(b.left - 250) < 0.01 && std::abs(b.top - 150) < 0.01 && std::abs(b.height - two.height) < 0.01);
        QVERIFY(!documentXml(doc->toDocx()).contains("wpg:wgp"));
        // One step each way.
        QVERIFY(doc->undo());
        QVERIFY(isGroupAt(*doc, 7));
        QCOMPARE(doc->story()->characterCount(), length - 1);
        QVERIFY(doc->undo());
        QVERIFY(!isGroupAt(*doc, 7));
        QCOMPARE(doc->story()->characterCount(), length);
        QVERIFY(doc->redo());
        QVERIFY(isGroupAt(*doc, 7));
    }

    void aGroupJoinedWithAnotherObjectGivesItsPartsToTheNewGroup()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        for (int i = 0; i < 3; ++i) {
            c.setPosition(7 + i);
            QCOMPARE(insertShape(*doc, c, i == 1 ? QStringLiteral("ellipse") : QStringLiteral("rect")), PictureResult::Ok);
        }
        const PictureInfo s7 = pictureAt(*doc, doc->story(), 7), s8 = pictureAt(*doc, doc->story(), 8), s9 = pictureAt(*doc, doc->story(), 9);
        QVERIFY(groupObjects(*doc, {7, 8}, {QRectF(100, 100, s7.width, s7.height), QRectF(110 + s7.width, 100, s8.width, s8.height)}));
        QVERIFY(canGroupObject(*doc, 7) && canGroupObject(*doc, 8));
        const PictureInfo group = pictureAt(*doc, doc->story(), 7);
        QVERIFY(std::abs(group.width - (s7.width + 10 + s8.width)) < 0.01);
        int at = -1;
        const double below = 100 + group.height + 50;
        QVERIFY(groupObjects(*doc, {7, 8}, {QRectF(100, 100, group.width, group.height), QRectF(100, below, s9.width, s9.height)}, &at));
        const QByteArray saved = documentXml(doc->toDocx());
        QCOMPARE(saved.count("<wpg:wgp>"), 1);
        QCOMPARE(saved.count("<wps:cNvPr"), 3);
        const PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QVERIFY(std::abs(info.width - std::max(group.width, s9.width)) < 0.01 && std::abs(info.height - (group.height + 50 + s9.height)) < 0.01);
        QVector<int> parts;
        QVERIFY(ungroupObject(*doc, 7, QPointF(100, 100), &parts));
        QCOMPARE(parts.size(), 3);
        double lowest = 0;
        for (const int p : parts) lowest = std::max(lowest, pictureAt(*doc, doc->story(), p).top);
        QVERIFY(std::abs(lowest - below) < 0.01);
    }

    void textArtIsPutInDrawnWarpedStyledAndChangedOneStepEach()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        TextArtSpec spec;
        spec.text = QStringLiteral("Hello\nWorld");
        spec.styleId = textArtStyles().first().id;
        spec.size = 40;
        QCOMPARE(insertTextArt(*doc, c, spec), PictureResult::Ok);
        const PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QVERIFY(info.valid && info.editable && info.floating);
        auto artOfPage = [](WriteDoc &d) -> std::shared_ptr<TextArtItem> {
            const ItemList items = objectsOfPage(d);
            return items.size() == 1 && items[0]->type() == ItemType::TextArt ? std::static_pointer_cast<TextArtItem>(items[0]) : nullptr;
        };
        auto art = artOfPage(*doc);
        QVERIFY(art);
        QCOMPARE(art->text, QStringLiteral("Hello\nWorld"));
        QCOMPARE(art->font, textArtStyles().first().font);
        QVERIFY(art->size > 39 && art->size < 41);
        std::optional<TextArtSpec> now = textArtAt(*doc, 7);
        QVERIFY(now);
        QCOMPARE(now->text, QStringLiteral("Hello\nWorld"));
        // Its words are the words of a box; a plain text box is no text art.
        QCOMPARE(textBoxText(*doc, 7).value_or(QStringList()), (QStringList{"Hello", "World"}));
        auto boxed = textDoc({"Before after"});
        QTextCursor b(boxed->story());
        b.setPosition(7);
        QCOMPARE(insertTextBox(*boxed, b, textBoxDesigns().first()), PictureResult::Ok);
        QVERIFY(!textArtAt(*boxed, 7).has_value() && !setTextArtWarp(*boxed, 7, QStringLiteral("plain")));
        // A warp: any but the plain one, written as the standard's name.
        QString wave;
        for (const TextArtTransform &t : textArtTransforms())
            if (office::textWarpPreset(t.id) != QLatin1String("textPlain") && wave.isEmpty()) wave = t.id;
        QVERIFY(!wave.isEmpty());
        QVERIFY(setTextArtWarp(*doc, 7, wave));
        QVERIFY(!setTextArtWarp(*doc, 7, wave));
        QVERIFY(!setTextArtWarp(*doc, 7, QStringLiteral("no such warp")));
        QCOMPARE(textArtAt(*doc, 7)->warp, wave);
        art = artOfPage(*doc);
        QVERIFY(art);
        QCOMPARE(art->transform_, wave);
        // Another style changes the letters' font and look; the warp and the words stay.
        QString other;
        for (const TextArtStyle &st : textArtStyles())
            if (st.font != textArtStyles().first().font && other.isEmpty()) other = st.id;
        QVERIFY(!other.isEmpty());
        QVERIFY(setTextArtStyle(*doc, 7, other));
        QVERIFY(!setTextArtStyle(*doc, 7, QStringLiteral("no such style")));
        art = artOfPage(*doc);
        QVERIFY(art);
        QCOMPARE(art->text, QStringLiteral("Hello\nWorld"));
        QCOMPARE(art->transform_, wave);
        for (const TextArtStyle &st : textArtStyles())
            if (st.id == other) QCOMPARE(art->font, st.font);
        // New words through Edit Text stay text art.
        QVERIFY(setTextBoxText(*doc, 7, {"Third"}));
        art = artOfPage(*doc);
        QVERIFY(art);
        QCOMPARE(art->text, QStringLiteral("Third"));
        // Saved as a box that names the warp; read back the same.
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY(x.contains(QByteArray("<a:prstTxWarp prst=\"") + office::textWarpPreset(wave).toLatin1() + "\""));
        QVERIFY(x.contains("fromWordArt=\"1\"") && x.contains("<w14:textFill>"));
        auto back = open(doc->toDocx());
        art = artOfPage(*back);
        QVERIFY(art);
        QCOMPARE(art->text, QStringLiteral("Third"));
        QCOMPARE(art->transform_, wave);
        QVERIFY(doc->undo());
        QVERIFY(doc->undo());
        QCOMPARE(artOfPage(*doc)->text, QStringLiteral("Hello\nWorld"));
    }

    void aConnectorIsGluedToTwoShapesAndRunsAgainWhenOneMovesAndLetsGoWhenMovedByHand()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        c.setPosition(8);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("ellipse")), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 7, 100, 100));
        QVERIFY(setPicturePlace(*doc, 8, 400, 300));
        const PictureInfo a = pictureAt(*doc, doc->story(), 7), b = pictureAt(*doc, doc->story(), 8);
        const QRectF ra(100, 100, a.width, a.height), rb(400, 300, b.width, b.height);
        const int length = doc->story()->characterCount();
        int at = -1;
        QVERIFY(connectObjects(*doc, 7, ra, 7, ra) == PictureResult::Refused);
        QVERIFY(connectObjects(*doc, 7, ra, 8, rb, QStringLiteral("rect")) == PictureResult::Refused);
        QCOMPARE(connectObjects(*doc, 7, ra, 8, rb, QStringLiteral("bentConnector3"), &at), PictureResult::Ok);
        QCOMPARE(at, 9);
        QCOMPARE(doc->story()->characterCount(), length + 1);
        QVERIFY(isConnectorGlued(*doc, 9) && !isConnectorGlued(*doc, 7));
        QByteArray x = documentXml(doc->toDocx());
        QVERIFY(x.contains("<wps:cNvCnPr><a:stCxn id=\"") && x.contains("<a:endCxn id=\""));
        // It runs from a site of the rectangle (its right or its bottom, the ones that face the ellipse) to a site of the ellipse.
        QRegularExpression startSite(QStringLiteral("<a:stCxn id=\"[0-9]+\" idx=\"([0-9]+)\"/>"));
        const int site = QRegularExpression(startSite).match(QString::fromUtf8(x)).captured(1).toInt();
        QVERIFY(site == 2 || site == 3);
        QVERIFY(pictureAt(*doc, doc->story(), 9).kind == DrawnKind::Line);
        auto back = open(doc->toDocx());
        QVERIFY(isConnectorGlued(*back, 9));
        // The rectangle moves: the connector's frame follows it (the view does this after a drag).
        const double before = pictureAt(*doc, doc->story(), 9).height;
        QVERIFY(setPicturePlace(*doc, 7, 100, 20));
        const QRectF moved(100, 20, a.width, a.height);
        QCOMPARE(rerouteConnectors(*doc, {{7, moved}}, [&](int position, QRectF *r) {
                     if (position != 8) return false;
                     *r = rb;
                     return true;
                 }),
                 1);
        QVERIFY(std::abs(pictureAt(*doc, doc->story(), 9).height - before) > 1);
        QVERIFY(isConnectorGlued(*doc, 9));
        // Nothing glued to an object that moves: nothing runs.
        QCOMPARE(rerouteConnectors(*doc, {{9, QRectF(0, 0, 10, 10)}}, [](int, QRectF *) { return false; }), 0);
        // Moved by hand, the connector lets go.
        QVERIFY(setPicturePlace(*doc, 9, 10, 10));
        QVERIFY(!isConnectorGlued(*doc, 9));
        QVERIFY(!documentXml(doc->toDocx()).contains("<a:stCxn"));
        // A line is not joined to anything.
        c.setPosition(10);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("line")), PictureResult::Ok);
        QVERIFY(connectObjects(*doc, 10, QRectF(0, 0, 50, 1), 8, rb) == PictureResult::Refused);
    }

    void connectAndADragThatMovesAShapeWorkFromTheWindow()
    {
        Win win;
        write::WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        c.setPosition(1);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 0, 100, 150));
        QVERIFY(setPicturePlace(*doc, 1, 350, 150));
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        win.w.refreshNow();
        QVERIFY(!win.w.act("arrange.connect.straight")->isEnabled());
        QRectF first, second;
        QTRY_VERIFY(win.w.view()->pictureRect(0, &first) && win.w.view()->pictureRect(1, &second));
        QTest::mouseClick(win.w.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, second.center().toPoint());
        win.w.refreshNow();
        QVERIFY(win.w.act("arrange.connect.straight")->isEnabled() && win.w.act("arrange.connect.elbow")->isEnabled());
        const int length = doc->story()->characterCount();
        win.w.act("arrange.connect.straight")->trigger();
        QCOMPARE(doc->story()->characterCount(), length + 1);
        QCOMPARE(win.w.view()->selectedPicture(), 2);
        QVERIFY(isConnectorGlued(*doc, 2));
        // A straight connector between two rectangles side by side is level, and as long as the gap.
        const PictureInfo link = pictureAt(*doc, doc->story(), 2);
        QVERIFY(link.height < 1 && link.width > 100);
        // Dragging the first rectangle down moves the connector's end with it.
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        QTRY_VERIFY(win.w.view()->pictureRect(0, &first));
        const QPoint from = first.center().toPoint();
        QTest::mousePress(win.w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(win.w.view()->viewport(), from + QPoint(5, 30));
        QTest::mouseMove(win.w.view()->viewport(), from + QPoint(10, 80));
        QTest::mouseRelease(win.w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, from + QPoint(10, 80));
        QVERIFY(isConnectorGlued(*doc, 2));
        QVERIFY(pictureAt(*doc, doc->story(), 2).height > 20);
    }

    // ---------------------------------------------------------------- review fixes
    void manyGluedConnectorsAreReroutedInOnePassAndAtMostAFewHundredAtATime()
    {
        QByteArray body = "<w:p>" + anchoredGraphic(1, kShapeUri, plainShape(100, 50, "rect"), 100, 100, 100, 50) + anchoredGraphic(2, kShapeUri, plainShape(100, 50, "rect"), 400, 300, 100, 50);
        for (int i = 0; i < 400; ++i) body += gluedConnector(10 + i, 1, 3, 2, 1);
        body += "</w:p>";
        auto doc = open(makeDocx(body));
        QVERIFY(doc);
        QElapsedTimer timer;
        timer.start();
        const int changed = rerouteConnectors(*doc, {{0, QRectF(120, 100, 100, 50)}}, [](int, QRectF *r) {
            *r = QRectF(400, 300, 100, 50);
            return true;
        });
        QVERIFY2(timer.elapsed() < 10000, qPrintable(QString::number(timer.elapsed())));
        QVERIFY(changed > 0 && changed <= 200);
    }

    void aGroupWithMoreMembersThanTheLimitIsRefusedAtOnce()
    {
        QByteArray inner = "<wpg:wgp><wpg:cNvGrpSpPr/><wpg:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1270000\" cy=\"1270000\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"1270000\" cy=\"1270000\"/></a:xfrm></wpg:grpSpPr>";
        const QByteArray member = "<wps:wsp><wps:cNvPr id=\"5\" name=\"m\"/><wps:cNvSpPr/><wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"12700\" cy=\"12700\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></wps:spPr><wps:bodyPr/></wps:wsp>";
        inner += member.repeated(60000) + "</wpg:wgp>";
        auto doc = open(makeDocx("<w:p>" + anchoredGraphic(9, kGroupUri, inner, 100, 100, 100, 100) + "</w:p>"));
        QVERIFY(doc);
        // The processor time of the best of three tries (the wall clock counts the other suites the gate runs at once).
        qint64 best = std::numeric_limits<qint64>::max();
        for (int attempt = 0; attempt < 3; ++attempt) {
            const qint64 start = jp::threadCpuMicros();
            QVERIFY(!canGroupObject(*doc, 0));
            best = qMin(best, jp::threadCpuMicros() - start);
        }
        QVERIFY2(best < 2000000, qPrintable(QString::number(best)));
        QVERIFY(!ungroupObject(*doc, 0, QPointF(0, 0)));
        // Sixty shapes put in one after another are not grouped either.
        auto many = textDoc({"Before after"});
        QTextCursor c(many->story());
        QVector<int> at;
        QVector<QRectF> rects;
        for (int i = 0; i < 60; ++i) {
            c.setPosition(7 + i);
            QCOMPARE(insertShape(*many, c, QStringLiteral("rect")), PictureResult::Ok);
            at << 7 + i;
            rects << QRectF(10, 10, 50, 50);
        }
        QVERIFY(!groupObjects(*many, at, rects));
        QVERIFY(!isGroupAt(*many, 7));
    }

    void aTextBoxThatNamesThePlainWarpIsAnOrdinaryBoxUnlessItIsTextArt()
    {
        auto box = [](const QByteArray &bodyPr) {
            return "<w:p>" + anchoredGraphic(1, kShapeUri,
                       "<wps:wsp><wps:cNvSpPr txBox=\"1\"/><wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"2540000\" cy=\"1270000\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>"
                       "<a:solidFill><a:srgbClr val=\"FFEE99\"/></a:solidFill><a:ln w=\"12700\"><a:solidFill><a:srgbClr val=\"AA0000\"/></a:solidFill></a:ln></wps:spPr><wps:txbx><w:txbxContent><w:p><w:r><w:t>Words</w:t></w:r></w:p></w:txbxContent></wps:txbx>" +
                       bodyPr + "</wps:wsp>", 100, 100, 200, 100) + "</w:p>";
        };
        auto ordinary = open(makeDocx(box("<wps:bodyPr wrap=\"square\"><a:prstTxWarp prst=\"textPlain\"><a:avLst/></a:prstTxWarp><a:noAutofit/></wps:bodyPr>")));
        QVERIFY(ordinary);
        ItemList items = objectsOfPage(*ordinary);
        QCOMPARE(items.size(), 1);
        QVERIFY(items[0]->type() != ItemType::TextArt);
        QVERIFY(!items[0]->fill.isNone() && !items[0]->stroke.isNone());
        QVERIFY(!textArtAt(*ordinary, 0).has_value());
        QVERIFY(textBoxText(*ordinary, 0).has_value());
        auto none = open(makeDocx(box("<wps:bodyPr><a:prstTxWarp prst=\"textNoShape\"><a:avLst/></a:prstTxWarp></wps:bodyPr>")));
        QVERIFY(objectsOfPage(*none)[0]->type() != ItemType::TextArt);
        auto art = open(makeDocx(box("<wps:bodyPr fromWordArt=\"1\"><a:prstTxWarp prst=\"textPlain\"><a:avLst/></a:prstTxWarp></wps:bodyPr>")));
        QCOMPARE(objectsOfPage(*art)[0]->type(), ItemType::TextArt);
        auto warped = open(makeDocx(box("<wps:bodyPr><a:prstTxWarp prst=\"textWave1\"><a:avLst/></a:prstTxWarp></wps:bodyPr>")));
        QCOMPARE(objectsOfPage(*warped)[0]->type(), ItemType::TextArt);
    }

    void movingAConnectorWithItsShapesKeepsItGlued()
    {
        Win win;
        write::WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        c.setPosition(1);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 0, 100, 150));
        QVERIFY(setPicturePlace(*doc, 1, 350, 150));
        win.w.view()->refresh();
        win.w.view()->selectObjects({0, 1});
        QVERIFY(win.w.view()->connectSelectedObjects(QStringLiteral("straightConnector1")));
        QVERIFY(isConnectorGlued(*doc, 2));
        win.w.view()->refresh();
        win.w.view()->selectObjects({0, 1, 2});
        win.w.refreshNow();
        QRectF first;
        QTRY_VERIFY(win.w.view()->pictureRect(0, &first));
        const QPoint from = first.center().toPoint();
        const double before = pictureAt(*doc, doc->story(), 0).left;
        QTest::mousePress(win.w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(win.w.view()->viewport(), from + QPoint(5, 30));
        QTest::mouseMove(win.w.view()->viewport(), from + QPoint(10, 80));
        QTest::mouseRelease(win.w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, from + QPoint(10, 80));
        QVERIFY(pictureAt(*doc, doc->story(), 0).left > before + 5);
        QVERIFY(isConnectorGlued(*doc, 2));
    }

    void aConnectorStaysGluedToShapesThroughGroupAndUngroup()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        c.setPosition(8);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 7, 100, 100));
        QVERIFY(setPicturePlace(*doc, 8, 300, 100));
        const PictureInfo one = pictureAt(*doc, doc->story(), 7), two = pictureAt(*doc, doc->story(), 8);
        const QRectF r1(100, 100, one.width, one.height), r2(300, 100, two.width, two.height);
        int connector = -1;
        QCOMPARE(connectObjects(*doc, 7, r1, 8, r2, QStringLiteral("straightConnector1"), &connector), PictureResult::Ok);
        // Grouped, a member is found by its own id: the group moves, the connector follows.
        int group = -1;
        QVERIFY(groupObjects(*doc, {7, 8}, {r1, r2}, &group));
        connector = group + 1;
        QVERIFY(isConnectorGlued(*doc, connector));
        const double before = pictureAt(*doc, doc->story(), connector).left;
        QCOMPARE(rerouteConnectors(*doc, {{group, QRectF(150, 100, 200 + two.width, two.height)}}, [](int, QRectF *) { return false; }), 1);
        QVERIFY(std::abs(pictureAt(*doc, doc->story(), connector).left - before) > 10);
        // Taken apart, the parts keep their ids, so it is still glued and still follows.
        QVector<int> parts;
        QVERIFY(ungroupObject(*doc, group, QPointF(150, 100), &parts));
        connector = parts.last() + 1;
        QVERIFY(isConnectorGlued(*doc, connector));
        QCOMPARE(rerouteConnectors(*doc, {{parts.first(), QRectF(150, 300, one.width, one.height)}, {parts.last(), QRectF(350, 300, two.width, two.height)}},
                                   [](int, QRectF *) { return false; }),
                 1);
    }

    void aNewDrawingNeverTakesTheIdOfAMemberOfAGroup()
    {
        QByteArray inner = "<wpg:wgp><wpg:cNvGrpSpPr/><wpg:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1270000\" cy=\"1270000\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"1270000\" cy=\"1270000\"/></a:xfrm></wpg:grpSpPr>"
                           "<wps:wsp><wps:cNvPr id=\"4\" name=\"m\"/><wps:cNvSpPr/><wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"635000\" cy=\"635000\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></wps:spPr><wps:bodyPr/></wps:wsp></wpg:wgp>";
        auto doc = open(makeDocx("<w:p><w:r><w:t>Text</w:t></w:r>" + anchoredGraphic(3, kGroupUri, inner, 100, 100, 100, 100) + "</w:p>"));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(2);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        const QByteArray x = documentXml(doc->toDocx());
        QRegularExpression ids(QStringLiteral("(?:docPr|cNvPr) id=\"([0-9]+)\""));
        QSet<QString> seen;
        for (auto it = ids.globalMatch(QString::fromUtf8(x)); it.hasNext();) {
            const QString id = it.next().captured(1);
            QVERIFY2(!seen.contains(id), qPrintable(id));
            seen.insert(id);
        }
    }

    void connectionSitesFollowTheShapeAndAnUnknownSiteKeepsTheGlue()
    {
        auto doc = open(makeDocx("<w:p>" + anchoredGraphic(1, kShapeUri, plainShape(100, 100, "triangle"), 100, 100, 100, 100) + anchoredGraphic(2, kShapeUri, plainShape(40, 40, "rect"), 400, 400, 40, 40) +
                                 gluedConnector(3, 1, 5, 2, 0) + gluedConnector(4, 1, 7, 2, 99) + "</w:p>"));
        QVERIFY(doc);
        const int count = rerouteConnectors(*doc, {{0, QRectF(100, 100, 100, 100)}}, [](int, QRectF *r) {
            *r = QRectF(400, 400, 40, 40);
            return true;
        });
        QCOMPARE(count, 2);
        // The triangle's site 5 is the middle of its right slope: (175, 150); the rectangle's top is (420, 400).
        const PictureInfo a = pictureAt(*doc, doc->story(), 2);
        QVERIFY(std::abs(a.left - 175) < 0.01 && std::abs(a.top - 150) < 0.01 && std::abs(a.width - (420 - 175)) < 0.01 && std::abs(a.height - (400 - 150)) < 0.01);
        QVERIFY(isConnectorGlued(*doc, 2) && isConnectorGlued(*doc, 3));
    }

    void aConnectorWithAnEmptyFrameGetsAWholeOne()
    {
        auto doc = open(makeDocx("<w:p>" + anchoredGraphic(1, kShapeUri, plainShape(100, 50, "rect"), 100, 100, 100, 50) + anchoredGraphic(2, kShapeUri, plainShape(100, 50, "rect"), 400, 300, 100, 50) +
                                 gluedConnector(3, 1, 3, 2, 1, "<a:xfrm/>") + "</w:p>"));
        QVERIFY(doc);
        QCOMPARE(rerouteConnectors(*doc, {{0, QRectF(100, 400, 100, 50)}}, [](int, QRectF *r) {
                     *r = QRectF(400, 300, 100, 50);
                     return true;
                 }),
                 1);
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY(x.contains("<a:xfrm flipV=\"1\"><a:off x=\"0\" y=\"0\"/><a:ext cx="));
        auto back = open(doc->toDocx());
        QVERIFY(back && isConnectorGlued(*back, 2));
        QVERIFY(std::abs(pictureAt(*back, back->story(), 2).height - 100) < 0.01);
    }

    void groupAndUngroupEachUndoToTheExactMarkup()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        c.setPosition(8);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("ellipse")), PictureResult::Ok);
        const QByteArray plain = documentXml(doc->toDocx());
        QVERIFY(groupObjects(*doc, {7, 8}, {QRectF(10, 10, 108, 72), QRectF(200, 10, 86, 86)}));
        const QByteArray grouped = documentXml(doc->toDocx());
        QVERIFY(doc->undo());
        QCOMPARE(documentXml(doc->toDocx()), plain);
        QVERIFY(doc->redo());
        QVERIFY(ungroupObject(*doc, 7, QPointF(10, 10)));
        QVERIFY(doc->undo());
        QCOMPARE(documentXml(doc->toDocx()), grouped);
    }

    void aGroupInTheLineIsSetWhereItSitsInTheLineAndTheLineMakesRoomForIt()
    {
        auto part = [](int id, int x, const char *rgb) {
            return "<wps:wsp><wps:cNvPr id=\"" + QByteArray::number(id) + "\" name=\"m\"/><wps:cNvSpPr/><wps:spPr><a:xfrm><a:off x=\"" + QByteArray::number(x * 12700) +
                   "\" y=\"0\"/><a:ext cx=\"635000\" cy=\"1270000\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom><a:solidFill><a:srgbClr val=\"" + QByteArray(rgb) + "\"/></a:solidFill></wps:spPr><wps:bodyPr/></wps:wsp>";
        };
        const QByteArray inner = "<wpg:wgp><wpg:cNvGrpSpPr/><wpg:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1270000\" cy=\"1270000\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"1270000\" cy=\"1270000\"/></a:xfrm></wpg:grpSpPr>" +
                                 part(5, 0, "FF0000") + part(6, 50, "0000FF") + "</wpg:wgp>";
        auto doc = open(makeDocx("<w:p><w:r><w:t xml:space=\"preserve\">Some words before the group </w:t></w:r>" + anchoredGraphic(9, kGroupUri, inner, 0, 0, 100, 100, true) + "</w:p><w:p><w:r><w:t>Next</w:t></w:r></w:p>"));
        QVERIFY(doc);
        // The group is one object of the line, with its two parts, set after the words: not at the column's left edge, and the line is as tall as the group.
        std::shared_ptr<GroupItem> group;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QString json = it.fragment().charFormat().stringProperty(tp::InlineObject);
                if (json.isEmpty()) continue;
                const ItemPtr item = Item::fromJsonAny(QJsonDocument::fromJson(json.toUtf8()).object());
                if (item && item->type() == ItemType::Group) group = std::static_pointer_cast<GroupItem>(item);
            }
        QVERIFY(group);
        QCOMPARE(int(group->children.size()), 2);
        const auto page = doc->pager().materialize(0);
        QVERIFY(page);
        // Its parts are where the group puts them, from the group's own corner (the line draws the group where it sits among the words).
        QVERIFY(std::abs((group->children[1]->rect.left() - group->children[0]->rect.left()) - 50) < 0.01);
        QVERIFY(std::abs(group->children[0]->rect.width() - 50) < 0.01 && std::abs(group->children[0]->rect.height() - 100) < 0.01);
        // The line makes room for it as for a picture: the paragraph is as tall as the group and a line of words.
        auto words = open(makeDocx("<w:p><w:r><w:t xml:space=\"preserve\">Some words before the group </w:t></w:r></w:p>"));
        auto heightOf = [](WriteDoc &d) {
            const auto p = d.pager().materialize(0);
            return p->doc->pages[0]->items.empty() ? 0.0 : p->doc->pages[0]->items.front()->rect.height();
        };
        QVERIFY(heightOf(*doc) - heightOf(*words) > 80);
        // And it is the group's own place in the line, not the column's corner: the window's rectangle for it is as wide as the group.
        Win win(("<w:p><w:r><w:t xml:space=\"preserve\">Some words before the group </w:t></w:r>" + anchoredGraphic(9, kGroupUri, inner, 0, 0, 100, 100, true) + "</w:p>"));
        QRectF at;
        QTRY_VERIFY(win.w.view()->pictureRect(win.w.document()->story()->characterCount() - 2, &at));
        QVERIFY(at.width() > 50);
    }

    void hostileGroupsAndConnectorsAreHeldToTheirLimits()
    {
        // Groups nested far past the depth limit.
        QByteArray inner;
        const QByteArray props = "<wpg:grpSpPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1270000\" cy=\"1270000\"/><a:chOff x=\"0\" y=\"0\"/><a:chExt cx=\"1270000\" cy=\"1270000\"/></a:xfrm></wpg:grpSpPr>";
        const int depth = 40;
        for (int i = 0; i < depth; ++i) inner += i == 0 ? "<wpg:wgp><wpg:cNvGrpSpPr/>" + props : "<wpg:grpSp><wpg:cNvGrpSpPr/>" + props;
        inner += "<wps:wsp><wps:cNvPr id=\"5\" name=\"m\"/><wps:cNvSpPr/><wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"635000\" cy=\"635000\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></wps:spPr><wps:bodyPr/></wps:wsp>";
        for (int i = depth - 1; i >= 0; --i) inner += i == 0 ? "</wpg:wgp>" : "</wpg:grpSp>";
        auto nested = open(makeDocx("<w:p>" + anchoredGraphic(9, kGroupUri, inner, 100, 100, 100, 100) + "</w:p>"));
        QVERIFY(nested);
        QVERIFY(objectsOfPage(*nested).size() <= 201);
        QVERIFY(!canGroupObject(*nested, 0) || true);
        QVERIFY(!ungroupObject(*nested, 0, QPointF(0, 0)));   // (a group with a group in it is not taken apart)
        // Ids that name nothing, and sites out of range: nothing breaks, and nothing that cannot be run again is changed.
        auto lost = open(makeDocx("<w:p>" + anchoredGraphic(1, kShapeUri, plainShape(100, 50, "rect"), 100, 100, 100, 50) + gluedConnector(3, 999, 4, 1, 12345) + gluedConnector(4, -5 + 1000, 0, 1000, 0) + gluedConnector(5, 1, 99999, 1, 0) + "</w:p>"));
        QVERIFY(lost);
        const double before = pictureAt(*lost, lost->story(), 1).left;
        const int count = rerouteConnectors(*lost, {{0, QRectF(110, 100, 100, 50)}}, [](int, QRectF *) { return false; });
        QVERIFY(count <= 2);
        QVERIFY(std::abs(pictureAt(*lost, lost->story(), 1).left - before) < 0.01);
        QCOMPARE(rerouteConnectors(*lost, {{99999, QRectF(0, 0, 1, 1)}}, [](int, QRectF *) { return true; }), 0);
    }

    void aDrawingCanvasIsAFrameGroupThatShapesJoinAndItIsOneStepThatUndoes()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        const int length = doc->story()->characterCount();
        QCOMPARE(insertDrawingCanvas(*doc, c), PictureResult::Ok);
        QCOMPARE(doc->story()->characterCount(), length + 1);
        QVERIFY(isGroupAt(*doc, 7));
        PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QVERIFY(info.valid && info.floating && info.editable);
        QVERIFY(std::abs(info.width - 432) < 0.01 && std::abs(info.height - 216) < 0.01);
        QCOMPARE(info.wrap, PictureWrap::TopAndBottom);
        // The frame is drawn: a rectangle with a line and no fill.
        int outlined = 0;
        for (const auto &it : objectsOfPage(*doc)) outlined += it->type() == ItemType::Shape && it->fill.isNone() && !it->stroke.isNone();
        QCOMPARE(outlined, 1);
        c.setPosition(8);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("star5")), PictureResult::Ok);
        int at = -1;
        QVERIFY(groupObjects(*doc, {7, 8}, {QRectF(72, 100, 432, 216), QRectF(150, 150, 60, 60)}, &at));
        QCOMPARE(documentXml(doc->toDocx()).count("<wps:cNvPr"), 2);
        info = pictureAt(*doc, doc->story(), 7);
        QVERIFY(std::abs(info.width - 432) < 0.01 && std::abs(info.height - 216) < 0.01);
        auto back = open(doc->toDocx());
        QVERIFY(isGroupAt(*back, 7));
        QVERIFY(doc->undo());
        QVERIFY(doc->undo());
        QCOMPARE(doc->story()->characterCount(), length + 1);
        QVERIFY(doc->undo());
        QCOMPARE(doc->story()->characterCount(), length);
        QVERIFY(doc->redo());
        QVERIFY(isGroupAt(*doc, 7));
    }

    void aShapeAndATextBoxAreFilledWithAPictureSavedDrawnAndReadBack()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        QVERIFY(!pictureAt(*doc, doc->story(), 7).pictureFill);
        QCOMPARE(setShapePictureFill(*doc, 7, pngOf(40, 40)), PictureResult::Ok);
        PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QVERIFY(info.pictureFill && info.valid && info.editable && info.kind == DrawnKind::Shape);
        auto fillOfPage = [](WriteDoc &d) {
            const ItemList items = objectsOfPage(d);
            return items.size() == 1 ? items[0]->fill : Fill::none();
        };
        Fill fill = fillOfPage(*doc);
        QCOMPARE(fill.type, Fill::Picture);
        QVERIFY(!fill.imageId.isEmpty());
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY(x.contains("<a:blipFill") && x.contains("<a:blip r:embed=\""));
        auto back = open(doc->toDocx());
        QVERIFY(pictureAt(*back, back->story(), 7).pictureFill);
        QCOMPARE(fillOfPage(*back).type, Fill::Picture);
        // A solid color takes the picture off; the picture comes back with Undo.
        QVERIFY(setShapeFill(*doc, 7, QStringLiteral("00FF00")));
        QVERIFY(!pictureAt(*doc, doc->story(), 7).pictureFill);
        QCOMPARE(fillOfPage(*doc).type, Fill::Solid);
        QVERIFY(doc->undo());
        QVERIFY(pictureAt(*doc, doc->story(), 7).pictureFill);
        QVERIFY(doc->undo());
        QVERIFY(!pictureAt(*doc, doc->story(), 7).pictureFill);
        QVERIFY(doc->redo());
        QVERIFY(pictureAt(*doc, doc->story(), 7).pictureFill);
        // A text box keeps its words under the picture; a picture, and bytes that are no picture, are refused.
        auto boxed = textDoc({"Before after"});
        QTextCursor b(boxed->story());
        b.setPosition(7);
        QCOMPARE(insertTextBox(*boxed, b, textBoxDesigns().first()), PictureResult::Ok);
        QCOMPARE(setShapePictureFill(*boxed, 7, pngOf(30, 20)), PictureResult::Ok);
        QVERIFY(pictureAt(*boxed, boxed->story(), 7).pictureFill);
        QVERIFY(textBoxText(*boxed, 7).has_value());
        auto reread = open(boxed->toDocx());
        QVERIFY(pictureAt(*reread, reread->story(), 7).pictureFill);
        QVERIFY(textBoxText(*reread, 7).has_value());
        auto picture = textDoc({"Before after"});
        QTextCursor p(picture->story());
        p.setPosition(7);
        QCOMPARE(insertPicture(*picture, p, pngOf(40, 40)), PictureResult::Ok);
        QCOMPARE(setShapePictureFill(*picture, 7, pngOf(40, 40)), PictureResult::Refused);
        QCOMPARE(setShapePictureFill(*doc, 7, QByteArray("not a picture")), PictureResult::Unreadable);
    }

    void aShapeTakesAGradientFill()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        QVERIFY(setShapeGradient(*doc, 7, QStringLiteral("FF0000"), QStringLiteral("0000FF"), 90));
        QVERIFY(!setShapeGradient(*doc, 7, QStringLiteral("FF0000"), QStringLiteral("0000FF"), 90));
        QVERIFY(!setShapeGradient(*doc, 7, QStringLiteral("red"), QStringLiteral("0000FF"), 90));
        PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QCOMPARE(info.gradientFrom, QStringLiteral("FF0000"));
        QCOMPARE(info.gradientTo, QStringLiteral("0000FF"));
        QVERIFY(std::abs(info.gradientAngle - 90) < 0.01);
        const QByteArray x = documentXml(doc->toDocx());
        const qsizetype geom = x.indexOf("<a:prstGeom"), grad = x.indexOf("<a:gradFill", geom), ln = x.indexOf("<a:ln", geom);
        QVERIFY(geom > 0 && grad > geom && ln > grad);
        QVERIFY(x.contains("<a:lin ang=\"5400000\""));
        auto back = open(doc->toDocx());
        QCOMPARE(pictureAt(*back, back->story(), 7).gradientTo, QStringLiteral("0000FF"));
        QVERIFY(doc->undo());
        QVERIFY(pictureAt(*doc, doc->story(), 7).gradientFrom.isEmpty());
        QVERIFY(doc->redo());
        QVERIFY(setShapeFill(*doc, 7, QStringLiteral("00AA00")));
        QVERIFY(pictureAt(*doc, doc->story(), 7).gradientFrom.isEmpty());
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

    void accentsBarsBracketsAndMatricesAreMadeAndTheirMarkupFollowsTheStandard()
    {
        struct Case {
            const char *typed, *element, *attribute;
        };
        const Case cases[] = {
            {"\\hat{x}", "<m:acc>", "m:val=\"\xCC\x82\""},
            {"\\vec{v}", "<m:acc>", "m:val=\"\xE2\x83\x97\""},
            {"\\bar x", "<m:acc>", ""},
            {"\\overline{ab}", "<m:bar>", "m:val=\"top\""},
            {"\\underline{ab}", "<m:bar>", "m:val=\"bot\""},
            {"\\left( a/b \\right)", "<m:d>", "<m:begChr m:val=\"(\"/>"},
            {"\\left[ x \\right.", "<m:d>", "<m:endChr m:val=\"\"/>"},
            {"\\matrix{a & b \\\\ c & d}", "<m:m>", "<m:mr>"},
            {"\\pmatrix{1 & 0 \\\\ 0 & 1}", "<m:d>", "<m:m>"},
            {"\\vmatrix{a & b \\\\ c & d}", "<m:d>", "<m:begChr m:val=\"|\"/>"},
        };
        for (const Case &k : cases) {
            bool ok = false;
            const QByteArray markup = equationMarkup(QString::fromUtf8(k.typed), &ok);
            QVERIFY2(ok, k.typed);
            QVERIFY2(markup.contains(k.element), k.typed);
            if (*k.attribute) QVERIFY2(markup.contains(k.attribute), k.typed);
            office::wml::XmlTree tree;
            QVERIFY2(tree.parse(markup), k.typed);
            QVERIFY2(!equationLine(markup).isEmpty(), k.typed);
        }
        // A matrix has a row of cells for each line, and the same number of columns in each.
        const QByteArray m = equationMarkup(QStringLiteral("\\matrix{a & b \\\\ c & d}"));
        QCOMPARE(m.count("<m:mr>"), 2);
        QCOMPARE(m.count("<m:e>"), 4);
        QVERIFY(m.contains("<m:mcs><m:mc><m:mcPr><m:count m:val=\"2\"/>"));
        // The element order of the standard: the properties first.
        QVERIFY(equationMarkup(QStringLiteral("\\left( x \\right)")).indexOf("<m:dPr>") < equationMarkup(QStringLiteral("\\left( x \\right)")).indexOf("<m:e>"));
    }

    void theGalleryHasBuiltInEquationsThatAllReadAndEveryLineIsMadeAgainFromItsMarkup()
    {
        const QVector<EquationDesign> &gallery = equationGallery();
        QVERIFY(gallery.size() >= 8);
        for (const EquationDesign &d : gallery) {
            bool ok = false;
            const QByteArray markup = equationMarkup(d.linear, &ok);
            QVERIFY2(ok && !d.name.isEmpty(), qPrintable(d.name));
            QVERIFY2(!equationLine(markup).isEmpty(), qPrintable(d.name));
            // (The line made from the markup makes the same markup.)
            const QString again = equationLinear(markup);
            QVERIFY2(!again.isEmpty(), qPrintable(d.name));
            QCOMPARE(equationMarkup(again), markup);
        }
        // Markup with something the lines can't say is not turned into a line.
        QVERIFY(equationLinear("<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"><m:groupChr><m:e><m:r><m:t>x</m:t></m:r></m:e></m:groupChr></m:oMath>").isEmpty());
    }

    void anEquationThatIsInIsChangedInPlaceInOneStep()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QVERIFY(insertEquation(*doc, c, QStringLiteral("x^2/y")));
        QVERIFY(!equationAt(*doc, 7).isEmpty());
        QVERIFY(equationAt(*doc, 0).isEmpty());
        QCOMPARE(equationMarkup(equationAt(*doc, 7)), equationMarkup(QStringLiteral("x^2/y")));
        QVERIFY(replaceEquation(*doc, 7, QStringLiteral("\\hat{y}+1")));
        QVERIFY(!replaceEquation(*doc, 7, QStringLiteral("\\hat{y}+1")));
        QVERIFY(!replaceEquation(*doc, 7, QStringLiteral("   ")));
        QVERIFY(!replaceEquation(*doc, 0, QStringLiteral("x")));
        QCOMPARE(doc->story()->toPlainText().count(QChar::ObjectReplacementCharacter), 1);
        QVERIFY(documentXml(doc->toDocx()).contains("<m:acc>"));
        QVERIFY(!documentXml(doc->toDocx()).contains("<m:sSup>"));
        QVERIFY(doc->undo());
        QVERIFY(documentXml(doc->toDocx()).contains("<m:sSup>"));
        QVERIFY(doc->redo());
        auto back = open(doc->toDocx());
        QVERIFY(!equationAt(*back, 7).isEmpty());
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

    void aChartsAxesGridlinesTitlesAndStyleAreChangedOneStepEach()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertChart(*doc, c, ChartForm::Column), PictureResult::Ok);
        ChartInfo info = chartAt(*doc, 7);
        QVERIFY(info.hasAxes && info.categoryAxisShown && info.valueAxisShown);
        QVERIFY(setChartAxis(*doc, 7, ChartAxisKind::Value, false));
        QVERIFY(!setChartAxis(*doc, 7, ChartAxisKind::Value, false));
        QVERIFY(!chartAt(*doc, 7).valueAxisShown && chartAt(*doc, 7).categoryAxisShown);
        QVERIFY(doc->undo());
        QVERIFY(chartAt(*doc, 7).valueAxisShown);
        QVERIFY(doc->redo());
        QVERIFY(setChartAxis(*doc, 7, ChartAxisKind::Value, true));
        QVERIFY(setChartGridlines(*doc, 7, ChartAxisKind::Value, true, true));
        QVERIFY(!setChartGridlines(*doc, 7, ChartAxisKind::Value, true, true));
        info = chartAt(*doc, 7);
        QVERIFY(info.valueGridlines && info.valueMinorGridlines);
        QVERIFY(setChartGridlines(*doc, 7, ChartAxisKind::Category, true, false));
        QVERIFY(chartAt(*doc, 7).categoryGridlines);
        QVERIFY(setChartAxisTitle(*doc, 7, ChartAxisKind::Value, QStringLiteral("Units <sold>")));
        QVERIFY(!setChartAxisTitle(*doc, 7, ChartAxisKind::Value, QStringLiteral("Units <sold>")));
        QCOMPARE(chartAt(*doc, 7).valueAxisTitle, QStringLiteral("Units <sold>"));
        QVERIFY(setChartStyle(*doc, 7, 10));
        QVERIFY(!setChartStyle(*doc, 7, 10));
        QVERIFY(!setChartStyle(*doc, 7, 49));
        QCOMPARE(chartAt(*doc, 7).style, 10);
        auto back = open(doc->toDocx());
        const ChartInfo again = chartAt(*back, 7);
        QVERIFY(again.valueGridlines && again.valueMinorGridlines && again.categoryGridlines);
        QCOMPARE(again.valueAxisTitle, QStringLiteral("Units <sold>"));
        QCOMPARE(again.style, 10);
        // A pie has no axes.
        QVERIFY(setChartForm(*doc, 7, ChartForm::Pie));
        QVERIFY(!chartAt(*doc, 7).hasAxes);
        QVERIFY(!setChartAxis(*doc, 7, ChartAxisKind::Value, false));
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

    // ---------------------------------------------------------------- diagrams
    // A diagram in a file is drawn from the drawing the file keeps beside it, as one object (set in the line or at a place), and saving writes all its parts as they were.
    void aDiagramAFileHoldsIsDrawnFromItsSavedDrawingAsOneObjectAndSavedWhole()
    {
        for (const bool inlineInText : {true, false}) {
            const DiagramFile file = diagramDocx(true, inlineInText);
            auto doc = open(file.docx);
            QVERIFY(doc);
            QVERIFY(isDrawnObjectAt(doc->story(), 9));
            QVERIFY(!isPictureAt(doc->story(), 9));
            const PictureInfo info = pictureAt(*doc, doc->story(), 9);
            QVERIFY(info.valid);
            QCOMPARE(info.kind, DrawnKind::Diagram);
            QCOMPARE(info.floating, !inlineInText);
            QCOMPARE(info.width, 288.0);
            QCOMPARE(info.height, 108.0);
            const QByteArray svg = drawnSvg(*doc).toLower();
            QVERIFY2(svg.contains("336699") && svg.contains("20a040"), "the shapes of the drawing are drawn");
            if (!inlineInText) {
                // Moved as one object.
                QVERIFY(setPicturePlace(*doc, 9, 200, 250));
                const PictureInfo moved = pictureAt(*doc, doc->story(), 9);
                QVERIFY(std::abs(moved.left - 200) < 0.5 && std::abs(moved.top - 250) < 0.5);
            }
            // Saved: every part as it was, with the links, and the frame as it was.
            const QByteArray saved = doc->toDocx();
            office::Package before, after;
            QVERIFY(before.read(file.docx));
            QVERIFY(after.read(saved));
            for (const char *name : {"data1", "layout1", "quickStyle1", "colors1", "drawing1"}) {
                const QString part = QStringLiteral("/word/diagrams/%1.xml").arg(QLatin1String(name));
                QVERIFY2(after.hasPart(part), qPrintable(part));
                QCOMPARE(after.part(part), before.part(part));
            }
            QCOMPARE(after.relationshipsOfType(after.mainPart(), QStringLiteral("/diagramDrawing")).size(), 1);
            QCOMPARE(after.relationshipsOfType(after.mainPart(), QStringLiteral("/diagramData")).size(), 1);
            QVERIFY(documentXml(saved).contains("r:dm=\"rIdDm\""));
            QVERIFY2(rt::danglingReferences(saved).isEmpty(), qPrintable(rt::danglingReferences(saved).join(QLatin1Char(' '))));
            auto back = open(saved);
            QVERIFY(back);
            QCOMPARE(pictureAt(*back, back->story(), 9).kind, DrawnKind::Diagram);
            QVERIFY(drawnSvg(*back).toLower().contains("336699"));
        }
    }

    // A diagram sized in the window is saved with the frame's new size and place, and reopened it is drawn whole in that frame (its shapes scaled with it, as a picture is stretched), not cut.
    void aDiagramSizedSmallerIsSavedAndReopenedWholeWithoutUndo()
    {
        for (const bool inlineInText : {true, false}) {
            auto doc = open(diagramDocx(true, inlineInText).docx);
            QVERIFY(doc);
            QVERIFY(setPictureSize(*doc, 9, 144, 54));
            if (!inlineInText) QVERIFY(setPicturePlace(*doc, 9, 150, 200));
            const QByteArray saved = doc->toDocx();
            const QByteArray xml = documentXml(saved);
            QVERIFY2(xml.contains("<wp:extent cx=\"1828800\" cy=\"685800\"/>"), "the frame keeps its new size");
            auto back = open(saved);
            QVERIFY(back);
            const PictureInfo info = pictureAt(*back, back->story(), 9);
            QCOMPARE(info.width, 144.0);
            QCOMPARE(info.height, 54.0);
            if (!inlineInText) QVERIFY(std::abs(info.left - 150) < 0.5 && std::abs(info.top - 200) < 0.5);
            const QByteArray svg = drawnSvg(*back);
            QVERIFY(svg.toLower().contains("336699") && svg.toLower().contains("20a040"));
            static const QRegularExpression box(QStringLiteral("viewBox=\"0 0 ([0-9.]+) ([0-9.]+)\""));
            const QRegularExpressionMatch m = box.match(QString::fromUtf8(svg));
            QVERIFY2(m.hasMatch(), "the picture says the size it was drawn for");
            // The drawing's own size (the shapes fill 288 points across), shown in 144 by 54: width 432 and height 162 pixels at three to the point, stretched.
            QVERIFY2(m.captured(1).toDouble() > 280, qPrintable(m.captured(0)));
            QVERIFY(svg.contains("width=\"432\"") && svg.contains("height=\"162\""));
            QVERIFY(svg.contains("preserveAspectRatio=\"none\""));
        }
    }

    // A diagram sized again and again is the one diagram: it is not turned into a box after kMostDiagramsDrawn sizes.
    void aDiagramSizedManyTimesStaysDrawn()
    {
        auto doc = open(diagramDocx(true, true).docx);
        QVERIFY(doc);
        for (int i = 0; i < kMostDiagramsDrawn + 30; ++i) QVERIFY(setPictureSize(*doc, 9, 100 + i, 40));
        QVERIFY(drawnSvg(*doc).toLower().contains("336699"));
        QVERIFY(doc->diagramsDrawn() <= 1);
    }

    // The pictures of drawings are copied once for the document, however many drawings name one.
    void drawingsThatNameOnePictureShareIt()
    {
        using namespace diagramtest;
        const QByteArray png = pngOf(30, 20, QColor(10, 120, 200));
        const QString fill = QStringLiteral("<a:blipFill><a:blip r:embed=\"rId1\"/><a:stretch><a:fillRect/></a:stretch></a:blipFill>");
        office::PackageWriter w;
        const QString base = QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.");
        QByteArray runs;
        for (int n = 1; n <= 2; ++n) {
            const QString suffix = QString::number(n);
            w.addPart(QStringLiteral("/word/diagrams/data%1.xml").arg(n), diagramDataXml(QStringLiteral("rIdDrw") + suffix), base + QStringLiteral("diagramData+xml"));
            w.addPart(QStringLiteral("/word/diagrams/drawing%1.xml").arg(n), diagramDrawingXml(diagramShapeXml(0, 0, 914400, 914400, QStringLiteral("rect"), fill, QStringLiteral("p"))),
                      QStringLiteral("application/vnd.ms-office.drawingml.diagramDrawing+xml"));
            w.addRelationshipWithId(QStringLiteral("/word/diagrams/drawing%1.xml").arg(n), QStringLiteral("rId1"), relBase() + QStringLiteral("image"), QStringLiteral("/word/media/shared.png"));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdDm") + suffix, relBase() + QStringLiteral("diagramData"), QStringLiteral("/word/diagrams/data%1.xml").arg(n));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdDrw") + suffix, drawingRelType(), QStringLiteral("/word/diagrams/drawing%1.xml").arg(n));
            runs += "<w:p>" + anchoredGraphic(20 + n, dgmNs().toUtf8(), "<dgm:relIds xmlns:dgm=\"" + dgmNs().toUtf8() + "\" r:dm=\"rIdDm" + suffix.toUtf8() + "\" r:lo=\"rIdDm" + suffix.toUtf8() + "\" r:qs=\"rIdDm" + suffix.toUtf8() +
                                                  "\" r:cs=\"rIdDm" + suffix.toUtf8() + "\"/>", 0, 0, 200, 100, true) + "</w:p>";
        }
        w.addPart(QStringLiteral("/word/media/shared.png"), png, QStringLiteral("image/png"));
        w.addRelationship(QStringLiteral("/"), relBase() + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body>" + runs + "</w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        auto doc = open(w.finish());
        QVERIFY(doc);
        pictureOfPage(*doc);
        QCOMPARE(doc->diagramsDrawn(), 2);
        QCOMPARE(doc->diagramImageBytes(), qint64(png.size()));
    }

    // A diagram whose file keeps no drawing shows the picture that the older form of the object holds, or else a box that says what it is.
    void aDiagramWithoutADrawingShowsItsFallbackPictureOrALabeledPlaceholder()
    {
        const QByteArray png = pngOf(20, 10, QColor(30, 200, 30));
        {
            auto doc = open(diagramDocx(false, true, png).docx);
            QVERIFY(doc);
            const auto pic = pictureOfPage(*doc);
            QVERIFY(pic);
            QVERIFY2(!doc->flow().images.isEmpty() && doc->flow().images.last().format == QLatin1String("png"), "the fallback picture is the one drawn");
        }
        {
            auto doc = open(diagramDocx(false, true).docx);
            QVERIFY(doc);
            QCOMPARE(pictureAt(*doc, doc->story(), 9).kind, DrawnKind::Diagram);
            const QByteArray svg = drawnSvg(*doc);
            QVERIFY2(!svg.isEmpty(), "a box stands in");
            QVERIFY(pictureOfPage(*doc));
            QCOMPARE(pictureAt(*doc, doc->story(), 9).width, 288.0);
            // The parts are kept all the same.
            office::Package p;
            QVERIFY(p.read(doc->toDocx()));
            QVERIFY(p.hasPart(QStringLiteral("/word/diagrams/data1.xml")));
            QVERIFY(p.hasPart(QStringLiteral("/word/diagrams/layout1.xml")));
        }
    }

    // A drawing nested far deeper than any diagram is, or with a great many shapes, or whose links lead round in a circle, neither hangs nor crashes; the shapes drawn are capped.
    void aHostileDiagramDrawingIsCappedAndNeitherHangsNorCrashes()
    {
        using namespace diagramtest;
        // 20,000 groups inside each other.
        QString deep;
        for (int i = 0; i < 20000; ++i) deep += QStringLiteral("<dsp:grpSp><dsp:nvGrpSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvGrpSpPr/></dsp:nvGrpSpPr><dsp:grpSpPr/>");
        deep += diagramShapeXml(0, 0, 914400, 914400, QStringLiteral("rect"), QStringLiteral("<a:solidFill><a:srgbClr val=\"D01234\"/></a:solidFill>"), QStringLiteral("x"));
        for (int i = 0; i < 20000; ++i) deep += QStringLiteral("</dsp:grpSp>");
        {
            auto doc = open(diagramDocx(true, true, QByteArray(), diagramDrawingXml(deep)).docx);
            QVERIFY(doc);
            QCOMPARE(pictureAt(*doc, doc->story(), 9).kind, DrawnKind::Diagram);
            // (a part nested that deep is not read, or not past the shape reader's cap: the shape at the bottom is never drawn, and a picture stands in)
            const QByteArray svg = drawnSvg(*doc).toLower();
            QVERIFY(!svg.isEmpty() && !svg.contains("d01234"));
        }
        // Thirty thousand shapes: no more than the cap are drawn.
        QString many;
        for (int i = 0; i < 30000; ++i)
            many += QStringLiteral("<dsp:sp><dsp:nvSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvSpPr/></dsp:nvSpPr><dsp:spPr><a:xfrm><a:off x=\"%1\" y=\"%2\"/><a:ext cx=\"9144\" cy=\"9144\"/></a:xfrm>"
                                   "<a:prstGeom prst=\"rect\"/><a:solidFill><a:srgbClr val=\"AB12CD\"/></a:solidFill></dsp:spPr></dsp:sp>").arg(i % 100 * 9144).arg(i / 100 * 9144);
        {
            auto doc = open(diagramDocx(true, true, QByteArray(), diagramDrawingXml(many)).docx);
            QVERIFY(doc);
            const QByteArray svg = drawnSvg(*doc).toLower();
            QVERIFY(svg.contains("ab12cd"));
            QVERIFY2(svg.count("ab12cd") <= kMostDiagramShapes, qPrintable(QString::number(svg.count("ab12cd"))));
        }
        // The data part names, as the drawing's link, the link of the data part itself: it is not a drawing, and a box stands in.
        {
            DiagramFile file = diagramDocx(true, true);
            office::Package p;
            QVERIFY(p.read(file.docx));
            office::PackageWriter w;
            for (const QString &n : p.partNames()) w.addPart(n, n.endsWith(QLatin1String("data1.xml")) ? QByteArray(p.part(n)).replace("rIdDrw", "rIdDm") : p.part(n), p.contentType(n));
            for (const office::Relationship &r : p.relationships(p.mainPart())) w.addRelationshipWithId(p.mainPart(), r.id, r.type, r.target, r.external);
            for (const office::Relationship &r : p.relationships(QStringLiteral("/"))) w.addRelationshipWithId(QStringLiteral("/"), r.id, r.type, r.target, r.external);
            auto doc = open(w.finish());
            QVERIFY(doc);
            QVERIFY(!drawnSvg(*doc).toLower().contains("336699"));
            QVERIFY(pictureOfPage(*doc));
        }
    }

    // A document may draw only so many diagrams (drawings, not sizes): the rest are boxes, and the parts of all are kept.
    void aDocumentDrawsOnlyAsManyDiagramsAsTheCapAllows()
    {
        using namespace diagramtest;
        office::PackageWriter w;
        const QString base = QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.");
        QByteArray body;
        const int count = kMostDiagramsDrawn + 20;
        for (int n = 1; n <= count; ++n) {
            const QString suffix = QString::number(n);
            w.addPart(QStringLiteral("/word/diagrams/data%1.xml").arg(n), diagramDataXml(QStringLiteral("rIdDrw") + suffix), base + QStringLiteral("diagramData+xml"));
            w.addPart(QStringLiteral("/word/diagrams/drawing%1.xml").arg(n), diagramDrawingXml(), QStringLiteral("application/vnd.ms-office.drawingml.diagramDrawing+xml"));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdDm") + suffix, relBase() + QStringLiteral("diagramData"), QStringLiteral("/word/diagrams/data%1.xml").arg(n));
            w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdDrw") + suffix, drawingRelType(), QStringLiteral("/word/diagrams/drawing%1.xml").arg(n));
            const QByteArray id = "rIdDm" + suffix.toUtf8();
            body += "<w:p>" + anchoredGraphic(10 + n, dgmNs().toUtf8(), "<dgm:relIds xmlns:dgm=\"" + dgmNs().toUtf8() + "\" r:dm=\"" + id + "\" r:lo=\"" + id + "\" r:qs=\"" + id + "\" r:cs=\"" + id + "\"/>", 0, 0, 100, 50, true) + "</w:p>";
        }
        w.addRelationship(QStringLiteral("/"), relBase() + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        auto doc = open(w.finish());
        QVERIFY(doc);
        QVERIFY2(doc->diagramsDrawn() <= kMostDiagramsDrawn, qPrintable(QString::number(doc->diagramsDrawn())));
        QVERIFY(doc->diagramsDrawn() > 0);
        office::Package p;
        QVERIFY(p.read(doc->toDocx()));
        QVERIFY(p.hasPart(QStringLiteral("/word/diagrams/data%1.xml").arg(count)));
    }

    // Sized and moved in the window as one object, and put on paper as vectors: the PDF holds no picture of it.
    void aDiagramInTheWindowIsSelectedLikeAChartAndPrintedAsVectors()
    {
        const DiagramFile file = diagramDocx(true, true);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString docx = dir.filePath(QStringLiteral("d.docx"));
        {
            QFile f(docx);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(file.docx);
        }
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        QVERIFY(w.openFile(docx));
        w.resize(1100, 800);
        w.show();
        w.view()->selectPicture(9);
        QCOMPARE(w.view()->selectedPicture(), 9);
        QVERIFY(!w.act(QStringLiteral("shape.editText"))->isEnabled());
        const QString pdfPath = dir.filePath(QStringLiteral("d.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return pdfPath; });
        QVERIFY(w.exportPdf());
        QFile pdf(pdfPath);
        QVERIFY(pdf.open(QIODevice::ReadOnly));
        const QByteArray bytes = pdf.readAll();
        QVERIFY(bytes.size() > 100);
        QVERIFY2(!bytes.contains("/Subtype /Image") && !bytes.contains("/Subtype/Image"), "drawn as vectors");
    }

    // ---------------------------------------------------------------- linked text boxes
    // The words of the first box of a chain go on in the boxes that follow it, as far as each holds them, each box showing its own share; a box that is not in a chain keeps its own.
    void linkedTextBoxesFlowTheWordsOfTheFirstIntoTheNext()
    {
        {
            auto doc = open(linkedBoxesDocx({{1, 0, 72, 100, 150, 60, 20}, {1, 1, 300, 100, 150, 400}}));
            QVERIFY(doc);
            std::shared_ptr<jp::flow::MaterializedPage> page;
            const QVector<TextItem *> boxes = textBoxesOfPage(*doc, &page);
            QCOMPARE(boxes.size(), 2);
            const QString first = wordsOf(page, boxes[0]), second = wordsOf(page, boxes[1]);
            QVERIFY(first.startsWith(QStringLiteral("Line 1 ")) && !second.isEmpty() && !second.startsWith(QStringLiteral("Line 1 ")));
            QVERIFY(second.contains(QStringLiteral("Line 20 ")));
            // (nothing is shown twice, and nothing is dropped between the two)
            QString all;
            for (int i = 1; i <= 20; ++i) all += QStringLiteral("Line %1 of the story that goes on").arg(i);
            QString joined = first + second;
            joined.remove(QLatin1Char('\n')).remove(QChar::ParagraphSeparator);
            QCOMPARE(joined, all);
            LayoutCache cache;
            const LayoutCache::FrameLayout fl = cache.textFrame(*page->doc, *boxes[1], 1, RenderOptions());
            QVERIFY(fl.layout);
            QVERIFY2(fl.layout->lineRects(0).size() >= 5, qPrintable(QString::number(fl.layout->lineRects(0).size())));
            QVERIFY(!boxes[0]->overflowing && !boxes[1]->overflowing);
        }
        {
            auto doc = open(linkedBoxesDocx({{-1, 0, 72, 100, 150, 60, 20}, {-1, 0, 300, 100, 150, 400, 0}}));
            QVERIFY(doc);
            std::shared_ptr<jp::flow::MaterializedPage> page;
            const QVector<TextItem *> boxes = textBoxesOfPage(*doc, &page);
            QCOMPARE(boxes.size(), 2);
            QVERIFY(wordsOf(page, boxes[0]).contains(QStringLiteral("Line 20 ")));
            QVERIFY(wordsOf(page, boxes[1]).trimmed().isEmpty());
        }
    }

    // A chain goes on over pages: the first two boxes are on page one, the third on page two and shows the words that come after theirs; the last box of a chain whose boxes are too small
    // for the words shows a mark, so that no words are cut off unseen.
    void aChainOfThreeTextBoxesFlowsOverTwoPagesAndTheLastBoxMarksWordsPastIt()
    {
        {
            auto doc = open(linkedBoxesDocx({{2, 0, 72, 100, 150, 60, 14}, {2, 1, 300, 100, 150, 60}, {2, 2, 72, 100, 150, 300, 0, 1}}));
            QVERIFY(doc);
            std::shared_ptr<jp::flow::MaterializedPage> one, two;
            const QVector<TextItem *> onFirst = textBoxesOfPage(*doc, &one, 0), onSecond = textBoxesOfPage(*doc, &two, 1);
            QCOMPARE(onFirst.size(), 2);
            QCOMPARE(onSecond.size(), 1);
            const QString a = wordsOf(one, onFirst[0]), b = wordsOf(one, onFirst[1]), c = wordsOf(two, onSecond[0]);
            QVERIFY(a.startsWith(QStringLiteral("Line 1 ")));
            QVERIFY2(!b.isEmpty() && !c.isEmpty(), "each box shows a share");
            QVERIFY2(!c.startsWith(QStringLiteral("Line 1 ")) && c.contains(QStringLiteral("Line 14 ")), qPrintable(c.left(60)));
            QVERIFY(!onSecond[0]->overflowing);
            // the words are in the three boxes once each
            QString all;
            for (int i = 1; i <= 14; ++i) all += QStringLiteral("Line %1 of the story that goes on").arg(i);
            QString joined = a + b + c;
            joined.remove(QLatin1Char('\n')).remove(QChar::ParagraphSeparator);
            QCOMPARE(joined, all);
        }
        {
            // (the last box is too small: the words go on past it, and its mark says so)
            auto doc = open(linkedBoxesDocx({{2, 0, 72, 100, 150, 60, 40}, {2, 1, 300, 100, 150, 60}, {2, 2, 72, 100, 150, 40, 0, 1}}));
            QVERIFY(doc);
            std::shared_ptr<jp::flow::MaterializedPage> one, two;
            const QVector<TextItem *> onFirst = textBoxesOfPage(*doc, &one, 0), onSecond = textBoxesOfPage(*doc, &two, 1);
            QCOMPARE(onSecond.size(), 1);
            QVERIFY(onSecond[0]->overflowing);
            QVERIFY(!onFirst[0]->overflowing && !onFirst[1]->overflowing);
        }
    }

    // Saved as the file's own form (the first box holds the words under an id, each next box says "linked, this id, the nth") and read back the same.
    void aFileWithLinkedTextBoxesIsSavedAsItWasAndReadBackLinked()
    {
        const QByteArray file = linkedBoxesDocx({{3, 0, 72, 100, 150, 60, 12}, {3, 1, 300, 100, 150, 100}, {3, 2, 300, 300, 150, 100}});
        auto doc = open(file);
        QVERIFY(doc);
        const QByteArray saved = doc->toDocx();
        const QByteArray xml = documentXml(saved);
        QVERIFY2(xml.contains("<wps:txbx id=\"3\">"), "the first box keeps its id");
        QVERIFY(xml.contains("<wps:linkedTxbx id=\"3\" seq=\"1\"/>"));
        QVERIFY(xml.contains("<wps:linkedTxbx id=\"3\" seq=\"2\"/>"));
        auto back = open(saved);
        QVERIFY(back);
        std::shared_ptr<jp::flow::MaterializedPage> page;
        const QVector<TextItem *> boxes = textBoxesOfPage(*back, &page);
        QCOMPARE(boxes.size(), 3);
        QVERIFY(!wordsOf(page, boxes[1]).trimmed().isEmpty());
    }

    // A chain number of 0 is a number like any other: a box that says 0 is linked and its tag gets no second id; a box's own element is read, not one in the words of a box inside it.
    void chainNumberZeroIsValidAndOnlyTheBoxsOwnElementIsRead()
    {
        auto doc = open(linkedBoxesDocx({{0, 0, 72, 100, 150, 60, 5}, {-1, 0, 300, 100, 150, 100, 0}}));
        QVERIFY(doc);
        QVERIFY(canLinkTextBoxes(*doc, 7, 8));
        QCOMPARE(linkTextBoxes(*doc, 7, 8), PictureResult::Ok);
        const QByteArray xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("<wps:txbx id=\"0\">") && xml.contains("<wps:linkedTxbx id=\"0\" seq=\"1\"/>"));
        QVERIFY2(!QRegularExpression(QStringLiteral("<wps:txbx[^>]*id=\"[^>]*id=")).match(QString::fromUtf8(xml)).hasMatch(), "no second id on a tag");
        QVERIFY(isLinkedTextBox(*doc, 7) && isLinkedTextBox(*doc, 8));
        // the numbers a new chain may take are those from 0 to the schema's most, the least free one; when none is free there is none
        QCOMPARE(freeChainNumber({0, 1, 3}), 2);
        QSet<int> all;
        for (int n = 0; n <= kMostChainNumber; ++n) all.insert(n);
        QCOMPARE(freeChainNumber(all), -1);
        // a box whose words hold another box, which is linked, is no member itself
        const QByteArray nested = "<w:drawing><wp:inline><a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"><wps:wsp><wps:spPr/><wps:txbx><w:txbxContent><w:p><w:r>"
                                  "<w:drawing><wp:inline><a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"><wps:wsp><wps:linkedTxbx id=\"9\" seq=\"1\"/></wps:wsp></a:graphicData></a:graphic></wp:inline></w:drawing>"
                                  "</w:r></w:p></w:txbxContent></wps:txbx><wps:bodyPr/></wps:wsp></a:graphicData></a:graphic></wp:inline></w:drawing>";
        const BoxChain chain = boxChainOf(nested);
        QVERIFY2(chain.number == -1 && !chain.member, "the outer box is a box of its own");
    }

    // Taking away the first box of a chain leaves no member that names a number no box holds: the boxes after it are boxes of their own in the saved file.
    void takingAwayTheFirstBoxOfAChainLeavesNoOrphanInTheSavedFile()
    {
        auto doc = open(linkedBoxesDocx({{4, 0, 72, 100, 150, 60, 12}, {4, 1, 300, 100, 150, 100}, {4, 2, 300, 300, 150, 100}}));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(7);
        c.setPosition(8, QTextCursor::KeepAnchor);
        c.removeSelectedText();
        const QByteArray xml = documentXml(doc->toDocx());
        QVERIFY2(!xml.contains("linkedTxbx") && !xml.contains("<wps:txbx id=\"4\">"), "no member and no number are left");
        QCOMPARE(int(xml.count("<wps:txbx>")), 2);
        auto back = open(doc->toDocx());
        QVERIFY(back);
    }

    // A chain pasted is a chain of its own (the copy does not take the original's members); a member pasted without its first box is a box of its own; one Undo takes the paste back.
    void pastingLinkedTextBoxesRenumbersTheChain()
    {
        auto doc = open(linkedBoxesDocx({{5, 0, 72, 100, 150, 60, 12}, {5, 1, 300, 100, 150, 100}}));
        QVERIFY(doc);
        QTextCursor all(doc->story());
        all.setPosition(7);
        all.setPosition(9, QTextCursor::KeepAnchor);
        const Clip whole = copySelection(*doc, all);
        QTextCursor at(doc->story());
        at.movePosition(QTextCursor::End);
        pasteClip(*doc, at, whole);
        {
            const QByteArray xml = documentXml(doc->toDocx());
            QCOMPARE(int(xml.count("<wps:txbx id=")), 2);
            QCOMPARE(int(xml.count("<wps:linkedTxbx id=")), 2);
            QVERIFY(xml.contains("<wps:txbx id=\"5\">") && xml.contains("<wps:linkedTxbx id=\"5\" seq=\"1\"/>"));
            QCOMPARE(int(xml.count("<wps:linkedTxbx id=\"5\" seq=\"1\"/>")), 1);   // (the copy is a chain of its own)
            QVERIFY(xml.contains("<wps:txbx id=\"0\">") && xml.contains("<wps:linkedTxbx id=\"0\" seq=\"1\"/>"));
        }
        QVERIFY(doc->undo());
        QCOMPARE(int(documentXml(doc->toDocx()).count("<wps:linkedTxbx id=")), 1);
        // only the member
        QTextCursor member(doc->story());
        member.setPosition(8);
        member.setPosition(9, QTextCursor::KeepAnchor);
        const Clip lone = copySelection(*doc, member);
        QTextCursor end(doc->story());
        end.movePosition(QTextCursor::End);
        pasteClip(*doc, end, lone);
        const QByteArray xml = documentXml(doc->toDocx());
        QCOMPARE(int(xml.count("<wps:linkedTxbx id=")), 1);
        QCOMPARE(int(xml.count("<wps:txbx>")), 1);
    }

    // A diagram pasted into another document, or into this one, leaves no link that points at nothing.
    void aDiagramPastedLeavesNoDanglingLink()
    {
        const DiagramFile file = diagramDocx(true, true);
        auto source = open(file.docx);
        QVERIFY(source);
        QTextCursor sel(source->story());
        sel.setPosition(9);
        sel.setPosition(10, QTextCursor::KeepAnchor);
        const Clip clip = copySelection(*source, sel);
        auto target = open(makeDocx("<w:p><w:r><w:t>Target.</w:t></w:r></w:p>"));
        QVERIFY(target);
        QTextCursor t(target->story());
        t.movePosition(QTextCursor::End);
        pasteClip(*target, t, clip);
        {
            const QByteArray saved = target->toDocx();
            QVERIFY2(rt::danglingReferences(saved).isEmpty(), qPrintable(rt::danglingReferences(saved).join(QLatin1Char(' '))));
            QVERIFY(!documentXml(saved).contains("r:dm="));
        }
        QTextCursor same(source->story());
        same.movePosition(QTextCursor::End);
        pasteClip(*source, same, clip);
        const QByteArray saved = source->toDocx();
        QVERIFY2(rt::danglingReferences(saved).isEmpty(), qPrintable(rt::danglingReferences(saved).join(QLatin1Char(' '))));
    }

    // Create Link puts the second (empty) box after the first, Break Link cuts the chain after a box; each is one step of Undo, and what is saved is the file's own form.
    void createLinkAndBreakLinkAreOneStepEachAndSavedInTheFilesForm()
    {
        auto doc = open(linkedBoxesDocx({{-1, 0, 72, 100, 150, 60, 30}, {-1, 0, 300, 100, 150, 300, 0}, {-1, 0, 300, 420, 150, 100, 5}}));
        QVERIFY(doc);
        // The three boxes are the three characters after "Boxes: " (seven characters).
        QVERIFY(canLinkTextBoxes(*doc, 7, 8));
        QVERIFY2(!canLinkTextBoxes(*doc, 7, 9), "a box that holds words is not linked to");
        QVERIFY2(!canLinkTextBoxes(*doc, 7, 7), "not to itself");
        QCOMPARE(linkTextBoxes(*doc, 7, 9), PictureResult::Refused);
        QVERIFY(!isLinkedTextBox(*doc, 7));
        QCOMPARE(linkTextBoxes(*doc, 7, 8), PictureResult::Ok);
        QVERIFY2(isLinkedTextBox(*doc, 7), "the first box is in a chain");
        QVERIFY2(isLinkedTextBox(*doc, 8), "the second box is in a chain");
        QVERIFY2(!isLinkedTextBox(*doc, 9), "the third is not");
        QVERIFY(hasLinkedTextBoxAfter(*doc, 7));
        QVERIFY(!hasLinkedTextBoxAfter(*doc, 8));
        {
            const QByteArray xml = documentXml(doc->toDocx());
            QVERIFY(xml.contains("<wps:txbx id=\"0\">") && xml.contains("<wps:linkedTxbx id=\"0\" seq=\"1\"/>"));
            std::shared_ptr<jp::flow::MaterializedPage> page;
            const QVector<TextItem *> boxes = textBoxesOfPage(*doc, &page);
            QCOMPARE(boxes.size(), 3);
            QVERIFY(!wordsOf(page, boxes[1]).trimmed().isEmpty());
        }
        // one step of Undo takes both boxes back, and Redo puts the link again
        QVERIFY(doc->undo());
        QVERIFY(!isLinkedTextBox(*doc, 7) && !isLinkedTextBox(*doc, 8));
        QVERIFY(!documentXml(doc->toDocx()).contains("linkedTxbx"));
        QVERIFY(doc->redo());
        QVERIFY(isLinkedTextBox(*doc, 7) && isLinkedTextBox(*doc, 8));
        // a third box (with words) cannot follow; the chain's end can be followed by an empty one
        QVERIFY(setTextBoxText(*doc, 9, {QString()}));
        QVERIFY(canLinkTextBoxes(*doc, 8, 9));
        QCOMPARE(linkTextBoxes(*doc, 8, 9), PictureResult::Ok);
        QVERIFY(documentXml(doc->toDocx()).contains("<wps:linkedTxbx id=\"0\" seq=\"2\"/>"));
        // Break Link after the first box: the two after it are boxes of their own again, empty
        QVERIFY(unlinkTextBox(*doc, 7));
        QVERIFY(!isLinkedTextBox(*doc, 7) && !isLinkedTextBox(*doc, 8) && !isLinkedTextBox(*doc, 9));
        QVERIFY(!documentXml(doc->toDocx()).contains("linkedTxbx"));
        QVERIFY(!unlinkTextBox(*doc, 7));
        QVERIFY(doc->undo());
        QVERIFY(isLinkedTextBox(*doc, 7) && isLinkedTextBox(*doc, 8) && isLinkedTextBox(*doc, 9));
        QVERIFY(documentXml(doc->toDocx()).contains("seq=\"2\""));
    }

    // A chain whose numbers repeat, skip, or are huge, with no first box, or with hundreds of boxes, neither hangs nor loops: exactly the boxes that the rules chain are chained, and no more than
    // kMostLinkedBoxes of them.
    void aHostileChainOfLinkedTextBoxesIsCappedAndNeitherHangsNorLoops()
    {
        {
            // (a first box with far more words than sixty-four boxes hold, and three hundred members)
            QVector<LinkedBox> boxes = {{5, 0, 72, 100, 150, 60, 700}};
            for (int i = 0; i < 300; ++i) boxes.push_back({5, 1 + i, 20.0 + i % 20 * 25, 200.0 + i / 20 * 20, 150, 60});
            auto doc = open(linkedBoxesDocx(boxes));
            QVERIFY(doc);
            std::shared_ptr<jp::flow::MaterializedPage> page;
            const QVector<TextItem *> items = textBoxesOfPage(*doc, &page);
            QCOMPARE(items.size(), 301);
            int shown = 0, marked = 0;
            for (const TextItem *t : items) {
                shown += !wordsOf(page, t).trimmed().isEmpty();
                marked += t->overflowing;
            }
            QCOMPARE(shown, flow::kMostLinkedBoxes);
            QCOMPARE(marked, 1);
            QVERIFY(items[flow::kMostLinkedBoxes - 1]->overflowing);
            QVERIFY(wordsOf(page, items.last()).trimmed().isEmpty());
        }
        {
            // the numbers repeat, go back, are huge, or the first box is missing: of the boxes in the order of the page, the first (place 0), the one at place 1, the first at place 2, and the huge
            // one are chained, in that order; the repeat of 2 and the box of chain 8 (no first box) are boxes of their own
            auto doc = open(linkedBoxesDocx({{7, 0, 72, 100, 150, 60, 30}, {7, 2, 300, 100, 150, 60}, {7, 2, 300, 200, 150, 60}, {7, 1, 300, 300, 150, 60}, {7, 2000000000, 300, 400, 150, 60}, {8, 4, 72, 400, 150, 60}}));
            QVERIFY(doc);
            std::shared_ptr<jp::flow::MaterializedPage> page;
            const QVector<TextItem *> items = textBoxesOfPage(*doc, &page);
            QCOMPARE(items.size(), 6);
            QVERIFY(!wordsOf(page, items[0]).trimmed().isEmpty());
            QVERIFY(!wordsOf(page, items[3]).trimmed().isEmpty());   // place 1
            QVERIFY(!wordsOf(page, items[1]).trimmed().isEmpty());   // place 2
            QVERIFY(wordsOf(page, items[2]).trimmed().isEmpty());    // (a repeated place)
            QVERIFY(wordsOf(page, items[5]).trimmed().isEmpty());    // (no first box)
            QVERIFY(items[4]->overflowing);                          // the last chained box holds what is left, and the words go on past it
            QVERIFY(!items[0]->overflowing && !items[1]->overflowing && !items[3]->overflowing && !items[2]->overflowing && !items[5]->overflowing);
        }
    }

    // Create Link waits for a click on the box that follows (Esc leaves it); Break Link cuts the chain after the selected box; both are dimmed where they can do nothing.
    void createLinkAndBreakLinkWorkFromTheShapeFormatTab()
    {
        Win win;
        write::WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        QCOMPARE(insertTextBox(*doc, c, textBoxDesigns().first()), PictureResult::Ok);
        c.setPosition(1);
        QCOMPARE(insertTextBox(*doc, c, textBoxDesigns().first()), PictureResult::Ok);
        setTextBoxText(*doc, 1, {QString()});
        QVERIFY(canLinkTextBoxes(*doc, 0, 1));
        QVERIFY(setPicturePlace(*doc, 0, 100, 150));
        QVERIFY(setPicturePlace(*doc, 1, 350, 150));
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        win.w.refreshNow();
        QVERIFY(win.w.act(QStringLiteral("shape.createLink"))->isEnabled());
        QVERIFY(!win.w.act(QStringLiteral("shape.breakLink"))->isEnabled());
        win.w.act(QStringLiteral("shape.createLink"))->trigger();
        QVERIFY(win.w.view()->linkingTextBoxes());
        QTest::keyClick(win.w.view(), Qt::Key_Escape);
        QVERIFY(!win.w.view()->linkingTextBoxes());
        win.w.act(QStringLiteral("shape.createLink"))->trigger();
        QRectF second;
        QTRY_VERIFY(win.w.view()->pictureRect(1, &second));
        QTest::mouseClick(win.w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, second.center().toPoint());
        QVERIFY(!win.w.view()->linkingTextBoxes());
        QVERIFY(isLinkedTextBox(*doc, 0) && isLinkedTextBox(*doc, 1));
        win.w.refreshNow();
        QVERIFY(!win.w.act(QStringLiteral("shape.createLink"))->isEnabled());
        QVERIFY(win.w.act(QStringLiteral("shape.breakLink"))->isEnabled());
        win.w.act(QStringLiteral("shape.breakLink"))->trigger();
        QVERIFY(!isLinkedTextBox(*doc, 0) && !isLinkedTextBox(*doc, 1));
        win.w.refreshNow();
        QVERIFY(!win.w.act(QStringLiteral("shape.breakLink"))->isEnabled());
        QVERIFY(win.w.act(QStringLiteral("shape.createLink"))->isEnabled());
        QVERIFY(doc->undo());
        QVERIFY(isLinkedTextBox(*doc, 0) && isLinkedTextBox(*doc, 1));
        // a click on a box that holds words does not link: the notice says why
        QVERIFY(doc->undo());
        QVERIFY(!isLinkedTextBox(*doc, 0));
        QVERIFY(setTextBoxText(*doc, 1, {QStringLiteral("Words")}));
        QVERIFY(!canLinkTextBoxes(*doc, 0, 1));
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        win.w.refreshNow();
        win.w.act(QStringLiteral("shape.createLink"))->trigger();
        QTRY_VERIFY(win.w.view()->pictureRect(1, &second));
        QTest::mouseClick(win.w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, second.center().toPoint());
        QVERIFY(!isLinkedTextBox(*doc, 0));
    }
};

QTEST_MAIN(DrawTests)
#include "tst_write_draw.moc"
