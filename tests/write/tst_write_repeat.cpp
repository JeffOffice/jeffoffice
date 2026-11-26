// Tests of JeffWrite's Repeat (F4, and Ctrl+Y while there is nothing to redo) and of the history lists on Undo and Redo.
// Each document is built here from a few lines of XML.

#include "../isolatesettings.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "app/ribbon.h"
#include "app/theme.h"
#include "app/undohistory.h"
#include "app/widgets.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/editops.h"
#include "core/fonts.h"
#include "app/shapetextedit.h"
#include "core/reviewops.h"
#include "core/textboxops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/package.h"

#include <QAction>
#include <QMenu>
#include <QPointer>
#include <QToolButton>
#include <QSignalSpy>
#include <QTest>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp;
using namespace jp::write;

namespace {

const char kNs[] = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";

QByteArray xml(const QByteArray &body) { return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n") + body; }

QByteArray makeDocx(const QByteArray &body)
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    w.addPart(QStringLiteral("/word/document.xml"), xml(QByteArray("<w:document ") + kNs + "><w:body>" + body + "</w:body></w:document>"),
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    return w.finish();
}

QByteArray para(const char *text) { return QByteArray("<w:p><w:r><w:t xml:space=\"preserve\">") + text + "</w:t></w:r></w:p>"; }

std::unique_ptr<WriteDoc> open(const QByteArray &zip)
{
    QString err;
    auto doc = WriteDoc::fromDocx(zip, &err);
    if (!doc) qWarning("open: %s", qPrintable(err));
    return doc;
}

// A window with a document of the paragraphs given.
struct Win {
    WriteWindow w;
    explicit Win(const QList<const char *> &paragraphs)
    {
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        QByteArray body;
        for (const char *p : paragraphs) body += para(p);
        w.setDocument(open(makeDocx(body)), QString());
    }
    QTextDocument *story() { return w.document()->story(); }
    QString text() { return story()->toPlainText(); }
    // The words of the text selected (the first time `words` is in it).
    void select(const QString &words)
    {
        const int at = text().indexOf(words);
        QTextCursor c(story());
        c.setPosition(at);
        c.setPosition(at + int(words.size()), QTextCursor::KeepAnchor);
        w.view()->setCursor(c);
    }
    bool boldAt(const QString &words)
    {
        QTextCursor c(story());
        c.setPosition(text().indexOf(words) + 1);
        return c.charFormat().fontWeight() >= QFont::DemiBold;
    }
    QAction *act(const char *id) { return w.act(QString::fromLatin1(id)); }
    QAction *ctrlY() { return w.findChild<QAction *>(QStringLiteral("edit.repeat.ctrlY")); }
    void settle() { w.refreshForTest(); }
    int steps() { return int(w.document()->undoNames(100000).size()); }
    // The undos it takes to get back to the start.
    int undosToStart()
    {
        int n = 0;
        while (w.document()->canUndo() && n < 1000) {
            w.document()->undo();
            ++n;
        }
        return n;
    }
};

} // namespace

class RepeatTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        initCore();
    }

    // Which commands Repeat does again: those that format, set a paragraph, style, list, table, or insert something with nothing to answer; never file, view, find, review, or clipboard commands.
    void repeatableCommandsAreNamedOneByOne()
    {
        for (const char *id : {"fmt.bold", "fmt.alignCenter", "fmt.grow", "para.bullets", "para.indentInc", "style.normal", "style.heading2", "list.level.3", "table.insertBelow", "table.deleteRows",
                               "table.border.all", "insert.pageBreak", "insert.special.emDash", "fmt.case.cycle"})
            QVERIFY2(WriteWindow::repeatableCommand(QString::fromLatin1(id)), id);
        for (const char *id : {"file.save", "file.close", "file.new", "view.zoom", "zoom.in", "edit.find", "edit.replace", "edit.paste", "edit.cut", "edit.undo", "edit.redo", "edit.repeat", "edit.selectAll",
                               "review.trackChanges", "review.acceptAll", "fmt.fontDialog", "para.dialog", "insert.table", "insert.picture", "table.selectRow", "style.manage", "help.show", ""})
            QVERIFY2(!WriteWindow::repeatableCommand(QString::fromLatin1(id)), id);
        // Every command named is a command of the window (a name with a typo would never be repeated).
        Win win({"one"});
        int found = 0;
        for (const char *id : {"fmt.bold", "fmt.italic", "fmt.underline", "fmt.strike", "fmt.subscript", "fmt.superscript", "fmt.charBorder", "fmt.grow", "fmt.shrink", "fmt.clear", "fmt.alignLeft",
                               "fmt.alignCenter", "fmt.alignRight", "fmt.justify", "fmt.case.cycle", "para.bullets", "para.numbers", "para.indentInc", "para.indentDec", "para.horizontalLine",
                               "style.normal", "style.heading1", "list.bulletNone", "list.numberNone", "table.insertAbove", "table.insertBelow", "table.insertLeft", "table.insertRight",
                               "table.deleteCells", "table.deleteRows", "table.deleteColumns", "table.deleteTable", "table.moveUp", "table.moveDown", "table.merge", "table.autofitContents",
                               "table.autofitWindow", "table.fixedWidth", "table.distributeRows", "table.distributeColumns", "table.splitTable", "table.style.clear", "insert.pageBreak", "insert.blankPage"}) {
            QVERIFY2(win.act(id), id);
            QVERIFY2(WriteWindow::repeatableCommand(QString::fromLatin1(id)), id);
            ++found;
        }
        QCOMPARE(found, 44);
    }

    void nothingToRepeatAtFirst()
    {
        Win win({"one two three"});
        win.settle();
        QVERIFY(!win.act("edit.repeat")->isEnabled());
        QVERIFY(!win.ctrlY()->isEnabled());
        QCOMPARE(QKeySequence(Qt::Key_F4).toString(), win.act("edit.repeat")->shortcut().toString());
        win.act("edit.repeat")->trigger();   // (nothing happens)
        QCOMPARE(win.steps(), 0);
    }

    // The last command done again on what is selected now, as one step, and the menu says which.
    void repeatDoesTheLastCommandOnTheSelectionNowAsOneStep()
    {
        Win win({"one two three"});
        win.select(QStringLiteral("one"));
        win.act("fmt.bold")->trigger();
        QVERIFY(win.boldAt(QStringLiteral("one")));
        win.settle();
        QVERIFY(win.act("edit.repeat")->isEnabled());
        QVERIFY2(win.act("edit.repeat")->toolTip().contains(QLatin1String("Repeat Bold")), qPrintable(win.act("edit.repeat")->toolTip()));
        QVERIFY(win.act("edit.repeat")->statusTip().contains(QLatin1String("Repeat Bold")));
        const int before = win.steps();
        win.select(QStringLiteral("three"));
        win.act("edit.repeat")->trigger();
        QVERIFY(win.boldAt(QStringLiteral("three")));
        QVERIFY(!win.boldAt(QStringLiteral("two")));
        QCOMPARE(win.steps(), before + 1);
        QCOMPARE(win.w.document()->undoNames(1).first(), QStringLiteral("Bold"));
        // One Undo takes back the repeat and only it.
        win.w.view()->undo();
        QVERIFY(!win.boldAt(QStringLiteral("three")));
        QVERIFY(win.boldAt(QStringLiteral("one")));
        // Repeat goes on repeating the same command.
        win.select(QStringLiteral("two"));
        win.act("edit.repeat")->trigger();
        QVERIFY(win.boldAt(QStringLiteral("two")));
    }

    // A paragraph command and a style are repeated on the paragraph of now.
    void aParagraphCommandAndAStyleAreRepeated()
    {
        Win win({"first", "second", "third"});
        win.select(QStringLiteral("first"));
        win.act("fmt.alignCenter")->trigger();
        win.select(QStringLiteral("second"));
        win.act("edit.repeat")->trigger();
        QCOMPARE(win.story()->findBlockByNumber(1).blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
        QCOMPARE(win.story()->findBlockByNumber(2).blockFormat().alignment() & Qt::AlignHorizontal_Mask, Qt::AlignLeft);
        auto styleOf = [&win](int block) {
            QTextCursor c(win.story()->findBlockByNumber(block));
            return paragraphStyleAt(*win.w.document(), c);
        };
        const QString normal = styleOf(0);
        win.select(QStringLiteral("first"));
        win.act("style.heading1")->trigger();
        const QString heading = styleOf(0);
        QVERIFY(heading != normal);
        win.select(QStringLiteral("third"));
        win.act("edit.repeat")->trigger();
        QCOMPARE(styleOf(2), heading);
        QCOMPARE(styleOf(1), normal);
    }

    // A command whose value is chosen in a box of the ribbon (the size of the letters) is repeated with the value.
    void aValueChosenInTheRibbonIsRepeatedWithTheValue()
    {
        Win win({"one two three"});
        win.select(QStringLiteral("one"));
        QTRY_VERIFY(!win.w.findChildren<SizeCombo *>().isEmpty());
        const auto combos = win.w.findChildren<SizeCombo *>();
        Q_EMIT combos.first()->sizeChosen(23.0);
        win.settle();
        QVERIFY(win.act("edit.repeat")->toolTip().contains(QLatin1String("Repeat Font Size")));
        win.select(QStringLiteral("three"));
        win.act("edit.repeat")->trigger();
        QTextCursor c(win.story());
        c.setPosition(win.text().indexOf(QStringLiteral("three")) + 1);
        QCOMPARE(c.charFormat().fontPointSize(), 23.0);
        c.setPosition(win.text().indexOf(QStringLiteral("two")) + 1);
        QVERIFY(c.charFormat().fontPointSize() != 23.0);
    }

    // The words typed in a run are the last command: Repeat types them again where the caret is, as one step.
    void aRunOfTypingIsRepeatedAsOneStep()
    {
        Win win({"start end"});
        win.w.view()->setFocus();
        QTextCursor c(win.story());
        c.setPosition(5);
        win.w.view()->setCursor(c);
        QTest::keyClicks(win.w.view(), QStringLiteral("xyz"));
        QCOMPARE(win.text(), QStringLiteral("startxyz end"));
        QCOMPARE(win.w.view()->typedRun(), QStringLiteral("xyz"));
        win.settle();
        QVERIFY(win.act("edit.repeat")->isEnabled());
        QVERIFY(win.act("edit.repeat")->toolTip().contains(QLatin1String("Repeat Typing")));
        QTextCursor end(win.story());
        end.movePosition(QTextCursor::End);
        win.w.view()->setCursor(end);
        const int before = win.steps();
        win.act("edit.repeat")->trigger();
        QCOMPARE(win.text(), QStringLiteral("startxyz endxyz"));
        QCOMPARE(win.steps(), before + 1);
        win.w.view()->undo();
        QCOMPARE(win.text(), QStringLiteral("startxyz end"));
        // The run is still what was typed (a repeat is not a new run), so a second repeat types the same.
        win.act("edit.repeat")->trigger();
        QCOMPARE(win.text(), QStringLiteral("startxyz endxyz"));
    }

    // A run that the corrections change (a word fixed as the space is typed) is typed again with them, and is still one step.
    void aRunWithACorrectionIsRepeatedWithItAsOneStep()
    {
        Win win({"Go to "});
        Settings::get().setValue(QStringLiteral("proof/autocorrect"), true);
        win.w.view()->setFocus();
        QTextCursor c(win.story());
        c.movePosition(QTextCursor::End);
        win.w.view()->setCursor(c);
        QTest::keyClicks(win.w.view(), QStringLiteral("teh "));
        QCOMPARE(win.text(), QStringLiteral("Go to the "));
        QCOMPARE(win.w.view()->typedRun(), QStringLiteral("the "));   // (what the corrections made of it: Repeat puts it in as it is)
        const int before = win.steps();
        win.settle();
        win.act("edit.repeat")->trigger();
        QCOMPARE(win.text(), QStringLiteral("Go to the the "));
        QCOMPARE(win.steps(), before + 1);
        // One real Undo takes the whole repeat back.
        win.w.view()->undo();
        QCOMPARE(win.text(), QStringLiteral("Go to the "));
    }

    // A command given after the typing is the one repeated, and typing after the command is the one repeated then.
    void theLatestOfTypingAndACommandIsRepeated()
    {
        Win win({"abc def"});
        win.w.view()->setFocus();
        QTextCursor c(win.story());
        c.setPosition(0);
        win.w.view()->setCursor(c);
        QTest::keyClicks(win.w.view(), QStringLiteral("QQ"));
        win.select(QStringLiteral("def"));
        win.act("fmt.bold")->trigger();
        win.settle();
        QVERIFY(win.act("edit.repeat")->toolTip().contains(QLatin1String("Repeat Bold")));
        QTextCursor at(win.story());
        at.setPosition(win.text().indexOf(QStringLiteral("abc")));
        win.w.view()->setCursor(at);
        QTest::keyClicks(win.w.view(), QStringLiteral("W"));   // (a new run: the caret was moved)
        win.settle();
        QVERIFY(win.act("edit.repeat")->toolTip().contains(QLatin1String("Repeat Typing")));
    }

    // A run longer than the limit is not kept: Repeat is off rather than doing part of it.
    void aRunOfTypingPastTheLimitIsNotKept()
    {
        Win win({"x"});
        win.w.view()->setFocus();
        win.act("fmt.bold")->trigger();   // (a command is recorded first: the long run takes its place, and then goes)
        QTextCursor c(win.story());
        c.movePosition(QTextCursor::End);
        win.w.view()->setCursor(c);
        const int limit = write::PageView::kMaxTypedRun;
        for (int i = 0; i < limit; ++i) win.w.view()->insertText(QStringLiteral("a"));
        win.settle();
        QCOMPARE(win.w.view()->typedRun().size(), limit);
        QVERIFY(win.act("edit.repeat")->isEnabled());
        win.w.view()->insertText(QStringLiteral("a"));
        win.settle();
        QVERIFY(win.w.view()->typedRun().isEmpty());
        QVERIFY(!win.act("edit.repeat")->isEnabled());
    }

    // Under Track Changes the repeat is tracked as the command was.
    void aRepeatUnderTrackChangesIsTrackedLikeTheCommand()
    {
        Win win({"one two three"});
        win.w.document()->setTrackChanges(true);
        win.select(QStringLiteral("one"));
        win.act("fmt.bold")->trigger();
        auto changes = [&win] {
            QString err;
            office::Package p;
            p.read(win.w.document()->toDocx(&err));
            return QString::fromUtf8(p.part(QStringLiteral("/word/document.xml"))).count(QLatin1String("<w:rPrChange"));
        };
        const int one = changes();
        QVERIFY(one >= 1);
        win.select(QStringLiteral("three"));
        win.act("edit.repeat")->trigger();
        QVERIFY(win.boldAt(QStringLiteral("three")));
        QCOMPARE(changes(), one * 2);
    }

    // A document that is read only, protected, or held by a search takes no repeat, and the command says why where it is given.
    void aReadOnlyOrProtectedDocumentTakesNoRepeat()
    {
        Win win({"one two three"});
        win.select(QStringLiteral("one"));
        win.act("fmt.bold")->trigger();
        win.settle();
        QVERIFY(win.act("edit.repeat")->isEnabled());
        win.w.setReadOnly(true);
        win.select(QStringLiteral("three"));
        const int before = win.steps();
        // (Before the commands follow the state, as a key pressed at once would find them: the command itself refuses, and says why.)
        win.act("edit.repeat")->trigger();
        QCOMPARE(win.steps(), before);
        QVERIFY(!win.boldAt(QStringLiteral("three")));
        QVERIFY2(win.w.statusBar()->currentMessage().contains(QLatin1String("reading only")), qPrintable(win.w.statusBar()->currentMessage()));
        win.settle();
        QVERIFY(!win.act("edit.repeat")->isEnabled());
        win.w.setReadOnly(false);
        win.settle();
        QVERIFY(win.act("edit.repeat")->isEnabled());
        win.w.document()->setProtection(office::wml::makeDocumentProtection(QStringLiteral("readOnly"), QString()));
        QVERIFY(win.w.isReadOnly());
        win.settle();
        QVERIFY(!win.act("edit.repeat")->isEnabled());
        win.act("edit.repeat")->trigger();
        QVERIFY(!win.boldAt(QStringLiteral("three")));
    }

    // Nothing is carried from one document to another: not to another window, and not to the document that takes the place of the window's.
    void aRepeatNeverReachesAnotherDocument()
    {
        Win a({"one two three"});
        Win b({"alpha beta gamma"});
        a.select(QStringLiteral("one"));
        a.act("fmt.bold")->trigger();
        a.settle();
        b.settle();
        QVERIFY(a.act("edit.repeat")->isEnabled());
        QVERIFY(!b.act("edit.repeat")->isEnabled());
        b.select(QStringLiteral("beta"));
        b.act("edit.repeat")->trigger();
        QVERIFY(!b.boldAt(QStringLiteral("beta")));
        QCOMPARE(b.steps(), 0);
        // The window's document replaced by another: what was recorded is not for it.
        a.w.setDocument(open(makeDocx(para("new words here"))), QString());
        a.settle();
        QVERIFY(!a.act("edit.repeat")->isEnabled());
        a.select(QStringLiteral("words"));
        a.act("edit.repeat")->trigger();
        QVERIFY(!a.boldAt(QStringLiteral("words")));
        QCOMPARE(a.steps(), 0);
        // A command given in the new document is repeated in it.
        a.act("fmt.italic")->trigger();
        a.settle();
        QVERIFY(a.act("edit.repeat")->toolTip().contains(QLatin1String("Repeat Italic")));
    }

    // F4 is always Repeat; Ctrl+Y is Redo while there is a step to redo, and Repeat when there is none.
    void ctrlYIsRedoWhileThereIsAStepToRedoAndRepeatAfter()
    {
        Win win({"one two three"});
        win.select(QStringLiteral("one"));
        win.act("fmt.bold")->trigger();
        win.settle();
        QVERIFY(!win.act("edit.redo")->isEnabled());
        QVERIFY(win.ctrlY()->isEnabled());
        QCOMPARE(win.ctrlY()->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_Y));
        win.act("edit.undo")->trigger();
        win.settle();
        QVERIFY(win.act("edit.redo")->isEnabled());
        QVERIFY(!win.ctrlY()->isEnabled());   // (the key is Redo now)
        QVERIFY(win.act("edit.repeat")->isEnabled());   // (F4 still repeats)
        win.act("edit.redo")->trigger();
        win.settle();
        QVERIFY(win.ctrlY()->isEnabled());
        QVERIFY(win.boldAt(QStringLiteral("one")));
    }

    // The lists on Undo and Redo name the steps, newest first, and a name is one Undo.
    void theHistoryListsNameEachStepAndCountOneUndoEach()
    {
        Win win({"one two three"});
        win.select(QStringLiteral("one"));
        win.act("fmt.bold")->trigger();
        win.select(QStringLiteral("two"));
        win.act("fmt.italic")->trigger();
        win.w.view()->setFocus();
        QTextCursor end(win.story());
        end.movePosition(QTextCursor::End);
        win.w.view()->setCursor(end);
        QTest::keyClicks(win.w.view(), QStringLiteral("ab"));
        win.select(QStringLiteral("three"));
        win.act("edit.repeat")->trigger();   // (typing, by the last run: it types "ab" over "three")
        const QStringList names = win.w.document()->undoNames(100);
        QVERIFY(names.size() >= 4);
        QCOMPARE(names.at(0), QStringLiteral("Typing"));
        QCOMPARE(names.at(names.size() - 1), QStringLiteral("Bold"));
        QCOMPARE(names.at(names.size() - 2), QStringLiteral("Italic"));
        QCOMPARE(win.w.document()->undoNames(2), names.mid(0, 2));
        QVERIFY(win.w.document()->redoNames(100).isEmpty());
        win.w.document()->undo();
        QCOMPARE(win.w.document()->redoNames(100), QStringList{names.first()});
        QCOMPARE(win.w.document()->undoNames(100), names.mid(1));
        // One name, one Undo: all the names are all the Undos there are.
        QCOMPARE(win.undosToStart(), int(names.size()) - 1);
        QCOMPARE(win.w.document()->redoNames(100).size(), int(names.size()));
        int redos = 0;
        while (win.w.document()->canRedo() && redos < 1000) {
            win.w.document()->redo();
            ++redos;
        }
        QCOMPARE(redos, int(names.size()));
    }

    // The steps a command made in several places count as one name.
    void aGroupOfStepsIsOneNameAndOneUndo()
    {
        Win win({"one two three"});
        win.select(QStringLiteral("one"));
        win.act("fmt.bold")->trigger();
        {
            const WriteDoc::StepGroup group(*win.w.document());
            win.w.document()->setStepName(QStringLiteral("Both"));
            win.select(QStringLiteral("two"));
            win.act("fmt.italic")->trigger();
            win.select(QStringLiteral("three"));
            win.act("fmt.underline")->trigger();
        }
        const QStringList names = win.w.document()->undoNames(100);
        QCOMPARE(names.size(), 2);
        QCOMPARE(win.undosToStart(), 2);
    }

    // The list on the button does the steps it marks, through the window's own Undo (so a read-only document refuses).
    void theHistoryListOnTheButtonDoesTheStepsItMarks()
    {
        Win win({"one two three"});
        for (const char *word : {"one", "two", "three"}) {
            win.select(QString::fromLatin1(word));
            win.act("fmt.bold")->trigger();
        }
        UndoHistoryMenu *list = nullptr;
        QTRY_VERIFY(([&] {
            for (QToolButton *b : win.w.findChildren<QToolButton *>())
                if (b->defaultAction() == win.act("edit.undo") && qobject_cast<UndoHistoryMenu *>(b->menu())) list = qobject_cast<UndoHistoryMenu *>(b->menu());
            return list != nullptr;
        }()));
        Q_EMIT list->aboutToShow();
        QCOMPARE(list->shownSteps(), 3);
        QCOMPARE(list->rowName(0), QStringLiteral("Bold"));
        list->choose(2);
        QVERIFY(!win.boldAt(QStringLiteral("three")));
        QVERIFY(!win.boldAt(QStringLiteral("two")));
        QVERIFY(win.boldAt(QStringLiteral("one")));
    }
    // A repeat is always a step of its own, wherever the caret is (typing after typing joins the step before it, so it is made not to).
    void aRepeatOfTypingAtTheEndOfTheRunIsItsOwnStep()
    {
        Win win({"start"});
        win.w.view()->setFocus();
        QTextCursor c(win.story());
        c.movePosition(QTextCursor::End);
        win.w.view()->setCursor(c);
        QTest::keyClicks(win.w.view(), QStringLiteral("abc"));
        win.settle();
        QCOMPARE(win.steps(), 1);
        win.act("edit.repeat")->trigger();   // (the caret is still where the typing left it)
        QCOMPARE(win.text(), QStringLiteral("startabcabc"));
        QCOMPARE(win.steps(), 2);
        win.act("edit.repeat")->trigger();
        QCOMPARE(win.text(), QStringLiteral("startabcabcabc"));
        QCOMPARE(win.steps(), 3);
        QCOMPARE(win.w.document()->undoNames(10), (QStringList{"Typing", "Typing", "Typing"}));
        win.w.view()->undo();
        QCOMPARE(win.text(), QStringLiteral("startabcabc"));
        win.w.view()->undo();
        QCOMPARE(win.text(), QStringLiteral("startabc"));
        // Typing after a repeat is a step of its own too.
        win.act("edit.repeat")->trigger();
        QTest::keyClicks(win.w.view(), QStringLiteral("z"));
        QCOMPARE(win.steps(), 3);
        win.w.view()->undo();
        QCOMPARE(win.text(), QStringLiteral("startabcabc"));
    }

    // A command that closes the window leaves the window alone when it is repeated (the window may be gone when it ends).
    void aRepeatedCommandThatClosesTheWindowLeavesItAlone()
    {
        QPointer<WriteWindow> w = new WriteWindow;
        w->setNotifier([](const QString &, const QString &) {});
        w->show();
        w->setDocument(open(makeDocx(para("one two"))), QString());
        int runs = 0;
        bool closeNow = false;
        QPointer<WriteWindow> raw = w;
        QAction *a = w->addRepeatableCommandForTest(QStringLiteral("test.closer"), [&runs, &closeNow, raw] {
            ++runs;
            if (closeNow) delete raw.data();
        });
        a->trigger();
        QCOMPARE(runs, 1);
        QVERIFY(w);
        closeNow = true;
        w->refreshForTest();
        w->act(QStringLiteral("edit.repeat"))->trigger();
        QCOMPARE(runs, 2);
        QVERIFY(!w);
    }
    // The commands that turn a look on or off are repeated as the state they made: on is on, off is off, a selection that has part of it gets all of it or none, and one that has it already
    // is left alone with no step.
    void aToggleIsRepeatedAsTheStateItMadeNotAsAToggle()
    {
        struct Look { const char *id; CharToggle which; };
        for (const Look &look : {Look{"fmt.bold", CharToggle::Bold}, Look{"fmt.italic", CharToggle::Italic}, Look{"fmt.underline", CharToggle::Underline}, Look{"fmt.strike", CharToggle::Strike},
                                 Look{"fmt.subscript", CharToggle::Subscript}, Look{"fmt.superscript", CharToggle::Superscript}, Look{"fmt.charBorder", CharToggle::Border}}) {
            Win win({"one two three four"});
            auto has = [&win, &look](const QString &words) {
                win.select(words);
                return selectionHasLook(win.w.view()->cursor(), look.which);
            };
            auto repeat = [&win] { win.act("edit.repeat")->trigger(); };
            const QString where = QString::fromLatin1(look.id);
            // On: the command made it on, so Repeat makes the next selection have it, whatever it had.
            win.select(QStringLiteral("one"));
            win.act(look.id)->trigger();
            QVERIFY2(has(QStringLiteral("one")), qPrintable(where));
            int steps = win.steps();
            win.select(QStringLiteral("two"));
            repeat();
            QVERIFY2(has(QStringLiteral("two")), qPrintable(where));
            QCOMPARE(win.steps(), steps + 1);
            // Mixed: one and two have it, three has not: all of them have it after.
            win.select(QStringLiteral("two three"));
            repeat();
            QVERIFY2(has(QStringLiteral("two three")), qPrintable(where));
            QCOMPARE(win.steps(), steps + 2);
            // Already in the state: nothing changes and there is no step.
            win.select(QStringLiteral("one"));
            repeat();
            QVERIFY2(has(QStringLiteral("one")), qPrintable(where + " stays on"));
            QCOMPARE(win.steps(), steps + 2);
            // Off: the command made it off, so Repeat takes it off a selection that has it.
            win.select(QStringLiteral("one two three"));
            win.act(look.id)->trigger();   // (the spaces had not: all of it has it now)
            QVERIFY2(has(QStringLiteral("one two three")), qPrintable(where));
            win.act(look.id)->trigger();   // (all have it: it comes off)
            QVERIFY2(!has(QStringLiteral("one")) && !has(QStringLiteral("three")), qPrintable(where));
            win.select(QStringLiteral("four"));
            win.act(look.id)->trigger();   // (on)
            win.select(QStringLiteral("two"));
            win.act(look.id)->trigger();   // (on: two has it, then it is off)
            QVERIFY2(has(QStringLiteral("two")), qPrintable(where));
            win.select(QStringLiteral("two"));
            win.act(look.id)->trigger();
            QVERIFY2(!has(QStringLiteral("two")), qPrintable(where));
            // (The last command made it off.) A selection that has it all, and one that has part of it, lose it.
            win.select(QStringLiteral("four"));
            QVERIFY2(has(QStringLiteral("four")), qPrintable(where));
            repeat();
            QVERIFY2(!has(QStringLiteral("four")), qPrintable(where + " off"));
            win.select(QStringLiteral("one"));
            setCharToggle(*win.w.document(), win.w.view()->cursor(), look.which, true);
            win.select(QStringLiteral("one two"));
            QVERIFY2(!selectionHasLook(win.w.view()->cursor(), look.which), qPrintable(where));
            steps = win.steps();
            repeat();
            QVERIFY2(!has(QStringLiteral("one")) && !has(QStringLiteral("two")), qPrintable(where + " mixed off"));
            QCOMPARE(win.steps(), steps + 1);
            // Already off: nothing changes and there is no step.
            win.select(QStringLiteral("three four"));
            repeat();
            QCOMPARE(win.steps(), steps + 1);
        }
    }

    // Bullets and numbering are repeated as the state they made, the same way.
    void aListCommandIsRepeatedAsTheStateItMade()
    {
        for (const bool numbered : {false, true}) {
            Win win({"aa", "bb", "cc", "dd"});
            const char *id = numbered ? "para.numbers" : "para.bullets";
            const ListKind kind = numbered ? ListKind::Numbers : ListKind::Bullets;
            auto in = [&win, kind](int block) { return listKindOf(*win.w.document(), win.story()->findBlockByNumber(block)) == kind; };
            win.select(QStringLiteral("aa"));
            win.act(id)->trigger();
            QVERIFY(in(0));
            int steps = win.steps();
            win.select(QStringLiteral("bb"));
            win.act("edit.repeat")->trigger();
            QVERIFY(in(1));
            QCOMPARE(win.steps(), steps + 1);
            // Already in a list: no step.
            win.select(QStringLiteral("aa"));
            win.act("edit.repeat")->trigger();
            QVERIFY(in(0));
            QCOMPARE(win.steps(), steps + 1);
            // Mixed: all get the list.
            win.select(QStringLiteral("bb"));
            win.w.view()->cursor().movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
            QTextCursor c(win.story());
            c.setPosition(win.story()->findBlockByNumber(1).position());
            c.setPosition(win.story()->findBlockByNumber(2).position() + 1, QTextCursor::KeepAnchor);
            win.w.view()->setCursor(c);
            win.act("edit.repeat")->trigger();
            QVERIFY(in(1) && in(2));
            // Off: taken off all of a selection that has it, and off one that has part of it.
            win.select(QStringLiteral("aa"));
            win.act(id)->trigger();
            QVERIFY(!in(0));
            QVERIFY(in(1));
            steps = win.steps();
            c.setPosition(win.story()->findBlockByNumber(1).position());
            c.setPosition(win.story()->findBlockByNumber(3).position() + 1, QTextCursor::KeepAnchor);
            win.w.view()->setCursor(c);
            win.act("edit.repeat")->trigger();
            QVERIFY(!in(1) && !in(2) && !in(3));
            QCOMPARE(win.steps(), steps + 1);
            win.select(QStringLiteral("dd"));
            win.act("edit.repeat")->trigger();
            QCOMPARE(win.steps(), steps + 1);
        }
    }

    // Every command that Repeat does that has a check mark is a toggle that is repeated as a state (a command added later that is checkable has to be named here, or it would toggle).
    void everyCheckableRepeatedCommandIsRepeatedAsAState()
    {
        Win win({"one"});
        const QSet<QString> states = {"fmt.bold", "fmt.italic", "fmt.underline", "fmt.strike", "fmt.subscript", "fmt.superscript", "fmt.charBorder", "para.bullets", "para.numbers",
                                      "fmt.alignLeft", "fmt.alignCenter", "fmt.alignRight", "fmt.justify"};   // (the alignments set a value: they are states already)
        for (QAction *a : win.w.findChildren<QAction *>()) {
            if (!a->isCheckable() || !WriteWindow::repeatableCommand(a->objectName())) continue;
            QVERIFY2(states.contains(a->objectName()), qPrintable(a->objectName()));
        }
    }

    // Repeating where the person is editing: in a box of text, the repeat goes into the box (never into the text behind it), and a command the box cannot take is refused.
    void aRepeatInABoxGoesIntoTheBoxOrIsRefused()
    {
        Win win({"outside words"});
        win.w.view()->setFocus();
        QTextCursor c(win.story());
        c.setPosition(3);
        QCOMPARE(insertTextBoxAt(*win.w.document(), c, QPointF(300, 200), QPointF(400, 260)), PictureResult::Ok);
        QTextCursor end(win.story());
        end.movePosition(QTextCursor::End);
        win.w.view()->setCursor(end);
        QTest::keyClicks(win.w.view(), QStringLiteral("QQ"));
        win.select(QStringLiteral("words"));
        win.act("fmt.strike")->trigger();
        win.w.view()->setCursor(end);
        QTest::keyClicks(win.w.view(), QStringLiteral("QQ"));
        win.w.view()->setCursor(end);
        const int box = win.story()->toPlainText().indexOf(QChar(0xFFFC));
        QVERIFY(box >= 0);
        win.w.view()->selectPicture(box);
        QVERIFY(win.w.view()->beginShapeTextEdit());
        ShapeTextEdit *editor = win.w.view()->shapeEditor();
        QVERIFY(editor);
        const QString body = win.text();
        const int steps = win.steps();
        win.settle();
        // The typing goes into the box.
        QVERIFY(win.act("edit.repeat")->toolTip().contains(QLatin1String("Repeat Typing")));
        win.act("edit.repeat")->trigger();
        QCOMPARE(editor->toPlainText(), QStringLiteral("QQ"));
        QCOMPARE(win.text(), body);
        QCOMPARE(win.steps(), steps);
        // A command the box has is done in the box.
        win.select(QStringLiteral("words"));
        win.act("fmt.bold")->trigger();   // (the box is being edited: it is the box that this does)
        editor->selectAll();
        win.act("fmt.bold")->trigger();
        QVERIFY(editor->charFormat().font().bold());
        editor->moveCursor(QTextCursor::Start);
        editor->moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);
        editor->setBold(false);
        win.act("edit.repeat")->trigger();
        QVERIFY(editor->charFormat().font().bold());
        QCOMPARE(win.text(), body);
        // A command the box cannot take is refused with the reason, and does nothing behind it.
        win.act("fmt.strike")->trigger();   // (it is recorded; the box does not take it)
        win.w.statusBar()->clearMessage();
        win.act("edit.repeat")->trigger();
        QVERIFY2(win.w.statusBar()->currentMessage().contains(QLatin1String("box")), qPrintable(win.w.statusBar()->currentMessage()));
        QCOMPARE(win.text(), body);
        win.w.view()->endShapeTextEdit(false);
    }

    // Repeating in a header or footer, a note, or a cell of a table goes where the caret is.
    void aRepeatInAHeaderANoteAndATableCellGoesWhereTheCaretIs()
    {
        // A header.
        {
            Win win({"body words"});
            win.w.view()->setFocus();
            QTextCursor c(win.story());
            c.movePosition(QTextCursor::End);
            win.w.view()->setCursor(c);
            QTest::keyClicks(win.w.view(), QStringLiteral("qq"));
            const QString body = win.text();
            QVERIFY(win.w.view()->enterHeaderFooter(0, false, true));
            QVERIFY(win.w.view()->inHeaderFooter());
            win.settle();
            win.act("edit.repeat")->trigger();
            QVERIFY2(win.w.view()->cursor().document()->toPlainText().contains(QLatin1String("qq")), qPrintable(win.w.view()->cursor().document()->toPlainText()));
            QCOMPARE(win.text(), body);
        }
        // A note.
        {
            Win win({"body words"});
            win.w.view()->setFocus();
            QTextCursor c(win.story());
            c.setPosition(4);
            win.w.view()->setCursor(c);
            win.act("refs.footnote")->trigger();
            QVERIFY(win.w.view()->inNote());
            QTest::keyClicks(win.w.view(), QStringLiteral("nn"));
            const QString body = win.text();
            win.settle();
            win.act("edit.repeat")->trigger();
            QVERIFY(win.w.view()->cursor().document()->toPlainText().contains(QLatin1String("nnnn")));
            QCOMPARE(win.text(), body);
        }
        // A cell.
        {
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            w.show();
            const QByteArray table = "<w:tbl><w:tblPr/><w:tblGrid><w:gridCol w:w=\"3000\"/><w:gridCol w:w=\"3000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>a1</w:t></w:r></w:p></w:tc>"
                                     "<w:tc><w:p><w:r><w:t>b1</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>after</w:t></w:r></w:p>";
            w.setDocument(open(makeDocx(table)), QString());
            w.view()->setFocus();
            QTextDocument *story = w.document()->story();
            QTextCursor c(story);
            c.setPosition(story->toPlainText().indexOf(QLatin1String("a1")) + 2);
            w.view()->setCursor(c);
            QTest::keyClicks(w.view(), QStringLiteral("xy"));
            QVERIFY(story->toPlainText().contains(QLatin1String("a1xy")));
            w.refreshForTest();
            w.act(QStringLiteral("edit.repeat"))->trigger();
            QVERIFY2(story->toPlainText().contains(QLatin1String("a1xyxy")), qPrintable(story->toPlainText()));
            QVERIFY(story->toPlainText().contains(QLatin1String("b1")) && !story->toPlainText().contains(QLatin1String("b1x")));
        }
    }

    // A long run is put in as one edit: one caret update for the whole repeat, not one for each letter.
    void aLongRunIsRepeatedAsOneEditWithOneCaretUpdate()
    {
        Win win({"x"});
        win.w.view()->setFocus();
        QTextCursor c(win.story());
        c.movePosition(QTextCursor::End);
        win.w.view()->setCursor(c);
        for (int i = 0; i < 1000; ++i) win.w.view()->insertText(QStringLiteral("a"));
        QCOMPARE(win.w.view()->typedRun().size(), 1000);
        win.settle();
        QSignalSpy carets(win.w.view(), &PageView::caretChanged);
        win.act("edit.repeat")->trigger();
        QCOMPARE(win.text().size(), 2001);
        QCOMPARE(carets.count(), 1);
        QCOMPARE(win.steps(), 2);
    }

    // A command that types inside itself, or runs another command, keeps its own name for the whole step.
    void aCompoundCommandKeepsItsNameForTheWholeStep()
    {
        Win win({"one two three"});
        win.w.view()->setFocus();
        WriteWindow *w = &win.w;
        QAction *compound = win.w.addRepeatableCommandForTest(QStringLiteral("Compound"), [w] {
            w->view()->insertText(QStringLiteral("zz"));
            w->view()->selectAll();
            w->act(QStringLiteral("fmt.italic"))->trigger();
        });
        win.select(QStringLiteral("two"));
        compound->trigger();
        QCOMPARE(win.w.document()->undoNames(10), (QStringList{"Compound", "Compound"}));
        win.select(QStringLiteral("three"));
        win.act("edit.repeat")->trigger();
        QVERIFY(win.w.document()->undoNames(1).first() == QLatin1String("Compound"));
        for (const QString &name : win.w.document()->undoNames(10)) QCOMPARE(name, QStringLiteral("Compound"));
        // The command that is named for a step by a command outside it does not leave its name behind.
        win.select(QStringLiteral("one"));
        win.act("fmt.italic")->trigger();
        QCOMPARE(win.w.document()->undoNames(1).first(), QStringLiteral("Italic"));
    }

    // Ctrl+Y is Redo while the history has a step to redo, by the history as it is that moment (not by a state that follows it later).
    void ctrlYFollowsTheHistoryAtOnceAfterUndoAndRedo()
    {
        Win win({"one two three"});
        win.select(QStringLiteral("one"));
        win.act("fmt.bold")->trigger();
        win.settle();
        QVERIFY(win.ctrlY()->isEnabled());
        win.w.view()->undo();   // (no settling: a key pressed now finds the commands as they are)
        QVERIFY(win.act("edit.redo")->isEnabled());
        QVERIFY(!win.ctrlY()->isEnabled());
        win.w.view()->redo();
        QVERIFY(!win.act("edit.redo")->isEnabled());
        QVERIFY(win.ctrlY()->isEnabled());
        win.w.view()->undo();
        win.w.document()->redo();
        QVERIFY(!win.act("edit.redo")->isEnabled());
        QVERIFY(win.ctrlY()->isEnabled());
    }

    // The run is what was typed: arrow keys do not take it away, and Backspace takes off it the letters it deletes.
    void backspaceShortensTheRunAndArrowKeysLeaveIt()
    {
        Win win({"start"});
        win.w.view()->setFocus();
        QTextCursor c(win.story());
        c.movePosition(QTextCursor::End);
        win.w.view()->setCursor(c);
        QTest::keyClicks(win.w.view(), QStringLiteral("abcd"));
        QTest::keyClick(win.w.view(), Qt::Key_Backspace);
        QCOMPARE(win.w.view()->typedRun(), QStringLiteral("abc"));
        QTest::keyClick(win.w.view(), Qt::Key_Backspace);
        QCOMPARE(win.w.view()->typedRun(), QStringLiteral("ab"));
        QTest::keyClicks(win.w.view(), QStringLiteral("e"));   // (typing goes on from there)
        QCOMPARE(win.w.view()->typedRun(), QStringLiteral("abe"));
        QTest::keyClick(win.w.view(), Qt::Key_Left);
        QTest::keyClick(win.w.view(), Qt::Key_Right);
        QCOMPARE(win.w.view()->typedRun(), QStringLiteral("abe"));
        QTest::keyClick(win.w.view(), Qt::Key_Left);
        QTest::keyClick(win.w.view(), Qt::Key_Backspace);   // (not at the end of the run: it is not the run's letter)
        QCOMPARE(win.w.view()->typedRun(), QStringLiteral("abe"));
        QTextCursor end(win.story());
        end.movePosition(QTextCursor::End);
        win.w.view()->setCursor(end);
        win.settle();
        win.act("edit.repeat")->trigger();
        QVERIFY2(win.text().endsWith(QLatin1String("abe")), qPrintable(win.text()));
        // All of it deleted: nothing is left to repeat.
        Win other({"start"});
        other.w.view()->setFocus();
        QTextCursor oc(other.story());
        oc.movePosition(QTextCursor::End);
        other.w.view()->setCursor(oc);
        QTest::keyClicks(other.w.view(), QStringLiteral("xy"));
        QTest::keyClick(other.w.view(), Qt::Key_Backspace);
        QTest::keyClick(other.w.view(), Qt::Key_Backspace);
        QVERIFY(other.w.view()->typedRun().isEmpty());
        other.settle();
        QVERIFY(!other.act("edit.repeat")->isEnabled());
    }

    // Protection that lets nothing through, and a search that holds the text, refuse a repeat with the reason, and leave the text as it is.
    void everyKindOfProtectionAndASearchRefuseARepeat()
    {
        for (const char *kind : {"forms", "comments", "readOnly"}) {
            Win win({"one two three"});
            win.select(QStringLiteral("one"));
            win.act("fmt.bold")->trigger();
            win.settle();
            win.w.document()->setProtection(office::wml::makeDocumentProtection(QString::fromLatin1(kind), QString()));
            QVERIFY2(win.w.isReadOnly(), kind);
            win.select(QStringLiteral("three"));
            const int steps = win.steps();
            win.w.statusBar()->clearMessage();
            win.act("edit.repeat")->trigger();
            QVERIFY2(!win.boldAt(QStringLiteral("three")), kind);
            QCOMPARE(win.steps(), steps);
            QVERIFY2(!win.w.statusBar()->currentMessage().isEmpty(), kind);
        }
        // Tracked changes only: the repeat goes through, and is tracked.
        {
            Win win({"one two three"});
            win.select(QStringLiteral("one"));
            win.act("fmt.bold")->trigger();
            win.w.document()->setProtection(office::wml::makeDocumentProtection(QStringLiteral("trackedChanges"), QString()));
            QVERIFY(!win.w.isReadOnly());
            QVERIFY(win.w.document()->trackChanges());
            win.select(QStringLiteral("three"));
            win.act("edit.repeat")->trigger();
            QVERIFY(win.boldAt(QStringLiteral("three")));
            QString err;
            office::Package p;
            p.read(win.w.document()->toDocx(&err));
            QVERIFY(QString::fromUtf8(p.part(QStringLiteral("/word/document.xml"))).contains(QLatin1String("<w:rPrChange")));
        }
        // A search that holds the text.
        {
            Win win({"one two three"});
            win.select(QStringLiteral("one"));
            win.act("fmt.bold")->trigger();
            win.w.beginSearchBlock();
            win.w.view()->beginSearchBlock();
            win.select(QStringLiteral("three"));
            const int steps = win.steps();
            win.w.statusBar()->clearMessage();
            win.act("edit.repeat")->trigger();
            QVERIFY(!win.boldAt(QStringLiteral("three")));
            QCOMPARE(win.steps(), steps);
            QVERIFY(!win.w.statusBar()->currentMessage().isEmpty());
            win.w.view()->endSearchBlock();
            win.w.endSearchBlock();
        }
        // Words kept locked by a control are left alone.
        {
            const QByteArray locked = "<w:p><w:r><w:t>free</w:t></w:r></w:p><w:sdt><w:sdtPr><w:lock w:val=\"sdtContentLocked\"/></w:sdtPr><w:sdtContent><w:p><w:r><w:t>kept words</w:t></w:r></w:p></w:sdtContent></w:sdt>";
            WriteWindow w;
            w.setNotifier([](const QString &, const QString &) {});
            w.show();
            w.setDocument(open(makeDocx(locked)), QString());
            QTextDocument *story = w.document()->story();
            auto select = [&](const QString &words) {
                QTextCursor c(story);
                const int at = story->toPlainText().indexOf(words);
                c.setPosition(at);
                c.setPosition(at + int(words.size()), QTextCursor::KeepAnchor);
                w.view()->setCursor(c);
            };
            select(QStringLiteral("free"));
            w.act(QStringLiteral("fmt.bold"))->trigger();
            select(QStringLiteral("kept"));
            const int steps = int(w.document()->undoNames(1000).size());
            w.act(QStringLiteral("edit.repeat"))->trigger();
            QTextCursor probe(story);
            probe.setPosition(story->toPlainText().indexOf(QLatin1String("kept")) + 1);
            QVERIFY(probe.charFormat().fontWeight() < QFont::DemiBold);
            QCOMPARE(int(w.document()->undoNames(1000).size()), steps);
        }
    }

    // The window a command came from closed, and the object a command worked on deleted.
    void aRepeatAfterTheDocumentOrTheObjectIsGoneDoesNothingHarmful()
    {
        Win b({"alpha beta"});
        {
            WriteWindow *a = new WriteWindow;
            a->setNotifier([](const QString &, const QString &) {});
            a->show();
            a->setDocument(open(makeDocx(para("one two three"))), QString());
            QTextCursor c(a->document()->story());
            c.setPosition(0);
            c.setPosition(3, QTextCursor::KeepAnchor);
            a->view()->setCursor(c);
            a->act(QStringLiteral("fmt.bold"))->trigger();
            delete a;
        }
        b.settle();
        QVERIFY(!b.act("edit.repeat")->isEnabled());
        b.select(QStringLiteral("beta"));
        b.act("edit.repeat")->trigger();
        QVERIFY(!b.boldAt(QStringLiteral("beta")));
        QCOMPARE(b.steps(), 0);
        // A table command, then the table deleted, then Repeat where there is no table: nothing breaks, and nothing is made.
        WriteWindow w;
        w.setNotifier([](const QString &, const QString &) {});
        w.show();
        const QByteArray table = "<w:tbl><w:tblPr/><w:tblGrid><w:gridCol w:w=\"3000\"/><w:gridCol w:w=\"3000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>a1</w:t></w:r></w:p></w:tc>"
                                 "<w:tc><w:p><w:r><w:t>b1</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>after</w:t></w:r></w:p>";
        w.setDocument(open(makeDocx(table)), QString());
        QTextDocument *story = w.document()->story();
        QTextCursor c(story);
        c.setPosition(story->toPlainText().indexOf(QLatin1String("a1")) + 1);
        w.view()->setCursor(c);
        w.act(QStringLiteral("table.insertBelow"))->trigger();
        w.act(QStringLiteral("table.deleteTable"))->trigger();
        QVERIFY(!story->toPlainText().contains(QLatin1String("a1")));
        w.act(QStringLiteral("table.insertBelow"))->trigger();   // (the repeated command, now with no table at the caret, is the last one given)
        const QString text = story->toPlainText();
        const int steps = int(w.document()->undoNames(1000).size());
        QTextCursor out(story);
        out.setPosition(text.indexOf(QLatin1String("after")) + 1);
        w.view()->setCursor(out);
        w.act(QStringLiteral("edit.repeat"))->trigger();
        QCOMPARE(story->toPlainText(), text);
        QVERIFY(int(w.document()->undoNames(1000).size()) <= steps + 1);
    }
};

QTEST_MAIN(RepeatTests)
#include "tst_write_repeat.moc"
