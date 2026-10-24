// Tests of the reader of OpenDocument spreadsheets added after its review: spaces and the like bounded as text is, the parts that hold styles and settings capped and stopped,
// the style budget, the names of functions that differ between the two formats, the null dates, what is skipped and what is changed being said, sheet names the .xlsx form
// does not allow, and plain words in every message. Every document is built in memory by odshelpers.
#include "odshelpers.h"

#include "sheet/odsformula.h"
#include "sheet/xlsxbase.h"

#include <QFile>

namespace {

// The most memory the process has held, in kilobytes (-1 where the system does not say: only Linux does).
qint64 peakKilobytes()
{
    QFile f(QStringLiteral("/proc/self/status"));
    if (!f.open(QIODevice::ReadOnly)) return -1;
    for (const QByteArray &line : f.readAll().split('\n'))
        if (line.startsWith("VmHWM:")) return line.mid(6).trimmed().split(' ').value(0).toLongLong();
    return -1;
}

} // namespace

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
        for (int i = 0; i < 400000; ++i) many += QStringLiteral("<text:s text:c=\"10000\"/>");   // (four billion spaces)
        Workbook wb;
        QString why;
        const QByteArray bomb = simple(table(QStringLiteral("S"), columns(1) + row(floatCell(1)) + row(cellWith(many))));
        many.clear();
        many.squeeze();
        QVERIFY2(openIt(bomb, &wb, &why, limits), qPrintable(why));
        Sheet s;
        const qint64 before = peakKilobytes();
        QVERIFY(!wb.readSheet(0, &s));
        // (the string is never made past the limit: the bomb stands for 8 GB of spaces, and the read takes less than a tenth of one)
        if (before >= 0) QVERIFY2(peakKilobytes() - before < 800 * 1024, qPrintable(QString::number(peakKilobytes() - before) + QStringLiteral(" KB")));
        QVERIFY2(s.error.contains(QLatin1String("longer")), qPrintable(s.error));
        QVERIFY(s.stats.longestString <= limits.stringChars);
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
            for (const char *word : {"XML", "DOCTYPE", "attribute", " tag", "entity", "OpenOffice", "LibreOffice", "StarOffice", "Excel", "Microsoft"})
                QVERIFY2(!m.contains(QLatin1String(word), Qt::CaseSensitive), qPrintable(m + QStringLiteral(" has ") + QLatin1String(word)));
            QVERIFY2(!m.isEmpty() && m.endsWith(QLatin1Char('.')), qPrintable(m));
        }
    }
};

JP_TEST_CLASS(SheetOdsReviewTests)
#include "tst_sheet_odsreview.moc"
