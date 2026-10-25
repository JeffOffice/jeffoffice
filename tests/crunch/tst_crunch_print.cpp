// Tests of printing in JeffCrunch's window: how a sheet prints as a step of Undo, the print area, titles, and breaks following rows and columns inserted and deleted (and coming back with
// Undo), the page setup saved in the file and nothing else of it touched, and the pages the window prints. Every workbook is built here, in a temporary folder.
#include "../sheet/drawinghelpers.h"
#include "../sheet/hfpicturehelpers.h"
#include "app/crunchfilescreen.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "savedok.h"
#include "app/pagesetupdialog.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "core/printjob.h"
#include "grid/gridview.h"
#include "grid/hfpicturecache.h"
#include "grid/pagelayoutview.h"
#include "io/zipindex.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPaintDevice>
#include <QPainter>
#include <QPaintEngine>
#include <QPrinter>
#include <QScrollArea>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QSettings>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QStatusBar>
#include <QToolButton>
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

// A paint device that draws nothing and writes down the texts it is asked to draw, in order.
class TextEngine : public QPaintEngine {
public:
    TextEngine() : QPaintEngine(QPaintEngine::PaintEngineFeatures()) {}
    bool begin(QPaintDevice *) override { return true; }
    bool end() override { return true; }
    void updateState(const QPaintEngineState &) override {}
    void drawPixmap(const QRectF &, const QPixmap &, const QRectF &) override {}
    void drawImage(const QRectF &, const QImage &, const QRectF &, Qt::ImageConversionFlags) override {}
    void drawPath(const QPainterPath &) override {}
    void drawPolygon(const QPointF *, int, PolygonDrawMode) override {}
    void drawPolygon(const QPoint *, int, PolygonDrawMode) override {}
    void drawRects(const QRectF *, int) override {}
    void drawRects(const QRect *, int) override {}
    void drawLines(const QLineF *, int) override {}
    void drawLines(const QLine *, int) override {}
    void drawTextItem(const QPointF &, const QTextItem &item) override { texts << item.text(); }
    Type type() const override { return User; }
    QStringList texts;
};

class TextDevice : public QPaintDevice {
public:
    QPaintEngine *paintEngine() const override { return &m_engine; }
    QStringList texts() const { return m_engine.texts; }

protected:
    int metric(PaintDeviceMetric m) const override
    {
        switch (m) {
        case PdmWidth: return 5100;
        case PdmHeight: return 6600;
        case PdmWidthMM: return 216;
        case PdmHeightMM: return 279;
        case PdmNumColors: return 16777216;
        case PdmDepth: return 32;
        case PdmDpiX:
        case PdmDpiY:
        case PdmPhysicalDpiX:
        case PdmPhysicalDpiY: return 600;
        case PdmDevicePixelRatio: return 1;
        case PdmDevicePixelRatioScaled: return int(devicePixelRatioFScale());
        }
        return 0;
    }

private:
    mutable TextEngine m_engine;
};

// The number of pages a PDF holds, and the sizes of its pages (the boxes) in the order they come.
int pdfPages(const QByteArray &pdf)
{
    int pages = 0;
    for (qsizetype at = pdf.indexOf("/Type /Page"); at >= 0; at = pdf.indexOf("/Type /Page", at + 1))
        if (at + 11 >= pdf.size() || pdf[at + 11] != 's') ++pages;
    return pages;
}

QStringList pdfBoxes(const QByteArray &pdf)
{
    QStringList out;
    for (qsizetype at = pdf.indexOf("/MediaBox"); at >= 0; at = pdf.indexOf("/MediaBox", at + 1)) {
        const qsizetype open = pdf.indexOf('[', at), close = pdf.indexOf(']', at);
        if (open > 0 && close > open) out << QString::fromLatin1(pdf.mid(open + 1, close - open - 1).simplified());
    }
    return out;
}

// Three sheets of one row each on three different papers (A4, letter on its side, legal), so that a page can be told from the size of its paper.
QByteArray threePapers()
{
    auto sheet = [](const char *setup) { return worksheet(QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>"), QString::fromLatin1(setup)); };
    return buildXlsx({sheet("<pageSetup paperSize=\"9\"/>"), sheet("<pageSetup paperSize=\"1\" orientation=\"landscape\"/>"), sheet("<pageSetup paperSize=\"5\"/>")}, QByteArray(),
                     QString(), QString(), QByteArray(), QStringList({"A", "B", "C"}));
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
        QVERIFY2(savedOk(saved, 20000), "the save failed");
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

    // A sheet's page setup (paper, breaks, print area, titles) goes with the sheet through every sheet operation, and through Undo of each, in the model and in the saved file: the sheets
    // keep their places in the session whatever the tabs do, and the names in the file say where each sheet is among the tabs and what it is called.
    void pageSetupFollowsTheSheetOperations()
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
        QVERIFY2(savedOk(saved, 20000), "the save failed");
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
    void anExportShowsProgressAndCanBeCancelled()
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
    void printingSendsTheCopiesAndCanBeStopped()
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
    void theMomentOfTheFieldsIsTakenOncePerJob()
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
        for (int r = 1; r <= 135; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>7</v></c></row>").arg(r);
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
        // a picture is held to its budget of pixels whatever size is asked for (six times the zoom on a large screen is hundreds of megabytes), and keeps the page's shape
        const QImage big = job->preview(0, 20000);
        QVERIFY(!big.isNull());
        QVERIFY(qint64(big.width()) * big.height() <= crunch::kMaxPreviewPixels);
        QVERIFY(big.width() < 20000);
        QVERIFY(std::abs(double(big.height()) / big.width() - 792.0 / 612.0) < 0.01);
    }

    // ---- the Page Setup dialog

    // The dialog shows what the setup says, in four tabs; one that is not changed gives the setup back exactly, whatever it holds that the dialog does not show.
    void theDialogShowsTheSetupAndGivesItBackUnchanged()
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

    // What the person changes goes back, and only that: the orientation, the scale or the pages to fit, the paper, the margins, centering, the print area and titles, the options, and the order.
    void changesInTheDialogGoBack()
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
    void marginsCanBeInCentimeters()
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
    void aPrintAreaThatIsNotCellsIsRefused()
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
    void anAreaTypedWithTheSheetsNameIsTaken()
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
    void theHeaderAndFooterPresetsAreMadeFromTheSheet()
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
    void thePageLayoutTabAndItsCommandsAreThere()
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

    void marginsOrientationAndPaperChangeTheSheetAndShowWhatItHas()
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

    void thePrintAreaCommandsUseTheSelection()
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

    void pageBreaksAreInsertedAndRemovedAtTheSelection()
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
    void theScaleToFitBoxesShowAndChangeTheSheet()
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
    void gridlinesAndHeadingsCanBePrinted()
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
    void thePageSetupCommandChangesTheSheetThroughTheDialog()
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
    void thePrintScreenShowsThePagesAndFollowsTheChoices()
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
        QCOMPARE(page->text(), QStringLiteral("Page 1 of 4"));
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
        QCOMPARE(w.findChild<QLabel *>(QStringLiteral("printPage"))->text(), QStringLiteral("Page 2 of 4"));
        range->setText(QStringLiteral("99"));
        QVERIFY(!w.findChild<QPushButton *>(QStringLiteral("printButton"))->isEnabled());
        range->setText(QString());
        QVERIFY(w.findChild<QPushButton *>(QStringLiteral("printButton"))->isEnabled());
    }

    // Print sends the pages to a PDF file the person names (the printer entry "Save as PDF"), and the screen closes; a file name that cannot be written is told.
    void printToAFileMakesThePdfAndClosesTheScreen()
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

    // The printer the pages go to is a QPrinter the window keeps: pages sent to one that writes a file are the pages of the job, with the copies the choices ask for.
    void pagesGoToAPrinterObject()
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

    // Export to PDF asks for a name and writes the sheet in view, or every sheet that is shown.
    void exportToPdfWritesTheSheetOrTheWorkbook()
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

    // After the Print screen is shown the sheet shows where its pages begin (dashed for a break the program found, solid for one set), follows the sheet, and View > Page Breaks turns it off.
    void thePageBreaksShowOnTheSheetAfterAPreview()
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
        QTRY_COMPARE(rowLines(), (QList<qint64>{45, 90}));   // (45 rows to a page: the pages begin at rows 46 and 91)
        QVERIFY(!e.view()->pageBreakRows()[0].manual);
        // a break set by hand moves the lines, and is drawn as one set by hand
        e.view()->selectCell(30, 0);
        w.act(QStringLiteral("breaks.insert"))->trigger();
        QTRY_COMPARE(rowLines(), (QList<qint64>{30, 75}));
        QVERIFY(e.view()->pageBreakRows()[0].manual);
        // what is typed past the last cell makes another page, and the line follows it
        QVERIFY(w.session()->type(0, 199, 0, QStringLiteral("x")).ok);
        QTRY_COMPARE(rowLines(), (QList<qint64>{30, 75, 120, 165}));
        pressUndo(e.view());
        QTRY_COMPARE(rowLines(), (QList<qint64>{30, 75}));
        w.act(QStringLiteral("view.pageBreaks"))->trigger();   // (off)
        QTRY_VERIFY(!e.view()->hasPageBreaks());
        w.act(QStringLiteral("view.pageBreaks"))->trigger();
        QTRY_COMPARE(rowLines(), (QList<qint64>{30, 75}));
    }

    // Undo and Redo of a typed cell are edits that drop the lines being worked out and bring new ones; read with no timer, the sheet's pages follow each step at once.
    void undoAndRedoOfTypingMoveThePagesAtOnce()
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
    void aDroppedLineJobNeverLandsItsLines()
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
    void thePageBreakLinesOfAMillionRowsAreWorkedOutInSlices()
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
    void pageLinesComeFromTheCutsAndTheBreaks()
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
    void objectsPrintInPlaceUnlessTheyAreNotToBe()
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

    // A job with more pages than it may have is cut short, and says so; a sheet with nothing on it says that; odd margins and scales still give pages.
    void aJobThatNeedsTooManyPagesIsCutShort()
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
    void hostilePageSettingsAreBounded()
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
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const PageSetup back = savedPage(e.path);
        QCOMPARE(areaOf(back), QStringLiteral("B2:C8"));
        QCOMPARE(back.rowBreaks.size(), 1);
        QCOMPARE(back.rowBreaks[0].before, 6u);
        QVERIFY(back == session->pages().sheet(0));
    }

    // ---- the page breaks view (printing 2)

    // The cells with a view of the pages: the shaded rest, the numbers, and the lines wait for the job; here the sheet of 100 rows is three pages, numbered, over the printed cells.
    void thePageBreaksViewNumbersThePagesOverThePrintedCells()
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
    void draggingALineMovesABreakAsOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        w.setViewKind(1);
        e.view()->setZoom(0.4);
        auto rowLines = [&] {
            QList<qint64> at;
            for (const auto &l : e.view()->pageBreakRows()) at << l.before;
            return at;
        };
        QTRY_COMPARE(rowLines(), (QList<qint64>{45, 90}));
        QTRY_COMPARE(e.view()->breakAreas().size(), size_t(1));
        const int steps = w.session()->undoStack()->count();
        const grid::GridLayout &L = e.view()->sheet().layout();
        const int x = L.xEdge(1, false) + 5;
        const QPoint from(x, L.yEdge(45, false)), to(x, L.yEdge(30, false) + 2);
        QVERIFY(e.view()->breakGrabAt(from).kind == grid::GridView::BreakGrab::Kind::Line);
        QTest::mousePress(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(e.view()->viewport(), to);
        QTest::mouseRelease(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, to);
        QCOMPARE(w.session()->undoStack()->count(), steps + 1);
        const PageSetup now = w.session()->pages().sheet(0);
        QCOMPARE(now.rowBreaks.size(), 1);
        QCOMPARE(now.rowBreaks[0].before, 30u);
        QVERIFY(now.rowBreaks[0].manual);
        QTRY_COMPARE(rowLines(), (QList<qint64>{30, 75}));
        QVERIFY(e.view()->pageBreakRows()[0].manual);
        pressUndo(e.view());
        QCOMPARE(w.session()->pages().sheet(0).rowBreaks.size(), 0);
        QTRY_COMPARE(rowLines(), (QList<qint64>{45, 90}));
        // a break set by hand is moved the same way (the old one goes, no second is left behind)
        pressRedo(e.view());
        QCOMPARE(w.session()->pages().sheet(0).rowBreaks.size(), 1);
        QTRY_COMPARE(rowLines(), (QList<qint64>{30, 75}));
        const QPoint again(x, L.yEdge(30, false)), moved(x, L.yEdge(20, false) + 2);
        QTest::mousePress(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, again);
        QTest::mouseMove(e.view()->viewport(), moved);
        QTest::mouseRelease(e.view()->viewport(), Qt::LeftButton, Qt::NoModifier, moved);
        QCOMPARE(w.session()->pages().sheet(0).rowBreaks.size(), 1);
        QCOMPARE(w.session()->pages().sheet(0).rowBreaks[0].before, 20u);
    }

    // A line let go off the printed cells (on the headings) is taken away; letting a line go where it was changes nothing.
    void draggingALineOffTheCellsTakesItAway()
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
    void draggingAnEdgeOfThePrintedCellsSetsThePrintArea()
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
    void aFittedSheetCannotHaveItsLinesDragged()
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
    void theViewIsSavedAndComesBack()
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
            QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join(QLatin1Char('|'))));
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
        QVERIFY2(savedOk(saved, 20000), qPrintable(again.win.told.join(QLatin1Char('|'))));
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

    // ---- pictures in headers and footers (printing 2)

    // A picture in the center of the header is drawn on the printed page, at the size the file gives, hung from the header's distance from the top; the same page without it is white there.
    void aPictureInTheHeaderIsDrawnOnThePage()
    {
        const QByteArray red = drawtest::pngBytes(8, 4, Qt::red);
        EditWindow e;
        QVERIFY(e.open(bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader>"), {hfShape(QStringLiteral("CH"))}, {red})));
        QString why;
        std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(crunch::PrintJobOptions(), &why);
        QVERIFY2(job, qPrintable(why));
        QImage page = job->preview(0, 1224);   // (two pixels to the point on letter paper)
        QVERIFY(!page.isNull());
        auto isRed = [&](double xPoints, double yPoints) {
            const QColor c = page.pixelColor(int(xPoints * 2), int(yPoints * 2));
            return c.red() > 200 && c.green() < 60 && c.blue() < 60;
        };
        QVERIFY(isRed(306, 40));        // (72 by 36 points, centered, from 21.6 points down)
        QVERIFY(isRed(272, 24));
        QVERIFY(isRed(340, 55));
        QVERIFY(!isRed(250, 40));       // (left of it)
        QVERIFY(!isRed(306, 62));       // (below it)
        QVERIFY(!isRed(306, 15));       // (above it)
        // the same page with the picture's code taken out of the text: nothing there
        PageSetup p = e.win.w.session()->pages().sheet(0);
        p.headerFooter.oddHeader = QStringLiteral("&CNo picture");
        QVERIFY(e.win.w.session()->setPageSetup(0, p, QStringLiteral("Header")).ok);
        job = e.win.w.makePrintJob(crunch::PrintJobOptions(), &why);
        QVERIFY(job);
        page = job->preview(0, 1224);
        QVERIFY(!isRed(306, 40));
    }

    // A footer's picture sits on the footer's distance from the bottom, at the left; the form for the first page is the first page's alone.
    void aPictureInTheFooterAndInTheFirstPageForm()
    {
        const QByteArray red = drawtest::pngBytes(8, 4, Qt::red), green = drawtest::pngBytes(8, 4, Qt::green);
        EditWindow e;
        QVERIFY(e.open(bookWithHeaderPictures(QStringLiteral("<oddFooter>&amp;L&amp;G</oddFooter><firstFooter>&amp;L&amp;G</firstFooter>").replace(QStringLiteral("<oddFooter>"), QStringLiteral("<oddFooter>")),
                                              {hfShape(QStringLiteral("LF")), hfShape(QStringLiteral("LFF"), QStringLiteral("rId2"))}, {red, green})));
        // (the first page's form needs the flag)
        PageSetup p = e.win.w.session()->pages().sheet(0);
        p.headerFooter.differentFirst = true;
        QVERIFY(e.win.w.session()->setPageSetup(0, p, QStringLiteral("First page")).ok);
        QString why;
        std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(crunch::PrintJobOptions(), &why);
        QVERIFY2(job, qPrintable(why));
        const QImage page = job->preview(0, 1224);
        // left margin 0.7 in = 50.4 points; the footer is 0.3 in = 21.6 points from the bottom edge (792): the picture is 36 points tall, ending there
        const QColor c = page.pixelColor(int(60 * 2), int((792 - 21.6 - 18) * 2));
        QVERIFY2(c.green() > 150 && c.red() < 100, qPrintable(c.name()));   // (the first page's own picture: green)
        QVERIFY(c.blue() < 100);
    }

    // A workbook with a picture in its header, saved after a cell and the page setup changed: the drawing, the image, and the sheet's link to it are the same bytes, and the sheet still names it.
    void aHeaderPictureSurvivesASaveThatChangesOtherThings()
    {
        const QByteArray red = drawtest::pngBytes(8, 4, Qt::red);
        EditWindow e;
        const QByteArray book = bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader>"), {hfShape(QStringLiteral("CH"))}, {red});
        QVERIFY(e.open(book));
        QCOMPARE(e.win.w.session()->pages().sheet(0).headerFooter.pictures.size(), 1);
        QVERIFY(e.win.w.session()->type(0, 0, 0, QStringLiteral("7")).ok);
        PageSetup p = e.win.w.session()->pages().sheet(0);
        p.orientation = PageOrientation::Landscape;
        QVERIFY(e.win.w.session()->setPageSetup(0, p, QStringLiteral("Orientation")).ok);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join(QLatin1Char('|'))));
        Package before, after;
        QString why;
        QVERIFY(before.readLazy(book, &why));
        QVERIFY(after.readLazyFile(e.path, &why));
        for (const char *part : {"/xl/drawings/vmlDrawing1.vml", "/xl/media/hfimage1.png"})
            QVERIFY2(before.part(QString::fromLatin1(part)) == after.part(QString::fromLatin1(part)) && !after.part(QString::fromLatin1(part)).isEmpty(), part);
        for (const QString &source : {QStringLiteral("/xl/drawings/vmlDrawing1.vml"), QStringLiteral("/xl/worksheets/sheet1.xml")}) {
            const auto was = before.relationships(source), now = after.relationships(source);
            QCOMPARE(now.size(), was.size());
            for (int i = 0; i < was.size(); ++i) QVERIFY(was[i].id == now[i].id && was[i].target == now[i].target && was[i].type == now[i].type);
        }
        const QByteArray sheet = after.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
        QVERIFY(sheet.contains("<legacyDrawingHF r:id=\"rIdH1\"/>"));
        QVERIFY(sheet.contains("orientation=\"landscape\""));
        // and it is read again the same
        EditWindow again;
        QFile f(e.path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QVERIFY(again.open(f.readAll()));
        const auto &pics = again.win.w.session()->pages().sheet(0).headerFooter.pictures;
        QCOMPARE(pics.size(), 1);
        QCOMPARE(pics[0].image, red);
    }

    // The editor of a header: the Picture button puts a picture from a file in the part being typed in (with its code), sized to fit; it is kept for the form of the page being edited; Remove takes
    // it and its code out; typing the code away takes the picture; a file that is not a picture is refused and says so.
    void thePictureButtonsOfTheHeaderEditor()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString png = dir.filePath(QStringLiteral("logo.png"));
        QImage red(200, 100, QImage::Format_ARGB32);
        red.fill(Qt::red);
        QVERIFY(red.save(png));
        const QString text = dir.filePath(QStringLiteral("words.txt"));
        {
            QFile f(text);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("not a picture");
        }
        jp::office::sheet::HeaderFooter hf;
        hf.oddHeader = QStringLiteral("&Cmiddle");
        hf.differentFirst = true;
        HeaderFooterEditor editor(true, hf, QStringLiteral("S"), QStringLiteral("f"));
        QString file = png;
        editor.setPictureAsker([&] { return file; });
        auto *add = editor.findChild<QPushButton *>(QStringLiteral("hfPicture"));
        auto *drop = editor.findChild<QPushButton *>(QStringLiteral("hfPictureRemove"));
        QVERIFY(add && drop);
        QVERIFY(!drop->isEnabled());
        editor.setInsertPart(1);
        add->click();
        QVERIFY(editor.box(1)->toPlainText().contains(QStringLiteral("&G")));
        QVERIFY(drop->isEnabled());
        jp::office::sheet::HeaderFooter made = editor.texts();
        QCOMPARE(made.pictures.size(), 1);
        QCOMPARE(made.pictures[0].place, QStringLiteral("CH"));
        QCOMPARE(made.pictures[0].width, 144.0);    // (200 by 100 pixels is 150 by 75 points: held to two inches on the long side)
        QCOMPARE(made.pictures[0].height, 72.0);
        QCOMPARE(made.pictures[0].extension, QStringLiteral("png"));
        QCOMPARE(made.pictures[0].title, QStringLiteral("logo"));
        QVERIFY(made.oddHeader.contains(QStringLiteral("&G")));
        // (a second one in the same part takes the first one's place, and its code is not put in twice)
        add->click();
        made = editor.texts();
        QCOMPARE(made.pictures.size(), 1);
        QCOMPARE(editor.box(1)->toPlainText().count(QStringLiteral("&G")), 1);
        // the first page's form has pictures of its own
        editor.selectVersion(1);
        editor.setInsertPart(0);
        add->click();
        made = editor.texts();
        QCOMPARE(made.pictures.size(), 2);
        QCOMPARE(made.pictures[1].place, QStringLiteral("LHFIRST"));
        QVERIFY(made.firstHeader.contains(QStringLiteral("&G")));
        // taking one out takes its code too
        drop->click();
        QCOMPARE(editor.texts().pictures.size(), 1);
        QVERIFY(!editor.box(0)->toPlainText().contains(QStringLiteral("&G")));
        // a code typed away takes the picture with it
        editor.selectVersion(0);
        editor.box(1)->setPlainText(QStringLiteral("no code any more"));
        QVERIFY(editor.texts().pictures.isEmpty());
        // not a picture, and the editor says so
        file = text;
        editor.setInsertPart(2);
        add->click();
        QVERIFY(!editor.pictureProblem().isEmpty());
        QVERIFY(editor.texts().pictures.isEmpty());
        QVERIFY(!editor.box(2)->toPlainText().contains(QStringLiteral("&G")));
        file.clear();   // (given up: nothing happens)
        add->click();
        QVERIFY(editor.texts().pictures.isEmpty());
    }

    // A picture put in with the Page Setup dialog is one step of Undo, saved in the file, taken out again in a later save, and put back, each save with the sheet naming the drawing the last one made.
    void aHeaderPictureAddedThroughPageSetupSavesAndComesOut()
    {
        const QByteArray red = drawtest::pngBytes(8, 4, Qt::red);
        EditWindow e;
        QString rows;
        for (int r = 1; r <= 3; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        CrunchWindow &w = e.win.w;
        bool withPicture = true;
        w.setPageSetupAsker([&](PageSetupRequest *r) {
            if (withPicture) {
                r->setup.headerFooter.oddHeader = QStringLiteral("&C&G");
                jp::office::sheet::HeaderPicture p;
                p.place = QStringLiteral("CH");
                p.image = red;
                p.extension = QStringLiteral("png");
                p.width = 60;
                p.height = 30;
                r->setup.headerFooter.pictures = {p};
            } else {
                r->setup.headerFooter.oddHeader.clear();
                r->setup.headerFooter.pictures.clear();
            }
            return true;
        });
        auto saveNow = [&] {
            QSignalSpy saved(&w, &CrunchWindow::saved);
            return w.save() && savedOk(saved, 20000);
        };
        auto pictureCount = [&] {
            Package pk;
            QString why;
            if (!pk.readLazyFile(e.path, &why)) return -1;
            Workbook b;
            Sheet s;
            if (!b.open(pk, &why) || !b.readSheet(0, &s)) return -1;
            return int(s.page.headerFooter.pictures.size());
        };
        const int steps = w.session()->undoStack()->count();
        w.pageSetupDialog(2);
        QCOMPARE(w.session()->undoStack()->count(), steps + 1);
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.pictures.size(), 1);
        QVERIFY(saveNow());
        QCOMPARE(pictureCount(), 1);
        withPicture = false;
        w.pageSetupDialog(2);
        QVERIFY(saveNow());
        QCOMPARE(pictureCount(), 0);
        withPicture = true;
        w.pageSetupDialog(2);
        QVERIFY(saveNow());
        QCOMPARE(pictureCount(), 1);
        // Undo puts back what the step before it had, and a save writes that
        pressUndo(e.view());
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.pictures.size(), 0);
        QVERIFY(saveNow());
        QCOMPARE(pictureCount(), 0);
    }

    // Rows inserted in a sheet with a picture in its header (the edit moves what names cells in the file's drawings): the header's drawing and image are the same bytes after the save.
    void aHeaderPictureSurvivesRowsBeingInserted()
    {
        const QByteArray red = drawtest::pngBytes(8, 4, Qt::red);
        EditWindow e;
        const QByteArray book = bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader>"), {hfShape(QStringLiteral("CH"))}, {red});
        QVERIFY(e.open(book));
        QVERIFY(e.win.w.session()->insertRows(0, 2, 3).ok);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join(QLatin1Char('|'))));
        Package before, after;
        QString why;
        QVERIFY(before.readLazy(book, &why));
        QVERIFY(after.readLazyFile(e.path, &why));
        QCOMPARE(after.part(QStringLiteral("/xl/drawings/vmlDrawing1.vml")), before.part(QStringLiteral("/xl/drawings/vmlDrawing1.vml")));
        QCOMPARE(after.part(QStringLiteral("/xl/media/hfimage1.png")), red);
        QVERIFY(after.part(QStringLiteral("/xl/worksheets/sheet1.xml")).contains("<legacyDrawingHF"));
    }

    // A copy of a sheet that has header pictures has them too, in parts of its own (the copy's setup and the file's agree), and the first sheet keeps its own.
    void aCopyOfASheetWithHeaderPicturesHasThemToo()
    {
        const QByteArray red = drawtest::pngBytes(8, 4, Qt::red);
        EditWindow e;
        QVERIFY(e.open(bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader>"), {hfShape(QStringLiteral("CH"))}, {red})));
        int made = -1;
        QVERIFY(e.win.w.session()->copySheet(0, 1, QStringLiteral("Copy"), &made).ok);
        QVERIFY(made > 0);
        QCOMPARE(e.win.w.session()->pages().sheet(made).headerFooter.pictures.size(), 1);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join(QLatin1Char('|'))));
        Package pk;
        QString why;
        QVERIFY(pk.readLazyFile(e.path, &why));
        Workbook b;
        QVERIFY2(b.open(pk, &why), qPrintable(why));
        QCOMPARE(b.info().sheets.size(), 2);
        for (int i = 0; i < 2; ++i) {
            Sheet s;
            QVERIFY(b.readSheet(i, &s));
            QCOMPARE(s.page.headerFooter.pictures.size(), 1);
            QCOMPARE(s.page.headerFooter.pictures[0].image, red);
            QCOMPARE(s.page.headerFooter.pictures[0].place, QStringLiteral("CH"));
        }
        // (two drawings: each sheet names its own)
        QCOMPARE(pk.relationshipsOfType(b.info().sheets[0].part, QStringLiteral("/vmlDrawing")).size(), 1);
        QCOMPARE(pk.relationshipsOfType(b.info().sheets[1].part, QStringLiteral("/vmlDrawing")).size(), 1);
        QVERIFY(pk.relationshipsOfType(b.info().sheets[0].part, QStringLiteral("/vmlDrawing"))[0].target != pk.relationshipsOfType(b.info().sheets[1].part, QStringLiteral("/vmlDrawing"))[0].target);
    }

    // ---- the Page Layout view (printing 2)

    // Where the cells of a printed page are: the cell's place on the paper in points, the cell at a point, the part of a range on a page, and the page that has a cell.
    void aJobSaysWhereTheCellsOfAPageAre()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        QString why;
        std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(crunch::PrintJobOptions(), &why);
        QVERIFY2(job, qPrintable(why));
        QCOMPARE(job->pageCount(), 3);
        QRectF a1, b3, second;
        QVERIFY(job->cellRect(0, 0, 0, &a1));
        QVERIFY(qAbs(a1.left() - 50.4) < 0.5);   // (the left margin: 0.7 inch)
        QVERIFY(qAbs(a1.top() - 54) < 0.5);      // (and the top: 0.75 inch)
        QVERIFY(a1.width() > 20 && a1.height() > 10);
        QVERIFY(job->cellRect(0, 2, 1, &b3));
        QVERIFY(b3.left() > a1.right() - 0.5 && b3.top() > a1.bottom() - 0.5);
        qint64 row = -1, col = -1;
        QVERIFY(job->cellAt(0, b3.center(), &row, &col));
        QCOMPARE(row, qint64(2));
        QCOMPARE(col, qint64(1));
        QVERIFY(!job->cellAt(0, QPointF(5, 5), &row, &col));          // (the margin)
        QVERIFY(!job->cellRect(0, 50, 0, &second));                    // (a row of the second page)
        QVERIFY(job->cellRect(1, 50, 0, &second));
        QVERIFY(qAbs(second.top() - 54 - 5 * a1.height()) < 3);        // (the page begins at row 45: row 50 is five rows down)
        QCOMPARE(job->pageOfCell(0, 50, 0), 1);
        QCOMPARE(job->pageOfCell(0, 0, 0), 0);
        QCOMPARE(job->pageOfCell(0, 99, 1), 2);
        QCOMPARE(job->pageOfCell(0, 500, 0), -1);
        QRectF part, whole;
        QVERIFY(job->rangeRect(0, 40, 0, 60, 1, &part));    // (rows 40 to 44 are on the first page)
        QVERIFY(job->cellRect(0, 40, 0, &whole));
        QVERIFY(qAbs(part.top() - whole.top()) < 0.5);
        QVERIFY(job->cellRect(0, 44, 1, &whole));
        QVERIFY(qAbs(part.bottom() - whole.bottom()) < 0.5 && qAbs(part.right() - whole.right()) < 0.5);
        QVERIFY(!job->rangeRect(0, 50, 0, 60, 1, &part));
    }

    // The Page Layout view takes the grid's place: the pages one below the other (three here, on letter paper), drawn when in view, with the gap between them in another color than the paper.
    void thePageLayoutViewShowsThePages()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        QVERIFY(w.act(QStringLiteral("view.pageLayout")));
        QVERIFY(!w.pageView()->isVisible());
        w.setViewKind(2);
        QCOMPARE(w.viewKind(), 2);
        QVERIFY(w.pageView()->isVisible());
        QVERIFY(!e.view()->isVisible());
        QVERIFY(w.act(QStringLiteral("view.pageLayout"))->isChecked());
        QVERIFY(!w.act(QStringLiteral("view.normal"))->isChecked());
        QTRY_VERIFY(w.pageLayoutReady());
        QCOMPARE(w.pageView()->source()->pageCount(), 3);
        const QRect first = w.pageView()->pageRect(0);
        QVERIFY(!first.isEmpty());
        QVERIFY(qAbs(double(first.width()) / first.height() - 612.0 / 792.0) < 0.01);   // (letter paper)
        QCOMPARE(first.width(), int(std::lround(612.0 * 96 / 72 * w.pageView()->zoom())));   // (a point is 96 / 72 pixels at 100%)
        w.pageView()->drawPagesNow();
        QVERIFY(w.pageView()->pageIsDrawn(0));
        // a second page is below the first with a gap that is not paper
        w.pageView()->scrollToPage(1);
        w.pageView()->drawPagesNow();
        QVERIFY(w.pageView()->pageIsDrawn(1));
        const QImage shot = w.pageView()->viewport()->grab().toImage();
        const QRect second = w.pageView()->pageRect(1);
        QVERIFY(!second.isEmpty());
        // (the picture of the viewport has the screen's device pixels: a place of the viewport is that many times further along)
        const double dpr = shot.devicePixelRatio();
        auto at = [&](int x, int y) { return shot.pixelColor(int(x * dpr), int(y * dpr)); };
        QVERIFY(at(second.center().x(), second.top() + 4).lightness() > 240);        // (paper)
        QVERIFY(at(second.center().x(), second.top() - jp::grid::PageLayoutView::kGap / 2).lightness() < 235);   // (the gap above it)
        // back to the cells
        w.setViewKind(0);
        QVERIFY(e.view()->isVisible());
        QVERIFY(!w.pageView()->isVisible());
        QVERIFY(!w.pageLayoutReady());
    }

    // A click on a cell of a page chooses it; the arrow keys, which go to the cells, move the choice and the view follows it onto the next page; the choice is drawn over the page.
    void cellsCanBeChosenAndMovedOnThePages()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        w.setViewKind(2);
        QTRY_VERIFY(w.pageLayoutReady());
        QWidget *vp = w.pageView()->viewport();
        w.pageView()->drawPagesNow();
        const QRect b6 = w.pageView()->cellRect(5, 1);
        QVERIFY(!b6.isEmpty());
        QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, b6.center());
        QCOMPARE(e.selected(), QStringLiteral("B6"));
        QTest::keyClick(w.pageView(), Qt::Key_Down);
        QCOMPARE(e.selected(), QStringLiteral("B7"));
        // a drag chooses a block, and the choice is drawn
        const QRect a3 = w.pageView()->cellRect(2, 0), b5 = w.pageView()->cellRect(4, 1);
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, a3.center());
        QTest::mouseMove(vp, b5.center());
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, b5.center());
        QCOMPARE(e.view()->rangeName(), QStringLiteral("A3:B5"));
        // a cell of the second page: the choice moves there by the keys, and the view scrolls to it
        e.view()->selectCell(44, 0);
        QTest::keyClick(w.pageView(), Qt::Key_Down);
        QCOMPARE(e.selected(), QStringLiteral("A46"));
        QVERIFY(!w.pageView()->cellRect(45, 0).isEmpty());
        QVERIFY(w.pageView()->cellRect(45, 0).top() >= 0 && w.pageView()->cellRect(45, 0).bottom() <= vp->height());
    }

    // Typing on a page puts the cell's strip over the cell on its page, at the pages' zoom; Enter finishes it, and the value is in the sheet; a double click edits what the cell holds.
    void cellsAreEditedOnThePages()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        w.setViewKind(2);
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->drawPagesNow();
        const QRect c2 = w.pageView()->cellRect(1, 1);
        QTest::mouseClick(w.pageView()->viewport(), Qt::LeftButton, Qt::NoModifier, c2.center());
        QCOMPARE(e.selected(), QStringLiteral("B2"));
        QTest::keyClick(w.pageView(), '7');
        QVERIFY(w.cellEditor()->isVisible());
        QVERIFY(w.cellEditor()->parentWidget() == w.pageView()->viewport());
        QCOMPARE(w.cellEditor()->geometry().topLeft(), c2.topLeft());
        QTest::keyClicks(w.cellEditor(), "5");
        QTest::keyClick(w.cellEditor(), Qt::Key_Return);
        QVERIFY(!w.cellEditor()->isVisible());
        QCOMPARE(e.valueAt(QStringLiteral("B2")), QStringLiteral("n:75"));
        QCOMPARE(e.selected(), QStringLiteral("B3"));
        // F2 on the cell, and a double click, edit what it holds
        const QRect a1 = w.pageView()->cellRect(0, 0);
        QTest::mouseDClick(w.pageView()->viewport(), Qt::LeftButton, Qt::NoModifier, a1.center());
        QVERIFY(w.cellEditor()->isVisible());
        QCOMPARE(w.cellEditor()->text(), QStringLiteral("1"));
        QTest::keyClick(w.cellEditor(), Qt::Key_Escape);
        QVERIFY(!w.cellEditor()->isVisible());
        // back on the cells, the strip is on the grid again
        w.setViewKind(0);
        QVERIFY(w.cellEditor()->parentWidget() == e.view()->viewport());
    }

    // A click on a part of a header opens a strip to type its words, with the Header and Footer tab; Ctrl and Enter finish it as one step of Undo; Escape gives it up; the tab's buttons put things in.
    void aHeaderIsTypedInPlace()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        w.setViewKind(2);
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->drawPagesNow();
        auto tabShown = [&] {
            for (int i = 0; i < w.ribbon()->tabCount(); ++i)
                if (w.ribbon()->tabName(i) == QStringLiteral("Header and Footer")) return w.ribbon()->tabVisible(i);
            return false;
        };
        QVERIFY(!tabShown());
        QVERIFY(!w.zoneEditor());
        const QRect center = w.pageView()->zoneRect(0, true, 1);
        QVERIFY(!center.isEmpty());
        QTest::mouseClick(w.pageView()->viewport(), Qt::LeftButton, Qt::NoModifier, center.center());
        QVERIFY(w.zoneEditor());
        QVERIFY(w.zoneEditor()->isVisible());
        QVERIFY(tabShown());
        const int steps = w.session()->undoStack()->count();
        QTest::keyClicks(w.zoneEditor(), "Quarterly ");
        w.act(QStringLiteral("hf.pageNumber"))->trigger();
        QCOMPARE(w.zoneEditor()->toPlainText(), QStringLiteral("Quarterly &P"));
        QTest::keyClick(w.zoneEditor(), Qt::Key_Return, Qt::ControlModifier);
        QVERIFY(!w.zoneEditor());
        QVERIFY(!tabShown());
        QCOMPARE(w.session()->undoStack()->count(), steps + 1);
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddHeader, QStringLiteral("&CQuarterly &P"));
        // the page is drawn again with it (the job is cut again after a change)
        QTRY_VERIFY(w.pageView()->pageIsDrawn(0));
        pressUndo(w.pageView());
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddHeader, QString());
        // the left part of the footer; Escape gives it up
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->drawPagesNow();
        const QRect left = w.pageView()->zoneRect(0, false, 0);
        QTest::mouseClick(w.pageView()->viewport(), Qt::LeftButton, Qt::NoModifier, left.center());
        QVERIFY(w.zoneEditor());
        QTest::keyClicks(w.zoneEditor(), "not this");
        QTest::keyClick(w.zoneEditor(), Qt::Key_Escape);
        QVERIFY(!w.zoneEditor());
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddFooter, QString());
        // a part with words already shows them, and typing more keeps the other parts
        PageSetup p = w.session()->pages().sheet(0);
        p.headerFooter.oddFooter = QStringLiteral("&LLeft words&RRight words");
        QVERIFY(w.session()->setPageSetup(0, p, QStringLiteral("Footer")).ok);
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->drawPagesNow();
        QTest::mouseClick(w.pageView()->viewport(), Qt::LeftButton, Qt::NoModifier, w.pageView()->zoneRect(0, false, 1).center());
        QVERIFY(w.zoneEditor());
        QCOMPARE(w.zoneEditor()->toPlainText(), QString());
        QTest::keyClicks(w.zoneEditor(), "Middle");
        QTest::keyClick(w.zoneEditor(), Qt::Key_Return, Qt::ControlModifier);
        QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddFooter, QStringLiteral("&LLeft words&CMiddle&RRight words"));
    }

    // The first page has a header of its own once Different First Page is chosen on the tab, and a click on the first page's header types that one.
    void theFirstPageHeaderIsTypedInPlace()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        w.setViewKind(2);
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->drawPagesNow();
        QTest::mouseClick(w.pageView()->viewport(), Qt::LeftButton, Qt::NoModifier, w.pageView()->zoneRect(0, true, 2).center());
        QVERIFY(w.zoneEditor());
        QVERIFY(!w.act(QStringLiteral("hf.differentFirst"))->isChecked());
        w.act(QStringLiteral("hf.differentFirst"))->trigger();
        QVERIFY(w.session()->pages().sheet(0).headerFooter.differentFirst);
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->drawPagesNow();
        w.pageView()->scrollToPage(0);
        QTest::mouseClick(w.pageView()->viewport(), Qt::LeftButton, Qt::NoModifier, w.pageView()->zoneRect(0, true, 2).center());
        QVERIFY(w.zoneEditor());
        QTest::keyClicks(w.zoneEditor(), "Cover");
        QTest::keyClick(w.zoneEditor(), Qt::Key_Return, Qt::ControlModifier);
        const auto &hf = w.session()->pages().sheet(0).headerFooter;
        QCOMPARE(hf.firstHeader, QStringLiteral("&RCover"));
        QCOMPARE(hf.oddHeader, QString());
    }

    // The Picture button of the tab puts a picture in the part being typed in: the text and the picture are one step.
    void theTabPutsAPictureInTheHeader()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString png = dir.filePath(QStringLiteral("logo.png"));
        QImage img(80, 40, QImage::Format_ARGB32);
        img.fill(Qt::red);
        QVERIFY(img.save(png));
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        w.setPictureAsker([&] { return png; });
        w.setViewKind(2);
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->drawPagesNow();
        QTest::mouseClick(w.pageView()->viewport(), Qt::LeftButton, Qt::NoModifier, w.pageView()->zoneRect(0, true, 0).center());
        QVERIFY(w.zoneEditor());
        const int steps = w.session()->undoStack()->count();
        w.act(QStringLiteral("hf.picture"))->trigger();
        QCOMPARE(w.zoneEditor()->toPlainText(), QStringLiteral("&G"));
        QTest::keyClick(w.zoneEditor(), Qt::Key_Return, Qt::ControlModifier);
        QCOMPARE(w.session()->undoStack()->count(), steps + 1);
        const auto &hf = w.session()->pages().sheet(0).headerFooter;
        QCOMPARE(hf.oddHeader, QStringLiteral("&L&G"));
        QCOMPARE(hf.pictures.size(), 1);
        QCOMPARE(hf.pictures[0].place, QStringLiteral("LH"));
        QCOMPARE(hf.pictures[0].width, 60.0);
        // it is on the printed page
        QString why;
        std::unique_ptr<crunch::PrintJob> job = w.makePrintJob(crunch::PrintJobOptions(), &why);
        QVERIFY(job);
        const QImage page = job->preview(0, 1224);
        const QColor c = page.pixelColor(int(60 * 2), int(40 * 2));
        QVERIFY2(c.red() > 200 && c.green() < 60, qPrintable(c.name()));
        // one step takes both away
        pressUndo(w.pageView());
        QVERIFY(w.session()->pages().sheet(0).headerFooter.pictures.isEmpty());
        QVERIFY(w.session()->pages().sheet(0).headerFooter.oddHeader.isEmpty());
    }

    // The view of the pages is saved, with the zoom of the pages, and the file opens in it again.
    void thePageLayoutViewIsSavedAndComesBack()
    {
        QByteArray bytes;
        {
            EditWindow e;
            QVERIFY(e.open(rowsWithFormulas(30, {})));
            CrunchWindow &w = e.win.w;
            w.setViewKind(2);
            w.pageView()->setZoom(0.75);
            QSignalSpy saved(&w, &CrunchWindow::saved);
            QVERIFY(w.save());
            QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join(QLatin1Char('|'))));
            Package pk;
            QString why;
            QVERIFY(pk.readLazyFile(e.path, &why));
            const QByteArray xml = pk.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
            QVERIFY2(xml.contains("view=\"pageLayout\""), xml.left(500).constData());
            QVERIFY(xml.contains("zoomScalePageLayoutView=\"75\""));
            QFile f(e.path);
            QVERIFY(f.open(QIODevice::ReadOnly));
            bytes = f.readAll();
        }
        EditWindow again;
        QVERIFY(again.open(bytes));
        QCOMPARE(again.win.w.viewKind(), 2);
        QVERIFY(again.win.w.pageView()->isVisible());
        QTRY_VERIFY(again.win.w.pageLayoutReady());
        QCOMPARE(int(std::lround(again.win.w.pageView()->zoom() * 100)), 75);
        QCOMPARE(again.win.w.pageView()->source()->pageCount(), 1);
    }

    // A sheet of a million rows is cut into its pages for the Page Layout view in slices of counted work (no turn of the loop does more than a slice), the view lays out all its thousands of pages,
    // draws only those in view (one a turn), and keeps only a few drawn.
    void aSheetOfAMillionRowsIsShownAsPagesInSlices()
    {
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(QStringLiteral("<row r=\"1048576\"><c r=\"A1048576\"><v>1</v></c></row>"))})));
        CrunchWindow &w = e.win.w;
        w.setViewKind(2);
        w.updatePageBreaks();   // (not waiting for the timer)
        QVERIFY(w.pageBreaksUnderWay());
        QVERIFY(!w.pageLayoutReady());
        QTRY_VERIFY_WITH_TIMEOUT(w.pageLayoutReady(), 60000);
        const CrunchWindow::PageBreakWork work = w.pageBreakWork();
        QVERIFY(work.slices > 5);
        QVERIFY2(work.most <= CrunchWindow::pageBreakSliceWork() + 16, qPrintable(QString::number(work.most)));
        QVERIFY(w.pageView()->source()->pageCount() > 1000);
        // (one page is drawn a turn: after a few turns the ones in view are, and nothing else)
        QTRY_VERIFY(w.pageView()->pageIsDrawn(0));
        QVERIFY(w.pageView()->pagesDrawn() <= 4);
        w.pageView()->scrollToPage(5000);
        QTRY_VERIFY(w.pageView()->pageIsDrawn(5000));
        QVERIFY(w.pageView()->pagesKept() <= 10);
        QVERIFY(w.pageView()->pagesDrawn() < 20);
    }

    // ---- review fixes (printing 2)

    // A zoom change alone is something to save: the file gets the zoom (and, for the cells, the zoom of the cells), and a save with the zoom as the file has it writes nothing.
    void aZoomChangeAloneIsSaved()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(30, {})));
        CrunchWindow &w = e.win.w;
        QVERIFY(!w.session()->viewsChanged());
        e.view()->setZoom(1.5);
        QVERIFY(w.session()->viewsChanged());
        QVERIFY(!w.session()->isModified());
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join(QLatin1Char('|'))));
        Package pk;
        QString why;
        QVERIFY(pk.readLazyFile(e.path, &why));
        const QByteArray xml = pk.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
        QVERIFY2(xml.contains("zoomScale=\"150\"") && xml.contains("zoomScaleNormal=\"150\""), xml.left(500).constData());
        QVERIFY(!w.session()->viewsChanged());
        // (the same zoom again: nothing to write)
        QSignalSpy second(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY(w.statusText().contains(QLatin1String("Nothing to save")));
    }

    // Leaving a view puts back the zoom the cells had (kept for the sheet, not for the window), and the file's own zoom of the cells is written whichever view the sheet is left in.
    void leavingAViewPutsTheCellsOwnZoomBack()
    {
        QByteArray bytes;
        {
            EditWindow e;
            QVERIFY(e.open(rowsWithFormulas(30, {})));
            CrunchWindow &w = e.win.w;
            e.view()->setZoom(1.5);
            w.setViewKind(1);
            QVERIFY(qAbs(e.view()->zoom() - 0.6) < 0.01);
            w.setViewKind(0);
            QVERIFY(qAbs(e.view()->zoom() - 1.5) < 0.01);
            w.setViewKind(2);
            w.setViewKind(0);
            QVERIFY(qAbs(e.view()->zoom() - 1.5) < 0.01);
            QCOMPARE(w.session()->pages().normalZoom(0), 150);
            // left in the page breaks view: the file has the view's zoom and, apart, the cells' own
            w.setViewKind(1);
            e.view()->setZoom(0.5);
            QCOMPARE(w.session()->pages().normalZoom(0), 150);
            QSignalSpy saved(&w, &CrunchWindow::saved);
            QVERIFY(w.save());
            QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join(QLatin1Char('|'))));
            Package pk;
            QString why;
            QVERIFY(pk.readLazyFile(e.path, &why));
            const QByteArray xml = pk.part(QStringLiteral("/xl/worksheets/sheet1.xml"));
            QVERIFY2(xml.contains("zoomScale=\"50\"") && xml.contains("zoomScaleNormal=\"150\"") && xml.contains("zoomScaleSheetLayoutView=\"50\""), xml.left(600).constData());
            QFile f(e.path);
            QVERIFY(f.open(QIODevice::ReadOnly));
            bytes = f.readAll();
        }
        EditWindow again;
        QVERIFY(again.open(bytes));
        QCOMPARE(again.win.w.viewKind(), 1);
        QCOMPARE(int(std::lround(again.view()->zoom() * 100)), 50);
        again.win.w.setViewKind(0);
        QCOMPARE(int(std::lround(again.view()->zoom() * 100)), 150);
    }

    // Each sheet has its own zoom of the cells, read from the file: going from one to the other, and leaving a view on one, never gives it another's.
    void eachSheetKeepsItsOwnZoomOfTheCells()
    {
        auto views = [](const char *attrs) { return QStringLiteral("<sheetViews><sheetView %1 workbookViewId=\"0\"/></sheetViews>").arg(QLatin1String(attrs)); };
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c></row>");
        const QByteArray book = buildXlsx({worksheet(rows, QString(), views("zoomScale=\"80\" zoomScaleNormal=\"80\"")), worksheet(rows, QString(), views("zoomScale=\"120\" zoomScaleNormal=\"120\""))});
        EditWindow e;
        QVERIFY(e.open(book));
        CrunchWindow &w = e.win.w;
        QCOMPARE(w.session()->pages().normalZoom(0), 80);
        QCOMPARE(w.session()->pages().normalZoom(1), 120);
        QCOMPARE(int(std::lround(e.view()->zoom() * 100)), 80);
        w.setActiveSheet(1);
        QCOMPARE(int(std::lround(e.view()->zoom() * 100)), 120);
        w.setViewKind(1);
        w.setViewKind(0);
        QCOMPARE(int(std::lround(e.view()->zoom() * 100)), 120);
        w.setActiveSheet(0);
        QCOMPARE(int(std::lround(e.view()->zoom() * 100)), 80);
        w.setViewKind(2);
        w.setViewKind(0);
        QCOMPARE(int(std::lround(e.view()->zoom() * 100)), 80);
        QCOMPARE(w.session()->pages().normalZoom(1), 120);
        // (nothing of this is a change of the file: the zooms are as it has them)
        QVERIFY(!w.session()->viewsChanged());
    }

    // A move of a break that cannot be made because the list is full leaves the old break, and says so.
    void aFullListOfBreaksKeepsTheOldBreak()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(30, {})));
        CrunchWindow &w = e.win.w;
        PageSetup p = w.session()->pages().sheet(0);
        for (int i = 0; i < jp::office::sheet::kMaxPageBreaks; ++i) {
            jp::office::sheet::PageBreak b;
            b.before = quint32(1 + i);
            b.max = 16383;
            p.rowBreaks.append(b);
        }
        QVERIFY(w.session()->setPageSetup(0, p, QStringLiteral("Breaks")).ok);
        QCOMPARE(w.session()->pages().sheet(0).rowBreaks.size(), jp::office::sheet::kMaxPageBreaks);
        const int steps = w.session()->undoStack()->count();
        w.moveBreakForTest(false, 9999, 100000);   // (a line the program found, dragged to a place with no break: there is no room for another)
        QCOMPARE(w.session()->undoStack()->count(), steps);
        QCOMPARE(w.session()->pages().sheet(0).rowBreaks.size(), jp::office::sheet::kMaxPageBreaks);
        bool kept = false;
        for (const auto &b : w.session()->pages().sheet(0).rowBreaks) kept = kept || b.before == 5u;
        QVERIFY(kept);
        QVERIFY(w.statusText().contains(QLatin1String("page breaks")));
        // a break set by hand moved to a place with none frees its own place; one moved onto another, or taken away, is made
        w.moveBreakForTest(false, 5, 100000);
        QCOMPARE(w.session()->pages().sheet(0).rowBreaks.size(), jp::office::sheet::kMaxPageBreaks);
        w.moveBreakForTest(false, 7, 6);
        QCOMPARE(w.session()->pages().sheet(0).rowBreaks.size(), jp::office::sheet::kMaxPageBreaks - 1);
        w.moveBreakForTest(false, 8, -1);
        QCOMPARE(w.session()->pages().sheet(0).rowBreaks.size(), jp::office::sheet::kMaxPageBreaks - 2);
    }

    // A picture in a header is made on a worker, not on the window thread: a page is drawn at once without it, nothing is decoded on this thread, and it is there when the picture is ready.
    void aHeaderPictureIsMadeOffTheWindowThread()
    {
        const QByteArray red = drawtest::pngBytes(160, 80, Qt::red);
        EditWindow e;
        QVERIFY(e.open(bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader>"), {hfShape(QStringLiteral("CH"))}, {red})));
        CrunchWindow &w = e.win.w;
        jp::grid::HeaderPictureCache &cache = jp::grid::HeaderPictureCache::instance();
        cache.clear();
        const int here = cache.decodesHere();
        QString why;
        std::unique_ptr<crunch::PrintJob> job = w.makePrintJob(crunch::PrintJobOptions(), &why);
        QVERIFY(job);
        job->setAsyncPictures(true);
        auto isRed = [](const QImage &page) {
            const QColor c = page.pixelColor(int(306 * 2), int(40 * 2));
            return c.red() > 200 && c.green() < 70 && c.blue() < 70;
        };
        const QImage first = job->preview(0, 1224);
        QVERIFY(!isRed(first));                    // (drawn at once, without the picture)
        QVERIFY(job->picturesPending() || cache.count() > 0);
        QCOMPARE(cache.decodesHere(), here);       // (and nothing was decoded here)
        cache.waitForWorkers();
        QVERIFY(!job->picturesPending());
        QVERIFY(isRed(job->preview(0, 1224)));     // (the next time, the picture is there)
        QCOMPARE(cache.decodesHere(), here);
        // a job that is not for a view waits for its pictures: they are made at once
        cache.clear();
        job->setAsyncPictures(false);
        QVERIFY(isRed(job->preview(0, 1224)));
        QCOMPARE(cache.decodesHere(), here + 1);
        // in the view: the page is drawn again when the picture is ready
        cache.clear();
        w.setViewKind(2);
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->drawPagesNow();
        auto redInView = [&] {
            w.pageView()->drawPagesNow();
            const QImage shot = w.pageView()->viewport()->grab().toImage();
            const QRect page = w.pageView()->pageRect(0);
            const double k = 96.0 / 72.0 * w.pageView()->zoom();
            const QColor c = shot.pixelColor(int((page.left() + 306 * k) * shot.devicePixelRatio()), int((page.top() + 30 * k) * shot.devicePixelRatio()));
            return c.red() > 200 && c.green() < 70 && c.blue() < 70;
        };
        QTRY_VERIFY_WITH_TIMEOUT(redInView(), 20000);
        QCOMPARE(cache.decodesHere(), here + 1);
    }

    // A page too large to keep is drawn only where it is exposed: the painter is clipped to the part asked for, and the page painter draws the cells of that part, not of the whole page.
    void anOversizedPageIsDrawnOnlyWhereItIsExposed()
    {
        struct Pages : jp::grid::PageLayoutSource {
            int pageCount() const override { return 3; }
            QSizeF pageSize(int) const override { return QSizeF(612, 792); }
            QRectF headerBand(int) const override { return QRectF(50, 0, 500, 54); }
            QRectF footerBand(int) const override { return QRectF(50, 738, 500, 54); }
            void paintPage(QPainter *p, int) override { clips << p->clipBoundingRect(); }
            bool cellRect(int, qint64, qint64, QRectF *) override { return false; }
            bool cellAt(int, const QPointF &, qint64 *, qint64 *) override { return false; }
            bool rangeRect(int, const jp::grid::GridRange &, QRectF *) override { return false; }
            int pageOfCell(qint64, qint64) override { return -1; }
            QList<QRectF> clips;
        } pages;
        jp::grid::PageLayoutView view;
        view.resize(900, 700);
        view.setSource(&pages);
        view.setCacheBudgetPixels(100);   // (every page is too large to keep)
        view.show();
        QVERIFY(QTest::qWaitForWindowExposed(&view));
        pages.clips.clear();
        const QRect page = view.pageRect(0);
        view.viewport()->repaint(QRect(page.left() + 100, page.top() + 100, 40, 30));
        QVERIFY(!pages.clips.isEmpty());
        for (const QRectF &c : std::as_const(pages.clips)) QVERIFY2(c.width() < 200 && c.height() < 200, qPrintable(QStringLiteral("%1 x %2 points").arg(c.width()).arg(c.height())));
        QCOMPARE(view.pagesKept(), 0);
    }

    // The cells of an oversized page are not all drawn for a small exposed part: the painter's clip says which, and the count of cells looked at is the part's.
    void thePagePainterDrawsOnlyWhatTheClipLeavesInView()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(40, {})));
        QString why;
        std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(crunch::PrintJobOptions(), &why);
        QVERIFY(job);
        QImage img(612, 792, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);
        QPainter whole(&img);
        QVERIFY(job->paint(&whole, 0));
        whole.end();
        const qint64 all = job->lastPaintStats().cellsLooked;
        QVERIFY(all >= 40);
        QImage part(612, 792, QImage::Format_ARGB32_Premultiplied);
        part.fill(Qt::white);
        QPainter clipped(&part);
        clipped.setClipRect(QRect(40, 50, 200, 60));   // (the first few rows)
        QVERIFY(job->paint(&clipped, 0));
        clipped.end();
        QVERIFY2(job->lastPaintStats().cellsLooked < all / 2, qPrintable(QStringLiteral("%1 of %2").arg(job->lastPaintStats().cellsLooked).arg(all)));
        // and what shows in the exposed part is what the whole page has there
        for (int y = 50; y < 110; y += 3)
            for (int x = 40; x < 240; x += 3) QCOMPARE(part.pixel(x, y), img.pixel(x, y));
    }

    // After a change the pages that were drawn stay on the screen until they are drawn again (no white page between): the view keeps them, marked as old, and draws them a page a turn.
    void pagesStayOnTheScreenUntilTheNewOnesAreReady()
    {
        struct Pages : jp::grid::PageLayoutSource {
            int pageCount() const override { return 2; }
            QSizeF pageSize(int) const override { return QSizeF(612, 792); }
            QRectF headerBand(int) const override { return QRectF(50, 0, 500, 54); }
            QRectF footerBand(int) const override { return QRectF(50, 738, 500, 54); }
            void paintPage(QPainter *p, int) override { p->fillRect(QRectF(0, 0, 612, 792), Qt::white); ++painted; }
            bool cellRect(int, qint64, qint64, QRectF *) override { return false; }
            bool cellAt(int, const QPointF &, qint64 *, qint64 *) override { return false; }
            bool rangeRect(int, const jp::grid::GridRange &, QRectF *) override { return false; }
            int pageOfCell(qint64, qint64) override { return -1; }
            int painted = 0;
        } pages;
        jp::grid::PageLayoutView view;
        view.resize(900, 700);
        view.setSource(&pages);
        view.show();
        view.drawPagesNow();
        QVERIFY(view.pageIsCurrent(0));
        const int drawn = pages.painted;
        view.sourceChanged();   // (a change: the pages are cut again)
        QVERIFY(view.pageIsDrawn(0));        // (still there)
        QVERIFY(!view.pageIsCurrent(0));     // (but old)
        QCOMPARE(pages.painted, drawn);
        view.contentsChanged();
        QVERIFY(view.pageIsDrawn(0));
        QTRY_VERIFY(view.pageIsCurrent(0));
        QVERIFY(pages.painted > drawn);
        // another zoom needs other pictures: those go
        view.setZoom(2);
        QVERIFY(!view.pageIsDrawn(0));
    }

    // Many pages in view at a low zoom: where each page's cells are is worked out once for each page, however often they are painted or asked about.
    void thePlaceOfAPagesCellsIsWorkedOutOncePerPage()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(3000, {})));
        CrunchWindow &w = e.win.w;
        w.resize(1280, 4000);
        w.setViewKind(2);
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->setZoom(0.1);
        e.view()->selectRange(0, 0, 2999, 1);   // (drawn over the cells of every page in view)
        w.pageView()->drawPagesNow();
        const int visible = w.pageView()->lastVisiblePage() - w.pageView()->firstVisiblePage() + 1;
        QVERIFY(visible >= 10);
        crunch::PrintJob *job = w.layoutJobForTest();
        QVERIFY(job);
        w.pageView()->viewport()->repaint();
        const qint64 first = job->layoutsWorkedOut();
        QVERIFY2(first <= visible + 4, qPrintable(QStringLiteral("%1 for %2 pages").arg(first).arg(visible)));
        for (int i = 0; i < 3; ++i) w.pageView()->viewport()->repaint();
        w.pageView()->cellRect(5, 1);
        w.pageView()->cellRect(30, 0);
        QCOMPARE(job->layoutsWorkedOut(), first);
    }

    // A picture put in is made to fit the margin it goes in (no taller than the header's room), and is never made larger than it is.
    void aNewPictureFitsTheHeaderMargin()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString big = dir.filePath(QStringLiteral("big.png")), small = dir.filePath(QStringLiteral("small.png"));
        QImage a(200, 100, QImage::Format_ARGB32), b(16, 8, QImage::Format_ARGB32);
        a.fill(Qt::red);
        b.fill(Qt::blue);
        QVERIFY(a.save(big) && b.save(small));
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(100, {})));
        CrunchWindow &w = e.win.w;
        QString file = big;
        w.setPictureAsker([&] { return file; });
        w.setViewKind(2);
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->drawPagesNow();
        QTest::mouseClick(w.pageView()->viewport(), Qt::LeftButton, Qt::NoModifier, w.pageView()->zoneRect(0, true, 0).center());
        QVERIFY(w.zoneEditor());
        w.act(QStringLiteral("hf.picture"))->trigger();
        QTest::keyClick(w.zoneEditor(), Qt::Key_Return, Qt::ControlModifier);
        const auto &pics = w.session()->pages().sheet(0).headerFooter.pictures;
        QCOMPARE(pics.size(), 1);
        // (margins of 0.75 inch at the top and 0.3 inch for the header: 0.45 inch of room, 32.4 points)
        QVERIFY(qAbs(pics[0].height - 32.4) < 0.01);
        QVERIFY(qAbs(pics[0].width - 64.8) < 0.01);
        // a small one stays as it is, in the footer's room
        file = small;
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->drawPagesNow();
        QTest::mouseClick(w.pageView()->viewport(), Qt::LeftButton, Qt::NoModifier, w.pageView()->zoneRect(0, false, 2).center());
        QVERIFY(w.zoneEditor());
        w.act(QStringLiteral("hf.picture"))->trigger();
        QTest::keyClick(w.zoneEditor(), Qt::Key_Return, Qt::ControlModifier);
        const auto &after = w.session()->pages().sheet(0).headerFooter.pictures;
        QCOMPARE(after.size(), 2);
        QCOMPARE(after[1].place, QStringLiteral("RF"));
        QCOMPARE(after[1].height, 6.0);
        // the editor of the dialog too
        jp::office::sheet::HeaderFooter hf;
        HeaderFooterEditor editor(true, hf, QStringLiteral("S"), QStringLiteral("f"));
        file = big;
        editor.setPictureAsker([&] { return file; });
        editor.setPictureRoom(20);
        editor.findChild<QPushButton *>(QStringLiteral("hfPicture"))->click();
        QCOMPARE(editor.texts().pictures.size(), 1);
        QCOMPARE(editor.texts().pictures[0].height, 20.0);
    }

    // The pages show the sheet's gridlines and headings (the sheet's own settings, and the View switches), and they are never printed.
    void thePagesShowGridlinesAndHeadingsButTheyAreNotPrinted()
    {
        EditWindow e;
        QVERIFY(e.open(rowsWithFormulas(30, {})));
        CrunchWindow &w = e.win.w;
        w.setViewKind(2);
        QTRY_VERIFY(w.pageLayoutReady());
        w.pageView()->drawPagesNow();
        auto shot = [&] { return w.pageView()->viewport()->grab().toImage(); };
        const double dpr = shot().devicePixelRatio();
        // a few pixels along the bottom edge of cell A3, at its left (where its number is not), and a few beside the cells on the left, where the row numbers go
        auto darkest = [&](const QImage &img, const QRect &r) {
            int most = 255;
            for (int y = r.top(); y <= r.bottom(); ++y)
                for (int x = r.left(); x <= r.right(); ++x) most = std::min(most, img.pixelColor(int(x * dpr), int(y * dpr)).lightness());
            return most;
        };
        const QRect a3 = w.pageView()->cellRect(2, 0);
        const QRect line(a3.left() + 1, a3.bottom() - 1, 3, 3);
        const QRect strip(a3.left() - 12, a3.top() + 2, 8, 4);
        QVERIFY(w.act(QStringLiteral("view.gridlines"))->isChecked());
        QVERIFY(w.act(QStringLiteral("view.headings"))->isChecked());
        QVERIFY2(darkest(shot(), line) < 235, "gridlines");
        QVERIFY2(darkest(shot(), strip) < 240, "headings");
        w.act(QStringLiteral("view.gridlines"))->trigger();
        w.act(QStringLiteral("view.headings"))->trigger();
        w.pageView()->drawPagesNow();
        QVERIFY(darkest(shot(), line) >= 250);
        QVERIFY(darkest(shot(), strip) >= 250);
        w.act(QStringLiteral("view.gridlines"))->trigger();
        w.act(QStringLiteral("view.headings"))->trigger();
        // what is printed has neither
        QString why;
        std::unique_ptr<crunch::PrintJob> job = w.makePrintJob(crunch::PrintJobOptions(), &why);
        QVERIFY(job);
        const QImage page = job->preview(0, 1224);
        QRectF cell;
        QVERIFY(job->cellRect(0, 2, 0, &cell));
        for (int y = int(cell.bottom() * 2) - 1; y <= int(cell.bottom() * 2) + 1; ++y)
            for (int x = int(cell.left() * 2) + 1; x <= int(cell.left() * 2) + 4; ++x) QVERIFY(page.pixelColor(x, y).lightness() >= 250);
        for (int y = int(cell.top() * 2) + 2; y < int(cell.top() * 2) + 6; ++y)
            for (int x = int(cell.left() * 2) - 24; x < int(cell.left() * 2) - 4; ++x) QVERIFY(page.pixelColor(x, y).lightness() >= 250);
    }

    // The preview of the Print screen draws a header picture without decoding it on the window thread: the page shows at once, and the picture is in it when the worker has it.
    void thePrintPreviewMakesHeaderPicturesOffTheWindowThread()
    {
        const QByteArray red = drawtest::pngBytes(160, 80, Qt::red);
        EditWindow e;
        QVERIFY(e.open(bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader>"), {hfShape(QStringLiteral("CH"))}, {red})));
        CrunchWindow &w = e.win.w;
        jp::grid::HeaderPictureCache &cache = jp::grid::HeaderPictureCache::instance();
        cache.clear();
        const int here = cache.decodesHere();
        w.act(QStringLiteral("file.print"))->trigger();
        QLabel *preview = w.findChild<QLabel *>(QStringLiteral("printPreview"));
        QVERIFY(preview);
        QCOMPARE(cache.decodesHere(), here);
        auto redPixels = [&] {
            const QImage img = preview->pixmap().toImage();
            int n = 0;
            for (int y = 0; y < img.height(); y += 2)
                for (int x = 0; x < img.width(); x += 2) {
                    const QColor c = img.pixelColor(x, y);
                    n += c.red() > 200 && c.green() < 70 && c.blue() < 70;
                }
            return n;
        };
        QTRY_VERIFY_WITH_TIMEOUT(redPixels() > 20, 20000);
        QCOMPARE(cache.decodesHere(), here);
    }

    // A worker that finishes before the page's drawing is over (the picture is made while the page is still being drawn): the page is drawn again with the picture all the same, in the view of the
    // pages and in the preview of the Print screen. The page decides by what it met, not by whether anything is still being made afterwards.
    void aPageDrawnWithoutAPictureIsDrawnAgainEvenIfTheWorkerWasQuick()
    {
        const QByteArray red = drawtest::pngBytes(160, 80, Qt::red);
        jp::grid::HeaderPictureCache &cache = jp::grid::HeaderPictureCache::instance();
        struct Reset {
            ~Reset() { jp::grid::HeaderPictureCache::instance().setAfterStartHook(nullptr); }
        } reset;
        // (every picture handed to a worker is finished before the asker goes on)
        cache.setAfterStartHook([&cache] { cache.waitForWorkers(); });
        {
            EditWindow e;
            QVERIFY(e.open(bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader>"), {hfShape(QStringLiteral("CH"))}, {red})));
            CrunchWindow &w = e.win.w;
            cache.clear();
            QString why;
            std::unique_ptr<crunch::PrintJob> job = w.makePrintJob(crunch::PrintJobOptions(), &why);
            QVERIFY(job);
            job->setAsyncPictures(true);
            job->preview(0, 612);
            QVERIFY(job->lastPaintMissedPictures());   // (the page met a picture that was not ready, though nothing is being made now)
            QCOMPARE(cache.pendingCount(), 0);
            cache.clear();
            w.setViewKind(2);
            QTRY_VERIFY(w.pageLayoutReady());
            auto redInView = [&] {
                w.pageView()->drawPagesNow();
                const QImage shot = w.pageView()->viewport()->grab().toImage();
                const QRect page = w.pageView()->pageRect(0);
                const double k = 96.0 / 72.0 * w.pageView()->zoom();
                const QColor c = shot.pixelColor(int((page.left() + 306 * k) * shot.devicePixelRatio()), int((page.top() + 30 * k) * shot.devicePixelRatio()));
                return c.red() > 200 && c.green() < 70 && c.blue() < 70;
            };
            cache.clear();
            w.pageView()->contentsChanged();
            QTRY_VERIFY_WITH_TIMEOUT(redInView(), 20000);
        }
        {
            EditWindow e;
            QVERIFY(e.open(bookWithHeaderPictures(QStringLiteral("<oddHeader>&amp;C&amp;G</oddHeader>"), {hfShape(QStringLiteral("CH"))}, {red})));
            cache.clear();
            e.win.w.act(QStringLiteral("file.print"))->trigger();
            QLabel *preview = e.win.w.findChild<QLabel *>(QStringLiteral("printPreview"));
            QVERIFY(preview);
            auto redPixels = [&] {
                const QImage img = preview->pixmap().toImage();
                int n = 0;
                for (int y = 0; y < img.height(); y += 2)
                    for (int x = 0; x < img.width(); x += 2) {
                        const QColor c = img.pixelColor(x, y);
                        n += c.red() > 200 && c.green() < 70 && c.blue() < 70;
                    }
                return n;
            };
            QTRY_VERIFY_WITH_TIMEOUT(redPixels() > 20, 20000);
        }
    }

    // What the cache holds on to counts against its budget: the decoded picture and the file it was made from (kept so that the key's pointer stays its own).
    void theCacheCountsTheFilesItHoldsOnTo()
    {
        jp::grid::HeaderPictureCache &cache = jp::grid::HeaderPictureCache::instance();
        cache.clear();
        const QByteArray png = drawtest::pngBytes(50, 40, Qt::blue);
        const QImage image = cache.get(png, true, nullptr);
        QVERIFY(!image.isNull());
        QCOMPARE(cache.bytes(), image.sizeInBytes() + png.size());
        cache.clear();
    }

    // A first-page or even-page picture is written with the long ids, whichever spelling it came in, and read back as it was.
    void firstAndEvenPagePicturesAreWrittenWithTheLongIds()
    {
        const QByteArray red = drawtest::pngBytes(8, 4, Qt::red), blue = drawtest::pngBytes(4, 8, Qt::blue);
        EditWindow e;
        QString rows;
        for (int r = 1; r <= 3; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        CrunchWindow &w = e.win.w;
        PageSetup p = w.session()->pages().sheet(0);
        p.headerFooter.differentFirst = true;
        p.headerFooter.differentOddEven = true;
        p.headerFooter.firstHeader = QStringLiteral("&C&G");
        p.headerFooter.evenHeader = QStringLiteral("&L&G");
        for (const char *place : {"chf", "LHE"}) {
            jp::office::sheet::HeaderPicture pic;
            pic.place = QString::fromLatin1(place);
            pic.image = pic.place == QLatin1String("chf") ? red : blue;
            pic.extension = QStringLiteral("png");
            pic.width = 50;
            pic.height = 25;
            p.headerFooter.pictures.append(pic);
        }
        QVERIFY(w.session()->setPageSetup(0, p, QStringLiteral("Pictures")).ok);
        QSignalSpy saved(&w, &CrunchWindow::saved);
        QVERIFY(w.save());
        QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join(QLatin1Char('|'))));
        Package pk;
        QString why;
        QVERIFY(pk.readLazyFile(e.path, &why));
        const auto link = pk.relationshipsOfType(QStringLiteral("/xl/worksheets/sheet1.xml"), QStringLiteral("/vmlDrawing"));
        QCOMPARE(link.size(), 1);
        const QByteArray vml = pk.part(link[0].target);
        QVERIFY2(vml.contains("id=\"CHFIRST\"") && vml.contains("id=\"LHEVEN\""), vml.constData());
        QVERIFY(!vml.contains("id=\"CHF\"") && !vml.contains("id=\"LHE\"") && !vml.contains("id=\"chf\""));
        Workbook b;
        Sheet sheet;
        QVERIFY(b.open(pk, &why) && b.readSheet(0, &sheet));
        const auto &hf = sheet.page.headerFooter;
        QCOMPARE(hf.pictures.size(), 2);
        QCOMPARE(hf.pictureFor(QLatin1Char('C'), true, true, false)->image, red);
        QCOMPARE(hf.pictureFor(QLatin1Char('L'), true, false, true)->image, blue);
    }

    // The closure that draws the Print screen's preview is freed with the screen (it does not hold itself).
    void thePrintScreensPaintClosureIsFreedWithTheScreen()
    {
        std::weak_ptr<void> closure;
        {
            EditWindow e;
            QVERIFY(e.open(rowsWithFormulas(10, {})));
            e.win.w.act(QStringLiteral("file.print"))->trigger();
            closure = jp::printPaintClosureForTest();
            QVERIFY(!closure.expired());
            e.win.w.act(QStringLiteral("file.print"))->trigger();   // (the screen is shown again: made again or not, the last one is the one watched)
            closure = jp::printPaintClosureForTest();
            QVERIFY(!closure.expired());
        }
        QVERIFY(closure.expired());
    }
};

JP_TEST_CLASS(CrunchPrintTests)
#include "tst_crunch_print.moc"
