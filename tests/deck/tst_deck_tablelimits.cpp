// JeffDeck's tests of the limit of the table commands (the slowest test of the program, alone in its class so that the other tests run beside it).
#include "deckhelpers.h"

class DeckTableLimitsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // A table has at most 256 rows and 256 columns (a file with more is not read back): Insert and Tab stop there and say so, and what is
    // redone is what was done. The two limits do not depend on each other, so each is tried on a table that is 255 long one way and 2 the other way (a table of 255 by 255 made the
    // one test take 61 s).
    void rowsStopAtTheLimit()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.insertTable(2, 2);
        const QString id = theTable(ed)->id;
        ed->beginChange(QStringLiteral("Grow"));
        while (theTable(ed)->rows < 255) tableInsertRow(ed, theTable(ed), theTable(ed)->rows);
        ed->endChange();
        ed->selectCells(id, 254, 1, 254, 1);
        QVERIFY(command(w, "tbl.insBelow"));
        QCOMPARE(theTable(ed)->rows, 256);
        ed->undo();
        ed->redo();   // as it was done, not as a file would clamp it
        QCOMPARE(theTable(ed)->rows, 256);
        const int steps = ed->undoStack()->index();
        QSignalSpy status(ed, &Editor::status);
        ed->selectCells(id, 255, 1, 255, 1);
        QVERIFY(command(w, "tbl.insBelow"));
        QVERIFY(command(w, "tbl.insAbove"));
        QCOMPARE(theTable(ed)->rows, 256);
        QCOMPARE(ed->undoStack()->index(), steps);
        QCOMPARE(status.size(), 2);
        QVERIFY(status.first().first().toString().contains(QStringLiteral("256")));
        // Tab in the last cell adds no row either.
        ed->beginTextEdit(id, 0, 255, 1);
        QTest::keyClick(w.canvas(), Qt::Key_Tab);
        QCOMPARE(theTable(ed)->rows, 256);
        QCOMPARE(ed->undoStack()->index(), steps);
        // And a damaged span (a file that says a cell is a million rows tall) stops at the table's last row.
        QJsonObject json = theTable(ed)->toJson();
        QJsonArray cells = json["cells"].toArray();
        QJsonObject last = cells.last().toObject();
        last["rs"] = 1000000;
        cells[cells.size() - 1] = last;
        json["cells"] = cells;
        TableItem damaged;
        damaged.fromJson(json);
        QCOMPARE(damaged.cell(255, 1).rowSpan, 1);
    }

    void columnsStopAtTheLimit()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.insertTable(2, 2);
        const QString id = theTable(ed)->id;
        ed->beginChange(QStringLiteral("Grow"));
        while (theTable(ed)->cols < 255) tableInsertCol(ed, theTable(ed), theTable(ed)->cols);
        ed->endChange();
        ed->selectCells(id, 1, 254, 1, 254);
        QVERIFY(command(w, "tbl.insRight"));
        QCOMPARE(theTable(ed)->cols, 256);
        ed->undo();
        ed->redo();
        QCOMPARE(theTable(ed)->cols, 256);
        const int steps = ed->undoStack()->index();
        QSignalSpy status(ed, &Editor::status);
        ed->selectCells(id, 1, 255, 1, 255);
        QVERIFY(command(w, "tbl.insLeft"));
        QVERIFY(command(w, "tbl.insRight"));
        QCOMPARE(theTable(ed)->cols, 256);
        QCOMPARE(ed->undoStack()->index(), steps);
        QCOMPARE(status.size(), 2);
        QVERIFY(status.first().first().toString().contains(QStringLiteral("256")));
        // A damaged span (a file that says a cell is a million columns wide) stops at the table's last column, and one of a hundred stops at the edge too.
        QJsonObject json = theTable(ed)->toJson();
        QJsonArray cells = json["cells"].toArray();
        QJsonObject last = cells.last().toObject();
        last["cs"] = 1000000;
        cells[cells.size() - 1] = last;
        json["cells"] = cells;
        TableItem damaged;
        damaged.fromJson(json);
        QCOMPARE(damaged.cell(1, 255).colSpan, 1);
        QJsonObject middle = cells[1 * 256 + 250].toObject();
        middle["cs"] = 100;
        cells[1 * 256 + 250] = middle;
        json["cells"] = cells;
        damaged.fromJson(json);
        QCOMPARE(damaged.cell(1, 250).colSpan, 6);
    }
};

JP_TEST_CLASS(DeckTableLimitsTests)
#include "tst_deck_tablelimits.moc"
