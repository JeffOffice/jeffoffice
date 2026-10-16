// JeffPub's tests of reading and writing .pub files, and templates.
#include "pubhelpers.h"

// Rebuilds a .pub's drawing records (Escher/EscherStm) with the first
// shape properties table (OPT) that `edit` changes: `edit` gets its body
// and header and returns true once it has changed them. Container lengths
// are recomputed; padding between drawings is kept.
static QByteArray editFirstOpt(const QByteArray &escher, const std::function<bool(QByteArray &body, quint16 &head)> &edit)
{
    bool done = false;
    std::function<QByteArray(int, int)> rebuild = [&](int from, int to) {
        QByteArray out;
        int off = from;
        while (off + 8 <= to) {
            const quint16 vi = qFromLittleEndian<quint16>(escher.constData() + off);
            const quint16 type = qFromLittleEndian<quint16>(escher.constData() + off + 2);
            const quint32 len = qFromLittleEndian<quint32>(escher.constData() + off + 4);
            if (type < 0xF000 || type > 0xF200 || off + 8 + qint64(len) > to) {   // padding between drawings
                out += escher.at(off++);
                continue;
            }
            QByteArray body = escher.mid(off + 8, len);
            quint16 head = vi;
            if ((vi & 0xF) == 0xF) body = rebuild(off + 8, off + 8 + int(len));
            else if (type == 0xF00B && !done) done = edit(body, head);
            QByteArray h(8, 0);
            qToLittleEndian<quint16>(head, h.data());
            qToLittleEndian<quint16>(type, h.data() + 2);
            qToLittleEndian<quint32>(quint32(body.size()), h.data() + 4);
            out += h + body;
            off += 8 + int(len);
        }
        out += escher.mid(off, to - off);
        return out;
    };
    const QByteArray out = rebuild(0, int(escher.size()));
    return done ? out : QByteArray();
}

// Adds a property to an OPT body (after the others, its complex data last).
static void addOptProp(QByteArray &body, quint16 &head, quint16 id, quint32 value, const QByteArray &complex = {})
{
    const int count = head >> 4;
    QByteArray entry(6, 0);
    qToLittleEndian<quint16>(id, entry.data());
    qToLittleEndian<quint32>(complex.isEmpty() ? value : quint32(complex.size()), entry.data() + 2);
    body = body.left(count * 6) + entry + body.mid(count * 6) + complex;
    head = quint16((head & 0xF) | ((count + 1) << 4));
}

// Where a complex property's data starts in an OPT body, or -1.
static int optComplexAt(const QByteArray &body, quint16 head, quint16 want)
{
    const int count = head >> 4;
    int at = count * 6;
    for (int i = 0; i < count; ++i) {
        const quint16 id = qFromLittleEndian<quint16>(body.constData() + i * 6);
        const quint32 v = qFromLittleEndian<quint32>(body.constData() + i * 6 + 2);
        if ((id & 0x3FFF) == want) return at;
        if (id & 0x8000) at += int(v);
    }
    return -1;
}

class PubFileTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }
    void publisherSamplesImport_data()
    {
        QTest::addColumn<QString>("path");
        QDir dir(QStringLiteral(JP_TEST_DATA "/pub"));
        for (const QString &f : dir.entryList({"*.pub"}, QDir::Files)) QTest::newRow(qPrintable(f)) << dir.filePath(f);
    }
    void publisherSamplesImport()
    {
        QFETCH(QString, path);
        QString err;
        PubImportReport rep;
        auto doc = importPublisherFile(path, &err, &rep);
        QVERIFY2(doc, qPrintable(err));
        QVERIFY(!doc->pages.isEmpty());
        // Round trip through .jpub keeps everything.
        auto back = publicationFromBytes(publicationBytes(*doc, QImage()), &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->pages.size(), doc->pages.size());
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = doc.get();
        ctx.cache = &cache;
        for (int i = 0; i < doc->pages.size(); ++i) QVERIFY(!Renderer::renderToImage(ctx, i, 0.25).isNull());
    }
    // Photos stay JPEG when saved with a gray setting or cut to a shape
    // (a converted picture always has an alpha channel, which once made
    // every one of them a much larger PNG).
    void pubWriterKeepsJpegPhotos()
    {
        QImage photo(320, 200, QImage::Format_RGB32);
        for (int y = 0; y < photo.height(); ++y)
            for (int x = 0; x < photo.width(); ++x) photo.setPixel(x, y, qRgb(x * 255 / 320, y * 255 / 200, 128));
        QByteArray jpg;
        QBuffer buf(&jpg);
        buf.open(QIODevice::WriteOnly);
        photo.save(&buf, "JPEG", 90);
        auto doc = jp::Document::blank(QSizeF(612, 792));
        const QString id = doc->addImage(jpg, QStringLiteral("jpg"));
        auto gray = std::make_shared<jp::PictureItem>();
        gray->imageId = id;
        gray->rect = QRectF(72, 72, 160, 100);
        gray->recolor = jp::PictureItem::Grayscale;
        auto oval = std::make_shared<jp::PictureItem>();
        oval->imageId = id;
        oval->rect = QRectF(72, 300, 160, 100);
        oval->maskShape = QStringLiteral("ellipse");
        oval->brightness = 20;
        doc->pages[0]->items.push_back(gray);
        doc->pages[0]->items.push_back(oval);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("photos.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->images.size(), 2);
        for (auto it = back->images.cbegin(); it != back->images.cend(); ++it) QCOMPARE(it->format, QStringLiteral("jpg"));
    }

    // The first page number is read within the limits the .pub format and the
    // Page Numbers dialog keep (1 to 1000), however the file says it.
    void firstPageNumberLoadsWithinTheLimits()
    {
        const std::pair<int, int> cases[] = {{5000, 1000}, {99999, 1000}, {1001, 1000}, {1000, 1000}, {999, 999}, {1, 1}, {0, 1}, {-7, 1}};
        for (const auto &c : cases) {
            QJsonObject json = jp::Document::blank(QSizeF(612, 792))->toJson();
            QJsonObject setup = json["setup"].toObject();
            setup["firstPageNumber"] = c.first;
            json["setup"] = setup;
            jp::Document doc;
            doc.fromJson(json);
            QVERIFY2(doc.setup.firstPageNumber == c.second, qPrintable(QString::number(c.first)));
        }
    }

    // A file whose style names point outside their section (made from
    // JeffPub's own styles sample): it still opens with all its text,
    // losing only the damaged names.
    void damagedStyleNamesStillOpen()
    {
        QString err;
        auto doc = importPublisherFile(QStringLiteral(JP_TEST_DATA "/pub/jp-damaged-style-names.pub"), &err);
        QVERIFY2(doc, qPrintable(err));
        QString text;
        for (const auto &story : doc->stories) text += story->doc->toPlainText();
        QVERIFY2(text.contains(QStringLiteral("A pull quote")) && text.contains(QStringLiteral("Plain text")), qPrintable(text));
    }

    // A .jpub from someone else may claim anything: a huge table, a page a
    // million miles wide, NaN geometry, or a zip directory whose names run
    // past the end. It opens with sane values, or is refused, never crashes.
    void jpubHostileContent()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 2;
        t->cols = 2;
        t->colW = {100, 100};
        t->rowH = {30, 30};
        t->rect = QRectF(72, 72, 0, 0);
        t->syncRect();
        t->cells.resize(4);
        doc->pages[0]->items.push_back(t);
        QMap<QString, QByteArray> entries;
        QVERIFY(jp::readZip(jp::publicationBytes(*doc, QImage()), entries));
        QJsonObject o = QJsonDocument::fromJson(entries["document.json"]).object();
        QJsonObject setup = o["setup"].toObject();
        setup["size"] = QJsonArray{1e12, -5};
        o["setup"] = setup;
        QJsonArray pages = o["pages"].toArray();
        QJsonObject page = pages[0].toObject();
        QJsonArray items = page["items"].toArray();
        QJsonObject table = items[0].toObject();
        table["rows"] = 100000;
        table["cols"] = 100000;
        table["rect"] = QJsonArray{1e300, -1e300, 5, 5};
        items[0] = table;
        page["items"] = items;
        pages[0] = page;
        o["pages"] = pages;
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), entries["mimetype"]);
        z.add(QStringLiteral("document.json"), QJsonDocument(o).toJson());
        QString err;
        auto back = jp::publicationFromBytes(z.finish(), &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(back->pageSize().width() <= 20000 && back->pageSize().height() >= 1);
        const auto *bt = static_cast<const jp::TableItem *>(back->pages[0]->items[0].get());
        QVERIFY(bt->rows <= 256 && bt->cols <= 256);
        QCOMPARE(bt->cells.size(), qsizetype(bt->rows) * bt->cols);
        QVERIFY(std::abs(bt->rect.x()) <= 1e6);

        // A directory entry whose name length runs past the end of the file.
        QByteArray zip = jp::publicationBytes(*doc, QImage());
        const qsizetype dir = zip.lastIndexOf(QByteArray::fromHex("504b0102"));
        QVERIFY(dir > 0);
        zip[dir + 28] = char(0xFF);
        zip[dir + 29] = char(0xFF);
        QMap<QString, QByteArray> bad;
        QVERIFY(!jp::readZip(zip, bad));
    }

    // A picture kept in a .jpub file is held to what any picture is: an SVG
    // that names a file to draw from (or has a script) is left out; the
    // others stay.
    void jpubPicturesAreHeldToThePictureChecks()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        QMap<QString, QByteArray> entries;
        QVERIFY(jp::readZip(jp::publicationBytes(*doc, QImage()), entries));
        jp::ZipWriter z;
        z.add(QStringLiteral("mimetype"), entries["mimetype"]);
        z.add(QStringLiteral("document.json"), entries["document.json"]);
        const QByteArray head("<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"10\" height=\"10\">");
        z.add(QStringLiteral("images/bad.svg"), head + "<image xlink:href=\"/etc/hostname\" width=\"10\" height=\"10\"/></svg>");
        z.add(QStringLiteral("images/good.svg"), head + "<rect width=\"10\" height=\"10\" fill=\"red\"/></svg>");
        QString err;
        auto back = jp::publicationFromBytes(z.finish(), &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(back->images.value(QStringLiteral("bad")).bytes.isEmpty());
        QVERIFY(!back->images.value(QStringLiteral("good")).bytes.isEmpty());
        QVERIFY(!back->image(QStringLiteral("good")).isNull());
    }

    void publisherFuzzFilesDoNotCrash()
    {
        QDir dir(QStringLiteral(JP_TEST_DATA "/pub/fuzz"));
        for (const QString &f : dir.entryList({"*.pub"}, QDir::Files)) {
            QString err;
            auto doc = importPublisherFile(dir.filePath(f), &err);
            if (doc) {
                LayoutCache cache;
                PaintContext ctx;
                ctx.doc = doc.get();
                ctx.cache = &cache;
                for (int i = 0; i < doc->pages.size(); ++i) Renderer::renderToImage(ctx, i, 0.1);
            }
        }
    }
    void privateCorpus()
    {
        // Set JP_PUB_CORPUS to a folder of real .pub files to check them all (not part of CI).
        const QString root = qEnvironmentVariable("JP_PUB_CORPUS");
        if (root.isEmpty()) QSKIP("JP_PUB_CORPUS not set");
        QDirIterator it(root, {"*.pub", "*.PUB"}, QDir::Files, QDirIterator::Subdirectories);
        int ok = 0, total = 0;
        QStringList failed;
        while (it.hasNext()) {
            const QString f = it.next();
            QFile file(f);
            if (!file.open(QIODevice::ReadOnly) || !isPublisherFile(file.read(8))) continue;
            ++total;
            QString err;
            if (importPublisherFile(f, &err)) ++ok; else failed << f + ": " + err;
            // Compound-file round trip on real files too.
            file.seek(0);
            const QByteArray bytes = file.readAll();
            jp::cfb::File a, b;
            if (!jp::cfb::read(bytes, &a, &err) || !jp::cfb::read(jp::cfb::write(a), &b, &err)) { failed << f + ": cfb " + err; continue; }
            for (const QString &sp : a.streamPaths())
                if (b.stream(sp) != a.stream(sp)) { failed << f + ": cfb stream " + sp; break; }
        }
        qInfo("%d of %d .pub files imported", ok, total);
        for (const auto &f : failed) qWarning("%s", qPrintable(f));
        QCOMPARE(ok, total);
    }
    // Every built-in template must open with all of its text visible: no text box
    // (except a linked chain's last box, which may continue) and no shape text overflows.
    // Template options: "Include logo" places the business logo (or an empty
    // frame for it) and makes room; "Include mailing address" adds the
    // address side of a postcard. Templates show only their own options.
    void templateOptions()
    {
        QImage logo(40, 40, QImage::Format_RGB32);
        logo.fill(QColor(200, 30, 30));
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        logo.save(&buf, "PNG");
        auto logos = [](const jp::Document &d) {
            QVector<const jp::PictureItem *> out;
            for (const auto &pg : d.pages)
                jp::walkItems(pg->items, [&](const jp::ItemPtr &it) {
                    if (it->type() == jp::ItemType::Picture && it->name == QLatin1String("Logo")) out << static_cast<const jp::PictureItem *>(it.get());
                });
            return out;
        };
        int withLogo = 0;
        for (const jp::TemplateInfo &t : jp::templates()) {
            jp::TemplateOptions on;
            on.options["logo"] = true;
            on.logoBytes = png;
            on.logoFormat = QStringLiteral("png");
            auto a = t.build(on);
            auto b = t.build(jp::TemplateOptions());
            QVERIFY2(logos(*b).isEmpty(), qPrintable(t.id));
            if (!t.optionKeys.contains(QStringLiteral("logo"))) continue;
            ++withLogo;
            const auto got = logos(*a);
            QCOMPARE(got.size(), 1);
            QCOMPARE(a->image(got.first()->imageId).pixelColor(20, 20), QColor(200, 30, 30));
            // No text box on its page sits on top of the logo.
            const QRectF lr = got.first()->rect.adjusted(2, 2, -2, -2);
            for (const auto &pg : a->pages) {
                bool here = false;
                for (const auto &it : pg->items) here = here || it.get() == got.first();
                if (!here) continue;
                for (const auto &it : pg->items)
                    if (it->type() == jp::ItemType::Text) QVERIFY2(!it->rect.intersects(lr), qPrintable(t.id + QStringLiteral(" ") + a->storyDoc(static_cast<const jp::TextItem *>(it.get())->storyId)->toPlainText().left(30)));
            }
        }
        QVERIFY(withLogo >= 10);
        auto mergeBlocks = [](const jp::Document &d) {
            int n = 0;
            for (const auto &pg : d.pages)
                jp::walkItems(pg->items, [&](const jp::ItemPtr &it) {
                    if (it->type() != jp::ItemType::Text) return;
                    QTextDocument *sd = d.storyDoc(static_cast<const jp::TextItem *>(it.get())->storyId);
                    for (QTextBlock bl = sd->begin(); bl.isValid(); bl = bl.next())
                        for (auto f = bl.begin(); !f.atEnd(); ++f)
                            n += f.fragment().charFormat().stringProperty(jp::tp::Field) == QLatin1String("mergeblock:address");
                });
            return n;
        };
        const jp::TemplateInfo *pc = jp::findTemplate(QStringLiteral("postcard-greetings"));
        QVERIFY(pc && pc->optionKeys.contains(QStringLiteral("address")));
        jp::TemplateOptions off;
        off.options["address"] = false;
        QCOMPARE(mergeBlocks(*pc->build(off)), 0);
        QCOMPARE(mergeBlocks(*pc->build(jp::TemplateOptions())), 1);
    }

    // Page Design > Change Template applies another design to the open
    // publication: each story goes into the new design's box of the same
    // role, the pictures replace its placeholder pictures in reading order,
    // and what has no place waits in Extra Content. One undo step undoes it.
    void changeTemplate()
    {
        auto png = [](const QColor &c) {
            QImage im(60, 40, QImage::Format_RGB32);
            im.fill(c);
            QByteArray bytes;
            QBuffer buf(&bytes);
            buf.open(QIODevice::WriteOnly);
            im.save(&buf, "PNG");
            return bytes;
        };
        auto textOf = [](const jp::Document &d, const jp::Item *it) {
            if (it->type() == jp::ItemType::TextArt) return static_cast<const jp::TextArtItem *>(it)->text;
            return d.storyDoc(static_cast<const jp::TextItem *>(it)->storyId)->toPlainText();
        };
        // The boxes, TextArt, and pictures of a role, in the order they were made.
        auto withRole = [](const jp::Document &d, const QString &role) {
            QVector<jp::Item *> out;
            for (const auto &pg : d.pages)
                for (const auto &it : pg->items)
                    if (it->role == role) out << it.get();
            return out;
        };
        auto typeOver = [](jp::Document &d, jp::Item *it, const QString &text) {
            QTextCursor c(d.storyDoc(static_cast<jp::TextItem *>(it)->storyId));
            c.select(QTextCursor::Document);
            c.insertText(text);
        };
        auto noImages = [](QJsonObject o) {   // pictures stay in the file after an undo, like any edit's
            o.remove(QStringLiteral("images"));
            return o;
        };

        // Every built-in design tags its text boxes, TextArt, and pictures.
        for (const jp::TemplateInfo &t : jp::templates()) {
            auto built = t.build(jp::TemplateOptions());
            for (const auto &pg : built->pages)
                for (const auto &it : pg->items)
                    if (it->type() == jp::ItemType::Text || it->type() == jp::ItemType::TextArt || it->type() == jp::ItemType::Picture)
                        QVERIFY2(!it->role.isEmpty(), qPrintable(t.id));
        }

        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        jp::Editor *ed = w.editor();
        auto first = jp::findTemplate(QStringLiteral("flyer-announce"))->build(jp::TemplateOptions());
        first->templateId = QStringLiteral("flyer-announce");
        ed->setDocument(std::move(first));
        jp::Document *d = ed->doc();
        const QString scheme = d->colors.name, fonts = d->fonts.name;
        // Typed over: the headline and the story. Added: a date, a note, and
        // a second picture. The footer stays as the design made it.
        typeOver(*d, withRole(*d, QStringLiteral("title")).first(), QStringLiteral("Grand Opening Weekend"));
        typeOver(*d, withRole(*d, QStringLiteral("body")).first(), QStringLiteral("Tulips bloom along the whole river walk."));
        ed->addItem(ed->newTextBox(QRectF(72, 700, 200, 24), QStringLiteral("Friday, March 13")));
        ed->addItem(ed->newTextBox(QRectF(72, 730, 200, 24), QStringLiteral("Bring a lawn chair")));
        static_cast<jp::PictureItem *>(withRole(*d, QStringLiteral("picture")).first())->imageId = d->addImage(png(Qt::red), "png");
        auto blue = std::make_shared<jp::PictureItem>();
        blue->rect = QRectF(400, 40, 120, 80);
        blue->imageId = d->addImage(png(Qt::blue), "png");
        blue->imgRect = QRectF(QPointF(), blue->rect.size());
        ed->addItem(blue);

        const jp::TemplateInfo *event = jp::findTemplate(QStringLiteral("flyer-event"));
        auto reference = event->build(jp::optionsForChange(*d, jp::TemplateOptions()));
        auto refBox = [&](const QString &role) {
            const auto found = withRole(*reference, role);
            return found.size() == 1 ? found.first() : nullptr;
        };
        QVERIFY(refBox(QStringLiteral("title")) && refBox(QStringLiteral("body")) && refBox(QStringLiteral("date")) && refBox(QStringLiteral("picture")));

        const QJsonObject before = d->toJson();
        auto design = event->build(jp::optionsForChange(*d, jp::TemplateOptions()));
        design->templateId = event->id;
        const int steps = ed->undoStack()->count();
        jp::ChangeReport rep;
        applyTemplate(ed, std::move(design), &rep);
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        d = ed->doc();
        QCOMPARE(d->templateId, event->id);
        QCOMPARE(d->colors.name, scheme);   // only the gallery's choices replace the schemes
        QCOMPARE(d->fonts.name, fonts);
        // The headline went into the new design's TextArt title, the story and the date
        // into their boxes, and the red picture into the placeholder.
        const auto title = withRole(*d, QStringLiteral("title"));
        QCOMPARE(title.size(), 1);
        QCOMPARE(title.first()->type(), jp::ItemType::TextArt);
        QCOMPARE(textOf(*d, title.first()), QStringLiteral("Grand Opening Weekend"));
        QCOMPARE(title.first()->rect, refBox(QStringLiteral("title"))->rect);
        const auto body = withRole(*d, QStringLiteral("body"));
        QCOMPARE(body.size(), 1);
        QCOMPARE(textOf(*d, body.first()), QStringLiteral("Tulips bloom along the whole river walk."));
        QCOMPARE(body.first()->rect, refBox(QStringLiteral("body"))->rect);
        QCOMPARE(static_cast<jp::TextItem *>(body.first())->columns, 2);   // the new design's box
        const auto date = withRole(*d, QStringLiteral("date"));
        QCOMPARE(date.size(), 1);
        QCOMPARE(textOf(*d, date.first()), QStringLiteral("Friday, March 13"));
        QCOMPARE(date.first()->rect, refBox(QStringLiteral("date"))->rect);
        // Text the design keeps is its own: nothing of the old footer came across.
        const auto footers = withRole(*d, QStringLiteral("address"));
        QCOMPARE(footers.size(), withRole(*reference, QStringLiteral("address")).size());
        QCOMPARE(textOf(*d, footers.first()), textOf(*reference, withRole(*reference, QStringLiteral("address")).first()));
        const auto pics = withRole(*d, QStringLiteral("picture"));
        QCOMPARE(pics.size(), 1);
        auto *placed = static_cast<jp::PictureItem *>(pics.first());
        QCOMPARE(placed->rect, refBox(QStringLiteral("picture"))->rect);
        QCOMPARE(d->image(placed->imageId).pixelColor(30, 20), QColor(Qt::red));
        // The note and the second picture have no place: they are in Extra Content.
        QCOMPARE(rep.stories, 3);
        QCOMPARE(rep.pictures, 1);
        QCOMPARE(rep.extraStories, 1);
        QCOMPARE(rep.extraPictures, 1);
        QCOMPARE(int(d->extra.size()), 2);
        QCOMPARE(d->extra[0]->type(), jp::ItemType::Text);
        QCOMPARE(textOf(*d, d->extra[0].get()), QStringLiteral("Bring a lawn chair"));
        QCOMPARE(d->extra[1]->type(), jp::ItemType::Picture);
        QCOMPARE(d->image(static_cast<jp::PictureItem *>(d->extra[1].get())->imageId).pixelColor(30, 20), QColor(Qt::blue));
        // They stay with the publication when it is saved and opened again.
        {
            QString err;
            auto back = jp::publicationFromBytes(jp::publicationBytes(*d, QImage()), &err);
            QVERIFY2(back, qPrintable(err));
            QCOMPARE(int(back->extra.size()), 2);
            QCOMPARE(textOf(*back, back->extra[0].get()), QStringLiteral("Bring a lawn chair"));
            QCOMPARE(back->image(static_cast<jp::PictureItem *>(back->extra[1].get())->imageId).pixelColor(30, 20), QColor(Qt::blue));
        }

        // One undo step brings the first design back, and redo applies it again.
        ed->undo();
        QCOMPARE(noImages(ed->doc()->toJson()), noImages(before));
        QVERIFY(ed->doc()->extra.empty());
        ed->redo();
        QCOMPARE(int(ed->doc()->extra.size()), 2);
        d = ed->doc();

        // Extra Content: the pane lists what is left, each can be placed on the page or discarded.
        QTRY_VERIFY(w.act(QStringLiteral("pd.extraContent"))->isEnabled());   // the window updates its commands soon after a change
        w.showTaskPane(QStringLiteral("extra"));
        QCOMPARE(w.currentTaskPane(), QStringLiteral("extra"));
        auto *pane = w.findChild<jp::TaskPane *>();
        QVERIFY(pane);
        auto *list = pane->findChild<QListWidget *>(QStringLiteral("extraContentList"));
        QVERIFY(list);
        QCOMPARE(list->count(), 2);
        auto button = [&](const QString &text) -> QPushButton * {
            for (QPushButton *b : pane->findChildren<QPushButton *>())
                if (b->text() == text) return b;
            return nullptr;
        };
        QVERIFY(button(QStringLiteral("Place on Page")) && button(QStringLiteral("Discard")));
        {   // What a drag from the list carries: the ids of the items.
            std::unique_ptr<QMimeData> drag(list->model()->mimeData({list->model()->index(1, 0)}));
            QVERIFY(drag && drag->hasFormat(QString::fromLatin1(jp::kExtraContentMime)));
            QCOMPARE(QString::fromUtf8(drag->data(QString::fromLatin1(jp::kExtraContentMime))), d->extra[1]->id);
        }
        const int onPage = int(d->pages[ed->currentPage()]->items.size());
        list->setCurrentRow(1);   // the blue picture
        button(QStringLiteral("Place on Page"))->click();
        QCOMPARE(list->count(), 1);
        QCOMPARE(int(d->extra.size()), 1);
        QCOMPARE(int(d->pages[ed->currentPage()]->items.size()), onPage + 1);
        auto *landed = dynamic_cast<jp::PictureItem *>(d->pages[ed->currentPage()]->items.back().get());
        QVERIFY(landed);
        QCOMPARE(d->image(landed->imageId).pixelColor(30, 20), QColor(Qt::blue));
        QVERIFY(QRectF(QPointF(0, 0), d->pageSize()).contains(landed->rect));
        // A story placed becomes a text box big enough for its text.
        ed->placeExtra(d->extra[0]->id, QPointF(100, 600));
        auto *note = dynamic_cast<jp::TextItem *>(d->pages[ed->currentPage()]->items.back().get());
        QVERIFY(note);
        QCOMPARE(textOf(*d, note), QStringLiteral("Bring a lawn chair"));
        QCOMPARE(note->rect.topLeft(), QPointF(100, 600));
        QVERIFY(d->extra.empty());
        w.refreshUi();   // the pane follows the publication when the window updates, and only while it shows
        QCOMPARE(list->count(), 0);
        ed->undo();   // back in Extra Content
        QCOMPARE(int(d->extra.size()), 1);
        // Dragged from the pane onto the page, it lands where it is dropped.
        QMimeData dragged;
        dragged.setData(QString::fromLatin1(jp::kExtraContentMime), d->extra[0]->id.toUtf8());
        QDragEnterEvent enter(QPoint(300, 300), Qt::CopyAction, &dragged, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(w.canvas()->viewport(), &enter);
        QVERIFY(enter.isAccepted());
        QDropEvent drop(QPointF(300, 300), Qt::CopyAction, &dragged, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(w.canvas()->viewport(), &drop);
        QVERIFY(d->extra.empty());
        auto *dropped = dynamic_cast<jp::TextItem *>(d->pages[ed->currentPage()]->items.back().get());
        QVERIFY(dropped);
        QCOMPARE(dropped->rect.topLeft(), w.canvas()->toPage(QPointF(300, 300)));
        ed->undo();
        QCOMPARE(int(d->extra.size()), 1);
        ed->discardExtra(d->extra[0]->id);
        QVERIFY(d->extra.empty());
        QTRY_VERIFY(!w.act(QStringLiteral("pd.extraContent"))->isEnabled());
        ed->undo();
        QCOMPARE(int(d->extra.size()), 1);

        // "Create a new publication": the gallery's color scheme applies, the new
        // design's page size, and this publication stays as it is.
        const QJsonObject mid = noImages(d->toJson());
        const jp::TemplateInfo *invite = jp::findTemplate(QStringLiteral("invitation-evening"));
        jp::TemplateOptions pick;
        pick.colorScheme = QStringLiteral("Cherry");
        auto second = invite->build(jp::optionsForChange(*d, pick));
        second->templateId = invite->id;
        jp::MainWindow *other = w.changeTemplate(std::move(second), true);
        QVERIFY(other && other != &w);
        QCOMPARE(noImages(ed->doc()->toJson()), mid);
        const jp::Document *nd = other->editor()->doc();
        QCOMPARE(nd->pageSize(), QSizeF(5 * 72, 7 * 72));
        QCOMPARE(nd->colors.name, QStringLiteral("Cherry"));
        QCOMPARE(nd->fonts.name, fonts);
        QCOMPARE(nd->templateId, invite->id);
        const auto headline = withRole(*nd, QStringLiteral("title"));   // TextArt text into a text box
        QCOMPARE(headline.size(), 1);
        QCOMPARE(headline.first()->type(), jp::ItemType::Text);
        QCOMPARE(textOf(*nd, headline.first()), QStringLiteral("Grand Opening Weekend"));
        QVERIFY(!nd->extra.empty());   // the story and the note have no box in an invitation
        delete other;

        // Pictures take placeholders in reading order, page by page, top to
        // bottom, left to right; stories of one role take the boxes the same way.
        ed->setDocument(jp::Document::blank(QSizeF(612, 792)));
        d = ed->doc();
        auto addPicture = [&](const QColor &c, const QRectF &r) {
            auto p = std::make_shared<jp::PictureItem>();
            p->rect = r;
            p->imageId = d->addImage(png(c), "png");
            p->imgRect = QRectF(QPointF(), r.size());
            ed->addItem(p);
        };
        addPicture(Qt::red, QRectF(300, 100, 120, 80));
        addPicture(Qt::green, QRectF(50, 104, 120, 80));   // in red's row, to its left
        addPicture(Qt::blue, QRectF(50, 400, 120, 80));
        ed->addItem(ed->newTextBox(QRectF(50, 600, 300, 60), QStringLiteral("Lower story, written first, long enough to count as running text.")));
        ed->addItem(ed->newTextBox(QRectF(50, 300, 300, 60), QStringLiteral("Upper story, written second, long enough to count as running text.")));
        auto catalog = jp::findTemplate(QStringLiteral("catalog-fall"))->build(jp::optionsForChange(*d, jp::TemplateOptions()));
        applyTemplate(ed, std::move(catalog));
        d = ed->doc();
        auto reading = [&](int page, jp::ItemType type) {
            QVector<jp::Item *> out;
            for (const auto &it : d->pages[page]->items)
                if (it->type() == type && it->role != QLatin1String("logo")) out << it.get();
            std::stable_sort(out.begin(), out.end(), [](const jp::Item *a, const jp::Item *b) {
                return a->rect.top() != b->rect.top() ? a->rect.top() < b->rect.top() : a->rect.left() < b->rect.left();
            });
            return out;
        };
        auto colorOf = [&](const jp::Item *it) { return d->image(static_cast<const jp::PictureItem *>(it)->imageId).pixelColor(30, 20); };
        QCOMPARE(colorOf(reading(0, jp::ItemType::Picture).first()), QColor(Qt::green));
        const auto grid = reading(1, jp::ItemType::Picture);
        QCOMPARE(colorOf(grid[0]), QColor(Qt::red));
        QCOMPARE(colorOf(grid[1]), QColor(Qt::blue));
        QVERIFY(colorOf(grid[2]) != QColor(Qt::blue) && colorOf(grid[2]) != QColor(Qt::red));   // the design's own picture
        const auto boxes = reading(1, jp::ItemType::Text);
        QVERIFY(textOf(*d, boxes[0]).startsWith(QLatin1String("Upper story")));
        QVERIFY(textOf(*d, boxes[1]).startsWith(QLatin1String("Lower story")));
        QVERIFY(d->extra.empty());

        // Nothing the user made is lost. A table, shape, line, group, or Text Art
        // stays behind only when it is the old design's own and unchanged; any
        // other goes to Extra Content, whole, and can be placed back.
        auto extrasOf = [&](jp::ItemType t) {
            int n = 0;
            for (const auto &e : d->extra) n += e->type() == t;
            return n;
        };
        auto firstOf = [&](jp::ItemType t) -> jp::Item * {
            for (const auto &it : d->pages[0]->items)
                if (it->type() == t) return it.get();
            return nullptr;
        };
        auto start = [&](const QString &id) {
            auto made = jp::findTemplate(id)->build(jp::TemplateOptions());
            made->templateId = id;
            ed->setDocument(std::move(made));
            d = ed->doc();
        };
        auto change = [&](const QString &id, jp::ChangeReport *r) {
            const jp::TemplateInfo *to = jp::findTemplate(id);
            auto design = to->build(jp::optionsForChange(*d, jp::TemplateOptions()));
            design->templateId = id;
            applyTemplate(ed, std::move(design), r);
            d = ed->doc();
        };
        auto editStory = [&](const QString &storyId, const QString &text) {
            QTextCursor c(d->storyDoc(storyId));
            c.select(QTextCursor::Document);
            c.insertText(text);
        };
        // Untouched, a design's own shapes and lines stay with it.
        start(QStringLiteral("flyer-announce"));
        jp::ChangeReport kept;
        change(QStringLiteral("flyer-event"), &kept);
        QVERIFY(d->extra.empty());
        QCOMPARE(kept.extraObjects, 0);
        // Edited, moved, or drawn by the user, they go to Extra Content.
        start(QStringLiteral("flyer-announce"));
        auto *sidebar = static_cast<jp::ShapeItem *>(firstOf(jp::ItemType::Shape));
        QVERIFY(sidebar && !sidebar->storyId.isEmpty());
        editStory(sidebar->storyId, QStringLiteral("Doors open at six"));
        static_cast<jp::LineItem *>(firstOf(jp::ItemType::Line))->moveBy(0, 10);
        auto box = std::make_shared<jp::ShapeItem>();
        box->rect = QRectF(300, 600, 80, 40);
        ed->addItem(box);
        auto stroke = std::make_shared<jp::LineItem>();
        stroke->p1 = QPointF(40, 600);
        stroke->p2 = QPointF(240, 640);
        stroke->syncRect();
        ed->addItem(stroke);
        auto wordArt = std::make_shared<jp::TextArtItem>();
        wordArt->text = QStringLiteral("Drawn WordArt");
        wordArt->rect = QRectF(72, 640, 200, 60);
        ed->addItem(wordArt);
        auto table = std::static_pointer_cast<jp::TableItem>(ed->newTable(QRectF(72, 500, 240, 60), 2, 3));
        editStory(table->cell(0, 0).storyId, QStringLiteral("Cell A"));
        ed->addItem(table);
        auto inner = std::make_shared<jp::ShapeItem>();
        inner->rect = QRectF(420, 640, 50, 50);
        auto rule = std::make_shared<jp::LineItem>();
        rule->p1 = QPointF(420, 700);
        rule->p2 = QPointF(520, 700);
        rule->syncRect();
        auto group = std::make_shared<jp::GroupItem>();
        group->children = {inner, rule};
        group->syncRect();
        ed->addItem(group);
        jp::ChangeReport lost;
        change(QStringLiteral("flyer-event"), &lost);
        QCOMPARE(extrasOf(jp::ItemType::Shape), 2);   // the sidebar with its new words, and the drawn one
        QCOMPARE(extrasOf(jp::ItemType::Line), 2);    // the divider that was moved, and the drawn one
        QCOMPARE(extrasOf(jp::ItemType::Table), 1);
        QCOMPARE(extrasOf(jp::ItemType::Group), 1);
        QCOMPARE(extrasOf(jp::ItemType::TextArt), 1);
        QCOMPARE(lost.extraObjects, 6);
        QCOMPARE(lost.extraStories, 1);   // the Text Art is a story with no title box to take it
        // Placed back, a table is whole: its size, and the words in its cells.
        jp::Item *extraTable = nullptr, *extraGroup = nullptr;
        for (const auto &e : d->extra) {
            if (e->type() == jp::ItemType::Table) extraTable = e.get();
            if (e->type() == jp::ItemType::Group) extraGroup = e.get();
        }
        QVERIFY(extraTable && extraGroup);
        ed->placeExtra(extraTable->id);
        auto *back = dynamic_cast<jp::TableItem *>(d->pages[ed->currentPage()]->items.back().get());
        QVERIFY(back);
        QCOMPARE(back->rows, 2);
        QCOMPARE(back->cols, 3);
        QCOMPARE(d->storyDoc(back->cell(0, 0).storyId)->toPlainText(), QStringLiteral("Cell A"));
        ed->placeExtra(extraGroup->id);
        auto *regrouped = dynamic_cast<jp::GroupItem *>(d->pages[ed->currentPage()]->items.back().get());
        QVERIFY(regrouped);
        QCOMPARE(int(regrouped->children.size()), 2);
        // A design's own table stays behind unchanged; once a cell is edited, it goes to Extra Content.
        start(QStringLiteral("newsletter-classic"));
        change(QStringLiteral("letterhead"), &kept);
        QVERIFY(d->extra.empty());
        start(QStringLiteral("newsletter-classic"));
        ed->setCurrentPage(1);
        auto *calendar = static_cast<jp::TableItem *>(d->pages[1]->items.back().get());
        QCOMPARE(calendar->type(), jp::ItemType::Table);
        editStory(calendar->cell(1, 1).storyId, QStringLiteral("Opening night: the tulip show"));
        change(QStringLiteral("letterhead"), &lost);
        QCOMPARE(extrasOf(jp::ItemType::Table), 1);
        ed->placeExtra(d->extra.back()->id);
        auto *calendarBack = dynamic_cast<jp::TableItem *>(d->pages[ed->currentPage()]->items.back().get());
        QVERIFY(calendarBack);
        QCOMPARE(calendarBack->rows, 5);
        QCOMPARE(d->storyDoc(calendarBack->cell(1, 1).storyId)->toPlainText(), QStringLiteral("Opening night: the tulip show"));
        QCOMPARE(d->storyDoc(calendarBack->cell(2, 1).storyId)->toPlainText(), QStringLiteral("Open house at the new studio"));
        // Built with its logo (which moves a line), and saved and opened again, a design's own parts are still its own.
        jp::TemplateOptions withLogo;
        withLogo.options["logo"] = true;
        auto card = jp::findTemplate(QStringLiteral("bizcard-classic"))->build(withLogo);
        card->templateId = QStringLiteral("bizcard-classic");
        QString err;
        auto reopened = jp::publicationFromBytes(jp::publicationBytes(*card, QImage()), &err);
        QVERIFY2(reopened, qPrintable(err));
        ed->setDocument(std::move(reopened));
        d = ed->doc();
        change(QStringLiteral("bizcard-band"), &kept);
        QVERIFY(d->extra.empty());
    }

    // The gallery Change Template opens is the New page's, in a mode of its
    // own: its title and button say so, it offers designs and no blank
    // sizes, its schemes default to the publication's own, and choosing a
    // design asks whether to apply it here or to make a new publication.
    void changeTemplateGallery()
    {
        jp::MainWindow w;
        w.resize(1400, 900);
        w.show();
        jp::Editor *ed = w.editor();
        auto first = jp::findTemplate(QStringLiteral("flyer-announce"))->build(jp::TemplateOptions());
        first->templateId = QStringLiteral("flyer-announce");
        ed->setDocument(std::move(first));
        for (const auto &it : ed->doc()->pages[0]->items)
            if (it->role == QLatin1String("body")) {
                QTextCursor c(ed->doc()->storyDoc(static_cast<jp::TextItem *>(it.get())->storyId));
                c.select(QTextCursor::Document);
                c.insertText(QStringLiteral("Tulips bloom along the whole river walk."));
            }
        auto gallery = [&]() -> QWidget * {
            for (QWidget *c : w.findChildren<QWidget *>())
                if (c->inherits("jp::Backstage")) return c;
            return nullptr;
        };
        // What shows: the File view keeps its other pages, hidden.
        auto label = [](QWidget *in, const QString &text) {
            for (QLabel *l : in->findChildren<QLabel *>())
                if (l->isVisible() && l->text() == text) return true;
            return false;
        };
        auto button = [](QWidget *in, const QString &text) -> QAbstractButton * {
            for (QAbstractButton *b : in->findChildren<QAbstractButton *>())
                if (b->isVisible() && b->text() == text) return b;
            return nullptr;
        };
        // The New page is as it was.
        w.act(QStringLiteral("file.new"))->trigger();
        QVERIFY(gallery() && gallery()->isVisible());
        QVERIFY(label(gallery(), QStringLiteral("New Publication")));
        QVERIFY(button(gallery(), QStringLiteral("Create")));
        bool blankSizes = false;
        for (QListWidget *l : gallery()->findChildren<QListWidget *>())
            for (int i = 0; i < l->count(); ++i) blankSizes |= l->isVisible() && l->item(i)->text() == QLatin1String("Blank Sizes");
        QVERIFY(blankSizes);
        w.hideBackstage();

        w.act(QStringLiteral("pd.changeTemplate"))->trigger();
        QVERIFY(gallery()->isVisible());
        QVERIFY(label(gallery(), QStringLiteral("Change Template")));
        QVERIFY(!label(gallery(), QStringLiteral("New Publication")));
        QAbstractButton *go = button(gallery(), QStringLiteral("Change Template"));
        QVERIFY(go && !button(gallery(), QStringLiteral("Create")));
        QListWidget *designs = nullptr;
        blankSizes = false;
        for (QListWidget *l : gallery()->findChildren<QListWidget *>()) {
            if (!l->isVisible()) continue;
            if (l->count() && l->item(0)->data(Qt::UserRole).toString().startsWith(QLatin1String("tpl:"))) designs = l;
            for (int i = 0; i < l->count(); ++i) blankSizes |= l->item(i)->text() == QLatin1String("Blank Sizes");
        }
        QVERIFY(designs && !blankSizes);
        int keeping = 0;
        for (QComboBox *c : gallery()->findChildren<QComboBox *>()) keeping += c->isVisible() && c->itemText(0) == QLatin1String("(keep current)");
        QCOMPARE(keeping, 2);   // the color scheme and the font scheme
        for (int i = 0; i < designs->count(); ++i)
            if (designs->item(i)->data(Qt::UserRole).toString() == QLatin1String("tpl:flyer-event")) designs->setCurrentRow(i);
        QCOMPARE(designs->currentItem()->data(Qt::UserRole).toString(), QStringLiteral("tpl:flyer-event"));
        // The dialog gives the two ways; answer it the way it starts, applying here.
        QStringList choices;
        QTimer answer;
        answer.setInterval(20);
        connect(&answer, &QTimer::timeout, &answer, [&] {
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!dlg) return;
            for (QAbstractButton *b : dlg->findChildren<QAbstractButton *>())
                if (b->inherits("QRadioButton")) choices << b->text() + (b->isChecked() ? QStringLiteral(" (chosen)") : QString());
            answer.stop();
            dlg->accept();
        });
        answer.start();
        const int steps = ed->undoStack()->count();
        go->click();
        answer.stop();
        QCOMPARE(choices, (QStringList{QStringLiteral("Apply template to the current publication (chosen)"),
                                       QStringLiteral("Create a new publication with my text and graphics")}));
        QCOMPARE(ed->doc()->templateId, QStringLiteral("flyer-event"));
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QVERIFY(!gallery()->isVisible());
        bool landed = false;
        for (const auto &it : ed->doc()->pages[0]->items)
            if (it->type() == jp::ItemType::Text && ed->doc()->storyDoc(static_cast<jp::TextItem *>(it.get())->storyId)->toPlainText() == QLatin1String("Tulips bloom along the whole river walk."))
                landed = it->role == QLatin1String("body");
        QVERIFY(landed);
        // Cancelling the question changes nothing.
        w.act(QStringLiteral("pd.changeTemplate"))->trigger();
        go = button(gallery(), QStringLiteral("Change Template"));
        QTimer cancel;
        cancel.setInterval(20);
        connect(&cancel, &QTimer::timeout, &cancel, [&] {
            if (auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget())) {
                cancel.stop();
                dlg->reject();
            }
        });
        cancel.start();
        go->click();
        cancel.stop();
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QVERIFY(gallery()->isVisible());   // still choosing
    }

    void templatesFitTheirText()
    {
        QStringList problems;
        for (int withLogo = 0; withLogo < 2; ++withLogo)
        for (const jp::TemplateInfo &t : jp::templates()) {
            if (withLogo && !t.optionKeys.contains(QStringLiteral("logo"))) continue;
            jp::TemplateOptions opts;
            if (withLogo) opts.options["logo"] = true;
            std::unique_ptr<jp::Document> doc = t.build(opts);
            QVERIFY2(doc && !doc->pages.isEmpty(), qPrintable(t.id));
            jp::LayoutCache cache;
            jp::PaintContext ctx;
            ctx.doc = doc.get();
            ctx.cache = &cache;
            ctx.opt.output = true;
            for (int pi = 0; pi < doc->pages.size(); ++pi) {
                ctx.pageNumber = pi + 1;
                ctx.pageCount = int(doc->pages.size());
                jp::walkItems(doc->pages[pi]->items, [&](const jp::ItemPtr &it) {
                    if (auto *tx = dynamic_cast<jp::TextItem *>(it.get())) {
                        if (!tx->nextId.isEmpty()) return;
                        auto fl = cache.textFrame(*doc, *tx, pi + 1, ctx.opt);
                        if (fl.layout && fl.layout->overflow())
                            problems << QStringLiteral("%1 p%2 text \"%3\" used=%4 box=%5x%6").arg(t.id).arg(pi + 1).arg(doc->storyDoc(tx->storyId)->toPlainText().left(30)).arg(fl.layout->usedHeight(0)).arg(tx->rect.width()).arg(tx->rect.height());
                    } else if (auto *sh = dynamic_cast<jp::ShapeItem *>(it.get())) {
                        if (sh->storyId.isEmpty()) return;
                        QPointF o;
                        const jp::StoryLayout *l = jp::Renderer::shapeTextLayout(ctx, *sh, &o);
                        if (l && l->overflow())
                            problems << QStringLiteral("%1 p%2 shape %3 \"%4\"").arg(t.id).arg(pi + 1).arg(sh->shape, doc->storyDoc(sh->storyId)->toPlainText().left(30));
                    }
                });
            }
        }
        QVERIFY2(problems.isEmpty(), qPrintable(problems.join('\n')));
    }

    // A .pub with many formatting pages: the text stream's index of
    // sections goes on in chained 512-byte blocks, as Publisher writes it
    // (19 entries in the first, 20 in each after). Written in one block, a
    // long index ran into the text and Publisher refused the file.
    void pubTextIndexChains()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(36, 36, 540, 720);
        QStringList paras;
        for (int i = 0; i < 400; ++i) paras << QStringLiteral("Paragraph %1 of a long story.").arg(i + 1);
        t->storyId = doc->createStory(paras.join(QChar('\n')));
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("many.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        const QByteArray q = cfb::readStream(path, QStringLiteral("Quill/QuillSub/CONTENTS"));
        QVERIFY(q.size() > 1024);
        auto u16 = [&](qsizetype at) { return qFromLittleEndian<quint16>(q.constData() + at); };
        auto u32 = [&](qsizetype at) { return qFromLittleEndian<quint32>(q.constData() + at); };
        const int total = u16(0x0c);
        QVERIFY(total > 21);
        QCOMPARE(u32(0x14), quint32(q.size()));
        int seen = 0, blocks = 0;
        QStringList names;
        for (quint32 at = 0x18; at != 0xffffffffu; ++blocks) {
            QVERIFY(at + 8 <= quint32(q.size()) && blocks < 50);
            const int count = u16(at + 2);
            QVERIFY(count <= (at == 0x18 ? 19 : 20));
            // A block's entries stay in its own 512 bytes, clear of the text at 512.
            QVERIFY(at == 0x18 ? 0x20 + 24 * count <= 0x200 : at % 512 == 0 && 8 + 24 * count <= 512);
            for (int k = 0; k < count; ++k) names << QString::fromLatin1(q.mid(at + 8 + 24 * k + 2, 4));
            seen += count;
            at = u32(at + 4);
        }
        QCOMPARE(seen, total);
        QCOMPARE(u32(0x10), quint32(512 * blocks));
        QCOMPARE(names.first(), QStringLiteral("TEXT"));
        QVERIFY(names.contains(QStringLiteral("PL  ")) && names.count(QStringLiteral("FDPP")) >= 3);
        // Each formatting page names where its text starts: the page before's
        // last end (0 on the first), as Publisher writes it.
        for (const char *kind : {"FDPP", "FDPC"}) {
            quint32 prevEnd = 0;
            int pages = 0;
            for (quint32 at = 0x18; at != 0xffffffffu; at = u32(at + 4))
                for (int k = 0; k < u16(at + 2); ++k) {
                    const qsizetype e = at + 8 + 24 * k;
                    if (q.mid(e + 2, 4) != kind) continue;
                    const quint32 pg = u32(e + 16);
                    QCOMPARE(u32(pg + 4), prevEnd);
                    prevEnd = u32(pg + 8 + 4 * (u16(pg) - 1));
                    ++pages;
                }
            QVERIFY(pages >= 2);
        }
        // And it reads back whole.
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        const QString text = back->storyDoc(static_cast<TextItem *>(back->pages[0]->items[0].get())->storyId)->toPlainText();
        QVERIFY(text.startsWith(QStringLiteral("Paragraph 1 of")));
        QVERIFY(text.contains(QStringLiteral("Paragraph 400 of a long story.")));
    }

    // In Publisher 2000 and 98 files the objects on the scratch area are
    // under a page chunk without a canvas size; it's not a page (Publisher
    // shows the samples with 2 pages; JeffPub showed an empty third).
    void pub2000ScratchIsNotAPage()
    {
        for (const char *name : {"poi-Sample2000.pub", "poi-Sample98.pub"}) {
            QString err;
            auto doc = jp::importPublisherFile(QStringLiteral(JP_TEST_DATA "/pub/") + QLatin1String(name), &err);
            QVERIFY2(doc, qPrintable(err));
            QCOMPARE(int(doc->pages.size()), 2);
            for (const auto &pg : doc->pages) QVERIFY(!pg->items.empty());
        }
    }

    // AutoFit Text in .pub lives in the story's record: 05 = 1 for best fit,
    // 3 for shrink text on overflow, and a 0c flag for grow the box, as
    // Publisher writes them when each setting is chosen. A box without one
    // keeps its overflowing text hidden, as Publisher shows it.
    void pubAutofitSetting()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        const TextItem::Autofit fits[] = {TextItem::NoAutofit, TextItem::BestFit, TextItem::ShrinkOnOverflow, TextItem::GrowBox};
        for (int i = 0; i < 4; ++i) {
            auto t = std::make_shared<TextItem>();
            t->rect = QRectF(72, 72 + 150 * i, 200, 60);
            t->storyId = doc->createStory(QStringLiteral("The quick brown fox jumps over the lazy dog. ").repeated(12));
            t->autofit = fits[i];
            doc->pages[0]->items.push_back(t);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("fit.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        const QByteArray contents = cfb::readStream(path, QStringLiteral("Contents"));
        QVERIFY(contents.contains(QByteArray("\x05\x10\x01\x00\x07\x20", 6)));
        QVERIFY(contents.contains(QByteArray("\x05\x10\x03\x00\x07\x20", 6)));
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QMap<int, int> got;
        for (const auto &it : back->pages[0]->items)
            if (auto *t = dynamic_cast<TextItem *>(it.get())) got[qRound(t->rect.top())] = t->autofit;
        QCOMPARE(got.size(), 4);
        for (int i = 0; i < 4; ++i) QCOMPARE(got.value(72 + 150 * i, -1), int(fits[i]));
    }

    // Saving as .pub: pages, formatted text boxes, rectangles and ovals
    // come back through the independent .pub reader (libmspub).
    void pubWriterRoundTrip()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 300, 120);
        t->storyId = doc->createStory();
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            QTextCharFormat plain;
            plain.setFontFamilies(QStringList{QStringLiteral("Arimo")});
            plain.setFontPointSize(18);
            c.insertText(QStringLiteral("Hello "), plain);
            QTextCharFormat bold = plain;
            bold.setFontWeight(QFont::Bold);
            c.insertText(QStringLiteral("bold "), bold);
            QTextCharFormat red = plain;
            red.setForeground(QColor(200, 0, 0));
            red.setFontItalic(true);
            c.insertText(QStringLiteral("red italic"), red);
            QTextBlockFormat center;
            center.setAlignment(Qt::AlignHCenter);
            c.insertBlock(center, plain);
            c.insertText(QStringLiteral("Second paragraph, centered"));
        }
        doc->pages[0]->items.push_back(t);
        auto r = std::make_shared<jp::ShapeItem>();
        r->shape = QStringLiteral("rect");
        r->rect = QRectF(72, 300, 200, 100);
        r->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(220, 30, 30)));
        r->stroke = jp::Stroke::line(jp::ColorRef::rgb(Qt::black), 2);
        doc->pages[0]->items.push_back(r);
        auto e = std::make_shared<jp::ShapeItem>();
        e->shape = QStringLiteral("ellipse");
        e->rect = QRectF(320, 300, 150, 150);
        e->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(30, 60, 200)));
        e->stroke = jp::Stroke::none();
        doc->pages[0]->items.push_back(e);

        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("written.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) QFile::copy(path, qEnvironmentVariable("JP_SHOT_DIR") + "/jeffpub-test.pub");
        QString e1;
        auto back = jp::importPublisherFile(path, &e1);
        QVERIFY2(back, qPrintable(e1));
        QCOMPARE(back->pages.size(), 1);
        QCOMPARE(back->pageSize(), QSizeF(612, 792));
        // Text: both paragraphs, with bold, italic and color where they were.
        const jp::TextItem *bt = nullptr;
        int shapes = 0;
        QVector<QRectF> shapeRects;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Text) bt = static_cast<const jp::TextItem *>(it.get());
            if (it->type() == jp::ItemType::Shape) { ++shapes; shapeRects << it->rect; }
        });
        QVERIFY(bt);
        QVERIFY(std::abs(bt->rect.left() - 72) < 1 && std::abs(bt->rect.width() - 300) < 1);
        QTextDocument *sd = back->storyDoc(bt->storyId);
        QVERIFY(sd);
        QCOMPARE(sd->blockCount(), 2);
        QCOMPARE(sd->begin().text(), QStringLiteral("Hello bold red italic"));
        QCOMPARE(sd->begin().next().text(), QStringLiteral("Second paragraph, centered"));
        QVERIFY(sd->begin().next().blockFormat().alignment() & Qt::AlignHCenter);
        bool sawBold = false, sawItalicRed = false;
        for (auto it = sd->begin().begin(); !it.atEnd(); ++it) {
            const QTextCharFormat f = it.fragment().charFormat();
            if (it.fragment().text().contains(QStringLiteral("bold"))) sawBold = f.fontWeight() >= QFont::Bold;
            if (it.fragment().text().contains(QStringLiteral("red"))) {
                const QString cref = f.stringProperty(jp::tp::ColorRefP);
                const QColor c = cref.isEmpty() ? f.foreground().color() : jp::ColorRef::fromString(cref).resolve(back->colors);
                sawItalicRed = f.fontItalic() && c.red() > 150 && c.green() < 60;
            }
            if (it.fragment().text().startsWith(QStringLiteral("Hello"))) QVERIFY(std::abs(f.fontPointSize() - 18) < 0.1);
        }
        QVERIFY(sawBold);
        QVERIFY(sawItalicRed);
        // Arimo is saved as Arial, which it matches width for width.
        QCOMPARE(sd->begin().begin().fragment().charFormat().fontFamilies().toStringList().value(0), QStringLiteral("Arial"));
        QVERIFY2(shapes >= 2, qPrintable(QString::number(shapes)));
    }

    // Text Art saved as Publisher's own Text Art reads back with its words,
    // warp and settings.
    void pubWriterTextArt()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        struct Spec { const char *text, *transform; bool bold, italic; double spacing, rotation; QColor color; };
        const Spec specs[] = {{"test19 Plain", "plain", true, false, 1.0, 0, QColor(200, 30, 30)},
                              {"Arch Up", "archUp", false, false, 1.0, 0, QColor(30, 90, 200)},
                              {"Around the circle", "circle", true, false, 1.2, 20, QColor(30, 150, 60)},
                              {"Wave", "waveUp", false, true, 1.0, 0, QColor(150, 60, 160)},
                              {"Inflate", "inflate", true, false, 0.9, 0, QColor(220, 140, 20)},
                              {"Slant Up", "slantUp", false, false, 1.0, 0, QColor(20, 20, 20)}};
        QVector<std::shared_ptr<jp::TextArtItem>> made;
        for (int i = 0; i < 6; ++i) {
            auto ta = std::make_shared<jp::TextArtItem>();
            ta->text = QString::fromLatin1(specs[i].text);
            ta->transform_ = QString::fromLatin1(specs[i].transform);
            ta->font = QStringLiteral("Arimo");
            ta->bold = specs[i].bold;
            ta->italic = specs[i].italic;
            ta->spacing = specs[i].spacing;
            ta->rotation = specs[i].rotation;
            ta->rect = QRectF(60 + (i % 2) * 260, 60 + (i / 2) * 230, 220, 150);
            ta->fill = jp::Fill::solid(jp::ColorRef::rgb(specs[i].color));
            ta->stroke = jp::Stroke::none();
            doc->pages[0]->items.push_back(ta);
            made << ta;
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("test19-textart.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test20-textart";
            QFile::remove(out + ".pub");
            QFile::copy(path, out + ".pub");
            QString e2;
            jp::savePublication(*doc, out + ".jpub", QImage(), &e2);
            // The same with an ordinary text box too.
            auto withText = jp::Document::blank(QSizeF(612, 792));
            for (const auto &ta : made) withText->pages[0]->items.push_back(ta->clone());
            auto label = std::make_shared<jp::TextItem>();
            label->rect = QRectF(60, 720, 400, 30);
            label->storyId = withText->createStory(QStringLiteral("test21 Text Art with a text box"));
            withText->pages[0]->items.push_back(label);
            QFile::remove(qEnvironmentVariable("JP_SHOT_DIR") + "/test21-textart-and-text.pub");
            QVERIFY(jp::exportPublisher(*withText, qEnvironmentVariable("JP_SHOT_DIR") + "/test21-textart-and-text.pub", &e2));
        }
        QString e1;
        auto back = jp::importPublisherFile(path, &e1);
        QVERIFY2(back, qPrintable(e1));
        QVector<const jp::TextArtItem *> got;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::TextArt) got << static_cast<const jp::TextArtItem *>(it.get());
        });
        QCOMPARE(got.size(), 6);
        for (const auto &want : made) {
            const jp::TextArtItem *g = nullptr;
            for (const jp::TextArtItem *c : got)
                if (c->text == want->text) g = c;
            QVERIFY2(g, qPrintable(want->text));
            QCOMPARE(g->transform_, want->transform_);
            QCOMPARE(g->bold, want->bold);
            QCOMPARE(g->italic, want->italic);
            QCOMPARE(g->font, QStringLiteral("Arial"));
            QVERIFY(std::abs(g->spacing - want->spacing) < 0.01);
            QVERIFY2(std::abs(g->rotation - want->rotation) < 1, qPrintable(QString::number(g->rotation)));
            QVERIFY(QLineF(g->rect.center(), want->rect.center()).length() < 1 && std::abs(g->rect.width() - want->rect.width()) < 1);
            QCOMPARE(g->fill.color.resolve(back->colors), want->fill.color.resolve(doc->colors));
        }
    }

    // Shapes holding text keep their text, placed where JeffPub puts it.
    void pubWriterShapeText()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        const QStringList kinds = {QStringLiteral("rect"), QStringLiteral("star5"), QStringLiteral("wedgeRoundRectCallout")};
        for (int i = 0; i < kinds.size(); ++i) {
            auto sh = std::make_shared<jp::ShapeItem>();
            sh->shape = kinds[i];
            sh->rect = QRectF(72, 72 + i * 200, 250, 150);
            sh->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(250, 220, 120)));
            sh->stroke = jp::Stroke::line(jp::ColorRef::rgb(Qt::black), 1);
            sh->storyId = doc->createStory(QStringLiteral("test18 text in a %1").arg(kinds[i]));
            sh->valign = i == 0 ? jp::VAlign::Top : jp::VAlign::Middle;
            doc->pages[0]->items.push_back(sh);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("test18-shape-text.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test18-shape-text";
            QFile::remove(out + ".pub");
            QFile::copy(path, out + ".pub");
            QString e2;
            jp::savePublication(*doc, out + ".jpub", QImage(), &e2);
        }
        QString e1;
        auto back = jp::importPublisherFile(path, &e1);
        QVERIFY2(back, qPrintable(e1));
        QStringList texts;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            QString sid;
            if (it->type() == jp::ItemType::Text) sid = static_cast<const jp::TextItem *>(it.get())->storyId;
            if (it->type() == jp::ItemType::Shape) sid = static_cast<const jp::ShapeItem *>(it.get())->storyId;
            if (const QTextDocument *d = sid.isEmpty() ? nullptr : back->storyDoc(sid)) texts << d->toPlainText();
        });
        for (const QString &k : kinds) QVERIFY2(texts.contains(QStringLiteral("test18 text in a %1").arg(k)), qPrintable(texts.join(" | ")));
    }

    // Preset shapes saved to .pub (Publisher's own where they match, freeforms
    // otherwise) read back with the same outline.
    void pubWriterShapes()
    {
        const QStringList ids = {"roundRect", "triangle", "hexagon", "foldedCorner", "fcDecision", "fcPredefined", "star5", "star12",
                                 "heart", "cloud", "rightArrow", "curvedRightArrow", "chevron", "wave", "ribbon2", "smiley", "sun",
                                 "moon", "lightning", "donut", "blockArc", "wedgeRoundRectCallout", "cloudCallout", "mathMultiply",
                                 "bracePair", "arc"};
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto label = std::make_shared<jp::TextItem>();
        label->rect = QRectF(54, 30, 500, 30);
        label->storyId = doc->createStory(QStringLiteral("test16 shapes"));
        doc->pages[0]->items.push_back(label);
        QVector<std::shared_ptr<jp::ShapeItem>> made;
        for (int i = 0; i < ids.size(); ++i) {
            auto sh = std::make_shared<jp::ShapeItem>();
            sh->shape = ids[i];
            sh->rect = QRectF(60 + (i % 5) * 104, 80 + (i / 5) * 100, 80, 60);
            const bool open = jp::shapeDef(ids[i]) && jp::shapeDef(ids[i])->open;
            sh->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor::fromHsv((i * 37) % 360, 160, 220)));
            sh->stroke = open ? jp::Stroke::line(jp::ColorRef::rgb(Qt::black), 2) : jp::Stroke::line(jp::ColorRef::rgb(Qt::black), 1);
            doc->pages[0]->items.push_back(sh);
            made << sh;
        }
        // A right triangle as drawn, and the same one flipped and turned 30°.
        for (int k = 0; k < 2; ++k) {
            auto t = std::make_shared<jp::ShapeItem>();
            t->shape = QStringLiteral("rtTriangle");
            t->rect = QRectF(160 + k * 200, 650, 100, 80);
            t->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(220, 120, 30)));
            t->stroke = jp::Stroke::none();
            if (k) {
                t->flipH = true;
                t->rotation = 30;
            }
            doc->pages[0]->items.push_back(t);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("test16-shapes.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test16-shapes";
            QFile::remove(out + ".pub");
            QFile::copy(path, out + ".pub");
            QString e2;
            jp::savePublication(*doc, out + ".jpub", QImage(), &e2);
        }
        QString e1;
        auto back = jp::importPublisherFile(path, &e1);
        QVERIFY2(back, qPrintable(e1));
        // Each shape's outline lands where it was (filled area overlap).
        auto area = [](const QPainterPath &p, bool stroke) {
            QImage img(612, 792, QImage::Format_Grayscale8);
            img.fill(0);
            QPainter g(&img);
            if (stroke) g.strokePath(p, QPen(Qt::white, 3));
            else g.fillPath(p, Qt::white);
            return img;
        };
        QVector<const jp::ShapeItem *> got;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Shape) got << static_cast<const jp::ShapeItem *>(it.get());
        });
        for (const auto &sh : made) {
            auto outline = [](const jp::ShapeItem *x) {
                QPainterPath p = x->customPath.isEmpty() ? jp::shapePath(x->shape, x->rect.size()) : x->customPath;
                return x->transform().map(p);
            };
            const jp::ShapeItem *match = nullptr;
            for (const jp::ShapeItem *g : got)
                if (QLineF(outline(g).boundingRect().center(), outline(sh.get()).boundingRect().center()).length() < 10) match = g;
            QVERIFY2(match, qPrintable(sh->shape));
            const bool open = jp::shapeDef(sh->shape)->open;
            QPainterPath gp = match->customPath;
            if (gp.isEmpty()) gp = jp::shapePath(match->shape, match->rect.size(), match->adj);
            // Publisher's own shapes come back as the same presets.
            if (!jp::presetForPubShapeType(jp::pubShapeType(sh->shape)).isEmpty()) QCOMPARE(match->shape, sh->shape);
            const QImage a = area(sh->transform().map(jp::shapePath(sh->shape, sh->rect.size())), open);
            const QImage b = area(match->transform().map(gp), open);
            qint64 both = 0, any = 0;
            for (int y = 0; y < a.height(); ++y)
                for (int x = 0; x < a.width(); ++x) {
                    const bool pa = a.constScanLine(y)[x], pb = b.constScanLine(y)[x];
                    both += pa && pb;
                    any += pa || pb;
                }
            QVERIFY2(any && 100.0 * both / any > 97, qPrintable(QStringLiteral("%1 %2%").arg(sh->shape).arg(100.0 * both / std::max<qint64>(1, any))));
        }
    }

    // Gradients, picture fills, transparency and shadows survive .pub.
    void pubWriterFills()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto text = [&](const QRectF &r, const QString &s) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = r;
            t->storyId = doc->createStory(s);
            doc->pages[0]->items.push_back(t);
            return t;
        };
        text(QRectF(54, 30, 500, 30), QStringLiteral("test26 fills"));
        const QColor red(220, 30, 30), blue(30, 60, 200), yellow(250, 220, 40), green(30, 160, 60);
        struct Case { QString label; jp::Fill fill; jp::Stroke stroke = jp::Stroke::line(jp::ColorRef::rgb(Qt::black), 1); bool shadow = false; };
        QVector<Case> cases;
        for (double a : {90.0, 0.0, 45.0, 135.0, 270.0})
            cases << Case{QStringLiteral("linear %1").arg(a), jp::Fill::gradient(jp::ColorRef::rgb(red), jp::ColorRef::rgb(blue), a)};
        jp::Fill three = jp::Fill::gradient(jp::ColorRef::rgb(red), jp::ColorRef::rgb(green), 0);
        three.stops = {{0, jp::ColorRef::rgb(red), 0}, {0.5, jp::ColorRef::rgb(yellow), 0}, {1, jp::ColorRef::rgb(green), 0}};
        cases << Case{QStringLiteral("three colors"), three};
        jp::Fill radial = jp::Fill::gradient(jp::ColorRef::rgb(Qt::white), jp::ColorRef::rgb(blue));
        radial.gradType = jp::Fill::Radial;
        cases << Case{QStringLiteral("radial"), radial};
        jp::Fill fade = jp::Fill::gradient(jp::ColorRef::rgb(blue), jp::ColorRef::rgb(blue), 0);
        fade.stops = {{0, jp::ColorRef::rgb(blue), 0}, {1, jp::ColorRef::rgb(blue), 1}};
        cases << Case{QStringLiteral("fade out"), fade};
        cases << Case{QStringLiteral("50% clear"), jp::Fill::solid(jp::ColorRef::rgb(blue), 0.5)};
        jp::Stroke thick = jp::Stroke::line(jp::ColorRef::rgb(red), 8);
        thick.transparency = 0.5;
        cases << Case{QStringLiteral("clear line"), jp::Fill::solid(jp::ColorRef::rgb(yellow)), thick};
        cases << Case{QStringLiteral("shadow"), jp::Fill::solid(jp::ColorRef::rgb(yellow)), jp::Stroke::line(jp::ColorRef::rgb(Qt::black), 1), true};
        QImage checks(32, 32, QImage::Format_RGB32);
        for (int y = 0; y < 32; ++y)
            for (int x = 0; x < 32; ++x) checks.setPixel(x, y, ((x / 8 + y / 8) % 2) ? qRgb(30, 60, 200) : qRgb(250, 220, 40));
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        checks.save(&buf, "PNG");
        jp::Fill pic;
        pic.type = jp::Fill::Picture;
        pic.imageId = doc->addImage(png, QStringLiteral("png"));
        cases << Case{QStringLiteral("picture"), pic};
        jp::Fill tiled = pic;
        tiled.type = jp::Fill::Texture;
        cases << Case{QStringLiteral("texture"), tiled};
        jp::Fill pattern;
        pattern.type = jp::Fill::Pattern;
        pattern.color = jp::ColorRef::rgb(blue);
        pattern.color2 = jp::ColorRef::rgb(yellow);
        pattern.pattern = 9;
        cases << Case{QStringLiteral("pattern"), pattern};
        QVector<std::shared_ptr<jp::ShapeItem>> made;
        for (int i = 0; i < cases.size(); ++i) {
            auto sh = std::make_shared<jp::ShapeItem>();
            sh->shape = QStringLiteral("rect");
            sh->rect = QRectF(60 + (i % 4) * 130, 80 + (i / 4) * 150, 100, 90);
            sh->fill = cases[i].fill;
            sh->stroke = cases[i].stroke;
            if (cases[i].shadow) {
                sh->fx.shadow.on = true;
                sh->fx.shadow.distance = 6;
                sh->fx.shadow.color = jp::ColorRef::rgb(Qt::black);
                sh->fx.shadow.transparency = 0.5;
            }
            doc->pages[0]->items.push_back(sh);
            made << sh;
            text(QRectF(sh->rect.left(), sh->rect.bottom() + 8, 120, 24), cases[i].label);
        }
        // A text box with a gradient, a border and a shadow.
        auto box = text(QRectF(60, 700, 300, 50), QStringLiteral("A text box with a gradient and a shadow"));
        box->fill = jp::Fill::gradient(jp::ColorRef::rgb(yellow), jp::ColorRef::rgb(Qt::white), 90);
        box->stroke = jp::Stroke::line(jp::ColorRef::rgb(blue), 2);
        box->fx.shadow.on = true;
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("test26-fills.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test26-fills";
            QFile::remove(out + ".pub");
            QFile::copy(path, out + ".pub");
            QString e2;
            jp::savePublication(*doc, out + ".jpub", QImage(), &e2);
        }
        QString e1;
        auto back = jp::importPublisherFile(path, &e1);
        QVERIFY2(back, qPrintable(e1));
        QVector<const jp::Item *> got;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Shape || it->type() == jp::ItemType::Picture) got << it.get();
        });
        auto near = [&](const QRectF &r) -> const jp::Item * {
            for (const jp::Item *g : got)
                if (QLineF(g->rect.center(), r.center()).length() < 3 && std::abs(g->rect.width() - r.width()) < 3) return g;
            return nullptr;
        };
        auto angleDiff = [](double a, double b) { return std::abs(std::remainder(a - b, 360.0)); };
        for (int i = 0; i < cases.size(); ++i) {
            const jp::Item *g = near(made[i]->rect);
            QVERIFY2(g, qPrintable(cases[i].label));
            const jp::Fill &want = cases[i].fill;
            if (want.type == jp::Fill::Picture) {
                QCOMPARE(g->type(), jp::ItemType::Picture);
                continue;
            }
            if (want.type == jp::Fill::Texture || want.type == jp::Fill::Pattern) {
                // Textures and patterns come back as tiled pictures, pixel for pixel.
                QCOMPARE(g->type(), jp::ItemType::Shape);
                const auto *sh = static_cast<const jp::ShapeItem *>(g);
                QCOMPARE(int(sh->fill.type), int(jp::Fill::Texture));
                const QImage want = cases[i].fill.type == jp::Fill::Pattern ? cases[i].fill.patternTile(doc->colors) : checks;
                QCOMPARE(back->image(sh->fill.imageId).convertToFormat(QImage::Format_ARGB32), want.convertToFormat(QImage::Format_ARGB32));
                continue;
            }
            QCOMPARE(g->type(), jp::ItemType::Shape);
            const auto *sh = static_cast<const jp::ShapeItem *>(g);
            QCOMPARE(int(sh->fill.type), int(want.type));
            if (want.type == jp::Fill::Gradient) {
                QCOMPARE(int(sh->fill.gradType == jp::Fill::Linear), int(want.gradType == jp::Fill::Linear));
                if (want.gradType == jp::Fill::Linear)
                    QVERIFY2(angleDiff(sh->fill.angle, want.angle) < 0.5, qPrintable(QStringLiteral("%1 -> %2").arg(want.angle).arg(sh->fill.angle)));
                const QVector<jp::GradientStop> ws = want.stops.isEmpty()
                    ? QVector<jp::GradientStop>{{0, want.color, 0}, {1, want.color2, 0}} : want.stops;
                QCOMPARE(sh->fill.stops.size(), ws.size());
                for (int k = 0; k < ws.size(); ++k) {
                    QVERIFY(std::abs(sh->fill.stops[k].pos - ws[k].pos) < 0.011);
                    QCOMPARE(sh->fill.stops[k].color.resolve(back->colors).rgb(), ws[k].color.resolve(doc->colors).rgb());
                    QVERIFY2(std::abs(sh->fill.stops[k].transparency - ws[k].transparency) < 0.011, qPrintable(cases[i].label));
                }
            } else {
                QVERIFY(std::abs(sh->fill.transparency - want.transparency) < 0.01);
            }
            QVERIFY2(std::abs(sh->stroke.transparency - cases[i].stroke.transparency) < 0.01, qPrintable(cases[i].label));
            QCOMPARE(sh->fx.shadow.on, cases[i].shadow);
            if (cases[i].shadow) {
                QVERIFY(std::abs(sh->fx.shadow.distance - 6) < 0.1);
                QVERIFY(angleDiff(sh->fx.shadow.angle, 45) < 0.5);
                QVERIFY(std::abs(sh->fx.shadow.transparency - 0.5) < 0.01);
            }
        }
    }

    // Picture settings and pictures cut to a shape survive .pub.
    void pubWriterPictures()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto label = [&](const QRectF &r, const QString &s) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = r;
            t->storyId = doc->createStory(s);
            doc->pages[0]->items.push_back(t);
        };
        label(QRectF(54, 30, 500, 30), QStringLiteral("test27 pictures"));
        QImage photo(120, 90, QImage::Format_RGB32);
        for (int y = 0; y < photo.height(); ++y)
            for (int x = 0; x < photo.width(); ++x) photo.setPixel(x, y, qRgb(x * 2, y * 2, 255 - x));
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        photo.save(&buf, "PNG");
        const QString id = doc->addImage(png, QStringLiteral("png"));
        struct Case { QString label; std::function<void(jp::PictureItem &)> set; };
        const QVector<Case> cases = {
            {QStringLiteral("as is"), [](jp::PictureItem &) {}},
            {QStringLiteral("brighter, less contrast"), [](jp::PictureItem &p) { p.brightness = 30; p.contrast = -20; }},
            {QStringLiteral("darker, more contrast"), [](jp::PictureItem &p) { p.brightness = -25; p.contrast = 40; }},
            {QStringLiteral("grayscale"), [](jp::PictureItem &p) { p.recolor = jp::PictureItem::Grayscale; }},
            {QStringLiteral("black and white"), [](jp::PictureItem &p) { p.recolor = jp::PictureItem::BlackWhite; }},
            {QStringLiteral("washout"), [](jp::PictureItem &p) { p.recolor = jp::PictureItem::Washout; }},
            {QStringLiteral("sepia"), [](jp::PictureItem &p) { p.recolor = jp::PictureItem::Sepia; }},
            {QStringLiteral("recolored red"), [](jp::PictureItem &p) { p.recolor = jp::PictureItem::ColorTint; p.recolorColor = jp::ColorRef::rgb(QColor(200, 0, 0)); }},
            {QStringLiteral("clear color"), [](jp::PictureItem &p) { p.hasTransparentColor = true; p.transparentColor = QColor(0, 0, 255); }},
            {QStringLiteral("oval"), [](jp::PictureItem &p) { p.maskShape = QStringLiteral("ellipse"); }},
            {QStringLiteral("triangle, cropped"), [](jp::PictureItem &p) { p.maskShape = QStringLiteral("triangle"); p.imgRect = QRectF(-30, -20, 160, 120); }},
            {QStringLiteral("hexagon, gray, turned"), [](jp::PictureItem &p) { p.maskShape = QStringLiteral("hexagon"); p.recolor = jp::PictureItem::Grayscale; p.rotation = 20; }},
            {QStringLiteral("cropped"), [](jp::PictureItem &p) { p.imgRect = QRectF(-25, -10, 150, 100); }},
            {QStringLiteral("40% see-through"), [](jp::PictureItem &p) { p.transparency = 0.4; }},
        };
        QVector<std::shared_ptr<jp::PictureItem>> made;
        for (int i = 0; i < cases.size(); ++i) {
            auto pic = std::make_shared<jp::PictureItem>();
            pic->imageId = id;
            pic->rect = QRectF(60 + (i % 4) * 130, 80 + (i / 4) * 150, 100, 75);
            pic->imgRect = QRectF(0, 0, 100, 75);
            cases[i].set(*pic);
            doc->pages[0]->items.push_back(pic);
            made << pic;
            label(QRectF(pic->rect.left(), pic->rect.bottom() + 10, 125, 24), cases[i].label);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("test27-pictures.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test27-pictures";
            QFile::remove(out + ".pub");
            QFile::copy(path, out + ".pub");
            QString e2;
            jp::savePublication(*doc, out + ".jpub", QImage(), &e2);
        }
        QString e1;
        auto back = jp::importPublisherFile(path, &e1);
        QVERIFY2(back, qPrintable(e1));
        QVector<const jp::PictureItem *> got;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Picture) got << static_cast<const jp::PictureItem *>(it.get());
        });
        QCOMPARE(got.size(), made.size());
        for (int i = 0; i < made.size(); ++i) {
            const jp::PictureItem *want = made[i].get();
            const jp::PictureItem *g = nullptr;
            for (const jp::PictureItem *c : got)
                if (QLineF(c->rect.center(), want->rect.center()).length() < 1.5) g = c;
            QVERIFY2(g, qPrintable(cases[i].label));
            QVERIFY2(std::abs(g->rect.width() - want->rect.width()) < 1 && std::abs(g->rect.height() - want->rect.height()) < 1, qPrintable(cases[i].label));
            QVERIFY2(std::abs(std::remainder(g->rotation - want->rotation, 360.0)) < 0.5, qPrintable(cases[i].label));
            QCOMPARE(g->maskShape, want->maskShape);
            if (want->maskShape != QLatin1String("rect")) {
                // Cut out with its settings applied: the picture shows as it did.
                const QImage a = jp::Renderer::processedImage(*doc, *want, QSizeF());
                const QImage shown = back->image(g->imageId);
                QVERIFY(!shown.isNull());
                const QRectF ir = want->imgRect;
                const QPointF probe(want->rect.width() * 0.5, want->rect.height() * 0.6);
                const QColor wa = a.pixelColor(int((probe.x() - ir.left()) / ir.width() * a.width()), int((probe.y() - ir.top()) / ir.height() * a.height()));
                const QColor gb = shown.pixelColor(int(probe.x() / want->rect.width() * shown.width()), int(probe.y() / want->rect.height() * shown.height()));
                QVERIFY2(std::abs(wa.red() - gb.red()) < 12 && std::abs(wa.green() - gb.green()) < 12 && std::abs(wa.blue() - gb.blue()) < 12,
                         qPrintable(QStringLiteral("%1: %2 vs %3").arg(cases[i].label, wa.name(), gb.name())));
                continue;
            }
            QVERIFY2(std::abs(g->transparency - want->transparency) < 0.01, qPrintable(cases[i].label));
            if (want->transparency < 0.001)
                QVERIFY2(QLineF(g->imgRect.topLeft(), want->imgRect.topLeft()).length() < 0.5 && std::abs(g->imgRect.width() - want->imgRect.width()) < 0.5
                             && std::abs(g->imgRect.height() - want->imgRect.height()) < 0.5,
                         qPrintable(QStringLiteral("%1: %2,%3 %4x%5").arg(cases[i].label).arg(g->imgRect.x()).arg(g->imgRect.y()).arg(g->imgRect.width()).arg(g->imgRect.height())));
            QVERIFY2(std::abs(g->brightness - want->brightness) < 0.05, qPrintable(cases[i].label));
            QVERIFY2(std::abs(g->contrast - want->contrast) < 0.05, qPrintable(QStringLiteral("%1 %2").arg(cases[i].label).arg(g->contrast)));
            QCOMPARE(int(g->recolor), int(want->recolor));
            if (want->recolor == jp::PictureItem::ColorTint) QCOMPARE(g->recolorColor.resolve(back->colors), want->recolorColor.resolve(doc->colors));
            // Gray and black and white pictures are saved already converted,
            // since the other program shows the saved colors.
            if (want->recolor == jp::PictureItem::Grayscale || want->recolor == jp::PictureItem::BlackWhite) {
                const QImage saved = back->image(g->imageId);
                QVERIFY(!saved.isNull());
                for (int y = 0; y < saved.height(); y += 7)
                    for (int x = 0; x < saved.width(); x += 7) {
                        const QColor px = saved.pixelColor(x, y);
                        QVERIFY2(std::abs(px.red() - px.green()) <= 2 && std::abs(px.green() - px.blue()) <= 2, qPrintable(cases[i].label));
                        if (want->recolor == jp::PictureItem::BlackWhite) QVERIFY2(px.red() <= 2 || px.red() >= 253, qPrintable(cases[i].label));
                    }
            }
            QCOMPARE(g->hasTransparentColor, want->hasTransparentColor);
            if (want->hasTransparentColor) QCOMPARE(g->transparentColor, want->transparentColor);
        }
    }

    // A chain of linked text boxes (across two pages) is one story in .pub.
    void pubWriterLinked()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->addPage();
        QString text = QStringLiteral("test14 linked boxes.");
        for (int i = 1; i <= 60; ++i) text += QStringLiteral(" Sentence %1 flows on through the linked boxes.").arg(i);
        const QString story = doc->createStory(text);
        const QRectF rects[3] = {QRectF(72, 72, 220, 150), QRectF(320, 72, 220, 150), QRectF(72, 72, 220, 200)};
        QVector<std::shared_ptr<jp::TextItem>> boxes;
        for (int i = 0; i < 3; ++i) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = rects[i];
            t->storyId = story;
            if (i) boxes.last()->nextId = t->id;
            boxes << t;
            doc->pages[i < 2 ? 0 : 1]->items.push_back(t);
        }
        QTemporaryDir dir;
        // Also two linked boxes on one page, the simplest chain.
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            auto two = jp::Document::blank(QSizeF(612, 792));
            const QString st = two->createStory(QStringLiteral("test15 two linked boxes.") + text.mid(20));
            auto a = std::make_shared<jp::TextItem>(), b = std::make_shared<jp::TextItem>();
            a->rect = rects[0];
            b->rect = rects[1];
            a->storyId = b->storyId = st;
            a->nextId = b->id;
            two->pages[0]->items.push_back(a);
            two->pages[0]->items.push_back(b);
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test15-linked-one-page.pub";
            QFile::remove(out);
            QString e;
            QVERIFY(jp::exportPublisher(*two, out, &e));
        }
        const QString path = dir.filePath(QStringLiteral("test14-linked.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test14-linked.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
        QString e1;
        auto back = jp::importPublisherFile(path, &e1);
        QVERIFY2(back, qPrintable(e1));
        QVector<jp::TextItem *> got;
        for (int pi = 0; pi < back->pages.size(); ++pi)
            jp::walkItems(back->pages[pi]->items, [&](const jp::ItemPtr &it) {
                if (it->type() == jp::ItemType::Text) got << static_cast<jp::TextItem *>(it.get());
            });
        QCOMPARE(got.size(), 3);
        // One chain, in order, holding the whole text.
        QVERIFY(!back->prevFrame(got[0]->id));
        QCOMPARE(got[0]->nextId, got[1]->id);
        QCOMPARE(got[1]->nextId, got[2]->id);
        QVERIFY(got[2]->nextId.isEmpty());
        QCOMPARE(got[1]->storyId, got[0]->storyId);
        QCOMPARE(back->storyDoc(got[0]->storyId)->toPlainText(), text);
        for (int i = 0; i < 3; ++i) QVERIFY(std::abs(got[i]->rect.left() - rects[i].left()) < 1 && std::abs(got[i]->rect.height() - rects[i].height()) < 1);
    }

    // Character effects, indents, spacing and line spacing written to .pub
    // read back as they were.
    void pubWriterTextFormats()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 468, 500);
        t->storyId = doc->createStory();
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            QTextCharFormat plain;
            plain.setFontFamilies(QStringList{QStringLiteral("Arimo")});
            plain.setFontPointSize(12);
            QTextBlockFormat b0;
            b0.setBottomMargin(0);
            c.setBlockFormat(b0);
            c.insertText(QStringLiteral("test24 "), plain);
            QTextCharFormat u = plain;
            u.setFontUnderline(true);
            c.insertText(QStringLiteral("underline"), u);
            c.insertText(QStringLiteral(" "), plain);
            QTextCharFormat sc = plain;
            sc.setFontCapitalization(QFont::SmallCaps);
            c.insertText(QStringLiteral("smallcaps"), sc);
            c.insertText(QStringLiteral(" "), plain);
            QTextCharFormat ac = plain;
            ac.setFontCapitalization(QFont::AllUppercase);
            c.insertText(QStringLiteral("allcaps"), ac);
            c.insertText(QStringLiteral(" x"), plain);
            QTextCharFormat sup = plain;
            sup.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
            c.insertText(QStringLiteral("2"), sup);
            c.insertText(QStringLiteral(" H"), plain);
            QTextCharFormat sub = plain;
            sub.setVerticalAlignment(QTextCharFormat::AlignSubScript);
            c.insertText(QStringLiteral("2"), sub);
            c.insertText(QStringLiteral("O"), plain);
            QTextBlockFormat ind;
            ind.setLeftMargin(72);
            ind.setTextIndent(-18);
            ind.setRightMargin(36);
            ind.setTopMargin(12);
            ind.setBottomMargin(0);
            c.insertBlock(ind, plain);
            c.insertText(QStringLiteral("Indented one inch with a hanging first line and a half inch on the right, after twelve points."));
            QTextBlockFormat dbl;
            dbl.setLineHeight(200, QTextBlockFormat::ProportionalHeight);
            dbl.setBottomMargin(0);
            c.insertBlock(dbl, plain);
            c.insertText(QStringLiteral("Double spaced. Double spaced. Double spaced. Double spaced. Double spaced. Double spaced."));
            QTextBlockFormat fixed;
            fixed.setLineHeight(24, QTextBlockFormat::FixedHeight);
            fixed.setBottomMargin(0);
            c.insertBlock(fixed, plain);
            c.insertText(QStringLiteral("Exactly 24 points between lines. Exactly 24 points between lines. Exactly 24 points."));
        }
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("test24-formats.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test24-formats";
            QFile::remove(out + ".pub");
            QFile::copy(path, out + ".pub");
            QString e2;
            jp::savePublication(*doc, out + ".jpub", QImage(), &e2);
        }
        QString e1;
        auto back = jp::importPublisherFile(path, &e1);
        QVERIFY2(back, qPrintable(e1));
        const jp::TextItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Text) bt = static_cast<const jp::TextItem *>(it.get());
        });
        QVERIFY(bt);
        QTextDocument *d = back->storyDoc(bt->storyId);
        QCOMPARE(d->blockCount(), 4);
        QHash<QString, QTextCharFormat> fmt;
        for (auto it = d->begin().begin(); !it.atEnd(); ++it) fmt[it.fragment().text().trimmed()] = it.fragment().charFormat();
        QVERIFY(fmt.value(QStringLiteral("underline")).fontUnderline());
        QCOMPARE(fmt.value(QStringLiteral("smallcaps")).fontCapitalization(), QFont::SmallCaps);
        QCOMPARE(fmt.value(QStringLiteral("allcaps")).fontCapitalization(), QFont::AllUppercase);
        QCOMPARE(fmt.value(QStringLiteral("2")).verticalAlignment() == QTextCharFormat::AlignSuperScript ||
                     fmt.value(QStringLiteral("2")).verticalAlignment() == QTextCharFormat::AlignSubScript, true);
        int supers = 0, subs = 0;
        for (auto it = d->begin().begin(); !it.atEnd(); ++it) {
            if (it.fragment().text() != QStringLiteral("2")) continue;
            supers += it.fragment().charFormat().verticalAlignment() == QTextCharFormat::AlignSuperScript;
            subs += it.fragment().charFormat().verticalAlignment() == QTextCharFormat::AlignSubScript;
        }
        QCOMPARE(supers, 1);
        QCOMPARE(subs, 1);
        const QTextBlockFormat bi = d->begin().next().blockFormat();
        QVERIFY2(std::abs(bi.leftMargin() - 72) < 0.5 && std::abs(bi.textIndent() + 18) < 0.5 && std::abs(bi.rightMargin() - 36) < 0.5,
                 qPrintable(QStringLiteral("%1 %2 %3").arg(bi.leftMargin()).arg(bi.textIndent()).arg(bi.rightMargin())));
        QVERIFY(std::abs(bi.topMargin() - 12) < 0.5);
        QVERIFY(std::abs(d->begin().blockFormat().bottomMargin()) < 0.01);
        const QTextBlockFormat bd = d->begin().next().next().blockFormat();
        QCOMPARE(bd.lineHeightType(), int(QTextBlockFormat::ProportionalHeight));
        QVERIFY2(std::abs(bd.lineHeight() - 200) < 1, qPrintable(QString::number(bd.lineHeight())));
        const QTextBlockFormat bf = d->lastBlock().blockFormat();
        QCOMPARE(bf.lineHeightType(), int(QTextBlockFormat::FixedHeight));
        QVERIFY2(std::abs(bf.lineHeight() - 24) < 0.1, qPrintable(QString::number(bf.lineHeight())));
    }

    // Strikethrough, letter spacing, tab stops and lists written to .pub read back.
    void pubWriterTabsLists()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 468, 400);
        t->storyId = doc->createStory();
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            QTextCharFormat plain;
            plain.setFontFamilies(QStringList{QStringLiteral("Arimo")});
            plain.setFontPointSize(12);
            c.insertText(QStringLiteral("test25 "), plain);
            QTextCharFormat st = plain;
            st.setFontStrikeOut(true);
            c.insertText(QStringLiteral("strike"), st);
            c.insertText(QStringLiteral(" "), plain);
            QTextCharFormat tr = plain;
            tr.setFontLetterSpacingType(QFont::PercentageSpacing);
            tr.setFontLetterSpacing(125);
            c.insertText(QStringLiteral("tracked"), tr);
            c.insertText(QStringLiteral(" "), plain);
            QTextCharFormat kn = plain;
            kn.setFontLetterSpacingType(QFont::AbsoluteSpacing);
            kn.setFontLetterSpacing(2);
            c.insertText(QStringLiteral("kerned"), kn);
            c.insertText(QStringLiteral(" "), plain);
            QTextCharFormat both = plain;
            both.setProperty(jp::tp::Tracking, 87.5);
            both.setFontLetterSpacingType(QFont::AbsoluteSpacing);
            both.setFontLetterSpacing(-1);
            c.insertText(QStringLiteral("both"), both);
            QTextBlockFormat tabs;
            QList<QTextOption::Tab> tl;
            tl << QTextOption::Tab(72, QTextOption::LeftTab) << QTextOption::Tab(216, QTextOption::CenterTab) << QTextOption::Tab(360, QTextOption::RightTab);
            tabs.setTabPositions(tl);
            tabs.setProperty(jp::tp::TabLeaders, QStringLiteral(" .-"));   // none, dots, dashes
            c.insertBlock(tabs, plain);
            c.insertText(QStringLiteral("\tleft\tcenter\tright"));
            QTextBlockFormat item;
            item.setLeftMargin(18);
            item.setTextIndent(-18);
            QTextListFormat bl;
            bl.setStyle(QTextListFormat::ListDisc);
            bl.setIndent(0);
            c.insertBlock(item, plain);
            c.insertText(QStringLiteral("bullet one"));
            QTextList *bullets = c.createList(bl);
            c.insertBlock(item, plain);
            c.insertText(QStringLiteral("bullet two"));
            bullets->add(c.block());
            QTextListFormat nl;
            nl.setStyle(QTextListFormat::ListUpperRoman);
            nl.setNumberSuffix(QStringLiteral("."));
            nl.setIndent(0);
            c.insertBlock(item, plain);
            c.insertText(QStringLiteral("number one"));
            QTextList *numbers = c.createList(nl);
            c.insertBlock(item, plain);
            c.insertText(QStringLiteral("number two"));
            numbers->add(c.block());
        }
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("test25-tabs-lists.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test25-tabs-lists";
            QFile::remove(out + ".pub");
            QFile::copy(path, out + ".pub");
            QString e2;
            jp::savePublication(*doc, out + ".jpub", QImage(), &e2);
        }
        QString e1;
        auto back = jp::importPublisherFile(path, &e1);
        QVERIFY2(back, qPrintable(e1));
        const jp::TextItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Text) bt = static_cast<const jp::TextItem *>(it.get());
        });
        QVERIFY(bt);
        QTextDocument *d = back->storyDoc(bt->storyId);
        QCOMPARE(d->blockCount(), 6);
        QHash<QString, QTextCharFormat> fmt;
        for (auto it = d->begin().begin(); !it.atEnd(); ++it) fmt[it.fragment().text().trimmed()] = it.fragment().charFormat();
        QVERIFY(fmt.value(QStringLiteral("strike")).fontStrikeOut());
        // Tracking (a percentage) and kerning (points) come back apart, and
        // a run can have both.
        QVERIFY(std::abs(jp::tp::trackingOf(fmt.value(QStringLiteral("tracked"))) - 125) < 0.5);
        QVERIFY(std::abs(jp::tp::kerningOf(fmt.value(QStringLiteral("tracked")))) < 0.001);
        QVERIFY(std::abs(jp::tp::kerningOf(fmt.value(QStringLiteral("kerned"))) - 2) < 0.05);
        QVERIFY(std::abs(jp::tp::trackingOf(fmt.value(QStringLiteral("kerned"))) - 100) < 0.01);
        QVERIFY(std::abs(jp::tp::trackingOf(fmt.value(QStringLiteral("both"))) - 87.5) < 0.5);
        QVERIFY(std::abs(jp::tp::kerningOf(fmt.value(QStringLiteral("both"))) + 1) < 0.05);
        const QList<QTextOption::Tab> got = d->begin().next().blockFormat().tabPositions();
        QCOMPARE(got.size(), 3);
        QVERIFY(std::abs(got[1].position - 216) < 0.5 && got[1].type == QTextOption::CenterTab);
        // Leaders: none before the left stop, dots before center, dashes before right.
        QCOMPARE(d->begin().next().blockFormat().stringProperty(jp::tp::TabLeaders), QStringLiteral(" .-"));
        QVERIFY(std::abs(got[2].position - 360) < 0.5 && got[2].type == QTextOption::RightTab);
        const QTextBlock b1 = d->begin().next().next(), b2 = b1.next(), n1 = b2.next(), n2 = n1.next();
        QVERIFY(b1.textList() && b1.textList() == b2.textList());
        QCOMPARE(b1.textList()->format().style(), QTextListFormat::ListDisc);
        QVERIFY(n1.textList() && n1.textList() == n2.textList() && n1.textList() != b1.textList());
        QCOMPARE(n1.textList()->format().style(), QTextListFormat::ListUpperRoman);
        QCOMPARE(n1.textList()->format().numberSuffix(), QStringLiteral("."));
        QVERIFY(std::abs(n2.blockFormat().leftMargin() - 18) < 0.5 && std::abs(n2.blockFormat().textIndent() + 18) < 0.5);
    }

    // Objects on the master page are saved there and come back there, once.
    void pubWriterMaster()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 2);
        QVERIFY(!doc->masters.isEmpty());
        auto mt = std::make_shared<jp::TextItem>();
        mt->rect = QRectF(72, 700, 300, 40);
        mt->storyId = doc->createStory(QStringLiteral("test23 master page footer"));
        doc->masters.first()->items.push_back(mt);
        auto bar = std::make_shared<jp::ShapeItem>();
        bar->rect = QRectF(72, 690, 468, 6);
        bar->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(30, 60, 200)));
        bar->stroke = jp::Stroke::none();
        doc->masters.first()->items.push_back(bar);
        for (int i = 0; i < 2; ++i) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(72, 72, 300, 40);
            t->storyId = doc->createStory(QStringLiteral("Page %1 text").arg(i + 1));
            doc->pages[i]->items.push_back(t);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("test23-master.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test23-master.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
        QString e1;
        auto back = jp::importPublisherFile(path, &e1);
        QVERIFY2(back, qPrintable(e1));
        QCOMPARE(back->pages.size(), 2);
        const jp::MasterPage *m = back->master(QStringLiteral("A"));
        QVERIFY(m);
        QCOMPARE(int(m->items.size()), 2);
        bool footer = false;
        jp::walkItems(m->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Text)
                footer = back->storyDoc(static_cast<const jp::TextItem *>(it.get())->storyId)->toPlainText() == QStringLiteral("test23 master page footer");
        });
        QVERIFY(footer);
        for (int i = 0; i < 2; ++i) {
            QCOMPARE(back->pages[i]->masterId, QStringLiteral("A"));
            QCOMPARE(int(back->pages[i]->items.size()), 1);
        }
    }

    // A story's automatic hyphenation setting survives .pub.
    void pubWriterHyphenation()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        for (int i = 0; i < 2; ++i) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(72, 72 + i * 100, 300, 60);
            t->storyId = doc->createStory(i ? QStringLiteral("not hyphenated") : QStringLiteral("hyphenated"));
            t->hyphenate = i == 0;
            doc->pages[0]->items.push_back(t);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("hyph.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test29-hyphenation.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        int checked = 0;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() != jp::ItemType::Text) return;
            const auto *t = static_cast<const jp::TextItem *>(it.get());
            QCOMPARE(t->hyphenate, back->storyDoc(t->storyId)->toPlainText() == QStringLiteral("hyphenated"));
            ++checked;
        });
        QCOMPARE(checked, 2);
    }

    // Named paragraph styles: the style sheet keeps them, and paragraphs keep
    // which one they use.
    void pubWriterStyles()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        jp::TextStyle pq;
        pq.name = QStringLiteral("Pull Quote");
        pq.basedOn = QStringLiteral("Normal");
        pq.next = QStringLiteral("Normal");
        pq.chr.setFontPointSize(16);
        pq.chr.setFontWeight(QFont::Bold);
        pq.chr.setFontItalic(true);
        pq.chr.setFontFamilies(QStringList{QStringLiteral("Georgia")});
        pq.blk.setAlignment(Qt::AlignHCenter);
        pq.blk.setTopMargin(12);
        pq.blk.setBottomMargin(6);
        pq.blk.setProperty(jp::tp::StyleName, pq.name);
        doc->styles << pq;
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 400, 300);
        t->storyId = doc->createStory(QString());
        doc->pages[0]->items.push_back(t);
        QTextDocument *d = doc->storyDoc(t->storyId);
        QTextCursor c(d);
        auto para = [&](const QString &text, const QString &style, bool first) {
            const jp::TextStyle *st = doc->style(style);
            QTextBlockFormat bf = st->blk;
            bf.setProperty(jp::tp::StyleName, style);
            if (first) c.setBlockFormat(bf);
            else c.insertBlock(bf);
            c.insertText(text, st->chr);
        };
        para(QStringLiteral("test30 A title"), QStringLiteral("Title"), true);
        para(QStringLiteral("Plain text in the Normal style."), QStringLiteral("Normal"), false);
        para(QStringLiteral("A pull quote, in its own style"), QStringLiteral("Pull Quote"), false);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("styles.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test30-styles.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        const jp::TextItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Text) bt = static_cast<const jp::TextItem *>(it.get());
        });
        QVERIFY(bt);
        QTextBlock b = back->storyDoc(bt->storyId)->begin();
        QCOMPARE(b.blockFormat().stringProperty(jp::tp::StyleName), QStringLiteral("Title"));
        QVERIFY(b.next().blockFormat().stringProperty(jp::tp::StyleName).isEmpty());
        QCOMPARE(b.next().next().blockFormat().stringProperty(jp::tp::StyleName), QStringLiteral("Pull Quote"));
        // Bold and italic are saved relative to the style, so they come back
        // as they were: the title and pull quote bold, the plain text not.
        auto firstFormat = [](const QTextBlock &blk) { return blk.begin().fragment().charFormat(); };
        QVERIFY(firstFormat(b).fontWeight() >= QFont::Bold);
        QVERIFY(firstFormat(b.next()).fontWeight() < QFont::DemiBold && !firstFormat(b.next()).fontItalic());
        QVERIFY(firstFormat(b.next().next()).fontWeight() >= QFont::Bold && firstFormat(b.next().next()).fontItalic());
        const jp::TextStyle *got = back->style(QStringLiteral("Pull Quote"));
        QVERIFY(got);
        QVERIFY(std::abs(got->chr.fontPointSize() - 16) < 0.01);
        QVERIFY(got->chr.fontWeight() >= QFont::Bold && got->chr.fontItalic());
        QCOMPARE(got->chr.fontFamilies().toStringList().value(0), QStringLiteral("Georgia"));
        QCOMPARE(got->blk.alignment() & Qt::AlignHorizontal_Mask, Qt::AlignHCenter);
        QVERIFY(std::abs(got->blk.topMargin() - 12) < 0.01);
    }

    // Two master pages: each page keeps its own master and its objects.
    void pubWriterTwoMasters()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792), QStringLiteral("Letter"), 3);
        auto b = std::make_shared<jp::MasterPage>();
        b->abbr = QStringLiteral("B");
        b->id = QStringLiteral("B");
        b->name = QStringLiteral("Chapter openers");
        doc->masters << b;
        auto footer = [&](jp::MasterPage *m, const QString &text, const QColor &c) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(72, 700, 300, 40);
            t->storyId = doc->createStory(text);
            m->items.push_back(t);
            auto bar = std::make_shared<jp::ShapeItem>();
            bar->rect = QRectF(72, 690, 468, 6);
            bar->fill = jp::Fill::solid(jp::ColorRef::rgb(c));
            bar->stroke = jp::Stroke::none();
            m->items.push_back(bar);
        };
        footer(doc->masters[0].get(), QStringLiteral("test28 master A (blue bar)"), QColor(30, 60, 200));
        footer(b.get(), QStringLiteral("test28 master B (red bar)"), QColor(200, 30, 30));
        const QStringList uses = {QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("A")};
        for (int i = 0; i < 3; ++i) {
            doc->pages[i]->masterId = uses[i];
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(72, 72, 400, 40);
            t->storyId = doc->createStory(QStringLiteral("Page %1 uses master %2").arg(i + 1).arg(uses[i]));
            doc->pages[i]->items.push_back(t);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("test28-masters.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));   // nothing left out
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test28-masters.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
        QString e1;
        auto back = jp::importPublisherFile(path, &e1);
        QVERIFY2(back, qPrintable(e1));
        QCOMPARE(back->pages.size(), 3);
        QCOMPARE(back->masters.size(), 2);
        for (int i = 0; i < 3; ++i) QCOMPARE(back->pages[i]->masterId, uses[i]);
        auto footerText = [&](const QString &id) {
            QString text;
            const jp::MasterPage *m = back->master(id);
            if (!m) return text;
            jp::walkItems(m->items, [&](const jp::ItemPtr &it) {
                if (it->type() == jp::ItemType::Text) text = back->storyDoc(static_cast<const jp::TextItem *>(it.get())->storyId)->toPlainText();
            });
            return text;
        };
        QCOMPARE(footerText(QStringLiteral("A")), QStringLiteral("test28 master A (blue bar)"));
        QCOMPARE(footerText(QStringLiteral("B")), QStringLiteral("test28 master B (red bar)"));
    }

    // Two pages written to .pub read back as two pages, each with its own objects.
    void pubWriterPages()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->addPage();
        QCOMPARE(doc->pages.size(), 2);
        for (int i = 0; i < 2; ++i) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(72, 72 + 100 * i, 300, 60);
            t->storyId = doc->createStory(i == 0 ? QStringLiteral("test13 page one") : QStringLiteral("Page two"));
            doc->pages[i]->items.push_back(t);
            auto sh = std::make_shared<jp::ShapeItem>();
            sh->shape = i == 0 ? QStringLiteral("rect") : QStringLiteral("ellipse");
            sh->rect = QRectF(100, 400, 150, 100);
            sh->fill = jp::Fill::solid(jp::ColorRef::rgb(i == 0 ? QColor(220, 30, 30) : QColor(30, 60, 200)));
            sh->stroke = jp::Stroke::none();
            doc->pages[i]->items.push_back(sh);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("test13-two-pages.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test13-two-pages.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
        QString e1;
        auto back = jp::importPublisherFile(path, &e1);
        QVERIFY2(back, qPrintable(e1));
        QCOMPARE(back->pages.size(), 2);
        for (int i = 0; i < 2; ++i) {
            QString text;
            int shapes = 0;
            jp::walkItems(back->pages[i]->items, [&](const jp::ItemPtr &it) {
                if (it->type() == jp::ItemType::Text) {
                    const QTextDocument *d = back->storyDoc(static_cast<const jp::TextItem *>(it.get())->storyId);
                    if (d) text = d->toPlainText();
                }
                if (it->type() == jp::ItemType::Shape) ++shapes;
            });
            QCOMPARE(text, i == 0 ? QStringLiteral("test13 page one") : QStringLiteral("Page two"));
            QCOMPARE(shapes, 1);
        }
    }

    // A table written to .pub reads back with its grid, text, merge, fills and rules.
    void pubWriterTable()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 3;
        t->cols = 2;
        t->colW = {100, 150};
        t->rowH = {30, 40, 50};
        t->rect = QRectF(72, 144, 0, 0);
        t->syncRect();
        t->cells.resize(6);
        const QStringList texts = {QStringLiteral("Name"), QStringLiteral("Amount"), QStringLiteral("Paper"), QStringLiteral("$5.00"),
                                   QStringLiteral("Total due"), QString()};
        for (int i = 0; i < 6; ++i) t->cells[i].storyId = doc->createStory(texts[i]);
        const QColor gold(255, 230, 120), blue(0, 0, 200);
        for (int c = 0; c < 2; ++c) {
            t->cell(0, c).fill = jp::Fill::solid(jp::ColorRef::rgb(gold));
            t->cell(0, c).border.bottom = jp::Stroke::line(jp::ColorRef::rgb(Qt::black), 1.5);
        }
        t->cell(1, 1).border.left = jp::Stroke::line(jp::ColorRef::rgb(blue), 1);
        t->cell(2, 0).colSpan = 2;
        t->cell(2, 1).covered = true;
        doc->pages[0]->items.push_back(t);

        // The table alone, then with a text box too (two stories).
        QTemporaryDir dir;
        QString path;
        for (const QString &name : {QStringLiteral("test10-table.pub"), QStringLiteral("test11-table-text.pub")}) {
            if (name.startsWith(QLatin1String("test11"))) {
                auto label = std::make_shared<jp::TextItem>();
                label->rect = QRectF(72, 36, 400, 40);
                label->storyId = doc->createStory(QStringLiteral("test11-table-text"));
                doc->pages[0]->items.insert(doc->pages[0]->items.begin(), label);
            }
            path = dir.filePath(name);
            QString err;
            QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
            QVERIFY2(err.isEmpty(), qPrintable(err));
            if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
                const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/" + name;
                QFile::remove(out);
                QFile::copy(path, out);
            }
        }
        QString e1;
        auto back = jp::importPublisherFile(path, &e1);
        QVERIFY2(back, qPrintable(e1));
        const jp::TableItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Table) bt = static_cast<const jp::TableItem *>(it.get());
        });
        QVERIFY(bt);
        QCOMPARE(bt->rows, 3);
        QCOMPARE(bt->cols, 2);
        QVERIFY2(std::abs(bt->rect.left() - 72) < 1 && std::abs(bt->rect.top() - 144) < 1,
                 qPrintable(QStringLiteral("%1,%2").arg(bt->rect.left()).arg(bt->rect.top())));
        QVERIFY(std::abs(bt->colW.value(0) - 100) < 1 && std::abs(bt->colW.value(1) - 150) < 1);
        for (int r = 0; r < 3; ++r) QVERIFY2(std::abs(bt->rowH.value(r) - t->rowH[r]) < 1, qPrintable(QString::number(bt->rowH.value(r))));
        auto text = [&](int r, int c) {
            const QTextDocument *d = back->storyDoc(bt->cell(r, c).storyId);
            return d ? d->toPlainText() : QString();
        };
        QCOMPARE(text(0, 0), QStringLiteral("Name"));
        QCOMPARE(text(0, 1), QStringLiteral("Amount"));
        QCOMPARE(text(1, 1), QStringLiteral("$5.00"));
        QCOMPARE(text(2, 0), QStringLiteral("Total due"));
        QCOMPARE(bt->cell(2, 0).colSpan, 2);
        QCOMPARE(bt->cell(0, 1).fill.color.resolve(back->colors), gold);
        QVERIFY(bt->cell(1, 0).fill.type != jp::Fill::Solid);
        const jp::Stroke under = bt->cell(0, 0).border.bottom;
        QVERIFY(!under.isNone() && std::abs(under.width - 1.5) < 0.1);
        QCOMPARE(under.color.resolve(back->colors), QColor(Qt::black));
        QVERIFY(!bt->cell(1, 1).border.left.isNone());
        QCOMPARE(bt->cell(1, 1).border.left.color.resolve(back->colors), blue);
        QVERIFY(bt->cell(1, 0).border.left.isNone());
    }

    // Line weights and a list's indent are written as 32-bit lengths, so a damaged huge one stops at 1584 points
    // (the largest weight Publisher offers) instead of wrapping around to something small or nothing.
    void pubLineWeightsAndListIndentsDoNotWrap()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto line = std::make_shared<jp::LineItem>();
        line->p1 = QPointF(72, 72);
        line->p2 = QPointF(300, 72);
        line->stroke = jp::Stroke::line(jp::ColorRef::rgb(Qt::black), 1e300);
        doc->pages[0]->items.push_back(line);
        auto box = std::make_shared<jp::TextItem>();
        box->rect = QRectF(72, 100, 300, 100);
        box->storyId = doc->createStory();
        QTextCursor tc(doc->storyDoc(box->storyId));
        tc.insertText(QStringLiteral("More details"));
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        lf.setProperty(jp::tp::BulletTab, 1e300);
        tc.createList(lf);
        doc->pages[0]->items.push_back(box);
        auto t = std::make_shared<jp::TableItem>();
        t->rows = 1;
        t->cols = 2;
        t->colW = {100, 100};
        t->rowH = {40};
        t->rect = QRectF(72, 300, 0, 0);
        t->syncRect();
        t->cells.resize(2);
        t->cell(0, 0).border.right = jp::Stroke::line(jp::ColorRef::rgb(Qt::black), 400000);
        t->cell(0, 1).diagonal = 1;
        t->cell(0, 1).diagonalLine = jp::Stroke::line(jp::ColorRef::rgb(Qt::red), 1e300);
        doc->pages[0]->items.push_back(t);

        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("huge-lengths.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        const jp::LineItem *bl = nullptr;
        const jp::TextItem *bx = nullptr;
        const jp::TableItem *bt = nullptr;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Line) bl = static_cast<const jp::LineItem *>(it.get());
            if (it->type() == jp::ItemType::Text) bx = static_cast<const jp::TextItem *>(it.get());
            if (it->type() == jp::ItemType::Table) bt = static_cast<const jp::TableItem *>(it.get());
        });
        QVERIFY(bl && bx && bt);
        QCOMPARE(bl->stroke.width, 1584.0);
        // The table's rule and diagonal weights (line width, property 0x01cb) are the same 1584 points in the drawing.
        QByteArray weight("\xcb\x01\0\0\0\0", 6);
        qToLittleEndian<quint32>(1584 * 12700, weight.data() + 2);
        QCOMPARE(jp::cfb::readStream(path, QStringLiteral("Escher/EscherStm")).count(weight), 3);
        const QTextBlock b = back->storyDoc(bx->storyId)->begin();
        QVERIFY(b.textList());
        QCOMPARE(b.textList()->format().property(jp::tp::BulletTab).toDouble(), 1584.0);
    }

    // Lines, turned shapes and pictures written to .pub read back in place.
    void pubWriterObjects()
    {
        auto save = [](const jp::Document &d, const QString &name, QTemporaryDir &dir) {
            const QString path = dir.filePath(name);
            QString err;
            if (!jp::exportPublisher(d, path, &err) || !err.isEmpty()) qWarning() << err;
            if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
                const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/" + name;
                QFile::remove(out);
                QFile::copy(path, out);
            }
            return path;
        };
        auto line = std::make_shared<jp::LineItem>();
        line->p1 = QPointF(100, 520);
        line->p2 = QPointF(400, 450);
        line->syncRect();
        line->stroke = jp::Stroke::line(jp::ColorRef::rgb(QColor(200, 0, 0)), 3);
        line->stroke.dash = jp::Stroke::DashLine;
        line->stroke.endArrow = jp::Arrow::Triangle;
        auto rect = std::make_shared<jp::ShapeItem>();
        rect->rect = QRectF(72, 100, 200, 80);
        rect->rotation = 30;
        rect->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(30, 160, 60)));
        rect->stroke = jp::Stroke::none();
        auto oval = std::make_shared<jp::ShapeItem>();
        oval->shape = QStringLiteral("ellipse");
        oval->rect = QRectF(350, 100, 160, 60);
        oval->rotation = 90;
        oval->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(30, 60, 200)));
        oval->stroke = jp::Stroke::none();
        QImage img(40, 20, QImage::Format_RGB32);
        img.fill(QColor(220, 30, 30));
        for (int y = 0; y < 20; ++y)
            for (int x = 20; x < 40; ++x) img.setPixelColor(x, y, QColor(30, 30, 220));
        QByteArray png;
        {
            QBuffer b(&png);
            b.open(QIODevice::WriteOnly);
            img.save(&b, "PNG");
        }

        QTemporaryDir dir;
        // Each object alone, with a label (text already opens in Publisher).
        auto one = [&](const jp::ItemPtr &it, const QString &name, std::function<void(jp::Document &)> prep = {}) {
            auto d = jp::Document::blank(QSizeF(612, 792));
            if (prep) prep(*d);
            auto label = std::make_shared<jp::TextItem>();
            label->rect = QRectF(72, 36, 400, 40);
            label->storyId = d->createStory();
            QTextCursor(d->storyDoc(label->storyId)).insertText(QFileInfo(name).completeBaseName());
            d->pages[0]->items.push_back(label);
            d->pages[0]->items.push_back(it->clone());
            return save(*d, name, dir);
        };
        one(line, QStringLiteral("test6-line.pub"));
        one(rect, QStringLiteral("test7-rotated.pub"));
        auto pic = std::make_shared<jp::PictureItem>();
        pic->rect = QRectF(100, 600, 160, 80);
        pic->imgRect = QRectF(0, 0, 160, 80);
        one(pic, QStringLiteral("test8-picture.pub"), [&](jp::Document &d) { pic->imageId = d.addImage(png, QStringLiteral("png"), QStringLiteral("/tmp/two-colors.png")); });

        auto doc = jp::Document::blank(QSizeF(612, 792));
        pic->imageId = doc->addImage(png, QStringLiteral("png"), QStringLiteral("/tmp/two-colors.png"));
        for (const jp::ItemPtr &it : jp::ItemList{line, rect, oval, pic}) doc->pages[0]->items.push_back(it);
        const QString path = save(*doc, QStringLiteral("test9-all.pub"), dir);
        QString e1;
        auto back = jp::importPublisherFile(path, &e1);
        QVERIFY2(back, qPrintable(e1));
        const jp::LineItem *bl = nullptr;
        const jp::PictureItem *bp = nullptr;
        QVector<const jp::ShapeItem *> shapes;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() == jp::ItemType::Line) bl = static_cast<const jp::LineItem *>(it.get());
            if (it->type() == jp::ItemType::Picture) bp = static_cast<const jp::PictureItem *>(it.get());
            if (it->type() == jp::ItemType::Shape) shapes << static_cast<const jp::ShapeItem *>(it.get());
        });
        auto near = [](QPointF a, QPointF b) { return QLineF(a, b).length() < 1.5; };
        QVERIFY(bl);
        QVERIFY2(near(bl->p1, line->p1) && near(bl->p2, line->p2),
                 qPrintable(QStringLiteral("%1,%2 %3,%4").arg(bl->p1.x()).arg(bl->p1.y()).arg(bl->p2.x()).arg(bl->p2.y())));
        QVERIFY(std::abs(bl->stroke.width - 3) < 0.1);
        QVERIFY(bl->stroke.dash != jp::Stroke::SolidLine);
        QCOMPARE(bl->stroke.color.resolve(back->colors), QColor(200, 0, 0));
        QVERIFY(bl->stroke.endArrow != jp::Arrow::None);
        QCOMPARE(bl->stroke.startArrow, jp::Arrow::None);
        QCOMPARE(shapes.size(), 2);
        // The rectangle and the oval come back turned, as themselves.
        for (const jp::ShapeItem *s : shapes) {
            const QString got = QStringLiteral("rot %1 %2,%3 %4x%5").arg(s->rotation).arg(s->rect.x()).arg(s->rect.y()).arg(s->rect.width()).arg(s->rect.height());
            QVERIFY2(s->customPath.isEmpty(), qPrintable(got));
            if (s->shape == QLatin1String("rect")) {
                QVERIFY2(std::abs(s->rotation - 30) < 0.5, qPrintable(got));
                QVERIFY2(QLineF(s->rect.center(), rect->rect.center()).length() < 1 && std::abs(s->rect.width() - 200) < 1 &&
                             std::abs(s->rect.height() - 80) < 1, qPrintable(got));
            } else {
                QCOMPARE(s->shape, QStringLiteral("ellipse"));
                QVERIFY2(std::abs(s->rotation - 90) < 0.5 && QLineF(s->rect.center(), oval->rect.center()).length() < 1
                             && std::abs(s->rect.width() - 160) < 1 && std::abs(s->rect.height() - 60) < 1, qPrintable(got));
            }
        }
        QVERIFY(bp);
        QVERIFY(std::abs(bp->rect.left() - 100) < 1 && std::abs(bp->rect.width() - 160) < 1 && std::abs(bp->rect.height() - 80) < 1);
        const QImage got = back->image(bp->imageId);
        QCOMPARE(got.size(), QSize(40, 20));
        QCOMPARE(got.pixelColor(5, 5), QColor(220, 30, 30));
        QCOMPARE(got.pixelColor(35, 5), QColor(30, 30, 220));
    }

    // How a .pub file packs process inks, checked against values Publisher
    // wrote (three reference files) and read (single inks, checked one at a
    // time in its color dialog).
    void pubInkPacking()
    {
        using P = QPair<quint32, quint32>;
        QCOMPARE(jp::packPubInks(QColor::fromCmykF(250 / 255.f, 194 / 255.f, 34 / 255.f, 77 / 255.f)), P(0x4585F5E8u, 308u));
        QCOMPARE(jp::packPubInks(QColor::fromCmykF(0.6f, 0.4f, 0.4f, 1.0f)), P(0x4CCD33E8u, 1021u));
        QCOMPARE(jp::packPubInks(QColor::fromCmykF(192 / 255.f, 0.4f, 0, 0)), P(0x00CD8188u, 0u));
        QCOMPARE(jp::packPubInks(QColor::fromCmykF(1, 0, 0, 0)), P(0x0001FF08u, 0u));   // read as C 100
        QCOMPARE(jp::packPubInks(QColor::fromCmykF(0, 1, 0, 0)), P(0x0001FE88u, 0u));   // read as M 100
        // Only the inks used are packed, in C M Y K order.
        QCOMPARE(jp::packPubInks(QColor::fromCmykF(0, 0, 1, 0)), P(0x0001FE48u, 0u));                // read as Y 100
        QCOMPARE(jp::packPubInks(QColor::fromCmykF(0, 0, 0, 1)), P(0x0001FE28u, 0u));                // read as K 100
        QCOMPARE(jp::packPubInks(QColor::fromCmykF(0, 128 / 255.f, 1, 51 / 255.f)), P(0x67FF00E8u, 0u));   // read as M 50 Y 100 K 20
    }

    // Character scaling (0x20, tenths of a percent, like tracking): a flyer's
    // motto at 100.1% came out 40 times as wide (read as 10010%). The test
    // file turns a saved tracking value into a scaling one in place.
    void pubTextScaleRead()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 400, 100);
        t->storyId = doc->createStory(QStringLiteral("Ever Vigilant Ever Ready"));
        doc->pages[0]->items.push_back(t);
        QTextCursor all(doc->storyDoc(t->storyId));
        all.select(QTextCursor::Document);
        QTextCharFormat tf;
        tf.setProperty(jp::tp::Tracking, 123.4);
        all.mergeCharFormat(tf);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("scale.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        jp::cfb::File c;
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::ReadOnly));
            QVERIFY2(jp::cfb::read(f.readAll(), &c, &err), qPrintable(err));
        }
        const QString quill = QStringLiteral("Quill/QuillSub/CONTENTS");
        QByteArray q = c.stream(quill);
        const QByteArray tracking("\x1f\x1a\xd2\x04", 4), scaling("\x20\x1a\xdc\x05", 4);   // 1234 -> 1500 (150%)
        QVERIFY(q.contains(tracking));
        q.replace(tracking, scaling);
        QVERIFY(c.setStream(quill, q));
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(jp::cfb::write(c));
        }
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QTextDocument *story = nullptr;
        for (const auto &it : back->pages[0]->items)
            if (auto tx = std::dynamic_pointer_cast<jp::TextItem>(it)) story = back->storyDoc(tx->storyId);
        QVERIFY(story);
        QTextCursor at(story);
        at.setPosition(3);
        QCOMPARE(at.charFormat().fontStretch(), 150);
    }

    // Saving keeps spot inks and character scaling: the fill names its ink
    // exactly as Publisher's covers do, and both come back on opening.
    void pubWriterSpotAndScaling()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        const QColor ink = QColor::fromCmyk(192, 102, 0, 0);
        doc->print.model = jp::PrintInfo::SpotColors;
        doc->print.spotColors = {ink};
        doc->print.spotNames = {QStringLiteral("PANTONE 2727 C")};
        auto box = std::make_shared<jp::ShapeItem>();
        box->rect = QRectF(72, 72, 300, 200);
        box->fill = jp::Fill::solid(jp::ColorRef::inks(ink, QColor(0x3d, 0x7e, 0xdb)));
        box->stroke = jp::Stroke::none();
        doc->pages[0]->items.push_back(box);
        auto text = [&](const QRectF &r, const QString &words, int stretch) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = r;
            t->storyId = doc->createStory(words);
            doc->pages[0]->items.push_back(t);
            if (stretch != 100) {
                QTextCursor all(doc->storyDoc(t->storyId));
                all.select(QTextCursor::Document);
                QTextCharFormat wide;
                wide.setFontStretch(stretch);
                all.mergeCharFormat(wide);
            }
        };
        text(QRectF(72, 400, 400, 40), QStringLiteral("Wide words"), 150);   // checked below
        // Labels for checking the file in Publisher.
        text(QRectF(72, 280, 400, 30), QStringLiteral("A: the box above is PANTONE 2727 C (a spot color)"), 100);
        text(QRectF(72, 440, 400, 30), QStringLiteral("B: the words above are scaled 150%"), 100);
        text(QRectF(72, 500, 400, 30), QStringLiteral("Narrow words"), 80);
        text(QRectF(72, 540, 400, 30), QStringLiteral("C: the words above are scaled 80%"), 100);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("spot-scale.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test33-spot-and-scaling.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
        jp::cfb::File c;
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::ReadOnly));
            QVERIFY2(jp::cfb::read(f.readAll(), &c, &err), qPrintable(err));
        }
        const QString name = QStringLiteral("P2,#003d007e00db0000,PANTONE 2727 C");
        QVERIFY(c.stream(QStringLiteral("Escher/EscherStm")).contains(QByteArray(reinterpret_cast<const char *>(name.utf16()), name.size() * 2)));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(back->print.usesSpots());
        QCOMPARE(back->print.spotNames, QStringList{QStringLiteral("PANTONE 2727 C")});
        QTextDocument *story = nullptr;
        for (const auto &it : back->pages[0]->items)
            if (auto tx = std::dynamic_pointer_cast<jp::TextItem>(it); tx && !story) story = back->storyDoc(tx->storyId);
        QVERIFY(story);
        QCOMPARE(story->toPlainText().trimmed(), QStringLiteral("Wide words"));
        QTextCursor at(story);
        at.setPosition(2);
        QCOMPARE(at.charFormat().fontStretch(), 150);
    }

    // A .pub saved by JeffPub carries a preview picture of page 1 as
    // Publisher writes one (a metafile copying in a 24-bit bitmap), which the
    // Open page then shows, and each run's language, which comes back.
    void pubPreviewAndLanguages()
    {
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto band = std::make_shared<jp::ShapeItem>();
        band->shape = QStringLiteral("rect");
        band->rect = QRectF(0, 0, 612, 300);
        band->fill = jp::Fill::solid(jp::ColorRef::rgb(QColor(200, 30, 40)));
        band->stroke = jp::Stroke::none();
        doc->pages[0]->items.push_back(band);
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 360, 468, 300);
        t->storyId = doc->createStory(QStringLiteral("This text is in English.\nEste texto está en español.\nCe texte est en français.\nDieser Text ist auf Deutsch."));
        doc->pages[0]->items.push_back(t);
        const char *langs[] = {"en-US", "es-MX", "fr-FR", "de-DE"};
        QTextBlock b = doc->storyDoc(t->storyId)->begin();
        for (const char *lang : langs) {
            QTextCursor c(b);
            c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            QTextCharFormat f;
            f.setProperty(jp::tp::Language, QString::fromLatin1(lang));
            f.setFontPointSize(24);
            c.mergeCharFormat(f);
            b = b.next();
        }
        w.editor()->setDocument(std::move(doc));
        const QImage thumb = w.pageThumbnail(0, 160);
        QCOMPARE(qMax(thumb.width(), thumb.height()), 160);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("preview.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*w.editor()->doc(), path, &err, thumb), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test35-preview-and-languages.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
        // Publisher's layout: property 17, a clipboard metafile picture whose
        // metafile fills white and copies in the bitmap.
        const QByteArray si = jp::cfb::readStream(path, QStringLiteral("\x05SummaryInformation"));
        const qsizetype at = si.indexOf(QByteArray::fromHex("47000000"));
        QVERIFY(at > 0);
        QCOMPARE(si.mid(at + 8, 10), QByteArray::fromHex("ffffffff030000000800"));
        const QByteArray wmf = si.mid(at + 24);
        QCOMPARE(wmf.left(6), QByteArray::fromHex("010009000003"));
        for (const char *rec : {"05000000 0b02", "05000000 0c02", "07000000 fc02", "04000000 2d01", "09000000 1d06", "4009", "03000000 0000"})
            QVERIFY2(wmf.contains(QByteArray::fromHex(QByteArray(rec).replace(' ', ""))), rec);
        // Read back as the Open page shows it: red at the top, white below.
        const QImage back = jp::publicationThumbnail(path);
        QVERIFY(!back.isNull());
        QVERIFY(qAbs(double(back.width()) / back.height() - double(thumb.width()) / thumb.height()) < 0.02);   // drawn at the Open page's size
        const QColor top = back.pixelColor(back.width() / 2, 5), low = back.pixelColor(5, back.height() - 5);
        QVERIFY2(top.red() > 180 && top.green() < 60, qPrintable(top.name()));
        QVERIFY2(low.lightness() > 240, qPrintable(low.name()));
        // Each run's language code in both of Publisher's places (Spanish
        // (Mexico) 0x080A; Publisher showed all four languages, test35, Oct 6).
        const QByteArray quill = jp::cfb::readStream(path, QStringLiteral("Quill/QuillSub/CONTENTS"));
        QVERIFY(quill.contains(QByteArray::fromHex("12220a080000")) && quill.contains(QByteArray::fromHex("3e220a080000")));
        QVERIFY(quill.contains(QByteArray::fromHex("12220c040000")) && quill.contains(QByteArray::fromHex("12220704" "0000")));
        // The languages, through the independent reader.
        auto reopened = jp::importPublisherFile(path, &err);
        QVERIFY2(reopened, qPrintable(err));
        QTextDocument *story = nullptr;
        for (const auto &it : reopened->pages[0]->items)
            if (auto tx = std::dynamic_pointer_cast<jp::TextItem>(it)) story = reopened->storyDoc(tx->storyId);
        QVERIFY(story);
        QStringList got;
        for (QTextBlock blk = story->begin(); blk.isValid(); blk = blk.next()) {
            QTextCursor c(blk);
            c.movePosition(QTextCursor::NextCharacter);
            got << c.charFormat().stringProperty(jp::tp::Language);
        }
        QCOMPARE(got.mid(0, 4), (QStringList{"en-US", "es-MX", "fr-FR", "de-DE"}));
        // A bigger picture is fitted to Publisher's size.
        QVERIFY(jp::exportPublisher(*w.editor()->doc(), path, &err, w.pageThumbnail(0, 2000)));
        const QImage fitted = jp::publicationThumbnail(path);
        QVERIFY(!fitted.isNull());
        const QByteArray si2 = jp::cfb::readStream(path, QStringLiteral("\x05SummaryInformation"));
        const qsizetype copy = si2.indexOf(QByteArray::fromHex("2000cc00"));   // DIBBitBlt's SRCCOPY, then six numbers and the bitmap
        QVERIFY(copy > 0);
        QVERIFY(qFromLittleEndian<quint32>(si2.constData() + copy + 4 + 12 + 4) < 160);              // bitmap width
        QCOMPARE(qFromLittleEndian<quint32>(si2.constData() + copy + 4 + 12 + 8), quint32(160));   // and height
    }

    // A right-to-left paragraph through .pub: written as in a sample made in
    // Publisher (0x06 = 0 and 0x3A = 0xF3FF on that paragraph only; 0x3A
    // appears in none of 247 left-to-right reference files), read back.
    void pubRightToLeft()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 400, 100);
        t->storyId = doc->createStory(QStringLiteral("Left to Right\nRight to Left"));
        doc->pages[0]->items.push_back(t);
        QTextBlock second = doc->storyDoc(t->storyId)->begin().next();
        QVERIFY(second.isValid());
        QTextCursor c(second);
        QTextBlockFormat bf = second.blockFormat();
        bf.setLayoutDirection(Qt::RightToLeft);
        c.setBlockFormat(bf);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("rtl.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        const QByteArray quill = jp::cfb::readStream(path, QStringLiteral("Quill/QuillSub/CONTENTS"));
        // Both in the right-to-left paragraph's property list (kept in id order).
        QCOMPARE(quill.count(QByteArray::fromHex("3a12fff3")), 1);
        const qsizetype at = quill.indexOf(QByteArray::fromHex("3a12fff3"));
        QVERIFY(quill.mid(at - 40, 40).contains(QByteArray::fromHex("062200000000")));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QTextDocument *story = nullptr;
        for (const auto &it : back->pages[0]->items)
            if (auto tx = std::dynamic_pointer_cast<jp::TextItem>(it)) story = back->storyDoc(tx->storyId);
        QVERIFY(story);
        QVERIFY(story->begin().blockFormat().layoutDirection() != Qt::RightToLeft);
        QCOMPARE(story->begin().next().blockFormat().layoutDirection(), Qt::RightToLeft);
        // Left-aligned (its start) is at the right edge for right-to-left text.
        jp::FrameSpec fs;
        fs.size = QSizeF(400, 100);
        fs.insets = QMarginsF(0, 0, 0, 0);
        jp::StoryLayout lay;
        lay.build(story, {fs}, jp::LayoutEnv());
        const QTextBlock b1 = story->begin(), b2 = b1.next();
        const QRectF ltr = lay.rangeRects(0, b1.position(), b1.position() + b1.length() - 1).value(0);
        const QRectF rtl = lay.rangeRects(0, b2.position(), b2.position() + b2.length() - 1).value(0);
        QVERIFY2(ltr.left() < 5 && rtl.right() > 395, qPrintable(QStringLiteral("%1 %2").arg(ltr.left()).arg(rtl.right())));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test34-right-to-left.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
    }

    // Every dash style comes back from .pub as it was saved (square dots
    // came back as dashes, and dash-dot patterns as plain dashes).
    void pubDashStylesRoundTrip()
    {
        using S = jp::Stroke;
        const S::Dash dashes[] = {S::RoundDot, S::SquareDot, S::DashLine, S::DashDot, S::LongDash, S::LongDashDot, S::LongDashDotDot};
        auto doc = jp::Document::blank(QSizeF(612, 792));
        for (int i = 0; i < 7; ++i) {
            auto line = std::make_shared<jp::LineItem>();
            line->p1 = QPointF(72, 72 + i * 60);
            line->p2 = QPointF(400, 72 + i * 60);
            line->rect = QRectF(line->p1, line->p2).normalized();
            line->stroke = S::line(jp::ColorRef::rgb(Qt::black), 4);
            line->stroke.dash = dashes[i];
            doc->pages[0]->items.push_back(line);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("dashes.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QList<int> got;
        for (const auto &it : back->pages[0]->items)
            if (auto l = std::dynamic_pointer_cast<jp::LineItem>(it)) got << int(l->stroke.dash);
        QList<int> want;
        for (S::Dash d : dashes) want << int(d);
        QCOMPARE(got, want);
    }

    // The Open page shows a .pub file's own preview: the picture kept in its
    // summary information (property 17; here a bitmap, in Publisher's own
    // files a metafile, both read the same way).
    void pubPreviewThumbnail()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("preview.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        QVERIFY(jp::publicationThumbnail(path).isNull());   // JeffPub's own .pub has none
        // A 40 x 30 red picture as a clipboard bitmap (a BMP without its file header).
        QImage red(40, 30, QImage::Format_RGB32);
        red.fill(QColor(200, 20, 30));
        QByteArray bmp;
        QBuffer buf(&bmp);
        buf.open(QIODevice::WriteOnly);
        red.save(&buf, "BMP");
        const QByteArray dib = bmp.mid(14);
        auto u16 = [](quint16 v) { QByteArray b(2, 0); qToLittleEndian(v, b.data()); return b; };
        auto u32 = [](quint32 v) { QByteArray b(4, 0); qToLittleEndian(v, b.data()); return b; };
        const QByteArray cf = u32(0xFFFFFFFF) + u32(8) + dib;
        const QByteArray prop = u32(0x47) + u32(quint32(cf.size())) + cf;
        const QByteArray section = u32(quint32(16 + prop.size())) + u32(1) + u32(17) + u32(16) + prop;
        const QByteArray fmtid = QByteArray::fromHex("E0859FF2F94F6810AB9108002B27B3D9");
        const QByteArray set = u16(0xFFFE) + u16(0) + u32(0x00020006) + QByteArray(16, 0) + u32(1) + fmtid + u32(48) + section;
        jp::cfb::File c;
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::ReadOnly));
            QVERIFY(jp::cfb::read(f.readAll(), &c, &err));
        }
        QVERIFY(c.setStream(QStringLiteral("\x05SummaryInformation"), set));
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(jp::cfb::write(c));
        }
        // Reading one stream straight from the file gives the same bytes.
        QCOMPARE(jp::cfb::readStream(path, QStringLiteral("\x05SummaryInformation")), set);
        QCOMPARE(jp::cfb::readStream(path, QStringLiteral("Quill/QuillSub/CONTENTS")), c.stream(QStringLiteral("Quill/QuillSub/CONTENTS")));
        const QImage th = jp::publicationThumbnail(path);
        QVERIFY(!th.isNull());
        QCOMPARE(th.width() * 3, th.height() * 4);
        QCOMPARE(th.pixelColor(th.width() / 2, th.height() / 2), QColor(200, 20, 30));
    }

    // A spot color names its ink in the fill's extra drawing properties
    // (0x01A1, as Publisher writes "P2,#003d007e00db0000,PANTONE 2727 C" on
    // book covers whose PDFs print that ink on its own plate): the
    // publication opens with that spot color, and its PDFs keep the ink.
    void pubSpotColorRead()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->print.model = jp::PrintInfo::ProcessCMYK;
        auto box = std::make_shared<jp::ShapeItem>();
        box->rect = QRectF(0, 0, 612, 792);
        box->fill = jp::Fill::solid(jp::ColorRef::inks(QColor::fromCmykF(192 / 255.f, 102 / 255.f, 0, 0), QColor(0x3d, 0x7e, 0xdb)));
        box->stroke = jp::Stroke::none();
        doc->pages[0]->items.push_back(box);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("spot.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        jp::cfb::File c;
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::ReadOnly));
            QVERIFY2(jp::cfb::read(f.readAll(), &c, &err), qPrintable(err));
        }
        // Add the name to the record of extra properties that holds the
        // fill's shown color (0x019E), growing the records around it.
        const QString text = QStringLiteral("P2,#003d007e00db0000,PANTONE 2727 C");
        QByteArray name(reinterpret_cast<const char *>(text.utf16()), text.size() * 2);
        name.append(2, '\0');
        auto u16 = [](quint16 v) { QByteArray b(2, 0); qToLittleEndian(v, b.data()); return b; };
        auto u32 = [](quint32 v) { QByteArray b(4, 0); qToLittleEndian(v, b.data()); return b; };
        int added = 0;
        std::function<QByteArray(const QByteArray &)> rebuild = [&](const QByteArray &in) {
            QByteArray out;
            int at = 0;
            while (at + 8 <= in.size()) {
                quint16 vi = qFromLittleEndian<quint16>(in.constData() + at);
                const quint16 type = qFromLittleEndian<quint16>(in.constData() + at + 2);
                const quint32 len = qFromLittleEndian<quint32>(in.constData() + at + 4);
                if (qint64(at) + 8 + len > in.size()) {   // not a record: the stream keeps a value between drawings
                    out += in.mid(at, 4);
                    at += 4;
                    continue;
                }
                QByteArray body = in.mid(at + 8, len);
                if ((vi & 0xF) == 0xF) {
                    body = rebuild(body);
                } else if (type == 0xF122) {
                    const int n = vi >> 4;
                    bool shown = false;
                    for (int k = 0; k < n && k * 6 + 6 <= body.size(); ++k)
                        shown |= (qFromLittleEndian<quint16>(body.constData() + k * 6) & 0x3FFF) == 0x019E;
                    if (shown) {
                        body = body.left(n * 6) + u16(0xC1A1) + u32(quint32(name.size())) + body.mid(n * 6) + name;
                        vi = quint16((vi & 0xF) | ((n + 1) << 4));
                        ++added;
                    }
                }
                out += u16(vi) + u16(type) + u32(quint32(body.size())) + body;
                at += 8 + int(len);
            }
            return out + in.mid(at);
        };
        QVERIFY(c.setStream(QStringLiteral("Escher/EscherStm"), rebuild(c.stream(QStringLiteral("Escher/EscherStm")))));
        QCOMPARE(added, 1);
        {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(jp::cfb::write(c));
        }
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->print.model, jp::PrintInfo::SpotColors);
        QCOMPARE(back->print.spotNames, QStringList{QStringLiteral("PANTONE 2727 C")});
        const QColor ink = back->print.spotColors.value(0).toCmyk();
        QCOMPARE(ink.cyan(), 192);
        QCOMPARE(ink.magenta(), 102);
        // The page still shows Publisher's screen color.
        bool found = false;
        for (const auto &it : back->pages[0]->items)
            if (auto sh = std::dynamic_pointer_cast<jp::ShapeItem>(it); sh && sh->fill.type == jp::Fill::Solid) {
                QCOMPARE(sh->fill.color.shownValue(), QColor(0x3d, 0x7e, 0xdb));
                found = true;
            }
        QVERIFY(found);
    }

    // Process inks survive saving as .pub: the file keeps the color as shown
    // and the inks (checked against values reference files hold).
    void pubWriterProcessInks()
    {
        struct Ink { float c, m, y, k; QColor shown; const char *label; };
        const Ink inks[] = {{0.98f, 0.761f, 0.133f, 0.302f, QColor(51, 61, 97), "navy C98 M76 Y13 K30"},
                            {0.6f, 0.4f, 0.4f, 1.0f, QColor(29, 31, 25), "rich black C60 M40 Y40 K100"},
                            {0.753f, 0.4f, 0, 0, QColor(91, 125, 179), "blue C75 M40"},
                            {0, 0, 1.0f, 0, QColor(255, 242, 0), "yellow Y100"},
                            {0, 0, 0, 1.0f, QColor(35, 31, 32), "black K100"},
                            {0, 0.5f, 1.0f, 0.2f, QColor(220, 140, 30), "orange M50 Y100 K20"},
                            {0.3f, 0, 0, 0.6f, QColor(80, 100, 110), "slate C30 K60"}};
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->print.model = jp::PrintInfo::ProcessCMYK;
        for (int i = 0; i < 7; ++i) {
            auto box = std::make_shared<jp::ShapeItem>();
            box->rect = QRectF(72, 50 + i * 100, 200, 80);
            box->fill = jp::Fill::solid(jp::ColorRef::inks(QColor::fromCmykF(inks[i].c, inks[i].m, inks[i].y, inks[i].k), inks[i].shown));
            box->stroke = jp::Stroke::none();
            doc->pages[0]->items.push_back(box);
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(290, 70 + i * 100, 260, 40);
            t->storyId = doc->createStory(QString::fromLatin1(inks[i].label));
            doc->pages[0]->items.push_back(t);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("inks.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString out = qEnvironmentVariable("JP_SHOT_DIR") + "/test31-process-colors.pub";
            QFile::remove(out);
            QFile::copy(path, out);
        }
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->print.model, jp::PrintInfo::ProcessCMYK);
        int found = 0;
        jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
            if (it->type() != jp::ItemType::Shape) return;
            const jp::ColorRef c = static_cast<const jp::ShapeItem *>(it.get())->fill.color;
            const QColor k = c.rgbValue();
            for (const Ink &in : inks)
                if (c.shownValue() == in.shown) {
                    QCOMPARE(k.spec(), QColor::Cmyk);
                    QVERIFY2(std::abs(k.cyanF() - in.c) < 0.003 && std::abs(k.magentaF() - in.m) < 0.003 && std::abs(k.yellowF() - in.y) < 0.003
                                 && std::abs(k.blackF() - in.k) < 0.003, in.label);
                    ++found;
                }
        });
        QCOMPARE(found, 7);
    }

    // Gradients keep their direction and colors through a .pub: Publisher
    // runs the first color toward 270 + the file's angle (its own pictures
    // of 113 gradients), and from the center the first color is the center.
    void pubGradientsRoundTrip()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        auto add = [&](Fill::GradType type, double angle, double x) {
            auto s = std::make_shared<ShapeItem>();
            s->shape = QStringLiteral("rect");
            s->rect = QRectF(x, 72, 80, 120);
            s->stroke.color = ColorRef::none();
            s->fill.type = Fill::Gradient;
            s->fill.gradType = type;
            s->fill.angle = angle;
            s->fill.stops = {GradientStop{0, ColorRef::rgb(QColor(200, 0, 0)), 0}, GradientStop{1, ColorRef::rgb(QColor(0, 0, 200)), 0}};
            s->fill.color = s->fill.stops.first().color;
            s->fill.color2 = s->fill.stops.last().color;
            doc->pages[0]->items.push_back(s);
        };
        const double angles[] = {0, 90, 180, 270};
        for (int i = 0; i < 4; ++i) add(Fill::Linear, angles[i], 36 + 90 * i);
        add(Fill::Radial, 0, 400);
        add(Fill::PathGrad, 0, 490);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("grad.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        auto back = importPublisher(f.readAll(), nullptr);
        QVERIFY(back);
        QVector<const ShapeItem *> got;
        for (const auto &it : back->pages[0]->items)
            if (auto *s = dynamic_cast<const ShapeItem *>(it.get()); s && s->fill.type == Fill::Gradient) got << s;
        std::sort(got.begin(), got.end(), [](auto *a, auto *b) { return a->rect.x() < b->rect.x(); });
        QCOMPARE(got.size(), 6);
        for (int i = 0; i < 4; ++i) {
            QCOMPARE(got[i]->fill.gradType, Fill::Linear);
            QVERIFY2(std::abs(std::remainder(got[i]->fill.angle - angles[i], 360.0)) < 0.5, qPrintable(QString::number(got[i]->fill.angle)));
        }
        QCOMPARE(got[4]->fill.gradType, Fill::Radial);
        QCOMPARE(got[5]->fill.gradType, Fill::PathGrad);
        for (int i : {4, 5}) {
            const auto &st = got[i]->fill.stops;
            QVERIFY(st.size() >= 2);
            QCOMPARE(st.first().color.rgbValue(), QColor(200, 0, 0));   // the center
            QCOMPARE(st.last().color.rgbValue(), QColor(0, 0, 200));
        }
    }

    // A picture keeps its quarter turns and its own fill through a .pub:
    // rotated flowers lost their quarter turns (the outline's turn is folded
    // within 45 degrees), and pinwheel art lost the colors its shape's fill
    // shows through the metafile's gaps.
    void pubPictureTurnsAndFill()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        QImage img(40, 20, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QPainter(&img).fillRect(0, 0, 20, 20, Qt::blue);
        QByteArray png;
        {
            QBuffer buf(&png);
            buf.open(QIODevice::WriteOnly);
            img.save(&buf, "PNG");
        }
        const double turns[] = {0, 90, 200, 290};
        for (int i = 0; i < 4; ++i) {
            auto pic = std::make_shared<PictureItem>();
            pic->imageId = doc->addImage(png, QStringLiteral("png"));
            pic->rect = QRectF(72 + 130 * i, 300, 120, 60);
            pic->rotation = turns[i];
            if (i == 1) pic->fill = Fill::solid(ColorRef::rgb(QColor(0x33, 0x66, 0x66)));
            doc->pages[0]->items.push_back(pic);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("pics.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        auto back = importPublisher(f.readAll(), nullptr);
        QVERIFY(back);
        QVector<const PictureItem *> got;
        for (const auto &it : back->pages[0]->items)
            if (auto *p = dynamic_cast<const PictureItem *>(it.get())) got << p;
        std::sort(got.begin(), got.end(), [](auto *a, auto *b) { return a->rect.center().x() < b->rect.center().x(); });
        QCOMPARE(got.size(), 4);
        for (int i = 0; i < 4; ++i) {
            QVERIFY2(std::abs(std::remainder(got[i]->rotation - turns[i], 360.0)) < 0.5, qPrintable(QStringLiteral("%1: %2").arg(i).arg(got[i]->rotation)));
            QVERIFY2(std::abs(got[i]->rect.width() - 120) < 0.5 && std::abs(got[i]->rect.height() - 60) < 0.5,
                     qPrintable(QStringLiteral("%1: %2x%3").arg(i).arg(got[i]->rect.width()).arg(got[i]->rect.height())));
        }
        QCOMPARE(got[1]->fill.type, Fill::Solid);
        QCOMPARE(got[1]->fill.color.rgbValue(), QColor(0x33, 0x66, 0x66));
        QCOMPARE(got[0]->fill.type, Fill::NoFill);
    }

    // A freeform's formulas can refer to earlier ones (up to 256 back),
    // each up to three times: evaluated afresh at every reference, a chain
    // of them took exponential time, so a crafted file hung on opening.
    // Each formula is now worked out once per point.
    void pubFormulaChainIsFast()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        auto s = std::make_shared<ShapeItem>();
        s->shape = QStringLiteral("rect");
        s->rect = QRectF(100, 100, 200, 150);
        QPainterPath path;
        path.moveTo(0, 0);
        path.lineTo(200, 20);
        path.lineTo(120, 150);
        path.closeSubpath();
        s->customPath = path;
        s->fill = Fill::solid(ColorRef::rgb(Qt::red));
        doc->pages[0]->items.push_back(s);
        QTemporaryDir dir;
        const QString pathName = dir.filePath(QStringLiteral("chain.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, pathName, &err), qPrintable(err));
        QFile f(pathName);
        QVERIFY(f.open(QIODevice::ReadOnly));
        cfb::File file;
        QVERIFY(cfb::read(f.readAll(), &file, &err));
        const QByteArray escher = cfb::readStream(pathName, QStringLiteral("Escher/EscherStm"));

        // 22 formulas: the first is 1, each next the average of the one
        // before taken twice (still 1); every point's x refers to the last.
        const int n = 22;
        QByteArray guides(6 + 8 * n, 0);
        qToLittleEndian<quint16>(n, guides.data());
        qToLittleEndian<quint16>(n, guides.data() + 2);
        qToLittleEndian<quint16>(8, guides.data() + 4);
        for (int i = 0; i < n; ++i) {
            char *g = guides.data() + 6 + 8 * i;
            if (i == 0) {
                qToLittleEndian<quint16>(0, g);   // a + b - c
                qToLittleEndian<qint16>(1, g + 2);
            } else {
                qToLittleEndian<quint16>(0x6002, g);   // (a + b) / 2, a and b refer
                qToLittleEndian<qint16>(qint16(0x400 | (i - 1)), g + 2);
                qToLittleEndian<qint16>(qint16(0x400 | (i - 1)), g + 4);
            }
        }
        // The formulas go with the freeform's points (0xC145), whose x
        // values are set to refer to the last formula.
        const QByteArray rebuilt = editFirstOpt(escher, [&](QByteArray &body, quint16 &head) {
            const int pointsAt = optComplexAt(body, head, 0x0145);
            if (pointsAt < 0) return false;
            const int points = qFromLittleEndian<quint16>(body.constData() + pointsAt);
            for (int i = 0; i < points; ++i) qToLittleEndian<quint32>(0x80000000u | (n - 1), body.data() + pointsAt + 6 + 8 * i);
            addOptProp(body, head, 0xC156, 0, guides);
            return true;
        });
        QVERIFY(!rebuilt.isEmpty());
        QVERIFY(file.setStream(QStringLiteral("Escher/EscherStm"), rebuilt));
        QElapsedTimer t;
        t.start();
        auto back = importPublisher(cfb::write(file), nullptr);
        QVERIFY(back);
        QVERIFY2(t.elapsed() < 1500, qPrintable(QStringLiteral("%1 ms").arg(t.elapsed())));
    }

    // A freeform's coordinate space (0x0140-0x0143) is any 32-bit range;
    // its width came from a 32-bit subtraction, which overflows (undefined
    // behavior; the sanitizer build reports it) for -2^31 to 2^31-1.
    void pubHugeCoordinateSpace()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        auto s = std::make_shared<ShapeItem>();
        s->shape = QStringLiteral("rect");
        s->rect = QRectF(100, 100, 200, 150);
        QPainterPath path;
        path.moveTo(0, 0);
        path.lineTo(200, 20);
        path.lineTo(120, 150);
        path.closeSubpath();
        s->customPath = path;
        s->fill = Fill::solid(ColorRef::rgb(Qt::red));
        doc->pages[0]->items.push_back(s);
        QTemporaryDir dir;
        const QString name = dir.filePath(QStringLiteral("space.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, name, &err), qPrintable(err));
        QFile f(name);
        QVERIFY(f.open(QIODevice::ReadOnly));
        cfb::File file;
        QVERIFY(cfb::read(f.readAll(), &file, &err));
        const QByteArray rebuilt = editFirstOpt(cfb::readStream(name, QStringLiteral("Escher/EscherStm")), [&](QByteArray &body, quint16 &head) {
            if (optComplexAt(body, head, 0x0145) < 0) return false;
            for (int i = 0; i < (head >> 4); ++i)
                if (qFromLittleEndian<quint16>(body.constData() + i * 6) == 0x0142) qToLittleEndian<quint32>(0x7FFFFFFFu, body.data() + i * 6 + 2);
            addOptProp(body, head, 0x0140, 0x80000000u);
            return true;
        });
        QVERIFY(!rebuilt.isEmpty());
        QVERIFY(file.setStream(QStringLiteral("Escher/EscherStm"), rebuilt));
        auto back = importPublisher(cfb::write(file), nullptr);
        QVERIFY(back);
        for (const auto &it : back->pages[0]->items) {
            const QRectF r = it->rect;
            QVERIFY(std::isfinite(r.x()) && std::isfinite(r.y()) && std::isfinite(r.width()) && std::isfinite(r.height()));
        }
    }

    // Publisher's default text box shadow is stored as "the line color,
    // lightened halfway" (0x107F02F2): gray for a black line. libmspub
    // lightened the reference's own bytes into pink.
    void pubShadowFromLineColor()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(72, 72, 200, 100);
        t->storyId = doc->createStory(QStringLiteral("Shadowed"));
        t->fill = Fill::solid(ColorRef::rgb(Qt::white));
        t->stroke.color = ColorRef::rgb(Qt::black);
        t->stroke.width = 1;
        t->fx.shadow.on = true;
        t->fx.shadow.distance = 6;
        t->fx.shadow.color = ColorRef::rgb(QColor(0x12, 0x34, 0x56));
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("shadow.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        cfb::File file;
        QVERIFY(cfb::read(f.readAll(), &file, &err));
        QByteArray escher = cfb::readStream(path, QStringLiteral("Escher/EscherStm"));
        auto prop = [](quint16 id, quint32 value) {
            QByteArray b(6, 0);
            qToLittleEndian<quint16>(id, b.data());
            qToLittleEndian<quint32>(value, b.data() + 2);
            return b;
        };
        QCOMPARE(int(escher.count(prop(0x0201, 0x00563412))), 1);
        escher.replace(prop(0x0201, 0x00563412), prop(0x0201, 0x107F02F2));
        QVERIFY(file.setStream(QStringLiteral("Escher/EscherStm"), escher));
        auto back = importPublisher(cfb::write(file), nullptr);
        QVERIFY(back);
        const Item *got = nullptr;
        for (const auto &it : back->pages[0]->items)
            if (it->fx.shadow.on) got = it.get();
        QVERIFY(got);
        const QColor c = got->fx.shadow.color.rgbValue();
        QVERIFY2(std::abs(c.red() - 128) <= 2 && std::abs(c.green() - 128) <= 2 && std::abs(c.blue() - 128) <= 2, qPrintable(c.name()));
    }

    // A line's corners go through a .pub: Publisher's designs store round
    // corners (0x01D6 = 2) and JeffPub drew them mitered.
    void pubLineCornersRoundTrip()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        const Qt::PenJoinStyle joins[] = {Qt::MiterJoin, Qt::RoundJoin, Qt::BevelJoin};
        for (int i = 0; i < 3; ++i) {
            auto s = std::make_shared<ShapeItem>();
            s->shape = QStringLiteral("triangle");
            s->rect = QRectF(72 + 160 * i, 100, 120, 100);
            s->fill = Fill::none();
            s->stroke.color = ColorRef::rgb(Qt::black);
            s->stroke.width = 12;
            s->stroke.join = joins[i];
            doc->pages[0]->items.push_back(s);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("joins.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        auto back = importPublisher(f.readAll(), nullptr);
        QVERIFY(back);
        QVector<const Item *> got;
        for (const auto &it : back->pages[0]->items)
            if (!it->stroke.isNone()) got << it.get();
        std::sort(got.begin(), got.end(), [](auto *a, auto *b) { return a->rect.x() < b->rect.x(); });
        QCOMPARE(got.size(), 3);
        for (int i = 0; i < 3; ++i) QCOMPARE(got[i]->stroke.join, joins[i]);
    }

    // An empty rectangle's outline comes back on its frame: libmspub moved
    // the outline's top edge out by half the line (it applies one edge per
    // side line, and a plain outline is one line), so a sign's 12-pt frame
    // drawn inside its edge sat 6 pt too high.
    void pubOutlineKeepsFrame()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        for (bool inset : {false, true}) {
            auto s = std::make_shared<ShapeItem>();
            s->shape = QStringLiteral("rect");
            s->rect = QRectF(inset ? 340 : 72, 100, 200, 150);
            s->fill = Fill::none();
            s->stroke.color = ColorRef::rgb(Qt::black);
            s->stroke.width = 12;
            s->stroke.inset = inset;
            doc->pages[0]->items.push_back(s);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("frame.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        auto back = importPublisher(f.readAll(), nullptr);
        QVERIFY(back);
        int n = 0;
        for (const auto &it : back->pages[0]->items) {
            if (it->stroke.isNone()) continue;
            ++n;
            const QRectF want(it->rect.x() < 300 ? 72 : 340, 100, 200, 150);
            QVERIFY2(std::abs(it->rect.top() - want.top()) < 0.1 && std::abs(it->rect.bottom() - want.bottom()) < 0.1
                         && std::abs(it->rect.left() - want.left()) < 0.1 && std::abs(it->rect.right() - want.right()) < 0.1,
                     qPrintable(QStringLiteral("%1,%2 %3x%4").arg(it->rect.x()).arg(it->rect.y()).arg(it->rect.width()).arg(it->rect.height())));
        }
        QCOMPARE(n, 2);
    }

    // Gradient angles as Publisher reads them (its pictures of JeffPub's
    // files and of its own): a stored angle a of 0 or more runs the first
    // color toward 270 - a, a negative one toward 90 - a. A gradient turns
    // with its shape only when the tertiary fill flags (0x01BF) carry 0x20
    // with its use bit; otherwise it keeps its direction on the page.
    // JeffPub saved 45 degrees as 135 (mirrored) and never set the flag.
    void pubGradientAnglesAsPublisher()
    {
        struct Case { double angle, rotation; bool flipH, flipV; };
        const Case cases[] = {{45, 0, false, false}, {45, 0, true, false}, {45, 0, false, true}, {135, 90, false, false}, {60, 0, false, false}, {300, 30, false, false}};
        auto doc = Document::blank(QSizeF(792, 612));
        int i = 0;
        for (const Case &c : cases) {
            auto s = std::make_shared<ShapeItem>();
            s->shape = QStringLiteral("rect");
            s->rect = QRectF(36 + 120 * i++, 72, 100, 100);
            s->rotation = c.rotation;
            s->flipH = c.flipH;
            s->flipV = c.flipV;
            s->stroke.color = ColorRef::none();
            s->fill.type = Fill::Gradient;
            s->fill.gradType = Fill::Linear;
            s->fill.angle = c.angle;
            s->fill.stops = {GradientStop{0, ColorRef::rgb(QColor(200, 0, 0)), 0}, GradientStop{1, ColorRef::rgb(QColor(0, 0, 200)), 0}};
            s->fill.color = s->fill.stops.first().color;
            s->fill.color2 = s->fill.stops.last().color;
            doc->pages[0]->items.push_back(s);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("grad.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        auto prop = [](quint16 id, quint32 value) {
            QByteArray b(6, 0);
            qToLittleEndian<quint16>(id, b.data());
            qToLittleEndian<quint32>(value, b.data() + 2);
            return b;
        };
        QByteArray escher = cfb::readStream(path, QStringLiteral("Escher/EscherStm"));
        QVERIFY(escher.contains(prop(0x018b, 225u << 16)));   // 45 degrees, as Publisher's dialog stores 225
        QCOMPARE(int(escher.count(prop(0x01bf, 0x00600020))), 6);
        // Where each first color runs on the page.
        auto shown = [](const ShapeItem *s) {
            double a = s->fill.angle;
            if (s->flipH) a = 180 - a;
            if (s->flipV) a = -a;
            return a + s->rotation;
        };
        auto read = [&](const QByteArray &bytes) {
            auto back = importPublisher(bytes, nullptr);
            QVector<const ShapeItem *> got;
            if (back) {
                for (const auto &it : back->pages[0]->items)
                    if (auto *s = dynamic_cast<const ShapeItem *>(it.get()); s && s->fill.type == Fill::Gradient) got << s;
                std::sort(got.begin(), got.end(), [](auto *a, auto *b) { return a->rect.center().x() < b->rect.center().x(); });
            }
            return std::make_pair(std::move(back), got);
        };
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        {
            auto [back, got] = read(bytes);
            QCOMPARE(got.size(), 6);
            for (int k = 0; k < 6; ++k) {
                const Case &c = cases[k];
                double want = c.angle;
                if (c.flipH) want = 180 - want;
                if (c.flipV) want = -want;
                want += c.rotation;
                QVERIFY2(std::abs(std::remainder(shown(got[k]) - want, 360.0)) < 0.5, qPrintable(QStringLiteral("%1: %2").arg(k).arg(shown(got[k]))));
            }
        }
        // Without the flag Publisher keeps each gradient's stored direction
        // on the page, whatever the shape's turns and flips.
        cfb::File file;
        QVERIFY(cfb::read(bytes, &file, &err));
        escher.replace(prop(0x01bf, 0x00600020), prop(0x01bf, 0));
        QVERIFY(file.setStream(QStringLiteral("Escher/EscherStm"), escher));
        {
            auto [back, got] = read(cfb::write(file));
            QCOMPARE(got.size(), 6);
            for (int k = 0; k < 6; ++k)
                QVERIFY2(std::abs(std::remainder(shown(got[k]) - cases[k].angle, 360.0)) < 0.5, qPrintable(QStringLiteral("%1: %2").arg(k).arg(shown(got[k]))));
        }
    }

    // A straight line from Publisher's line tool (shape 20, two points) comes
    // back as a line with flat ends. It came in as a closed outline of two
    // points, and the join where that outline turns back on itself pushed a
    // spike out past each end: a car wash flyer's 18-pt bars stuck out of
    // their frame by twice their width.
    void pubStraightLineIsALine()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        const QPointF ends[2][2] = {{QPointF(100, 200), QPointF(400, 200)}, {QPointF(500, 300), QPointF(500, 600)}};
        for (const auto &e : ends) {
            auto l = std::make_shared<LineItem>();
            l->p1 = e[0];
            l->p2 = e[1];
            l->stroke.color = ColorRef::rgb(Qt::black);
            l->stroke.width = 18;
            l->syncRect();
            doc->pages[0]->items.push_back(l);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("lines.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        // JeffPub writes a line as a straight connector (shape 32); Publisher's
        // own are shape 20.
        auto kind = [](int k) {
            QByteArray b(4, 0);
            qToLittleEndian<quint16>(quint16(2 | (k << 4)), b.data());
            qToLittleEndian<quint16>(0xf00a, b.data() + 2);
            return b;
        };
        QByteArray escher = cfb::readStream(path, QStringLiteral("Escher/EscherStm"));
        QCOMPARE(int(escher.count(kind(32))), 2);
        escher.replace(kind(32), kind(20));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        cfb::File file;
        QVERIFY(cfb::read(f.readAll(), &file, &err));
        QVERIFY(file.setStream(QStringLiteral("Escher/EscherStm"), escher));
        auto back = importPublisher(cfb::write(file), nullptr);
        QVERIFY(back);
        int lines = 0;
        for (const auto &it : back->pages[0]->items) lines += it->type() == ItemType::Line;
        QCOMPARE(lines, 2);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = back.get();
        ctx.cache = &cache;
        ctx.opt.output = true;
        const QImage page = Renderer::renderToImage(ctx, 0, 1.0);
        auto dark = [&](int x, int y) { return QColor(page.pixel(x, y)).value() < 128; };
        QVERIFY(dark(250, 200) && dark(105, 200) && dark(395, 200));
        QVERIFY2(!dark(95, 200) && !dark(405, 200), "the horizontal line has flat ends");
        QVERIFY(dark(500, 450) && dark(500, 305) && dark(500, 595));
        QVERIFY2(!dark(500, 295) && !dark(500, 605), "the vertical line has flat ends");
    }

    // Text whose size no run, style, or paragraph gives is 12 pt, as
    // Publisher draws its car wash flyer's body text (the file's styles name
    // only a font): it was 11, the size of JeffPub's own blank text, and
    // each line came out an eleventh shorter.
    void pubTextWithoutSizeIs12()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(72, 72, 300, 100);
        t->storyId = doc->createStory(QStringLiteral("Sized by nobody"));
        {
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            c.setCharFormat(QTextCharFormat());
        }
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("size.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        auto sizeOf = [&](const QByteArray &bytes) {
            auto back = importPublisher(bytes, nullptr);
            double size = -1;
            if (back) {
                for (const auto &it : back->pages[0]->items)
                    if (auto *tb = dynamic_cast<const TextItem *>(it.get()); tb && tb->storyId.size()) {
                        const QTextBlock b = back->storyDoc(tb->storyId)->begin();
                        size = b.begin().fragment().charFormat().fontPointSize();
                    }
            }
            return size;
        };
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray bytes = f.readAll();
        // The Normal style names its size (JeffPub writes 10 pt)...
        QCOMPARE(sizeOf(bytes), 10.0);
        // ...and without that, nothing does.
        cfb::File file;
        QVERIFY(cfb::read(bytes, &file, &err));
        QByteArray quill = cfb::readStream(path, QStringLiteral("Quill/QuillSub/CONTENTS"));
        auto prop = [](quint8 id) {
            QByteArray b(6, 0);
            b[0] = char(id);
            b[1] = char(0x22);
            qToLittleEndian<quint32>(127000u, b.data() + 2);
            return b;
        };
        QVERIFY(quill.count(prop(0x0c)) >= 1);
        quill.replace(prop(0x0c), prop(0x7e));
        quill.replace(prop(0x39), prop(0x7f));
        QVERIFY(file.setStream(QStringLiteral("Quill/QuillSub/CONTENTS"), quill));
        QCOMPARE(sizeOf(cfb::write(file)), 12.0);
    }

    // Publisher's superscript keeps flags in the high byte (0xF001): "5th"
    // read back from such a file keeps its raised letters.
    void pubSuperscriptHighByte()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(72, 72, 300, 100);
        t->storyId = doc->createStory(QStringLiteral("5th"));
        QTextCursor c(doc->storyDoc(t->storyId));
        c.setPosition(1);
        c.setPosition(3, QTextCursor::KeepAnchor);
        QTextCharFormat up;
        up.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
        c.mergeCharFormat(up);
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("sup.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        cfb::File file;
        QVERIFY(cfb::read(f.readAll(), &file));
        f.close();
        QByteArray q = file.stream(QStringLiteral("Quill/QuillSub/CONTENTS"));
        const QByteArray mark("\x0f\x12\x01\x00", 4);
        QCOMPARE(q.count(mark), 1);
        q.replace(mark, QByteArray("\x0f\x12\x01\xf0", 4));
        QVERIFY(file.setStream(QStringLiteral("Quill/QuillSub/CONTENTS"), q));
        auto back = importPublisher(cfb::write(file), nullptr);
        QVERIFY(back);
        bool raised = false;
        for (const auto &st : back->stories)
            for (QTextBlock b = st->doc->begin(); b.isValid(); b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it)
                    if (it.fragment().text().startsWith(QStringLiteral("th")))
                        raised = it.fragment().charFormat().verticalAlignment() == QTextCharFormat::AlignSuperScript;
        QVERIFY(raised);
    }

    // A link with no color of its own shows as a link: the .pub reader's
    // fallback black made e-mail links black where Publisher shows them blue.
    void pubLinkKeepsLinkColor()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(72, 72, 300, 100);
        t->storyId = doc->createStory(QStringLiteral("Write to us"));
        QTextCursor c(doc->storyDoc(t->storyId));
        c.setPosition(9);
        c.setPosition(11, QTextCursor::KeepAnchor);
        QTextCharFormat link;
        link.setAnchor(true);
        link.setAnchorHref(QStringLiteral("mailto:office@example.com"));
        c.mergeCharFormat(link);
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("link.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        auto back = importPublisher(f.readAll(), nullptr);
        QVERIFY(back);
        int links = 0;
        for (const auto &st : back->stories)
            for (QTextBlock b = st->doc->begin(); b.isValid(); b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it)
                    if (it.fragment().charFormat().isAnchor()) {
                        ++links;
                        QVERIFY2(!it.fragment().charFormat().hasProperty(tp::ColorRefP), qPrintable(it.fragment().charFormat().stringProperty(tp::ColorRefP)));
                    }
        QVERIFY(links > 0);
    }

    // Publisher's numbering "(none)" (list kind 255) is no list: the sample
    // newsletter's paragraph with it took a number.
    void pubNoneNumberingIsNoList()
    {
        QFile f(QStringLiteral(JP_TEST_DATA "/pub/poi-SampleNewsletter.pub"));
        QVERIFY(f.open(QIODevice::ReadOnly));
        auto doc = importPublisher(f.readAll(), nullptr);
        QVERIFY(doc);
        int numbered = 0;
        for (const auto &st : doc->stories)
            for (QTextBlock b = st->doc->begin(); b.isValid(); b = b.next())
                if (b.textList() && !jp::isBulletList(b.textList()->format().style())) ++numbered;
        QCOMPARE(numbered, 0);
        // Its bulleted paragraphs take their bullet from their paragraph
        // style (they have no list of their own), as in Publisher.
        int bulleted = 0;
        for (const auto &st : doc->stories)
            for (QTextBlock b = st->doc->begin(); b.isValid(); b = b.next())
                if (b.textList() && jp::isBulletList(b.textList()->format().style()) && !b.text().trimmed().isEmpty()) ++bulleted;
        QCOMPARE(bulleted, 6);   // 3 with a list of their own, 3 from their style
    }
    void colorRefRoundTrip()
    {
        const ColorRef a = ColorRef::scheme(Accent2, 40);
        QCOMPARE(ColorRef::fromString(a.toString()), a);
        QCOMPARE(ColorRef::fromString("#1F5FAD").resolve(builtinColorSchemes()[0]), QColor("#1F5FAD"));
        QVERIFY(builtinColorSchemes().size() >= 90);
    }
    void documentRoundTrip()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->storyId = doc->createStory("Hello world\nSecond paragraph");
        doc->pages[0]->items.push_back(t);
        auto s = std::make_shared<ShapeItem>();
        s->shape = "star5";
        doc->pages[0]->items.push_back(s);
        QTextCursor c(doc->storyDoc(t->storyId));
        c.movePosition(QTextCursor::End);
        QTextCharFormat f;
        f.setProperty(tp::Field, QStringLiteral("page"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), f);

        const QByteArray bytes = publicationBytes(*doc, QImage());
        QString err;
        auto back = publicationFromBytes(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->pages[0]->items.size(), size_t(2));
        auto *t2 = static_cast<TextItem *>(back->pages[0]->items[0].get());
        QCOMPARE(back->storyDoc(t2->storyId)->toPlainText(), doc->storyDoc(t->storyId)->toPlainText());
        QCOMPARE(QJsonDocument(back->toJson()).toJson(), QJsonDocument(doc->toJson()).toJson());
    }
    void layoutFlowsAcrossFrames()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        QString text;
        for (int i = 0; i < 40; ++i) text += QStringLiteral("Paragraph %1 has enough words to wrap across a narrow column of text.\n").arg(i);
        const QString sid = doc->createStory(text);
        FrameSpec a; a.size = QSizeF(144, 144); a.insets = QMarginsF(3, 3, 3, 3);
        FrameSpec b = a;
        LayoutEnv env; env.colors = doc->colors; env.fonts = doc->fonts;
        StoryLayout one;
        one.build(doc->storyDoc(sid), {a}, env);
        QVERIFY(one.overflow());
        StoryLayout two;
        two.build(doc->storyDoc(sid), {a, b}, env);
        QVERIFY(two.firstPosition(1) > two.firstPosition(0));
        QVERIFY(two.lastPosition(0) <= two.firstPosition(1));
        // Caret round trip
        int fr = -1; QRectF r;
        QVERIFY(two.caretRect(5, &fr, &r));
        QCOMPARE(fr, 0);
        QCOMPARE(two.hitTest(0, QPointF(r.x() + 0.1, r.center().y())), 5);
    }
    void fieldsDisplay()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        const QString sid = doc->createStory("Page ");
        QTextCursor c(doc->storyDoc(sid));
        c.movePosition(QTextCursor::End);
        QTextCharFormat f;
        f.setProperty(tp::Field, QStringLiteral("page"));
        c.insertText(QString(QChar::ObjectReplacementCharacter), f);
        FrameSpec a; a.size = QSizeF(300, 100); a.ctx.pageNumber = 12;
        LayoutEnv env; env.colors = doc->colors; env.fonts = doc->fonts;
        StoryLayout l;
        l.build(doc->storyDoc(sid), {a}, env);
        QVERIFY(!l.overflow());
        int fr; QRectF r5, r6;
        QVERIFY(l.caretRect(5, &fr, &r5));
        QVERIFY(l.caretRect(6, &fr, &r6));
        QVERIFY(r6.x() - r5.x() > 8);   // "12" is wider than one character
    }
    void renderSmoke()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        doc->colors = *findColorScheme("Seventy-Nine");
        auto &items = doc->pages[0]->items;
        auto band = std::make_shared<ShapeItem>();
        band->rect = QRectF(0, 0, 612, 230);
        band->fill = Fill::gradient(ColorRef::scheme(Accent1), ColorRef::scheme(Accent2), 0);
        band->stroke = Stroke::none();
        items.push_back(band);
        auto wa = std::make_shared<TextArtItem>();
        wa->rect = QRectF(60, 40, 492, 140);
        wa->text = "Summer Concert";
        applyTextArtStyle(*wa, textArtStyles()[11]);
        wa->fill = Fill::solid(ColorRef::rgb(Qt::white));
        wa->fx.shadow.on = true;
        items.push_back(wa);
        int x = 40;
        for (const char *sh : {"star5", "heart", "rightArrow", "cloud", "wedgeRoundRectCallout", "fcDocument", "irregularSeal1"}) {
            auto s = std::make_shared<ShapeItem>();
            s->shape = sh;
            s->rect = QRectF(x, 260, 70, 70);
            s->fill = Fill::solid(ColorRef::scheme(Accent3));
            s->fx.shadow.on = std::string(sh) == "heart";
            items.push_back(s);
            x += 78;
        }
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(40, 360, 250, 380);
        t->columns = 2;
        t->stroke = Stroke::line(ColorRef::scheme(Accent4), 1);
        QString text;
        for (int i = 0; i < 6; ++i) text += "Bring a blanket and join your neighbors for an evening of music under the stars. Food trucks open at six.\n";
        t->storyId = doc->createStory(text);
        QTextCursor c(doc->storyDoc(t->storyId));
        QTextBlockFormat bf;
        bf.setProperty(tp::DropCapLines, 3);
        c.mergeBlockFormat(bf);
        items.push_back(t);
        auto t2 = std::make_shared<TextItem>();
        t2->rect = QRectF(310, 360, 260, 200);
        t2->storyId = doc->createStory("");
        t->nextId = t2->id;
        items.push_back(t2);
        auto pic = std::make_shared<PictureItem>();
        pic->rect = QRectF(330, 580, 220, 160);
        pic->maskShape = "ellipse";
        items.push_back(pic);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = doc.get();
        ctx.cache = &cache;
        const QImage img = Renderer::renderToImage(ctx, 0, 1.5);
        QVERIFY(!img.isNull());
        const QString out = qEnvironmentVariable("JP_RENDER_OUT");
        if (!out.isEmpty()) img.save(out);
    }
};

JP_TEST_CLASS(PubFileTests)
#include "tst_pub_file.moc"
