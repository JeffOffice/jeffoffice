// Tests of JeffCrunch's window (apps/crunch): opening a workbook on a worker with the first rows on the screen early, the calculation after the load, the sheet tabs, the name box and
// the formula bar, the commands that work and those that are dimmed, copying, the view commands, the File screen, Help, and a screenshot of a workbook that looks like a real one. Every
// workbook is built here, in a temporary folder.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/filescreen.h"
#include "app/help.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "app/sheettabs.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "grid/gridview.h"

#include <QApplication>
#include <QClipboard>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSettings>
#include <QSignalSpy>
#include <QTextDocument>

using namespace jp;
using namespace crunchtest;

namespace {

// A window with the notifier standing in for message boxes: what it was told is kept.
struct Window {
    CrunchWindow w;
    QStringList told;
    Window()
    {
        w.setNotifier([this](const QString &title, const QString &text) { told << title + QStringLiteral(": ") + text; });
        w.resize(1100, 640);
        w.show();
    }
    // Opens a workbook and waits until it is read and every formula has its value.
    bool open(const QString &path)
    {
        QSignalSpy first(&w, &CrunchWindow::firstScreenShown), done(&w, &CrunchWindow::calculationFinished), loaded(&w, &CrunchWindow::loadFinished);
        if (!w.openFile(path)) return false;
        return first.wait(20000) && (loaded.count() > 0 || loaded.wait(20000)) && (done.count() > 0 || done.wait(20000));
    }
};

} // namespace

class CrunchWindowTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        // (the settings this run changes are its own)
        static QTemporaryDir settings;
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        setAppInfo(crunchAppInfo());
        initCore();   // the program's own fonts, as JeffCrunch loads them
    }

    void itsIdentityIsItsOwn()
    {
        QCOMPARE(appInfo().name, QStringLiteral("JeffCrunch"));
        QCOMPARE(appInfo().accent, QColor(0x13, 0x77, 0x3A));
        QCOMPARE(appInfo().badge, QStringLiteral("JC"));
        QCOMPARE(appInfo().helpFolder, QStringLiteral("crunch"));
    }

    // The window has its four tabs and builds clean; the commands that work are on, and those that come with editing are on the ribbon, dimmed.
    void theWindowHasItsTabsAndItsCommands()
    {
        CrunchWindow w;
        QCOMPARE(w.ribbonErrors(), QString());
        QStringList tabs;
        for (int i = 0; i < w.ribbon()->tabCount(); ++i) tabs << w.ribbon()->tabName(i);
        QCOMPARE(tabs, (QStringList{"Home", "Insert", "Formulas", "View"}));
        QCOMPARE(w.ribbon()->fileKeytip(), QStringLiteral("F"));
        QVERIFY(w.windowTitle().contains(QLatin1String("JeffCrunch")));
        QVERIFY(w.windowTitle().startsWith(QLatin1String("Book")));
        for (const char *id : {"file.open", "file.close", "file.options", "edit.copy", "view.gridlines", "view.headings", "view.formulaBar", "view.formulas", "view.freeze", "view.unfreeze",
                               "zoom.in", "zoom.out", "zoom.100", "formulas.calculate", "help.show"}) {
            QVERIFY2(w.act(QString::fromLatin1(id)), id);
            QVERIFY2(w.act(QString::fromLatin1(id))->isEnabled(), id);
        }
        for (const char *id : {"edit.paste", "edit.cut", "edit.undo", "edit.redo", "fmt.bold", "fmt.italic", "fmt.alignCenter", "fmt.merge", "cells.insert", "edit.sort", "insert.chart", "insert.table",
                               "formulas.insertFunction"}) {
            QVERIFY2(w.act(QString::fromLatin1(id)), id);
            QVERIFY2(!w.act(QString::fromLatin1(id))->isEnabled(), id);
        }
        // every command the ribbon names is one of the window's
        QFile json(QStringLiteral(":/crunch/ribbon.json"));
        QVERIFY(json.open(QIODevice::ReadOnly));
        const QByteArray text = json.readAll();
        static const QRegularExpression cmd(QStringLiteral("\"(?:large|small|icon|cmd)\"\\s*:\\s*\"([a-z]+\\.[A-Za-z0-9]+)\""));
        int found = 0;
        for (auto it = cmd.globalMatch(QString::fromUtf8(text)); it.hasNext();) {
            const QString id = it.next().captured(1);
            QVERIFY2(w.act(id), qPrintable(id));
            ++found;
        }
        QVERIFY(found > 40);
        // the ribbon's text is the list translators start from
        QFile listed(QStringLiteral(JC_SOURCE_DIR "/apps/crunch/app/ribbon_strings.cpp"));
        QVERIFY(listed.open(QIODevice::ReadOnly));
        QSet<QString> want, have;
        std::function<void(const QJsonValue &)> walk = [&](const QJsonValue &v) {
            if (v.isObject()) {
                const QJsonObject o = v.toObject();
                for (auto it = o.begin(); it != o.end(); ++it) {
                    if (it.value().isString() && QSet<QString>{"name", "text", "launcherTip", "tip", "label", "section", "submenu", "context"}.contains(it.key()) && !it.value().toString().isEmpty())
                        want.insert(it.value().toString());
                    walk(it.value());
                }
            } else if (v.isArray()) {
                for (const QJsonValue &x : v.toArray()) walk(x);
            }
        };
        walk(QJsonDocument::fromJson(text).object());
        static const QRegularExpression re(QStringLiteral("QT_TRANSLATE_NOOP\\(\"Ribbon\", \"([^\"]*)\"\\)"));
        for (auto it = re.globalMatch(QString::fromUtf8(listed.readAll())); it.hasNext();) have.insert(it.next().captured(1));
        QCOMPARE(have, want);
    }

    // A workbook opens on a worker: the first rows are on the screen while the rest is read, with the cell the file was left on selected, and then the whole workbook is in.
    void aWorkbookOpensWithItsFirstScreenEarly()
    {
        Folder folder;
        const QString path = folder.write(QStringLiteral("budget.xlsx"), budgetWorkbook());
        Window win;
        QSignalSpy first(&win.w, &CrunchWindow::firstScreenShown), loaded(&win.w, &CrunchWindow::loadFinished), done(&win.w, &CrunchWindow::calculationFinished);
        QVERIFY(win.w.openFile(path));
        QVERIFY(win.w.isLoading());
        QVERIFY(first.wait(20000));
        // the first rows are in the grid, whether or not the rest has been read
        QVERIFY(win.w.hasWorkbook());
        QCOMPARE(win.w.view()->selectionName(), QStringLiteral("B3"));
        QCOMPARE(win.w.formulaBar()->text(), QStringLiteral("100"));
        QCOMPARE(win.w.nameBox()->text(), QStringLiteral("B3"));
        QVERIFY(win.w.windowTitle().startsWith(QLatin1String("budget.xlsx")));
        QVERIFY(win.w.view()->sheet().data().sheet != nullptr);
        QVERIFY(loaded.count() <= 1);
        QVERIFY(loaded.count() > 0 || loaded.wait(20000));
        QVERIFY(done.count() > 0 || done.wait(20000));
        QVERIFY(!win.w.isLoading() && !win.w.isCalculating());
        QCOMPARE(loaded.first().first().toBool(), true);
        QVERIFY(win.w.calculation() != nullptr);
        QCOMPARE(win.w.statusText(), QStringLiteral("Ready"));
        // the grid's geometry of each sheet was made by the worker (the chart sheet of a workbook has none)
        QCOMPARE(int(win.w.loaded()->geometry.size()), 3);
        for (const auto &g : win.w.loaded()->geometry) QVERIFY(g);
        // the file's own view is kept: the panes it froze, and the cell it was left on
        const grid::GridLayout &layout = win.w.view()->sheet().layout();
        QCOMPARE(layout.frozenRows(), qint64(2));
        QCOMPARE(layout.frozenCols(), qint64(1));
        QCOMPARE(win.w.view()->selectionName(), QStringLiteral("B3"));
        QVERIFY(win.told.isEmpty());
        QCOMPARE(win.w.filePath(), path);
    }

    // After the load every formula is worked out again: the totals the file stored as 0 are the sums, and the formula bar shows the formula.
    void formulasAreCalculatedAfterTheLoad()
    {
        Folder folder;
        Window win;
        QVERIFY(win.open(folder.write(QStringLiteral("budget.xlsx"), budgetWorkbook())));
        grid::GridView *v = win.w.view();
        // B9 is the total of B3:B8: 100 + 137 + 174 + 211 + 248 + 285 = 1155
        v->selectCell(8, 1);
        QCOMPARE(v->activeCellText(), QStringLiteral("=SUM(B3:B8)"));
        QCOMPARE(win.w.formulaBar()->text(), QStringLiteral("=SUM(B3:B8)"));
        const grid::Cell *b9 = v->sheet().cellAt(8, 1);
        QVERIFY(b9);
        QCOMPARE(v->sheet().valueOf(*b9).debug(), QStringLiteral("n:1155"));
        QCOMPARE(v->sheet().shown(8, 1, *b9).text, QStringLiteral("$1,155.00"));
        // B11 = C9/B9 - 1, as a percent
        const grid::Cell *b11 = v->sheet().cellAt(10, 1);
        QVERIFY(b11);
        QCOMPARE(v->sheet().shown(10, 1, *b11).text, QStringLiteral("6.0%"));
        // the date, and the number too wide for its cell
        QCOMPARE(v->sheet().shown(10, 2, *v->sheet().cellAt(10, 2)).text, QStringLiteral("Mar 19, 2026"));
        QVERIFY(v->sheet().shown(10, 3, *v->sheet().cellAt(10, 3)).hashes);
        // Calculate Now works every formula out again
        QSignalSpy done(&win.w, &CrunchWindow::calculationFinished);
        win.w.act("formulas.calculate")->trigger();
        QVERIFY(done.wait(20000));
        QCOMPARE(v->sheet().valueOf(*v->sheet().cellAt(8, 1)).debug(), QStringLiteral("n:1155"));
    }

    // A file that is not a workbook is said to be unreadable, and the window stays empty and ready for another.
    void aFileThatIsNotAWorkbookIsRefused()
    {
        Folder folder;
        Window win;
        QSignalSpy loaded(&win.w, &CrunchWindow::loadFinished);
        QVERIFY(win.w.openFile(folder.write(QStringLiteral("broken.xlsx"), QByteArray("this is not a zip file at all"))));
        QVERIFY(loaded.wait(20000));
        QCOMPARE(loaded.first().first().toBool(), false);
        QVERIFY(!win.w.hasWorkbook() && !win.w.isLoading());
        QCOMPARE(win.told.size(), 1);
        QVERIFY2(win.told.first().contains(QLatin1String("broken.xlsx")), qPrintable(win.told.first()));
        QVERIFY(win.w.filePath().isEmpty());
        // a file that isn't there
        QVERIFY(!win.w.openFile(folder.dir.filePath(QStringLiteral("nothing.xlsx"))));
        QCOMPARE(win.told.size(), 2);
        // and the same window then opens a good one
        QVERIFY(win.open(folder.write(QStringLiteral("budget.xlsx"), budgetWorkbook())));
        QVERIFY(win.w.hasWorkbook());
    }

    // The tabs are the sheets that are shown (a hidden one is kept out); another sheet is shown from its tab, and each sheet keeps where the person was in it.
    void sheetTabsShowTheShownSheetsAndKeepEachOnesPlace()
    {
        Folder folder;
        Window win;
        QVERIFY(win.open(folder.write(QStringLiteral("budget.xlsx"), budgetWorkbook())));
        crunch::SheetTabBar *tabs = win.w.sheetTabs();
        QCOMPARE(tabs->count(), 2);
        QCOMPARE(tabs->tabText(0), QStringLiteral("Budget"));
        QCOMPARE(tabs->tabText(1), QStringLiteral("Notes"));
        QCOMPARE(win.w.shownSheets(), (QVector<int>{0, 2}));
        QVERIFY(tabs->stripe(0).isValid());   // (the color the Budget sheet asks for)
        QCOMPARE(tabs->stripe(0), QColor(0x13, 0x77, 0x3A));
        QVERIFY(!tabs->stripe(1).isValid());
        // in the Budget: select a cell, scroll
        win.w.view()->selectCell(5, 3);
        QCOMPARE(win.w.view()->selectionName(), QStringLiteral("D6"));
        tabs->setCurrentIndex(1);
        QCOMPARE(win.w.activeSheet(), 2);
        QCOMPARE(win.w.view()->selectionName(), QStringLiteral("A1"));
        QCOMPARE(win.w.view()->activeCellText(), QStringLiteral("other"));
        win.w.view()->selectCell(1, 0);
        QCOMPARE(win.w.view()->activeCellText(), QStringLiteral("3"));
        // back to the Budget: where the person was is where it is
        tabs->setCurrentIndex(0);
        QCOMPARE(win.w.activeSheet(), 0);
        QCOMPARE(win.w.view()->selectionName(), QStringLiteral("D6"));
        QCOMPARE(win.w.view()->sheet().layout().frozenRows(), qint64(2));
        // and the other sheet too
        tabs->setCurrentIndex(1);
        QCOMPARE(win.w.view()->selectionName(), QStringLiteral("A2"));
        win.w.setActiveSheet(0);
        QCOMPARE(tabs->currentIndex(), 0);
    }

    // The sheet shown first is the one that was open when the workbook was saved; if that one is hidden (or is a chart), the first that is shown.
    void theSheetShownFirstIsTheOneThatWasOpen()
    {
        const QByteArray sheet = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"));
        Folder folder;
        {
            Window win;
            QVERIFY(win.open(folder.write(QStringLiteral("third.xlsx"), buildXlsx({sheet, sheet, sheet}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), QString(), QByteArray(),
                                                                                 QStringLiteral("<workbookView activeTab=\"2\"/>")))));
            QCOMPARE(win.w.activeSheet(), 2);
            QCOMPARE(win.w.sheetTabs()->currentIndex(), 2);
        }
        {
            Window win;
            QVERIFY(win.open(folder.write(QStringLiteral("hidden.xlsx"), buildXlsx({sheet, sheet, sheet}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), QString(), QByteArray(),
                                                                                  QStringLiteral("<workbookView activeTab=\"0\"/>"), {0}))));
            QCOMPARE(win.w.activeSheet(), 1);   // (the first sheet is hidden: the first that is shown)
            QCOMPARE(win.w.sheetTabs()->count(), 2);
        }
        {
            Window win;
            QVERIFY(win.open(folder.write(QStringLiteral("none.xlsx"), buildXlsx({sheet}, QByteArray(), QString(), QString(), QByteArray(), QStringList(), QString(), QByteArray(), QString(), {0}))));
            QVERIFY(win.w.hasWorkbook());   // (every sheet hidden: the window still opens it)
            QCOMPARE(win.w.sheetTabs()->count(), 0);
        }
    }

    // The name box goes to an address (a cell, a range, columns, rows); one that is no address says so and keeps the selection.
    void theNameBoxGoesToAnAddress()
    {
        Folder folder;
        Window win;
        QVERIFY(win.open(folder.write(QStringLiteral("budget.xlsx"), budgetWorkbook())));
        auto go = [&](const QString &text) {
            win.w.nameBox()->setText(text);
            QTest::keyClick(win.w.nameBox(), Qt::Key_Return);
        };
        go(QStringLiteral("e7"));
        QCOMPARE(win.w.view()->selectionName(), QStringLiteral("E7"));
        QCOMPARE(win.w.view()->rangeName(), QStringLiteral("E7"));
        go(QStringLiteral("B3:D6"));
        QCOMPARE(win.w.view()->rangeName(), QStringLiteral("B3:D6"));
        go(QStringLiteral("C:E"));
        QCOMPARE(win.w.view()->rangeName(), QStringLiteral("C:E"));
        go(QStringLiteral("4:9"));
        QCOMPARE(win.w.view()->rangeName(), QStringLiteral("4:9"));
        go(QStringLiteral("not an address"));
        QCOMPARE(win.w.view()->rangeName(), QStringLiteral("4:9"));
        QVERIFY(win.w.statusText().contains(QLatin1String("not an address")));
        QCOMPARE(win.w.nameBox()->text(), win.w.view()->selectionName());
    }

    // Copy puts what the selected cells show on the clipboard, as text with tabs and lines.
    void copyPutsTheSelectionOnTheClipboardAsText()
    {
        Folder folder;
        Window win;
        QVERIFY(win.open(folder.write(QStringLiteral("budget.xlsx"), budgetWorkbook())));
        win.w.view()->selectRange(1, 0, 3, 2);   // A2:C4
        win.w.act("edit.copy")->trigger();
        QTRY_COMPARE(QApplication::clipboard()->text(), QStringLiteral("Category\tJan\tFeb\nRent\t$100.00\t$111.50\nGroceries\t$137.00\t$148.50\n"));
    }

    // The view commands: gridlines, headings, the formula bar, show formulas, zoom, and freezing panes.
    void theViewCommandsChangeTheView()
    {
        Folder folder;
        Window win;
        QVERIFY(win.open(folder.write(QStringLiteral("budget.xlsx"), budgetWorkbook())));
        grid::GridView *v = win.w.view();
        win.w.act("view.gridlines")->trigger();
        QVERIFY(!v->showGridlines());
        win.w.act("view.gridlines")->trigger();
        QVERIFY(v->showGridlines());
        win.w.act("view.headings")->trigger();
        QVERIFY(!v->showHeadings());
        QCOMPARE(v->sheet().layout().headerWidth(), 0);
        win.w.act("view.headings")->trigger();
        QVERIFY(v->sheet().layout().headerWidth() > 0);
        QVERIFY(win.w.formulaBar()->isVisible());
        win.w.act("view.formulaBar")->trigger();
        QVERIFY(!win.w.formulaBar()->isVisible());
        win.w.act("view.formulaBar")->trigger();
        QVERIFY(win.w.formulaBar()->isVisible());
        // show formulas
        win.w.act("view.formulas")->trigger();
        QCOMPARE(v->sheet().shown(8, 1, *v->sheet().cellAt(8, 1)).text, QStringLiteral("=SUM(B3:B8)"));
        win.w.act("view.formulas")->trigger();
        QCOMPARE(v->sheet().shown(8, 1, *v->sheet().cellAt(8, 1)).text, QStringLiteral("$1,155.00"));
        // zoom
        win.w.act("zoom.in")->trigger();
        QCOMPARE(v->zoom(), 1.1);
        win.w.act("zoom.200")->trigger();
        QCOMPARE(v->zoom(), 2.0);
        win.w.act("zoom.100")->trigger();
        QCOMPARE(v->zoom(), 1.0);
        win.w.act("zoom.out")->trigger();
        QCOMPARE(v->zoom(), 0.9);
        // freeze: the rows above the selected cell and the columns to its left; unfreeze
        v->selectCell(4, 2);
        win.w.act("view.freeze")->trigger();
        QCOMPARE(v->sheet().layout().frozenRows(), qint64(4));
        QCOMPARE(v->sheet().layout().frozenCols(), qint64(2));
        win.w.act("view.unfreeze")->trigger();
        QCOMPARE(v->sheet().layout().frozenRows(), qint64(0));
        win.w.act("view.freezeRow")->trigger();
        QCOMPARE(v->sheet().layout().frozenRows(), qint64(1));
        win.w.act("view.freezeCol")->trigger();
        QCOMPARE(v->sheet().layout().frozenCols(), qint64(1));
        // the numbers of the selection reach the status bar
        v->selectRange(2, 1, 4, 2);
        QTRY_VERIFY(win.w.findChild<QLabel *>(QStringLiteral("selectionStats"))->text().startsWith(QLatin1String("Average: ")));
    }

    // The File screen has its Open and About pages (Open lists the workbooks opened lately), and Esc closes it; Help opens on the topic for the page.
    void theFileScreenAndHelp()
    {
        Folder folder;
        Window win;
        const QString path = folder.write(QStringLiteral("budget.xlsx"), budgetWorkbook());
        QVERIFY(win.open(path));
        win.w.showFileScreen(QStringLiteral("open"));
        auto *screen = win.w.findChild<FileScreen *>();
        QVERIFY(screen && screen->isVisible());
        QCOMPARE(screen->currentPage(), QStringLiteral("open"));
        // the recent workbooks include the one just opened
        bool listed = false;
        for (QListWidget *list : screen->findChildren<QListWidget *>())
            for (int i = 0; i < list->count(); ++i) listed = listed || list->item(i)->data(Qt::UserRole).toString() == path;
        QVERIFY(listed);
        win.w.showFileScreen(QStringLiteral("about"));
        QCOMPARE(screen->currentPage(), QStringLiteral("about"));
        QVERIFY(!screen->findChildren<QLabel *>().isEmpty());
        QTest::keyClick(&win.w, Qt::Key_Escape);
        QVERIFY(!screen->isVisible());
        // Help
        win.w.showHelp(QStringLiteral("view"));
        QVERIFY(win.w.helpView());
        QCOMPARE(win.w.helpView()->currentTopic(), QStringLiteral("view"));
        win.w.act("help.show")->trigger();
        QVERIFY(win.w.helpView()->isVisible());
    }

    // Help's topics are complete: each has a title and keywords, links to topics that are there, a place in the contents, and enough text; each ribbon tab and File page has its topic.
    void helpTopicsAreComplete()
    {
        const QString src = QStringLiteral(JC_SOURCE_DIR "/resources/help");
        QStringList onDisk, built;
        for (QDirIterator it(src, QDir::Files, QDirIterator::Subdirectories); it.hasNext();) onDisk << QDir(src).relativeFilePath(it.next());
        for (QDirIterator it(QStringLiteral(":/help"), QDir::Files, QDirIterator::Subdirectories); it.hasNext();) built << it.next().mid(7);
        onDisk.sort();
        built.sort();
        QCOMPARE(built, onDisk);   // resources.qrc lists every file in resources/help (python3 tools/help_files.py)
        const QStringList ids = help::topicIds();
        QVERIFY2(ids.size() >= 7, qPrintable(QString::number(ids.size())));
        static const QRegularExpression link(QStringLiteral("\\]\\(([^)]+)\\)"));
        static const QRegularExpression banned(QStringLiteral("\\b(Microsoft|Publisher|Office|Excel|Word|PowerPoint)\\b"));
        const help::Topic contents = help::topic(QStringLiteral("index"));
        QStringList problems;
        for (const QString &id : ids) {
            const help::Topic t = help::topic(id);
            if (t.title.isEmpty()) problems << id + QStringLiteral(": no title");
            if (t.keywords.isEmpty()) problems << id + QStringLiteral(": no keywords");
            if (const auto m = banned.match(t.markdown); m.hasMatch()) problems << id + QStringLiteral(": says ") + m.captured(1);
            for (auto m = link.globalMatch(t.markdown); m.hasNext();) {
                const QString target = m.next().captured(1);
                if (!target.startsWith(QLatin1String("https://")) && !ids.contains(target)) problems << id + QStringLiteral(": link to ") + target;
            }
            if (id != QLatin1String("index") && !contents.markdown.contains(QStringLiteral("](%1)").arg(id))) problems << id + QStringLiteral(": not in the contents");
            QTextDocument doc;
            doc.setMarkdown(help::displayMarkdown(t, nullptr), QTextDocument::MarkdownDialectGitHub);
            if (doc.toPlainText().trimmed().size() < 100) problems << id + QStringLiteral(": nearly empty");
        }
        QFile cf(src + QStringLiteral("/en/crunch/context.json"));
        QVERIFY(cf.open(QIODevice::ReadOnly));
        const QJsonObject context = QJsonDocument::fromJson(cf.readAll()).object();
        for (auto it = context.begin(); it != context.end(); ++it)
            if (!ids.contains(it.value().toString())) problems << QStringLiteral("context %1: no topic %2").arg(it.key(), it.value().toString());
        CrunchWindow w;
        for (int i = 0; i < w.ribbon()->tabCount(); ++i)
            if (help::contextTopic(QStringLiteral("tab:") + w.ribbon()->tabName(i)).isEmpty()) problems << QStringLiteral("no topic for the tab ") + w.ribbon()->tabName(i);
        for (const char *page : {"open", "about"})
            if (help::contextTopic(QStringLiteral("file:") + QLatin1String(page)).isEmpty()) problems << QStringLiteral("no topic for the File page ") + QLatin1String(page);
        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QStringLiteral("; "))));
        // the keyboard topic's table comes from the commands themselves
        const QString keys = help::displayMarkdown(help::topic(QStringLiteral("keyboard")), &w);
        QVERIFY(!keys.contains(QLatin1String("<!--")));
        QVERIFY(keys.contains(QLatin1String("Ctrl+O")));
    }

    // Closing a window that is still reading a large workbook stops the read and does not wait for it; the program goes on.
    void closingWhileLoadingStopsTheRead()
    {
        // a workbook with 60,000 rows of 20 numbers
        QString rows;
        rows.reserve(60000 * 400);
        for (int r = 1; r <= 60000; ++r) {
            rows += QStringLiteral("<row r=\"%1\">").arg(r);
            for (int c = 0; c < 20; ++c) rows += QStringLiteral("<c r=\"%1%2\"><v>%3</v></c>").arg(QChar('A' + c)).arg(r).arg(r * 20 + c);
            rows += QStringLiteral("</row>");
        }
        Folder folder;
        const QString path = folder.write(QStringLiteral("big.xlsx"), buildXlsx({worksheet(rows)}));
        QElapsedTimer t;
        t.start();
        auto *w = new CrunchWindow();
        w->setNotifier([](const QString &, const QString &) {});
        w->show();
        QSignalSpy first(w, &CrunchWindow::firstScreenShown);
        QVERIFY(w->openFile(path));
        QVERIFY(first.wait(20000));
        QElapsedTimer closing;
        closing.start();
        w->close();
        delete w;
        QVERIFY2(closing.elapsed() < 1000, qPrintable(QString::number(closing.elapsed())));   // (the read has seconds to go)
        QTest::qWait(100);
    }

    // A workbook made to be hostile (every column a character wide, rows of 0 and of 4,000 points, a hundred thousand merged ranges, a hundred thousand cell formats, texts of
    // 32,000 characters) opens, and its screen is drawn within the bounds, whatever the file says.
    void aHostileWorkbookIsDrawnWithinTheBounds()
    {
        QString cols;
        for (int c = 1; c <= 16384; ++c) cols += QStringLiteral("<col min=\"%1\" max=\"%1\" width=\"%2\" customWidth=\"1\"/>").arg(c).arg(c % 7 == 0 ? 0.0 : 1.0);
        QString rows;
        const QString huge(32000, QLatin1Char('x'));
        for (int r = 1; r <= 300; ++r) {
            rows += QStringLiteral("<row r=\"%1\"%2>").arg(r).arg(r == 1 ? QStringLiteral(" ht=\"0\" customHeight=\"1\"") : r == 2 ? QStringLiteral(" ht=\"4000\" customHeight=\"1\"") : QString());
            for (int c = 0; c < 20; ++c) {
                const QString ref = QStringLiteral("%1%2").arg(QChar('A' + c)).arg(r);
                const int style = (r * 20 + c) % 100000;
                if (r % 5 == 0 && c < 3) rows += QStringLiteral("<c r=\"%1\" s=\"%2\" t=\"inlineStr\"><is><t>%3</t></is></c>").arg(ref).arg(style).arg(huge);
                else rows += QStringLiteral("<c r=\"%1\" s=\"%2\"><v>%3</v></c>").arg(ref).arg(style).arg(r * c);
            }
            rows += QStringLiteral("</row>");
        }
        QString merges = QStringLiteral("<mergeCells count=\"100000\">");
        for (int r = 1; r <= 100000; ++r) merges += QStringLiteral("<mergeCell ref=\"A%1:B%1\"/>").arg(r);
        merges += QStringLiteral("</mergeCells>");
        QString xfs;
        for (int i = 0; i < 100000; ++i) xfs += QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"><alignment indent=\"%1\"/></xf>").arg(i % 16);
        Folder folder;
        const QByteArray xlsx = buildXlsx({worksheet(rows, merges, QStringLiteral("<cols>") + cols + QStringLiteral("</cols>"))}, QByteArray(), QString(), QString(),
                                          stylesXml(QString(), xfs, QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/></font></fonts>")));
        Window win;
        QVERIFY(win.open(folder.write(QStringLiteral("hostile.xlsx"), xlsx)));
        QVERIFY2(win.told.isEmpty(), qPrintable(win.told.join('\n')));
        QElapsedTimer t;
        t.start();
        win.w.view()->viewport()->repaint();
        QVERIFY2(t.elapsed() < 2000, qPrintable(QString::number(t.elapsed())));
        const grid::PaintStats s = win.w.view()->lastPaintStats();
        QVERIFY(s.rowsVisited <= 4 * grid::GridPainter::kMaxRows && s.colsVisited <= 4 * grid::GridPainter::kMaxCols);
        QVERIFY(s.cellsLooked <= 4 * grid::GridPainter::kMaxCells * 3);
        QVERIFY(s.textChars <= qint64(grid::GridPainter::kMaxTextRuns) * 1024);
        QVERIFY(s.mergesDrawn <= 4 * grid::GridPainter::kMaxMerges && s.mergeMarks <= 4 * grid::GridPainter::kMaxMergeMarks);
        QVERIFY(s.looksMade <= 4 * grid::GridPainter::kMaxCells);
        // the rows of 0 and of 4,000 points are the hidden row and the tallest row the grid allows
        const grid::GridLayout &L = win.w.view()->sheet().layout();
        QVERIFY(L.rows.isHidden(0));
        QCOMPARE(L.rows.size(1), 546);
        QVERIFY(L.cols.size(6) == 0 && L.cols.size(0) > 0);
        // every key and a scroll to the end of the sheet are as quick
        QTest::keyClick(win.w.view(), Qt::Key_End, Qt::ControlModifier);
        QTest::keyClick(win.w.view(), Qt::Key_PageDown);
        win.w.view()->verticalScrollBar()->setValue(win.w.view()->verticalScrollBar()->maximum());
        win.w.view()->grab();
        QVERIFY(win.w.view()->lastPaintStats().rowsVisited <= 4 * grid::GridPainter::kMaxRows);
    }

    // Esc stops the calculation: what is not worked out keeps the result the file stored; the window says so and is ready.
    void escStopsTheCalculation()
    {
        QString rows;
        rows.reserve(200000 * 90);
        for (int r = 1; r <= 200000; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*2</f><v>-1</v></c></row>").arg(r);
        Folder folder;
        Window win;
        QSignalSpy loaded(&win.w, &CrunchWindow::loadFinished), done(&win.w, &CrunchWindow::calculationFinished);
        QVERIFY(win.w.openFile(folder.write(QStringLiteral("formulas.xlsx"), buildXlsx({worksheet(rows)}))));
        QVERIFY(loaded.wait(60000));
        QVERIFY(win.w.isCalculating());   // (two hundred thousand formulas take longer than a slice)
        QTest::keyClick(&win.w, Qt::Key_Escape);
        QVERIFY(!win.w.isCalculating());
        QCOMPARE(win.w.statusText(), QStringLiteral("Calculation stopped"));
        QTest::qWait(50);
        QCOMPARE(done.count(), 0);
        // the cells not reached show what the file stored
        const grid::GridView *v = win.w.view();
        const grid::Cell *last = v->sheet().cellAt(199999, 1);
        QVERIFY(last);
        QVERIFY(v->sheet().valueOf(*last).debug() == QStringLiteral("n:-1") || v->sheet().valueOf(*last).debug() == QStringLiteral("n:399998"));
        // and Calculate Now works everything out again
        win.w.act("formulas.calculate")->trigger();
        QVERIFY(done.wait(60000));
        QCOMPARE(v->sheet().valueOf(*v->sheet().cellAt(199999, 1)).debug(), QStringLiteral("n:400000"));
    }

    // The window drawn with a workbook that looks like a real one: the title merged over the columns, a green header and a frozen row and column, fills and borders, number formats, a
    // number too wide for its cell as "#", wrapped notes. The picture is saved when JC_SCREENSHOT names a file.
    void theWindowShowsAWorkbookThatLooksLikeARealOne()
    {
        Folder folder;
        Window win;
        win.w.resize(1100, 640);
        QVERIFY(win.open(folder.write(QStringLiteral("budget.xlsx"), budgetWorkbook())));
        win.w.view()->setZoom(1.0);
        QApplication::processEvents();
        QImage shot = win.w.grab().toImage();
        QVERIFY(!shot.isNull());
        const grid::GridLayout &L = win.w.view()->sheet().layout();
        const QPoint topLeft = win.w.view()->viewport()->mapTo(&win.w, QPoint(0, 0));
        const QRect title = L.cellRect(0, 0).translated(topLeft);
        // the merged title is one green rectangle over the columns A to G
        QCOMPARE(QColor(shot.pixel(title.left() + 20, title.top() + 4)), QColor(0x13, 0x77, 0x3A));
        QCOMPARE(QColor(shot.pixel(title.right() - 20, title.bottom() - 4)), QColor(0x13, 0x77, 0x3A));
        QVERIFY(title.width() > 6 * 80);
        // the header row is the light green; the totals row has its double line under it
        QCOMPARE(QColor(shot.pixel(L.cellRect(1, 2).center().x() + topLeft.x() + 25, L.cellRect(1, 2).top() + topLeft.y() + 3)), QColor(0xE2, 0xF0, 0xE6));
        const QRect total = L.cellRect(8, 1).translated(topLeft);
        QCOMPARE(QColor(shot.pixel(total.left() + 6, total.bottom() - 1)).rgb(), QColor(Qt::black).rgb());   // (the first of the two lines of the double border)
        const QString out = qEnvironmentVariable("JC_SCREENSHOT");
        if (!out.isEmpty()) QVERIFY(shot.save(out));
    }
};

JP_TEST_CLASS(CrunchWindowTests)
#include "tst_crunch.moc"
