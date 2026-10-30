// The checks every app's window must pass, for JeffCrunch (see appchecks.h): its first five minutes, its standard commands, and every command it has.
//
// A new window with no file must hold a blank workbook that takes typing (JeffCrunch 0.10.0 shipped without one: its window was an empty grid that said the workbook "is still being
// opened", and nothing tested a window that was not opening a file). What these checks need of that fix is marked "NEEDS crunch-new" (the change that makes a blank workbook and File > New): they
// fail on a program without it, which is what they are for.
#include "appchecks.h"

#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/filescreen.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/editsession.h"
#include "crunch/editwindow.h"
#include "grid/gridview.h"
#include "testrunner.h"

#include <QClipboard>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLabel>
#include <QToolButton>
#include <QStatusBar>
#include <memory>

using namespace jp;
using namespace appchecks;

namespace {

CrunchWindow *asCrunch(AppWindow *w) { return static_cast<CrunchWindow *>(w); }

// What the program does when it is started with no file: a new workbook. (A window of a program that has no way to make one is left empty, which is what the person then gets.)
template <class W>
auto startBlank(W &w, int) -> decltype(w.newBlank(), void())
{
    w.newBlank();
}
template <class W>
void startBlank(W &, long)
{
}

// Whether the window is ready for a person: nothing opening, nothing being worked out.
bool quiet(CrunchWindow *w) { return !w->isLoading() && !w->isCalculating() && !w->isSaving(); }

CrunchWindow *launchBlank()
{
    auto *w = new CrunchWindow();
    w->resize(1100, 640);
    w->show();
    startBlank(*w, 0);
    waitFor([&] { return quiet(w); });
    return w;
}

// Types into a cell as a person does: the first character at the grid (which starts the entry), the rest into the cell's line, then Return.
void enter(CrunchWindow *w, qint64 row, qint64 col, const QString &text)
{
    w->view()->selectCell(row, col);
    QTest::keyClick(w->view(), text.at(0).toLatin1());
    if (w->cellEditor() && text.size() > 1) QTest::keyClicks(w->cellEditor(), text.mid(1));
    if (w->cellEditor()) QTest::keyClick(w->cellEditor(), Qt::Key_Return);
}

// What the grid shows in a cell.
QString shown(CrunchWindow *w, quint32 row, quint32 col)
{
    const grid::Cell *cell = w->view()->sheet().cellAt(row, col);
    return cell ? w->view()->sheet().textOf(row, col, *cell) : QString();
}

}

class CrunchAppChecks : public QObject {
    Q_OBJECT

    Adapter m_app;

private Q_SLOTS:
    void initTestCase()
    {
        isolateSettings();
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        setAppInfo(crunchAppInfo());
        initCore();
        m_app.name = QStringLiteral("JeffCrunch");
        m_app.launch = [] { return static_cast<AppWindow *>(launchBlank()); };
        m_app.launchWith = [](const QString &path) -> AppWindow * {
            // (a file that is open in a window is not opened in a second: each window has a copy of its own)
            static int copies = 0;
            const QString copy = tempFile(QStringLiteral("copy-%1.%2").arg(++copies).arg(QFileInfo(path).suffix()));
            QFile::remove(copy);
            if (!QFile::copy(path, copy)) return nullptr;
            auto *w = new CrunchWindow();
            w->resize(1100, 640);
            w->show();
            if (!w->openFile(copy)) {
                delete w;
                return nullptr;
            }
            waitFor([&] { return w->hasWorkbook() && quiet(w); });
            return w;
        };
        m_app.type = [](AppWindow *w) { enter(asCrunch(w), 0, 0, QStringLiteral("12")); };
        m_app.documentView = [](AppWindow *w) -> QWidget * { return asCrunch(w)->view()->viewport(); };
        m_app.settle = [](AppWindow *w) {
            waitFor([&] { return quiet(asCrunch(w)); });
            drain();
        };
        m_app.listen = [](AppWindow *aw, Watch *watch) {
            auto *w = asCrunch(aw);
            w->setNotifier([watch](const QString &title, const QString &text) { watch->told << title + QLatin1String(": ") + text; });
            w->setSavePicker([](const QString &start, const QString &filter) {
                static int n = 0;
                const QString ending = filter.contains(QLatin1String("*.csv")) && !filter.contains(QLatin1String("*.xlsx")) ? QStringLiteral(".csv") : QStringLiteral(".xlsx");
                Q_UNUSED(start);
                return tempFile(QStringLiteral("every-%1%2").arg(++n).arg(ending));
            });
        };
        m_app.docState = [](AppWindow *aw) {
            auto *w = asCrunch(aw);
            QString s = QStringLiteral("%1%2%3|%4|%5|%6|%7").arg(w->hasWorkbook()).arg(w->isLoading()).arg(w->isModified()).arg(w->filePath(), w->statusText(), w->view()->selectionName()).arg(w->isEditing());
            if (w->session()) {
                QByteArray bytes;
                QString why;
                s += w->snapshotBytesForTest(&bytes, &why) ? QLatin1Char('|') + zipDigest(bytes) : QLatin1Char('|') + why;
            }
            const grid::GridView::ViewState v = w->view()->viewState();
            return s + QStringLiteral("|%1|%2,%3|%4,%5|%6,%7|%8").arg(w->view()->zoom()).arg(v.frozenRows).arg(v.frozenCols).arg(v.scrollX).arg(v.scrollY).arg(v.gridlines).arg(v.headings).arg(w->view()->rangeName());
        };
        // The sample: a small workbook built here (numbers, text, formulas), written to a file.
        {
            QFile f(tempFile(QStringLiteral("sample.xlsx")));
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(crunchtest::editBook());
        }
        m_app.sampleFile = tempFile(QStringLiteral("sample.xlsx"));
        // The places a person is in when a command is there to use: cells selected (a block, rows, columns), a formula's cell, cells copied, and a cell being typed in.
        m_app.contexts = {
            {QStringLiteral("a block of cells selected"), [](AppWindow *w, bool) { asCrunch(w)->view()->selectRange(0, 0, 2, 2); }, false},
            {QStringLiteral("whole rows selected"), [](AppWindow *w, bool) { asCrunch(w)->view()->selectRows(0, 1); }, false},
            {QStringLiteral("whole columns selected"), [](AppWindow *w, bool) { asCrunch(w)->view()->selectColumns(0, 1); }, false},
            {QStringLiteral("a formula's cell selected"), [](AppWindow *w, bool) { asCrunch(w)->view()->selectCell(0, 1); }, false},
            {QStringLiteral("cells copied"), [](AppWindow *aw, bool) {
                 auto *w = asCrunch(aw);
                 w->view()->selectRange(0, 0, 1, 1);
                 w->act(QStringLiteral("edit.copy"))->trigger();
                 waitFor([&] { return !w->isCopying(); });
                 w->view()->selectCell(5, 5);
             }, false},
            {QStringLiteral("a cell below and right of the first"), [](AppWindow *w, bool) { asCrunch(w)->view()->selectCell(3, 2); }, false},
            {QStringLiteral("the cursor in a table"), [](AppWindow *aw, bool) {
                 auto *w = asCrunch(aw);
                 w->setTableMakeAsker([](QString *, bool *headers) {
                     *headers = true;
                     return true;
                 });
                 // (a blank workbook has nothing to make a table of: a few cells are typed first)
                 if (!w->session() || w->view()->sheet().cellAt(0, 0) == nullptr) {
                     enter(w, 0, 0, QStringLiteral("Name"));
                     enter(w, 0, 1, QStringLiteral("Count"));
                     enter(w, 1, 0, QStringLiteral("a"));
                     enter(w, 1, 1, QStringLiteral("3"));
                     waitFor([&] { return quiet(w); });
                 }
                 w->view()->selectCell(1, 1);
                 w->act(QStringLiteral("insert.table"))->trigger();
                 waitFor([&] { return quiet(w); });
             }, false},
            {QStringLiteral("a cell being typed in"), [](AppWindow *aw, bool) {
                 auto *w = asCrunch(aw);
                 w->view()->selectCell(5, 5);
                 QTest::keyClick(w->view(), Qt::Key_Equal);
             }, false},
        };
        m_app.skipped = {};
        // What a command may leave as it is, where what it does has nothing to do.
        const QString menu = QStringLiteral("the name of a menu: its choices are the commands, and it has none of its own");
        m_app.quiet = {
            {QStringLiteral("view.formulas@blank"), QStringLiteral("it shows the formulas in place of their values, and a blank sheet has none")},
            {QStringLiteral("cells.format"), menu},
            {QStringLiteral("formulas.options"), menu},
            {QStringLiteral("sheet.tabColor"), menu},
            {QStringLiteral("filter.clear"), QStringLiteral("no filter is on, so none can be cleared")},
            {QStringLiteral("filter.reapply"), QStringLiteral("no filter is on, so none can be applied again")},
            {QStringLiteral("fmt.decimalDec"), QStringLiteral("the cells show no decimals to take one away from")},
            {QStringLiteral("fmt.indentDec"), QStringLiteral("the cells have no indent to take away")},
            {QStringLiteral("formulas.calcAuto"), QStringLiteral("the workbook calculates by itself already")},
            {QStringLiteral("formulas.calculate"), QStringLiteral("every value is up to date, so a calculation changes none")},
            {QStringLiteral("formulas.calculateSheet"), QStringLiteral("every value is up to date, so a calculation changes none")},
            {QStringLiteral("formulas.removeArrows"), QStringLiteral("no arrows are drawn")},
            {QStringLiteral("printarea.clear"), QStringLiteral("no print area is set")},
            {QStringLiteral("sheet.unhide"), QStringLiteral("no sheet is hidden")},
            {QStringLiteral("view.normal"), QStringLiteral("a window begins in the normal view")},
            {QStringLiteral("view.pageBreaks"), QStringLiteral("the page lines are worked out for the printed pages, and a workbook this small has none to show")},
            {QStringLiteral("view.unfreeze"), QStringLiteral("no rows or columns are held in place")},
            {QStringLiteral("zoom.100"), QStringLiteral("a window begins at 100%")},
        };
    }

    // ---- 1. the first five minutes
    void firstFiveMinutes()
    {
        Watch watch;
        std::unique_ptr<CrunchWindow> w(launchBlank());
        QStringList told;
        w->setNotifier([&](const QString &title, const QString &text) { told << title + QLatin1String(": ") + text; });
        // A new window holds a workbook that is ready (NEEDS crunch-new: a real blank workbook), named for what it is.
        QVERIFY2(w->hasWorkbook(), "NEEDS crunch-new: a window with no file holds no workbook, only an empty grid");
        QVERIFY(w->session() != nullptr);
        QVERIFY2(w->windowTitle().contains(w->displayName()), qPrintable(w->windowTitle()));
        QVERIFY(w->windowTitle().contains(QLatin1String("JeffCrunch")));
        QVERIFY2(!saysNotYet(w->statusText()), qPrintable(QStringLiteral("NEEDS crunch-new: the status line says ") + w->statusText()));
        QVERIFY(!w->isModified());

        // Typing goes into the cells, a formula works, and the title shows the change.
        enter(w.get(), 0, 0, QStringLiteral("21"));
        QVERIFY2(w->isModified(), "NEEDS crunch-new: typing into a window with no file changes nothing");
        enter(w.get(), 0, 1, QStringLiteral("=A1*2"));
        QVERIFY(waitFor([&] { return quiet(w.get()); }));
        QCOMPARE(shown(w.get(), 0, 0), QStringLiteral("21"));
        QCOMPARE(shown(w.get(), 0, 1), QStringLiteral("42"));
        QVERIFY2(w->isWindowModified(), qPrintable(w->windowTitle()));   // (the title is "name[*]": the program puts the star in place of [*] while it is changed)
        QVERIFY2(!saysNotYet(w->statusText()), qPrintable(w->statusText()));
        QVERIFY2(told.isEmpty(), qPrintable(told.join('\n')));

        // Undo takes the formula back, Redo puts it again.
        QAction *undo = w->act(QStringLiteral("edit.undo")), *redo = w->act(QStringLiteral("edit.redo"));
        QVERIFY(waitFor([&] { return undo->isEnabled(); }));
        undo->trigger();
        QVERIFY(waitFor([&] { return quiet(w.get()); }));
        QCOMPARE(shown(w.get(), 0, 1), QString());
        QCOMPARE(shown(w.get(), 0, 0), QStringLiteral("21"));
        QVERIFY(waitFor([&] { return redo->isEnabled(); }));
        redo->trigger();
        QVERIFY(waitFor([&] { return quiet(w.get()); }));
        QCOMPARE(shown(w.get(), 0, 1), QStringLiteral("42"));

        // Save As in the program's own format (the name is picked in the test's folder): the window takes the file's name.
        const QString path = tempFile(QStringLiteral("five.xlsx"));
        QFile::remove(path);
        w->setSavePicker([&](const QString &, const QString &) { return path; });
        QSignalSpy saved(w.get(), &CrunchWindow::saved);
        w->act(QStringLiteral("file.saveAs"))->trigger();
        QVERIFY(saved.count() > 0 || saved.wait(20000));
        QVERIFY(saved.last().at(1).toBool());
        QVERIFY(QFileInfo::exists(path));
        QVERIFY(!w->isModified());
        QVERIFY2(w->windowTitle().startsWith(QFileInfo(path).completeBaseName()), qPrintable(w->windowTitle()));
        QVERIFY2(!w->isWindowModified(), qPrintable(w->windowTitle()));

        // The saved file is among the recent ones, on the File screen's Open page.
        QVERIFY(Settings::get().recentFiles().contains(path));
        w->showFileScreen(QStringLiteral("open"));
        QVERIFY(listsRecentFile(w->findChild<FileScreen *>(), path));
        w->hideFileScreen();

        // Closed (nothing changed since the save: no question), and opened again: the same cells.
        const CloseResult closed = closeWindow(w.get(), watch);
        QVERIFY(closed.closed);
        QCOMPARE(closed.asked, 0);
        w.reset();
        std::unique_ptr<CrunchWindow> again(new CrunchWindow());
        again->resize(1100, 640);
        again->show();
        QVERIFY(again->openFile(path));
        QVERIFY(waitFor([&] { return again->hasWorkbook() && quiet(again.get()); }));
        QCOMPARE(shown(again.get(), 0, 0), QStringLiteral("21"));
        QCOMPARE(shown(again.get(), 0, 1), QStringLiteral("42"));
        QVERIFY(!again->isModified());
        QCOMPARE(again->filePath(), path);

        // File > New: a second window with a blank workbook of its own that takes typing (NEEDS crunch-new: there is no File > New).
        const int windows = AppWindow::windows().size();
        QAction *fileNew = again->act(QStringLiteral("file.new"));
        QVERIFY2(fileNew, "NEEDS crunch-new: JeffCrunch has no File > New");
        fileNew->trigger();
        drain();
        QCOMPARE(AppWindow::windows().size(), windows + 1);
        CrunchWindow *second = nullptr;
        for (AppWindow *other : AppWindow::windows())
            if (other != again.get()) second = asCrunch(other);
        QVERIFY(second);
        QPointer<CrunchWindow> guard(second);
        QVERIFY(waitFor([&] { return second->hasWorkbook() && quiet(second); }));
        enter(second, 0, 0, QStringLiteral("5"));
        QCOMPARE(shown(second, 0, 0), QStringLiteral("5"));
        // (a changed workbook asks before it goes; the answer here is no)
        const CloseResult asked = closeWindow(second, watch);
        QCOMPARE(asked.asked, 1);
        QVERIFY(!asked.closed);
        second->setAttribute(Qt::WA_DeleteOnClose, false);
        delete second;

        // The print layout: the File screen's Print page, with the sheet drawn.
        again->act(QStringLiteral("file.print"))->trigger();
        auto *screen = again->findChild<FileScreen *>();
        QVERIFY(screen && screen->isVisible());
        QCOMPARE(screen->currentPage(), QStringLiteral("print"));
        QVERIFY(waitFor([&] {
            const QLabel *preview = screen->findChild<QLabel *>(QStringLiteral("printPreview"));
            return preview && !preview->pixmap().isNull();
        }));
        again->hideFileScreen();

        // An unchanged workbook goes without a question.
        const CloseResult plain = closeWindow(again.get(), watch);
        QVERIFY(plain.closed);
        QCOMPARE(plain.asked, 0);
        QVERIFY2(watch.warnings.isEmpty(), qPrintable(watch.warnings.join('\n')));
    }

    // The way these checks see a window does not change by itself: a call that does nothing is seen to do nothing (or every command would pass).
    void aCallThatDoesNothingChangesNothing()
    {
        Watch watch;
        std::unique_ptr<AppWindow> w(m_app.launch());
        m_app.settle(w.get());
        const Outcome o = observe(m_app, w.get(), nullptr, watch, [] {}, false, 300);
        QVERIFY2(!o.changed, qPrintable(o.how + o.detail));
    }

    // No command on the ribbon is a stub that never works (a button that is dimmed for good, its tip saying it is not there yet), and no command that works says it does not.
    void noRibbonCommandIsAStub()
    {
        std::unique_ptr<AppWindow> w(m_app.launch());
        QStringList stubs;
        for (const QToolButton *b : w->ribbon()->findChildren<QToolButton *>())
            if (const QAction *a = b->defaultAction(); a && a->toolTip().contains(QLatin1String("not available in JeffCrunch yet"))) stubs << a->objectName();
        QVERIFY2(stubs.isEmpty(), qPrintable(stubs.join(QLatin1Char(' '))));
    }

    // ---- 2. the standard commands (NEEDS crunch-new: Save, Save As, Undo, and Redo are dimmed with no workbook, and there is no New)
    void standardCommands()
    {
        const QStringList fails = checkStandardCommands(m_app);
        QVERIFY2(fails.isEmpty(), qPrintable(QStringLiteral("NEEDS crunch-new for what is about File > New and the dimmed commands of a window with no workbook:\n") + fails.join('\n')));
    }

    // ---- 3. every command (the blank window's commands NEED crunch-new: with no workbook they are dimmed, or say the workbook is still being opened)
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
};

JP_TEST_CLASS(CrunchAppChecks)
#include "tst_appchecks_crunch.moc"
