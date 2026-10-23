// Tests of the number formats a workbook carries: the styles part (which format a cell's style shows its value with), and the yardstick that checks the
// number-format engine against the results of TEXT, FIXED, and DOLLAR that a file stored. Every workbook is built here, in memory.
#include "sheethelpers.h"

#include "calc/numfmt.h"
#include "sheet/formatcases.h"
#include "sheet/styles.h"

namespace {

bool readStylesXml(const QByteArray &xml, Styles *out, QString *error = nullptr, const Limits &limits = Limits())
{
    QByteArray copy = xml;
    QBuffer buffer(&copy);
    buffer.open(QIODevice::ReadOnly);
    return readStyles(&buffer, limits, out, error);
}

QByteArray stylesDoc(const QString &inside) { return ("<?xml version=\"1.0\"?>\n<styleSheet xmlns=\"" + kMainNs + "\">" + inside + "</styleSheet>").toUtf8(); }

// A cell with a number.
QString num(const QString &ref, double v) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref).arg(v, 0, 'g', 17); }

// A formula cell whose stored result is the text (t="str").
QString fstr(const QString &ref, const QString &formula, const QString &stored)
{
    return QStringLiteral("<c r=\"%1\" t=\"str\"><f>%2</f><v>%3</v></c>").arg(ref, formula.toHtmlEscaped(), stored.toHtmlEscaped());
}

QString row(int r, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(r).arg(cells); }

QVector<FormatCase> casesOf(const QByteArray &xlsx, QString *why = nullptr)
{
    Package package;
    QVector<FormatCase> none;
    QString err;
    if (!package.readLazy(xlsx, &err)) {
        if (why) *why = err;
        return none;
    }
    Workbook book;
    if (!book.open(package, &err)) {
        if (why) *why = err;
        return none;
    }
    FormatCaseFinder finder(book);
    for (int i = 0; i < book.info().sheets.size(); ++i) {
        Sheet sheet;
        book.readSheet(i, &sheet);
        finder.addSheet(i, sheet);
    }
    return finder.finish();
}

} // namespace

class SheetStylesTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The custom formats by id, and the format id of each cell format; a style names its code, the built-in one of its id, or General.
    void numberFormatsAndCellFormatsAreRead()
    {
        const QByteArray xml = stylesXml(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"0.000\"/>"
                                                        "<numFmt numFmtId=\"165\" formatCode=\"&quot;$&quot;#,##0.00\"/>"
                                                        "<numFmt numFmtId=\"170\" formatCode=\"[$-409]d\\-mmm\"/>"
                                                        "<numFmt numFmtId=\"14\" formatCode=\"dd/mm/yyyy\"/>"),
                                         QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"164\"/><xf numFmtId=\"15\"/><xf numFmtId=\"165\"/><xf numFmtId=\"170\"/>"
                                                        "<xf numFmtId=\"14\"/><xf numFmtId=\"999\"/><xf/>"),
                                         QStringLiteral("<dxfs count=\"1\"><dxf><numFmt numFmtId=\"171\" formatCode=\"0.0\"/></dxf></dxfs>"));
        Styles st;
        QString why;
        QVERIFY2(readStylesXml(xml, &st, &why), qPrintable(why));
        QCOMPARE(st.numFmts.size(), 4);
        QCOMPARE(st.cellXfs.size(), 8);
        QCOMPARE(st.numFmtId(1), 164);
        QCOMPARE(st.numFmtId(2), 15);
        QCOMPARE(st.formatCode(0), QStringLiteral("General"));
        QCOMPARE(st.formatCode(1), QStringLiteral("0.000"));
        QCOMPARE(st.formatCode(2), QStringLiteral("d-mmm-yy"));
        QCOMPARE(st.formatCode(3), QStringLiteral("\"$\"#,##0.00"));
        QCOMPARE(st.formatCode(4), QStringLiteral("[$-409]d\\-mmm"));
        // A format the file defines under a built-in id is the file's.
        QCOMPARE(st.formatCode(5), QStringLiteral("dd/mm/yyyy"));
        // An id with no format, an xf with no id, and a style the file doesn't have, are General.
        QCOMPARE(st.formatCode(6), QStringLiteral("General"));
        QCOMPARE(st.formatCode(7), QStringLiteral("General"));
        QCOMPARE(st.formatCode(8), QStringLiteral("General"));
        QCOMPARE(st.formatCode(100000), QStringLiteral("General"));
        // The formats of the dxfs and the cellStyleXfs are not those of cells.
        QVERIFY(!st.numFmts.contains(171));
        QVERIFY(st.chars > 0);
        // The codes work as formats.
        QCOMPARE(jp::calc::NumberFormat(st.formatCode(3)).format(1234.5).text, QStringLiteral("$1,234.50"));
    }

    // A styles part that isn't one, is damaged, or is past a limit: refused with a message, and what came before stays.
    void aStylesPartPastALimitIsRefused()
    {
        Styles st;
        QString why;
        QVERIFY(!readStylesXml(QByteArray("<?xml version=\"1.0\"?><other/>"), &st, &why));
        QVERIFY(!why.isEmpty());
        why.clear();
        QVERIFY(!readStylesXml(stylesDoc(QStringLiteral("<numFmts><numFmt numFmtId=\"164\" formatCode=\"0\"/>")), &st, &why));
        QVERIFY(!why.isEmpty());
        why.clear();
        QVERIFY(!readStylesXml(QByteArray("<!DOCTYPE x [<!ENTITY a \"b\">]><styleSheet xmlns=\"" + kMainNs.toUtf8() + "\"/>"), &st, &why));
        QVERIFY(!why.isEmpty());
        Limits few;
        few.numFmts = 2;
        why.clear();
        const QString three = QStringLiteral("<numFmts><numFmt numFmtId=\"164\" formatCode=\"0\"/><numFmt numFmtId=\"165\" formatCode=\"0.0\"/><numFmt numFmtId=\"166\" formatCode=\"0.00\"/></numFmts>");
        QVERIFY(!readStylesXml(stylesDoc(three), &st, &why, few));
        QVERIFY(!why.isEmpty());
        QCOMPARE(st.numFmts.size(), 2);
        Limits fewXfs;
        fewXfs.cellXfs = 2;
        why.clear();
        QVERIFY(!readStylesXml(stylesDoc(QStringLiteral("<cellXfs><xf numFmtId=\"1\"/><xf numFmtId=\"2\"/><xf numFmtId=\"3\"/></cellXfs>")), &st, &why, fewXfs));
        QVERIFY(!why.isEmpty());
        QCOMPARE(st.cellXfs.size(), 2);
        Limits fewChars;
        fewChars.numFmtChars = 10;
        why.clear();
        QVERIFY(!readStylesXml(stylesDoc(QStringLiteral("<numFmts><numFmt numFmtId=\"164\" formatCode=\"0.000000\"/><numFmt numFmtId=\"165\" formatCode=\"0.000000\"/></numFmts>")), &st, &why, fewChars));
        QVERIFY(!why.isEmpty());
        // A code longer than a format can be is kept, and is General when shown.
        const QString longCode(5000, QLatin1Char('0'));
        QVERIFY(readStylesXml(stylesXml(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"%1\"/>").arg(longCode), QStringLiteral("<xf numFmtId=\"164\"/>")), &st));
        QCOMPARE(st.numFmts.value(164).size(), 5000);
        QVERIFY(!jp::calc::NumberFormat(st.formatCode(0)).isValid());
    }

    // A workbook opens its styles with the rest; one without a styles part is all General, and a damaged one doesn't stop the workbook opening.
    void aWorkbookOpensItsStyles()
    {
        const QVector<QByteArray> sheets = {worksheet(row(1, QStringLiteral("<c r=\"A1\" s=\"1\"><v>1.5</v></c>")))};
        Package package;
        QString why;
        QVERIFY2(package.readLazy(buildXlsx(sheets, QByteArray(), QString(), QString(), stylesXml(QStringLiteral("<numFmt numFmtId=\"164\" formatCode=\"0.000\"/>"), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"164\"/>"))), &why), qPrintable(why));
        Workbook book;
        QVERIFY2(book.open(package, &why), qPrintable(why));
        QVERIFY(book.stylesError().isEmpty());
        QCOMPARE(book.styles().cellXfs.size(), 2);
        Sheet sheet;
        QVERIFY(book.readSheet(0, &sheet));
        const Cell *c = at(sheet, QStringLiteral("A1"));
        QVERIFY(c);
        QCOMPARE(book.styles().formatCode(c->style), QStringLiteral("0.000"));
        // No styles part.
        Package plain;
        QVERIFY(plain.readLazy(buildXlsx(sheets), &why));
        Workbook b2;
        QVERIFY(b2.open(plain, &why));
        QVERIFY(b2.stylesError().isEmpty());
        QCOMPARE(b2.styles().formatCode(1), QStringLiteral("General"));
        // A damaged one.
        Package damaged;
        QVERIFY(damaged.readLazy(buildXlsx(sheets, QByteArray(), QString(), QString(), QByteArray("<?xml version=\"1.0\"?><styleSheet xmlns=\"x\"><numFmts><numFmt numFmtId=\"164\" formatCode=\"0\"/>")), &why));
        Workbook b3;
        QVERIFY(b3.open(damaged, &why));
        QVERIFY(!b3.stylesError().isEmpty());
        QVERIFY(b3.readSheet(0, &sheet));
    }

    // A styles part damaged partway keeps the formats read before the damage, in the reader and in a workbook.
    void aDamagedStylesPartKeepsWhatCameBefore()
    {
        const QByteArray cut = ("<?xml version=\"1.0\"?>\n<styleSheet xmlns=\"" + kMainNs + "\"><numFmts><numFmt numFmtId=\"164\" formatCode=\"0.000\"/>"
                                "<numFmt numFmtId=\"165\" formatCode=\"0.0%\"/></numFmts><cellXfs><xf numFmtId=\"164\"/><xf numFmtId=\"165\"/><xf numFmtId=\"2\"/>").toUtf8();
        Styles st;
        QString why;
        QVERIFY(!readStylesXml(cut, &st, &why));
        QVERIFY(!why.isEmpty());
        QCOMPARE(st.numFmts.size(), 2);
        QCOMPARE(st.cellXfs.size(), 3);
        QCOMPARE(st.formatCode(0), QStringLiteral("0.000"));
        QCOMPARE(st.formatCode(1), QStringLiteral("0.0%"));
        QCOMPARE(st.formatCode(2), QStringLiteral("0.00"));
        const QVector<QByteArray> sheets = {worksheet(row(1, QStringLiteral("<c r=\"A1\" s=\"1\"><v>0.25</v></c>")))};
        Package package;
        QVERIFY2(package.readLazy(buildXlsx(sheets, QByteArray(), QString(), QString(), cut), &why), qPrintable(why));
        Workbook book;
        QVERIFY2(book.open(package, &why), qPrintable(why));
        QVERIFY(!book.stylesError().isEmpty());
        QCOMPARE(book.styles().formatCode(1), QStringLiteral("0.0%"));
        QCOMPARE(jp::calc::NumberFormat(book.styles().formatCode(1)).format(0.25).text, QStringLiteral("25.0%"));
    }
};

JP_TEST_CLASS(SheetStylesTests)

class SheetFormatCaseTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // TEXT, FIXED, and DOLLAR of a number or a cell, alone or joined with other text, are cases; everything else a formula can be is not.
    void callsOfTheThreeFunctionsAreCases()
    {
        const QString s1 = row(1, num("A1", 1234.5) + fstr("B1", "TEXT(A1,\"#,##0.00\")", "1,234.50")) + row(2, num("A2", 0.456) + fstr("B2", "TEXT(A2,\"0.0%\")", "45.6%"))
                           + row(3, QStringLiteral("<c r=\"A3\" t=\"inlineStr\"><is><t>abc</t></is></c>") + fstr("B3", "TEXT(2.675,\"0.00\")", "2.68"))
                           + row(4, num("A4", 2.675) + fstr("B4", "TEXT($A$4,\"0.00\")", "2.68"))
                           + row(5, fstr("B5", "FIXED(A1,1)", "1,234.5")) + row(6, fstr("B6", "FIXED(A1,0,TRUE)", "1235")) + row(7, fstr("B7", "DOLLAR(-A1)", "($1,234.50)"))
                           + row(8, fstr("B8", "\"Total: \"&TEXT(A1,\"0\")&\" units\"", "Total: 1235 units"))
                           + row(9, fstr("B9", "CONCATENATE(\"a\",TEXT(A2,\"0.00\"),\"b\")", "a0.46b"))
                           + row(10, fstr("B10", "TEXT(Sheet2!A1,\"0.0\")", "7.3"))
                           + row(11, fstr("B11", "TEXT(A1,\"0.0\")", "9.9"))
                           + row(12, QStringLiteral("<c r=\"B12\"><f>SUM(A1,1)</f><v>1235.5</v></c>"))
                           + row(13, fstr("B13", "TEXT(A3,\"0\")", "abc"))
                           + row(14, fstr("B14", "TEXT(A1+1,\"0\")", "1236"))
                           + row(15, fstr("B15", "TEXT(A1,B1)", "1234.5"))
                           + row(16, fstr("B16", "TEXT(A1,\"0\")&B3", "12352.68"))   // (B3 is a cell with text: joined as it is)
                           + row(17, fstr("B17", "TEXT(Z99,\"0\")", "0"))
                           + row(18, fstr("B18", "TEXT('My Sheet'!A1,\"0.00\")", "-3.50"))
                           + row(19, QStringLiteral("<c r=\"B19\"><f>TEXT(A1,\"0\")</f><v>1235</v></c>"))
                           + row(20, num("A20", 12.25) + QStringLiteral("<c r=\"B20\" t=\"str\"><f t=\"shared\" ref=\"B20:B21\" si=\"0\">TEXT(A20,\"0.0\")</f><v>12.3</v></c>"))
                           + row(21, num("A21", 0.05) + QStringLiteral("<c r=\"B21\" t=\"str\"><f t=\"shared\" si=\"0\"/><v>0.1</v></c>"))
                           + row(22, fstr("B22", "TEXT(A1:A2,\"0\")", "1235"))
                           + row(23, fstr("B23", "text( a1 , \"0.0\" ) & \"\"\"\"", "1234.5\""));
        const QString s2 = row(1, num("A1", 7.25)) + row(2, num("A2", -3.5)) + row(3, num("A3", 8));
        // 'My Sheet'!A1 is the third sheet's, which has -3.5 in A1.
        const QString s3 = row(1, num("A1", -3.5));
        const QByteArray xlsx = buildXlsx({worksheet(s1), worksheet(s2), worksheet(s3)}, QByteArray(), QString(), QString(), QByteArray(),
                                          {QStringLiteral("Sheet1"), QStringLiteral("Sheet2"), QStringLiteral("My Sheet")});
        QString why;
        const QVector<FormatCase> cases = casesOf(xlsx, &why);
        QVERIFY2(why.isEmpty(), qPrintable(why));
        QStringList got;
        for (const FormatCase &c : cases)
            got << QStringLiteral("%1|%2|%3|%4|%5|%6|%7|%8").arg(c.sheet, c.cell, c.function, QString::number(c.value, 'g', 15), c.code, c.stored, c.ours, c.match ? QStringLiteral("1") : QStringLiteral("0"));
        const QStringList want = {
            "Sheet1|B1|TEXT|1234.5|#,##0.00|1,234.50|1,234.50|1",
            "Sheet1|B2|TEXT|0.456|0.0%|45.6%|45.6%|1",
            "Sheet1|B3|TEXT|2.675|0.00|2.68|2.68|1",
            "Sheet1|B4|TEXT|2.675|0.00|2.68|2.68|1",
            "Sheet1|B5|FIXED|1234.5|1|1,234.5|1,234.5|1",
            "Sheet1|B6|FIXED|1234.5|0,TRUE|1235|1235|1",
            "Sheet1|B7|DOLLAR|-1234.5|2|($1,234.50)|($1,234.50)|1",
            "Sheet1|B8|TEXT|1234.5|0|Total: 1235 units|Total: 1235 units|1",
            "Sheet1|B9|TEXT|0.456|0.00|a0.46b|a0.46b|1",
            "Sheet1|B10|TEXT|7.25|0.0|7.3|7.3|1",
            "Sheet1|B11|TEXT|1234.5|0.0|9.9|1234.5|0",
            "Sheet1|B16|TEXT|1234.5|0|12352.68|12352.68|1",
            "Sheet1|B18|TEXT|-3.5|0.00|-3.50|-3.50|1",
            "Sheet1|B20|TEXT|12.25|0.0|12.3|12.3|1",
            "Sheet1|B21|TEXT|0.05|0.0|0.1|0.1|1",
            "Sheet1|B23|TEXT|1234.5|0.0|1234.5\"|1234.5\"|1",
        };
        QCOMPARE(got.join(QLatin1Char('\n')), want.join(QLatin1Char('\n')));
    }

    // Other cells joined in (text as it is, a number as General writes it) and names that stand for one cell.
    void cellsAndNamesJoinedWithText()
    {
        const QString s1 = row(1, num("A1", 0.075) + QStringLiteral("<c r=\"C1\" t=\"inlineStr\"><is><t>Rate</t></is></c>") + fstr("B1", "CONCATENATE(C1,\" - \",TEXT(Disc,\"0.0%\"))", "Rate - 7.5%"))
                           + row(2, num("A2", 1234.5678) + fstr("B2", "C1&\": \"&A2&\" \"&TEXT(A2,\"0.0\")", "Rate: 1234.5678 1234.6"))
                           + row(3, fstr("B3", "TEXT(Tax,\"0.00\")", "5.00"))
                           + row(4, fstr("B4", "TEXT(Unknown,\"0\")", "x"))
                           + row(5, fstr("B5", "TEXT(Whole,\"0\")", "x"))
                           + row(6, fstr("B6", "TEXT(A2,\"0\")&A9", "1235"))
                           + row(7, fstr("B7", "B1&\"!\"&TEXT(A2,\"0\")", "Rate - 7.5%!1235"))
                           + row(8, num("A8", 1e-7) + fstr("B8", "TEXT(A8,\"0.0E+0\")&\" \"&A8", "1.0E-7 1E-07"))
                           + row(10, num("A10", 123456789012345678.0) + fstr("B10", "\"v\"&A10&TEXT(1,\"0\")", "v1.23456789012346E+171"))
                           + row(11, fstr("B11", "[1]Sheet1!A1&TEXT(1,\"0\")", "1"));
        const QString s2 = row(1, fstr("C1", "TEXT(Disc,\"0.0\")", "9.0")) + row(2, QStringLiteral("<c r=\"B2\"><v>5</v></c>")) + row(3, QStringLiteral("<c r=\"B3\"><v>9</v></c>"));
        const QString names = QStringLiteral("<definedName name=\"Disc\">Sheet1!$A$1</definedName><definedName name=\"Disc\" localSheetId=\"1\">Sheet2!$B$3</definedName>"
                                             "<definedName name=\"Tax\">Sheet2!$B$2</definedName><definedName name=\"Whole\">Sheet1!$A$1:$A$2</definedName>");
        const QByteArray xlsx = buildXlsx({worksheet(s1), worksheet(s2)}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), names);
        QString why;
        const QVector<FormatCase> cases = casesOf(xlsx, &why);
        QVERIFY2(why.isEmpty(), qPrintable(why));
        QStringList got;
        for (const FormatCase &c : cases)
            got << QStringLiteral("%1|%2|%3|%4|%5|%6|%7|%8").arg(c.sheet, c.cell, c.function, QString::number(c.value, 'g', 15), c.code, c.stored, c.ours, c.match ? QStringLiteral("1") : QStringLiteral("0"));
        const QStringList want = {
            "Sheet1|B1|TEXT|0.075|0.0%|Rate - 7.5%|Rate - 7.5%|1",
            "Sheet1|B2|TEXT|1234.5678|0.0|Rate: 1234.5678 1234.6|Rate: 1234.5678 1234.6|1",
            "Sheet1|B3|TEXT|5|0.00|5.00|5.00|1",
            "Sheet1|B6|TEXT|1234.5678|0|1235|1235|1",
            "Sheet1|B7|TEXT|1234.5678|0|Rate - 7.5%!1235|Rate - 7.5%!1235|1",
            "Sheet1|B8|TEXT|1e-07|0.0E+0|1.0E-7 1E-07|1.0E-7 1E-07|1",
            "Sheet1|B10|TEXT|1|0|v1.23456789012346E+171|v1.23456789012346E+171|1",
            "Sheet2|C1|TEXT|9|0.0|9.0|9.0|1",
        };
        QCOMPARE(got.join(QLatin1Char('\n')), want.join(QLatin1Char('\n')));
    }

    // DATEVALUE of what TEXT writes: the day number is stored, and it is checked against the day the engine's text reads back as.
    void dateValueOfATextIsACase()
    {
        const QString s1 = row(1, num("A1", 45366) + QStringLiteral("<c r=\"B1\"><f>DATEVALUE(TEXT(A1,\"mm/dd/yy\"))</f><v>45366</v></c>"))
                           + row(2, num("A2", 44927.75) + QStringLiteral("<c r=\"B2\"><f>DATEVALUE(TEXT(A2,\"mmm d, yyyy\"))</f><v>44927</v></c>"))
                           + row(3, QStringLiteral("<c r=\"B3\"><f>DATEVALUE(TEXT(A1,\"mm/dd/yy\"))</f><v>45367</v></c>"))
                           + row(4, QStringLiteral("<c r=\"B4\"><f>DATEVALUE(A1)</f><v>45366</v></c>"))
                           + row(5, QStringLiteral("<c r=\"B5\"><f>DATEVALUE(TEXT(A1,\"0.00\"))</f><v>45366</v></c>"))
                           + row(6, QStringLiteral("<c r=\"B6\"><f>DATEVALUE(\"3/15/24\"&amp;TEXT(A1,\"\"))</f><v>45366</v></c>"));
        const QVector<FormatCase> cases = casesOf(buildXlsx({worksheet(s1)}));
        QStringList got;
        for (const FormatCase &c : cases)
            got << QStringLiteral("%1|%2|%3|%4|%5|%6|%7|%8").arg(c.sheet, c.cell, c.function, QString::number(c.value, 'g', 15), c.code, c.stored, c.ours, c.match ? QStringLiteral("1") : QStringLiteral("0"));
        const QStringList want = {
            "Sheet1|B1|DATEVALUE(TEXT)|45366|mm/dd/yy|45366|45366|1",
            "Sheet1|B2|DATEVALUE(TEXT)|44927.75|mmm d, yyyy|44927|44927|1",
            "Sheet1|B3|DATEVALUE(TEXT)|45366|mm/dd/yy|45367|45366|0",
            "Sheet1|B5|DATEVALUE(TEXT)|45366|0.00|45366|#VALUE!|0",
            "Sheet1|B6|DATEVALUE(TEXT)|45366||45366|45366|1",
        };
        QCOMPARE(got.join(QLatin1Char('\n')), want.join(QLatin1Char('\n')));
    }

    // A number joined to text is written as General writes it: a formula that only joins, with a cell that holds a number, is a case of General.
    void numbersJoinedAsGeneralAreCases()
    {
        const QString s1 = row(1, num("A1", 0.1 + 0.2) + fstr("B1", "\"x\"&A1", "x0.3"))
                           + row(2, num("A2", 1.0 / 3) + fstr("B2", "A2&\"\"", "0.333333333333333"))
                           + row(3, num("A3", 1e-7) + fstr("B3", "\"v\"&A3", "v1E-07"))
                           + row(4, num("A4", 1e15) + fstr("B4", "A4&\" \"", "1E+15 "))
                           + row(5, QStringLiteral("<c r=\"A5\" t=\"inlineStr\"><is><t>t</t></is></c>") + fstr("B5", "\"abc\"&A5", "abct"))
                           + row(6, fstr("B6", "A1+1", "1.3"))
                           + row(7, fstr("B7", "CONCATENATE(A1,A2)", "0.30.333333333333333"))
                           + row(8, fstr("B8", "A1&A5", "0.3t"))
                           + row(9, fstr("B9", "A2&\"\"", "0.3333"));
        const QVector<FormatCase> cases = casesOf(buildXlsx({worksheet(s1)}));
        QStringList got;
        for (const FormatCase &c : cases)
            got << QStringLiteral("%1|%2|%3|%4|%5|%6|%7|%8").arg(c.sheet, c.cell, c.function, QString::number(c.value, 'g', 15), c.code, c.stored, c.ours, c.match ? QStringLiteral("1") : QStringLiteral("0"));
        const QStringList want = {
            "Sheet1|B1|&|0.3|General|x0.3|x0.3|1",
            "Sheet1|B2|&|0.333333333333333|General|0.333333333333333|0.333333333333333|1",
            "Sheet1|B3|&|1e-07|General|v1E-07|v1E-07|1",
            "Sheet1|B4|&|1e+15|General|1E+15 |1E+15 |1",
            "Sheet1|B7|&|0.3|General|0.30.333333333333333|0.30.333333333333333|1",
            "Sheet1|B8|&|0.3|General|0.3t|0.3t|1",
            "Sheet1|B9|&|0.333333333333333|General|0.3333|0.333333333333333|0",
        };
        QCOMPARE(got.join(QLatin1Char('\n')), want.join(QLatin1Char('\n')));
    }

    // The checking in a workbook with a 1904 date system, where TEXT of a date goes by that system.
    void aCaseFollowsTheWorkbooksDateSystem()
    {
        const QString s1 = row(1, num("A1", 43904) + fstr("B1", "TEXT(A1,\"yyyy-mm-dd\")", "2024-03-15"));
        const QVector<FormatCase> c1904 = casesOf(buildXlsx({worksheet(s1)}, QByteArray(), QStringLiteral("date1904=\"1\"")));
        QCOMPARE(c1904.size(), 1);
        QVERIFY(c1904[0].match);
        const QVector<FormatCase> c1900 = casesOf(buildXlsx({worksheet(s1)}));
        QCOMPARE(c1900.size(), 1);
        QVERIFY(!c1900[0].match);
        QCOMPARE(c1900[0].ours, QStringLiteral("2020-03-14"));
    }

    // A number in another sheet is looked up once whatever the number of cells that use it; a sheet that isn't there gives no case.
    void cellsOfOtherSheetsAreFollowed()
    {
        QString rows;
        for (int r = 1; r <= 50; ++r) rows += row(r, fstr(QStringLiteral("A%1").arg(r), QStringLiteral("TEXT(Sheet2!B%1,\"0.00\")").arg(r), QStringLiteral("%1.50").arg(r)));
        rows += row(51, fstr("A51", "TEXT(Missing!B1,\"0.00\")", "1.00"));
        QString other;
        for (int r = 1; r <= 50; ++r) other += row(r, QStringLiteral("<c r=\"B%1\"><v>%1.5</v></c>").arg(r));
        const QVector<FormatCase> cases = casesOf(buildXlsx({worksheet(rows), worksheet(other)}));
        QCOMPARE(cases.size(), 50);
        int matched = 0;
        for (const FormatCase &c : cases) matched += c.match ? 1 : 0;
        QCOMPARE(matched, 50);
        QCOMPARE(cases[0].cell, QStringLiteral("A1"));
        QCOMPARE(cases[49].cell, QStringLiteral("A50"));
    }
};

JP_TEST_CLASS(SheetFormatCaseTests)
#include "tst_sheet_formats.moc"
