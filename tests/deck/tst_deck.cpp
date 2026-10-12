// JeffDeck's tests: its presentation model on the shared page model, and
// its window.

#include "app/deckinfo.h"
#include "app/deckwindow.h"
#include "app/editor.h"
#include "app/notespane.h"
#include "app/presenterview.h"
#include "app/slideshow.h"
#include "app/slidespane.h"
#include "core/deck.h"
#include "core/fonts.h"
#include "core/transition.h"
#include "render/renderer.h"

#include <QAbstractButton>
#include <QDesktopServices>
#include <QElapsedTimer>
#include <QPainter>
#include <QPointer>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTextDocument>
#include <QUrl>

using namespace jp;

// Takes the addresses the slide show opens in place of a browser.
class LinkCatcher : public QObject {
    Q_OBJECT
public:
    QStringList urls;
public Q_SLOTS:
    void open(const QUrl &url) { urls << url.toString(); }
};

class DeckTests : public QObject {
    Q_OBJECT
    QTemporaryDir m_settingsDir;

private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(deck);
        setAppInfo(deckAppInfo());
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, m_settingsDir.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settingsDir.path());
        initCore();
    }

    // A new presentation: widescreen, one slide master with the nine
    // standard layouts over it, the default theme, and a Title Slide whose
    // placeholders are empty and prompt for text.
    void newPresentationHasMasterLayoutsAndTitleSlide()
    {
        auto d = deck::newPresentation();
        QCOMPARE(d->pageSize(), QSizeF(960, 540));
        QCOMPARE(int(d->masters.size()), 10);
        QVERIFY(deck::isSlideMaster(*d->masters[0]));
        const QVector<MasterPage *> layouts = deck::layouts(*d);
        QCOMPARE(int(layouts.size()), 9);
        for (MasterPage *l : layouts) QCOMPARE(l->baseId, d->masters[0]->id);
        QStringList types;
        for (MasterPage *l : layouts) types << deck::layoutType(*l);
        QCOMPARE(types, (QStringList{"title", "obj", "secHead", "twoObj", "twoTxTwoObj", "titleOnly", "blank", "objTx", "picTx"}));
        QCOMPARE(deck::themeOf(*d).name, QStringLiteral("JeffOffice"));
        QCOMPARE(int(d->pages.size()), 1);
        const Page &slide = *d->pages[0];
        QCOMPARE(deck::layoutType(*d->masterFor(slide)), QStringLiteral("title"));
        QStringList placeholders;
        for (const auto &it : slide.items) {
            placeholders << deck::placeholderType(*it);
            const auto *t = static_cast<const TextItem *>(it.get());
            QVERIFY(d->storyDoc(t->storyId)->isEmpty());
            QVERIFY(!t->prompt.isEmpty());
        }
        QCOMPARE(placeholders, (QStringList{"ctrTitle", "subTitle"}));
        QVERIFY(d->storyDoc(slide.notesStoryId));
        // Empty placeholders and the layout's prompts print nothing.
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        const QImage img = Renderer::renderToImage(ctx, 0, 0.25);
        for (int y = 0; y < img.height(); y += 7)
            for (int x = 0; x < img.width(); x += 7) QCOMPARE(img.pixelColor(x, y), QColor(Qt::white));
    }

    // Slides copy their layout's placeholders; the date, footer, and slide
    // number only when the presentation shows them.
    void addSlideCopiesPlaceholders()
    {
        auto d = deck::newPresentation();
        const MasterPage *two = deck::layoutOfType(*d, QStringLiteral("twoObj"));
        const int i = deck::addSlide(*d, *two);
        QCOMPARE(i, 1);
        QStringList types;
        for (const auto &it : d->pages[i]->items) types << deck::placeholderType(*it);
        QCOMPARE(types, (QStringList{"title", "obj", "obj"}));
        // Each its own story and object, not the layout's.
        const auto *layoutTitle = static_cast<const TextItem *>(two->items[0].get());
        const auto *slideTitle = static_cast<const TextItem *>(d->pages[i]->items[0].get());
        QVERIFY(layoutTitle->id != slideTitle->id);
        QVERIFY(layoutTitle->storyId != slideTitle->storyId);
        QCOMPARE(slideTitle->rect, layoutTitle->rect);
        d->appData[QStringLiteral("footers")] = true;
        const int j = deck::addSlide(*d, *two, 0);
        QCOMPARE(j, 0);
        types.clear();
        for (const auto &it : d->pages[j]->items) types << deck::placeholderType(*it);
        QCOMPARE(types, (QStringList{"title", "obj", "obj", "dt", "ftr", "sldNum"}));
    }

    // A new layout moves the placeholders to its places, by kind, and keeps
    // their text; its places that nothing filled get empty placeholders.
    void applyLayoutKeepsText()
    {
        auto d = deck::newPresentation();
        Page &slide = *d->pages[0];
        auto *title = static_cast<TextItem *>(slide.items[0].get());
        d->storyDoc(title->storyId)->setPlainText(QStringLiteral("Quarterly results"));
        const MasterPage *twoContent = deck::layoutOfType(*d, QStringLiteral("twoObj"));
        deck::applyLayout(*d, slide, *twoContent);
        QCOMPARE(slide.masterId, twoContent->id);
        QCOMPARE(title->rect, twoContent->items[0]->rect);
        QCOMPARE(d->storyDoc(title->storyId)->toPlainText(), QStringLiteral("Quarterly results"));
        QCOMPARE(int(slide.items.size()), 3);   // title, the subtitle in the first content place, a new second one
        // Reset puts a moved placeholder back.
        title->rect.translate(50, 50);
        deck::resetSlide(*d, slide);
        QCOMPARE(title->rect, twoContent->items[0]->rect);
    }

    void duplicateSlideCopiesNotes()
    {
        auto d = deck::newPresentation();
        d->storyDoc(d->pages[0]->notesStoryId)->setPlainText(QStringLiteral("Say hello"));
        const int i = deck::duplicateSlide(*d, 0);
        QCOMPARE(i, 1);
        QCOMPARE(deck::notesText(*d, *d->pages[1]), QStringLiteral("Say hello"));
        QVERIFY(d->pages[1]->notesStoryId != d->pages[0]->notesStoryId);
        QVERIFY(d->pages[1]->items[0]->id != d->pages[0]->items[0]->id);
    }

    // `n` content slides, each titled and with notes of its own, so that
    // each draws differently.
    static std::unique_ptr<Document> showDeck(int n)
    {
        auto d = deck::newPresentation();
        while (int(d->pages.size()) < n) deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        for (int i = 0; i < n; ++i) {
            Page &s = *d->pages[i];
            d->storyDoc(static_cast<TextItem *>(s.items[0].get())->storyId)->setPlainText(QStringLiteral("Slide %1 title").arg(i + 1));
            d->storyDoc(s.notesStoryId)->setPlainText(QStringLiteral("Notes %1").arg(i + 1));
        }
        return d;
    }

    static deck::Transition showTransition(const char *type, double seconds)
    {
        deck::Transition t;
        t.type = QString::fromLatin1(type);
        t.duration = seconds;
        return t;
    }

    // A transition lives in the slide's props and comes back whole; what is
    // missing or makes no sense takes its default.
    void transitionJsonRoundTrip()
    {
        deck::Transition t;
        QCOMPARE(t.type, QStringLiteral("none"));
        QCOMPARE(t.dir, QStringLiteral("l"));
        QCOMPARE(t.duration, 0.7);
        QVERIFY(!t.advanceAfter);
        QVERIFY(t.advanceOnClick);
        QVERIFY(deck::Transition::fromJson(QJsonObject()) == t);
        QVERIFY(!t.toJson().contains(QStringLiteral("advanceAfter")));   // absent, not zero
        t.type = QStringLiteral("push");
        t.dir = QStringLiteral("u");
        t.duration = 1.5;
        t.advanceAfter = 4.0;
        t.advanceOnClick = false;
        const QJsonObject o = t.toJson();
        QCOMPARE(o.value("type").toString(), QStringLiteral("push"));
        QCOMPARE(o.value("dir").toString(), QStringLiteral("u"));
        QCOMPARE(o.value("duration").toDouble(), 1.5);
        QCOMPARE(o.value("advanceAfter").toDouble(), 4.0);
        QCOMPARE(o.value("advanceOnClick").toBool(true), false);
        QVERIFY(deck::Transition::fromJson(o) == t);
        // Split and zoom go in or out, the rest in a direction.
        QCOMPARE(deck::Transition::fromJson({{"type", "split"}}).dir, QStringLiteral("out"));
        QCOMPARE(deck::Transition::fromJson({{"type", "zoom"}}).dir, QStringLiteral("in"));
        QCOMPARE(deck::Transition::fromJson({{"type", "zoom"}, {"dir", "in"}}).dir, QStringLiteral("in"));
        QCOMPARE(deck::Transition::fromJson({{"type", "split"}, {"dir", "in"}}).dir, QStringLiteral("in"));
        QCOMPARE(deck::Transition::fromJson({{"type", "wipe"}, {"dir", "out"}}).dir, QStringLiteral("l"));
        QCOMPARE(deck::Transition::fromJson({{"type", "wipe"}, {"dir", "d"}}).dir, QStringLiteral("d"));
        QCOMPARE(deck::Transition::fromJson({{"type", "zoom"}, {"dir", "u"}}).dir, QStringLiteral("in"));
        // An unknown type is none; a duration stays within what a person can sit through.
        QCOMPARE(deck::Transition::fromJson({{"type", "spin"}}).type, QStringLiteral("none"));
        QCOMPARE(deck::Transition::fromJson({{"type", "fade"}, {"duration", 500}}).duration, 59.0);
        QCOMPARE(deck::Transition::fromJson({{"type", "fade"}, {"duration", 0}}).duration, 0.01);
        QVERIFY(!deck::Transition::fromJson({{"advanceAfter", -3}}).advanceAfter);
        QCOMPARE(*deck::Transition::fromJson({{"advanceAfter", 0}}).advanceAfter, 0.0);

        // On a slide: kept in its props beside whatever else is there, and
        // gone from them when it is the default again.
        Page slide;
        slide.props[QStringLiteral("kind")] = QStringLiteral("other");
        QVERIFY(deck::transitionOf(slide) == deck::Transition());
        deck::setTransition(slide, t);
        QVERIFY(slide.props.value("transition").isObject());
        QVERIFY(deck::transitionOf(slide) == t);
        deck::setTransition(slide, deck::Transition());
        QVERIFY(!slide.props.contains(QStringLiteral("transition")));
        QCOMPARE(slide.props.value("kind").toString(), QStringLiteral("other"));
        QCOMPARE(deck::transitionTypes(),
                 (QStringList{"none", "fade", "push", "wipe", "split", "reveal", "cover", "uncover", "zoom", "cut"}));
    }

    // Going forward onto a slide with a transition keeps the old slide on the
    // screen while it plays and lands on the new one when it ends; going
    // back, or jumping, shows the slide at once.
    void showTransitionPlaysOnNext()
    {
        auto d = showDeck(3);
        deck::setTransition(*d->pages[1], showTransition("fade", 0.15));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        QSignalSpy changes(&show, &SlideShow::changed);
        show.next();
        QCOMPARE(show.currentSlide(), 0);   // still the old slide on the screen
        QCOMPARE(show.targetSlide(), 1);
        QVERIFY(show.isAnimating());
        QVERIFY(changes.count() >= 1);      // Presenter View already knows
        QTRY_VERIFY(!show.isAnimating());
        QCOMPARE(show.currentSlide(), 1);
        QCOMPARE(show.targetSlide(), 1);
        // Going back has no transition, though the slide left has one.
        show.previous();
        QVERIFY(!show.isAnimating());
        QCOMPARE(show.currentSlide(), 0);
        // Finishing it by hand lands on the new slide too.
        deck::setTransition(*d->pages[1], showTransition("push", 30));
        show.next();
        QVERIFY(show.isAnimating());
        show.finishTransition();
        QVERIFY(!show.isAnimating());
        QCOMPARE(show.currentSlide(), 1);
        // Next in the middle of a transition finishes it first, then plays the next one.
        show.goTo(0);
        QVERIFY(!show.isAnimating());   // a jump shows the slide at once
        show.next();
        QVERIFY(show.isAnimating());
        deck::setTransition(*d->pages[2], showTransition("wipe", 30));
        show.next();
        QCOMPARE(show.currentSlide(), 1);
        QCOMPARE(show.targetSlide(), 2);
        QVERIFY(show.isAnimating());
        show.finishTransition();
        QCOMPARE(show.currentSlide(), 2);
        // Every type plays and ends.
        for (const QString &type : deck::transitionTypes()) {
            deck::setTransition(*d->pages[1], showTransition(type.toLatin1().constData(), 30));
            show.goTo(0);
            show.next();
            QCOMPARE(show.isAnimating(), type != QLatin1String("none"));
            show.finishTransition();
            QCOMPARE(show.currentSlide(), 1);
        }
    }

    // After a slide's seconds the show moves on by itself, counting from when
    // the slide's transition has finished.
    void showAdvancesAfterItsTime()
    {
        auto d = showDeck(3);
        deck::Transition t = showTransition("none", 0.7);
        t.advanceAfter = 0.05;
        deck::setTransition(*d->pages[0], t);
        t = showTransition("fade", 0.8);
        t.advanceAfter = 0.3;
        deck::setTransition(*d->pages[1], t);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        QVERIFY(!show.autoAdvanceActive());   // not before it is shown
        show.show();
        QTRY_COMPARE(show.targetSlide(), 1);
        QElapsedTimer sinceFade;
        sinceFade.start();
        // The fade plays out first; the clock starts after it.
        QVERIFY(show.isAnimating());
        QVERIFY(!show.autoAdvanceActive());
        QTRY_COMPARE(show.currentSlide(), 2);
        QVERIFY(sinceFade.elapsed() >= 700);   // not 0.3 seconds into the fade
        QVERIFY(!show.autoAdvanceActive());    // the third slide has no time: the show waits there
        show.next();
        QCOMPARE(show.currentSlide(), -1);
    }

    // A slide that doesn't advance on a click ignores clicks but not keys.
    void showAdvanceOnClickOff()
    {
        auto d = showDeck(3);
        deck::Transition t;
        t.advanceOnClick = false;
        deck::setTransition(*d->pages[0], t);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(640, 360);
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
        QCOMPARE(show.currentSlide(), 0);
        QTest::keyClick(&show, Qt::Key_Right);
        QCOMPARE(show.currentSlide(), 1);
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
        QCOMPARE(show.currentSlide(), 2);
    }

    // Esc in the middle of a transition ends the show.
    void showEscapeDuringTransition()
    {
        auto d = showDeck(3);
        deck::setTransition(*d->pages[1], showTransition("zoom", 30));
        LayoutCache cache;
        QPointer<SlideShow> show = new SlideShow(d.get(), &cache, 0);
        QSignalSpy ended(show.data(), &SlideShow::ended);
        show->show();
        show->next();
        QVERIFY(show->isAnimating());
        QTest::keyClick(show.data(), Qt::Key_Escape);
        QCOMPARE(ended.count(), 1);
        QCOMPARE(ended.at(0).at(0).toInt(), 1);   // the slide it was moving to
        QTRY_VERIFY(show.isNull());
    }

    // The end screen, black and white screens, number and Enter, and hidden
    // slides work with transitions on.
    void showKeysWithTransitions()
    {
        auto d = showDeck(4);
        d->pages[2]->hidden = true;
        for (int i = 1; i < 4; ++i) deck::setTransition(*d->pages[i], showTransition("fade", 0.05));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        QTest::keyClick(&show, Qt::Key_B);
        QVERIFY(show.isBlank());
        QTest::keyClick(&show, Qt::Key_B);
        QVERIFY(!show.isBlank());
        QTest::keyClick(&show, Qt::Key_W);
        QVERIFY(show.isBlank());
        QTest::keyClick(&show, Qt::Key_Space);   // moving on shows the slide again
        QVERIFY(!show.isBlank());
        QTRY_COMPARE(show.currentSlide(), 1);
        QTest::keyClick(&show, Qt::Key_Right);
        QTRY_COMPARE(show.currentSlide(), 3);   // past the hidden slide
        QTest::keyClicks(&show, "1");
        QTest::keyClick(&show, Qt::Key_Return);
        QCOMPARE(show.currentSlide(), 0);
        QTest::keyClick(&show, Qt::Key_End);
        QCOMPARE(show.currentSlide(), 3);
        QTest::keyClick(&show, Qt::Key_Right);
        QTRY_COMPARE(show.currentSlide(), -1);   // the end screen
        QVERIFY(!show.isAnimating());
        QTest::keyClick(&show, Qt::Key_Left);
        QCOMPARE(show.currentSlide(), 3);
    }

    // A click on an item with a link opens the link and stays on the slide.
    void showClickOnLinkOpensIt()
    {
        auto d = showDeck(2);
        Item *title = d->pages[0]->items[0].get();
        title->hyperlink = QStringLiteral("https://example.com/talk");
        LinkCatcher catcher;
        QDesktopServices::setUrlHandler(QStringLiteral("https"), &catcher, "open");
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);   // the slide's own size: slide points are pixels
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, title->bounds().center().toPoint());
        QCOMPARE(catcher.urls, (QStringList{"https://example.com/talk"}));
        QCOMPARE(show.currentSlide(), 0);
        // Elsewhere a click moves on.
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(2, 538));
        QCOMPARE(show.currentSlide(), 1);
        QCOMPARE(catcher.urls.size(), 1);
        QDesktopServices::unsetUrlHandler(QStringLiteral("https"));
    }

    // Presenter View: where the show is, the next slide, the notes, the timer;
    // its buttons drive the show.
    void presenterViewFollowsTheShow()
    {
        auto d = showDeck(4);
        d->pages[2]->hidden = true;
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        PresenterView pv(&show);
        QCOMPARE(pv.counterText(), QStringLiteral("Slide 1 of 4"));
        QCOMPARE(pv.notesText(), QStringLiteral("Notes 1"));
        QCOMPARE(pv.nextSlide(), 1);
        const QImage firstNext = pv.nextImage();
        QVERIFY(!firstNext.isNull());
        show.next();
        QCOMPARE(pv.counterText(), QStringLiteral("Slide 2 of 4"));
        QCOMPARE(pv.notesText(), QStringLiteral("Notes 2"));
        QCOMPARE(pv.nextSlide(), 3);   // the hidden slide is skipped
        QVERIFY(pv.nextImage() != firstNext);
        // The buttons call the show.
        pv.findChild<QAbstractButton *>("previousButton")->click();
        QCOMPARE(show.currentSlide(), 0);
        QCOMPARE(pv.counterText(), QStringLiteral("Slide 1 of 4"));
        pv.findChild<QAbstractButton *>("nextButton")->click();
        QCOMPARE(show.currentSlide(), 1);
        pv.findChild<QAbstractButton *>("blackButton")->click();
        QVERIFY(show.isBlank());
        pv.findChild<QAbstractButton *>("blackButton")->click();
        QVERIFY(!show.isBlank());
        // Keys work in Presenter View too.
        QTest::keyClick(&pv, Qt::Key_Right);
        QCOMPARE(show.currentSlide(), 3);
        QCOMPARE(pv.nextSlide(), -1);
        QTest::keyClick(&pv, Qt::Key_Right);   // the end
        QCOMPARE(pv.counterText(), QStringLiteral("End of slide show"));
        QTest::keyClick(&pv, Qt::Key_Left);
        QCOMPARE(pv.counterText(), QStringLiteral("Slide 4 of 4"));
        // The notes grow and shrink.
        const int size = pv.notesPointSize();
        pv.findChild<QAbstractButton *>("notesBigger")->click();
        QVERIFY(pv.notesPointSize() > size);
        pv.findChild<QAbstractButton *>("notesSmaller")->click();
        pv.findChild<QAbstractButton *>("notesSmaller")->click();
        QVERIFY(pv.notesPointSize() < size);
        // The slide grid jumps to a slide.
        pv.findChild<QAbstractButton *>("allSlidesButton")->click();
        QVERIFY(pv.isShowingAllSlides());
        pv.jumpTo(0);
        QCOMPARE(show.currentSlide(), 0);
        QVERIFY(!pv.isShowingAllSlides());
        // It draws: the slides and the controls, not a blank window.
        pv.resize(1100, 700);
        const QImage shot = pv.grab().toImage();
        QCOMPARE(shot.size(), QSize(1100, 700));
        QSet<QRgb> colors;
        for (int y = 0; y < shot.height(); y += 9)
            for (int x = 0; x < shot.width(); x += 9) colors.insert(shot.pixel(x, y));
        QVERIFY(colors.size() >= 4);
        // End Slide Show closes the show, and Presenter View with it.
        QPointer<SlideShow> alive = new SlideShow(d.get(), &cache, 0);
        QPointer<PresenterView> pv2 = new PresenterView(alive.data());
        pv2->findChild<QAbstractButton *>("endButton")->click();
        QTRY_VERIFY(alive.isNull());
        QTRY_VERIFY(pv2.isNull());
    }

    // The timer counts from the start of the show, pauses and resumes, and resets.
    void presenterViewTimer()
    {
        auto d = showDeck(2);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        PresenterView pv(&show);
        QCOMPARE(pv.timerText(), QStringLiteral("00:00"));
        QVERIFY(!pv.isTimerPaused());
        QAbstractButton *pause = pv.findChild<QAbstractButton *>("pauseButton");
        QVERIFY(pause);
        const QString runningLabel = pause->text();
        pause->click();
        QVERIFY(pv.isTimerPaused());
        QVERIFY(pause->text() != runningLabel);   // it says Resume
        pv.setTimerElapsed(65 * 1000);
        QCOMPARE(pv.timerText(), QStringLiteral("01:05"));
        pause->click();
        QVERIFY(!pv.isTimerPaused());
        QCOMPARE(pause->text(), runningLabel);
        pv.setTimerElapsed(3725 * 1000);
        QCOMPARE(pv.timerText(), QStringLiteral("1:02:05"));
        pv.findChild<QAbstractButton *>("resetButton")->click();
        QVERIFY(pv.timerElapsed() < 1000);
        QCOMPARE(pv.timerText(), QStringLiteral("00:00"));
    }

    // Painting only the changed part of a frame over the last one gives the
    // next frame, for every type and direction.
    void transitionChangeRegionIsComplete()
    {
        for (const qreal dpr : {1.0, 2.0}) {
            const QSize size(160, 90);
            const QSize px(qRound(size.width() * dpr), qRound(size.height() * dpr));
            QImage a(px, QImage::Format_RGB32), b(px, QImage::Format_RGB32);
            a.setDevicePixelRatio(dpr);
            b.setDevicePixelRatio(dpr);
            for (int y = 0; y < px.height(); ++y)
                for (int x = 0; x < px.width(); ++x) {
                    a.setPixel(x, y, qRgb((x * 7) % 256, (y * 5) % 256, 40));
                    b.setPixel(x, y, qRgb(200, (x * 3) % 256, (y * 11) % 256));
                }
            auto frameAt = [&](const deck::Transition &t, double progress, const QRegion &only, QImage *over) {
                QImage frame = over ? *over : QImage(px, QImage::Format_RGB32);
                frame.setDevicePixelRatio(dpr);
                QPainter p(&frame);
                if (!only.isEmpty()) p.setClipRegion(only);
                paintTransition(&p, t, a, b, progress);
                return frame;
            };
            for (const QString &type : deck::transitionTypes()) {
                deck::Transition t;
                t.type = type;
                QStringList dirs{"l", "r", "u", "d"};
                if (t.hasInOut()) dirs = QStringList{"in", "out"};
                for (const QString &dir : dirs) {
                    t.dir = dir;
                    for (const auto &[before, after] : {std::pair(0.0, 0.1), std::pair(0.1, 0.13), std::pair(0.34, 0.36), std::pair(0.64, 0.66), std::pair(0.9, 1.0)}) {
                        QImage last = frameAt(t, before, QRegion(), nullptr);
                        const QRegion changed = transitionChange(t, QSizeF(size), before, after);
                        QImage next = changed.isEmpty() ? last : frameAt(t, after, changed, &last);
                        const QImage expected = frameAt(t, after, QRegion(), nullptr);
                        QVERIFY2(next == expected, qPrintable(QStringLiteral("%1 %2 from %3 to %4 at %5x").arg(type, dir).arg(before).arg(after).arg(dpr)));
                    }
                }
            }
        }
        // A wipe changes a strip, not the screen; a cut changes twice.
        deck::Transition wipe;
        wipe.type = QStringLiteral("wipe");
        QVERIFY(transitionChange(wipe, QSizeF(1920, 1080), 0.5, 0.52).boundingRect().width() < 80);
        deck::Transition cut;
        cut.type = QStringLiteral("cut");
        QVERIFY(transitionChange(cut, QSizeF(1920, 1080), 0.4, 0.5).isEmpty());
        QVERIFY(!transitionChange(cut, QSizeF(1920, 1080), 0.3, 0.4).isEmpty());
    }


    // Prints what the show's drawing costs: a slide drawn to an image the
    // size of the screen, and a frame of each transition painted from two.
    void showDrawingTimes()
    {
        // A busy slide: a title, eight lines of body text, and shapes with
        // gradients and a shadow.
        auto d = showDeck(3);
        for (int i = 0; i < 2; ++i) {
            Page &slide = *d->pages[i];
            QStringList lines;
            for (int k = 0; k < 8; ++k) lines << QStringLiteral("Point %1 of slide %2, with enough words to fill a line of the body text").arg(k + 1).arg(i + 1);
            d->storyDoc(static_cast<TextItem *>(slide.items[1].get())->storyId)->setPlainText(lines.join('\n'));
            for (int k = 0; k < 4; ++k) {
                auto shape = std::make_shared<ShapeItem>();
                shape->shape = k % 2 ? QStringLiteral("ellipse") : QStringLiteral("roundRect");
                shape->rect = QRectF(60 + k * 220, 330, 200, 150);
                shape->fill = Fill::gradient(ColorRef::rgb(QColor(40 + k * 50, 90, 180)), ColorRef::rgb(QColor(240, 200 - k * 30, 60)), 45 + k * 30);
                shape->fx.shadow.on = k == 0;
                slide.items.push_back(shape);
            }
        }
        LayoutCache cache;
        auto median = [](QVector<double> v) { std::sort(v.begin(), v.end()); return v[v.size() / 2]; };
        for (const QSize size : {QSize(1920, 1080), QSize(3840, 2160)}) {
            QVector<double> renders;
            QImage a, b;
            for (int i = 0; i < 5; ++i) {
                QElapsedTimer t;
                t.start();
                a = renderSlideImage(d.get(), &cache, 0, size, 1.0);
                renders << t.nsecsElapsed() / 1e6;
            }
            b = renderSlideImage(d.get(), &cache, 1, size, 1.0);
            QCOMPARE(a.size(), size);
            QVERIFY(a != b);
            QImage frame(size, QImage::Format_RGB32);
            QStringList line;
            line << QStringLiteral("render %1 ms").arg(median(renders), 0, 'f', 1);
            for (const QString &type : deck::transitionTypes()) {
                deck::Transition tr;
                tr.type = type;
                QVector<double> times;
                for (int i = 0; i < 7; ++i) {
                    QPainter p(&frame);
                    QElapsedTimer t;
                    t.start();
                    paintTransition(&p, tr, a, b, 0.3 + i * 0.05);
                    times << t.nsecsElapsed() / 1e6;
                }
                line << QStringLiteral("%1 %2 ms").arg(type).arg(median(times), 0, 'f', 1);
            }
            for (const char *type : {"wipe", "split"}) {
                deck::Transition tr = showTransition(type, 0.7);
                const QRegion changed = transitionChange(tr, QSizeF(size), 0.5, 0.5 + 1 / 42.0);
                QVector<double> times;
                for (int i = 0; i < 7; ++i) {
                    QPainter p(&frame);
                    p.setClipRegion(changed);
                    QElapsedTimer t;
                    t.start();
                    paintTransition(&p, tr, a, b, 0.5 + 1 / 42.0);
                    times << t.nsecsElapsed() / 1e6;
                }
                line << QStringLiteral("%1 changed strip %2 ms").arg(QString::fromLatin1(type)).arg(median(times), 0, 'f', 1);
            }
            qInfo().noquote() << QStringLiteral("SHOWTIMES %1x%2:").arg(size.width()).arg(size.height()) << line.join(QStringLiteral(", "));
        }
    }

    // The window: the ribbon builds from its file with nothing missing, and
    // the slide commands are steps that undo.
    void windowSlideCommands()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        Editor *ed = w.editor();
        QCOMPARE(w.slidesPane()->count(), 1);
        w.newSlide();
        QCOMPARE(int(ed->doc()->pages.size()), 2);
        QCOMPARE(ed->currentPage(), 1);
        QCOMPARE(deck::layoutType(*ed->doc()->masterFor(*ed->doc()->pages[1])), QStringLiteral("obj"));   // content after a title slide
        QTRY_COMPARE(w.slidesPane()->count(), 2);
        w.duplicateSlide();
        QCOMPARE(int(ed->doc()->pages.size()), 3);
        w.toggleHidden();
        QVERIFY(ed->doc()->pages[2]->hidden);
        w.setLayout(QStringLiteral("titleOnly"));
        QCOMPARE(deck::layoutType(*ed->doc()->masterFor(*ed->doc()->pages[2])), QStringLiteral("titleOnly"));
        ed->undo();
        ed->undo();
        ed->undo();
        QCOMPARE(int(ed->doc()->pages.size()), 2);
        w.deleteSlide();
        QCOMPARE(int(ed->doc()->pages.size()), 1);
        w.deleteSlide();   // the last slide stays
        QCOMPARE(int(ed->doc()->pages.size()), 1);
        // Notes typed in the pane reach the slide as an undo step.
        QTest::keyClicks(w.notesPane(), QStringLiteral("Remember the demo"));
        QVERIFY(w.notesPane()->hasTyping());
        w.notesPane()->commit();
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QStringLiteral("Remember the demo"));
        ed->undo();
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QString());
        QCOMPARE(w.notesPane()->toPlainText(), QString());   // the pane shows the undone notes
        w.newSlide();   // and moving on doesn't write them back
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QString());
        ed->markSaved();
    }

    // The slide show skips hidden slides, goes back, ends after the last.
    void slideShowSteps()
    {
        auto d = deck::newPresentation();
        for (int i = 0; i < 3; ++i) deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        d->pages[2]->hidden = true;
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        QCOMPARE(show.currentSlide(), 0);
        show.next();
        QCOMPARE(show.currentSlide(), 1);
        show.next();
        QCOMPARE(show.currentSlide(), 3);   // past the hidden one
        show.previous();
        QCOMPARE(show.currentSlide(), 1);
        show.goTo(3);
        show.next();   // the end screen
        show.previous();
        QCOMPARE(show.currentSlide(), 3);
        // Drawn as the slide, without crashing, at any size.
        show.resize(320, 180);
        QVERIFY(!show.grab().isNull());
    }
};

QTEST_MAIN(DeckTests)
#include "tst_deck.moc"
