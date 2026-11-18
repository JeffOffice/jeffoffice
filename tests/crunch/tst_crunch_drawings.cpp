// Tests of the charts, pictures, and shapes over the cells in JeffCrunch's window: they are shown where their anchors put them, a chart is drawn from the cells it reads and again when one
// changes (typed, or worked out), and objects and charts move when rows are inserted or deleted, undone and redone. Every workbook is built here.
#include "../sheet/drawinghelpers.h"
#include "../office/diagramsample.h"
#include "../stablesaves.h"
#include "savedok.h"
#include "editwindow.h"

#include "core/fonts.h"
#include "core/printjob.h"
#include "core/sheetobjects.h"
#include "app/theme.h"
#include "io/zip.h"
#include "sheet/drawingsave.h"
#include "office/chart.h"
#include "office/diagram.h"
#include "office/package.h"
#include "office/theme.h"
#include "render/diagramdrawing.h"
#include "office/xml.h"

#include <QDir>
#include <QMenu>
#include <QRegularExpression>
#include <QSet>
#include <QProcess>
#include <QRandomGenerator>
#include <QToolButton>
#include <QThreadPool>
#include <QFileInfo>

using namespace crunchtest;
using namespace drawtest;

namespace {

// Sheet "Sheet1": A1:C4 hold the labels and numbers of dataSheet() (B2:B4 are 10, 20, 30; C2:C4 are 5, 10, 15) with B4 a formula, =B2*3, stored as 30 (which the calculation makes 30 again, and
// 60 when B2 is 20). The drawing: a chart of B2:B4 by A2:A4 over E2:K13, a red picture over A7:B9, a blue shape over A11:B12, a group of two shapes over D15:F16, and a line.
QByteArray drawingBook()
{
    auto text = [](const QString &ref, const QString &t) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, t); };
    const QString rows = QStringLiteral("<row r=\"1\">") + text("A1", "Region") + text("B1", "East") + text("C1", "West") + QStringLiteral("</row>")
                         + QStringLiteral("<row r=\"2\">") + text("A2", "Q1") + QStringLiteral("<c r=\"B2\"><v>10</v></c><c r=\"C2\"><v>5</v></c></row>")
                         + QStringLiteral("<row r=\"3\">") + text("A3", "Q2") + QStringLiteral("<c r=\"B3\"><v>20</v></c><c r=\"C3\"><v>10</v></c></row>")
                         + QStringLiteral("<row r=\"4\">") + text("A4", "Q3") + QStringLiteral("<c r=\"B4\"><f>B2*3</f><v>30</v></c><c r=\"C4\"><v>15</v></c></row>");
    const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
    const QString anchors = twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales"))) + twoCell(0, 6, 2, 9, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("Logo")))
                            + twoCell(0, 10, 2, 12, shapeElement(4, QStringLiteral("Box"), QStringLiteral("rect"), QString(), QStringLiteral("3366CC")), QStringLiteral("oneCell"))
                            + twoCell(3, 14, 6, 16, groupElement(5, shapeElement(6, QStringLiteral("In1"), QStringLiteral("rect"), QString(), QStringLiteral("00AA00"), 952500, 476250), 952500, 476250))
                            + twoCell(0, 17, 4, 17, connectorElement(7));
    return bookWithDrawing({worksheet(rows)}, drawingPart(anchors),
                           {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series)), imageTarget(QStringLiteral("rIdP"), 1, pngBytes(8, 4, Qt::red))});
}

// A diagram's frame, as the spreadsheet program writes one: it names the four parts by the links of the drawing, and the frame's own size is 0 by 0 (the anchor says how large it is).
QString diagramFrame(int id, const QString &name, const QString &dataId = QStringLiteral("rIdDm"))
{
    return QStringLiteral("<xdr:graphicFrame macro=\"\"><xdr:nvGraphicFramePr><xdr:cNvPr id=\"%1\" name=\"%2\" descr=\"A process\"/><xdr:cNvGraphicFramePr/></xdr:nvGraphicFramePr>"
                          "<xdr:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/></xdr:xfrm><a:graphic><a:graphicData uri=\"%3\"><dgm:relIds xmlns:dgm=\"%3\" r:dm=\"%4\" r:lo=\"rIdLo\" r:qs=\"rIdQs\" r:cs=\"rIdCs\"/>"
                          "</a:graphicData></a:graphic></xdr:graphicFrame>")
        .arg(id).arg(name, diagramtest::dgmNs(), dataId);
}

// The five parts of a diagram, named by the links of the drawing, the numbered one `n` (the links are suffixed with it when n > 1).
QVector<DrawingTarget> diagramTargets(bool withDrawing = true, const QByteArray &drawingXml = diagramtest::diagramDrawingXml(), int n = 1)
{
    using namespace diagramtest;
    const QString base = QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.");
    const QString suffix = n > 1 ? QString::number(n) : QString();
    auto part = [&](const QString &id, const QString &kind, const QString &file, const QByteArray &bytes, const QString &type) {
        return DrawingTarget{id + suffix, QStringLiteral("xl/diagrams/%1%2.xml").arg(file).arg(n), bytes, type, kind};
    };
    QVector<DrawingTarget> out;
    out << part(QStringLiteral("rIdDm"), QStringLiteral("diagramData"), QStringLiteral("data"), diagramDataXml(withDrawing ? QStringLiteral("rIdDrw") + suffix : QString()), base + QStringLiteral("diagramData+xml"));
    out << part(QStringLiteral("rIdLo"), QStringLiteral("diagramLayout"), QStringLiteral("layout"), diagramOtherXml("layoutDef"), base + QStringLiteral("diagramLayout+xml"));
    out << part(QStringLiteral("rIdQs"), QStringLiteral("diagramQuickStyle"), QStringLiteral("quickStyle"), diagramOtherXml("styleDef"), base + QStringLiteral("diagramStyle+xml"));
    out << part(QStringLiteral("rIdCs"), QStringLiteral("diagramColors"), QStringLiteral("colors"), diagramOtherXml("colorsDef"), base + QStringLiteral("diagramColors+xml"));
    if (withDrawing) out << part(QStringLiteral("rIdDrw"), QStringLiteral("diagramDrawing"), QStringLiteral("drawing"), drawingXml, QStringLiteral("application/vnd.ms-office.drawingml.diagramDrawing+xml"));
    return out;
}

// A workbook with a diagram over A7:E14 (the frame of 0 by 0, as a real file has it; its drawing is 4 by 1.5 inches in its own space).
QByteArray diagramBook(bool withDrawing = true, const QByteArray &drawingXml = diagramtest::diagramDrawingXml())
{
    return bookWithDrawing({dataSheet()}, drawingPart(twoCell(0, 6, 5, 14, diagramFrame(2, QStringLiteral("Diagram 1")))), diagramTargets(withDrawing, drawingXml));
}

// A PNG that says it is 100,000 by 100,000 pixels and holds no pixels (only its header and its end).
QByteArray hugePng()
{
    auto crc = [](const QByteArray &data) {
        quint32 c = 0xFFFFFFFFu;
        for (const char ch : data) {
            c ^= quint8(ch);
            for (int k = 0; k < 8; ++k) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1u)));
        }
        return c ^ 0xFFFFFFFFu;
    };
    auto be32 = [](quint32 v) {
        QByteArray b;
        for (int shift = 24; shift >= 0; shift -= 8) b.append(char((v >> shift) & 0xFF));
        return b;
    };
    auto chunk = [&](const char *type, const QByteArray &body) {
        const QByteArray typed = QByteArray(type) + body;
        return be32(quint32(body.size())) + typed + be32(crc(typed));
    };
    QByteArray ihdr = be32(100000) + be32(100000);
    ihdr.append(char(8));
    ihdr.append(char(2));
    ihdr.append(char(0));
    ihdr.append(char(0));
    ihdr.append(char(0));
    return QByteArray("\x89PNG\r\n\x1a\n", 8) + chunk("IHDR", ihdr) + chunk("IEND", QByteArray());
}

// The data sheet, then a chart sheet "Chart1": a page that holds one chart of Sheet1!B2:B4 and no cells. The chart part has no fill of its own (the page behind it is the sheet's).
QByteArray bookWithChartSheet(const QString &chartSheetPrinting = QString())
{
    const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
    QByteArray chart = chartPart(QStringLiteral("barChart"), series);
    chart.replace("</c:chart>", "</c:chart><c:spPr><a:noFill/><a:ln><a:noFill/></a:ln></c:spPr>");
    QMap<QString, QByteArray> parts;
    jp::readZip(buildXlsx({dataSheet()}), parts);
    const QString relNs = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships");
    parts[QStringLiteral("xl/workbook.xml")].replace("</sheets>", "<sheet name=\"Chart1\" sheetId=\"9\" r:id=\"rIdCS1\"/></sheets>");
    parts[QStringLiteral("xl/_rels/workbook.xml.rels")].replace("</Relationships>", QStringLiteral("<Relationship Id=\"rIdCS1\" Type=\"%1/chartsheet\" Target=\"chartsheets/sheet1.xml\"/></Relationships>").arg(relNs).toUtf8());
    parts[QStringLiteral("[Content_Types].xml")].replace("</Types>", "<Override PartName=\"/xl/chartsheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.chartsheet+xml\"/>"
                                                                      "<Override PartName=\"/xl/drawings/drawing1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawing+xml\"/>"
                                                                      "<Override PartName=\"/xl/charts/chart1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawingml.chart+xml\"/></Types>");
    parts[QStringLiteral("xl/chartsheets/sheet1.xml")] = QStringLiteral("<?xml version=\"1.0\"?><chartsheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"%1\">%2<drawing r:id=\"rIdD\"/></chartsheet>").arg(relNs, chartSheetPrinting).toUtf8();
    parts[QStringLiteral("xl/chartsheets/_rels/sheet1.xml.rels")] = QStringLiteral("<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                                                                                  "<Relationship Id=\"rIdD\" Type=\"%1/drawing\" Target=\"../drawings/drawing1.xml\"/></Relationships>").arg(relNs).toUtf8();
    parts[QStringLiteral("xl/drawings/drawing1.xml")] = drawingPart(absolute(0, 0, 9525 * 640, 9525 * 480, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Page"))));
    parts[QStringLiteral("xl/drawings/_rels/drawing1.xml.rels")] = QStringLiteral("<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                                                                                  "<Relationship Id=\"rIdC\" Type=\"%1/chart\" Target=\"../charts/chart1.xml\"/></Relationships>").arg(relNs).toUtf8();
    parts[QStringLiteral("xl/charts/chart1.xml")] = chart;
    jp::ZipWriter z;
    for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}


// A chart on "Sheet1" that reads "Sheet2" (B2:B4 by A2:A4 there: 10, 20, 30), with "Sheet1" a sheet of one cell.
QByteArray chartOfTheOtherSheetBook()
{
    const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet2!$B$1"), QStringLiteral("Sheet2!$A$2:$A$4"), QStringLiteral("Sheet2!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
    const QString anchors = twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales")));
    return bookWithDrawing({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>")), dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series))});
}

// drawingBook() with its sheet read from right to left.
QByteArray rightToLeftDrawingBook()
{
    QMap<QString, QByteArray> parts;
    jp::readZip(drawingBook(), parts);
    parts[QStringLiteral("xl/worksheets/sheet1.xml")].replace("<sheetData>", "<sheetViews><sheetView rightToLeft=\"1\" workbookViewId=\"0\"/></sheetViews><sheetData>");
    jp::ZipWriter z;
    for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

// A chart with the style and colors parts Excel-era files carry, and a picture beside it.
QByteArray chartWithStylePartsBook(QByteArray *chartXml, QByteArray *styleXml, QByteArray *colorsXml, int sheets = 1)
{
    const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
    *chartXml = chartPart(QStringLiteral("barChart"), series);
    *styleXml = "<?xml version=\"1.0\"?><cs:chartStyle xmlns:cs=\"http://schemas.microsoft.com/office/drawing/2012/chartStyle\" id=\"201\"><cs:axisTitle/></cs:chartStyle>";
    *colorsXml = "<?xml version=\"1.0\"?><cs:colorStyle xmlns:cs=\"http://schemas.microsoft.com/office/drawing/2012/chartStyle\" meth=\"cycle\" id=\"10\"/>";
    const QString anchors = twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales"))) + twoCell(0, 6, 2, 9, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("Logo")));
    QVector<QByteArray> sheetList;
    for (int i = 0; i < sheets; ++i) sheetList << dataSheet();
    const QByteArray book = bookWithDrawing(sheetList, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, *chartXml), imageTarget(QStringLiteral("rIdP"), 1, pngBytes(8, 4, Qt::red))});
    const QByteArray rels = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                            "<Relationship Id=\"rId1\" Type=\"http://schemas.microsoft.com/office/2011/relationships/chartStyle\" Target=\"style1.xml\"/>"
                            "<Relationship Id=\"rId2\" Type=\"http://schemas.microsoft.com/office/2011/relationships/chartColorStyle\" Target=\"colors1.xml\"/></Relationships>";
    return sheettest::withParts(book, {{QStringLiteral("xl/charts/style1.xml"), *styleXml, QStringLiteral("application/vnd.ms-office.chartstyle+xml"), -1, QString(), QString(), QString()},
                                       {QStringLiteral("xl/charts/colors1.xml"), *colorsXml, QStringLiteral("application/vnd.ms-office.chartcolorstyle+xml"), -1, QString(), QString(), QString()},
                                       {QStringLiteral("xl/charts/_rels/chart1.xml.rels"), rels, QString(), -1, QString(), QString(), QString()}});
}

// The part a saved file's drawing reaches by a relationship of this type (the last word), and its bytes; the parts its own relationships reach, by type.
struct Reached {
    QString part;
    QByteArray bytes;
    QMap<QString, QByteArray> children;   // by the last word of the type
};
QVector<Reached> reachedBy(const QString &path, const QString &typeWord)
{
    QVector<Reached> out;
    office::Package p;
    QString why;
    if (!p.readLazyFile(path, &why)) return out;
    for (const office::Relationship &r : p.relationships(QStringLiteral("/xl/drawings/drawing1.xml"))) {
        if (r.external || !r.type.endsWith(typeWord) || !p.hasPart(r.target)) continue;
        Reached x;
        x.part = r.target;
        x.bytes = p.part(r.target);
        for (const office::Relationship &c : p.relationships(r.target))
            if (!c.external && p.hasPart(c.target)) x.children.insert(c.type.mid(c.type.lastIndexOf(QLatin1Char('/')) + 1), p.part(c.target));
        out << x;
    }
    return out;
}
QStringList savedNames(const QString &path)
{
    office::Package p;
    QString why;
    QStringList names;
    if (!p.readLazyFile(path, &why)) return names;
    QDomDocument drawing;
    if (!office::parseXml(p.part(QStringLiteral("/xl/drawings/drawing1.xml")), &drawing)) return names;
    const QDomNodeList all = drawing.elementsByTagName(QStringLiteral("cNvPr"));
    for (int i = 0; i < all.size(); ++i) names << all.at(i).toElement().attribute(QStringLiteral("name"));
    return names;
}
bool saveWindow(EditWindow &e)
{
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    return e.win.w.save() && savedOk(saved);
}


// The parts of a saved file that nothing relates to (media, charts, and drawings other than a sheet's): none are left behind by an object that was taken away and brought back.
QStringList orphanParts(const QString &path)
{
    QMap<QString, QByteArray> parts;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) jp::readZip(f.readAll(), parts);
    QSet<QString> referenced;
    for (auto it = parts.begin(); it != parts.end(); ++it) {
        if (!it.key().endsWith(QLatin1String(".rels"))) continue;
        const QString dir = it.key().left(it.key().indexOf(QLatin1String("_rels/")));
        static const QRegularExpression target(QStringLiteral("Target=\"([^\"]*)\""));
        auto m = target.globalMatch(QString::fromUtf8(it.value()));
        while (m.hasNext()) {
            QString t = m.next().captured(1);
            if (t.startsWith(QLatin1Char('/'))) referenced.insert(t.mid(1));
            else referenced.insert(QDir::cleanPath(dir + t));
        }
    }
    QStringList out;
    for (auto it = parts.begin(); it != parts.end(); ++it)
        if ((it.key().startsWith(QLatin1String("xl/media/")) || it.key().startsWith(QLatin1String("xl/charts/")) || it.key().startsWith(QLatin1String("xl/drawings/"))) && !it.key().contains(QLatin1String("_rels/")) && !referenced.contains(it.key()))
            out << it.key();
    return out;
}

// Whether every relationship of every part of a saved file names a part that is there (or leaves the package).
bool relationshipsAllResolve(const QString &path, QString *bad)
{
    office::Package p;
    QString why;
    if (!p.readLazyFile(path, &why)) return false;
    for (const QString &name : p.partNames())
        for (const office::Relationship &r : p.relationships(name))
            if (!r.external && !p.hasPart(r.target)) {
                if (bad) *bad = name + QStringLiteral(" -> ") + r.target;
                return false;
            }
    return true;
}

// The content types hold an override with exactly this part name (not a name that merely contains it).
bool hasExactOverride(const QMap<QString, QByteArray> &parts, const QString &partName)
{
    static const QRegularExpression part(QStringLiteral("<Override\\b[^>]*\\bPartName=\"([^\"]*)\""));
    auto m = part.globalMatch(QString::fromUtf8(parts.value(QStringLiteral("[Content_Types].xml"))));
    while (m.hasNext())
        if (m.next().captured(1).compare(partName, Qt::CaseInsensitive) == 0) return true;
    return false;
}

} // namespace

class CrunchDrawingTests : public QObject {
    Q_OBJECT

    // The picture of the viewport, and where an object is in it.
    static QImage shot(EditWindow &e)
    {
        e.view()->viewport()->repaint();
        return e.view()->viewport()->grab().toImage();
    }
    // The color at a logical point of a grabbed picture (a picture of a screen with a device pixel ratio above 1 has more pixels than the widget has points).
    static QRgb at(const QImage &img, int x, int y)
    {
        const qreal r = img.devicePixelRatio();
        return img.pixel(qRound(x * r), qRound(y * r));
    }
    static QRgb at(const QImage &img, const QPoint &p) { return at(img, p.x(), p.y()); }
    static bool shown(EditWindow &e, int count, int sheet = 0)
    {
        crunch::SheetObjects *o = e.win.w.objects();
        return o && o->shownSheet() == sheet && o->layer()->count() == count;
    }
    // The parts of a saved file, by name (the names as the zip has them, no leading slash): what the package holds, [Content_Types].xml and the relationships among them.
    static QMap<QString, QByteArray> savedParts(const QString &path)
    {
        QMap<QString, QByteArray> parts;
        QFile f(path);
        if (f.open(QIODevice::ReadOnly)) jp::readZip(f.readAll(), parts);
        return parts;
    }
    // Whether the relationships part holds a relationship with this id, and whether the content types hold an override for this part.
    static bool hasRelationship(const QMap<QString, QByteArray> &parts, const QString &relsPart, const QString &id)
    {
        return parts.value(relsPart).contains(QStringLiteral("Id=\"%1\"").arg(id).toUtf8());
    }
    static bool hasOverride(const QMap<QString, QByteArray> &parts, const QString &partName)
    {
        return parts.value(QStringLiteral("[Content_Types].xml")).contains(QStringLiteral("PartName=\"%1\"").arg(partName).toUtf8());
    }
    // The anchor the saved drawing has for the object of this name: its first row and its last row (a one-cell anchor has the one), and the anchor's editAs.
    struct SavedAnchor {
        bool found = false;
        QString editAs, kind;
        int fromRow = -1, toRow = -1;
    };
    static SavedAnchor savedAnchor(const QString &path, const QString &name)
    {
        SavedAnchor out;
        office::Package p;
        QString why;
        if (!p.readLazyFile(path, &why)) return out;
        QDomDocument doc;
        if (!office::parseXml(p.part(QStringLiteral("/xl/drawings/drawing1.xml")), &doc)) return out;
        const QDomNodeList all = doc.elementsByTagName(QStringLiteral("cNvPr"));
        for (int i = 0; i < all.size(); ++i) {
            if (all.at(i).toElement().attribute(QStringLiteral("name")) != name) continue;
            QDomElement anchor = all.at(i).parentNode().toElement();
            while (!anchor.isNull() && !anchor.localName().endsWith(QLatin1String("Anchor"))) anchor = anchor.parentNode().toElement();
            if (anchor.isNull()) return out;
            out.found = true;
            out.kind = anchor.localName();
            out.editAs = anchor.attribute(QStringLiteral("editAs"));
            auto rowOf = [&](const char *tag) {
                for (QDomElement c = anchor.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
                    if (c.localName() == QLatin1String(tag)) return c.firstChildElement(QStringLiteral("row")).text().toInt();
                return -1;
            };
            out.fromRow = rowOf("from");
            out.toRow = out.kind == QLatin1String("twoCellAnchor") ? rowOf("to") : out.fromRow;
            return out;
        }
        return out;
    }

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // The objects of the sheet in view are made between events after it is shown, put where their anchors say, over the cells.
    void objectsAreShownOverTheCells()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        const crunch::DrawingCounts c = o->counts(0);
        QCOMPARE(c.charts, 1);
        QCOMPARE(c.pictures, 1);
        QCOMPARE(c.shapes, 1);
        QCOMPARE(c.groups, 1);
        QCOMPARE(c.connectors, 1);
        QCOMPARE(c.shown, 5);
        QCOMPARE(c.leftOut, 0);
        QCOMPARE(o->drawingsParsedApart(), 0);   // (a small drawing is read where it is)
        o->layer()->prepareAll();
        const QImage img = shot(e);
        // the picture: red where the anchor puts it (A7 to B9, rows 6 to 8), the shape blue (A11 to B12)
        const grid::GridLayout &L = e.view()->sheet().layout();
        const QRect pic = L.rangeRect(6, 0, 8, 1), box = L.rangeRect(10, 0, 11, 1), chart = L.rangeRect(1, 4, 12, 10), group = L.rangeRect(14, 3, 15, 5);
        QCOMPARE(at(img, pic.center()), QColor(Qt::red).rgb());
        QCOMPARE(at(img, box.left() + 6, box.top() + 6), QColor(0x33, 0x66, 0xCC).rgb());
        QVERIFY(at(img, box.left() - 2, box.center().y()) != QColor(0x33, 0x66, 0xCC).rgb());
        QCOMPARE(at(img, group.left() + 6, group.top() + 6), QColor(0x00, 0xAA, 0x00).rgb());
        // the chart: a picture with more than the grid's white and gray in it, inside its anchor and not outside
        QSet<QRgb> colors;
        for (int y = chart.top(); y < chart.bottom(); y += 3)
            for (int x = chart.left(); x < chart.right(); x += 3) colors.insert(at(img, x, y));
        QVERIFY2(colors.size() >= 4, qPrintable(QString::number(colors.size())));
        QCOMPARE(at(img, chart.right() + 4, chart.center().y()), at(img, chart.right() + 8, chart.center().y()));   // (the plain cells beside it)
        // a click on the picture picks it; a click on a cell lets it go and moves the cursor
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, pic.center());
        QCOMPARE(o->layer()->selectedId(), 1);
        const QRect cell = L.cellRect(2, 3);
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, cell.center());
        QCOMPARE(o->layer()->selectedId(), -1);
        QCOMPARE(e.selected(), QStringLiteral("D3"));
    }

    // The chart is drawn from the cells: what the workbook holds, not what the chart part stored (the part says 10, 20, 30), and again when a cell it reads is typed over or a formula
    // it reads is worked out again.
    void aChartFollowsItsCells()
    {
        EditWindow e;
        // the part stores other numbers than the cells have
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        QCOMPARE(o->chartsDrawn() > 0, true);
        auto values = [&] { return o->chartNow(0, 0).plots.value(0).series.value(0).values; };
        QCOMPARE(values(), (QVector<double>{10, 20, 30}));
        QCOMPARE(o->chartReads(0, 0).size(), 3);   // (name, categories, values)
        const QImage before = shot(e).copy(e.view()->sheet().layout().rangeRect(1, 4, 12, 10));
        // a typed cell
        e.select(QStringLiteral("B3"));
        e.enter(QStringLiteral("55"));
        QVERIFY(e.settle());
        QCOMPARE(values(), (QVector<double>{10, 55, 30}));
        QTRY_COMPARE(o->layer()->pendingRenders(), 0);
        o->layer()->prepareAll();
        const QImage typed = shot(e).copy(e.view()->sheet().layout().rangeRect(1, 4, 12, 10));
        QVERIFY(typed != before);
        // a cell a formula reads: the formula's result changes, and the chart with it
        const qint64 drawn = o->chartsDrawn();
        e.select(QStringLiteral("B2"));
        e.enter(QStringLiteral("20"));
        QVERIFY(e.settle());
        QCOMPARE(values(), (QVector<double>{20, 55, 60}));
        QTRY_COMPARE(o->layer()->pendingRenders(), 0);
        QVERIFY(o->chartsDrawn() > drawn);
        // a cell the chart does not read changes nothing of it
        o->layer()->prepareAll();
        const qint64 steady = o->chartsDrawn();
        e.select(QStringLiteral("C3"));
        e.enter(QStringLiteral("77"));
        QVERIFY(e.settle());
        QCOMPARE(o->layer()->pendingRenders(), 0);
        QCOMPARE(o->chartsDrawn(), steady);
        // the name, and a category
        e.select(QStringLiteral("B1"));
        e.enter(QStringLiteral("Eastern"));
        e.select(QStringLiteral("A3"));
        e.enter(QStringLiteral("Second"));
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].name, QStringLiteral("Eastern"));
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].categories.value(1), QStringLiteral("Second"));
        // undo puts the number back, and the chart follows
        pressUndo(e.view());
        pressUndo(e.view());
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].name, QStringLiteral("East"));
    }

    // A sheet renamed: a chart that reads it reads it by its new name, on the screen and in the file (its references are written by the saver with the name, and the numbers kept in the
    // part are those under the references as they are written); the edits to rows are still found for the sheet by the name the file has for it, and Undo gives the old name back.
    void aChartFollowsASheetThatIsRenamed()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(e.win.w.session()->renameSheet(0, QStringLiteral("Sales")).ok);
        QVERIFY(e.settle());
        QCOMPARE(o->chartReads(0, 0).size(), 3);
        QVERIFY(e.win.w.session()->insertRows(0, 0, 2).ok);   // (the sheet is found by the name the file has: "Sheet1")
        QVERIFY(e.settle());
        QCOMPARE(o->chartReads(0, 0).value(2).range.row1, quint32(3));
        e.select(QStringLiteral("B5"));
        e.enter(QStringLiteral("77"));
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values, (QVector<double>{10, 77, 30}));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        const QString chart = QString::fromUtf8(savedParts(e.path).value(QStringLiteral("xl/charts/chart1.xml")));
        QVERIFY2(chart.contains(QStringLiteral("<c:f>Sales!$B$4:$B$6</c:f>")), qPrintable(chart));
        QVERIFY2(chart.contains(QStringLiteral("<c:v>77</c:v>")), qPrintable(chart));   // (the numbers under the reference as it is written)
        QVERIFY2(!chart.contains(QStringLiteral("Sheet1!")), qPrintable(chart));
    }

    // A sheet deleted: a chart that read it reads nothing (its references are #REF!), and a sheet added after and given the same name is not the one it reads; Undo gives the sheet back
    // and the chart its numbers.
    void aDeletedSheetsNameGivenToANewSheetIsNotReadByTheChart()
    {
        EditWindow e;
        QVERIFY(e.open(chartOfTheOtherSheetBook()));
        QTRY_VERIFY(shown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(o->chartReads(0, 0).size(), 3);
        QVERIFY(s->deleteSheet(1).ok);
        QVERIFY(e.settle());
        QCOMPARE(o->chartReads(0, 0).size(), 0);
        int made = -1;
        QVERIFY(s->insertSheet(-1, QStringLiteral("Sheet2"), &made).ok);
        QVERIFY(s->type(made, 1, 1, QStringLiteral("999")).ok);
        QVERIFY(e.settle());
        QCOMPARE(o->chartReads(0, 0).size(), 0);   // (the new "Sheet2" is another sheet)
        int refs = 0, found = 0;
        o->chartRefCounts(0, 0, &refs, &found);
        QCOMPARE(found, 0);
        // taken back: the sheet and the chart's cells
        pressUndo(e.view());
        pressUndo(e.view());
        pressUndo(e.view());
        QVERIFY(e.settle());
        QCOMPARE(o->chartReads(0, 0).size(), 3);
    }

    // A sheet added or copied has room among the objects' sheets; a copy of a sheet with a chart says plainly that it does not have the chart, shows none, and the original keeps its own.
    void aCopiedSheetHasNoObjectsAndSaysSo()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        int made = -1;
        const crunch::EditSession::Outcome copied = e.win.w.session()->copySheet(0, -1, QStringLiteral("Copy"), &made);
        QVERIFY(copied.ok);
        QVERIFY2(copied.problem.contains(QStringLiteral("pictures and charts")), qPrintable(copied.problem));
        QVERIFY(e.settle());
        o->show(made);
        QCOMPARE(o->shownSheet(), made);
        QCOMPARE(o->layer()->count(), 0);
        QCOMPARE(o->counts(made).total(), 0);
        o->show(0);
        QTRY_VERIFY(shown(e, 5));
        QVERIFY(e.win.w.session()->insertRows(0, 0, 1).ok);   // (the original's objects still move with its cells)
        QCOMPARE(o->chartReads(0, 0).value(2).range.row1, quint32(2));
    }

    // A sheet that reads right to left is drawn mirrored, and its objects with it: an object over the first columns is at the right of the screen, its picture reads as it was made (it is
    // not turned over), and a click where it is picks it (a click where it would be in a sheet that reads left to right does not).
    void objectsOfASheetThatReadsRightToLeftAreMirroredAndPicked()
    {
        EditWindow e;
        QVERIFY(e.open(rightToLeftDrawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        const grid::GridLayout &L = e.view()->sheet().layout();
        const QRect inLayout = L.rangeRect(6, 0, 8, 1);                       // the picture's cells, as the layout has them
        const QRect onScreen = e.view()->cellRect(6, 0).united(e.view()->cellRect(8, 1));   // (where the screen has them)
        QVERIFY(onScreen.center().x() != inLayout.center().x());
        QCOMPARE(at(shot(e), onScreen.center()), QColor(Qt::red).rgb());
        QVERIFY(at(shot(e), inLayout.center()) != QColor(Qt::red).rgb());
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, inLayout.center());
        QVERIFY(o->layer()->selectedId() != 1);
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, onScreen.center());
        QCOMPARE(o->layer()->selectedId(), 1);
    }

    // Rows and columns inserted or deleted move the objects with the cells, and the chart's references with its cells; Undo puts them back; a hidden row is left out of the chart.
    void objectsAndChartsMoveWithTheCells()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        auto top = [&](int id) { return o->layer()->deviceRect(id, grid::Pane::Body).top(); };
        const grid::GridLayout &L = e.view()->sheet().layout();
        QCOMPARE(top(1), L.cellRect(6, 0).top());
        QVERIFY(e.win.w.session()->insertRows(0, 0, 3).ok);
        QCOMPARE(top(1), L.cellRect(9, 0).top());          // (the picture is three rows lower: its anchor was at row 7)
        QCOMPARE(top(0), L.cellRect(4, 4).top());          // (and the chart: from row 2 to row 5)
        QCOMPARE(o->chartReads(0, 0).value(2).range.row1, quint32(4));   // (its values were B2:B4: they are B5:B7)
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values, (QVector<double>{10, 20, 30}));
        pressUndo(e.view());
        QVERIFY(e.settle());
        QCOMPARE(top(1), L.cellRect(6, 0).top());
        QCOMPARE(top(0), L.cellRect(1, 4).top());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values, (QVector<double>{10, 20, 30}));
        // a column inserted before the chart, and a row hidden in its data
        QVERIFY(e.win.w.session()->insertColumns(0, 0, 2).ok);
        QCOMPARE(o->layer()->deviceRect(0, grid::Pane::Body).left(), L.cellRect(1, 6).left());
        pressUndo(e.view());
        crunch::EditSession::SizeChange hide;
        hide.setHidden = true;
        hide.hidden = true;
        QVERIFY(e.win.w.session()->resizeRows(0, 2, 2, hide, QStringLiteral("Hide")).ok);
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values, (QVector<double>{10, 30}));
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].categories, (QStringList{"Q1", "Q3"}));
        // the rows the picture is over are deleted: it is gone from the screen (no height)
        pressUndo(e.view());
        QVERIFY(e.win.w.session()->deleteRows(0, 6, 3).ok);
        QVERIFY(o->layer()->deviceRect(1, grid::Pane::Body).isEmpty());
        pressUndo(e.view());
        QVERIFY(!o->layer()->deviceRect(1, grid::Pane::Body).isEmpty());
    }

    // Delete with an object picked takes it away as a step Undo gives back; it is saved without it, and with the numbers of a chart whose cells changed; what was not touched is the old
    // file's to the byte.
    void anObjectIsDeletedUndoneAndSaved()
    {
        EditWindow e;
        const QByteArray original = drawingBook();
        QVERIFY(e.open(original));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        const grid::GridLayout &L = e.view()->sheet().layout();
        const QRect pic = L.rangeRect(6, 0, 8, 1);
        QVERIFY(!e.win.w.isModified());
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, pic.center());
        QCOMPARE(o->layer()->selectedId(), 1);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(o->isDeleted(0, 1));
        QVERIFY(e.win.w.isModified());
        QCOMPARE(o->layer()->selectedId(), -1);
        QVERIFY(at(shot(e), pic.center()) != QColor(Qt::red).rgb());
        pressUndo(e.view());
        QVERIFY(!o->isDeleted(0, 1));
        QVERIFY(!e.win.w.isModified());
        o->layer()->prepareAll();
        QCOMPARE(at(shot(e), pic.center()), QColor(Qt::red).rgb());
        pressRedo(e.view());
        QVERIFY(o->isDeleted(0, 1));
        // a cell the chart reads is typed over too
        e.select(QStringLiteral("B3"));
        e.enter(QStringLiteral("55"));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        office::Package before, after;
        QString why;
        QVERIFY(before.readLazy(original, &why));
        QVERIFY2(after.readLazyFile(e.path, &why), qPrintable(why));
        QDomDocument drawing;
        QVERIFY(office::parseXml(after.part(QStringLiteral("/xl/drawings/drawing1.xml")), &drawing));
        QStringList names;
        const QDomNodeList all = drawing.elementsByTagName(QStringLiteral("cNvPr"));
        for (int i = 0; i < all.size(); ++i) names << all.at(i).toElement().attribute(QStringLiteral("name"));
        QVERIFY2(!names.contains(QStringLiteral("Logo")) && names.contains(QStringLiteral("Sales")) && names.contains(QStringLiteral("Box")), qPrintable(names.join(QLatin1Char(','))));
        office::Chart chart;
        QVERIFY(office::readChart(after.part(QStringLiteral("/xl/charts/chart1.xml")), office::defaultTheme(), office::ColorMap(), &chart, &why));
        QCOMPARE(chart.plots[0].series[0].values, (QVector<double>{10, 55, 30}));   // (the cache says what the cells hold: the file stored 10, 20, 30)
        QCOMPARE(chart.plots[0].series[0].name, QStringLiteral("East"));
        // (the deleted picture's own part is gone with it, and its relationship, and its entry of the content types; the chart's, which is used, stays)
        QVERIFY(!after.hasPart(QStringLiteral("/xl/media/image1.png")));
        const QMap<QString, QByteArray> parts = savedParts(e.path);
        QVERIFY(!parts.contains(QStringLiteral("xl/media/image1.png")));
        QVERIFY(hasRelationship(savedParts(e.copyOfFile()), QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"), QStringLiteral("rIdC")));
        QVERIFY2(!hasRelationship(parts, QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"), QStringLiteral("rIdP")), "the picture's relationship is gone");
        QVERIFY2(!hasOverride(parts, QStringLiteral("/xl/media/image1.png")), "the picture's entry of the content types is gone");
        QVERIFY(hasOverride(parts, QStringLiteral("/xl/charts/chart1.xml")));
        QVERIFY(hasOverride(parts, QStringLiteral("/xl/drawings/drawing1.xml")));
        for (const QString &name : before.partNames()) {
            if (name == QLatin1String("/xl/drawings/drawing1.xml") || name == QLatin1String("/xl/charts/chart1.xml") || name.contains(QLatin1String("/worksheets/")) || name == QLatin1String("/xl/workbook.xml") || name == QLatin1String("/xl/media/image1.png")) continue;
            QVERIFY2(after.part(name) == before.part(name), qPrintable(name));
        }
        // the objects are as the file has them: four
        QCOMPARE(int(o->idsOf(0).size()), 4);
        // a chart whose cells did not change keeps its part as it was, whatever else is saved
        e.select(QStringLiteral("C3"));
        e.enter(QStringLiteral("77"));
        QVERIFY(e.settle());
        const QByteArray chartBefore = after.part(QStringLiteral("/xl/charts/chart1.xml"));
        saved.clear();
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        office::Package third;
        QVERIFY(third.readLazyFile(e.path, &why));
        QCOMPARE(third.part(QStringLiteral("/xl/charts/chart1.xml")), chartBefore);
        QCOMPARE(third.part(QStringLiteral("/xl/drawings/drawing1.xml")), after.part(QStringLiteral("/xl/drawings/drawing1.xml")));
        // Undo goes back over the typing, and over the delete too: the picture is written again by the next save
        pressUndo(e.view());
        pressUndo(e.view());
        pressUndo(e.view());
        QVERIFY(!e.win.w.session()->canUndo());
        QVERIFY(!o->isDeleted(0, 1));
        // the file opens with four objects
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 4);
    }

    // A picture taken away, saved, and brought back by Undo is written again by the next save: the picture is in the reopened file with the bytes it had, and its relationship names a part that is there.
    void aDeletedPictureComesBackAfterASaveAndUndo()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        o->layer()->select(1);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(o->isDeleted(0, 1));
        QVERIFY(saveWindow(e));
        QVERIFY(!savedNames(e.path).contains(QStringLiteral("Logo")));
        QVERIFY(reachedBy(e.path, QStringLiteral("image")).isEmpty());
        QVERIFY2(e.win.w.session()->canUndo(), "Undo is offered for a delete that is in the saved file");
        pressUndo(e.view());
        QVERIFY(!o->isDeleted(0, 1));
        QVERIFY(e.win.w.isModified());
        QVERIFY(saveWindow(e));
        QVERIFY(savedNames(e.path).contains(QStringLiteral("Logo")));
        const QVector<Reached> images = reachedBy(e.path, QStringLiteral("image"));
        QCOMPARE(int(images.size()), 1);
        QCOMPARE(images[0].bytes, pngBytes(8, 4, Qt::red));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 5);
        // Redo takes it away again, and Undo once more brings it back
        e.win.w.session()->undoStack()->redo();
        QVERIFY(o->isDeleted(0, 1));
        QVERIFY(saveWindow(e));
        QVERIFY(!savedNames(e.path).contains(QStringLiteral("Logo")));
        e.win.w.session()->undoStack()->undo();
        QVERIFY(saveWindow(e));
        QVERIFY(savedNames(e.path).contains(QStringLiteral("Logo")));
        QCOMPARE(reachedBy(e.path, QStringLiteral("image"))[0].bytes, pngBytes(8, 4, Qt::red));
    }

    // A chart that has style and colors parts comes back with them, byte for byte (only the names of the parts and the ids of the relationships are new).
    void aDeletedChartWithItsStyleAndColorsComesBackWhole()
    {
        QByteArray chartXml, styleXml, colorsXml;
        EditWindow e;
        QVERIFY(e.open(chartWithStylePartsBook(&chartXml, &styleXml, &colorsXml)));
        QTRY_VERIFY(shown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        o->layer()->select(0);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(o->isDeleted(0, 0));
        QVERIFY(saveWindow(e));
        QVERIFY(reachedBy(e.path, QStringLiteral("chart")).isEmpty());
        const QMap<QString, QByteArray> gone = savedParts(e.path);
        for (const QString &name : gone.keys()) QVERIFY2(!name.startsWith(QStringLiteral("xl/charts/")), qPrintable(name));
        QVERIFY(e.win.w.session()->canUndo());
        pressUndo(e.view());
        QVERIFY(saveWindow(e));
        const QVector<Reached> charts = reachedBy(e.path, QStringLiteral("chart"));
        QCOMPARE(int(charts.size()), 1);
        QCOMPARE(charts[0].bytes, chartXml);
        QCOMPARE(charts[0].children.value(QStringLiteral("chartStyle")), styleXml);
        QCOMPARE(charts[0].children.value(QStringLiteral("chartColorStyle")), colorsXml);
        // (the content types name the new parts)
        const QMap<QString, QByteArray> parts = savedParts(e.path);
        QVERIFY2(hasExactOverride(parts, charts[0].part), qPrintable(charts[0].part));
        for (const char *word : {"chartStyle", "chartColorStyle"}) {
            Q_UNUSED(word);
        }
        QString bad;
        QVERIFY2(relationshipsAllResolve(e.path, &bad), qPrintable(bad));
        QVERIFY(orphanParts(e.path).isEmpty());
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("chartstyle+xml"));
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("chartcolorstyle+xml"));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 2);
        QCOMPARE(int(second.w.objects()->chartNow(0, 0).plots.size()), 1);
    }

    // A mix: A is deleted and saved, B is deleted and saved, Undo twice brings both back, and the next save writes both.
    void twoObjectsDeletedInTwoSavesBothComeBack()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        o->layer()->select(1);   // the picture
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(saveWindow(e));
        o->layer()->select(0);   // the chart
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(saveWindow(e));
        const QStringList names = savedNames(e.path);
        QVERIFY(!names.contains(QStringLiteral("Logo")) && !names.contains(QStringLiteral("Sales")));
        pressUndo(e.view());
        pressUndo(e.view());
        QVERIFY(!o->isDeleted(0, 0) && !o->isDeleted(0, 1));
        QVERIFY(saveWindow(e));
        const QStringList back = savedNames(e.path);
        QVERIFY2(back.contains(QStringLiteral("Logo")) && back.contains(QStringLiteral("Sales")) && back.contains(QStringLiteral("Box")), qPrintable(back.join(QLatin1Char(','))));
        QCOMPARE(int(reachedBy(e.path, QStringLiteral("image")).size()), 1);
        QCOMPARE(int(reachedBy(e.path, QStringLiteral("chart")).size()), 1);
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 5);
    }

    // What is kept to bring an object back counts against the bytes Undo holds: a big picture is counted when it is deleted, and let go of, with its step, when the limit drops the step.
    void theBytesKeptForAnObjectCountAgainstTheUndoLimit()
    {
        EditWindow e;
        QRandomGenerator gen(7);
        QImage big(500, 500, QImage::Format_RGB32);
        for (int y = 0; y < big.height(); ++y)
            for (int x = 0; x < big.width(); ++x) big.setPixel(x, y, qRgb(int(gen.bounded(256)), int(gen.bounded(256)), int(gen.bounded(256))));
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        big.save(&buffer, "PNG");
        QVERIFY(png.size() > 400000);
        const QString anchors = twoCell(0, 6, 2, 9, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("Big")));
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {imageTarget(QStringLiteral("rIdP"), 1, png)})));
        QTRY_VERIFY(shown(e, 1));
        crunch::EditSession *s = e.win.w.session();
        s->setUndoByteLimit(qint64(png.size()) * 2);
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        const qint64 before = s->undoBytes();
        o->layer()->select(0);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY2(s->undoBytes() >= before + png.size(), "the picture's bytes are counted with the step");
        // a limit lower than that drops the step (the newest step is kept, so another is made first)
        s->setUndoByteLimit(qint64(png.size()) / 2);
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("1"));
        QVERIFY(e.settle());
        QVERIFY2(s->undoBytes() < png.size(), "the step that held the picture is let go of");
        QVERIFY(saveWindow(e));
        QVERIFY(reachedBy(e.path, QStringLiteral("image")).isEmpty());
        // what is held is the typing only: Undo takes it back and no further
        QVERIFY(s->canUndo());
        pressUndo(e.view());
        QVERIFY(!s->canUndo());
        QVERIFY(o->isDeleted(0, 0));
    }

    // A chart edited before it is deleted comes back edited (written from its model), keeping its style and colors parts; one that reads other cells (a row was inserted, a sheet renamed) reads them.
    void anEditedChartComesBackEditedWithItsStyleAndColors()
    {
        QByteArray chartXml, styleXml, colorsXml;
        EditWindow e;
        QVERIFY(e.open(chartWithStylePartsBook(&chartXml, &styleXml, &colorsXml)));
        QTRY_VERIFY(shown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        o->layer()->select(0);
        QTRY_VERIFY(e.win.w.chartElementsButton() && e.win.w.chartElementsButton()->isVisible());
        QMenu *menu = e.win.w.chartElementsButton()->menu();
        QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
        for (QAction *a : menu->actions())
            if (a->text() == QLatin1String("Chart Title"))
                for (QAction *b : a->menu()->actions())
                    if (b->text() == QLatin1String("On Top")) b->trigger();
        QVERIFY(o->chartNow(0, 0).title.overlay);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(saveWindow(e));
        e.win.w.session()->undoStack()->undo();   // (the delete; the title is the step under it)
        QVERIFY(!o->isDeleted(0, 0));
        QVERIFY(saveWindow(e));
        const QVector<Reached> charts = reachedBy(e.path, QStringLiteral("chart"));
        QCOMPARE(int(charts.size()), 1);
        QVERIFY2(charts[0].bytes != chartXml, "written from the edited model");
        office::Chart now;
        QString why;
        QVERIFY(office::readChart(charts[0].bytes, office::defaultTheme(), office::ColorMap(), &now, &why));
        QVERIFY2(now.title.overlay, "the edit is in the file");
        QCOMPARE(charts[0].children.value(QStringLiteral("chartStyle")), styleXml);
        QCOMPARE(charts[0].children.value(QStringLiteral("chartColorStyle")), colorsXml);
        QString bad;
        QVERIFY2(relationshipsAllResolve(e.path, &bad), qPrintable(bad));
        QVERIFY(orphanParts(e.path).isEmpty());
    }

    // A row inserted above a chart before it is deleted, and a rename of the sheet it reads: the chart comes back where the rows put it, reading the sheet by its new name.
    void aRestoredChartFollowsRowsInsertedAndASheetRenamed()
    {
        QByteArray chartXml, styleXml, colorsXml;
        EditWindow e;
        QVERIFY(e.open(chartWithStylePartsBook(&chartXml, &styleXml, &colorsXml)));
        QTRY_VERIFY(shown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        crunch::EditSession *s = e.win.w.session();
        o->layer()->prepareAll();
        QVERIFY(s->insertRows(0, 0, 1).ok);   // the chart was from row 2 (index 1): it is one row down
        QVERIFY(s->renameSheet(0, QStringLiteral("Data")).ok);
        QVERIFY(e.settle());
        o->layer()->select(0);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(saveWindow(e));
        s->undoStack()->undo();
        QVERIFY(saveWindow(e));
        office::Package p;
        QString why;
        QVERIFY(p.readLazyFile(e.path, &why));
        jp::office::sheet::Workbook book;
        QVERIFY(book.open(p, &why));
        jp::office::sheet::SheetDrawing d;
        QVERIFY(jp::office::sheet::readDrawing(p, jp::office::sheet::drawingPartOf(p, book.info().sheets[0].part), &d, &why));
        int chartRow = -1;
        for (const jp::office::sheet::DrawingObject &obj : std::as_const(d.objects))
            if (obj.kind == jp::office::sheet::ObjectKind::Chart) chartRow = int(obj.anchor.from.row);
        QCOMPARE(chartRow, 2);
        const QVector<Reached> charts = reachedBy(e.path, QStringLiteral("chart"));
        QCOMPARE(int(charts.size()), 1);
        office::Chart c;
        QVERIFY(office::readChart(charts[0].bytes, office::defaultTheme(), office::ColorMap(), &c, &why));
        QVERIFY2(c.plots[0].series[0].valueRef.startsWith(QStringLiteral("Data!")), qPrintable(c.plots[0].series[0].valueRef));
        QVERIFY2(c.plots[0].series[0].valueRef.contains(QStringLiteral("$B$3:$B$5")), qPrintable(c.plots[0].series[0].valueRef));
    }

    // A record is kept while the object is back: a new edit after Undo (which gives up the steps that held it) does not stop the picture, or the chart's style, from being written.
    void aRestoredObjectSurvivesANewEditBeforeTheSave()
    {
        QByteArray chartXml, styleXml, colorsXml;
        EditWindow e;
        QVERIFY(e.open(chartWithStylePartsBook(&chartXml, &styleXml, &colorsXml)));
        QTRY_VERIFY(shown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        for (const int id : {1, 0}) {
            o->layer()->select(id);
            QTest::keyClick(e.view(), Qt::Key_Delete);
        }
        QVERIFY(saveWindow(e));
        pressUndo(e.view());
        pressUndo(e.view());
        e.select(QStringLiteral("E1"));
        e.enter(QStringLiteral("7"));   // (a new step: the steps that were undone are given up)
        QVERIFY(e.settle());
        QVERIFY(saveWindow(e));
        QCOMPARE(reachedBy(e.path, QStringLiteral("image"))[0].bytes, pngBytes(8, 4, Qt::red));
        const QVector<Reached> charts = reachedBy(e.path, QStringLiteral("chart"));
        QCOMPARE(int(charts.size()), 1);
        QCOMPARE(charts[0].bytes, chartXml);
        QCOMPARE(charts[0].children.value(QStringLiteral("chartStyle")), styleXml);
    }

    // A restored chart that reached a drawing of its own (user shapes) and a sheet that gets its first drawing in the same save do not fight over a part name.
    void aRestoredChartsPartsDoNotCollideWithNamesChosenForNewObjects()
    {
        QByteArray chartXml, styleXml, colorsXml;
        QByteArray book = chartWithStylePartsBook(&chartXml, &styleXml, &colorsXml, 2);
        // (a second worksheet, no drawing; the chart gets user shapes: a drawing part its relationships reach)
        const QByteArray rels = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                                "<Relationship Id=\"rId1\" Type=\"http://schemas.microsoft.com/office/2011/relationships/chartStyle\" Target=\"style1.xml\"/>"
                                "<Relationship Id=\"rId2\" Type=\"http://schemas.microsoft.com/office/2011/relationships/chartColorStyle\" Target=\"colors1.xml\"/>"
                                "<Relationship Id=\"rId3\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/chartUserShapes\" Target=\"../drawings/drawing2.xml\"/></Relationships>";
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(book, parts));
        parts[QStringLiteral("xl/charts/_rels/chart1.xml.rels")] = rels;
        parts[QStringLiteral("xl/drawings/drawing2.xml")] = "<c:userShapes xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"/>";
        QByteArray types = parts[QStringLiteral("[Content_Types].xml")];
        types.replace("</Types>", "<Override PartName=\"/xl/drawings/drawing2.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawingml.chartshapes+xml\"/></Types>");
        parts[QStringLiteral("[Content_Types].xml")] = types;
        jp::ZipWriter zip;
        for (auto it = parts.begin(); it != parts.end(); ++it) zip.add(it.key(), it.value(), true);
        book = zip.finish();
        EditWindow e;
        QVERIFY(e.open(book));
        QTRY_VERIFY(shown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        o->layer()->select(0);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(saveWindow(e));
        e.win.w.session()->undoStack()->undo();
        e.win.w.setActiveSheet(1);   // (a sheet with data and no drawing: a chart put on it is given one)
        QTRY_COMPARE(e.win.w.activeSheet(), 1);
        QVERIFY(e.settle());
        e.view()->selectRange(0, 0, 3, 2);
        QVERIFY(e.win.w.insertChart(jp::office::sheet::ChartKind::Column));
        QVERIFY(e.settle());
        QVERIFY2(saveWindow(e), "the save with a restored chart and a new drawing");
        QString bad;
        QVERIFY2(relationshipsAllResolve(e.path, &bad), qPrintable(bad));
        QVERIFY(orphanParts(e.path).isEmpty());
        const QVector<Reached> charts = reachedBy(e.path, QStringLiteral("chart"));
        QCOMPARE(int(charts.size()), 1);
    }

    // An attribute that names a relationship in single quotes is found as well, and one that names none is left out (the restored picture has no dangling id).
    void relationshipIdsInSingleQuotesAndUnresolvedOnesAreHandled()
    {
        QByteArray book;
        {
            QString pic = pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("Quoted"));
            pic.replace(QStringLiteral("r:embed=\"rIdP\""), QStringLiteral("r:embed='rIdP'"));
            QString second = pictureElement(4, QStringLiteral("rIdMissing"), QStringLiteral("Dangling"));
            const QString anchors = twoCell(0, 6, 2, 9, pic) + twoCell(4, 6, 6, 9, second);
            book = bookWithDrawing({dataSheet()}, drawingPart(anchors), {imageTarget(QStringLiteral("rIdP"), 1, pngBytes(8, 4, Qt::red))});
        }
        EditWindow e;
        QVERIFY(e.open(book));
        QTRY_VERIFY(shown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        for (const int id : {0, 1}) {
            o->layer()->select(id);
            QTest::keyClick(e.view(), Qt::Key_Delete);
        }
        QVERIFY(saveWindow(e));
        pressUndo(e.view());
        pressUndo(e.view());
        QVERIFY(saveWindow(e));
        QCOMPARE(reachedBy(e.path, QStringLiteral("image"))[0].bytes, pngBytes(8, 4, Qt::red));
        const QByteArray drawing = savedParts(e.path).value(QStringLiteral("xl/drawings/drawing1.xml"));
        QVERIFY2(!drawing.contains("rIdMissing") && !drawing.contains("@@revive"), "no id that names nothing, and no token left");
        QString bad;
        QVERIFY2(relationshipsAllResolve(e.path, &bad), qPrintable(bad));
    }

    // A group, and the connector beside it, come back with the objects they hold; an external relationship stays external.
    void aDeletedGroupAndAnExternalLinkComeBack()
    {
        QByteArray book = drawingBook();
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(book, parts));
        QByteArray rels = parts[QStringLiteral("xl/drawings/_rels/drawing1.xml.rels")];
        rels.replace("</Relationships>", "<Relationship Id=\"rIdExt\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink\" Target=\"https://example.com/x\" TargetMode=\"External\"/></Relationships>");
        parts[QStringLiteral("xl/drawings/_rels/drawing1.xml.rels")] = rels;
        QByteArray drawing = parts[QStringLiteral("xl/drawings/drawing1.xml")];
        drawing.replace("<xdr:cNvPr id=\"4\" name=\"Box\"/>", "<xdr:cNvPr id=\"4\" name=\"Box\"><a:hlinkClick xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" r:id=\"rIdExt\"/></xdr:cNvPr>");
        parts[QStringLiteral("xl/drawings/drawing1.xml")] = drawing;
        jp::ZipWriter zip;
        for (auto it = parts.begin(); it != parts.end(); ++it) zip.add(it.key(), it.value(), true);
        EditWindow e;
        QVERIFY(e.open(zip.finish()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        for (const int id : {3, 2}) {   // the group, the box with the link
            o->layer()->select(id);
            QTest::keyClick(e.view(), Qt::Key_Delete);
        }
        QVERIFY(saveWindow(e));
        QVERIFY(!savedNames(e.path).contains(QStringLiteral("In1")));
        pressUndo(e.view());
        pressUndo(e.view());
        QVERIFY(saveWindow(e));
        const QStringList names = savedNames(e.path);
        QVERIFY2(names.contains(QStringLiteral("In1")) && names.contains(QStringLiteral("Box")), qPrintable(names.join(QLatin1Char(','))));
        const QByteArray out = savedParts(e.path).value(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"));
        QVERIFY(out.contains("https://example.com/x") && out.contains("TargetMode=\"External\""));
        QString bad;
        QVERIFY2(relationshipsAllResolve(e.path, &bad), qPrintable(bad));
    }

    // Three rounds of delete, save, Undo, save, Redo, save leave one picture, one chart, and not one part nothing relates to.
    void severalRoundsOfUndoAndRedoAcrossSavesLeaveNoOrphans()
    {
        QByteArray chartXml, styleXml, colorsXml;
        EditWindow e;
        QVERIFY(e.open(chartWithStylePartsBook(&chartXml, &styleXml, &colorsXml)));
        QTRY_VERIFY(shown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        crunch::EditSession *s = e.win.w.session();
        o->layer()->prepareAll();
        for (const int id : {1, 0}) {
            o->layer()->select(id);
            QTest::keyClick(e.view(), Qt::Key_Delete);
        }
        for (int round = 0; round < 3; ++round) {
            QVERIFY(saveWindow(e));
            QVERIFY2(reachedBy(e.path, QStringLiteral("chart")).isEmpty(), "taken away");
            QVERIFY(orphanParts(e.path).isEmpty());
            s->undoStack()->undo();
            s->undoStack()->undo();
            QVERIFY(saveWindow(e));
            QCOMPARE(int(reachedBy(e.path, QStringLiteral("chart")).size()), 1);
            QCOMPARE(int(reachedBy(e.path, QStringLiteral("image")).size()), 1);
            QCOMPARE(reachedBy(e.path, QStringLiteral("chart"))[0].children.value(QStringLiteral("chartColorStyle")), colorsXml);
            QVERIFY2(orphanParts(e.path).isEmpty(), qPrintable(orphanParts(e.path).join(QLatin1Char(','))));
            QString bad;
            QVERIFY2(relationshipsAllResolve(e.path, &bad), qPrintable(bad));
            s->undoStack()->redo();
            s->undoStack()->redo();
        }
    }

    // Redo after a save writes a pasted picture and a pasted shape again.
    void aPastedPictureAndShapeAreWrittenAgainByRedoAfterASave()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        for (const int id : {1, 2}) {   // the picture, then the box
            o->layer()->select(id);
            e.win.w.act(QStringLiteral("edit.copy"))->trigger();
            e.win.w.act(QStringLiteral("edit.paste"))->trigger();
            QVERIFY(e.settle());
        }
        QCOMPARE(int(o->idsOf(0).size()), 7);
        QVERIFY(saveWindow(e));
        const int imagesWith = int(reachedBy(e.path, QStringLiteral("image")).size());
        QVERIFY(imagesWith >= 2);
        e.win.w.session()->undoStack()->undo();
        e.win.w.session()->undoStack()->undo();
        QVERIFY(saveWindow(e));
        QCOMPARE(int(reachedBy(e.path, QStringLiteral("image")).size()), 1);
        e.win.w.session()->undoStack()->redo();
        e.win.w.session()->undoStack()->redo();
        QVERIFY(saveWindow(e));
        QCOMPARE(int(reachedBy(e.path, QStringLiteral("image")).size()), imagesWith);
        int boxes = 0;
        for (const QString &n : savedNames(e.path)) boxes += n.startsWith(QStringLiteral("Box"));
        QVERIFY(boxes >= 2);
        QVERIFY(orphanParts(e.path).isEmpty());
        QString bad;
        QVERIFY2(relationshipsAllResolve(e.path, &bad), qPrintable(bad));
    }

    // Two pictures that use one image: one is deleted and saved (the image stays, for the other), Undo and a save put it back, and both pictures show their image.
    void twoPicturesSharingOneImageBothComeBack()
    {
        const QString anchors = twoCell(0, 6, 2, 9, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("First"))) + twoCell(4, 6, 6, 9, pictureElement(4, QStringLiteral("rIdP"), QStringLiteral("Second")));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {imageTarget(QStringLiteral("rIdP"), 1, pngBytes(8, 4, Qt::red))})));
        QTRY_VERIFY(shown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        o->layer()->select(1);   // the second
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(saveWindow(e));
        QVERIFY2(!savedNames(e.path).contains(QStringLiteral("Second")) && savedNames(e.path).contains(QStringLiteral("First")), "one picture is gone");
        const QVector<Reached> inUse = reachedBy(e.path, QStringLiteral("image"));
        QCOMPARE(int(inUse.size()), 1);
        QCOMPARE(inUse[0].bytes, pngBytes(8, 4, Qt::red));   // (the image is not taken away while the first picture uses it)
        e.win.w.session()->undoStack()->undo();
        QVERIFY(saveWindow(e));
        const QStringList names = savedNames(e.path);
        QVERIFY(names.contains(QStringLiteral("First")) && names.contains(QStringLiteral("Second")));
        const QVector<Reached> images = reachedBy(e.path, QStringLiteral("image"));
        QVERIFY(!images.isEmpty());
        for (const Reached &r : images) QCOMPARE(r.bytes, pngBytes(8, 4, Qt::red));
        QString bad;
        QVERIFY2(relationshipsAllResolve(e.path, &bad), qPrintable(bad));
        QVERIFY2(orphanParts(e.path).isEmpty(), qPrintable(orphanParts(e.path).join(QLatin1Char(','))));
        // the reopened file draws both
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 2);
        // and a delete of the first, Undo, and a save leave both again
        o->layer()->select(0);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(saveWindow(e));
        e.win.w.session()->undoStack()->undo();
        QVERIFY(saveWindow(e));
        QVERIFY(savedNames(e.path).contains(QStringLiteral("First")) && savedNames(e.path).contains(QStringLiteral("Second")));
        QVERIFY2(relationshipsAllResolve(e.path, &bad), qPrintable(bad));
    }

    // A chart is deleted and saved, then the sheet it is on is deleted; Undo of the sheet and Undo of the chart bring both back, and the next save writes the chart on its sheet.
    void aChartDeletedThenItsSheetDeletedComesBackWithBothUndone()
    {
        QByteArray chartXml, styleXml, colorsXml;
        EditWindow e;
        QVERIFY(e.open(chartWithStylePartsBook(&chartXml, &styleXml, &colorsXml, 2)));
        QTRY_VERIFY(shown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        crunch::EditSession *s = e.win.w.session();
        o->layer()->prepareAll();
        o->layer()->select(0);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(saveWindow(e));
        QVERIFY(reachedBy(e.path, QStringLiteral("chart")).isEmpty());
        QVERIFY(s->deleteSheet(0).ok);
        QVERIFY(e.settle());
        s->undoStack()->undo();   // the sheet
        QVERIFY(e.settle());
        QVERIFY(s->canUndo());
        s->undoStack()->undo();   // the chart
        QVERIFY(e.settle());
        QVERIFY(saveWindow(e));
        const QVector<Reached> charts = reachedBy(e.path, QStringLiteral("chart"));
        QCOMPARE(int(charts.size()), 1);
        QCOMPARE(charts[0].bytes, chartXml);
        QCOMPARE(charts[0].children.value(QStringLiteral("chartStyle")), styleXml);
        QString bad;
        QVERIFY2(relationshipsAllResolve(e.path, &bad), qPrintable(bad));
        QVERIFY(orphanParts(e.path).isEmpty());
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 2);
    }

    // A chart moved to another sheet, and then moved again, carries the parts it reached each time.
    void aChartMovedTwiceCarriesItsStyleAndColors()
    {
        QByteArray chartXml, styleXml, colorsXml;
        EditWindow e;
        QVERIFY(e.open(chartWithStylePartsBook(&chartXml, &styleXml, &colorsXml, 3)));
        QTRY_VERIFY(shown(e, 2));
        int to = 0;
        e.win.w.setMoveChartAsker([&](const QStringList &, int *chosen) {
            *chosen = to;
            return true;
        });
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        o->layer()->select(0);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("chartdesign.moveChart"))->isEnabled());
        to = 0;   // Sheet2
        e.win.w.act(QStringLiteral("chartdesign.moveChart"))->trigger();
        QCOMPARE(e.win.w.activeSheet(), 1);
        QTRY_VERIFY(shown(e, 1, 1));
        o->layer()->select(0);
        QTRY_VERIFY(e.win.w.act(QStringLiteral("chartdesign.moveChart"))->isEnabled());
        to = 1;   // (Sheet1 and Sheet3 are offered, in that order)
        e.win.w.act(QStringLiteral("chartdesign.moveChart"))->trigger();
        QCOMPARE(e.win.w.activeSheet(), 2);
        QVERIFY(saveWindow(e));
        const QMap<QString, QByteArray> parts = savedParts(e.path);
        int charts = 0;
        for (const QString &name : parts.keys()) {
            if (!name.startsWith(QStringLiteral("xl/charts/chart")) || name.contains(QStringLiteral("_rels"))) continue;
            ++charts;
            QCOMPARE(parts.value(name), chartXml);
            const QByteArray rels = parts.value(QStringLiteral("xl/charts/_rels/") + name.mid(name.lastIndexOf(QLatin1Char('/')) + 1) + QStringLiteral(".rels"));
            QVERIFY2(rels.contains("chartStyle") && rels.contains("chartColorStyle"), qPrintable(name));
        }
        QCOMPARE(charts, 1);
        QVERIFY(orphanParts(e.path).isEmpty());
    }

    // A part many ways reach is read once, and a part too big for what is kept is refused before it is inflated: the delete is then not undone after a save, as before.
    void aPartTooBigForWhatIsKeptIsRefusedAndOneSharedIsHeldOnce()
    {
        QByteArray huge(60 << 20, '\0');   // (compresses to almost nothing)
        const QString anchors = twoCell(0, 6, 2, 9, pictureElement(3, QStringLiteral("rIdH"), QStringLiteral("Huge")));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {imageTarget(QStringLiteral("rIdH"), 1, huge)})));
        QTRY_VERIFY(shown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->select(0);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(o->isDeleted(0, 0));
        QVERIFY2(e.win.w.session()->undoBytes() < (8 << 20), "its bytes were not read into the step");
        QVERIFY(saveWindow(e));
        QVERIFY(!e.win.w.session()->canUndo());
    }

    // An object put in, saved, and taken away by Undo leaves nothing of itself in the next save: no part, no relationship, no entry of the content types.
    void anObjectAddedSavedAndUndoneLeavesNothingBehind()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        e.view()->selectRange(0, 0, 3, 2);
        e.win.w.insertChart(jp::office::sheet::ChartKind::Line);
        QVERIFY(e.settle());
        QCOMPARE(int(o->idsOf(0).size()), 6);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        QVERIFY(savedParts(e.path).contains(QStringLiteral("xl/charts/chart2.xml")));
        QVERIFY(e.win.w.session()->canUndo());
        pressUndo(e.view());
        saved.clear();
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        const QMap<QString, QByteArray> parts = savedParts(e.path);
        QVERIFY2(!parts.contains(QStringLiteral("xl/charts/chart2.xml")), "the chart's part is gone");
        QVERIFY(parts.contains(QStringLiteral("xl/charts/chart1.xml")));
        QVERIFY2(!hasOverride(parts, QStringLiteral("/xl/charts/chart2.xml")), "its entry of the content types is gone");
        QVERIFY2(!parts.value(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels")).contains("chart2"), "its relationship is gone");
        office::Package after;
        QString why;
        QVERIFY(after.readLazyFile(e.path, &why));
        for (const office::Relationship &r : after.relationships(QStringLiteral("/xl/drawings/drawing1.xml")))
            QVERIFY2(r.external || after.hasPart(r.target), qPrintable(r.target));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 5);
    }

    // Hostile drawings: more anchors than may be read, groups nested past the limit, a picture that says it has ten billion pixels, a chart of many series that read whole columns, and
    // objects at the far corner of the sheet. The window opens each, makes what it can, and the work of a paint stays what the layer bounds it to.
    void hostileDrawingsAreBounded()
    {
        {
            QString many;
            for (int i = 0; i < jp::office::sheet::kMaxDrawingAnchors + 2000; ++i) many += twoCell(i % 40, i % 300, i % 40 + 1, i % 300 + 1, shapeElement(2 + i, QStringLiteral("S")));
            EditWindow e;
            QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(many), {})));
            QTRY_VERIFY_WITH_TIMEOUT(e.win.w.objects()->shownSheet() == 0 && e.win.w.objects()->layer()->count() > 0, 30000);   // (a drawing of ten thousand anchors is parsed on a worker: on a machine running every suite at once that takes more than five seconds)
            crunch::SheetObjects *o = e.win.w.objects();
            // the sheet is listed, and an object is made when a picture of it is wanted: not the ten thousand, but those on and near the screen
            QCOMPARE(o->layer()->count(), jp::office::sheet::kMaxDrawingAnchors);
            QCOMPARE(o->drawingsParsedApart(), 1);   // (a drawing of megabytes is parsed on a worker: the window's thread only reads what the parser made)
            QTRY_VERIFY_WITH_TIMEOUT(!o->layer()->busy(), 60000);
            QVERIFY2(o->objectsMade() < jp::office::sheet::kMaxDrawingAnchors / 2, qPrintable(QString::number(o->objectsMade())));
            const qint64 renders = o->layer()->stats().renders;
            shot(e);   // (a paint of the window makes nothing)
            QCOMPARE(o->layer()->stats().renders, renders);
            QCOMPARE(o->layer()->stats().paintRenders, qint64(0));
            QVERIFY(o->layer()->cacheBytes() <= jp::grid::ObjectLayer::kCacheBytes);
            const crunch::DrawingCounts c = o->counts(0);   // (this one makes them all)
            QCOMPARE(c.anchors, jp::office::sheet::kMaxDrawingAnchors + 2000);
            QVERIFY(c.truncated);
            QCOMPARE(c.shapes, jp::office::sheet::kMaxDrawingAnchors);
            QVERIFY(!c.why.isEmpty());
            QCOMPARE(o->objectsMade(), jp::office::sheet::kMaxDrawingAnchors);
        }
        {
            QString nest = shapeElement(9, QStringLiteral("Core"));
            for (int i = 0; i < 120; ++i) nest = groupElement(10 + i, nest);
            EditWindow e;
            QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(twoCell(1, 1, 5, 5, nest)), {})));
            QTRY_VERIFY_WITH_TIMEOUT(e.win.w.objects()->shownSheet() == 0, 60000);   // (a worker parses the drawing: on a machine running every suite at once that takes far more than five seconds)
            const crunch::DrawingCounts c = e.win.w.objects()->counts(0);
            QCOMPARE(c.groups, 1);
            QVERIFY(c.shown + c.leftOut == 1);   // (drawn, or left out in words: no crash, no loop)
            e.win.w.objects()->layer()->prepareAll();
            shot(e);
        }
        {
            EditWindow e;
            const QString anchors = twoCell(1, 1, 5, 5, pictureElement(2, QStringLiteral("rIdP"))) + twoCell(7, 1, 9, 3, shapeElement(3, QStringLiteral("Box")));
            QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {drawtest::DrawingTarget{QStringLiteral("rIdP"), QStringLiteral("xl/media/image1.png"), hugePng(), QStringLiteral("image/png"), QStringLiteral("image")}})));
            QTRY_VERIFY_WITH_TIMEOUT(e.win.w.objects()->shownSheet() == 0, 60000);   // (a worker parses the drawing: on a machine running every suite at once that takes far more than five seconds)
            e.win.w.objects()->layer()->prepareAll();
            shot(e);
            // the picture says it has ten billion pixels and holds none: no byte of one is decoded (counted in the bytes the drawing code holds decoded), and what the layer holds is the shape's
            QVERIFY(e.win.w.objects()->counts(0).pictures == 1);
            QCOMPARE(e.win.w.objects()->imageBytesDecoded(), qint64(0));
            QVERIFY(e.win.w.objects()->layer()->cacheBytes() < (qint64(1) << 20));
        }
        {
            // thirty series that read whole columns of a sheet that holds a few cells, and objects at the far corner
            QVector<SeriesSpec> series;
            for (int i = 0; i < 30; ++i) series.push_back(SeriesSpec{QString(), QStringLiteral("Sheet1!$A:$A"), QStringLiteral("Sheet1!$B:$B"), QStringLiteral("S"), {"a"}, {1}, QString()});
            const QString anchors = twoCell(0, 5, 10, 15, chartFrame(2, QStringLiteral("rIdC"))) + twoCell(16383, 1048575, 16383, 1048575, shapeElement(3, QStringLiteral("Corner")), QString(), 9525 * 900, 9525 * 900, 9525 * 900, 9525 * 900)
                                    + twoCell(99999999, 2000000000, 2147483647, 2147483647, shapeElement(4, QStringLiteral("Far")));
            EditWindow e;
            QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("lineChart"), series))})));
            QTRY_VERIFY_WITH_TIMEOUT(e.win.w.objects()->shownSheet() == 0, 60000);   // (a worker parses the drawing: on a machine running every suite at once that takes far more than five seconds)
            e.win.w.objects()->layer()->prepareAll();
            shot(e);
            const jp::office::Chart c = e.win.w.objects()->chartNow(0, 0);
            QCOMPARE(int(c.plots[0].series.size()), 30);
            QCOMPARE(int(c.plots[0].series[0].values.size()), 4);   // (the column read as far as the sheet's cells go)
            QVERIFY(e.win.w.objects()->layer()->cacheBytes() <= jp::grid::ObjectLayer::kCacheBytes);
        }
    }

    // A chart sheet is a chart and nothing else: no cell cursor, no typing into a cell or the formula bar, no heading or gridlines; the page is white behind a chart with no fill of its own.
    void aChartSheetIsOnlyAChart()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithChartSheet()));
        QCOMPARE(e.win.w.shownSheets(), (QVector<int>{0, 1}));
        QVERIFY(e.view()->cellsEnabled());
        // (the cell cursor on a sheet of cells: the color it is drawn in at the left edge of A1, and how much of it there is)
        e.select(QStringLiteral("A1"));
        const QRect a1 = e.view()->sheet().layout().cellRect(0, 0);
        const QRect around = a1.adjusted(-3, -3, 3, 3);
        auto count = [&](const QImage &img, QRgb color) {
            int n = 0;
            for (int y = around.top(); y <= around.bottom(); ++y)
                for (int x = around.left(); x <= around.right(); ++x) n += at(img, x, y) == color;
            return n;
        };
        const QImage cells = shot(e);
        QRgb cursor = 0xFFFFFFFF;
        for (int x = a1.left() - 1; x <= a1.left() + 2; ++x)
            if (qGray(at(cells, x, a1.center().y())) < qGray(cursor)) cursor = at(cells, x, a1.center().y());
        QVERIFY2(qGray(cursor) < 160, "the cursor is dark at the edge of the cell");
        QVERIFY(count(cells, cursor) >= a1.height());
        e.win.w.setActiveSheet(1);
        QTRY_VERIFY(shown(e, 1, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(o->isChartSheet(1));
        QVERIFY(!e.view()->cellsEnabled());
        QVERIFY(e.win.w.formulaBar()->isReadOnly());
        QVERIFY(!e.win.w.nameBox()->isEnabled());
        o->layer()->prepareAll();
        const QImage img = shot(e);
        // a white page, the chart's ink on it, and no cell cursor or heading
        QCOMPARE(at(img, 2, 2), QColor(Qt::white).rgb());
        QCOMPARE(count(img, cursor), 0);   // (no cell cursor over the page, by its pixels)
        QSet<QRgb> colors;
        const QRect box = o->layer()->deviceRect(0, grid::Pane::Body);
        QVERIFY(!box.isEmpty());
        for (int y = box.top(); y < box.bottom(); y += 3)
            for (int x = box.left(); x < box.right(); x += 3) colors.insert(at(img, x, y));
        QVERIFY2(colors.size() >= 4, qPrintable(QString::number(colors.size())));
        QVERIFY(!e.view()->showHeadings());
        // nothing a person types or clicks starts an entry or changes a cell
        const QString before = e.selected();
        QTest::keyClick(e.view(), Qt::Key_X);
        QVERIFY(!e.win.w.cellEditor()->isVisible());
        QTest::keyClick(e.win.w.formulaBar(), Qt::Key_Y);
        QVERIFY(e.win.w.formulaBar()->text().isEmpty());
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(box.center()));
        QCOMPARE(e.selected(), before);
        QVERIFY(!e.win.w.isModified());
        // the first sheet has its cells and its bars again
        e.win.w.setActiveSheet(0);
        QVERIFY(e.view()->cellsEnabled());
        QVERIFY(!e.win.w.formulaBar()->isReadOnly());
        QVERIFY(e.win.w.nameBox()->isEnabled());
        QTRY_VERIFY(shown(e, 0, 0));
    }

    // The Draw Text Box tool belongs to the sheet it was picked on: showing another sheet puts it down, and a chart sheet has no place for a text box, which the status bar says.
    void aDrawnTextBoxToolIsPutDownWhenAnotherSheetIsShown()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithChartSheet()));
        crunch::SheetObjects *o = e.win.w.objects();
        e.win.w.act(QStringLiteral("insert.textBox.draw"))->trigger();
        QVERIFY(o->layer()->boxArmed());
        e.win.w.setActiveSheet(1);
        QTRY_VERIFY(shown(e, 1, 1));
        QVERIFY(!o->layer()->boxArmed());
        QVERIFY2(e.win.w.statusText().contains(QStringLiteral("chart sheet")), qPrintable(e.win.w.statusText()));
        e.win.w.setActiveSheet(0);
        QTRY_VERIFY(shown(e, 0, 0));
        QVERIFY(!o->layer()->boxArmed());   // (not armed again by coming back)
    }

    // The commands that work on cells are off while a chart sheet is shown (they would act on cells nobody sees), by the commands themselves and on the ribbon's buttons, whatever is
    // triggered (by the command, or by its keys) does nothing, and they are on again on a sheet of cells: those that were on before.
    void aChartSheetTurnsOffEveryCommandOnCells()
    {
        const QStringList commands = {"cells.insert", "cells.insert.cells", "cells.insert.rows", "cells.insert.columns", "cells.delete", "cells.delete.cells", "cells.delete.rows", "cells.delete.columns",
                                      "cells.format", "format.rowHeight", "format.columnWidth", "format.fitRow", "format.fitColumn", "format.hideRows", "format.hideColumns", "format.unhideRows",
                                      "format.unhideColumns", "edit.cut", "edit.copy", "edit.paste", "paste.values", "paste.formulas", "paste.formats", "paste.transpose", "paste.special",
                                      "fill.down", "fill.right", "fill.up", "fill.left", "fill.series", "edit.fill", "edit.autosum", "edit.autosum.sum", "edit.autosum.average",
                                      "edit.autosum.count", "edit.autosum.max", "edit.autosum.min", "edit.selectAll", "edit.clear", "edit.sort", "edit.find", "view.formulas", "view.gridlines",
                                      "view.headings", "view.freeze", "view.freezeRow", "view.freezeCol", "view.unfreeze"};
        EditWindow e;
        QVERIFY(e.open(bookWithChartSheet()));
        CrunchWindow &w = e.win.w;
        QStringList on;
        for (const QString &id : commands) {
            QVERIFY2(w.act(id), qPrintable(id));
            if (w.act(id)->isEnabled()) on << id;
        }
        // (the ones that work are on, on a sheet of cells)
        for (const char *id : {"cells.insert", "cells.delete", "cells.format", "edit.autosum", "edit.fill", "edit.cut", "edit.copy", "edit.paste", "fill.down", "format.rowHeight", "view.freeze"})
            QVERIFY2(on.contains(QLatin1String(id)), id);
        w.setActiveSheet(1);
        QTRY_VERIFY(shown(e, 1, 1));
        for (const QString &id : commands) QVERIFY2(!w.act(id)->isEnabled(), qPrintable(id));
        // the buttons of the ribbon that carry them are off
        int buttons = 0;
        for (QToolButton *b : w.findChildren<QToolButton *>())
            if (b->defaultAction() && commands.contains(b->defaultAction()->objectName())) {
                ++buttons;
                QVERIFY2(!b->isEnabled(), qPrintable(b->defaultAction()->objectName()));
            }
        QVERIFY2(buttons >= 8, qPrintable(QString::number(buttons)));
        // triggering any of them, and pressing their keys, changes nothing: no step, no change of the file, no notice
        const bool undoable = w.session()->canUndo();
        for (const QString &id : commands) w.act(id)->trigger();
        for (const Qt::Key key : {Qt::Key_Plus, Qt::Key_Minus, Qt::Key_9, Qt::Key_0, Qt::Key_C, Qt::Key_X, Qt::Key_V, Qt::Key_A, Qt::Key_D, Qt::Key_R})
            QTest::keyClick(e.view(), key, Qt::ControlModifier);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QCOMPARE(w.session()->canUndo(), undoable);
        QVERIFY(!w.isModified());
        QVERIFY(e.win.told.isEmpty());
        QVERIFY(!w.cellEditor()->isVisible());
        // back on a sheet of cells: the ones that were on are on, the ones that were off (not yet available) are off
        w.setActiveSheet(0);
        QVERIFY(e.view()->cellsEnabled());
        for (const QString &id : commands) QCOMPARE(w.act(id)->isEnabled(), on.contains(id));
        // and a command works there again
        e.select(QStringLiteral("B2"));
        w.act("cells.insert.rows")->trigger();
        QVERIFY(w.isModified());
    }

    // Two objects use one picture, by one relationship or by two: deleting one leaves the picture, the relationship the other uses, and the picture's entry of the content types; deleting the
    // other takes them. (With two relationships to the picture the deleted object's own goes, and the picture stays because the other still goes to it.)
    void aPictureTwoObjectsUseStaysUntilBothAreGone()
    {
        for (const bool oneRelationship : {true, false}) {
            const QString secondId = oneRelationship ? QStringLiteral("rIdP") : QStringLiteral("rIdQ");
            const QString anchors = twoCell(0, 6, 2, 9, pictureElement(3, QStringLiteral("rIdP"), QStringLiteral("First"))) + twoCell(4, 6, 6, 9, pictureElement(4, secondId, QStringLiteral("Second")))
                                    + twoCell(0, 11, 2, 13, shapeElement(5, QStringLiteral("Box")));
            QVector<DrawingTarget> targets = {imageTarget(QStringLiteral("rIdP"), 1, pngBytes(8, 4, Qt::red))};
            if (!oneRelationship) targets.push_back(DrawingTarget{QStringLiteral("rIdQ"), QStringLiteral("xl/media/image1.png"), pngBytes(8, 4, Qt::red), QString(), QStringLiteral("image")});
            EditWindow e;
            QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), targets)));
            QTRY_VERIFY(shown(e, 3));
            crunch::SheetObjects *o = e.win.w.objects();
            auto save = [&]() -> bool {
                QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
                return e.win.w.save() && savedOk(saved);
            };
            auto takeAway = [&](int id) {
                const crunch::ObjectState before = o->stateOf(0, id);
                crunch::ObjectState after = before;
                after.deleted = true;
                return e.win.w.session()->deleteObject(0, id, QStringLiteral("Delete Object"), before, after).ok;
            };
            const QString rels = QStringLiteral("xl/drawings/_rels/drawing1.xml.rels");
            QVERIFY(hasRelationship(savedParts(e.path), rels, QStringLiteral("rIdP")));
            QVERIFY(takeAway(0));
            QVERIFY(save());
            QMap<QString, QByteArray> parts = savedParts(e.path);
            QVERIFY2(parts.contains(QStringLiteral("xl/media/image1.png")), "the picture the other uses stays");
            QVERIFY(hasRelationship(parts, rels, secondId));
            QCOMPARE(hasRelationship(parts, rels, QStringLiteral("rIdP")), oneRelationship);   // (the one the deleted object had, if the other does not use it)
            QVERIFY(hasOverride(parts, QStringLiteral("/xl/media/image1.png")));
            QCOMPARE(int(o->idsOf(0).size()), 2);
            QVERIFY(takeAway(1));
            QVERIFY(save());
            parts = savedParts(e.path);
            QVERIFY2(!parts.contains(QStringLiteral("xl/media/image1.png")), "the last that used it is gone, and the picture");
            QVERIFY(!hasRelationship(parts, rels, QStringLiteral("rIdP")));
            QVERIFY(!hasRelationship(parts, rels, QStringLiteral("rIdQ")));
            QVERIFY(!hasOverride(parts, QStringLiteral("/xl/media/image1.png")));
            QCOMPARE(int(o->idsOf(0).size()), 1);   // (the shape)
            // the file opens with the one object
            Window second;
            QVERIFY(second.open(e.copyOfFile()));
            QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 1);
        }
    }

    // An object anchored at a place of the sheet (an absolute anchor) keeps its place and size when rows and columns are inserted and resized, saved and opened again: on the screen,
    // and in the file, where its anchor is as it was.
    void anAbsoluteAnchorKeepsItsPlaceAndSize()
    {
        const qint64 emu = 9525;
        const QString anchors = absolute(300 * emu, 100 * emu, 120 * emu, 60 * emu, shapeElement(2, QStringLiteral("Fixed"))) + twoCell(0, 8, 2, 10, shapeElement(3, QStringLiteral("Moving")));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {})));
        QTRY_VERIFY(shown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        const QRect fixed = o->layer()->deviceRect(0, grid::Pane::Body), moving = o->layer()->deviceRect(1, grid::Pane::Body);
        QCOMPARE(fixed.size(), QSize(120, 60));
        QCOMPARE(fixed.topLeft() - e.view()->sheet().layout().gridRect().topLeft(), QPoint(300, 100));
        QVERIFY(e.win.w.session()->insertRows(0, 0, 4).ok);
        QVERIFY(e.win.w.session()->insertColumns(0, 0, 2).ok);
        crunch::EditSession::SizeChange tall;
        tall.setSize = true;
        tall.size = 60;   // (points)
        QVERIFY(e.win.w.session()->resizeRows(0, 2, 5, tall, QStringLiteral("Taller")).ok);
        QVERIFY(e.win.w.session()->resizeColumns(0, 0, 3, tall, QStringLiteral("Wider")).ok);
        QCOMPARE(o->layer()->deviceRect(0, grid::Pane::Body), fixed);        // (where it was, as large as it was)
        QVERIFY(o->layer()->deviceRect(1, grid::Pane::Body) != moving);      // (the object of the cells moved)
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        // the file has the anchor as it was: an absolute one, the same place and size
        office::Package p;
        QString why;
        QVERIFY2(p.readLazyFile(e.path, &why), qPrintable(why));
        QDomDocument doc;
        QVERIFY(office::parseXml(p.part(QStringLiteral("/xl/drawings/drawing1.xml")), &doc));
        bool found = false;
        const QDomNodeList absolutes = doc.elementsByTagName(QStringLiteral("absoluteAnchor"));
        for (int i = 0; i < absolutes.size(); ++i) {
            const QDomElement a = absolutes.at(i).toElement();
            const QDomElement pos = a.firstChildElement(QStringLiteral("pos")), ext = a.firstChildElement(QStringLiteral("ext"));
            if (a.elementsByTagName(QStringLiteral("cNvPr")).at(0).toElement().attribute(QStringLiteral("name")) != QLatin1String("Fixed")) continue;
            found = true;
            QCOMPARE(pos.attribute(QStringLiteral("x")).toLongLong(), 300 * emu);
            QCOMPARE(pos.attribute(QStringLiteral("y")).toLongLong(), 100 * emu);
            QCOMPARE(ext.attribute(QStringLiteral("cx")).toLongLong(), 120 * emu);
            QCOMPARE(ext.attribute(QStringLiteral("cy")).toLongLong(), 60 * emu);
        }
        QVERIFY2(found, "the absolute anchor is still one");
        // opened again, it is where it was
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 2);
        second.w.objects()->layer()->prepareAll();
        QCOMPARE(second.w.objects()->layer()->deviceRect(0, grid::Pane::Body).size(), fixed.size());
        QCOMPARE(second.w.objects()->layer()->deviceRect(0, grid::Pane::Body).topLeft() - second.w.view()->sheet().layout().gridRect().topLeft(), QPoint(300, 100));
    }

    // A name of the sheet itself in a chart on the second sheet, with no sheet in front of it, is that sheet's name: the numbers of that sheet's cells, not those of the first sheet's name
    // that has the same words.
    void aSheetLocalNameInAChartOnTheSecondSheetIsThatSheets()
    {
        auto numbers = [](int scale) {
            QString rows;
            for (int i = 0; i < 3; ++i) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\" t=\"inlineStr\"><is><t>Q%1</t></is></c><c r=\"B%1\"><v>%2</v></c></row>").arg(i + 2).arg((i + 1) * scale);
            return worksheet(rows);
        };
        const QVector<SeriesSpec> series = {SeriesSpec{QString(), QString(), QStringLiteral("Local"), QStringLiteral("S"), {}, {1, 2, 3}, QString()}};
        const QString names = QStringLiteral("<definedName name=\"Local\" localSheetId=\"0\">Sheet1!$B$2:$B$4</definedName><definedName name=\"Local\" localSheetId=\"1\">Sheet2!$B$2:$B$4</definedName>");
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({numbers(10), numbers(100)}, drawingPart(twoCell(4, 1, 11, 13, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales")))),
                                       {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series))}, QStringList(), names, QByteArray(), 1)));
        crunch::SheetObjects *o = e.win.w.objects();
        QCOMPARE(o->chartNow(1, 0).plots[0].series[0].values, (QVector<double>{100, 200, 300}));   // (the second sheet's own: not the first's 10, 20, 30)
        QVERIFY(e.settle());
        QCOMPARE(o->chartReads(1, 0).value(0).sheet, 1);
    }

    // A part that several anchors name is read once, and a chart drawn again for its cells is not read from its part again.
    void aChartPartIsReadOnce()
    {
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
        QString anchors;
        for (int i = 0; i < 6; ++i) anchors += twoCell(4 + i, 1, 6 + i, 8, chartFrame(2 + i, QStringLiteral("rIdC"), QStringLiteral("Sales")));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series))})));
        QTRY_VERIFY(shown(e, 6));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        QCOMPARE(o->chartPartsRead(), 1);
        e.select(QStringLiteral("B3"));
        e.enter(QStringLiteral("55"));
        QVERIFY(e.settle());
        o->layer()->prepareAll();
        QCOMPARE(o->chartNow(0, 3).plots[0].series[0].values, (QVector<double>{10, 55, 30}));
        QCOMPARE(o->chartPartsRead(), 1);
    }

    // Saving looks for the charts in the parts of the file: a sheet that was never shown has its chart's numbers made again, and its drawing is not read, nor an object of it made.
    void savingDoesNotBuildTheSheetsNeverShown()
    {
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet(), dataSheet()}, drawingPart(twoCell(1, 1, 8, 12, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales")))),
                                       {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series))}, QStringList(), QString(), QByteArray(), 1)));
        crunch::SheetObjects *o = e.win.w.objects();
        QTRY_VERIFY(o->listed(0));
        e.select(QStringLiteral("B3"));
        e.enter(QStringLiteral("55"));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        QVERIFY(!o->listed(1));
        QCOMPARE(o->objectsMade(), 0);
        QCOMPARE(o->chartPartsRead(), 0);
        office::Package after;
        QString why;
        QVERIFY2(after.readLazyFile(e.path, &why), qPrintable(why));
        office::Chart chart;
        QVERIFY(office::readChart(after.part(QStringLiteral("/xl/charts/chart1.xml")), office::defaultTheme(), office::ColorMap(), &chart, &why));
        QCOMPARE(chart.plots[0].series[0].values, (QVector<double>{10, 55, 30}));
    }

    // A shape that keeps its size when cells are resized (editAs "oneCell") does: a row inserted inside its span does not make it taller, and a taller row under it does not either; and the
    // anchor saved is the one that gives it the size it has.
    void anObjectThatKeepsItsSizeKeepsItAndIsSavedSo()
    {
        {
            EditWindow e;
            QVERIFY(e.open(drawingBook()));
            QTRY_VERIFY(shown(e, 5));
            crunch::SheetObjects *o = e.win.w.objects();
            o->layer()->prepareAll();
            const QRect box = o->layer()->deviceRect(2, grid::Pane::Body);
            QCOMPARE(savedAnchor(e.path, QStringLiteral("Box")).toRow, 12);
            QVERIFY(e.win.w.session()->insertRows(0, 11, 1).ok);   // (inside its two rows)
            const QRect now = o->layer()->deviceRect(2, grid::Pane::Body);
            QCOMPARE(now.height(), box.height());
            QCOMPARE(now.top(), box.top());
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.save());
            QVERIFY2(savedOk(saved), "the save failed");
            const SavedAnchor a = savedAnchor(e.path, QStringLiteral("Box"));
            QVERIFY(a.found);
            QCOMPARE(a.fromRow, 10);
            QCOMPARE(a.toRow, 12);   // (as tall as before: not 13)
            QCOMPARE(a.editAs, QStringLiteral("oneCell"));
            // a taller row under it: it keeps its height, and the file says where that ends
            crunch::EditSession::SizeChange tall;
            tall.setSize = true;
            tall.size = 60;   // (points)
            QVERIFY(e.win.w.session()->resizeRows(0, 10, 10, tall, QStringLiteral("Taller")).ok);
            QCOMPARE(o->layer()->deviceRect(2, grid::Pane::Body).height(), box.height());
            saved.clear();
            QVERIFY(e.win.w.save());
            QVERIFY2(savedOk(saved), "the save failed");
            const SavedAnchor b = savedAnchor(e.path, QStringLiteral("Box"));
            QVERIFY(b.found);
            QCOMPARE(b.fromRow, 10);
            QVERIFY2(b.toRow < 12, qPrintable(QString::number(b.toRow)));   // (a row of 80 pixels holds most of its 40)
            QCOMPARE(b.editAs, QStringLiteral("oneCell"));
        }
        {
            // an object that is not told to keep its size grows with the rows inserted inside it (the chart, over rows 1 to 13)
            EditWindow e;
            QVERIFY(e.open(drawingBook()));
            QTRY_VERIFY(shown(e, 5));
            crunch::SheetObjects *o = e.win.w.objects();
            o->layer()->prepareAll();
            const int height = o->layer()->deviceRect(0, grid::Pane::Body).height();
            QVERIFY(e.win.w.session()->insertRows(0, 5, 2).ok);
            QCOMPARE(o->layer()->deviceRect(0, grid::Pane::Body).height(), height + 2 * e.view()->sheet().layout().cellRect(5, 0).height());
        }
    }

    // A hidden row stays hidden from the chart when rows are inserted above it: the chart reads the rows it read, and still leaves the hidden one out.
    void aHiddenRowStaysLeftOutWhenRowsAreInserted()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        crunch::EditSession::SizeChange hide;
        hide.setHidden = true;
        hide.hidden = true;
        QVERIFY(e.win.w.session()->resizeRows(0, 2, 2, hide, QStringLiteral("Hide")).ok);
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values, (QVector<double>{10, 30}));
        QVERIFY(e.win.w.session()->insertRows(0, 0, 2).ok);
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values, (QVector<double>{10, 30}));
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].categories, (QStringList{"Q1", "Q3"}));
        // and the same when the hidden row's place is deleted along with the rows above it, and undone
        pressUndo(e.view());
        QVERIFY(e.settle());
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values, (QVector<double>{10, 30}));
    }

    // Rows inserted, and rows made taller, move the objects in place: the pictures made stay (no object is made again, no picture drawn again), and the object picked is still the one picked.
    void insertingRowsMovesTheObjectsInPlace()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        const grid::GridLayout &L = e.view()->sheet().layout();
        // (what the window does after an edit is queued: it is let run before anything is looked at)
        auto letRun = [] {
            QCoreApplication::processEvents();
            QCoreApplication::sendPostedEvents(nullptr, 0);
            QCoreApplication::processEvents();
        };
        // the picture is picked
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, L.rangeRect(6, 0, 8, 1).center());
        QCOMPARE(o->layer()->selectedId(), 1);
        QTRY_COMPARE(o->layer()->pendingRenders(), 0);
        const jp::grid::ObjectLayer::Stats before = o->layer()->stats();
        const int made = o->objectsMade();
        const qint64 charts = o->chartsDrawn();
        const qint64 held = o->layer()->cacheBytes();
        QVERIFY(held > 0);
        const int top = o->layer()->deviceRect(1, grid::Pane::Body).top();
        // below every object, between them, and above them: the objects below move, none is made or drawn again, and the one picked stays picked
        QVERIFY(e.win.w.session()->insertRows(0, 30, 2).ok);
        QVERIFY(e.win.w.session()->insertRows(0, 14, 2).ok);
        letRun();
        QCOMPARE(o->layer()->deviceRect(1, grid::Pane::Body).top(), top);
        QCOMPARE(o->layer()->selectedId(), 1);
        QVERIFY(e.win.w.session()->insertRows(0, 0, 3).ok);
        letRun();
        QCOMPARE(o->layer()->deviceRect(1, grid::Pane::Body).top(), L.cellRect(9, 0).top());
        QCOMPARE(o->layer()->selectedId(), 1);
        // a row made taller, under no object
        crunch::EditSession::SizeChange tall;
        tall.setSize = true;
        tall.size = 60;
        QVERIFY(e.win.w.session()->resizeRows(0, 40, 40, tall, QStringLiteral("Taller")).ok);
        letRun();
        QCOMPARE(o->layer()->selectedId(), 1);
        QCOMPARE(o->layer()->cacheBytes(), held);
        o->layer()->prepareAll();
        QTRY_COMPARE(o->layer()->pendingRenders(), 0);
        QCOMPARE(o->objectsMade(), made);
        QCOMPARE(o->chartsDrawn(), charts);
        QCOMPARE(o->layer()->stats().renders, before.renders);
        QCOMPARE(o->layer()->cacheBytes(), held);
        // Undo and Redo of them keep the one picked, too
        pressUndo(e.view());
        pressUndo(e.view());
        letRun();
        QCOMPARE(o->layer()->selectedId(), 1);
        QCOMPARE(o->layer()->deviceRect(1, grid::Pane::Body).top(), top);   // (back where it was: the rows above it are as they were)
        pressRedo(e.view());
        letRun();
        QCOMPARE(o->layer()->selectedId(), 1);
        QCOMPARE(o->objectsMade(), made);
    }

    // A chart is drawn again when what it reads is not what it was drawn from, wherever it is: a row inserted among its cells (so there is a blank between) does it, though the chart only
    // moves down; a row inserted above them (they all move together) does not.
    void aChartIsDrawnAgainOnlyWhenItsNumbersChange()
    {
        const QVector<SeriesSpec> series = {SeriesSpec{QStringLiteral("Sheet1!$B$1"), QStringLiteral("Sheet1!$A$2:$A$4"), QStringLiteral("Sheet1!$B$2:$B$4"), QStringLiteral("East"), {"Q1", "Q2", "Q3"}, {10, 20, 30}, QString()}};
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(twoCell(4, 8, 11, 18, chartFrame(2, QStringLiteral("rIdC"), QStringLiteral("Sales")))), {chartTarget(QStringLiteral("rIdC"), 1, chartPart(QStringLiteral("barChart"), series))})));
        QTRY_VERIFY(shown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        QTRY_COMPARE(o->layer()->pendingRenders(), 0);
        const qint64 first = o->chartsDrawn();
        QVERIFY(first > 0);
        const int height = o->layer()->deviceRect(0, grid::Pane::Body).height();
        QVERIFY(e.win.w.session()->insertRows(0, 0, 1).ok);
        QVERIFY(e.settle());
        o->layer()->prepareAll();
        QTRY_COMPARE(o->layer()->pendingRenders(), 0);
        QCOMPARE(o->chartsDrawn(), first);   // (the same three numbers, a row lower)
        QVERIFY(e.win.w.session()->insertRows(0, 3, 1).ok);   // (inside B3:B5, now where they are)
        QVERIFY(e.settle());
        QCOMPARE(o->layer()->deviceRect(0, grid::Pane::Body).height(), height);   // (the chart is below them: it moves, and is as large)
        QCOMPARE(o->chartNow(0, 0).plots[0].series[0].values.size(), 4);
        o->layer()->prepareAll();
        QTRY_COMPARE(o->layer()->pendingRenders(), 0);
        QVERIFY(o->chartsDrawn() > first);
    }

    // Each edit of rows is worked into the objects once, however many came before it (a hundred insertions are not a hundred times the work of the last), and Undo goes back over all.
    void manyEditsAreWorkedInOneAtATime()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        const qint64 before = o->editSteps();
        for (int i = 0; i < 40; ++i) QVERIFY(e.win.w.session()->insertRows(0, 20 + quint32(i), 1).ok);
        QCOMPARE(o->editSteps() - before, qint64(40 * 5));   // (five objects, each given each edit once)
        const grid::GridLayout &L = e.view()->sheet().layout();
        o->layer()->prepareAll();
        QCOMPARE(o->layer()->deviceRect(1, grid::Pane::Body).top(), L.cellRect(6, 0).top());
        pressUndo(e.view());
        QCOMPARE(o->layer()->deviceRect(2, grid::Pane::Body).top(), L.cellRect(10, 0).top());
        // (an Undo is worked out from the objects as they were: each of the edits left, for each object)
        QCOMPARE(o->editSteps() - before, qint64(40 * 5 + 5 * 39));
    }

    // The program that saves a picture of its window and quits ends, with the window still open: the worker that calculates for the window is told to stop, or the program would wait
    // for it for ever as it ended (the hang was in the end of the program, after the picture was written).
    void aScreenshotRunEndsWithItsWindowOpen()
    {
        // (the program this build makes: its path is the build's, and the tests are not built without it; a test that did not run for want of it would say nothing)
        const QString exe = QStringLiteral(JC_CRUNCH_EXE);
        QVERIFY2(QFileInfo::exists(exe), qPrintable(exe));
        Folder folder;
        const QString book = folder.write(QStringLiteral("book.xlsx"), drawingBook());
        const QString png = folder.dir.filePath(QStringLiteral("shot.png"));
        QProcess child;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
        env.insert(QStringLiteral("JP_NO_TELEMETRY"), QStringLiteral("1"));
        child.setProcessEnvironment(env);
        child.start(exe, {QStringLiteral("--screenshot"), png, QStringLiteral("--size"), QStringLiteral("700x500"), book});
        QVERIFY(child.waitForStarted());
        // (a program that does not end is told by this bound; one that is right ends in a few seconds)
        const bool ended = child.waitForFinished(120000);
        if (!ended) {
            child.kill();
            child.waitForFinished();
        }
        QVERIFY2(ended, "the program did not end after saving its picture");
        QCOMPARE(child.exitStatus(), QProcess::NormalExit);
        QCOMPARE(child.exitCode(), 0);
        const QImage img(png);
        QVERIFY(!img.isNull());
        QVERIFY(img.width() >= 700 && img.height() >= 500);
    }

    // A chart sheet prints as one page with its chart over the room the page has inside its margins (the page setup of the chart sheet is read: the paper, its turn, the margins), and
    // the page after the cells' pages when the whole workbook is printed.
    void aChartSheetPrintsAsOnePageWithItsChart()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithChartSheet(QStringLiteral("<pageMargins left=\"1\" right=\"1\" top=\"1\" bottom=\"1\" header=\"0.3\" footer=\"0.3\"/><pageSetup orientation=\"landscape\"/>"))));
        CrunchWindow &w = e.win.w;
        w.setActiveSheet(1);
        QTRY_VERIFY(shown(e, 1, 1));
        QString why;
        std::unique_ptr<crunch::PrintJob> job = w.makePrintJob(crunch::PrintJobOptions(), &why);
        QVERIFY2(job, qPrintable(why));
        QCOMPARE(job->pageCount(), 1);
        QCOMPARE(job->pageSize(0), QSizeF(792, 612));   // (turned to landscape, as the chart sheet says)
        QImage img = job->preview(0, 792);
        // the chart's bars are inside the 72-point margins, and the margins are white
        auto ink = [&](const QRect &r) {
            int n = 0;
            for (int y = r.top(); y < r.bottom(); ++y)
                for (int x = r.left(); x < r.right(); ++x) n += qGray(img.pixel(x, y)) < 235;
            return n;
        };
        QVERIFY(ink(QRect(72, 72, 648, 468)) > 500);
        QCOMPARE(ink(QRect(0, 0, 792, 70)), 0);
        QCOMPARE(ink(QRect(0, 0, 70, 612)), 0);
        QCOMPARE(ink(QRect(722, 0, 70, 612)), 0);
        // the whole workbook: the cells' pages, then this one
        crunch::PrintJobOptions all;
        all.what = crunch::PrintWhat::Workbook;
        job = w.makePrintJob(all, &why);
        QVERIFY2(job, qPrintable(why));
        QCOMPARE(job->sheetOf(job->pageCount() - 1), 1);
        QVERIFY(job->pageCount() >= 2);
    }

    // A drawing that is parsed on a worker, and asked about before it is done (the sheet's counts want it now): it is listed there and then, the worker's answer is let go, and the
    // objects still come into the layer when the answer arrives (they were wanted on the screen).
    void aDrawingBeingParsedIsListedWhenAskedAndStillShown()
    {
        QString many;
        const int n = 3000;
        for (int i = 0; i < n; ++i) many += twoCell(i % 40, i % 300, i % 40 + 1, i % 300 + 1, shapeElement(2 + i, QStringLiteral("S"), QStringLiteral("rect"), QStringLiteral("some words to make it larger than a few bytes")));
        const QByteArray bytes = bookWithDrawing({dataSheet()}, drawingPart(many), {});
        Folder folder;
        auto book = std::make_shared<office::sheet::Workbook>();
        QString why;
        QVERIFY2(book->openFile(folder.write(QStringLiteral("many.xlsx"), bytes), &why), qPrintable(why));
        crunch::SheetObjects objects;
        objects.open(book);
        objects.show(0);   // (the parsing is given to a worker; the layer is empty and the sheet is the one shown)
        QCOMPARE(objects.shownSheet(), 0);
        QCOMPARE(objects.layer()->count(), 0);
        QVERIFY(!objects.listed(0));
        const crunch::DrawingCounts c = objects.counts(0);   // (wants the drawing now: listed here, with the events not yet run)
        QCOMPARE(c.shapes, n);
        QVERIFY(objects.listed(0));
        QCOMPARE(objects.drawingsParsedApart(), 0);
        QTRY_COMPARE(objects.layer()->count(), n);   // (the worker's answer is let go, and the objects come in)
        QCOMPARE(objects.drawingsParsedApart(), 0);
        // the window says the sheet's rows and columns are whole (a workbook that is read whole) while the drawing is still being parsed: that does not list it on the window's thread
        crunch::SheetObjects third;
        third.open(book);
        third.show(0);
        third.geometryKnown(0);
        QVERIFY(!third.listed(0));
        QTRY_COMPARE(third.layer()->count(), n);
        QCOMPARE(third.drawingsParsedApart(), 1);
        // and a workbook opened again meanwhile is not given the answer of the one before (this one has fewer shapes, and is waited for as well)
        QString fewer;
        for (int i = 0; i < 2000; ++i) fewer += twoCell(i % 40, i % 300, i % 40 + 1, i % 300 + 1, shapeElement(2 + i, QStringLiteral("S"), QStringLiteral("rect"), QStringLiteral("some words to make it larger than a few bytes")));
        crunch::SheetObjects second;
        second.open(book);
        second.show(0);
        auto other = std::make_shared<office::sheet::Workbook>();
        QVERIFY2(other->openFile(folder.write(QStringLiteral("other.xlsx"), bookWithDrawing({dataSheet()}, drawingPart(fewer), {})), &why), qPrintable(why));
        second.open(other);
        second.show(0);
        QThreadPool::globalInstance()->waitForDone();   // (the worker has parsed both drawings, and posted its answers: the first is for the workbook before)
        QTRY_COMPARE(second.layer()->count(), 2000);
        QCOMPARE(second.drawingsParsedApart(), 1);
    }

    // A read that is told to have failed after the first screen was shown (the loader says so only before it, today, but the window must not depend on that) leaves no object over the
    // blank grid it goes back to, and no cell cursor missing.
    void aFailureAfterTheFirstScreenLeavesNoObjects()
    {
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, e.view()->sheet().layout().rangeRect(6, 0, 8, 1).center());
        QCOMPARE(o->layer()->selectedId(), 1);
        Q_EMIT e.win.w.workbookLoader().finished(false, QStringLiteral("The file ends too soon."));
        QCOMPARE(o->layer()->count(), 0);
        QCOMPARE(o->layer()->selectedId(), -1);
        QCOMPARE(o->shownSheet(), -1);
        QVERIFY(e.view()->cellsEnabled());
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY2(e.win.told[0].contains(QLatin1String("ends too soon")), qPrintable(e.win.told[0]));
        shot(e);
        QVERIFY(!e.win.w.hasWorkbook());
    }

    // Two objects the file keeps in one element (a wrapper with two shapes in it) are not taken away one at a time: Delete says why, and both stay.
    void objectsKeptTogetherAreNotDeletedApart() 
    {
        const QString wrapped = QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"a14\">%1%2</mc:Choice><mc:Fallback/></mc:AlternateContent>")
                                    .arg(twoCell(0, 2, 2, 4, shapeElement(2, QStringLiteral("Left"), QStringLiteral("rect"), QString(), QStringLiteral("CC3333"))),
                                         twoCell(3, 2, 5, 4, shapeElement(3, QStringLiteral("Right"), QStringLiteral("rect"), QString(), QStringLiteral("33CC33"))));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(twoCell(0, 8, 2, 10, shapeElement(4, QStringLiteral("Alone"))) + wrapped), {})));
        QTRY_VERIFY(shown(e, 3));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        const grid::GridLayout &L = e.view()->sheet().layout();
        QString why;
        QVERIFY(o->canDelete(0, 0, &why));   // (the one that is alone)
        QVERIFY(!o->canDelete(0, 1, &why));
        QVERIFY2(why.contains(QLatin1String("together")), qPrintable(why));
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, L.rangeRect(2, 0, 3, 1).center());
        QCOMPARE(o->layer()->selectedId(), 1);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY2(e.win.told[0].contains(QLatin1String("together")), qPrintable(e.win.told[0]));
        QVERIFY(!o->isDeleted(0, 1));
        QVERIFY(!o->isDeleted(0, 2));
        QVERIFY(!e.win.w.isModified());
        QCOMPARE(o->layer()->count(), 3);
        // the one that is alone is taken away as before
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, L.rangeRect(8, 0, 9, 1).center());
        QCOMPARE(o->layer()->selectedId(), 0);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(o->isDeleted(0, 0));
        QCOMPARE(e.win.told.size(), 1);
    }

    // Not a test of its own: the work of a process that the next function starts. A workbook with a chart, a picture, and shapes has a cell typed over and a shape deleted, is saved.
    void savesTheWorkbookWithObjects()
    {
        QString out;
        if (!stable::childOutput(&out)) return;
        EditWindow e;
        QVERIFY(e.open(drawingBook()));
        QTRY_VERIFY(shown(e, 5));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        const grid::GridLayout &L = e.view()->sheet().layout();
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, L.rangeRect(6, 0, 8, 1).center());
        QCOMPARE(o->layer()->selectedId(), 1);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        e.select(QStringLiteral("B3"));
        e.enter(QStringLiteral("55"));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        QFile in(e.path);
        QVERIFY(in.open(QIODevice::ReadOnly));
        QFile f(out);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(in.readAll());
    }

    // The workbook is the same bytes each time it is saved, in whatever process: nothing written follows the order of a hash that Qt seeds anew for each process.
    void aSavedWorkbookIsTheSameBytesInEachProcess() { JP_VERIFY_SAME_SAVES("savesTheWorkbookWithObjects"); }

    // A diagram of a sheet is drawn from the drawing its file keeps (the frame's own size is 0, so the anchor gives the size), is picked as one object, and is saved with every part as it was.
    void aDiagramIsShownFromItsSavedDrawingPickedAndKeptWhole()
    {
        EditWindow e;
        const QByteArray original = diagramBook();
        QVERIFY(e.open(original));
        QTRY_VERIFY(shown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        const crunch::DrawingCounts c = o->counts(0);
        QCOMPARE(c.diagrams, 1);
        QCOMPARE(c.shown, 1);
        QCOMPARE(c.leftOut, 0);
        QCOMPARE(c.skipped, 0);
        o->layer()->prepareAll();
        const QImage img = shot(e);
        const grid::GridLayout &L = e.view()->sheet().layout();
        const QRect box = L.rangeRect(6, 0, 13, 4);
        // the second shape's blue and the third's green, where the drawing puts them in the anchor's box
        auto near = [&](double fx, double fy, const QColor &want) {
            const QColor got = QColor::fromRgb(at(img, box.left() + int(box.width() * fx), box.top() + int(box.height() * fy)));
            return std::abs(got.red() - want.red()) < 6 && std::abs(got.green() - want.green()) < 6 && std::abs(got.blue() - want.blue()) < 6;
        };
        QVERIFY2(near(0.37, 0.65, QColor(0x33, 0x66, 0x99)), "the blue shape");
        QVERIFY2(near(0.72, 0.65, QColor(0x20, 0xA0, 0x40)), "the green shape");
        // picked as one object
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, box.center());
        QCOMPARE(o->layer()->selectedId(), 0);
        // a cell is typed over and the file saved: every part of the diagram is as it was
        e.select(QStringLiteral("B3"));
        e.enter(QStringLiteral("55"));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        office::Package before, after;
        QString why;
        QVERIFY(before.readLazy(original, &why));
        QVERIFY2(after.readLazyFile(e.path, &why), qPrintable(why));
        for (const char *name : {"data1", "layout1", "quickStyle1", "colors1", "drawing1"}) {
            const QString part = QStringLiteral("/xl/diagrams/%1.xml").arg(QLatin1String(name));
            QVERIFY2(after.hasPart(part), qPrintable(part));
            QCOMPARE(after.part(part), before.part(part));
        }
        QCOMPARE(after.relationships(QStringLiteral("/xl/drawings/drawing1.xml")).size(), 5);
        QVERIFY(after.part(QStringLiteral("/xl/drawings/drawing1.xml")).contains("r:dm=\"rIdDm\""));
    }

    // A diagram taken away and the file saved comes back with Undo whole: its four parts, its drawing (the data part names its link, which is a new id), and the frame.
    void aDeletedDiagramComesBackWholeAfterASaveAndUndo()
    {
        EditWindow e;
        QVERIFY(e.open(diagramBook()));
        QTRY_VERIFY(shown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        o->layer()->select(0);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(o->isDeleted(0, 0));
        QVERIFY(saveWindow(e));
        QVERIFY(reachedBy(e.path, QStringLiteral("diagramData")).isEmpty());
        QVERIFY(reachedBy(e.path, QStringLiteral("diagramDrawing")).isEmpty());
        QVERIFY2(e.win.w.session()->canUndo(), "Undo is offered for a delete that is in the saved file");
        pressUndo(e.view());
        QVERIFY(!o->isDeleted(0, 0));
        QVERIFY(saveWindow(e));
        const QVector<Reached> data = reachedBy(e.path, QStringLiteral("diagramData")), drawing = reachedBy(e.path, QStringLiteral("diagramDrawing"));
        QCOMPARE(int(data.size()), 1);
        QCOMPARE(int(drawing.size()), 1);
        QCOMPARE(drawing[0].bytes, diagramtest::diagramDrawingXml());
        for (const char *kind : {"diagramLayout", "diagramQuickStyle", "diagramColors"}) QCOMPARE(int(reachedBy(e.path, QLatin1String(kind)).size()), 1);
        // the data part names the drawing's link as it is now: the link of the saved drawing part that leads to the drawing
        office::Package p;
        QString why;
        QVERIFY2(p.readLazyFile(e.path, &why), qPrintable(why));
        const QString id = office::diagramDrawingLinkId(data[0].bytes);
        QVERIFY(!id.isEmpty());
        const office::Relationship link = p.relationship(QStringLiteral("/xl/drawings/drawing1.xml"), id);
        QVERIFY2(link.type.endsWith(QLatin1String("/diagramDrawing")) && link.target == drawing[0].part, qPrintable(id));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 1);
        QCOMPARE(second.w.objects()->counts(0).diagrams, 1);
        QVERIFY(second.w.objects()->diagramPicture(0, 0).toLower().contains("336699"));
    }

    // A diagram moved or sized by keys is one step of Undo each, and its parts are as they were when the file is saved.
    void aDiagramMovedAndSizedIsUndone()
    {
        EditWindow e;
        QVERIFY(e.open(diagramBook()));
        QTRY_VERIFY(shown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        o->layer()->prepareAll();
        o->layer()->select(0);
        shot(e);
        auto rect = [&] {
            shot(e);
            return o->layer()->deviceRect(0, grid::Pane::Body);
        };
        rect();
        QApplication::processEvents();
        const QRect was = rect();
        QVERIFY(!was.isEmpty());
        QVERIFY(o->layer()->nudge(5, 3, false, false));
        QCOMPARE(rect().topLeft(), was.topLeft() + QPoint(30, 18));
        QCOMPARE(rect().size(), was.size());
        QVERIFY(e.win.w.isModified());
        pressUndo(&e.win.w);
        QCOMPARE(rect(), was);
        QVERIFY(!e.win.w.isModified());
        QVERIFY(o->layer()->nudge(5, 3, true, false));
        QCOMPARE(rect().size(), was.size() + QSize(30, 18));
        pressUndo(&e.win.w);
        QCOMPARE(rect(), was);
        pressRedo(&e.win.w);
        QCOMPARE(rect().size(), was.size() + QSize(30, 18));
        QVERIFY(saveWindow(e));
        const QVector<Reached> drawing = reachedBy(e.path, QStringLiteral("diagramDrawing"));
        QCOMPARE(int(drawing.size()), 1);
        QCOMPARE(drawing[0].bytes, diagramtest::diagramDrawingXml());
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QTRY_VERIFY(second.w.objects()->shownSheet() == 0 && second.w.objects()->layer()->count() == 1);
        second.w.objects()->layer()->prepareAll();
        second.w.objects()->layer()->select(0);
        second.w.grab();
        QCOMPARE(second.w.objects()->layer()->deviceRect(0, grid::Pane::Body).size(), was.size() + QSize(30, 18));
    }

    // A diagram whose file keeps no drawing is a box that says what it is, and its parts are kept all the same.
    void aDiagramWithoutADrawingIsALabeledBoxAndKept()
    {
        EditWindow e;
        QVERIFY(e.open(diagramBook(false)));
        QTRY_VERIFY(shown(e, 1));
        crunch::SheetObjects *o = e.win.w.objects();
        const crunch::DrawingCounts c = o->counts(0);
        QCOMPARE(c.diagrams, 1);
        QCOMPARE(c.shown, 1);
        QCOMPARE(c.leftOut, 0);
        o->layer()->prepareAll();
        const QImage img = shot(e);
        const grid::GridLayout &L = e.view()->sheet().layout();
        const QRect box = L.rangeRect(6, 0, 13, 4);
        const QColor pale = QColor::fromRgb(at(img, box.left() + 12, box.top() + 12));
        QVERIFY2(pale.red() > 225 && pale.red() < 250 && pale.green() == pale.red(), qPrintable(pale.name()));
        e.select(QStringLiteral("B3"));
        e.enter(QStringLiteral("56"));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        const QMap<QString, QByteArray> parts = savedParts(e.path);
        QVERIFY(parts.contains(QStringLiteral("xl/diagrams/data1.xml")) && parts.contains(QStringLiteral("xl/diagrams/layout1.xml")));
    }

    // A drawing nested far deeper than a diagram is, one with a great many shapes, and more diagrams than a workbook draws, neither hang nor crash; each is capped, and every part is kept.
    void aHostileDiagramDrawingIsCapped()
    {
        using namespace diagramtest;
        QString deep;
        for (int i = 0; i < 20000; ++i) deep += QStringLiteral("<dsp:grpSp><dsp:nvGrpSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvGrpSpPr/></dsp:nvGrpSpPr><dsp:grpSpPr/>");
        deep += diagramShapeXml(0, 0, 914400, 914400, QStringLiteral("rect"), QStringLiteral("<a:solidFill><a:srgbClr val=\"336699\"/></a:solidFill>"), QStringLiteral("x"));
        for (int i = 0; i < 20000; ++i) deep += QStringLiteral("</dsp:grpSp>");
        {
            EditWindow e;
            QVERIFY(e.open(diagramBook(true, diagramDrawingXml(deep))));
            QTRY_VERIFY(shown(e, 1));
            crunch::SheetObjects *o = e.win.w.objects();
            QCOMPARE(o->counts(0).diagrams, 1);
            o->layer()->prepareAll();
            QVERIFY(!shot(e).isNull());
        }
        {
            QString many;
            for (int i = 0; i < 30000; ++i)
                many += QStringLiteral("<dsp:sp><dsp:nvSpPr><dsp:cNvPr id=\"0\" name=\"\"/><dsp:cNvSpPr/></dsp:nvSpPr><dsp:spPr><a:xfrm><a:off x=\"%1\" y=\"%2\"/><a:ext cx=\"9144\" cy=\"9144\"/></a:xfrm>"
                                       "<a:prstGeom prst=\"rect\"/><a:solidFill><a:srgbClr val=\"AB12CD\"/></a:solidFill></dsp:spPr></dsp:sp>").arg(i % 100 * 9144).arg(i / 100 * 9144);
            EditWindow e;
            QVERIFY(e.open(diagramBook(true, diagramDrawingXml(many))));
            QTRY_VERIFY(shown(e, 1));
            e.win.w.objects()->layer()->prepareAll();
            const QByteArray svg = e.win.w.objects()->diagramPicture(0, 0).toLower();
            QVERIFY(!svg.isEmpty());
            QVERIFY2(svg.count("ab12cd") <= jp::kMostDiagramShapes, qPrintable(QString::number(svg.count("ab12cd"))));
        }
        {
            // 230 diagrams, each with parts of its own: 200 are drawn from their drawings, the rest are boxes
            QString anchors;
            QVector<DrawingTarget> targets;
            for (int i = 1; i <= 230; ++i) {
                anchors += twoCell(0, 20 + i, 3, 21 + i, diagramFrame(i + 1, QStringLiteral("Diagram %1").arg(i), i > 1 ? QStringLiteral("rIdDm%1").arg(i) : QStringLiteral("rIdDm")));
                targets += diagramTargets(true, diagramDrawingXml(), i);
            }
            EditWindow e;
            QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), targets)));
            QTRY_VERIFY(shown(e, 230));
            crunch::SheetObjects *o = e.win.w.objects();
            const crunch::DrawingCounts c = o->counts(0);
            QCOMPARE(c.diagrams, 230);
            QCOMPARE(c.shown, 230);
            QVERIFY2(o->diagramsDrawn() <= crunch::kMaxDiagramsDrawn && o->diagramsDrawn() > 0, qPrintable(QString::number(o->diagramsDrawn())));
            // (the cap comes first: no more data parts are inflated than diagrams may be drawn)
            QVERIFY2(o->diagramPartsRead() <= crunch::kMaxDiagramsDrawn, qPrintable(QString::number(o->diagramPartsRead())));
        }
    }

    // ---- a shape or text box that shows a cell's text (v1-shapelink)
    // A shape inserted and picked; its id.
    static int pickedNewShape(EditWindow &e)
    {
        if (!e.win.w.insertShape(QStringLiteral("rect"))) return -1;
        return e.win.w.objects()->layer()->selectedId();
    }
    static QByteArray savedDrawing(const QString &path)
    {
        const QMap<QString, QByteArray> parts = savedParts(path);
        for (auto it = parts.begin(); it != parts.end(); ++it)
            if (it.key().startsWith(QLatin1String("xl/drawings/")) && it.key().endsWith(QLatin1String(".xml")) && !it.key().contains(QLatin1String("_rels"))) return it.value();
        return {};
    }
    static QLineEdit *linkBar(EditWindow &e) { return e.win.w.findChild<QLineEdit *>(QStringLiteral("formulaBar")); }
    // Types an entry in the formula bar as a person does, and presses Enter.
    static void typeLink(EditWindow &e, const QString &text)
    {
        QLineEdit *bar = linkBar(e);
        bar->setFocus();
        bar->selectAll();
        QTest::keyClick(bar, Qt::Key_Delete);
        if (!text.isEmpty()) QTest::keyClicks(bar, text);
        QTest::keyClick(bar, Qt::Key_Return);
    }
    static QString linkOf(EditWindow &e) { return e.win.w.objects()->textLinkOf(0, e.win.w.objects()->layer()->selectedId()); }
    static QString shapeText(EditWindow &e) { return e.win.w.objects()->linkedTextOf(0, e.win.w.objects()->layer()->selectedId()); }
    static void letEventsRun()
    {
        for (int i = 0; i < 3; ++i) QCoreApplication::processEvents();
    }
    static QByteArray twoSheets() { return buildXlsx({dataSheet(), dataSheet()}, QByteArray(), QString(), QString(), stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/>"))); }
    // A shape of a file with the textlink it has and the words it stored.
    static QString linkedShape(int id, const QString &link, const QString &words, const QString &name = QString())
    {
        return shapeElement(id, name.isEmpty() ? QStringLiteral("S%1").arg(id) : name, QStringLiteral("rect"), words).replace(QStringLiteral("textlink=\"\""), QStringLiteral("textlink=\"%1\"").arg(link.toHtmlEscaped()));
    }
    static QByteArray fileDrawing(const QByteArray &book)
    {
        QMap<QString, QByteArray> parts;
        jp::readZip(book, parts);
        return parts.value(QStringLiteral("xl/drawings/drawing1.xml"));
    }
    static bool saveIt(EditWindow &e)
    {
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        return e.win.w.save() && savedOk(saved);
    }

    // =B2 in the formula bar with a shape picked links it: the bar shows the link, the shape shows the cell's text, and the whole is one step of Undo.
    void aShapeIsLinkedByTheFormulaBar()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({dataSheet()})));
        QVERIFY(e.settle());
        QTRY_VERIFY(shown(e, 0));
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(pickedNewShape(e) >= 0);
        QCOMPARE(linkBar(e)->text(), QString());
        const int steps = e.win.w.undoStack()->count();
        typeLink(e, QStringLiteral("=B2"));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(linkOf(e), QStringLiteral("Sheet1!$B$2"));
        QCOMPARE(shapeText(e), QStringLiteral("10"));
        QCOMPARE(linkBar(e)->text(), QStringLiteral("=Sheet1!$B$2"));
        QCOMPARE(int(o->idsOf(0).size()), 1);
        // (the cell under the shape was not edited)
        QCOMPARE(e.shownAt(QStringLiteral("A1")), QStringLiteral("Region"));
        // an empty entry takes the link off, the words stay
        typeLink(e, QString());
        QCOMPARE(linkOf(e), QString());
        QCOMPARE(e.win.w.undoStack()->count(), steps + 2);
        QCOMPARE(linkBar(e)->text(), QString());
        e.win.w.undoStack()->undo();
        QCOMPARE(e.win.w.objects()->textLinkOf(0, e.win.w.objects()->idsOf(0).front()), QStringLiteral("Sheet1!$B$2"));
        QCOMPARE(e.win.w.objects()->linkedTextOf(0, e.win.w.objects()->idsOf(0).front()), QStringLiteral("10"));
    }

    // A row inserted above a linked shape's cell moves the link with it.
    void aRowInsertedAboveMovesTheLink()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({dataSheet()})));
        QVERIFY(e.settle());
        QTRY_VERIFY(shown(e, 0));
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(pickedNewShape(e) >= 0);
        typeLink(e, QStringLiteral("=B3"));
        QVERIFY(e.win.w.session()->insertRows(0, 0, 2).ok);
        QCOMPARE(o->textLinkOf(0, o->idsOf(0).front()), QStringLiteral("Sheet1!$B$5"));
        QTRY_COMPARE(o->linkedTextOf(0, o->idsOf(0).front()), QStringLiteral("20"));
        QVERIFY(e.win.w.session()->type(0, 4, 1, QStringLiteral("moved")).ok);
        QTRY_COMPARE(o->linkedTextOf(0, o->idsOf(0).front()), QStringLiteral("moved"));
    }

    // The shape follows its cell: a typed value, the cell's format, a recalculation, and a cell of another sheet.
    void aLinkedShapeFollowsItsCell()
    {
        EditWindow e;
        QVERIFY(e.open(twoSheets()));
        QVERIFY(e.settle());
        QTRY_VERIFY(shown(e, 0));
        auto *s = e.win.w.session();
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(pickedNewShape(e) >= 0);
        typeLink(e, QStringLiteral("=B2"));
        QCOMPARE(shapeText(e), QStringLiteral("10"));
        const qint64 updates = o->linkUpdates();
        QVERIFY(s->type(0, 1, 1, QStringLiteral("42")).ok);
        QTRY_COMPARE(shapeText(e), QStringLiteral("42"));
        QVERIFY(o->linkUpdates() > updates);
        // its format
        const int shapeId = o->idsOf(0).front();
        o->layer()->select(-1);
        e.select(QStringLiteral("B2"));
        QVERIFY(e.win.w.act(QStringLiteral("fmt.percent"))->isEnabled());
        e.win.w.act(QStringLiteral("fmt.percent"))->trigger();
        o->layer()->select(shapeId);
        QTRY_COMPARE(shapeText(e), QStringLiteral("4200%"));
        // a recalculation: the shape of a formula's cell
        QVERIFY(s->type(0, 0, 3, QStringLiteral("=B2*2")).ok);
        QVERIFY(e.settle());
        o->layer()->select(o->idsOf(0).front());
        typeLink(e, QStringLiteral("=D1"));
        QTRY_COMPARE(shapeText(e), QStringLiteral("84"));
        QVERIFY(s->type(0, 1, 1, QStringLiteral("5")).ok);
        QVERIFY(e.settle());
        QTRY_COMPARE(shapeText(e), QStringLiteral("10"));
        // a cell of another sheet
        typeLink(e, QStringLiteral("=Sheet2!B3"));
        QCOMPARE(linkOf(e), QStringLiteral("Sheet2!$B$3"));
        QCOMPARE(shapeText(e), QStringLiteral("20"));
        QVERIFY(s->type(1, 2, 1, QStringLiteral("hello")).ok);
        QTRY_COMPARE(shapeText(e), QStringLiteral("hello"));
        // a refused entry leaves the link, with a word
        QStringList told;
        e.win.w.setNotifier([&](const QString &, const QString &text) { told << text; });
        typeLink(e, QStringLiteral("=A1:B2"));
        QCOMPARE(told.size(), 1);
        typeLink(e, QStringLiteral("A1"));
        QCOMPARE(told.size(), 2);
        typeLink(e, QStringLiteral("=Nowhere!A1"));
        QCOMPARE(told.size(), 3);
        QCOMPARE(linkOf(e), QStringLiteral("Sheet2!$B$3"));
        QCOMPARE(e.win.w.objects()->idsOf(0).size(), size_t(1));
    }

    // A name that stands for one cell links too.
    void aLinkCanBeAName()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(QString()), {}, QStringList(), QStringLiteral("<definedName name=\"Total\">Sheet1!$B$3</definedName>"))));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        QVERIFY(pickedNewShape(e) >= 0);
        typeLink(e, QStringLiteral("=Total"));
        QCOMPARE(linkOf(e), QStringLiteral("Total"));
        QCOMPARE(shapeText(e), QStringLiteral("20"));
    }

    // Words typed in the shape take it off its cell; Undo puts the link back, Redo takes it off again.
    void typingInAShapeRemovesItsLink()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({dataSheet()})));
        QVERIFY(e.settle());
        QTRY_VERIFY(shown(e, 0));
        QVERIFY(pickedNewShape(e) >= 0);
        typeLink(e, QStringLiteral("=B2"));
        QCOMPARE(linkOf(e), QStringLiteral("Sheet1!$B$2"));
        CrunchWindow::ShapeTextChoices c;
        c.text = QStringLiteral("typed words");
        QVERIFY(e.win.w.setObjectText(c));
        QCOMPARE(linkOf(e), QString());
        QCOMPARE(e.win.w.objects()->idsOf(0).size(), size_t(1));
        QVERIFY(e.win.w.session()->type(0, 1, 1, QStringLiteral("77")).ok);
        letEventsRun();
        QCOMPARE(shapeText(e), QString());
        e.win.w.undoStack()->undo();   // (the typed value)
        e.win.w.undoStack()->undo();
        QCOMPARE(e.win.w.objects()->textLinkOf(0, e.win.w.objects()->idsOf(0).front()), QStringLiteral("Sheet1!$B$2"));
        e.win.w.undoStack()->redo();
        QCOMPARE(e.win.w.objects()->textLinkOf(0, e.win.w.objects()->idsOf(0).front()), QString());
    }

    // Undo and Redo of the link, the shape being as its cell is at each moment.
    void aLinkUndoesAndRedoes()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({dataSheet()})));
        QVERIFY(e.settle());
        QTRY_VERIFY(shown(e, 0));
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(pickedNewShape(e) >= 0);
        typeLink(e, QStringLiteral("=B2"));
        QVERIFY(e.win.w.session()->type(0, 1, 1, QStringLiteral("31")).ok);
        QTRY_COMPARE(o->linkedTextOf(0, o->idsOf(0).front()), QStringLiteral("31"));
        e.win.w.undoStack()->undo();   // (the typed value)
        QTRY_COMPARE(o->linkedTextOf(0, o->idsOf(0).front()), QStringLiteral("10"));
        e.win.w.undoStack()->undo();   // (the link)
        QCOMPARE(o->textLinkOf(0, o->idsOf(0).front()), QString());
        e.win.w.undoStack()->redo();
        QCOMPARE(o->textLinkOf(0, o->idsOf(0).front()), QStringLiteral("Sheet1!$B$2"));
        QCOMPARE(o->linkedTextOf(0, o->idsOf(0).front()), QStringLiteral("10"));
        e.win.w.undoStack()->redo();
        QTRY_COMPARE(o->linkedTextOf(0, o->idsOf(0).front()), QStringLiteral("31"));
    }

    // The cell's row deleted: the shape shows #REF! and keeps its link, as a formula would, in the window and in the saved file.
    void aDeletedRowLeavesRefInTheShape()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({dataSheet()})));
        QVERIFY(e.settle());
        QTRY_VERIFY(shown(e, 0));
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(pickedNewShape(e) >= 0);
        typeLink(e, QStringLiteral("=B3"));
        QVERIFY(e.win.w.session()->deleteRows(0, 2, 1).ok);
        QTRY_COMPARE(o->linkedTextOf(0, o->idsOf(0).front()), QStringLiteral("#REF!"));
        QVERIFY(o->textLinkOf(0, o->idsOf(0).front()).contains(QLatin1String("#REF!")));
        // (Undo brings the cell, and the link to it, back)
        e.win.w.undoStack()->undo();
        QTRY_COMPARE(o->linkedTextOf(0, o->idsOf(0).front()), QStringLiteral("20"));
        QCOMPARE(o->textLinkOf(0, o->idsOf(0).front()), QStringLiteral("Sheet1!$B$3"));
        e.win.w.undoStack()->redo();
        QTRY_COMPARE(o->linkedTextOf(0, o->idsOf(0).front()), QStringLiteral("#REF!"));
        QVERIFY(saveIt(e));
        const QByteArray drawing = savedDrawing(e.path);
        QVERIFY2(drawing.contains("#REF!"), drawing.left(600).constData());
    }

    // Saved, the shape has the textlink and the text its cell showed; opened again it is linked, shows the cell, and follows it; saved again after the cell changed, the file has the new text.
    void aLinkedShapeSurvivesSaveAndReopen()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({dataSheet()})));
        QVERIFY(e.settle());
        QTRY_VERIFY(shown(e, 0));
        QVERIFY(pickedNewShape(e) >= 0);
        typeLink(e, QStringLiteral("=B2"));
        QVERIFY(saveIt(e));
        QByteArray drawing = savedDrawing(e.path);
        QVERIFY2(drawing.contains("textlink=\"Sheet1!$B$2\""), drawing.left(800).constData());
        QVERIFY2(drawing.contains(">10</a:t>"), drawing.mid(drawing.indexOf("txBody") - 20, 900).constData());
        EditWindow again;
        QVERIFY(again.open([&] {
            QFile f(e.path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        }()));
        QVERIFY(again.settle());
        QTRY_VERIFY(shown(again, 1));
        crunch::SheetObjects *o = again.win.w.objects();
        const int id = o->idsOf(0).front();
        QCOMPARE(o->textLinkOf(0, id), QStringLiteral("Sheet1!$B$2"));
        QTRY_COMPARE(o->linkedTextOf(0, id), QStringLiteral("10"));
        QVERIFY(again.win.w.session()->type(0, 1, 1, QStringLiteral("55")).ok);
        QTRY_COMPARE(o->linkedTextOf(0, id), QStringLiteral("55"));
        QVERIFY(saveIt(again));
        drawing = savedDrawing(again.path);
        QVERIFY2(drawing.contains(">55</a:t>") && drawing.contains("textlink=\"Sheet1!$B$2\""), drawing.left(1200).constData());
        QVERIFY(!drawing.contains(">10</a:t>"));
        // (a part of the package is held once: the drawing is one part)
        int drawings = 0;
        for (const QString &k : savedParts(again.path).keys()) drawings += k.startsWith(QLatin1String("xl/drawings/drawing")) && k.endsWith(QLatin1String(".xml"));
        QCOMPARE(drawings, 1);
    }

    // A file made elsewhere: its shape with a textlink opens linked, with the text it stored until its cell's value is known, then the cell's.
    void aFileShapeWithATextlinkOpensLinked()
    {
        const QString shape = shapeElement(2, QStringLiteral("Note"), QStringLiteral("rect"), QStringLiteral("stored words"), QStringLiteral("3366CC")).replace(QStringLiteral("textlink=\"\""), QStringLiteral("textlink=\"$B$3\""));
        const QString plain = shapeElement(3, QStringLiteral("Plain"), QStringLiteral("rect"), QStringLiteral("kept"));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(twoCell(5, 1, 8, 4, shape) + twoCell(5, 6, 8, 9, plain)), {})));
        QTRY_VERIFY(shown(e, 2));
        crunch::SheetObjects *o = e.win.w.objects();
        QVERIFY(e.settle());
        const std::vector<int> ids = o->idsOf(0);
        QTRY_COMPARE(o->textLinkOf(0, ids[0]), QStringLiteral("$B$3"));
        QTRY_COMPARE(o->linkedTextOf(0, ids[0]), QStringLiteral("20"));
        QCOMPARE(o->textLinkOf(0, ids[1]), QString());
        QVERIFY(e.win.w.session()->type(0, 2, 1, QStringLiteral("changed")).ok);
        QTRY_COMPARE(o->linkedTextOf(0, ids[0]), QStringLiteral("changed"));
        // the formula bar of the picked shape shows its link
        o->layer()->select(ids[0]);
        QCOMPARE(linkBar(e)->text(), QStringLiteral("=$B$3"));
        // saved, the shape that was not linked is as it was (its bytes untouched), and the linked one has the new words
        QVERIFY(saveIt(e));
        const QByteArray drawing = savedDrawing(e.path);
        QVERIFY(drawing.contains(">changed</a:t>"));
        QVERIFY(drawing.contains(">kept</a:t>"));
        QVERIFY(drawing.contains("textlink=\"$B$3\""));
    }

    // Many linked shapes: a change of cells looks at the shapes of those cells alone (an index from cell to shapes), counted, not timed.
    void tenThousandLinkedShapesUpdateWithoutAScanOfThemAll()
    {
        const int n = 10000;
        QString rows, anchors;
        for (int i = 0; i < n; ++i) {
            rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%2</v></c></row>").arg(i + 1).arg(i);
            const QString shape = shapeElement(i + 2, QStringLiteral("S%1").arg(i), QStringLiteral("rect"), QStringLiteral("old")).replace(QStringLiteral("textlink=\"\""), QStringLiteral("textlink=\"Sheet1!$A$%1\"").arg(i + 1));
            anchors += twoCell(2 + i % 10, i / 10, 3 + i % 10, i / 10 + 1, shape);
        }
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({sheettest::worksheet(rows)}, drawingPart(anchors), {})));
        QTRY_VERIFY2(shown(e, n), "all the shapes are in the layer");
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        QCOMPARE(o->counts(0).shapes, n);
        QCOMPARE(o->linkedTextOf(0, o->idsOf(0).front()), QStringLiteral("0"));
        QCOMPARE(o->linkedTextOf(0, o->idsOf(0).back()), QString::number(n - 1));
        const qint64 visits = o->linkVisits(), updates = o->linkUpdates(), builds = o->linkIndexBuilds();
        for (int i = 0; i < 40; ++i) QVERIFY(e.win.w.session()->type(0, quint32(i * 250), 0, QStringLiteral("x%1").arg(i)).ok);
        QTRY_COMPARE(o->linkUpdates() - updates, qint64(40));
        // (each shape of a cell that changed was looked at once: no scan of the rest)
        QCOMPARE(o->linkVisits() - visits, qint64(40));
        QVERIFY2(o->linkIndexBuilds() - builds <= 1, qPrintable(QString::number(o->linkIndexBuilds() - builds)));
        QCOMPARE(o->linkedTextOf(0, o->idsOf(0)[250]), QStringLiteral("x1"));
        // (no shape was made again, and no chart was looked at: the scan of every object for a chart that reads the cells is gone)
        QCOMPARE(o->linkRebuilds(), qint64(0));
        QCOMPARE(o->readerVisits(), qint64(0));
    }

    // A textlink that is huge, a list, or a range is refused, with a note in the counts; the shape keeps its words; the text shown is cut to what a cell holds.
    void aHostileTextlinkIsRefusedWithANote()
    {
        const QString anchors = twoCell(5, 1, 7, 3, linkedShape(2, QString(300, QLatin1Char('A')) + QStringLiteral("1"), QStringLiteral("words"))) + twoCell(5, 4, 7, 6, linkedShape(3, QStringLiteral("Sheet1!A1:B2"), QStringLiteral("words")))
                                + twoCell(5, 7, 7, 9, linkedShape(4, QStringLiteral("A1,B2"), QStringLiteral("words")));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(anchors), {})));
        QTRY_VERIFY(shown(e, 3));
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        const crunch::DrawingCounts c = o->counts(0);
        QCOMPARE(c.linksRefused, 3);   // (counted when the sheet's objects are read, not when a shape is drawn)
        QVERIFY(!c.why.isEmpty());
        const std::vector<int> ids = o->idsOf(0);
        for (int i = 0; i < 3; ++i) QCOMPARE(o->textLinkOf(0, ids[size_t(i)]), QString());
        // the text shown is cut to what a cell holds
        const QString big = QString(40000, QLatin1Char('w'));
        QDomDocument doc;
        QVERIFY(doc.setContent(QStringLiteral("<xdr:sp xmlns:xdr=\"x\" xmlns:a=\"y\"><xdr:txBody><a:p><a:r><a:t>q</a:t></a:r></a:p></xdr:txBody></xdr:sp>")));
        QDomElement sp = doc.documentElement();
        jp::office::sheet::setShapeLinkText(sp, big);
        QCOMPARE(jp::office::sheet::shapeLinkStoredText(sp).size(), jp::office::sheet::kMaxShapeLinkText);
    }

    // A reference the workbook cannot resolve to one cell (a cell of another file, a name that holds a formula or a number, a sheet number that is huge, a sheet that is not there, a reference that
    // is #REF! in the file) is left as the file has it: no link, the words it stored, and the same part when the file is saved with nothing changed. #REF! is for a link that had a cell.
    void aLinkThatIsNotACellIsLeftAsTheFileHasIt()
    {
        const QStringList links = {QStringLiteral("[1]Sheet1!A1"), QStringLiteral("Formula"), QStringLiteral("Constant"), QStringLiteral("Sheet99999999999!A1"), QStringLiteral("Nope!A1"), QStringLiteral("Sheet1!#REF!")};
        QString anchors;
        for (int i = 0; i < links.size(); ++i) anchors += twoCell(5, 1 + 3 * i, 7, 3 + 3 * i, linkedShape(2 + i, links[i], QStringLiteral("words%1").arg(i)));
        const QByteArray book = bookWithDrawing({dataSheet()}, drawingPart(anchors), {}, QStringList(), QStringLiteral("<definedName name=\"Formula\">Sheet1!$B$2*2</definedName><definedName name=\"Constant\">5</definedName>"));
        const QByteArray original = fileDrawing(book);
        QVERIFY(original.contains("textlink=\"[1]Sheet1!A1\""));
        EditWindow e;
        QVERIFY(e.open(book));
        QTRY_VERIFY(shown(e, links.size()));
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        const std::vector<int> ids = o->idsOf(0);
        QCOMPARE(int(ids.size()), links.size());
        for (const int id : ids) {
            QCOMPARE(o->textLinkOf(0, id), QString());
            QCOMPARE(o->linkedTextOf(0, id), QString());
        }
        QCOMPARE(o->counts(0).linksRefused, 0);
        QVERIFY(e.win.w.session()->type(0, 1, 1, QStringLiteral("77")).ok);
        QVERIFY(e.settle());
        letEventsRun();
        for (const int id : ids) QCOMPARE(o->linkedTextOf(0, id), QString());
        QVERIFY(saveIt(e));
        // (a change of a cell does not write the shapes: the part is the file's)
        QCOMPARE(savedDrawing(e.path), original);
        // a row inserted moves the shapes' anchors, and the links and words stay as the file had them
        QVERIFY(e.win.w.session()->insertRows(0, 0, 1).ok);
        QVERIFY(saveIt(e));
        const QByteArray after = savedDrawing(e.path);
        for (int i = 0; i < links.size(); ++i) {
            QVERIFY2(after.contains(QStringLiteral("textlink=\"%1\"").arg(links[i]).toUtf8()), qPrintable(links[i]));
            QVERIFY(after.contains(QStringLiteral(">words%1</a:t>").arg(i).toUtf8()));
        }
    }

    // A link to a cell on a sheet that is never shown: it is in the index, follows the rows and columns put in and taken out, and is saved with the reference it has now and the words its cell shows.
    void aLinkOnASheetNeverShownFollowsEditsAndSaves()
    {
        const QByteArray book = bookWithDrawing({dataSheet(), dataSheet()}, drawingPart(twoCell(5, 1, 8, 4, linkedShape(2, QStringLiteral("Sheet2!$B$3"), QStringLiteral("stored words")))), {}, QStringList(), QString(), QByteArray(), 1);
        EditWindow e;
        QVERIFY(e.open(book));
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        QCOMPARE(o->shownSheet(), 0);
        auto *s = e.win.w.session();
        const std::vector<int> ids = o->idsOf(1);
        QCOMPARE(int(ids.size()), 1);
        QTRY_COMPARE(o->linkedTextOf(1, ids[0]), QStringLiteral("20"));
        QVERIFY(s->insertRows(1, 0, 2).ok);      // B5
        QVERIFY(s->insertColumns(1, 0, 1).ok);   // C5
        QVERIFY(s->insertColumns(1, 5, 1).ok);   // (after it: C5 still)
        QVERIFY(s->deleteColumns(1, 0, 1).ok);   // B5
        QVERIFY(s->insertRows(1, 0, 1).ok);      // B6
        QTRY_COMPARE(o->linkedTextOf(1, ids[0]), QStringLiteral("20"));
        QVERIFY(s->type(1, 5, 1, QStringLiteral("moved")).ok);
        QVERIFY(e.settle());
        QTRY_COMPARE(o->linkedTextOf(1, ids[0]), QStringLiteral("moved"));
        QCOMPARE(o->shownSheet(), 0);
        QVERIFY(saveIt(e));
        const QByteArray drawing = savedDrawing(e.path);
        QVERIFY2(drawing.contains("textlink=\"Sheet2!$B$6\""), drawing.left(900).constData());
        QVERIFY2(drawing.contains(">moved</a:t>"), drawing.left(900).constData());
        QVERIFY(!drawing.contains("stored words"));
        EditWindow again;
        QVERIFY(again.open([&] {
            QFile f(e.path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        }()));
        QVERIFY(again.settle());
        crunch::SheetObjects *o2 = again.win.w.objects();
        const std::vector<int> ids2 = o2->idsOf(1);
        QCOMPARE(int(ids2.size()), 1);
        QCOMPARE(o2->textLinkOf(1, ids2[0]), QStringLiteral("Sheet2!$B$6"));
        QTRY_COMPARE(o2->linkedTextOf(1, ids2[0]), QStringLiteral("moved"));
        // a row deleted above takes the shape's cell with it: #REF! in the window and in the file; and a rename of the sheet follows
        QVERIFY(again.win.w.session()->renameSheet(1, QStringLiteral("Data")).ok);
        QTRY_COMPARE(o2->linkedTextOf(1, ids2[0]), QStringLiteral("moved"));
        QVERIFY(again.win.w.session()->deleteRows(1, 5, 1).ok);
        QTRY_COMPARE(o2->linkedTextOf(1, ids2[0]), QStringLiteral("#REF!"));
        QVERIFY(saveIt(again));
        const QByteArray gone = savedDrawing(again.path);
        QVERIFY2(gone.contains("#REF!") && !gone.contains(">moved</a:t>"), gone.left(900).constData());
    }

    // A file's drawing is saved byte for byte when nothing changed: a shape that is not linked, and a linked one whose cell shows the words it stored (its "=" kept).
    void anUnchangedDrawingIsSavedByteForByte()
    {
        const QString anchors = twoCell(5, 1, 7, 3, linkedShape(2, QStringLiteral("=$B$3"), QStringLiteral("20"))) + twoCell(5, 4, 7, 6, shapeElement(3, QStringLiteral("Plain"), QStringLiteral("rect"), QStringLiteral("kept")))
                                + twoCell(5, 7, 7, 9, linkedShape(4, QStringLiteral("Sheet1!$B$2"), QStringLiteral("10")));
        const QByteArray book = bookWithDrawing({dataSheet()}, drawingPart(anchors), {});
        const QByteArray original = fileDrawing(book);
        EditWindow e;
        QVERIFY(e.open(book));
        QTRY_VERIFY(shown(e, 3));
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        const std::vector<int> ids = o->idsOf(0);
        QTRY_COMPARE(o->linkedTextOf(0, ids[0]), QStringLiteral("20"));
        QTRY_COMPARE(o->linkedTextOf(0, ids[2]), QStringLiteral("10"));
        QVERIFY(e.win.w.session()->type(0, 6, 5, QStringLiteral("1")).ok);   // (a cell no shape shows: the workbook is changed, and saved)
        QVERIFY(saveIt(e));
        QCOMPARE(savedDrawing(e.path), original);
        // the cell changes: the linked shape is written, the "=" is as the file had it, and the others' bytes stay
        QVERIFY(e.win.w.session()->type(0, 2, 1, QStringLiteral("21")).ok);
        QVERIFY(e.settle());
        QTRY_COMPARE(o->linkedTextOf(0, ids[0]), QStringLiteral("21"));
        QVERIFY(saveIt(e));
        const QByteArray changed = savedDrawing(e.path);
        QVERIFY(changed.contains("textlink=\"=$B$3\"") && changed.contains(">21</a:t>"));
        QVERIFY(changed.contains(">kept</a:t>") && changed.contains("textlink=\"Sheet1!$B$2\""));
    }

    // A shape copied with a link that has no sheet ($B$3) and put on another sheet still shows the cell of the sheet it came from.
    void aCopyOfALinkWithNoSheetKeepsItsSheet()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet(), dataSheet()}, drawingPart(twoCell(5, 1, 8, 4, linkedShape(2, QStringLiteral("$B$3"), QStringLiteral("stored")))), {})));
        QTRY_VERIFY(shown(e, 1));
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        const int id = o->idsOf(0).front();
        QTRY_COMPARE(o->linkedTextOf(0, id), QStringLiteral("20"));
        crunch::ObjectClip clip;
        QVERIFY(o->copyElement(0, id, &clip));
        QVERIFY2(clip.elementXml.contains("textlink=\"Sheet1!$B$3\""), clip.elementXml.left(500).constData());
        // and put on the other sheet by the commands
        o->layer()->select(id);
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        e.win.w.setActiveSheet(1);
        QTRY_VERIFY(shown(e, 0, 1));
        e.win.w.act(QStringLiteral("edit.paste"))->trigger();
        QTRY_VERIFY(shown(e, 1, 1));
        const std::vector<int> pasted = o->idsOf(1);
        QCOMPARE(int(pasted.size()), 1);
        QCOMPARE(o->textLinkOf(1, pasted[0]), QStringLiteral("Sheet1!$B$3"));
        QTRY_COMPARE(o->linkedTextOf(1, pasted[0]), QStringLiteral("20"));
    }

    // 10,000 shapes on one cell of 32,767 characters: a change of the cell gives each shape its text (counted) and makes no item or element again; the text they show in all is held to the bound.
    void aLongCellOnTenThousandShapesIsCappedAndUpdatesTextOnly()
    {
        const int n = 10000;
        QString anchors;
        for (int i = 0; i < n; ++i) anchors += twoCell(2 + i % 10, i / 10, 3 + i % 10, i / 10 + 1, linkedShape(i + 2, QStringLiteral("Sheet1!$A$1"), QStringLiteral("old")));
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({sheettest::worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"))}, drawingPart(anchors), {})));
        QTRY_VERIFY(shown(e, n));
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        const std::vector<int> ids = o->idsOf(0);
        QCOMPARE(int(ids.size()), n);
        for (int i = 0; i < 3; ++i) QCOMPARE(o->itemTextOf(0, ids[size_t(i)]), QStringLiteral("1"));   // (three are made, as if drawn)
        const qint64 rebuilds = o->linkRebuilds(), updates = o->linkUpdates(), scans = o->readerVisits();
        const QString big(jp::office::sheet::kMaxShapeLinkText, QLatin1Char('x'));
        QVERIFY(e.win.w.session()->type(0, 0, 0, big).ok);
        QTRY_COMPARE(o->linkUpdates() - updates, qint64(n));
        QCOMPARE(o->linkRebuilds() - rebuilds, qint64(0));
        QCOMPARE(o->readerVisits() - scans, qint64(0));
        QVERIFY2(o->linkedChars() <= crunch::SheetObjects::kMaxLinkedChars + n * crunch::SheetObjects::kMinLinkedChars, qPrintable(QString::number(o->linkedChars())));
        QVERIFY(o->linkedChars() >= crunch::SheetObjects::kMaxLinkedChars);   // (what fits is shown: the bound is reached, not far under it)
        QCOMPARE(o->linkedTextOf(0, ids.front()).size(), jp::office::sheet::kMaxShapeLinkText);
        QCOMPARE(qint64(o->linkedTextOf(0, ids.back()).size()), crunch::SheetObjects::kMinLinkedChars);
        // a shape that was made is made again when it is drawn, once, with its text; one not made yet is made from its element, which holds the cut text
        QCOMPARE(o->itemTextOf(0, ids[0]).size(), jp::office::sheet::kMaxShapeLinkText);
        QCOMPARE(o->linkRebuilds() - rebuilds, qint64(1));
        QCOMPARE(o->itemTextOf(0, ids.back()).size(), int(crunch::SheetObjects::kMinLinkedChars));
        QCOMPARE(o->linkRebuilds() - rebuilds, qint64(1));
    }

    // Characters XML cannot hold in a cell's text are left out of what the shape stores: the saved part reads, and the file opens.
    void controlCharactersInALinkedCellAreLeftOutOfTheDrawing()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({dataSheet()})));
        QVERIFY(e.settle());
        QTRY_VERIFY(shown(e, 0));
        QVERIFY(pickedNewShape(e) >= 0);
        QVERIFY(e.win.w.session()->type(0, 1, 1, QStringLiteral("a\x01") + QStringLiteral("b\x0b") + QStringLiteral("c")).ok);
        typeLink(e, QStringLiteral("=B2"));
        QVERIFY(saveIt(e));
        const QByteArray drawing = savedDrawing(e.path);
        QDomDocument doc;
        QVERIFY2(doc.setContent(drawing), drawing.left(900).constData());
        QVERIFY2(drawing.contains(">abc</a:t>"), drawing.left(1200).constData());
        EditWindow again;
        QVERIFY(again.open([&] {
            QFile f(e.path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        }()));
        QVERIFY(again.settle());
        QTRY_VERIFY(shown(again, 1));
        crunch::SheetObjects *o = again.win.w.objects();
        QTRY_COMPARE(o->linkedTextOf(0, o->idsOf(0).front()), QStringLiteral("abc"));
    }

    // Saved as .ods, a shape that shows a cell keeps the words the cell showed and has no link (the format has none), and the save is whole.
    void aLinkedShapeSavedAsOdsKeepsItsWordsAndNoLink()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({dataSheet()})));
        QVERIFY(e.settle());
        QTRY_VERIFY(shown(e, 0));
        QVERIFY(pickedNewShape(e) >= 0);
        typeLink(e, QStringLiteral("=B2"));
        QSignalSpy copied(&e.win.w, &CrunchWindow::copyExported);
        const QString ods = e.folder.dir.filePath(QStringLiteral("copy.ods"));
        QVERIFY(e.win.w.saveAs(ods));
        QVERIFY2(copied.count() > 0 || copied.wait(20000), "the copy failed");
        QFile f(ods);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(f.readAll(), parts));
        const QByteArray content = parts.value(QStringLiteral("content.xml"));
        QVERIFY(content.contains("<draw:text-box"));
        QVERIFY2(content.contains(">10</text:span>"), content.mid(content.indexOf("<table:shapes"), 700).constData());
        QVERIFY(!content.contains("textlink") && !content.contains("Sheet1!$B$2"));
        QCOMPARE(linkOf(e), QStringLiteral("Sheet1!$B$2"));   // (the workbook open here is as it was)
    }

    // After a change of parts the shape is made again with the text its cell has now, not the words the part holds.
    void aShapeMadeAgainAfterAChangeOfPartsShowsItsCell()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithDrawing({dataSheet()}, drawingPart(twoCell(5, 1, 8, 4, linkedShape(2, QStringLiteral("$B$3"), QStringLiteral("stored")))), {})));
        QTRY_VERIFY(shown(e, 1));
        QVERIFY(e.settle());
        crunch::SheetObjects *o = e.win.w.objects();
        const int id = o->idsOf(0).front();
        QTRY_COMPARE(o->linkedTextOf(0, id), QStringLiteral("20"));
        QCOMPARE(o->itemTextOf(0, id), QStringLiteral("20"));
        QVERIFY(e.win.w.session()->type(0, 2, 1, QStringLiteral("fresh")).ok);
        QVERIFY(e.settle());
        QTRY_COMPARE(o->linkedTextOf(0, id), QStringLiteral("fresh"));
        QCOMPARE(o->itemTextOf(0, id), QStringLiteral("fresh"));
        QVERIFY(e.win.w.session()->type(0, 2, 1, QStringLiteral("fresher")).ok);
        QVERIFY(e.settle());
        QTRY_COMPARE(o->linkedTextOf(0, id), QStringLiteral("fresher"));
        o->partsChanged({QStringLiteral("xl/drawings/drawing1.xml")});
        QCOMPARE(o->itemTextOf(0, id), QStringLiteral("fresher"));
    }

    // A formula that has no value yet (or one that is empty) shows empty text, not the words the file stored.
    void aFormulaThatIsEmptyShowsNothing()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({dataSheet()})));
        QVERIFY(e.settle());
        auto *s = e.win.w.session();
        QVERIFY(s->type(0, 0, 3, QStringLiteral("=B2*2")).ok);
        QString text = QStringLiteral("stored");
        QVERIFY(s->linkShownText(0, 0, 3, &text));   // (before the calculation has given it a value)
        QCOMPARE(text, QString());
        QVERIFY(e.settle());
        QVERIFY(s->linkShownText(0, 0, 3, &text));
        QCOMPARE(text, QStringLiteral("20"));
    }

    // After a cell entry is committed while a shape is picked, the formula bar shows the cell, not the shape's link.
    void theFormulaBarShowsTheCellAgainAfterACellEntry()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({dataSheet()})));
        QVERIFY(e.settle());
        QTRY_VERIFY(shown(e, 0));
        QVERIFY(pickedNewShape(e) >= 0);
        typeLink(e, QStringLiteral("=B2"));
        QCOMPARE(linkBar(e)->text(), QStringLiteral("=Sheet1!$B$2"));
        // (a cell entry that is begun with the shape still picked: AutoSum)
        e.select(QStringLiteral("B5"));
        QCOMPARE(e.win.w.objects()->layer()->selectedId(), e.win.w.objects()->idsOf(0).front());
        QVERIFY(e.win.w.act(QStringLiteral("edit.autosum"))->isEnabled());
        e.win.w.act(QStringLiteral("edit.autosum"))->trigger();
        QVERIFY(e.win.w.cellEditor()->isVisible());
        QVERIFY(e.win.w.commitEdit(CellEditor::Way::Stay));
        QVERIFY(e.settle());
        QCOMPARE(e.selected(), QStringLiteral("B5"));
        QTRY_COMPARE(linkBar(e)->text(), QStringLiteral("=SUM(B2:B4)"));
    }

    // Where the DrawingML namespace is the default one, the text made for a link is written with no prefix.
    void aLinkedShapeInADefaultNamespaceIsWrittenWithoutAPrefix()
    {
        QDomDocument doc;
        QVERIFY(doc.setContent(QStringLiteral("<xdr:sp xmlns:xdr=\"x\" xmlns=\"y\"><xdr:spPr><xfrm/></xdr:spPr><xdr:txBody><bodyPr/><p><r><t>q</t></r></p></xdr:txBody></xdr:sp>")));
        QDomElement sp = doc.documentElement();
        jp::office::sheet::setShapeLinkText(sp, QStringLiteral("new\nlines"));
        const QString xml = doc.toString();
        QVERIFY2(!xml.contains(QLatin1String("a:p")) && !xml.contains(QLatin1String("a:r")) && !xml.contains(QLatin1String("a:t")), qPrintable(xml));
        QCOMPARE(jp::office::sheet::shapeLinkStoredText(sp), QStringLiteral("new\nlines"));
    }

    // The picture of a shape that shows =B2 with the formula bar, for the docs.
    void aPictureOfALinkedShapeForTheDocs()
    {
        if (qEnvironmentVariableIsEmpty("JO_SHOT_DIR")) QSKIP("set JO_SHOT_DIR to take the picture");
        jp::applyUiTheme(1);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({dataSheet()})));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        e.win.w.resize(1300, 820);
        e.win.w.show();
        QVERIFY(pickedNewShape(e) >= 0);
        typeLink(e, QStringLiteral("=B2"));
        QTRY_COMPARE(shapeText(e), QStringLiteral("10"));
        e.win.w.objects()->layer()->prepareAll();
        const QImage shot = e.win.w.grab().toImage();
        QVERIFY(shot.save(qEnvironmentVariable("JO_SHOT_DIR") + QStringLiteral("/crunch-shape-link.png")));
    }
};

JP_TEST_CLASS(CrunchDrawingTests)
#include "tst_crunch_drawings.moc"
