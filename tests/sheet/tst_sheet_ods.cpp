// Tests of the reader of OpenDocument spreadsheets (.ods and the flat .fods): the sheets, every kind of value, text with its spaces and line breaks and spans, formulas and their
// stored results, data styles as number formats, cell styles, repeated rows and cells, merged cells, sizes, the view, names, what is not read, and what is refused. Every document is
// built in memory by odshelpers.
#include "odshelpers.h"

#include "calc/numfmt.h"
#include "io/zip.h"
#include "sheet/ods.h"
#include "sheet/xlsxbase.h"

class SheetOdsTests : public QObject {
    Q_OBJECT

    static Workbook open(const QByteArray &ods, bool *ok = nullptr, QString *why = nullptr)
    {
        Workbook wb;
        QString message;
        const bool good = openBytes(ods, QStringLiteral("book.ods"), &wb, &message);
        if (ok) *ok = good;
        if (why) *why = message;
        else if (!good && !ok) qWarning("%s", qPrintable(message));
        return wb;
    }
    static Sheet readSheet(const Workbook &wb, int index = 0)
    {
        Sheet s;
        wb.readSheet(index, &s);
        return s;
    }
    static QByteArray simple(const QString &tables, const QString &styles = QString(), const QString &before = QString()) { return buildOds(contentXml(tables, styles, before)); }

private Q_SLOTS:
    // The sheets: names, hidden ones (a table style that does not display them), protection, the color of a tab, the sheet that was open, the kind of the workbook.
    void theSheetsOfADocumentAreListed()
    {
        const QString styles = QStringLiteral("<style:style style:name=\"ta_hidden\" style:family=\"table\"><style:table-properties table:display=\"false\"/></style:style>"
                                              "<style:style style:name=\"ta_tab\" style:family=\"table\"><style:table-properties tableooo:tab-color=\"#ff8000\" style:writing-mode=\"rl-tb\"/></style:style>");
        const QString tables = table(QStringLiteral("First"), row(floatCell(1))) + table(QStringLiteral("Hidden & more"), row(floatCell(2)), QStringLiteral("table:style-name=\"ta_hidden\""))
                               + table(QStringLiteral("Tab"), row(floatCell(3)), QStringLiteral("table:style-name=\"ta_tab\" table:protected=\"true\""));
        const QByteArray settings = settingsXml({}, QStringLiteral("Tab"));
        Workbook wb = open(buildOds(contentXml(tables, styles), QString(), settings));
        QVERIFY(wb.isConverted());
        QVERIFY(!wb.isBinary());
        QCOMPARE(wb.convertedKind(), QStringLiteral(".ods"));
        QCOMPARE(wb.info().sheets.size(), 3);
        QCOMPARE(wb.info().sheets[0].name, QStringLiteral("First"));
        QCOMPARE(wb.info().sheets[1].name, QStringLiteral("Hidden & more"));
        QVERIFY(wb.info().sheets[0].state == SheetState::Visible);
        QVERIFY(wb.info().sheets[1].state == SheetState::Hidden);
        QCOMPARE(wb.info().activeTab, 2);
        const Sheet third = readSheet(wb, 2);
        QVERIFY(third.isProtected);
        QVERIFY(third.view.rightToLeft);
        QVERIFY(third.tabColor.kind == ColorSpec::Kind::Rgb);
        QCOMPARE(third.tabColor.rgb, 0xFF8000u);
        QVERIFY(third.view.tabSelected);
        QVERIFY(!readSheet(wb, 0).view.tabSelected);
        QCOMPARE(shown(wb, readSheet(wb, 1), QStringLiteral("A1")), QStringLiteral("n:2"));
        Sheet none;
        QVERIFY(!wb.readSheet(3, &none));
        QVERIFY(!wb.readSheet(-1, &none));
    }

    // Every value type: numbers, percentages, amounts of money, dates (in the 1900 system), times, true and false, texts (in the cell or in the attribute), an empty text, and errors.
    void cellsOfEveryValueTypeAreRead()
    {
        double serial = 0;
        QVERIFY(jp::calc::dateToSerial(2024, 3, 15, DateSystem::Date1900, &serial));
        double withTime = 0;
        QVERIFY(jp::calc::dateToSerial(2024, 3, 15, DateSystem::Date1900, &withTime));
        withTime += 0.5;
        const QString r = row(floatCell(1.5) + cell(QStringLiteral("office:value-type=\"percentage\" office:value=\"0.25\""), QStringLiteral("<text:p>25%</text:p>"))
                              + cell(QStringLiteral("office:value-type=\"currency\" office:currency=\"USD\" office:value=\"12.5\""), QStringLiteral("<text:p>$12.50</text:p>"))
                              + cell(QStringLiteral("office:value-type=\"date\" office:date-value=\"2024-03-15\""), QStringLiteral("<text:p>03/15/24</text:p>"))
                              + cell(QStringLiteral("office:value-type=\"date\" office:date-value=\"2024-03-15T12:00:00\""))
                              + cell(QStringLiteral("office:value-type=\"time\" office:time-value=\"PT10H30M00S\""))
                              + cell(QStringLiteral("office:value-type=\"time\" office:time-value=\"PT36H00M00S\""))
                              + cell(QStringLiteral("office:value-type=\"boolean\" office:boolean-value=\"true\""))
                              + cell(QStringLiteral("office:value-type=\"boolean\" office:boolean-value=\"false\"")));
        const QString r2 = row(stringCell(QStringLiteral("Hello")) + cell(QStringLiteral("office:value-type=\"string\" office:string-value=\"From the attribute\""), QStringLiteral("<text:p>shown</text:p>"))
                               + cell(QStringLiteral("office:value-type=\"string\""), QStringLiteral("<text:p></text:p>"))
                               + cell(QStringLiteral("office:value-type=\"string\" table:formula=\"of:=1/0\" calcext:value-type=\"error\""), QStringLiteral("<text:p>#DIV/0!</text:p>"))
                               + cell(QStringLiteral("office:value-type=\"string\" table:formula=\"of:=NA()\" calcext:value-type=\"error\""), QStringLiteral("<text:p>#N/A</text:p>"))
                               + cell(QString(), QStringLiteral("<text:p>no type</text:p>")));
        Workbook wb = open(simple(table(QStringLiteral("S"), columns(9) + r + r2)));
        const Sheet s = readSheet(wb);
        QVERIFY(s.error.isEmpty());
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:1.5"));
        QCOMPARE(shown(wb, s, QStringLiteral("B1")), QStringLiteral("n:0.25"));
        QCOMPARE(shown(wb, s, QStringLiteral("C1")), QStringLiteral("n:12.5"));
        QCOMPARE(shown(wb, s, QStringLiteral("D1")), QStringLiteral("n:") + QString::number(serial, 'g', 17));
        QCOMPARE(shown(wb, s, QStringLiteral("E1")), QStringLiteral("n:") + QString::number(withTime, 'g', 17));
        QCOMPARE(shown(wb, s, QStringLiteral("F1")), QStringLiteral("n:0.4375"));
        QCOMPARE(shown(wb, s, QStringLiteral("G1")), QStringLiteral("n:1.5"));
        QCOMPARE(shown(wb, s, QStringLiteral("H1")), QStringLiteral("b:1"));
        QCOMPARE(shown(wb, s, QStringLiteral("I1")), QStringLiteral("b:0"));
        QCOMPARE(shown(wb, s, QStringLiteral("A2")), QStringLiteral("i:Hello"));
        QCOMPARE(shown(wb, s, QStringLiteral("B2")), QStringLiteral("i:From the attribute"));
        QCOMPARE(shown(wb, s, QStringLiteral("C2")), QStringLiteral("none"));   // (an empty text is no value)
        QCOMPARE(shown(wb, s, QStringLiteral("D2")), QStringLiteral("e:#DIV/0!"));
        QCOMPARE(shown(wb, s, QStringLiteral("E2")), QStringLiteral("e:#N/A"));
        QCOMPARE(shown(wb, s, QStringLiteral("F2")), QStringLiteral("i:no type"));
        QCOMPARE(formulaOf(s, QStringLiteral("D2")), QStringLiteral("1/0"));
    }

    // A date in a document counted from 1904 is a day number of that system.
    void theDateSystemIsTheDocuments()
    {
        const QString before = QStringLiteral("<table:calculation-settings><table:null-date table:date-value=\"1904-01-01\"/></table:calculation-settings>");
        Workbook wb = open(simple(table(QStringLiteral("S"), columns(1) + row(cell(QStringLiteral("office:value-type=\"date\" office:date-value=\"1904-01-11\"")))), QString(), before));
        QVERIFY(wb.info().dateSystem == DateSystem::Date1904);
        QCOMPARE(shown(wb, readSheet(wb), QStringLiteral("A1")), QStringLiteral("n:10"));
        Workbook plain = open(simple(table(QStringLiteral("S"), columns(3) + row(cell(QStringLiteral("office:value-type=\"date\" office:date-value=\"1900-03-01\""))
                                                                                  + cell(QStringLiteral("office:value-type=\"date\" office:date-value=\"1899-12-30\""))
                                                                                  + cell(QStringLiteral("office:value-type=\"date\" office:date-value=\"2024-03-15T06:00:00\""))))));
        QVERIFY(plain.info().dateSystem == DateSystem::Date1900);
        // (the numbers count the days from the document's null date, 1899-12-30: the first day of March 1900 is 61, and the null date itself is 0, not a text)
        QCOMPARE(shown(plain, readSheet(plain), QStringLiteral("A1")), QStringLiteral("n:61"));
        QCOMPARE(shown(plain, readSheet(plain), QStringLiteral("B1")), QStringLiteral("n:0"));
        QCOMPARE(shown(plain, readSheet(plain), QStringLiteral("C1")), QStringLiteral("n:45366.25"));
    }

    // Text: spaces, tabs, and line breaks are kept; paragraphs are lines; spans are text; a note or a drawing in the cell is not text.
    void textKeepsItsSpacesBreaksAndParagraphs()
    {
        const QString inside = QStringLiteral("<text:p>a<text:s text:c=\"3\"/>b<text:tab/>c<text:line-break/>d</text:p><text:p>second <text:span text:style-name=\"T1\">bold</text:span> end</text:p>"
                                              "<office:annotation><text:p>a comment</text:p></office:annotation><draw:frame><draw:text-box><text:p>in a frame</text:p></draw:text-box></draw:frame>");
        Workbook wb = open(simple(table(QStringLiteral("S"), columns(1) + row(cell(QStringLiteral("office:value-type=\"string\""), inside)))));
        const Sheet s = readSheet(wb);
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("i:a   b\tc\nd\nsecond bold end"));
    }

    // A span in a style of its own is a run of formatting inside the text (the font it has differs from the cell's); a span in the cell's own look is not.
    void spansWithTheirOwnLookAreRuns()
    {
        const QString styles = QStringLiteral("<style:style style:name=\"T1\" style:family=\"text\"><style:text-properties fo:font-weight=\"bold\" fo:color=\"#ff0000\"/></style:style>"
                                              "<style:style style:name=\"T2\" style:family=\"text\"/>");
        const QString inside = QStringLiteral("<text:p>plain <text:span text:style-name=\"T1\">bold</text:span> and <text:span text:style-name=\"T2\">same</text:span></text:p>");
        Workbook wb = open(simple(table(QStringLiteral("S"), columns(1) + row(cell(QStringLiteral("office:value-type=\"string\""), inside))), styles));
        const Sheet s = readSheet(wb);
        const Cell *c = at(s, QStringLiteral("A1"));
        QVERIFY(c);
        const auto runs = s.richTexts.constFind(c->textIndex());
        QVERIFY(runs != s.richTexts.cend());
        QVERIFY(runs->size() >= 2);
        // "plain " in the cell's font, "bold" in a bold red one
        QCOMPARE(runs.value()[0].start, 0u);
        QCOMPARE(runs.value()[0].font, RichRun::kCellFont);
        QCOMPARE(runs.value()[1].start, 6u);
        QVERIFY(runs.value()[1].font != RichRun::kCellFont);
        QVERIFY(s.richFonts[int(runs.value()[1].font)].bold);
        QCOMPARE(s.richFonts[int(runs.value()[1].font)].color.rgb, 0xFF0000u);
        // a text with spans that look like the cell has no runs
        Workbook same = open(simple(table(QStringLiteral("S"), columns(1) + row(cell(QStringLiteral("office:value-type=\"string\""),
                                                                                       QStringLiteral("<text:p>a <text:span text:style-name=\"T2\">b</text:span></text:p>")))), styles));
        QVERIFY(readSheet(same).richTexts.isEmpty());
    }

    // Formulas: the text is converted, the result the file stored is kept (a number, a text, an error), and a formula that cannot be converted leaves its value as a constant.
    void formulasKeepTheirResults()
    {
        const QString r1 = row(floatCell(1) + floatCell(2) + cell(QStringLiteral("table:formula=\"of:=SUM([.A1:.B1])\" office:value-type=\"float\" office:value=\"3\""), QStringLiteral("<text:p>3</text:p>"))
                               + cell(QStringLiteral("table:formula=\"of:=[.A1]&amp;&quot;x&quot;\" office:value-type=\"string\" office:string-value=\"1x\""), QStringLiteral("<text:p>1x</text:p>"))
                               + cell(QStringLiteral("table:formula=\"of:=[$Other.A1]\" office:value-type=\"float\" office:value=\"9\""))
                               + cell(QStringLiteral("table:formula=\"of:=[ 'file:///x.ods'#$S.A1]\" office:value-type=\"float\" office:value=\"7\""))
                               + cell(QStringLiteral("table:formula=\"of:=COM.MICROSOFT.STDEV.S([.A1:.B1])\" office:value-type=\"float\" office:value=\"0.70710678118654757\"")));
        Workbook wb = open(simple(table(QStringLiteral("S"), columns(7) + r1)));
        const Sheet s = readSheet(wb);
        QCOMPARE(shown(wb, s, QStringLiteral("C1")), QStringLiteral("n:3"));
        QCOMPARE(formulaOf(s, QStringLiteral("C1")), QStringLiteral("SUM(A1:B1)"));
        QCOMPARE(shown(wb, s, QStringLiteral("D1")), QStringLiteral("t:1x"));
        QCOMPARE(formulaOf(s, QStringLiteral("D1")), QStringLiteral("A1&\"x\""));
        QCOMPARE(formulaOf(s, QStringLiteral("E1")), QStringLiteral("Other!A1"));
        QCOMPARE(shown(wb, s, QStringLiteral("F1")), QStringLiteral("n:7"));
        QVERIFY(!at(s, QStringLiteral("F1"))->hasFormula());   // (another file's cells: a constant)
        QCOMPARE(formulaOf(s, QStringLiteral("G1")), QStringLiteral("_xlfn.STDEV.S(A1:B1)"));
        QCOMPARE(s.formulaCells, qint64(4));
        QCOMPARE(s.storedResults, qint64(4));
        QCOMPARE(s.stats.unreadFormulas, qint64(1));
    }

    // An array formula (matrix) holds its range; the other cells of the range hold the values it made.
    void arrayFormulasHoldTheirRange()
    {
        const QString r1 = row(cell(QStringLiteral("table:formula=\"of:={1;2|3;4}\" table:number-matrix-columns-spanned=\"2\" table:number-matrix-rows-spanned=\"2\" office:value-type=\"float\" office:value=\"1\""))
                               + cell(QStringLiteral("office:value-type=\"float\" office:value=\"2\"")).replace(QLatin1String("table-cell"), QLatin1String("covered-table-cell")));
        const QString r2 = row(QStringLiteral("<table:covered-table-cell office:value-type=\"float\" office:value=\"3\"/><table:covered-table-cell office:value-type=\"float\" office:value=\"4\"/>"));
        Workbook wb = open(simple(table(QStringLiteral("S"), columns(2) + r1 + r2)));
        const Sheet s = readSheet(wb);
        const Cell *a1 = at(s, QStringLiteral("A1"));
        QVERIFY(a1 && a1->hasFormula());
        QVERIFY(a1->flags & ArrayFormula);
        const CellRange range = s.formulaRange(*a1);
        QVERIFY(range.valid);
        QCOMPARE(range.row2, 1u);
        QCOMPARE(range.col2, 1u);
        QCOMPARE(s.merges.size(), 0);   // (the covered cells of an array are not a merged range)
        QCOMPARE(shown(wb, s, QStringLiteral("B2")), QStringLiteral("n:4"));
        QCOMPARE(formulaOf(s, QStringLiteral("A1")), QStringLiteral("{1,2;3,4}"));
    }

    // Data styles become number formats: numbers with grouping and decimals, percents, money, dates, times, elapsed times, text, scientific, fractions, and the sections of conditions.
    void dataStylesAreNumberFormats()
    {
        const QString data = QStringLiteral(
            "<number:number-style style:name=\"N2\"><number:number number:decimal-places=\"2\" number:min-integer-digits=\"1\" number:grouping=\"true\"/></number:number-style>"
            "<number:percentage-style style:name=\"P1\"><number:number number:decimal-places=\"1\" number:min-integer-digits=\"1\"/><number:text>%</number:text></number:percentage-style>"
            "<number:currency-style style:name=\"C1\"><number:currency-symbol number:language=\"en\" number:country=\"US\">$</number:currency-symbol><number:number number:decimal-places=\"2\" number:min-integer-digits=\"1\" number:grouping=\"true\"/></number:currency-style>"
            "<number:date-style style:name=\"D1\"><number:day number:style=\"long\"/><number:text>/</number:text><number:month number:style=\"long\"/><number:text>/</number:text><number:year number:style=\"long\"/></number:date-style>"
            "<number:date-style style:name=\"D2\"><number:day-of-week number:style=\"long\"/><number:text>, </number:text><number:month number:textual=\"true\" number:style=\"long\"/><number:text> </number:text><number:day/></number:date-style>"
            "<number:time-style style:name=\"T1\"><number:hours number:style=\"long\"/><number:text>:</number:text><number:minutes number:style=\"long\"/><number:text>:</number:text><number:seconds number:style=\"long\" number:decimal-places=\"1\"/></number:time-style>"
            "<number:time-style style:name=\"T2\" number:truncate-on-overflow=\"false\"><number:hours/><number:text>:</number:text><number:minutes number:style=\"long\"/></number:time-style>"
            "<number:time-style style:name=\"T3\"><number:hours/><number:text>:</number:text><number:minutes number:style=\"long\"/><number:text> </number:text><number:am-pm/></number:time-style>"
            "<number:text-style style:name=\"X1\"><number:text-content/></number:text-style>"
            "<number:number-style style:name=\"S1\"><number:scientific-number number:decimal-places=\"2\" number:min-integer-digits=\"1\" number:min-exponent-digits=\"2\"/></number:number-style>"
            "<number:number-style style:name=\"F1\"><number:fraction number:min-integer-digits=\"1\" number:min-numerator-digits=\"1\" number:min-denominator-digits=\"1\"/></number:number-style>"
            "<number:number-style style:name=\"K1\"><number:number number:decimal-places=\"1\" number:min-integer-digits=\"1\" number:display-factor=\"1000\"/><number:text>K</number:text></number:number-style>"
            "<number:number-style style:name=\"G1\"><number:number number:decimal-places=\"3\" number:min-decimal-places=\"1\" number:min-integer-digits=\"0\"/></number:number-style>"
            "<number:number-style style:name=\"R0\"><number:number number:decimal-places=\"2\" number:min-integer-digits=\"1\" number:grouping=\"true\"/></number:number-style>"
            "<number:number-style style:name=\"R1\"><style:text-properties fo:color=\"#ff0000\"/><number:text>-</number:text><number:number number:decimal-places=\"2\" number:min-integer-digits=\"1\" number:grouping=\"true\"/>"
            "<style:map style:condition=\"value()&gt;=0\" style:apply-style-name=\"R0\"/></number:number-style>"
            // the accounting form: spaces as wide as a parenthesis, a fill, and the sections for positive, negative, zero, and text
            "<number:number-style style:name=\"AP0\"><number:text loext:blank-width-char=\"(\"> $</number:text><number:fill-character> </number:fill-character><number:number number:decimal-places=\"2\" "
            "number:min-decimal-places=\"2\" number:min-integer-digits=\"1\" number:grouping=\"true\"/><number:text loext:blank-width-char=\")\"> </number:text></number:number-style>"
            "<number:number-style style:name=\"AP1\"><number:text loext:blank-width-char=\"(\"> $</number:text><number:fill-character> </number:fill-character><number:text>(</number:text><number:number "
            "number:decimal-places=\"2\" number:min-decimal-places=\"2\" number:min-integer-digits=\"1\" number:grouping=\"true\"/><number:text>)</number:text></number:number-style>"
            "<number:number-style style:name=\"AP2\"><number:text loext:blank-width-char=\"(\"> $</number:text><number:fill-character> </number:fill-character><number:text>-</number:text><number:number "
            "number:decimal-places=\"0\" number:min-integer-digits=\"2\" loext:max-blank-integer-digits=\"2\"/><number:text loext:blank-width-char=\")\"> </number:text></number:number-style>"
            "<number:text-style style:name=\"AC\"><number:text loext:blank-width-char=\"(\"> </number:text><number:text-content/><number:text loext:blank-width-char=\")\"> </number:text>"
            "<style:map style:condition=\"value()&gt;0\" style:apply-style-name=\"AP0\"/><style:map style:condition=\"value()&lt;0\" style:apply-style-name=\"AP1\"/>"
            "<style:map style:condition=\"value()=0\" style:apply-style-name=\"AP2\"/></number:text-style>"
            // a format whose zeros show nothing: "0;-0;"
            "<number:number-style style:name=\"ZP\"><number:number number:decimal-places=\"0\" number:min-integer-digits=\"1\"/></number:number-style>"
            "<number:number-style style:name=\"ZN\"><number:text>-</number:text><number:number number:decimal-places=\"0\" number:min-integer-digits=\"1\"/></number:number-style>"
            "<number:number-style style:name=\"ZZ\"/>"
            "<number:number-style style:name=\"Z1\"><style:map style:condition=\"value()&gt;0\" style:apply-style-name=\"ZP\"/><style:map style:condition=\"value()&lt;0\" style:apply-style-name=\"ZN\"/>"
            "<style:map style:condition=\"value()=0\" style:apply-style-name=\"ZZ\"/></number:number-style>"
            "<number:number-style style:name=\"GN\"><number:number number:min-integer-digits=\"1\"/></number:number-style>"
            "<number:number-style style:name=\"SN\"><number:scientific-number number:decimal-places=\"1\" number:min-integer-digits=\"1\" number:min-exponent-digits=\"2\" number:forced-exponent-sign=\"false\"/></number:number-style>");
        QString styles = data;
        QStringList codes = {QStringLiteral("N2"), QStringLiteral("P1"), QStringLiteral("C1"), QStringLiteral("D1"), QStringLiteral("D2"), QStringLiteral("T1"), QStringLiteral("T2"), QStringLiteral("T3"),
                             QStringLiteral("X1"), QStringLiteral("S1"), QStringLiteral("F1"), QStringLiteral("K1"), QStringLiteral("G1"), QStringLiteral("R1"), QStringLiteral("AC"), QStringLiteral("GN"), QStringLiteral("SN"), QStringLiteral("Z1")};
        QString cells;
        for (int i = 0; i < codes.size(); ++i) {
            styles += QStringLiteral("<style:style style:name=\"ce%1\" style:family=\"table-cell\" style:data-style-name=\"%2\"/>").arg(i).arg(codes[i]);
            cells += floatCell(1234.5, QStringLiteral("table:style-name=\"ce%1\"").arg(i));
        }
        Workbook wb = open(simple(table(QStringLiteral("S"), columns(int(codes.size())) + row(cells)), styles));
        const Sheet s = readSheet(wb);
        const QStringList expected = {QStringLiteral("#,##0.00"), QStringLiteral("0.0%"), QStringLiteral("\"$\"#,##0.00"), QStringLiteral("dd/mm/yyyy"), QStringLiteral("dddd\", \"mmmm d"),
                                      QStringLiteral("hh:mm:ss.0"), QStringLiteral("[h]:mm"), QStringLiteral("h:mm AM/PM"), QStringLiteral("@"), QStringLiteral("0.00E+00"), QStringLiteral("# ?/?"),
                                      QStringLiteral("0.0,\"K\""), QStringLiteral("#.0##"), QStringLiteral("#,##0.00;[Red]-#,##0.00"),
                                      QStringLiteral("_(\"$\"* #,##0.00_);_(\"$\"* \"(\"#,##0.00\")\";_(\"$\"* -??_);_(@_)"), QStringLiteral("General"), QStringLiteral("0.0E-00"), QStringLiteral("0;-0;")};
        for (int i = 0; i < expected.size(); ++i) {
            const Cell *c = at(s, cellName(0, quint32(i)));
            QVERIFY2(c, qPrintable(QString::number(i) + QLatin1Char(' ') + s.error));
            QCOMPARE(wb.styles().formatCode(c->style), expected[i]);
        }
        // and the formats show a value as the code says
        const jp::calc::NumberFormat grouped(wb.styles().formatCode(at(s, QStringLiteral("A1"))->style));
        QCOMPARE(grouped.format(1234.5, DateSystem::Date1900, 0).text, QStringLiteral("1,234.50"));
        const jp::calc::NumberFormat negative(wb.styles().formatCode(at(s, cellName(0, 13))->style));
        QCOMPARE(negative.format(-5, DateSystem::Date1900, 0).text, QStringLiteral("-5.00"));
    }

    // A cell style's font, fill, border, alignment, and protection, with a style that follows another one, and the default style under them all.
    void cellStylesAreFormats()
    {
        const QString common = QStringLiteral("<style:default-style style:family=\"table-cell\"><style:text-properties style:font-name=\"Liberation Sans\" fo:font-size=\"10pt\"/></style:default-style>"
                                              "<style:style style:name=\"Default\" style:family=\"table-cell\"/>"
                                              "<style:style style:name=\"Heading\" style:family=\"table-cell\" style:parent-style-name=\"Default\"><style:text-properties fo:font-weight=\"bold\" fo:font-size=\"14pt\"/></style:style>");
        const QString fonts = QStringLiteral("<style:font-face style:name=\"Liberation Sans\" svg:font-family=\"&apos;Liberation Sans&apos;\"/>");
        const QString styles = QStringLiteral(
            "<style:style style:name=\"ce1\" style:family=\"table-cell\" style:parent-style-name=\"Heading\"><style:table-cell-properties fo:background-color=\"#ffff00\" fo:border=\"1.76pt solid #ff0000\" "
            "fo:border-bottom=\"0.74pt dashed #0000ff\" style:vertical-align=\"middle\" fo:wrap-option=\"wrap\" style:rotation-angle=\"90\" style:cell-protect=\"none\"/>"
            "<style:paragraph-properties fo:text-align=\"center\" fo:margin-left=\"20pt\"/><style:text-properties fo:font-style=\"italic\" style:text-underline-style=\"solid\" "
            "style:text-line-through-style=\"solid\" fo:color=\"#00ff00\"/></style:style>"
            "<style:style style:name=\"ce2\" style:family=\"table-cell\" style:parent-style-name=\"Default\"><style:table-cell-properties style:text-align-source=\"value-type\"/>"
            "<style:paragraph-properties fo:text-align=\"end\"/></style:style>");
        Workbook wb = open(buildOds(contentXml(table(QStringLiteral("S"), columns(3) + row(floatCell(1) + floatCell(2, QStringLiteral("table:style-name=\"ce1\"")) + floatCell(3, QStringLiteral("table:style-name=\"ce2\"")))),
                                               styles, QString(), fonts), common));
        const Sheet s = readSheet(wb);
        const Styles &st = wb.styles();
        const Cell *plain = at(s, QStringLiteral("A1")), *styled = at(s, QStringLiteral("B1")), *general = at(s, QStringLiteral("C1"));
        QVERIFY(plain && styled && general);
        QCOMPARE(plain->style, 0u);
        QCOMPARE(st.font(st.format(0).font).name, QStringLiteral("Liberation Sans"));
        QCOMPARE(st.font(st.format(0).font).size, 10.0);
        QVERIFY(styled->style != 0);
        const CellFormat &f = st.format(styled->style);
        const FontSpec &font = st.font(f.font);
        QVERIFY(font.bold);
        QVERIFY(font.italic);
        QVERIFY(font.strike);
        QVERIFY(font.underline == Underline::Single);
        QCOMPARE(font.size, 14.0);
        QCOMPARE(font.name, QStringLiteral("Liberation Sans"));
        QVERIFY(font.color.kind == ColorSpec::Kind::Rgb);
        QCOMPARE(font.color.rgb, 0x00FF00u);
        const FillSpec &fill = st.fill(f.fill);
        QVERIFY(fill.pattern == PatternKind::Solid);
        QCOMPARE(fill.fg.rgb, 0xFFFF00u);
        const BorderSpec &border = st.border(f.border);
        QVERIFY(border.left.style == BorderStyle::Medium);
        QVERIFY(border.top.style == BorderStyle::Medium);
        QCOMPARE(border.left.color.rgb, 0xFF0000u);
        QVERIFY(border.bottom.style == BorderStyle::Dashed);
        QCOMPARE(border.bottom.color.rgb, 0x0000FFu);
        QVERIFY(f.horizontal == HAlign::Center);
        QVERIFY(f.vertical == VAlign::Center);
        QVERIFY(f.wrap);
        QCOMPARE(int(f.rotation), 90);
        QCOMPARE(int(f.indent), 2);
        QVERIFY(!f.locked);
        QVERIFY(st.format(general->style).horizontal == HAlign::General);   // (the alignment follows the kind of value)
        QVERIFY(st.format(0).locked);
    }

    // A cell with no style of its own has the style of its column; an empty cell in the look of its column is no cell, and one in another look is.
    void columnStylesAreTheDefaultOfTheirCells()
    {
        const QString styles = QStringLiteral("<style:style style:name=\"red\" style:family=\"table-cell\"><style:table-cell-properties fo:background-color=\"#ff0000\"/></style:style>"
                                              "<style:style style:name=\"blue\" style:family=\"table-cell\"><style:table-cell-properties fo:background-color=\"#0000ff\"/></style:style>");
        const QString cols = QStringLiteral("<table:table-column table:default-cell-style-name=\"red\"/><table:table-column/>");
        const QString r1 = row(floatCell(1) + floatCell(2));
        const QString r2 = row(emptyCell() + emptyCell(QStringLiteral("table:style-name=\"blue\"")));
        Workbook wb = open(simple(table(QStringLiteral("S"), cols + r1 + r2), styles));
        const Sheet s = readSheet(wb);
        const Styles &st = wb.styles();
        QCOMPARE(st.fill(st.format(at(s, QStringLiteral("A1"))->style).fill).fg.rgb, 0xFF0000u);   // (column A's look)
        QCOMPARE(at(s, QStringLiteral("B1"))->style, 0u);
        QVERIFY(!at(s, QStringLiteral("A2")));              // (empty, in the look of its column)
        QVERIFY(at(s, QStringLiteral("B2")));               // (empty, in a look of its own)
        QCOMPARE(st.fill(st.format(at(s, QStringLiteral("B2"))->style).fill).fg.rgb, 0x0000FFu);
        QCOMPARE(s.columns.size(), 1);
        QCOMPARE(s.columns[0].first, 0u);
        QCOMPARE(s.columns[0].style, at(s, QStringLiteral("A1"))->style);
    }

    // Repeated cells, rows, and columns: they are made as many times as they are repeated, formulas move with the copy, and a row of cells with nothing is no cell however often it repeats.
    void repeatsAreExpandedWithinTheSheet()
    {
        const QString r1 = row(floatCell(5, QStringLiteral("table:number-columns-repeated=\"3\"")) + cell(QStringLiteral("table:formula=\"of:=[.A1]+1\" office:value-type=\"float\" office:value=\"6\"")));
        const QString rep = row(cell(QStringLiteral("table:formula=\"of:=[.A1]+1\" office:value-type=\"float\" office:value=\"6\"")) + stringCell(QStringLiteral("x")), QStringLiteral("table:number-rows-repeated=\"3\""));
        const QString blank = row(emptyCell(QStringLiteral("table:number-columns-repeated=\"16384\"")), QStringLiteral("table:number-rows-repeated=\"1000000\""));
        const QString last = row(floatCell(9));
        Workbook wb = open(simple(table(QStringLiteral("S"), columns(16384) + r1 + rep + blank + last)));
        const Sheet s = readSheet(wb);
        QVERIFY(s.error.isEmpty());
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:5"));
        QCOMPARE(shown(wb, s, QStringLiteral("C1")), QStringLiteral("n:5"));
        QCOMPARE(shown(wb, s, QStringLiteral("D1")), QStringLiteral("n:6"));
        // rows 2 to 4 are the repeated row: the formula moves down with each copy
        QCOMPARE(formulaOf(s, QStringLiteral("A2")), QStringLiteral("A1+1"));
        QCOMPARE(formulaOf(s, QStringLiteral("A3")), QStringLiteral("A2+1"));
        QCOMPARE(formulaOf(s, QStringLiteral("A4")), QStringLiteral("A3+1"));
        QCOMPARE(shown(wb, s, QStringLiteral("B4")), QStringLiteral("i:x"));
        // the million blank rows only move the next row down
        QCOMPARE(shown(wb, s, QStringLiteral("A1000005")), QStringLiteral("n:9"));
        QCOMPARE(s.cells.cellCount(), qint64(4 + 3 * 2 + 1));
        QCOMPARE(s.extentRows, 1000005u);
    }

    // A cell that spans is a merged range, the cells it covers are not cells of their own (unless they have a look), and a span past the sheet is cut.
    void mergedCellsAreRanges()
    {
        const QString styles = QStringLiteral("<style:style style:name=\"b\" style:family=\"table-cell\"><style:table-cell-properties fo:border=\"0.74pt solid #000000\"/></style:style>");
        const QString r1 = row(stringCell(QStringLiteral("wide"), QStringLiteral("table:number-columns-spanned=\"3\" table:number-rows-spanned=\"2\""))
                               + QStringLiteral("<table:covered-table-cell table:number-columns-repeated=\"2\" table:style-name=\"b\"/>") + floatCell(1));
        const QString r2 = row(QStringLiteral("<table:covered-table-cell table:number-columns-repeated=\"3\"/>") + floatCell(2));
        Workbook wb = open(simple(table(QStringLiteral("S"), columns(4) + r1 + r2), styles));
        const Sheet s = readSheet(wb);
        QCOMPARE(s.merges.size(), 1);
        QCOMPARE(s.merges[0].row1, 0u);
        QCOMPARE(s.merges[0].col1, 0u);
        QCOMPARE(s.merges[0].row2, 1u);
        QCOMPARE(s.merges[0].col2, 2u);
        QCOMPARE(s.mergeCount, qint64(1));
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("i:wide"));
        QCOMPARE(shown(wb, s, QStringLiteral("D1")), QStringLiteral("n:1"));
        QVERIFY(at(s, QStringLiteral("B1")) != nullptr);   // (a covered cell with a border has the look)
        QVERIFY(at(s, QStringLiteral("B1"))->style != 0);
        QVERIFY(!at(s, QStringLiteral("A2")));
        QCOMPARE(shown(wb, s, QStringLiteral("D2")), QStringLiteral("n:2"));
    }

    // Column widths and row heights (in the document's units), hidden and filtered rows and columns, and the sizes most columns and rows share becoming the sheet's own.
    void sizesAndHiddenRowsAndColumns()
    {
        const QString styles = QStringLiteral("<style:style style:name=\"co1\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"2.54cm\"/></style:style>"
                                              "<style:style style:name=\"co2\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"1in\"/></style:style>"
                                              "<style:style style:name=\"co3\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"0.5in\"/></style:style>"
                                              "<style:style style:name=\"ro1\" style:family=\"table-row\"><style:table-row-properties style:row-height=\"0.2in\" style:use-optimal-row-height=\"true\"/></style:style>"
                                              "<style:style style:name=\"ro2\" style:family=\"table-row\"><style:table-row-properties style:row-height=\"0.5in\" style:use-optimal-row-height=\"false\"/></style:style>");
        const QString cols = QStringLiteral("<table:table-column table:style-name=\"co1\" table:number-columns-repeated=\"3\"/><table:table-column table:style-name=\"co3\"/>"
                                            "<table:table-column table:style-name=\"co2\" table:visibility=\"collapse\"/><table:table-column table:style-name=\"co1\"/>");
        const QString rows = row(floatCell(1), QStringLiteral("table:style-name=\"ro1\" table:number-rows-repeated=\"5\"")) + row(floatCell(2), QStringLiteral("table:style-name=\"ro2\""))
                             + row(floatCell(3), QStringLiteral("table:style-name=\"ro1\" table:visibility=\"collapse\"")) + row(floatCell(4), QStringLiteral("table:style-name=\"ro1\" table:visibility=\"filter\""));
        Workbook wb = open(simple(table(QStringLiteral("S"), cols + rows), styles));
        const Sheet s = readSheet(wb);
        QCOMPARE(s.format.defaultRowHeight, 14.4);   // (0.2in, the height most rows have)
        QCOMPARE(s.rowSpecs.size(), 3);
        QCOMPARE(s.rowSpecs[0].row, 5u);
        QCOMPARE(s.rowSpecs[0].height, 36.0);
        QVERIFY(s.rowSpecs[0].customHeight);
        QCOMPARE(s.rowSpecs[1].row, 6u);
        QVERIFY(s.rowSpecs[1].hidden);
        QVERIFY(s.rowSpecs[2].hidden);
        // the width most columns have (1 inch is 96 pixels, which the widest digit of the default font, 7 pixels, counts in characters)
        const double oneInch = 96.0 / 7.0;
        QVERIFY(qAbs(s.format.defaultColWidth - oneInch) < 0.1);
        QCOMPARE(s.columns.size(), 2);   // (the half inch column, and the hidden one with the same width as the sheet's)
        QCOMPARE(s.columns[0].first, 3u);
        QVERIFY(qAbs(s.columns[0].width - oneInch / 2) < 0.1);
        QCOMPARE(s.columns[1].first, 4u);
        QVERIFY(s.columns[1].hidden);
    }

    // How the sheet was left: the cell it was on, the zoom, the frozen panes (the rows and columns that stay), and the grid.
    void theViewComesFromTheSettings()
    {
        QMap<QString, QMap<QString, QString>> sheets;
        sheets[QStringLiteral("S")] = {{QStringLiteral("CursorPositionX"), QStringLiteral("3")}, {QStringLiteral("CursorPositionY"), QStringLiteral("7")}, {QStringLiteral("ZoomValue"), QStringLiteral("150")},
                                       {QStringLiteral("HorizontalSplitMode"), QStringLiteral("2")}, {QStringLiteral("VerticalSplitMode"), QStringLiteral("2")},
                                       {QStringLiteral("HorizontalSplitPosition"), QStringLiteral("2")}, {QStringLiteral("VerticalSplitPosition"), QStringLiteral("3")},
                                       {QStringLiteral("PositionLeft"), QStringLiteral("0")}, {QStringLiteral("PositionRight"), QStringLiteral("2")},
                                       {QStringLiteral("PositionTop"), QStringLiteral("0")}, {QStringLiteral("PositionBottom"), QStringLiteral("10")}};
        sheets[QStringLiteral("T")] = {{QStringLiteral("HorizontalSplitMode"), QStringLiteral("1")}};
        Workbook wb = open(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1))) + table(QStringLiteral("T"), row(floatCell(1)))), QString(),
                                    settingsXml(sheets, QStringLiteral("S"), {{QStringLiteral("ShowGrid"), QStringLiteral("false")}})));
        const Sheet s = readSheet(wb);
        QCOMPARE(s.view.activeCol, 3u);
        QCOMPARE(s.view.activeRow, 7u);
        QCOMPARE(s.view.zoom, 150);
        QVERIFY(s.view.frozen);
        QCOMPARE(s.view.frozenCols, 2u);
        QCOMPARE(s.view.frozenRows, 3u);
        QCOMPARE(s.view.paneCol, 2u);
        QCOMPARE(s.view.paneRow, 10u);
        QVERIFY(!s.view.showGridLines);
        const Sheet t = readSheet(wb, 1);
        QVERIFY(!t.view.frozen);   // (a split that is not fixed is not kept)
        QCOMPARE(t.view.zoom, 100);
    }

    // Named ranges and named expressions of the workbook and of a sheet.
    void namedRangesAreDefinedNames()
    {
        const QString names = QStringLiteral("<table:named-expressions><table:named-range table:name=\"Prices\" table:base-cell-address=\"$S.$A$1\" table:cell-range-address=\"$S.$A$1:.$A$9\"/>"
                                             "<table:named-expression table:name=\"Tax\" table:base-cell-address=\"$S.$A$1\" table:expression=\"[$S.$B$1]*2\"/></table:named-expressions>");
        const QString local = QStringLiteral("<table:named-expressions><table:named-range table:name=\"Local\" table:base-cell-address=\"$S.$A$1\" table:cell-range-address=\"$S.$C$3\"/></table:named-expressions>");
        Workbook wb = open(simple(table(QStringLiteral("S"), row(floatCell(1)) + local) + table(QStringLiteral("T"), row(floatCell(1))) + names));
        QCOMPARE(wb.info().definedNames.size(), 3);
        QCOMPARE(wb.info().definedNames[0].name, QStringLiteral("Local"));
        QCOMPARE(wb.info().definedNames[0].localSheet, 0);
        QCOMPARE(wb.info().definedNames[0].ref, QStringLiteral("S!$C$3"));
        QCOMPARE(wb.info().definedNames[1].name, QStringLiteral("Prices"));
        QCOMPARE(wb.info().definedNames[1].localSheet, -1);
        QCOMPARE(wb.info().definedNames[1].ref, QStringLiteral("S!$A$1:$A$9"));
        QCOMPARE(wb.info().definedNames[2].name, QStringLiteral("Tax"));
        QCOMPARE(wb.info().definedNames[2].ref, QStringLiteral("S!$B$1*2"));
    }

    // What is not read is said: charts, pictures and other drawings, comments, pivot tables, macros, conditional formats, and data validation.
    void whatIsNotReadIsSaid()
    {
        const QString inside = QStringLiteral("<draw:frame><draw:image xlink:href=\"Pictures/a.png\"/></draw:frame><draw:frame><draw:object xlink:href=\"./Object 1\"/></draw:frame>"
                                              "<office:annotation><text:p>note</text:p></office:annotation>");
        const QString tables = table(QStringLiteral("S"), columns(1) + row(cell(QString(), inside)) + QStringLiteral("<table:data-pilot-tables><table:data-pilot-table table:name=\"p\"/></table:data-pilot-tables>")
                                                              + QStringLiteral("<table:content-validations><table:content-validation table:name=\"v\"/></table:content-validations>")
                                                              + QStringLiteral("<calcext:conditional-formats><calcext:conditional-format/></calcext:conditional-formats>"));
        QMap<QString, QByteArray> extra;
        extra[QStringLiteral("Basic/script-lc.xml")] = "<x/>";
        Workbook wb = open(buildOds(contentXml(tables), QString(), QByteArray(), extra));
        const QStringList skipped = wb.skippedFeatures();
        for (const char *what : {"charts", "pictures and other drawing objects", "pivot tables", "comments", "macros", "conditional formats", "data validation"})
            QVERIFY2(skipped.contains(QString::fromLatin1(what)), what);
        Workbook none = open(simple(table(QStringLiteral("S"), row(floatCell(1)))));
        QVERIFY(none.skippedFeatures().isEmpty());
    }

    // The flat XML form is the same document in one piece.
    void aFlatDocumentIsRead()
    {
        const QString styles = QStringLiteral("<style:style style:name=\"ce1\" style:family=\"table-cell\"><style:text-properties fo:font-weight=\"bold\"/></style:style>");
        const QByteArray flat = buildFods(table(QStringLiteral("Flat"), columns(2) + row(floatCell(1, QStringLiteral("table:style-name=\"ce1\"")) + stringCell(QStringLiteral("é")))), styles);
        Workbook wb;
        QString why;
        QVERIFY2(openBytes(flat, QStringLiteral("book.fods"), &wb, &why), qPrintable(why));
        QVERIFY(wb.isConverted());
        QCOMPARE(wb.convertedKind(), QStringLiteral(".fods"));
        const Sheet s = readSheet(wb);
        QCOMPARE(wb.info().sheets[0].name, QStringLiteral("Flat"));
        QCOMPARE(shown(wb, s, QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(shown(wb, s, QStringLiteral("B1")), QStringLiteral("i:é"));
        QVERIFY(wb.styles().font(wb.styles().format(at(s, QStringLiteral("A1"))->style).font).bold);
    }

    // The prefixes in a file are its own choice: the namespaces are what count.
    void anyPrefixIsRead()
    {
        const QByteArray content =
            "<?xml version=\"1.0\"?><o:document-content xmlns:o=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:t=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" "
            "xmlns:x=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\"><o:body><o:spreadsheet><t:table t:name=\"P\"><t:table-column/><t:table-row><t:table-cell o:value-type=\"string\"><x:p>prefixed</x:p></t:table-cell>"
            "</t:table-row></t:table></o:spreadsheet></o:body></o:document-content>";
        Workbook wb = open(buildOds(content));
        QCOMPARE(wb.info().sheets[0].name, QStringLiteral("P"));
        QCOMPARE(shown(wb, readSheet(wb), QStringLiteral("A1")), QStringLiteral("i:prefixed"));
    }

    // A table inside a drawing in a cell (a chart's data) is not a sheet.
    void aTableInsideADrawingIsNotASheet()
    {
        const QString nested = QStringLiteral("<draw:frame><draw:object><table:table table:name=\"chart-data\"><table:table-row><table:table-cell office:value-type=\"float\" office:value=\"99\"/></table:table-row></table:table></draw:object></draw:frame>");
        Workbook wb = open(simple(table(QStringLiteral("Real"), columns(2) + row(cell(QString(), nested) + floatCell(1)))));
        QCOMPARE(wb.info().sheets.size(), 1);
        const Sheet s = readSheet(wb);
        QCOMPARE(shown(wb, s, QStringLiteral("B1")), QStringLiteral("n:1"));
        QCOMPARE(s.cells.cellCount(), qint64(1));
    }

    // The first rows of a sheet are read at once, and the sheet says it was cut short.
    void theFirstScreenStopsEarly()
    {
        QString rows;
        for (int i = 0; i < 50; ++i) rows += row(floatCell(i));
        Workbook wb = open(simple(table(QStringLiteral("S"), columns(1) + rows)));
        Sheet s;
        QVERIFY(wb.readSheet(0, &s, nullptr, 9));
        QVERIFY(s.partial);
        QCOMPARE(s.cells.cellCount(), qint64(10));
        QVERIFY(wb.readSheet(0, &s, nullptr, -1, 4));
        QVERIFY(s.partial);
        QCOMPARE(s.cells.cellCount(), qint64(4));
        QVERIFY(wb.readSheet(0, &s));
        QVERIFY(!s.partial);
        QCOMPARE(s.cells.cellCount(), qint64(50));
    }

    // What is refused is told in words: a password, other kinds of documents, a package with no sheets, bytes that are no package.
    void whatIsRefusedIsExplained()
    {
        const QByteArray encrypted = "<?xml version=\"1.0\"?><manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\"><manifest:file-entry manifest:full-path=\"content.xml\">"
                                     "<manifest:encryption-data manifest:checksum-type=\"SHA1\"/></manifest:file-entry></manifest:manifest>";
        bool ok = true;
        QString why;
        open(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1)))), QString(), QByteArray(), {}, QStringLiteral("application/vnd.oasis.opendocument.spreadsheet"), encrypted), &ok, &why);
        QVERIFY(!ok);
        QVERIFY2(why.contains(QLatin1String("password")), qPrintable(why));
        open(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1)))), QString(), QByteArray(), {}, QStringLiteral("application/vnd.oasis.opendocument.text")), &ok, &why);
        QVERIFY(!ok);
        QVERIFY2(why.contains(QLatin1String("text document")), qPrintable(why));
        open(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1)))), QString(), QByteArray(), {}, QStringLiteral("application/vnd.oasis.opendocument.presentation")), &ok, &why);
        QVERIFY(!ok);
        QVERIFY2(why.contains(QLatin1String("presentation")), qPrintable(why));
        open(buildOds(contentXml(QString())), &ok, &why);   // (no sheets)
        QVERIFY(!ok);
        QVERIFY(!why.isEmpty());
        open(QByteArray("this is not a zip at all"), &ok, &why);
        QVERIFY(!ok);
        // a package of the other kind that happens to have a content.xml is that kind (it has [Content_Types].xml)
        jp::ZipWriter zip;
        zip.add(QStringLiteral("content.xml"), QByteArray("<x/>"));
        zip.add(QStringLiteral("[Content_Types].xml"), QByteArray("<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\"/>"));
        QVERIFY(!looksLikeOpenDocumentZip(zip.finish()));
    }

    // A document read as an ODS is saved as the .xlsx the other readers write: written from the model and read back, its cells, formulas, formats, merged cells, widths, names, and
    // panes are as they were.
    void aConvertedWorkbookIsWrittenAsXlsx()
    {
        const QString styles = QStringLiteral("<number:percentage-style style:name=\"P1\"><number:number number:decimal-places=\"1\" number:min-integer-digits=\"1\"/><number:text>%</number:text></number:percentage-style>"
                                              "<style:style style:name=\"ce1\" style:family=\"table-cell\" style:data-style-name=\"P1\"><style:text-properties fo:font-weight=\"bold\"/></style:style>"
                                              "<style:style style:name=\"co1\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"1.5in\"/></style:style>");
        const QString r1 = row(floatCell(0.25, QStringLiteral("table:style-name=\"ce1\"")) + stringCell(QStringLiteral("text")) + cell(QStringLiteral("table:formula=\"of:=[.A1]*2\" office:value-type=\"float\" office:value=\"0.5\"")));
        const QString r2 = row(stringCell(QStringLiteral("merged"), QStringLiteral("table:number-columns-spanned=\"2\"")) + QStringLiteral("<table:covered-table-cell/>"));
        const QString names = QStringLiteral("<table:named-expressions><table:named-range table:name=\"R\" table:base-cell-address=\"$S.$A$1\" table:cell-range-address=\"$S.$A$1:.$B$1\"/></table:named-expressions>");
        Workbook ods = open(simple(table(QStringLiteral("S"), QStringLiteral("<table:table-column table:style-name=\"co1\" table:number-columns-repeated=\"3\"/>") + r1 + r2) + names, styles));
        QVERIFY(ods.isConverted());
        QByteArray xlsx;
        QString why;
        QVERIFY2(writeBaseXlsx(ods, &xlsx, &why), qPrintable(why));
        Workbook back;
        QVERIFY2(openBytes(xlsx, QStringLiteral("back.xlsx"), &back, &why), qPrintable(why));
        QVERIFY(!back.isConverted());
        const Sheet a = readSheet(ods), b = readSheet(back);
        QCOMPARE(b.cells.cellCount(), a.cells.cellCount());
        QCOMPARE(shown(back, b, QStringLiteral("A1")), QStringLiteral("n:0.25"));
        QCOMPARE(shown(ods, a, QStringLiteral("B1")), QStringLiteral("i:text"));
        QCOMPARE(shown(back, b, QStringLiteral("B1")), QStringLiteral("i:text"));
        QCOMPARE(formulaOf(b, QStringLiteral("C1")), QStringLiteral("A1*2"));
        QCOMPARE(shown(back, b, QStringLiteral("C1")), QStringLiteral("n:0.5"));
        QCOMPARE(back.styles().formatCode(at(b, QStringLiteral("A1"))->style), QStringLiteral("0.0%"));
        QVERIFY(back.styles().font(back.styles().format(at(b, QStringLiteral("A1"))->style).font).bold);
        QCOMPARE(b.merges.size(), 1);
        QVERIFY(qAbs(b.format.defaultColWidth - 144.0 / 7.0) < 0.1);   // (the three columns share a width, which is the sheet's)
        QCOMPARE(back.info().definedNames.size(), 1);
        QCOMPARE(back.info().definedNames[0].ref, QStringLiteral("S!$A$1:$B$1"));
    }
};

JP_TEST_CLASS(SheetOdsTests)
#include "tst_sheet_ods.moc"
