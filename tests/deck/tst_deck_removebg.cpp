// Taking the background off a picture: how well the mask follows a known subject, what the strokes change, and how the removal is kept.
#include "deckhelpers.h"
#include "core/picturestyles.h"
#include "core/removebg.h"
#include "app/lossywarning.h"
#include "app/removebgprogress.h"
#include "app/picturetools.h"
#include "io/deckoutput.h"
#include "app/theme.h"
#include "../withdialog.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QDomDocument>
#include <QImageReader>
#include <QImageWriter>
#include <QScopeGuard>
#include <QSignalSpy>
#include <QThread>
#include <atomic>
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
        // The subject touches the left and the top edges, so the edge colors include it and it goes; a stroke over it keeps it. (A subject that meets one edge only is found by itself.)
        const QPointF at(0.02, 0.02);
        const QImage im = disc(300, 240, Qt::red, [](int, int) { return QColor(Qt::white); }, 0, at, 0.3);
        const BackgroundRemoval before = findBackground(im, {});
        BackgroundMark stroke;
        stroke.keep = true;
        stroke.radius = 0.03;
        for (int i = 0; i <= 4; ++i) stroke.points << QPointF(0.02 + 0.027 * i, 0.02 + 0.027 * i);
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

    void theMadePicturesMeetTheirQualityNumbers()
    {
        // The numbers the first version had (see the review's notes): a: 83.6 agreement, 72.2% of the subject lost; b: 84.4, 100% lost, and one stroke brought back 3%.
        // No picture is worse than it was by more than one point.
        const Made a = madePortrait();
        const Quality qa = qualityOf(a, findBackground(a.image, {}));
        qInfo("QUALITY %s default: agreement %.1f lost %.1f kept %.1f", qPrintable(a.name), qa.agreement, qa.subjectLost, qa.backgroundKept);
        QVERIFY(qa.agreement >= 98.0);        // a subject that meets the picture's edge stays
        QVERIFY(qa.subjectLost <= 2.0);
        QVERIFY(qa.backgroundKept <= 1.0);
        const Made b = madeSimilarColors();
        const Quality qb = qualityOf(b, findBackground(b.image, {}));
        qInfo("QUALITY %s default: agreement %.1f lost %.1f kept %.1f", qPrintable(b.name), qb.agreement, qb.subjectLost, qb.backgroundKept);
        QVERIFY(qb.agreement >= 83.4);
        for (const Made &m : {madeWhiteProduct(), madeAlreadyClear(), madeOneBit(), madeSixteenBit()}) {
            const Quality q = qualityOf(m, findBackground(m.image, {}));
            qInfo("QUALITY %s default: agreement %.1f lost %.1f kept %.1f", qPrintable(m.name), q.agreement, q.subjectLost, q.backgroundKept);
            QVERIFY2(q.agreement >= 99.0, qPrintable(m.name));
            QVERIFY2(q.subjectLost <= 1.0, qPrintable(m.name));
            QVERIFY2(q.backgroundKept <= 1.0, qPrintable(m.name));
        }
    }
    void oneKeepStrokeAcrossASubjectOfSimilarColorsBringsItBack()
    {
        const Made b = madeSimilarColors();
        // One stroke across the subject brings back at least nine tenths of it, without the background.
        const Quality qs = qualityOf(b, findBackground(b.image, {keepStroke({QPointF(0.5, 0.5), QPointF(0.55, 0.5)})}));
        qInfo("QUALITY %s one keep stroke: agreement %.1f lost %.1f kept %.1f", qPrintable(b.name), qs.agreement, qs.subjectLost, qs.backgroundKept);
        QVERIFY(qs.subjectLost <= 10.0);
        QVERIFY(qs.backgroundKept <= 1.0);
        QVERIFY(qs.agreement >= 95.0);
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
    // ---- made pictures for the quality numbers (the reviewer's, as fixtures built here) ----
    static QImage withGrain(QImage im, int a, int seed = 3)
    {
        QRandomGenerator g(seed);
        for (int y = 0; y < im.height(); ++y)
            for (int x = 0; x < im.width(); ++x) {
                const QColor c = im.pixelColor(x, y);
                auto j = [&](int v) { return std::clamp(v + int(g.bounded(2 * a + 1)) - a, 0, 255); };
                im.setPixelColor(x, y, QColor(j(c.red()), j(c.green()), j(c.blue()), c.alpha()));
            }
        return im;
    }
    static QImage throughJpeg(const QImage &im)
    {
        QByteArray b;
        QBuffer buf(&b);
        buf.open(QIODevice::WriteOnly);
        QImageWriter wr(&buf, "jpg");
        wr.setQuality(92);
        wr.write(im);
        QImage out;
        out.loadFromData(b, "JPG");
        return out.convertToFormat(QImage::Format_ARGB32);
    }
    struct Made {
        QString name;
        QImage image;
        std::function<bool(int, int)> inside;
    };
    static Made madePortrait()
    {
        auto in = [](int x, int y) {
            const double hx = (x - 300) / 70.0, hy = (y - 170) / 85.0;
            if (hx * hx + hy * hy <= 1) return true;
            if (x >= 275 && x <= 325 && y >= 240 && y <= 300) return true;
            const QPolygonF shirt({QPointF(120, 450), QPointF(160, 330), QPointF(300, 290), QPointF(440, 330), QPointF(480, 450)});
            return shirt.containsPoint(QPointF(x, y), Qt::OddEvenFill);
        };
        QImage im(600, 450, QImage::Format_ARGB32);
        for (int y = 0; y < 450; ++y)
            for (int x = 0; x < 600; ++x) {
                QColor c(180 - y / 8, 200 - y / 8, 220 - y / 10);
                if (in(x, y)) c = y < 260 ? QColor(225, 180, 150) : QColor(60, 70, 90);
                im.setPixelColor(x, y, c);
            }
        return {QStringLiteral("a-portrait-bottom-edge"), throughJpeg(withGrain(im, 5)), in};
    }
    static Made madeSimilarColors()
    {
        auto in = [](int x, int y) { return (x - 300) * (x - 300) + (y - 225) * (y - 225) <= 120 * 120; };
        QImage im(600, 450, QImage::Format_ARGB32);
        for (int y = 0; y < 450; ++y)
            for (int x = 0; x < 600; ++x) im.setPixelColor(x, y, in(x, y) ? QColor(135, 168, 205) : QColor(110, 150, 190));
        return {QStringLiteral("b-similar-colors"), withGrain(im, 4), in};
    }
    static Made madeWhiteProduct()
    {
        auto in = [](int x, int y) { return x >= 200 && x < 400 && y >= 110 && y < 320; };
        QImage im(600, 450, QImage::Format_ARGB32);
        im.fill(QColor(250, 250, 250));
        QPainter p(&im);
        p.setRenderHint(QPainter::Antialiasing);
        QRadialGradient g(300, 345, 150);
        g.setColorAt(0, QColor(150, 150, 150, 200));
        g.setColorAt(1, QColor(250, 250, 250, 0));
        p.setPen(Qt::NoPen);
        p.setBrush(g);
        p.drawEllipse(QPointF(300, 345), 170, 40);
        QLinearGradient lg(200, 0, 400, 0);
        lg.setColorAt(0, QColor(245, 245, 245));
        lg.setColorAt(1, QColor(222, 222, 226));
        p.setBrush(lg);
        p.setPen(QPen(QColor(205, 205, 210), 2));
        p.drawRect(200, 110, 200, 210);
        p.end();
        return {QStringLiteral("c-white-product-shadow"), throughJpeg(withGrain(im, 2)), in};
    }
    static Made madeAlreadyClear()
    {
        auto in = [](int x, int y) { return (x - 300) * (x - 300) + (y - 225) * (y - 225) <= 130 * 130; };
        QImage im(600, 450, QImage::Format_ARGB32);
        for (int y = 0; y < 450; ++y)
            for (int x = 0; x < 600; ++x) im.setPixelColor(x, y, in(x, y) ? QColor(200, 60, 40) : QColor(0, 0, 0, 0));
        return {QStringLiteral("d-already-transparent"), im, in};
    }
    static Made madeOneBit()
    {
        auto in = [](int x, int y) { return (x - 200) * (x - 200) + (y - 150) * (y - 150) <= 90 * 90; };
        QImage m(400, 300, QImage::Format_Mono);
        m.setColorCount(2);
        m.setColor(0, qRgb(0, 0, 0));
        m.setColor(1, qRgb(255, 255, 255));
        for (int y = 0; y < 300; ++y)
            for (int x = 0; x < 400; ++x) m.setPixel(x, y, in(x, y) ? 0 : 1);
        return {QStringLiteral("e-1bit"), m, in};
    }
    static Made madeSixteenBit()
    {
        auto in = [](int x, int y) { return (x - 200) * (x - 200) + (y - 150) * (y - 150) <= 90 * 90; };
        QImage s(400, 300, QImage::Format_RGBA64);
        for (int y = 0; y < 300; ++y)
            for (int x = 0; x < 400; ++x) s.setPixelColor(x, y, in(x, y) ? QColor::fromRgbF(0.8, 0.3, 0.2) : QColor::fromRgbF(0.9, 0.95, 0.8));
        return {QStringLiteral("f-16bit"), s, in};
    }
    struct Quality {
        double agreement = 0, subjectLost = 0, backgroundKept = 0;
    };
    // Agreement with the made picture's truth (a band of five pixels around its edge left out), and the shares of the subject lost and of the background kept.
    static Quality qualityOf(const Made &m, const BackgroundRemoval &r)
    {
        const QImage out = applyBackgroundRemoval(m.image, r);
        const int band = 5;
        qint64 total = 0, good = 0, subject = 0, lost = 0, background = 0, kept = 0;
        for (int y = 0; y < out.height(); ++y)
            for (int x = 0; x < out.width(); ++x) {
                const bool inS = m.inside(x, y);
                bool edge = false;
                for (int d = -band; d <= band && !edge; d += band)
                    for (int e = -band; e <= band; e += band)
                        if (m.inside(std::clamp(x + d, 0, out.width() - 1), std::clamp(y + e, 0, out.height() - 1)) != inS) { edge = true; break; }
                if (edge) continue;
                ++total;
                const bool stays = qAlpha(out.pixel(x, y)) > 127;
                good += stays == inS;
                if (inS) { ++subject; lost += !stays; } else { ++background; kept += stays; }
            }
        return {100.0 * good / std::max<qint64>(1, total), 100.0 * lost / std::max<qint64>(1, subject), 100.0 * kept / std::max<qint64>(1, background)};
    }
    static BackgroundMark keepStroke(const QVector<QPointF> &points, double radius = 0.02)
    {
        BackgroundMark m;
        m.keep = true;
        m.radius = radius;
        m.points = points;
        return m;
    }

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
        QCOMPARE(ed->removeBgItem, pic->id);
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
        QVERIFY(ed->removeBgItem.isEmpty());
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
        QVERIFY(ed->removeBgItem.isEmpty());
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
        QVERIFY(ed->removeBgItem.isEmpty());
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
        for (int i = 0; i < 100; ++i) QCoreApplication::processEvents();   // (a picture for the docs only: the slide list draws its small pictures late)
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
        const QImage tinted = Renderer::processedImage(doc, *pic, QSizeF(), QSizeF(), QColor(255, 0, 255, 200));
        QCOMPARE(qAlpha(tinted.pixel(2, 2)), 255);
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
    // ---- the fixes of the review (Nov 24) ----
    static QPoint viewOf(DeckWindow &w, const QPointF &page) { return w.canvas()->pageToView(page).toPoint(); }
    // A real drag over the picture: a press, four moves with the button down, and a release.
    static void realStroke(DeckWindow &w, const QPointF &from, const QPointF &to)
    {
        QWidget *view = w.canvas()->viewport();
        const QPoint a = viewOf(w, from), b = viewOf(w, to);
        QTest::mousePress(view, Qt::LeftButton, Qt::NoModifier, a);
        for (int s = 1; s <= 4; ++s) {
            const QPoint cur = a + (b - a) * s / 4;
            QMouseEvent move(QEvent::MouseMove, QPointF(cur), view->mapToGlobal(QPointF(cur)), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(view, &move);
        }
        QTest::mouseRelease(view, Qt::LeftButton, Qt::NoModifier, b);
    }
    static void turns(int n = 20)
    {
        for (int i = 0; i < n; ++i) QCoreApplication::processEvents();
    }
    // The part that goes is tinted pink over a white slide; a picture drawn for any output has none of it.
    static int pinkPixels(const QImage &im)
    {
        int n = 0;
        for (int y = 0; y < im.height(); ++y)
            for (int x = 0; x < im.width(); ++x) {
                const QColor c = im.pixelColor(x, y);
                n += c.red() > 200 && c.green() < 150 && c.blue() > 150 && c.blue() > c.green() + 40;
            }
        return n;
    }
    void realMouseStrokesEscEnterAndAClickOutsideEndTheModeCleanly()
    {
        DeckWindow w;
        w.resize(1300, 900);
        w.show();
        addDiscPicture(w);
        Editor *ed = w.editor();
        const int steps = ed->undoStack()->count();
        auto open = [&] {
            ed->select(firstPicture(w)->id);
            w.act(QStringLiteral("pic.removeBackground"))->trigger();
            QTRY_VERIFY(!ed->removeBackgroundBusy());
            QCOMPARE(ed->removeBgItem, firstPicture(w)->id);
        };
        open();
        // A stroke that keeps (the tool the mode starts with) and one that takes away, each a real press, moves, and release.
        w.act(QStringLiteral("pic.markKeep"))->trigger();
        QCOMPARE(ed->removeBgTool, Editor::RemoveBgTool::Keep);
        realStroke(w, QPointF(200, 175), QPointF(230, 175));
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QCOMPARE(firstPicture(w)->removal.marks.size(), 1);
        QVERIFY(firstPicture(w)->removal.marks[0].keep);
        QCOMPARE(QGuiApplication::mouseButtons(), Qt::NoButton);
        w.act(QStringLiteral("pic.markRemove"))->trigger();
        QCOMPARE(ed->removeBgTool, Editor::RemoveBgTool::Remove);
        realStroke(w, QPointF(110, 110), QPointF(135, 110));
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QCOMPARE(firstPicture(w)->removal.marks.size(), 2);
        QVERIFY(!firstPicture(w)->removal.marks[1].keep);
        QCOMPARE(QGuiApplication::mouseButtons(), Qt::NoButton);
        // Esc takes the whole session back.
        QTest::keyClick(w.canvas(), Qt::Key_Escape);
        QVERIFY(ed->removeBgItem.isEmpty());
        QVERIFY(!firstPicture(w)->removal.isValid());
        QCOMPARE(ed->undoStack()->count(), steps);
        // Enter keeps it, as one step.
        open();
        realStroke(w, QPointF(200, 175), QPointF(230, 175));
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QTest::keyClick(w.canvas(), Qt::Key_Return);
        QVERIFY(ed->removeBgItem.isEmpty());
        QVERIFY(firstPicture(w)->removal.isValid());
        QCOMPARE(firstPicture(w)->removal.marks.size(), 1);
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        // A click outside the picture keeps it too.
        ed->undo();
        QVERIFY(!firstPicture(w)->removal.isValid());
        open();
        w.act(QStringLiteral("pic.markRemove"))->trigger();
        realStroke(w, QPointF(110, 110), QPointF(135, 110));
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QTest::mouseClick(w.canvas()->viewport(), Qt::LeftButton, Qt::NoModifier, viewOf(w, QPointF(700, 450)));
        QVERIFY(ed->removeBgItem.isEmpty());
        QVERIFY(firstPicture(w)->removal.isValid());
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QCOMPARE(QGuiApplication::mouseButtons(), Qt::NoButton);
        // Discard All Changes, from the ribbon's own command.
        ed->undo();
        open();
        realStroke(w, QPointF(200, 175), QPointF(230, 175));
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        w.act(QStringLiteral("pic.discardRemoval"))->trigger();
        QVERIFY(ed->removeBgItem.isEmpty());
        QVERIFY(!firstPicture(w)->removal.isValid());
        QCOMPARE(ed->undoStack()->count(), steps + 1);   // (the one step the undone session left on the stack, redoable)
    }
    // Wrap-point, rotate, and bend drags finish with a step of their own (the Remove Background release had been put between their labels).
    void dragsOfTheRotateHandleTheWrapPointAndTheLineBendFinish()
    {
        for (const Canvas::HitKind kind : {Canvas::HitKind::Rotate, Canvas::HitKind::WrapPoint, Canvas::HitKind::LineBend}) {
            DeckWindow w;
            w.resize(1300, 900);
            w.show();
            Editor *ed = w.editor();
            ed->setCurrentPage(0);
            ItemPtr it;
            if (kind == Canvas::HitKind::Rotate) {
                auto s = std::make_shared<ShapeItem>();
                s->shape = QStringLiteral("rect");
                s->rect = QRectF(300, 200, 200, 100);
                it = s;
            } else if (kind == Canvas::HitKind::WrapPoint) {
                auto p = std::make_shared<PictureItem>();
                p->imageId = ed->doc()->addImage(QByteArray(), QStringLiteral("png"));
                const QImage im(80, 60, QImage::Format_ARGB32);
                QByteArray png;
                QBuffer b(&png);
                b.open(QIODevice::WriteOnly);
                im.save(&b, "PNG");
                p->imageId = ed->doc()->addImage(png, QStringLiteral("png"));
                p->rect = QRectF(300, 200, 200, 150);
                p->imgRect = QRectF(QPointF(0, 0), p->rect.size());
                p->wrap.points = QPolygonF({QPointF(0, 0), QPointF(200, 0), QPointF(200, 150), QPointF(0, 150)});
                it = p;
            } else {
                auto l = std::make_shared<LineItem>();
                l->p1 = QPointF(300, 300);
                l->p2 = QPointF(500, 380);
                l->route = LineItem::Curved;
                it = l;
            }
            ed->addItem(it, false);
            ed->select(it->id);
            if (kind == Canvas::HitKind::WrapPoint) ed->setWrapItem(it->id);
            turns();
            // The first spot in and around the item where the handle answers.
            QPoint at;
            const QRect region = QRect(viewOf(w, QPointF(250, 120)), viewOf(w, QPointF(560, 420)));
            for (int y = region.top(); y <= region.bottom() && at.isNull(); y += 2)
                for (int x = region.left(); x <= region.right(); x += 2)
                    if (w.canvas()->hitTest(QPointF(x, y)).kind == kind) { at = QPoint(x, y); break; }
            QVERIFY2(!at.isNull(), "the handle was not found");
            const int steps = ed->undoStack()->count();
            QWidget *view = w.canvas()->viewport();
            QTest::mousePress(view, Qt::LeftButton, Qt::NoModifier, at);
            const QPoint to = at + QPoint(25, -18);
            for (int s = 1; s <= 5; ++s) {
                const QPoint cur = at + (to - at) * s / 5;
                QMouseEvent move(QEvent::MouseMove, QPointF(cur), view->mapToGlobal(QPointF(cur)), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
                QApplication::sendEvent(view, &move);
            }
            QTest::mouseRelease(view, Qt::LeftButton, Qt::NoModifier, to);
            QCOMPARE(ed->undoStack()->count(), steps + 1);
            QCOMPARE(QGuiApplication::mouseButtons(), Qt::NoButton);
        }
    }
    void resetPictureDuringTheWorkStopsTheWorkAtOnce()
    {
        struct Hold {
            std::atomic<int> held{0}, byGo{0}, byCancel{0}, byBound{0};
            std::atomic<bool> go{false};
        } hold;
        // Holds each job where it begins, until the test lets it go or the editor tells it to stop (a count of turns, never a time, is the way out if neither comes).
        Editor::removeBgWorkerHook = [&hold](const std::atomic<bool> &cancel) {
            ++hold.held;
            for (int i = 0; i < 20'000'000; ++i) {
                if (hold.go) { ++hold.byGo; return; }
                if (cancel) { ++hold.byCancel; return; }
                QThread::yieldCurrentThread();
            }
            ++hold.byBound;
        };
        const auto clear = qScopeGuard([] { Editor::removeBgWorkerHook = nullptr; });
        // Reset Picture while the first answer is still being worked on: the work is told to stop, and the picture is left as the reset leaves it.
        DeckWindow w;
        w.resize(1300, 900);
        w.show();
        addDiscPicture(w);
        Editor *ed = w.editor();
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_COMPARE(hold.held.load(), 1);
        QVERIFY(ed->removeBackgroundBusy());
        w.act(QStringLiteral("pic.reset"))->trigger();
        QVERIFY(ed->removeBgItem.isEmpty());
        QCOMPARE(hold.byCancel.load(), 1);   // (it was stopped, not waited for)
        QCOMPARE(hold.byBound.load(), 0);
        turns();
        QVERIFY(!firstPicture(w)->removal.isValid());
    }
    void changePictureDuringTheWorkStopsTheWorkAtOnce()
    {
        struct Hold {
            std::atomic<int> held{0}, byGo{0}, byCancel{0}, byBound{0};
            std::atomic<bool> go{false};
        } hold;
        // Holds each job where it begins, until the test lets it go or the editor tells it to stop (a count of turns, never a time, is the way out if neither comes).
        Editor::removeBgWorkerHook = [&hold](const std::atomic<bool> &cancel) {
            ++hold.held;
            for (int i = 0; i < 20'000'000; ++i) {
                if (hold.go) { ++hold.byGo; return; }
                if (cancel) { ++hold.byCancel; return; }
                QThread::yieldCurrentThread();
            }
            ++hold.byBound;
        };
        const auto clear = qScopeGuard([] { Editor::removeBgWorkerHook = nullptr; });
        // The same for Change Picture (the command is dimmed in the window; the library function is what a caller outside the window would use).
        DeckWindow w;
        w.resize(1300, 900);
        w.show();
        addDiscPicture(w);
        Editor *ed = w.editor();
        const QImage other = disc(200, 150, Qt::blue, [](int, int) { return QColor(Qt::white); });
        QByteArray png;
        QBuffer b(&png);
        b.open(QIODevice::WriteOnly);
        other.save(&b, "PNG");
        const QString otherId = ed->doc()->addImage(png, QStringLiteral("png"));
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_COMPARE(hold.held.load(), 1);
        QVERIFY(changePictureTo(ed, firstPicture(w)->id, otherId));
        QCOMPARE(hold.byCancel.load(), 1);
        turns();
        QVERIFY(ed->removeBgItem.isEmpty());
        QCOMPARE(firstPicture(w)->imageId, otherId);
        QVERIFY(!firstPicture(w)->removal.isValid());
    }
    void anAnswerForAPictureThatIsNoLongerTheOneTheWorkBeganOnIsNotTaken()
    {
        struct Hold {
            std::atomic<int> held{0}, byGo{0}, byCancel{0}, byBound{0};
            std::atomic<bool> go{false};
        } hold;
        // Holds each job where it begins, until the test lets it go or the editor tells it to stop (a count of turns, never a time, is the way out if neither comes).
        Editor::removeBgWorkerHook = [&hold](const std::atomic<bool> &cancel) {
            ++hold.held;
            for (int i = 0; i < 20'000'000; ++i) {
                if (hold.go) { ++hold.byGo; return; }
                if (cancel) { ++hold.byCancel; return; }
                QThread::yieldCurrentThread();
            }
            ++hold.byBound;
        };
        const auto clear = qScopeGuard([] { Editor::removeBgWorkerHook = nullptr; });
        // An answer that comes for a picture that is no longer the one the work began on (a change made outside the commands) is not put on it.
        DeckWindow w;
        w.resize(1300, 900);
        w.show();
        PictureItem *pic = addDiscPicture(w);
        Editor *ed = w.editor();
        const QImage other = disc(200, 150, Qt::blue, [](int, int) { return QColor(Qt::white); });
        QByteArray png;
        QBuffer b(&png);
        b.open(QIODevice::WriteOnly);
        other.save(&b, "PNG");
        const QString otherId = ed->doc()->addImage(png, QStringLiteral("png"));
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_COMPARE(hold.held.load(), 1);
        pic->imageId = otherId;   // (the same pixel size: nothing else tells the answer from one for this picture)
        hold.go = true;
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        turns();
        QCOMPARE(hold.byGo.load(), 1);
        QVERIFY(!pic->removal.isValid());
        ed->endRemoveBackground(false);
    }
    void theModeEndsWhenThePageChanges()
    {
        DeckWindow w;
        w.resize(1300, 900);
        w.show();
        addDiscPicture(w);
        w.newSlide();
        Editor *ed = w.editor();
        ed->setCurrentPage(0);
        ed->select(firstPicture(w)->id);
        const int steps = ed->undoStack()->count();
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        ed->setCurrentPage(1);
        QVERIFY(ed->removeBgItem.isEmpty());
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        ed->setCurrentPage(0);
        QVERIFY(firstPicture(w)->removal.isValid());   // (kept, on the slide it was made on)
    }
    void aDeleteIsAStepOfItsOwnAfterTheModesStep()
    {
        // A delete is a step of its own after the mode's.
        DeckWindow w;
        w.resize(1300, 900);
        w.show();
        addDiscPicture(w);
        Editor *ed = w.editor();
        const QString id = firstPicture(w)->id;
        const int steps = ed->undoStack()->count();
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        ed->deleteItems({id});
        QVERIFY(ed->removeBgItem.isEmpty());
        QCOMPARE(ed->undoStack()->count(), steps + 2);
        QVERIFY(!ed->doc()->item(id));
        ed->undo();   // the delete, and only that
        QVERIFY(ed->doc()->item(id));
        QVERIFY(firstPicture(w)->removal.isValid());
        ed->undo();   // the removal
        QVERIFY(!firstPicture(w)->removal.isValid());
    }
    void anotherViewOrASlideShowEndsTheMode()
    {
        // Another view, and a slide show.
        DeckWindow w;
        w.resize(1300, 900);
        w.show();
        addDiscPicture(w);
        Editor *ed = w.editor();
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        w.setView(DeckWindow::View::Sorter);
        QVERIFY(ed->removeBgItem.isEmpty());
        w.setView(DeckWindow::View::Normal);
        ed->select(firstPicture(w)->id);
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        w.startShow(0);
        QVERIFY(ed->removeBgItem.isEmpty());
        for (QWidget *top : QApplication::topLevelWidgets())
            if (qobject_cast<SlideShow *>(top)) top->close();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
    void aSaveOrAnotherDocumentEndsTheMode()
    {
        // Another document in the window, and the window's close.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.resize(1300, 900);
        w.show();
        addDiscPicture(w);
        Editor *ed = w.editor();
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("saved.pptx"))));   // (a save ends the mode, keeping what it did)
        QVERIFY(ed->removeBgItem.isEmpty());
        QVERIFY(firstPicture(w)->removal.isValid());
        QVERIFY(!ed->isModified());
        ed->select(firstPicture(w)->id);
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        ed->setDocument(std::make_unique<Document>(), QString());
        QVERIFY(ed->removeBgItem.isEmpty());
        QVERIFY(!ed->removeBackgroundBusy());
    }
    void whileTheModeIsOpenTheTintNeverReachesOutput()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.resize(1300, 900);
        w.show();
        PictureItem *pic = addDiscPicture(w);
        Editor *ed = w.editor();
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QVERIFY(pic->removal.isValid());
        const double scale = 1.0;
        // The window itself shows the tint (so the checks below are looking for something that is there).
        QCoreApplication::processEvents();
        QVERIFY(pinkPixels(w.canvas()->viewport()->grab().toImage()) > 500);
        // A thumbnail, a slide as a picture or for print, and the slide show's own drawing.
        PaintContext ctx;
        ctx.doc = ed->doc();
        ctx.cache = &ed->cache();
        ctx.opt = ed->renderOptions();
        QCOMPARE(pinkPixels(Renderer::renderToImage(ctx, 0, scale)), 0);
        LayoutCache cache;
        QCOMPARE(pinkPixels(deck::slideImage(*ed->doc(), &cache, 0, 960)), 0);
        const QSizeF page = ed->doc()->pageSize();
        QCOMPARE(pinkPixels(renderSlideImage(ed->doc(), &cache, 0, QSize(960, int(960 * page.height() / page.width())), 1.0, Qt::white)), 0);
        // A PDF: the same bytes with the mode open as with it closed.
        QString err;
        const QString open = dir.filePath(QStringLiteral("open.pdf")), closed = dir.filePath(QStringLiteral("closed.pdf"));
        QVERIFY2(deck::exportPdf(*ed->doc(), open, deck::OutputOptions(), &err), qPrintable(err));
        QCOMPARE(pinkPixels(slideImage(*ed->doc())), 0);
        // A save: the mode is ended first, and the file has the removal and no tint.
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("talk.pptx"))));
        QVERIFY(ed->removeBgItem.isEmpty());
        QVERIFY(pic->removal.isValid());
        QVERIFY2(deck::exportPdf(*ed->doc(), closed, deck::OutputOptions(), &err), qPrintable(err));
        QVERIFY(QFileInfo(open).size() > 0);
        DeckWindow v;
        QVERIFY(v.openFile(dir.filePath(QStringLiteral("talk.pptx"))));
        QVERIFY(firstPicture(v)->removal.isValid());
        QCOMPARE(pinkPixels(slideImage(*v.editor()->doc())), 0);
        QVERIFY(v.editor()->removeBgItem.isEmpty());
    }
    void aKeptPartKeepsItsExtensionAndTypeAsAPictureIsCopiedAndSaved()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile in(dir.filePath(QStringLiteral("theirs.pptx")));
        QVERIFY(in.open(QIODevice::WriteOnly));
        in.write(foreignRemoval());
        in.close();
        DeckWindow source;
        QVERIFY(source.openFile(dir.filePath(QStringLiteral("theirs.pptx"))));
        const PictureItem *theirs = firstPicture(source);
        QVERIFY(theirs && theirs->keptBlip.size() == 1);
        QCOMPARE(theirs->keptBlip[0].parts[0].extension, QStringLiteral("wdp"));
        // Copied and pasted into another presentation (a JSON round trip), which also has a picture with a part of another program that ends in .bin.
        const QJsonObject copied = theirs->toJson();
        DeckWindow target;
        target.show();
        Editor *ed = target.editor();
        auto pasted = std::make_shared<PictureItem>();
        pasted->fromJson(copied);
        pasted->imageId = ed->doc()->addImage(source.editor()->doc()->images.value(theirs->imageId).bytes, QStringLiteral("png"));
        pasted->rect = QRectF(50, 50, 100, 80);
        pasted->imgRect = QRectF(QPointF(0, 0), pasted->rect.size());
        QCOMPARE(pasted->keptBlip.size(), 1);
        QCOMPARE(pasted->keptBlip[0].parts[0].extension, QStringLiteral("wdp"));
        auto embedded = std::make_shared<PictureItem>();
        embedded->imageId = pasted->imageId;
        embedded->rect = QRectF(250, 50, 100, 80);
        embedded->imgRect = QRectF(QPointF(0, 0), embedded->rect.size());
        PictureItem::KeptBlipExtension k;
        k.xml = QStringLiteral("<a:ext uri=\"{11111111-2222-3333-4444-555555555555}\"><x:data xmlns:x=\"urn:test:object\" r:id=\"@@rel0@@\"/></a:ext>");
        k.parts.append({QByteArray("EMBEDDED-OBJECT-BYTES"), QStringLiteral("application/vnd.openxmlformats-officedocument.oleObject"),
                        QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject"), QStringLiteral("bin")});
        embedded->keptBlip.append(k);
        ed->addItem(pasted, false);
        ed->addItem(embedded, false);
        QVERIFY(target.saveTo(dir.filePath(QStringLiteral("target.pptx"))));
        Exported e;
        QString why;
        QVERIFY2(unpack(fileBytes(dir.filePath(QStringLiteral("target.pptx"))), &e, &why), qPrintable(why));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QString wdp, bin;
        for (const QString &name : e.pkg.partNames()) {
            if (name.endsWith(QLatin1String(".wdp"))) wdp = name;
            if (name.endsWith(QLatin1String(".bin"))) bin = name;
        }
        QVERIFY2(!wdp.isEmpty(), "the part lost its extension");
        QVERIFY(e.pkg.part(wdp).startsWith("OTHER-PROGRAM-LAYER"));
        QCOMPARE(e.pkg.contentType(wdp), QStringLiteral("image/vnd.ms-photo"));
        QVERIFY(!bin.isEmpty());
        QCOMPARE(e.pkg.part(bin), QByteArray("EMBEDDED-OBJECT-BYTES"));
        QCOMPARE(e.pkg.contentType(bin), QStringLiteral("application/vnd.openxmlformats-officedocument.oleObject"));
        // No type for a whole extension is written for them: every other part with that extension would be read by it.
        const QString types = QString::fromUtf8(e.zip.value(QStringLiteral("[Content_Types].xml")));
        QVERIFY2(!types.contains(QStringLiteral("Extension=\"bin\"")), qPrintable(types));
        QVERIFY2(!types.contains(QStringLiteral("Extension=\"wdp\"")), qPrintable(types));
    }
    // A kept part whose extension makes a part name the package refuses is left out whole: no relationship points at it, and the file reopens.
    void aKeptPartWithAnUnusableExtensionLeavesNoDanglingRelationship()
    {
        for (const QString &extension : {QStringLiteral("rels"), QString(300, QLatin1Char('x'))}) {
            QTemporaryDir dir;
            QVERIFY(dir.isValid());
            DeckWindow target;
            target.show();
            PictureItem *pic = addDiscPicture(target);
            PictureItem::KeptBlipExtension k;
            k.xml = QStringLiteral("<a:ext uri=\"{11111111-2222-3333-4444-555555555555}\"><x:data xmlns:x=\"urn:test:object\" r:id=\"@@rel0@@\"/></a:ext>");
            k.parts.append({QByteArray("KEPT-BYTES"), QStringLiteral("application/octet-stream"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/oleObject"), extension});
            pic->keptBlip.append(k);
            const QString path = dir.filePath(QStringLiteral("out.pptx"));
            QVERIFY(target.saveTo(path));
            Exported e;
            QString why;
            QVERIFY2(unpack(fileBytes(path), &e, &why), qPrintable(why));
            QVERIFY2(packageProblem(e).isEmpty(), qPrintable(extension.left(8) + QLatin1Char(' ') + packageProblem(e)));
            DeckWindow again;
            QVERIFY(again.openFile(path));
            QVERIFY(firstPicture(again));
        }
    }
    // A presentation open for reading only does not offer Remove Background, whatever is selected.
    void removeBackgroundIsOffInAPresentationOpenForReadingOnly()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        PictureItem *pic = addDiscPicture(w);
        QVERIFY(w.act(QStringLiteral("pic.removeBackground"))->isEnabled());
        w.setReadOnly(true);
        QVERIFY(!w.act(QStringLiteral("pic.removeBackground"))->isEnabled());
        w.editor()->select(QString());
        w.editor()->select(pic->id);   // a new selection refreshes the picture commands
        QVERIFY(!w.act(QStringLiteral("pic.removeBackground"))->isEnabled());
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QVERIFY(w.editor()->removeBgItem.isEmpty());
        w.setReadOnly(false);
    }
    // Where a stroke lands is where the pointer is, at every zoom (400% among them), on a flipped and turned picture, and on one whose whole picture is larger than its frame.
    void aStrokeLandsWhereItIsDrawnAtEveryZoomAndOnTurnedPictures()
    {
        for (const double zoom : {0.5, 1.0, 2.0, 4.0})
            for (int variant = 0; variant < 3; ++variant) {
                DeckWindow w;
                w.resize(1300, 900);
                w.show();
                PictureItem *pic = addDiscPicture(w);
                if (variant == 1) {
                    pic->flipH = true;
                    pic->rotation = 30;
                }
                if (variant == 2) {
                    pic->flipV = true;
                    pic->rotation = 90;
                    pic->imgRect = QRectF(-40, -30, 400, 300);
                }
                Editor *ed = w.editor();
                const QPointF frac(0.45, 0.55);
                auto viewOfFraction = [&] {
                    const QPointF page = pic->transform().map(pic->imgRect.topLeft() + QPointF(frac.x() * pic->imgRect.width(), frac.y() * pic->imgRect.height()));
                    return w.canvas()->pageToView(page);
                };
                ed->select(pic->id);
                w.canvas()->zoomToFit(Canvas::Fit::Selection);
                w.canvas()->setZoom(zoom, viewOfFraction());
                QCoreApplication::processEvents();
                w.act(QStringLiteral("pic.removeBackground"))->trigger();
                QTRY_VERIFY(!ed->removeBackgroundBusy());
                w.act(QStringLiteral("pic.markRemove"))->trigger();
                const QPoint at = viewOfFraction().toPoint();
                QVERIFY2(w.canvas()->viewport()->rect().contains(at), "the spot is not in the window");
                QTest::mouseClick(w.canvas()->viewport(), Qt::LeftButton, Qt::NoModifier, at);
                QTRY_VERIFY(!ed->removeBackgroundBusy());
                const auto &marks = pic->removal.marks;
                QVERIFY2(!marks.isEmpty() && !marks[0].points.isEmpty(), qPrintable(QStringLiteral("no stroke: zoom %1 variant %2").arg(zoom).arg(variant)));
                QVERIFY2(std::abs(marks[0].points[0].x() - frac.x()) < 0.03 && std::abs(marks[0].points[0].y() - frac.y()) < 0.03,
                         qPrintable(QStringLiteral("zoom %1 variant %2: at %3, %4").arg(zoom).arg(variant).arg(marks[0].points[0].x()).arg(marks[0].points[0].y())));
                ed->endRemoveBackground(false);
            }
    }
    void anExtremeShapeKeepsTheWorkCopyUnderItsCap()
    {
        // (a picture one pixel wide, or one pixel high, and millions long: each side is at least one pixel, and the count is held to the cap all the same)
        for (const QSize size : {QSize(1, 3'000'000), QSize(3'000'000, 1)}) {
            QImage im(size, QImage::Format_ARGB32);
            im.fill(QColor(200, 200, 200));
            const BackgroundRemoval r = findBackground(im, {});
            QVERIFY(r.isValid());
            QVERIFY(qint64(r.size.width()) * r.size.height() <= kRemoveBgWorkPixels);
            QCOMPARE(r.source, size);
        }
    }
    void aLongRunOfStrokesCanBeStoppedBetweenStrokesAndInsideOne()
    {
        const QImage im = disc(200, 150, Qt::red, [](int, int) { return QColor(Qt::white); });
        // The work tells how far it is at 10 once before the strokes and then for each stroke: the second ask stops it, at the first stroke (sixty strokes of a point: fewer
        // points than the work looks again after).
        const QVector<BackgroundMark> marks(60, keepStroke({QPointF(0.5, 0.5)}));
        int tens = 0;
        const BackgroundRemoval stopped = findBackground(im, marks, [&](int percent) { return !(percent == 10 && ++tens >= 2); });
        QVERIFY(!stopped.isValid());
        QCOMPARE(tens, 2);
        QVERIFY(findBackground(im, marks).isValid());
        // And inside one stroke of thousands of points.
        QVector<QPointF> points(5000, QPointF(0.5, 0.5));
        tens = 0;
        QVERIFY(!findBackground(im, {keepStroke(points)}, [&](int percent) { return !(percent == 10 && ++tens >= 2); }).isValid());
        QCOMPARE(tens, 2);
    }
    void strokesPastTheCapsAreRefusedWithAMessage()
    {
        DeckWindow w;
        w.resize(1300, 900);
        w.show();
        PictureItem *pic = addDiscPicture(w);
        Editor *ed = w.editor();
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QSignalSpy said(ed, &Editor::status);
        for (int i = 0; i < kRemoveBgMaxMarks; ++i) ed->addRemoveBackgroundMark(keepStroke({QPointF(0.3 + 0.001 * i, 0.5)}));
        QCOMPARE(said.count(), 0);
        ed->addRemoveBackgroundMark(keepStroke({QPointF(0.9, 0.9)}));
        QCOMPARE(said.count(), 1);
        QVERIFY(!said.last().at(0).toString().isEmpty());
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QCOMPARE(pic->removal.marks.size(), kRemoveBgMaxMarks);
        ed->endRemoveBackground(false);
        // A stroke with more points than a removal keeps.
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_VERIFY(!ed->removeBackgroundBusy());
        QVector<QPointF> many(kRemoveBgMaxMarkPoints + 1, QPointF(0.5, 0.5));
        ed->addRemoveBackgroundMark(keepStroke(many));
        QCOMPARE(said.count(), 2);
        QCOMPARE(firstPicture(w)->removal.marks.size(), 0);
        ed->endRemoveBackground(false);
    }
    void aRemovalWritesNoMoreThanItReadsBack()
    {
        BackgroundRemoval r;
        r.source = r.size = QSize(10, 10);
        r.mask = QByteArray(100, char(255));
        for (int i = 0; i < kRemoveBgMaxMarks + 40; ++i) r.marks << keepStroke(QVector<QPointF>(100, QPointF(0.5, 0.5)));
        QVERIFY(r.isValid());
        auto count = [](const QJsonObject &o, int *points) {
            *points = 0;
            const QJsonArray ms = o["marks"].toArray();
            for (const QJsonValue &m : ms) *points += m.toObject()["points"].toArray().size();
            return int(ms.size());
        };
        int points = 0;
        const QJsonObject o = r.toJson();
        const int marks = count(o, &points);
        QVERIFY(marks <= kRemoveBgMaxMarks);
        QVERIFY(points <= kRemoveBgMaxMarkPoints);
        // What was written is all read back.
        const BackgroundRemoval back = BackgroundRemoval::fromJson(o);
        int readPoints = 0;
        for (const BackgroundMark &m : back.marks) readPoints += m.points.size();
        QCOMPARE(int(back.marks.size()), marks);
        QCOMPARE(readPoints, points);
        // Strokes of a point or two: the most strokes.
        r.marks.clear();
        for (int i = 0; i < kRemoveBgMaxMarks + 40; ++i) r.marks << keepStroke({QPointF(0.5, 0.5)});
        QCOMPARE(count(r.toJson(), &points), kRemoveBgMaxMarks);
    }
    void resetPictureTakesOffOurRemovalAndLeavesAnotherProgramsOwn()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QFile in(dir.filePath(QStringLiteral("theirs.pptx")));
        QVERIFY(in.open(QIODevice::WriteOnly));
        in.write(foreignRemoval());
        in.close();
        DeckWindow w;
        QVERIFY(w.openFile(dir.filePath(QStringLiteral("theirs.pptx"))));
        const QStringList before = blipExtensions(foreignRemoval());
        w.editor()->select(firstPicture(w)->id);
        w.act(QStringLiteral("pic.reset"))->trigger();
        QCOMPARE(firstPicture(w)->keptBlip.size(), 1);
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("after.pptx"))));
        QCOMPARE(blipExtensions(fileBytes(dir.filePath(QStringLiteral("after.pptx")))), before);
        // Change Picture takes it off.
        changePictureImage(firstPicture(w), firstPicture(w)->imageId + QStringLiteral("x"), QSize(10, 10));
        QVERIFY(firstPicture(w)->keptBlip.isEmpty());
    }
    void theFinishedPictureOtherProgramsShowIsNoLargerThanItsShownSize()
    {
        // A photograph-like picture of 2400 x 1600 pixels, shown 288 points (four inches) wide: the file keeps the original whole and a finished picture of 880 pixels across.
        QImage im(2400, 1600, QImage::Format_ARGB32);
        for (int y = 0; y < im.height(); ++y)
            for (int x = 0; x < im.width(); ++x) {
                const bool in = (x - 1200) * (x - 1200) + (y - 800) * (y - 800) < 500 * 500;
                im.setPixelColor(x, y, in ? QColor(200, 60 + y / 40, 40) : QColor(150 + x / 40, 190 + y / 60, 230 - x / 60));
            }
        im = withGrain(im, 3);
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        im.save(&buffer, "PNG");
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.resize(1300, 900);
        w.show();
        PictureItem *pic = addDiscPicture(w);
        pic->imageId = w.editor()->doc()->addImage(png, QStringLiteral("png"));
        pic->rect = QRectF(100, 100, 288, 192);
        pic->imgRect = QRectF(QPointF(0, 0), pic->rect.size());
        w.act(QStringLiteral("pic.removeBackground"))->trigger();
        QTRY_VERIFY(!w.editor()->removeBackgroundBusy());
        w.editor()->endRemoveBackground(true);
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("big.pptx"))));
        Exported e;
        QString why;
        QVERIFY2(unpack(fileBytes(dir.filePath(QStringLiteral("big.pptx"))), &e, &why), qPrintable(why));
        int whole = 0, finished = 0;
        for (const QString &name : e.pkg.partNames()) {
            if (e.pkg.contentType(name) != QLatin1String("image/png")) continue;
            const QImage got = QImage::fromData(e.pkg.part(name));
            if (got.size() == im.size()) {
                ++whole;
                QCOMPARE(e.pkg.part(name), png);   // (the original, byte for byte)
            } else if (got.width() <= 881 && got.width() >= 879) {
                ++finished;
                QCOMPARE(qAlpha(got.pixel(2, 2)), 0);
                QCOMPARE(qAlpha(got.pixel(got.width() / 2, got.height() / 2)), 255);
            }
            qInfo("FILESIZE part %s %dx%d %lld bytes", qPrintable(name), got.width(), got.height(), qint64(e.pkg.part(name).size()));
        }
        QCOMPARE(whole, 1);
        QCOMPARE(finished, 1);
        // Opened again, the picture is the whole original with the mask on it.
        DeckWindow v;
        QVERIFY(v.openFile(dir.filePath(QStringLiteral("big.pptx"))));
        QCOMPARE(v.editor()->doc()->image(firstPicture(v)->imageId).size(), im.size());
        QVERIFY(firstPicture(v)->removal.isValid());
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
