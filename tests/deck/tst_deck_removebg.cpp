// Taking the background off a picture: how well the mask follows a known subject, what the strokes change, and how the removal is kept.
#include "deckhelpers.h"
#include "core/removebg.h"

#include <QPainter>
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
};

JP_TEST_CLASS(DeckRemoveBgTests)
#include "tst_deck_removebg.moc"
