// Tests of JeffCrunch's window with a workbook of the older binary format (.xls): it opens and shows, the formulas are worked out, what is not read is said, and saving asks for a
// new name ending in .xlsx and never offers the file the workbook came from. Every workbook is built here, in a temporary folder.
#include "../sheet/xlshelpers.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "savedok.h"
#include "app/oldformat.h"
#include "app/settings.h"
#include "app/sheettabs.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "grid/gridview.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace jp;

namespace {

struct Window {
    CrunchWindow w;
    QStringList told;
    Window()
    {
        w.setNotifier([this](const QString &title, const QString &text) { told << title + QStringLiteral(": ") + text; });
        w.resize(1100, 640);
        w.show();
    }
    bool open(const QString &path)
    {
        QSignalSpy first(&w, &CrunchWindow::firstScreenShown), done(&w, &CrunchWindow::calculationFinished), loaded(&w, &CrunchWindow::loadFinished);
        if (!w.openFile(path)) return false;
        return first.wait(20000) && (loaded.count() > 0 || loaded.wait(20000)) && (done.count() > 0 || done.wait(20000));
    }
};

struct Folder {
    QTemporaryDir dir;
    QString write(const QString &name, const QByteArray &bytes)
    {
        const QString path = dir.filePath(name);
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly)) return QString();
        f.write(bytes);
        return path;
    }
};

// The keys of a standard command on this computer (Cmd+S on a Mac, Ctrl+S elsewhere), pressed in the window.
void pressStandard(QWidget *w, QKeySequence::StandardKey command)
{
    const QKeySequence keys(command);
    const QKeyCombination k = keys[0];
    QTest::keyClick(w, k.key(), k.keyboardModifiers());
}

// A window with a workbook of the old form (or another) open from a folder of its own, a file dialog that answers what the test says and remembers what it was asked, and what was
// said to the person.
struct OldWindow {
    Folder folder;
    Window win;
    QString path;
    QByteArray original;
    QString answer;
    QStringList asked, filters;
    OldWindow()
    {
        win.w.setSavePicker([this](const QString &start, const QString &filter) {
            asked << start;
            filters << filter;
            return answer;
        });
        // (a question about a file another program changed is never asked of these: if it were, the test would wait for a person)
        win.w.setChangedAsker([this](const QString &name) {
            told << QStringLiteral("changed: ") + name;
            return CrunchWindow::ChangedAnswer::Cancel;
        });
    }
    QStringList &told = win.told;
    bool open(const QByteArray &bytes, const QString &name = QStringLiteral("old.xls"))
    {
        original = bytes;
        path = folder.write(name, bytes);
        return win.open(path) && activate();
    }
    // The window is the active one, as shortcuts are looked for in the active window only.
    bool activate()
    {
        win.w.activateWindow();
        return QTest::qWaitForWindowActive(&win.w);
    }
    grid::GridView *view() { return win.w.view(); }
    void select(quint32 row, quint32 col) { view()->selectCell(row, col); }
    // Types into the selected cell as a person does: the first character at the grid, the rest into the cell's line, then Enter.
    void enter(const QString &text)
    {
        QTest::keyClick(view(), text.at(0).toLatin1());
        if (text.size() > 1) QTest::keyClicks(win.w.cellEditor(), text.mid(1));
        QTest::keyClick(win.w.cellEditor(), Qt::Key_Return);
    }
    bool settle() { return QTest::qWaitFor([&] { return !win.w.isCalculating(); }, 20000); }
    bool originalIsUntouched() const
    {
        QFile f(path);
        return f.open(QIODevice::ReadOnly) && f.readAll() == original;
    }
    // A copy of a file written, under another name (a file that is open in a window is not opened in a second: the copy is).
    QString copyOfSaved(const QString &name, const QString &copy)
    {
        QFile f(folder.dir.filePath(name));
        if (!f.open(QIODevice::ReadOnly)) return QString();
        return folder.write(copy, f.readAll());
    }
};

// A workbook of the old form: A1 = 2, A2 = 3, A3 = SUM(A1:A2) (its stored result wrong on purpose), and the text "hello" in B1.
QByteArray oldBook()
{
    xlstest::Book b;
    b.after(xlstest::sst({QStringLiteral("hello")}));
    b.sheet(QStringLiteral("S"), xlstest::number(0, 0, 2) + xlstest::labelSst(0, 1, 0) + xlstest::number(1, 0, 3) + xlstest::formulaNumber(2, 0, xlstest::tArea(0, 0, 1, 0) + xlstest::tFuncVar(4, 1, 0x40), 99));
    return b.file();
}

} // namespace

class CrunchXlsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        static QTemporaryDir settings;
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        setAppInfo(crunchAppInfo());
        initCore();
    }

    // A workbook of the older form opens as a new one does: its sheets as tabs, its cells shown, its formulas worked out from the constants (the stored result here is wrong on
    // purpose), and nothing said when there is nothing it does not read.
    void anOldWorkbookOpensAndItsFormulasAreWorkedOut()
    {
        Folder folder;
        Window win;
        xlstest::Book b;
        b.sheet(QStringLiteral("Data"), xlstest::number(0, 0, 2) + xlstest::number(1, 0, 3) + xlstest::formulaNumber(2, 0, xlstest::tArea(0, 0, 1, 0) + xlstest::tFuncVar(4, 1, 0x40), 99));
        b.sheet(QStringLiteral("Hidden"), xlstest::number(0, 0, 1), 1);
        QVERIFY(win.open(folder.write(QStringLiteral("old.xls"), b.file())));
        QVERIFY(win.w.hasWorkbook());
        QCOMPARE(win.w.shownSheets().size(), 1);   // (the hidden sheet is kept out)
        grid::GridView *v = win.w.view();
        v->selectCell(2, 0);
        QCOMPARE(v->activeCellText(), QStringLiteral("=SUM(A1:A2)"));
        QCOMPARE(v->sheet().valueOf(*v->sheet().cellAt(2, 0)).debug(), QStringLiteral("n:5"));
        QVERIFY2(win.told.isEmpty(), qPrintable(win.told.join(QLatin1Char('\n'))));
    }

    // What the file has that is not read is said once, in plain words, as the workbook opens.
    void whatIsNotReadIsSaid()
    {
        Folder folder;
        Window win;
        xlstest::Book b;
        b.sheet(QStringLiteral("Data"), xlstest::number(0, 0, 2) + xlstest::rec(0x005D, QByteArray(20, '\0')) + xlstest::rec(0x001C, QByteArray(20, '\0')));
        QVERIFY(win.open(folder.write(QStringLiteral("pictures.xls"), b.file())));
        QCOMPARE(win.told.size(), 1);
        QVERIFY2(win.told.first().contains(QLatin1String("pictures and other drawing objects")), qPrintable(win.told.first()));
        QVERIFY2(win.told.first().contains(QLatin1String("comments")), qPrintable(win.told.first()));
        QVERIFY(win.w.hasWorkbook());
    }

    // A workbook that asks for a password is refused in words, and the window stays ready for another.
    void anEncryptedWorkbookIsRefused()
    {
        Folder folder;
        Window win;
        xlstest::Book b;
        b.globals(xlstest::rec(0x002F, xlstest::W().u16(1).u16(1).u16(0x1234).u16(0x5678)));
        b.sheet(QStringLiteral("S"), xlstest::number(0, 0, 1));
        QSignalSpy loaded(&win.w, &CrunchWindow::loadFinished);
        QVERIFY(win.w.openFile(folder.write(QStringLiteral("locked.xls"), b.file())));
        QVERIFY(loaded.wait(20000));
        QCOMPARE(loaded.first().first().toBool(), false);
        QVERIFY(!win.w.hasWorkbook());
        QCOMPARE(win.told.size(), 1);
        QVERIFY2(win.told.first().contains(QLatin1String("encrypted")), qPrintable(win.told.first()));
    }

    // Save (by the keys of the computer) on a workbook that came from an .xls file: the person is told in plain words that it is saved as .xlsx and asked for a name (offered with that
    // ending, never the file it came from); the .xlsx is written with the edit, the .xls is as it was, and the next Save goes to the .xlsx with no question.
    void saveAsksForAnXlsxNameAndNeverWritesTheOriginal()
    {
        OldWindow o;
        QVERIFY(o.open(oldBook()));
        o.select(0, 0);
        o.enter(QStringLiteral("10"));
        QVERIFY(o.settle());
        QVERIFY(o.win.w.isModified());
        o.answer = o.folder.dir.filePath(QStringLiteral("copy"));
        QSignalSpy saved(&o.win.w, &CrunchWindow::saved);
        pressStandard(&o.win.w, QKeySequence::Save);
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QCOMPARE(o.asked, QStringList{o.folder.dir.filePath(QStringLiteral("old.xlsx"))});
        QVERIFY2(o.filters.first().contains(QLatin1String("*.xlsx")) && !o.filters.first().contains(QLatin1String("*.xls)")), qPrintable(o.filters.first()));
        QCOMPARE(saved.first().at(0).toString(), o.folder.dir.filePath(QStringLiteral("copy.xlsx")));
        QCOMPARE(saved.first().at(1).toBool(), true);
        QCOMPARE(o.told.size(), 1);
        QVERIFY2(o.told.first().contains(QLatin1String("old.xls")) && o.told.first().contains(QLatin1String(".xlsx")) && o.told.first().contains(QLatin1String("not be changed")), qPrintable(o.told.first()));
        QVERIFY(o.originalIsUntouched());
        QCOMPARE(o.win.w.filePath(), o.folder.dir.filePath(QStringLiteral("copy.xlsx")));
        QVERIFY(!o.win.w.isModified());
        // the file written holds the edit, the cells that were not touched, and the formula with its result
        {
            Window second;
            QVERIFY(second.open(o.copyOfSaved(QStringLiteral("copy.xlsx"), QStringLiteral("again1.xlsx"))));
            grid::GridView *v = second.w.view();
            QCOMPARE(second.w.shownSheets().size(), 1);
            QCOMPARE(v->sheet().valueOf(*v->sheet().cellAt(0, 0)).debug(), QStringLiteral("n:10"));
            QCOMPARE(v->sheet().valueOf(*v->sheet().cellAt(1, 0)).debug(), QStringLiteral("n:3"));
            v->selectCell(2, 0);
            QCOMPARE(v->activeCellText(), QStringLiteral("=SUM(A1:A2)"));
            QCOMPARE(v->sheet().valueOf(*v->sheet().cellAt(2, 0)).debug(), QStringLiteral("n:13"));
            QCOMPARE(v->sheet().textOf(*v->sheet().cellAt(0, 1)), QStringLiteral("hello"));
        }
        // the next edit is saved over the .xlsx, with no question and no explanation
        QVERIFY(o.activate());   // (the window that read the file closed, and no window is the active one)
        o.select(1, 0);
        o.enter(QStringLiteral("20"));
        QVERIFY(o.settle());
        pressStandard(&o.win.w, QKeySequence::Save);
        QTRY_COMPARE_WITH_TIMEOUT(saved.count(), 2, 20000);
        QCOMPARE(saved.last().at(0).toString(), o.folder.dir.filePath(QStringLiteral("copy.xlsx")));
        QCOMPARE(saved.last().at(1).toBool(), true);
        QCOMPARE(o.asked.size(), 1);
        QCOMPARE(o.told.size(), 1);
        QVERIFY(o.originalIsUntouched());
        {
            Window third;
            QVERIFY(third.open(o.copyOfSaved(QStringLiteral("copy.xlsx"), QStringLiteral("again2.xlsx"))));
            grid::GridView *v = third.w.view();
            QCOMPARE(v->sheet().valueOf(*v->sheet().cellAt(0, 0)).debug(), QStringLiteral("n:10"));
            QCOMPARE(v->sheet().valueOf(*v->sheet().cellAt(1, 0)).debug(), QStringLiteral("n:20"));
            QCOMPARE(v->sheet().valueOf(*v->sheet().cellAt(2, 0)).debug(), QStringLiteral("n:30"));
        }
    }

    // Save with nothing changed still asks (the workbook has no .xlsx file yet), and a person who cancels the question writes nothing and is asked again next time.
    void saveWithNothingChangedAsksAndACancelWritesNothing()
    {
        OldWindow o;
        QVERIFY(o.open(oldBook()));
        QVERIFY(!o.win.w.isModified());
        o.answer = QString();   // (cancelled)
        QSignalSpy saved(&o.win.w, &CrunchWindow::saved);
        QVERIFY(!o.win.w.save());
        QCOMPARE(o.asked.size(), 1);
        QCOMPARE(saved.count(), 0);
        QCOMPARE(o.told.size(), 1);
        QVERIFY(o.originalIsUntouched());
        QVERIFY(!QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("old.xlsx"))));
        QCOMPARE(o.win.w.filePath(), o.path);
        // and the next Save asks, and tells, again
        o.answer = o.folder.dir.filePath(QStringLiteral("now.xlsx"));
        QVERIFY(o.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QCOMPARE(o.asked.size(), 2);
        QCOMPARE(o.told.size(), 2);
        QVERIFY(QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("now.xlsx"))));
        QVERIFY(o.originalIsUntouched());
    }

    // Save As on such a workbook is the same question; an .xls name typed is given the ending .xlsx, and the file is not the original.
    void saveAsGivesTheNameTheEndingOfTheNewForm()
    {
        OldWindow o;
        QVERIFY(o.open(oldBook()));
        o.select(0, 0);
        o.enter(QStringLiteral("11"));
        QVERIFY(o.settle());
        o.answer = o.folder.dir.filePath(QStringLiteral("renamed.xls"));
        QSignalSpy saved(&o.win.w, &CrunchWindow::saved);
        o.win.w.act("file.saveAs")->trigger();
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QCOMPARE(saved.first().at(0).toString(), o.folder.dir.filePath(QStringLiteral("renamed.xlsx")));
        QVERIFY(QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("renamed.xlsx"))));
        QVERIFY(!QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("renamed.xls"))));
        QVERIFY(o.originalIsUntouched());
        QCOMPARE(o.asked.size(), 1);
        QCOMPARE(o.told.size(), 1);
        // (and the macro-enabled ending, which this workbook's file cannot have, becomes the plain one)
        OldWindow p;
        QVERIFY(p.open(oldBook()));
        p.answer = p.folder.dir.filePath(QStringLiteral("macro.xlsm"));
        QSignalSpy savedP(&p.win.w, &CrunchWindow::saved);
        p.win.w.act("file.saveAs")->trigger();
        QVERIFY2(savedOk(savedP, 20000), "the save failed");
        QCOMPARE(savedP.first().at(0).toString(), p.folder.dir.filePath(QStringLiteral("macro.xlsx")));
        QVERIFY(p.originalIsUntouched());
    }

    // Closing a window with changes asks the usual question about the .xls's changes; Save asks for the .xlsx name (a person who cancels that keeps the window open), then closes.
    void theCloseQuestionSavesAnOldWorkbookAsXlsx()
    {
        OldWindow o;
        QVERIFY(o.open(oldBook()));
        o.win.w.setAttribute(Qt::WA_DeleteOnClose, false);
        QStringList questions;
        o.win.w.setAsker([&](const QString &, const QString &text) {
            questions << text;
            return CrunchWindow::Answer::Save;
        });
        o.select(0, 0);
        o.enter(QStringLiteral("12"));
        QVERIFY(o.settle());
        // the name is not given: the window stays open, unsaved
        o.answer = QString();
        o.win.w.close();
        QCOMPARE(questions.size(), 1);
        QVERIFY2(questions.first().contains(QLatin1String("old.xls")), qPrintable(questions.first()));
        QCOMPARE(o.asked.size(), 1);
        QVERIFY(o.win.w.isVisible());
        QVERIFY(o.win.w.isModified());
        QVERIFY(o.originalIsUntouched());
        // the name given: saved as .xlsx, then closed
        o.answer = o.folder.dir.filePath(QStringLiteral("closing.xlsx"));
        QSignalSpy saved(&o.win.w, &CrunchWindow::saved);
        o.win.w.close();
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QTRY_VERIFY(!o.win.w.isVisible());
        QCOMPARE(questions.size(), 2);
        QVERIFY(o.originalIsUntouched());
        Window second;
        QVERIFY(second.open(o.copyOfSaved(QStringLiteral("closing.xlsx"), QStringLiteral("again3.xlsx"))));
        QCOMPARE(second.w.view()->sheet().valueOf(*second.w.view()->sheet().cellAt(0, 0)).debug(), QStringLiteral("n:12"));
    }

    // A file of the old form that has an .xlsx name is the file it came from all the same: it is never written, whatever name the person gives it.
    void aFileOfTheOldFormWithAnXlsxNameIsNotWrittenOver()
    {
        OldWindow o;
        QVERIFY(o.open(oldBook(), QStringLiteral("disguised.xlsx")));
        o.select(0, 0);
        o.enter(QStringLiteral("13"));
        QVERIFY(o.settle());
        o.answer = o.path;
        QSignalSpy saved(&o.win.w, &CrunchWindow::saved);
        QVERIFY(!o.win.w.save());
        QCOMPARE(saved.count(), 0);
        QVERIFY(o.originalIsUntouched());
        QCOMPARE(o.told.size(), 2);   // (the explanation, then that the name is the file's own)
        QVERIFY2(o.told.last().contains(QLatin1String("opened from")), qPrintable(o.told.last()));
        // (and asked for outright, with that name, it is refused all the same)
        QVERIFY(!o.win.w.saveAs(o.path));
        QCOMPARE(saved.count(), 0);
        QVERIFY(o.originalIsUntouched());
        QCOMPARE(o.told.size(), 3);
        QVERIFY2(o.told.last().contains(QLatin1String("opened from")), qPrintable(o.told.last()));
        o.answer = o.folder.dir.filePath(QStringLiteral("elsewhere.xlsx"));
        QVERIFY(o.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QVERIFY(o.originalIsUntouched());
        QVERIFY(QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("elsewhere.xlsx"))));
    }

    // A workbook of the XML form is saved where it is with no question, and none of the above is said.
    void aWorkbookOfTheXmlFormIsSavedWhereItIsWithoutAQuestion()
    {
        OldWindow o;
        QVERIFY(o.open(sheettest::buildXlsx({sheettest::worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>5</v></c></row>"))}), QStringLiteral("new.xlsx")));
        o.select(0, 0);
        o.enter(QStringLiteral("6"));
        QVERIFY(o.settle());
        QSignalSpy saved(&o.win.w, &CrunchWindow::saved);
        pressStandard(&o.win.w, QKeySequence::Save);
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QCOMPARE(saved.first().at(0).toString(), o.path);
        QVERIFY(o.asked.isEmpty());
        QVERIFY(o.told.isEmpty());
        // a window with no workbook has nothing to save
        Window empty;
        QCOMPARE(empty.w.pathToSave(), QString());
    }

    // The sheets of a workbook of the older form can be added, renamed, moved, and given a color: the first save writes them into the .xlsx it makes (a package made from the model), and
    // the next save patches that file; the original is never written.
    void sheetChangesOfAnOldWorkbookAreSaved()
    {
        OldWindow o;
        xlstest::Book b;
        b.sheet(QStringLiteral("One"), xlstest::number(0, 0, 2));
        b.sheet(QStringLiteral("Two"), xlstest::number(0, 0, 7));
        QVERIFY(o.open(b.file()));
        auto names = [](Window &w) {
            QStringList out;
            for (int i = 0; i < w.w.sheetTabs()->count(); ++i) out << w.w.sheetTabs()->tabData(i).toString();
            return out;
        };
        QCOMPARE(names(o.win), QStringList({QStringLiteral("One"), QStringLiteral("Two")}));
        o.win.w.act("sheet.insertEnd")->trigger();
        QVERIFY(o.win.w.renameSheetTo(0, QStringLiteral("Plan")));
        QVERIFY(o.win.w.session()->moveSheet(0, -1).ok);
        QVERIFY(o.win.w.session()->setTabColor(1, office::sheet::ColorSpec::fromRgb(0x00B050)).ok);
        QCOMPARE(names(o.win), QStringList({QStringLiteral("Two"), QStringLiteral("Sheet3"), QStringLiteral("Plan")}));
        QVERIFY(o.settle());
        o.answer = o.folder.dir.filePath(QStringLiteral("copy"));
        QSignalSpy saved(&o.win.w, &CrunchWindow::saved);
        pressStandard(&o.win.w, QKeySequence::Save);
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QCOMPARE(saved.first().at(1).toBool(), true);
        QVERIFY(o.originalIsUntouched());
        {
            Window second;
            QVERIFY(second.open(o.copyOfSaved(QStringLiteral("copy.xlsx"), QStringLiteral("sheets1.xlsx"))));
            QCOMPARE(names(second), QStringList({QStringLiteral("Two"), QStringLiteral("Sheet3"), QStringLiteral("Plan")}));
            const auto &info = second.w.loaded()->book->info();
            QCOMPARE(info.sheets.size(), 3);
            // (the sheets are in the file in the order the tabs show)
            QCOMPARE(info.sheets[0].name, QStringLiteral("Two"));
            QCOMPARE(info.sheets[2].name, QStringLiteral("Plan"));
            second.w.setActiveSheet(2);
            QCOMPARE(second.w.view()->sheet().valueOf(*second.w.view()->sheet().cellAt(0, 0)).debug(), QStringLiteral("n:2"));
            second.w.setActiveSheet(0);
            QCOMPARE(second.w.view()->sheet().valueOf(*second.w.view()->sheet().cellAt(0, 0)).debug(), QStringLiteral("n:7"));
            QVERIFY(second.w.calculation()->sheet(0).tabColor.isSet());
        }
        // the next save patches the .xlsx: a sheet renamed again, and one added
        QVERIFY(o.activate());
        QVERIFY(o.win.w.renameSheetTo(2, QStringLiteral("Totals")));
        QVERIFY(o.win.w.session()->insertSheet(-1, QStringLiteral("Notes")).ok);
        pressStandard(&o.win.w, QKeySequence::Save);
        QTRY_COMPARE_WITH_TIMEOUT(saved.count(), 2, 20000);
        QCOMPARE(saved.last().at(1).toBool(), true);
        QCOMPARE(o.asked.size(), 1);
        {
            Window third;
            QVERIFY(third.open(o.copyOfSaved(QStringLiteral("copy.xlsx"), QStringLiteral("sheets2.xlsx"))));
            QCOMPARE(names(third), QStringList({QStringLiteral("Two"), QStringLiteral("Totals"), QStringLiteral("Plan"), QStringLiteral("Notes")}));
        }
        QVERIFY(o.originalIsUntouched());
    }

    // The names: what ends as the older form does, the name offered (never the original, nor a file that is there), and the ending put on what the person typed.
    void theNamesAreChosenSoThatTheOriginalIsNeverTheAnswer()
    {
        QVERIFY(crunch::isOlderFormatName(QStringLiteral("/a/b/Budget.XLS")));
        QVERIFY(crunch::isOlderFormatName(QStringLiteral("budget.xls")));
        QVERIFY(!crunch::isOlderFormatName(QStringLiteral("budget.xlsx")));
        QVERIFY(!crunch::isOlderFormatName(QStringLiteral("budget.xlsm")));
        QVERIFY(!crunch::isOlderFormatName(QStringLiteral("xls")));
        QTemporaryDir dir;
        const QString old = dir.filePath(QStringLiteral("Plan.xls"));
        QCOMPARE(crunch::suggestedSaveName(old), dir.filePath(QStringLiteral("Plan.xlsx")));
        for (const QString &name : {QStringLiteral("Plan.xlsx"), QStringLiteral("Plan (2).xlsx")}) {
            QFile f(dir.filePath(name));
            QVERIFY(f.open(QIODevice::WriteOnly));
        }
        QCOMPARE(crunch::suggestedSaveName(old), dir.filePath(QStringLiteral("Plan (3).xlsx")));
        QCOMPARE(crunch::withWorkbookEnding(QStringLiteral("/x/a")), QStringLiteral("/x/a.xlsx"));
        QCOMPARE(crunch::withWorkbookEnding(QStringLiteral("/x/a.xls")), QStringLiteral("/x/a.xlsx"));
        QCOMPARE(crunch::withWorkbookEnding(QStringLiteral("/x/a.XLSX")), QStringLiteral("/x/a.XLSX"));
        QCOMPARE(crunch::withWorkbookEnding(QStringLiteral("/x/a.xlsm")), QStringLiteral("/x/a.xlsx"));   // (what is saved from an old workbook has no macros)
        QCOMPARE(crunch::withWorkbookEnding(QStringLiteral("/x/a.v2")), QStringLiteral("/x/a.v2.xlsx"));
        QCOMPARE(crunch::withWorkbookEnding(QString()), QString());
    }

    // What the person is told says it plainly: the old format, that .xlsx is what is saved, and that the file is not changed; and it names no other program.
    void theExplanationIsInPlainWords()
    {
        const QString text = crunch::saveAsExplanation(QStringLiteral("Plan.xls"));
        QVERIFY(text.contains(QLatin1String("Plan.xls")));
        QVERIFY(text.contains(QLatin1String(".xlsx")));
        QVERIFY(text.contains(QLatin1String("not be changed")));
        QVERIFY(!text.contains(QLatin1String("macros")));
        const QString withMacros = crunch::saveAsExplanation(QStringLiteral("Plan.xls"), {QStringLiteral("macros"), QStringLiteral("charts")});
        QVERIFY2(withMacros.contains(QLatin1String("macros, charts")) && withMacros.contains(QLatin1String("will not be in")), qPrintable(withMacros));
        for (const QString &word : {QStringLiteral("Excel"), QStringLiteral("Microsoft"), QStringLiteral("Office")}) QVERIFY(!text.contains(word));  // (no other program is named)
        QVERIFY(crunch::notReadNotice({}).isEmpty());
        const QString notice = crunch::notReadNotice({QStringLiteral("charts"), QStringLiteral("comments")});
        QVERIFY(notice.contains(QLatin1String("charts, comments")));
        QVERIFY(!notice.contains(QLatin1String("Excel")));
    }
};

JP_TEST_CLASS(CrunchXlsTests)
#include "tst_crunch_xls.moc"
