#include "tst_crunch_print.h"
#include "app/theme.h"
#include "app/settings.h"
#include <QBuffer>
#include <QPdfWriter>
#include <QPainter>
#include <QImageReader>
#include <QRandomGenerator>
#include "findbooks.h"

#include "../plainpdfwatch.h"
#include "../emlpdf.h"
#include "../pdfmarkcheck.h"
#include "render/pdfpage.h"
#include "io/qtpdf.h"
#include "io/pdftransparency.h"
#include "sheet/links.h"
#include "core/editsession.h"
#include "app/passworddialogs.h"
#include "app/pdfprotection.h"
#include "io/pdfcrypt.h"
#include "render/pdfpage.h"

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>
#include <QScopeGuard>
#include <QScrollBar>

void CrunchPrintTests::initTestCase()
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


// A page break before the first row or column is no break (the file cannot hold one): it is refused, so that what is kept is what is saved.
void CrunchPrintTests::aBreakBeforeTheFirstRowOrColumnIsRefused()
{
    EditWindow e;
    QVERIFY(e.open(printBook()));
    crunch::EditSession *session = e.win.w.session();
    PageSetup setup = session->pages().sheet(0);
    jp::office::sheet::PageBreak first;
    first.before = 0;
    first.max = 16383;
    setup.rowBreaks.append(first);
    QVERIFY(!session->setPageSetup(0, setup, QStringLiteral("Break")).ok);
    QCOMPARE(e.win.w.undoStack()->count(), 0);
    setup = session->pages().sheet(0);
    first.max = 1048575;
    setup.colBreaks.append(first);
    QVERIFY(!session->setPageSetup(0, setup, QStringLiteral("Break")).ok);
    first.before = 2;
    setup = session->pages().sheet(0);
    setup.rowBreaks.append(first);
    QVERIFY(session->setPageSetup(0, setup, QStringLiteral("Break")).ok);
}


// A change to how a sheet prints is one step: Undo puts it back, Redo does it again, and the window says it has changes; the same setup is no step.
void CrunchPrintTests::aPageChangeIsOneStepOfUndo()
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


// A header or footer of more than 255 characters is one the other program refuses the file for: it is not a step, a text of 255 is, and a text the file came with that is longer stays as it is
// when only something else of the sheet is changed.
void CrunchPrintTests::aHeaderTooLongForTheFileIsRefused()
{
    EditWindow e;
    QVERIFY(e.open(printBook()));
    crunch::EditSession *session = e.win.w.session();
    PageSetup p = session->pages().sheet(0);
    p.headerFooter.oddHeader = QStringLiteral("&C") + QString(253, QLatin1Char('h'));
    QVERIFY(session->setPageSetup(0, p, QStringLiteral("Header")).ok);
    for (QString jp::office::sheet::HeaderFooter::*member : {&jp::office::sheet::HeaderFooter::oddHeader, &jp::office::sheet::HeaderFooter::oddFooter, &jp::office::sheet::HeaderFooter::evenHeader,
                                                             &jp::office::sheet::HeaderFooter::evenFooter, &jp::office::sheet::HeaderFooter::firstHeader, &jp::office::sheet::HeaderFooter::firstFooter}) {
        PageSetup q = session->pages().sheet(0);
        q.headerFooter.*member = QString(256, QLatin1Char('x'));
        const auto refused = session->setPageSetup(0, q, QStringLiteral("Header"));
        QVERIFY(!refused.ok);
        QVERIFY2(refused.problem.contains(QStringLiteral("255")), qPrintable(refused.problem));
    }
    QCOMPARE(session->pages().sheet(0).headerFooter.oddHeader.size(), 255);
}


// A sheet has up to 1,024 manual breaks of each kind in a file the other program opens: more are refused, and a sheet that came with more keeps them when only something else changes.
void CrunchPrintTests::moreBreaksThanTheFileMayHoldAreRefused()
{
    EditWindow e;
    QVERIFY(e.open(printBook()));
    crunch::EditSession *session = e.win.w.session();
    PageSetup p = session->pages().sheet(0);
    for (int i = 1; i <= jp::office::sheet::kMaxPageBreaks; ++i) {
        jp::office::sheet::PageBreak b;
        b.before = quint32(i);
        b.max = 16383;
        p.rowBreaks.append(b);
    }
    QVERIFY(session->setPageSetup(0, p, QStringLiteral("Breaks")).ok);
    jp::office::sheet::PageBreak more;
    more.before = 2000;
    more.max = 16383;
    p.rowBreaks.append(more);
    const auto refused = session->setPageSetup(0, p, QStringLiteral("Breaks"));
    QVERIFY(!refused.ok);
    QVERIFY2(refused.problem.contains(QStringLiteral("1,024")) || refused.problem.contains(QStringLiteral("1024")), qPrintable(refused.problem));
    QCOMPARE(session->pages().sheet(0).rowBreaks.size(), jp::office::sheet::kMaxPageBreaks);
}


// The window is told when how a sheet prints changes: by the step, by Undo, and by rows inserted that move its print area.
void CrunchPrintTests::theSessionTellsWhenASheetPrintsDifferently()
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
void CrunchPrintTests::thePrintSettingsFollowInsertedAndDeletedRows()
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
void CrunchPrintTests::aSavedFileHasTheSetupAndNothingElseChanges()
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
    QVERIFY2(savedOk(saved), "the save failed");
    QVERIFY2(saved.first().at(1).toBool(), "the save failed");
    const PageSetup back = savedPage(e.path);
    QVERIFY(back == p);
    // the second sheet's part is the same bytes, and the first sheet's cells are as they were
    QFile after(e.path);
    QVERIFY(after.open(QIODevice::ReadOnly));
    const QByteArray now = after.readAll();
    after.close();   // (an open file can't be replaced on Windows: the next save would fail)
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
    QVERIFY2(savedOk(savedAgain, 20000), "the save failed");
    QVERIFY2(savedAgain.first().at(1).toBool(), "the second save failed");
    const PageSetup undone = savedPage(e.path);
    QVERIFY(!undone.hasPrintArea());
    QVERIFY(!undone.titles.hasRows);
    QVERIFY(undone.orientation == PageOrientation::Default);
    QCOMPARE(undone.headerFooter.oddFooter, QString());
}


// A job cuts the sheet in view into pages with the setup the session holds; the whole workbook takes the sheets that are shown, numbering the pages on through them.
void CrunchPrintTests::aJobCutsTheSheetsIntoPages()
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


void CrunchPrintTests::theWholeWorkbookPrintsTheShownSheets()
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


// With several sheet tabs picked, "this sheet" prints all of them, one after the other.
void CrunchPrintTests::theSelectedSheetsPrintTogether()
{
    EditWindow e;
    QVERIFY(e.open(budgetWorkbook()));   // three sheets, the second hidden
    crunch::PrintJobOptions options;
    options.what = crunch::PrintWhat::ActiveSheet;
    QString why;
    std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(options, &why);
    QVERIFY2(job, qPrintable(why));
    QCOMPARE(job->pageCount(), 2);   // (the first sheet alone: two pages across)
    e.win.w.sheetTabs()->setSelectedTabs({0, 1});
    job = e.win.w.makePrintJob(options, &why);
    QVERIFY2(job, qPrintable(why));
    QCOMPARE(job->pageCount(), 3);
    QCOMPARE(job->sheetOf(2), 2);
    QCOMPARE(job->numberOf(2), 3);
}


void CrunchPrintTests::aSelectionPrintsAlone()
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


// A PDF/A export says what it is (the identification in its metadata and an output intent for color); the usual export does not.
void CrunchPrintTests::aPdfCanBeExportedAsPdfA()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(30, {})));
    auto make = [&](bool archival, const QString &name) {
        const QString path = e.folder.dir.filePath(name);
        crunch::PrintJobOptions options;
        options.archival = archival;
        QString why;
        if (!e.win.w.exportPdfTo(path, options, QString(), &why)) return QByteArray();
        QFile f(path);
        return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
    };
    const QByteArray plain = make(false, QStringLiteral("plain.pdf")), archive = make(true, QStringLiteral("archive.pdf"));
    QVERIFY(plain.startsWith("%PDF-") && archive.startsWith("%PDF-"));
    QVERIFY(!plain.contains("/OutputIntents"));
    QVERIFY(archive.contains("/OutputIntents"));
    QVERIFY(archive.contains("pdfaid:part=\"1\""));
    QVERIFY(!archive.contains("pdfaid:part=\"2\""));
}


// PDF/A-1 allows no transparency: a chart and a shape on a sheet are made into pictures with clear corners, which a PDF holds with a soft mask; in a PDF/A export they are
// drawn solid over the page (the usual export keeps them as they are).
void CrunchPrintTests::aPdfAFileOfASheetWithObjectsHasNoTransparency()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(30, {})));
    e.view()->selectRange(0, 0, 3, 2);
    QVERIFY(e.win.w.insertChart(jp::office::sheet::ChartKind::Column));
    QVERIFY(e.win.w.insertShape(QStringLiteral("ellipse")));
    QVERIFY(e.settle());
    QImage looks[2];
    for (const bool archival : {true, false}) {
        const QString path = e.folder.dir.filePath(archival ? QStringLiteral("a.pdf") : QStringLiteral("p.pdf"));
        crunch::PrintJobOptions options;
        options.archival = archival;
        QString why;
        QVERIFY2(e.win.w.exportPdfTo(path, options, QString(), &why), qPrintable(why));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        const jp::PdfTransparency found = jp::pdfTransparency(bytes);
        QCOMPARE(found.found, !archival);
        const PdfDocument pdf(bytes);
        QVERIFY(pdf.isValid());
        QImage page(pdf.pageSize(0).toSize() * 2, QImage::Format_RGB32);
        page.fill(Qt::white);
        {
            QPainter on(&page);
            on.drawImage(0, 0, pdf.render(0, page.size()));
        }
        looks[archival ? 1 : 0] = page;
    }
    // the page looks the same (the clear corners of the pictures are not white boxes over the cells beneath)
    QCOMPARE(looks[0].size(), looks[1].size());
    int far = 0;
    for (int y = 0; y < looks[0].height(); ++y)
        for (int x = 0; x < looks[0].width(); ++x) {
            const QRgb a = looks[0].pixel(x, y), b = looks[1].pixel(x, y);
            far += std::abs(qGray(a) - qGray(b)) > 60;
        }
    QVERIFY2(far < looks[0].width() * looks[0].height() / 500, qPrintable(QString::number(far)));
}


namespace {
// The text of a PDF string written as UTF-16 in hexadecimal (<FEFF...>), as the outline's titles are.
QString pdfHexTitle(const QByteArray &hex)
{
    const QByteArray bytes = QByteArray::fromHex(hex);
    QString out;
    for (qsizetype i = 2; i + 1 < bytes.size(); i += 2) out += QChar(ushort((uchar(bytes[i]) << 8) | uchar(bytes[i + 1])));
    return out;
}

struct OutlineRead {
    QStringList titles;
    QVector<int> pageIndex;   // the PDF page (from 0) each entry goes to
    bool hasOutlines = false;
};

OutlineRead readOutline(const QByteArray &pdf)
{
    OutlineRead out;
    jp::QtPdf q;
    // (the pages are those of the file as Qt wrote it; the outline is an update after its first end)
    if (!q.loadBytes(pdf.left(int(pdf.indexOf("%%EOF")) + 6))) return out;
    QVector<int> pageIds;
    for (const jp::QtPdf::Obj &obj : q.objects)
        if (obj.body.contains("/Type /Page\n")) pageIds << obj.id;
    out.hasOutlines = pdf.contains("/Type /Outlines");
    static const QRegularExpression entry(QStringLiteral("/Title <([0-9A-Fa-f]+)> /Parent \\d+ 0 R[^>]*?/Dest \\[(\\d+) 0 R"));
    for (auto it = entry.globalMatch(QString::fromLatin1(pdf)); it.hasNext();) {
        const auto m = it.next();
        out.titles << pdfHexTitle(m.captured(1).toLatin1());
        out.pageIndex << int(pageIds.indexOf(m.captured(2).toInt()));
    }
    return out;
}

QByteArray textRows(int rows)
{
    QString xml;
    for (int r = 1; r <= rows; ++r) xml += QStringLiteral("<row r=\"%1\"><c r=\"A%1\" t=\"inlineStr\"><is><t>row %1</t></is></c></row>").arg(r);
    return worksheet(xml);
}
} // namespace

// An exported PDF has a bookmark for each sheet, going to the page its first page is on (a sheet of several pages is one bookmark, and the next sheet's goes after those pages).
void CrunchPrintTests::aPdfHasABookmarkForEachSheet()
{
    EditWindow e;
    QVERIFY(e.open(buildXlsx({textRows(150), textRows(1), textRows(1)}, QByteArray(), QString(), QString(), budgetStyles(), QStringList({"Long", "Second", "Third & last"}))));
    const QString path = e.folder.dir.filePath(QStringLiteral("marks.pdf"));
    crunch::PrintJobOptions options;
    options.what = crunch::PrintWhat::Workbook;
    QString why;
    QVERIFY2(e.win.w.exportPdfTo(path, options, QString(), &why), qPrintable(why));
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray pdf = f.readAll();
    const OutlineRead o = readOutline(pdf);
    QVERIFY(o.hasOutlines);
    QCOMPARE(o.titles, QStringList({"Long", "Second", "Third & last"}));
    QCOMPARE(o.pageIndex[0], 0);
    QVERIFY2(o.pageIndex[1] >= 2, "the long sheet has more than one page");
    QCOMPARE(o.pageIndex[2], o.pageIndex[1] + 1);
    QVERIFY(pdf.contains("/PageMode /UseOutlines"));
}

// A workbook of a thousand sheets (the most the reader takes) gets a bookmark each, and the bookmarks added to a PDF are held to jp::kMostPdfBookmarks whatever list they are given.
void CrunchPrintTests::aPdfOfThousandsOfSheetsHasBoundedBookmarks()
{
    const int sheets = 1000;   // (the most the reader takes: a file with more is refused)
    QVector<QByteArray> parts;
    QStringList names;
    const QByteArray one = textRows(1);
    for (int i = 0; i < sheets; ++i) {
        parts << one;
        names << QStringLiteral("S%1").arg(i + 1);
    }
    EditWindow e;
    QVERIFY(e.open(buildXlsx(parts, QByteArray(), QString(), QString(), budgetStyles(), names)));
    const QString path = e.folder.dir.filePath(QStringLiteral("many.pdf"));
    crunch::PrintJobOptions options;
    options.what = crunch::PrintWhat::Workbook;
    QString why;
    std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(options, &why);
    QVERIFY2(job, qPrintable(why));
    QVector<int> pages;
    for (int i = 0; i < job->pageCount(); ++i) pages << i;
    // (sixteen pages to a sheet of paper: a few dozen PDF pages hold them all)
    QVERIFY2(job->exportPdf(path, pages, &why, nullptr, 16), qPrintable(why));
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const OutlineRead o = readOutline(f.readAll());
    QVERIFY(o.titles.size() <= jp::kMostPdfBookmarks);
    QCOMPARE(int(o.titles.size()), 1000);
    QCOMPARE(o.titles.first(), QStringLiteral("S1"));
    QCOMPARE(o.titles.last(), QStringLiteral("S1000"));
    // (the sheet of the sixteenth is on the first PDF page, the seventeenth's on the second)
    QCOMPARE(o.pageIndex[15], 0);
    QCOMPARE(o.pageIndex[16], 1);
    // The list a PDF is given is cut: more than the most make exactly the most.
    QByteArray made;
    {
        QBuffer buffer(&made);
        buffer.open(QIODevice::WriteOnly);
        QPdfWriter pdf(&buffer);
        QPainter p(&pdf);
        for (int i = 0; i < jp::kMostPdfBookmarks + 100; ++i) {
            if (i) pdf.newPage();
            p.drawText(10, 10, QStringLiteral("x"));
        }
    }
    QVector<jp::PdfBookmark> marks;
    for (int i = 0; i < jp::kMostPdfBookmarks + 100; ++i) marks.push_back({QStringLiteral("M%1").arg(i), 0, i});
    QCOMPARE(int(readOutline(jp::addPdfBookmarks(made, marks)).titles.size()), jp::kMostPdfBookmarks);
}

namespace {
// The structure elements of a PDF by type, each with its attribute text, in file order.
QVector<QPair<QString, QString>> structureElements(const QByteArray &pdf)
{
    QVector<QPair<QString, QString>> out;
    jp::QtPdf q;
    if (!q.loadBytes(pdf)) return out;
    static const QRegularExpression re(QStringLiteral("/Type /StructElem /S /(\\w+)"));
    for (const jp::QtPdf::Obj &obj : q.objects) {
        const QString dict = QString::fromLatin1(jp::QtPdf::dictOf(obj.body));
        const auto m = re.match(dict);
        if (m.hasMatch()) out.push_back({m.captured(1), dict});
    }
    return out;
}
} // namespace

// The rows printed again on every page (the print titles) are header cells (TH, each heading the cells below it), and a merged range says how many columns and rows it covers (ColSpan and
// RowSpan), in a PDF and in a PDF/A one.
void CrunchPrintTests::theTaggedCellsHaveHeadersAndSpans()
{
    const QByteArray sheet = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>h1</t></is></c><c r=\"B1\" t=\"inlineStr\"><is><t>h2</t></is></c></row>"
                                                      "<row r=\"2\"><c r=\"A2\" t=\"inlineStr\"><is><t>a</t></is></c><c r=\"B2\" t=\"inlineStr\"><is><t>b</t></is></c></row>"
                                                      "<row r=\"3\"><c r=\"A3\" t=\"inlineStr\"><is><t>wide</t></is></c></row>"
                                                      "<row r=\"4\"><c r=\"A4\" t=\"inlineStr\"><is><t>tall</t></is></c><c r=\"B4\" t=\"inlineStr\"><is><t>c</t></is></c></row>"),
                                       QStringLiteral("<mergeCells count=\"2\"><mergeCell ref=\"A3:B3\"/><mergeCell ref=\"A4:A5\"/></mergeCells>"));
    EditWindow e;
    QVERIFY(e.open(buildXlsx({sheet})));
    crunch::EditSession *session = e.win.w.session();
    PageSetup p = session->pages().sheet(0);
    p.titles.hasRows = true;
    p.titles.row1 = 0;
    p.titles.row2 = 0;
    QVERIFY(session->setPageSetup(0, p, QStringLiteral("Print titles")).ok);
    for (const bool archival : {false, true}) {
        crunch::PrintJobOptions options;
        options.pdfTags = true;
        options.archival = archival;
        const QString path = e.folder.dir.filePath(QStringLiteral("spans%1.pdf").arg(archival));
        QString why;
        QVERIFY2(e.win.w.exportPdfTo(path, options, QString(), &why), qPrintable(why));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray pdf = f.readAll();
        QCOMPARE(pdfcheck::structureProblems(pdf), QString());
        int th = 0, td = 0, wide = 0, tall = 0;
        for (const auto &el : structureElements(pdf)) {
            if (el.first == QLatin1String("TH")) {
                ++th;
                QVERIFY2(el.second.contains(QLatin1String("/Scope /Column")), qPrintable(el.second));
            }
            if (el.first == QLatin1String("TD")) {
                ++td;
                QVERIFY2(!el.second.contains(QLatin1String("/Scope")), qPrintable(el.second));
            }
            wide += el.second.contains(QLatin1String("/ColSpan 2")) && !el.second.contains(QLatin1String("/RowSpan"));
            tall += el.second.contains(QLatin1String("/RowSpan 2")) && !el.second.contains(QLatin1String("/ColSpan"));
        }
        QCOMPARE(th, 2);
        QCOMPARE(td, 5);
        QCOMPARE(wide, 1);
        QCOMPARE(tall, 1);
    }
}

// A gray job is made gray by the PDF, not as pictures of its pages: its words are words (and are tagged), and it comes out gray.
void CrunchPrintTests::aGrayJobIsTaggedLikeAColorOne()
{
    const QByteArray sheet = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>gray words</t></is></c></row>"));
    EditWindow e;
    QVERIFY(e.open(buildXlsx({sheet})));
    crunch::PrintJobOptions options;
    options.pdfTags = true;
    options.grayscale = true;
    const QString path = e.folder.dir.filePath(QStringLiteral("gray-tagged.pdf"));
    QString why;
    QVERIFY2(e.win.w.exportPdfTo(path, options, QString(), &why), qPrintable(why));
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray pdf = f.readAll();
    QVERIFY(pdf.contains("/StructTreeRoot"));
    QCOMPARE(pdfcheck::marksLeft(pdf), 0);
    QCOMPARE(pdfcheck::structureProblems(pdf), QString());
    const PdfDocument doc(pdf);
    QVERIFY(doc.isValid());
    QVERIFY2(doc.text(0).contains(QStringLiteral("gray words")), "the page is a picture, not words");
}

// The PDFs of Print > Save as PDF and of Make an Email are tagged as an export's are (the Export page's choice, on unless turned off).
void CrunchPrintTests::savedAndEmailedPdfsAreTagged()
{
    EditWindow e;
    QVERIFY(e.open(budgetWorkbook()));
    CrunchWindow &w = e.win.w;
    jp::Settings::get().setValue(QStringLiteral("export/pdfTags"), true);
    const QString file = e.folder.dir.filePath(QStringLiteral("saved.pdf"));
    crunch::PrintChoices c;
    c.toFile = true;
    w.setPrinterPreparer(nullptr);
    w.setSavePicker([&](const QString &, const QString &) { return file; });
    QVERIFY(w.printWith(c));
    {
        QFile f(file);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QVERIFY2(f.readAll().contains("/StructTreeRoot"), "Save as PDF is not tagged");
    }
    const QString eml = e.folder.dir.filePath(QStringLiteral("tagged.eml"));
    w.setSavePicker([&](const QString &, const QString &) { return eml; });
    QVERIFY(w.exportEmail(CrunchWindow::EmailWhat::Pdf));
    {
        QFile f(eml);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QVERIFY2(pdfAttachmentOf(f.readAll()).contains("/StructTreeRoot"), "the emailed PDF is not tagged");
    }
    // with the choice off, neither is
    jp::Settings::get().setValue(QStringLiteral("export/pdfTags"), false);
    const auto restore = qScopeGuard([] { jp::Settings::get().setValue(QStringLiteral("export/pdfTags"), true); });
    w.setSavePicker([&](const QString &, const QString &) { return file; });
    QVERIFY(w.printWith(c));
    QFile f(file);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QVERIFY(!f.readAll().contains("/StructTreeRoot"));
}

// An export that would pass the most tags a PDF is given is made without any (never half tagged), and says so; the bound is a count of tags, not of time.
void CrunchPrintTests::aPdfTooBigToTagIsMadeUntaggedAndSaysSo()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(300, {})));
    crunch::PrintJobOptions options;
    options.pdfTags = true;
    options.what = crunch::PrintWhat::Workbook;
    QString why;
    const auto restore = qScopeGuard([old = jp::grid::mostSheetTags()] { jp::grid::mostSheetTags() = old; });
    auto make = [&](const QString &name, QStringList *said) {
        std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(options, &why);
        QVERIFY2(job, qPrintable(why));
        QVector<int> pages;
        for (int i = 0; i < job->pageCount(); ++i) pages << i;
        const QString path = e.folder.dir.filePath(name);
        QVERIFY2(job->exportPdf(path, pages, &why), qPrintable(why));
        *said = job->messages();
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        m_made = f.readAll();
    };
    QStringList said;
    make(QStringLiteral("fits.pdf"), &said);
    QVERIFY(m_made.contains("/StructTreeRoot"));
    QVERIFY(said.isEmpty());
    jp::grid::mostSheetTags() = 40;
    make(QStringLiteral("big.pdf"), &said);
    QVERIFY(m_made.startsWith("%PDF-"));
    QVERIFY2(!m_made.contains("/StructTreeRoot") && !m_made.contains("/MarkInfo"), "half tagged");
    QCOMPARE(pdfcheck::marksLeft(m_made), 0);
    QVERIFY(said.join(QLatin1Char(' ')).contains(QLatin1String("too large to tag")));
}

// The tagging is part of the progress (the bar counts it as a second share, as long as the pages), and Cancel during it stops cleanly: no file is made.
void CrunchPrintTests::taggingCountsInTheProgressAndCanBeCancelled()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(200, {})));
    crunch::PrintJobOptions options;
    options.pdfTags = true;
    options.what = crunch::PrintWhat::Workbook;
    QString why;
    std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(options, &why);
    QVERIFY2(job, qPrintable(why));
    QVector<int> pages;
    for (int i = 0; i < job->pageCount(); ++i) pages << i;
    const int n = int(pages.size());
    QVERIFY(n >= 2);
    const QString path = e.folder.dir.filePath(QStringLiteral("progress.pdf"));
    int last = 0, total = 0, tagging = 0;
    QVERIFY2(job->exportPdf(path, pages, &why, [&](int done, int count) {
        total = count;
        if (done > n) ++tagging;
        last = done;
        return true;
    }), qPrintable(why));
    QCOMPARE(total, 2 * n);
    QVERIFY(tagging > 0);
    QVERIFY(last >= 2 * n - 1);
    // Cancel once the tagging has begun
    QFile::remove(path);
    std::unique_ptr<crunch::PrintJob> again = e.win.w.makePrintJob(options, &why);
    QVERIFY(again);
    QVERIFY(!again->exportPdf(path, pages, &why, [&](int done, int) { return done <= n; }));
    QVERIFY(again->wasCancelled());
    QVERIFY(!QFile::exists(path));
}

// A PDF can be tagged for screen readers: each page's cells are a table (a row for each row that has text, a cell for each cell that has), the marks are all gone, and the language
// is named; without the option there is no structure.
void CrunchPrintTests::aTaggedPdfHasTheCellsAsATable()
{
    const QByteArray sheet = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>a</t></is></c><c r=\"B1\" t=\"inlineStr\"><is><t>b</t></is></c></row>"
                                                      "<row r=\"2\"><c r=\"A2\" t=\"inlineStr\"><is><t>c</t></is></c><c r=\"C2\" t=\"inlineStr\"><is><t>d</t></is></c></row>"
                                                      "<row r=\"4\"><c r=\"B4\" t=\"inlineStr\"><is><t>e</t></is></c></row>"));
    EditWindow e;
    QVERIFY(e.open(buildXlsx({sheet})));
    auto make = [&](bool tags, bool archival, const QString &name) {
        crunch::PrintJobOptions options;
        options.pdfTags = tags;
        options.archival = archival;
        QString why;
        const QString path = e.folder.dir.filePath(name);
        if (!e.win.w.exportPdfTo(path, options, QString(), &why)) return QByteArray();
        QFile f(path);
        return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
    };
    const QByteArray plain = make(false, false, QStringLiteral("plain.pdf"));
    QVERIFY(plain.startsWith("%PDF-") && !plain.contains("/StructTreeRoot"));
    for (bool archival : {false, true}) {
        const QByteArray pdf = make(true, archival, QStringLiteral("tagged%1.pdf").arg(archival));
        QVERIFY(pdf.startsWith("%PDF-"));
        QVERIFY(pdf.contains("/StructTreeRoot"));
        QVERIFY(pdf.contains("/MarkInfo"));
        QVERIFY(pdf.contains("/Lang ("));
        QCOMPARE(pdfcheck::marksLeft(pdf), 0);
        QCOMPARE(pdfcheck::structureProblems(pdf), QString());
        jp::QtPdf q;
        QVERIFY(q.loadBytes(pdf));
        QMap<QString, int> types;
        for (const jp::QtPdf::Obj &obj : q.objects) {
            const auto m = QRegularExpression(QStringLiteral("/Type /StructElem /S /(\\w+)")).match(QString::fromLatin1(obj.body));
            if (m.hasMatch()) ++types[m.captured(1)];
        }
        QCOMPARE(types.value(QStringLiteral("Table")), 1);
        QCOMPARE(types.value(QStringLiteral("TR")), 3);
        QCOMPARE(types.value(QStringLiteral("TD")), 5);
        if (archival) QVERIFY(pdf.contains("pdfaid:part=\"1\""));
    }
}

// The cells are read in their order (a row from left to right, rows from the top down), whatever order the grid paints them in, and each cell's words are in the cell that holds them: the
// cells' words are 1, 2, 3, and 4 letters long, so the glyphs under each TD, read in the tree's order, are counted 1, 2, 3, 4 (the merged range, painted last, is the fourth).
void CrunchPrintTests::theTaggedCellsAreReadRowByRow()
{
    const QByteArray sheet = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>a</t></is></c><c r=\"B1\" t=\"inlineStr\"><is><t>bb</t></is></c></row>"
                                                      "<row r=\"2\"><c r=\"A2\" t=\"inlineStr\"><is><t>ccc</t></is></c></row>"
                                                      "<row r=\"3\"><c r=\"A3\" t=\"inlineStr\"><is><t>dddd</t></is></c></row>"),
                                       QStringLiteral("<mergeCells count=\"1\"><mergeCell ref=\"A3:B3\"/></mergeCells>"));
    EditWindow e;
    QVERIFY(e.open(buildXlsx({sheet})));
    crunch::PrintJobOptions options;
    options.pdfTags = true;
    const QString path = e.folder.dir.filePath(QStringLiteral("order.pdf"));
    QString why;
    QVERIFY2(e.win.w.exportPdfTo(path, options, QString(), &why), qPrintable(why));
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray pdf = f.readAll();
    QCOMPARE(pdfcheck::structureProblems(pdf), QString());
    jp::QtPdf q;
    QVERIFY(q.loadBytes(pdf));
    // the glyphs of each marked content of the page, by MCID
    const QVector<QByteArray> pages = pdfcheck::pageContents(pdf);
    QCOMPARE(int(pages.size()), 1);
    QHash<int, int> glyphs;
    const QString content = QString::fromLatin1(pages[0]);
    for (auto it = QRegularExpression(QStringLiteral("/TD <</MCID (\\d+)>> BDC\\n(.*?)EMC\\n"), QRegularExpression::DotMatchesEverythingOption).globalMatch(content); it.hasNext();) {
        const auto m = it.next();
        int digits = 0;
        for (auto h = QRegularExpression(QStringLiteral("<([0-9A-Fa-f]+)>")).globalMatch(m.captured(2)); h.hasNext();) digits += int(h.next().captured(1).size());
        glyphs.insert(m.captured(1).toInt(), digits / 4);
    }
    QCOMPARE(int(glyphs.size()), 4);
    QHash<int, QByteArray> body;
    for (const jp::QtPdf::Obj &obj : q.objects) body.insert(obj.id, obj.body);
    int table = -1;
    for (const jp::QtPdf::Obj &obj : q.objects)
        if (obj.body.contains("/S /Table")) table = obj.id;
    QVERIFY(table > 0);
    static const QRegularExpression ref(QStringLiteral("(\\d+) 0 R"));
    auto kids = [&](int id) {
        QVector<int> out;
        const QString k = QString::fromLatin1(body[id].mid(body[id].indexOf("/K [")));
        for (auto it = ref.globalMatch(k); it.hasNext();) out << it.next().captured(1).toInt();
        return out;
    };
    QVector<int> read;
    for (const int row : kids(table)) {
        QVERIFY(body[row].contains("/S /TR"));
        for (const int cell : kids(row)) {
            QVERIFY(body[cell].contains("/S /TD"));
            const auto k = QRegularExpression(QStringLiteral("/K (\\d+)\\b")).match(QString::fromLatin1(body[cell]));
            QVERIFY(k.hasMatch());
            read << glyphs.value(k.captured(1).toInt(), -1);
        }
    }
    QCOMPARE(read, (QVector<int>{1, 2, 3, 4}));
}


// The pages are put in a PDF file, whole or by a range, and a file that cannot be made is not left half made; the page ranges and the order of copies are read as they are typed.
void CrunchPrintTests::thePagesGoToAPdfFile()
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


// A sheet's page setup (paper, breaks, print area, titles) goes with the sheet through every sheet operation, and through Undo of each, in the model and in the saved file: the sheets
// keep their places in the session whatever the tabs do, and the names in the file say where each sheet is among the tabs and what it is called.
void CrunchPrintTests::pageSetupFollowsTheSheetOperations()
{
    const QByteArray one = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"),
                                     QStringLiteral("<pageSetup orientation=\"landscape\"/><rowBreaks count=\"1\" manualBreakCount=\"1\"><brk id=\"3\" max=\"16383\" man=\"1\"/></rowBreaks>"));
    const QByteArray two = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>2</v></c></row>"));
    const QByteArray three = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>3</v></c></row>"));
    EditWindow e;
    QVERIFY(e.open(buildXlsx({one, two, three}, QByteArray(), QString(), QString(), QByteArray(), QStringList({"A", "B", "C"}),
                             QStringLiteral("<definedName name=\"_xlnm.Print_Area\" localSheetId=\"1\">B!$A$1:$B$5</definedName><definedName name=\"_xlnm.Print_Titles\" localSheetId=\"2\">C!$1:$1</definedName>"))));
    crunch::EditSession *s = e.win.w.session();
    QVERIFY(e.settle());
    auto setup = [&](int sheet) { return s->pages().sheet(sheet); };
    QVERIFY(setup(0).orientation == jp::office::sheet::PageOrientation::Landscape);
    QCOMPARE(setup(0).rowBreaks.size(), 1);
    QVERIFY(setup(1).hasPrintArea());
    QVERIFY(setup(2).titles.hasRows);
    // a new sheet prints as the defaults say, and Undo takes it away (and Redo brings it back)
    int made = -1;
    QVERIFY(s->insertSheet(0, QStringLiteral("New"), &made).ok);
    QCOMPARE(made, 3);
    QCOMPARE(s->pages().sheetCount(), 4);
    QVERIFY(!setup(3).hasPrintArea() && setup(3).rowBreaks.isEmpty());
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    QCOMPARE(s->pages().sheetCount(), 3);
    e.win.w.act(QStringLiteral("edit.redo"))->trigger();
    QCOMPARE(s->pages().sheetCount(), 4);
    QCOMPARE(setup(1).printArea.size(), 1);   // (the others are where they were)
    // a copy prints as the sheet it is a copy of: its print area, its breaks, its paper
    int copied = -1;
    QVERIFY(s->copySheet(1, -1, QStringLiteral("B copy"), &copied).ok);
    QCOMPARE(copied, 4);
    QVERIFY(setup(4).hasPrintArea());
    QCOMPARE(setup(4).printArea[0].row2, 4u);
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    QCOMPARE(s->pages().sheetCount(), 4);
    e.win.w.act(QStringLiteral("edit.redo"))->trigger();
    QVERIFY(setup(4).hasPrintArea());
    // a rename, a move, and a delete keep the setups with their sheets (and Undo of each)
    QVERIFY(s->renameSheet(1, QStringLiteral("Renamed")).ok);
    QVERIFY(setup(1).hasPrintArea());
    QVERIFY(s->moveSheet(2, 0).ok);   // C first
    QVERIFY(setup(2).titles.hasRows);
    QVERIFY(s->deleteSheet(3).ok);    // the new sheet goes
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    QVERIFY(s->deleteSheet(3).ok);
    QVERIFY(setup(0).rowBreaks.size() == 1 && setup(1).hasPrintArea() && setup(2).titles.hasRows);
    // the saved file: tabs are C, A, New (deleted, so not there), Renamed, and the copy; each has its own setup
    QVERIFY(e.settle());
    QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
    QVERIFY(e.win.w.save());
    QVERIFY2(savedOk(saved), "the save failed");
    Package package;
    QString why;
    QVERIFY2(package.readLazyFile(e.path, &why), qPrintable(why));
    Workbook book;
    QVERIFY2(book.open(package, &why), qPrintable(why));
    QStringList names;
    for (const SheetEntry &entry : book.info().sheets) names << entry.name;
    QCOMPARE(names, QStringList({QStringLiteral("C"), QStringLiteral("A"), QStringLiteral("Renamed"), QStringLiteral("B copy")}));
    auto page = [&](int place) {
        Sheet sheet;
        book.readSheet(place, &sheet);
        return sheet.page;
    };
    QVERIFY(page(0).titles.hasRows);                       // C: its titles
    QCOMPARE(page(1).rowBreaks.size(), 1);                 // A: landscape, with its break
    QVERIFY(page(1).orientation == jp::office::sheet::PageOrientation::Landscape);
    QVERIFY(page(2).hasPrintArea() && page(2).printArea[0].row2 == 4u);   // B, renamed
    QVERIFY(page(3).hasPrintArea() && page(3).printArea[0].row2 == 4u);   // its copy
    QVERIFY(!page(0).hasPrintArea() && !page(1).hasPrintArea());
}


// An export tells how far it has got and can be cancelled (it says so, and leaves no file); copies of many pages are never made into a list of millions.
void CrunchPrintTests::anExportShowsProgressAndCanBeCancelled()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    const QString path = e.folder.dir.filePath(QStringLiteral("progress.pdf"));
    QList<QPair<int, int>> seen;
    e.win.w.setProgressHook([&](int done, int total) {
        seen << qMakePair(done, total);
        return true;
    });
    QString why;
    bool cancelled = true;
    QVERIFY2(e.win.w.exportPdfTo(path, crunch::PrintJobOptions(), QString(), &why, &cancelled), qPrintable(why));
    QVERIFY(!cancelled);
    QCOMPARE(seen, (QList<QPair<int, int>>{{1, 3}, {2, 3}, {3, 3}}));
    // cancelled after the first page: not a failure to write, no file, and the words say so
    QVERIFY(QFile::remove(path));
    e.win.w.setProgressHook([&](int done, int) { return done < 1; });
    QVERIFY(!e.win.w.exportPdfTo(path, crunch::PrintJobOptions(), QString(), &why, &cancelled));
    QVERIFY(cancelled);
    QVERIFY2(why.contains(QLatin1String("cancel")), qPrintable(why));
    QVERIFY(!why.contains(QLatin1String("could not")) && !why.contains(QLatin1String("couldn't")));
    QVERIFY(!QFileInfo::exists(path));
    // copies: the list is held to the most a job may send
    QVector<int> many(100000);
    for (int i = 0; i < many.size(); ++i) many[i] = i;
    QCOMPARE(qint64(crunch::sheetOrder(many, 100000, true).size()), qint64(crunch::kMaxSentPages));
    QVERIFY(qint64(crunch::sheetOrder(many, 100000, false).size()) <= crunch::kMaxSentPages);
}


// Printing tells how far it has got and can be stopped as an export can, and the copies are made on the print path itself: what is sent is counted in the PDF a printer writes, in the order
// of the papers the pages are on. Stopped, it leaves no file; the most a job sends is the same for a file and a printer.
void CrunchPrintTests::printingSendsTheCopiesAndCanBeStopped()
{
    EditWindow e;
    QVERIFY(e.open(threePapers()));
    QString why;
    crunch::PrintJobOptions all;
    all.what = crunch::PrintWhat::Workbook;
    const std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(all, &why);
    QVERIFY2(job, qPrintable(why));
    QCOMPARE(job->pageCount(), 3);
    const QString path = e.folder.dir.filePath(QStringLiteral("sent.pdf"));
    auto send = [&](const QVector<int> &pages, int copies, bool collate, const std::function<bool(int, int)> &progress = nullptr) {
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(path);
        return job->printTo(&printer, pages, &why, progress, copies, collate);
    };
    auto sentBoxes = [&] {
        QFile f(path);
        return f.open(QIODevice::ReadOnly) ? pdfBoxes(f.readAll()) : QStringList();
    };
    QVERIFY2(send({0, 1, 2}, 1, true), qPrintable(why));
    const QStringList once = sentBoxes();
    QCOMPARE(once.size(), 3);
    QVERIFY(once[0] != once[1] && once[1] != once[2] && once[0] != once[2]);
    QVERIFY2(send({0, 1, 2}, 2, true), qPrintable(why));
    QCOMPARE(sentBoxes(), (QStringList{once[0], once[1], once[2], once[0], once[1], once[2]}));
    QVERIFY2(send({0, 1, 2}, 2, false), qPrintable(why));
    QCOMPARE(sentBoxes(), (QStringList{once[0], once[0], once[1], once[1], once[2], once[2]}));
    QVERIFY2(send({2, 0}, 3, false), qPrintable(why));
    QCOMPARE(sentBoxes(), (QStringList{once[2], once[2], once[2], once[0], once[0], once[0]}));
    // it tells how far it has got, counting the copies
    QList<QPair<int, int>> seen;
    QVERIFY2(send({0, 1, 2}, 2, true, [&](int done, int total) {
                  seen << qMakePair(done, total);
                  return true;
              }), qPrintable(why));
    QCOMPARE(seen, (QList<QPair<int, int>>{{1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 6}}));
    QVERIFY(!job->wasCancelled());
    // stopped after the second sheet of paper: it says so, it is not a failure, and no file is left with the pages that went
    QVERIFY(!send({0, 1, 2}, 2, true, [](int done, int) { return done < 2; }));
    QVERIFY(job->wasCancelled());
    QVERIFY2(why.contains(QLatin1String("stopped")), qPrintable(why));
    QVERIFY(!QFileInfo::exists(path));
    // the most a job sends is held, copies counted, for a printer and for a file
    QVector<int> many(100000, 0);
    qint64 printerTotal = 0, fileTotal = 0;
    QVERIFY(!send(many, 100, true, [&](int, int total) {
        printerTotal = total;
        return false;
    }));
    QCOMPARE(printerTotal, crunch::kMaxSentPages);
    QVector<int> tooMany(int(crunch::kMaxSentPages) + 5, 0);
    QVERIFY(!job->exportPdf(path, tooMany, &why, [&](int, int total) {
        fileTotal = total;
        return false;
    }));
    QVERIFY(job->wasCancelled());
    QCOMPARE(fileTotal, crunch::kMaxSentPages);
    QVERIFY(!QFileInfo::exists(path));
}


// The date and time of the fields are one moment for the whole job (the options' if they give one, else the clock read once when it was made), not read again for each page: the
// header of every page shows the same, and a page drawn again too, though the clock has gone on.
void CrunchPrintTests::theMomentOfTheFieldsIsTakenOncePerJob()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    QString why;
    crunch::PrintJobOptions options;
    options.now = QDateTime::fromString(QStringLiteral("2026-03-05T14:07:09-05:00"), Qt::ISODate);
    std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(options, &why);
    QVERIFY2(job, qPrintable(why));
    QCOMPARE(job->moment(), options.now);
    const QDateTime before = QDateTime::currentDateTime();
    job = e.win.w.makePrintJob(crunch::PrintJobOptions(), &why);
    QVERIFY2(job, qPrintable(why));
    const QDateTime after = QDateTime::currentDateTime();
    QVERIFY(job->moment() >= before && job->moment() <= after);

    // a sheet of the same number in every cell, three pages, a date and a time in the header: the texts of its pages are the same if the header is
    QString rows;
    for (int r = 1; r <= 141; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>7</v></c></row>").arg(r);
    EditWindow same;
    QVERIFY(same.open(buildXlsx({worksheet(rows, QStringLiteral("<headerFooter><oddHeader>&amp;D &amp;T</oddHeader></headerFooter>"))})));
    int reads = 0;
    crunch::PrintJobOptions timed;
    timed.clock = [&reads] { return QDateTime(QDate(2026, 3, 5).addDays(reads++ * 40), QTime(14, 7, 9)); };
    job = same.win.w.makePrintJob(timed, &why);
    QVERIFY2(job, qPrintable(why));
    QVERIFY(job->pageCount() >= 3);
    QCOMPARE(reads, 1);
    auto textsOf = [&](crunch::PrintJob &j, int page) {
        TextDevice device;
        QPainter painter(&device);
        j.paint(&painter, page);
        painter.end();
        return device.texts();
    };
    const QStringList first = textsOf(*job, 0);
    QVERIFY(first.size() > 2);
    QCOMPARE(textsOf(*job, 1), first);
    QCOMPARE(textsOf(*job, 2), first);
    QCOMPARE(textsOf(*job, 0), first);
    QCOMPARE(reads, 1);
    // (and the header does show the moment: a job of another moment draws other words)
    timed.clock = nullptr;
    timed.now = job->moment().addDays(40);
    const std::unique_ptr<crunch::PrintJob> later = same.win.w.makePrintJob(timed, &why);
    QVERIFY2(later, qPrintable(why));
    QVERIFY(textsOf(*later, 0) != first);
}


// Page Layout > Background: the picture is tiled behind the cells in the grid, and a page printed (or seen as a picture, or made into a PDF) does not have it.
void CrunchPrintTests::aBackgroundIsShownOnTheScreenAndNeverPrinted()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(10, {})));
    CrunchWindow &w = e.win.w;
    QVERIFY(e.settle());
    QTemporaryDir dir;
    QImage tile(2, 2, QImage::Format_RGB32);
    tile.fill(QColor(0, 0xC0, 0));   // (a green that nothing else in the sheet is)
    const QString path = dir.filePath(QStringLiteral("tile.png"));
    QVERIFY(tile.save(path));
    // the print as it is before a background: the same after it (the picture is on the screen only)
    QString why;
    const std::unique_ptr<crunch::PrintJob> plainJob = w.makePrintJob(crunch::PrintJobOptions(), &why);
    QVERIFY2(plainJob, qPrintable(why));
    const QImage plainPage = plainJob->preview(0, 612);
    w.setPictureAsker([path] { return path; });
    QVERIFY(w.act(QStringLiteral("page.background"))->isEnabled());
    QVERIFY(!w.act(QStringLiteral("page.deleteBackground"))->isEnabled());   // (nothing to delete yet)
    QVERIFY(!w.view()->hasBackground());
    w.act(QStringLiteral("page.background"))->trigger();
    QVERIFY(e.settle());
    QVERIFY(w.view()->hasBackground());
    QVERIFY(w.act(QStringLiteral("page.deleteBackground"))->isEnabled());
    // on the screen: a cell with nothing in it shows the picture, and a cell with a fill hides it
    // (the view is drawn into a picture of its own, a pixel to a pixel, so that this holds on any screen and with no window shown)
    w.resize(900, 600);
    w.view()->resize(800, 500);
    QImage screen(w.view()->viewport()->size(), QImage::Format_RGB32);
    screen.setDevicePixelRatio(1);
    screen.fill(Qt::white);
    w.view()->viewport()->render(&screen);
    QVERIFY(!screen.isNull() && screen.width() > 100);
    int green = 0;
    for (int y = 0; y < screen.height(); ++y)
        for (int x = 0; x < screen.width(); ++x) green += screen.pixel(x, y) == QColor(0, 0xC0, 0).rgb();
    QVERIFY2(green > screen.width() * screen.height() / 4, qPrintable(QString::number(green)));
    // printed: none of it
    const std::unique_ptr<crunch::PrintJob> job = w.makePrintJob(crunch::PrintJobOptions(), &why);
    QVERIFY2(job, qPrintable(why));
    const QImage page = job->preview(0, 612);
    QVERIFY(page == plainPage);   // (pixel for pixel the page that was made with no background)
    for (int y = 0; y < page.height(); ++y)
        for (int x = 0; x < page.width(); ++x) QVERIFY(page.pixel(x, y) != QColor(0, 0xC0, 0).rgb());
    if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
        QImage big(64, 64, QImage::Format_RGB32);
        QPainter p(&big);
        for (int y = 0; y < 64; y += 8)
            for (int x = 0; x < 64; x += 8) p.fillRect(x, y, 8, 8, ((x + y) / 8) % 2 ? QColor(0xF2, 0xC2, 0x4E) : QColor(0xFF, 0xF1, 0xC4));
        p.end();
        const QString shot = dir.filePath(QStringLiteral("pattern.png"));
        QVERIFY(big.save(shot));
        w.setPictureAsker([shot] { return shot; });
        w.act(QStringLiteral("page.background"))->trigger();
        QVERIFY(e.settle());
        QTest::qWait(200);
        QVERIFY(w.grab().save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/sheet-background.png")));
    }
}

// The step is one Undo (and Redo), Delete Background takes it away, and the file keeps it: saved, and opened again.
void CrunchPrintTests::aBackgroundIsOneStepOfUndoAndSavedWithTheFile()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(10, {})));
    CrunchWindow &w = e.win.w;
    QVERIFY(e.settle());
    QTemporaryDir dir;
    QImage tile(4, 4, QImage::Format_RGB32);
    tile.fill(Qt::blue);
    const QString path = dir.filePath(QStringLiteral("tile.png"));
    QVERIFY(tile.save(path));
    w.setPictureAsker([path] { return path; });
    const int steps = w.session()->undoStack()->count();
    w.act(QStringLiteral("page.background"))->trigger();
    QVERIFY(e.settle());
    QCOMPARE(w.session()->undoStack()->count(), steps + 1);
    QVERIFY(w.session()->pages().sheet(0).background.extension == QLatin1String("png"));
    w.session()->undoStack()->undo();
    QVERIFY(!w.view()->hasBackground());
    w.session()->undoStack()->redo();
    QVERIFY(w.view()->hasBackground());
    // saved and opened again
    QVERIFY(e.settle());
    {
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), "the save failed");
    }
    EditWindow again;
    QFile f(e.path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QVERIFY(again.open(f.readAll(), QStringLiteral("again.xlsx")));
    QVERIFY(again.settle());
    QVERIFY(again.win.w.view()->hasBackground());
    QVERIFY(!again.win.w.session()->pages().sheet(0).background.isEmpty());
    // Delete Background: one step, the picture gone, and gone from the file
    QVERIFY(again.win.w.act(QStringLiteral("page.deleteBackground"))->isEnabled());
    const int before = again.win.w.session()->undoStack()->count();
    again.win.w.act(QStringLiteral("page.deleteBackground"))->trigger();
    QCOMPARE(again.win.w.session()->undoStack()->count(), before + 1);
    QVERIFY(!again.win.w.view()->hasBackground());
    QVERIFY(!again.win.w.act(QStringLiteral("page.deleteBackground"))->isEnabled());
    QVERIFY(again.settle());
    {
        QSignalSpy saved(&again.win.w, &CrunchWindow::saved);
        QVERIFY(again.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
    }
    EditWindow third;
    QFile g(again.path);
    QVERIFY(g.open(QIODevice::ReadOnly));
    QVERIFY(third.open(g.readAll(), QStringLiteral("third.xlsx")));
    QVERIFY(third.settle());
    QVERIFY(!third.win.w.view()->hasBackground());
}

// A file is read back with the pictures of its backgrounds, headers, and footers held to one total: the ninth big background is refused when it is chosen, with a reason, and no step is made.
void CrunchPrintTests::picturesOverTheTotalAFileKeepsAreRefusedWhenChosen()
{
    QVector<QByteArray> sheets;
    for (int i = 0; i < 9; ++i) sheets.append(worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>")));
    EditWindow e;
    QVERIFY(e.open(buildXlsx(sheets)));
    CrunchWindow &w = e.win.w;
    QVERIFY(e.settle());
    const QByteArray fake = QByteArray("\x89PNG\r\n\x1a\n", 8) + QByteArray(8300000, 'x');   // (eight of these are 66 MB: under the total, which is 64 MiB)
    for (int i = 0; i < 8; ++i) {
        PageSetup p = w.session()->pages().sheet(i);
        p.background.image = fake;
        p.background.extension = QStringLiteral("png");
        QVERIFY(w.session()->setPageSetup(i, p, QStringLiteral("Background")).ok);
    }
    QTemporaryDir dir;
    QImage noise(700, 700, QImage::Format_RGB32);
    for (int y = 0; y < noise.height(); ++y)
        for (int x = 0; x < noise.width(); ++x) noise.setPixel(x, y, qRgb(int(QRandomGenerator::global()->bounded(256)), int(QRandomGenerator::global()->bounded(256)), int(QRandomGenerator::global()->bounded(256))));
    const QString path = dir.filePath(QStringLiteral("noise.png"));
    QVERIFY(noise.save(path));
    QVERIFY(QFileInfo(path).size() > 800000);
    QString told;
    w.setNotifier([&](const QString &, const QString &text) { told = text; });
    w.setPictureAsker([path] { return path; });
    w.setActiveSheet(8);
    QVERIFY(e.settle());
    const int steps = w.session()->undoStack()->index();
    w.act(QStringLiteral("page.background"))->trigger();
    QCOMPARE(w.session()->undoStack()->index(), steps);
    QVERIFY(told.contains(QStringLiteral("larger than a file can keep")));
    QVERIFY(w.session()->pages().sheet(8).background.isEmpty());
    // replacing a big one with this one is fine: its own bytes are not counted twice
    w.setActiveSheet(0);
    QVERIFY(e.settle());
    w.act(QStringLiteral("page.background"))->trigger();
    QCOMPARE(w.session()->undoStack()->index(), steps + 1);
}

// A picture of more than 2,400 pixels on a side is held smaller and tiled at the size it really is.
void CrunchPrintTests::aLargePictureIsTiledAtItsRealSize()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(10, {})));
    CrunchWindow &w = e.win.w;
    QVERIFY(e.settle());
    QTemporaryDir dir;
    QImage wide(4800, 40, QImage::Format_RGB32);
    wide.fill(Qt::darkCyan);
    const QString path = dir.filePath(QStringLiteral("wide.png"));
    QVERIFY(wide.save(path));
    w.setPictureAsker([path] { return path; });
    w.act(QStringLiteral("page.background"))->trigger();
    QVERIFY(e.settle());
    QVERIFY(w.view()->hasBackground());
    QCOMPARE(w.view()->backgroundScale(), 2.0);   // (held at 2,400 pixels, so tiled twice as large)
    QImage small(10, 10, QImage::Format_RGB32);
    small.fill(Qt::red);
    const QString smallPath = dir.filePath(QStringLiteral("small.png"));
    QVERIFY(small.save(smallPath));
    w.setPictureAsker([smallPath] { return smallPath; });
    w.act(QStringLiteral("page.background"))->trigger();
    QVERIFY(e.settle());
    QCOMPARE(w.view()->backgroundScale(), 1.0);
}

// The sheets this window did not change count too, as the file has their pictures: a workbook opened from a file that holds 66 MB of backgrounds takes no more on another sheet, by the Background
// command or by a picture in a header (the Page Setup dialog's, which is told what the rest of the workbook holds).
void CrunchPrintTests::picturesOfSheetsNotChangedCountWhenAPictureIsChosen()
{
    QVector<QByteArray> sheets;
    for (int i = 0; i < 9; ++i) sheets.append(worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>")));
    QByteArray file;
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx(sheets)));
        QVERIFY(e.settle());
        const QByteArray fake = QByteArray("\x89PNG\r\n\x1a\n", 8) + QByteArray(8300000, 'x');
        for (int i = 0; i < 8; ++i) {
            PageSetup p = e.win.w.session()->pages().sheet(i);
            p.background.image = fake;
            p.background.extension = QStringLiteral("png");
            QVERIFY(e.win.w.session()->setPageSetup(i, p, QStringLiteral("Background")).ok);
        }
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        file = f.readAll();
    }
    EditWindow e;
    QVERIFY(e.open(file, QStringLiteral("held.xlsx")));
    CrunchWindow &w = e.win.w;
    QVERIFY(e.settle());
    QVERIFY(!w.session()->pages().sheet(0).background.isEmpty());   // (read back from the file)
    QTemporaryDir dir;
    QImage noise(700, 700, QImage::Format_RGB32);
    for (int y = 0; y < noise.height(); ++y)
        for (int x = 0; x < noise.width(); ++x) noise.setPixel(x, y, qRgb(int(QRandomGenerator::global()->bounded(256)), int(QRandomGenerator::global()->bounded(256)), int(QRandomGenerator::global()->bounded(256))));
    const QString path = dir.filePath(QStringLiteral("noise.png"));
    QVERIFY(noise.save(path));
    QString told;
    w.setNotifier([&](const QString &, const QString &text) { told = text; });
    w.setPictureAsker([path] { return path; });
    w.setActiveSheet(8);
    QVERIFY(e.settle());
    const int steps = w.session()->undoStack()->index();
    w.act(QStringLiteral("page.background"))->trigger();
    QCOMPARE(w.session()->undoStack()->index(), steps);
    QVERIFY(told.contains(QStringLiteral("larger than a file can keep")));
    // the header editor of the Page Setup dialog is told what the rest of the workbook holds, and refuses the same picture
    PageSetupRequest request;
    request.setup = w.session()->pages().sheet(8);
    request.otherPictureBytes = 66400000;
    HeaderFooterEditor editor(true, request.setup.headerFooter, QStringLiteral("S"), QStringLiteral("f"));
    editor.setOtherPictureBytes(request.otherPictureBytes);
    editor.setPictureAsker([path] { return path; });
    editor.findChild<QPushButton *>(QStringLiteral("hfPicture"))->click();
    QVERIFY(editor.texts().pictures.isEmpty());
    QVERIFY(editor.findChild<QLabel *>(QStringLiteral("hfPictureState"))->text().contains(QStringLiteral("larger than a file can keep")));
    // with nothing else held it is taken
    HeaderFooterEditor free(true, request.setup.headerFooter, QStringLiteral("S"), QStringLiteral("f"));
    free.setPictureAsker([path] { return path; });
    free.findChild<QPushButton *>(QStringLiteral("hfPicture"))->click();
    QCOMPARE(free.texts().pictures.size(), 1);
}

// A file that is not a picture, a type that is not allowed, a picture too large: told, and the sheet is as it was.
void CrunchPrintTests::aBadBackgroundPictureIsRefusedWithAReason()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(10, {})));
    CrunchWindow &w = e.win.w;
    QVERIFY(e.settle());
    QTemporaryDir dir;
    QFile notPicture(dir.filePath(QStringLiteral("text.png")));
    QVERIFY(notPicture.open(QIODevice::WriteOnly));
    notPicture.write("this is not a picture");
    notPicture.close();
    jp::office::sheet::SheetBackground bg;
    QString why;
    QVERIFY(!loadBackgroundPicture(notPicture.fileName(), &bg, &why));
    QVERIFY(!why.isEmpty());
    QVERIFY(!loadBackgroundPicture(dir.filePath(QStringLiteral("missing.png")), &bg, &why));
    // over the bytes every picture is held to
    QFile big(dir.filePath(QStringLiteral("big.bmp")));
    QVERIFY(big.open(QIODevice::WriteOnly));
    big.write(QByteArray(int(jp::office::sheet::kMaxHeaderPictureBytes) + 1, 'x'));
    big.close();
    QVERIFY(!loadBackgroundPicture(big.fileName(), &bg, &why));
    QVERIFY(why.contains(QStringLiteral("too large")));
    // more pixels than every picture is held to: a header that says 20000 x 20000 pixels (the file is a few bytes, and none of it is decoded)
    {
        auto be32 = [](quint32 v) { return QByteArray(1, char(v >> 24)) + QByteArray(1, char(v >> 16)) + QByteArray(1, char(v >> 8)) + QByteArray(1, char(v)); };
        auto crc32 = [](const QByteArray &data) {
            quint32 crc = 0xFFFFFFFFu;
            for (const char ch : data) {
                crc ^= quint8(ch);
                for (int k = 0; k < 8; ++k) crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
            }
            return ~crc;
        };
        const QByteArray ihdrBody = QByteArray("IHDR") + be32(20000) + be32(20000) + QByteArray("\x01\x00\x00\x00\x00", 5);
        QByteArray png = QByteArray("\x89PNG\r\n\x1a\n", 8) + be32(13) + ihdrBody + be32(crc32(ihdrBody));
        const QByteArray dataBody = QByteArray("IDAT");   // (no data in it: the reader is only asked how large the picture says it is)
        png += be32(0) + dataBody + be32(crc32(dataBody));
        const QByteArray endBody = QByteArray("IEND");
        png += be32(0) + endBody + be32(crc32(endBody));
        QFile hugeFile(dir.filePath(QStringLiteral("huge.png")));
        QVERIFY(hugeFile.open(QIODevice::WriteOnly));
        hugeFile.write(png);
        hugeFile.close();
        QVERIFY(hugeFile.size() < 100);
        QBuffer probe(&png);
        QVERIFY(probe.open(QIODevice::ReadOnly));
        QCOMPARE(QImageReader(&probe).size(), QSize(20000, 20000));   // (the file says what it says)
        QVERIFY(!loadBackgroundPicture(hugeFile.fileName(), &bg, &why));
        QVERIFY(why.contains(QStringLiteral("too large")));
    }
    // through the command: a notice, and no step
    w.setPictureAsker([p = notPicture.fileName()] { return p; });
    const int steps = w.session()->undoStack()->count();
    w.act(QStringLiteral("page.background"))->trigger();
    QCOMPARE(w.session()->undoStack()->count(), steps);
    QVERIFY(!w.view()->hasBackground());
}

// A page as a picture: the width asked for, the height of the paper's shape, and ink on it.
void CrunchPrintTests::aPageCanBeSeenAsAPicture()
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
    // a picture is held to its budget of pixels whatever size is asked for (six times the zoom on a large screen is hundreds of megabytes), and keeps the page's shape
    const QImage big = job->preview(0, 20000);
    QVERIFY(!big.isNull());
    QVERIFY(qint64(big.width()) * big.height() <= crunch::kMaxPreviewPixels);
    QVERIFY(big.width() < 20000);
    QVERIFY(std::abs(double(big.height()) / big.width() - 792.0 / 612.0) < 0.01);
}


// ---- the Page Setup dialog

// The dialog shows what the setup says, in four tabs; one that is not changed gives the setup back exactly, whatever it holds that the dialog does not show.
void CrunchPrintTests::theDialogShowsTheSetupAndGivesItBackUnchanged()
{
    PageSetup p;
    p.orientation = PageOrientation::Landscape;
    p.paperSize = 9;
    p.scale = 85;
    p.margins = {0.5, 0.6, 0.7, 0.8, 0.333, 0.25};   // (more places than the boxes show: they come back as they were)
    p.centerHorizontally = true;
    p.gridLines = true;
    p.comments = jp::office::sheet::PrintComments::AtEnd;
    p.errors = jp::office::sheet::PrintErrors::Dash;
    p.pageOrder = jp::office::sheet::PageOrder::OverThenDown;
    p.headerFooter.oddHeader = QStringLiteral("&LMy &\"Arial,Bold\"title&R&D");
    p.headerFooter.firstFooter = QStringLiteral("&Cfirst");
    p.headerFooter.differentFirst = true;
    p.paperWidth = 300;   // (a size of its own that the dialog does not show)
    p = withArea(p, QStringLiteral("A1:D20"));
    p.titles.hasRows = true;
    p.titles.row1 = 0;
    p.titles.row2 = 1;
    jp::office::sheet::PageBreak br;
    br.before = 10;
    br.max = 16383;
    p.rowBreaks.append(br);
    PageSetupRequest request;
    request.setup = p;
    request.sheetName = QStringLiteral("Q3 Sales");
    request.fileName = QStringLiteral("Budget.xlsx");
    PageSetupDialog dialog(request);
    QVERIFY(dialog.findChild<QRadioButton *>(QStringLiteral("psLandscape"))->isChecked());
    QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("psScale"))->value(), 85);
    QCOMPARE(dialog.findChild<QDoubleSpinBox *>(QStringLiteral("psMarginLeft"))->value(), 0.5);
    QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("psArea"))->text(), QStringLiteral("A1:D20"));
    QCOMPARE(dialog.findChild<QLineEdit *>(QStringLiteral("psTitleRows"))->text(), QStringLiteral("1:2"));
    QVERIFY(dialog.findChild<QCheckBox *>(QStringLiteral("psGrid"))->isChecked());
    QVERIFY(dialog.findChild<QRadioButton *>(QStringLiteral("psAcross"))->isChecked());
    QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("psPaper"))->currentText(), QStringLiteral("A4"));
    QCOMPARE(dialog.tabs()->count(), 4);
    QVERIFY(dialog.tabs()->tabText(2).contains(QStringLiteral("Header")));
    dialog.accept();
    QCOMPARE(dialog.result(), int(QDialog::Accepted));
    QVERIFY(dialog.setup() == p);
    // it opens on the tab asked for
    request.tab = 3;
    PageSetupDialog onSheet(request);
    QCOMPARE(onSheet.tabs()->currentIndex(), 3);
}


// The Page tab lists the print quality, shows the file's, and gives back a change.
void CrunchPrintTests::thePageTabHasThePrintQuality()
{
    PageSetupRequest request;
    request.setup = PageSetup();
    request.setup.horizontalDpi = request.setup.verticalDpi = 300;
    PageSetupDialog d(request);
    QComboBox *quality = d.findChild<QComboBox *>(QStringLiteral("psQuality"));
    QVERIFY(quality);
    QCOMPARE(quality->currentData().toInt(), 300);
    quality->setCurrentIndex(quality->findData(1200));
    d.accept();
    QCOMPARE(d.setup().horizontalDpi, 1200);
    QCOMPARE(d.setup().verticalDpi, 1200);
    // a quality the list does not have is kept and listed, and not changed by an Ok
    request.setup.horizontalDpi = request.setup.verticalDpi = 360;
    PageSetupDialog other(request);
    QCOMPARE(other.findChild<QComboBox *>(QStringLiteral("psQuality"))->currentData().toInt(), 360);
    other.accept();
    QCOMPARE(other.setup().horizontalDpi, 360);
}


// What the person changes goes back, and only that: the orientation, the scale or the pages to fit, the paper, the margins, centering, the print area and titles, the options, and the order.
void CrunchPrintTests::changesInTheDialogGoBack()
{
    PageSetupRequest request;
    request.setup = PageSetup();
    request.sheetName = QStringLiteral("Data");
    PageSetupDialog d(request);
    d.findChild<QRadioButton *>(QStringLiteral("psLandscape"))->setChecked(true);
    d.findChild<QRadioButton *>(QStringLiteral("psFitRadio"))->setChecked(true);
    d.findChild<QSpinBox *>(QStringLiteral("psFitAcross"))->setValue(2);
    d.findChild<QSpinBox *>(QStringLiteral("psFitDown"))->setValue(0);
    d.findChild<QComboBox *>(QStringLiteral("psPaper"))->setCurrentIndex(d.findChild<QComboBox *>(QStringLiteral("psPaper"))->findData(9));
    d.findChild<QCheckBox *>(QStringLiteral("psUseFirst"))->setChecked(true);
    d.findChild<QSpinBox *>(QStringLiteral("psFirstNumber"))->setValue(5);
    d.findChild<QDoubleSpinBox *>(QStringLiteral("psMarginTop"))->setValue(1.25);
    d.findChild<QDoubleSpinBox *>(QStringLiteral("psMarginFooter"))->setValue(0.5);
    d.findChild<QCheckBox *>(QStringLiteral("psCenterAcross"))->setChecked(true);
    d.findChild<QCheckBox *>(QStringLiteral("psCenterDown"))->setChecked(true);
    d.findChild<QLineEdit *>(QStringLiteral("psArea"))->setText(QStringLiteral("A1:D20, $F$1:$G$9"));
    d.findChild<QLineEdit *>(QStringLiteral("psTitleRows"))->setText(QStringLiteral("$1:$3"));
    d.findChild<QLineEdit *>(QStringLiteral("psTitleCols"))->setText(QStringLiteral("A"));
    d.findChild<QCheckBox *>(QStringLiteral("psGrid"))->setChecked(true);
    d.findChild<QCheckBox *>(QStringLiteral("psHeadings"))->setChecked(true);
    d.findChild<QCheckBox *>(QStringLiteral("psBlackWhite"))->setChecked(true);
    d.findChild<QCheckBox *>(QStringLiteral("psDraft"))->setChecked(true);
    d.findChild<QComboBox *>(QStringLiteral("psNotes"))->setCurrentIndex(1);
    d.findChild<QComboBox *>(QStringLiteral("psErrors"))->setCurrentIndex(3);
    d.findChild<QRadioButton *>(QStringLiteral("psAcross"))->setChecked(true);
    d.findChild<QCheckBox *>(QStringLiteral("psDifferentFirst"))->setChecked(true);
    d.findChild<QCheckBox *>(QStringLiteral("psScaleWith"))->setChecked(false);
    d.accept();
    QCOMPARE(d.result(), int(QDialog::Accepted));
    const PageSetup s = d.setup();
    QVERIFY(s.orientation == PageOrientation::Landscape);
    QVERIFY(s.fitToPage);
    QCOMPARE(s.fitToWidth, 2);
    QCOMPARE(s.fitToHeight, 0);
    QCOMPARE(s.paperSize, 9);
    QVERIFY(s.useFirstPageNumber);
    QCOMPARE(s.firstPageNumber, 5);
    QCOMPARE(s.margins.top, 1.25);
    QCOMPARE(s.margins.footer, 0.5);
    QCOMPARE(s.margins.left, 0.7);   // (as it was)
    QVERIFY(s.centerHorizontally && s.centerVertically);
    QCOMPARE(s.printArea.size(), 2);
    QCOMPARE(areaOf(s), QStringLiteral("A1:D20 F1:G9"));
    QVERIFY(s.titles.hasRows && s.titles.hasCols);
    QCOMPARE(s.titles.row2, 2u);
    QCOMPARE(s.titles.col1, 0u);
    QCOMPARE(s.titles.col2, 0u);
    QVERIFY(s.gridLines && s.headings && s.blackAndWhite && s.draft);
    QVERIFY(s.comments == jp::office::sheet::PrintComments::AtEnd);
    QVERIFY(s.errors == jp::office::sheet::PrintErrors::NA);
    QVERIFY(s.pageOrder == jp::office::sheet::PageOrder::OverThenDown);
    QVERIFY(s.headerFooter.differentFirst);
    QVERIFY(!s.headerFooter.scaleWithDoc);
    // the scale is used when the cells are not shrunk to fit
    PageSetupDialog scaled(request);
    scaled.findChild<QSpinBox *>(QStringLiteral("psScale"))->setValue(60);
    QCOMPARE(scaled.setup().scale, 60);
    QVERIFY(!scaled.setup().fitToPage);
}


// Margins in centimeters: the boxes show them so, and what is put in is kept as inches; an unchanged one is not rounded.
void CrunchPrintTests::marginsCanBeInCentimeters()
{
    PageSetupRequest request;
    request.setup.margins.left = 0.7;
    request.metric = true;
    PageSetupDialog d(request);
    QCOMPARE(d.findChild<QDoubleSpinBox *>(QStringLiteral("psMarginLeft"))->value(), 1.78);
    QVERIFY(d.setup().margins.left == 0.7);
    d.findChild<QDoubleSpinBox *>(QStringLiteral("psMarginTop"))->setValue(2.54);
    QCOMPARE(d.setup().margins.top, 1.0);
}


// A print area or titles that are not cells are refused where they are typed, with a reason; the dialog stays.
void CrunchPrintTests::aPrintAreaThatIsNotCellsIsRefused()
{
    PageSetupRequest request;
    PageSetupDialog d(request);
    d.show();
    d.findChild<QLineEdit *>(QStringLiteral("psArea"))->setText(QStringLiteral("A1:D20, banana"));
    d.accept();
    QVERIFY(d.result() != int(QDialog::Accepted));
    QVERIFY(d.findChild<QLabel *>(QStringLiteral("psProblem"))->isVisible());
    QVERIFY(d.findChild<QLabel *>(QStringLiteral("psProblem"))->text().contains(QStringLiteral("banana")));
    QCOMPARE(d.tabs()->currentIndex(), 3);
    d.findChild<QLineEdit *>(QStringLiteral("psArea"))->setText(QStringLiteral("A1:D20"));
    d.findChild<QLineEdit *>(QStringLiteral("psTitleRows"))->setText(QStringLiteral("A:B"));
    d.accept();
    QVERIFY(d.result() != int(QDialog::Accepted));
    d.findChild<QLineEdit *>(QStringLiteral("psTitleRows"))->setText(QStringLiteral("2"));
    d.accept();
    QCOMPARE(d.result(), int(QDialog::Accepted));
    QCOMPARE(d.setup().titles.row1, 1u);
    QCOMPARE(d.setup().titles.row2, 1u);
}


// An area or titles typed with the sheet's name in front (as the names of the workbook write them) are taken; another sheet's are not this sheet's.
void CrunchPrintTests::anAreaTypedWithTheSheetsNameIsTaken()
{
    PageSetupRequest request;
    request.sheetName = QStringLiteral("My Sheet");
    PageSetupDialog d(request);
    d.show();
    d.findChild<QLineEdit *>(QStringLiteral("psArea"))->setText(QStringLiteral("'My Sheet'!$A$1:$D$20, 'My Sheet'!F1:G9"));
    d.findChild<QLineEdit *>(QStringLiteral("psTitleRows"))->setText(QStringLiteral("'My Sheet'!$1:$2"));
    d.accept();
    QCOMPARE(d.result(), int(QDialog::Accepted));
    QCOMPARE(d.setup().printArea.size(), 2);
    QCOMPARE(d.setup().printArea[0].row2, 19u);
    QCOMPARE(d.setup().titles.row2, 1u);
    PageSetupDialog other(request);
    other.show();
    other.findChild<QLineEdit *>(QStringLiteral("psArea"))->setText(QStringLiteral("Elsewhere!A1:D20"));
    other.accept();
    QVERIFY(other.result() != int(QDialog::Accepted));
}


// The header and footer lists are made from the sheet's name, the file, the page, and the date; choosing one sets the text, and a text of its own is kept as it is.
void CrunchPrintTests::theHeaderAndFooterPresetsAreMadeFromTheSheet()
{
    const auto presets = PageSetupDialog::headerFooterPresets(QStringLiteral("Q3 Sales"), QStringLiteral("Budget.xlsx"));
    QVERIFY(presets.size() >= 6);
    QCOMPARE(presets[0].second, QString());
    bool sheetName = false, pageOf = false;
    for (const auto &p : presets) {
        sheetName = sheetName || p.first == QStringLiteral("Q3 Sales");
        pageOf = pageOf || p.second == QStringLiteral("&CPage &P of &N");
    }
    QVERIFY(sheetName);
    QVERIFY(pageOf);
    PageSetupRequest request;
    request.sheetName = QStringLiteral("Q3 Sales");
    request.setup.headerFooter.oddFooter = QStringLiteral("&CMy own &P");
    PageSetupDialog d(request);
    QComboBox *header = d.findChild<QComboBox *>(QStringLiteral("psHeaderPreset")), *footer = d.findChild<QComboBox *>(QStringLiteral("psFooterPreset"));
    QCOMPARE(header->currentIndex(), 0);
    QCOMPARE(footer->currentData().toString(), QStringLiteral("&CMy own &P"));   // (a text of its own is an item of its own)
    const int sheetItem = header->findData(QStringLiteral("&C&A"));
    QVERIFY(sheetItem > 0);
    Q_EMIT header->activated(sheetItem);
    QCOMPARE(d.setup().headerFooter.oddHeader, QStringLiteral("&C&A"));
    QCOMPARE(d.setup().headerFooter.oddFooter, QStringLiteral("&CMy own &P"));
    QVERIFY(d.findChild<QLabel *>(QStringLiteral("psHeaderShown"))->text().contains(QStringLiteral("Q3 Sales")));
    // (header and footer text comes from the file: never read as markup, which could name a picture to fetch)
    for (const char *name : {"psHeaderShown", "psFooterShown", "hfPreview"})
        if (auto *label = d.findChild<QLabel *>(QLatin1String(name))) QCOMPARE(label->textFormat(), Qt::PlainText);
    // the editor: the three boxes, the buttons that put things in, and a version for each kind of page
    jp::office::sheet::HeaderFooter hf;
    hf.differentFirst = true;
    hf.oddHeader = QStringLiteral("&Lleft&Cmiddle");
    HeaderFooterEditor editor(true, hf, QStringLiteral("Q3 Sales"), QStringLiteral("Budget.xlsx"));
    QCOMPARE(editor.versionNames().size(), 2);
    QCOMPARE(editor.box(0)->toPlainText(), QStringLiteral("left"));
    QCOMPARE(editor.box(1)->toPlainText(), QStringLiteral("middle"));
    editor.setInsertPart(2);
    editor.findChild<QPushButton *>(QStringLiteral("hfPageNumber"))->click();
    QVERIFY(editor.box(2)->toPlainText().contains(QStringLiteral("&P")));
    editor.selectVersion(1);
    editor.box(1)->setPlainText(QStringLiteral("only the first"));
    const jp::office::sheet::HeaderFooter made = editor.texts();
    QCOMPARE(made.oddHeader, QStringLiteral("&Lleft&Cmiddle&R&P"));
    QCOMPARE(made.firstHeader, QStringLiteral("&Conly the first"));
    // flags that make versions: odd and even
    hf.differentOddEven = true;
    HeaderFooterEditor three(false, hf, QStringLiteral("S"), QStringLiteral("f"));
    QCOMPARE(three.versionNames().size(), 3);
}


// ---- the commands of the Page Layout tab

// The tab is on the ribbon and its commands are there; they work when the workbook is open (and are off while it is not).
void CrunchPrintTests::thePageLayoutTabAndItsCommandsAreThere()
{
    {
        CrunchWindow bare;
        QCOMPARE(bare.ribbonErrors(), QString());
        bool tab = false;
        for (int i = 0; i < bare.ribbon()->tabCount(); ++i) tab = tab || bare.ribbon()->tabName(i) == QStringLiteral("Page Layout");
        QVERIFY(tab);
        QVERIFY(!bare.act(QStringLiteral("orient.landscape"))->isEnabled());
    }
    EditWindow e;
    QVERIFY(e.open(printBook()));
    for (const char *id : {"pagesetup.dialog", "margins.standard", "margins.wide", "margins.narrow", "margins.custom", "orient.portrait", "orient.landscape", "size.1", "size.9", "size.more",
                           "printarea.set", "printarea.add", "breaks.insert", "breaks.remove", "breaks.reset", "titles.dialog", "sheetopt.gridPrint", "sheetopt.headPrint", "sheetopt.gridView",
                           "sheetopt.headView", "file.print"}) {
        QVERIFY2(e.win.w.act(QString::fromLatin1(id)), id);
        QVERIFY2(e.win.w.act(QString::fromLatin1(id))->isEnabled(), id);
    }
    QVERIFY(!e.win.w.act(QStringLiteral("printarea.clear"))->isEnabled());   // (there is no print area to clear)
}


void CrunchPrintTests::marginsOrientationAndPaperChangeTheSheetAndShowWhatItHas()
{
    EditWindow e;
    QVERIFY(e.open(printBook()));
    crunch::EditSession *session = e.win.w.session();
    CrunchWindow &w = e.win.w;
    QVERIFY(w.act(QStringLiteral("margins.standard"))->isChecked());   // (the file says the usual)
    QVERIFY(w.act(QStringLiteral("orient.portrait"))->isChecked());
    QVERIFY(w.act(QStringLiteral("size.1"))->isChecked());
    w.act(QStringLiteral("margins.wide"))->trigger();
    QCOMPARE(session->pages().sheet(0).margins.left, 1.0);
    QCOMPARE(session->pages().sheet(0).margins.header, 0.5);
    QVERIFY(w.act(QStringLiteral("margins.wide"))->isChecked());
    QVERIFY(!w.act(QStringLiteral("margins.standard"))->isChecked());
    w.act(QStringLiteral("margins.narrow"))->trigger();
    QCOMPARE(session->pages().sheet(0).margins.left, 0.25);
    w.act(QStringLiteral("orient.landscape"))->trigger();
    QVERIFY(session->pages().sheet(0).orientation == PageOrientation::Landscape);
    QVERIFY(w.act(QStringLiteral("orient.landscape"))->isChecked());
    QVERIFY(!w.act(QStringLiteral("orient.portrait"))->isChecked());
    w.act(QStringLiteral("size.9"))->trigger();
    QCOMPARE(session->pages().sheet(0).paperSize, 9);
    QVERIFY(w.act(QStringLiteral("size.9"))->isChecked());
    QVERIFY(!w.act(QStringLiteral("size.1"))->isChecked());
    // each is a step; Undo shows the check marks of what it brings back
    QCOMPARE(w.undoStack()->count(), 4);
    w.act(QStringLiteral("edit.undo"))->trigger();
    QVERIFY(w.act(QStringLiteral("size.1"))->isChecked());
    w.act(QStringLiteral("edit.undo"))->trigger();
    QVERIFY(w.act(QStringLiteral("orient.portrait"))->isChecked());
    // another sheet has its own
    w.setActiveSheet(1);
    QVERIFY(w.act(QStringLiteral("orient.portrait"))->isChecked());
    w.act(QStringLiteral("orient.landscape"))->trigger();
    QVERIFY(session->pages().sheet(1).orientation == PageOrientation::Landscape);
    QVERIFY(session->pages().sheet(0).orientation == PageOrientation::Default);
}


void CrunchPrintTests::thePrintAreaCommandsUseTheSelection()
{
    EditWindow e;
    QVERIFY(e.open(printBook()));
    crunch::EditSession *session = e.win.w.session();
    CrunchWindow &w = e.win.w;
    e.view()->selectRange(1, 0, 4, 1);
    w.act(QStringLiteral("printarea.set"))->trigger();
    QCOMPARE(areaOf(session->pages().sheet(0)), QStringLiteral("A2:B5"));
    QVERIFY(w.act(QStringLiteral("printarea.clear"))->isEnabled());
    e.view()->selectRange(8, 0, 9, 1);
    w.act(QStringLiteral("printarea.add"))->trigger();
    QCOMPARE(areaOf(session->pages().sheet(0)), QStringLiteral("A2:B5 A9:B10"));
    w.act(QStringLiteral("printarea.add"))->trigger();   // (the same block again adds nothing)
    QCOMPARE(areaOf(session->pages().sheet(0)), QStringLiteral("A2:B5 A9:B10"));
    e.view()->selectColumns(2, 3);
    w.act(QStringLiteral("printarea.set"))->trigger();
    QCOMPARE(areaOf(session->pages().sheet(0)), QStringLiteral("C1:D1048576"));
    w.act(QStringLiteral("printarea.clear"))->trigger();
    QVERIFY(!session->pages().sheet(0).hasPrintArea());
    QVERIFY(!w.act(QStringLiteral("printarea.clear"))->isEnabled());
    w.act(QStringLiteral("edit.undo"))->trigger();
    QCOMPARE(areaOf(session->pages().sheet(0)), QStringLiteral("C1:D1048576"));
    // adding when there is none sets
    w.act(QStringLiteral("edit.undo"))->trigger();
    w.act(QStringLiteral("edit.undo"))->trigger();
    w.act(QStringLiteral("edit.undo"))->trigger();
    w.act(QStringLiteral("edit.undo"))->trigger();
    QVERIFY(!session->pages().sheet(0).hasPrintArea());
    e.view()->selectRange(0, 0, 1, 1);
    w.act(QStringLiteral("printarea.add"))->trigger();
    QCOMPARE(areaOf(session->pages().sheet(0)), QStringLiteral("A1:B2"));
}


void CrunchPrintTests::pageBreaksAreInsertedAndRemovedAtTheSelection()
{
    EditWindow e;
    QVERIFY(e.open(printBook()));
    crunch::EditSession *session = e.win.w.session();
    CrunchWindow &w = e.win.w;
    // a break above the row and left of the column of the cell
    e.view()->selectCell(5, 1);
    w.act(QStringLiteral("breaks.insert"))->trigger();
    QCOMPARE(session->pages().sheet(0).rowBreaks.size(), 1);
    QCOMPARE(session->pages().sheet(0).rowBreaks[0].before, 5u);
    QVERIFY(session->pages().sheet(0).rowBreaks[0].manual);
    QCOMPARE(session->pages().sheet(0).colBreaks.size(), 1);
    QCOMPARE(session->pages().sheet(0).colBreaks[0].before, 1u);
    // a whole row: a break above it and no more; a whole column: one to its left
    e.view()->selectRows(8, 8);
    w.act(QStringLiteral("breaks.insert"))->trigger();
    QCOMPARE(session->pages().sheet(0).rowBreaks.size(), 2);
    QCOMPARE(session->pages().sheet(0).colBreaks.size(), 1);
    e.view()->selectColumns(3, 3);
    w.act(QStringLiteral("breaks.insert"))->trigger();
    QCOMPARE(session->pages().sheet(0).colBreaks.size(), 2);
    QCOMPARE(session->pages().sheet(0).rowBreaks.size(), 2);
    // the first row and column have nothing above or left of them: nothing is inserted, and the window says so
    const int steps = w.undoStack()->count();
    e.view()->selectCell(0, 0);
    w.act(QStringLiteral("breaks.insert"))->trigger();
    QCOMPARE(w.undoStack()->count(), steps);
    // removing: at the cell's row and column
    e.view()->selectCell(5, 1);
    w.act(QStringLiteral("breaks.remove"))->trigger();
    QCOMPARE(session->pages().sheet(0).rowBreaks.size(), 1);
    QCOMPARE(session->pages().sheet(0).rowBreaks[0].before, 8u);
    QCOMPARE(session->pages().sheet(0).colBreaks.size(), 1);
    QCOMPARE(session->pages().sheet(0).colBreaks[0].before, 3u);
    e.view()->selectCell(11, 9);   // (no break there)
    const int before = w.undoStack()->count();
    w.act(QStringLiteral("breaks.remove"))->trigger();
    QCOMPARE(w.undoStack()->count(), before);
    w.act(QStringLiteral("breaks.reset"))->trigger();
    QVERIFY(session->pages().sheet(0).rowBreaks.isEmpty());
    QVERIFY(session->pages().sheet(0).colBreaks.isEmpty());
}


// The scale to fit boxes show the sheet's setting and change it: a number of pages across or down turns the fit on, Automatic in both leaves the scale; the scale box works only then.
void CrunchPrintTests::theScaleToFitBoxesShowAndChangeTheSheet()
{
    EditWindow e;
    QVERIFY(e.open(printBook()));
    crunch::EditSession *session = e.win.w.session();
    QComboBox *width = e.win.w.findChild<QComboBox *>(QStringLiteral("fitWidthBox")), *height = e.win.w.findChild<QComboBox *>(QStringLiteral("fitHeightBox"));
    QSpinBox *scale = e.win.w.findChild<QSpinBox *>(QStringLiteral("scaleBox"));
    QVERIFY(width && height && scale);
    QCOMPARE(width->currentData().toInt(), 0);
    QCOMPARE(scale->value(), 100);
    QVERIFY(scale->isEnabled());
    scale->setValue(70);
    QCOMPARE(session->pages().sheet(0).scale, 70);
    QVERIFY(!session->pages().sheet(0).fitToPage);
    Q_EMIT width->activated(width->findData(1));
    QVERIFY(session->pages().sheet(0).fitToPage);
    QCOMPARE(session->pages().sheet(0).fitToWidth, 1);
    QCOMPARE(session->pages().sheet(0).fitToHeight, 0);
    QVERIFY(!scale->isEnabled());   // (the cells are being shrunk to fit: the scale is not what is used)
    Q_EMIT height->activated(height->findData(3));
    QCOMPARE(session->pages().sheet(0).fitToHeight, 3);
    Q_EMIT width->activated(width->findData(0));
    Q_EMIT height->activated(height->findData(0));
    QVERIFY(!session->pages().sheet(0).fitToPage);
    QVERIFY(scale->isEnabled());
    QCOMPARE(scale->value(), 70);
    // the boxes follow Undo
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    e.win.w.act(QStringLiteral("edit.undo"))->trigger();
    QCOMPARE(height->currentData().toInt(), 3);
    QCOMPARE(width->currentData().toInt(), 1);
}


// The sheet options: printing gridlines and headings is a setting of the sheet, and the two that show them are the View tab's.
void CrunchPrintTests::gridlinesAndHeadingsCanBePrinted()
{
    EditWindow e;
    QVERIFY(e.open(printBook()));
    crunch::EditSession *session = e.win.w.session();
    CrunchWindow &w = e.win.w;
    QVERIFY(!w.act(QStringLiteral("sheetopt.gridPrint"))->isChecked());
    w.act(QStringLiteral("sheetopt.gridPrint"))->trigger();
    QVERIFY(session->pages().sheet(0).gridLines);
    w.act(QStringLiteral("sheetopt.headPrint"))->trigger();
    QVERIFY(session->pages().sheet(0).headings);
    QVERIFY(!w.act(QStringLiteral("view.gridlines"))->isChecked() == false);
    w.act(QStringLiteral("sheetopt.gridView"))->trigger();   // (the same as the View tab's)
    QVERIFY(!w.act(QStringLiteral("view.gridlines"))->isChecked());
    QVERIFY(!w.act(QStringLiteral("sheetopt.gridView"))->isChecked());
    w.act(QStringLiteral("view.gridlines"))->trigger();
    QVERIFY(w.act(QStringLiteral("sheetopt.gridView"))->isChecked());
    w.act(QStringLiteral("edit.undo"))->trigger();
    QVERIFY(!session->pages().sheet(0).headings);
    QVERIFY(!w.act(QStringLiteral("sheetopt.headPrint"))->isChecked());
}


// The Page Setup command opens the dialog with the sheet's setup and the selection, on the tab asked for; what comes back is one step; a dialog given up changes nothing.
void CrunchPrintTests::thePageSetupCommandChangesTheSheetThroughTheDialog()
{
    EditWindow e;
    QVERIFY(e.open(printBook()));
    crunch::EditSession *session = e.win.w.session();
    CrunchWindow &w = e.win.w;
    int asked = 0, tab = -1;
    QString sheet, selection;
    w.setPageSetupAsker([&](PageSetupRequest *r) {
        ++asked;
        tab = r->tab;
        sheet = r->sheetName;
        selection = r->selectionText;
        r->setup.orientation = PageOrientation::Landscape;
        r->setup.margins.top = 2;
        return true;
    });
    e.view()->selectRange(0, 0, 2, 1);
    w.act(QStringLiteral("pagesetup.dialog"))->trigger();
    QCOMPARE(asked, 1);
    QCOMPARE(tab, 0);
    QCOMPARE(sheet, QStringLiteral("Data"));
    QCOMPARE(selection, QStringLiteral("A1:B3"));
    QVERIFY(session->pages().sheet(0).orientation == PageOrientation::Landscape);
    QCOMPARE(session->pages().sheet(0).margins.top, 2.0);
    QCOMPARE(w.undoStack()->count(), 1);
    w.act(QStringLiteral("margins.custom"))->trigger();
    QCOMPARE(tab, 1);
    w.act(QStringLiteral("titles.dialog"))->trigger();
    QCOMPARE(tab, 3);
    w.setPageSetupAsker([&](PageSetupRequest *) { return false; });
    const int steps = w.undoStack()->count();
    w.act(QStringLiteral("pagesetup.dialog"))->trigger();
    QCOMPARE(w.undoStack()->count(), steps);
}


// ---- the Print screen, Export, and the page breaks on the screen

// File > Print shows the pages one at a time, with the choices beside them: the page label, the arrows, the picture, and a message when something needs saying.
void CrunchPrintTests::thePrintScreenShowsThePagesAndFollowsTheChoices()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    w.act(QStringLiteral("file.print"))->trigger();
    QPushButton *print = w.findChild<QPushButton *>(QStringLiteral("printButton"));
    QVERIFY(print && print->isVisible());
    QLabel *page = w.findChild<QLabel *>(QStringLiteral("printPage"));
    QVERIFY(page);
    QTRY_COMPARE(page->text(), QStringLiteral("Page 1 of 3"));
    QLabel *preview = w.findChild<QLabel *>(QStringLiteral("printPreview"));
    QTRY_VERIFY(!preview->pixmap(Qt::ReturnByValue).isNull());
    QToolButton *next = w.findChild<QToolButton *>(QStringLiteral("printNext")), *previous = w.findChild<QToolButton *>(QStringLiteral("printPrevious"));
    QVERIFY(!previous->isEnabled());
    QVERIFY(next->isEnabled());
    next->click();
    QCOMPARE(page->text(), QStringLiteral("Page 2 of 3"));
    next->click();
    QVERIFY(!next->isEnabled());
    previous->click();
    QCOMPARE(page->text(), QStringLiteral("Page 2 of 3"));
    // a choice makes the job again: wide paper cuts the sheet into more pages, and the scaling into one
    QComboBox *orientation = w.findChild<QComboBox *>(QStringLiteral("printOrientation")), *scaling = w.findChild<QComboBox *>(QStringLiteral("printScaling"));
    orientation->setCurrentIndex(orientation->findData(int(PageOrientation::Landscape)));
    QCOMPARE(page->text(), QStringLiteral("Page 1 of 3"));
    scaling->setCurrentIndex(scaling->findData(int(crunch::PrintScaling::FitSheet)));
    QCOMPARE(page->text(), QStringLiteral("Page 1 of 1"));
    QVERIFY(!next->isEnabled());
    QVERIFY(w.findChild<QLabel *>(QStringLiteral("printNotes"))->text().contains(QStringLiteral("small")));   // (shrunk to a few percent: the letters are small)
    QCOMPARE(w.findChild<QLabel *>(QStringLiteral("printNotes"))->textFormat(), Qt::PlainText);
    QCOMPARE(w.printChoices().job.scaling, crunch::PrintScaling::FitSheet);
    // the choices are there when the screen is shown again
    w.hideFileScreen();
    w.act(QStringLiteral("file.print"))->trigger();
    QCOMPARE(w.findChild<QComboBox *>(QStringLiteral("printScaling"))->currentData().toInt(), int(crunch::PrintScaling::FitSheet));
    // the pages to print: a range names some of them, and one that is not a page turns Print off
    scaling = w.findChild<QComboBox *>(QStringLiteral("printScaling"));
    scaling->setCurrentIndex(0);
    QLineEdit *range = w.findChild<QLineEdit *>(QStringLiteral("printRange"));
    range->setText(QStringLiteral("2-3"));
    QCOMPARE(w.findChild<QLabel *>(QStringLiteral("printPage"))->text(), QStringLiteral("Page 2 of 3"));
    range->setText(QStringLiteral("99"));
    QVERIFY(!w.findChild<QPushButton *>(QStringLiteral("printButton"))->isEnabled());
    range->setText(QString());
    QVERIFY(w.findChild<QPushButton *>(QStringLiteral("printButton"))->isEnabled());
}


// Printing on both sides by hand: the odd pages, or the even ones, of the range chosen; the preview and Print follow, and a range with none of them turns Print off.
void CrunchPrintTests::theOddOrTheEvenPagesCanBePrintedAlone()
{
    const QVector<int> all = {0, 1, 2, 3, 4};
    QCOMPARE(crunch::pagesOfSide(all, crunch::PrintSides::All), all);
    QCOMPARE(crunch::pagesOfSide(all, crunch::PrintSides::Odd), (QVector<int>{0, 2, 4}));
    QCOMPARE(crunch::pagesOfSide(all, crunch::PrintSides::Even), (QVector<int>{1, 3}));
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    w.act(QStringLiteral("file.print"))->trigger();
    QLabel *page = w.findChild<QLabel *>(QStringLiteral("printPage"));
    QTRY_COMPARE(page->text(), QStringLiteral("Page 1 of 3"));
    QComboBox *sides = w.findChild<QComboBox *>(QStringLiteral("printSides"));
    QVERIFY(sides);
    QToolButton *next = w.findChild<QToolButton *>(QStringLiteral("printNext"));
    sides->setCurrentIndex(sides->findData(int(crunch::PrintSides::Odd)));
    QCOMPARE(page->text(), QStringLiteral("Page 1 of 3"));
    QVERIFY(next->isEnabled());
    next->click();
    QCOMPARE(page->text(), QStringLiteral("Page 3 of 3"));
    QVERIFY(!next->isEnabled());
    QCOMPARE(w.printChoices().sides, crunch::PrintSides::Odd);
    sides->setCurrentIndex(sides->findData(int(crunch::PrintSides::Even)));
    QCOMPARE(page->text(), QStringLiteral("Page 2 of 3"));
    QVERIFY(!next->isEnabled());
    w.findChild<QLineEdit *>(QStringLiteral("printRange"))->setText(QStringLiteral("1"));
    QVERIFY(!w.findChild<QPushButton *>(QStringLiteral("printButton"))->isEnabled());
    w.findChild<QLineEdit *>(QStringLiteral("printRange"))->setText(QString());
    sides->setCurrentIndex(0);
    QCOMPARE(page->text(), QStringLiteral("Page 1 of 3"));
}


// Print sends the pages to a PDF file the person names (the printer entry "Save as PDF"), and the screen closes; a file name that cannot be written is told.
void CrunchPrintTests::printToAFileMakesThePdfAndClosesTheScreen()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    const QString target = e.folder.dir.filePath(QStringLiteral("printed.pdf"));
    w.setSavePicker([&](const QString &, const QString &) { return target; });
    w.act(QStringLiteral("file.print"))->trigger();
    QComboBox *printer = w.findChild<QComboBox *>(QStringLiteral("printPrinter"));
    printer->setCurrentIndex(printer->count() - 1);   // (the last is "Save as PDF")
    QCOMPARE(printer->currentText(), QStringLiteral("Save as PDF"));
    w.findChild<QLineEdit *>(QStringLiteral("printRange"))->setText(QStringLiteral("1-2"));
    w.findChild<QPushButton *>(QStringLiteral("printButton"))->click();
    QFile f(target);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray pdf = f.readAll();
    QVERIFY(pdf.startsWith("%PDF-"));
    int pages = 0;
    for (qsizetype at = pdf.indexOf("/Type /Page"); at >= 0; at = pdf.indexOf("/Type /Page", at + 1))
        if (at + 11 >= pdf.size() || pdf[at + 11] != 's') ++pages;
    QCOMPARE(pages, 2);
    QVERIFY(!w.findChild<QPushButton *>(QStringLiteral("printButton"))->isVisible());   // (the screen is closed)
}


namespace {
// The pages of a PDF, each as the words on it.
QStringList pdfPageTexts(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    const jp::PdfDocument doc(f.readAll());
    QStringList out;
    for (int i = 0; doc.isValid() && i < doc.pageCount(); ++i) out << doc.text(i);
    return out;
}
// How many of the picture's pixels have a color (their red, green, and blue are far apart), and how many are dark.
struct Ink {
    int colored = 0, dark = 0;
};
Ink inkOf(const QImage &image)
{
    Ink ink;
    for (int y = 0; y < image.height(); ++y)
        for (int x = 0; x < image.width(); ++x) {
            const QColor c = image.pixelColor(x, y);
            if (std::max({c.red(), c.green(), c.blue()}) - std::min({c.red(), c.green(), c.blue()}) > 60) ++ink.colored;
            if (qGray(c.rgb()) < 128) ++ink.dark;
        }
    return ink;
}
} // namespace


// Last page first: the preview starts on the last page and the file's pages come out in reverse; by hand the order is the program's, so the choice is off.
void CrunchPrintTests::theLastPageFirstChoiceReversesThePreviewAndThePrintedFile()
{
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>Alphaword</t></is></c></row><row r=\"2\"><c r=\"A2\" t=\"inlineStr\"><is><t>Bravoword</t></is></c></row>"
                                       "<row r=\"3\"><c r=\"A3\" t=\"inlineStr\"><is><t>Charlieword</t></is></c></row>");
    const QByteArray book = buildXlsx({worksheet(rows, QStringLiteral("<rowBreaks count=\"2\" manualBreakCount=\"2\"><brk id=\"1\" max=\"16383\" man=\"1\"/><brk id=\"2\" max=\"16383\" man=\"1\"/></rowBreaks>"))});
    EditWindow e;
    QVERIFY(e.open(book));
    CrunchWindow &w = e.win.w;
    const QString target = e.folder.dir.filePath(QStringLiteral("order.pdf"));
    w.setSavePicker([&](const QString &, const QString &) { return target; });
    auto printed = [&](bool reversed) {
        w.act(QStringLiteral("file.print"))->trigger();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);   // (the page that was there is gone, so what is found is the new one)
        QComboBox *printer = w.findChild<QComboBox *>(QStringLiteral("printPrinter"));
        printer->setCurrentIndex(printer->count() - 1);
        QCheckBox *reverse = w.findChild<QCheckBox *>(QStringLiteral("printReverse"));
        QVERIFY(reverse);
        QCOMPARE(reverse->text(), QStringLiteral("Last page first"));
        reverse->setChecked(reversed);
        QLabel *page = w.findChild<QLabel *>(QStringLiteral("printPage"));
        QTRY_COMPARE(page->text(), reversed ? QStringLiteral("Page 3 of 3") : QStringLiteral("Page 1 of 3"));
        w.findChild<QPushButton *>(QStringLiteral("printButton"))->click();
    };
    printed(false);
    QStringList texts = pdfPageTexts(target);
    QCOMPARE(texts.size(), 3);
    QVERIFY(texts[0].contains(QStringLiteral("Alphaword")) && texts[1].contains(QStringLiteral("Bravoword")) && texts[2].contains(QStringLiteral("Charlieword")));
    printed(true);
    texts = pdfPageTexts(target);
    QCOMPARE(texts.size(), 3);
    QVERIFY2(texts[0].contains(QStringLiteral("Charlieword")) && texts[1].contains(QStringLiteral("Bravoword")) && texts[2].contains(QStringLiteral("Alphaword")), qPrintable(texts.join('|')));
    // (the order is chosen again on the page, and both sides by hand turns it off, which a printer that has no sides of its own is for)
    setDuplexSupportStandIn([](const QString &) { return QList<QPrinter::DuplexMode>{QPrinter::DuplexNone, QPrinter::DuplexLongSide, QPrinter::DuplexShortSide}; });
    const auto done = qScopeGuard([] { setDuplexSupportStandIn(nullptr); });
    w.act(QStringLiteral("file.print"))->trigger();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);   // (the page that was there is gone, so what is found is the new one)
    QComboBox *printer = w.findChild<QComboBox *>(QStringLiteral("printPrinter"));
    printer->insertItem(0, QStringLiteral("a printer"), QStringLiteral("a printer"));
    printer->setCurrentIndex(0);
    QCheckBox *reverse = w.findChild<QCheckBox *>(QStringLiteral("printReverse"));
    QVERIFY(reverse->isEnabled());
    QComboBox *duplex = w.findChild<QComboBox *>(QStringLiteral("printDuplex"));
    QVERIFY(duplex);
    duplex->setCurrentIndex(duplex->findData(int(SidesChoice::ByHand)));
    QVERIFY(!reverse->isEnabled());
    duplex->setCurrentIndex(0);
}


// What prints (markup, hidden text, background colors) is JeffWrite's alone: the other apps' Print pages have none of those choices.
void CrunchPrintTests::thePrintPageHasNoChoicesOfWhatPrints()
{
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>Some words</t></is></c></row>");
    const QByteArray book = buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), findStyles());
    EditWindow e;
    QVERIFY(e.open(book));
    CrunchWindow &w = e.win.w;
    w.act(QStringLiteral("file.print"))->trigger();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(w.findChild<QCheckBox *>(QStringLiteral("printGray")));   // (the page is there)
    for (const char *name : {"printMarkup", "printHidden", "printBackground"}) QVERIFY2(!w.findChild<QWidget *>(QString::fromLatin1(name)), name);
}


// Grayscale: a cell filled with a color comes out gray in the preview and in the PDF; without the choice it keeps its color.
void CrunchPrintTests::theGrayscaleChoiceMakesThePreviewAndThePrintedFileGray()
{
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" s=\"3\" t=\"inlineStr\"><is><t>Redcell</t></is></c><c r=\"B1\" s=\"3\"/><c r=\"C1\" s=\"3\"/></row>");
    const QByteArray book = buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), findStyles());
    EditWindow e;
    QVERIFY(e.open(book));
    CrunchWindow &w = e.win.w;
    const QString target = e.folder.dir.filePath(QStringLiteral("gray.pdf"));
    w.setSavePicker([&](const QString &, const QString &) { return target; });
    auto printed = [&](bool gray, Ink *shown, Ink *made) {
        w.act(QStringLiteral("file.print"))->trigger();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);   // (the page that was there is gone, so what is found is the new one)
        QComboBox *printer = w.findChild<QComboBox *>(QStringLiteral("printPrinter"));
        printer->setCurrentIndex(printer->count() - 1);
        QCheckBox *check = w.findChild<QCheckBox *>(QStringLiteral("printGray"));
        QVERIFY(check);
        QCOMPARE(check->text(), QStringLiteral("Grayscale"));
        check->setChecked(gray);
        QLabel *preview = w.findChild<QLabel *>(QStringLiteral("printPreview"));
        QTRY_VERIFY(!preview->pixmap().isNull());
        // (the picture is drawn again with the choice: wait for the one that has it)
        if (gray) QTRY_COMPARE(inkOf(preview->pixmap().toImage()).colored, 0);
        else QTRY_VERIFY(inkOf(preview->pixmap().toImage()).colored > 20);
        *shown = inkOf(preview->pixmap().toImage());
        w.findChild<QPushButton *>(QStringLiteral("printButton"))->click();
        QFile f(target);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const jp::PdfDocument doc(f.readAll());
        QVERIFY(doc.isValid());
        *made = inkOf(doc.render(0, QSize(612, 792)));
    };
    Ink shown, color, gray;
    printed(false, &shown, &color);
    QVERIFY2(color.colored > 20, qPrintable(QString::number(color.colored)));
    printed(true, &shown, &gray);
    QCOMPARE(gray.colored, 0);
    QVERIFY(gray.dark > 5);   // (the words, and the fill, are there)
    QVERIFY(shown.dark > 5);
    w.printChoices() = crunch::PrintChoices();
}


namespace {
// A workbook whose rows each print on a page of their own (a break after each), each saying its word.
QByteArray wordPageBook(int pages)
{
    static const char *words[] = {"Alphaword", "Bravoword", "Charlieword", "Deltaword"};
    QString rows, breaks;
    for (int i = 0; i < pages; ++i) {
        rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\" t=\"inlineStr\"><is><t>%2</t></is></c></row>").arg(i + 1).arg(QLatin1String(words[i]));
        if (i + 1 < pages) breaks += QStringLiteral("<brk id=\"%1\" max=\"16383\" man=\"1\"/>").arg(i + 1);
    }
    return buildXlsx({worksheet(rows, QStringLiteral("<rowBreaks count=\"%1\" manualBreakCount=\"%1\">%2</rowBreaks>").arg(pages - 1).arg(breaks))});
}
// What each page of the PDF has of the words (A for Alphaword, ...), "" for a blank page, joined.
QString printedWords(const QString &path)
{
    QStringList out;
    for (const QString &text : pdfPageTexts(path)) {
        QStringList has;
        for (const char *word : {"Alpha", "Bravo", "Charlie", "Delta"})
            if (text.contains(QLatin1String(word))) has << QString(QLatin1Char(word[0]));
        out << has.join(QLatin1Char('+'));
    }
    return out.join(QLatin1Char(' '));
}
// Prints with the choices on a printer that writes a file (the way a test stands in for a real one), and what came out.
QString printedWith(CrunchWindow &w, crunch::PrintChoices c, const QString &file)
{
    c.printer = QStringLiteral("a printer");
    w.setPrinterPreparer([&](QPrinter &p) {
        p.setOutputFormat(QPrinter::PdfFormat);
        p.setOutputFileName(file);
    });
    QFile::remove(file);
    if (!w.printWith(c)) return QStringLiteral("(not printed)");
    return printedWords(file);
}
} // namespace


// Last page first with both sides of the paper: each front stays with its back (pages 1-4 come out as 3 4, then 1 2); an odd last page has a blank back.
void CrunchPrintTests::reverseKeepsEachFrontWithItsBackWhenPrintingOnBothSides()
{
    EditWindow e;
    QVERIFY(e.open(wordPageBook(4)));
    CrunchWindow &w = e.win.w;
    const QString file = e.folder.dir.filePath(QStringLiteral("d.pdf"));
    crunch::PrintChoices c;
    c.reverse = true;
    c.duplex = QPrinter::DuplexLongSide;
    QCOMPARE(printedWith(w, c, file), QStringLiteral("C D A B"));
    c.duplex = QPrinter::DuplexNone;
    QCOMPARE(printedWith(w, c, file), QStringLiteral("D C B A"));
    c.pageRange = QStringLiteral("1-3");
    c.duplex = QPrinter::DuplexLongSide;
    QCOMPARE(printedWith(w, c, file), QStringLiteral("C  A B"));
}


// Last page first with several pages on a sheet: the pages are put in sheets first, and the sheets last first, in a printer and in a PDF file.
void CrunchPrintTests::reverseGroupsTheSheetsFirstWhenSeveralPagesShareOne()
{
    EditWindow e;
    QVERIFY(e.open(wordPageBook(4)));
    CrunchWindow &w = e.win.w;
    const QString file = e.folder.dir.filePath(QStringLiteral("n.pdf"));
    crunch::PrintChoices c;
    c.reverse = true;
    c.pagesPerSheet = 2;
    QCOMPARE(printedWith(w, c, file), QStringLiteral("C+D A+B"));
    c.pageRange = QStringLiteral("1-3");
    QCOMPARE(printedWith(w, c, file), QStringLiteral("C A+B"));
    // a PDF file
    c.pageRange.clear();
    c.toFile = true;
    c.printer.clear();
    w.setPrinterPreparer(nullptr);
    w.setSavePicker([&](const QString &, const QString &) { return file; });
    QFile::remove(file);
    QVERIFY(w.printWith(c));
    QCOMPARE(printedWords(file), QStringLiteral("C+D A+B"));
    // the preview follows: the first sheet shown is the last
    w.act(QStringLiteral("file.print"))->trigger();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    w.findChild<QComboBox *>(QStringLiteral("printPerSheet"))->setCurrentIndex(w.findChild<QComboBox *>(QStringLiteral("printPerSheet"))->findData(2));
    w.findChild<QCheckBox *>(QStringLiteral("printReverse"))->setChecked(true);
    QTRY_COMPARE(w.findChild<QLabel *>(QStringLiteral("printPage"))->text(), QStringLiteral("Sheet 1 of 2"));
    w.printChoices() = crunch::PrintChoices();
}


// Copies the program sends again, collated, on both sides of the paper: each copy begins on a new sheet (an odd number of pages ends with a blank back).
void CrunchPrintTests::collatedCopiesOnBothSidesEachBeginOnANewSheet()
{
    {
        QPrinter probe;
        probe.setOutputFormat(QPrinter::PdfFormat);
        if (probe.supportsMultipleCopies()) QSKIP("this printer engine makes copies itself, so the pages are not sent again");
    }
    EditWindow e;
    QVERIFY(e.open(wordPageBook(3)));
    CrunchWindow &w = e.win.w;
    const QString file = e.folder.dir.filePath(QStringLiteral("c.pdf"));
    crunch::PrintChoices c;
    c.duplex = QPrinter::DuplexLongSide;
    c.copies = 2;
    c.collate = true;
    QCOMPARE(printedWith(w, c, file), QStringLiteral("A B C  A B C "));
    c.duplex = QPrinter::DuplexNone;
    QCOMPARE(printedWith(w, c, file), QStringLiteral("A B C A B C"));
}


// A printer that is not there any more is said in plain words, and nothing is sent or said to be sent.
void CrunchPrintTests::aPrinterThatIsGoneIsSaidAndNothingIsSent()
{
    EditWindow e;
    QVERIFY(e.open(wordPageBook(2)));
    CrunchWindow &w = e.win.w;
    QStringList said;
    w.setNotifier([&](const QString &, const QString &text) { said << text; });
    setPrinterNamesStandIn([] { return QStringList{QStringLiteral("Front Desk")}; });
    const auto done = qScopeGuard([] { setPrinterNamesStandIn(nullptr); });
    w.statusBar()->clearMessage();
    crunch::PrintChoices c;
    c.printer = QStringLiteral("No Such Printer");
    QVERIFY(!w.printWith(c));
    QCOMPARE(said.size(), 1);
    QVERIFY2(said[0].contains(QStringLiteral("No Such Printer")), qPrintable(said[0]));
    QVERIFY(!w.statusBar()->currentMessage().contains(QStringLiteral("Sent")));
    // the list is asked for when the Print page is opened, each time, and not before
    int asked = 0;
    setPrinterNamesStandIn([&] {
        ++asked;
        return QStringList{QStringLiteral("Front Desk")};
    });
    QCOMPARE(asked, 0);
    w.act(QStringLiteral("file.print"))->trigger();
    QCOMPARE(asked, 1);
    w.act(QStringLiteral("file.print"))->trigger();
    QVERIFY(asked >= 2);
}


// Gray pages that cannot be made (no room for the pixels) stay in color, and the page says so; the preview is never blank.
void CrunchPrintTests::grayPagesThatCannotBeMadeStayInColorAndAreSaid()
{
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" s=\"3\" t=\"inlineStr\"><is><t>Redcell</t></is></c><c r=\"B1\" s=\"3\"/><c r=\"C1\" s=\"3\"/></row>");
    EditWindow e;
    QVERIFY(e.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), findStyles())));
    CrunchWindow &w = e.win.w;
    crunch::PrintJob::setGrayPixelLimitForTest(-1);
    const auto done = qScopeGuard([] { crunch::PrintJob::setGrayPixelLimitForTest(16.0 * 1000 * 1000); });
    w.act(QStringLiteral("file.print"))->trigger();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    w.findChild<QCheckBox *>(QStringLiteral("printGray"))->setChecked(true);
    QLabel *preview = w.findChild<QLabel *>(QStringLiteral("printPreview"));
    QTRY_VERIFY(!preview->pixmap().isNull());
    QTRY_VERIFY(inkOf(preview->pixmap().toImage()).colored > 20);   // (in color, not blank)
    QTRY_VERIFY(w.findChild<QLabel *>(QStringLiteral("printNotes"))->text().contains(QStringLiteral("not room")));
    w.printChoices() = crunch::PrintChoices();
}


// Several pages on a sheet: the pages of the job are put 2, 4, and so on to a sheet of the file, so it has fewer sheets (3 pages: 2 sheets for 2 on a sheet, 1 for 4).
void CrunchPrintTests::severalPagesCanShareASheet()
{
    auto sheetsWith = [&](int perSheet) {
        EditWindow e;
        if (!e.open(rowsWithFormulas(100, {}))) return -1;
        CrunchWindow &w = e.win.w;
        const QString target = e.folder.dir.filePath(QStringLiteral("n.pdf"));
        w.setSavePicker([&](const QString &, const QString &) { return target; });
        w.act(QStringLiteral("file.print"))->trigger();
        QComboBox *printer = w.findChild<QComboBox *>(QStringLiteral("printPrinter"));
        printer->setCurrentIndex(printer->count() - 1);
        QComboBox *per = w.findChild<QComboBox *>(QStringLiteral("printPerSheet"));
        if (!per) return -2;
        per->setCurrentIndex(per->findData(perSheet));
        w.findChild<QPushButton *>(QStringLiteral("printButton"))->click();
        QFile f(target);
        if (!f.open(QIODevice::ReadOnly)) return -3;
        const QByteArray pdf = f.readAll();
        int pages = 0;
        for (qsizetype at = pdf.indexOf("/Type /Page"); at >= 0; at = pdf.indexOf("/Type /Page", at + 1))
            if (at + 11 >= pdf.size() || pdf[at + 11] != 's') ++pages;
        return pages;
    };
    QCOMPARE(sheetsWith(1), 3);
    QCOMPARE(sheetsWith(2), 2);
    QCOMPARE(sheetsWith(4), 1);
}


// The printer the pages go to is a QPrinter the window keeps: pages sent to one that writes a file are the pages of the job, with the copies the choices ask for.
void CrunchPrintTests::pagesGoToAPrinterObject()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    const QString target = e.folder.dir.filePath(QStringLiteral("viaprinter.pdf"));
    w.setPrinterPreparer([&](QPrinter &p) {
        p.setOutputFormat(QPrinter::PdfFormat);
        p.setOutputFileName(target);
    });
    crunch::PrintChoices c;
    c.copies = 1;
    c.pageRange = QStringLiteral("2");
    QVERIFY(w.printWith(c));
    QFile f(target);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QVERIFY(f.readAll().startsWith("%PDF-"));
    // a range that is not a page is told and nothing is sent
    c.pageRange = QStringLiteral("40");
    QVERIFY(!w.printWith(c));
    QVERIFY(!e.win.told.isEmpty());
    QVERIFY(e.win.told.last().contains(QStringLiteral("not pages")));
}


namespace {
// Runs act on the modal box once one is up: it looks every 20 ms for half a minute (a slow machine shows the box late, and a single look
// at 50 ms left it open for good on Windows CI), and act says whether the box was the one it wanted. The looks stop with the window.
void whenModal(QObject *window, std::function<bool(QWidget *)> act, int tries = 1500)
{
    QTimer::singleShot(20, window, [window, act, tries] {
        QWidget *box = QApplication::activeModalWidget();
        if (box && act(box)) return;
        if (tries > 0) whenModal(window, act, tries - 1);
        else if (auto *d = qobject_cast<QDialog *>(box)) d->reject();   // (the test fails instead of waiting for ever)
    });
}

// Printers that can and can't print on both sides, for a test: the names say which.
struct DuplexStandIn {
    QVector<QPrinter::DuplexMode> asked;   // what the jobs asked the printer for, in order (a machine with no printers has a printer that keeps nothing, so this is where to look)
    DuplexStandIn()
    {
        setDuplexRecorder([this](QPrinter::DuplexMode mode) { asked << mode; });
        setDuplexSupportStandIn([](const QString &name) {
            if (name == QLatin1String("simplex printer")) return QList<QPrinter::DuplexMode>{QPrinter::DuplexNone};
            return QList<QPrinter::DuplexMode>{QPrinter::DuplexNone, QPrinter::DuplexLongSide, QPrinter::DuplexShortSide};
        });
    }
    ~DuplexStandIn()
    {
        setDuplexSupportStandIn(nullptr);
        setDuplexRecorder(nullptr);
    }
};

// The Print page, with the two printers a test needs put in front of "Save as PDF" (once, however often the page is opened).
QComboBox *openPrintPage(CrunchWindow &w)
{
    w.act(QStringLiteral("file.print"))->trigger();
    auto *printer = w.findChild<QComboBox *>(QStringLiteral("printPrinter"));
    if (!printer) return nullptr;
    if (printer->findText(QStringLiteral("duplex printer")) < 0) {
        printer->insertItem(0, QStringLiteral("simplex printer"), QStringLiteral("simplex printer"));
        printer->insertItem(0, QStringLiteral("duplex printer"), QStringLiteral("duplex printer"));
        printer->setCurrentIndex(0);
    }
    return printer;
}
}


// The Sides choice: one side, or both sides flipped on the long or the short edge; the printer that gets the pages is set to it, and the choice is there again when the page is opened again.
void CrunchPrintTests::theSidesChoiceSetsTheDuplexModeOfThePrinter()
{
    const DuplexStandIn standIn;
    const QVector<QPrinter::DuplexMode> wanted = {QPrinter::DuplexNone, QPrinter::DuplexLongSide, QPrinter::DuplexShortSide};
    for (int i = 0; i < 3; ++i) {
        // (a window each: the printer a window keeps stays a PDF writer once a test has made it one, and a PDF writer keeps no duplex mode)
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        Settings::get().setValue(QStringLiteral("print/sides"), 0);
        QVERIFY(openPrintPage(w));
        auto *duplex = w.findChild<QComboBox *>(QStringLiteral("printDuplex"));
        QVERIFY(duplex);
        if (i == 0) {
            QCOMPARE(duplex->count(), 4);
            QCOMPARE(duplex->itemText(3), QStringLiteral("Print on both sides by hand"));
            QCOMPARE(duplex->itemText(0), QStringLiteral("Print on one side"));
            QCOMPARE(duplex->itemText(1), QStringLiteral("Print on both sides, flip on long edge"));
            QCOMPARE(duplex->itemText(2), QStringLiteral("Print on both sides, flip on short edge"));
            QCOMPARE(duplex->currentIndex(), 0);   // (one side is where it starts)
            // the odd and even pages stay a choice of their own
            QVERIFY(w.findChild<QComboBox *>(QStringLiteral("printSides")));
        }
        QVector<QPrinter::DuplexMode> seen;
        w.setPrinterPreparer([&](QPrinter &p) {
            seen << (standIn.asked.isEmpty() ? QPrinter::DuplexNone : standIn.asked.last());
            p.setOutputFormat(QPrinter::PdfFormat);
            p.setOutputFileName(e.folder.dir.filePath(QStringLiteral("d.pdf")));
        });
        duplex->setCurrentIndex(i);
        w.findChild<QPushButton *>(QStringLiteral("printButton"))->click();
        QCOMPARE(seen, (QVector<QPrinter::DuplexMode>{wanted[i]}));
        // what was chosen is on the page when it is opened again
        QVERIFY(openPrintPage(w));
        QCOMPARE(w.findChild<QComboBox *>(QStringLiteral("printDuplex"))->currentIndex(), i);
    }
}


// Both sides by hand: the odd pages, a message to turn the stack over, then the even pages (last first); the message is left out for a job of one page, and declining it stops the job.
void CrunchPrintTests::theSidesChoiceByHandPrintsTheOddPagesThenTheEvenPages()
{
    const DuplexStandIn standIn;
    auto pagesIn = [](const QString &path) {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) return -1;
        const QByteArray pdf = f.readAll();
        int pages = 0;
        for (qsizetype at = pdf.indexOf("/Type /Page"); at >= 0; at = pdf.indexOf("/Type /Page", at + 1))
            if (at + 11 >= pdf.size() || pdf[at + 11] != 's') ++pages;
        return pages;
    };
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    QComboBox *printer = openPrintPage(w);
    QVERIFY(printer);
    auto *duplex = w.findChild<QComboBox *>(QStringLiteral("printDuplex"));
    QVERIFY(duplex);
    duplex->setCurrentIndex(3);
    QStringList order;
    QVector<int> counts;
    QVector<QPrinter::DuplexMode> modes;
    w.setPrinterPreparer([&](QPrinter &p) {
        order << QStringLiteral("print");
        modes << (standIn.asked.isEmpty() ? QPrinter::DuplexNone : standIn.asked.last());
        p.setOutputFormat(QPrinter::PdfFormat);
        p.setOutputFileName(e.folder.dir.filePath(QStringLiteral("h%1.pdf").arg(order.size())));
    });
    bool turn = true;
    setStackTurnerStandIn([&] {
        order << QStringLiteral("turn");
        counts << pagesIn(e.folder.dir.filePath(QStringLiteral("h1.pdf")));
        return turn;
    });
    const auto done = qScopeGuard([] { setStackTurnerStandIn(nullptr); });
    w.findChild<QPushButton *>(QStringLiteral("printButton"))->click();
    QCOMPARE(order, (QStringList{QStringLiteral("print"), QStringLiteral("turn"), QStringLiteral("print")}));
    QCOMPARE(modes, (QVector<QPrinter::DuplexMode>{QPrinter::DuplexNone, QPrinter::DuplexNone}));
    QCOMPARE(counts, (QVector<int>{2}));   // (pages 1 and 3 first)
    QCOMPARE(pagesIn(e.folder.dir.filePath(QStringLiteral("h3.pdf"))), 2);   // (then page 2, behind the last page 3, which has no back: a blank sheet comes first)
    // declined: nothing more is sent
    order.clear();
    QVERIFY(openPrintPage(w));
    w.findChild<QComboBox *>(QStringLiteral("printDuplex"))->setCurrentIndex(3);
    turn = false;
    crunch::PrintChoices c = w.printChoices();
    QVERIFY(c.byHand);
    QVERIFY(!w.printWith(c));
    QCOMPARE(order, (QStringList{QStringLiteral("print"), QStringLiteral("turn")}));
    // one page: it is printed, and there is nothing to turn over
    order.clear();
    c.pageRange = QStringLiteral("2");
    QVERIFY(w.printWith(c));
    QCOMPARE(order, (QStringList{QStringLiteral("print")}));
}


// Both sides by hand with several pages on a sheet: the runs are made of physical sheets, a short last sheet keeps its pages together, and every back comes behind its front (a
// front with no back gets a blank sheet in the second run, which is empty here).
void CrunchPrintTests::theHandRunsAreMadeOfPhysicalSheets()
{
    using Sheets = QVector<QVector<int>>;
    auto range = [](int from, int to) {
        QVector<int> v;
        for (int i = from; i <= to; ++i) v << i;
        return v;
    };
    auto pagesOf = [&](int n) { return range(0, n - 1); };
    {   // 7 pages, 2 on a sheet: four sheets, the last one short
        const auto runs = crunch::handSheetRuns(pagesOf(7), 2);
        QCOMPARE(runs.first, (Sheets{{0, 1}, {4, 5}}));
        QCOMPARE(runs.second, (Sheets{{6}, {2, 3}}));
    }
    {   // 9 pages, 4 on a sheet: three sheets; the third has no back
        const auto runs = crunch::handSheetRuns(pagesOf(9), 4);
        QCOMPARE(runs.first, (Sheets{{0, 1, 2, 3}, {8}}));
        QCOMPARE(runs.second, (Sheets{{}, {4, 5, 6, 7}}));
    }
    {   // 14 pages, 6 on a sheet
        const auto runs = crunch::handSheetRuns(pagesOf(14), 6);
        QCOMPARE(runs.first, (Sheets{range(0, 5), {12, 13}}));
        QCOMPARE(runs.second, (Sheets{{}, range(6, 11)}));
    }
    {   // 5 pages, 2 on a sheet, a range that skips: the pages given are what is shared out
        const auto runs = crunch::handSheetRuns({0, 2, 4, 6, 8}, 2);
        QCOMPARE(runs.first, (Sheets{{0, 2}, {8}}));
        QCOMPARE(runs.second, (Sheets{{}, {4, 6}}));
    }
    {   // one sheet: nothing to turn over
        const auto runs = crunch::handSheetRuns(pagesOf(3), 4);
        QCOMPARE(runs.first, (Sheets{{0, 1, 2}}));
        QVERIFY(runs.second.isEmpty());
    }
}


// The Sides choice is kept for the next time, in the settings every app shares for it.
void CrunchPrintTests::theSidesChoiceIsRememberedAcrossRestarts()
{
    const DuplexStandIn standIn;
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        Settings::get().setValue(QStringLiteral("print/sides"), 0);
        QVERIFY(openPrintPage(e.win.w));
        e.win.w.findChild<QComboBox *>(QStringLiteral("printDuplex"))->setCurrentIndex(2);
        QCOMPARE(Settings::get().value(QStringLiteral("print/sides")).toInt(), 2);
    }
    EditWindow later;   // (a window that has never seen the choice)
    QVERIFY(later.open(rowsWithFormulas(100, {})));
    QVERIFY(openPrintPage(later.win.w));
    QCOMPARE(later.win.w.findChild<QComboBox *>(QStringLiteral("printDuplex"))->currentIndex(), 2);
    later.win.w.findChild<QComboBox *>(QStringLiteral("printDuplex"))->setCurrentIndex(0);
}


void CrunchPrintTests::theSidesChoiceFollowsThePrinterAndTheFileTarget()
{
    const DuplexStandIn standIn;
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    QComboBox *printer = openPrintPage(w);
    QVERIFY(printer);
    auto *duplex = w.findChild<QComboBox *>(QStringLiteral("printDuplex"));
    QVERIFY(duplex);
    auto enabled = [&](int i) { return duplex->model()->flags(duplex->model()->index(i, 0)).testFlag(Qt::ItemIsEnabled); };
    QVERIFY(duplex->isEnabled() && enabled(0) && enabled(1) && enabled(2));
    duplex->setCurrentIndex(1);
    // a printer that can't print on both sides: the automatic choices are off, with a reason
    printer->setCurrentIndex(printer->findText(QStringLiteral("simplex printer")));
    QVERIFY(duplex->isEnabled() && enabled(0) && !enabled(1) && !enabled(2));
    QVERIFY(!duplex->itemData(1, Qt::ToolTipRole).toString().isEmpty());
    QVERIFY(!duplex->itemData(2, Qt::ToolTipRole).toString().isEmpty());
    QCOMPARE(duplex->currentIndex(), 0);
    printer->setCurrentIndex(printer->findText(QStringLiteral("duplex printer")));
    QCOMPARE(duplex->currentIndex(), 1);
    // a PDF file has no sides: the choice is off, and the file is made as it is
    printer->setCurrentIndex(printer->count() - 1);
    QVERIFY(!duplex->isEnabled());
    const QString target = e.folder.dir.filePath(QStringLiteral("nosides.pdf"));
    w.setSavePicker([&](const QString &, const QString &) { return target; });
    w.findChild<QPushButton *>(QStringLiteral("printButton"))->click();
    QVERIFY(QFileInfo::exists(target));
    QCOMPARE(w.printChoices().duplex, QPrinter::DuplexNone);
    QVERIFY(w.printChoices().toFile);
}


// Export to PDF asks for a name and writes the sheet in view, or every sheet that is shown.
void CrunchPrintTests::exportToPdfWritesTheSheetOrTheWorkbook()
{
    EditWindow e;
    QVERIFY(e.open(budgetWorkbook()));
    CrunchWindow &w = e.win.w;
    const QString one = e.folder.dir.filePath(QStringLiteral("one.pdf")), all = e.folder.dir.filePath(QStringLiteral("all"));
    QString answer = one;
    w.setSavePicker([&](const QString &, const QString &filter) {
        Q_UNUSED(filter);
        return answer;
    });
    QVERIFY(w.exportPdf(false));
    QVERIFY(QFileInfo::exists(one));
    answer = all;   // (no ending: .pdf is added)
    QVERIFY(w.exportPdf(true));
    QVERIFY(QFileInfo::exists(all + QStringLiteral(".pdf")));
    QVERIFY(QFileInfo(all + QStringLiteral(".pdf")).size() > QFileInfo(one).size());
    answer.clear();   // (the person gives it up)
    QVERIFY(!w.exportPdf(false));
    // the File screen's Export page has the two commands
    w.showFileScreen(QStringLiteral("export"));
    QVERIFY(w.findChild<QAbstractButton *>(QStringLiteral("exportSheetPdf")));
    QVERIFY(w.findChild<QAbstractButton *>(QStringLiteral("exportBookPdf")));
}

// A cell with a link is a clickable box in the PDF: a web address goes to the web, a place in the workbook to the page that shows it, a HYPERLINK cell to its address; a link to a file on
// another computer is never made clickable. The same in a PDF/A file.
void CrunchPrintTests::aPdfHasClickableLinksOverLinkedCells()
{
    using namespace jp::office::sheet;
    const QByteArray first = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"str\"><f>HYPERLINK(\"https://example.org/f\",\"go\")</f><v>go</v></c></row>"
                                                      "<row r=\"3\"><c r=\"A3\" t=\"inlineStr\"><is><t>web</t></is></c></row><row r=\"4\"><c r=\"A4\" t=\"inlineStr\"><is><t>place</t></is></c></row>"
                                                      "<row r=\"5\"><c r=\"A5\" t=\"inlineStr\"><is><t>unc</t></is></c></row><row r=\"6\"><c r=\"A6\" t=\"inlineStr\"><is><t>file</t></is></c></row>"));
    const QByteArray other = worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\"><is><t>there</t></is></c></row>"));
    EditWindow e;
    QVERIFY(e.open(buildXlsx({first, other}, QByteArray(), QString(), QString(), budgetStyles(), QStringList({"Links", "Other"}))));
    CrunchWindow &w = e.win.w;
    crunch::EditSession *s = w.session();
    auto link = [&](const char *ref, const QString &target, const QString &location) {
        CellRange r;
        parseRangeRef(QString::fromLatin1(ref), &r);
        LinkSpec l;
        l.target = target;
        l.location = location;
        QVERIFY(s->setLink(0, r, l, QStringLiteral("Link")).ok);
    };
    link("A3", QStringLiteral("https://example.org/a"), QString());
    link("A4", QString(), QStringLiteral("Other!A1"));
    link("A5", QStringLiteral("\\\\server\\share\\book.xlsx"), QString());
    link("A6", QStringLiteral("file://host/share/book.xlsx"), QString());
    QString path;
    w.setSavePicker([&](const QString &, const QString &) { return path; });
    for (bool archival : {false, true}) {
        path = e.folder.dir.filePath(QStringLiteral("links%1.pdf").arg(archival));
        QVERIFY(w.exportPdf(true, archival));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray pdf = f.readAll();
        f.close();
        jp::QtPdf q;
        QVERIFY(q.loadBytes(pdf.left(int(pdf.indexOf("%%EOF")) + 6)));
        QVector<int> pageIds;
        for (const jp::QtPdf::Obj &obj : q.objects)
            if (obj.body.contains("/Type /Page\n")) pageIds << obj.id;
        QCOMPARE(pageIds.size(), 2);
        QVERIFY2(pdf.contains("/URI (https://example.org/a)"), "no link for the web address");
        QVERIFY2(pdf.contains("/URI (https://example.org/f)"), "no link for the HYPERLINK cell");
        // (a bookmark for the other sheet goes to its page too: the link is the one in an annotation)
        int toOther = 0;
        for (const QByteArray &part : pdf.split('\n'))
            if (part.contains("/Subtype /Link") && part.contains("/Dest [" + QByteArray::number(pageIds[1]) + " 0 R")) ++toOther;
        QVERIFY2(toOther == 1, "no link to the page of the other sheet");
        QVERIFY(!pdf.contains("server") && !pdf.contains("host/share"));
        QCOMPARE(int(pdf.count("/Subtype /Link")), 3);
        if (archival) QVERIFY(pdf.contains("pdfaid:part=\"1\""));
    }
}

// A link to a defined name goes to the name's first cell, to the spot of the cell on its page (not the page's top) when that page is in the PDF.
void CrunchPrintTests::aLinkToADefinedNameGoesToItsCellsSpot()
{
    using namespace jp::office::sheet;
    const QByteArray first = worksheet(QStringLiteral("<row r=\"3\"><c r=\"A3\" t=\"inlineStr\"><is><t>name</t></is></c></row>"));
    QString rows;
    for (int i = 1; i <= 10; ++i) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\" t=\"inlineStr\"><is><t>row</t></is></c></row>").arg(i);
    const QByteArray other = worksheet(rows);
    EditWindow e;
    QVERIFY(e.open(buildXlsx({first, other}, QByteArray(), QString(), QString(), budgetStyles(), QStringList({"Links", "Other"}))));
    CrunchWindow &w = e.win.w;
    crunch::EditSession *s = w.session();
    crunch::EditSession::NameDraft draft;
    draft.name = QStringLiteral("Target");
    draft.text = QStringLiteral("=Other!$A$8:$B$9");
    int made = -1;
    QVERIFY(s->defineName(draft, &made).ok);
    CellRange r;
    parseRangeRef(QStringLiteral("A3"), &r);
    LinkSpec l;
    l.location = QStringLiteral("Target");
    QVERIFY(s->setLink(0, r, l, QStringLiteral("Link")).ok);
    QString path = e.folder.dir.filePath(QStringLiteral("name.pdf"));
    w.setSavePicker([&](const QString &, const QString &) { return path; });
    QVERIFY(w.exportPdf(true, false));
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray pdf = f.readAll();
    jp::QtPdf q;
    QVERIFY(q.loadBytes(pdf.left(int(pdf.indexOf("%%EOF")) + 6)));
    QVector<int> pageIds;
    for (const jp::QtPdf::Obj &obj : q.objects)
        if (obj.body.contains("/Type /Page\n")) pageIds << obj.id;
    QCOMPARE(pageIds.size(), 2);
    const QRegularExpression xyz(QStringLiteral("/Dest \\[%1 0 R /XYZ ([-0-9.]+) ([-0-9.]+) null\\]").arg(pageIds[1]));
    const auto m = xyz.match(QString::fromLatin1(pdf));
    QVERIFY2(m.hasMatch(), "no link to the cell of the name");
    // the eighth row is below the page's top margin by seven rows' height: the spot is under the top of the page (a point count under 792 less the margin)
    QVERIFY2(m.captured(2).toDouble() > 0 && m.captured(2).toDouble() < 700, qPrintable(m.captured(2)));
}

// A PDF can be protected with a password: the file is encrypted (AES-256), opens with the password and not without, is not PDF/A, and the plain PDF is not left beside it.
void CrunchPrintTests::exportToPdfCanBeProtectedWithAPassword()
{
    EditWindow e;
    QVERIFY(e.open(budgetWorkbook()));
    CrunchWindow &w = e.win.w;
    QTemporaryDir keep;
    ScratchTemp scratch;
    QString path = keep.filePath(QStringLiteral("plain.pdf"));
    w.setSavePicker([&](const QString &, const QString &) { return path; });
    // The control: with no password the watch sees the plain PDF.
    PlainPdfWatch watch;
    watch.dirs = {keep.path()};
    QVERIFY(w.exportPdf(true));
    QVERIFY2(!watch.plain.isEmpty(), "the watch does not see a plain PDF");
    watch.dirs.clear();
    watch.plain.clear();
    path = e.folder.dir.filePath(QStringLiteral("locked.pdf"));
    const QStringList watched = {e.folder.dir.path(), scratch.path()};
    watch.dirs = watched;
    const QStringList before = QDir(e.folder.dir.path()).entryList(QDir::Files);
    jp::PdfProtection p;
    p.openPassword = QStringLiteral("pw");
    p.limited = true;
    p.ownerPassword = QStringLiteral("owner");
    p.allowPrint = false;
    p.allowCopy = true;
    p.allowChange = true;
    QVERIFY(w.exportPdf(true, true, p));   // (PDF/A asked for, and not made)
    watch.dirs.clear();
    QVERIFY2(watch.plain.isEmpty(), qPrintable(watch.plain.join(QLatin1Char(' '))));
    QVERIFY2(QDir(scratch.path()).entryList(QDir::Files).isEmpty(), qPrintable(QDir(scratch.path()).entryList(QDir::Files).join(QLatin1Char(' '))));
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray made = f.readAll();
    jp::PdfEncryptionInfo info;
    QVERIFY(jp::readPdfEncryption(made, QStringLiteral("pw"), &info));
    QCOMPARE(info.version, 5);
    QCOMPARE(info.revision, 6);
    QVERIFY(!info.allowPrint && info.allowCopy && info.allowChange);
    QVERIFY(!jp::PdfDocument(made).isValid());
    QVERIFY(jp::PdfDocument(made, QStringLiteral("pw")).isValid());
    QVERIFY(!jp::decryptPdf(made, QStringLiteral("pw")).contains("pdfaid:part"));
    QStringList expected = before;
    expected << QStringLiteral("locked.pdf");
    expected.sort();
    QStringList got = QDir(e.folder.dir.path()).entryList(QDir::Files);
    got.sort();
    QCOMPARE(got, expected);
}

// Make an Email with the PDF can put a password on it, from the box and from the Export page's choice: the attachment is encrypted.
void CrunchPrintTests::anEmailedPdfCanHaveAPassword()
{
    EditWindow e;
    QVERIFY(e.open(budgetWorkbook()));
    CrunchWindow &w = e.win.w;
    ScratchTemp scratch;
    const QString eml = e.folder.dir.filePath(QStringLiteral("mail.eml"));
    w.setSavePicker([&](const QString &, const QString &) { return eml; });
    jp::PdfProtection p;
    p.openPassword = QStringLiteral("pw");
    PlainPdfWatch watch;
    watch.dirs = {e.folder.dir.path(), scratch.path()};
    QVERIFY(w.exportEmail(CrunchWindow::EmailWhat::Pdf, p));
    watch.dirs.clear();
    QVERIFY2(watch.plain.isEmpty(), qPrintable(watch.plain.join(QLatin1Char(' '))));
    QByteArray attached;
    {
        // (closed again before the next message is saved over it: Windows replaces no file that is open)
        QFile f(eml);
        QVERIFY(f.open(QIODevice::ReadOnly));
        attached = pdfAttachmentOf(f.readAll());
    }
    jp::PdfEncryptionInfo info;
    QVERIFY(jp::readPdfEncryption(attached, QStringLiteral("pw"), &info));
    QCOMPARE(info.version, 5);
    QCOMPARE(info.revision, 6);
    QVERIFY(!jp::PdfDocument(attached).isValid());
    QVERIFY(jp::PdfDocument(attached, QStringLiteral("pw")).isValid());
    // The Make an Email box has the choice, and the Export page's password is where it starts.
    jp::setPdfProtectionStandIn([](bool, jp::PdfProtection *q) {
        q->openPassword = QStringLiteral("boxpw");
        return jp::NewPasswordChoice::Set;
    });
    const auto reset = qScopeGuard([] { jp::setPdfProtectionStandIn(nullptr); });
    bool offered = false;
    whenModal(&w, [&](QWidget *box) {
        auto *d = qobject_cast<QDialog *>(box);
        auto *button = d ? d->findChild<QPushButton *>(QStringLiteral("pdfEncryptButton")) : nullptr;
        auto *what = d ? d->findChild<QComboBox *>(QStringLiteral("emailWhat")) : nullptr;
        if (!button || !what) return false;
        offered = true;
        what->setCurrentIndex(what->findData(int(CrunchWindow::EmailWhat::Pdf)));
        button->click();
        d->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        return true;
    });
    QVERIFY(w.exportEmailAsk());
    QVERIFY(offered);
    QVERIFY(jp::readPdfEncryption(pdfAttachmentOf(([&] { QFile g(eml); g.open(QIODevice::ReadOnly); return g.readAll(); })()), QStringLiteral("boxpw"), &info));
}

// A save that is turned away, or an export that fails, leaves the password on the Export page for the next try; a PDF made forgets it.
void CrunchPrintTests::aCancelledExportKeepsThePasswordOnThePage()
{
    EditWindow e;
    QVERIFY(e.open(budgetWorkbook()));
    CrunchWindow &w = e.win.w;
    QString answer;
    w.setSavePicker([&](const QString &, const QString &) { return answer; });
    w.showFileScreen(QStringLiteral("export"));
    auto *button = w.findChild<QPushButton *>(QStringLiteral("pdfEncryptButton"));
    auto *archive = w.findChild<QAbstractButton *>(QStringLiteral("exportBookPdfA"));
    QVERIFY(button && archive);
    jp::setPdfProtectionStandIn([](bool, jp::PdfProtection *p) {
        p->openPassword = QStringLiteral("pw");
        return jp::NewPasswordChoice::Set;
    });
    const auto reset = qScopeGuard([] { jp::setPdfProtectionStandIn(nullptr); });
    button->click();
    QVERIFY(!archive->isEnabled());
    answer.clear();   // (the person turns the save away)
    w.findChild<QAbstractButton *>(QStringLiteral("exportBookPdf"))->click();
    QVERIFY(!archive->isEnabled());
    QVERIFY(w.findChild<QLabel *>(QStringLiteral("pdfEncryptNote"))->isVisible() || !w.findChild<QLabel *>(QStringLiteral("pdfEncryptNote"))->text().isEmpty());
    // The same for a place that can't be written.
    answer = e.folder.dir.filePath(QStringLiteral("missing/folder/out.pdf"));
    whenModal(&w, [](QWidget *box) {
        auto *message = qobject_cast<QMessageBox *>(box);
        if (message) message->accept();
        return message != nullptr;
    });
    w.findChild<QAbstractButton *>(QStringLiteral("exportBookPdf"))->click();
    QVERIFY(!archive->isEnabled());
    // Then a good one: the PDF is made with the password, and the page forgets it.
    answer = e.folder.dir.filePath(QStringLiteral("good.pdf"));
    w.showFileScreen(QStringLiteral("export"));
    w.findChild<QAbstractButton *>(QStringLiteral("exportBookPdf"))->click();
    QFile f(answer);
    QVERIFY(f.open(QIODevice::ReadOnly));
    jp::PdfEncryptionInfo info;
    QVERIFY(jp::readPdfEncryption(f.readAll(), QStringLiteral("pw"), &info));
}

// The Export page has "Encrypt with a password…"; with one set, the PDF/A command is out of reach, and the next export starts without it.
void CrunchPrintTests::theExportPageOffersAPasswordAndTurnsOffPdfA()
{
    EditWindow e;
    QVERIFY(e.open(budgetWorkbook()));
    CrunchWindow &w = e.win.w;
    const QString path = e.folder.dir.filePath(QStringLiteral("page.pdf"));
    w.setSavePicker([&](const QString &, const QString &) { return path; });
    w.showFileScreen(QStringLiteral("export"));
    auto *button = w.findChild<QPushButton *>(QStringLiteral("pdfEncryptButton"));
    auto *archive = w.findChild<QAbstractButton *>(QStringLiteral("exportBookPdfA"));
    QVERIFY(button && archive);
    QVERIFY(archive->isEnabled());
    jp::setPdfProtectionStandIn([](bool, jp::PdfProtection *p) {
        p->openPassword = QStringLiteral("pw");
        return jp::NewPasswordChoice::Set;
    });
    const auto reset = qScopeGuard([] { jp::setPdfProtectionStandIn(nullptr); });
    button->click();
    QVERIFY(!archive->isEnabled());
    w.findChild<QAbstractButton *>(QStringLiteral("exportBookPdf"))->click();
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    jp::PdfEncryptionInfo info;
    QVERIFY(jp::readPdfEncryption(f.readAll(), QStringLiteral("pw"), &info));
    QVERIFY(info.encrypted);
    // (the password was for that export)
    w.showFileScreen(QStringLiteral("export"));
    archive = w.findChild<QAbstractButton *>(QStringLiteral("exportBookPdfA"));
    QVERIFY(archive && archive->isEnabled());
}


// The Export page has the tag choice beside the PDF/A card, on by default: the PDFs it makes are tagged for screen readers, or not when it is off.
void CrunchPrintTests::theExportPageOffersTagsForScreenReaders()
{
    EditWindow e;
    QVERIFY(e.open(budgetWorkbook()));
    CrunchWindow &w = e.win.w;
    const QString path = e.folder.dir.filePath(QStringLiteral("page.pdf"));
    w.setSavePicker([&](const QString &, const QString &) { return path; });
    jp::Settings::get().setValue(QStringLiteral("export/pdfTags"), true);
    w.showFileScreen(QStringLiteral("export"));
    auto *tags = w.findChild<QCheckBox *>(QStringLiteral("exportPdfTags"));
    QVERIFY(tags);
    QVERIFY(tags->isChecked());
    QVERIFY(tags->text().contains(QLatin1String("every PDF")));
    auto exported = [&](const char *card) {
        w.showFileScreen(QStringLiteral("export"));
        w.findChild<QAbstractButton *>(QString::fromLatin1(card))->click();
        QFile f(path);
        return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
    };
    QVERIFY(exported("exportBookPdf").contains("/StructTreeRoot"));
    QVERIFY(exported("exportBookPdfA").contains("/StructTreeRoot"));
    w.showFileScreen(QStringLiteral("export"));
    w.findChild<QCheckBox *>(QStringLiteral("exportPdfTags"))->setChecked(false);
    // (the choice is kept: the page shows it off the next time)
    w.showFileScreen(QStringLiteral("export"));
    QVERIFY(!w.findChild<QCheckBox *>(QStringLiteral("exportPdfTags"))->isChecked());
    w.findChild<QAbstractButton *>(QStringLiteral("exportBookPdf"))->click();
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QVERIFY(!f.readAll().contains("/StructTreeRoot"));
    f.close();
    w.showFileScreen(QStringLiteral("export"));
    w.findChild<QCheckBox *>(QStringLiteral("exportPdfTags"))->setChecked(true);
}


// A picture of the Export page, and the PDFs the cards make, for the docs (only with CR_SHOT_DIR: a folder).
void CrunchPrintTests::pictureOfTheExportPage()
{
    const QString dir = qEnvironmentVariable("CR_SHOT_DIR");
    if (dir.isEmpty()) QSKIP("CR_SHOT_DIR is not set");
    jp::applyUiTheme(1);
    EditWindow e;
    QVERIFY(e.open(budgetWorkbook()));
    CrunchWindow &w = e.win.w;
    w.resize(1200, 800);
    w.show();
    w.showFileScreen(QStringLiteral("export"));
    QString path = QDir(dir).filePath(QStringLiteral("crunch-tagged.pdf"));
    w.setSavePicker([&](const QString &, const QString &) { return path; });
    QTRY_VERIFY(w.findChild<QCheckBox *>(QStringLiteral("exportPdfTags"))->isVisible());
    QVERIFY(w.grab().save(QDir(dir).filePath(QStringLiteral("crunch-export-page.png"))));
    w.findChild<QAbstractButton *>(QStringLiteral("exportBookPdf"))->click();
    path = QDir(dir).filePath(QStringLiteral("crunch-archive.pdf"));
    w.showFileScreen(QStringLiteral("export"));
    w.findChild<QAbstractButton *>(QStringLiteral("exportBookPdfA"))->click();
}


// After the Print screen is shown the sheet shows where its pages begin (dashed for a break the program found, solid for one set), follows the sheet, and View > Page Breaks turns it off.
void CrunchPrintTests::thePageBreaksShowOnTheSheetAfterAPreview()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    auto rowLines = [&] {
        QList<qint64> at;
        for (const auto &l : e.view()->pageBreakRows()) at << l.before;
        return at;
    };
    QVERIFY(!e.view()->hasPageBreaks());
    w.act(QStringLiteral("file.print"))->trigger();
    w.hideFileScreen();
    QTRY_VERIFY(e.view()->hasPageBreaks());
    QTRY_COMPARE(rowLines(), (QList<qint64>{47, 94}));   // (47 rows to a page, as the printer makes the rows: the pages begin at rows 48 and 95)
    QVERIFY(!e.view()->pageBreakRows()[0].manual);
    // a break set by hand moves the lines, and is drawn as one set by hand
    e.view()->selectCell(30, 0);
    w.act(QStringLiteral("breaks.insert"))->trigger();
    QTRY_COMPARE(rowLines(), (QList<qint64>{30, 77}));
    QVERIFY(e.view()->pageBreakRows()[0].manual);
    // what is typed past the last cell makes another page, and the line follows it
    QVERIFY(w.session()->type(0, 199, 0, QStringLiteral("x")).ok);
    QTRY_COMPARE(rowLines(), (QList<qint64>{30, 77, 124, 171}));
    pressUndo(e.view());
    QTRY_COMPARE(rowLines(), (QList<qint64>{30, 77}));
    w.act(QStringLiteral("view.pageBreaks"))->trigger();   // (off)
    QTRY_VERIFY(!e.view()->hasPageBreaks());
    w.act(QStringLiteral("view.pageBreaks"))->trigger();
    QTRY_COMPARE(rowLines(), (QList<qint64>{30, 77}));
}


// Undo and Redo of a typed cell are edits that drop the lines being worked out and bring new ones; read with no timer, the sheet's pages follow each step at once.
void CrunchPrintTests::undoAndRedoOfTypingMoveThePagesAtOnce()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    auto pages = [&] {
        QString why;
        const std::unique_ptr<crunch::PrintJob> job = w.makePrintJob(crunch::PrintJobOptions(), &why);
        return job ? job->pageCount() : -1;
    };
    QCOMPARE(pages(), 3);
    for (int i = 0; i < 20; ++i) {
        QVERIFY(w.session()->type(0, 199, 0, QStringLiteral("x")).ok);
        QCOMPARE(pages(), 5);
        pressUndo(e.view());
        QCOMPARE(pages(), 3);
        pressRedo(e.view());
        QCOMPARE(pages(), 5);
        pressUndo(e.view());
        QCOMPARE(pages(), 3);
    }
}


// A job for the lines that is part way through (driven by hand here, no timer) is dropped by an edit, by Undo, and by Redo: a slice asked for after that does nothing, and no lines of the
// dropped job land; only the job begun after the edit gives lines, and one job is under way at a time.
void CrunchPrintTests::aDroppedLineJobNeverLandsItsLines()
{
    EditWindow e;
    QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1048576\"><c r=\"A1048576\"><v>1</v></c></row>"))})));
    CrunchWindow &w = e.win.w;
    w.armPageBreaks();
    w.updatePageBreaks();
    QVERIFY(w.pageBreaksUnderWay());   // (a million rows take many slices)
    QVERIFY(!e.view()->hasPageBreaks());
    w.stepPageBreaksNow();
    QVERIFY(!e.view()->hasPageBreaks());
    auto dropped = [&](const std::function<void()> &edit) {
        QVERIFY(w.pageBreaksUnderWay());
        edit();
        QVERIFY(!w.pageBreaksUnderWay());
        const int slices = w.pageBreakWork().slices;
        for (int i = 0; i < 100; ++i) w.stepPageBreaksNow();   // (a slice from the dropped job, as a timer left over would give it)
        QCOMPARE(w.pageBreakWork().slices, slices);
        QVERIFY(!e.view()->hasPageBreaks());
        w.updatePageBreaks();
        QVERIFY(w.pageBreaksUnderWay());
    };
    dropped([&] { QVERIFY(w.session()->type(0, 5, 0, QStringLiteral("x")).ok); });
    dropped([&] { pressUndo(e.view()); });
    dropped([&] { pressRedo(e.view()); });
    // the job begun last runs to its end, and its lines are the sheet's
    for (int i = 0; i < 1000 && w.pageBreaksUnderWay(); ++i) w.stepPageBreaksNow();
    QVERIFY(!w.pageBreaksUnderWay());
    QVERIFY(e.view()->pageBreakRows().size() > 1000);
}


// The lines where the pages begin on a sheet a million rows long are worked out in slices of counted work, one turn of the event loop each, and are the lines one pass makes. Each slice
// stops soon after its limit, whatever the sheet; the window is free between them.
void CrunchPrintTests::thePageBreakLinesOfAMillionRowsAreWorkedOutInSlices()
{
    EditWindow e;
    QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1048576\"><c r=\"A1048576\"><v>1</v></c></row>"))})));
    CrunchWindow &w = e.win.w;
    QString why;
    const std::unique_ptr<crunch::PrintJob> whole = w.makePrintJob(crunch::PrintJobOptions(), &why);
    QVERIFY2(whole, qPrintable(why));
    std::vector<jp::grid::GridView::PageBreakLine> rows, cols;
    CrunchWindow::pageBreakLines(*whole->printOf(0), *whole->setupOf(0), &rows, &cols);
    QVERIFY(rows.size() > 1000);
    int turns = 0;
    QTimer counter;
    counter.setInterval(0);
    connect(&counter, &QTimer::timeout, &counter, [&turns] { ++turns; });
    counter.start();
    w.act(QStringLiteral("file.print"))->trigger();
    w.hideFileScreen();
    QTRY_VERIFY_WITH_TIMEOUT(e.view()->hasPageBreaks(), 30000);
    QCOMPARE(e.view()->pageBreakRows().size(), rows.size());
    QCOMPARE(e.view()->pageBreakRows().back().before, rows.back().before);
    const CrunchWindow::PageBreakWork work = w.pageBreakWork();
    QVERIFY(work.slices > 5);
    QVERIFY2(work.most <= CrunchWindow::pageBreakSliceWork() + 16, qPrintable(QString::number(work.most)));
    // (other things were done between the slices: the loop turned about as often)
    QVERIFY2(turns >= work.slices / 2, qPrintable(QStringLiteral("%1 turns, %2 slices").arg(turns).arg(work.slices)));
}


// The lines are looked up, not searched for: a sheet of thousands of pages and of breaks gets its lines, each marked as set by hand or not, as the setup says.
void CrunchPrintTests::pageLinesComeFromTheCutsAndTheBreaks()
{
    jp::grid::SheetPrint print;
    jp::office::sheet::PageSetup setup;
    print.areas.push_back(jp::office::sheet::CellRange{0, 0, 9999, 3, true});
    for (int i = 0; i < 2000; ++i) {
        jp::grid::PageCut c;
        c.area = 0;
        c.row1 = i * 5;
        c.row2 = i * 5 + 4;
        c.rowBand = i;
        c.colBand = 0;
        print.pages.push_back(c);
    }
    for (int i = 1; i < 2000; i += 2) setup.rowBreaks.append({quint32(i * 5), 0, 3, true});
    std::vector<jp::grid::GridView::PageBreakLine> rows, cols;
    CrunchWindow::pageBreakLines(print, setup, &rows, &cols);
    QCOMPARE(int(rows.size()), 1999);
    QVERIFY(cols.empty());
    QVERIFY(rows[0].manual);    // (the line above page 2 is at row 5, a break set by hand)
    QVERIFY(!rows[1].manual);
    QCOMPARE(rows[0].from, qint64(0));
    QCOMPARE(rows[0].to, qint64(3));
    setup.fitToPage = true;   // (a sheet fitted to pages ignores the breaks)
    rows.clear();
    CrunchWindow::pageBreakLines(print, setup, &rows, &cols);
    QVERIFY(!rows[0].manual);
}


// The objects over the cells print where they are, at the page's scale, unless the file says an object stays off the paper; a draft leaves them all out.
void CrunchPrintTests::objectsPrintInPlaceUnlessTheyAreNotToBe()
{
    QString rows;
    for (int r = 1; r <= 3; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
    QString red = drawtest::twoCell(3, 1, 6, 5, drawtest::shapeElement(2, QStringLiteral("Red"), QStringLiteral("rect"), QString(), QStringLiteral("FF0000")));
    QString blue = drawtest::twoCell(3, 7, 6, 10, drawtest::shapeElement(3, QStringLiteral("Blue"), QStringLiteral("rect"), QString(), QStringLiteral("0000FF")));
    blue.replace(QStringLiteral("<xdr:clientData/>"), QStringLiteral("<xdr:clientData fPrintsWithSheet=\"0\"/>"));
    EditWindow e;
    QVERIFY(e.open(drawtest::bookWithDrawing({worksheet(rows)}, drawtest::drawingPart(red + blue), {})));
    CrunchWindow &w = e.win.w;
    QString why;
    std::unique_ptr<crunch::PrintJob> job = w.makePrintJob(crunch::PrintJobOptions(), &why);
    QVERIFY2(job, qPrintable(why));
    QVERIFY(job->printOf(0)->areas[0].row2 >= 4);   // (the red shape reaches row 5: the block takes it in)
    QVERIFY(job->printOf(0)->areas[0].col2 >= 5);
    QImage img = job->preview(0, 612);   // (a pixel to the point)
    auto at = [&](double x, double y) { return img.pixel(int(x), int(y)); };
    // column D begins 3 columns past the margin (a column is 67 of the grid's pixels, 50.25 points, on paper); row 2 is 15 points below the top margin
    const QRgb inRed = at(50.4 + 3 * 50.25 + 8, 54 + 15 + 5);
    QVERIFY2(qRed(inRed) > 200 && qGreen(inRed) < 60 && qBlue(inRed) < 60, qPrintable(QString::number(inRed, 16)));
    const QRgb inBlue = at(50.4 + 3 * 50.25 + 8, 54 + 7 * 15 + 5);
    QVERIFY2(qBlue(inBlue) > 200 ? qRed(inBlue) > 200 : true, qPrintable(QString::number(inBlue, 16)));   // (white: the shape stays off the paper)
    QVERIFY(qRed(inBlue) > 200 && qGreen(inBlue) > 200 && qBlue(inBlue) > 200);
    // at half scale the red shape is half the size
    crunch::PrintJobOptions half;
    half.scaling = crunch::PrintScaling::NoScaling;
    PageSetup p = e.win.w.session()->pages().sheet(0);
    p.scale = 50;
    QVERIFY(e.win.w.session()->setPageSetup(0, p, QStringLiteral("Page")).ok);
    job = w.makePrintJob(crunch::PrintJobOptions(), &why);
    img = job->preview(0, 612);
    const QRgb halfRed = at(50.4 + 3 * 25.125 + 8, 54 + 7 + 3);
    QVERIFY2(qRed(halfRed) > 200 && qGreen(halfRed) < 60, qPrintable(QString::number(halfRed, 16)));
    // a draft prints no objects
    p.draft = true;
    QVERIFY(e.win.w.session()->setPageSetup(0, p, QStringLiteral("Page")).ok);
    job = w.makePrintJob(crunch::PrintJobOptions(), &why);
    img = job->preview(0, 612);
    const QRgb none = at(50.4 + 3 * 25.125 + 8, 54 + 7 + 3);
    QVERIFY(qGreen(none) > 200);
}


// A sheet with no cells but with a drawing prints the drawing: the block runs from A1 to the last cell it covers. Without a drawing that prints (hidden, or told not to print), it has nothing.
void CrunchPrintTests::aSheetOfOnlyDrawingsPrintsThem()
{
    const QString red = drawtest::twoCell(3, 1, 6, 5, drawtest::shapeElement(2, QStringLiteral("Red"), QStringLiteral("rect"), QString(), QStringLiteral("FF0000")));
    EditWindow e;
    QVERIFY(e.open(drawtest::bookWithDrawing({worksheet(QString())}, drawtest::drawingPart(red), {})));
    CrunchWindow &w = e.win.w;
    QString why;
    const std::unique_ptr<crunch::PrintJob> job = w.makePrintJob(crunch::PrintJobOptions(), &why);
    QVERIFY2(job, qPrintable(why));
    QCOMPARE(job->pageCount(), 1);
    QVERIFY(job->printOf(0)->areas[0].row2 >= 4);
    QVERIFY(job->printOf(0)->areas[0].col2 >= 5);
    const QImage img = job->preview(0, 612);
    const QRgb inRed = img.pixel(int(50.4 + 3 * 50.25 + 8), int(54 + 15 + 5));
    QVERIFY2(qRed(inRed) > 200 && qGreen(inRed) < 60 && qBlue(inRed) < 60, qPrintable(QString::number(inRed, 16)));
    // one told not to print leaves the sheet with nothing
    QString quiet = red;
    quiet.replace(QStringLiteral("<xdr:clientData/>"), QStringLiteral("<xdr:clientData fPrintsWithSheet=\"0\"/>"));
    EditWindow none;
    QVERIFY(none.open(drawtest::bookWithDrawing({worksheet(QString())}, drawtest::drawingPart(quiet), {})));
    QString noneWhy;
    QVERIFY(!none.win.w.makePrintJob(crunch::PrintJobOptions(), &noneWhy));
    QVERIFY(noneWhy.contains(QStringLiteral("nothing to print")));
}

// A job with more pages than it may have is cut short, and says so; a sheet with nothing on it says that; odd margins and scales still give pages.
void CrunchPrintTests::aJobThatNeedsTooManyPagesIsCutShort()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(2000, {})));
    CrunchWindow &w = e.win.w;
    crunch::PrintJobOptions options;
    options.maxPages = 10;
    QString why;
    const std::unique_ptr<crunch::PrintJob> job = w.makePrintJob(options, &why);
    QVERIFY2(job, qPrintable(why));
    QCOMPARE(job->pageCount(), 10);
    QVERIFY(!job->messages().isEmpty());
    QVERIFY(job->messages().first().contains(QStringLiteral("Only the first 10")));
    // margins that leave no room, a scale far past the limits: still pages, none empty
    crunch::EditSession *session = w.session();
    PageSetup p = session->pages().sheet(0);
    p.margins = {-5, 500, 500, -1, 900, 900};
    p.scale = 5;
    QVERIFY(session->setPageSetup(0, p, QStringLiteral("Page")).ok);
    const std::unique_ptr<crunch::PrintJob> odd = w.makePrintJob(crunch::PrintJobOptions(), &why);
    QVERIFY2(odd, qPrintable(why));
    QVERIFY(odd->pageCount() >= 1);
    QVERIFY(!odd->preview(0, 200).isNull());
    // a sheet with nothing on it has nothing to print
    EditWindow empty;
    QVERIFY(empty.open(buildXlsx({worksheet(QString())})));
    QString emptyWhy;
    QVERIFY(!empty.win.w.makePrintJob(crunch::PrintJobOptions(), &emptyWhy));
    QVERIFY(emptyWhy.contains(QStringLiteral("nothing to print")));
}


// Hostile page settings in a file: numbers that are not numbers or are far out, a thousand breaks, a header of ten thousand codes, a paper of a thousand inches, on a sheet of a million
// rows. Opening, cutting, drawing a page, and writing a PDF of its first pages all finish with the work bounded: no page count past the bound, no empty page, no paper without a size.
void CrunchPrintTests::hostilePageSettingsAreBounded()
{
    QString breaks;
    for (int i = 1; i <= 6000; ++i) breaks += QStringLiteral("<brk id=\"%1\" max=\"16383\" man=\"1\"/>").arg(i * 150);
    const QString codes(10000, QLatin1Char('&'));
    const QString after = QStringLiteral("<printOptions horizontalCentered=\"x\" gridLines=\"1\" headings=\"1\"/>"
                                         "<pageMargins left=\"-9\" right=\"1e300\" top=\"nan\" bottom=\"99999\" header=\"-1\" footer=\"1e9\"/>"
                                         "<pageSetup paperSize=\"-5\" paperWidth=\"90000in\" paperHeight=\"1mm\" scale=\"-100\" firstPageNumber=\"99999999999\" fitToWidth=\"-1\" fitToHeight=\"99999999\" "
                                         "orientation=\"sideways\" pageOrder=\"x\" errors=\"???\"/>"
                                         "<headerFooter differentFirst=\"1\" differentOddEven=\"1\"><oddHeader>") + codes + QStringLiteral("&amp;P&amp;N&amp;D</oddHeader><oddFooter>&amp;C&amp;99999Big &amp;\"") + QString(5000, QLatin1Char('x'))
                          + QStringLiteral("</oddFooter><firstHeader>&amp;L&amp;G&amp;G&amp;G</firstHeader></headerFooter><rowBreaks>") + breaks + QStringLiteral("</rowBreaks><colBreaks>") + breaks + QStringLiteral("</colBreaks>");
    QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row><row r=\"1048576\"><c r=\"C1048576\"><v>2</v></c></row>");
    EditWindow e;
    QVERIFY(e.open(buildXlsx({worksheet(rows, after)})));
    CrunchWindow &w = e.win.w;
    QString why;
    crunch::PrintJobOptions options;
    options.maxPages = 300;
    const std::unique_ptr<crunch::PrintJob> job = w.makePrintJob(options, &why);
    QVERIFY2(job, qPrintable(why));
    QVERIFY(job->pageCount() >= 1 && job->pageCount() <= 300);
    QVERIFY(!job->messages().isEmpty());   // (it was cut short)
    const jp::grid::SheetPrint *print = job->printOf(0);
    QVERIFY(print->scalePercent >= 10 && print->scalePercent <= 400);
    QVERIFY(print->paper.width >= 72 && print->paper.height >= 72);
    QVERIFY(print->paper.content().width() >= 36 && print->paper.content().height() >= 36);
    for (const jp::grid::PageCut &c : print->pages) {
        QVERIFY(c.row2 >= c.row1);
        QVERIFY(c.col2 >= c.col1);
    }
    QVERIFY(!job->preview(0, 300).isNull());
    QVERIFY(!job->preview(job->pageCount() - 1, 300).isNull());
    const QString pdf = e.folder.dir.filePath(QStringLiteral("hostile.pdf"));
    QVERIFY2(job->exportPdf(pdf, {0, 1, 2}, &why), qPrintable(why));
    QVERIFY(QFileInfo(pdf).size() > 1000);
    // the setup of the sheet is the file's, bounded, and it is saved and read again as it is
    QVERIFY(w.session()->pages().sheet(0).scale >= 10);
    QVERIFY(w.session()->pages().sheet(0).rowBreaks.size() <= jp::office::sheet::kMaxPageBreaks);
    QVERIFY(w.session()->pages().sheet(0).headerFooter.oddHeader.size() <= jp::office::sheet::kMaxHeaderFooterChars);
}


// Rows inserted and the print area changed, then saved: the file has the area as the window shows it.
void CrunchPrintTests::insertedRowsAndAPrintAreaAreSavedTogether()
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
    QVERIFY2(savedOk(saved), "the save failed");
    const PageSetup back = savedPage(e.path);
    QCOMPARE(areaOf(back), QStringLiteral("B2:C8"));
    QCOMPARE(back.rowBreaks.size(), 1);
    QCOMPARE(back.rowBreaks[0].before, 6u);
    QVERIFY(back == session->pages().sheet(0));
}


// ---- the page breaks view (printing 2)

// The cells with a view of the pages: the shaded rest, the numbers, and the lines wait for the job; here the sheet of 100 rows is three pages, numbered, over the printed cells.
void CrunchPrintTests::thePageBreaksViewNumbersThePagesOverThePrintedCells()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    QCOMPARE(w.viewKind(), 0);
    QVERIFY(!e.view()->breakView());
    // (the buttons for the views are at the bottom of the window)
    bool inStatusBar = false;
    for (QToolButton *b : w.statusBar()->findChildren<QToolButton *>())
        if (b->defaultAction() == w.act(QStringLiteral("view.pageBreakView"))) inStatusBar = true;
    QVERIFY(inStatusBar);
    w.setViewKind(1);
    QCOMPARE(w.viewKind(), 1);
    QVERIFY(e.view()->breakView());
    QVERIFY(w.act(QStringLiteral("view.pageBreakView"))->isChecked());
    QVERIFY(!w.act(QStringLiteral("view.normal"))->isChecked());
    QTRY_COMPARE(e.view()->breakAreas().size(), size_t(1));
    const grid::GridRange area = e.view()->breakAreas()[0];
    QCOMPARE(area.row1, qint64(0));
    QCOMPARE(area.row2, qint64(99));
    QCOMPARE(area.col2, qint64(1));   // (A and B have numbers)
    // the lines are on whether or not the Page Lines switch is
    w.act(QStringLiteral("view.pageBreaks"))->trigger();
    QCOMPARE(e.view()->hasPageBreaks() ? 1 : 0, 1);
    w.setViewKind(0);
    QVERIFY(!e.view()->breakView());
    QVERIFY(e.view()->breakAreas().empty());
    QTRY_VERIFY(!e.view()->hasPageBreaks());
}


// A line dragged to another row is a break set by hand there, and the whole drag is one step of Undo.
void CrunchPrintTests::draggingALineMovesABreakAsOneStep()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    w.setViewKind(1);
    e.view()->setZoom(0.3);
    auto rowLines = [&] {
        QList<qint64> at;
        for (const auto &l : e.view()->pageBreakRows()) at << l.before;
        return at;
    };
    QTRY_COMPARE(rowLines(), (QList<qint64>{47, 94}));
    QTRY_COMPARE(e.view()->breakAreas().size(), size_t(1));
    const int steps = w.session()->undoStack()->count();
    const grid::GridLayout &L = e.view()->sheet().layout();
    const int x = L.xEdge(1, false) + 5;
    const QPoint from(x, L.yEdge(47, false)), to(x, L.yEdge(30, false) + 2);
    QVERIFY(e.view()->breakGrabAt(from).kind == grid::GridView::BreakGrab::Kind::Line);
    QTest::mousePress(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, from);
    QTest::mouseMove(e.view()->viewport(), to);
    QTest::mouseRelease(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, to);
    QCOMPARE(w.session()->undoStack()->count(), steps + 1);
    const PageSetup now = w.session()->pages().sheet(0);
    QCOMPARE(now.rowBreaks.size(), 1);
    QCOMPARE(now.rowBreaks[0].before, 30u);
    QVERIFY(now.rowBreaks[0].manual);
    QTRY_COMPARE(rowLines(), (QList<qint64>{30, 77}));
    QVERIFY(e.view()->pageBreakRows()[0].manual);
    pressUndo(e.view());
    QCOMPARE(w.session()->pages().sheet(0).rowBreaks.size(), 0);
    QTRY_COMPARE(rowLines(), (QList<qint64>{47, 94}));
    // a break set by hand is moved the same way (the old one goes, no second is left behind)
    pressRedo(e.view());
    QCOMPARE(w.session()->pages().sheet(0).rowBreaks.size(), 1);
    QTRY_COMPARE(rowLines(), (QList<qint64>{30, 77}));
    const QPoint again(x, L.yEdge(30, false)), moved(x, L.yEdge(20, false) + 2);
    QTest::mousePress(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, again);
    QTest::mouseMove(e.view()->viewport(), moved);
    QTest::mouseRelease(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, moved);
    QCOMPARE(w.session()->pages().sheet(0).rowBreaks.size(), 1);
    QCOMPARE(w.session()->pages().sheet(0).rowBreaks[0].before, 20u);
}


// A line let go off the printed cells (on the headings) is taken away; letting a line go where it was changes nothing.
void CrunchPrintTests::draggingALineOffTheCellsTakesItAway()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    PageSetup p = w.session()->pages().sheet(0);
    jp::office::sheet::PageBreak b;
    b.before = 30;
    b.max = 16383;
    p.rowBreaks.append(b);
    QVERIFY(w.session()->setPageSetup(0, p, QStringLiteral("Break")).ok);
    w.setViewKind(1);
    e.view()->setZoom(0.4);
    QTRY_COMPARE(e.view()->pageBreakRows().size(), size_t(2));   // (30 by hand, and 75 as 45 rows later)
    QTRY_COMPARE(e.view()->breakAreas().size(), size_t(1));
    const grid::GridLayout &L = e.view()->sheet().layout();
    const int x = L.xEdge(1, false) + 5;
    const int steps = w.session()->undoStack()->count();
    // (let go where it was: nothing happens)
    QTest::mousePress(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(x, L.yEdge(30, false)));
    QTest::mouseRelease(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(x, L.yEdge(30, false) + 1));
    QCOMPARE(w.session()->undoStack()->count(), steps);
    // (the column headings are not cells: off the printed cells)
    QTest::mousePress(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(x, L.yEdge(30, false)));
    QTest::mouseMove(e.view()->viewport(), QPoint(x, 3));
    QTest::mouseRelease(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(x, 3));
    QCOMPARE(w.session()->undoStack()->count(), steps + 1);
    QCOMPARE(w.session()->pages().sheet(0).rowBreaks.size(), 0);
    pressUndo(e.view());
    QCOMPARE(w.session()->pages().sheet(0).rowBreaks.size(), 1);
}


// An edge of the printed cells dragged sets the print area (from the cells that print of themselves, when there is none), one step.
void CrunchPrintTests::draggingAnEdgeOfThePrintedCellsSetsThePrintArea()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(50, {})));
    CrunchWindow &w = e.win.w;
    w.setViewKind(1);
    e.view()->setZoom(0.3);
    QTRY_COMPARE(e.view()->breakAreas().size(), size_t(1));
    QCOMPARE(e.view()->breakAreas()[0].row2, qint64(49));
    const grid::GridLayout &L = e.view()->sheet().layout();
    QVERIFY(!w.session()->pages().sheet(0).hasPrintArea());
    // (the bottom edge of the cells that print of themselves is taken hold of, and the area is made of them)
    const int x = L.xEdge(1, false) + 5;
    const int steps = w.session()->undoStack()->count();
    const QPoint from(x, L.yEdge(50, false)), to(x, L.yEdge(20, false) + 1);
    const auto grab = e.view()->breakGrabAt(from);
    QVERIFY(grab.kind == grid::GridView::BreakGrab::Kind::Edge);
    QCOMPARE(grab.side, 1);
    QTest::mousePress(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, from);
    QTest::mouseMove(e.view()->viewport(), to);
    QTest::mouseRelease(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, to);
    QCOMPARE(w.session()->undoStack()->count(), steps + 1);
    QCOMPARE(areaOf(w.session()->pages().sheet(0)), QStringLiteral("A1:B20"));
    QTRY_COMPARE(e.view()->breakAreas()[0].row2, qint64(19));
    // the right edge of that area, the same way
    const QPoint right(L.xEdge(2, false), L.yEdge(10, false) + 3), narrower(L.xEdge(1, false) + 1, L.yEdge(10, false) + 3);
    QCOMPARE(e.view()->breakGrabAt(right).side, 3);
    QTest::mousePress(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, right);
    QTest::mouseMove(e.view()->viewport(), narrower);
    QTest::mouseRelease(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, narrower);
    QCOMPARE(areaOf(w.session()->pages().sheet(0)), QStringLiteral("A1:A20"));
    pressUndo(e.view());
    QCOMPARE(areaOf(w.session()->pages().sheet(0)), QStringLiteral("A1:B20"));
    pressUndo(e.view());
    QVERIFY(!w.session()->pages().sheet(0).hasPrintArea());
}


// A sheet fitted to pages uses no breaks of its own, so its lines and edges cannot be taken hold of.
void CrunchPrintTests::aFittedSheetCannotHaveItsLinesDragged()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    PageSetup p = w.session()->pages().sheet(0);
    p.fitToPage = true;
    p.fitToWidth = 1;
    p.fitToHeight = 3;
    QVERIFY(w.session()->setPageSetup(0, p, QStringLiteral("Fit")).ok);
    w.setViewKind(1);
    e.view()->setZoom(0.4);
    QTRY_COMPARE(e.view()->breakAreas().size(), size_t(1));
    const grid::GridLayout &L = e.view()->sheet().layout();
    const grid::GridRange area = e.view()->breakAreas()[0];
    QVERIFY(e.view()->breakGrabAt(QPoint(L.xEdge(1, false) + 5, L.yEdge(area.row2 + 1, false))).kind == grid::GridView::BreakGrab::Kind::None);
}


// The view a sheet was left in is saved with the workbook, comes back when the file is opened, and a workbook never shown in it is saved with no view attribute.
void CrunchPrintTests::theViewIsSavedAndComesBack()
{
    QByteArray bytes;
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        w.setViewKind(1);
        e.view()->setZoom(0.5);
        // (a view alone, with no cell changed, is something to save)
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), qPrintable(e.win.told.join(QLatin1Char('|'))));
        Package pk;
        QString why;
        QVERIFY(pk.readLazyFile(e.path, &why));
        const QByteArray xml = pk.part(QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY2(xml.contains("view=\"pageBreakPreview\""), xml.left(600).constData());
        QVERIFY(xml.contains("zoomScale=\"50\""));
        QVERIFY(xml.contains("zoomScaleSheetLayoutView=\"50\""));
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        bytes = f.readAll();
    }
    EditWindow again;
    QVERIFY(again.open(bytes));
    QCOMPARE(again.win.w.viewKind(), 1);
    QVERIFY(again.view()->breakView());
    QCOMPARE(int(std::lround(again.view()->zoom() * 100)), 50);
    // back to the cells: the attribute goes, and the sheet is saved the same way again
    again.win.w.setViewKind(0);
    QSignalSpy saved(&again.win.w, &CrunchWindow::saved);
    QVERIFY(again.win.w.save());
    QVERIFY2(savedOk(saved), qPrintable(again.win.told.join(QLatin1Char('|'))));
    Package pk;
    QString why;
    QVERIFY(pk.readLazyFile(again.path, &why));
    QVERIFY(!pk.part(QStringLiteral("xl/worksheets/sheet1.xml")).contains("view=\"pageBreakPreview\""));
    // a second save with nothing changed writes nothing
    QSignalSpy second(&again.win.w, &CrunchWindow::saved);
    QVERIFY(again.win.w.save());
    QVERIFY(second.count() == 1);
    QVERIFY(again.win.w.statusText().contains(QLatin1String("Nothing to save")));
}


// Frozen panes made in the window are saved (the `pane` of the sheet's view, with the cursor's cell), come back when the file is opened, and Unfreeze takes them out again.
void CrunchPrintTests::frozenPanesAreSavedAndComeBack()
{
    QByteArray bytes;
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        e.select(QStringLiteral("C5"));
        w.act(QStringLiteral("view.freeze"))->trigger();
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), qPrintable(e.win.told.join(QLatin1Char('|'))));
        Package pk;
        QString why;
        QVERIFY(pk.readLazyFile(e.path, &why));
        const QByteArray xml = pk.part(QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY2(xml.contains("<pane xSplit=\"2\" ySplit=\"4\" topLeftCell=\"C5\" activePane=\"bottomRight\" state=\"frozen\"/>"), xml.left(900).constData());
        QVERIFY(xml.contains("activeCell=\"C5\""));
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        bytes = f.readAll();
    }
    EditWindow again;
    QVERIFY(again.open(bytes));
    QCOMPARE(again.view()->viewState().frozenRows, qint64(4));
    QCOMPARE(again.view()->viewState().frozenCols, qint64(2));
    QCOMPARE(again.selected(), QStringLiteral("C5"));
    // the top row alone, then nothing
    again.win.w.act(QStringLiteral("view.unfreeze"))->trigger();
    again.win.w.act(QStringLiteral("view.freezeRow"))->trigger();
    QSignalSpy saved(&again.win.w, &CrunchWindow::saved);
    QVERIFY(again.win.w.save());
    QVERIFY2(savedOk(saved), qPrintable(again.win.told.join(QLatin1Char('|'))));
    Package pk;
    QString why;
    QVERIFY(pk.readLazyFile(again.path, &why));
    const QByteArray xml = pk.part(QStringLiteral("xl/worksheets/sheet1.xml"));
    QVERIFY2(xml.contains("<pane ySplit=\"1\" topLeftCell=\"A2\" activePane=\"bottomLeft\" state=\"frozen\"/>"), xml.left(900).constData());
    QVERIFY(!xml.contains("xSplit"));
    again.win.w.act(QStringLiteral("view.unfreeze"))->trigger();
    QSignalSpy last(&again.win.w, &CrunchWindow::saved);
    QVERIFY(again.win.w.save());
    QVERIFY2(savedOk(last, 20000), qPrintable(again.win.told.join(QLatin1Char('|'))));
    Package pk2;
    QVERIFY(pk2.readLazyFile(again.path, &why));
    QVERIFY(!pk2.part(QStringLiteral("xl/worksheets/sheet1.xml")).contains("<pane"));
}

// Split panes made with View > Split are saved (the `pane` of the sheet's view in the state "split", in twentieths of a point), come back when the file is opened, and the command
// is checked while they are there; holding panes takes them away.
void CrunchPrintTests::splitPanesAreSavedAndComeBack()
{
    QByteArray bytes;
    quint32 wide = 0;
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        QVERIFY(!w.act(QStringLiteral("view.split"))->isChecked());
        e.select(QStringLiteral("C5"));
        w.act(QStringLiteral("view.split"))->trigger();
        QVERIFY(w.act(QStringLiteral("view.split"))->isChecked());
        QVERIFY(e.view()->isSplit());
        wide = quint32(e.view()->viewState().splitW);
        QVERIFY(wide > 0);

        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), qPrintable(e.win.told.join(QLatin1Char('|'))));
        Package pk;
        QString why;
        QVERIFY(pk.readLazyFile(e.path, &why));
        const QByteArray xml = pk.part(QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY2(xml.contains(QStringLiteral("<pane xSplit=\"%1\" ySplit=\"%2\" topLeftCell=\"C5\" activePane=\"bottomRight\" state=\"split\"/>").arg(wide * 15).arg(e.view()->viewState().splitH * 15).toUtf8()), xml.left(900).constData());
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        bytes = f.readAll();
        // (a picture, with each pane scrolled to a place of its own)
        w.resize(1100, 700);
        w.show();
        QVERIFY(e.view()->scrollSplitPart(true, 30));
        e.view()->verticalScrollBar()->setValue(int(e.view()->sheet().layout().rows.position(60)));
        if (jp::RibbonTab *t = w.ribbon()->tab(QStringLiteral("View"))) w.ribbon()->showTab(t);
        crunchtest::saveShot(&w, QStringLiteral("split-panes"));
    }
    EditWindow again;
    QVERIFY(again.open(bytes));
    QVERIFY(again.view()->isSplit());
    QCOMPARE(quint32(again.view()->viewState().splitW), wide);
    QVERIFY(again.win.w.act(QStringLiteral("view.split"))->isChecked());
    // holding panes takes the split away, and the file then holds panes and no split
    again.select(QStringLiteral("B10"));
    again.win.w.act(QStringLiteral("view.freeze"))->trigger();
    QVERIFY(!again.view()->isSplit());
    QVERIFY(!again.win.w.act(QStringLiteral("view.split"))->isChecked());
    QSignalSpy saved(&again.win.w, &CrunchWindow::saved);
    QVERIFY(again.win.w.save());
    QVERIFY2(savedOk(saved), qPrintable(again.win.told.join(QLatin1Char('|'))));
    Package pk;
    QString why;
    QVERIFY(pk.readLazyFile(again.path, &why));
    const QByteArray xml = pk.part(QStringLiteral("xl/worksheets/sheet1.xml"));
    QVERIFY2(xml.contains("state=\"frozen\"") && !xml.contains("state=\"split\""), xml.left(900).constData());
    // the command again, and again, takes it away
    again.win.w.act(QStringLiteral("view.split"))->trigger();
    QVERIFY(again.view()->isSplit());
    again.win.w.act(QStringLiteral("view.split"))->trigger();
    QVERIFY(!again.view()->isSplit());
}


// Page Layout > Sheet Right-to-Left turns the grid round (column A at the right), is kept when another sheet is shown and back, is saved as the view's `rightToLeft`, and comes back when the
// file is opened; turned off again, the attribute is gone.
void CrunchPrintTests::sheetRightToLeftIsSavedAndComesBack()
{
    QByteArray bytes;
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c></row><row r=\"3\"><c r=\"B3\"><v>3</v></c></row>")),
                                  worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>4</v></c></row>"))})));
        CrunchWindow &w = e.win.w;
        QAction *rtl = w.act(QStringLiteral("view.rightToLeft"));
        QVERIFY(rtl && rtl->isCheckable() && rtl->isEnabled() && !rtl->isChecked());
        QVERIFY(!e.view()->sheet().rightToLeft());
        e.select(QStringLiteral("B3"));
        rtl->trigger();
        QVERIFY(rtl->isChecked());
        QVERIFY(e.view()->sheet().rightToLeft());
        QCOMPARE(e.selected(), QStringLiteral("B3"));
        // another sheet reads as its file says; back on the first, it reads from the right still
        w.setActiveSheet(1);
        QVERIFY(!e.view()->sheet().rightToLeft());
        QVERIFY(!rtl->isChecked());
        w.setActiveSheet(0);
        QVERIFY(e.view()->sheet().rightToLeft());
        QVERIFY(rtl->isChecked());
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved), qPrintable(e.win.told.join(QLatin1Char('|'))));
        Package pk;
        QString why;
        QVERIFY(pk.readLazyFile(e.path, &why));
        const QByteArray xml = pk.part(QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY2(xml.contains("rightToLeft=\"1\""), xml.left(900).constData());
        QVERIFY(!pk.part(QStringLiteral("xl/worksheets/sheet2.xml")).contains("rightToLeft"));
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        bytes = f.readAll();
    }
    EditWindow again;
    QVERIFY(again.open(bytes));
    QVERIFY(again.view()->sheet().rightToLeft());
    QVERIFY(again.win.w.act(QStringLiteral("view.rightToLeft"))->isChecked());
    again.win.w.act(QStringLiteral("view.rightToLeft"))->trigger();
    QVERIFY(!again.view()->sheet().rightToLeft());
    QSignalSpy saved(&again.win.w, &CrunchWindow::saved);
    QVERIFY(again.win.w.save());
    QVERIFY2(savedOk(saved), qPrintable(again.win.told.join(QLatin1Char('|'))));
    Package pk;
    QString why;
    QVERIFY(pk.readLazyFile(again.path, &why));
    QVERIFY(!pk.part(QStringLiteral("xl/worksheets/sheet1.xml")).contains("rightToLeft"));
}


// With the sheet's comments set to print at the end, the notes are printed after the sheet's last page, on pages of their own that carry the next numbers: each note by the cell it is on, its
// author, and its words; a note's lines are kept; with the setting off, or on "as displayed", no page is added.
void CrunchPrintTests::notesPrintAtTheEndOfTheSheet()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(60, {})));
    CrunchWindow &w = e.win.w;
    crunch::EditSession *s = w.session();
    QVERIFY(s->setNote(0, 1, 0, QStringLiteral("Check this"), QStringLiteral("Ann"), false, QStringLiteral("Add Note")).ok);
    QVERIFY(s->setNote(0, 4, 2, QStringLiteral("Second\nnote"), QString(), false, QStringLiteral("Add Note")).ok);
    QString why;
    auto make = [&](jp::office::sheet::PrintComments how) {
        jp::office::sheet::PageSetup p = s->pages().sheet(0);
        p.comments = how;
        if (!s->setPageSetup(0, p, QStringLiteral("Comments")).ok) return std::unique_ptr<crunch::PrintJob>();
        return w.makePrintJob(crunch::PrintJobOptions(), &why);
    };
    const std::unique_ptr<crunch::PrintJob> none = make(jp::office::sheet::PrintComments::None);
    QVERIFY2(none, qPrintable(why));
    const int pages = none->pageCount();
    QCOMPARE(none->noteLines(pages - 1), QStringList());
    QCOMPARE(make(jp::office::sheet::PrintComments::AsDisplayed)->pageCount(), pages);
    const std::unique_ptr<crunch::PrintJob> job = make(jp::office::sheet::PrintComments::AtEnd);
    QVERIFY2(job, qPrintable(why));
    QCOMPARE(job->pageCount(), pages + 1);
    QCOMPARE(job->noteLines(pages - 1), QStringList());
    QCOMPARE(job->noteLines(pages), (QStringList{"Cell A2: Ann: Check this", "", "Cell C5: Second", "note"}));
    QCOMPARE(job->sheetOf(pages), 0);
    QCOMPARE(job->numberOf(pages), job->numberOf(pages - 1) + 1);
    QCOMPARE(job->pageSize(pages), job->pageSize(0));
    QRectF r;
    QVERIFY(!job->cellRect(pages, 1, 0, &r));
    QCOMPARE(job->pageOfCell(0, 1, 0), 0);
    // the page is drawn: text on it, within the margins
    const QImage image = job->preview(pages, 612);
    QVERIFY(!image.isNull());
    int dark = 0, outside = 0;
    const double k = image.width() / job->pageSize(pages).width();
    const jp::grid::PaperBox box = job->paperOf(pages);
    for (int y = 0; y < image.height(); ++y)
        for (int x = 0; x < image.width(); ++x)
            if (qGray(image.pixel(x, y)) < 128) {
                ++dark;
                if (x < int(box.left * k) - 2 || y < int(box.top * k * 0.5)) ++outside;
            }
    QVERIFY2(dark > 40, qPrintable(QString::number(dark)));
    QCOMPARE(outside, 0);
    // many notes need more than one page, and every one is printed once, in the order of the cells
    for (quint32 i = 0; i < 200; ++i) QVERIFY(s->setNote(0, 10 + i % 40, 4 + i / 40, QStringLiteral("note %1").arg(i), QStringLiteral("Bo"), false, QStringLiteral("Add Note")).ok);
    const std::unique_ptr<crunch::PrintJob> many = make(jp::office::sheet::PrintComments::AtEnd);
    QVERIFY(many->pageCount() > pages + 2);
    QStringList all;
    for (int p = pages; p < many->pageCount(); ++p) all += many->noteLines(p);
    QCOMPARE(all.filter(QLatin1String("Bo: note")).size(), 200);
    QVERIFY(all.indexOf(QStringLiteral("Cell A2: Ann: Check this")) < all.indexOf(QStringLiteral("Cell C5: Second")));
}


JP_TEST_CLASS(CrunchPrintTests)

// The top pane's scroll position is saved with the sheet's view, and rows inserted above the bar leave the lower pane at the row it was at, with the split still there.
void CrunchPrintTests::theTopPanesScrollAndRowsInsertedAboveTheBarAreKept()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    e.select(QStringLiteral("C5"));
    w.act(QStringLiteral("view.split"))->trigger();
    QVERIFY(e.view()->scrollSplitPart(true, 30));
    QVERIFY(e.view()->scrollSplitPart(false, 1));
    e.view()->verticalScrollBar()->setValue(int(e.view()->sheet().layout().rows.position(60)));
    const qint64 lower = e.view()->sheet().layout().pane(grid::Pane::Body).firstRow;
    QVERIFY(lower >= 60);
    QSignalSpy saved(&w, &CrunchWindow::saved);
    QVERIFY(w.save());
    QVERIFY2(savedOk(saved), qPrintable(e.win.told.join(QLatin1Char('|'))));
    Package pk;
    QString why;
    QVERIFY(pk.readLazyFile(e.path, &why));
    const QByteArray xml = pk.part(QStringLiteral("xl/worksheets/sheet1.xml"));
    QVERIFY2(xml.contains("<sheetView workbookViewId=\"0\" topLeftCell=\"B31\">") || xml.contains("topLeftCell=\"B31\""), xml.left(900).constData());
    // rows put in above the bar
    QVERIFY(w.session()->insertRows(0, 1, 2).ok);
    QVERIFY(e.settle());
    QVERIFY(e.view()->isSplit());
    QCOMPARE(e.view()->sheet().layout().pane(grid::Pane::Body).firstRow, lower);
}
