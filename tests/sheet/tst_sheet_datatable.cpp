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

    // A table that comes to texts is left as the file has it.
    void aTableOfTextsIsLeftAlone()
    {
        QString rows = row(2, num(QStringLiteral("A2"), 1) + formula(QStringLiteral("C2"), QStringLiteral("IF(A2>1,\"big\",\"small\")"), 0));
        rows += row(4, formula(QStringLiteral("B4"), QStringLiteral("C2"), 0));
        rows += row(5, num(QStringLiteral("A5"), 1) + QStringLiteral("<c r=\"B5\"><f t=\"dataTable\" ref=\"B5:B7\" dt2D=\"0\" dtr=\"0\" r1=\"A2\"/><v>7</v></c>"));
        rows += row(6, num(QStringLiteral("A6"), 2) + num(QStringLiteral("B6"), 8));
        rows += row(7, num(QStringLiteral("A7"), 3) + num(QStringLiteral("B7"), 9));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        QVERIFY(b.calc->recalculate().finished);
        QCOMPARE(b.at(QStringLiteral("B5")), QStringLiteral("n:7"));
        QCOMPARE(b.at(QStringLiteral("B6")), QStringLiteral("n:8"));
        QCOMPARE(b.at(QStringLiteral("B7")), QStringLiteral("n:9"));
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

    // ---- review fixes

    // Working a table out does not write the cells of the sheet (the worker's results are told, as a formula's are, and the sheet's cells are the file's, which the window reads at the same time).
    void workingATableOutLeavesTheCellsOfTheSheetAsTheFileHadThem()
    {
        Book b;
        QVERIFY(b.open(twoInputs()));
        QVERIFY(b.calc->recalculate().finished);
        QCOMPARE(b.at(QStringLiteral("C5")), QStringLiteral("n:4"));   // (what the table comes to, as the engine reads it)
        quint32 r = 0, c = 0;
        QVERIFY(parseCellRef(QStringLiteral("C5"), &r, &c));
        const Cell *cell = b.calc->sheet(0).cells.find(r, c);
        QVERIFY(cell);
        QCOMPARE(cell->number, 0.0);   // (what the file stored)
    }

    // A table is worked out again only when something it read has changed: an edit of a cell nothing in it reads costs none, and one of a cell its formula reads costs the tables that read it.
    void aTableIsWorkedOutAgainOnlyWhenWhatItReadChanged()
    {
        Book b;
        QVERIFY(b.open(oneInput()));
        QVERIFY(b.calc->recalculate().finished);
        const qint64 first = b.calc->dataTablesWorked();
        QVERIFY(first >= 2);
        QVERIFY(b.set(QStringLiteral("H20"), 5));   // (read by nothing)
        QVERIFY(b.calc->recalculate().finished);
        QCOMPARE(b.calc->dataTablesWorked(), first);
        QVERIFY(b.set(QStringLiteral("B2"), 5));   // (read by C2, which both tables' formulas come to)
        QVERIFY(b.calc->recalculate().finished);
        QVERIFY(b.calc->dataTablesWorked() > first);
        QCOMPARE(b.at(QStringLiteral("B7")), QStringLiteral("n:15"));
        const qint64 second = b.calc->dataTablesWorked();
        QVERIFY(b.set(QStringLiteral("A6"), 10));   // (an edge of the first table only)
        QVERIFY(b.calc->recalculate().finished);
        QCOMPARE(b.calc->dataTablesWorked(), second + 1);
        QCOMPARE(b.at(QStringLiteral("B6")), QStringLiteral("n:50"));
    }

    // A table that reads the cells of another is worked out after it, whichever of them comes first in the sheet.
    void aTableThatReadsAnothersCellsIsWorkedOutAfterIt()
    {
        for (int layout = 0; layout < 2; ++layout) {
            // the first table: C2 = A2*B2 as a function of A2 (the values in A5:A7, the table B5:B7); the second table's formula, at E4, is B7 + 1, and its input is D2, the values D5:D6: E5:E6 come to B7 + 1 whatever D2 is,
            // so what matters is that B7 is worked out when E4 reads it
            QString rows = row(2, num(QStringLiteral("A2"), 1) + num(QStringLiteral("B2"), 2) + formula(QStringLiteral("C2"), QStringLiteral("A2*B2"), 2) + num(QStringLiteral("D2"), 1));
            rows += row(4, formula(QStringLiteral("B4"), QStringLiteral("C2"), 2) + formula(QStringLiteral("E4"), QStringLiteral("B7+D2"), 0));
            const QString first = QStringLiteral("<c r=\"B5\"><f t=\"dataTable\" ref=\"B5:B7\" dt2D=\"0\" dtr=\"0\" r1=\"A2\"/><v>0</v></c>");
            const QString second = QStringLiteral("<c r=\"E5\"><f t=\"dataTable\" ref=\"E5:E6\" dt2D=\"0\" dtr=\"0\" r1=\"D2\"/><v>0</v></c>");
            rows += row(5, num(QStringLiteral("A5"), 1) + first + num(QStringLiteral("D5"), 10) + second);
            rows += row(6, num(QStringLiteral("A6"), 2) + num(QStringLiteral("B6"), 0) + num(QStringLiteral("D6"), 20) + num(QStringLiteral("E6"), 0));
            rows += row(7, num(QStringLiteral("A7"), 3) + num(QStringLiteral("B7"), 0));
            if (layout == 1) {
                // (the same tables with the second one's cells placed first in the file's order: the formulas are the same, the order of the sheet's cells differs)
                rows = row(1, num(QStringLiteral("Z1"), 0)) + rows;
            }
            Book b;
            QVERIFY(b.open(buildXlsx({worksheet(rows)})));
            QVERIFY(b.calc->recalculate().finished);
            QCOMPARE(b.at(QStringLiteral("B7")), QStringLiteral("n:6"));
            QCOMPARE(b.at(QStringLiteral("E5")), QStringLiteral("n:16"));   // (B7 = 6, D2 = 10)
            QCOMPARE(b.at(QStringLiteral("E6")), QStringLiteral("n:26"));
            QCOMPARE(b.calc->dataTablesInCircles(), qint64(0));
        }
    }

    // Tables that read each other's cells in a circle are left as the file has them, and counted.
    void tablesThatReadEachOtherInACircleAreLeftAlone()
    {
        QString rows = row(2, num(QStringLiteral("A2"), 1) + formula(QStringLiteral("C2"), QStringLiteral("A2+E5"), 0) + num(QStringLiteral("D2"), 1));
        rows += row(4, formula(QStringLiteral("B4"), QStringLiteral("C2"), 0) + formula(QStringLiteral("E4"), QStringLiteral("D2+B5"), 0));
        rows += row(5, num(QStringLiteral("A5"), 1) + QStringLiteral("<c r=\"B5\"><f t=\"dataTable\" ref=\"B5:B6\" dt2D=\"0\" dtr=\"0\" r1=\"A2\"/><v>7</v></c>") + num(QStringLiteral("D5"), 1)
                          + QStringLiteral("<c r=\"E5\"><f t=\"dataTable\" ref=\"E5:E6\" dt2D=\"0\" dtr=\"0\" r1=\"D2\"/><v>8</v></c>"));
        rows += row(6, num(QStringLiteral("A6"), 2) + num(QStringLiteral("B6"), 9) + num(QStringLiteral("D6"), 2) + num(QStringLiteral("E6"), 10));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        QVERIFY(b.calc->recalculate().finished);
        QCOMPARE(b.calc->dataTablesInCircles(), qint64(2));
        QCOMPARE(b.at(QStringLiteral("B5")), QStringLiteral("n:7"));
        QCOMPARE(b.at(QStringLiteral("B6")), QStringLiteral("n:9"));
        QCOMPARE(b.at(QStringLiteral("E5")), QStringLiteral("n:8"));
        QCOMPARE(b.at(QStringLiteral("E6")), QStringLiteral("n:10"));
    }

    // A table of IRR: the rate comes to what the flows come to with the input set, as a table of NPV does (the flows are formulas that read the input).
    void aTableOfIrrFollowsTheInput()
    {
        QMap<int, QString> grid;
        grid[1] += num(QStringLiteral("A1"), 1000);
        grid[2] += formula(QStringLiteral("B2"), QStringLiteral("-A1"), -1000) + num(QStringLiteral("C2"), 400) + num(QStringLiteral("D2"), 400) + num(QStringLiteral("E2"), 400);
        grid[4] += formula(QStringLiteral("B4"), QStringLiteral("IRR(B2:E2)"), 0.097) + formula(QStringLiteral("C4"), QStringLiteral("NPV(0.1,C2:E2)+B2"), 0);
        grid[5] += num(QStringLiteral("A5"), 1100) + QStringLiteral("<c r=\"B5\"><f t=\"dataTable\" ref=\"B5:C6\" dt2D=\"0\" dtr=\"0\" r1=\"A1\"/><v>9</v></c>") + num(QStringLiteral("C5"), 9);
        grid[6] += num(QStringLiteral("A6"), 900) + num(QStringLiteral("B6"), 9) + num(QStringLiteral("C6"), 9);
        QString rows;
        for (int r = 1; r <= 6; ++r) rows += row(r, grid.value(r));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        QVERIFY(b.calc->recalculate().finished);
        const QString irr1100 = b.at(QStringLiteral("B5")), irr900 = b.at(QStringLiteral("B6"));
        QVERIFY2(irr1100 != irr900, qPrintable(irr1100 + " " + irr900));
        // (-1100, 400 x 3: the rate is under 6%, and for -900 it is over 15%)
        QVERIFY(irr1100.mid(2).toDouble() < 0.06 && irr900.mid(2).toDouble() > 0.15);
    }

    // All the tables together may take only the work a recalculation may: past it the rest are left as the file has them, and counted.
    void tablesTooCostlyToWorkOutAreLeftAloneAndCounted()
    {
        // a table of 200 x 200 cells whose formula sums a thousand cells each time
        QMap<int, QString> grid;
        grid[1] += num(QStringLiteral("A1"), 1) + num(QStringLiteral("B1"), 1) + formula(QStringLiteral("C1"), QStringLiteral("SUM(%1%2:%1%3)*A1*B1").arg(columnName(210)).arg(1).arg(1000), 0);
        grid[3] += formula(QStringLiteral("A3"), QStringLiteral("C1"), 0);
        for (int c = 1; c <= 200; ++c) grid[3] += num(QStringLiteral("%1%2").arg(columnName(quint32(c))).arg(3), c);
        for (int r = 4; r <= 203; ++r) {
            grid[r] += num(QStringLiteral("A%1").arg(r), r);
            for (int c = 1; c <= 200; ++c)
                grid[r] += r == 4 && c == 1 ? QStringLiteral("<c r=\"B4\"><f t=\"dataTable\" ref=\"B4:GS203\" dt2D=\"1\" dtr=\"1\" r1=\"A1\" r2=\"B1\"/><v>5</v></c>")
                                            : num(QStringLiteral("%1%2").arg(columnName(quint32(c))).arg(r), 5);
        }
        QString rows;
        for (int r = 1; r <= 1000; ++r) rows += row(r, grid.value(r) + num(columnName(210) + QString::number(r), 1));
        Book b;
        CalcOptions options;
        options.keepApart = true;
        options.workOutWhatHasNoResult = true;
        options.workOutDataTables = true;
        options.recalcWorkLimit = 300000;
        QString why;
        QVERIFY(b.package.readLazy(buildXlsx({worksheet(rows)}), &why) && b.book.open(b.package, &why));
        b.calc.reset(new Calculation(b.book, options));
        QVERIFY(b.calc->load());
        QVERIFY(b.calc->recalculate().finished);
        QCOMPARE(b.calc->dataTablesTooCostly(), qint64(1));
        QCOMPARE(b.at(QStringLiteral("C5")), QStringLiteral("n:5"));   // (as the file has it)
    }

    // A calculation that is stopped in the middle of a table leaves the table as it was (none of its cells is half done), and works it out when it is gone on with.
    void aStopInTheMiddleOfATableLeavesItAsItWas()
    {
        QMap<int, QString> grid;
        grid[1] += num(QStringLiteral("A1"), 1) + formula(QStringLiteral("C1"), QStringLiteral("SUM(F1:F50)*A1"), 0);
        grid[3] += formula(QStringLiteral("B3"), QStringLiteral("C1"), 0);
        for (int r = 4; r <= 43; ++r)
            grid[r] += num(QStringLiteral("A%1").arg(r), r) + (r == 4 ? QStringLiteral("<c r=\"B4\"><f t=\"dataTable\" ref=\"B4:B43\" dt2D=\"0\" dtr=\"0\" r1=\"A1\"/><v>5</v></c>") : num(QStringLiteral("B%1").arg(r), 5));
        QString rows;
        for (int r = 1; r <= 50; ++r) rows += row(r, grid.value(r) + num(QStringLiteral("F%1").arg(r), 1));
        Book b;
        QVERIFY(b.open(buildXlsx({worksheet(rows)})));
        std::atomic<bool> stop{false};
        int visits = 0;
        b.calc->setVisitHook([&] {
            if (++visits == 30) stop = true;   // (after the cells are worked out and the table has begun)
        });
        const Calculation::Progress first = b.calc->recalculate(0, &stop);
        QVERIFY(first.cancelled);
        QCOMPARE(b.at(QStringLiteral("B4")), QStringLiteral("n:5"));
        QCOMPARE(b.at(QStringLiteral("B43")), QStringLiteral("n:5"));
        stop = false;
        b.calc->setVisitHook(nullptr);
        QVERIFY(b.calc->recalculate(0, &stop).finished);
        QCOMPARE(b.at(QStringLiteral("B4")), QStringLiteral("n:200"));
        QCOMPARE(b.at(QStringLiteral("B43")), QStringLiteral("n:2150"));   // (50 cells of 1, times the edge, 43)
    }
};

JP_TEST_CLASS(SheetDataTableTests)
#include "tst_sheet_datatable.moc"
