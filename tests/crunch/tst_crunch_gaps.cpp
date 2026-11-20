// Tests of the everyday editing gaps closed in round A: array formulas entered with Ctrl+Shift+Enter, selecting more than one range, the status bar's words, and notes' boxes. Every workbook is built here
#include "../cmdsearchcheck.h"
#include "app/celleditor.h"
#include "app/settings.h"
#include "app/sheettabs.h"
#include "app/crunchwindow.h"
#include "app/anchordialogs.h"
#include "app/finddialog.h"
#include "core/editsession.h"
#include "core/notebox.h"
#include "core/printjob.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "findbooks.h"
#include "grid/gridview.h"
#include "office/package.h"
#include "savedok.h"
#include "sheet/notes.h"
#include "sheet/pagesetup.h"

#include <QAccessible>
#include <QAction>
#include <QDateTime>
#include <QFocusEvent>
#include <QImage>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTest>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;

namespace {

QByteArray savedSheet(const QString &path)
{
    jp::office::Package p;
    QString why;
    if (!p.readLazyFile(path, &why)) return QByteArray();
    return p.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
}

// The middle of a cell, in the grid's viewport.
QPoint middleOf(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return e.view()->sheet().layout().cellRect(r, c).center();
}

QString formulaOf(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return e.win.w.session()->editText(0, r, c);
}

void clickCell(EditWindow &e, const QString &ref, Qt::KeyboardModifiers m = Qt::NoModifier) { QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, m, middleOf(e, ref)); }

// Presses at `from`, moves to `to`, and lets go (a drag), with the modifiers held at the press.
void dragFromTo(EditWindow &e, const QPoint &from, const QPoint &to, Qt::KeyboardModifiers m = Qt::NoModifier)
{
    QWidget *vp = e.view()->viewport();
    QTest::mousePress(vp, Qt::LeftButton, m, from);
    QTest::mouseMove(vp, to);
    QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, to);
}

// A point on the border of a cell's block: just inside its left edge, halfway down.
QPoint onLeftBorder(EditWindow &e, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    const QRect rect = e.view()->sheet().layout().cellRect(r, c);
    return QPoint(rect.left() + 1, rect.center().y());
}

const char *kBoxComments =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors><author>Ann</author></authors><commentList>"
    "<comment ref=\"C3\" authorId=\"0\" shapeId=\"0\"><text><t>Check the total, please, and say so</t></text></comment>"
    "<comment ref=\"A1\" authorId=\"0\" shapeId=\"0\"><text><t>Plain</t></text></comment></commentList></comments>";

const char *kBoxVml =
    "<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:x=\"urn:schemas-microsoft-com:office:excel\">"
    "<v:shapetype id=\"_x0000_t202\" coordsize=\"21600,21600\" o:spt=\"202\" path=\"m,l,21600r21600,l21600,xe\"><v:stroke joinstyle=\"miter\"/><v:path gradientshapeok=\"t\" o:connecttype=\"rect\"/></v:shapetype>"
    "<v:shape id=\"_x0000_s1025\" type=\"#_x0000_t202\" style=\"position:absolute;width:108pt;height:59pt;z-index:1;visibility:hidden\"><x:ClientData ObjectType=\"Note\"><x:Anchor>1, 15, 0, 2, 3, 15, 3, 16</x:Anchor>"
    "<x:Row>0</x:Row><x:Column>0</x:Column></x:ClientData></v:shape>"
    "<v:shape id=\"_x0000_s1026\" type=\"#_x0000_t202\" style=\"position:absolute;width:108pt;height:59pt;z-index:2;visibility:visible\"><x:ClientData ObjectType=\"Note\"><x:Anchor>3, 15, 1, 2, 5, 15, 4, 16</x:Anchor>"
    "<x:Row>2</x:Row><x:Column>2</x:Column><x:Visible/></x:ClientData></v:shape></xml>";

// The same two notes, but C3's box is anchored 100,000 cells and pixels before its cell and ends as far after it: the file's own numbers, at their largest.
const char *kHostileVml =
    "<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:x=\"urn:schemas-microsoft-com:office:excel\">"
    "<v:shapetype id=\"_x0000_t202\" coordsize=\"21600,21600\" o:spt=\"202\" path=\"m,l,21600r21600,l21600,xe\"><v:stroke joinstyle=\"miter\"/><v:path gradientshapeok=\"t\" o:connecttype=\"rect\"/></v:shapetype>"
    "<v:shape id=\"_x0000_s1026\" type=\"#_x0000_t202\" style=\"position:absolute;width:108pt;height:59pt;z-index:2;visibility:visible\"><x:ClientData ObjectType=\"Note\">"
    "<x:Anchor>-100000, -100000, -100000, -100000, 100000, 100000, 100000, 100000</x:Anchor><x:Row>2</x:Row><x:Column>2</x:Column><x:Visible/></x:ClientData></v:shape></xml>";

// A1:C3 hold numbers; a note on A1 (hidden) and one on C3 (shown, its box from D2 to F5).
QByteArray bookWithBoxes(const QByteArray &drawing = QByteArray(kBoxVml))
{
    QVector<ExtraPart> parts;
    parts.push_back({QStringLiteral("xl/comments1.xml"), QByteArray(kBoxComments), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml"), 0, QStringLiteral("rId11"),
                     QStringLiteral("comments"), QStringLiteral("../comments1.xml")});
    parts.push_back({QStringLiteral("xl/drawings/vmlDrawing1.vml"), drawing, QStringLiteral("application/vnd.openxmlformats-officedocument.vmlDrawing"), 0, QStringLiteral("rId12"),
                     QStringLiteral("vmlDrawing"), QStringLiteral("../drawings/vmlDrawing1.vml")});
    QMap<QString, QByteArray> all;
    jp::readZip(withParts(editBook(), parts), all);
    QByteArray sheet = all.value(QStringLiteral("xl/worksheets/sheet1.xml"));
    sheet.replace("</worksheet>", "<legacyDrawing r:id=\"rId12\"/></worksheet>");
    all[QStringLiteral("xl/worksheets/sheet1.xml")] = sheet;
    jp::ZipWriter z;
    for (auto it = all.begin(); it != all.end(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

QByteArray savedPart(const QString &path, const QString &name)
{
    jp::office::Package p;
    QString why;
    return p.readLazyFile(path, &why) ? p.part(name) : QByteArray();
}

// Counts the announcements an accessibility client would get (the update handler sees every event, whether or not a client is there); one at a time.
struct Announcements {
    static inline QStringList words;
    static void see(QAccessibleEvent *event)
    {
        if (event->type() == QAccessible::Announcement) words << static_cast<QAccessibleAnnouncementEvent *>(event)->message();
    }
    Announcements() { words.clear(); previous = QAccessible::installUpdateHandler(&see); }
    ~Announcements() { QAccessible::installUpdateHandler(previous); }
    QAccessible::UpdateHandler previous = nullptr;
};

}   // namespace

class CrunchGapTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    // Ctrl+Shift+Enter over a selected block makes one array formula for it: shown in braces in the bar for each of its cells, worked out for the block, one step of Undo.
    void ctrlShiftEnterMakesOneArrayOverTheSelection()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:4"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:6"));
        const auto made = e.win.w.session()->arrayAt(0, 1, 2);
        QVERIFY(made.valid);
        QCOMPARE(int(made.row1), 0);
        QCOMPARE(int(made.row2), 2);
        QCOMPARE(e.view()->rangeName(), QStringLiteral("C1:C3"));
        for (const char *ref : {"C1", "C2", "C3"}) {
            e.select(QString::fromLatin1(ref));
            QCOMPARE(e.view()->activeCellText(), QStringLiteral("{=A1:A3*2}"));
        }
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("-"));
        QVERIFY(!e.win.w.session()->arrayAt(0, 1, 2).valid);
        pressRedo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:6"));
        QVERIFY(e.win.w.session()->arrayAt(0, 2, 2).valid);
    }

    // An array is edited as a whole from any of its cells (Enter alone says part of it can't be changed), and deleted as a whole; part of it is refused.
    void anArrayIsEditedAndDeletedAsAWhole()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        // part of it can't be typed over or cleared
        e.select(QStringLiteral("C2"));
        e.enter(QStringLiteral("5"));
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("part of an array")), qPrintable(e.win.told.join('|')));
        e.key(Qt::Key_Escape);
        e.win.told.clear();
        e.view()->selectRange(0, 2, 1, 2);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("part of an array")), qPrintable(e.win.told.join('|')));
        QVERIFY(e.win.w.session()->arrayAt(0, 0, 2).valid);
        // one cell of it, F2, a new formula, Ctrl+Shift+Enter: the whole array has it
        e.select(QStringLiteral("C2"));
        QTest::keyClick(e.view(), Qt::Key_F2);
        QVERIFY(e.win.w.cellEditor()->isVisible());
        e.win.w.cellEditor()->setText(QStringLiteral("=A1:A3*3"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:3"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:9"));
        QCOMPARE(e.win.w.undoStack()->count(), 2);
        // the whole block selected, Delete clears it all, as one step that Undo takes back
        e.view()->selectRange(0, 2, 2, 2);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(e.settle());
        for (const char *ref : {"C1", "C2", "C3"}) QCOMPARE(e.valueAt(QString::fromLatin1(ref)), QStringLiteral("-"));
        QVERIFY(!e.win.w.session()->arrayAt(0, 1, 2).valid);
        QCOMPARE(e.win.w.undoStack()->count(), 3);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:6"));
        QVERIFY(e.win.w.session()->arrayAt(0, 2, 2).valid);
        // and the cell typed over, once the array is gone, is an ordinary one again
        pressRedo(&e.win.w);
        QVERIFY(e.settle());
        e.select(QStringLiteral("C2"));
        e.enter(QStringLiteral("5"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:5"));
    }

    // An array is saved as one (t="array" with its range) and read back as one.
    void anArrayIsSavedAndReadBack()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        const QByteArray xml = savedSheet(e.path);
        QVERIFY2(xml.contains("t=\"array\" ref=\"C1:C3\""), xml.constData());
        EditWindow again;
        QVERIFY(again.open(xml.isEmpty() ? QByteArray() : [&] {
            QFile f(e.path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        }()));
        QVERIFY(again.settle());
        QVERIFY(again.win.w.session()->arrayAt(0, 1, 2).valid);
        QCOMPARE(again.valueAt(QStringLiteral("C3")), QStringLiteral("n:6"));
        again.select(QStringLiteral("C2"));
        QCOMPARE(again.view()->activeCellText(), QStringLiteral("{=A1:A3*2}"));
    }

    // A single cell gets an array too, and a cell outside any array: Ctrl+Shift+Enter on a plain formula makes it an array of one cell.
    void anArrayOfOneCellAndArrayOverCellsThatHoldValues()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "C2=old"})));
        QVERIFY(e.settle());
        e.select(QStringLiteral("D1"));
        e.type(QStringLiteral("=SUM(A1:A3*2)"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:12"));
        QVERIFY(e.win.w.session()->arrayAt(0, 0, 3).valid);
        // over cells that hold something: it is replaced, and Undo brings it back
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3+1"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:3"));
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("s:old"));
    }

    // Ctrl with a click or a drag adds a range; a click without Ctrl leaves one.
    void ctrlWithAClickOrADragAddsARange()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "C3=2", "E1=3"})));
        QVERIFY(e.settle());
        e.win.w.resize(1100, 640);
        clickCell(e, QStringLiteral("A1"));
        QVERIFY(!e.view()->hasSeveralRanges());
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        QVERIFY(e.view()->hasSeveralRanges());
        QCOMPARE(int(e.view()->extraRanges().size()), 1);
        QCOMPARE(int(e.view()->extraRanges()[0].row1), 0);
        QCOMPARE(int(e.view()->extraRanges()[0].col1), 0);
        QCOMPARE(e.selected(), QStringLiteral("C3"));
        QTest::mousePress(e.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, middleOf(e, QStringLiteral("E1")));
        QTest::mouseMove(e.view()->viewport(), middleOf(e, QStringLiteral("F2")));
        QTest::mouseRelease(e.view()->viewport(), Qt::LeftButton, Qt::ControlModifier, middleOf(e, QStringLiteral("F2")));
        const auto all = e.view()->allRanges();
        QCOMPARE(int(all.size()), 3);
        QCOMPARE(int(all[2].row1), 0);
        QCOMPARE(int(all[2].col1), 4);
        QCOMPARE(int(all[2].row2), 1);
        QCOMPARE(int(all[2].col2), 5);
        clickCell(e, QStringLiteral("B2"));
        QVERIFY(!e.view()->hasSeveralRanges());
        QCOMPARE(int(e.view()->allRanges().size()), 1);
    }

    // Delete clears every selected range as one step, and Undo takes it back.
    void deleteClearsEveryRange()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "C3=2", "E1=3"})));
        QVERIFY(e.settle());
        clickCell(e, QStringLiteral("A1"));
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("-"));
        QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("n:3"));
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:2"));
    }

    // Every command the window has either refuses with several ranges selected (and changes nothing), or is on this short list of commands that need no range, or do their work on every range.
    void everyCommandRefusesSeveralRangesOrIsOnTheSafeList()
    {
        // (the whole list: a command is safe when its id is one of these, or begins with one of the first)
        const QStringList prefixes{"file.", "help.", "view.", "win.", "zoom.", "sheetopt.", "protect.", "hf.", "chartdesign.", "object.", "sheet.", "margins.", "orient.", "size.", "page.", "pagesetup.", "titles.",
                                   "insert.control.", "insert.shape.", "insert.equation"};
        const QStringList exact{"edit.undo", "edit.redo", "edit.selectAll", "edit.find", "edit.clear.contents", "find.find", "find.replace", "find.next", "find.goto", "find.replaceFonts", "printarea.clear",
                                "format.defaultHeight", "format.defaultWidth", "comments.next", "comments.prev", "notes.next", "notes.prev", "notes.showAll", "insert.headerFooter", "insert.icons",
                                "insert.object.chart", "insert.object.page", "insert.onlinePicture", "insert.picture", "insert.symbol", "insert.textBox", "insert.textBox.draw", "review.accessibility",
                                "review.statistics", "review.thesaurus", "formulas.calcAuto", "formulas.calcBeforeSave", "formulas.calcManual", "formulas.calcNoTables", "formulas.calculate",
                                "formulas.calculateEverything", "formulas.calculateSheet", "formulas.circular", "formulas.errorChecking", "formulas.evaluate", "formulas.insertFunction",
                                "formulas.iteration", "formulas.names", "formulas.nextError", "formulas.options", "formulas.precision", "formulas.previousError", "formulas.rebuildAndCalculate",
                                "formulas.removeArrows", "formulas.traceDependents", "formulas.traceError", "formulas.tracePrecedents", "formulas.useName"};
        auto listed = [&](const QString &id) {
            if (exact.contains(id)) return true;
            for (const QString &p : prefixes)
                if (id.startsWith(p)) return true;
            return false;
        };
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "C3=2", "E1=3"})));
        QVERIFY(e.settle());
        clickCell(e, QStringLiteral("A1"));
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        QVERIFY(e.view()->hasSeveralRanges());
        const QStringList ids = e.win.w.actionIds();
        QVERIFY(ids.size() > 300);
        int refused = 0;
        for (const QString &id : ids) {
            QCOMPARE(CrunchWindow::worksOnSeveralRanges(id), listed(id));
            if (listed(id)) continue;
            QAction *a = e.win.w.act(id);
            QVERIFY2(a, qPrintable(id));
            a->setEnabled(true);
            const qsizetype told = e.win.told.size();
            a->trigger();
            QVERIFY2(e.win.told.size() == told + 1 && e.win.told.last().contains(QLatin1String("several ranges")), qPrintable(id));
            QVERIFY2(e.view()->hasSeveralRanges(), qPrintable(id));
            ++refused;
        }
        QVERIFY(refused > 200);
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:2"));
    }

    // Find in the selection, Replace All, and printing the selection refuse with several ranges, and write and print nothing
    void searchAndPrintOfTheSelectionRefuseSeveralRanges()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=apple", "C3=apple", "E1=pear"})));
        QVERIFY(e.settle());
        clickCell(e, QStringLiteral("A1"));
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        CrunchWindow &w = e.win.w;
        w.act(QStringLiteral("find.replace"))->trigger();
        FindDialog *d = w.findDialog();
        QVERIFY(d);
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("apple"));
        d->findChild<QLineEdit *>(QStringLiteral("replaceText"))->setText(QStringLiteral("pear"));
        e.win.told.clear();
        d->findChild<QPushButton *>(QStringLiteral("replaceAll"))->click();
        QTRY_VERIFY(!e.win.told.isEmpty());
        QVERIFY2(e.win.told.last().contains(QLatin1String("several ranges")), qPrintable(e.win.told.join('|')));
        QCOMPARE(w.undoStack()->count(), 0);
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("s:apple"));
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("s:apple"));
        // the selection printed
        crunch::PrintJobOptions o;
        o.what = crunch::PrintWhat::Selection;
        QString why;
        QVERIFY(!w.makePrintJob(o, &why));
        QVERIFY2(why.contains(QLatin1String("several ranges")), qPrintable(why));
    }

    // The other ranges belong to the sheet they were selected on: a switch to another sheet leaves one range there and back, and Undo of a row put in leaves one, so that Delete never clears
    // coordinates that were of another place.
    void severalRangesAreOneAfterASheetSwitchOrAnUndoOfARowPutIn()
    {
        EditWindow e;
        QVERIFY(e.open(budgetWorkbook()));
        QVERIFY(e.settle());
        clickCell(e, QStringLiteral("A1"));
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        QVERIFY(e.view()->hasSeveralRanges());
        e.win.w.setActiveSheet(2);
        QCOMPARE(e.win.w.activeSheet(), 2);
        QVERIFY(!e.view()->hasSeveralRanges());
        QCOMPARE(int(e.view()->allRanges().size()), 1);
        // (Delete there clears the one cell the sheet shows selected, and nothing else)
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:7"));
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("n:3"));
        e.win.w.setActiveSheet(0);
        QCOMPARE(int(e.view()->allRanges().size()), 1);
        // a row put in, and taken out by Undo
        clickCell(e, QStringLiteral("A1"));
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        QVERIFY(e.win.w.session()->insertRows(0, 5, 1).ok);
        QVERIFY(!e.view()->hasSeveralRanges());
        clickCell(e, QStringLiteral("A1"));
        clickCell(e, QStringLiteral("C3"), Qt::ControlModifier);
        e.win.w.undoStack()->undo();
        QVERIFY(!e.view()->hasSeveralRanges());
        QCOMPARE(int(e.view()->allRanges().size()), 1);
    }

    // An array entered over a block that holds another array, that begins inside it, covers every cell of the new block: none is left out; Undo brings the old array back, and Redo makes the new again.
    void anArrayOverAnotherThatBeginsInsideLeavesNoHole()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "A4=4"})));
        QVERIFY(e.settle());
        e.view()->selectRange(1, 3, 2, 3);
        e.type(QStringLiteral("=A1:A2*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QVERIFY(e.win.w.session()->arrayAt(0, 2, 3).valid);
        auto allOfOne = [&](quint32 first, quint32 last) {
            for (quint32 row = 0; row < 5; ++row) {
                const auto r = e.win.w.session()->arrayAt(0, row, 3);
                if (row < first || row > last) {
                    if (r.valid) return false;
                    continue;
                }
                if (!r.valid || r.row1 != first || r.row2 != last) return false;
            }
            return true;
        };
        QVERIFY(allOfOne(1, 2));
        e.view()->selectRange(0, 3, 3, 3);
        e.type(QStringLiteral("=A1:A4+10"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QVERIFY(allOfOne(0, 3));
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:11"));
        QCOMPARE(e.valueAt(QStringLiteral("D2")), QStringLiteral("n:12"));
        QCOMPARE(e.valueAt(QStringLiteral("D4")), QStringLiteral("n:14"));
        pressUndo(&e.win.w);
        QVERIFY(e.settle());
        QVERIFY(allOfOne(1, 2));
        pressRedo(&e.win.w);
        QVERIFY(e.settle());
        QVERIFY(allOfOne(0, 3));
        QCOMPARE(e.valueAt(QStringLiteral("D3")), QStringLiteral("n:13"));
    }

    // F2 on any cell of an array opens the array's formula, as the bar shows it (not the cell's value); on a protected sheet, and in a range with a password, an array cell is refused or asks as any cell does.
    void f2OnAnArrayCellOpensItsFormulaAndAsksAsAnyCellDoes()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        for (const char *ref : {"C1", "C2", "C3"}) {
            e.select(QString::fromLatin1(ref));
            QTest::keyClick(e.view(), Qt::Key_F2);
            QVERIFY2(e.win.w.cellEditor()->isVisible(), ref);
            QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("=A1:A3*2"));
            e.key(Qt::Key_Escape);
            QVERIFY(!e.win.w.cellEditor()->isVisible());
        }
        // a protected sheet: its cells are locked, and the array's are
        e.win.w.anchorHooks().protectAsker = [](ProtectRequest *) { return true; };
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QVERIFY(e.win.w.session()->protectionOf(0).enabled);
        e.win.told.clear();
        for (const char *ref : {"C1", "C2"}) {
            e.select(QString::fromLatin1(ref));
            QTest::keyClick(e.view(), Qt::Key_F2);
            QVERIFY2(!e.win.w.cellEditor()->isVisible(), ref);
            QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
            e.win.told.clear();
        }
    }

    void anArrayCellInARangeWithAPasswordAsksForIt()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        e.win.w.anchorHooks().protectAsker = [](ProtectRequest *) { return true; };
        e.win.w.act(QStringLiteral("protect.sheet"))->trigger();
        QVERIFY(e.win.w.session()->protectionOf(0).enabled);
        e.win.w.anchorHooks().editRangesAsker = [](EditRangesRequest *r) {
            jp::office::sheet::EditRange mine;
            mine.title = QStringLiteral("Mine");
            mine.sqref = QStringLiteral("C1:C3");
            r->ranges.append(mine);
            r->passwords.insert(QStringLiteral("Mine"), QStringLiteral("secret"));
            return true;
        };
        e.win.w.act(QStringLiteral("protect.editRanges"))->trigger();
        QTRY_VERIFY_WITH_TIMEOUT(!e.win.w.session()->protectionOf(0).editRanges.isEmpty(), 60000);
        int asked = 0;
        e.win.w.anchorHooks().passwordAsker = [&](PasswordRequest *) {
            ++asked;
            return false;   // (the person cancels)
        };
        e.select(QStringLiteral("C2"));
        QTest::keyClick(e.view(), Qt::Key_F2);
        QTRY_COMPARE(asked, 1);
        QVERIFY(!e.win.w.cellEditor()->isVisible());
    }

    // An array over cells of an array too big for its cells to be flagged is refused by the array's bounds: no array is made inside it, over part of it, or from one cell of it.
    void anArrayInsideOrOverPartOfAHugeArrayIsRefused()
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f t=\"array\" ref=\"B1:B1000001\">1</f><v>1</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.session()->arrayAt(0, 0, 1).valid);
        QCOMPARE(int(e.win.w.session()->arrayAt(0, 0, 1).row2), 1000000);
        // one cell inside, and a block inside: refused, and nothing changes
        for (const bool block : {false, true}) {
            if (block) e.view()->selectRange(5000, 1, 5002, 1);
            else e.select(QStringLiteral("B5001"));
            e.type(QStringLiteral("=A1+1"));
            e.win.told.clear();
            e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
            QVERIFY(e.settle());
            QVERIFY2(!e.win.told.isEmpty(), block ? "block" : "cell");
            QCOMPARE(e.win.w.undoStack()->count(), 0);
            QCOMPARE(int(e.win.w.session()->arrayAt(0, 0, 1).row2), 1000000);
            QVERIFY(!e.win.w.session()->arrayAt(0, 5000, 1).valid || e.win.w.session()->arrayAt(0, 5000, 1).row1 == 0);
            e.key(Qt::Key_Escape);
        }
    }

    // The first cell of an array, and the whole array, are copied and filled as what they are worth: no formula of one cell that would spill, no error.
    void copyingAndFillingFromAnArrayGiveItsValues()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        // the whole array, to E1
        copyRange(e, QStringLiteral("C1:C3"));
        paste(e, QStringLiteral("E1"));
        QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("E3")), QStringLiteral("n:6"));
        QCOMPARE(e.win.w.session()->editText(0, 0, 4), QStringLiteral("2"));
        // the first cell alone, to G1
        copyRange(e, QStringLiteral("C1:C1"));
        paste(e, QStringLiteral("G1"));
        QCOMPARE(e.valueAt(QStringLiteral("G1")), QStringLiteral("n:2"));
        QCOMPARE(e.win.w.session()->editText(0, 0, 6), QStringLiteral("2"));
        // Fill Right from the first cell
        e.view()->selectRange(0, 2, 0, 3);
        e.win.w.act(QStringLiteral("fill.right"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:2"));
        QCOMPARE(e.win.w.session()->editText(0, 0, 3), QStringLiteral("2"));
    }

    // An array is kept by a copy of its sheet, and a formula that refers into an array reads its values.
    void anArraySurvivesASheetCopyAndAFormulaReadsItsCells()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        e.view()->selectRange(0, 2, 2, 2);
        e.type(QStringLiteral("=A1:A3*2"));
        e.key(Qt::Key_Return, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("=C3+C2"));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:10"));
        int made = -1;
        QVERIFY(e.win.w.session()->copySheet(0, 1, QStringLiteral("Copy"), &made).ok);
        QVERIFY(made >= 0 && made != 0);
        const auto copied = e.win.w.session()->arrayAt(made, 1, 2);
        QVERIFY(copied.valid);
        QCOMPARE(int(copied.row1), 0);
        QCOMPARE(int(copied.row2), 2);
        e.win.w.setActiveSheet(made);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("C3")), QStringLiteral("n:6"));
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("n:10"));
        e.select(QStringLiteral("C2"));
        QCOMPARE(e.view()->activeCellText(), QStringLiteral("{=A1:A3*2}"));
    }

    // Go To takes several references with commas between them, and selects them all.
    void goToSelectsSeveralReferences()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1"})));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.goToReference(QStringLiteral("A1:B2,D4")));
        QCOMPARE(int(e.view()->allRanges().size()), 2);
        QCOMPARE(e.selected(), QStringLiteral("D4"));
        QCOMPARE(int(e.view()->extraRanges()[0].row2), 1);
        QVERIFY(!e.win.w.goToReference(QStringLiteral("A1,Nowhere!B2")));
    }

    // Ctrl+A in data selects the block around the cursor first, and the sheet on the next press; Ctrl+Shift+* is the block alone.
    void ctrlASelectsTheCurrentRegionFirst()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1=2", "A2=3", "B2=4", "A3=5", "D8=9"})));
        QVERIFY(e.settle());
        e.select(QStringLiteral("B2"));
        QTest::keyClick(e.view(), Qt::Key_A, Qt::ControlModifier);
        QCOMPARE(e.view()->rangeName(), QStringLiteral("A1:B3"));
        QCOMPARE(e.selected(), QStringLiteral("B2"));
        QTest::keyClick(e.view(), Qt::Key_A, Qt::ControlModifier);
        QVERIFY(e.view()->selection().whole == grid::Selection::Whole::All);
        e.select(QStringLiteral("A2"));
        QTest::keyClick(e.view(), Qt::Key_Asterisk, Qt::ControlModifier | Qt::ShiftModifier);
        QCOMPARE(e.view()->rangeName(), QStringLiteral("A1:B3"));
        // a selection that is already past the block goes to the sheet; it does not shrink to the block
        e.view()->selectRange(0, 0, 9, 4);
        QTest::keyClick(e.view(), Qt::Key_A, Qt::ControlModifier);
        QVERIFY(e.view()->selection().whole == grid::Selection::Whole::All);
        // a cell with nothing around it: the whole sheet at once
        e.select(QStringLiteral("H20"));
        QTest::keyClick(e.view(), Qt::Key_A, Qt::ControlModifier);
        QVERIFY(e.view()->selection().whole == grid::Selection::Whole::All);
    }

    // End, then an arrow, goes to the edge of the data as Ctrl and the arrow does; the status bar says End meanwhile.
    void endThenAnArrowGoesToTheEdgeOfTheData()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "A6=9"})));
        QVERIFY(e.settle());
        e.select(QStringLiteral("A1"));
        QTest::keyClick(e.view(), Qt::Key_End);
        QVERIFY(e.view()->endMode());
        QCOMPARE(e.win.w.statusText(), QStringLiteral("End"));
        QTest::keyClick(e.view(), Qt::Key_Down);
        QTRY_COMPARE(e.selected(), QStringLiteral("A3"));
        QVERIFY(!e.view()->endMode());
        QCOMPARE(e.win.w.statusText(), QStringLiteral("Ready"));
        // a click ends it too, and the status bar stops saying End
        QTest::keyClick(e.view(), Qt::Key_End);
        QCOMPARE(e.win.w.statusText(), QStringLiteral("End"));
        clickCell(e, QStringLiteral("A2"));
        QVERIFY(!e.view()->endMode());
        QCOMPARE(e.win.w.statusText(), QStringLiteral("Ready"));
        e.select(QStringLiteral("A3"));
        // another key ends the mode without moving
        QTest::keyClick(e.view(), Qt::Key_End);
        QTest::keyClick(e.view(), Qt::Key_Escape);
        QVERIFY(!e.view()->endMode());
        QCOMPARE(e.selected(), QStringLiteral("A3"));
    }

    // Shift with a drag of a selected block's border puts the block between the cells (those in the way move down), not over them: one step of Undo, and formulas that point at cells that
    // moved keep pointing at them. The same drag without Shift writes over what is there.
    void shiftDragOfTheBorderInsertsAndAPlainDragWritesOver()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "C1==A3*10", "C2==A1"})));
        QVERIFY(e.settle());
        e.select(QStringLiteral("A3"));
        const QPoint from = onLeftBorder(e, QStringLiteral("A3")), to(from.x(), middleOf(e, QStringLiteral("A1")).y());
        const int steps = e.win.w.undoStack()->count();
        dragFromTo(e, from, to, Qt::ShiftModifier);
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Moving cells"));   // (the step is named for what the person did)
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:3"));
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:30"));   // (it pointed at the cell that moved)
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:1"));    // (and this at the cell that was pushed down)
        QCOMPARE(e.selected(), QStringLiteral("A1"));
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:3"));
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:1"));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);   // (one step, not two)
        QCOMPARE(e.win.w.undoStack()->index(), steps);
        // a plain drag writes over the cell
        e.select(QStringLiteral("A3"));
        dragFromTo(e, from, to);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:3"));
        QCOMPARE(e.valueAt(QStringLiteral("A2")), QStringLiteral("n:2"));
        QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("-"));
    }

    // Dragged across, the cells in the way move right; a block of two cells makes room for two. A formula that points at a cell that moved, and one at a cell that was pushed over, follow
    // them, and Undo puts everything back as one step.
    void shiftDragAcrossMovesTheCellsInTheWayRight()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "B1=7", "B2=8", "D1=4", "D2=5", "F1==D1+1", "F2==B1"})));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        e.view()->selectRange(0, 3, 1, 3);   // D1:D2
        const QRect d = e.view()->sheet().layout().cellRect(0, 3);
        const QPoint from(d.left() + 1, d.center().y()), to(middleOf(e, QStringLiteral("B1")).x() - 4, from.y());
        const int steps = e.win.w.undoStack()->count();
        dragFromTo(e, from, to, Qt::ShiftModifier);
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:4"));
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:5"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:7"));
        QCOMPARE(e.valueAt(QStringLiteral("C2")), QStringLiteral("n:8"));
        QCOMPARE(e.valueAt(QStringLiteral("D1")), QStringLiteral("-"));
        // (F1 and F2 are pushed over too, to G1 and G2)
        QCOMPARE(e.valueAt(QStringLiteral("G1")), QStringLiteral("n:5"));   // (it pointed at a cell that moved)
        QCOMPARE(e.valueAt(QStringLiteral("G2")), QStringLiteral("n:7"));   // (and this at a cell that was pushed over)
        QCOMPARE(s->editText(0, 0, 6), QStringLiteral("=B1+1"));
        QCOMPARE(s->editText(0, 1, 6), QStringLiteral("=C1"));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->index(), steps);
        const std::pair<const char *, const char *> back[] = {{"A1", "n:1"}, {"A2", "n:2"}, {"B1", "n:7"}, {"B2", "n:8"}, {"D1", "n:4"}, {"D2", "n:5"}, {"F1", "n:5"}, {"F2", "n:7"}, {"C1", "-"}, {"C2", "-"}, {"G1", "-"}};
        for (const auto &cell : back) QCOMPARE(e.valueAt(QString::fromLatin1(cell.first)), QString::fromLatin1(cell.second));
        QCOMPARE(s->editText(0, 0, 5), QStringLiteral("=D1+1"));
        QCOMPARE(s->editText(0, 1, 5), QStringLiteral("=B1"));
        e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:4"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:7"));
    }

    // What points at the cells from another sheet, or by a name, follows them as well: a cell that moved and one that was pushed over; Undo brings all of it back.
    void formulasOnOtherSheetsAndNamesFollowTheCellsThatMoveAndThoseThatArePushedOver()
    {
        const QString names = QStringLiteral("<definedName name=\"Moved\">Sheet1!$A$3</definedName><definedName name=\"Pushed\">Sheet1!$A$1</definedName>");
        EditWindow e;
        QVERIFY(e.open(crunchtest::bookOfSheets({{"A1=1", "A2=2", "A3=3"}, {"A1==Sheet1!A3", "A2==Sheet1!A1", "A3==Moved", "A4==Pushed"}}, QMap<int, QString>(), QMap<int, QString>(), names)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const int moved = s->names().find(-1, QStringLiteral("Moved")), pushed = s->names().find(-1, QStringLiteral("Pushed"));
        QVERIFY(moved >= 0 && pushed >= 0);
        QVERIFY(s->moveCellsInserting(0, rangeOf(QStringLiteral("A3")), 0, 0, true).ok);
        QVERIFY(e.settle());
        QCOMPARE(s->editText(1, 0, 0), QStringLiteral("=Sheet1!A1"));
        QCOMPARE(s->editText(1, 1, 0), QStringLiteral("=Sheet1!A2"));
        QCOMPARE(s->nameTextOf(moved), QStringLiteral("=Sheet1!$A$1"));
        QCOMPARE(s->nameTextOf(pushed), QStringLiteral("=Sheet1!$A$2"));
        QCOMPARE(s->editText(1, 2, 0), QStringLiteral("=Moved"));
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(s->editText(1, 0, 0), QStringLiteral("=Sheet1!A3"));
        QCOMPARE(s->editText(1, 1, 0), QStringLiteral("=Sheet1!A1"));
        QCOMPARE(s->nameTextOf(moved), QStringLiteral("=Sheet1!$A$3"));
        QCOMPARE(s->nameTextOf(pushed), QStringLiteral("=Sheet1!$A$1"));
    }

    // A shift-click on a selection's border is a click like any other (it extends the selection); only a move past the drag distance makes it an insert.
    void aShiftClickOnTheBorderExtendsTheSelectionAndOnlyADragInserts()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B2=2", "C3=3", "D4=4"})));
        QVERIFY(e.settle());
        QWidget *vp = e.view()->viewport();
        const jp::grid::GridRange block{1, 1, 2, 2};   // B2:C3
        e.view()->selectRange(1, 1, 2, 2);
        QTest::mouseClick(vp, Qt::LeftButton, Qt::ShiftModifier, middleOf(e, QStringLiteral("B3")));
        const jp::grid::GridRange extended = e.view()->selection().range;
        QVERIFY(!(extended == block));
        const int steps = e.win.w.undoStack()->count();
        e.view()->selectRange(1, 1, 2, 2);
        QTest::mouseClick(vp, Qt::LeftButton, Qt::ShiftModifier, onLeftBorder(e, QStringLiteral("B3")));
        QVERIFY(e.view()->selection().range == extended);
        // (a move of a pixel is not a drag)
        e.view()->selectRange(1, 1, 2, 2);
        const QPoint at = onLeftBorder(e, QStringLiteral("B3"));
        QTest::mousePress(vp, Qt::LeftButton, Qt::ShiftModifier, at);
        QTest::mouseMove(vp, at + QPoint(0, 1));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, at + QPoint(0, 1));
        QVERIFY(e.view()->selection().range == extended);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:2"));
    }

    // A shift-click or a ctrl-click on a shown note's box acts on the cell under it, as it does on the sheet; a plain click picks the box.
    void aShiftOrCtrlClickOnAShownNoteBoxKeepsItsMeaningOnTheCellUnderIt()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        e.win.w.resize(1100, 640);
        e.win.w.show();
        QWidget *vp = e.view()->viewport();
        const QRect box = e.view()->shownNoteRect(2, 2);
        QVERIFY(!box.isEmpty());
        e.view()->selectRange(0, 0, 0, 0);
        QTest::mouseClick(vp, Qt::LeftButton, Qt::ShiftModifier, box.center());
        QVERIFY(!e.view()->hasPickedNote());
        QVERIFY(e.view()->selection().range.row2 > 0 && e.view()->selection().range.col2 > 0);   // (A1 to the cell under the box)
        e.view()->selectRange(0, 0, 0, 0);
        QTest::mouseClick(vp, Qt::LeftButton, Qt::ControlModifier, box.center());
        QVERIFY(!e.view()->hasPickedNote());
        QVERIFY(e.view()->hasSeveralRanges());
        e.view()->selectRange(0, 0, 0, 0);
        QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, box.center());
        QVERIFY(e.view()->hasPickedNote());
    }

    // A move that makes room and then moves is one thing: if either part passes what Undo holds the person is asked once, before anything is done, and then both are done (and the history
    // is let go of at the end, not between them); if the person says no, nothing is done.
    void aPairThatPassesTheUndoLimitIsAskedOnceAndDoneWholeOrNotAtAll()
    {
        QStringList cells;
        for (int i = 1; i <= 80; ++i) cells << QStringLiteral("A%1=%1").arg(i);
        cells << QStringLiteral("C1==A41*10");
        EditWindow e;
        QVERIFY(e.open(sheetOf(cells)));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        s->setUndoByteLimit(4000);
        const int steps = e.win.w.undoStack()->count();
        const crunch::EditSession::Outcome o = s->moveCellsInserting(0, rangeOf(QStringLiteral("A41:A80")), 0, 0, true);
        QVERIFY(!o.ok && o.confirm);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("A41")), QStringLiteral("n:41"));
        QStringList asked;
        bool answer = false;
        e.win.w.setBigStepAsker([&](const QString &text) {
            asked << text;
            return answer;
        });
        e.win.told.clear();
        e.view()->moveInsertRequested(jp::grid::GridRange{40, 0, 79, 0}, jp::grid::GridRange{0, 0, 39, 0});
        QCOMPARE(asked.size(), 1);
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QVERIFY(e.win.told.isEmpty());
        answer = true;
        e.view()->moveInsertRequested(jp::grid::GridRange{40, 0, 79, 0}, jp::grid::GridRange{0, 0, 39, 0});
        QCOMPARE(asked.size(), 2);   // (once for the two parts)
        QVERIFY(e.settle());
        QVERIFY2(e.win.told.isEmpty(), qPrintable(e.win.told.join('|')));
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:41"));
        QCOMPARE(e.valueAt(QStringLiteral("A40")), QStringLiteral("n:80"));
        QCOMPARE(e.valueAt(QStringLiteral("A41")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("A80")), QStringLiteral("n:40"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:410"));
    }

    // If the second part is refused after the first was done, the first is taken back, the person is told why, and Undo and Redo are as they were (nothing is left to redo).
    void aRefusedSecondPartTakesBackTheFirstAndLeavesNothingToRedo()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, 5, 5, QStringLiteral("x")).ok);
        const int steps = e.win.w.undoStack()->count(), index = e.win.w.undoStack()->index();
        const bool canUndo = s->canUndo();
        jp::office::sheet::SheetProtection locked;
        locked.enabled = true;
        s->setBetweenStepsHook([&] { s->setSheetProtection(0, locked, QStringLiteral("Protect")); });
        crunch::EditSession::Outcome o = s->moveCellsInserting(0, rangeOf(QStringLiteral("A3")), 0, 0, true);
        QVERIFY(!o.ok && !o.confirm);
        QVERIFY2(o.problem.contains(QLatin1String("protected")), qPrintable(o.problem));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(e.win.w.undoStack()->index(), index);
        QCOMPARE(s->canUndo(), canUndo);
        QVERIFY(!e.win.w.undoStack()->canRedo());
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:3"));
        // through the window it is said
        e.win.told.clear();
        e.view()->moveInsertRequested(jp::grid::GridRange{2, 0, 2, 0}, jp::grid::GridRange{0, 0, 0, 0});
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(!e.win.w.undoStack()->canRedo());
        s->setBetweenStepsHook(nullptr);
        QVERIFY(s->type(0, 6, 6, QStringLiteral("y")).ok);   // (the sheet is not left protected)
    }

    // The part that makes room is asked about as the sheet stands, and the move as it stands after: a shift onto cells that move away is not refused for a merged range or an array just
    // below them that would move too.
    void aMoveOntoCellsThatShiftAwayIsNotRefusedForAMergedRangeThatShiftsWithThem()
    {
        EditWindow e;
        const QByteArray book = buildXlsx({worksheet(crunchtest::rowsOfSpecs({"A2=5", "A10=1", "B10=2", "A11=3", "B11=4"}), QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A2:B3\"/></mergeCells>"))},
                                          QByteArray(), QString(), QString(), QByteArray());
        QVERIFY(e.open(book));
        QVERIFY(e.settle());
        const crunch::EditSession::Outcome o = e.win.w.session()->moveCellsInserting(0, rangeOf(QStringLiteral("A10:B11")), 0, 0, true);
        QVERIFY2(o.ok, qPrintable(o.problem));
        QVERIFY(e.settle());
        QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1"));
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:4"));
        QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:5"));
    }

    // A protected sheet, a merged range the room would cut, and a block that is in its own way refuse in words, and leave nothing done and nothing to redo.
    void insertingMovedCellsRefusesCleanly()
    {
        QString rows;
        for (int r = 1; r <= 5; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
        const QByteArray merged = buildXlsx({worksheet(rows, QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A1:A2\"/></mergeCells>"))}, QByteArray(), QString(), QString(), QByteArray());
        const QByteArray locked = buildXlsx({worksheet(rows, QStringLiteral("<sheetProtection sheet=\"1\"/>"))}, QByteArray(), QString(), QString(), QByteArray());
        for (int which = 0; which < 3; ++which) {
            EditWindow e;
            QVERIFY(e.open(which == 0 ? merged : which == 1 ? locked : merged));
            QVERIFY(e.settle());
            crunch::EditSession *s = e.win.w.session();
            const jp::office::sheet::CellRange from = rangeOf(which == 2 ? QStringLiteral("A3:A4") : QStringLiteral("A4"));
            // (the second goes to A2, cutting the merged range A1:A2 and, for the third, over its own cells)
            const crunch::EditSession::Outcome o = s->moveCellsInserting(0, from, which == 2 ? 3 : 1, 0, true);
            QVERIFY2(!o.ok && !o.problem.isEmpty(), qPrintable(QString::number(which)));
            if (which == 2) QCOMPARE(o.problem, QStringLiteral("Cells can't be inserted into themselves. Drag them somewhere else."));
            QCOMPARE(e.win.w.undoStack()->count(), 0);
            QVERIFY(!e.win.w.undoStack()->canRedo());
            QCOMPARE(e.valueAt(QStringLiteral("A4")), QStringLiteral("n:4"));
        }
    }

    // A note's box is read from the file's anchor (as cells and pixels from the note's own cell) and drawn there.
    void aNoteBoxIsReadFromTheFileAndDrawnThere()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        const jp::office::sheet::NoteInfo *n = e.win.w.session()->anchors().noteAt(0, 2, 2);
        QVERIFY(n && n->shown && n->box.set);
        QCOMPARE(n->box.col1, 1);
        QCOMPARE(n->box.x1, 15);
        QCOMPARE(n->box.row1, -1);
        QCOMPARE(n->box.y1, 2);
        QCOMPARE(n->box.col2, 3);
        QCOMPARE(n->box.row2, 2);
        QCOMPARE(n->box.y2, 16);
        QVERIFY(!e.win.w.session()->anchors().noteAt(0, 0, 0)->shown);
        const QRect box = e.view()->shownNoteRect(2, 2);
        const QRect c3 = e.view()->sheet().layout().cellRect(2, 2), d2 = e.view()->sheet().layout().cellRect(1, 3);
        QVERIFY(!box.isEmpty());
        QCOMPARE(box.left(), d2.left() + 15);
        QCOMPARE(box.top(), d2.top() + 2);
        QVERIFY(box.width() > c3.width() * 2 - 30);
        QVERIFY(box.left() > c3.right());
    }

    // Dragging a shown note's box moves it, and a handle sizes it; each is one step of Undo, and the file keeps the new anchor (and the note that was not touched as it was).
    void aNoteBoxIsMovedAndSizedWithThePointerAndSavedInTheFile()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        e.win.w.resize(1100, 640);
        e.win.w.show();
        crunch::EditSession *s = e.win.w.session();
        const jp::office::sheet::NoteBox first = s->anchors().noteAt(0, 2, 2)->box;
        const QRect was = e.view()->shownNoteRect(2, 2);
        const int steps = e.win.w.undoStack()->count();
        // a click only picks the box (and shows its handles), nothing is changed
        QTest::mouseClick(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, was.center());
        QVERIFY(e.view()->hasPickedNote());
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        // moved
        dragFromTo(e, was.center(), was.center() + QPoint(40, 30));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        const jp::office::sheet::NoteBox moved = s->anchors().noteAt(0, 2, 2)->box;
        QVERIFY(!(moved == first));
        const QRect after = e.view()->shownNoteRect(2, 2);
        QVERIFY(qAbs(after.left() - (was.left() + 40)) <= 1);
        QVERIFY(qAbs(after.top() - (was.top() + 30)) <= 1);
        QCOMPARE(after.size(), was.size());
        // sized by the corner handle at the bottom right
        const QRect handle = e.view()->noteHandleRect(4);
        QVERIFY(!handle.isEmpty());
        dragFromTo(e, handle.center(), handle.center() + QPoint(25, 20));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 2);
        const QRect sized = e.view()->shownNoteRect(2, 2);
        QVERIFY(qAbs(sized.width() - (after.width() + 25)) <= 1);
        QVERIFY(qAbs(sized.height() - (after.height() + 20)) <= 1);
        QCOMPARE(sized.topLeft(), after.topLeft());
        // Undo and Redo
        e.win.w.undoStack()->undo();
        QCOMPARE(e.view()->shownNoteRect(2, 2), after);
        e.win.w.undoStack()->undo();
        QVERIFY(s->anchors().noteAt(0, 2, 2)->box == first);
        QCOMPARE(e.view()->shownNoteRect(2, 2), was);
        e.win.w.undoStack()->redo();
        e.win.w.undoStack()->redo();
        QCOMPARE(e.view()->shownNoteRect(2, 2), sized);
        // saved, and read again
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        const QByteArray vml = savedPart(e.path, QStringLiteral("xl/drawings/vmlDrawing1.vml"));
        QVERIFY(!vml.isEmpty());
        const jp::office::sheet::NoteBox now = s->anchors().noteAt(0, 2, 2)->box;
        QVERIFY(vml.contains("<x:Anchor>" + QByteArray::number(2 + now.col1) + ", " + QByteArray::number(now.x1) + ", " + QByteArray::number(2 + now.row1) + ", " + QByteArray::number(now.y1) + ", "));
        QVERIFY(vml.contains("<x:Anchor>1, 15, 0, 2, 3, 15, 3, 16</x:Anchor>"));   // (the other note, as the file had it)
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QVERIFY(second.w.session()->anchors().noteAt(0, 2, 2)->box == now);
        QVERIFY(second.w.session()->anchors().noteAt(0, 0, 0)->box.set);
    }

    // A box that follows its cell: rows put in above carry it, and Undo brings it back.
    void aNoteBoxStaysWithItsCellWhenRowsAreInserted()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const jp::office::sheet::NoteBox before = s->anchors().noteAt(0, 2, 2)->box;
        QVERIFY(before.set);
        QVERIFY(s->insertRows(0, 0, 2).ok);
        const jp::office::sheet::NoteInfo *moved = s->anchors().noteAt(0, 4, 2);
        QVERIFY(moved && moved->box == before);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const jp::office::sheet::NoteInfo *again = second.w.session()->anchors().noteAt(0, 4, 2);
        QVERIFY(again && again->box == before);
    }

    // A protected sheet's notes can't be moved or sized (the window says so, and the box goes back), and a cell with no note has no box.
    void aNoteBoxOnAProtectedSheetIsRefused()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        e.win.w.resize(1100, 640);
        e.win.w.show();
        crunch::EditSession *s = e.win.w.session();
        jp::office::sheet::NoteBox b = s->anchors().noteAt(0, 2, 2)->box;
        b.x1 += 10;
        b.x2 += 10;
        QVERIFY(!s->setNoteBox(0, 1, 1, b, QStringLiteral("Move")).ok);   // (no note there)
        jp::office::sheet::SheetProtection locked;
        locked.enabled = true;
        locked.objects = true;
        QVERIFY(s->setSheetProtection(0, locked, QStringLiteral("Protect")).ok);
        const int steps = e.win.w.undoStack()->count();
        const QRect was = e.view()->shownNoteRect(2, 2);
        const jp::office::sheet::NoteBox first = s->anchors().noteAt(0, 2, 2)->box;
        e.win.told.clear();
        dragFromTo(e, was.center(), was.center() + QPoint(40, 30));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(s->anchors().noteAt(0, 2, 2)->box == first);
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(e.view()->shownNoteRect(2, 2), was);
    }

    // A box with anchors at the file's largest numbers is drawn at a size and a distance from its cell that are bounded, and does not take the clicks of the sheet: a cell and a header still
    // work.
    void aHostileNoteBoxCannotSwallowTheClicksOfTheSheet()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes(QByteArray(kHostileVml))));
        QVERIFY(e.settle());
        e.win.w.resize(1100, 640);
        e.win.w.show();
        QWidget *vp = e.view()->viewport();
        const QRect box = e.view()->shownNoteRect(2, 2);
        QVERIFY2(box.width() <= 640 && box.height() <= 480, qPrintable(QString::number(box.width()) + "x" + QString::number(box.height())));
        const QRect c3 = e.view()->sheet().layout().cellRect(2, 2);
        QVERIFY(box.isEmpty() || (qAbs(box.left() - c3.left()) <= 1000 && qAbs(box.top() - c3.top()) <= 1000));
        QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, middleOf(e, QStringLiteral("B2")));
        QVERIFY(!e.view()->hasPickedNote());
        QCOMPARE(e.selected(), QStringLiteral("B2"));
        const jp::grid::GridLayout &L = e.view()->sheet().layout();
        QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, QPoint(L.cellRect(0, 4).center().x(), L.headerHeight() / 2));
        QVERIFY(!e.view()->hasPickedNote());
        QVERIFY(e.view()->selection().whole == jp::grid::Selection::Whole::Columns);
    }

    // A box that reaches up over the column headers (the sheet is scrolled) is drawn and hit only over the cells of its own pane: a click on a header is a click on the header.
    void aNoteBoxIsClippedToTheCellsOfItsPane()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        e.win.w.resize(1100, 640);
        e.win.w.show();
        QWidget *vp = e.view()->viewport();
        const jp::grid::GridLayout &L = e.view()->sheet().layout();
        const int two = int(L.rows.position(2));
        QTRY_VERIFY(e.view()->verticalScrollBar()->maximum() >= two);
        e.view()->verticalScrollBar()->setValue(two);   // (C3 is the first row in view; its box begins a row above it, over the headers)
        QTRY_COMPARE(e.view()->firstRowInView(), qint64(2));
        const QRect box = e.view()->shownNoteRect(2, 2);
        QVERIFY(!box.isEmpty());
        QVERIFY2(box.top() < L.headerHeight() - 2, qPrintable(QString::number(box.top())));
        const QPoint onHeader(box.center().x(), L.headerHeight() - 3);
        QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, onHeader);
        QVERIFY(!e.view()->hasPickedNote());
        QVERIFY(e.view()->selection().whole == jp::grid::Selection::Whole::Columns);
        // nor is it painted there
        const QImage shot = vp->grab().toImage();
        QVERIFY(shot.pixelColor(onHeader) != QColor(0xFF, 0xFF, 0xE1));
    }

    // The positions a box is worked out in are those of all the rows, also with some scrolled out of sight above the frozen rows: the box of a note in a frozen row is the same size, and a
    // drag saves the anchor it was drawn at.
    void aNoteBoxIsConvertedAtThePositionsOfAllRowsAndColumnsEvenWhenSomeAreSkipped()
    {
        jp::grid::Axis rows(200, 20), cols(50, 64);
        jp::office::sheet::NoteBox b;
        b.set = true;
        b.row1 = -2;
        b.y1 = 3;
        b.col1 = 1;
        b.x1 = 5;
        b.row2 = 2;
        b.y2 = 7;
        b.col2 = 3;
        b.x2 = 9;
        const QRect plain = jp::crunch::noteBoxPixels(b, rows, cols, 2, 2);
        QVERIFY(plain.top() < 0 && plain.height() > 60);
        jp::grid::Axis skippedRows = rows, skippedCols = cols;
        skippedRows.setSkipped(2);
        skippedCols.setSkipped(1);
        QCOMPARE(jp::crunch::noteBoxPixels(b, skippedRows, skippedCols, 2, 2), plain);
        QVERIFY(jp::crunch::noteBoxFromPixels(plain, skippedRows, skippedCols, 2, 2) == b);
        QVERIFY(jp::crunch::noteBoxFromPixels(plain.translated(20, 30), skippedRows, skippedCols, 2, 2) == jp::crunch::noteBoxFromPixels(plain.translated(20, 30), rows, cols, 2, 2));
    }

    // A box dragged while a cell is being edited finishes the entry first, as a block dragged by its border does (the grid asks for it by its signal, whatever took the focus).
    void draggingANoteBoxFinishesAnEditInProgressFirst()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        e.win.w.resize(1100, 640);
        e.win.w.show();
        const QRect was = e.view()->shownNoteRect(2, 2);
        e.select(QStringLiteral("A5"));
        e.type(QStringLiteral("hello"));
        QVERIFY(e.win.w.isEditing());
        const int steps = e.win.w.undoStack()->count();
        e.view()->noteBoxRequested(2, 2, QRect(60, 10, 150, 80));
        QVERIFY(!e.win.w.isEditing());
        QCOMPARE(e.shownAt(QStringLiteral("A5")), QStringLiteral("hello"));
        QCOMPARE(e.win.w.undoStack()->count(), steps + 2);   // (the entry, and the box)
        QVERIFY(e.view()->shownNoteRect(2, 2) != was);
    }

    // A note that is moved with its cell keeps its box, also when it is touched after, and the file has it where it was relative to the cell.
    void aNoteKeepsItsBoxWhenItsCellMovesAndIsTouchedAfter()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        const jp::office::sheet::NoteBox before = s->anchors().noteAt(0, 2, 2)->box;
        QVERIFY(before.set);
        QVERIFY(s->moveCells(0, rangeOf(QStringLiteral("C3")), 0, 7, 4).ok);
        QVERIFY(s->anchors().noteAt(0, 7, 4));
        QVERIFY(s->anchors().noteAt(0, 7, 4)->box == before);
        QVERIFY(s->setNote(0, 7, 4, QStringLiteral("Changed"), QStringLiteral("Ann"), true, QStringLiteral("Edit Note")).ok);
        QVERIFY(s->anchors().noteAt(0, 7, 4)->box == before);
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const jp::office::sheet::NoteInfo *again = second.w.session()->anchors().noteAt(0, 7, 4);
        QVERIFY(again && again->box == before);
    }

    // A sheet that reads right to left prints as it does left to right (the page is not mirrored), so the box that has no place of its own is beside its cell on the same side in both.
    void notesPrintOnASheetThatReadsRightToLeftWhereTheyDoOnOne()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        QVERIFY(s->setNote(0, 1, 0, QStringLiteral("Look here"), QStringLiteral("Ann"), true, QStringLiteral("Add Note")).ok);
        jp::office::sheet::PageSetup p = s->pages().sheet(0);
        p.comments = jp::office::sheet::PrintComments::AsDisplayed;
        QVERIFY(s->setPageSetup(0, p, QStringLiteral("Comments")).ok);
        auto page = [&] {
            crunch::PrintJobOptions options;
            options.now = QDateTime::fromString(QStringLiteral("2026-03-05T14:07:09-05:00"), Qt::ISODate);
            QString why;
            const std::unique_ptr<crunch::PrintJob> job = w.makePrintJob(options, &why);
            return job ? job->preview(0, 900) : QImage();
        };
        const QImage ltr = page();
        QVERIFY(!ltr.isNull());
        w.act(QStringLiteral("view.rightToLeft"))->trigger();
        QVERIFY(w.act(QStringLiteral("view.rightToLeft"))->isChecked());
        QVERIFY(e.view()->sheet().rightToLeft());
        QVERIFY(page() == ltr);
    }

    // Comments set to print as displayed draw the boxes of the notes that are shown over the page's cells; at the end, or not at all, they do not.
    void notesPrintAsDisplayedOnTheSheet()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        QVERIFY(s->setNote(0, 1, 0, QStringLiteral("Look here"), QStringLiteral("Ann"), true, QStringLiteral("Add Note")).ok);
        QVERIFY(s->setNote(0, 2, 2, QStringLiteral("Hidden one"), QStringLiteral("Ann"), false, QStringLiteral("Add Note")).ok);
        QString why;
        // (the first page as an image, for how comments are set to print: judged by what differs from the page with none, whatever colors the job paints in)
        auto pageWith = [&](jp::office::sheet::PrintComments how) {
            jp::office::sheet::PageSetup p = s->pages().sheet(0);
            p.comments = how;
            if (!s->setPageSetup(0, p, QStringLiteral("Comments")).ok) return QImage();
            crunch::PrintJobOptions options;
            options.now = QDateTime::fromString(QStringLiteral("2026-03-05T14:07:09-05:00"), Qt::ISODate);   // (the footer shows the time: it is the same for every page made here)
            const std::unique_ptr<crunch::PrintJob> job = w.makePrintJob(options, &why);
            return job ? job->preview(0, 900) : QImage();
        };
        const QImage none = pageWith(jp::office::sheet::PrintComments::None);
        QVERIFY2(!none.isNull(), qPrintable(why));
        auto differing = [&](const QImage &other, QRect *where) {
            int count = 0;
            QRect box;
            if (other.size() != none.size()) return -1;
            for (int y = 0; y < none.height(); ++y)
                for (int x = 0; x < none.width(); ++x)
                    if (other.pixel(x, y) != none.pixel(x, y)) {
                        ++count;
                        box = box.isNull() ? QRect(x, y, 1, 1) : box.united(QRect(x, y, 1, 1));
                    }
            if (where) *where = box;
            return count;
        };
        QCOMPARE(differing(pageWith(jp::office::sheet::PrintComments::AtEnd), nullptr), 0);
        QRect where;
        const int shown = differing(pageWith(jp::office::sheet::PrintComments::AsDisplayed), &where);
        QVERIFY2(shown > 400, qPrintable(why + QString::number(shown)));
        // (one box: the note that is hidden has none; it is to the right of the note's cell, A2, and no larger than the usual box)
        QVERIFY(where.width() < 450 && where.height() < 250);
        QVERIFY(where.left() > 0);
    }
    // Cut cells put between others by a command (no pointer): the cells go to the selected cell and the ones there move down, in the step the drag has, with what points at them following;
    // the cut is used up.
    void insertCutCellsMovesThemBetweenOthersInOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "A4=4", "A5=5", "C1==A5"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QAction *insertClip = w.act(QStringLiteral("edit.insertClip"));
        QVERIFY(!insertClip->isEnabled());   // (nothing is cut or copied yet)
        copyRange(e, QStringLiteral("A5"), "edit.cut");
        QVERIFY(insertClip->isEnabled());
        QCOMPARE(insertClip->text(), QStringLiteral("Insert Cut Cells"));
        QList<CrunchWindow::ShiftRequest> asked;
        w.setShiftAsker([&](CrunchWindow::ShiftRequest *r) {
            asked << *r;
            r->way = CrunchWindow::ShiftRequest::Way::Down;
            return true;
        });
        e.select(QStringLiteral("A1"));
        const int steps = w.undoStack()->count();
        insertClip->trigger();
        QVERIFY(e.settle());
        QCOMPARE(asked.size(), 1);
        QVERIFY(asked[0].inserting && asked[0].cellsOnly);
        QCOMPARE(shown(e, QStringLiteral("A1:A5")), QStringLiteral("5|1|2|3|4"));
        QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:5"));
        QCOMPARE(formulaOf(e, QStringLiteral("C1")), QStringLiteral("=A1"));
        QCOMPARE(w.undoStack()->count(), steps + 1);
        QCOMPARE(w.undoStack()->undoText(), QStringLiteral("Moving cells"));
        QVERIFY(!e.view()->hasCopyBorder());
        QVERIFY(!insertClip->isEnabled());
        QCOMPARE(insertClip->text(), QStringLiteral("Insert Cut or Copied Cells"));
        pressUndo(&w);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:A5")), QStringLiteral("1|2|3|4|5"));
        QCOMPARE(formulaOf(e, QStringLiteral("C1")), QStringLiteral("=A5"));
    }

    // Copied cells put between others: room is made and the copy pasted, as one step; the copy is still ready to be put in again; the way can be right.
    void insertCopiedCellsMakesRoomAndPastesInOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1=2", "C1=3", "A2=7", "A3=8", "A4=9"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QAction *insertClip = w.act(QStringLiteral("edit.insertClip"));
        auto way = CrunchWindow::ShiftRequest::Way::Right;
        w.setShiftAsker([&](CrunchWindow::ShiftRequest *r) {
            r->way = way;
            return true;
        });
        copyRange(e, QStringLiteral("A1"));
        QCOMPARE(insertClip->text(), QStringLiteral("Insert Copied Cells"));
        e.select(QStringLiteral("B1"));
        const int steps = w.undoStack()->count();
        insertClip->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:D1")), QStringLiteral("1|1|2|3"));
        QCOMPARE(w.undoStack()->count(), steps + 1);
        QCOMPARE(w.undoStack()->undoText(), QStringLiteral("Inserting cells"));
        QVERIFY(e.view()->hasCopyBorder());   // (a copy stays ready)
        QVERIFY(insertClip->isEnabled());
        // again, down, from another cell: the two cells below move
        way = CrunchWindow::ShiftRequest::Way::Down;
        e.select(QStringLiteral("A2"));
        insertClip->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A2:A5")), QStringLiteral("1|7|8|9"));
        QCOMPARE(w.undoStack()->count(), steps + 2);
        pressUndo(&w);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A2:A4")), QStringLiteral("7|8|9"));
        QCOMPARE(shown(e, QStringLiteral("A1:D1")), QStringLiteral("1|1|2|3"));
        pressUndo(&w);
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:D1")), QStringLiteral("1|2|3|"));
    }

    // Insert (Ctrl+Plus, and Ctrl+Shift+Plus) with cells cut or copied inserts them; with nothing cut or copied it asks as it always did (cells, or whole rows and columns).
    void theInsertKeyPutsInCutCellsAndIsAsBeforeWithNone()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QList<bool> cellsOnly;
        w.setShiftAsker([&](CrunchWindow::ShiftRequest *r) {
            cellsOnly << r->cellsOnly;
            r->way = CrunchWindow::ShiftRequest::Way::Down;
            return true;
        });
        const QList<QKeySequence> keys = w.act(QStringLiteral("cells.insert"))->shortcuts();
        QVERIFY(keys.contains(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Plus)));
        e.select(QStringLiteral("A1"));
        QTest::keyClick(e.view(), Qt::Key_Plus, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QCOMPARE(cellsOnly, QList<bool>{false});
        QCOMPARE(shown(e, QStringLiteral("A1:A4")), QStringLiteral("|1|2|3"));
        pressUndo(&w);
        QVERIFY(e.settle());
        copyRange(e, QStringLiteral("A3"), "edit.cut");
        e.select(QStringLiteral("A1"));
        QTest::keyClick(e.view(), Qt::Key_Plus, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QCOMPARE(cellsOnly, (QList<bool>{false, true}));
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("3|1|2"));
        // whole rows selected: the rows are inserted, as before, whatever is on the clipboard
        copyRange(e, QStringLiteral("A1"));
        e.view()->selectRows(1, 1);
        QTest::keyClick(e.view(), Qt::Key_Plus, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QCOMPARE(w.undoStack()->undoText(), QStringLiteral("Inserting a row"));
        QCOMPARE(cellsOnly.size(), 2);
    }

    // Refusals say so in the words the drag uses, and leave nothing done and nothing to redo: a protected sheet, a block in its own way, and giving up the question.
    void insertingCutOrCopiedCellsRefusesCleanly()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "A4=4"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        bool answer = true;
        w.setShiftAsker([&](CrunchWindow::ShiftRequest *r) {
            r->way = CrunchWindow::ShiftRequest::Way::Down;
            return answer;
        });
        QAction *insertClip = w.act(QStringLiteral("edit.insertClip"));
        const int steps = w.undoStack()->count();
        // cut A3:A4 and put them at A4: in their own way
        copyRange(e, QStringLiteral("A3:A4"), "edit.cut");
        e.select(QStringLiteral("A4"));
        e.win.told.clear();
        insertClip->trigger();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("Cells can't be inserted into themselves")), qPrintable(e.win.told.join('|')));
        QCOMPARE(w.undoStack()->count(), steps);
        QVERIFY(!w.undoStack()->canRedo());
        QVERIFY(e.view()->hasCopyBorder());   // (the cut is still there to put somewhere else)
        // giving up the question
        answer = false;
        e.select(QStringLiteral("A1"));
        e.win.told.clear();
        insertClip->trigger();
        QCOMPARE(w.undoStack()->count(), steps);
        QVERIFY(e.win.told.isEmpty());
        QCOMPARE(shown(e, QStringLiteral("A1:A4")), QStringLiteral("1|2|3|4"));
        // a protected sheet
        answer = true;
        copyRange(e, QStringLiteral("A1"));
        jp::office::sheet::SheetProtection locked;
        locked.enabled = true;
        QVERIFY(w.session()->setSheetProtection(0, locked, QStringLiteral("Protect")).ok);
        const int protectedSteps = w.undoStack()->count();
        e.select(QStringLiteral("B1"));
        e.win.told.clear();
        insertClip->trigger();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(w.undoStack()->count(), protectedSteps);
        QVERIFY(!w.undoStack()->canRedo());
        // nothing cut or copied: the command is off (there is no way to ask it)
        QTest::keyClick(e.view(), Qt::Key_Escape);   // (the border goes, and the copy with it)
        QVERIFY(!insertClip->isEnabled());
        e.win.told.clear();
        insertClip->trigger();
        QVERIFY(e.win.told.isEmpty());
        QCOMPARE(w.undoStack()->count(), protectedSteps);
    }

    // Insert (Ctrl+Plus) puts copied cells in only while nothing has been changed since the copy; after an edit it inserts blank cells as it always did, and the copy is still there to paste and
    // for the command in the menu. Cut cells on another sheet than the cut's: blank cells, with no message.
    void theInsertKeyPutsInCopiedCellsOnlyUntilSomethingIsChanged()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=1", "A2=2", "A3=3"}, {"A1=a", "A2=b"}})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QList<bool> cellsOnly;
        w.setShiftAsker([&](CrunchWindow::ShiftRequest *r) {
            cellsOnly << r->cellsOnly;
            r->way = CrunchWindow::ShiftRequest::Way::Down;
            return true;
        });
        QAction *insertClip = w.act(QStringLiteral("edit.insertClip"));
        copyRange(e, QStringLiteral("A3"));
        e.select(QStringLiteral("C5"));
        e.enter(QStringLiteral("x"));   // (a change made after the copy)
        QVERIFY(e.settle());
        e.select(QStringLiteral("A1"));
        QTest::keyClick(e.view(), Qt::Key_Plus, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(e.settle());
        QCOMPARE(cellsOnly, QList<bool>{false});   // (the box of the plain Insert)
        QCOMPARE(shown(e, QStringLiteral("A1:A4")), QStringLiteral("|1|2|3"));
        QVERIFY(insertClip->isEnabled());   // (the copy is still there)
        paste(e, QStringLiteral("E1"));
        QCOMPARE(shown(e, QStringLiteral("E1")), QStringLiteral("3"));
        // the menu's command puts the copy in all the same
        e.select(QStringLiteral("A1"));
        insertClip->trigger();
        QVERIFY(e.settle());
        QCOMPARE(cellsOnly, (QList<bool>{false, true}));
        QCOMPARE(shown(e, QStringLiteral("A1:A5")), QStringLiteral("3||1|2|3"));
        // copied again, and Insert at once: the copied cells go in
        copyRange(e, QStringLiteral("A5"));
        e.select(QStringLiteral("A1"));
        QTest::keyClick(e.view(), Qt::Key_Plus, Qt::ControlModifier);
        QVERIFY(e.settle());
        QCOMPARE(cellsOnly, (QList<bool>{false, true, true}));
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("3|3|"));
        // an Undo is a change too
        copyRange(e, QStringLiteral("A5"));
        pressUndo(&w);
        QVERIFY(e.settle());
        e.select(QStringLiteral("A1"));
        QTest::keyClick(e.view(), Qt::Key_Plus, Qt::ControlModifier);
        QVERIFY(e.settle());
        QCOMPARE(cellsOnly, (QList<bool>{false, true, true, false}));
        // cells cut on one sheet and Insert on another: blank cells, and no message
        pressUndo(&w);
        QVERIFY(e.settle());
        copyRange(e, QStringLiteral("A1"), "edit.cut");
        w.sheetTabs()->setCurrentIndex(1);
        QTRY_COMPARE(w.activeSheet(), 1);
        e.select(QStringLiteral("A1"));
        e.win.told.clear();
        QVERIFY(insertClip->isEnabled());   // (the menu's command keeps its message for this sheet)
        insertClip->trigger();
        QVERIFY(!e.win.told.isEmpty());
        e.win.told.clear();
        QTest::keyClick(e.view(), Qt::Key_Plus, Qt::ControlModifier);
        QVERIFY(e.settle());
        QCOMPARE(cellsOnly.last(), false);
        QVERIFY2(e.win.told.isEmpty(), qPrintable(e.win.told.join('|')));
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("|a|b"));
    }

    // Whole rows, whole columns, and the whole sheet have their own Insert: the command for cut or copied cells is off for them in every menu, and on again for cells. A block of cells that is
    // not the size of the copy takes the copy once, at its top left cell.
    void insertingCutOrCopiedCellsIsOffForWholeRowsAndColumns()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "B1=7"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.setShiftAsker([&](CrunchWindow::ShiftRequest *r) {
            r->way = CrunchWindow::ShiftRequest::Way::Down;
            return true;
        });
        QAction *insertClip = w.act(QStringLiteral("edit.insertClip"));
        copyRange(e, QStringLiteral("A1"));
        QVERIFY(insertClip->isEnabled());
        e.view()->selectRows(1, 1);
        QVERIFY(!insertClip->isEnabled());
        e.view()->selectColumns(1, 2);
        QVERIFY(!insertClip->isEnabled());
        e.view()->selectAll();
        QVERIFY(!insertClip->isEnabled());
        const int steps = w.undoStack()->count();
        insertClip->trigger();
        QCOMPARE(w.undoStack()->count(), steps);
        // a block of 2 by 2 cells and a copy of one: it goes in once, at the block's top left cell
        e.view()->selectRange(1, 1, 2, 2);
        QVERIFY(insertClip->isEnabled());
        insertClip->trigger();
        QVERIFY(e.settle());
        QCOMPARE(w.undoStack()->count(), steps + 1);
        QCOMPARE(shown(e, QStringLiteral("B1:C3")), QStringLiteral("7||1|||"));
    }

    // Insert Copied Cells pastes as the default paste says (Values here), as a plain Paste does.
    void insertingCopiedCellsPastesAsTheDefaultPasteSays()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=4", "B1==A1+1", "B2=9"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.setShiftAsker([&](CrunchWindow::ShiftRequest *r) {
            r->way = CrunchWindow::ShiftRequest::Way::Down;
            return true;
        });
        const QVariant was = Settings::get().value(QStringLiteral("crunch/defaultPaste"));
        Settings::get().setValue(QStringLiteral("crunch/defaultPaste"), int(crunch::EditSession::PasteOptions::What::Values));
        copyRange(e, QStringLiteral("B1"));
        e.select(QStringLiteral("D1"));
        w.act(QStringLiteral("edit.insertClip"))->trigger();
        QVERIFY(e.settle());
        paste(e, QStringLiteral("F1"));
        const QString inserted = formulaOf(e, QStringLiteral("D1")), plain = formulaOf(e, QStringLiteral("F1"));
        if (was.isValid()) Settings::get().setValue(QStringLiteral("crunch/defaultPaste"), was);
        else Settings::get().remove(QStringLiteral("crunch/defaultPaste"));
        QCOMPARE(plain, QStringLiteral("5"));   // (the value, not =E1+1)
        QCOMPARE(inserted, plain);
    }

    // A formula in the copied block is changed for its new place as a plain Paste changes it, and a formula elsewhere that points at cells that move follows them.
    void insertedCopiedFormulasAreAdjustedAsAPasteDoesAndWhatPointsAtShiftedCellsFollows()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "B1==A1+1", "B2==B1*2", "D5=9", "D6=8", "G1==D6", "G2==SUM(D5:D6)"})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.setShiftAsker([&](CrunchWindow::ShiftRequest *r) {
            r->way = CrunchWindow::ShiftRequest::Way::Down;
            return true;
        });
        copyRange(e, QStringLiteral("B1:B2"));
        paste(e, QStringLiteral("F5"));
        QCOMPARE(formulaOf(e, QStringLiteral("F5")), QStringLiteral("=E5+1"));
        QCOMPARE(formulaOf(e, QStringLiteral("F6")), QStringLiteral("=F5*2"));
        e.select(QStringLiteral("D5"));
        w.act(QStringLiteral("edit.insertClip"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(formulaOf(e, QStringLiteral("D5")), QStringLiteral("=C5+1"));
        QCOMPARE(formulaOf(e, QStringLiteral("D6")), QStringLiteral("=D5*2"));
        QCOMPARE(shown(e, QStringLiteral("D7:D8")), QStringLiteral("9|8"));
        QCOMPARE(formulaOf(e, QStringLiteral("G1")), QStringLiteral("=D8"));
        QCOMPARE(formulaOf(e, QStringLiteral("G2")), QStringLiteral("=SUM(D7:D8)"));
        QCOMPARE(e.valueAt(QStringLiteral("G1")), QStringLiteral("n:8"));
        QCOMPARE(formulaOf(e, QStringLiteral("B1")), QStringLiteral("=A1+1"));   // (the copied cells are where they were)
    }

    // The second part of copied cells put between others is refused after the room was made (a sheet that got protected): the room is taken back, the person is told, and nothing is left to redo.
    void aRefusedPasteAfterTheRoomWasMadeTakesTheRoomBack()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3"})));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(0, 5, 5, QStringLiteral("x")).ok);
        const crunch::ClipData clip = s->copyCells(0, rangeOf(QStringLiteral("A3")), false);
        const int steps = e.win.w.undoStack()->count(), index = e.win.w.undoStack()->index();
        jp::office::sheet::SheetProtection locked;
        locked.enabled = true;
        s->setBetweenStepsHook([&] { s->setSheetProtection(0, locked, QStringLiteral("Protect")); });
        const crunch::EditSession::Outcome o = s->insertCellsPasting(0, 0, 0, clip, true, crunch::EditSession::PasteOptions());
        QVERIFY(!o.ok && !o.confirm);
        QVERIFY2(o.problem.contains(QLatin1String("protected")), qPrintable(o.problem));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(e.win.w.undoStack()->index(), index);
        QVERIFY(!e.win.w.undoStack()->canRedo());
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("A1:A3")), QStringLiteral("1|2|3"));
        s->setBetweenStepsHook(nullptr);
    }

    // Rows hidden by hand: a copy leaves them out (the clip is shorter), and a paste writes into the target's rows as they come, so the room made for the clip is exactly where the paste writes.
    void insertingCopiedCellsWithRowsHiddenByHandMakesRoomForWhatIsPasted()
    {
        QStringList cells = {"A1=1", "A2=2", "A3=3", "A4=4"};
        for (int r = 5; r <= 10; ++r) cells << QStringLiteral("C%1=c%1").arg(r);
        EditWindow e;
        QVERIFY(e.open(sheetOf(cells)));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.setShiftAsker([&](CrunchWindow::ShiftRequest *r) {
            r->way = CrunchWindow::ShiftRequest::Way::Down;
            return true;
        });
        // row 3 of the source hidden: the copy is 1, 2, 4
        e.view()->selectRows(2, 2);
        w.act(QStringLiteral("format.hideRows"))->trigger();
        copyRange(e, QStringLiteral("A1:A4"));
        e.select(QStringLiteral("C5"));
        w.act(QStringLiteral("edit.insertClip"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("C5:C12")), QStringLiteral("1|2|4|c5|c6|c7|c8|c9"));
        pressUndo(&w);
        QVERIFY(e.settle());
        // a row of the target hidden by hand: the paste writes into it, and so does the room
        e.view()->selectRows(5, 5);
        w.act(QStringLiteral("format.hideRows"))->trigger();
        copyRange(e, QStringLiteral("A1:A2"));
        e.select(QStringLiteral("C5"));
        w.act(QStringLiteral("edit.insertClip"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(shown(e, QStringLiteral("C5:C12")), QStringLiteral("1|2|c5|c6|c7|c8|c9|c10"));
    }

    // The new commands refuse several ranges (they are on neither list of commands that do not), they are found by the command search, and the words it has for them name commands the window has.
    void theNewCommandsAreInTheListsAndTheSearch()
    {
        for (const char *id : {"edit.insertClip", "notes.moveSize", "notes.resetSize"}) QVERIFY2(!CrunchWindow::worksOnSeveralRanges(QString::fromLatin1(id)), id);
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        CrunchWindow w;
        w.resize(1000, 700);
        w.show();
        for (const char *id : {"edit.insertClip", "notes.moveSize", "notes.resetSize"}) QVERIFY2(w.act(QString::fromLatin1(id)), id);
        QString problem = jptest::commandSearchProblem(w, QStringLiteral(":/crunch/cmdsearch.json"), QStringLiteral("move note"), QStringLiteral("Move or Size Note"));
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        problem = jptest::commandSearchProblem(w, QStringLiteral(":/crunch/cmdsearch.json"), QStringLiteral("insert cut cells"), QStringLiteral("Insert Cut or Copied Cells"));
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        problem = jptest::commandSearchProblem(w, QStringLiteral(":/crunch/cmdsearch.json"), QStringLiteral("default note size"), QStringLiteral("Reset Note Size"));
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
    }

    // Move or Size Note takes the shown note's box by the keys: arrows move it, Shift with an arrow sizes it, Ctrl makes fine steps, Enter keeps it as one step of Undo, and the sheet tells the
    // mode and the place to a screen reader.
    void aNoteBoxIsMovedAndSizedWithTheKeysInOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.resize(1100, 640);
        w.show();
        crunch::EditSession *s = w.session();
        const jp::office::sheet::NoteBox first = s->anchors().noteAt(0, 2, 2)->box;
        const QString usualName = e.view()->accessibleName(), usualDescription = e.view()->accessibleDescription();
        e.select(QStringLiteral("A1"));
        QVERIFY(!w.act(QStringLiteral("notes.moveSize"))->isEnabled());   // (the note on A1 is hidden)
        e.select(QStringLiteral("C3"));
        QAction *command = w.act(QStringLiteral("notes.moveSize"));
        QVERIFY(command->isEnabled());
        QCOMPARE(command->text(), QStringLiteral("Move or Size Note"));
        const int steps = w.undoStack()->count();
        const QRect was = e.view()->shownNoteRect(2, 2);
        const double z = e.view()->sheet().layout().zoom();
        command->trigger();
        QVERIFY(e.view()->inNoteKeys());
        QVERIFY2(e.view()->accessibleName().contains(QLatin1String("C3")), qPrintable(e.view()->accessibleName()));
        QVERIFY(e.view()->accessibleName() != usualName);
        QVERIFY(e.view()->accessibleDescription().contains(QLatin1String("Enter")));
        const QString modeDescription = e.view()->accessibleDescription();
        const QRect rel0 = e.view()->noteKeysBox();
        const Qt::KeyboardModifier fine = grid::GridView::noteFineModifier();
        QTest::keyClick(e.view(), Qt::Key_Right);
        QTest::keyClick(e.view(), Qt::Key_Down);
        QTest::keyClick(e.view(), Qt::Key_Down, fine);
        QTest::keyClick(e.view(), Qt::Key_Right, Qt::ShiftModifier);
        QTest::keyClick(e.view(), Qt::Key_Down, Qt::ShiftModifier);
        QTest::keyClick(e.view(), Qt::Key_Left, Qt::ShiftModifier | fine);
        const QRect rel = e.view()->noteKeysBox();
        QCOMPARE(rel.x(), rel0.x() + 10);
        QCOMPARE(rel.y(), rel0.y() + 11);
        QCOMPARE(rel.width(), rel0.width() + 10 - 1);
        QCOMPARE(rel.height(), rel0.height() + 10);
        QCOMPARE(e.view()->accessibleDescription(), modeDescription);   // (the steps are said by the announcements, not again in the description)
        QVERIFY(qAbs(e.view()->shownNoteRect(2, 2).left() - (was.left() + int(std::lround(10 * z)))) <= 1);
        // nothing has been changed yet, and a key that is not the box's does nothing to the sheet
        QCOMPARE(w.undoStack()->count(), steps);
        QTest::keyClick(e.view(), Qt::Key_A);
        QTest::keyClick(e.view(), Qt::Key_Delete);
        QVERIFY(!w.isEditing());
        QCOMPARE(e.selected(), QStringLiteral("C3"));
        QVERIFY(s->anchors().noteAt(0, 2, 2)->box == first);
        QTest::keyClick(e.view(), Qt::Key_Return);
        QVERIFY(!e.view()->inNoteKeys());
        QCOMPARE(e.view()->accessibleName(), usualName);
        QCOMPARE(e.view()->accessibleDescription(), usualDescription);
        QCOMPARE(w.undoStack()->count(), steps + 1);
        QCOMPARE(w.undoStack()->undoText(), QStringLiteral("Move or Size Note"));
        const jp::office::sheet::NoteBox kept = s->anchors().noteAt(0, 2, 2)->box;
        QVERIFY(!(kept == first));
        const QRect after = e.view()->shownNoteRect(2, 2);
        QVERIFY(qAbs(after.width() - int(std::lround(rel.width() * z))) <= 1);
        QVERIFY(qAbs(after.left() - (was.left() + int(std::lround(10 * z)))) <= 1);
        pressUndo(&w);
        QVERIFY(s->anchors().noteAt(0, 2, 2)->box == first);
        QCOMPARE(e.view()->shownNoteRect(2, 2), was);
        w.undoStack()->redo();
        QVERIFY(s->anchors().noteAt(0, 2, 2)->box == kept);
    }

    // Escape gives the keys' box up (nothing changed), Enter with no change makes no step, and a click elsewhere, or the keys leaving the sheet, gives it up too.
    void aNoteBoxMovedWithTheKeysCanBeGivenUp()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.resize(1100, 640);
        w.show();
        crunch::EditSession *s = w.session();
        const jp::office::sheet::NoteBox first = s->anchors().noteAt(0, 2, 2)->box;
        const QRect was = e.view()->shownNoteRect(2, 2);
        const int steps = w.undoStack()->count();
        e.select(QStringLiteral("C3"));
        w.act(QStringLiteral("notes.moveSize"))->trigger();
        QVERIFY(e.view()->inNoteKeys());
        QTest::keyClick(e.view(), Qt::Key_Right);
        QVERIFY(e.view()->shownNoteRect(2, 2) != was);   // (drawn where the keys have it)
        QTest::keyClick(e.view(), Qt::Key_Escape);
        QVERIFY(!e.view()->inNoteKeys());
        QCOMPARE(e.view()->shownNoteRect(2, 2), was);
        QCOMPARE(w.undoStack()->count(), steps);
        QVERIFY(s->anchors().noteAt(0, 2, 2)->box == first);
        w.act(QStringLiteral("notes.moveSize"))->trigger();
        QTest::keyClick(e.view(), Qt::Key_Return);   // (not moved: no step)
        QVERIFY(!e.view()->inNoteKeys());
        QCOMPARE(w.undoStack()->count(), steps);
        w.act(QStringLiteral("notes.moveSize"))->trigger();
        QTest::keyClick(e.view(), Qt::Key_Left);
        clickCell(e, QStringLiteral("B2"));
        QVERIFY(!e.view()->inNoteKeys());
        QCOMPARE(w.undoStack()->count(), steps);
        QCOMPARE(e.view()->shownNoteRect(2, 2), was);
        w.act(QStringLiteral("notes.moveSize"))->trigger();   // (the active cell is B2: it has no note, and the command is off)
        QVERIFY(!e.view()->inNoteKeys());
        e.select(QStringLiteral("C3"));
        w.act(QStringLiteral("notes.moveSize"))->trigger();
        QVERIFY(e.view()->inNoteKeys());
        QVERIFY(w.undoStack()->undoText() != QStringLiteral("Move or Size Note"));
        // the notes changing under the keys (an Undo, say) gives the box up
        QVERIFY(s->showNotes(0, {{2, 2}}, false, QStringLiteral("Hide")).ok);
        QVERIFY(!e.view()->inNoteKeys());
    }

    // The keys' box is kept to what the pointer's is: at most 640 by 480, within 1,000 pixels of its cell, and no smaller than a drag can make it.
    void aNoteBoxMovedWithTheKeysKeepsTheLimitsOfTheMouse()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.resize(1100, 640);
        w.show();
        e.select(QStringLiteral("C3"));
        w.act(QStringLiteral("notes.moveSize"))->trigger();
        QVERIFY(e.view()->inNoteKeys());
        for (int i = 0; i < 80; ++i) QTest::keyClick(e.view(), Qt::Key_Right, Qt::ShiftModifier);
        for (int i = 0; i < 60; ++i) QTest::keyClick(e.view(), Qt::Key_Down, Qt::ShiftModifier);
        QCOMPARE(e.view()->noteKeysBox().width(), 640);
        QCOMPARE(e.view()->noteKeysBox().height(), 480);
        for (int i = 0; i < 120; ++i) QTest::keyClick(e.view(), Qt::Key_Right);
        for (int i = 0; i < 160; ++i) QTest::keyClick(e.view(), Qt::Key_Up);
        QCOMPARE(e.view()->noteKeysBox().x(), 1000);
        QCOMPARE(e.view()->noteKeysBox().y(), -1000);
        for (int i = 0; i < 100; ++i) {
            QTest::keyClick(e.view(), Qt::Key_Left, Qt::ShiftModifier);
            QTest::keyClick(e.view(), Qt::Key_Up, Qt::ShiftModifier);
        }
        QCOMPARE(e.view()->noteKeysBox().width(), 24);
        QCOMPARE(e.view()->noteKeysBox().height(), 16);
        QTest::keyClick(e.view(), Qt::Key_Return);
        const jp::office::sheet::NoteBox kept = w.session()->anchors().noteAt(0, 2, 2)->box;
        QVERIFY(kept.set);
        const QRect px = crunch::noteBoxPixels(kept, e.view()->sheet().layout().rows, e.view()->sheet().layout().cols, 2, 2);
        QVERIFY2(px.width() <= 640 && px.height() <= 480, qPrintable(QString::number(px.width()) + "x" + QString::number(px.height())));
        QVERIFY(qAbs(px.x()) <= 1000 && qAbs(px.y()) <= 1000);
    }

    // On a protected sheet the keys' box is refused at Enter in the words the pointer's is, and goes back; Reset Note Size is refused there too.
    void aNoteBoxMovedWithTheKeysOnAProtectedSheetIsRefused()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.resize(1100, 640);
        w.show();
        crunch::EditSession *s = w.session();
        const QRect was = e.view()->shownNoteRect(2, 2);
        const jp::office::sheet::NoteBox first = s->anchors().noteAt(0, 2, 2)->box;
        e.select(QStringLiteral("C3"));
        jp::office::sheet::SheetProtection locked;
        locked.enabled = true;
        locked.objects = true;
        QVERIFY(s->setSheetProtection(0, locked, QStringLiteral("Protect")).ok);   // (a change to the workbook gives the keys' box up, so the sheet is protected first)
        QTRY_VERIFY(w.act(QStringLiteral("notes.moveSize"))->isEnabled());
        w.act(QStringLiteral("notes.moveSize"))->trigger();
        QVERIFY(e.view()->inNoteKeys());
        QTest::keyClick(e.view(), Qt::Key_Right);
        const int steps = w.undoStack()->count();
        e.win.told.clear();
        QTest::keyClick(e.view(), Qt::Key_Return);
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(w.undoStack()->count(), steps);
        QVERIFY(s->anchors().noteAt(0, 2, 2)->box == first);
        QCOMPARE(e.view()->shownNoteRect(2, 2), was);
        e.win.told.clear();
        w.act(QStringLiteral("notes.resetSize"))->setEnabled(true);
        w.act(QStringLiteral("notes.resetSize"))->trigger();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(w.undoStack()->count(), steps);
    }

    // While the keys have a note's box no chord with a modifier other than the fine key (Ctrl; Option on a Mac, where Command is Qt's Control) and Shift reaches the sheet or the box, and the fine key
    // makes 1 pixel steps. Run for each platform's fine key.
    void noChordMovesTheSelectionWhileTheKeysHaveANoteBox()
    {
        struct Reset { ~Reset() { grid::GridView::forceNoteFineModifier(Qt::NoModifier); } } reset;
        for (const Qt::KeyboardModifier fine : {Qt::ControlModifier, Qt::AltModifier}) {
            grid::GridView::forceNoteFineModifier(fine);
            QCOMPARE(grid::GridView::noteFineModifier(), fine);
            EditWindow e;
            QVERIFY(e.open(bookWithBoxes()));
            QVERIFY(e.settle());
            CrunchWindow &w = e.win.w;
            w.resize(1100, 640);
            w.show();
            e.select(QStringLiteral("C3"));
            w.act(QStringLiteral("notes.moveSize"))->trigger();
            QVERIFY(e.view()->inNoteKeys());
            const QRect rel0 = e.view()->noteKeysBox();
            QSignalSpy lists(e.view(), &grid::GridView::dropDownRequested);
            const int steps = w.undoStack()->count();
            const Qt::KeyboardModifiers chordKeys = Qt::AltModifier | Qt::MetaModifier | Qt::ControlModifier;
            for (const Qt::Key key : {Qt::Key_Right, Qt::Key_Down, Qt::Key_Left, Qt::Key_Up, Qt::Key_Home, Qt::Key_PageDown, Qt::Key_Return, Qt::Key_Tab, Qt::Key_F2, Qt::Key_Delete, Qt::Key_A}) {
                for (int mask = 0; mask < 16; ++mask) {
                    Qt::KeyboardModifiers m;
                    if (mask & 1) m |= Qt::AltModifier;
                    if (mask & 2) m |= Qt::MetaModifier;
                    if (mask & 4) m |= Qt::ControlModifier;
                    if (mask & 8) m |= Qt::ShiftModifier;
                    if (!(m & chordKeys & ~Qt::KeyboardModifiers(fine))) continue;   // (only the chords that are not the fine key's or Shift's own)
                    QTest::keyClick(e.view(), key, m);
                    QVERIFY2(e.selected() == QStringLiteral("C3") && e.view()->noteKeysBox() == rel0 && e.view()->inNoteKeys() && !w.isEditing(),
                             qPrintable(QStringLiteral("key %1 modifiers %2 fine %3").arg(int(key)).arg(int(m)).arg(int(fine))));
                }
            }
            QCOMPARE(lists.size(), 0);
            QCOMPARE(w.undoStack()->count(), steps);
            QTest::keyClick(e.view(), Qt::Key_Right, fine);
            QCOMPARE(e.view()->noteKeysBox().x(), rel0.x() + 1);
            QTest::keyClick(e.view(), Qt::Key_Down, fine | Qt::ShiftModifier);
            QCOMPARE(e.view()->noteKeysBox().height(), rel0.height() + 1);
            QTest::keyClick(e.view(), Qt::Key_Down);
            QCOMPARE(e.view()->noteKeysBox().y(), rel0.y() + 10);
            QCOMPARE(e.selected(), QStringLiteral("C3"));
        }
    }

    // Any sheet switch, any other command, an Undo, a Redo, and a change to the workbook end the keys' hold of a note's box (even when the other sheet has the same note); running Move or Size Note
    // while it is on does nothing, and what was moved is kept.
    void theKeysHoldOfANoteBoxEndsWithAnythingElseThatHappens()
    {
        EditWindow e;
        QVERIFY(e.open(bookOfSheets({{"A1=1"}, {"A1=2"}})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.resize(1100, 640);
        w.show();
        crunch::EditSession *s = w.session();
        jp::office::sheet::NoteBox box;
        box.set = true;
        box.col1 = 2;
        box.col2 = 4;
        box.row1 = 0;
        box.row2 = 3;
        for (int sheet = 0; sheet < 2; ++sheet) {   // (the same note, shown, with the same box, on both sheets)
            QVERIFY(s->setNote(sheet, 0, 0, QStringLiteral("Same"), QStringLiteral("me"), true, QStringLiteral("Add Note")).ok);
            QVERIFY(s->setNoteBox(sheet, 0, 0, box, QStringLiteral("Move or Size Note")).ok);
        }
        QAction *command = w.act(QStringLiteral("notes.moveSize"));
        e.select(QStringLiteral("A1"));
        QTRY_VERIFY(command->isEnabled());
        // another sheet
        command->trigger();
        QVERIFY(e.view()->inNoteKeys());
        w.sheetTabs()->setCurrentIndex(1);
        QTRY_COMPARE(w.activeSheet(), 1);
        QVERIFY(!e.view()->inNoteKeys());
        w.sheetTabs()->setCurrentIndex(0);
        QTRY_COMPARE(w.activeSheet(), 0);
        e.select(QStringLiteral("A1"));
        // running the command again while it is on: it stays on, and what was moved is not lost
        const int steps = w.undoStack()->count();
        command->trigger();
        QVERIFY(e.view()->inNoteKeys());
        QTest::keyClick(e.view(), Qt::Key_Right);
        QTest::keyClick(e.view(), Qt::Key_Right);
        const QRect moved = e.view()->noteKeysBox();
        command->trigger();
        QVERIFY(e.view()->inNoteKeys());
        QCOMPARE(e.view()->noteKeysBox(), moved);
        QTest::keyClick(e.view(), Qt::Key_Return);
        QVERIFY(!e.view()->inNoteKeys());
        QCOMPARE(w.undoStack()->count(), steps + 1);
        const jp::office::sheet::NoteBox kept = s->anchors().noteAt(0, 0, 0)->box;
        QVERIFY(!(kept == box));
        // another command
        command->trigger();
        QVERIFY(e.view()->inNoteKeys());
        w.act(QStringLiteral("edit.copy"))->trigger();
        QVERIFY(!e.view()->inNoteKeys());
        // an Undo and a Redo
        command->trigger();
        QVERIFY(e.view()->inNoteKeys());
        w.undoStack()->undo();
        QVERIFY(!e.view()->inNoteKeys());
        command->trigger();
        QVERIFY(e.view()->inNoteKeys());
        w.undoStack()->redo();
        QVERIFY(!e.view()->inNoteKeys());
        // a change to the workbook
        command->trigger();
        QVERIFY(e.view()->inNoteKeys());
        QVERIFY(s->type(0, 5, 5, QStringLiteral("x")).ok);
        QVERIFY(!e.view()->inNoteKeys());
    }

    // The keys leaving the sheet give the box up, except for a menu or popup that takes them for a moment.
    void theKeysHoldOfANoteBoxEndsWhenTheSheetLosesTheFocus()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.resize(1100, 640);
        w.show();
        e.select(QStringLiteral("C3"));
        w.act(QStringLiteral("notes.moveSize"))->trigger();
        QVERIFY(e.view()->inNoteKeys());
        QFocusEvent popup(QEvent::FocusOut, Qt::PopupFocusReason);
        QCoreApplication::sendEvent(e.view(), &popup);
        QVERIFY(e.view()->inNoteKeys());
        QFocusEvent away(QEvent::FocusOut, Qt::TabFocusReason);
        QCoreApplication::sendEvent(e.view(), &away);
        QVERIFY(!e.view()->inNoteKeys());
    }

    // The keys' box is announced once at each key press and not at each repeat of a key held down (the place is said when it is let go), the beginning and the end are said, and the place says
    // its unit. The announcements are the events an accessibility client gets.
    void theKeysBoxIsAnnouncedOnceAKeyAndNotAtEachRepeat()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.resize(1100, 640);
        w.show();
        e.select(QStringLiteral("C3"));
        Announcements said;
        w.act(QStringLiteral("notes.moveSize"))->trigger();
        QCOMPARE(Announcements::words.size(), 1);
        QVERIFY2(Announcements::words.last().contains(QLatin1String("C3")) && Announcements::words.last().contains(QLatin1String("100%")), qPrintable(Announcements::words.join('|')));
        const int x0 = e.view()->noteKeysBox().x();
        QTest::keyClick(e.view(), Qt::Key_Right);
        QCOMPARE(Announcements::words.size(), 2);
        QCOMPARE(Announcements::words.last(), QStringLiteral("%1 across, %2 down, %3 wide, %4 high, in pixels at 100% zoom").arg(x0 + 10).arg(e.view()->noteKeysBox().y()).arg(e.view()->noteKeysBox().width()).arg(e.view()->noteKeysBox().height()));
        // a key held down: the first press is said, the repeats are not, and the release says where it ended
        QKeyEvent press(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier, QString(), false), repeat(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier, QString(), true);
        QKeyEvent repeatUp(QEvent::KeyRelease, Qt::Key_Right, Qt::NoModifier, QString(), true), up(QEvent::KeyRelease, Qt::Key_Right, Qt::NoModifier, QString(), false);
        QCoreApplication::sendEvent(e.view(), &press);
        QCOMPARE(Announcements::words.size(), 3);
        for (int i = 0; i < 5; ++i) {
            QCoreApplication::sendEvent(e.view(), &repeatUp);
            QCoreApplication::sendEvent(e.view(), &repeat);
        }
        QCOMPARE(e.view()->noteKeysBox().x(), x0 + 70);   // (the box went on moving)
        QCOMPARE(Announcements::words.size(), 3);
        QCoreApplication::sendEvent(e.view(), &up);
        QCOMPARE(Announcements::words.size(), 4);
        QVERIFY2(Announcements::words.last().startsWith(QString::number(x0 + 70) + QLatin1String(" across")), qPrintable(Announcements::words.last()));
        QCoreApplication::sendEvent(e.view(), &up);   // (a release alone says nothing more)
        QCOMPARE(Announcements::words.size(), 4);
        // the end is said, kept or given up
        QTest::keyClick(e.view(), Qt::Key_Return);
        QCOMPARE(Announcements::words.size(), 5);
        QVERIFY2(Announcements::words.last().contains(QLatin1String("kept")), qPrintable(Announcements::words.last()));
        w.act(QStringLiteral("notes.moveSize"))->trigger();
        QTest::keyClick(e.view(), Qt::Key_Escape);
        QVERIFY2(Announcements::words.last().contains(QLatin1String("given up")), qPrintable(Announcements::words.last()));
        QCOMPARE(Announcements::words.size(), 7);
    }

    // Reset Note Size puts the box back to the default (beside its cell, as large as its words need) in one step of Undo, is off when the box is already the default, and is saved.
    void resetNoteSizePutsTheBoxBackToTheDefault()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithBoxes()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.resize(1100, 640);
        w.show();
        crunch::EditSession *s = w.session();
        const jp::office::sheet::NoteBox first = s->anchors().noteAt(0, 2, 2)->box;
        QVERIFY(first.set);
        e.select(QStringLiteral("C3"));
        QAction *reset = w.act(QStringLiteral("notes.resetSize"));
        QCOMPARE(reset->text(), QStringLiteral("Reset Note Size"));
        QVERIFY(reset->isEnabled());
        const int steps = w.undoStack()->count();
        reset->trigger();
        QCOMPARE(w.undoStack()->count(), steps + 1);
        QCOMPARE(w.undoStack()->undoText(), QStringLiteral("Reset Note Size"));
        QVERIFY(!s->anchors().noteAt(0, 2, 2)->box.set);
        const QRect c3 = e.view()->sheet().layout().cellRect(2, 2);
        const QRect box = e.view()->shownNoteRect(2, 2);
        QVERIFY(!box.isEmpty());
        QVERIFY(box.left() > c3.right());   // (beside the cell)
        QVERIFY(!reset->isEnabled());
        pressUndo(&w);
        QVERIFY(s->anchors().noteAt(0, 2, 2)->box == first);
        QVERIFY(reset->isEnabled());
        w.undoStack()->redo();
        // saved: the note is written with no anchor of its own choosing, and reads back without a box
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        const jp::office::sheet::NoteInfo *again = second.w.session()->anchors().noteAt(0, 2, 2);
        QVERIFY(again && again->shown);
        // (the file's form has no "no box": the default anchor is written, and read back as the box beside the cell, not the one it had)
        QVERIFY(again->box.set && !(again->box == first));
        QCOMPARE(again->box.y1, 10);
        QCOMPARE(again->box.y2, 4);
        QCOMPARE(again->box.col1, 1);
        QCOMPARE(again->box.row1, -1);
        QCOMPARE(again->box.col2, 3);
        QCOMPARE(again->box.row2, 2);
    }
};

JP_TEST_CLASS(CrunchGapTests)
#include "tst_crunch_gaps.moc"
