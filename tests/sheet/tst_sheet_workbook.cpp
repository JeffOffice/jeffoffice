// Tests of the workbook: its part, the two date systems, and opening one from its package with the sheets read one at a time.
#include "sheethelpers.h"

namespace {

QByteArray workbookXml(const QString &body, const QString &rootExtra = QString())
{
    return ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<workbook xmlns=\"" + kMainNs + "\" xmlns:r=\"" + kRelNs + "\"" + rootExtra + ">" + body + "</workbook>").toUtf8();
}

bool readWorkbookXml(const QByteArray &xml, WorkbookInfo *out, QString *error = nullptr, const Limits &limits = Limits())
{
    QByteArray copy = xml;
    QBuffer buffer(&copy);
    buffer.open(QIODevice::ReadOnly);
    return readWorkbookPart(&buffer, limits, out, error);
}

} // namespace

class SheetWorkbookTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // The workbook part: the sheets in order, with their names, ids, states, and the Ids of their relationships; the date system; how it calculates.
    void theWorkbookPartIsRead()
    {
        WorkbookInfo info;
        QString why;
        QVERIFY2(readWorkbookXml(workbookXml("<workbookPr date1904=\"1\" defaultThemeVersion=\"1\"/><sheets>"
                                             "<sheet name=\"Data &amp; Notes\" sheetId=\"3\" r:id=\"rId7\"/>"
                                             "<sheet name=\"Hidden\" sheetId=\"1\" state=\"hidden\" r:id=\"rId2\"/>"
                                             "<sheet name=\"Secret\" sheetId=\"2\" state=\"veryHidden\" r:id=\"rId1\"/>"
                                             "<sheet name=\"Café\" sheetId=\"9\" state=\"visible\" r:id=\"rId9\"/></sheets>"
                                             "<calcPr calcId=\"191029\" calcMode=\"manual\" fullCalcOnLoad=\"1\"/>"), &info, &why),
                 qPrintable(why));
        QCOMPARE(info.sheets.size(), 4);
        QCOMPARE(info.sheets[0].name, QStringLiteral("Data & Notes"));
        QCOMPARE(info.sheets[0].sheetId, 3);
        QCOMPARE(info.sheets[0].relId, QStringLiteral("rId7"));
        QVERIFY(info.sheets[0].state == SheetState::Visible);
        QVERIFY(info.sheets[1].state == SheetState::Hidden);
        QVERIFY(info.sheets[2].state == SheetState::VeryHidden);
        QCOMPARE(info.sheets[3].name, QStringLiteral("Café"));
        QVERIFY(info.dateSystem == DateSystem::Date1904);
        QVERIFY(info.calcMode == CalcMode::Manual);
        QVERIFY(info.fullCalcOnLoad);
        // The defaults: the 1900 date system, calculating automatically; and the other spellings.
        WorkbookInfo plain;
        QVERIFY(readWorkbookXml(workbookXml("<sheets><sheet name=\"S\" sheetId=\"1\" r:id=\"rId1\"/></sheets>"), &plain));
        QVERIFY(plain.dateSystem == DateSystem::Date1900);
        QVERIFY(plain.calcMode == CalcMode::Auto);
        QVERIFY(!plain.fullCalcOnLoad);
        WorkbookInfo other;
        QVERIFY(readWorkbookXml(workbookXml("<workbookPr date1904=\"true\"/><sheets/><calcPr calcMode=\"autoNoTable\"/>"), &other));
        QVERIFY(other.dateSystem == DateSystem::Date1904);
        QVERIFY(other.calcMode == CalcMode::AutoNoTable);
        QCOMPARE(other.sheets.size(), 0);
        WorkbookInfo off;
        QVERIFY(readWorkbookXml(workbookXml("<workbookPr date1904=\"0\"/><sheets/>"), &off));
        QVERIFY(off.dateSystem == DateSystem::Date1900);
        // Strict files: other namespaces, and the relationship attribute in the strict one.
        WorkbookInfo strict;
        QVERIFY(readWorkbookXml("<workbook xmlns=\"http://purl.oclc.org/ooxml/spreadsheetml/main\" xmlns:r=\"http://purl.oclc.org/ooxml/officeDocument/relationships\">"
                                "<workbookPr date1904=\"1\"/><sheets><sheet name=\"S\" sheetId=\"1\" r:id=\"rId5\"/></sheets></workbook>", &strict));
        QCOMPARE(strict.sheets.size(), 1);
        QCOMPARE(strict.sheets[0].relId, QStringLiteral("rId5"));
        QVERIFY(strict.dateSystem == DateSystem::Date1904);
        // Not a workbook, and not XML.
        WorkbookInfo no;
        QVERIFY(!readWorkbookXml("<worksheet><sheets><sheet name=\"S\"/></sheets></worksheet>", &no, &why));
        QVERIFY(!why.isEmpty());
        QVERIFY(!readWorkbookXml("<workbook><sheets>", &no, &why));
        // Defined names with formulas in the workbook are not sheets.
        WorkbookInfo names;
        QVERIFY(readWorkbookXml(workbookXml("<sheets><sheet name=\"S\" sheetId=\"1\" r:id=\"rId1\"/></sheets><definedNames><definedName name=\"x\">S!$A$1</definedName></definedNames>"), &names));
        QCOMPARE(names.sheets.size(), 1);
    }

    // Defined names: the name, what it stands for, the sheet it belongs to by position, and whether it is hidden.
    void definedNamesAreRead()
    {
        WorkbookInfo info;
        QString why;
        const QString sheets = "<sheets><sheet name=\"A\" sheetId=\"1\" r:id=\"rId1\"/><sheet name=\"B\" sheetId=\"2\" r:id=\"rId2\"/></sheets>";
        QVERIFY2(readWorkbookXml(workbookXml(sheets + "<definedNames><definedName name=\"Rate\">Sheet1!$A$1</definedName>"
                                                       "<definedName name=\"Local\" localSheetId=\"1\">B!$C$3</definedName>"
                                                       "<definedName name=\"_xlnm.Print_Area\" localSheetId=\"0\" hidden=\"1\">A!$A$1:$D$9</definedName>"
                                                       "<definedName name=\"Total\" comment=\"x\">SUM(A!$A$1:$A$9)&amp;&quot;t&quot;</definedName>"
                                                       "<definedName name=\"Empty\"/></definedNames>"),
                                 &info, &why),
                 qPrintable(why));
        QCOMPARE(info.sheets.size(), 2);
        QCOMPARE(info.definedNames.size(), 5);
        QCOMPARE(info.definedNames[0].name, QStringLiteral("Rate"));
        QCOMPARE(info.definedNames[0].ref, QStringLiteral("Sheet1!$A$1"));
        QCOMPARE(info.definedNames[0].localSheet, -1);
        QVERIFY(!info.definedNames[0].hidden);
        QCOMPARE(info.definedNames[1].localSheet, 1);
        QCOMPARE(info.definedNames[2].name, QStringLiteral("_xlnm.Print_Area"));
        QCOMPARE(info.definedNames[2].localSheet, 0);
        QVERIFY(info.definedNames[2].hidden);
        QCOMPARE(info.definedNames[3].ref, QStringLiteral("SUM(A!$A$1:$A$9)&\"t\""));
        QCOMPARE(info.definedNames[4].ref, QString());
        // A workbook with none has none; more than the limit, or more characters, is refused with what came before kept.
        QVERIFY(readWorkbookXml(workbookXml(sheets), &info, &why));
        QVERIFY(info.definedNames.isEmpty());
        const QByteArray three = workbookXml(sheets + "<definedNames><definedName name=\"a\">A!$A$1</definedName><definedName name=\"b\">A!$A$2</definedName>"
                                                      "<definedName name=\"c\">A!$A$3</definedName></definedNames>");
        Limits few;
        few.definedNames = 2;
        why.clear();
        QVERIFY(!readWorkbookXml(three, &info, &why, few));
        QVERIFY(!why.isEmpty());
        QCOMPARE(info.definedNames.size(), 2);
        Limits fewChars;
        fewChars.definedNameChars = 10;
        why.clear();
        QVERIFY(!readWorkbookXml(three, &info, &why, fewChars));
        QVERIFY(!why.isEmpty());
        QVERIFY(info.definedNames.size() < 3);
    }

    // The 1900 system counts February 29, 1900, which never was, as day 60; the 1904 system starts at January 1, 1904.
    void bothDateSystemsAreCounted()
    {
        struct Case { int y, m, d; double serial1900, serial1904; };
        const Case cases[] = {
            {1900, 1, 1, 1, -1},
            {1900, 2, 28, 59, -1},
            {1900, 2, 29, 60, -1},
            {1900, 3, 1, 61, -1},
            {1904, 1, 1, 1462, 0},
            {1904, 2, 29, 1462 + 59, 59},
            {1970, 1, 1, 25569, 24107},
            {2000, 2, 29, 36585, 35123},
            {2024, 3, 15, 45366, 43904},
            {9999, 12, 31, 2958465, 2957003},
        };
        for (const Case &c : cases) {
            double s = -9;
            QVERIFY2(dateToSerial(c.y, c.m, c.d, DateSystem::Date1900, &s), qPrintable(QString("%1-%2-%3").arg(c.y).arg(c.m).arg(c.d)));
            QCOMPARE(s, c.serial1900);
            int y = 0, m = 0, d = 0;
            QVERIFY(serialToDate(c.serial1900, DateSystem::Date1900, &y, &m, &d));
            QCOMPARE(QString("%1-%2-%3").arg(y).arg(m).arg(d), QString("%1-%2-%3").arg(c.y).arg(c.m).arg(c.d));
            if (c.serial1904 >= 0) {
                QVERIFY(dateToSerial(c.y, c.m, c.d, DateSystem::Date1904, &s));
                QCOMPARE(s, c.serial1904);
                QVERIFY(serialToDate(c.serial1904, DateSystem::Date1904, &y, &m, &d));
                QCOMPARE(QString("%1-%2-%3").arg(y).arg(m).arg(d), QString("%1-%2-%3").arg(c.y).arg(c.m).arg(c.d));
            } else {
                QVERIFY(!dateToSerial(c.y, c.m, c.d, DateSystem::Date1904, &s));
            }
        }
        double s = 0;
        QVERIFY(!dateToSerial(1899, 12, 31, DateSystem::Date1900, &s));
        QVERIFY(!dateToSerial(1900, 2, 30, DateSystem::Date1900, &s));
        QVERIFY(!dateToSerial(1901, 2, 29, DateSystem::Date1900, &s));
        QVERIFY(!dateToSerial(2024, 13, 1, DateSystem::Date1900, &s));
        QVERIFY(!dateToSerial(2024, 0, 1, DateSystem::Date1900, &s));
        QVERIFY(!dateToSerial(10000, 1, 1, DateSystem::Date1900, &s));
        int y, m, d;
        QVERIFY(!serialToDate(0, DateSystem::Date1900, &y, &m, &d));
        QVERIFY(!serialToDate(2958466, DateSystem::Date1900, &y, &m, &d));
        QVERIFY(!serialToDate(-1, DateSystem::Date1904, &y, &m, &d));
        QVERIFY(serialToDate(45366.75, DateSystem::Date1900, &y, &m, &d));
        QCOMPARE(d, 15);
        // The same text is two different numbers in the two systems; times are fractions of a day.
        QVERIFY(parseIsoDate(QStringLiteral("2024-03-15"), DateSystem::Date1900, &s));
        QCOMPARE(s, 45366.0);
        QVERIFY(parseIsoDate(QStringLiteral("2024-03-15"), DateSystem::Date1904, &s));
        QCOMPARE(s, 43904.0);
        QVERIFY(parseIsoDate(QStringLiteral("2024-03-15T12:00:00"), DateSystem::Date1900, &s));
        QCOMPARE(s, 45366.5);
        QVERIFY(parseIsoDate(QStringLiteral("2024-03-15T18:00:00.000Z"), DateSystem::Date1904, &s));
        QCOMPARE(s, 43904.75);
        QVERIFY(parseIsoDate(QStringLiteral("2024-03-15T06:00:00+02:00"), DateSystem::Date1900, &s));
        QCOMPARE(s, 45366.25);
        QVERIFY(parseIsoDate(QStringLiteral("13:45:30"), DateSystem::Date1900, &s));
        QVERIFY(qAbs(s - 49530.0 / 86400.0) < 1e-12);
        for (const char *bad : {"", "2024", "2024-3-15x", "2024-02-30", "2024-03-15T25:00:00", "tomorrow", "2024-03-15T12:61:00", "2024-03-15T"})
            QVERIFY2(!parseIsoDate(QString::fromLatin1(bad), DateSystem::Date1900, &s), bad);
    }

    // A workbook opened from its package: the sheets are found by their relationships (whatever their part names), read one at a time, in the system the workbook counts dates in.
    void aWorkbookIsOpenedFromItsPackageAndItsSheetsReadOneAtATime()
    {
        const QByteArray sheetOne = worksheet("<row r=\"1\"><c r=\"A1\" t=\"s\"><v>1</v></c><c r=\"B1\" t=\"d\"><v>2024-03-15</v></c><c r=\"C1\"><f>B1+1</f><v>2</v></c></row>");
        const QByteArray sheetTwo = worksheet("<row r=\"1\"><c r=\"A1\"><v>42</v></c></row>");
        for (const char *system : {"", "date1904=\"1\""}) {
            const QByteArray zip = buildXlsx({sheetOne, sheetTwo}, sharedStringsXml({QStringLiteral("<t>zero</t>"), QStringLiteral("<r><t>one</t></r><r><t> two</t></r>")}),
                                             QString::fromLatin1(system), QStringLiteral("calcMode=\"manual\""));
            Package package;
            QString why;
            QVERIFY2(package.readLazy(zip, &why), qPrintable(why));
            Workbook book;
            QVERIFY2(book.open(package, &why), qPrintable(why));
            QCOMPARE(book.info().sheets.size(), 2);
            QCOMPARE(book.info().sheets[0].name, QStringLiteral("Sheet1"));
            QCOMPARE(book.info().sheets[1].part, QStringLiteral("/xl/worksheets/sheet2.xml"));
            QVERIFY(book.info().calcMode == CalcMode::Manual);
            QCOMPARE(book.sharedStrings().strings.size(), 2);
            QVERIFY(book.sharedStringsError().isEmpty());
            const bool is1904 = QLatin1String(system).size() > 0;
            QVERIFY((book.info().dateSystem == DateSystem::Date1904) == is1904);
            Sheet one, two;
            QVERIFY2(book.readSheet(0, &one), qPrintable(one.error));
            QVERIFY2(book.readSheet(1, &two), qPrintable(two.error));
            QCOMPARE(show(one, book.sharedStrings(), "A1"), QStringLiteral("s:one two"));
            QCOMPARE(show(one, book.sharedStrings(), "B1"), is1904 ? QStringLiteral("d:43904") : QStringLiteral("d:45366"));
            QCOMPARE(show(two, book.sharedStrings(), "A1"), QStringLiteral("n:42"));
            QVERIFY(!book.readSheet(2, &two));
            QVERIFY(!two.error.isEmpty());
        }
    }

    // Opening a workbook inflates its workbook part and shared strings, and no sheet: the sheets are inflated as each is read.
    void openingAWorkbookInflatesNoSheet()
    {
        QString rows;
        for (int r = 1; r <= 20000; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
        const QByteArray big = worksheet(rows);
        const QByteArray zip = buildXlsx({worksheet("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), big});
        Package package;
        QVERIFY(package.readLazy(zip));
        Workbook book;
        QString why;
        QVERIFY2(book.open(package, &why), qPrintable(why));
        QVERIFY(book.package().inflatedBytes() < big.size() / 2);
        const qint64 afterOpen = book.package().inflatedBytes();
        Sheet one;
        QVERIFY(book.readSheet(0, &one));
        QVERIFY(book.package().inflatedBytes() - afterOpen < 1000);   // the first sheet is small, and the second is still not inflated
        Sheet two;
        QVERIFY(book.readSheet(1, &two));
        QCOMPARE(two.cells.cellCount(), qint64(20000));
        QCOMPARE(book.package().inflatedBytes() - afterOpen, qint64(one.stats.xmlBytes + two.stats.xmlBytes));
        QCOMPARE(two.stats.xmlBytes, qint64(big.size()));
    }

    // Each sheet has a part of its own: a workbook whose sheets share one (which would have one part read again for each) is refused.
    void sheetsCannotShareAPart()
    {
        PackageWriter w;
        const QString relBase = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        w.addPart(QStringLiteral("/xl/workbook.xml"), workbookXml("<sheets><sheet name=\"A\" sheetId=\"1\" r:id=\"rId1\"/><sheet name=\"B\" sheetId=\"2\" r:id=\"rId2\"/>"
                                                                  "<sheet name=\"C\" sheetId=\"3\" r:id=\"rId3\"/></sheets>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
        w.addRelationship(QStringLiteral("/"), relBase + "officeDocument", QStringLiteral("/xl/workbook.xml"));
        w.addPart(QStringLiteral("/xl/worksheets/sheet1.xml"), worksheet("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), QStringLiteral("application/xml"));
        w.addPart(QStringLiteral("/xl/worksheets/sheet2.xml"), worksheet("<row r=\"1\"><c r=\"A1\"><v>2</v></c></row>"), QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "worksheet", QStringLiteral("/xl/worksheets/sheet1.xml"));
        w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "worksheet", QStringLiteral("/xl/worksheets/sheet2.xml"));
        w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "worksheet", QStringLiteral("/XL/Worksheets/Sheet1.xml"));   // the first again, in other letters
        Package package;
        QVERIFY(package.readLazy(w.finish()));
        Workbook book;
        QString why;
        QVERIFY(!book.open(package, &why));
        QVERIFY2(why.contains(QStringLiteral("share")), qPrintable(why));
        QCOMPARE(book.info().sheets.size(), 0);
    }

    // The XML of the workbook part and of the shared strings is refused as a sheet's is when it has a DOCTYPE (how an entity is defined), when it is
    // not UTF-8, and when an element has more attributes than any file has, whatever its letters.
    void theWorkbookAndTheSharedStringsAreGuardedAsSheetsAre()
    {
        const QByteArray doctype = "<!DOCTYPE x [<!ENTITY a \"b\">]>";
        WorkbookInfo info;
        QString why;
        QVERIFY(!readWorkbookXml("<?xml version=\"1.0\"?>" + doctype + workbookXml("<sheets/>"), &info, &why));
        QVERIFY2(why.contains(QStringLiteral("DOCTYPE")), qPrintable(why));
        QByteArray sst = sharedStringsXml({QStringLiteral("<t>a</t>")});
        SharedStrings strings;
        QByteArray copy = "<?xml version=\"1.0\"?>" + doctype + sst.mid(sst.indexOf("<sst"));
        QBuffer in(&copy);
        in.open(QIODevice::ReadOnly);
        QVERIFY(!readSharedStrings(&in, Limits(), &strings, &why));
        QVERIFY2(why.contains(QStringLiteral("DOCTYPE")), qPrintable(why));
        // UTF-16, in each form the parser would know it by: a byte order mark, a "<" with a zero beside it, and what the declaration says.
        const auto utf16 = [](const QByteArray &utf8, bool bigEndian, bool bom) {
            QStringEncoder encode(bigEndian ? QStringConverter::Utf16BE : QStringConverter::Utf16LE, QStringConverter::Flag::Default);
            QByteArray out = bom ? (bigEndian ? QByteArray("\xfe\xff", 2) : QByteArray("\xff\xfe", 2)) : QByteArray();
            return out + QByteArray(encode(QString::fromUtf8(utf8)));
        };
        const QByteArray plain = workbookXml("<sheets/>");
        const QByteArray body = plain.mid(plain.indexOf("<workbook"));
        QByteArray attributes;
        for (int i = 0; i < 20000; ++i) attributes += QString(" a\u0122%1=\"v\u0122\"").arg(i).toUtf8();
        const QByteArray crowded = "<workbook xmlns=\"" + kMainNs.toUtf8() + "\"><sheets><sheet name=\"S\"" + attributes + "/></sheets></workbook>";
        for (const QByteArray &xml : {utf16(plain, false, true), utf16(plain, true, true), utf16(plain, false, false), utf16(plain, true, false), utf16(crowded, false, true),
                                      QByteArray("\xff\xfe\0\0<\0\0\0", 8), QByteArray("\0\0\xfe\xff\0\0\0<", 8),
                                      "<?xml version=\"1.0\" encoding=\"UTF-16\"?>" + body, "<?xml version=\"1.0\" encoding='utf-32'?>" + body,
                                      "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" + body}) {
            WorkbookInfo w;
            QVERIFY2(!readWorkbookXml(xml, &w, &why), xml.left(24).toHex().constData());
            QVERIFY2(why.contains(QStringLiteral("UTF-8")), qPrintable(why));
            QByteArray text = xml;
            QBuffer buffer(&text);
            buffer.open(QIODevice::ReadOnly);
            SharedStrings s;
            QVERIFY(!readSharedStrings(&buffer, Limits(), &s, &why));
            QVERIFY2(why.contains(QStringLiteral("UTF-8")), qPrintable(why));
        }
        // UTF-8, with or without its mark, and with the name in any letters, is what is read.
        for (const QByteArray &xml : {plain, "\xef\xbb\xbf" + plain, "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>" + body,
                                      "\xef\xbb\xbf<?xml version='1.0' encoding='UTF-8'?>\n" + body, body}) {
            WorkbookInfo w;
            QVERIFY2(readWorkbookXml(xml, &w, &why), qPrintable(why));
        }
    }

    // A part larger than any real part is said to be too large, not damaged.
    void aSheetPartTooLargeIsSaidSo()
    {
        QByteArray zip = buildXlsx({worksheet("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>")});
        for (qsizetype at = zip.indexOf(QByteArray("PK\x01\x02", 4)); at >= 0; at = zip.indexOf(QByteArray("PK\x01\x02", 4), at + 4)) {
            const quint16 nameLength = qFromLittleEndian<quint16>(zip.constData() + at + 28);
            if (QString::fromUtf8(zip.mid(at + 46, nameLength)).endsWith(QLatin1String("sheet1.xml"))) qToLittleEndian<quint32>(0xF0000000u, zip.data() + at + 24);
        }
        Package package;
        QString why;
        QVERIFY2(package.readLazy(zip, &why, qint64(8) << 30), qPrintable(why));
        Workbook book;
        QVERIFY2(book.open(package, &why), qPrintable(why));
        Sheet sheet;
        QVERIFY(!book.readSheet(0, &sheet));
        QVERIFY2(sheet.error.contains(QStringLiteral("too large")) || sheet.error.contains(QStringLiteral("larger")), qPrintable(sheet.error));
        QVERIFY2(!sheet.error.contains(QStringLiteral("damaged")), qPrintable(sheet.error));
    }

    // A sheet whose relationship is missing or is not a worksheet, and a package with no workbook.
    void sheetsThatCantBeReadAreSaid()
    {
        PackageWriter w;
        const QString relBase = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        w.addPart(QStringLiteral("/xl/workbook.xml"), workbookXml("<sheets><sheet name=\"A\" sheetId=\"1\" r:id=\"rId1\"/><sheet name=\"B\" sheetId=\"2\" r:id=\"rId2\"/>"
                                                                  "<sheet name=\"C\" sheetId=\"3\" r:id=\"rId3\"/><sheet name=\"D\" sheetId=\"4\" r:id=\"rIdMissing\"/><sheet name=\"E\" sheetId=\"5\" r:id=\"\"/></sheets>"),
                  QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
        w.addRelationship(QStringLiteral("/"), relBase + "officeDocument", QStringLiteral("/xl/workbook.xml"));
        w.addPart(QStringLiteral("/xl/worksheets/sheet1.xml"), worksheet("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "worksheet", QStringLiteral("/xl/worksheets/sheet1.xml"));
        w.addPart(QStringLiteral("/xl/chartsheets/sheet1.xml"), "<chartsheet/>", QStringLiteral("application/xml"));
        w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "chartsheet", QStringLiteral("/xl/chartsheets/sheet1.xml"));
        w.addRelationship(QStringLiteral("/xl/workbook.xml"), relBase + "worksheet", QStringLiteral("/xl/worksheets/gone.xml"));
        Package package;
        QVERIFY(package.readLazy(w.finish()));
        Workbook book;
        QString why;
        QVERIFY2(book.open(package, &why), qPrintable(why));
        QCOMPARE(book.info().sheets.size(), 5);
        QVERIFY(book.info().sheets[0].kind == SheetKind::Worksheet);
        QVERIFY(book.info().sheets[1].kind == SheetKind::Chartsheet);
        QVERIFY(book.info().sheets[2].part.isEmpty() || !package.hasPart(book.info().sheets[2].part));
        QVERIFY(book.info().sheets[3].part.isEmpty());
        Sheet s;
        QVERIFY(book.readSheet(0, &s));
        // A chart sheet is a sheet with no cells.
        QVERIFY(book.readSheet(1, &s));
        QCOMPARE(s.cells.cellCount(), qint64(0));
        QVERIFY(!book.readSheet(2, &s));
        QVERIFY(!s.error.isEmpty());
        QVERIFY(!book.readSheet(3, &s));
        // A sheet the file lists with no relationship at all is a sheet with nothing in it.
        QVERIFY(book.readSheet(4, &s));
        QVERIFY(s.error.isEmpty());
        QCOMPARE(s.cells.cellCount(), qint64(0));
        QVERIFY(!book.readSheet(-1, &s));
        // No workbook part.
        PackageWriter empty;
        empty.addPart(QStringLiteral("/a.xml"), "<a/>", QStringLiteral("application/xml"));
        Package none;
        QVERIFY(none.readLazy(empty.finish()));
        Workbook nothing;
        QVERIFY(!nothing.open(none, &why));
        QVERIFY(!why.isEmpty());
    }
};

JP_TEST_CLASS(SheetWorkbookTests)
#include "tst_sheet_workbook.moc"
