// Tests of saving the page setup: the elements of a sheet's XML changed only where the setup differs from what they say, missing elements put where the format has them, the fit to
// pages in sheetPr, the names of the print area and titles in the workbook, and the whole save through saveXlsx. Every file is built here.
#include "sheethelpers.h"

#include "io/zipindex.h"
#include "sheet/pagesetupxml.h"
#include "sheet/xlsxsave.h"
#include "sheet/xmlscan.h"

#include <QXmlStreamReader>

using namespace jp;

namespace {

QStringList topLevelNames(const QByteArray &xml)
{
    QStringList names;
    xmlscan::Tag t;
    qsizetype pos = 0;
    int depth = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing) {
            --depth;
            continue;
        }
        if (depth == 0) names << QString::fromLatin1(t.local.toByteArray());
        if (!t.selfClosing) ++depth;
    }
    return names;
}

// What follows the cells of a sheet, with the closing tag of the root.
QByteArray tailOf(const QByteArray &inside) { return inside + "</worksheet>"; }

} // namespace

class SheetPageSaveTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A setup that says what the elements say changes no byte, however the file spaced and spelled them.
    void anUnchangedSetupLeavesEveryByte()
    {
        const QByteArray tail = tailOf("<mergeCells count=\"1\"><mergeCell ref=\"A1:B2\"/></mergeCells>\n  <printOptions   gridLines=\"1\" />\n<pageMargins left=\"0.70\" right='0.7' top=\"0.75\" bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/>"
                                       "<pageSetup paperSize=\"9\" scale=\"90\" orientation=\"landscape\" horizontalDpi=\"600\" r:id=\"rId1\"/>"
                                       "<headerFooter><oddHeader>&amp;CTitle</oddHeader></headerFooter><rowBreaks count=\"1\" manualBreakCount=\"1\"><brk id=\"10\" max=\"16383\" man=\"1\"/></rowBreaks>"
                                       "<drawing r:id=\"rId2\"/>");
        const PageSetup now = pageSetupOfXml(tail);
        QCOMPARE(now.paperSize, 9);
        QCOMPARE(patchPageSetupTail(tail, now, QByteArray()), tail);
    }

    // Only the attributes whose value differs are written again; the others, and those the model does not hold, stay as they were.
    void onlyTheChangedAttributesAreTouched()
    {
        const QByteArray tail = tailOf("<pageMargins left=\"0.70\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.30\" footer=\"0.3\"/>"
                                       "<pageSetup paperSize=\"9\" scale=\"90\" orientation=\"landscape\" horizontalDpi=\"600\" r:id=\"rId1\"/>");
        PageSetup now = pageSetupOfXml(tail);
        now.margins.left = 1;
        now.scale = 70;
        now.blackAndWhite = true;
        const QByteArray out = patchPageSetupTail(tail, now, QByteArray());
        QCOMPARE(out, tailOf("<pageMargins left=\"1\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.30\" footer=\"0.3\"/>"
                             "<pageSetup paperSize=\"9\" scale=\"70\" orientation=\"landscape\" horizontalDpi=\"600\" r:id=\"rId1\" blackAndWhite=\"1\"/>"));
        QCOMPARE(pageSetupOfXml(out), now);
    }

    // Elements the file lacks are put in the order the format has them: after what comes before them and before what comes after.
    void missingElementsGoWhereTheFormatPutsThem()
    {
        const QByteArray tail = tailOf("<hyperlinks><hyperlink ref=\"A1\" r:id=\"rId1\"/></hyperlinks><drawing r:id=\"rId2\"/><tableParts count=\"1\"><tablePart r:id=\"rId3\"/></tableParts>");
        PageSetup now;
        now.gridLines = true;
        now.centerHorizontally = true;
        now.margins.top = 1.25;
        now.orientation = PageOrientation::Landscape;
        now.fitToWidth = 2;
        now.headerFooter.oddHeader = QStringLiteral("&CReport & <notes>");
        now.headerFooter.differentFirst = true;
        now.headerFooter.firstHeader = QStringLiteral("&LFirst");
        PageBreak b;
        b.before = 20;
        now.rowBreaks.append(b);
        b.before = 5;
        now.colBreaks.append(b);
        const QByteArray out = patchPageSetupTail(tail, now, QByteArray());
        QCOMPARE(topLevelNames(out), QStringList({"hyperlinks", "printOptions", "pageMargins", "pageSetup", "headerFooter", "rowBreaks", "colBreaks", "drawing", "tableParts"}));
        const PageSetup back = pageSetupOfXml(out);
        QCOMPARE(back.gridLines, true);
        QCOMPARE(back.centerHorizontally, true);
        QCOMPARE(back.margins.top, 1.25);
        QVERIFY(back.orientation == PageOrientation::Landscape);
        QCOMPARE(back.fitToWidth, 2);
        QCOMPARE(back.headerFooter.oddHeader, now.headerFooter.oddHeader);
        QCOMPARE(back.headerFooter.firstHeader, now.headerFooter.firstHeader);
        QVERIFY(back.headerFooter.differentFirst);
        QCOMPARE(back.rowBreaks.size(), 1);
        QCOMPARE(back.rowBreaks[0].before, 20u);
        QVERIFY(back.rowBreaks[0].manual);
        QCOMPARE(back.colBreaks[0].before, 5u);
        QVERIFY(out.contains("<rowBreaks count=\"1\" manualBreakCount=\"1\"><brk id=\"20\" max=\"16383\" man=\"1\"/></rowBreaks>"));
        QVERIFY(out.contains("<colBreaks count=\"1\" manualBreakCount=\"1\"><brk id=\"5\" max=\"1048575\" man=\"1\"/></colBreaks>"));
        // (what was there before and after is as it was)
        QVERIFY(out.startsWith("<hyperlinks><hyperlink ref=\"A1\" r:id=\"rId1\"/></hyperlinks>"));
        QVERIFY(out.endsWith("<drawing r:id=\"rId2\"/><tableParts count=\"1\"><tablePart r:id=\"rId3\"/></tableParts></worksheet>"));
        // a root with a prefix, and a part that ends with the root
        const QByteArray prefixed = patchPageSetupTail(QByteArray("</x:worksheet>"), now, "x:");
        QVERIFY(prefixed.contains("<x:pageMargins "));
        QVERIFY(prefixed.endsWith("</x:worksheet>"));
    }

    // A setup of the defaults adds nothing; one that says no more than the defaults takes away the elements whose content it empties.
    void defaultsAddNothingAndEmptyElementsGo()
    {
        const QByteArray plain = tailOf("<drawing r:id=\"rId2\"/>");
        QCOMPARE(patchPageSetupTail(plain, PageSetup(), QByteArray()), plain);
        const QByteArray tail = tailOf("<headerFooter><oddFooter>&amp;P</oddFooter></headerFooter><rowBreaks count=\"2\" manualBreakCount=\"2\"><brk id=\"10\" max=\"16383\" man=\"1\"/><brk id=\"20\" max=\"16383\" man=\"1\"/></rowBreaks>"
                                       "<colBreaks count=\"1\" manualBreakCount=\"1\"><brk id=\"3\" max=\"1048575\" man=\"1\"/></colBreaks><drawing r:id=\"rId2\"/>");
        PageSetup now = pageSetupOfXml(tail);
        now.headerFooter = HeaderFooter();
        now.rowBreaks.removeLast();
        now.colBreaks.clear();
        const QByteArray out = patchPageSetupTail(tail, now, QByteArray());
        QCOMPARE(out, tailOf("<rowBreaks count=\"1\" manualBreakCount=\"1\"><brk id=\"10\" max=\"16383\" man=\"1\"/></rowBreaks><drawing r:id=\"rId2\"/>"));
    }

    // An attribute of the header and footer, and its texts, are changed; the others stay.
    void theHeaderAndFooterAreChanged()
    {
        const QByteArray tail = tailOf("<headerFooter alignWithMargins=\"0\"><oddHeader>&amp;Cold</oddHeader><oddFooter>&amp;Rfoot</oddFooter></headerFooter>");
        PageSetup now = pageSetupOfXml(tail);
        QVERIFY(!now.headerFooter.alignWithMargins);
        now.headerFooter.oddHeader = QStringLiteral("&Lnew");
        now.headerFooter.scaleWithDoc = false;
        const QByteArray out = patchPageSetupTail(tail, now, QByteArray());
        QCOMPARE(out, tailOf("<headerFooter alignWithMargins=\"0\" scaleWithDoc=\"0\"><oddHeader>&amp;Lnew</oddHeader><oddFooter>&amp;Rfoot</oddFooter></headerFooter>"));
    }

    // The fit to pages of sheetPr: changed where it is, added to sheetPr, or made with the element.
    void theFitToPagesIsPatchedInSheetPr()
    {
        PageSetup on;
        on.fitToPage = true;
        const QByteArray decl = "<?xml version=\"1.0\"?>\n<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">";
        // none at all: sheetPr is made, first
        QByteArray head = decl + "<dimension ref=\"A1\"/>";
        QByteArray out = patchFitToPageHead(head, on);
        QCOMPARE(out, decl + "<sheetPr><pageSetUpPr fitToPage=\"1\"/></sheetPr><dimension ref=\"A1\"/>");
        QVERIFY(fitToPageOfXml(out));
        // sheetPr with a tab color: pageSetUpPr goes last in it
        head = decl + "<sheetPr><tabColor rgb=\"FF00FF00\"/></sheetPr><dimension ref=\"A1\"/>";
        QCOMPARE(patchFitToPageHead(head, on), decl + "<sheetPr><tabColor rgb=\"FF00FF00\"/><pageSetUpPr fitToPage=\"1\"/></sheetPr><dimension ref=\"A1\"/>");
        // sheetPr that closes itself
        QCOMPARE(patchFitToPageHead(decl + "<sheetPr codeName=\"S1\"/><dimension ref=\"A1\"/>", on), decl + "<sheetPr codeName=\"S1\"><pageSetUpPr fitToPage=\"1\"/></sheetPr><dimension ref=\"A1\"/>");
        // the element is there: its attribute is set, or the setup says the same and nothing is touched
        head = decl + "<sheetPr><pageSetUpPr fitToPage=\"1\" autoPageBreaks=\"0\"/></sheetPr>";
        QCOMPARE(patchFitToPageHead(head, on), head);
        QCOMPARE(patchFitToPageHead(head, PageSetup()), decl + "<sheetPr><pageSetUpPr fitToPage=\"0\" autoPageBreaks=\"0\"/></sheetPr>");
        // nothing to say and nothing there
        QCOMPARE(patchFitToPageHead(decl + "<dimension ref=\"A1\"/>", PageSetup()), decl + "<dimension ref=\"A1\"/>");
    }

    // The names of the print area and titles are changed where the setup differs from them, added, and taken away.
    void thePrintNamesArePatched()
    {
        const QByteArray head = "<?xml version=\"1.0\"?>\n<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">";
        const QByteArray sheets = "<sheets><sheet name=\"Data\" sheetId=\"1\" r:id=\"rId1\"/><sheet name=\"My Sheet\" sheetId=\"2\" r:id=\"rId2\"/></sheets>";
        // a name that says what the setup says is not touched
        const QByteArray names = "<definedNames><definedName name=\"Total\">Data!$A$1</definedName><definedName name=\"_xlnm.Print_Area\" localSheetId=\"0\">Data!$A$1:$D$20</definedName></definedNames>";
        const QByteArray xml = head + sheets + names + "<calcPr calcId=\"1\"/></workbook>";
        PrintNameChange same;
        same.sheet = 0;
        same.sheetName = QStringLiteral("Data");
        parseRangeRef(QStringLiteral("A1:D20"), &same.setup.printArea.emplace_back());
        QByteArray out;
        QVERIFY(patchPrintNames(xml, {same}, &out));
        QCOMPARE(out, xml);
        // changed, and a second sheet's added (with titles), in a part that has the names
        PrintNameChange changed = same;
        changed.setup.printArea[0].row2 = 29;
        PrintNameChange second;
        second.sheet = 1;
        second.sheetName = QStringLiteral("My Sheet");
        second.setup.titles.hasRows = true;
        second.setup.titles.row1 = 0;
        second.setup.titles.row2 = 1;
        QVERIFY(patchPrintNames(xml, {changed, second}, &out));
        // (the whole part, parsed: well formed, and exactly the names that should be there)
        QCOMPARE(out, head + sheets + "<definedNames><definedName name=\"Total\">Data!$A$1</definedName><definedName name=\"_xlnm.Print_Area\" localSheetId=\"0\">Data!$A$1:$D$30</definedName>"
                                      "<definedName name=\"_xlnm.Print_Titles\" localSheetId=\"1\">'My Sheet'!$1:$2</definedName></definedNames><calcPr calcId=\"1\"/></workbook>");
        {
            QXmlStreamReader r(out);
            int names = 0;
            while (!r.atEnd()) {
                if (r.readNext() == QXmlStreamReader::StartElement && r.name() == QLatin1String("definedName")) ++names;
            }
            QVERIFY2(!r.hasError(), qPrintable(r.errorString()));
            QCOMPARE(names, 3);
        }
        // taken away
        PrintNameChange none = same;
        none.setup.printArea.clear();
        QVERIFY(patchPrintNames(xml, {none}, &out));
        QVERIFY(!out.contains("Print_Area"));
        QVERIFY(out.contains("<definedName name=\"Total\">Data!$A$1</definedName>"));
        // a part with no names gets them after the sheets (and before calcPr)
        const QByteArray bare = head + sheets + "<calcPr calcId=\"1\"/></workbook>";
        QVERIFY(patchPrintNames(bare, {same}, &out));
        QCOMPARE(out, head + sheets + "<definedNames><definedName name=\"_xlnm.Print_Area\" localSheetId=\"0\">Data!$A$1:$D$20</definedName></definedNames><calcPr calcId=\"1\"/></workbook>");
        // nothing to say and no names: untouched
        QVERIFY(patchPrintNames(bare, {PrintNameChange()}, &out));
        QCOMPARE(out, bare);
    }

    // The whole save: a sheet's page setup and the names change, the other sheet and every other part are copied as they were.
    void aSavedFileHasTheNewSetupAndLeavesTheRestAlone()
    {
        const QByteArray first = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), QStringLiteral("<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/>"));
        const QByteArray second = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>2</v></c></row>"));
        const QByteArray book = buildXlsx({first, second}, QByteArray(), QString(), QString(), QByteArray(), QStringList({"A", "B"}));
        Package package;
        QString why;
        QVERIFY2(package.readLazy(book, &why), qPrintable(why));
        Workbook wb;
        QVERIFY2(wb.open(package, &why), qPrintable(why));
        SaveChanges changes;
        SheetChanges &sc = changes.sheets[0];
        sc.setPage = true;
        sc.page.orientation = PageOrientation::Landscape;
        sc.page.fitToPage = true;
        sc.page.fitToWidth = 1;
        sc.page.fitToHeight = 0;
        sc.page.margins.left = 0.25;
        sc.page.gridLines = true;
        sc.page.headerFooter.oddFooter = QStringLiteral("&CPage &P of &N");
        sc.page.printArea.append(CellRange());
        parseRangeRef(QStringLiteral("A1:C5"), &sc.page.printArea[0]);
        PrintNameChange pn;
        pn.sheet = 0;
        pn.sheetName = QStringLiteral("A");
        pn.setup = sc.page;
        changes.printNames.push_back(pn);
        QByteArray out;
        SaveReport report;
        QVERIFY2(saveXlsx(wb, changes, &out, &why, &report), qPrintable(why));
        QCOMPARE(report.sheetsRewritten, 1);
        Package again;
        QVERIFY2(again.readLazy(out, &why), qPrintable(why));
        Workbook reopened;
        QVERIFY2(reopened.open(again, &why), qPrintable(why));
        Sheet a, b;
        QVERIFY(reopened.readSheet(0, &a));
        QVERIFY(reopened.readSheet(1, &b));
        QVERIFY(a.page.orientation == PageOrientation::Landscape);
        QVERIFY(a.page.fitToPage);
        QCOMPARE(a.page.fitToHeight, 0);
        QCOMPARE(a.page.margins.left, 0.25);
        QVERIFY(a.page.gridLines);
        QCOMPARE(a.page.headerFooter.oddFooter, QStringLiteral("&CPage &P of &N"));
        QCOMPARE(a.page.printArea.size(), 1);
        QCOMPARE(rangeName(a.page.printArea[0]), QStringLiteral("A1:C5"));
        QCOMPARE(b.page, PageSetup());
        // the other sheet's part is the same bytes
        QVector<ZipEntry> was, now;
        QVERIFY(readZipDirectory(book, &was) && readZipDirectory(out, &now));
        auto raw = [](const QByteArray &zip, const QVector<ZipEntry> &dir, const QString &name) {
            for (const ZipEntry &e : dir)
                if (e.name == name) return zip.mid(int(e.dataAt), int(e.csize));
            return QByteArray();
        };
        QVERIFY(!raw(book, was, QStringLiteral("xl/worksheets/sheet2.xml")).isEmpty());
        QCOMPARE(raw(out, now, QStringLiteral("xl/worksheets/sheet2.xml")), raw(book, was, QStringLiteral("xl/worksheets/sheet2.xml")));
    }
};

JP_TEST_CLASS(SheetPageSaveTests)
#include "tst_sheet_pagesave.moc"
