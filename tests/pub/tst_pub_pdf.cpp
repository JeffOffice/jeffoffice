// JeffPub's tests of PDF output, printing, and the print shop's files.
#include "pubhelpers.h"

// A small CMYK printer profile (ICC version 2, 2-point lookup tables), as
// a printer's own .icc file would be, made here so no real one is needed.
static QByteArray testCmykProfile()
{
    QByteArray tags;
    struct Tag { const char *sig; QByteArray data; };
    auto u16 = [](QByteArray &b, int v) { b.append(char((v >> 8) & 0xff)).append(char(v & 0xff)); };
    auto u32 = [](QByteArray &b, quint32 v) { for (int k = 3; k >= 0; --k) b.append(char((v >> (8 * k)) & 0xff)); };
    auto s15 = [&](QByteArray &b, double v) { u32(b, quint32(qint32(std::lround(v * 65536)))); };
    auto lut = [&](int in, int out, const QVector<int> &clut) {
        QByteArray b("mft2\0\0\0\0", 8);
        b.append(char(in)).append(char(out)).append(char(2)).append(char(0));
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c) s15(b, r == c ? 1 : 0);
        u16(b, 2);
        u16(b, 2);
        for (int i = 0; i < in; ++i) { u16(b, 0); u16(b, 65535); }
        for (int v : clut) u16(b, std::clamp(v, 0, 65535));
        for (int i = 0; i < out; ++i) { u16(b, 0); u16(b, 65535); }
        return b;
    };
    QVector<int> a2b, b2a;   // corners: CMYK to Lab, Lab to CMYK
    for (int i = 0; i < 16; ++i) {
        const int c = i >> 3 & 1, m = i >> 2 & 1, y = i >> 1 & 1, k = i & 1;
        const double L = 100 * (1 - k) * (1 - 0.25 * c) * (1 - 0.35 * m) * (1 - 0.1 * y);
        a2b << int(L / 100 * 65280) << int((128 + (60 * m - 40 * c) * (1 - k)) * 256) << int((128 + (70 * y - 50 * c) * (1 - k)) * 256);
    }
    for (int i = 0; i < 8; ++i) {
        const double L = (i >> 2 & 1) * 100.0, a = -128 + (i >> 1 & 1) * 255.0, b = -128 + (i & 1) * 255.0;
        b2a << int(65535 * std::max(0.0, -a / 256)) << int(65535 * std::max(0.0, a / 256)) << int(65535 * std::max(0.0, b / 256)) << int(65535 * (1 - L / 100));
    }
    QByteArray desc("desc\0\0\0\0", 8);
    u32(desc, 10);
    desc.append("Test CMYK", 10);
    desc.append(QByteArray(8 + 3 + 67, '\0'));
    QByteArray cprt("text\0\0\0\0", 8);
    cprt.append("No copyright, use freely", 25);
    QByteArray wtpt("XYZ \0\0\0\0", 8);
    s15(wtpt, 0.9642);
    s15(wtpt, 1.0);
    s15(wtpt, 0.8249);
    const QVector<Tag> list{{"desc", desc}, {"cprt", cprt}, {"wtpt", wtpt}, {"A2B0", lut(4, 3, a2b)}, {"B2A0", lut(3, 4, b2a)}};
    QByteArray table, body;
    const int start = 128 + 4 + 12 * int(list.size());
    for (const Tag &t : list) {
        table.append(t.sig, 4);
        u32(table, quint32(start + body.size()));
        u32(table, quint32(t.data.size()));
        body += t.data;
        while (body.size() % 4) body.append('\0');
    }
    QByteArray head;
    u32(head, quint32(start + body.size()));
    head.append("none", 4);
    head.append("\x02\x10\0\0", 4);
    head.append("prtrCMYKLab ", 12);
    for (int v : {2026, 1, 1, 0, 0, 0}) u16(head, v);
    head.append("acsp", 4);
    head.append(QByteArray(24, '\0'));   // platform, flags, maker, model, attributes
    u32(head, 0);                        // perceptual
    s15(head, 0.9642);
    s15(head, 1.0);
    s15(head, 0.8249);
    head.append("none", 4);
    head.append(QByteArray(44, '\0'));
    QByteArray count;
    u32(count, quint32(list.size()));
    return head + count + table + body;
}

class PubPdfTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // Print preview: rulers and page numbers on a booklet sheet.
    void printPreviewRulersAndNumbers()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        auto doc = jp::Document::blank(QSizeF(396, 612));
        for (int i = 0; i < 3; ++i) doc->pages.push_back(std::make_shared<jp::Page>());
        w.editor()->setDocument(std::move(doc));
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.showBackstage(QStringLiteral("print"));
        QTest::qWait(100);
        QComboBox *layout = nullptr;
        for (QComboBox *c : w.findChildren<QComboBox *>())
            for (int i = 0; i < c->count(); ++i)
                if (c->itemText(i).contains(QStringLiteral("Booklet"), Qt::CaseInsensitive)) { layout = c; c->setCurrentIndex(i); break; }
        QVERIFY(layout);
        QCheckBox *rulers = nullptr, *numbers = nullptr;
        for (QCheckBox *c : w.findChildren<QCheckBox *>()) {
            if (c->text() == QStringLiteral("Show rulers")) rulers = c;
            if (c->text() == QStringLiteral("Show page numbers")) numbers = c;
        }
        QVERIFY(rulers && numbers);
        rulers->setChecked(true);
        numbers->setChecked(true);
        QTest::qWait(100);
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) w.grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/print-preview.png");
    }

    // File > Export > PDF/A writes a file that identifies itself as PDF/A-1b
    // and carries the sRGB output intent the standard requires.
    void pdfaExport()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        w.editor()->setDocument(jp::findTemplate(QStringLiteral("newsletter-classic"))->build(jp::TemplateOptions()));
        const QString path = dir.filePath(QStringLiteral("archive.pdf"));
        w.exportPdf(path, false, true);
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray pdf = f.readAll();
        QVERIFY(pdf.size() > 10000);
        QVERIFY2(pdf.contains("pdfaid:part"), "no PDF/A identification in the XMP metadata");
        QVERIFY2(pdf.contains("GTS_PDFA1"), "no PDF/A output intent");
    }

    // The words of an SVG picture are in a PDF as text with no ink (a pen with no color, which Qt writes as a transparent fill), but not where the
    // PDF allows no transparency, PDF/A-1b and PDF/X-1a: there the pen would be drawn black, over the picture. The picture here has words that
    // are not shown (a text the renderer is told not to draw, as a diagram's are: its class jo-words, its display none) over white, so every page
    // of every kind shows nothing of them: no dark pixel; and where it can, the PDF has them as text.
    void pdfSaysSvgWordsOnlyWhereTheyStayInvisible()
    {
        const QByteArray svg = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"300\" height=\"200\" viewBox=\"0 0 300 200\">"
                               "<rect x=\"0\" y=\"0\" width=\"300\" height=\"200\" fill=\"#ffffff\"/>"
                               "<text class=\"jo-words\" display=\"none\" x=\"10\" y=\"100\" font-family=\"Arimo\" font-size=\"40\" fill=\"#000000\" fill-opacity=\"0\">Whitewords</text></svg>";
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto pic = std::make_shared<jp::PictureItem>();
        pic->imageId = doc->addImage(svg, QStringLiteral("svg"));
        pic->rect = QRectF(100, 100, 300, 200);
        pic->imgRect = QRectF(0, 0, 300, 200);
        doc->pages[0]->items.push_back(pic);
        jp::MainWindow w;
        w.editor()->setDocument(std::move(doc));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        struct Kind {
            const char *name;
            bool archival, x1a;
        };
        for (const Kind &kind : {Kind{"plain", false, false}, Kind{"pdfa", true, false}, Kind{"pdfx1a", false, true}}) {
            jp::MainWindow::PdfSettings s;
            s.archival = kind.archival;
            s.pdfx = kind.x1a;
            const QString path = dir.filePath(QString::fromLatin1(kind.name) + QStringLiteral(".pdf"));
            QVERIFY2(w.exportPdfTo(path, s), kind.name);
            QFile f(path);
            QVERIFY(f.open(QIODevice::ReadOnly));
            const QByteArray bytes = f.readAll();
            const PdfDocument pdf(bytes);
            QVERIFY2(pdf.isValid(), kind.name);
            // (PDFium draws on a clear ground: the page is on white.)
            QImage page(306, 396, QImage::Format_RGB32);
            page.fill(Qt::white);
            const QImage drawn = pdf.render(0, page.size());
            QVERIFY2(!drawn.isNull(), kind.name);
            {
                QPainter on(&page);
                on.drawImage(0, 0, drawn);
            }
            int dark = 0;
            for (int y = 0; y < page.height(); ++y)
                for (int x = 0; x < page.width(); ++x) dark += qGray(page.pixel(x, y)) < 250;
            QVERIFY2(dark == 0, qPrintable(QStringLiteral("%1: %2 dark pixels").arg(QLatin1String(kind.name)).arg(dark)));
            const bool said = pdf.text(0).contains(QStringLiteral("Whitewords"));
            if (kind.archival || kind.x1a) QVERIFY2(!said, kind.name);
            else QVERIFY2(said, qPrintable(pdf.text(0)));
            if (kind.x1a) QVERIFY2(!bytes.contains("/ca ") && !bytes.contains("/CA "), "PDF/X-1a has no transparency");
        }
    }

    // PDF/A has no transparency, so see-through picture pixels are flattened
    // onto what lies beneath them instead of turning white.
    void flattenKeepsSeeThroughPictures()
    {
        auto doc = jp::Document::blank(QSizeF(300, 300));
        doc->pages[0]->background = jp::Fill::solid(jp::ColorRef::rgb(QColor(40, 50, 70)));
        QImage pic(100, 100, QImage::Format_ARGB32);
        pic.fill(Qt::transparent);
        QPainter pp(&pic);
        pp.fillRect(QRect(0, 0, 50, 100), Qt::white);   // left half opaque white, right half clear
        pp.end();
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        pic.save(&buf, "PNG");
        auto item = std::make_shared<jp::PictureItem>();
        item->imageId = doc->addImage(png, "png");
        item->rect = QRectF(100, 100, 100, 100);
        item->imgRect = QRectF(0, 0, 100, 100);
        doc->pages[0]->items.push_back(item);
        QVERIFY(jp::Renderer::usesTransparency(*doc, *item));
        jp::LayoutCache cache;
        jp::PaintContext ctx;
        ctx.doc = doc.get();
        ctx.cache = &cache;
        ctx.opt.output = true;
        ctx.opt.flattenTransparency = true;
        const QImage out = jp::Renderer::renderToImage(ctx, 0, 1.0);
        QCOMPARE(QColor(out.pixel(125, 150)), QColor(Qt::white));          // opaque half
        QCOMPARE(QColor(out.pixel(175, 150)), QColor(40, 50, 70));         // clear half shows the page
    }

    // PDF/X-1a:2001 for a commercial printer: everything in ink (the RGB
    // photo and the flattened see-through shape become CMYK), no
    // transparency, PDF 1.3, trim and bleed boxes, the printing condition,
    // and the version keys a printer's preflight checks.
    void pdfxExport()
    {
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->props.title = QStringLiteral("Cover");
        auto pic = std::make_shared<jp::PictureItem>();
        QImage img(64, 64, QImage::Format_RGB32);
        for (int y = 0; y < 64; ++y)
            for (int x = 0; x < 64; ++x) img.setPixel(x, y, qRgb(x * 4, y * 4, 128));
        QByteArray png;
        QBuffer b(&png);
        b.open(QIODevice::WriteOnly);
        img.save(&b, "PNG");
        pic->imageId = doc->addImage(png, "png");
        pic->rect = QRectF(72, 72, 200, 200);
        pic->imgRect = QRectF(0, 0, 200, 200);
        doc->pages[0]->items.push_back(pic);
        auto see = std::make_shared<jp::ShapeItem>();
        see->shape = QStringLiteral("rect");
        see->rect = QRectF(150, 150, 200, 200);
        see->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(200, 30, 40)));
        see->fill.transparency = 0.5;
        doc->pages[0]->items.push_back(see);
        auto grad = std::make_shared<jp::ShapeItem>();
        grad->shape = QStringLiteral("ellipse");
        grad->rect = QRectF(350, 500, 150, 100);
        grad->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(20, 90, 200)));
        doc->pages[0]->items.push_back(grad);
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 400, 400, 100);
        t->storyId = doc->createStory(QStringLiteral("Ready for press"));
        doc->pages[0]->items.push_back(t);
        w.editor()->setDocument(std::move(doc));
        QTemporaryDir dir;
        for (bool press : {false, true}) {
            const QString path = dir.filePath(press ? QStringLiteral("press.pdf") : QStringLiteral("plain.pdf"));
            jp::MainWindow::PdfSettings s;
            s.preset = press ? jp::MainWindow::PdfSettings::CommercialPress : jp::MainWindow::PdfSettings::HighQuality;
            s.pdfx = true;
            QVERIFY(w.exportPdfTo(path, s));
            if (press && !qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
                QFile::remove(qEnvironmentVariable("JP_SHOT_DIR") + "/pdfx-press.pdf");
                QFile::copy(path, qEnvironmentVariable("JP_SHOT_DIR") + "/pdfx-press.pdf");
            }
            QFile f(path);
            QVERIFY(f.open(QIODevice::ReadOnly));
            const QByteArray pdf = f.readAll();
            QVERIFY(pdf.startsWith("%PDF-1.3"));
            QVERIFY(pdf.contains("/GTS_PDFXVersion (PDF/X-1:2001)") && pdf.contains("/GTS_PDFXConformance (PDF/X-1a:2001)"));
            QVERIFY(pdf.contains("/Trapped /False") && pdf.contains("/OutputIntents [") && pdf.contains("/S /GTS_PDFX"));
            QVERIFY(pdf.contains("/OutputConditionIdentifier (CGATS TR 001)") && pdf.contains("/RegistryName (http://www.color.org)"));
            QVERIFY(!pdf.contains("DeviceRGB"));
            QVERIFY(!pdf.contains("/SMask") && !pdf.contains("/ca ") && !pdf.contains("/CA "));
            // Every picture in ink; the see-through shape was flattened into one.
            jp::QtPdf parsed;
            QVERIFY(parsed.load(path));
            int pictures = 0;
            for (const auto &o : parsed.objects)
                if (jp::QtPdf::dictOf(o.body).contains("/Subtype /Image")) {
                    ++pictures;
                    QVERIFY(jp::QtPdf::dictOf(o.body).contains("/DeviceCMYK") || jp::QtPdf::dictOf(o.body).contains("/DeviceGray"));
                }
            QVERIFY(pictures >= 2);
            // Trim is the page; with marks, the bleed reaches past it.
            const double m = press ? (pdf.contains("/MediaBox [0 0 708") ? 48 : -1) : 0;
            QVERIFY(m >= 0);
            QVERIFY2(pdf.contains(QStringLiteral("/TrimBox [%1 %1 %2 %3]").arg(m, 0, 'f', 3).arg(m + 612, 0, 'f', 3).arg(m + 792, 0, 'f', 3).toLatin1()),
                     pdf.mid(pdf.indexOf("/TrimBox"), 80).constData());
            QVERIFY(pdf.contains("/BleedBox ["));
            if (!press) QVERIFY(pdf.contains("/BleedBox [0.000 0.000 612.000 792.000]"));
        }
        // The check before a file for a printer: the 64-pixel picture over
        // 200 points is 23 ppi; a font this computer lacks.
        QStringList problems = jp::pressProblems(*w.editor()->doc());
        QCOMPARE(problems.size(), 1);
        QVERIFY(problems.first().contains(QLatin1String("23 ppi")));
        QTextCursor c(w.editor()->doc()->storyDoc(t->storyId));
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setFontFamilies({QStringLiteral("No Such Font Anywhere")});
        c.mergeCharFormat(f);
        problems = jp::pressProblems(*w.editor()->doc());
        QCOMPARE(problems.size(), 2);
        QVERIFY(problems.last().contains(QLatin1String("No Such Font Anywhere")));
    }

    // Usage statistics: nothing is counted until the person says yes; then
    // commands count by id, in the form the collector takes, and a ping holds
    // only the install id, version, system, language, launches and counts.
    // Turning them off forgets what was counted and the id.
    // The Print page's Pages box takes a list such as "1-3, 5", as its
    // example says (it printed 1-3 and dropped the rest).
    void printPageList()
    {
        QCOMPARE(jp::parsePageList(QStringLiteral("1-3, 5"), 10), (QVector<int>{0, 1, 2, 4}));
        QCOMPARE(jp::parsePageList(QStringLiteral("5-3;9"), 10), (QVector<int>{2, 3, 4, 8}));
        QCOMPARE(jp::parsePageList(QStringLiteral("2, 2, 8-20"), 10), (QVector<int>{1, 7, 8, 9}));
        QVERIFY(jp::parsePageList(QStringLiteral("0, 11, x, 1-2-3"), 10).isEmpty());
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        ed->insertPages(0, 4, false, false);
        QCOMPARE(ed->doc()->pages.size(), qsizetype(5));
        QTemporaryDir dir;
        auto pagesPrinted = [&](const QJsonObject &opts) {
            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(dir.filePath("p.pdf"));
            printer.setResolution(72);
            printer.setFullPage(true);
            jp::printDocument(ed, &printer, opts);
            QFile f(dir.filePath("p.pdf"));
            if (!f.open(QIODevice::ReadOnly)) return -1;
            const QByteArray pdf = f.readAll();
            return int(pdf.count("/Type /Page\n") + pdf.count("/Type /Page\r") + pdf.count("/Type /Page ") + pdf.count("/Type /Page/"));
        };
        QCOMPARE(pagesPrinted({{"layout", "one"}, {"pages", "1-2, 5"}}), 3);
        QCOMPARE(pagesPrinted({{"layout", "one"}}), 5);
    }

    // Pack and Go for a printer makes the commercial press PDF its card
    // describes (it made a plain one, with no marks).
    void packForPrinterHasMarks()
    {
        jp::MainWindow w;
        QTemporaryDir dir;
        jp::packForPrinter(&w, dir.path());
        QFile f(QDir(dir.path()).filePath(w.editor()->displayName() + QStringLiteral(".pdf")));
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray pdf = f.readAll();
        static const QRegularExpression media(QStringLiteral("/MediaBox \\[\\s*0 0 ([0-9.]+) ([0-9.]+)"));
        const auto m = media.match(QString::fromLatin1(pdf));
        QVERIFY(m.hasMatch());
        QVERIFY2(m.captured(1).toDouble() > w.editor()->doc()->pageSize().width() + 36, qPrintable(m.captured(0)));   // room for the marks
        QVERIFY(QFile::exists(QDir(dir.path()).filePath(w.editor()->displayName() + QStringLiteral(".jpub"))));
    }

    // Pack and Go for a printer says so when the publication can't be saved,
    // instead of leaving a folder with the PDF alone (and a happy message).
    void packForPrinterReportsAFailedSave()
    {
        jp::MainWindow w;
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        // A folder where the publication's file must go: it can't be written there.
        QVERIFY(QDir(dir.path()).mkdir(w.editor()->displayName() + QStringLiteral(".jpub")));
        QString err;
        QVERIFY(!jp::packForPrinter(&w, dir.path(), &err));
        QVERIFY2(err.contains(QStringLiteral("publication")), qPrintable(err));
        // With a place to write all three, it says nothing.
        QTemporaryDir fine;
        QVERIFY(fine.isValid());
        err.clear();
        QVERIFY(jp::packForPrinter(&w, fine.path(), &err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        QVERIFY(QFile::exists(QDir(fine.path()).filePath(w.editor()->displayName() + QStringLiteral(".jpub"))));
    }

    // The Design Checker's low-resolution check is a final publishing check:
    // it runs with only those checks on (it needed the general ones too).
    void designCheckerFinalChecksAlone()
    {
        jp::MainWindow w;
        w.show();
        QImage tiny(20, 20, QImage::Format_RGB32);
        tiny.fill(Qt::red);
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        tiny.save(&buf, "PNG");
        auto pic = std::make_shared<jp::PictureItem>();
        pic->imageId = w.editor()->doc()->addImage(png, "png");
        pic->rect = QRectF(72, 72, 288, 288);
        pic->imgRect = QRectF(0, 0, 288, 288);   // 20 pixels across 4 inches: 5 ppi
        w.editor()->addItem(pic);
        w.showTaskPane(QStringLiteral("designchecker"));
        QCheckBox *general = nullptr;
        for (auto *c : w.findChildren<QCheckBox *>())
            if (c->text() == QLatin1String("Run general design checks")) general = c;
        QVERIFY(general);
        general->setChecked(false);
        QStringList found;
        for (auto *l : w.findChildren<QListWidget *>())
            for (int i = 0; i < l->count(); ++i) found << l->item(i)->text();
        QVERIFY2(found.contains(QStringLiteral("Picture has low resolution (5 ppi) (Page 1)")), qPrintable(found.join(QStringLiteral(" / "))));
    }

    // A booklet is saved as Publisher saves one: each master in a right-hand
    // and a left-hand part, odd pages on the right part and even ones on the
    // left, pages after the first marked 0. Publisher read JeffPub's copy of
    // its booklet as its own (layout, spreads, masters) and pictured it the
    // same (Oct 7). On opening, the parts make one two-page master, and every
    // page comes back (pages marked 0 counted as special: one page did).
    // PDF/X-4 with the printer's own color profile: transparency kept,
    // pictures in CMYK with their see-through parts, the profile in the
    // output intent, the PDF/X-4 identification in the metadata.
    // Black overprints in a file for a printer, as the publication's
    // overprinting settings say (Publisher's defaults: text below 24 pt and
    // lines; not fills): the black ink prints over the colors under it,
    // which then show no white edge where the plates don't line up.
    void pdfXOverprint()
    {
        auto doc = Document::blank(QSizeF(400, 300));
        auto addText = [&](const QString &s, double pt, const QColor &c, const QRectF &r) {
            auto t = std::make_shared<TextItem>();
            t->rect = r;
            t->storyId = doc->createStory(s);
            QTextCursor cur(doc->storyDoc(t->storyId));
            cur.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontPointSize(pt);
            f.setForeground(c);
            cur.mergeCharFormat(f);
            doc->pages[0]->items.push_back(t);
        };
        auto red = std::make_shared<ShapeItem>();
        red->shape = QStringLiteral("rect");
        red->rect = QRectF(20, 20, 360, 260);
        red->fill = Fill::solid(ColorRef::rgb(QColor(220, 30, 40)));
        red->stroke.color = ColorRef::none();
        doc->pages[0]->items.push_back(red);
        addText(QStringLiteral("small"), 10, Qt::black, QRectF(40, 40, 150, 30));
        addText(QStringLiteral("BIG"), 36, Qt::black, QRectF(40, 90, 200, 60));
        auto line = std::make_shared<LineItem>();
        line->p1 = QPointF(40, 180);
        line->p2 = QPointF(300, 180);
        line->stroke.color = ColorRef::rgb(Qt::black);
        line->stroke.width = 2;
        doc->pages[0]->items.push_back(line);
        auto blackBox = std::make_shared<ShapeItem>();
        blackBox->shape = QStringLiteral("rect");
        blackBox->rect = QRectF(40, 200, 60, 40);
        blackBox->fill = Fill::solid(ColorRef::rgb(Qt::black));
        blackBox->stroke.color = ColorRef::none();
        doc->pages[0]->items.push_back(blackBox);

        // Saved with the publication.
        doc->print.overprint.textBelow = 30;
        {
            Document again;
            again.fromJson(doc->toJson());
            QVERIFY(again.print.overprint == doc->print.overprint);
        }
        doc->print.overprint = OverprintSettings();

        MainWindow w;
        w.editor()->setDocument(std::move(doc));
        Document *d = w.editor()->doc();
        QTemporaryDir dir;
        // The page's drawing, and the PDF still PDF/X that PDFium opens.
        auto drawing = [&](const QString &name) {
            const QString path = dir.filePath(name);
            MainWindow::PdfSettings ps;
            ps.pdfx = true;
            if (!w.exportPdfTo(path, ps)) return QByteArray();
            if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
                QFile::remove(qEnvironmentVariable("JP_SHOT_DIR") + "/overprint-" + name);
                QFile::copy(path, qEnvironmentVariable("JP_SHOT_DIR") + "/overprint-" + name);
            }
            QFile f(path);
            if (!f.open(QIODevice::ReadOnly)) return QByteArray();
            const QByteArray bytes = f.readAll();
            if (!bytes.startsWith("%PDF-1.3") || !PdfDocument(bytes).isValid()) return QByteArray();
            QtPdf parsed;
            parsed.load(path);
            QByteArray content;
            for (const auto &o : parsed.objects) {
                const QByteArray dict = QtPdf::dictOf(o.body);
                bool ok = false;
                if (QtPdf::isStream(o.body) && dict.contains("/FlateDecode") && !dict.contains("/Length1") && !dict.contains("/Subtype"))
                    content += parsed.streamData(o, &ok);
                if (dict.contains("/Type /ExtGState") && dict.contains("/OP true")) content += "\n%switch " + dict + "\n";
            }
            return content;
        };
        auto between = [](const QByteArray &c, const QByteArray &from, const QByteArray &to) {
            const qsizetype a = c.indexOf(from);
            return a < 0 ? QByteArray() : c.mid(a, c.indexOf(to, a) - a);
        };
        const QByteArray smallText = "/F11 80 Tf", bigText = "/F11 288 Tf", lineStart = "40 180 m", boxStart = "666.666666 3333.33333 m";

        // Publisher's defaults: the 10-point black text and the black line
        // overprint; the 36-point text and the black box knock out.
        QByteArray c = drawing(QStringLiteral("defaults.pdf"));
        QVERIFY(!c.isEmpty());
        QVERIFY2(between(c, smallText, "ET").contains("/JPop2 gs"), c.constData());
        QVERIFY(!between(c, bigText, "ET").contains("/JPop"));
        QVERIFY(c.contains("/JPop1 gs\n" + lineStart));
        QVERIFY(c.contains(boxStart) && !c.mid(c.indexOf(boxStart) - 12, 12).contains("/JPop"));
        QVERIFY(c.contains("/OPM 1"));
        // The colored background never overprints.
        QVERIFY(!c.contains("/JPop3"));

        // Fills too; then a 90% black box counts only from a threshold below it.
        d->print.overprint.fills = true;
        c = drawing(QStringLiteral("fills.pdf"));
        QVERIFY(c.contains("/JPop2 gs\n" + boxStart));
        blackBox->fill = Fill::solid(ColorRef::rgb(QColor(25, 25, 25)));
        c = drawing(QStringLiteral("gray.pdf"));
        QVERIFY(c.contains(boxStart) && !c.mid(c.indexOf(boxStart) - 12, 12).contains("/JPop"));
        d->print.overprint.threshold = 85;
        c = drawing(QStringLiteral("gray85.pdf"));
        QVERIFY(c.contains("/JPop2 gs\n" + boxStart));
        // Text up to 40 points: the big text too.
        d->print.overprint.textBelow = 40;
        c = drawing(QStringLiteral("big.pdf"));
        QVERIFY(between(c, bigText, "ET").contains("/JPop2 gs"));
        // All off: nothing overprints.
        d->print.overprint.text = d->print.overprint.lines = d->print.overprint.fills = false;
        c = drawing(QStringLiteral("off.pdf"));
        QVERIFY(!c.isEmpty() && !c.contains("/JPop"));
    }

    void pdfX4WithOwnProfile()
    {
        const QByteArray icc = testCmykProfile();
        const QColorSpace cs = QColorSpace::fromIccProfile(icc);
        QVERIFY(cs.isValid());
        QCOMPARE(cs.colorModel(), QColorSpace::ColorModel::Cmyk);
        QTemporaryDir dir;
        const QString iccPath = dir.filePath(QStringLiteral("printer.icc"));
        {
            QFile f(iccPath);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(icc);
        }
        auto doc = Document::blank(QSizeF(400, 300));
        auto shape = std::make_shared<ShapeItem>();
        shape->shape = QStringLiteral("rect");
        shape->rect = QRectF(50, 50, 200, 100);
        shape->fill = Fill::solid(ColorRef::rgb(QColor(0, 0, 255)));
        shape->fill.transparency = 0.5;
        doc->pages[0]->items.push_back(shape);
        QImage img(8, 8, QImage::Format_ARGB32);
        img.fill(QColor(255, 0, 0, 120));
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "PNG");
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = doc->addImage(png, QStringLiteral("png"));
        pic->rect = QRectF(200, 120, 100, 100);
        pic->imgRect = QRectF(0, 0, 100, 100);
        doc->pages[0]->items.push_back(pic);

        MainWindow w;
        w.editor()->setDocument(std::move(doc));
        MainWindow::PdfSettings ps;
        ps.pdfx = true;
        for (int i = 0; i < pdfXConditions().size(); ++i)
            if (pdfXConditions()[i].ownProfile) ps.pdfxCondition = i;
        QVERIFY(pdfXConditions().value(ps.pdfxCondition).x4);
        // Without the profile there's no PDF/X-4 (and no file).
        const QString none = dir.filePath(QStringLiteral("none.pdf"));
        QVERIFY(!w.exportPdfTo(none, ps));
        QVERIFY(!QFile::exists(none));
        ps.pdfxProfile = iccPath;
        const QString path = dir.filePath(QStringLiteral("x4.pdf"));
        QVERIFY(w.exportPdfTo(path, ps));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        QVERIFY(bytes.startsWith("%PDF-1.6"));
        QVERIFY(bytes.contains("pdfxid:GTS_PDFXVersion=\"PDF/X-4\""));

        QtPdf parsed;
        QVERIFY(parsed.load(path));
        bool intent = false, profile = false, seeThrough = false, cmykPicture = false, mask = false, trim = false;
        for (const auto &o : parsed.objects) {
            const QByteArray dict = QtPdf::dictOf(o.body);
            QVERIFY2(!dict.contains("/DeviceRGB"), dict.constData());
            if (dict.contains("/Type /OutputIntent")) intent = dict.contains("/S/GTS_PDFX") && dict.contains("/DestOutputProfile");
            if (dict.contains("/N 4") && QtPdf::isStream(o.body)) {
                bool ok = false;
                profile = parsed.streamData(o, &ok) == icc;
            }
            if (QRegularExpression(QStringLiteral("/ca\\s+0\\.[0-9]")).match(QString::fromLatin1(dict)).hasMatch()) seeThrough = true;
            if (dict.contains("/Subtype /Image") && dict.contains("/DeviceCMYK")) {
                cmykPicture = true;
                mask = mask || dict.contains("/SMask");
            }
            trim = trim || (dict.contains("/TrimBox") && dict.contains("/BleedBox"));
        }
        QVERIFY(intent);
        QVERIFY(profile);
        QVERIFY(seeThrough);
        QVERIFY(cmykPicture);
        QVERIFY(mask);
        QVERIFY(trim);
        // It opens, and the blue is half see-through (PDFium draws the page
        // with nothing behind it).
        const PdfDocument out(bytes);
        QVERIFY(out.isValid());
        const QImage page = out.render(0, QSize(400, 300));
        const QRgb mid = page.pixel(100, 100);
        QVERIFY2(qAlpha(mid) > 100 && qAlpha(mid) < 160 && qBlue(mid) > qRed(mid) + 40, qPrintable(QString::number(mid, 16)));
    }

    // ECI's profile, downloaded: used only when the download and the
    // profile in it are the files expected.
    void pdfXProfileDownloadChecked()
    {
        PdfXCondition c;
        c.profileFile = QStringLiteral("Profile.icc");
        const QByteArray icc = testCmykProfile();
        ZipWriter zw;
        zw.add(QStringLiteral("Profile.icc"), icc, true);
        zw.add(QStringLiteral("Profile_info.pdf"), QByteArray("%PDF-1.4"));
        const QByteArray zip = zw.finish();
        c.zipSha256 = QCryptographicHash::hash(zip, QCryptographicHash::Sha256).toHex();
        c.profileSha256 = QCryptographicHash::hash(icc, QCryptographicHash::Sha256).toHex();
        QString err;
        QCOMPARE(pdfXProfileFromZip(c, zip, &err), icc);
        // A page of HTML (a moved file) or a changed zip: no profile.
        QVERIFY(pdfXProfileFromZip(c, QByteArray("<html>moved</html>"), &err).isEmpty());
        QVERIFY(err.contains(QLatin1String("isn't the file expected")));
        PdfXCondition other = c;
        other.profileSha256 = QByteArray(64, '0');
        QVERIFY(pdfXProfileFromZip(other, zip, &err).isEmpty());
        QVERIFY(err.contains(QLatin1String("profile")));
        // ECI's own: a FOGRA51 condition, checked by hash.
        bool eci = false;
        for (const PdfXCondition &x : pdfXConditions())
            if (x.identifier == QLatin1String("FOGRA51")) eci = x.x4 && x.profileUrl.startsWith(QLatin1String("https://")) && x.zipSha256.size() == 64 && x.profileSha256.size() == 64;
        QVERIFY(eci);
        // With ECI's download at hand (JP_ECI_ZIP), the real thing.
        if (qEnvironmentVariableIsSet("JP_ECI_ZIP")) {
            QFile f(qEnvironmentVariable("JP_ECI_ZIP"));
            QVERIFY(f.open(QIODevice::ReadOnly));
            for (const PdfXCondition &x : pdfXConditions())
                if (x.identifier == QLatin1String("FOGRA51")) {
                    const QByteArray real = pdfXProfileFromZip(x, f.readAll(), &err);
                    QVERIFY2(!real.isEmpty(), qPrintable(err));
                    QCOMPARE(QColorSpace::fromIccProfile(real).colorModel(), QColorSpace::ColorModel::Cmyk);
                }
        }
    }

    // A PDF page placed as a picture: shown from PDFium's raster, exported
    // to PDF as vectors, kept as a PDF in .jpub and as a picture in .pub.
    void pdfPagePlacedAsPicture()
    {
        using namespace jp;
        QByteArray pdf;
        {
            QBuffer buf(&pdf);
            buf.open(QIODevice::WriteOnly);
            QPdfWriter w(&buf);
            w.setPageSize(QPageSize(QSizeF(300, 200), QPageSize::Point));
            w.setPageMargins(QMarginsF(0, 0, 0, 0));
            w.setResolution(72);
            QPainter p(&w);
            p.fillRect(QRectF(0, 0, 300, 200), Qt::blue);
            w.newPage();
            p.fillRect(QRectF(20, 20, 260, 160), QColor(220, 0, 0));
            p.end();
        }
        const QByteArray page2 = PdfDocument(pdf).extractPage(1);
        QVERIFY(!page2.isEmpty());

        auto doc = Document::blank(QSizeF(612, 792));
        const QString id = doc->addImage(page2, QStringLiteral("pdf"));
        const QImage shown = doc->image(id);
        QVERIFY(!shown.isNull());
        QCOMPARE(QColor(shown.pixel(shown.width() / 2, shown.height() / 2)), QColor(220, 0, 0));
        QVERIFY(qAlpha(shown.pixel(2, 2)) == 0);   // no page behind it
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = id;
        pic->rect = QRectF(100, 100, 300, 200);
        pic->imgRect = QRectF(0, 0, 300, 200);
        doc->pages[0]->items.push_back(pic);

        QTemporaryDir dir;
        QString err;
        // .jpub keeps the PDF.
        const QString jpub = dir.filePath(QStringLiteral("pdf.jpub"));
        QVERIFY2(savePublication(*doc, jpub, QImage(), &err), qPrintable(err));
        auto back = loadPublication(jpub, &err);
        QVERIFY(back);
        QCOMPARE(back->images.value(id).format, QStringLiteral("pdf"));
        QCOMPARE(back->images.value(id).bytes, page2);
        // .pub has no PDF pictures: a PNG of it.
        const QString pub = dir.filePath(QStringLiteral("pdf.pub"));
        QVERIFY2(exportPublisher(*doc, pub, &err), qPrintable(err));
        auto fromPub = importPublisherFile(pub, &err);
        QVERIFY(fromPub);
        QCOMPARE(fromPub->images.size(), 1);
        QCOMPARE(fromPub->images.first().format, QStringLiteral("png"));

        // Exported to PDF it's a filled area, not a picture.
        MainWindow w;
        w.editor()->setDocument(std::move(doc));
        const QString path = dir.filePath(QStringLiteral("placed.pdf"));
        MainWindow::PdfSettings ps;
        ps.preset = MainWindow::PdfSettings::HighQuality;
        QVERIFY(w.exportPdfTo(path, ps));
        QtPdf parsed;
        QVERIFY(parsed.load(path));
        for (const auto &o : parsed.objects) QVERIFY(!QtPdf::dictOf(o.body).contains("/Subtype /Image"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const PdfDocument out(f.readAll());
        QVERIFY(out.isValid());
        const QImage page = out.render(0, QSize(612, 792));
        QCOMPARE(QColor(page.pixel(250, 200)), QColor(220, 0, 0));
    }

    // A PDF page drawn as vectors looks like PDFium's own raster of it: text
    // (as glyph outlines), filled and dashed shapes, and a picture.
    void pdfPagePlaysLikeItsRaster()
    {
        QByteArray pdf;
        {
            QBuffer buf(&pdf);
            buf.open(QIODevice::WriteOnly);
            QPdfWriter w(&buf);
            w.setPageSize(QPageSize(QSizeF(200, 100), QPageSize::Point));
            w.setPageMargins(QMarginsF(0, 0, 0, 0));
            w.setResolution(72);
            QPainter p(&w);
            p.fillRect(QRectF(10, 10, 60, 40), QColor(200, 30, 30));
            // (Square caps would give the line's last, empty dash a square
            // in PDF readers but not in Qt; the check catches that, below.)
            p.setPen(QPen(Qt::blue, 3, Qt::DashLine, Qt::FlatCap));
            p.drawLine(QPointF(10, 70), QPointF(190, 70));
            QFont f(QStringLiteral("DejaVu Sans"));
            f.setPixelSize(24);
            p.setFont(f);
            p.setPen(Qt::black);
            p.drawText(QPointF(80, 40), QStringLiteral("Hgx"));
            p.save();
            p.translate(30, 95);
            p.rotate(-20);
            p.drawText(QPointF(0, 0), QStringLiteral("ab"));
            p.restore();
            QImage img(8, 8, QImage::Format_RGB32);
            img.fill(Qt::green);
            p.drawImage(QRectF(150, 10, 30, 30), img);
            w.newPage();
            p.fillRect(QRectF(0, 0, 50, 50), Qt::yellow);
            p.end();
        }
        jp::PdfDocument doc(pdf);
        QVERIFY(doc.isValid());
        QCOMPARE(doc.pageCount(), 2);
        QCOMPARE(doc.pageSize(0).toSize(), QSize(200, 100));
        const QSize px(800, 400);
        const QImage ref = doc.render(0, px).convertToFormat(QImage::Format_ARGB32);
        QVERIFY(!ref.isNull());
        QImage played(px, QImage::Format_ARGB32);
        played.fill(Qt::transparent);
        {
            QPainter p(&played);
            doc.play(&p, 0, QRectF(QPointF(0, 0), QSizeF(px)));
        }
        // Over white, pixels differing by more than an edge's antialiasing.
        auto over = [](QRgb c) { const int a = qAlpha(c); return QColor(255 - a + qRed(c) * a / 255, 255 - a + qGreen(c) * a / 255, 255 - a + qBlue(c) * a / 255); };
        int differ = 0, inked = 0;
        for (int y = 0; y < px.height(); ++y)
            for (int x = 0; x < px.width(); ++x) {
                const QColor a = over(ref.pixel(x, y)), b = over(played.pixel(x, y));
                if (a != Qt::white) ++inked;
                if (std::abs(a.red() - b.red()) + std::abs(a.green() - b.green()) + std::abs(a.blue() - b.blue()) > 120) ++differ;
            }
        if (qEnvironmentVariableIsSet("JP_PDF_DEBUG")) {
            ref.save(qEnvironmentVariable("JP_PDF_DEBUG") + QStringLiteral("/ref.png"));
            played.save(qEnvironmentVariable("JP_PDF_DEBUG") + QStringLiteral("/played.png"));
            QFile out(qEnvironmentVariable("JP_PDF_DEBUG") + QStringLiteral("/t.pdf"));
            if (out.open(QIODevice::WriteOnly)) out.write(pdf);
        }
        QVERIFY(inked > 20000);
        QVERIFY2(differ < inked / 50, qPrintable(QStringLiteral("%1 of %2 inked pixels differ").arg(differ).arg(inked)));
        // The check finds nothing for the raster to draw.
        QCOMPARE(doc.rasterShare(0), 0.0);

        // In SVG the text is paths and only the picture is an image.
        QByteArray svg;
        {
            QBuffer buf(&svg);
            QSvgGenerator gen;
            gen.setOutputDevice(&buf);
            gen.setSize(QSize(200, 100));
            QPainter p(&gen);
            doc.play(&p, 0, QRectF(0, 0, 200, 100));
        }
        QCOMPARE(svg.count("<image"), 1);
        QVERIFY(svg.count("<path") >= 4);

        // One page taken out is a PDF of its own.
        jp::PdfDocument second(doc.extractPage(1));
        QVERIFY(second.isValid());
        QCOMPARE(second.pageCount(), 1);
        QCOMPARE(second.pageSize(0).toSize(), QSize(200, 100));
        QVERIFY(jp::PdfDocument::looksLikePdf(doc.extractPage(0)));
        QVERIFY(!jp::PdfDocument(QByteArray("not a pdf")).isValid());
        QVERIFY(doc.extractPage(5).isEmpty());
        // Cut short or scrambled, it's drawn as far as it goes, or not at all.
        for (const QByteArray &bad : {pdf.left(pdf.size() / 2), QByteArray(pdf).replace("obj", "jbo"), QByteArray("%PDF-1.4\n%%EOF")}) {
            jp::PdfDocument d(bad);
            QImage img(200, 100, QImage::Format_ARGB32_Premultiplied);
            QPainter p(&img);
            for (int i = -1; i <= d.pageCount(); ++i) {
                d.render(i, QSize(200, 100));
                d.play(&p, i, QRectF(0, 0, 200, 100));
                d.extractPage(i);
                QVERIFY(d.rasterShare(i) >= 0);
            }
        }
    }

    // Spot colors: saved with their names, offered in the color drop-downs,
    // and printed on plates of their own (tints as lighter ink), knocked out
    // of the process plates.
    void spotColorPlates()
    {
        const QColor spot(0, 90, 170);
        QCOMPARE(jp::spotAmount(spot, spot), 1.0);
        QVERIFY(std::abs(jp::spotAmount(jp::mix(spot, Qt::white, 0.6), spot) - 0.4) < 0.02);
        QCOMPARE(jp::spotAmount(QColor(200, 30, 30), spot), -1.0);
        QCOMPARE(jp::spotAmount(Qt::white, spot), -1.0);

        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->print.model = jp::PrintInfo::ProcessPlusSpot;
        doc->print.spotColors = {spot};
        doc->print.spotNames = {QStringLiteral("Harbor Blue")};
        auto box = [&](const QRectF &r, const QColor &c) {
            auto s = std::make_shared<jp::ShapeItem>();
            s->rect = r;
            s->fill = jp::Fill::solid(jp::ColorRef::rgb(c));
            s->stroke = jp::Stroke::none();
            doc->pages[0]->items.push_back(s);
        };
        box(QRectF(72, 72, 100, 100), spot);                                // the spot color
        box(QRectF(200, 72, 100, 100), jp::mix(spot, Qt::white, 0.5));     // a 50% tint
        box(QRectF(330, 72, 100, 100), QColor(200, 30, 30));                // red, process
        QString err;
        auto again = jp::publicationFromBytes(jp::publicationBytes(*doc, QImage()), &err);
        QCOMPARE(again->print.spotName(0), QStringLiteral("Harbor Blue"));
        QCOMPARE(again->print.spotColors.first(), spot);
        QCOMPARE(jp::plateName(4, doc.get()), QStringLiteral("Harbor Blue"));

        auto ink = [](const QImage &plate, const QPointF &pt) { return 1 - qGray(plate.pixel(int(pt.x() * 72 / 72), int(pt.y()))) / 255.0; };
        const QImage spotPlate = jp::renderPlate(*doc, 0, 4, 72);
        QVERIFY(ink(spotPlate, QPointF(122, 122)) > 0.95);
        QVERIFY(std::abs(ink(spotPlate, QPointF(250, 122)) - 0.5) < 0.06);
        QVERIFY(ink(spotPlate, QPointF(380, 122)) < 0.02);
        const QImage cyan = jp::renderPlate(*doc, 0, 0, 72), magenta = jp::renderPlate(*doc, 0, 1, 72);
        QVERIFY(ink(cyan, QPointF(122, 122)) < 0.02);       // knocked out
        QVERIFY(ink(cyan, QPointF(250, 122)) < 0.02);
        QVERIFY(ink(magenta, QPointF(380, 122)) > 0.7);     // red stays on the process plates
        // Text colors go through the plates too.
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 300, 400, 80);
        t->storyId = doc->createStory(QStringLiteral("HHHHHHHH"));
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontPointSize(48);
            f.setProperty(jp::tp::ColorRefP, jp::ColorRef::rgb(spot).toString());
            c.mergeCharFormat(f);
        }
        doc->pages[0]->items.push_back(t);
        auto inkIn = [](const QImage &plate, const QRect &r) {
            double sum = 0;
            for (int y = r.top(); y <= r.bottom(); ++y)
                for (int x = r.left(); x <= r.right(); ++x) sum += 1 - qGray(plate.pixel(x, y)) / 255.0;
            return sum;
        };
        QVERIFY(inkIn(jp::renderPlate(*doc, 0, 4, 72), QRect(72, 300, 400, 80)) > 300);
        QVERIFY(inkIn(jp::renderPlate(*doc, 0, 0, 72), QRect(72, 300, 400, 80)) < 5);

        // The Commercial Print tab lists the spot colors.
        jp::MainWindow w;
        w.editor()->setDocument(std::move(doc));
        // A PDF names the ink: Separation colors at the spot's strength.
        {
            QTemporaryDir pd;
            const QString pdfPath = pd.filePath(QStringLiteral("spot.pdf"));
            QVERIFY(w.exportPdfTo(pdfPath, jp::MainWindow::PdfSettings()));
            if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) { QFile::remove(qEnvironmentVariable("JP_SHOT_DIR") + "/spot.pdf"); QFile::copy(pdfPath, qEnvironmentVariable("JP_SHOT_DIR") + "/spot.pdf"); }
            QFile pf(pdfPath);
            QVERIFY(pf.open(QIODevice::ReadOnly));
            const QByteArray bytes = pf.readAll();
            QVERIFY(bytes.contains("/Separation /Harbor#20Blue /DeviceCMYK"));
            QVERIFY(bytes.contains("startxref"));
            QByteArray text;
            for (qsizetype at = 0; (at = bytes.indexOf(">>\nstream\n", at)) >= 0; at += 10) {
                const qsizetype st = at + 10, en = bytes.indexOf("\nendstream", st);
                bool ok = false;
                text += jp::QtPdf::inflate(bytes.mid(st, en - st), &ok);   // pictures aren't zlib: nothing
            }
            QVERIFY2(text.contains("/CSspot0 cs 1 scn"), text.left(300).constData());
            QVERIFY(text.contains("/CSspot0 cs 0.5 scn") || text.contains("/CSspot0 cs 0.4"));
            QVERIFY(text.contains("/CSpcmyk cs"));   // the red stays process
        }
        QTimer::singleShot(0, [] {
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QVERIFY(dlg);
            auto *list = dlg->findChild<QListWidget *>();
            QVERIFY(list);
            QCOMPARE(list->count(), 1);
            QCOMPARE(list->item(0)->text(), QStringLiteral("Harbor Blue"));
            if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) dlg->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/commercial-print.png");
            dlg->reject();
        });
        jp::documentPropertiesDialog(&w, w.editor(), 1);
    }

    // A saved PDF opens in the system's viewer when the option is on (the
    // default), not when it's off, and never for command-line exports.
    void pdfOpensAfterSaving()
    {
        QStringList opened;
        const auto hook = jp::MainWindow::openFileHook;
        jp::MainWindow::openFileHook = [&](const QString &path) { opened << path; return true; };
        const bool was = jp::MainWindow::openPdfAfterSaving();
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        QTemporaryDir dir;
        const QString a = dir.filePath(QStringLiteral("a.pdf")), b = dir.filePath(QStringLiteral("b.pdf")), c = dir.filePath(QStringLiteral("c.pdf"));
        jp::MainWindow::setOpenPdfAfterSaving(true);
        QVERIFY(w.exportPdfTo(a, jp::MainWindow::PdfSettings()));   // a window not shown, as from the command line
        w.show();
        QVERIFY(w.exportPdfTo(b, jp::MainWindow::PdfSettings()));
        jp::MainWindow::setOpenPdfAfterSaving(false);
        QVERIFY(w.exportPdfTo(c, jp::MainWindow::PdfSettings()));
        QCOMPARE(opened, QStringList{b});
        jp::MainWindow::setOpenPdfAfterSaving(was);
        jp::MainWindow::openFileHook = hook;
    }

    // Process inks survive reading on a system whose language writes
    // decimals with a comma (German, French...): nothing on the way may go
    // through locale-dependent text.
    void processInksAnyLocale()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto box = std::make_shared<jp::ShapeItem>();
        box->rect = QRectF(72, 72, 200, 100);
        box->fill = jp::Fill::solid(jp::ColorRef::inks(QColor::fromCmykF(250 / 255.f, 194 / 255.f, 34 / 255.f, 77 / 255.f), QColor(51, 61, 97)));
        box->stroke = jp::Stroke::none();
        doc->pages[0]->items.push_back(box);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("inks.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        const std::string before = setlocale(LC_NUMERIC, nullptr);
        const char *comma = nullptr;
        for (const char *l : {"de_DE.UTF-8", "de_DE.utf8", "de_AT.utf8", "fr_FR.UTF-8", "fr_FR.utf8"})
            if (setlocale(LC_NUMERIC, l)) { comma = l; break; }
        if (!comma) QSKIP("no comma-decimal locale on this system");
        char probe[16];
        snprintf(probe, sizeof probe, "%.1f", 0.5);
        auto back = jp::importPublisherFile(path, &err);
        setlocale(LC_NUMERIC, before.c_str());
        QCOMPARE(QString::fromLatin1(probe), QStringLiteral("0,5"));   // the locale really writes commas
        QVERIFY2(back, qPrintable(err));
        int found = 0;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() != jp::ItemType::Shape) return;
            const QColor k = static_cast<const jp::ShapeItem *>(it.get())->fill.color.rgbValue();
            QCOMPARE(k.spec(), QColor::Cmyk);
            QVERIFY2(std::abs(k.cyanF() - 250 / 255.0) < 0.003 && std::abs(k.blackF() - 77 / 255.0) < 0.003, qPrintable(jp::colorToString(k)));
            ++found;
        });
        QCOMPARE(found, 1);
    }

    // A process color from a .pub file: the screen shows the color the file
    // shows, and a CMYK PDF and the plates get its exact inks.
    void processInksShownColor()
    {
        const jp::ColorRef ref = jp::ColorRef::inks(QColor::fromCmykF(0.6f, 0.4f, 0.4f, 1.0f), QColor(29, 31, 25));
        QCOMPARE(ref.toString(), QStringLiteral("cmyk(60,40,40,100)=#1D1F19"));
        QCOMPARE(jp::ColorRef::fromString(ref.toString()), ref);
        QVERIFY(ref != jp::ColorRef::rgb(QColor::fromCmykF(0.6f, 0.4f, 0.4f, 1.0f)));
        jp::ColorScheme scheme;
        QCOMPARE(ref.resolve(scheme).toRgb(), QColor(29, 31, 25));
        {
            jp::InkOutput inks;
            const QColor c = ref.resolve(scheme);
            QCOMPARE(c.spec(), QColor::Cmyk);
            QVERIFY(std::abs(c.cyanF() - 0.6) < 0.002 && std::abs(c.blackF() - 1.0) < 0.002);
        }
        QCOMPARE(ref.resolve(scheme).toRgb(), QColor(29, 31, 25));
        QColor seen;
        jp::setColorFilter([&](const QColor &c) { seen = c; return c; });
        ref.resolve(scheme);
        jp::setColorFilter({});
        QCOMPARE(seen.spec(), QColor::Cmyk);   // plates see the inks

        // A process-color publication's PDF carries the inks.
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->print.model = jp::PrintInfo::ProcessCMYK;
        auto box = std::make_shared<jp::ShapeItem>();
        box->rect = QRectF(72, 72, 200, 100);
        box->fill = jp::Fill::solid(ref);
        box->stroke = jp::Stroke::none();
        doc->pages[0]->items.push_back(box);
        w.editor()->setDocument(std::move(doc));
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("inks.pdf"));
        QVERIFY(w.exportPdfTo(path, jp::MainWindow::PdfSettings()));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray pdf = f.readAll();
        QByteArray text;
        for (qsizetype at = 0; (at = pdf.indexOf("stream", at)) >= 0; at += 6) {
            qsizetype start = at + 6;
            if (pdf.mid(start, 1) == "\n") ++start;
            const qsizetype end = pdf.indexOf("endstream", start);
            if (end < 0) break;
            bool ok = false;
            text += jp::QtPdf::inflate(pdf.mid(start, end - start), &ok);
        }
        bool found = false;
        for (const QByteArray &l : text.split('\n')) {
            const QList<QByteArray> v = l.split(' ');
            if (v.size() == 5 && v[4] == "scn")
                found = found || (std::abs(v[0].toDouble() - 0.6) < 1e-3 && std::abs(v[1].toDouble() - 0.4) < 1e-3 && std::abs(v[2].toDouble() - 0.4) < 1e-3
                                  && std::abs(v[3].toDouble() - 1.0) < 1e-3);
        }
        QVERIFY(found);
        QVERIFY(!jp::keepInks());
    }

    void cmykColors()
    {
        const QColor ink = QColor::fromCmykF(0.2f, 0.0f, 0.55f, 0.1f);
        const jp::ColorRef ref = jp::ColorRef::rgb(ink);
        QCOMPARE(ref.toString(), QStringLiteral("cmyk(20,0,55,10)"));
        const jp::ColorRef back = jp::ColorRef::fromString(ref.toString());
        QCOMPARE(back.rgbValue().spec(), QColor::Cmyk);
        QCOMPARE(back, ref);
        QCOMPARE(jp::ColorRef::fromString(QStringLiteral("#12AB34")).rgbValue(), QColor(0x12, 0xab, 0x34));
        QCOMPARE(jp::colorFromString(QStringLiteral("cmyk(0,100,0,0,128)")).alpha(), 128);
        const QColor tint = jp::mix(ink, Qt::white, 0.5);
        QCOMPARE(tint.spec(), QColor::Cmyk);
        QVERIFY(std::abs(tint.cyanF() - 0.1) < 0.002 && std::abs(tint.yellowF() - 0.275) < 0.002 && std::abs(tint.blackF() - 0.05) < 0.002);
        const QColor shade = jp::mix(ink, Qt::black, 0.5);
        QVERIFY(std::abs(shade.blackF() - 0.55) < 0.002 && std::abs(shade.cyanF() - 0.1) < 0.002);

        // The Colors dialog's CMYK entry.
        QColor picked;
        QTimer::singleShot(0, [] {
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QVERIFY(dlg);
            auto *model = dlg->findChild<QComboBox *>();
            model->setCurrentIndex(2);
            const auto spins = dlg->findChildren<jp::DecimalSpin *>();
            QCOMPARE(spins.size(), 4);
            const double inks[] = {35, 5, 0, 20};
            for (int i = 0; i < 4; ++i) spins[i]->setValue(inks[i]);
            if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
                auto *tabs = dlg->findChild<QTabWidget *>();
                tabs->setCurrentIndex(1);
                dlg->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/colors-custom.png");
                tabs->setCurrentIndex(0);
                dlg->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/colors-standard.png");
            }
            dlg->accept();
        });
        picked = jp::colorsDialog(nullptr, Qt::red);
        QCOMPARE(picked.spec(), QColor::Cmyk);
        QVERIFY(std::abs(picked.cyanF() - 0.35) < 0.002 && std::abs(picked.magentaF() - 0.05) < 0.002 && std::abs(picked.blackF() - 0.2) < 0.002);

        // A process-color publication's PDF paints in CMYK.
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->print.model = jp::PrintInfo::ProcessCMYK;
        auto box = std::make_shared<jp::ShapeItem>();
        box->rect = QRectF(72, 72, 200, 100);
        box->fill = jp::Fill::solid(ref);
        box->stroke = jp::Stroke::none();
        doc->pages[0]->items.push_back(box);
        w.editor()->setDocument(std::move(doc));
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("cmyk.pdf"));
        QVERIFY(w.exportPdfTo(path, jp::MainWindow::PdfSettings()));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) { QFile::remove(qEnvironmentVariable("JP_SHOT_DIR") + "/cmyk.pdf"); QFile::copy(path, qEnvironmentVariable("JP_SHOT_DIR") + "/cmyk.pdf"); }
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray pdf = f.readAll();
        // Page contents are deflated: inflate every stream and look for the
        // fill color as ink amounts ("c m y k k").
        QByteArray text;
        for (qsizetype at = 0; (at = pdf.indexOf("stream", at)) >= 0; at += 6) {
            qsizetype start = at + 6;
            if (pdf.mid(start, 2) == "\r\n") start += 2;
            else if (pdf.mid(start, 1) == "\n") ++start;
            const qsizetype end = pdf.indexOf("endstream", start);
            if (end < 0) break;
            QByteArray z = pdf.mid(start, end - start);
            bool ok = false;
            const QByteArray plain = jp::QtPdf::inflate(z, &ok);
            text += ok ? plain : z;
        }
        // Qt sets a CMYK color space and gives the inks with "scn".
        QVERIFY(text.contains("cmyk cs"));
        bool found = false;
        for (const QByteArray &l : text.split('\n')) {
            const QList<QByteArray> v = l.split(' ');
            if (v.size() == 5 && v[4] == "scn")
                found = found || (std::abs(v[0].toDouble() - 0.2) < 1e-3 && v[1].toDouble() < 1e-3 && std::abs(v[2].toDouble() - 0.55) < 1e-3
                                  && std::abs(v[3].toDouble() - 0.1) < 1e-3);
        }
        QVERIFY(found);
    }

    // Create PDF presets shrink pictures, Commercial press adds room for marks,
    // and a page range exports only those pages; printing separations makes
    // one sheet per plate with printer's marks.
    void pdfPresetsAndPrintMarks()
    {
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        for (int i = 0; i < 3; ++i) doc->pages.push_back(std::make_shared<jp::Page>());
        QImage noise(1600, 1600, QImage::Format_RGB32);
        quint32 seed = 7;
        for (int y = 0; y < noise.height(); ++y)
            for (int x = 0; x < noise.width(); ++x) { seed = seed * 1664525u + 1013904223u; noise.setPixel(x, y, seed >> 8); }
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        noise.save(&buf, "PNG");
        auto item = std::make_shared<jp::PictureItem>();
        item->imageId = doc->addImage(png, "png");
        item->rect = QRectF(72, 72, 288, 288);   // 4 in: 400 dpi
        item->imgRect = QRectF(0, 0, 288, 288);
        item->fill = jp::Fill();
        doc->pages[0]->items.push_back(item);
        w.editor()->setDocument(std::move(doc));
        QTemporaryDir dir;
        auto pdfBytes = [&](jp::MainWindow::PdfSettings::Preset preset, int from = 0, int to = -1) {
            jp::MainWindow::PdfSettings s;
            s.preset = preset;
            s.from = from;
            s.to = to;
            const QString path = dir.filePath(QStringLiteral("p%1-%2-%3.pdf").arg(int(preset)).arg(from).arg(to));
            w.exportPdfTo(path, s);
            QFile f(path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        };
        auto pageCount = [](const QByteArray &pdf) { return int(pdf.count("/Type /Page\n") + pdf.count("/Type /Page\r") + pdf.count("/Type /Page ") + pdf.count("/Type /Page/")); };
        const QByteArray minimum = pdfBytes(jp::MainWindow::PdfSettings::Minimum);
        const QByteArray high = pdfBytes(jp::MainWindow::PdfSettings::HighQuality);
        const QByteArray press = pdfBytes(jp::MainWindow::PdfSettings::CommercialPress);
        QVERIFY2(minimum.size() * 4 < high.size(), qPrintable(QStringLiteral("%1 vs %2").arg(minimum.size()).arg(high.size())));
        QVERIFY(high.size() < press.size());
        QCOMPARE(pageCount(high), 4);
        // 612 x 792 plus 48 pt of marks on every side.
        const int mb = press.indexOf("/MediaBox");
        QVERIFY2(press.mid(mb, 40).contains("708.000000 888.000000"), press.mid(mb, 40).constData());
        QCOMPARE(pageCount(pdfBytes(jp::MainWindow::PdfSettings::Standard, 1, 2)), 2);

        // Separations on tabloid paper with every mark: 4 pages x 2 plates.
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(dir.filePath("plates.pdf"));
        printer.setPageSize(QPageSize(QPageSize::Tabloid));
        printer.setResolution(150);
        printer.setFullPage(true);
        QJsonObject opts{{"layout", "one"}, {"separations", true}, {"plates", "CK"}, {"cropMarks", true}, {"bleedMarks", true},
                         {"registration", true}, {"densityBars", true}, {"colorBars", true}, {"jobInfo", true}, {"allowBleeds", true}};
        jp::printDocument(w.editor(), &printer, opts);
        QFile f(dir.filePath("plates.pdf"));
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(pageCount(f.readAll()), 8);
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) QFile::copy(dir.filePath("plates.pdf"), qEnvironmentVariable("JP_SHOT_DIR") + "/plates.pdf");
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) QFile::copy(dir.filePath("p3-0--1.pdf"), qEnvironmentVariable("JP_SHOT_DIR") + "/press.pdf");
        // A plate image: white stays blank, pure cyan is full ink on cyan only.
        QImage rgb(2, 1, QImage::Format_RGB32);
        rgb.setPixel(0, 0, qRgb(255, 255, 255));
        rgb.setPixel(1, 0, qRgb(0, 255, 255));
        QCOMPARE(qGray(jp::separationPlate(rgb, 0).pixel(0, 0)), 255);
        QCOMPARE(qGray(jp::separationPlate(rgb, 0).pixel(1, 0)), 0);
        QCOMPARE(qGray(jp::separationPlate(rgb, 1).pixel(1, 0)), 255);
        QCOMPARE(qGray(jp::separationPlate(rgb, 3).pixel(1, 0)), 255);
    }
};

JP_TEST_CLASS(PubPdfTests)
#include "tst_pub_pdf.moc"
