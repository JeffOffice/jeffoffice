// Tests of the second slice of editing and view gaps: the expanded formula bar, Find by format and Replace in notes, Merge Styles and Modify for a cell style, the Options settings, and the
// small commands (Repeat, the Undo list, Show and Hide Detail, outline settings, Remove Hyperlinks). Every workbook is built in the test.
#include "../cmdsearchcheck.h"
#include "app/celleditor.h"
#include "app/crunchwindow.h"
#include "app/finddialog.h"
#include "app/formatdialog.h"
#include "app/formulabaredit.h"
#include "core/formatfind.h"
#include "core/outline.h"
#include "sheet/notes.h"
#include "app/settings.h"
#include "core/editsession.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "savedok.h"

#include <QAction>
#include <QKeyEvent>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTest>
#include <QToolButton>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;

namespace {

// The text a cell holds, read with the worker held back.
QString textAt(EditWindow &e, const QString &ref, int sheet = 0)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    auto hold = e.win.w.workbookLoader().hold();
    return e.win.w.session()->editText(sheet, r, c);
}

QString textIn(Window &w, const QString &ref, int sheet = 0)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    auto hold = w.w.workbookLoader().hold();
    return w.w.session()->editText(sheet, r, c);
}

// Whether the cell is set in bold, as its format says.
bool boldAt(Window &w, const QString &ref)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    auto hold = w.w.workbookLoader().hold();
    const grid::GridSheet &gs = w.w.view()->sheet();
    const jp::office::sheet::Styles &st = w.w.loaded()->book->styles();
    return st.font(st.format(gs.styleOf(r, c, gs.cellAt(r, c))).font).bold;
}

// The style format a named style has in the window's workbook (none: a default one).
jp::office::sheet::CellFormat styleOfName(Window &w, const QString &name)
{
    auto hold = w.w.workbookLoader().hold();
    const jp::office::sheet::Styles &st = w.w.loaded()->book->styles();
    for (const jp::office::sheet::CellStyleName &n : st.cellStyles)
        if (n.name == name && n.xfId < quint32(st.styleXfs.size())) return st.styleXfs[int(n.xfId)];
    return jp::office::sheet::CellFormat();
}

bool styleBold(Window &w, const QString &name)
{
    const jp::office::sheet::CellFormat f = styleOfName(w, name);
    auto hold = w.w.workbookLoader().hold();
    return w.w.loaded()->book->styles().font(f.font).bold;
}

bool hasStyle(Window &w, const QString &name)
{
    for (const jp::office::sheet::CellStyleName &n : w.w.loaded()->book->styles().cellStyles)
        if (n.name == name && !n.hidden) return true;
    return false;
}

// A small workbook that can keep named styles (A3 is a percent).
QByteArray styledBook()
{
    const QString lists = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/><scheme val=\"minor\"/></font></fonts>"
                                         "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                         "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                                         "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>");
    const QByteArray styles = stylesXml(QString(),
                                        QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/><xf numFmtId=\"9\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyNumberFormat=\"1\"/>"), lists);
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c></row><row r=\"2\"><c r=\"A2\"><v>10</v></c><c r=\"B2\"><v>3</v></c></row>"
                                        "<row r=\"3\"><c r=\"A3\" s=\"1\"><v>0.25</v></c><c r=\"B3\"><v>4</v></c></row>");
    return buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), styles);
}

std::unique_ptr<Window> reopen(const QString &path)
{
    auto again = std::make_unique<Window>();
    if (!again->open(path)) again.reset();
    return again;
}

} // namespace

class CrunchGap2Tests : public QObject {
    Q_OBJECT
private:
    FormulaBarEdit *bar(EditWindow &e) { return static_cast<FormulaBarEdit *>(e.win.w.formulaBar()); }

    // A formula typed into the bar with a line break after each comma, finished with Enter.
    void typeBroken(EditWindow &e)
    {
        FormulaBarEdit *b = bar(e);
        b->setFocus();
        QTest::keyClicks(b, QStringLiteral("=SUM(A1,"));
        QTest::keyClick(b, Qt::Key_Return, Qt::AltModifier);
        QTest::keyClicks(b, QStringLiteral("A2)"));
    }

private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        Settings::get().remove(QStringLiteral("crunch/formulaBarExpanded"));
        Settings::get().remove(QStringLiteral("crunch/formulaBarLines"));
    }

    void init()
    {
        Settings::get().remove(QStringLiteral("crunch/formulaBarExpanded"));
        Settings::get().remove(QStringLiteral("crunch/formulaBarLines"));
    }
    void cleanup() { init(); }

    // ---- 1. the expanded formula bar
    void theFormulaBarGrowsAndShrinksWithTheCommand()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        FormulaBarEdit *b = bar(e);
        QAction *toggle = e.win.w.act(QStringLiteral("view.formulaBarExpand"));
        QVERIFY(toggle);
        QVERIFY(!toggle->isChecked() && !b->isExpanded());
        const int one = b->height();
        QCOMPARE(toggle->shortcut(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U));
        toggle->trigger();
        QVERIFY(b->isExpanded() && toggle->isChecked());
        QVERIFY2(b->height() > one * 2, qPrintable(QString::number(b->height())));
        QVERIFY(b->grip()->isVisibleTo(b));
        toggle->trigger();
        QVERIFY(!b->isExpanded());
        QTRY_COMPARE(b->height(), one);
        QVERIFY(!b->grip()->isVisibleTo(b));
        // (the same widget and the same interface in both)
        QCOMPARE(static_cast<QLineEdit *>(b), e.win.w.formulaBar());
    }

    void theExpandedBarIsAPersonsSettingAndTheHandleKeepsItsHeight()
    {
        {
            EditWindow e;
            QVERIFY(e.open(editBook()));
            e.win.w.act(QStringLiteral("view.formulaBarExpand"))->trigger();
            QVERIFY(Settings::get().value(QStringLiteral("crunch/formulaBarExpanded")).toBool());
            FormulaBarEdit *b = bar(e);
            const int lines = b->expandedLines();
            // (the handle: Down makes the bar a line taller, Up a line shorter, and the height is kept)
            b->grip()->setFocus();
            QTest::keyClick(b->grip(), Qt::Key_Down);
            QCOMPARE(b->expandedLines(), lines + 1);
            QTest::keyClick(b->grip(), Qt::Key_Up);
            QTest::keyClick(b->grip(), Qt::Key_Up);
            QCOMPARE(b->expandedLines(), lines - 1);
            for (int i = 0; i < 40; ++i) QTest::keyClick(b->grip(), Qt::Key_Up);
            QCOMPARE(b->expandedLines(), int(FormulaBarEdit::kMinLines));
            for (int i = 0; i < 40; ++i) QTest::keyClick(b->grip(), Qt::Key_Down);
            QCOMPARE(b->expandedLines(), int(FormulaBarEdit::kMaxLines));
            QTest::keyClick(b->grip(), Qt::Key_Up);
        }
        EditWindow again;   // (another window starts the way the person left the bar)
        QVERIFY(again.open(editBook()));
        QVERIFY(bar(again)->isExpanded());
        QVERIFY(again.win.w.act(QStringLiteral("view.formulaBarExpand"))->isChecked());
        QCOMPARE(bar(again)->expandedLines(), int(FormulaBarEdit::kMaxLines) - 1);
        again.win.w.act(QStringLiteral("view.formulaBarExpand"))->trigger();
        QVERIFY(!Settings::get().value(QStringLiteral("crunch/formulaBarExpanded")).toBool());
    }

    void theHandleIsDraggedWithTheMouse()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.act(QStringLiteral("view.formulaBarExpand"))->trigger();
        FormulaBarEdit *b = bar(e);
        const int before = b->expandedLines();
        QTRY_VERIFY(b->grip()->height() > 0 && b->height() > 40);
        QWidget *g = b->grip();
        const int lh = b->fontMetrics().lineSpacing();
        QTest::mousePress(g, Qt::LeftButton, Qt::NoModifier, g->rect().center());
        QMouseEvent move(QEvent::MouseMove, QPointF(g->rect().center()) + QPointF(0, 3 * lh), g->mapToGlobal(QPointF(g->rect().center()) + QPointF(0, 3 * lh)), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(g, &move);
        QTest::mouseRelease(g, Qt::LeftButton, Qt::NoModifier, g->rect().center());
        QVERIFY2(b->expandedLines() >= before + 2, qPrintable(QString::number(b->expandedLines())));
        e.win.w.act(QStringLiteral("view.formulaBarExpand"))->trigger();
    }

    void aLongFormulaScrollsAndTheCaretMovesByLine()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.act(QStringLiteral("view.formulaBarExpand"))->trigger();
        FormulaBarEdit *b = bar(e);
        b->setExpandedLines(FormulaBarEdit::kMinLines);
        QTRY_VERIFY(b->height() < 60);
        QVERIFY(!b->scrollBar()->isVisibleTo(b));
        QStringList parts;
        for (int i = 0; i < 12; ++i) parts << QStringLiteral("A%1").arg(i + 1);
        b->setFocus();
        QTest::keyClicks(b, QStringLiteral("=SUM(") + parts.join(QLatin1Char(',')));
        for (int i = 0; i < 5; ++i) {
            QTest::keyClick(b, Qt::Key_Return, Qt::AltModifier);
            QTest::keyClicks(b, QStringLiteral("B%1+").arg(i));
        }
        QVERIFY2(b->lineCount() >= 6, qPrintable(QString::number(b->lineCount())));
        QVERIFY(b->scrollBar()->isVisibleTo(b));
        QVERIFY(b->scrollBar()->maximum() >= 4);
        QVERIFY2(b->firstLine() > 0, "the end of the text is in view with the caret");
        const int last = b->lineOfPosition(b->cursorPosition());
        QTest::keyClick(b, Qt::Key_Up);
        QCOMPARE(b->lineOfPosition(b->cursorPosition()), last - 1);
        QTest::keyClick(b, Qt::Key_Up, Qt::ShiftModifier);
        QVERIFY(b->hasSelectedText());
        QTest::keyClick(b, Qt::Key_Down);
        QTest::keyClick(b, Qt::Key_Home, Qt::ControlModifier);
        QCOMPARE(b->cursorPosition(), 0);
        QTRY_COMPARE(b->firstLine(), 0);
        e.win.w.cancelEdit();
        e.win.w.act(QStringLiteral("view.formulaBarExpand"))->trigger();
    }

    void aLineBreakInTheBarStaysInTheFormulaAndIsOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.win.w.act(QStringLiteral("view.formulaBarExpand"))->trigger();
        e.select(QStringLiteral("E5"));
        const int steps = e.win.w.undoStack()->count();
        typeBroken(e);
        QCOMPARE(bar(e)->text(), QStringLiteral("=SUM(A1,\nA2)"));
        QCOMPARE(e.win.w.undoStack()->count(), steps);   // (the entry is not done: Alt+Enter did not finish it)
        QTest::keyClick(bar(e), Qt::Key_Return);
        QVERIFY(e.settle());
        QCOMPARE(e.win.w.undoStack()->count(), steps + 1);
        QCOMPARE(textAt(e, QStringLiteral("E5")), QStringLiteral("=SUM(A1,\nA2)"));
        QCOMPARE(e.valueAt(QStringLiteral("E5")), QStringLiteral("n:11"));
        e.win.w.act(QStringLiteral("view.formulaBarExpand"))->trigger();
    }

    void aLineBreakIsKeptInEveryFormatTheAppWritesAndCanHoldIt()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select(QStringLiteral("E5"));
        typeBroken(e);
        QTest::keyClick(bar(e), Qt::Key_Return);
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        QCOMPARE(savedFormula(e.path, QStringLiteral("E5")), QStringLiteral("SUM(A1,\nA2)"));
        auto reopened = [&](const QString &path) {
            auto again = std::make_unique<Window>();
            if (!again->open(path)) again.reset();
            return again;
        };
        {
            const auto again = reopened(e.copyOfFile());
            QVERIFY(again);
            QCOMPARE(textIn(*again, QStringLiteral("E5")), QStringLiteral("=SUM(A1,\nA2)"));
        }
        for (const QString &kind : {QStringLiteral("ods"), QStringLiteral("xls")}) {
            const QString copy = e.folder.dir.filePath(QStringLiteral("copy.") + kind);
            QSignalSpy exported(&e.win.w, &CrunchWindow::copyExported);
            QVERIFY(e.win.w.saveAs(copy, true));
            QVERIFY(exported.count() > 0 || exported.wait(20000));
            QVERIFY2(exported.first().at(1).toBool(), qPrintable(e.win.told.join(QLatin1Char('|'))));
            const auto again = reopened(copy);
            QVERIFY(again);
            const QString got = textIn(*again, QStringLiteral("E5"));
            // (the formula is kept, and those two forms keep no line break in a formula)
            QCOMPARE(got, QStringLiteral("=SUM(A1,A2)"));
        }
    }

    void theExpandedBarIsReachableByKeysSearchAndTheRibbon()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QString problem = jptest::commandSearchProblem(e.win.w, QStringLiteral(":/crunch/cmdsearch.json"), QStringLiteral("multiline formula"), QStringLiteral("Expand Formula Bar"));
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        // the shortcut works from the grid, and the bar shows when the command is used with the bar hidden
        e.win.w.act(QStringLiteral("view.formulaBar"))->trigger();
        QVERIFY(!bar(e)->isVisible());
        e.view()->setFocus();
        QTest::keyClick(e.view(), Qt::Key_U, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(bar(e)->isExpanded() && bar(e)->isVisible());
        QCOMPARE(bar(e)->accessibleName(), QStringLiteral("Formula bar"));
        QVERIFY(!bar(e)->grip()->accessibleName().isEmpty());
        QVERIFY(!bar(e)->scrollBar()->accessibleName().isEmpty());
        QTest::keyClick(e.view(), Qt::Key_U, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(!bar(e)->isExpanded());
    }

    // ---- 2. Find by format, and Replace in notes
    void findByFormatListsOnlyTheCellsThatHaveTheFormat()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("A3"));   // (the percent format)
        w.act(QStringLiteral("edit.find"))->trigger();
        FindDialog *d = w.findDialog();
        QVERIFY(d);
        d->findChild<QPushButton *>(QStringLiteral("findFormatFromCell"))->click();
        QVERIFY(d->format());
        QVERIFY(!d->findChild<QLabel *>(QStringLiteral("findFormatLabel"))->text().isEmpty());
        QPushButton *all = d->findChild<QPushButton *>(QStringLiteral("findAll"));
        all->click();   // (no text: every cell with that format)
        QTRY_COMPARE(d->results()->topLevelItemCount(), 1);
        QCOMPARE(d->results()->topLevelItem(0)->text(1), QStringLiteral("A3"));
        // with a text too, both must hold
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("25%"));
        all->click();
        QTRY_COMPARE(d->results()->topLevelItemCount(), 1);
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("hello"));
        all->click();
        QTRY_VERIFY(d->findChild<QLabel *>(QStringLiteral("findStatus"))->text().contains(QLatin1String("No cell matches")));
        // the format off: the same text is found in its own cell
        d->findChild<QPushButton *>(QStringLiteral("findFormatClear"))->click();
        QVERIFY(!d->format());
        all->click();
        QTRY_COMPARE(d->results()->topLevelItemCount(), 1);
        QCOMPARE(d->results()->topLevelItem(0)->text(1), QStringLiteral("C1"));
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("*"));
        all->click();
        QTRY_VERIFY(d->results()->topLevelItemCount() > 3);
    }

    void findByTheFormatBoxFindsTheCellsThatAreBold()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("B2"));
        w.act(QStringLiteral("fmt.bold"))->trigger();
        QVERIFY(boldAt(e.win, QStringLiteral("B2")));
        w.act(QStringLiteral("edit.find"))->trigger();
        FindDialog *d = w.findDialog();
        w.setFormatAsker([](const FormatCellsDialog::Input &, FormatCellsDialog::Result *r) {
            r->change.bold = true;
            return true;
        });
        d->findChild<QPushButton *>(QStringLiteral("findFormat"))->click();
        QVERIFY(d->format() && d->format()->bold && *d->format()->bold);
        d->findChild<QPushButton *>(QStringLiteral("findAll"))->click();
        QTRY_VERIFY(d->results()->topLevelItemCount() > 0);
        QStringList got;
        for (int i = 0; i < d->results()->topLevelItemCount(); ++i) got << d->results()->topLevelItem(i)->text(1);
        QCOMPARE(got.join(QLatin1Char(' ')), QStringLiteral("B2"));
        // a dialog that changes nothing leaves no format
        w.setFormatAsker([](const FormatCellsDialog::Input &, FormatCellsDialog::Result *) { return true; });
        d->findChild<QPushButton *>(QStringLiteral("findFormat"))->click();
        QVERIFY(!d->format());
    }

    void replaceAllInNotesIsOneStepRefusedOnAProtectedSheetAndSavedInEveryFormat()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        QVERIFY(s->setNote(0, 0, 0, QStringLiteral("say hello twice: hello"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok);
        QVERIFY(s->setNote(0, 2, 2, QStringLiteral("Hello there"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok);
        QVERIFY(s->setNote(0, 1, 1, QStringLiteral("nothing here"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok);
        auto noteText = [&](crunch::EditSession *session, int row, int col) {
            const auto *n = session->anchors().noteAt(0, quint32(row), quint32(col));
            return n ? n->text : QString();
        };
        w.act(QStringLiteral("find.replace"))->trigger();
        FindDialog *d = w.findDialog();
        d->setTab(FindDialog::Tab::Replace);
        d->findChild<QComboBox *>(QStringLiteral("findLookIn"))->setCurrentIndex(2);
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("hello"));
        d->findChild<QLineEdit *>(QStringLiteral("replaceText"))->setText(QStringLiteral("bye"));
        // a protected sheet refuses, and nothing changes
        jp::office::sheet::SheetProtection locked;
        locked.enabled = true;
        locked.objects = true;
        QVERIFY(s->setSheetProtection(0, locked, QStringLiteral("Protect")).ok);
        int steps = w.undoStack()->index();
        e.win.told.clear();
        d->findChild<QPushButton *>(QStringLiteral("replaceAll"))->click();
        QTRY_VERIFY(!e.win.told.isEmpty());
        QVERIFY2(e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(w.undoStack()->index(), steps);
        QCOMPARE(noteText(s, 0, 0), QStringLiteral("say hello twice: hello"));
        w.undoStack()->undo();   // (the protection)
        QVERIFY(!s->protectionOf(0).enabled);
        steps = w.undoStack()->index();
        d->findChild<QPushButton *>(QStringLiteral("replaceAll"))->click();
        QTRY_COMPARE(w.undoStack()->index(), steps + 1);
        QCOMPARE(noteText(s, 0, 0), QStringLiteral("say bye twice: bye"));
        QCOMPARE(noteText(s, 2, 2), QStringLiteral("bye there"));   // (matching is not case sensitive)
        QCOMPARE(noteText(s, 1, 1), QStringLiteral("nothing here"));
        const QString said = d->findChild<QLabel *>(QStringLiteral("findStatus"))->text();
        QVERIFY2(said.contains(QLatin1String("Changed 3 ")) && said.contains(QLatin1String(" in 2 ")), qPrintable(said));
        // one step takes both back, and gives both again
        w.undoStack()->undo();
        QCOMPARE(noteText(s, 0, 0), QStringLiteral("say hello twice: hello"));
        QCOMPARE(noteText(s, 2, 2), QStringLiteral("Hello there"));
        w.undoStack()->redo();
        QCOMPARE(noteText(s, 0, 0), QStringLiteral("say bye twice: bye"));
        // the app's own save, and the copies in the other formats, keep the new words
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        {
            const auto again = reopen(e.copyOfFile());
            QVERIFY(again);
            QCOMPARE(noteText(again->w.session(), 0, 0), QStringLiteral("say bye twice: bye"));
            QCOMPARE(noteText(again->w.session(), 2, 2), QStringLiteral("bye there"));
        }
        for (const QString &kind : {QStringLiteral("xls"), QStringLiteral("ods")}) {
            const QString copy = e.folder.dir.filePath(QStringLiteral("copy.") + kind);
            QSignalSpy exported(&w, &CrunchWindow::copyExported);
            QVERIFY(w.saveAs(copy, true));
            QVERIFY(exported.count() > 0 || exported.wait(20000));
            QVERIFY2(exported.first().at(1).toBool(), qPrintable(e.win.told.join(QLatin1Char('|'))));
            const auto again = reopen(copy);
            QVERIFY(again);
            QVERIFY2(noteText(again->w.session(), 0, 0).contains(QLatin1String("bye twice: bye")), qPrintable(kind + noteText(again->w.session(), 0, 0)));
            QVERIFY2(noteText(again->w.session(), 2, 2).contains(QLatin1String("bye there")), qPrintable(kind));
        }
    }

    void replaceOneInNotesChangesTheNoteOfTheCursorCellAndGoesOn()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        QVERIFY(s->setNote(0, 0, 0, QStringLiteral("a cat"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok);
        QVERIFY(s->setNote(0, 2, 2, QStringLiteral("a cat too"), QStringLiteral("Ann"), false, QStringLiteral("Note")).ok);
        e.select(QStringLiteral("A1"));
        w.act(QStringLiteral("find.replace"))->trigger();
        FindDialog *d = w.findDialog();
        d->setTab(FindDialog::Tab::Replace);
        d->findChild<QComboBox *>(QStringLiteral("findLookIn"))->setCurrentIndex(2);
        d->findChild<QLineEdit *>(QStringLiteral("findText"))->setText(QStringLiteral("cat"));
        d->findChild<QLineEdit *>(QStringLiteral("replaceText"))->setText(QStringLiteral("dog"));
        const int steps = w.undoStack()->count();
        d->findChild<QPushButton *>(QStringLiteral("replaceOne"))->click();
        QTRY_COMPARE(w.undoStack()->count(), steps + 1);
        QCOMPARE(s->anchors().noteAt(0, 0, 0)->text, QStringLiteral("a dog"));
        QCOMPARE(s->anchors().noteAt(0, 2, 2)->text, QStringLiteral("a cat too"));
        QTRY_COMPARE(e.selected(), QStringLiteral("C3"));   // (and the search went on to the next note)
        QCOMPARE(w.undoStack()->count(), steps + 1);
    }

    // ---- 3. Modify and Merge Styles
    void modifyingAStyleChangesItAndEveryCellThatUsesItInOneStepAndIsSaved()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.setStyleNameAsker([](QString *n) {
            *n = QStringLiteral("Mine");
            return true;
        });
        e.select(QStringLiteral("A3"));
        w.act(QStringLiteral("style.new"))->trigger();
        QVERIFY(hasStyle(e.win, QStringLiteral("Mine")));
        QVERIFY(w.session()->applyStyleNamed(0, rangeOf(QStringLiteral("B1")), QStringLiteral("Mine"), QStringLiteral("Style")).ok);   // B1
        QVERIFY(e.settle());
        QVERIFY(!styleBold(e.win, QStringLiteral("Mine")) && !boldAt(e.win, QStringLiteral("A3")) && !boldAt(e.win, QStringLiteral("B1")));
        w.setFormatAsker([](const FormatCellsDialog::Input &in, FormatCellsDialog::Result *r) {
            // (the dialog starts from the style's own look: a percent)
            if (in.view.numberFormat != QLatin1String("0%")) return false;
            r->change.bold = true;
            return true;
        });
        const int steps = w.undoStack()->count();
        w.act(QStringLiteral("style.modify"))->trigger();
        QVERIFY(e.settle());
        QCOMPARE(w.undoStack()->count(), steps + 1);
        QVERIFY(styleBold(e.win, QStringLiteral("Mine")));
        QVERIFY(boldAt(e.win, QStringLiteral("A3")) && boldAt(e.win, QStringLiteral("B1")));
        QVERIFY2(!boldAt(e.win, QStringLiteral("A1")), "a cell that does not use the style");
        w.undoStack()->undo();
        QVERIFY(!styleBold(e.win, QStringLiteral("Mine")) && !boldAt(e.win, QStringLiteral("A3")) && !boldAt(e.win, QStringLiteral("B1")));
        w.undoStack()->redo();
        QVERIFY(styleBold(e.win, QStringLiteral("Mine")) && boldAt(e.win, QStringLiteral("B1")));
        // the app's own save keeps the style's new look (a cell given the style afterwards has it) and the cells'
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        {
            const auto again = reopen(e.copyOfFile());
            QVERIFY(again);
            QVERIFY(styleBold(*again, QStringLiteral("Mine")));
            QVERIFY(boldAt(*again, QStringLiteral("A3")) && boldAt(*again, QStringLiteral("B1")));
            QVERIFY(again->w.session()->applyStyleNamed(0, rangeOf(QStringLiteral("A1")), QStringLiteral("Mine"), QStringLiteral("Style")).ok);
            QVERIFY(boldAt(*again, QStringLiteral("A1")));
        }
        for (const QString &kind : {QStringLiteral("xls"), QStringLiteral("ods")}) {
            const QString copy = e.folder.dir.filePath(QStringLiteral("copy.") + kind);
            QSignalSpy exported(&w, &CrunchWindow::copyExported);
            QVERIFY(w.saveAs(copy, true));
            QVERIFY(exported.count() > 0 || exported.wait(20000));
            QVERIFY2(exported.first().at(1).toBool(), qPrintable(e.win.told.join(QLatin1Char('|'))));
            const auto again = reopen(copy);
            QVERIFY(again);
            QVERIFY2(boldAt(*again, QStringLiteral("A3")) && boldAt(*again, QStringLiteral("B1")), qPrintable(kind));
        }
    }

    void modifyingAStyleOnAProtectedSheetIsRefusedWholeAndNormalCannotBeModified()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        w.setStyleNameAsker([](QString *n) {
            *n = QStringLiteral("Mine");
            return true;
        });
        e.select(QStringLiteral("A3"));
        w.act(QStringLiteral("style.new"))->trigger();
        w.setFormatAsker([](const FormatCellsDialog::Input &, FormatCellsDialog::Result *r) {
            r->change.bold = true;
            return true;
        });
        jp::office::sheet::SheetProtection locked;
        locked.enabled = true;
        QVERIFY(w.session()->setSheetProtection(0, locked, QStringLiteral("Protect")).ok);
        const int steps = w.undoStack()->count();
        e.win.told.clear();
        w.act(QStringLiteral("style.modify"))->trigger();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(w.undoStack()->count(), steps);
        QVERIFY(!styleBold(e.win, QStringLiteral("Mine")) && !boldAt(e.win, QStringLiteral("A3")));
        w.undoStack()->undo();
        e.win.told.clear();
        w.setStyleNameAsker([](QString *n) {
            *n = QStringLiteral("Normal");
            return true;
        });
        w.act(QStringLiteral("style.modify"))->trigger();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("Normal")), qPrintable(e.win.told.join('|')));
        QVERIFY(!styleBold(e.win, QStringLiteral("Normal")));
    }

    void mergeStylesBringsInTheStylesOfAnotherWorkbookAndAsksBeforeReplacing()
    {
        EditWindow dest, source;
        QVERIFY(dest.open(styledBook()));
        QVERIFY(source.open(styledBook()));
        QVERIFY(dest.settle() && source.settle());
        // the source: "Mine" in bold, and "Fancy"; the destination: its own "Mine" (not bold)
        source.win.w.setStyleNameAsker([](QString *n) {
            *n = QStringLiteral("Mine");
            return true;
        });
        source.select(QStringLiteral("B1"));
        source.win.w.act(QStringLiteral("fmt.bold"))->trigger();
        source.win.w.act(QStringLiteral("style.new"))->trigger();
        source.win.w.setStyleNameAsker([](QString *n) {
            *n = QStringLiteral("Fancy");
            return true;
        });
        source.select(QStringLiteral("A3"));
        source.win.w.act(QStringLiteral("style.new"))->trigger();
        dest.win.w.setStyleNameAsker([](QString *n) {
            *n = QStringLiteral("Mine");
            return true;
        });
        dest.select(QStringLiteral("A3"));
        dest.win.w.act(QStringLiteral("style.new"))->trigger();
        QVERIFY(styleBold(source.win, QStringLiteral("Mine")) && !styleBold(dest.win, QStringLiteral("Mine")) && !hasStyle(dest.win, QStringLiteral("Fancy")));
        CrunchWindow &w = dest.win.w;
        const QString title = source.win.w.displayName();
        w.setWindowPicker([&](const QStringList &titles) { return int(titles.indexOf(title)); });
        // kept: the destination's own "Mine" stays, "Fancy" comes in
        int asked = -1;
        w.setStyleMergeAsker([&](int count) {
            asked = count;
            return false;
        });
        int steps = w.undoStack()->index();
        w.act(QStringLiteral("style.merge"))->trigger();
        QCOMPARE(asked, 1);
        QCOMPARE(w.undoStack()->index(), steps + 1);
        QVERIFY(hasStyle(dest.win, QStringLiteral("Fancy")) && !styleBold(dest.win, QStringLiteral("Mine")));
        w.undoStack()->undo();
        QVERIFY(!hasStyle(dest.win, QStringLiteral("Fancy")));
        // replaced: asked once, and "Mine" takes the source's look, in one step that Undo takes back
        w.setStyleMergeAsker([&](int count) {
            asked = count;
            return true;
        });
        asked = -1;
        steps = w.undoStack()->index();
        w.act(QStringLiteral("style.merge"))->trigger();
        QVERIFY(dest.settle());
        QCOMPARE(asked, 1);
        QCOMPARE(w.undoStack()->index(), steps + 1);
        QVERIFY(hasStyle(dest.win, QStringLiteral("Fancy")) && styleBold(dest.win, QStringLiteral("Mine")));
        QVERIFY(boldAt(dest.win, QStringLiteral("A3")));   // (the cell that uses "Mine" follows)
        w.undoStack()->undo();
        QVERIFY(!hasStyle(dest.win, QStringLiteral("Fancy")) && !styleBold(dest.win, QStringLiteral("Mine")) && !boldAt(dest.win, QStringLiteral("A3")));
        w.undoStack()->redo();
        QVERIFY(styleBold(dest.win, QStringLiteral("Mine")));
        // merged again, the styles are the same: nothing is asked and nothing is added
        asked = -1;
        steps = w.undoStack()->index();
        w.act(QStringLiteral("style.merge"))->trigger();
        QCOMPARE(asked, -1);
        QCOMPARE(w.undoStack()->index(), steps);
        // saved: the file the app writes has them
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        const auto again = reopen(dest.copyOfFile());
        QVERIFY(again);
        QVERIFY(hasStyle(*again, QStringLiteral("Fancy")) && styleBold(*again, QStringLiteral("Mine")));
    }

    void mergeStylesWithNoOtherWorkbookSaysSo()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        e.win.told.clear();
        const int steps = e.win.w.undoStack()->count();
        e.win.w.act(QStringLiteral("style.merge"))->trigger();
        QVERIFY(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("Open the workbook")));
        QCOMPARE(e.win.w.undoStack()->count(), steps);
    }

    // ---- 5. Show Detail, Hide Detail, and Remove Hyperlinks
    void hideAndShowDetailCollapseAndOpenTheGroupInOneStepEachAndAreSaved()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        auto hidden = [&](Window &win, quint32 row) {
            auto hold = win.w.workbookLoader().hold();
            return win.w.session()->outlineOf(0, false).hiddenOf(row);
        };
        // rows 1 to 2 are a group, and row 3 sums them up
        QVERIFY(w.session()->changeOutline(0, false, crunch::Outline({}, 1048576, true).group(0, 1), QStringLiteral("Group")).ok);
        e.select(QStringLiteral("A3"));
        int steps = w.undoStack()->index();
        w.act(QStringLiteral("outline.hideDetail"))->trigger();
        QCOMPARE(w.undoStack()->index(), steps + 1);
        QVERIFY(hidden(e.win, 0) && hidden(e.win, 1) && !hidden(e.win, 2));
        // saved: the file the app writes, and the copies, hold the hidden lines
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        {
            const auto again = reopen(e.copyOfFile());
            QVERIFY(again);
            QVERIFY(hidden(*again, 0) && hidden(*again, 1));
        }
        for (const QString &kind : {QStringLiteral("xls"), QStringLiteral("ods")}) {
            const QString copy = e.folder.dir.filePath(QStringLiteral("copy.") + kind);
            QSignalSpy exported(&w, &CrunchWindow::copyExported);
            QVERIFY(w.saveAs(copy, true));
            QVERIFY(exported.count() > 0 || exported.wait(20000));
            QVERIFY2(exported.first().at(1).toBool(), qPrintable(e.win.told.join(QLatin1Char('|'))));
            const auto again = reopen(copy);
            QVERIFY(again);
            QVERIFY2(hidden(*again, 0) && hidden(*again, 1), qPrintable(kind));
        }
        // Hide Detail again: nothing to hide (the group is collapsed), said in a message
        e.win.told.clear();
        steps = w.undoStack()->index();
        w.act(QStringLiteral("outline.hideDetail"))->trigger();
        QVERIFY(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("Nothing was changed")));
        QCOMPARE(w.undoStack()->index(), steps);
        w.act(QStringLiteral("outline.showDetail"))->trigger();
        QCOMPARE(w.undoStack()->index(), steps + 1);
        QVERIFY(!hidden(e.win, 0) && !hidden(e.win, 1));
        w.undoStack()->undo();
        QVERIFY(hidden(e.win, 0));
        // with a cell that is in no group: said, nothing changed
        e.select(QStringLiteral("A9"));
        e.win.told.clear();
        w.act(QStringLiteral("outline.showDetail"))->trigger();
        QVERIFY(!e.win.told.isEmpty());
        QVERIFY(hidden(e.win, 0));
    }

    void showAndHideDetailOnAProtectedSheetAreRefused()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        QVERIFY(w.session()->changeOutline(0, false, crunch::Outline({}, 1048576, true).group(0, 1), QStringLiteral("Group")).ok);
        jp::office::sheet::SheetProtection locked;
        locked.enabled = true;
        QVERIFY(w.session()->setSheetProtection(0, locked, QStringLiteral("Protect")).ok);
        e.select(QStringLiteral("A3"));
        const int steps = w.undoStack()->index();
        e.win.told.clear();
        w.act(QStringLiteral("outline.hideDetail"))->trigger();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(w.undoStack()->index(), steps);
        auto hold = w.workbookLoader().hold();
        QVERIFY(!w.session()->outlineOf(0, false).hiddenOf(0));
    }

    void removeHyperlinksTakesTheLinkAndItsLookOffInOneStepAndIsSaved()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        crunch::EditSession *s = w.session();
        jp::office::sheet::LinkSpec link;
        link.target = QStringLiteral("https://example.com/");
        QVERIFY(s->setLink(0, rangeOf(QStringLiteral("A1")), link, QStringLiteral("Link")).ok);
        QVERIFY(s->applyStyleNamed(0, rangeOf(QStringLiteral("A1")), QStringLiteral("Hyperlink"), QStringLiteral("Link look")).ok);
        QVERIFY(e.settle());
        auto underlined = [&](Window &win) {
            auto hold = win.w.workbookLoader().hold();
            const grid::GridSheet &gs = win.w.view()->sheet();
            const jp::office::sheet::Styles &st = win.w.loaded()->book->styles();
            return st.font(st.format(gs.styleOf(0, 0, gs.cellAt(0, 0))).font).underline != jp::office::sheet::Underline::None;
        };
        QVERIFY(underlined(e.win) && s->anchors().linkAt(0, 0, 0));
        // a protected sheet that does not allow links refuses, and nothing changes
        jp::office::sheet::SheetProtection locked;
        locked.enabled = true;
        QVERIFY(s->setSheetProtection(0, locked, QStringLiteral("Protect")).ok);
        e.select(QStringLiteral("A1"));
        int steps = w.undoStack()->index();
        e.win.told.clear();
        w.act(QStringLiteral("link.removeAll"))->setEnabled(true);
        w.act(QStringLiteral("link.removeAll"))->trigger();
        QVERIFY2(!e.win.told.isEmpty() && e.win.told.last().contains(QLatin1String("protected")), qPrintable(e.win.told.join('|')));
        QCOMPARE(w.undoStack()->index(), steps);
        QVERIFY(underlined(e.win) && s->anchors().linkAt(0, 0, 0));
        w.undoStack()->undo();   // (the protection)
        e.select(QStringLiteral("A1"));
        QVERIFY(w.act(QStringLiteral("link.removeAll"))->isEnabled());
        steps = w.undoStack()->index();
        w.act(QStringLiteral("link.removeAll"))->trigger();
        QCOMPARE(w.undoStack()->index(), steps + 1);
        QVERIFY(!underlined(e.win) && !s->anchors().linkAt(0, 0, 0));
        w.undoStack()->undo();
        QVERIFY(underlined(e.win) && s->anchors().linkAt(0, 0, 0));
        w.undoStack()->redo();
        QVERIFY(!underlined(e.win) && !s->anchors().linkAt(0, 0, 0));
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        {
            const auto again = reopen(e.copyOfFile());
            QVERIFY(again);
            QVERIFY(!underlined(*again) && !again->w.session()->anchors().linkAt(0, 0, 0));
        }
        for (const QString &kind : {QStringLiteral("xls"), QStringLiteral("ods")}) {
            const QString copy = e.folder.dir.filePath(QStringLiteral("copy.") + kind);
            QSignalSpy exported(&w, &CrunchWindow::copyExported);
            QVERIFY(w.saveAs(copy, true));
            QVERIFY(exported.count() > 0 || exported.wait(20000));
            const auto again = reopen(copy);
            QVERIFY(again);
            QVERIFY2(!underlined(*again) && !again->w.session()->anchors().linkAt(0, 0, 0), qPrintable(kind));
        }
    }

    void removeHyperlinksIsFoundBySearchAndIsOnTheClearMenu()
    {
        EditWindow e;
        QVERIFY(e.open(styledBook()));
        const QString problem = jptest::commandSearchProblem(e.win.w, QStringLiteral(":/crunch/cmdsearch.json"), QStringLiteral("strip hyperlinks"), QStringLiteral("Remove Hyperlinks"));
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        const QString problem2 = jptest::commandSearchProblem(e.win.w, QStringLiteral(":/crunch/cmdsearch.json"), QStringLiteral("collapse group"), QStringLiteral("Hide Detail"));
        QVERIFY2(problem2.isEmpty(), qPrintable(problem2));
    }
};

JP_TEST_CLASS(CrunchGap2Tests)
#include "tst_crunch_gaps2.moc"
