// Tests of what a person does with a block of cells besides typing in it: AutoSum and the functions of its menu, fill, the clipboard, inserting and deleting, the sizes of rows and
// columns, and the rules of validation. Every workbook is built here, in a temporary folder; each step is one on the undo stack.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QLineEdit>
#include <QSettings>
#include <QSignalSpy>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

// Numbers to total:
//        A    B    C    D
//   1    1    2    x
//   2    3    4
//   3    5    6
//   4
//   5    7    8
QByteArray numbersBook()
{
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c><c r=\"C1\" t=\"inlineStr\"><is><t>x</t></is></c></row>"
                                        "<row r=\"2\"><c r=\"A2\"><v>3</v></c><c r=\"B2\"><v>4</v></c></row>"
                                        "<row r=\"3\"><c r=\"A3\"><v>5</v></c><c r=\"B3\"><v>6</v></c></row>"
                                        "<row r=\"5\"><c r=\"A5\"><v>7</v></c><c r=\"B5\"><v>8</v></c></row>");
    return buildXlsx({worksheet(rows)});
}

} // namespace

class CrunchOpsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        static QTemporaryDir settings;
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        setAppInfo(crunchAppInfo());
        initCore();
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    // ---- AutoSum

    // AutoSum on one cell below numbers puts =SUM over the numbers above it in the cell to be edited (Enter keeps it); with none above it takes the numbers to its left; with neither it
    // gives =SUM() with the cursor inside the brackets. The keys are the platform's.
    void autoSumGuessesTheRangeOfNumbers()
    {
        EditWindow e;
        QVERIFY(e.open(numbersBook()));
        QAction *sum = e.win.w.act(QStringLiteral("edit.autosum"));
        QVERIFY(sum && sum->isEnabled());
        QCOMPARE(sum->shortcut().toString(QKeySequence::NativeText), crunch::autoSumKey().toString(QKeySequence::NativeText));
        e.select(QStringLiteral("A4"));
        sum->trigger();
        QVERIFY(e.win.w.isEditing());
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(A1:A3)"));
        QCOMPARE(e.selected(), QStringLiteral("A4"));
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:9"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        // the numbers above stop at a blank cell or a text: A5 has the blank A4 above it... now filled, so A6 sums A1:A5
        e.select(QStringLiteral("A6"));
        sum->trigger();
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(A1:A5)"));
        e.key(Qt::Key_Escape);
        // a cell with numbers to its left only
        e.select(QStringLiteral("C5"));
        sum->trigger();
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(A5:B5)"));
        e.key(Qt::Key_Escape);
        // a text above stops the guess, and the numbers to the left are taken
        e.select(QStringLiteral("C2"));
        sum->trigger();
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(A2:B2)"));
        e.key(Qt::Key_Escape);
        // nothing to guess: the brackets, with the cursor between them
        e.select(QStringLiteral("F9"));
        sum->trigger();
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM()"));
        QCOMPARE(e.win.w.cellEditor()->cursorPosition(), 5);
        e.key(Qt::Key_Escape);
        // and the keys do it
        e.select(QStringLiteral("B4"));
        const QKeyCombination combo = crunch::autoSumKey()[0];
        QTest::keyClick(&e.win.w, combo.key(), combo.keyboardModifiers());
        QVERIFY(e.win.w.isEditing());
        QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=SUM(B1:B3)"));
        e.key(Qt::Key_Escape);
    }

    // The menu of AutoSum: Average, Count Numbers, Max, and Min work the same way.
    void autoSumsMenuHasTheOtherFunctions()
    {
        EditWindow e;
        QVERIFY(e.open(numbersBook()));
        const QList<QPair<QString, QString>> functions = {{QStringLiteral("edit.autosum.average"), QStringLiteral("=AVERAGE(B1:B3)")},
                                                          {QStringLiteral("edit.autosum.count"), QStringLiteral("=COUNT(B1:B3)")},
                                                          {QStringLiteral("edit.autosum.max"), QStringLiteral("=MAX(B1:B3)")},
                                                          {QStringLiteral("edit.autosum.min"), QStringLiteral("=MIN(B1:B3)")},
                                                          {QStringLiteral("edit.autosum.sum"), QStringLiteral("=SUM(B1:B3)")}};
        for (const auto &f : functions) {
            e.select(QStringLiteral("B4"));
            QAction *a = e.win.w.act(f.first);
            QVERIFY2(a && a->isEnabled(), qPrintable(f.first));
            a->trigger();
            QVERIFY(e.win.w.isEditing());
            QCOMPARE(e.win.w.cellEditor()->text(), f.second);
            e.key(Qt::Key_Escape);
        }
        e.select(QStringLiteral("B4"));
        e.win.w.act(QStringLiteral("edit.autosum.average"))->trigger();
        e.key(Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B4")), QStringLiteral("n:4"));
    }

    // A block that is selected gets its totals under it (a column each) in one step, or to the right when it is one row; a last row (column) of the selection that is empty is where they go.
    void autoSumOfABlockPutsTheTotalsAtItsEnd()
    {
        EditWindow e;
        QVERIFY(e.open(numbersBook()));
        e.view()->selectRange(0, 0, 2, 1);                    // A1:B3
        e.win.w.act(QStringLiteral("edit.autosum"))->trigger();
        QVERIFY(!e.win.w.isEditing());
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:9"));
        QCOMPARE(e.valueAt(QStringLiteral("B4")), QStringLiteral("n:12"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("AutoSum"));
        pressUndo(&e.win.w);
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("B4")), QStringLiteral("-"));
        // a selection whose last row is empty: the totals go there
        e.view()->selectRange(0, 0, 3, 1);                    // A1:B4
        e.win.w.act(QStringLiteral("edit.autosum.max"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:5"));
        QCOMPARE(e.valueAt(QStringLiteral("B4")), QStringLiteral("n:6"));
        // one row: the total goes to the right
        e.view()->selectRange(4, 0, 4, 1);                    // A5:B5
        e.win.w.act(QStringLiteral("edit.autosum"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C5")), QStringLiteral("n:15"));
        // a column of text and nothing to total changes nothing
        const int steps = e.win.w.undoStack()->count();
        e.view()->selectRange(0, 2, 0, 3);                    // C1:D1 (a text, and nothing)
        e.win.w.act(QStringLiteral("edit.autosum"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps);
    }
};

JP_TEST_CLASS(CrunchOpsTests)
#include "tst_crunch_ops.moc"
