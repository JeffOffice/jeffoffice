// Tests of rows, columns, and cells inserted and deleted in a calculation (Calculation::applyEdit): the cells move, every formula that reads cells that moved is written again with its
// references following them (on every sheet, and in the defined names), a reference to deleted cells is #REF!, what depends is worked out again, and Undo puts everything back. Counted
// work only: what moved, and what was written again. Every workbook is built in memory.
#include "sheethelpers.h"

#include "sheet/calculation.h"
#include "sheet/tables.h"

#include <QRandomGenerator>

#include <memory>

namespace {

// A sheet's cells from "A1=1" (a number), "B1=text", "C1==SUM(A1:A5)" (a formula), in any order.
QByteArray sheetXml(const QStringList &specs)
{
    struct Item {
        quint32 row, col;
        QString xml;
    };
    QVector<Item> items;
    for (const QString &spec : specs) {
        const int eq = int(spec.indexOf(QLatin1Char('=')));
        const QString ref = spec.left(eq), value = spec.mid(eq + 1);
        quint32 row = 0, col = 0;
        parseCellRef(ref, &row, &col);
        QString xml;
        if (value.startsWith(QLatin1Char('='))) {
            xml = QStringLiteral("<c r=\"%1\"><f>%2</f><v>0</v></c>").arg(ref, value.mid(1).toHtmlEscaped());
        } else {
            bool number = false;
            value.toDouble(&number);
            xml = number ? QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref, value)
                         : QStringLiteral("<c r=\"%1\" t=\"inlineStr\"><is><t>%2</t></is></c>").arg(ref, value.toHtmlEscaped());
        }
        items.append({row, col, xml});
    }
    std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) { return a.row != b.row ? a.row < b.row : a.col < b.col; });
    QString rows;
    for (int i = 0; i < items.size();) {
        const quint32 row = items[i].row;
        rows += QStringLiteral("<row r=\"%1\">").arg(row + 1);
        for (; i < items.size() && items[i].row == row; ++i) rows += items[i].xml;
        rows += QStringLiteral("</row>");
    }
    return worksheet(rows);
}

struct Book {
    Package package;
    Workbook book;
    std::unique_ptr<Calculation> calc;
    // `after` is what follows the cells in the first sheet (merged ranges), `names` the inside of definedNames.
    bool open(const QVector<QStringList> &sheets, const QString &after = QString(), const QString &names = QString())
    {
        QVector<QByteArray> parts;
        for (int i = 0; i < sheets.size(); ++i) {
            QByteArray xml = sheetXml(sheets[i]);
            if (i == 0 && !after.isEmpty()) xml.replace("</worksheet>", (after + QStringLiteral("</worksheet>")).toUtf8());
            parts << xml;
        }
        QString why;
        const QByteArray zip = buildXlsx(parts, QByteArray(), QString(), QString(), QByteArray(), QStringList(), names);
        if (!package.readLazy(zip, &why) || !book.open(package, &why)) return false;
        CalcOptions options;
        options.keepApart = true;
        options.workOutWhatHasNoResult = true;
        calc.reset(new Calculation(book, options));
        if (!calc->load()) return false;
        finish();
        return true;
    }
    // The first sheet as the XML of its part.
    bool openXml(const QByteArray &sheet)
    {
        QString why;
        const QByteArray zip = buildXlsx({sheet});
        if (!package.readLazy(zip, &why) || !book.open(package, &why)) return false;
        CalcOptions options;
        options.keepApart = true;
        options.workOutWhatHasNoResult = true;
        calc.reset(new Calculation(book, options));
        if (!calc->load()) return false;
        finish();
        return true;
    }
    static quint32 row(const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return r;
    }
    static quint32 col(const QString &ref)
    {
        quint32 r = 0, c = 0;
        parseCellRef(ref, &r, &c);
        return c;
    }
    QString at(const QString &ref, int sheet = 0) const { return calc->cell(sheet, row(ref), col(ref)).debug(); }
    QString formula(const QString &ref, int sheet = 0) const { return calc->content(sheet, row(ref), col(ref)).formula; }
    bool has(const QString &ref, int sheet = 0) const { return calc->content(sheet, row(ref), col(ref)).exists; }
    void finish() { while (!calc->recalculate().finished) {} }
    bool apply(const StructEdit &e, Calculation::EditRecord *rec)
    {
        const bool ok = calc->applyEdit(e, rec);
        finish();
        return ok;
    }
};

} // namespace

class SheetStructureTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // A row inserted: the cells at and below it move down, formulas that read them (on any sheet) follow, a range with the row inside grows, and what depends has its value again.
    void insertingARowMovesTheCellsAndTheReferences()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B1==SUM(A1:A5)", "B2==A5*2", "B3==A2+1", "C1==$A$4+A$3", "D1==SUM(A:A)"}, {"A1==Sheet1!A5+Sheet1!B1", "A2==SUM(Sheet1!A1:A3)"}}));
        QCOMPARE(b.at("B1"), QStringLiteral("n:15"));
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertRows(0, 2, 1), &rec));                 // a row at row 3
        QCOMPARE(b.at("A1"), QStringLiteral("n:1"));
        QCOMPARE(b.at("A2"), QStringLiteral("n:2"));
        QVERIFY(!b.has("A3"));
        QCOMPARE(b.at("A4"), QStringLiteral("n:3"));
        QCOMPARE(b.at("A6"), QStringLiteral("n:5"));
        QCOMPARE(b.formula("B1"), QStringLiteral("SUM(A1:A6)"));                // (the range grew)
        QCOMPARE(b.formula("B2"), QStringLiteral("A6*2"));
        QCOMPARE(b.formula("B4"), QStringLiteral("A2+1"));                       // (B3 moved down to B4 and still reads A2)
        QCOMPARE(b.formula("C1"), QStringLiteral("$A$5+A$4"));
        QCOMPARE(b.formula("D1"), QStringLiteral("SUM(A:A)"));
        QCOMPARE(b.formula("A1", 1), QStringLiteral("Sheet1!A6+Sheet1!B1"));    // (another sheet's formula)
        QCOMPARE(b.formula("A2", 1), QStringLiteral("SUM(Sheet1!A1:A4)"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:15"));
        QCOMPARE(b.at("B2"), QStringLiteral("n:10"));
        QCOMPARE(b.at("A1", 1), QStringLiteral("n:20"));
        QCOMPARE(b.at("A2", 1), QStringLiteral("n:6"));                         // (1 + 2 + the empty row + 3)
        // the graph follows: a change to a cell that moved is read by what reads it
        QVERIFY(b.calc->setValue(0, 5, 0, Value::number(50)));                   // A6
        b.finish();
        QCOMPARE(b.at("B2"), QStringLiteral("n:100"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:60"));
        // a value put in the row that was made is read by the range that grew over it
        QVERIFY(b.calc->setValue(0, 2, 0, Value::number(1000)));                 // A3
        b.finish();
        QCOMPARE(b.at("B1"), QStringLiteral("n:1060"));
        // counted: the cells at and below the row moved (A3-A5, B3, and nothing above)
        QCOMPARE(rec.cellsMoved, qint64(4));
    }

    // Rows deleted: the cells below move up, a range loses the rows deleted from it, a reference to a deleted cell is #REF!, and Undo puts everything back as it was (the cells, the
    // formulas, and the values).
    void deletingRowsMakesRefAndUndoPutsItBack()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B1==SUM(A1:A5)", "B2==A3*2", "B3==A5*2", "B4==A2+1", "C1==B2+1"}}));
        auto snapshot = [&] {
            QStringList out;
            for (int r = 1; r <= 8; ++r)
                for (const char *c : {"A", "B", "C"}) {
                    const QString ref = QString::fromLatin1(c) + QString::number(r);
                    out << ref + QLatin1Char(':') + (b.has(ref) ? b.at(ref) + QLatin1Char(' ') + b.formula(ref) : QStringLiteral("-"));
                }
            return out.join(QLatin1Char('\n'));
        };
        const QString before = snapshot();
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::deleteRows(0, 2, 2), &rec));                 // rows 3 and 4
        QCOMPARE(b.formula("B1"), QStringLiteral("SUM(A1:A3)"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:8"));                             // 1 + 2 + 5
        QCOMPARE(b.at("A3"), QStringLiteral("n:5"));
        QCOMPARE(b.formula("B3"), QString());                                    // (B3 and B4 were in the rows that went)
        QCOMPARE(b.formula("B2"), QStringLiteral("#REF!*2"));
        QCOMPARE(b.at("B2"), QStringLiteral("e:#REF!"));
        QCOMPARE(b.at("C1"), QStringLiteral("e:#REF!"));                         // what reads it has the error too
        QCOMPARE(rec.gone.size(), size_t(4));                                    // A3, B3, A4, B4 (B3 has a formula)
        // Undo: all as it was
        QVERIFY(b.calc->undoEdit(rec));
        b.finish();
        QCOMPARE(snapshot(), before);
        // and the edit again
        Calculation::EditRecord again;
        QVERIFY(b.apply(StructEdit::deleteRows(0, 2, 2), &again));
        QCOMPARE(b.formula("B1"), QStringLiteral("SUM(A1:A3)"));
        QVERIFY(b.calc->undoEdit(again));
        b.finish();
        QCOMPARE(snapshot(), before);
    }

    // Columns inserted and deleted, the same way; and cells shifted down, up, right, and left (only the references that lie in the columns (rows) of the block follow).
    void columnsAndShiftedCellsFollowTheSameRules()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "B1=2", "C1=3", "D1=4", "A2=5", "B2=6", "C2=7", "D2=8", "E1==SUM(A1:D1)", "E2==C2+B1", "F1==SUM(A:B)"}}));
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertColumns(0, 1, 1), &rec));              // a column at B
        QCOMPARE(b.formula("F1"), QStringLiteral("SUM(A1:E1)"));                // (E moved to F)
        QCOMPARE(b.formula("F2"), QStringLiteral("D2+C1"));
        QCOMPARE(b.formula("G1"), QStringLiteral("SUM(A:C)"));
        QCOMPARE(b.at("F1"), QStringLiteral("n:10"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:2"));
        QVERIFY(!b.has("B1"));
        QVERIFY(b.calc->undoEdit(rec));
        b.finish();
        QCOMPARE(b.formula("E1"), QStringLiteral("SUM(A1:D1)"));
        QCOMPARE(b.at("E1"), QStringLiteral("n:10"));
        Calculation::EditRecord del;
        QVERIFY(b.apply(StructEdit::deleteColumns(0, 1, 2), &del));              // columns B and C
        QCOMPARE(b.formula("C1"), QStringLiteral("SUM(A1:B1)"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:5"));                             // 1 + 4
        QCOMPARE(b.formula("C2"), QStringLiteral("#REF!+#REF!"));
        QCOMPARE(b.formula("D1"), QStringLiteral("SUM(A:A)"));
        QVERIFY(b.calc->undoEdit(del));
        b.finish();
        QCOMPARE(b.at("E2"), QStringLiteral("n:9"));
        // cells shifted down in B1:C1 (the cells below them in B and C move; the rest do not)
        CellRange block;
        parseRangeRef(QStringLiteral("B1:C1"), &block);
        Calculation::EditRecord down;
        QVERIFY(b.apply(StructEdit::insertCellsDown(0, block), &down));
        QCOMPARE(b.at("B2"), QStringLiteral("n:2"));
        QCOMPARE(b.at("B3"), QStringLiteral("n:6"));
        QCOMPARE(b.at("A2"), QStringLiteral("n:5"));                             // (column A did not move)
        QCOMPARE(b.formula("E2"), QStringLiteral("C3+B2"));                      // (E2 stays, in a column the block is not in: what it reads in B and C moved)
        QCOMPARE(b.at("E2"), QStringLiteral("n:9"));
        QCOMPARE(b.formula("E1"), QStringLiteral("SUM(A1:D1)"));                // (a range across other columns is as it was)
        QVERIFY(b.calc->undoEdit(down));
        b.finish();
        QCOMPARE(b.at("B1"), QStringLiteral("n:2"));
        QCOMPARE(b.at("B2"), QStringLiteral("n:6"));
    }

    // The defined names follow the cells they name, and Undo gives them back.
    void definedNamesFollowTheCells()
    {
        Book b;
        QVERIFY(b.open({{"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "B1==SUM(Total)", "B2==Last*2"}}, QString(),
                       QStringLiteral("<definedName name=\"Total\">Sheet1!$A$1:$A$5</definedName><definedName name=\"Last\">Sheet1!$A$5</definedName>")));
        QCOMPARE(b.at("B1"), QStringLiteral("n:15"));
        QCOMPARE(b.at("B2"), QStringLiteral("n:10"));
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertRows(0, 2, 2), &rec));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Total")), QStringLiteral("Sheet1!$A$1:$A$7"));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Last")), QStringLiteral("Sheet1!$A$7"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:15"));
        QCOMPARE(b.at("B2"), QStringLiteral("n:10"));
        QVERIFY(b.calc->setValue(0, 6, 0, Value::number(50)));                   // A7, what both names stand for part of
        b.finish();
        QCOMPARE(b.at("B2"), QStringLiteral("n:100"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:60"));
        QVERIFY(b.calc->setValue(0, 6, 0, Value::number(5)));
        QVERIFY(b.calc->undoEdit(rec));
        b.finish();
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Total")), QStringLiteral("Sheet1!$A$1:$A$5"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:15"));
        // a row deleted that a name stands for gives #REF!
        Calculation::EditRecord del;
        QVERIFY(b.apply(StructEdit::deleteRows(0, 4, 1), &del));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Last")), QStringLiteral("Sheet1!#REF!"));
        QCOMPARE(b.calc->definedName(0, QStringLiteral("Total")), QStringLiteral("Sheet1!$A$1:$A$4"));
        QCOMPARE(b.at("B2"), QStringLiteral("e:#REF!"));
        QCOMPARE(b.at("B1"), QStringLiteral("n:10"));
    }

    // Formulas that share a text: the cells that move keep their formulas, the references among them follow the cells, and the group is split where the edit is in it.
    void sharedFormulasKeepTheirMeaning()
    {
        QString rows;
        for (int r = 1; r <= 5; ++r) {
            rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c>").arg(r);
            rows += r == 1 ? QStringLiteral("<c r=\"B1\"><f t=\"shared\" ref=\"B1:B5\" si=\"0\">A1*2+$A$1</f><v>3</v></c>")
                           : QStringLiteral("<c r=\"B%1\"><f t=\"shared\" si=\"0\"/><v>0</v></c>").arg(r);
            rows += QStringLiteral("</row>");
        }
        Book b;
        QVERIFY(b.openXml(worksheet(rows)));
        QCOMPARE(b.at("B3"), QStringLiteral("n:7"));
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertRows(0, 2, 1), &rec));
        const QStringList wanted{QStringLiteral("A1*2+$A$1"), QStringLiteral("A2*2+$A$1"), QString(), QStringLiteral("A4*2+$A$1"), QStringLiteral("A5*2+$A$1"), QStringLiteral("A6*2+$A$1")};
        for (int r = 1; r <= 6; ++r) QCOMPARE(b.formula(QStringLiteral("B%1").arg(r)), wanted[r - 1]);
        QCOMPARE(b.at("B6"), QStringLiteral("n:11"));
        QCOMPARE(b.at("B4"), QStringLiteral("n:7"));
        // a formula's own row moves it; deleting the first row (the master's) leaves the others as they were
        Calculation::EditRecord del;
        QVERIFY(b.apply(StructEdit::deleteRows(0, 0, 1), &del));
        QCOMPARE(b.formula("B1"), QStringLiteral("A1*2+#REF!"));
        QCOMPARE(b.formula("B3"), QStringLiteral("A3*2+#REF!"));
        QVERIFY(b.calc->undoEdit(del));
        QVERIFY(b.calc->undoEdit(rec));
        b.finish();
        for (int r = 1; r <= 5; ++r) QCOMPARE(b.formula(QStringLiteral("B%1").arg(r)), QStringLiteral("A%1*2+$A$1").arg(r));
        QCOMPARE(b.at("B5"), QStringLiteral("n:11"));
    }

    // An edit that would cut through an array formula, or push cells off the sheet, is refused (and nothing changes); one that moves the array whole is made, and its range follows.
    void arraysAndTheEdgeOfTheSheetRefuseWhatCannotBeDone()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"C1\"><f t=\"array\" ref=\"C1:C3\">A1:A3*2</f><v>2</v></c></row>"
                                            "<row r=\"2\"><c r=\"A2\"><v>2</v></c><c r=\"C2\"><v>4</v></c></row><row r=\"3\"><c r=\"A3\"><v>3</v></c><c r=\"C3\"><v>6</v></c></row>");
        Book b;
        QVERIFY(b.openXml(worksheet(rows)));
        QString why;
        QVERIFY(!b.calc->canApply(StructEdit::insertRows(0, 1, 1), &why));
        QVERIFY2(why.contains(QLatin1String("array")), qPrintable(why));
        QVERIFY(!b.calc->canApply(StructEdit::deleteRows(0, 1, 1), &why));
        QVERIFY(!b.calc->canApply(StructEdit::deleteRows(0, 0, 3), &why));
        Calculation::EditRecord none;
        QVERIFY(!b.calc->applyEdit(StructEdit::insertRows(0, 2, 1), &none));
        QCOMPARE(b.at("A3"), QStringLiteral("n:3"));
        QVERIFY(b.calc->canApply(StructEdit::insertRows(0, 3, 2), &why));          // below the array
        QVERIFY(b.calc->canApply(StructEdit::insertColumns(0, 0, 1), &why));       // left of it: it moves whole
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 2), &rec));                   // above it, and everything: moves whole
        QCOMPARE(b.at("C3"), QStringLiteral("n:2"));
        QCOMPARE(b.at("C5"), QStringLiteral("n:6"));
        QCOMPARE(b.calc->sheet(0).ranges.first().row1, quint32(2));
        QVERIFY(b.calc->setValue(0, 2, 0, Value::number(10)));                      // A3 (was A1)
        b.finish();
        QCOMPARE(b.at("C3"), QStringLiteral("n:20"));
        // cells at the end of the sheet that hold something are not pushed off it
        Book e;
        QVERIFY(e.open({{"A1=1", "A1048576=9"}}));
        QVERIFY(!e.calc->canApply(StructEdit::insertRows(0, 0, 1), &why));
        QVERIFY2(why.contains(QLatin1String("edge")), qPrintable(why));
        QVERIFY(e.calc->canApply(StructEdit::insertRows(0, 0, 1).inverse(), &why));
        // a protected sheet refuses
        Book p;
        QVERIFY(p.openXml(worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), QStringLiteral("<sheetProtection sheet=\"1\"/>"))));
        QVERIFY(!p.calc->canApply(StructEdit::insertRows(0, 0, 1), &why));
        QVERIFY2(why.contains(QLatin1String("protected")), qPrintable(why));
    }

    // A formula kept apart for a function the engine lacks has the result the file stored until something it reads changes: then it has #NAME?, and so it has when a cell it read is
    // deleted (its text has #REF! in it, and nothing it reads changes any more, so it is given at once); Undo gives back its text.
    void aFormulaKeptApartForItsFunctionKeepsItsResultUnlessACellItReadsIsDeleted()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\"><c r=\"A2\"><v>2</v></c></row><row r=\"3\"><c r=\"C3\"><f>_xll.NOPE(A2)</f><v>7</v></c></row>");
        Book b;
        QVERIFY(b.openXml(worksheet(rows)));
        QCOMPARE(b.at("C3"), QStringLiteral("n:7"));
        Calculation::EditRecord up, gone;
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), &up));   // the cell it reads is A3 now (it moved, which is a change of what it reads: #NAME?)
        QCOMPARE(b.formula("C4"), QStringLiteral("_xll.NOPE(A3)"));
        QCOMPARE(b.at("C4"), QStringLiteral("e:#NAME?"));
        QVERIFY(b.calc->undoEdit(up));
        b.finish();
        QCOMPARE(b.formula("C3"), QStringLiteral("_xll.NOPE(A2)"));
        QVERIFY(b.apply(StructEdit::deleteRows(0, 1, 1), &gone));   // A2 is deleted
        QCOMPARE(b.formula("C2"), QStringLiteral("_xll.NOPE(#REF!)"));
        QCOMPARE(b.at("C2"), QStringLiteral("e:#NAME?"));
        QVERIFY(b.calc->undoEdit(gone));
        b.finish();
        QCOMPARE(b.formula("C3"), QStringLiteral("_xll.NOPE(A2)"));
    }

    // A data table reads its input cells: deleting one is refused (the file cannot say it is gone), and a row or column inserted or deleted elsewhere moves the table's reference with the cell.
    void aDataTablesInputCellIsNotDeleted()
    {
        const QString rows = QStringLiteral("<row r=\"2\"><c r=\"B2\"><v>1</v></c><c r=\"D2\"><f t=\"dataTable\" ref=\"D2:D3\" r1=\"B2\"/><v>5</v></c></row><row r=\"3\"><c r=\"D3\"><v>6</v></c></row>");
        Book b;
        QVERIFY(b.openXml(worksheet(rows)));
        QString why;
        QVERIFY(!b.calc->canApply(StructEdit::deleteRows(0, 1, 1), &why));   // row 2 holds B2
        QVERIFY2(why.contains(QLatin1String("data table")), qPrintable(why));
        QVERIFY(!b.calc->canApply(StructEdit::deleteColumns(0, 0, 3), &why));   // columns A to C hold it
        Calculation::EditRecord none;
        QVERIFY(!b.calc->applyEdit(StructEdit::deleteColumns(0, 1, 1), &none));
        QCOMPARE(b.calc->content(0, 1, 1).value.number(), 1.0);              // (nothing changed)
        // a row above the cell, and a column beside it, are fine: the table's text follows the cell
        QVERIFY(b.calc->canApply(StructEdit::deleteRows(0, 0, 1), &why));
        QVERIFY(b.calc->canApply(StructEdit::deleteColumns(0, 0, 1), &why));
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 2), &rec));
        QCOMPARE(b.calc->content(0, 3, 3).formula, QStringLiteral("TABLE(,B4)"));
    }

    // Where a table is after rows, columns, and cells are inserted and deleted around it: it moves, grows, and shrinks with its rows, and is not cut through.
    void aTableMovesGrowsShrinksOrRefusesTheEdit()
    {
        Table t;
        t.sheet = 0;
        t.row1 = 2;
        t.col1 = 1;
        t.row2 = 6;
        t.col2 = 3;   // B3:D7
        t.headerRows = 1;
        auto fate = [&](const StructEdit &e, QString *place = nullptr) {
            CellRange where;
            QString why;
            const TableFate f = tableAfter(e, t, &where, &why);
            if (place) *place = f == TableFate::Refused ? why : rangeName(where);
            return f;
        };
        QString place;
        QCOMPARE(fate(StructEdit::insertRows(0, 0, 2), &place), TableFate::Moved);
        QCOMPARE(place, QStringLiteral("B5:D9"));
        QCOMPARE(fate(StructEdit::insertRows(0, 2, 1), &place), TableFate::Moved);   // above the header: it moves
        QCOMPARE(place, QStringLiteral("B4:D8"));
        QCOMPARE(fate(StructEdit::insertRows(0, 4, 3), &place), TableFate::Moved);   // inside: it grows
        QCOMPARE(place, QStringLiteral("B3:D10"));
        QCOMPARE(fate(StructEdit::insertRows(0, 7, 3)), TableFate::Same);            // below it
        QCOMPARE(fate(StructEdit::insertRows(1, 0, 3)), TableFate::Same);            // on another sheet
        QCOMPARE(fate(StructEdit::deleteRows(0, 0, 2), &place), TableFate::Moved);
        QCOMPARE(place, QStringLiteral("B1:D5"));
        QCOMPARE(fate(StructEdit::deleteRows(0, 4, 2), &place), TableFate::Moved);   // two data rows
        QCOMPARE(place, QStringLiteral("B3:D5"));
        QCOMPARE(fate(StructEdit::deleteRows(0, 6, 3), &place), TableFate::Moved);   // the last row, and below
        QCOMPARE(place, QStringLiteral("B3:D6"));
        QCOMPARE(fate(StructEdit::deleteRows(0, 2, 1), &place), TableFate::Refused);   // the header
        QVERIFY(place.contains(QStringLiteral("names")));
        QCOMPARE(fate(StructEdit::deleteRows(0, 1, 9)), TableFate::Refused);         // all of it
        QCOMPARE(fate(StructEdit::insertColumns(0, 0, 2), &place), TableFate::Moved);
        QCOMPARE(place, QStringLiteral("D3:F7"));
        QCOMPARE(fate(StructEdit::insertColumns(0, 4, 2)), TableFate::Same);
        QCOMPARE(fate(StructEdit::insertColumns(0, 2, 1)), TableFate::Refused);      // inside
        QCOMPARE(fate(StructEdit::deleteColumns(0, 3, 1)), TableFate::Refused);
        QCOMPARE(fate(StructEdit::deleteColumns(0, 0, 1), &place), TableFate::Moved);
        QCOMPARE(place, QStringLiteral("A3:C7"));
        CellRange block;
        block.valid = true;
        block.row1 = 0;
        block.row2 = 1;
        block.col1 = 1;
        block.col2 = 3;
        QCOMPARE(fate(StructEdit::insertCellsDown(0, block), &place), TableFate::Moved);   // all its columns: it moves whole
        QCOMPARE(place, QStringLiteral("B5:D9"));
        block.col2 = 2;
        QCOMPARE(fate(StructEdit::insertCellsDown(0, block)), TableFate::Refused);         // only some of its columns
        block.row1 = 8;
        block.row2 = 9;
        QCOMPARE(fate(StructEdit::insertCellsDown(0, block)), TableFate::Same);            // below it
        block.row1 = 2;
        block.row2 = 6;
        block.col1 = 4;
        block.col2 = 5;
        QCOMPARE(fate(StructEdit::deleteCellsLeft(0, block)), TableFate::Same);            // right of it: its cells do not move
        block.col1 = 0;
        block.col2 = 0;
        QCOMPARE(fate(StructEdit::deleteCellsLeft(0, block), &place), TableFate::Moved);   // left of it, in all its rows
        QCOMPARE(place, QStringLiteral("A3:C7"));
        block.row2 = 3;
        QCOMPARE(fate(StructEdit::deleteCellsLeft(0, block)), TableFate::Refused);         // in some of its rows
    }

    // The cost is what moves and what reads it: an insert in the middle of a tall sheet moves the cells below it and writes again only the formulas that read them, and the search for
    // those looks at little (counted).
    void theCostIsWhatMovesAndWhatReadsIt()
    {
        QString rows;
        const int n = 100000;
        rows.reserve(n * 60);
        for (int r = 1; r <= n; ++r) {
            rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c>").arg(r);
            if (r == 1) rows += QStringLiteral("<c r=\"B1\"><f>SUM(A1:A100)</f><v>0</v></c><c r=\"C1\"><f>A99999*2</f><v>0</v></c>");
            if (r == 2) rows += QStringLiteral("<c r=\"B2\"><f>A50000*2</f><v>0</v></c>");
            rows += QStringLiteral("</row>");
        }
        Book b;
        QVERIFY(b.openXml(worksheet(rows)));
        const qint64 queried = b.calc->graph().queryWork();
        // an edit at the end of the sheet moves nothing and refers to nothing: it looks at next to nothing, though the sheet has a hundred thousand formulas and cells
        {
            const qint64 work = b.calc->editWork();
            Calculation::EditRecord end;
            QVERIFY(b.apply(StructEdit::insertRows(0, 100500, 1), &end));
            QCOMPARE(end.cellsMoved, qint64(0));
            QCOMPARE(end.formulasRewritten, qint64(0));
            QVERIFY2(b.calc->editWork() - work < 100, qPrintable(QString::number(b.calc->editWork() - work)));
            QVERIFY(b.calc->graph().queryWork() - queried < 2000);
            QVERIFY(b.apply(StructEdit::insertRows(0, 100500, 1).inverse(), &end));
        }
        const qint64 workBefore = b.calc->editWork();
        Calculation::EditRecord rec;
        QVERIFY(b.apply(StructEdit::insertRows(0, 50000, 1), &rec));               // a row at 50,001
        QVERIFY2(b.calc->editWork() - workBefore < qint64(n - 50000) * 2 + 100, qPrintable(QString::number(b.calc->editWork() - workBefore)));   // (the cells that moved, and the one formula)
        QCOMPARE(rec.cellsMoved, qint64(n - 50000));
        QCOMPARE(rec.formulasRewritten, qint64(1));                                 // (C1 reads A99999: B1 reads rows 1 to 100, and B2 reads A50000, which is above)
        QVERIFY2(b.calc->graph().queryWork() - queried < 5000, qPrintable(QString::number(b.calc->graph().queryWork() - queried)));
        QCOMPARE(b.formula("C1"), QStringLiteral("A100000*2"));
        QCOMPARE(b.formula("B2"), QStringLiteral("A50000*2"));
        QCOMPARE(b.at("C1"), QStringLiteral("n:199998"));
        // an insert at the top moves everything, and rewrites the three formulas
        Calculation::EditRecord top;
        QVERIFY(b.apply(StructEdit::insertRows(0, 0, 1), &top));
        QCOMPARE(top.cellsMoved, qint64(n + 3));                                    // (every cell: the column of numbers and the three formulas')
        QCOMPARE(top.formulasRewritten, qint64(3));
        QCOMPARE(b.formula("B2"), QStringLiteral("SUM(A2:A101)"));
        QCOMPARE(b.at("B2"), QStringLiteral("n:%1").arg(5050));
    }

    // What the edits do is what a calculation made again from the cells would give: after each of many random edits (rows, columns, and cells, inserted and deleted, on two sheets)
    // every value is what a whole new calculation of the same cells gives (the graph has not gone stale), and Undo of them all gives back the cells and formulas that were.
    void randomEditsKeepTheGraphRightAndUndoesBack()
    {
        for (quint32 seed = 1; seed <= 24; ++seed) {
            QRandomGenerator rng(seed);
            QStringList first, second;
            for (int r = 1; r <= 12; ++r)
                for (int c = 0; c < 6; ++c) {
                    const QString ref = QString(QLatin1Char(char('A' + c))) + QString::number(r);
                    if (rng.bounded(5) == 0) continue;
                    if (c < 3 || rng.bounded(3) == 0) first << ref + QLatin1Char('=') + QString::number(rng.bounded(100));
                    else {
                        const int kind = rng.bounded(5);
                        const int r1 = 1 + int(rng.bounded(12)), r2 = 1 + int(rng.bounded(12));
                        const QString a = QString(QLatin1Char(char('A' + rng.bounded(3)))) + QString::number(std::min(r1, r2)), z = QString(QLatin1Char(char('A' + rng.bounded(3)))) + QString::number(std::max(r1, r2));
                        const QString formula = kind == 0 ? QStringLiteral("SUM(%1:%2)").arg(a, z) : kind == 1 ? QStringLiteral("%1*2+Sheet2!A1").arg(a) : kind == 2 ? QStringLiteral("MAX(%1,%2)").arg(a, z)
                                              : kind == 3 ? QStringLiteral("SUM(A:A)+%1").arg(a) : QStringLiteral("Sheet2!B%1+$%2").arg(1 + int(rng.bounded(6))).arg(a);
                        first << ref + QStringLiteral("==") + formula;
                    }
                }
            for (int r = 1; r <= 6; ++r) {
                second << QStringLiteral("A%1=%2").arg(r).arg(r * 3);
                second << QStringLiteral("B%1==SUM(Sheet1!A1:A%2)+Sheet1!C%1").arg(r).arg(r + 3);
            }
            Book b;
            QVERIFY2(b.open({first, second}), qPrintable(QString::number(seed)));
            auto snapshot = [&](bool values) {
                QStringList out;
                for (int sheet = 0; sheet < 2; ++sheet)
                    b.calc->sheet(sheet).cells.forEach([&](const Cell &c) {
                        const auto content = b.calc->content(sheet, c.row, c.col);
                        out << QStringLiteral("%1!%2,%3 %4 %5").arg(sheet).arg(c.row).arg(c.col).arg(content.isFormula ? QLatin1Char('=') + content.formula : content.value.debug(), values ? b.calc->cell(sheet, c.row, c.col).debug() : QString());
                    });
                return out.join(QLatin1Char('\n'));
            };
            const QString cellsAtStart = snapshot(false);
            std::vector<Calculation::EditRecord> done;
            for (int step = 0; step < 14; ++step) {
                const quint32 at = quint32(rng.bounded(10)), count = 1 + quint32(rng.bounded(2));
                CellRange block;
                block.valid = true;
                block.row1 = at;
                block.row2 = at + count - 1;
                block.col1 = quint32(rng.bounded(4));
                block.col2 = block.col1 + quint32(rng.bounded(2));
                const int sheet = rng.bounded(4) == 0 ? 1 : 0;
                StructEdit e;
                switch (rng.bounded(8)) {
                case 0: e = StructEdit::insertRows(sheet, at, count); break;
                case 1: e = StructEdit::deleteRows(sheet, at, count); break;
                case 2: e = StructEdit::insertColumns(sheet, at % 6, count); break;
                case 3: e = StructEdit::deleteColumns(sheet, at % 6, count); break;
                case 4: e = StructEdit::insertCellsDown(sheet, block); break;
                case 5: e = StructEdit::deleteCellsUp(sheet, block); break;
                case 6: e = StructEdit::insertCellsRight(sheet, block); break;
                default: e = StructEdit::deleteCellsLeft(sheet, block); break;
                }
                Calculation::EditRecord rec;
                if (!b.calc->canApply(e, nullptr)) continue;
                QVERIFY(b.apply(e, &rec));
                done.push_back(rec);
                // every value is what a calculation made again from these cells gives
                const QString incremental = snapshot(true);
                b.calc->invalidateAll();
                b.finish();
                QCOMPARE(snapshot(true), incremental);
            }
            for (size_t i = done.size(); i-- > 0;) {
                QVERIFY(b.calc->undoEdit(done[i]));
                b.finish();
            }
            QCOMPARE(snapshot(false), cellsAtStart);
        }
    }
};

JP_TEST_CLASS(SheetStructureTests)
#include "tst_sheet_structure.moc"
