// Tests of the sheets of a workbook as a person changes them through the edit session: a sheet inserted, copied, renamed, deleted, moved, hidden, shown, and given a color; each one step of
// Undo, the formulas that name a sheet following it, the names a sheet may have, and what is refused. The workbook is built here.
#include "app/crunchwindow.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"

#include <QSignalSpy>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;
using namespace jp::office::sheet;

namespace {

QString cell(const QString &ref, const QString &value) { return QStringLiteral("<c r=\"%1\"><v>%2</v></c>").arg(ref, value); }
QString formula(const QString &ref, const QString &text) { return QStringLiteral("<c r=\"%1\"><f>%2</f><v>0</v></c>").arg(ref, text.toHtmlEscaped()); }
QString row(int n, const QString &cells) { return QStringLiteral("<row r=\"%1\">%2</row>").arg(n).arg(cells); }

//   Sheet1: A1 = 1, B1 = Sheet2!A1 + 1, C1 = SUM(Sheet2!A1:A2)
//   Sheet2: A1 = 10, A2 = Sheet1!A1 * 2
//   Sheet3: A1 = 7, C1 = Sheet1!A1 + Sheet3!A1
QByteArray sheetsBook(const QStringList &names = QStringList())
{
    const QByteArray one = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("1")) + formula(QStringLiteral("B1"), QStringLiteral("Sheet2!A1+1")) + formula(QStringLiteral("C1"), QStringLiteral("SUM(Sheet2!A1:A2)"))));
    const QByteArray two = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("10"))) + row(2, formula(QStringLiteral("A2"), QStringLiteral("Sheet1!A1*2"))));
    const QByteArray three = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("7")) + formula(QStringLiteral("C1"), QStringLiteral("Sheet1!A1+Sheet3!A1"))));
    return buildXlsx({one, two, three}, QByteArray(), QString(), QString(), QByteArray(), names);
}

QString valueOf(EditWindow &e, int sheet, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return e.win.w.session()->valueAt(sheet, r, c).debug();
}

QString textOf(EditWindow &e, int sheet, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    return e.win.w.session()->editText(sheet, r, c);
}

} // namespace

class CrunchSheetsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // A sheet inserted is before the one named (or last), has the next name, is empty, and can be typed into; Undo takes it away, Redo brings it back with its place, and the other sheets keep
    // their places throughout.
    void aSheetIsInsertedAndUndone()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        crunch::EditSession *s = e.win.w.session();
        QSignalSpy changed(s, &crunch::EditSession::sheetsChanged);
        QCOMPARE(s->sheetOrder(), QVector<int>({0, 1, 2}));
        int made = -1;
        QVERIFY(s->insertSheet(1, QString(), &made).ok);
        QCOMPARE(made, 3);
        QCOMPARE(s->sheetOrder(), QVector<int>({0, 3, 1, 2}));
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Insert Sheet"));
        QCOMPARE(e.win.w.calculation()->sheetCount(), 4);
        QCOMPARE(e.win.w.loaded()->book->info().sheets[3].name, QStringLiteral("Sheet4"));
        QVERIFY(changed.count() >= 1);
        QVERIFY(s->canEditCell(3, 0, 0));
        QVERIFY(s->type(3, 0, 0, QStringLiteral("5")).ok);
        QVERIFY(s->type(0, 4, 0, QStringLiteral("=Sheet4!A1*2")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, 0, QStringLiteral("A5")), QStringLiteral("n:10"));
        // Undo the entries and the sheet
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(s->sheetOrder(), QVector<int>({0, 1, 2}));
        QCOMPARE(e.win.w.calculation()->sheetCount(), 3);
        QCOMPARE(e.win.w.loaded()->book->info().sheets.size(), 3);
        // a sheet at the end, then Redo
        QVERIFY(s->insertSheet(-1, QStringLiteral("Totals")).ok);
        QCOMPARE(s->sheetOrder(), QVector<int>({0, 1, 2, 3}));
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->redo();
        QCOMPARE(s->sheetOrder(), QVector<int>({0, 1, 2, 3}));
        QCOMPARE(e.win.w.loaded()->book->info().sheets[3].name, QStringLiteral("Totals"));
        // the other sheets are as they were
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, 0, QStringLiteral("B1")), QStringLiteral("n:11"));
        QCOMPARE(valueOf(e, 2, QStringLiteral("C1")), QStringLiteral("n:8"));
    }

    // The names a sheet may have, and that a name that is refused changes nothing.
    void sheetNamesAreChecked()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        crunch::EditSession *s = e.win.w.session();
        QString why;
        QVERIFY(!s->validSheetName(QString(), -1, &why) && !why.isEmpty());
        QVERIFY(!s->validSheetName(QStringLiteral("   "), -1));
        QVERIFY(!s->validSheetName(QString(32, QLatin1Char('x')), -1, &why));
        QVERIFY(s->validSheetName(QString(31, QLatin1Char('x')), -1));
        for (const char *bad : {"a:b", "a\\b", "a/b", "a?b", "a*b", "a[b", "a]b", "'quoted", "quoted'", "History", "history"}) QVERIFY2(!s->validSheetName(QString::fromLatin1(bad), -1), bad);
        QVERIFY(!s->validSheetName(QStringLiteral("sheet2"), -1, &why));   // (any case)
        QVERIFY(why.contains(QStringLiteral("Sheet2")));
        QVERIFY(s->validSheetName(QStringLiteral("sheet2"), 1));            // (its own name is no clash)
        QVERIFY(s->validSheetName(QStringLiteral("Q1 2024 (draft)"), -1));
        QVERIFY(s->validSheetName(QStringLiteral("Ünïcode 売上"), -1));
        QCOMPARE(s->newSheetName(), QStringLiteral("Sheet4"));
        QCOMPARE(s->copyName(1), QStringLiteral("Sheet2 (2)"));
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(!s->renameSheet(1, QStringLiteral("Sheet3")).ok);
        QVERIFY(!s->insertSheet(-1, QStringLiteral("a:b")).ok);
        QVERIFY(s->renameSheet(1, QStringLiteral("Sheet2")).ok);   // (the same name: nothing to do)
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        // a copy's name counts up
        QVERIFY(s->copySheet(1, -1, QString()).ok);
        QCOMPARE(e.win.w.loaded()->book->info().sheets[3].name, QStringLiteral("Sheet2 (2)"));
        QCOMPARE(s->copyName(3), QStringLiteral("Sheet2 (3)"));
        QCOMPARE(s->copyName(1), QStringLiteral("Sheet2 (3)"));
    }

    // Renaming: the formulas that name the sheet name the new name, the values do not change, and Undo and Redo go both ways.
    void renamingRewritesTheFormulasThatNameTheSheet()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        QVERIFY(s->renameSheet(1, QStringLiteral("Data 2024")).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Rename Sheet"));
        QCOMPARE(textOf(e, 0, QStringLiteral("B1")), QStringLiteral("='Data 2024'!A1+1"));
        QCOMPARE(textOf(e, 0, QStringLiteral("C1")), QStringLiteral("=SUM('Data 2024'!A1:A2)"));
        QCOMPARE(textOf(e, 1, QStringLiteral("A2")), QStringLiteral("=Sheet1!A1*2"));
        QCOMPARE(valueOf(e, 0, QStringLiteral("B1")), QStringLiteral("n:11"));
        QCOMPARE(valueOf(e, 0, QStringLiteral("C1")), QStringLiteral("n:12"));
        QCOMPARE(e.win.w.loaded()->book->info().sheets[1].name, QStringLiteral("Data 2024"));
        QCOMPARE(e.win.w.loaded()->book->info().sheets[1].fileName, QStringLiteral("Sheet2"));
        // a formula typed names it as it is called now, and what is changed in it is read by the formulas
        QVERIFY(s->type(2, 2, 0, QStringLiteral("='Data 2024'!A1*3")).ok);
        QVERIFY(s->type(1, 0, 0, QStringLiteral("20")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, 2, QStringLiteral("A3")), QStringLiteral("n:60"));
        QCOMPARE(valueOf(e, 0, QStringLiteral("B1")), QStringLiteral("n:21"));
        // Undo
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(textOf(e, 0, QStringLiteral("B1")), QStringLiteral("=Sheet2!A1+1"));
        QCOMPARE(e.win.w.loaded()->book->info().sheets[1].name, QStringLiteral("Sheet2"));
        QCOMPARE(valueOf(e, 0, QStringLiteral("B1")), QStringLiteral("n:11"));
        // Redo gives the new name again
        e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QCOMPARE(textOf(e, 0, QStringLiteral("B1")), QStringLiteral("='Data 2024'!A1+1"));
        QCOMPARE(valueOf(e, 0, QStringLiteral("C1")), QStringLiteral("n:12"));
    }

    // Deleting: the formulas that read the sheet have #REF! (and so the error), the sheet keeps its place for Undo, the last sheet that is shown can't be deleted, and Undo gives it back with
    // everything it read.
    void deletingMakesRefsAndUndoGivesItBack()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        QVERIFY(s->sheetHasCells(1));
        QVERIFY(s->deleteSheet(1).ok);
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Delete Sheet"));
        QCOMPARE(s->sheetOrder(), QVector<int>({0, 2}));
        QCOMPARE(textOf(e, 0, QStringLiteral("B1")), QStringLiteral("=#REF!+1"));
        QCOMPARE(textOf(e, 0, QStringLiteral("C1")), QStringLiteral("=SUM(#REF!)"));
        QVERIFY(valueOf(e, 0, QStringLiteral("B1")).startsWith(QLatin1String("e:")));
        QCOMPARE(valueOf(e, 2, QStringLiteral("C1")), QStringLiteral("n:8"));   // (the third sheet keeps its place)
        QVERIFY(!s->canEditSheet(1));
        QVERIFY(!s->type(1, 0, 0, QStringLiteral("1")).ok);
        QCOMPARE(e.win.w.calculation()->sheetCount(), 3);
        // the name is free again
        QVERIFY(s->renameSheet(2, QStringLiteral("Sheet2")).ok);
        e.win.w.undoStack()->undo();
        // Undo
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(s->sheetOrder(), QVector<int>({0, 1, 2}));
        QCOMPARE(textOf(e, 0, QStringLiteral("B1")), QStringLiteral("=Sheet2!A1+1"));
        QCOMPARE(valueOf(e, 0, QStringLiteral("B1")), QStringLiteral("n:11"));
        QCOMPARE(valueOf(e, 0, QStringLiteral("C1")), QStringLiteral("n:12"));
        QVERIFY(s->canEditSheet(1));
        // Redo
        e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QCOMPARE(textOf(e, 0, QStringLiteral("B1")), QStringLiteral("=#REF!+1"));
        // down to one sheet that is shown: the last can't go
        e.win.w.undoStack()->undo();
        QVERIFY(s->deleteSheet(1).ok);
        QVERIFY(s->deleteSheet(2).ok);
        const int steps = e.win.w.undoStack()->count();
        const auto last = s->deleteSheet(0);
        QVERIFY(!last.ok);
        QVERIFY(last.problem.contains(QStringLiteral("at least one")));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(!s->setSheetHidden(0, true).ok);
    }

    void aSheetIsMovedHiddenAndColored()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->moveSheet(0, -1).ok);
        QCOMPARE(s->sheetOrder(), QVector<int>({1, 2, 0}));
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Move Sheet"));
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(s->moveSheet(0, -1).ok);                       // (already last: no step)
        QVERIFY(s->moveSheet(1, 2).ok);                        // (already before it: no step)
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(s->moveSheet(2, 1).ok);
        e.win.w.undoStack()->undo();
        QVERIFY(s->moveSheet(0, 1).ok);
        QCOMPARE(s->sheetOrder(), QVector<int>({0, 1, 2}));
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QCOMPARE(s->sheetOrder(), QVector<int>({0, 1, 2}));
        // hidden: the sheet stays in the order and its place; it can be shown again
        QVERIFY(s->setSheetHidden(1, true).ok);
        QCOMPARE(e.win.w.loaded()->book->info().sheets[1].state, SheetState::Hidden);
        QCOMPARE(s->sheetOrder(), QVector<int>({0, 1, 2}));
        QVERIFY(s->setSheetHidden(1, false).ok);
        QCOMPARE(e.win.w.loaded()->book->info().sheets[1].state, SheetState::Visible);
        e.win.w.undoStack()->undo();
        QCOMPARE(e.win.w.loaded()->book->info().sheets[1].state, SheetState::Hidden);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Hide Sheet"));
        // a color
        QVERIFY(s->setTabColor(2, ColorSpec::fromRgb(0xC00000)).ok);
        QVERIFY(e.win.w.calculation()->sheet(2).tabColor == ColorSpec::fromRgb(0xC00000));
        e.win.w.undoStack()->undo();
        QVERIFY(!e.win.w.calculation()->sheet(2).tabColor.isSet());
        QVERIFY(s->setTabColor(2, ColorSpec()).ok);   // (none to none: no step)
    }

    // A copy: the cells, the formulas (that read the sheet they are on, and the sheets they named), put where asked; the copy is its own sheet, and Undo takes it away.
    void aSheetIsCopied()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        int made = -1;
        QVERIFY(s->copySheet(1, 2, QStringLiteral("Copy of two"), &made).ok);
        QCOMPARE(made, 3);
        QCOMPARE(s->sheetOrder(), QVector<int>({0, 1, 3, 2}));
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Copy Sheet"));
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, 3, QStringLiteral("A1")), QStringLiteral("n:10"));
        QCOMPARE(valueOf(e, 3, QStringLiteral("A2")), QStringLiteral("n:2"));
        QCOMPARE(textOf(e, 3, QStringLiteral("A2")), QStringLiteral("=Sheet1!A1*2"));
        QVERIFY(s->type(3, 0, 0, QStringLiteral("99")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, 1, QStringLiteral("A1")), QStringLiteral("n:10"));
        QCOMPARE(valueOf(e, 0, QStringLiteral("B1")), QStringLiteral("n:11"));
        e.win.w.undoStack()->undo();
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.calculation()->sheetCount(), 3);
        QCOMPARE(s->sheetOrder(), QVector<int>({0, 1, 2}));
        QVERIFY(!s->copySheet(7, -1, QString()).ok);
    }

    // A formula over a block of sheets is not given to adding, renaming, moving, or deleting (its meaning is by the order of the sheets).
    void aBlockOfSheetsRefusesTheChanges()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->type(2, 5, 0, QStringLiteral("=SUM(Sheet1:Sheet3!A1)")).ok);
        QVERIFY(e.settle());
        const int steps = e.win.w.undoStack()->count();
        for (const auto &o : {s->insertSheet(-1, QString()), s->renameSheet(1, QStringLiteral("X")), s->deleteSheet(1), s->moveSheet(0, -1), s->copySheet(1, -1, QString())}) {
            QVERIFY(!o.ok);
            QVERIFY2(o.problem.contains(QStringLiteral("block of sheets")), qPrintable(o.problem));
        }
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QVERIFY(s->setSheetHidden(1, true).ok);   // (hiding and colors do not change what a formula reads)
    }
};

JP_TEST_CLASS(CrunchSheetsTests)
#include "tst_crunch_sheets.moc"
