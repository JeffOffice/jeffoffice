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
#include "core/reviewops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "office/package.h"

#include <QAction>
#include <QMenu>
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
        QCOMPARE(win.w.view()->typedRun(), QStringLiteral("teh "));
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
};

QTEST_MAIN(RepeatTests)
#include "tst_write_repeat.moc"
