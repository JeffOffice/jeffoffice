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
        const odfcheck::Result r = odfcheck::checkPackage(ods, {QStringLiteral("content.xml"), QStringLiteral("styles.xml"), QStringLiteral("settings.xml"), QStringLiteral("META-INF/manifest.xml")});
        if (!r.ran) QSKIP(qPrintable(QStringLiteral("not checked: ") + r.skip));
        QVERIFY2(r.errors.isEmpty(), qPrintable(QString::fromLatin1(what) + QLatin1String(" (") + QString::number(r.errors.size()) + QLatin1String(" errors): ") + r.errors.mid(0, 6).join(QLatin1Char('\n'))));
        for (const QString &name : r.privateNames)
            QVERIFY2(name.startsWith(QLatin1String("{urn:jeffoffice:odf-extension}")) || name.startsWith(QLatin1String("{urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0}")), qPrintable(QString::fromLatin1(what) + QLatin1String(" has a name of another maker: ") + name));
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

private Q_SLOTS:
    // The check itself must see a fault: a part whose elements are out of the standard's order, and one with an attribute the element does not take, are errors; ours are only listed.
    void theCheckSeesAnElementOutOfOrderAndAStrayAttribute()
    {
        if (!odfcheck::unavailableReason().isEmpty()) QSKIP(qPrintable(QStringLiteral("not checked: ") + odfcheck::unavailableReason()));
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
        if (!r.ran) QSKIP(qPrintable(QStringLiteral("not checked: ") + r.skip));
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
};

JP_TEST_CLASS(SheetOdsValidTests)
#include "tst_sheet_odsvalid.moc"
