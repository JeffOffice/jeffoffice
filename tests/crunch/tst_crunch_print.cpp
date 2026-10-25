// Tests of printing in JeffCrunch's window: how a sheet prints as a step of Undo, the print area, titles, and breaks following rows and columns inserted and deleted (and coming back with
// Undo), the page setup saved in the file and nothing else of it touched, and the pages the window prints. Every workbook is built here, in a temporary folder.
#include "../sheet/drawinghelpers.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
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
        QVERIFY(saved.wait(20000));
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
