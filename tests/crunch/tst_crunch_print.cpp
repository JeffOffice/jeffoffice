// Tests of printing in JeffCrunch's window: how a sheet prints as a step of Undo, the print area, titles, and breaks following rows and columns inserted and deleted (and coming back with
// Undo), the page setup saved in the file and nothing else of it touched, and the pages the window prints. Every workbook is built here, in a temporary folder.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "core/printjob.h"
#include "grid/gridview.h"
#include "io/zipindex.h"

#include <QAction>
#include <QApplication>
#include <QSettings>
#include <QSignalSpy>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;
using jp::office::sheet::PageOrientation;
using jp::office::sheet::PageSetup;

namespace {

// Two sheets with a few numbers; the second has a note of its own so that its part can be told from the first's.
QByteArray printBook()
{
    QString rows;
    for (int r = 1; r <= 12; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><v>%2</v></c></row>").arg(r).arg(r * 2);
    const QByteArray first = worksheet(rows, QStringLiteral("<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" header=\"0.3\" footer=\"0.3\"/>"));
    const QByteArray second = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>second</t></is></c></row>"));
    return buildXlsx({first, second}, QByteArray(), QString(), QString(), QByteArray(), QStringList({"Data", "Notes"}));
}

QString areaOf(const PageSetup &p)
{
    QStringList l;
    for (const jp::office::sheet::CellRange &r : p.printArea) l << rangeName(r);
    return l.join(QLatin1Char(' '));
}

PageSetup withArea(PageSetup p, const QString &ref)
{
    jp::office::sheet::CellRange r;
    parseRangeRef(ref, &r);
    p.printArea = {r};
    return p;
}

// A saved file's page setup for a sheet.
PageSetup savedPage(const QString &path, int sheet = 0)
{
    Package p;
    QString why;
    if (!p.readLazyFile(path, &why)) return PageSetup();
    Workbook b;
    if (!b.open(p, &why)) return PageSetup();
    Sheet s;
    if (!b.readSheet(sheet, &s)) return PageSetup();
    return s.page;
}

} // namespace

class CrunchPrintTests : public QObject {
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
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }

    // A change to how a sheet prints is one step: Undo puts it back, Redo does it again, and the window says it has changes; the same setup is no step.
    void aPageChangeIsOneStepOfUndo()
    {
        EditWindow e;
        QVERIFY(e.open(printBook()));
        crunch::EditSession *session = e.win.w.session();
        QVERIFY(!e.win.w.isModified());
        PageSetup landscape = session->pages().sheet(0);
        landscape.orientation = PageOrientation::Landscape;
        QVERIFY(session->setPageSetup(0, landscape, QStringLiteral("Page orientation")).ok);
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        QVERIFY(session->pages().sheet(0).orientation == PageOrientation::Landscape);
        QVERIFY(e.win.w.isModified());
        QCOMPARE(e.win.w.undoStack()->undoText(), QStringLiteral("Page orientation"));
        QVERIFY(session->pages().sheet(1).orientation == PageOrientation::Default);
        QVERIFY(session->setPageSetup(0, landscape, QStringLiteral("Again")).ok);   // (nothing to change)
        QCOMPARE(e.win.w.undoStack()->count(), 1);
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(session->pages().sheet(0).orientation == PageOrientation::Default);
        QVERIFY(!e.win.w.isModified());
        e.win.w.act(QStringLiteral("edit.redo"))->trigger();
        QVERIFY(session->pages().sheet(0).orientation == PageOrientation::Landscape);
        // a sheet that does not exist is refused
        QVERIFY(!session->setPageSetup(7, landscape, QStringLiteral("x")).ok);
    }

    // The window is told when how a sheet prints changes: by the step, by Undo, and by rows inserted that move its print area.
    void theSessionTellsWhenASheetPrintsDifferently()
    {
        EditWindow e;
        QVERIFY(e.open(printBook()));
        crunch::EditSession *session = e.win.w.session();
        QSignalSpy changed(session, &crunch::EditSession::pageSetupChanged);
        QVERIFY(session->setPageSetup(0, withArea(session->pages().sheet(0), QStringLiteral("A1:B5")), QStringLiteral("Print area")).ok);
        QCOMPARE(changed.count(), 1);
        QCOMPARE(changed.last().at(0).toInt(), 0);
        QVERIFY(session->insertRows(0, 0, 1).ok);
        QVERIFY(changed.count() >= 2);
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(changed.count() >= 4);
    }

    // The print area, the titles, and the breaks move with the rows and columns inserted and deleted, and come back when the step is undone (and again when it is done again).
    void thePrintSettingsFollowInsertedAndDeletedRows()
    {
        EditWindow e;
        QVERIFY(e.open(printBook()));
        crunch::EditSession *session = e.win.w.session();
        PageSetup p = withArea(session->pages().sheet(0), QStringLiteral("A3:B8"));
        p.titles.hasRows = true;
        p.titles.row1 = 0;
        p.titles.row2 = 1;
        jp::office::sheet::PageBreak b;
        b.before = 6;
        b.max = 16383;
        p.rowBreaks.append(b);
        QVERIFY(session->setPageSetup(0, p, QStringLiteral("Print settings")).ok);
        QVERIFY(session->insertRows(0, 4, 2).ok);   // two rows inside the area, below the titles
        QCOMPARE(areaOf(session->pages().sheet(0)), QStringLiteral("A3:B10"));
        QCOMPARE(session->pages().sheet(0).rowBreaks[0].before, 8u);
        QCOMPARE(session->pages().sheet(0).titles.row2, 1u);
        QVERIFY(session->insertRows(0, 0, 1).ok);   // and one above everything
        QCOMPARE(areaOf(session->pages().sheet(0)), QStringLiteral("A4:B11"));
        QCOMPARE(session->pages().sheet(0).titles.row1, 1u);
        QCOMPARE(session->pages().sheet(0).rowBreaks[0].before, 9u);
        QVERIFY(session->deleteRows(0, 0, 3).ok);   // the title rows are among the three
        QVERIFY(!session->pages().sheet(0).titles.hasRows);
        QCOMPARE(areaOf(session->pages().sheet(0)), QStringLiteral("A1:B8"));
        // Undo puts each back, in turn
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(session->pages().sheet(0).titles.hasRows);
        QCOMPARE(areaOf(session->pages().sheet(0)), QStringLiteral("A4:B11"));
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QCOMPARE(areaOf(session->pages().sheet(0)), QStringLiteral("A3:B10"));
        QCOMPARE(session->pages().sheet(0).titles.row1, 0u);
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QCOMPARE(areaOf(session->pages().sheet(0)), QStringLiteral("A3:B8"));
        QCOMPARE(session->pages().sheet(0).rowBreaks[0].before, 6u);
        // and Redo does them again
        e.win.w.act(QStringLiteral("edit.redo"))->trigger();
        QCOMPARE(areaOf(session->pages().sheet(0)), QStringLiteral("A3:B10"));
        e.win.w.act(QStringLiteral("edit.redo"))->trigger();
        e.win.w.act(QStringLiteral("edit.redo"))->trigger();
        QVERIFY(!session->pages().sheet(0).titles.hasRows);
        QCOMPARE(areaOf(session->pages().sheet(0)), QStringLiteral("A1:B8"));
    }

    // Saving writes the setup (the elements of the sheet and the names of the print area and titles) and leaves the other sheet's part as it was; the file reads back as the window had it.
    void aSavedFileHasTheSetupAndNothingElseChanges()
    {
        EditWindow e;
        QVERIFY(e.open(printBook()));
        crunch::EditSession *session = e.win.w.session();
        PageSetup p = withArea(session->pages().sheet(0), QStringLiteral("A1:B6"));
        p.orientation = PageOrientation::Landscape;
        p.paperSize = 9;
        p.fitToPage = true;
        p.fitToWidth = 1;
        p.fitToHeight = 0;
        p.titles.hasRows = true;
        p.titles.row1 = 0;
        p.titles.row2 = 0;
        p.margins.left = 0.5;
        p.gridLines = true;
        p.headerFooter.oddFooter = QStringLiteral("&CPage &P of &N");
        jp::office::sheet::PageBreak br;
        br.before = 3;
        br.max = 16383;
        p.rowBreaks.append(br);
        QVERIFY(session->setPageSetup(0, p, QStringLiteral("Print settings")).ok);
        QFile before(e.path);
        QVERIFY(before.open(QIODevice::ReadOnly));
        const QByteArray was = before.readAll();
        before.close();
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        const PageSetup back = savedPage(e.path);
        QVERIFY(back == p);
        // the second sheet's part is the same bytes, and the first sheet's cells are as they were
        QFile after(e.path);
        QVERIFY(after.open(QIODevice::ReadOnly));
        const QByteArray now = after.readAll();
        QVector<jp::ZipEntry> a, b;
        QVERIFY(jp::readZipDirectory(was, &a) && jp::readZipDirectory(now, &b));
        auto raw = [](const QByteArray &zip, const QVector<jp::ZipEntry> &dir, const QString &name) {
            for (const jp::ZipEntry &entry : dir)
                if (entry.name == name) return zip.mid(int(entry.dataAt), int(entry.csize));
            return QByteArray();
        };
        QCOMPARE(raw(now, b, QStringLiteral("xl/worksheets/sheet2.xml")), raw(was, a, QStringLiteral("xl/worksheets/sheet2.xml")));
        QCOMPARE(savedCell(e.path, QStringLiteral("B12")), QStringLiteral("n:24"));
        QVERIFY(!e.win.w.isModified());
        // undoing the step after the save, and saving again, writes the old setup back
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QVERIFY(e.win.w.isModified());
        QSignalSpy savedAgain(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(savedAgain.wait(20000));
        const PageSetup undone = savedPage(e.path);
        QVERIFY(!undone.hasPrintArea());
        QVERIFY(!undone.titles.hasRows);
        QVERIFY(undone.orientation == PageOrientation::Default);
        QCOMPARE(undone.headerFooter.oddFooter, QString());
    }

    // A job cuts the sheet in view into pages with the setup the session holds; the whole workbook takes the sheets that are shown, numbering the pages on through them.
    void aJobCutsTheSheetsIntoPages()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        QString why;
        crunch::PrintJobOptions options;
        std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(options, &why);
        QVERIFY2(job, qPrintable(why));
        QCOMPARE(job->pageCount(), 3);   // (100 rows: 45, 45, and 10 on letter paper with the usual margins)
        QCOMPARE(job->sheetOf(0), 0);
        QCOMPARE(job->numberOf(2), 3);
        QCOMPARE(job->pageSize(0), QSizeF(612, 792));
        // what the session holds is what prints: landscape paper, and the whole sheet on one page
        crunch::EditSession *session = e.win.w.session();
        PageSetup p = session->pages().sheet(0);
        p.orientation = PageOrientation::Landscape;
        p.fitToPage = true;
        p.fitToWidth = 1;
        p.fitToHeight = 1;
        QVERIFY(session->setPageSetup(0, p, QStringLiteral("Page")).ok);
        job = e.win.w.makePrintJob(options, &why);
        QVERIFY2(job, qPrintable(why));
        QCOMPARE(job->pageCount(), 1);
        QCOMPARE(job->pageSize(0), QSizeF(792, 612));
        QVERIFY(job->printOf(0)->scalePercent < 100);
        // the choices of the job are put over the sheet's: no scaling, and A4 paper turned upright
        options.scaling = crunch::PrintScaling::NoScaling;
        options.setPaper = true;
        options.paperSize = 9;
        options.setOrientation = true;
        options.orientation = PageOrientation::Portrait;
        job = e.win.w.makePrintJob(options, &why);
        QVERIFY2(job, qPrintable(why));
        QVERIFY(job->pageCount() > 1);
        QVERIFY(qAbs(job->pageSize(0).width() - 595.276) < 0.01);
    }

    void theWholeWorkbookPrintsTheShownSheets()
    {
        EditWindow e;
        QVERIFY(e.open(budgetWorkbook()));   // three sheets, the second hidden
        crunch::PrintJobOptions options;
        options.what = crunch::PrintWhat::Workbook;
        QString why;
        const std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(options, &why);
        QVERIFY2(job, qPrintable(why));
        QCOMPARE(job->pageCount(), 3);   // (the budget is wider than a page: two pages across; the notes sheet is one)
        QCOMPARE(job->sheetOf(0), 0);
        QCOMPARE(job->sheetOf(1), 0);
        QCOMPARE(job->sheetOf(2), 2);
        QCOMPARE(job->sheetNameOf(2), QStringLiteral("Notes"));
        QCOMPARE(job->numberOf(0), 1);
        QCOMPARE(job->numberOf(2), 3);   // (the numbers go on from one sheet to the next)
        // a sheet's setup that starts its numbers anew
        crunch::EditSession *session = e.win.w.session();
        PageSetup p = session->pages().sheet(2);
        p.useFirstPageNumber = true;
        p.firstPageNumber = 7;
        QVERIFY(session->setPageSetup(2, p, QStringLiteral("Page")).ok);
        const std::unique_ptr<crunch::PrintJob> second = e.win.w.makePrintJob(options, &why);
        QCOMPARE(second->numberOf(2), 7);
    }

    void aSelectionPrintsAlone()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        e.view()->selectRange(50, 0, 59, 1);
        crunch::PrintJobOptions options;
        options.what = crunch::PrintWhat::Selection;
        QString why;
        const std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(options, &why);
        QVERIFY2(job, qPrintable(why));
        QCOMPARE(job->pageCount(), 1);
        QCOMPARE(job->printOf(0)->pages[0].row1, qint64(50));
        QCOMPARE(job->printOf(0)->pages[0].row2, qint64(59));
    }

    // The pages are put in a PDF file, whole or by a range, and a file that cannot be made is not left half made; the page ranges and the order of copies are read as they are typed.
    void thePagesGoToAPdfFile()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        const QString path = e.folder.dir.filePath(QStringLiteral("out.pdf"));
        QString why;
        QVERIFY2(e.win.w.exportPdfTo(path, crunch::PrintJobOptions(), QString(), &why), qPrintable(why));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray pdf = f.readAll();
        f.close();
        QVERIFY(pdf.startsWith("%PDF-"));
        auto pageObjects = [](const QByteArray &data) {
            int n = 0;
            for (qsizetype at = data.indexOf("/Type /Page"); at >= 0; at = data.indexOf("/Type /Page", at + 1))
                if (at + 11 >= data.size() || data[at + 11] != 's') ++n;
            return n;
        };
        QCOMPARE(pageObjects(pdf), 3);
        QVERIFY2(e.win.w.exportPdfTo(path, crunch::PrintJobOptions(), QStringLiteral("2-3"), &why), qPrintable(why));
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(pageObjects(f.readAll()), 2);
        f.close();
        // a range that names no page, and a place that cannot be written
        QVERIFY(!e.win.w.exportPdfTo(path, crunch::PrintJobOptions(), QStringLiteral("9-12"), &why));
        QVERIFY(!why.isEmpty());
        QVERIFY(!e.win.w.exportPdfTo(e.folder.dir.filePath(QStringLiteral("missing/out.pdf")), crunch::PrintJobOptions(), QString(), &why));
        QVERIFY(!QFileInfo::exists(e.folder.dir.filePath(QStringLiteral("missing/out.pdf"))));
        QCOMPARE(crunch::parsePageRange(QStringLiteral("1-3, 5"), 10), (QVector<int>{0, 1, 2, 4}));
        QCOMPARE(crunch::parsePageRange(QStringLiteral("8-"), 10), (QVector<int>{7, 8, 9}));
        QCOMPARE(crunch::parsePageRange(QStringLiteral("-2, 2, 99, x"), 10), (QVector<int>{0, 1}));
        QVERIFY(crunch::parsePageRange(QStringLiteral("abc"), 10).isEmpty());
        QCOMPARE(crunch::sheetOrder({0, 1, 2}, 2, true), (QVector<int>{0, 1, 2, 0, 1, 2}));
        QCOMPARE(crunch::sheetOrder({0, 1, 2}, 2, false), (QVector<int>{0, 0, 1, 1, 2, 2}));
        QCOMPARE(crunch::sheetOrder({0}, 0, true), (QVector<int>{0}));
    }

    // A page as a picture: the width asked for, the height of the paper's shape, and ink on it.
    void aPageCanBeSeenAsAPicture()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        QString why;
        const std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(crunch::PrintJobOptions(), &why);
        QVERIFY2(job, qPrintable(why));
        const QImage img = job->preview(0, 306);
        QCOMPARE(img.width(), 306);
        QCOMPARE(img.height(), 396);
        bool ink = false;
        for (int y = 0; y < img.height() && !ink; ++y)
            for (int x = 0; x < img.width() && !ink; ++x) ink = qGray(img.pixel(x, y)) < 100;
        QVERIFY(ink);
        QVERIFY(job->preview(50, 100).isNull());
    }

    // Rows inserted and the print area changed, then saved: the file has the area as the window shows it.
    void insertedRowsAndAPrintAreaAreSavedTogether()
    {
        EditWindow e;
        QVERIFY(e.open(printBook()));
        crunch::EditSession *session = e.win.w.session();
        PageSetup p = withArea(session->pages().sheet(0), QStringLiteral("A2:B6"));
        jp::office::sheet::PageBreak br;
        br.before = 4;
        br.max = 16383;
        p.rowBreaks.append(br);
        QVERIFY(session->setPageSetup(0, p, QStringLiteral("Print settings")).ok);
        QVERIFY(session->insertRows(0, 3, 2).ok);
        QVERIFY(session->insertColumns(0, 0, 1).ok);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY(saved.wait(20000));
        const PageSetup back = savedPage(e.path);
        QCOMPARE(areaOf(back), QStringLiteral("B2:C8"));
        QCOMPARE(back.rowBreaks.size(), 1);
        QCOMPARE(back.rowBreaks[0].before, 6u);
        QVERIFY(back == session->pages().sheet(0));
    }
};

JP_TEST_CLASS(CrunchPrintTests)
#include "tst_crunch_print.moc"
