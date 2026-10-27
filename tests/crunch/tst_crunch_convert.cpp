// Tests of JeffCrunch's window with workbooks of the other kinds of files: an OpenDocument spreadsheet (.ods) and a text file of values (.csv) open and show; saving either asks for
// a new name ending in .xlsx (or .csv, for the values of the sheet) and never offers the file the workbook came from; and Save As of any workbook can write the values of the sheet in
// view as a .csv. Every file is built here, in a temporary folder.
#include "../sheet/odshelpers.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "savedok.h"
#include "app/oldformat.h"
#include "app/otherformats.h"
#include "app/settings.h"
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

// The default locale for the length of a test.
struct LocaleScope {
    QLocale old;
    explicit LocaleScope(const QLocale &l) : old() { QLocale::setDefault(l); }
    ~LocaleScope() { QLocale::setDefault(old); }
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
    QByteArray read(const QString &name) const
    {
        QFile f(dir.filePath(name));
        return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
    }
};

// A window with a workbook open from a folder of its own, a file dialog that answers what the test says and remembers what it was asked, and what was said to the person.
struct Win {
    Folder folder;
    CrunchWindow w;
    QStringList told, asked, filters;
    QString path, answer;
    QByteArray original;
    Win()
    {
        w.setNotifier([this](const QString &title, const QString &text) { told << title + QStringLiteral(": ") + text; });
        w.setSavePicker([this](const QString &start, const QString &filter) {
            asked << start;
            filters << filter;
            return answer;
        });
        w.setChangedAsker([this](const QString &name) {
            told << QStringLiteral("changed: ") + name;
            return CrunchWindow::ChangedAnswer::Cancel;
        });
        w.resize(1100, 640);
        w.show();
    }
    bool open(const QByteArray &bytes, const QString &name)
    {
        original = bytes;
        path = folder.write(name, bytes);
        QSignalSpy first(&w, &CrunchWindow::firstScreenShown), done(&w, &CrunchWindow::calculationFinished), loaded(&w, &CrunchWindow::loadFinished);
        if (!w.openFile(path)) return false;
        if (!(first.wait(20000) && (loaded.count() > 0 || loaded.wait(20000)) && (done.count() > 0 || done.wait(20000)))) return false;
        w.activateWindow();
        return QTest::qWaitForWindowActive(&w);
    }
    grid::GridView *view() { return w.view(); }
    void select(quint32 row, quint32 col) { view()->selectCell(row, col); }
    void enter(const QString &text)
    {
        QTest::keyClick(view(), text.at(0).toLatin1());
        if (text.size() > 1) QTest::keyClicks(w.cellEditor(), text.mid(1));
        QTest::keyClick(w.cellEditor(), Qt::Key_Return);
    }
    bool settle() { return QTest::qWaitFor([&] { return !w.isCalculating(); }, 20000); }
    bool originalIsUntouched() const
    {
        QFile f(path);
        return f.open(QIODevice::ReadOnly) && f.readAll() == original;
    }
    QString shown(quint32 row, quint32 col) { return view()->sheet().textOf(row, col, *view()->sheet().cellAt(row, col)); }
};

// An .ods: A1 = 2, A2 = 3, A3 = SUM(A1:A2) (its stored result wrong on purpose), B1 the text "hello".
QByteArray odsBook(const QString &extraInCell = QString())
{
    const QString r1 = row(floatCell(2) + stringCell(QStringLiteral("hello")) + cell(QString(), extraInCell));
    const QString r2 = row(floatCell(3));
    const QString r3 = row(cell(QStringLiteral("table:formula=\"of:=SUM([.A1:.A2])\" office:value-type=\"float\" office:value=\"99\""), QStringLiteral("<text:p>99</text:p>")));
    return buildOds(contentXml(table(QStringLiteral("S"), columns(3) + r1 + r2 + r3) + table(QStringLiteral("Other"), row(floatCell(7)))));
}

} // namespace

class CrunchConvertTests : public QObject {
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

    // An .ods opens as an .xlsx does: its sheets as tabs, its cells shown, its formulas worked out from the constants (the stored result here is wrong on purpose), and nothing said
    // when there is nothing it does not read.
    void anOdsWorkbookOpensAndItsFormulasAreWorkedOut()
    {
        Win o;
        QVERIFY(o.open(odsBook(), QStringLiteral("book.ods")));
        QVERIFY(o.w.hasWorkbook());
        QCOMPARE(o.w.shownSheets().size(), 2);
        o.select(2, 0);
        QCOMPARE(o.view()->activeCellText(), QStringLiteral("=SUM(A1:A2)"));
        QCOMPARE(o.shown(2, 0), QStringLiteral("5"));
        QCOMPARE(o.shown(0, 1), QStringLiteral("hello"));
        QVERIFY2(o.told.isEmpty(), qPrintable(o.told.join(QLatin1Char('\n'))));
    }

    // What the document has that is not read is said once, in plain words, as it opens.
    void whatIsNotReadIsSaid()
    {
        Win o;
        QVERIFY(o.open(odsBook(QStringLiteral("<draw:frame><draw:image xlink:href=\"Pictures/a.png\"/></draw:frame><office:annotation><text:p>note</text:p></office:annotation>")), QStringLiteral("pictures.ods")));
        QCOMPARE(o.told.size(), 1);
        QVERIFY2(o.told.first().contains(QLatin1String("pictures and other drawing objects")) && o.told.first().contains(QLatin1String("comments")), qPrintable(o.told.first()));
    }

    // A text file of values opens: numbers in the person's locale, the percent shown as typed, the first screen on at once.
    void aCsvFileOpensAndShowsItsValues()
    {
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        Win o;
        QVERIFY(o.open("name,share,total\nann,12%,\"1,234.50\"\nbob,5%,=B2+B3\n", QStringLiteral("people.csv")));
        QCOMPARE(o.w.shownSheets().size(), 1);
        QCOMPARE(o.shown(0, 0), QStringLiteral("name"));
        QCOMPARE(o.shown(1, 1), QStringLiteral("12%"));
        QCOMPARE(o.shown(1, 2), QStringLiteral("1,234.50"));
        o.select(2, 2);
        QCOMPARE(o.view()->activeCellText(), QStringLiteral("=B2+B3"));
        QCOMPARE(o.shown(2, 2), QStringLiteral("0.17"));
        QVERIFY(o.told.isEmpty());
    }

    // Save on a workbook that came from an .ods: the person is told in plain words that it is saved as .xlsx and asked for a name (offered with that ending, never the file it came from);
    // the .xlsx is written with the edit, the .ods is as it was, and the next Save goes to the .xlsx with no question.
    void saveAsksForAnXlsxNameAndNeverWritesTheOds()
    {
        Win o;
        QVERIFY(o.open(odsBook(), QStringLiteral("book.ods")));
        o.select(0, 0);
        o.enter(QStringLiteral("10"));
        QVERIFY(o.settle());
        o.answer = o.folder.dir.filePath(QStringLiteral("copy"));
        QSignalSpy saved(&o.w, &CrunchWindow::saved);
        QVERIFY(o.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        QCOMPARE(o.asked, QStringList{o.folder.dir.filePath(QStringLiteral("book.xlsx"))});
        QVERIFY2(o.filters.first().contains(QLatin1String("*.xlsx")) && o.filters.first().contains(QLatin1String("*.csv")), qPrintable(o.filters.first()));
        QCOMPARE(saved.first().at(0).toString(), o.folder.dir.filePath(QStringLiteral("copy.xlsx")));
        QCOMPARE(saved.first().at(1).toBool(), true);
        QCOMPARE(o.told.size(), 1);
        QVERIFY2(o.told.first().contains(QLatin1String("book.ods")) && o.told.first().contains(QLatin1String("OpenDocument")) && o.told.first().contains(QLatin1String(".xlsx"))
                     && o.told.first().contains(QLatin1String("not be changed")),
                 qPrintable(o.told.first()));
        QVERIFY(o.originalIsUntouched());
        QVERIFY(!o.w.isModified());
        // the .xlsx has the edit, the cells that were not touched, and the formula with its result
        {
            Win second;
            QVERIFY(second.open(o.folder.read(QStringLiteral("copy.xlsx")), QStringLiteral("again.xlsx")));
            QCOMPARE(second.w.shownSheets().size(), 2);
            QCOMPARE(second.shown(0, 0), QStringLiteral("10"));
            QCOMPARE(second.shown(1, 0), QStringLiteral("3"));
            QCOMPARE(second.shown(2, 0), QStringLiteral("13"));
            QCOMPARE(second.shown(0, 1), QStringLiteral("hello"));
        }
        // the next edit is saved over the .xlsx, with no question
        o.w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&o.w));
        o.select(1, 0);
        o.enter(QStringLiteral("20"));
        QVERIFY(o.settle());
        QVERIFY(o.w.save());
        QTRY_COMPARE_WITH_TIMEOUT(saved.count(), 2, 20000);
        QCOMPARE(saved.last().at(0).toString(), o.folder.dir.filePath(QStringLiteral("copy.xlsx")));
        QCOMPARE(o.asked.size(), 1);
        QCOMPARE(o.told.size(), 1);
        QVERIFY(o.originalIsUntouched());
    }

    // Save on a workbook that came from a .csv: the same, saying what a text file of values is; a name ending in .csv gives the values of the sheet (and writes nowhere else); the
    // name of the file it came from is refused.
    void saveOfACsvWorkbookAsksAndNeverWritesTheOriginal()
    {
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        Win o;
        QVERIFY(o.open("a,b\n1,2\n", QStringLiteral("values.csv")));
        o.select(0, 0);
        o.enter(QStringLiteral("5"));
        QVERIFY(o.settle());
        o.answer = o.path;
        QSignalSpy saved(&o.w, &CrunchWindow::saved), exported(&o.w, &CrunchWindow::csvExported);
        QVERIFY(!o.w.save());
        QCOMPARE(o.told.size(), 2);
        QVERIFY2(o.told.first().contains(QLatin1String("text file of values")) && o.told.first().contains(QLatin1String("not be changed")), qPrintable(o.told.first()));
        QVERIFY2(o.told.last().contains(QLatin1String("opened from")), qPrintable(o.told.last()));
        QVERIFY(o.originalIsUntouched());
        QCOMPARE(o.asked, QStringList{o.folder.dir.filePath(QStringLiteral("values.xlsx"))});
        // (and asked for outright, the same)
        QVERIFY(!o.w.saveAs(o.path));
        QVERIFY(!o.w.exportCsv(o.path));
        QVERIFY(o.originalIsUntouched());
        QCOMPARE(saved.count() + exported.count(), 0);
        // a copy as a .csv: the values of the sheet
        o.answer = o.folder.dir.filePath(QStringLiteral("copy.csv"));
        QVERIFY(o.w.save());
        QVERIFY(exported.wait(20000));
        QCOMPARE(exported.first().at(1).toBool(), true);
        QCOMPARE(o.folder.read(QStringLiteral("copy.csv")), QByteArray("\xEF\xBB\xBF" "5,b\r\n" "1,2\r\n"));
        QVERIFY(o.originalIsUntouched());
        QVERIFY(o.w.isModified());   // (the workbook itself is not saved by it)
    }

    // Save As of a workbook of the XML form offers .csv besides .xlsx; a name ending in .csv writes the values of the sheet in view as the cells show them, with the separator of the
    // person's language, text in quotes when it needs them, every row (hidden ones too), and none of the other sheets; the person is told what is written; the workbook is left as it was.
    void saveAsCsvWritesTheValuesOfTheSheetInView()
    {
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        const QString rows = QStringLiteral(
            "<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>name</t></is></c><c r=\"B1\" t=\"inlineStr\"><is><t>say \"hi\", ok</t></is></c><c r=\"C1\" s=\"1\"><v>0.25</v></c></row>"
            "<row r=\"2\" hidden=\"1\"><c r=\"A2\"><v>1234.5</v></c><c r=\"B2\" t=\"inlineStr\"><is><t>two&#10;lines</t></is></c><c r=\"C2\"><f>A2*2</f><v>1</v></c></row>"
            "<row r=\"4\"><c r=\"B4\" t=\"inlineStr\"><is><t>é</t></is></c></row>");
        const QByteArray book = buildXlsx({worksheet(rows), worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>99</v></c></row>"))}, QByteArray(), QString(), QString(),
                                          stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"9\"/>")));
        Win o;
        QVERIFY(o.open(book, QStringLiteral("book.xlsx")));
        QVERIFY(!o.w.isModified());
        o.answer = o.folder.dir.filePath(QStringLiteral("out.csv"));
        QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
        o.w.activateWindow();
        QVERIFY(QTest::qWaitForWindowActive(&o.w));
        o.w.act(QStringLiteral("file.saveAs"))->trigger();
        QVERIFY(exported.wait(20000));
        QCOMPARE(exported.first().at(0).toString(), o.answer);
        QCOMPARE(exported.first().at(1).toBool(), true);
        QVERIFY2(o.filters.first().contains(QLatin1String("*.xlsx")) && o.filters.first().contains(QLatin1String("*.csv")), qPrintable(o.filters.first()));
        QCOMPARE(o.told.size(), 1);
        QVERIFY2(o.told.first().contains(QLatin1String("Only the values")) && o.told.first().contains(QLatin1String("Sheet1")), qPrintable(o.told.first()));
        const QByteArray expected = QByteArray("\xEF\xBB\xBF") + "name,\"say \"\"hi\"\", ok\",25%\r\n" + "1234.5,\"two\nlines\",2469\r\n" + ",,\r\n" + ",\xC3\xA9,\r\n";
        QCOMPARE(o.folder.read(QStringLiteral("out.csv")), expected);
        QVERIFY(!o.w.isModified());
        QVERIFY(o.originalIsUntouched());
        // the window takes edits again
        o.select(0, 0);
        o.enter(QStringLiteral("7"));
        QVERIFY(o.w.isModified());
    }

    // The separator is the semicolon where the language writes a comma in its numbers; the sheet in view is the one that is written.
    void theSeparatorIsTheLanguagesAndTheSheetIsTheOneInView()
    {
        {
            LocaleScope german(QLocale(QLocale::German, QLocale::Germany));
            QCOMPARE(crunch::csvSeparatorFor(QLocale()), QLatin1Char(';'));
        }
        LocaleScope english(QLocale(QLocale::English, QLocale::UnitedStates));
        QCOMPARE(crunch::csvSeparatorFor(QLocale()), QLatin1Char(','));
        const QByteArray book = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>")), worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>2</v></c><c r=\"B1\"><v>3</v></c></row>"))});
        Win o;
        QVERIFY(o.open(book, QStringLiteral("two.xlsx")));
        o.w.setActiveSheet(1);
        QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
        QVERIFY(o.w.saveAs(o.folder.dir.filePath(QStringLiteral("second.csv"))));
        QVERIFY(exported.wait(20000));
        QCOMPARE(o.folder.read(QStringLiteral("second.csv")), QByteArray("\xEF\xBB\xBF" "2,3\r\n"));
    }

    // A file that is there is kept if the write fails, and a folder that cannot be written to is told in words.
    void aFailedWriteIsToldAndLeavesWhatWasThere()
    {
        Win o;
        QVERIFY(o.open(odsBook(), QStringLiteral("book.ods")));
        QVERIFY(!o.w.exportCsv(o.folder.dir.filePath(QStringLiteral("no/such/folder/x.csv"))));
        QVERIFY2(o.told.last().contains(QLatin1String("couldn't save")), qPrintable(o.told.last()));
        QVERIFY(!o.w.isSaving());
        // a file of that name that is there already is as it was when the folder cannot be written to
        QVERIFY(QDir(o.folder.dir.path()).mkdir(QStringLiteral("locked")));
        const QString locked = o.folder.dir.filePath(QStringLiteral("locked"));
        QVERIFY(!o.folder.write(QStringLiteral("locked/there.csv"), "kept\n").isEmpty());
        const QFile::Permissions before = QFile::permissions(locked);
        QVERIFY(QFile::setPermissions(locked, QFile::ReadOwner | QFile::ExeOwner));
        const bool refused = !QFileInfo(locked).isWritable();
        struct Restore {
            QString path;
            QFile::Permissions was;
            ~Restore() { QFile::setPermissions(path, was); }
        } restore{locked, before};
        if (!refused) QSKIP("this account can write to a folder that is read only");
        o.told.clear();
        QVERIFY(!o.w.exportCsv(o.folder.dir.filePath(QStringLiteral("locked/there.csv"))));
        QVERIFY2(o.told.last().contains(QLatin1String("couldn't save")) && o.told.last().contains(QLatin1String("as it was")), qPrintable(o.told.last()));
        QVERIFY(!o.w.isSaving());
        QCOMPARE(o.folder.read(QStringLiteral("locked/there.csv")), QByteArray("kept\n"));
    }

    // The file a workbook was read from is never written, whatever it is called and however long the window has had it: a link to it is it, and after the first save (the window's file is then
    // the .xlsx) it is still not written, by Save As to its name, to a link to it, or as a .csv.
    void theOriginalIsGuardedByWhatItIsAndForTheWholeSession()
    {
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        {
            Win o;
            QVERIFY(o.open(odsBook(), QStringLiteral("book.ods")));
            const QString link = o.folder.dir.filePath(QStringLiteral("link.xlsx"));
            if (!QFile::link(o.path, link) || !QFileInfo(link).isSymLink()) QSKIP("this system makes no symbolic links");
            o.select(0, 0);
            o.enter(QStringLiteral("10"));
            QVERIFY(o.settle());
            // a link to the original, offered as the copy's name: refused, and the original is as it was
            o.answer = link;
            QVERIFY(!o.w.save());
            QVERIFY2(o.told.last().contains(QLatin1String("opened from")), qPrintable(o.told.last()));
            QVERIFY(o.originalIsUntouched());
            QVERIFY(!o.w.saveAs(link));
            QVERIFY(o.originalIsUntouched());
            // a first save under a name of its own; then the original is still not written, by its own name or by the link
            o.answer = o.folder.dir.filePath(QStringLiteral("copy.xlsx"));
            QSignalSpy saved(&o.w, &CrunchWindow::saved);
            QVERIFY(o.w.save());
            QVERIFY2(savedOk(saved, 20000), "the save failed");
            QVERIFY(o.originalIsUntouched());
            o.told.clear();
            QVERIFY(!o.w.saveAs(link));
            QVERIFY2(o.told.last().contains(QLatin1String("opened from")), qPrintable(o.told.last()));
            QVERIFY(!o.w.saveAs(o.path));
            QVERIFY(!o.w.exportCsv(o.path));
            QVERIFY(o.originalIsUntouched());
            QCOMPARE(saved.count(), 1);
        }
        {
            Win o;
            QVERIFY(o.open("a,b\n1,2\n", QStringLiteral("values.csv")));
            const QString link = o.folder.dir.filePath(QStringLiteral("again.csv"));
            if (!QFile::link(o.path, link) || !QFileInfo(link).isSymLink()) QSKIP("this system makes no symbolic links");
            QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
            QVERIFY(!o.w.exportCsv(link));
            QVERIFY(!o.w.saveAs(link));
            QVERIFY(o.originalIsUntouched());
            QCOMPARE(exported.count(), 0);
            QVERIFY2(o.told.last().contains(QLatin1String("opened from")), qPrintable(o.told.last()));
        }
        // the same file under two spellings
        QTemporaryDir dir;
        QVERIFY(QFile(dir.filePath(QStringLiteral("x.ods"))).open(QIODevice::WriteOnly));
        QVERIFY(crunch::sameFile(dir.filePath(QStringLiteral("x.ods")), dir.path() + QStringLiteral("/./x.ods")));
        QVERIFY(crunch::sameFile(dir.filePath(QStringLiteral("x.ods")), dir.path() + QStringLiteral("/sub/../x.ods")));
        QVERIFY(!crunch::sameFile(dir.filePath(QStringLiteral("x.ods")), dir.filePath(QStringLiteral("y.ods"))));
        QVERIFY(!crunch::sameFile(QString(), QString()));
        QVERIFY(!crunch::sameFile(dir.filePath(QStringLiteral("x.ods")), QString()));
    }

    // Exporting a .csv from the question asked when the window is closed with changes is the person's choice: the window closes when the file is written, and does not ask again. An
    // export that fails leaves the window open, and does not leave the close waiting for the next export.
    void exportingFromTheClosePromptClosesAndAFailedExportDoesNot()
    {
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        {
            Win o;
            QVERIFY(o.open("a,b\n1,2\n", QStringLiteral("values.csv")));
            o.select(0, 0);
            o.enter(QStringLiteral("5"));
            QVERIFY(o.settle());
            int asked = 0;
            o.w.setAsker([&](const QString &, const QString &) { return ++asked == 1 ? CrunchWindow::Answer::Save : CrunchWindow::Answer::Cancel; });
            o.answer = o.folder.dir.filePath(QStringLiteral("values.csv.out.csv"));
            QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
            o.w.close();
            QVERIFY(exported.wait(20000));
            QCOMPARE(exported.first().at(1).toBool(), true);
            QTRY_VERIFY(!o.w.isVisible());
            QCOMPARE(asked, 1);
        }
        {
            Win o;
            QVERIFY(o.open("a,b\n1,2\n", QStringLiteral("values.csv")));
            o.select(0, 0);
            o.enter(QStringLiteral("5"));
            QVERIFY(o.settle());
            int asked = 0;
            o.w.setAsker([&](const QString &, const QString &) { return ++asked == 1 ? CrunchWindow::Answer::Save : CrunchWindow::Answer::Cancel; });
            QVERIFY(QDir(o.folder.dir.path()).mkdir(QStringLiteral("out")));
            o.answer = o.folder.dir.filePath(QStringLiteral("out/gone.csv"));
            QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
            o.w.close();
            QVERIFY(o.w.isSaving());
            // (the folder goes while the file is written)
            if (!QDir(o.folder.dir.path()).rename(QStringLiteral("out"), QStringLiteral("away"))) QSKIP("this system does not move a folder that has a file open in it");
            QVERIFY(exported.wait(20000));
            QCOMPARE(exported.first().at(1).toBool(), false);
            QVERIFY(o.w.isVisible());
            QVERIFY(o.w.isModified());
            // a later export that works is only an export: the window stays
            QSignalSpy again(&o.w, &CrunchWindow::csvExported);
            QVERIFY(o.w.saveAs(o.folder.dir.filePath(QStringLiteral("good.csv"))));
            QVERIFY(again.wait(20000));
            QCOMPARE(again.first().at(1).toBool(), true);
            QTest::qWait(100);
            QVERIFY(o.w.isVisible());
            QCOMPARE(asked, 1);
        }
    }

    // An export from a sheet that was read only in part says so in the note; a name chosen with the .csv kind of file and no ending is a .csv.
    void thePartOfASheetAndThePickedKindAreSaid()
    {
        const QString whole = crunch::csvExportNote(QStringLiteral("Data"), QLatin1Char(','), false);
        const QString part = crunch::csvExportNote(QStringLiteral("Data"), QLatin1Char(','), true);
        QVERIFY(!whole.contains(QLatin1String("only in part")));
        QVERIFY2(part.contains(QLatin1String("only in part")) && part.startsWith(whole.left(20)), qPrintable(part));
        QCOMPARE(crunch::withPickedEnding(QStringLiteral("/a/b/values"), crunch::csvFilter()), QStringLiteral("/a/b/values.csv"));
        QCOMPARE(crunch::withPickedEnding(QStringLiteral("/a/b/values.txt"), crunch::csvFilter()), QStringLiteral("/a/b/values.txt"));
        QCOMPARE(crunch::withPickedEnding(QStringLiteral("/a/b/values"), QStringLiteral("Workbooks (*.xlsx)")), QStringLiteral("/a/b/values"));
        QCOMPARE(crunch::withPickedEnding(QString(), crunch::csvFilter()), QString());
        // in the window: the kind picked and nothing typed after the name
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        Win o;
        QVERIFY(o.open(odsBook(), QStringLiteral("book.ods")));
        o.answer = o.folder.dir.filePath(QStringLiteral("typed"));
        o.w.setPickedFilter(crunch::csvFilter());
        QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
        QVERIFY(o.w.save());
        QVERIFY(exported.wait(20000));
        QCOMPARE(exported.first().at(0).toString(), o.folder.dir.filePath(QStringLiteral("typed.csv")));
        QVERIFY(QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("typed.csv"))));
        QVERIFY(!QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("typed.xlsx"))));
    }

    // What was changed to be kept (sheet names, formulas that could not be converted) is told with what is not read, when the file opens and when the copy is to be named.
    void whatWasChangedIsToldWithWhatIsNotRead()
    {
        const QString text = crunch::saveAsExplanation(QStringLiteral("Plan.ods"), QStringLiteral(".ods"), {QStringLiteral("charts")}, {QStringLiteral("2 sheet names, made valid")});
        QVERIFY2(text.contains(QLatin1String("charts")) && text.contains(QLatin1String("2 sheet names, made valid")), qPrintable(text));
        const QString opened = crunch::notReadNotice({}, {QStringLiteral("3 formulas kept")});
        QVERIFY2(opened.contains(QLatin1String("3 formulas kept")) && !opened.contains(QLatin1String("doesn't read")), qPrintable(opened));
        QVERIFY(crunch::notReadNotice({}, {}).isEmpty());
        // a document with sheet names the .xlsx form does not allow says so as it opens
        const QString longName = QString(40, QLatin1Char('n'));
        Win o;
        QVERIFY(o.open(buildOds(contentXml(table(longName, row(floatCell(1))) + table(QStringLiteral("a:b"), row(floatCell(2))))), QStringLiteral("names.ods")));
        QCOMPARE(o.told.size(), 1);
        QVERIFY2(o.told.first().contains(QLatin1String("2 sheet name")), qPrintable(o.told.first()));
    }

    // The kinds the Open dialog lists, and the names.
    void theOpenDialogListsTheKindsThatOpen()
    {
        const QString filter = crunch::openFilter();
        for (const char *ending : {"*.xlsx", "*.xlsm", "*.xls", "*.ods", "*.fods", "*.csv", "*.tsv", "*.txt"}) QVERIFY2(filter.contains(QLatin1String(ending)), ending);
        QVERIFY(crunch::isCsvName(QStringLiteral("/a/Data.CSV")));
        QVERIFY(!crunch::isCsvName(QStringLiteral("/a/data.csv.xlsx")));
        QVERIFY(!crunch::isCsvName(QStringLiteral("csv")));
        for (const QString &kind : {QStringLiteral(".ods"), QStringLiteral(".fods"), QStringLiteral(".csv"), QStringLiteral(".tsv")}) {
            const QString text = crunch::saveAsExplanation(QStringLiteral("Plan") + kind, kind, {QStringLiteral("charts")});
            QVERIFY(text.contains(QLatin1String("not be changed")));
            QVERIFY(text.contains(QLatin1String(".xlsx")));
            QVERIFY(text.contains(QLatin1String("charts")));
        }
        QVERIFY(crunch::csvExportNote(QStringLiteral("Data"), QLatin1Char(';')).contains(QLatin1String("\";\"")));
    }
};

JP_TEST_CLASS(CrunchConvertTests)
#include "tst_crunch_convert.moc"
