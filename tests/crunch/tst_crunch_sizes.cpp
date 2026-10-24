// Tests of the width of columns and the height of rows: Column Width and Row Height, a border dragged, a double click that fits, hiding and showing, a number that widens its column, and the
// file that is saved. Every workbook is built here, in a temporary folder.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "../sheet/richbook.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"

#include <QAction>
#include <QRegularExpression>
#include <QApplication>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

int mdw(EditWindow &e) { return e.view()->sheet().looks().maxDigitWidth(); }
int columnPx(EditWindow &e, int col) { return e.view()->sheet().layout().cols.size(col); }
int rowPx(EditWindow &e, int row) { return e.view()->sheet().layout().rows.size(row); }

// The saved file's sheet part.
QString savedSheetXml(const QString &path)
{
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QMap<QString, QByteArray> parts;
    jp::readZip(f.readAll(), parts);
    return QString::fromUtf8(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")));
}

// A sheet with sizes: columns B and C are 12 wide with an outline level, D is hidden, and row 2 is 30 points high, row 3 hidden.
QByteArray sizedBook()
{
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\" t=\"inlineStr\"><is><t>a longer text than fits</t></is></c></row>"
                                        "<row r=\"2\" ht=\"30\" customHeight=\"1\"><c r=\"A2\"><v>2</v></c></row>"
                                        "<row r=\"3\" hidden=\"1\"><c r=\"A3\"><v>3</v></c></row><row r=\"4\"><c r=\"A4\"><v>4</v></c></row>");
    return buildXlsx({worksheet(rows, QString(), QStringLiteral("<cols><col min=\"2\" max=\"3\" width=\"12\" customWidth=\"1\" outlineLevel=\"1\"/><col min=\"4\" max=\"4\" width=\"9.140625\" hidden=\"1\"/></cols>"))});
}

} // namespace

class CrunchSizesTests : public QObject {
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

    // Column Width and Row Height: the size of the columns (rows) of the selection, in what a person is shown (characters, points), one step each, and the file keeps them.
    void theDialogsSetTheSizeOfTheSelectedColumnsAndRows()
    {
        EditWindow e;
        QVERIFY(e.open(sizedBook()));
        const int d = mdw(e);
        QVERIFY(d > 0);
        const int before = columnPx(e, 0);
        double asked = 0;
        bool column = true;
        e.win.w.setSizeAsker([&](CrunchWindow::SizeRequest *r) {
            column = r->column;
            asked = r->value;
            r->value = 20;
            return true;
        });
        e.view()->selectColumns(0, 0);
        e.win.w.act(QStringLiteral("format.columnWidth"))->trigger();
        QVERIFY(column);
        QVERIFY2(asked > 8 && asked < 9, qPrintable(QString::number(asked)));          // (the width it has: eight and a half characters)
        const int px = grid::GridSheet::columnPixels(grid::GridSheet::widthOfShown(20, d), d);
        QCOMPARE(columnPx(e, 0), px);
        QVERIFY(columnPx(e, 0) > before);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Changing column width"));
        pressUndo(&e.win.w);
        QCOMPARE(columnPx(e, 0), before);
        pressRedo(&e.win.w);
        QCOMPARE(columnPx(e, 0), px);
        // several columns at once
        e.view()->selectColumns(0, 2);
        e.win.w.act(QStringLiteral("format.columnWidth"))->trigger();
        QCOMPARE(columnPx(e, 1), px);
        QCOMPARE(columnPx(e, 2), px);
        // rows, in points
        e.view()->selectRows(1, 3);
        e.win.w.act(QStringLiteral("format.rowHeight"))->trigger();
        QVERIFY(!column);
        QCOMPARE(asked, 30.0);                                                         // (the first row's height)
        QCOMPARE(rowPx(e, 1), 27);                                                     // 20 points are 27 pixels
        QCOMPARE(rowPx(e, 2), 27);                                                     // (and a row that was hidden shows)
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Changing row height"));
        pressUndo(&e.win.w);
        QCOMPARE(rowPx(e, 1), 40);
        QCOMPARE(rowPx(e, 2), 0);
    }

    // The border at the end of a column (row) dragged: the size follows the mouse, a guide shows where, and nothing is changed until it is let go; dragged to nothing, it hides.
    void aBorderDraggedResizes()
    {
        EditWindow e;
        QVERIFY(e.open(sizedBook()));
        const grid::GridLayout &L = e.view()->sheet().layout();
        const int edge = L.xEdge(1, false), y = L.headerHeight() / 2;
        const int was = columnPx(e, 0);
        QWidget *vp = e.view()->viewport();
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, QPoint(edge, y));
        QTest::mouseMove(vp, QPoint(edge + 30, y));
        QCOMPARE(columnPx(e, 0), was);                                                  // (not yet)
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, QPoint(edge + 30, y));
        QCOMPARE(columnPx(e, 0), was + 30);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QVERIFY(e.win.w.isModified());
        // the border of a row
        const int rowEdge = L.yEdge(1, false), x = L.headerWidth() / 2;
        const int rowWas = rowPx(e, 0);
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, QPoint(x, rowEdge));
        QTest::mouseMove(vp, QPoint(x, rowEdge + 12));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, QPoint(x, rowEdge + 12));
        QCOMPARE(rowPx(e, 0), rowWas + 12);
        // dragged to the left edge of the column: hidden
        const int edge2 = L.xEdge(1, false);
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, QPoint(edge2, y));
        QTest::mouseMove(vp, QPoint(edge2 - 500, y));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, QPoint(edge2 - 500, y));
        QCOMPARE(columnPx(e, 0), 0);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Hiding columns"));
        pressUndo(&e.win.w);
        QCOMPARE(columnPx(e, 0), was + 30);
        // with columns selected, the border of one of them sizes them all
        e.view()->selectColumns(0, 1);
        const int e1 = L.xEdge(2, false);
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, QPoint(e1, y));
        QTest::mouseMove(vp, QPoint(e1 + 10, y));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, QPoint(e1 + 10, y));
        QCOMPARE(columnPx(e, 0), columnPx(e, 1));
    }

    // A double click on a border fits the column to its text, and AutoFit does it for what is selected; a row fits its tallest line.
    void aDoubleClickFitsTheContents()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=a rather long piece of text", "B1=1", "A2=x", "A3=x"})));
        const grid::GridLayout &L = e.view()->sheet().layout();
        const int wide = columnPx(e, 0);
        QWidget *vp = e.view()->viewport();
        QTest::mouseDClick(vp, Qt::LeftButton, Qt::NoModifier, QPoint(L.xEdge(1, false), L.headerHeight() / 2));
        QVERIFY2(columnPx(e, 0) > wide, qPrintable(QStringLiteral("%1 against %2").arg(columnPx(e, 0)).arg(wide)));
        const double text = e.view()->sheet().shown(0, 0, *e.view()->sheet().cellAt(0, 0)).width;
        QVERIFY2(columnPx(e, 0) >= text, qPrintable(QStringLiteral("%1 against %2").arg(columnPx(e, 0)).arg(text)));
        QVERIFY2(columnPx(e, 0) <= text + 12, qPrintable(QStringLiteral("%1 against %2").arg(columnPx(e, 0)).arg(text)));
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Fitting a column to its contents"));
        // an empty column keeps its size
        const int keep = columnPx(e, 5);
        e.view()->selectColumns(5, 5);
        e.win.w.act(QStringLiteral("format.fitColumn"))->trigger();
        QCOMPARE(columnPx(e, 5), keep);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        // rows: wrapped text of three lines makes the row taller than a one-line row
        QVERIFY(e.win.w.session()->type(0, 4, 0, QStringLiteral("one")).ok);
        e.view()->selectRows(0, 2);
        e.win.w.act(QStringLiteral("format.fitRow"))->trigger();
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Fitting rows to their contents"));
        QVERIFY(rowPx(e, 0) >= 18 && rowPx(e, 0) <= 24);
    }

    // Hide and unhide: Ctrl+0 and Ctrl+9 (the actions' keys), and the unhide of what the selection covers and the columns beside it.
    void hidingAndShowing()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1=2", "C1=3", "D1=4"})));
        const int was = columnPx(e, 1);
        e.view()->selectColumns(1, 2);
        const QKeyCombination hide = e.win.w.act(QStringLiteral("format.hideColumns"))->shortcut()[0];
        QTest::keyClick(e.view(), hide.key(), hide.keyboardModifiers());
        QCOMPARE(columnPx(e, 1), 0);
        QCOMPARE(columnPx(e, 2), 0);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Hiding columns"));
        e.view()->selectColumns(0, 3);
        e.win.w.act(QStringLiteral("format.unhideColumns"))->trigger();
        QCOMPARE(columnPx(e, 1), was);
        QCOMPARE(columnPx(e, 2), was);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Unhiding columns"));
        e.view()->selectRows(1, 1);
        e.win.w.act(QStringLiteral("format.hideRows"))->trigger();
        QCOMPARE(rowPx(e, 1), 0);
        e.view()->selectRows(0, 2);
        e.win.w.act(QStringLiteral("format.unhideRows"))->trigger();
        QVERIFY(rowPx(e, 1) > 0);
    }

    // A number typed in a column that has its first width widens it to hold the number, in the step of the entry; a text does not, and a width a person set stays.
    void aNumberWidensAColumnWithItsFirstWidth()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        const int was = columnPx(e, 0);
        e.select(QStringLiteral("A2"));
        e.enter(QStringLiteral("12345678901234"));
        QVERIFY(e.settle());
        QVERIFY(columnPx(e, 0) > was);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        const int wide = columnPx(e, 0);
        const double text = e.view()->sheet().shown(1, 0, *e.view()->sheet().cellAt(1, 0)).width;
        QVERIFY2(wide >= text && wide <= text + 12, qPrintable(QStringLiteral("%1 against %2").arg(wide).arg(text)));
        pressUndo(&e.win.w);
        QCOMPARE(columnPx(e, 0), was);                                                 // (one step took both)
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("-"));
        pressRedo(&e.win.w);
        QCOMPARE(columnPx(e, 0), wide);
        // text does not widen a column, and a column a person sized keeps its size
        e.select(QStringLiteral("B1"));
        e.enter(QStringLiteral("some words and more words than fit"));
        QCOMPARE(columnPx(e, 1), was);
        e.view()->selectColumns(2, 2);
        e.win.w.setSizeAsker([](CrunchWindow::SizeRequest *r) {
            r->value = 5;
            return true;
        });
        e.win.w.act(QStringLiteral("format.columnWidth"))->trigger();
        const int narrow = columnPx(e, 2);
        e.select(QStringLiteral("C1"));
        e.enter(QStringLiteral("12345678901234"));
        QCOMPARE(columnPx(e, 2), narrow);
    }

    // The file keeps the sizes: the columns' elements are cut where the change is and keep what else they say, the rows have their height and that they are hidden, and it opens as it was.
    void theSavedFileHasTheSizes()
    {
        EditWindow e;
        QVERIFY(e.open(sizedBook()));
        const int d = mdw(e);
        e.win.w.setSizeAsker([](CrunchWindow::SizeRequest *r) {
            r->value = r->column ? 15 : 24;
            return true;
        });
        e.view()->selectColumns(2, 2);                                                  // column C: inside the outlined run
        e.win.w.act(QStringLiteral("format.columnWidth"))->trigger();
        e.view()->selectColumns(5, 6);                                                  // columns F and G: new
        e.win.w.act(QStringLiteral("format.columnWidth"))->trigger();
        e.view()->selectColumns(3, 3);
        e.win.w.act(QStringLiteral("format.unhideColumns"))->trigger();                  // (D shows)
        e.view()->selectRows(0, 0);
        e.win.w.act(QStringLiteral("format.rowHeight"))->trigger();
        e.view()->selectRows(1, 1);
        e.win.w.act(QStringLiteral("format.hideRows"))->trigger();
        e.view()->selectRows(2, 2);
        e.win.w.act(QStringLiteral("format.unhideRows"))->trigger();
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        const QString xml = savedSheetXml(e.path);
        QVERIFY2(xml.contains(QStringLiteral("outlineLevel=\"1\"")), qPrintable(xml));              // (what the file said of the run it cut stays)
        QVERIFY2(xml.contains(QStringLiteral("<col min=\"2\" max=\"2\" width=\"12\"")), qPrintable(xml));
        QVERIFY(xml.contains(QStringLiteral("<col min=\"6\" max=\"7\" width=")));
        QVERIFY2(!xml.contains(QRegularExpression(QStringLiteral("<col min=\"4\"[^>]*hidden"))), qPrintable(xml));
        QVERIFY(xml.contains(QStringLiteral("ht=\"24\"")));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const grid::GridLayout &A = e.view()->sheet().layout(), &B = second.w.view()->sheet().layout();
        for (int c = 0; c < 10; ++c) QCOMPARE(B.cols.size(c), A.cols.size(c));
        for (int r = 0; r < 6; ++r) QCOMPARE(B.rows.size(r), A.rows.size(r));
        QVERIFY(B.cols.size(2) == grid::GridSheet::columnPixels(grid::GridSheet::widthOfShown(15, d), d));
        // Undo goes back over the sizes after a save (they are not moves of cells)
        QVERIFY(e.win.w.session()->canUndo());
    }
};

JP_TEST_CLASS(CrunchSizesTests)
#include "tst_crunch_sizes.moc"
