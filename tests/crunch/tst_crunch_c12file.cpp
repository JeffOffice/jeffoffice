// Tests of the File, Review, and View items of round 12: Check Accessibility's blank layout cells and reading order, and the others as they were added. Every workbook is built here.
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/settings.h"
#include "app/anchordialogs.h"
#include "core/accesscheck.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/printjob.h"
#include "sheet/calculation.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "savedok.h"
#include "sheet/protection.h"
#include "sheet/sheetreader.h"
#include "../localepin.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTimer>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QPointer>
#include <QSignalSpy>
#include <QUndoStack>
#include <QScrollBar>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

#include <cmath>
#include <memory>

using namespace jp;
using namespace crunchtest;
using jp::office::sheet::EditRange;

namespace {
// A1:C4 hold numbers, all locked; the sheet is protected, with an open range A1:A2 ("Open"), and one with the password "test" at C1:C2 ("Secret").
QByteArray rangesBook()
{
    QString rows;
    for (int r = 1; r <= 4; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c><c r=\"B%1\"><v>2</v></c><c r=\"C%1\"><v>3</v></c></row>").arg(r);
    return buildXlsx({worksheet(rows, QStringLiteral("<sheetProtection sheet=\"1\" objects=\"1\" scenarios=\"1\"/><protectedRanges><protectedRange sqref=\"A1:A2\" name=\"Open\"/>"
                                                    "<protectedRange password=\"CBEB\" sqref=\"C1:C2\" name=\"Secret\"/></protectedRanges>"))});
}
QByteArray savedPart(const QString &path, const QString &part)
{
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QMap<QString, QByteArray> parts;
    jp::readZip(f.readAll(), parts);
    return parts.value(part);
}
}

class CrunchC12FileTests : public QObject {
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
        m_localeBefore = QLocale();
        m_pin = std::make_unique<PinnedLocale>();
    }

    void cleanupTestCase()
    {
        m_pin.reset();
        QCOMPARE(QLocale().name(), m_localeBefore.name());
    }

    // Check Accessibility: a blank row or column between cells that hold data is a layout trick a screen reader reads as the end of the table; the first row and column, and the edges, are not.
    void checkAccessibilityFindsBlankRowsAndColumnsUsedForLayout()
    {
        // data in rows 1, 2, 4, and 5 (row 3 is blank) and in columns A, B, and D (column C is blank)
        const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\"><v>1</v></c><c r=\"B1\"><v>2</v></c><c r=\"D1\"><v>9</v></c></row>"
                                            "<row r=\"2\"><c r=\"A2\"><v>3</v></c><c r=\"B2\"><v>4</v></c><c r=\"D2\"><v>9</v></c></row>"
                                            "<row r=\"4\"><c r=\"A4\"><v>5</v></c><c r=\"B4\"><v>6</v></c><c r=\"D4\"><v>9</v></c></row>"
                                            "<row r=\"5\"><c r=\"A5\"><v>7</v></c><c r=\"B5\"><v>8</v></c><c r=\"D5\"><v>9</v></c></row>");
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), QByteArray(), {QStringLiteral("Budget")})));
        QVERIFY(e.settle());
        const auto issues = crunch::checkAccessibility(*e.win.w.session(), nullptr);
        QStringList where;
        for (const auto &i : issues)
            if (i.kind == crunch::AccessIssue::BlankLayoutCells) where << QStringLiteral("%1,%2").arg(i.row).arg(i.col);
        QCOMPARE(where, (QStringList{QStringLiteral("2,0"), QStringLiteral("0,2")}));
    }

    // Reading order: objects listed (stacked) in another order than their places on the sheet.
    void objectsStackedAgainstTheReadingOrderAreFound()
    {
        using P = std::pair<quint32, quint32>;
        QCOMPARE(crunch::firstObjectOutOfReadingOrder({}), -1);
        QCOMPARE(crunch::firstObjectOutOfReadingOrder({P{1, 1}, P{1, 5}, P{4, 0}}), -1);
        QCOMPARE(crunch::firstObjectOutOfReadingOrder({P{1, 1}, P{4, 0}, P{1, 5}}), 2);
        QCOMPARE(crunch::firstObjectOutOfReadingOrder({P{4, 0}, P{1, 5}}), 1);
    }

    // View > New Window shows the file in a second window that can't change it; Arrange All sets the windows in columns; Side by Side puts two on halves of the screen and ties their scrolling.
    void newWindowArrangeAllAndSideBySideWork()
    {
        EditWindow e;
        QString rows;
        for (int r = 1; r <= 300; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        QVERIFY(e.settle());
        CrunchWindow &w = e.win.w;
        const int before = CrunchWindow::crunchWindows().size();
        w.act(QStringLiteral("win.new"))->trigger();
        QTRY_COMPARE(CrunchWindow::crunchWindows().size(), before + 1);
        CrunchWindow *second = nullptr;
        for (CrunchWindow *c : CrunchWindow::crunchWindows())
            if (c != &w && c->filePath() == e.path) second = c;
        QVERIFY(second);
        QPointer<CrunchWindow> guard(second);
        QTRY_VERIFY(second->hasWorkbook());
        QVERIFY(second->isReadOnly());
        // Arrange All: the two windows each have their own column
        w.act(QStringLiteral("win.arrange"))->trigger();
        QVERIFY(w.geometry().right() < second->geometry().left() || second->geometry().right() < w.geometry().left());
        // Side by Side: each is half the screen, and scrolling one scrolls the other
        w.act(QStringLiteral("view.sideBySide"))->trigger();
        QCOMPARE(w.sideBySidePartner(), second);
        QCOMPARE(second->sideBySidePartner(), &w);
        QScrollBar *bar = w.view()->verticalScrollBar();
        QTRY_VERIFY(second->view()->verticalScrollBar()->maximum() > 0 && bar->maximum() > 0);
        bar->setValue(7);
        QCOMPARE(second->view()->verticalScrollBar()->value(), 7);
        // with Synchronous Scrolling off the other stays
        w.act(QStringLiteral("view.syncScroll"))->trigger();
        bar->setValue(12);
        QCOMPARE(second->view()->verticalScrollBar()->value(), 7);
        w.act(QStringLiteral("view.sideBySide"))->trigger();
        QVERIFY(!w.sideBySidePartner());
        QVERIFY(!second->sideBySidePartner());
        second->close();
        QTRY_VERIFY(guard.isNull());
    }

    // Allow Edit Ranges: a range with no password lets its cells be changed on a protected sheet; one with a password does once the password is given; the rest stay locked.
    void aProtectedSheetLeavesItsEditRangesOpen()
    {
        EditWindow e;
        QVERIFY(e.open(rangesBook()));
        crunch::EditSession *s = e.win.w.session();
        QCOMPARE(s->protectionOf(0).editRanges.size(), 2);
        QCOMPARE(s->protectionOf(0).editRanges[1].title, QStringLiteral("Secret"));
        QCOMPARE(s->protectionOf(0).editRanges[1].password.legacy, QStringLiteral("CBEB"));
        QVERIFY(s->canEditCell(0, 0, 0));      // A1: in "Open"
        QVERIFY(!s->canEditCell(0, 0, 1));     // B1: locked
        QVERIFY(!s->canEditCell(0, 0, 2));     // C1: "Secret" is closed
        QCOMPARE(s->passwordRangeAt(0, 0, 2), QStringLiteral("Secret"));
        QVERIFY(s->passwordRangeAt(0, 0, 1).isEmpty());
        QVERIFY(s->openEditRange(0, QStringLiteral("Secret")));
        QVERIFY(s->canEditCell(0, 0, 2));
        QVERIFY(s->type(0, 0, 0, QStringLiteral("7")).ok);
        QVERIFY(!s->type(0, 0, 1, QStringLiteral("7")).ok);
    }

    // The dialog's ranges go in as one step of Undo, a typed password is kept only as its hash, the file holds them right after the sheet's protection, and a range left alone keeps what the file had.
    void editRangesAreKeptWithTheirHashAndRoundTrip()
    {
        EditWindow e;
        QVERIFY(e.open(rangesBook()));
        e.win.w.anchorHooks().editRangesAsker = [](EditRangesRequest *r) {
            EditRange mine;
            mine.title = QStringLiteral("Mine");
            mine.sqref = QStringLiteral("B1:B2");
            r->ranges.append(mine);
            r->passwords.insert(QStringLiteral("Mine"), QStringLiteral("correct horse"));
            return true;
        };
        e.win.w.act(QStringLiteral("protect.editRanges"))->trigger();
        crunch::EditSession *s = e.win.w.session();
        QTRY_COMPARE(s->protectionOf(0).editRanges.size(), 3);
        const EditRange mine = s->protectionOf(0).editRanges[2];
        QCOMPARE(mine.password.algorithm, QStringLiteral("SHA-512"));
        QVERIFY(!mine.password.hash.isEmpty());
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QByteArray xml = savedPart(e.path, QStringLiteral("xl/worksheets/sheet1.xml"));
        const int prot = xml.indexOf("<sheetProtection"), ranges = xml.indexOf("<protectedRanges"), after = xml.indexOf("</protectedRanges>");
        QVERIFY2(prot >= 0 && ranges > prot && after > ranges, xml.constData());
        QVERIFY(!xml.contains("correct horse"));
        QVERIFY(xml.contains("password=\"CBEB\"") && xml.contains("name=\"Mine\"") && xml.contains("algorithmName=\"SHA-512\""));
        Window second;
        QVERIFY(second.open(e.copyOfFile()));
        QCOMPARE(second.w.session()->protectionOf(0).editRanges.size(), 3);
        QVERIFY(second.w.session()->protectionOf(0).editRanges[2] == mine);
        QVERIFY(second.w.session()->protectionOf(0).editRanges[0].password.legacy.isEmpty());
        // Undo puts the file's ranges back
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QTRY_COMPARE(s->protectionOf(0).editRanges.size(), 2);
    }

    // Typing in a cell of a range that has a password asks for it; the right one opens the range for the rest of the session, a wrong one leaves it closed.
    void aCellOfARangeWithAPasswordAsksForIt()
    {
        EditWindow e;
        QVERIFY(e.open(rangesBook()));
        QString answer = QStringLiteral("nope");
        int asked = 0;
        e.win.w.anchorHooks().passwordAsker = [&](PasswordRequest *r) {
            ++asked;
            r->password = answer;
            return true;
        };
        e.select(QStringLiteral("C1"));
        e.type(QStringLiteral("9"));
        QTRY_COMPARE(asked, 1);
        QTRY_VERIFY(!e.win.told.isEmpty());   // (the password was wrong)
        QVERIFY(!e.win.w.session()->canEditCell(0, 0, 2));
        answer = QStringLiteral("test");
        e.type(QStringLiteral("9"));
        QTRY_VERIFY(e.win.w.session()->canEditCell(0, 0, 2));
        QCOMPARE(asked, 2);
        QTRY_VERIFY(e.win.w.isEditing());
    }

    // Print scaled to a paper size: every sheet of the job is that paper (turned the way the page lies), with the page shrunk or grown to fit it; the preview and the PDF both show it.
    void pagesCanBeScaledToAPaperSize()
    {
        QString rows;
        for (int r = 1; r <= 100; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        QString why;
        crunch::PrintJobOptions options;
        std::unique_ptr<crunch::PrintJob> plain = e.win.w.makePrintJob(options, &why);
        QVERIFY2(plain, qPrintable(why));
        QCOMPARE(plain->sheetFor({0}, 1), QSizeF(612, 792));
        options.scaleToPaper = 9;   // A4
        std::unique_ptr<crunch::PrintJob> job = e.win.w.makePrintJob(options, &why);
        QVERIFY2(job, qPrintable(why));
        double w = 0, h = 0;
        jp::office::sheet::paperSizePoints(9, &w, &h);
        QCOMPARE(job->sheetFor({0}, 1), QSizeF(w, h));
        QCOMPARE(job->pageSize(0), QSizeF(612, 792));   // (the page itself is as it was)
        const QImage sheet = job->previewSheet({0}, 1, 300);
        QVERIFY(!sheet.isNull());
        QCOMPARE(sheet.height(), int(std::lround(300 * h / w)));
        // the page is put in the middle, a little smaller than the sheet is wide: the strip at the left edge is white and the text is inside
        bool inkAtEdge = false, inkInside = false;
        for (int y = 0; y < sheet.height(); ++y) {
            if (qGray(sheet.pixel(0, y)) < 200) inkAtEdge = true;
            for (int x = 20; x < 280; ++x)
                if (qGray(sheet.pixel(x, y)) < 100) inkInside = true;
        }
        QVERIFY(!inkAtEdge);
        QVERIFY(inkInside);
    }

    // The Print screen: Scale to paper writes a PDF whose sheets are that paper, and with several pages on a sheet the preview is of the sheet and counts sheets.
    void thePrintScreenScalesToPaperAndPreviewsTheSheet()
    {
        QString rows;
        for (int r = 1; r <= 100; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>%1</v></c></row>").arg(r);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows)})));
        CrunchWindow &w = e.win.w;
        const QString target = e.folder.dir.filePath(QStringLiteral("scaled.pdf"));
        w.setSavePicker([&](const QString &, const QString &) { return target; });
        w.act(QStringLiteral("file.print"))->trigger();
        QComboBox *printer = w.findChild<QComboBox *>(QStringLiteral("printPrinter"));
        printer->setCurrentIndex(printer->count() - 1);
        QComboBox *scale = w.findChild<QComboBox *>(QStringLiteral("printScaleToPaper"));
        QVERIFY(scale);
        QComboBox *per = w.findChild<QComboBox *>(QStringLiteral("printPerSheet"));
        QLabel *page = w.findChild<QLabel *>(QStringLiteral("printPage"));
        QTRY_COMPARE(page->text(), QStringLiteral("Page 1 of 3"));
        per->setCurrentIndex(per->findData(2));
        QTRY_COMPARE(page->text(), QStringLiteral("Sheet 1 of 2"));
        per->setCurrentIndex(per->findData(1));
        scale->setCurrentIndex(scale->findData(9));
        QTRY_COMPARE(page->text(), QStringLiteral("Sheet 1 of 3"));
        w.findChild<QPushButton *>(QStringLiteral("printButton"))->click();
        QFile f(target);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray pdf = f.readAll();
        QVERIFY(pdf.startsWith("%PDF-"));
        QVERIFY2(QRegularExpression("/MediaBox \\[0 0 59[45]").match(QString::fromLatin1(pdf)).hasMatch(), "the sheets are not A4");
    }

    // A web page opens as a workbook: each table is a sheet; a cell a neighbor spans over is blank, not shifted; a cell that reads as a formula stays text; the text outside the tables is not read.
    void aWebPageOpensAsAWorkbookOfItsTables()
    {
        const QByteArray page = "<html><head><title>Report</title></head><body><p>Intro</p><table><tr><th>Name</th><th>Qty</th><th>Note</th></tr>"
                                "<tr><td rowspan=\"2\">Apples</td><td>3</td><td>=1+1</td></tr><tr><td>4</td><td>x &amp; y</td></tr></table>"
                                "<p>Then</p><table><tr><td>Only</td><td>1,250.5</td></tr></table></body></html>";
        EditWindow e;
        QVERIFY(e.open(page, QStringLiteral("report.html")));
        QVERIFY(e.settle());
        const auto &sheets = e.win.w.session()->workbook()->info().sheets;
        QCOMPARE(int(sheets.size()), 2);
        QCOMPARE(sheets[0].name, QStringLiteral("Table1"));
        QCOMPARE(sheets[1].name, QStringLiteral("Table2"));
        QCOMPARE(e.shownAt(QStringLiteral("A1")), QStringLiteral("Name"));
        QCOMPARE(e.shownAt(QStringLiteral("A2")), QStringLiteral("Apples"));
        QCOMPARE(e.shownAt(QStringLiteral("B2")), QStringLiteral("3"));
        QVERIFY(e.shownAt(QStringLiteral("A3")).isEmpty());
        QCOMPARE(e.shownAt(QStringLiteral("B3")), QStringLiteral("4"));
        QCOMPARE(e.shownAt(QStringLiteral("C3")), QStringLiteral("x & y"));
        QVERIFY(!e.win.w.session()->calculation()->content(0, 1, 2).isFormula);
        // a page with no table is said not to be a workbook
        const QString plain = e.folder.write(QStringLiteral("plain.html"), QByteArray("<html><body><p>No table here</p></body></html>"));
        jp::office::sheet::Workbook book;
        QString why;
        QVERIFY(!book.openFile(plain, &why));
        QVERIFY2(why.contains(QStringLiteral("no table")), qPrintable(why));
    }

    // Options: Editing has "edit directly in cells" (off: the entry goes to the formula bar), Display has the gridline color, the zoom of a new workbook, and the standard width of the sheet's columns,
    // and Privacy takes the names out of the properties on save.
    void theOptionsKeepEditInCellGridColorZoomWidthAndPrivacy()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        CrunchWindow &w = e.win.w;
        QVERIFY(w.editInCell());
        auto *watch = new QTimer(&w);
        watch->setInterval(10);
        connect(watch, &QTimer::timeout, watch, [watch] {
            QWidget *dialog = QApplication::activeModalWidget();
            if (!dialog || dialog->findChildren<QCheckBox *>(QStringLiteral("optionEditInCell")).isEmpty()) return;
            watch->stop();
            dialog->findChild<QCheckBox *>(QStringLiteral("optionEditInCell"))->setChecked(false);
            dialog->findChild<QPushButton *>(QStringLiteral("optionGridColor"))->setProperty("color", QColor(0x33, 0x66, 0x99));
            dialog->findChild<QSpinBox *>(QStringLiteral("optionDefaultZoom"))->setValue(150);
            dialog->findChild<QDoubleSpinBox *>(QStringLiteral("optionStandardWidth"))->setValue(20);
            dialog->findChild<QCheckBox *>(QStringLiteral("optionRemovePersonal"))->setChecked(true);
            if (auto *buttons = dialog->findChild<QDialogButtonBox *>()) buttons->button(QDialogButtonBox::Ok)->click();
        });
        watch->start();
        w.act(QStringLiteral("file.options"))->trigger();
        QVERIFY(!w.editInCell());
        QCOMPARE(w.gridlineColor(), QColor(0x33, 0x66, 0x99));
        QCOMPARE(w.defaultZoom(), 150);
        QVERIFY(w.removePersonalOnSave());
        QCOMPARE(Settings::get().value(QStringLiteral("crunch/gridColor")).toString(), QStringLiteral("#336699"));
        // the standard width is the sheet's: 20 characters
        const double standard = w.session()->calculation()->sheet(0).format.defaultColWidth;
        QVERIFY2(standard > 20 && standard < 21.5, qPrintable(QString::number(standard)));
        // typing in a cell now goes to the formula bar
        e.select(QStringLiteral("E1"));
        QTest::keyClick(e.view(), '5');
        QVERIFY(w.isEditing());
        QVERIFY(w.editingInFormulaBar());
        e.key(Qt::Key_Escape);
        // a new workbook starts at the zoom
        EditWindow fresh;
        QVERIFY(fresh.openBlank());
        QCOMPARE(int(std::lround(fresh.view()->zoom() * 100)), 150);
        w.setEditInCell(true);
        w.setRemovePersonalOnSave(false);
    }

    // Edit ranges move with inserted and deleted rows and columns, as one step of Undo with the edit; a range cut by a deleted column shrinks, one taken out whole is gone, and the file holds the new cells.
    void editRangesFollowInsertedAndDeletedLines()
    {
        QString rows;
        for (int r = 1; r <= 4; ++r) rows += QStringLiteral("<row r=\"%1\"><c r=\"A%1\"><v>1</v></c><c r=\"B%1\"><v>2</v></c><c r=\"C%1\"><v>3</v></c></row>").arg(r);
        EditWindow e;
        QVERIFY(e.open(buildXlsx({worksheet(rows, QStringLiteral("<protectedRanges><protectedRange sqref=\"A1:A2\" name=\"Open\"/>"
                                                                "<protectedRange sqref=\"B1:D4\" name=\"Wide\"/></protectedRanges>"))})));
        crunch::EditSession *s = e.win.w.session();
        auto cells = [&] {
            QStringList out;
            for (const EditRange &r : s->protectionOf(0).editRanges) out << r.title + QLatin1Char('=') + r.sqref;
            return out.join(QLatin1Char(' '));
        };
        QCOMPARE(cells(), QStringLiteral("Open=A1:A2 Wide=B1:D4"));
        QVERIFY(s->insertRows(0, 0, 1).ok);   // a row above both
        QCOMPARE(cells(), QStringLiteral("Open=A2:A3 Wide=B2:D5"));
        QVERIFY(s->deleteColumns(0, 2, 1).ok);   // column C, inside "Wide"
        QCOMPARE(cells(), QStringLiteral("Open=A2:A3 Wide=B2:C5"));
        QVERIFY(s->deleteColumns(0, 0, 1).ok);   // column A: "Open" is gone, "Wide" moves left
        QTRY_COMPARE(cells(), QStringLiteral("Wide=A2:B5"));
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QTRY_COMPARE(cells(), QStringLiteral("Open=A2:A3 Wide=B2:C5"));
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QTRY_COMPARE(cells(), QStringLiteral("Open=A2:A3 Wide=B2:D5"));
        e.win.w.act(QStringLiteral("edit.undo"))->trigger();
        QTRY_COMPARE(cells(), QStringLiteral("Open=A1:A2 Wide=B1:D4"));
        e.win.w.act(QStringLiteral("edit.redo"))->trigger();
        e.win.w.act(QStringLiteral("edit.redo"))->trigger();
        QTRY_COMPARE(cells(), QStringLiteral("Open=A2:A3 Wide=B2:C5"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved, 20000), "the save failed");
        const QByteArray xml = savedPart(e.path, QStringLiteral("xl/worksheets/sheet1.xml"));
        QVERIFY2(xml.contains("sqref=\"A2:A3\"") && xml.contains("sqref=\"B2:C5\""), xml.constData());
    }

    // An edit range's password is kept as its salted hash (never the text), and one the file holds in the older form is still checked.
    void anEditRangePasswordIsHashedAndTheOlderFormIsChecked()
    {
        const EditRange legacy = [] { EditRange r; r.password.legacy = QStringLiteral("CBEB"); return r; }();
        QCOMPARE(jp::office::sheet::checkPassword(legacy.password, QStringLiteral("test")), jp::office::sheet::PasswordResult::Match);
        QCOMPARE(jp::office::sheet::checkPassword(legacy.password, QStringLiteral("nope")), jp::office::sheet::PasswordResult::Wrong);
        EditRange made;
        QVERIFY(jp::office::sheet::makePasswordHash(QStringLiteral("correct horse"), &made.password));
        made.title = QStringLiteral("T");
        made.sqref = QStringLiteral("A1");
        jp::office::sheet::SheetProtection p;
        p.editRanges.append(made);
        const QByteArray xml = jp::office::sheet::protectedRangesElement(p, QByteArray());
        QVERIFY2(xml.contains("algorithmName=\"SHA-512\"") && xml.contains("hashValue=\"") && xml.contains("saltValue=\"") && xml.contains("spinCount=\"100000\""), xml.constData());
        QVERIFY(!xml.contains("correct horse") && !xml.contains(" password="));
        QCOMPARE(jp::office::sheet::checkPassword(made.password, QStringLiteral("correct horse")), jp::office::sheet::PasswordResult::Match);
    }

private:
    QLocale m_localeBefore;
    std::unique_ptr<PinnedLocale> m_pin;
};

JP_TEST_CLASS(CrunchC12FileTests)
#include "tst_crunch_c12file.moc"
