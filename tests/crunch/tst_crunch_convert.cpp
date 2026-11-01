// Tests of JeffCrunch's window with workbooks of the other kinds of files: an OpenDocument spreadsheet (.ods) and a text file of values (.csv) open and show; saving either asks for
// a new name ending in .xlsx (or .csv, for the values of the sheet) and never offers the file the workbook came from; and Save As of any workbook can write the values of the sheet in
// view as a .csv. Every file is built here, in a temporary folder.
#include "../sheet/odshelpers.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/csvsavedialog.h"
#include "savedok.h"
#include "app/oldformat.h"
#include "app/otherformats.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "grid/gridview.h"

#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include "io/zip.h"
#include "io/zipindex.h"
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
        // (the question before a save that leaves something out is told like the notices are, and answered as the test says)
        w.setLossAsker([this](const QString &title, const QString &text) {
            told << title + QStringLiteral(": ") + text;
            if (lossAnswer == CrunchWindow::LossAnswer::SaveXlsx && (!answerAfterLoss.isEmpty() || cancelAfterLoss)) answer = answerAfterLoss;
            return lossAnswer;
        });
        w.resize(1100, 640);
        w.show();
    }
    CrunchWindow::LossAnswer lossAnswer = CrunchWindow::LossAnswer::Continue;
    bool cancelAfterLoss = false;   // the file dialog that follows gives up (answers no name)
    QString answerAfterLoss;   // the name the file dialog answers once the person chose to save as .xlsx instead
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
        QCOMPARE(o.folder.read(QStringLiteral("copy.csv")), QByteArray("5,b\n" "1,2\n"));   // (in the form the file came in: no mark, line feeds)
        QVERIFY(o.originalIsUntouched());
        QVERIFY(o.w.isModified());   // (the workbook itself is not saved by it)
    }

    // A file of values opened and saved again as a copy with nothing changed comes out as the same bytes: its separator, quotes, line ends, encoding, and the text of every cell.
    void aCsvSavedWithoutEditsIsTheSameFile()
    {
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        const QByteArray utf16 = QByteArray("\xFF\xFE") + QByteArray("a\0,\0b\0\r\0\n\0001\0,\0" "0\0" "0\0" "2\0\r\0\n\0", 22);
        const struct {
            const char *name, *copy;
            QByteArray bytes;
        } files[] = {{"a.csv", "copy.csv", "a;b\n1;2.50\n"},
                     {"b.csv", "copy.csv", "caf\xE9,007\r\n"},
                     {"c.csv", "copy.csv", "\xEF\xBB\xBF" "x,y\r\n\"q\"\"z\",\"3\"\r\n"},
                     {"d.tsv", "copy.tsv", "a\tb\n1\t2\r"},
                     {"e.csv", "copy.csv", utf16},
                     {"f.csv", "copy.csv", "a|b\n1|2\n"}};
        for (const auto &f : files) {
            Win o;
            QVERIFY2(o.open(f.bytes, QString::fromLatin1(f.name)), f.name);
            o.answer = o.folder.dir.filePath(QString::fromLatin1(f.copy));
            QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
            QVERIFY(o.w.save());
            QVERIFY2(exported.count() > 0 || exported.wait(20000), qPrintable(QString::fromLatin1(f.name) + QStringLiteral(" ") + o.told.join(QLatin1Char('|'))));
            QVERIFY2(exported.first().at(1).toBool(), f.name);
            QVERIFY2(o.folder.read(QString::fromLatin1(f.copy)) == f.bytes, f.name);
            QVERIFY(o.originalIsUntouched());
        }
    }

    // Save As of a file of values asks how to write it, whether or not it was edited: the question's answer decides. The file's own bytes are copied only when the answer is the file's own form;
    // another separator or encoding is written as asked.
    void saveAsOfAnUneditedCsvAlwaysAsksHowToWriteIt()
    {
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        const QByteArray bytes = "a;b\n1;2\n";
        struct Result {
            bool asked = false;
            QByteArray written;
            bool exists = false;
        };
        auto run = [&](const std::function<void(CsvSaveDialog *)> &act) {
            Result r;
            Win o;
            if (!o.open(bytes, QStringLiteral("v.csv"))) return r;
            o.w.setLossAsker(nullptr);   // (the question itself)
            QTimer finder;
            finder.setInterval(10);
            QObject::connect(&finder, &QTimer::timeout, &finder, [&] {
                auto *dialog = qobject_cast<CsvSaveDialog *>(QApplication::activeModalWidget());
                if (!dialog || r.asked) return;
                r.asked = true;
                act(dialog);
            });
            finder.start();
            QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
            const bool started = o.w.saveAs(o.folder.dir.filePath(QStringLiteral("copy.csv")));
            if (started && exported.count() == 0) exported.wait(20000);
            r.exists = QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("copy.csv")));
            r.written = o.folder.read(QStringLiteral("copy.csv"));
            return r;
        };
        const Result same = run([](CsvSaveDialog *d) { d->findChild<QPushButton *>(QStringLiteral("csvSaveValues"))->click(); });
        QVERIFY(same.asked);
        QCOMPARE(same.written, bytes);
        const Result tab = run([](CsvSaveDialog *d) {
            auto *sep = d->findChild<QComboBox *>(QStringLiteral("csvSeparator"));
            sep->setCurrentIndex(sep->findText(QStringLiteral("Tab")));
            d->findChild<QPushButton *>(QStringLiteral("csvSaveValues"))->click();
        });
        QVERIFY(tab.asked);
        QCOMPARE(tab.written, QByteArray("a\tb\n1\t2\n"));
        const Result utf16 = run([](CsvSaveDialog *d) {
            auto *enc = d->findChild<QComboBox *>(QStringLiteral("csvEncoding"));
            enc->setCurrentIndex(enc->findText(QStringLiteral("Unicode (UTF-16, little end first)")));
            d->findChild<QPushButton *>(QStringLiteral("csvSaveValues"))->click();
        });
        QVERIFY(utf16.asked);
        QVERIFY(utf16.written.startsWith("\xFF\xFE"));
        const Result cancelled = run([](CsvSaveDialog *d) { d->reject(); });
        QVERIFY(cancelled.asked);
        QVERIFY(!cancelled.exists);
    }

    // Once a cell is changed the file is written in the form it came in: its separator, its line ends, its encoding with or without the mark; a file of another ending is written the usual way.
    void anEditedCsvIsWrittenInTheFormItCameIn()
    {
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        const QByteArray utf16 = QByteArray("\xFF\xFE") + QByteArray("a\0,\0b\0\r\0\n\0001\0,\0" "2\0\r\0\n\0", 20);
        const QByteArray utf16Out = QByteArray("\xFF\xFE") + QByteArray("5\0,\0b\0\r\0\n\0001\0,\0" "2\0\r\0\n\0", 20);
        auto wide = [](const char *text) {
            QByteArray out("\xFE\xFF");
            for (const char *c = text; *c; ++c) {
                out += '\0';
                out += *c;
            }
            return out;
        };
        const struct {
            const char *name, *copy;
            QByteArray bytes, expected;
        } files[] = {{"a.csv", "copy.csv", "a;b\n1;2\n", "5;b\n1;2\n"},
                     {"b.csv", "copy.csv", "a,b\n\"caf\xE9\",2\n", "5,b\ncaf\xE9,2\n"},
                     {"c.csv", "copy.csv", "\xEF\xBB\xBF" "a,b\r\n1,2\r\n", "\xEF\xBB\xBF" "5,b\r\n1,2\r\n"},
                     {"d.tsv", "copy.tsv", "a\tb\r1\t2\r", "5\tb\r1\t2\r"},
                     {"e.csv", "copy.csv", utf16, utf16Out},
                     {"f.csv", "copy.csv", "a|b\n1|2\n", "5|b\n1|2\n"},
                     {"g.csv", "copy.csv", "a,b\n\" x\",2\n", "5,b\n\" x\",2\n"},
                     {"h.tsv", "copy.csv", "a\tb\n1\t2\n", "\xEF\xBB\xBF" "5,b\r\n1,2\r\n"},
                     {"j.csv", "copy.csv", wide("a,b\r\n1,2\r\n"), wide("5,b\r\n1,2\r\n")}};   // (UTF-16, big end first, stays so)
        for (const auto &f : files) {
            Win o;
            QVERIFY2(o.open(f.bytes, QString::fromLatin1(f.name)), f.name);
            o.select(0, 0);
            o.enter(QStringLiteral("5"));
            QVERIFY(o.settle());
            o.answer = o.folder.dir.filePath(QString::fromLatin1(f.copy));
            QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
            QVERIFY(o.w.save());
            QVERIFY2(exported.wait(20000), f.name);
            QVERIFY2(exported.first().at(1).toBool(), f.name);
            const QByteArray got = o.folder.read(QString::fromLatin1(f.copy));
            QVERIFY2(got == f.expected, qPrintable(QString::fromLatin1(f.name) + QStringLiteral(": ") + QString::fromLatin1(got.toHex())));
        }
    }

    // Save As of a workbook offers the tab-separated kind, and a name ending in .tsv or .txt writes the values of the sheet with tabs between them.
    void aTabSeparatedFileIsOneOfTheKindsSaved()
    {
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        const QByteArray book = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\" t=\"inlineStr\"><is><t>two words</t></is></c><c r=\"C1\" t=\"inlineStr\"><is><t>a,b</t></is></c></row>"))});
        for (const char *name : {"out.tsv", "out.txt"}) {
            Win o;
            QVERIFY(o.open(book, QStringLiteral("book.xlsx")));
            o.answer = o.folder.dir.filePath(QString::fromLatin1(name));
            QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
            o.w.activateWindow();
            QVERIFY(QTest::qWaitForWindowActive(&o.w));
            o.w.act(QStringLiteral("file.saveAs"))->trigger();
            QVERIFY2(exported.wait(20000), name);
            QVERIFY2(o.filters.first().contains(QLatin1String("*.tsv")), qPrintable(o.filters.first()));
            QCOMPARE(o.folder.read(QString::fromLatin1(name)), QByteArray("\xEF\xBB\xBF" "1\ttwo words\ta,b\r\n"));
        }
    }

    // A sheet is written as the person chooses: the separator, the encoding (with or without the mark, UTF-16, a single-byte page), the line ends, and the cells as they are shown or as they are
    // stored (every digit, a date as ISO text); a text with a space at either end is in quotes; a letter the page has no byte for is a question mark, and the person is told how many.
    void aSheetIsWrittenAsChosen()
    {
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        const QString rows = QStringLiteral(
            "<row r=\"1\"><c r=\"A1\" s=\"1\"><v>1234.5</v></c><c r=\"B1\" s=\"2\"><v>45293</v></c><c r=\"C1\" s=\"3\"><v>0.1</v></c><c r=\"D1\" t=\"inlineStr\"><is><t xml:space=\"preserve\"> pad</t></is></c>"
            "<c r=\"E1\" t=\"inlineStr\"><is><t>caf\u00e9 \u20ac</t></is></c><c r=\"F1\"><f>0.1+0.2</f><v>0.30000000000000004</v></c></row>");
        const QByteArray book = buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(),
                                          stylesXml(QString(), QStringLiteral("<xf numFmtId=\"0\"/><xf numFmtId=\"4\" applyNumberFormat=\"1\"/><xf numFmtId=\"14\" applyNumberFormat=\"1\"/><xf numFmtId=\"9\" applyNumberFormat=\"1\"/>")));
        Win o;
        QVERIFY(o.open(book, QStringLiteral("book.xlsx")));
        int n = 0;
        auto write = [&](const crunch::CsvWrite &how) {
            const QString name = QStringLiteral("out%1.csv").arg(++n);
            QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
            if (!o.w.exportCsv(o.folder.dir.filePath(name), &how) || !exported.wait(20000) || !exported.first().at(1).toBool()) return QByteArray("FAILED");
            return o.folder.read(name);
        };
        crunch::CsvWrite how;
        QCOMPARE(write(how), QByteArray("\xEF\xBB\xBF\"1,234.50\",1/2/2024,10%,\" pad\",caf\xC3\xA9 \xE2\x82\xAC,0.3\r\n"));
        how.asStored = true;
        how.separator = QLatin1Char(';');
        how.encoding = crunch::CsvWrite::Encoding::Utf8;
        how.lineEnd = QStringLiteral("\n");
        QCOMPARE(write(how), QByteArray("1234.5;2024-01-02;0.1;\" pad\";caf\xC3\xA9 \xE2\x82\xAC;0.30000000000000004\n"));
        how = crunch::CsvWrite();
        how.separator = QLatin1Char('|');
        how.encoding = crunch::CsvWrite::Encoding::Utf16Le;
        QByteArray wide("\xFF\xFE");
        for (const QChar c : QString::fromUtf8("1,234.50|1/2/2024|10%|\" pad\"|caf\xC3\xA9 \xE2\x82\xAC|0.3\r\n")) {
            wide += char(c.unicode() & 0xFF);
            wide += char(c.unicode() >> 8);
        }
        QCOMPARE(write(how), wide);
        how = crunch::CsvWrite();
        how.separator = QLatin1Char('\t');
        how.encoding = crunch::CsvWrite::Encoding::CodePage;
        how.codePage = 1252;
        QCOMPARE(write(how), QByteArray("1,234.50\t1/2/2024\t10%\t\" pad\"\tcaf\xE9 \x80\t0.3\r\n"));
        how.codePage = 28592;   // (Latin-2 has no euro sign)
        QCOMPARE(write(how), QByteArray("1,234.50\t1/2/2024\t10%\t\" pad\"\tcaf\xE9 ?\t0.3\r\n"));
        QVERIFY2(o.w.statusText().contains(QLatin1String("1 characters")), qPrintable(o.w.statusText()));
    }

    // The question before a sheet is saved as values offers those choices; the answer carries them, and `sameAsFile` stays on only while they are the file's own.
    void theQuestionBeforeACsvOffersTheChoices()
    {
        crunch::CsvWrite start;
        start.separator = QLatin1Char(';');
        start.lineEnd = QStringLiteral("\n");
        start.sameAsFile = true;
        CsvSaveDialog d(QStringLiteral("Only the values are saved."), start);
        QCOMPARE(d.options(), start);
        QVERIFY(d.options().sameAsFile);
        auto combo = [&](const char *name) { return d.findChild<QComboBox *>(QLatin1String(name)); };
        QVERIFY(combo("csvSeparator") && combo("csvEncoding") && combo("csvLineEnd") && combo("csvValues"));
        QCOMPARE(combo("csvSeparator")->currentText(), QStringLiteral("Semicolon"));
        QVERIFY(!d.findChild<QLineEdit *>(QStringLiteral("csvOtherSeparator"))->isEnabled());
        combo("csvSeparator")->setCurrentIndex(combo("csvSeparator")->findText(QStringLiteral("Tab")));
        QCOMPARE(d.options().separator, QLatin1Char('\t'));
        QVERIFY(!d.options().sameAsFile);
        combo("csvSeparator")->setCurrentIndex(combo("csvSeparator")->findText(QStringLiteral("Semicolon")));
        QVERIFY(d.options().sameAsFile);
        combo("csvSeparator")->setCurrentIndex(combo("csvSeparator")->count() - 1);
        QVERIFY(d.findChild<QLineEdit *>(QStringLiteral("csvOtherSeparator"))->isEnabled());
        d.findChild<QLineEdit *>(QStringLiteral("csvOtherSeparator"))->setText(QStringLiteral("#"));
        QCOMPARE(d.options().separator, QLatin1Char('#'));
        combo("csvEncoding")->setCurrentIndex(combo("csvEncoding")->findText(QStringLiteral("Unicode (UTF-16, little end first)")));
        QCOMPARE(d.options().encoding, crunch::CsvWrite::Encoding::Utf16Le);
        combo("csvEncoding")->setCurrentIndex(combo("csvEncoding")->findText(QStringLiteral("Central European (ISO 8859-2)")));
        QCOMPARE(d.options().encoding, crunch::CsvWrite::Encoding::CodePage);
        QCOMPARE(d.options().codePage, 28592);
        combo("csvLineEnd")->setCurrentIndex(0);
        QCOMPARE(d.options().lineEnd, QStringLiteral("\r\n"));
        combo("csvValues")->setCurrentIndex(1);
        QVERIFY(d.options().asStored);
        QCOMPARE(d.answer(), CsvSaveDialog::Answer::Cancel);
        d.findChild<QPushButton *>(QStringLiteral("csvSaveValues"))->click();
        QCOMPARE(d.answer(), CsvSaveDialog::Answer::SaveValues);
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

    // Before a sheet is saved as .csv the person is told what the file will not have, naming what this workbook has of it, and may save the workbook as .xlsx instead or give it up.
    void aCsvSaveNamesWhatItLeavesOutAndAsks()
    {
        const QByteArray book = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><f>A1+1</f><v>2</v></c></row>")),
                                           worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>5</v></c></row>"))});
        const QByteArray withPicture = withParts(book, {{QStringLiteral("xl/drawings/drawing1.xml"), QByteArray("<xdr:wsDr xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\"/>"),
                                                         QStringLiteral("application/vnd.openxmlformats-officedocument.drawing+xml"), 0, QStringLiteral("rIdD"), QStringLiteral("drawing"), QStringLiteral("../drawings/drawing1.xml")}});
        // go on: the values are written, after the question named the sheets, the formulas, and the picture
        {
            Win o;
            QVERIFY(o.open(withPicture, QStringLiteral("book.xlsx")));
            QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
            QVERIFY(o.w.saveAs(o.folder.dir.filePath(QStringLiteral("out.csv"))));
            QVERIFY(exported.wait(20000));
            QCOMPARE(o.told.size(), 1);
            const QString said = o.told.first();
            QVERIFY2(said.contains(QLatin1String("1 other sheet")) && said.contains(QLatin1String("formulas")) && said.contains(QLatin1String("pictures and charts"))
                         && said.contains(QLatin1String("column widths")) && said.contains(QLatin1String("leaves out")),
                     qPrintable(said));
            QVERIFY(QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("out.csv"))));
        }
        // a workbook of one sheet with no formulas is not told of what it does not have
        {
            Win o;
            QVERIFY(o.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"))}), QStringLiteral("plain.xlsx")));
            QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
            QVERIFY(o.w.saveAs(o.folder.dir.filePath(QStringLiteral("out.csv"))));
            QVERIFY(exported.wait(20000));
            const QString said = o.told.first().mid(o.told.first().indexOf(QLatin1String("In this workbook")));
            QVERIFY2(said.contains(QLatin1String("column widths")) && !said.contains(QLatin1String("other sheet")) && !said.contains(QLatin1String("formulas")) && !said.contains(QLatin1String("pictures")), qPrintable(said));
        }
        // cancel: nothing is written, and the window is as it was
        {
            Win o;
            QVERIFY(o.open(withPicture, QStringLiteral("book.xlsx")));
            o.lossAnswer = CrunchWindow::LossAnswer::Cancel;
            QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
            QVERIFY(!o.w.saveAs(o.folder.dir.filePath(QStringLiteral("out.csv"))));
            QCOMPARE(o.told.size(), 1);
            QVERIFY(!QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("out.csv"))));
            QCOMPARE(exported.count(), 0);
            QVERIFY(o.asked.isEmpty());
        }
        // save as .xlsx instead: the file dialog is asked for a workbook, and the workbook is saved as one with the picture in it
        {
            Win o;
            QVERIFY(o.open(withPicture, QStringLiteral("book.xlsx")));
            o.lossAnswer = CrunchWindow::LossAnswer::SaveXlsx;
            o.answerAfterLoss = o.folder.dir.filePath(QStringLiteral("copy.xlsx"));
            o.select(0, 0);
            o.enter(QStringLiteral("9"));
            QTRY_VERIFY(!o.w.isCalculating());
            QSignalSpy saved(&o.w, &CrunchWindow::saved), exported(&o.w, &CrunchWindow::csvExported);
            QVERIFY(!o.w.saveAs(o.folder.dir.filePath(QStringLiteral("out.csv"))));
            QVERIFY(savedOk(saved));
            QCOMPARE(exported.count(), 0);
            QVERIFY(!QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("out.csv"))));
            QCOMPARE(o.asked.size(), 1);
            QVERIFY2(o.filters.first().contains(QLatin1String("*.xlsx")), qPrintable(o.filters.first()));
            QVERIFY(QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("copy.xlsx"))));
        }
    }

    // Sheets that are hidden, and very hidden, are named as such in the question: one that shows and five that do not are "5 hidden sheets", not "other sheets".
    void hiddenSheetsAreNamedInTheQuestionBeforeACsv()
    {
        QVector<QByteArray> sheets;
        for (int i = 0; i < 6; ++i) sheets << worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>%1</v></c></row>").arg(i));
        QByteArray book = buildXlsx(sheets, QByteArray(), QString(), QString(), QByteArray(), QStringList(), QString(), QByteArray(), QString(), {1, 2, 3, 4, 5});
        // (one of them very hidden)
        QMap<QString, QByteArray> parts;
        readZip(book, parts);
        QByteArray &workbook = parts[QStringLiteral("xl/workbook.xml")];
        const qsizetype first = workbook.indexOf("state=\"hidden\"");
        QVERIFY(first >= 0);
        workbook.replace(first, 14, "state=\"veryHidden\"");
        ZipWriter z;
        for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
        book = z.finish();
        Win o;
        QVERIFY(o.open(book, QStringLiteral("hidden.xlsx")));
        o.lossAnswer = CrunchWindow::LossAnswer::Cancel;
        QVERIFY(!o.w.saveAs(o.folder.dir.filePath(QStringLiteral("out.csv"))));
        QCOMPARE(o.told.size(), 1);
        const QString said = o.told.first().mid(o.told.first().indexOf(QLatin1String("In this workbook")));
        QVERIFY2(said.contains(QLatin1String("5 hidden sheets")) && !said.contains(QLatin1String("other sheet")), qPrintable(o.told.first()));
    }

    // From the question asked when the window is closed with changes, "Save as .xlsx" saves the workbook and then closes the window; if the Save As is given up, or the question
    // is, the window stays, and a later save does not close it.
    void savingAsXlsxFromTheCsvQuestionOnCloseFinishesTheClose()
    {
        LocaleScope locale(QLocale(QLocale::English, QLocale::UnitedStates));
        auto start = [&](Win &o) {
            QVERIFY(o.open("a,b\n1,2\n", QStringLiteral("values.csv")));
            o.select(0, 0);
            o.enter(QStringLiteral("5"));
            QVERIFY(o.settle());
            o.w.setAsker([](const QString &, const QString &) { return CrunchWindow::Answer::Save; });
            o.answer = o.folder.dir.filePath(QStringLiteral("values.out.csv"));
            o.lossAnswer = CrunchWindow::LossAnswer::SaveXlsx;
        };
        {
            Win o;
            start(o);
            o.answerAfterLoss = o.folder.dir.filePath(QStringLiteral("book.xlsx"));
            QSignalSpy saved(&o.w, &CrunchWindow::saved);
            o.w.close();
            QVERIFY(savedOk(saved));
            QTRY_VERIFY(!o.w.isVisible());
            QVERIFY(QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("book.xlsx"))));
            QVERIFY(!QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("values.out.csv"))));
        }
        {
            Win o;
            start(o);
            o.cancelAfterLoss = true;
            o.w.close();
            QVERIFY(o.w.isVisible());
            QVERIFY(o.w.isModified());
            // a save that follows is only a save: the window stays
            o.lossAnswer = CrunchWindow::LossAnswer::Continue;
            QSignalSpy saved(&o.w, &CrunchWindow::saved);
            QVERIFY(o.w.saveAs(o.folder.dir.filePath(QStringLiteral("later.xlsx"))));
            QVERIFY(savedOk(saved));
            QVERIFY(QTest::qWaitFor([&] { return !o.w.isSaving(); }, 5000));   // (a close, if one was to be, was made when the file was done)
            QCoreApplication::processEvents();
            QVERIFY(o.w.isVisible());
        }
        {
            Win o;
            start(o);
            o.lossAnswer = CrunchWindow::LossAnswer::Cancel;
            o.w.close();
            QVERIFY(o.w.isVisible());
            QVERIFY(o.w.isModified());
            QVERIFY(!QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("values.out.csv"))));
            o.lossAnswer = CrunchWindow::LossAnswer::Continue;
            QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
            QVERIFY(o.w.saveAs(o.folder.dir.filePath(QStringLiteral("again.csv"))));
            QVERIFY(exported.wait(20000));
            QVERIFY(QTest::qWaitFor([&] { return !o.w.isSaving(); }, 5000));   // (a close, if one was to be, was made when the file was done)
            QCoreApplication::processEvents();
            QVERIFY(o.w.isVisible());
        }
    }

    // The question itself, as a person sees it: the default is to save the workbook as .xlsx (nothing is lost), Esc and the close box are Cancel, and each button does what it says.
    void theRealQuestionBeforeACsvDefaultsToXlsxAndEscapeCancels()
    {
        const QByteArray book = buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>")), worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>5</v></c></row>"))});
        auto ask = [&](const std::function<void(QDialog *)> &act, QString *defaultText, QString *answerAsked) {
            Win o;
            if (!o.open(book, QStringLiteral("book.xlsx"))) return QStringLiteral("(no open)");
            o.w.setLossAsker(nullptr);   // (the box itself)
            o.answer = o.folder.dir.filePath(QStringLiteral("picked.xlsx"));
            QTimer finder;
            finder.setInterval(10);
            bool acted = false;
            QObject::connect(&finder, &QTimer::timeout, &finder, [&] {
                if (acted) return;
                auto *box = qobject_cast<CsvSaveDialog *>(QApplication::activeModalWidget());
                if (!box) return;
                acted = true;
                if (defaultText)
                    for (QPushButton *button : box->findChildren<QPushButton *>())
                        if (button->isDefault()) *defaultText = button->text();
                act(box);
            });
            finder.start();
            QSignalSpy exported(&o.w, &CrunchWindow::csvExported);
            const bool started = o.w.saveAs(o.folder.dir.filePath(QStringLiteral("out.csv")));
            if (answerAsked) *answerAsked = o.asked.isEmpty() ? QString() : o.asked.first();
            if (started) exported.wait(20000);
            return started ? (QFileInfo::exists(o.folder.dir.filePath(QStringLiteral("out.csv"))) ? QStringLiteral("csv") : QStringLiteral("started")) : (o.asked.isEmpty() ? QStringLiteral("cancel") : QStringLiteral("xlsx"));
        };
        QString defaultText, asked;
        QCOMPARE(ask([](QDialog *b) { QTest::keyClick(b, Qt::Key_Escape); }, &defaultText, &asked), QStringLiteral("cancel"));
        QVERIFY2(defaultText.contains(QLatin1String(".xlsx")), qPrintable(defaultText));
        QCOMPARE(ask([](QDialog *b) { b->close(); }, nullptr, nullptr), QStringLiteral("cancel"));
        QCOMPARE(ask([](QDialog *b) { QTest::keyClick(b, Qt::Key_Return); }, nullptr, &asked), QStringLiteral("xlsx"));   // the default: the file dialog for the workbook follows
        QCOMPARE(ask([](QDialog *b) {
                      b->findChild<QPushButton *>(QStringLiteral("csvSaveValues"))->click();
                  },
                  nullptr, nullptr),
                 QStringLiteral("csv"));
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
            QVERIFY(QTest::qWaitFor([&] { return !o.w.isSaving(); }, 5000));
            QCoreApplication::processEvents();
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
