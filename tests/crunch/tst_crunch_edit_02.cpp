#include "tst_crunch_edit.h"


void CrunchEditTests::saveAsWritesAnotherFileAndTheWindowIsThatFile()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("3"));
    QVERIFY(e.settle());
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    const QString other = e.folder.dir.filePath(QStringLiteral("copy"));   // (no ending: it is given one)
    QVERIFY(e.win.w.saveAs(other));
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(saved.first().at(0).toString(), other + QStringLiteral(".xlsx"));
    QVERIFY(QFileInfo::exists(other + QStringLiteral(".xlsx")));
    QCOMPARE(e.win.w.filePath(), other + QStringLiteral(".xlsx"));
    // the first file is as it was
    Package p;
    QVERIFY(p.readLazyFile(e.path));
    Workbook b;
    QVERIFY(b.open(p));
    Sheet s;
    QVERIFY(b.readSheet(0, &s));
    QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("A1")), QStringLiteral("n:1"));
    // a wrong ending is refused
    QVERIFY(!e.win.w.saveAs(e.folder.dir.filePath(QStringLiteral("x.xlsm"))));
    QCOMPARE(e.win.told.size(), 1);
    QVERIFY(e.win.told[0].contains(QLatin1String(".xlsx")));
}


// A save that fails says why and leaves the file that was there as it was.
void CrunchEditTests::aSaveThatFailsLeavesTheOldFile()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    QFile f(e.path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray before = f.readAll();
    f.close();
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("3"));
    QVERIFY(e.settle());
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.saveAs(e.folder.dir.filePath(QStringLiteral("no/such/folder/book.xlsx"))));
    QVERIFY(saved.wait(20000));   // (this save is meant to fail: its ok is false, below)
    QCOMPARE(saved.first().at(1).toBool(), false);
    QCOMPARE(e.win.told.size(), 1);
    QVERIFY2(e.win.told[0].startsWith(QLatin1String("Save: ")) && e.win.told[0].contains(QLatin1String("couldn't save")), qPrintable(e.win.told[0]));
    QVERIFY(f.open(QIODevice::ReadOnly));
    QVERIFY(f.readAll() == before);
    QVERIFY(e.win.w.isModified());                  // (still to be saved)
    QVERIFY(!e.win.w.isSaving());
    QCOMPARE(e.win.w.filePath(), e.path);
    // a file that is there, and cannot be written (read-only): the save fails with a message and the file is as it was
    const QString locked = e.folder.write(QStringLiteral("locked.xlsx"), QByteArray("not a workbook, and read-only"));
    QVERIFY(QFile::setPermissions(locked, QFileDevice::ReadOwner));
    if (!QFileInfo(locked).isWritable()) {
        e.win.told.clear();
        QVERIFY(e.win.w.saveAs(locked));
        QVERIFY(saved.wait(20000));   // (this save is meant to fail too)
        QCOMPARE(saved.last().at(1).toBool(), false);
        QCOMPARE(e.win.told.size(), 1);
        QVERIFY2(e.win.told[0].contains(QLatin1String("couldn't save")), qPrintable(e.win.told[0]));
        QFile now(locked);
        QVERIFY(now.open(QIODevice::ReadOnly));
        QCOMPARE(now.readAll(), QByteArray("not a workbook, and read-only"));
        QCOMPARE(e.win.w.filePath(), e.path);
        QVERIFY(e.win.w.isModified());
        QCOMPARE(QDir(e.folder.dir.path()).entryList(QDir::Files, QDir::Name), (QStringList{QStringLiteral("book.xlsx"), QStringLiteral("locked.xlsx")}));   // (no stray file)
    }
    QFile::setPermissions(locked, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    // a file that is there and can be written is written over, whole
    const QString existing = e.folder.write(QStringLiteral("existing.xlsx"), QByteArray("an old file that is not a workbook"));
    e.win.told.clear();
    QVERIFY(e.win.w.saveAs(existing));
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(saved.last().at(1).toBool(), true);
    QCOMPARE(e.win.told.size(), 0);
    QCOMPARE(savedCell(existing, QStringLiteral("A1")), QStringLiteral("n:3"));
    QCOMPARE(e.win.w.filePath(), existing);
    // and it can be edited and saved again
    e.select(QStringLiteral("A2"));
    e.enter(QStringLiteral("4"));
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(saved.last().at(1).toBool(), true);
}


// A window with changes that are not saved asks before it closes: Cancel keeps it, Don't Save closes it, Save saves and then closes. One without changes closes.
void CrunchEditTests::closingAsksAboutChangesThatAreNotSaved()
{
    auto make = [](EditWindow &e, CrunchWindow::Answer answer, int *asked) {
        e.win.w.setAsker([answer, asked](const QString &, const QString &text) {
            ++*asked;
            return text.contains(QLatin1String("book.xlsx")) ? answer : CrunchWindow::Answer::Cancel;
        });
    };
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        int asked = 0;
        make(e, CrunchWindow::Answer::Cancel, &asked);
        QVERIFY(e.win.w.close());          // nothing changed: closes without asking
        QCOMPARE(asked, 0);
    }
    for (const auto answer : {CrunchWindow::Answer::Cancel, CrunchWindow::Answer::Discard, CrunchWindow::Answer::Save}) {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        int asked = 0;
        make(e, answer, &asked);
        e.win.w.setAttribute(Qt::WA_DeleteOnClose, false);
        e.select(QStringLiteral("A1"));
        e.enter(QStringLiteral("8"));
        QVERIFY(e.settle());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        e.win.w.close();
        QCOMPARE(asked, 1);
        if (answer == CrunchWindow::Answer::Cancel) {
            QVERIFY(e.win.w.isVisible());
            QVERIFY(e.win.w.isModified());
        } else if (answer == CrunchWindow::Answer::Discard) {
            QVERIFY(!e.win.w.isVisible());
        } else {
            QVERIFY2(savedOk(saved, 20000), "the save failed");
            QTRY_VERIFY(!e.win.w.isVisible());
            Package p;
            QVERIFY(p.readLazyFile(e.path));
            Workbook b;
            QVERIFY(b.open(p));
            Sheet s;
            QVERIFY(b.readSheet(0, &s));
            QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("A1")), QStringLiteral("n:8"));
        }
    }
}


// A save asked for while the formulas are still being worked out waits until they are: the file has their results.
void CrunchEditTests::saveWaitsForTheCalculation()
{
    const int n = 20000;
    QString rows;
    rows.reserve(n * 90);
    for (int r = 1; r <= n; ++r)
        rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*2+$D$1</f><v>-1</v></c>%2</row>").arg(r).arg(r == 1 ? QStringLiteral("<c r=\"D1\"><v>0</v></c>") : QString());
    EditWindow e;
    QVERIFY(e.open(buildXlsx({worksheet(rows)})));
    e.select(QStringLiteral("D1"));
    e.win.w.setWorkerPace(3000);
    e.enter(QStringLiteral("1"));
    QVERIFY(e.win.w.isCalculating());
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.save());
    QVERIFY(e.win.w.isCalculating());
    QCOMPARE(saved.count(), 0);
    QVERIFY(e.win.w.statusText().contains(QLatin1String("calculation")));
    e.win.w.setWorkerPace(0);
    QVERIFY2(savedOk(saved, 60000), "the save failed");
    QCOMPARE(saved.first().at(1).toBool(), true);
    Package p;
    QVERIFY(p.readLazyFile(e.path));
    Workbook b;
    QVERIFY(b.open(p));
    Sheet s;
    QVERIFY(b.readSheet(0, &s));
    QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B1")), QStringLiteral("n:3"));
    QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B%1").arg(n)), QStringLiteral("n:%1").arg(2 * n + 1));
}


// Saving over a file that another program changed since it was opened would put this program's copy of its untouched parts over that work: it is asked about first (Save a Copy,
// Overwrite, or Cancel). A file only touched (the same bytes), and one this program wrote itself, are saved over without a question.
void CrunchEditTests::savingOverAFileThatChangedAsks()
{
    const QByteArray other = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>12345</v></c></row>"))});
    EditWindow e;
    QVERIFY(e.open(editBook()));
    int asked = 0;
    CrunchWindow::ChangedAnswer answer = CrunchWindow::ChangedAnswer::Cancel;
    QString shownName;
    e.win.w.setChangedAsker([&](const QString &name) {
        ++asked;
        shownName = name;
        return answer;
    });
    QString copyPath = e.folder.dir.filePath(QStringLiteral("copy.xlsx"));
    e.win.w.setSavePicker([&](const QString &, const QString &) { return copyPath; });
    auto disk = [&] {
        QFile f(e.path);
        f.open(QIODevice::ReadOnly);
        return f.readAll();
    };
    const QByteArray original = disk();
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("100"));
    QVERIFY(e.settle());
    // another program changes it: the bytes differ
    QVERIFY(e.folder.write(QStringLiteral("book.xlsx"), other) == e.path);
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    // Cancel: nothing is written, the changes stay
    QVERIFY(!e.win.w.save());
    QCOMPARE(asked, 1);
    QCOMPARE(shownName, QStringLiteral("book.xlsx"));
    QCOMPARE(saved.count(), 0);
    QVERIFY(disk() == other);
    QVERIFY(e.win.w.isModified());
    // Save a Copy: the Save As is asked for (here answered with a path), the other program's file stays as it is
    answer = CrunchWindow::ChangedAnswer::SaveCopy;
    QVERIFY(!e.win.w.save());
    QCOMPARE(asked, 2);
    QTRY_COMPARE(saved.count(), 1);
    QCOMPARE(saved.first().at(0).toString(), copyPath);
    QVERIFY(disk() == other);
    QVERIFY(QFileInfo::exists(copyPath));
    QCOMPARE(e.win.w.filePath(), copyPath);   // (the window is that file now: saving it again asks nothing)
    e.select(QStringLiteral("A2"));
    e.enter(QStringLiteral("7"));
    QVERIFY(e.settle());
    QVERIFY(e.win.w.save());
    QTRY_COMPARE(saved.count(), 2);
    QCOMPARE(asked, 2);
}


// A file that is as this program read it is saved over without a question, and Overwrite saves over what the other program wrote.
void CrunchEditTests::savingOverAFileThatIsAsItWasReadAsksNothingAndOverwriteWrites()
{
    const QByteArray other = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>12345</v></c></row>"))});
    EditWindow e;
    QVERIFY(e.open(editBook()));
    int asked = 0;
    e.win.w.setChangedAsker([&](const QString &) {
        ++asked;
        return CrunchWindow::ChangedAnswer::Overwrite;
    });
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("100"));
    QVERIFY(e.settle());
    QVERIFY(e.win.w.save());
    QTRY_COMPARE(saved.count(), 1);
    QCOMPARE(asked, 0);
    e.select(QStringLiteral("A2"));
    e.enter(QStringLiteral("200"));
    QVERIFY(e.settle());
    // touched only: the same bytes, a newer time (a backup program, a sync)
    {
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        f.close();
        QVERIFY(e.folder.write(QStringLiteral("book.xlsx"), bytes) == e.path);
        QVERIFY(f.open(QIODevice::ReadWrite));
        QVERIFY(f.setFileTime(QDateTime::currentDateTime().addSecs(120), QFileDevice::FileModificationTime));
    }
    QVERIFY(e.win.w.save());
    QTRY_COMPARE(saved.count(), 2);
    QCOMPARE(asked, 0);
    e.select(QStringLiteral("A3"));
    e.enter(QStringLiteral("300"));
    QVERIFY(e.settle());
    QVERIFY(e.folder.write(QStringLiteral("book.xlsx"), other) == e.path);
    QVERIFY(e.win.w.save());
    QTRY_COMPARE(saved.count(), 3);
    QCOMPARE(asked, 1);
    QCOMPARE(savedCell(e.path, QStringLiteral("A3")), QStringLiteral("n:300"));
    QCOMPARE(savedCell(e.path, QStringLiteral("A2")), QStringLiteral("n:200"));
    QCOMPARE(savedCell(e.path, QStringLiteral("A1")), QStringLiteral("n:100"));
}


// A file that is open in a window is not opened in another: the window that has it is brought forward.
void CrunchEditTests::aFileThatIsOpenIsBroughtForwardNotOpenedAgain()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    QSignalSpy forward(&e.win.w, &CrunchWindow::broughtForward);
    Window second;
    QVERIFY(second.w.openFile(e.path));
    QCOMPARE(forward.count(), 1);
    QVERIFY(!second.w.hasWorkbook() && !second.w.isLoading());
    // by another spelling of the same path, and from the window that has it
    QVERIFY(second.w.openFile(QFileInfo(e.path).absolutePath() + QStringLiteral("/../") + QFileInfo(QFileInfo(e.path).absolutePath()).fileName() + QLatin1Char('/') + QFileInfo(e.path).fileName()));
    QCOMPARE(forward.count(), 2);
    QVERIFY(e.win.w.openFile(e.path));
    QCOMPARE(forward.count(), 3);
    QVERIFY(e.win.w.hasWorkbook());
    // another file opens in a window of its own (the window that has one)
    const QString other = e.folder.write(QStringLiteral("other.xlsx"), editBook());
    QVERIFY(second.w.openFile(other));
    QCOMPARE(forward.count(), 3);
    QVERIFY(second.w.isLoading() || second.w.hasWorkbook());
}


// ---- a new workbook

// A window with no file holds a real, empty workbook: one sheet named Sheet1, a name like Book1, an editing session; typing and formulas work, the status never says it is being opened.
void CrunchEditTests::aNewWorkbookTakesTypingAndFormulas()
{
    EditWindow e;
    QVERIFY(e.openBlank());
    QVERIFY(e.win.w.hasWorkbook() && !e.win.w.isLoading());
    QVERIFY(e.win.w.session() != nullptr);
    QVERIFY(e.win.w.filePath().isEmpty());
    QVERIFY2(e.win.w.windowTitle().contains(e.win.w.displayName()), qPrintable(e.win.w.windowTitle()));
    QVERIFY(e.win.w.displayName().startsWith(QLatin1String("Book")));
    QCOMPARE(e.win.w.statusText(), QStringLiteral("Ready"));
    QCOMPARE(e.win.w.loadedBook()->book->info().sheets.size(), 1);
    QCOMPARE(e.win.w.loadedBook()->book->info().sheets[0].name, QStringLiteral("Sheet1"));
    QVERIFY(!e.win.w.isModified());
    QVERIFY(e.win.w.act(QStringLiteral("file.save"))->isEnabled());
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("21"));
    QVERIFY(e.win.w.isModified());
    QCOMPARE(e.shownAt(QStringLiteral("A1")), QStringLiteral("21"));
    e.select(QStringLiteral("B1"));
    e.enter(QStringLiteral("=A1*2"));
    QVERIFY(e.settle());
    QCOMPARE(e.shownAt(QStringLiteral("B1")), QStringLiteral("42"));
    QVERIFY(!e.win.w.statusText().contains(QLatin1String("still being opened")));
    QCOMPARE(e.win.told.size(), 0);
}


// Undo takes a typed value back. A key typed before the new workbook is ready is not lost: the status says it is getting ready (not that a file is being opened), and the entry begins with
// the key when the workbook is ready.
void CrunchEditTests::undoWorksOnANewWorkbook()
{
    EditWindow e;
    QSignalSpy loaded(&e.win.w, &CrunchWindow::loadFinished);
    QVERIFY(e.win.w.newBlank());
    QVERIFY(e.win.w.session() == nullptr);
    QTest::keyClick(e.view(), Qt::Key_7);
    QVERIFY(e.win.w.statusText().contains(QLatin1String("getting ready")));
    QVERIFY(!e.win.w.statusText().contains(QLatin1String("still being opened")));
    QVERIFY(!e.win.w.isEditing());
    QVERIFY(loaded.wait(20000));
    QVERIFY(e.win.w.isEditing());
    QCOMPARE(e.win.w.cellEditor()->text(), QStringLiteral("7"));
    e.key(Qt::Key_Return);
    QVERIFY(e.settle());
    QCOMPARE(e.shownAt(QStringLiteral("A1")), QStringLiteral("7"));
    QVERIFY(e.win.w.act(QStringLiteral("edit.undo"))->isEnabled());
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    QCOMPARE(e.shownAt(QStringLiteral("A1")), QString());
    QVERIFY(!e.win.w.isModified());
    QVERIFY(e.win.w.act(QStringLiteral("edit.redo"))->isEnabled());
    QCOMPARE(e.win.told.size(), 0);
    QVERIFY(e.settle());
}


// A new window takes typing at once: the keys go to the grid, not to the File button, so a person who starts typing sees the cell fill (and Enter does not open the File screen).
void CrunchEditTests::aNewWindowTakesTypingWithoutAClick()
{
    Window w;
    w.w.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&w.w));
    QVERIFY(w.openBlank());
    QWidget *focus = QApplication::focusWidget();
    QVERIFY(focus != nullptr);
    QTest::keyClick(focus, Qt::Key_7);
    QVERIFY2(w.w.isEditing(), focus->metaObject()->className());
    QCOMPARE(w.w.cellEditor()->text(), QStringLiteral("7"));
}


// Save on a workbook that was never saved asks for a name (as Save As does) and writes an .xlsx; opening it again gives the same cells, and the window then has that file.
void CrunchEditTests::saveOnANewWorkbookGoesToSaveAs()
{
    EditWindow e;
    QVERIFY(e.openBlank());
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("12"));
    e.select(QStringLiteral("A2"));
    e.enter(QStringLiteral("hello"));
    e.select(QStringLiteral("B1"));
    e.enter(QStringLiteral("=A1+1"));
    QVERIFY(e.settle());
    const QString name = e.win.w.displayName();
    const QString target = e.folder.dir.filePath(QStringLiteral("made"));   // (no ending: .xlsx is added)
    QString startName;
    e.win.w.setSavePicker([&](const QString &start, const QString &) {
        startName = start;
        return target;
    });
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    e.win.w.act(QStringLiteral("file.save"))->trigger();
    QVERIFY(savedOk(saved));
    QCOMPARE(startName, name + QStringLiteral(".xlsx"));
    const QString written = target + QStringLiteral(".xlsx");
    QVERIFY(QFileInfo::exists(written));
    QCOMPARE(e.win.w.filePath(), written);
    QVERIFY(!e.win.w.isModified());
    QCOMPARE(savedCell(written, QStringLiteral("A1")), QStringLiteral("n:12"));
    QCOMPARE(savedCell(written, QStringLiteral("A2")).mid(2), QStringLiteral("hello"));
    QVERIFY(savedCell(written, QStringLiteral("B1")).startsWith(QLatin1String("t:")) || savedCell(written, QStringLiteral("B1")).startsWith(QLatin1String("n:")));
    QFile file(written);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QString copy = e.folder.write(QStringLiteral("copy.xlsx"), file.readAll());   // (a file that is open in a window is not opened in a second: its copy is)
    file.close();
    Window again;
    QVERIFY(again.open(copy));
    QCOMPARE(again.w.view()->sheet().textOf(0, 0, *again.w.view()->sheet().cellAt(0, 0)), QStringLiteral("12"));
    QCOMPARE(again.w.view()->sheet().textOf(0, 1, *again.w.view()->sheet().cellAt(0, 1)), QStringLiteral("13"));
}


// The defaults of a new workbook: the font, its size, and how many sheets it starts with.
void CrunchEditTests::aNewWorkbookStartsWithTheChosenFontSizeAndSheets()
{
    crunch::BlankDefaults d;
    d.font = QStringLiteral("Georgia");
    d.size = 13;
    d.sheets = 3;
    QMap<QString, QByteArray> parts;
    QVERIFY(jp::readZip(crunch::blankWorkbookBytes(d), parts));
    const QString book = QString::fromUtf8(parts.value(QStringLiteral("xl/workbook.xml")));
    QVERIFY2(book.contains(QLatin1String("name=\"Sheet3\"")) && !book.contains(QLatin1String("Sheet4")), qPrintable(book));
    const QString styles = QString::fromUtf8(parts.value(QStringLiteral("xl/styles.xml")));
    QVERIFY(styles.contains(QLatin1String("<sz val=\"13\"/><name val=\"Georgia\"/>")));
    QVERIFY(parts.contains(QStringLiteral("xl/worksheets/sheet3.xml")));
    // (a name or a count that makes no sense is the usual one)
    crunch::BlankDefaults bad;
    bad.font = QStringLiteral("<x>");
    bad.size = 0;
    bad.sheets = 9000;
    parts.clear();
    QVERIFY(jp::readZip(crunch::blankWorkbookBytes(bad), parts));
    QVERIFY(QString::fromUtf8(parts.value(QStringLiteral("xl/styles.xml"))).contains(QLatin1String("<sz val=\"11\"/><name val=\"Calibri\"/>")));
    QVERIFY(!QString::fromUtf8(parts.value(QStringLiteral("xl/workbook.xml"))).contains(QLatin1String("Sheet256")));
}


// A new workbook prints on Letter in the United States and Canada and on A4 elsewhere, and the page setup shows centimeters outside the United States; a file with no page setup is Letter as before.
void CrunchEditTests::aNewWorkbooksPaperAndUnitsFollowTheRegion()
{
    struct Case { const char *locale; int paper; bool metric; };
    for (const Case &c : {Case{"en_US", 1, false}, Case{"en_CA", 1, true}, Case{"en_GB", 9, true}, Case{"en_AU", 9, true}, Case{"de_DE", 9, true}}) {
        PinnedLocale pinned{QLocale(QLatin1String(c.locale))};
        EditWindow e;
        QVERIFY(e.openBlank());
        QVERIFY2(e.win.w.session()->pages().sheet(0).paperSize == c.paper, c.locale);
        bool metric = !c.metric;
        e.win.w.setPageSetupAsker([&](PageSetupRequest *r) {
            metric = r->metric;
            return false;
        });
        e.win.w.act(QStringLiteral("pagesetup.dialog"))->trigger();
        QVERIFY2(metric == c.metric, c.locale);
    }
    QMap<QString, QByteArray> parts;
    QVERIFY(jp::readZip(crunch::blankWorkbookBytes(), parts));
    QVERIFY(!QString::fromUtf8(parts.value(QStringLiteral("xl/worksheets/sheet1.xml"))).contains(QLatin1String("pageSetup")));
    crunch::BlankDefaults a4;
    a4.paperSize = 9;
    QVERIFY(jp::readZip(crunch::blankWorkbookBytes(a4), parts));
    QVERIFY(QString::fromUtf8(parts.value(QStringLiteral("xl/worksheets/sheet1.xml"))).contains(QLatin1String("<pageSetup paperSize=\"9\"/>")));
}


void CrunchEditTests::theWindowMakesNewWorkbooksFromTheSettings()
{
    Settings::get().setValue(QStringLiteral("crunch/newSheets"), 2);
    Settings::get().setValue(QStringLiteral("crunch/newFont"), QStringLiteral("Georgia"));
    EditWindow e;
    QVERIFY(e.openBlank());
    QCOMPARE(e.win.w.session()->sheetOrder().size(), 2);
    Settings::get().setValue(QStringLiteral("crunch/newSheets"), 1);
    Settings::get().setValue(QStringLiteral("crunch/newFont"), QStringLiteral("Calibri"));
}


// File > New makes a second window with a new workbook of its own, named after the first; closing a new workbook nobody changed asks nothing, and a changed one asks.
void CrunchEditTests::fileNewMakesAnotherWindowWithABlankWorkbook()
{
    EditWindow e;
    QVERIFY(e.openBlank());
    const QString first = e.win.w.displayName();
    QVERIFY(first.startsWith(QLatin1String("Book")));
    const QList<QWidget *> before = QApplication::topLevelWidgets();
    e.win.w.act(QStringLiteral("file.new"))->trigger();
    CrunchWindow *second = nullptr;
    for (QWidget *top : QApplication::topLevelWidgets())
        if (auto *c = qobject_cast<CrunchWindow *>(top); c && c != &e.win.w && !before.contains(top)) second = c;
    QVERIFY(second);
    QPointer<CrunchWindow> guard(second);
    second->setNotifier([](const QString &, const QString &) {});
    QVERIFY(QTest::qWaitFor([&] { return second->session() != nullptr && !second->isCalculating(); }, 20000));
    QCOMPARE(second->displayName(), QStringLiteral("Book%1").arg(first.mid(4).toInt() + 1));
    QVERIFY(second->windowTitle().startsWith(second->displayName()));
    QCOMPARE(e.win.w.displayName(), first);
    second->view()->selectCell(0, 0);
    QTest::keyClick(second->view(), Qt::Key_9);
    QVERIFY(second->isEditing());
    QTest::keyClick(second->cellEditor(), Qt::Key_Return);
    QVERIFY(second->isModified());
    QVERIFY(!e.win.w.isModified());
    // the changed one asks before it closes; the other one does not
    int asked = 0;
    second->setAsker([&](const QString &, const QString &) {
        ++asked;
        return CrunchWindow::Answer::Cancel;
    });
    QVERIFY(!second->close());
    QCOMPARE(asked, 1);
    second->setAsker([&](const QString &, const QString &) {
        ++asked;
        return CrunchWindow::Answer::Discard;
    });
    QVERIFY(second->close());
    QCOMPARE(asked, 2);
    QTRY_VERIFY(guard.isNull());
    e.win.w.setAsker([&](const QString &, const QString &) {
        ++asked;
        return CrunchWindow::Answer::Cancel;
    });
    e.win.w.setAttribute(Qt::WA_DeleteOnClose, false);
    QVERIFY(e.win.w.close());
    QCOMPARE(asked, 2);
}


// A file opened in a new window nobody has changed takes its place; once the new workbook has been changed, the file opens in a window of its own. A file that can't be read
// leaves the window with a new workbook again.
void CrunchEditTests::aFileOpenedInAnUnchangedNewWorkbookReplacesIt()
{
    EditWindow e;
    QVERIFY(e.openBlank());
    QVERIFY(e.win.w.isUntouchedBlank());
    const QString book = e.folder.write(QStringLiteral("budget.xlsx"), editBook());
    QVERIFY(e.win.open(book));
    QVERIFY(!e.win.w.isUntouchedBlank());
    QCOMPARE(e.win.w.filePath(), book);
    QCOMPARE(e.shownAt(QStringLiteral("A2")), QStringLiteral("10"));
    QCOMPARE(e.win.w.displayName(), QStringLiteral("budget"));
    QVERIFY(e.win.w.windowTitle().startsWith(QStringLiteral("budget.xlsx")));
    // a changed new workbook keeps its window
    Window changed;
    QVERIFY(changed.openBlank());
    changed.w.view()->selectCell(0, 0);
    QTest::keyClick(changed.w.view(), Qt::Key_3);
    QTest::keyClick(changed.w.cellEditor(), Qt::Key_Return);
    QVERIFY(!changed.w.isUntouchedBlank());
    const QList<QWidget *> before = QApplication::topLevelWidgets();
    QVERIFY(changed.w.openFile(book));
    QVERIFY(changed.w.filePath().isEmpty());
    QVERIFY(changed.w.isModified());
    for (QWidget *top : QApplication::topLevelWidgets())
        if (auto *c = qobject_cast<CrunchWindow *>(top); c && !before.contains(top)) {
            c->setAttribute(Qt::WA_DeleteOnClose, false);
            QVERIFY(c->filePath() == book);
            delete c;
        }
    // a file that is not a workbook: the new workbook comes back
    Window broken;
    QVERIFY(broken.openBlank());
    QSignalSpy bad(&broken.w, &CrunchWindow::loadFinished);
    QVERIFY(broken.w.openFile(e.folder.write(QStringLiteral("broken.xlsx"), QByteArray("not a zip"))));
    QVERIFY(bad.wait(20000));
    QVERIFY(bad.first().first().toBool() == false);
    QTRY_VERIFY(broken.w.session() != nullptr && broken.w.isUntouchedBlank());
}


// ---- what keeps a window from being left with nothing

QList<CrunchWindow *> CrunchEditTests::crunchWindows()
{
    QList<CrunchWindow *> all;
    for (QWidget *top : QApplication::topLevelWidgets())
        if (auto *c = qobject_cast<CrunchWindow *>(top)) all << c;
    return all;
}

CrunchWindow *CrunchEditTests::windowSince(const QList<CrunchWindow *> &before)
{
    for (CrunchWindow *c : crunchWindows())
        if (!before.contains(c)) return c;
    return nullptr;
}

// Closes the windows a test made (a window with changes is let go without asking).
void CrunchEditTests::closeSince(const QList<CrunchWindow *> &before)
{
    for (CrunchWindow *c : crunchWindows())
        if (!before.contains(c)) {
            c->setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
            c->close();
        }
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

// A window of its own, as File > New and Open make (it is deleted when it is closed), with a new workbook that is ready.
CrunchWindow *CrunchEditTests::readyNewWindow(QStringList *told)
{
    auto *w = new CrunchWindow();
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setNotifier([told](const QString &title, const QString &text) { *told << title + QStringLiteral(": ") + text; });
    w->resize(1100, 640);
    w->show();
    QSignalSpy loaded(w, &CrunchWindow::loadFinished), done(w, &CrunchWindow::calculationFinished);
    if (!w->newBlank() || !loaded.wait(20000) || (done.isEmpty() && !done.wait(20000))) return nullptr;
    return w;
}

void CrunchEditTests::typeInto(CrunchWindow *w, const QString &ref, char key)
{
    quint32 r = 0, c = 0;
    parseCellRef(ref, &r, &c);
    w->view()->selectCell(r, c);
    QTest::keyClick(w->view(), key);
    QTest::keyClick(w->cellEditor(), Qt::Key_Return);
}


// A file that can't be read, opened from a window that holds a workbook (the second file on the command line, a recent file, File > Open), opens in a window made for it; that window goes
// after the message, and the window that was asked keeps its workbook. A window that had a new workbook, or none, has a new workbook that can be changed.
void CrunchEditTests::aFileThatCannotBeReadLeavesNoDeadWindow()
{
    EditWindow e;
    QVERIFY(e.open(editBook()));
    const QString bad = e.folder.write(QStringLiteral("bad.xlsx"), QByteArray("this is not a zip file"));
    const QList<CrunchWindow *> before = crunchWindows();
    QVERIFY(e.win.w.openFile(bad));
    CrunchWindow *made = windowSince(before);
    QVERIFY(made);
    QPointer<CrunchWindow> guard(made);
    QTRY_VERIFY(guard.isNull());
    QCOMPARE(e.win.told.size(), 1);
    QVERIFY2(e.win.told.first().contains(QLatin1String("bad.xlsx")), qPrintable(e.win.told.first()));
    QCOMPARE(e.win.w.filePath(), e.path);
    QVERIFY(e.win.w.session() != nullptr);
    QCOMPARE(crunchWindows().size(), before.size());
    // a window that is not made for the file has a new workbook afterwards, in every case
    Window plain;
    QSignalSpy failed(&plain.w, &CrunchWindow::loadFinished);
    QVERIFY(plain.w.openFile(bad));
    QVERIFY(failed.wait(20000));
    QTRY_VERIFY(plain.w.session() != nullptr && plain.w.isUntouchedBlank());
    QVERIFY(plain.w.view()->cellsEnabled());
    Window blank;
    QVERIFY(blank.openBlank());
    QSignalSpy failedAgain(&blank.w, &CrunchWindow::loadFinished);
    QVERIFY(blank.w.openFile(bad));
    QVERIFY(failedAgain.wait(20000));
    QTRY_VERIFY(blank.w.session() != nullptr && blank.w.isUntouchedBlank());
    QCOMPARE(blank.w.filePath(), QString());
    closeSince(before);
}


// The numbers of new workbooks are taken by new workbooks only: a window made for a file does not use one, and a number is not given again.
void CrunchEditTests::bookNumbersAreForNewWorkbooksOnly()
{
    EditWindow e;
    QVERIFY(e.openBlank());
    const int n = e.win.w.displayName().mid(4).toInt();
    QVERIFY(n > 0);
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("1"));
    const QList<CrunchWindow *> before = crunchWindows();
    const QString file = e.folder.write(QStringLiteral("other.xlsx"), editBook());
    QVERIFY(e.win.w.openFile(file));
    CrunchWindow *forFile = windowSince(before);
    QVERIFY(forFile);
    QVERIFY(forFile->displayName() != QStringLiteral("Book%1").arg(n + 1));
    const QList<CrunchWindow *> beforeNew = crunchWindows();
    e.win.w.act(QStringLiteral("file.new"))->trigger();
    CrunchWindow *fresh = windowSince(beforeNew);
    QVERIFY(fresh);
    QCOMPARE(fresh->displayName(), QStringLiteral("Book%1").arg(n + 1));
    // closed, its number is not used again
    QPointer<CrunchWindow> guard(fresh);
    fresh->setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Discard; });
    fresh->close();
    QTRY_VERIFY(guard.isNull());
    const QList<CrunchWindow *> beforeLast = crunchWindows();
    e.win.w.act(QStringLiteral("file.new"))->trigger();
    CrunchWindow *last = windowSince(beforeLast);
    QVERIFY(last);
    QCOMPARE(last->displayName(), QStringLiteral("Book%1").arg(n + 2));
    closeSince(before);
}


// A save that waits for the calculation (the values of the sheet as a .csv) belongs to the workbook that asked: a file that takes the place of the new workbook is not written to it.
void CrunchEditTests::aWaitingSaveIsNotDoneForTheFileThatReplacesTheBlank()
{
    Folder folder;
    Window w;
    const QString csv = folder.dir.filePath(QStringLiteral("out.csv"));
    const QString book = folder.write(QStringLiteral("good.xlsx"), editBook());
    bool fired = false, waiting = false;
    connect(&w.w, &CrunchWindow::loadFinished, &w.w, [&](bool ok) {
        if (!ok || fired) return;
        fired = true;
        w.w.saveAs(csv);
        waiting = w.w.statusText().contains(QLatin1String("Saving when"));
        w.w.openFile(book);
    });
    QVERIFY(w.w.newBlank());
    QTRY_VERIFY(fired);
    QVERIFY(waiting);
    QTRY_VERIFY(w.w.filePath() == book && w.w.session() != nullptr && !w.w.isCalculating());
    QVERIFY2(!w.w.statusText().contains(QLatin1String("Saving")), qPrintable(w.w.statusText()));
    QVERIFY(!QFileInfo::exists(csv));
    QCOMPARE(w.told.size(), 0);
}


// A new workbook that is being saved is not given up for a file: the file opens in a window of its own.
void CrunchEditTests::aNewWorkbookBeingSavedKeepsItsWindow()
{
    Folder folder;
    Window w;
    QVERIFY(w.openBlank());
    QVERIFY(w.w.isUntouchedBlank());
    const QList<CrunchWindow *> before = crunchWindows();
    QVERIFY(w.w.saveAs(folder.dir.filePath(QStringLiteral("values.csv"))));
    QVERIFY(!w.w.isUntouchedBlank());
    QVERIFY(w.w.openFile(folder.write(QStringLiteral("good.xlsx"), editBook())));
    QVERIFY(windowSince(before) != nullptr);
    QVERIFY(w.w.filePath().isEmpty());
    closeSince(before);
}


// A file asked for while the new workbook is still getting ready takes its place.
void CrunchEditTests::aFileOpenedWhileTheNewWorkbookIsLoadingTakesItsPlace()
{
    Folder folder;
    Window w;
    const QList<CrunchWindow *> before = crunchWindows();
    QVERIFY(w.w.newBlank());
    QVERIFY(w.w.isUntouchedBlank());
    QVERIFY(w.open(folder.write(QStringLiteral("good.xlsx"), editBook())));
    QCOMPARE(crunchWindows().size(), before.size());
    QCOMPARE(QFileInfo(w.w.filePath()).fileName(), QStringLiteral("good.xlsx"));
    QVERIFY(w.w.windowTitle().startsWith(QStringLiteral("good.xlsx")));
    QVERIFY(w.w.session() != nullptr);
    QCOMPARE(w.told.size(), 0);
}


// Close asks to save a changed new workbook. Save, with no name given (the dialog is cancelled), keeps the window; and the close that asked is not left waiting: a later Save saves and
// leaves the window open. With a name, the workbook is saved and the window then closes.
void CrunchEditTests::closingAChangedNewWorkbookThroughSave()
{
    Folder folder;
    const QList<CrunchWindow *> before = crunchWindows();
    QStringList told;
    CrunchWindow *w = readyNewWindow(&told);
    QVERIFY(w);
    QPointer<CrunchWindow> guard(w);
    typeInto(w, QStringLiteral("A1"), '4');
    QVERIFY(w->isModified());
    int asked = 0;
    w->setAsker([&](const QString &, const QString &) {
        ++asked;
        return CrunchWindow::Answer::Save;
    });
    QString answer;
    w->setSavePicker([&](const QString &, const QString &) { return answer; });
    QVERIFY(!w->close());
    QCOMPARE(asked, 1);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(!guard.isNull());
    QVERIFY(w->isModified());
    // (the cancelled close is forgotten: a Save made now does not close the window)
    answer = folder.dir.filePath(QStringLiteral("kept.xlsx"));
    QSignalSpy saved(w, &CrunchWindow::saved);
    w->act(QStringLiteral("file.save"))->trigger();
    QVERIFY(savedOk(saved));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(!guard.isNull());
    QVERIFY(!w->isModified());
    // with a name given at once, the close saves and goes
    QStringList told2;
    CrunchWindow *second = readyNewWindow(&told2);
    QVERIFY(second);
    QPointer<CrunchWindow> guard2(second);
    typeInto(second, QStringLiteral("A1"), '6');
    const QString closing = folder.dir.filePath(QStringLiteral("closing.xlsx"));
    second->setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Save; });
    second->setSavePicker([&](const QString &, const QString &) { return closing; });
    QVERIFY(!second->close());
    QTRY_VERIFY(guard2.isNull());
    QVERIFY(QFileInfo::exists(closing));
    QCOMPARE(savedCell(closing, QStringLiteral("A1")), QStringLiteral("n:6"));
    QCOMPARE(told.size() + told2.size(), 0);
    closeSince(before);
}


// Ctrl+N (the shortcut of this computer) makes a window with a new workbook.
void CrunchEditTests::theNewShortcutMakesAWindow()
{
    Window w;
    QVERIFY(w.openBlank());
    w.w.activateWindow();
    const QList<CrunchWindow *> before = crunchWindows();
    const QKeyCombination k = QKeySequence(QKeySequence::New)[0];
    QTest::keyClick(&w.w, k.key(), k.keyboardModifiers());
    QTRY_VERIFY(windowSince(before) != nullptr);
    CrunchWindow *made = windowSince(before);
    QTRY_VERIFY(made->session() != nullptr);
    closeSince(before);
}


// The New page of the File screen: its card leaves a new workbook nobody changed as it is (and closes the screen), and makes a window of its own when the workbook has been changed.
void CrunchEditTests::theFileScreensNewPageMakesABlankWorkbook()
{
    Window w;
    QVERIFY(w.openBlank());
    auto card = [&]() -> QAbstractButton * {
        w.w.showFileScreen(QStringLiteral("new"));
        auto *screen = w.w.findChild<CrunchFileScreen *>();
        if (!screen) return nullptr;
        for (auto *c : screen->findChildren<QAbstractButton *>())
            if (dynamic_cast<ActionCard *>(c) && c->text() == QLatin1String("Blank Workbook")) return c;
        return nullptr;
    };
    const QList<CrunchWindow *> before = crunchWindows();
    QAbstractButton *c = card();
    QVERIFY(c);
    c->click();
    QCOMPARE(crunchWindows().size(), before.size());
    QVERIFY(!w.w.findChild<CrunchFileScreen *>()->isVisible());
    const int n = w.w.displayName().mid(4).toInt();
    typeInto(&w.w, QStringLiteral("A1"), '1');
    c = card();
    QVERIFY(c);
    c->click();
    CrunchWindow *made = windowSince(before);
    QVERIFY(made);
    QCOMPARE(made->displayName(), QStringLiteral("Book%1").arg(n + 1));
    QTRY_VERIFY(made->session() != nullptr);
    closeSince(before);
}


// Esc while a save waits for the calculation stops the calculation and cancels the save, and says so; the save does not happen later when something finishes a calculation. A close that
// was waiting for that save asks again.
void CrunchEditTests::escCancelsASaveThatWaitsForTheCalculation()
{
    const int n = 2000;
    QString rows;
    for (int r = 1; r <= n; ++r)
        rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f>A%1*2+$D$1</f><v>-1</v></c>%2</row>").arg(r).arg(r == 1 ? QStringLiteral("<c r=\"D1\"><v>0</v></c>") : QString());
    const QByteArray book = buildXlsx({worksheet(rows)});
    for (const bool closing : {false, true}) {
        EditWindow e;
        QVERIFY(e.open(book, QStringLiteral("book.xlsx")));
        int asked = 0;
        e.win.w.setAsker([&](const QString &, const QString &) { return ++asked == 1 ? CrunchWindow::Answer::Save : CrunchWindow::Answer::Cancel; });
        e.win.w.setAttribute(Qt::WA_DeleteOnClose, false);
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("1"));
        // The worker is held back from here to the end of what is checked, so that the calculation is going whatever the speed of the machine: it can neither be done before the save is
        // asked for nor finish while Esc is pressed. (The window is told it is calculating when the edit is made, and no batch is looked at while no event is handled.)
        std::unique_ptr<crunch::WorkbookLoader::Hold> held = e.win.w.workbookLoader().hold();
        QVERIFY(held);
        QVERIFY(e.win.w.isCalculating());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        if (closing) {
            e.win.w.close();                                  // (the question is answered with Save: the save waits for the calculation)
            QCOMPARE(asked, 1);
        } else {
            QVERIFY(e.win.w.save());
        }
        QVERIFY(e.win.w.isCalculating());
        QVERIFY(e.win.w.statusText().contains(QLatin1String("calculation")));
        QTest::keyClick(&e.win.w, Qt::Key_Escape);
        QVERIFY(!e.win.w.isCalculating());
        QVERIFY2(e.win.w.statusText().contains(QLatin1String("save"), Qt::CaseInsensitive), qPrintable(e.win.w.statusText()));
        QCOMPARE(asked, closing ? 2 : 0);                    // (the close that was waiting asks again, and the answer to that is Cancel)
        QVERIFY(e.win.w.isVisible());
        held.reset();
        QTRY_VERIFY(e.win.w.loaded()->calcIdle.load());
        // something finishes a calculation: the save that was cancelled does not happen
        e.select(QStringLiteral("D1"));
        e.enter(QStringLiteral("2"));
        QVERIFY(e.settle());
        QCOMPARE(saved.count(), 0);
        QVERIFY(e.win.w.isModified());
        QVERIFY(e.win.w.isVisible());
        QVERIFY(!e.win.w.isSaving());
    }
}


// The first cell of a shared formula typed over, and the workbook saved: the cells that shared its formula keep theirs (each written out), with the results the worker gave them.
void CrunchEditTests::aSharedFormulaSurvivesItsFirstCellBeingTypedOver()
{
    QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f t=\"shared\" ref=\"B1:B4\" si=\"0\">A1*2</f><v>2</v></c></row>");
    for (int r = 2; r <= 4; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><f t=\"shared\" si=\"0\"/><v>%2</v></c></row>").arg(r).arg(r * 2);
    EditWindow e;
    QVERIFY(e.open(buildXlsx({worksheet(rows)})));
    e.select(QStringLiteral("B1"));
    e.enter(QStringLiteral("99"));
    e.select(QStringLiteral("A3"));
    e.enter(QStringLiteral("30"));
    QVERIFY(e.settle());
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(saved.first().at(1).toBool(), true);
    Package p;
    QVERIFY(p.readLazyFile(e.path));
    Workbook b;
    QVERIFY(b.open(p));
    Sheet s;
    QVERIFY(b.readSheet(0, &s));
    QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B1")), QStringLiteral("n:99"));
    QVERIFY(!at(s, QStringLiteral("B1"))->hasFormula());
    for (const int r : {2, 3, 4}) {
        const Cell *c = at(s, QStringLiteral("B%1").arg(r));
        QVERIFY(c && c->hasFormula());
        QCOMPARE(s.formulaText(*c), QStringLiteral("A%1*2").arg(r));
    }
    QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B3")), QStringLiteral("n:60"));
    QCOMPARE(show(s, b.sharedStrings(), QStringLiteral("B2")), QStringLiteral("n:4"));
}


// Only the results the edit changed are written: a formula that is worked out again with the value the engine gave it before keeps the result the file stored (the engine's differences from
// the program that wrote the file are not made into changes).
void CrunchEditTests::onlyTheResultsTheEditChangedAreWritten()
{
    // A1 is 1; B1 = A1*0 and C1 = A1*2, both stored as 999 (which the engine does not give)
    const QByteArray book = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f>A1*0</f><v>999</v></c><c r=\"C1\"><f>A1*2</f><v>999</v></c></row>"))});
    EditWindow e;
    QVERIFY(e.open(book));
    QCOMPARE(e.valueAt(QStringLiteral("B1")), QStringLiteral("n:0"));
    QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:2"));
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    auto savedValue = [&](const QString &ref) -> QString {
        Package p;
        Workbook b;
        Sheet s;
        if (!p.readLazyFile(e.path) || !b.open(p) || !b.readSheet(0, &s)) return QStringLiteral("the file does not open");
        return show(s, b.sharedStrings(), ref);
    };
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("5"));
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:10"));
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QCOMPARE(savedValue(QStringLiteral("A1")), QStringLiteral("n:5"));
    QCOMPARE(savedValue(QStringLiteral("C1")), QStringLiteral("n:10"));    // the engine changed it
    QCOMPARE(savedValue(QStringLiteral("B1")), QStringLiteral("n:999"));   // worked out again, and the same (0): the file's own stays
    // typed back: C1 is what the engine says again (2), B1 still the file's
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("1"));
    QVERIFY(e.settle());
    QVERIFY(e.win.w.save());
    QVERIFY2((saved.count() == 2 || saved.wait(20000)) && saved.last().value(1).toBool(), "the save failed");
    QCOMPARE(savedValue(QStringLiteral("C1")), QStringLiteral("n:2"));
    QCOMPARE(savedValue(QStringLiteral("B1")), QStringLiteral("n:999"));
}


// A workbook with macros is saved as a .xlsm with its macros as they were, and cannot be given another ending.
// What Undo of an edit of lines holds of the formulas as they were held (the id frame's `goneHeld` and `wasHeld`) is counted in the step's bytes, else the limit that makes a big step ask first
// would not see them.
void CrunchEditTests::theBytesOfAStructuralStepCountTheFormulasHeldInIds()
{
    const QString text(1000, QLatin1Char('x'));
    for (const int which : {0, 1}) {
        jp::crunch::StructChange change;
        const qint64 empty = change.bytes();
        jp::office::sheet::Calculation::EditRecord::HeldText held;
        held.text = text;
        held.has = true;
        std::vector<jp::office::sheet::Calculation::EditRecord::HeldText> &list = which == 0 ? change.record.goneHeld : change.record.wasHeld;
        list.push_back(held);
        list.push_back(held);
        QVERIFY2(change.bytes() - empty >= 2 * (qint64(sizeof(held)) + 2000), qPrintable(QStringLiteral("held %1: %2 bytes more").arg(which).arg(change.bytes() - empty)));
    }
}


// The formula as it is held, which a cell's content carries in the id frame, counts in the bytes of a step that holds it (the limit that makes a big step ask first reads them).
void CrunchEditTests::theBytesOfACellsContentCountTheFormulaAsItIsHeld()
{
    jp::office::sheet::Calculation::CellContent c;
    c.exists = c.isFormula = true;
    c.formula = QStringLiteral("A1+B1");
    const qint64 without = jp::crunch::contentBytes(c);
    c.hasHeld = true;
    c.held = QString(1000, QLatin1Char('x'));
    c.heldEpochs = {1, 2, 3};
    QVERIFY2(jp::crunch::contentBytes(c) - without >= 2000 + 12, qPrintable(QString::number(jp::crunch::contentBytes(c) - without)));
}

// Saves the window's workbook and reads the saved file's parts.
bool CrunchEditTests::saveParts(EditWindow &e, QMap<QString, QByteArray> *parts)
{
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    if (!e.win.w.save() || !savedOk(saved, 20000)) return false;
    QFile f(e.path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    parts->clear();
    return readZip(f.readAll(), *parts);
}

QByteArray CrunchEditTests::stopgapBook()
{
    const QString rows = QStringLiteral(
        "<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f t=\"shared\" ref=\"B1:B3\" si=\"0\">A1*2</f><v>2</v></c><c r=\"C1\"><f ca=\"1\">NOW()</f><v>45000</v></c><c r=\"D1\"><f>SUM(A1:A3)</f><v>6</v></c></row>"
        "<row r=\"2\"><c r=\"A2\"><v>2</v></c><c r=\"B2\"><f t=\"shared\" si=\"0\"/><v>4</v></c></row>"
        "<row r=\"3\"><c r=\"A3\"><v>3</v></c><c r=\"B3\"><f t=\"shared\" si=\"0\"/><v>6</v></c></row>"
        "<row r=\"6\"><c r=\"A6\"><v>6</v></c><c r=\"B6\"><f>A6+1</f><v>7</v></c><c r=\"F6\"><f t=\"dataTable\" ref=\"F6:F7\" r1=\"A6\"/><v>5</v></c></row>"
        "<row r=\"7\"><c r=\"F7\"><v>8</v></c></row>");
    return sheettest::buildXlsx({sheettest::worksheet(rows)});
}

// A row put in below the first rows, and the file saved: a formula that reads only the rows above keeps its volatile mark and its result; the one that moved is written whole, with its result; and a
// data table is still one (read back).
void CrunchEditTests::aSaveAfterARowWasPutInKeepsWhatDidNotMoveAndWritesWhatDid()
{
    EditWindow e;
    QVERIFY(e.open(stopgapBook()));
    QVERIFY(e.settle());
    QVERIFY(e.win.w.session()->insertRows(0, 4, 1).ok);
    QVERIFY(e.settle());
    QMap<QString, QByteArray> parts;
    QVERIFY(saveParts(e, &parts));
    const QByteArray sheet = parts.value(QStringLiteral("xl/worksheets/sheet1.xml"));
    QVERIFY(!sheet.isEmpty());
    QVERIFY2(sheet.contains("ca=\"1\""), sheet.constData());                      // (the volatile formula)
    QVERIFY2(sheet.contains("<f>SUM(A1:A3)</f><v>6</v>"), sheet.constData());       // (and the result the file had)
    QVERIFY2(sheet.contains("<f>A7+1</f><v>7</v>"), sheet.constData());             // (the one that moved with its row: written whole, with its result)
    QVERIFY2(sheet.contains("t=\"dataTable\"") && !sheet.contains("TABLE("), sheet.constData());   // (a data table stays one)
    Window second;
    QVERIFY(second.open(e.copyOfFile()));
    QCOMPARE(savedFormula(e.path, QStringLiteral("B7")), QStringLiteral("A7+1"));
}

// A formula written whole whose result the window does not have (the worker has not told it) asks for a full calculation when the file is opened, and the saved workbook says so; A formula the saver rewrites (it reads cells that moved) keeps the result the file has for it, and a workbook the worker has worked out does not ask.
void CrunchEditTests::aFormulaWrittenWholeWithNoKnownResultAsksForAFullCalculationOnLoad()
{
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f>A1*2</f><v>2</v></c></row><row r=\"2\"><c r=\"A2\"><v>2</v></c><c r=\"C2\"><f>A2+1</f><v>3</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(sheettest::buildXlsx({sheettest::worksheet(rows)})));
        QVERIFY(e.settle());
        // a formula typed (written whole), the result of which the window's store does not have
        e.select(QStringLiteral("D5"));
        e.enter(QStringLiteral("=A1+A2"));
        QVERIFY(e.settle());
        QVERIFY(const_cast<jp::grid::ValueStore &>(e.win.w.values()).erase(0, 4, 3));
        QVERIFY(e.win.w.session()->insertRows(0, 0, 1).ok);   // (both formulas read cells that moved)
        QVERIFY(e.settle());
        crunch::EditSession::Snapshot snap;
        QString why;
        QVERIFY2(e.win.w.session()->takeSnapshot(&snap, &why), qPrintable(why));
        bool withoutResult = false;
        for (const auto &c : snap.changes.sheets[0].cells) withoutResult = withoutResult || (c.isFormula && !c.hasResult);
        QVERIFY(withoutResult);
        QVERIFY(snap.changes.fullCalcOnLoad);
        QMap<QString, QByteArray> parts;
        QVERIFY(saveParts(e, &parts));
        QVERIFY2(parts.value(QStringLiteral("xl/workbook.xml")).contains("fullCalcOnLoad=\"1\""), parts.value(QStringLiteral("xl/workbook.xml")).constData());
        QVERIFY2(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<f>A3+1</f>"), parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).constData());
        QVERIFY2(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<f>A2*2</f><v>2</v>"), parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).constData());   // (the saver's: the file's result)
    }
}

// A name with every line of it deleted and the sheet renamed after is the same text before the save and after the file is read again (`Sheet1!#REF!`: the dead part keeps the title it was written with), in both
// ways of holding the formulas; a name that is alive follows the rename.
void CrunchEditTests::aNameWithDeadLinesIsTheSameTextAfterASaveAndReopeningAsBefore()
{
    {
        QStringList cells;
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"3\"><c r=\"A3\"><v>3</v></c></row><row r=\"4\"><c r=\"A4\"><v>4</v></c></row><row r=\"5\"><c r=\"A5\"><v>5</v></c></row><row r=\"7\"><c r=\"A7\"><v>7</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(sheettest::buildXlsx({sheettest::worksheet(rows)}, QByteArray(), QString(), QString(), QByteArray(), QStringList(),
                                            QStringLiteral("<definedName name=\"Dead\">Sheet1!$A$3:$A$5</definedName><definedName name=\"Live\">Sheet1!$A$7</definedName>"))));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->deleteRows(0, 2, 3).ok);
        QVERIFY(s->renameSheet(0, QStringLiteral("Edited Sheet")).ok);
        QVERIFY(e.settle());
        QString deadBefore, liveBefore;
        QVERIFY(s->calculation()->nameText(-1, QStringLiteral("Dead"), &deadBefore));
        QVERIFY(s->calculation()->nameText(-1, QStringLiteral("Live"), &liveBefore));
        QCOMPARE(deadBefore, QStringLiteral("Sheet1!#REF!"));
        QMap<QString, QByteArray> parts;
        QVERIFY(saveParts(e, &parts));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QVERIFY(second.w.session() && second.w.session()->calculation());
        QString deadAfter, liveAfter;
        QVERIFY(second.w.session()->calculation()->nameText(-1, QStringLiteral("Dead"), &deadAfter));
        QVERIFY(second.w.session()->calculation()->nameText(-1, QStringLiteral("Live"), &liveAfter));
        QCOMPARE(deadAfter, deadBefore);
        QCOMPARE(liveAfter, liveBefore);
        QVERIFY2(liveBefore.contains(QLatin1String("Edited Sheet")), qPrintable(liveBefore));
    }
}

// Which sheets are written: only those whose formulas can have changed (the sheet edited, and those that name it), and not a sheet where the rows were put in and taken out again. The saver
// rewrites the formulas, so none is among the cells the workbook lists as changed, and the sheet that reads nothing that moved is the file's, byte for byte.
void CrunchEditTests::aSaveWritesNoFormulaOfASheetThatTheEditDidNotMoveNorOfOnePutBack()
{
    EditWindow e;
    const QByteArray one = sheettest::worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f>A1+1</f><v>2</v></c></row><row r=\"2\"><c r=\"A2\"><v>5</v></c><c r=\"B2\"><f>A2*2</f><v>10</v></c></row>"));
    const QByteArray reader = sheettest::worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><f>Sheet1!A2+1</f><v>6</v></c></row>"));
    const QByteArray alone = sheettest::worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>3</v></c><c r=\"B1\"><f>A1+1</f><v>4</v></c></row>"));
    const QByteArray book = sheettest::buildXlsx({one, reader, alone});
    QVERIFY(e.open(book));
    QVERIFY(e.settle());
    crunch::EditSession *s = e.win.w.session();
    QVERIFY(s->insertRows(0, 1, 1).ok);   // between the rows of Sheet1
    QVERIFY(e.settle());
    crunch::EditSession::Snapshot snap;
    QString why;
    QVERIFY2(s->takeSnapshot(&snap, &why), qPrintable(why));
    QVERIFY(snap.changes.rewriteFormulas);
    for (int sheet = 0; sheet < 3; ++sheet) QVERIFY2(!snap.changes.sheets.count(sheet) || snap.changes.sheets[sheet].cells.empty(), qPrintable(QString::number(sheet)));   // (no formula is listed)
    // put back: nothing is written, though a row was put in
    e.win.w.undoStack()->undo();
    QVERIFY(e.settle());
    crunch::EditSession::Snapshot after;
    QVERIFY2(s->takeSnapshot(&after, &why), qPrintable(why));
    QVERIFY(after.changes.edits.empty());
    for (int sheet = 0; sheet < 3; ++sheet) QVERIFY2(!after.changes.sheets.count(sheet) || after.changes.sheets[sheet].cells.empty(), qPrintable(QString::number(sheet)));
    // and the file of a window that kept the row in
    EditWindow f;
    QVERIFY(f.open(book));
    QVERIFY(f.settle());
    QVERIFY(f.win.w.session()->insertRows(0, 1, 1).ok);
    QVERIFY(f.settle());
    QMap<QString, QByteArray> parts;
    QVERIFY(saveParts(f, &parts));
    QMap<QString, QByteArray> original;
    QVERIFY(readZip(book, original));
    QVERIFY(parts.value(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<f>A3*2</f><v>10</v>"));   // (the sheet edited: the formula that reads the row that moved)
    QVERIFY(parts.value(QStringLiteral("xl/worksheets/sheet2.xml")).contains("<f>Sheet1!A3+1</f><v>6</v>"));   // (the sheet that names it)
    QCOMPARE(parts.value(QStringLiteral("xl/worksheets/sheet3.xml")), original.value(QStringLiteral("xl/worksheets/sheet3.xml")));   // (the third reads only itself)
}

CrunchEditTests::Dump CrunchEditTests::dumpOfWindow(EditWindow &e)
{
    Dump d;
    auto hold = e.win.w.workbookLoader().hold();
    const Calculation *calc = hold->calculation();
    for (int i = 0; i < calc->sheetCount(); ++i) dumpSheet(&d, i, calc->sheet(i), calc->sheet(i).cells);
    return d;
}

CrunchEditTests::Dump CrunchEditTests::dumpOfFile(const QString &path)
{
    Dump d;
    Package p;
    QString why;
    if (!p.readLazyFile(path, &why)) return d;
    Workbook b;
    if (!b.open(p, &why)) return d;
    for (int i = 0; i < b.info().sheets.size(); ++i) {
        Sheet s;
        if (b.readSheet(i, &s)) dumpSheet(&d, i, s, s.cells);
    }
    return d;
}

// Two sheets with formulas of every kind of reference, a shared family on each, and the other sheet read.
QByteArray CrunchEditTests::editsBook()
{
    // (built by plain appends: the macOS compiler planted a trap in the earlier form, chained arg() calls inside a conditional appended to the row)
    QString one, two;
    for (int r = 1; r <= 12; ++r) {
        const QString n = QString::number(r);
        one += QLatin1String("<row r=\"") + n + QLatin1String("\"><c r=\"A") + n + QLatin1String("\"><v>") + n + QLatin1String("</v></c><c r=\"B") + n + QLatin1String("\"><v>")
            + QString::number(r * 2) + QLatin1String("</v></c>");
        if (r == 1) one += QLatin1String("<c r=\"C1\"><f t=\"shared\" ref=\"C1:C12\" si=\"0\">A1+B1</f><v>3</v></c>");
        else one += QLatin1String("<c r=\"C") + n + QLatin1String("\"><f t=\"shared\" si=\"0\"/><v>") + QString::number(r * 3) + QLatin1String("</v></c>");
        one += QLatin1String("<c r=\"D") + n + QLatin1String("\"><f>SUM(A$1:A") + n + QLatin1String(")</f><v>1</v></c><c r=\"E") + n + QLatin1String("\"><f>Sheet2!A")
            + QString::number(1 + r % 8) + QLatin1String("*2</f><v>1</v></c></row>");
    }
    one += QStringLiteral("<row r=\"13\"><c r=\"A13\"><f>SUM(Sheet2!A:A)</f><v>1</v></c><c r=\"B13\"><f>SUM(B1:B12)</f><v>1</v></c><c r=\"C13\"><f>$A$1+C$5</f><v>1</v></c><c r=\"D13\"><f ca=\"1\">IF(A1&gt;5,B2,B3)+NOW()</f><v>1</v></c><c r=\"E13\"><f>COUNT(2:4)</f><v>1</v></c></row>");
    for (int r = 1; r <= 8; ++r) {
        const QString n = QString::number(r);
        two += QLatin1String("<row r=\"") + n + QLatin1String("\"><c r=\"A") + n + QLatin1String("\"><v>") + n + QLatin1String("</v></c>");
        if (r == 1) two += QLatin1String("<c r=\"B1\"><f t=\"shared\" ref=\"B1:B8\" si=\"0\">Sheet1!A1+A1</f><v>2</v></c>");
        else two += QLatin1String("<c r=\"B") + n + QLatin1String("\"><f t=\"shared\" si=\"0\"/><v>") + QString::number(r * 2) + QLatin1String("</v></c>");
        two += QLatin1String("<c r=\"C") + n + QLatin1String("\"><f>SUM(Sheet1!B1:B12)+A") + n + QLatin1String("</f><v>1</v></c><c r=\"D") + n + QLatin1String("\"><f>A") + n + QLatin1String("+B") + n
            + QLatin1String("</f><v>1</v></c></row>");
    }
    return sheettest::buildXlsx({sheettest::worksheet(one), sheettest::worksheet(two)});
}

// Makes `count` random edits of rows and columns in the window (Undo and Redo among them), the file saved after every `saveEvery` of them (0: only at the end); returns whether the file read again
// has the formulas and numbers the window has. With the formulas in ids and no save before the end, no formula is among the cells the workbook lists as changed.
void CrunchEditTests::randomEditsThenSave(quint32 seed, int count, int saveEvery)
{
    EditWindow e;
    QVERIFY(e.open(editsBook()));
    QVERIFY(e.settle());
    crunch::EditSession *s = e.win.w.session();
    quint32 state = seed * 2654435761u + 12345u;
    auto next = [&](quint32 below) {
        state = state * 1664525u + 1013904223u;
        return (state >> 8) % below;
    };
    int made = 0;
    for (int i = 0; i < count; ++i) {
        const int sheet = int(next(2));
        const quint32 count1 = 1 + next(2);
        const quint32 pick = next(16);
        if (pick < 6) made += s->insertRows(sheet, next(16), count1).ok ? 1 : 0;
        else if (pick < 8) made += s->deleteRows(sheet, next(40), count1).ok ? 1 : 0;
        else if (pick < 11) made += s->insertColumns(sheet, next(7), count1).ok ? 1 : 0;
        else if (pick < 12) made += s->deleteColumns(sheet, next(30), count1).ok ? 1 : 0;
        else if (pick < 14) e.win.w.undoStack()->undo();
        else e.win.w.undoStack()->redo();
        if (saveEvery > 0 && i % saveEvery == saveEvery - 1 && i + 1 < count) {
            QVERIFY2(e.settle(), qPrintable(QStringLiteral("seed %1, after edit %2: calculating %3").arg(seed).arg(i).arg(e.win.w.isCalculating())));
            QMap<QString, QByteArray> parts;
            QVERIFY(saveParts(e, &parts));
        }
    }
    QVERIFY(made > count / 4);
    QVERIFY(e.settle());
    if (saveEvery == 0) {
        crunch::EditSession::Snapshot snap;
        QString why;
        QVERIFY2(s->takeSnapshot(&snap, &why), qPrintable(why));
        QVERIFY(snap.changes.rewriteFormulas);
        for (const auto &entry : snap.changes.sheets)
            for (const auto &c : entry.second.cells) QVERIFY2(!c.isFormula, qPrintable(QStringLiteral("sheet %1 row %2 col %3").arg(entry.first).arg(c.row).arg(c.col)));
    }
    QMap<QString, QByteArray> parts;
    QVERIFY(saveParts(e, &parts));
    const Dump live = dumpOfWindow(e), file = dumpOfFile(e.path);
    QVERIFY2(live.formulas.size() > 10, qPrintable(QString::number(live.formulas.size())));
    for (auto it = live.formulas.begin(); it != live.formulas.end(); ++it)
        QVERIFY2(file.formulas.value(it.key(), QStringLiteral("-")) == it.value(), qPrintable(QStringLiteral("seed %1, cell %2: the window has '%3', the file '%4'").arg(seed).arg(it.key(), it.value(), file.formulas.value(it.key(), QStringLiteral("-")))));
    QCOMPARE(file.formulas.size(), live.formulas.size());
    QCOMPARE(file.numbers, live.numbers);
}

// Five hundred random edits of rows and columns on a small workbook, saved once and read again, every formula and every number of the file is the window's. The workbook lists no formula among the cells it changed (the saver writes them).
void CrunchEditTests::fiveHundredRandomEditsSavedAndReadAgainAreTheWindowsState()
{
    for (const quint32 seed : {1u, 2u}) randomEditsThenSave(seed, 500, 0);
}

// The same with the file saved every thirty edits: each save is of the edits since the one before, from the file as that left it, and Undo made after a save is of the rows the file lost.
void CrunchEditTests::editsSavedInStagesGiveTheWindowsStateEachTime()
{
    for (const quint32 seed : {3u, 4u}) randomEditsThenSave(seed, 300, 30);
}

// A row put in and the file saved, and the row taken out again by Undo: the next save has the formulas as they were before it (a delete that a save has seen cannot be undone, so only a put-in row comes
// out this way).
void CrunchEditTests::aRowPutInAndSavedThenUndoneWritesTheFormulasAsTheyWere()
{
    {
        EditWindow e;
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f>SUM(A1:A5)</f><v>15</v></c><c r=\"C1\"><f>A3*2</f><v>6</v></c></row>"
                                            "<row r=\"2\"><c r=\"A2\"><v>2</v></c></row><row r=\"3\"><c r=\"A3\"><v>3</v></c></row><row r=\"4\"><c r=\"A4\"><v>4</v></c></row><row r=\"5\"><c r=\"A5\"><v>5</v></c></row>");
        QVERIFY(e.open(sheettest::buildXlsx({sheettest::worksheet(rows)})));
        QVERIFY(e.settle());
        QVERIFY(e.win.w.session()->insertRows(0, 2, 1).ok);
        QVERIFY(e.settle());
        QMap<QString, QByteArray> parts;
        QVERIFY(saveParts(e, &parts));
        QCOMPARE(savedFormula(e.path, QStringLiteral("C1")), QStringLiteral("A4*2"));
        QCOMPARE(savedFormula(e.path, QStringLiteral("B1")), QStringLiteral("SUM(A1:A6)"));
        e.win.w.undoStack()->undo();
        QVERIFY(e.settle());
        QVERIFY(saveParts(e, &parts));
        QVERIFY2(savedFormula(e.path, QStringLiteral("C1")) == QStringLiteral("A3*2"), qPrintable(savedFormula(e.path, QStringLiteral("C1"))));
        QCOMPARE(savedFormula(e.path, QStringLiteral("B1")), QStringLiteral("SUM(A1:A5)"));
    }
}

// After a save the next edits are made against the file as it is: a row put in, saved, and another put in, saved, is what one save of both would give.
void CrunchEditTests::twoSavesOfRowsPutInGiveWhatOneSaveGives()
{
    QByteArray last;
    {
        for (const int stages : {1, 2}) {
            EditWindow e;
            QVERIFY(e.open(stopgapBook()));
            QVERIFY(e.settle());
            QVERIFY(e.win.w.session()->insertRows(0, 4, 1).ok);
            QMap<QString, QByteArray> parts;
            if (stages == 2) {
                QVERIFY(e.settle());
                QVERIFY(saveParts(e, &parts));
            }
            QVERIFY(e.win.w.session()->insertRows(0, 0, 2).ok);
            QVERIFY(e.settle());
            QVERIFY(saveParts(e, &parts));
            const QByteArray sheet = parts.value(QStringLiteral("xl/worksheets/sheet1.xml"));
            QVERIFY2(sheet.contains("<f>A9+1</f><v>7</v>") && sheet.contains("<f>SUM(A3:A5)</f><v>6</v>") && sheet.contains("ca=\"1\""), sheet.constData());
            if (stages == 1) last = sheet;
            else QCOMPARE(sheet, last);
        }
    }
}


// ---- round 7 review: reference through several sheets, cached values, and the families a sheet keeps
// The cached value of a cell as the saved sheet's XML has it (the bytes, not what the program reads from them); "" for none.
QString CrunchEditTests::rawValue(const QMap<QString, QByteArray> &parts, const QString &ref, int sheet)
{
    const QString xml = QString::fromUtf8(parts.value(QStringLiteral("xl/worksheets/sheet%1.xml").arg(sheet)));
    const QRegularExpression re(QStringLiteral("<c r=\"%1\"[^>]*>(?:(?!</c>).)*?<v>([^<]*)</v>").arg(ref), QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch m = re.match(xml);
    return m.hasMatch() ? m.captured(1) : QString();
}

QByteArray CrunchEditTests::sumBook()
{
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\"><c r=\"A2\"><v>2</v></c></row><row r=\"3\"><c r=\"A3\"><v>3</v></c></row>"
                                        "<row r=\"4\"><c r=\"A4\"><f>SUM(A1:A3)</f><v>6</v></c><c r=\"B4\"><f>A4*2</f><v>12</v></c></row>");
    return sheettest::buildXlsx({sheettest::worksheet(rows)});
}

// A reference through several sheets ("Sheet1:Sheet3!A1:A3") follows the rows put in and taken out of the sheets it spans, in the file as it does in the window (the saver follows it edit by edit, as the
// window does, since the sheets have each an order of their own).
void CrunchEditTests::aReferenceThroughSeveralSheetsFollowsTheEditsInTheSavedFile()
{
    {
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"2\"><c r=\"A2\"><v>2</v></c></row><row r=\"3\"><c r=\"A3\"><v>3</v></c></row>");
        const QString third = rows + QStringLiteral("<row r=\"4\"><c r=\"B4\"><f>SUM(Sheet1:Sheet3!A1:A3)</f><v>18</v></c><c r=\"C4\"><f>Sheet1:Sheet3!A2+A1</f><v>5</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(sheettest::buildXlsx({sheettest::worksheet(rows), sheettest::worksheet(rows), sheettest::worksheet(third)})));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(s->insertRows(1, 1, 1).ok);   // inside the range, on the sheet between
        QVERIFY(s->deleteRows(0, 0, 1).ok);   // the first row of the first sheet
        QVERIFY(s->insertColumns(2, 0, 1).ok);
        QVERIFY(e.settle());
        QMap<QString, QByteArray> parts;
        QVERIFY(saveParts(e, &parts));
        const Dump live = dumpOfWindow(e), file = dumpOfFile(e.path);
        QVERIFY(live.formulas.size() >= 2);
        QCOMPARE(live.formulas.value(QStringLiteral("2 3 3")), QStringLiteral("Sheet1:Sheet3!A2+B1"));   // (the reference through the sheets stays; the plain one follows the column put in)
        for (auto it = live.formulas.begin(); it != live.formulas.end(); ++it)
            QVERIFY2(file.formulas.value(it.key(), QStringLiteral("-")) == it.value(), qPrintable(QStringLiteral("cell %1: the window has '%2', the file '%3'").arg(it.key(), it.value(), file.formulas.value(it.key(), QStringLiteral("-")))));
    }
}

// The cached value of a formula in the file is the window's: a number typed into a row put in inside a sum, and a summed row deleted (read from the bytes of the saved sheet).
void CrunchEditTests::theCachedValueOfAMovedFormulaIsTheWindowsAfterATypedRowAndADeletedRow()
{
    {
        {
            EditWindow e;
            QVERIFY(e.open(sumBook()));
            QVERIFY(e.settle());
            crunch::EditSession *s = e.win.w.session();
            QVERIFY(s->insertRows(0, 1, 1).ok);
            QVERIFY(s->type(0, 1, 0, QStringLiteral("10")).ok);
            QVERIFY(e.settle());
            QCOMPARE(e.valueAt(QStringLiteral("A5")), QStringLiteral("n:16"));
            QMap<QString, QByteArray> parts;
            QVERIFY(saveParts(e, &parts));
            QVERIFY2(rawValue(parts, QStringLiteral("A5"), 1) == QStringLiteral("16") && rawValue(parts, QStringLiteral("B5"), 1) == QStringLiteral("32"), qPrintable(QStringLiteral("%1 %2").arg(rawValue(parts, QStringLiteral("A5"), 1), rawValue(parts, QStringLiteral("B5"), 1))));
        }
        {
            EditWindow e;
            QVERIFY(e.open(sumBook()));
            QVERIFY(e.settle());
            QVERIFY(e.win.w.session()->deleteRows(0, 1, 1).ok);
            QVERIFY(e.settle());
            QCOMPARE(e.valueAt(QStringLiteral("A3")), QStringLiteral("n:4"));
            QMap<QString, QByteArray> parts;
            QVERIFY(saveParts(e, &parts));
            QVERIFY2(rawValue(parts, QStringLiteral("A3"), 1) == QStringLiteral("4") && rawValue(parts, QStringLiteral("B3"), 1) == QStringLiteral("8"), qPrintable(QStringLiteral("%1 %2").arg(rawValue(parts, QStringLiteral("A3"), 1), rawValue(parts, QStringLiteral("B3"), 1))));
        }
    }
}

// With the calculation by hand the worker does not work a moved formula out again: the file either holds the window's value or asks to be worked out when it is opened.
void CrunchEditTests::aFormulaNotWorkedOutAfterAMoveHasTheRightValueOrAsksForACalculation()
{
    {
        EditWindow e;
        QVERIFY(e.open(sumBook()));
        QVERIFY(e.settle());
        crunch::EditSession *s = e.win.w.session();
        crunch::CalcSettings now = s->calcSettings();
        now.mode = jp::office::sheet::CalcMode::Manual;
        QVERIFY(s->setCalcSettings(now).ok);
        QVERIFY(s->insertRows(0, 1, 1).ok);
        QVERIFY(s->type(0, 1, 0, QStringLiteral("10")).ok);
        QVERIFY(s->deleteRows(0, 0, 1).ok);
        QCoreApplication::processEvents();   // (the worker does not work in manual mode: there is nothing to wait for)
        QMap<QString, QByteArray> parts;
        QVERIFY(saveParts(e, &parts));
        const bool asks = parts.value(QStringLiteral("xl/workbook.xml")).contains("fullCalcOnLoad=\"1\"");
        const QString window = e.valueAt(QStringLiteral("A4")).mid(2);   // (the sum, now in row 4)
        const QString file = rawValue(parts, QStringLiteral("A4"), 1);
        QVERIFY2(asks || file == window, qPrintable(QStringLiteral("the file has %1, the window %2, and does not ask for a calculation").arg(file, window)));
    }
}

// ---- the results the session keeps for the saver
// A text that is overwritten, or taken out, gives its place back: texts that come and go (a formula that gives a new text at every calculation) do not pile up.
void CrunchEditTests::theResultTableReusesThePlaceOfATextThatIsOverwrittenOrTakenOut()
{
    using jp::calc::Value;
    jp::crunch::ResultMap m;
    for (int i = 0; i < 100000; ++i) m.set(42, Value::text(QStringLiteral("text number %1").arg(i)));
    QCOMPARE(m.size(), size_t(1));
    QCOMPARE(m.value(42).text(), QStringLiteral("text number 99999"));
    QVERIFY2(m.textPlaces() <= 2, qPrintable(QString::number(m.textPlaces())));
    for (int i = 0; i < 100000; ++i) {
        m.set(quint64(1000 + i % 50), Value::text(QStringLiteral("t%1").arg(i)));
        if (i % 3 == 0) m.erase(quint64(1000 + (i + 7) % 50));
    }
    QVERIFY2(m.textPlaces() <= 60, qPrintable(QString::number(m.textPlaces())));
    for (int i = 0; i < 100000; ++i) m.set(5, Value::number(i));   // (a number over a text frees the text)
    QVERIFY(m.textPlaces() <= 60);
    QCOMPARE(m.value(5).number(), 99999.0);
}

// The values it keeps come back as they were, an entry taken out while walking the table does not hide the next, and the two keys it uses for itself are refused, not found, and not erased.
void CrunchEditTests::theResultTableKeepsValuesAndRefusesItsOwnKeys()
{
    using jp::calc::Value;
    using jp::calc::ErrorKind;
    jp::crunch::ResultMap m;
    QVERIFY(m.set(1, Value::number(2.5)));
    QVERIFY(m.set(2, Value::boolean(true)));
    QVERIFY(m.set(3, Value::error(ErrorKind::Ref)));
    QVERIFY(m.set(4, Value::text(QStringLiteral("x"))));
    QVERIFY(m.set(5, Value()));
    QVERIFY(m.find(1)->second == Value::number(2.5) && m.find(2)->second == Value::boolean(true) && m.find(3)->second == Value::error(ErrorKind::Ref) && m.find(4)->second == Value::text(QStringLiteral("x")) && m.find(5)->second.isEmpty());
    QVERIFY(!m.set(jp::crunch::ResultMap::kEmptyKey, Value::number(1)));
    QVERIFY(!m.set(jp::crunch::ResultMap::kDeadKey, Value::number(1)));
    for (const quint64 k : {jp::crunch::ResultMap::kEmptyKey, jp::crunch::ResultMap::kDeadKey}) {
        QVERIFY(m.find(k) == m.end());
        QCOMPARE(m.count(k), size_t(0));
        QCOMPARE(m.erase(k), size_t(0));
    }
    QCOMPARE(m.size(), size_t(5));
    for (quint64 k = 100; k < 5000; ++k) m.set(k, Value::number(double(k)));
    size_t seen = 0;
    for (auto it = m.begin(); it != m.end();) {
        ++seen;
        it = it->first % 2 == 0 ? m.erase(it) : std::next(it);
    }
    QCOMPARE(seen, size_t(4905));
    QCOMPARE(m.size(), size_t(4905 - 2450 - 2));   // (the even keys of 100 to 4999, and the keys 2 and 4)
    for (quint64 k = 100; k < 5000; ++k) QCOMPARE(m.count(k), size_t(k % 2));
}

// What a million entries cost: at most 45 bytes an entry (a node of a tree holding a whole value is about 106).
void CrunchEditTests::theResultTableCostsUnderHalfWhatATreeOfValuesDoes()
{
    jp::crunch::ResultMap m;
    for (quint64 k = 0; k < 1000000; ++k) m.set((quint64(1) << 40) | k, jp::calc::Value::number(double(k)));
    const double perEntry = double(m.slotCount()) * 17.0 / double(m.size());
    QVERIFY2(perEntry <= 45.0, qPrintable(QString::number(perEntry)));
}


// A circle of a workbook with iteration on, an edit of what it reads, and the edit undone and done again through the session: the circle is gone round from the values it had each time (A1 = IF(B1=0,0,1/C1)+1
// and B1 = A1*0+3 are the circle, C1 = F1*2 is read by it), never given an error that it would feed itself.
void CrunchEditTests::anEditOfWhatACircleReadsAndItsUndoAndRedoGoRoundFromTheValuesItHad()
{
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><f>IF(B1=0,0,1/C1)+1</f><v>1.2</v></c><c r=\"B1\"><f>A1*0+3+F1*0</f><v>3</v></c><c r=\"C1\"><f>F1*2</f><v>5</v></c><c r=\"E1\"><f>A1+B1</f><v>4.2</v></c><c r=\"F1\"><v>2.5</v></c></row>");
    EditWindow e;
    QVERIFY(e.open(sheettest::buildXlsx({sheettest::worksheet(rows)}, QByteArray(), QString(), QStringLiteral("iterate=\"1\" iterateCount=\"100\" iterateDelta=\"0.00001\""))));
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1.2"));
    const qint64 before = e.win.w.session()->calculation()->staleReadCorrections();
    QVERIFY(e.win.w.session()->type(0, 0, 5, QStringLiteral("5")).ok);   // F1 = 5: C1 = 10, A1 = 1.1 (the circle reads F1 as well as C1, and is worked out first)
    QVERIFY(e.settle());
    QVERIFY2(e.win.w.session()->calculation()->staleReadCorrections() > before, "the circle read C1 before it was worked out");
    QCOMPARE(e.valueAt(QStringLiteral("C1")), QStringLiteral("n:10"));
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1.1"));
    QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("n:4.1"));
    e.win.w.undoStack()->undo();
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1.2"));
    QCOMPARE(e.valueAt(QStringLiteral("E1")), QStringLiteral("n:4.2"));
    e.win.w.undoStack()->redo();
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1.1"));
    e.win.w.undoStack()->undo();
    QVERIFY(e.settle());
    QCOMPARE(e.valueAt(QStringLiteral("A1")), QStringLiteral("n:1.2"));
}


void CrunchEditTests::aWorkbookWithMacrosKeepsThem()
{
    QByteArray macroBook;
    {
        // the editBook, with a macro-enabled main part and a VBA project
        const QByteArray plain = editBook();
        QMap<QString, QByteArray> parts;
        QVERIFY(readZip(plain, parts));
        ZipWriter z;
        for (auto it = parts.begin(); it != parts.end(); ++it) {
            QByteArray data = it.value();
            if (it.key() == QLatin1String("[Content_Types].xml"))
                data.replace("spreadsheetml.sheet.main+xml", "ms-excel.sheet.macroEnabled.main+xml").replace("</Types>", "<Override PartName=\"/xl/vbaProject.bin\" ContentType=\"application/vnd.ms-office.vbaProject\"/></Types>");
            z.add(it.key(), data, true);
        }
        z.add(QStringLiteral("xl/vbaProject.bin"), QByteArray("VBA project bytes \x01\x02\x03 that must not change", 40), true);
        macroBook = z.finish();
    }
    EditWindow e;
    QVERIFY(e.open(macroBook, QStringLiteral("macros.xlsm")));
    e.select(QStringLiteral("A1"));
    e.enter(QStringLiteral("2"));
    QVERIFY(e.settle());
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(!e.win.w.saveAs(e.folder.dir.filePath(QStringLiteral("macros.xlsb"))));
    QVERIFY(e.win.told.last().contains(QLatin1String(".xlsm")));
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved, 20000), "the save failed");
    QFile f(e.path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray now = f.readAll();
    QCOMPARE(rawParts(now).value(QStringLiteral("xl/vbaProject.bin")), rawParts(macroBook).value(QStringLiteral("xl/vbaProject.bin")));
    QVERIFY(entriesOf(now).value(QStringLiteral("[Content_Types].xml")).contains("macroEnabled"));
}

// A workbook like a real one's package: the editBook, a calculation chain, a theme, and a part nothing reads.
QByteArray CrunchEditTests::editBookWithParts()
{
    const QByteArray plain = editBook();
    QMap<QString, QByteArray> parts;
    readZip(plain, parts);
    ZipWriter z;
    for (auto it = parts.begin(); it != parts.end(); ++it) {
        QByteArray data = it.value();
        if (it.key() == QLatin1String("[Content_Types].xml"))
            data.replace("</Types>", "<Override PartName=\"/xl/calcChain.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.calcChain+xml\"/></Types>");
        if (it.key() == QLatin1String("xl/_rels/workbook.xml.rels"))
            data.replace("</Relationships>", "<Relationship Id=\"rId90\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/calcChain\" Target=\"calcChain.xml\"/></Relationships>");
        z.add(it.key(), data, true);
    }
    z.add(QStringLiteral("xl/calcChain.xml"), QByteArray("<calcChain xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><c r=\"B1\" i=\"1\"/></calcChain>"), true);
    z.add(QStringLiteral("xl/theme/theme1.xml"), QByteArray("<a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"/>"), true);
    z.add(QStringLiteral("customXml/item1.xml"), QByteArray("<x>not read here</x>"), true);
    return z.finish();
}

QMap<QString, QByteArray> CrunchEditTests::entriesOf(const QByteArray &zip)
{
    QMap<QString, QByteArray> out;
    readZip(zip, out);
    return out;
}

QMap<QString, QByteArray> CrunchEditTests::rawParts(const QByteArray &zip)
{
    QMap<QString, QByteArray> out;
    QVector<ZipEntry> dir;
    if (readZipDirectory(zip, &dir))
        for (const ZipEntry &e : dir) out.insert(e.name, zip.mid(int(e.dataAt), int(e.csize)));
    return out;
}
