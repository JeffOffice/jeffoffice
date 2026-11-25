#include "tst_pub_file.h"
#include "core/fontfiles.h"
#include "core/sfnt.h"
#include "office/eot.h"
#include <QRegularExpression>


// Two master pages: each page keeps its own master and its objects.
void PubFileTests::pubWriterTwoMasters()
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
void PubFileTests::pubWriterPages()
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
void PubFileTests::pubWriterTable()
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


// A table cell fill names its cell by the cell's number in the table's cell list (a merged cell counts once), so fills below the first row
// read back on the right cells.
void PubFileTests::pubTableFillsInLaterRows()
{
    auto doc = jp::Document::blank(QSizeF(612, 792));
    auto t = std::make_shared<jp::TableItem>();
    t->rows = 3;
    t->cols = 3;
    t->colW = {80, 90, 100};
    t->rowH = {30, 30, 30};
    t->rect = QRectF(72, 144, 0, 0);
    t->syncRect();
    t->cells.resize(9);
    for (auto &cl : t->cells) cl.storyId = doc->createStory(QStringLiteral("x"));
    const QColor red(200, 0, 0), green(0, 160, 0), blue(0, 0, 220);
    t->cell(1, 0).fill = jp::Fill::solid(jp::ColorRef::rgb(red));
    t->cell(2, 2).fill = jp::Fill::solid(jp::ColorRef::rgb(green));
    t->cell(1, 1).colSpan = 2;
    t->cell(1, 2).covered = true;
    t->cell(1, 1).fill = jp::Fill::solid(jp::ColorRef::rgb(blue));
    doc->pages[0]->items.push_back(t);

    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("fills-later-rows.pub"));
    QString err;
    QVERIFY2(jp::exportPublisher(*doc, path, &err), qPrintable(err));
    auto back = jp::importPublisherFile(path, &err);
    QVERIFY2(back, qPrintable(err));
    const jp::TableItem *bt = nullptr;
    jp::walkItems(back->pages[0]->items, [&](const jp::ItemPtr &it) {
        if (it->type() == jp::ItemType::Table) bt = static_cast<const jp::TableItem *>(it.get());
    });
    QVERIFY(bt);
    QCOMPARE(bt->rows, 3);
    QCOMPARE(bt->cols, 3);
    auto fillOf = [&](int r, int c) { return bt->cell(r, c).fill.type == jp::Fill::Solid ? bt->cell(r, c).fill.color.resolve(back->colors) : QColor(); };
    QCOMPARE(fillOf(1, 0), red);
    QCOMPARE(fillOf(1, 1), blue);
    QCOMPARE(fillOf(2, 2), green);
    QVERIFY(!fillOf(0, 0).isValid());
    QVERIFY(!fillOf(0, 2).isValid());
    QVERIFY(!fillOf(2, 0).isValid());
    QVERIFY(!fillOf(2, 1).isValid());
}


// Line weights and a list's indent are written as 32-bit lengths, so a damaged huge one stops at 1584 points
// (the largest weight Publisher offers) instead of wrapping around to something small or nothing.
void PubFileTests::pubLineWeightsAndListIndentsDoNotWrap()
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
void PubFileTests::pubWriterObjects()
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
void PubFileTests::pubInkPacking()
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
void PubFileTests::pubTextScaleRead()
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
void PubFileTests::pubWriterSpotAndScaling()
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
void PubFileTests::pubPreviewAndLanguages()
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
void PubFileTests::pubRightToLeft()
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
void PubFileTests::pubDashStylesRoundTrip()
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
void PubFileTests::pubPreviewThumbnail()
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
void PubFileTests::pubSpotColorRead()
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
void PubFileTests::pubWriterProcessInks()
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
void PubFileTests::pubGradientsRoundTrip()
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
void PubFileTests::pubPictureTurnsAndFill()
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
void PubFileTests::pubFormulaChainIsFast()
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
    const QByteArray rebuiltFile = cfb::write(file);
    const auto steps = libmspub::calculationsDoneForTest();
    auto back = importPublisher(rebuiltFile, nullptr);
    QVERIFY(back);
    // Counted: each of the 22 formulas is worked out once for each point it is asked for (3 points), not afresh at every reference (2 to the 22nd).
    const auto done = libmspub::calculationsDoneForTest() - steps;
    QVERIFY2(done > 0 && done <= 22 * 3 * 4, qPrintable(QString::number(done)));
}


// A freeform's coordinate space (0x0140-0x0143) is any 32-bit range;
// its width came from a 32-bit subtraction, which overflows (undefined
// behavior; the sanitizer build reports it) for -2^31 to 2^31-1.
void PubFileTests::pubHugeCoordinateSpace()
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
namespace {
// A publication in the oldest layout, made here: a document chunk, page chunks (sizes in EMU, 0 = none stored), and one rectangle on each page whose
// parent is given. No text and no Quill streams, as the oldest files have.
// With a marker, a record, and quill set, the file is the one with the Quill part (the 2k reader): the header word at 0x0c is the marker, and each
// shape is a record of recSize bytes that starts with the bytes in rec (from offset 6) instead of the solid-fill byte at 0x20.
QByteArray oldLayoutPub(const QVector<QPair<quint16, quint32>> &pages, const QVector<QPair<quint16, quint16>> &shapes, quint16 marker = 0,
                        const QByteArray &rec = QByteArray(), int recSize = 0x100, bool quill = false, quint16 groupType = 0,
                        const QVector<quint16> &pageList = {}, const QVector<quint32> &palette = {}, bool groupImage = false)
{
    QByteArray c(0x100, '\0');
    auto put32 = [](QByteArray &b, int at, quint32 v) { for (int i = 0; i < 4; ++i) b[at + i] = char((v >> (8 * i)) & 0xff); };
    auto put16 = [](QByteArray &b, int at, quint32 v) { b[at] = char(v & 0xff); b[at + 1] = char((v >> 8) & 0xff); };
    c[0] = char(0xe8);
    c[1] = char(0xac);
    c[2] = 0x22;
    put16(c, 0x0c, marker);
    put32(c, 0x12, 0x40);   // where the (empty) text part is
    struct Chunk { quint16 id, parent; quint32 at; };
    QVector<Chunk> chunks;
    auto add = [&](quint16 id, quint16 parent, quint16 type, int size) {
        chunks.push_back({id, parent, quint32(c.size())});
        QByteArray b(size, '\0');
        put16(b, 0, type);
        c.append(b);
        return int(chunks.back().at);
    };
    // (a page list ends the document chunk: the count twice, 2, two zero words, then the ids)
    const int doc = add(1, 0, 0x15, pageList.isEmpty() ? 0x40 : 0x44 + 10 + 2 * pageList.size());
    put32(c, doc + 0x14, 8 * 914400);
    put32(c, doc + 0x18, 10 * 914400);
    if (!pageList.isEmpty()) {
        const int at = doc + 0x44;
        put16(c, at, quint32(pageList.size()));
        put16(c, at + 2, quint32(pageList.size()));
        put16(c, at + 4, 2);
        for (int i = 0; i < pageList.size(); ++i)
            put16(c, at + 10 + 2 * i, pageList[i]);
    }
    if (!palette.isEmpty()) {   // a palette chunk: eight colors from 0xA0
        const int at = add(2, 1, 0x47, 0xA0 + 8 * 4);
        for (int i = 0; i < palette.size() && i < 8; ++i)
            put32(c, at + 0xA0 + 4 * i, palette[i]);
    }
    for (const auto &p : pages) {
        const int at = add(p.first, 1, 0x14, 0x40);
        put32(c, at + 12, p.second);
        put32(c, at + 16, p.second);
    }
    if (groupType)   // a group chunk with id 30 on the last page
        add(30, pages.last().first, groupType, 0x40);
    if (groupType && groupImage) {   // a picture inside the group: a metafile with only its header and end record
        const int img = add(31, 30, 0x02, 0x40);
        const quint32 origin = 25 * 914400;
        put32(c, img + 6, origin + 914400);
        put32(c, img + 10, origin + 914400);
        put32(c, img + 14, origin + 3 * 914400);
        put32(c, img + 18, origin + 3 * 914400);
        QByteArray wmf(18, '\0');
        wmf[0] = 1;
        wmf[2] = 9;
        wmf[5] = 3;
        wmf.append(QByteArray::fromHex("030000000000"));
        const int data = add(32, 31, 0x21, 8 + wmf.size());
        put32(c, data + 4, quint32(wmf.size()));
        for (int i = 0; i < wmf.size(); ++i)
            c[data + 8 + i] = wmf[i];
    }
    for (const auto &s : shapes) {
        const int at = add(s.first, s.second, 0x05, recSize);
        if (rec.isEmpty())
            c[at + 0x20] = 2;   // a solid fill, so the rectangle shows
        else
            for (int i = 0; i < rec.size() && 6 + i < recSize; ++i)
                c[at + 6 + i] = rec[i];
        const quint32 origin = 25 * 914400;   // the oldest layout counts from 25 inches before the page
        put32(c, at + 6, origin + 914400);
        put32(c, at + 10, origin + 914400);
        put32(c, at + 14, origin + 3 * 914400);
        put32(c, at + 18, origin + 3 * 914400);
        for (int i = 0; i < rec.size() && 6 + i < recSize; ++i)
            if (6 + i >= 0x16)
                c[at + 6 + i] = rec[i];
    }
    c.resize(c.size() + 0x10);
    const quint32 trailer = quint32(c.size());
    put32(c, 0x16, trailer);
    QByteArray t(2, '\0');
    put16(t, 0, quint32(chunks.size()));
    for (const Chunk &k : chunks) {
        QByteArray e(10, '\0');
        put16(e, 2, k.id);
        put16(e, 4, k.parent);
        put32(e, 6, k.at);
        t.append(e);
    }
    c.append(t);
    jp::cfb::File f;
    jp::cfb::Entry root;
    root.type = jp::cfb::Entry::Root;
    root.name = QStringLiteral("Root Entry");
    root.children << 1;
    jp::cfb::Entry st;
    st.name = QStringLiteral("Contents");
    st.data = c;
    f.entries << root << st;
    if (quill) {
        // (a Quill part with no chunks: the first chunk list, at 0x18, holds none and ends)
        QByteArray q(0x20, '\0');
        put32(q, 0x1a + 2, 0xffffffffu);
        jp::cfb::Entry a, b, d;
        a.type = jp::cfb::Entry::Storage;
        a.name = QStringLiteral("Quill");
        b.type = jp::cfb::Entry::Storage;
        b.name = QStringLiteral("QuillSub");
        d.name = QStringLiteral("CONTENTS");
        d.data = q;
        f.entries[0].children << 2;
        f.entries << a << b << d;
        f.entries[2].children << 3;
        f.entries[3].children << 4;
    }
    return jp::cfb::write(f);
}

// The rectangles an imported older-layout file holds, wherever they sit (the importer may put them in a group).
void collectItems(const ItemList &list, QVector<const Item *> &out)
{
    for (const auto &it : list) {
        out << it.get();
        if (auto g = dynamic_cast<const GroupItem *>(it.get()))
            collectItems(g->children, out);
    }
}
} // namespace

// The oldest layout stores no canvas size in any page chunk: the first page chunk is the scratch area and the others are pages (the shape on the
// second page is read). When some page chunks do store a size, a page without one is still the scratch area.
void PubFileTests::pubOldLayoutPagesWithoutSize()
{
    PubImportReport rep;
    QString err;
    auto doc = importPublisher(oldLayoutPub({{10, 0}, {11, 0}}, {{20, 11}}), &err, &rep);
    QVERIFY2(doc, qPrintable(err));
    QCOMPARE(rep.shapes, 1);
    QCOMPARE(doc->pages.size(), 1);
    PubImportReport sized;
    auto other = importPublisher(oldLayoutPub({{10, 0}, {11, 5 * 914400}}, {{20, 10}, {21, 11}}), &err, &sized);
    QVERIFY2(other, qPrintable(err));
    QCOMPARE(sized.shapes, 1);
}

// The older layouts keep a shape's fill, line, and padding at offsets that depend on the header word at 0x0c: 0x0268 puts the two fill colors at
// 0x20 and 0x24, the fill kind at 0x28, and the first line's width and color at 0x2A, where 0x02cd has them two bytes later. Made records: a solid fill
// with a known color and a 3-point line; a record too short to hold a field is read without that field.
void PubFileTests::pubOldLayoutFillOffsetsByMarker()
{
    auto record = [](int fillAt, int kindAt, int lineAt) {
        QByteArray r(0x41 - 6, '\0');
        auto put = [&](int at, quint32 v) { for (int i = 0; i < 4; ++i) r[at - 6 + i] = char((v >> (8 * i)) & 0xff); };
        put(fillAt, 0x20336699);   // an explicit color: red 0x99, green 0x66, blue 0x33
        r[kindAt - 6] = 2;
        r[lineAt - 6] = 3;   // a 3-point line
        put(lineAt + 1, 0x20ff0000);   // blue
        for (int at : {0x35, 0x3b, 0x41})   // the top, right, and bottom lines, the same in both
            if (at + 5 <= 0x46) {
                r.resize(qMax(r.size(), at - 6 + 5));
                r[at - 6] = 3;
                for (int i = 0; i < 4; ++i)
                    r[at - 5 + i] = char((0x20ff0000 >> (8 * i)) & 0xff);
            }
        return r;
    };
    QString err;
    auto first = [&](const QByteArray &bytes) -> std::shared_ptr<Document> {
        auto d = importPublisher(bytes, &err);
        return d;
    };
    const QColor want(0x99, 0x66, 0x33);
    for (quint16 marker : {quint16(0x0268), quint16(0x02cd)}) {
        const bool early = marker == 0x0268;
        auto doc = first(oldLayoutPub({{10, 0}, {11, 5 * 914400}}, {{20, 11}}, marker, record(early ? 0x20 : 0x22, early ? 0x28 : 0x2A, early ? 0x2A : 0x2C), 0x46, true));
        QVERIFY2(doc, qPrintable(err));
        QCOMPARE(doc->pages.size(), 1);
        QVector<const Item *> all;
        collectItems(doc->pages[0]->items, all);
        const Item *got = nullptr;
        for (const Item *it : all)
            if (!it->fill.isNone()) got = it;
        QVERIFY2(got, qPrintable(QStringLiteral("no filled shape, marker %1").arg(marker)));
        QCOMPARE(got->fill.color.rgbValue(), want);
        QVERIFY(!got->stroke.isNone());
        QVERIFY2(std::abs(got->stroke.width - 3) < 0.5, qPrintable(QString::number(got->stroke.width)));
    }
    // (a record that ends before the fill kind has no fill and no line: nothing to show, so the file is not opened as a page with it)
    auto cut = first(oldLayoutPub({{10, 0}, {11, 5 * 914400}}, {{20, 11}}, 0x0268, record(0x20, 0x28, 0x2A), 0x24, true));
    QVERIFY(!cut);
    QVector<const Item *> shortItems;
    // (a record that ends right after the fill kind: the color is there, the line is not)
    auto shortLine = first(oldLayoutPub({{10, 0}, {11, 5 * 914400}}, {{20, 11}}, 0x0268, record(0x20, 0x28, 0x2A), 0x2A, true));
    QVERIFY2(shortLine, qPrintable(err));
    collectItems(shortLine->pages[0]->items, shortItems);
    QVERIFY(!shortItems.isEmpty());
    for (const Item *it : shortItems)
        QVERIFY(it->stroke.isNone());
}


// A chunk of type 0x0e holds shapes, like a group (0x0f) does: the shape inside it is read; a chunk of another unknown type holds nothing.
void PubFileTests::pubOldLayoutGroupMarker0e()
{
    QString err;
    for (quint16 type : {quint16(0x0e), quint16(0x0f), quint16(0x0d)}) {
        PubImportReport rep;
        QByteArray rec(0x41 - 6, '\0');
        rec[0x28 - 6] = 2;
        auto doc = importPublisher(oldLayoutPub({{10, 0}, {11, 5 * 914400}}, {{31, 30}}, 0x0268, rec, 0x46, true, type), &err, &rep);
        QVERIFY2(doc, qPrintable(err));
        QCOMPARE(rep.shapes, type == 0x0d ? 0 : 1);
    }
}

// The older layouts end the document chunk with the page list: the first id is a blank page, the second the master page, the last three are special
// pages, and the ids between are the pages. Seven ids name two pages; a list that does not hold together (a count that disagrees with the one before
// it, or a page that is no page chunk) is not used, and the pages are then found as before.
void PubFileTests::pubOldLayoutPageListNamesThePages()
{
    QString err;
    QByteArray rec(0x41 - 6, '\0');
    rec[0x2A - 6] = 2;
    const QVector<QPair<quint16, quint32>> pages = {{10, 0}, {11, 5 * 914400}, {12, 5 * 914400}, {13, 5 * 914400}, {14, 5 * 914400}, {15, 5 * 914400}, {16, 5 * 914400}};
    const QVector<QPair<quint16, quint16>> shapes = {{20, 13}, {21, 14}, {22, 15}, {23, 16}};
    auto pageCount = [&](const QVector<quint16> &list) {
        PubImportReport rep;
        auto doc = importPublisher(oldLayoutPub(pages, shapes, 0x02cd, rec, 0x46, true, 0, list), &err, &rep);
        return doc ? int(doc->pages.size()) : -1;
    };
    QCOMPARE(pageCount({11, 12, 13, 14, 15, 16, 99}), 2);
    QCOMPARE(pageCount({}), 4);
    QCOMPARE(pageCount({11, 12, 13, 14, 99, 98, 97}), 2);   // (the ids between the first two and the last three: 13 and 14)
    QCOMPARE(pageCount({11, 12, 13, 14, 15, 16, 17, 18, 19}), 4);   // (page 17 is no page chunk: the list is not used)
    QCOMPARE(pageCount({11, 12, 13, 99}), 4);   // (too short to hold a page between its ends)
}

// A palette color's third byte is its tint: 0xE4 is the color itself, 0x80 plus the percent mixes it with white (0xBC is 60 percent).
void PubFileTests::pubOldLayoutPaletteTint()
{
    QString err;
    auto fillOf = [&](quint32 colorWord) {
        QByteArray rec(0x41 - 6, '\0');
        rec[0x2A - 6] = 2;
        for (int i = 0; i < 4; ++i)
            rec[0x22 - 6 + i] = char((colorWord >> (8 * i)) & 0xff);
        auto doc = importPublisher(oldLayoutPub({{10, 0}, {11, 5 * 914400}}, {{20, 11}}, 0x02cd, rec, 0x46, true, 0, {}, {0, 0x20336600}), &err);
        QColor got;
        if (doc && !doc->pages.isEmpty()) {
            QVector<const Item *> all;
            collectItems(doc->pages[0]->items, all);
            for (const Item *it : all)
                if (!it->fill.isNone())
                    got = it->fill.color.rgbValue();
        }
        return got;
    };
    const QColor whole = fillOf(0xC0E40001);   // palette entry 1, no tint
    QCOMPARE(whole, QColor(0x00, 0x66, 0x33));
    const QColor tinted = fillOf(0xC0BC0001);   // 60 percent
    QVERIFY(tinted.isValid());
    QVERIFY2(std::abs(tinted.red() - 0x66) <= 2 && std::abs(tinted.green() - 0xA3) <= 2 && std::abs(tinted.blue() - 0x85) <= 2, qPrintable(tinted.name()));
    const QColor faint = fillOf(0xC0940001);   // 20 percent
    QVERIFY2(std::abs(faint.red() - 0xCC) <= 2 && std::abs(faint.green() - 0xE0) <= 2 && std::abs(faint.blue() - 0xD6) <= 2, qPrintable(faint.name()));
}

// Fill kinds 0x8A and 0x8C are shaded fills from the first color to the second, one running down the shape and the other across it; the kinds this
// has not seen in a picture stay unfilled.
void PubFileTests::pubOldLayoutShadedFills()
{
    QString err;
    auto fillOf = [&](unsigned char kind) -> Fill {
        QByteArray rec(0x41 - 6, '\0');
        rec[0x2A - 6] = char(kind);
        const quint32 first = 0x20336600, second = 0x20ffcc99;
        for (int i = 0; i < 4; ++i) {
            rec[0x22 - 6 + i] = char((first >> (8 * i)) & 0xff);
            rec[0x26 - 6 + i] = char((second >> (8 * i)) & 0xff);
        }
        auto doc = importPublisher(oldLayoutPub({{10, 0}, {11, 5 * 914400}}, {{20, 11}}, 0x02cd, rec, 0x46, true), &err);
        Fill got = Fill::none();
        if (doc && !doc->pages.isEmpty()) {
            QVector<const Item *> all;
            collectItems(doc->pages[0]->items, all);
            for (const Item *it : all)
                if (!it->fill.isNone())
                    got = it->fill;
        }
        return got;
    };
    const Fill down = fillOf(0x8A), across = fillOf(0x8C);
    QCOMPARE(int(down.type), int(Fill::Gradient));
    QCOMPARE(int(across.type), int(Fill::Gradient));
    QCOMPARE(down.color.rgbValue(), QColor(0x00, 0x66, 0x33));
    QCOMPARE(down.color2.rgbValue(), QColor(0x99, 0xCC, 0xFF));
    QCOMPARE(down.angle, 90.0);   // top to bottom
    QCOMPARE(across.angle, 0.0);   // left to right
    QVERIFY(fillOf(0x8B).isNone());
}

// A picture inside a group is read: older layouts keep the picture's own chunk (type 2) under the group's.
void PubFileTests::pubOldLayoutPictureInGroup()
{
    QString err;
    PubImportReport rep;
    QByteArray rec(0x41 - 6, '\0');
    rec[0x2A - 6] = 2;
    auto doc = importPublisher(oldLayoutPub({{10, 0}, {11, 5 * 914400}}, {{31 + 10, 30}}, 0x02cd, rec, 0x46, true, 0x0f, {}, {}, true), &err, &rep);
    QVERIFY2(doc, qPrintable(err));
    QCOMPARE(rep.pictures, 1);
}

// A page marked 0 is a special one, but the first page listed is a real page: a file can list four special pages after a single page marked 0.
void PubFileTests::pubFirstPageMarkedSpecialIsKept()
{
    auto doc = Document::blank(QSizeF(612, 792));
    auto s = std::make_shared<ShapeItem>();
    s->shape = QStringLiteral("rect");
    s->rect = QRectF(100, 100, 200, 150);
    s->fill = Fill::solid(ColorRef::rgb(Qt::red));
    doc->pages[0]->items.push_back(s);
    QTemporaryDir dir;
    const QString name = dir.filePath(QStringLiteral("marked.pub"));
    QString err;
    QVERIFY2(exportPublisher(*doc, name, &err), qPrintable(err));
    QFile f(name);
    QVERIFY(f.open(QIODevice::ReadOnly));
    cfb::File file;
    QVERIFY(cfb::read(f.readAll(), &file, &err));
    f.close();
    QByteArray contents = file.stream(QStringLiteral("Contents"));
    // (the page's mark: block 6, 8 bytes, 2 = a normal page, then its index, 0)
    const QByteArray mark = QByteArray("\x06\x28\x02\0\0\0\0\0\0\0", 10);
    const int at = contents.indexOf(mark);
    QVERIFY(at >= 0);
    QCOMPARE(contents.indexOf(mark, at + 1), -1);
    contents[at + 2] = 0;
    QVERIFY(file.setStream(QStringLiteral("Contents"), contents));
    PubImportReport rep;
    auto back = importPublisher(cfb::write(file), &err, &rep);
    QVERIFY2(back, qPrintable(err));
    QCOMPARE(rep.shapes, 1);
}

// A file that holds a story and a shape but puts none on a page is refused with a plain message, not opened as blank pages that could be saved over it
// (a file with nothing in it opens as it always did), and the window that is asked to open it says so and keeps the file as it was.
void PubFileTests::pubContentWithNoObjectIsRefused()
{
    auto doc = Document::blank(QSizeF(612, 792));
    auto t = std::make_shared<TextItem>();
    t->rect = QRectF(100, 100, 200, 100);
    t->storyId = doc->createStory(QStringLiteral("Some words on the page"));
    doc->pages[0]->items.push_back(t);
    QTemporaryDir dir;
    const QString name = dir.filePath(QStringLiteral("unplaced.pub"));
    QString err;
    QVERIFY2(exportPublisher(*doc, name, &err), qPrintable(err));
    QFile f(name);
    QVERIFY(f.open(QIODevice::ReadOnly));
    cfb::File file;
    QVERIFY(cfb::read(f.readAll(), &file, &err));
    f.close();
    QByteArray contents = file.stream(QStringLiteral("Contents"));
    // (the page's list of its shapes: a count of 1, the list, and its one reference: made to name a shape that is not there)
    const QByteArray list = QByteArray("\x01\x20\x01\0\0\0\x02\xa0", 8);
    const int at = contents.indexOf(list);
    QVERIFY(at >= 0);
    QCOMPARE(contents.mid(at + 12, 2), QByteArray("\x00\x70", 2));
    contents[at + 14] = char(0xfe);
    contents[at + 15] = char(0xff);
    QVERIFY(file.setStream(QStringLiteral("Contents"), contents));
    const QByteArray bytes = cfb::write(file);
    PubImportReport rep;
    auto back = importPublisher(bytes, &err, &rep);
    QVERIFY(!back);
    QVERIFY2(err.contains(QStringLiteral("can't place")), qPrintable(err));
    QVERIFY(rep.fileStories > 0 || rep.fileRecords > 0);
    // (a file with no story and no shape is not refused)
    QVERIFY(importPublisher(oldLayoutPub({{10, 0}, {11, 0}}, {}), &err));
    const QString path = dir.filePath(QStringLiteral("unplaced2.pub"));
    QFile w(path);
    QVERIFY(w.open(QIODevice::WriteOnly));
    w.write(bytes);
    w.close();
    jp::MainWindow win;
    QStringList told;
    win.setNotifier([&](const QString &, const QString &text) { told << text; });
    QVERIFY(!win.openFile(path));
    QVERIFY2(told.join(QLatin1Char('|')).contains(QStringLiteral("can't place")), qPrintable(told.join(QLatin1Char('|'))));
    QFile r(path);
    QVERIFY(r.open(QIODevice::ReadOnly));
    QCOMPARE(r.readAll(), bytes);
}


void PubFileTests::pubShadowFromLineColor()
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
void PubFileTests::pubLineCornersRoundTrip()
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


// A compound line (double, thick-thin, thin-thick, triple) goes through a .pub: Publisher's page frames store one (0x01CD) and JeffPub drew a single heavy line.
void PubFileTests::pubCompoundLinesRoundTrip()
{
    auto doc = Document::blank(QSizeF(612, 792));
    const Stroke::Compound kinds[] = {Stroke::Single, Stroke::Double, Stroke::ThickThin, Stroke::ThinThick, Stroke::Triple};
    for (int i = 0; i < 5; ++i) {
        auto s = std::make_shared<ShapeItem>();
        s->shape = QStringLiteral("rect");
        s->rect = QRectF(40 + 100 * i, 100, 80, 100);
        s->fill = Fill::none();
        s->stroke.color = ColorRef::rgb(Qt::black);
        s->stroke.width = 6;
        s->stroke.compound = kinds[i];
        doc->pages[0]->items.push_back(s);
    }
    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("compound.pub"));
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
    QCOMPARE(got.size(), 5);
    for (int i = 0; i < 5; ++i) QCOMPARE(got[i]->stroke.compound, kinds[i]);
    // A file can store any number there: one that is no kind of line is a single line, and nothing else changes.
    cfb::File file;
    QVERIFY(f.seek(0));
    QVERIFY(cfb::read(f.readAll(), &file, &err));
    QByteArray escher = cfb::readStream(path, QStringLiteral("Escher/EscherStm"));
    auto prop = [](quint16 id, quint32 value) {
        QByteArray b(6, 0);
        qToLittleEndian<quint16>(id, b.data());
        qToLittleEndian<quint32>(value, b.data() + 2);
        return b;
    };
    QCOMPARE(int(escher.count(prop(0x01cd, 4))), 1);
    escher.replace(prop(0x01cd, 4), prop(0x01cd, 0xFFFFFFF0u));
    QVERIFY(file.setStream(QStringLiteral("Escher/EscherStm"), escher));
    auto hostile = importPublisher(cfb::write(file), nullptr);
    QVERIFY(hostile);
    int kindsSeen[5] = {0, 0, 0, 0, 0};
    for (const auto &it : hostile->pages[0]->items)
        if (!it->stroke.isNone()) kindsSeen[int(it->stroke.compound)]++;
    QCOMPARE(kindsSeen[0], 2);   // the single line and the one with the impossible kind
    QCOMPARE(kindsSeen[1] + kindsSeen[2] + kindsSeen[3], 3);
    QCOMPARE(kindsSeen[4], 0);
}


// A compound line is drawn in its bands, the outer edge on the outline's edge's outer side: a triple line of 12 points is a thin line, a gap, a line twice as thick, a gap, and a thin line (a sixth, a sixth, a third, a sixth, a sixth); a thick-thin line has its thick band outside.
void PubFileTests::compoundLinesAreDrawnInBands()
{
    jp::initCore();
    auto draw = [](Stroke::Compound kind) {
        auto doc = Document::blank(QSizeF(612, 792));
        auto s = std::make_shared<ShapeItem>();
        s->shape = QStringLiteral("rect");
        s->rect = QRectF(100, 100, 200, 100);
        s->fill = Fill::none();
        s->stroke.color = ColorRef::rgb(Qt::black);
        s->stroke.width = 10;
        s->stroke.compound = kind;
        doc->pages[0]->items.push_back(s);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = doc.get();
        ctx.cache = &cache;
        ctx.opt.output = true;
        return Renderer::renderToImage(ctx, 0, 4.0);
    };
    // One row across the left edge, from 5 points outside the edge to 5 points inside, sampled in the middle of each point of it.
    auto row = [](const QImage &img) {
        QString r;
        for (int pt = 0; pt < 10; ++pt) r += QColor(img.pixel(int((95 + pt + 0.5) * 4), 600)).value() < 128 ? 'x' : '.';
        return r;
    };
    QCOMPARE(row(draw(Stroke::Single)), QStringLiteral("xxxxxxxxxx"));
    QCOMPARE(row(draw(Stroke::Double)), QStringLiteral("xxx....xxx"));
    QCOMPARE(row(draw(Stroke::ThickThin)), QStringLiteral("xxxxxxx.xx"));
    QCOMPARE(row(draw(Stroke::ThinThick)), QStringLiteral("xx.xxxxxxx"));
    QCOMPARE(row(draw(Stroke::Triple)), QStringLiteral("xx.xxxx.xx"));
}



// An empty rectangle's outline comes back on its frame: libmspub moved
// the outline's top edge out by half the line (it applies one edge per
// side line, and a plain outline is one line), so a sign's 12-pt frame
// drawn inside its edge sat 6 pt too high.
void PubFileTests::pubOutlineKeepsFrame()
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
void PubFileTests::pubGradientAnglesAsPublisher()
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


// A picture's Transparency setting is the tertiary property 0x0120, the share of the picture that shows (16.16: 0x3333 is 20%). A file
// with it reads as a see-through picture, and saves and reopens with the same opacity. The test takes a saved picture with a tinted
// recolor (one tertiary property, 6 bytes) and writes 0x0120 in its place.
void PubFileTests::pubPictureOpacityFromTertiaryProperty()
{
    auto doc = Document::blank(QSizeF(612, 792));
    QImage img(8, 8, QImage::Format_RGB32);
    img.fill(QColor(30, 90, 200));
    QByteArray png;
    {
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "PNG");
    }
    auto pic = std::make_shared<PictureItem>();
    pic->imageId = doc->addImage(png, QStringLiteral("png"));
    pic->rect = QRectF(72, 72, 120, 90);
    pic->recolor = PictureItem::ColorTint;
    pic->recolorColor = ColorRef::rgb(QColor(10, 20, 30));
    doc->pages[0]->items.push_back(pic);
    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("opacity.pub"));
    QString err;
    QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
    auto prop = [](quint16 id, quint32 value) {
        QByteArray b(6, 0);
        qToLittleEndian<quint16>(id, b.data());
        qToLittleEndian<quint32>(value, b.data() + 2);
        return b;
    };
    // The transparency of the document's one picture, or -1 when it has none.
    auto transparencyOf = [](const std::unique_ptr<Document> &d) {
        double found = -1;
        if (d)
            for (const auto &it : d->pages[0]->items)
                if (auto *p = dynamic_cast<const PictureItem *>(it.get())) found = p->transparency;
        return found;
    };
    QByteArray escher = cfb::readStream(path, QStringLiteral("Escher/EscherStm"));
    const QByteArray tint = prop(0x011a, 0x001e140a);
    QCOMPARE(int(escher.count(tint)), 1);
    {
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(transparencyOf(importPublisher(f.readAll(), nullptr)), 0.0);
    }
    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray bytes = f.readAll();
    cfb::File file;
    QVERIFY(cfb::read(bytes, &file, &err));
    escher.replace(tint, prop(0x0120, 0x3333));
    QVERIFY(file.setStream(QStringLiteral("Escher/EscherStm"), escher));
    auto opened = importPublisher(cfb::write(file), nullptr);
    const double got = transparencyOf(opened);
    QVERIFY2(std::abs(got - 0.8) < 0.001, qPrintable(QString::number(got)));
    // Saved and reopened (through the app's own writer), it keeps its opacity.
    const QString again = dir.filePath(QStringLiteral("again.pub"));
    QVERIFY2(exportPublisher(*opened, again, &err), qPrintable(err));
    QFile g(again);
    QVERIFY(g.open(QIODevice::ReadOnly));
    const double kept = transparencyOf(importPublisher(g.readAll(), nullptr));
    QVERIFY2(std::abs(kept - 0.8) < 0.01, qPrintable(QString::number(kept)));
    // A picture frame that also has a picture fill with its own opacity (property 0x0182, 50%): the picture's Transparency setting wins.
    {
        QByteArray both = escher;
        both.replace(prop(0x0181, 0x08000001), prop(0x0180, 3));
        both.replace(prop(0x0183, 0x08000007), prop(0x0182, 0x8000));
        QVERIFY(file.setStream(QStringLiteral("Escher/EscherStm"), both));
        const double won = transparencyOf(importPublisher(cfb::write(file), nullptr));
        QVERIFY2(std::abs(won - 0.8) < 0.001, qPrintable(QString::number(won)));
    }
    // A property that says the picture is fully shown, or names more than that, changes nothing.
    for (quint32 v : {0x10000u, 0x20000u}) {
        QByteArray e2 = escher;
        e2.replace(prop(0x0120, 0x3333), prop(0x0120, v));
        QVERIFY(file.setStream(QStringLiteral("Escher/EscherStm"), e2));
        QCOMPARE(transparencyOf(importPublisher(cfb::write(file), nullptr)), 0.0);
    }
}

// A straight line from Publisher's line tool (shape 20, two points) comes
// back as a line with flat ends. It came in as a closed outline of two
// points, and the join where that outline turns back on itself pushed a
// spike out past each end: a car wash flyer's 18-pt bars stuck out of
// their frame by twice their width.
void PubFileTests::pubStraightLineIsALine()
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
void PubFileTests::pubTextWithoutSizeIs12()
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
void PubFileTests::pubSuperscriptHighByte()
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
void PubFileTests::pubLinkKeepsLinkColor()
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
void PubFileTests::pubNoneNumberingIsNoList()
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

void PubFileTests::colorRefRoundTrip()
{
    const ColorRef a = ColorRef::scheme(Accent2, 40);
    QCOMPARE(ColorRef::fromString(a.toString()), a);
    QCOMPARE(ColorRef::fromString("#1F5FAD").resolve(builtinColorSchemes()[0]), QColor("#1F5FAD"));
    QVERIFY(builtinColorSchemes().size() >= 90);
}

void PubFileTests::documentRoundTrip()
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

void PubFileTests::layoutFlowsAcrossFrames()
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

void PubFileTests::fieldsDisplay()
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

void PubFileTests::renderSmoke()
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


// What libmspub hands over for a font the .pub carries (its name and its Embedded OpenType, as base64): a font of the computer's own kind is taken as a face of the style
// it says, and a part that is not a font is a note, not a failure. (The importer gives each carried font to this one function.)
namespace {
librevenge::RVNGPropertyList pubFontProps(const QString &name, const QByteArray &eot)
{
    librevenge::RVNGPropertyList p;
    p.insert("librevenge:name", librevenge::RVNGString(name.toUtf8().constData()));
    p.insert("office:binary-data", librevenge::RVNGBinaryData(reinterpret_cast<const unsigned char *>(eot.constData()), static_cast<unsigned long>(eot.size())));
    return p;
}
QByteArray pubLatoFile(bool bold)
{
    QFile f(jp::bundledFontFile(QStringLiteral("Lato"), bold, false).path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}
} // namespace

void PubFileTests::aPublicationsEmbeddedFontsAreTakenWithinTheLimits()
{
    const QByteArray regular = jp::fontWithFamilyName(pubLatoFile(false), QStringLiteral("Zobster")), bold = jp::fontWithFamilyName(pubLatoFile(true), QStringLiteral("Zobster"));
    QVERIFY(!regular.isEmpty() && !bold.isEmpty());
    jp::office::EmbeddedFaceIntake intake;
    jp::takePubEmbeddedFont(intake, pubFontProps(QStringLiteral("Zobster"), jp::office::makeEot(regular)));
    jp::takePubEmbeddedFont(intake, pubFontProps(QStringLiteral("Zobster"), jp::office::makeEot(bold)));
    jp::takePubEmbeddedFont(intake, pubFontProps(QStringLiteral("Junk"), QByteArray(3000, 'j')));
    jp::takePubEmbeddedFont(intake, librevenge::RVNGPropertyList());   // (no data at all)
    QStringList notes;
    const auto faces = intake.take(&notes);
    QCOMPARE(faces.size(), 2);
    QCOMPARE(faces[0].typeface, QStringLiteral("Zobster"));
    QCOMPARE(int(faces[0].slot), int(jp::office::RegularSlot));
    QCOMPARE(int(faces[1].slot), int(jp::office::BoldSlot));
    QCOMPARE(notes.size(), 1);
    QVERIFY2(notes[0].contains(QStringLiteral("can't be used")), qPrintable(notes[0]));
    // The publication's own while it is open: private name, never installed, gone when released.
    const int before = jp::office::EmbeddedFonts::addedFontCount();
    int owner = 0;
    const auto aliases = jp::office::EmbeddedFonts::use(&owner, faces);
    QVERIFY(aliases.contains(QStringLiteral("zobster")));
    QVERIFY(!QFontDatabase::families().contains(QStringLiteral("Zobster")));
    QVERIFY(QFontDatabase::families().contains(aliases.value(QStringLiteral("zobster"))));
    jp::office::EmbeddedFonts::release(&owner);
    QCOMPARE(jp::office::EmbeddedFonts::addedFontCount(), before);
}

// Thousands of fonts, and one whose text is larger than a font may be: at most the most are taken, and the oversize part is never decoded.
void PubFileTests::aHostilePublicationFontListIsBounded()
{
    const QByteArray eot = jp::office::makeEot(pubLatoFile(false));
    jp::office::EmbeddedFaceIntake intake;
    for (int i = 0; i < 3000; ++i) jp::takePubEmbeddedFont(intake, pubFontProps(QStringLiteral("F%1").arg(i), eot));
    // (a base64 text for more than a font may be: 4 characters a 3 bytes, over the limit)
    librevenge::RVNGPropertyList big;
    big.insert("librevenge:name", "Big");
    big.insert("office:binary-data", librevenge::RVNGBinaryData(reinterpret_cast<const unsigned char *>(QByteArray(int(jp::kMaxEmbeddedFontBytes) + 200000, 'x').constData()), static_cast<unsigned long>(jp::kMaxEmbeddedFontBytes) + 200000));
    jp::office::EmbeddedFaceIntake over;
    QVERIFY(!jp::takePubEmbeddedFont(over, big));
    QCOMPARE(over.decodedCount(), 0);
    QStringList overNotes;
    QCOMPARE(over.take(&overNotes).size(), 0);
    QVERIFY2(overNotes.join(QLatin1Char('|')).contains(QStringLiteral("they are too large")), qPrintable(overNotes.join(QLatin1Char('|'))));
    QCOMPARE(intake.decodedCount(), jp::kMaxEmbeddedFonts);
    QStringList notes;
    QCOMPARE(intake.take(&notes).size(), jp::kMaxEmbeddedFonts);
    QVERIFY2(QRegularExpression(QStringLiteral("Only the first 64 embedded font(\\(s\\)|s) are read\\.")).match(notes.join(QLatin1Char('|'))).hasMatch(), qPrintable(notes.join(QLatin1Char('|'))));
}

// Each cap of the intake keeps a part from being decoded from its text at all: the count used up, too many parts tried (usable or not), and the total held.
void PubFileTests::aPublicationFontIsNotDecodedWhenTheIntakeHasNoRoomForIt()
{
    const QByteArray lato = pubLatoFile(false);
    QVERIFY(!lato.isEmpty());
    {
        jp::office::EmbeddedFontLimits limits;
        limits.faces = 1;
        jp::office::EmbeddedFaceIntake intake(limits);
        QVERIFY(jp::takePubEmbeddedFont(intake, pubFontProps(QStringLiteral("A"), lato)));
        for (int i = 0; i < 5; ++i) QVERIFY(!jp::takePubEmbeddedFont(intake, pubFontProps(QStringLiteral("B%1").arg(i), lato)));
        QCOMPARE(intake.decodedCount(), 1);
    }
    {
        jp::office::EmbeddedFontLimits limits;
        limits.tried = 3;
        jp::office::EmbeddedFaceIntake intake(limits);
        int decoded = 0;
        for (int i = 0; i < 20; ++i) decoded += jp::takePubEmbeddedFont(intake, pubFontProps(QStringLiteral("J%1").arg(i), QByteArray(300, 'j'))) ? 1 : 0;
        QCOMPARE(decoded, 3);
        QCOMPARE(intake.decodedCount(), 3);
    }
    {
        jp::office::EmbeddedFontLimits limits;
        limits.totalBytes = lato.size() * 5 / 2;   // (a font taken is held twice: as the part and as the font)
        jp::office::EmbeddedFaceIntake intake(limits);
        QVERIFY(jp::takePubEmbeddedFont(intake, pubFontProps(QStringLiteral("A"), lato)));
        QVERIFY(!jp::takePubEmbeddedFont(intake, pubFontProps(QStringLiteral("B"), lato)));
        QCOMPARE(intake.decodedCount(), 1);
        QVERIFY(intake.heldBytes() <= limits.totalBytes);
    }
    // A publication keeps no part as the file had it: nothing reads it.
    jp::office::EmbeddedFontLimits lean;
    lean.keepParts = false;
    jp::office::EmbeddedFaceIntake intake(lean);
    QVERIFY(jp::takePubEmbeddedFont(intake, pubFontProps(QStringLiteral("Zobster"), lato)));
    const auto faces = intake.take(nullptr);
    QCOMPARE(faces.size(), 1);
    QVERIFY(faces[0].part.isEmpty());
}
