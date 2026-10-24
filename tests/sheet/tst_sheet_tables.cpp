// Tests of tables: reading where they are and what their columns are called, renaming a column in the formulas that name it and in the table's part (so that the file stays valid when
// a person types in a header cell), the name a typed text makes, and saving the part. Every workbook is built in memory.
#include "sheethelpers.h"

#include "sheet/tables.h"
#include "sheet/xlsxsave.h"

namespace {

const char *const kNs = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

QString tableXml(const QString &name, const QString &ref, const QStringList &columns, const QStringList &formulas = QStringList())
{
    QString cols;
    for (int i = 0; i < columns.size(); ++i) {
        const QString formula = i < formulas.size() && !formulas[i].isEmpty() ? QStringLiteral("<calculatedColumnFormula>%1</calculatedColumnFormula>").arg(formulas[i].toHtmlEscaped()) : QString();
        cols += QStringLiteral("<tableColumn id=\"%1\" name=\"%2\" dataDxfId=\"0\">%3</tableColumn>").arg(i + 1).arg(columns[i].toHtmlEscaped(), formula);
    }
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<table xmlns=\"%1\" id=\"1\" name=\"%2\" displayName=\"%2\" ref=\"%3\" totalsRowShown=\"0\"><autoFilter ref=\"%3\"/>"
                          "<tableColumns count=\"%4\">%5</tableColumns><tableStyleInfo name=\"TableStyleMedium2\" showRowStripes=\"1\"/></table>")
        .arg(QLatin1String(kNs), name, ref).arg(columns.size()).arg(cols);
}

QString inlineCell(const QString &ref, const QString &text) { return QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, text.toHtmlEscaped()); }

QByteArray salesBook()
{
    const QString rows = QStringLiteral("<row r=\"1\">%1%2%3</row><row r=\"2\"><c r=\"A2\" t=\"inlineStr\"><is><t>East</t></is></c><c r=\"B2\"><v>5</v></c><c r=\"C2\"><f>Sales[[#This Row],[Q1]]*2</f><v>10</v></c></row>")
                             .arg(inlineCell(QStringLiteral("A1"), QStringLiteral("Region")), inlineCell(QStringLiteral("B1"), QStringLiteral("Q1")), inlineCell(QStringLiteral("C1"), QStringLiteral("Total")));
    return withTables(buildXlsx({worksheet(rows)}),
                      {tableXml(QStringLiteral("Sales"), QStringLiteral("A1:C2"), {QStringLiteral("Region"), QStringLiteral("Q1"), QStringLiteral("Total")}, {QString(), QString(), QStringLiteral("Sales[[#This Row],[Q1]]*2")})});
}

} // namespace

class SheetTablesTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A table is read with where it is and its columns' names; a cell of the header row says which column it is.
    void tablesAreReadWithTheirHeaders()
    {
        Package p;
        QString why;
        QVERIFY(p.readLazy(salesBook(), &why));
        Workbook b;
        QVERIFY2(b.open(p, &why), qPrintable(why));
        QCOMPARE(b.tables().size(), 1);
        const Table &t = b.tables().first();
        QCOMPARE(t.name, QStringLiteral("Sales"));
        QCOMPARE(t.sheet, 0);
        QCOMPARE(t.part, QStringLiteral("/xl/tables/table1.xml"));
        QCOMPARE(t.columns, (QStringList{QStringLiteral("Region"), QStringLiteral("Q1"), QStringLiteral("Total")}));
        QCOMPARE(int(t.row1), 0);
        QCOMPARE(int(t.col2), 2);
        int column = -1;
        QCOMPARE(b.tableHeaderAt(0, 0, 1, &column), 0);
        QCOMPARE(column, 1);
        QCOMPARE(b.tableHeaderAt(0, 1, 1, &column), -1);    // (a cell below the header)
        QCOMPARE(b.tableHeaderAt(0, 0, 3, &column), -1);    // (beside the table)
        QCOMPARE(b.tableHeaderAt(1, 0, 1, &column), -1);    // (another sheet)
        // a workbook with no table has none
        Package q;
        QVERIFY(q.readLazy(buildXlsx({worksheet(QString())}), &why));
        Workbook c;
        QVERIFY(c.open(q, &why));
        QVERIFY(c.tables().isEmpty());
    }

    // A column renamed in the formulas that name it: with the table's name, with a row's own cell, in a range of columns, with the name escaped; not in a text, not in another table's
    // formula, not a column of another name; and a formula in the table with no table's name only when it is inside it.
    void aColumnIsRenamedWhereFormulasNameIt()
    {
        auto rename = [](const QString &f, bool inside = false) { return renameColumnInFormula(f, QStringLiteral("Sales"), QStringLiteral("Q1"), QStringLiteral("First quarter"), inside); };
        QCOMPARE(rename(QStringLiteral("SUM(Sales[Q1])")), QStringLiteral("SUM(Sales[First quarter])"));
        QCOMPARE(rename(QStringLiteral("sales[q1]+1")), QStringLiteral("sales[First quarter]+1"));
        QCOMPARE(rename(QStringLiteral("Sales[[#This Row],[Q1]]*2")), QStringLiteral("Sales[[#This Row],[First quarter]]*2"));
        QCOMPARE(rename(QStringLiteral("Sales[[#Headers],[Q1]]")), QStringLiteral("Sales[[#Headers],[First quarter]]"));
        QCOMPARE(rename(QStringLiteral("SUM(Sales[[Q1]:[Total]])")), QStringLiteral("SUM(Sales[[First quarter]:[Total]])"));
        QCOMPARE(rename(QStringLiteral("Sales[#All]+Sales[Region]")), QStringLiteral("Sales[#All]+Sales[Region]"));
        QCOMPARE(rename(QStringLiteral("Other[Q1]+\"Sales[Q1]\"")), QStringLiteral("Other[Q1]+\"Sales[Q1]\""));
        QCOMPARE(rename(QStringLiteral("'Sheet [Q1]'!A1+Sales[Q1]")), QStringLiteral("'Sheet [Q1]'!A1+Sales[First quarter]"));
        QCOMPARE(rename(QStringLiteral("A1+B1")), QStringLiteral("A1+B1"));
        // no table's name: the table that holds the formula
        QCOMPARE(rename(QStringLiteral("[@Q1]*2"), true), QStringLiteral("[@First quarter]*2"));
        QCOMPARE(rename(QStringLiteral("[@[Q1]]*2+[@Total]"), true), QStringLiteral("[@[First quarter]]*2+[@Total]"));
        QCOMPARE(rename(QStringLiteral("[[#This Row],[Q1]]"), true), QStringLiteral("[[#This Row],[First quarter]]"));
        QCOMPARE(rename(QStringLiteral("[@Q1]*2"), false), QStringLiteral("[@Q1]*2"));
        // names with characters that are escaped, or need the whole of the name in brackets
        QCOMPARE(renameColumnInFormula(QStringLiteral("Sales[Q1]"), QStringLiteral("Sales"), QStringLiteral("Q1"), QStringLiteral("It's [done]"), false), QStringLiteral("Sales[[It''s '[done']]]"));
        QCOMPARE(renameColumnInFormula(QStringLiteral("Sales[Q1]"), QStringLiteral("Sales"), QStringLiteral("Q1"), QStringLiteral("A, B"), false), QStringLiteral("Sales[[A, B]]"));
        QCOMPARE(renameColumnInFormula(QStringLiteral("Sales[@Q1]"), QStringLiteral("Sales"), QStringLiteral("Q1"), QStringLiteral("A, B"), false), QStringLiteral("Sales[@[A, B]]"));
        // a name that has an escape in it is found as it was escaped
        QCOMPARE(renameColumnInFormula(QStringLiteral("Sales[It'''s]"), QStringLiteral("Sales"), QStringLiteral("It's"), QStringLiteral("Its"), false), QStringLiteral("Sales[Its]"));
    }

    // The name a typed text makes: trimmed, an apostrophe in front dropped, a number after it when another column has the name (whatever the capitals), nothing for an empty text or one
    // of over 255 characters.
    void aTypedTextMakesAUniqueColumnName()
    {
        const QStringList columns{QStringLiteral("Region"), QStringLiteral("Q1"), QStringLiteral("Total")};
        QCOMPARE(uniqueColumnName(QStringLiteral("  First quarter "), columns, 1), QStringLiteral("First quarter"));
        QCOMPARE(uniqueColumnName(QStringLiteral("Q1"), columns, 1), QStringLiteral("Q1"));          // (its own name again)
        QCOMPARE(uniqueColumnName(QStringLiteral("total"), columns, 1), QStringLiteral("total2"));
        QCOMPARE(uniqueColumnName(QStringLiteral("Region"), QStringList{QStringLiteral("Region"), QStringLiteral("Region2"), QStringLiteral("Q")}, 2), QStringLiteral("Region3"));
        QCOMPARE(uniqueColumnName(QStringLiteral("'007"), columns, 1), QStringLiteral("007"));
        bool tooLong = false;
        QVERIFY(uniqueColumnName(QStringLiteral("   "), columns, 1, &tooLong).isEmpty() && !tooLong);
        QVERIFY(uniqueColumnName(QString(256, QLatin1Char('x')), columns, 1, &tooLong).isEmpty() && tooLong);
        QCOMPARE(uniqueColumnName(QString(255, QLatin1Char('x')), columns, 1).size(), 255);
    }

    // The table part with a column renamed: the name of the column, and the formulas of the part follow; nothing else of it is touched.
    void aTablePartIsWrittenWithTheNewNames()
    {
        const QString xml = tableXml(QStringLiteral("Sales"), QStringLiteral("A1:C2"), {QStringLiteral("Region"), QStringLiteral("Q1 & Q2"), QStringLiteral("Total")},
                                     {QString(), QString(), QStringLiteral("Sales[[#This Row],[Q1 & Q2]]*2+[@Region]")});
        const QStringList old{QStringLiteral("Region"), QStringLiteral("Q1 & Q2"), QStringLiteral("Total")};
        const QStringList now{QStringLiteral("Region"), QStringLiteral("Half \"one\" <1>"), QStringLiteral("Total")};
        QByteArray out;
        QVERIFY(renameTableXml(xml.toUtf8(), old, now, QStringLiteral("Sales"), &out));
        const QString text = QString::fromUtf8(out);
        QVERIFY2(text.contains(QLatin1String("<tableColumn id=\"2\" name=\"Half &quot;one&quot; &lt;1&gt;\" dataDxfId=\"0\">")), qPrintable(text));
        QVERIFY2(text.contains(QLatin1String("<calculatedColumnFormula>Sales[[#This Row],[Half \"one\" &lt;1&gt;]]*2+[@Region]</calculatedColumnFormula>")), qPrintable(text));
        QVERIFY(text.contains(QLatin1String("<tableColumn id=\"1\" name=\"Region\" dataDxfId=\"0\">")));
        QVERIFY(text.contains(QLatin1String("<tableStyleInfo name=\"TableStyleMedium2\" showRowStripes=\"1\"/>")));
        QVERIFY(text.contains(QLatin1String("<autoFilter ref=\"A1:C2\"/>")));
        // nothing renamed: the same part
        QVERIFY(renameTableXml(xml.toUtf8(), old, old, QStringLiteral("Sales"), &out));
        QCOMPARE(QString::fromUtf8(out), xml);
        // a part that is no table's
        QVERIFY(!renameTableXml(QByteArray("<worksheet/>"), old, now, QStringLiteral("Sales"), &out));
    }

    // Saving: a table part whose column was renamed goes in the file written anew, and every other part is the old file's.
    void aRenamedColumnIsInTheSavedFile()
    {
        const QByteArray original = salesBook();
        Package p;
        QString why;
        QVERIFY(p.readLazy(original, &why));
        Workbook b;
        QVERIFY2(b.open(p, &why), qPrintable(why));
        QVERIFY(b.changedTableParts().empty());
        b.setTableColumnName(0, 1, QStringLiteral("First quarter"));
        const std::map<QString, QByteArray> changed = b.changedTableParts();
        QCOMPARE(int(changed.size()), 1);
        QVERIFY(changed.count(QStringLiteral("/xl/tables/table1.xml")) == 1);
        SaveChanges changes;
        changes.replacedParts = changed;
        QByteArray out;
        SaveReport report;
        QVERIFY2(saveXlsx(b, changes, &out, &why, &report), qPrintable(why));
        QMap<QString, QByteArray> before, after;
        QVERIFY(jp::readZip(original, before) && jp::readZip(out, after));
        QCOMPARE(after.keys(), before.keys());
        for (auto it = before.begin(); it != before.end(); ++it) {
            if (it.key() == QLatin1String("xl/tables/table1.xml")) {
                QVERIFY(after.value(it.key()).contains("name=\"First quarter\""));
                QVERIFY(after.value(it.key()).contains("<calculatedColumnFormula>Sales[[#This Row],[First quarter]]*2</calculatedColumnFormula>"));
            } else {
                QVERIFY2(after.value(it.key()) == it.value(), qPrintable(it.key()));
            }
        }
        QCOMPARE(report.partsChanged, 1);
        // the file read again has the new name; after the save the workbook has it as the file's
        Package q;
        QVERIFY(q.readLazy(out, &why));
        b.savedTo(q, 0, 0, 0);
        QCOMPARE(b.tables().first().columns.at(1), QStringLiteral("First quarter"));
        QVERIFY(b.changedTableParts().empty());
    }
};

JP_TEST_CLASS(SheetTablesTests)
#include "tst_sheet_tables.moc"
