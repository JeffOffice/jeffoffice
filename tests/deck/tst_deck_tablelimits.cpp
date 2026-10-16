// JeffDeck's tests of the limit of the table commands (the slowest test of the program, alone in its class so that the other tests run beside it).
#include "deckhelpers.h"

class DeckTableLimitsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // A table has at most 256 rows and 256 columns (a file with more is not read back): Insert and Tab stop there and say so, and what is
    // redone is what was done.
    void tableCommandsStopAtTheLimit()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.insertTable(60, 2);
        const QString id = theTable(ed)->id;
        ed->beginChange(QStringLiteral("Grow"));
        while (theTable(ed)->rows < 255) tableInsertRow(ed, theTable(ed), theTable(ed)->rows);
        while (theTable(ed)->cols < 255) tableInsertCol(ed, theTable(ed), theTable(ed)->cols);
        ed->endChange();
        ed->selectCells(id, 254, 254, 254, 254);
        QVERIFY(command(w, "tbl.insBelow"));
        QCOMPARE(theTable(ed)->rows, 256);
        QVERIFY(command(w, "tbl.insRight"));
        QCOMPARE(theTable(ed)->cols, 256);
        ed->undo();
        ed->redo();   // as it was done, not as a file would clamp it
        QCOMPARE(theTable(ed)->cols, 256);
        QCOMPARE(theTable(ed)->rows, 256);
        const int steps = ed->undoStack()->index();
        QSignalSpy status(ed, &Editor::status);
        ed->selectCells(id, 255, 255, 255, 255);
        QVERIFY(command(w, "tbl.insBelow"));
        QVERIFY(command(w, "tbl.insAbove"));
        QVERIFY(command(w, "tbl.insLeft"));
        QVERIFY(command(w, "tbl.insRight"));
        QCOMPARE(theTable(ed)->rows, 256);
        QCOMPARE(theTable(ed)->cols, 256);
        QCOMPARE(ed->undoStack()->index(), steps);
        QCOMPARE(status.size(), 4);
        QVERIFY(status.first().first().toString().contains(QStringLiteral("256")));
        // Tab in the last cell adds no row either.
        ed->beginTextEdit(id, 0, 255, 255);
        QTest::keyClick(w.canvas(), Qt::Key_Tab);
        QCOMPARE(theTable(ed)->rows, 256);
        QCOMPARE(ed->undoStack()->index(), steps);
        // And a damaged span (a file that says a cell is a million rows tall) stops at the table's last row.
        QJsonObject json = theTable(ed)->toJson();
        QJsonArray cells = json["cells"].toArray();
        QJsonObject last = cells.last().toObject();
        last["rs"] = 1000000;
        last["cs"] = 1000000;
        cells[cells.size() - 1] = last;
        json["cells"] = cells;
        TableItem damaged;
        damaged.fromJson(json);
        QCOMPARE(damaged.cell(255, 255).rowSpan, 1);
        QCOMPARE(damaged.cell(255, 255).colSpan, 1);
        QJsonObject middle = cells[255 * 256 + 250].toObject();
        middle["cs"] = 100;
        cells[255 * 256 + 250] = middle;
        json["cells"] = cells;
        damaged.fromJson(json);
        QCOMPARE(damaged.cell(255, 250).colSpan, 6);
    }
};

JP_TEST_CLASS(DeckTableLimitsTests)
#include "tst_deck_tablelimits.moc"
