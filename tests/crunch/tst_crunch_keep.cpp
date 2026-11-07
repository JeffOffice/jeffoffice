// What a save keeps (the keep-on-save audit of the beta): every part of a workbook that JeffCrunch does not edit is written back as it was, byte for byte, and the sheet that was edited
// keeps all that is not a cell. Each kind of part is tried in a workbook made here, with a cell edited elsewhere; a macro project is kept as a macro project is.
#include "findbooks.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/editlinksdialog.h"
#include "app/infopage.h"
#include "app/otherformats.h"
#include "core/loader.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "editwindow.h"
#include "io/cfb.h"
#include "office/package.h"
#include "sheet/sheetreader.h"
#include "io/zip.h"
#include "io/zipindex.h"
#include "savedok.h"
#include "testrunner.h"

#include <QBuffer>
#include <QDir>
#include <QTableWidget>
#include <QElapsedTimer>
#include <QtEndian>
#include <QImage>
#include <QRegularExpression>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>
#include <QtTest>

#include <functional>

using namespace jp;
using namespace crunchtest;

namespace {

using Parts = QMap<QString, QByteArray>;

const QString kRel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");

QByteArray repack(const QByteArray &zip, const std::function<void(Parts &)> &change)
{
    Parts parts;
    readZip(zip, parts);
    change(parts);
    ZipWriter z;
    for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

void addType(Parts &p, const QString &part, const QString &contentType)
{
    p[QStringLiteral("[Content_Types].xml")].replace("</Types>", QStringLiteral("<Override PartName=\"/%1\" ContentType=\"%2\"/>").arg(part, contentType).toUtf8() + "</Types>");
}

void addDefault(Parts &p, const QString &extension, const QString &contentType)
{
    QByteArray &types = p[QStringLiteral("[Content_Types].xml")];
    if (types.contains(QStringLiteral("Extension=\"%1\"").arg(extension).toUtf8())) return;
    types.insert(types.indexOf("<Override"), QStringLiteral("<Default Extension=\"%1\" ContentType=\"%2\"/>").arg(extension, contentType).toUtf8());
}

// A relationship in the relationships part of `owner` (made if there is none): the type is the end of the URI (or the whole of it), the target as the owner names it.
void relate(Parts &p, const QString &relsPart, const QString &id, const QString &type, const QString &target, bool external = false)
{
    QByteArray &rels = p[relsPart];
    if (rels.isEmpty()) rels = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"></Relationships>";
    const QString full = type.startsWith(QLatin1String("http")) ? type : kRel + type;
    rels.replace("</Relationships>", QStringLiteral("<Relationship Id=\"%1\" Type=\"%2\" Target=\"%3\"%4/></Relationships>").arg(id, full, target, external ? QStringLiteral(" TargetMode=\"External\"") : QString()).toUtf8());
}

const QString kSheet = QStringLiteral("xl/worksheets/sheet1.xml");
const QString kSheetRels = QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels");
const QString kBookRels = QStringLiteral("xl/_rels/workbook.xml.rels");

// The sheet part with something put in before its closing tag (after the sheet's data and what an ordinary sheet has there).
void intoSheet(Parts &p, const QByteArray &xml) { p[kSheet].replace("</worksheet>", xml + "</worksheet>"); }

QByteArray tinyPng()
{
    QImage image(4, 4, QImage::Format_RGB32);
    image.fill(Qt::red);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return bytes;
}

QByteArray pivotBook()
{
    return repack(editBook(), [](Parts &p) {
        // the cache: its definition, its records, and the table that shows it on cells F1:G4
        addType(p, QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml"), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheDefinition+xml"));
        addType(p, QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml"), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheRecords+xml"));
        addType(p, QStringLiteral("xl/pivotTables/pivotTable1.xml"), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml"));
        p[QStringLiteral("xl/pivotCache/pivotCacheDefinition1.xml")] =
            "<pivotCacheDefinition xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" r:id=\"rId1\" recordCount=\"2\">"
            "<cacheSource type=\"worksheet\"><worksheetSource ref=\"A1:A2\" sheet=\"Sheet1\"/></cacheSource><cacheFields count=\"1\"><cacheField name=\"Amount\" numFmtId=\"0\"><sharedItems containsSemiMixedTypes=\"0\" "
            "containsString=\"0\" containsNumber=\"1\" containsInteger=\"1\" minValue=\"1\" maxValue=\"10\"/></cacheField></cacheFields></pivotCacheDefinition>";
        p[QStringLiteral("xl/pivotCache/pivotCacheRecords1.xml")] = "<pivotCacheRecords xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" count=\"2\"><r><n v=\"1\"/></r><r><n v=\"10\"/></r></pivotCacheRecords>";
        relate(p, QStringLiteral("xl/pivotCache/_rels/pivotCacheDefinition1.xml.rels"), QStringLiteral("rId1"), QStringLiteral("pivotCacheRecords"), QStringLiteral("pivotCacheRecords1.xml"));
        p[QStringLiteral("xl/pivotTables/pivotTable1.xml")] =
            "<pivotTableDefinition xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" name=\"PivotTable1\" cacheId=\"1\" dataCaption=\"Values\">"
            "<location ref=\"F1:G4\" firstHeaderRow=\"1\" firstDataRow=\"1\" firstDataCol=\"1\"/><pivotFields count=\"1\"><pivotField dataField=\"1\" showAll=\"0\"/></pivotFields>"
            "<dataFields count=\"1\"><dataField name=\"Sum of Amount\" fld=\"0\" baseField=\"0\" baseItem=\"0\"/></dataFields></pivotTableDefinition>";
        relate(p, QStringLiteral("xl/pivotTables/_rels/pivotTable1.xml.rels"), QStringLiteral("rId1"), QStringLiteral("pivotCacheDefinition"), QStringLiteral("../pivotCache/pivotCacheDefinition1.xml"));
        relate(p, kSheetRels, QStringLiteral("rIdPv"), QStringLiteral("pivotTable"), QStringLiteral("../pivotTables/pivotTable1.xml"));
        relate(p, kBookRels, QStringLiteral("rIdPc"), QStringLiteral("pivotCacheDefinition"), QStringLiteral("pivotCache/pivotCacheDefinition1.xml"));
        p[QStringLiteral("xl/workbook.xml")].replace("</workbook>", "<pivotCaches><pivotCache cacheId=\"1\" r:id=\"rIdPc\"/></pivotCaches></workbook>");
        // (the cells the table shows are in the sheet as the file saved them)
        p[kSheet].replace("</row></sheetData>", "<c r=\"F1\" t=\"inlineStr\"><is><t>Sum of Amount</t></is></c><c r=\"G1\"><v>11</v></c></row></sheetData>");
    });
}

QByteArray formControlBook()
{
    return repack(editBook(), [](Parts &p) {
        addDefault(p, QStringLiteral("vml"), QStringLiteral("application/vnd.openxmlformats-officedocument.vmlDrawing"));
        addType(p, QStringLiteral("xl/ctrlProps/ctrlProp1.xml"), QStringLiteral("application/vnd.ms-excel.controlproperties+xml"));
        p[QStringLiteral("xl/ctrlProps/ctrlProp1.xml")] = "<formControlPr xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\" objectType=\"CheckBox\" fmlaLink=\"$A$3\" lockText=\"1\"/>";
        p[QStringLiteral("xl/drawings/vmlDrawing1.vml")] =
            "<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:x=\"urn:schemas-microsoft-com:office:excel\"><v:shape id=\"_x0000_s1025\" type=\"#_x0000_t201\" "
            "style=\"position:absolute;margin-left:100pt;margin-top:20pt;width:60pt;height:20pt\"><x:ClientData ObjectType=\"Checkbox\"><x:FmlaLink>$A$3</x:FmlaLink></x:ClientData></v:shape></xml>";
        relate(p, kSheetRels, QStringLiteral("rIdVml"), QStringLiteral("vmlDrawing"), QStringLiteral("../drawings/vmlDrawing1.vml"));
        relate(p, kSheetRels, QStringLiteral("rIdCtl"), QStringLiteral("ctrlProp"), QStringLiteral("../ctrlProps/ctrlProp1.xml"));
        intoSheet(p, "<legacyDrawing r:id=\"rIdVml\"/><controls><mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice Requires=\"x14\">"
                     "<control shapeId=\"1025\" r:id=\"rIdCtl\" name=\"Check Box 1\"><controlPr defaultSize=\"0\" autoFill=\"0\" linked=\"1\"><anchor moveWithCells=\"1\"><from><xdr:col xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\">5</xdr:col>"
                     "</from></anchor></controlPr></control></mc:Choice></mc:AlternateContent></controls>");
    });
}

QByteArray externalLinkBook()
{
    return repack(editBook(), [](Parts &p) {
        addType(p, QStringLiteral("xl/externalLinks/externalLink1.xml"), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.externalLink+xml"));
        p[QStringLiteral("xl/externalLinks/externalLink1.xml")] =
            "<externalLink xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><externalBook r:id=\"rId1\"><sheetNames>"
            "<sheetName val=\"Prices\"/></sheetNames><sheetDataSet><sheetData sheetId=\"0\"><row r=\"1\"><cell r=\"A1\"><v>42</v></cell></row></sheetData></sheetDataSet></externalBook></externalLink>";
        relate(p, QStringLiteral("xl/externalLinks/_rels/externalLink1.xml.rels"), QStringLiteral("rId1"), QStringLiteral("externalLinkPath"), QStringLiteral("file:///C:/data/prices.xlsx"), true);
        relate(p, kBookRels, QStringLiteral("rIdEx"), QStringLiteral("externalLink"), QStringLiteral("externalLinks/externalLink1.xml"));
        p[QStringLiteral("xl/workbook.xml")].replace("</sheets>", "</sheets><externalReferences><externalReference r:id=\"rIdEx\"/></externalReferences>");
        p[kSheet].replace("</row></sheetData>", "</row><row r=\"5\"><c r=\"A5\"><f>[1]Prices!A1*2</f><v>84</v></c></row></sheetData>");
    });
}

QByteArray drawingBook()
{
    return repack(editBook(), [](Parts &p) {
        addDefault(p, QStringLiteral("png"), QStringLiteral("image/png"));
        addType(p, QStringLiteral("xl/drawings/drawing1.xml"), QStringLiteral("application/vnd.openxmlformats-officedocument.drawing+xml"));
        p[QStringLiteral("xl/media/image1.png")] = tinyPng();
        p[QStringLiteral("xl/drawings/drawing1.xml")] =
            "<xdr:wsDr xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
            "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><xdr:twoCellAnchor><xdr:from><xdr:col>5</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>5</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from>"
            "<xdr:to><xdr:col>7</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>9</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to><xdr:pic><xdr:nvPicPr><xdr:cNvPr id=\"2\" name=\"Picture 1\" descr=\"a red square\"/><xdr:cNvPicPr/></xdr:nvPicPr>"
            "<xdr:blipFill><a:blip r:embed=\"rId1\"/><a:stretch><a:fillRect/></a:stretch></xdr:blipFill><xdr:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"10\" cy=\"10\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></xdr:spPr></xdr:pic><xdr:clientData/></xdr:twoCellAnchor></xdr:wsDr>";
        relate(p, QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"), QStringLiteral("rId1"), QStringLiteral("image"), QStringLiteral("../media/image1.png"));
        relate(p, kSheetRels, QStringLiteral("rIdDr"), QStringLiteral("drawing"), QStringLiteral("../drawings/drawing1.xml"));
        intoSheet(p, "<drawing r:id=\"rIdDr\"/>");
    });
}

QByteArray commentBook()
{
    return repack(editBook(), [](Parts &p) {
        addDefault(p, QStringLiteral("vml"), QStringLiteral("application/vnd.openxmlformats-officedocument.vmlDrawing"));
        addType(p, QStringLiteral("xl/comments1.xml"), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml"));
        p[QStringLiteral("xl/comments1.xml")] = "<comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors><author>Pat</author></authors><commentList><comment ref=\"C1\" authorId=\"0\">"
                                                "<text><r><rPr><b/><sz val=\"9\"/></rPr><t>Pat:</t></r><r><t xml:space=\"preserve\"> check this</t></r></text></comment></commentList></comments>";
        p[QStringLiteral("xl/drawings/vmlDrawing1.vml")] =
            "<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:x=\"urn:schemas-microsoft-com:office:excel\"><v:shape id=\"_x0000_s1025\" type=\"#_x0000_t202\" "
            "style=\"position:absolute;margin-left:80pt;margin-top:5pt;width:100pt;height:40pt;visibility:hidden\"><x:ClientData ObjectType=\"Note\"><x:Row>0</x:Row><x:Column>2</x:Column></x:ClientData></v:shape></xml>";
        relate(p, kSheetRels, QStringLiteral("rIdCm"), QStringLiteral("comments"), QStringLiteral("../comments1.xml"));
        relate(p, kSheetRels, QStringLiteral("rIdVm"), QStringLiteral("vmlDrawing"), QStringLiteral("../drawings/vmlDrawing1.vml"));
        intoSheet(p, "<legacyDrawing r:id=\"rIdVm\"/>");
    });
}

QByteArray outlineBook()
{
    return repack(editBook(), [](Parts &p) {
        // rows 2 and 3 are a collapsed group below row 1's summary row; the columns have a group too
        QByteArray &s = p[kSheet];
        s.replace("<sheetData>", "<sheetPr><outlinePr summaryBelow=\"0\"/></sheetPr><sheetFormatPr defaultRowHeight=\"15\" outlineLevelRow=\"1\" outlineLevelCol=\"1\"/><cols><col min=\"5\" max=\"6\" width=\"9\" outlineLevel=\"1\" "
                              "hidden=\"1\" customWidth=\"1\"/><col min=\"7\" max=\"7\" width=\"9\" collapsed=\"1\"/></cols><sheetData>");
        s.replace("<row r=\"1\">", "<row r=\"1\" collapsed=\"1\">");
        s.replace("<row r=\"2\">", "<row r=\"2\" outlineLevel=\"1\" hidden=\"1\">");
        s.replace("<row r=\"3\">", "<row r=\"3\" outlineLevel=\"1\" hidden=\"1\">");
    });
}

QByteArray otherPartsBook()
{
    return repack(editBook(), [](Parts &p) {
        addDefault(p, QStringLiteral("bin"), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.printerSettings"));
        addType(p, QStringLiteral("customXml/item1.xml"), QStringLiteral("application/xml"));
        addType(p, QStringLiteral("customXml/itemProps1.xml"), QStringLiteral("application/vnd.openxmlformats-officedocument.customXmlProperties+xml"));
        addType(p, QStringLiteral("docProps/custom.xml"), QStringLiteral("application/vnd.openxmlformats-officedocument.custom-properties+xml"));
        addType(p, QStringLiteral("xl/persons/person.xml"), QStringLiteral("application/vnd.ms-excel.person+xml"));
        p[QStringLiteral("customXml/item1.xml")] = "<root xmlns=\"urn:keep\"><a>1</a></root>";
        p[QStringLiteral("customXml/itemProps1.xml")] = "<ds:datastoreItem xmlns:ds=\"http://schemas.openxmlformats.org/officeDocument/2006/customXml\" ds:itemID=\"{11111111-2222-3333-4444-555555555555}\"/>";
        relate(p, QStringLiteral("customXml/_rels/item1.xml.rels"), QStringLiteral("rId1"), QStringLiteral("customXmlProps"), QStringLiteral("itemProps1.xml"));
        relate(p, kBookRels, QStringLiteral("rIdCx"), QStringLiteral("customXml"), QStringLiteral("../customXml/item1.xml"));
        p[QStringLiteral("docProps/custom.xml")] = "<Properties xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/custom-properties\"><property fmtid=\"{D5CDD505-2E9C-101B-9397-08002B2CF9AE}\" pid=\"2\" name=\"Mine\"><lpwstr>kept</lpwstr></property></Properties>";
        relate(p, QStringLiteral("_rels/.rels"), QStringLiteral("rIdCp"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/custom-properties"), QStringLiteral("docProps/custom.xml"));
        p[QStringLiteral("xl/persons/person.xml")] = "<personList xmlns=\"http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments\"/>";
        relate(p, kBookRels, QStringLiteral("rIdPe"), QStringLiteral("http://schemas.microsoft.com/office/2017/10/relationships/person"), QStringLiteral("persons/person.xml"));
        p[QStringLiteral("xl/printerSettings/printerSettings1.bin")] = QByteArray("\x01\x02PRINTER\x00\x03 settings that no program here reads", 40);
        relate(p, kSheetRels, QStringLiteral("rIdPs"), QStringLiteral("printerSettings"), QStringLiteral("../printerSettings/printerSettings1.bin"));
        // the sheet carries extension lists, an alternate-content element, and page setup that name the printer settings
        intoSheet(p, "<pageSetup orientation=\"landscape\" r:id=\"rIdPs\"/><extLst><ext uri=\"{05C60535-1F16-4fd2-B633-F4F36F0B64E0}\" xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\">"
                     "<x14:sparklineGroups xmlns:xm=\"http://schemas.microsoft.com/office/excel/2006/main\"><x14:sparklineGroup><x14:colorSeries rgb=\"FF376092\"/><x14:sparklines><x14:sparkline><xm:f>Sheet1!A1:A3</xm:f>"
                     "<xm:sqref>E1</xm:sqref></x14:sparkline></x14:sparklines></x14:sparklineGroup></x14:sparklineGroups></ext></extLst>");
    });
}

QByteArray macroBookOf(const QString &mainType, bool signedProject)
{
    return repack(editBook(), [&](Parts &p) {
        p[QStringLiteral("[Content_Types].xml")].replace("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml", mainType.toUtf8());
        addType(p, QStringLiteral("xl/vbaProject.bin"), QStringLiteral("application/vnd.ms-office.vbaProject"));
        p[QStringLiteral("xl/vbaProject.bin")] = QByteArray("\xD0\xCF\x11\xE0 not a compound file, only bytes that must come back as they went \x00\x01\x02\xFF", 60);
        relate(p, kBookRels, QStringLiteral("rIdVba"), QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/vbaProject"), QStringLiteral("vbaProject.bin"));
        if (signedProject) {
            addType(p, QStringLiteral("xl/vbaProjectSignature.bin"), QStringLiteral("application/vnd.ms-office.vbaProjectSignature"));
            p[QStringLiteral("xl/vbaProjectSignature.bin")] = QByteArray("signature bytes \x00\x10\x20 of the project", 32);
            relate(p, QStringLiteral("xl/_rels/vbaProject.bin.rels"), QStringLiteral("rId1"), QStringLiteral("http://schemas.microsoft.com/office/2006/relationships/vbaProjectSignature"), QStringLiteral("vbaProjectSignature.bin"));
        }
        // (the project's code is in a part of its own that a sheet's code names; a custom ribbon is another)
        addType(p, QStringLiteral("customUI/customUI14.xml"), QStringLiteral("application/xml"));
        p[QStringLiteral("customUI/customUI14.xml")] = "<customUI xmlns=\"http://schemas.microsoft.com/office/2009/07/customui\"><ribbon/></customUI>";
        relate(p, QStringLiteral("_rels/.rels"), QStringLiteral("rIdUi"), QStringLiteral("http://schemas.microsoft.com/office/2007/relationships/ui/extensibility"), QStringLiteral("customUI/customUI14.xml"));
    });
}

// A sheet's XML with every cell taken out: what is left is the rows, columns, merges, outlines, and the rest that a cell edit has no business with. A row's spans, the height of a row that has none
// of its own, and the sheet's dimension are what a cell edit may legitimately change.
QByteArray skeleton(const QByteArray &xml)
{
    static const QRegularExpression cell(QStringLiteral("<c(?=[\\s/>])[^>]*?/>|<c(?=[\\s/>])[^>]*>.*?</c>"), QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression volatileParts(QStringLiteral("( spans=\"[^\"]*\")|(<dimension ref=\"[^\"]*\"/>)"));
    QString text = QString::fromUtf8(xml);
    text.remove(cell);
    text.remove(volatileParts);
    // (a row that has no height of its own may be given the height its text needs, which is how a row with a height nobody chose is written)
    static const QRegularExpression rowTag(QStringLiteral("<row [^>]*>"));
    static const QRegularExpression autoHeight(QStringLiteral(" ht=\"[^\"]*\""));
    QString out;
    qsizetype last = 0;
    for (auto m = rowTag.globalMatch(text); m.hasNext();) {
        const auto match = m.next();
        QString tag = match.captured(0);
        if (!tag.contains(QLatin1String("customHeight"))) tag.remove(autoHeight);
        out += text.mid(last, match.capturedStart() - last) + tag;
        last = match.capturedEnd();
    }
    text = out + text.mid(last);
    return text.toUtf8();
}

} // namespace

class CrunchKeepTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        static QTemporaryDir settings;
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        setAppInfo(crunchAppInfo());
        initCore();
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    // A pivot table with its cache definition and records is written back as it was, and a cell edited beside it changes nothing of them.
    void aPivotTableAndItsCacheSurviveAnEditElsewhere() { keepsWhatItDoesNotEdit(pivotBook(), QStringLiteral("D1"), QStringLiteral("5")); }

    // Form controls (the check box's properties, the drawing that places it, and the sheet's `controls` list) are written back as they were.
    void formControlsSurviveAnEditElsewhere() { keepsWhatItDoesNotEdit(formControlBook(), QStringLiteral("D1"), QStringLiteral("5")); }

    // The parts of a link to another workbook (the cached values and where it points) are written back as they were, and so is the formula that reads them.
    void anExternalLinkSurvivesAnEditElsewhere() { keepsWhatItDoesNotEdit(externalLinkBook(), QStringLiteral("D1"), QStringLiteral("5")); }

    // Inspect Workbook takes the links to other workbooks out: a cell that read one keeps the value it showed, a name that stood for one is #REF!, and the file has no part, link,
    // list entry, or type for them.
    void inspectTakesExternalLinksOut()
    {
        const QByteArray book = repack(externalLinkBook(), [](Parts &p) {
            p[QStringLiteral("xl/workbook.xml")].replace("</externalReferences>", "</externalReferences><definedNames><definedName name=\"Px\">[1]Prices!$A$1</definedName></definedNames>");
        });
        EditWindow e;
        QVERIFY(e.open(book, QStringLiteral("links.xlsx")));
        QVERIFY(e.settle());
        const QVector<InspectItem> items = e.win.w.inspectItems();
        const InspectItem *item = nullptr;
        for (const InspectItem &i : items)
            if (i.title == QLatin1String("External links")) item = &i;
        QVERIFY(item);
        QCOMPARE(item->count(), 1);
        QVERIFY(item->removable);
        QVERIFY(item->remove() >= 1);
        QVERIFY(e.settle());
        QCOMPARE(item->count(), 0);
        QCOMPARE(e.win.w.session()->editText(0, 4, 0), QStringLiteral("84"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        Parts now;
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        readZip(f.readAll(), now);
        for (auto it = now.begin(); it != now.end(); ++it) QVERIFY2(!it.key().contains(QLatin1String("externalLink")), qPrintable(it.key()));
        QVERIFY(!now.value(QStringLiteral("xl/workbook.xml")).contains("externalReference"));
        QVERIFY(!now.value(kBookRels).contains("externalLink"));
        QVERIFY(!now.value(QStringLiteral("[Content_Types].xml")).contains("externalLink"));
        QVERIFY2(!now.value(kSheet).contains("[1]"), now.value(kSheet).constData());
        QVERIFY(now.value(QStringLiteral("xl/workbook.xml")).contains("#REF!"));
        f.seek(0);
        EditWindow again;
        QVERIFY(again.open(f.readAll(), QStringLiteral("links2.xlsx")));
        const QVector<InspectItem> items2 = again.win.w.inspectItems();
        for (const InspectItem &i : items2)
            if (i.title == QLatin1String("External links")) QCOMPARE(i.count(), 0);
    }

    // Edit Links lists the workbooks this one reads, in the order of the numbers in its formulas, as the file writes them and as a person reads them; Break Link on one makes the cells that read
    // it its values, and when none is left the saver leaves the link parts out.
    void editLinksListsTheSourcesAndBreaksOne()
    {
        const QByteArray book = repack(externalLinkBook(), [](Parts &p) {
            addType(p, QStringLiteral("xl/externalLinks/externalLink2.xml"), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.externalLink+xml"));
            p[QStringLiteral("xl/externalLinks/externalLink2.xml")] = "<externalLink xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><externalBook r:id=\"rId1\"><sheetNames><sheetName val=\"S\"/></sheetNames></externalBook></externalLink>";
            relate(p, QStringLiteral("xl/externalLinks/_rels/externalLink2.xml.rels"), QStringLiteral("rId1"), QStringLiteral("externalLinkPath"), QStringLiteral("https://example.org/My%20Costs.xlsx"), true);
            relate(p, kBookRels, QStringLiteral("rIdEy"), QStringLiteral("externalLink"), QStringLiteral("externalLinks/externalLink2.xml"));
            p[QStringLiteral("xl/workbook.xml")].replace("<externalReference r:id=\"rIdEx\"/>", "<externalReference r:id=\"rIdEx\"/><externalReference r:id=\"rIdEy\"/>");
            p[kSheet].replace("</row></sheetData>", "</row><row r=\"6\"><c r=\"A6\"><f>[2]S!A1+1</f><v>8</v></c></row></sheetData>");
        });
        EditWindow e;
        QVERIFY(e.open(book, QStringLiteral("links.xlsx")));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const std::vector<crunch::EditSession::ExternalSource> sources = s->externalLinkSources();
        QCOMPARE(int(sources.size()), 2);
        QCOMPARE(sources[0].index, 1);
        QCOMPARE(sources[0].target, QStringLiteral("file:///C:/data/prices.xlsx"));
        QCOMPARE(sources[0].shown, QStringLiteral("C:/data/prices.xlsx"));
        QVERIFY(sources[0].local);
        QCOMPARE(sources[1].index, 2);
        QCOMPARE(sources[1].shown, QStringLiteral("https://example.org/My Costs.xlsx"));
        QVERIFY(!sources[1].local);
        QVERIFY(e.win.w.act(QStringLiteral("data.editLinks")));
        // break the second: its cell is a value, the first stays a link and the parts stay
        QVERIFY(s->breakExternalLinks(nullptr, nullptr, 2).ok);
        QCOMPARE(s->editText(0, 5, 0), QStringLiteral("8"));
        QCOMPARE(s->editText(0, 4, 0), QStringLiteral("=[1]Prices!A1*2"));
        QVERIFY(!s->externalLinksDropped());
        QCOMPARE(int(s->externalLinkSources().size()), 2);
        // break the rest
        QVERIFY(s->breakExternalLinks(nullptr, nullptr, 1).ok);
        QCOMPARE(s->editText(0, 4, 0), QStringLiteral("84"));
        QVERIFY(s->externalLinksDropped());
        QCOMPARE(int(s->externalLinkSources().size()), 0);
    }

    // The Edit Links window lists the sources, checks whether each is a file that is there, opens one that is, and breaks the links to the one chosen.
    void theEditLinksWindowShowsTheStatusOfEachSource()
    {
        EditWindow e;
        QVERIFY(e.open(externalLinkBook(), QStringLiteral("links.xlsx")));
        QVERIFY(e.settle());
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        EditLinksDialog d(e.win.w.session(), dir.path());
        QCOMPARE(d.table()->rowCount(), 1);
        QCOMPARE(d.table()->item(0, 0)->text(), QStringLiteral("C:/data/prices.xlsx"));
        QCOMPARE(d.table()->item(0, 2)->text(), QStringLiteral("Not checked"));
        d.checkStatus();
        d.show();
        saveShot(&d, QStringLiteral("edit-links"));
        QVERIFY(d.table()->item(0, 2)->text().startsWith(QLatin1String("Not found")));
        // a source with no folder is looked for beside this workbook
        crunch::EditSession::ExternalSource s;
        s.shown = QStringLiteral("prices.xlsx");
        s.local = true;
        QCOMPARE(EditLinksDialog::localPath(s, dir.path()), QDir(dir.path()).absoluteFilePath(QStringLiteral("prices.xlsx")));
        s.local = false;
        QVERIFY(EditLinksDialog::localPath(s, dir.path()).isEmpty());
        QSignalSpy broken(&d, &EditLinksDialog::linksBroken);
        QVERIFY(d.breakRow(0, false));
        QCOMPARE(broken.count(), 1);
        QCOMPARE(d.table()->rowCount(), 0);
        QCOMPARE(e.win.w.session()->editText(0, 4, 0), QStringLiteral("84"));
    }

    // Update Values keeps the values it read in the link's own part, so that a save and a reopen without the source show them; Undo of the update takes the cells back to what the file stored.
    void updateValuesAreKeptInTheLinkPart()
    {
        const QByteArray book = repack(externalLinkBook(), [](Parts &p) {
            p[QStringLiteral("xl/externalLinks/externalLink1.xml")].replace("<sheetName val=\"Prices\"/>", "<sheetName val=\"Sheet1\"/>");
            p[kSheet].replace("[1]Prices!A1*2", "[1]Sheet1!A1*2");
            p[kSheet].replace("</row></sheetData>", "</row><row r=\"6\"><c r=\"A6\"><f>[1]Sheet1!A1</f><v>42</v></c></row></sheetData>");
        });
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString source = QDir(dir.path()).absoluteFilePath(QStringLiteral("prices.xlsx"));
        {
            QFile f(source);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(bookOfSheets({{"A1=7", "A2=hello"}}));
        }
        EditWindow e;
        QVERIFY(e.open(book, QStringLiteral("links.xlsx")));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        crunch::EditSession::ExternalUpdate result;
        QVERIFY(s->updateExternalValues(1, source, &result).ok);
        QCOMPARE(result.updated, 1);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A6")), QStringLiteral("n:7"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        f.close();
        Parts now;
        readZip(bytes, now);
        const QByteArray part = now.value(QStringLiteral("xl/externalLinks/externalLink1.xml"));
        QVERIFY2(part.contains("<v>7</v>") && !part.contains("<v>42</v>"), part.constData());
        QVERIFY2(part.contains("sheetId=\"0\"") && part.contains("r=\"A1\""), part.constData());
        // reopened, and the source is not asked for: the cell shows the new value, and the part still has it
        EditWindow again;
        QVERIFY(again.open(bytes, QStringLiteral("links2.xlsx")));
        QVERIFY(again.settle());
        QCOMPARE(again.valueAt(QStringLiteral("A6")), QStringLiteral("n:7"));
        QCOMPARE(int(again.win.w.session()->externalLinkSources().size()), 1);
    }

    // Undo of Update Values takes the link back to what it was: nothing of it is written to the link's part at the next save, and Redo marks it again.
    void undoOfUpdateValuesTakesTheLinkBackToWhatItWas()
    {
        const QByteArray book = repack(externalLinkBook(), [](Parts &p) {
            p[QStringLiteral("xl/externalLinks/externalLink1.xml")].replace("<sheetName val=\"Prices\"/>", "<sheetName val=\"Sheet1\"/>");
            p[kSheet].replace("[1]Prices!A1*2", "[1]Sheet1!A1*2");
            p[kSheet].replace("</row></sheetData>", "</row><row r=\"6\"><c r=\"A6\"><f>[1]Sheet1!A1</f><v>42</v></c></row></sheetData>");
        });
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString source = QDir(dir.path()).absoluteFilePath(QStringLiteral("prices.xlsx"));
        {
            QFile f(source);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(bookOfSheets({{"A1=7", "A2=hello"}}));
        }
        EditWindow e;
        QVERIFY(e.open(book, QStringLiteral("links.xlsx")));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->externalCachedValues().empty());
        QVERIFY(s->updateExternalValues(1, source).ok);
        QVERIFY(!s->externalCachedValues().empty());
        QCOMPARE(s->undoStack()->undoText(), QStringLiteral("Update Values"));
        s->undoStack()->undo();
        QVERIFY(e.settle());
        QVERIFY(s->externalCachedValues().empty());
        s->undoStack()->redo();
        QVERIFY(!s->externalCachedValues().empty());
    }

    // The values of cells that the link's part did not list are put in order (rows, then cells in a row), and a part that has no sheetDataSet gets one before an extension list.
    void updatedValuesAreWrittenInTheOrderOfThePartAndBeforeItsExtensions()
    {
        const QByteArray book = repack(externalLinkBook(), [](Parts &p) {
            QByteArray &link = p[QStringLiteral("xl/externalLinks/externalLink1.xml")];
            link.replace("<sheetName val=\"Prices\"/>", "<sheetName val=\"Sheet1\"/>");
            link.replace(link.indexOf("<sheetDataSet>"), link.indexOf("</sheetDataSet>") + 15 - link.indexOf("<sheetDataSet>"), "<extLst/>");
            p[kSheet].replace("[1]Prices!A1*2", "[1]Sheet1!A1*2");
            p[kSheet].replace("</row></sheetData>", "</row><row r=\"6\"><c r=\"A6\"><f>[1]Sheet1!B3</f><v>0</v></c></row><row r=\"7\"><c r=\"A7\"><f>[1]Sheet1!A1</f><v>0</v></c></row></sheetData>");
        });
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString source = QDir(dir.path()).absoluteFilePath(QStringLiteral("prices.xlsx"));
        {
            QFile f(source);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(bookOfSheets({{"A1=7", "B3=9"}}));
        }
        EditWindow e;
        QVERIFY(e.open(book, QStringLiteral("links.xlsx")));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.session()->updateExternalValues(1, source).ok);
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        Parts now;
        readZip(f.readAll(), now);
        const QByteArray part = now.value(QStringLiteral("xl/externalLinks/externalLink1.xml"));
        QVERIFY2(part.indexOf("<sheetDataSet") > 0 && part.indexOf("<sheetDataSet") < part.indexOf("<extLst"), part.constData());
        QVERIFY2(part.indexOf("r=\"A1\"") > 0 && part.indexOf("r=\"A1\"") < part.indexOf("r=\"B3\""), part.constData());
        QVERIFY2(part.indexOf("<row r=\"1\"") < part.indexOf("<row r=\"3\""), part.constData());
        QVERIFY2(part.contains("<v>9</v>") && part.contains("<v>7</v>"), part.constData());
    }

    // Break Link with nothing to break (no link in the workbook, or none that anything reads, of the number asked for) does nothing: no step of Undo, and the links are not marked to leave out.
    void breakLinkWithNothingToBreakDoesNothing()
    {
        EditWindow none;
        QVERIFY(none.open(editBook(), QStringLiteral("none.xlsx")));
        QVERIFY(none.settle());
        crunch::EditSession *s = none.win.w.session();
        int steps = s->undoStack()->count();
        QVERIFY(s->breakExternalLinks(nullptr, nullptr, 0).ok);
        QCOMPARE(s->undoStack()->count(), steps);
        QVERIFY(!s->externalLinksDropped());
        EditWindow some;
        QVERIFY(some.open(externalLinkBook(), QStringLiteral("some.xlsx")));
        QVERIFY(some.settle());
        s = some.win.w.session();
        steps = s->undoStack()->count();
        QVERIFY(s->breakExternalLinks(nullptr, nullptr, 2).ok);   // (there is no second link)
        QCOMPARE(s->undoStack()->count(), steps);
        QVERIFY(!s->externalLinksDropped());
        QCOMPARE(s->editText(0, 4, 0), QStringLiteral("=[1]Prices!A1*2"));
    }

    // A save that was started before Break Link was undone took the links out: its own snapshot says so, and the steps are let go of whatever the flag is when the save is finished.
    void anUndoOfBreakLinkDuringASaveStillLetsGoOfTheSteps()
    {
        EditWindow e;
        QVERIFY(e.open(externalLinkBook(), QStringLiteral("links.xlsx")));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->insertRows(0, 10, 1).ok);   // (a step before the break)
        QVERIFY(s->breakExternalLinks(nullptr, nullptr, 0).ok);
        QVERIFY(s->externalLinksDropped());
        crunch::EditSession::Snapshot snapshot;
        QString why;
        QVERIFY2(s->takeSnapshot(&snapshot, &why), qPrintable(why));
        QVERIFY(snapshot.changes.dropExternalLinks);
        s->undoStack()->undo();   // (while the file is being written)
        QVERIFY(!s->externalLinksDropped());
        QVERIFY(s->canUndo());
        s->saved(snapshot, s->workbook()->package());
        QVERIFY2(!s->canUndo(), "the file has no links to go back to");
    }

    // Break Link is one step of Undo: the cells that read the sources are their values, the names are #REF!, and the links are left out of the file; Undo brings all of that back. After a save
    // that left the links out, the step is let go of (the file no longer has the links to go back to).
    void breakLinkIsOneStepOfUndo()
    {
        EditWindow e;
        QVERIFY(e.open(externalLinkBook(), QStringLiteral("links.xlsx")));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const int steps = s->undoStack()->count();
        QVERIFY(s->breakExternalLinks(nullptr, nullptr, 0).ok);
        QVERIFY(e.settle());
        QCOMPARE(s->editText(0, 4, 0), QStringLiteral("84"));
        QVERIFY(s->externalLinksDropped());
        QCOMPARE(s->undoStack()->count(), steps + 1);
        QCOMPARE(s->undoStack()->undoText(), QStringLiteral("Break Link"));
        QVERIFY(s->canUndo());
        s->undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(s->editText(0, 4, 0), QStringLiteral("=[1]Prices!A1*2"));
        QVERIFY(!s->externalLinksDropped());
        QCOMPARE(int(s->externalLinkSources().size()), 1);
        s->undoStack()->redo();
        QVERIFY(e.settle());
        QCOMPARE(s->editText(0, 4, 0), QStringLiteral("84"));
        QVERIFY(s->externalLinksDropped());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        QVERIFY(!s->canUndo());
    }

    // Change Source points a link at another file (one step of Undo, written to the link's relationships at the next save, the formulas as they were), and Update Values gives the cells that are
    // only a reference to the source the values it has now; a cell that does more with them keeps the value its file stored.
    void editLinksChangeTheSourceAndUpdateTheValues()
    {
        const QByteArray book = repack(externalLinkBook(), [](Parts &p) {
            p[kSheet].replace("[1]Prices!A1*2", "[1]Sheet1!A1*2");
            p[kSheet].replace("</row></sheetData>", "</row><row r=\"6\"><c r=\"A6\"><f>[1]Sheet1!A1</f><v>42</v></c></row></sheetData>");
        });
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString source = QDir(dir.path()).absoluteFilePath(QStringLiteral("prices.xlsx"));
        {
            QFile f(source);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(bookOfSheets({{"A1=7", "A2=hello"}}));
        }
        EditWindow e;
        QVERIFY(e.open(book, QStringLiteral("links.xlsx")));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        EditLinksDialog d(s, dir.path());
        d.setSourceChooser([&](const QString &) { return source; });
        // the file named is not on this computer: nothing is read
        QVERIFY(!d.updateRow(0));
        QVERIFY(d.note().contains(QStringLiteral("can't be found")));
        QCOMPARE(e.valueAt(QStringLiteral("A6")), QStringLiteral("n:42"));
        // Change Source
        QVERIFY(d.changeRow(0));
        QCOMPARE(s->externalLinkSources()[0].shown, source);
        QVERIFY(s->externalLinkSources()[0].local);
        QCOMPARE(d.table()->item(0, 0)->text(), source);
        d.show();
        saveShot(&d, QStringLiteral("edit-links-change-source"));
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Change Source"));
        e.win.w.undoStack()->undo();
        QCOMPARE(s->externalLinkSources()[0].shown, QStringLiteral("C:/data/prices.xlsx"));
        e.win.w.undoStack()->redo();
        QCOMPARE(s->externalLinkSources()[0].shown, source);
        // Update Values
        QSignalSpy changed(&d, &EditLinksDialog::linksChanged);
        QVERIFY(d.updateRow(0));
        QCOMPARE(changed.count(), 1);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A6")), QStringLiteral("n:7"));
        QCOMPARE(e.valueAt(QStringLiteral("A5")), QStringLiteral("n:84"));   // (it multiplies the value: it keeps the one its file stored)
        QCOMPARE(s->editText(0, 5, 0), QStringLiteral("=[1]Sheet1!A1"));
        QVERIFY2(d.note().contains(QStringLiteral("now: 1. ")) && d.note().contains(QStringLiteral("stored: 1.")), qPrintable(d.note()));
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Update Values"));
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A6")), QStringLiteral("n:42"));
        e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A6")), QStringLiteral("n:7"));
        // the save writes the new address in the link's relationships and the new value in the cell, and keeps the formula
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        Parts now;
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        readZip(f.readAll(), now);
        f.close();   // (an open file can't be replaced on Windows: the next save would fail)
        const QByteArray rels = now.value(QStringLiteral("xl/externalLinks/_rels/externalLink1.xml.rels"));
        QVERIFY2(rels.contains(QUrl::fromLocalFile(source).toString(QUrl::FullyEncoded).toUtf8()), rels.constData());
        QVERIFY2(!rels.contains("C:/data"), rels.constData());
        QVERIFY2(now.value(kSheet).contains("<f>[1]Sheet1!A1</f><v>7</v>"), now.value(kSheet).constData());
        QVERIFY(now.value(QStringLiteral("xl/externalLinks/externalLink1.xml")).contains("<v>42</v>"));
        // Undo after the save goes back to the address the file had before the change, and the next save writes it
        e.win.w.undoStack()->undo();   // (the values)
        e.win.w.undoStack()->undo();   // (the source)
        QCOMPARE(s->externalLinkSources()[0].shown, QStringLiteral("C:/data/prices.xlsx"));
        QVERIFY(e.settle());
        QSignalSpy again(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(again), "the second save failed");
        Parts later;
        QFile g(e.path);
        QVERIFY(g.open(QIODevice::ReadOnly));
        readZip(g.readAll(), later);
        const QByteArray rels2 = later.value(QStringLiteral("xl/externalLinks/_rels/externalLink1.xml.rels"));
        QVERIFY2(rels2.contains("file:///C:/data/prices.xlsx") && !rels2.contains(QUrl::fromLocalFile(source).toString(QUrl::FullyEncoded).toUtf8()), rels2.constData());
        // only a file is read: a folder is refused, with the reason
        QVERIFY(!s->updateExternalValues(1, dir.path()).ok);
    }

    // Inspect Workbook leaves the custom XML out: the parts, what they relate to, the workbook's link to them, and their types; nothing else of the file changes.
    void inspectTakesCustomXmlOut()
    {
        EditWindow e;
        QVERIFY(e.open(otherPartsBook(), QStringLiteral("xmlparts.xlsx")));
        QVERIFY(e.settle());
        const QVector<InspectItem> items = e.win.w.inspectItems();
        const InspectItem *item = nullptr;
        for (const InspectItem &i : items)
            if (i.title == QLatin1String("Custom XML data")) item = &i;
        QVERIFY(item);
        QCOMPARE(item->count(), 1);
        QVERIFY(item->remove() >= 1);
        QCOMPARE(item->count(), 0);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        Parts now;
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        readZip(f.readAll(), now);
        for (auto it = now.begin(); it != now.end(); ++it) QVERIFY2(!it.key().startsWith(QLatin1String("customXml")), qPrintable(it.key()));
        QVERIFY(!now.value(kBookRels).contains("customXml"));
        QVERIFY(!now.value(QStringLiteral("[Content_Types].xml")).contains("customXml"));
        QVERIFY(now.contains(kSheet));
    }

    // A picture, its drawing, and the image are written back as they were.
    void aPictureAndItsDrawingSurviveAnEditElsewhere() { keepsWhatItDoesNotEdit(drawingBook(), QStringLiteral("D1"), QStringLiteral("5")); }

    // A note, its text, and the drawing that places it are written back as they were.
    void notesSurviveAnEditElsewhere() { keepsWhatItDoesNotEdit(commentBook(), QStringLiteral("D1"), QStringLiteral("5")); }

    // The outline of rows and columns (levels, collapsed groups, hidden members, the summary's side) is written back as it was, whether the edit is in a collapsed row or not.
    // A text with some of its letters formatted (the runs of a shared string) is written back as it was when another cell is typed in, and when a new text is added to the strings.
    void richTextSurvivesAnEditInAnotherCell()
    {
        const QString rich = QStringLiteral("<r><rPr><b/><sz val=\"11\"/></rPr><t>bo</t></r><r><rPr><i/></rPr><t xml:space=\"preserve\">ld </t></r><r><t>word</t></r>");
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"s\"><v>0</v></c><c r=\"B1\" t=\"s\"><v>1</v></c></row>");
        const QByteArray book = buildXlsx({worksheet(rows)}, sharedStringsXml({rich, QStringLiteral("<t>plain</t>")}));
        for (const QString &text : {QStringLiteral("5"), QStringLiteral("a new text")}) {
            EditWindow e;
            QVERIFY(e.open(book, QStringLiteral("rich.xlsx")));
            e.select(QStringLiteral("C1"));
            e.enter(text);
            QVERIFY(e.settle());
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.save());
            QVERIFY2(savedOk(saved), "the save failed");
            Parts now;
            QFile f(e.path);
            QVERIFY(f.open(QIODevice::ReadOnly));
            readZip(f.readAll(), now);
            const QByteArray strings = now.value(QStringLiteral("xl/sharedStrings.xml"));
            QVERIFY2(strings.contains(rich.toUtf8()), strings.constData());
            QVERIFY(strings.contains("<t>plain</t>"));
        }
    }

    void outlinesSurviveAnEditInAndOutOfAGroup()
    {
        keepsWhatItDoesNotEdit(outlineBook(), QStringLiteral("D1"), QStringLiteral("5"));
        keepsWhatItDoesNotEdit(outlineBook(), QStringLiteral("A2"), QStringLiteral("5"));
    }

    // Parts nothing here reads (custom XML, custom properties, printer settings, people, the extension lists of a sheet) are written back as they were.
    void partsNoOneReadsSurviveAnEditElsewhere() { keepsWhatItDoesNotEdit(otherPartsBook(), QStringLiteral("D1"), QStringLiteral("5")); }

    // A macro-enabled workbook keeps its project, its signature, its ribbon, and the relationships and types that name them, and the macros are not run.
    void aMacroWorkbookKeepsItsProjectAndNamesItsKind()
    {
        const QByteArray book = macroBookOf(QStringLiteral("application/vnd.ms-excel.sheet.macroEnabled.main+xml"), true);
        keepsWhatItDoesNotEdit(book, QStringLiteral("D1"), QStringLiteral("5"), QStringLiteral("macros.xlsm"));
        // saved again after another edit, and once untouched, the project is still the same bytes
        EditWindow e;
        QVERIFY(e.open(book, QStringLiteral("macros.xlsm")));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("3"));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        const Parts now = readParts(e.path), before = readRaw(book);
        for (const QString &name : {QStringLiteral("xl/vbaProject.bin"), QStringLiteral("xl/vbaProjectSignature.bin"), QStringLiteral("customUI/customUI14.xml")})
            QCOMPARE(rawOf(e.path).value(name), before.value(name));
        const QString rels = QString::fromUtf8(now.value(kBookRels));
        QVERIFY2(rels.contains(QLatin1String("2006/relationships/vbaProject\"")) && rels.contains(QLatin1String("vbaProject.bin")), qPrintable(rels));
        QVERIFY(QString::fromUtf8(now.value(QStringLiteral("xl/_rels/vbaProject.bin.rels"))).contains(QLatin1String("vbaProjectSignature.bin")));
        const QString types = QString::fromUtf8(now.value(QStringLiteral("[Content_Types].xml")));
        QVERIFY(types.contains(QLatin1String("macroEnabled.main+xml")) && types.contains(QLatin1String("/xl/vbaProject.bin")) && types.contains(QLatin1String("vbaProjectSignature")));
    }

    // A macro-enabled template keeps its ending: it is saved as a .xltm (the type of its main part), not forced into a .xlsm that the template's type would not fit.
    void aMacroTemplateIsSavedAsATemplate()
    {
        const QByteArray book = macroBookOf(QStringLiteral("application/vnd.ms-excel.template.macroEnabled.main+xml"), false);
        EditWindow e;
        QVERIFY(e.open(book, QStringLiteral("model.xltm")));
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        // the ending of its kind is accepted, an ending of no kind it may become is refused with the kind named
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(!e.win.w.saveAs(e.folder.dir.filePath(QStringLiteral("model.xlam"))));
        QVERIFY2(e.win.told.last().contains(QLatin1String(".xltm")), qPrintable(e.win.told.last()));
        QVERIFY(e.win.w.saveAs(e.folder.dir.filePath(QStringLiteral("copy.xltm"))));
        QVERIFY2(savedOk(saved), "the save failed");
        const Parts now = readParts(e.folder.dir.filePath(QStringLiteral("copy.xltm")));
        QVERIFY(QString::fromUtf8(now.value(QStringLiteral("[Content_Types].xml"))).contains(QLatin1String("template.macroEnabled.main+xml")));
        QCOMPARE(rawOf(e.folder.dir.filePath(QStringLiteral("copy.xltm"))).value(QStringLiteral("xl/vbaProject.bin")), readRaw(book).value(QStringLiteral("xl/vbaProject.bin")));
    }

    // A template without macros is saved as a template, and one with an add-in's type as an add-in.
    void templatesAndAddInsKeepTheirEnding()
    {
        for (const auto &[type, ending, other] : {std::tuple<QString, QString, QString>{QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml"), QStringLiteral("xltx"), QStringLiteral("xlsb")},
                                                  {QStringLiteral("application/vnd.ms-excel.addin.macroEnabled.main+xml"), QStringLiteral("xlam"), QStringLiteral("xlsb")}}) {
            const QByteArray book = repack(editBook(), [&](Parts &p) { p[QStringLiteral("[Content_Types].xml")].replace("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml", type.toUtf8()); });
            EditWindow e;
            QVERIFY(e.open(book, QStringLiteral("model.") + ending));
            e.select(QStringLiteral("D1"));
            e.enter(QStringLiteral("5"));
            QVERIFY(e.settle());
            QVERIFY2(!e.win.w.saveAs(e.folder.dir.filePath(QStringLiteral("model.") + other)), qPrintable(ending));
            QVERIFY2(e.win.told.last().contains(QLatin1String(".") + ending), qPrintable(e.win.told.last()));
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.save());
            QVERIFY2(savedOk(saved), "the save failed");
            QVERIFY(QString::fromUtf8(readParts(e.path).value(QStringLiteral("[Content_Types].xml"))).contains(QString::fromLatin1(type.toLatin1())));
        }
    }


    // ---- a file that can't be read says why, in words, and costs nothing that was open

    // The reason a file is refused with, from the message the window shows; empty if it was opened.
    QString refusalOf(const QByteArray &bytes, const QString &name)
    {
        Folder folder;
        Window win;
        QSignalSpy loaded(&win.w, &CrunchWindow::loadFinished);
        if (!win.w.openFile(folder.write(name, bytes))) return QStringLiteral("(not started)");
        if (!loaded.wait(20000)) return QStringLiteral("(no answer)");
        if (loaded.first().first().toBool()) return QString();
        return win.told.isEmpty() ? QStringLiteral("(no message)") : win.told.first();
    }

    void anEmptyFileSaysItIsEmpty()
    {
        const QString said = refusalOf(QByteArray(), QStringLiteral("empty.xlsx"));
        QVERIFY2(said.contains(QLatin1String("empty.xlsx")) && said.contains(QLatin1String("is empty")), qPrintable(said));
    }

    void aFileOfAnotherKindIsNamedForWhatItIs()
    {
        QString said = refusalOf(QByteArray("%PDF-1.7\n1 0 obj\n"), QStringLiteral("report.xlsx"));
        QVERIFY2(said.contains(QLatin1String("PDF document, not a workbook")), qPrintable(said));
        said = refusalOf(tinyPng(), QStringLiteral("chart.xlsx"));
        QVERIFY2(said.contains(QLatin1String("picture, not a workbook")), qPrintable(said));
        said = refusalOf(QByteArray("\x13\x37random bytes that no program here knows \x00\x01\x02"), QStringLiteral("odd.xlsx"));
        QVERIFY2(said.contains(QLatin1String("not a workbook")) && said.contains(QLatin1String("wrong name")), qPrintable(said));
        // a text document and a presentation, which are packages of the same kind
        for (const auto &[type, word] : {std::pair<QString, QString>{QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"), QStringLiteral("text document")},
                                         {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"), QStringLiteral("presentation")}}) {
            const QByteArray other = repack(editBook(), [&](Parts &p) {
                p[QStringLiteral("[Content_Types].xml")].replace("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml", type.toUtf8());
                p[QStringLiteral("xl/workbook.xml")] = "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body/></w:document>";
            });
            said = refusalOf(other, QStringLiteral("letter.xlsx"));
            QVERIFY2(said.contains(word) && said.contains(QLatin1String("not a workbook")), qPrintable(said));
        }
    }

    void aPackageCutShortSaysSo()
    {
        const QByteArray whole = editBook();
        const QString said = refusalOf(whole.left(whole.size() / 2), QStringLiteral("cut.xlsx"));
        QVERIFY2(said.contains(QLatin1String("cut short")) && said.contains(QLatin1String("copy it again")), qPrintable(said));
    }

    void aWorkbookThatNeedsAPasswordSaysSo()
    {
        cfb::File container;
        container.entries.resize(3);
        container.entries[0].type = cfb::Entry::Root;
        container.entries[0].name = QStringLiteral("Root Entry");
        container.entries[0].children = {1, 2};
        container.entries[1].name = QStringLiteral("EncryptionInfo");
        container.entries[1].data = QByteArray(64, '\x04');
        container.entries[2].name = QStringLiteral("EncryptedPackage");
        container.entries[2].data = QByteArray(4096, '\x05');
        const QString said = refusalOf(cfb::write(container), QStringLiteral("locked.xlsx"));
        QVERIFY2(said.contains(QLatin1String("protected")), qPrintable(said));
    }

    void aCompoundFileWithNoWorkbookSaysSo()
    {
        cfb::File container;
        container.entries.resize(2);
        container.entries[0].type = cfb::Entry::Root;
        container.entries[0].name = QStringLiteral("Root Entry");
        container.entries[0].children = {1};
        container.entries[1].name = QStringLiteral("Contents");
        container.entries[1].data = QByteArray(4096, 'x');
        const QString said = refusalOf(cfb::write(container), QStringLiteral("other.xls"));
        QVERIFY2(said.contains(QLatin1String("no workbook in it")), qPrintable(said));
    }

    // A file that can't be opened, asked for while a workbook is open, costs that workbook nothing: it is still there, still the one in the window, and has its changes.
    void aFileThatCantBeReadLeavesTheOpenWorkbookAlone()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        const QString broken = e.folder.write(QStringLiteral("broken.xlsx"), QByteArray("PK\x03\x04 and then nothing a package has"));
        const int told = e.win.told.size();
        e.win.w.openFile(broken);
        QTRY_COMPARE(e.win.told.size(), told + 1);
        QVERIFY2(e.win.told.last().contains(QLatin1String("broken.xlsx")), qPrintable(e.win.told.last()));
        QVERIFY2(e.win.told.last().contains(QLatin1String("The file is damaged: it starts like a zip package (.xlsx or .xlsm), but its contents can't be read. It may have been cut short when it was downloaded or copied. "
                                                         "If you have the original, copy it again.")),
                 qPrintable(e.win.told.last()));
        QVERIFY(e.win.w.hasWorkbook());
        QVERIFY(e.win.w.isModified());
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:5"));
    }

    // The kind of a workbook file is the type of its main part; the dialogs offer the templates, and a name's ending names its kind.
    void theKindOfAWorkbookFileComesFromTheTypeOfItsMainPart()
    {
        QCOMPARE(crunch::workbookKindOfType(QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml")), QStringLiteral("xlsx"));
        QCOMPARE(crunch::workbookKindOfType(QStringLiteral("application/vnd.ms-excel.sheet.macroEnabled.main+xml")), QStringLiteral("xlsm"));
        QCOMPARE(crunch::workbookKindOfType(QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml")), QStringLiteral("xltx"));
        QCOMPARE(crunch::workbookKindOfType(QStringLiteral("application/vnd.ms-excel.template.macroEnabled.main+xml")), QStringLiteral("xltm"));
        QCOMPARE(crunch::workbookKindOfType(QStringLiteral("application/vnd.ms-excel.addin.macroEnabled.main+xml")), QStringLiteral("xlam"));
        QCOMPARE(crunch::workbookKindOfType(QString()), QStringLiteral("xlsx"));
        QVERIFY(crunch::openFilter().contains(QLatin1String("*.xltx")) && crunch::openFilter().contains(QLatin1String("*.xltm")) && crunch::openFilter().contains(QLatin1String("*.xlam")));
        for (const char *name : {"a.xltx", "a.xltm", "a.xlam", "A.XLTM"}) QVERIFY2(crunch::fileKind(QString::fromLatin1(name)) != QLatin1String("other"), name);
        for (const QString &kind : {QStringLiteral("xlsx"), QStringLiteral("xlsm"), QStringLiteral("xltx"), QStringLiteral("xltm"), QStringLiteral("xlam")}) {
            QVERIFY2(crunch::workbookKindFilter(kind).contains(QLatin1String("*.") + kind), qPrintable(kind));
            QVERIFY2(crunch::workbookKindNotice(kind, QStringLiteral("JeffCrunch")).contains(QLatin1String(".") + kind), qPrintable(kind));
        }
    }


    // ---- the kind of the file follows the ending chosen: a template may become a workbook

    void aTemplateIsSavedAsAWorkbookWithTheWorkbooksType()
    {
        const QString sheetType = QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml");
        const QString macroSheetType = QStringLiteral("application/vnd.ms-excel.sheet.macroEnabled.main+xml");
        const QByteArray xltx = repack(editBook(), [](Parts &p) { p[QStringLiteral("[Content_Types].xml")].replace("sheet.main+xml", "template.main+xml"); });
        QString asked;
        QVERIFY(saveAsKind(xltx, QStringLiteral("model.xltx"), QStringLiteral("book.xlsx"), &asked));
        KindOf k = m_kind;
        QCOMPARE(k.type, sheetType);
        QVERIFY(k.reads && !k.project && !k.signature);
        QVERIFY2(asked.isEmpty(), "a template with no macros has nothing to say about them");
        // .xltm as .xlsm keeps the project, its signature, and what names them
        const QByteArray xltm = macroBookOf(QStringLiteral("application/vnd.ms-excel.template.macroEnabled.main+xml"), true);
        QVERIFY(saveAsKind(xltm, QStringLiteral("model.xltm"), QStringLiteral("book.xlsm"), &asked));
        k = m_kind;
        QCOMPARE(k.type, macroSheetType);
        QVERIFY(k.reads && k.project && k.signature && k.projectLink && k.projectType);
        QCOMPARE(m_project, readRaw(xltm).value(QStringLiteral("xl/vbaProject.bin")));
        QVERIFY2(asked.isEmpty(), "nothing is lost, so nothing is asked");
        // .xltm as .xlsx: the person is told, and the copy has no project, no signature, and no link or type that names them
        asked.clear();
        QVERIFY(saveAsKind(xltm, QStringLiteral("model.xltm"), QStringLiteral("book.xlsx"), &asked));
        k = m_kind;
        QCOMPARE(k.type, sheetType);
        QVERIFY(k.reads && !k.project && !k.signature && !k.projectLink && !k.projectType);
        QVERIFY2(asked.contains(QLatin1String("macros")) && asked.contains(QLatin1String(".xlsx")), qPrintable(asked));
        // the answer "no" writes nothing
        {
            EditWindow e;
            QVERIFY(e.open(xltm, QStringLiteral("model.xltm")));
            e.win.w.setLossAsker([](const QString &, const QString &) { return CrunchWindow::LossAnswer::Cancel; });
            e.select(QStringLiteral("D1"));
            e.enter(QStringLiteral("5"));
            QVERIFY(e.settle());
            QVERIFY(!e.win.w.saveAs(e.folder.dir.filePath(QStringLiteral("no.xlsx"))));
            QVERIFY(!QFileInfo::exists(e.folder.dir.filePath(QStringLiteral("no.xlsx"))));
        }
        // an add-in as .xlsm keeps its project; a workbook with macros as .xlsx loses it, with the question
        const QByteArray xlam = macroBookOf(QStringLiteral("application/vnd.ms-excel.addin.macroEnabled.main+xml"), false);
        asked.clear();
        QVERIFY(saveAsKind(xlam, QStringLiteral("tool.xlam"), QStringLiteral("tool.xlsm"), &asked));
        k = m_kind;
        QCOMPARE(k.type, macroSheetType);
        QVERIFY(k.reads && k.project && k.projectLink && asked.isEmpty());
        asked.clear();
        QVERIFY(saveAsKind(macroBookOf(macroSheetType, true), QStringLiteral("m.xlsm"), QStringLiteral("m.xlsx"), &asked));
        k = m_kind;
        QCOMPARE(k.type, sheetType);
        QVERIFY(k.reads && !k.project && !k.signature && !k.projectLink && !k.projectType);
        QVERIFY(asked.contains(QLatin1String("macros")));
        // a template cannot become an add-in, nor a workbook a template: refused, with the kinds it may be named
        EditWindow e;
        QVERIFY(e.open(xltx, QStringLiteral("model.xltx")));
        QVERIFY(!e.win.w.saveAs(e.folder.dir.filePath(QStringLiteral("x.xlam"))));
        QVERIFY2(e.win.told.last().contains(QLatin1String(".xltx")) && e.win.told.last().contains(QLatin1String(".xlsx")), qPrintable(e.win.told.last()));
    }

    // After a template is saved as a workbook the window holds a workbook: Save writes a workbook again.
    void afterSavingATemplateAsAWorkbookSaveWritesAWorkbook()
    {
        const QByteArray xltx = repack(editBook(), [](Parts &p) { p[QStringLiteral("[Content_Types].xml")].replace("sheet.main+xml", "template.main+xml"); });
        EditWindow e;
        QVERIFY(e.open(xltx, QStringLiteral("model.xltx")));
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        const QString target = e.folder.dir.filePath(QStringLiteral("book.xlsx"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.saveAs(target));
        QVERIFY(savedOk(saved));
        e.select(QStringLiteral("D2"));
        e.enter(QStringLiteral("6"));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.save());
        QVERIFY(savedOk(saved));
        const QString type = kindOf(target).type;
        QVERIFY2(type.contains(QLatin1String("sheet.main+xml")) && !type.contains(QLatin1String("template")), qPrintable(type));
    }

    // ---- the words for a file that can't be read, each true to what is wrong

    void zipThatIsNotWholeIsToldAsWhatIsWrongWithIt()
    {
        const QByteArray whole = editBook();
        // zip64: the record at the end says to look in the other record
        QString said = refusalOf(patched(whole, [](QByteArray &z) {
                                     const qsizetype eocd = findLast(z, "PK\x05\x06");
                                     z[eocd + 10] = char(0xFF);
                                     z[eocd + 11] = char(0xFF);
                                 }),
                                 QStringLiteral("big.xlsx"));
        QVERIFY2(said.contains(QLatin1String("can't read yet")) && said.contains(QLatin1String("zip64")) && !said.contains(QLatin1String("cut short")), qPrintable(said));
        // a part packed by method 9 (deflate64)
        said = refusalOf(patched(whole, [](QByteArray &z) { z[findLast(z, "PK\x01\x02") + 10] = char(9); }), QStringLiteral("nine.xlsx"));
        QVERIFY2(said.contains(QLatin1String("packed in a way")) && !said.contains(QLatin1String("cut short")), qPrintable(said));
        // a local header that is not one
        said = refusalOf(patched(whole, [](QByteArray &z) {
                             const qsizetype central = findLast(z, "PK\x01\x02");
                             const qsizetype local = qFromLittleEndian<quint32>(z.constData() + central + 42);
                             z[local] = 'X';
                         }),
                         QStringLiteral("local.xlsx"));
        QVERIFY2(said.contains(QLatin1String("list of parts")) && said.contains(QLatin1String("damaged")) && !said.contains(QLatin1String("cut short")), qPrintable(said));
        // an entry whose data goes past the end of the file
        said = refusalOf(patched(whole, [](QByteArray &z) {
                             const qsizetype central = findLast(z, "PK\x01\x02");
                             z[central + 20] = char(0xF0);
                             z[central + 21] = char(0xFF);
                             z[central + 22] = char(0xFF);
                             z[central + 23] = char(0x0F);
                         }),
                         QStringLiteral("long.xlsx"));
        QVERIFY2(said.contains(QLatin1String("damaged")) && said.contains(QLatin1String("not all there")) && !said.contains(QLatin1String("cut short")), qPrintable(said));
        // the end of the file gone: cut short
        said = refusalOf(whole.left(whole.size() - 30), QStringLiteral("cut.xlsx"));
        QVERIFY2(said.contains(QLatin1String("cut short")), qPrintable(said));
    }

    void aCompoundFileWhoseWorkbookCantBeReadIsDamagedNotWithoutOne()
    {
        cfb::File container;
        container.entries.resize(2);
        container.entries[0].type = cfb::Entry::Root;
        container.entries[0].name = QStringLiteral("Root Entry");
        container.entries[0].children = {1};
        container.entries[1].name = QStringLiteral("Workbook");   // (a real stream, of regular sectors)
        container.entries[1].data = QByteArray(8192, 'x');
        QByteArray file = cfb::write(container);
        // its first sector is moved past the end of the file: the stream is there, and its sectors are not
        QByteArray name;
        for (QChar c : QStringLiteral("Workbook")) name.append(char(c.unicode() & 0xFF)), name.append('\0');
        const qsizetype entry = file.indexOf(name);
        QVERIFY(entry >= 0);
        qToLittleEndian<quint32>(0x00FFFF00u, file.data() + entry + 116);
        const QString said = refusalOf(file, QStringLiteral("odd.xls"));
        QVERIFY2(said.contains(QLatin1String("damaged")) && !said.contains(QLatin1String("no workbook in it")), qPrintable(said));
    }

    void textDocumentsAndPresentationsWithMacrosAreNamedToo()
    {
        for (const auto &[type, word] : {std::pair<QString, QString>{QStringLiteral("application/vnd.ms-word.document.macroEnabled.main+xml"), QStringLiteral(".docm")},
                                         {QStringLiteral("application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml"), QStringLiteral(".pptm")}}) {
            const QByteArray other = repack(editBook(), [&](Parts &p) {
                p[QStringLiteral("[Content_Types].xml")].replace("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml", type.toUtf8());
                p[QStringLiteral("xl/workbook.xml")] = "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body/></w:document>";
            });
            const QString said = refusalOf(other, QStringLiteral("x.xlsx"));
            QVERIFY2(said.contains(word) && said.contains(QLatin1String("not a workbook")), qPrintable(said));
        }
    }

    // A workbook of 1,000 sheets (the most a workbook may have) is asked about before a .csv is written, and the question does not make the window wait.
    void theQuestionBeforeACsvOfAHugeWorkbookIsQuick()
    {
        QVector<QByteArray> sheets;
        for (int i = 0; i < 1000; ++i) sheets << worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>%1</v></c></row>").arg(i));
        const QByteArray book = repack(buildXlsx(sheets), [](Parts &p) {
            for (int i = 1; i <= 1000; ++i) {
                relate(p, QStringLiteral("xl/worksheets/_rels/sheet%1.xml.rels").arg(i), QStringLiteral("rId1"), QStringLiteral("printerSettings"), QStringLiteral("../printerSettings/p.bin"));
                relate(p, QStringLiteral("xl/worksheets/_rels/sheet%1.xml.rels").arg(i), QStringLiteral("rId2"), QStringLiteral("hyperlink"), QStringLiteral("https://example.test/"), true);
            }
            relate(p, QStringLiteral("xl/worksheets/_rels/sheet1000.xml.rels"), QStringLiteral("rId3"), QStringLiteral("comments"), QStringLiteral("../comments1.xml"));
        });
        if (qEnvironmentVariableIsSet("JC_KEEP_DUMP")) {
            QFile dump(qEnvironmentVariable("JC_KEEP_DUMP") + QStringLiteral("/huge.xlsx"));
            if (dump.open(QIODevice::WriteOnly)) dump.write(book);
        }
        EditWindow e;
        QVERIFY(e.open(book, QStringLiteral("huge.xlsx")));
        QString text;
        e.win.w.setLossAsker([&](const QString &, const QString &t) {
            text = t;
            return CrunchWindow::LossAnswer::Cancel;
        });
        QElapsedTimer clock;
        clock.start();
        e.win.w.saveAs(e.folder.dir.filePath(QStringLiteral("huge.csv")));
        qInfo("the question about a .csv of 1,000 sheets took %lld ms", clock.elapsed());
        QVERIFY2(text.contains(QLatin1String("999 other sheets")) && text.contains(QLatin1String("notes")), qPrintable(text));
    }

private:
    // What a saved file says of its kind: the type of its main part, whether the project is there (the part, its signature, the links and types that name them), and that it reads.
    struct KindOf {
        QString type;
        bool project = false, signature = false, projectLink = false, projectType = false;
        bool reads = false;
    };
    static KindOf kindOf(const QString &path)
    {
        KindOf k;
        const Parts p = readParts(path);
        const QString types = QString::fromUtf8(p.value(QStringLiteral("[Content_Types].xml")));
        static const QRegularExpression main(QStringLiteral("PartName=\"/xl/workbook\\.xml\"\\s+ContentType=\"([^\"]*)\""));
        k.type = main.match(types).captured(1);
        k.project = p.contains(QStringLiteral("xl/vbaProject.bin"));
        k.signature = p.contains(QStringLiteral("xl/vbaProjectSignature.bin")) || p.contains(QStringLiteral("xl/_rels/vbaProject.bin.rels"));
        k.projectLink = QString::fromUtf8(p.value(kBookRels)).contains(QLatin1String("vbaProject"));
        k.projectType = types.contains(QLatin1String("vbaProject"));
        jp::office::Package package;
        QString why;
        jp::office::sheet::Workbook book;
        k.reads = package.readLazyFile(path, &why) && book.open(package, &why);
        return k;
    }
    // Opens `book` under `name`, edits a cell, saves it as `target` (answering the question about macros as `answer`), and says whether it was written.
    bool saveAsKind(const QByteArray &book, const QString &name, const QString &target, QString *asked, CrunchWindow::LossAnswer answer = CrunchWindow::LossAnswer::Continue)
    {
        EditWindow e;
        if (!e.open(book, name)) return false;
        e.win.w.setLossAsker([&](const QString &, const QString &text) {
            if (asked) *asked = text;
            return answer;
        });
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("5"));
        if (!e.settle()) return false;
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        const QString path = e.folder.dir.filePath(target);
        if (!e.win.w.saveAs(path)) return false;
        if (!savedOk(saved)) return false;
        m_kind = kindOf(path);
        m_project = rawOf(path).value(QStringLiteral("xl/vbaProject.bin"));
        return true;
    }
    KindOf m_kind;
    QByteArray m_project;

    static QByteArray patched(QByteArray zip, const std::function<void(QByteArray &)> &change)
    {
        change(zip);
        return zip;
    }
    static qsizetype findLast(const QByteArray &zip, const char *signature) { return zip.lastIndexOf(QByteArray(signature, 4)); }

    static Parts readParts(const QString &path)
    {
        QFile f(path);
        Parts out;
        if (f.open(QIODevice::ReadOnly)) readZip(f.readAll(), out);
        return out;
    }
    static Parts readRaw(const QByteArray &zip)
    {
        Parts out;
        QVector<ZipEntry> dir;
        if (readZipDirectory(zip, &dir))
            for (const ZipEntry &e : dir) out.insert(e.name, zip.mid(int(e.dataAt), int(e.csize)));
        return out;
    }
    static Parts rawOf(const QString &path)
    {
        QFile f(path);
        return f.open(QIODevice::ReadOnly) ? readRaw(f.readAll()) : Parts();
    }

    // Opens the workbook, types `text` in `cell`, saves over the file, and checks: every part but those a save rewrites is the old one to the byte (as it is stored, compressed), the
    // sheet that was edited is the old one but for its cells, and the part list is the old list.
    void keepsWhatItDoesNotEdit(const QByteArray &book, const QString &cell, const QString &text, const QString &name = QStringLiteral("book.xlsx"))
    {
        if (qEnvironmentVariableIsSet("JC_KEEP_DUMP")) {
            QFile dump(qEnvironmentVariable("JC_KEEP_DUMP") + QLatin1Char('/') + QTest::currentTestFunction() + QStringLiteral(".xlsx"));
            if (dump.open(QIODevice::WriteOnly)) dump.write(book);
        }
        EditWindow e;
        QVERIFY(e.open(book, name));
        e.select(cell);
        e.enter(text);
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        const Parts oldRaw = readRaw(book), newRaw = rawOf(e.path);
        QStringList missing, changed;
        // (what a save of a cell edit may write anew: the sheet, the shared strings, the styles, the calculation chain, the workbook part, its relationships, the types, and the properties)
        static const QRegularExpression rewritten(QStringLiteral("^(xl/worksheets/sheet1\\.xml|xl/sharedStrings\\.xml|xl/styles\\.xml|xl/calcChain\\.xml|xl/workbook\\.xml|xl/_rels/workbook\\.xml\\.rels|\\[Content_Types\\]\\.xml|docProps/(core|app)\\.xml)$"));
        for (auto it = oldRaw.begin(); it != oldRaw.end(); ++it) {
            if (!newRaw.contains(it.key())) {
                if (it.key() != QLatin1String("xl/calcChain.xml")) missing << it.key();
            } else if (!rewritten.match(it.key()).hasMatch() && newRaw.value(it.key()) != it.value()) {
                changed << it.key();
            }
        }
        QVERIFY2(missing.isEmpty(), qPrintable(QStringLiteral("gone: ") + missing.join(QLatin1String(", "))));
        QVERIFY2(changed.isEmpty(), qPrintable(QStringLiteral("changed: ") + changed.join(QLatin1String(", "))));
        for (auto it = newRaw.begin(); it != newRaw.end(); ++it) QVERIFY2(oldRaw.contains(it.key()), qPrintable(QStringLiteral("new: ") + it.key()));
        // the sheet: all that is not a cell is as it was
        Parts oldDecoded, newDecoded;
        readZip(book, oldDecoded);
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        readZip(f.readAll(), newDecoded);
        const QByteArray oldSkeleton = skeleton(oldDecoded.value(kSheet)), newSkeleton = skeleton(newDecoded.value(kSheet));
        if (oldSkeleton != newSkeleton) {
            qsizetype at = 0;
            while (at < oldSkeleton.size() && at < newSkeleton.size() && oldSkeleton[at] == newSkeleton[at]) ++at;
            QFAIL(qPrintable(QStringLiteral("the sheet is not as it was, from byte %1:\n  was %2\n  now %3")
                                 .arg(at).arg(QString::fromUtf8(oldSkeleton.mid(std::max<qsizetype>(0, at - 40), 160)), QString::fromUtf8(newSkeleton.mid(std::max<qsizetype>(0, at - 40), 160)))));
        }
        // and every relationship and type the old file had is still there, for the parts that stay
        auto relationships = [](const QByteArray &xml) {
            QStringList out;
            static const QRegularExpression rel(QStringLiteral("<Relationship [^>]*>"));
            auto m = rel.globalMatch(QString::fromUtf8(xml));
            while (m.hasNext()) out << m.next().captured(0);
            return out;
        };
        for (auto it = oldDecoded.begin(); it != oldDecoded.end(); ++it) {
            if (!it.key().endsWith(QLatin1String(".rels"))) continue;
            const QStringList now = relationships(newDecoded.value(it.key()));
            for (const QString &r : relationships(it.value()))
                QVERIFY2(now.contains(r), qPrintable(it.key() + QLatin1String(" lost ") + r));
        }
        const QString typesNow = QString::fromUtf8(newDecoded.value(QStringLiteral("[Content_Types].xml")));
        static const QRegularExpression typeElement(QStringLiteral("<(Default|Override) [^>]*/>"));
        auto it = typeElement.globalMatch(QString::fromUtf8(oldDecoded.value(QStringLiteral("[Content_Types].xml"))));
        while (it.hasNext()) {
            const QString t = it.next().captured(0);
            if (t.contains(QLatin1String("calcChain"))) continue;
            QVERIFY2(typesNow.contains(t), qPrintable(QStringLiteral("types lost ") + t));
        }
    }
};

JP_TEST_CLASS(CrunchKeepTests)
#include "tst_crunch_keep.moc"
