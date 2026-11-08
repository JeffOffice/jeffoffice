#include "tst_write.h"


void WriteTests::thePaperIsDescribedAsTheSectionSetsIt()
{
    QCOMPARE(paperDescription(QSizeF(612, 792)), QStringLiteral("Letter, portrait"));
    QCOMPARE(paperDescription(QSizeF(792, 612)), QStringLiteral("Letter, landscape"));
    QCOMPARE(paperDescription(QSizeF(595.3, 841.9)), QStringLiteral("A4, portrait"));
    QVERIFY(paperDescription(QSizeF(500, 300)).endsWith(QStringLiteral(", landscape")));
    QVERIFY(paperDescription(QSizeF(500, 300)).contains(QLatin1Char('x')));
}


void WriteTests::thePrintScreenHasThePrinterCopiesCollateRangeAndPreview()
{
    WriteWindow w;
    w.setDocument(open(makeDocx(paragraphs(150), kStyles)), QString());
    w.show();
    QCOMPARE(w.act("file.print")->shortcut(), QKeySequence(QKeySequence::Print));
    w.act("file.print")->trigger();
    auto *printer = w.findChild<QComboBox *>(QStringLiteral("printPrinter"));
    auto *copies = w.findChild<QSpinBox *>(QStringLiteral("printCopies"));
    auto *collate = w.findChild<QCheckBox *>(QStringLiteral("printCollate"));
    auto *paper = w.findChild<QLabel *>(QStringLiteral("printPaper"));
    auto *page = w.findChild<QLabel *>(QStringLiteral("printPage"));
    auto *text = w.findChild<QLineEdit *>(QStringLiteral("printRangeText"));
    auto *print = w.findChild<QPushButton *>(QStringLiteral("printButton"));
    QVERIFY(printer && copies && collate && paper && page && text && print);
    QVERIFY(printer->findText(QStringLiteral("Save as PDF")) >= 0);
    QCOMPARE(copies->value(), 1);
    QVERIFY(!collate->isEnabled());   // (one copy has nothing to collate)
    copies->setValue(3);
    QVERIFY(collate->isEnabled() && collate->isChecked());
    // The paper is the document's: its size and way up.
    QVERIFY(paper->text().endsWith(QStringLiteral("portrait")));
    // A range typed is chosen, and the preview and the count follow it.
    w.findChild<QRadioButton *>(QStringLiteral("printRangePages"))->click();
    text->setText(QStringLiteral("2-3"));
    QVERIFY(w.findChild<QRadioButton *>(QStringLiteral("printRangePages"))->isChecked());
    QTRY_VERIFY(page->text().startsWith(QStringLiteral("Page 2 of ")));
    QVERIFY(print->isEnabled());
    text->setText(QStringLiteral("x"));
    QVERIFY(!print->isEnabled());
    QVERIFY(page->text().contains(QStringLiteral("page numbers")));
}


void WriteTests::printingGoesThroughThePainterThePdfExportUses()
{
    QTemporaryDir dir;
    WriteWindow w;
    // Two sections: a portrait page, then landscape ones.
    {
        auto doc = WriteDoc::blank();
        QTextCursor c(doc->story());
        typeText(*doc, c, QStringLiteral("first section"));
        insertSectionBreak(*doc, c);
        typeText(*doc, c, QStringLiteral("a landscape page"));
        wml::SectionProps s = doc->sectionProps(1);
        s.pageW = 15840;
        s.pageH = 12240;
        s.orient = QStringLiteral("landscape");
        doc->setSectionProps(1, s);
        w.setDocument(std::move(doc), QString());
    }
    w.view()->paginateAll();
    QCOMPARE(w.view()->pageCount(), 2);
    struct Drawn {
        int number;
        bool output;
        TextOutput text;
        int pageCount;
        bool operator==(const Drawn &o) const { return number == o.number && output == o.output && text == o.text && pageCount == o.pageCount; }
    };
    QVector<Drawn> exported, printed;
    QVector<Drawn> *into = &exported;
    const ObserverGuard observer;
    write::setPaintObserver([&](const flow::MaterializedPage &page, const write::PageDrawing &d) { *into << Drawn{page.number, d.output, d.textOutput, d.pageCount}; });
    const QString pdfPath = dir.filePath(QStringLiteral("export.pdf")), printPath = dir.filePath(QStringLiteral("print.pdf"));
    QString next = pdfPath;
    w.setSavePathChooser([&](const QString &, const QString &) { return next; });
    QVERIFY(w.exportPdf());
    into = &printed;
    next = printPath;
    WriteOutputOptions o;
    o.toFile = true;
    QVERIFY(w.printOutput(o));
    write::setPaintObserver(nullptr);
    // The same pages, drawn for output; a printer's text is a printer's, a PDF export's a PDF's.
    QCOMPARE(exported.size(), 2);
    QCOMPARE(printed.size(), 2);
    for (int i = 0; i < 2; ++i) {
        QCOMPARE(printed[i].number, exported[i].number);
        QVERIFY(printed[i].output && exported[i].output);
        QCOMPARE(printed[i].pageCount, exported[i].pageCount);
        QCOMPARE(exported[i].text, TextOutput::Pdf);
        QCOMPARE(printed[i].text, TextOutput::Print);
    }
    // Each sheet is the size of its section's page, the landscape one turned.
    auto boxes = [](const QString &path) {
        QFile f(path);
        f.open(QIODevice::ReadOnly);
        const QByteArray pdf = f.readAll();
        QList<QSizeF> sizes;
        static const QRegularExpression box(QStringLiteral("/MediaBox\\s*\\[\\s*0\\s+0\\s+([0-9.]+)\\s+([0-9.]+)\\s*\\]"));
        auto it = box.globalMatch(QString::fromLatin1(pdf));
        while (it.hasNext()) {
            const auto m = it.next();
            sizes << QSizeF(m.captured(1).toDouble(), m.captured(2).toDouble());
        }
        return sizes;
    };
    const QList<QSizeF> a = boxes(pdfPath), b = boxes(printPath);
    QCOMPARE(a.size(), 2);
    QCOMPARE(b.size(), 2);
    for (int i = 0; i < 2; ++i) {
        QVERIFY2(qAbs(a[i].width() - b[i].width()) < 1 && qAbs(a[i].height() - b[i].height()) < 1, qPrintable(QStringLiteral("%1x%2 or %3x%4").arg(a[i].width()).arg(a[i].height()).arg(b[i].width()).arg(b[i].height())));
    }
    QVERIFY(b[0].height() > b[0].width());
    QVERIFY(b[1].width() > b[1].height());
}


void WriteTests::printingTakesThePagesOfTheRangeInTheOrderGiven()
{
    QTemporaryDir dir;
    WriteWindow w;
    w.setDocument(open(makeDocx(paragraphs(150), kStyles)), QString());
    w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("range.pdf")); });
    QVector<int> numbers;
    const ObserverGuard observer;
    write::setPaintObserver([&](const flow::MaterializedPage &page, const write::PageDrawing &) { numbers << page.number; });
    WriteOutputOptions o;
    o.toFile = true;
    o.range = WriteOutputOptions::Range::Pages;
    o.pages = QStringLiteral("3, 1-2");
    QVERIFY(w.printOutput(o));
    write::setPaintObserver(nullptr);
    QCOMPARE(numbers, (QVector<int>{3, 1, 2}));
    QFile f(dir.filePath(QStringLiteral("range.pdf")));
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray pdf = f.readAll();
    int pages = 0;
    for (qsizetype at = pdf.indexOf("/Type /Page\n"); at >= 0; at = pdf.indexOf("/Type /Page\n", at + 1)) ++pages;
    QCOMPARE(pages, 3);
    // A range that names no page prints nothing.
    o.pages = QStringLiteral("9999");
    QVERIFY(!w.printOutput(o));
}


// Pages on a sheet: four pages of the range share one sheet, two share one turned on its side, and a page scaled to a paper size makes a sheet of that size.
void WriteTests::severalPagesShareASheetAndAPageCanBeScaledToAPaperSize()
{
    QTemporaryDir dir;
    WriteWindow w;
    w.setDocument(open(makeDocx(paragraphs(150), kStyles)), QString());
    QString target;
    w.setSavePathChooser([&](const QString &, const QString &) { return target; });
    auto boxes = [](const QString &path) {
        QFile f(path);
        f.open(QIODevice::ReadOnly);
        const QByteArray pdf = f.readAll();
        QList<QSizeF> sizes;
        static const QRegularExpression box(QStringLiteral("/MediaBox\\s*\\[\\s*0\\s+0\\s+([0-9.]+)\\s+([0-9.]+)\\s*\\]"));
        auto it = box.globalMatch(QString::fromLatin1(pdf));
        while (it.hasNext()) {
            const auto m = it.next();
            sizes << QSizeF(m.captured(1).toDouble(), m.captured(2).toDouble());
        }
        return sizes;
    };
    WriteOutputOptions o;
    o.toFile = true;
    o.range = WriteOutputOptions::Range::Pages;
    o.pages = QStringLiteral("1-3");
    target = dir.filePath(QStringLiteral("one.pdf"));
    QVERIFY(w.printOutput(o));
    const QList<QSizeF> single = boxes(target);
    QCOMPARE(single.size(), 3);
    QVERIFY(single[0].height() > single[0].width());
    o.perSheet = 4;
    target = dir.filePath(QStringLiteral("four.pdf"));
    QVERIFY(w.printOutput(o));
    const QList<QSizeF> four = boxes(target);
    QCOMPARE(four.size(), 1);
    QVERIFY(qAbs(four[0].width() - single[0].width()) < 1 && qAbs(four[0].height() - single[0].height()) < 1);
    o.perSheet = 2;
    target = dir.filePath(QStringLiteral("two.pdf"));
    QVERIFY(w.printOutput(o));
    const QList<QSizeF> two = boxes(target);
    QCOMPARE(two.size(), 2);
    QVERIFY(two[0].width() > two[0].height());
    // The four on a sheet are drawn: the observer sees the three pages of the range, once each.
    QVector<int> numbers;
    {
        const ObserverGuard observer;
        write::setPaintObserver([&](const flow::MaterializedPage &page, const write::PageDrawing &) { numbers << page.number; });
        o.perSheet = 4;
        target = dir.filePath(QStringLiteral("four2.pdf"));
        QVERIFY(w.printOutput(o));
        write::setPaintObserver(nullptr);
    }
    QCOMPARE(numbers, (QVector<int>{1, 2, 3}));
    o.perSheet = 1;
    o.scaleTo = QSizeF(419.528, 595.276);   // (A5)
    target = dir.filePath(QStringLiteral("a5.pdf"));
    QVERIFY(w.printOutput(o));
    const QList<QSizeF> small = boxes(target);
    QCOMPARE(small.size(), 3);
    QVERIFY2(qAbs(small[0].width() - 419.5) < 1.5 && qAbs(small[0].height() - 595.3) < 1.5, qPrintable(QStringLiteral("%1x%2").arg(small[0].width()).arg(small[0].height())));
}


void WriteTests::aPrinterNamedPdfIsAPrinterAndTheFileEntryIsFoundByItsRole()
{
    QComboBox box;
    fillPrinterBox(&box, QStringList{QStringLiteral("Front Desk"), QStringLiteral("pdf"), QStringLiteral("Save as PDF")}, QStringLiteral("Front Desk"));
    QCOMPARE(box.count(), 4);
    QCOMPARE(printerChoice(&box).printer, QStringLiteral("Front Desk"));   // (the one to start on)
    for (int i = 0; i < 3; ++i) {
        box.setCurrentIndex(i);
        const PrinterChoice choice = printerChoice(&box);
        QVERIFY(!choice.toFile);
        QCOMPARE(choice.printer, box.itemText(i));   // (a printer called "pdf", or "Save as PDF", is a printer)
    }
    box.setCurrentIndex(3);
    QVERIFY(printerChoice(&box).toFile);
    QVERIFY(printerChoice(&box).printer.isEmpty());
    // With no printer there is the file alone.
    QComboBox none;
    fillPrinterBox(&none, QStringList(), QString());
    QCOMPARE(none.count(), 1);
    QVERIFY(printerChoice(&none).toFile);
}


void WriteTests::savingAsPdfThroughPrintLeavesAnExistingFileWholeUntilTheNewOneIsDone()
{
    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("old.pdf"));
    {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("the old file");
    }
    WriteWindow w;
    w.setDocument(open(makeDocx(paragraphs(60), kStyles)), QString());
    w.setSavePathChooser([&](const QString &, const QString &) { return path; });
    int drawn = 0, changed = 0;
    const ObserverGuard observer;
    write::setPaintObserver([&](const flow::MaterializedPage &, const write::PageDrawing &) {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly) || f.readAll() != QByteArray("the old file")) ++changed;
        ++drawn;
    });
    WriteOutputOptions o;
    o.toFile = true;
    QVERIFY(w.printOutput(o));
    write::setPaintObserver(nullptr);
    QVERIFY(drawn > 1);
    QCOMPARE(changed, 0);   // (the file is not touched while the pages are being made)
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QVERIFY(f.readAll().startsWith("%PDF"));
    QCOMPARE(QDir(dir.path()).entryList(QDir::Files).size(), 1);   // (and nothing is left beside it)
}


void WriteTests::copiesAndCollateReachAPrinterThatCannotMakeThem()
{
    QTemporaryDir dir;
    {
        QPrinter probe;
        probe.setOutputFormat(QPrinter::PdfFormat);
        if (probe.supportsMultipleCopies()) QSKIP("this printer engine makes copies itself, so the pages are not sent again");
    }
    WriteWindow w;
    w.setDocument(open(makeDocx(paragraphs(150), kStyles)), QString());
    // A printer that writes to a file stands in for a real one: the window's own way with a printer (not the file entry) is what runs.
    int made = 0;
    w.setPrinterPreparer([&](QPrinter &printer) {
        ++made;
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(dir.filePath(QStringLiteral("copies%1.pdf").arg(made)));
    });
    QVector<int> numbers;
    const ObserverGuard observer;
    write::setPaintObserver([&](const flow::MaterializedPage &page, const write::PageDrawing &) { numbers << page.number; });
    WriteOutputOptions o;
    o.printer = QStringLiteral("a printer");
    o.range = WriteOutputOptions::Range::Pages;
    o.pages = QStringLiteral("1-3");
    o.copies = 2;
    o.collate = true;
    QVERIFY(w.printOutput(o));
    QCOMPARE(numbers, (QVector<int>{1, 2, 3, 1, 2, 3}));
    numbers.clear();
    o.collate = false;
    QVERIFY(w.printOutput(o));
    QCOMPARE(numbers, (QVector<int>{1, 1, 2, 2, 3, 3}));
    numbers.clear();
    o.copies = 1;
    QVERIFY(w.printOutput(o));
    QCOMPARE(numbers, (QVector<int>{1, 2, 3}));
    // A PDF file is made once whatever the copies.
    numbers.clear();
    w.setPrinterPreparer(nullptr);
    w.setSavePathChooser([&](const QString &, const QString &) { return dir.filePath(QStringLiteral("file.pdf")); });
    o.toFile = true;
    o.copies = 3;
    QVERIFY(w.printOutput(o));
    QCOMPARE(numbers, (QVector<int>{1, 2, 3}));
}


// ---------------------------------------------------------------- help
void WriteTests::helpTopicsAreComplete()
{
    const QString src = QStringLiteral(JW_SOURCE_DIR "/resources/help/en/write");
    QStringList onDisk, built;
    for (QDirIterator it(src, QDir::Files); it.hasNext();) onDisk << QFileInfo(it.next()).fileName();
    for (QDirIterator it(QStringLiteral(":/help/en/write"), QDir::Files); it.hasNext();) built << QFileInfo(it.next()).fileName();
    onDisk.sort();
    built.sort();
    QCOMPARE(built, onDisk);   // resources.qrc lists every file (tools/help_files.py)
    const QStringList ids = jp::help::topicIds();
    QVERIFY2(ids.contains(QLatin1String("start")) && ids.contains(QLatin1String("index")), qPrintable(ids.join(QLatin1Char(' '))));
    static const QRegularExpression link(QStringLiteral("\\]\\(([^)]+)\\)"));
    static const QRegularExpression banned(QStringLiteral("\\b(Microsoft|Publisher|Office|Word)\\b"));
    const jp::help::Topic contents = jp::help::topic(QStringLiteral("index"));
    QStringList problems;
    for (const QString &id : ids) {
        const jp::help::Topic t = jp::help::topic(id);
        if (t.title.isEmpty()) problems << id + QStringLiteral(": no title");
        if (t.keywords.isEmpty()) problems << id + QStringLiteral(": no keywords");
        if (const auto m = banned.match(t.markdown); m.hasMatch()) problems << id + QStringLiteral(": says ") + m.captured(1);
        for (auto m = link.globalMatch(t.markdown); m.hasNext();) {
            const QString target = m.next().captured(1);
            if (!target.startsWith(QLatin1String("https://")) && !ids.contains(target)) problems << id + QStringLiteral(": link to ") + target;
        }
        if (id != QLatin1String("index") && !contents.markdown.contains(QStringLiteral("](%1)").arg(id))) problems << id + QStringLiteral(": not in the contents");
        QTextDocument doc;
        WriteWindow w;
        doc.setMarkdown(jp::help::displayMarkdown(t, &w), QTextDocument::MarkdownDialectGitHub);
        if (doc.toPlainText().trimmed().size() < 100) problems << id + QStringLiteral(": nearly empty");
    }
    QFile cf(src + QStringLiteral("/context.json"));
    QVERIFY(cf.open(QIODevice::ReadOnly));
    const QJsonObject context = QJsonDocument::fromJson(cf.readAll()).object();
    for (auto it = context.begin(); it != context.end(); ++it)
        if (!ids.contains(it.value().toString())) problems << QStringLiteral("context %1: no topic %2").arg(it.key(), it.value().toString());
    WriteWindow w;
    for (int i = 0; i < w.ribbon()->tabCount(); ++i)
        if (jp::help::contextTopic(QStringLiteral("tab:") + w.ribbon()->tabName(i)).isEmpty()) problems << QStringLiteral("no topic for the tab ") + w.ribbon()->tabName(i);
    QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QStringLiteral("; "))));
    // The keyboard topic's table comes from the commands themselves.
    const QString keys = jp::help::displayMarkdown(jp::help::topic(QStringLiteral("keyboard")), &w);
    QVERIFY(!keys.contains(QLatin1String("<!--")));
    QVERIFY2(keys.contains(QStringLiteral("| Save | %1 |").arg(QKeySequence(QKeySequence::Save).toString(QKeySequence::NativeText))), qPrintable(keys.right(1500)));
    QVERIFY(keys.contains(QStringLiteral("| Bold |")));
}


void WriteTests::f1OpensHelpOnWhatIsBeingDone()
{
    WriteWindow w;
    w.resize(1300, 800);
    w.show();
    QVERIFY(QTest::qWaitForWindowActive(&w));
    w.view()->setFocus();
    QTest::keyClick(w.view(), Qt::Key_F1);
    QVERIFY(w.helpView() && w.helpView()->isVisible());
    QCOMPARE(w.helpView()->currentTopic(), QStringLiteral("index"));
    w.showFileScreen(QStringLiteral("print"));
    w.act("help.show")->trigger();   // over the File screen: a window of its own, on that page's topic
    const auto *over = w.findChild<jp::HelpWindow *>();
    QVERIFY(over && over->isVisible());
    QCOMPARE(over->view()->currentTopic(), QStringLiteral("print"));
    // In the Spelling pane, help is on spelling.
    w.findChild<jp::HelpWindow *>()->close();
    w.hideFileScreen();
    w.activateWindow();
    QVERIFY(QTest::qWaitForWindowActive(&w));
    w.act("review.spelling")->trigger();
    w.findChild<SpellPane *>()->findChild<QListWidget *>(QStringLiteral("spellSuggestions"))->setFocus();
    w.act("help.show")->trigger();
    QCOMPARE(w.helpView()->currentTopic(), QStringLiteral("spelling"));
    // The start topic says what JeffWrite is and that it is a beta.
    const jp::help::Topic start = jp::help::topic(QStringLiteral("start"));
    QVERIFY(start.markdown.contains(QLatin1String("beta")));
    QVERIFY(start.markdown.contains(QLatin1String("What works")) && start.markdown.contains(QLatin1String("What does not work yet")));
}


// ---------------------------------------------------------------- the page view
void WriteTests::theViewShowsTheFirstPageAtOnceAndLaysOutTheRestInTheBackground()
{
    auto doc = open(makeDocx(paragraphs(3000), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    QVERIFY(view.pageCount() >= 1);
    view.show();
    QVERIFY(view.grab().width() == 1000);   // the first page is drawn
    // (Without waiting for the document: a few pages are laid out, not all of them.)
    QVERIFY2(doc->pager().pagesLaidOut() < 20 && !view.pagesComplete(), qPrintable(QString::number(doc->pager().pagesLaidOut())));
    QTRY_VERIFY_WITH_TIMEOUT(view.pagesComplete(), 60000);
    QVERIFY(view.pageCount() > 100);
    // The page count grew as pages were laid out, and the scroll range with it.
    QVERIFY(view.verticalScrollBar()->maximum() > 100 * 700);
}


// A page in the view that shows the number of pages is made again when that number changes: the first page, drawn while the rest is laid out, doesn't
// keep saying there are a few.
void WriteTests::aPageShowingTheTotalIsMadeAgainWhenTheTotalChanges()
{
    const QByteArray sect = "<w:sectPr><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
    const QByteArray footer = headerPart("ftr", {"<w:r><w:t xml:space=\"preserve\">of </w:t></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> NUMPAGES </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>"});
    auto doc = open(makeDocxWithParts(paragraphs(2000), sect, {{QStringLiteral("/word/footer1.xml"), footer}}));
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    view.grab();   // (the first page is made when only a few pages are known)
    QVERIFY(!view.pagesComplete());
    const int early = view.shownTotal(0);
    QVERIFY2(early > 0 && early < 100, qPrintable(QString::number(early)));
    QTRY_VERIFY_WITH_TIMEOUT(view.pagesComplete(), 60000);
    QVERIFY(view.pageCount() > 100);
    const int now = view.shownTotal(0);
    QVERIFY2(now < 0 || now == view.pageCount(), qPrintable(QStringLiteral("page 1 says %1 pages of %2").arg(now).arg(view.pageCount())));
}


void WriteTests::onlyThePagesInViewAreMade()
{
    auto doc = open(makeDocx(paragraphs(2000), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    QTRY_VERIFY_WITH_TIMEOUT(view.pagesComplete(), 60000);
    const qint64 before = doc->pager().pagesLaidOut();
    view.goToPage(60);
    view.grab();
    // Reaching page 60 and drawing it laid out no page again but the ones drawn (a page or two), not sixty.
    QVERIFY2(doc->pager().pagesLaidOut() - before < 12, qPrintable(QString::number(doc->pager().pagesLaidOut() - before)));
    QCOMPARE(view.topPage(), 60);
}


void WriteTests::zoomRunsFromTenToFiveHundredPercent()
{
    auto doc = open(makeDocx(paragraphs(400), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.paginateAll();
    view.setZoom(0.01);
    QCOMPARE(view.zoom(), 0.1);
    view.setZoom(50);
    QCOMPARE(view.zoom(), 5.0);
    view.setZoom(1.0);
    QCOMPARE(view.pageRect(0).width(), 816);   // 8.5 inches at 96 to the inch
    // Page width fits the page between the margins; one page fits it whole; three pages across.
    view.setZoomMode(PageView::ZoomMode::PageWidth);
    const QRect w = view.pageRect(0);
    QVERIFY(w.width() >= view.viewport()->width() - 2 * 24 - 2 && w.width() <= view.viewport()->width() - 2 * 24 + 2);
    view.setZoomMode(PageView::ZoomMode::OnePage);
    const QRect one = view.pageRect(view.topPage());
    QVERIFY(one.height() <= view.viewport()->height() - 2 * 24 + 2 && one.width() <= view.viewport()->width());
    view.setZoomMode(PageView::ZoomMode::ManyPages, 3, 1);
    QCOMPARE(view.pagesAcross(), 3);
    QCOMPARE(view.pageRect(0).top(), view.pageRect(2).top());
    QVERIFY(view.pageRect(1).left() > view.pageRect(0).right());
    QVERIFY(view.pageRect(3).top() > view.pageRect(0).bottom());
}


void WriteTests::thePagesAreDrawnWithTheirText()
{
    auto doc = open(makeDocx("<w:p><w:r><w:rPr><w:sz w:val=\"48\"/></w:rPr><w:t>HELLO</w:t></w:r></w:p>" + paragraphs(1), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    const QImage img = view.grab().toImage();
    // The gray ground, the white page, and dark text on it.
    const QRect page = view.pageRect(0);
    QVERIFY(page.width() > 700);
    int dark = 0, white = 0;
    for (int y = page.top(); y < std::min(page.bottom(), img.height()); ++y)
        for (int x = page.left(); x < page.right(); ++x) {
            const QColor c = img.pixelColor(x, y);
            if (c.lightness() < 100) ++dark;
            else if (c.lightness() > 250) ++white;
        }
    QVERIFY2(dark > 200, "text is drawn");
    QVERIFY(white > page.width() * page.height() / 2);
    QVERIFY(img.pixelColor(2, 300).lightness() < 240);   // (the ground is not white)
}


// View Gridlines draws the edges of the cells of a table that has no lines, on the screen: along the line between the two columns the page has marks with it on, and none without. (The
// strip that is looked at is the line's own, a little way from any text, so how the text's edges are drawn does not matter.)
void WriteTests::viewGridlinesDrawsTheEdgesOfCellsThatHaveNoLines()
{
    auto doc = open(makeDocx(tableX(rowX(cellX(cellP("aa")) + cellX(cellP("bb"))) + rowX(cellX(cellP("cc")) + cellX(cellP("dd"))), 2) + cellP("after"), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    view.paginateAll();
    view.grab();
    const auto page = doc->pager().materialize(0);
    QVERIFY(page && !page->tables.isEmpty());
    QRectF firstCell;
    for (const auto &cell : page->tables[0]->cellBoxes()) {
        firstCell = cell.box;
        break;
    }
    QVERIFY(firstCell.isValid());
    const QRect pr = view.pageRect(0);
    const double scale = pr.width() / page->size.width();
    const int x = int(std::lround(pr.left() + firstCell.right() * scale));
    const int y0 = int(std::lround(pr.top() + firstCell.top() * scale)) + 2, y1 = int(std::lround(pr.top() + firstCell.bottom() * scale)) - 2;
    QVERIFY(y1 - y0 > 6);
    auto marks = [&] {
        const QImage img = view.grab().toImage();
        int n = 0;
        for (int y = y0; y <= y1; ++y)
            for (int dx = -1; dx <= 1; ++dx) {
                const QColor c = img.pixelColor(x + dx, y);
                if (c.lightness() < 250) ++n;
            }
        return n;
    };
    QCOMPARE(marks(), 0);
    QVERIFY(!view.showGridlines());
    view.setShowGridlines(true);
    QVERIFY(view.showGridlines());
    QTRY_VERIFY2(marks() >= (y1 - y0) / 4, qPrintable(QString::number(marks())));
    view.setShowGridlines(false);
    QTRY_COMPARE(marks(), 0);
}


// Pulling the border at the right of a column with the mouse sets that column's width (one step that undoes), and the pointer shows a split cursor over the border.
void WriteTests::draggingAColumnBorderSetsTheColumnsWidth()
{
    auto doc = open(makeDocx(tableX(rowX(cellX(cellP("aa")) + cellX(cellP("bb"))) + rowX(cellX(cellP("cc")) + cellX(cellP("dd"))), 2) + cellP("after"), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    view.paginateAll();
    view.grab();
    const auto page = doc->pager().materialize(0);
    QVERIFY(page && !page->tables.isEmpty());
    QRectF firstCell;
    page->tables[0]->forCellBoxes([&](const QRectF &box, int, int, int) {
        if (!firstCell.isValid()) firstCell = box;
    });
    QVERIFY(firstCell.isValid());
    const QRect pr = view.pageRect(0);
    const double scale = pr.width() / page->size.width();
    const QPoint border(int(std::lround(pr.left() + firstCell.right() * scale)), int(std::lround(pr.top() + firstCell.center().y() * scale)));
    const QString before = tablesSignature(*doc);
    QVERIFY(before.contains(QStringLiteral("cols=120.0,120.0")));
    QTest::mouseMove(view.viewport(), border);
    QTRY_COMPARE(view.viewport()->cursor().shape(), Qt::SplitHCursor);
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, border);
    QTest::mouseMove(view.viewport(), border + QPoint(int(std::lround(30 * scale)), 0));
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, border + QPoint(int(std::lround(30 * scale)), 0));
    const QString now = tablesSignature(*doc);
    QVERIFY2(now.contains(QStringLiteral("cols=150.0,120.0")), qPrintable(now));
    checkTablesReopenTheSame(*doc);
    doc->story()->undo();
    QCOMPARE(tablesSignature(*doc), before);
}


// Draw Table: the pen draws a table of one cell where there is none, and a line across a cell divides it; the eraser joins cells it is dragged across; Esc puts the tool down.
void WriteTests::thePenDrawsATableAndDividesItsCellsAndTheEraserJoinsThem()
{
    auto doc = open(makeDocx(cellP("before text") + cellP("after text"), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    view.paginateAll();
    view.grab();
    auto counts = [&](int *tables, int *rows, int *cells) {
        QSet<const flow::TableData *> t;
        QSet<const flow::RowData *> r;
        QSet<int> c;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            if (const flow::CellRecord *rec = doc->flow().tables.get(cellKeyOf(b))) {
                t.insert(rec->table.get());
                r.insert(rec->row.get());
                c.insert(rec->key);
            }
        *tables = int(t.size());
        *rows = int(r.size());
        *cells = int(c.size());
    };
    int tables = 0, rows = 0, cells = 0;
    auto drag = [&](const QPoint &from, const QPoint &to) {
        QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(view.viewport(), to);
        QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, to);
        view.paginateAll();
        view.grab();
    };
    // Without the pen a drag selects text and makes no table.
    drag(pagePoint(view, 0, 100, 90), pagePoint(view, 0, 250, 150));
    counts(&tables, &rows, &cells);
    QCOMPARE(tables, 0);
    view.setTableTool(PageView::TableTool::Draw);
    QCOMPARE(view.viewport()->cursor().shape(), Qt::CrossCursor);
    // A rectangle drawn in the text: a table of one cell.
    view.setCursor(QTextCursor(doc->story()));
    drag(pagePoint(view, 0, 100, 90), pagePoint(view, 0, 250, 150));
    counts(&tables, &rows, &cells);
    QCOMPARE(tables, 1);
    QCOMPARE(rows, 1);
    QCOMPARE(cells, 1);
    QVERIFY(tablesSignature(*doc).contains(QStringLiteral("cols=150.0")));
    // A line across the cell makes two rows, and one down makes two columns.
    auto firstCell = [&] {
        const auto page = doc->pager().materialize(0);
        QRectF box;
        if (page && !page->tables.isEmpty()) page->tables[0]->forCellBoxes([&](const QRectF &b, int, int, int) {
            if (!box.isValid()) box = b;
        });
        return box;
    };
    QRectF box = firstCell();
    QVERIFY(box.isValid());
    drag(pagePoint(view, 0, box.left() + 10, box.center().y()), pagePoint(view, 0, box.right() - 10, box.center().y()));
    counts(&tables, &rows, &cells);
    QCOMPARE(rows, 2);
    QCOMPARE(cells, 2);
    checkTablesReopenTheSame(*doc);
    box = firstCell();
    drag(pagePoint(view, 0, box.center().x(), box.top() + 3), pagePoint(view, 0, box.center().x(), box.bottom() - 3));
    counts(&tables, &rows, &cells);
    QCOMPARE(cells, 3);
    QVERIFY(doc->story()->isUndoAvailable());
    // The eraser joins the two cells of the first row.
    view.setTableTool(PageView::TableTool::Erase);
    box = firstCell();
    const double middle = box.center().y();
    drag(pagePoint(view, 0, box.left() + 10, middle), pagePoint(view, 0, box.right() + 20, middle));
    counts(&tables, &rows, &cells);
    QCOMPARE(cells, 2);
    // Esc puts the tool down, and the pointer is the text's again.
    QTest::keyClick(&view, Qt::Key_Escape);
    QCOMPARE(view.tableTool(), PageView::TableTool::None);
    QVERIFY(view.viewport()->cursor().shape() != Qt::CrossCursor);
}


// Pulling the border at the foot of a row makes the row at least that tall (one step that undoes).
void WriteTests::draggingARowBorderSetsTheRowsHeight()
{
    auto doc = open(makeDocx(tableX(rowX(cellX(cellP("aa")) + cellX(cellP("bb"))) + rowX(cellX(cellP("cc")) + cellX(cellP("dd"))), 2) + cellP("after"), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    view.paginateAll();
    view.grab();
    const auto page = doc->pager().materialize(0);
    QVERIFY(page && !page->tables.isEmpty());
    QRectF firstCell;
    page->tables[0]->forCellBoxes([&](const QRectF &box, int, int, int) {
        if (!firstCell.isValid()) firstCell = box;
    });
    QVERIFY(firstCell.isValid());
    const QRect pr = view.pageRect(0);
    const double scale = pr.width() / page->size.width();
    const QPoint border(int(std::lround(pr.left() + firstCell.center().x() * scale)), int(std::lround(pr.top() + firstCell.bottom() * scale)));
    const QString before = tablesSignature(*doc);
    QVERIFY(before.contains(QStringLiteral("ROW height=0 exact=0")));
    QTest::mouseMove(view.viewport(), border);
    QTRY_COMPARE(view.viewport()->cursor().shape(), Qt::SplitVCursor);
    const QPoint to = border + QPoint(0, int(std::lround(30 * scale)));
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, border);
    QTest::mouseMove(view.viewport(), to);
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, to);
    const QString now = tablesSignature(*doc);
    const auto m = QRegularExpression(QStringLiteral("ROW height=([0-9.]+) exact=0")).match(now);
    QVERIFY2(m.hasMatch(), qPrintable(now));
    QVERIFY2(std::abs(m.captured(1).toDouble() - (firstCell.height() + 30)) < 2.0, qPrintable(m.captured(1)));
    checkTablesReopenTheSame(*doc);
    doc->story()->undo();
    QCOMPARE(tablesSignature(*doc), before);
}


// A press on a border that does not move is a click (the caret goes where it was pressed); the pull starts once the pointer has moved, and Escape lets it go.
void WriteTests::aPressOnABorderIsAClickUntilItMovesAndEscapeLetsGo()
{
    auto doc = open(makeDocx(tableX(rowX(cellX(cellP("aa")) + cellX(cellP("bb"))) + rowX(cellX(cellP("cc")) + cellX(cellP("dd"))), 2) + cellP("after"), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    view.paginateAll();
    view.grab();
    const auto page = doc->pager().materialize(0);
    QVERIFY(page && !page->tables.isEmpty());
    QRectF firstCell;
    for (const auto &cell : page->tables[0]->cellBoxes()) {
        firstCell = cell.box;
        break;
    }
    const QRect pr = view.pageRect(0);
    const double scale = pr.width() / page->size.width();
    const QPoint border(int(std::lround(pr.left() + firstCell.right() * scale)), int(std::lround(pr.top() + firstCell.center().y() * scale)));
    const QString before = tablesSignature(*doc);
    // A click on the border: no change, and the caret is in a cell of the table.
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, border);
    QCOMPARE(tablesSignature(*doc), before);
    QVERIFY(cellKeyOf(view.cursor().block()) != 0);
    // A pull that is let go with Escape changes nothing.
    const QPoint to = border + QPoint(int(std::lround(40 * scale)), 0);
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, border);
    QTest::mouseMove(view.viewport(), border + QPoint(QApplication::startDragDistance() + 2, 0));
    QTest::mouseMove(view.viewport(), to);
    QTest::keyClick(&view, Qt::Key_Escape);
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, to);
    QCOMPARE(tablesSignature(*doc), before);
    // The same pull without Escape does change it.
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, border);
    QTest::mouseMove(view.viewport(), border + QPoint(QApplication::startDragDistance() + 2, 0));
    QTest::mouseMove(view.viewport(), to);
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, to);
    QVERIFY(tablesSignature(*doc) != before);
}


// The pointer moving over a table asks for the cells' boxes of each fragment once, not at every move.
void WriteTests::movingThePointerOverATableMakesItsCellBoxesOnce()
{
    auto doc = open(makeDocx(tableX(rowX(cellX(cellP("aa")) + cellX(cellP("bb"))) + rowX(cellX(cellP("cc")) + cellX(cellP("dd"))), 2) + cellP("after"), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    view.paginateAll();
    view.grab();
    const QRect pr = view.pageRect(0);
    const qint64 before = flow::TableFragment::cellBoxListsMadeForTest();
    for (int i = 0; i < 200; ++i) QTest::mouseMove(view.viewport(), QPoint(pr.left() + 60 + i, pr.top() + 70 + i % 40));
    QVERIFY2(flow::TableFragment::cellBoxListsMadeForTest() - before <= 2, qPrintable(QString::number(flow::TableFragment::cellBoxListsMadeForTest() - before)));
}


// The handle at the table's lower right corner: pulling it sets the table's width, each column in the same proportion (one step that undoes).
void WriteTests::draggingTheTableHandleSetsTheTablesWidth()
{
    auto doc = open(makeDocx(tableX(rowX(cellX(cellP("aa")) + cellX(cellP("bb"))) + rowX(cellX(cellP("cc")) + cellX(cellP("dd"))), 2) + cellP("after"), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    QTextCursor c = cursorAt(*doc, QStringLiteral("aa"));
    view.cursor() = c;
    view.paginateAll();
    view.grab();
    const auto page = doc->pager().materialize(0);
    QVERIFY(page && !page->tables.isEmpty());
    const QRectF table = page->tables[0]->rect();
    const QRect pr = view.pageRect(0);
    const double scale = pr.width() / page->size.width();
    const QPoint corner(int(std::lround(pr.left() + (table.right() + 4) * scale)), int(std::lround(pr.top() + (table.bottom() + 4) * scale)));
    const QString before = tablesSignature(*doc);
    QVERIFY(before.contains(QStringLiteral("cols=120.0,120.0")));
    QTest::mouseMove(view.viewport(), corner);
    QTRY_COMPARE(view.viewport()->cursor().shape(), Qt::SizeFDiagCursor);
    const QPoint to = corner + QPoint(int(std::lround(60 * scale)), 0);
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, corner);
    QTest::mouseMove(view.viewport(), to);
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, to);
    QVERIFY2(tablesSignature(*doc).contains(QStringLiteral("cols=150.0,150.0")), qPrintable(tablesSignature(*doc)));
    checkTablesReopenTheSame(*doc);
    doc->story()->undo();
    QCOMPARE(tablesSignature(*doc), before);
}


void WriteTests::setTableWidthKeepsTheColumnsProportions()
{
    auto doc = open(tableDocx(2, 3));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r0c0"));
    setColumnWidth(*doc, c, 200);
    setTableWidth(*doc, c, 400);
    const QString now = tablesSignature(*doc);
    QVERIFY2(now.contains(QStringLiteral("cols=200.0,100.0,100.0")), qPrintable(now));
    setTableWidth(*doc, c, 200);
    QVERIFY2(tablesSignature(*doc).contains(QStringLiteral("cols=100.0,50.0,50.0")), qPrintable(tablesSignature(*doc)));
    checkTablesReopenTheSame(*doc);
}


// A modified table style reaches the tables inside cells, and a thousand tables of the style are made again in bounded work.
void WriteTests::modifyingATableStyleRestylesNestedAndManyTables()
{
    office::wml::TableStyleSpec spec;
    spec.name = QStringLiteral("Nest Style");
    spec.lineColor = QStringLiteral("FF0000");
    const QByteArray inner = tableX(rowX(cellX(cellP("n0")) + cellX(cellP("n1"))), 2).replace("<w:tblPr>", "<w:tblPr><w:tblStyle w:val=\"NestStyle\"/>");
    auto doc = open(makeDocx(tableX(rowX(cellX(cellP("A") + inner + cellP("Z")) + cellX(cellP("B"))), 2) + cellP("after"), kStyles));
    QString problem;
    QCOMPARE(createTableStyle(*doc, spec, &problem), QStringLiteral("NestStyle"));
    spec.lineColor = QStringLiteral("00FF00");
    QVERIFY(modifyTableStyle(*doc, QStringLiteral("NestStyle"), spec, &problem));
    QVERIFY2(tablesSignature(*doc).contains(QStringLiteral("#00ff00")), qPrintable(tablesSignature(*doc)));
    checkTablesReopenTheSame(*doc);
    QByteArray body;
    for (int i = 0; i < 1000; ++i) body += tableX(rowX(cellX(cellP(("h" + QByteArray::number(i))))), 1).replace("<w:tblPr>", "<w:tblPr><w:tblStyle w:val=\"NestStyle\"/>") + cellP("gap");
    auto big = open(makeDocx(body, kStyles));
    spec.lineColor = QStringLiteral("112233");
    QVERIFY(createTableStyle(*big, spec, &problem) == QStringLiteral("NestStyle"));
    spec.lineColor = QStringLiteral("445566");
    const qint64 before = tableIndexRebuildsForTest();
    QElapsedTimer timer;
    timer.start();
    QVERIFY(modifyTableStyle(*big, QStringLiteral("NestStyle"), spec, &problem));
    qWarning("restyle 1000 tables: %lld index rebuilds, %lld ms", tableIndexRebuildsForTest() - before, timer.elapsed());
    QVERIFY2(tableIndexRebuildsForTest() - before <= 5, qPrintable(QString::number(tableIndexRebuildsForTest() - before)));
    QCOMPARE(tablesSignature(*big).count(QStringLiteral("#445566")), 1000 * 6);   // (six lines of the table)
    QVERIFY(big->undo());
}


// The built-in table styles are shown by our own plain names on the screen; the names in the file (and the ids) are as they were.
void WriteTests::builtInTableStylesAreShownByOurOwnNames()
{
    QSet<QString> seen;
    for (const QString &id : QStringList{QStringLiteral("TableNormal")} + office::wml::builtinTableStyleIds()) {
        const office::wml::Style s = office::wml::builtinStyle(id);
        QVERIFY(!s.id.isEmpty());
        const QString shown = office::wml::tableStyleScreenName(s);
        QVERIFY2(!shown.isEmpty() && shown != s.name, qPrintable(id + " " + shown));
        QVERIFY2(!shown.contains(QStringLiteral("Table")) && !shown.contains(QStringLiteral(" - ")), qPrintable(shown));
        QVERIFY2(!seen.contains(shown), qPrintable(shown));
        seen.insert(shown);
    }
    QCOMPARE(office::wml::tableStyleScreenName(office::wml::builtinStyle(QStringLiteral("GridTable4-Accent1"))), QStringLiteral("Banded Grid, Blue"));
    QCOMPARE(office::wml::tableStyleScreenName(office::wml::builtinStyle(QStringLiteral("ListTable3-Accent2"))), QStringLiteral("Banded List, Orange"));
    QCOMPARE(office::wml::tableStyleScreenName(office::wml::builtinStyle(QStringLiteral("GridTable1Light"))), QStringLiteral("Light Grid"));
    // The names a file holds are not changed.
    QCOMPARE(office::wml::builtinStyle(QStringLiteral("GridTable4-Accent1")).name, QStringLiteral("Grid Table 4 - Accent 1"));
    // A file's own style under a name of its own is shown by it.
    office::wml::Style own = office::wml::builtinStyle(QStringLiteral("GridTable4-Accent1"));
    own.name = QStringLiteral("Quarter Report");
    QCOMPARE(office::wml::tableStyleScreenName(own), QStringLiteral("Quarter Report"));
}


// A file's own definition of a table style that has a built-in name is the one used; a style the file defines is editable (its name too) even without w:customStyle, a built-in one
// the file lacks keeps its name.
void WriteTests::aFilesOwnTableStyleWinsOverTheBuiltInOfTheSameName()
{
    auto line = [](const char *side) { return QByteArray("<w:") + side + " w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"00FF00\"/>"; };
    const QByteArray mine = "<w:style w:type=\"table\" w:styleId=\"GridTable4-Accent1\"><w:name w:val=\"Mine\"/><w:basedOn w:val=\"TableNormal\"/><w:tblPr><w:tblBorders>" + line("top") + line("left") + line("bottom") +
                            line("right") + line("insideH") + line("insideV") + "</w:tblBorders></w:tblPr></w:style>"
                            "<w:style w:type=\"table\" w:styleId=\"Plainish\"><w:name w:val=\"Plainish\"/><w:tblPr/></w:style>";
    QByteArray table = tableX(rowX(cellX(cellP("a")) + cellX(cellP("b"))), 2);
    table.replace("<w:tblPr>", "<w:tblPr><w:tblStyle w:val=\"GridTable4-Accent1\"/>");
    auto doc = open(makeDocx(table + cellP("after"), kStyles + mine));
    QVERIFY2(tablesSignature(*doc).contains(QStringLiteral("#00ff00")), qPrintable(tablesSignature(*doc)));
    QVERIFY(!tablesSignature(*doc).contains(QStringLiteral("#8eaadb")));
    QTextCursor c = cursorAt(*doc, QStringLiteral("a"));
    setTableStyle(*doc, c, QStringLiteral("GridTable4-Accent1"));
    QVERIFY(tablesSignature(*doc).contains(QStringLiteral("#00ff00")));
    const QByteArray styles = styleXml(doc->toDocx());
    QVERIFY(styles.contains("00FF00") && !styles.contains("8EAADB"));
    checkTablesReopenTheSame(*doc);
    // Editable: the file's style may be renamed, though it has no w:customStyle.
    office::wml::TableStyleSpec spec = office::wml::tableStyleSpecOf(*doc->styleSheet().find(QStringLiteral("Plainish")));
    spec.name = QStringLiteral("Renamed");
    QString problem;
    QVERIFY(modifyTableStyle(*doc, QStringLiteral("Plainish"), spec, &problem));
    QCOMPARE(doc->styleSheet().find(QStringLiteral("Plainish"))->name, QStringLiteral("Renamed"));
    QVERIFY(tableStyleNameIsEditable(*doc, QStringLiteral("Plainish")));
    QVERIFY(tableStyleNameIsEditable(*doc, QStringLiteral("GridTable4-Accent1")));   // (the file defines it)
    QVERIFY(!tableStyleNameIsEditable(*doc, QStringLiteral("GridTable4-Accent2")));   // (built in, the file has none)
    // A built-in style the file lacks keeps its name even after it is changed.
    office::wml::TableStyleSpec built = office::wml::tableStyleSpecOf(office::wml::builtinStyle(QStringLiteral("GridTable4-Accent2")));
    built.headerFill = QStringLiteral("123456");
    built.name = QStringLiteral("Other");
    QVERIFY(modifyTableStyle(*doc, QStringLiteral("GridTable4-Accent2"), built, &problem));
    QVERIFY(!tableStyleNameIsEditable(*doc, QStringLiteral("GridTable4-Accent2")));
    QVERIFY(styleXml(doc->toDocx()).contains("Grid Table 4 - Accent 2"));
}


// New Table Style makes a style from the choices (one step that undoes), a table can take it, and the file keeps it; Modify Table Style changes it in place.
void WriteTests::aTableStyleIsMadeAndModifiedFromChoices()
{
    auto doc = open(tableDocx(2, 2));
    QTextCursor c = cursorAt(*doc, QStringLiteral("r0c0"));
    office::wml::TableStyleSpec spec;
    spec.name = QStringLiteral("Report Grid");
    spec.basedOn = QStringLiteral("TableNormal");
    spec.lineColor = QStringLiteral("336699");
    spec.headerFill = QStringLiteral("FF0000");
    spec.headerText = QStringLiteral("FFFFFF");
    spec.bandedRowFill = QStringLiteral("EEEEEE");
    QString problem;
    const QString id = createTableStyle(*doc, spec, &problem);
    QVERIFY2(!id.isEmpty(), qPrintable(problem));
    QCOMPARE(id, QStringLiteral("ReportGrid"));
    QVERIFY(createTableStyle(*doc, spec, &problem).isEmpty());   // (the name is taken)
    QVERIFY(!problem.isEmpty());
    setTableStyle(*doc, c, id);
    QByteArray styles = styleXml(doc->toDocx());
    QVERIFY(styles.contains("w:styleId=\"ReportGrid\""));
    QVERIFY(styles.contains("w:fill=\"FF0000\"") && styles.contains("w:fill=\"EEEEEE\"") && styles.contains("w:color=\"336699\""));
    QVERIFY(documentXml(doc->toDocx()).contains("<w:tblStyle w:val=\"ReportGrid\"/>"));
    checkTablesReopenTheSame(*doc);
    auto back = open(doc->toDocx());
    const auto read = office::wml::tableStyleSpecOf(*back->styleSheet().find(QStringLiteral("ReportGrid")));
    QCOMPARE(read.headerFill, spec.headerFill);
    QCOMPARE(read.bandedRowFill, spec.bandedRowFill);
    QCOMPARE(read.lineColor, spec.lineColor);
    QVERIFY(read.headerBold);
    // Modify: another fill, one step.
    spec.headerFill = QStringLiteral("00AA00");
    spec.lines = office::wml::TableStyleSpec::Box;
    QVERIFY(modifyTableStyle(*doc, id, spec, &problem));
    styles = styleXml(doc->toDocx());
    QVERIFY(styles.contains("w:fill=\"00AA00\"") && !styles.contains("w:fill=\"FF0000\""));
    QVERIFY(!styles.contains("insideH"));
    checkTablesReopenTheSame(*doc);
    const QString signatureBefore = tablesSignature(*doc);
    QVERIFY(doc->undo());
    QVERIFY(styleXml(doc->toDocx()).contains("w:fill=\"FF0000\""));
    QVERIFY(tablesSignature(*doc) != signatureBefore);
    checkTablesReopenTheSame(*doc);
    QVERIFY(doc->redo());
    QCOMPARE(tablesSignature(*doc), signatureBefore);
    QVERIFY(doc->undo());
    // A built-in style is changed in place (and its name stays).
    setTableStyle(*doc, c, QStringLiteral("GridTable4-Accent1"));
    office::wml::TableStyleSpec built = office::wml::tableStyleSpecOf(office::wml::builtinStyle(QStringLiteral("GridTable4-Accent1")));
    QCOMPARE(built.headerFill, QStringLiteral("4472C4"));
    QVERIFY(modifyTableStyle(*doc, QStringLiteral("GridTable4-Accent1"), built, &problem));   // (unchanged: nothing happens)
    built.headerFill = QStringLiteral("123456");
    built.name = QStringLiteral("Other name");
    QVERIFY(modifyTableStyle(*doc, QStringLiteral("GridTable4-Accent1"), built, &problem));
    styles = styleXml(doc->toDocx());
    QVERIFY(styles.contains("w:fill=\"123456\"") && styles.contains("Grid Table 4 - Accent 1") && !styles.contains("Other name"));
    checkTablesReopenTheSame(*doc);
}


// Ready-Made Tables put a table with its words in at the cursor as one step.
void WriteTests::readyTablesPutATableWithItsWordsIn()
{
    auto doc = open(makeDocx(cellP("before") + cellP("after"), kStyles));
    QTextCursor c = cursorAt(*doc, QStringLiteral("after"));
    const QString before = storyText(*doc);
    QVERIFY(insertReadyTable(*doc, c, ReadyTable::List));
    QString now = tablesSignature(*doc);
    QVERIFY2(now.contains(QStringLiteral("cols=")), qPrintable(now));
    QCOMPARE(now.count(QStringLiteral("ROW ")), 5);
    QVERIFY2(storyText(*doc).contains(QStringLiteral("Item|Description|Quantity|Price")), qPrintable(storyText(*doc)));
    QVERIFY(documentXml(doc->toDocx()).contains("ListTable3-Accent1"));
    checkTablesReopenTheSame(*doc);
    QVERIFY(doc->undo());
    QCOMPARE(storyText(*doc), before);
    QVERIFY(tablesSignature(*doc).count(QStringLiteral("ROW ")) == 0);
    QVERIFY(doc->redo());
    QCOMPARE(tablesSignature(*doc), now);
    // A calendar has the days of its month under the days' names, 7 columns.
    auto cal = open(makeDocx(cellP("after"), kStyles));
    QTextCursor cc = cursorAt(*cal, QStringLiteral("after"));
    QVERIFY(insertReadyTable(*cal, cc, ReadyTable::Calendar, QDate(2026, 11, 15)));   // (November 2026 begins on a Sunday and has 30 days)
    now = tablesSignature(*cal);
    QCOMPARE(now.count(QStringLiteral("ROW ")), 6);
    QVERIFY(storyText(*cal).contains(QStringLiteral("1|2|3|4|5|6|7|8")));
    QVERIFY(storyText(*cal).contains(QStringLiteral("29|30")));
    checkTablesReopenTheSame(*cal);
    // In a table nothing is put in.
    auto in = open(tableDocx(2, 2));
    QTextCursor ic = cursorAt(*in, QStringLiteral("r0c0"));
    QVERIFY(!insertReadyTable(*in, ic, ReadyTable::Matrix));
}


void WriteTests::theCaretIsDrawnOnThePageThatHoldsIt()
{
    auto doc = open(makeDocx(paragraphs(120), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.paginateAll();
    QTextCursor &c = view.cursor();
    c.movePosition(QTextCursor::End);
    view.setCursor(c);
    const int page = view.currentPage();
    QCOMPARE(page, view.pageCount() - 1);
    QRect r;
    QVERIFY(view.caretRect(&r));
    QVERIFY(view.pageRect(page).adjusted(-2, -2, 2, 2).contains(r.center()));
    QVERIFY(view.topPage() >= page - 1);   // (the view scrolled to it)
}


// The caret and a click use the page's text boxes only: a header, a footer, and the numbers of the lines are boxes too, and are not the text's columns.
void WriteTests::theCaretAndClicksIgnoreTheHeaderAndTheLineNumbers()
{
    for (int scenario = 0; scenario < 4; ++scenario) {
        const bool header = scenario & 1, numbers = scenario & 2;
        QByteArray body;
        for (int i = 0; i < 12; ++i) body += plainP(QByteArray("paragraph number ") + QByteArray::number(i) + " has some words in it to fill a line and then another line of words");
        const QByteArray sect = sectionXml(12240, 15840, 1440, header ? "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>" : "", QByteArray(), numbers ? "<w:lnNumType w:countBy=\"1\" w:distance=\"360\"/>" : "");
        const QMap<QString, QByteArray> parts{{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>Header text</w:t></w:r>"})}};
        auto doc = open(makeDocxWithParts(body, sect, header ? parts : QMap<QString, QByteArray>()));
        QVERIFY(doc);
        PageView view;
        view.resize(1000, 900);
        view.show();
        view.setDocument(doc.get());
        view.paginateAll();
        int bad = 0, total = 0;
        for (const int position : {3, 60, 150, 400, 700, 1000}) {
            QTextCursor c(doc->story());
            c.setPosition(position);
            view.showCursor(c);
            QRect r;
            QVERIFY2(view.caretRect(&r), qPrintable(QStringLiteral("scenario %1: no caret at %2").arg(scenario).arg(position)));
            QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(r.center().x() + 1, r.center().y()));
            ++total;
            bad += std::abs(view.cursor().position() - position) > 1;
        }
        QVERIFY2(bad == 0, qPrintable(QStringLiteral("scenario %1 (header %2, line numbers %3): %4 of %5 clicks at a caret missed it").arg(scenario).arg(header).arg(numbers).arg(bad).arg(total)));
        // A click in the margin, level with the second line, puts the caret in that line (the line numbers are not where a click goes).
        QTextCursor c(doc->story());
        c.setPosition(100);
        view.showCursor(c);
        QRect r;
        QVERIFY(view.caretRect(&r));
        const QRect page = view.pageRect(0);
        QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(page.left() + 20, r.center().y()));
        QVERIFY2(view.cursor().position() > 40 && view.cursor().position() < 160, qPrintable(QStringLiteral("scenario %1: a click in the margin gave position %2").arg(scenario).arg(view.cursor().position())));
    }
}


void WriteTests::clickingPlacesTheCaretAndDraggingSelects()
{
    auto doc = open(makeDocx("<w:p><w:r><w:t>Alpha beta gamma delta</w:t></w:r></w:p><w:p><w:r><w:t>Second paragraph here</w:t></w:r></w:p>", kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    QRect page = view.pageRect(0);
    // A click at the left edge of the first line puts the caret at the start; one past its end, at the end.
    const QPoint first(page.left() + 72 * 96 / 72 + 2, page.top() + 72 * 96 / 72 + 8);
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, first);
    QCOMPARE(view.cursor().position(), 0);
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(page.right() - 20, first.y()));
    QCOMPARE(view.cursor().position(), int(QStringLiteral("Alpha beta gamma delta").size()));
    // A double click selects a word; a drag from the start to the second paragraph selects across.
    QTest::mouseDClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(first.x() + 4, first.y()));
    QCOMPARE(view.cursor().selectedText(), QStringLiteral("Alpha"));
    QTest::mousePress(view.viewport(), Qt::LeftButton, Qt::NoModifier, first);
    QTest::mouseMove(view.viewport(), QPoint(first.x() + 40, first.y() + 60));
    QTest::mouseRelease(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(first.x() + 40, first.y() + 60));
    QVERIFY(view.cursor().hasSelection());
    QVERIFY(view.cursor().selectionEnd() > 22);
}


// The caret is drawn in a cell where the cell's own layout puts it, a click in a cell puts it there, Up and Down go through the cell's lines and on to the cell above
// or below, and Home and End are to the cell's line.
void WriteTests::theCaretAndTheMouseWorkInCells()
{
    auto doc = open(tableDocx(3, 2));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    view.paginateAll();
    QTextDocument *s = doc->story();
    auto block = [&](const QString &text) { for (QTextBlock b = s->begin(); b.isValid(); b = b.next()) if (b.text() == text) return b; return QTextBlock(); };
    auto caretAt = [&](const QString &text, int offset) {
        view.cursor().setPosition(block(text).position() + offset);
        view.setCursor(view.cursor());
        QRect r;
        return view.caretRect(&r) ? r : QRect();
    };
    const QRect a = caretAt(QStringLiteral("r0c0"), 0), b = caretAt(QStringLiteral("r0c1"), 0), c = caretAt(QStringLiteral("r1c0"), 0), d = caretAt(QStringLiteral("r1c1"), 2);
    QVERIFY(a.isValid() && b.isValid() && c.isValid() && d.isValid());
    QVERIFY2(b.x() > a.x() + 100, "the second column is to the right of the first");
    QVERIFY2(c.y() > a.y() + 8 && std::abs(c.x() - a.x()) < 2, "the second row is below the first");
    QVERIFY(d.x() > c.x() + 100 && d.y() == c.y());
    QVERIFY(view.pageRect(0).contains(d.center()));
    // A click in a cell: the cell's text, at the place.
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(b.x() + 1, b.y() + b.height() / 2));
    QCOMPARE(view.cursor().block().text(), QStringLiteral("r0c1"));
    QCOMPARE(view.cursor().positionInBlock(), 0);
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(d.x() + 1, d.y() + 4));
    QCOMPARE(view.cursor().block().text(), QStringLiteral("r1c1"));
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(c.x() + 90, c.y() + 4));   // (past the end of "r1c0": its end)
    QCOMPARE(view.cursor().block().text(), QStringLiteral("r1c0"));
    QCOMPARE(view.cursor().positionInBlock(), 4);
    // Up and Down from a cell go to the cell above and below, along the same x.
    caretAt(QStringLiteral("r1c1"), 1);
    view.typeKey(Qt::Key_Up, Qt::NoModifier);
    QCOMPARE(view.cursor().block().text(), QStringLiteral("r0c1"));
    view.typeKey(Qt::Key_Down, Qt::NoModifier);
    QCOMPARE(view.cursor().block().text(), QStringLiteral("r1c1"));
    view.typeKey(Qt::Key_Down, Qt::NoModifier);
    QCOMPARE(view.cursor().block().text(), QStringLiteral("r2c1"));
    // Home and End are in the cell's line.
    view.typeKey(Qt::Key_End, Qt::NoModifier);
    QCOMPARE(view.cursor().positionInBlock(), 4);
    view.typeKey(Qt::Key_Home, Qt::NoModifier);
    QCOMPARE(view.cursor().positionInBlock(), 0);
    // Typing and Tab.
    view.insertText(QStringLiteral("Z"));
    QCOMPARE(view.cursor().block().text(), QStringLiteral("Zr2c1"));
    view.typeKey(Qt::Key_Backtab, Qt::NoModifier);
    QCOMPARE(view.cursor().selectedText(), QStringLiteral("r2c0"));
    // A selection across cells is drawn in them: the cells' text has the selection color somewhere.
    view.cursor().setPosition(block(QStringLiteral("r0c0")).position());
    view.cursor().setPosition(block(QStringLiteral("r1c1")).position() + 4, QTextCursor::KeepAnchor);
    view.setCursor(view.cursor());
    const QImage img = view.pageImage(0, 816);   // (the page as it is printed: the cells' text is in it)
    QVERIFY(!img.isNull());
    const QRect pr = view.pageRect(0);
    int dark = 0;
    for (int y = std::max(0, a.y() - pr.y() - 4); y < std::min(img.height(), d.bottom() - pr.y() + 30); ++y)
        for (int x = std::max(0, a.x() - pr.x() - 2); x < std::min(img.width(), d.right() - pr.x() + 150); ++x) dark += qGray(img.pixel(x, y)) < 128;
    QVERIFY2(dark > 300, qPrintable(QString::number(dark)));
    view.cursor().setPosition(0);
    QRect r;
    QVERIFY(view.caretRect(&r));
}


void WriteTests::theKeysMoveTheCaret()
{
    auto doc = open(makeDocx("<w:p><w:r><w:t>one two three</w:t></w:r></w:p><w:p><w:r><w:t>four five</w:t></w:r></w:p><w:p><w:r><w:t>six</w:t></w:r></w:p>", kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.typeKey(Qt::Key_End, Qt::ControlModifier);
    QCOMPARE(view.cursor().position(), doc->story()->characterCount() - 1);
    view.typeKey(Qt::Key_Home, Qt::ControlModifier);
    QCOMPARE(view.cursor().position(), 0);
    view.typeKey(Qt::Key_End, Qt::NoModifier);
    QCOMPARE(view.cursor().position(), 13);
    view.typeKey(Qt::Key_Down, Qt::NoModifier);
    QCOMPARE(view.cursor().blockNumber(), 1);
    view.typeKey(Qt::Key_Down, Qt::NoModifier);
    QCOMPARE(view.cursor().blockNumber(), 2);
    view.typeKey(Qt::Key_Up, Qt::NoModifier);
    QCOMPARE(view.cursor().blockNumber(), 1);
    // ("six" is shorter than the column the caret keeps, so Up went to the end of "four five": Ctrl+Left goes to the start of "five".)
    view.typeKey(Qt::Key_Left, Qt::ControlModifier);
    QCOMPARE(view.cursor().positionInBlock(), 5);
    view.typeKey(Qt::Key_Home, Qt::NoModifier);
    QCOMPARE(view.cursor().positionInBlock(), 0);
    view.typeKey(Qt::Key_Right, Qt::ShiftModifier);
    view.typeKey(Qt::Key_Right, Qt::ShiftModifier);
    QCOMPARE(view.cursor().selectedText(), QStringLiteral("fo"));
    // Up and Down keep the column they started from, across a short line.
    view.cursor().setPosition(doc->story()->findBlockByNumber(0).position() + 11);
    view.typeKey(Qt::Key_Down, Qt::NoModifier);   // "four five" has 9 letters: the end
    QCOMPARE(view.cursor().blockNumber(), 1);
    view.typeKey(Qt::Key_Down, Qt::NoModifier);   // "six" has 3
    QCOMPARE(view.cursor().blockNumber(), 2);
    view.typeKey(Qt::Key_Up, Qt::NoModifier);
    view.typeKey(Qt::Key_Up, Qt::NoModifier);
    QCOMPARE(view.cursor().positionInBlock(), 11);   // back where it began
}


void WriteTests::pageUpAndPageDownMoveByAWindow()
{
    auto doc = open(makeDocx(paragraphs(200), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    view.typeKey(Qt::Key_PageDown, Qt::NoModifier);
    const int a = view.cursor().position();
    QVERIFY(a > 500);
    view.typeKey(Qt::Key_PageDown, Qt::NoModifier);
    QVERIFY(view.cursor().position() > a);
    view.typeKey(Qt::Key_PageUp, Qt::NoModifier);
    QVERIFY(view.cursor().position() <= a + 40);
}


void WriteTests::upAndDownCrossFromPageToPage()
{
    auto doc = open(makeDocx(paragraphs(100), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.paginateAll();
    // To the last line of the first page, then one down: the second page.
    view.cursor().setPosition(doc->pager().page(1).start.pos - 1);
    QCOMPARE(view.currentPage(), 0);
    for (int i = 0; i < 3 && view.currentPage() == 0; ++i) view.typeKey(Qt::Key_Down, Qt::NoModifier);
    QCOMPARE(view.currentPage(), 1);
    for (int i = 0; i < 3 && view.currentPage() == 1; ++i) view.typeKey(Qt::Key_Up, Qt::NoModifier);
    QCOMPARE(view.currentPage(), 0);
}


void WriteTests::typingInTheWindowReachesTheDocument()
{
    WriteWindow w;
    w.show();
    w.view()->setFocus();
    QTest::keyClicks(w.view(), QStringLiteral("Hello"));
    QTest::keyClick(w.view(), Qt::Key_Return);
    QTest::keyClicks(w.view(), QStringLiteral("World"));
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Hello\nWorld"));
    QVERIFY(w.isModified());
    QVERIFY(w.windowTitle().contains(QLatin1Char('*')));
    QTest::keyClick(w.view(), Qt::Key_Backspace);
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Hello\nWorl"));
    QTest::keyClick(w.view(), Qt::Key_Home, Qt::ShiftModifier);
    QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("Worl"));
    QTest::keyClick(w.view(), Qt::Key_Delete);
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Hello\n"));
    // Undo and Redo from the ribbon's own commands.
    QTRY_VERIFY(w.act("edit.undo")->isEnabled());   // (the commands follow the text a moment after it changes)
    w.act("edit.undo")->trigger();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Hello\nWorl"));
    QTRY_VERIFY(w.act("edit.redo")->isEnabled());
    w.act("edit.redo")->trigger();
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("Hello\n"));
}


// ---------------------------------------------------------------- spelling
void WriteTests::spellCheckFindsTheMisspelledWordsOfTheStory()
{
    const SettingGuard upper(QStringLiteral("proof/ignoreUpper"), true, true), numbers(QStringLiteral("proof/ignoreNumbers"), true, true);
    auto doc = textDoc({"The quick brwon fox jumpd over teh lazy dog.", "A second paragraph, correctly spelled.", "NASA and abc123 are let go, so are the 3rd and 21st, but Teh is not."});
    SpellCheck sc(doc->story());
    QCOMPARE(flaggedWords(sc, *doc), (QStringList{"brwon", "jumpd", "teh", "Teh"}));
    // Switched off, it finds nothing; on again, the same.
    sc.setEnabled(false);
    QVERIFY(flaggedWords(sc, *doc).isEmpty());
    sc.setEnabled(true);
    QCOMPARE(flaggedWords(sc, *doc).size(), 4);
}


void WriteTests::spellCheckKeepsItsAnswerForEachParagraph()
{
    QStringList texts;
    for (int i = 0; i < 100; ++i) texts << QStringLiteral("Paragraph number %1 has a misspeled word in it.").arg(i);
    auto doc = textDoc(texts);
    QTextDocument *story = doc->story();
    SpellCheck sc(story);
    const QTextBlock third = story->findBlockByNumber(3);
    QCOMPARE(sc.misspelledIn(third.position(), third.position() + third.length()).size(), 1);
    QCOMPARE(sc.paragraphsChecked(), qint64(1));
    sc.misspelledIn(third.position(), third.position() + third.length());
    QCOMPARE(sc.paragraphsChecked(), qint64(1));   // (kept)
    const int end = story->characterCount();
    QCOMPARE(sc.misspelledIn(0, end).size(), 100);
    QCOMPARE(sc.paragraphsChecked(), qint64(100));
    // An edit in one paragraph makes only that one checked again, and the others move without being checked.
    QTextCursor c(story);
    c.setPosition(third.position() + 9);   // (the end of "Paragraph")
    insertText(*doc, c, QStringLiteral("s"));
    QCOMPARE(sc.misspelledIn(0, story->characterCount()).size(), 100);
    QCOMPARE(sc.paragraphsChecked(), qint64(101));
    c.setPosition(0);
    insertParagraph(*doc, c);
    QCOMPARE(sc.misspelledIn(0, story->characterCount()).size(), 100);
    QVERIFY2(sc.paragraphsChecked() <= 103, "a paragraph cut in two is checked, not the rest");
    // Undo brings the old text back; the answers for it are made again, and right.
    story->undo();
    story->undo();
    QCOMPARE(sc.misspelledIn(0, story->characterCount()).size(), 100);
}


void WriteTests::spellCheckGivesWayWhenThePassIsOutOfTimeAndFinishesInIdleTime()
{
    QStringList texts;
    for (int i = 0; i < 100; ++i) texts << QStringLiteral("Paragraph %1 has a misspeled word.").arg(i);
    auto doc = textDoc(texts);
    SpellCheck sc(doc->story());
    QSignalSpy changed(&sc, &SpellCheck::changed);
    const int end = doc->story()->characterCount();
    QVector<QPair<int, int>> some;
    {
        SpellCheck::Pass pass(sc, 0);
        some = sc.misspelledIn(0, end);
    }
    QVERIFY(some.size() < 100);
    QVERIFY(sc.paragraphsChecked() < 100);
    QVERIFY(sc.pending());
    // The rest is done when nothing else is: the event loop's idle time, and the view is told.
    QTRY_VERIFY(!sc.pending());
    QVERIFY(changed.count() > 0);
    QCOMPARE(sc.paragraphsChecked(), qint64(100));
    QCOMPARE(sc.misspelledIn(0, end).size(), 100);
    QCOMPARE(sc.paragraphsChecked(), qint64(100));
}


void WriteTests::spellCheckFollowsTheLanguageOfTheText()
{
    // "colour" is right in British English and wrong in American.
    const QByteArray body = "<w:p><w:r><w:rPr><w:lang w:val=\"en-GB\"/></w:rPr><w:t xml:space=\"preserve\">colour </w:t></w:r><w:r><w:t>colour</w:t></w:r></w:p>"
                            "<w:p><w:pPr><w:pStyle w:val=\"British\"/></w:pPr><w:r><w:t xml:space=\"preserve\">colour </w:t></w:r><w:r><w:rPr><w:lang w:val=\"en-US\"/></w:rPr><w:t>colour</w:t></w:r></w:p>"
                            "<w:p><w:r><w:rPr><w:lang w:val=\"tlh-KX\"/></w:rPr><w:t>qapla brwon</w:t></w:r></w:p>"
                            "<w:p><w:r><w:rPr><w:noProof/></w:rPr><w:t>brwon</w:t></w:r></w:p>";
    const QByteArray styles = kStyles + "<w:style w:type=\"paragraph\" w:styleId=\"British\"><w:name w:val=\"British\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:lang w:val=\"en-GB\"/></w:rPr></w:style>";
    auto doc = open(makeDocx(body, styles));
    SpellCheck sc(doc->story());
    sc.setDefaultLanguage(doc->proofingLanguage());
    const QString text = doc->story()->toPlainText();
    QStringList found;
    for (const auto &r : sc.misspelledIn(0, doc->story()->characterCount())) found << QStringLiteral("%1:%2").arg(doc->story()->findBlock(r.first).blockNumber()).arg(text.mid(r.first, r.second - r.first));
    // A run's own language, else its style's, else the document's (American): the second "colour" of the first paragraph, and the one said American in the second.
    // A language with no dictionary, and text marked "do not check", is let alone.
    QCOMPARE(found, (QStringList{"0:colour", "1:colour"}));
    // The document's own default (docDefaults) is what text that says none is: here British.
    QByteArray british = kStyles;
    british.replace("<w:sz w:val=\"23\"/></w:rPr></w:rPrDefault>", "<w:sz w:val=\"23\"/><w:lang w:val=\"en-GB\"/></w:rPr></w:rPrDefault>");
    auto doc2 = textDoc({"colour and brwon"}, british);
    QCOMPARE(doc2->proofingLanguage(), QStringLiteral("en-GB"));
    SpellCheck sc2(doc2->story());
    sc2.setDefaultLanguage(doc2->proofingLanguage());
    QCOMPARE(flaggedWords(sc2, *doc2), (QStringList{"brwon"}));
    // The file still says what it said.
    const QByteArray saved = documentXml(doc->toDocx());
    QVERIFY(saved.contains("<w:lang w:val=\"en-GB\"/>"));
    QVERIFY(saved.contains("<w:lang w:val=\"tlh-KX\"/>"));
}


void WriteTests::theIgnoreListAndTheUserDictionaryChangeTheAnswers()
{
    auto doc = textDoc({"The zorbifyx and the quixzotl are strange, but vlorpish is stranger."});
    SpellCheck sc(doc->story());
    QCOMPARE(flaggedWords(sc, *doc), (QStringList{"zorbifyx", "quixzotl", "vlorpish"}));
    spellingIgnore(QStringLiteral("zorbifyx"));
    QCOMPARE(flaggedWords(sc, *doc), (QStringList{"quixzotl", "vlorpish"}));
    // (Added words go in a file of the test's own.)
    {
        jptest::TestModeGuard testMode;
        spellingAdd(QStringLiteral("quixzotl"));
        QFile::remove(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/custom.dic"));
    }
    QCOMPARE(flaggedWords(sc, *doc), (QStringList{"vlorpish"}));
}


void WriteTests::misspellingsAreFoundByPositionAndInOrder()
{
    auto doc = textDoc({"One brwon fox", "and teh dog"});
    SpellCheck sc(doc->story());
    const auto first = sc.nextMisspelling(0);
    QCOMPARE(first, qMakePair(4, 9));
    // At or after a position: from inside the word, the word after it.
    const auto second = sc.nextMisspelling(first.second);
    QCOMPARE(doc->story()->toPlainText().mid(second.first, second.second - second.first), QStringLiteral("teh"));
    QCOMPARE(sc.nextMisspelling(second.second), qMakePair(-1, -1));
    // The word a position is in, or at the end of; not the one after the space.
    QCOMPARE(sc.misspellingAt(4), first);
    QCOMPARE(sc.misspellingAt(7), first);
    QCOMPARE(sc.misspellingAt(9), first);
    QCOMPARE(sc.misspellingAt(10), qMakePair(-1, -1));
    QCOMPARE(sc.misspellingAt(0), qMakePair(-1, -1));
}


void WriteTests::misspelledWordsAreUnderlinedInRedOnTheScreenAndNotInTheProof()
{
    auto doc = textDoc({"Alpha brwon gamma and a second line of words"});
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    // Where the word is on the page: the caret at its start and at its end.
    QTextCursor &c = view.cursor();
    c.setPosition(6);
    view.setCursor(c);
    QRect start;
    QVERIFY(view.caretRect(&start));
    c.setPosition(11);
    view.setCursor(c);
    QRect end;
    QVERIFY(view.caretRect(&end));
    c.setPosition(0);
    view.setCursor(c);
    // (Checked as the page is drawn, or a moment after.)
    int count = 0;
    QRect wave;
    QTRY_VERIFY((wave = redBounds(view.grab().toImage(), view.pageRect(0), &count), count > 8));
    // Under the word and nowhere else, just below the line of its letters.
    QVERIFY2(wave.left() >= start.left() - 3 && wave.right() <= end.left() + 3, qPrintable(QStringLiteral("%1..%2 for the word at %3..%4").arg(wave.left()).arg(wave.right()).arg(start.left()).arg(end.left())));
    QVERIFY(wave.top() >= start.top() && wave.bottom() <= start.bottom() + 8);
    // As printed, and in the proof the picture of a page makes, there is none.
    redBounds(view.pageImage(0, 900), QRect(0, 0, 900, 1200), &count);
    QCOMPARE(count, 0);
    // Switched off, it goes.
    view.setSpellingEnabled(false);
    redBounds(view.grab().toImage(), view.pageRect(0), &count);
    QCOMPARE(count, 0);
    view.setSpellingEnabled(true);
    QTRY_VERIFY((redBounds(view.grab().toImage(), view.pageRect(0), &count), count > 8));
}


// Red lines show under the misspelled words of a table's cells, as under those of the paragraphs; the words of a header are not underlined yet.
void WriteTests::misspelledWordsInATableCellAreUnderlinedAndThoseInAHeaderAreNot()
{
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray cell = "<w:tbl><w:tblPr><w:tblW w:w=\"6000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"6000\"/></w:tblGrid><w:tr><w:tc><w:tcPr><w:tcW w:w=\"6000\" w:type=\"dxa\"/></w:tcPr>"
                            "<w:p><w:r><w:t>Alpha brwon gamma</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>after</w:t></w:r></w:p>";
    const QByteArray header = headerPart("hdr", {"<w:r><w:t>Alpha brwon gamma</w:t></w:r>"});
    auto redIn = [&](const QByteArray &body, const QMap<QString, QByteArray> &parts) {
        auto doc = open(makeDocxWithParts(body, sect, parts));
        if (!doc) return -1;
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        view.show();
        int count = 0;
        for (int i = 0; i < 20 && count == 0; ++i) {   // (checked as the page is drawn, or a moment after)
            redBounds(view.grab().toImage(), view.pageRect(0), &count);
            QTest::qWait(20);
        }
        return count;
    };
    const int inCell = redIn(cell, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>plain words</w:t></w:r>"})}});
    QVERIFY2(inCell > 8, qPrintable(QStringLiteral("%1 red pixels under the misspelled word of a table's cell").arg(inCell)));
    const int inHeader = redIn("<w:p><w:r><w:t>after</w:t></w:r></w:p>", {{QStringLiteral("/word/header1.xml"), header}});
    QCOMPARE(inHeader, 0);
}


void WriteTests::aWordStillBeingTypedIsNotUnderlinedUntilTheCaretMovesOn()
{
    auto doc = textDoc({""});
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    int count = 0;
    Settings::get().setValue(QStringLiteral("proof/autocorrect"), false);   // (AutoCorrect would mend the slip)
    for (const QChar ch : QStringLiteral("recieve")) view.insertText(QString(ch));
    redBounds(view.grab().toImage(), view.pageRect(0), &count);
    QCOMPARE(count, 0);
    QCoreApplication::processEvents();   // (what idle time would do)
    QCoreApplication::processEvents();
    redBounds(view.grab().toImage(), view.pageRect(0), &count);
    QCOMPARE(count, 0);
    view.insertText(QStringLiteral(" "));
    QTRY_VERIFY((redBounds(view.grab().toImage(), view.pageRect(0), &count), count > 3));
    Settings::get().setValue(QStringLiteral("proof/autocorrect"), true);
}


void WriteTests::onlyTheParagraphsOnThePagesInViewAreChecked()
{
    auto doc = open(makeDocx(paragraphs(150), kStyles));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    view.grab();
    SpellCheck *spelling = view.spelling();
    QVERIFY(spelling);
    view.paginateAll();
    QTRY_VERIFY(!spelling->pending());
    const qint64 first = spelling->paragraphsChecked();
    QVERIFY2(first > 0 && first < 40, qPrintable(QString::number(first)));
    // A page further on is checked when it is drawn; the rest of the document, never.
    view.goToPage(4);
    view.grab();
    QTRY_VERIFY(!spelling->pending());
    QVERIFY(spelling->paragraphsChecked() > first);
    QVERIFY(spelling->paragraphsChecked() < 100);
    // A typed character: that paragraph is checked again, and no other.
    view.goToPage(0);
    view.grab();
    const qint64 before = spelling->paragraphsChecked();
    QTextCursor &c = view.cursor();
    c.setPosition(3);
    view.setCursor(c);
    view.insertText(QStringLiteral("x"));
    view.grab();
    QTRY_VERIFY(!spelling->pending());
    QCOMPARE(spelling->paragraphsChecked(), before + 1);
}


void WriteTests::theSpellingMenuOffersSuggestionsIgnoreAllAndAddToDictionary()
{
    auto doc = textDoc({"This is adress test, and this is fine."});
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    QTextCursor &c = view.cursor();
    // Off a misspelled word: nothing is added.
    c.setPosition(2);
    view.setCursor(c);
    QMenu none;
    QVERIFY(!addSpellingSuggestions(none, &view));
    QVERIFY(none.isEmpty());
    // On one (inside it, and at its end): the dictionary's first suggestions, then Ignore All and Add to Dictionary.
    for (int at : {10, 14}) {
        c.setPosition(at);
        view.setCursor(c);
        QMenu menu;
        QVERIFY(addSpellingSuggestions(menu, &view));
        QStringList texts;
        for (QAction *a : menu.actions()) texts << a->text();
        const QStringList wanted = spellingSuggestions(QStringLiteral("adress")).mid(0, 6);
        QVERIFY(!wanted.isEmpty());
        QCOMPARE(texts.mid(0, wanted.size()), wanted);
        QCOMPARE(texts[wanted.size()], QStringLiteral("Ignore All"));
        QCOMPARE(texts[wanted.size() + 1], QStringLiteral("Add to Dictionary"));
        QVERIFY(menu.actions().first()->font().bold());
    }
    // Choosing a suggestion changes the word, in one step that undoes.
    c.setPosition(10);
    view.setCursor(c);
    QMenu menu;
    QVERIFY(addSpellingSuggestions(menu, &view));
    const QString chosen = menu.actions().first()->text();
    menu.actions().first()->trigger();
    QCOMPARE(doc->story()->toPlainText(), QStringLiteral("This is %1 test, and this is fine.").arg(chosen));
    doc->story()->undo();
    QCOMPARE(doc->story()->toPlainText(), QStringLiteral("This is adress test, and this is fine."));
    // Ignore All lets the word go for the session, and the pages are drawn again.
    QMenu again;
    QVERIFY(addSpellingSuggestions(again, &view));
    QSignalSpy changed(view.spelling(), &SpellCheck::changed);
    for (QAction *a : again.actions())
        if (a->text() == QStringLiteral("Ignore All")) a->trigger();
    QCOMPARE(view.spelling()->misspellingAt(10), qMakePair(-1, -1));
    QVERIFY(changed.count() > 0);
}


void WriteTests::rightClickingAMisspelledWordInTheWindowShowsItsSuggestions()
{
    const SettingGuard asType(QStringLiteral("proof/asYouType"), true, true);
    WriteWindow w;
    w.setDocument(textDoc({"A word that is wrongg here."}), QString());
    w.show();
    QTextCursor &c = w.view()->cursor();
    c.setPosition(17);
    w.view()->setCursor(c);
    QStringList texts;
    QTimer::singleShot(100, [&] {
        if (auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
            for (QAction *a : m->actions()) texts << a->text();
            m->close();
        }
    });
    Q_EMIT w.view()->contextMenuWanted(w.view()->mapToGlobal(QPoint(300, 300)));
    QVERIFY2(texts.contains(QStringLiteral("Ignore All")) && texts.contains(QStringLiteral("Add to Dictionary")), qPrintable(texts.join(QLatin1Char('|'))));
    QVERIFY(texts.indexOf(QStringLiteral("Ignore All")) > 0);   // (the suggestions come first, then the usual commands)
    QVERIFY(texts.size() > texts.indexOf(QStringLiteral("Add to Dictionary")) + 3);
}


void WriteTests::reviewSpellingStepsThroughTheMisspellingsFromTheCaret()
{
    const SettingGuard asType(QStringLiteral("proof/asYouType"), true, true);
    WriteWindow w;
    w.setDocument(textDoc({"First wrongg word, and wrongg again.", "And a secnd one, plus fourh and then bluh."}), QString());
    w.show();
    QCOMPARE(w.act("review.spelling")->shortcut(), QKeySequence(Qt::Key_F7));
    w.act("review.spelling")->trigger();
    auto *pane = w.findChild<SpellPane *>();
    QVERIFY(pane && pane->isVisible());
    auto button = [&](const char *name) { return pane->findChild<QPushButton *>(QString::fromLatin1(name)); };
    auto *suggestions = pane->findChild<QListWidget *>(QStringLiteral("spellSuggestions"));
    QVERIFY(button("spellChange") && button("spellChangeAll") && button("spellIgnore") && button("spellIgnoreAll") && button("spellAdd") && suggestions);
    // The first misspelled word from the caret is shown, selected in the text, with the dictionary's suggestions.
    QCOMPARE(pane->word(), QStringLiteral("wrongg"));
    QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("wrongg"));
    QVERIFY(suggestions->count() > 0);
    QCOMPARE(suggestions->currentRow(), 0);
    QCOMPARE(pane->findChild<QLabel *>(QStringLiteral("spellWord"))->text(), QStringLiteral("wrongg"));
    // Change All puts the chosen word in place of every one like it, in one step that undoes.
    const QString chosen = suggestions->currentItem()->text();
    button("spellChangeAll")->click();
    QCOMPARE(w.document()->story()->toPlainText().section(QLatin1Char('\n'), 0, 0), QStringLiteral("First %1 word, and %1 again.").arg(chosen));
    QCOMPARE(pane->word(), QStringLiteral("secnd"));
    w.document()->story()->undo();
    QCOMPARE(w.document()->story()->toPlainText().section(QLatin1Char('\n'), 0, 0), QStringLiteral("First wrongg word, and wrongg again."));   // (both come back)
    w.document()->story()->redo();
    // Ignore passes this one by; Change takes the chosen suggestion; the pane goes on.
    button("spellIgnore")->click();
    QCOMPARE(pane->word(), QStringLiteral("fourh"));
    QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("fourh"));
    suggestions->setCurrentRow(0);
    const QString fix = suggestions->currentItem()->text();
    button("spellChange")->click();
    QVERIFY(w.document()->story()->toPlainText().contains(QStringLiteral("plus %1 and then").arg(fix)));
    QCOMPARE(pane->word(), QStringLiteral("bluh"));
    // Ignore All, and Add to Dictionary, take the word out for good; when none is left the check is complete.
    {
        jptest::TestModeGuard testMode;
        button("spellAdd")->click();
        QFile::remove(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/custom.dic"));
    }
    QCOMPARE(pane->word(), QString());
    QVERIFY(pane->findChild<QLabel *>(QStringLiteral("spellStatus"))->text().contains(QStringLiteral("complete")));
    QVERIFY(!button("spellChange")->isEnabled() && !button("spellIgnore")->isEnabled());
    // (The check came back to where it began: the one left by Ignore is still marked on the page.)
    QCOMPARE(w.view()->spelling()->misspellingAt(w.document()->story()->toPlainText().indexOf(QStringLiteral("secnd"))).first, w.document()->story()->toPlainText().indexOf(QStringLiteral("secnd")));
}


void WriteTests::reviewSpellingBeginsAtTheCaretAndComesBackToWhereItBegan()
{
    const SettingGuard asType(QStringLiteral("proof/asYouType"), true, true);
    WriteWindow w;
    w.setDocument(textDoc({"Zzalpha is first.", "Then zzbeta follows.", "And zzgamma last."}), QString());
    w.show();
    const QString text = w.document()->story()->toPlainText();
    QTextCursor &c = w.view()->cursor();
    c.setPosition(text.indexOf(QStringLiteral("zzbeta")));
    w.view()->setCursor(c);
    w.act("review.spelling")->trigger();
    auto *pane = w.findChild<SpellPane *>();
    QVERIFY(pane);
    auto ignore = [&] { pane->findChild<QPushButton *>(QStringLiteral("spellIgnore"))->click(); };
    QCOMPARE(pane->word(), QStringLiteral("zzbeta"));
    ignore();
    QCOMPARE(pane->word(), QStringLiteral("zzgamma"));
    ignore();
    QCOMPARE(pane->word(), QStringLiteral("Zzalpha"));   // (from the top)
    ignore();
    QCOMPARE(pane->word(), QString());
    QVERIFY(pane->complete());
    // Ignore All on the last one of a document with one left.
    w.setDocument(textDoc({"Only zzlonely here."}), QString());
    w.act("review.spelling")->trigger();
    QCOMPARE(w.findChild<SpellPane *>()->word(), QStringLiteral("zzlonely"));
    w.findChild<SpellPane *>()->findChild<QPushButton *>(QStringLiteral("spellIgnoreAll"))->click();
    QVERIFY(w.findChild<SpellPane *>()->complete());
    QCOMPARE(w.view()->spelling()->misspellingAt(6), qMakePair(-1, -1));
}


// Text from a document that looks like markup naming a picture (around a word the spelling pane stops at, or the name of a file that can't be opened) is shown
// as the text it is: no label or message box takes it for markup.
void WriteTests::hostileDocumentTextIsShownAsText()
{
    using plainprobe::readsAPicture;
    QVERIFY(plainprobe::probeSeesTheTrap());
    const SettingGuard asType(QStringLiteral("proof/asYouType"), true, true);
    WriteWindow w;
    w.setDocument(textDoc({QStringLiteral("zzlonely ") + plainprobe::hostile().toHtmlEscaped()}), QString());
    w.show();
    QTextCursor &start = w.view()->cursor();
    start.setPosition(0);
    w.view()->setCursor(start);
    w.act("review.spelling")->trigger();
    auto *pane = w.findChild<SpellPane *>();
    QVERIFY(pane);
    QVERIFY(!pane->word().isEmpty());   // (which word it stops at first depends on the computer's word lists: any of them shows the text around it)
    auto *wordLabel = pane->findChild<QLabel *>(QStringLiteral("spellWord"));
    QVERIFY(wordLabel);
    QCOMPARE(wordLabel->textFormat(), Qt::PlainText);
    int around = 0;
    for (const QLabel *l : pane->findChildren<QLabel *>()) {
        QVERIFY2(!readsAPicture(l), qPrintable(l->text()));
        around += l->text().contains(QStringLiteral("hostname"));
    }
    QVERIFY(around >= 1);   // the words around it are shown
    // The box that says a file can't be opened.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile f(dir.filePath(QStringLiteral("bad.docx")));
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("not a document");
    f.close();
    bool seen = false, plain = false;
    QTimer::singleShot(0, [&] {
        if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
            seen = true;
            plain = box->textFormat() == Qt::PlainText && !readsAPicture(box);
            box->close();
        }
    });
    w.openFile(f.fileName());
    QVERIFY(seen);
    QVERIFY(plain);
}


void WriteTests::theSpellingPaneNeverHoldsTheWindowForAWholeDocument()
{
    const SettingGuard asType(QStringLiteral("proof/asYouType"), true, true);
    WriteWindow w;
    w.setDocument(open(makeDocx(paragraphs(1500) + "<w:p><w:r><w:t>and one recieve at the very end</w:t></w:r></w:p>", kStyles)), QString());
    w.show();
    w.act("review.spelling")->trigger();   // (back at once, with what a slice of time could check)
    auto *pane = w.findChild<SpellPane *>();
    QVERIFY(pane);
    QVERIFY(pane->searching());
    QVERIFY(!pane->complete());
    QVERIFY(pane->findChild<QLabel *>(QStringLiteral("spellStatus"))->text().contains(QStringLiteral("Checking")));
    QVERIFY(!pane->findChild<QPushButton *>(QStringLiteral("spellIgnore"))->isEnabled());
    // The window goes on working, and the pane gets to the word.
    QTRY_COMPARE_WITH_TIMEOUT(pane->word(), QStringLiteral("recieve"), 30000);
    QVERIFY(!pane->searching());
    QVERIFY(pane->findChild<QPushButton *>(QStringLiteral("spellIgnore"))->isEnabled());
    QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("recieve"));
}


void WriteTests::changeAllGoesOnFromTheWordWhateverTheEarlierOnesChangedBy()
{
    const SettingGuard asType(QStringLiteral("proof/asYouType"), true, true);
    WriteWindow w;
    w.setDocument(textDoc({"wrongg wrongg wrongg", "zztwo wrongg zzthree"}), QString());
    w.show();
    QTextCursor &c = w.view()->cursor();
    c.setPosition(w.document()->story()->findBlockByNumber(1).position());
    w.view()->setCursor(c);
    w.act("review.spelling")->trigger();
    auto *pane = w.findChild<SpellPane *>();
    auto click = [&](const char *name) { pane->findChild<QPushButton *>(QString::fromLatin1(name))->click(); };
    QCOMPARE(pane->word(), QStringLiteral("zztwo"));
    click("spellIgnore");
    QCOMPARE(pane->word(), QStringLiteral("wrongg"));
    auto *suggestions = pane->findChild<QListWidget *>(QStringLiteral("spellSuggestions"));
    QVERIFY(suggestions->count() > 0);
    const QString chosen = suggestions->item(0)->text();
    QVERIFY2(chosen.size() != 6, "a suggestion of another length shifts what follows");
    click("spellChangeAll");
    QCOMPARE(w.document()->story()->toPlainText(), QStringLiteral("%1 %1 %1\nzztwo %1 zzthree").arg(chosen));
    // The next is the one just after the word, though the three before it changed length; then the check ends where it began, not at "zztwo" again.
    QCOMPARE(pane->word(), QStringLiteral("zzthree"));
    click("spellIgnore");
    QVERIFY2(pane->complete(), qPrintable(pane->word()));
}


void WriteTests::checkingSpellingAsYouTypeCanBeTurnedOffInTheReviewTab()
{
    const SettingGuard asType(QStringLiteral("proof/asYouType"), true, true);   // (what the test begins with, and what the person had comes back)
    WriteWindow w;
    w.setDocument(textDoc({"A recieve here."}), QString());
    w.show();
    QAction *action = w.act("review.checkAsType");
    QVERIFY(action->isCheckable() && action->isChecked());
    QVERIFY(w.view()->spellingEnabled());
    action->trigger();
    QVERIFY(!action->isChecked());
    QVERIFY(!w.view()->spellingEnabled());
    int count = 0;
    redBounds(w.view()->grab().toImage(), w.view()->pageRect(0), &count);
    QCOMPARE(count, 0);
    // A window opened after remembers the choice.
    WriteWindow other;
    QVERIFY(!other.view()->spellingEnabled());
    action->trigger();
    QVERIFY(w.view()->spellingEnabled());
    WriteWindow third;
    QVERIFY(third.view()->spellingEnabled());
}


void WriteTests::aStyleThatSetsALanguageGivesItToTheTextAndTakesItAway()
{
    const QByteArray styles = kStyles +
                              "<w:style w:type=\"paragraph\" w:styleId=\"British\"><w:name w:val=\"British\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:lang w:val=\"en-GB\"/></w:rPr></w:style>"
                              "<w:style w:type=\"character\" w:styleId=\"BritishChar\"><w:name w:val=\"BritishChar\"/><w:rPr><w:lang w:val=\"en-GB\"/></w:rPr></w:style>";
    const QByteArray body = "<w:p><w:r><w:t>colour</w:t></w:r></w:p><w:p><w:r><w:t>colour</w:t></w:r></w:p><w:p><w:r><w:rPr><w:lang w:val=\"en-GB\"/></w:rPr><w:t>colour</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, styles));
    QTextDocument *story = doc->story();
    SpellCheck sc(story);
    sc.setDefaultLanguage(doc->proofingLanguage());
    auto flagged = [&] {
        QList<int> blocks;
        for (const auto &r : sc.misspelledIn(0, story->characterCount())) blocks << story->findBlock(r.first).blockNumber();
        return blocks;
    };
    QCOMPARE(flagged(), (QList<int>{0, 1}));
    // A paragraph style that says British gives it to the paragraph's text; another style takes it away again. What a run says itself stays.
    QTextCursor c(story);
    c.setPosition(story->findBlockByNumber(0).position());
    applyParagraphStyle(*doc, c, QStringLiteral("British"));
    QCOMPARE(flagged(), (QList<int>{1}));
    applyParagraphStyle(*doc, c, QStringLiteral("Normal"));
    QCOMPARE(flagged(), (QList<int>{0, 1}));
    c.setPosition(story->findBlockByNumber(2).position());
    applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
    QCOMPARE(flagged(), (QList<int>{0, 1}));   // (its own British stays)
    applyParagraphStyle(*doc, c, QStringLiteral("Normal"));
    // A character style does the same for the letters it is given to.
    c.setPosition(story->findBlockByNumber(1).position());
    c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    applyCharacterStyle(*doc, c, QStringLiteral("BritishChar"));
    QCOMPARE(flagged(), (QList<int>{0}));
    // Clearing the formatting leaves a run's own language, as the file keeps it.
    c.setPosition(story->findBlockByNumber(2).position());
    c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    clearFormatting(*doc, c);
    QCOMPARE(flagged(), (QList<int>{0}));
    QVERIFY(documentXml(doc->toDocx()).contains("<w:lang w:val=\"en-GB\"/>"));
}


void WriteTests::searchingForMisspellingsGoesInSlicesOfBoundedWork()
{
    QStringList texts;
    for (int i = 0; i < 400; ++i) texts << (i == 300 ? QStringLiteral("One recieve here.") : QStringLiteral("Paragraph %1 is correct and clean.").arg(i));
    auto doc = textDoc(texts);
    QTextDocument *story = doc->story();
    SpellCheck sc(story);
    int from = 0, slices = 0, next = 0;
    QPair<int, int> found{-1, -1};
    qint64 before = sc.paragraphsChecked();
    SpellCheck::Step step;
    while ((step = sc.search(from, 0, &found, &next)) == SpellCheck::Step::More) {
        ++slices;
        // (A slice with no time to spend does the work of one paragraph, so that the search goes on.)
        QCOMPARE(sc.paragraphsChecked() - before, qint64(1));
        before = sc.paragraphsChecked();
        QVERIFY(next > from);
        from = next;
        QVERIFY(slices < 1000);
    }
    QCOMPARE(step, SpellCheck::Step::Found);
    QCOMPARE(slices, 300);
    QCOMPARE(story->toPlainText().mid(found.first, found.second - found.first), QStringLiteral("recieve"));
    // On from the word: the rest of the story has none, and the search says so.
    from = found.second;
    while ((step = sc.search(from, 0, &found, &next)) == SpellCheck::Step::More) from = next;
    QCOMPARE(step, SpellCheck::Step::None);
    // What was checked is kept: the whole story again takes one slice and no checking.
    before = sc.paragraphsChecked();
    QCOMPARE(sc.search(0, 0, &found, &next), SpellCheck::Step::Found);
    QCOMPARE(sc.search(found.second, 0, &found, &next), SpellCheck::Step::None);
    QCOMPARE(sc.paragraphsChecked(), before);
}


void WriteTests::aLongParagraphIsCheckedInPiecesOfBoundedWork()
{
    QString text;
    for (int i = 0; i < 30000; ++i) text += (i % 1000 == 999 ? QStringLiteral("recieve ") : QStringLiteral("word "));
    auto doc = textDoc({text});
    QTextDocument *story = doc->story();
    SpellCheck sc(story);
    const int end = story->characterCount();
    {
        SpellCheck::Pass pass(sc, 0);
        sc.misspelledIn(0, end);
    }
    // A pass with no time does one piece of it, not the whole; the rest is for idle time.
    QVERIFY2(sc.wordsChecked() > 0 && sc.wordsChecked() < 3000, qPrintable(QString::number(sc.wordsChecked())));
    QVERIFY(sc.pending());
    QTRY_VERIFY(!sc.pending());
    QCOMPARE(sc.wordsChecked(), qint64(30000));   // (each word once, though it took pieces)
    QCOMPARE(sc.misspelledIn(0, end).size(), 30);
    QCOMPARE(sc.paragraphsChecked(), qint64(1));
    // A typed character in it: the same, one piece within the pass.
    QTextCursor c(story);
    c.setPosition(5);
    insertText(*doc, c, QStringLiteral("s"));
    const qint64 before = sc.wordsChecked();
    {
        SpellCheck::Pass pass(sc, 0);
        sc.misspelledIn(0, story->characterCount());
    }
    QVERIFY2(sc.wordsChecked() - before > 0 && sc.wordsChecked() - before < 3000, qPrintable(QString::number(sc.wordsChecked() - before)));
    QTRY_VERIFY(!sc.pending());
    QCOMPARE(sc.misspelledIn(0, story->characterCount()).size(), 30);
    QCOMPARE(sc.paragraphsChecked(), qint64(2));
    // The search, too, goes through it in pieces: a misspelling near the start is found without checking the rest.
    QTextCursor d(story);
    d.setPosition(7);
    insertText(*doc, d, QStringLiteral("s"));   // ("swsord")
    const qint64 beforeSearch = sc.wordsChecked();
    QPair<int, int> found;
    int next = 0;
    QCOMPARE(sc.search(0, 0, &found, &next), SpellCheck::Step::Found);
    QVERIFY2(sc.wordsChecked() - beforeSearch < 3000, qPrintable(QString::number(sc.wordsChecked() - beforeSearch)));
    QCOMPARE(story->toPlainText().mid(found.first, found.second - found.first), QStringLiteral("swsord"));
}


void WriteTests::theSpellingCacheDoesNotKeepTheTextOfTheDocument()
{
    QStringList texts;
    for (int i = 0; i < 2000; ++i) texts << QStringLiteral("Paragraph number %1 is made of ordinary words that are all spelled right, and it goes on for a while so that it is long enough to count.").arg(i);
    auto doc = textDoc(texts);
    SpellCheck sc(doc->story());
    sc.misspelledIn(0, doc->story()->characterCount());
    QCOMPARE(sc.paragraphsChecked(), qint64(2000));
    const qint64 textBytes = qint64(doc->story()->characterCount()) * 2;
    QVERIFY2(sc.keptBytes() * 2 < textBytes, qPrintable(QStringLiteral("%1 bytes kept for %2 bytes of text").arg(sc.keptBytes()).arg(textBytes)));
}


void WriteTests::theUserDictionaryIsReadAndWrittenSafely()
{
    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("custom.dic"));
    auto put = [&](const QByteArray &bytes) {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write(bytes);
    };
    auto get = [&] {
        QFile f(path);
        return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
    };
    // A file that doesn't end with a newline: the next word begins a line of its own.
    put("alpha\nbeta");
    QString error;
    QVERIFY(addToUserDictionary(path, QStringLiteral("gamma"), &error));
    QCOMPARE(get(), QByteArray("alpha\nbeta\ngamma\n"));
    QCOMPARE(userDictionaryWords(path), (QStringList{"alpha", "beta", "gamma"}));
    // A word that is empty or has a line in it is not written, so that no two words are made of one.
    QVERIFY(!addToUserDictionary(path, QStringLiteral("  "), &error));
    QVERIFY(!addToUserDictionary(path, QStringLiteral("two\nwords"), &error));
    QCOMPARE(get(), QByteArray("alpha\nbeta\ngamma\n"));
    // A byte order mark, Windows line ends, blanks, and spaces around a word.
    put("\xEF\xBB\xBF" "one\r\n\r\n  two \r\nthree");
    QCOMPARE(userDictionaryWords(path), (QStringList{"one", "two", "three"}));
    // The read is bounded: a file of a great many words is read as far as the limit.
    QByteArray big;
    for (int i = 0; i < 400000; ++i) big += "word" + QByteArray::number(i) + "\n";
    put(big);
    const int words = int(userDictionaryWords(path).size());
    QVERIFY2(words > 10000 && words < 400000, qPrintable(QString::number(words)));
    // A file that can't be written is said, with the reason.
    QVERIFY(!addToUserDictionary(dir.filePath(QStringLiteral("none/custom.dic")), QStringLiteral("x"), &error));
    QVERIFY(!error.isEmpty());
    QVERIFY(userDictionaryWords(dir.filePath(QStringLiteral("none/custom.dic"))).isEmpty());
}


void WriteTests::addingAWordThatCantBeSavedStillCountsAndIsSaidOnce()
{
    QTemporaryDir dir;
    struct Restore {
        ~Restore() { Speller::setUserDictionaryPath(QString()); }
    } restore;
    // (The path is a folder, so nothing can be written there.)
    Speller::setUserDictionaryPath(dir.path());
    QString first, second;
    QVERIFY(!spellingAdd(QStringLiteral("qzxfailone"), &first));
    QVERIFY(!first.isEmpty());
    QVERIFY(!spellingAdd(QStringLiteral("qzxfailtwo"), &second));
    QVERIFY(second.isEmpty());   // (said once)
    QVERIFY(Speller::get().check(QStringLiteral("en-US"), QStringLiteral("qzxfailone")));   // (the word is accepted for the session all the same)
    // When a save works the next failure is news again.
    Speller::setUserDictionaryPath(dir.filePath(QStringLiteral("custom.dic")));
    QVERIFY(spellingAdd(QStringLiteral("qzxsaved")));
    QVERIFY(userDictionaryWords(dir.filePath(QStringLiteral("custom.dic"))).contains(QStringLiteral("qzxsaved")));
    Speller::setUserDictionaryPath(dir.path());
    QString third;
    QVERIFY(!spellingAdd(QStringLiteral("qzxfailthree"), &third));
    QVERIFY(!third.isEmpty());
}


// The suite's own names are right in every language the checker has, with a possessive, whatever the options let go; a typo beside a name is still flagged.
void WriteTests::theNamesOfTheSuiteAreNeverFlaggedInAnyLanguage()
{
    const QStringList names = {QStringLiteral("JeffWrite"), QStringLiteral("JeffPub"),    QStringLiteral("JeffDeck"), QStringLiteral("JeffOffice"),
                               QStringLiteral("JeffCrunch"), QStringLiteral("JeffMail"), QStringLiteral("JeffBase"), QStringLiteral("LLC")};
    QVERIFY(dict::languages().size() >= 12);
    for (const dict::Language &l : dict::languages()) {
        for (const bool upper : {false, true}) {
            Speller::Rules rules;
            rules.ignoreUpper = upper;
            rules.ignoreNumbers = false;
            for (const QString &name : names)
                for (const QString &form : {name, name + QStringLiteral("'s"), name + QChar(0x2019) + QLatin1Char('s'), name.toUpper() + QStringLiteral("'S"), name.toUpper() + QChar(0x2019) + QLatin1Char('S'),
                                            name + QStringLiteral("'S")})   // (the possessive of a name in capitals is in capitals)
                    QVERIFY2(Speller::get().check(l.code, form, rules), qPrintable(l.code + QLatin1Char(' ') + form + (upper ? " (capitals let go)" : "")));
        }
        // In a paragraph, as the checker as you type reads it.
        QTextDocument doc;
        doc.setPlainText(names.join(QLatin1Char(' ')) + QStringLiteral(" JeffWrite's, JeffPub\u2019s."));
        QVERIFY2(misspellingsIn(doc.begin(), l.code).isEmpty(), qPrintable(l.code));
    }
    // Only the names are let through: a word that is not one, and a name misspelled, are still wrong.
    QTextDocument doc;
    doc.setPlainText(QStringLiteral("JeffWritee Jeffoffice qzxqzx"));
    QCOMPARE(int(misspellingsIn(doc.begin(), QStringLiteral("en-US")).size()), 3);
}
