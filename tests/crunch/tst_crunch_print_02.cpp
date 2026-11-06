#include "tst_crunch_print.h"


// Where the view is scrolled to and where the cursor is are taken when the file is saved, not when the panes were frozen: a sheet frozen and then scrolled and moved on is saved so; and a sheet
// that is not frozen is saved scrolled as it is.
void CrunchPrintTests::theScrollAndTheCursorAreTakenAtTheSave()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    e.select(QStringLiteral("C5"));
    w.act(QStringLiteral("view.freeze"))->trigger();
    e.select(QStringLiteral("E70"));   // (scrolled down, and the cursor moved)
    const qint64 top = e.view()->sheet().layout().pane(grid::Pane::Body).firstRow;
    QVERIFY(top > 10);
    auto saveAndRead = [&] {
        QSignalSpy saved(&w, &CrunchWindow::saved);
        if (!w.save() || !savedOk(saved, 20000)) return QByteArray();
        Package pk;
        QString why;
        if (!pk.readLazyFile(e.path, &why)) return QByteArray();
        return pk.part(QStringLiteral("xl/worksheets/sheet1.xml"));
    };
    QByteArray xml = saveAndRead();
    QVERIFY(!xml.isEmpty());
    QVERIFY2(xml.contains(QStringLiteral("<pane xSplit=\"2\" ySplit=\"4\" topLeftCell=\"C%1\"").arg(top + 1).toUtf8()), xml.left(1200).constData());
    QVERIFY2(xml.contains("pane=\"bottomRight\" activeCell=\"E70\""), xml.left(1200).constData());
    // unfrozen, the sheet keeps where it is scrolled to
    w.act(QStringLiteral("view.unfreeze"))->trigger();
    e.select(QStringLiteral("B90"));
    const qint64 first = e.view()->firstRowInView();
    QVERIFY(first > 10);
    xml = saveAndRead();
    QVERIFY(!xml.contains("<pane"));
    QVERIFY2(xml.contains(QStringLiteral("topLeftCell=\"A%1\"").arg(first + 1).toUtf8()), xml.left(1200).constData());
    QVERIFY(xml.contains("activeCell=\"B90\""));
}


// ---- pictures in headers and footers (printing 2)

// A picture in the center of the header is drawn on the printed page, at the size the file gives, hung from the header's distance from the top; the same page without it is white there.
void CrunchPrintTests::aPictureInTheHeaderIsDrawnOnThePage()
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
void CrunchPrintTests::aPictureInTheFooterAndInTheFirstPageForm()
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
void CrunchPrintTests::aHeaderPictureSurvivesASaveThatChangesOtherThings()
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
void CrunchPrintTests::thePictureButtonsOfTheHeaderEditor()
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
void CrunchPrintTests::aHeaderPictureAddedThroughPageSetupSavesAndComesOut()
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
void CrunchPrintTests::aHeaderPictureSurvivesRowsBeingInserted()
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
void CrunchPrintTests::aCopyOfASheetWithHeaderPicturesHasThemToo()
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
void CrunchPrintTests::aJobSaysWhereTheCellsOfAPageAre()
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
void CrunchPrintTests::thePageLayoutViewShowsThePages()
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
void CrunchPrintTests::cellsCanBeChosenAndMovedOnThePages()
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
void CrunchPrintTests::cellsAreEditedOnThePages()
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
void CrunchPrintTests::aHeaderIsTypedInPlace()
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


// Insert > Header & Footer shows the pages and opens the middle of the first page's header to type in; what is typed is drawn on the preview's page and is still there when the file is saved and opened again.
void CrunchPrintTests::insertHeaderFooterOpensTheHeader()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    QVERIFY(w.act(QStringLiteral("insert.headerFooter")));
    QCOMPARE(w.viewKind(), 0);
    w.act(QStringLiteral("insert.headerFooter"))->trigger();
    QCOMPARE(w.viewKind(), 2);
    QTRY_VERIFY(w.zoneEditor());
    QVERIFY(w.zoneEditor()->isVisible());
    QTest::keyClicks(w.zoneEditor(), "Budget ");
    w.act(QStringLiteral("hf.pageNumber"))->trigger();
    QTest::keyClick(w.zoneEditor(), Qt::Key_Return, Qt::ControlModifier);
    QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddHeader, QStringLiteral("&CBudget &P"));
    // (again, with the pages already shown)
    QTRY_VERIFY(w.pageLayoutReady());
    w.act(QStringLiteral("insert.headerFooter"))->trigger();
    QTRY_VERIFY(w.zoneEditor());
    QCOMPARE(w.zoneEditor()->toPlainText(), QStringLiteral("Budget &P"));
    QTest::keyClick(w.zoneEditor(), Qt::Key_Escape);
    // the file (a save is written in the background: wait for it to be done before reading the file)
    QSignalSpy saved(&w, &CrunchWindow::saved);
    QVERIFY(w.save());
    QVERIFY2(savedOk(saved, 20000), qPrintable(e.win.told.join(QLatin1Char('|'))));
    EditWindow again;
    QVERIFY(again.win.open(e.copyOfFile()));
    QCOMPARE(again.win.w.session()->pages().sheet(0).headerFooter.oddHeader, QStringLiteral("&CBudget &P"));
}


// The Header and Footer galleries write a ready form into the header or the footer (the version being typed in, when one is), and Go to Header and Go to Footer open that part to type in.
void CrunchPrintTests::theGalleriesWriteReadyFormsAndGoToOpensThePart()
{
    EditWindow e;
    QVERIFY(e.open(rowsWithFormulas(100, {})));
    CrunchWindow &w = e.win.w;
    w.setViewKind(2);
    QTRY_VERIFY(w.pageLayoutReady());
    w.pageView()->drawPagesNow();
    w.zoneClicked(0, true, 1);
    QVERIFY(w.zoneEditor());
    w.zoneEditor()->insertPlainText(QStringLiteral("typed first"));
    const int steps = w.session()->undoStack()->count();
    w.applyHeaderForm(true, QStringLiteral("&CPage &P of &N"));   // (the form replaces the header as a whole: what was typed in the part is not a step of its own)
    QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddHeader, QStringLiteral("&CPage &P of &N"));
    QVERIFY(!w.zoneEditor());
    QCOMPARE(w.session()->undoStack()->count(), steps + 1);
    QTRY_VERIFY(w.pageLayoutReady());
    w.act(QStringLiteral("hf.footer.sheetPage"))->trigger();
    QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddFooter, QStringLiteral("&L&A&RPage &P"));
    w.act(QStringLiteral("hf.footer.none"))->trigger();
    QCOMPARE(w.session()->pages().sheet(0).headerFooter.oddFooter, QString());
    QTRY_VERIFY(w.pageLayoutReady());
    w.pageView()->drawPagesNow();
    w.goToHeaderOrFooter(false);
    QVERIFY(w.zoneEditor());
    w.zoneEditor()->hide();
    w.goToHeaderOrFooter(true);
    QVERIFY(w.zoneEditor());
    QCOMPARE(w.zoneEditor()->toPlainText(), QStringLiteral("Page &P of &N"));
}


// The first page has a header of its own once Different First Page is chosen on the tab, and a click on the first page's header types that one.
void CrunchPrintTests::theFirstPageHeaderIsTypedInPlace()
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
void CrunchPrintTests::theTabPutsAPictureInTheHeader()
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
void CrunchPrintTests::thePageLayoutViewIsSavedAndComesBack()
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
void CrunchPrintTests::aSheetOfAMillionRowsIsShownAsPagesInSlices()
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
void CrunchPrintTests::aZoomChangeAloneIsSaved()
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
void CrunchPrintTests::leavingAViewPutsTheCellsOwnZoomBack()
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
void CrunchPrintTests::eachSheetKeepsItsOwnZoomOfTheCells()
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
void CrunchPrintTests::aFullListOfBreaksKeepsTheOldBreak()
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
void CrunchPrintTests::aHeaderPictureIsMadeOffTheWindowThread()
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
void CrunchPrintTests::anOversizedPageIsDrawnOnlyWhereItIsExposed()
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
void CrunchPrintTests::thePagePainterDrawsOnlyWhatTheClipLeavesInView()
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
void CrunchPrintTests::pagesStayOnTheScreenUntilTheNewOnesAreReady()
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
void CrunchPrintTests::thePlaceOfAPagesCellsIsWorkedOutOncePerPage()
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
void CrunchPrintTests::aNewPictureFitsTheHeaderMargin()
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
void CrunchPrintTests::thePagesShowGridlinesAndHeadingsButTheyAreNotPrinted()
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
void CrunchPrintTests::thePrintPreviewMakesHeaderPicturesOffTheWindowThread()
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
void CrunchPrintTests::aPageDrawnWithoutAPictureIsDrawnAgainEvenIfTheWorkerWasQuick()
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
void CrunchPrintTests::theCacheCountsTheFilesItHoldsOnTo()
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
void CrunchPrintTests::firstAndEvenPagePicturesAreWrittenWithTheLongIds()
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
void CrunchPrintTests::thePrintScreensPaintClosureIsFreedWithTheScreen()
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
