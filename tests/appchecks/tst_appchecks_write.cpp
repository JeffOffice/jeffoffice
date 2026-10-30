// The checks every app's window must pass, for JeffWrite (see appchecks.h): its first five minutes, its standard commands, and every command it has.
#include "appchecks.h"

#include "app/filescreen.h"
#include "app/pageview.h"
#include "app/settings.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/writedoc.h"
#include "testrunner.h"

#include <QClipboard>
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
        m_app.settle = [](AppWindow *w) {
            asWrite(w)->refreshNow();
            drain();
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
        };
        m_app.quiet = {
            {QStringLiteral("view.printLayout"), QStringLiteral("the print layout is the only layout JeffWrite has, and the window begins in it")},
        };
        m_app.neverRun = {};
    }

    // ---- 1. the first five minutes
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

        // The print layout: the File screen's Print page, with the page drawn.
        again->act(QStringLiteral("file.print"))->trigger();
        auto *screen = again->findChild<FileScreen *>();
        QVERIFY(screen);
        QCOMPARE(screen->currentPage(), QStringLiteral("print"));
        QVERIFY(waitFor([&] {
            const QLabel *preview = screen->findChild<QLabel *>(QStringLiteral("printPreview"));
            return preview && hasDrawnContent(preview->pixmap());
        }));
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

    // ---- 3. every command
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
