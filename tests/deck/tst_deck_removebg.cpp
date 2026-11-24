// Taking the background off a picture: how well the mask follows a known subject, what the strokes change, and how the removal is kept.
#include "deckhelpers.h"
#include "core/picturestyles.h"
#include "core/removebg.h"
#include "app/lossywarning.h"
#include "app/removebgprogress.h"
#include "app/theme.h"
#include "../withdialog.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QDomDocument>
#include <QImageReader>
#include <QPainter>
#include <QPointer>
#include <QProgressDialog>
#include <QTemporaryDir>
#include <QRandomGenerator>

class DeckRemoveBgTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

private:
    // A disc (center, radius) over a background; `noise` adds grain of that size to every pixel, from a generator of a fixed seed.
    static QImage disc(int w, int h, const QColor &subject, const std::function<QColor(int, int)> &background, int noise = 0, QPointF at = QPointF(0.5, 0.5), double radius = 0.3)
    {
        QImage im(w, h, QImage::Format_ARGB32);
        QRandomGenerator rng(7);
        const double cx = at.x() * w, cy = at.y() * h, r = radius * std::min(w, h);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x) {
                const bool in = (x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r;
                QColor c = in ? subject : background(x, y);
                if (noise) {
                    auto j = [&](int v) { return std::clamp(v + int(rng.bounded(2 * noise + 1)) - noise, 0, 255); };
                    c = QColor(j(c.red()), j(c.green()), j(c.blue()));
                }
                im.setPixelColor(x, y, c);
            }
        return im;
    }
    static bool insideDisc(int x, int y, int w, int h, QPointF at = QPointF(0.5, 0.5), double radius = 0.3)
    {
        const double cx = at.x() * w, cy = at.y() * h, r = radius * std::min(w, h);
        return (x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r;
    }
    // The share of pixels whose kept-or-removed answer matches the disc's, leaving out a band of `band` pixels around its edge.
    static double agreement(const QImage &im, const BackgroundRemoval &r, QPointF at = QPointF(0.5, 0.5), double radius = 0.3, int band = 4)
    {
        const QImage out = applyBackgroundRemoval(im, r);
        const double cx = at.x() * im.width(), cy = at.y() * im.height(), rad = radius * std::min(im.width(), im.height());
        qint64 total = 0, good = 0;
        for (int y = 0; y < im.height(); ++y)
            for (int x = 0; x < im.width(); ++x) {
                const double d = std::hypot(x - cx, y - cy);
                if (std::abs(d - rad) < band) continue;
                ++total;
                const bool kept = qAlpha(out.pixel(x, y)) > 127;
                good += kept == (d < rad);
            }
        return double(good) / double(total);
    }

private Q_SLOTS:
    void aRedDiscOnWhiteIsKept()
    {
        const QImage im = disc(300, 240, Qt::red, [](int, int) { return QColor(Qt::white); });
        const BackgroundRemoval r = findBackground(im, {});
        QVERIFY(r.isValid());
        QVERIFY2(agreement(im, r) > 0.995, "disc on white");
        QCOMPARE(qAlpha(applyBackgroundRemoval(im, r).pixel(2, 2)), 0);
        QCOMPARE(qAlpha(applyBackgroundRemoval(im, r).pixel(150, 120)), 255);
    }
    void aGradientBackgroundIsRemoved()
    {
        const QImage im = disc(320, 240, QColor(200, 30, 30), [](int x, int y) { return QColor(20 + x * 200 / 320, 60 + y * 150 / 240, 255 - x * 180 / 320); });
        const BackgroundRemoval r = findBackground(im, {});
        QVERIFY2(agreement(im, r) > 0.98, qPrintable(QString::number(agreement(im, r))));
    }
    void aNoisyPhotoLikeFieldIsRemoved()
    {
        const QImage im = disc(400, 300, QColor(190, 70, 60), [](int, int) { return QColor(110, 130, 120); }, 25);
        const BackgroundRemoval r = findBackground(im, {});
        QVERIFY2(agreement(im, r) > 0.95, "disc on noise");
    }
    void aBigPictureIsWorkedOnACopyAndTheMaskScaledUp()
    {
        const QImage im = disc(1800, 1200, Qt::blue, [](int, int) { return QColor(Qt::white); });   // 2.16 million pixels
        const BackgroundRemoval r = findBackground(im, {});
        QVERIFY(r.isValid());
        QVERIFY(qint64(r.size.width()) * r.size.height() <= kRemoveBgWorkPixels + 4000);
        QCOMPARE(r.source, im.size());
        QVERIFY2(agreement(im, r, QPointF(0.5, 0.5), 0.3, 6) > 0.995, "big disc");
    }
    void aKeepStrokeBringsBackWhatWasTakenAway()
    {
        // The subject touches the left edge, so the edge colors include it and it goes; a stroke over it keeps it.
        const QPointF at(0.02, 0.5);
        const QImage im = disc(300, 240, Qt::red, [](int, int) { return QColor(Qt::white); }, 0, at, 0.3);
        const BackgroundRemoval before = findBackground(im, {});
        BackgroundMark stroke;
        stroke.keep = true;
        stroke.radius = 0.03;
        for (int i = 0; i <= 8; ++i) stroke.points << QPointF(0.02 + 0.027 * i, 0.5);
        const BackgroundRemoval after = findBackground(im, {stroke});
        QVERIFY2(agreement(im, after, at) > agreement(im, before, at) + 0.05, qPrintable(QString::number(agreement(im, after, at)) + " before " + QString::number(agreement(im, before, at))));
        QVERIFY2(agreement(im, after, at) > 0.99, qPrintable(QString::number(agreement(im, after, at)) + " before " + QString::number(agreement(im, before, at))));
        QCOMPARE(after.marks.size(), 1);
    }
    void aRemoveStrokeTakesAwayWhatWasLeft()
    {
        // A white ring around the disc is not joined to the edge by white, so it stays; a stroke across it takes it.
        QImage im(300, 240, QImage::Format_ARGB32);
        im.fill(Qt::green);
        QPainter p(&im);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);
        p.drawEllipse(QPointF(150, 120), 90, 90);
        p.setBrush(Qt::red);
        p.drawEllipse(QPointF(150, 120), 50, 50);
        p.end();
        const BackgroundRemoval before = findBackground(im, {});
        QVERIFY(qAlpha(applyBackgroundRemoval(im, before).pixel(150 + 70, 120)) > 200);   // the ring stays
        BackgroundMark stroke;
        stroke.keep = false;
        stroke.radius = 0.03;
        for (int i = 0; i <= 6; ++i) stroke.points << QPointF((150 + 62 + 4.0 * i) / 300, 0.5);
        const BackgroundRemoval after = findBackground(im, {stroke});
        QVERIFY(qAlpha(applyBackgroundRemoval(im, after).pixel(150 + 70, 120)) < 60);
        QVERIFY(qAlpha(applyBackgroundRemoval(im, after).pixel(150, 120)) > 200);       // the disc stays
    }
    void whereTwoStrokesMeetTheLastOneWins()
    {
        const QImage im = disc(300, 240, Qt::red, [](int, int) { return QColor(Qt::white); });
        auto stroke = [](bool keep, QPointF at) {
            BackgroundMark m;
            m.keep = keep;
            m.radius = 0.04;
            m.points << at << QPointF(at.x() + 0.01, at.y());
            return m;
        };
        // Over the subject: taken away then kept, and kept then taken away.
        const QPointF inSubject(0.5, 0.5), inBackground(0.08, 0.1);
        auto kept = [&](const QVector<BackgroundMark> &marks, QPointF at) {
            return qAlpha(applyBackgroundRemoval(im, findBackground(im, marks)).pixel(int(at.x() * 300), int(at.y() * 240))) > 127;
        };
        QVERIFY(kept({stroke(false, inSubject), stroke(true, inSubject)}, inSubject));
        QVERIFY(!kept({stroke(true, inSubject), stroke(false, inSubject)}, inSubject));
        // Over the background, the same.
        QVERIFY(kept({stroke(false, inBackground), stroke(true, inBackground)}, inBackground));
        QVERIFY(!kept({stroke(true, inBackground), stroke(false, inBackground)}, inBackground));
    }
    void aKeepStrokeDoesNotSpillIntoOtherColors()
    {
        // A white background, a red disc, and a blue square that is not joined to the edges. A stroke that keeps the red disc changes nothing but the disc.
        QImage im = disc(300, 240, Qt::red, [](int, int) { return QColor(Qt::white); }, 0, QPointF(0.3, 0.5), 0.2);
        for (int y = 150; y < 200; ++y)
            for (int x = 200; x < 260; ++x) im.setPixelColor(x, y, Qt::blue);
        const QImage plain = applyBackgroundRemoval(im, findBackground(im, {}));
        BackgroundMark stroke;
        stroke.keep = true;
        stroke.radius = 0.03;
        stroke.points << QPointF(0.3, 0.5) << QPointF(0.33, 0.5);
        const QImage marked = applyBackgroundRemoval(im, findBackground(im, {stroke}));
        qint64 outside = 0, total = 0;
        for (int y = 0; y < im.height(); ++y)
            for (int x = 0; x < im.width(); ++x) {
                if (std::hypot(x - 90.0, y - 120.0) < 0.2 * 240 + 4) continue;
                ++total;
                outside += std::abs(qAlpha(plain.pixel(x, y)) - qAlpha(marked.pixel(x, y))) > 64;
            }
        QCOMPARE(outside, qint64(0));
        QVERIFY(total > 50000);
        QVERIFY(qAlpha(marked.pixel(230, 175)) > 200);   // the blue square is as it was
        QVERIFY(qAlpha(marked.pixel(150, 20)) < 60);     // the white around is gone
    }
    void theWorkCanBeStopped()
    {
        const QImage im = disc(300, 240, Qt::red, [](int, int) { return QColor(Qt::white); });
        int told = 0;
        const BackgroundRemoval r = findBackground(im, {}, [&](int) { return ++told < 3; });
        QVERIFY(!r.isValid());
        QCOMPARE(told, 3);
    }
    void aMaskOfAnotherSizeIsIgnored()
    {
        const QImage im = disc(300, 240, Qt::red, [](int, int) { return QColor(Qt::white); });
        const BackgroundRemoval r = findBackground(im, {});
        const QImage other(301, 240, QImage::Format_ARGB32);
        QCOMPARE(applyBackgroundRemoval(other, r), other);
        BackgroundRemoval bad = r;
        bad.mask.chop(1);
        QVERIFY(!bad.isValid());
        QCOMPARE(applyBackgroundRemoval(im, bad), im);
    }
    void theRemovalSurvivesItsJsonAndHostileJsonIsRefused()
    {
        const QImage im = disc(200, 150, Qt::red, [](int, int) { return QColor(Qt::white); });
        BackgroundMark stroke;
        stroke.keep = false;
        stroke.points << QPointF(0.1, 0.1) << QPointF(0.2, 0.1);
        const BackgroundRemoval r = findBackground(im, {stroke});
        const BackgroundRemoval back = BackgroundRemoval::fromJson(r.toJson());
        QVERIFY(back.isValid());
        QCOMPARE(back.mask, r.mask);
        QCOMPARE(back.marks.size(), 1);
        QCOMPARE(back.key(), r.key());
        // A size that does not match the data, and a size past the limit, are refused before anything is unpacked.
        QJsonObject o = r.toJson();
        o["size"] = QJsonArray{199, 150};
        QVERIFY(!BackgroundRemoval::fromJson(o).isValid());
        o["size"] = QJsonArray{100000, 100000};
        QVERIFY(!BackgroundRemoval::fromJson(o).isValid());
        o = r.toJson();
        o["mask"] = QStringLiteral("AAAA");
        QVERIFY(!BackgroundRemoval::fromJson(o).isValid());
        // More strokes than are allowed are cut to the limit.
        o = r.toJson();
        QJsonArray many;
        for (int i = 0; i < 1000; ++i) many << QJsonObject{{"keep", true}, {"radius", 0.02}, {"points", QJsonArray{QJsonArray{0.5, 0.5}}}};
        o["marks"] = many;
        QCOMPARE(BackgroundRemoval::fromJson(o).marks.size(), kRemoveBgMaxMarks);
    }
    void theTintShowsTheRemovedPart()
    {
        const QImage im = disc(300, 240, Qt::red, [](int, int) { return QColor(Qt::white); });
        const BackgroundRemoval r = findBackground(im, {});
        const QImage shown = applyBackgroundRemoval(im, r, QColor(255, 0, 255, 180));
        QCOMPARE(qAlpha(shown.pixel(2, 2)), 255);
        QVERIFY(qGreen(shown.pixel(2, 2)) < 120);                 // white turned toward the tint
        QCOMPARE(shown.pixel(150, 120), im.pixel(150, 120));      // the subject as it was
    }

private:
    // ---- the mode, in JeffDeck's window ----
    static PictureItem *addDiscPicture(DeckWindow &w, const QString &format = QStringLiteral("png"))
    {
        const QImage im = disc(200, 150, Qt::red, [](int, int) { return QColor(Qt::white); });
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        im.save(&buffer, "PNG");
        auto pic = std::make_shared<PictureItem>();
        pic->name = QStringLiteral("disc");
        pic->imageId = w.editor()->doc()->addImage(bytes, format);
        pic->rect = QRectF(100, 100, 200, 150);
        pic->imgRect = QRectF(QPointF(0, 0), pic->rect.size());
        w.editor()->setCurrentPage(0);
        w.editor()->addItem(pic, false);
        w.editor()->select(pic->id);
        return pic.get();
    }
    static PictureItem *firstPicture(DeckWindow &w)
    {
        for (const auto &it : w.editor()->doc()->pages[0]->items)
            if (it->type() == ItemType::Picture) return static_cast<PictureItem *>(it.get());
        return nullptr;
    }

private Q_SLOTS:
    void theModeIsOneStepOfUndoAndKeepsTheOriginal()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        PictureItem *pic = addDiscPicture(w);
        const QString imageId = pic->imageId;
        Editor *ed = w.editor();
        const int steps = ed->undoStack()->count();
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QCOMPARE(w.ribbon()->current(), w.ribbon()->tab(QStringLiteral("Background Removal")));   // the tab opens
        QCOMPARE(ed->removeBgItem, pic->id);
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QVERIFY(pic->removal.isValid());
        QVERIFY(pic->removalTint.isValid());
        // A stroke runs the work again with it fixed; the mode is still open, and nothing is a step yet.
        BackgroundMark stroke;
        stroke.keep = false;
        stroke.points << QPointF(0.05, 0.05) << QPointF(0.1, 0.05);
        ed->addRemoveBackgroundMark(stroke);
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QCOMPARE(pic->removal.marks.size(), 1);
        QCOMPARE(ed->undoStack()->count(), steps);
        ed->endRemoveBackground(true);
        QVERIFY(ed->removeBgItem.isEmpty());
        QVERIFY(!pic->removalTint.isValid());
        QCOMPARE(ed->undoStack()->count(), steps + 1);   // one step for the whole of it
        QCOMPARE(pic->imageId, imageId);                  // the original is as it was
        QVERIFY(pic->removal.isValid());
        ed->undo();
        QVERIFY(!firstPicture(w)->removal.isValid());
        ed->redo();
        QVERIFY(firstPicture(w)->removal.isValid());
        QCOMPARE(firstPicture(w)->removal.marks.size(), 1);
        // The mode again on the same picture starts from the removal it has, and Esc takes the changes of the session back.
        pic = firstPicture(w);
        const QByteArray before = pic->removal.mask;
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QCOMPARE(ed->removeBgItem, pic->id);
        BackgroundMark keep;
        keep.keep = true;
        keep.points << QPointF(0.5, 0.5);
        ed->addRemoveBackgroundMark(keep);
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QCOMPARE(firstPicture(w)->removal.marks.size(), 2);
        ed->endRemoveBackground(false);
        QCOMPARE(firstPicture(w)->removal.mask, before);
        QCOMPARE(firstPicture(w)->removal.marks.size(), 1);
        QVERIFY(!firstPicture(w)->removalTint.isValid());
        QCOMPARE(ed->undoStack()->count(), steps + 1);
    }
    void anAnimatedPictureIsRefusedWithAPlainMessage()
    {
        // A two-frame GIF made by hand.
        QByteArray gif = QByteArray::fromHex("47494638396101000100800000000000ffffff");
        const QByteArray frame = QByteArray::fromHex("21f9040100000000" "2c00000000010001000002024401" "00");
        gif += frame + frame + QByteArray::fromHex("3b");
        QBuffer probe(&gif);
        probe.open(QIODevice::ReadOnly);
        QImageReader reader(&probe);
        QCOMPARE(reader.imageCount(), 2);
        DeckWindow w;
        w.show();
        PictureItem *pic = addDiscPicture(w);
        pic->imageId = w.editor()->doc()->addImage(gif, QStringLiteral("gif"));
        QVERIFY(!pic->imageId.isEmpty());
        QString why;
        QVERIFY(!w.editor()->beginRemoveBackground(pic->id, &why));
        QVERIFY2(why.contains(QStringLiteral("animated")), qPrintable(why));
        QVERIFY(w.editor()->removeBgItem.isEmpty());
        QVERIFY(!firstPicture(w)->removal.isValid());
    }
    static QByteArray bigDiscPng()
    {
        const QImage im = disc(1800, 1200, Qt::red, [](int, int) { return QColor(Qt::white); });   // 2.16 million pixels
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        im.save(&buffer, "PNG");
        return png;
    }
    void aBigPictureShowsAProgressBoxAndCancelLeavesThePictureUntouched()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        PictureItem *pic = addDiscPicture(w);
        pic->imageId = w.editor()->doc()->addImage(bigDiscPng(), QStringLiteral("png"));
        const QString imageId = pic->imageId;
        Editor *ed = w.editor();
        const int steps = ed->undoStack()->count();
        // A small picture has no box.
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QVERIFY(ed->removeBgPixels >= kRemoveBgProgressPixels);
        // (the first report of the thread is queued: the window's own signal stands in for it, before the event loop can deliver the answer)
        Q_EMIT ed->removeBackgroundProgress(10);
        QPointer<QProgressDialog> box = w.removeBackgroundProgress()->box();
        QVERIFY(box);
        QCOMPARE(box->maximum(), 100);
        Q_EMIT box->canceled();   // (what the Cancel button does)
        QVERIFY(ed->removeBgItem.isEmpty());          // nothing was found yet: the mode ends as Esc ends it
        QTRY_VERIFY(!box);
        QVERIFY(!firstPicture(w)->removal.isValid());
        QVERIFY(!firstPicture(w)->removalTint.isValid());
        QCOMPARE(ed->undoStack()->count(), steps);
        QCOMPARE(firstPicture(w)->imageId, imageId);
        // The same with a stroke: the removal the picture had stays, and the stroke is not added.
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QVERIFY(firstPicture(w)->removal.isValid());
        QVERIFY(!w.removeBackgroundProgress()->box());
        const QByteArray before = firstPicture(w)->removal.mask;
        BackgroundMark stroke;
        stroke.keep = false;
        stroke.points << QPointF(0.5, 0.5);
        ed->addRemoveBackgroundMark(stroke);
        Q_EMIT ed->removeBackgroundProgress(10);
        box = w.removeBackgroundProgress()->box();
        QVERIFY(box);
        Q_EMIT box->canceled();   // (what the Cancel button does)
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QVERIFY(!ed->removeBgItem.isEmpty());          // the mode is still open
        QCOMPARE(firstPicture(w)->removal.mask, before);
        QCOMPARE(firstPicture(w)->removal.marks.size(), 0);
        ed->endRemoveBackground(true);
        QCOMPARE(firstPicture(w)->removal.mask, before);
        QCOMPARE(firstPicture(w)->removal.marks.size(), 0);
        QCOMPARE(ed->undoStack()->count(), steps + 1);
    }
    // The pictures for the docs, when JO_SHOT_DIR names a folder: the mode with the tint, a stroke being dragged, and the result on a colored slide.
    void picturesForTheDocs()
    {
        if (qEnvironmentVariableIsEmpty("JO_SHOT_DIR")) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        Editor *ed = w.editor();
        ed->doc()->pages[0]->items.clear();
        auto back = std::make_shared<ShapeItem>();
        back->shape = QStringLiteral("rect");
        back->rect = QRectF(0, 0, ed->doc()->pageSize().width(), ed->doc()->pageSize().height());
        back->fill.type = Fill::Solid;
        back->fill.color = ColorRef::fromString(QStringLiteral("#1f4e79"));
        ed->addItem(back, false);
        const QImage im = disc(480, 360, QColor(230, 120, 30), [](int x, int y) { return QColor(150 + x / 8, 190 + y / 12, 230 - x / 12); }, 5, QPointF(0.5, 0.52), 0.32);
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        im.save(&buffer, "PNG");
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = ed->doc()->addImage(png, QStringLiteral("png"));
        pic->rect = QRectF(180, 90, 480, 360);
        pic->imgRect = QRectF(QPointF(0, 0), pic->rect.size());
        ed->addItem(pic, false);
        ed->select(pic->id);
        QTRY_VERIFY(w.act(QStringLiteral("pic.removeBackground"))->isEnabled());
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QCoreApplication::processEvents();
        shotOf(&w, QStringLiteral("deck-removebg-mode"));
        // A stroke, being dragged with real mouse events.
        w.act(QStringLiteral("pic.markKeep"))->trigger();
        QWidget *view = w.canvas()->viewport();
        const QPoint a = w.canvas()->pageToView(QPointF(330, 270)).toPoint(), b = w.canvas()->pageToView(QPointF(400, 300)).toPoint();
        QTest::mousePress(view, Qt::LeftButton, Qt::NoModifier, a);
        QMouseEvent mv(QEvent::MouseMove, QPointF(b), view->mapToGlobal(QPointF(b)), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(view, &mv);
        shotOf(&w, QStringLiteral("deck-removebg-stroke"));
        QTest::mouseRelease(view, Qt::LeftButton, Qt::NoModifier, b);
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        w.act(QStringLiteral("pic.keepRemoval"))->trigger();
        ed->select(QString());
        QTest::qWait(500);   // (a picture for the docs only: the slide list draws its small pictures late)
        shotOf(&w, QStringLiteral("deck-removebg-result"));
    }

    void resetPictureBringsTheWholePictureBack()
    {
        DeckWindow w;
        w.show();
        PictureItem *pic = addDiscPicture(w);
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_VERIFY(!w.editor()->removeBackgroundBusy());
        w.editor()->endRemoveBackground(true);
        QVERIFY(firstPicture(w)->removal.isValid());
        w.editor()->select(pic->id);
        w.act(QStringLiteral("pic.reset"))->trigger();
        QVERIFY(!firstPicture(w)->removal.isValid());
    }
    void aDrawingMadeOfLinesIsRefusedWithAPlainMessage()
    {
        DeckWindow w;
        w.show();
        addDiscPicture(w);
        QString why;
        Document &d = *w.editor()->doc();
        const QString vector = d.addImage("<svg xmlns='http://www.w3.org/2000/svg' width='20' height='20'><rect width='20' height='20'/></svg>", QStringLiteral("svg"));
        QVERIFY(!vector.isEmpty());
        firstPicture(w)->imageId = vector;
        QVERIFY(!w.editor()->beginRemoveBackground(firstPicture(w)->id, &why));
        QVERIFY2(why.contains(QStringLiteral("drawing")), qPrintable(why));
        QVERIFY(w.editor()->removeBgItem.isEmpty());
    }
    void thePictureIsDrawnWithTheBackgroundClearAndAMismatchedMaskIsIgnored()
    {
        DeckWindow w;
        w.show();
        PictureItem *pic = addDiscPicture(w);
        const Document &doc = *w.editor()->doc();
        const QImage whole = doc.image(pic->imageId);
        pic->removal = findBackground(whole, {});
        QVERIFY(pic->removal.isValid());
        const QImage drawn = Renderer::processedImage(doc, *pic, QSizeF(), QSizeF());
        QCOMPARE(drawn.size(), whole.size());
        QCOMPARE(qAlpha(drawn.pixel(2, 2)), 0);
        QCOMPARE(qAlpha(drawn.pixel(100, 75)), 255);
        // With the tint the removed part is shown, not clear.
        pic->removalTint = QColor(255, 0, 255, 200);
        const QImage tinted = Renderer::processedImage(doc, *pic, QSizeF(), QSizeF());
        QCOMPARE(qAlpha(tinted.pixel(2, 2)), 255);
        pic->removalTint = QColor();
        // A mask made for a picture of another size is not used: the picture is shown whole.
        pic->removal.source = QSize(whole.width() + 1, whole.height());
        QCOMPARE(qAlpha(Renderer::processedImage(doc, *pic, QSizeF(), QSizeF()).pixel(2, 2)), 255);
    }
    void thePresentationKeepsTheRemovalInItsOwnFileAndOtherFormatsKeepTheResult()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        BackgroundRemoval made;
        {
            DeckWindow w;
            w.show();
            PictureItem *pic = addDiscPicture(w);
            w.act(QStringLiteral("pic.removeBackground"))->trigger();
            QTRY_VERIFY(!w.editor()->removeBackgroundBusy());
            w.editor()->endRemoveBackground(true);
            made = firstPicture(w)->removal;
            QVERIFY(made.isValid());
            QVERIFY(w.saveTo(dir.filePath(QStringLiteral("talk.pptx"))));
            // The other kind of file is told to the person: the box lists what it can't hold.
            QString said;
            jp::setLossyStandIn([&](const jp::LossyAsk &a, bool *) {
                said = a.lost.join(QLatin1Char('|'));
                return jp::LossyChoice::SaveAnyway;
            });
            const bool saved = w.saveTo(dir.filePath(QStringLiteral("talk.odp")), true);
            jp::setLossyStandIn(nullptr);
            QVERIFY(saved);
            QVERIFY2(said.contains(QStringLiteral("background was removed")), qPrintable(said));
            Q_UNUSED(pic);
        }
        // The presentation file: the original and the mask are back, so the removal can be edited.
        {
            DeckWindow w;
            QVERIFY(w.openFile(dir.filePath(QStringLiteral("talk.pptx"))));
            const PictureItem *pic = firstPicture(w);
            QVERIFY(pic);
            QVERIFY(pic->removal.isValid());
            QCOMPARE(pic->removal.mask, made.mask);
            QCOMPARE(w.editor()->doc()->image(pic->imageId).size(), made.source);
            QCOMPARE(qAlpha(w.editor()->doc()->image(pic->imageId).pixel(2, 2)), 255);   // the original, not the result
        }
        // The other kind of file holds the finished picture.
        {
            DeckWindow w;
            QVERIFY(w.openFile(dir.filePath(QStringLiteral("talk.odp"))));
            const PictureItem *pic = firstPicture(w);
            QVERIFY(pic);
            QVERIFY(!pic->removal.isValid());
            QCOMPARE(qAlpha(w.editor()->doc()->image(pic->imageId).pixel(2, 2)), 0);
        }
    }
    void aPresentationFromOtherProgramsWithoutOurExtensionShowsThePictureAsItIs()
    {
        // The blip's own picture is what such a file draws; the extension is optional, so a file without it opens as a plain picture.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.show();
        addDiscPicture(w);
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("plain.pptx"))));
        DeckWindow v;
        QVERIFY(v.openFile(dir.filePath(QStringLiteral("plain.pptx"))));
        QVERIFY(!firstPicture(v)->removal.isValid());
    }
    // ---- a file from another program that carries its own removal of the background on a picture ----
    static QByteArray foreignRemoval(int extra = 0, bool external = false)
    {
        // A second picture the extension points at, with bytes of its own (not a picture this program reads).
        QByteArray second("OTHER-PROGRAM-LAYER");
        for (int i = 0; i < 300; ++i) second.append(char(i * 7));
        Parts p = samplePptx();
        QString ext = QStringLiteral("<a:extLst><a:ext uri=\"{BEBA8EAE-BF5A-486C-A8C5-ECC9F3942E4B}\"><a14:imgProps xmlns:a14=\"http://schemas.microsoft.com/office/drawing/2010/main\">"
                                     "<a14:imgLayer r:embed=\"rId4\"><a14:imgEffect><a14:backgroundRemoval t=\"10\" b=\"90\" l=\"10\" r=\"90\"><a14:foregroundMark x1=\"1\" y1=\"2\" x2=\"3\" y2=\"4\"/>"
                                     "<a14:backgroundMark x1=\"5\" y1=\"6\" x2=\"7\" y2=\"8\"/></a14:backgroundRemoval></a14:imgEffect></a14:imgLayer></a14:imgProps></a:ext>");
        for (int i = 0; i < extra; ++i) ext += QStringLiteral("<a:ext uri=\"{00000000-0000-0000-0000-00000000000%1}\"><x:more xmlns:x=\"urn:test:more\" r:id=\"rId4\"/></a:ext>").arg(i);
        ext += QStringLiteral("</a:extLst>");
        p.data["/ppt/slides/slide1.xml"].xml.replace("<a:blip r:embed=\"rId2\"/>", ("<a:blip r:embed=\"rId2\">" + ext + "</a:blip>").toUtf8());
        p.data["/ppt/media/layer1.wdp"] = {QStringLiteral("image/vnd.ms-photo"), second};
        p.rels << Rel{"/ppt/slides/slide1.xml", "image", "/ppt/media/layer1.wdp"};
        if (external) p.rels.last() = Rel{"/ppt/slides/slide1.xml", "image", "https://example.com/layer.wdp", true};
        // (the relationship the extension names is the next one the sample's writer hands out: found by its target)
        return pack(p);
    }
    // An element as the file says it: names, attributes in order of name, and what the relationships in it name (the part's bytes, by hash), children, text.
    static QString canonical(const QDomElement &e, const office::Package &pkg, const QString &source)
    {
        QString out = QLatin1Char('<') + e.nodeName();
        QStringList attrs;
        const QDomNamedNodeMap map = e.attributes();
        for (int i = 0; i < map.length(); ++i) {
            const QDomAttr a = map.item(i).toAttr();
            if (a.nodeName().startsWith(QLatin1String("xmlns"))) continue;
            QString v = a.value();
            if (a.namespaceURI().endsWith(QLatin1String("/relationships"))) {
                const office::Relationship rel = pkg.relationship(source, v);
                v = QStringLiteral("part:") + QString::fromLatin1(QCryptographicHash::hash(pkg.part(rel.target), QCryptographicHash::Sha1).toHex()) + rel.type + pkg.contentType(rel.target);
            }
            attrs << a.nodeName() + QLatin1Char('=') + v;
        }
        attrs.sort();
        out += QLatin1Char(' ') + attrs.join(QLatin1Char(' ')) + QLatin1Char('>');
        for (QDomNode c = e.firstChild(); !c.isNull(); c = c.nextSibling())
            out += c.isElement() ? canonical(c.toElement(), pkg, source) : c.nodeValue();
        return out + QStringLiteral("</") + e.nodeName() + QLatin1Char('>');
    }
    // The extensions of the first slide's picture's blip, as canonical text, in order.
    static QStringList blipExtensions(const QByteArray &pptx, const QString &slide = QStringLiteral("/ppt/slides/slide1.xml"))
    {
        office::Package pkg;
        QString why;
        if (!pkg.read(pptx, &why)) return {};
        QDomDocument dom;
        if (!dom.setContent(pkg.part(slide), true)) return {};
        QStringList found;
        const QDomNodeList blips = dom.elementsByTagNameNS(QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/main"), QStringLiteral("blip"));
        for (int i = 0; i < blips.size(); ++i)
            for (QDomElement x = blips.at(i).firstChildElement(); !x.isNull(); x = x.nextSiblingElement())
                for (QDomElement ext = x.firstChildElement(); !ext.isNull(); ext = ext.nextSiblingElement()) found << canonical(ext, pkg, slide);
        return found;
    }
    void theOtherProgramsRemovalDataComesBackUntouchedAfterOpenAndSave()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QByteArray original = foreignRemoval();
        const QStringList before = blipExtensions(original);
        QCOMPARE(before.size(), 1);
        QVERIFY(before[0].contains(QStringLiteral("backgroundMark")));
        QFile in(dir.filePath(QStringLiteral("theirs.pptx")));
        QVERIFY(in.open(QIODevice::WriteOnly));
        in.write(original);
        in.close();
        DeckWindow w;
        QVERIFY(w.openFile(dir.filePath(QStringLiteral("theirs.pptx"))));
        QVERIFY(firstPicture(w));
        QCOMPARE(firstPicture(w)->keptBlip.size(), 1);
        QVERIFY(!firstPicture(w)->removal.isValid());
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("again.pptx"))));
        const QByteArray saved = fileBytes(dir.filePath(QStringLiteral("again.pptx")));
        QCOMPARE(blipExtensions(saved), before);
        Exported e;
        QString why;
        QVERIFY2(unpack(saved, &e, &why), qPrintable(why));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        // The second picture is still a part of the file, with the type it had.
        bool found = false;
        for (const QString &name : e.pkg.partNames())
            if (e.pkg.contentType(name) == QLatin1String("image/vnd.ms-photo")) found = found || e.pkg.part(name).startsWith("OTHER-PROGRAM-LAYER");
        QVERIFY(found);
        // Saved again after being opened again: the same.
        DeckWindow v;
        QVERIFY(v.openFile(dir.filePath(QStringLiteral("again.pptx"))));
        QCOMPARE(firstPicture(v)->keptBlip.size(), 1);
        QVERIFY(v.saveTo(dir.filePath(QStringLiteral("third.pptx"))));
        QCOMPARE(blipExtensions(fileBytes(dir.filePath(QStringLiteral("third.pptx")))), before);
        // The native file keeps it too (what a copy or a paste carries).
        const QJsonObject o = firstPicture(v)->toJson();
        PictureItem copy;
        copy.fromJson(o);
        QCOMPARE(copy.keptBlip.size(), 1);
        QCOMPARE(copy.keptBlip[0].xml, firstPicture(v)->keptBlip[0].xml);
        QCOMPARE(copy.keptBlip[0].parts[0].bytes, firstPicture(v)->keptBlip[0].parts[0].bytes);
    }
    void ourRemovalReplacesTheirsAndTheSaveSaysSo()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile in(dir.filePath(QStringLiteral("theirs.pptx")));
        QVERIFY(in.open(QIODevice::WriteOnly));
        in.write(foreignRemoval());
        in.close();
        DeckWindow w;
        w.show();
        QVERIFY(w.openFile(dir.filePath(QStringLiteral("theirs.pptx"))));
        // The mode needs a picture it can work on: the sample's is a small one-color square; give the picture a made one.
        PictureItem *pic = firstPicture(w);
        QVERIFY(pic);
        QImage im = disc(200, 160, Qt::red, [](int, int) { return QColor(Qt::white); });
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        im.save(&buffer, "PNG");
        pic->imageId = w.editor()->doc()->addImage(png, QStringLiteral("png"));
        w.editor()->select(pic->id);
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_VERIFY(!w.editor()->removeBackgroundBusy());
        w.editor()->endRemoveBackground(true);
        QVERIFY(firstPicture(w)->removal.isValid());
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*w.editor()->doc(), nullptr, &skipped, deck::PresentationKind::Presentation, false, false, false);
        QVERIFY(skipped.join(QLatin1Char('|')).contains(QStringLiteral("another program made")));
        // Only ours is in the file now.
        const QStringList exts = blipExtensions(bytes);
        QCOMPARE(exts.size(), 1);
        QVERIFY(exts[0].contains(QStringLiteral("removal")));
        QVERIFY(!exts[0].contains(QStringLiteral("backgroundMark")));
        // The other kind of file says what it can't keep.
        QString said;
        jp::setLossyStandIn([&](const jp::LossyAsk &a, bool *) {
            said = a.lost.join(QLatin1Char('|'));
            return jp::LossyChoice::SaveAnyway;
        });
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("out.odp")), true));
        jp::setLossyStandIn(nullptr);
        QVERIFY2(said.contains(QStringLiteral("background was removed")), qPrintable(said));
    }
    void aChangedPictureDropsTheOtherProgramsData()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile in(dir.filePath(QStringLiteral("theirs.pptx")));
        QVERIFY(in.open(QIODevice::WriteOnly));
        in.write(foreignRemoval());
        in.close();
        DeckWindow w;
        QVERIFY(w.openFile(dir.filePath(QStringLiteral("theirs.pptx"))));
        PictureItem *pic = firstPicture(w);
        QVERIFY(!pic->keptBlip.isEmpty());
        changePictureImage(pic, pic->imageId + QStringLiteral("x"), QSize(10, 10));
        QVERIFY(pic->keptBlip.isEmpty());
    }
    void hostileExtensionsAreBounded()
    {
        // Seven more extensions that name the same part: no more than the limit are kept, and each keeps its part once in the file.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile in(dir.filePath(QStringLiteral("many.pptx")));
        QVERIFY(in.open(QIODevice::WriteOnly));
        in.write(foreignRemoval(7));
        in.close();
        DeckWindow w;
        QVERIFY(w.openFile(dir.filePath(QStringLiteral("many.pptx"))));
        QCOMPARE(firstPicture(w)->keptBlip.size(), kMaxKeptBlipExtensions);
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("many2.pptx"))));
        const QByteArray saved = fileBytes(dir.filePath(QStringLiteral("many2.pptx")));
        QCOMPARE(blipExtensions(saved).size(), kMaxKeptBlipExtensions);
        Exported e;
        QString why;
        QVERIFY2(unpack(saved, &e, &why), qPrintable(why));
        int copies = 0;
        for (const QString &name : e.pkg.partNames()) copies += e.pkg.contentType(name) == QLatin1String("image/vnd.ms-photo");
        QCOMPARE(copies, 1);
        // An extension that names a part outside the file is not kept, and the picture opens as it is.
        QFile out(dir.filePath(QStringLiteral("outside.pptx")));
        QVERIFY(out.open(QIODevice::WriteOnly));
        out.write(foreignRemoval(0, true));
        out.close();
        DeckWindow v;
        QVERIFY(v.openFile(dir.filePath(QStringLiteral("outside.pptx"))));
        QVERIFY(firstPicture(v));
        QVERIFY(firstPicture(v)->keptBlip.isEmpty());
        // A part larger than a picture may keep is not kept.
        PictureItem big;
        QJsonObject o = big.toJson();
        QJsonObject kept{{"xml", QStringLiteral("<a:ext xmlns:a=\"urn:x\"/>")},
                         {"parts", QJsonArray{QJsonObject{{"data", QString::fromLatin1(QByteArray(int(kMaxKeptBlipBytes) + 4096, 'x').toBase64())}, {"type", "image/x"}, {"rel", "urn:r"}}}}};
        o["keptBlip"] = QJsonArray{kept};
        big.fromJson(o);
        QVERIFY(big.keptBlip.isEmpty());
    }
};

JP_TEST_CLASS(DeckRemoveBgTests)
#include "tst_deck_removebg.moc"
