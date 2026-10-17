// JeffPub's tests of SVG, ebook, and other output, barcodes, and metafiles.
#include "pubhelpers.h"
#include "../timing.h"

class PubExportTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // Save as E-book can make a fixed-layout EPUB: each page as designed (an
    // SVG drawing) at the page's size, its words underneath in reading order,
    // the contents from Heading 1-3 at the pages they're on.
    void fixedLayoutEbook()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        ed->insertPages(0, 1, false, false);
        auto heading = std::make_shared<jp::TextItem>();
        heading->rect = QRectF(72, 72, 400, 60);
        heading->storyId = ed->doc()->createStory();
        QTextCursor c(ed->doc()->storyDoc(heading->storyId));
        QTextBlockFormat hf;
        hf.setProperty(jp::tp::StyleName, QStringLiteral("Heading 1"));
        c.setBlockFormat(hf);
        c.insertText(QStringLiteral("Chapter One"));
        c.insertBlock(QTextBlockFormat());
        c.insertText(QStringLiteral("The story begins."));
        ed->doc()->pages[0]->items.push_back(heading);
        auto second = std::make_shared<jp::TextItem>();
        second->rect = QRectF(72, 72, 400, 60);
        second->storyId = ed->doc()->createStory();
        QTextCursor(ed->doc()->storyDoc(second->storyId)).insertText(QStringLiteral("And it ends."));
        ed->doc()->pages[1]->items.push_back(second);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("book.epub"));
        QString err;
        QVERIFY2(w.exportFixedEpubTo(path, QStringLiteral("A Book"), QStringLiteral("Pat"), &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QMap<QString, QByteArray> zip;
        QVERIFY(jp::readZip(f.readAll(), zip, &err));
        const QString opf = QString::fromUtf8(zip.value(QStringLiteral("OEBPS/content.opf")));
        QVERIFY(opf.contains(QStringLiteral("<meta property=\"rendition:layout\">pre-paginated</meta>")));
        QVERIFY(opf.contains(QStringLiteral("<itemref idref=\"p1\"/>")) && opf.contains(QStringLiteral("<itemref idref=\"p2\"/>")));
        const QString p1 = QString::fromUtf8(zip.value(QStringLiteral("OEBPS/page1.xhtml")));
        QVERIFY(p1.contains(QStringLiteral("content=\"width=816, height=1056\"")));   // a letter page in CSS pixels
        QVERIFY(p1.contains(QStringLiteral("<p>Chapter One</p>")) && p1.contains(QStringLiteral("<p>The story begins.</p>")));
        QVERIFY(QString::fromUtf8(zip.value(QStringLiteral("OEBPS/page2.xhtml"))).contains(QStringLiteral("<p>And it ends.</p>")));
        QVERIFY(zip.value(QStringLiteral("OEBPS/pages/page1.svg")).startsWith("<?xml") || zip.value(QStringLiteral("OEBPS/pages/page1.svg")).contains("<svg"));
        const QString nav = QString::fromUtf8(zip.value(QStringLiteral("OEBPS/nav.xhtml")));
        QVERIFY2(nav.contains(QStringLiteral("<a href=\"page1.xhtml\">Chapter One</a>")), qPrintable(nav));
        QVERIFY(nav.contains(QStringLiteral("epub:type=\"page-list\"")));
        QCOMPARE(zip.value(QStringLiteral("mimetype")), QByteArray("application/epub+zip"));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) QFile::copy(path, qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/fixed.epub"));
    }

    // What is wrong with an EPUB's files: names that lead nowhere in it (the
    // package's items, and every link, picture, and style sheet of each page),
    // and a printf-style doubled percent sign left in a style.
    static QStringList epubProblems(const QMap<QString, QByteArray> &zip)
    {
        QStringList problems;
        const QRegularExpression ref(QStringLiteral("(?:href|src)=\"([^\"]*)\""));
        const QRegularExpression styleBlock(QStringLiteral("<style[^>]*>(.*?)</style>"), QRegularExpression::DotMatchesEverythingOption);
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) {
            const QString name = it.key();
            const bool markup = name.endsWith(QLatin1String(".xhtml")) || name.endsWith(QLatin1String(".opf"));
            if (name.endsWith(QLatin1String(".css")) && it.value().contains("%%")) problems << name + QStringLiteral(": %% in a style");
            if (!markup) continue;
            const QString text = QString::fromUtf8(it.value());
            const QString dir = name.contains(QLatin1Char('/')) ? name.left(name.lastIndexOf(QLatin1Char('/')) + 1) : QString();
            for (auto m = ref.globalMatch(text); m.hasNext();) {
                const QString target = m.next().captured(1).section(QLatin1Char('#'), 0, 0);
                if (!target.isEmpty() && !target.contains(QLatin1String("://")) && !zip.contains(QDir::cleanPath(dir + target))) problems << name + QStringLiteral(" -> ") + target;
            }
            for (auto m = styleBlock.globalMatch(text); m.hasNext();)
                if (m.next().captured(1).contains(QLatin1String("%%"))) problems << name + QStringLiteral(": %% in a style");
        }
        return problems;
    }

    // The fixed-layout e-book is whole: the contents page's style sheet is in
    // the file and listed, no style carries a doubled percent sign, and a
    // title with %-and-digit in it comes through as typed.
    void fixedLayoutEbookIsConsistent()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        ed->insertPages(0, 1, false, false);
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(72, 72, 400, 60);
        box->storyId = ed->doc()->createStory(QStringLiteral("Growth 100%3 of %4 plans"));
        ed->doc()->pages[0]->items.push_back(box);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("book.epub"));
        QString err;
        const QString title = QStringLiteral("Growth 100%3 of %4 plans");
        QVERIFY2(w.exportFixedEpubTo(path, title, QStringLiteral("Pat %1"), &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QMap<QString, QByteArray> zip;
        QVERIFY(jp::readZip(f.readAll(), zip, &err));
        QVERIFY2(zip.contains(QStringLiteral("OEBPS/nav.xhtml")) && zip.contains(QStringLiteral("OEBPS/page1.xhtml")), qPrintable(QStringList(zip.keys()).join(' ')));
        QCOMPARE(epubProblems(zip), QStringList());
        QVERIFY(QString::fromUtf8(zip.value(QStringLiteral("OEBPS/nav.xhtml"))).contains(QStringLiteral("href=\"style.css\"")));
        QVERIFY(zip.contains(QStringLiteral("OEBPS/style.css")));
        QVERIFY(QString::fromUtf8(zip.value(QStringLiteral("OEBPS/content.opf"))).contains(QStringLiteral("href=\"style.css\"")));
        QVERIFY2(QString::fromUtf8(zip.value(QStringLiteral("OEBPS/page1.xhtml"))).contains(QStringLiteral("<title>Growth 100%3 of %4 plans, page 1</title>")),
                 qPrintable(QString::fromUtf8(zip.value(QStringLiteral("OEBPS/page1.xhtml")))));
        QVERIFY(QString::fromUtf8(zip.value(QStringLiteral("OEBPS/page2.xhtml"))).contains(QStringLiteral("<title>Growth 100%3 of %4 plans, page 2</title>")));
        QVERIFY(QString::fromUtf8(zip.value(QStringLiteral("OEBPS/page1.xhtml"))).contains(QStringLiteral("clip-path: inset(50%)")));
        QVERIFY(QString::fromUtf8(zip.value(QStringLiteral("OEBPS/content.opf"))).contains(QStringLiteral("<dc:creator>Pat %1</dc:creator>")));
    }

    // The same in the reflowable e-book: a picture's description with
    // %-and-digit in it is written as typed, and every name resolves.
    void epubTextWithPercentSignsIsWrittenAsTyped()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(72, 72, 400, 60);
        box->storyId = ed->doc()->createStory(QStringLiteral("Growth 100%3 of %4 plans"));
        ed->doc()->pages[0]->items.push_back(box);
        QImage img(40, 30, QImage::Format_RGB32);
        img.fill(Qt::darkGreen);
        QByteArray png;
        QBuffer pb(&png);
        pb.open(QIODevice::WriteOnly);
        img.save(&pb, "PNG");
        auto pic = std::make_shared<jp::PictureItem>();
        pic->imageId = ed->doc()->addImage(png, QStringLiteral("png"));
        pic->rect = QRectF(72, 400, 200, 100);
        pic->imgRect = QRectF(0, 0, 200, 100);
        pic->altText = QStringLiteral("Half of it, 50%3 sure");
        ed->doc()->pages[0]->items.push_back(pic);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("book.epub"));
        QString err;
        QVERIFY2(w.exportEpubTo(path, QStringLiteral("Growth 100%3 of %4 plans"), QString(), true, &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QMap<QString, QByteArray> zip;
        QVERIFY(jp::readZip(f.readAll(), zip, &err));
        QCOMPARE(epubProblems(zip), QStringList());
        const QString chapter = QString::fromUtf8(zip.value(QStringLiteral("OEBPS/chapter1.xhtml")));
        QVERIFY2(chapter.contains(QStringLiteral("alt=\"Half of it, 50%3 sure\"")), qPrintable(chapter));
        QVERIFY(chapter.contains(QStringLiteral("Growth 100%3 of %4 plans")));
        QVERIFY(QString::fromUtf8(zip.value(QStringLiteral("OEBPS/content.opf"))).contains(QStringLiteral("<dc:title>Growth 100%3 of %4 plans</dc:title>")));
    }

    // A publication's name goes into an email's headers: a line break must
    // not start a header of its own, and accented names are encoded.
    void emailHeaderNames()
    {
        QCOMPARE(jp::emlHeaderText(QStringLiteral("Spring Flyer")), QStringLiteral("Spring Flyer"));
        const QString forged = jp::emlHeaderText(QStringLiteral("Flyer\r\nBcc: someone@example.com"));
        QVERIFY(!forged.contains(QLatin1Char('\n')) && !forged.contains(QLatin1Char('\r')));
        QVERIFY(!jp::emlHeaderText(QStringLiteral("say \"hi\"")).contains(QLatin1Char('"')));
        const QString accented = jp::emlHeaderText(QStringLiteral("Café menu"));
        QVERIFY(accented.startsWith(QLatin1String("=?UTF-8?B?")));
        QCOMPARE(QString::fromUtf8(QByteArray::fromBase64(accented.mid(10).chopped(2).toLatin1())), QStringLiteral("Café menu"));
    }

    // Vector pictures inside .pub files come from the file, so their
    // records may lie: a header shorter than it claims, a text length that is
    // negative, a character count that wraps a 32-bit size check. Each must
    // play without reading outside the picture (run under AddressSanitizer
    // to see a stray read; a wrapped count crashed outright).
    void metafileHostileRecords()
    {
        auto le16 = [](QByteArray &b, int v) { b.append(char(v & 0xFF)); b.append(char((v >> 8) & 0xFF)); };
        auto le32 = [](QByteArray &b, quint32 v) { for (int i = 0; i < 4; ++i) b.append(char((v >> (8 * i)) & 0xFF)); };
        auto play = [](const QByteArray &data) {
            jp::Metafile m;
            if (!m.load(data)) return;
            QImage img(64, 64, QImage::Format_ARGB32);
            img.fill(Qt::white);
            QPainter p(&img);
            m.play(&p, QRectF(0, 0, 64, 64));
        };
        // 1. An EMF cut off after its signature: 44 bytes, the header needs 88.
        {
            QByteArray emf;
            le32(emf, 1);
            le32(emf, 88);
            while (emf.size() < 40) le32(emf, 0);
            le32(emf, 0x464D4520u);
            QCOMPARE(emf.size(), 44);
            play(emf);
        }
        // 2. An EMF text record whose character count wraps "count * 2".
        {
            QByteArray emf;
            le32(emf, 1);
            le32(emf, 88);
            for (int v : {0, 0, 100, 100, 0, 0, 2540, 2540}) le32(emf, quint32(v));   // bounds, frame
            le32(emf, 0x464D4520u);
            le32(emf, 0x10000);
            le32(emf, 88 + 84);
            le32(emf, 2);
            le16(emf, 1);
            le16(emf, 0);
            for (int i = 0; i < 3; ++i) le32(emf, 0);
            for (int v : {1024, 768, 320, 240}) le32(emf, quint32(v));   // device, millimeters
            QCOMPARE(emf.size(), 88);
            le32(emf, 84);   // EMR_EXTTEXTOUTW
            le32(emf, 84);
            for (int i = 0; i < 4; ++i) le32(emf, 0);   // bounds
            le32(emf, 1);
            le32(emf, 0);
            le32(emf, 0);   // mode, scales
            le32(emf, 10);
            le32(emf, 10);   // reference point
            le32(emf, 0x80000001u);   // characters: twice this wraps to 2
            le32(emf, 76);            // the string, inside the record
            le32(emf, 0);
            for (int i = 0; i < 4; ++i) le32(emf, 0);   // clip
            le32(emf, 0);                                // no spacing array
            le32(emf, 0x00410041u);
            le32(emf, 0x00410041u);
            QCOMPARE(emf.size(), 88 + 84);
            play(emf);
        }
        // 3. A WMF text record with a negative length (which once moved the
        // reading position before the start of the picture).
        {
            QByteArray wmf;
            le32(wmf, 0x9AC6CDD7u);
            le16(wmf, 0);
            for (int v : {0, 0, 1000, 1000}) le16(wmf, v);
            le16(wmf, 1440);
            le32(wmf, 0);
            le16(wmf, 0);
            le16(wmf, 1);
            le16(wmf, 9);
            le16(wmf, 0x300);
            le32(wmf, 0);
            le16(wmf, 0);
            le32(wmf, 0);
            le16(wmf, 0);
            le32(wmf, 8);       // record of 8 words
            le16(wmf, 0x0521);  // TEXTOUT
            le16(wmf, -32768);  // length
            for (int i = 0; i < 9; ++i) wmf.append(char('A'));
            wmf.append(char(0));
            le32(wmf, 3);
            le16(wmf, 0);       // end of file
            play(wmf);
        }
    }

    // Web page export: links are clickable areas over the page picture.
    void webPageLinks()
    {
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto shape = std::make_shared<jp::ShapeItem>();
        shape->rect = QRectF(100, 100, 50, 40);
        shape->fill = jp::Fill::solid(jp::ColorRef::rgb(Qt::red));
        shape->hyperlink = QStringLiteral("https://example.org/shape");
        doc->pages[0]->items.push_back(shape);
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 300, 300, 60);
        t->storyId = doc->createStory(QStringLiteral("Visit "));
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            c.movePosition(QTextCursor::End);
            QTextCharFormat f;
            f.setAnchor(true);
            f.setAnchorHref(QStringLiteral("https://example.org/text"));
            c.insertText(QStringLiteral("our site"), f);
        }
        doc->pages[0]->items.push_back(t);
        w.editor()->setDocument(std::move(doc));
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("page.html"));
        QVERIFY(w.exportHtmlTo(path));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QString html = QString::fromUtf8(f.readAll());
        QVERIFY(html.contains(QStringLiteral("usemap=\"#p1\"")));
        QVERIFY(html.contains(QStringLiteral("coords=\"200,200,300,280\" href=\"https://example.org/shape\"")));
        QVERIFY(html.contains(QStringLiteral("href=\"https://example.org/text\"")));
    }

    // "Always create backup copy" keeps the file as it was before saving.
    void saveBackupCopy()
    {
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("letter.jpub"));
        QVERIFY(w.saveTo(path));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray first = f.readAll();
        f.close();
        jp::Settings::get().setValue("save/backup", true);
        w.editor()->doc()->props.title = QStringLiteral("changed");
        QVERIFY(w.saveTo(path));
        jp::Settings::get().setValue("save/backup", false);
        QFile b(dir.filePath(QStringLiteral("Backup of letter.jpub")));
        QVERIFY(b.open(QIODevice::ReadOnly));
        QCOMPARE(b.readAll(), first);
    }

    // The SVG reader: path data (implicit lines, relative moves, reflected
    // controls, run-together arc flags), shapes, transforms, <use>, style
    // sheets, gradients and currentColor; and every bundled icon draws the
    // same as Qt's own SVG renderer draws it.
    void svgReader()
    {
        using namespace jp;
        QPainterPath p = svg::pathData(QStringLiteral("M10 10h5v5z m10 0 5 0 0 5"));
        QCOMPARE(p.elementCount(), 7);
        QCOMPARE(QPointF(p.elementAt(4)), QPointF(20, 10));      // a relative move after a close starts at the subpath's start
        QCOMPARE(QPointF(p.elementAt(6)), QPointF(25, 15));      // pairs after a move are lines
        p = svg::pathData(QStringLiteral("M0 0a10 10 0 1010 0")); // flags "1" "0" run into "10"
        QCOMPARE(p.currentPosition(), QPointF(10, 0));
        QVERIFY(p.boundingRect().height() > 18);                  // the large arc goes the long way round
        p = svg::pathData(QStringLiteral("M0 0C0 10 10 10 10 0S20 -10 20 0"));
        QCOMPARE(QPointF(p.elementAt(4)), QPointF(10, -10));      // S reflects the last control point
        p = svg::pathData(QStringLiteral("M1.5.5L-2-3e1"));
        QCOMPARE(QPointF(p.elementAt(0)), QPointF(1.5, 0.5));
        QCOMPARE(QPointF(p.elementAt(1)), QPointF(-2, -30));

        const QByteArray doc =
            "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"2in\" height=\"1in\" viewBox=\"0 0 200 100\">"
            "<style>.a { fill: #ff0000; stroke: none } #b { fill: rgb(0, 0, 255) }</style>"
            "<defs><linearGradient id=\"g\"><stop offset=\"0\" stop-color=\"#00ff00\"/><stop offset=\"1\" stop-color=\"#000\"/></linearGradient>"
            "<rect id=\"sq\" width=\"10\" height=\"10\"/></defs>"
            "<rect class=\"a\" x=\"10\" y=\"10\" width=\"20\" height=\"20\"/>"
            "<g transform=\"translate(100 0) scale(2)\" stroke=\"currentColor\" stroke-width=\"3\" fill=\"none\">"
            "<circle id=\"b\" cx=\"5\" cy=\"5\" r=\"5\"/>"
            "<line x1=\"0\" y1=\"20\" x2=\"10\" y2=\"20\" stroke-linecap=\"round\"/>"
            "</g>"
            "<use xlink:href=\"#sq\" x=\"50\" y=\"50\" fill=\"url(#g)\"/>"
            "<text x=\"0\" y=\"90\">words</text>"
            "<rect width=\"5\" height=\"5\" display=\"none\"/>"
            "</svg>";
        const svg::Drawing d = svg::read(doc, QColor(10, 20, 30));
        QVERIFY(d.isValid());
        QCOMPARE(d.viewBox, QRectF(0, 0, 200, 100));
        QCOMPARE(d.size, QSizeF(144, 72));
        QVERIFY(d.skipped);                                       // the text
        QCOMPARE(d.elements.size(), 4);
        QCOMPARE(d.elements[0].fill, QColor(255, 0, 0));
        QVERIFY(!d.elements[0].stroke.isValid());
        QCOMPARE(d.elements[0].path.boundingRect(), QRectF(10, 10, 20, 20));
        QCOMPARE(d.elements[1].fill, QColor(0, 0, 255));          // the id rule beats the group's fill="none"
        QCOMPARE(d.elements[1].stroke, QColor(10, 20, 30));
        QCOMPARE(d.elements[1].strokeWidth, 6.0);                 // scaled with the group
        QCOMPARE(d.elements[1].path.boundingRect(), QRectF(100, 0, 20, 20));
        QVERIFY(!d.elements[2].fill.isValid());                   // a line has no inside
        QCOMPARE(d.elements[2].cap, Qt::RoundCap);
        QCOMPARE(d.elements[3].fill, QColor(0, 255, 0));          // a gradient's first color
        QCOMPARE(d.elements[3].path.boundingRect(), QRectF(50, 50, 10, 10));

        // A drawing that repeats itself through <use>, ten times over at
        // each of nine levels, stops at a bounded number of parts.
        QByteArray bomb = "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" viewBox=\"0 0 10 10\">"
                          "<defs><g id=\"g0\"><rect width=\"1\" height=\"1\"/></g>";
        for (int level = 1; level < 10; ++level) {
            bomb += "<g id=\"g" + QByteArray::number(level) + "\">";
            for (int i = 0; i < 10; ++i) bomb += "<use xlink:href=\"#g" + QByteArray::number(level - 1) + "\"/>";
            bomb += "</g>";
        }
        bomb += "</defs><use xlink:href=\"#g9\"/></svg>";
        svg::Drawing many;
        const qint64 bombMs = bestNanoseconds([&] { many = svg::read(bomb); }, 5, [&] { many = svg::Drawing(); }) / 1000000;
        QVERIFY(bombMs < 5000);
        QVERIFY(many.elements.size() <= 100000);
        QVERIFY(many.skipped);
        // A box too small to scale from gives no shapes, not endless ones.
        svg::Drawing tiny;
        tiny.viewBox = QRectF(0, 0, 1e-300, 1e-300);
        tiny.elements << svg::Element{svg::pathData(QStringLiteral("M0 0L1e-300 1e-300")), QColor(Qt::black)};
        QVERIFY(!svg::shapes(tiny, QRectF(0, 0, 72, 72), true));

        // Every icon, drawn by this reader and by Qt's renderer at 48 pixels.
        int icons = 0, worst = 0;
        QString worstName;
        QDirIterator it(QStringLiteral(":/icons"), {QStringLiteral("*.svg")});
        while (it.hasNext()) {
            const QString path = it.next();
            QFile f(path);
            QVERIFY(f.open(QIODevice::ReadOnly));
            const QByteArray bytes = f.readAll();
            if (!bytes.contains("lucide")) continue;
            QImage qt(48, 48, QImage::Format_ARGB32_Premultiplied), ours(qt.size(), qt.format());
            qt.fill(Qt::transparent);
            ours.fill(Qt::transparent);
            {
                QPainter pq(&qt);
                QSvgRenderer(bytes).render(&pq);
            }
            const svg::Drawing icon = svg::read(bytes);
            QVERIFY2(icon.isValid() && !icon.elements.isEmpty(), qPrintable(path));
            {
                QPainter po(&ours);
                po.setRenderHint(QPainter::Antialiasing);
                po.scale(48 / icon.viewBox.width(), 48 / icon.viewBox.height());
                po.translate(-icon.viewBox.topLeft());
                for (const svg::Element &e : icon.elements) {
                    QPen pen(e.stroke.isValid() ? QBrush(e.stroke) : QBrush(Qt::NoBrush), e.strokeWidth, Qt::SolidLine, e.cap, e.join);
                    if (!e.dashes.isEmpty()) {
                        QVector<qreal> pattern;
                        for (double x : e.dashes) pattern << x / e.strokeWidth;
                        pen.setDashPattern(pattern);
                        pen.setDashOffset(e.dashOffset / e.strokeWidth);
                    }
                    po.setPen(e.stroke.isValid() ? pen : QPen(Qt::NoPen));
                    po.setBrush(e.fill.isValid() ? QBrush(e.fill) : QBrush(Qt::NoBrush));
                    po.drawPath(e.path);
                }
            }
            int differ = 0;
            for (int y = 0; y < 48; ++y)
                for (int x = 0; x < 48; ++x)
                    if (std::abs(qAlpha(qt.pixel(x, y)) - qAlpha(ours.pixel(x, y))) > 64) ++differ;
            if (differ > worst) { worst = differ; worstName = path; }
            ++icons;
        }
        QVERIFY(icons > 1500);
        QVERIFY2(worst <= 4, qPrintable(QStringLiteral("%1: %2 pixels differ").arg(worstName).arg(worst)));
    }

    // An SVG picture prints and exports to PDF as vectors, and Convert to
    // Shapes turns it into artwork shapes in its place: flipped, turned and
    // faded as the picture was, text left out (and said so), one undo step.
    void svgPictureToShapes()
    {
        using namespace jp;
        const QByteArray art =
            "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 20 10\">"
            "<rect width=\"10\" height=\"10\" fill=\"#ff0000\"/>"
            "<circle cx=\"15\" cy=\"5\" r=\"4\" fill=\"none\" stroke=\"#0000ff\" stroke-width=\"1\"/></svg>";
        auto doc = Document::blank(QSizeF(612, 792));
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = doc->addImage(art, QStringLiteral("svg"));
        pic->rect = QRectF(100, 100, 200, 100);
        pic->imgRect = QRectF(0, 0, 200, 100);
        pic->flipH = true;
        pic->transparency = 0.5;

        // Straight conversion: the red half lands on the right when flipped.
        bool partial = true;
        ItemPtr made = svg::pictureShapes(art, *pic, &partial);
        QVERIFY(made && made->type() == ItemType::Group);
        QVERIFY(!partial);
        auto *g = static_cast<GroupItem *>(made.get());
        QCOMPARE(g->children.size(), size_t(2));
        auto *red = static_cast<ShapeItem *>(g->children[0].get());
        QVERIFY(red->isArt());
        QCOMPARE(red->rect, QRectF(200, 100, 100, 100));
        QCOMPARE(red->fill.color, ColorRef::rgb(QColor(255, 0, 0)));
        QCOMPARE(red->fill.transparency, 0.5);
        auto *ring = static_cast<ShapeItem *>(g->children[1].get());
        QCOMPARE(ring->stroke.width, 10.0);                       // 1 unit at 10 points a unit
        QVERIFY(ring->rect.right() < 200);
        QVERIFY(svg::pictureShapes(QByteArray(art).replace("</svg>", "<text>hi</text></svg>"), *pic, &partial));
        QVERIFY(partial);
        pic->rotation = 90;
        made = svg::pictureShapes(art, *pic, &partial);
        // A quarter turn about the picture's center takes the red square
        // (flipped to 50 points right of it) to 50 points below it.
        const Item *turned = static_cast<GroupItem *>(made.get())->children[0].get();
        QCOMPARE(turned->rotation, 90.0);
        QVERIFY(QLineF(turned->rect.center(), QPointF(200, 200)).length() < 1e-9);
        pic->rotation = 0;
        pic->flipH = false;
        pic->transparency = 0;
        doc->pages[0]->items.push_back(pic);

        MainWindow w;
        w.editor()->setDocument(std::move(doc));
        Editor *ed = w.editor();
        // Print and PDF draw it as lines and areas, not as a picture.
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("svg.pdf"));
        MainWindow::PdfSettings ps;
        ps.preset = MainWindow::PdfSettings::HighQuality;
        QVERIFY(w.exportPdfTo(path, ps));
        QtPdf parsed;
        QVERIFY(parsed.load(path));
        for (const auto &o : parsed.objects) QVERIFY(!QtPdf::dictOf(o.body).contains("/Subtype /Image"));

        ed->select(pic->id);
        QTRY_VERIFY(w.act(QStringLiteral("pic.toShapes"))->isEnabled());
        w.act(QStringLiteral("pic.toShapes"))->trigger();
        QCOMPARE(ed->doc()->pages[0]->items.size(), size_t(1));
        QCOMPARE(ed->doc()->pages[0]->items[0]->type(), ItemType::Group);
        QCOMPARE(ed->selectedItems().size(), 1);
        ed->undo();
        QCOMPARE(ed->doc()->pages[0]->items[0]->type(), ItemType::Picture);
        ed->clearSelection();
        QTRY_VERIFY(!w.act(QStringLiteral("pic.toShapes"))->isEnabled());
    }

    // Clipping inside an SVG picture (clip-path, which Qt's SVG renderer
    // leaves out) shows on screen, in print and PDF, and in Save as Picture:
    // the big square is cut to the circle, and what is drawn before and
    // after it keeps its place in the stacking order.
    void svgPictureClips()
    {
        using namespace jp;
        const QByteArray plain =
            "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 50\">"
            "<defs><clipPath id=\"c\"><circle cx=\"50\" cy=\"25\" r=\"20\"/></clipPath></defs>"
            "<rect x=\"60\" y=\"20\" width=\"20\" height=\"10\" fill=\"#ffff00\"/>"
            "<rect width=\"100\" height=\"50\" fill=\"#ff0000\" clip-path=\"url(#c)\"/>"
            "<rect width=\"10\" height=\"10\" fill=\"#0000ff\"/></svg>";
        // The clip is in the box's own units, and moves with the group's transform.
        const QByteArray boxed =
            "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 50\">"
            "<defs><clipPath id=\"b\" clipPathUnits=\"objectBoundingBox\"><circle cx=\".5\" cy=\".5\" r=\".5\"/></clipPath></defs>"
            "<g transform=\"translate(50 0)\"><rect width=\"50\" height=\"50\" fill=\"#00ff00\" style=\"clip-path:url(#b)\"/></g></svg>";
        auto empty = [](QRgb c) { return qAlpha(c) < 10 || (qRed(c) > 245 && qGreen(c) > 245 && qBlue(c) > 245); };
        auto is = [](QRgb c, QColor want) {
            return std::abs(qRed(c) - want.red()) < 12 && std::abs(qGreen(c) - want.green()) < 12 && std::abs(qBlue(c) - want.blue()) < 12 && qAlpha(c) > 245;
        };
        // The picture is 200 by 100 points at (100, 100): two points to a unit.
        auto place = [](double x, double y) { return QPoint(int(100 + 2 * x), int(100 + 2 * y)); };
        auto check = [&](const QImage &page, bool first, const QString &how) {
            auto px = [&](double x, double y) { return page.pixel(place(x, y)); };
            const QByteArray msg = how.toUtf8();
            if (first) {
                QVERIFY2(is(px(50, 25), Qt::red), msg.constData());                 // inside the circle
                QVERIFY2(is(px(62, 37), Qt::red), msg.constData());
                QVERIFY2(is(px(65, 25), Qt::red), msg.constData());                 // the yellow under it is covered
                QVERIFY2(is(px(75, 25), Qt::yellow), msg.constData());              // and shows beyond the circle
                QVERIFY2(empty(px(30, 5)), msg.constData());                        // the square's corners are cut off
                QVERIFY2(empty(px(67, 42)), msg.constData());
                QVERIFY2(empty(px(95, 45)), msg.constData());
                QVERIFY2(is(px(5, 5), Qt::blue), msg.constData());                  // drawn after, outside the clip
            } else {
                QVERIFY2(is(px(75, 25), Qt::green), msg.constData());
                QVERIFY2(is(px(60, 40), Qt::green), msg.constData());
                QVERIFY2(empty(px(52, 2)), msg.constData());
                QVERIFY2(empty(px(98, 48)), msg.constData());
                QVERIFY2(empty(px(25, 25)), msg.constData());
            }
        };
        for (const bool first : {true, false}) {
            auto doc = Document::blank(QSizeF(612, 792));
            auto pic = std::make_shared<PictureItem>();
            pic->imageId = doc->addImage(first ? plain : boxed, QStringLiteral("svg"));
            pic->rect = QRectF(100, 100, 200, 100);
            pic->imgRect = QRectF(0, 0, 200, 100);
            doc->pages[0]->items.push_back(pic);
            MainWindow w;
            w.editor()->setDocument(std::move(doc));
            Editor *ed = w.editor();
            PaintContext ctx;
            ctx.doc = ed->doc();
            ctx.cache = &ed->cache();
            for (const bool output : {false, true}) {
                ctx.opt.output = output;
                check(Renderer::renderToImage(ctx, 0, 1.0, true), first, output ? QStringLiteral("print path") : QStringLiteral("screen path"));
                if (QTest::currentTestFailed()) return;
            }
            QTemporaryDir dir;
            const QString path = dir.filePath(QStringLiteral("clip.pdf"));
            QVERIFY(w.exportPdfTo(path, MainWindow::PdfSettings()));
            QFile f(path);
            QVERIFY(f.open(QIODevice::ReadOnly));
            const PdfDocument pdf(f.readAll());
            QVERIFY(pdf.isValid());
            check(pdf.render(0, QSize(612, 792)), first, QStringLiteral("PDF"));
            if (QTest::currentTestFailed()) return;
            // Save as Picture (SVG) writes the cut as a clip path: round,
            // not the picture's frame.
            ctx.opt.output = true;
            const QString svgPage = QString::fromUtf8(pageSvg(ctx, 0, QStringLiteral("clip")));
            const QRegularExpression clipPath(QStringLiteral("<clipPath[^>]*>\\s*<path[^>]* d=\"([^\"]*)\""));
            int cuts = 0;
            for (auto m = clipPath.globalMatch(svgPage); m.hasNext();)
                cuts += m.next().captured(1).count(QLatin1Char('L')) > 20;
            QVERIFY(cuts > 0);
        }
        // Convert to Shapes cuts the square to the circle too: 40 units, 80 points, across.
        PictureItem pic;
        pic.rect = QRectF(100, 100, 200, 100);
        pic.imgRect = QRectF(0, 0, 200, 100);
        bool partial = false;
        const ItemPtr made = svg::pictureShapes(plain, pic, &partial);
        QVERIFY(made && made->type() == ItemType::Group);
        auto *parts = static_cast<GroupItem *>(made.get());
        QCOMPARE(parts->children.size(), size_t(3));
        const QRectF cut = parts->children[1]->rect;
        QVERIFY2(std::abs(cut.width() - 80) < 1 && std::abs(cut.height() - 80) < 1, qPrintable(QStringLiteral("%1 x %2").arg(cut.width()).arg(cut.height())));
    }

    // Save as Picture > SVG: each page as a vector drawing with letters as
    // outlines (no <text> to be respaced by another font), pictures cropped
    // to their shape, and the whole looking as JeffPub draws it.
    void svgExport()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(72, 72, 300, 100);
        t->storyId = doc->createStory(QStringLiteral("Hello vector world, in letters drawn as outlines."));
        doc->pages[0]->items.push_back(t);
        auto sh = std::make_shared<ShapeItem>();
        sh->rect = QRectF(72, 300, 100, 60);
        doc->pages[0]->items.push_back(sh);
        doc->pages[0]->items.push_back(iconItem(QStringLiteral("heart"), QPointF(150, 500), 72, ColorRef::scheme(Accent1)));
        QImage photo(200, 100, QImage::Format_RGB32);
        photo.fill(Qt::darkGreen);
        QByteArray png;
        QBuffer pb(&png);
        pb.open(QIODevice::WriteOnly);
        photo.save(&pb, "PNG");
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = doc->addImage(png, QStringLiteral("png"));
        pic->rect = QRectF(300, 300, 100, 100);
        pic->imgRect = QRectF(-50, 0, 200, 100);
        pic->maskShape = QStringLiteral("ellipse");
        doc->pages[0]->items.push_back(pic);
        doc->pages.push_back(std::make_shared<Page>());

        MainWindow w;
        w.editor()->setDocument(std::move(doc));
        QTemporaryDir dir;
        QString err;
        QVERIFY2(w.exportSvgTo(dir.filePath(QStringLiteral("pages.svg")), &err), qPrintable(err));
        QVERIFY(QFile::exists(dir.filePath(QStringLiteral("pages-1.svg"))) && QFile::exists(dir.filePath(QStringLiteral("pages-2.svg"))));
        QFile f(dir.filePath(QStringLiteral("pages-1.svg")));
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray svg = f.readAll();
        QVERIFY(!svg.contains("<text"));
        QVERIFY(svg.contains("<image") && svg.contains("clip-path"));
        QVERIFY(svg.contains("viewBox=\"0 0 612 792\""));

        // The oval crop is a clip path around the picture's frame.
        const qsizetype clipAt = svg.indexOf("<clipPath");
        QVERIFY(clipAt > 0 && svg.indexOf("d=\"M400,350 C", clipAt) > clipAt);

        // Drawn by Qt's SVG renderer, it matches JeffPub's own drawing. (Qt's
        // renderer ignores clip paths, which browsers follow, so the cropped
        // picture's square is left out of the comparison.)
        PaintContext ctx;
        ctx.doc = w.editor()->doc();
        ctx.cache = &w.editor()->cache();
        ctx.opt.output = true;
        const QImage ours = Renderer::renderToImage(ctx, 0, 1.0).convertToFormat(QImage::Format_RGB32);
        QImage drawn(ours.size(), QImage::Format_RGB32);
        drawn.fill(Qt::white);
        {
            QPainter p(&drawn);
            QSvgRenderer(svg).render(&p, QRectF(0, 0, drawn.width(), drawn.height()));
        }
        int differ = 0, inked = 0;
        for (int y = 0; y < ours.height(); ++y)
            for (int x = 0; x < ours.width(); ++x) {
                if (QRectF(250, 300, 200, 100).contains(x, y)) continue;
                const QRgb a = ours.pixel(x, y), b = drawn.pixel(x, y);
                inked += a != qRgb(255, 255, 255);
                differ += std::max({std::abs(qRed(a) - qRed(b)), std::abs(qGreen(a) - qGreen(b)), std::abs(qBlue(a) - qBlue(b))}) > 96;
            }
        QVERIFY(inked > 3000);
        QVERIFY2(differ < inked / 50, qPrintable(QStringLiteral("%1 of %2 inked pixels differ").arg(differ).arg(inked)));

        QVERIFY(!w.exportSvgTo(dir.filePath(QStringLiteral("missing/folder/page.svg")), &err));
        QVERIFY(!err.isEmpty());
    }

    // File > Export > Save as E-book: a valid EPUB 3 (the type first and
    // uncompressed, every file it lists present, well-formed XHTML), the
    // stories in reading order split into chapters at Heading 1, formatting,
    // links, lists, pictures, tables, footnotes beside the text, endnotes at
    // the back, contents from the headings, and no page numbers.
    void epubExport()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        doc->props.author = QStringLiteral("Pat Writer");
        const QString footId = doc->createStory(QStringLiteral("A footnote's words."));
        const QString endId = doc->createStory(QStringLiteral("An endnote's words."));
        auto field = [](QTextCursor &c, const QString &code) {
            QTextCharFormat f;
            f.setProperty(tp::Field, code);
            c.insertText(QString(QChar(0xFFFC)), f);
            c.setCharFormat(QTextCharFormat());
        };
        auto heading = [](QTextCursor &c, const QString &style, const QString &text, bool first) {
            QTextBlockFormat bf;
            bf.setProperty(tp::StyleName, style);
            if (first) c.setBlockFormat(bf);
            else c.insertBlock(bf, QTextCharFormat());
            c.insertText(text);
        };
        auto t1 = std::make_shared<TextItem>();
        t1->rect = QRectF(72, 72, 468, 300);
        t1->storyId = doc->createStory();
        {
            QTextCursor c(doc->storyDoc(t1->storyId));
            heading(c, QStringLiteral("Heading 1"), QStringLiteral("Chapter One"), true);
            c.insertBlock(QTextBlockFormat(), QTextCharFormat());
            c.insertText(QStringLiteral("Plain, "));
            QTextCharFormat b;
            b.setFontWeight(QFont::Bold);
            c.insertText(QStringLiteral("bold"), b);
            c.insertText(QStringLiteral(", "), QTextCharFormat());
            QTextCharFormat i;
            i.setFontItalic(true);
            c.insertText(QStringLiteral("italic"), i);
            c.insertText(QStringLiteral(" & a "), QTextCharFormat());
            QTextCharFormat a;
            a.setAnchor(true);
            a.setAnchorHref(QStringLiteral("https://example.com/"));
            c.insertText(QStringLiteral("link"), a);
            c.insertText(QStringLiteral("."), QTextCharFormat());
            field(c, QStringLiteral("footnote:") + footId);
            c.insertBlock(QTextBlockFormat(), QTextCharFormat());
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListDecimal);
            c.createList(lf);
            c.insertText(QStringLiteral("First"));
            c.insertBlock();
            c.insertText(QStringLiteral("Second"));
            c.insertBlock(QTextBlockFormat(), QTextCharFormat());
            c.currentList() ? c.currentList()->remove(c.block()) : void();
        }
        doc->pages[0]->items.push_back(t1);
        QImage green(80, 40, QImage::Format_RGB32);
        green.fill(Qt::darkGreen);
        QByteArray png;
        QBuffer pb(&png);
        pb.open(QIODevice::WriteOnly);
        green.save(&pb, "PNG");
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = doc->addImage(png, QStringLiteral("png"));
        pic->rect = QRectF(72, 400, 200, 100);
        pic->imgRect = QRectF(0, 0, 200, 100);
        pic->altText = QStringLiteral("A green box");
        doc->pages[0]->items.push_back(pic);

        doc->pages.push_back(std::make_shared<Page>());
        auto t2 = std::make_shared<TextItem>();
        t2->rect = QRectF(72, 72, 468, 400);
        t2->storyId = doc->createStory();
        {
            QTextCursor c(doc->storyDoc(t2->storyId));
            QTextBlockFormat toc;
            toc.setProperty(tp::TocLevel, 1);
            c.setBlockFormat(toc);
            c.insertText(QStringLiteral("Contents line\t1"));
            heading(c, QStringLiteral("Heading 1"), QStringLiteral("Chapter Two"), false);
            heading(c, QStringLiteral("Heading 2"), QStringLiteral("Part A"), false);
            c.insertBlock(QTextBlockFormat(), QTextCharFormat());
            c.insertText(QStringLiteral("Text with an endnote"));
            field(c, QStringLiteral("endnote:") + endId);
            c.insertText(QStringLiteral(" on page "));
            field(c, QStringLiteral("page"));
            c.insertText(QStringLiteral("."));
        }
        doc->pages[1]->items.push_back(t2);

        MainWindow w;
        w.editor()->setDocument(std::move(doc));
        Editor *ed = w.editor();
        auto table = std::static_pointer_cast<TableItem>(ed->newTable(QRectF(72, 600, 300, 60), 2, 2));
        const QStringList cells = {QStringLiteral("A1"), QStringLiteral("B1"), QStringLiteral("A2"), QStringLiteral("B2")};
        for (int k = 0; k < 4; ++k) setStoryText(ed->doc()->storyDoc(table->cells[k].storyId), cells[k]);
        ed->doc()->pages[1]->items.push_back(table);

        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("book.epub"));
        QString err;
        QVERIFY2(w.exportEpubTo(path, QStringLiteral("My Book"), QString(), true, &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray epub = f.readAll();
        QCOMPARE(epub.mid(30, 28), QByteArray("mimetypeapplication/epub+zip"));   // first, stored
        QCOMPARE(int(epub[8]), 0);
        QMap<QString, QByteArray> z;
        QVERIFY2(readZip(epub, z, &err), qPrintable(err));
        QVERIFY(z.value(QStringLiteral("META-INF/container.xml")).contains("full-path=\"OEBPS/content.opf\""));

        // Every XML file is well formed.
        for (auto it = z.cbegin(); it != z.cend(); ++it) {
            if (!it.key().endsWith(QLatin1String(".xhtml")) && !it.key().endsWith(QLatin1String(".opf")) && !it.key().endsWith(QLatin1String(".xml"))) continue;
            QXmlStreamReader r(it.value());
            while (!r.atEnd()) r.readNext();
            QVERIFY2(!r.hasError(), qPrintable(it.key() + QStringLiteral(": ") + r.errorString()));
        }
        // The package lists only files that are there, and reads them in order.
        const QByteArray opf = z.value(QStringLiteral("OEBPS/content.opf"));
        QXmlStreamReader r(opf);
        QStringList ids, spine;
        while (!r.atEnd()) {
            r.readNext();
            if (!r.isStartElement()) continue;
            if (r.name() == QLatin1String("item")) {
                ids << r.attributes().value(QStringLiteral("id")).toString();
                QVERIFY2(z.contains(QStringLiteral("OEBPS/") + r.attributes().value(QStringLiteral("href")).toString()), qPrintable(r.attributes().value(QStringLiteral("href")).toString()));
            } else if (r.name() == QLatin1String("itemref")) {
                spine << r.attributes().value(QStringLiteral("idref")).toString();
            }
        }
        for (const QString &s : std::as_const(spine)) QVERIFY(ids.contains(s));
        QCOMPARE(spine, QStringList({QStringLiteral("cover"), QStringLiteral("c1"), QStringLiteral("c2"), QStringLiteral("notes")}));
        QVERIFY(opf.contains("properties=\"nav\"") && opf.contains("properties=\"cover-image\""));
        QVERIFY(opf.contains("<dc:title>My Book</dc:title>") && opf.contains("<dc:creator>Pat Writer</dc:creator>") && opf.contains("<dc:language>en-US</dc:language>"));

        const QString c1 = QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter1.xhtml")));
        QVERIFY(c1.contains(QStringLiteral("<h1 id=\"h1\">Chapter One</h1>")));
        QVERIFY(c1.contains(QStringLiteral("<strong>bold</strong>")) && c1.contains(QStringLiteral("<em>italic</em>")) && c1.contains(QStringLiteral(" &amp; a ")));
        QVERIFY(c1.contains(QStringLiteral("<a href=\"https://example.com/\">link</a>")));
        QVERIFY(c1.contains(QStringLiteral("<ol type=\"1\">\n<li>First</li>\n<li>Second</li>\n</ol>")));
        QVERIFY(c1.contains(QStringLiteral("href=\"#fn1\">1</a>")) && c1.contains(QStringLiteral("<aside epub:type=\"footnote\" id=\"fn1\">")));
        QVERIFY(c1.contains(QStringLiteral("A footnote's words.")));
        QVERIFY(c1.contains(QStringLiteral("<img src=\"images/pic1.png\" alt=\"A green box\"")));
        QVERIFY(c1.indexOf(QStringLiteral("Plain")) < c1.indexOf(QStringLiteral("<img")));   // the text above comes first
        const QString c2 = QString::fromUtf8(z.value(QStringLiteral("OEBPS/chapter2.xhtml")));
        QVERIFY(c2.contains(QStringLiteral(">Chapter Two</h1>")) && c2.contains(QStringLiteral(">Part A</h2>")));
        QVERIFY(!c2.contains(QStringLiteral("Contents line")));                                 // the book has its own contents
        QVERIFY(c2.contains(QStringLiteral("href=\"notes.xhtml#en1\"")));
        QVERIFY(c2.contains(QStringLiteral(" on page .</p>")));                                  // no page numbers
        QCOMPARE(c2.count(QStringLiteral("<td>")), 4);
        QVERIFY(c2.contains(QStringLiteral("<td><p>B2</p></td>")));
        const QString notes = QString::fromUtf8(z.value(QStringLiteral("OEBPS/notes.xhtml")));
        QVERIFY(notes.contains(QStringLiteral("An endnote's words.")) && notes.contains(QStringLiteral("chapter2.xhtml#enref1")));
        const QString nav = QString::fromUtf8(z.value(QStringLiteral("OEBPS/nav.xhtml")));
        QVERIFY(nav.contains(QStringLiteral("<a href=\"chapter1.xhtml#h1\">Chapter One</a>")));
        QVERIFY(nav.contains(QStringLiteral("Chapter Two</a><ol>\n<li><a href=\"chapter2.xhtml#h3\">Part A</a>")));   // nested under its chapter
        QVERIFY(!QImage::fromData(z.value(QStringLiteral("OEBPS/images/cover.jpg"))).isNull());
        QCOMPARE(z.value(QStringLiteral("OEBPS/images/pic1.png")), png);                        // a picture used as it is keeps its file

        // A cropped picture wider than the e-book's largest picture size
        // still comes out, scaled down.
        {
            auto big = Document::blank(QSizeF(3000, 2000));
            QImage photo(1000, 500, QImage::Format_RGB32);
            photo.fill(Qt::darkBlue);
            QByteArray jpg;
            QBuffer jb(&jpg);
            jb.open(QIODevice::WriteOnly);
            photo.save(&jb, "JPG");
            auto wide = std::make_shared<PictureItem>();
            wide->imageId = big->addImage(jpg, QStringLiteral("jpg"));
            wide->rect = QRectF(0, 0, 2800, 1400);
            wide->imgRect = QRectF(-100, 0, 3000, 1400);
            big->pages[0]->items.push_back(wide);
            const QString bigPath = dir.filePath(QStringLiteral("big.epub"));
            QVERIFY2(exportEpub(*big, bigPath, EpubOptions(), &err), qPrintable(err));
            QFile bf(bigPath);
            QVERIFY(bf.open(QIODevice::ReadOnly));
            QMap<QString, QByteArray> bz;
            QVERIFY(readZip(bf.readAll(), bz));
            const QImage out = QImage::fromData(bz.value(QStringLiteral("OEBPS/images/pic1.jpg")));
            QVERIFY(!out.isNull());
            QVERIFY(out.width() <= 1601 && out.width() >= 1000);
        }

        // A compressed entry that claims another size, or a huge one, is refused.
        ZipWriter zw;
        zw.add(QStringLiteral("a.txt"), QByteArray(5000, 'a'), true);
        const QByteArray good = zw.finish();
        QMap<QString, QByteArray> back;
        QVERIFY(readZip(good, back) && back.value(QStringLiteral("a.txt")) == QByteArray(5000, 'a'));
        const qsizetype dirAt = qFromLittleEndian<quint32>(good.constData() + good.size() - 22 + 16);
        for (const quint32 claimed : {4999u, 0x7FFFFFFFu}) {
            QByteArray bad = good;
            qToLittleEndian<quint32>(claimed, bad.data() + dirAt + 24);
            back.clear();
            QVERIFY(!readZip(bad, back));
        }
    }

    // docs/jpub-format.md stays true: its example opens as a publication,
    // and the property numbers it lists are the ones the code uses.
    void jpubSpecExample()
    {
        using namespace jp;
        QFile f(QStringLiteral(JP_TEST_DATA "/../../docs/jpub-format.md"));
        QVERIFY(f.open(QIODevice::ReadOnly));
        // Git on Windows checks text out with CR LF line ends.
        const QString spec = QString::fromUtf8(f.readAll()).replace(QLatin1String("\r\n"), QLatin1String("\n"));
        const qsizetype a = spec.indexOf(QStringLiteral("```json\n")), b = spec.indexOf(QStringLiteral("```"), a + 8);
        QVERIFY(a > 0 && b > a);
        ZipWriter z;
        z.add(QStringLiteral("mimetype"), "application/x-jeffpub");
        z.add(QStringLiteral("document.json"), spec.mid(a + 8, b - a - 8).toUtf8());
        QString err;
        auto doc = publicationFromBytes(z.finish(), &err);
        QVERIFY2(doc, qPrintable(err));
        QCOMPARE(doc->pages.size(), 1);
        QCOMPARE(doc->pageSize(), QSizeF(612, 792));
        QCOMPARE(doc->pages[0]->items.size(), size_t(1));
        auto *t = dynamic_cast<TextItem *>(doc->pages[0]->items[0].get());
        QVERIFY(t);
        QCOMPARE(t->rect, QRectF(72, 72, 468, 100));
        const QTextDocument *sd = doc->storyDoc(t->storyId);
        QVERIFY(sd);
        QCOMPARE(sd->toPlainText(), QStringLiteral("Hello, world"));
        QTextCursor c(const_cast<QTextDocument *>(sd));
        c.setPosition(9);
        QCOMPARE(c.charFormat().fontWeight(), 700);
        QCOMPARE(doc->colors.c[1], QColor(0x1F, 0x4E, 0x79));

        const int qt[] = {QTextFormat::BlockAlignment, QTextFormat::LayoutDirection, QTextFormat::BlockTopMargin, QTextFormat::BlockBottomMargin,
                          QTextFormat::BlockLeftMargin, QTextFormat::BlockRightMargin, QTextFormat::TextIndent, QTextFormat::TabPositions,
                          QTextFormat::LineHeight, QTextFormat::LineHeightType, QTextFormat::PageBreakPolicy, QTextFormat::FontCapitalization,
                          QTextFormat::FontLetterSpacing, QTextFormat::FontKerning, QTextFormat::FontFamilies, QTextFormat::FontPointSize,
                          QTextFormat::FontWeight, QTextFormat::FontItalic, QTextFormat::FontUnderline, QTextFormat::FontOverline,
                          QTextFormat::FontStrikeOut, QTextFormat::TextUnderlineStyle, QTextFormat::TextUnderlineColor,
                          QTextFormat::TextVerticalAlignment, QTextFormat::IsAnchor, QTextFormat::AnchorHref, QTextFormat::ForegroundBrush,
                          QTextFormat::BackgroundBrush, QTextFormat::TextOutline, QTextFormat::ListStyle, QTextFormat::ListIndent, QTextFormat::ListStart};
        for (int n : qt) QVERIFY2(spec.contains(QRegularExpression(QStringLiteral("\\| (\\d+, )*%1(, \\d+)* \\|").arg(n))), qPrintable(QString::number(n)));
        for (int n = tp::ColorRefP; n <= tp::Tracking; ++n) QVERIFY2(spec.contains(QString::number(n)), qPrintable(QString::number(n)));
        for (int n = tp::StyleName; n <= tp::TocLevel; ++n) QVERIFY2(spec.contains(QString::number(n)), qPrintable(QString::number(n)));
    }

    // Barcodes: ISBN-10 and -13 with their check digits, price add-ons, and
    // every symbology's bars (the patterns pinned here decoded correctly with
    // an independent reader, zbar: ISBN with price as 978030640615751995).
    void barcodeEncodings()
    {
        using namespace jp::barcode;
        QString err;
        QCOMPARE(isbn13(QStringLiteral("0-306-40615-2"), &err), QStringLiteral("9780306406157"));
        QCOMPARE(isbn13(QStringLiteral("978-0-306-40615-7"), &err), QStringLiteral("9780306406157"));
        QVERIFY(isbn13(QStringLiteral("978-0-306-40615-0"), &err).isEmpty());
        QVERIFY(err.contains(QLatin1String("should be 7")));
        QCOMPARE(isbn13(QStringLiteral("080442957X"), &err), QStringLiteral("9780804429573"));
        QCOMPARE(isbnCaption(QStringLiteral("978-0-306-40615-7")), QStringLiteral("ISBN 978-0-306-40615-7"));
        // Standard hyphens for the English-language groups (as printed on 88
        // ISBNs in Michigan Legal Publishing's PDFs, all reproduced).
        QCOMPARE(hyphenateIsbn(QStringLiteral("9781640021631")), QStringLiteral("978-1-64002-163-1"));
        QCOMPARE(hyphenateIsbn(QStringLiteral("9781942842187")), QStringLiteral("978-1-942842-18-7"));
        QCOMPARE(hyphenateIsbn(QStringLiteral("9780306406157")), QStringLiteral("978-0-306-40615-7"));
        QCOMPARE(hyphenateIsbn(QStringLiteral("9780199535569")), QStringLiteral("978-0-19-953556-9"));
        QCOMPARE(isbnCaption(QStringLiteral("9781640021631")), QStringLiteral("ISBN 978-1-64002-163-1"));
        // Other groups wait for the ISBN agency's table (downloaded by the
        // dialog). A few of its rules, in its format:
        QVERIFY(hyphenateIsbn(QStringLiteral("9783161484100")).isEmpty());
        auto rules = [](const char *tag, const char *prefix, const char *list) {
            QByteArray r;
            for (const QByteArray &rule : QByteArray(list).split(' '))
                r += "<Rule><Range>" + rule.split(':')[0] + "</Range><Length>" + rule.split(':')[1] + "</Length></Rule>";
            return "<" + QByteArray(tag) + "><Prefix>" + prefix + "</Prefix><Agency>x</Agency><Rules>" + r + "</Rules></" + tag + ">";
        };
        const QByteArray xml = "<?xml version='1.0' encoding='utf-8'?><ISBNRangeMessage><MessageDate>x</MessageDate><EAN.UCCPrefixes>"
            + rules("EAN.UCC", "978", "0000000-5999999:1 6000000-6499999:3 9990000-9999999:5")
            + rules("EAN.UCC", "979", "0000000-0999999:0 1000000-1599999:2 8000000-8999999:1") + "</EAN.UCCPrefixes><RegistrationGroups>"
            + rules("Group", "978-3", "0400000-1999999:2") + rules("Group", "979-8", "8850000-8999999:5")
            + rules("Group", "978-99937", "0000000-1999999:1") + rules("Group", "978-1", "5500000-6499999:5") + "</RegistrationGroups></ISBNRangeMessage>";
        QVERIFY(!loadIsbnRanges("<html>Not found</html>"));
        QVERIFY(loadIsbnRanges(xml));
        QCOMPARE(hyphenateIsbn(QStringLiteral("9783161484100")), QStringLiteral("978-3-16-148410-0"));    // Germany
        QCOMPARE(hyphenateIsbn(QStringLiteral("9798886450002")), QStringLiteral("979-8-88645-000-2"));    // the newer US prefix
        QCOMPARE(hyphenateIsbn(QStringLiteral("9789993701231")), QStringLiteral("978-99937-0-123-1"));   // a five-digit group
        QCOMPARE(hyphenateIsbn(QStringLiteral("9781640021631")), QStringLiteral("978-1-64002-163-1"));
        QVERIFY(hyphenateIsbn(QStringLiteral("9790000000001")).isEmpty());   // 979-0 isn't an ISBN range (music)
        QCOMPARE(isbnCaption(QStringLiteral("979-0-000000-00-1")), QStringLiteral("ISBN 979-0-000000-00-1"));
        QVERIFY(!loadIsbnRanges(QByteArray()));   // back to the built-in ranges
        QVERIFY(hyphenateIsbn(QStringLiteral("9783161484100")).isEmpty());
        QCOMPARE(isbnCaption(QStringLiteral("978-3-16-148410-0")), QStringLiteral("ISBN 978-3-16-148410-0"));
        QCOMPARE(priceAddOn(Currency::UsDollar, 19.95, &err), QStringLiteral("51995"));
        QCOMPARE(priceAddOn(Currency::CanadianDollar, 24.99, &err), QStringLiteral("62499"));
        QCOMPARE(priceAddOn(Currency::Pound, 7.5, &err), QStringLiteral("00750"));
        QCOMPARE(priceAddOn(Currency::None, 0, &err), QStringLiteral("90000"));
        QCOMPARE(eanCheckDigit(QStringLiteral("400638133393")), 1);

        auto modules = [](const Layout &l) {
            const double unit = 0.33 * 72 / 25.4;
            double x0 = 1e9, x1 = 0;
            for (const QRectF &r : l.bars) { x0 = std::min(x0, r.left()); x1 = std::max(x1, r.right()); }
            QString m(int(std::lround((x1 - x0) / unit)), QLatin1Char('0'));
            for (const QRectF &r : l.bars)
                for (int k = int(std::lround((r.left() - x0) / unit)); k < int(std::lround((r.right() - x0) / unit)); ++k) m[k] = QLatin1Char('1');
            return m;
        };
        auto bars = [&](Type t, const QString &data, const QString &addOn = QString()) {
            Options o;
            o.type = t;
            o.data = data;
            o.addOn = addOn;
            const Layout l = make(o);
            return l.error.isEmpty() ? modules(l) : l.error;
        };
        QCOMPARE(bars(Type::Ean13, QStringLiteral("4006381333931")),
                 QStringLiteral("10100011010100111010111101111010001001011001101010100001010000101000010111010010000101100110101"));
        QCOMPARE(bars(Type::Ean8, QStringLiteral("96385074")), QStringLiteral("1010001011010111101111010110111010101001110111001010001001011100101"));
        QCOMPARE(bars(Type::UpcA, QStringLiteral("036000291452")),
                 QStringLiteral("10100011010111101010111100011010001101000110101010110110011101001100110101110010011101101100101"));
        QCOMPARE(bars(Type::Isbn, QStringLiteral("0-306-40615-2"), QStringLiteral("51995")),
                 QStringLiteral("1010111011000100101001110111101010011101011110101010111001110010101000011001101001110100010010100000000010110110001010110011010001011010010111010110001"));
        QCOMPARE(bars(Type::Code128, QStringLiteral("AB12345CD7")),
                 QStringLiteral("1101001000010100011000100010110001001110011010111011110111011011101011101100010111101110100010001101011000100011101101110101110111101100011101011"));
        QCOMPARE(bars(Type::Code39, QStringLiteral("MLP")), QStringLiteral("1000101110111010111011101010001010111010100011101011101110100010100010111011101"));
        // What can't be drawn says why.
        QVERIFY(bars(Type::Ean13, QStringLiteral("4006381333932")).contains(QLatin1String("should be 1")));
        QVERIFY(bars(Type::Code39, QStringLiteral("a~b")).contains(QLatin1String("Code 39")));
    }

    // Insert > Barcode places one group: the white quiet zone and the bars
    // with their digits as a single vector outline.
    void barcodeInsert()
    {
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        jp::Settings::get().setValue(QStringLiteral("barcode/type"), 0);
        QTimer::singleShot(0, &w, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QVERIFY(d);
            d->findChild<QLineEdit *>()->setText(QStringLiteral("978-0-306-40615-7"));
            for (auto *r : d->findChildren<QRadioButton *>())
                if (r->text() == QLatin1String("Price:")) r->click();
            d->findChild<QDoubleSpinBox *>()->setValue(19.95);
            if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) d->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/barcode-dialog.png");
            for (auto *b : d->findChildren<QPushButton *>())
                if (b->text() == QLatin1String("Insert")) b->click();
        });
        jp::barcodeDialog(&w, w.editor());
        const auto &items = w.editor()->doc()->pages[0]->items;
        QCOMPARE(int(items.size()), 1);
        auto g = std::dynamic_pointer_cast<jp::GroupItem>(items[0]);
        QVERIFY(g);
        QCOMPARE(g->altText, QStringLiteral("Barcode: ISBN 978-0-306-40615-7"));
        QCOMPARE(int(g->children.size()), 2);
        auto bars = std::dynamic_pointer_cast<jp::ShapeItem>(g->children[1]);
        QVERIFY(bars && !bars->customPath.isEmpty());
        // Main symbol plus add-on: 167 modules (2.17 in) by about 1.2 in at 100%.
        QVERIFY2(std::abs(g->rect.width() - 167 * 0.33 * 72 / 25.4) < 0.01 && g->rect.height() > 80 && g->rect.height() < 100,
                 qPrintable(QStringLiteral("%1 x %2").arg(g->rect.width()).arg(g->rect.height())));
        QCOMPARE(g->barcode.value(QLatin1String("priceMode")).toInt(), 1);

        // With the barcode selected, Barcode edits it: a pasted ISBN gets its
        // hyphens, "no suggested price" replaces the price, and the new
        // barcode takes the old one's place (one undo step back).
        const QString id = g->id;
        g->moveBy(-100, 50);
        const QPointF where = g->rect.topLeft();
        w.editor()->select(id);
        QTimer::singleShot(0, &w, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QVERIFY(d);
            QCOMPARE(d->windowTitle(), QStringLiteral("Edit Barcode"));
            auto *isbn = d->findChild<QLineEdit *>();
            QCOMPARE(isbn->text(), QStringLiteral("978-0-306-40615-7"));
            isbn->setText(QStringLiteral("9781640021631"));
            QCOMPARE(isbn->text(), QStringLiteral("978-1-64002-163-1"));
            for (auto *r : d->findChildren<QRadioButton *>())
                if (r->text().startsWith(QLatin1String("No suggested price"))) r->click();
            for (auto *b : d->findChildren<QPushButton *>())
                if (b->text() == QLatin1String("Update")) b->click();
        });
        jp::barcodeDialog(&w, w.editor());
        QCOMPARE(int(items.size()), 1);
        auto g2 = std::dynamic_pointer_cast<jp::GroupItem>(items[0]);
        QVERIFY(g2 && g2 != g);
        QCOMPARE(g2->id, id);
        QCOMPARE(g2->rect.topLeft(), where);
        QCOMPARE(g2->altText, QStringLiteral("Barcode: ISBN 978-1-64002-163-1"));
        QCOMPARE(g2->barcode.value(QLatin1String("priceMode")).toInt(), 2);
        // Saved and opened again, it can still be edited.
        auto copy = jp::publicationFromBytes(jp::publicationBytes(*w.editor()->doc(), QImage()), nullptr);
        QVERIFY(copy);
        auto g3 = std::dynamic_pointer_cast<jp::GroupItem>(copy->pages[0]->items[0]);
        QVERIFY(g3);
        QCOMPARE(g3->barcode, g2->barcode);
        w.editor()->undoStack()->undo();
        QCOMPARE(std::dynamic_pointer_cast<jp::GroupItem>(w.editor()->doc()->pages[0]->items[0])->altText, QStringLiteral("Barcode: ISBN 978-0-306-40615-7"));
    }

    // A barcode is one object on the page: clicking it again and dragging
    // moves the bars and the white quiet zone together (a click used to
    // reach inside the group and move only the bars); a second barcode
    // doesn't land on the first; double-clicking opens it for editing.
    void barcodeMovesAsOne()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        QApplication::processEvents();
        jp::barcode::Options o;
        o.data = QStringLiteral("978-1-64002-163-1");
        const jp::barcode::Layout l = jp::barcode::make(o);
        const QJsonObject settings{{"type", 0}, {"data", o.data}, {"priceMode", 0}};
        // Two inserted the way the dialog does: the second offset from the first.
        auto insert = [&] {
            QTimer::singleShot(0, &w, [&] {
                auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                QVERIFY(d);
                d->findChild<QLineEdit *>()->setText(o.data);
                for (auto *r : d->findChildren<QRadioButton *>())
                    if (r->text() == QLatin1String("No add-on")) r->click();
                for (auto *b : d->findChildren<QPushButton *>())
                    if (b->text() == QLatin1String("Insert")) b->click();
            });
            w.editor()->select(QStringList());
            jp::barcodeDialog(&w, w.editor());
        };
        insert();
        insert();
        auto &items = w.editor()->doc()->pages[0]->items;
        QCOMPARE(int(items.size()), 2);
        QCOMPARE(items[1]->rect.topLeft() - items[0]->rect.topLeft(), QPointF(18, 18));
        // The second (selected) barcode: click its bars, drag an inch right.
        auto g = std::dynamic_pointer_cast<jp::GroupItem>(items[1]);
        const QRectF before = g->children[0]->rect, barsBefore = g->children[1]->rect;
        jp::Canvas *c = w.canvas();
        QWidget *vp = c->viewport();
        auto view = [&](QPointF page) { return c->pageToView(page).toPoint(); };
        QPointF onBar;
        for (const QRectF &r : l.bars)
            if (r.width() > 1.5) { onBar = g->rect.topLeft() + r.center(); break; }
        QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, view(onBar));
        QCOMPARE(w.editor()->selection(), QStringList{g->id});
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, view(onBar));
        QTest::mouseMove(vp, view(onBar + QPointF(36, 0)));
        QTest::mouseMove(vp, view(onBar + QPointF(72, 0)));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, view(onBar + QPointF(72, 0)));
        g = std::dynamic_pointer_cast<jp::GroupItem>(w.editor()->doc()->pages[0]->items[1]);
        QVERIFY2(std::abs(g->children[0]->rect.left() - before.left() - 72) < 1.5, qPrintable(QString::number(g->children[0]->rect.left() - before.left())));
        QCOMPARE(g->children[1]->rect.left() - barsBefore.left(), g->children[0]->rect.left() - before.left());
        // Double-click: the Edit Barcode dialog.
        QString title;
        QTimer::singleShot(0, &w, [&] {
            if (auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget())) {
                title = d->windowTitle();
                d->reject();
            }
        });
        QTest::mouseDClick(vp, Qt::LeftButton, Qt::NoModifier, view(onBar + QPointF(72, 0)));
        QCOMPARE(title, QStringLiteral("Edit Barcode"));
    }

    // Compound-file round trip: every stream, class id and flag survives a
    // read, write and read; libmspub imports the rewritten file the same way.
    void cfbRoundTrip_data()
    {
        QTest::addColumn<QString>("path");
        QDirIterator it(QStringLiteral(JP_TEST_DATA "/pub"), {"*.pub"}, QDir::Files);
        QStringList files;
        while (it.hasNext()) files << it.next();
        files.sort();
        for (const QString &f : files) QTest::newRow(qPrintable(QFileInfo(f).fileName())) << f;
    }
    void cfbRoundTrip()
    {
        QFETCH(QString, path);
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        const QByteArray original = file.readAll();
        jp::cfb::File a, b;
        QString err;
        QVERIFY2(jp::cfb::read(original, &a, &err), qPrintable(err));
        const QByteArray rewritten = jp::cfb::write(a);
        QVERIFY2(jp::cfb::read(rewritten, &b, &err), qPrintable(err));
        QCOMPARE(b.streamPaths(), a.streamPaths());
        for (const QString &sp : a.streamPaths()) QVERIFY2(b.stream(sp) == a.stream(sp), qPrintable(sp));
        QCOMPARE(b.entries.size(), a.entries.size());
        for (int i = 0; i < a.entries.size(); ++i) {
            QCOMPARE(b.entries[i].clsid, a.entries[i].clsid);
            QCOMPARE(b.entries[i].stateBits, a.entries[i].stateBits);
        }
        // Independent reader: libmspub sees the same publication.
        QTemporaryDir dir;
        const QString out = dir.filePath(QStringLiteral("rewritten.pub"));
        QFile o(out);
        QVERIFY(o.open(QIODevice::WriteOnly));
        o.write(rewritten);
        o.close();
        QString e1, e2;
        auto d1 = jp::importPublisherFile(path, &e1);
        auto d2 = jp::importPublisherFile(out, &e2);
        QCOMPARE(bool(d2), bool(d1));
        if (d1) {
            QCOMPARE(d2->pages.size(), d1->pages.size());
            // Item ids are generated afresh, so compare what is drawn.
            jp::LayoutCache c1, c2;
            jp::PaintContext p1, p2;
            p1.doc = d1.get(); p1.cache = &c1; p1.opt.output = true;
            p2.doc = d2.get(); p2.cache = &c2; p2.opt.output = true;
            for (int i = 0; i < d1->pages.size(); ++i)
                QVERIFY2(jp::Renderer::renderToImage(p1, i, 0.5) == jp::Renderer::renderToImage(p2, i, 0.5), qPrintable(QStringLiteral("page %1 differs").arg(i + 1)));
        }
    }

    // A compound file with a root and the given streams, named s0, s1, ...
    static jp::cfb::File cfbWith(const QVector<QByteArray> &streams)
    {
        jp::cfb::File f;
        jp::cfb::Entry root;
        root.type = jp::cfb::Entry::Root;
        root.name = QStringLiteral("Root Entry");
        f.entries << root;
        for (int i = 0; i < streams.size(); ++i) {
            jp::cfb::Entry e;
            e.name = QStringLiteral("s%1").arg(i);
            e.data = streams[i];
            f.entries[0].children << int(f.entries.size());
            f.entries << e;
        }
        return f;
    }

    // A damaged compound file can't make the reader loop, run out of memory,
    // or run out of stack: each chain is read once, the streams together stay
    // near the file's size, and storages stop 32 deep.
    void cfbDamageIsBounded()
    {
        auto put32 = [](QByteArray &b, qsizetype o, quint32 v) { qToLittleEndian(v, b.data() + o); };
        auto get32 = [](const QByteArray &b, qsizetype o) { return qFromLittleEndian<quint32>(b.constData() + o); };
        jp::cfb::File out;
        const qint64 ms = bestNanoseconds([&] {

            // A DIFAT sector that names itself as the next one, and a count of
            // four billion of them.
            {
                QByteArray b = jp::cfb::write(cfbWith({QByteArray(5000, 'a')}));
                const quint32 self = quint32(b.size() / 512 - 1);
                QByteArray extra(512, '\xFF');
                put32(extra, 508, self);
                b += extra;
                put32(b, 68, self);
                put32(b, 72, 0xFFFFFFFF);
                QVERIFY(jp::cfb::read(b, &out));
                QCOMPARE(out.stream(QStringLiteral("s0")), QByteArray(5000, 'a'));
            }
            // The one FAT sector listed 109 times, and a stream whose first sector
            // leads back to itself, claiming 4 GB.
            {
                QByteArray b = jp::cfb::write(cfbWith({QByteArray(5000, 'b')}));
                const quint32 fatSector = get32(b, 76);
                for (int i = 0; i < 109; ++i) put32(b, 76 + i * 4, fatSector);
                put32(b, 44, 109);
                const qsizetype entry = (qsizetype(get32(b, 48)) + 1) * 512 + 128;   // the stream, after the root
                const quint32 first = get32(b, entry + 116);
                put32(b, (qsizetype(fatSector) + 1) * 512 + qsizetype(first) * 4, first);
                put32(b, entry + 120, 0xFFFFFFFF);
                QVERIFY(jp::cfb::read(b, &out));
                QVERIFY(out.stream(QStringLiteral("s0")).size() <= b.size());
            }
            // Three hundred streams pointed at one long chain.
            {
                QVector<QByteArray> streams{QByteArray(100000, 'c')};
                for (int i = 0; i < 300; ++i) streams << QByteArray();
                QByteArray b = jp::cfb::write(cfbWith(streams));
                const qsizetype dir = (qsizetype(get32(b, 48)) + 1) * 512;
                const quint32 start = get32(b, dir + 128 + 116);
                for (int i = 2; i <= 301; ++i) {
                    put32(b, dir + i * 128 + 116, start);
                    put32(b, dir + i * 128 + 120, 100000);
                }
                QVERIFY(jp::cfb::read(b, &out));
                qint64 total = 0;
                for (const QString &p : out.streamPaths()) total += out.stream(p).size();
                QVERIFY2(total <= 2 * b.size() + (1 << 20), qPrintable(QString::number(total)));
                QCOMPARE(out.stream(QStringLiteral("s0")), QByteArray(100000, 'c'));
            }
            // Storages nested 50,000 deep.
            {
                jp::cfb::File f = cfbWith({});
                for (int i = 1; i <= 50000; ++i) {
                    jp::cfb::Entry e;
                    e.type = jp::cfb::Entry::Storage;
                    e.name = QStringLiteral("d");
                    f.entries[i - 1].children << i;
                    f.entries << e;
                }
                jp::cfb::Entry leaf;
                leaf.name = QStringLiteral("x");
                leaf.data = "deep";
                f.entries.last().children << int(f.entries.size());
                f.entries << leaf;
                jp::cfb::Entry near;
                near.name = QStringLiteral("y");
                near.data = "near";
                f.entries[1].children << int(f.entries.size());
                f.entries << near;
                QVERIFY(jp::cfb::read(jp::cfb::write(f), &out));
                QCOMPARE(out.stream(QStringLiteral("d/y")), QByteArray("near"));
                QVERIFY(out.entries.size() < 100);
            }
        }, 3) / 1000000;
        QVERIFY2(ms < 20000, qPrintable(QString::number(ms)));
    }

    void metafileWmfRenders()
    {
        QFile f(QStringLiteral(JP_TEST_DATA "/pub/poi-SampleBrochure.pub"));
        QVERIFY(f.open(QIODevice::ReadOnly));
        auto doc = importPublisher(f.readAll(), nullptr);
        QVERIFY(doc);
        int wmf = 0;
        for (auto it = doc->images.cbegin(); it != doc->images.cend(); ++it) {
            if (it->format != "wmf") continue;
            ++wmf;
            Metafile m;
            QVERIFY(m.load(it->bytes));
            const QImage img = m.toImage(200);
            int painted = 0;
            for (int y = 0; y < img.height(); y += 4)
                for (int x = 0; x < img.width(); x += 4) painted += qAlpha(img.pixel(x, y)) > 0;
            QVERIFY(painted > 10);
        }
        QVERIFY(wmf > 0);
        // Recolored, a metafile changes every color it draws (border art's
        // tinted pieces): nothing comes out but the new color.
        const auto first = std::find_if(doc->images.cbegin(), doc->images.cend(), [](const ImageData &d) { return d.format == "wmf"; });
        Metafile m;
        QVERIFY(m.load(first->bytes));
        QImage img(200, 200, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        {
            QPainter p(&img);
            m.play(&p, QRectF(0, 0, 200, 200), [](const QColor &c) { return QColor(0, 128, 128, c.alpha()); });
        }
        int teal = 0, other = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) {
                const QRgb c = img.pixel(x, y);
                if (qAlpha(c) < 250) continue;
                (qRed(c) < 10 && std::abs(qGreen(c) - 128) < 10 && std::abs(qBlue(c) - 128) < 10 ? teal : other)++;
            }
        QVERIFY2(teal > 100 && other == 0, qPrintable(QStringLiteral("%1 %2").arg(teal).arg(other)));
        // And a recolored metafile prints as vectors, not as a picture.
        auto one = Document::blank(QSizeF(300, 300));
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = one->addImage(first->bytes, QStringLiteral("wmf"));
        pic->rect = pic->imgRect = QRectF(20, 20, 200, 200);
        pic->imgRect.moveTo(0, 0);
        pic->recolor = PictureItem::ColorTint;
        pic->recolorColor = ColorRef::rgb(QColor(0, 128, 128));
        one->pages[0]->items.push_back(pic);
        MainWindow w;
        w.editor()->setDocument(std::move(one));
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("tinted.pdf"));
        QVERIFY(w.exportPdfTo(path, MainWindow::PdfSettings()));
        QtPdf parsed;
        QVERIFY(parsed.load(path));
        for (const auto &o : parsed.objects) QVERIFY(!QtPdf::dictOf(o.body).contains("/Subtype /Image"));
    }

    // Metafiles made here play the same on every system: boxes and ovals,
    // text, a bitmap, and text in the symbol fonts (Wingdings, Symbol) that
    // Windows has and Linux lacks, which shows as the same pictures from Unicode.
    void metafileDrawnFromScratch()
    {
        auto le16 = [](QByteArray &b, int v) { b.append(char(v & 0xFF)); b.append(char((v >> 8) & 0xFF)); };
        auto le32 = [](QByteArray &b, quint32 v) { for (int i = 0; i < 4; ++i) b.append(char((v >> (8 * i)) & 0xFF)); };
        auto words = [&](std::initializer_list<int> v) { QByteArray b; for (int w : v) le16(b, w); return b; };
        auto ints = [&](std::initializer_list<qint32> v) { QByteArray b; for (qint32 w : v) le32(b, quint32(w)); return b; };
        // A record: a metafile's has its length in words and a function, an
        // enhanced metafile's a type and its length in bytes.
        auto wmfRecord = [&](int fn, const QByteArray &params) {
            QByteArray b;
            le32(b, quint32((6 + params.size() + (params.size() & 1)) / 2));
            le16(b, fn);
            b += params;
            if (params.size() & 1) b.append('\0');
            return b;
        };
        auto emfRecord = [&](quint32 type, const QByteArray &payload) {
            QByteArray pad = payload;
            while (pad.size() % 4) pad.append('\0');
            QByteArray b;
            le32(b, type);
            le32(b, quint32(8 + pad.size()));
            return b + pad;
        };
        // A 400 by 300 drawing of a blue box (20 to 120 across), a red oval
        // (280 to 380), and a line of text in `face` at (x, y), `em` tall;
        // `more` is records after them.
        auto wmf = [&](const QString &face, int charset, const QByteArray &text, int x, int y, int em, const QByteArray &more = QByteArray()) {
            QByteArray body = wmfRecord(0x020B, words({0, 0})) + wmfRecord(0x020C, words({300, 400})) + wmfRecord(0x0102, words({1}));   // window, transparent text
            QByteArray blue = words({0}), red = words({0}), pen = words({5, 0, 0});
            le32(blue, 0xFF0000);
            le16(blue, 0);
            le32(red, 0x0000FF);
            le16(red, 0);
            le32(pen, 0);
            body += wmfRecord(0x02FC, blue) + wmfRecord(0x02FC, red) + wmfRecord(0x02FA, pen);   // objects 0, 1, 2
            body += wmfRecord(0x012D, words({2})) + wmfRecord(0x012D, words({0})) + wmfRecord(0x041B, words({80, 120, 20, 20}));
            body += wmfRecord(0x012D, words({1})) + wmfRecord(0x0418, words({80, 380, 20, 280}));
            QByteArray font = words({-em, 0, 0, 0, 400});
            font.append(char(0)).append(char(0)).append(char(0)).append(char(charset));
            font += QByteArray(4, '\0') + face.toLatin1() + '\0';
            body += wmfRecord(0x02FB, font) + wmfRecord(0x012D, words({3}));
            QByteArray line = words({int(text.size())}) + text;
            if (text.size() & 1) line.append('\0');
            le16(line, y);
            le16(line, x);
            body += wmfRecord(0x0209, QByteArray(4, '\0')) + wmfRecord(0x0521, line) + more + wmfRecord(0x0000, QByteArray());
            QByteArray head, standard;
            le32(head, 0x9AC6CDD7u);
            le16(head, 0);
            for (int v : {0, 0, 400, 300, 72}) le16(head, v);   // box, units to the inch
            le32(head, 0);
            quint16 sum = 0;
            for (int i = 0; i < 20; i += 2) sum ^= quint16(quint8(head[i]) | (quint8(head[i + 1]) << 8));
            le16(head, sum);
            for (int v : {1, 9, 0x300}) le16(standard, v);
            le32(standard, quint32((18 + body.size()) / 2));
            le16(standard, 4);
            le32(standard, 60);
            le16(standard, 0);
            return head + standard + body;
        };
        // The same drawing as an enhanced metafile, its text in UTF-16.
        auto emf = [&](const QString &face, int charset, const QString &text, int x, int y, int em, const QByteArray &more = QByteArray()) {
            QByteArray body = emfRecord(18, ints({1}));                           // transparent text
            body += emfRecord(39, ints({1, 0, 0xFF0000, 0})) + emfRecord(39, ints({2, 0, 0x0000FF, 0})) + emfRecord(37, ints({qint32(0x80000008u)}));   // blue, red, no pen
            body += emfRecord(37, ints({1})) + emfRecord(43, ints({20, 20, 120, 80})) + emfRecord(37, ints({2})) + emfRecord(42, ints({280, 20, 380, 80}));
            QByteArray lf = ints({3, -em, 0, 0, 0, 400});
            lf.append(char(0)).append(char(0)).append(char(0)).append(char(charset));
            lf += QByteArray(4, '\0');
            QByteArray name(64, '\0');
            for (int i = 0; i < face.size() && i < 31; ++i) { name[2 * i] = char(face[i].unicode() & 0xFF); name[2 * i + 1] = char(face[i].unicode() >> 8); }
            body += emfRecord(82, lf + name) + emfRecord(37, ints({3})) + emfRecord(24, ints({0}));
            QByteArray chars;
            for (QChar c : text) le16(chars, c.unicode());
            while (chars.size() % 4) chars.append('\0');
            const int fixed = 8 + 16 + 12 + 40;   // record header, bounds, scales, and the text's own fields
            QByteArray spacing;
            for (int i = 0; i < text.size(); ++i) le32(spacing, 13);
            const QByteArray run = ints({0, 0, 0, 0, 1}) + QByteArray(8, '\0') + ints({x, y, int(text.size()), fixed, 0, 0, 0, 0, 0, fixed + int(chars.size())}) + chars + spacing;
            body += emfRecord(84, run) + more + emfRecord(14, ints({0, 16, 20}));
            // Bounds, a frame of 400 by 300 pixels (in hundredths of a millimeter on a 3,840 pixel screen 1,016 millimeters wide), and the counts.
            const QByteArray head = ints({0, 0, 399, 299, 0, 0, 10583, 7937, qint32(0x464D4520u), 0x10000, 0, 1, 1, 0, 0, 0, 3840, 2880, 1016, 762});
            QByteArray all = emfRecord(1, head) + body;
            qToLittleEndian<quint32>(quint32(all.size()), all.data() + 48);
            return all;
        };
        auto picture = [](const QByteArray &data) {
            Metafile m;
            if (!m.load(data)) return QImage();
            return m.toImage(400);
        };
        // The dark pixels in an area, and the box they fill.
        auto ink = [](const QImage &img, const QRect &area, QRect *box = nullptr) {
            int n = 0, x0 = 1 << 20, y0 = 1 << 20, x1 = -1, y1 = -1;
            for (int y = area.top(); y <= area.bottom(); ++y)
                for (int x = area.left(); x <= area.right(); ++x) {
                    const QRgb c = img.pixel(x, y);
                    if (qAlpha(c) > 128 && qRed(c) < 100 && qGreen(c) < 100 && qBlue(c) < 100) {
                        ++n;
                        x0 = std::min(x0, x); y0 = std::min(y0, y); x1 = std::max(x1, x); y1 = std::max(y1, y);
                    }
                }
            if (box) *box = n ? QRect(QPoint(x0, y0), QPoint(x1, y1)) : QRect();
            return n;
        };
        const QByteArray wmfBytes = wmf(QStringLiteral("Arial"), 0, "Arial Hello", 20, 120, 28), emfBytes = emf(QStringLiteral("Arial"), 0, QStringLiteral("Arial Hello"), 20, 120, 28);
        for (const QImage &img : {picture(wmfBytes), picture(emfBytes)}) {
            QCOMPARE(img.size(), QSize(400, 300));
            QCOMPARE(QColor(img.pixel(70, 50)), QColor(0, 0, 255));       // the box
            QCOMPARE(QColor(img.pixel(330, 50)), QColor(255, 0, 0));      // the oval
            QCOMPARE(qAlpha(img.pixel(200, 50)), 0);                      // nothing between them
            QVERIFY(ink(img, QRect(0, 110, 400, 70)) > 150);              // the words
        }
        // Wingdings' round bullet is a solid disc, with the font or without
        // it: not an empty box for a missing letter, nor a thin "l".
        for (const QImage &img : {picture(wmf(QStringLiteral("Wingdings"), 2, "l", 120, 150, 120)), picture(emf(QStringLiteral("Wingdings"), 2, QString(QChar(0xF06C)), 120, 150, 120))}) {
            QRect box;
            const int dark = ink(img, QRect(0, 120, 400, 180), &box);
            QVERIFY2(!box.isEmpty() && dark > box.width() * box.height() * 0.6, qPrintable(QStringLiteral("%1 dark in %2 x %3").arg(dark).arg(box.width()).arg(box.height())));
        }
        // Symbol's Greek letters show too (Linux calls another font in for Symbol).
        for (const QImage &img : {picture(wmf(QStringLiteral("Symbol"), 2, "a", 120, 150, 120)), picture(emf(QStringLiteral("Symbol"), 2, QString(QChar(0xF061)), 120, 150, 120))})
            QVERIFY(ink(img, QRect(0, 120, 400, 180)) > 300);
        auto near = [](QRgb c, QColor want) { return qAlpha(c) == 255 && std::abs(qRed(c) - want.red()) < 12 && std::abs(qGreen(c) - want.green()) < 12 && std::abs(qBlue(c) - want.blue()) < 12; };
        // A bitmap inside one (2 by 2 pixels over 150 to 250 across and 100
        // to 200 down: blue and white above red and green) is decoded and drawn.
        QByteArray dib;
        for (quint32 v : {40u, 2u, 2u}) le32(dib, v);
        le16(dib, 1);
        le16(dib, 24);
        for (quint32 v : {0u, 16u, 0u, 0u, 0u, 0u}) le32(dib, v);
        dib += QByteArray::fromHex("0000FF00FF000000") + QByteArray::fromHex("FF0000FFFFFF0000");   // bottom row first, blue-green-red, each row padded to 4 bytes
        QByteArray stretchWmf, stretchEmf;
        le32(stretchWmf, 0x00CC0020);   // copy
        for (int v : {0, 2, 2, 0, 0, 100, 100, 100, 150}) le16(stretchWmf, v);   // usage; source height, width, y, x; destination height, width, y, x
        for (quint32 v : {0u, 0u, 0u, 0u, 150u, 100u, 0u, 0u, 2u, 2u, 80u, 40u, 120u, 16u, 0u, 0x00CC0020u, 100u, 100u}) le32(stretchEmf, v);   // bounds, destination, source, where the bitmap and its bits are
        stretchWmf += dib;
        stretchEmf += dib;
        for (const QImage &img : {picture(wmf(QStringLiteral("Arial"), 0, "", 0, 0, 12, wmfRecord(0x0F43, stretchWmf))),
                                  picture(emf(QStringLiteral("Arial"), 0, QString(), 0, 0, 12, emfRecord(81, stretchEmf)))}) {
            QVERIFY(near(img.pixel(175, 125), QColor(0, 0, 255)));
            QVERIFY(near(img.pixel(225, 125), QColor(255, 255, 255)));
            QVERIFY(near(img.pixel(175, 175), QColor(255, 0, 0)));
            QVERIFY(near(img.pixel(225, 175), QColor(0, 255, 0)));
        }
        // PatBlt fills a rectangle (here 150 to 250 across and 200 to 260 down) with the brush, as table cells in a pasted table are.
        QByteArray patBlt;
        le32(patBlt, 0x00F00021);   // copy the brush
        for (int v : {60, 100, 200, 150}) le16(patBlt, v);   // height, width, y, x
        const QImage filled = picture(wmf(QStringLiteral("Arial"), 0, "", 0, 0, 12, wmfRecord(0x012D, words({0})) + wmfRecord(0x061D, patBlt)));
        QVERIFY(near(filled.pixel(200, 230), QColor(0, 0, 255)));
        QCOMPARE(qAlpha(filled.pixel(100, 230)), 0);
        // Text in a font of the Greek character set is Greek: the byte 0xF4 is a tau there, where in the Western set it is an o with a
        // circumflex.
        {
            QRect greek, tau;
            ink(picture(wmf(QStringLiteral("Arial"), 161, "\xF4", 120, 150, 120)), QRect(0, 100, 400, 200), &greek);
            ink(picture(emf(QStringLiteral("Arial"), 0, QString(QChar(0x03C4)), 120, 150, 120)), QRect(0, 100, 400, 200), &tau);
            QVERIFY(!greek.isEmpty() && !tau.isEmpty());
            QVERIFY2(std::abs(greek.height() - tau.height()) <= 2 && std::abs(greek.width() - tau.width()) <= 2, qPrintable(QStringLiteral("greek %1 x %2, tau %3 x %4").arg(greek.width()).arg(greek.height()).arg(tau.width()).arg(tau.height())));
        }
        // The other letters of those sets that are not where the Western set has them: Greek capital alpha with a tonos (0xA2), and the Cyrillic
        // capital and small yo (0xA8 and 0xB8, in the Western set a diaeresis and a cedilla).
        for (const auto &letter : {std::make_tuple(161, "\xA2", 0x0386), std::make_tuple(204, "\xA8", 0x0401), std::make_tuple(204, "\xB8", 0x0451)}) {
            QRect fromBytes, fromUnicode;
            ink(picture(wmf(QStringLiteral("Arial"), std::get<0>(letter), std::get<1>(letter), 120, 150, 120)), QRect(0, 100, 400, 200), &fromBytes);
            ink(picture(emf(QStringLiteral("Arial"), 0, QString(QChar(std::get<2>(letter))), 120, 150, 120)), QRect(0, 100, 400, 200), &fromUnicode);
            QVERIFY(!fromBytes.isEmpty() && !fromUnicode.isEmpty());
            QVERIFY2(std::abs(fromBytes.height() - fromUnicode.height()) <= 2 && std::abs(fromBytes.width() - fromUnicode.width()) <= 2,
                     qPrintable(QStringLiteral("0x%1: %2 x %3 instead of %4 x %5").arg(std::get<2>(letter), 0, 16).arg(fromBytes.width()).arg(fromBytes.height()).arg(fromUnicode.width()).arg(fromUnicode.height())));
        }
        // Text drawn with an opaque background (a label's white box on a map) fills the string's cell with the background color, in a
        // metafile as in an enhanced one.
        {
            QByteArray colorRecord = QByteArray();
            le32(colorRecord, 0x0000FFFF);   // yellow (red 255, green 255, blue 0)
            QByteArray line = words({1}) + "I";
            line.append('\0');
            le16(line, 150);
            le16(line, 150);
            const QByteArray more = wmfRecord(0x0102, words({2})) + wmfRecord(0x0201, colorRecord) + wmfRecord(0x0521, line);
            const QImage opaque = picture(wmf(QStringLiteral("Arial"), 0, "", 0, 0, 120, more));
            QVERIFY(near(opaque.pixel(152, 152), QColor(255, 255, 0)));
            QCOMPARE(qAlpha(opaque.pixel(120, 152)), 0);   // (left of the cell)
        }
        // A flood of records that each paint the whole picture (400,000 of them, 7 MB, over a minute before) is stopped after the work a
        // picture may take, with what it drew before it still there, and so is one of records that each nest a saved state.
        {
            QByteArray patBlt;
            le32(patBlt, 0x00F00021);
            for (int v : {300, 400, 0, 0}) le16(patBlt, v);   // height, width, y, x
            QByteArray flood = wmfRecord(0x012D, words({0}));
            const QByteArray fill = wmfRecord(0x061D, patBlt);
            for (int i = 0; i < 400000; ++i) flood += fill;
            QImage drawn;
            const qint64 floodMs = bestNanoseconds([&] { drawn = picture(wmf(QStringLiteral("Arial"), 0, "", 0, 0, 12, flood)); }, 3) / 1000000;
            QVERIFY2(floodMs < 6000, qPrintable(QString::number(floodMs)));
            QVERIFY(near(drawn.pixel(200, 150), QColor(0, 0, 255)));
            QByteArray nested;
            for (int i = 0; i < 200000; ++i) nested += wmfRecord(0x001E, QByteArray());
            QImage deep;
            const qint64 deepMs = bestNanoseconds([&] { deep = picture(wmf(QStringLiteral("Arial"), 0, "", 0, 0, 12, nested + wmfRecord(0x012D, words({0})) + fill)); }, 3) / 1000000;
            QVERIFY2(deepMs < 6000, qPrintable(QString::number(deepMs)));
            QVERIFY(near(deep.pixel(200, 150), QColor(0, 0, 255)));
            // Saves past the cap are counted, so their restores undo nothing: a clip set after the first save
            // still holds once as many restores as later saves have run (before, they unwound past it).
            QByteArray balanced = wmfRecord(0x001E, QByteArray()) + wmfRecord(0x0416, words({10, 10, 0, 0}));
            for (int i = 0; i < 1005; ++i) balanced += wmfRecord(0x001E, QByteArray());
            for (int i = 0; i < 1005; ++i) balanced += wmfRecord(0x0127, words({0xFFFF}));
            const QImage kept = picture(wmf(QStringLiteral("Arial"), 0, "", 0, 0, 12, balanced + wmfRecord(0x012D, words({0})) + fill));
            QVERIFY(!near(kept.pixel(200, 150), QColor(0, 0, 255)));
        }
        // From here on, a warning that a painter ended with saved states fails the test.
        QTest::failOnWarning(QRegularExpression(QStringLiteral("saved states")));
        // A picture that sets a clip, then saves twice and restores once, is left with a saved state of its own open. Played onto a painter
        // with a clip of its own, it must leave that painter's clip and transform as they were, and what is drawn after it at (150, 150) shows.
        {
            const QByteArray leftOpen = wmfRecord(0x0416, words({100, 100, 0, 0})) + wmfRecord(0x001E, QByteArray()) + wmfRecord(0x001E, QByteArray())
                + wmfRecord(0x0127, words({-1}));
            QImage canvas(400, 300, QImage::Format_ARGB32_Premultiplied);
            canvas.fill(Qt::transparent);
            QPainter p(&canvas);
            p.setClipRect(QRect(0, 0, 300, 300));
            const QRegion clipBefore = p.clipRegion();
            const QTransform transformBefore = p.transform();
            Metafile m;
            QVERIFY(m.load(wmf(QStringLiteral("Arial"), 0, "", 0, 0, 12, leftOpen)));
            m.play(&p, QRectF(40, 30, 200, 150));
            QVERIFY2(p.clipRegion() == clipBefore, "the picture changed the painter's clip");
            QVERIFY2(p.transform() == transformBefore, "the picture changed the painter's transform");
            p.fillRect(QRect(150, 150, 50, 50), Qt::red);
            p.end();
            QCOMPARE(QColor(canvas.pixel(175, 175)), QColor(255, 0, 0));
        }
        // A RestoreDC of -2 after three saves restores two levels: to the state of the second save, a clip of 300 by 260, not the 100 by 100
        // corner the third save came after. The picture then unwinds cleanly, and the painter is as it was.
        {
            QByteArray fullPatBlt;
            le32(fullPatBlt, 0x00F00021);   // copy the brush
            for (int v : {300, 400, 0, 0}) le16(fullPatBlt, v);   // height, width, y, x
            const QByteArray twoLevels = wmfRecord(0x001E, QByteArray()) + wmfRecord(0x0416, words({260, 300, 0, 0})) + wmfRecord(0x001E, QByteArray())
                + wmfRecord(0x0416, words({100, 100, 0, 0})) + wmfRecord(0x001E, QByteArray()) + wmfRecord(0x0127, words({-2}))
                + wmfRecord(0x012D, words({0})) + wmfRecord(0x061D, fullPatBlt);
            QImage canvas(400, 300, QImage::Format_ARGB32_Premultiplied);
            canvas.fill(Qt::transparent);
            QPainter p(&canvas);
            p.setClipRect(QRect(0, 0, 250, 250));
            const QRegion clipBefore = p.clipRegion();
            const QTransform transformBefore = p.transform();
            Metafile m;
            QVERIFY(m.load(wmf(QStringLiteral("Arial"), 0, "", 0, 0, 12, twoLevels)));
            m.play(&p, QRectF(0, 0, 400, 300));
            QVERIFY2(p.clipRegion() == clipBefore, "the picture changed the painter's clip");
            QVERIFY2(p.transform() == transformBefore, "the picture changed the painter's transform");
            p.end();
            QCOMPARE(QColor(canvas.pixel(200, 200)), QColor(0, 0, 255));   // inside the 300 by 260 clip, outside the corner
        }
        // Insert Picture takes both from files, whatever the case of the
        // extension, and the page shows the picture on screen and in print.
        QTemporaryDir dir;
        MainWindow w;
        const QStringList names = {QStringLiteral("clip.WMF"), QStringLiteral("clip.emf")};
        for (int i = 0; i < 2; ++i) {
            QFile f(dir.filePath(names[i]));
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(i ? emfBytes : wmfBytes);
            f.close();
            w.insertFiles({f.fileName()}, QPointF(20, 20 + 360 * i));
        }
        Editor *ed = w.editor();
        QCOMPARE(ed->doc()->pages[0]->items.size(), size_t(2));
        PaintContext ctx;
        ctx.doc = ed->doc();
        ctx.cache = &ed->cache();
        for (int i = 0; i < 2; ++i) {
            const auto *pic = static_cast<PictureItem *>(ed->doc()->pages[0]->items[i].get());
            QCOMPARE(ed->doc()->images.value(pic->imageId).format, i ? QStringLiteral("emf") : QStringLiteral("wmf"));
            for (const bool output : {false, true}) {
                ctx.opt.output = output;
                const QImage page = Renderer::renderToImage(ctx, 0, 1.0);
                // The blue box, 70 of the picture's 400 units across and 50 of its 300 down.
                const QPoint box(int(pic->rect.x() + pic->rect.width() * 70 / 400), int(pic->rect.y() + pic->rect.height() * 50 / 300));
                QVERIFY2(QColor(page.pixel(box)) == QColor(0, 0, 255), qPrintable(QStringLiteral("picture %1, output %2").arg(i).arg(output)));
            }
        }
    }
};

JP_TEST_CLASS(PubExportTests)
#include "tst_pub_export.moc"
