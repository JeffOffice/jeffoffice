// Tests of the sheets of a workbook as a person changes them through the edit session: a sheet inserted, copied, renamed, deleted, moved, hidden, shown, and given a color; each one step of
// Undo, the formulas that name a sheet following it, the names a sheet may have, and what is refused. The workbook is built here.
#include "app/crunchwindow.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "io/zip.h"

#include <QLineEdit>
#include <QMenu>
#include <QSignalSpy>
#include <QToolButton>
#include <QWidgetAction>
#include "app/sheettabs.h"
#include <QUndoStack>

#include "../testrunner.h"
#include "../localepin.h"

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
        const std::optional<Cell> c = at(s, ref);
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


// The sheet picked, copied to the other workbook through the window's own command.
void copyToOther(EditWindow &from, EditWindow &to, int sheet)
{
    from.win.w.setMoveCopyAsker([&](CrunchWindow::MoveCopyRequest *r) {
        r->copy = true;
        r->before = -1;
        r->toBook = &to.win.w;
        return true;
    });
    from.win.w.sheetTabs()->selectOnly(sheet);
    from.win.w.setActiveSheet(sheet);
    from.win.w.act(QStringLiteral("sheet.moveCopy"))->trigger();
}

// The sheets picked (their places), copied to the other workbook through the window's own command.
void copyManyToOther(EditWindow &from, EditWindow *to, const QSet<int> &sheets)
{
    from.win.w.setMoveCopyAsker([&](CrunchWindow::MoveCopyRequest *r) {
        r->copy = true;
        r->before = -1;
        r->toBook = &to->win.w;
        return true;
    });
    from.win.w.sheetTabs()->setSelectedTabs(sheets);
    from.win.w.setActiveSheet(*sheets.begin());
    from.win.w.act(QStringLiteral("sheet.moveCopy"))->trigger();
}

// A workbook of two sheets and a look:
//   Data:  A1 = 5 (bold, green fill), B1 = A1*2, C1 = Other!A1+1, D1 = Data!A1+1, A2 = "text", A3:B3 merged; column B is wide; 
//   Other: A1 = 100; with `withName` a third sheet has A1 = Total, a name of the workbook for Other!A1
QByteArray dataBook(bool withName = false)
{
    const QString fonts = QStringLiteral("<fonts count=\"2\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font><font><b/><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>"
                                         "<fills count=\"3\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"
                                         "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FF00FF00\"/></patternFill></fill></fills>"
                                         "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                                         "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>");
    const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/><xf numFmtId=\"0\" fontId=\"1\" fillId=\"2\" borderId=\"0\" xfId=\"0\" applyFont=\"1\" applyFill=\"1\"/>"), fonts);
    const QString a1 = QStringLiteral("<c r=\"A1\" s=\"1\"><v>5</v></c>");
    const QByteArray data = worksheet(row(1, a1 + formula(QStringLiteral("B1"), QStringLiteral("A1*2")) + formula(QStringLiteral("C1"), QStringLiteral("Other!A1+1")) + formula(QStringLiteral("D1"), QStringLiteral("Data!A1+1")))
                                         + row(2, QStringLiteral("<c r=\"A2\" t=\"inlineStr\"><is><t>text</t></is></c>")) + row(3, cell(QStringLiteral("A3"), QStringLiteral("1"))),
                                      QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A3:B3\"/></mergeCells>"), QStringLiteral("<cols><col min=\"2\" max=\"2\" width=\"30\" customWidth=\"1\"/></cols>"));
    const QByteArray other = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("100"))));
    const QByteArray third = worksheet(row(1, formula(QStringLiteral("A1"), QStringLiteral("Total"))));
    return withName ? buildXlsx({data, other, third}, QByteArray(), QString(), QString(), styles, QStringList({QStringLiteral("Data"), QStringLiteral("Other"), QStringLiteral("Third")}),
                                QStringLiteral("<definedName name=\"Total\">Other!$A$1</definedName>"))
                    : buildXlsx({data, other}, QByteArray(), QString(), QString(), styles, QStringList({QStringLiteral("Data"), QStringLiteral("Other")}));
}


// One sheet that is shown in its own way: no gridlines or headings or zeros, reading right to left, zoomed, with the first two rows and the first column held, and its own default sizes. The
// second sheet has nothing.
QByteArray viewBook()
{
    const QString head = QStringLiteral("<sheetViews><sheetView showGridLines=\"0\" showRowColHeaders=\"0\" showZeros=\"0\" rightToLeft=\"1\" zoomScale=\"150\" zoomScaleNormal=\"150\" workbookViewId=\"0\">"
                                        "<pane xSplit=\"1\" ySplit=\"2\" topLeftCell=\"B3\" activePane=\"bottomRight\" state=\"frozen\"/><selection pane=\"bottomRight\" activeCell=\"C4\" sqref=\"C4\"/></sheetView></sheetViews>"
                                        "<sheetFormatPr baseColWidth=\"8\" defaultColWidth=\"12\" defaultRowHeight=\"21\" customHeight=\"1\"/>");
    const QByteArray one = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("5"))) + row(4, cell(QStringLiteral("B4"), QStringLiteral("0"))), QString(), head);
    return buildXlsx({one, worksheet(QString())}, QByteArray(), QString(), QString(), QByteArray(), QStringList({QStringLiteral("Shown"), QStringLiteral("Empty")}));
}

// Three looks: a diagonal line going up (A1), one going down (A2), and a green fill (A3).
QByteArray lookBook()
{
    const QString fonts = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>"
                                         "<fills count=\"3\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill>"
                                         "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FF00FF00\"/></patternFill></fill></fills>"
                                         "<borders count=\"3\"><border><left/><right/><top/><bottom/><diagonal/></border>"
                                         "<border diagonalUp=\"1\"><left/><right/><top/><bottom/><diagonal style=\"thin\"><color rgb=\"FF000000\"/></diagonal></border>"
                                         "<border diagonalDown=\"1\"><left/><right/><top/><bottom/><diagonal style=\"thin\"><color rgb=\"FF000000\"/></diagonal></border></borders>"
                                         "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>");
    const QByteArray styles = stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"
                                                                  "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"1\" xfId=\"0\" applyBorder=\"1\"/>"
                                                                  "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"2\" xfId=\"0\" applyBorder=\"1\"/>"
                                                                  "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"2\" borderId=\"0\" xfId=\"0\" applyFill=\"1\"/>"), fonts);
    auto styled = [](const QString &ref, int xf) { return QStringLiteral("<c r=\"%1\" s=\"%2\"><v>1</v></c>").arg(ref).arg(xf); };
    return buildXlsx({worksheet(row(1, styled(QStringLiteral("A1"), 1)) + row(2, styled(QStringLiteral("A2"), 2)) + row(3, styled(QStringLiteral("A3"), 3)))}, QByteArray(), QString(), QString(), styles,
                     QStringList({QStringLiteral("Looks")}));
}

// A sheet that reads a third workbook (a link its file has no part for: the stored result is what it shows), one with "[" inside a text, and one that reads another sheet.
QByteArray thirdBook()
{
    const QByteArray one = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("5")) + QStringLiteral("<c r=\"B1\"><f>[1]Elsewhere!A1+1</f><v>7</v></c>")
                                                + QStringLiteral("<c r=\"C1\" t=\"str\"><f>\"a[b]\"&amp;A1</f><v>a[b]5</v></c>") + formula(QStringLiteral("D1"), QStringLiteral("Other!A1+1"))));
    const QByteArray other = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("100"))));
    return buildXlsx({one, other}, QByteArray(), QString(), QString(), QByteArray(), QStringList({QStringLiteral("Mixed"), QStringLiteral("Other")}));
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
    // A file whose sheets are all hidden opens with its first sheet shown: a window always has a tab to work in.
    void aFileWithEverySheetHiddenOpensWithTheFirstShown()
    {
        EditWindow e;
        const QByteArray a = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"));
        const QByteArray b = worksheet(QStringLiteral("<row r=\"2\"><c r=\"B2\"><v>2</v></c></row>"));
        QVERIFY(e.open(buildXlsx({a, b}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), QString(), QByteArray(), QString(), QList<int>{0, 1})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QCOMPARE(w.shownSheets(), QVector<int>{0});
        QCOMPARE(w.activeSheet(), 0);
        QCOMPARE(w.loaded()->book->info().sheets[1].state, SheetState::Hidden);
    }

    // A hidden sheet is not gone to by a reference: the person is told, and the sheet in view stays one with a tab. A sheet that was deleted is not either.
    void hiddenAndDeletedSheetsAreNotShown()
    {
        EditWindow e;
        const QByteArray a = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"));
        const QByteArray b = worksheet(QStringLiteral("<row r=\"2\"><c r=\"B2\"><v>2</v></c></row>"));
        const QByteArray c = worksheet(QStringLiteral("<row r=\"3\"><c r=\"C3\"><v>3</v></c></row>"));
        QVERIFY(e.open(buildXlsx({a, b, c}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), QString(), QByteArray(), QString(), QList<int>{1})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(!w.goToReference(QStringLiteral("Sheet2!B2")));
        QCOMPARE(w.activeSheet(), 0);
        QVERIFY(!e.win.told.isEmpty());
        QVERIFY(w.shownSheets().contains(w.activeSheet()));
        w.setActiveSheet(1);
        QCOMPARE(w.activeSheet(), 0);
        QVERIFY(w.session()->deleteSheet(2).ok);
        QVERIFY(e.settle());
        w.setActiveSheet(2);
        QCOMPARE(w.activeSheet(), 0);
        QVERIFY(w.goToReference(QStringLiteral("Sheet1!A1")));
    }

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

    // A sheet copied to ANOTHER open workbook (the first is only read): its cells, looks, merges, and sizes; a formula that reads another sheet of the first workbook is a link to its file, one that
    // reads the copied sheet stays on it (also when it names the sheet), and the whole is one step of Undo in the second workbook only. Saved and opened again by the app's own paths.
    void aSheetIsCopiedToAnotherOpenWorkbook()
    {
        EditWindow src, dst;
        QVERIFY(src.open(dataBook()));
        QVERIFY(dst.open(dataBook()));
        QVERIFY(src.settle());
        QVERIFY(dst.settle());
        crunch::EditSession *to = dst.win.w.session();
        const int sheetsBefore = dst.win.w.calculation()->sheetCount();
        copyToOther(src, dst, 0);
        QVERIFY2(src.win.told.size() == 1 && src.win.told.last().contains(QStringLiteral("will not be copied")) && src.win.told.last().contains(QStringLiteral("become links")), qPrintable(src.win.told.join(QLatin1Char('|'))));
        QVERIFY(dst.settle());
        QCOMPARE(dst.win.w.calculation()->sheetCount(), sheetsBefore + 1);
        const int made = sheetsBefore;
        QCOMPARE(dst.win.w.loaded()->book->info().sheets[made].name, QStringLiteral("Data (2)"));
        QCOMPARE(valueOf(dst, made, QStringLiteral("A1")), QStringLiteral("n:5"));
        QCOMPARE(valueOf(dst, made, QStringLiteral("B1")), QStringLiteral("n:10"));     // (reads the copy's own A1)
        QCOMPARE(valueOf(dst, made, QStringLiteral("D1")), QStringLiteral("n:6"));      // (names its own sheet, whose name changed here: stays on the copy)
        QCOMPARE(textOf(dst, made, QStringLiteral("D1")), QStringLiteral("='Data (2)'!A1+1"));
        QCOMPARE(textOf(dst, made, QStringLiteral("C1")), QStringLiteral("=[1]Other!A1+1"));   // (reads another sheet of the first workbook: a link to its file)
        QTRY_COMPARE(valueOf(dst, made, QStringLiteral("C1")), QStringLiteral("n:101"));
        QCOMPARE(textOf(dst, made, QStringLiteral("A2")), QStringLiteral("text"));
        // the look came: the font and the fill the cell had, as formats of the second workbook
        {
            const Styles &was = src.win.w.loaded()->book->styles();
            const Calculation::CellContent s1 = src.win.w.calculation()->content(0, 0, 0);
            QVERIFY2(was.font(was.format(s1.style).font).bold, "the fixture has the look");
        }
        const Styles &styles = dst.win.w.loaded()->book->styles();
        const Calculation::CellContent a1 = dst.win.w.calculation()->content(made, 0, 0);
        QVERIFY(styles.font(styles.format(a1.style).font).bold);
        QCOMPARE(styles.resolve(styles.fill(styles.format(a1.style).fill).fg, 0), quint32(0x00FF00));
        QCOMPARE(dst.win.w.calculation()->mergesOf(made).size(), 1);
        QCOMPARE(dst.win.w.calculation()->mergesOf(made).first().col2, quint32(1));
        QVERIFY(dst.win.w.calculation()->columnRuns(made, 1, 1).front().customWidth);
        // the first workbook is as it was; the second has one step to undo
        QCOMPARE(src.win.w.calculation()->sheetCount(), 2);
        QCOMPARE(src.win.w.undoStack()->count(), 0);
        QCOMPARE(dst.win.w.undoStack()->undoText(), QStringLiteral("Copy Sheet to Another Workbook"));
        // Undo takes the sheet away
        dst.win.w.undoStack()->undo();
        QVERIFY(dst.settle());
        QCOMPARE(dst.win.w.calculation()->sheetCount(), sheetsBefore);
        QCOMPARE(to->sheetOrder().size(), sheetsBefore);
        dst.win.w.undoStack()->redo();
        QVERIFY(dst.settle());
        QCOMPARE(valueOf(dst, made, QStringLiteral("B1")), QStringLiteral("n:10"));
        // saved and opened again: the link has its part, naming the first file, and the sheet is in the file
        QVERIFY2(saveNow(dst), qPrintable(dst.win.told.join(QLatin1Char('|'))));
        Saved f;
        QVERIFY(f.open(dst.path));
        QCOMPARE(f.names().last(), QStringLiteral("Data (2)"));
        QCOMPARE(f.formula(made, QStringLiteral("C1")), QStringLiteral("[1]Other!A1+1"));
        QCOMPARE(f.formula(made, QStringLiteral("D1")), QStringLiteral("'Data (2)'!A1+1"));
        QFile file(dst.path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QMap<QString, QByteArray> parts;
        QVERIFY(jp::readZip(file.readAll(), parts));
        QVERIFY(parts.contains(QStringLiteral("xl/externalLinks/externalLink1.xml")));
        QVERIFY2(QString::fromUtf8(parts.value(QStringLiteral("xl/externalLinks/_rels/externalLink1.xml.rels"))).contains(QFileInfo(src.path).fileName()), "the link names the first file");
        // (the part names the sheet the formulas read: a part with no sheet names is not one a spreadsheet program opens)
        QVERIFY2(QString::fromUtf8(parts.value(QStringLiteral("xl/externalLinks/externalLink1.xml"))).contains(QStringLiteral("<sheetName val=\"Other\"/>")), "the link part names the sheet");
        QVERIFY2(QString::fromUtf8(parts.value(QStringLiteral("xl/externalLinks/externalLink1.xml"))).contains(QStringLiteral("<v>100</v>")), "the link holds the value it read");
    }

    // What can't be carried is refused in words and nothing changes (a workbook that was never saved, dates counted from another year, a sheet past the cap); moving to another workbook is not done
    // yet.
    void copyingToAnotherWorkbookRefusesWhatItCannotCarry()
    {
        EditWindow src, dst;
        QVERIFY(src.open(dataBook()));
        QVERIFY(dst.open(sheetsBook()));
        QVERIFY(src.settle());
        QVERIFY(dst.settle());
        const int steps = dst.win.w.undoStack()->count();
        // changes not saved are no reason to refuse (the links name the file; see aSourceWithUnsavedChangesIsCopiedWithLinksToItsSavedFile)
        QVERIFY(src.win.w.session()->type(0, 5, 0, QStringLiteral("9")).ok);
        QVERIFY(src.settle());
        src.win.told.clear();
        copyToOther(src, dst, 0);
        QVERIFY2(!src.win.told.isEmpty() && src.win.told.last().contains(QStringLiteral("to keep these values")), qPrintable(src.win.told.join(QLatin1Char('|'))));
        QCOMPARE(dst.win.w.calculation()->sheetCount(), 4);
        dst.win.w.undoStack()->undo();
        QVERIFY(dst.settle());
        QCOMPARE(dst.win.w.undoStack()->index(), steps);
        QCOMPARE(dst.win.w.calculation()->sheetCount(), 3);
        // (a sheet that reads no other sheet is copied from it all the same)
        {
            const crunch::EditSession::SheetExport own = src.win.w.session()->exportSheet(1, src.path);
            QVERIFY(own.ok);
            QVERIFY(!own.readsOtherSheets);
            QVERIFY(dst.win.w.session()->checkImport(own).ok);
        }
        // a workbook that was never saved has no file to link to
        EditWindow blank;
        QVERIFY(blank.openBlank());
        QVERIFY(blank.settle());
        QVERIFY(blank.win.w.session()->insertSheet(-1, QString()).ok);
        QVERIFY(blank.win.w.session()->type(0, 0, 0, QStringLiteral("=Sheet2!A1")).ok);
        const crunch::EditSession::SheetExport unsaved = blank.win.w.session()->exportSheet(0, QString());
        QVERIFY(unsaved.ok && unsaved.readsOtherSheets);
        const crunch::EditSession::Outcome refused = dst.win.w.session()->checkImport(unsaved);
        QVERIFY(!refused.ok && refused.problem.contains(QStringLiteral("Save")));
        QVERIFY(!dst.win.w.session()->importSheet(unsaved, -1, QString()).ok);
        QCOMPARE(dst.win.w.calculation()->sheetCount(), 3);
        // dates counted from another year
        EditWindow old;
        QVERIFY(old.open(buildXlsx({worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("1"))))}, QByteArray(), QStringLiteral("date1904=\"1\""))));
        QVERIFY(old.settle());
        const crunch::EditSession::Outcome years = dst.win.w.session()->checkImport(old.win.w.session()->exportSheet(0, old.path));
        QVERIFY2(!years.ok && years.problem.contains(QStringLiteral("1904")), qPrintable(years.problem));
        // more cells than the cap
        struct Cap {
            Cap() { crunch::EditSession::setTransferCapForTest(3); }
            ~Cap() { crunch::EditSession::setTransferCapForTest(0); }
        } cap;
        const crunch::EditSession::SheetExport big = src.win.w.session()->exportSheet(0, src.path);
        QVERIFY2(!big.ok && big.problem.contains(QStringLiteral("more cells")), qPrintable(big.problem));
        // moving is not done yet, and the box is not asked about it
        crunch::EditSession::setTransferCapForTest(0);
        src.win.w.undoStack()->undo();
        QVERIFY(src.settle());
        src.win.told.clear();
        src.win.w.setMoveCopyAsker([&](CrunchWindow::MoveCopyRequest *r) {
            r->copy = false;
            r->toBook = &dst.win.w;
            return true;
        });
        src.win.w.sheetTabs()->selectOnly(0);
        src.win.w.setActiveSheet(0);
        src.win.w.act(QStringLiteral("sheet.moveCopy"))->trigger();
        QVERIFY2(!src.win.told.isEmpty() && src.win.told.last().contains(QStringLiteral("not done yet")), qPrintable(src.win.told.join(QLatin1Char('|'))));
        QCOMPARE(dst.win.w.calculation()->sheetCount(), 3);
        QCOMPARE(src.win.w.calculation()->sheetCount(), 2);
    }

    // A workbook open for reading only can be copied from but not to.
    void aWorkbookOpenForReadingOnlyIsCopiedFromNotTo()
    {
        EditWindow src, dst;
        QVERIFY(src.open(dataBook()));
        QVERIFY(dst.open(sheetsBook()));
        QVERIFY(src.settle());
        QVERIFY(dst.settle());
        src.win.w.setReadOnly(true);
        copyToOther(src, dst, 0);
        QVERIFY(dst.settle());
        QCOMPARE(dst.win.w.calculation()->sheetCount(), 4);
        dst.win.w.undoStack()->undo();
        dst.win.w.setReadOnly(true);
        src.win.w.setReadOnly(false);
        src.win.told.clear();
        copyToOther(src, dst, 0);
        QCOMPARE(dst.win.w.calculation()->sheetCount(), 3);
        QVERIFY2(!src.win.told.isEmpty(), "the person is told");
    }

    // The names a copy's formulas could not carry are said, and the cells keep their values: a formula that uses a name of the workbook comes as the value it shows.
    void aFormulaThatUsesANameComesAsItsValue()
    {
        EditWindow src, dst;
        QVERIFY(src.open(dataBook(true)));
        QVERIFY(dst.open(sheetsBook()));
        QVERIFY(src.settle());
        QVERIFY(dst.settle());
        const crunch::EditSession::SheetExport data = src.win.w.session()->exportSheet(2, src.path);
        QVERIFY(data.ok);
        QCOMPARE(data.frozen, qint64(1));
        const crunch::EditSession::Outcome o = dst.win.w.session()->checkImport(data);
        QVERIFY2(o.ok && o.problem.contains(QStringLiteral("use names of the other workbook")) && o.problem.contains(QStringLiteral("1 cell comes as the value it shows")), qPrintable(o.problem));
        QVERIFY(dst.win.w.session()->importSheet(data, -1, QString()).ok);
        QVERIFY(dst.settle());
        QCOMPARE(valueOf(dst, 3, QStringLiteral("A1")), QStringLiteral("n:100"));
        QCOMPARE(textOf(dst, 3, QStringLiteral("A1")), QStringLiteral("100"));
    }

    // Every cell a spill fills comes as the value it shows (not only the cell with the formula), and the box counts them all, naming only what was there (an array or a spill).
    void aSpillIsCopiedAsItsValues()
    {
        EditWindow src, dst;
        QVERIFY(src.open(dataBook()));
        QVERIFY(dst.open(dataBook()));
        QVERIFY(src.settle());
        QVERIFY(dst.settle());
        QVERIFY(src.win.w.session()->type(0, 5, 5, QStringLiteral("=SEQUENCE(3)")).ok);
        QVERIFY(src.settle());
        QCOMPARE(valueOf(src, 0, QStringLiteral("F8")), QStringLiteral("n:3"));
        const int made = dst.win.w.calculation()->sheetCount();
        copyToOther(src, dst, 0);
        QVERIFY(dst.settle());
        QVERIFY2(src.win.told.size() == 1 && src.win.told.last().contains(QStringLiteral("3 ")) && src.win.told.last().contains(QStringLiteral("are part of an array or a spill")), qPrintable(src.win.told.join(QLatin1Char('|'))));
        QVERIFY2(!src.win.told.last().contains(QStringLiteral("names")) && !src.win.told.last().contains(QStringLiteral("tables")), qPrintable(src.win.told.last()));
        for (const char *ref : {"F6", "F7", "F8"}) QCOMPARE(textOf(dst, made, QString::fromLatin1(ref)), QString::number(QString::fromLatin1(ref).mid(1).toInt() - 5));
        QCOMPARE(valueOf(dst, made, QStringLiteral("F8")), QStringLiteral("n:3"));
        QVERIFY2(saveNow(dst), qPrintable(dst.win.told.join(QLatin1Char('|'))));
        Saved f;
        QVERIFY(f.open(dst.path));
        QCOMPARE(f.shown(made, QStringLiteral("F7")), QStringLiteral("n:2"));
        QCOMPARE(f.shown(made, QStringLiteral("F8")), QStringLiteral("n:3"));
    }

    // How a sheet is shown comes with it: held rows and columns, zoom, gridlines, headings, zeros, the way it reads, and its default sizes (rows with no size of their own stay as tall).
    void aCopiedSheetIsShownAsTheOriginalWas()
    {
        EditWindow src, dst;
        QVERIFY(src.open(viewBook()));
        QVERIFY(dst.open(dataBook()));
        QVERIFY(src.settle());
        QVERIFY(dst.settle());
        const int made = dst.win.w.calculation()->sheetCount();
        copyToOther(src, dst, 0);
        QVERIFY(dst.settle());
        auto checkView = [&](const SheetView &v, const SheetFormat &f, const QString &where) {
            QVERIFY2(!v.showGridLines && !v.showHeadings && !v.showZeros && v.rightToLeft, qPrintable(where));
            QVERIFY2(v.zoom == 150 && v.frozen && v.frozenRows == 2 && v.frozenCols == 1, qPrintable(where));
            QVERIFY2(f.defaultRowHeight == 21 && f.defaultColWidth == 12 && f.customHeight, qPrintable(where));
        };
        checkView(dst.win.w.calculation()->sheet(made).view, dst.win.w.calculation()->sheet(made).format, QStringLiteral("in the second workbook"));
        // the window shows it so
        dst.win.w.setActiveSheet(made);
        QCOMPARE(dst.view()->viewState().frozenRows, qint64(2));
        QCOMPARE(dst.view()->viewState().frozenCols, qint64(1));
        QVERIFY(!dst.view()->showGridlines());
        QVERIFY(!dst.view()->showHeadings());
        // Undo takes it away, Redo gives it back
        dst.win.w.undoStack()->undo();
        QVERIFY(dst.settle());
        dst.win.w.undoStack()->redo();
        QVERIFY(dst.settle());
        checkView(dst.win.w.calculation()->sheet(made).view, dst.win.w.calculation()->sheet(made).format, QStringLiteral("after Redo"));
        // saved by the second workbook's own save and read again
        QVERIFY2(saveNow(dst), qPrintable(dst.win.told.join(QLatin1Char('|'))));
        Saved f;
        QVERIFY(f.open(dst.path));
        Sheet back;
        QVERIFY(f.book.readSheet(made, &back));
        checkView(back.view, back.format, QStringLiteral("in the saved file"));
    }

    // A shape made in this session, not yet in any file, is named in the box like one the file had.
    void aShapeMadeInThisSessionIsNamedBeforeTheCopy()
    {
        EditWindow src, dst;
        QVERIFY(src.open(dataBook()));
        QVERIFY(dst.open(dataBook()));
        QVERIFY(src.settle());
        QVERIFY(dst.settle());
        copyToOther(src, dst, 0);
        QVERIFY2(src.win.told.size() == 1 && !src.win.told.last().contains(QStringLiteral("shapes")), qPrintable(src.win.told.join(QLatin1Char('|'))));
        dst.win.w.undoStack()->undo();
        src.win.told.clear();
        QVERIFY(src.win.w.insertShape(QStringLiteral("rect")));
        QVERIFY(src.settle());
        copyToOther(src, dst, 0);
        QVERIFY2(src.win.told.size() == 1 && src.win.told.last().contains(QStringLiteral("shapes, text boxes, and lines")), qPrintable(src.win.told.join(QLatin1Char('|'))));
    }

    // The direction of a diagonal border goes with its look, into the second workbook and through its save.
    void aDiagonalBorderKeepsItsDirection()
    {
        EditWindow src, dst;
        QVERIFY(src.open(lookBook()));
        QVERIFY(dst.open(dataBook()));
        QVERIFY(src.settle());
        QVERIFY(dst.settle());
        const int made = dst.win.w.calculation()->sheetCount();
        copyToOther(src, dst, 0);
        QVERIFY(dst.settle());
        auto direction = [](const Styles &styles, quint32 xf) { return QStringLiteral("%1%2").arg(styles.border(styles.format(xf).border).diagonalUp ? 'u' : '-').arg(styles.border(styles.format(xf).border).diagonalDown ? 'd' : '-'); };
        const Styles &now = dst.win.w.loaded()->book->styles();
        QCOMPARE(direction(now, dst.win.w.calculation()->content(made, 0, 0).style), QStringLiteral("u-"));
        QCOMPARE(direction(now, dst.win.w.calculation()->content(made, 1, 0).style), QStringLiteral("-d"));
        QVERIFY2(saveNow(dst), qPrintable(dst.win.told.join(QLatin1Char('|'))));
        Saved f;
        QVERIFY(f.open(dst.path));
        Sheet back;
        QVERIFY(f.book.readSheet(made, &back));
        QCOMPARE(direction(f.book.styles(), at(back, QStringLiteral("A1"))->style), QStringLiteral("u-"));
        QCOMPARE(direction(f.book.styles(), at(back, QStringLiteral("A2"))->style), QStringLiteral("-d"));
    }

    // The formats are asked for room before any is added: a workbook that has room for fewer than the sheet's looks refuses in words, with no format added, no sheet, and no step; one that has room
    // takes them, and what Undo leaves is those formats only (Redo adds no more).
    void aWorkbookCloseToItsFormatLimitIsAskedFirst()
    {
        auto fillTo = [](Workbook &book, qint64 formats) {
            for (int n = 0; qint64(book.styles().formats.size()) < formats && n < 70000; ++n) {
                FormatChange c;
                c.indent = n % 256;
                c.rotation = n / 256;
                book.formatWith(0, c);
            }
            return qint64(book.styles().formats.size()) == formats;
        };
        EditWindow src, tight, roomy;
        QVERIFY(src.open(lookBook()));
        QVERIFY(tight.open(dataBook()));
        QVERIFY(roomy.open(dataBook()));
        QVERIFY(src.settle());
        QVERIFY(tight.settle());
        QVERIFY(roomy.settle());
        // room for two, and the sheet has three looks
        Workbook &closed = *tight.win.w.loaded()->book;
        QVERIFY(fillTo(closed, kMaxCellFormatsWritten - 2));
        const int sheets = tight.win.w.calculation()->sheetCount();
        const int steps = tight.win.w.undoStack()->count();
        copyToOther(src, tight, 0);
        QVERIFY2(!src.win.told.isEmpty() && src.win.told.last().contains(QStringLiteral("Nothing was copied")), qPrintable(src.win.told.join(QLatin1Char('|'))));
        QCOMPARE(tight.win.w.calculation()->sheetCount(), sheets);
        QCOMPARE(tight.win.w.undoStack()->count(), steps);
        QCOMPARE(qint64(closed.styles().formats.size()), kMaxCellFormatsWritten - 2);
        // room for three
        src.win.told.clear();
        Workbook &room = *roomy.win.w.loaded()->book;
        QVERIFY(fillTo(room, kMaxCellFormatsWritten - 3));
        const int count = roomy.win.w.calculation()->sheetCount();
        copyToOther(src, roomy, 0);
        QVERIFY2(src.win.told.size() == 1 && !src.win.told.last().contains(QStringLiteral("Nothing was copied")), qPrintable(src.win.told.join(QLatin1Char('|'))));
        QVERIFY(roomy.settle());
        QCOMPARE(roomy.win.w.calculation()->sheetCount(), count + 1);
        const qint64 after = room.styles().formats.size();
        QVERIFY(after <= kMaxCellFormatsWritten && after > kMaxCellFormatsWritten - 3);
        roomy.win.w.undoStack()->undo();
        QVERIFY(roomy.settle());
        QCOMPARE(roomy.win.w.calculation()->sheetCount(), count);
        QCOMPARE(qint64(room.styles().formats.size()), after);   // (what Undo leaves: those formats, no more)
        roomy.win.w.undoStack()->redo();
        QVERIFY(roomy.settle());
        QCOMPARE(qint64(room.styles().formats.size()), after);
        QCOMPARE(roomy.win.w.calculation()->sheetCount(), count + 1);
    }

    // A sheet is counted by the cells that hold something, not by the rows and columns they reach: one cell far from the rest copies though the rows times the columns come to billions.
    void aSparseSheetIsCountedByItsCells()
    {
        struct Cap {
            Cap() { crunch::EditSession::setTransferCapForTest(5); }
            ~Cap() { crunch::EditSession::setTransferCapForTest(0); }
        } cap;
        const QByteArray far = worksheet(row(1, cell(QStringLiteral("A1"), QStringLiteral("1"))) + row(1048576, cell(QStringLiteral("XFD1048576"), QStringLiteral("42"))));
        EditWindow src, dst;
        QVERIFY(src.open(buildXlsx({far, worksheet(QString())})));
        QVERIFY(dst.open(dataBook()));
        QVERIFY(src.settle());
        QVERIFY(dst.settle());
        const int made = dst.win.w.calculation()->sheetCount();
        copyToOther(src, dst, 0);
        QVERIFY2(src.win.told.size() <= 1 && (src.win.told.isEmpty() || !src.win.told.last().contains(QStringLiteral("larger"))), qPrintable(src.win.told.join(QLatin1Char('|'))));
        QVERIFY(dst.settle());
        QCOMPARE(dst.win.w.calculation()->sheetCount(), made + 1);
        QCOMPARE(valueOf(dst, made, QStringLiteral("XFD1048576")), QStringLiteral("n:42"));
        // and the count of cells still holds: six cells are over the cap of five
        src.win.told.clear();
        QVERIFY(src.win.w.session()->type(0, 1, 0, QStringLiteral("2")).ok);
        QVERIFY(src.win.w.session()->type(0, 2, 0, QStringLiteral("3")).ok);
        QVERIFY(src.win.w.session()->type(0, 3, 0, QStringLiteral("4")).ok);
        QVERIFY(src.win.w.session()->type(0, 4, 0, QStringLiteral("5")).ok);
        const crunch::EditSession::SheetExport six = src.win.w.session()->exportSheet(0, src.path);
        QVERIFY2(!six.ok && six.problem.contains(QStringLiteral("more cells")), qPrintable(six.problem));
    }

    // A formula that reads a third workbook comes as the value it shows, and the box says exactly that (not "names or tables"); a "[" in a text in quotes is no link and the formula stays a formula.
    void aFormulaThatReadsAThirdWorkbookComesAsItsValue()
    {
        EditWindow src, dst;
        QVERIFY(src.open(thirdBook()));
        QVERIFY(dst.open(dataBook()));
        QVERIFY(src.settle());
        QVERIFY(dst.settle());
        const int made = dst.win.w.calculation()->sheetCount();
        copyToOther(src, dst, 0);
        QVERIFY(dst.settle());
        QVERIFY2(src.win.told.size() == 1 && src.win.told.last().contains(QStringLiteral("1 cell comes as the value it shows")) && src.win.told.last().contains(QStringLiteral("read cells of a third workbook")), qPrintable(src.win.told.join(QLatin1Char('|'))));
        QVERIFY2(!src.win.told.last().contains(QStringLiteral("names")) && !src.win.told.last().contains(QStringLiteral("tables")), qPrintable(src.win.told.last()));
        QCOMPARE(textOf(dst, made, QStringLiteral("B1")), QStringLiteral("7"));
        QCOMPARE(valueOf(dst, made, QStringLiteral("B1")), QStringLiteral("n:7"));
        QCOMPARE(textOf(dst, made, QStringLiteral("C1")), QStringLiteral("=\"a[b]\"&A1"));   // (still a formula)
        QCOMPARE(textOf(dst, made, QStringLiteral("D1")), QStringLiteral("=[1]Other!A1+1"));   // (and the one that reads another sheet is still the link)
    }

    // A workbook with a file and changes that are not saved is not refused: the links name its saved file, the box says so in a sentence, and the copy shows what the open workbook shows now. One
    // that was never saved is refused (it has no file to name).
    void aSourceWithUnsavedChangesIsCopiedWithLinksToItsSavedFile()
    {
        EditWindow src, dst;
        QVERIFY(src.open(dataBook()));
        QVERIFY(dst.open(dataBook()));
        QVERIFY(src.settle());
        QVERIFY(dst.settle());
        QVERIFY(src.win.w.session()->type(1, 0, 0, QStringLiteral("9")).ok);   // (Other!A1: not saved)
        QVERIFY(src.settle());
        QVERIFY(src.win.w.session()->isModified());
        const int made = dst.win.w.calculation()->sheetCount();
        copyToOther(src, dst, 0);
        QVERIFY(dst.settle());
        QVERIFY2(src.win.told.size() == 1 && src.win.told.last().contains(QStringLiteral("Update Values in Edit Links")) && src.win.told.last().contains(QStringLiteral("so save book.xlsx to keep these values")), qPrintable(src.win.told.join(QLatin1Char('|'))));
        QCOMPARE(dst.win.w.calculation()->sheetCount(), made + 1);
        QCOMPARE(textOf(dst, made, QStringLiteral("C1")), QStringLiteral("=[1]Other!A1+1"));
        QTRY_COMPARE(valueOf(dst, made, QStringLiteral("C1")), QStringLiteral("n:10"));   // (the open workbook's 9, plus 1: not the file's 100)
    }

    // One rule for a copy's values: the cell and the link part's cache both hold what the open workbook shows now, whether or not it has changes that are not saved (the values a program reads from
    // the saved copy without updating its links are the cell's own).
    void theLinkPartHoldsTheValuesTheCellShows()
    {
        for (const bool unsaved : {false, true}) {
            EditWindow src, dst;
            QVERIFY(src.open(dataBook()));
            QVERIFY(dst.open(dataBook()));
            QVERIFY(src.settle());
            QVERIFY(dst.settle());
            if (unsaved) {
                QVERIFY(src.win.w.session()->type(1, 0, 0, QStringLiteral("200")).ok);   // (Other!A1: not saved)
                QVERIFY(src.settle());
                QVERIFY(src.win.w.session()->isModified());
            }
            const int made = dst.win.w.calculation()->sheetCount();
            copyToOther(src, dst, 0);
            QVERIFY(dst.settle());
            const QString shown = unsaved ? QStringLiteral("n:201") : QStringLiteral("n:101");
            QTRY_COMPARE(valueOf(dst, made, QStringLiteral("C1")), shown);
            QVERIFY2(saveNow(dst), qPrintable(dst.win.told.join(QLatin1Char('|'))));
            QFile file(dst.path);
            QVERIFY(file.open(QIODevice::ReadOnly));
            QMap<QString, QByteArray> parts;
            QVERIFY(jp::readZip(file.readAll(), parts));
            file.close();
            const QString link = QString::fromUtf8(parts.value(QStringLiteral("xl/externalLinks/externalLink1.xml")));
            QVERIFY2(link.contains(QStringLiteral("<v>%1</v>").arg(unsaved ? 200 : 100)), qPrintable(link));
            QVERIFY2(!link.contains(QStringLiteral("<v>%1</v>").arg(unsaved ? 100 : 200)), qPrintable(link));
            Saved f;
            QVERIFY(f.open(dst.path));
            QCOMPARE(f.shown(made, QStringLiteral("C1")), shown);
        }
    }

    // The box's sentence about the cells that come as values reads right at one and at many, and its list has the last comma whatever the system's language does with lists.
    void theBoxCountsTheValueCellsAndJoinsItsListWithOxfordCommas()
    {
        PinnedLocale pin(QLocale(QLocale::English, QLocale::UnitedKingdom));
        EditWindow src, dst;
        QVERIFY(src.open(dataBook()));
        QVERIFY(dst.open(dataBook()));
        QVERIFY(src.settle());
        QVERIFY(dst.settle());
        crunch::EditSession::SheetExport data = src.win.w.session()->exportSheet(0, src.path);
        QVERIFY(data.ok);
        data.frozen = 1;
        data.frozenKinds = QStringList{QStringLiteral("aa")};
        QString said = dst.win.w.session()->checkImport(data).problem;
        QVERIFY2(said.contains(QStringLiteral("1 cell comes as the value it shows, not as a formula: one of those that aa.")), qPrintable(said));
        data.frozenKinds = QStringList{QStringLiteral("aa"), QStringLiteral("bb")};
        said = dst.win.w.session()->checkImport(data).problem;
        QVERIFY2(said.contains(QStringLiteral("one of those that aa and bb.")), qPrintable(said));
        data.frozen = 4;
        data.frozenKinds = QStringList{QStringLiteral("aa"), QStringLiteral("bb"), QStringLiteral("cc"), QStringLiteral("dd")};
        said = dst.win.w.session()->checkImport(data).problem;
        QVERIFY2(said.contains(QStringLiteral("4 ")) && said.contains(QStringLiteral("that aa, bb, cc, and dd.")), qPrintable(said));   // (the plural wording comes with the forms file)
        data.frozenKinds.clear();
        said = dst.win.w.session()->checkImport(data).problem;
        QVERIFY2(said.contains(QStringLiteral("4 ")) && !said.contains(QStringLiteral("one of those")), qPrintable(said));
    }

    // A copied link behaves as a link typed in the second workbook does, when the first workbook's cell changes and is saved: the same value, whichever it is.
    void aCopiedLinkBehavesAsATypedLinkDoes()
    {
        EditWindow src, dst;
        QVERIFY(src.open(dataBook()));
        QVERIFY(dst.open(dataBook()));
        QVERIFY(src.settle());
        QVERIFY(dst.settle());
        const int made = dst.win.w.calculation()->sheetCount();
        copyToOther(src, dst, 0);
        QVERIFY(dst.settle());
        QTRY_COMPARE(valueOf(dst, made, QStringLiteral("C1")), QStringLiteral("n:101"));
        // a link typed in the same workbook, to the same cell of the open workbook
        QVERIFY(dst.win.w.session()->type(made, 5, 0, QStringLiteral("=[book.xlsx]Other!A1+1")).ok);
        QVERIFY(dst.settle());
        QTRY_COMPARE(valueOf(dst, made, QStringLiteral("A6")), QStringLiteral("n:101"));
        // the first workbook's cell changes, and is saved
        QVERIFY(src.win.w.session()->type(1, 0, 0, QStringLiteral("200")).ok);
        QVERIFY(src.settle());
        QVERIFY2(saveNow(src), "the first workbook is saved");
        QVERIFY(dst.settle());
        for (int turns = 0; turns < 20; ++turns) QCoreApplication::processEvents();
        QCOMPARE(valueOf(dst, made, QStringLiteral("C1")), valueOf(dst, made, QStringLiteral("A6")));
        // (neither follows the first workbook's saved file by itself: both keep what they showed until the links are updated)
        QCOMPARE(valueOf(dst, made, QStringLiteral("C1")), QStringLiteral("n:101"));
    }

    // Several sheets are copied all or none: when the second is refused, the first is not put in; a workbook closed while the sheets were being chosen is told in words and nothing is copied.
    void severalSheetsAreCopiedAllOrNone()
    {
        EditWindow src, dst;
        QVERIFY(src.open(dataBook(true)));
        QVERIFY(dst.open(dataBook()));
        QVERIFY(src.settle());
        QVERIFY(dst.settle());
        // a block of sheets in a formula of the second sheet that is picked is refused
        QVERIFY(src.win.w.session()->type(1, 3, 0, QStringLiteral("=SUM(Data:Third!A1)")).ok);
        QVERIFY(src.settle());
        const int sheets = dst.win.w.calculation()->sheetCount();
        const int steps = dst.win.w.undoStack()->count();
        copyManyToOther(src, &dst, QSet<int>({0, 1}));
        QVERIFY2(!src.win.told.isEmpty() && src.win.told.last().contains(QStringLiteral("block of sheets")), qPrintable(src.win.told.join(QLatin1Char('|'))));
        QCOMPARE(dst.win.w.calculation()->sheetCount(), sheets);
        QCOMPARE(dst.win.w.undoStack()->count(), steps);
        // both pass when the refused one is taken away: one step of Undo for the two
        src.win.w.undoStack()->undo();
        QVERIFY(src.settle());
        src.win.told.clear();
        copyManyToOther(src, &dst, QSet<int>({0, 1}));
        QVERIFY(dst.settle());
        QCOMPARE(dst.win.w.calculation()->sheetCount(), sheets + 2);
        QCOMPARE(dst.win.w.undoStack()->count(), steps + 1);
        dst.win.w.undoStack()->undo();
        QVERIFY(dst.settle());
        QCOMPARE(dst.win.w.calculation()->sheetCount(), sheets);
        // the second workbook is closed before the copy is made
        auto gone = std::make_unique<EditWindow>();
        QVERIFY(gone->open(dataBook()));
        QVERIFY(gone->settle());
        src.win.told.clear();
        src.win.w.setMoveCopyAsker([&](CrunchWindow::MoveCopyRequest *r) {
            r->copy = true;
            r->toBook = &gone->win.w;
            r->toOther = true;
            gone.reset();   // (closed while the box was open)
            return true;
        });
        src.win.w.sheetTabs()->selectOnly(0);
        src.win.w.setActiveSheet(0);
        src.win.w.act(QStringLiteral("sheet.moveCopy"))->trigger();
        QVERIFY2(!src.win.told.isEmpty() && src.win.told.last().contains(QStringLiteral("was closed")), qPrintable(src.win.told.join(QLatin1Char('|'))));
        QCOMPARE(src.win.w.calculation()->sheetCount(), 3);
    }

    // A plain Insert Sheet is undone and redone; once the workbook is saved the step is not undoable (the saved file has the sheet, as it does a deleted row), and the session says it forgot it. The sheet
    // stays: this is the design, not a lost undo (checked through the window's own command and save).
    void anInsertedSheetIsUndoneUntilTheWorkbookIsSaved()
    {
        EditWindow e;
        QVERIFY(e.open(dataBook()));
        QVERIFY(e.settle());
        e.win.w.act(QStringLiteral("sheet.insertEnd"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.calculation()->sheetCount(), 3);
        QVERIFY(e.win.w.session()->canUndo());
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.calculation()->sheetCount(), 2);
        e.win.w.undoStack()->redo();
        QVERIFY(e.settle());
        QVERIFY2(saveNow(e), qPrintable(e.win.told.join(QLatin1Char('|'))));
        QVERIFY(!e.win.w.session()->canUndo());
        QCOMPARE(e.win.w.session()->stepsForgottenBySave(), 1);
        QCOMPARE(e.win.w.calculation()->sheetCount(), 3);
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

    // A copy of a sheet taken away by Undo while the worker is still working out its formulas leaves nothing behind: the results that come for it are dropped (they named a sheet that was
    // not there), and a sheet added at its place later starts with no values.
    void aSheetUndoneWhileItsFormulasAreWorkedOutLeavesNothingBehind()
    {
        EditWindow e;
        QVERIFY(e.open(sheetsBook()));
        crunch::EditSession *s = e.win.w.session();
        int made = -1;
        // at once, with no wait between (the worker may or may not have started on it)
        QVERIFY(s->copySheet(1, -1, QString(), &made).ok);
        QCOMPARE(made, 3);
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.calculation()->sheetCount(), 3);
        // again, with the copy worked out first: its values are there, and then they are not
        QVERIFY(s->copySheet(1, -1, QString(), &made).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, 3, QStringLiteral("A2")), QStringLiteral("n:2"));   // (the copy's A2 = Sheet1!A1*2)
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QVERIFY(s->insertSheet(-1, QString(), &made).ok);
        QCOMPARE(made, 3);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, 3, QStringLiteral("A2")), QStringLiteral("-"));
        QVERIFY(s->type(3, 1, 0, QStringLiteral("=1+1")).ok);
        QVERIFY(e.settle());
        QCOMPARE(valueOf(e, 3, QStringLiteral("A2")), QStringLiteral("n:2"));
        QVERIFY(saveNow(e));
        Saved f;
        QVERIFY(f.open(e.path));
        QCOMPARE(f.shown(3, QStringLiteral("A2")), QStringLiteral("n:2"));
    }

    // A sheet deleted takes its drawing, its chart, and its notes with it out of the saved package (and out of the list of content types); the sheet that is kept keeps its own, byte for
    // byte. Copying a sheet that has them says that they are not copied.
    void aDeletedSheetsDrawingsAndNotesAreLeftOutOfTheFile()
    {
        const QByteArray chart1 = "<?xml version=\"1.0\"?><c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"><c:chart/></c:chartSpace>";
        const QByteArray chart2 = "<?xml version=\"1.0\"?><c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"><c:chart><c:autoTitleDeleted val=\"1\"/></c:chart></c:chartSpace>";
        const QByteArray drawing = "<?xml version=\"1.0\"?><xdr:wsDr xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\"/>";
        const QByteArray comments = "<?xml version=\"1.0\"?><comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors><author>a</author></authors><commentList/></comments>";
        auto drawingRels = [](const QString &chart) {
            return QStringLiteral("<?xml version=\"1.0\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                                  "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart\" Target=\"../charts/%1\"/></Relationships>").arg(chart).toUtf8();
        };
        const QString chartType = QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml");
        const QString drawingType = QStringLiteral("application/vnd.openxmlformats-officedocument.drawing+xml");
        QVector<ExtraPart> parts;
        // Sheet2 (the second sheet): a drawing with a chart, and notes
        parts.append({QStringLiteral("xl/drawings/drawing1.xml"), drawing, drawingType, 1, QStringLiteral("rIdD"), QStringLiteral("drawing"), QStringLiteral("../drawings/drawing1.xml")});
        parts.append({QStringLiteral("xl/drawings/_rels/drawing1.xml.rels"), drawingRels(QStringLiteral("chart1.xml")), QString(), -1, QString(), QString(), QString()});
        parts.append({QStringLiteral("xl/charts/chart1.xml"), chart1, chartType, -1, QString(), QString(), QString()});
        parts.append({QStringLiteral("xl/comments1.xml"), comments, QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml"), 1, QStringLiteral("rIdC"), QStringLiteral("comments"), QStringLiteral("../comments1.xml")});
        // Sheet1: its own drawing and chart
        parts.append({QStringLiteral("xl/drawings/drawing2.xml"), drawing, drawingType, 0, QStringLiteral("rIdD"), QStringLiteral("drawing"), QStringLiteral("../drawings/drawing2.xml")});
        parts.append({QStringLiteral("xl/drawings/_rels/drawing2.xml.rels"), drawingRels(QStringLiteral("chart2.xml")), QString(), -1, QString(), QString(), QString()});
        parts.append({QStringLiteral("xl/charts/chart2.xml"), chart2, chartType, -1, QString(), QString(), QString()});
        EditWindow e;
        QVERIFY(e.open(withParts(sheetsBook(), parts)));
        // a copy of the sheet says what is not copied
        crunch::EditSession *s = e.win.w.session();
        int made = -1;
        const crunch::EditSession::Outcome copied = s->copySheet(1, -1, QString(), &made);
        QVERIFY(copied.ok);
        QVERIFY2(copied.problem.contains(QStringLiteral("not copied")), qPrintable(copied.problem));
        e.win.w.undoStack()->undo();
        // and through the window's command, the person is told
        e.win.w.setMoveCopyAsker([](CrunchWindow::MoveCopyRequest *r) {
            r->copy = true;
            r->before = -1;
            return true;
        });
        e.win.w.sheetTabs()->selectOnly(1);
        e.win.w.setActiveSheet(1);
        e.win.told.clear();
        e.win.w.act(QStringLiteral("sheet.moveCopy"))->trigger();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QStringLiteral("not copied")), qPrintable(e.win.told.join(QLatin1Char('|'))));
        e.win.w.undoStack()->undo();
        // delete the sheet, and save
        QVERIFY(s->deleteSheet(1).ok);
        QVERIFY(saveNow(e));
        QFile file(e.path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QMap<QString, QByteArray> out;
        QVERIFY(jp::readZip(file.readAll(), out));
        for (const char *gone : {"xl/worksheets/sheet2.xml", "xl/worksheets/_rels/sheet2.xml.rels", "xl/drawings/drawing1.xml", "xl/drawings/_rels/drawing1.xml.rels", "xl/charts/chart1.xml", "xl/comments1.xml"})
            QVERIFY2(!out.contains(QString::fromLatin1(gone)), gone);
        for (const char *kept : {"xl/worksheets/sheet1.xml", "xl/worksheets/sheet3.xml", "xl/worksheets/_rels/sheet1.xml.rels", "xl/drawings/drawing2.xml", "xl/drawings/_rels/drawing2.xml.rels", "xl/charts/chart2.xml"})
            QVERIFY2(out.contains(QString::fromLatin1(kept)), kept);
        QCOMPARE(out.value(QStringLiteral("xl/charts/chart2.xml")), chart2);
        const QString types = QString::fromUtf8(out.value(QStringLiteral("[Content_Types].xml")));
        for (const char *gone : {"/xl/worksheets/sheet2.xml", "/xl/drawings/drawing1.xml", "/xl/charts/chart1.xml", "/xl/comments1.xml"})
            QVERIFY2(!types.contains(QString::fromLatin1(gone)), gone);
        QVERIFY(types.contains(QStringLiteral("/xl/charts/chart2.xml")) && types.contains(QStringLiteral("/xl/worksheets/sheet3.xml")));
        Saved f;
        QVERIFY(f.open(e.path));
        QCOMPARE(f.names(), QStringList({QStringLiteral("Sheet1"), QStringLiteral("Sheet3")}));
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

    // A drag of a tab whose release never came (focus lost, Esc, the tabs taken away) is over: tabs that move later are no drag, and the release that comes at last tells nothing.
    void aTabDragIsGivenUpWhenItsReleaseIsLost()
    {
        for (int how = 0; how < 4; ++how) {   // 0 released as usual, 1 focus out, 2 Esc, 3 the tabs cleared
            crunch::SheetTabBar tabs;
            for (const char *name : {"One", "Two", "Three"}) tabs.addTab(QString::fromLatin1(name));
            tabs.resize(400, 30);
            tabs.show();
            QSignalSpy dragged(&tabs, &crunch::SheetTabBar::tabDragged);
            QTest::mousePress(&tabs, Qt::LeftButton, Qt::NoModifier, tabs.tabRect(0).center());
            if (how == 1) {
                QFocusEvent out(QEvent::FocusOut, Qt::OtherFocusReason);
                QCoreApplication::sendEvent(&tabs, &out);
            }
            if (how == 2) QTest::keyClick(&tabs, Qt::Key_Escape);
            if (how == 3) {
                tabs.clearTabs();
                for (const char *name : {"One", "Two", "Three"}) tabs.addTab(QString::fromLatin1(name));
            }
            tabs.moveTab(0, 1);
            QTest::mouseRelease(&tabs, Qt::LeftButton, Qt::NoModifier, tabs.tabRect(1).center());
            QCOMPARE(dragged.count(), how == 0 ? 1 : 0);
        }
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
