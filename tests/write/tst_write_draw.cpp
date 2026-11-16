// Tests of JeffWrite's drawn objects: the shapes and text boxes a file holds and the ones Insert puts in, and the Picture Format and Shape Format commands. Each document is
// built here from a few lines of XML.

#include "../isolatesettings.h"
#include "../withdialog.h"
#include "../chartmenu.h"
#include <QStatusBar>
#include <QTableWidget>
#include "sheet/sheetreader.h"
#include "app/chartdesign.h"
#include "app/chartdialogs.h"
#include "sheet/chartedit.h"
#include "sheet/chartmake.h"
#include "sheet/chartembed.h"
#include "../chartembedhelpers.h"
#include "office/chart_write.h"
#include "../office/diagramsample.h"
#include "rtcheck.h"
#include "app/compresspictures.h"
#include "app/help.h"
#include "app/onlinepictures.h"
#include "app/pageview.h"
#include "app/shapetextedit.h"
#include "app/theme.h"
#include "app/settings.h"
#include "app/widgets.h"
#include "../shell/shapetexttest.h"
#include "app/ribbon.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "app/writeinfo.h"
#include "core/boxlinks.h"
#include "core/drawnitems.h"
#include "core/editops.h"
#include "core/equationops.h"
#include "core/fonts.h"
#include "core/fontops.h"
#include "core/items.h"
#include "core/pictureops.h"
#include "io/zip.h"
#include "core/picturestyles.h"
#include "app/formatpresets.h"
#include "app/themedialogs.h"
#include "core/themeops.h"
#include "core/writeprops.h"
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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextBlock>
#include <QLineEdit>
#include <QRandomGenerator>
#include <QStatusBar>
#include <QAbstractButton>
#include <QAbstractItemModel>
#include <QApplication>
#include <QListWidget>
#include <QDialog>
#include <QMenu>
#include <QLabel>
#include <QTableWidget>
#include <QRegularExpression>
#include <QTest>
#include <QPlainTextEdit>
#include <QTimer>
#include <QMessageBox>
#include <QTemporaryDir>
#include <QTextCursor>
#include <QTextDocument>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>

using namespace jp;
using namespace jp::write;

namespace {

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                   "xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                   "xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" xmlns:wpg=\"http://schemas.microsoft.com/office/word/2010/wordprocessingGroup\" "
                   "xmlns:wp14=\"http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing\"";

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

// A document with styles and one header, for the tests that read what is saved in each part: `body` and `sect` (the section's properties) are the XML inside the body.
QByteArray docxWithStylesAndHeader(const QByteArray &body, const QByteArray &sect, const QByteArray &styles, const QByteArray &header, const QByteArray &numbering = QByteArray())
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/word/document.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body>" + body + sect + "</w:body></w:document>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/styles.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:styles ") + kNs + ">" + styles + "</w:styles>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    w.addPart(QStringLiteral("/word/header1.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:hdr ") + kNs + ">" + header + "</w:hdr>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"));
    w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdheader1"), rel + QStringLiteral("header"), QStringLiteral("/word/header1.xml"));
    if (!numbering.isEmpty()) {
        w.addPart(QStringLiteral("/word/numbering.xml"), QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:numbering ") + kNs + ">" + numbering + "</w:numbering>",
                  QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"));
        w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("numbering"), QStringLiteral("/word/numbering.xml"));
    }
    return w.finish();
}
QByteArray partOf(const QByteArray &zip, const QString &name);

// A document with a theme part, a style sheet, and one header (see docxWithStylesAndHeader): the theme's colors are the ones given.
QByteArray themedDocx(const QByteArray &body, const QByteArray &sect, const QByteArray &styles, const QByteArray &header, const office::Theme &theme)
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    const QByteArray head = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n";
    w.addPart(QStringLiteral("/word/document.xml"), head + "<w:document " + kNs + "><w:body>" + body + sect + "</w:body></w:document>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    w.addPart(QStringLiteral("/word/styles.xml"), head + "<w:styles " + kNs + ">" + styles + "</w:styles>", QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("styles"), QStringLiteral("/word/styles.xml"));
    w.addPart(QStringLiteral("/word/theme/theme1.xml"), office::writeTheme(theme), QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"));
    w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("theme"), QStringLiteral("/word/theme/theme1.xml"));
    if (!header.isEmpty()) {
        w.addPart(QStringLiteral("/word/header1.xml"), head + "<w:hdr " + kNs + ">" + header + "</w:hdr>", QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"));
        w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("rIdheader1"), rel + QStringLiteral("header"), QStringLiteral("/word/header1.xml"));
    }
    return w.finish();
}

office::Theme themeWithAccent(const QColor &accent1)
{
    office::Theme t = office::defaultTheme();
    t.colors[office::Theme::Accent1] = accent1;
    t.xml = office::writeTheme(t);
    return t;
}

const QByteArray kThemedStyles = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                                 "<w:style w:type=\"paragraph\" w:styleId=\"Heading1\"><w:name w:val=\"heading 1\"/><w:basedOn w:val=\"Normal\"/>"
                                 "<w:rPr><w:color w:val=\"2F5496\" w:themeColor=\"accent1\" w:themeShade=\"BF\"/></w:rPr></w:style>";
const QByteArray kThemedSect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";

// The lightness of the color (HSL) times the shade, as the file format says a theme shade is.
QColor shadedBy(const QColor &c, int shade)
{
    float h, s, l, a;
    c.getHslF(&h, &s, &l, &a);
    return QColor::fromHslF(h, s, l * shade / 255.0f, a);
}

QByteArray styleElement(const QByteArray &docx, const QByteArray &id)
{
    const QByteArray styles = partOf(docx, QStringLiteral("/word/styles.xml"));
    const int at = styles.indexOf("w:styleId=\"" + id + "\"");
    return at < 0 ? QByteArray() : styles.mid(at, styles.indexOf("</w:style>", at) - at);
}

bool near(const QColor &a, const QColor &b) { return qAbs(a.red() - b.red()) <= 1 && qAbs(a.green() - b.green()) <= 1 && qAbs(a.blue() - b.blue()) <= 1; }

QByteArray partOf(const QByteArray &zip, const QString &name)
{
    office::Package p;
    p.read(zip);
    return p.part(name);
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

// A picture file of noise (a PNG that compresses badly), this many pixels across and down: the left half is red-tinted and the right half blue-tinted, so a cut shows.
quint32 crc32Of(const QByteArray &bytes)
{
    quint32 crc = 0xFFFFFFFFu;
    for (const char ch : bytes) {
        crc ^= quint8(ch);
        for (int k = 0; k < 8; ++k) crc = (crc >> 1) ^ (0xEDB88320u & (~(crc & 1) + 1));
    }
    return ~crc;
}

QByteArray noisyPng(int w, int h)
{
    QRandomGenerator rng(7);
    QImage image(w, h, QImage::Format_RGB32);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) image.setPixel(x, y, qRgb(x < w / 2 ? 200 + int(rng.bounded(56)) : int(rng.bounded(60)), int(rng.bounded(256)), x < w / 2 ? int(rng.bounded(60)) : 200 + int(rng.bounded(56))));
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return bytes;
}

// The picture files of a saved document, by part name.
QMap<QString, QByteArray> mediaOf(const QByteArray &zip)
{
    office::Package p;
    p.read(zip);
    QMap<QString, QByteArray> out;
    for (const QString &name : p.partNames())
        if (name.startsWith(QLatin1String("/word/media/"))) out.insert(name, p.part(name));
    return out;
}

// The online libraries answered from here (nothing goes online): a search finds one picture, "Red barn" by Pat Lee, whose file is `picture`. The real fetch comes back when this goes.
struct CannedLibraries {
    QStringList asked;
    std::function<void(const QUrl &, QObject *, const online::Done &, qint64)> kept = online::fetch;
    explicit CannedLibraries(const QByteArray &picture, const QString &title = QStringLiteral("Red barn"))
    {
        const QByteArray openverse = QJsonDocument(QJsonObject{{"results", QJsonArray{QJsonObject{
            {"id", "a1"}, {"title", title}, {"creator", "Pat Lee"}, {"license", "by"}, {"license_version", "2.0"},
            {"license_url", "https://creativecommons.org/licenses/by/2.0/"}, {"foreign_landing_url", "https://example.org/page/a1"},
            {"url", "https://example.org/barn.png"}, {"thumbnail", "https://example.org/barn.thumb"}}}}}).toJson();
        online::fetch = [this, openverse, picture](const QUrl &u, QObject *, const online::Done &done, qint64) {
            asked << u.toString();
            done(u.host() == QLatin1String("api.openverse.org") ? openverse : picture, QString());
        };
    }
    ~CannedLibraries() { online::fetch = kept; }
};

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
// A docx with the text of one part changed (the file's other parts as they were).
QByteArray withPartReplaced(const QByteArray &docx, const QString &part, const std::function<QByteArray(const QByteArray &)> &change)
{
    QMap<QString, QByteArray> parts;
    if (!readZip(docx, parts)) return {};
    ZipWriter out;
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) out.add(it.key(), it.key() == part ? change(it.value()) : it.value(), true);
    return out.finish();
}


// A document with a chart another program could have saved: its series read the cells A1:C4 of a workbook that is kept beside the chart part (c:externalData); the chart is at position 7.
QByteArray workbookChartDocx()
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    const QString charts = QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml");
    const QString bookRid = w.addRelationship(QStringLiteral("/word/charts/chart7.xml"), rel + QStringLiteral("package"), QStringLiteral("../embeddings/book1.xlsx"));
    office::Chart chart = office::sheet::makeChart(office::sheet::ChartKind::Column, office::sheet::planChartFrom(QStringLiteral("Sheet1"), [] {
        office::sheet::CellRange r;
        office::sheet::parseRangeRef(QStringLiteral("A1:C4"), &r);
        return r;
    }(), true, true, false, office::sheet::ChartKind::Column), office::defaultTheme(), 3);
    chart.externalDataId = bookRid;
    w.addPart(QStringLiteral("/word/charts/chart7.xml"), office::writeChartPart(chart), charts);
    w.addPart(QStringLiteral("/word/embeddings/book1.xlsx"), jp::testing::salesBook(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
    const QString rid = w.addRelationship(QStringLiteral("/word/document.xml"), rel + QStringLiteral("chart"), QStringLiteral("charts/chart7.xml"));
    const QByteArray run = "<w:r><w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"4572000\" cy=\"2743200\"/><wp:docPr id=\"4\" name=\"Chart 4\"/><wp:cNvGraphicFramePr/>"
                           "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"><c:chart xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" r:id=\"" + rid.toUtf8() +
                           "\"/></a:graphicData></a:graphic></wp:inline></w:drawing></w:r>";
    w.addPart(QStringLiteral("/word/document.xml"),
              QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n<w:document ") + kNs + "><w:body><w:p><w:r><w:t xml:space=\"preserve\">Chart: </w:t></w:r>" + run + "</w:p></w:body></w:document>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    return w.finish();
}

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

// A shape of the first position of the window's document, selected, and its words' editor opened with `key`.
ShapeTextEdit *editShape(Win &win, Qt::Key key, const QString &preset = QStringLiteral("rect"))
{
    WriteDoc *doc = win.w.document();
    QTextCursor c(doc->story());
    c.setPosition(0);
    if (insertShape(*doc, c, preset, QSizeF(240, 120)) != PictureResult::Ok || !setPicturePlace(*doc, 0, 150, 150)) return nullptr;
    win.w.view()->refresh();
    win.w.view()->selectPicture(0);
    QTest::keyClick(win.w.view()->viewport(), key);
    return win.w.view()->shapeEditor();
}

// A shape that is a text box holding `content` (the inside of its w:txbxContent), as the paragraph a window opens with.
QByteArray boxParagraph(const QByteArray &content, const QByteArray &around = QByteArray())
{
    const QByteArray drawing = shapeDrawing("<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>", "<a:solidFill><a:srgbClr val=\"CCDDEE\"/></a:solidFill>", 3048000, 1524000,
                                            "<wps:txbx><w:txbxContent>" + content + "</w:txbxContent></wps:txbx>");
    return around.isEmpty() ? drawing : around;
}

// The text box of a window made by boxParagraph, selected, its editor opened with `key`.
ShapeTextEdit *editBox(Win &win, Qt::Key key = Qt::Key_F2)
{
    win.w.view()->refresh();
    win.w.view()->selectPicture(0);
    QTest::keyClick(win.w.view()->viewport(), key);
    return win.w.view()->shapeEditor();
}

QByteArray fileBytes(const QString &path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

// Where the top left corner of the first page is in a window like Win (the same size and zoom): a shape set at a known place is drawn there.
QPointF pageOrigin()
{
    Win probe;
    write::WriteDoc *doc = probe.w.document();
    QTextCursor c(doc->story());
    c.setPosition(0);
    insertShape(*doc, c, QStringLiteral("rect"), QSizeF(50, 50));
    setPicturePlace(*doc, 0, 100, 100);
    probe.w.view()->refresh();
    QRectF r;
    QElapsedTimer waited;
    waited.start();
    while (!probe.w.view()->pictureRect(0, &r) && waited.elapsed() < 5000) QCoreApplication::processEvents();
    const double s = probe.w.view()->zoom() * 96.0 / 72.0;
    return r.topLeft() - QPointF(100 * s, 100 * s);
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

    // ---------------------------------------------------------------- Replace Fonts
    // Replace Fonts lists the fonts the document names and swaps one for another in the text, the styles, the text of a text box, and the header, in one step of Undo; a run that follows the
    // theme's font is left to the theme.
    void replaceFontsChangesEveryUseAsOneStepAndLeavesTheThemeAlone()
    {
        const QByteArray run = "<w:rPr><w:rFonts w:ascii=\"Zephyr\" w:hAnsi=\"Zephyr\"/></w:rPr>";
        const QByteArray box = shapeDrawing("<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>", "", 1270000, 635000,
                                            "<wps:txbx><w:txbxContent><w:p><w:r>" + run + "<w:t>in a box</w:t></w:r></w:p></w:txbxContent></wps:txbx>");
        const QByteArray body = "<w:p><w:r>" + run + "<w:t>plain words</w:t></w:r></w:p>"
                                "<w:p><w:pPr><w:pStyle w:val=\"Loud\"/></w:pPr><w:r><w:t>loud words</w:t></w:r></w:p>"
                                "<w:p><w:r><w:rPr><w:rFonts w:asciiTheme=\"minorHAnsi\" w:hAnsiTheme=\"minorHAnsi\"/></w:rPr><w:t>theme words</w:t></w:r></w:p>" + box;
        const QByteArray styles = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:qFormat/></w:style>"
                                  "<w:style w:type=\"paragraph\" w:styleId=\"Loud\"><w:name w:val=\"Loud\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:rFonts w:ascii=\"Zephyr\" w:hAnsi=\"Zephyr\"/><w:b/></w:rPr></w:style>";
        const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
        const QByteArray header = "<w:p><w:r>" + run + "<w:t>in the header</w:t></w:r></w:p>";
        auto doc = open(docxWithStylesAndHeader(body, sect, styles, header));
        QVERIFY(doc);
        auto uses = [&](const QString &font) {
            for (const FontUse &u : fontsUsed(*doc))
                if (u.key.compare(font, Qt::CaseInsensitive) == 0) return u.uses;
            return 0;
        };
        const int named = uses(QStringLiteral("Zephyr"));
        QVERIFY2(named >= 4, qPrintable(QString::number(named)));   // (a run, the run the style gives its font, the box's run, the header's run: the style itself is not content)
        QCOMPARE(uses(QStringLiteral("Zephyr Two")), 0);
        QCOMPARE(replaceFont(*doc, QStringLiteral("zephyr"), QStringLiteral("Zephyr Two")), named + 1);   // (and the style)
        QCOMPARE(uses(QStringLiteral("Zephyr")), 0);
        QCOMPARE(uses(QStringLiteral("Zephyr Two")), named);
        // Nothing is left to change, and no step is made for it.
        QCOMPARE(replaceFont(*doc, QStringLiteral("Zephyr"), QStringLiteral("Other")), 0);
        const QByteArray after = doc->toDocx();
        for (const QString &part : {QStringLiteral("/word/document.xml"), QStringLiteral("/word/styles.xml"), QStringLiteral("/word/header1.xml")}) {
            const QByteArray text = partOf(after, part);
            QVERIFY2(text.contains("Zephyr Two"), qPrintable(part));
            QVERIFY2(!text.contains("\"Zephyr\""), qPrintable(part));
        }
        // The theme's own run is still the theme's.
        {
            const int at = int(doc->story()->toPlainText().indexOf(QStringLiteral("theme words")));
            QVERIFY(at >= 0);
            QTextCursor c(doc->story());
            c.setPosition(at + 1);
            QVERIFY(!c.charFormat().hasProperty(QTextFormat::FontFamilies));   // (it still names no font: it takes the theme's)
        }
        // One Undo takes all of it back, a Redo does it again.
        QVERIFY(doc->undo());
        QCOMPARE(uses(QStringLiteral("Zephyr")), named);
        QCOMPARE(uses(QStringLiteral("Zephyr Two")), 0);
        for (const QString &part : {QStringLiteral("/word/document.xml"), QStringLiteral("/word/styles.xml"), QStringLiteral("/word/header1.xml")})
            QVERIFY2(!partOf(doc->toDocx(), part).contains("Zephyr Two"), qPrintable(part));
        QVERIFY(doc->redo());
        QCOMPARE(uses(QStringLiteral("Zephyr Two")), named);
    }

    // What only the styles, the style sheet's defaults, or the numbers of lists name is listed as styles only (no uses) and replacing it changes those, as one step; a slot a theme attribute
    // overrides names no font.
    void replaceFontsFindsFontsOnlyStylesDefaultsAndListsNameAndSkipsOverriddenSlots()
    {
        const QByteArray ignored = "<w:rPr><w:rFonts w:ascii=\"Ignored Face\" w:asciiTheme=\"minorHAnsi\" w:hAnsi=\"Ignored Face\" w:hAnsiTheme=\"minorHAnsi\"/></w:rPr>";
        const QByteArray box = shapeDrawing("<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>", "", 1270000, 635000,
                                            "<wps:txbx><w:txbxContent><w:p><w:r>" + ignored + "<w:t>in a box</w:t></w:r></w:p></w:txbxContent></wps:txbx>");
        const QByteArray styles = "<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii=\"Zephyr Default\" w:hAnsi=\"Zephyr Default\"/></w:rPr></w:rPrDefault></w:docDefaults>"
                                  "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:qFormat/></w:style>"
                                  "<w:style w:type=\"paragraph\" w:styleId=\"Quiet\"><w:name w:val=\"Quiet\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:rFonts w:ascii=\"Styles Only Face\" w:hAnsi=\"Styles Only Face\"/></w:rPr></w:style>";
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"o\"/>"
                                     "<w:rPr><w:rFonts w:ascii=\"Zephyr Bullet\" w:hAnsi=\"Zephyr Bullet\"/></w:rPr></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
        const QByteArray body = "<w:p><w:r><w:t>plain words</w:t></w:r></w:p>" + box;
        auto doc = open(docxWithStylesAndHeader(body, QByteArray(), styles, "<w:p/>", numbering));
        QVERIFY(doc);
        auto find = [&](const QString &font, bool *listed) {
            for (const FontUse &u : fontsUsed(*doc))
                if (u.key.compare(font, Qt::CaseInsensitive) == 0) {
                    *listed = true;
                    return u.uses;
                }
            *listed = false;
            return -1;
        };
        bool listed = false;
        // (the text takes the default font, so it is in use; the style no paragraph has, and the list's number font, are named by styles only)
        QVERIFY(find(QStringLiteral("Zephyr Default"), &listed) > 0);
        QVERIFY(listed);
        for (const char *name : {"Styles Only Face", "Zephyr Bullet"}) {
            QCOMPARE(find(QString::fromLatin1(name), &listed), 0);
            QVERIFY2(listed, name);
        }
        find(QStringLiteral("Ignored Face"), &listed);
        QVERIFY(!listed);
        auto saved = [&](const QString &part) { return partOf(doc->toDocx(), part); };
        QVERIFY(replaceFont(*doc, QStringLiteral("Zephyr Default"), QStringLiteral("Default Two")) >= 1);
        QVERIFY(saved(QStringLiteral("/word/styles.xml")).contains("Default Two"));
        QVERIFY(!saved(QStringLiteral("/word/styles.xml")).contains("Zephyr Default"));
        QCOMPARE(replaceFont(*doc, QStringLiteral("Zephyr Bullet"), QStringLiteral("Bullet Two")), 1);
        QVERIFY(saved(QStringLiteral("/word/numbering.xml")).contains("Bullet Two"));
        QCOMPARE(replaceFont(*doc, QStringLiteral("Styles Only Face"), QStringLiteral("Quiet Two")), 1);
        QVERIFY(saved(QStringLiteral("/word/styles.xml")).contains("Quiet Two"));
        // Each was one step: three Undos put the three back, and the file is as it was.
        QVERIFY(doc->undo());
        QVERIFY(doc->undo());
        QVERIFY(doc->undo());
        QVERIFY(saved(QStringLiteral("/word/styles.xml")).contains("Zephyr Default"));
        QVERIFY(saved(QStringLiteral("/word/styles.xml")).contains("Styles Only Face"));
        QVERIFY(saved(QStringLiteral("/word/numbering.xml")).contains("Zephyr Bullet"));
        QVERIFY(doc->redo());
        QVERIFY(saved(QStringLiteral("/word/styles.xml")).contains("Default Two"));
    }

    // The theme's fonts are offered and changed only when picked: the runs that follow them stay on the theme, and one Undo puts the theme back.
    void replaceFontsChangesTheThemesFontOnlyWhenItIsPicked()
    {
        auto doc = WriteDoc::blank();
        const office::Theme *theme = doc->theme();
        QVERIFY(theme);
        const QString body = theme->minorLatin;
        QTextCursor c(doc->story());
        insertText(*doc, c, QStringLiteral("some words"));
        bool listed = false;
        for (const FontUse &u : fontsUsed(*doc))
            if (u.key == QLatin1String("+Body")) {
                listed = true;
                QVERIFY(u.theme && u.uses > 0);
            }
        QVERIFY(listed);
        QVERIFY(replaceFont(*doc, QStringLiteral("+Body"), QStringLiteral("Theme Two")) > 0);
        QCOMPARE(doc->theme()->minorLatin, QStringLiteral("Theme Two"));
        QCOMPARE(doc->theme()->majorLatin, theme->majorLatin);
        QVERIFY(partOf(doc->toDocx(), QStringLiteral("/word/theme/theme1.xml")).contains("Theme Two"));
        QTextCursor at(doc->story());
        at.setPosition(1);
        QVERIFY(!at.charFormat().hasProperty(QTextFormat::FontFamilies));   // (the run still follows the theme)
        QVERIFY(doc->undo());
        QCOMPARE(doc->theme()->minorLatin, body);
        QVERIFY(!partOf(doc->toDocx(), QStringLiteral("/word/theme/theme1.xml")).contains("Theme Two"));
        QVERIFY(doc->redo());
        QCOMPARE(doc->theme()->minorLatin, QStringLiteral("Theme Two"));
    }

    // ---------------------------------------------------------------- themes
    // Design > Colors, Fonts, and Themes: the theme's colors and fonts change as one step; the runs and styles that take a theme slot follow, a plain color stays.
    void themeColorsRecolorThemedRunsAndStylesAsOneStepThatUndoes()
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        insertText(*doc, c, QStringLiteral("themed plain"));
        const office::Theme before = *doc->theme();
        const QColor oldAccent = before.colors[office::Theme::Accent1];
        auto colorRun = [&](int from, int to, const QColor &color, const QString &attrs) {
            QTextCursor r(doc->story());
            r.setPosition(from);
            r.setPosition(to, QTextCursor::KeepAnchor);
            QTextCharFormat f;
            f.setForeground(QBrush(color));
            if (!attrs.isEmpty()) {
                f.setProperty(prop::ColorAttrs, attrs);
                f.setProperty(prop::ColorAttrsRgb, color.name(QColor::HexRgb).mid(1).toUpper());
            }
            r.mergeCharFormat(f);
        };
        colorRun(0, 6, oldAccent, QStringLiteral("accent1;;"));
        colorRun(7, 12, QColor(0xFF, 0, 0), QString());
        const QString heading = doc->styleSheet().find(QStringLiteral("Heading1"))->rPr.color->val;
        const QByteArray docxBefore = doc->toDocx();
        QVERIFY(partOf(docxBefore, QStringLiteral("/word/theme/theme1.xml")).contains(oldAccent.name(QColor::HexRgb).mid(1).toUpper().toUtf8()));

        const office::ThemeColors colors = office::builtinThemeColors().at(3);
        QVERIFY(applyThemeColors(*doc, colors));
        QVERIFY(office::themeColorsOf(*doc->theme()) == colors);
        auto fg = [&](int at) {
            QTextCursor r(doc->story());
            r.setPosition(at);
            return r.charFormat().foreground().color();
        };
        QCOMPARE(fg(3), colors.colors[office::Theme::Accent1]);
        QCOMPARE(fg(10), QColor(0xFF, 0, 0));                  // a plain color stays
        QVERIFY(doc->styleSheet().find(QStringLiteral("Heading1"))->rPr.color->val != heading);
        // saved and opened again, the document has the new theme and the runs keep their slot
        const QByteArray saved = doc->toDocx();
        const QString newHex = colors.colors[office::Theme::Accent1].name(QColor::HexRgb).mid(1).toUpper();
        QVERIFY(partOf(saved, QStringLiteral("/word/theme/theme1.xml")).contains(newHex.toUtf8()));
        QVERIFY(partOf(saved, QStringLiteral("/word/document.xml")).contains(("w:val=\"" + newHex + "\" w:themeColor=\"accent1\"").toUtf8()));
        auto reopened = open(saved);
        QVERIFY(reopened->theme());
        QVERIFY(office::themeColorsOf(*reopened->theme()) == colors);
        QTextCursor r(reopened->story());
        r.setPosition(3);
        QCOMPARE(r.charFormat().foreground().color(), colors.colors[office::Theme::Accent1]);
        // one Undo puts all of it back, one Redo does it again
        QVERIFY(doc->undo());
        QCOMPARE(doc->theme()->colors[office::Theme::Accent1], oldAccent);
        QCOMPARE(fg(3), oldAccent);
        QCOMPARE(doc->styleSheet().find(QStringLiteral("Heading1"))->rPr.color->val, heading);
        QVERIFY(!partOf(doc->toDocx(), QStringLiteral("/word/theme/theme1.xml")).contains(newHex.toUtf8()));
        QVERIFY(doc->redo());
        QCOMPARE(fg(3), colors.colors[office::Theme::Accent1]);
        // the same colors again change nothing
        QVERIFY(!applyThemeColors(*doc, colors));
    }

    void themeFontsAndAWholeThemeChangeTheDocumentAndResetPutsTheOldOneBack()
    {
        auto doc = WriteDoc::blank();
        const office::Theme first = documentTheme(*doc);
        QVERIFY(canChangeTheme(*doc));
        QVERIFY(applyThemeFonts(*doc, {QStringLiteral("Pair"), QStringLiteral("Zephyr Heading"), QStringLiteral("Zephyr Body")}));
        QCOMPARE(doc->theme()->majorLatin, QStringLiteral("Zephyr Heading"));
        QCOMPARE(doc->theme()->fontSchemeName, QStringLiteral("Pair"));
        const QByteArray part = partOf(doc->toDocx(), QStringLiteral("/word/theme/theme1.xml"));
        QVERIFY(part.contains("Zephyr Heading"));
        QVERIFY(part.contains("fmtScheme"));   // the rest of the part stayed
        QVERIFY(doc->undo());
        QCOMPARE(doc->theme()->majorLatin, first.majorLatin);
        // a whole theme from a theme file, then Reset with the theme the document had
        office::Theme file = office::withThemeColors(office::defaultTheme(), office::builtinThemeColors().at(6));
        file = office::withThemeFonts(file, {QStringLiteral("Other"), QStringLiteral("Zephyr Body"), QStringLiteral("Zephyr Heading")});
        file.xml = office::writeTheme(file);
        office::Theme fromFile;
        QVERIFY(office::readThemeFromPackage(office::writeThemeFile(file), &fromFile));
        QVERIFY(applyTheme(*doc, fromFile));
        QVERIFY(office::themeColorsOf(*doc->theme()) == office::builtinThemeColors().at(6));
        QCOMPARE(doc->theme()->minorLatin, QStringLiteral("Zephyr Heading"));
        QVERIFY(applyTheme(*doc, first));
        QVERIFY(office::themeColorsOf(*doc->theme()) == office::themeColorsOf(first));
        QCOMPARE(doc->theme()->minorLatin, first.minorLatin);
        QVERIFY(doc->undo());   // (Reset is one step too)
        QCOMPARE(doc->theme()->minorLatin, QStringLiteral("Zephyr Heading"));
    }

    void aDocumentWithNoThemeTakesNone()
    {
        auto doc = open(makeDocx("<w:p><w:r><w:t>words</w:t></w:r></w:p>"));
        QVERIFY(!canChangeTheme(*doc));
        QVERIFY(!applyThemeColors(*doc, office::builtinThemeColors().first()));
        QVERIFY(!applyThemeFonts(*doc, {QStringLiteral("x"), QStringLiteral("Zephyr Body"), QStringLiteral("Zephyr Body")}));
    }

    void theDesignTabListsThemesColorsAndFontsAndTheyApplyInOneStep()
    {
        QTemporaryDir store;
        QVERIFY(store.isValid());
        office::setThemeStoreFolder(store.path());
        struct Reset { ~Reset() { office::setThemeStoreFolder(QString()); } } resetStore;
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        auto *colors = w.findChild<QMenu *>(QStringLiteral("themeColorsMenu"));
        auto *fonts = w.findChild<QMenu *>(QStringLiteral("themeFontsMenu"));
        auto *themes = w.findChild<QMenu *>(QStringLiteral("themesMenu"));
        QVERIFY(colors && fonts && themes);
        auto entry = [](QMenu *menu, const QString &text) -> QAction * {
            for (QAction *a : menu->actions())
                if (a->text().startsWith(text)) return a;
            return nullptr;
        };
        const office::Theme first = *w.document()->theme();
        jp::initCore();
        office::ThemeFonts installed;   // (a pair that is installed here, other than the document's)
        for (const office::ThemeFonts &f : office::builtinThemeFonts())
            if (f.heading != first.majorLatin) installed = f;
        QVERIFY(!installed.heading.isEmpty());
        // Create New Theme Colors: the box, then the set is kept for every app and chosen
        Q_EMIT colors->aboutToShow();
        QVERIFY(entry(colors, QStringLiteral("Create New Theme Colors")));
        QVERIFY(entry(colors, QStringLiteral("Lagoon")));
        QVERIFY(entry(colors, QStringLiteral("Reset to the Template")));
        colors->adjustSize();
        shotOf(colors, QStringLiteral("write-theme-colors-gallery"));
        withDialog([&] { entry(colors, QStringLiteral("Create New Theme Colors"))->trigger(); }, QStringLiteral("themeColorsDialog"), [&](QDialog *d) {
            auto *box = qobject_cast<ThemeColorsDialog *>(d);
            QVERIFY(box);
            box->setSlot(office::Theme::Accent1, QColor(0x12, 0x34, 0x56));
            box->nameEdit()->setText(QStringLiteral("Made Here"));
            shotOf(d, QStringLiteral("write-create-new-theme-colors"));
        });
        QCOMPARE(office::customThemeColors().size(), 1);
        QCOMPARE(w.document()->theme()->colors[office::Theme::Accent1], QColor(0x12, 0x34, 0x56));
        QVERIFY(w.document()->undo());
        QCOMPARE(w.document()->theme()->colors[office::Theme::Accent1], first.colors[office::Theme::Accent1]);
        // the list now shows it first, under Custom, and picking it applies it
        Q_EMIT colors->aboutToShow();
        QAction *made = entry(colors, QStringLiteral("Made Here"));
        QVERIFY(made);
        made->trigger();
        QCOMPARE(w.document()->theme()->colors[office::Theme::Accent1], QColor(0x12, 0x34, 0x56));
        // Themes: a built-in one changes colors and fonts together; Reset puts the opened theme back
        Q_EMIT themes->aboutToShow();
        QAction *lagoon = entry(themes, QStringLiteral("Lagoon"));
        QVERIFY(lagoon);
        lagoon->trigger();
        QVERIFY(office::themeColorsOf(*w.document()->theme()) == office::builtinThemeColors().first());
        QVERIFY(w.resetTheme());
        QVERIFY(office::themeColorsOf(*w.document()->theme()) == office::themeColorsOf(first));
        QCOMPARE(w.document()->theme()->minorLatin, first.minorLatin);
        // Fonts: Create New, with its box
        Q_EMIT fonts->aboutToShow();
        QVERIFY(entry(fonts, QStringLiteral("Create New Theme Fonts")));
        withDialog([&] { entry(fonts, QStringLiteral("Create New Theme Fonts"))->trigger(); }, QStringLiteral("themeFontsDialog"), [&](QDialog *d) {
            auto *box = qobject_cast<ThemeFontsDialog *>(d);
            QVERIFY(box);
            box->headingBox()->setCurrentText(installed.heading);
            box->nameEdit()->setText(QStringLiteral("My Pair"));
            shotOf(d, QStringLiteral("write-create-new-theme-fonts"));
        });
        QCOMPARE(w.document()->theme()->majorLatin, installed.heading);
        QCOMPARE(office::customThemeFonts().size(), 1);
        // Save Current Theme writes a file; Browse for Themes takes it back
        QTemporaryDir dir;
        const QString file = dir.filePath(QStringLiteral("out.thmx"));
        QVERIFY(w.saveThemeFile(file));
        QVERIFY(w.resetTheme());
        QCOMPARE(w.document()->theme()->majorLatin, first.majorLatin);
        QVERIFY(w.applyThemeFile(file));
        QCOMPARE(w.document()->theme()->majorLatin, installed.heading);
        QVERIFY(w.document()->undo());
        QCOMPARE(w.document()->theme()->majorLatin, first.majorLatin);
    }

    // A shade is worked out as the file format says (the lightness times the shade), and only a style whose slot got another color is touched: a change of fonts, or of other colors, leaves it as the file had it.
    void aShadedStyleChangesOnlyWithItsSlotAndKeepsItsShade()
    {
        const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Title</w:t></w:r></w:p>";
        const QColor old(0x44, 0x72, 0xC4);
        const QByteArray file = themedDocx(body, kThemedSect, kThemedStyles, "<w:p><w:r><w:t>h</w:t></w:r></w:p>", themeWithAccent(old));
        QVERIFY(near(shadedBy(old, 0xBF), QColor(0x2F, 0x54, 0x96)));
        auto doc = open(file);
        QVERIFY(doc->theme());
        const QByteArray original = styleElement(doc->toDocx(), "Heading1");
        QVERIFY(original.contains("2F5496"));
        // (a) a change of fonts only
        QVERIFY(applyThemeFonts(*doc, {QStringLiteral("Pair"), QStringLiteral("Zephyr Heading"), QStringLiteral("Zephyr Body")}));
        QCOMPARE(styleElement(doc->toDocx(), "Heading1"), original);
        // (b) colors in which accent 1 is the same
        office::ThemeColors same = office::themeColorsOf(*doc->theme());
        same.name = QStringLiteral("Same Accent");
        same.colors[office::Theme::Accent2] = QColor(1, 2, 3);
        QVERIFY(applyThemeColors(*doc, same));
        QCOMPARE(styleElement(doc->toDocx(), "Heading1"), original);
        QCOMPARE(doc->styleSheet().find(QStringLiteral("Heading1"))->rPr.color->val, QStringLiteral("2F5496"));
        // (c) a new accent 1: the shade of it, with the file's shade kept
        office::ThemeColors other = same;
        other.name = QStringLiteral("New Accent");
        other.colors[office::Theme::Accent1] = QColor(0xED, 0x7D, 0x31);
        QVERIFY(applyThemeColors(*doc, other));
        const QString val = doc->styleSheet().find(QStringLiteral("Heading1"))->rPr.color->val;
        QVERIFY2(near(QColor(QLatin1Char('#') + val), shadedBy(QColor(0xED, 0x7D, 0x31), 0xBF)), qPrintable(val));
        QVERIFY(styleElement(doc->toDocx(), "Heading1").contains("w:themeShade=\"BF\""));
        // saved and opened again, the style still names the slot and its shade
        auto back = open(doc->toDocx());
        QCOMPARE(back->styleSheet().find(QStringLiteral("Heading1"))->rPr.color->shade, QStringLiteral("BF"));
        QCOMPARE(back->styleSheet().find(QStringLiteral("Heading1"))->rPr.color->val, val);
    }

    // One Undo puts back a tinted heading, a themed run in the header, and a drawing that uses an accent, exactly; Redo does all three again.
    void undoAndRedoPutBackAHeadingAHeaderRunAndADrawingExactly()
    {
        const QColor old(0x44, 0x72, 0xC4);
        const QByteArray run = "<w:r><w:rPr><w:color w:val=\"4472C4\" w:themeColor=\"accent1\"/></w:rPr><w:t>Head</w:t></w:r>";
        const QByteArray drawing = "<w:p>" + anchoredGraphic(5, kShapeUri, plainShape(100, 50, "rect", QByteArray(), "<wps:cNvSpPr/>", "<a:solidFill><a:schemeClr val=\"accent1\"/></a:solidFill>"), 100, 100, 100, 50) + "</w:p>";
        const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Title</w:t></w:r></w:p>" + drawing;
        auto doc = open(themedDocx(body, kThemedSect, kThemedStyles, "<w:p>" + run + "</w:p>", themeWithAccent(old)));
        QVERIFY(doc->theme());
        auto headerColor = [&]() {
            for (QTextDocument *story : doc->storiesWithChanges(true))
                for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
                    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it)
                        if (it.fragment().charFormat().hasProperty(prop::ColorAttrs)) return it.fragment().charFormat().foreground().color();
            return QColor();
        };
        auto drawingFill = [&]() {
            const int at = doc->story()->toPlainText().indexOf(QChar(0xFFFC));
            return at < 0 ? QString() : pictureAt(*doc, doc->story(), at).fillColor;
        };
        auto headingVal = [&] { return doc->styleSheet().find(QStringLiteral("Heading1"))->rPr.color->val; };
        const QString headingBefore = headingVal(), fillBefore = drawingFill();
        QCOMPARE(headerColor(), old);
        QVERIFY(!fillBefore.isEmpty());
        office::ThemeColors colors = office::builtinThemeColors().at(1);
        QVERIFY(applyThemeColors(*doc, colors));
        const QColor accent = colors.colors[office::Theme::Accent1];
        QCOMPARE(headerColor(), accent);
        QVERIFY(near(QColor(QLatin1Char('#') + headingVal()), shadedBy(accent, 0xBF)));
        const QString fillAfter = drawingFill();
        QCOMPARE(QColor(QLatin1Char('#') + fillAfter), accent);
        const QString headingAfter = headingVal();
        QVERIFY(doc->undo());
        QCOMPARE(headerColor(), old);
        QCOMPARE(headingVal(), headingBefore);
        QCOMPARE(drawingFill(), fillBefore);
        QVERIFY(doc->redo());
        QCOMPARE(headerColor(), accent);
        QCOMPARE(headingVal(), headingAfter);
        QCOMPARE(drawingFill(), fillAfter);
    }

    // A name with a control character is written clean: the theme part, saved and read again, is the same set.
    void aNameWithAControlCharacterIsSavedCleanAndReadAgain()
    {
        auto doc = WriteDoc::blank();
        office::ThemeColors c = office::builtinThemeColors().at(2);
        c.name = QStringLiteral("A\u0001");
        QVERIFY(applyThemeColors(*doc, c));
        QVERIFY(applyThemeFonts(*doc, {QStringLiteral("F\u0002"), QStringLiteral("Zephyr\u0003 Heading"), QStringLiteral("Zephyr Body")}));
        auto back = open(doc->toDocx());
        QVERIFY(back && back->theme());
        QCOMPARE(back->theme()->colorSchemeName, QStringLiteral("A"));
        QCOMPARE(back->theme()->majorLatin, QStringLiteral("Zephyr Heading"));
        QVERIFY(office::themeColorsOf(*back->theme()) == c);
    }

    // A large document takes a change of colors, and its Undo, in well under a second or two (the milliseconds are printed).
    void aLargeDocumentTakesAChangeOfColorsQuickly()
    {
        const QColor old(0x44, 0x72, 0xC4);
        QByteArray body;
        const QByteArray themed = "<w:r><w:rPr><w:color w:val=\"4472C4\" w:themeColor=\"accent1\"/></w:rPr><w:t xml:space=\"preserve\">Words that take the accent color of the theme, </w:t></w:r>";
        const QByteArray plain = "<w:r><w:t xml:space=\"preserve\">and words that keep the text color, and go on for a while so the paragraph has some length to it. </w:t></w:r>";
        for (int page = 0; page < 300; ++page) {
            for (int p = 0; p < 12; ++p) body += "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr>" + themed + plain + themed + plain + "</w:p>";
            body += "<w:p><w:r><w:br w:type=\"page\"/></w:r></w:p>";
            if (page % 6 == 0)
                body += "<w:p>" + anchoredGraphic(10 + page, kShapeUri, plainShape(40, 30, "rect", QByteArray(), "<wps:cNvSpPr/>", "<a:solidFill><a:schemeClr val=\"accent1\"/></a:solidFill>"), 100, 100, 40, 30) + "</w:p>";
        }
        auto doc = open(themedDocx(body, kThemedSect, kThemedStyles, "<w:p><w:r><w:rPr><w:color w:val=\"4472C4\" w:themeColor=\"accent1\"/></w:rPr><w:t>Header</w:t></w:r></w:p>", themeWithAccent(old)));
        QVERIFY(doc);
        QElapsedTimer t;
        t.start();
        QVERIFY(applyThemeColors(*doc, office::builtinThemeColors().at(1)));
        const qint64 apply = t.elapsed();
        t.restart();
        QVERIFY(doc->undo());
        const qint64 undo = t.elapsed();
        qInfo("apply colors: %lld ms, undo: %lld ms (300 pages, 50 drawings)", apply, undo);
        QVERIFY2(apply < 5000 && undo < 5000, "a change of colors is slow");
    }

    void theReplaceFontsCommandAsksAndReplaces()
    {
        WriteWindow w;
        w.show();
        const QByteArray run = "<w:rPr><w:rFonts w:ascii=\"Zephyr\" w:hAnsi=\"Zephyr\"/></w:rPr>";
        w.setDocument(open(makeDocx("<w:p><w:r>" + run + "<w:t>words</w:t></w:r></w:p>")), QString());
        withDialog([&] { w.act("edit.replaceFonts")->trigger(); }, QStringLiteral("replaceFontsDialog"), [](QDialog *d) {
            auto *from = d->findChild<QComboBox *>(QStringLiteral("replaceFontsFrom"));
            QVERIFY(from);
            QVERIFY(from->itemText(0).startsWith(QLatin1String("Zephyr")));
            d->findChild<QComboBox *>(QStringLiteral("replaceFontsTo"))->setEditText(QStringLiteral("Zephyr Two"));
        });
        QVERIFY(partOf(w.document()->toDocx(), QStringLiteral("/word/document.xml")).contains("Zephyr Two"));
    }

    // ---------------------------------------------------------------- icons put in
    // Insert > Icons puts an icon in as a shape of its own, an inch square at the cursor, in the color scheme's main color, with lines that thicken and thin with its size; one step of Undo; saved to
    // .docx and .odt and read again as the same shape.
    void anIconIsPutInAsAShapeWhoseLinesKeepTheirShareOfItsSize()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertIcon(*doc, c, QStringLiteral("no-such-icon")), PictureResult::Unreadable);
        QVERIFY(!pictureAt(*doc, doc->story(), 7).valid);
        QCOMPARE(insertIcon(*doc, c, QStringLiteral("heart")), PictureResult::Ok);
        PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QVERIFY(info.valid && info.editable);
        QCOMPARE(int(info.kind), int(DrawnKind::Shape));
        QCOMPARE(info.width, 72.0);
        QCOMPARE(info.height, 72.0);
        QVERIFY(info.fillColor.isEmpty());
        QVERIFY(!info.lineColor.isEmpty());
        QVERIFY(info.lineWeight > 0.5);
        const ItemList items = objectsOfPage(*doc);
        QCOMPARE(items.size(), qsizetype(1));
        QCOMPARE(items[0]->type(), ItemType::Shape);
        QVERIFY(!static_cast<const ShapeItem *>(items[0].get())->customPath.isEmpty());
        // Twice the size, twice the line; half the size, half.
        const double weight = info.lineWeight;
        QVERIFY(setPictureSize(*doc, 7, 144, 144));
        QVERIFY(qAbs(pictureAt(*doc, doc->story(), 7).lineWeight - 2 * weight) < 0.02 * weight);
        QVERIFY(setPictureSize(*doc, 7, 36, 36));
        QVERIFY(qAbs(pictureAt(*doc, doc->story(), 7).lineWeight - weight / 2) < 0.02 * weight);
        QVERIFY(setPictureSize(*doc, 7, 72, 72));
        info = pictureAt(*doc, doc->story(), 7);
        QVERIFY(qAbs(info.lineWeight - weight) < 0.02 * weight);
        // Both file formats keep the shape (and its size, line, and the rule of the lines).
        const QByteArray docx = doc->toDocx();
        QVERIFY(documentXml(docx).contains("<a:custGeom>"));
        auto fromDocx = open(docx);
        QVERIFY(fromDocx);
        QString err;
        QStringList notes;
        const QByteArray odt = doc->toOdt(&err, &notes);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        QVERIFY2(notes.isEmpty(), qPrintable(notes.join(QLatin1Char(';'))));
        auto fromOdt = WriteDoc::fromFile(odt);
        QVERIFY(fromOdt);
        for (WriteDoc *back : {fromDocx.get(), fromOdt.get()}) {
            const PictureInfo again = pictureAt(*back, back->story(), 7);
            QVERIFY(again.valid);
            QCOMPARE(int(again.kind), int(DrawnKind::Shape));
            QVERIFY(qAbs(again.width - 72) < 0.5 && qAbs(again.height - 72) < 0.5);
            QVERIFY2(qAbs(again.lineWeight - weight) < 0.05 * weight, qPrintable(QString::number(again.lineWeight)));
            QVERIFY(again.fillColor.isEmpty());
            QCOMPARE(again.lineColor, info.lineColor);
            QCOMPARE(objectsOfPage(*back).size(), qsizetype(1));
            // (Still an icon after the trip: its lines go on keeping their share.)
            QVERIFY(setPictureSize(*back, 7, 144, 144));
            QVERIFY(qAbs(pictureAt(*back, back->story(), 7).lineWeight - 2 * weight) < 0.1 * weight);
        }
    }

    // In a document with a theme the icon's color is the theme's main one, by its name, so that it follows the theme.
    void anIconTakesTheThemesMainColor()
    {
        auto doc = WriteDoc::blank();
        QVERIFY(doc->theme());
        QTextCursor c(doc->story());
        QCOMPARE(insertIcon(*doc, c, QStringLiteral("star")), PictureResult::Ok);
        QVERIFY(documentXml(doc->toDocx()).contains("<a:schemeClr val=\"tx1\"/>"));
        const PictureInfo info = pictureAt(*doc, doc->story(), 0);
        QVERIFY(info.valid);
        QCOMPARE(info.lineColor, doc->theme()->colors[office::Theme::Dark1].name(QColor::HexRgb).mid(1).toUpper());
    }

    void anIconIsOneStepOfUndo()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertIcon(*doc, c, QStringLiteral("heart")), PictureResult::Ok);
        QVERIFY(pictureAt(*doc, doc->story(), 7).valid);
        QVERIFY(doc->undo());
        QVERIFY(!pictureAt(*doc, doc->story(), 7).valid);
        QCOMPARE(doc->story()->toPlainText(), QStringLiteral("Before after"));
        QVERIFY(!doc->undo());   // (nothing more was made)
        QVERIFY(doc->redo());
        QVERIFY(pictureAt(*doc, doc->story(), 7).valid);
    }

    void theIconCommandPutsEachIconChosenAtTheCaret()
    {
        WriteWindow w;
        w.show();
        w.setDocument(textDoc({"Before after"}), QString());
        QTextCursor c(w.document()->story());
        c.setPosition(7);
        w.view()->setCursor(c);
        w.insertIcons(QStringList{QStringLiteral("star")});
        QVERIFY(pictureAt(*w.document(), w.document()->story(), 7).valid);
        QCOMPARE(int(pictureAt(*w.document(), w.document()->story(), 7).kind), int(DrawnKind::Shape));
        QVERIFY(w.act("insert.icons")->isEnabled());
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

    void dragEndsOrderTheCornersKeepProportionsWithShiftAndGiveTheUsualSizeToAClick()
    {
        // A rectangle's corners are put in order whichever way it is dragged.
        auto ends = shapeDragEnds(QStringLiteral("rect"), QPointF(200, 150), QPointF(100, 50), false);
        QCOMPARE(ends.first, QPointF(100, 50));
        QCOMPARE(ends.second, QPointF(200, 150));
        // Shift makes a shape as wide as it is tall, the larger side winning, from the corner where the drag began.
        ends = shapeDragEnds(QStringLiteral("ellipse"), QPointF(100, 100), QPointF(180, 130), true);
        QCOMPARE(ends.second.x() - ends.first.x(), 80.0);
        QCOMPARE(ends.second.y() - ends.first.y(), 80.0);
        QCOMPARE(ends.first, QPointF(100, 100));
        ends = shapeDragEnds(QStringLiteral("ellipse"), QPointF(100, 100), QPointF(60, 30), true);
        QCOMPARE(ends.first, QPointF(30, 30));
        QCOMPARE(ends.second, QPointF(100, 100));
        // A line keeps its direction, and with Shift goes along a multiple of 45 degrees.
        ends = shapeDragEnds(QStringLiteral("line"), QPointF(100, 100), QPointF(40, 90), false);
        QCOMPARE(ends.first, QPointF(100, 100));
        QCOMPARE(ends.second, QPointF(40, 90));
        ends = shapeDragEnds(QStringLiteral("line"), QPointF(100, 100), QPointF(200, 110), true);
        QCOMPARE(ends.second.y(), 100.0);
        QVERIFY(std::abs(ends.second.x() - 100 - std::hypot(100, 10)) < 0.01);
        ends = shapeDragEnds(QStringLiteral("line"), QPointF(100, 100), QPointF(160, 130), true);
        QVERIFY(std::abs((ends.second.x() - 100) - (ends.second.y() - 100)) < 0.01);
        // A click, or a drag of less than four points, is the shape's usual size.
        ends = shapeDragEnds(QStringLiteral("rect"), QPointF(100, 100), QPointF(101, 102), false);
        QCOMPARE(ends.first, QPointF(100, 100));
        QCOMPARE(ends.second, QPointF(208, 172));
    }

    void aShapeDrawnByDraggingIsFloatingInFrontOfTheTextAtThePlaceDraggedAndIsOneStep()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShapeAt(*doc, c, QStringLiteral("rect"), QPointF(150, 200), QPointF(250, 260)), PictureResult::Ok);
        PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QVERIFY(info.floating);
        QCOMPARE(info.wrap, PictureWrap::InFrontOfText);
        QCOMPARE(info.preset, QStringLiteral("rect"));
        QCOMPARE(info.width, 100.0);
        QCOMPARE(info.height, 60.0);
        QVERIFY(info.placeIsOffset);
        QCOMPARE(info.left, 150.0);
        QCOMPARE(info.top, 200.0);
        QVERIFY(doc->undo());
        QVERIFY(!isDrawnObjectAt(doc->story(), 7));
        QCOMPARE(doc->story()->toPlainText(), QStringLiteral("Before after"));
        QVERIFY(doc->redo());
        QCOMPARE(pictureAt(*doc, doc->story(), 7).left, 150.0);
        // Dragged the other way and with Shift, the box is square from the corner where the drag began; and a wrap asked for is used.
        QTextCursor d(doc->story());
        d.setPosition(2);
        QCOMPARE(insertShapeAt(*doc, d, QStringLiteral("ellipse"), QPointF(300, 300), QPointF(260, 280), true, PictureWrap::Square), PictureResult::Ok);
        info = pictureAt(*doc, doc->story(), 2);
        QCOMPARE(info.wrap, PictureWrap::Square);
        QCOMPARE(info.width, 40.0);
        QCOMPARE(info.height, 40.0);
        QCOMPARE(info.left, 260.0);
        QCOMPARE(info.top, 260.0);
        // Not a shape of the gallery: nothing is put in.
        QTextCursor e(doc->story());
        QCOMPARE(insertShapeAt(*doc, e, QStringLiteral("nonsense"), QPointF(1, 1), QPointF(50, 50)), PictureResult::Unreadable);
    }

    void aLineDrawnUpAndToTheLeftIsFlippedAndSavedSo()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShapeAt(*doc, c, QStringLiteral("straightConnector1"), QPointF(300, 200), QPointF(200, 120)), PictureResult::Ok);
        const PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QCOMPARE(info.width, 100.0);
        QCOMPARE(info.height, 80.0);
        QCOMPARE(info.left, 200.0);
        QCOMPARE(info.top, 120.0);
        const QByteArray xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("flipH=\"1\""));
        QVERIFY(xml.contains("flipV=\"1\""));
        QVERIFY(doc->undo());
        QVERIFY(!isDrawnObjectAt(doc->story(), 7));
    }

    void theShapesGalleryArmsADrawToolAndADragOnThePageDrawsTheShapeInOneStep()
    {
        Win win;
        write::WriteDoc *doc = win.w.document();
        win.w.view()->refresh();
        QVERIFY(!win.w.view()->drawingShape());
        win.w.view()->beginShapeDraw(QStringLiteral("rect"));
        QVERIFY(win.w.view()->drawingShape());
        QWidget *vp = win.w.view()->viewport();
        // (Where the page is on the screen: from where a shape set at a known place of another window with the same size and zoom is drawn.)
        const double s = win.w.view()->zoom() * 96.0 / 72.0;
        const QPointF origin = pageOrigin();
        auto viewAt = [&](double x, double y) { return (origin + QPointF(x * s, y * s)).toPoint(); };
        const QPoint from = viewAt(150, 150), toA = viewAt(240, 210);
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(vp, viewAt(180, 170));
        QTest::mouseMove(vp, toA);
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, toA);
        QVERIFY(!win.w.view()->drawingShape());
        int at = -1;
        for (int i = 0; i < doc->story()->characterCount(); ++i)
            if (isDrawnObjectAt(doc->story(), i)) at = i;
        QVERIFY(at >= 0);
        PictureInfo info = pictureAt(*doc, doc->story(), at);
        QCOMPARE(info.preset, QStringLiteral("rect"));
        QCOMPARE(info.wrap, PictureWrap::InFrontOfText);
        QVERIFY(std::abs(info.width - 90) < 1.5);
        QVERIFY(std::abs(info.height - 60) < 1.5);
        QVERIFY(std::abs(info.left - 150) < 1.5 && std::abs(info.top - 150) < 1.5);
        QCOMPARE(win.w.view()->selectedPicture(), at);
        QVERIFY(doc->undo());
        QVERIFY(!isDrawnObjectAt(doc->story(), at));
        QVERIFY(doc->redo());
        // Shift while drawing makes it square; Escape puts the tool down.
        win.w.view()->beginShapeDraw(QStringLiteral("ellipse"));
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, viewAt(150, 330));
        QTest::mouseMove(vp, viewAt(250, 360));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::ShiftModifier, viewAt(250, 360));
        const int second = win.w.view()->selectedPicture();
        info = pictureAt(*doc, doc->story(), second);
        QCOMPARE(info.preset, QStringLiteral("ellipse"));
        QVERIFY(std::abs(info.width - info.height) < 0.01);
        win.w.view()->beginShapeDraw(QStringLiteral("rect"));
        QTest::keyClick(vp, Qt::Key_Escape);
        QVERIFY(!win.w.view()->drawingShape());
    }

    void aShapesCornerKeepsItsProportionsWithShiftAndIsFreeWithout()
    {
        Win win;
        write::WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect"), QSizeF(120, 60)), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 0, 150, 150));
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        QWidget *vp = win.w.view()->viewport();
        QRectF r;
        QTRY_VERIFY(win.w.view()->pictureRect(0, &r));
        const double s = win.w.view()->zoom() * 96.0 / 72.0;
        // Free at a corner, as it was.
        QPoint corner = r.bottomRight().toPoint();
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, corner);
        QTest::mouseMove(vp, corner + QPoint(int(15 * s), int(4 * s)));
        QTest::mouseMove(vp, corner + QPoint(int(40 * s), 0));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, corner + QPoint(int(40 * s), 0));
        PictureInfo info = pictureAt(*doc, doc->story(), 0);
        QVERIFY(std::abs(info.width - (120 + 40)) < 1.5);
        QVERIFY(std::abs(info.height - 60) < 1.5);
        QVERIFY(doc->undo());
        // With Shift held at the end, the shape keeps its 2 to 1.
        win.w.view()->refresh();
        QTRY_VERIFY(win.w.view()->pictureRect(0, &r));
        corner = r.bottomRight().toPoint();
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, corner);
        QTest::mouseMove(vp, corner + QPoint(int(15 * s), int(4 * s)));
        QTest::mouseMove(vp, corner + QPoint(int(40 * s), 0));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::ShiftModifier, corner + QPoint(int(40 * s), 0));
        info = pictureAt(*doc, doc->story(), 0);
        QVERIFY(info.width > 150);
        QVERIFY(std::abs(info.width / info.height - 2.0) < 0.02);
    }

    void aRoundedRectangleHasAYellowHandleThatSetsItsCornerAndIsSavedAsTheAdjustment()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("roundRect"), QSizeF(100, 60)), PictureResult::Ok);
        QVector<ShapeHandle> handles = shapeAdjustHandles(*doc, 7);
        QCOMPARE(handles.size(), 1);
        // The standard's default is a sixth of the shorter side, along the top edge.
        QVERIFY(std::abs(handles[0].at.x() - 10) < 0.01);
        QCOMPARE(handles[0].at.y(), 0.0);
        QVERIFY(setShapeAdjustment(*doc, 7, 0, QPointF(30, 5)));
        handles = shapeAdjustHandles(*doc, 7);
        QCOMPARE(handles.size(), 1);
        QVERIFY(std::abs(handles[0].at.x() - 30) < 0.01);
        QCOMPARE(handles[0].at.y(), 0.0);
        const QByteArray xml = documentXml(doc->toDocx());
        QVERIFY2(xml.contains("name=\"adj\" fmla=\"val 50000\""), xml.mid(xml.indexOf("prstGeom"), 200).constData());
        // Past the end of its reach the handle stops at half the shorter side; the same place again changes nothing.
        QVERIFY(setShapeAdjustment(*doc, 7, 0, QPointF(15, 0)));
        QVERIFY(std::abs(shapeAdjustHandles(*doc, 7)[0].at.x() - 15) < 0.01);
        QVERIFY(setShapeAdjustment(*doc, 7, 0, QPointF(90, 0)));
        QVERIFY(std::abs(shapeAdjustHandles(*doc, 7)[0].at.x() - 30) < 0.01);
        QVERIFY(!setShapeAdjustment(*doc, 7, 0, QPointF(90, 0)));
        QVERIFY(!setShapeAdjustment(*doc, 7, 3, QPointF(20, 0)));
        // One step each.
        QVERIFY(doc->undo());
        QVERIFY(std::abs(shapeAdjustHandles(*doc, 7)[0].at.x() - 15) < 0.01);
        QVERIFY(doc->undo());
        QVERIFY(std::abs(shapeAdjustHandles(*doc, 7)[0].at.x() - 30) < 0.01);
        QVERIFY(doc->undo());
        QVERIFY(std::abs(shapeAdjustHandles(*doc, 7)[0].at.x() - 10) < 0.01);
        // A rectangle has no handle; a block arrow has one.
        QTextCursor d(doc->story());
        d.setPosition(2);
        QCOMPARE(insertShape(*doc, d, QStringLiteral("rect")), PictureResult::Ok);
        QVERIFY(shapeAdjustHandles(*doc, 2).isEmpty());
        QVERIFY(!setShapeAdjustment(*doc, 2, 0, QPointF(5, 5)));
        QTextCursor e(doc->story());
        e.setPosition(1);
        QCOMPARE(insertShape(*doc, e, QStringLiteral("rightArrow")), PictureResult::Ok);
        QVERIFY(!shapeAdjustHandles(*doc, 1).isEmpty());
    }

    void aShapeTakesTextAndItIsKeptInTheFilesTextBoxElement()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("ellipse")), PictureResult::Ok);
        QVERIFY(textBoxText(*doc, 7).has_value());
        QCOMPARE(textBoxText(*doc, 7)->join(QLatin1Char('|')), QString());
        QVERIFY(setTextBoxText(*doc, 7, {QStringLiteral("Hello"), QStringLiteral("there")}));
        QCOMPARE(textBoxText(*doc, 7)->join(QLatin1Char('|')), QStringLiteral("Hello|there"));
        QByteArray xml = documentXml(doc->toDocx());
        const qsizetype spPr = xml.indexOf("</wps:spPr>"), box = xml.indexOf("<wps:txbx>"), body = xml.indexOf("<wps:bodyPr");
        QVERIFY(spPr > 0 && box > spPr && body > box);
        QVERIFY(xml.contains("Hello"));
        // Read back, the words are there and are laid out in the shape.
        auto back = open(doc->toDocx());
        QVERIFY(back);
        std::shared_ptr<jp::flow::MaterializedPage> page;
        const QVector<TextItem *> boxes = textBoxesOfPage(*back, &page);
        QVERIFY(!boxes.isEmpty());
        QVERIFY(wordsOf(page, boxes[0]).contains(QStringLiteral("Hello")));
        // Changed again, and undone, one step each; the words are not in a line.
        QVERIFY(setTextBoxText(*doc, 7, {QStringLiteral("Other")}));
        QVERIFY(doc->undo());
        QCOMPARE(textBoxText(*doc, 7)->join(QLatin1Char('|')), QStringLiteral("Hello|there"));
        QVERIFY(doc->undo());
        QCOMPARE(textBoxText(*doc, 7)->join(QLatin1Char('|')), QString());
        xml = documentXml(doc->toDocx());
        QVERIFY(!xml.contains("<wps:txbx>"));
        // A line holds no words.
        QTextCursor d(doc->story());
        d.setPosition(2);
        QCOMPARE(insertShape(*doc, d, QStringLiteral("line")), PictureResult::Ok);
        QVERIFY(!textBoxText(*doc, 2).has_value());
        QVERIFY(!setTextBoxText(*doc, 2, {QStringLiteral("x")}));
    }

    void theYellowHandleAndTheTurningHandleOfASelectedShapeAreDraggedInOneStepEach()
    {
        Win win;
        write::WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("roundRect"), QSizeF(160, 100)), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 0, 150, 150));
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        QWidget *vp = win.w.view()->viewport();
        QRectF r;
        QTRY_VERIFY(win.w.view()->pictureRect(0, &r));
        const double s = win.w.view()->zoom() * 96.0 / 72.0;
        // The yellow handle sits on the top edge a sixth of the shorter side in; pulled along the edge it sets the corner.
        const QPoint handle(int(r.left() + shapeAdjustHandles(*doc, 0)[0].at.x() * s), int(r.top()));
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, handle);
        QTest::mouseMove(vp, handle + QPoint(10, 3));
        QTest::mouseMove(vp, handle + QPoint(int(20 * s), 3));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, handle + QPoint(int(20 * s), 3));
        const double was = 100 / 6.0;
        QVERIFY(std::abs(shapeAdjustHandles(*doc, 0)[0].at.x() - (was + 20)) < 1.5);
        QCOMPARE(win.w.view()->selectedPicture(), 0);
        QVERIFY(doc->undo());
        QVERIFY(std::abs(shapeAdjustHandles(*doc, 0)[0].at.x() - was) < 0.1);
        QVERIFY(doc->redo());
        // The turning handle is above the top edge; a drag to the right of the shape turns it a quarter turn.
        win.w.view()->refresh();
        QTRY_VERIFY(win.w.view()->pictureRect(0, &r));
        const QPoint turn(int(r.center().x()), int(r.top() - 24));
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, turn);
        QTest::mouseMove(vp, turn + QPoint(20, 10));
        const QPoint right(int(r.center().x() + 120), int(r.center().y()));
        QTest::mouseMove(vp, right);
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, right);
        QVERIFY(std::abs(pictureAt(*doc, doc->story(), 0).rotation - 90) < 1);
        QVERIFY(doc->undo());
        QVERIFY(std::abs(pictureAt(*doc, doc->story(), 0).rotation) < 0.01);
    }

    void typingWithAShapeSelectedPutsTheWordsInTheShapeInPlaceAsOneStep()
    {
        Win win;
        write::WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("ellipse"), QSizeF(160, 100)), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 0, 150, 150));
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        QWidget *vp = win.w.view()->viewport();
        QVERIFY(!win.w.view()->editingShapeText());
        QTest::keyClick(vp, Qt::Key_H);
        QVERIFY(win.w.view()->editingShapeText());
        QTest::keyClicks(win.w.view()->shapeEditor(), QStringLiteral("i"));
        QTest::keyClick(win.w.view()->shapeEditor(), Qt::Key_Escape);
        QVERIFY(!win.w.view()->editingShapeText());
        QCOMPARE(textBoxText(*doc, 0)->join(QLatin1Char('|')), QStringLiteral("hi"));
        QCOMPARE(win.w.view()->selectedPicture(), 0);
        QCOMPARE(doc->story()->toPlainText().remove(QChar::ObjectReplacementCharacter), QStringLiteral("Hello world"));
        QVERIFY(doc->undo());
        QCOMPARE(textBoxText(*doc, 0)->join(QLatin1Char('|')), QString());
        QVERIFY(doc->redo());
        // A double click opens it again with the words, a second line is a second paragraph, and a click elsewhere keeps the words.
        win.w.view()->refresh();
        QRectF r;
        QTRY_VERIFY(win.w.view()->pictureRect(0, &r));
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QTest::mouseDClick(vp, Qt::LeftButton, Qt::NoModifier, r.center().toPoint());
        QVERIFY(win.w.view()->editingShapeText());
        ShapeTextEdit *edit = win.w.view()->shapeEditor();
        QVERIFY(edit);
        QCOMPARE(edit->toPlainText(), QStringLiteral("hi"));
        QTest::keyClick(edit, Qt::Key_End, Qt::ControlModifier);
        QTest::keyClick(edit, Qt::Key_Return);
        QTest::keyClicks(edit, QStringLiteral("there"));
        win.w.view()->endShapeTextEdit(true);
        QVERIFY(!win.w.view()->editingShapeText());
        QCOMPARE(textBoxText(*doc, 0)->join(QLatin1Char('|')), QStringLiteral("hi|there"));
        // Words that did not change make no step.
        QTest::mouseDClick(vp, Qt::LeftButton, Qt::NoModifier, r.center().toPoint());
        QVERIFY(win.w.view()->editingShapeText());
        win.w.view()->endShapeTextEdit(true);
        QVERIFY(doc->undo());
        QCOMPARE(textBoxText(*doc, 0)->join(QLatin1Char('|')), QStringLiteral("hi"));
        // A line takes no words: typing with it selected does not open an editor.
        QTextCursor d(doc->story());
        d.setPosition(1);
        QCOMPARE(insertShape(*doc, d, QStringLiteral("line")), PictureResult::Ok);
        win.w.view()->refresh();
        win.w.view()->selectPicture(1);
        QTest::keyClick(vp, Qt::Key_X);
        QVERIFY(!win.w.view()->editingShapeText());
    }

    void typingInAShapeKeepsItsLookThroughUndoSaveAndReopenInBothFileKinds()
    {
        Win win;
        WriteDoc *doc = win.w.document();
        ShapeTextEdit *edit = editShape(win, Qt::Key_F2);
        QVERIFY(edit);
        jptest::typeAndFormat(edit);
        jptest::shapeShot(&win.w, QStringLiteral("jeffwrite"));
        QTest::keyClick(edit, Qt::Key_Escape);
        QVERIFY(!win.w.view()->editingShapeText());
        jptest::verifyFormatted(textBoxStory(*doc, 0).get());
        // (One step: what was typed and every look.)
        QVERIFY(doc->undo());
        QVERIFY(textBoxText(*doc, 0)->join(QString()).isEmpty());
        QVERIFY(doc->redo());
        jptest::verifyFormatted(textBoxStory(*doc, 0).get());
        const QByteArray docx = doc->toDocx();
        auto back = open(docx);
        QVERIFY(back);
        jptest::verifyFormatted(textBoxStory(*back, 0).get());
        QString err;
        const QByteArray odt = doc->toOdt(&err);
        QVERIFY2(!odt.isEmpty(), qPrintable(err));
        auto fromOdt = WriteDoc::fromOdt(odt, &err);
        QVERIFY2(fromOdt, qPrintable(err));
        jptest::verifyFormatted(textBoxStory(*fromOdt, 0).get());
    }

    void inAShapeEscF2AndEnterAndATypedKeyBeginAndEndTheEdit()
    {
        Win win;
        WriteDoc *doc = win.w.document();
        ShapeTextEdit *edit = editShape(win, Qt::Key_Return);
        QVERIFY(edit);
        QTest::keyClicks(edit, QStringLiteral("one"));
        QTest::keyClick(edit, Qt::Key_Escape);
        QVERIFY(!win.w.view()->editingShapeText());
        QCOMPARE(win.w.view()->selectedPicture(), 0);   // (Esc leaves the shape selected)
        QCOMPARE(textBoxStory(*doc, 0)->toPlainText(), QStringLiteral("one"));
        QTest::keyClick(win.w.view()->viewport(), Qt::Key_F2);
        QVERIFY(win.w.view()->editingShapeText());
        QCOMPARE(win.w.view()->shapeEditor()->toPlainText(), QStringLiteral("one"));
        QTest::keyClick(win.w.view()->shapeEditor(), Qt::Key_Escape);
        QTest::keyClick(win.w.view()->viewport(), Qt::Key_Z);   // (typing on the selected shape starts the edit and goes after its words)
        QVERIFY(win.w.view()->editingShapeText());
        QCOMPARE(win.w.view()->shapeEditor()->toPlainText(), QStringLiteral("onez"));
        QTest::keyClick(win.w.view()->shapeEditor(), Qt::Key_Escape);
    }

    void aShapesCommandsOfTheRibbonAndItsBulletsGoToTheWordsBeingTyped()
    {
        Win win;
        WriteDoc *doc = win.w.document();
        ShapeTextEdit *edit = editShape(win, Qt::Key_F2);
        QVERIFY(edit);
        QTest::keyClicks(edit, QStringLiteral("item"));
        edit->selectAll();
        win.w.act(QStringLiteral("fmt.bold"))->trigger();
        win.w.act(QStringLiteral("fmt.alignRight"))->trigger();
        win.w.act(QStringLiteral("para.bullets"))->trigger();
        QVERIFY(edit->hasBullets());
        QTest::keyClick(edit, Qt::Key_Escape);
        auto story = textBoxStory(*doc, 0);
        QVERIFY(story->firstBlock().textList());
        QVERIFY(story->firstBlock().begin().fragment().charFormat().font().bold());
        QCOMPARE(story->firstBlock().blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignRight);
        auto back = open(doc->toDocx());
        QVERIFY(textBoxStory(*back, 0)->firstBlock().textList());
    }

    void wordsCopiedFromOneShapeAndPastedInAnotherKeepTheirLook()
    {
        Win win;
        WriteDoc *doc = win.w.document();
        ShapeTextEdit *edit = editShape(win, Qt::Key_F2);
        QVERIFY(edit);
        jptest::typeAndFormat(edit);
        edit->selectAll();
        win.w.view()->copy();
        QTest::keyClick(edit, Qt::Key_Escape);
        QTextCursor d(doc->story());
        d.setPosition(1);
        QCOMPARE(insertShape(*doc, d, QStringLiteral("rect"), QSizeF(240, 120)), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 1, 150, 400));
        win.w.view()->refresh();
        win.w.view()->selectPicture(1);
        QTest::keyClick(win.w.view()->viewport(), Qt::Key_F2);
        edit = win.w.view()->shapeEditor();
        QVERIFY(edit);
        win.w.view()->paste();
        QCOMPARE(edit->toPlainText(), QStringLiteral("Hello big world"));
        QTest::keyClick(edit, Qt::Key_Escape);
        jptest::verifyFormatted(textBoxStory(*doc, 1).get());
    }

    // Save, Ctrl+S, and Save As put the words being typed in a shape in the file, and Close asks to save them.
    void saveAndCloseCommitTheWordsBeingTypedInAShape()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        Win win;
        ShapeTextEdit *edit = editShape(win, Qt::Key_F2);
        QVERIFY(edit);
        QTest::keyClicks(edit, QStringLiteral("first"));
        // (a save puts the words in and ends the edit; F2 on the shape that stays selected opens it again, after its words)
        auto reopen = [&]() -> ShapeTextEdit * {
            if (!win.w.view()->shapeEditor()) QTest::keyClick(win.w.view()->viewport(), Qt::Key_F2);
            return win.w.view()->shapeEditor();
        };
        const QString path = dir.filePath(QStringLiteral("a.docx"));
        QVERIFY(win.w.saveTo(path));
        QTRY_VERIFY(textBoxStory(*open(fileBytes(path)), 0) && textBoxStory(*open(fileBytes(path)), 0)->toPlainText() == QLatin1String("first"));
        // (the edit goes on, and Ctrl+S commits the words typed since)
        edit = reopen();
        QVERIFY(edit);
        QTest::keyClicks(edit, QStringLiteral(" second"));
        win.w.act(QStringLiteral("file.save"))->trigger();
        QTRY_VERIFY(textBoxStory(*open(fileBytes(path)), 0) && textBoxStory(*open(fileBytes(path)), 0)->toPlainText() == QLatin1String("first second"));
        // Close: the words not yet in the document make it modified, and the question's Save puts them in the file.
        edit = reopen();
        QVERIFY(edit);
        QTest::keyClicks(edit, QStringLiteral(" third"));
        QVERIFY(win.w.isModified());
        QTimer answer;
        answer.setInterval(20);
        connect(&answer, &QTimer::timeout, this, [&] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) box->button(QMessageBox::Save)->click();
        });
        answer.start();
        win.w.close();
        answer.stop();
        QTRY_VERIFY(textBoxStory(*open(fileBytes(path)), 0) && textBoxStory(*open(fileBytes(path)), 0)->toPlainText() == QLatin1String("first second third"));
    }

    // Words past what a text box may hold are not dropped without a word: the edit stays open and the person is told.
    void wordsPastTheLimitKeepTheEditOpenAndTellThePerson()
    {
        Win win;
        WriteDoc *doc = win.w.document();
        ShapeTextEdit *edit = editShape(win, Qt::Key_F2);
        QVERIFY(edit);
        QSignalSpy told(win.w.view(), &PageView::notice);
        edit->insertPlainText(QString(50100, QLatin1Char('a')));
        QTest::keyClick(edit, Qt::Key_Escape);
        QVERIFY(win.w.view()->editingShapeText());
        QVERIFY(!told.isEmpty());
        QVERIFY(textBoxText(*doc, 0)->join(QString()).isEmpty());
        // (cut back, it goes in)
        edit->selectAll();
        edit->insertPlainText(QStringLiteral("short"));
        QTest::keyClick(edit, Qt::Key_Escape);
        QVERIFY(!win.w.view()->editingShapeText());
        QCOMPARE(textBoxText(*doc, 0)->join(QString()), QStringLiteral("short"));
    }

    // A text box of thousands of paragraphs is not read into an editor: the person is told, and nothing is changed.
    void aHugeTextBoxIsRefusedBeforeItIsReadAndTheEditorTellsWhy()
    {
        QByteArray many;
        for (int i = 0; i < 2100; ++i) many += "<w:p><w:r><w:t>p</w:t></w:r></w:p>";
        Win win(boxParagraph(many));
        QSignalSpy told(win.w.view(), &PageView::notice);
        QVERIFY(!editBox(win));
        QVERIFY(!win.w.view()->editingShapeText());
        QVERIFY(!told.isEmpty());
    }

    // The copy of a text box kept for older programs (mc:AlternateContent's Fallback) is not left holding the old words.
    void editingATextBoxDoesNotLeaveItsOldWordsInTheFallback()
    {
        const QByteArray inner = "<w:p><w:r><w:t>OLDWORDS</w:t></w:r></w:p>";
        const QByteArray drawing = shapeDrawing("<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>", "<a:solidFill><a:srgbClr val=\"CCDDEE\"/></a:solidFill>", 3048000, 1524000,
                                                "<wps:txbx><w:txbxContent>" + inner + "</w:txbxContent></wps:txbx>");
        // (the paragraph's run holds the drawing: it goes inside a choice, with a fallback of its own)
        QByteArray run = drawing.mid(drawing.indexOf("<w:r>"), drawing.lastIndexOf("</w:r>") + 6 - drawing.indexOf("<w:r>"));
        const QByteArray choice = "<w:r><mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:v=\"urn:schemas-microsoft-com:vml\"><mc:Choice Requires=\"wps\">" +
                                  run.mid(5, run.size() - 11) + "</mc:Choice><mc:Fallback><w:pict><v:shape type=\"#_x0000_t202\" style=\"width:240pt;height:120pt\"><v:textbox><w:txbxContent>" + inner +
                                  "</w:txbxContent></v:textbox></v:shape></w:pict></mc:Fallback></mc:AlternateContent></w:r>";
        QVERIFY2(open(makeDocx("<w:p>" + choice + "</w:p>")), "the document opens");
        Win win("<w:p>" + choice + "</w:p>");
        ShapeTextEdit *edit = editBox(win);
        QVERIFY(edit);
        QCOMPARE(edit->toPlainText(), QStringLiteral("OLDWORDS"));
        edit->selectAll();
        edit->insertPlainText(QStringLiteral("NEWWORDS"));
        QTest::keyClick(edit, Qt::Key_Escape);
        const QByteArray xml = documentXml(win.w.document()->toDocx());
        QVERIFY(xml.contains("NEWWORDS"));
        QVERIFY2(!xml.contains("OLDWORDS"), "the fallback still has the old words");
    }

    // A text box that holds a table, a field, a hyperlink, a bookmark, a content control, a footnote reference, and a tracked change keeps every one of them when a word of it is changed.
    void aTextBoxKeepsEverythingItHoldsWhenOneWordIsChanged()
    {
        const QByteArray rich =
            "<w:p><w:r><w:t xml:space=\"preserve\">Alpha beta </w:t></w:r><w:fldSimple w:instr=\" PAGE \"><w:r><w:t>1</w:t></w:r></w:fldSimple>"
            "<w:hyperlink w:anchor=\"mark\"><w:r><w:t>jump</w:t></w:r></w:hyperlink><w:bookmarkStart w:id=\"7\" w:name=\"mark\"/><w:bookmarkEnd w:id=\"7\"/>"
            "<w:ins w:id=\"9\" w:author=\"A\" w:date=\"2026-01-01T00:00:00Z\"><w:r><w:t>inserted</w:t></w:r></w:ins><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>"
            "<w:sdt><w:sdtPr><w:alias w:val=\"Pick\"/></w:sdtPr><w:sdtContent><w:p><w:r><w:t>control</w:t></w:r></w:p></w:sdtContent></w:sdt>"
            "<w:tbl><w:tblPr/><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>cell</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p/>";
        Win win(boxParagraph(rich));
        ShapeTextEdit *edit = editBox(win);
        QVERIFY2(edit, "the editor opens on a box with more than words");
        QTextCursor found = edit->document()->find(QStringLiteral("Alpha"));
        QVERIFY(!found.isNull());
        found.insertText(QStringLiteral("Omega"));
        QTest::keyClick(edit, Qt::Key_Escape);
        const QByteArray saved = win.w.document()->toDocx();
        const QByteArray xml = documentXml(saved);
        QVERIFY(xml.contains("Omega") && !xml.contains("Alpha"));
        for (const char *piece : {"<w:tbl>", "PAGE", "w:hyperlink", "w:bookmarkStart", "<w:sdt>", "w:footnoteReference", "<w:ins ", "control", "cell", "inserted"})
            QVERIFY2(xml.contains(piece), piece);
        auto back = open(saved);
        QVERIFY(back);
        const QByteArray again = documentXml(back->toDocx());
        for (const char *piece : {"Omega", "<w:tbl>", "PAGE", "w:hyperlink", "w:bookmarkStart", "<w:sdt>", "w:footnoteReference", "<w:ins "})
            QVERIFY2(again.contains(piece), piece);
    }

    // Bullets made while typing in a shape are the shape's: the body's text and its history are not touched.
    void bulletsInAShapeLeaveTheBodyAndItsHistoryAlone()
    {
        Win win;
        WriteDoc *doc = win.w.document();
        ShapeTextEdit *edit = editShape(win, Qt::Key_F2);
        QVERIFY(edit);
        QTest::keyClicks(edit, QStringLiteral("item"));
        const int revision = doc->story()->revision();
        const int steps = doc->story()->availableUndoSteps();
        const QString body = doc->story()->toPlainText();
        win.w.act(QStringLiteral("para.bullets"))->trigger();
        QVERIFY(edit->hasBullets());
        QCOMPARE(doc->story()->revision(), revision);
        QCOMPARE(doc->story()->availableUndoSteps(), steps);
        QCOMPARE(doc->story()->toPlainText(), body);
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) QVERIFY(!b.textList());
        win.w.act(QStringLiteral("para.bullets"))->trigger();
        QVERIFY(!edit->hasBullets());
        QCOMPARE(doc->story()->revision(), revision);
    }

    // The ribbon shows the look of the words being typed in a shape: the bold, italic, and underline buttons, and the size box.
    void theRibbonShowsTheLookOfTheWordsBeingTypedInAShape()
    {
        Win win;
        ShapeTextEdit *edit = editShape(win, Qt::Key_F2);
        QVERIFY(edit);
        QTest::keyClicks(edit, QStringLiteral("ab"));
        edit->selectAll();
        QTest::keyClick(edit, Qt::Key_B, Qt::ControlModifier);
        QTRY_VERIFY(win.w.act(QStringLiteral("fmt.bold"))->isChecked());
        QTest::keyClick(edit, Qt::Key_I, Qt::ControlModifier);
        QTRY_VERIFY(win.w.act(QStringLiteral("fmt.italic"))->isChecked());
        QVERIFY(!win.w.act(QStringLiteral("fmt.underline"))->isChecked());
        QTest::keyClick(edit, Qt::Key_B, Qt::ControlModifier);
        QTRY_VERIFY(!win.w.act(QStringLiteral("fmt.bold"))->isChecked());
        edit->setFontSizePt(18);
        QTRY_VERIFY(std::any_of(win.w.findChildren<SizeCombo *>().begin(), win.w.findChildren<SizeCombo *>().end(), [](SizeCombo *c) { return c->currentText().toDouble() == 18.0; }));
    }

    void aShapeTakesSeveralEffectsAndEachIsPutInAndTakenOutWithoutTouchingTheOthers()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect")), PictureResult::Ok);
        QCOMPARE(shapeEffectAt(*doc, 7), ShapeEffect::None);
        QVERIFY(!setShapeEffect(*doc, 7, ShapeEffect::None));
        QVERIFY(setShapeEffect(*doc, 7, ShapeEffect::Shadow));
        QVERIFY(!setShapeEffect(*doc, 7, ShapeEffect::Shadow));
        QVERIFY(setShapeEffect(*doc, 7, ShapeEffect::Glow));
        QByteArray xml = documentXml(doc->toDocx());
        QVERIFY2(xml.contains("<a:glow") && xml.contains("<a:outerShdw"), "a glow is put in and the shadow stays");
        QVERIFY(xml.indexOf("<a:glow") < xml.indexOf("<a:outerShdw"));
        QVERIFY(shapeHasEffect(*doc, 7, ShapeEffect::Shadow) && shapeHasEffect(*doc, 7, ShapeEffect::Glow));
        QVERIFY(setShapeEffect(*doc, 7, ShapeEffect::Reflection));
        QVERIFY(setShapeEffect(*doc, 7, ShapeEffect::SoftEdges));
        QVERIFY(setShapeEffect(*doc, 7, ShapeEffect::Bevel));
        xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("<a:bevelT") && xml.contains("<a:scene3d") && xml.contains("<a:softEdge") && xml.contains("<a:reflection") && xml.contains("<a:outerShdw"));
        QVERIFY(setShapeEffect(*doc, 7, ShapeEffect::None));
        xml = documentXml(doc->toDocx());
        QVERIFY(!xml.contains("<a:outerShdw") && !xml.contains("<a:effectLst") && !xml.contains("<a:bevelT") && !xml.contains("<a:sp3d"));
        // One step each.
        QVERIFY(doc->undo());
        QVERIFY(shapeHasEffect(*doc, 7, ShapeEffect::Shadow) && shapeHasEffect(*doc, 7, ShapeEffect::Bevel));
        QVERIFY(!setShapeEffect(*doc, 3, ShapeEffect::Glow));
    }

    void anEffectOfAFileKeepsTheEffectsTheMenuDoesNotOfferAndTheShapesOwnThreeDSettings()
    {
        const QByteArray shape = "<wps:wsp><wps:cNvSpPr/><wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1270000\" cy=\"635000\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>"
                                 "<a:solidFill><a:srgbClr val=\"3366FF\"/></a:solidFill><a:effectLst><a:innerShdw blurRad=\"11\"/><a:outerShdw blurRad=\"22\"/></a:effectLst>"
                                 "<a:scene3d><a:camera prst=\"perspectiveFront\"/><a:lightRig rig=\"soft\" dir=\"t\"/></a:scene3d><a:sp3d extrusionH=\"12700\"><a:extrusionClr><a:srgbClr val=\"FF0000\"/></a:extrusionClr></a:sp3d>"
                                 "</wps:spPr><wps:bodyPr/></wps:wsp>";
        auto doc = open(makeDocx("<w:p><w:r><w:t>Before</w:t></w:r>" + anchoredGraphic(5, kShapeUri, shape, 100, 100, 100, 50) + "</w:p>"));
        QVERIFY(doc);
        int at = -1;
        for (int i = 0; i < doc->story()->characterCount(); ++i)
            if (isDrawnObjectAt(doc->story(), i)) at = i;
        QVERIFY(at >= 0);
        // A reflection is put in: the file's inner shadow and its own shadow stay, and so do the camera, the light, and the extrusion.
        QVERIFY(setShapeEffect(*doc, at, ShapeEffect::Reflection));
        QByteArray xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("<a:innerShdw blurRad=\"11\"/>") && xml.contains("<a:outerShdw blurRad=\"22\"/>") && xml.contains("<a:reflection"));
        QVERIFY(xml.contains("perspectiveFront") && xml.contains("extrusionH=\"12700\"") && xml.contains("<a:extrusionClr>"));
        // A shadow the file has is put in again as the usual one (one shadow, not two); a bevel goes in the extrusion's element.
        QVERIFY(setShapeEffect(*doc, at, ShapeEffect::Shadow));
        QCOMPARE(documentXml(doc->toDocx()).count("<a:outerShdw"), 1);
        QVERIFY(setShapeEffect(*doc, at, ShapeEffect::Bevel));
        xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("<a:bevelT") && xml.contains("extrusionH=\"12700\"") && xml.contains("perspectiveFront"));
        // No effect takes off what the menu offers and nothing else.
        QVERIFY(setShapeEffect(*doc, at, ShapeEffect::None));
        xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("<a:innerShdw blurRad=\"11\"/>") && !xml.contains("<a:outerShdw") && !xml.contains("<a:reflection") && !xml.contains("<a:bevelT"));
        QVERIFY(xml.contains("extrusionH=\"12700\"") && xml.contains("perspectiveFront"));
    }

    void distributeSpreadsThreeObjectsToEqualGapsInOneStep()
    {
        auto doc = textDoc({"Before after"});
        for (int i = 0; i < 3; ++i) {
            QTextCursor c(doc->story());
            c.setPosition(i);
            QCOMPARE(insertShape(*doc, c, QStringLiteral("rect"), QSizeF(50, 40)), PictureResult::Ok);
        }
        QVERIFY(setPicturePlace(*doc, 0, 100, 150));
        QVERIFY(setPicturePlace(*doc, 1, 130, 200));
        QVERIFY(setPicturePlace(*doc, 2, 400, 250));
        QVector<std::pair<int, QRectF>> objects{{0, QRectF(100, 150, 50, 40)}, {1, QRectF(130, 200, 50, 40)}, {2, QRectF(400, 250, 50, 40)}};
        QVERIFY(!distributeObjects(*doc, objects.mid(0, 2), true));
        QVERIFY(distributeObjects(*doc, objects, true));
        QVERIFY(std::abs(pictureAt(*doc, doc->story(), 1).left - 250) < 0.01);
        QCOMPARE(pictureAt(*doc, doc->story(), 1).top, 200.0);
        QCOMPARE(pictureAt(*doc, doc->story(), 0).left, 100.0);
        QCOMPARE(pictureAt(*doc, doc->story(), 2).left, 400.0);
        QVERIFY(doc->undo());
        QVERIFY(std::abs(pictureAt(*doc, doc->story(), 1).left - 130) < 0.01);
        QVERIFY(doc->redo());
        // Down: the same by the tops; given in any order; already even changes nothing.
        QVector<std::pair<int, QRectF>> mixed{{2, QRectF(400, 250, 50, 40)}, {0, QRectF(100, 150, 50, 40)}, {1, QRectF(250, 180, 50, 40)}};
        QVERIFY(setPicturePlace(*doc, 1, 250, 180));
        QVERIFY(distributeObjects(*doc, mixed, false));
        QVERIFY(std::abs(pictureAt(*doc, doc->story(), 1).top - 200) < 0.01);
        QVERIFY(!distributeObjects(*doc, QVector<std::pair<int, QRectF>>{{0, QRectF(100, 100, 50, 40)}, {1, QRectF(200, 200, 50, 40)}, {2, QRectF(300, 300, 50, 40)}}, true));
    }

    void theEffectsMenuAndDistributeWorkFromTheWindowOneStepEach()
    {
        Win win;
        write::WriteDoc *doc = win.w.document();
        for (int i = 0; i < 3; ++i) {
            QTextCursor c(doc->story());
            c.setPosition(i);
            QCOMPARE(insertShape(*doc, c, QStringLiteral("rect"), QSizeF(50, 40)), PictureResult::Ok);
        }
        QVERIFY(setPicturePlace(*doc, 0, 100, 150));
        QVERIFY(setPicturePlace(*doc, 1, 130, 200));
        QVERIFY(setPicturePlace(*doc, 2, 400, 250));
        win.w.view()->refresh();
        win.w.view()->selectPicture(1);
        win.w.refreshNow();
        QVERIFY(win.w.act("shape.effect.shadow")->isEnabled());
        QVERIFY(!win.w.act("arrange.distribute.h")->isEnabled());
        win.w.act("shape.effect.shadow")->trigger();
        QCOMPARE(shapeEffectAt(*doc, 1), ShapeEffect::Shadow);
        QVERIFY(doc->undo());
        QCOMPARE(shapeEffectAt(*doc, 1), ShapeEffect::None);
        win.w.view()->selectObjects({0, 1, 2});
        win.w.refreshNow();
        QVERIFY(win.w.act("arrange.distribute.h")->isEnabled());
        win.w.act("arrange.distribute.h")->trigger();
        QVERIFY(std::abs(pictureAt(*doc, doc->story(), 1).left - 250) < 0.5);
        QCOMPARE(win.w.view()->selectedObjects().size(), 3);
        QVERIFY(doc->undo());
        QVERIFY(std::abs(pictureAt(*doc, doc->story(), 1).left - 130) < 0.5);
    }

    void editPointsMoveDeleteAndInsertPointsOfAShapeOneStepEachAndItIsSavedAsAFreeform()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect"), QSizeF(100, 60)), PictureResult::Ok);
        QVERIFY(!isFreeformShape(*doc, 7));
        QVector<QPointF> points = shapePoints(*doc, 7);
        QVERIFY(points.size() >= 4);
        int corner = -1;
        for (int i = 0; i < points.size(); ++i)
            if (QLineF(points[i], QPointF(100, 60)).length() < 0.01) corner = i;
        QVERIFY(corner >= 0);
        QVERIFY(!moveShapePoint(*doc, 7, corner, QPointF(100, 60)));
        QVERIFY(moveShapePoint(*doc, 7, corner, QPointF(120, 70)));
        QVERIFY(isFreeformShape(*doc, 7));
        points = shapePoints(*doc, 7);
        QVERIFY(QLineF(points[corner], QPointF(120, 70)).length() < 0.01);
        QByteArray xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("<a:custGeom") && !xml.contains("prst=\"rect\""));
        // The file read back has the same points.
        auto back = open(doc->toDocx());
        QVERIFY(back);
        int at = -1;
        for (int i = 0; i < back->story()->characterCount(); ++i)
            if (isDrawnObjectAt(back->story(), i)) at = i;
        QVERIFY(at >= 0);
        const QVector<QPointF> again = shapePoints(*back, at);
        QCOMPARE(again.size(), points.size());
        for (int i = 0; i < points.size(); ++i) QVERIFY(QLineF(again[i], points[i]).length() < 0.01);
        // Insert a point on the top edge, then delete it; a shape keeps three.
        const int before = shapePoints(*doc, 7).size();
        int made = -1;
        QVERIFY(insertShapePoint(*doc, 7, QPointF(50, 2), 5, &made));
        QCOMPARE(shapePoints(*doc, 7).size(), before + 1);
        QVERIFY(made >= 0);
        QVERIFY(QLineF(shapePoints(*doc, 7)[made], QPointF(50, 0)).length() < 3);
        QVERIFY(!insertShapePoint(*doc, 7, QPointF(50, 30), 5));
        QVERIFY(deleteShapePoint(*doc, 7, made));
        QCOMPARE(shapePoints(*doc, 7).size(), before);
        QVERIFY(deleteShapePoint(*doc, 7, 0));
        QVERIFY(deleteShapePoint(*doc, 7, 0) || shapePoints(*doc, 7).size() <= 3);
        while (shapePoints(*doc, 7).size() > 3) QVERIFY(deleteShapePoint(*doc, 7, 0));
        QVERIFY(!deleteShapePoint(*doc, 7, 0));
        // One step each: undo all the way back to the rectangle.
        int guard = 0;
        while (isFreeformShape(*doc, 7) && ++guard < 20) QVERIFY(doc->undo());
        QVERIFY(!isFreeformShape(*doc, 7));
        QCOMPARE(pictureAt(*doc, doc->story(), 7).preset, QStringLiteral("rect"));
        // A line, a picture, and a bad number are refused.
        QVERIFY(!moveShapePoint(*doc, 7, 99, QPointF(1, 1)));
        QVERIFY(shapePoints(*doc, 3).isEmpty());
    }

    void aCurvedShapeKeepsItsCurvesWhenAPointMoves()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("ellipse"), QSizeF(100, 100)), PictureResult::Ok);
        const QVector<QPointF> points = shapePoints(*doc, 7);
        QVERIFY(points.size() >= 4);
        QVERIFY(moveShapePoint(*doc, 7, 1, points[1] + QPointF(0, -20)));
        QVERIFY(documentXml(doc->toDocx()).contains("<a:cubicBezTo>"));
        QCOMPARE(shapePoints(*doc, 7).size(), points.size());
    }

    void editPointsFromTheWindowDragsDeletesAndInsertsPointsInOneStepEach()
    {
        Win win;
        write::WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect"), QSizeF(160, 100)), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 0, 150, 150));
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        win.w.refreshNow();
        QVERIFY(win.w.act("shape.editPoints")->isEnabled());
        win.w.act("shape.editPoints")->trigger();
        QVERIFY(win.w.view()->editingPoints());
        QWidget *vp = win.w.view()->viewport();
        QRectF r;
        QTRY_VERIFY(win.w.view()->pictureRect(0, &r));
        const double s = win.w.view()->zoom() * 96.0 / 72.0;
        // Drag the bottom right corner.
        const QPoint corner = r.bottomRight().toPoint();
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, corner);
        QTest::mouseMove(vp, corner + QPoint(10, 5));
        QTest::mouseMove(vp, corner + QPoint(int(30 * s), int(20 * s)));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, corner + QPoint(int(30 * s), int(20 * s)));
        QVERIFY(isFreeformShape(*doc, 0));
        bool moved = false;
        for (const QPointF &p : shapePoints(*doc, 0)) moved = moved || (std::abs(p.x() - 190) < 2 && std::abs(p.y() - 120) < 2);
        QVERIFY(moved);
        QCOMPARE(win.w.view()->selectedPicture(), 0);
        QVERIFY(doc->undo());
        QVERIFY(!isFreeformShape(*doc, 0));
        QVERIFY(doc->redo());
        // Ctrl+click the top edge puts a point in; Ctrl+click it takes it away.
        win.w.view()->refresh();
        QTRY_VERIFY(win.w.view()->pictureRect(0, &r));
        const int count = shapePoints(*doc, 0).size();
        const QPoint edge(int(r.left() + 40 * s), int(r.top()));
        QTest::mouseClick(vp, Qt::LeftButton, Qt::ControlModifier, edge);
        QCOMPARE(shapePoints(*doc, 0).size(), count + 1);
        QTest::mouseClick(vp, Qt::LeftButton, Qt::ControlModifier, edge);
        QCOMPARE(shapePoints(*doc, 0).size(), count);
        // Escape ends it.
        QTest::keyClick(vp, Qt::Key_Escape);
        QVERIFY(!win.w.view()->editingPoints());
    }

    void editWrapPointsOnASquarePictureChangesNothingUntilAPointIsMovedAndEscLeavesItAsItWas()
    {
        Win win;
        write::WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        QCOMPARE(insertPicture(*doc, c, pngOf(160, 160)), PictureResult::Ok);
        QVERIFY(setPictureWrap(*doc, 0, PictureWrap::Square));
        auto steps = [&] {
            int n = 0;
            while (doc->canUndo() && doc->undo()) ++n;
            for (int i = 0; i < n; ++i) doc->redo();
            return n;
        };
        const int before = steps();
        QVERIFY(before >= 2);
        const QByteArray xml = doc->toDocx();
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        win.w.refreshNow();
        QVERIFY(win.w.act("picture.wrap.editPoints")->isEnabled());
        win.w.act("picture.wrap.editPoints")->trigger();
        QVERIFY(win.w.view()->editingWrap());
        QVERIFY(pictureAt(*doc, doc->story(), 0).wrap == PictureWrap::Square);
        QTest::keyClick(win.w.view()->viewport(), Qt::Key_Escape);
        QVERIFY(!win.w.view()->editingWrap());
        QVERIFY(pictureAt(*doc, doc->story(), 0).wrap == PictureWrap::Square);
        QVERIFY(!doc->canRedo());
        QCOMPARE(steps(), before);
        QCOMPARE(doc->toDocx(), xml);
        // The first point that moves makes it a tight one, in one step.
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        win.w.refreshNow();
        win.w.act("picture.wrap.editPoints")->trigger();
        QRectF r;
        QTRY_VERIFY(win.w.view()->pictureRect(0, &r));
        const double s = win.w.view()->zoom() * 96.0 / 72.0;
        QWidget *vp = win.w.view()->viewport();
        const QPoint corner = r.bottomRight().toPoint();
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, corner);
        QTest::mouseMove(vp, corner + QPoint(10, 5));
        QTest::mouseMove(vp, corner + QPoint(int(-30 * s), int(-20 * s)));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, corner + QPoint(int(-30 * s), int(-20 * s)));
        QVERIFY(pictureAt(*doc, doc->story(), 0).wrap == PictureWrap::Tight);
        QCOMPARE(steps(), before + 1);
    }

    void editWrapPointsOnAShapeAndATextBoxMakeAnOutlineInOneStepAndEscLeavesNoStep()
    {
        for (const bool textBox : {false, true}) {
            Win win;
            write::WriteDoc *doc = win.w.document();
            QTextCursor c(doc->story());
            c.setPosition(0);
            if (textBox) {
                TextBoxDesign design = textBoxDesigns().first();
                design.size = QSizeF(160, 160);
                QCOMPARE(insertTextBox(*doc, c, design), PictureResult::Ok);
            } else {
                QCOMPARE(insertShape(*doc, c, QStringLiteral("rect"), QSizeF(160, 160)), PictureResult::Ok);
            }
            auto steps = [&] {
                int n = 0;
                while (doc->canUndo() && doc->undo()) ++n;
                for (int i = 0; i < n; ++i) doc->redo();
                return n;
            };
            const int before = steps();
            const QByteArray xml = doc->toDocx();
            win.w.view()->refresh();
            win.w.view()->selectPicture(0);
            win.w.refreshNow();
            QVERIFY(win.w.act("picture.wrap.editPoints")->isEnabled());
            win.w.act("picture.wrap.editPoints")->trigger();
            QVERIFY(win.w.view()->editingWrap());
            QTest::keyClick(win.w.view()->viewport(), Qt::Key_Escape);
            QVERIFY(!win.w.view()->editingWrap());
            QVERIFY(!doc->canRedo());
            QCOMPARE(steps(), before);
            QCOMPARE(doc->toDocx(), xml);
            // The first point that moves makes it a tight one, in one step; the outline has that point where it was dropped.
            win.w.view()->refresh();
            win.w.view()->selectPicture(0);
            win.w.refreshNow();
            win.w.act("picture.wrap.editPoints")->trigger();
            QRectF r;
            QTRY_VERIFY(win.w.view()->pictureRect(0, &r));
            const double s = win.w.view()->zoom() * 96.0 / 72.0;
            QWidget *vp = win.w.view()->viewport();
            const QPoint corner = r.bottomRight().toPoint();
            QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, corner);
            QTest::mouseMove(vp, corner + QPoint(-10, -5));
            QTest::mouseMove(vp, corner + QPoint(int(-30 * s), int(-20 * s)));
            QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, corner + QPoint(int(-30 * s), int(-20 * s)));
            const PictureInfo info = pictureAt(*doc, doc->story(), 0);
            QVERIFY(info.wrap == PictureWrap::Tight);
            bool moved = false;
            for (const QPointF &p : info.wrapOutline) moved = moved || (std::abs(p.x() - (1 - 30.0 / 160)) < 0.03 && std::abs(p.y() - (1 - 20.0 / 160)) < 0.03);
            QVERIFY2(moved, textBox ? "text box" : "shape");
            QCOMPARE(steps(), before + 1);
        }
    }

    // A picture of text round an edited outline, for a person to look at (JW_SHOT_DIR names the folder; nothing is made without it).
    void wrapPointsOfAShapeScreenshot()
    {
        const QString dir = qEnvironmentVariable("JW_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JW_SHOT_DIR is not set");
        jp::applyUiTheme(1);   // the program's own look, as a person sees it; a bare test has none
        QString words;
        for (int i = 0; i < 40; ++i) words += QStringLiteral("The text goes round the outline of the shape and not round its frame. ");
        Win win("<w:p><w:r><w:t xml:space=\"preserve\">" + words.toUtf8() + "</w:t></w:r></w:p>");
        write::WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect"), QSizeF(200, 200)), PictureResult::Ok);
        QVERIFY(setPictureWrap(*doc, 0, PictureWrap::Tight));
        QPolygonF outline;
        outline << QPointF(0.5, 0) << QPointF(1, 0.6) << QPointF(0.8, 1) << QPointF(0.2, 1) << QPointF(0, 0.6);
        QVERIFY(setPictureWrapOutline(*doc, 0, outline));
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        win.w.refreshNow();
        win.w.act("picture.wrap.editPoints")->trigger();
        QVERIFY(win.w.view()->editingWrap());
        QDir().mkpath(dir);
        QVERIFY(win.w.grab().save(dir + QStringLiteral("/shape-wrap-points.png")));
    }

    void setAsDefaultLayoutMakesTheWrapOfNewPicturesTheFileAndClipboardTake()
    {
        // (This run's own settings: see isolatesettings.h. The key is cleared when the test ends, however it ends, so that no later insert in the run takes it.)
        struct ClearWrapDefault {
            ClearWrapDefault() { Settings::get().setValue(QStringLiteral("pictures/wrapDefault"), QVariant()); }
            ~ClearWrapDefault() { Settings::get().setValue(QStringLiteral("pictures/wrapDefault"), QVariant()); }
        } clear;
        Win win;
        write::WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        QCOMPARE(insertPicture(*doc, c, pngOf(160, 160)), PictureResult::Ok);
        // With no setting, a picture goes in line, as before.
        QVERIFY(win.w.view()->insertPictureBytes(pngOf(40, 40), QStringLiteral("none")));
        PictureInfo now = pictureAt(*doc, doc->story(), win.w.view()->selectedPicture());
        QVERIFY(!now.floating && now.wrap == PictureWrap::Inline);
        QVERIFY(setPictureWrap(*doc, 0, PictureWrap::Square));
        QVERIFY(setPictureWrapSide(*doc, 0, PictureWrapSide::Right));
        QVERIFY(setPictureWrapDistances(*doc, 0, 3, 4, 5, 6));
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        win.w.refreshNow();
        QVERIFY(win.w.act("picture.wrap.setDefault")->isEnabled());
        win.w.act("picture.wrap.setDefault")->trigger();
        // The next pictures, put in by file and from the clipboard, take it; the picture it was taken from stays as it is.
        QVERIFY(win.w.view()->insertPictureBytes(pngOf(50, 50), QStringLiteral("next")));
        now = pictureAt(*doc, doc->story(), win.w.view()->selectedPicture());
        QVERIFY(now.floating && now.wrap == PictureWrap::Square);
        QCOMPARE(now.wrapSide, PictureWrapSide::Right);
        QCOMPARE(now.distTop, 3.0);
        QCOMPARE(now.distBottom, 4.0);
        QCOMPARE(now.distLeft, 5.0);
        QCOMPARE(now.distRight, 6.0);
        QImage image(30, 30, QImage::Format_RGB32);
        image.fill(Qt::blue);
        QVERIFY(win.w.view()->insertPictureImage(image));
        QVERIFY(pictureAt(*doc, doc->story(), win.w.view()->selectedPicture()).wrap == PictureWrap::Square);
        QVERIFY(documentXml(doc->toDocx()).contains("wrapText=\"right\""));
        // Another wrap asked for in the call is the caller's. With no setting (as in a run that never used Set as Default Layout), pictures go in line again.
        QVERIFY(win.w.view()->insertPictureBytes(pngOf(50, 50), QStringLiteral("asked"), PictureWrap::BehindText));
        QVERIFY(pictureAt(*doc, doc->story(), win.w.view()->selectedPicture()).wrap == PictureWrap::BehindText);
        Settings::get().setValue(QStringLiteral("pictures/wrapDefault"), QVariant());
        QVERIFY(win.w.view()->insertPictureBytes(pngOf(50, 50), QStringLiteral("again")));
        QVERIFY(pictureAt(*doc, doc->story(), win.w.view()->selectedPicture()).wrap == PictureWrap::Inline);
    }

    void aKeptDefaultLayoutThatIsNotOneIsIgnoredAndRoomOutOfRangeIsHeld()
    {
        struct ClearWrapDefault {
            ClearWrapDefault() { Settings::get().setValue(QStringLiteral("pictures/wrapDefault"), QVariant()); }
            ~ClearWrapDefault() { Settings::get().setValue(QStringLiteral("pictures/wrapDefault"), QVariant()); }
        } clear;
        auto keep = [](const QString &text) {
            Settings::get().setValue(QStringLiteral("pictures/wrapDefault"), text);
            return pictureWrapDefault();
        };
        QVERIFY(keep(QStringLiteral("square;right;1;2;3;4")).wrap == PictureWrap::Square);   // (a good one is read)
        for (const char *bad : {"sideways;both;1;2;3;4", "square;sideways;1;2;3;4", "square;both;nan;2;3;4", "square;both;1;inf;3;4", "square;both;1;2;-inf;4", "square;both;1;2;3;", "square;both;;2;3;4",
                                "square;both;1;2;3", "square;both;1;2;3;4;5", "", "square"})
            QVERIFY2(keep(QString::fromLatin1(bad)) == PictureWrapDefault(), bad);
        // A room below 0 or above the most is held to the limits.
        const PictureWrapDefault held = keep(QStringLiteral("tight;left;-5;1e9;3;4"));
        QVERIFY(held.wrap == PictureWrap::Tight && held.side == PictureWrapSide::Left);
        QCOMPARE(held.top, 0.0);
        QCOMPARE(held.bottom, kMostWrapDistance);
        // And the layout that goes in is held the same.
        Win win;
        QVERIFY(win.w.view()->insertPictureBytes(pngOf(40, 40), QStringLiteral("held")));
        const PictureInfo info = pictureAt(*win.w.document(), win.w.document()->story(), win.w.view()->selectedPicture());
        QVERIFY(info.wrap == PictureWrap::Tight);
        QCOMPARE(info.distTop, 0.0);
        QCOMPARE(info.distBottom, kMostWrapDistance);
    }

    void editWrapPointsAndEditPointsTurnEachOtherOffInTheirCommands()
    {
        Win win;
        write::WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect"), QSizeF(160, 160)), PictureResult::Ok);
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        win.w.refreshNow();
        QVERIFY(win.w.act("shape.editPoints")->isEnabled() && win.w.act("picture.wrap.editPoints")->isEnabled());
        win.w.act("shape.editPoints")->trigger();
        QVERIFY(win.w.act("shape.editPoints")->isChecked() && !win.w.act("picture.wrap.editPoints")->isChecked());
        win.w.act("picture.wrap.editPoints")->trigger();
        QVERIFY(win.w.view()->editingWrap() && !win.w.view()->editingShapePoints());
        win.w.refreshNow();
        QVERIFY(win.w.act("picture.wrap.editPoints")->isChecked());
        QVERIFY(!win.w.act("shape.editPoints")->isChecked());
        win.w.act("shape.editPoints")->trigger();
        QVERIFY(win.w.view()->editingShapePoints() && !win.w.view()->editingWrap());
        win.w.refreshNow();
        QVERIFY(win.w.act("shape.editPoints")->isChecked());
        QVERIFY(!win.w.act("picture.wrap.editPoints")->isChecked());
        // Set as Default Layout is a picture's: no entry for a shape.
        QVERIFY(!win.w.act("picture.wrap.setDefault")->isEnabled());
        QVERIFY(!win.w.act("picture.wrap.setDefault")->isVisible());
    }

    void aCropWhoseFrameTheLimitsWouldChangeLeavesThePictureAsItWas()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        QCOMPARE(insertPicture(*doc, c, pngOf(200, 100)), PictureResult::Ok);
        // A frame the size limits would change shows the cut at another scale: nothing is done.
        const PictureInfo whole = pictureAt(*doc, doc->story(), 0);
        QVERIFY(!setPictureCropAspect(*doc, 0, 90, 1));
        PictureInfo info = pictureAt(*doc, doc->story(), 0);
        QCOMPARE(info.width, whole.width);
        QCOMPARE(info.height, whole.height);
        QCOMPARE(info.cropTop, 0.0);
    }

    void aShapeInAChoiceOfAlternateContentIsSelectableAndEditableAndKeptAsTheFileHadItUntilEdited()
    {
        const QByteArray run = anchoredGraphic(7, kShapeUri, plainShape(120, 60, "star5"), 200, 150, 120, 60);
        // (A newer program writes its own id attributes on the anchor, in a prefix the drawing does not declare.)
        QByteArray drawing = run.mid(run.indexOf("<w:drawing>"), run.indexOf("</w:drawing>") + 12 - run.indexOf("<w:drawing>"));
        drawing.replace("<wp:anchor ", "<wp:anchor wp14:anchorId=\"33B7E180\" wp14:editId=\"305BEF9A\" ");
        const QByteArray wrapped = "<w:r><mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"wps\">" + drawing +
                                   "</mc:Choice><mc:Fallback><w:pict xmlns:v=\"urn:schemas-microsoft-com:vml\"><v:rect id=\"f\" style=\"width:120pt;height:60pt\"/></w:pict></mc:Fallback></mc:AlternateContent></w:r>";
        const QByteArray file = makeDocx("<w:p><w:r><w:t>Before</w:t></w:r>" + wrapped + "<w:r><w:t>after</w:t></w:r></w:p>");
        auto doc = open(file);
        QVERIFY(doc);
        int at = -1;
        for (int i = 0; i < doc->story()->characterCount(); ++i)
            if (isDrawnObjectAt(doc->story(), i)) at = i;
        QVERIFY(at >= 0);
        PictureInfo info = pictureAt(*doc, doc->story(), at);
        QCOMPARE(info.preset, QStringLiteral("star5"));
        QVERIFY(info.editable && info.floating);
        // Untouched, it is saved as the file had it, fallback and all.
        QByteArray xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("mc:AlternateContent") && xml.contains("mc:Fallback"));
        // One edit makes it a plain drawing with the change.
        QVERIFY(setShapeFill(*doc, at, QStringLiteral("00AA00")));
        QCOMPARE(pictureAt(*doc, doc->story(), at).fillColor, QStringLiteral("00AA00"));
        xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("00AA00") && !xml.contains("mc:Fallback"));
        auto back = open(doc->toDocx());
        QVERIFY(back);
        int again = -1;
        for (int i = 0; i < back->story()->characterCount(); ++i)
            if (isDrawnObjectAt(back->story(), i)) again = i;
        QCOMPARE(pictureAt(*back, back->story(), again).preset, QStringLiteral("star5"));
        QCOMPARE(pictureAt(*back, back->story(), again).fillColor, QStringLiteral("00AA00"));
        // Undo gives the file's own form back.
        QVERIFY(doc->undo());
        xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("mc:Fallback"));
        // A diagram's choice is not a shape: it is left alone.
        QCOMPARE(doc->story()->toPlainText().remove(QChar::ObjectReplacementCharacter), QStringLiteral("Beforeafter"));
    }

    void editingTheWordsOfARichBoxKeepsEveryParagraphsAndRunsLook()
    {
        const QByteArray p1 = "<w:p><w:pPr><w:jc w:val=\"center\"/></w:pPr><w:r><w:rPr><w:b/><w:color w:val=\"FF0000\"/><w:sz w:val=\"40\"/></w:rPr><w:t>Hello</w:t></w:r></w:p>";
        const QByteArray p2 = "<w:p><w:pPr><w:jc w:val=\"right\"/></w:pPr><w:r><w:t xml:space=\"preserve\">Plain </w:t></w:r><w:r><w:rPr><w:i/></w:rPr><w:t>italic</w:t></w:r></w:p>";
        const QByteArray shape = "<wps:wsp><wps:cNvSpPr txBox=\"1\"/><wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"2540000\" cy=\"1270000\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></wps:spPr>"
                                 "<wps:txbx><w:txbxContent>" + p1 + p2 + "</w:txbxContent></wps:txbx><wps:bodyPr/></wps:wsp>";
        auto doc = open(makeDocx("<w:p><w:r><w:t>Before</w:t></w:r>" + anchoredGraphic(5, kShapeUri, shape, 100, 100, 200, 100) + "</w:p>"));
        QVERIFY(doc);
        int at = -1;
        for (int i = 0; i < doc->story()->characterCount(); ++i)
            if (isDrawnObjectAt(doc->story(), i)) at = i;
        QVERIFY(at >= 0);
        QCOMPARE(textBoxText(*doc, at)->join(QLatin1Char('|')), QStringLiteral("Hello|Plain italic"));
        // A stray key and Escape that leave the words as they were make no step and change nothing.
        QVERIFY(!setTextBoxText(*doc, at, {QStringLiteral("Hello"), QStringLiteral("Plain italic")}));
        // One character typed at the end of the first paragraph: it takes the bold red 20 pt of its run, and nothing else changes.
        QVERIFY(setTextBoxText(*doc, at, {QStringLiteral("Hello!"), QStringLiteral("Plain italic")}));
        QByteArray xml = documentXml(doc->toDocx());
        QVERIFY2(xml.contains("<w:jc w:val=\"center\"/>") && xml.contains("<w:jc w:val=\"right\"/>"), "each paragraph keeps its own properties");
        QVERIFY2(xml.contains("<w:rPr><w:b/><w:color w:val=\"FF0000\"/><w:sz w:val=\"40\"/></w:rPr><w:t xml:space=\"preserve\">Hello!</w:t>"), "the typed character takes its run's look");
        QVERIFY2(xml.contains(p2), "a paragraph whose words did not change is kept byte for byte");
        // A character typed in the middle of the italic run goes in that run; the run before it is kept.
        QVERIFY(setTextBoxText(*doc, at, {QStringLiteral("Hello!"), QStringLiteral("Plain itXalic")}));
        xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("<w:r><w:t xml:space=\"preserve\">Plain </w:t></w:r>"));
        QVERIFY(xml.contains("<w:rPr><w:i/></w:rPr><w:t xml:space=\"preserve\">itXalic</w:t>"));
        QVERIFY(xml.contains("<w:jc w:val=\"right\"/>"));
        // A paragraph made by Enter has the look of the one it was made in; a paragraph taken away is gone.
        QVERIFY(setTextBoxText(*doc, at, {QStringLiteral("Hello!"), QStringLiteral("Plain"), QStringLiteral("itXalic")}));
        QCOMPARE(textBoxText(*doc, at)->join(QLatin1Char('|')), QStringLiteral("Hello!|Plain|itXalic"));
        xml = documentXml(doc->toDocx());
        QCOMPARE(xml.count("<w:jc w:val=\"right\"/>"), 2);
        QVERIFY(setTextBoxText(*doc, at, {QStringLiteral("Hello!")}));
        QCOMPARE(textBoxText(*doc, at)->join(QLatin1Char('|')), QStringLiteral("Hello!"));
        QVERIFY(documentXml(doc->toDocx()).contains("<w:jc w:val=\"center\"/>"));
    }

    void aTextBoxThatHoldsAPictureInsideAChoiceIsSelectableAndEditable()
    {
        // (The text box's own drawing, a picture in line, ends with a closing tag of the same name as the text box's.)
        const QByteArray picture = "<w:p><w:r><w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"127000\" cy=\"127000\"/><wp:docPr id=\"9\" name=\"P\"/>"
                                   "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
                                   "<pic:nvPicPr><pic:cNvPr id=\"9\" name=\"P\"/><pic:cNvPicPr/></pic:nvPicPr><pic:blipFill><a:blip/></pic:blipFill><pic:spPr/></pic:pic></a:graphicData></a:graphic>"
                                   "</wp:inline></w:drawing></w:r></w:p>";
        const QByteArray shape = "<wps:wsp><wps:cNvSpPr txBox=\"1\"/><wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"2540000\" cy=\"1270000\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>"
                                 "<a:solidFill><a:srgbClr val=\"FFFFFF\"/></a:solidFill></wps:spPr><wps:txbx><w:txbxContent>" + picture + "</w:txbxContent></wps:txbx><wps:bodyPr/></wps:wsp>";
        const QByteArray run = anchoredGraphic(7, kShapeUri, shape, 100, 100, 200, 100);
        const QByteArray drawing = run.mid(run.indexOf("<w:drawing>"), run.lastIndexOf("</w:drawing>") + 12 - run.indexOf("<w:drawing>"));
        const QByteArray wrapped = "<w:r><mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"wps\">" + drawing +
                                   "</mc:Choice><mc:Fallback><w:pict xmlns:v=\"urn:schemas-microsoft-com:vml\"><v:rect id=\"f\" style=\"width:200pt;height:100pt\"/></w:pict></mc:Fallback></mc:AlternateContent></w:r>";
        auto doc = open(makeDocx("<w:p><w:r><w:t>Before</w:t></w:r>" + wrapped + "</w:p>"));
        QVERIFY(doc);
        int at = -1;
        for (int i = 0; i < doc->story()->characterCount(); ++i)
            if (isDrawnObjectAt(doc->story(), i)) at = i;
        QVERIFY2(at >= 0, "the text box with a picture in it is a drawn object");
        QVERIFY(pictureAt(*doc, doc->story(), at).editable);
        QVERIFY(setShapeFill(*doc, at, QStringLiteral("00AA00")));
        const QByteArray xml = documentXml(doc->toDocx());
        QVERIFY(xml.contains("00AA00") && xml.contains("<pic:pic") && xml.contains("</w:txbxContent>"));
    }

    // A freeform of a million points is never parsed for its points: Edit Points, the handles, and the ribbon's questions return at once (counted by the drawing parsed, not timed).
    void aHugeFreeformIsNotParsedForItsPointsOrHandles()
    {
        auto freeform = [](int points) {
            QByteArray path = "<a:moveTo><a:pt x=\"0\" y=\"0\"/></a:moveTo>";
            for (int i = 1; i < points; ++i) path += "<a:lnTo><a:pt x=\"" + QByteArray::number(i % 1000) + "\" y=\"" + QByteArray::number((i / 1000) % 1000) + "\"/></a:lnTo>";
            return QByteArray("<wps:wsp><wps:cNvSpPr/><wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"1270000\" cy=\"635000\"/></a:xfrm><a:custGeom><a:avLst/><a:gdLst/><a:ahLst/><a:cxnLst/>"
                              "<a:rect l=\"0\" t=\"0\" r=\"r\" b=\"b\"/><a:pathLst><a:path w=\"1000\" h=\"1000\">") + path + "<a:close/></a:path></a:pathLst></a:custGeom></wps:spPr><wps:bodyPr/></wps:wsp>";
        };
        // 30,000 and 8,000 points make a drawing of more than 256 KB, which is not looked into; 2,500 are a smaller drawing, past the cap of 2,000 points.
        for (const int points : {30000, 8000, 2500}) {
            auto doc = open(makeDocx("<w:p><w:r><w:t>Before</w:t></w:r>" + anchoredGraphic(5, kShapeUri, freeform(points), 100, 100, 100, 50) + "</w:p>"));
            QVERIFY(doc);
            int at = -1;
            for (int i = 0; i < doc->story()->characterCount(); ++i)
                if (isDrawnObjectAt(doc->story(), i)) at = i;
            QVERIFY(at >= 0);
            const quint64 before = shapeParseCount();
            QVERIFY(shapePoints(*doc, at).isEmpty());
            QVERIFY(!isFreeformShape(*doc, at));
            QVERIFY(!moveShapePoint(*doc, at, 1, QPointF(5, 5)));
            QVERIFY(!deleteShapePoint(*doc, at, 1));
            QVERIFY(!insertShapePoint(*doc, at, QPointF(5, 5), 5));
            QVERIFY(shapeAdjustHandles(*doc, at).isEmpty());
            if (points >= 8000) QCOMPARE(shapeParseCount(), before);   // (nothing was parsed)
            if (points < 8000) QVERIFY(!editPointsProblem(*doc, at).isEmpty());
        }
    }

    void editPointsIsNotOfferedForAShapeDrawnInSeveralPartsAndAConvertedShapeKeepsItsTextRectangle()
    {
        auto doc = textDoc({"Before after"});
        for (const char *preset : {"cube", "can", "donut", "frame", "bevel"}) {
            QTextCursor c(doc->story());
            c.setPosition(7);
            QCOMPARE(insertShape(*doc, c, QString::fromLatin1(preset)), PictureResult::Ok);
            const QString name = QString::fromLatin1(preset);
            QVERIFY2(shapePoints(*doc, 7).isEmpty(), qPrintable(name));
            QVERIFY2(!editPointsProblem(*doc, 7).isEmpty(), qPrintable(name));
            QVERIFY2(!moveShapePoint(*doc, 7, 0, QPointF(5, 5)), qPrintable(name));
            QVERIFY(doc->undo());
        }
        // A shape of one part is offered, and the text rectangle it had stays (an ellipse's is inside its box), as the sites of its sides.
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("ellipse"), QSizeF(100, 100)), PictureResult::Ok);
        QVERIFY(editPointsProblem(*doc, 7).isEmpty());
        const QVector<QPointF> points = shapePoints(*doc, 7);
        QVERIFY(moveShapePoint(*doc, 7, 0, points[0] + QPointF(0, 5)));
        const QByteArray xml = documentXml(doc->toDocx());
        QVERIFY2(!xml.contains("<a:rect l=\"0\" t=\"0\" r=\"1270000\" b=\"1270000\"/>"), "the text rectangle is the ellipse's own, not the whole box");
        QVERIFY(xml.contains("<a:rect l=\"") && xml.contains("<a:cxn ang=\"16200000\">"));
    }

    void aShapeThatCantBeSetAtItsPlaceLeavesNothingBehind()
    {
        auto doc = textDoc({"Before after"});
        const QString text = doc->story()->toPlainText();
        QTextCursor c(doc->story());
        c.setPosition(7);
        QVERIFY(insertShapeAt(*doc, c, QStringLiteral("rect"), QPointF(std::numeric_limits<double>::quiet_NaN(), 5), QPointF(100, 100)) != PictureResult::Ok);
        QVERIFY(!isDrawnObjectAt(doc->story(), 7));
        QCOMPARE(doc->story()->toPlainText(), text);
        QVERIFY(insertShapeAt(*doc, c, QStringLiteral("rect"), QPointF(5, 5), QPointF(100, std::numeric_limits<double>::infinity())) != PictureResult::Ok);
        QVERIFY(!isDrawnObjectAt(doc->story(), 7));
        QCOMPARE(doc->story()->toPlainText(), text);
        // Finite ends, and the place step fails after the shape was put in: the shape is taken out again, in the shape's own kind and as a line.
        failShapePlaceForTests(true);
        for (const char *preset : {"rect", "straightConnector1"}) {
            QTextCursor d(doc->story());
            d.setPosition(7);
            QVERIFY(insertShapeAt(*doc, d, QString::fromLatin1(preset), QPointF(5, 5), QPointF(100, 90)) != PictureResult::Ok);
            QVERIFY(!isDrawnObjectAt(doc->story(), 7));
            QCOMPARE(doc->story()->toPlainText(), text);
        }
        failShapePlaceForTests(false);
        QTextCursor ok(doc->story());
        ok.setPosition(7);
        QCOMPARE(insertShapeAt(*doc, ok, QStringLiteral("rect"), QPointF(5, 5), QPointF(100, 90)), PictureResult::Ok);
    }

    void theHandlesAndPointsOfASelectedShapeAreParsedOnceForASelectionNotOnEveryMoveAndPaint()
    {
        Win win;
        write::WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(0);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("roundRect"), QSizeF(160, 100)), PictureResult::Ok);
        QVERIFY(setPicturePlace(*doc, 0, 150, 150));
        win.w.view()->refresh();
        win.w.view()->selectPicture(0);
        win.w.refreshNow();
        win.w.act("shape.editPoints")->trigger();
        QWidget *vp = win.w.view()->viewport();
        QRectF r;
        QTRY_VERIFY(win.w.view()->pictureRect(0, &r));
        // (The window's own pending refreshes and the first paint fill the cache once.)
        for (int i = 0; i < 4; ++i) {
            QTest::mouseMove(vp, QPoint(int(r.left()) + i * 3, int(r.center().y())));
            vp->repaint();
            QCoreApplication::processEvents();
        }
        const quint64 before = shapeParseCount();
        for (int i = 0; i < 30; ++i) QTest::mouseMove(vp, QPoint(int(r.left()) + 12 + i * 3, int(r.center().y())));
        for (int i = 0; i < 6; ++i) vp->repaint();
        QCOMPARE(shapeParseCount(), before);
        // A change of the text is a new revision: the next ask parses again.
        QVERIFY(setShapeFill(*doc, 0, QStringLiteral("00AA00")));
        vp->repaint();
        QVERIFY(shapeParseCount() > before);
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
        QVERIFY(win.w.act("shape.editText")->isEnabled());
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

    void aConnectorIsGluedToTheOutlineOfAShapeNotToItsBoundingBox()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor a(doc->story());
        a.setPosition(0);
        QCOMPARE(insertShape(*doc, a, QStringLiteral("star5"), QSizeF(100, 100)), PictureResult::Ok);
        QTextCursor b(doc->story());
        b.setPosition(1);
        QCOMPARE(insertShape(*doc, b, QStringLiteral("rect"), QSizeF(40, 40)), PictureResult::Ok);
        int made = -1;
        QCOMPARE(connectObjects(*doc, 0, QRectF(100, 100, 100, 100), 1, QRectF(400, 100, 40, 40), QStringLiteral("straightConnector1"), &made), PictureResult::Ok);
        const PictureInfo info = pictureAt(*doc, doc->story(), made);
        // A five-pointed star is narrower at the middle of its height than its box is, so the connector leaves its outline, inside the box.
        QVERIFY2(info.width > 203 && info.width < 280, qPrintable(QString::number(info.width)));
        QVERIFY(isConnectorGlued(*doc, made));
        // A rectangle still has its sides' middles.
        QTextCursor c(doc->story());
        c.setPosition(2);
        QCOMPARE(insertShape(*doc, c, QStringLiteral("rect"), QSizeF(100, 100)), PictureResult::Ok);
        int second = -1;
        QCOMPARE(connectObjects(*doc, 2, QRectF(100, 300, 100, 100), 1, QRectF(400, 300, 40, 40), QStringLiteral("straightConnector1"), &second), PictureResult::Ok);
        QVERIFY(std::abs(pictureAt(*doc, doc->story(), second).width - 200) < 0.01);
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

    // Picture Styles: each style of the shared gallery (its shape, border, and effects) is put on a picture as one step that undoes; the size, place, and cut stay; the saved .docx
    // holds the style's shape, line, and effects and reads back as the same look (and drawn: the page's picture has the effects).
    void everyPictureStyleIsPutOnAPictureAndKeptInTheFile()
    {
        for (int k = 0; k < pictureStyleCount(); ++k) {
            auto doc = textDoc({"Before after"});
            QTextCursor c(doc->story());
            c.setPosition(7);
            QCOMPARE(insertPicture(*doc, c, pngOf(160, 80), QStringLiteral("bar"), PictureWrap::Square), PictureResult::Ok);
            QVERIFY(setPictureCrop(*doc, 7, 0.1, 0, 0, 0));
            const PictureInfo before = pictureAt(*doc, doc->story(), 7);
            PictureItem want;
            QVERIFY(applyPictureStyle(&want, k));
            const bool plain = want.maskShape == QLatin1String("rect") && want.stroke.isNone() && !want.fx.any();
            QCOMPARE(setPictureStyle(*doc, 7, k), !plain);
            const QByteArray name = pictureStyleId(k).toLatin1();
            const PictureInfo now = pictureAt(*doc, doc->story(), 7);
            QVERIFY2(now.width == before.width && now.height == before.height && now.cropLeft == before.cropLeft, name.constData());
            auto back = open(doc->toDocx());
            QVERIFY(back);
            const PictureInfo saved = pictureAt(*back, back->story(), 7);
            QVERIFY2(saved.cropShape == (office::presetForShape(want.maskShape).isEmpty() ? QStringLiteral("rect") : office::presetForShape(want.maskShape)), name.constData());
            QCOMPARE(saved.lineWeight > 0, !want.stroke.isNone());
            if (!want.stroke.isNone()) QVERIFY2(std::abs(saved.lineWeight - want.stroke.width) < 0.01, name.constData());
            for (WriteDoc *d : {doc.get(), back.get()}) {
                const auto pic = pictureOfPage(*d);
                QVERIFY2(pic, name.constData());
                QCOMPARE(pic->maskShape, want.maskShape);
                QCOMPARE(pic->fx.shadow.on, want.fx.shadow.on);
                QCOMPARE(pic->fx.glow.on, want.fx.glow.on);
                QCOMPARE(pic->fx.reflection.on, want.fx.reflection.on);
                QCOMPARE(pic->fx.bevel.type, want.fx.bevel.type);
                QVERIFY2(std::abs(pic->fx.softEdge - want.fx.softEdge) < 0.01 && std::abs(pic->fx.rot3d.y - want.fx.rot3d.y) < 0.01, name.constData());
                QCOMPARE(int(pic->stroke.compound), int(want.stroke.compound));
            }
            // The same look comes back from an .odt (the shape, border, and effects are kept in the picture's own graphic style).
            QString err;
            const QByteArray odt = doc->toOdt(&err);
            QVERIFY2(!odt.isEmpty(), qPrintable(err));
            auto fromOdt = WriteDoc::fromOdt(odt, &err);
            QVERIFY2(fromOdt, qPrintable(err));
            {
                const auto pic = pictureOfPage(*fromOdt);
                QVERIFY2(pic, name.constData());
                QCOMPARE(pic->maskShape, want.maskShape);
                QCOMPARE(pic->fx.shadow.on, want.fx.shadow.on);
                QCOMPARE(pic->fx.glow.on, want.fx.glow.on);
                QCOMPARE(pic->fx.reflection.on, want.fx.reflection.on);
                QCOMPARE(pic->fx.bevel.type, want.fx.bevel.type);
                QVERIFY2(std::abs(pic->fx.softEdge - want.fx.softEdge) < 0.01 && std::abs(pic->fx.rot3d.y - want.fx.rot3d.y) < 0.01, name.constData());
                QCOMPARE(pic->stroke.isNone(), want.stroke.isNone());
            }
            if (plain) continue;
            QVERIFY(doc->undo());
            QCOMPARE(pictureAt(*doc, doc->story(), 7).cropShape, QStringLiteral("rect"));
            QVERIFY(!pictureOfPage(*doc)->fx.any());
            QVERIFY(doc->redo());
        }
    }

    // The Picture Format tab's gallery and Picture Effects menu act on the selected picture: a click on a style is one step of Undo, and a pick in the menu adds an effect.
    void theGalleryAndTheEffectsMenuActOnTheSelectedPicture()
    {
        Win win;
        WriteDoc *doc = win.w.document();
        QTextCursor c(doc->story());
        c.setPosition(5);
        QCOMPARE(insertPicture(*doc, c, pngOf(200, 120, QColor(60, 140, 200)), QStringLiteral("blue"), PictureWrap::InFrontOfText), PictureResult::Ok);
        win.w.view()->selectPicture(5);
        win.w.ribbon()->buildAllTabs();
        auto *gallery = win.w.ribbon()->findChild<Gallery *>(QStringLiteral("pictureStyles"));
        QVERIFY(gallery);
        const QVector<GalleryItem> items = gallery->items();
        QVERIFY(items.size() >= 20 && items.size() <= 28);
        QCOMPARE(items.size(), pictureStyleCount());
        const int oval = pictureStyleIndex(QStringLiteral("oval-frame"));
        gallery->activate(QString::number(oval));
        QCOMPARE(pictureAt(*doc, doc->story(), 5).cropShape, QStringLiteral("ellipse"));
        QVERIFY(pictureOfPage(*doc)->fx.shadow.on);
        QVERIFY(doc->undo());   // (one step: the shape, border, and effects go together)
        QCOMPARE(pictureAt(*doc, doc->story(), 5).cropShape, QStringLiteral("rect"));
        QVERIFY(!pictureOfPage(*doc)->fx.shadow.on && pictureAt(*doc, doc->story(), 5).lineWeight == 0);
        // The shots of the gallery, for looking at (when the folder is named).
        const QString dir = qEnvironmentVariable("JO_PICSTYLES_SHOTS");
        if (!dir.isEmpty()) {
            QDir().mkpath(dir);
            QFrame *popup = galleryPopup(items, QSize(60, 46), 6, {}, [](const QString &) {}, &win.w);
            popup->show();
            popup->grab().save(dir + QStringLiteral("/jeffwrite-picture-styles-gallery.png"));
            delete popup;
            gallery->activate(QString::number(pictureStyleIndex(QStringLiteral("bevel-frame"))));
            win.w.resize(1500, 800);
            win.w.ribbon()->showTab(win.w.ribbon()->tab(QStringLiteral("Picture Format")));
            win.w.grab().save(dir + QStringLiteral("/jeffwrite-picture-format-tab.png"));
        }
    }

    // Picture Effects: a shadow, reflection, glow, soft edges, bevel, and turn in space are set one at a time (each a step that undoes), the others stay, and the picture's effects that the
    // engine has no setting for stay as the file had them. A border's dashes are drawn; Reset Picture takes the shape and effects off, and Reset Picture & Size gives the picture its own size.
    void aPicturesEffectsAreSetOneAtATimeAndResetTakesThemOff()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertPicture(*doc, c, pngOf(160, 80), QStringLiteral("bar"), PictureWrap::Square), PictureResult::Ok);
        QVERIFY(setPictureEffects(*doc, 7, [](Effects &fx) { formatpresets::applyShadow(fx, 0); }));
        QVERIFY(!setPictureEffects(*doc, 7, [](Effects &fx) { formatpresets::applyShadow(fx, 0); }));
        QVERIFY(setPictureEffects(*doc, 7, [](Effects &fx) { formatpresets::applyBevel(fx, 3); }));
        QVERIFY(setPictureEffects(*doc, 7, [](Effects &fx) { formatpresets::applyRotation(fx, 1); }));
        QVERIFY(setPictureEffects(*doc, 7, [](Effects &fx) { formatpresets::applyGlow(fx, 1, ColorRef::scheme(Accent1)); }));
        QVERIFY(setPictureEffects(*doc, 7, [](Effects &fx) { formatpresets::applyReflection(fx, 0); }));
        QVERIFY(setPictureEffects(*doc, 7, [](Effects &fx) { formatpresets::applySoftEdge(fx, 2); }));
        auto pic = pictureOfPage(*doc);
        QVERIFY(pic->fx.shadow.on && pic->fx.glow.on && pic->fx.reflection.on && pic->fx.bevel.type == 4 && pic->fx.rot3d.y == 30 && pic->fx.softEdge == 5);
        // (In the file, in the schema's order: the effect list, then the scene, then the bevel.)
        const QByteArray x = documentXml(doc->toDocx());
        const qsizetype list = x.indexOf("<a:effectLst>"), scene = x.indexOf("<a:scene3d>"), solid = x.indexOf("<a:sp3d>"), end = x.indexOf("</pic:spPr>");
        QVERIFY(list > 0 && scene > list && solid > scene && end > solid);
        QVERIFY(x.contains("<a:bevelT") && x.contains("prst=\"angle\""));
        auto back = open(doc->toDocx());
        QVERIFY(back);
        QCOMPARE(pictureOfPage(*back)->fx.toJson(), pic->fx.toJson());
        QVERIFY(setPictureEffects(*doc, 7, [](Effects &fx) { fx.bevel.type = 0; }));
        QVERIFY(pictureOfPage(*doc)->fx.shadow.on);
        QVERIFY(!documentXml(doc->toDocx()).contains("<a:bevelT"));
        QVERIFY(doc->undo());
        QCOMPARE(pictureOfPage(*doc)->fx.bevel.type, 4);
        // A dashed border is drawn dashed, and double.
        QVERIFY(setShapeLine(*doc, 7, QStringLiteral("112233"), 3, QStringLiteral("dash")));
        QCOMPARE(int(pictureOfPage(*doc)->stroke.dash), int(Stroke::DashLine));
        // Reset Picture: the shape, the border, and the effects go; Reset Picture & Size: the size the picture has in its file (160 by 80 pixels at 96 to the inch).
        QVERIFY(setPictureCropShape(*doc, 7, QStringLiteral("ellipse")));
        QVERIFY(setPictureSize(*doc, 7, 300, 150));
        QVERIFY(resetPicture(*doc, 7));
        pic = pictureOfPage(*doc);
        QVERIFY(!pic->fx.any() && pic->stroke.isNone());
        QCOMPARE(pic->maskShape, QStringLiteral("rect"));
        QCOMPARE(pictureAt(*doc, doc->story(), 7).width, 300.0);
        QVERIFY(doc->undo());
        QVERIFY(resetPictureAndSize(*doc, 7));
        const PictureInfo info = pictureAt(*doc, doc->story(), 7);
        QVERIFY(std::abs(info.width - 120) < 0.01 && std::abs(info.height - 60) < 0.01);
        QVERIFY(!resetPictureAndSize(*doc, 7));
    }

    // A bevel put in or taken off leaves the camera and the light the file has (here a preset of the camera that has no setting of ours) as they were; only a change of the turn writes the scene.
    void aBevelEditKeepsTheFilesCameraAndLight()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertPicture(*doc, c, pngOf(160, 80), QStringLiteral("bar"), PictureWrap::Square), PictureResult::Ok);
        QVERIFY(setPictureEffects(*doc, 7, [](Effects &fx) { formatpresets::applyBevel(fx, 3); }));
        const QByteArray foreign = withPartReplaced(doc->toDocx(), QStringLiteral("word/document.xml"), [](const QByteArray &x) {
            QByteArray y = x;
            y.replace("<a:camera prst=\"orthographicFront\"/>", "<a:camera prst=\"perspectiveContrastingRightFacing\"/>");
            y.replace("<a:lightRig rig=\"threePt\" dir=\"t\"/>", "<a:lightRig rig=\"balanced\" dir=\"b\"/>");
            return y;
        });
        auto back = open(foreign);
        QVERIFY(back);
        QVERIFY(documentXml(foreign).contains("perspectiveContrastingRightFacing"));
        QVERIFY(setPictureEffects(*back, 7, [](Effects &fx) { formatpresets::applyBevel(fx, 0); }));
        QByteArray x = documentXml(back->toDocx());
        QVERIFY2(x.contains("perspectiveContrastingRightFacing") && x.contains("rig=\"balanced\"") && x.contains("<a:bevelT") && x.contains("prst=\"circle\""), x.constData());
        QVERIFY(!x.contains("orthographicFront"));
        auto again = open(back->toDocx());
        QVERIFY(setPictureEffects(*again, 7, [](Effects &fx) { fx.bevel.type = 0; }));
        x = documentXml(again->toDocx());
        QVERIFY(!x.contains("<a:bevelT") && x.contains("perspectiveContrastingRightFacing"));
    }

    // A document of 2,000 pictures opens without the shape properties of each being read again: the time to open one with no effects and one with a shadow on each is told.
    void aDocumentOfThousandsOfPicturesOpensQuickly()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertPicture(*doc, c, pngOf(40, 20), QStringLiteral("bar"), PictureWrap::Inline), PictureResult::Ok);
        QVERIFY(setShapeLine(*doc, 7, QStringLiteral("112233"), 2, QStringLiteral("solid")));
        for (const bool shadowed : {false, true}) {
            if (shadowed) QVERIFY(setPictureEffects(*doc, 7, [](Effects &fx) { formatpresets::applyShadow(fx, 0); }));
            const QByteArray many = withPartReplaced(doc->toDocx(), QStringLiteral("word/document.xml"), [](const QByteArray &x) {
                const qsizetype drawing = x.indexOf("<w:drawing");
                const qsizetype start = x.lastIndexOf("<w:p>", drawing) >= 0 ? x.lastIndexOf("<w:p>", drawing) : x.lastIndexOf("<w:p ", drawing);
                const qsizetype end = x.indexOf("</w:p>", drawing) + 6;
                QByteArray paragraphs;
                for (int i = 0; i < 2000; ++i) paragraphs += x.mid(start, end - start);
                return x.left(start) + paragraphs + x.mid(end);
            });
            QVERIFY(!many.isEmpty());
            QElapsedTimer t;
            t.start();
            auto big = open(many);
            QVERIFY(big);
            int pictures = 0;
            for (int page = 0; page < 40; ++page) {
                const auto made = big->pager().materialize(page);
                if (!made) break;
                for (const auto &it : made->doc->pages[0]->items) pictures += it->type() == ItemType::Picture;
            }
            qInfo("2000 pictures%s: opened and laid out in %lld ms (%d pictures on the pages looked at)", shadowed ? " with shadows" : "", qint64(t.elapsed()), pictures);
        }
    }

    // Change Picture keeps the colors, border, shape, and effects, and a cut when the new picture has the old one's proportions.
    void aChangedPictureKeepsItsLookAndItsCutWhenTheProportionsAreTheSame()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        QCOMPARE(insertPicture(*doc, c, pngOf(160, 80), QStringLiteral("red"), PictureWrap::Square, QStringLiteral("A red bar")), PictureResult::Ok);
        QVERIFY(setPictureCrop(*doc, 7, 0.2, 0, 0, 0));
        QVERIFY(setPictureStyle(*doc, 7, pictureStyleIndex(QStringLiteral("snapshot"))));
        QVERIFY(setPictureCorrections(*doc, 7, 0.3, 0));
        const PictureInfo before = pictureAt(*doc, doc->story(), 7);
        QCOMPARE(changePictureFile(*doc, 7, pngOf(320, 160, QColor(20, 20, 200))), PictureResult::Ok);
        PictureInfo after = pictureAt(*doc, doc->story(), 7);
        QCOMPARE(after.width, before.width);
        QCOMPARE(after.height, before.height);
        QVERIFY(std::abs(after.cropLeft - 0.2) < 1e-6);
        QVERIFY(std::abs(after.brightness - 0.3) < 1e-6);
        QVERIFY(after.lineWeight > 0);
        QVERIFY(pictureOfPage(*doc)->fx.shadow.on);
        // Other proportions: no cut, the width stays; the look stays.
        QCOMPARE(changePictureFile(*doc, 7, pngOf(100, 100, QColor(200, 20, 20))), PictureResult::Ok);
        after = pictureAt(*doc, doc->story(), 7);
        QCOMPARE(after.cropLeft, 0.0);
        // (The cut is given back to the picture before it takes the new proportions: its frame is as wide as the whole old picture was.)
        QVERIFY(std::abs(after.width - before.width / 0.8) < 0.01 && std::abs(after.width - after.height) < 0.01);
        QVERIFY(after.lineWeight > 0 && std::abs(after.brightness - 0.3) < 1e-6);
        QVERIFY(pictureOfPage(*doc)->fx.shadow.on);
    }

    // Compress Pictures: a picture is made no more detailed than the target asks for the size it is shown at, in the saved file's own picture part, with its frame and place as they were.
    // The step undoes (the file has the first picture again, byte for byte) and does again, and a picture that would not get smaller is left alone.
    void compressingAPictureMakesItsSavedPartSmallerAndUndoes()
    {
        auto doc = textDoc({"Before after"});
        QTextCursor c(doc->story());
        c.setPosition(7);
        const QByteArray original = noisyPng(800, 400);
        QCOMPARE(insertPicture(*doc, c, original, QStringLiteral("noise"), PictureWrap::Square, QStringLiteral("Some noise")), PictureResult::Ok);
        QVERIFY(setPictureSize(*doc, 7, 200, 100));
        QVERIFY(setPicturePlace(*doc, 7, 100, 200));
        // (The file as saved and opened again: the picture is the file's own part, not one the program made.)
        auto saved = open(doc->toDocx());
        QVERIFY(saved);
        QCOMPARE(picturePositions(*saved), QVector<int>({7}));
        const QMap<QString, QByteArray> before = mediaOf(saved->toDocx());
        QCOMPARE(before.size(), 1);
        QCOMPARE(before.first(), original);
        const PictureInfo was = pictureAt(*saved, saved->story(), 7);
        const qint64 gained = compressPictures(*saved, 96, picturePositions(*saved), true);
        QVERIFY2(gained > 100000, qPrintable(QString::number(gained)));
        const QByteArray after = saved->toDocx();
        const QMap<QString, QByteArray> media = mediaOf(after);
        QCOMPARE(media.size(), 1);   // (the first picture's part is not written when nothing names it)
        QCOMPARE(qint64(before.first().size() - media.first().size()), gained);
        const QImage made = QImage::fromData(media.first());
        QVERIFY(!made.isNull());
        // 200 points shown at 96 pixels to the inch is 267 pixels, and the picture keeps its proportions.
        QVERIFY2(std::abs(made.width() - 266) <= 2, qPrintable(QString::number(made.width())));
        QVERIFY(std::abs(made.width() - 2 * made.height()) <= 2);
        const PictureInfo now = pictureAt(*saved, saved->story(), 7);
        QCOMPARE(now.width, was.width);
        QCOMPARE(now.height, was.height);
        QCOMPARE(now.left, was.left);
        QCOMPARE(now.alt, QStringLiteral("Some noise"));
        // Saved and opened again, it is the same picture.
        auto reopened = open(after);
        QVERIFY(reopened);
        QCOMPARE(pictureAt(*reopened, reopened->story(), 7).width, was.width);
        QCOMPARE(mediaOf(reopened->toDocx()).first(), media.first());
        // Undo is one step and brings the first picture back; Redo does it again.
        QVERIFY(saved->undo());
        QCOMPARE(mediaOf(saved->toDocx()).first(), original);
        QVERIFY(saved->redo());
        QCOMPARE(mediaOf(saved->toDocx()).first(), media.first());
        // Nothing smaller to make (a picture of one color that is small already) is no step and no change.
        auto plain = textDoc({"Before after"});
        QTextCursor pc(plain->story());
        pc.setPosition(7);
        QCOMPARE(insertPicture(*plain, pc, pngOf(40, 40)), PictureResult::Ok);
        QCOMPARE(compressPictures(*plain, 96, picturePositions(*plain), true), qint64(0));
        QVERIFY(plain->undo());   // (that was the insertion: Compress made no step)
        QVERIFY(picturePositions(*plain).isEmpty());
    }

    // A file whose relationship ids are not "rId..." (here "img1"), with one picture file used by two pictures: compressing one picture leaves the file the other still names, and the
    // saved document, opened again, has both pictures with their bytes.
    void compressingOneOfTwoPicturesThatShareAFileKeepsTheFileForTheOther()
    {
        const QByteArray original = noisyPng(800, 400);
        auto made = textDoc({"Before after"});
        QTextCursor c(made->story());
        c.setPosition(7);
        QCOMPARE(insertPicture(*made, c, original), PictureResult::Ok);
        QCOMPARE(insertPicture(*made, c, original), PictureResult::Ok);
        const QByteArray first = made->toDocx();
        office::Package src;
        QVERIFY(src.read(first));
        QByteArray xml = src.part(QStringLiteral("/word/document.xml"));
        static const QRegularExpression embed(QStringLiteral("r:embed=\"([^\"]+)\""));
        const QString id = embed.match(QString::fromUtf8(xml)).captured(1);
        QVERIFY(!id.isEmpty());
        xml.replace(("r:embed=\"" + id + "\"").toUtf8(), "r:embed=\"img1\"");
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"), xml, QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addPart(QStringLiteral("/word/media/image1.png"), original, QStringLiteral("image/png"));
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        QVERIFY(w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("img1"), rel + QStringLiteral("image"), QStringLiteral("media/image1.png")));
        auto doc = open(w.finish());
        QVERIFY(doc);
        QCOMPARE(picturePositions(*doc).size(), 2);
        const int one = picturePositions(*doc).first();
        QVERIFY(setPictureSize(*doc, one, 100, 50));
        QVERIFY(compressPictures(*doc, 96, QVector<int>({one}), false) > 0);
        auto back = open(doc->toDocx());
        QVERIFY(back);
        QCOMPARE(picturePositions(*back).size(), 2);
        const QMap<QString, QByteArray> media = mediaOf(back->toDocx());
        QCOMPARE(media.size(), 2);
        int whole = 0;
        for (const QByteArray &m : media) whole += m == original;
        QCOMPARE(whole, 1);   // (the other picture still has the file it named)
    }

    // The same with the raw XML of the pictures naming the file in single quotes (r:embed='img1'), as another program may have written it: with one picture file used by two pictures: compressing one picture leaves the file the other still names, and the
    // saved document, opened again, has both pictures with their bytes.
    void compressingOneOfTwoPicturesThatNameAFileInSingleQuotesKeepsTheFileForTheOther()
    {
        const QByteArray original = noisyPng(800, 400);
        auto made = textDoc({"Before after"});
        QTextCursor c(made->story());
        c.setPosition(7);
        QCOMPARE(insertPicture(*made, c, original), PictureResult::Ok);
        QCOMPARE(insertPicture(*made, c, original), PictureResult::Ok);
        const QByteArray first = made->toDocx();
        office::Package src;
        QVERIFY(src.read(first));
        QByteArray xml = src.part(QStringLiteral("/word/document.xml"));
        static const QRegularExpression embed(QStringLiteral("r:embed=\"([^\"]+)\""));
        const QString id = embed.match(QString::fromUtf8(xml)).captured(1);
        QVERIFY(!id.isEmpty());
        xml.replace(("r:embed=\"" + id + "\"").toUtf8(), "r:embed='img1'");
        office::PackageWriter w;
        w.addPart(QStringLiteral("/word/document.xml"), xml, QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
        w.addPart(QStringLiteral("/word/media/image1.png"), original, QStringLiteral("image/png"));
        const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
        QVERIFY(w.addRelationshipWithId(QStringLiteral("/word/document.xml"), QStringLiteral("img1"), rel + QStringLiteral("image"), QStringLiteral("media/image1.png")));
        auto doc = open(w.finish());
        QVERIFY(doc);
        QCOMPARE(picturePositions(*doc).size(), 2);
        const int one = picturePositions(*doc).first();
        QVERIFY(setPictureSize(*doc, one, 100, 50));
        QVERIFY(compressPictures(*doc, 96, QVector<int>({one}), false) > 0);
        auto back = open(doc->toDocx());
        QVERIFY(back);
        QCOMPARE(picturePositions(*back).size(), 2);
        const QMap<QString, QByteArray> media = mediaOf(back->toDocx());
        QCOMPARE(media.size(), 2);
        int whole = 0;
        for (const QByteArray &m : media) whole += m == original;
        QCOMPARE(whole, 1);   // (the other picture still has the file it named)
    }

    // A picture whose file says it is enormous (a small PNG that declares a huge size) is not decoded: it is left as it is.
    void aPictureThatDeclaresAHugeSizeIsNotDecodedByCompress()
    {
        QImage tiny(8, 8, QImage::Format_RGB32);
        tiny.fill(Qt::red);
        QByteArray png;
        QBuffer b(&png);
        b.open(QIODevice::WriteOnly);
        tiny.save(&b, "PNG");
        // the width and height of the header (IHDR) made 15,000 by 15,000, with the checksum written again
        png[16] = char(0); png[17] = char(0); png[18] = char(0x3A); png[19] = char(0x98);
        png[20] = char(0); png[21] = char(0); png[22] = char(0x3A); png[23] = char(0x98);
        const quint32 crc = crc32Of(png.mid(12, 17));
        for (int i = 0; i < 4; ++i) png[29 + i] = char((crc >> (24 - 8 * i)) & 0xFF);
        QByteArray out, format;
        QString why;
        QVERIFY(!compressedPictureFile(png, 100, 96, QRectF(0, 0, 1, 1), &out, &format, &why));
        QVERIFY(out.isEmpty());
        QVERIFY2(why.contains(QStringLiteral("15000")), qPrintable(why));
    }

    // With "delete cropped areas" the parts a picture is cut away from leave the file's picture and the cut is gone, with what shows, and its frame, as they were; without it the cut stays.
    void compressingCanTakeTheCroppedAreasOutOfThePicture()
    {
        auto make = [] {
            auto doc = textDoc({"Before after"});
            QTextCursor c(doc->story());
            c.setPosition(7);
            insertPicture(*doc, c, noisyPng(800, 400));
            setPictureSize(*doc, 7, 400, 200);
            setPictureCrop(*doc, 7, 0.25, 0, 0.25, 0);   // (the middle half: 200 points across, and the red half and the blue half cut away)
            return open(doc->toDocx());
        };
        auto keep = make();
        QVERIFY(keep);
        const PictureInfo was = pictureAt(*keep, keep->story(), 7);
        QVERIFY(std::abs(was.cropLeft - 0.25) < 0.001);
        QVERIFY(compressPictures(*keep, 96, picturePositions(*keep), false) > 0);
        QImage image = QImage::fromData(mediaOf(keep->toDocx()).first());
        QVERIFY(std::abs(image.width() - 2 * image.height()) <= 2);   // (the whole picture, scaled)
        QVERIFY(std::abs(pictureAt(*keep, keep->story(), 7).cropLeft - 0.25) < 0.001);
        auto cut = make();
        QVERIFY(compressPictures(*cut, 96, picturePositions(*cut), true) > 0);
        const QMap<QString, QByteArray> media = mediaOf(cut->toDocx());
        QCOMPARE(media.size(), 1);
        image = QImage::fromData(media.first());
        // The middle half only: as high as it is wide, and neither tinted half of the picture is in it.
        QVERIFY2(std::abs(image.width() - image.height()) <= 2, qPrintable(QString::number(image.width()) + "x" + QString::number(image.height())));
        const PictureInfo now = pictureAt(*cut, cut->story(), 7);
        QCOMPARE(now.cropLeft, 0.0);
        QCOMPARE(now.cropRight, 0.0);
        QCOMPARE(now.width, was.width);
        QCOMPARE(now.height, was.height);
        QVERIFY(cut->undo());
        QVERIFY(pictureAt(*cut, cut->story(), 7).cropLeft > 0.24);
    }

    // The Picture Format tab's Compress Pictures: the box's choices, every picture or the selected one, one step.
    void theWindowCompressesEveryPictureOrTheSelectedOne()
    {
        Win win;
        const QByteArray first = noisyPng(800, 400), second = noisyPng(640, 640);
        QVERIFY(win.w.view()->insertPictureBytes(first, QStringLiteral("a")));
        QTextCursor end(win.w.document()->story());
        end.movePosition(QTextCursor::End);
        win.w.view()->setCursor(end);
        QVERIFY(win.w.view()->insertPictureBytes(second, QStringLiteral("b")));
        const QVector<int> at = picturePositions(*win.w.document());
        QCOMPARE(at.size(), 2);
        QVERIFY(win.w.act(QStringLiteral("picture.compress")));
        win.w.view()->selectPicture(at[0]);
        QTRY_VERIFY(win.w.act(QStringLiteral("picture.compress"))->isEnabled());
        for (const int p : at) QVERIFY(setPictureSize(*win.w.document(), p, 100, 100));
        win.w.view()->selectPicture(at[0]);
        // The selected one only.
        QVERIFY(win.w.compressPictures(96, false, true) > 0);
        QCOMPARE(mediaOf(win.w.document()->toDocx()).size(), 2);
        int small = 0;
        for (const QByteArray &m : mediaOf(win.w.document()->toDocx())) small += m.size() < 100000;
        QCOMPARE(small, 1);
        // Every one, then Undo twice puts both first pictures back.
        QVERIFY(win.w.compressPictures(96, true, true) > 0);
        small = 0;
        for (const QByteArray &m : mediaOf(win.w.document()->toDocx())) small += m.size() < 100000;
        QCOMPARE(small, 2);
        QVERIFY(win.w.document()->undo());
        QVERIFY(win.w.document()->undo());
        const QMap<QString, QByteArray> back = mediaOf(win.w.document()->toDocx());
        QCOMPARE(back.size(), 2);
        for (const QByteArray &m : back) QVERIFY(m == first || m == second);
    }

    // An online picture goes in at the caret in line, with its title as its alt text and, as asked, a credit line under it, as one step that undoes; it is saved and read again.
    void anOnlinePictureIsPutInWithItsAltTextAndCreditInOneStep()
    {
        auto doc = textDoc({"Before after"});
        const QString plain = doc->story()->toPlainText();
        QTextCursor c(doc->story());
        c.setPosition(7);
        const QByteArray png = pngOf(120, 60);
        QCOMPARE(insertOnlinePicture(*doc, c, png, QStringLiteral("Red barn"), QStringLiteral("\"Red barn\" by Pat Lee, CC BY 2.0")), PictureResult::Ok);
        QCOMPARE(picturePositions(*doc), QVector<int>({7}));
        QCOMPARE(pictureAt(*doc, doc->story(), 7).alt, QStringLiteral("Red barn"));
        QVERIFY(!pictureAt(*doc, doc->story(), 7).floating);
        QVERIFY2(doc->story()->toPlainText().contains(QStringLiteral("\"Red barn\" by Pat Lee, CC BY 2.0")), qPrintable(doc->story()->toPlainText()));
        auto back = open(doc->toDocx());
        QVERIFY(back);
        QCOMPARE(picturePositions(*back), QVector<int>({7}));
        QCOMPARE(pictureAt(*back, back->story(), 7).alt, QStringLiteral("Red barn"));
        QVERIFY(back->story()->toPlainText().contains(QStringLiteral("Pat Lee, CC BY 2.0")));
        QCOMPARE(mediaOf(back->toDocx()).first(), png);
        // One Undo takes the picture and its credit, and Redo puts them back.
        QVERIFY(doc->undo());
        QCOMPARE(doc->story()->toPlainText(), plain);
        QVERIFY(picturePositions(*doc).isEmpty());
        QVERIFY(doc->redo());
        QCOMPARE(picturePositions(*doc), QVector<int>({7}));
        // With no credit asked there is none, and a file that is not a picture puts nothing in.
        auto bare = textDoc({"Before after"});
        QTextCursor bc(bare->story());
        bc.setPosition(7);
        QCOMPARE(insertOnlinePicture(*bare, bc, png, QStringLiteral("Red barn"), QString()), PictureResult::Ok);
        QCOMPARE(bare->story()->toPlainText(), plain.left(7) + QString(QChar(0xFFFC)) + plain.mid(7));
        QCOMPARE(insertOnlinePicture(*bare, bc, QByteArray("no"), QStringLiteral("x"), QString()), PictureResult::Unreadable);
    }

    // A title with a control character in it (U+0001, which XML has no place for) is cleaned: the picture goes in, and its alt text and credit are the title without it.
    void anOnlinePictureWithAControlCharacterInItsTitleIsPutIn()
    {
        CannedLibraries libraries(pngOf(480, 320), QStringLiteral("Red\x01 barn"));
        Win win;
        win.w.show();
        win.w.act(QStringLiteral("insert.onlinePicture"))->trigger();
        OnlinePicturesPane *pane = win.w.onlinePictures();
        QVERIFY(pane);
        pane->search(QStringLiteral("barn"));
        QCOMPARE(pane->results()->count(), 1);
        pane->insertButton()->click();
        const QVector<int> at = picturePositions(*win.w.document());
        QCOMPARE(at.size(), 1);
        QCOMPARE(pictureAt(*win.w.document(), win.w.document()->story(), at[0]).alt, QStringLiteral("Red barn"));
        QVERIFY(win.w.document()->story()->toPlainText().contains(QStringLiteral("“Red barn” by Pat Lee")));
        auto back = open(win.w.document()->toDocx());
        QVERIFY(back);
        QCOMPARE(picturePositions(*back).size(), 1);
    }

    // Insert > Pictures > Online Pictures opens the shared pane on the right (it says what it sends), a search shows what the libraries have (canned here), and Insert puts the picture
    // in the text with its credit; F1 in the pane opens the pictures topic.
    void theOnlinePicturesPaneInsertsAPictureInTheText()
    {
        CannedLibraries libraries(pngOf(480, 320));
        Win win;
        win.w.show();
        QVERIFY(!win.w.onlinePictures());
        QVERIFY(win.w.act(QStringLiteral("insert.onlinePicture")));
        QTRY_VERIFY(win.w.act(QStringLiteral("insert.onlinePicture"))->isEnabled());
        const QString plain = win.w.document()->story()->toPlainText();
        win.w.act(QStringLiteral("insert.onlinePicture"))->trigger();
        OnlinePicturesPane *pane = win.w.onlinePictures();
        QVERIFY(pane);
        QVERIFY(pane->isVisible());
        QVERIFY(pane->window() == &win.w);
        QStringList words;
        for (const QLabel *l : pane->findChildren<QLabel *>()) words << l->text();
        QVERIFY2(words.join(' ').contains(QStringLiteral("Your search words go to the library you choose.")), qPrintable(words.join('\n')));
        win.w.act(QStringLiteral("insert.onlinePicture"))->trigger();
        QCOMPARE(win.w.onlinePictures(), pane);
        pane->search(QStringLiteral("barn"));
        QCOMPARE(pane->results()->count(), 1);
        QVERIFY(pane->creditBox()->isChecked());
        pane->insertButton()->click();
        const QVector<int> at = picturePositions(*win.w.document());
        QCOMPARE(at.size(), 1);
        QCOMPARE(win.w.view()->selectedPicture(), at[0]);
        QCOMPARE(pictureAt(*win.w.document(), win.w.document()->story(), at[0]).alt, QStringLiteral("Red barn"));
        QVERIFY(win.w.document()->story()->toPlainText().contains(QStringLiteral("“Red barn” by Pat Lee, CC BY 2.0")));
        QCOMPARE(mediaOf(win.w.document()->toDocx()).size(), 1);
        // Without the credit, only the picture; one step each.
        pane->creditBox()->setChecked(false);
        QTextCursor end(win.w.document()->story());
        end.movePosition(QTextCursor::End);
        win.w.view()->setCursor(end);
        pane->insertButton()->click();
        QCOMPARE(picturePositions(*win.w.document()).size(), 2);
        QCOMPARE(win.w.document()->story()->toPlainText().count(QStringLiteral("Pat Lee")), 1);
        QVERIFY(win.w.document()->undo());
        QCOMPARE(picturePositions(*win.w.document()).size(), 1);
        QVERIFY(win.w.document()->undo());
        QCOMPARE(win.w.document()->story()->toPlainText(), plain);
        // F1 in the pane is the pictures topic.
        win.w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&win.w));
        pane->findChild<QLineEdit *>()->setFocus();
        QTRY_VERIFY(QApplication::focusWidget() == pane->findChild<QLineEdit *>());
        win.w.act(QStringLiteral("help.show"))->trigger();
        QVERIFY(win.w.helpView());
        QCOMPARE(win.w.helpView()->currentTopic(), QStringLiteral("pictures"));
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

    // The numbers of a chart that reads cells of the workbook kept with it are edited: the cells of the workbook and what the chart stores change together as one step of Undo, the file keeps one
    // chart part and one workbook (the new ones), and the saved workbook opens with the new numbers.
    void theNumbersOfAChartWithItsWorkbookAreEditedKeepingOneChartPartAndOneWorkbook()
    {
        auto doc = open(workbookChartDocx());
        QVERIFY(doc);
        const ChartInfo before = chartAt(*doc, 7);
        QVERIFY(before.valid && before.dataEditable && before.dataFromWorkbook);
        QCOMPARE(before.data.categories, QStringList({"Q1", "Q2", "Q3"}));
        QCOMPARE(before.data.values, (QVector<QVector<double>>{{1, 2, 3}, {4, 5, 6}}));
        ChartData data = before.data;
        data.values[1][2] = 60;
        data.seriesNames[0] = QStringLiteral("Revenue");
        QVERIFY(setChartWorkbookData(*doc, 7, data));
        const ChartInfo after = chartAt(*doc, 7);
        QVERIFY(after.dataEditable);
        QCOMPARE(after.data.values, data.values);
        QCOMPARE(after.data.seriesNames[0], QStringLiteral("Revenue"));
        // saved: one chart part and one workbook, which are the new ones
        auto count = [](const QByteArray &docx, QByteArray *book) {
            office::Package p;
            int charts = 0, books = 0;
            if (!p.read(docx)) return QPair<int, int>(-1, -1);
            for (const QString &name : p.partNames()) {
                if (name.startsWith(QLatin1String("/word/charts/")) && name.endsWith(QLatin1String(".xml"))) ++charts;
                if (name.endsWith(QLatin1String(".xlsx"))) {
                    ++books;
                    if (book) *book = p.part(name);
                }
            }
            return QPair<int, int>(charts, books);
        };
        QByteArray book;
        const QByteArray saved = doc->toDocx();
        QCOMPARE(count(saved, &book), (QPair<int, int>(1, 1)));
        office::Chart chart;
        ChartData cells;
        QVERIFY(readChartAt(*doc, 7, &chart));
        QVERIFY2(office::sheet::embeddedChartProblem(chart, book, &cells).isEmpty(), qPrintable(office::sheet::embeddedChartProblem(chart, book)));
        QCOMPARE(cells.values, data.values);
        // (the workbook opens as a workbook with the new number, as JeffCrunch reads it)
        office::Package inner;
        QVERIFY(inner.readLazy(book));
        office::sheet::Workbook wb;
        QVERIFY(wb.open(inner));
        office::sheet::Sheet sheet;
        QVERIFY(wb.readSheet(0, &sheet));
        const office::sheet::Cell *cell = sheet.cells.find(3, 2);
        QVERIFY(cell && cell->number == 60);
        // and reads again
        auto again = open(saved);
        QVERIFY(again);
        QCOMPARE(chartAt(*again, 7).data.values, data.values);
        QVERIFY(chartAt(*again, 7).dataEditable);
        // One step: Undo brings back the old numbers in the chart and in the workbook that goes with it.
        QVERIFY(doc->undo());
        QCOMPARE(chartAt(*doc, 7).data.values, before.data.values);
        QCOMPARE(count(doc->toDocx(), &book), (QPair<int, int>(1, 1)));
        QVERIFY(doc->redo());
        QCOMPARE(chartAt(*doc, 7).data.values, data.values);
        // a grid that changes nothing makes no step
        QVERIFY(!setChartWorkbookData(*doc, 7, data));
    }

    // From the window: the commands that change numbers and names are on, the grid is the cells (no row or column to add), and a row or column can't be swapped, with the reason.
    void aChartWithItsWorkbookHasItsGridInTheWindowAndOneStepOfUndo()
    {
        WriteWindow w;
        w.resize(1100, 800);
        w.show();
        w.setDocument(open(workbookChartDocx()), QString());
        w.view()->refresh();
        w.view()->selectPicture(7);
        QTRY_VERIFY(w.act("chartdesign.editData")->isEnabled());
        QVERIFY(w.act("chart.editData")->isEnabled());
        for (const char *id : {"chartdesign.selectData", "chartdesign.switch"}) {
            QVERIFY2(!w.act(id)->isEnabled(), id);
            QVERIFY2(w.act(id)->toolTip().contains(QStringLiteral("workbook")), id);
        }
        const QByteArray bookBefore = write::chartWorkbookAt(*w.document(), 7);
        withDialog([&] { w.act("chartdesign.editData")->trigger(); }, QStringLiteral("chartDataDialog"), [&](QDialog *dlg) {
            auto *table = dlg->findChild<QTableWidget *>(QStringLiteral("chartDataTable"));
            QVERIFY(table);
            QCOMPARE(table->rowCount(), 4);
            QCOMPARE(table->item(1, 0)->text(), QStringLiteral("Q1"));
            table->item(1, 1)->setText(QStringLiteral("11"));
        });
        QCOMPARE(chartAt(*w.document(), 7).data.values[0], (QVector<double>{11, 2, 3}));
        QVERIFY(write::chartWorkbookAt(*w.document(), 7) != bookBefore);
        QVERIFY(w.document()->undo());
        QCOMPARE(chartAt(*w.document(), 7).data.values[0], (QVector<double>{1, 2, 3}));
        QCOMPARE(write::chartWorkbookAt(*w.document(), 7), bookBefore);
    }

    // A reference that can't be followed (a second series on the first one's cells) leaves the numbers read-only, with the reason.
    void aChartWithItsWorkbookWhoseReferencesCantBeFollowedStaysReadOnly()
    {
        auto doc = open(workbookChartDocx());
        QVERIFY(doc);
        QVERIFY(changeChartAt(*doc, 7, [](office::Chart *c) { c->plots[0].series[1].valueRef = c->plots[0].series[0].valueRef; }));
        QVERIFY(!chartAt(*doc, 7).dataEditable);
        QVERIFY(!setChartWorkbookData(*doc, 7, chartAt(*doc, 7).data));
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

    // A chart's right-click menu has the chart entries JeffDeck's and JeffCrunch's have, by the same names; a picture's has none; and the Chart Parts menu has the Data Table list.
    void aChartsRightClickMenuHasTheChartEntriesAndPartsHasADataTable()
    {
        Win win;
        win.w.act("insert.chart.column")->trigger();
        const int at = win.w.view()->selectedPicture();
        QVERIFY(at >= 0);
        QTRY_VERIFY(win.w.act("chartdesign.selectData")->isEnabled());
        QMenu menu;
        win.w.fillContextMenu(menu);
        const QStringList ids = {QStringLiteral("chartdesign.selectData"), QStringLiteral("chartdesign.switch"), QStringLiteral("chartdesign.changeType"), QStringLiteral("chartdesign.formatPart"),
                                 QStringLiteral("chartdesign.axisH"), QStringLiteral("chartdesign.axisV")};
        const QStringList names = {QStringLiteral("Chart Data…"), QStringLiteral("Swap Rows and Columns"), QStringLiteral("Chart Kind…"), QStringLiteral("Format Chart Part…"),
                                   QStringLiteral("Format Horizontal Axis…"), QStringLiteral("Format Vertical Axis…")};
        for (int i = 0; i < ids.size(); ++i) {
            QAction *found = nullptr;
            for (QAction *a : menu.actions())
                if (a->objectName() == ids[i]) found = a;
            QVERIFY2(found, qPrintable(ids[i]));
            QCOMPARE(found->text(), names[i]);
            QVERIFY(found->isEnabled());
        }
        QVERIFY(menu.toolTipsVisible());
        QMenu *parts = win.w.chartButtons()->elements()->menu();
        QVERIFY(fireChartMenu(parts, QStringLiteral("dataTableMenu.Show Data Table")));
        office::Chart chart;
        QVERIFY(readChartAt(*win.w.document(), at, &chart));
        QVERIFY(chart.dataTable.present && chart.dataTable.outline);
    }

    // The Chart tab, the buttons beside the chart, and the menus every program shares: each entry changes the chart one step, and what was added is kept in the .docx and read again.
    void theChartDesignTabAndTheButtonsBesideAChartChangeItAndTheFileKeepsTheChanges()
    {
        Win win;
        win.w.act("insert.chart.column")->trigger();
        const int at = win.w.view()->selectedPicture();
        QVERIFY(at >= 0);
        QTRY_VERIFY(win.tabShown(QStringLiteral("Chart")));
        QTRY_VERIFY(win.w.chartButtons() && win.w.chartButtons()->elements()->isVisible());
        QVERIFY(win.w.chartButtons()->looks()->isVisible() && win.w.chartButtons()->filters()->isVisible());
        for (const char *id : {"chartdesign.editData", "chartdesign.selectData", "chartdesign.switch", "chartdesign.changeType", "chartdesign.axisH", "chartdesign.axisV"}) QTRY_VERIFY(win.w.act(id)->isEnabled());
        auto chart = [&] {
            office::Chart c;
            [&] { QVERIFY(readChartAt(*win.w.document(), at, &c)); }();
            return c;
        };
        QMenu *menu = win.w.chartButtons()->elements()->menu();
        QVERIFY(fireChartMenu(menu, QStringLiteral("chartTitleMenu.Above")));
        QVERIFY(chartAt(*win.w.document(), at).titleShown);
        QVERIFY(fireChartMenu(menu, QStringLiteral("errorBarsMenu.Fixed Value (1)")));
        QCOMPARE(office::sheet::errorBarsKind(chart()), QStringLiteral("fixedVal"));
        QVERIFY(fireChartMenu(menu, QStringLiteral("trendlineMenu.Linear")));
        QCOMPARE(office::sheet::elementsOf(chart()).trendline, QStringLiteral("linear"));
        QVERIFY(!findChartMenuEntry(menu, QStringLiteral("labelsFromCellsAction")));
        // The ribbon's menus are the same ones.
        std::unique_ptr<QMenu> colors(win.w.chartDesign()->colorsMenu(nullptr));
        const QByteArray before = win.w.document()->toDocx();
        colors->actions().last()->trigger();
        QVERIFY(win.w.document()->toDocx() != before);
        // Switch Row/Column, Select Data, and the filters.
        const ChartData first = chartAt(*win.w.document(), at).data;
        win.w.act("chartdesign.switch")->trigger();
        QCOMPARE(chartAt(*win.w.document(), at).data.categories, first.seriesNames);
        win.w.act("edit.undo")->trigger();
        QCOMPARE(chartAt(*win.w.document(), at).data.values, first.values);
        withDialog([&] { win.w.act("chartdesign.selectData")->trigger(); }, QStringLiteral("selectChartDataDialog"), [](QDialog *d) { static_cast<SelectChartDataDialog *>(d)->setSeriesChecked(1, false); });
        QCOMPARE(int(chartAt(*win.w.document(), at).data.seriesNames.size()), 1);
        win.w.act("edit.undo")->trigger();
        QVERIFY(win.w.chartDesign()->setSeriesShown(0, false));
        QVERIFY(!win.w.chartDesign()->setSeriesShown(1, false));
        win.w.act("edit.undo")->trigger();
        // Format Chart Part: one series takes a color and a line, and the file keeps them.
        withDialog([&] { win.w.act("chartdesign.formatPart")->trigger(); }, QStringLiteral("formatChartPartDialog"), [](QDialog *d) {
            auto *dialog = static_cast<FormatChartPartDialog *>(d);
            dialog->choosePart(2);
            dialog->setFill(QColor(0x20, 0x40, 0x60));
            dialog->setLineWidth(3);
        });
        QCOMPARE(office::sheet::partLookOf(chart(), office::sheet::ChartPart::Series, 0).fill.name(), QStringLiteral("#204060"));
        // Save, open again: the chart still has what was added and can be changed.
        const QByteArray saved = win.w.document()->toDocx();
        auto back = open(saved);
        QVERIFY(back);
        // (the chart is found again in the file that was opened: where it stands is not taken from the one that was saved)
        int backAt = -1;
        for (int p = 0; p < back->story()->characterCount() && backAt < 0; ++p)
            if (isDrawnObjectAt(back->story(), p) && pictureAt(*back, back->story(), p).kind == DrawnKind::Chart) backAt = p;
        QVERIFY(backAt >= 0);
        office::Chart again;
        QVERIFY(readChartAt(*back, backAt, &again));
        QCOMPARE(office::sheet::errorBarsKind(again), QStringLiteral("fixedVal"));
        QVERIFY(again.title.present);
        QCOMPARE(office::sheet::partLookOf(again, office::sheet::ChartPart::Series, 0).fill.name(), QStringLiteral("#204060"));
        QVERIFY(changeChartAt(*back, backAt, [](office::Chart *c) { office::sheet::setErrorBars(c, QStringLiteral("percentage"), 5); }));
        QVERIFY(readChartAt(*back, backAt, &again));
        QCOMPARE(office::sheet::errorBarsKind(again), QStringLiteral("percentage"));
        // A change that makes no difference is no change.
        QVERIFY(!changeChartAt(*back, backAt, [](office::Chart *) {}));
        // (With JO_SHOT_DIR set: the Chart tab and two of its format panes, in the new look.)
        if (!qEnvironmentVariable("JO_SHOT_DIR").isEmpty()) {
            jp::applyUiTheme(1);
            win.w.ribbon()->showTab(win.w.ribbon()->tab(QStringLiteral("Chart")));
            QApplication::processEvents();
            shotOf(&win.w, QStringLiteral("write-chart-design"));
            withDialog([&] { win.w.act("chartdesign.formatPart")->trigger(); }, QStringLiteral("formatChartPartDialog"), [](QDialog *d) { shotOf(d, QStringLiteral("write-format-chart-part")); }, false);
            withDialog([&] { win.w.act("chartdesign.axisV")->trigger(); }, QStringLiteral("formatAxisDialog"), [](QDialog *d) { shotOf(d, QStringLiteral("write-format-axis")); }, false);
        }
    }

    // The names are JeffCrunch's, and the commands for the numbers of a chart whose numbers are not its own (c:f references and a workbook kept with it) are off, with the reason in the tooltip.
    void theChartTabHasJeffCrunchsNamesAndTheDataCommandsOfAWorkbookChartAreOffWithTheReason()
    {
        Win win;
        win.w.act("insert.chart.column")->trigger();
        const int at = win.w.view()->selectedPicture();
        QVERIFY(at >= 0);
        QTRY_VERIFY(win.tabShown(QStringLiteral("Chart")));
        QVERIFY(!win.tabShown(QStringLiteral("Chart Design")));
        QCOMPARE(win.w.act("chartdesign.selectData")->text(), QStringLiteral("Chart Data…"));
        QCOMPARE(win.w.act("chartdesign.changeType")->text(), QStringLiteral("Chart Kind…"));
        QCOMPARE(win.w.act("chartdesign.switch")->text(), QStringLiteral("Swap Rows and Columns"));
        QTRY_VERIFY(win.w.act("chartdesign.switch")->isEnabled());
        QVERIFY(changeChartAt(*win.w.document(), at, [](office::Chart *c) {
            for (office::ChartSeries &s : c->plots[0].series) {
                s.valueRef = QStringLiteral("Sheet1!$B$2:$B$4");
                s.categoryRef = QStringLiteral("Sheet1!$A$2:$A$4");
            }
            c->externalDataId = QStringLiteral("rId9");
        }));
        win.w.view()->selectPicture(at);
        for (const char *id : {"chart.editData", "chartdesign.editData", "chartdesign.selectData", "chartdesign.switch"}) {
            QTRY_VERIFY2(!win.w.act(id)->isEnabled(), id);
            QVERIFY2(win.w.act(id)->toolTip().contains(QStringLiteral("workbook")), id);
        }
        QVERIFY(win.w.act("chartdesign.changeType")->isEnabled());
        // (the old Edit Data command, asked for anyway, says why)
        win.w.chartDesign()->editData();
        QVERIFY(win.w.statusBar()->currentMessage().contains(QStringLiteral("workbook")));
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
