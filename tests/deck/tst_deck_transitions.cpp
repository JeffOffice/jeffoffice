// JeffDeck's tests of slide transitions, morph, and the presenter view.
#include "deckhelpers.h"

class DeckTransitionTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // A transition read from a file plays in the show and shows on the Transitions tab; saved unchanged, the XML read is
    // written back (what the show has no form for stays); changed here, or made here or from an old file, it is written
    // as the presentation program writes it, and reads back the same.
    void transitionsAreReadPlayedAndSaved()
    {
        // Each kind, written and read back.
        const QStringList types = deck::transitionTypes();
        for (const QString &type : types) {
            deck::Transition t;
            t.type = type;
            if (t.hasDirection()) t.dir = type == QLatin1String("reveal") ? QStringLiteral("r") : QStringLiteral("u");
            if (t.hasInOut()) t.dir = QStringLiteral("in");
            t.duration = 1.0;
            t.advanceAfter = 4.5;
            t.advanceOnClick = false;
            // As a slide holds it: the slide's root declares the p: prefix.
            QString xml = deck::transitionXml(t);
            xml.insert(xml.indexOf(QLatin1Char(' ')), QStringLiteral(" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\""));
            QDomDocument dom;
            QVERIFY2(office::parseXml(xml.toUtf8(), &dom), qPrintable(type + QLatin1Char(' ') + xml));
            const deck::Transition back = deck::transitionFromXml(xml);
            QVERIFY2(back == t, qPrintable(type + QLatin1Char(' ') + xml + QLatin1Char(' ') + QString::fromUtf8(QJsonDocument(back.toJson()).toJson(QJsonDocument::Compact))));
        }
        QVERIFY(deck::transitionXml(deck::Transition()).isEmpty());
        QVERIFY(deck::transitionXml(deck::Transition{QStringLiteral("reveal"), QStringLiteral("l")}).contains(QStringLiteral("<mc:Fallback><p:transition spd=\"med\"><p:fade/>")));

        // Read from a file: the sample slide fades at medium speed.
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(deck::transitionOf(*d->pages[0]).type, QStringLiteral("fade"));
        QCOMPARE(deck::transitionOf(*d->pages[0]).duration, 0.75);
        // An effect the show has no form for plays as a fade, and is written back as read.
        d->pages[0]->props[QStringLiteral("transitionXml")] = QStringLiteral("<p:transition xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" spd=\"slow\"><p:dissolve/></p:transition>");
        deck::setTransition(*d->pages[0], deck::transitionFromXml(d->pages[0]->props.value(QStringLiteral("transitionXml")).toString()));
        QCOMPARE(deck::transitionOf(*d->pages[0]).type, QStringLiteral("fade"));
        // A transition set here on the second slide.
        deck::Transition push;
        push.type = QStringLiteral("push");
        push.dir = QStringLiteral("l");
        deck::setTransition(*d->pages[1], push);
        QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<p:dissolve/>")));
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide2.xml")).contains(QStringLiteral("<p:push dir=\"l\"/>")));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(deck::transitionOf(*back->pages[1]) == push);
        // Changed here, the first is written as it is now.
        deck::Transition wipe;
        wipe.type = QStringLiteral("wipe");
        wipe.dir = QStringLiteral("d");
        deck::setTransition(*d->pages[0], wipe);
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        const QString first = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(first.contains(QStringLiteral("<p:wipe dir=\"d\"/>")) && !first.contains(QStringLiteral("dissolve")));
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
                 (QStringList{"none", "morph", "fade", "push", "wipe", "split", "reveal", "cover", "uncover", "zoom", "cut"}));
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

    // Alt+F5 on one screen rehearses: the show in a window and Presenter View
    // in another; Esc in either ends both.
    void windowPresenterViewRehearsal()
    {
        DeckWindow w;
        w.newSlide();
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QVERIFY(w.act("show.presenterView"));
        QCOMPARE(w.act("show.presenterView")->shortcut(), QKeySequence(Qt::ALT | Qt::Key_F5));
        w.act("show.presenterView")->trigger();
        QPointer<SlideShow> show = w.slideShow();
        QVERIFY(show);
        QPointer<PresenterView> pv = show->presenterView();
        QVERIFY(pv);
        QCOMPARE(pv->counterText(), QStringLiteral("Slide 2 of 2"));   // from the current slide
        QTest::keyClick(pv.data(), Qt::Key_Left);
        QCOMPARE(show->currentSlide(), 0);
        QTest::keyClick(show.data(), Qt::Key_Right);
        QCOMPARE(pv->counterText(), QStringLiteral("Slide 2 of 2"));
        QTest::keyClick(pv.data(), Qt::Key_Escape);
        QTRY_VERIFY(show.isNull());
        QTRY_VERIFY(pv.isNull());
        // One screen and F5: the show alone.
        w.startShow(0);
        QVERIFY(w.slideShow());
        QVERIFY(!w.slideShow()->presenterView());
        QTest::keyClick(w.slideShow(), Qt::Key_Escape);
        QTRY_VERIFY(!w.slideShow());
    }

    // The Transitions tab: each command is a step that undoes; Apply To All
    // gives every slide the transition of the one being edited.
    void windowTransitionsTab()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QVERIFY(w.ribbon()->tab(QStringLiteral("Transitions")));
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        QCOMPARE(ed->currentPage(), 2);
        auto current = [&] { return deck::transitionOf(*ed->doc()->pages[ed->currentPage()]); };
        QCOMPARE(current().type, QStringLiteral("none"));
        for (const QString &type : deck::transitionTypes()) QVERIFY2(w.act("transition." + type), qPrintable(type));
        w.act("transition.push")->trigger();
        QCOMPARE(current().type, QStringLiteral("push"));
        QCOMPARE(deck::transitionOf(*ed->doc()->pages[1]).type, QStringLiteral("none"));   // only this slide
        QVERIFY(w.act("transition.push")->isChecked());
        w.act("transition.dir.u")->trigger();
        QCOMPARE(current().dir, QStringLiteral("u"));
        w.act("transition.zoom")->trigger();   // a type that goes in or out takes its default
        QCOMPARE(current().dir, QStringLiteral("in"));
        w.act("transition.dir.out")->trigger();
        QCOMPARE(current().dir, QStringLiteral("out"));
        w.act("transition.push")->trigger();
        QCOMPARE(current().dir, QStringLiteral("l"));   // and back to one that goes in a direction
        // Timing.
        auto *duration = w.ribbon()->findChild<QDoubleSpinBox *>("transitionDuration");
        auto *onClick = w.ribbon()->findChild<QCheckBox *>("advanceOnClick");
        auto *after = w.ribbon()->findChild<QCheckBox *>("advanceAfter");
        auto *seconds = w.ribbon()->findChild<QDoubleSpinBox *>("advanceAfterSeconds");
        QVERIFY(duration && onClick && after && seconds);
        QCOMPARE(duration->value(), 0.7);
        QVERIFY(onClick->isChecked());
        QVERIFY(!after->isChecked());
        QVERIFY(!seconds->isEnabled());
        duration->setValue(1.5);
        QCOMPARE(current().duration, 1.5);
        onClick->setChecked(false);
        QVERIFY(!current().advanceOnClick);
        after->setChecked(true);
        QVERIFY(seconds->isEnabled());
        QVERIFY(current().advanceAfter);
        seconds->setValue(4);
        QCOMPARE(*current().advanceAfter, 4.0);
        // Another slide shows its own settings; coming back shows these.
        ed->setCurrentPage(0);
        QTRY_COMPARE(duration->value(), 0.7);
        QVERIFY(onClick->isChecked());
        QVERIFY(!after->isChecked());
        ed->setCurrentPage(2);
        QTRY_COMPARE(duration->value(), 1.5);
        QVERIFY(!onClick->isChecked());
        QVERIFY(after->isChecked());
        QCOMPARE(seconds->value(), 4.0);
        // Apply To All is one step.
        const deck::Transition mine = current();
        QVERIFY(mine.type == QLatin1String("push"));
        w.act("transition.applyToAll")->trigger();
        for (int i = 0; i < 3; ++i) QVERIFY(deck::transitionOf(*ed->doc()->pages[i]) == mine);
        ed->undo();
        QVERIFY(deck::transitionOf(*ed->doc()->pages[0]) == deck::Transition());
        QVERIFY(deck::transitionOf(*ed->doc()->pages[1]) == deck::Transition());
        QVERIFY(deck::transitionOf(*ed->doc()->pages[2]) == mine);
        // Each of the others was a step too.
        ed->undo();   // seconds
        QCOMPARE(current().advanceAfter.value_or(-1), 5.0);
        ed->undo();   // After on
        QVERIFY(!current().advanceAfter);
        ed->undo();   // On Mouse Click off
        QVERIFY(current().advanceOnClick);
        ed->undo();   // duration
        QCOMPARE(current().duration, 0.7);
        ed->undo();   // push again
        QCOMPARE(current().type, QStringLiteral("zoom"));
        QCOMPARE(current().dir, QStringLiteral("out"));
        // None takes the transition off and the star with it.
        w.act("transition.none")->trigger();
        QVERIFY(deck::transitionOf(*ed->doc()->pages[2]) == deck::Transition());
        QVERIFY(!ed->doc()->pages[2]->props.contains(QStringLiteral("transition")));
    }

    // ---- Morph ----

    // Morph is written as the presentation program writes it (a p159 choice holding p159:morph and a fade for programs that don't
    // know it), and its three options read back; a file's own morph is read the same way.
    void morphTransitionFile()
    {
        const QString p159 = QStringLiteral("http://schemas.microsoft.com/office/powerpoint/2015/09/main");
        deck::Transition t;
        t.type = QStringLiteral("morph");
        QVERIFY(t.isMorph());
        QCOMPARE(t.by, QStringLiteral("objects"));
        QCOMPARE(deck::Transition::morphUnits(), (QStringList{"objects", "words", "characters"}));
        const QMap<QString, QString> options{{"objects", "byObject"}, {"words", "byWord"}, {"characters", "byChar"}};
        for (const QString &by : deck::Transition::morphUnits()) {
            t.by = by;
            t.duration = 2.0;
            const QString xml = deck::transitionXml(t);
            QVERIFY2(xml.contains(QStringLiteral("<mc:Choice xmlns:p159=\"%1\"").arg(p159)), qPrintable(xml));
            QVERIFY2(xml.contains(QStringLiteral("Requires=\"p159\"")), qPrintable(xml));
            QVERIFY2(xml.contains(QStringLiteral("<p:transition spd=\"slow\" p14:dur=\"2000\"><p159:morph option=\"%1\"/></p:transition></mc:Choice>").arg(options[by])), qPrintable(xml));
            QVERIFY2(xml.contains(QStringLiteral("<mc:Fallback><p:transition spd=\"slow\"><p:fade/></p:transition></mc:Fallback>")), qPrintable(xml));
            // As a slide holds it: the slide's root declares the p: prefix.
            QString held = xml;
            held.insert(held.indexOf(QLatin1Char(' ')), QStringLiteral(" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\""));
            QDomDocument dom;
            QVERIFY2(office::parseXml(held.toUtf8(), &dom), qPrintable(xml));
            QVERIFY2(deck::transitionFromXml(held) == t, qPrintable(held));
        }
        // Other times, with the click and the seconds on both forms.
        t.by = QStringLiteral("words");
        t.duration = 0.5;
        t.advanceOnClick = false;
        t.advanceAfter = 3.0;
        QString timed = deck::transitionXml(t);
        QVERIFY2(timed.contains(QStringLiteral("<p:transition spd=\"fast\" advClick=\"0\" advTm=\"3000\" p14:dur=\"500\"><p159:morph")), qPrintable(timed));
        QVERIFY2(timed.contains(QStringLiteral("<mc:Fallback><p:transition spd=\"fast\" advClick=\"0\" advTm=\"3000\"><p:fade/>")), qPrintable(timed));
        timed.insert(timed.indexOf(QLatin1Char(' ')), QStringLiteral(" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\""));
        QVERIFY(deck::transitionFromXml(timed) == t);

        // As the program writes it (p14 declared once above, the time of its own, three seconds).
        const QString theirs =
            QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\""
                           " xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\"><mc:Choice xmlns:p159=\"%1\" Requires=\"p159\">"
                           "<p:transition spd=\"slow\" p14:dur=\"3000\"><p159:morph option=\"byChar\"/></p:transition></mc:Choice>"
                           "<mc:Fallback><p:transition spd=\"slow\"><p:fade/></p:transition></mc:Fallback></mc:AlternateContent>")
                .arg(p159);
        const deck::Transition read = deck::transitionFromXml(theirs);
        QCOMPARE(read.type, QStringLiteral("morph"));
        QCOMPARE(read.by, QStringLiteral("characters"));
        QCOMPARE(read.duration, 3.0);
        // No option, or one this program doesn't know, is objects.
        QString none = theirs;
        none.replace(QStringLiteral(" option=\"byChar\""), QString());
        QCOMPARE(deck::transitionFromXml(none).by, QStringLiteral("objects"));
        QString odd = theirs;
        odd.replace(QStringLiteral("byChar"), QStringLiteral("byParagraph"));
        QCOMPARE(deck::transitionFromXml(odd).type, QStringLiteral("morph"));
        QCOMPARE(deck::transitionFromXml(odd).by, QStringLiteral("objects"));
        // A morph in another namespace isn't one; neither is another effect of the namespace: the fallback is read.
        QString other = theirs;
        other.replace(QStringLiteral("<p159:morph option=\"byChar\"/>"), QStringLiteral("<p159:spin/>"));
        QCOMPARE(deck::transitionFromXml(other).type, QStringLiteral("fade"));
        QString elsewhere = theirs;
        elsewhere.replace(p159, QStringLiteral("urn:elsewhere"));
        QCOMPARE(deck::transitionFromXml(elsewhere).type, QStringLiteral("fade"));
        // The model: a by that means nothing to another type goes; one that isn't a unit takes the default.
        QCOMPARE(deck::Transition::fromJson({{"type", "morph"}, {"by", "words"}}).by, QStringLiteral("words"));
        QCOMPARE(deck::Transition::fromJson({{"type", "morph"}, {"by", "letters"}}).by, QStringLiteral("objects"));
        QCOMPARE(deck::Transition::fromJson({{"type", "fade"}, {"by", "words"}}).by, QStringLiteral("objects"));
        QVERIFY(!deck::Transition::fromJson({{"type", "fade"}}).toJson().contains(QStringLiteral("by")));
        deck::Transition j;
        j.type = QStringLiteral("morph");
        j.by = QStringLiteral("characters");
        j.duration = 2.0;
        QVERIFY(deck::Transition::fromJson(j.toJson()) == j);
        QVERIFY(j != deck::Transition::fromJson({{"type", "morph"}, {"duration", 2.0}}));
    }

    // A morph on a slide is saved as the program writes it and comes back; a file's own morph, saved unchanged, is written as it was.
    void morphTransitionIsSavedAndKept()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        deck::Transition t;
        t.type = QStringLiteral("morph");
        t.by = QStringLiteral("words");
        t.duration = 2.0;
        t.advanceAfter = 4.0;
        deck::setTransition(*d->pages[1], t);
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString second = part(e, QStringLiteral("/ppt/slides/slide2.xml"));
        QVERIFY(second.contains(QStringLiteral("Requires=\"p159\"")));
        QVERIFY(second.contains(QStringLiteral("<p159:morph option=\"byWord\"/>")));
        QVERIFY(second.contains(QStringLiteral("p14:dur=\"2000\"")));
        QVERIFY(second.contains(QStringLiteral("<mc:Fallback><p:transition spd=\"slow\" advTm=\"4000\"><p:fade/>")));
        QDomDocument dom;
        QVERIFY(office::parseXml(second.toUtf8(), &dom));
        auto back = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(deck::transitionOf(*back->pages[1]) == t);
        // What a file holds beyond what the show has a form for stays when it is not changed here.
        const QString theirs =
            QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\""
                           " xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\" xmlns:zz=\"urn:kept\"><mc:Choice xmlns:p159=\"http://schemas.microsoft.com/office/powerpoint/2015/09/main\" Requires=\"p159\">"
                           "<p:transition spd=\"slow\" p14:dur=\"2500\"><p159:morph option=\"byChar\" zz:mark=\"1\"/></p:transition></mc:Choice>"
                           "<mc:Fallback><p:transition spd=\"slow\"><p:fade/></p:transition></mc:Fallback></mc:AlternateContent>");
        d->pages[0]->props[QStringLiteral("transitionXml")] = theirs;
        deck::setTransition(*d->pages[0], deck::transitionFromXml(theirs));
        QCOMPARE(deck::transitionOf(*d->pages[0]).by, QStringLiteral("characters"));
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("zz:mark=\"1\"")));
        // Changed here, it is written from what the show has.
        deck::Transition changed = deck::transitionOf(*d->pages[0]);
        changed.by = QStringLiteral("objects");
        deck::setTransition(*d->pages[0], changed);
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        const QString first = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(!first.contains(QStringLiteral("zz:mark")));
        QVERIFY(first.contains(QStringLiteral("<p159:morph option=\"byObject\"/>")));
    }

    // Morph is in the Transitions tab's gallery with an icon of its own, takes two seconds, and has Objects, Words, and Characters
    // for its Effect Options; each change is one step.
    void windowMorphOnTheTransitionsTab()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        auto current = [&] { return deck::transitionOf(*ed->doc()->pages[ed->currentPage()]); };
        // In the gallery, second after None, with a picture.
        auto *gallery = w.ribbon()->findChild<Gallery *>(QStringLiteral("transitionGallery"));
        QVERIFY(gallery);
        auto *list = gallery->findChild<QListWidget *>();
        QVERIFY(list);
        QCOMPARE(list->count(), int(deck::transitionTypes().size()));
        QCOMPARE(list->item(1)->data(Qt::UserRole).toString(), QStringLiteral("morph"));
        QCOMPARE(list->item(1)->toolTip(), w.act("transition.morph")->text());
        QVERIFY(!list->item(1)->icon().isNull());
        // Its picture is its own, not the fade's.
        const QSize pic = list->iconSize();
        QVERIFY(list->item(1)->icon().pixmap(pic).toImage() != list->item(2)->icon().pixmap(pic).toImage());
        // The options mean nothing to the other types.
        for (const QString &by : deck::Transition::morphUnits()) QVERIFY2(w.act("transition.by." + by), qPrintable(by));
        w.act("transition.fade")->trigger();
        for (const QString &by : deck::Transition::morphUnits()) QVERIFY(!w.act("transition.by." + by)->isEnabled());
        // Choosing it gives two seconds, objects, and the Preview.
        w.act("transition.morph")->trigger();
        QCOMPARE(current().type, QStringLiteral("morph"));
        QCOMPARE(current().duration, 2.0);
        QCOMPARE(current().by, QStringLiteral("objects"));
        QVERIFY(w.act("transition.morph")->isChecked());
        QVERIFY(w.act("transition.by.objects")->isChecked());
        QVERIFY(w.act("transition.by.words")->isEnabled() && !w.act("transition.by.words")->isChecked());
        auto *duration = w.ribbon()->findChild<QDoubleSpinBox *>(QStringLiteral("transitionDuration"));
        QVERIFY(duration);
        QTRY_COMPARE(duration->value(), 2.0);
        QVERIFY(w.act("transition.preview")->isEnabled());
        // Effect Options.
        w.act("transition.by.words")->trigger();
        QCOMPARE(current().by, QStringLiteral("words"));
        QVERIFY(w.act("transition.by.words")->isChecked() && !w.act("transition.by.objects")->isChecked());
        w.act("transition.by.characters")->trigger();
        QCOMPARE(current().by, QStringLiteral("characters"));
        QCOMPARE(current().duration, 2.0);
        ed->undo();
        QCOMPARE(current().by, QStringLiteral("words"));
        QVERIFY(w.act("transition.by.words")->isChecked());
        // The menu lists the three and no directions.
        QToolButton *effects = nullptr;
        for (auto *b : w.ribbon()->findChildren<QToolButton *>())
            if (b->menu() && b->text() == QStringLiteral("Effect Options")) effects = b;
        QVERIFY(effects);
        QStringList listed;
        Q_EMIT effects->menu()->aboutToShow();
        for (QAction *a : effects->menu()->actions()) listed << a->text();
        QCOMPARE(listed, (QStringList{w.act("transition.by.objects")->text(), w.act("transition.by.words")->text(), w.act("transition.by.characters")->text()}));
        // A time that was chosen stays; the usual one goes back with the type.
        duration->setValue(3.0);
        w.act("transition.push")->trigger();
        QCOMPARE(current().duration, 3.0);
        QCOMPARE(current().by, QStringLiteral("objects"));   // a morph's own option goes with it
        w.act("transition.morph")->trigger();
        QCOMPARE(current().duration, 3.0);
        duration->setValue(2.0);
        w.act("transition.fade")->trigger();
        QCOMPARE(current().duration, 0.7);
        // None takes a morph away whole.
        w.act("transition.morph")->trigger();
        w.act("transition.by.words")->trigger();
        w.act("transition.none")->trigger();
        QVERIFY(!ed->doc()->pages[ed->currentPage()]->props.contains(QStringLiteral("transition")));
        // Apply To All carries it, and the Preview plays it over the slide area.
        ed->setCurrentPage(1);
        w.act("transition.morph")->trigger();
        w.act("transition.by.characters")->trigger();
        w.act("transition.applyToAll")->trigger();
        for (int i = 0; i < 3; ++i) QCOMPARE(deck::transitionOf(*ed->doc()->pages[i]).by, QStringLiteral("characters"));
        w.show();
        w.act("transition.preview")->trigger();
        auto *preview = w.findChild<TransitionPreview *>(QStringLiteral("transitionPreview"));
        QVERIFY(preview);
        QVERIFY(preview->isPlaying());
        QTest::keyClick(preview, Qt::Key_Escape);
        QVERIFY(!preview->isPlaying());
    }

    // A deck of empty white slides to put objects on.
    static std::unique_ptr<Document> morphDeck(int slides)
    {
        auto d = deck::newPresentation();
        while (int(d->pages.size()) < slides) deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        for (auto &page : d->pages) page->items.clear();
        return d;
    }

    static ItemPtr morphShape(const QString &name, const QString &preset, const QRectF &rect, const QColor &color = QColor(0xC0, 0x20, 0x20))
    {
        auto s = std::make_shared<ShapeItem>();
        s->name = name;
        s->shape = preset;
        s->rect = rect;
        s->fill = Fill::solid(ColorRef::rgb(color));
        s->stroke = Stroke::none();
        return s;
    }

    static ItemPtr morphText(Document &d, const QString &name, const QString &words, const QRectF &rect)
    {
        auto t = std::make_shared<TextItem>();
        t->name = name;
        t->storyId = d.createStory(words);
        t->rect = rect;
        QTextCharFormat cf;
        cf.setFontPointSize(32);
        cf.setForeground(QColor(Qt::black));
        QTextCursor c(d.storyDoc(t->storyId));
        c.select(QTextCursor::Document);
        c.mergeCharFormat(cf);
        return t;
    }

    static ItemPtr morphPicture(Document &d, const QString &name, const QColor &color, const QRectF &rect)
    {
        auto p = std::make_shared<PictureItem>();
        p->name = name;
        QImage img(8, 8, QImage::Format_RGB32);
        img.fill(color);
        p->imageId = d.addImage(imageBytes(img), QStringLiteral("png"));
        p->rect = rect;
        p->imgRect = QRectF(QPointF(0, 0), rect.size());
        return p;
    }

    // How many pixels of two frames differ by more than a little.
    static int morphDifference(const QImage &a, const QImage &b)
    {
        if (a.size() != b.size()) return a.width() * a.height();
        int n = 0;
        for (int y = 0; y < a.height(); ++y)
            for (int x = 0; x < a.width(); ++x) {
                const QColor p = a.pixelColor(x, y), q = b.pixelColor(x, y);
                if (std::abs(p.red() - q.red()) > 12 || std::abs(p.green() - q.green()) > 12 || std::abs(p.blue() - q.blue()) > 12) ++n;
            }
        return n;
    }

    static bool morphColorIs(const QImage &frame, int x, int y, const QColor &want, int slack = 3)
    {
        const QColor got = frame.pixelColor(x, y);
        return std::abs(got.red() - want.red()) <= slack && std::abs(got.green() - want.green()) <= slack && std::abs(got.blue() - want.blue()) <= slack;
    }

    // Which objects of one slide go with which of the next: a forced name, then the kind and name, the text, the picture, and the kind of
    // shape; the nearest first; each object once.
    void morphPairsObjects()
    {
        auto d = deck::newPresentation();
        using Rule = MorphPair::Rule;
        // Names that start with !! make a pair of any two objects.
        {
            ItemList from{morphShape("!!hero", "rect", QRectF(0, 0, 10, 10))}, to{morphText(*d, "!!hero", "Hello", QRectF(200, 0, 50, 20))};
            const MorphPlan plan = planMorph(*d, from, to);
            QCOMPARE(plan.pairs.size(), 1);
            QVERIFY(plan.pairs[0].rule == Rule::Forced);
            QVERIFY(plan.leaving.isEmpty() && plan.arriving.isEmpty());
            // Both names have to start with it, and be the same; one that doesn't is a name like any other.
            to = {morphText(*d, "!!other", "Hello", QRectF(200, 0, 50, 20))};
            QVERIFY(planMorph(*d, from, to).pairs.isEmpty());
            to = {morphShape("hero", "ellipse", QRectF(200, 0, 50, 20))};
            QVERIFY(planMorph(*d, from, to).pairs.isEmpty());
        }
        // The same kind of object with the same name.
        {
            ItemList from{morphShape("Title 1", "rect", QRectF(0, 0, 10, 10)), morphText(*d, "Title 1", "A", QRectF(0, 50, 10, 10))};
            ItemList to{morphText(*d, "Title 1", "B", QRectF(100, 0, 10, 10))};
            const MorphPlan plan = planMorph(*d, from, to);
            QCOMPARE(plan.pairs.size(), 1);
            QVERIFY(plan.pairs[0].rule == Rule::Name);
            QCOMPARE(plan.pairs[0].from, 1);   // the text box, not the shape of the same name
            QCOMPARE(plan.leaving, (QVector<int>{0}));
        }
        // The same kind of object with the same text, whatever the names.
        {
            ItemList from{morphText(*d, "One", "Quarterly results", QRectF(0, 0, 100, 20)), morphText(*d, "Two", "Other words", QRectF(0, 50, 100, 20))};
            ItemList to{morphText(*d, "Three", "  Quarterly   results ", QRectF(300, 300, 100, 20)), morphText(*d, "Four", "New words", QRectF(0, 50, 100, 20))};
            const MorphPlan plan = planMorph(*d, from, to);
            QCOMPARE(plan.pairs.size(), 1);
            QVERIFY(plan.pairs[0].rule == Rule::Text);
            QCOMPARE(plan.pairs[0].from, 0);
            QCOMPARE(plan.pairs[0].to, 0);
            QCOMPARE(plan.leaving, (QVector<int>{1}));
            QCOMPARE(plan.arriving, (QVector<int>{1}));
        }
        // The same picture, renamed; a picture of another image is not it.
        {
            ItemPtr red = morphPicture(*d, "A", QColor(Qt::red), QRectF(0, 0, 80, 60)), blue = morphPicture(*d, "B", QColor(Qt::blue), QRectF(0, 100, 80, 60));
            ItemPtr redAgain = morphPicture(*d, "C", QColor(Qt::red), QRectF(400, 0, 160, 120));
            QCOMPARE(static_cast<PictureItem *>(redAgain.get())->imageId, static_cast<PictureItem *>(red.get())->imageId);
            const MorphPlan plan = planMorph(*d, ItemList{red, blue}, ItemList{redAgain});
            QCOMPARE(plan.pairs.size(), 1);
            QVERIFY(plan.pairs[0].rule == Rule::Picture);
            QCOMPARE(plan.pairs[0].from, 0);
            QCOMPARE(plan.leaving, (QVector<int>{1}));
        }
        // The same kind of shape: a preset with itself, a line with a line; a text box with another is not one.
        {
            ItemPtr line = std::make_shared<LineItem>(), line2 = std::make_shared<LineItem>();
            ItemList from{morphShape("a", "ellipse", QRectF(0, 0, 10, 10)), morphShape("b", "rect", QRectF(50, 0, 10, 10)), line, morphText(*d, "t", "x", QRectF(0, 90, 10, 10))};
            ItemList to{line2, morphShape("c", "ellipse", QRectF(300, 0, 10, 10)), morphShape("d", "triangle", QRectF(50, 0, 10, 10)), morphText(*d, "u", "y", QRectF(0, 90, 10, 10))};
            const MorphPlan plan = planMorph(*d, from, to);
            QCOMPARE(plan.pairs.size(), 2);
            for (const MorphPair &p : plan.pairs) QVERIFY(p.rule == Rule::Shape);
            QCOMPARE(plan.pairs[0].to, 0);   // by the new slide's order: the line
            QCOMPARE(plan.pairs[0].from, 2);
            QCOMPARE(plan.pairs[1].to, 1);
            QCOMPARE(plan.pairs[1].from, 0);
            QCOMPARE(plan.leaving, (QVector<int>{1, 3}));
            QCOMPARE(plan.arriving, (QVector<int>{2, 3}));
        }
        // The rules go in order, and an object is in one pair: the name takes the circle before the shape rule can.
        {
            ItemList from{morphShape("x", "ellipse", QRectF(0, 0, 10, 10)), morphShape("y", "ellipse", QRectF(500, 0, 10, 10))};
            ItemList to{morphShape("y", "ellipse", QRectF(0, 0, 10, 10)), morphShape("z", "ellipse", QRectF(500, 0, 10, 10))};
            const MorphPlan plan = planMorph(*d, from, to);
            QCOMPARE(plan.pairs.size(), 2);
            QCOMPARE(plan.pairs[0].from, 1);   // y with y
            QCOMPARE(plan.pairs[0].to, 0);
            QVERIFY(plan.pairs[0].rule == Rule::Name);
            QCOMPARE(plan.pairs[1].from, 0);   // what is left
            QCOMPARE(plan.pairs[1].to, 1);
            QVERIFY(plan.pairs[1].rule == Rule::Shape);
        }
        // Among several alike, the nearest go together.
        {
            ItemList from{morphShape("a", "ellipse", QRectF(0, 100, 20, 20)), morphShape("b", "ellipse", QRectF(400, 100, 20, 20))};
            ItemList to{morphShape("c", "ellipse", QRectF(380, 100, 20, 20)), morphShape("d", "ellipse", QRectF(20, 100, 20, 20))};
            const MorphPlan plan = planMorph(*d, from, to);
            QCOMPARE(plan.pairs.size(), 2);
            QCOMPARE(plan.pairs[0].to, 0);
            QCOMPARE(plan.pairs[0].from, 1);
            QCOMPARE(plan.pairs[1].to, 1);
            QCOMPARE(plan.pairs[1].from, 0);
        }
        // Nothing on either: nothing to pair.
        const MorphPlan empty = planMorph(*d, ItemList(), ItemList());
        QVERIFY(empty.pairs.isEmpty() && empty.leaving.isEmpty() && empty.arriving.isEmpty());
    }

    // A pair moves, grows, turns, and changes color; the others fade; the ends are the slides.
    void morphFramesMoveAndFade()
    {
        auto d = morphDeck(2);
        const QSize size = d->pageSize().toSize();
        d->pages[0]->items.push_back(morphShape("R", "rect", QRectF(100, 100, 100, 100), QColor(255, 0, 0)));
        d->pages[1]->items.push_back(morphShape("R", "rect", QRectF(500, 300, 200, 200), QColor(0, 0, 255)));
        // One that only the old slide has, and one that only the new has.
        d->pages[0]->items.push_back(morphShape("Gone", "ellipse", QRectF(700, 50, 100, 100), QColor(0, 0, 0)));
        d->pages[1]->items.push_back(morphShape("New", "triangle", QRectF(50, 400, 100, 100), QColor(0, 0, 0)));
        MorphFrames morph(d.get(), 0, 1, QStringLiteral("objects"), size, 1.0);
        QVERIFY(morph.isUsable());
        QCOMPARE(morph.plan().pairs.size(), 1);
        QCOMPARE(morph.plan().leaving, (QVector<int>{1}));
        QCOMPARE(morph.plan().arriving, (QVector<int>{1}));
        const QColor white(Qt::white);
        const QImage start = morph.frame(0), middle = morph.frame(0.5), end = morph.frame(1);
        QCOMPARE(start.size(), size);
        QVERIFY(morphColorIs(start, 150, 150, QColor(255, 0, 0)));
        QVERIFY(morphColorIs(start, 600, 400, white));
        QVERIFY(morphColorIs(start, 750, 100, QColor(0, 0, 0)));   // fully there at the start
        QVERIFY(morphColorIs(start, 100, 450, white));             // not yet
        QVERIFY(morphColorIs(end, 600, 400, QColor(0, 0, 255)));
        QVERIFY(morphColorIs(end, 150, 150, white));
        QVERIFY(morphColorIs(end, 750, 100, white));
        QVERIFY(morphColorIs(end, 100, 450, QColor(0, 0, 0)));
        // Halfway the red square of 100 at 150,150 and the blue one of 200 at 600,400 are one of 150 at 375,275, in between colors.
        QVERIFY(morphColorIs(middle, 375, 275, QColor(128, 0, 128), 4));
        QVERIFY(morphColorIs(middle, 305, 205, QColor(128, 0, 128), 4));
        QVERIFY(morphColorIs(middle, 445, 345, QColor(128, 0, 128), 4));
        QVERIFY(morphColorIs(middle, 290, 275, white));
        QVERIFY(morphColorIs(middle, 460, 275, white));
        QVERIFY(morphColorIs(middle, 150, 150, white));
        QVERIFY(morphColorIs(middle, 600, 400, white));
        // The others are half there.
        QVERIFY(morphColorIs(middle, 750, 100, QColor(127, 127, 127), 4));
        QVERIFY(morphColorIs(middle, 100, 450, QColor(127, 127, 127), 4));
        // Easing is the caller's: a quarter is a quarter of the way.
        QVERIFY(morphColorIs(morph.frame(0.25), 750, 100, QColor(64, 64, 64), 4));
        // Out of range is the ends.
        QVERIFY(morphDifference(morph.frame(-3), start) == 0 && morphDifference(morph.frame(7), end) == 0);
        // The ends are the slides.
        LayoutCache cache;
        QVERIFY(morphDifference(start, renderSlideImage(d.get(), &cache, 0, size, 1.0)) < 40);
        QVERIFY(morphDifference(end, renderSlideImage(d.get(), &cache, 1, size, 1.0)) < 40);
    }

    // A pair turns the shorter way, and two shapes of one color that differ in geometry dissolve without dimming where they overlap.
    void morphFramesTurnAndDissolve()
    {
        auto d = morphDeck(2);
        const QSize size = d->pageSize().toSize();
        ItemPtr bar = morphShape("Bar", "rect", QRectF(100, 200, 300, 20), QColor(0, 128, 0));
        d->pages[0]->items.push_back(bar);
        ItemPtr turned = morphShape("Bar", "rect", QRectF(100, 200, 300, 20), QColor(0, 128, 0));
        turned->rotation = 90;
        d->pages[1]->items.push_back(turned);
        d->pages[0]->items.push_back(morphShape("S", "ellipse", QRectF(600, 100, 200, 100), QColor(200, 0, 0)));
        d->pages[1]->items.push_back(morphShape("S", "rect", QRectF(600, 100, 200, 100), QColor(200, 0, 0)));
        MorphFrames morph(d.get(), 0, 1, QStringLiteral("objects"), size, 1.0);
        QCOMPARE(morph.plan().pairs.size(), 2);
        const QImage middle = morph.frame(0.5);
        const QColor green(0, 128, 0), white(Qt::white);
        // At 45 degrees: along the diagonal through the middle (250,210), not along the level or the upright.
        QVERIFY(morphColorIs(middle, 250 + 70, 210 + 70, green));
        QVERIFY(morphColorIs(middle, 250 - 70, 210 - 70, green));
        QVERIFY(morphColorIs(middle, 250 + 100, 210, white));
        QVERIFY(morphColorIs(middle, 250, 210 + 100, white));
        // The ellipse and the rectangle, one color: the middle of both is that color, not a dimmer one; a corner only the rectangle has is half.
        QVERIFY(morphColorIs(middle, 700, 150, QColor(200, 0, 0)));
        QVERIFY(morphColorIs(middle, 603, 103, QColor(228, 128, 128), 6));
        // The turn is the short way from 350 to 10 degrees, through 0.
        auto d2 = morphDeck(2);
        ItemPtr a = morphShape("Bar", "rect", QRectF(100, 200, 300, 20), QColor(0, 128, 0)), b = morphShape("Bar", "rect", QRectF(100, 200, 300, 20), QColor(0, 128, 0));
        a->rotation = 350;
        b->rotation = 10;
        d2->pages[0]->items.push_back(a);
        d2->pages[1]->items.push_back(b);
        MorphFrames flat(d2.get(), 0, 1, QStringLiteral("objects"), size, 1.0);
        const QImage level = flat.frame(0.5);
        QVERIFY(morphColorIs(level, 250 + 140, 210, green));   // level: the far end of the bar is on the line
        QVERIFY(morphColorIs(level, 250 - 140, 210, green));
    }

    // Text that is not alike fades into the other text in the same place, where it is alike it stays; a picture of one image grows as its frame does.
    void morphFramesTextAndPictures()
    {
        auto d = morphDeck(2);
        const QSize size = d->pageSize().toSize();
        d->pages[0]->items.push_back(morphText(*d, "Words", "AAAA AAAA", QRectF(100, 100, 400, 100)));
        d->pages[1]->items.push_back(morphText(*d, "Words", "BBBB BBBB", QRectF(100, 300, 400, 100)));
        d->pages[0]->items.push_back(morphText(*d, "Same", "Steady words", QRectF(100, 450, 400, 60)));
        d->pages[1]->items.push_back(morphText(*d, "Same", "Steady words", QRectF(100, 450, 400, 60)));
        MorphFrames morph(d.get(), 0, 1, QStringLiteral("objects"), size, 1.0);
        QCOMPARE(morph.plan().pairs.size(), 2);
        const QImage start = morph.frame(0), middle = morph.frame(0.5), end = morph.frame(1);
        QVERIFY(morphDifference(start, middle) > 50);
        QVERIFY(morphDifference(middle, end) > 50);
        QVERIFY(morphDifference(start, end) > 50);
        // The text that is the same is not dimmed in the middle.
        auto dark = [&](const QImage &img, const QRect &area) {
            int n = 0;
            for (int y = area.top(); y < area.bottom(); ++y)
                for (int x = area.left(); x < area.right(); ++x) n += img.pixelColor(x, y).lightness() < 100;
            return n;
        };
        const QRect same(100, 450, 400, 60);
        QVERIFY(dark(start, same) > 100);
        QCOMPARE(dark(middle, same), dark(start, same));
        QCOMPARE(dark(end, same), dark(start, same));
        // The ends are the slides.
        LayoutCache cache;
        QVERIFY(morphDifference(start, renderSlideImage(d.get(), &cache, 0, size, 1.0)) < 40);
        QVERIFY(morphDifference(end, renderSlideImage(d.get(), &cache, 1, size, 1.0)) < 40);

        // A picture of the same image, renamed, in another frame.
        auto p = morphDeck(2);
        p->pages[0]->items.push_back(morphPicture(*p, "A", QColor(0, 0, 255), QRectF(100, 100, 100, 100)));
        p->pages[1]->items.push_back(morphPicture(*p, "B", QColor(0, 0, 255), QRectF(500, 300, 200, 200)));
        MorphFrames pic(p.get(), 0, 1, QStringLiteral("objects"), size, 1.0);
        QCOMPARE(pic.plan().pairs.size(), 1);
        const QImage half = pic.frame(0.5);
        QVERIFY(morphColorIs(half, 375, 275, QColor(0, 0, 255)));
        QVERIFY(morphColorIs(half, 305, 205, QColor(0, 0, 255)));
        QVERIFY(morphColorIs(half, 290, 275, QColor(Qt::white)));
        QVERIFY(morphColorIs(half, 150, 150, QColor(Qt::white)));
    }

    // By the word or the character, the words (characters) two texts share move to their new places instead of fading: halfway they are as
    // dark as at the ends; what they don't share fades; a text too long for a morph to move fades as it does by object.
    void morphFramesMoveWordsAndCharacters()
    {
        auto d = morphDeck(2);
        const QSize size = d->pageSize().toSize();
        const QRectF box(60, 200, 840, 90);
        d->pages[0]->items.push_back(morphText(*d, "Line", "WWWW bbbbbbbbbb iiii", box));
        d->pages[1]->items.push_back(morphText(*d, "Line", "iiii bbbbbbbbbb WWWW", box));
        // The pixels that are black: a word that is moving whole is, one that is fading is not (where words pass over each other, ink is counted once).
        auto dark = [&](const QImage &img) {
            int n = 0;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x) n += img.pixelColor(x, y).lightness() < 40;
            return n;
        };
        MorphFrames objects(d.get(), 0, 1, QStringLiteral("objects"), size, 1.0);
        MorphFrames words(d.get(), 0, 1, QStringLiteral("words"), size, 1.0);
        MorphFrames characters(d.get(), 0, 1, QStringLiteral("characters"), size, 1.0);
        QVERIFY2(words.describe().join('\n').contains(QStringLiteral("3 of its words move")), qPrintable(words.describe().join('\n')));
        QVERIFY2(characters.describe().join('\n').contains(QStringLiteral("18 of its characters move")), qPrintable(characters.describe().join('\n')));
        QVERIFY(!objects.describe().join('\n').contains(QStringLiteral("move")));
        const int atStart = dark(words.frame(0));
        QVERIFY(atStart > 500);
        // By object the two texts fade into each other: not as dark halfway. Moving, they are.
        QVERIFY(dark(objects.frame(0.5)) < atStart * 0.6);
        QVERIFY(dark(words.frame(0.5)) > atStart * 0.8);
        QVERIFY(dark(characters.frame(0.5)) > atStart * 0.8);
        QVERIFY(dark(words.frame(0.3)) > atStart * 0.8);
        // They are somewhere between: not the old picture, not the new.
        QVERIFY(morphDifference(words.frame(0.5), words.frame(0)) > 200);
        QVERIFY(morphDifference(words.frame(0.5), words.frame(1)) > 200);
        QVERIFY(morphDifference(characters.frame(0.5), characters.frame(0)) > 200);
        QVERIFY(morphDifference(characters.frame(0.5), characters.frame(1)) > 200);
        // And the ends are the slides.
        LayoutCache cache;
        const QImage oldSlide = renderSlideImage(d.get(), &cache, 0, size, 1.0), newSlide = renderSlideImage(d.get(), &cache, 1, size, 1.0);
        for (MorphFrames *m : {&words, &characters}) {
            QVERIFY2(morphDifference(m->frame(0), oldSlide) < 60, qPrintable(QString::number(morphDifference(m->frame(0), oldSlide))));
            QVERIFY2(morphDifference(m->frame(1), newSlide) < 60, qPrintable(QString::number(morphDifference(m->frame(1), newSlide))));
        }
        // The box that holds the words moves and turns with them.
        auto turned = morphDeck(2);
        turned->pages[0]->items.push_back(morphText(*turned, "Line", "alpha beta", QRectF(100, 100, 400, 60)));
        ItemPtr there = morphText(*turned, "Line", "beta alpha", QRectF(300, 300, 400, 60));
        there->rotation = 20;
        turned->pages[1]->items.push_back(there);
        MorphFrames spinning(turned.get(), 0, 1, QStringLiteral("words"), size, 1.0);
        QVERIFY2(spinning.describe().join('\n').contains(QStringLiteral("2 of its words move")), qPrintable(spinning.describe().join('\n')));
        // Turned text is smoothed into grays, more on macOS than elsewhere, so ink is
        // counted at a lighter cut here; a word fading at half (lightness about 127) still isn't.
        auto inked = [&](const QImage &img) {
            int n = 0;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x) n += img.pixelColor(x, y).lightness() < 110;
            return n;
        };
        QVERIFY(inked(spinning.frame(0)) > 100);
        QVERIFY2(inked(spinning.frame(0.5)) > inked(spinning.frame(0)) * 0.7, qPrintable(QStringLiteral("%1 of %2").arg(inked(spinning.frame(0.5))).arg(inked(spinning.frame(0)))));   // whole words on the way
        QVERIFY(morphDifference(spinning.frame(0.5), spinning.frame(0)) > 200);
        QVERIFY(morphDifference(spinning.frame(0.5), spinning.frame(1)) > 200);
        // Nothing in common: the texts fade, as they do by object.
        auto apart = morphDeck(2);
        apart->pages[0]->items.push_back(morphText(*apart, "Line", "aaaa bbbb", box));
        apart->pages[1]->items.push_back(morphText(*apart, "Line", "zzzz yyyy", box));
        MorphFrames fading(apart.get(), 0, 1, QStringLiteral("words"), size, 1.0);
        QVERIFY(!fading.describe().join('\n').contains(QStringLiteral("move")));
        QVERIFY(dark(fading.frame(0.5)) < dark(fading.frame(0)) * 0.6);
        // Words that are not alike (a size and a color) fade as they move; the words of a shape's text move too.
        auto looks = morphDeck(2);
        auto shapeWith = [&](const QString &words, double size, const QColor &color, const QRectF &rect) {
            ItemPtr s = morphShape("Card", "rect", rect, QColor(Qt::white));
            auto *shape = static_cast<ShapeItem *>(s.get());
            shape->storyId = looks->createStory(words);
            QTextCharFormat cf;
            cf.setFontPointSize(size);
            cf.setForeground(color);
            QTextCursor c(looks->storyDoc(shape->storyId));
            c.select(QTextCursor::Document);
            c.mergeCharFormat(cf);
            return s;
        };
        looks->pages[0]->items.push_back(shapeWith(QStringLiteral("one two three"), 20, QColor(Qt::black), QRectF(60, 100, 300, 80)));
        looks->pages[1]->items.push_back(shapeWith(QStringLiteral("three two one"), 40, QColor(Qt::red), QRectF(300, 300, 500, 120)));
        MorphFrames changing(looks.get(), 0, 1, QStringLiteral("words"), size, 1.0);
        QVERIFY2(changing.describe().join('\n').contains(QStringLiteral("3 of its words move")), qPrintable(changing.describe().join('\n')));
        const QImage halfway = changing.frame(0.5);
        QVERIFY(morphDifference(halfway, changing.frame(0)) > 200);
        QVERIFY(morphDifference(halfway, changing.frame(1)) > 200);
        // Past what a morph holds, the words fade as they do by object, and quickly.
        auto longer = morphDeck(2);
        QString many;
        for (int i = 0; i < kMorphMaxTextCharacters / 3 + 10; ++i) many += QStringLiteral("ab ");
        longer->pages[0]->items.push_back(morphText(*longer, "Line", many, QRectF(0, 0, 900, 500)));
        longer->pages[1]->items.push_back(morphText(*longer, "Line", many + QStringLiteral("cd"), QRectF(0, 0, 900, 500)));
        const MorphWork overlongBefore = morphWorkForTest();
        auto overlong = std::make_unique<MorphFrames>(longer.get(), 0, 1, QStringLiteral("words"), size, 1.0);
        QVERIFY(!overlong->describe().join('\n').contains(QStringLiteral("move")));
        QVERIFY(!overlong->frame(0.5).isNull());
        // Counted: the words are not cut into tokens or paired, and no picture of the text is painted for moving it.
        QCOMPARE(morphWorkForTest().tokensMade, overlongBefore.tokensMade);
        QCOMPARE(morphWorkForTest().textPicturesPainted, overlongBefore.textPicturesPainted);
    }

    // Objects the animations hide are not in the morph; a slide with more objects than a morph holds fades.
    void morphFramesStatesAndLimits()
    {
        auto d = morphDeck(2);
        const QSize size = d->pageSize().toSize();
        ItemPtr shown = morphShape("A", "rect", QRectF(100, 100, 100, 100)), hidden = morphShape("B", "rect", QRectF(400, 100, 100, 100)), moved = morphShape("C", "ellipse", QRectF(100, 400, 100, 100));
        shown->sourceId = 4;
        hidden->sourceId = 5;
        moved->sourceId = 6;
        d->pages[1]->items = {shown, hidden, moved};
        d->pages[0]->items = {morphShape("A", "rect", QRectF(100, 100, 100, 100)), morphShape("B", "rect", QRectF(400, 100, 100, 100)), morphShape("C", "ellipse", QRectF(100, 400, 100, 100))};
        deck::States states;
        deck::TargetState gone;
        gone.visible = false;
        states.insert(deck::Target{5, -1, -1}, gone);
        deck::TargetState shifted;
        shifted.offset = QPointF(300, 0);
        states.insert(deck::Target{6, -1, -1}, shifted);
        MorphFrames morph(d.get(), 0, 1, QStringLiteral("objects"), size, 1.0, Qt::black, nullptr, &states);
        QCOMPARE(morph.plan().pairs.size(), 2);   // B, the new slide's hidden one, has none
        QCOMPARE(morph.plan().leaving.size(), 1);
        QVERIFY(morphColorIs(morph.frame(1), 450, 150, QColor(Qt::white)));
        QVERIFY(morphColorIs(morph.frame(1), 450, 450, QColor(0xC0, 0x20, 0x20)));   // the circle ended 300 to the right
        QVERIFY(morphColorIs(morph.frame(1), 150, 450, QColor(Qt::white)));
        // More objects than a morph holds: a plain fade, and quickly.
        auto big = morphDeck(2);
        for (int i = 0; i < kMorphMaxObjects + 1; ++i) {
            big->pages[0]->items.push_back(morphShape(QStringLiteral("S%1").arg(i), "rect", QRectF(i % 30 * 30, i / 30 * 15, 10, 10), QColor(255, 0, 0)));
            big->pages[1]->items.push_back(morphShape(QStringLiteral("S%1").arg(i), "rect", QRectF(i % 30 * 30, i / 30 * 15, 10, 10), QColor(0, 0, 255)));
        }
        const MorphWork hostileBefore = morphWorkForTest();
        auto hostile = std::make_unique<MorphFrames>(big.get(), 0, 1, QStringLiteral("objects"), size, 1.0);
        QVERIFY(!hostile->isUsable());
        QVERIFY(hostile->plan().pairs.isEmpty());
        const QImage faded = hostile->frame(0.5);
        QVERIFY(morphColorIs(faded, 5, 5, QColor(128, 0, 128), 6));
        // Counted: a slide with more objects than a morph holds is not planned (the pairing of a thousand and more is not made).
        QCOMPARE(morphWorkForTest().objectsPlanned, hostileBefore.objectsPlanned);
        // Slides that aren't there make no frame.
        MorphFrames none(d.get(), 0, 9, QStringLiteral("objects"), size, 1.0);
        QVERIFY(!none.isUsable());
        QVERIFY(none.frame(0.5).isNull());
    }

    // The show plays a morph between two slides and lands on the second.
    void showMorphPlays()
    {
        auto d = showDeck(3);
        deck::Transition t = showTransition("morph", 0.4);
        deck::setTransition(*d->pages[1], t);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(640, 360);
        show.show();
        show.next();
        QVERIFY(show.isAnimating());
        QCOMPARE(show.currentSlide(), 0);
        QCOMPARE(show.targetSlide(), 1);
        const QImage during = show.grab().toImage();   // a frame is painted
        QVERIFY(!during.isNull());
        QTRY_VERIFY(!show.isAnimating());
        QCOMPARE(show.currentSlide(), 1);
        // The transition's own settings play it to the end at once too.
        t.by = QStringLiteral("words");
        deck::setTransition(*d->pages[2], t);
        show.next();
        QVERIFY(show.isAnimating());
        show.finishTransition();
        QCOMPARE(show.currentSlide(), 2);
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
};

JP_TEST_CLASS(DeckTransitionTests)
#include "tst_deck_transitions.moc"
