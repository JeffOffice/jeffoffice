// Tests of the first-release lines closed in round 13 of JeffCrunch: the built-in themed cell styles, Repeat, and the Undo history list.
#include "app/crunchwindow.h"
#include "core/editsession.h"
#include "core/fonts.h"
#include "core/cellstyles.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "io/zip.h"
#include "savedok.h"
#include "sheet/sheetreader.h"

#include "app/undohistory.h"
#include "../sheet/sheethelpers.h"

#include <QAction>
#include <QFile>
#include <QMenu>
#include <QSignalSpy>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;

namespace {

QColor lookFill(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    const grid::GridSheet &gs = e.view()->sheet();
    return QColor::fromRgb(gs.lookOf(r, c, gs.styleOf(r, c, gs.cellAt(r, c))).fg);
}

grid::CellLook lookOf(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    const grid::GridSheet &gs = e.view()->sheet();
    return gs.lookOf(r, c, gs.styleOf(r, c, gs.cellAt(r, c)));
}

QAction *entryOf(QMenu *menu, const QString &text)
{
    for (QAction *a : menu->actions())
        if (a->text() == text) return a;
    return nullptr;
}

} // namespace

class CrunchLines13Tests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // Each themed built-in style, applied from the gallery, draws its accent (not black), in a blank workbook and in one opened from a text file (which has no named styles of its own).
    void aThemedCellStyleDrawsItsFillWhereverTheWorkbookCameFrom()
    {
        for (int source : {0, 2}) {
            EditWindow e;
            if (source == 0) QVERIFY(e.openBlank());
            else if (source == 1) QVERIFY(e.open(editBook()));
            else QVERIFY(e.open(QByteArray("a,b\n1,2\n"), QStringLiteral("t.csv")));
            QVERIFY(e.settle());
            auto *gallery = e.win.w.findChild<QMenu *>(QStringLiteral("stylesMenu"));
            QVERIFY(gallery);
            for (int a = 1; a <= 6; ++a) {
                for (const QString &name : {QStringLiteral("Good"), QStringLiteral("Accent%1").arg(a), QStringLiteral("20% - Accent%1").arg(a), QStringLiteral("60% - Accent%1").arg(a)}) {
                    e.select(QStringLiteral("E5"));
                    Q_EMIT gallery->aboutToShow();
                    QAction *act = entryOf(gallery, name);
                    QVERIFY2(act, qPrintable(name));
                    act->trigger();
                    const QColor fill = lookFill(e, QStringLiteral("E5"));
                    quint32 rr = 0, cc = 0;
                    parseCellRef(QStringLiteral("E5"), &rr, &cc);
                    const grid::GridSheet &gs = e.view()->sheet();
                    const grid::CellLook look = gs.lookOf(rr, cc, gs.styleOf(rr, cc, gs.cellAt(rr, cc)));
                    QVERIFY2(look.pattern == grid::PatternKind::Solid, qPrintable(QStringLiteral("%1 source %2 has no solid fill").arg(name).arg(source)));
                    QVERIFY2(fill != QColor(Qt::black) && fill.alpha() == 255 && fill.lightness() > 20, qPrintable(QStringLiteral("%1 source %2 gave %3").arg(name).arg(source).arg(fill.name())));
                }
            }
        }
    }

    // A workbook opened from a text file has no named styles of the XML form: the style still gives its look to the cell, as one step of Undo, and the look is in the saved file when
    // the window saves and opens it again.
    void aBuiltInStyleOnATextFileWorkbookIsKeptByTheWindowsSaveAndOpen()
    {
        EditWindow e;
        QVERIFY(e.open(QByteArray("a,b\n1,2\n"), QStringLiteral("t.csv")));
        QVERIFY(e.settle());
        auto *gallery = e.win.w.findChild<QMenu *>(QStringLiteral("stylesMenu"));
        QVERIFY(gallery);
        e.select(QStringLiteral("A2"));
        const int steps = e.win.w.undoStack()->count();
        Q_EMIT gallery->aboutToShow();
        QAction *act = entryOf(gallery, QStringLiteral("Accent1"));
        QVERIFY(act);
        act->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        const QColor drawn = lookFill(e, QStringLiteral("A2"));
        QVERIFY(drawn != QColor(Qt::black));
        e.win.w.undoStack()->undo();
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QVERIFY(lookFill(e, QStringLiteral("A2")) == QColor(Qt::black));
        e.win.w.undoStack()->redo();
        QCOMPARE(lookFill(e, QStringLiteral("A2")), drawn);
        const QString path = e.folder.dir.filePath(QStringLiteral("styled.xlsx"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.saveAs(path) && savedOk(saved));
        EditWindow again;
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        f.close();
        QVERIFY(again.open(bytes));
        QVERIFY(again.settle());
        QCOMPARE(lookFill(again, QStringLiteral("A2")), drawn);
        const grid::GridSheet &gs = again.view()->sheet();
        QVERIFY(gs.lookOf(1, 0, gs.styleOf(1, 0, gs.cellAt(1, 0))).pattern == grid::PatternKind::Solid);
    }

    // Repeat (F4) does the last format command again on the cells selected now, as one step of Undo, and the result is kept by the window's own save and open.
    void repeatDoesTheLastFormatAgainAsOneStep()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        auto *repeat = e.win.w.findChild<QAction *>(QStringLiteral("edit.repeat"));
        QVERIFY(repeat);
        QVERIFY(!repeat->isEnabled());
        e.select(QStringLiteral("A1"));
        const int s0 = e.win.w.undoStack()->count();
        e.win.w.findChild<QAction *>(QStringLiteral("fmt.bold"))->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), s0 + 1);
        QVERIFY(lookOf(e, QStringLiteral("A1")).bold);
        QVERIFY(repeat->isEnabled());
        QVERIFY(repeat->toolTip().contains(QStringLiteral("Bold")));
        e.view()->selectRange(1, 1, 2, 1);   // B2:B3
        const int steps = e.win.w.undoStack()->count();
        repeat->trigger();
        QVERIFY(lookOf(e, QStringLiteral("B2")).bold && lookOf(e, QStringLiteral("B3")).bold);
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        e.win.w.undoStack()->undo();
        QVERIFY(!lookOf(e, QStringLiteral("B2")).bold && !lookOf(e, QStringLiteral("B3")).bold);
        e.win.w.undoStack()->redo();
        QVERIFY(lookOf(e, QStringLiteral("B2")).bold);
        // a style, then Repeat gives the same style to the new selection
        e.select(QStringLiteral("C1"));
        auto *gallery = e.win.w.findChild<QMenu *>(QStringLiteral("stylesMenu"));
        QVERIFY(gallery);
        Q_EMIT gallery->aboutToShow();
        QAction *good = entryOf(gallery, QStringLiteral("Good"));
        QVERIFY(good);
        good->trigger();
        const QRgb first = lookOf(e, QStringLiteral("C1")).fg;
        e.select(QStringLiteral("D1"));
        QVERIFY(repeat->toolTip().contains(QStringLiteral("Cell Style")));
        repeat->trigger();
        QCOMPARE(lookOf(e, QStringLiteral("D1")).fg, first);
        QVERIFY(first != 0);
        // a row inserted, then again at the new selection
        e.select(QStringLiteral("A5"));
        e.win.w.findChild<QAction *>(QStringLiteral("cells.insert.rows"))->trigger();
        const int rowsBefore = e.win.w.undoStack()->count();
        e.select(QStringLiteral("A9"));
        repeat->trigger();
        QCOMPARE(e.win.w.undoStack()->count(), rowsBefore + 1);
        // saved and opened by the window
        QVERIFY(e.settle());
        const QString path = e.folder.dir.filePath(QStringLiteral("repeated.xlsx"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.saveAs(path) && savedOk(saved));
        EditWindow again;
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        f.close();
        QVERIFY(again.open(bytes));
        QVERIFY(again.settle());
        QVERIFY(lookOf(again, QStringLiteral("B2")).bold && lookOf(again, QStringLiteral("B3")).bold);
        QCOMPARE(lookOf(again, QStringLiteral("D1")).fg, first);
    }

    // Ctrl+Y repeats only while there is nothing to redo (while Redo has a step the key is Redo), and a command that was refused is not kept to repeat.
    void ctrlYRepeatsOnlyWhenThereIsNothingToRedo()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        auto *key = e.win.w.findChild<QAction *>(QStringLiteral("edit.repeat.ctrlY"));
        QVERIFY(key);
        e.select(QStringLiteral("A1"));
        e.win.w.findChild<QAction *>(QStringLiteral("fmt.italic"))->trigger();
        QVERIFY(key->isEnabled());
        e.win.w.undoStack()->undo();
        QTRY_VERIFY(!key->isEnabled());   // (a step to redo: Ctrl+Y is Redo)
        e.win.w.undoStack()->redo();
        QTRY_VERIFY(key->isEnabled());
        key->trigger();
        QVERIFY(lookOf(e, QStringLiteral("A1")).italic);
    }

    // The Undo and Redo buttons have a list of steps, newest first: a click on one does it and the steps before it, as the steps themselves.
    void theHistoryListOnUndoDoesTheStepsUpToTheOneChosen()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY(e.settle());
        for (const char *cell : {"A1", "A2", "A3"}) {
            e.select(QString::fromLatin1(cell));
            e.win.w.findChild<QAction *>(QStringLiteral("fmt.bold"))->trigger();
        }
        e.select(QStringLiteral("B1"));
        e.win.w.findChild<QAction *>(QStringLiteral("fmt.italic"))->trigger();
        auto *undoList = e.win.w.findChild<jp::UndoHistoryMenu *>(QStringLiteral("undoHistoryMenu"));
        auto *redoList = e.win.w.findChild<jp::UndoHistoryMenu *>(QStringLiteral("redoHistoryMenu"));
        QVERIFY(undoList && redoList);
        Q_EMIT undoList->aboutToShow();
        QCOMPARE(undoList->shownSteps(), 4);
        undoList->markSteps(3);
        QCOMPARE(undoList->marked(), 3);
        const int top = e.win.w.undoStack()->index();
        undoList->choose(3);
        QCOMPARE(e.win.w.undoStack()->index(), top - 3);
        QVERIFY(!lookOf(e, QStringLiteral("B1")).italic);
        QVERIFY(!lookOf(e, QStringLiteral("A3")).bold && !lookOf(e, QStringLiteral("A2")).bold);
        QVERIFY(lookOf(e, QStringLiteral("A1")).bold);
        Q_EMIT redoList->aboutToShow();
        QCOMPARE(redoList->shownSteps(), 3);
        redoList->choose(2);
        QCOMPARE(e.win.w.undoStack()->index(), top - 1);
        QVERIFY(lookOf(e, QStringLiteral("A2")).bold && lookOf(e, QStringLiteral("A3")).bold);
        QVERIFY(!lookOf(e, QStringLiteral("B1")).italic);
    }

    // The old binary form's list of sheets a formula names writes a reference to a sheet that is not there with the place 0xFFFF (sheet index -1 is packed unsigned, no shift of a negative number),
    // and a 3-D range of sheets and a formula of the other sheet are written beside it; the window's save and open keep them.
    void theOldFormatSaveWritesAReferenceToAMissingSheet()
    {
        EditWindow e;
        const QString one = QStringLiteral("<row r=\"1\"><c r=\"A1\"><f>Gone!A1</f><v>0</v></c><c r=\"B1\"><f>SUM(Sheet1:Sheet2!A2)</f><v>3</v></c><c r=\"C1\"><f>Sheet2!A2*2</f><v>4</v></c></row>"
                                            "<row r=\"2\"><c r=\"A2\"><v>1</v></c></row>");
        const QString two = QStringLiteral("<row r=\"2\"><c r=\"A2\"><v>2</v></c></row>");
        QVERIFY(e.open(buildXlsx({worksheet(one), worksheet(two)}, QByteArray(), QString(), QString(), QByteArray())));
        QVERIFY(e.settle());
        const QString path = e.folder.dir.filePath(QStringLiteral("gone.xls"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        e.win.w.setLossAsker([](const QString &, const QString &) { return CrunchWindow::LossAnswer::Continue; });
        QVERIFY(e.win.w.saveAs(path, true));
        QTRY_VERIFY_WITH_TIMEOUT(!e.win.w.isSaving(), 60000);
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        f.close();
        QVERIFY(bytes.startsWith(QByteArray::fromHex("d0cf11e0a1b11ae1")));
        QVERIFY(bytes.contains(QByteArray::fromHex("0000ffffffff")));   // (the entry of the list for a sheet that is gone: this workbook, from 0xFFFF to 0xFFFF)
        EditWindow again;
        QVERIFY(again.open(bytes, QStringLiteral("gone.xls")));
        QVERIFY(again.settle());
        QCOMPARE(again.valueAt(QStringLiteral("B1")), e.valueAt(QStringLiteral("B1")));
        QCOMPARE(again.valueAt(QStringLiteral("C1")), e.valueAt(QStringLiteral("C1")));
    }
};

JP_TEST_CLASS(CrunchLines13Tests)
#include "tst_crunch_c13lines.moc"
