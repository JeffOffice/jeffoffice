// Tests of the reader of OpenDocument spreadsheets added after its review: spaces and the like bounded as text is, the parts that hold styles and settings capped and stopped,
// the style budget, the names of functions that differ between the two formats, the null dates, what is skipped and what is changed being said, sheet names the .xlsx form
// does not allow, and plain words in every message. Every document is built in memory by odshelpers.
#include "odshelpers.h"

#include "sheet/odsformula.h"
#include "sheet/xlsxbase.h"

class SheetOdsReviewTests : public QObject {
    Q_OBJECT

    static bool openIt(const QByteArray &bytes, Workbook *wb, QString *why, const Limits &limits = Limits()) { return openBytes(bytes, QStringLiteral("review.ods"), wb, why, limits); }
    static QByteArray simple(const QString &tables, const QString &styles = QString(), const QString &before = QString()) { return buildOds(contentXml(tables, styles, before)); }
    static QString cellWith(const QString &inside) { return cell(QStringLiteral("office:value-type=\"string\""), QStringLiteral("<text:p>") + inside + QStringLiteral("</text:p>")); }

private Q_SLOTS:
    // Item 1: spaces, tabs, and line breaks are bounded by the limit of a string as text is, in a cell and in all: a document of a few kilobytes that stands for gigabytes of spaces is
    // refused, and a cell is never given more than the limit.
    void spacesTabsAndBreaksAreBoundedLikeText()
    {
        Limits limits;
        limits.stringChars = 100000;
        QString many;
        for (int i = 0; i < 20000; ++i) many += QStringLiteral("<text:s text:c=\"10000\"/>");   // (two hundred million spaces: 400 MB, if they were made)
        Workbook wb;
        QString why;
        const QByteArray bomb = simple(table(QStringLiteral("S"), columns(1) + row(floatCell(1)) + row(cellWith(many))));
        many.clear();
        many.squeeze();
        QVERIFY2(openIt(bomb, &wb, &why, limits), qPrintable(why));
        Sheet s;
        QVERIFY(!wb.readSheet(0, &s));
        // (the reader's own bound: the text of the cell is never made longer than the limit, however many spaces the document stands for; it counts the longest text it made)
        QVERIFY2(s.error.contains(QLatin1String("longer")), qPrintable(s.error));
        QVERIFY2(s.stats.longestString <= limits.stringChars, qPrintable(QString::number(s.stats.longestString)));
        QString tabs;
        for (int i = 0; i < 300000; ++i) tabs += QStringLiteral("<text:tab/><text:line-break/>");
        Workbook wb2;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(1) + row(cellWith(tabs)))), &wb2, &why, limits), qPrintable(why));
        QVERIFY(!wb2.readSheet(0, &s));
        QVERIFY2(s.error.contains(QLatin1String("longer")), qPrintable(s.error));
        QVERIFY(s.stats.longestString <= limits.stringChars);
        // paragraphs count too
        QString paragraphs;
        for (int i = 0; i < 2000; ++i) paragraphs += QStringLiteral("</text:p><text:p>x");
        Workbook wb3;
        limits.stringChars = 1000;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(1) + row(cellWith(paragraphs)))), &wb3, &why, limits), qPrintable(why));
        QVERIFY(!wb3.readSheet(0, &s));
        // in all: many cells that are each within the limit are bounded by what a sheet may take
        Limits total;
        total.sheetBytes = 2000000;
        QString cells;
        for (int i = 0; i < 400; ++i) cells += cellWith(QString(10000, QLatin1Char('a')));
        Workbook wb4;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(400) + row(cells))), &wb4, &why, total), qPrintable(why));
        QVERIFY(!wb4.readSheet(0, &s));
        QVERIFY2(s.error.contains(QLatin1String("larger")), qPrintable(s.error));
    }

    // Item 3: the text in the attribute of a string cell is bounded as the text in the cell is.
    void theStringValueAttributeIsBounded()
    {
        Limits limits;
        limits.stringChars = 1000;
        Workbook wb;
        QString why;
        const QString cells = floatCell(1) + cell(QStringLiteral("office:value-type=\"string\" office:string-value=\"") + QString(5000, QLatin1Char('b')) + QStringLiteral("\""));
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(2) + row(cells))), &wb, &why, limits), qPrintable(why));
        Sheet s;
        QVERIFY(!wb.readSheet(0, &s));
        QVERIFY2(s.error.contains(QLatin1String("longer")), qPrintable(s.error));
    }

    // Item 2: the parts that hold the styles and the settings are capped as the content is, and a read that is stopped stops while they are read.
    void stylesAndSettingsAreCappedAndCanBeStopped()
    {
        Limits limits;
        limits.odsContentBytes = 20000;
        Workbook wb;
        QString why;
        QString bigStyles;
        for (int i = 0; i < 1000; ++i) bigStyles += QStringLiteral("<style:style style:name=\"x%1\" style:family=\"table-cell\"/>").arg(i);
        QVERIFY(!openIt(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1)))), bigStyles), &wb, &why, limits));
        QVERIFY2(why.contains(QLatin1String("larger")), qPrintable(why));
        QMap<QString, QMap<QString, QString>> sheets;
        for (int i = 0; i < 500; ++i) sheets[QStringLiteral("Sheet%1").arg(i)] = {{QStringLiteral("CursorPositionX"), QStringLiteral("1")}};
        QVERIFY(!openIt(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1)))), QString(), settingsXml(sheets)), &wb, &why, limits));
        QVERIFY2(why.contains(QLatin1String("larger")), qPrintable(why));
        // the manifest too
        const QByteArray manifest = "<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\">" + QByteArray(30000, ' ') + "</manifest:manifest>";
        QVERIFY(!openIt(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1)))), QString(), QByteArray(), {}, QStringLiteral("application/vnd.oasis.opendocument.spreadsheet"), manifest), &wb, &why, limits));
        QVERIFY2(why.contains(QLatin1String("larger")), qPrintable(why));
        // stopped
        QString styles;
        for (int i = 0; i < 5000; ++i) styles += QStringLiteral("<style:style style:name=\"x%1\" style:family=\"table-cell\"/>").arg(i);
        Limits stopped;
        int asked = 0;
        stopped.cancelled = [&asked] {
            ++asked;
            return true;
        };
        QVERIFY(!openIt(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1))), styles)), &wb, &why, stopped));
        QVERIFY2(why.contains(QLatin1String("stopped")), qPrintable(why));
        QVERIFY(asked > 0);
        // (and with no one stopping it, the same document opens)
        QVERIFY2(openIt(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1))), styles)), &wb, &why), qPrintable(why));
    }

    // Item 4: the styles a document defines, and their properties, are limited, and a document past a limit is refused in words (not cut short).
    void stylesAndTheirPropertiesAreLimited()
    {
        QString styles;
        for (int i = 0; i < 100; ++i) styles += QStringLiteral("<style:style style:name=\"s%1\" style:family=\"table-cell\"><style:text-properties fo:font-size=\"%2pt\"/></style:style>").arg(i).arg(8 + i % 40);
        Workbook wb;
        QString why;
        Limits limits;
        limits.odsStyles = 50;
        QVERIFY(!openIt(simple(table(QStringLiteral("S"), row(floatCell(1))), styles), &wb, &why, limits));
        QVERIFY2(why.contains(QLatin1String("more styles")), qPrintable(why));
        limits = Limits();
        limits.odsProperties = 60;
        QVERIFY(!openIt(simple(table(QStringLiteral("S"), row(floatCell(1))), styles), &wb, &why, limits));
        QVERIFY2(why.contains(QLatin1String("more styles")), qPrintable(why));
        // data styles count as styles
        QString data;
        for (int i = 0; i < 100; ++i) data += QStringLiteral("<number:number-style style:name=\"N%1\"><number:number number:decimal-places=\"2\"/></number:number-style>").arg(i);
        limits = Limits();
        limits.odsStyles = 50;
        QVERIFY(!openIt(simple(table(QStringLiteral("S"), row(floatCell(1))), data), &wb, &why, limits));
        QVERIFY2(why.contains(QLatin1String("more styles")), qPrintable(why));
        // the limit as the program has it is well under what the budget of a read can hold (a style costs about 4 KB)
        QVERIFY(kMaxOdsStyles <= 100000);
        QVERIFY(kMaxOdsStyles * 4096 < qint64(1) << 30);
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), row(floatCell(1))), styles), &wb, &why), qPrintable(why));
    }

    // Item 9: the names of functions that differ between the two formats become the model's, and the other way: the tilde forms, the .XCL forms, the old names, the add-ins.
    void namesOfFunctionsThatDifferAreMapped()
    {
        struct Case {
            const char *odf;
            const char *model;
        };
        const Case cases[] = {{"of:=ERRORTYPE([.A1])", "ERROR.TYPE(A1)"},
                              {"of:=ERROR.TYPE([.A1])", "ERROR.TYPE(A1)"},
                              {"of:=COM.MICROSOFT.CEILING.XCL([.A1];1)", "CEILING(A1,1)"},
                              {"of:=COM.MICROSOFT.FLOOR.XCL([.A1];1)", "FLOOR(A1,1)"},
                              {"of:=CEILING([.A1];1;1)", "_xlfn.CEILING.MATH(A1,1,1)"},
                              {"of:=FLOOR([.A1];2)", "_xlfn.FLOOR.MATH(A1,2)"},
                              {"of:=COM.MICROSOFT.CEILING.MATH([.A1])", "_xlfn.CEILING.MATH(A1)"},
                              {"of:=COM.MICROSOFT.CEILING.PRECISE([.A1])", "_xlfn.CEILING.PRECISE(A1)"},
                              {"of:=CHISQDIST([.A1];2;TRUE())", "_xlfn.CHISQ.DIST(A1,2,TRUE())"},
                              {"of:=CHISQINV([.A1];2)", "_xlfn.CHISQ.INV(A1,2)"},
                              {"of:=FORMULA([.A1])", "_xlfn.FORMULATEXT(A1)"},
                              {"of:=LEGACY.CHIDIST([.A1];2)", "CHIDIST(A1,2)"},
                              {"of:=LEGACY.FDIST([.A1];2;3)", "FDIST(A1,2,3)"},
                              {"of:=ORG.OPENOFFICE.EASTERSUNDAY(2024)", "EASTERSUNDAY(2024)"},
                              {"of:=COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETEDATE([.A1];1)", "EDATE(A1,1)"},
                              {"of:=COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETBIN2DEC(\"101\")", "BIN2DEC(\"101\")"},
                              {"of:=COM.MICROSOFT.STDEV.S([.A1:.A3])", "_xlfn.STDEV.S(A1:A3)"},
                              {"of:=IFERROR(NA();1)", "IFERROR(NA(),1)"}};
        for (const Case &c : cases) {
            bool ok = false;
            const QString text = odfFormulaToText(QString::fromLatin1(c.odf), &ok);
            QVERIFY2(ok, c.odf);
            QCOMPARE(text, QString::fromLatin1(c.model));
        }
        // and the other way: the model's name as the standard writes it
        QCOMPARE(odfFunctionName(QStringLiteral("ERROR.TYPE")), QStringLiteral("ERROR.TYPE"));
        QCOMPARE(odfFunctionName(QStringLiteral("FORMULATEXT")), QStringLiteral("FORMULA"));
        QCOMPARE(odfFunctionName(QStringLiteral("CEILING")), QStringLiteral("COM.MICROSOFT.CEILING.XCL"));
        QCOMPARE(odfFunctionName(QStringLiteral("CEILING.MATH")), QStringLiteral("CEILING"));
        QCOMPARE(odfFunctionName(QStringLiteral("CHISQ.DIST")), QStringLiteral("CHISQDIST"));
        QCOMPARE(odfFunctionName(QStringLiteral("STDEV.S")), QStringLiteral("COM.MICROSOFT.STDEV.S"));
        QCOMPARE(odfFunctionName(QStringLiteral("SUM")), QStringLiteral("SUM"));
        for (const char *model : {"ERROR.TYPE", "FORMULATEXT", "CEILING", "FLOOR", "CEILING.MATH", "FLOOR.MATH", "CHISQ.DIST", "CHISQ.INV", "STDEV.S", "SUM", "IFS", "TEXTJOIN"})
            QCOMPARE(modelFunctionName(odfFunctionName(QString::fromLatin1(model))), QString::fromLatin1(model));
    }

    // Item 10: a document that counts days from another null date has its dates where they are (2024-03-15 is 2024-03-15 in the model), and the numbers of cells formatted as dates are moved
    // by the difference; other numbers are not.
    void otherNullDatesKeepTheirDates()
    {
        const QString before = QStringLiteral("<table:calculation-settings><table:null-date table:date-value=\"1900-01-01\"/></table:calculation-settings>");
        const QString styles = QStringLiteral("<number:date-style style:name=\"D1\"><number:day/><number:text>.</number:text><number:month/><number:text>.</number:text><number:year number:style=\"long\"/></number:date-style>"
                                              "<style:style style:name=\"dt\" style:family=\"table-cell\" style:data-style-name=\"D1\"/>");
        // 45364 days after 1900-01-01 is 2024-03-15; the model counts from 1899-12-30, where that day is 45366
        const QString cells = cell(QStringLiteral("office:value-type=\"date\" office:date-value=\"2024-03-15\"")) + floatCell(45364, QStringLiteral("table:style-name=\"dt\"")) + floatCell(45364);
        Workbook wb;
        QString why;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(3) + row(cells)), styles, before), &wb, &why), qPrintable(why));
        QVERIFY(wb.info().dateSystem == DateSystem::Date1900);
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:45366"));
        QCOMPARE(shown(wb, s, QStringLiteral("B1")), QStringLiteral("n:45366"));
        QCOMPARE(shown(wb, s, QStringLiteral("C1")), QStringLiteral("n:45364"));
        // the 1904 null date is the 1904 system, as it was; the usual one needs no change
        const QString before1904 = QStringLiteral("<table:calculation-settings><table:null-date table:date-value=\"1904-01-01\"/></table:calculation-settings>");
        const QString cells1904 = cell(QStringLiteral("office:value-type=\"date\" office:date-value=\"2024-03-15\"")) + floatCell(44000, QStringLiteral("table:style-name=\"dt\""));
        Workbook w1904;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(2) + row(cells1904)), styles, before1904), &w1904, &why), qPrintable(why));
        QVERIFY(w1904.info().dateSystem == DateSystem::Date1904);
        QVERIFY(w1904.readSheet(0, &s));
        QCOMPARE(shown(w1904, s, QStringLiteral("A1")), QStringLiteral("n:43904"));
        QCOMPARE(shown(w1904, s, QStringLiteral("B1")), QStringLiteral("n:44000"));
        // a null date in the middle of the century
        const QString before1950 = QStringLiteral("<table:calculation-settings><table:null-date table:date-value=\"1950-06-15\"/></table:calculation-settings>");
        const QString cells1950 = cell(QStringLiteral("office:value-type=\"date\" office:date-value=\"2024-03-15\"")) + floatCell(1, QStringLiteral("table:style-name=\"dt\""));
        Workbook w1950;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(2) + row(cells1950)), styles, before1950), &w1950, &why), qPrintable(why));
        QVERIFY(w1950.readSheet(0, &s));
        QCOMPARE(shown(w1950, s, QStringLiteral("A1")), QStringLiteral("n:45366"));
        QCOMPARE(shown(w1950, s, QStringLiteral("B1")), QStringLiteral("n:18430"));   // (1950-06-16 is 18,430 days after 1899-12-30)
    }

    // Item 11: what is not read is always said, whichever way the document was found to have it, and so are the hyperlinks (the text stays) and the formulas that could not be
    // converted, with a count; every copy of a repeated row has its merged cells.
    void whatIsSkippedIsAlwaysSaid()
    {
        const QString shapes[] = {QStringLiteral("<draw:line draw:z-index=\"0\"/>"), QStringLiteral("<draw:control draw:control=\"c1\"/>"), QStringLiteral("<draw:connector draw:z-index=\"0\"/>"),
                                  QStringLiteral("<draw:g><draw:rect/></draw:g>"), QStringLiteral("<draw:frame/>"), QStringLiteral("<draw:ellipse/>")};
        for (const QString &shape : shapes) {
            Workbook wb;
            QString why;
            QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(1) + row(cell(QString(), shape)))), &wb, &why), qPrintable(why));
            QVERIFY2(wb.skippedFeatures().contains(QStringLiteral("pictures and other drawing objects")), qPrintable(shape));
        }
        // a document whose sheets are not written as the usual programs write them is found by the slow way, and says the same
        const QByteArray other =
            "<?xml version=\"1.0\"?><o:document-content xmlns:o=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:t=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" "
            "xmlns:x=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:d=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\"><o:body><o:spreadsheet><t:table t:name=\"P\"><t:table-row><t:table-cell>"
            "<d:line/><x:p>hi</x:p></t:table-cell></t:table-row></t:table></o:spreadsheet></o:body></o:document-content>";
        Workbook wb;
        QString why;
        QVERIFY2(openIt(buildOds(other), &wb, &why), qPrintable(why));
        QVERIFY(wb.skippedFeatures().contains(QStringLiteral("pictures and other drawing objects")));
        // hyperlinks: the text is kept, the link is said to be lost
        Workbook links;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(1) + row(cell(QStringLiteral("office:value-type=\"string\""),
                                                                                       QStringLiteral("<text:p>see <text:a xlink:href=\"http://example.com\">the page</text:a></text:p>"))))), &links, &why), qPrintable(why));
        QVERIFY(links.skippedFeatures().contains(QStringLiteral("hyperlinks")));
        Sheet s;
        QVERIFY(links.readSheet(0, &s));
        QCOMPARE(shown(links, s, QStringLiteral("A1")), QStringLiteral("i:see the page"));
        // formulas that could not be converted keep their values, and the notice says how many once the sheets are read
        const QString bad = cell(QStringLiteral("table:formula=\"of:=[ 'file:///x.ods'#$S.A1]\" office:value-type=\"float\" office:value=\"7\"")) + cell(QStringLiteral("table:formula=\"of:=MULTIPLE.OPERATIONS([.A1];[.B1];[.C1])\" office:value-type=\"float\" office:value=\"8\""));
        Workbook failed;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(2) + row(bad))), &failed, &why), qPrintable(why));
        QVERIFY(failed.changedFeatures().filter(QStringLiteral("formula")).isEmpty());   // (not known until the sheet is read)
        QVERIFY(failed.readSheet(0, &s));
        const QStringList changed = failed.changedFeatures();
        QVERIFY2(!changed.filter(QStringLiteral("2 formula")).isEmpty(), qPrintable(changed.join(QLatin1Char('|'))));
        // (read again, a sheet is counted once)
        QVERIFY(failed.readSheet(0, &s));
        QVERIFY(!failed.changedFeatures().filter(QStringLiteral("2 formula")).isEmpty());
        // names that could not be converted are left out, and the notice says how many
        const QString badNames = QStringLiteral("<table:named-expressions><table:named-expression table:name=\"Far\" table:base-cell-address=\"$S.$A$1\" table:expression=\"[ 'file:///x.ods'#$S.A1]\"/>"
                                                "<table:named-expression table:name=\"Fine\" table:base-cell-address=\"$S.$A$1\" table:expression=\"[$S.A1]*2\"/></table:named-expressions>");
        Workbook withNames;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), row(floatCell(1))) + badNames), &withNames, &why), qPrintable(why));
        QCOMPARE(withNames.info().definedNames.size(), 1);
        const QStringList namesChanged = withNames.changedFeatures();
        QVERIFY2(!namesChanged.filter(QStringLiteral("1 name")).isEmpty(), qPrintable(namesChanged.join(QLatin1Char('|'))));
        // merged cells in every copy of a repeated row
        const QString merged = row(stringCell(QStringLiteral("m"), QStringLiteral("table:number-columns-spanned=\"2\"")) + QStringLiteral("<table:covered-table-cell/>") + floatCell(1), QStringLiteral("table:number-rows-repeated=\"3\""));
        Workbook rep;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(3) + merged)), &rep, &why), qPrintable(why));
        QVERIFY(rep.readSheet(0, &s));
        QCOMPARE(s.merges.size(), 3);
        QCOMPARE(s.merges[2].row1, 2u);
        QCOMPARE(s.merges[2].col2, 1u);
    }

    // Item 13: sheet names the .xlsx form does not allow (over 31 characters, [ ] : * ? / \, an apostrophe at either end, a name another one repeats in other letters) are made valid and
    // different, the formulas and names that use them follow, and the notice says so.
    void sheetNamesAreMadeValidAndUnique()
    {
        const QString longName = QStringLiteral("A sheet whose name is far longer than thirty-one characters");
        const QString tables = table(QStringLiteral("Data"), columns(2) + row(floatCell(1) + cell(QStringLiteral("table:formula=\"of:=[$'%1'.A1]+[$'Odd[1]:x'.A1]+[$Data.A1]\" office:value-type=\"float\" office:value=\"3\"").arg(longName)))) + table(longName, row(floatCell(2)))
                              + table(QStringLiteral("Odd[1]:x"), row(floatCell(5))) + table(QStringLiteral("data"), row(floatCell(3))) + table(QStringLiteral("'quoted'"), row(floatCell(4))) + table(QStringLiteral("History"), row(floatCell(6)));
        const QString names = QStringLiteral("<table:named-expressions><table:named-range table:name=\"R\" table:base-cell-address=\"$Data.$A$1\" table:cell-range-address=\"$'%1'.$A$1\"/></table:named-expressions>").arg(longName);
        Workbook wb;
        QString why;
        QVERIFY2(openIt(simple(tables + names), &wb, &why), qPrintable(why));
        QStringList seen;
        for (const SheetEntry &e : wb.info().sheets) {
            QVERIFY2(e.name.size() <= 31, qPrintable(e.name));
            for (const QChar c : QStringLiteral("[]:*?/\\")) QVERIFY2(!e.name.contains(c), qPrintable(e.name));
            QVERIFY2(!e.name.startsWith(QLatin1Char('\'')) && !e.name.endsWith(QLatin1Char('\'')), qPrintable(e.name));
            QVERIFY2(!seen.contains(e.name.toLower()), qPrintable(e.name));
            QVERIFY2(e.name.compare(QLatin1String("History"), Qt::CaseInsensitive) != 0, qPrintable(e.name));
            seen << e.name.toLower();
        }
        QCOMPARE(wb.info().sheets[0].name, QStringLiteral("Data"));
        const QString first = wb.info().sheets[1].name, odd = wb.info().sheets[2].name;
        Sheet s;
        QVERIFY(wb.readSheet(0, &s));
        QCOMPARE(formulaOf(s, QStringLiteral("B1")), (first.contains(QLatin1Char(' ')) ? QStringLiteral("'%1'").arg(first) : first) + QStringLiteral("!A1+") + QStringLiteral("%1!A1+Data!A1").arg(odd.contains(QRegularExpression(QStringLiteral("[^A-Za-z0-9_]"))) ? QStringLiteral("'%1'").arg(odd) : odd));
        QVERIFY(!wb.info().definedNames.isEmpty());
        QVERIFY2(wb.info().definedNames[0].ref.contains(first), qPrintable(wb.info().definedNames[0].ref));
        QVERIFY(!wb.changedFeatures().filter(QStringLiteral("sheet name")).isEmpty());
        // and what is written reads back the same
        QByteArray xlsx;
        QVERIFY2(writeBaseXlsx(wb, &xlsx, &why), qPrintable(why));
        Workbook back;
        QVERIFY2(openBytes(xlsx, QStringLiteral("back.xlsx"), &back, &why), qPrintable(why));
        QCOMPARE(back.info().sheets.size(), wb.info().sheets.size());
        for (int i = 0; i < back.info().sheets.size(); ++i) QCOMPARE(back.info().sheets[i].name, wb.info().sheets[i].name);
        // a document whose names are fine says nothing of the kind
        Workbook fine;
        QVERIFY2(openIt(simple(table(QStringLiteral("One"), row(floatCell(1))) + table(QStringLiteral("Two"), row(floatCell(2)))), &fine, &why), qPrintable(why));
        QVERIFY(fine.changedFeatures().isEmpty());
    }

    // Item 17: the messages are in plain words: no XML, DOCTYPE, attribute, or tag in them, and no other program's name.
    void messagesAreInPlainWords()
    {
        QStringList messages;
        auto collect = [&](const QByteArray &ods, const Limits &limits = Limits()) {
            Workbook wb;
            QString why;
            if (!openIt(ods, &wb, &why, limits)) {
                messages << why;
                return;
            }
            Sheet s;
            wb.readSheet(0, &s);
            if (!s.error.isEmpty()) messages << s.error;
        };
        // a DOCTYPE, a piece too long, an element with too many attributes, a nesting too deep, a document that is cut off, bytes that are not UTF-8, and the like
        collect(buildOds("<?xml version=\"1.0\"?><!DOCTYPE x [<!ENTITY a \"b\">]><office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"/>"));
        QString attributes;
        for (int i = 0; i < 1500; ++i) attributes += QStringLiteral(" a%1=\"1\"").arg(i);
        collect(simple(table(QStringLiteral("S"), QStringLiteral("<table:table-row%1/>").arg(attributes))));
        QString deep;
        for (int i = 0; i < 300; ++i) deep += QStringLiteral("<text:span>");
        collect(simple(table(QStringLiteral("S"), row(cellWith(deep)))));
        collect(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1)))).left(300)));
        collect(buildOds(QByteArray("<?xml version=\"1.0\" encoding=\"UTF-16\"?><a/>")));
        collect(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1)))), QString(), QByteArray(), {}, QStringLiteral("application/vnd.sun.xml.calc")));
        collect(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1)))), QString(), QByteArray(), {}, QStringLiteral("application/vnd.oasis.opendocument.text")));
        Limits tight;
        tight.odsContentBytes = 100;
        collect(simple(table(QStringLiteral("S"), row(floatCell(1)))), tight);
        QString markup = QStringLiteral("<table:table-row>") + cell(QStringLiteral("office:value-type=\"string\" office:string-value=\"") + QString(70 << 20, QLatin1Char('b')) + QStringLiteral("\""));
        collect(simple(table(QStringLiteral("S"), markup + QStringLiteral("</table:table-row>"))));
        QVERIFY(messages.size() >= 8);
        for (const QString &m : std::as_const(messages)) {
            for (const char *word : {"XML", "DOCTYPE", "attribute", " tag", "entity", "OpenOffice", "LibreOffice", "StarOffice", "Ex" "cel", "Micro" "soft"})
                QVERIFY2(!m.contains(QLatin1String(word), Qt::CaseSensitive), qPrintable(m + QStringLiteral(" has ") + QLatin1String(word)));
            QVERIFY2(!m.isEmpty() && m.endsWith(QLatin1Char('.')), qPrintable(m));
        }
    }

    // The fonts a document names and the parts of all its number formats together have a total cap, and a document past it is refused in words.
    void fontsAndPartsOfNumberFormatsHaveTotalCaps()
    {
        QString faces;
        for (int i = 0; i < 100; ++i) faces += QStringLiteral("<style:font-face style:name=\"F%1\" svg:font-family=\"Family %1\"/>").arg(i);
        Limits few;
        few.odsFonts = 50;
        Workbook wb;
        QString why;
        QVERIFY(!openIt(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1))), QString(), QString(), faces)), &wb, &why, few));
        QVERIFY2(why.contains(QLatin1String("fonts")), qPrintable(why));
        QVERIFY2(openIt(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1))), QString(), QString(), faces)), &wb, &why), qPrintable(why));
        // many number formats of a few parts each
        QString formats;
        for (int i = 0; i < 20; ++i) {
            formats += QStringLiteral("<number:number-style style:name=\"N%1\"><number:number number:decimal-places=\"2\"/>").arg(i);
            for (int k = 0; k < 20; ++k) formats += QStringLiteral("<number:text>x</number:text>");
            formats += QStringLiteral("</number:number-style>");
        }
        Limits tight;
        tight.odsDataParts = 100;
        Workbook wb2;
        QVERIFY(!openIt(simple(table(QStringLiteral("S"), row(floatCell(1))), formats), &wb2, &why, tight));
        QVERIFY2(why.contains(QLatin1String("number formats")) && why.contains(QLatin1String("more parts")), qPrintable(why));
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), row(floatCell(1))), formats), &wb2, &why), qPrintable(why));
    }

    // What a style or a number format has past the most one keeps (4,000 settings, 1,000 parts) and the merged ranges past the limit of a sheet are left out, and the notice says how many.
    void whatIsLeftOutPastTheLimitsIsCounted()
    {
        // a style with 5,000 settings, in elements of 500 (an element has at most so many attributes)
        QString style = QStringLiteral("<style:style style:name=\"big\" style:family=\"table-cell\">");
        for (int e = 0; e < 10; ++e) {
            style += QStringLiteral("<style:text-properties");
            for (int i = 0; i < 500; ++i) style += QStringLiteral(" fo:x%1_%2=\"1\"").arg(e).arg(i);
            style += QStringLiteral("/>");
        }
        style += QStringLiteral("</style:style>");
        Workbook wb;
        QString why;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), row(floatCell(1))), style), &wb, &why), qPrintable(why));
        QStringList changed = wb.changedFeatures();
        QVERIFY2(!changed.filter(QStringLiteral("setting(s) of cell styles")).isEmpty(), qPrintable(changed.join(QLatin1Char('|'))));
        QVERIFY2(!changed.filter(QStringLiteral("999 setting")).isEmpty(), qPrintable(changed.join(QLatin1Char('|'))));   // (4,001 are kept, of the 5,000)
        // a number format with 1,100 parts
        QString format = QStringLiteral("<number:number-style style:name=\"long\"><number:number number:decimal-places=\"2\"/>");
        for (int k = 0; k < 1100; ++k) format += QStringLiteral("<number:text>x</number:text>");
        format += QStringLiteral("</number:number-style>");
        Workbook wb2;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), row(floatCell(1))), format), &wb2, &why), qPrintable(why));
        changed = wb2.changedFeatures();
        QVERIFY2(!changed.filter(QStringLiteral("part(s) of number formats")).isEmpty(), qPrintable(changed.join(QLatin1Char('|'))));
        // a document that has none of this says nothing
        Workbook fine;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), row(floatCell(1)))), &fine, &why), qPrintable(why));
        QVERIFY(fine.changedFeatures().isEmpty());
        // merged ranges past the limit of a sheet: each is counted, the first ones are kept, and the notice says how many were not
        Limits limits;
        limits.merges = 2;
        const QString merged = row(stringCell(QStringLiteral("m"), QStringLiteral("table:number-columns-spanned=\"2\"")) + QStringLiteral("<table:covered-table-cell/>") + floatCell(1), QStringLiteral("table:number-rows-repeated=\"5\""));
        Workbook many;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(3) + merged)), &many, &why, limits), qPrintable(why));
        Sheet s;
        QVERIFY(many.readSheet(0, &s));
        QCOMPARE(s.mergeCount, qint64(5));
        QCOMPARE(s.merges.size(), 2);
        changed = many.changedFeatures();
        QVERIFY2(!changed.filter(QStringLiteral("3 merged range")).isEmpty(), qPrintable(changed.join(QLatin1Char('|'))));
        // (within the limit nothing is said)
        Workbook within;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(3) + merged)), &within, &why), qPrintable(why));
        QVERIFY(within.readSheet(0, &s));
        QVERIFY(within.changedFeatures().isEmpty());
    }

    // Merges of repeated cells and rows: a cell that spans two columns and is repeated has one merge for each pair, a cell that spans two rows in a row repeated has one for each pair of rows, and a
    // row repeated has one for each copy.
    void mergesOfRepeatedCellsAndRowsAreAllMade()
    {
        Sheet s;
        QString why;
        const QString wide = cell(QStringLiteral("office:value-type=\"string\" table:number-columns-spanned=\"2\" table:number-columns-repeated=\"4\""), QStringLiteral("<text:p>m</text:p>"));
        Workbook cols;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(8) + row(wide))), &cols, &why), qPrintable(why));
        QVERIFY(cols.readSheet(0, &s));
        QCOMPARE(s.merges.size(), 2);
        QCOMPARE(s.merges[0].col1, 0u);
        QCOMPARE(s.merges[0].col2, 1u);
        QCOMPARE(s.merges[1].col1, 2u);
        QCOMPARE(s.merges[1].col2, 3u);
        const QString tall = cell(QStringLiteral("office:value-type=\"string\" table:number-rows-spanned=\"2\""), QStringLiteral("<text:p>m</text:p>"));
        Workbook rows;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(2) + row(tall, QStringLiteral("table:number-rows-repeated=\"4\"")))), &rows, &why), qPrintable(why));
        QVERIFY(rows.readSheet(0, &s));
        QCOMPARE(s.merges.size(), 2);
        QCOMPARE(s.merges[0].row1, 0u);
        QCOMPARE(s.merges[0].row2, 1u);
        QCOMPARE(s.merges[1].row1, 2u);
        QCOMPARE(s.merges[1].row2, 3u);
        // a row repeated, with a cell that spans two columns in it, has one merge in each copy
        Workbook copies;
        QVERIFY2(openIt(simple(table(QStringLiteral("S"), columns(3) + row(cell(QStringLiteral("office:value-type=\"string\" table:number-columns-spanned=\"2\""), QStringLiteral("<text:p>m</text:p>")) + QStringLiteral("<table:covered-table-cell/>"),
                                                                                 QStringLiteral("table:number-rows-repeated=\"4\"")))), &copies, &why), qPrintable(why));
        QVERIFY(copies.readSheet(0, &s));
        QCOMPARE(s.merges.size(), 4);
        QCOMPARE(s.mergeCount, qint64(4));
    }

    // The messages of the reader of .xlsx files are in plain words too: no XML, DOCTYPE, attribute, tag, or entity in them.
    void xlsxMessagesAreInPlainWords()
    {
        QStringList messages;
        auto collect = [&](const QByteArray &sheet, const QByteArray &sharedStrings = QByteArray()) {
            Workbook wb;
            QString why;
            if (!openBytes(buildXlsx({sheet}, sharedStrings), QStringLiteral("plain.xlsx"), &wb, &why)) {
                messages << why;
                return;
            }
            Sheet s;
            wb.readSheet(0, &s);
            if (!s.error.isEmpty()) messages << s.error;
        };
        const QByteArray head = "<?xml version=\"1.0\"?>";
        collect(head + "<!DOCTYPE worksheet [<!ENTITY x \"y\">]><worksheet xmlns=\"" + kMainNs.toUtf8() + "\"><sheetData/></worksheet>");
        QString attributes;
        for (int i = 0; i < 1500; ++i) attributes += QStringLiteral(" a%1=\"1\"").arg(i);
        collect(worksheet(QStringLiteral("<row r=\"1\"%1/>").arg(attributes)));
        QString deep;
        for (int i = 0; i < 300; ++i) deep += QStringLiteral("<x>");
        collect(worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\">%1</c></row>").arg(deep)));
        collect(worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>")).left(150));
        collect(QByteArray("\xFF\xFE<\0?\0x\0m\0l\0", 10));   // (UTF-16: the only form of text the reader takes is UTF-8)
        collect(worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>")), head + "<!DOCTYPE sst [<!ENTITY a \"b\">]><sst xmlns=\"" + kMainNs.toUtf8() + "\"/>");
        QVERIFY2(messages.size() >= 5, qPrintable(messages.join(QLatin1Char('|'))));
        for (const QString &m : std::as_const(messages)) {
            for (const char *word : {"XML", "DOCTYPE", "attribute", " tag", "entity", "UTF-8", "Ex" "cel", "Micro" "soft"})
                QVERIFY2(!m.contains(QLatin1String(word), Qt::CaseSensitive), qPrintable(m + QStringLiteral(" has ") + QLatin1String(word)));
            QVERIFY2(!m.isEmpty() && m.endsWith(QLatin1Char('.')), qPrintable(m));
        }
    }
};

JP_TEST_CLASS(SheetOdsReviewTests)
#include "tst_sheet_odsreview.moc"
