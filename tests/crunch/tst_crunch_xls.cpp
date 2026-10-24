// Tests of JeffCrunch's window with a workbook of the older binary format (.xls): it opens and shows, the formulas are worked out, what is not read is said, and saving asks for a
// new name ending in .xlsx and never offers the file the workbook came from. Every workbook is built here, in a temporary folder.
#include "../sheet/xlshelpers.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/oldformat.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "grid/gridview.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>

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

    // Saving a workbook that came from an .xls file asks for a name with the ending .xlsx and never answers with the file it came from; one that came from the XML form is saved
    // to its own path without a question.
    void savingAnOldWorkbookAsksForANewName()
    {
        Folder folder;
        Window win;
        xlstest::Book b;
        b.sheet(QStringLiteral("S"), xlstest::number(0, 0, 1));
        const QString path = folder.write(QStringLiteral("old.xls"), b.file());
        QVERIFY(win.open(path));
        QStringList asked;
        QString answer;
        win.w.setSaveNameAsker([&](const QString &opened) {
            asked << opened;
            return answer;
        });
        answer = folder.dir.filePath(QStringLiteral("copy"));
        QCOMPARE(win.w.pathToSave(), folder.dir.filePath(QStringLiteral("copy.xlsx")));
        QCOMPARE(asked, QStringList{path});
        answer = folder.dir.filePath(QStringLiteral("copy.xls"));
        QCOMPARE(win.w.pathToSave(), folder.dir.filePath(QStringLiteral("copy.xlsx")));
        answer = path;   // (the file it came from: it is given the ending of the new form, and so is another file)
        QCOMPARE(win.w.pathToSave(), folder.dir.filePath(QStringLiteral("old.xlsx")));
        QVERIFY(win.w.pathToSave() != path);
        answer = QString();   // (cancelled)
        QCOMPARE(win.w.pathToSave(), QString());
        // the file is untouched
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(f.readAll(), b.file());
        // a workbook of the XML form is saved where it is, with no question
        Window other;
        other.w.setSaveNameAsker([&](const QString &) {
            asked << QStringLiteral("again");
            return QString();
        });
        const int before = int(asked.size());
        QVERIFY(other.open(folder.write(QStringLiteral("new.xlsx"), sheettest::buildXlsx({sheettest::worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>5</v></c></row>"))}))));
        QCOMPARE(other.w.pathToSave(), folder.dir.filePath(QStringLiteral("new.xlsx")));
        QCOMPARE(int(asked.size()), before);
        // a window with no workbook has nothing to save
        Window empty;
        QCOMPARE(empty.w.pathToSave(), QString());
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
        QCOMPARE(crunch::withWorkbookEnding(QStringLiteral("/x/a.xlsm")), QStringLiteral("/x/a.xlsm"));
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
        for (const QString &word : {QStringLiteral("Excel"), QStringLiteral("Microsoft"), QStringLiteral("Office")}) QVERIFY(!text.contains(word));  // (no other program is named)
        QVERIFY(crunch::notReadNotice({}).isEmpty());
        const QString notice = crunch::notReadNotice({QStringLiteral("charts"), QStringLiteral("comments")});
        QVERIFY(notice.contains(QLatin1String("charts, comments")));
        QVERIFY(!notice.contains(QLatin1String("Excel")));
    }
};

JP_TEST_CLASS(CrunchXlsTests)
#include "tst_crunch_xls.moc"
