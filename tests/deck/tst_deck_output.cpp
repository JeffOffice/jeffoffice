// JeffDeck's tests of PDF, pictures, handouts, and printed output.
#include "deckhelpers.h"

class DeckOutputTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

private:
    // The text of a page (0-based) of a PDF as a reader that copies it gets it, its white space single spaces.
    static QString pdfText(const QByteArray &pdf, int page = 0)
    {
        const PdfDocument doc(pdf);
        return doc.isValid() ? doc.text(page).simplified() : QString();
    }
    // The PDF of a presentation's slides (empty when it can't be made).
    static QByteArray pdfOf(const Document &d, QString *err = nullptr)
    {
        QTemporaryDir dir;
        if (!dir.isValid()) return QByteArray();
        const QString path = dir.filePath(QStringLiteral("out.pdf"));
        return deck::exportPdf(d, path, deck::OutputOptions(), err) ? fileBytes(path) : QByteArray();
    }
    // A picture on a slide, `rect` big, showing the image `id` in `imgRect` (its own frame when none is given).
    static PictureItem *addPicture(Page &slide, const QString &id, const QRectF &rect, const QRectF &imgRect = QRectF())
    {
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = id;
        pic->rect = rect;
        pic->imgRect = imgRect.isNull() ? QRectF(QPointF(0, 0), rect.size()) : imgRect;
        slide.items.push_back(pic);
        return pic.get();
    }
    // The size of every page of a PDF, in points.
    static QVector<QSizeF> pdfPageSizes(const QByteArray &pdf)
    {
        static const QRegularExpression box(QStringLiteral("/MediaBox \\[0 0 ([0-9.]+) ([0-9.]+)\\]"));
        QVector<QSizeF> sizes;
        for (auto it = box.globalMatch(QString::fromLatin1(pdf)); it.hasNext();) {
            const QRegularExpressionMatch m = it.next();
            sizes << QSizeF(m.captured(1).toDouble(), m.captured(2).toDouble());
        }
        return sizes;
    }
    // What the pages of a PDF draw: its compressed streams, one after another.
    static QByteArray pdfContent(const QByteArray &pdf)
    {
        QByteArray all;
        for (qsizetype at = pdf.indexOf("stream\n"); at >= 0; at = pdf.indexOf("stream\n", at + 1)) {
            if (at >= 3 && pdf.mid(at - 3, 3) == "end") continue;
            const qsizetype from = at + 7, to = pdf.indexOf("endstream", from);
            if (to < 0) break;
            // The compressed streams (they start as zlib's do), without the length qCompress puts before one.
            if (pdf[from] == 'x') all += qUncompress(QByteArray(4, '\0') + pdf.mid(from, to - from));
        }
        return all;
    }
    // Five slides, titled and with notes, the third hidden.
    static std::unique_ptr<Document> outputDeck(int n = 5)
    {
        auto d = showDeck(n);
        if (n > 2) d->pages[2]->hidden = true;
        return d;
    }
    // Pixels darker than light gray in `area`.
    static int inkIn(const QImage &img, const QRect &area)
    {
        int n = 0;
        for (int y = std::max(0, area.top()); y <= std::min(img.height() - 1, area.bottom()); ++y)
            for (int x = std::max(0, area.left()); x <= std::min(img.width() - 1, area.right()); ++x)
                if (qGray(img.pixel(x, y)) < 200) ++n;
        return n;
    }
    // A page of the output on white, `scale` pixels to the point.
    static QImage outputImage(const Document &d, LayoutCache *cache, const deck::OutputOptions &o, int page, const QSizeF &paper, double scale = 1)
    {
        QImage img((paper * scale).toSize(), QImage::Format_RGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        p.scale(scale, scale);
        deck::paintOutputPage(&p, d, cache, o, page, paper);
        p.end();
        return img;
    }
    // The separate regions of `img` painted `color` (a few levels off is the same), each with its box; those under `least` pixels are specks of text.
    static QVector<QRect> colorRegions(const QImage &img, const QColor &color, int least = 400)
    {
        auto near = [&](int x, int y) {
            const QColor c = img.pixelColor(x, y);
            return std::abs(c.red() - color.red()) < 12 && std::abs(c.green() - color.green()) < 12 && std::abs(c.blue() - color.blue()) < 12;
        };
        QVector<bool> seen(img.width() * img.height(), false);
        QVector<QRect> boxes;
        for (int y0 = 0; y0 < img.height(); ++y0)
            for (int x0 = 0; x0 < img.width(); ++x0) {
                if (seen[y0 * img.width() + x0] || !near(x0, y0)) continue;
                QRect box(x0, y0, 1, 1);
                int size = 0;
                QVector<QPoint> todo{QPoint(x0, y0)};
                seen[y0 * img.width() + x0] = true;
                while (!todo.isEmpty()) {
                    const QPoint at = todo.takeLast();
                    ++size;
                    box |= QRect(at, QSize(1, 1));
                    for (const QPoint &to : {at + QPoint(1, 0), at + QPoint(-1, 0), at + QPoint(0, 1), at + QPoint(0, -1)})
                        if (to.x() >= 0 && to.y() >= 0 && to.x() < img.width() && to.y() < img.height() && !seen[to.y() * img.width() + to.x()] && near(to.x(), to.y())) {
                            seen[to.y() * img.width() + to.x()] = true;
                            todo << to;
                        }
                }
                if (size >= least) boxes << box;
            }
        return boxes;
    }
    static bool hasColor(const QImage &img)
    {
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) {
                const QRgb c = img.pixel(x, y);
                if (qRed(c) != qGreen(c) || qGreen(c) != qBlue(c)) return true;
            }
        return false;
    }

private Q_SLOTS:
    // How many pages each layout makes: a slide each, a notes page each,
    // the handouts' slides to the page. Hidden slides are out unless asked for,
    // and a range names the slides to print.
    void outputPageCountsPerLayout()
    {
        auto d = outputDeck();   // five slides, the third hidden
        deck::OutputOptions o;
        auto pages = [&](deck::PrintLayout layout) {
            o.layout = layout;
            return deck::outputPageCount(*d, o);
        };
        QCOMPARE(pages(deck::PrintLayout::Slides), 4);
        QCOMPARE(pages(deck::PrintLayout::NotesPages), 4);
        QCOMPARE(pages(deck::PrintLayout::Handout1), 4);
        QCOMPARE(pages(deck::PrintLayout::Handout2), 2);
        QCOMPARE(pages(deck::PrintLayout::Handout3), 2);
        QCOMPARE(pages(deck::PrintLayout::Handout4), 1);
        QCOMPARE(pages(deck::PrintLayout::Handout6), 1);
        QCOMPARE(pages(deck::PrintLayout::Handout9), 1);
        QVERIFY(pages(deck::PrintLayout::Outline) >= 1);
        o.hiddenSlides = true;
        QCOMPARE(pages(deck::PrintLayout::Slides), 5);
        QCOMPARE(pages(deck::PrintLayout::NotesPages), 5);
        QCOMPARE(pages(deck::PrintLayout::Handout4), 2);
        QCOMPARE(pages(deck::PrintLayout::Handout6), 1);
        QCOMPARE(pages(deck::PrintLayout::Handout3), 2);
        QCOMPARE(pages(deck::PrintLayout::Handout2), 3);
        o.hiddenSlides = false;
        // A range: those slides, in the order named, without the hidden one or any the presentation lacks.
        o.slides = {4, 0, 2, 7, -1};
        QCOMPARE(deck::outputSlides(*d, o), (QVector<int>{4, 0}));
        QCOMPARE(pages(deck::PrintLayout::Slides), 2);
        o.slides = {2};
        QCOMPARE(pages(deck::PrintLayout::Slides), 0);
        QCOMPARE(pages(deck::PrintLayout::Outline), 0);
        o.hiddenSlides = true;
        QCOMPARE(pages(deck::PrintLayout::Slides), 1);
        QCOMPARE(deck::slidesPerPage(deck::PrintLayout::Handout9), 9);
        QCOMPARE(deck::slidesPerPage(deck::PrintLayout::Slides), 0);
        // Many slides on the outline take several pages; more paper takes fewer.
        auto many = showDeck(60);
        o = deck::OutputOptions();
        o.layout = deck::PrintLayout::Outline;
        const int letter = deck::outputPageCount(*many, o, QSizeF(612, 792)), tall = deck::outputPageCount(*many, o, QSizeF(612, 3000));
        QVERIFY(letter > 1);
        QVERIFY(tall < letter);
    }

    // A slide range as typed: numbers and spans, counted from 1, made 0-based;
    // anything else is no range.
    void slideRangesAreParsed()
    {
        using R = QVector<int>;
        QCOMPARE(deck::parseSlideRange(QStringLiteral("1-3, 5"), 8), (R{0, 1, 2, 4}));
        QCOMPARE(deck::parseSlideRange(QStringLiteral(" 2 ;7-"), 8), (R{1, 6, 7}));
        QCOMPARE(deck::parseSlideRange(QStringLiteral("-2"), 8), (R{0, 1}));
        QCOMPARE(deck::parseSlideRange(QStringLiteral("6-99"), 8), (R{5, 6, 7}));
        QCOMPARE(deck::parseSlideRange(QStringLiteral("5,1"), 8), (R{4, 0}));
        QCOMPARE(deck::parseSlideRange(QStringLiteral("3"), 3), (R{2}));
        for (const char *bad : {"", "  ", "9", "0", "4-2", "1-a", "x", "1,,", "1-2-3", "-"}) QCOMPARE(deck::parseSlideRange(QString::fromLatin1(bad), 8), R());
        QCOMPARE(deck::parseSlideRange(QStringLiteral("1"), 0), R());
    }

    // Printed slides are as big as the slide; the rest are on the locale's paper.
    void outputPageSizes()
    {
        auto d = outputDeck();
        deck::OutputOptions o;
        QCOMPARE(deck::outputPageSize(*d, o), QSizeF(960, 540));
        const QSizeF paper = deck::localePaper();
        QVERIFY(paper == QSizeF(612, 792) || qAbs(paper.width() - 595.28) < 0.5);
        QVERIFY(paper.height() > paper.width());
        for (deck::PrintLayout layout : {deck::PrintLayout::NotesPages, deck::PrintLayout::Handout6, deck::PrintLayout::Outline}) {
            o.layout = layout;
            QCOMPARE(deck::outputPageSize(*d, o), paper);
            QCOMPARE(deck::outputPageSize(*d, o, QSizeF(500, 700)), QSizeF(500, 700));
        }
    }

    // The slides PDF has a page for each slide, each as big as the slide,
    // vector text with its fonts in the file, and the presentation's title.
    void exportPdfWritesOnePagePerSlide()
    {
        auto d = outputDeck();
        d->props.title = QStringLiteral("Quarterly talk");
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        deck::OutputOptions o;
        QString err;
        const QString path = dir.filePath(QStringLiteral("slides.pdf"));
        QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
        QVERIFY(err.isEmpty());
        const QByteArray pdf = fileBytes(path);
        QVERIFY(pdf.startsWith("%PDF-"));
        QCOMPARE(pdfPageCount(pdf), 4);
        QCOMPARE(pdfPageSizes(pdf), (QVector<QSizeF>(4, QSizeF(960, 540))));   // each page is the slide's size
        QVERIFY2(pdf.contains("/FontFile"), "the fonts are in the file");
        QVERIFY(pdf.contains("/Title"));
        QVERIFY(pdf.contains("/Creator"));
        // The only file left in the folder is the PDF (the temporary file was renamed).
        QCOMPARE(QDir(dir.path()).entryList(QDir::Files), QStringList{QStringLiteral("slides.pdf")});
        o.hiddenSlides = true;
        QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
        QCOMPARE(pdfPageCount(fileBytes(path)), 5);
        // A deck without a title is named for the file.
        d->props.title.clear();
        QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
        QVERIFY(fileBytes(path).contains("/Title"));
    }

    // Notes pages and handouts are on the locale's paper, upright.
    void exportPdfNotesAndHandoutsAreOnPaper()
    {
        auto d = outputDeck();
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QSizeF paper = deck::localePaper();
        struct Case { deck::PrintLayout layout; int pages; };
        for (const Case &c : {Case{deck::PrintLayout::NotesPages, 4}, Case{deck::PrintLayout::Handout6, 1}, Case{deck::PrintLayout::Handout3, 2},
                              Case{deck::PrintLayout::Outline, deck::outputPageCount(*d, [] { deck::OutputOptions o; o.layout = deck::PrintLayout::Outline; return o; }())}}) {
            deck::OutputOptions o;
            o.layout = c.layout;
            QString err;
            const QString path = dir.filePath(QStringLiteral("out%1.pdf").arg(int(c.layout)));
            QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
            const QByteArray pdf = fileBytes(path);
            QCOMPARE(pdfPageCount(pdf), c.pages);
            const QVector<QSizeF> sizes = pdfPageSizes(pdf);
            QCOMPARE(int(sizes.size()), c.pages);
            for (const QSizeF &size : sizes) QVERIFY(qAbs(size.width() - paper.width()) < 0.01 && qAbs(size.height() - paper.height()) < 0.01);
            QVERIFY(pdf.contains("/FontFile"));
        }
    }

    // Nothing to print, or nowhere to write, is an error that says so and
    // leaves what was there.
    void exportPdfFailsCleanly()
    {
        auto d = outputDeck();
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        deck::OutputOptions o;
        QString err;
        QVERIFY(!deck::exportPdf(*d, dir.filePath(QStringLiteral("missing/folder/a.pdf")), o, &err));
        QVERIFY(!err.isEmpty());
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("missing"))));
        QVERIFY(QDir(dir.path()).isEmpty());
        // A path that is a folder.
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("folder.pdf")));
        err.clear();
        QVERIFY(!deck::exportPdf(*d, dir.filePath(QStringLiteral("folder.pdf")), o, &err));
        QVERIFY(!err.isEmpty());
        QVERIFY(QFileInfo(dir.filePath(QStringLiteral("folder.pdf"))).isDir());
        QCOMPARE(QDir(dir.path()).entryList(QDir::NoDotAndDotDot | QDir::AllEntries), QStringList{QStringLiteral("folder.pdf")});
        // No slides to print: the old PDF stays whole.
        const QString kept = dir.filePath(QStringLiteral("kept.pdf"));
        QVERIFY(deck::exportPdf(*d, kept, o, &err));
        const QByteArray before = fileBytes(kept);
        QVERIFY(!before.isEmpty());
        o.slides = {2};   // the hidden slide
        err.clear();
        QVERIFY(!deck::exportPdf(*d, kept, o, &err));
        QVERIFY(!err.isEmpty());
        QCOMPARE(fileBytes(kept), before);
        QVERIFY(!deck::exportPdf(*d, kept, o, nullptr));   // no place for the reason is fine
    }

    // The PDF's text names each letter the page draws, so that it can be found and copied: a ligature (the bundled Carlito has "ti", "tt", "ft",
    // and the others) is one glyph that stands for several letters, and a reader can name none of them ("Objectives" copies as "Objec ves"). The
    // PDF's text is set without ligatures.
    void exportPdfTextNamesEveryLetterItDraws()
    {
        auto d = showDeck(1);
        QTextDocument *story = d->storyDoc(static_cast<TextItem *>(d->pages[0]->items[0].get())->storyId);
        story->setPlainText(QStringLiteral("Objectives: creative attitude, fifty flies, offline"));
        QTextCursor c(story);
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setFontFamilies({QStringLiteral("Carlito")});
        f.setFontPointSize(32);
        c.mergeCharFormat(f);
        QString err;
        const QByteArray pdf = pdfOf(*d, &err);
        QVERIFY2(!pdf.isEmpty(), qPrintable(err));
        const QString text = pdfText(pdf);
        for (const char *word : {"Objectives", "creative", "attitude", "fifty", "flies", "offline"}) QVERIFY2(text.contains(QLatin1String(word)), qPrintable(text));
    }

    // A drawing's text is in the PDF as text with no ink, where the drawing's own letters are outlines (Qt's renderer draws an SVG's text as
    // paths): an SVG picture's words (a chart's labels), turned and anchored as the drawing says, and not the words that are hidden.
    void exportPdfCarriesTheTextOfAnSvgPicture()
    {
        const QByteArray svg =
            "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"300\" height=\"200\" viewBox=\"0 0 300 200\">"
            "<rect x=\"0\" y=\"0\" width=\"300\" height=\"200\" fill=\"#eeeeee\"/>"
            "<text x=\"150\" y=\"40\" text-anchor=\"middle\" font-family=\"Arimo\" font-size=\"20\">Sales by region</text>"
            "<text x=\"10\" y=\"80\" font-family=\"Carlito\" font-size=\"20\">Objectives: craft attitude</text>"
            "<g transform=\"translate(30,180) rotate(-90)\"><text x=\"0\" y=\"0\" font-family=\"Arimo\" font-size=\"12\">Revenue axis</text></g>"
            "<g display=\"none\"><text x=\"10\" y=\"10\" font-size=\"12\">Hidden words</text></g>"
            "<defs><text id=\"t\" x=\"10\" y=\"10\" font-size=\"12\">Defined only</text></defs>"
            "<text x=\"60\" y=\"190\" font-size=\"10\"><tspan>Tick </tspan><tspan>2.5</tspan></text></svg>";
        auto d = deck::newPresentation();
        addPicture(*d->pages[0], d->addImage(svg, QStringLiteral("svg")), QRectF(100, 100, 300, 200));
        QString err;
        const QByteArray pdf = pdfOf(*d, &err);
        QVERIFY2(!pdf.isEmpty(), qPrintable(err));
        const QString text = pdfText(pdf);
        QVERIFY2(text.contains(QStringLiteral("Sales by region")), qPrintable(text));
        // (Said without ligatures, as the PDF's own text is: "ti" and "ft" are one glyph each in the bundled Carlito.)
        QVERIFY2(text.contains(QStringLiteral("Objectives: craft attitude")), qPrintable(text));
        // (A reader that copies turned text may set a space between its letters.)
        QVERIFY2(QString(text).remove(QLatin1Char(' ')).contains(QStringLiteral("Revenueaxis")), qPrintable(text));
        QVERIFY2(text.contains(QStringLiteral("Tick 2.5")), qPrintable(text));
        QVERIFY2(!text.contains(QStringLiteral("Hidden")), qPrintable(text));
        QVERIFY2(!text.contains(QStringLiteral("Defined")), qPrintable(text));
    }

    // A drawing can't make the PDF hold more words than it may: at most five thousand texts of one drawing are said, and at most two hundred thousand
    // letters, each text cut at a thousand. (Counted in the text that comes back, not timed.)
    void exportPdfBoundsTheTextOfAnSvgPicture()
    {
        QString err;
        auto say = [&](const QByteArray &svg) {
            auto d = deck::newPresentation();
            addPicture(*d->pages[0], d->addImage(svg, QStringLiteral("svg")), QRectF(100, 100, 300, 200));
            const QByteArray pdf = pdfOf(*d, &err);
            return pdf.isEmpty() ? QString() : pdfText(pdf);
        };
        // Many small texts, each one letter of its own place on a grid (5,300 of them, 100 to a row): five thousand are said, and the first are among them.
        QByteArray many = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"300\" height=\"200\" viewBox=\"0 0 300 200\">";
        for (int i = 0; i < 5300; ++i)
            many += "<text x=\"" + QByteArray::number(2 + (i % 100) * 2.9) + "\" y=\"" + QByteArray::number(5 + (i / 100) * 3.2) + "\" font-size=\"2\">" + (i < 5000 ? "a" : "q") + "</text>";
        many += "</svg>";
        const QString first = say(many);
        QVERIFY2(!first.isEmpty(), qPrintable(err));
        const int as = first.count(QLatin1Char('a'));
        QVERIFY2(as >= 4000 && as <= 5000, qPrintable(QString::number(as)));
        QCOMPARE(first.count(QLatin1Char('q')), 0);   // (the ones after the five thousandth)
        // Long texts: each is cut at a thousand letters, and the letters said stop at two hundred thousand (250 texts of 5,000 give 200). (They are
        // texts the renderer is told not to draw, as a diagram's are, so that the picture isn't a million outlines.)
        QByteArray long_ = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"300\" height=\"200\" viewBox=\"0 0 300 200\">";
        for (int i = 0; i < 250; ++i)
            long_ += "<text class=\"jo-words\" display=\"none\" x=\"2\" y=\"" + QByteArray::number(3 + i * 0.75) + "\" font-size=\"2\">" + QByteArray(5000, 'z') + "</text>";
        long_ += "</svg>";
        const QString second = say(long_);
        QVERIFY2(!second.isEmpty(), qPrintable(err));
        const int zs = second.count(QLatin1Char('z'));
        QCOMPARE(zs, 200000);
    }

    // Text the picture doesn't show isn't said: that a style sheet hides (display none, hidden visibility) or sizes to nothing, text in a nested drawing
    // or in a group another element uses, and (a style sheet with a rule this can't read that sets them) any text at all, as what shows can't be told.
    // The text that shows, sized by the style sheet, is said.
    void exportPdfLeavesOutSvgTextThePictureDoesNotShow()
    {
        QString err;
        auto say = [&](const QByteArray &svg) {
            auto d = deck::newPresentation();
            addPicture(*d->pages[0], d->addImage(svg, QStringLiteral("svg")), QRectF(100, 100, 300, 200));
            const QByteArray pdf = pdfOf(*d, &err);
            return pdf.isEmpty() ? QString() : pdfText(pdf);
        };
        const QByteArray svg =
            "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"300\" height=\"200\" viewBox=\"0 0 300 200\">"
            "<style>.gone{display:none} #ghost{visibility:hidden} .tiny{font-size:0.1px} .cap{font-size:20px}</style>"
            "<text class=\"gone\" x=\"10\" y=\"20\" font-size=\"20\" font-family=\"Arimo\">CssGone</text>"
            "<text id=\"ghost\" x=\"10\" y=\"45\" font-size=\"20\" font-family=\"Arimo\">CssGhost</text>"
            "<text class=\"tiny\" x=\"10\" y=\"70\" font-size=\"20\" font-family=\"Arimo\">CssTiny</text>"
            "<text class=\"cap\" x=\"10\" y=\"95\" font-family=\"Arimo\">CssSized</text>"
            "<svg x=\"150\" y=\"10\" width=\"100\" height=\"50\" viewBox=\"0 0 10 5\"><text x=\"1\" y=\"4\" font-size=\"3\" font-family=\"Arimo\">InsideNested</text></svg>"
            "<g id=\"again\"><text x=\"10\" y=\"125\" font-size=\"20\" font-family=\"Arimo\">UsedTwice</text></g><use xlink:href=\"#again\" x=\"100\"/>"
            "<text x=\"10\" y=\"185\" font-size=\"20\" font-family=\"Arimo\">PlainWords</text></svg>";
        const QString text = say(svg);
        QVERIFY2(!text.isEmpty(), qPrintable(err));
        QVERIFY2(text.contains(QStringLiteral("CssSized")) && text.contains(QStringLiteral("PlainWords")), qPrintable(text));
        for (const char *hidden : {"CssGone", "CssGhost", "CssTiny", "InsideNested", "UsedTwice"}) QVERIFY2(!text.contains(QLatin1String(hidden)), qPrintable(QLatin1String(hidden) + QStringLiteral(": ") + text));
        // A rule of a kind this doesn't read (a descendant) that hides: what shows can't be told, so no text is said.
        const QString unsure = say("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"300\" height=\"200\" viewBox=\"0 0 300 200\"><style>g text{display:none}</style>"
                                   "<g><text x=\"10\" y=\"30\" font-size=\"20\" font-family=\"Arimo\">Descendant</text></g><text x=\"10\" y=\"90\" font-size=\"20\" font-family=\"Arimo\">Elsewhere</text></svg>");
        QVERIFY2(!unsure.contains(QStringLiteral("Descendant")) && !unsure.contains(QStringLiteral("Elsewhere")), qPrintable(unsure));
    }

    // A PDF page breaks its lines where the screen does: the presentation's text is laid out without ligatures wherever it is set (the presentation
    // program applies none), so a ligature-rich paragraph in a tight box has the same lines in the PDF as on the screen, at any width of the box.
    void exportPdfBreaksLinesAsTheScreenDoes()
    {
        const QString words = QStringLiteral("The official office staff offered fifty beautiful flowers, affiliated artifacts, and attitude; the effective traffic of stuff after offline "
                                             "workflows; creative objectives often fit. Definitive effort, fluffy waffle, differing first flights, little shuffle, notification, "
                                             "certificate, sufficient.");
        for (const double width : {100.0, 100.75, 131.5, 163.25, 200.0, 233.25, 290.5, 340.0}) {
            auto d = deck::newPresentation();
            auto t = std::make_shared<TextItem>();
            t->rect = QRectF(20, 20, width, 480);
            t->hyphenate = false;
            t->storyId = d->createStory();
            QTextDocument *story = d->storyDoc(t->storyId);
            QTextCursor c(story);
            QTextCharFormat f;
            f.setFontFamilies({QStringLiteral("Carlito")});
            f.setFontPointSize(18);
            c.insertText(words, f);
            d->pages[0]->items.push_back(t);
            LayoutCache cache;
            const auto screen = cache.textFrame(*d, *t, 1, RenderOptions());
            QVERIFY(screen.layout);
            QStringList onScreen;
            for (const auto &line : screen.layout->lineInfo(0)) onScreen << line.text.simplified();
            QString err;
            const QByteArray pdf = pdfOf(*d, &err);
            QVERIFY2(!pdf.isEmpty(), qPrintable(err));
            const PdfDocument doc(pdf);
            QStringList inPdf;
            for (const QString &line : doc.lines(0)) inPdf << line.simplified();
            QVERIFY2(onScreen == inPdf, qPrintable(QStringLiteral("width %1\n screen: %2\n pdf:    %3").arg(width).arg(onScreen.join('|'), inPdf.join('|'))));
        }
    }

    // A diagram (SmartArt) is a picture of its drawing, whose letters are outlines: the PDF has its words as text too, and the picture itself looks as
    // it did (what says the words draws nothing: the picture without any text element is the same pixels).
    void exportPdfCarriesTheTextOfADiagram()
    {
        QString err;
        auto d = deck::importPresentation(pack(smartArtPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray pdf = pdfOf(*d, &err);
        QVERIFY2(!pdf.isEmpty(), qPrintable(err));
        // (The shapes are narrower than their words, which break where the line ends: the words are whole across it.)
        const QString text = QString(pdfText(pdf)).remove(QRegularExpression(QStringLiteral("\\s")));
        for (const char *word : {"First", "Second", "Third"}) QVERIFY2(text.contains(QLatin1String(word)), qPrintable(text));
        const PictureItem *pic = keptPicture(*d, *d->pages[0], QStringLiteral("diagram"));
        QVERIFY(pic);
        QByteArray svg = d->images.value(pic->imageId).bytes;
        auto draw = [](const QByteArray &bytes) {
            QSvgRenderer r(bytes);
            QImage img(288, 144, QImage::Format_ARGB32_Premultiplied);
            img.fill(Qt::transparent);
            QPainter painter(&img);
            r.render(&painter);
            return img;
        };
        QString stripped = QString::fromUtf8(svg);
        stripped.remove(QRegularExpression(QStringLiteral("<text\\b.*?</text>"), QRegularExpression::DotMatchesEverythingOption));
        const QByteArray without = stripped.toUtf8();
        QVERIFY(without.size() < svg.size());   // (the words are in the drawing as text elements)
        QVERIFY(draw(svg) == draw(without));
    }

    // A diagram's text takes the theme's body font (the drawing's style names it: fontRef minor) over the one the presentation's default text style
    // sets: the picture of the diagram is the same whether that style says the theme's body font (+mn-lt) or Arial (here the theme's body font is Verdana).
    void diagramTextTakesTheThemesBodyFontItsStyleNames()
    {
        QString err;
        auto picture = [&](const QString &defaultFont) {
            Parts p = smartArtPptx();
            QString pres = QString::fromUtf8(p.data["/ppt/presentation.xml"].xml);
            if (defaultFont != QLatin1String("+mn-lt")) pres.replace(QStringLiteral("<a:latin typeface=\"+mn-lt\"/>"), QStringLiteral("<a:latin typeface=\"%1\"/>").arg(defaultFont));
            p.data["/ppt/presentation.xml"].xml = pres.toUtf8();
            auto d = deck::importPresentation(pack(p), &err);
            if (!d) return QByteArray();
            const PictureItem *pic = keptPicture(*d, *d->pages[0], QStringLiteral("diagram"));
            return pic ? d->images.value(pic->imageId).bytes : QByteArray();
        };
        const QByteArray themed = picture(QStringLiteral("+mn-lt")), arial = picture(QStringLiteral("Arial"));
        QVERIFY2(!themed.isEmpty() && !arial.isEmpty(), qPrintable(err));
        // (A pattern in the picture has a number of its own each time: the same picture is the same but for that.)
        auto same = [](QByteArray svg) { return QString::fromUtf8(svg).replace(QRegularExpression(QStringLiteral("texpattern_[0-9a-f]+")), QStringLiteral("texpattern")); };
        QVERIFY(same(themed) == same(arial));
    }

    // A style's body font doesn't take the place of a font a placeholder inherits: the title of a slide whose shape style says fontRef minor keeps
    // the heading font its master's title style names.
    void pptxPlaceholderKeepsTheFontItInherits()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        const QString at = QStringLiteral("<p:spPr></p:spPr><p:txBody>");
        QVERIFY(xml.contains(at));
        xml.replace(xml.indexOf(at), at.size(),
                    QStringLiteral("<p:spPr></p:spPr><p:style><a:lnRef idx=\"0\"><a:scrgbClr r=\"0\" g=\"0\" b=\"0\"/></a:lnRef><a:fillRef idx=\"0\"><a:scrgbClr r=\"0\" g=\"0\" b=\"0\"/></a:fillRef>"
                                   "<a:effectRef idx=\"0\"><a:scrgbClr r=\"0\" g=\"0\" b=\"0\"/></a:effectRef><a:fontRef idx=\"minor\"><a:schemeClr val=\"tx1\"/></a:fontRef></p:style><p:txBody>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const QTextDocument *title = placeholderStory(*d, *d->pages[0], QStringLiteral("ph:title"));
        QVERIFY(title && !title->isEmpty());
        const QTextCharFormat f = title->begin().begin().fragment().charFormat();
        QCOMPARE(f.stringProperty(tp::ThemeFont), QStringLiteral("major"));
    }

    // The words a picture's pixels hold (kept with a Macintosh picture's PNG) are in the PDF as text with no ink, where they were drawn, and not the
    // ones that fall outside the part of the picture the slide shows.
    void exportPdfCarriesTheWordsKeptWithAPicture()
    {
        QImage img(400, 100, QImage::Format_ARGB32);
        img.fill(Qt::white);
        img.setText(QString::fromLatin1(kDrawnWordsKey), drawnWordsToText({{QStringLiteral("Outside 1.5"), QStringLiteral("Arimo"), QPointF(0.1, 0.5), 0.1, false, false},
                                                                           {QStringLiteral("Inside 2.5"), QStringLiteral("Arimo"), QPointF(0.6, 0.5), 0.1, true, false}}));
        QByteArray png;
        QBuffer buf(&png);
        QVERIFY(buf.open(QIODevice::WriteOnly) && img.save(&buf, "PNG"));
        QVERIFY(QImage::fromData(png).text(QString::fromLatin1(kDrawnWordsKey)).contains(QLatin1String("Inside 2.5")));
        auto d = deck::newPresentation();
        // A picture 200 wide that shows the right half of the image (twice its width, cropped): the first word is cropped out.
        addPicture(*d->pages[0], d->addImage(png, QStringLiteral("png")), QRectF(100, 100, 200, 100), QRectF(-200, 0, 400, 100));
        QString err;
        const QByteArray pdf = pdfOf(*d, &err);
        QVERIFY2(!pdf.isEmpty(), qPrintable(err));
        const QString text = pdfText(pdf);
        QVERIFY2(text.contains(QStringLiteral("Inside 2.5")), qPrintable(text));
        QVERIFY2(!text.contains(QStringLiteral("Outside")), qPrintable(text));
    }

    // The text of a metafile picture (a chart pasted as a picture, an embedded object's preview), in either kind of metafile, is in the PDF as text.
    void exportPdfCarriesTheTextOfAMetafilePicture()
    {
        auto words = [](std::initializer_list<int> v) { return wmfWords(v); };
        auto ints = [](std::initializer_list<qint32> v) {
            QByteArray b;
            for (qint32 w : v) b += le(quint32(w), 4);
            return b;
        };
        const QString text = QStringLiteral("Axis 2.5");
        // A metafile of the older kind: a window, a font (24 units tall, Arimo), the text color, and the string at 20 across and 60 down.
        QByteArray font = words({-24, 0, 0, 0, 400});
        font.append(char(0)).append(char(0)).append(char(0)).append(char(0));
        font += QByteArray(4, '\0') + QByteArray("Arimo") + '\0';
        QByteArray line = words({int(text.size())}) + text.toLatin1();
        line += words({60, 20});   // y, then x
        QByteArray body = wmfRecord(0x020B, words({0, 0})) + wmfRecord(0x020C, words({300, 400})) + wmfRecord(0x0102, words({1})) + wmfRecord(0x02FB, font) + wmfRecord(0x012D, words({0}))
                          + wmfRecord(0x0209, QByteArray(4, '\0')) + wmfRecord(0x0521, line) + wmfRecord(0x0000, QByteArray());
        QByteArray head = wmfDword(0x9AC6CDD7u) + wmfWords({0, 0, 0, 400, 300, 72}) + wmfDword(0);
        quint16 sum = 0;
        for (int i = 0; i < 20; i += 2) sum ^= quint16(quint8(head[i]) | (quint8(head[i + 1]) << 8));
        head += wmfWords({int(sum)});
        const QByteArray standard = wmfWords({1, 9, 0x300}) + wmfDword(quint32((18 + body.size()) / 2)) + wmfWords({2}) + wmfDword(60) + wmfWords({0});
        const QByteArray older = head + standard + body;
        // The same in the newer kind: records with a type and their length in bytes, the text in UTF-16.
        auto record = [&](quint32 type, const QByteArray &payload) {
            QByteArray pad = payload;
            while (pad.size() % 4) pad.append('\0');
            return le(type, 4) + le(quint32(8 + pad.size()), 4) + pad;
        };
        QByteArray lf = ints({1, -24, 0, 0, 0, 400});
        lf.append(char(0)).append(char(0)).append(char(0)).append(char(0));
        lf += QByteArray(4, '\0');
        QByteArray name(64, '\0');
        const QString face = QStringLiteral("Arimo");
        for (int i = 0; i < face.size(); ++i) name[2 * i] = char(face[i].unicode());
        QByteArray chars;
        for (QChar c : text) chars += le(c.unicode(), 2);
        while (chars.size() % 4) chars.append('\0');
        const int fixed = 8 + 16 + 12 + 40;
        // (No widths for the letters: the string is drawn whole.)
        const QByteArray run = ints({0, 0, 0, 0, 1}) + QByteArray(8, '\0') + ints({20, 60, int(text.size()), fixed, 0, 0, 0, 0, 0, 0}) + chars;
        QByteArray newer = record(1, ints({0, 0, 399, 299, 0, 0, 10583, 7937, qint32(0x464D4520u), 0x10000, 0, 1, 1, 0, 0, 0, 3840, 2880, 1016, 762}))
                           + record(18, ints({1})) + record(82, lf + name) + record(37, ints({1})) + record(24, ints({0})) + record(84, run) + record(14, ints({0, 16, 20}));
        qToLittleEndian<quint32>(quint32(newer.size()), newer.data() + 48);
        for (const auto &kind : {qMakePair(older, QStringLiteral("wmf")), qMakePair(newer, QStringLiteral("emf"))}) {
            auto d = deck::newPresentation();
            const QString id = d->addImage(kind.first, kind.second);
            QVERIFY2(!id.isEmpty(), qPrintable(kind.second));
            addPicture(*d->pages[0], id, QRectF(100, 100, 400, 300));
            QString err;
            const QByteArray pdf = pdfOf(*d, &err);
            QVERIFY2(!pdf.isEmpty(), qPrintable(err));
            QVERIFY2(pdfText(pdf).contains(text), qPrintable(kind.second + QLatin1Char(' ') + pdfText(pdf)));
        }
    }

    // A notes page has the slide framed in its upper half and the speaker
    // notes under it; a page without notes has nothing there.
    void notesPagePaintsTheSlideAboveItsNotes()
    {
        auto d = outputDeck();
        deck::setSlideBackground(*d->pages[0], QColor(0xC0, 0x20, 0x20));
        d->storyDoc(d->pages[1]->notesStoryId)->clear();
        LayoutCache cache;
        deck::OutputOptions o;
        o.layout = deck::PrintLayout::NotesPages;
        const QSizeF paper = deck::localePaper();
        const int w = int(paper.width()), h = int(paper.height());
        const QImage with = outputImage(*d, &cache, o, 0, paper, 2);
        // The slide: one red region in the upper half, framed, in the margins.
        const QVector<QRect> slide = colorRegions(with, QColor(0xC0, 0x20, 0x20));
        QCOMPARE(int(slide.size()), 1);
        QVERIFY(slide[0].top() >= 20);   // (pixels, 2 to the point)
        QVERIFY2(slide[0].bottom() < h, "the slide is above the middle");
        QVERIFY(slide[0].left() > 20 && slide[0].right() < 2 * w - 20);
        QVERIFY2(slide[0].width() * 9 / 16 - 4 <= slide[0].height() && slide[0].height() <= slide[0].width() * 9 / 16 + 4, "the slide keeps its shape");
        QVERIFY2(inkIn(with, QRect(slide[0].left() - 1, slide[0].center().y(), 1, 1)) == 1, "the slide is framed");
        // The notes are under the slide, and no lower than the page number.
        const QRect lower(40 * 2, slide[0].bottom() + 12, (w - 80) * 2, 2 * h - 160 - slide[0].bottom());
        QVERIFY2(inkIn(with, lower) > 20, "the notes");
        QCOMPARE(inkIn(with, QRect(0, 0, 2 * w, 30)), 0);
        // No notes: nothing there, but the page number at the foot.
        const QImage without = outputImage(*d, &cache, o, 1, paper, 2);
        QCOMPARE(inkIn(without, lower), 0);
        QVERIFY(inkIn(without, QRect(2 * w - 200, 2 * h - 100, 200, 100)) > 0);
        QVERIFY(inkIn(without, QRect(0, 2 * h - 100, w, 100)) == 0);
    }

    // A handout of six has six slides in a grid, each framed; the last page has the rest.
    void handoutOfSixFramesSixSlides()
    {
        auto d = showDeck(7);
        for (auto &page : d->pages) deck::setSlideBackground(*page, QColor(0x1F, 0x5F, 0xBF));
        LayoutCache cache;
        deck::OutputOptions o;
        o.layout = deck::PrintLayout::Handout6;
        QCOMPARE(deck::outputPageCount(*d, o), 2);
        const QSizeF paper = deck::localePaper();
        const QImage first = outputImage(*d, &cache, o, 0, paper, 2);
        const QVector<QRect> six = colorRegions(first, QColor(0x1F, 0x5F, 0xBF));
        QCOMPARE(int(six.size()), 6);
        for (const QRect &slide : six) {
            QVERIFY2(slide.width() > 100 && slide.width() * 9 / 16 - 4 <= slide.height() && slide.height() <= slide.width() * 9 / 16 + 4, "each keeps its shape");
            // The frame is dark on every side.
            for (const QPoint &outside : {QPoint(slide.left() - 1, slide.center().y()), QPoint(slide.right() + 1, slide.center().y()),
                                          QPoint(slide.center().x(), slide.top() - 1), QPoint(slide.center().x(), slide.bottom() + 1)})
                QVERIFY2(qGray(first.pixel(outside)) < 120, qPrintable(QStringLiteral("frame at %1,%2").arg(outside.x()).arg(outside.y())));
        }
        // Two columns of three rows.
        QSet<int> xs, ys;
        for (const QRect &slide : six) {
            xs << slide.left() / 20;
            ys << slide.top() / 20;
        }
        QCOMPARE(int(xs.size()), 2);
        QCOMPARE(int(ys.size()), 3);
        const QImage last = outputImage(*d, &cache, o, 1, paper, 2);
        QCOMPARE(int(colorRegions(last, QColor(0x1F, 0x5F, 0xBF)).size()), 1);
        // Across, then down puts the second slide beside the first; down, then across puts it under.
        auto firstTwo = [&](bool horizontal) {
            o.horizontalOrder = horizontal;
            for (int i = 0; i < 6; ++i) d->pages[i]->background = Fill();
            deck::setSlideBackground(*d->pages[0], QColor(0xC0, 0x20, 0x20));
            deck::setSlideBackground(*d->pages[1], QColor(0x20, 0xA0, 0x20));
            const QImage img = outputImage(*d, &cache, o, 0, paper, 1);
            return QPair<QVector<QRect>, QVector<QRect>>(colorRegions(img, QColor(0xC0, 0x20, 0x20), 100), colorRegions(img, QColor(0x20, 0xA0, 0x20), 100));
        };
        auto across = firstTwo(true);
        QCOMPARE(int(across.first.size()), 1);
        QCOMPARE(int(across.second.size()), 1);
        QVERIFY(across.second[0].left() > across.first[0].right() && across.second[0].top() == across.first[0].top());
        auto down = firstTwo(false);
        QCOMPARE(int(down.first.size()), 1);
        QCOMPARE(int(down.second.size()), 1);
        QVERIFY(down.second[0].top() > down.first[0].bottom() && down.second[0].left() == down.first[0].left());
    }

    // Three to a page leave lines for notes to the right of the slides.
    void handoutOfThreeHasLinesBesideTheSlides()
    {
        auto d = showDeck(3);
        for (auto &page : d->pages) deck::setSlideBackground(*page, QColor(0x1F, 0x5F, 0xBF));
        LayoutCache cache;
        deck::OutputOptions o;
        o.layout = deck::PrintLayout::Handout3;
        const QSizeF paper = deck::localePaper();
        const QImage img = outputImage(*d, &cache, o, 0, paper);
        const QVector<QRect> three = colorRegions(img, QColor(0x1F, 0x5F, 0xBF));
        QCOMPARE(int(three.size()), 3);
        int right = 0;
        for (const QRect &slide : three) right = std::max(right, slide.right());
        QVERIFY2(right < paper.width() * 0.55, "the slides are to the left");
        // Lines: dark runs in the right part, none of which is a slide.
        const int from = int(paper.width() * 0.62);
        QVERIFY(inkIn(img, QRect(from, 80, int(paper.width()) - from - 20, int(paper.height()) - 160)) > 300);
        o.layout = deck::PrintLayout::Handout4;
        QCOMPARE(int(colorRegions(outputImage(*d, &cache, o, 0, paper), QColor(0x1F, 0x5F, 0xBF)).size()), 3);
    }

    // Gray output has no color in it, on any layout; the same page in color does.
    void grayscaleOutputHasNoColor()
    {
        auto d = showDeck(3);
        deck::setSlideBackground(*d->pages[0], QColor(0xC0, 0x20, 0x20));
        deck::setSlideBackground(*d->pages[1], QColor(0x20, 0xA0, 0x20));
        LayoutCache cache;
        deck::OutputOptions o;
        for (deck::PrintLayout layout : {deck::PrintLayout::Slides, deck::PrintLayout::NotesPages, deck::PrintLayout::Handout6}) {
            o.layout = layout;
            const QSizeF paper = deck::outputPageSize(*d, o);
            o.grayscale = false;
            const QImage color = outputImage(*d, &cache, o, 0, paper, 0.5);
            QVERIFY2(hasColor(color), "the slide is red in color");
            o.grayscale = true;
            const QImage gray = outputImage(*d, &cache, o, 0, paper, 0.5);
            QVERIFY2(!hasColor(gray), qPrintable(QStringLiteral("layout %1").arg(int(layout))));
            QVERIFY(inkIn(gray, QRect(QPoint(0, 0), gray.size())) > 100);   // still drawn
        }
        // The PDF is made in gray.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString err;
        o.layout = deck::PrintLayout::Slides;
        const QString path = dir.filePath(QStringLiteral("gray.pdf"));
        QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
        QCOMPARE(pdfPageCount(fileBytes(path)), 3);
        // Colors in the pages are set with three numbers, grays with one.
        static const QRegularExpression rgb(QStringLiteral("[0-9.]+ [0-9.]+ [0-9.]+ scn"));
        const QString gray = QString::fromLatin1(pdfContent(fileBytes(path)));
        QVERIFY(gray.contains(QStringLiteral(" scn")));
        QVERIFY2(!gray.contains(rgb), "a color in the gray PDF");
        o.grayscale = false;
        QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
        QVERIFY2(QString::fromLatin1(pdfContent(fileBytes(path))).contains(rgb), "the PDF in color has colors");
    }

    // Printed slides are framed only when asked for.
    void printedSlidesAreFramedOnRequest()
    {
        auto d = showDeck(2);
        deck::setSlideBackground(*d->pages[0], QColor(0xE0, 0xE0, 0x20));
        LayoutCache cache;
        deck::OutputOptions o;
        const QSizeF paper(960, 540);
        QImage plain = outputImage(*d, &cache, o, 0, paper);
        QCOMPARE(plain.pixelColor(0, 270), QColor(0xE0, 0xE0, 0x20));
        QCOMPARE(plain.pixelColor(959, 270), QColor(0xE0, 0xE0, 0x20));
        o.frameSlides = true;
        QImage framed = outputImage(*d, &cache, o, 0, paper);
        QVERIFY(qGray(framed.pixel(0, 270)) < 120);
        QVERIFY(qGray(framed.pixel(959, 270)) < 120);
        QVERIFY(qGray(framed.pixel(480, 0)) < 120);
        QVERIFY(qGray(framed.pixel(480, 539)) < 120);
        QCOMPARE(framed.pixelColor(20, 270), QColor(0xE0, 0xE0, 0x20));
        // On bigger paper the slide is as large as fits, centered.
        const QImage onPaper = outputImage(*d, &cache, o, 0, QSizeF(1200, 900));
        const QVector<QRect> slide = colorRegions(onPaper, QColor(0xE0, 0xE0, 0x20));
        QCOMPARE(int(slide.size()), 1);
        QVERIFY(qAbs(slide[0].width() - 1200) <= 6 && qAbs(slide[0].center().y() - 450) <= 3);
    }

    // The outline is the slides' titles with their body text by level.
    void outlineListsTitlesAndBodyByLevel()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        auto story = [&](const Page &slide, const QString &type) -> QTextDocument * {
            for (const auto &it : slide.items)
                if (deck::placeholderType(*it) == type && it->type() == ItemType::Text) return d->storyDoc(static_cast<TextItem *>(it.get())->storyId);
            return nullptr;
        };
        story(*d->pages[0], QStringLiteral("ctrTitle"))->setPlainText(QStringLiteral("Quarterly results"));
        story(*d->pages[0], QStringLiteral("subTitle"))->setPlainText(QStringLiteral("North region"));
        story(*d->pages[1], QStringLiteral("title"))->setPlainText(QStringLiteral("Highlights"));
        QTextDocument *body = story(*d->pages[1], QStringLiteral("body"));
        if (!body) body = story(*d->pages[1], QStringLiteral("obj"));
        QVERIFY(body);
        body->setPlainText(QStringLiteral("Sales grew\nUp 12 percent\nCosts fell\n\nHiring"));
        QTextCursor c(body->findBlockByNumber(1));
        QTextBlockFormat deeper;
        deeper.setProperty(tp::ListLevel, 1);
        c.mergeBlockFormat(deeper);
        // The third slide has no title and no text.
        deck::OutputOptions o;
        o.layout = deck::PrintLayout::Outline;
        QCOMPARE(deck::outputOutlineText(*d, o),
                 QStringLiteral("1  Quarterly results\n  North region\n2  Highlights\n  Sales grew\n    Up 12 percent\n  Costs fell\n  Hiring\n3\n"));
        // A range keeps the numbers of the slides.
        o.slides = {1};
        QCOMPARE(deck::outputOutlineText(*d, o), QStringLiteral("2  Highlights\n  Sales grew\n    Up 12 percent\n  Costs fell\n  Hiring\n"));
        // The page has them in ink.
        o.slides.clear();
        LayoutCache cache;
        const QSizeF paper = deck::localePaper();
        const QImage page = outputImage(*d, &cache, o, 0, paper);
        QVERIFY(inkIn(page, QRect(40, 40, int(paper.width()) - 80, 200)) > 200);
        QCOMPARE(inkIn(page, QRect(40, int(paper.height()) / 2, int(paper.width()) - 80, int(paper.height()) / 2 - 40)), 0);
    }

    // A slide as a picture: 1920 pixels across, the slide's shape, on its own background.
    void slideImageIs1920PixelsWide()
    {
        auto d = showDeck(2);
        deck::setSlideBackground(*d->pages[1], QColor(0x20, 0x80, 0x40));
        LayoutCache cache;
        const QImage img = deck::slideImage(*d, &cache, 1);
        QCOMPARE(img.size(), QSize(1920, 1080));
        QCOMPARE(img.pixelColor(5, 5), QColor(0x20, 0x80, 0x40));
        QCOMPARE(deck::slideImage(*d, &cache, 0, 640).size(), QSize(640, 360));
        QVERIFY(deck::slideImage(*d, &cache, 9).isNull());
        // A standard-shaped slide keeps its shape.
        deck::setSlideSize(*d, QSizeF(deck::kStandardWidth, deck::kStandardHeight));
        QCOMPARE(deck::slideImage(*d, &cache, 0, 800).size(), QSize(800, 600));
    }

    // Printing paints each page on the printer's sheet, in gray (as pictures) when the printer or the options ask.
    void printerOutputPaintsEveryPage()
    {
        auto d = outputDeck();
        deck::setSlideBackground(*d->pages[0], QColor(0xC0, 0x20, 0x20));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        LayoutCache cache;
        for (bool gray : {false, true}) {
            const QString path = dir.filePath(gray ? QStringLiteral("gray.pdf") : QStringLiteral("color.pdf"));
            {
                QPrinter printer(QPrinter::HighResolution);
                printer.setOutputFormat(QPrinter::PdfFormat);
                printer.setOutputFileName(path);
                printer.setPageSize(QPageSize(deck::localePaper(), QPageSize::Point, QString(), QPageSize::FuzzyMatch));
                deck::OutputOptions o;
                o.layout = deck::PrintLayout::Handout3;
                if (gray) printer.setColorMode(QPrinter::GrayScale);
                o.grayscale = gray;
                QPainter p(&printer);
                QVERIFY(p.isActive());
                const QSizeF sheet = printer.pageRect(QPrinter::Point).size();
                const int pages = deck::outputPageCount(*d, o, sheet);
                QCOMPARE(pages, 2);
                for (int i = 0; i < pages; ++i) {
                    if (i) printer.newPage();
                    p.save();
                    p.scale(printer.resolution() / 72.0, printer.resolution() / 72.0);
                    deck::paintOutputPage(&p, *d, &cache, o, i, sheet);
                    p.restore();
                }
            }
            const QByteArray pdf = fileBytes(path);
            QCOMPARE(pdfPageCount(pdf), 2);
            static const QRegularExpression rgb(QStringLiteral("[0-9.]+ [0-9.]+ [0-9.]+ scn"));
            const QString content = QString::fromLatin1(pdfContent(pdf));
            QCOMPARE(content.contains(rgb), !gray);
            QCOMPARE(pdf.contains("/Subtype /Image"), gray);
        }
    }

    // Print and the exports are in the File menu's commands, and Ctrl+P prints.
    void outputCommandsExist()
    {
        DeckWindow w;
        for (const char *id : {"file.exportPdf", "file.print", "file.exportPicture"}) {
            QVERIFY2(w.act(id), id);
            QVERIFY2(!w.act(id)->text().isEmpty(), id);
        }
        QCOMPARE(w.act("file.print")->shortcut(), QKeySequence(QKeySequence::Print));
        QVERIFY(w.act("file.exportPdf")->text().endsWith(QChar(0x2026)));
    }

    // Export to PDF asks what to print, then where, and writes the file; a
    // refusal at either step writes nothing, and a file that can't be written is said so.
    void windowExportsPdf()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.newSlide();
        w.newSlide();
        const QString path = dir.filePath(QStringLiteral("talk.pdf"));
        QStringList asked;
        w.setSavePathChooser([&](const QString &suggested, const QString &filter) {
            asked << suggested << filter;
            return path;
        });
        auto choose = [](deck::PrintLayout layout) {
            return [layout](QDialog *dialog) {
                auto *what = dialog->findChild<QComboBox *>(QStringLiteral("outputWhat"));
                QVERIFY(what);
                const int at = what->findData(int(layout));
                QVERIFY(at >= 0);
                what->setCurrentIndex(at);
            };
        };
        // Cancelled at the first step.
        answerOutputDialog(nullptr, QDialogButtonBox::Cancel);
        w.act("file.exportPdf")->trigger();
        QVERIFY(asked.isEmpty());
        QVERIFY(!QFileInfo::exists(path));
        // Slides, as the dialog starts: a page for each.
        answerOutputDialog();
        w.act("file.exportPdf")->trigger();
        QCOMPARE(asked.size(), 2);
        QVERIFY2(asked[0].endsWith(QLatin1String("Presentation1.pdf")), qPrintable(asked[0]));
        QVERIFY(asked[1].contains(QLatin1String("*.pdf")));
        QCOMPARE(pdfPageCount(fileBytes(path)), 3);
        QCOMPARE(pdfPageSizes(fileBytes(path)).value(0), QSizeF(960, 540));
        QVERIFY(w.statusBar()->currentMessage().contains(QLatin1String("talk.pdf")));
        // Notes pages, only the slides 2 to 3.
        answerOutputDialog([&](QDialog *dialog) {
            choose(deck::PrintLayout::NotesPages)(dialog);
            dialog->findChild<QRadioButton *>(QStringLiteral("outputRangeSlides"))->setChecked(true);
            dialog->findChild<QLineEdit *>(QStringLiteral("outputRangeText"))->setText(QStringLiteral("2-3"));
        });
        w.act("file.exportPdf")->trigger();
        QCOMPARE(pdfPageCount(fileBytes(path)), 2);
        QCOMPARE(pdfPageSizes(fileBytes(path)).value(0), deck::localePaper());
        // The slide being edited, hidden slides left out.
        w.editor()->setCurrentPage(1);
        w.toggleHidden();
        QVERIFY(QFile::remove(path));
        answerOutputDialog(
            [&](QDialog *dialog) {
                dialog->findChild<QRadioButton *>(QStringLiteral("outputRangeCurrent"))->setChecked(true);
                QVERIFY2(!dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->isEnabled(), "the only slide chosen is hidden: nothing to write");
            },
            QDialogButtonBox::Cancel);
        w.act("file.exportPdf")->trigger();
        QVERIFY(!QFileInfo::exists(path));
        answerOutputDialog([&](QDialog *dialog) { dialog->findChild<QCheckBox *>(QStringLiteral("outputHidden"))->setChecked(true); });
        w.act("file.exportPdf")->trigger();
        QCOMPARE(pdfPageCount(fileBytes(path)), 3);
        // No name chosen writes nothing.
        QVERIFY(QFile::remove(path));
        w.setSavePathChooser([](const QString &, const QString &) { return QString(); });
        answerOutputDialog();
        w.act("file.exportPdf")->trigger();
        QVERIFY(!QFileInfo::exists(path));
        // A place that can't be written is an error the person sees.
        QString label;
        w.setSavePathChooser([&](const QString &, const QString &) {
            answerDialogs({QMessageBox::Ok}, &label);
            return dir.filePath(QStringLiteral("missing/talk.pdf"));
        });
        answerOutputDialog();
        w.act("file.exportPdf")->trigger();
        QCOMPARE(label, QStringLiteral("OK"));
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("missing"))));
    }

    // Export slide as picture saves the slide being edited, 1920 pixels across,
    // as the kind of picture the name asks for.
    void windowExportsSlidePicture()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.newSlide();
        deck::setSlideBackground(*w.editor()->doc()->pages[1], QColor(0x20, 0x80, 0x40));
        w.editor()->setCurrentPage(1);
        QString suggested, path = dir.filePath(QStringLiteral("slide.png"));
        w.setSavePathChooser([&](const QString &name, const QString &) {
            suggested = name;
            return path;
        });
        w.act("file.exportPicture")->trigger();
        QVERIFY2(suggested.endsWith(QLatin1String(".png")) && suggested.contains(QLatin1String("2")), qPrintable(suggested));
        QImage png(path);
        QCOMPARE(png.size(), QSize(1920, 1080));
        QCOMPARE(png.pixelColor(4, 4), QColor(0x20, 0x80, 0x40));
        path = dir.filePath(QStringLiteral("slide.jpg"));
        w.act("file.exportPicture")->trigger();
        QImage jpeg(path);
        QCOMPARE(jpeg.size(), QSize(1920, 1080));
        QVERIFY(qAbs(jpeg.pixelColor(4, 4).green() - 0x80) < 8);
        QCOMPARE(fileBytes(path).left(2), QByteArray("\xff\xd8"));
        // No ending takes the kind the dialog showed.
        path = dir.filePath(QStringLiteral("bare"));
        w.act("file.exportPicture")->trigger();
        QVERIFY(QFileInfo::exists(path + QLatin1String(".png")));
        // No name, no file; a place that can't be written is said.
        const int before = int(QDir(dir.path()).entryList(QDir::Files).size());
        w.setSavePathChooser([](const QString &, const QString &) { return QString(); });
        w.act("file.exportPicture")->trigger();
        QCOMPARE(int(QDir(dir.path()).entryList(QDir::Files).size()), before);
        QString label;
        w.setSavePathChooser([&](const QString &, const QString &) {
            answerDialogs({QMessageBox::Ok}, &label);
            return dir.filePath(QStringLiteral("missing/slide.png"));
        });
        w.act("file.exportPicture")->trigger();
        QCOMPARE(label, QStringLiteral("OK"));
    }
};

JP_TEST_CLASS(DeckOutputTests)
#include "tst_deck_output.moc"
