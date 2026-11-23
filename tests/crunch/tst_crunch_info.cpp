// Tests of JeffCrunch's Info page: the properties a workbook keeps (written at the next save, into parts the file may not have had), Mark as Final, and Inspect Workbook. Every workbook is
// built here.
#include <QLocale>
#include "app/anchordialogs.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/filescreen.h"
#include "app/settings.h"
#include "app/theme.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/crunchtemplates.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "findbooks.h"
#include "core/lossnotes.h"
#include "sheet/oldsave.h"
#include "office/package.h"
#include "editwindow.h"
#include "office/packageprops.h"
#include "../localepin.h"
#include "../office/cfbdamage.h"
#include "../sheet/drawinghelpers.h"
#include "sheet/xlsxsave.h"
#include "sheet/drawingsave.h"
#include "core/sheetobjects.h"
#include "../privacyhooks.h"
#include "../packedpackagecheck.h"
#include "office/privacy.h"

#include <QDir>
#include <QScrollArea>
#include <QScrollBar>
#include <QAction>
#include <QThread>
#include <atomic>
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QScopeGuard>
#include <QLineEdit>
#include <QSettings>
#include <QSignalSpy>
#include <QCheckBox>
#include <QTemporaryDir>
#include <memory>

using namespace jp;
using namespace crunchtest;

namespace {

QMap<QString, QByteArray> partsOf(const QString &path)
{
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QMap<QString, QByteArray> parts;
    jp::readZip(f.readAll(), parts);
    return parts;
}

bool saveAndWaitHere(CrunchWindow &w)
{
    QSignalSpy saved(&w, &CrunchWindow::saved);
    if (!w.save()) return false;
    return (saved.count() > 0 || saved.wait(20000)) && saved.last().value(1).toBool();
}

// A workbook with what people really have: a formula, two sheets, a defined name, a number format, a fill and borders, merged cells, a note, a chart, a picture, a table, a filter, conditional
// formatting, data validation, frozen panes, a link, a column width, print settings, and a formula that spills.
QByteArray everythingBook()
{
    auto text = [](const QString &ref, const QString &t, const QString &style = QString()) {
        return QStringLiteral("<c r=\"%1\"%3 t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, t, style.isEmpty() ? QString() : QStringLiteral(" s=\"%1\"").arg(style));
    };
    const QString rows = QStringLiteral("<row r=\"1\">") + text("A1", "Item") + QStringLiteral("<c r=\"B1\" s=\"1\"><v>1234.5</v></c><c r=\"C1\"><f>B1*2</f><v>2469</v></c>") + text("E1", "Key") + text("F1", "Val")
        + QStringLiteral("</row><row r=\"2\">") + text("A2", "go") + QStringLiteral("<c r=\"B2\" s=\"2\"><v>5</v></c>") + text("E2", "a") + QStringLiteral("<c r=\"F2\"><v>1</v></c></row><row r=\"3\">")
        + text("A3", "merged") + text("E3", "b") + QStringLiteral("<c r=\"F3\"><v>2</v></c></row><row r=\"4\">") + text("A4", "noted") + QStringLiteral("<c r=\"B4\"><v>-3</v></c></row>")
        + QStringLiteral("<row r=\"5\"><c r=\"A5\" cm=\"1\"><f t=\"array\" ref=\"A5:A7\">_xlfn.SEQUENCE(3)</f><v>1</v></c></row><row r=\"6\"><c r=\"A6\"><v>2</v></c></row><row r=\"7\"><c r=\"A7\"><v>3</v></c></row>");
    const QString head = QStringLiteral("<dimension ref=\"A1:F7\"/><sheetViews><sheetView workbookViewId=\"0\"><pane xSplit=\"1\" ySplit=\"1\" topLeftCell=\"B2\" activePane=\"bottomRight\" state=\"frozen\"/></sheetView></sheetViews>"
                                        "<cols><col min=\"1\" max=\"1\" width=\"28\" customWidth=\"1\"/></cols>");
    const QString after = QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A3:B3\"/></mergeCells>"
                                         "<conditionalFormatting sqref=\"B1:B4\"><cfRule type=\"cellIs\" priority=\"1\" operator=\"lessThan\"><formula>0</formula></cfRule></conditionalFormatting>"
                                         "<dataValidations count=\"1\"><dataValidation type=\"list\" sqref=\"D2\"><formula1>\"yes,no\"</formula1></dataValidation></dataValidations>"
                                         "<hyperlinks><hyperlink ref=\"A2\" r:id=\"rIdH\" display=\"go\"/></hyperlinks>"
                                         "<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/><pageSetup orientation=\"landscape\" paperSize=\"9\"/>"
                                         "<drawing r:id=\"rIdD1\"/><legacyDrawing r:id=\"rIdV\"/>");
    const QByteArray second = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>second</t></is></c></row><row r=\"2\"><c r=\"A2\"><v>4</v></c></row><row r=\"3\"><c r=\"A3\"><v>9</v></c></row>"),
                                        QStringLiteral("<autoFilter ref=\"A1:A3\"/>"));
    const QByteArray styles = stylesXml(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"#,##0.00 &quot;units&quot;\"/>"),
                                        QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/><xf numFmtId=\"164\" fontId=\"0\" fillId=\"0\" borderId=\"0\" applyNumberFormat=\"1\"/>"
                                                       "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"2\" borderId=\"1\" applyFill=\"1\" applyBorder=\"1\"/>"),
                                        QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>"
                                                       "<fills count=\"3\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"
                                                       "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FFFFFF00\"/><bgColor indexed=\"64\"/></patternFill></fill></fills>"
                                                       "<borders count=\"2\"><border><left/><right/><top/><bottom/><diagonal/></border>"
                                                       "<border><left style=\"thin\"><color auto=\"1\"/></left><right style=\"thin\"><color auto=\"1\"/></right><top style=\"thin\"><color auto=\"1\"/></top>"
                                                       "<bottom style=\"thin\"><color auto=\"1\"/></bottom><diagonal/></border></borders>"));
    QByteArray book = buildXlsx({worksheet(rows, after, head), second}, QByteArray(), QString(), QString(), styles, QStringList({"Data", "Notes"}), QStringLiteral("<definedName name=\"Total\">Data!$B$1:$B$4</definedName>"));
    book = withTables(book, {QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" id=\"1\" name=\"Pairs\" displayName=\"Pairs\" ref=\"E1:F3\" totalsRowShown=\"0\">"
                                            "<autoFilter ref=\"E1:F3\"/><tableColumns count=\"2\"><tableColumn id=\"1\" name=\"Key\"/><tableColumn id=\"2\" name=\"Val\"/></tableColumns><tableStyleInfo name=\"TableStyleMedium2\" showRowStripes=\"1\"/></table>")});
    drawtest::SeriesSpec series;
    series.valRef = QStringLiteral("Data!$B$1:$B$4");
    series.values = {1234.5, 5, 0, -3};
    QVector<ExtraPart> parts;
    auto add = [&](const QString &name, const QByteArray &data, const QString &type, int sheet = -1, const QString &id = QString(), const QString &rel = QString(), const QString &target = QString()) {
        parts.append({name, data, type, sheet, id, rel, target});
    };
    add(QStringLiteral("xl/drawings/drawing1.xml"), drawtest::drawingPart(drawtest::twoCell(7, 0, 12, 8, drawtest::chartFrame(2, QStringLiteral("rIdC1"))) + drawtest::twoCell(7, 9, 9, 12, drawtest::pictureElement(3, QStringLiteral("rIdI1")))),
        QStringLiteral("application/vnd.openxmlformats-officedocument.drawing+xml"), 0, QStringLiteral("rIdD1"), QStringLiteral("drawing"), QStringLiteral("../drawings/drawing1.xml"));
    add(QStringLiteral("xl/charts/chart1.xml"), drawtest::chartPart(QStringLiteral("barChart"), {series}), QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"));
    add(QStringLiteral("xl/media/image1.png"), drawtest::pngBytes(20, 20, Qt::red), QStringLiteral("image/png"));
    add(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"),
        QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                   "<Relationship Id=\"rIdC1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart\" Target=\"/xl/charts/chart1.xml\"/>"
                   "<Relationship Id=\"rIdI1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"/xl/media/image1.png\"/></Relationships>"),
        QString());
    add(QStringLiteral("xl/comments1.xml"),
        QByteArray("<?xml version=\"1.0\"?><comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors><author>a</author></authors><commentList><comment ref=\"A4\" authorId=\"0\"><text><t>Note words</t></text></comment></commentList></comments>"),
        QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml"), 0, QStringLiteral("rIdC"), QStringLiteral("comments"), QStringLiteral("../comments1.xml"));
    add(QStringLiteral("xl/drawings/vmlDrawing1.vml"),
        QByteArray("<xml><v:shape id=\"s1\"><x:ClientData ObjectType=\"Note\"><x:Anchor>1, 15, 3, 2, 3, 15, 5, 16</x:Anchor><x:Row>3</x:Row><x:Column>0</x:Column></x:ClientData></v:shape></xml>"), QString(), 0, QStringLiteral("rIdV"),
        QStringLiteral("vmlDrawing"), QStringLiteral("../drawings/vmlDrawing1.vml"));
    add(QStringLiteral("xl/metadata.xml"), office::sheet::dynamicArrayMetadataXml(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheetMetadata+xml"));
    book = withParts(book, parts);
    // (the link goes to a web address: its relationship is external)
    QMap<QString, QByteArray> zipParts;
    jp::readZip(book, zipParts);
    QByteArray &rels = zipParts[QStringLiteral("xl/worksheets/_rels/sheet1.xml.rels")];
    rels.replace("</Relationships>", "<Relationship Id=\"rIdH\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink\" Target=\"https://example.com/\" TargetMode=\"External\"/></Relationships>");
    jp::ZipWriter zip;
    for (auto it = zipParts.cbegin(); it != zipParts.cend(); ++it) zip.add(it.key(), it.value(), true);
    return zip.finish();
}

// The element with its object set to be hidden.
QString hideElement(QString element) { return element.replace(QStringLiteral("<xdr:cNvPr "), QStringLiteral("<xdr:cNvPr hidden=\"1\" ")); }

const QString kMc = QStringLiteral("http://schemas.openxmlformats.org/markup-compatibility/2006");

// A workbook whose first sheet has the drawing that the anchors make (and the parts it names).
QByteArray drawingBookOf(const QString &anchors, const QVector<drawtest::DrawingTarget> &targets = {})
{
    return drawtest::bookWithDrawing({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"))}, drawtest::drawingPart(anchors), targets);
}

InspectItem inspectItemOf(EditWindow &e, const char *title)
{
    for (const InspectItem &i : e.win.w.inspectItems())
        if (i.title == QLatin1String(title)) return i;
    return InspectItem();
}

// A workbook whose drawing holds a picture that shows and four objects set to be hidden (a chart, a picture, a shape, and a text box), each with a name that can be looked for in the file.
QByteArray hiddenObjectsBook()
{
    auto hide = [](QString element) { return element.replace(QStringLiteral("<xdr:cNvPr "), QStringLiteral("<xdr:cNvPr hidden=\"1\" ")); };
    QString textBox = hide(drawtest::shapeElement(6, QStringLiteral("HiddenSecretTextBox"), QStringLiteral("rect"), QStringLiteral("HiddenSecretWords")));
    textBox.replace(QStringLiteral("<xdr:cNvSpPr/>"), QStringLiteral("<xdr:cNvSpPr txBox=\"1\"/>"));
    const QString anchors = drawtest::twoCell(7, 0, 12, 8, hide(drawtest::chartFrame(2, QStringLiteral("rIdC1"), QStringLiteral("HiddenSecretChart"))))
        + drawtest::twoCell(7, 9, 9, 12, drawtest::pictureElement(3, QStringLiteral("rIdI1"), QStringLiteral("ShownPicture")))
        + drawtest::twoCell(1, 9, 3, 12, hide(drawtest::pictureElement(4, QStringLiteral("rIdI2"), QStringLiteral("HiddenSecretPicture"))))
        + drawtest::twoCell(1, 13, 3, 15, hide(drawtest::shapeElement(5, QStringLiteral("HiddenSecretShape"), QStringLiteral("rect"), QStringLiteral("HiddenSecretLabel"))))
        + drawtest::twoCell(4, 13, 6, 15, textBox)
        + drawtest::twoCell(10, 9, 12, 12, hide(drawtest::pictureElement(7, QStringLiteral("rIdI1"), QStringLiteral("HiddenSharedPicture"))));
    drawtest::SeriesSpec series;
    series.valRef = QStringLiteral("Sheet1!$A$1:$A$2");
    series.values = {1, 2};
    QVector<ExtraPart> parts;
    auto add = [&](const QString &name, const QByteArray &data, const QString &type, int sheet = -1, const QString &id = QString(), const QString &rel = QString(), const QString &target = QString()) {
        parts.append({name, data, type, sheet, id, rel, target});
    };
    add(QStringLiteral("xl/drawings/drawing1.xml"), drawtest::drawingPart(anchors), QStringLiteral("application/vnd.openxmlformats-officedocument.drawing+xml"), 0, QStringLiteral("rIdD1"), QStringLiteral("drawing"),
        QStringLiteral("../drawings/drawing1.xml"));
    QByteArray chart = drawtest::chartPart(QStringLiteral("barChart"), {series});
    chart.replace("</c:chartSpace>", "<c:externalData r:id=\"rIdW1\"><c:autoUpdate val=\"0\"/></c:externalData></c:chartSpace>");
    add(QStringLiteral("xl/charts/chart1.xml"), chart, QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"));
    add(QStringLiteral("xl/charts/_rels/chart1.xml.rels"),
        QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                   "<Relationship Id=\"rIdW1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/package\" Target=\"/xl/embeddings/chartbook1.xlsx\"/></Relationships>"),
        QString());
    add(QStringLiteral("xl/embeddings/chartbook1.xlsx"), QByteArray("HiddenSecretEmbeddedBook"), QStringLiteral("application/octet-stream"));
    add(QStringLiteral("xl/media/image1.png"), drawtest::pngBytes(20, 20, Qt::red), QStringLiteral("image/png"));
    add(QStringLiteral("xl/media/image2.png"), drawtest::pngBytes(24, 24, Qt::blue), QStringLiteral("image/png"));
    add(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"),
        QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                   "<Relationship Id=\"rIdC1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart\" Target=\"/xl/charts/chart1.xml\"/>"
                   "<Relationship Id=\"rIdI1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"/xl/media/image1.png\"/>"
                   "<Relationship Id=\"rIdI2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"/xl/media/image2.png\"/></Relationships>"),
        QString());
    const QByteArray book = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\"><c r=\"A2\"><v>2</v></c></row>"), QStringLiteral("<drawing r:id=\"rIdD1\"/>"))});
    return withParts(book, parts);
}

// Whether the bytes hold the text, as the file writes it (one byte a letter, or two as the old form writes it).
bool holdsText(const QByteArray &bytes, const QString &text)
{
    return bytes.contains(text.toUtf8()) || bytes.contains(QByteArray(reinterpret_cast<const char *>(text.utf16()), qsizetype(text.size() * 2)));
}

bool fileHolds(const QString &path, const QString &text)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    const QByteArray bytes = f.readAll();
    QMap<QString, QByteArray> parts;
    if (!jp::readZip(bytes, parts)) return holdsText(bytes, text);
    for (auto it = parts.cbegin(); it != parts.cend(); ++it)
        if (holdsText(it.value(), text)) return true;
    return false;
}

} // namespace

class CrunchInfoTests : public QObject {
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
    }
    void init() { Settings::get().setValue(QStringLiteral("user/name"), QStringLiteral("Tester")); }

    void propertiesAreWrittenIntoPartsTheFileDidNotHave()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(!partsOf(e.path).contains(QStringLiteral("docProps/core.xml")));
        office::PackageProps p = e.win.w.properties();
        p.title = QStringLiteral("Budget 2027");
        p.creator = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        p.custom = {{QStringLiteral("Project"), QStringLiteral("lpwstr"), QStringLiteral("Apollo")}};
        e.win.w.applyProperties(p);
        QVERIFY(e.win.w.isWindowModified());
        QVERIFY(saveAndWaitHere(e.win.w));
        const auto parts = partsOf(e.path);
        QVERIFY(parts.contains(QStringLiteral("docProps/core.xml")));
        QVERIFY(parts.contains(QStringLiteral("docProps/app.xml")));
        QVERIFY(parts.contains(QStringLiteral("docProps/custom.xml")));
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("/docProps/core.xml"));
        QVERIFY(parts.value(QStringLiteral("[Content_Types].xml")).contains("custom-properties+xml"));
        QVERIFY(parts.value(QStringLiteral("_rels/.rels")).contains("metadata/core-properties"));
        QVERIFY(parts.value(QStringLiteral("_rels/.rels")).contains("custom-properties"));
        // reopened, the workbook has them
        const QString second = e.folder.dir.filePath(QStringLiteral("second.xlsx"));
        QVERIFY(QFile::copy(e.path, second));
        Window again;
        QVERIFY(again.open(second));
        const office::PackageProps back = again.w.properties();
        QCOMPARE(back.title, QStringLiteral("Budget 2027"));
        QCOMPARE(back.creator, QStringLiteral("Ann Lee"));
        QCOMPARE(back.company, QStringLiteral("Acme"));
        QCOMPARE(back.custom.size(), 1);
        QVERIFY(back.created.isValid());
        // and a second change replaces the parts that are there now
        p = again.w.properties();
        p.title = QStringLiteral("Budget 2028");
        p.custom.clear();
        again.w.applyProperties(p);
        QVERIFY(saveAndWaitHere(again.w));
        const auto after = partsOf(second);
        QVERIFY(after.value(QStringLiteral("docProps/core.xml")).contains("Budget 2028"));
        QVERIFY(!after.contains(QStringLiteral("docProps/custom.xml")));
        QVERIFY(!after.value(QStringLiteral("_rels/.rels")).contains("custom-properties"));
        QVERIFY(!after.value(QStringLiteral("[Content_Types].xml")).contains("custom-properties"));
    }

    // A version of a workbook that asks for a password is kept protected, and a password put on, changed, or taken off is put on the versions already kept.
    void versionsOfAProtectedWorkbookAreProtected()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        write::VersionStore store(e.path, QStringLiteral(".xlsx"));
        store.removeAll();
        auto firstVersion = [&] { return store.read(e.win.w.versions().first().file); };
        QSignalSpy kept(&e.win.w, &CrunchWindow::versionKept);
        QVERIFY(e.win.w.saveVersion(QStringLiteral("Plain")));
        QVERIFY(kept.count() > 0 || kept.wait(20000));
        QVERIFY(firstVersion().startsWith("PK"));
        auto setPassword = [&](const QString &password, bool remove) {
            e.win.w.anchorHooks().setFilePasswordAsker = [=](SetFilePasswordRequest *r) {
                r->password = password;
                r->remove = remove;
                return true;
            };
            e.win.w.act(QStringLiteral("protect.password"))->trigger();
        };
        setPassword(QStringLiteral("one"), false);
        QVERIFY(firstVersion().startsWith("PK"));   // (the file is not saved with it yet)
        QVERIFY(saveAndWaitHere(e.win.w));
        QVERIFY(firstVersion().startsWith(QByteArray("\xD0\xCF\x11\xE0", 4)));
        kept.clear();
        QVERIFY(e.win.w.saveVersion(QStringLiteral("Protected")));
        QVERIFY(kept.count() > 0 || kept.wait(20000));
        QVERIFY(firstVersion().startsWith(QByteArray("\xD0\xCF\x11\xE0", 4)));
        setPassword(QString(), true);
        QVERIFY(saveAndWaitHere(e.win.w));
        for (const write::DocVersion &v : e.win.w.versions()) QVERIFY2(store.read(v.file).startsWith("PK"), qPrintable(v.name));
        e.win.w.setWindowModified(false);
        store.removeAll();
    }

    // A password taken off or changed and then not saved leaves the versions as the file on disk has them: protected, and opening with the file's password.
    void aPasswordChangeThatIsNotSavedLeavesTheVersionsAlone()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        write::VersionStore store(e.path, QStringLiteral(".xlsx"));
        store.removeAll();
        auto setPassword = [&](const QString &password, bool remove) {
            e.win.w.anchorHooks().setFilePasswordAsker = [=](SetFilePasswordRequest *r) {
                r->password = password;
                r->remove = remove;
                return true;
            };
            e.win.w.act(QStringLiteral("protect.password"))->trigger();
        };
        QSignalSpy kept(&e.win.w, &CrunchWindow::versionKept);
        auto keep = [&](const char *name) {
            kept.clear();
            return e.win.w.saveVersion(QString::fromLatin1(name)) && (kept.count() > 0 || kept.wait(20000));
        };
        setPassword(QStringLiteral("one"), false);
        QVERIFY(saveAndWaitHere(e.win.w));
        QVERIFY(keep("V"));
        auto opensWith = [&](const QString &password) {
            QByteArray package;
            return write::versionPackage(store.read(e.win.w.versions().first().file), password, &package);
        };
        QVERIFY(store.read(e.win.w.versions().first().file).startsWith(QByteArray("\xD0\xCF\x11\xE0", 4)));
        setPassword(QString(), true);
        QVERIFY(keep("W"));
        for (const write::DocVersion &v : e.win.w.versions()) QVERIFY2(store.read(v.file).startsWith(QByteArray("\xD0\xCF\x11\xE0", 4)), qPrintable(v.name));
        setPassword(QStringLiteral("two"), false);
        QVERIFY(opensWith(QStringLiteral("one")));
        QVERIFY(!opensWith(QStringLiteral("two")));
        QVERIFY(saveAndWaitHere(e.win.w));
        QVERIFY(opensWith(QStringLiteral("two")));
        QVERIFY(!opensWith(QStringLiteral("one")));
        e.win.w.setWindowModified(false);
        store.removeAll();
    }

    void aWorkbookMarkedAsFinalOpensForReadingOnly()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.setMarkedFinal(true);
        QTRY_VERIFY_WITH_TIMEOUT(e.win.w.isReadOnly(), 30000);   // (a busy Windows runner took 5.5 s, Nov 17)
        QVERIFY(!e.win.w.isWindowModified());
        const QString second = e.folder.dir.filePath(QStringLiteral("second.xlsx"));
        QVERIFY(QFile::copy(e.path, second));
        Window again;
        QVERIFY(again.open(second));
        QVERIFY(again.w.isReadOnly());
        QVERIFY(again.w.markedAsFinal());
        again.w.setMarkedFinal(false);
        QVERIFY(!again.w.isReadOnly());
        QVERIFY(!again.w.markedAsFinal());
        // a copy of it is for editing
        Window copy;
        QVERIFY(copy.open(second, OpenHow::Copy));
        QVERIFY(!copy.w.isReadOnly());
    }

    void alwaysOpenReadOnlyIsKeptInTheFileAndOpensItForReadingOnly()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(!e.win.w.alwaysOpensReadOnly());
        e.win.w.setAlwaysOpenReadOnly(true);
        QVERIFY(e.win.w.alwaysOpensReadOnly());
        QVERIFY(!e.win.w.isReadOnly());   // (it applies the next time the file is opened)
        QVERIFY(saveAndWaitHere(e.win.w));
        const QString second = e.copyOfFile();
        QVERIFY(!second.isEmpty());
        Window again;
        QVERIFY(again.open(second));
        QVERIFY(again.w.isReadOnly());
        QVERIFY(again.w.alwaysOpensReadOnly());
        // the Info page's card turns it off and lets the window edit
        std::unique_ptr<QWidget> page(again.w.buildInfoPage(nullptr));
        auto *card = page->findChild<QAbstractButton *>(QStringLiteral("infoAlwaysReadOnly"));
        QVERIFY(card);
        QVERIFY(card->isEnabled());
        card->click();
        QVERIFY(!again.w.alwaysOpensReadOnly());
        QVERIFY(!again.w.isReadOnly());
        // opened as a copy it is a workbook to edit
        Window copy;
        QVERIFY(copy.open(second, OpenHow::Copy));
        QVERIFY(!copy.w.isReadOnly());
    }

    void turningAlwaysOpenReadOnlyOffKeepsAWindowThatIsReadOnlyForAnotherReason()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.setAlwaysOpenReadOnly(true);
        QVERIFY(saveAndWaitHere(e.win.w));
        const QString second = e.copyOfFile();
        Window b;
        QVERIFY(b.open(second, OpenHow::ReadOnly));
        QVERIFY(b.w.isReadOnly());
        b.w.setAlwaysOpenReadOnly(false);
        QVERIFY(!b.w.alwaysOpensReadOnly());
        QVERIFY(b.w.isReadOnly());
    }

    void alwaysOpenReadOnlyIsRefusedForTextFilesAnXlsAndAnOds()
    {
        EditWindow t;
        QVERIFY(t.open("a,b\n1,2\n", QStringLiteral("data.csv")));
        QVERIFY(!t.win.w.alwaysReadOnlyBlocked().isEmpty());
        t.win.w.setAlwaysOpenReadOnly(true);
        QVERIFY(!t.win.w.alwaysOpensReadOnly());
        QVERIFY(t.win.told.last().contains(QLatin1String("can't hold this choice")));
        for (const char *name : {"old.xls", "sheet.ods"}) {
            EditWindow o;
            QVERIFY(o.open(editBook(), QString::fromLatin1(name)));
            QVERIFY2(!o.win.w.alwaysReadOnlyBlocked().isEmpty(), name);
        }
    }

    // Check Compatibility for a .csv: the file holds the values of one sheet, so the other sheets, the formulas, and the formats are named, with the choices the file's properties keep.
    void checkCsvCompatibilityNamesWhatACsvFileCannotHold()
    {
        EditWindow e;
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f>A1*2</f><v>2</v></c></row>");
        QVERIFY(e.open(buildXlsx({worksheet(rows), worksheet(rows)})));
        QTRY_VERIFY(e.win.w.csvLosses().join(QLatin1Char('|')).contains(QLatin1String("formulas")));
        const QString before = e.win.w.csvLosses().join(QLatin1Char('|'));
        QVERIFY(before.contains(QLatin1String("1 other sheet")));
        QVERIFY(before.contains(QLatin1String("number formats")));
        QVERIFY(!before.contains(QLatin1String("always open")));
        e.win.w.setAlwaysOpenReadOnly(true);
        QVERIFY(e.win.w.csvLosses().join(QLatin1Char('|')).contains(QLatin1String("always open")));
        // Check Compatibility has the .csv kind in its one box, and it says the same in the shared sentence
        e.win.w.checkCompatibility(2);
        QVERIFY(!e.win.told.isEmpty());
        QVERIFY2(e.win.told.last().contains(QLatin1String("A .csv file can't hold some of what this workbook has")), qPrintable(e.win.told.last()));
        QVERIFY(e.win.told.last().contains(QLatin1String("1 other sheet")));
    }

    // The .xls and .ods kinds of the shared box name the read-only choice too (the .xls check once left it out), only when it is on.
    void checkCompatibilityNamesTheReadOnlyChoiceForXlsAndOds()
    {
        EditWindow e;
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>");
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        for (int kind = 0; kind < 2; ++kind) {
            e.win.told.clear();
            e.win.w.checkCompatibility(kind);
            QTRY_VERIFY_WITH_TIMEOUT(!e.win.told.isEmpty(), 20000);
            QVERIFY(!e.win.told.last().contains(QLatin1String("always open")));
        }
        e.win.w.setAlwaysOpenReadOnly(true);
        for (int kind = 0; kind < 2; ++kind) {
            e.win.told.clear();
            e.win.w.checkCompatibility(kind);
            QTRY_VERIFY_WITH_TIMEOUT(!e.win.told.isEmpty(), 20000);
            QVERIFY2(e.win.told.last().contains(QLatin1String("always open the workbook for reading only")), qPrintable(e.win.told.last()));
        }
        QCOMPARE(e.win.w.compatHost().formats.size(), 3);
    }

    // Pictures for a person to look at (JC_SHOT_DIR names the folder; nothing is made without it).
    void shotInfoPage()
    {
        const QString dir = qEnvironmentVariable("JC_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JC_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.setAlwaysOpenReadOnly(true);
        e.win.w.resize(1300, 800);
        e.win.w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&e.win.w));
        e.win.w.showFileScreen(QStringLiteral("info"));
        QTest::qWait(300);
        QDir().mkpath(dir);
        QVERIFY(e.win.w.grab().save(dir + QStringLiteral("/crunch-info.png")));
    }

    // A check that cannot be made says so, for each kind, and never "nothing is lost": no workbook, and the same window once a workbook is read.
    void checkCompatibilitySaysWhenItCouldNotCheck()
    {
        {
            Window blank;
            const jp::CompatHost host = blank.w.compatHost();
            QCOMPARE(host.formats.size(), 3);
            for (int kind = 0; kind < 3; ++kind) {
                jp::CompatibilityDialog box(host, kind, nullptr);
                QVERIFY2(box.reportText().startsWith(QLatin1String("Couldn't check")), qPrintable(box.reportText()));
                QVERIFY(!box.reportText().contains(QLatin1String("Nothing in this")));
            }
        }
        EditWindow e;
        e.path = e.folder.write(QStringLiteral("book.xlsx"), editBook());
        QVERIFY(e.win.w.openFile(e.path));
        QSignalSpy loaded(&e.win.w, &CrunchWindow::calculationFinished);
        QTRY_VERIFY_WITH_TIMEOUT(loaded.count() > 0, 120000);
        // once it is read, the same box checks
        e.win.told.clear();
        e.win.w.checkCompatibility(2);
        QVERIFY2(!e.win.told.isEmpty() && !e.win.told.last().contains(QLatin1String("Couldn't check")), qPrintable(e.win.told.join(QLatin1Char('|'))));
    }

    // The .xls list is bounded where the writer builds it: three hundred defined names the old form can't hold are a hundred lines and a count of two hundred more (one line each before),
    // and the box says "And 200 more."
    void theXlsListIsBoundedWhereItIsBuiltAndTheBoxSaysHowManyMore()
    {
        QString names;
        const QString longDefinition = QStringLiteral("1") + QStringLiteral("+1").repeated(6000);
        for (int i = 0; i < 300; ++i) names += QStringLiteral("<definedName name=\"Big%1\">%2</definedName>").arg(i).arg(longDefinition);
        const QByteArray bytes = bookOfSheets({{QStringLiteral("A1=1")}}, {}, {}, names);
        {
            office::Package package;
            office::sheet::Workbook book;
            QString why;
            QVERIFY2(package.readLazy(bytes, &why) && book.open(package, &why), qPrintable(why));
            QByteArray xls;
            office::sheet::CopyReport report;
            QVERIFY2(office::sheet::saveXls(book, &xls, &report, &why), qPrintable(why));
            QCOMPARE(report.trimmed.size(), jp::kMaxLossNotes);
            QCOMPARE(report.trimmedMore, qint64(200));
        }
        EditWindow e;
        QVERIFY(e.open(bytes));
        jp::CompatibilityDialog box(e.win.w.compatHost(), 0, nullptr);
        QTRY_VERIFY_WITH_TIMEOUT(!box.working(), 60000);
        QVERIFY2(box.reportText().contains(QLatin1String("And 200 more.")), qPrintable(box.reportText().right(120)));
        QCOMPARE(box.reportText().count(QLatin1String("Cut or changed to fit")), jp::kMaxLossNotes);
    }

    // The .ods list is bounded where the writer builds it too: a hundred and fifty sheets, each with a merged block too large to write, are one line a sheet up to the bound, then "And 50 more."
    void theOdsListIsBoundedWhereItIsBuiltAndTheBoxSaysHowManyMore()
    {
        QVector<QByteArray> sheetParts;
        for (int i = 0; i < 150; ++i)
            sheetParts.push_back(worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A1:Z50000\"/></mergeCells>")));
        const QByteArray bytes = buildXlsx(sheetParts);
        {
            office::Package package;
            office::sheet::Workbook book;
            QString why;
            QVERIFY2(package.readLazy(bytes, &why) && book.open(package, &why), qPrintable(why));
            QByteArray ods;
            office::sheet::CopyReport report;
            QVERIFY2(office::sheet::saveOds(book, &ods, &report, &why), qPrintable(why));
            QCOMPARE(report.trimmed.size(), jp::kMaxLossNotes);
            QCOMPARE(report.trimmedMore, qint64(50));
            for (const QString &line : std::as_const(report.trimmed)) QVERIFY2(line.contains(QLatin1String("merged ranges too large")), qPrintable(line));
        }
        EditWindow e;
        QVERIFY(e.open(bytes));
        jp::CompatibilityDialog box(e.win.w.compatHost(), 1, nullptr);
        QTRY_VERIFY_WITH_TIMEOUT(!box.working(), 120000);
        QVERIFY2(box.reportText().contains(QLatin1String("And 50 more.")), qPrintable(box.reportText().right(120)));
        QCOMPARE(box.reportText().count(QLatin1String("merged ranges too large")), jp::kMaxLossNotes);
    }

    // Only one check runs at a time: a new pick stops the one running and waits for it, and the window's end does the same (counted, not timed: each held check leaves only when it is stopped).
    void oneCheckRunsAtATimeAndTheWindowsEndStopsIt()
    {
        std::atomic<int> left{0};
        jp::compatWorkerHookForTest() = [&left](const std::atomic<bool> &stop) {
            for (int i = 0; i < 20000 && !stop.load(); ++i) QThread::msleep(1);   // (held until stopped; the count only keeps a broken run from waiting for ever)
            if (stop.load()) ++left;
        };
        auto reset = qScopeGuard([] { jp::compatWorkerHookForTest() = nullptr; });
        {
            EditWindow e;
            QVERIFY(e.open(editBook()));
            {
                jp::CompatibilityDialog box(e.win.w.compatHost(), 0, nullptr);   // (the first kind starts a check)
                QCOMPARE(e.win.w.compatChecksStarted(), qint64(1));
                QTRY_COMPARE_WITH_TIMEOUT(jp::compatAliveForTest().load(), 1, 20000);
                box.choose(1);   // (stops the first and waits for it)
                QCOMPARE(e.win.w.compatChecksStarted(), qint64(2));
                QCOMPARE(left.load(), 1);
                QTRY_COMPARE_WITH_TIMEOUT(jp::compatAliveForTest().load(), 1, 20000);
                box.choose(2);   // (the .csv kind is worked out here: nothing starts)
                QCOMPARE(e.win.w.compatChecksStarted(), qint64(2));
                box.choose(0);
                QCOMPARE(e.win.w.compatChecksStarted(), qint64(3));
                QCOMPARE(left.load(), 2);
                QTRY_COMPARE_WITH_TIMEOUT(jp::compatAliveForTest().load(), 1, 20000);
            }   // (the box goes while a check runs)
        }   // (and the window)
        QCOMPARE(left.load(), 3);
        QCOMPARE(jp::compatAliveForTest().load(), 0);
    }

    // The real box, every kind picked in it: each shows the kind it was asked for and, once it is worked out, a sentence that says what that kind holds.
    void everyKindCanBePickedInTheRealBox()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        const jp::CompatHost host = e.win.w.compatHost();
        jp::CompatibilityDialog box(host, 0, nullptr);
        QCOMPARE(box.formatCount(), 3);
        for (int kind = 0; kind < box.formatCount(); ++kind) {
            box.choose(kind);
            QTRY_VERIFY_WITH_TIMEOUT(!box.working(), 60000);
            QVERIFY2(box.reportText().contains(host.formats[kind].format), qPrintable(box.reportText()));
            QVERIFY(!box.reportText().startsWith(QLatin1String("Couldn't check")));
        }
    }

    // A workbook with a password is written to a .csv file in the clear: the list says so, and the save asks with the same words before writing.
    void aPasswordIsListedForCsvAndTheSaveSaysTheSame()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(!e.win.w.csvFound().join(QLatin1Char('|')).contains(QLatin1String("The password")));
        e.win.w.anchorHooks().setFilePasswordAsker = [](SetFilePasswordRequest *r) {
            r->password = QStringLiteral("one");
            r->remove = false;
            return true;
        };
        e.win.w.act(QStringLiteral("protect.password"))->trigger();
        const QString line = QStringLiteral("The password: the .csv file is written without it, so anyone can read it.");
        QVERIFY2(e.win.w.csvFound().contains(line), qPrintable(e.win.w.csvFound().join(QLatin1Char('|'))));
        jp::CompatibilityDialog box(e.win.w.compatHost(), 2, nullptr);
        QVERIFY(box.reportText().contains(line));
        e.win.told.clear();
        e.win.w.exportCsv(e.folder.dir.filePath(QStringLiteral("clear.csv")));
        QTRY_VERIFY_WITH_TIMEOUT(!e.win.told.isEmpty(), 20000);
        QVERIFY2(e.win.told.join(QLatin1Char('|')).contains(line), qPrintable(e.win.told.join(QLatin1Char('|'))));
    }

    // What the .xls, .ods, and .csv lists say and what a save keeps (the table is written to JO_TRUTH_DIR/crunch-truth.txt when that is set): a workbook with each thing people really have, saved through
    // the window's own save, opened again in a window, and saved as an .xlsx to look in. A thing that is gone and not listed is a failure, and so is one that is listed and kept.
    void whatEachKindListsIsWhatItsSaveLoses()
    {
        using Parts = QMap<QString, QByteArray>;
        struct Thing {
            QString name;
            std::function<bool(const Parts &)> in;
            QRegularExpression listedBy;
        };
        auto all = [](const Parts &p, const QString &prefix) {
            QString out;
            for (auto it = p.cbegin(); it != p.cend(); ++it)
                if (it.key().startsWith(prefix) && !it.key().endsWith(QLatin1String(".rels"))) out += QString::fromUtf8(it.value());
            return out;
        };
        auto sheets = [&](const Parts &p) { return all(p, QStringLiteral("xl/worksheets/")); };
        auto hasPart = [](const Parts &p, const QString &prefix) {
            for (auto it = p.cbegin(); it != p.cend(); ++it)
                if (it.key().startsWith(prefix)) return true;
            return false;
        };
        auto re = [](const char *t) { return QRegularExpression(QString::fromLatin1(t), QRegularExpression::CaseInsensitiveOption); };
        const QVector<Thing> things = {
            {QStringLiteral("formula"), [&](const Parts &p) { return sheets(p).contains(QLatin1String("B1*2")); }, re("(^|\\n)formulas \\(")},
            {QStringLiteral("second sheet"), [&](const Parts &p) { return all(p, QStringLiteral("xl/workbook.xml")).count(QLatin1String("<sheet ")) >= 2; }, re("other sheet")},
            {QStringLiteral("defined name"), [&](const Parts &p) { return all(p, QStringLiteral("xl/workbook.xml")).contains(QLatin1String("name=\"Total\"")); }, re("defined names")},
            {QStringLiteral("number format"), [&](const Parts &p) { return all(p, QStringLiteral("xl/styles.xml")).contains(QLatin1String("units")); }, re("number formats")},
            {QStringLiteral("fill"), [&](const Parts &p) { return all(p, QStringLiteral("xl/styles.xml")).contains(QLatin1String("FFFF00")); }, re("colors|fills")},
            {QStringLiteral("border"), [&](const Parts &p) { return all(p, QStringLiteral("xl/styles.xml")).contains(QLatin1String("style=\"thin\"")); }, re("borders")},
            {QStringLiteral("merged cells"), [&](const Parts &p) { return sheets(p).contains(QLatin1String("<mergeCell ")); }, re("merged")},
            {QStringLiteral("note"), [&](const Parts &p) { return all(p, QStringLiteral("xl/comments")).contains(QLatin1String("Note words")); }, re("notes")},
            {QStringLiteral("chart"), [&](const Parts &p) { return hasPart(p, QStringLiteral("xl/charts/chart")); }, re("chart")},
            {QStringLiteral("picture"), [&](const Parts &p) { return hasPart(p, QStringLiteral("xl/media/")); }, re("picture")},
            {QStringLiteral("table"), [&](const Parts &p) { return hasPart(p, QStringLiteral("xl/tables/table")); }, re("(?<!pivot )(?<!data )tables")},
            {QStringLiteral("filter"), [&](const Parts &p) { return sheets(p).contains(QLatin1String("<autoFilter")); }, re("filter")},
            {QStringLiteral("conditional formatting"), [&](const Parts &p) { return sheets(p).contains(QLatin1String("<conditionalFormatting")); }, re("conditional formatting")},
            {QStringLiteral("data validation"), [&](const Parts &p) { return sheets(p).contains(QLatin1String("<dataValidation ")); }, re("data validation")},
            {QStringLiteral("frozen panes"), [&](const Parts &p) { return sheets(p).contains(QLatin1String("state=\"frozen\"")); }, re("frozen|freeze")},
            {QStringLiteral("hyperlink"), [&](const Parts &p) { return sheets(p).contains(QLatin1String("<hyperlink ")); }, re("links")},
            {QStringLiteral("column width"), [&](const Parts &p) { return sheets(p).contains(QLatin1String("customWidth=\"1\"")); }, re("column widths")},
            {QStringLiteral("print settings"), [&](const Parts &p) { return sheets(p).contains(QLatin1String("orientation=\"landscape\"")); }, re("print")},
            {QStringLiteral("spilled formula"), [&](const Parts &p) { return sheets(p).contains(QLatin1String("cm=\"")); }, re("fill the cells beside|(^|\\n)formulas \\(")},
        };
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        // (saved through the window's own path: a save, a csv, or a copy says how it ended on its own signal)
        auto saveInto = [](Window &w, const QString &path, bool asCopy) {
            QSignalSpy saved(&w.w, &CrunchWindow::saved), csv(&w.w, &CrunchWindow::csvExported), copy(&w.w, &CrunchWindow::copyExported);
            if (!w.w.saveAs(path, asCopy)) return false;
            if (!QTest::qWaitFor([&] { return saved.count() + csv.count() + copy.count() > 0; }, 120000)) return false;
            for (QSignalSpy *spy : {&saved, &csv, &copy})
                if (spy->count() > 0) return spy->last().value(1).toBool();
            return false;
        };
        auto xlsxOf = [&](Window &w, const QString &name) {
            const QString path = dir.filePath(name);
            return saveInto(w, path, true) ? partsOf(path) : Parts();
        };
        const QString book = dir.filePath(QStringLiteral("every.xlsx"));
        {
            QFile f(book);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(everythingBook());
        }
        Window a;
        QVERIFY(a.open(book));
        const Parts inFixture = xlsxOf(a, QStringLiteral("fixture-copy.xlsx"));
        QVERIFY(!inFixture.isEmpty());
        QStringList log, missing, hidden, over;
        for (const Thing &t : things)
            if (!t.in(inFixture)) missing << t.name;
        log << QStringLiteral("not in the workbook as the window saves it: ") + missing.join(QStringLiteral(", "));
        const QVector<jp::CompatFormat> formats = a.w.compatHost().formats;
        QCOMPARE(formats.size(), 3);
        for (int k = 0; k < formats.size(); ++k) {
            const QString ending = formats[k].format;
            QStringList lines;
            bool got = false;
            formats[k].lost([&](const jp::CompatAnswer &answer) {
                lines = answer.found;
                got = true;
            });
            QTRY_VERIFY_WITH_TIMEOUT(got, 120000);
            const QString listed = lines.join(QLatin1Char('\n'));
            const QString path = dir.filePath(QStringLiteral("saved") + ending);
            QVERIFY2(saveInto(a, path, false), qPrintable(ending));
            Window b;
            QVERIFY2(b.open(path), qPrintable(ending));
            const Parts parts = xlsxOf(b, QStringLiteral("again%1.xlsx").arg(k));
            QVERIFY2(!parts.isEmpty(), qPrintable(ending));
            log << QStringLiteral("== %1: the list says:\n   %2").arg(ending, lines.join(QStringLiteral("\n   ")));
            for (const Thing &t : things) {
                if (missing.contains(t.name)) continue;
                const bool kept = t.in(parts), isListed = t.listedBy.match(listed).hasMatch();
                log << QStringLiteral("%1 | %2 | listed=%3 | kept=%4").arg(ending, t.name, isListed ? QStringLiteral("yes") : QStringLiteral("no"), kept ? QStringLiteral("yes") : QStringLiteral("no"));
                if (!kept && !isListed) hidden << ending + QLatin1Char(' ') + t.name;
                if (kept && isListed) over << ending + QLatin1Char(' ') + t.name;
            }
        }
        // the password: .xls and .ods keep it (the saved file asks for it); .csv is written in the clear and the list says so
        {
            Window p;
            const QString second = dir.filePath(QStringLiteral("protected.xlsx"));   // (a file that is open in a window is not opened in a second)
            QVERIFY(QFile::copy(book, second));
            QVERIFY(p.open(second));
            p.w.anchorHooks().setFilePasswordAsker = [](SetFilePasswordRequest *r) {
                r->password = QStringLiteral("one");
                r->remove = false;
                return true;
            };
            p.w.act(QStringLiteral("protect.password"))->trigger();
            const QVector<jp::CompatFormat> pformats = p.w.compatHost().formats;
            for (int k = 0; k < pformats.size(); ++k) {
                const QString ending = pformats[k].format;
                QStringList lines;
                bool got = false;
                pformats[k].lost([&](const jp::CompatAnswer &answer) {
                    lines = answer.found;
                    got = true;
                });
                QTRY_VERIFY_WITH_TIMEOUT(got, 120000);
                const bool isListed = lines.join(QLatin1Char('\n')).contains(QLatin1String("password"), Qt::CaseInsensitive);
                const QString path = dir.filePath(QStringLiteral("locked") + ending);
                QVERIFY2(saveInto(p, path, false), qPrintable(ending));
                bool kept = false;
                if (k == 2) {
                    QFile f(path);
                    QVERIFY(f.open(QIODevice::ReadOnly));
                    kept = !f.readAll().contains("Item");   // (a protected file does not hold its words as they are)
                } else {
                    Window b;
                    bool asked = false;
                    b.w.anchorHooks().filePasswordAsker = [&asked](FilePasswordRequest *r) {
                        asked = true;
                        r->password = QStringLiteral("one");
                        return true;
                    };
                    QVERIFY2(b.open(path), qPrintable(ending));
                    kept = asked;
                }
                log << QStringLiteral("%1 | password | listed=%2 | kept=%3").arg(ending, isListed ? QStringLiteral("yes") : QStringLiteral("no"), kept ? QStringLiteral("yes") : QStringLiteral("no"));
                if (!kept && !isListed) hidden << ending + QStringLiteral(" password");
                if (kept && isListed) over << ending + QStringLiteral(" password");
            }
        }
        const QString out = qEnvironmentVariable("JO_TRUTH_DIR");
        if (!out.isEmpty()) {
            QFile f(out + QStringLiteral("/crunch-truth.txt"));
            if (f.open(QIODevice::WriteOnly)) f.write(log.join(QLatin1Char('\n')).toUtf8());
        }
        QVERIFY2(hidden.isEmpty(), qPrintable(QStringLiteral("lost and not listed: ") + hidden.join(QStringLiteral("; "))));
        QVERIFY2(over.isEmpty(), qPrintable(QStringLiteral("listed but kept: ") + over.join(QStringLiteral("; "))));
    }

    // A picture of Check Compatibility (JO_SHOT_DIR names the folder; nothing is made without it).
    void shotCheckCompatibility()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.setAlwaysOpenReadOnly(true);
        jp::CompatibilityDialog dialog(e.win.w.compatHost(), 2, nullptr);
        dialog.resize(520, 300);
        dialog.show();
        QVERIFY(QTest::qWaitForWindowExposed(&dialog));
        QTest::qWait(200);
        QDir().mkpath(dir);
        QVERIFY(dialog.grab().save(dir + QStringLiteral("/crunch-compat.png")));
        // the other states: a check that could not be made, and a list that was cut ("And N more.")
        auto states = [&](const QString &name, const std::function<void(const jp::CompatDone &)> &lost) {
            jp::CompatHost host = e.win.w.compatHost();
            host.formats[0].lost = lost;
            jp::CompatibilityDialog box(host, 0, nullptr);
            box.resize(520, 300);
            box.show();
            QVERIFY(QTest::qWaitForWindowExposed(&box));
            if (auto *scroll = box.findChild<QScrollArea *>()) scroll->verticalScrollBar()->setValue(scroll->verticalScrollBar()->maximum());   // (the end of a long list, where it says how many more)
            QVERIFY(box.grab().save(dir + name));
        };
        states(QStringLiteral("/crunch-compat-could-not.png"), [](const jp::CompatDone &done) { done(jp::CompatAnswer::failed(QStringLiteral("The workbook is still being read. Check it when the status bar says Ready."))); });
        states(QStringLiteral("/crunch-compat-more.png"), [](const jp::CompatDone &done) {
            QStringList many;
            for (int i = 1; i <= 230; ++i) many << QStringLiteral("Part %1 of the file").arg(i);
            done(jp::CompatAnswer(many));
        });

    }

    void inspectFindsHiddenSheetsAndPersonalInformation()
    {
        EditWindow e;
        QVERIFY(e.open(budgetWorkbook()));
        office::PackageProps p = e.win.w.properties();
        p.creator = QStringLiteral("Ann");
        p.manager = QStringLiteral("Bo");
        e.win.w.applyProperties(p);
        const QVector<InspectItem> items = e.win.w.inspectItems();
        auto find = [&](const char *title) -> const InspectItem * {
            for (const InspectItem &i : items)
                if (i.title == QLatin1String(title)) return &i;
            return nullptr;
        };
        QVERIFY(find("Hidden sheets"));
        QCOMPARE(find("Hidden sheets")->count(), 1);
        QCOMPARE(find("Document properties and personal information")->count(), 3);   // (the person who saved is named by the edit)
        QCOMPARE(find("Document properties and personal information")->remove(), 3);
        QCOMPARE(find("Document properties and personal information")->count(), 0);
        QCOMPARE(find("Hidden sheets")->remove(), 1);
        QCOMPARE(find("Hidden sheets")->count(), 0);
        QVERIFY(find("External links")->removable);   // (taken out with Remove All; tried in tst_crunch_keep)
        QVERIFY(find("Custom XML data")->removable);
    }

    void inspectFindsAndShowsHiddenRowsAndColumns()
    {
        EditWindow e;
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\" hidden=\"1\"><c r=\"A2\"><v>2</v></c></row>"
                                            "<row r=\"3\" hidden=\"1\"><c r=\"A3\"><v>3</v></c></row><row r=\"5\" hidden=\"1\"><c r=\"A5\"><v>5</v></c></row>");
        const QString cols = QStringLiteral("<cols><col min=\"2\" max=\"3\" width=\"9\" hidden=\"1\" customWidth=\"1\"/></cols>");
        QVERIFY(e.open(buildXlsx({worksheet(rows, QString(), cols)})));
        const QVector<InspectItem> items = e.win.w.inspectItems();
        const InspectItem *item = nullptr;
        for (const InspectItem &i : items)
            if (i.title == QLatin1String("Hidden rows and columns")) item = &i;
        QVERIFY(item);
        QCOMPARE(item->count(), 5);   // (three rows and two columns)
        QVERIFY(item->removable);
        QCOMPARE(item->remove(), 5);
        QCOMPARE(item->count(), 0);
    }

    // Rows a filter hides stay hidden and are not in the count of hidden rows, and the row says how many there are, so "None found" is never read as "no hidden rows".
    void inspectSaysHowManyRowsAFilterHides()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>7</v></c></row><row r=\"2\"><c r=\"A2\"><v>200</v></c></row><row r=\"3\"><c r=\"A3\"><v>2</v></c></row>"
                                            "<row r=\"4\"><c r=\"A4\"><v>3</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        QVERIFY(e.settle());
        jp::office::sheet::AutoFilter filter;
        filter.ref = jp::office::sheet::CellRange{0, 0, 3, 0, true};
        jp::office::sheet::FilterColumn condition;
        condition.kind = jp::office::sheet::FilterColumn::Kind::Custom;
        condition.column = 0;
        condition.conditions.append({jp::office::sheet::FilterColumn::Op::Greater, QStringLiteral("100")});
        filter.columns.append(condition);
        QVERIFY(e.win.w.session()->setFilter(0, filter, QStringLiteral("Filtering")).ok);
        QVERIFY(e.settle());
        const InspectItem item = inspectItemOf(e, "Hidden rows and columns");
        QCOMPARE(item.count(), 0);   // (the filter's rows are not counted)
        QVERIFY2(item.unread().startsWith(QLatin1String("2 row")), qPrintable(item.unread()));
        InspectDialog box(QStringLiteral("Inspect"), {item}, nullptr);
        QVERIFY(box.summaries().first().contains(QLatin1String("Not fully checked")));
    }

    // Show All on the names a file hides is one step of Undo for all of them, however many (5,000 here), and each is shown after it and hidden again by that one step.
    void inspectShowsThousandsOfHiddenNamesInOneStep()
    {
        QString names;
        const int total = 5000;
        for (int i = 0; i < total; ++i) names += QStringLiteral("<definedName name=\"Hid_name_%1\" hidden=\"1\">Sheet1!$A$1</definedName>").arg(i);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"))}, QByteArray(), QString(), QString(), QByteArray(), QStringList({"Sheet1"}), names), QStringLiteral("many.xlsx")));
        QVERIFY(e.settle());
        QCOMPARE(inspectItemOf(e, "Hidden names").count(), total);
        const int steps = e.win.w.undoStack()->count();
        QCOMPARE(inspectItemOf(e, "Hidden names").remove(), total);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(inspectItemOf(e, "Hidden names").count(), 0);
        e.win.w.undoStack()->undo();
        QCOMPARE(inspectItemOf(e, "Hidden names").count(), total);
        e.win.w.undoStack()->redo();
        QCOMPARE(inspectItemOf(e, "Hidden names").count(), 0);
    }

    // Inspect finds the objects a file hides (a chart, a picture, a shape, and a text box), and Remove All takes them out of the file whole, with the parts only they used, in every form the
    // workbook is saved in; the object that shows, and the rest of the file, stay as they were.
    void inspectTakesHiddenObjectsOutOfEveryFormat()
    {
        EditWindow e;
        QVERIFY(e.open(hiddenObjectsBook(), QStringLiteral("hidden.xlsx")));
        QVERIFY(e.settle());
        const QStringList secrets = {QStringLiteral("HiddenSecretChart"), QStringLiteral("HiddenSecretPicture"), QStringLiteral("HiddenSecretShape"), QStringLiteral("HiddenSecretTextBox"), QStringLiteral("HiddenSecretWords"), QStringLiteral("HiddenSharedPicture"), QStringLiteral("HiddenSecretEmbeddedBook")};
        const auto itemOf = [&]() -> InspectItem {
            for (const InspectItem &i : e.win.w.inspectItems())
                if (i.title == QLatin1String("Hidden objects")) return i;
            return InspectItem();
        };
        const InspectItem item = itemOf();
        QVERIFY(!item.title.isEmpty());
        QCOMPARE(item.count(), 5);
        QVERIFY(item.removable);
        const QMap<QString, QByteArray> before = partsOf(e.path);
        QVERIFY(before.value(QStringLiteral("xl/drawings/drawing1.xml")).contains("HiddenSecretPicture"));
        QCOMPARE(item.remove(), 5);
        QCOMPARE(itemOf().count(), 0);
        QCOMPARE(itemOf().remove(), 0);
        QVERIFY(saveAndWaitHere(e.win.w));
        const QMap<QString, QByteArray> now = partsOf(e.path);
        for (const QString &s : secrets) QVERIFY2(!fileHolds(e.path, s), qPrintable(s));
        QVERIFY(now.value(QStringLiteral("xl/drawings/drawing1.xml")).contains("ShownPicture"));
        QVERIFY(now.contains(QStringLiteral("xl/media/image1.png")));
        QVERIFY(!now.contains(QStringLiteral("xl/media/image2.png")));
        QVERIFY(!now.contains(QStringLiteral("xl/charts/chart1.xml")));
        QVERIFY(!now.contains(QStringLiteral("xl/charts/_rels/chart1.xml.rels")));
        QVERIFY(!now.contains(QStringLiteral("xl/embeddings/chartbook1.xlsx")));
        QVERIFY(now.value(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels")).contains("rIdI1"));
        QVERIFY(!now.value(QStringLiteral("xl/drawings/_rels/drawing1.xml.rels")).contains("rIdI2"));
        QCOMPARE(now.value(QStringLiteral("xl/media/image1.png")), before.value(QStringLiteral("xl/media/image1.png")));
        QCOMPARE(now.value(QStringLiteral("xl/worksheets/sheet1.xml")), before.value(QStringLiteral("xl/worksheets/sheet1.xml")));
        QVERIFY(!now.value(QStringLiteral("[Content_Types].xml")).contains("chart1.xml"));
        // the copies in the other forms
        for (const char *name : {"copy.ods", "copy.ots", "copy.xls"}) {
            const QString target = e.folder.dir.filePath(QString::fromLatin1(name));
            QSignalSpy copied(&e.win.w, &CrunchWindow::copyExported);
            QVERIFY(e.win.w.saveAs(target, true));
            QVERIFY(copied.count() > 0 || copied.wait(20000));
            for (const QString &s : secrets) QVERIFY2(!fileHolds(target, s), qPrintable(QString::fromLatin1(name) + QLatin1Char(' ') + s));
        }
        // and the file read again has the shown picture, and the item finds nothing
        EditWindow again;
        QVERIFY(again.open(QFile(e.path).exists() ? [&] { QFile f(e.path); f.open(QIODevice::ReadOnly); return f.readAll(); }() : QByteArray(), QStringLiteral("again.xlsx")));
        QVERIFY(again.settle());
        for (const InspectItem &i : again.win.w.inspectItems())
            if (i.title == QLatin1String("Hidden objects")) QCOMPARE(i.count(), 0);
    }

    // Inspect reads the drawing's bytes, so no kind of object is skipped: a hidden shape inside a group that shows, one inside an alternate-content wrapper (in the anchor, and around the anchor),
    // a hidden table frame, and a hidden chart whose part is missing are all counted and taken out, and what shows (the group, its other shape, the shape beside) stays. A plain save first
    // does not make the count forget them.
    void inspectReachesTheObjectsTheReaderSkips()
    {
        const QString secrets[] = {QStringLiteral("NestedHiddenShape"), QStringLiteral("WrappedHiddenShape"), QStringLiteral("HiddenTableFrame"), QStringLiteral("HiddenOrphanChart"), QStringLiteral("TopWrappedHidden")};
        const QString table = QStringLiteral("<xdr:graphicFrame macro=\"\"><xdr:nvGraphicFramePr><xdr:cNvPr id=\"14\" name=\"HiddenTableFrame\"/><xdr:cNvGraphicFramePr/></xdr:nvGraphicFramePr>"
                                             "<xdr:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/></xdr:xfrm><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/table\"/></a:graphic></xdr:graphicFrame>");
        const QString wrapped = QStringLiteral("<mc:AlternateContent xmlns:mc=\"%1\"><mc:Choice Requires=\"a14\" xmlns:a14=\"http://schemas.microsoft.com/office/drawing/2010/main\">%2</mc:Choice><mc:Fallback/></mc:AlternateContent>");
        const QString anchors = drawtest::twoCell(1, 1, 4, 3, drawtest::groupElement(10, hideElement(drawtest::shapeElement(11, QStringLiteral("NestedHiddenShape"))) + drawtest::shapeElement(12, QStringLiteral("NestedShownShape"))))
            + drawtest::twoCell(1, 5, 3, 7, wrapped.arg(kMc, hideElement(drawtest::shapeElement(13, QStringLiteral("WrappedHiddenShape")))))
            + drawtest::twoCell(5, 1, 8, 3, hideElement(table))
            + drawtest::twoCell(5, 5, 8, 7, hideElement(drawtest::chartFrame(15, QStringLiteral("rIdNone"), QStringLiteral("HiddenOrphanChart"))))
            + wrapped.arg(kMc, drawtest::twoCell(9, 1, 12, 3, hideElement(drawtest::shapeElement(16, QStringLiteral("TopWrappedHidden")))))
            + drawtest::twoCell(9, 5, 12, 7, drawtest::shapeElement(17, QStringLiteral("ShownShape")));
        EditWindow e;
        QVERIFY(e.open(drawingBookOf(anchors), QStringLiteral("kinds.xlsx")));
        QVERIFY(e.settle());
        QCOMPARE(inspectItemOf(e, "Hidden objects").count(), 5);
        QVERIFY(inspectItemOf(e, "Hidden objects").unread().isEmpty());
        QVERIFY(saveAndWaitHere(e.win.w));   // (a save that takes nothing out: the file still has them, and the count says so)
        QCOMPARE(inspectItemOf(e, "Hidden objects").count(), 5);
        QCOMPARE(inspectItemOf(e, "Hidden objects").remove(), 5);
        QCOMPARE(inspectItemOf(e, "Hidden objects").count(), 0);
        QVERIFY(saveAndWaitHere(e.win.w));
        for (const QString &secret : secrets) QVERIFY2(!fileHolds(e.path, secret), qPrintable(secret));
        const QByteArray drawing = partsOf(e.path).value(QStringLiteral("xl/drawings/drawing1.xml"));
        for (const char *kept : {"NestedShownShape", "ShownShape", "name=\"Group\""}) QVERIFY2(drawing.contains(kept), kept);
        for (const char *name : {"copy.ods", "copy.xls"}) {
            const QString target = e.folder.dir.filePath(QString::fromLatin1(name));
            QSignalSpy copied(&e.win.w, &CrunchWindow::copyExported);
            QVERIFY(e.win.w.saveAs(target, true));
            QVERIFY(copied.count() > 0 || copied.wait(20000));
            for (const QString &secret : secrets) QVERIFY2(!fileHolds(target, secret), qPrintable(QString::fromLatin1(name) + QLatin1Char(' ') + secret));
        }
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray reopened = f.readAll();
        f.close();
        EditWindow again;
        QVERIFY(again.open(reopened, QStringLiteral("again.xlsx")));
        QVERIFY(again.settle());
        QCOMPARE(inspectItemOf(again, "Hidden objects").count(), 0);
    }

    // A drawing of the old kind (pictures, text boxes, and controls in a vmlDrawing part) is not read, so the row says how many shapes it could not judge, and does not say "None found"; a part
    // that holds only the boxes of notes has nothing it could not judge.
    void inspectSaysItCannotJudgeOldStyleDrawings()
    {
        const QByteArray notesOnly = "<xml><v:shape id=\"s1\"><x:ClientData ObjectType=\"Note\"><x:Row>3</x:Row></x:ClientData></v:shape></xml>";
        const QByteArray withOthers = "<xml><v:shape id=\"s1\"><x:ClientData ObjectType=\"Note\"><x:Row>3</x:Row></x:ClientData></v:shape>"
                                      "<v:shape id=\"s2\"><v:imagedata/></v:shape><v:shape id=\"s3\"><v:textbox/><x:ClientData ObjectType=\"Button\"/></v:shape><v:shapetype id=\"t\"/></xml>";
        const QString kinds[] = {QStringLiteral("notes"), QStringLiteral("others")};
        for (int k = 0; k < 2; ++k) {
            QVector<ExtraPart> parts;
            parts.append({QStringLiteral("xl/drawings/vmlDrawing1.vml"), k == 0 ? notesOnly : withOthers, QString(), 0, QStringLiteral("rIdV"), QStringLiteral("vmlDrawing"), QStringLiteral("../drawings/vmlDrawing1.vml")});
            EditWindow e;
            QVERIFY(e.open(withParts(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"))}), parts), QStringLiteral("old.xlsx")));
            QVERIFY(e.settle());
            const InspectItem item = inspectItemOf(e, "Hidden objects");
            QCOMPARE(item.count(), 0);
            QVERIFY2(k == 0 ? item.unread().isEmpty() : item.unread().startsWith(QLatin1String("2 shape")), qPrintable(kinds[k] + QLatin1Char(' ') + item.unread()));
            InspectDialog box(QStringLiteral("Inspect"), {item}, nullptr);
            QVERIFY2(box.summaries().first().contains(QLatin1String(k == 0 ? "None found" : "Not fully checked")), qPrintable(box.summaries().first()));
        }
    }

    // A drawing past the limit of what is looked through is counted as far as it was, and the row says the rest was not: Remove All does not claim they are gone, and the shape past the
    // limit is still in the file.
    void inspectSaysWhenADrawingWasNotLookedThroughToTheEnd()
    {
        struct Reset {
            ~Reset() { office::sheet::setHiddenScanLimit(0); }
        } reset;
        QString anchors = drawtest::twoCell(1, 1, 3, 3, hideElement(drawtest::shapeElement(2, QStringLiteral("EarlyHiddenShape"))));
        for (int i = 0; i < 40; ++i) anchors += drawtest::twoCell(4, 1 + i, 6, 2 + i, drawtest::shapeElement(10 + i, QStringLiteral("Plain%1").arg(i)));
        anchors += drawtest::twoCell(1, 50, 3, 52, hideElement(drawtest::shapeElement(99, QStringLiteral("LateHiddenShape"))));
        const QByteArray book = drawingBookOf(anchors);
        // (the limit counts the tags looked at: 40 is about the first anchor and a half)
        office::sheet::setHiddenScanLimit(40);
        EditWindow e;
        QVERIFY(e.open(book, QStringLiteral("long.xlsx")));
        QVERIFY(e.settle());
        const InspectItem item = inspectItemOf(e, "Hidden objects");
        QCOMPARE(item.count(), 1);
        QVERIFY2(item.unread().startsWith(QLatin1String("1 drawing")), qPrintable(item.unread()));
        QCOMPARE(item.remove(), 1);
        const InspectItem after = inspectItemOf(e, "Hidden objects");
        QCOMPARE(after.count(), 0);
        QVERIFY(!after.unread().isEmpty());
        InspectDialog box(QStringLiteral("Inspect"), {after}, nullptr);
        QVERIFY2(box.summaries().first().contains(QLatin1String("Not fully checked")) && !box.summaries().first().contains(QLatin1String("None found")), qPrintable(box.summaries().first()));
        QVERIFY(saveAndWaitHere(e.win.w));
        QVERIFY(!fileHolds(e.path, QStringLiteral("EarlyHiddenShape")));
        QVERIFY(fileHolds(e.path, QStringLiteral("LateHiddenShape")));
        // (looked through to the end, the late one is found too)
        office::sheet::setHiddenScanLimit(0);
        EditWindow whole;
        QVERIFY(whole.open(book, QStringLiteral("whole.xlsx")));
        QVERIFY(whole.settle());
        QCOMPARE(inspectItemOf(whole, "Hidden objects").count(), 2);
        QVERIFY(inspectItemOf(whole, "Hidden objects").unread().isEmpty());
    }

    // A drawing large enough to be read on a worker (more than SheetObjects::kParseApartBytes) is counted in full when Inspect is asked at once after opening, before that read is done: the count
    // is made on the asking thread from the bytes (one inflate and one scan of the part), so it never waits and is never short.
    void inspectCountsALargeDrawingAtOnceAfterOpening()
    {
        QString anchors;
        const int total = 900;
        for (int i = 0; i < total; ++i) {
            QString shape = drawtest::shapeElement(2 + i, QStringLiteral("Shape%1").arg(i), QStringLiteral("rect"), QStringLiteral("Some words of text to make the part larger %1").arg(i));
            if (i == 5 || i == total - 1) shape = hideElement(shape);
            anchors += drawtest::twoCell(1, i % 50, 3, i % 50 + 1, shape);
        }
        const QByteArray book = drawingBookOf(anchors);
        QVERIFY(drawtest::drawingPart(anchors).size() > crunch::SheetObjects::kParseApartBytes);
        EditWindow e;
        QVERIFY(e.open(book, QStringLiteral("large.xlsx")));
        // (no settle: nothing waited for the worker)
        QCOMPARE(inspectItemOf(e, "Hidden objects").count(), 2);
        QVERIFY(inspectItemOf(e, "Hidden objects").unread().isEmpty());
        QCOMPARE(inspectItemOf(e, "Hidden objects").remove(), 2);
        QVERIFY(e.settle());
        QCOMPARE(inspectItemOf(e, "Hidden objects").count(), 0);
    }

    // Inspect finds the names a file hides, and Show All makes them show (the names stay: a formula may read one); the step can be undone, and what is saved, in every form, has none hidden.
    void inspectShowsHiddenNames()
    {
        const QByteArray book = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f>Quiet*2</f><v>2</v></c></row>"))}, QByteArray(), QString(), QString(), QByteArray(),
                                          QStringList({"Sheet1"}), QStringLiteral("<definedName name=\"Quiet\" hidden=\"1\">Sheet1!$A$1</definedName><definedName name=\"Seen\">Sheet1!$A$1:$A$2</definedName>"));
        EditWindow e;
        QVERIFY(e.open(book, QStringLiteral("names.xlsx")));
        QVERIFY(e.settle());
        const auto itemOf = [&](EditWindow &w) -> InspectItem {
            for (const InspectItem &i : w.win.w.inspectItems())
                if (i.title == QLatin1String("Hidden names")) return i;
            return InspectItem();
        };
        InspectItem item = itemOf(e);
        QVERIFY(!item.title.isEmpty());
        QCOMPARE(item.count(), 1);
        QCOMPARE(item.action, QStringLiteral("Show All"));
        QCOMPARE(item.remove(), 1);
        QCOMPARE(itemOf(e).count(), 0);
        e.win.w.undoStack()->undo();
        QCOMPARE(itemOf(e).count(), 1);
        e.win.w.undoStack()->redo();
        QCOMPARE(itemOf(e).count(), 0);
        QVERIFY(saveAndWaitHere(e.win.w));
        const QMap<QString, QByteArray> now = partsOf(e.path);
        const QByteArray workbook = now.value(QStringLiteral("xl/workbook.xml"));
        QVERIFY2(workbook.contains("name=\"Quiet\"") && workbook.contains("name=\"Seen\"") && !workbook.contains("hidden=\"1\""), workbook.constData());
        QVERIFY(now.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("Quiet*2"));
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray reopened = f.readAll();
        f.close();
        EditWindow again;
        QVERIFY(again.open(reopened, QStringLiteral("again.xlsx")));
        QVERIFY(again.settle());
        QCOMPARE(itemOf(again).count(), 0);
        QVERIFY(again.win.w.session()->names().find(-1, QStringLiteral("Quiet")) >= 0);
        for (const char *name : {"copy.ods", "copy.xls"}) {
            const QString target = e.folder.dir.filePath(QString::fromLatin1(name));
            QSignalSpy copied(&e.win.w, &CrunchWindow::copyExported);
            QVERIFY(e.win.w.saveAs(target, true));
            QVERIFY(copied.count() > 0 || copied.wait(20000));
            QFile c(target);
            QVERIFY(c.open(QIODevice::ReadOnly));
            EditWindow other;
            QVERIFY(other.open(c.readAll(), QString::fromLatin1(name)));
            QVERIFY(other.settle());
            QCOMPARE(itemOf(other).count(), 0);
            QVERIFY2(other.win.w.session()->names().find(-1, QStringLiteral("Quiet")) >= 0, name);
        }
    }

    // Pictures of the Inspect box (JO_SHOT_DIR): what it listed before the objects and names the file hides were added, and now.
    void shotInspect()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        EditWindow e;
        QVERIFY(e.open(hiddenObjectsBook(), QStringLiteral("hidden.xlsx")));
        QVERIFY(e.settle());
        QDir().mkpath(dir);
        const QVector<InspectItem> now = e.win.w.inspectItems();
        QVector<InspectItem> before;
        for (const InspectItem &i : now)
            if (i.title != QLatin1String("Hidden objects") && i.title != QLatin1String("Hidden names")) before.push_back(i);
        int n = 0;
        for (const QVector<InspectItem> &items : {before, now}) {
            InspectDialog box(QStringLiteral("Inspect Workbook"), items, nullptr);
            box.show();
            QVERIFY(QTest::qWaitForWindowExposed(&box));
            QVERIFY(box.grab().save(dir + (n++ == 0 ? QStringLiteral("/crunch-inspect-before.png") : QStringLiteral("/crunch-inspect-after.png"))));
        }
    }

    void inspectFindsAndTakesOutHeadersAndFooters()
    {
        EditWindow e;
        const QString after = QStringLiteral("<headerFooter><oddHeader>&amp;CQuarterly</oddHeader><oddFooter>&amp;LPage &amp;P</oddFooter></headerFooter>");
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), after)})));
        const QVector<InspectItem> items = e.win.w.inspectItems();
        const InspectItem *item = nullptr;
        for (const InspectItem &i : items)
            if (i.title == QLatin1String("Headers and footers")) item = &i;
        QVERIFY(item);
        QCOMPARE(item->count(), 1);
        QCOMPARE(item->remove(), 1);
        QCOMPARE(item->count(), 0);
    }

    // The Inspect box never says "None found" for a kind it could not fully look at: the row says so, in words, and a count of nothing is "Not fully checked", not "None found".
    void inspectBoxSaysWhenARowCouldNotCheck()
    {
        int unread = 4;
        InspectItem a;
        a.title = QStringLiteral("Looked");
        a.count = [] { return 0; };
        InspectItem b;
        b.title = QStringLiteral("Partly");
        b.count = [] { return 0; };
        b.remove = [] { return 0; };
        b.unread = [&unread] { return unread > 0 ? QStringLiteral("%1 could not be looked at.").arg(unread) : QString(); };
        InspectItem c;
        c.title = QStringLiteral("Some found");
        c.count = [] { return 2; };
        c.unread = [] { return QStringLiteral("One more could not be looked at."); };
        InspectDialog box(QStringLiteral("Inspect"), {a, b, c}, nullptr);
        const QStringList rows = box.summaries();
        QCOMPARE(rows.value(0), QStringLiteral("Looked: None found"));
        QVERIFY2(rows.value(1).contains(QLatin1String("Not fully checked")) && !rows.value(1).contains(QLatin1String("None found")) && rows.value(1).contains(QLatin1String("4 could not be looked at.")), qPrintable(rows.value(1)));
        QVERIFY2(rows.value(2).contains(QLatin1String("2 found")) && rows.value(2).contains(QLatin1String("One more")), qPrintable(rows.value(2)));
        unread = 0;
        box.removeRow(1);
        QCOMPARE(box.summaries().value(1), QStringLiteral("Partly: None found"));
    }

    // Every built-in template opens, works its formulas out with no error, and says what it should.
    void everyBuiltInTemplateOpensAndWorksItsFormulasOut()
    {
        const QVector<crunch::TemplateInfo> all = crunch::builtinTemplates();
        QVERIFY(all.size() >= 12);
        QStringList ids;
        for (const crunch::TemplateInfo &t : all) {
            QVERIFY(!ids.contains(t.id));
            ids << t.id;
            QVERIFY(!t.name.isEmpty() && !t.category.isEmpty() && !t.description.isEmpty());
            const QByteArray bytes = crunch::builtinTemplateBytes(t.id);
            QVERIFY2(!bytes.isEmpty(), qPrintable(t.id));
            const QImage picture = crunch::builtinTemplatePreview(t.id, 200);
            QVERIFY2(picture.width() == 200 && picture.height() > 50, qPrintable(t.id));
            bool drawn = false;
            for (int y = 0; y < picture.height() && !drawn; ++y)
                for (int x = 0; x < picture.width() && !drawn; ++x) drawn = picture.pixelColor(x, y) != QColor(Qt::white);
            QVERIFY2(drawn, qPrintable(t.id));
            EditWindow e;
            QVERIFY2(e.open(bytes), qPrintable(t.id));
            QVERIFY2(e.settle(), qPrintable(t.id));
            for (int row = 1; row <= 80; ++row)
                for (int col = 0; col < 16; ++col) {
                    const QString ref = QString(QChar(u'A' + col)) + QString::number(row);
                    QVERIFY2(!e.shownAt(ref).startsWith(QLatin1Char('#')), qPrintable(t.id + QLatin1Char(' ') + ref + QLatin1Char(' ') + e.shownAt(ref)));
                }
            if (t.id == QLatin1String("loan")) {
                QCOMPARE(e.shownAt(QStringLiteral("B6")), QStringLiteral("$386") + QLocale().decimalPoint() + QStringLiteral("66"));
                QVERIFY(e.shownAt(QStringLiteral("E70")).endsWith(QStringLiteral("0") + QLocale().decimalPoint() + QStringLiteral("00")));   // (sixty payments: the balance is paid off at the last)
                QVERIFY(e.shownAt(QStringLiteral("A71")).isEmpty());
            }
            if (t.id == QLatin1String("calendar")) {
                QCOMPARE(e.shownAt(QStringLiteral("A4")), QStringLiteral("January 2027"));   // (month names follow the interface language, numbers the computer's)
                QCOMPARE(e.shownAt(QStringLiteral("F6")), QStringLiteral("1"));   // (January 1, 2027 is a Friday)
            }
            if (t.id == QLatin1String("budget")) QCOMPARE(e.shownAt(QStringLiteral("B7")), QStringLiteral("$0") + QLocale().decimalPoint() + QStringLiteral("00"));   // (in the computer's own decimal mark)
        }
        QVERIFY(crunch::builtinTemplateBytes(QStringLiteral("none")).isEmpty());
    }

    // Open and Repair of an .xls file whose container is cut short: the workbook is found by its records, the cells that can be read are kept, and the person is told.
    void aDamagedXlsFileIsRepairedByItsRecords()
    {
        QByteArray rows;
        for (int r = 1; r <= 700; ++r) rows += "<row r=\"" + QByteArray::number(r) + "\"><c r=\"A" + QByteArray::number(r) + "\"><v>" + QByteArray::number(r * 3) + "</v></c></row>";
        QByteArray damaged;
        {
            EditWindow e;
            QVERIFY(e.open(buildXlsx({worksheet(QString::fromLatin1(rows))})));
            const QString target = e.folder.dir.filePath(QStringLiteral("whole.xls"));
            QSignalSpy copied(&e.win.w, &CrunchWindow::copyExported);
            QVERIFY(e.win.w.saveAs(target));
            QVERIFY(copied.count() > 0 || copied.wait(20000));
            QFile f(target);
            QVERIFY(f.open(QIODevice::ReadOnly));
            const QByteArray whole = f.readAll();
            QVERIFY(whole.size() > 6000);
            damaged = whole.left(whole.size() * 4 / 5);   // (the tables at the end that name the streams are gone)
        }
        EditWindow fixed;
        const QString again = fixed.folder.write(QStringLiteral("cut.xls"), damaged);
        QVERIFY2(fixed.win.open(again, OpenHow::Repair), qPrintable(fixed.win.told.join(QLatin1Char('|'))));
        QVERIFY(fixed.settle());
        QCOMPARE(fixed.shownAt(QStringLiteral("A1")), QStringLiteral("3"));
        QCOMPARE(fixed.shownAt(QStringLiteral("A100")), QStringLiteral("300"));
        QVERIFY2(fixed.win.told.join(QLatin1Char('|')).contains(QLatin1String("repaired")), qPrintable(fixed.win.told.join(QLatin1Char('|'))));
    }

    // The window's own Open and Repair on three kinds of damage to an .xls container (a broken link of the tables, a chain ended early by the tables' own end marker, and a cut): the person is
    // told in plain words what was done, and a join made by guess is named with the sheet where what was not checked begins.
    void theWindowTellsWhatOpenAndRepairDidToAnXls()
    {
        QByteArray rows;
        for (int r = 1; r <= 700; ++r) rows += "<row r=\"" + QByteArray::number(r) + "\"><c r=\"A" + QByteArray::number(r) + "\"><v>" + QByteArray::number(r * 3) + "</v></c></row>";
        QByteArray whole;
        {
            EditWindow e;
            QVERIFY(e.open(buildXlsx({worksheet(QString::fromLatin1(rows))})));
            const QString target = e.folder.dir.filePath(QStringLiteral("whole.xls"));
            QSignalSpy copied(&e.win.w, &CrunchWindow::copyExported);
            QVERIFY(e.win.w.saveAs(target));
            QVERIFY(copied.count() > 0 || copied.wait(20000));
            QFile f(target);
            QVERIFY(f.open(QIODevice::ReadOnly));
            whole = f.readAll();
        }
        QList<cfbdamage::Variant> kinds = cfbdamage::sectorDamage(whole, QStringLiteral("Workbook"), QStringLiteral("Other"));
        kinds += cfbdamage::cuts(whole);
        int checked = 0;
        for (const cfbdamage::Variant &v : std::as_const(kinds)) {
            if (v.id != QLatin1String("b") && v.id != QLatin1String("d") && v.id != QLatin1String("h50")) continue;
            ++checked;
            EditWindow fixed;
            QVERIFY2(fixed.win.open(fixed.folder.write(QStringLiteral("damaged.xls"), v.file), OpenHow::Repair), qPrintable(v.id + fixed.win.told.join(QLatin1Char('|'))));
            QVERIFY(fixed.settle());
            const QString shown = fixed.win.told.join(QLatin1Char('\n'));
            QVERIFY2(shown.contains(QLatin1String("Open and Repair")), qPrintable(v.id + shown));
            // (what was pieced together by a guess is shown, and the message says so, with a place for a person to look: the sheet and row, or the whole workbook)
            QVERIFY2(shown.contains(QLatin1String("pieced together from a damaged file")) && shown.contains(QLatin1String("Compare it with a backup")), qPrintable(v.id + shown));
            if (v.id != QLatin1String("h50")) QVERIFY2(shown.contains(QLatin1String("In sheet \"")) || shown.contains(QLatin1String("This whole workbook")), qPrintable(v.id + shown));
            else QVERIFY2(shown.contains(QLatin1String("cut short")), qPrintable(v.id + shown));
            QVERIFY2(!shown.contains(QLatin1String("ends where it should")) && !shown.contains(QLatin1String("read whole")) && !shown.contains(QLatin1String("left out")), qPrintable(v.id + shown));
        for (const QString &word : {QStringLiteral("sector"), QStringLiteral("stream"), QStringLiteral("directory"), QStringLiteral("chain"), QStringLiteral("FAT")})
            QVERIFY2(!shown.contains(word, Qt::CaseInsensitive), qPrintable(v.id + QLatin1Char(' ') + shown));
        }
        QCOMPARE(checked, 3);
    }

    // Versions of a workbook: one is kept, listed, and gone back to (the workbook as it was just before is kept first), and one opens as a copy in a window of its own.
    void versionsAreKeptListedAndRestored()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        write::VersionStore(e.path, QStringLiteral(".xlsx")).removeAll();
        QVERIFY(e.win.w.versions().isEmpty());
        QSignalSpy kept(&e.win.w, &CrunchWindow::versionKept);
        QVERIFY(e.win.w.saveVersion(QStringLiteral("First")));
        QVERIFY(kept.count() > 0 || kept.wait(20000));
        QVERIFY(kept.last().first().toBool());
        QCOMPARE(e.win.w.versions().size(), 1);
        QCOMPARE(e.win.w.versions().first().name, QStringLiteral("First"));
        QVERIFY(write::VersionStore(e.path, QStringLiteral(".xlsx")).read(e.win.w.versions().first().file).startsWith("PK"));
        // a change, then back to the first
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("77"));
        QVERIFY(e.settle());
        QCOMPARE(e.shownAt(QStringLiteral("A1")), QStringLiteral("77"));
        const QString first = e.win.w.versions().first().file;
        QSignalSpy loaded(&e.win.w, &CrunchWindow::loadFinished);
        QVERIFY(e.win.w.restoreVersion(first));
        QVERIFY(loaded.count() > 0 || loaded.wait(60000));
        QVERIFY(e.settle());
        QCOMPARE(e.shownAt(QStringLiteral("A1")), QStringLiteral("1"));
        QVERIFY(!e.win.w.isModified());
        const QVector<write::DocVersion> all = e.win.w.versions();
        QCOMPARE(all.size(), 2);
        QVERIFY(all.first().name.startsWith(QLatin1String("Before restoring")));   // (what the workbook was just before, kept first)
        // the kept state has the change
        QVERIFY(write::VersionStore(e.path, QStringLiteral(".xlsx")).read(all.first().file).startsWith("PK"));
        const QList<QWidget *> before = QApplication::topLevelWidgets();
        QVERIFY(e.win.w.openVersionCopy(all.first().file));
        bool opened = false;
        for (QWidget *top : QApplication::topLevelWidgets())
            if (auto *c = qobject_cast<CrunchWindow *>(top); c && c != &e.win.w && !before.contains(top)) {
                opened = true;
                c->setNotifier([](const QString &, const QString &) {});
                QVERIFY(QTest::qWaitFor([&] { return c->session() != nullptr && !c->isCalculating(); }, 20000));
                QVERIFY(c->windowTitle().contains(QLatin1String("Copy")));
                c->setWindowModified(false);
                c->close();
            }
        QVERIFY(opened);
        write::VersionStore(e.path, QStringLiteral(".xlsx")).removeAll();
    }

    // Make an Email: a .eml file with the workbook, or the workbook as a PDF, attached; nothing is sent.
    void aWorkbookIsMadeIntoAnEmailFile()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        for (const bool pdf : {false, true}) {
            const QString target = e.folder.dir.filePath(pdf ? QStringLiteral("pdf.eml") : QStringLiteral("book.eml"));
            e.win.w.setSavePicker([target](const QString &, const QString &) { return target; });
            QVERIFY2(e.win.w.exportEmail(pdf ? CrunchWindow::EmailWhat::Pdf : CrunchWindow::EmailWhat::Workbook), qPrintable(e.win.told.join(QLatin1Char('|'))));
            QFile f(target);
            QVERIFY(f.open(QIODevice::ReadOnly));
            const QByteArray message = f.readAll();
            QVERIFY(message.contains("MIME-Version: 1.0"));
            QVERIFY2(message.contains(pdf ? "application/pdf" : "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"), message.left(600).constData());
            QVERIFY(message.contains(pdf ? ".pdf" : ".xlsx"));
            QVERIFY(message.contains("Content-Transfer-Encoding: base64"));
        }
    }

    void personalInformationStaysOutOfLaterEdits()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        office::PackageProps p = e.win.w.properties();
        p.creator = QStringLiteral("Ann");
        e.win.w.applyProperties(p);
        QCOMPARE(e.win.w.removePersonalInformation(), 2);   // (the author, and the person who saved: the name in Settings)
        p = e.win.w.properties();
        p.title = QStringLiteral("After");
        e.win.w.applyProperties(p);
        QVERIFY(e.win.w.properties().lastModifiedBy.isEmpty());
        QVERIFY(e.win.w.properties().creator.isEmpty());
        QCOMPARE(e.win.w.properties().title, QStringLiteral("After"));
    }

    // The option takes every name out of the file, and nothing the file says: the authors of notes and of threads of comments as well as the properties; the cells, notes, and comment texts are as they were.
    void thePrivacyOptionTakesTheNamesOutOfTheFileAndKeepsTheContent()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] { Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false); });
        EditWindow e;
        QVERIFY(e.open(editBook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->setNote(0, 5, 0, QStringLiteral("Check this"), QStringLiteral("Ann Lee"), false, QStringLiteral("Add Note")).ok);
        QVERIFY(s->newComment(0, 5, 1, QStringLiteral("Is it right?"), QStringLiteral("Bo Ray"), QStringLiteral("New Comment")).ok);
        office::PackageProps p = e.win.w.properties();
        p.title = QStringLiteral("Kept");
        p.creator = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        p.manager = QStringLiteral("Bo Ray");
        e.win.w.applyProperties(p);
        QVERIFY(saveAndWaitHere(e.win.w));
        const auto parts = partsOf(e.path);
        for (auto it = parts.cbegin(); it != parts.cend(); ++it)
            for (const char *who : {"Ann Lee", "Acme", "Bo Ray", "Tester"})
                QVERIFY2(!it.value().contains(who), qPrintable(it.key() + QStringLiteral(": ") + QString::fromLatin1(who)));
        const QString second = e.folder.dir.filePath(QStringLiteral("second.xlsx"));
        QVERIFY(QFile::copy(e.path, second));
        Window again;
        QVERIFY(again.open(second));
        const office::PackageProps back = again.w.properties();
        QVERIFY(back.creator.isEmpty() && back.lastModifiedBy.isEmpty() && back.company.isEmpty() && back.manager.isEmpty());
        QCOMPARE(back.title, QStringLiteral("Kept"));
        const auto *note = again.w.session()->anchors().noteAt(0, 5, 0);
        QVERIFY(note);
        QCOMPARE(note->text, QStringLiteral("Check this"));
        QVERIFY(note->author != QStringLiteral("Ann Lee"));
        const auto *thread = again.w.session()->anchors().noteAt(0, 5, 1);
        QVERIFY(thread && thread->threaded && thread->thread);
        // The link of the legacy note to its thread (an author "tc={...}") is as the writer made it, and no note of the file begins with a name's run.
        bool threadLink = false;
        for (auto it = parts.cbegin(); it != parts.cend(); ++it)
            if (it.key().startsWith(QLatin1String("xl/comments"))) {
                threadLink = threadLink || it.value().contains("<author>tc={");
                QVERIFY2(!it.value().contains(">Ann Lee:<") && !it.value().contains(">Bo Ray:<"), qPrintable(it.key()));
            }
        QVERIFY(threadLink);
        QCOMPARE(thread->thread->size(), size_t(1));
        QCOMPARE(thread->thread->at(0).text, QStringLiteral("Is it right?"));
        QVERIFY(thread->thread->at(0).author != QStringLiteral("Bo Ray"));
    }

    // The open workbook is not what is scrubbed: a save and a copy leave its names, its history, and its mark as saved as they were.
    void thePrivacyOptionLeavesTheOpenWorkbookAsItWas()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] { Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false); });
        EditWindow e;
        QVERIFY(e.open(editBook()));
        office::PackageProps p = e.win.w.properties();
        p.creator = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        e.win.w.applyProperties(p);
        const int steps = e.win.w.undoStack()->count();
        QSignalSpy copied(&e.win.w, &CrunchWindow::copyExported);
        QVERIFY(e.win.w.saveAs(e.folder.dir.filePath(QStringLiteral("copy.xlsx")), true));
        QVERIFY(copied.count() > 0 || copied.wait(20000));
        // (a copy leaves the workbook as it was)
        QCOMPARE(e.win.w.properties().creator, QStringLiteral("Ann Lee"));
        QCOMPARE(e.win.w.properties().company, QStringLiteral("Acme"));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(e.win.w.isWindowModified());
        // (a save adds no step, and leaves the workbook saved: what it holds now is the file, which has no names)
        QVERIFY(saveAndWaitHere(e.win.w));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(!e.win.w.isWindowModified());
        const auto copy = partsOf(e.folder.dir.filePath(QStringLiteral("copy.xlsx")));
        QVERIFY(!copy.value(QStringLiteral("docProps/core.xml")).contains("Ann Lee"));
        QVERIFY(!partsOf(e.path).value(QStringLiteral("docProps/core.xml")).contains("Ann Lee"));
    }

    void whenTheNamesCantBeTakenOutThePersonIsToldAndTheFileIsStillWritten()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] {
            Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
            office::setPackageScrubHookForTest({});
        });
        EditWindow e;
        QVERIFY(e.open(editBook()));
        office::PackageProps p = e.win.w.properties();
        p.creator = QStringLiteral("Ann Lee");
        e.win.w.applyProperties(p);
        office::setPackageScrubHookForTest(jptest::damagePart("docProps/core.xml"));
        QVERIFY(saveAndWaitHere(e.win.w));
        QTRY_VERIFY2(e.win.told.join(QLatin1Char('|')).contains(QLatin1String("personal information could not be taken out")), qPrintable(e.win.told.join(QLatin1Char('|'))));
        QVERIFY(e.win.told.join(QLatin1Char('|')).contains(QLatin1String("docProps/core.xml")));
        QVERIFY(partsOf(e.path).contains(QStringLiteral("[Content_Types].xml")));
    }

    void withThePrivacyOptionOffASaveIsUnchanged()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
        EditWindow e;
        QVERIFY(e.open(editBook()));
        office::PackageProps p = e.win.w.properties();
        p.creator = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        e.win.w.applyProperties(p);
        QVERIFY(saveAndWaitHere(e.win.w));
        QByteArray again;
        QString why;
        QVERIFY(e.win.w.snapshotBytesForTest(&again, &why));
        auto stable = [](const QMap<QString, QByteArray> &parts) {
            QMap<QString, QByteArray> out;
            for (auto it = parts.cbegin(); it != parts.cend(); ++it)
                out.insert(it.key(), QString::fromUtf8(it.value()).replace(QRegularExpression(QStringLiteral("\\d{4}-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\d")), QStringLiteral("T")).toUtf8());
            return out;
        };
        QMap<QString, QByteArray> second;
        QVERIFY(jp::readZip(again, second));
        QCOMPARE(stable(partsOf(e.path)), stable(second));
        QVERIFY(partsOf(e.path).value(QStringLiteral("docProps/core.xml")).contains("Ann Lee"));
    }

    void aWorkbookSentByEmailHasNoNamesWhenTheOptionIsOn()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] { Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false); });
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.win.w.session()->setNote(0, 5, 0, QStringLiteral("Check this"), QStringLiteral("Ann Lee"), false, QStringLiteral("Add Note")).ok);
        const QString target = e.folder.dir.filePath(QStringLiteral("book.eml"));
        e.win.w.setSavePicker([target](const QString &, const QString &) { return target; });
        QVERIFY2(e.win.w.exportEmail(CrunchWindow::EmailWhat::Workbook), qPrintable(e.win.told.join(QLatin1Char('|'))));
        QFile f(target);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QMap<QString, QByteArray> parts;
        QVERIFY2(jp::readZip(jptest::attachmentOf(f.readAll(), ".xlsx"), parts), "the attachment is a package");
        QVERIFY(parts.contains(QStringLiteral("xl/comments1.xml")));
        for (auto it = parts.cbegin(); it != parts.cend(); ++it) QVERIFY2(!it.value().contains("Ann Lee"), qPrintable(it.key()));
    }

    // A copy as .ods or .xls (and a template) has no names in it either: the writers of those forms put none of the workbook's properties or notes' authors in the file.
    void thePrivacyOptionKeepsNamesOutOfOdsAndXlsCopies()
    {
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), true);
        const auto reset = qScopeGuard([] { Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false); });
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.win.w.session()->setNote(0, 5, 0, QStringLiteral("Check this"), QStringLiteral("Ann Lee"), false, QStringLiteral("Add Note")).ok);
        office::PackageProps p = e.win.w.properties();
        p.creator = QStringLiteral("Ann Lee");
        p.company = QStringLiteral("Acme");
        p.manager = QStringLiteral("Bo Ray");
        e.win.w.applyProperties(p);
        QVERIFY(saveAndWaitHere(e.win.w));   // (the workbook's own file has them taken out; a copy is made from what is open)
        for (const char *name : {"copy.ods", "copy.xls", "copy.ots"}) {
            const QString target = e.folder.dir.filePath(QString::fromLatin1(name));
            QSignalSpy copied(&e.win.w, &CrunchWindow::copyExported);
            QVERIFY(e.win.w.saveAs(target, true));
            QVERIFY(copied.count() > 0 || copied.wait(20000));
            QFile f(target);
            QVERIFY2(f.open(QIODevice::ReadOnly), name);
            const QByteArray bytes = f.readAll();
            QVERIFY(bytes.size() > 0);
            QMap<QString, QByteArray> parts;
            if (!jp::readZip(bytes, parts)) parts.insert(QStringLiteral("whole"), bytes);
            for (auto it = parts.cbegin(); it != parts.cend(); ++it)
                for (const char *who : {"Ann Lee", "Acme", "Bo Ray", "Tester"}) {
                    QVERIFY2(!it.value().contains(who), qPrintable(QString::fromLatin1(name) + QLatin1Char(' ') + it.key() + QStringLiteral(": ") + QString::fromLatin1(who)));
                    const QString w = QString::fromLatin1(who);
                    QVERIFY2(!it.value().contains(QByteArray(reinterpret_cast<const char *>(w.utf16()), qsizetype(w.size() * 2))), qPrintable(QString::fromLatin1(name) + QStringLiteral(" UTF-16 ") + w));
                }
        }
    }

    void theInfoPageHasTheBoxForThePrivacyOption()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        const QVariant priorSetting = Settings::get().value(QStringLiteral("privacy/removeOnSave"), false);
        const auto restore = qScopeGuard([priorSetting] { Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), priorSetting); });
        Settings::get().setValue(QStringLiteral("privacy/removeOnSave"), false);
        std::unique_ptr<QWidget> page(e.win.w.buildInfoPage(nullptr));
        auto *box = page->findChild<QCheckBox *>(QStringLiteral("infoRemovePersonal"));
        QVERIFY(box);
        QCOMPARE(box->text(), QStringLiteral("Remove personal information from this file when saving"));
        QVERIFY(!box->isChecked());
        box->setChecked(true);
        QVERIFY(Settings::get().value(QStringLiteral("privacy/removeOnSave")).toBool());
    }

    void markAsFinalIsRefusedForTextFilesAndStaysVisibleOnTheStatusBar()
    {
        EditWindow t;
        QVERIFY(t.open("a,b\n1,2\n", QStringLiteral("data.csv")));
        QVERIFY(!t.win.w.markFinalBlocked().isEmpty());
        t.win.w.setMarkedFinal(true);
        QVERIFY(!t.win.w.isReadOnly());
        QVERIFY(!t.win.w.markedAsFinal());
        QVERIFY(t.win.told.last().contains(QLatin1String("can't hold the mark")));
        // an .xls and an .ods hold no mark either (named so, whatever they hold)
        for (const char *name : {"old.xls", "sheet.ods"}) {
            EditWindow o;
            QVERIFY(o.open(editBook(), QString::fromLatin1(name)));
            QVERIFY2(!o.win.w.markFinalBlocked().isEmpty(), name);
            o.win.w.setMarkedFinal(true);
            QVERIFY(!o.win.w.isReadOnly());
            QVERIFY(!o.win.w.markedAsFinal());
        }
        // a workbook marked as final shows its note on the status bar for as long as it is read-only, whatever else the bar says
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.win.w.markFinalBlocked().isEmpty());
        e.win.w.setMarkedFinal(true);
        QTRY_VERIFY_WITH_TIMEOUT(e.win.w.isReadOnly(), 30000);   // (a busy Windows runner took 5.5 s, Nov 17)
        QLabel *note = e.win.w.findChild<QLabel *>(QStringLiteral("finalNote"));
        QVERIFY(note);
        QVERIFY(note->isVisibleTo(&e.win.w));
        QVERIFY(note->text().contains(QLatin1String("final")));
        e.win.w.setMarkedFinal(false);
        QVERIFY(!note->isVisibleTo(&e.win.w));
    }

    void theInfoPageHasItsCardsAndEditsProperties()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.showFileScreen(QStringLiteral("info"));
        auto *screen = e.win.w.findChild<FileScreen *>();
        QVERIFY(screen);
        QCOMPARE(screen->currentPage(), QStringLiteral("info"));
        QWidget *page = screen->findChild<QWidget *>(QStringLiteral("infoPage"));
        QVERIFY(page);
        for (const char *name : {"infoMarkFinal", "infoEncrypt", "infoProtectSheet", "infoProtectStructure", "infoInspect", "infoCompatibility", "infoCustom", "infoShowFolder", "infoCopyPath"})
            QVERIFY2(page->findChild<QWidget *>(QString::fromLatin1(name)), name);
        {   // (Open File Location is there under its name, and is as ready as Copy Path: both go with the file having a place.)
            auto *where = page->findChild<QPushButton *>(QStringLiteral("infoShowFolder"));
            QVERIFY(where);
            QCOMPARE(where->text(), QStringLiteral("Open File Location"));
            QCOMPARE(where->isEnabled(), page->findChild<QPushButton *>(QStringLiteral("infoCopyPath"))->isEnabled());
        }
        auto *title = page->findChild<QLineEdit *>(QStringLiteral("infoTitle"));
        QVERIFY(title);
        title->setText(QStringLiteral("From the page"));
        Q_EMIT title->editingFinished();
        QCOMPARE(e.win.w.properties().title, QStringLiteral("From the page"));
        QVERIFY(e.win.w.isWindowModified());
        e.win.w.hideFileScreen();
    }
    // Pack and Go saves the workbook with the picture it links to, the link moved inside the package, so that it opens from the new folder with the picture beside it.
    void packAndGoTakesALinkedPictureAndMovesTheLink()
    {
        EditWindow e;
        QImage image(40, 30, QImage::Format_RGB32);
        image.fill(Qt::blue);
        const QString picture = e.folder.dir.filePath(QStringLiteral("elsewhere/logo.png"));
        QVERIFY(QDir().mkpath(e.folder.dir.filePath(QStringLiteral("elsewhere"))));
        QVERIFY(image.save(picture));
        const QString drawing = QStringLiteral("<xdr:wsDr xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
                                               "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><xdr:twoCellAnchor><xdr:from><xdr:col>4</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>1</xdr:row>"
                                               "<xdr:rowOff>0</xdr:rowOff></xdr:from><xdr:to><xdr:col>6</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>4</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to><xdr:pic><xdr:nvPicPr>"
                                               "<xdr:cNvPr id=\"2\" name=\"Logo\"/><xdr:cNvPicPr/></xdr:nvPicPr><xdr:blipFill><a:blip r:link=\"rId1\"/><a:stretch><a:fillRect/></a:stretch></xdr:blipFill><xdr:spPr>"
                                               "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></xdr:spPr></xdr:pic><xdr:clientData/></xdr:twoCellAnchor></xdr:wsDr>");
        const QString rels = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                                            "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"%1\" TargetMode=\"External\"/></Relationships>").arg(picture);
        QVector<ExtraPart> parts;
        parts.append({QStringLiteral("xl/drawings/drawing1.xml"), drawing.toUtf8(), QStringLiteral("application/vnd.openxmlformats-officedocument.drawing+xml"), 0, QStringLiteral("rIdD1"), QStringLiteral("drawing"),
                      QStringLiteral("../drawings/drawing1.xml")});
        parts.append({QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"), rels.toUtf8(), QString(), -1, QString(), QString(), QString()});
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>");
        QVERIFY(e.open(withParts(buildXlsx({worksheet(rows, QStringLiteral("<drawing r:id=\"rIdD1\"/>"))}), parts)));
        QDir().mkpath(e.folder.dir.filePath(QStringLiteral("dest")));
        const jp::PackReport report = e.win.w.packAndGoTo(e.folder.dir.filePath(QStringLiteral("dest")), jp::PackMode::Folder);
        QVERIFY2(report.ok, qPrintable(report.error));
        QCOMPARE(report.packed(), 1);
        QVERIFY(QFileInfo(report.path + QStringLiteral("/linked/logo.png")).isFile());
        QFile copy(report.path + QStringLiteral("/") + QFileInfo(e.path).completeBaseName() + QStringLiteral(".xlsx"));
        QVERIFY(copy.open(QIODevice::ReadOnly));
        const QByteArray packed = copy.readAll();
        // every relationship of the packed file is inside it or external, and it opens with nothing said (no repair)
        int external = 0;
        QCOMPARE(packedcheck::problemsOf(packed, &external), QString());
        QVERIFY(external >= 1);
        const QVector<jp::PackLink> links = jp::OpcPackSource::linksOf(packed);
        QCOMPARE(links.size(), 1);
        QCOMPARE(links[0].target, QStringLiteral("linked/logo.png"));
        // (it opens from its folder)
        EditWindow v;
        QVERIFY2(v.win.open(copy.fileName()), qPrintable(v.win.told.join(QLatin1Char('|'))));
        QVERIFY2(v.win.told.isEmpty(), qPrintable(v.win.told.join(QLatin1Char('|'))));
    }
    // A picture of the Export page with the two Pack and Go cards (only with JO_SHOT_DIR set: it makes the program's style the new one for the rest of the process).
    void packAndGoPageScreenshot()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        EditWindow e;
        auto &w = e.win.w;
        w.resize(1280, 800);
        w.show();
        w.showFileScreen(QStringLiteral("export"));
        QTRY_VERIFY(w.findChild<QAbstractButton *>(QStringLiteral("packAndGoZip")) && w.findChild<QAbstractButton *>(QStringLiteral("packAndGoZip"))->isVisible());
        QVERIFY(w.grab().save(dir + QStringLiteral("/crunch-pack-and-go.png")));
    }
};

JP_TEST_CLASS(CrunchInfoTests)
#include "tst_crunch_info.moc"
