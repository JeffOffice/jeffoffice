// JeffDeck's tests of PDF, pictures, handouts, and printed output.
#include "deckhelpers.h"
#include "../localepin.h"
#include "../withdialog.h"
#include "../pdfmarkcheck.h"
#include "../withdialog.h"
#include <QSpinBox>
#include "../plainpdfwatch.h"
#include "app/passworddialogs.h"
#include "office/officecrypt.h"
#include "io/pdfcrypt.h"
#include "core/notesmaster.h"
#include "core/printheaderfooter.h"
#include <QBuffer>
#include <QImageReader>
#include "render/picturefile.h"
#include "render/svgexport.h"
#include <QXmlStreamReader>
#include "../settingguard.h"

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

    // Handouts and notes pages print on Letter in the United States and Canada, and on A4 elsewhere (the United Kingdom and Australia among them).
    void localePaperFollowsTheRegion()
    {
        for (const char *letter : {"en_US", "en_CA", "es_MX"}) {
            PinnedLocale pinned{QLocale(QLatin1String(letter))};
            QCOMPARE(deck::localePaper(), QSizeF(612, 792));
        }
        for (const char *a4 : {"en_GB", "en_AU", "de_DE", "en_NZ"}) {
            PinnedLocale pinned{QLocale(QLatin1String(a4))};
            QVERIFY2(qAbs(deck::localePaper().width() - 595.28) < 0.5, a4);
        }
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

    // Whatever font an SVG's text is set in, the PDF's text has every letter: the bundled blackletter joins "c" and "k" into one glyph.
    void exportPdfSvgTextKeepsLettersOfAJoiningFont()
    {
        const QByteArray svg =
            "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"300\" height=\"100\" viewBox=\"0 0 300 100\">"
            "<text x=\"10\" y=\"40\" font-family=\"UnifrakturMaguntia\" font-size=\"24\">Tick Black Backdrop</text></svg>";
        auto d = deck::newPresentation();
        addPicture(*d->pages[0], d->addImage(svg, QStringLiteral("svg")), QRectF(100, 100, 300, 100));
        QString err;
        const QByteArray pdf = pdfOf(*d, &err);
        QVERIFY2(!pdf.isEmpty(), qPrintable(err));
        const QString text = pdfText(pdf);
        QVERIFY2(text.contains(QStringLiteral("Tick Black Backdrop")), qPrintable(text));
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

    // A diagram is a picture of its drawing, whose letters are outlines: the PDF has its words as text too, and the picture itself looks as
    // it did (what says the words draws nothing: the picture without any text element is the same pixels).
    void exportPdfCarriesTheTextOfADiagram()
    {
        QString err;
        auto d = deck::importPresentation(pack(diagramPptx()), &err);
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
            Parts p = diagramPptx();
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
            QVERIFY(!pdf.contains("/Subtype /Image"));   // (a printer that makes a PDF makes it gray itself, as the PDF export does: the page stays vector, and its text stays text)
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

    // The PDF has a bookmark for each slide (its title, or "Slide N" for an untitled one) and is tagged: a section for each slide, its title a heading, its objects in the order the slide
    // lists them, a picture with its alt text. PDF/A keeps both and still opens.
    void pdfHasBookmarksAndTagsForEachSlide()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto d = showDeck(3);
        d->storyDoc(static_cast<TextItem *>(d->pages[1]->items[0].get())->storyId)->setPlainText(QString());
        // listed last, so read last, though it is above the title
        auto extra = std::make_shared<TextItem>();
        extra->rect = QRectF(10, 0, 200, 20);
        extra->storyId = d->createStory(QStringLiteral("Extra words"));
        d->pages[0]->items.push_back(extra);
        QImage img(8, 8, QImage::Format_RGB32);
        img.fill(Qt::red);
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "PNG");
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = d->addImage(png, "png");
        pic->rect = QRectF(100, 300, 120, 120);
        pic->imgRect = QRectF(QPointF(0, 0), pic->rect.size());
        pic->altText = QStringLiteral("A red square");
        d->pages[0]->items.push_back(pic);
        auto write = [&](const QString &name, const deck::OutputOptions &o) {
            QString err;
            const QString path = dir.filePath(name);
            if (!deck::exportPdf(*d, path, o, &err)) return QByteArray();
            return fileBytes(path);
        };
        const QByteArray bytes = write(QStringLiteral("a.pdf"), deck::OutputOptions());
        PdfDocument pdf(bytes);
        QVERIFY(pdf.isValid());
        QCOMPARE(pdf.pageCount(), 3);
        const auto outline = pdf.outline();
        QCOMPARE(outline.size(), 3);
        QCOMPARE(outline[0].title, QStringLiteral("Slide 1 title"));
        QCOMPARE(outline[1].title, QStringLiteral("Slide 2"));
        QCOMPARE(outline[2].title, QStringLiteral("Slide 3 title"));
        for (int i = 0; i < 3; ++i) QCOMPARE(outline[i].page, i);
        QVERIFY2(pdf.isTagged(), "not tagged");
        const auto tree = pdf.structure(0);
        QStringList kinds, read;
        QString alt;
        for (const auto &e : tree) {
            kinds << e.type;
            if (!e.text.isEmpty()) read << e.text;
            if (e.type == QLatin1String("Figure")) alt = e.alt;
        }
        QCOMPARE(kinds.value(0), QStringLiteral("Document"));
        QCOMPARE(kinds.value(1), QStringLiteral("Sect"));
        QCOMPARE(kinds.value(2), QStringLiteral("H1"));
        QCOMPARE(read, (QStringList{QStringLiteral("Slide 1 title"), QStringLiteral("Extra words")}));
        QCOMPARE(alt, QStringLiteral("A red square"));
        // left out when not asked for
        deck::OutputOptions bare;
        bare.tags = bare.bookmarks = false;
        PdfDocument plain(write(QStringLiteral("b.pdf"), bare));
        QVERIFY(plain.isValid());
        QVERIFY(plain.outline().isEmpty());
        QVERIFY(!plain.isTagged());
        // PDF/A keeps both
        deck::OutputOptions archive;
        archive.archive = true;
        const QByteArray kept = write(QStringLiteral("c.pdf"), archive);
        PdfDocument keptDoc(kept);
        QVERIFY(keptDoc.isValid());
        QVERIFY(keptDoc.isTagged());
        QVERIFY2(keptDoc.structure(0).size() > 2, "the PDF/A has a tag tree with nothing in it");
        QCOMPARE(keptDoc.outline().size(), 3);
        QVERIFY2(kept.contains("pdfaid:part=\"1\""), "no PDF/A-1 mark");
        QVERIFY(kept.contains("GTS_PDFA1"));
        // handouts: a bookmark for each slide, to the page it is on
        deck::OutputOptions handout;
        handout.layout = deck::PrintLayout::Handout2;
        PdfDocument handDoc(write(QStringLiteral("d.pdf"), handout));
        QCOMPARE(handDoc.outline().size(), 3);
        QCOMPARE(handDoc.outline()[2].page, 1);
    }

    // Where the tags can't be made, whatever the reason, the PDF has none of the marks, shows what an untagged one shows, and the caller is told it is not tagged.
    void aPdfThatCantBeTaggedHasNoMarksAndSaysSo()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto d = showDeck(2);
        deck::OutputOptions off;
        off.tags = false;
        QString err;
        const QString plainPath = dir.filePath(QStringLiteral("plain.pdf"));
        QVERIFY(deck::exportPdf(*d, plainPath, off, &err));
        PdfDocument plain(fileBytes(plainPath));
        const QImage reference = plain.render(0, QSize(480, 270));
        for (int failure = 1; failure <= 4; ++failure) {
            const auto reset = qScopeGuard([] { pdfTagTestFailure() = 0; });
            pdfTagTestFailure() = failure;
            const QString path = dir.filePath(QStringLiteral("f%1.pdf").arg(failure));
            QString notTagged;
            QVERIFY(deck::exportPdf(*d, path, deck::OutputOptions(), &err, nullptr, &notTagged));
            const QByteArray bytes = fileBytes(path);
            const QByteArray note = QByteArray("failure ") + QByteArray::number(failure);
            PdfDocument doc(bytes);
            QVERIFY2(doc.isValid(), note.constData());
            QVERIFY2(pdfcheck::marksLeft(bytes) == 0, note.constData());
            QVERIFY2(!doc.isTagged(), note.constData());
            QVERIFY2(doc.render(0, QSize(480, 270)) == reference, note.constData());
            QVERIFY2(!notTagged.isEmpty(), note.constData());
        }
    }

    // /Lang is the language of the slides' text; a divided table cell lists both its parts; a password keeps the tags; and every
    // mark of drawing is in a tag or in decoration, on slides, notes pages, and handouts.
    void slideTagsHaveTheTextsLanguageBothPartsOfACellAndAllDrawingIsTaggedOrDecoration()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto d = showDeck(2);
        // a band on the master, a page color, and a table with a divided cell
        d->pages[0]->background = Fill::solid(ColorRef::rgb(QColor(QStringLiteral("#EEEEFF"))));
        if (MasterPage *m = d->masterFor(*d->pages[0])) {
            auto band = std::make_shared<ShapeItem>();
            band->rect = QRectF(0, 0, 960, 20);
            band->fill = Fill::solid(ColorRef::rgb(QColor(Qt::blue)));
            m->items.push_back(band);
        }
        auto table = std::make_shared<TableItem>();
        table->rows = 1;
        table->cols = 1;
        table->colW = {300};
        table->rowH = {120};
        table->rect = QRectF(100, 300, 0, 0);
        table->syncRect();
        table->cells.resize(1);
        table->cells[0].diagonal = 1;
        table->cells[0].fill = Fill::solid(ColorRef::rgb(QColor(QStringLiteral("#DDFFDD"))));
        table->cells[0].storyId = d->createStory(QStringLiteral("Upper words"));
        table->cells[0].storyId2 = d->createStory(QStringLiteral("Lower words"));
        d->pages[0]->items.push_back(table);
        // all of the letters are French
        for (auto &story : d->stories) {
            QTextCursor cur(story->doc.get());
            cur.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setProperty(tp::Language, QStringLiteral("fr-FR"));
            cur.mergeCharFormat(f);
        }
        QString err;
        for (auto layout : {deck::PrintLayout::Slides, deck::PrintLayout::NotesPages, deck::PrintLayout::Handout2}) {
            deck::OutputOptions o;
            o.layout = layout;
            o.frameSlides = true;
            const QString path = dir.filePath(QStringLiteral("a%1.pdf").arg(int(layout)));
            QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
            const QByteArray bytes = fileBytes(path);
            PdfDocument pdf(bytes);
            QVERIFY(pdf.isValid());
            QVERIFY(pdf.isTagged());
            for (const QByteArray &page : pdfcheck::pageContents(bytes))
                QVERIFY2(pdfcheck::drawnOutsideMarkedContent(page) == 0, qPrintable(QStringLiteral("layout %1 draws outside the tags").arg(int(layout))));
            QVERIFY2(bytes.contains("/Lang (fr-FR)"), "/Lang is not the text's");
            if (layout != deck::PrintLayout::Slides) continue;
            // the divided cell has both of its parts
            QStringList cells;
            for (const auto &e : pdf.structure(0))
                if (e.type == QLatin1String("TD") || e.type == QLatin1String("TH")) cells << e.text;
            QCOMPARE(cells.size(), 1);
            QVERIFY2(cells[0].contains(QLatin1String("Upper")) && cells[0].contains(QLatin1String("Lower")), qPrintable(cells.join(QLatin1Char('|'))));
        }
        // English by default
        auto plainDeck = showDeck(1);
        QVERIFY(deck::exportPdf(*plainDeck, dir.filePath(QStringLiteral("en.pdf")), deck::OutputOptions(), &err));
        QVERIFY(fileBytes(dir.filePath(QStringLiteral("en.pdf"))).contains("/Lang (en-US)"));
        // with a password
        PdfProtection protect;
        const auto wipe = qScopeGuard([&protect] { protect.wipe(); });
        protect.openPassword = QStringLiteral("pw");
        QVERIFY(deck::exportPdf(*plainDeck, dir.filePath(QStringLiteral("pw.pdf")), deck::OutputOptions(), &err, &protect));
        const QByteArray locked = fileBytes(dir.filePath(QStringLiteral("pw.pdf")));
        QVERIFY(!PdfDocument(locked).isValid());
        PdfDocument opened(locked, QStringLiteral("pw"));
        QVERIFY(opened.isValid());
        QVERIFY(opened.isTagged());
        // a picture marked as decoration and a hidden object are not tagged
        auto deco = showDeck(1);
        auto img = std::make_shared<PictureItem>();
        QImage red(8, 8, QImage::Format_RGB32);
        red.fill(Qt::red);
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        red.save(&buf, "PNG");
        img->imageId = deco->addImage(png, "png");
        img->rect = QRectF(100, 300, 100, 100);
        img->imgRect = QRectF(QPointF(0, 0), img->rect.size());
        img->decorative = true;
        deco->pages[0]->items.push_back(img);
        auto secret = std::make_shared<TextItem>();
        secret->rect = QRectF(500, 300, 200, 40);
        secret->storyId = deco->createStory(QStringLiteral("Hidden words"));
        secret->hidden = true;
        deco->pages[0]->items.push_back(secret);
        QVERIFY(deck::exportPdf(*deco, dir.filePath(QStringLiteral("deco.pdf")), deck::OutputOptions(), &err));
        PdfDocument decoPdf(fileBytes(dir.filePath(QStringLiteral("deco.pdf"))));
        for (const auto &e : decoPdf.structure(0)) {
            QVERIFY2(e.type != QLatin1String("Figure"), "a picture marked as decoration is a figure");
            QVERIFY2(!e.text.contains(QLatin1String("Hidden")), "a hidden object is tagged");
        }
    }

    // An outline and a grayscale PDF can't be tagged: the box says so, and is off.
    void exportBoxTurnsTagsOffWhereTheyCantBeMade()
    {
        DeckWindow w;
        w.editor()->setDocument(showDeck(2));
        w.setSavePathChooser([](const QString &, const QString &) { return QString(); });
        answerOutputDialog(
            [&](QDialog *dialog) {
                auto *tags = dialog->findChild<QCheckBox *>(QStringLiteral("pdfTags"));
                auto *marks = dialog->findChild<QCheckBox *>(QStringLiteral("pdfBookmarks"));
                QVERIFY(tags->isEnabled());
                QVERIFY(tags->toolTip().isEmpty());
                dialog->findChild<QCheckBox *>(QStringLiteral("outputGray"))->setChecked(true);
                QVERIFY2(!tags->isEnabled(), "tags are offered for a grayscale PDF");
                QVERIFY(!tags->toolTip().isEmpty());
                QVERIFY(marks->isEnabled());
                dialog->findChild<QCheckBox *>(QStringLiteral("outputGray"))->setChecked(false);
                auto *what = dialog->findChild<QComboBox *>(QStringLiteral("outputWhat"));
                what->setCurrentIndex(what->findData(int(deck::PrintLayout::Outline)));
                QVERIFY2(!tags->isEnabled(), "tags are offered for an outline");
                QVERIFY(!marks->isEnabled());
                QVERIFY(!tags->toolTip().isEmpty());
            },
            QDialogButtonBox::Cancel);
        w.act("file.exportPdf")->trigger();
    }

    // The Export to PDF box offers the tags and the bookmarks, with the words JeffWrite's PDF options use, both on as it starts; turned off, the file has neither.
    void exportBoxOffersTagsAndBookmarks()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.editor()->setDocument(showDeck(2));
        const QString path = dir.filePath(QStringLiteral("t.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return path; });
        answerOutputDialog([&](QDialog *dialog) {
            auto *tags = dialog->findChild<QCheckBox *>(QStringLiteral("pdfTags"));
            auto *marks = dialog->findChild<QCheckBox *>(QStringLiteral("pdfBookmarks"));
            QVERIFY(tags && marks);
            QCOMPARE(tags->text(), QStringLiteral("Document structure tags for accessibility"));
            QCOMPARE(marks->text(), QStringLiteral("Create bookmarks"));
            QVERIFY(tags->isChecked() && marks->isChecked());
        });
        w.act("file.exportPdf")->trigger();
        QVERIFY(PdfDocument(fileBytes(path)).isTagged());
        QVERIFY(!PdfDocument(fileBytes(path)).outline().isEmpty());
        answerOutputDialog([&](QDialog *dialog) {
            dialog->findChild<QCheckBox *>(QStringLiteral("pdfTags"))->setChecked(false);
            dialog->findChild<QCheckBox *>(QStringLiteral("pdfBookmarks"))->setChecked(false);
        });
        w.act("file.exportPdf")->trigger();
        QVERIFY(!PdfDocument(fileBytes(path)).isTagged());
        QVERIFY(PdfDocument(fileBytes(path)).outline().isEmpty());
    }

    // The Export to PDF box offers PDF/A (as JeffWrite's does): checked, the file has the standard's mark and an output intent, and an ordinary export has neither; with a password, it is off.
    void windowExportsPdfA()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.newSlide();
        QString target;
        w.setSavePathChooser([&](const QString &, const QString &) { return target; });
        target = dir.filePath(QStringLiteral("plain.pdf"));
        answerOutputDialog();
        w.act("file.exportPdf")->trigger();
        target = dir.filePath(QStringLiteral("archive.pdf"));
        bool offered = false;
        answerOutputDialog([&](QDialog *dialog) {
            auto *archive = dialog->findChild<QCheckBox *>(QStringLiteral("pdfArchive"));
            offered = archive != nullptr;
            if (archive) archive->setChecked(true);
        });
        w.act("file.exportPdf")->trigger();
        QVERIFY(offered);
        const QByteArray plain = fileBytes(dir.filePath(QStringLiteral("plain.pdf"))), kept = fileBytes(target);
        QVERIFY(plain.startsWith("%PDF") && kept.startsWith("%PDF"));
        QVERIFY(!plain.contains("pdfaid:part"));
        QVERIFY2(kept.contains("pdfaid:part=\"1\""), "no PDF/A-1 mark");
        QVERIFY(kept.contains("GTS_PDFA1") || kept.contains("OutputIntent"));
    }

    // Make an Email puts the presentation in a message file (.eml) as a .pptx attachment, or a PDF of it; a presentation with a password is attached with the password on it.
    void anEmailHasThePresentationAttached()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString eml = dir.filePath(QStringLiteral("mail.eml"));
        auto attachmentOf = [&](const char *type) {
            const QByteArray mail = fileBytes(eml);
            const qsizetype at = mail.indexOf(type);
            if (at < 0) return QByteArray();
            const qsizetype body = mail.indexOf("\r\n\r\n", at);
            qsizetype end = mail.indexOf("\r\n--", body + 4);
            if (end < 0) end = mail.size();
            return QByteArray::fromBase64(mail.mid(body + 4, end - body - 4), QByteArray::Base64Encoding | QByteArray::IgnoreBase64DecodingErrors);
        };
        {
            DeckWindow w;
            w.newSlide();
            w.setSavePathChooser([&](const QString &, const QString &) { return eml; });
            QVERIFY(w.exportEmail(DeckWindow::EmailWhat::Presentation));
            QVERIFY(fileBytes(eml).contains("filename=\"Presentation1.pptx\""));
            QString err;
            auto back = deck::importPresentation(attachmentOf("presentationml.presentation"), &err);
            QVERIFY2(back, qPrintable(err));
            QCOMPARE(int(back->pages.size()), 2);
            QVERIFY(w.exportEmail(DeckWindow::EmailWhat::Pdf));
            QVERIFY(attachmentOf("application/pdf").startsWith("%PDF"));
        }
        // A presentation with a password is attached with the password on it: the attachment is the protected package, and opens with the password only.
        {
            auto d = deck::newPresentation();
            office::EncryptOptions quick;
            quick.spinCount = 50;
            const QString protectedPath = dir.filePath(QStringLiteral("locked.pptx"));
            {
                QFile f(protectedPath);
                QVERIFY(f.open(QIODevice::WriteOnly));
                f.write(office::encryptPackage(deck::exportPresentation(*d), QStringLiteral("hunter2"), quick));
            }
            setOpenPasswordStandIn([](const QString &, bool, QString *given) {
                *given = QStringLiteral("hunter2");
                return true;
            });
            const auto reset = qScopeGuard([] { setOpenPasswordStandIn(nullptr); });
            DeckWindow w;
            QVERIFY(w.openFile(protectedPath));
            QVERIFY(w.isPasswordProtected());
            w.setSavePathChooser([&](const QString &, const QString &) { return eml; });
            QVERIFY(w.exportEmail(DeckWindow::EmailWhat::Presentation));
            const QByteArray attached = attachmentOf("presentationml.presentation");
            QByteArray info, package, zip;
            QVERIFY2(office::splitProtectedPackage(attached, &info, &package), "the attachment is not protected");
            QVERIFY(office::decryptPackage(info, package, QStringLiteral("wrong"), 1 << 28, false, &zip) != office::CryptResult::Ok);
            QVERIFY(office::decryptPackage(info, package, QStringLiteral("hunter2"), 1 << 28, false, &zip) == office::CryptResult::Ok);
            QVERIFY(deck::importPresentation(zip) != nullptr);
        }
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
    // File > Export > Save slides as pictures: every slide to a folder as slide 1, 2, and so on, PNG or JPEG, at the width asked (1920 by default), the hidden ones only when asked.
    void windowSavesAllSlidesAsPictures()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.newSlide();
        w.newSlide();
        w.newSlide();
        w.editor()->doc()->pages[2]->hidden = true;
        deck::setSlideBackground(*w.editor()->doc()->pages[0], QColor(0x20, 0x80, 0x40));
        QVERIFY(w.act("file.exportSlidePictures"));
        deck::SlidePicturesOptions o;
        o.folder = dir.filePath(QStringLiteral("png"));
        QVERIFY(QDir().mkpath(o.folder));
        QCOMPARE(w.exportSlidePictures(o), 3);
        // (the names are the translated "Slide N", whatever the language)
        QStringList want = {deck::slidePictureName(0, false), deck::slidePictureName(1, false), deck::slidePictureName(3, false)};
        want.sort();
        QCOMPARE(QDir(o.folder).entryList(QDir::Files, QDir::Name), want);
        QVERIFY(deck::slidePictureName(0, false).endsWith(QLatin1String("1.png")) && deck::slidePictureName(3, true).endsWith(QLatin1String("4.jpg")));
        const QImage first(o.folder + QLatin1Char('/') + deck::slidePictureName(0, false));
        QCOMPARE(first.size(), QSize(1920, 1080));
        QCOMPARE(first.pixelColor(4, 4), QColor(0x20, 0x80, 0x40));
        // JPEG at another width, with the hidden slide too.
        o.folder = dir.filePath(QStringLiteral("jpg"));
        QVERIFY(QDir().mkpath(o.folder));
        o.jpeg = true;
        o.width = 640;
        o.hidden = true;
        QCOMPARE(w.exportSlidePictures(o), 4);
        QCOMPARE(QDir(o.folder).entryList(QDir::Files, QDir::Name).size(), 4);
        const QString third = o.folder + QLatin1Char('/') + deck::slidePictureName(2, true);
        QCOMPARE(QImage(third).size(), QSize(640, 360));
        QCOMPARE(fileBytes(third).left(2), QByteArray("\xff\xd8"));
        // A folder that is not there is said, and nothing is made.
        QString label;
        answerDialogs({QMessageBox::Ok}, &label);
        o.folder = dir.filePath(QStringLiteral("missing/here"));
        QCOMPARE(w.exportSlidePictures(o), 0);
        QCOMPARE(label, QStringLiteral("OK"));
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("missing"))));
    }
    // Files of the same names already in the folder are asked about once for the whole set (Replace or Cancel); Cancel changes nothing.
    void savingSlidePicturesAsksOnceBeforeReplacingFiles()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.newSlide();
        w.newSlide();
        deck::SlidePicturesOptions o;
        o.folder = dir.path();
        const QString two = dir.filePath(deck::slidePictureName(1, false));
        QFile old(two);
        QVERIFY(old.open(QIODevice::WriteOnly));
        old.write("old");
        old.close();
        answerDialogs({QMessageBox::Cancel});
        QCOMPARE(w.exportSlidePictures(o), 0);
        QCOMPARE(fileBytes(two), QByteArray("old"));
        QVERIFY(!QFileInfo::exists(dir.filePath(deck::slidePictureName(0, false))));
        QString label;
        answerDialogs({QMessageBox::Save}, &label);   // (one answer is all the set needs)
        QCOMPARE(w.exportSlidePictures(o), 3);
        QVERIFY(fileBytes(two) != QByteArray("old"));
        QCOMPARE(QDir(dir.path()).entryList(QDir::Files).size(), 3);
    }

    // Every slide hidden is said, and nothing is made.
    void savingSlidePicturesSaysWhenEverySlideIsHidden()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.editor()->doc()->pages[0]->hidden = true;
        deck::SlidePicturesOptions o;
        o.folder = dir.path();
        QString label;
        answerDialogs({QMessageBox::Ok}, &label);
        QCOMPARE(w.exportSlidePictures(o), 0);
        QCOMPARE(label, QStringLiteral("OK"));
        QVERIFY(QDir(dir.path()).entryList(QDir::Files).isEmpty());
    }

    // A picture is at most 64 million pixels in all: a wider one is made as wide as fits.
    void aSlidePictureIsHeldTo64MillionPixels()
    {
        auto d = outputDeck();
        const QSizeF size = d->pageSize();
        QCOMPARE(deck::slidePictureWidth(*d, 1920), 1920);
        const int wide = deck::slidePictureWidth(*d, 16000);
        QVERIFY(wide < 16000);
        QVERIFY(qint64(wide) * qint64(std::lround(wide * size.height() / size.width())) <= deck::kMostSlidePicturePixels);
        QVERIFY(qint64(wide + 40) * qint64(std::lround((wide + 40) * size.height() / size.width())) > deck::kMostSlidePicturePixels);
    }

    // The work reports each slide done and stops when asked; a file that can't be written says how many were written before it.
    void savingSlidePicturesReportsStopsAndCountsWhatWasWritten()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto d = outputDeck();   // five slides, the third hidden
        LayoutCache cache;
        deck::SlidePicturesOptions o;
        o.folder = dir.path();
        o.width = 200;
        QVector<int> seen;
        o.progress = [&](int done, int total) {
            seen << done << total;
            return done < 2;
        };
        deck::SlidePicturesResult r = deck::exportSlidePictures(*d, &cache, o);
        QVERIFY(r.cancelled && r.error.isEmpty());
        QCOMPARE(r.written.size(), qsizetype(2));
        QCOMPARE(seen, (QVector<int>{1, 4, 2, 4}));
        // (a directory where a file belongs: the third file can't be made)
        QTemporaryDir other;
        o.folder = other.path();
        o.progress = nullptr;
        QVERIFY(QDir().mkdir(other.filePath(deck::slidePictureName(1, false))));
        r = deck::exportSlidePictures(*d, &cache, o);
        QVERIFY(!r.error.isEmpty() && !r.cancelled);
        QCOMPARE(r.written, QStringList{deck::slidePictureName(0, false)});
    }

    // The box asks for the format, the folder, the width (1920 at first), and the hidden slides, and the command does what it says.
    void theSlidePicturesBoxAsksAndSaves()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.newSlide();
        w.editor()->doc()->pages[1]->hidden = true;
        withDialog([&] { w.act("file.exportSlidePictures")->trigger(); }, QStringLiteral("slidePicturesDialog"), [&](QDialog *d) {
            auto *width = d->findChild<QSpinBox *>(QStringLiteral("slidePicturesWidth"));
            QVERIFY(width);
            QCOMPARE(width->value(), 1920);
            QVERIFY(!d->findChild<QCheckBox *>(QStringLiteral("slidePicturesHidden"))->isChecked());
            d->findChild<QLineEdit *>(QStringLiteral("slidePicturesFolder"))->setText(dir.path());
            width->setValue(400);
            d->findChild<QCheckBox *>(QStringLiteral("slidePicturesHidden"))->setChecked(true);
            auto *format = d->findChild<QComboBox *>(QStringLiteral("slidePicturesFormat"));
            format->setCurrentIndex(1);
        });
        QTRY_COMPARE(QDir(dir.path()).entryList(QDir::Files, QDir::Name), (QStringList{QStringLiteral("Slide 1.jpg"), QStringLiteral("Slide 2.jpg")}));
        QCOMPARE(QImage(dir.filePath(QStringLiteral("Slide 1.jpg"))).width(), 400);
    }
    // Export to PDF can protect the file with a password: the file written is encrypted (AES-256), opens with the password and not without, and nothing else is left beside it.
    void anExportWithAPasswordIsEncrypted()
    {
        QTemporaryDir dir, keep;
        ScratchTemp scratch;
        DeckWindow w;
        QString path = keep.filePath(QStringLiteral("plain.pdf"));
        w.setSavePathChooser([&](const QString &, const QString &) { return path; });
        // The control: an export with no password leaves a plain PDF, and the watch sees it.
        PlainPdfWatch watch;
        watch.dirs = {keep.path()};
        answerOutputDialog();
        QVERIFY(w.exportPdf());
        QVERIFY2(!watch.plain.isEmpty(), "the watch does not see a plain PDF");
        jp::PdfEncryptionInfo info;
        QVERIFY(!jp::readPdfEncryption(fileBytes(path), QString(), &info) || !info.encrypted);
        QVERIFY(PdfDocument(fileBytes(path)).isValid());
        // With a password: nothing but encrypted PDFs in the folder and the temporary folder, at any moment of the export.
        path = dir.filePath(QStringLiteral("locked.pdf"));
        watch.dirs.clear();
        watch.plain.clear();
        watch.dirs = {dir.path(), scratch.path()};
        jp::setPdfProtectionStandIn([](bool, jp::PdfProtection *p) {
            p->openPassword = QStringLiteral("pw");
            return jp::NewPasswordChoice::Set;
        });
        const auto reset = qScopeGuard([] { jp::setPdfProtectionStandIn(nullptr); });
        answerOutputDialog([](QDialog *d) {
            auto *button = d->findChild<QPushButton *>(QStringLiteral("pdfEncryptButton"));
            QVERIFY(button);
            button->click();
        });
        QVERIFY(w.exportPdf());
        watch.dirs.clear();
        QVERIFY2(watch.plain.isEmpty(), qPrintable(watch.plain.join(QLatin1Char(' '))));
        const QByteArray made = fileBytes(path);
        QVERIFY(jp::readPdfEncryption(made, QStringLiteral("pw"), &info));
        QCOMPARE(info.version, 5);
        QCOMPARE(info.revision, 6);
        QVERIFY(!jp::readPdfEncryption(made, QString(), &info));
        QVERIFY(!PdfDocument(made).isValid());
        const PdfDocument opened(made, QStringLiteral("pw"));
        QVERIFY(opened.isValid());
        QCOMPARE(opened.pageCount(), int(w.editor()->doc()->pages.size()));
        QCOMPARE(QDir(dir.path()).entryList(QDir::NoDotAndDotDot | QDir::AllEntries), QStringList{QStringLiteral("locked.pdf")});
        QVERIFY2(QDir(scratch.path()).entryList(QDir::Files).isEmpty(), qPrintable(QDir(scratch.path()).entryList(QDir::Files).join(QLatin1Char(' '))));
        // The next export asks again: the password was for that one.
        path = dir.filePath(QStringLiteral("again.pdf"));
        answerOutputDialog();
        QVERIFY(w.exportPdf());
        QVERIFY(PdfDocument(fileBytes(path)).isValid());
    }

    // A PDF that asks not to be copied is placed all the same, with a note.
    void aPdfThatAsksNotToBeCopiedIsPlacedWithANote()
    {
        QTemporaryDir dir;
        DeckWindow w;
        const QString path = dir.filePath(QStringLiteral("nocopy.pdf"));
        {
            // (made by the shared function from a plain export)
            QString err;
            QVERIFY2(deck::exportPdf(*w.editor()->doc(), dir.filePath(QStringLiteral("plain.pdf")), deck::OutputOptions(), &err), qPrintable(err));
            jp::PdfProtection p;
            p.openPassword = QStringLiteral("pw");
            p.limited = true;
            p.ownerPassword = QStringLiteral("owner");
            p.allowCopy = false;
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(jp::encryptPdf(fileBytes(dir.filePath(QStringLiteral("plain.pdf"))), p));
        }
        jp::setOpenPasswordStandIn([](const QString &, bool, QString *password) {
            *password = QStringLiteral("pw");
            return true;
        });
        const auto reset = qScopeGuard([] { jp::setOpenPasswordStandIn(nullptr); });
        QVERIFY(w.insertPicture(path));
        QVERIFY2(w.statusBar()->currentMessage().contains(QStringLiteral("asks not to be copied")), qPrintable(w.statusBar()->currentMessage()));
    }

    // A picture that is a PDF with a password asks for it: a wrong one asks again, and Cancel places nothing.
    void insertingAPdfWithAPasswordAsksForIt()
    {
        QTemporaryDir dir;
        DeckWindow w;
        const QString path = dir.filePath(QStringLiteral("locked.pdf"));
        {
            jp::PdfProtection p;
            p.openPassword = QStringLiteral("s3cret");
            QString err;
            QVERIFY2(deck::exportPdf(*w.editor()->doc(), path, deck::OutputOptions(), &err, &p), qPrintable(err));
        }
        const auto reset = qScopeGuard([] { jp::setOpenPasswordStandIn(nullptr); });
        auto itemCount = [&] { return w.editor()->doc()->pages[w.editor()->currentPage()]->items.size(); };
        const size_t before = itemCount();
        QList<bool> wrongBefore;
        jp::setOpenPasswordStandIn([&](const QString &, bool wrong, QString *password) {
            wrongBefore << wrong;
            *password = wrongBefore.size() == 1 ? QStringLiteral("nope") : QStringLiteral("s3cret");
            return true;
        });
        QVERIFY(w.insertPicture(path));
        QCOMPARE(wrongBefore, (QList<bool>{false, true}));
        QCOMPARE(itemCount(), before + 1);
        int asked = 0;
        jp::setOpenPasswordStandIn([&](const QString &, bool, QString *password) {
            *password = QStringLiteral("nope");
            return ++asked < 2;
        });
        QVERIFY(!w.insertPicture(path));
        QCOMPARE(asked, 2);
        QCOMPARE(itemCount(), before + 1);
    }

    // ---- Print: color, scale to fit paper, and the header and footer of notes pages and handouts ----

    // The pixels of a printed page, drawn as the print preview draws it (a point to a pixel).
    static QImage printedPage(const Document &d, const deck::OutputOptions &o, int page = 0)
    {
        const QSizeF sheet = deck::printSheetSize(d, o);
        QImage img(sheet.toSize(), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);
        QPainter p(&img);
        LayoutCache cache;
        deck::paintOutputPage(&p, d, &cache, o, page, sheet);
        p.end();
        return img;
    }

    void pureBlackAndWhitePrintsNothingButBlackAndWhite()
    {
        auto d = deck::newPresentation();
        auto box = std::make_shared<ShapeItem>();
        box->rect = QRectF(40, 300, 400, 120);
        box->fill = Fill::solid(ColorRef::rgb(QColor(200, 30, 30)));
        d->pages[0]->items.push_back(box);
        deck::OutputOptions o;
        auto stats = [](const QImage &img, int *colored, int *gray, int *black) {
            *colored = *gray = *black = 0;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x) {
                    const QColor c = img.pixelColor(x, y);
                    if (std::max({c.red(), c.green(), c.blue()}) - std::min({c.red(), c.green(), c.blue()}) > 20) ++*colored;
                    else if (c.red() > 40 && c.red() < 215) ++*gray;
                    else if (c.red() <= 40) ++*black;
                }
        };
        int colored = 0, gray = 0, black = 0;
        stats(printedPage(*d, o), &colored, &gray, &black);
        QVERIFY2(colored > 100, "in color the red box is red");
        o.grayscale = true;
        stats(printedPage(*d, o), &colored, &gray, &black);
        QCOMPARE(colored, 0);
        QVERIFY2(gray > 100, "in grayscale the box is a gray");
        o.blackAndWhite = true;
        stats(printedPage(*d, o), &colored, &gray, &black);
        QCOMPARE(colored, 0);
        QCOMPARE(gray, 0);
        QVERIFY2(black > 100, "in black and white the box and the words are black");
    }

    void scaleToFitPaperFillsThePaperAndOffKeepsTheSlidesOwnSize()
    {
        auto d = deck::newPresentation();
        auto box = std::make_shared<ShapeItem>();
        box->rect = QRectF(QPointF(0, 0), d->pageSize());
        box->fill = Fill::solid(ColorRef::rgb(QColor(10, 10, 10)));
        d->pages[0]->items.push_back(box);
        deck::OutputOptions o;
        o.paper = QSizeF(842, 1191);   // (A3: larger than a slide in either way)
        auto inked = [](const QImage &img) {
            QRect r;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (img.pixelColor(x, y).red() < 60) r = r.united(QRect(x, y, 1, 1));
            return r;
        };
        QVERIFY(o.scaleToFit);   // (as printing always did)
        const QRect fitted = inked(printedPage(*d, o));
        QVERIFY2(fitted.width() > d->pageSize().width() * 1.1, qPrintable(QString::number(fitted.width())));
        o.scaleToFit = false;
        const QRect own = inked(printedPage(*d, o));
        QVERIFY2(qAbs(own.width() - d->pageSize().width()) <= 2, qPrintable(QString::number(own.width())));
        // (centered on the sheet)
        const QSizeF sheet = deck::printSheetSize(*d, o);
        QVERIFY(qAbs(own.center().x() - sheet.width() / 2) <= 2 && qAbs(own.center().y() - sheet.height() / 2) <= 2);
        // A slide larger than the paper is still made to fit it.
        o.paper = QSizeF(300, 400);
        const QSizeF small = deck::printSheetSize(*d, o);
        QVERIFY(inked(printedPage(*d, o)).width() <= small.width() + 1);
    }

    void notesPagesAndHandoutsPrintTheirHeaderAndFooter()
    {
        auto d = deck::newPresentation();
        d->props.title = QStringLiteral("Quarterly results");
        QString err;
        auto textOf = [&](deck::PrintLayout layout) {
            deck::OutputOptions o;
            o.layout = layout;
            o.tags = false;
            QTemporaryDir dir;
            const QString path = dir.filePath(QStringLiteral("o.pdf"));
            if (!deck::exportPdf(*d, path, o, &err)) return QString();
            return pdfText(fileBytes(path));
        };
        // What printing always showed, until a choice is made: a handout's title and page number; a notes page's number alone.
        QVERIFY(textOf(deck::PrintLayout::Handout4).contains(QStringLiteral("Quarterly results")));
        QVERIFY(!textOf(deck::PrintLayout::NotesPages).contains(QStringLiteral("Quarterly results")));
        deck::PrintHeaderFooter hf;
        hf.header = true;
        hf.headerText = QStringLiteral("HEADWORDS");
        hf.footer = true;
        hf.footerText = QStringLiteral("FOOTWORDS");
        hf.date = true;
        hf.dateText = QStringLiteral("DATEWORDS");
        hf.pageNumber = false;
        for (deck::PrintedPages pages : {deck::PrintedPages::Notes, deck::PrintedPages::Handouts}) deck::setPrintHeaderFooter(*d, pages, hf);
        for (deck::PrintLayout layout : {deck::PrintLayout::NotesPages, deck::PrintLayout::Handout6}) {
            const QString text = textOf(layout);
            for (const char *word : {"HEADWORDS", "FOOTWORDS", "DATEWORDS"}) QVERIFY2(text.contains(QLatin1String(word)), word);
            QVERIFY(!text.contains(QStringLiteral("Quarterly results")));
        }
        // Each off.
        hf.header = false;
        hf.footer = false;
        hf.date = false;
        for (deck::PrintedPages pages : {deck::PrintedPages::Notes, deck::PrintedPages::Handouts}) deck::setPrintHeaderFooter(*d, pages, hf);
        for (deck::PrintLayout layout : {deck::PrintLayout::NotesPages, deck::PrintLayout::Handout6}) {
            const QString text = textOf(layout);
            for (const char *word : {"HEADWORDS", "FOOTWORDS", "DATEWORDS"}) QVERIFY2(!text.contains(QLatin1String(word)), word);
        }
        // The notes page and the handout each have their own.
        deck::PrintHeaderFooter notesOnly;
        notesOnly.header = true;
        notesOnly.headerText = QStringLiteral("NOTESHEAD");
        deck::setPrintHeaderFooter(*d, deck::PrintedPages::Notes, notesOnly);
        QVERIFY(textOf(deck::PrintLayout::NotesPages).contains(QStringLiteral("NOTESHEAD")));
        QVERIFY(!textOf(deck::PrintLayout::Handout6).contains(QStringLiteral("NOTESHEAD")));
    }

    void thePrintPageOffersColorModesScaleToFitAndHeaderAndFooter()
    {
        const jptest::SettingGuard remembered(QStringLiteral("output/color"), 0, 0);
        DeckWindow w;
        w.resize(1300, 850);
        w.show();
        w.showFileScreen(QStringLiteral("print"));
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        auto *screen = w.findChild<FileScreen *>();
        QVERIFY(screen);
        auto *color = screen->findChild<QComboBox *>(QStringLiteral("printColor"));
        QVERIFY(color);
        QStringList choices;
        for (int i = 0; i < color->count(); ++i) choices << color->itemText(i);
        QCOMPARE(choices, (QStringList{QStringLiteral("Color"), QStringLiteral("Grayscale"), QStringLiteral("Pure Black and White")}));
        auto *fit = screen->findChild<QCheckBox *>(QStringLiteral("printScaleToFit"));
        QVERIFY(fit);
        QVERIFY(fit->isChecked());
        auto *headerFooter = screen->findChild<QPushButton *>(QStringLiteral("printHeaderFooter"));
        QVERIFY(headerFooter);
        // (the header and footer are for notes pages and handouts)
        auto *what = screen->findChild<QComboBox *>(QStringLiteral("printWhat"));
        what->setCurrentIndex(what->findData(int(deck::PrintLayout::Slides)));
        QVERIFY(!headerFooter->isEnabled());
        what->setCurrentIndex(what->findData(int(deck::PrintLayout::NotesPages)));
        QVERIFY(headerFooter->isEnabled());
        shotOf(&w, QStringLiteral("deck-print-page"));
        w.editor()->markSaved();
    }

    void theHeaderAndFooterBoxSetsTheNotesPagesAndHandouts()
    {
        DeckWindow w;
        w.show();
        withDialog([&w] { w.showPrintHeaderFooter(); }, QStringLiteral("printHeaderFooterDialog"), [](QDialog *dlg) {
            dlg->findChild<QCheckBox *>(QStringLiteral("hfHeader"))->setChecked(true);
            dlg->findChild<QLineEdit *>(QStringLiteral("hfHeaderText"))->setText(QStringLiteral("My header"));
            dlg->findChild<QCheckBox *>(QStringLiteral("hfFooter"))->setChecked(true);
            dlg->findChild<QLineEdit *>(QStringLiteral("hfFooterText"))->setText(QStringLiteral("My footer"));
            dlg->findChild<QCheckBox *>(QStringLiteral("hfPageNumber"))->setChecked(false);
            dlg->findChild<QCheckBox *>(QStringLiteral("hfDate"))->setChecked(true);
            shotOf(dlg, QStringLiteral("deck-print-header-footer"));
        });
        for (deck::PrintedPages pages : {deck::PrintedPages::Notes, deck::PrintedPages::Handouts}) {
            const deck::PrintHeaderFooter hf = deck::printHeaderFooter(*w.editor()->doc(), pages);
            QVERIFY(hf.header && hf.footer && hf.date && !hf.pageNumber);
            QCOMPARE(hf.headerText, QStringLiteral("My header"));
            QCOMPARE(hf.footerText, QStringLiteral("My footer"));
        }
        QVERIFY(w.editor()->isModified());
    }


    // ---- Notes Master and Handout Master (November 15, 2026)

    // The layout printing always had is what a presentation with no master gets: the slide at the top as large as the upper half lets it be, the notes under it, and the header, date,
    // footer, and number in the page's four corners.
    void aPresentationWithNoNotesMasterHasTheLayoutPrintingAlwaysHad()
    {
        auto d = outputDeck();
        QVERIFY(!deck::hasNotesMaster(*d) && !deck::hasHandoutMaster(*d));
        const QSizeF paper = deck::localePaper(), slide = d->pageSize();
        const deck::NotesMaster m = deck::notesMasterOf(*d, paper);
        QVERIFY(!m.custom);
        const deck::NotesPageGeometry g = deck::notesPageGeometry(m, paper, slide);
        QVERIFY(qAbs(g.slideImage.left() - 54) < 0.01 && qAbs(g.slideImage.top() - 54) < 0.01);
        QVERIFY2(qAbs(g.slideImage.width() / g.slideImage.height() - slide.width() / slide.height()) < 1e-6, "the slide keeps its shape");
        QVERIFY(g.slideImage.bottom() <= paper.height() / 2);
        QVERIFY(qAbs(g.body.top() - (g.slideImage.bottom() + 24)) < 0.01 && qAbs(g.body.left() - 54) < 0.01 && qAbs(g.body.width() - (paper.width() - 108)) < 0.01);
        QVERIFY(g.header.left() == 36 && g.date.right() > paper.width() - 37 && g.footer.bottom() > paper.height() - 37 && g.number.right() > paper.width() - 37);
        QVERIFY(g.header.right() <= g.date.left() + 0.01 && g.footer.right() <= g.number.left() + 0.01);
    }

    // The places of the notes page are the master's: moving the slide's picture moves the slide on the printed page.
    void theNotesMasterMovesTheSlideOnThePrintedPage()
    {
        auto d = outputDeck();
        deck::setSlideBackground(*d->pages[0], QColor(0xC0, 0x20, 0x20));
        const QSizeF paper = deck::localePaper();
        deck::NotesMaster m = deck::notesMasterOf(*d, paper);
        m.custom = true;
        m.slideImage = QRectF(0.5, 0.55, 0.4, 0.2);   // lower right, 245 by 158 points
        deck::setNotesMaster(*d, m);
        LayoutCache cache;
        deck::OutputOptions o;
        o.layout = deck::PrintLayout::NotesPages;
        const QImage img = outputImage(*d, &cache, o, 0, paper, 2);
        const QVector<QRect> slide = colorRegions(img, QColor(0xC0, 0x20, 0x20));
        QCOMPARE(int(slide.size()), 1);
        const QRectF want = deck::notesPageGeometry(deck::notesMasterOf(*d, paper), paper, d->pageSize()).slideImage;
        QVERIFY2(qAbs(slide[0].left() / 2.0 - want.left()) < 3 && qAbs(slide[0].top() / 2.0 - want.top()) < 3 && qAbs(slide[0].width() / 2.0 - want.width()) < 4, "the slide is where the master puts it");
        QVERIFY(slide[0].left() / 2.0 > paper.width() / 2 - 1 && slide[0].top() / 2.0 > paper.height() / 2 - 1);
        // The background is the master's too.
        m.background = QColor(0xDD, 0xEE, 0xFF);
        deck::setNotesMaster(*d, m);
        const QImage tinted = outputImage(*d, &cache, o, 0, paper, 2);
        QVERIFY(colorsNear(QColor(tinted.pixel(6, 6)), QColor(0xDD, 0xEE, 0xFF)));
        // It goes to the PDF as well.
        QTemporaryDir dir;
        deck::OutputOptions pdf = o;
        QString err;
        QVERIFY2(deck::exportPdf(*d, dir.filePath(QStringLiteral("n.pdf")), pdf, &err), qPrintable(err));
    }

    // A handout of N slides to a page has N places that tile its area, in reading order, each slide's room inside its cell; three leave lines beside each.
    void handoutGeometryHoldsNSlidesToAPage()
    {
        const QSizeF paper = deck::localePaper();
        const deck::HandoutMaster m = deck::defaultHandoutMaster(paper);
        const struct {
            int n, columns, rows;
        } cases[] = {{1, 1, 1}, {2, 1, 2}, {3, 1, 3}, {4, 2, 2}, {6, 2, 3}, {9, 3, 3}};
        for (const auto &c : cases) {
            const deck::HandoutGeometry g = deck::handoutGeometry(m, paper, c.n, true);
            QCOMPARE(int(g.slots.size()), c.n);
            QSet<int> xs, ys;
            double area = 0;
            for (int k = 0; k < c.n; ++k) {
                const deck::HandoutSlot &slot = g.slots[k];
                QVERIFY2(g.area.adjusted(-0.01, -0.01, 0.01, 0.01).contains(slot.cell), qPrintable(QStringLiteral("%1 per page, cell %2").arg(c.n).arg(k)));
                QVERIFY(slot.cell.adjusted(-0.01, -0.01, 0.01, 0.01).contains(slot.room));
                QCOMPARE(slot.lines, c.n == 3);
                xs << int(slot.cell.left() * 10);
                ys << int(slot.cell.top() * 10);
                area += slot.cell.width() * slot.cell.height();
                for (int j = 0; j < k; ++j) QVERIFY(!g.slots[j].cell.adjusted(0.1, 0.1, -0.1, -0.1).intersects(slot.cell.adjusted(0.1, 0.1, -0.1, -0.1)));
            }
            QCOMPARE(int(xs.size()), c.columns);
            QCOMPARE(int(ys.size()), c.rows);
            QVERIFY2(qAbs(area - g.area.width() * g.area.height()) < 1, "the cells fill the area");
            // Across, then down puts the second slide beside the first; down, then across puts it under.
            if (c.n >= 4) {
                QVERIFY(g.slots[1].cell.left() > g.slots[0].cell.left() && qAbs(g.slots[1].cell.top() - g.slots[0].cell.top()) < 0.01);
                const deck::HandoutGeometry down = deck::handoutGeometry(m, paper, c.n, false);
                QVERIFY(down.slots[1].cell.top() > down.slots[0].cell.top() && qAbs(down.slots[1].cell.left() - down.slots[0].cell.left()) < 0.01);
            }
        }
        // The page lies across when its master says so.
        deck::HandoutMaster wide = m;
        wide.landscape = true;
        QCOMPARE(deck::handoutPaper(wide, paper), paper.transposed());
        QCOMPARE(deck::handoutPaper(m, paper), paper);
        QCOMPARE(deck::handoutPaper(wide, paper.transposed()), paper.transposed());
    }

    // The handout master's places, background, and orientation reach the printed handout and its PDF.
    void theHandoutMasterChangesThePrintedHandout()
    {
        auto d = showDeck(7);
        for (auto &page : d->pages) deck::setSlideBackground(*page, QColor(0x1F, 0x5F, 0xBF));
        deck::OutputOptions o;
        o.layout = deck::PrintLayout::Handout6;
        const QSizeF paper = deck::localePaper();
        QCOMPARE(deck::outputPageSize(*d, o), paper);
        deck::HandoutMaster m = deck::handoutMasterOf(*d, paper);
        m.custom = true;
        m.area = QRectF(0.1, 0.1, 0.5, 0.5);
        m.background = QColor(0xFF, 0xEE, 0xDD);
        deck::setHandoutMaster(*d, m);
        LayoutCache cache;
        const QImage img = outputImage(*d, &cache, o, 0, paper, 2);
        QVERIFY(colorsNear(QColor(img.pixel(4, 4)), QColor(0xFF, 0xEE, 0xDD)));
        const QVector<QRect> six = colorRegions(img, QColor(0x1F, 0x5F, 0xBF), 100);
        QCOMPARE(int(six.size()), 6);
        for (const QRect &slide : six) QVERIFY2(slide.right() < 2 * (0.6 * paper.width() + 1) && slide.bottom() < 2 * (0.6 * paper.height() + 1), "the slides fill the master's area");
        // Lying across: the page, and the PDF's pages, are wider than tall.
        m.landscape = true;
        m.area = deck::defaultHandoutMaster(paper.transposed()).area;
        deck::setHandoutMaster(*d, m);
        QCOMPARE(deck::outputPageSize(*d, o), paper.transposed());
        QTemporaryDir dir;
        QString err;
        const QString path = dir.filePath(QStringLiteral("h.pdf"));
        QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
        const QVector<QSizeF> sizes = pdfPageSizes(fileBytes(path));
        QCOMPARE(int(sizes.size()), 2);
        QVERIFY(sizes[0].width() > sizes[0].height());
        // How many slides a page holds is the master's, kept apart from its places.
        deck::setHandoutPerPage(*d, 4);
        QCOMPARE(deck::handoutMasterOf(*d, paper).perPage, 4);
        deck::setHandoutPerPage(*d, 5);   // (not one of the counts)
        QCOMPARE(deck::handoutMasterOf(*d, paper).perPage, 6);
    }

    // Places read from a file are held inside the page and to a least size, and a count that is not one of the six is not taken.
    void placesAreHeldInsideThePage()
    {
        const QRectF fallback(0.1, 0.1, 0.2, 0.2);
        const double nan = std::numeric_limits<double>::quiet_NaN(), inf = std::numeric_limits<double>::infinity();
        QCOMPARE(deck::sanitizedPlace(QRectF(nan, 0, 0.5, 0.5), fallback), fallback);
        QCOMPARE(deck::sanitizedPlace(QRectF(0, 0, inf, 0.5), fallback), fallback);
        const QRectF big = deck::sanitizedPlace(QRectF(-5, -5, 1e9, 1e9), fallback);
        QVERIFY(big.left() >= 0 && big.top() >= 0 && big.right() <= 1.0000001 && big.bottom() <= 1.0000001);
        const QRectF tiny = deck::sanitizedPlace(QRectF(0.5, 0.5, 0, -3), fallback);
        QVERIFY(tiny.width() >= 0.0049 && tiny.height() >= 0.0049);
        QJsonObject o;
        o[QStringLiteral("header")] = QJsonArray{1e300, -1e300, 1e300, 5};
        o[QStringLiteral("body")] = QJsonArray{0.1, 0.2};   // the wrong length
        const deck::NotesMaster read = deck::notesMasterFromJson(o, deck::defaultNotesMaster(QSizeF(612, 792), QSizeF(960, 540)));
        QVERIFY(read.header.right() <= 1.0000001 && read.header.top() >= 0);
        QCOMPARE(read.body, deck::defaultNotesMaster(QSizeF(612, 792), QSizeF(960, 540)).body);
        QVERIFY(!deck::isHandoutCount(0) && !deck::isHandoutCount(5) && !deck::isHandoutCount(1000000) && deck::isHandoutCount(9));
    }

    // Slides as GIF, TIFF, BMP, and SVG pictures: one file to a slide, each a valid file of its type and size.
    void everyPictureFormatWritesValidFiles()
    {
        auto d = outputDeck();   // five slides, the third hidden
        deck::setSlideBackground(*d->pages[0], QColor(0xC0, 0x20, 0x20));
        LayoutCache cache;
        struct Case {
            deck::SlidePictureFormat format;
            const char *name, *extension;
        };
        for (const Case &c : {Case{deck::SlidePictureFormat::Png, "png", "png"}, Case{deck::SlidePictureFormat::Gif, "gif", "gif"}, Case{deck::SlidePictureFormat::Tiff, "tiff", "tif"},
                              Case{deck::SlidePictureFormat::Bmp, "bmp", "bmp"}}) {
            // (Qt writes TIFF only where its TIFF plugin is installed; the type is not offered where it is not)
            if (c.format == deck::SlidePictureFormat::Tiff && !jp::pictureFormatAvailable(jp::PictureFormat::Tiff)) continue;
            QTemporaryDir dir;
            QVERIFY(dir.isValid());
            deck::SlidePicturesOptions o;
            o.folder = dir.path();
            o.format = c.format;
            o.width = 320;
            const QStringList names = deck::slidePictureNames(*d, o);
            QCOMPARE(int(names.size()), 4);
            QVERIFY2(names[0].endsWith(QLatin1Char('.') + QLatin1String(c.extension)), c.name);
            const deck::SlidePicturesResult r = deck::exportSlidePictures(*d, &cache, o);
            QVERIFY2(r.error.isEmpty(), qPrintable(r.error));
            QCOMPARE(r.written, names);
            for (const QString &name : names) {
                QImageReader reader(dir.filePath(name));
                QVERIFY2(reader.canRead(), c.name);
                QCOMPARE(QString::fromLatin1(reader.format()).toLower(), QString::fromLatin1(c.name));
                QCOMPARE(reader.size(), QSize(320, 180));
            }
            // The first slide's background is the color asked for (a GIF's colors are cut down to 256: a few levels off is the same).
            const QImage first(dir.filePath(names[0]));
            QVERIFY2(!first.isNull(), c.name);
            QVERIFY2(colorsNear(QColor(first.pixel(4, 4)), QColor(0xC0, 0x20, 0x20), 10), c.name);
        }
        // The old way of asking for a JPEG still works.
        QTemporaryDir dir;
        deck::SlidePicturesOptions jpeg;
        jpeg.folder = dir.path();
        jpeg.jpeg = true;
        jpeg.width = 200;
        QVERIFY(deck::exportSlidePictures(*d, &cache, jpeg).error.isEmpty());
        QCOMPARE(QString::fromLatin1(QImageReader(dir.filePath(deck::slidePictureName(0, true))).format()).toLower(), QStringLiteral("jpeg"));
    }

    // An SVG slide is a drawing, not a picture of pixels: the slide's size, its letters as shapes, and no raster of the whole slide.
    void svgSlidesAreVector()
    {
        auto d = outputDeck();
        deck::setSlideBackground(*d->pages[0], QColor(0xC0, 0x20, 0x20));
        LayoutCache cache;
        QTemporaryDir dir;
        deck::SlidePicturesOptions o;
        o.folder = dir.path();
        o.format = deck::SlidePictureFormat::Svg;
        const deck::SlidePicturesResult r = deck::exportSlidePictures(*d, &cache, o);
        QVERIFY2(r.error.isEmpty(), qPrintable(r.error));
        QCOMPARE(int(r.written.size()), 4);
        QCOMPARE(r.written[0], deck::slidePictureName(0, deck::SlidePictureFormat::Svg));
        QFile f(dir.filePath(r.written[0]));
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray svg = f.readAll();
        QXmlStreamReader xml(svg);
        QVERIFY(xml.readNextStartElement());
        QCOMPARE(xml.name().toString(), QStringLiteral("svg"));
        const QString viewBox = xml.attributes().value(QStringLiteral("viewBox")).toString();
        QVERIFY2(viewBox.split(QLatin1Char(' '), Qt::SkipEmptyParts).size() == 4, qPrintable(viewBox));
        QVERIFY2(qAbs(viewBox.split(QLatin1Char(' '), Qt::SkipEmptyParts)[2].toDouble() - 960) < 0.5, qPrintable(viewBox));
        int paths = 0, images = 0;
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement()) {
                paths += xml.name() == QLatin1String("path");
                images += xml.name() == QLatin1String("image");
            }
        }
        QVERIFY2(!xml.hasError(), qPrintable(xml.errorString()));
        QVERIFY2(paths > 0, "the letters are shapes");
        QVERIFY2(images == 0, "no raster of the slide");
        QVERIFY(svg.contains("C02020") || svg.contains("c02020") || svg.contains("rgb(192,32,32)"));
        // The same slide, as a string: what the command for the current slide writes.
        QCOMPARE(deck::slideSvg(*d, &cache, 0), svg);
        QVERIFY(deck::slideSvg(*d, &cache, 99).isEmpty());
    }

    // The GIF writer: any picture comes back with its size and, where it has few colors, its colors; it refuses what GIF cannot hold.
    void gifEncodingKeepsSizeAndColors()
    {
        QImage img(300, 200, QImage::Format_ARGB32);
        img.fill(QColor(10, 120, 30));
        {
            QPainter p(&img);
            p.fillRect(QRect(50, 40, 100, 80), QColor(240, 200, 20));
            p.setPen(Qt::black);
            for (int i = 0; i < 300; i += 3) p.drawLine(i, 0, 300 - i, 199);   // many pixels of changing codes: the table fills and clears
        }
        const QByteArray gif = jp::encodeGif(img);
        QVERIFY(gif.startsWith("GIF89a") && gif.endsWith(char(0x3B)));
        QImage back;
        QVERIFY(back.loadFromData(gif, "GIF"));
        QCOMPARE(back.size(), img.size());
        QVERIFY(colorsNear(QColor(back.pixel(100, 80)), QColor(240, 200, 20), 10));
        QVERIFY(colorsNear(QColor(back.pixel(280, 100)), QColor(10, 120, 30), 10));
        // A noisy picture, which fills the 4096-code table over and over, is still read back whole.
        QImage noise(400, 300, QImage::Format_RGB32);
        quint32 seed = 12345;
        for (int y = 0; y < noise.height(); ++y)
            for (int x = 0; x < noise.width(); ++x) {
                seed = seed * 1664525u + 1013904223u;
                noise.setPixel(x, y, qRgb(int(seed >> 24) & 0xE0, int(seed >> 16) & 0xE0, int(seed >> 8) & 0xC0));
            }
        QImage noisy;
        QVERIFY(noisy.loadFromData(jp::encodeGif(noise), "GIF"));
        QCOMPARE(noisy.size(), noise.size());
        // One pixel, and nothing.
        QImage one(1, 1, QImage::Format_RGB32);
        one.fill(Qt::blue);
        QImage oneBack;
        QVERIFY(oneBack.loadFromData(jp::encodeGif(one), "GIF"));
        QVERIFY(colorsNear(QColor(oneBack.pixel(0, 0)), Qt::blue, 10));
        QVERIFY(jp::encodeGif(QImage()).isEmpty());
        QVERIFY(jp::encodeGif(QImage(70000, 1, QImage::Format_Mono)).isEmpty());
    }

    // The current slide as a picture, in each type, by the Export Slide as Picture command: the ending of the name chooses the type.
    void exportingTheCurrentSlideChoosesTheTypeByTheName()
    {
        DeckWindow w;
        QTemporaryDir dir;
        for (const char *ending : {"gif", "tif", "bmp", "svg", "png"}) {
            const QString path = dir.filePath(QStringLiteral("slide.") + QLatin1String(ending));
            w.setSavePathChooser([&](const QString &, const QString &) { return path; });
            QVERIFY2(w.exportPicture(), ending);
            QVERIFY2(QFileInfo(path).size() > 100, ending);
            if (QLatin1String(ending) == QLatin1String("svg")) {
                QVERIFY(fileBytes(path).contains("<svg"));
            } else {
                QImageReader reader(path);
                QVERIFY2(reader.canRead() && reader.size() == QSize(1920, 1080), ending);
            }
        }
    }

    // The picture types this computer can write: TIFF is offered only where Qt has its plugin, the rest always.
    void tiffIsOfferedOnlyWhereItCanBeWritten()
    {
        const bool plugin = QImageWriter::supportedImageFormats().contains("tiff");
        QCOMPARE(jp::pictureFormatAvailable(jp::PictureFormat::Tiff), plugin);
        for (jp::PictureFormat f : {jp::PictureFormat::Png, jp::PictureFormat::Jpeg, jp::PictureFormat::Gif, jp::PictureFormat::Bmp}) QVERIFY(jp::pictureFormatAvailable(f));
        if (!plugin) QSKIP("this Qt has no TIFF writer: the type is not offered, and the rest is checked by everyPictureFormatWritesValidFiles");
        QByteArray tiff;
        QBuffer buf(&tiff);
        buf.open(QIODevice::WriteOnly);
        QImage img(8, 8, QImage::Format_RGB32);
        img.fill(Qt::red);
        QVERIFY(jp::writePicture(&buf, img, jp::PictureFormat::Tiff));
        QVERIFY(tiff.startsWith("II") || tiff.startsWith("MM"));
    }

    // The SVG of a JeffDeck slide says JeffDeck made it; JeffPub's own text is the default of the shared writer.
    void svgSlidesNameTheAppThatMadeThem()
    {
        auto d = outputDeck();
        LayoutCache cache;
        const QByteArray svg = deck::slideSvg(*d, &cache, 0);
        QVERIFY(svg.contains("Made with JeffDeck"));
        QVERIFY(!svg.contains("JeffPub"));
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        QVERIFY(jp::pageSvg(ctx, 0, QStringLiteral("t")).contains("Made with JeffPub"));
        QVERIFY(jp::pageSvg(ctx, 0, QStringLiteral("t"), false, QStringLiteral("Made with Other")).contains("Made with Other"));
    }

    // An SVG holds every picture at its full size, so a slide with more pixels of pictures than the limit is refused, with the reason, before anything is written.
    void anSvgOfTooManyPicturePixelsIsRefused()
    {
        auto d = outputDeck();
        ImageData big;
        big.format = QStringLiteral("png");
        big.pixelSize = QSize(20000, 20000);   // 400 million pixels, never decoded
        d->images.insert(QStringLiteral("hostile"), big);
        addPicture(*d->pages[0], QStringLiteral("hostile"), QRectF(0, 0, 100, 100));
        QVERIFY(deck::slidePicturePixels(*d, 0) > deck::kMostSlidePicturePixels);
        QCOMPARE(deck::slidePicturePixels(*d, 1), qint64(0));
        LayoutCache cache;
        QString why;
        QVERIFY(deck::slideSvg(*d, &cache, 0, &why).isEmpty());
        QVERIFY2(why.contains(QLatin1String("slide 1")) && why.contains(QLatin1String("PNG")), qPrintable(why));
        QVERIFY(!deck::slideSvg(*d, &cache, 1).isEmpty());
        QTemporaryDir dir;
        deck::SlidePicturesOptions o;
        o.folder = dir.path();
        o.format = deck::SlidePictureFormat::Svg;
        const deck::SlidePicturesResult r = deck::exportSlidePictures(*d, &cache, o);
        QCOMPARE(r.error, why);
        QVERIFY(!QFileInfo::exists(dir.filePath(deck::slidePictureName(0, deck::SlidePictureFormat::Svg))));
    }

    // A GIF of a gradient has its error spread (Floyd-Steinberg), so over a small block the colors average to the picture's, where cutting each pixel to a palette color would band.
    void gifDitheringKeepsGradientsSmooth()
    {
        QImage img(400, 100, QImage::Format_RGB32);
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) {
                const int v = x * 255 / 399;
                img.setPixel(x, y, qRgb(v, std::min(255, v + y / 6), std::min(255, v + y / 12)));
            }
        QImage back;
        QVERIFY(back.loadFromData(jp::encodeGif(img), "GIF"));
        QCOMPARE(back.size(), img.size());
        double worst = 0;
        for (int by = 0; by + 8 <= img.height(); by += 8)
            for (int bx = 0; bx + 8 <= img.width(); bx += 8) {
                double a[3] = {0, 0, 0}, b[3] = {0, 0, 0};
                for (int y = by; y < by + 8; ++y)
                    for (int x = bx; x < bx + 8; ++x) {
                        const QRgb p = img.pixel(x, y), q = back.pixel(x, y);
                        a[0] += qRed(p); a[1] += qGreen(p); a[2] += qBlue(p);
                        b[0] += qRed(q); b[1] += qGreen(q); b[2] += qBlue(q);
                    }
                for (int k = 0; k < 3; ++k) worst = std::max(worst, std::abs(a[k] - b[k]) / 64);
            }
        QVERIFY2(worst < 3.5, qPrintable(QString::number(worst)));
        // A picture with see-through parts is put on white, from any format of pixels.
        for (QImage::Format f : {QImage::Format_ARGB32, QImage::Format_ARGB32_Premultiplied, QImage::Format_RGBA8888}) {
            QImage clear(20, 20, QImage::Format_ARGB32);
            clear.fill(Qt::transparent);
            QImage conv = clear.convertToFormat(f);
            QImage g;
            QVERIFY(g.loadFromData(jp::encodeGif(conv), "GIF"));
            QVERIFY(colorsNear(QColor(g.pixel(5, 5)), Qt::white, 10));
        }
    }
};

JP_TEST_CLASS(DeckOutputTests)
#include "tst_deck_output.moc"
