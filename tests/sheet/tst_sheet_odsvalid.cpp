// A saved spreadsheet, checked against the standard's schema (version 1.2, third_party/odf_schema; tests/office/odfvalidate.cpp, which loads the machine's libxml2: a machine without it skips these
// tests and says why). The parts the schema covers are checked: content.xml, styles.xml, settings.xml, and the manifest. Names of ours, and of the other suite's extension namespace for cells (calcext, which spreadsheets in this format commonly carry), are taken out first and listed apart.
#include "odshelpers.h"

#include "../office/odfvalidate.h"
#include "io/zip.h"

#include "sheet/oldsave.h"

class SheetOdsValidTests : public QObject {
    Q_OBJECT

    static QByteArray saved(const QByteArray &ods)
    {
        Workbook wb;
        QString why;
        if (!openBytes(ods, QStringLiteral("book.ods"), &wb, &why)) {
            qWarning("%s", qPrintable(why));
            return QByteArray();
        }
        return savedBook(wb);
    }
    static QByteArray savedBook(const Workbook &wb)
    {
        QByteArray out;
        CopyReport report;
        QString why;
        if (!saveOds(wb, &out, &report, &why)) {
            qWarning("%s", qPrintable(why));
            return QByteArray();
        }
        return out;
    }

    // The schema's verdict on a saved package: no error left once the names of ours are out, and what was taken out is only ours.
    static void mustBeValid(const QByteArray &ods, const char *what)
    {
        QVERIFY2(!ods.isEmpty(), what);
        const odfcheck::Result r = odfcheck::checkPackage(ods, {QStringLiteral("content.xml"), QStringLiteral("styles.xml"), QStringLiteral("settings.xml"), QStringLiteral("meta.xml"), QStringLiteral("META-INF/manifest.xml")});
        if (!r.ran) ODF_SKIP_OR_FAIL(r.skip);
        QVERIFY2(r.errors.isEmpty(), qPrintable(QString::fromLatin1(what) + QLatin1String(" (") + QString::number(r.errors.size()) + QLatin1String(" errors): ") + r.errors.mid(0, 6).join(QLatin1Char('\n'))));
        QVERIFY2(r.warnings.isEmpty(), qPrintable(QString::fromLatin1(what) + QLatin1String(" has warnings: ") + r.warnings.mid(0, 3).join(QLatin1Char('\n'))));
        QCOMPARE(r.parts, 5);
        QCOMPARE(r.wrappers, 0);
        QVERIFY2(odfcheck::unlistedPrivateName(r).isEmpty(), qPrintable(QString::fromLatin1(what) + QLatin1String(" has a name of another maker: ") + odfcheck::unlistedPrivateName(r)));
    }

    // A workbook of the XML form that links to the workbooks at these addresses, its cells `rows`.
    static QByteArray linksXlsx(const QString &rows, const QStringList &targets)
    {
        QMap<QString, QByteArray> parts;
        jp::readZip(sheettest::buildXlsx({sheettest::worksheet(rows)}), parts);
        QString references;
        for (int n = 1; n <= int(targets.size()); ++n) {
            const QString part = QStringLiteral("xl/externalLinks/externalLink%1.xml").arg(n);
            parts[part] = "<externalLink xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><externalBook r:id=\"rId1\">"
                          "<sheetNames><sheetName val=\"Sheet1\"/></sheetNames><sheetDataSet><sheetData sheetId=\"0\"><row r=\"1\"><c r=\"A1\"><v>5</v></c></row></sheetData></sheetDataSet></externalBook></externalLink>";
            parts[QStringLiteral("xl/externalLinks/_rels/externalLink%1.xml.rels").arg(n)] =
                QStringLiteral("<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\"><Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/externalLinkPath\" Target=\"%1\" TargetMode=\"External\"/></Relationships>")
                    .arg(targets.at(n - 1)).toUtf8();
            parts[QStringLiteral("[Content_Types].xml")].replace("</Types>", "<Override PartName=\"/" + part.toUtf8() + "\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.externalLink+xml\"/></Types>");
            parts[QStringLiteral("xl/_rels/workbook.xml.rels")].replace("</Relationships>", "<Relationship Id=\"rIdEx" + QByteArray::number(n) + "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/externalLink\" Target=\"externalLinks/externalLink" + QByteArray::number(n) + ".xml\"/></Relationships>");
            references += QStringLiteral("<externalReference r:id=\"rIdEx%1\"/>").arg(n);
        }
        parts[QStringLiteral("xl/workbook.xml")].replace("</sheets>", ("</sheets><externalReferences>" + references + "</externalReferences>").toUtf8());
        jp::ZipWriter z;
        for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
        return z.finish();
    }

    int m_validatedBefore = 0;

private Q_SLOTS:
    void initTestCase() { m_validatedBefore = odfcheck::validatedParts(); }
    void cleanupTestCase() { ODF_ASSERT_VALIDATED(m_validatedBefore); }

    // The check itself must see a fault: a part whose elements are out of the standard's order, and one with an attribute the element does not take, are errors; ours are only listed.
    void theCheckSeesAnElementOutOfOrderAndAStrayAttribute()
    {
        if (!odfcheck::unavailableReason().isEmpty()) ODF_SKIP_OR_FAIL(odfcheck::unavailableReason());
        const QByteArray fine = "<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" "
                                "xmlns:jo=\"urn:jeffoffice:odf-extension\" office:version=\"1.2\"><office:body><office:spreadsheet><table:table table:name=\"S\" jo:more=\"1\"><table:table-column/><table:table-row><table:table-cell/></table:table-row>"
                                "</table:table></office:spreadsheet></office:body></office:document-content>";
        const odfcheck::Result good = odfcheck::checkPart(QStringLiteral("content.xml"), fine);
        QVERIFY(good.ran);
        QVERIFY2(good.errors.isEmpty(), qPrintable(good.errors.join(QLatin1Char('\n'))));
        QCOMPARE(good.privateNames, QStringList{QStringLiteral("{urn:jeffoffice:odf-extension}more")});
        QByteArray order = fine;
        order.replace("<table:table-row>", "<table:table-row><table:table-cell/></table:table-row><table:table-column/><table:table-row>");
        const odfcheck::Result badOrder = odfcheck::checkPart(QStringLiteral("content.xml"), order);
        QVERIFY(!badOrder.errors.isEmpty());
        QVERIFY2(badOrder.errors.join(QLatin1Char('\n')).contains(QLatin1String("content.xml line ")), qPrintable(badOrder.errors.join(QLatin1Char('\n'))));
        QByteArray stray = fine;
        stray.replace("<table:table-cell/>", "<table:table-cell table:bogus=\"1\"/>");
        QVERIFY(!odfcheck::checkPart(QStringLiteral("content.xml"), stray).errors.isEmpty());
    }

    void aPlainWorkbookIsValidAgainstTheSchema()
    {
        const QString rows = row(floatCell(1.5) + stringCell(QStringLiteral("text")) + emptyCell() + floatCell(-3, QStringLiteral("table:formula=\"of:=[.A1]*2\""))) +
                             row(stringCell(QStringLiteral("second row")) + floatCell(42));
        mustBeValid(saved(buildOds(contentXml(table(QStringLiteral("First"), columns(4) + rows) + table(QStringLiteral("Second"), row(floatCell(7)))))), "a plain workbook");
    }

    void aWorkbookWithNotesIsValidAgainstTheSchema()
    {
        const QString note = QStringLiteral("<office:annotation office:display=\"true\" svg:width=\"4cm\" svg:height=\"2cm\" svg:x=\"3cm\" svg:y=\"1cm\"><dc:creator>Dee</dc:creator><dc:date>2026-11-20T10:00:00</dc:date>"
                                            "<text:p>Check this figure.</text:p></office:annotation>");
        const QString quiet = QStringLiteral("<office:annotation><dc:creator>Sam</dc:creator><text:p>Hidden until pointed at.</text:p></office:annotation>");
        const QString rows = row(cell(QStringLiteral("office:value-type=\"float\" office:value=\"1\""), QStringLiteral("<text:p>1</text:p>") + note) + cell(QStringLiteral("office:value-type=\"float\" office:value=\"2\""), QStringLiteral("<text:p>2</text:p>") + quiet));
        mustBeValid(saved(buildOds(contentXml(table(QStringLiteral("S"), columns(2) + rows)))), "a workbook with notes");
    }

    void aWorkbookWithLinksToAnotherWorkbookIsValidAgainstTheSchema()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><f>[1]Sheet1!A1+[2]Sheet1!A1</f><v>10</v></c><c r=\"B1\"><f>SUM('[2]My Sheet'!B2:C3)</f><v>4</v></c></row>");
        Package package;
        Workbook wb;
        QString why;
        QVERIFY2(package.readLazy(linksXlsx(rows, {QStringLiteral("file:///dir/book1.xlsx"), QStringLiteral("file:///dir/book2.xlsx")}), &why) && wb.open(package, &why), qPrintable(why));
        const QByteArray ods = savedBook(wb);
        QVERIFY(ods.size() > 0);
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(ods, parts));
        QVERIFY(parts.value(QStringLiteral("content.xml")).contains("file:///dir/book2.xlsx"));
        mustBeValid(ods, "a workbook with links");
    }

    // A sheet with no cell and no column is still a table with a column (the standard needs one).
    void anEmptySheetIsValidAgainstTheSchema()
    {
        mustBeValid(saved(buildOds(contentXml(table(QStringLiteral("Empty"), QString()) + table(QStringLiteral("Next"), row(floatCell(1)))))), "a workbook with an empty sheet");
    }

    // KNOWN GAP (libs/office/sheet/odsformat.cpp, number:number written with number:min-decimal-places; found Nov 20): that attribute is of the later version of the format, and the version 1.2 schema
    // (which the files say they follow, and is the only one here) does not have it; about 9 in 10 saved workbooks of a real sample carry it. Dropping it changes what other programs show.
    void aNumberFormatIsValidAgainstTheSchema()
    {
        const QString styles = QStringLiteral("<number:number-style style:name=\"N3\"><number:number number:decimal-places=\"3\" number:min-integer-digits=\"1\"/></number:number-style>"
                                              "<style:style style:name=\"ce3\" style:family=\"table-cell\" style:data-style-name=\"N3\"/>");
        const QByteArray ods = saved(buildOds(contentXml(table(QStringLiteral("S"), row(floatCell(1.5, QStringLiteral("table:style-name=\"ce3\"")))), styles)));
        QVERIFY(!ods.isEmpty());
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(ods, parts));
        QVERIFY(parts.value(QStringLiteral("content.xml")).contains("<number:number-style"));
        const odfcheck::Result r = odfcheck::checkPackage(ods, {QStringLiteral("content.xml"), QStringLiteral("styles.xml")});
        if (!r.ran) ODF_SKIP_OR_FAIL(r.skip);
        // only the known gap may be among the errors; a different error fails here
        const QStringList other = odfcheck::errorsWithout(r.errors, {QStringLiteral("min-decimal-places")}, {QStringLiteral("number-style"), QStringLiteral("number"), QStringLiteral("automatic-styles"), QStringLiteral("styles")});
        QVERIFY2(other.isEmpty(), qPrintable(QStringLiteral("an error other than the known gap: ") + other.mid(0, 3).join(QLatin1Char('\n'))));
        // the marker: this fails (as an unexpected pass) once the gap is gone, so that the expectation is removed
        QEXPECT_FAIL("", "known gap: number:min-decimal-places is not in the version 1.2 schema", Continue);
        QVERIFY2(r.errors.isEmpty(), qPrintable(r.errors.mid(0, 3).join(QLatin1Char('\n'))));
    }

    void aWorkbookWithAHiddenColumnIsValidAgainstTheSchema()
    {
        const QString styles = QStringLiteral("<style:style style:name=\"co1\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"1in\"/></style:style>"
                                              "<style:style style:name=\"co0\" style:family=\"table-column\"><style:table-column-properties style:column-width=\"0in\"/></style:style>");
        const QString cols = QStringLiteral("<table:table-column table:style-name=\"co1\" table:number-columns-repeated=\"2\"/><table:table-column table:style-name=\"co0\" table:visibility=\"collapse\"/><table:table-column table:style-name=\"co1\"/>");
        const QByteArray ods = saved(buildOds(contentXml(table(QStringLiteral("S"), cols + row(floatCell(1) + floatCell(2) + floatCell(3) + floatCell(4))), styles)));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(ods, parts));
        QVERIFY(parts.value(QStringLiteral("content.xml")).contains("collapse"));
        mustBeValid(ods, "a workbook with a hidden column");
    }

    // The runs of columns that have no width (hidden or not) or are hidden, as text: from, to, hidden, width, whether the width is custom.
    static QString zeroColumnsOf(const Workbook &wb)
    {
        QString out;
        Sheet sheet;
        if (!wb.readSheet(0, &sheet)) return QStringLiteral("unreadable");
        for (const ColumnSpan &c : sheet.columns)
            if (c.hidden || c.width == 0) out += QStringLiteral("%1-%2 hidden=%3 width=%4 custom=%5;").arg(c.first).arg(c.last).arg(c.hidden).arg(c.width).arg(c.customWidth);
        return out;
    }

    // A column of no width is not a valid length in the standard: it is written with the sheet's width and a mark of ours, and what comes back is the column as it was: hidden or not, width 0, custom.
    void aColumnOfNoWidthIsValidAndComesBackAsItWas()
    {
        const QString cols = QStringLiteral("<cols><col min=\"2\" max=\"2\" width=\"0\" hidden=\"1\" customWidth=\"1\"/><col min=\"4\" max=\"4\" width=\"0\" customWidth=\"1\"/><col min=\"5\" max=\"5\" width=\"12\" customWidth=\"1\"/></cols>");
        const QByteArray xlsx = sheettest::buildXlsx({sheettest::worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"E1\"><v>2</v></c></row>"), QString(), cols)});
        Workbook first, second;
        QString why;
        QVERIFY2(openBytes(xlsx, QStringLiteral("zero.xlsx"), &first, &why), qPrintable(why));
        QCOMPARE(zeroColumnsOf(first), QStringLiteral("1-1 hidden=1 width=0 custom=1;3-3 hidden=0 width=0 custom=1;"));
        const QByteArray ods = savedBook(first);
        mustBeValid(ods, "a workbook with columns of no width");
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(ods, parts));
        QVERIFY(!parts.value(QStringLiteral("content.xml")).contains("column-width=\"0"));
        QVERIFY(parts.value(QStringLiteral("content.xml")).contains("zero-width"));
        QVERIFY2(openBytes(ods, QStringLiteral("zero.ods"), &second, &why), qPrintable(why));
        QCOMPARE(zeroColumnsOf(second), zeroColumnsOf(first));
    }

    // The mark of ours is the only thing that makes the width 0 again: a file with no mark, whose hidden column has the sheet's width, reads as a column with that width.
    void aHiddenColumnWithAWidthHasNoMarkAndKeepsNone()
    {
        const QByteArray xlsx = sheettest::buildXlsx({sheettest::worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), QString(), QStringLiteral("<cols><col min=\"2\" max=\"2\" width=\"9\" hidden=\"1\" customWidth=\"1\"/></cols>"))});
        Workbook first, second;
        QString why;
        QVERIFY2(openBytes(xlsx, QStringLiteral("nine.xlsx"), &first, &why), qPrintable(why));
        const QByteArray ods = savedBook(first);
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(ods, parts));
        QVERIFY(!parts.value(QStringLiteral("content.xml")).contains("zero-width"));
        QVERIFY2(openBytes(ods, QStringLiteral("nine.ods"), &second, &why), qPrintable(why));
        QVERIFY(zeroColumnsOf(second).contains(QLatin1String("hidden=1 width=9")));
    }

    // The check sees what a part of another maker holds: a name of an unlisted namespace is an error (and a listed one only listed), an extension element's standard children are kept and checked in its place, and
    // a namespace that only begins with a listed one is not listed.
    void theCheckKeepsTheChildrenOfAnExtensionAndFailsOnUnlistedNames()
    {
        if (!odfcheck::unavailableReason().isEmpty()) ODF_SKIP_OR_FAIL(odfcheck::unavailableReason());
        const auto part = [](const QByteArray &inside) {
            return "<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" xmlns:jo=\"urn:jeffoffice:odf-extension\" "
                   "xmlns:x=\"urn:other:maker\" xmlns:y=\"urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0.1\" office:version=\"1.2\"><office:body><office:text>" + inside + "</office:text></office:body></office:document-content>";
        };
        const QString name = QStringLiteral("content.xml");
        // a wrapper of ours with a standard child: the child is validated where the wrapper was
        odfcheck::Result r = odfcheck::checkPart(name, part("<jo:wrap><text:p>a</text:p></jo:wrap><text:p>b</text:p>"));
        QVERIFY2(r.errors.isEmpty(), qPrintable(r.errors.join(QLatin1Char('\n'))));
        QCOMPARE(r.wrappers, 1);
        QVERIFY(r.privateNames.contains(QStringLiteral("{urn:jeffoffice:odf-extension}wrap")));
        // ... and a standard child that is not valid there is an error (it was dropped unseen with its wrapper before)
        r = odfcheck::checkPart(name, part("<jo:wrap><text:bogus/></jo:wrap>"));
        QVERIFY(!r.errors.isEmpty());
        // a namespace nobody listed: an error, and an address that only begins like a listed one is not listed either
        r = odfcheck::checkPart(name, part("<x:thing/><text:p>b</text:p>"));
        QVERIFY2(r.errors.join(QLatin1Char('\n')).contains(QLatin1String("urn:other:maker")), qPrintable(r.errors.join(QLatin1Char('\n'))));
        r = odfcheck::checkPart(name, part("<text:p y:a=\"1\">b</text:p>"));
        QVERIFY2(r.errors.join(QLatin1Char('\n')).contains(QLatin1String("calcext:1.0.1")), qPrintable(r.errors.join(QLatin1Char('\n'))));
        QVERIFY(!odfcheck::isExtensionNamespace(QStringLiteral("urn:org:documentfoundation:names:experimental:")));
        QVERIFY(odfcheck::isExtensionNamespace(QStringLiteral("urn:jeffoffice:odf-extension")));
        QVERIFY(!odfcheck::unlistedPrivateName(r).isEmpty());
    }

    // Errors and warnings are told apart; the part count is what was really checked; the switch that makes a missing validator a failure is read from the environment.
    void warningsAreNotErrorsAndTheSwitchIsRead()
    {
        QStringList errors, warnings;
        odfcheck::sortMessage(1, QStringLiteral("w"), &errors, &warnings);
        odfcheck::sortMessage(2, QStringLiteral("e"), &errors, &warnings);
        odfcheck::sortMessage(3, QStringLiteral("f"), &errors, &warnings);
        QCOMPARE(warnings, QStringList{QStringLiteral("w")});
        QCOMPARE(errors, (QStringList{QStringLiteral("e"), QStringLiteral("f")}));
        const QByteArray was = qgetenv("JO_REQUIRE_ODF_VALIDATE");
        qputenv("JO_REQUIRE_ODF_VALIDATE", "1");
        QVERIFY(odfcheck::required());
        qputenv("JO_REQUIRE_ODF_VALIDATE", "0");
        QVERIFY(!odfcheck::required());
        qunsetenv("JO_REQUIRE_ODF_VALIDATE");
        QVERIFY(!odfcheck::required());
        if (!was.isNull()) qputenv("JO_REQUIRE_ODF_VALIDATE", was);
        if (!odfcheck::unavailableReason().isEmpty()) return;
        const int before = odfcheck::validatedParts();
        const odfcheck::Result r = odfcheck::checkPart(QStringLiteral("content.xml"), "<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" office:version=\"1.2\"/>");
        QCOMPARE(r.parts, 1);
        QCOMPARE(odfcheck::validatedParts(), before + 1);
    }
};

JP_TEST_CLASS(SheetOdsValidTests)
#include "tst_sheet_odsvalid.moc"
