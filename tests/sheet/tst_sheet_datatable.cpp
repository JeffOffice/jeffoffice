// Tests of the what-if (data) tables of the calculation: a table's cells are what its formula comes to with its input set to each value at the edge of the table; one input or two; worked out
// again when something the formula reads is changed (unless the workbook leaves tables by hand), and what reads the table's cells is worked out after them. Every workbook is built in memory.
#include "sheethelpers.h"

#include "sheet/calculation.h"

#include <memory>

namespace {

struct Book {
    Package package;
    Workbook book;
    std::unique_ptr<Calculation> calc;
    bool open(const QByteArray &zip, bool tables = true)
    {
        QString why;
        if (!package.readLazy(zip, &why) || !book.open(package, &why)) return false;
        CalcOptions options;
        options.keepApart = true;
        options.workOutWhatHasNoResult = true;
        options.workOutDataTables = tables;
        calc.reset(new Calculation(book, options));
        return calc->load();
    }
    QString at(const QString &ref) const
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return calc->cell(0, r, c).debug();
    }
    bool set(const QString &ref, double v)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return calc->setValue(0, r, c, Value::number(v));
    }
};

QString num(const QString &ref, double v) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref).arg(v); }
QString formula(const QString &ref, const QString &f, double v = 0) { return QStringLiteral("<c r=\"%1\"><f>%2</f><v>%3</v></c>").arg(ref, f).arg(v); }
QString row(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(n).arg(cells); }

// Two inputs: D1 and E1 are read by A3 (their product); the values across B3:E3 go in D1 (the row input), those down A4:A6 in E1 (the column input).
QByteArray twoInputs()
{
    const QString table = QStringLiteral("<c r=\"B4\"><f t=\"dataTable\" ref=\"B4:E6\" dt2D=\"1\" dtr=\"1\" r1=\"D1\" r2=\"E1\"/><v>0</v></c>");
    QString rows = row(1, num(QStringLiteral("D1"), 1) + num(QStringLiteral("E1"), 1)) + row(3, formula(QStringLiteral("A3"), QStringLiteral("D1*E1"), 1) + num(QStringLiteral("B3"), 1) + num(QStringLiteral("C3"), 2) + num(QStringLiteral("D3"), 3) + num(QStringLiteral("E3"), 4));
    for (int r = 4; r <= 6; ++r) {
        QString cells = num(QStringLiteral("A%1").arg(r), r - 3);
        for (int c = 0; c < 4; ++c) {
            const QString ref = QStringLiteral("%1%2").arg(QChar('B' + c)).arg(r);
            cells += r == 4 && c == 0 ? table : num(ref, 0);
        }
        rows += row(r, cells);
    }
    // (and a formula that reads the table)
    rows += row(8, formula(QStringLiteral("A8"), QStringLiteral("SUM(B4:E6)"), 0));
    return buildXlsx({worksheet(rows)});
}

// One input each way: C2 is A2*B2. Down: B4 holds =C2, the values A5:A7 go in A2, the table is B5:B7. Across: D5 holds =C2, the values E4:G4 go in B2, the table is E5:G5.
QByteArray oneInput()
{
    QString rows = row(2, num(QStringLiteral("A2"), 1) + num(QStringLiteral("B2"), 2) + formula(QStringLiteral("C2"), QStringLiteral("A2*B2"), 2));
    rows += row(4, formula(QStringLiteral("B4"), QStringLiteral("C2"), 2) + num(QStringLiteral("E4"), 10) + num(QStringLiteral("F4"), 20) + num(QStringLiteral("G4"), 30));
    rows += row(5, num(QStringLiteral("A5"), 1) + QStringLiteral("<c r=\"B5\"><f t=\"dataTable\" ref=\"B5:B7\" dt2D=\"0\" dtr=\"0\" r1=\"A2\"/><v>0</v></c>") + formula(QStringLiteral("D5"), QStringLiteral("C2"), 2)
                          + QStringLiteral("<c r=\"E5\"><f t=\"dataTable\" ref=\"E5:G5\" dt2D=\"0\" dtr=\"1\" r1=\"B2\"/><v>0</v></c>") + num(QStringLiteral("F5"), 0) + num(QStringLiteral("G5"), 0));
    rows += row(6, num(QStringLiteral("A6"), 2) + num(QStringLiteral("B6"), 0));
    rows += row(7, num(QStringLiteral("A7"), 3) + num(QStringLiteral("B7"), 0));
    return buildXlsx({worksheet(rows)});
}

} // namespace

class SheetDataTableTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void aTableOfTwoInputsIsWorkedOutFromItsEdges()
    {
        Book b;
        QVERIFY(b.open(twoInputs()));
        QVERIFY(b.calc->recalculate().finished);
        QCOMPARE(b.calc->dataTableCount(), qint64(1));
        // the product of the value across and the value down
        QCOMPARE(b.at(QStringLiteral("B4")), QStringLiteral("n:1"));
        QCOMPARE(b.at(QStringLiteral("C5")), QStringLiteral("n:4"));
        QCOMPARE(b.at(QStringLiteral("E6")), QStringLiteral("n:12"));
        QCOMPARE(b.at(QStringLiteral("D4")), QStringLiteral("n:3"));
        // the cell the table reads is as it was, and what reads the table has the new values
        QCOMPARE(b.at(QStringLiteral("D1")), QStringLiteral("n:1"));
        QCOMPARE(b.at(QStringLiteral("A3")), QStringLiteral("n:1"));
        QCOMPARE(b.at(QStringLiteral("A8")), QStringLiteral("n:60"));   // (the table's cells add up to (1+2+3+4) times (1+2+3))
    }

    void aTableOfOneInputGoesDownOrAcross()
    {
        Book b;
        QVERIFY(b.open(oneInput()));
        QVERIFY(b.calc->recalculate().finished);
        QCOMPARE(b.calc->dataTableCount(), qint64(2));
        // down: C2 with A2 set to each of 1, 2, 3 (B2 stays 2)
        QCOMPARE(b.at(QStringLiteral("B5")), QStringLiteral("n:2"));
        QCOMPARE(b.at(QStringLiteral("B6")), QStringLiteral("n:4"));
        QCOMPARE(b.at(QStringLiteral("B7")), QStringLiteral("n:6"));
        // across: C2 with B2 set to each of 10, 20, 30 (A2 stays 1)
        QCOMPARE(b.at(QStringLiteral("E5")), QStringLiteral("n:10"));
        QCOMPARE(b.at(QStringLiteral("F5")), QStringLiteral("n:20"));
        QCOMPARE(b.at(QStringLiteral("G5")), QStringLiteral("n:30"));
    }

    void aChangeToWhatTheFormulaReadsWorksTheTableOutAgain()
    {
        Book b;
        QVERIFY(b.open(oneInput()));
        QVERIFY(b.calc->recalculate().finished);
        QVERIFY(b.set(QStringLiteral("B2"), 5));
        QVERIFY(b.calc->recalculate().finished);
        QCOMPARE(b.at(QStringLiteral("B5")), QStringLiteral("n:5"));   // (A2 := 1, B2 = 5)
        QCOMPARE(b.at(QStringLiteral("B7")), QStringLiteral("n:15"));
        // and one of the values at the table's edge
        QVERIFY(b.set(QStringLiteral("A6"), 10));
        QVERIFY(b.calc->recalculate().finished);
        QCOMPARE(b.at(QStringLiteral("B6")), QStringLiteral("n:50"));
    }

    void tablesLeftByHandAreWorkedOutWhenAskedFor()
    {
        Book b;
        QVERIFY(b.open(oneInput()));
        b.calc->setDataTablesAutomatic(false);
        QVERIFY(b.calc->recalculate().finished);
        QCOMPARE(b.at(QStringLiteral("B6")), QStringLiteral("n:0"));   // (what the file stored)
        QVERIFY(b.calc->workOutDataTablesNow() > 0);
        QCOMPARE(b.at(QStringLiteral("B6")), QStringLiteral("n:4"));
        QVERIFY(b.set(QStringLiteral("B2"), 5));
        QVERIFY(b.calc->recalculate().finished);
        QCOMPARE(b.at(QStringLiteral("B6")), QStringLiteral("n:4"));   // (left as it was until asked for)
        QVERIFY(b.calc->workOutDataTablesNow() > 0);
        QCOMPARE(b.at(QStringLiteral("B6")), QStringLiteral("n:10"));
    }

    void withoutTheOptionATableKeepsWhatTheFileStored()
    {
        Book b;
        QVERIFY(b.open(twoInputs(), false));
        QVERIFY(b.calc->recalculate().finished);
        QCOMPARE(b.calc->dataTableCount(), qint64(0));
        QCOMPARE(b.at(QStringLiteral("C5")), QStringLiteral("n:0"));
    }

    // A table that names no cell, or a cell that is not one, or has no room for its edges, is left as the file has it, and a table too large to work out is too.
    void aTableThatCannotBeWorkedOutIsLeftAlone()
    {
        const QString rows = row(1, QStringLiteral("<c r=\"A1\"><f t=\"dataTable\" ref=\"A1:B2\" dt2D=\"0\" dtr=\"0\" r1=\"C9\"/><v>7</v></c>") + num(QStringLiteral("B1"), 8))
                             + row(2, num(QStringLiteral("A2"), 9) + num(QStringLiteral("B2"), 10))
                             + row(5, QStringLiteral("<c r=\"B5\"><f t=\"dataTable\" ref=\"B5:XFC1048575\" dt2D=\"0\" dtr=\"0\" r1=\"B1\"/><v>3</v></c>"));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        QVERIFY(b.calc->recalculate().finished);
        QCOMPARE(b.at(QStringLiteral("A1")), QStringLiteral("n:7"));
        QCOMPARE(b.at(QStringLiteral("B2")), QStringLiteral("n:10"));
        QCOMPARE(b.at(QStringLiteral("B5")), QStringLiteral("n:3"));
    }
};

JP_TEST_CLASS(SheetDataTableTests)
#include "tst_sheet_datatable.moc"
