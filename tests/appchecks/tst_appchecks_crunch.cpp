// The checks every app's window must pass, for JeffCrunch (see appchecks.h): its first five minutes, its standard commands, and every command it has.
//
// A new window with no file must hold a blank workbook that takes typing and a name like Book1 (JeffCrunch 0.10.0 shipped without one: its window was an empty grid that said the workbook "is still
// being opened", and nothing tested a window that was not opening a file).
#include "appchecks.h"

#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/filescreen.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/editsession.h"
#include "office/package.h"
#include "sheet/sheetreader.h"
#include "crunch/editwindow.h"
#include "grid/gridview.h"
#include "grid/pagelayoutview.h"
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

// Whether the window is ready for a person: nothing opening, nothing being worked out.
bool quiet(CrunchWindow *w) { return !w->isLoading() && !w->isCalculating() && !w->isSaving(); }

CrunchWindow *launchBlank()
{
    auto *w = new CrunchWindow();
    w->resize(1100, 640);
    w->show();
    w->newBlank();   // (what the program does when it is started with no file)
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
        m_app.typedText = [](AppWindow *w) { return asCrunch(w)->isEditing() ? asCrunch(w)->cellEditor()->text() : QString(); };
        m_app.showPrint = [](AppWindow *w) -> QWidget * {
            w->act(QStringLiteral("file.print"))->trigger();
            return w->findChild<FileScreen *>();
        };
        m_app.reopen = [](const QString &path) -> QString {
            const QString ending = QFileInfo(path).suffix().toLower();
            if (ending != QLatin1String("xlsx") && ending != QLatin1String("xlsm")) return {};
            jp::office::Package p;
            QString why;
            if (!p.readLazyFile(path, &why)) return why.isEmpty() ? QStringLiteral("it is not read") : why;
            jp::office::sheet::Workbook b;
            return b.open(p, &why) ? QString() : why.isEmpty() ? QStringLiteral("it is not a workbook") : why;
        };
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
            QString s = QStringLiteral("%1%2%3|%4|%5|%6").arg(w->hasWorkbook()).arg(w->isLoading()).arg(w->isModified()).arg(w->filePath(), w->view()->selectionName()).arg(w->isEditing());
            if (w->session()) {
                QByteArray bytes;
                QString why;
                s += w->snapshotBytesForTest(&bytes, &why) ? QLatin1Char('|') + zipDigest(bytes) : QLatin1Char('|') + why;
            }
            const grid::GridView::ViewState v = w->view()->viewState();
            s += QStringLiteral("|arrows%1|formulas%2|breaks%3,%4|painter%5").arg(w->arrowCount()).arg(w->view()->sheet().showFormulas()).arg(w->view()->pageBreakRows().size()).arg(w->view()->pageBreakCols().size()).arg(w->formatPainterActive());
            return s + QStringLiteral("|%1|%2,%3|%4,%5|%6,%7|%8").arg(w->view()->zoom()).arg(v.frozenRows).arg(v.frozenCols).arg(v.scrollX).arg(v.scrollY).arg(v.gridlines).arg(v.headings).arg(w->view()->rangeName());
        };
        // The sample: a small workbook built here (numbers, text, formulas), written to a file.
        {
            QFile f(tempFile(QStringLiteral("sample.xlsx")));
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(crunchtest::editBook());
        }
        m_app.sampleFile = tempFile(QStringLiteral("sample.xlsx"));
        // The places a person is in when a command is there to use. A blank workbook has nothing in it, so a few cells are typed first (the sample has its own).
        auto fill = [](CrunchWindow *w) {
            if (w->view()->sheet().cellAt(0, 0) != nullptr) return;
            enter(w, 0, 0, QStringLiteral("3"));
            enter(w, 0, 1, QStringLiteral("5"));
            enter(w, 0, 2, QStringLiteral("=A1+B1"));
            enter(w, 1, 0, QStringLiteral("1"));
            enter(w, 1, 1, QStringLiteral("2"));
            enter(w, 1, 2, QStringLiteral("=A2+B2"));
            enter(w, 2, 0, QStringLiteral("text"));
            waitFor([&] { return quiet(w); });
        };
        using Ctx = Adapter::Context;
        auto in = [fill](const QString &name, std::function<void(CrunchWindow *)> enterIt, bool fileOnly = false) {
            return Ctx{name, [fill, enterIt](AppWindow *aw, bool) {
                           auto *w = asCrunch(aw);
                           fill(w);
                           enterIt(w);
                       }, fileOnly};
        };
        m_app.contexts = {
            in(QStringLiteral("a block of cells selected"), [](CrunchWindow *w) { w->view()->selectRange(0, 0, 2, 2); }),
            in(QStringLiteral("whole rows selected"), [](CrunchWindow *w) { w->view()->selectRows(0, 1); }),
            in(QStringLiteral("whole columns selected"), [](CrunchWindow *w) { w->view()->selectColumns(0, 1); }),
            in(QStringLiteral("a formula's cell selected"), [](CrunchWindow *w) { w->view()->selectCell(0, 1); }),
            in(QStringLiteral("cells copied"), [](CrunchWindow *w) {
                w->view()->selectRange(0, 0, 1, 1);
                w->act(QStringLiteral("edit.copy"))->trigger();
                waitFor([&] { return !w->isCopying(); });
                w->view()->selectCell(5, 5);
            }),
            in(QStringLiteral("a cell below and right of the first"), [](CrunchWindow *w) { w->view()->selectCell(3, 2); }),
            in(QStringLiteral("a change to undo"), [](CrunchWindow *w) { enter(w, 4, 4, QStringLiteral("9")); }),
            in(QStringLiteral("a change undone"), [](CrunchWindow *w) {
                enter(w, 4, 4, QStringLiteral("9"));
                waitFor([&] { return quiet(w); });
                w->act(QStringLiteral("edit.undo"))->trigger();
                waitFor([&] { return quiet(w); });
            }),
            in(QStringLiteral("the cursor in a table"), [](CrunchWindow *w) {
                w->setTableMakeAsker([](QString *, bool *headers) {
                    *headers = true;
                    return true;
                });
                w->view()->selectCell(1, 1);
                w->act(QStringLiteral("insert.table"))->trigger();
                waitFor([&] { return quiet(w); });
            }),
            in(QStringLiteral("a chart selected"), [](CrunchWindow *w) {
                w->view()->selectRange(0, 0, 1, 1);
                w->insertChart(jp::office::sheet::ChartKind::Column);
                waitFor([&] { return quiet(w); });
            }),
            in(QStringLiteral("two charts, the later one selected"), [](CrunchWindow *w) {
                w->view()->selectRange(0, 0, 1, 1);
                w->insertChart(jp::office::sheet::ChartKind::Column);
                waitFor([&] { return quiet(w); });
                w->insertChart(jp::office::sheet::ChartKind::Line);
                waitFor([&] { return quiet(w); });
            }),
            in(QStringLiteral("a note in two cells, the later one selected"), [](CrunchWindow *w) {
                w->session()->setNote(0, 0, 0, QStringLiteral("A note"), QStringLiteral("me"), false, QStringLiteral("Add Note"));
                w->session()->setNote(0, 1, 1, QStringLiteral("Another"), QStringLiteral("me"), false, QStringLiteral("Add Note"));
                w->view()->selectCell(1, 1);
            }),
            in(QStringLiteral("a note in two cells, the earlier one selected"), [](CrunchWindow *w) {
                w->session()->setNote(0, 0, 0, QStringLiteral("A note"), QStringLiteral("me"), false, QStringLiteral("Add Note"));
                w->session()->setNote(0, 1, 1, QStringLiteral("Another"), QStringLiteral("me"), false, QStringLiteral("Add Note"));
                w->view()->selectCell(0, 0);
            }),
            in(QStringLiteral("a filter on"), [](CrunchWindow *w) {
                jp::office::sheet::AutoFilter f;
                jp::office::sheet::parseRangeRef(QStringLiteral("A1:C3"), &f.ref);
                jp::office::sheet::FilterColumn c;
                c.column = 0;
                c.kind = jp::office::sheet::FilterColumn::Kind::Values;
                c.values = {QStringLiteral("1"), QStringLiteral("3")};
                f.columns.append(c);
                w->applyFilter(f, QStringLiteral("Filtering"));
                waitFor([&] { return !w->isFiltering() && quiet(w); });
                w->view()->selectCell(0, 0);
            }),
            in(QStringLiteral("a filter on, and a value changed after"), [](CrunchWindow *w) {
                jp::office::sheet::AutoFilter f;
                jp::office::sheet::parseRangeRef(QStringLiteral("A1:C3"), &f.ref);
                jp::office::sheet::FilterColumn c;
                c.column = 0;
                c.kind = jp::office::sheet::FilterColumn::Kind::Values;
                c.values = {QStringLiteral("1"), QStringLiteral("3")};
                f.columns.append(c);
                w->applyFilter(f, QStringLiteral("Filtering"));
                waitFor([&] { return !w->isFiltering() && quiet(w); });
                enter(w, 1, 0, shown(w, 1, 0) == QLatin1String("1") ? QStringLiteral("9") : QStringLiteral("1"));   // (what the filter lets through, or does not: the other)
                waitFor([&] { return quiet(w); });
                w->view()->selectCell(0, 0);
            }),
            in(QStringLiteral("arrows drawn to a formula's cells"), [](CrunchWindow *w) {
                w->view()->selectCell(0, 2);
                w->act(QStringLiteral("formulas.tracePrecedents"))->trigger();
            }),
            in(QStringLiteral("a print area set"), [](CrunchWindow *w) {
                w->view()->selectRange(0, 0, 1, 1);
                w->act(QStringLiteral("printarea.set"))->trigger();
            }),
            in(QStringLiteral("a sheet that runs over a page"), [](CrunchWindow *w) {
                w->setSizeAsker([](CrunchWindow::SizeRequest *r) {
                    r->value = 400;
                    return true;
                });
                w->view()->selectRows(0, 5);
                w->act(QStringLiteral("format.rowHeight"))->trigger();
                waitFor([&] { return quiet(w); });
                // (the page layout, seen once, has the page lines worked out)
                w->act(QStringLiteral("view.pageLayout"))->trigger();
                w->act(QStringLiteral("view.normal"))->trigger();
                waitFor([&] { return quiet(w); });
            }),
            in(QStringLiteral("a cell with a note"), [](CrunchWindow *w) {
                w->session()->setNote(0, 0, 0, QStringLiteral("A note"), QStringLiteral("me"), false, QStringLiteral("Add Note"));
                w->view()->selectCell(0, 0);
            }),
            in(QStringLiteral("a cell with a link"), [](CrunchWindow *w) {
                jp::office::sheet::CellRange cell;
                jp::office::sheet::parseRangeRef(QStringLiteral("B2"), &cell);
                jp::office::sheet::LinkSpec link;
                link.target = QStringLiteral("https://example.org/");
                w->session()->setLink(0, cell, link, QStringLiteral("Link"));
                w->view()->selectCell(1, 1);
            }),
            in(QStringLiteral("a formula that refers to itself"), [](CrunchWindow *w) {
                enter(w, 4, 0, QStringLiteral("=A5+1"));
                waitFor([&] { return quiet(w); });
                w->view()->selectCell(4, 0);
            }),
            in(QStringLiteral("a hidden row and column, around them selected"), [](CrunchWindow *w) {
                w->view()->selectRows(1, 1);
                w->act(QStringLiteral("format.hideRows"))->trigger();
                w->view()->selectColumns(1, 1);
                w->act(QStringLiteral("format.hideColumns"))->trigger();
                w->view()->selectRange(0, 0, 3, 3);
            }),
            in(QStringLiteral("a merged block"), [](CrunchWindow *w) {
                w->setConfirmAsker([](const QString &, const QString &) { return true; });
                w->view()->selectRange(0, 0, 0, 1);
                w->act(QStringLiteral("fmt.merge.cells"))->trigger();
                waitFor([&] { return quiet(w); });
            }),
            in(QStringLiteral("a second sheet"), [](CrunchWindow *w) {
                w->act(QStringLiteral("sheet.insertEnd"))->trigger();
                waitFor([&] { return quiet(w); });
            }),
            in(QStringLiteral("a hidden sheet"), [](CrunchWindow *w) {
                w->act(QStringLiteral("sheet.insertEnd"))->trigger();
                waitFor([&] { return quiet(w); });
                w->act(QStringLiteral("sheet.hide"))->trigger();
                waitFor([&] { return quiet(w); });
            }),
            in(QStringLiteral("the header of a page being typed in"), [](CrunchWindow *w) {
                w->act(QStringLiteral("view.pageLayout"))->trigger();
                auto *pages = w->findChild<grid::PageLayoutView *>();
                if (!pages) return;
                waitFor([&] { return !pages->zoneRect(0, true, 1).isEmpty(); });
                Q_EMIT pages->zoneClicked(0, true, 1);
            }),
            in(QStringLiteral("a cell being typed in"), [](CrunchWindow *w) {
                w->view()->selectCell(5, 5);
                QTest::keyClick(w->view(), Qt::Key_Equal);
            }),
        };
        // (where a command is best tried first: the others come after, so the hints only make the run quicker)
        m_app.hints = {
            {QStringLiteral("hf."), {QStringLiteral("the header of a page being typed in")}},
            {QStringLiteral("notes."), {QStringLiteral("a note in two cells, the later one selected"), QStringLiteral("a note in two cells, the earlier one selected"), QStringLiteral("a cell with a note")}},
            {QStringLiteral("object."), {QStringLiteral("two charts, the later one selected")}},
            {QStringLiteral("chartdesign."), {QStringLiteral("a chart selected")}},
            {QStringLiteral("link."), {QStringLiteral("a cell with a link")}},
            {QStringLiteral("filter."), {QStringLiteral("a filter on"), QStringLiteral("a filter on, and a value changed after")}},
            {QStringLiteral("printarea."), {QStringLiteral("a print area set")}},
            {QStringLiteral("sheet."), {QStringLiteral("a second sheet"), QStringLiteral("a hidden sheet")}},
            {QStringLiteral("formulas.trace"), {QStringLiteral("a formula's cell selected")}},
            {QStringLiteral("formulas.removeArrows"), {QStringLiteral("arrows drawn to a formula's cells")}},
            {QStringLiteral("formulas.circular"), {QStringLiteral("a formula that refers to itself")}},
            {QStringLiteral("format.unhide"), {QStringLiteral("a hidden row and column, around them selected")}},
            {QStringLiteral("fmt.merge"), {QStringLiteral("a merged block"), QStringLiteral("a block of cells selected")}},
            {QStringLiteral("table."), {QStringLiteral("the cursor in a table")}},
            {QStringLiteral("view.pageBreaks"), {QStringLiteral("a sheet that runs over a page")}},
        };
        m_app.skipped = {};
        // What a command may leave as it is, where what it does has nothing to do.
        const QString menu = QStringLiteral("the name of a menu: its choices are the commands, and it has none of its own");
        m_app.quiet = {
            {QStringLiteral("cells.format"), menu},
            {QStringLiteral("formulas.options"), menu},
            {QStringLiteral("sheet.tabColor"), menu},
            {QStringLiteral("sheet.unhide"), menu},
        };
        m_app.neverRun = {};
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
        std::unique_ptr<CrunchWindow> w(launchBlank());
        QStringList told;
        w->setNotifier([&](const QString &title, const QString &text) { told << title + QLatin1String(": ") + text; });
        // A new window holds a workbook that is ready, named for what it is.
        QVERIFY2(w->hasWorkbook(), "a window with no file holds no workbook, only an empty grid");
        QVERIFY(w->session() != nullptr);
        QVERIFY2(w->windowTitle().contains(w->displayName()), qPrintable(w->windowTitle()));
        QVERIFY(w->windowTitle().contains(QLatin1String("JeffCrunch")));
        QVERIFY2(!saysNotYet(w->statusText()), qPrintable(QStringLiteral("the status line says ") + w->statusText()));
        QVERIFY(!w->isModified());

        // Typing goes into the cells, a formula works, and the title shows the change.
        enter(w.get(), 0, 0, QStringLiteral("21"));
        QVERIFY2(w->isModified(), "typing into a window with no file changes nothing");
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

        // File > New: a second window with a blank workbook of its own that takes typing.
        const int windows = AppWindow::windows().size();
        QAction *fileNew = again->act(QStringLiteral("file.new"));
        QVERIFY2(fileNew, "JeffCrunch has no File > New");
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

        // The print layout: the File screen's Print page, with the sheet drawn (not a blank one).
        QVERIFY2(previewShowsTheDocument(m_app, again.get()), "the print preview shows no more than a blank page");
        auto *screen = again->findChild<FileScreen *>();
        QVERIFY(screen && screen->isVisible());
        QCOMPARE(screen->currentPage(), QStringLiteral("print"));
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
        for (bool fromFile : {false, true}) {
            std::unique_ptr<AppWindow> w(fromFile ? m_app.launchWith(m_app.sampleFile) : m_app.launch());
            QVERIFY(w);
            m_app.settle(w.get());
            const Outcome o = observe(m_app, w.get(), nullptr, watch, [] {}, false, 3000);
            QVERIFY2(!o.changed, qPrintable(o.how + o.detail));
        }
    }

    // No command on the ribbon is a stub that never works (a button that is dimmed for good, its tip saying it is not there yet).
    void noRibbonCommandIsAStub()
    {
        std::unique_ptr<AppWindow> w(m_app.launch());
        QStringList stubs;
        for (const QToolButton *b : w->ribbon()->findChildren<QToolButton *>())
            if (const QAction *a = b->defaultAction(); a && a->toolTip().contains(QLatin1String("not available in JeffCrunch yet"))) stubs << a->objectName();
        QVERIFY2(stubs.isEmpty(), qPrintable(stubs.join(QLatin1Char(' '))));
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
        QStringList fails = checkNeverRun(m_app, &report);
        // And none of them is on the ribbon, where a button that never works is a stub.
        std::unique_ptr<AppWindow> w(m_app.launch());
        for (const QToolButton *b : w->ribbon()->findChildren<QToolButton *>())
            if (const QAction *a = b->defaultAction(); a && m_app.allIds.contains(a->objectName()) && !m_app.ran.contains(a->objectName()) && !m_app.skipped.contains(a->objectName()))
                fails << QStringLiteral("%1 is on the ribbon, and no test could ever enable it").arg(a->objectName());
        qInfo("%s", qPrintable(report.first()));
        if (qEnvironmentVariableIsSet("JP_CHECKS_REPORT")) qInfo("%s", qPrintable(report.join('\n')));
        QVERIFY2(fails.isEmpty(), qPrintable(fails.join('\n')));
    }
};

JP_TEST_CLASS(CrunchAppChecks)
#include "tst_appchecks_crunch.moc"
