// Tests of the sheets of a workbook as a person changes them through the edit session: a sheet inserted, copied, renamed, deleted, moved, hidden, shown, and given a color; each one step of
// Undo, the formulas that name a sheet following it, the names a sheet may have, and what is refused. The workbook is built here.
#include "app/crunchwindow.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"

#include <QLineEdit>
#include <QMenu>
#include <QSignalSpy>
#include <QToolButton>
#include <QWidgetAction>
#include "app/sheettabs.h"
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
QByteArray sheetsBook(const QStringList &names = QStringList(), const QString &definedNames = QString(), const QString &bookViews = QString())
{
    const QByteArray one = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("1")) + formula(QStringLiteral("B1"), QStringLiteral("Sheet2!A1+1")) + formula(QStringLiteral("C1"), QStringLiteral("SUM(Sheet2!A1:A2)"))));
    const QByteArray two = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("10"))) + row(2, formula(QStringLiteral("A2"), QStringLiteral("Sheet1!A1*2"))));
    const QByteArray three = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("7")) + formula(QStringLiteral("C1"), QStringLiteral("Sheet1!A1+Sheet3!A1"))));
    return buildXlsx({one, two, three}, QByteArray(), QString(), QString(), QByteArray(), names, definedNames, QByteArray(), bookViews);
}

// A saved workbook, read again.
struct Saved {
    Package package;
    Workbook book;
    bool open(const QString &path)
    {
        QString why;
        return package.readLazyFile(path, &why) && book.open(package, &why);
    }
    QStringList names() const
    {
        QStringList out;
        for (const SheetEntry &e : book.info().sheets) out << e.name;
        return out;
    }
    QString formula(int sheet, const QString &ref) const
    {
        Sheet s;
        if (!book.readSheet(sheet, &s)) return QStringLiteral("unreadable");
        const Cell *c = at(s, ref);
        return c ? s.formulaText(*c) : QStringLiteral("-");
    }
    QString shown(int sheet, const QString &ref) const
    {
        Sheet s;
        if (!book.readSheet(sheet, &s)) return QStringLiteral("unreadable");
        return show(s, book.sharedStrings(), ref);
    }
};

bool saveNow(EditWindow &e)
{
    if (!e.settle()) return false;
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    if (!e.win.w.save()) return false;
    return saved.wait(20000) && saved.last().value(1).toBool();
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

    // The file saved after sheets were added, renamed, moved, hidden, and colored: its list of sheets (in the order of the tabs, by the names they have, hidden ones marked), the formulas that
    // name a renamed sheet, a new sheet with what was put in it, the names that belong to one sheet at their places, and the tab that is open where it is now.
    void theSavedFileHasTheSheetsAsTheyAre()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook(QStringList(), QStringLiteral("<definedName name=\"Everywhere\">Sheet2!$A$1:$A$2</definedName><definedName name=\"Local\" localSheetId=\"2\">Sheet3!$A$1</definedName>"),
                                  QStringLiteral("<workbookView activeTab=\"2\" firstSheet=\"1\"/>"))));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        QVERIFY(s->renameSheet(1, QStringLiteral("Data 2024")).ok);
        int made = -1;
        QVERIFY(s->insertSheet(1, QStringLiteral("Totals"), &made).ok);
        QCOMPARE(made, 3);
        QVERIFY(s->type(3, 0, 0, QStringLiteral("5")).ok);
        QVERIFY(s->type(3, 1, 0, QStringLiteral("=A1+'Data 2024'!A1")).ok);
        QVERIFY(s->type(0, 4, 0, QStringLiteral("=Totals!A2*2")).ok);
        QVERIFY(s->moveSheet(2, 0).ok);            // Sheet3 first
        QVERIFY(s->setSheetHidden(0, true).ok);    // Sheet1 hidden
        QVERIFY(s->setTabColor(1, ColorSpec::fromRgb(0xC00000)).ok);
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        // the places of the sheets in the new file are the order of the tabs
        QCOMPARE(f.names(), QStringList({QStringLiteral("Sheet3"), QStringLiteral("Sheet1"), QStringLiteral("Totals"), QStringLiteral("Data 2024")}));
        QCOMPARE(f.book.info().sheets[1].state, SheetState::Hidden);
        QCOMPARE(f.book.info().sheets[0].state, SheetState::Visible);
        // what names the renamed sheet follows it, in the cells, the names, and the new sheet
        QCOMPARE(f.formula(1, QStringLiteral("B1")), QStringLiteral("'Data 2024'!A1+1"));
        QCOMPARE(f.formula(1, QStringLiteral("C1")), QStringLiteral("SUM('Data 2024'!A1:A2)"));
        QCOMPARE(f.formula(0, QStringLiteral("C1")), QStringLiteral("Sheet1!A1+Sheet3!A1"));
        QCOMPARE(f.formula(1, QStringLiteral("A5")), QStringLiteral("Totals!A2*2"));
        QCOMPARE(f.shown(2, QStringLiteral("A1")), QStringLiteral("n:5"));
        QCOMPARE(f.formula(2, QStringLiteral("A2")), QStringLiteral("A1+'Data 2024'!A1"));
        QCOMPARE(f.shown(2, QStringLiteral("A2")), QStringLiteral("n:15"));
        QCOMPARE(f.shown(1, QStringLiteral("A5")), QStringLiteral("n:30"));
        QCOMPARE(f.shown(1, QStringLiteral("B1")), QStringLiteral("n:11"));
        // the names of the workbook: the one for all sheets follows the sheet's new name, the one for a sheet has that sheet's new place
        const QByteArray workbook = f.package.part(f.book.mainPart());
        QVERIFY2(workbook.contains(">'Data 2024'!$A$1:$A$2</definedName>") || workbook.contains(">&apos;Data 2024&apos;!$A$1:$A$2</definedName>"), workbook.constData());
        QVERIFY2(workbook.contains("localSheetId=\"0\""), workbook.constData());
        QVERIFY(!workbook.contains("localSheetId=\"2\""));
        // the sheet that was open (the third, Sheet3) is the first now
        QCOMPARE(f.book.info().activeTab, 0);
        // the file read back by the window: the same
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(second.w.shownSheets().size(), 3);   // (Sheet1 is hidden)
        QVERIFY(second.w.calculation()->sheet(3).tabColor == ColorSpec::fromRgb(0xC00000));   // (the color of "Data 2024", which is last now)
        QVERIFY(!second.w.calculation()->sheet(0).tabColor.isSet());
        // the new sheet has a part of its own, named in the content types and the relationships
        const SheetEntry &made2 = f.book.info().sheets[2];
        QVERIFY(!made2.part.isEmpty() && f.package.hasPart(made2.part));
        QVERIFY(f.package.contentType(made2.part).contains(QStringLiteral("worksheet")));
        QVERIFY(!f.package.relationship(f.book.mainPart(), made2.relId).id.isEmpty());
        // after the save Undo does not go back over what the file has made of the sheets (adding, renaming, and moving them), but over what a save can take back: the color and hiding
        QVERIFY(s->canUndo());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Tab Color"));
        e.win.w.undoStack()->undo();
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Hide Sheet"));
        e.win.w.undoStack()->undo();
        QVERIFY(!s->canUndo());
        e.win.w.undoStack()->redo();
        e.win.w.undoStack()->redo();
        // a change after the save is put in as any other
        QVERIFY(s->type(3, 2, 0, QStringLiteral("8")).ok);
        QVERIFY(saveNow(e));
        Saved g;
        QVERIFY(g.open(e.path));
        QCOMPARE(g.shown(2, QStringLiteral("A3")), QStringLiteral("n:8"));
        QCOMPARE(g.names(), f.names());
    }

    // A sheet deleted is not in the saved file (nor are the parts only it had), what read it has #REF!, and the names that were its own are gone with it.
    void aDeletedSheetIsNotInTheSavedFile()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook(QStringList(), QStringLiteral("<definedName name=\"Here\" localSheetId=\"1\">Sheet2!$A$1</definedName><definedName name=\"There\" localSheetId=\"2\">Sheet3!$A$1</definedName>"))));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        const QString part = e.win.w.loaded()->book->info().sheets[1].part;
        QVERIFY(!part.isEmpty());
        QVERIFY(s->deleteSheet(1).ok);
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        QCOMPARE(f.names(), QStringList({QStringLiteral("Sheet1"), QStringLiteral("Sheet3")}));
        QVERIFY(!f.package.hasPart(part));
        QCOMPARE(f.formula(0, QStringLiteral("B1")), QStringLiteral("#REF!+1"));
        QCOMPARE(f.formula(0, QStringLiteral("C1")), QStringLiteral("SUM(#REF!)"));
        QCOMPARE(f.formula(1, QStringLiteral("C1")), QStringLiteral("Sheet1!A1+Sheet3!A1"));
        const QByteArray workbook = f.package.part(f.book.mainPart());
        QVERIFY2(!workbook.contains("name=\"Here\""), workbook.constData());
        QVERIFY2(workbook.contains("name=\"There\"") && workbook.contains("localSheetId=\"1\""), workbook.constData());
        QVERIFY(!f.package.relationshipsOfType(f.book.mainPart(), QStringLiteral("/worksheet")).isEmpty());
        QCOMPARE(f.package.relationshipsOfType(f.book.mainPart(), QStringLiteral("/worksheet")).size(), 2);
        // the window reads it
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(second.w.shownSheets().size(), 2);
    }

    // A copy is saved whole: its cells and formulas (with the results the window had worked out), its sizes, and its place.
    void aCopyIsSavedWithItsCellsAndFormulas()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        crunch::EditSession::SizeChange by;
        by.setSize = true;
        by.size = 30;
        QVERIFY(s->resizeColumns(1, 0, 0, by, QStringLiteral("Changing column width")).ok);
        QVERIFY(s->copySheet(1, -1, QStringLiteral("Second copy")).ok);
        QVERIFY(e.settle());
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        QCOMPARE(f.names(), QStringList({QStringLiteral("Sheet1"), QStringLiteral("Sheet2"), QStringLiteral("Sheet3"), QStringLiteral("Second copy")}));
        QCOMPARE(f.shown(3, QStringLiteral("A1")), QStringLiteral("n:10"));
        QCOMPARE(f.formula(3, QStringLiteral("A2")), QStringLiteral("Sheet1!A1*2"));
        QCOMPARE(f.shown(3, QStringLiteral("A2")), QStringLiteral("n:2"));
        Sheet copy;
        QVERIFY(f.book.readSheet(3, &copy));
        QVERIFY(!copy.columns.isEmpty());
        QCOMPARE(copy.columns.first().width, 30.0);
        // the file read again has the sheet in the window
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(second.w.shownSheets().size(), 4);
        QCOMPARE(second.w.session()->valueAt(3, 1, 0).debug(), QStringLiteral("n:2"));
    }
};

namespace {

QStringList tabNames(EditWindow &e)
{
    QStringList out;
    for (int i = 0; i < e.win.w.sheetTabs()->count(); ++i) out << e.win.w.sheetTabs()->tabData(i).toString();
    return out;
}

void run(EditWindow &e, const char *id)
{
    QAction *a = e.win.w.act(QString::fromLatin1(id));
    QVERIFY2(a && a->isEnabled(), id);
    a->trigger();
}

QMenu *menuOf(EditWindow &e, const char *name)
{
    const auto menus = e.win.w.findChildren<QMenu *>(QString::fromLatin1(name));
    return menus.isEmpty() ? nullptr : menus.first();
}

// The menu a tab's menu holds under the title, made ready to show.
QMenu *submenuOf(QMenu *menu, const QString &title)
{
    for (QAction *a : menu->actions())
        if (a->menu() && a->text() == title) {
            Q_EMIT a->menu()->aboutToShow();
            return a->menu();
        }
    return nullptr;
}

// The tab's menu, opened as a right click on a tab does.
QMenu *openTabMenu(EditWindow &e, int tab)
{
    Q_EMIT e.win.w.sheetTabs()->contextRequested(tab, QPoint(10, 10));
    return menuOf(e, "sheetMenu");
}

void clickTab(EditWindow &e, int tab, Qt::KeyboardModifiers m = Qt::NoModifier)
{
    crunch::SheetTabBar *tabs = e.win.w.sheetTabs();
    QTest::mouseClick(tabs, Qt::LeftButton, m, tabs->tabRect(tab).center());
}

} // namespace

class CrunchSheetTabsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    // The tabs are the sheets that are shown, in the order of the tabs; Insert Sheet puts a sheet before the one that is open and opens it, the New Sheet button puts one at the end, and
    // Undo takes them away and opens a sheet that is there.
    void theTabsFollowTheSheetsAdded()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        QCOMPARE(tabNames(e), QStringList({QStringLiteral("Sheet1"), QStringLiteral("Sheet2"), QStringLiteral("Sheet3")}));
        QVERIFY(e.win.w.findChild<QToolButton *>(QStringLiteral("addSheet")));
        clickTab(e, 1);
        QCOMPARE(e.win.w.activeSheet(), 1);
        run(e, "sheet.insert");
        QCOMPARE(tabNames(e), QStringList({QStringLiteral("Sheet1"), QStringLiteral("Sheet4"), QStringLiteral("Sheet2"), QStringLiteral("Sheet3")}));
        QCOMPARE(e.win.w.activeSheet(), 3);                      // (the new sheet is open)
        QCOMPARE(e.win.w.sheetTabs()->currentIndex(), 1);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Insert Sheet"));
        run(e, "sheet.insertEnd");
        QCOMPARE(tabNames(e).last(), QStringLiteral("Sheet5"));
        QCOMPARE(e.win.w.activeSheet(), 4);
        // Undo: the sheet that was open is gone, and the one beside where it was is open
        e.win.w.undoStack()->undo();
        QCOMPARE(tabNames(e), QStringList({QStringLiteral("Sheet1"), QStringLiteral("Sheet4"), QStringLiteral("Sheet2"), QStringLiteral("Sheet3")}));
        QCOMPARE(e.win.w.activeSheet(), 2);
        e.win.w.undoStack()->undo();
        QCOMPARE(tabNames(e).size(), 3);
        QVERIFY(e.win.w.activeSheet() >= 0 && e.win.w.activeSheet() < 3);
        e.win.w.undoStack()->redo();
        QCOMPARE(tabNames(e).size(), 4);
        // typing into the sheet that is open goes to that sheet
        QVERIFY(e.settle());
        e.select(QStringLiteral("B2"));
        e.enter(QStringLiteral("12"));
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, e.win.w.activeSheet(), QStringLiteral("B2")), QStringLiteral("n:12"));
    }

    // Delete asks first when the sheet has cells (and says it can be undone), does nothing if the answer is no, and opens the sheet beside the one deleted; the last shown sheet is not deleted.
    void deletingASheetAsksAndOpensANeighbor()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        QStringList asked;
        bool yes = false;
        e.win.w.setConfirmAsker([&](const QString &title, const QString &text) {
            asked << title + QStringLiteral(": ") + text;
            return yes;
        });
        clickTab(e, 1);
        run(e, "sheet.delete");
        QCOMPARE(asked.size(), 1);
        QVERIFY2(asked.first().contains(QStringLiteral("Sheet2")) && asked.first().contains(QStringLiteral("#REF!")) && asked.first().contains(QStringLiteral("undo")), qPrintable(asked.first()));
        QCOMPARE(tabNames(e).size(), 3);
        QCOMPARE(e.win.w.undoStack()->count(), 0);
        yes = true;
        run(e, "sheet.delete");
        QCOMPARE(tabNames(e), QStringList({QStringLiteral("Sheet1"), QStringLiteral("Sheet3")}));
        QCOMPARE(e.win.w.activeSheet(), 2);   // (the one that was after it)
        e.win.w.undoStack()->undo();
        QCOMPARE(tabNames(e).size(), 3);
        QCOMPARE(e.win.w.sheetTabs()->tabData(1).toString(), QStringLiteral("Sheet2"));
        // an empty sheet is deleted without asking
        run(e, "sheet.insert");
        const int steps = e.win.w.undoStack()->count();
        asked.clear();
        run(e, "sheet.delete");
        QVERIFY(asked.isEmpty());
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        // down to the last shown sheet
        yes = true;
        clickTab(e, 0);
        run(e, "sheet.delete");
        run(e, "sheet.delete");
        QCOMPARE(tabNames(e).size(), 1);
        e.win.told.clear();
        run(e, "sheet.delete");
        QCOMPARE(tabNames(e).size(), 1);
        QVERIFY(!e.win.told.isEmpty() && e.win.told.last().contains(QStringLiteral("at least one")));
    }

    // Renaming by the tab: a double click opens a line over it, Enter makes the name (the formulas follow), Esc gives it up, and a name that is refused is told.
    void aTabIsRenamedByItsLine()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        crunch::SheetTabBar *tabs = e.win.w.sheetTabs();
        QTest::mouseDClick(tabs, Qt::LeftButton, Qt::NoModifier, tabs->tabRect(1).center());
        QVERIFY(tabs->isRenaming());
        QLineEdit *edit = tabs->editor();
        QVERIFY(edit);
        QCOMPARE(edit->text(), QStringLiteral("Sheet2"));
        edit->setText(QStringLiteral("Q & A"));
        QTest::keyClick(edit, Qt::Key_Return);
        QVERIFY(!tabs->isRenaming());
        QCOMPARE(e.win.w.loaded()->book->info().sheets[1].name, QStringLiteral("Q & A"));
        QCOMPARE(tabs->tabText(1), QStringLiteral("Q && A"));   // (the & is shown, not made a key)
        QVERIFY(e.settle());
        QCOMPARE(textOf(e, 0, QStringLiteral("B1")), QStringLiteral("='Q & A'!A1+1"));
        // Esc gives it up
        tabs->beginRename(1);
        edit = tabs->editor();
        edit->setText(QStringLiteral("Nope"));
        QTest::keyClick(edit, Qt::Key_Escape);
        QVERIFY(!tabs->isRenaming());
        QCOMPARE(e.win.w.loaded()->book->info().sheets[1].name, QStringLiteral("Q & A"));
        // a name that is refused: told, and nothing changes
        const int steps = e.win.w.undoStack()->count();
        QVERIFY(!e.win.w.renameSheetTo(1, QStringLiteral("Sheet3")));
        QVERIFY(!e.win.told.isEmpty() && e.win.told.last().contains(QStringLiteral("Sheet3")));
        QVERIFY(!e.win.w.renameSheetTo(1, QStringLiteral("a:b")));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        QCOMPARE(tabNames(e)[1], QStringLiteral("Q & A"));
        // by the command (the ribbon and the menu): the line opens on the tab that is open
        clickTab(e, 2);
        run(e, "sheet.rename");
        QVERIFY(tabs->isRenaming());
        QTest::keyClick(tabs->editor(), Qt::Key_Escape);
    }

    // Ctrl and Shift pick several tabs; Hide, Delete, Tab Color, and Move or Copy are for all of them, each as one step.
    void severalTabsArePickedAndTheCommandsAreForAll()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        crunch::SheetTabBar *tabs = e.win.w.sheetTabs();
        QCOMPARE(tabs->selectedTabs(), QVector<int>({0}));
        clickTab(e, 2, Qt::ControlModifier);
        QCOMPARE(tabs->selectedTabs(), QVector<int>({0, 2}));
        QCOMPARE(e.win.w.selectedSheets(), QVector<int>({0, 2}));
        QCOMPARE(e.win.w.activeSheet(), 0);                 // (Ctrl keeps the sheet that is open)
        clickTab(e, 2, Qt::ControlModifier);                // (again: not picked)
        QCOMPARE(tabs->selectedTabs(), QVector<int>({0}));
        clickTab(e, 2, Qt::ShiftModifier);
        QCOMPARE(tabs->selectedTabs(), QVector<int>({0, 1, 2}));
        clickTab(e, 1);                                     // (a click on a tab among those picked keeps the group)
        QCOMPARE(tabs->selectedTabs(), QVector<int>({0, 1, 2}));
        QCOMPARE(e.win.w.activeSheet(), 1);
        tabs->selectOnly(1);
        clickTab(e, 2);                                     // (a click on a tab that is not picked picks it alone)
        QCOMPARE(tabs->selectedTabs(), QVector<int>({2}));
        QCOMPARE(e.win.w.activeSheet(), 2);

        // two sheets colored in one step
        clickTab(e, 0);
        tabs->setSelectedTabs({0, 2});
        QMenu *menu = openTabMenu(e, 0);
        QVERIFY(menu);
        QMenu *color = submenuOf(menu, QStringLiteral("Tab Color"));
        QVERIFY(color);
        QToolButton *red = nullptr;
        for (QToolButton *b : color->findChildren<QToolButton *>())
            if (b->accessibleName() == QStringLiteral("#C00000")) red = b;
        QVERIFY(red);
        const int steps = e.win.w.undoStack()->count();
        red->click();
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QVERIFY(e.win.w.calculation()->sheet(0).tabColor.isSet() && e.win.w.calculation()->sheet(2).tabColor.isSet());
        QVERIFY(!e.win.w.calculation()->sheet(1).tabColor.isSet());
        QVERIFY(tabs->stripe(0).isValid() && tabs->stripe(2).isValid() && !tabs->stripe(1).isValid());
        delete menu;
        e.win.w.undoStack()->undo();
        QVERIFY(!e.win.w.calculation()->sheet(0).tabColor.isSet() && !e.win.w.calculation()->sheet(2).tabColor.isSet());
        QVERIFY(!tabs->stripe(0).isValid() && !tabs->stripe(2).isValid());

        // two sheets hidden in one step
        clickTab(e, 0);
        tabs->setSelectedTabs({0, 1});
        run(e, "sheet.hide");
        QCOMPARE(tabNames(e), QStringList({QStringLiteral("Sheet3")}));
        QCOMPARE(e.win.w.activeSheet(), 2);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Hide Sheets"));
        e.win.w.undoStack()->undo();
        QCOMPARE(tabNames(e).size(), 3);

        // and the last that is shown is not hidden
        tabs->setSelectedTabs({0, 1, 2});
        e.win.told.clear();
        const int before = e.win.w.undoStack()->count();
        run(e, "sheet.hide");
        QVERIFY(!e.win.told.isEmpty() && e.win.told.last().contains(QStringLiteral("at least one")));
        QCOMPARE(tabNames(e).size(), 3);
        QCOMPARE(e.win.w.undoStack()->count(), before);

        // two sheets deleted in one step; saving then forgets the step as one
        bool asked = false;
        e.win.w.setConfirmAsker([&](const QString &, const QString &text) {
            asked = text.contains(QStringLiteral("2"));
            return true;
        });
        clickTab(e, 0);
        tabs->setSelectedTabs({0, 1});
        run(e, "sheet.delete");
        QVERIFY(asked);
        QCOMPARE(tabNames(e), QStringList({QStringLiteral("Sheet3")}));
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Delete Sheets"));
        QVERIFY(e.win.w.session()->canUndo());
        e.win.w.undoStack()->undo();
        QCOMPARE(tabNames(e).size(), 3);
        e.win.w.undoStack()->redo();
        QCOMPARE(tabNames(e).size(), 1);
        QVERIFY(saveNow(e));
        QVERIFY(!e.win.w.session()->canUndo());   // (the macro is forgotten whole)
        QCOMPARE(tabNames(e).size(), 1);
        Saved f;
        QVERIFY(f.open(e.path));
        QCOMPARE(f.names(), QStringList({QStringLiteral("Sheet3")}));
    }

    // Move or Copy: the sheets picked go before the sheet chosen (or to the end), or copies of them do; the copy is opened.
    void sheetsAreMovedAndCopiedBeforeAnother()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        CrunchWindow::MoveCopyRequest seen;
        CrunchWindow::MoveCopyRequest answer;
        int asked = 0;
        e.win.w.setMoveCopyAsker([&](CrunchWindow::MoveCopyRequest *r) {
            seen = *r;
            ++asked;
            r->before = answer.before;
            r->copy = answer.copy;
            return true;
        });
        answer.before = -1;
        run(e, "sheet.moveCopy");
        QCOMPARE(asked, 1);
        QCOMPARE(seen.sheets, QVector<int>({0}));
        QCOMPARE(tabNames(e), QStringList({QStringLiteral("Sheet2"), QStringLiteral("Sheet3"), QStringLiteral("Sheet1")}));
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Move Sheet"));
        // two sheets before the third
        e.win.w.sheetTabs()->setSelectedTabs({1, 2});
        answer.before = 1;   // (before Sheet2, the place of the first sheet of the book, which is not one of the two: Sheet3 and Sheet1 go before it)
        run(e, "sheet.moveCopy");
        QCOMPARE(tabNames(e), QStringList({QStringLiteral("Sheet3"), QStringLiteral("Sheet1"), QStringLiteral("Sheet2")}));
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Move Sheets"));
        e.win.w.undoStack()->undo();
        QCOMPARE(tabNames(e), QStringList({QStringLiteral("Sheet2"), QStringLiteral("Sheet3"), QStringLiteral("Sheet1")}));
        // a copy, and the copy is open
        clickTab(e, 0);
        answer.before = 2;
        answer.copy = true;
        run(e, "sheet.moveCopy");
        QCOMPARE(tabNames(e), QStringList({QStringLiteral("Sheet2"), QStringLiteral("Sheet2 (2)"), QStringLiteral("Sheet3"), QStringLiteral("Sheet1")}));
        QCOMPARE(e.win.w.activeSheet(), 3);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, 3, QStringLiteral("A1")), QStringLiteral("n:10"));
        // asked and given up: nothing done
        e.win.w.setMoveCopyAsker([](CrunchWindow::MoveCopyRequest *) { return false; });
        const int steps = e.win.w.undoStack()->count();
        run(e, "sheet.moveCopy");
        QCOMPARE(e.win.w.undoStack()->count(), steps);
        // a tab dragged past the others to the end moves its sheet, as one step, and is the one shown and picked
        crunch::SheetTabBar *tabs = e.win.w.sheetTabs();
        const int before = e.win.w.undoStack()->count();
        const QPoint start = tabs->tabRect(0).center();
        const QPoint end = tabs->tabRect(3).center();
        QTest::mousePress(tabs, Qt::LeftButton, Qt::NoModifier, start);
        for (int i = 1; i <= 12; ++i) QTest::mouseMove(tabs, start + (end - start) * i / 12);
        QTest::mouseRelease(tabs, Qt::LeftButton, Qt::NoModifier, end);
        QCOMPARE(tabNames(e), QStringList({QStringLiteral("Sheet2 (2)"), QStringLiteral("Sheet3"), QStringLiteral("Sheet1"), QStringLiteral("Sheet2")}));
        QCOMPARE(e.win.w.undoStack()->count(), before + 1);
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Move Sheet"));
        QCOMPARE(tabs->selectedTabs(), QVector<int>({3}));
        QCOMPARE(e.win.w.activeSheet(), 1);   // (Sheet2, which was dragged)
        e.win.w.undoStack()->undo();
        QCOMPARE(tabNames(e), QStringList({QStringLiteral("Sheet2"), QStringLiteral("Sheet2 (2)"), QStringLiteral("Sheet3"), QStringLiteral("Sheet1")}));
    }

    // The menu of a tab, and showing hidden sheets again from a menu of the hidden.
    void theTabMenuAndUnhide()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        QVERIFY(e.win.w.session()->setSheetHidden(1, true).ok);
        QCOMPARE(tabNames(e), QStringList({QStringLiteral("Sheet1"), QStringLiteral("Sheet3")}));
        Q_EMIT e.win.w.sheetTabs()->contextRequested(1, QPoint(10, 10));
        QMenu *menu = e.win.w.findChild<QMenu *>(QStringLiteral("sheetMenu"));
        QVERIFY(menu);
        QStringList items;
        for (QAction *a : menu->actions()) items << a->text();
        for (const QString &want : {QStringLiteral("Insert Sheet"), QStringLiteral("Delete Sheet"), QStringLiteral("Rename Sheet"), QStringLiteral("Move or Copy Sheet…"), QStringLiteral("Hide Sheet"),
                                    QStringLiteral("Tab Color"), QStringLiteral("Unhide Sheet")})
            QVERIFY2(items.contains(want), qPrintable(want));
        QMenu *unhide = nullptr;
        for (QAction *a : menu->actions())
            if (a->menu() && a->text() == QStringLiteral("Unhide Sheet")) unhide = a->menu();
        QVERIFY(unhide);
        unhide->aboutToShow();
        QCOMPARE(unhide->actions().size(), 1);
        QCOMPARE(unhide->actions().first()->text(), QStringLiteral("Sheet2"));
        unhide->actions().first()->trigger();
        QCOMPARE(tabNames(e), QStringList({QStringLiteral("Sheet1"), QStringLiteral("Sheet2"), QStringLiteral("Sheet3")}));
        unhide->aboutToShow();
        QVERIFY(!unhide->actions().first()->isEnabled());   // (none are hidden: "No hidden sheets")
        delete menu;
        // the ribbon names the same menus, and every one of its parts is found
        QCOMPARE(e.win.w.ribbonErrors(), QString());
    }
};

JP_TEST_CLASS(CrunchSheetsTests)
JP_TEST_CLASS(CrunchSheetTabsTests)
#include "tst_crunch_sheets.moc"
