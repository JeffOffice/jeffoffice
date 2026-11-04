// The checks every app's window must pass, for JeffWrite (see appchecks.h): its first five minutes, its standard commands, and every command it has.
#include "appchecks.h"

#include "app/filescreen.h"
#include "app/pageview.h"
#include "app/ruler.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "app/stylespane.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "office/package.h"
#include "core/fonts.h"
#include "core/pagesetup.h"
#include "core/commentops.h"
#include "core/editops.h"
#include "core/fieldops.h"
#include "core/indexops.h"
#include "core/mergeops.h"
#include "core/tocops.h"
#include "core/pictureops.h"
#include "core/reviewops.h"
#include "core/rulerops.h"
#include "app/tabledialogs.h"
#include "app/widgets.h"
#include "core/tableops.h"
#include "core/writedoc.h"
#include "text/textprops.h"
#include "testrunner.h"

#include <QApplication>
#include <QBuffer>
#include <QImage>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QScrollBar>
#include <QTest>
#ifdef Q_OS_UNIX
#include <sys/stat.h>
#endif
#include <QCheckBox>
#include <QListWidget>
#include <QLineEdit>
#include <QClipboard>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QTimer>
#include <QCryptographicHash>
#include <QGuiApplication>
#include <QFileInfo>
#include <QLabel>
#include <QStatusBar>
#include <memory>

using namespace jp;
using namespace appchecks;

namespace {

WriteWindow *asWrite(AppWindow *w) { return static_cast<WriteWindow *>(w); }

// A table of three rows and three columns at the cursor, with the cursor in its first cell.
void tableAtCursor(WriteWindow *w)
{
    write::NewTable table;
    table.rows = 3;
    table.columns = 3;
    write::insertTable(*w->document(), w->view()->cursor(), table);
    w->view()->refresh();
    w->refreshNow();
}

WriteWindow *launchBlank()
{
    auto *w = new WriteWindow();
    w->resize(1000, 650);
    w->show();
    return w;
}

}

class WriteAppChecks : public QObject {
    Q_OBJECT

    Adapter m_app;

private Q_SLOTS:
    void initTestCase()
    {
        isolateSettings();
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        initCore();
        m_app.name = QStringLiteral("JeffWrite");
        m_app.launch = [] { return static_cast<AppWindow *>(launchBlank()); };
        m_app.launchWith = [](const QString &path) -> AppWindow * {
            auto *w = launchBlank();
            if (!w->openFile(path)) {
                delete w;
                return nullptr;
            }
            return w;
        };
        m_app.type = [](AppWindow *w) { typeInto(asWrite(w)->view(), QStringLiteral("Some words")); };
        m_app.typedText = [](AppWindow *w) { return asWrite(w)->document() ? asWrite(w)->document()->story()->toPlainText() : QString(); };
        m_app.settle = [](AppWindow *w) {
            asWrite(w)->refreshNow();
            drain();
        };
        m_app.showPrint = [](AppWindow *w) -> QWidget * {
            w->act(QStringLiteral("file.print"))->trigger();
            return w->findChild<FileScreen *>();
        };
        m_app.reopen = [](const QString &path) -> QString {
            const QString ending = QFileInfo(path).suffix().toLower();
            if (ending != QLatin1String("docx") && ending != QLatin1String("dotx")) return {};
            QFile f(path);
            if (!f.open(QIODevice::ReadOnly)) return QStringLiteral("it can't be opened");
            QString err;
            QStringList notes;
            return write::WriteDoc::fromFile(f.readAll(), &err, &notes) ? QString() : err.isEmpty() ? QStringLiteral("it is not read") : err;
        };
        m_app.listen = [](AppWindow *w, Watch *watch) {
            asWrite(w)->setNotifier([watch](const QString &title, const QString &text) { watch->told << title + QLatin1String(": ") + text; });
            // (Save As and the exports ask the save dialog, which the watch stands in for)
        };
        m_app.docState = [](AppWindow *aw) {
            auto *w = asWrite(aw);
            QString s;
            if (w->document()) {
                s += zipDigest(w->document()->toDocx());
                s += QLatin1Char('|') + w->document()->story()->toPlainText();
                s += QLatin1Char('|') + QString::number(w->document()->canUndo()) + QString::number(w->document()->canRedo());
                s += QStringLiteral("|codes%1,%2").arg(w->document()->allFieldCodesShown()).arg(w->document()->shownFieldCodes().size());   // (Show Field Codes changes what the window shows, not the document)
            }
            const QTextCursor c = w->view()->cursor();
            s += QStringLiteral("|%1,%2|%3|%4|%5|%6|%7|%8|%9").arg(c.position()).arg(c.anchor()).arg(w->view()->zoom()).arg(w->isModified()).arg(w->filePath()).arg(int(w->view()->zoomMode())).arg(w->view()->showMarks()).arg(w->view()->spellingEnabled()).arg(w->view()->pagesAcross());
            return s;
        };
        // The sample: a few paragraphs, saved by the program itself.
        {
            std::unique_ptr<WriteWindow> w(launchBlank());
            typeInto(w->view(), QStringLiteral("A sample letter.\nIts second line has more words in it."));
            w->act(QStringLiteral("edit.selectAll"))->trigger();
            w->act(QStringLiteral("fmt.alignCenter"))->trigger();
            w->act(QStringLiteral("para.indentInc"))->trigger();
            w->view()->setZoom(1.0);
            w->setSavePathChooser([](const QString &, const QString &) { return tempFile(QStringLiteral("sample.docx")); });
            QVERIFY(w->saveAs());
        }
        m_app.sampleFile = tempFile(QStringLiteral("sample.docx"));
        QVERIFY(QFileInfo::exists(m_app.sampleFile));
        // Commands that end the program or leave it for another one.
        m_app.skipped = {};
        m_app.prepare = [this](AppWindow *w, const QString &id, bool fromFile) {
            // A zoom command has something to do when the window is not at that zoom.
            if (id == QLatin1String("zoom.100")) asWrite(w)->view()->setZoom(1.5);
            // Save has something to save once something was changed.
            if (id == QLatin1String("file.save") && fromFile) m_app.type(w);
            if (id == QLatin1String("edit.paste")) QGuiApplication::clipboard()->setText(QStringLiteral("Pasted words"));
            // Insert Picture asks for a file, and From Clipboard needs a picture there.
            if (id == QLatin1String("insert.picture")) asWrite(w)->setPictureChooser([] { return pngFile(40, 30); });
            if (id == QLatin1String("insert.pictureClipboard")) {
                QImage image(40, 30, QImage::Format_ARGB32);
                image.fill(QColor(10, 160, 60));
                QGuiApplication::clipboard()->setImage(image);
            }
            // A format is for the words a person has selected.
            if (fromFile && (id.startsWith(QLatin1String("fmt.")) || id.startsWith(QLatin1String("para.")) || id.startsWith(QLatin1String("ls.")) || id.startsWith(QLatin1String("style."))))
                w->act(QStringLiteral("edit.selectAll"))->trigger();
        };
        // The places a person is in when a command is there to use: words selected, something to undo or to redo, a paragraph with an indent, and a header being edited.
        auto typed = [](AppWindow *w, bool fromFile) {
            if (!fromFile) typeInto(asWrite(w)->view(), QStringLiteral("Some words\nand a second line"));
        };
        m_app.contexts = {
            {QStringLiteral("words selected"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 w->act(QStringLiteral("edit.selectAll"))->trigger();
             }, false},
            {QStringLiteral("something to undo"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 typeInto(asWrite(w)->view(), QStringLiteral("more"));
             }, false},
            {QStringLiteral("something undone"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 typeInto(asWrite(w)->view(), QStringLiteral("more"));
                 asWrite(w)->refreshNow();
                 w->act(QStringLiteral("edit.undo"))->trigger();
             }, false},
            {QStringLiteral("a paragraph with an indent"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 w->act(QStringLiteral("edit.selectAll"))->trigger();
                 w->act(QStringLiteral("para.indentInc"))->trigger();
             }, false},
            {QStringLiteral("a centered paragraph"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 w->act(QStringLiteral("edit.selectAll"))->trigger();
                 w->act(QStringLiteral("fmt.alignCenter"))->trigger();
             }, false},
            {QStringLiteral("the header of a second section"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 w->act(QStringLiteral("layout.sectionBreak"))->trigger();
                 typeInto(asWrite(w)->view(), QStringLiteral("In the second section"));
                 asWrite(w)->view()->paginateAll();
                 asWrite(w)->view()->goToPage(asWrite(w)->view()->pageCount() - 1);
                 asWrite(w)->refreshNow();
                 w->act(QStringLiteral("insert.header"))->trigger();
             }, false},
            {QStringLiteral("a header being edited"), [](AppWindow *w, bool) { w->act(QStringLiteral("insert.header"))->trigger(); }, false},
            {QStringLiteral("the Styles pane open"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 w->act(QStringLiteral("style.pane"))->trigger();
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("Copy Formatting on"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 w->act(QStringLiteral("edit.selectAll"))->trigger();
                 w->act(QStringLiteral("edit.formatPainter"))->trigger();
             }, false},
            {QStringLiteral("a format copied"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 w->act(QStringLiteral("edit.selectAll"))->trigger();
                 w->act(QStringLiteral("edit.copyFormat"))->trigger();
             }, false},
            {QStringLiteral("a picture selected"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 asWrite(w)->setPictureChooser([] { return pngFile(80, 60); });
                 w->act(QStringLiteral("insert.picture"))->trigger();
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("comments, the caret in the first"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 write::WriteDoc *doc = asWrite(w)->document();
                 // (The later one first, so that the places of the earlier stay.)
                 QTextCursor c(doc->story());
                 c.setPosition(5);
                 c.setPosition(9, QTextCursor::KeepAnchor);
                 write::addComment(*doc, c, QStringLiteral("A later remark"));
                 c.setPosition(0);
                 c.setPosition(3, QTextCursor::KeepAnchor);
                 write::addComment(*doc, c, QStringLiteral("A remark"));
                 QTextCursor inside(doc->story());
                 inside.setPosition(2);
                 asWrite(w)->view()->setCursor(inside);
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("comments, the caret in the last"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 write::WriteDoc *doc = asWrite(w)->document();
                 QTextCursor c(doc->story());
                 c.setPosition(5);
                 c.setPosition(9, QTextCursor::KeepAnchor);
                 write::addComment(*doc, c, QStringLiteral("A later remark"));
                 c.setPosition(0);
                 c.setPosition(3, QTextCursor::KeepAnchor);
                 write::addComment(*doc, c, QStringLiteral("A remark"));
                 QTextCursor inside(doc->story());
                 inside.setPosition(11);   // (in the words of the later comment: the earlier one's three marks come before it)
                 asWrite(w)->view()->setCursor(inside);
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("changes tracked in the text"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 write::WriteDoc *doc = asWrite(w)->document();
                 doc->setReviewer(QStringLiteral("Checker"), QStringLiteral("C"));
                 doc->setTrackChanges(true);
                 QTextCursor c(doc->story());
                 c.setPosition(0);
                 c.setPosition(3, QTextCursor::KeepAnchor);
                 write::toggleBold(*doc, c);
                 c.setPosition(5);
                 c.setPosition(7, QTextCursor::KeepAnchor);
                 write::removeSelection(*doc, c);
                 c.setPosition(doc->story()->characterCount() - 1);
                 write::insertText(*doc, c, QStringLiteral("added"));
                 doc->setTrackChanges(false);
                 QTextCursor at(doc->story());
                 at.setPosition(1);
                 asWrite(w)->view()->setCursor(at);
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("the caret in a field"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 QTextCursor c(asWrite(w)->document()->story());
                 c.movePosition(QTextCursor::End);
                 write::insertField(*asWrite(w)->document(), c, QStringLiteral("= 2 + 3"));
                 asWrite(w)->view()->setCursor(c);
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("a footnote and an endnote, the caret before them"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 write::WriteDoc *doc = asWrite(w)->document();
                 QTextCursor n(doc->story());
                 n.setPosition(3);
                 write::insertFootnote(*doc, n);
                 asWrite(w)->view()->leaveNote();
                 QTextCursor e(doc->story());
                 e.setPosition(6);
                 write::insertEndnote(*doc, e);
                 asWrite(w)->view()->leaveNote();
                 QTextCursor at(doc->story());
                 at.setPosition(0);
                 asWrite(w)->view()->setCursor(at);
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("a footnote and an endnote, the caret after them"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 write::WriteDoc *doc = asWrite(w)->document();
                 QTextCursor n(doc->story());
                 n.setPosition(3);
                 write::insertFootnote(*doc, n);
                 asWrite(w)->view()->leaveNote();
                 QTextCursor e(doc->story());
                 e.setPosition(6);
                 write::insertEndnote(*doc, e);
                 asWrite(w)->view()->leaveNote();
                 QTextCursor at(doc->story());
                 at.setPosition(doc->story()->characterCount() - 1);
                 asWrite(w)->view()->setCursor(at);
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("the caret in a table of contents"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 write::WriteDoc *doc = asWrite(w)->document();
                 QTextCursor c(doc->story());
                 c.setPosition(0);
                 write::insertTableOfContents(*doc, c, write::TocOptions());
                 asWrite(w)->view()->setCursor(c);
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("the caret in an index"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 write::WriteDoc *doc = asWrite(w)->document();
                 QTextCursor c(doc->story());
                 c.setPosition(0);
                 write::insertIndex(*doc, c, write::IndexOptions());
                 // (An entry marked after the index was made: its update has a change to show.)
                 QTextCursor mark(doc->story());
                 mark.setPosition(1);
                 write::IndexMark im;
                 im.main = QStringLiteral("Entry");
                 write::markIndexEntry(*doc, mark, im);
                 asWrite(w)->view()->setCursor(c);
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("the caret in a table of authorities"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 write::WriteDoc *doc = asWrite(w)->document();
                 QTextCursor c(doc->story());
                 c.setPosition(0);
                 write::insertTableOfAuthorities(*doc, c, write::AuthoritiesOptions());
                 // (A citation marked after the table was made: its update has a change to show.)
                 QTextCursor mark(doc->story());
                 mark.setPosition(1);
                 write::AuthorityMark am;
                 am.longCitation = QStringLiteral("Smith v. Jones");
                 write::markAuthority(*doc, mark, am);
                 asWrite(w)->view()->setCursor(c);
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("a field with a stale result"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 write::WriteDoc *doc = asWrite(w)->document();
                 QTextCursor c(doc->story());
                 c.movePosition(QTextCursor::End);
                 write::insertField(*doc, c, QStringLiteral("NUMWORDS"));
                 QTextCursor start(doc->story());
                 start.setPosition(0);
                 write::insertText(*doc, start, QStringLiteral("a few more words "));
                 asWrite(w)->view()->setCursor(c);
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("the caret in a locked field"), [typed](AppWindow *w, bool f) {
                 typed(w, f);
                 QTextCursor c(asWrite(w)->document()->story());
                 c.movePosition(QTextCursor::End);
                 write::insertField(*asWrite(w)->document(), c, QStringLiteral("= 2 + 3"));
                 write::lockFields(*asWrite(w)->document(), c, true);
                 asWrite(w)->view()->setCursor(c);
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("the cursor in a table"), [](AppWindow *w, bool) { tableAtCursor(asWrite(w)); }, false},
            {QStringLiteral("the cursor in the second row of a table"), [](AppWindow *w, bool) {
                 tableAtCursor(asWrite(w));
                 for (int i = 0; i < 8 && !write::canSplitTable(*asWrite(w)->document(), asWrite(w)->view()->cursor()); ++i) write::tabThroughCells(*asWrite(w)->document(), asWrite(w)->view()->cursor(), false);
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("the whole table selected"), [](AppWindow *w, bool) {
                 tableAtCursor(asWrite(w));
                 write::selectInTable(*asWrite(w)->document(), asWrite(w)->view()->cursor(), write::TableUnit::Table);
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("a table with a wide column"), [](AppWindow *w, bool) {
                 tableAtCursor(asWrite(w));
                 write::setColumnWidth(*asWrite(w)->document(), asWrite(w)->view()->cursor(), 300);
                 asWrite(w)->view()->refresh();
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("a merge with a list of recipients"), [](AppWindow *w, bool) {
                 // (the list is put in place as Select Recipients puts it, without its file dialog)
                 write::mail::State &state = write::mail::stateOf(*asWrite(w)->document());
                 state.settings.type = write::mail::kindValue(write::mail::Kind::Letters);
                 MergeSource m;
                 m.fields = {QStringLiteral("First Name"), QStringLiteral("Last Name"), QStringLiteral("City")};
                 m.rows << QStringList{QStringLiteral("Mara"), QStringLiteral("Fenwick"), QStringLiteral("Arlington")} << QStringList{QStringLiteral("Nora"), QStringLiteral("Pell"), QStringLiteral("Dover")};
                 m.include << true << true;
                 state.data = m;
                 state.loaded = true;
                 asWrite(w)->refreshNow();
             }, false},
            {QStringLiteral("a merged cell"), [](AppWindow *w, bool) {
                 tableAtCursor(asWrite(w));
                 write::selectInTable(*asWrite(w)->document(), asWrite(w)->view()->cursor(), write::TableUnit::Row);
                 write::mergeCells(*asWrite(w)->document(), asWrite(w)->view()->cursor());
                 asWrite(w)->view()->refresh();
                 asWrite(w)->refreshNow();
             }, false},
        };
        m_app.quiet = {
            {QStringLiteral("mail.highlight"), QStringLiteral("it chooses whether merge fields are shaded on the screen, which shows only where the document has them; the mail tests check the pixels")},
            {QStringLiteral("mail.check"), QStringLiteral("it only reports on the merge (here: no errors are found); the mail tests check what it says")},
            {QStringLiteral("picture.lockAspect"), QStringLiteral("it is the setting the next change of a picture's width or height reads, and changes nothing by itself")},
            {QStringLiteral("edit.copyFormat"), QStringLiteral("it only holds the look of the selection for Paste Format; the paste is the change")},
            {QStringLiteral("edit.pasteFormat"), QStringLiteral("it puts the look held on the selection, which here has that look already, or nothing is held yet")},
            {QStringLiteral("edit.formatPainter"), QStringLiteral("it picks up the look of the selection and waits for the next selection; the paint is the change, and the window's tests check it")},
            {QStringLiteral("view.printLayout"), QStringLiteral("the print layout is the only layout JeffWrite has, and the window begins in it")},
            {QStringLiteral("review.show.comments"), QStringLiteral("it chooses whether the balloons for comments are painted in the pages' margin, which no control shows; the window's tests check the painting")},
            {QStringLiteral("table.gridlines"), QStringLiteral("it draws the edges of the cells that have no line, and the tables the checks make have lines; the window's tests check the drawing")},
            {QStringLiteral("review.show.formatting"), QStringLiteral("it chooses whether the bars for changes of formatting are painted in the pages' margin, which no control shows; the window's tests check the painting")},
        };
        m_app.neverRun = {
            {QStringLiteral("mail.updateLabels"), QStringLiteral("it is for a sheet of labels that Mailings > Labels makes, which no place here has; the mail tests run it on one")},
        };
    }

    // ---- 1. the first five minutes
    // A new window, and one from File > New, take typing without a click first.
    void typingNeedsNoClick()
    {
        const QStringList failed = checkTypingWithoutAClick(m_app);
        QVERIFY2(failed.isEmpty(), qPrintable(failed.join(QLatin1Char('\n'))));
    }

    void firstFiveMinutes()
    {
        Watch watch;
        std::unique_ptr<WriteWindow> w(launchBlank());
        QStringList told;
        w->setNotifier([&](const QString &title, const QString &text) { told << title + QLatin1String(": ") + text; });
        // A fresh document: its name in the title, nothing to save.
        QVERIFY2(w->windowTitle().startsWith(QLatin1String("Document")), qPrintable(w->windowTitle()));
        QVERIFY(w->windowTitle().contains(QLatin1String("JeffWrite")));
        QVERIFY(!w->isModified());

        // Typing goes into the document, and shows in the title.
        typeInto(w->view(), QStringLiteral("Hello there"));
        QCOMPARE(w->document()->story()->toPlainText(), QStringLiteral("Hello there"));
        QVERIFY(w->isModified());
        QVERIFY2(w->windowTitle().contains(QLatin1Char('*')), qPrintable(w->windowTitle()));
        QVERIFY2(!saysNotYet(w->statusBar()->currentMessage()), qPrintable(w->statusBar()->currentMessage()));

        // Undo takes it back, Redo puts it again.
        QAction *undo = w->act(QStringLiteral("edit.undo")), *redo = w->act(QStringLiteral("edit.redo"));
        QVERIFY(waitFor([&] { return undo->isEnabled(); }));
        for (int i = 0; i < 40 && undo->isEnabled(); ++i) undo->trigger();
        QCOMPARE(w->document()->story()->toPlainText(), QString());
        QVERIFY(waitFor([&] { return redo->isEnabled(); }));
        for (int i = 0; i < 40 && redo->isEnabled(); ++i) redo->trigger();
        QCOMPARE(w->document()->story()->toPlainText(), QStringLiteral("Hello there"));

        // Save As in the program's own format; the window takes the file's name.
        const QString path = tempFile(QStringLiteral("five.docx"));
        QFile::remove(path);
        w->setSavePathChooser([&](const QString &, const QString &) { return path; });
        w->act(QStringLiteral("file.saveAs"))->trigger();
        QVERIFY(QFileInfo::exists(path));
        QVERIFY(!w->isModified());
        QVERIFY2(w->windowTitle().startsWith(QLatin1String("five")), qPrintable(w->windowTitle()));
        QVERIFY2(!w->windowTitle().contains(QLatin1Char('*')), qPrintable(w->windowTitle()));
        QVERIFY2(told.isEmpty(), qPrintable(told.join('\n')));

        // The saved file is among the recent ones, on the File screen's Open page.
        QVERIFY(Settings::get().recentFiles().contains(path));
        w->showFileScreen(QStringLiteral("open"));
        QVERIFY(listsRecentFile(w->findChild<FileScreen *>(), path));
        w->hideFileScreen();

        // Closed (nothing changed since the save: no question), and opened again: the same text.
        const CloseResult closed = closeWindow(w.get(), watch);
        QVERIFY(closed.closed);
        QCOMPARE(closed.asked, 0);
        w.reset();
        std::unique_ptr<WriteWindow> again(static_cast<WriteWindow *>(m_app.launchWith(path)));
        QVERIFY(again);
        QCOMPARE(again->document()->story()->toPlainText(), QStringLiteral("Hello there"));
        QVERIFY(!again->isModified());
        QCOMPARE(again->filePath(), path);

        // File > New: a second window, with a blank document that takes typing.
        const int windows = AppWindow::windows().size();
        again->act(QStringLiteral("file.new"))->trigger();
        drain();
        QCOMPARE(AppWindow::windows().size(), windows + 1);
        WriteWindow *second = nullptr;
        for (AppWindow *other : AppWindow::windows())
            if (other != again.get()) second = asWrite(other);
        QVERIFY(second);
        QPointer<WriteWindow> guard(second);
        QVERIFY(second->windowTitle().startsWith(QLatin1String("Document")));
        typeInto(second->view(), QStringLiteral("Second"));
        QCOMPARE(second->document()->story()->toPlainText(), QStringLiteral("Second"));
        // (a changed document asks before it goes; here it is asked, and the answer is no)
        const CloseResult asked = closeWindow(second, watch);
        QCOMPARE(asked.asked, 1);
        QVERIFY(!asked.closed);
        second->setAttribute(Qt::WA_DeleteOnClose, false);
        delete second;

        // The print layout: the File screen's Print page, with the document's page drawn (not a blank one).
        QVERIFY2(previewShowsTheDocument(m_app, again.get()), "the print preview shows no more than a blank page");
        auto *screen = again->findChild<FileScreen *>();
        QVERIFY(screen);
        QCOMPARE(screen->currentPage(), QStringLiteral("print"));
        again->hideFileScreen();

        // An unchanged document goes without a question.
        const CloseResult plain = closeWindow(again.get(), watch);
        QVERIFY(plain.closed);
        QCOMPARE(plain.asked, 0);
        QVERIFY2(watch.warnings.isEmpty(), qPrintable(watch.warnings.join('\n')));
    }

    // The way these checks see a window does not change by itself: a call that does nothing is seen to do nothing (or every command would pass).
    void aCallThatDoesNothingChangesNothing()
    {
        Watch watch;
        for (bool fromFile : {false, true}) {
            std::unique_ptr<AppWindow> w(fromFile ? m_app.launchWith(m_app.sampleFile) : m_app.launch());
            QVERIFY(w);
            m_app.settle(w.get());
            const Outcome o = observe(m_app, w.get(), nullptr, watch, [] {}, false, 3000);
            QVERIFY2(!o.changed, qPrintable(o.how + o.detail));
        }
    }

    // ---- 2. the standard commands
    void standardCommands()
    {
        const QStringList fails = checkStandardCommands(m_app);
        QVERIFY2(fails.isEmpty(), qPrintable(fails.join('\n')));
    }

    // ---- tables (beta wave B)
    // A table made in the window shows the Table Design and Table Layout tabs and turns its commands on; the commands change the table, undo, and the file saved opens the same.
    void tableCommandsOfTheWindowChangeTheTableUndoAndSaveAndOpenTheSame()
    {
        std::unique_ptr<WriteWindow> w(launchBlank());
        typeInto(w->view(), QStringLiteral("Intro"));
        w->refreshNow();
        auto tabShown = [&](const char *name) {
            for (int i = 0; i < w->ribbon()->tabCount(); ++i)
                if (w->ribbon()->tabName(i) == QLatin1String(name)) return w->ribbon()->tabVisible(i);
            return false;
        };
        auto cells = [&] {
            int n = 0;
            for (QTextBlock b = w->document()->story()->begin(); b.isValid(); b = b.next())
                if (write::inTable(b)) ++n;
            return n;
        };
        QVERIFY(w->act(QStringLiteral("insert.table")));
        QVERIFY(!tabShown("Table Layout") && !tabShown("Table Design"));
        QVERIFY(!w->act(QStringLiteral("table.merge"))->isEnabled() && !w->act(QStringLiteral("table.properties"))->isEnabled());
        tableAtCursor(w.get());
        QCOMPARE(cells(), 9);
        QVERIFY(tabShown("Table Layout") && tabShown("Table Design"));
        for (const char *id : {"table.insertBelow", "table.properties", "table.split", "table.bordersShading", "table.align.mc"}) QVERIFY2(w->act(QString::fromLatin1(id))->isEnabled(), id);
        w->act(QStringLiteral("table.insertBelow"))->trigger();
        QCOMPARE(cells(), 12);
        w->act(QStringLiteral("table.selectTable"))->trigger();
        w->refreshNow();
        QVERIFY(w->act(QStringLiteral("table.merge"))->isEnabled());
        w->act(QStringLiteral("table.merge"))->trigger();
        QCOMPARE(cells(), 4);   // (the first row's cell, and an empty paragraph in each row under it, which the join goes through)
        w->act(QStringLiteral("table.border.all"))->trigger();
        w->act(QStringLiteral("table.align.br"))->trigger();
        const QString merged = zipDigest(w->document()->toDocx());
        for (int i = 0; i < 3; ++i) {
            w->refreshNow();
            w->act(QStringLiteral("edit.undo"))->trigger();
        }
        QCOMPARE(cells(), 12);
        for (int i = 0; i < 3; ++i) {
            w->refreshNow();
            w->act(QStringLiteral("edit.redo"))->trigger();
        }
        QCOMPARE(cells(), 4);
        QCOMPARE(zipDigest(w->document()->toDocx()), merged);
        // Saved, and opened again: the same.
        const QString path = tempFile(QStringLiteral("table.docx"));
        QFile::remove(path);
        w->setSavePathChooser([&](const QString &, const QString &) { return path; });
        QVERIFY(w->saveAs());
        std::unique_ptr<WriteWindow> again(static_cast<WriteWindow *>(m_app.launchWith(path)));
        QVERIFY(again);
        QCOMPARE(again->document()->story()->toPlainText(), w->document()->story()->toPlainText());
        {
            // (the parts of the two files are the same)
            office::Package a, b;
            a.read(again->document()->toDocx());
            b.read(w->document()->toDocx());
            QCOMPARE(a.partNames(), b.partNames());
            for (const QString &name : a.partNames()) QVERIFY2(a.part(name) == b.part(name), qPrintable(name));
        }
        // Out of the table, the tabs go.
        w->view()->cursor().setPosition(0);
        w->refreshNow();
        QVERIFY(!tabShown("Table Layout") && !w->act(QStringLiteral("table.merge"))->isEnabled());
    }

    // The table dialogs ask for what they are for, give it back, and remember what they were told to: a dialog is answered from the event loop it runs in, by the names of its fields.
    static void answer(const std::function<void(QDialog *)> &fill, bool accept = true)
    {
        QTimer::singleShot(0, qApp, [fill, accept] {
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!dlg) return;
            fill(dlg);
            if (accept) dlg->accept();
            else dlg->reject();
        });
    }

    void theInsertTableAndSplitCellsDialogsGiveWhatWasAskedFor()
    {
        write::NewTable table;
        answer([](QDialog *d) {
            d->findChild<QSpinBox *>(QStringLiteral("columns"))->setValue(5);
            d->findChild<QSpinBox *>(QStringLiteral("rows"))->setValue(4);
            d->findChild<QAbstractButton *>(QStringLiteral("fitWindow"))->setChecked(true);
            d->findChild<QCheckBox *>(QStringLiteral("remember"))->setChecked(true);
        });
        QVERIFY(write::insertTableDialog(nullptr, &table));
        QCOMPARE(table.columns, 5);
        QCOMPARE(table.rows, 4);
        QVERIFY(table.fit == write::TableFit::Window);
        // (remembered: the next one begins with them)
        write::NewTable next;
        answer([](QDialog *d) {
            QCOMPARE(d->findChild<QSpinBox *>(QStringLiteral("columns"))->value(), 5);
            QCOMPARE(d->findChild<QSpinBox *>(QStringLiteral("rows"))->value(), 4);
            QVERIFY(d->findChild<QAbstractButton *>(QStringLiteral("fitWindow"))->isChecked());
        }, false);
        QVERIFY(!write::insertTableDialog(nullptr, &next));
        int columns = 2, rows = 1;
        answer([](QDialog *d) {
            d->findChild<QSpinBox *>(QStringLiteral("columns"))->setValue(3);
            d->findChild<QSpinBox *>(QStringLiteral("rows"))->setValue(2);
        });
        QVERIFY(write::splitCellsDialog(nullptr, &columns, &rows, 50));
        QCOMPARE(columns, 3);
        QCOMPARE(rows, 2);
    }

    void theTablePropertiesAndBordersDialogsChangeTheTableInOneStep()
    {
        std::unique_ptr<WriteWindow> w(launchBlank());
        tableAtCursor(w.get());
        write::WriteDoc &doc = *w->document();
        QTextCursor &c = w->view()->cursor();
        const QByteArray before = doc.toDocx();
        answer([](QDialog *d) {
            d->findChild<QComboBox *>(QStringLiteral("alignment"))->setCurrentIndex(1);
            d->findChild<QCheckBox *>(QStringLiteral("headerRow"))->setChecked(true);
            d->findChild<QCheckBox *>(QStringLiteral("cantSplit"))->setChecked(true);
            d->findChild<QComboBox *>(QStringLiteral("verticalAlignment"))->setCurrentIndex(1);
        });
        QVERIFY(write::tablePropertiesDialog(nullptr, &doc, &c));
        const write::TableProps props = write::tablePropsAt(doc, c);
        QVERIFY(props.align == flow::TableData::Align::Center && props.header && props.cantSplit && props.valign == flow::CellAlign::Center);
        // Cancelled, nothing changes.
        const QByteArray now = doc.toDocx();
        answer([](QDialog *d) { d->findChild<QCheckBox *>(QStringLiteral("headerRow"))->setChecked(false); }, false);
        QVERIFY(!write::tablePropertiesDialog(nullptr, &doc, &c));
        QCOMPARE(doc.toDocx(), now);
        QVERIFY(now != before);
        // Borders and shading for the whole table: double lines, and a fill.
        answer([](QDialog *d) {
            d->findChild<QComboBox *>(QStringLiteral("lineStyle"))->setCurrentIndex(2);
            d->findChild<QComboBox *>(QStringLiteral("applyTo"))->setCurrentIndex(1);
            d->findChild<QCheckBox *>(QStringLiteral("insideH"))->setChecked(true);
            d->findChild<QCheckBox *>(QStringLiteral("insideV"))->setChecked(true);
        });
        QVERIFY(write::bordersShadingDialog(nullptr, &doc, &c));
        const flow::CellRecord *rec = doc.flow().tables.get(write::cellKeyOf(c.block()));
        QVERIFY(rec->cell.top && rec->cell.top->style == flow::TableLine::Style::Double && rec->cell.right && rec->cell.right->style == flow::TableLine::Style::Double);
        QVERIFY(doc.toDocx().size() > 0);
    }

    // The height and width fields of the Table Layout tab run a command only when the value was changed: a person who leaves the field as it was makes no step to undo and no change to save.
    void theSizeFieldsChangeNothingWhenTheValueIsTheSame()
    {
        std::unique_ptr<WriteWindow> w(launchBlank());
        tableAtCursor(w.get());
        write::setRowHeight(*w->document(), w->view()->cursor(), 30, false);
        w->view()->refresh();
        w->refreshNow();
        for (int i = 0; i < w->ribbon()->tabCount(); ++i)
            if (w->ribbon()->tabName(i) == QLatin1String("Table Layout")) w->ribbon()->showTab(w->ribbon()->tabPage(i));
        w->refreshNow();
        auto *height = w->findChild<MeasureSpin *>(QStringLiteral("tableRowHeightField"));
        auto *width = w->findChild<MeasureSpin *>(QStringLiteral("tableColumnWidthField"));
        QVERIFY(height && width);
        QVERIFY(qFuzzyCompare(height->value(), 30.0));
        w->document()->setModified(false);
        const QByteArray was = w->document()->toDocx();
        int steps = 0;
        while (w->document()->canUndo()) {
            w->document()->undo();
            ++steps;
        }
        for (int i = 0; i < steps; ++i) w->document()->redo();
        auto inTheTable = [&] {
            for (QTextBlock b = w->document()->story()->begin(); b.isValid(); b = b.next())
                if (write::inTable(b)) {
                    w->view()->cursor().setPosition(b.position());
                    break;
                }
        };
        inTheTable();
        Q_EMIT height->editingFinished();
        Q_EMIT width->editingFinished();
        QVERIFY(!w->document()->isModified());
        QCOMPARE(w->document()->toDocx(), was);
        int after = 0;
        while (w->document()->canUndo()) {
            w->document()->undo();
            ++after;
        }
        QCOMPARE(after, steps);
        for (int i = 0; i < after; ++i) w->document()->redo();
        inTheTable();
        // A changed value is a step.
        height->setValue(44);
        Q_EMIT height->editingFinished();
        QVERIFY(w->document()->isModified());
        QVERIFY(w->document()->toDocx() != was);
    }

    // ---- pictures, page setup, and the ruler (beta wave C)

    static QByteArray pngOf(int width, int height)
    {
        QImage image(width, height, QImage::Format_RGB32);
        image.fill(QColor(30, 120, 200));
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        return bytes;
    }

    static QString pngFile(int width, int height)
    {
        const QString path = tempFile(QStringLiteral("pic%1x%2.png").arg(width).arg(height));
        QFile f(path);
        if (f.open(QIODevice::WriteOnly)) f.write(pngOf(width, height));
        return path;
    }

    static bool tabShown(WriteWindow *w, const char *name)
    {
        for (int i = 0; i < w->ribbon()->tabCount(); ++i)
            if (w->ribbon()->tabName(i) == QLatin1String(name)) return w->ribbon()->tabVisible(i);
        return false;
    }

    static void settle(WriteWindow *w)
    {
        w->view()->paginateAll();
        w->view()->refresh();
        w->refreshNow();
    }

    static void drag(QWidget *target, QPoint from, QPoint to, int steps = 3)
    {
        QTest::mousePress(target, Qt::LeftButton, Qt::NoModifier, from);
        for (int i = 1; i <= steps; ++i) QTest::mouseMove(target, from + (to - from) * i / steps);
        QTest::mouseRelease(target, Qt::LeftButton, Qt::NoModifier, to);
    }

    // The Insert Picture command puts the file's picture in the text and selects it; the Picture Format tab, its Wrap Text choices, its size fields, and its handles change it in one step each;
    // and the file saved opens the same.
    void aPictureInsertedInTheWindowIsSelectedWrappedSizedAndMovedInOneStepEach()
    {
        std::unique_ptr<WriteWindow> w(launchBlank());
        typeInto(w->view(), QString(QStringLiteral("Some words that a picture will sit among, enough of them to fill several lines of the page. ")).repeated(8));
        for (const char *id : {"insert.picture", "insert.pictureClipboard", "picture.altText", "picture.wrap.square", "picture.lockAspect", "view.ruler", "layout.pageSetup"}) QVERIFY2(w->act(QString::fromLatin1(id)), id);
        settle(w.get());
        QVERIFY(!tabShown(w.get(), "Picture Format"));
        QVERIFY(!w->act(QStringLiteral("picture.altText"))->isEnabled());
        const QString file = pngFile(160, 160);
        w->setPictureChooser([file] { return file; });
        w->view()->cursor().setPosition(20);
        w->act(QStringLiteral("insert.picture"))->trigger();
        settle(w.get());
        const int at = w->view()->selectedPicture();
        QCOMPARE(at, 20);
        QVERIFY(tabShown(w.get(), "Picture Format"));
        QVERIFY(w->act(QStringLiteral("picture.wrap.inline"))->isChecked());
        QVERIFY(w->act(QStringLiteral("picture.altText"))->isEnabled());
        auto *height = w->findChild<MeasureSpin *>(QStringLiteral("pictureHeightField"));
        auto *width = w->findChild<MeasureSpin *>(QStringLiteral("pictureWidthField"));
        QVERIFY(height && width);
        QVERIFY(qFuzzyCompare(width->value(), 120.0) && qFuzzyCompare(height->value(), 120.0));
        const write::WriteDoc &doc = *w->document();
        auto info = [&] { return write::pictureAt(doc, doc.story(), at); };
        // Wrap Text: the picture is set at a place, with the text going round it; one Undo takes it back in line.
        const QString inLine = zipDigest(w->document()->toDocx());
        w->act(QStringLiteral("picture.wrap.square"))->trigger();
        settle(w.get());
        QVERIFY(info().floating && info().wrap == write::PictureWrap::Square);
        QVERIFY(w->act(QStringLiteral("picture.wrap.square"))->isChecked());
        QVERIFY(w->view()->selectedPicture() == at);
        w->act(QStringLiteral("edit.undo"))->trigger();
        settle(w.get());
        QCOMPARE(zipDigest(w->document()->toDocx()), inLine);
        QVERIFY(w->act(QStringLiteral("picture.wrap.inline"))->isChecked());
        w->act(QStringLiteral("edit.redo"))->trigger();
        settle(w.get());
        // The size fields keep the picture's proportions (Lock Aspect Ratio is on).
        width->setValue(60);
        Q_EMIT width->editingFinished();
        QCOMPARE(info().width, 60.0);
        QCOMPARE(info().height, 60.0);
        w->act(QStringLiteral("edit.undo"))->trigger();
        settle(w.get());
        QCOMPARE(info().width, 120.0);
        w->act(QStringLiteral("picture.lockAspect"))->trigger();
        height->setValue(30);
        Q_EMIT height->editingFinished();
        QCOMPARE(info().width, 120.0);
        QCOMPARE(info().height, 30.0);
        w->act(QStringLiteral("edit.undo"))->trigger();
        w->act(QStringLiteral("picture.lockAspect"))->trigger();
        settle(w.get());
        // The handles: dragging the bottom right corner makes it larger in the same proportions, in one step.
        const QString beforeDrag = zipDigest(w->document()->toDocx());
        QPointF corner;
        QVERIFY(w->view()->pictureHandlePoint(write::PageView::PictureHandle::BottomRight, &corner));
        QWidget *viewport = w->view()->viewport();
        QCOMPARE(w->view()->pictureHandleAt(corner.toPoint()), write::PageView::PictureHandle::BottomRight);
        const double scale = w->view()->zoom() * 96.0 / 72.0;
        drag(viewport, corner.toPoint(), corner.toPoint() + QPoint(int(60 * scale), int(10 * scale)));
        settle(w.get());
        QVERIFY2(std::abs(info().width - 180) < 1.5 && std::abs(info().width - info().height) < 0.5, qPrintable(QString::number(info().width)));
        QVERIFY(w->view()->selectedPicture() == at);
        w->act(QStringLiteral("edit.undo"))->trigger();
        settle(w.get());
        QCOMPARE(zipDigest(w->document()->toDocx()), beforeDrag);
        w->act(QStringLiteral("edit.redo"))->trigger();
        settle(w.get());
        // A side handle changes one side only.
        QPointF side;
        QVERIFY(w->view()->pictureHandlePoint(write::PageView::PictureHandle::Right, &side));
        drag(viewport, side.toPoint(), side.toPoint() - QPoint(int(90 * scale), 0));
        settle(w.get());
        QVERIFY2(std::abs(info().width - 90) < 1.5 && std::abs(info().height - 180) < 1.5, qPrintable(QString::number(info().width)));
        // Alt text: a dialog, one step.
        answer([](QDialog *d) { d->findChild<QPlainTextEdit *>(QStringLiteral("altText"))->setPlainText(QStringLiteral("A blue square")); });
        w->act(QStringLiteral("picture.altText"))->trigger();
        QCOMPARE(info().alt, QStringLiteral("A blue square"));
        // Moved: a picture in the line goes to the place in the text it is dropped at, in one step.
        w->act(QStringLiteral("picture.wrap.inline"))->trigger();
        settle(w.get());
        QVERIFY(!info().floating);
        const QString beforeMove = zipDigest(w->document()->toDocx());
        QRectF pic;
        QVERIFY(w->view()->pictureRect(at, &pic));
        w->view()->cursor().setPosition(60);
        QRect caret;
        QVERIFY(w->view()->caretRect(&caret));
        w->view()->selectPicture(at);
        QVERIFY(w->view()->pictureRect(at, &pic));
        drag(viewport, pic.center().toPoint(), QPoint(caret.x(), caret.center().y()), 4);
        settle(w.get());
        const int now = w->view()->selectedPicture();
        QVERIFY2(now > at, qPrintable(QString::number(now)));
        w->act(QStringLiteral("edit.undo"))->trigger();
        settle(w.get());
        QCOMPARE(zipDigest(w->document()->toDocx()), beforeMove);
        // A picture set at a place moves to the place it is dropped at.
        w->view()->selectPicture(at);
        settle(w.get());
        w->act(QStringLiteral("picture.wrap.square"))->trigger();
        settle(w.get());
        QVERIFY(w->view()->pictureRect(at, &pic));
        const QRectF startRect = pic;
        const QString beforePlace = zipDigest(w->document()->toDocx());
        drag(viewport, pic.center().toPoint(), pic.center().toPoint() + QPoint(int(100 * scale), int(80 * scale)));
        settle(w.get());
        QRectF moved;
        QVERIFY(w->view()->pictureRect(at, &moved));
        QVERIFY2(std::abs(moved.left() - startRect.left() - 100 * scale) < 3 && std::abs(moved.top() - startRect.top() - 80 * scale) < 3,
                 qPrintable(QStringLiteral("%1,%2 from %3,%4").arg(moved.left()).arg(moved.top()).arg(startRect.left()).arg(startRect.top())));
        QVERIFY(info().placeIsOffset);
        w->act(QStringLiteral("edit.undo"))->trigger();
        settle(w.get());
        QCOMPARE(zipDigest(w->document()->toDocx()), beforePlace);
        w->act(QStringLiteral("edit.redo"))->trigger();
        settle(w.get());
        // A click on the picture selects it, and a click in the text takes the selection away; the tab goes with it.
        w->view()->cursor().setPosition(3);
        settle(w.get());
        QVERIFY(w->view()->selectedPicture() < 0);
        QVERIFY(!tabShown(w.get(), "Picture Format"));
        QVERIFY(w->view()->pictureRect(at, &pic));
        QTest::mouseClick(viewport, Qt::LeftButton, Qt::NoModifier, pic.center().toPoint());
        settle(w.get());
        QCOMPARE(w->view()->selectedPicture(), at);
        QVERIFY(tabShown(w.get(), "Picture Format"));
        // Saved, and opened again: the picture, its wrap, size, place, and alt text.
        const QString path = tempFile(QStringLiteral("picture-window.docx"));
        QFile::remove(path);
        w->setSavePathChooser([&](const QString &, const QString &) { return path; });
        QVERIFY(w->saveAs());
        std::unique_ptr<WriteWindow> again(static_cast<WriteWindow *>(m_app.launchWith(path)));
        QVERIFY(again);
        const write::PictureInfo kept = write::pictureAt(*again->document(), again->document()->story(), at);
        QVERIFY(kept.valid && kept.floating);
        QCOMPARE(kept.alt, QStringLiteral("A blue square"));
        QCOMPARE(kept.width, info().width);
        QCOMPARE(kept.left, info().left);
        QCOMPARE(again->document()->story()->toPlainText(), w->document()->story()->toPlainText());
    }

    // A picture on the clipboard is pasted as a picture, and survives save and reopen; Insert > Pictures > From Clipboard does the same; with no picture there it says so.
    void aPictureFromTheClipboardSurvivesSaveAndReopen()
    {
        std::unique_ptr<WriteWindow> w(launchBlank());
        typeInto(w->view(), QStringLiteral("Before the picture: "));
        QImage image(64, 32, QImage::Format_ARGB32);
        image.fill(QColor(220, 40, 40));
        QGuiApplication::clipboard()->setImage(image);
        w->act(QStringLiteral("edit.paste"))->trigger();
        settle(w.get());
        const int at = 20;
        QCOMPARE(w->view()->selectedPicture(), at);
        QCOMPARE(write::pictureAt(*w->document(), w->document()->story(), at).width, 48.0);
        // One Undo takes it away.
        settle(w.get());
        w->act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(w->view()->selectedPicture() < 0);
        QCOMPARE(w->document()->story()->toPlainText(), QStringLiteral("Before the picture: "));
        settle(w.get());
        w->act(QStringLiteral("edit.redo"))->trigger();
        settle(w.get());
        // The menu's command.
        w->view()->cursor().setPosition(0);
        w->act(QStringLiteral("insert.pictureClipboard"))->trigger();
        QCOMPARE(w->view()->selectedPicture(), 0);
        // Saved, opened again: the pictures are there and are the pixels that were on the clipboard.
        const QString path = tempFile(QStringLiteral("clipboard-picture.docx"));
        QFile::remove(path);
        w->setSavePathChooser([&](const QString &, const QString &) { return path; });
        QVERIFY(w->saveAs());
        std::unique_ptr<WriteWindow> again(static_cast<WriteWindow *>(m_app.launchWith(path)));
        QVERIFY(again);
        QVERIFY(write::isPictureAt(again->document()->story(), 0));
        QVERIFY(write::isPictureAt(again->document()->story(), 21));
        office::Package pkg;
        QVERIFY(pkg.read(again->document()->toDocx()));
        QImage kept;
        QVERIFY(kept.loadFromData(pkg.part(QStringLiteral("/word/media/image1.png"))));
        QCOMPARE(kept.size(), image.size());
        QCOMPARE(kept.pixelColor(10, 10), QColor(220, 40, 40));
        // No picture on the clipboard: it says so, and nothing changes.
        QGuiApplication::clipboard()->setText(QStringLiteral("only words"));
        const QString digest = zipDigest(again->document()->toDocx());
        again->act(QStringLiteral("insert.pictureClipboard"))->trigger();
        QCOMPARE(zipDigest(again->document()->toDocx()), digest);
        QVERIFY(again->statusBar()->currentMessage().contains(QStringLiteral("no picture")));
    }

    // The Page Setup dialog: the margins, orientation, paper, and layout of the section or of every section, in one step.
    void thePageSetupDialogChangesTheSectionOrTheWholeDocumentInOneStep()
    {
        std::unique_ptr<WriteWindow> w(launchBlank());
        typeInto(w->view(), QStringLiteral("First section"));
        w->act(QStringLiteral("layout.sectionBreak"))->trigger();
        typeInto(w->view(), QStringLiteral("Second section"));
        settle(w.get());
        write::WriteDoc &doc = *w->document();
        QCOMPARE(doc.sectionCount(), 2);
        const QString original = zipDigest(doc.toDocx());
        QVERIFY(w->view()->cursorSection() == 1);
        // The second section only.
        answer([](QDialog *d) {
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("applyTo"))->currentIndex(), 0);
            d->findChild<QRadioButton *>(QStringLiteral("landscape"))->setChecked(true);   // (the page turns, and its margins with it)
            // (the page of the region's paper, turned)
            QVERIFY(d->findChild<MeasureSpin *>(QStringLiteral("paperWidth"))->value() > d->findChild<MeasureSpin *>(QStringLiteral("paperHeight"))->value());
            d->findChild<MeasureSpin *>(QStringLiteral("left"))->setValue(36);
            d->findChild<MeasureSpin *>(QStringLiteral("top"))->setValue(54);
        });
        w->act(QStringLiteral("layout.pageSetup"))->trigger();
        settle(w.get());
        const flow::Section second = doc.flow().section(1);
        QVERIFY(second.orientation == flow::Section::Orientation::Landscape);
        QVERIFY(doc.flow().section(0).orientation == flow::Section::Orientation::Portrait);
        QCOMPARE(second.margins.left(), 36.0);
        QCOMPARE(second.margins.top(), 54.0);
        w->act(QStringLiteral("edit.undo"))->trigger();
        settle(w.get());
        QCOMPARE(zipDigest(doc.toDocx()), original);
        // The whole document: A4, a different first page and odd and even pages, a continuous start.
        answer([](QDialog *d) {
            d->findChild<QComboBox *>(QStringLiteral("applyTo"))->setCurrentIndex(1);
            auto *size = d->findChild<QComboBox *>(QStringLiteral("paperSize"));
            size->setCurrentIndex(4);
            Q_EMIT size->activated(4);
            d->findChild<MeasureSpin *>(QStringLiteral("right"))->setValue(90);
            d->findChild<QComboBox *>(QStringLiteral("sectionStart"))->setCurrentIndex(1);
            d->findChild<QCheckBox *>(QStringLiteral("differentFirstPage"))->setChecked(true);
            d->findChild<QCheckBox *>(QStringLiteral("differentOddEven"))->setChecked(true);
        });
        w->act(QStringLiteral("layout.pageSetup"))->trigger();
        settle(w.get());
        for (int i = 0; i < 2; ++i) {
            const write::SectionSetup s = write::pageSetupOf(doc.sectionProps(i), doc.differentOddEven());
            QVERIFY(std::abs(s.paperWidth - 595.3) < 0.1 && std::abs(s.paperHeight - 841.9) < 0.1);
            QCOMPARE(s.right, 90.0);
            QVERIFY(s.start == QLatin1String("continuous") && s.differentFirstPage && s.differentOddEven);
        }
        w->act(QStringLiteral("edit.undo"))->trigger();
        settle(w.get());
        QCOMPARE(zipDigest(doc.toDocx()), original);
        QVERIFY(!doc.differentOddEven());
        // Cancelled: nothing changes.
        answer([](QDialog *d) { d->findChild<MeasureSpin *>(QStringLiteral("left"))->setValue(200); }, false);
        w->act(QStringLiteral("layout.pageSetup"))->trigger();
        QCOMPARE(zipDigest(doc.toDocx()), original);
    }

    // The ruler shows the margins, the indents of the paragraph the caret is in, and its tab stops; each marker is dragged, a click adds a tab stop, and each change is one step.
    void theRulerDragsIndentsMarginsAndTabsInOneStepEach()
    {
        Settings::get().setUnit(Unit::Inch);   // (the markers snap to the marks of the unit the person measures in)
        std::unique_ptr<WriteWindow> w(launchBlank());
        typeInto(w->view(), QStringLiteral("An indented paragraph of text for the ruler to show and change."));
        settle(w.get());
        write::Ruler *ruler = w->ruler();
        QVERIFY(ruler);
        QVERIFY(ruler->isVisible());
        QTRY_VERIFY(ruler->shows());
        const flow::Section page = w->document()->flow().section(0);
        QCOMPARE(ruler->textLeft(), page.margins.left());
        const double textWidth = page.pageSize().width() - page.margins.left() - page.margins.right();
        QCOMPARE(ruler->textWidth(), textWidth);
        const double scale = w->view()->zoom() * 96.0 / 72.0;
        auto format = [&] { return w->document()->story()->begin().blockFormat(); };
        auto marker = [&](write::Ruler::Marker m, int index = 0) {
            QPointF p;
            [&] { QVERIFY(ruler->markerPoint(m, index, &p)); }();
            return p.toPoint();
        };
        auto undoOnce = [&] {
            w->act(QStringLiteral("edit.undo"))->trigger();
            settle(w.get());
            ruler->sync();
        };
        const QString original = zipDigest(w->document()->toDocx());
        // The first line's marker: 36 points in.
        {
            const QPoint p = marker(write::Ruler::Marker::FirstLine);
            drag(ruler, p, p + QPoint(int(std::lround(36 * scale)), 0));
            settle(w.get());
            ruler->sync();
            QCOMPARE(format().textIndent(), 36.0);
            QCOMPARE(format().leftMargin(), 0.0);
            QVERIFY(zipDigest(w->document()->toDocx()) != original);
            undoOnce();
            QCOMPARE(zipDigest(w->document()->toDocx()), original);
            w->act(QStringLiteral("edit.redo"))->trigger();
            settle(w.get());
        }
        // The hanging indent moves the left indent and leaves the first line where it is.
        {
            ruler->sync();
            const QPoint p = marker(write::Ruler::Marker::Hanging);
            drag(ruler, p, p + QPoint(int(std::lround(72 * scale)), 0));
            settle(w.get());
            ruler->sync();
            QCOMPARE(format().leftMargin(), 72.0);
            QCOMPARE(format().textIndent(), 36.0 - 72.0);
        }
        // The left indent's box moves both.
        {
            ruler->sync();
            const QPoint p = marker(write::Ruler::Marker::LeftIndent);
            drag(ruler, p, p - QPoint(int(std::lround(36 * scale)), 0));
            settle(w.get());
            ruler->sync();
            QCOMPARE(format().leftMargin(), 36.0);
            QCOMPARE(format().textIndent(), -36.0);
        }
        // The right indent.
        {
            ruler->sync();
            const QPoint p = marker(write::Ruler::Marker::RightIndent);
            drag(ruler, p, p - QPoint(int(std::lround(54 * scale)), 0));
            settle(w.get());
            ruler->sync();
            QVERIFY2(std::abs(format().rightMargin() - 54.0) <= 2.3, qPrintable(QString::number(format().rightMargin())));   // (the marker snaps to the marks, counted from the left margin)
        }
        // Markers dragged far off the ruler are held to the text.
        {
            ruler->sync();
            const QPoint p = marker(write::Ruler::Marker::RightIndent);
            drag(ruler, p, QPoint(-5000, p.y()));
            settle(w.get());
            ruler->sync();
            QVERIFY(format().rightMargin() <= textWidth && format().rightMargin() >= 0);
            const QPoint f = marker(write::Ruler::Marker::FirstLine);
            drag(ruler, f, QPoint(50000, f.y()));
            settle(w.get());
            ruler->sync();
            QVERIFY(format().leftMargin() + format().textIndent() <= textWidth + 0.01);
        }
        // The margins: dragging the left margin in by 36 points.
        {
            ruler->sync();
            const QString before = zipDigest(w->document()->toDocx());
            const QPoint p = marker(write::Ruler::Marker::LeftMargin);
            drag(ruler, p, p + QPoint(int(std::lround(36 * scale)), 0));
            settle(w.get());
            ruler->sync();
            QCOMPARE(w->document()->flow().section(0).margins.left(), 108.0);
            QCOMPARE(ruler->textLeft(), 108.0);
            undoOnce();
            QCOMPARE(w->document()->flow().section(0).margins.left(), 72.0);
            QCOMPARE(zipDigest(w->document()->toDocx()), before);
            ruler->sync();
            const QPoint r = marker(write::Ruler::Marker::RightMargin);
            drag(ruler, r, r - QPoint(int(std::lround(36 * scale)), 0));
            settle(w.get());
            QVERIFY(std::abs(w->document()->flow().section(0).margins.right() - 108.0) <= 2.3);
            undoOnce();
            QCOMPARE(w->document()->flow().section(0).margins.right(), 72.0);
        }
        // Tab stops: a click adds one of the kind in the box at the ruler's left end; a drag moves it; a drag off the ruler takes it away.
        {
            w->document()->undo();   // (the indents above)
            while (w->document()->canUndo() && !format().tabPositions().isEmpty()) w->document()->undo();
            settle(w.get());
            ruler->sync();
            const QPoint zero = marker(write::Ruler::Marker::LeftMargin);
            QVERIFY(format().tabPositions().isEmpty());
            const QString before = zipDigest(w->document()->toDocx());
            QTest::mouseClick(ruler, Qt::LeftButton, Qt::NoModifier, QPoint(zero.x() + int(std::lround(144 * scale)), 13));
            settle(w.get());
            ruler->sync();
            QCOMPARE(ruler->tabStops().size(), 1);
            QCOMPARE(ruler->tabStops()[0].position, 144.0);
            QVERIFY(ruler->tabStops()[0].type == QTextOption::LeftTab);
            undoOnce();
            QCOMPARE(zipDigest(w->document()->toDocx()), before);
            w->act(QStringLiteral("edit.redo"))->trigger();
            settle(w.get());
            ruler->sync();
            // The kind is changed by a click on the box.
            QCOMPARE(int(ruler->tabKind()), int(QTextOption::LeftTab));
            QPointF box;
            QVERIFY(ruler->markerPoint(write::Ruler::Marker::TabKind, 0, &box));
            QTest::mouseClick(ruler, Qt::LeftButton, Qt::NoModifier, box.toPoint());
            QVERIFY(ruler->tabKind() == QTextOption::CenterTab);
            QTest::mouseClick(ruler, Qt::LeftButton, Qt::NoModifier, QPoint(zero.x() + int(std::lround(288 * scale)), 13));
            settle(w.get());
            ruler->sync();
            QCOMPARE(ruler->tabStops().size(), 2);
            QVERIFY(ruler->tabStops()[1].type == QTextOption::CenterTab);
            // Moved.
            const QPoint t = marker(write::Ruler::Marker::TabStop, 0);
            drag(ruler, t, t + QPoint(int(std::lround(36 * scale)), 0));
            settle(w.get());
            ruler->sync();
            QCOMPARE(ruler->tabStops()[0].position, 180.0);
            // Taken away.
            const QPoint u = marker(write::Ruler::Marker::TabStop, 1);
            drag(ruler, u, QPoint(u.x(), 200));
            settle(w.get());
            ruler->sync();
            QCOMPARE(ruler->tabStops().size(), 1);
        }
        // Saved, and opened again: the tab stop and the indents.
        const QString path = tempFile(QStringLiteral("ruler.docx"));
        QFile::remove(path);
        w->setSavePathChooser([&](const QString &, const QString &) { return path; });
        QVERIFY(w->saveAs());
        std::unique_ptr<WriteWindow> again(static_cast<WriteWindow *>(m_app.launchWith(path)));
        QVERIFY(again);
        QCOMPARE(write::tabStopsOf(again->document()->story()->begin().blockFormat()), write::tabStopsOf(format()));
        QCOMPARE(again->document()->story()->begin().blockFormat().rightMargin(), format().rightMargin());
        // The ruler goes and comes with View > Ruler.
        w->act(QStringLiteral("view.ruler"))->trigger();
        QVERIFY(!ruler->isVisible());
        w->act(QStringLiteral("view.ruler"))->trigger();
        QVERIFY(ruler->isVisible());
    }

    // ---- wave C review

    // A window with a floating picture of 120 by 60 points selected, among text that fills more than a page.
    static WriteWindow *windowWithAFloatingPicture(const QString &file)
    {
        auto *w = launchBlank();
        typeInto(w->view(), QString(QStringLiteral("Some words that a picture will sit among, enough of them to fill several lines of the page. ")).repeated(40));
        w->setPictureChooser([file] { return file; });
        w->view()->cursor().setPosition(20);
        w->act(QStringLiteral("insert.picture"))->trigger();
        settle(w);
        w->act(QStringLiteral("picture.wrap.square"))->trigger();
        settle(w);
        return w;
    }

    void aSizeThatKeepsItsRatioIsHeldToTheLimitsAsAWholeInTheWindow()
    {
        std::unique_ptr<WriteWindow> w(windowWithAFloatingPicture(pngFile(160, 80)));
        const int at = w->view()->selectedPicture();
        QVERIFY(at >= 0);
        auto info = [&] { return write::pictureAt(*w->document(), w->document()->story(), at); };
        QCOMPARE(info().width, 120.0);
        QCOMPARE(info().height, 60.0);
        auto *width = w->findChild<MeasureSpin *>(QStringLiteral("pictureWidthField"));
        QVERIFY(width);
        width->setValue(width->maximum());
        Q_EMIT width->editingFinished();
        QCOMPARE(info().width, write::kMostPictureSize);
        QCOMPARE(info().height, write::kMostPictureSize / 2);
        w->act(QStringLiteral("edit.undo"))->trigger();
        settle(w.get());
        // A corner dragged a great way out stops at the same ceiling, in the same proportions.
        QPointF corner;
        QVERIFY(w->view()->pictureHandlePoint(write::PageView::PictureHandle::BottomRight, &corner));
        drag(w->view()->viewport(), corner.toPoint(), corner.toPoint() + QPoint(100000, 100000));
        settle(w.get());
        QCOMPARE(info().width, write::kMostPictureSize);
        QCOMPARE(info().height, write::kMostPictureSize / 2);
    }

    // A drag keeps the scroll it began at: a wheel turned during it does not move the result; an Undo during it ends it with nothing done.
    void aPictureDragIsNotMovedByAScrollAndEndsWithAnUndo()
    {
        std::unique_ptr<WriteWindow> w(windowWithAFloatingPicture(pngFile(160, 80)));
        const int at = w->view()->selectedPicture();
        QVERIFY(at >= 0);
        auto info = [&] { return write::pictureAt(*w->document(), w->document()->story(), at); };
        QWidget *viewport = w->view()->viewport();
        const double scale = w->view()->zoom() * 96.0 / 72.0;
        QRectF pic;
        QVERIFY(w->view()->pictureRect(at, &pic));
        drag(viewport, pic.center().toPoint(), pic.center().toPoint() + QPoint(80, 60));   // (a place of its own on the page)
        settle(w.get());
        const double left = info().left, top = info().top;
        QVERIFY(info().placeIsOffset);
        QVERIFY(w->view()->pictureRect(at, &pic));
        const QPoint from = pic.center().toPoint();
        QTest::mousePress(viewport, Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(viewport, from + QPoint(30, 20));
        QScrollBar *bar = w->view()->verticalScrollBar();
        QVERIFY(bar->maximum() > 100);
        bar->setValue(bar->value() + 100);
        QTest::mouseMove(viewport, from + QPoint(50, 40));
        QTest::mouseRelease(viewport, Qt::LeftButton, Qt::NoModifier, from + QPoint(50, 40));
        settle(w.get());
        QVERIFY2(std::abs(info().left - (left + 50 / scale)) < 1 && std::abs(info().top - (top + 40 / scale)) < 1,
                 qPrintable(QStringLiteral("%1,%2 from %3,%4 by %5,%6").arg(info().left).arg(info().top).arg(left).arg(top).arg(50 / scale).arg(40 / scale)));
        // Undo in the middle of a drag: the drag is over, and what it would have done is not done.
        QVERIFY(w->view()->pictureRect(at, &pic));
        const QPoint start = pic.center().toPoint();
        QTest::mousePress(viewport, Qt::LeftButton, Qt::NoModifier, start);
        QTest::mouseMove(viewport, start + QPoint(40, 40));
        w->refreshNow();
        w->act(QStringLiteral("edit.undo"))->trigger();
        const QString afterUndo = zipDigest(w->document()->toDocx());
        QTest::mouseMove(viewport, start + QPoint(70, 70));
        QTest::mouseRelease(viewport, Qt::LeftButton, Qt::NoModifier, start + QPoint(70, 70));
        settle(w.get());
        QCOMPARE(zipDigest(w->document()->toDocx()), afterUndo);
    }

    // The pointer moving over a picture that is selected does not work out where its handles are every time.
    void thePointerOverASelectedPictureDoesNotMeasureItEachTime()
    {
        std::unique_ptr<WriteWindow> w(windowWithAFloatingPicture(pngFile(160, 80)));
        QVERIFY(w->view()->selectedPicture() >= 0);
        QRectF pic;
        QVERIFY(w->view()->pictureRect(w->view()->selectedPicture(), &pic));
        QWidget *viewport = w->view()->viewport();
        QTest::mouseMove(viewport, pic.center().toPoint());   // (the first look)
        const qint64 before = write::PageView::pictureRectsComputedForTest();
        for (int i = 0; i < 200; ++i) QTest::mouseMove(viewport, QPoint(int(pic.left()) - 20 + i, int(pic.top()) + i / 2));
        QVERIFY2(write::PageView::pictureRectsComputedForTest() - before <= 4, qPrintable(QString::number(write::PageView::pictureRectsComputedForTest() - before)));
        // A change of the picture is seen: the handle is where the picture is now.
        const QPointF was = pic.bottomRight();
        QPointF corner;
        QVERIFY(w->view()->pictureHandlePoint(write::PageView::PictureHandle::BottomRight, &corner));
        QCOMPARE(corner, was);
        w->view()->changeSelectedPicture([](write::WriteDoc &d, int position) { return write::setPictureSize(d, position, 60, 30); });
        settle(w.get());
        QVERIFY(w->view()->pictureHandlePoint(write::PageView::PictureHandle::BottomRight, &corner));
        QVERIFY(std::abs(corner.x() - (pic.left() + 60 * w->view()->zoom() * 96 / 72)) < 2);
    }

#ifdef Q_OS_UNIX
    // A name that is a pipe or a file that never ends is not read: the command says so and returns.
    void aPictureNameThatIsAPipeIsRefusedWithoutWaiting()
    {
        std::unique_ptr<WriteWindow> w(launchBlank());
        typeInto(w->view(), QStringLiteral("text"));
        const QString fifo = tempFile(QStringLiteral("pipe.png"));
        QFile::remove(fifo);
        QVERIFY(::mkfifo(QFile::encodeName(fifo).constData(), 0600) == 0);
        w->setPictureChooser([fifo] { return fifo; });
        w->act(QStringLiteral("insert.picture"))->trigger();
        QCOMPARE(w->document()->story()->toPlainText(), QStringLiteral("text"));
        QVERIFY(!w->statusBar()->currentMessage().isEmpty());
    }
#endif

    // The handle rectangle of another document is not the cached one, though the text, the picture, and the place are the same.
    void theHandleRectangleIsWorkedOutAgainForAnotherDocument()
    {
        std::unique_ptr<WriteWindow> w(windowWithAFloatingPicture(pngFile(160, 80)));
        const int at = w->view()->selectedPicture();
        QVERIFY(at >= 0);
        QRectF r;
        QVERIFY(w->view()->pictureRect(at, &r));
        const qint64 before = write::PageView::pictureRectsComputedForTest();
        QVERIFY(w->view()->pictureRect(at, &r));
        QCOMPARE(write::PageView::pictureRectsComputedForTest(), before);   // (the same document: the cached one)
        w->setDocument(write::WriteDoc::fromDocx(w->document()->toDocx()), QString());
        w->view()->selectPicture(at);
        settle(w.get());
        QRectF again;
        QVERIFY(w->view()->pictureRect(at, &again));
        QVERIFY(write::PageView::pictureRectsComputedForTest() > before);
    }

    // Every tab of the ribbon, at two window widths: nothing cut, overlapping, or sticking out of its group.
    void ribbonLayout()
    {
        const QStringList fails = checkRibbonLayout(m_app);
        qInfo("ribbon layout: %lld failures", qint64(fails.size()));
        QVERIFY2(fails.isEmpty(), qPrintable(fails.join('\n')));
    }

    // ---- 3. every command
    // The commands of wave D: Create Style, Modify Style, Update Style, Copy Formatting, Copy and Paste Format, Go To, and the Styles pane. Each is one step that undoes to the bytes before it,
    // and the style made saves and opens the same.
    void theStyleCommandsFormatPainterAndGoToOfTheWindowChangeTheDocumentUndoAndSaveAndOpenTheSame()
    {
        std::unique_ptr<WriteWindow> w(launchBlank());
        typeInto(w->view(), QStringLiteral("Alpha beta gamma\nSecond paragraph\nThird one"));
        settle(w.get());
        write::WriteDoc &doc = *w->document();
        auto select = [&](int from, int to) {
            QTextCursor c(doc.story());
            c.setPosition(from);
            c.setPosition(to, QTextCursor::KeepAnchor);
            w->view()->setCursor(c);
            settle(w.get());
        };
        // Create Style from the first paragraph, bold and centered.
        select(0, 16);
        w->act(QStringLiteral("fmt.bold"))->trigger();
        w->act(QStringLiteral("fmt.alignCenter"))->trigger();
        settle(w.get());
        const QString plainDigest = zipDigest(doc.toDocx());
        answer([](QDialog *d) { d->findChild<QLineEdit *>(QStringLiteral("styleName"))->setText(QStringLiteral("Pulled words")); });
        w->act(QStringLiteral("style.new"))->trigger();
        settle(w.get());
        QVERIFY(doc.styleSheet().find(QStringLiteral("Pulledwords")));
        QCOMPARE(doc.story()->begin().blockFormat().stringProperty(tp::StyleName), QStringLiteral("Pulledwords"));
        const QByteArray saved = doc.toDocx();
        QString error;
        QStringList notes;
        auto back = write::WriteDoc::fromDocx(saved, &error);
        QVERIFY2(back, qPrintable(error));
        QVERIFY(back->styleSheet().find(QStringLiteral("Pulledwords")));
        QCOMPARE(back->story()->begin().blockFormat().stringProperty(tp::StyleName), QStringLiteral("Pulledwords"));
        w->act(QStringLiteral("edit.undo"))->trigger();
        settle(w.get());
        QCOMPARE(zipDigest(doc.toDocx()), plainDigest);
        w->act(QStringLiteral("edit.redo"))->trigger();
        settle(w.get());
        QCOMPARE(zipDigest(doc.toDocx()), zipDigest(saved));
        // Modify Style, then Update Style to Match Selection (a size given by hand to the second paragraph's words).
        const QString madeDigest = zipDigest(doc.toDocx());
        select(0, 0);
        answer([](QDialog *d) { d->findChild<QComboBox *>(QStringLiteral("styleSize"))->setEditText(QStringLiteral("24")); });
        w->act(QStringLiteral("style.modify"))->trigger();
        settle(w.get());
        QCOMPARE(doc.styleSheet().find(QStringLiteral("Pulledwords"))->rPr.size, std::optional<int>(48));
        w->act(QStringLiteral("edit.undo"))->trigger();
        settle(w.get());
        QCOMPARE(zipDigest(doc.toDocx()), madeDigest);
        select(17, 23);
        w->act(QStringLiteral("fmt.italic"))->trigger();
        settle(w.get());
        const QString italicDigest = zipDigest(doc.toDocx());
        w->act(QStringLiteral("style.update"))->trigger();
        settle(w.get());
        QCOMPARE(doc.styleSheet().find(QStringLiteral("Normal")) ? doc.styleSheet().find(QStringLiteral("Normal"))->rPr.italic : std::optional<bool>(), std::optional<bool>(true));
        w->act(QStringLiteral("edit.undo"))->trigger();
        settle(w.get());
        QCOMPARE(zipDigest(doc.toDocx()), italicDigest);
        // Copy Format and Paste Format by their commands, then the painter by its command and a selection made with the mouse.
        select(0, 5);
        w->act(QStringLiteral("edit.copyFormat"))->trigger();
        select(24, 30);
        const QString beforePaste = zipDigest(doc.toDocx());
        w->act(QStringLiteral("edit.pasteFormat"))->trigger();
        settle(w.get());
        QVERIFY(zipDigest(doc.toDocx()) != beforePaste);
        w->act(QStringLiteral("edit.undo"))->trigger();
        settle(w.get());
        QCOMPARE(zipDigest(doc.toDocx()), beforePaste);
        select(0, 5);
        w->act(QStringLiteral("edit.formatPainter"))->trigger();
        QCOMPARE(asWrite(w.get())->painterMode(), WriteWindow::PainterMode::Once);
        QTest::keyClick(w->view(), Qt::Key_Escape);
        QCOMPARE(asWrite(w.get())->painterMode(), WriteWindow::PainterMode::Off);
        // Go To a page, and a place that is not there.
        select(30, 30);
        answer([](QDialog *d) {
            d->findChild<QListWidget *>(QStringLiteral("goToKinds"))->setCurrentRow(2);   // Line
            d->findChild<QLineEdit *>(QStringLiteral("goToWhat"))->setText(QStringLiteral("2"));
        });
        const QString beforeGo = zipDigest(doc.toDocx());
        w->act(QStringLiteral("edit.goTo"))->trigger();
        settle(w.get());
        QCOMPARE(w->view()->cursor().position(), 17);   // (the second paragraph's line)
        QCOMPARE(zipDigest(doc.toDocx()), beforeGo);
        // The Styles pane lists the style made.
        w->act(QStringLiteral("style.pane"))->trigger();
        settle(w.get());
        QVERIFY(asWrite(w.get())->stylesPane());
        QVERIFY(asWrite(w.get())->stylesPane()->rowOf(QStringLiteral("p:Pulledwords")) >= 0);
    }

    void everyCommandOnABlankDocument()
    {
        QStringList report;
        const QStringList fails = runEveryAction(m_app, false, &report);
        qInfo("%s", qPrintable(report.last()));
        if (qEnvironmentVariableIsSet("JP_CHECKS_REPORT")) qInfo("%s", qPrintable(report.join('\n')));
        QVERIFY2(fails.isEmpty(), qPrintable(fails.join('\n')));
    }

    void everyCommandOnADocumentFromAFile()
    {
        QStringList report;
        const QStringList fails = runEveryAction(m_app, true, &report);
        qInfo("%s", qPrintable(report.last()));
        if (qEnvironmentVariableIsSet("JP_CHECKS_REPORT")) qInfo("%s", qPrintable(report.join('\n')));
        QVERIFY2(fails.isEmpty(), qPrintable(fails.join('\n')));
    }
    // The commands that no test could enable in either state are named, each with the reason, and no more of them than the list says.
    void commandsNeverRun()
    {
        QStringList report;
        const QStringList fails = checkNeverRun(m_app, &report);
        qInfo("%s", qPrintable(report.first()));
        if (qEnvironmentVariableIsSet("JP_CHECKS_REPORT")) qInfo("%s", qPrintable(report.join('\n')));
        QVERIFY2(fails.isEmpty(), qPrintable(fails.join('\n')));
    }
};

JP_TEST_CLASS(WriteAppChecks)
#include "tst_appchecks_write.moc"
