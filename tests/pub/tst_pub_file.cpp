#include "tst_pub_file.h"

void PubFileTests::initTestCase() { initPubTestCase(); }

void PubFileTests::publisherSamplesImport_data()
{
    QTest::addColumn<QString>("path");
    QDir dir(QStringLiteral(JP_TEST_DATA "/pub"));
    for (const QString &f : dir.entryList({"*.pub"}, QDir::Files)) QTest::newRow(qPrintable(f)) << dir.filePath(f);
}

void PubFileTests::publisherSamplesImport()
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
void PubFileTests::pubWriterKeepsJpegPhotos()
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
void PubFileTests::firstPageNumberLoadsWithinTheLimits()
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
void PubFileTests::damagedStyleNamesStillOpen()
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
void PubFileTests::jpubHostileContent()
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
void PubFileTests::jpubPicturesAreHeldToThePictureChecks()
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


void PubFileTests::publisherFuzzFilesDoNotCrash()
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

void PubFileTests::privateCorpus()
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
void PubFileTests::templateOptions()
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
void PubFileTests::changeTemplate()
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


// Text that came from a publication (the author in its properties, the text of a message about a file) that looks like markup naming a picture is shown as the
// text it is: no label takes it for markup, so nothing is read from the place it names.
void PubFileTests::hostilePublicationTextIsShownAsText()
{
    using plainprobe::readsAPicture;
    QVERIFY(plainprobe::probeSeesTheTrap());
    jp::MainWindow w;
    w.resize(1400, 900);
    w.show();
    w.editor()->doc()->props.author = plainprobe::hostile();
    w.showBackstage(QStringLiteral("info"));
    QWidget *backstage = nullptr;
    for (QWidget *c : w.findChildren<QWidget *>())
        if (c->inherits("jp::Backstage")) backstage = c;
    QVERIFY(backstage && backstage->isVisible());
    int shown = 0;
    for (const QLabel *l : backstage->findChildren<QLabel *>()) {
        QVERIFY2(!readsAPicture(l), qPrintable(l->text()));
        shown += l->text().contains(QStringLiteral("probe.invalid"));
    }
    // (the Info page shows the author in a box to edit, which shows its text as text)
    for (const QLineEdit *e : backstage->findChildren<QLineEdit *>()) shown += e->text().contains(QStringLiteral("probe.invalid"));
    QVERIFY(shown >= 1);   // the author is there
    w.hideBackstage();
    // The box that says a file can't be opened shows its text as text (it names the file).
    const QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile f(dir.filePath(QStringLiteral("bad.jpub")));
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("not a publication");
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


// A web page or Markdown file put into a text box can name a picture (on this computer or on another); the picture is never read, however the box is drawn.
void PubFileTests::textFileImportNeverReadsThePicturesItNames()
{
    struct Probe : QAbstractFileEngineHandler {
        QStringList seen;
        std::unique_ptr<QAbstractFileEngine> create(const QString &name) const override
        {
            if (name.contains(QStringLiteral("probe-trap")) || name.contains(QStringLiteral("probe.invalid"))) const_cast<Probe *>(this)->seen << name;
            return nullptr;
        }
    } probe;
    const QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString local = QUrl::fromLocalFile(dir.filePath(QStringLiteral("probe-trap.png"))).toString();
    QImage(8, 8, QImage::Format_RGB32).save(dir.filePath(QStringLiteral("probe-trap.png")));
    probe.seen.clear();   // (saving the picture is ours)
    const QStringList pages = {
        QStringLiteral("<html><body><p>Before <img src=\"%1\"> and <img src=\"\\\\probe.invalid\\share\\x.png\"> after.</p></body></html>").arg(local),
        QStringLiteral("Before ![x](%1) and ![y](//probe.invalid/share/y.png) after.\n").arg(local)};
    // The probe can see: a plain text document that is laid out does read them.
    {
        QTextDocument control;
        control.setHtml(pages[0]);
        control.setTextWidth(300);
        QVERIFY(control.size().height() > 0);
        QVERIFY(!probe.seen.isEmpty());
        probe.seen.clear();
    }
    jp::MainWindow w;
    w.resize(1200, 800);
    w.show();
    jp::Editor *ed = w.editor();
    auto doc = jp::findTemplate(QStringLiteral("flyer-announce"))->build(jp::TemplateOptions());
    ed->setDocument(std::move(doc));
    for (int k = 0; k < pages.size(); ++k) {
        const QString path = dir.filePath(k == 0 ? QStringLiteral("page.html") : QStringLiteral("page.md"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(pages[k].toUtf8());
        f.close();
        for (const auto &it : ed->doc()->pages[0]->items)
            if (it->role == QLatin1String("body")) {
                QTextDocument *story = ed->doc()->storyDoc(static_cast<jp::TextItem *>(it.get())->storyId);
                QVERIFY(jp::loadTextFileInto(story, path));
                QVERIFY(story->toPlainText().contains(QStringLiteral("Before")));
                break;
            }
        w.grab();   // draws the page
        QApplication::processEvents();
    }
    QVERIFY2(probe.seen.isEmpty(), qPrintable(probe.seen.join(QLatin1Char('\n'))));
}


// The gallery Change Template opens is the New page's, in a mode of its
// own: its title and button say so, it offers designs and no blank
// sizes, its schemes default to the publication's own, and choosing a
// design asks whether to apply it here or to make a new publication.
void PubFileTests::changeTemplateGallery()
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


void PubFileTests::templatesFitTheirText()
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
void PubFileTests::pubTextIndexChains()
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
void PubFileTests::pub2000ScratchIsNotAPage()
{
    for (const char *name : {"poi-Sample2000.pub", "poi-Sample98.pub"}) {
        QString err;
        auto doc = jp::importPublisherFile(QStringLiteral(JP_TEST_DATA "/pub/") + QLatin1String(name), &err);
        QVERIFY2(doc, qPrintable(err));
        QCOMPARE(int(doc->pages.size()), 2);
        for (const auto &pg : doc->pages) QVERIFY(!pg->items.empty());
    }
}


// text fit in .pub lives in the story's record: 05 = 1 for best fit,
// 3 for shrink text on overflow, and a 0c flag for grow the box, as
// Publisher writes them when each setting is chosen. A box without one
// keeps its overflowing text hidden, as Publisher shows it.
void PubFileTests::pubAutofitSetting()
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
void PubFileTests::pubWriterRoundTrip()
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
void PubFileTests::pubWriterTextArt()
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
void PubFileTests::pubWriterShapeText()
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
void PubFileTests::pubWriterShapes()
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
void PubFileTests::pubWriterFills()
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
void PubFileTests::pubWriterPictures()
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
void PubFileTests::pubWriterLinked()
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
void PubFileTests::pubWriterTextFormats()
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
void PubFileTests::pubWriterTabsLists()
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
void PubFileTests::pubWriterMaster()
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
void PubFileTests::pubWriterHyphenation()
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
void PubFileTests::pubWriterStyles()
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


JP_TEST_CLASS(PubFileTests)
