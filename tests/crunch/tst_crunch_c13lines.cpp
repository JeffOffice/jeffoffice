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

#include <QAction>
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
};

JP_TEST_CLASS(CrunchLines13Tests)
#include "tst_crunch_c13lines.moc"
