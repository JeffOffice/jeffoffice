// Tests of the filter a sheet's file has (autoFilter): the conditions of its columns read whole, the rows it hid told from those a person hid, the filter written again in the place the
// format puts it (and taken away), and the rules of validation the extension list holds (lists from other sheets, and their messages).
#include "sheethelpers.h"

#include "../testrunner.h"
#include "sheet/xlsxsave.h"

#include <QBuffer>

using namespace jp;

namespace {

class BufferSink : public PartSink {
public:
    QByteArray data;
    void write(const char *d, qsizetype n) override { data.append(d, n); }
};

QByteArray patch(const QByteArray &xml, const SheetChanges &changes)
{
    QByteArray copy = xml;
    QBuffer in(&copy);
    in.open(QIODevice::ReadOnly);
    BufferSink out;
    QString why;
    if (!patchSheet(&in, &out, changes, true, &why)) return "FAILED: " + why.toUtf8();
    return out.data;
}

const char *kEverything =
    "<autoFilter ref=\"A1:H20\">"
    "<filterColumn colId=\"0\"><filters blank=\"1\"><filter val=\"apple\"/><filter val=\"x &amp; y\"/><dateGroupItem year=\"2024\" month=\"3\" day=\"5\" dateTimeGrouping=\"day\"/>"
    "<dateGroupItem year=\"2023\" dateTimeGrouping=\"year\"/></filters></filterColumn>"
    "<filterColumn colId=\"1\"><customFilters and=\"1\"><customFilter operator=\"greaterThan\" val=\"5\"/><customFilter operator=\"lessThanOrEqual\" val=\"9\"/></customFilters></filterColumn>"
    "<filterColumn colId=\"2\"><top10 top=\"0\" percent=\"1\" val=\"20\" filterVal=\"3.5\"/></filterColumn>"
    "<filterColumn colId=\"3\"><dynamicFilter type=\"aboveAverage\" val=\"4.25\" maxVal=\"4.25\"/></filterColumn>"
    "<filterColumn colId=\"4\"><colorFilter dxfId=\"1\"/></filterColumn>"
    "<filterColumn colId=\"5\"><colorFilter dxfId=\"0\" cellColor=\"0\"/></filterColumn>"
    "<filterColumn colId=\"6\" hiddenButton=\"1\"/>"
    "<sortState ref=\"A2:H20\"><sortCondition ref=\"B2:B20\" descending=\"1\"/></sortState>"
    "</autoFilter>";

QByteArray sheetWith(const QString &after, const QString &rows = QString(), const QString &head = QString())
{
    return worksheet(rows.isEmpty() ? QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>") : rows, after, head);
}

Sheet readIt(const QByteArray &xml)
{
    Sheet s;
    readSheetXml(xml, &s);
    return s;
}

} // namespace

class SheetFilterTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void everyKindOfConditionIsReadWhole()
    {
        const Sheet s = readIt(sheetWith(QString::fromLatin1(kEverything)));
        const AutoFilter &f = s.autoFilter;
        QVERIFY(f.valid());
        QCOMPARE(rangeName(f.ref), QStringLiteral("A1:H20"));
        QCOMPARE(f.columns.size(), 7);
        const FilterColumn &values = f.columns[0];
        QCOMPARE(int(values.kind), int(FilterColumn::Kind::Values));
        QVERIFY(values.blank);
        QCOMPARE(values.values, (QStringList{"apple", "x & y"}));
        QCOMPARE(values.dates.size(), 2);
        QCOMPARE(int(values.dates[0].unit), int(FilterColumn::DateGroup::Unit::Day));
        QCOMPARE(values.dates[0].year * 10000 + values.dates[0].month * 100 + values.dates[0].day, 20240305);
        QCOMPARE(int(values.dates[1].unit), int(FilterColumn::DateGroup::Unit::Year));
        const FilterColumn &custom = f.columns[1];
        QCOMPARE(int(custom.kind), int(FilterColumn::Kind::Custom));
        QVERIFY(custom.both);
        QCOMPARE(custom.conditions.size(), 2);
        QCOMPARE(int(custom.conditions[0].op), int(FilterColumn::Op::Greater));
        QCOMPARE(custom.conditions[1].value, QStringLiteral("9"));
        QCOMPARE(int(custom.conditions[1].op), int(FilterColumn::Op::LessEqual));
        const FilterColumn &top = f.columns[2];
        QCOMPARE(int(top.kind), int(FilterColumn::Kind::Top10));
        QVERIFY(!top.top && top.percent);
        QCOMPARE(top.count, 20.0);
        QVERIFY(top.hasLimit);
        QCOMPARE(top.limit, 3.5);
        const FilterColumn &dynamic = f.columns[3];
        QCOMPARE(int(dynamic.kind), int(FilterColumn::Kind::Dynamic));
        QCOMPARE(dynamic.dynamic, QStringLiteral("aboveAverage"));
        QCOMPARE(dynamic.dynVal, 4.25);
        QCOMPARE(int(f.columns[4].kind), int(FilterColumn::Kind::Color));
        QCOMPARE(f.columns[4].dxf, 1);
        QVERIFY(!f.columns[4].fontColor);
        QVERIFY(f.columns[5].fontColor);
        QCOMPARE(int(f.columns[6].kind), int(FilterColumn::Kind::None));
        QVERIFY(f.columns[6].hideButton);
        QVERIFY(f.sortState.contains(QLatin1String("descending=\"1\"")));
        QVERIFY(f.active());
        QCOMPARE(f.conditionColumns(), (QVector<quint32>{0, 1, 2, 3, 4, 5}));
        QVERIFY(f.columnAt(2) && !f.columnAt(7));
    }

    void rowsAFilterHidIsToldFromRowsAPersonHid()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\" hidden=\"1\"><c r=\"A2\"><v>1</v></c></row><row r=\"3\"><c r=\"A3\"><v>1</v></c></row>"
                                            "<row r=\"4\" hidden=\"1\"><c r=\"A4\"><v>1</v></c></row><row r=\"9\" hidden=\"1\"><c r=\"A9\"><v>1</v></c></row>");
        // a filter with a condition covers rows 2 to 5: the rows it hides are its own
        Sheet s = readIt(sheetWith(QStringLiteral("<autoFilter ref=\"A1:A5\"><filterColumn colId=\"0\"><filters><filter val=\"1\"/></filters></filterColumn></autoFilter>"), rows));
        QCOMPARE(s.rowSpecs.size(), 3);
        QVERIFY(s.rowSpecs[0].hidden && s.rowSpecs[0].filtered && !s.rowSpecs[0].hand);          // row 2
        QVERIFY(s.rowSpecs[1].hidden && s.rowSpecs[1].filtered && !s.rowSpecs[1].hand);          // row 4
        QVERIFY(s.rowSpecs[2].hidden && !s.rowSpecs[2].filtered && s.rowSpecs[2].hand);          // row 9 is outside it
        // a filter with buttons only: every hidden row was hidden by hand
        s = readIt(sheetWith(QStringLiteral("<autoFilter ref=\"A1:A5\"/>"), rows));
        for (const RowSpec &spec : s.rowSpecs) QVERIFY(spec.hand && !spec.filtered);
        // no filter at all
        s = readIt(sheetWith(QString(), rows));
        QVERIFY(!s.autoFilter.valid());
        for (const RowSpec &spec : s.rowSpecs) QVERIFY(spec.hand && !spec.filtered);
    }

    void aFilterIsWrittenAgainAsItWasRead()
    {
        const QByteArray xml = sheetWith(QString::fromLatin1(kEverything));
        const Sheet s = readIt(xml);
        SheetChanges c;
        c.replaceAutoFilter = true;
        c.autoFilter = s.autoFilter;
        const QByteArray out = patch(xml, c);
        QVERIFY2(!out.startsWith("FAILED"), out.constData());
        const Sheet again = readIt(out);
        QVERIFY(again.autoFilter == s.autoFilter);
        QCOMPARE(out.count("<autoFilter"), 1);
        QVERIFY(out.contains("x &amp; y"));
    }

    void aFilterIsPutWhereTheFormatHasItAndTakenAwayWhenThereIsNone()
    {
        const QString after = QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"C1:D1\"/></mergeCells><pageMargins left=\"1\" right=\"1\" top=\"1\" bottom=\"1\" header=\"0\" footer=\"0\"/>");
        const QByteArray xml = sheetWith(after);
        AutoFilter f;
        f.ref = CellRange{0, 0, 9, 2, true};
        FilterColumn col;
        col.column = 1;
        col.kind = FilterColumn::Kind::Values;
        col.values = {QStringLiteral("a"), QStringLiteral("b")};
        f.columns.append(col);
        SheetChanges c;
        c.replaceAutoFilter = true;
        c.autoFilter = f;
        c.filterMode = 1;
        QByteArray out = patch(xml, c);
        QVERIFY2(!out.startsWith("FAILED"), out.constData());
        QVERIFY(out.indexOf("</sheetData>") < out.indexOf("<autoFilter"));
        QVERIFY(out.indexOf("<autoFilter") < out.indexOf("<mergeCells"));          // (the format puts it before the merged cells)
        QVERIFY(out.contains("<sheetPr filterMode=\"1\"/>"));
        const Sheet s = readIt(out);
        QVERIFY(s.autoFilter == f);
        QCOMPARE(s.merges.size(), 1);
        // changed in place
        f.columns[0].values = {QStringLiteral("c")};
        c.autoFilter = f;
        QByteArray second = patch(out, c);
        QCOMPARE(second.count("<autoFilter"), 1);
        QVERIFY(readIt(second).autoFilter == f);
        // taken away
        SheetChanges none;
        none.replaceAutoFilter = true;
        none.filterMode = 0;
        const QByteArray gone = patch(second, none);
        QVERIFY(!gone.contains("<autoFilter"));
        QVERIFY(!gone.contains("filterMode"));
        QVERIFY(!readIt(gone).autoFilter.valid());
        QVERIFY(gone.contains("<mergeCells"));
        // the sheet's other parts are as they were
        QVERIFY(gone.contains("<pageMargins"));
    }

    void aFilterOfASheetWithAPrefixIsWrittenWithIt()
    {
        QByteArray xml = "<?xml version=\"1.0\"?><x:worksheet xmlns:x=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><x:sheetData><x:row r=\"1\"><x:c r=\"A1\"><x:v>1</x:v></x:c></x:row></x:sheetData>"
                         "<x:pageMargins left=\"1\" right=\"1\" top=\"1\" bottom=\"1\" header=\"0\" footer=\"0\"/></x:worksheet>";
        AutoFilter f;
        f.ref = CellRange{0, 0, 4, 1, true};
        FilterColumn col;
        col.column = 0;
        col.kind = FilterColumn::Kind::Custom;
        col.conditions.append({FilterColumn::Op::NotEqual, QStringLiteral("z")});
        f.columns.append(col);
        f.sortState = QStringLiteral("<sortState ref=\"A2:B5\"><sortCondition ref=\"A2:A5\"/></sortState>");
        SheetChanges c;
        c.replaceAutoFilter = true;
        c.autoFilter = f;
        const QByteArray out = patch(xml, c);
        QVERIFY(out.contains("<x:autoFilter"));
        QVERIFY(out.contains("<x:customFilter operator=\"notEqual\" val=\"z\"/>"));
        QVERIFY(out.contains("<x:sortCondition"));
        QVERIFY(out.contains("</x:autoFilter>"));
        QVERIFY(readIt(out).autoFilter == f);
    }

    void aHostileFilterIsReadWithinItsBounds()
    {
        QString filters;
        for (int i = 0; i < 300000; ++i) filters += QStringLiteral("<filter val=\"v%1\"/>").arg(i);
        const Sheet s = readIt(sheetWith(QStringLiteral("<autoFilter ref=\"A1:B5\"><filterColumn colId=\"0\"><filters>") + filters + QStringLiteral("</filters></filterColumn>"
                                                        "<filterColumn colId=\"99999\"><filters><filter val=\"x\"/></filters></filterColumn></autoFilter>")));
        QVERIFY(s.autoFilter.valid());
        QVERIFY(s.autoFilter.columns[0].values.size() <= 200000);
        QVERIFY(s.autoFilter.columns[0].values.size() >= 100000);
        QCOMPARE(s.autoFilter.columns.size(), 1);                     // (a column past the sheet is not kept)
        // a filter whose range is none
        QVERIFY(!readIt(sheetWith(QStringLiteral("<autoFilter ref=\"nonsense\"><filterColumn colId=\"0\"><filters/></filterColumn></autoFilter>"))).autoFilter.valid());
    }

    // ---- the hidden name a filter has in the workbook part
    void theHiddenNameOfAFiltersRangeIsMadeChangedAndTakenAway()
    {
        const QByteArray plain = "<?xml version=\"1.0\"?><workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><sheets><sheet name=\"A\" sheetId=\"1\"/><sheet name=\"B\" sheetId=\"2\"/></sheets>"
                                 "<calcPr calcId=\"1\"/></workbook>";
        QByteArray out;
        // made, where the format puts the list of names (before calcPr)
        QVERIFY(patchFilterNames(plain, {{1, QStringLiteral("'B'!$A$1:$C$9")}}, &out));
        QVERIFY(out.contains("<definedNames><definedName name=\"_xlnm._FilterDatabase\" localSheetId=\"1\" hidden=\"1\">'B'!$A$1:$C$9</definedName></definedNames><calcPr"));
        // changed in place, and one more added to the list that is there; a name of the file's own is left alone
        const QByteArray withNames = "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><sheets><sheet name=\"A\" sheetId=\"1\"/></sheets><definedNames>"
                                     "<definedName name=\"Mine\">A!$A$1</definedName><definedName name=\"_xlnm._FilterDatabase\" localSheetId=\"0\" hidden=\"1\">A!$A$1:$B$2</definedName></definedNames></workbook>";
        QByteArray second;
        QVERIFY(patchFilterNames(withNames, {{0, QStringLiteral("'A'!$A$1:$B$5")}, {1, QStringLiteral("'B'!$A$1:$B$2")}}, &second));
        QVERIFY(second.contains("<definedName name=\"Mine\">A!$A$1</definedName>"));
        QVERIFY(second.contains("localSheetId=\"0\" hidden=\"1\">'A'!$A$1:$B$5</definedName>"));
        QVERIFY(second.contains("localSheetId=\"1\" hidden=\"1\">'B'!$A$1:$B$2</definedName>"));
        QVERIFY(!second.contains("A!$A$1:$B$2<"));
        QCOMPARE(second.count("_xlnm._FilterDatabase"), 2);
        // taken away
        QByteArray third;
        QVERIFY(patchFilterNames(second, {{0, QString()}}, &third));
        QCOMPARE(third.count("_xlnm._FilterDatabase"), 1);
        QVERIFY(third.contains("<definedName name=\"Mine\">"));
        // nothing asked: nothing changed
        QByteArray same;
        QVERIFY(patchFilterNames(withNames, {}, &same));
        QCOMPARE(same, withNames);
        // an empty list of names is made whole
        const QByteArray empty = "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><sheets/><definedNames/></workbook>";
        QByteArray filled;
        QVERIFY(patchFilterNames(empty, {{0, QStringLiteral("'A'!$A$1:$B$2")}}, &filled));
        QVERIFY(filled.contains("<definedNames><definedName"));
    }

    // ---- the rules of validation the extension list holds
    void listsFromOtherSheetsAndTheirMessagesAreReadFromTheExtensionList()
    {
        const QString ext = QStringLiteral(
            "<dataValidations count=\"1\"><dataValidation type=\"whole\" sqref=\"A1\"><formula1>1</formula1><formula2>9</formula2></dataValidation></dataValidations>"
            "<extLst><ext xmlns:x14=\"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main\" uri=\"{CCE6A557-97BC-48b1-807F-77E2F8E8F89B}\">"
            "<x14:dataValidations xmlns:xm=\"http://schemas.microsoft.com/office/excel/2006/main\" count=\"2\">"
            "<x14:dataValidation type=\"list\" allowBlank=\"1\" showInputMessage=\"1\" showErrorMessage=\"1\" promptTitle=\"Pick\" prompt=\"Choose a name from the list.\" errorStyle=\"warning\">"
            "<x14:formula1><xm:f>Names!$A$1:$A$3</xm:f></x14:formula1><xm:sqref>C2:C9 E2</xm:sqref></x14:dataValidation>"
            "<x14:dataValidation type=\"decimal\" operator=\"between\" showErrorMessage=\"1\"><x14:formula1><xm:f>Limits!$B$1</xm:f></x14:formula1><x14:formula2><xm:f>Limits!$B$2</xm:f></x14:formula2>"
            "<xm:sqref>F1:F4</xm:sqref></x14:dataValidation></x14:dataValidations></ext></extLst>");
        const Sheet s = readIt(sheetWith(ext));
        QCOMPARE(s.validations.size(), 3);
        const Validation &list = s.validations[1];
        QCOMPARE(int(list.type), int(Validation::Type::List));
        QCOMPARE(list.formula1, QStringLiteral("Names!$A$1:$A$3"));
        QCOMPARE(list.ranges.size(), 2);
        QCOMPARE(rangeName(list.ranges[0]), QStringLiteral("C2:C9"));
        QVERIFY(list.showInput && list.showError && list.allowBlank);
        QCOMPARE(list.promptTitle, QStringLiteral("Pick"));
        QCOMPARE(list.prompt, QStringLiteral("Choose a name from the list."));
        QCOMPARE(int(list.style), int(Validation::Style::Warning));
        const Validation &between = s.validations[2];
        QCOMPARE(between.formula1, QStringLiteral("Limits!$B$1"));
        QCOMPARE(between.formula2, QStringLiteral("Limits!$B$2"));
        QCOMPARE(rangeName(between.ranges[0]), QStringLiteral("F1:F4"));
        // the extension is no longer left out
        QVERIFY(!s.notKept.contains(QLatin1String("validationLists")));
        // other things in the extension list still are
        const Sheet other = readIt(sheetWith(QStringLiteral("<extLst><ext uri=\"{x}\"><x14:sparklineGroups xmlns:x14=\"urn:x\"/></ext></extLst>")));
        QVERIFY(other.notKept.contains(QLatin1String("sparklines")));
    }

    void aSheetKeepsItsExtensionListThroughASave()
    {
        const QString ext = QStringLiteral("<extLst><ext uri=\"{CCE6A557-97BC-48b1-807F-77E2F8E8F89B}\"><x14:dataValidations xmlns:x14=\"urn:x14\" xmlns:xm=\"urn:xm\" count=\"1\">"
                                           "<x14:dataValidation type=\"list\"><x14:formula1><xm:f>Names!$A$1:$A$3</xm:f></x14:formula1><xm:sqref>C2</xm:sqref></x14:dataValidation></x14:dataValidations></ext></extLst>");
        const QByteArray xml = sheetWith(ext);
        SheetChanges c;
        SavedCell cell;
        cell.row = 0;
        cell.col = 0;
        cell.value.kind = SavedValue::Kind::Number;
        cell.value.number = 5;
        c.cells.push_back(cell);
        const QByteArray out = patch(xml, c);
        QVERIFY(out.contains("<xm:f>Names!$A$1:$A$3</xm:f>"));
        QVERIFY(out.contains("<xm:sqref>C2</xm:sqref>"));
        QCOMPARE(readIt(out).validations.size(), 1);
    }
};

JP_TEST_CLASS(SheetFilterTests)
#include "tst_sheet_filter.moc"
