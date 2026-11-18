// JeffDeck's tests of animations: reading, painting, editing, and writing them.
#include "deckhelpers.h"
#include "app/theme.h"
#include "core/effectmask.h"

class DeckAnimationTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // The hooks on painting a page (the show's animations) change nothing
    // while they are unset or only pass the drawing on; set, they are called
    // for the page's own objects (not the master's, and not a group's
    // children) and for the text of each object that has some.
    void rendererHooksChangeNothingUnset()
    {
        auto d = showDeck(2);
        Page &slide = *d->pages[0];
        auto inner = std::make_shared<ShapeItem>();
        inner->rect = QRectF(300, 380, 120, 80);
        inner->fill = Fill::solid(ColorRef::rgb(QColor(200, 60, 40)));
        auto group = std::make_shared<GroupItem>();
        group->children.push_back(inner);
        group->syncRect();
        slide.items.push_back(group);
        LayoutCache cache;
        const QImage plain = paintedSlide(d.get(), &cache, 0);
        QVERIFY(plain != paintedSlide(d.get(), &cache, 1));
        // Passed on, the page is the same picture.
        int arounds = 0, texts = 0;
        const QImage passed = paintedSlide(d.get(), &cache, 0, [&](PaintContext &ctx) {
            ctx.around = [&](QPainter *p, const Item &, const std::function<void(QPainter *)> &draw) {
                ++arounds;
                draw(p);
            };
            ctx.aroundText = [&](QPainter *p, const Item &, const StoryLayout &, int, const std::function<void(QPainter *)> &paintText) {
                ++texts;
                paintText(p);
            };
        });
        QCOMPARE(passed, plain);
        QCOMPARE(arounds, int(slide.items.size()));
        QVERIFY(texts >= 2);   // the title and the body; the group's shape has no text
        // Not passed on, the object is not drawn.
        const QImage without = paintedSlide(d.get(), &cache, 0, [&](PaintContext &ctx) {
            ctx.around = [&](QPainter *p, const Item &it, const std::function<void(QPainter *)> &draw) {
                if (&it != group.get()) draw(p);
            };
        });
        QVERIFY(without != plain);
        slide.items.pop_back();
        QCOMPARE(without, paintedSlide(d.get(), &cache, 0));
    }

    // One attribute animated from a value or formula to another.
    static QString animValues(int spid, const char *attribute, const char *from, const char *to, int ms)
    {
        return animXml("<p:anim calcmode='lin' valueType='num'><p:cBhvr additive='base'><p:cTn id='%1' dur='%2' fill='hold'/>%3<p:attrNameLst><p:attrName>%4</p:attrName></p:attrNameLst></p:cBhvr><p:tavLst><p:tav tm='0'><p:val><p:strVal val='%5'/></p:val></p:tav><p:tav tm='100000'><p:val><p:strVal val='%6'/></p:val></p:tav></p:tavLst></p:anim>")
            .arg(animId()).arg(ms).arg(animTarget(spid), QString::fromLatin1(attribute), QString::fromLatin1(from), QString::fromLatin1(to));
    }

    // Fly in from the left: x runs from half the shape's width off the slide's edge to where it is.
    static QString animFlyIn(int spid, int ms)
    {
        return animNode("entr", 2, 8, "clickEffect", 0,
                        {animShow(spid, true), animValues(spid, "ppt_x", "0-#ppt_w/2", "#ppt_x", ms), animValues(spid, "ppt_y", "#ppt_y", "#ppt_y", ms)});
    }

    static QString animFadeOut(int spid, int ms, const char *nodeType = "clickEffect", int delay = 0)
    {
        return animNode("exit", 10, 0, nodeType, delay, {animFilter(spid, "fade", false, ms), animShow(spid, false, ms - 1)});
    }

    static QString animWipe(int spid, const char *direction, bool entering, int ms)
    {
        return animNode(entering ? "entr" : "exit", 22, 8, "clickEffect", 0,
                        entering ? QStringList{animShow(spid, true), animFilter(spid, direction, true, ms)}
                                 : QStringList{animFilter(spid, direction, false, ms), animShow(spid, false, ms - 1)});
    }

    static QString animSpin(int spid, int ms, int degrees)
    {
        return animNode("emph", 8, 0, "clickEffect", 0,
                        {animXml("<p:animRot by='%1'><p:cBhvr><p:cTn id='%2' dur='%3' fill='hold'/>%4<p:attrNameLst><p:attrName>r</p:attrName></p:attrNameLst></p:cBhvr></p:animRot>")
                             .arg(degrees * 60000).arg(animId()).arg(ms).arg(animTarget(spid))});
    }

    static QString animGrow(int spid, int ms, int percent)
    {
        return animNode("emph", 6, 0, "clickEffect", 0,
                        {animXml("<p:animScale><p:cBhvr><p:cTn id='%1' dur='%2' fill='hold'/>%3</p:cBhvr><p:by x='%4' y='%4'/></p:animScale>")
                             .arg(animId()).arg(ms).arg(animTarget(spid)).arg(percent * 1000)});
    }

    static QString animPath(int spid, int ms, const char *path)
    {
        return animNode("path", 0, 0, "clickEffect", 0,
                        {animXml("<p:animMotion origin='layout' path='%1' pathEditMode='relative' rAng='0' ptsTypes='AA'><p:cBhvr><p:cTn id='%2' dur='%3' fill='hold'/>%4<p:attrNameLst><p:attrName>ppt_x</p:attrName><p:attrName>ppt_y</p:attrName></p:attrNameLst></p:cBhvr></p:animMotion>")
                             .arg(QString::fromLatin1(path)).arg(animId()).arg(ms).arg(animTarget(spid))});
    }

    // Play a movie: a step that shows nothing.
    static QString animMediaCall(int spid)
    {
        return animNode("mediacall", 1, 0, "clickEffect", 0,
                        {animXml("<p:cmd type='call' cmd='playFrom(0.0)'><p:cBhvr><p:cTn id='%1' dur='1' fill='hold'/>%2</p:cBhvr></p:cmd>").arg(animId()).arg(animTarget(spid))});
    }

    // Times a file writes as "inf" or "nan" are not times: they read as if they were not written, so nothing begins at the end of time and no
    // number the show works with is infinite.
    void animationIgnoresTimesThatAreNotNumbers()
    {
        QString xml = animTiming(animStep({animGroup(0, {animFadeIn(4, 500), animFlyIn(5, 400)})}));
        xml.replace(QStringLiteral("delay=\"0\""), QStringLiteral("delay=\"inf\""));
        xml.replace(QStringLiteral("dur=\"500\""), QStringLiteral("dur=\"inf\""));
        xml.replace(QStringLiteral("dur=\"400\""), QStringLiteral("dur=\"nan\""));
        xml.replace(QStringLiteral("fill=\"hold\""), QStringLiteral("fill=\"hold\" repeatCount=\"inf\" accel=\"nan\" decel=\"inf\" repeatDur=\"inf\""));
        QVERIFY(xml.contains(QStringLiteral("delay=\"inf\"")) && xml.contains(QStringLiteral("dur=\"nan\"")));
        const deck::Timeline tl = deck::readTimeline(xml);
        QVERIFY(!tl.steps.isEmpty());
        int seen = 0;
        for (const deck::Step &step : tl.steps) {
            QVERIFY(std::isfinite(step.duration()));
            for (const deck::Effect &e : step.effects) {
                QVERIFY2(std::isfinite(e.begin) && std::isfinite(e.repeat) && std::isfinite(e.end()), qPrintable(QString::number(e.begin)));
                for (const deck::Behavior &b : e.behaviors) {
                    const deck::Timing &t = b.timing;
                    QVERIFY2(std::isfinite(t.begin) && std::isfinite(t.duration) && std::isfinite(t.accel) && std::isfinite(t.decel) && std::isfinite(t.repeat) && std::isfinite(t.repeatDuration),
                             qPrintable(QStringLiteral("%1 %2 %3 %4 %5 %6").arg(t.begin).arg(t.duration).arg(t.accel).arg(t.decel).arg(t.repeat).arg(t.repeatDuration)));
                    ++seen;
                }
            }
        }
        QVERIFY(seen > 0);
        // Keyframes at a time that is not a number are not kept.
        const QString key = animXml("<p:anim calcmode='lin' valueType='num'><p:cBhvr><p:cTn id='%1' dur='500' fill='hold'/>%2<p:attrNameLst><p:attrName>ppt_x</p:attrName></p:attrNameLst></p:cBhvr>"
                                    "<p:tavLst><p:tav tm='nan'><p:val><p:fltVal val='inf'/></p:val></p:tav><p:tav tm='50000'><p:val><p:fltVal val='1'/></p:val></p:tav></p:tavLst></p:anim>")
                                .arg(animId()).arg(animTarget(4));
        const deck::Timeline withKeys = deck::readTimeline(animTiming(animStep({animGroup(0, {animNode("emph", 1, 0, "clickEffect", 0, {key})})})));
        int keys = 0;
        for (const deck::Step &step : withKeys.steps)
            for (const deck::Effect &e : step.effects)
                for (const deck::Behavior &b : e.behaviors)
                    for (const deck::Keyframe &k : b.keys) {
                        QVERIFY(std::isfinite(k.at));
                        ++keys;
                    }
        QVERIFY(keys >= 1);
    }

    // The steps of a slide's timing, their effects, and the behaviors of each,
    // with their times: a click starts a step, an effect that goes with the
    // one before starts with it (its delay on top), and one after starts when
    // the group before ends.
    void animationReadsClickSteps()
    {
        const deck::Timeline tl = deck::readTimeline(animTiming(
            animStep({animGroup(0, {animFadeIn(4, 500)})}) +
            animStep({animGroup(0, {animFlyIn(4, 400), animFadeIn(5, 300, "withEffect", 200)}), animGroup(700, {animFadeOut(6, 500, "afterEffect", 100)})})));
        QCOMPARE(int(tl.steps.size()), 2);
        QVERIFY(!tl.steps[0].automatic);
        QCOMPARE(int(tl.steps[0].effects.size()), 1);
        const deck::Effect &fade = tl.steps[0].effects[0];
        QVERIFY(fade.cls == deck::EffectClass::Entrance);
        QVERIFY(fade.start == deck::EffectStart::OnClick);
        QCOMPARE(fade.presetId, 10);
        QCOMPARE(fade.begin, 0.0);
        QCOMPARE(int(fade.behaviors.size()), 2);
        QVERIFY(fade.behaviors[0].kind == deck::Behavior::Kind::Set);
        QCOMPARE(fade.behaviors[0].attribute, QStringLiteral("style.visibility"));
        QCOMPARE(fade.behaviors[0].to, QStringLiteral("visible"));
        QCOMPARE(fade.behaviors[0].target.shape, 4);
        QVERIFY(fade.behaviors[0].timing.hold);
        QVERIFY(fade.behaviors[1].kind == deck::Behavior::Kind::Effect);
        QCOMPARE(fade.behaviors[1].filter, QStringLiteral("fade"));
        QVERIFY(fade.behaviors[1].entering);
        QCOMPARE(fade.behaviors[1].timing.duration, 500.0);
        QCOMPARE(fade.end(), 500.0);
        QCOMPARE(tl.steps[0].duration(), 500.0);
        QVERIFY(tl.steps[0].visual());
        // The second click: a fly in, a fade with it 200 ms later, and an exit after both, 100 ms after the group's start.
        const deck::Step &second = tl.steps[1];
        QCOMPARE(int(second.effects.size()), 3);
        QVERIFY(second.effects[0].start == deck::EffectStart::OnClick);
        QVERIFY(second.effects[1].start == deck::EffectStart::WithPrevious);
        QCOMPARE(second.effects[1].begin, 200.0);
        QVERIFY(second.effects[2].start == deck::EffectStart::AfterPrevious);
        QVERIFY(second.effects[2].cls == deck::EffectClass::Exit);
        QCOMPARE(second.effects[2].begin, 800.0);
        QCOMPARE(second.effects[2].end(), 1300.0);
        QCOMPARE(second.duration(), 1300.0);
        // The fly in is a set and a p:anim for x and for y; x runs from a formula to a formula.
        QCOMPARE(int(second.effects[0].behaviors.size()), 3);
        const deck::Behavior &x = second.effects[0].behaviors[1];
        QVERIFY(x.kind == deck::Behavior::Kind::Animate);
        QCOMPARE(x.attribute, QStringLiteral("ppt_x"));
        QCOMPARE(int(x.keys.size()), 2);
        QCOMPARE(x.keys[0].at, 0.0);
        QCOMPARE(x.keys[0].value, QStringLiteral("0-#ppt_w/2"));
        QCOMPARE(x.keys[1].at, 1.0);
        QCOMPARE(x.keys[1].value, QStringLiteral("#ppt_x"));
        QVERIFY(x.timing.hold);
        // A step that starts when the slide is shown says so; a slide without timing, or with a broken one, has no steps.
        QVERIFY(deck::readTimeline(animTiming(animStep({animGroup(0, {animAppear(4, "afterEffect")})}, true))).steps[0].automatic);
        // Even when the file leaves out the main sequence's start, if the first effect goes with or after one before it.
        QVERIFY(deck::readTimeline(animTiming(animStep({animGroup(0, {animAppear(4, "withEffect")})}))).steps[0].automatic);
        QVERIFY(!deck::readTimeline(animTiming(animStep({animGroup(0, {animAppear(4)})}))).steps[0].automatic);
        QVERIFY(!deck::readTimeline(animTiming(animSteps({animAppear(4), animAppear(5, "withEffect")}))).steps[1].automatic);
        QVERIFY(deck::readTimeline(QString()).isEmpty());
        QVERIFY(deck::readTimeline(QStringLiteral("<p:timing")).isEmpty());
        Page none;
        QVERIFY(deck::timelineOf(none).isEmpty());
        none.props[QStringLiteral("timingXml")] = animTiming(animSteps({animAppear(4)}));
        QCOMPARE(int(deck::timelineOf(none).steps.size()), 1);
    }

    // An entrance starts hidden; at its click it is shown and fades in with
    // the time, and played it stays.
    void animationFadeStates()
    {
        const deck::Timeline tl = deck::readTimeline(animTiming(animSteps({animFadeIn(4, 500)})));
        const deck::Geometry g = animGeometry();
        QVERIFY(!deck::statesAt(tl, g, 0).value(deck::Target{4}).visible);
        auto at = [&](double ms) { return deck::statesAt(tl, g, 0, ms).value(deck::Target{4}); };
        QVERIFY(at(0).visible);
        QCOMPARE(at(0).opacity, 0.0);
        QVERIFY(animNear(at(125).opacity, 0.25));
        QVERIFY(animNear(at(250).opacity, 0.5));
        QCOMPARE(at(500).opacity, 1.0);
        QCOMPARE(at(90000).opacity, 1.0);
        const deck::TargetState done = deck::statesAt(tl, g, 1).value(deck::Target{4});
        QVERIFY(done.visible);
        QCOMPARE(done.opacity, 1.0);
        QCOMPARE(done.offset, QPointF());
        QCOMPARE(done.scaleX, 1.0);
        QVERIFY(done.clip.isNull());
        QVERIFY(!deck::statesAt(tl, g, 0).contains(deck::Target{5}));
    }

    // Fly in from the left: x runs from -#ppt_w/2 (all of it off the slide) to
    // #ppt_x, so the shape is moved by the difference.
    void animationFlyInMoves()
    {
        const deck::Timeline tl = deck::readTimeline(animTiming(animSteps({animFlyIn(4, 500)})));
        const deck::Geometry g = animGeometry();
        auto at = [&](double ms) { return deck::statesAt(tl, g, 0, ms).value(deck::Target{4}); };
        // The middle starts at -100 points (half the width off the edge), 300 left of where it is.
        QVERIFY(animNear(at(0).offset.x(), -300));
        QVERIFY(animNear(at(250).offset.x(), -150));
        QVERIFY(animNear(at(500).offset.x(), 0));
        QCOMPARE(at(0).offset.y(), 0.0);
        QVERIFY(animNear(at(250).offset.y(), 0));
        QVERIFY(at(0).visible);
        QVERIFY(animNear(deck::statesAt(tl, g, 1).value(deck::Target{4}).offset.x(), 0));
    }

    // Effects with the one before and after it keep their own times, an exit
    // starts shown and ends hidden.
    void animationWithAndAfterPreviousAndExit()
    {
        const deck::Timeline tl = deck::readTimeline(animTiming(animStep(
            {animGroup(0, {animFlyIn(4, 400), animFadeIn(5, 300, "withEffect", 200)}), animGroup(700, {animFadeOut(6, 500, "afterEffect", 100)})})));
        const deck::Geometry g = animGeometry();
        auto at = [&](int shape, double ms) { return deck::statesAt(tl, g, 0, ms).value(deck::Target{shape}); };
        QVERIFY(deck::statesAt(tl, g, 0).value(deck::Target{6}).visible);   // an exit's target is there at first
        QVERIFY(!deck::statesAt(tl, g, 0).value(deck::Target{5}).visible);
        QVERIFY(at(4, 100).visible);
        QVERIFY(!at(5, 100).visible);                    // it waits 200 ms
        QVERIFY(at(5, 200).visible);
        QCOMPARE(at(5, 200).opacity, 0.0);
        QVERIFY(animNear(at(5, 350).opacity, 0.5));
        QVERIFY(at(6, 799).visible);
        QCOMPARE(at(6, 799).opacity, 1.0);               // the group at 700 and the effect's 100 ms: it starts at 800
        QVERIFY(animNear(at(6, 1050).opacity, 0.5));
        QVERIFY(at(6, 1298).visible);
        QVERIFY(!at(6, 1299).visible);                   // hidden when the fade is done
        const deck::States end = deck::statesAt(tl, g, 1);
        QVERIFY(end.value(deck::Target{4}).visible);
        QVERIFY(end.value(deck::Target{5}).visible);
        QCOMPARE(end.value(deck::Target{5}).opacity, 1.0);
        QVERIFY(!end.value(deck::Target{6}).visible);
        // An exit that comes after an entrance of the same shape: hidden, shown, then hidden.
        const deck::Timeline both = deck::readTimeline(animTiming(animSteps({animFadeIn(4, 100), animFadeOut(4, 100)})));
        QVERIFY(!deck::statesAt(both, g, 0).value(deck::Target{4}).visible);
        QVERIFY(deck::statesAt(both, g, 1).value(deck::Target{4}).visible);
        QVERIFY(!deck::statesAt(both, g, 2).value(deck::Target{4}).visible);
    }

    // A spin turns the shape about its middle, by the angle the file gives in
    // 60,000ths of a degree, and keeps the turn; a scale grows it about the middle.
    void animationSpinRotatesAndGrowScales()
    {
        const deck::Timeline tl = deck::readTimeline(animTiming(animSteps({animSpin(4, 2000, 360)})));
        const deck::Geometry g = animGeometry();
        QVERIFY(deck::statesAt(tl, g, 0).value(deck::Target{4}).visible);   // an emphasis starts in place
        QVERIFY(animNear(deck::statesAt(tl, g, 0, 0).value(deck::Target{4}).rotation, 0));
        QVERIFY(animNear(deck::statesAt(tl, g, 0, 500).value(deck::Target{4}).rotation, 90));
        QVERIFY(animNear(deck::statesAt(tl, g, 0, 1500).value(deck::Target{4}).rotation, 270));
        QVERIFY(animNear(deck::statesAt(tl, g, 1).value(deck::Target{4}).rotation, 360));
        const deck::Timeline grow = deck::readTimeline(animTiming(animSteps({animGrow(4, 1000, 150)})));
        QVERIFY(animNear(deck::statesAt(grow, g, 0, 500).value(deck::Target{4}).scaleX, 1.25));
        QVERIFY(animNear(deck::statesAt(grow, g, 0, 500).value(deck::Target{4}).scaleY, 1.25));
        QVERIFY(animNear(deck::statesAt(grow, g, 1).value(deck::Target{4}).scaleX, 1.5));
    }

    // A motion path is in fractions of the slide from where the shape is, and
    // the shape moves along it at an even speed.
    void animationMotionPath()
    {
        const deck::Geometry g = animGeometry();
        const deck::Timeline line = deck::readTimeline(animTiming(animSteps({animPath(4, 2000, "M 0 0 L 0.25 0.1 E")})));
        auto at = [&](const deck::Timeline &tl, double ms) { return deck::statesAt(tl, g, 0, ms).value(deck::Target{4}).offset; };
        QVERIFY(animNear(at(line, 1000).x(), 120));   // 0.125 of 960
        QVERIFY(animNear(at(line, 1000).y(), 27));    // 0.05 of 540
        QVERIFY(animNear(deck::statesAt(line, g, 1).value(deck::Target{4}).offset.x(), 240));
        QVERIFY(animNear(deck::statesAt(line, g, 1).value(deck::Target{4}).offset.y(), 54));
        QVERIFY(deck::statesAt(line, g, 0).value(deck::Target{4}).visible);
        const deck::Behavior &motion = line.steps[0].effects[0].behaviors[0];
        QVERIFY(motion.kind == deck::Behavior::Kind::Motion);
        QCOMPARE(motion.origin, QStringLiteral("layout"));
        QCOMPARE(motion.pathEditMode, QStringLiteral("relative"));
        QCOMPARE(motion.path.elementCount(), 2);
        // Along two lines, 96 and 108 points long: halfway is 6 points into the second.
        const deck::Timeline bend = deck::readTimeline(animTiming(animSteps({animPath(4, 2000, "M 0 0 L 0.1 0 L 0.1 0.2 E")})));
        QVERIFY(animNear(at(bend, 1000).x(), 96));
        QVERIFY(animNear(at(bend, 1000).y(), 6));
        // A curve: the path ends where it is told to.
        const deck::Timeline curve = deck::readTimeline(animTiming(animSteps({animPath(4, 2000, "M 0 0 C 0.1 0 0.2 0.1 0.25 0.1 E")})));
        QVERIFY(animNear(deck::statesAt(curve, g, 1).value(deck::Target{4}).offset.x(), 240));
        QVERIFY(at(curve, 1000).x() > 0 && at(curve, 1000).x() < 240);
    }

    // A build by paragraph aims each effect at a range of the text box's
    // paragraphs: they start hidden and come one click at a time, and the box
    // itself, which no effect aims at, is not in the states.
    void animationBuildsByParagraph()
    {
        const deck::Timeline tl = deck::readTimeline(animTiming(animSteps({animFadeIn(3, 300, "clickEffect", 0, 0), animFadeIn(3, 300, "clickEffect", 0, 1), animAppear(5)})));
        QCOMPARE(int(tl.steps.size()), 3);
        const deck::Target first{3, 0, 0}, second{3, 1, 1}, whole{3, -1, -1};
        QVERIFY(tl.steps[1].effects[0].behaviors[0].target == second);
        QVERIFY(second.paragraphs() && !whole.paragraphs());
        const deck::Geometry g = animGeometry();
        const deck::States start = deck::statesAt(tl, g, 0);
        QVERIFY(!start.value(first).visible);
        QVERIFY(!start.value(second).visible);
        QVERIFY(!start.contains(whole));
        const deck::States one = deck::statesAt(tl, g, 1);
        QVERIFY(one.value(first).visible);
        QVERIFY(!one.value(second).visible);
        const deck::States halfway = deck::statesAt(tl, g, 1, 150);
        QVERIFY(halfway.value(second).visible);
        QVERIFY(animNear(halfway.value(second).opacity, 0.5));
        QCOMPARE(halfway.value(first).opacity, 1.0);
        QVERIFY(deck::statesAt(tl, g, 3).value(deck::Target{5}).visible);
    }

    // The common effects uncover a shape with these filters; the part shown is a path over the unit square.
    void animationCommonMasksAreShapes()
    {
        auto at = [](const char *filter, double p, bool entering, double x, double y) {
            QPainterPath shown;
            return deck::effectMask(QString::fromLatin1(filter), p, entering, &shown) && shown.contains(QPointF(x, y));
        };
        QPainterPath none;
        QVERIFY(!deck::effectMask(QStringLiteral("fade"), 0.5, true, &none));
        QVERIFY(!deck::effectMask(QStringLiteral("wipe(left)"), 0.5, true, &none));
        // Blinds: each of the six slats is open for the first half of its height.
        QVERIFY(at("blinds(horizontal)", 0.5, true, 0.5, 0.02));
        QVERIFY(!at("blinds(horizontal)", 0.5, true, 0.5, 0.14));
        QVERIFY(at("blinds(horizontal)", 0.5, true, 0.5, 0.52));
        QVERIFY(at("blinds(vertical)", 0.5, true, 0.02, 0.5));
        QVERIFY(!at("blinds(vertical)", 0.5, true, 0.14, 0.5));
        // Checkerboard: one color of squares is in at half way, the other not yet; across fills each square from its left.
        QVERIFY(at("checkerboard(across)", 0.25, true, 0.03, 0.0625));   // first square, half filled
        QVERIFY(!at("checkerboard(across)", 0.25, true, 0.1, 0.0625));
        QVERIFY(!at("checkerboard(across)", 0.25, true, 0.19, 0.0625));    // the other color waits
        QVERIFY(at("checkerboard(across)", 0.5, true, 0.12, 0.0625));      // the first color is in
        QVERIFY(!at("checkerboard(across)", 0.5, true, 0.19, 0.0625));
        QVERIFY(at("checkerboard(down)", 0.25, true, 0.0625, 0.03));
        QVERIFY(!at("checkerboard(down)", 0.25, true, 0.0625, 0.1));
        // Random bars: more of them as it goes, all of them at the end, none at the start.
        auto area = [](const char *filter, double p) {
            QPainterPath shown;
            return deck::effectMask(QString::fromLatin1(filter), p, true, &shown) ? shown.simplified().boundingRect().width() * shown.simplified().boundingRect().height() : -1.0;
        };
        QCOMPARE(area("randombar(horizontal)", 0.0), 0.0);
        QVERIFY(area("randombar(horizontal)", 1.0) > 0.99);
        int slicesIn = 0;
        for (int i = 0; i < 20; ++i)
            if (at("randombar(horizontal)", 0.5, true, 0.5, (i + 0.5) / 20)) ++slicesIn;
        QVERIFY2(slicesIn > 3 && slicesIn < 17, qPrintable(QString::number(slicesIn)));
        int fewer = 0;
        for (int i = 0; i < 20; ++i)
            if (at("randombar(horizontal)", 0.25, true, 0.5, (i + 0.5) / 20) && !at("randombar(horizontal)", 0.75, true, 0.5, (i + 0.5) / 20)) ++fewer;
        QCOMPARE(fewer, 0);   // a bar that is in stays in
        // Box, circle, diamond, plus: out opens from the middle, in closes from the edges.
        for (const char *form : {"box", "circle", "diamond", "plus"}) {
            const QByteArray out = QByteArray(form) + "(out)", in = QByteArray(form) + "(in)";
            QVERIFY2(at(out, 0.5, true, 0.5, 0.5) && !at(out, 0.5, true, 0.02, 0.02), form);
            QVERIFY2(!at(in, 0.5, true, 0.5, 0.5) && at(in, 0.5, true, 0.02, 0.02), form);
            QVERIFY2(!at(out, 0.0, true, 0.5, 0.5) && !at(in, 0.0, true, 0.02, 0.02), form);
            QVERIFY2(at(out, 1.0, true, 0.02, 0.02) && at(in, 1.0, true, 0.5, 0.5), form);
            // An exit shows what an entrance has not.
            QVERIFY2(!at(out, 0.5, false, 0.5, 0.5) && at(out, 0.5, false, 0.02, 0.02), form);
        }
        QVERIFY(at("plus(out)", 0.2, true, 0.5, 0.1) && at("plus(out)", 0.2, true, 0.1, 0.5) && !at("plus(out)", 0.2, true, 0.1, 0.1));
        QVERIFY(at("diamond(out)", 0.25, true, 0.6, 0.55) && !at("diamond(out)", 0.25, true, 0.7, 0.7));
        QVERIFY(at("box(out)", 0.5, true, 0.7, 0.7) && !at("box(out)", 0.5, true, 0.8, 0.5));
        // Wheel: spokes sweep clockwise from the top.
        QVERIFY(at("wheel(1)", 0.5, true, 0.8, 0.5) && !at("wheel(1)", 0.5, true, 0.2, 0.5));   // the right half is in
        QVERIFY(at("wheel(4)", 0.5, true, 0.6, 0.3) && !at("wheel(4)", 0.5, true, 0.9, 0.4));   // half of each quarter
        QVERIFY(at("wheel(8)", 1.0, true, 0.1, 0.9) && at("wheel(8)", 1.0, true, 0.9, 0.1));
        // Dissolve: cells come in for good as it goes.
        int in25 = 0, in75 = 0, lost = 0;
        for (int j = 0; j < 20; ++j)
            for (int i = 0; i < 20; ++i) {
                const double x = (i + 0.5) / 20, y = (j + 0.5) / 20;
                const bool a = at("dissolve", 0.25, true, x, y), b = at("dissolve", 0.75, true, x, y);
                in25 += a;
                in75 += b;
                lost += a && !b;
            }
        QCOMPARE(lost, 0);
        QVERIFY2(in25 > 60 && in25 < 140 && in75 > 260 && in75 < 340, qPrintable(QStringLiteral("%1 %2").arg(in25).arg(in75)));
    }

    // Every effect of the common kinds, as the show plays it: nothing at its start, a part of the shape halfway, all of it at its end
    // (an entrance), and the other way round for an exit.
    void animationCommonEffectsPlayAsFrames()
    {
        auto d = editDeck();
        Page &slide = *d->pages[0];
        const deck::Geometry g = deck::geometryOf(*d, slide);
        const QStringList masked = {"blinds", "box", "checkerboard", "circle", "diamond", "plus", "dissolve", "randombars", "wheel"};
        const QStringList moved = {"peek", "stretch", "swivel", "expand", "spiral", "bounce"};
        int played = 0;
        for (const QString &id : masked + moved + QStringList{"flash"})
            for (const char *cls : {"entr.", "exit."}) {
                const deck::EffectKind *kind = deck::effectKind(QLatin1String(cls) + id);
                if (!kind) {
                    QVERIFY2(id == QLatin1String("flash") && cls == QLatin1String("exit."), qPrintable(QLatin1String(cls) + id));
                    continue;
                }
                const bool entering = kind->cls == deck::EffectClass::Entrance;
                for (const deck::EffectVariant &v : kind->variants) {
                    deck::AnimSequence seq;
                    deck::Anim a = deck::makeAnim(*kind, 3);
                    a.variant = v.key;
                    a.duration = 1000;
                    seq.add(a);
                    deck::setAnimations(*d, slide, seq);
                    const deck::Timeline tl = deck::readTimeline(slide.props.value(QStringLiteral("timingXml")).toString());
                    QVERIFY(!tl.isEmpty());
                    const QString where = kind->id + QLatin1Char(' ') + v.key;
                    const deck::Target t{3};
                    const deck::TargetState before = deck::statesAt(tl, g, 0).value(t), mid = deck::statesAt(tl, g, 0, 500).value(t), end = deck::statesAt(tl, g, 1).value(t);
                    if (id == QLatin1String("flash")) {
                        QVERIFY2(!before.visible, qPrintable(where));
                        QVERIFY2(mid.visible && !end.visible, qPrintable(where));   // it shows, and goes
                        ++played;
                        continue;
                    }
                    QVERIFY2(mid != before && mid != end, qPrintable(where + " halfway is the start or the end"));
                    if (masked.contains(id)) {
                        QVERIFY2(mid.masked && mid.visible, qPrintable(where));
                        QVERIFY2(!mid.mask.isEmpty() && !mid.mask.contains(QPointF(-1, -1)), qPrintable(where));
                    }
                    if (entering) {
                        QVERIFY2(end.visible && !end.masked && end.opacity == 1 && end.scaleX == 1 && end.scaleY == 1 && end.offset.isNull(), qPrintable(where + " does not end whole"));
                        QVERIFY2(!before.visible || before.masked || before.scaleX == 0 || before.scaleY == 0 || !before.offset.isNull(), qPrintable(where + " starts whole"));
                    } else {
                        QVERIFY2(!end.visible, qPrintable(where + " is still there"));
                    }
                    ++played;
                }
            }
        QVERIFY2(played >= 60, qPrintable(QString::number(played)));
    }

    // The mask a filter makes is what the timeline's state holds, halfway: the show, the gallery pictures, and the files all agree.
    void animationCommonEffectStateHoldsTheMask()
    {
        auto d = editDeck();
        Page &slide = *d->pages[0];
        const deck::Geometry g = deck::geometryOf(*d, slide);
        struct Case { const char *kind, *variant, *filter; bool entering; };
        for (const Case &c : {Case{"entr.blinds", "vertical", "blinds(vertical)", true}, Case{"entr.circle", "in", "circle(in)", true}, Case{"exit.checkerboard", "down", "checkerboard(down)", false},
                              Case{"entr.wheel", "s4", "wheel(4)", true}, Case{"exit.box", "out", "box(out)", false}, Case{"entr.peek", "bottom", "wipe(down)", true}}) {
            deck::AnimSequence seq;
            deck::Anim a = deck::makeAnim(QLatin1String(c.kind), 3);
            a.variant = QLatin1String(c.variant);
            a.duration = 1000;
            seq.add(a);
            deck::setAnimations(*d, slide, seq);
            QVERIFY2(slide.props.value(QStringLiteral("timingXml")).toString().contains(QLatin1String(c.filter)), c.filter);
            const deck::Timeline tl = deck::readTimeline(slide.props.value(QStringLiteral("timingXml")).toString());
            const deck::TargetState mid = deck::statesAt(tl, g, 0, 250).value(deck::Target{3});
            QPainterPath want;
            if (deck::effectMask(QLatin1String(c.filter), 0.25, c.entering, &want)) {
                QVERIFY2(mid.masked && mid.mask == want, c.filter);
            } else {
                QVERIFY2(!mid.masked && !mid.clip.isNull(), c.filter);   // a peek is a wipe's clip
            }
        }
    }

    // The common effects go through a saved file as they are: the preset numbers and ways (presetID, presetSubtype), the filter, and the
    // amount of a transparency, and read back as the same kinds and ways.
    void animationCommonEffectsSurviveAPptxFile()
    {
        auto d = editDeck();
        Page &slide = *d->pages[0];
        const QStringList ids = {"entr.blinds", "entr.box", "entr.checkerboard", "entr.circle", "entr.diamond", "entr.plus", "entr.dissolve", "entr.randombars", "entr.wheel",
                                 "entr.peek", "entr.flash", "entr.stretch", "entr.swivel", "entr.expand", "entr.spiral", "entr.bounce", "exit.blinds", "exit.box", "exit.checkerboard",
                                 "exit.circle", "exit.diamond", "exit.plus", "exit.dissolve", "exit.randombars", "exit.wheel", "exit.peek", "exit.stretch", "exit.swivel", "exit.expand",
                                 "exit.spiral", "exit.bounce", "emph.transparency"};
        deck::AnimSequence seq;
        QStringList wanted;
        for (const QString &id : ids) {
            const deck::EffectKind *kind = deck::effectKind(id);
            QVERIFY2(kind, qPrintable(id));
            for (const deck::EffectVariant &v : kind->variants) {
                deck::Anim a = deck::makeAnim(*kind, 3);
                a.variant = v.key;
                if (kind->amount == deck::EffectKind::Amount::Transparency) a.amount = 75;
                seq.add(a);
                wanted << id + QLatin1Char(' ') + v.key;
            }
        }
        deck::setAnimations(*d, slide, seq);
        const QString timing = slide.props.value(QStringLiteral("timingXml")).toString();
        for (const char *piece : {"presetID=\"3\"", "presetID=\"21\" presetClass=\"entr\" presetSubtype=\"8\"", "filter=\"blinds(horizontal)\"", "filter=\"checkerboard(down)\"", "filter=\"randombar(vertical)\"",
                                  "filter=\"box(out)\"", "filter=\"circle(in)\"", "filter=\"diamond(in)\"", "filter=\"plus(out)\"", "filter=\"wheel(8)\"", "filter=\"dissolve\"", "presetID=\"26\" presetClass=\"entr\"",
                                  "presetID=\"9\" presetClass=\"emph\"", "<p:strVal val=\"0.25\"/>"})
            QVERIFY2(timing.contains(QLatin1String(piece)), piece);
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const deck::AnimSequence again = deck::readAnimations(*back->pages[0]);
        QCOMPARE(int(again.effects.size()), int(wanted.size()));
        for (int i = 0; i < again.effects.size(); ++i) {
            const deck::Anim &b = again.effects[i];
            QVERIFY2(!b.custom() && b.kind + QLatin1Char(' ') + b.variant == wanted[i], qPrintable(wanted[i] + " came back as " + b.kind + " " + b.variant));
            if (b.kind == QLatin1String("emph.transparency")) QCOMPARE(b.amount, 75.0);
        }
    }

    // The masks are kept: the same filter, way, and progress (in 256ths) is built once; a few hundred of them at most are held.
    void animationMaskCacheKeepsWhatItBuilt()
    {
        deck::clearEffectMaskCache();
        QPainterPath first, second, near, other;
        QVERIFY(deck::effectMask(QStringLiteral("dissolve"), 0.5, true, &first));
        QCOMPARE(deck::effectMaskBuilds(), 1);
        QCOMPARE(deck::effectMaskCacheHits(), 0);
        QVERIFY(deck::effectMask(QStringLiteral("dissolve"), 0.5, true, &second));
        QCOMPARE(deck::effectMaskBuilds(), 1);   // not built again
        QCOMPARE(deck::effectMaskCacheHits(), 1);
        QCOMPARE(second, first);
        QVERIFY(deck::effectMask(QStringLiteral("dissolve"), 0.5 + 1.0 / 1000, true, &near));   // the same 256th
        QCOMPARE(deck::effectMaskBuilds(), 1);
        QCOMPARE(near, first);
        QVERIFY(deck::effectMask(QStringLiteral("dissolve"), 0.5, false, &other));   // an exit is another mask
        QVERIFY(deck::effectMask(QStringLiteral("dissolve"), 0.6, true, &other));
        QCOMPARE(deck::effectMaskBuilds(), 3);
        // A file's odd filters and every progress fill it to its limit and no further.
        for (int i = 0; i < 1000; ++i) {
            QPainterPath p;
            QVERIFY(deck::effectMask(QStringLiteral("blinds(h%1)").arg(i % 40), i / 1000.0, i % 2 == 0, &p));
        }
        QVERIFY2(deck::effectMaskCacheSize() > 0 && deck::effectMaskCacheSize() <= 128, qPrintable(QString::number(deck::effectMaskCacheSize())));
        // A name that is no filter is not kept, and a long one is refused before it is looked at.
        const int size = deck::effectMaskCacheSize();
        QPainterPath none;
        QVERIFY(!deck::effectMask(QStringLiteral("fade"), 0.5, true, &none));
        QVERIFY(!deck::effectMask(QStringLiteral("wheel(") + QString(5000, QLatin1Char('a')) + QLatin1Char(')'), 0.5, true, &none));
        QCOMPARE(deck::effectMaskCacheSize(), size);
    }

    // A slide of 200 dissolving shapes played for 60 frames builds each mask once per frame, not once per shape.
    void animationMasksAreBuiltOncePerFrame()
    {
        auto d = editDeck(200);
        Page &slide = *d->pages[0];
        deck::AnimSequence seq;
        for (int i = 0; i < 200; ++i) {
            deck::Anim a = deck::makeAnim(QStringLiteral("entr.dissolve"), 10 + i);
            a.duration = 1000;
            a.start = i == 0 ? deck::EffectStart::OnClick : deck::EffectStart::WithPrevious;
            seq.add(a);
        }
        deck::setAnimations(*d, slide, seq);
        const deck::Timeline tl = deck::readTimeline(slide.props.value(QStringLiteral("timingXml")).toString());
        const deck::Geometry g = deck::geometryOf(*d, slide);
        deck::clearEffectMaskCache();
        int masked = 0;
        for (int frame = 0; frame < 60; ++frame) {
            const deck::States states = deck::statesAt(tl, g, 0, frame * 1000.0 / 60);
            for (const deck::TargetState &s : states) masked += s.masked;
        }
        QVERIFY2(masked >= 200 * 59, qPrintable(QString::number(masked)));
        QVERIFY2(deck::effectMaskBuilds() <= 60, qPrintable(QString::number(deck::effectMaskBuilds())));
        QVERIFY2(deck::effectMaskCacheHits() >= 200 * 59 - 60, qPrintable(QString::number(deck::effectMaskCacheHits())));
    }

    // Painted through the show's own path, a checkerboard and a wheel halfway cover some of the shape and leave the rest as the slide has it.
    void animationMasksPaintInTheShow()
    {
        auto d = showDeck(1);
        Page &slide = *d->pages[0];
        numberShowShapes(slide);
        auto box = std::make_shared<ShapeItem>();
        box->rect = QRectF(600, 380, 120, 80);
        box->fill = Fill::solid(ColorRef::rgb(QColor(200, 60, 40)));
        box->sourceId = 7;
        slide.items.push_back(box);
        LayoutCache cache;
        const QColor fill(200, 60, 40), white(Qt::white);
        auto frame = [&](const char *kind, const char *variant, double ms) {
            deck::AnimSequence seq;
            deck::Anim a = deck::makeAnim(QLatin1String(kind), 7);
            a.variant = QLatin1String(variant);
            a.duration = 1000;
            seq.add(a);
            deck::setAnimations(*d, slide, seq);
            const deck::Timeline tl = deck::readTimeline(slide.props.value(QStringLiteral("timingXml")).toString());
            const deck::States states = deck::statesAt(tl, deck::geometryOf(*d, slide), 0, ms);
            return paintedSlide(d.get(), &cache, 0, [&](PaintContext &ctx) { deck::animate(ctx, *d, states); });
        };
        auto at = [&](const QImage &img, double x, double y) { return img.pixelColor(qRound(600 + x * 120), qRound(380 + y * 80)); };
        // Checkerboard across, half way: the squares of one color are in, the others not yet (a square is 15 by 10 points).
        QImage img = frame("entr.checkerboard", "across", 500);
        QCOMPARE(at(img, 0.5 / 8, 0.5 / 8), fill);
        QCOMPARE(at(img, 1.5 / 8, 0.5 / 8), white);
        QCOMPARE(at(img, 1.5 / 8, 1.5 / 8), fill);
        // Wheel of four, half way: half of each quarter, from the top, clockwise.
        img = frame("entr.wheel", "s4", 500);
        QCOMPARE(at(img, 0.6, 0.3), fill);
        QCOMPARE(at(img, 0.9, 0.4), white);
        // An exit shows what an entrance has not: the box opening out from the middle takes it away.
        img = frame("exit.box", "out", 500);
        QCOMPARE(at(img, 0.5, 0.5), white);
        QCOMPARE(at(img, 0.05, 0.05), fill);
        // At the end of an entrance all of it shows.
        img = frame("entr.wheel", "s4", 1000);
        QCOMPARE(at(img, 0.9, 0.4), fill);
        QCOMPARE(at(img, 0.05, 0.95), fill);
    }

    // Odd filters and sizes: a spoke count out of range is the nearest one, an unreadable way the first, a name with nothing in it no filter, and
    // an effect on a shape with no size or a very long one, or one that takes no time or forever, works a bounded amount and does not fall over.
    void animationHostileFiltersAreBounded()
    {
        auto mask = [](const char *filter, double p = 0.5) {
            QPainterPath shown;
            return deck::effectMask(QString::fromLatin1(filter), p, true, &shown) ? shown : QPainterPath(QPointF(-5, -5));
        };
        QCOMPARE(mask("wheel(999)"), mask("wheel(8)"));
        QCOMPARE(mask("wheel(0)"), mask("wheel(1)"));
        QCOMPARE(mask("wheel(x)"), mask("wheel(1)"));
        QCOMPARE(mask("blinds(x)"), mask("blinds(horizontal)"));
        QCOMPARE(mask("checkerboard(x)"), mask("checkerboard(across)"));
        QCOMPARE(mask("randombar(x)"), mask("randombar(horizontal)"));
        for (const char *none : {"checkerboard()", "wheel()", "wheel(-1)", "blinds(a b)", "box", "dissolve()", ""})
            QCOMPARE(mask(none), QPainterPath(QPointF(-5, -5)));
        for (const char *filter : {"wheel(999)", "wheel(8)", "dissolve", "checkerboard(across)", "randombar(vertical)", "plus(in)", "circle(in)", "blinds(vertical)"})
            for (double p : {0.0, 0.3, 1.0, 7.0, -3.0}) QVERIFY2(mask(filter, p).elementCount() < 10000, filter);
        // As a file has one.
        const deck::Timeline wheel = deck::readTimeline(animTiming(animSteps({animNode("entr", 21, 4, "clickEffect", 0, {animShow(10, true), animFilter(10, "wheel(999)", true, 1000)})})));
        auto d = editDeck(3);
        Page &slide = *d->pages[0];
        const deck::Geometry g = deck::geometryOf(*d, slide);
        const deck::TargetState viaFile = deck::statesAt(wheel, g, 0, 500).value(deck::Target{10});
        QVERIFY(viaFile.masked);
        QCOMPARE(viaFile.mask, mask("wheel(8)"));
        // A shape with no size, a very long one, and a normal one; no time at all and about eleven days.
        static_cast<ShapeItem *>(slide.items[2].get())->rect = QRectF(100, 100, 0, 0);
        static_cast<ShapeItem *>(slide.items[3].get())->rect = QRectF(0, 100, 20000, 1);
        LayoutCache cache;
        int painted = 0;
        for (int duration : {0, 1, 1000000000})
            for (const char *kind : {"entr.dissolve", "entr.wheel", "exit.checkerboard", "entr.blinds", "exit.circle"}) {
                deck::AnimSequence seq;
                for (int shape : {10, 11, 12}) {
                    deck::Anim a = deck::makeAnim(QLatin1String(kind), shape);
                    a.duration = duration;
                    a.start = deck::EffectStart::WithPrevious;
                    seq.add(a);
                }
                deck::setAnimations(*d, slide, seq);
                const deck::Timeline tl = deck::readTimeline(slide.props.value(QStringLiteral("timingXml")).toString());
                QVERIFY(!tl.isEmpty());
                for (double ms : {0.0, 500.0, 1e6}) {
                    const deck::States states = deck::statesAt(tl, g, 0, ms);
                    for (const deck::TargetState &s : states) QVERIFY2(s.mask.elementCount() < 10000, kind);
                    const QImage img = paintedSlide(d.get(), &cache, 0, [&](PaintContext &ctx) { deck::animate(ctx, *d, states); });
                    QVERIFY(!img.isNull());
                    ++painted;
                }
            }
        QCOMPARE(painted, 3 * 5 * 3);
    }

    // A line has no width or no height, so there is nothing to uncover: it shows at the end of an entrance and goes at the end of an exit.
    void animationDegenerateShapesShowAndHideAtTheEnd()
    {
        auto d = editDeck(2);
        Page &slide = *d->pages[0];
        static_cast<ShapeItem *>(slide.items[2].get())->rect = QRectF(100, 400, 200, 0);   // a flat line
        static_cast<ShapeItem *>(slide.items[3].get())->rect = QRectF(400, 400, 0, 60);    // an upright one
        const deck::Geometry g = deck::geometryOf(*d, slide);
        for (int shape : {10, 11})
            for (const char *kind : {"entr.checkerboard", "entr.wheel", "entr.dissolve", "exit.circle", "exit.blinds"}) {
                deck::AnimSequence seq;
                deck::Anim a = deck::makeAnim(QLatin1String(kind), shape);
                a.duration = 1000;
                seq.add(a);
                deck::setAnimations(*d, slide, seq);
                const deck::Timeline tl = deck::readTimeline(slide.props.value(QStringLiteral("timingXml")).toString());
                const QString where = QLatin1String(kind) + QLatin1Char(' ') + QString::number(shape);
                const deck::TargetState mid = deck::statesAt(tl, g, 0, 500).value(deck::Target{shape}), end = deck::statesAt(tl, g, 1).value(deck::Target{shape});
                QVERIFY2(!mid.masked && !end.masked, qPrintable(where));   // no mask over a box with no area
                if (kind[1] == 'n') {
                    QVERIFY2(!mid.visible && end.visible, qPrintable(where));   // an entrance: there at the end
                } else {
                    QVERIFY2(mid.visible && !end.visible, qPrintable(where));   // an exit: gone at the end
                }
            }
    }

    // What the program has no kind for stays as the file wrote it through a saved file: a Swivel whose path was changed by hand, and Strips.
    void animationUnknownEffectsSurviveSaving()
    {
        auto d = editDeck(2);
        Page &slide = *d->pages[0];
        deck::AnimSequence one;
        one.add(deck::makeAnim(QStringLiteral("entr.swivel"), 11));
        deck::setAnimations(*d, slide, one);
        QString swivel = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY(swivel.contains(QStringLiteral("#ppt_w*0.4")));
        QVERIFY(!deck::readAnimations(slide).effects[0].custom());   // as the program writes it, it is a kind
        swivel.replace(QStringLiteral("#ppt_w*0.4"), QStringLiteral("#ppt_w*0.45"));
        slide.props[QStringLiteral("timingXml")] = swivel;
        deck::AnimSequence seq = deck::readAnimations(slide);
        QCOMPARE(seq.effects.size(), 1);
        QVERIFY(seq.effects[0].custom());
        const QString strips = animNode("entr", 18, 1, "clickEffect", 0, {animShow(10, true), animFilter(10, "strips(rightDown)", true, 2000)});
        slide.props[QStringLiteral("timingXml")] = animTiming(animStep({animGroup(0, {strips})}));
        const deck::AnimSequence stripped = deck::readAnimations(slide);
        QCOMPARE(stripped.effects.size(), 1);
        QVERIFY(stripped.effects[0].custom());
        seq.effects.push_front(stripped.effects[0]);
        deck::setAnimations(*d, slide, seq);
        const deck::AnimSequence before = deck::readAnimations(slide);
        QCOMPARE(before.effects.size(), 2);
        QVERIFY(before.effects[0].custom() && before.effects[1].custom());
        QVERIFY(before.effects[0].xml.contains(QStringLiteral("strips(rightDown)")));
        QVERIFY(before.effects[1].xml.contains(QStringLiteral("#ppt_w*0.45")));
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const deck::AnimSequence after = deck::readAnimations(*back->pages[0]);
        QCOMPARE(after.effects.size(), 2);
        for (int i = 0; i < 2; ++i) {
            QVERIFY(after.effects[i].custom());
            QCOMPARE(after.effects[i].xml, before.effects[i].xml);   // byte for byte
        }
    }

    // A transparency is read as the effect it is whether the file animates the opacity or sets it, and both play.
    void animationTransparencyReadsBothForms()
    {
        auto d = editDeck(1);
        Page &slide = *d->pages[0];
        deck::AnimSequence seq;
        deck::Anim a = deck::makeAnim(QStringLiteral("emph.transparency"), 10);
        a.amount = 75;
        seq.add(a);
        deck::setAnimations(*d, slide, seq);
        const QString animated = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY(animated.contains(QStringLiteral("<p:anim ")));
        deck::AnimSequence read = deck::readAnimations(slide);
        QVERIFY(!read.effects[0].custom());
        QCOMPARE(read.effects[0].amount, 75.0);
        static const QRegularExpression form(QStringLiteral("<p:anim calcmode=\"lin\" valueType=\"num\">(<p:cBhvr>.*?</p:cBhvr>)<p:tavLst>.*?<p:tav tm=\"100000\"><p:val><p:strVal val=\"([^\"]*)\"/></p:val></p:tav></p:tavLst></p:anim>"),
                                             QRegularExpression::DotMatchesEverythingOption);
        QString set = animated;
        set.replace(form, QStringLiteral("<p:set>\\1<p:to><p:strVal val=\"\\2\"/></p:to></p:set>"));
        QVERIFY(set != animated && !set.contains(QStringLiteral("<p:anim ")));
        slide.props[QStringLiteral("timingXml")] = set;
        read = deck::readAnimations(slide);
        QCOMPARE(read.effects.size(), 1);
        QVERIFY2(!read.effects[0].custom(), "a set of the opacity is a Transparency too");
        QCOMPARE(read.effects[0].kind, QStringLiteral("emph.transparency"));
        QCOMPARE(read.effects[0].amount, 75.0);
        // And the shape is as clear as it says once it has played.
        const deck::Timeline tl = deck::readTimeline(set);
        QCOMPARE(deck::statesAt(tl, deck::geometryOf(*d, slide), 1).value(deck::Target{10}).opacity, 0.25);
    }

    // The Animations gallery lists each new effect (by the actions the command search and the ribbon use), with a picture that is not the
    // plain slide: halfway, the shape is cut or moved.
    void animationGalleryListsTheCommonEffects()
    {
        DeckWindow w;
        w.show();
        QTRY_VERIFY(w.windowHandle() && w.windowHandle()->isExposed());
        auto *gallery = w.ribbon()->findChild<Gallery *>(QStringLiteral("animationGallery"));
        QVERIFY(gallery);
        QSet<QString> ids;
        for (const GalleryItem &item : gallery->items()) ids.insert(item.id);
        Editor *ed = w.editor();
        Page &slide = *ed->doc()->pages[0];
        ed->select(slide.items[0]->id);
        const QStringList entrance = {"blinds", "box", "checkerboard", "circle", "diamond", "plus", "dissolve", "randombars", "wheel", "peek", "flash", "stretch", "swivel", "expand", "spiral", "bounce"};
        for (const QString &k : entrance) {
            QVERIFY2(ids.contains(QStringLiteral("entr.") + k), qPrintable(k));
            QVERIFY2(w.act(QStringLiteral("anim.set.entr.") + k), qPrintable(k));
            w.act(QStringLiteral("anim.set.entr.") + k)->trigger();   // and the effect is on the shape
            QVERIFY2(deck::readAnimations(slide).effects.size() == 1 && deck::readAnimations(slide).effects[0].kind == QStringLiteral("entr.") + k, qPrintable(k));
            if (k != QLatin1String("flash")) {
                QVERIFY2(ids.contains(QStringLiteral("exit.") + k), qPrintable(k));
                QVERIFY2(w.act(QStringLiteral("anim.set.exit.") + k), qPrintable(k));
                w.act(QStringLiteral("anim.set.exit.") + k)->trigger();
                QVERIFY2(deck::readAnimations(slide).effects.size() == 1 && deck::readAnimations(slide).effects[0].kind == QStringLiteral("exit.") + k, qPrintable(k));
            }
        }
        QVERIFY(ids.contains(QStringLiteral("emph.transparency")));
        // A picture of a masked effect differs from the same shape unmasked.
        const QSize size(64, 60);
        const QImage checker = effectIcon(*deck::effectKind(QStringLiteral("entr.checkerboard"))).pixmap(size).toImage(), circle = effectIcon(*deck::effectKind(QStringLiteral("entr.circle"))).pixmap(size).toImage(),
                     appear = effectIcon(*deck::effectKind(QStringLiteral("entr.appear"))).pixmap(size).toImage();
        QVERIFY(checker != appear && circle != appear && checker != circle);
        // The effect's options: the wheel's spokes and the transparency's amount.
        deck::Anim wheel = deck::makeAnim(QStringLiteral("entr.wheel"), 3);
        AnimationOptionsDialog dlg(wheel, false, AnimationOptionsDialog::EffectTab);
        auto *variant = dlg.findChild<QComboBox *>(QStringLiteral("animVariant"));
        QVERIFY(variant);
        QCOMPARE(variant->count(), 5);
        deck::Anim clear = deck::makeAnim(QStringLiteral("emph.transparency"), 3);
        QCOMPARE(clear.amount, 50.0);
        AnimationOptionsDialog dlg2(clear, false, AnimationOptionsDialog::EffectTab);
        auto *amount = dlg2.findChild<QDoubleSpinBox *>(QStringLiteral("animAmount"));
        QVERIFY(amount);
        amount->setValue(25);
        QCOMPARE(dlg2.result().amount, 25.0);
    }

    // The mid-way pictures of the new effects, when JO_SHOT_DIR names a folder: the gallery and two effects in the show.
    void animationCommonEffectScreenshots()
    {
        const QString folder = qEnvironmentVariable("JO_SHOT_DIR");
        if (folder.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QTRY_VERIFY(w.windowHandle() && w.windowHandle()->isExposed());
        Editor *ed = w.editor();
        w.newSlide(QStringLiteral("obj"));
        auto onSlide = std::make_shared<ShapeItem>();
        onSlide->rect = QRectF(560, 330, 300, 150);
        onSlide->fill = Fill::solid(ColorRef::rgb(QColor(0x3A, 0x6E, 0xA5)));
        ed->doc()->pages[ed->currentPage()]->items.push_back(onSlide);
        ed->select(onSlide->id);
        w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Animations")));
        auto *gallery = w.ribbon()->findChild<Gallery *>(QStringLiteral("animationGallery"));
        QVERIFY(gallery);
        QApplication::processEvents();
        for (QToolButton *b : gallery->findChildren<QToolButton *>())
            if (b->accessibleName() == QLatin1String("Show all")) b->click();
        QTRY_VERIFY(QApplication::activePopupWidget());
        QVERIFY(QApplication::activePopupWidget()->grab().toImage().save(QDir(folder).filePath(QStringLiteral("common-effects-gallery.png"))));
        QApplication::activePopupWidget()->close();
        // Two effects, mid-way: a checkerboard and a wheel, each on a colored shape of a slide.
        auto d = editDeck(2);
        Page &slide = *d->pages[0];
        for (int i : {0, 1}) {
            auto *box = static_cast<ShapeItem *>(slide.items[slide.items.size() - 2 + i].get());
            box->rect = QRectF(200 + 360 * i, 200, 300, 200);
            box->fill = Fill::solid(ColorRef::rgb(QColor(0x3A, 0x6E, 0xA5)));
        }
        deck::AnimSequence seq;
        seq.add(deck::makeAnim(QStringLiteral("entr.checkerboard"), 10));
        deck::Anim wheel = deck::makeAnim(QStringLiteral("entr.wheel"), 11);
        wheel.variant = QStringLiteral("s4");
        wheel.start = deck::EffectStart::WithPrevious;
        seq.add(wheel);
        deck::setAnimations(*d, slide, seq);
        const deck::Timeline tl = deck::playableTimeline(*d, slide);
        QCOMPARE(int(tl.steps.size()), 1);
        const deck::States states = deck::statesAt(tl, deck::geometryOf(*d, slide), 0, 250);
        LayoutCache cache;
        const QImage frame = renderSlideImage(d.get(), &cache, 0, QSize(960, 540), 1, Qt::black, &states);
        QVERIFY(!frame.isNull());
        QVERIFY(frame.save(QDir(folder).filePath(QStringLiteral("common-effects-midway.png"))));
        // The same two at another moment, so the pictures show the move.
        const deck::States later = deck::statesAt(tl, deck::geometryOf(*d, slide), 0, 400);
        QVERIFY(renderSlideImage(d.get(), &cache, 0, QSize(960, 540), 1, Qt::black, &later).save(QDir(folder).filePath(QStringLiteral("common-effects-midway2.png"))));
    }

    // A wipe is a clip of the shape: the part shown, as shares of its size.
    void animationWipeClips()
    {
        const deck::Geometry g = animGeometry();
        auto clipOf = [&](const char *direction, bool entering) {
            const deck::Timeline tl = deck::readTimeline(animTiming(animSteps({animWipe(4, direction, entering, 1000)})));
            return deck::statesAt(tl, g, 0, 500).value(deck::Target{4}).clip;
        };
        QCOMPARE(clipOf("wipe(left)", true), QRectF(0, 0, 0.5, 1));      // from the left edge
        QCOMPARE(clipOf("wipe(right)", true), QRectF(0.5, 0, 0.5, 1));
        QCOMPARE(clipOf("wipe(up)", true), QRectF(0, 0, 1, 0.5));        // from the top
        QCOMPARE(clipOf("wipe(down)", true), QRectF(0, 0.5, 1, 0.5));    // from the bottom
        // Going out, the part left is the part the wipe has not reached.
        QCOMPARE(clipOf("wipe(left)", false), QRectF(0.5, 0, 0.5, 1));
        QCOMPARE(clipOf("wipe(down)", false), QRectF(0, 0, 1, 0.5));
        // At the start nothing is shown, but the clip is there; at the end there is none.
        const deck::Timeline tl = deck::readTimeline(animTiming(animSteps({animWipe(4, "wipe(left)", true, 1000)})));
        QVERIFY(!deck::statesAt(tl, g, 0, 0).value(deck::Target{4}).clip.isNull());
        QVERIFY(deck::statesAt(tl, g, 1).value(deck::Target{4}).clip.isNull());
        // Another filter is drawn as a fade.
        const deck::Timeline wheel = deck::readTimeline(animTiming(animSteps({animNode("entr", 18, 1, "clickEffect", 0, {animShow(4, true), animFilter(4, "strips(rightDown)", true, 1000)})})));
        const deck::TargetState w = deck::statesAt(wheel, g, 0, 250).value(deck::Target{4});
        QVERIFY(animNear(w.opacity, 0.25));
        QVERIFY(w.clip.isNull());
    }

    // A value a formula can't give leaves the shape where it is; the
    // formulas read numbers, #names, + - * /, and parentheses.
    void animationFormulasAndFallbacks()
    {
        const QHash<QString, double> v{{"ppt_x", 0.25}, {"ppt_y", 0.5}, {"ppt_w", 0.2}, {"ppt_h", 0.1}};
        bool ok = false;
        QVERIFY(animNear(deck::evaluateFormula(QStringLiteral("0-#ppt_w/2"), v, &ok), -0.1) && ok);
        QVERIFY(animNear(deck::evaluateFormula(QStringLiteral("1+#ppt_h/2"), v, &ok), 1.05) && ok);
        QVERIFY(animNear(deck::evaluateFormula(QStringLiteral("#ppt_x"), v, &ok), 0.25) && ok);
        QVERIFY(animNear(deck::evaluateFormula(QStringLiteral("(#ppt_x+#ppt_y)*2"), v, &ok), 1.5) && ok);
        QVERIFY(animNear(deck::evaluateFormula(QStringLiteral(" 1 + 2 * 3 "), v, &ok), 7) && ok);
        QVERIFY(animNear(deck::evaluateFormula(QStringLiteral("2*(3+4)/7"), v, &ok), 2) && ok);
        QVERIFY(animNear(deck::evaluateFormula(QStringLiteral("-#ppt_x"), v, &ok), -0.25) && ok);
        QVERIFY(animNear(deck::evaluateFormula(QStringLiteral("0.5"), v, &ok), 0.5) && ok);
        QVERIFY(animNear(deck::evaluateFormula(QStringLiteral("1-1-1"), v, &ok), -1) && ok);   // left to right
        for (const char *bad : {"", "#nope", "1 +", "(1", "abc", "1 2", "1/0", "#ppt_x)"}) {
            ok = true;
            QVERIFY2(std::isnan(deck::evaluateFormula(QString::fromLatin1(bad), v, &ok)), bad);
            QVERIFY2(!ok, bad);
        }
        // An animation whose start value can't be read starts from where the shape is.
        const deck::Geometry g = animGeometry();
        const deck::Timeline tl = deck::readTimeline(animTiming(animSteps({animNode("emph", 0, 0, "clickEffect", 0, {animValues(4, "ppt_x", "bogus(1)", "#ppt_x", 400)})})));
        QCOMPARE(deck::statesAt(tl, g, 0, 100).value(deck::Target{4}).offset, QPointF());
        QCOMPARE(deck::statesAt(tl, g, 1).value(deck::Target{4}).offset, QPointF());
    }

    // A behavior's time: its delay, how long a run is, speeding up and
    // slowing down, going back, repeating, and the end.
    void animationTimingEasesRepeatsAndReverses()
    {
        using Phase = deck::Timing::Phase;
        deck::Timing t;
        t.begin = 100;
        t.duration = 1000;
        double p = -1;
        QVERIFY(t.phaseAt(99, &p) == Phase::Before);
        QVERIFY(t.phaseAt(100, &p) == Phase::Running);
        QCOMPARE(p, 0.0);
        QVERIFY(t.phaseAt(600, &p) == Phase::Running);
        QVERIFY(animNear(p, 0.5));
        QVERIFY(t.phaseAt(1100, &p) == Phase::After);
        QCOMPARE(p, 1.0);
        QCOMPARE(t.end(), 1100.0);
        // Half a run speeding up and half slowing down: a quarter in, an eighth of the way.
        t.accel = 0.5;
        t.decel = 0.5;
        t.phaseAt(350, &p);
        QVERIFY(animNear(p, 0.125));
        t.phaseAt(850, &p);
        QVERIFY(animNear(p, 0.875));
        t.phaseAt(600, &p);
        QVERIFY(animNear(p, 0.5));
        // Going back after going: twice as long, and it ends where it began.
        t = deck::Timing();
        t.duration = 1000;
        t.autoReverse = true;
        QCOMPARE(t.activeDuration(), 2000.0);
        QVERIFY(t.phaseAt(1500, &p) == Phase::Running);
        QVERIFY(animNear(p, 0.5));
        QVERIFY(t.phaseAt(2000, &p) == Phase::After);
        QCOMPARE(p, 0.0);
        // Twice: the second run starts over, and it ends at the end.
        t = deck::Timing();
        t.duration = 1000;
        t.repeat = 2;
        QCOMPARE(t.activeDuration(), 2000.0);
        t.phaseAt(1500, &p);
        QVERIFY(animNear(p, 0.5));
        QVERIFY(t.phaseAt(2000, &p) == Phase::After);
        QCOMPARE(p, 1.0);
        // "Until the end" is played once; a cap on all the runs cuts them off.
        t.repeat = -1;
        QCOMPARE(t.activeDuration(), 1000.0);
        t.repeat = 10;
        t.repeatDuration = 2500;
        QCOMPARE(t.activeDuration(), 2500.0);
        QVERIFY(t.phaseAt(2500, &p) == Phase::After);
        QVERIFY(animNear(p, 0.5));
        // A duration that is indefinite counts as done where it begins.
        t = deck::Timing();
        t.begin = 40;
        QVERIFY(t.phaseAt(39, &p) == Phase::Before);
        QVERIFY(t.phaseAt(40, &p) == Phase::After);
        QCOMPARE(p, 1.0);
        QCOMPARE(t.activeDuration(), 0.0);
        // And all of it from a file: the cTn's accel, decel, autoRev, repeatCount and fill.
        const QString xml = animTiming(animSteps({animNode("emph", 0, 0, "clickEffect", 0, {
            animXml("<p:animRot by='3600000'><p:cBhvr><p:cTn id='9' dur='1000' accel='50000' decel='50000' autoRev='1' repeatCount='2000' fill='hold'><p:stCondLst><p:cond delay='250'/></p:stCondLst></p:cTn>") + animTarget(4) +
            QStringLiteral("</p:cBhvr></p:animRot>")})}));
        const deck::Timing read = deck::readTimeline(xml).steps[0].effects[0].behaviors[0].timing;
        QCOMPARE(read.begin, 250.0);
        QCOMPARE(read.duration, 1000.0);
        QCOMPARE(read.accel, 0.5);
        QCOMPARE(read.decel, 0.5);
        QVERIFY(read.autoReverse && read.hold);
        QCOMPARE(read.repeat, 2.0);
        QCOMPARE(read.activeDuration(), 4000.0);
    }

    // Media calls and interactive sequences are read, not played: a step with
    // only a media call shows nothing and takes no click.
    void animationReadsMediaAndTriggersWithoutPlayingThem()
    {
        const QString trigger = animXml("<p:seq concurrent='1' nextAc='seek'><p:cTn id='40' restart='whenNotActive' fill='hold' evtFilter='cancelBubble' nodeType='interactiveSeq'><p:stCondLst><p:cond evt='onClick' delay='0'><p:tgtEl><p:spTgt spid='6'/></p:tgtEl></p:cond></p:stCondLst><p:endSync evt='end' delay='0'><p:rtn val='all'/></p:endSync><p:childTnLst>")
            + animGroup(0, {animFadeIn(5, 500)}) + animXml("</p:childTnLst></p:cTn><p:nextCondLst><p:cond evt='onClick' delay='0'><p:tgtEl><p:spTgt spid='6'/></p:tgtEl></p:cond></p:nextCondLst></p:seq>");
        const QString xml = animTiming(animSteps({animMediaCall(4)}), animXml("<p:video><p:cMediaNode vol='80000'><p:cTn id='30' fill='hold' display='0'><p:stCondLst><p:cond delay='indefinite'/></p:stCondLst></p:cTn><p:tgtEl><p:spTgt spid='4'/></p:tgtEl></p:cMediaNode></p:video>") + trigger);
        const deck::Timeline tl = deck::readTimeline(xml);
        QCOMPARE(int(tl.steps.size()), 1);
        QVERIFY(tl.steps[0].effects[0].cls == deck::EffectClass::Media);
        QVERIFY(tl.steps[0].effects[0].behaviors[0].kind == deck::Behavior::Kind::Command);
        QCOMPARE(tl.steps[0].effects[0].behaviors[0].command, QStringLiteral("playFrom(0.0)"));
        QVERIFY(!tl.steps[0].visual());
        QCOMPARE(int(tl.triggers.size()), 1);
        QCOMPARE(tl.triggers[0].shape, 6);
        QCOMPARE(int(tl.triggers[0].effects.size()), 1);
        // The interactive sequence's shape is not in the states of the main sequence.
        QVERIFY(deck::statesAt(tl, animGeometry(), 0).isEmpty());
        Page slide;
        slide.props[QStringLiteral("timingXml")] = xml;
        QVERIFY(deck::playableTimeline(slide).steps.isEmpty());
        slide.props[QStringLiteral("timingXml")] = animTiming(animSteps({animMediaCall(4), animAppear(5)}));
        QCOMPARE(int(deck::playableTimeline(slide).steps.size()), 1);
        QCOMPARE(int(deck::timelineOf(slide).steps.size()), 2);
    }

    // The effects of a timing as its tree lays them out: "<click> <part's delay> <how it starts> <shape> <preset>/<subtype>" each.
    static QStringList animOutline(const QString &timing)
    {
        QStringList out;
        QDomDocument dom;
        if (!office::parseXml(timing.toUtf8(), &dom)) return out;
        int clicks = 0;
        std::function<void(const QDomElement &, int, int)> walk = [&](const QDomElement &e, int click, int part) {
            for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
                if (c.localName() == QLatin1String("cTn")) {
                    const QString type = c.attribute(QStringLiteral("nodeType"));
                    if (type.endsWith(QLatin1String("Effect"))) {
                        QString spid;
                        QVector<QDomElement> stack{c};
                        while (!stack.isEmpty() && spid.isEmpty()) {
                            const QDomElement x = stack.takeLast();
                            if (x.localName() == QLatin1String("spTgt")) spid = x.attribute(QStringLiteral("spid"));
                            for (QDomElement k = x.firstChildElement(); !k.isNull(); k = k.nextSiblingElement()) stack << k;
                        }
                        out << QStringLiteral("%1 %2 %3 %4 %5/%6").arg(click).arg(part).arg(type, spid, c.attribute(QStringLiteral("presetID")), c.attribute(QStringLiteral("presetSubtype")));
                        continue;
                    }
                    const QDomElement cond = c.firstChildElement(QStringLiteral("stCondLst")).firstChildElement(QStringLiteral("cond"));
                    const QString delay = cond.attribute(QStringLiteral("delay"));
                    bool byItself = false;
                    for (QDomElement k = c.firstChildElement(QStringLiteral("stCondLst")).firstChildElement(); !k.isNull(); k = k.nextSiblingElement())
                        byItself |= k.attribute(QStringLiteral("evt")) == QLatin1String("onBegin");
                    if (delay == QLatin1String("indefinite")) walk(c.firstChildElement(QStringLiteral("childTnLst")), byItself ? 0 : ++clicks, 0);
                    else if (type.isEmpty()) walk(c.firstChildElement(QStringLiteral("childTnLst")), click, delay.toInt());
                    else walk(c.firstChildElement(QStringLiteral("childTnLst")), click, part);
                } else {
                    walk(c, click, part);
                }
            }
        };
        const QDomElement main = [&] {
            QVector<QDomElement> stack{dom.documentElement()};
            while (!stack.isEmpty()) {
                const QDomElement x = stack.takeLast();
                if (x.localName() == QLatin1String("cTn") && x.attribute(QStringLiteral("nodeType")) == QLatin1String("mainSeq")) return x;
                for (QDomElement k = x.firstChildElement(); !k.isNull(); k = k.nextSiblingElement()) stack << k;
            }
            return QDomElement();
        }();
        walk(main.firstChildElement(QStringLiteral("childTnLst")), 0, 0);
        return out;
    }

    // Each kind of effect, in each of its ways, is written so that it reads back as the same kind, way, and length: the
    // timing is exactly what the presentation program writes for it.
    void animationEditKindsRoundTrip()
    {
        const auto &kinds = deck::effectKinds();
        QVERIFY(kinds.size() > 20);
        QSet<QString> ids;
        QSet<deck::EffectClass> classes;
        for (const deck::EffectKind &kind : kinds) {
            QVERIFY2(!ids.contains(kind.id), qPrintable(kind.id));
            ids.insert(kind.id);
            classes.insert(kind.cls);
            QVERIFY(!kind.name.isEmpty() && !kind.variants.isEmpty());
        }
        QCOMPARE(int(classes.size()), 4);   // entrance, exit, emphasis, and motion path
        QVERIFY(deck::effectKind(QStringLiteral("entr.fade")) && deck::effectKind(QStringLiteral("exit.fly")) && deck::effectKind(QStringLiteral("emph.spin")) && deck::effectKind(QStringLiteral("path.lines")));
        auto d = editDeck();
        Page &slide = *d->pages[0];
        int made = 0;
        for (const deck::EffectKind &kind : kinds) {
            QSet<QString> keys;
            for (const deck::EffectVariant &v : kind.variants) {
                QVERIFY2(!keys.contains(v.key), qPrintable(kind.id + " " + v.key));
                keys.insert(v.key);
                deck::AnimSequence seq;
                deck::Anim a = deck::makeAnim(kind, 3);
                a.variant = v.key;
                seq.add(a);
                deck::setAnimations(*d, slide, seq);
                const QString where = kind.id + QLatin1Char(' ') + v.key;
                QVERIFY2(animIdProblem(slide.props.value(QStringLiteral("timingXml")).toString()).isEmpty(), qPrintable(where));
                QVERIFY2(!slide.props.value(QStringLiteral("timingXml")).toString().isEmpty(), qPrintable(where));
                const deck::AnimSequence back = deck::readAnimations(slide);
                QVERIFY2(back.effects.size() == 1, qPrintable(where));
                const deck::Anim &b = back.effects[0];
                QVERIFY2(!b.custom() && b.kind == kind.id && b.variant == v.key, qPrintable(where + " read as " + b.kind + " " + b.variant));
                QCOMPARE(b.duration, a.duration);
                QCOMPARE(b.shape, 3);
                QCOMPARE(b.smoothStart, a.smoothStart);
                QCOMPARE(b.amount, a.amount);
                ++made;
            }
        }
        QVERIFY(made > 100);   // the motion paths alone are 64
    }

    // The first effect of a title, as the presentation program writes it for a Fade on a click: the same XML, ids and all.
    void animationEditWritesAFadeAsTheProgramDoes()
    {
        auto d = editDeck();
        Page &slide = *d->pages[0];
        deck::AnimSequence seq;
        seq.add(deck::makeAnim(QStringLiteral("entr.fade"), 2));
        deck::setAnimations(*d, slide, seq);
        const QString expected = animXml(
            "<p:timing xmlns:p='http://schemas.openxmlformats.org/presentationml/2006/main'><p:tnLst><p:par><p:cTn id='1' dur='indefinite' restart='never' nodeType='tmRoot'><p:childTnLst>"
            "<p:seq concurrent='1' nextAc='seek'><p:cTn id='2' dur='indefinite' nodeType='mainSeq'><p:childTnLst>"
            "<p:par><p:cTn id='3' fill='hold'><p:stCondLst><p:cond delay='indefinite'/></p:stCondLst><p:childTnLst>"
            "<p:par><p:cTn id='4' fill='hold'><p:stCondLst><p:cond delay='0'/></p:stCondLst><p:childTnLst>"
            "<p:par><p:cTn id='5' presetID='10' presetClass='entr' presetSubtype='0' fill='hold' grpId='0' nodeType='clickEffect' nodePh='1'><p:stCondLst><p:cond delay='0'/></p:stCondLst><p:childTnLst>"
            "<p:set><p:cBhvr><p:cTn id='6' dur='1' fill='hold'><p:stCondLst><p:cond delay='0'/></p:stCondLst></p:cTn><p:tgtEl><p:spTgt spid='2'/></p:tgtEl><p:attrNameLst><p:attrName>style.visibility</p:attrName></p:attrNameLst></p:cBhvr><p:to><p:strVal val='visible'/></p:to></p:set>"
            "<p:animEffect transition='in' filter='fade'><p:cBhvr><p:cTn id='7' dur='500'/><p:tgtEl><p:spTgt spid='2'/></p:tgtEl></p:cBhvr></p:animEffect>"
            "</p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn>"
            "<p:prevCondLst><p:cond evt='onPrev' delay='0'><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:prevCondLst>"
            "<p:nextCondLst><p:cond evt='onNext' delay='0'><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:nextCondLst></p:seq>"
            "</p:childTnLst></p:cTn></p:par></p:tnLst><p:bldLst><p:bldP spid='2' grpId='0'/></p:bldLst></p:timing>");
        QCOMPARE(slide.props.value(QStringLiteral("timingXml")).toString(), expected);
        // A box with a fill is built with its background; a number it did not have goes into the timing.
        auto d2 = editDeck(1);
        deck::AnimSequence two;
        two.add(deck::makeAnim(QStringLiteral("entr.fade"), 10));
        deck::setAnimations(*d2, *d2->pages[0], two);
        QVERIFY(d2->pages[0]->props.value(QStringLiteral("timingXml")).toString().contains(QStringLiteral("<p:bldP spid=\"10\" grpId=\"0\" animBg=\"1\"/>")));
    }

    // The click steps, and the effects in them that start with or after the one before, nest as the program nests them, and the
    // part that follows another starts when it ends (delays, durations, and all).
    void animationEditNestsTheSteps()
    {
        auto d = editDeck(9);
        Page &slide = *d->pages[0];
        deck::AnimSequence seq;
        const deck::EffectStart C = deck::EffectStart::OnClick, W = deck::EffectStart::WithPrevious, A = deck::EffectStart::AfterPrevious;
        const deck::EffectStart starts[] = {C, W, A, W, C, A, A, W, C};
        for (int i = 0; i < 9; ++i) {
            deck::Anim a = deck::makeAnim(QStringLiteral("entr.fade"), 10 + i);
            a.start = starts[i];
            seq.add(a);
        }
        deck::setAnimations(*d, slide, seq);
        QCOMPARE(animOutline(slide.props.value(QStringLiteral("timingXml")).toString()),
                 (QStringList{QStringLiteral("1 0 clickEffect 10 10/0"), QStringLiteral("1 0 withEffect 11 10/0"), QStringLiteral("1 500 afterEffect 12 10/0"),
                              QStringLiteral("1 500 withEffect 13 10/0"), QStringLiteral("2 0 clickEffect 14 10/0"), QStringLiteral("2 500 afterEffect 15 10/0"),
                              QStringLiteral("2 1000 afterEffect 16 10/0"), QStringLiteral("2 1000 withEffect 17 10/0"), QStringLiteral("3 0 clickEffect 18 10/0")}));
        QCOMPARE(seq.clicks(), 3);
        QCOMPARE(seq.clickOf(3), 1);
        QCOMPARE(seq.clickOf(8), 3);
        // Those that start when the slide shows: the click step is marked to begin by itself.
        deck::AnimSequence early;
        for (int i = 0; i < 3; ++i) {
            deck::Anim a = deck::makeAnim(QStringLiteral("entr.fade"), 10 + i);
            a.start = i == 0 ? W : i == 1 ? A : C;
            early.add(a);
        }
        deck::setAnimations(*d, slide, early);
        const QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY(xml.contains(QStringLiteral("<p:cond evt=\"onBegin\" delay=\"0\"><p:tn val=\"2\"/></p:cond>")));
        QCOMPARE(animOutline(xml), (QStringList{QStringLiteral("0 0 withEffect 10 10/0"), QStringLiteral("0 500 afterEffect 11 10/0"), QStringLiteral("1 0 clickEffect 12 10/0")}));
        QCOMPARE(deck::readAnimations(slide).effects[0].start, W);
        // Delays and durations: a part starts after the delay and length of what came before it in its step.
        deck::AnimSequence timed;
        deck::Anim p0 = deck::makeAnim(QStringLiteral("entr.fade"), 10);
        p0.delay = 1000;
        deck::Anim p1 = deck::makeAnim(QStringLiteral("entr.wipe"), 11);
        p1.start = A;
        p1.delay = 500;
        p1.duration = 2000;
        deck::Anim p2 = deck::makeAnim(QStringLiteral("entr.fade"), 12);
        p2.start = W;
        p2.delay = 2000;
        deck::Anim p3 = deck::makeAnim(QStringLiteral("entr.fade"), 13);
        p3.start = A;
        timed.add(p0);
        timed.add(p1);
        timed.add(p2);
        timed.add(p3);
        deck::setAnimations(*d, slide, timed);
        QCOMPARE(animOutline(slide.props.value(QStringLiteral("timingXml")).toString()),
                 (QStringList{QStringLiteral("1 0 clickEffect 10 10/0"), QStringLiteral("1 1500 afterEffect 11 22/4"), QStringLiteral("1 1500 withEffect 12 10/0"),
                              QStringLiteral("1 4000 afterEffect 13 10/0")}));
        QCOMPARE(timed.beginOf(1), 1500 + 500);
        QCOMPARE(timed.endOf(3), 4000 + 500);
    }

    // What the presentation program writes for effects reads as the kind it is, with its own settings: an exit that flies
    // out to the bottom, and a fade with a delay, smooth start, and repeats.
    void animationEditReadsWhatTheProgramWrites()
    {
        auto d = editDeck();
        Page &slide = *d->pages[0];
        const QString flyOut = animXml(
            "<p:par><p:cTn id='5' presetID='2' presetClass='exit' presetSubtype='4' fill='hold' grpId='0' nodeType='clickEffect'><p:stCondLst><p:cond delay='0'/></p:stCondLst><p:childTnLst>"
            "<p:anim calcmode='lin' valueType='num'><p:cBhvr additive='base'><p:cTn id='6' dur='500'/><p:tgtEl><p:spTgt spid='3'/></p:tgtEl><p:attrNameLst><p:attrName>ppt_x</p:attrName></p:attrNameLst></p:cBhvr>"
            "<p:tavLst><p:tav tm='0'><p:val><p:strVal val='ppt_x'/></p:val></p:tav><p:tav tm='100000'><p:val><p:strVal val='ppt_x'/></p:val></p:tav></p:tavLst></p:anim>"
            "<p:anim calcmode='lin' valueType='num'><p:cBhvr additive='base'><p:cTn id='7' dur='500'/><p:tgtEl><p:spTgt spid='3'/></p:tgtEl><p:attrNameLst><p:attrName>ppt_y</p:attrName></p:attrNameLst></p:cBhvr>"
            "<p:tavLst><p:tav tm='0'><p:val><p:strVal val='ppt_y'/></p:val></p:tav><p:tav tm='100000'><p:val><p:strVal val='1+ppt_h/2'/></p:val></p:tav></p:tavLst></p:anim>"
            "<p:set><p:cBhvr><p:cTn id='8' dur='1' fill='hold'><p:stCondLst><p:cond delay='499'/></p:stCondLst></p:cTn><p:tgtEl><p:spTgt spid='3'/></p:tgtEl><p:attrNameLst><p:attrName>style.visibility</p:attrName></p:attrNameLst></p:cBhvr><p:to><p:strVal val='hidden'/></p:to></p:set>"
            "</p:childTnLst></p:cTn></p:par>");
        auto wrap = [](const QString &effects) {
            return animXml("<p:timing xmlns:p='http://schemas.openxmlformats.org/presentationml/2006/main'><p:tnLst><p:par><p:cTn id='1' dur='indefinite' restart='never' nodeType='tmRoot'><p:childTnLst>"
                           "<p:seq concurrent='1' nextAc='seek'><p:cTn id='2' dur='indefinite' nodeType='mainSeq'><p:childTnLst>"
                           "<p:par><p:cTn id='3' fill='hold'><p:stCondLst><p:cond delay='indefinite'/></p:stCondLst><p:childTnLst><p:par><p:cTn id='4' fill='hold'><p:stCondLst><p:cond delay='0'/></p:stCondLst><p:childTnLst>")
                + effects + animXml("</p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:seq></p:childTnLst></p:cTn></p:par></p:tnLst></p:timing>");
        };
        slide.props[QStringLiteral("timingXml")] = wrap(flyOut);
        deck::AnimSequence seq = deck::readAnimations(slide);
        QCOMPARE(seq.effects.size(), 1);
        QCOMPARE(seq.effects[0].kind, QStringLiteral("exit.fly"));
        QCOMPARE(seq.effects[0].variant, QStringLiteral("bottom"));
        QCOMPARE(seq.effects[0].duration, 500);
        QCOMPARE(seq.effects[0].shape, 3);
        QVERIFY(!seq.effects[0].custom());
        // A fade with a delay, smooth start and end, repeated twice, rewound.
        QString fade = animFadeIn(3, 1500, "withEffect", 250);
        fade.replace(QStringLiteral("presetSubtype=\"0\" fill=\"hold\""), QStringLiteral("presetSubtype=\"0\" repeatCount=\"2000\" accel=\"50000\" decel=\"50000\" fill=\"remove\" grpId=\"0\""));
        slide.props[QStringLiteral("timingXml")] = wrap(fade);
        seq = deck::readAnimations(slide);
        QCOMPARE(seq.effects.size(), 1);
        const deck::Anim &f = seq.effects[0];
        QVERIFY2(!f.custom(), "a fade with the settings the Timing dialog gives is a fade");
        QCOMPARE(f.start, deck::EffectStart::WithPrevious);
        QCOMPARE(f.delay, 250);
        QCOMPARE(f.duration, 1500);
        QVERIFY(f.smoothStart && f.smoothEnd && f.rewind);
        QCOMPARE(f.repeat, 2.0);
        // The same effect written again is the same effect (the ids aside).
        deck::setAnimations(*d, slide, seq);
        const deck::AnimSequence again = deck::readAnimations(slide);
        QCOMPARE(again.effects.size(), 1);
        QVERIFY(!again.effects[0].custom());
        QCOMPARE(again.effects[0].duration, 1500);
        QCOMPARE(again.effects[0].repeat, 2.0);
        QVERIFY(again.effects[0].rewind);
    }

    // What JeffDeck has no kind for stays as the file wrote it: it lists as Custom, can be moved, removed, and started another
    // way, and the rest of the timing is written around it.
    void animationEditKeepsWhatItDoesNotKnow()
    {
        auto d = editDeck(1);
        Page &slide = *d->pages[0];
        // Strips (entrance 18): an effect with no kind here.
        const QString strips = animNode("entr", 18, 1, "clickEffect", 0, {animShow(10, true), animFilter(10, "strips(rightDown)", true, 2000)});
        slide.props[QStringLiteral("timingXml")] = animTiming(animStep({animGroup(0, {strips})}) + animStep({animGroup(0, {animFadeIn(3, 500)})}));
        deck::AnimSequence seq = deck::readAnimations(slide);
        QCOMPARE(seq.effects.size(), 2);
        QVERIFY(seq.effects[0].custom());
        QCOMPARE(seq.effects[0].shape, 10);
        QCOMPARE(seq.effects[0].cls, deck::EffectClass::Entrance);
        QVERIFY(!seq.effects[1].custom());
        QCOMPARE(seq.effects[0].duration, 2000);
        // Written again with the fade before it, the strips' behaviors are as they were.
        seq.move(1, 0);
        QVERIFY(seq.effects[1].custom());
        deck::setAnimations(*d, slide, seq);
        const QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY(xml.contains(QStringLiteral("presetID=\"18\"")) && xml.contains(QStringLiteral("filter=\"strips(rightDown)\"")));
        QCOMPARE(animOutline(xml), (QStringList{QStringLiteral("1 0 clickEffect 3 10/0"), QStringLiteral("2 0 clickEffect 10 18/1")}));
        // Ids stay unique and sequential in document order.
        QDomDocument dom;
        QVERIFY(office::parseXml(xml.toUtf8(), &dom));
        QStringList ids;
        QVector<QDomElement> stack{dom.documentElement()};
        while (!stack.isEmpty()) {
            const QDomElement e = stack.takeFirst();
            if (e.localName() == QLatin1String("cTn")) ids << e.attribute(QStringLiteral("id"));
            for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack << c;
        }
        QCOMPARE(int(QSet<QString>(ids.cbegin(), ids.cend()).size()), int(ids.size()));
        // It can start with the one before, and be removed.
        seq = deck::readAnimations(slide);
        seq.effects[1].start = deck::EffectStart::WithPrevious;
        deck::setAnimations(*d, slide, seq);
        QCOMPARE(animOutline(slide.props.value(QStringLiteral("timingXml")).toString()), (QStringList{QStringLiteral("1 0 clickEffect 3 10/0"), QStringLiteral("1 0 withEffect 10 18/1")}));
        seq = deck::readAnimations(slide);
        seq.remove(1);
        deck::setAnimations(*d, slide, seq);
        QCOMPARE(deck::readAnimations(slide).effects.size(), 1);
        seq.remove(0);
        deck::setAnimations(*d, slide, seq);
        QVERIFY(!slide.props.contains(QStringLiteral("timingXml")));   // nothing left: no timing
    }

    // Removing an effect that took a click gives the click to the next effect of its step; moving and starting effects another way.
    void animationEditOrdersTheEffects()
    {
        auto d = editDeck(4);
        deck::AnimSequence seq;
        const deck::EffectStart C = deck::EffectStart::OnClick, W = deck::EffectStart::WithPrevious;
        const deck::EffectStart starts[] = {C, W, C, W};
        for (int i = 0; i < 4; ++i) {
            deck::Anim a = deck::makeAnim(QStringLiteral("entr.fade"), 10 + i);
            a.start = starts[i];
            seq.add(a);
        }
        QCOMPARE(seq.effectsOf(11), (QVector<int>{1}));
        seq.remove(0);
        QCOMPARE(seq.effects.size(), 3);
        QCOMPARE(seq.effects[0].shape, 11);
        QCOMPARE(seq.effects[0].start, C);   // the click stays
        QCOMPARE(seq.clicks(), 2);
        // Later: the effect goes after the next.
        seq.move(0, 1);
        QCOMPARE(seq.effects[1].shape, 11);
        QCOMPARE(seq.effects[0].shape, 12);
        seq.move(1, 99);   // out of range goes to the end
        QCOMPARE(seq.effects.last().shape, 11);
        seq.move(-1, 0);   // nothing
        QCOMPARE(seq.effects.size(), 3);
        // A new kind keeps what the effect aims at, how it starts, and its delay.
        deck::Anim a = deck::makeAnim(QStringLiteral("entr.wipe"), 11);
        a.start = deck::EffectStart::AfterPrevious;
        a.delay = 700;
        a.duration = 1200;
        deck::changeKind(a, *deck::effectKind(QStringLiteral("entr.fly")));
        QCOMPARE(a.kind, QStringLiteral("entr.fly"));
        QCOMPARE(a.start, deck::EffectStart::AfterPrevious);
        QCOMPARE(a.delay, 700);
        QCOMPARE(a.shape, 11);
        QCOMPARE(a.variant, QStringLiteral("bottom"));
        QCOMPARE(a.duration, 1200);
        deck::changeKind(a, *deck::effectKind(QStringLiteral("entr.appear")));
        QCOMPARE(a.duration, 0);
        deck::changeKind(a, *deck::effectKind(QStringLiteral("path.lines")));
        QCOMPARE(a.duration, 2000);
        QVERIFY(a.smoothStart && a.smoothEnd);
    }

    // A build by paragraph is one effect for each paragraph (with the ones indented below it), and back to one for the shape.
    void animationEditBuildsByParagraph()
    {
        auto d = editDeck();
        Page &slide = *d->pages[0];
        QCOMPARE(deck::paragraphGroups(*d, slide, 3), (QVector<QPair<int, int>>{{0, 0}, {1, 1}, {2, 2}}));
        deck::AnimSequence seq;
        seq.add(deck::makeAnim(QStringLiteral("entr.fade"), 3));
        deck::setByParagraph(seq, 0, true, *d, slide);
        QCOMPARE(seq.effects.size(), 3);
        QCOMPARE(seq.effects[1].firstParagraph, 1);
        QCOMPARE(seq.effects[2].start, deck::EffectStart::OnClick);
        deck::setAnimations(*d, slide, seq);
        const QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY(xml.contains(QStringLiteral("<p:bldP spid=\"3\" grpId=\"0\" build=\"p\"/>")));
        QVERIFY(xml.contains(QStringLiteral("<p:txEl><p:pRg st=\"2\" end=\"2\"/></p:txEl>")));
        QCOMPARE(xml.count(QStringLiteral("<p:bldP ")), 1);   // one build for the shape's three effects
        QCOMPARE(xml.count(QStringLiteral("grpId=\"0\" nodeType")), 3);
        const deck::AnimSequence back = deck::readAnimations(slide);
        QCOMPARE(back.effects.size(), 3);
        QVERIFY(!back.effects[0].custom() && back.effects[2].paragraphs());
        // Indented paragraphs go with the one above them.
        QTextDocument *story = d->storyDoc(static_cast<TextItem *>(slide.items[1].get())->storyId);
        QTextBlock second = story->findBlockByNumber(1);
        QTextCursor cur(second);
        QTextBlockFormat bf = second.blockFormat();
        bf.setProperty(tp::ListLevel, 1);
        cur.setBlockFormat(bf);
        QCOMPARE(deck::paragraphGroups(*d, slide, 3), (QVector<QPair<int, int>>{{0, 1}, {2, 2}}));
        // Back to the whole shape.
        deck::AnimSequence one = back;
        deck::setByParagraph(one, 1, false, *d, slide);
        QCOMPARE(one.effects.size(), 1);
        QVERIFY(!one.effects[0].paragraphs());
    }

    // Effects can be copied from one shape onto others (Animation Painter).
    void animationEditCopiesEffects()
    {
        auto d = editDeck(2);
        deck::AnimSequence seq;
        deck::Anim a = deck::makeAnim(QStringLiteral("entr.fly"), 10);
        a.variant = QStringLiteral("left");
        a.duration = 900;
        seq.add(a);
        seq.add(deck::makeAnim(QStringLiteral("emph.spin"), 10));
        deck::copyEffects(seq, 10, {11});
        QCOMPARE(seq.effects.size(), 4);
        QCOMPARE(seq.effects[2].shape, 11);
        QCOMPARE(seq.effects[2].variant, QStringLiteral("left"));
        QCOMPARE(seq.effects[2].duration, 900);
        QCOMPARE(seq.effects[3].kind, QStringLiteral("emph.spin"));
        QCOMPARE(int(seq.effectsOf(11).size()), 2);
    }

    // What the show plays of the effects JeffDeck makes: a fly in comes from off the slide, an exit moves out (its formulas
    // have no # in them), a spin turns, a pulse grows and returns, a split opens from the middle, and repeats play again.
    void animationEditPlaysWhatItMakes()
    {
        auto d = editDeck(1);
        Page &slide = *d->pages[0];
        auto states = [&](const deck::AnimSequence &seq, double ms) {
            deck::setAnimations(*d, slide, seq);
            const deck::Timeline tl = deck::playableTimeline(slide);
            return deck::statesAt(tl, deck::geometryOf(*d, slide), 0, ms);
        };
        const deck::Target box{10};
        // Fly in from the bottom: starts below the slide, ends where it is.
        deck::AnimSequence fly;
        fly.add(deck::makeAnim(QStringLiteral("entr.fly"), 10));
        QVERIFY(states(fly, 0)[box].offset.y() > 100);
        QCOMPARE(states(fly, 0)[box].offset.x(), 0.0);
        QCOMPARE(states(fly, 100000)[box].offset, QPointF());
        QVERIFY(states(fly, 100000)[box].visible);
        // And out to the bottom: where it is, then below.
        deck::AnimSequence out;
        out.add(deck::makeAnim(QStringLiteral("exit.fly"), 10));
        QCOMPARE(states(out, 0)[box].offset, QPointF());
        QVERIFY(states(out, 250)[box].offset.y() > 50);
        QVERIFY(!states(out, 100000)[box].visible);
        // A spin turns a full circle; grow/shrink resizes; a pulse grows and comes back.
        deck::AnimSequence spin;
        spin.add(deck::makeAnim(QStringLiteral("emph.spin"), 10));
        QVERIFY(std::abs(states(spin, 1000)[box].rotation - 180) < 1);
        QVERIFY(std::abs(states(spin, 100000)[box].rotation - 360) < 1e-6);
        deck::AnimSequence grow;
        grow.add(deck::makeAnim(QStringLiteral("emph.growshrink"), 10));
        QVERIFY(std::abs(states(grow, 100000)[box].scaleX - 1.5) < 1e-6);
        deck::AnimSequence pulse;
        pulse.add(deck::makeAnim(QStringLiteral("emph.pulse"), 10));
        QVERIFY(states(pulse, 250)[box].scaleX > 1.04);
        QVERIFY(states(pulse, 250)[box].opacity < 0.6);   // half see-through at its middle
        QVERIFY(std::abs(states(pulse, 100000)[box].scaleX - 1) < 1e-6);
        QCOMPARE(states(pulse, 100000)[box].opacity, 1.0);
        // A split from the middle shows a strip, then all.
        deck::AnimSequence split;
        deck::Anim sp = deck::makeAnim(QStringLiteral("entr.split"), 10);
        sp.variant = QStringLiteral("verticalOut");
        split.add(sp);
        const deck::TargetState half = states(split, 250)[box];
        QVERIFY(std::abs(half.clip.left() - 0.25) < 0.01 && std::abs(half.clip.width() - 0.5) < 0.01);
        sp.variant = QStringLiteral("verticalIn");
        deck::AnimSequence closing;
        closing.add(sp);
        const deck::TargetState parts = states(closing, 250)[box];
        QVERIFY(!parts.clip2.isNull());
        QVERIFY(std::abs(parts.clip.width() - 0.25) < 0.01 && std::abs(parts.clip2.right() - 1) < 0.01);
        // Repeated twice, an effect plays twice: a spin of 1 s by 90 degrees is 45 halfway, and halfway through the second run is 45 again.
        deck::AnimSequence twice;
        deck::Anim t = deck::makeAnim(QStringLiteral("emph.spin"), 10);
        t.duration = 1000;
        t.amount = 90;
        t.repeat = 2;
        twice.add(t);
        QVERIFY(std::abs(states(twice, 500)[box].rotation - 45) < 1);
        QVERIFY(std::abs(states(twice, 1500)[box].rotation - 45) < 1);
        QVERIFY(std::abs(states(twice, 100000)[box].rotation - 90) < 1);
        // Rewound, it goes back when it ends.
        t.repeat = 1;
        t.rewind = true;
        deck::AnimSequence rewound;
        rewound.add(t);
        QVERIFY(std::abs(states(rewound, 500)[box].rotation - 45) < 1);
        QCOMPARE(states(rewound, 100000)[box].rotation, 0.0);
    }

    // An effect plays smoothly when the effect says so, not only when each behavior does.
    void animationEditSmoothness()
    {
        auto d = editDeck(1);
        Page &slide = *d->pages[0];
        deck::AnimSequence seq;
        deck::Anim a = deck::makeAnim(QStringLiteral("emph.spin"), 10);
        a.duration = 1000;
        seq.add(a);
        auto at = [&](const deck::AnimSequence &s, double ms) {
            deck::setAnimations(*d, slide, s);
            return deck::statesAt(deck::playableTimeline(slide), deck::geometryOf(*d, slide), 0, ms)[deck::Target{10}].rotation;
        };
        QVERIFY(std::abs(at(seq, 250) - 90) < 1e-6);   // straight: a quarter of the way is a quarter turn
        seq.effects[0].smoothStart = true;
        seq.effects[0].smoothEnd = true;
        QVERIFY(at(seq, 250) < 80);                    // slow at the start
        QVERIFY(std::abs(at(seq, 500) - 180) < 1e-6);
    }

    // The builds are listed by where the shapes are on the slide, as the presentation program lists them, whatever order the
    // effects play in; a shape with two effects has two builds, numbered from 0.
    void animationEditBuildsListByShape()
    {
        auto d = editDeck(2);
        Page &slide = *d->pages[0];
        deck::AnimSequence seq;
        seq.add(deck::makeAnim(QStringLiteral("entr.fade"), 11));
        seq.add(deck::makeAnim(QStringLiteral("entr.fade"), 10));
        seq.add(deck::makeAnim(QStringLiteral("entr.fade"), 3));
        deck::setAnimations(*d, slide, seq);
        const QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
        const qsizetype a = xml.indexOf(QStringLiteral("<p:bldP spid=\"3\"")), b = xml.indexOf(QStringLiteral("<p:bldP spid=\"10\"")), c = xml.indexOf(QStringLiteral("<p:bldP spid=\"11\""));
        QVERIFY(a > 0 && b > a && c > b);
        deck::AnimSequence two;
        two.add(deck::makeAnim(QStringLiteral("entr.fade"), 10));
        two.add(deck::makeAnim(QStringLiteral("emph.spin"), 10));
        deck::setAnimations(*d, slide, two);
        const QString twoXml = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY(twoXml.contains(QStringLiteral("<p:bldP spid=\"10\" grpId=\"0\" animBg=\"1\"/><p:bldP spid=\"10\" grpId=\"1\" animBg=\"1\"/>")));
        QCOMPARE(twoXml.count(QStringLiteral("nodeType=\"clickEffect\"")), 2);
        QVERIFY(twoXml.contains(QStringLiteral("grpId=\"1\" nodeType")));
    }

    // Saved, the effects of shapes the slide no longer has are left out and the rest stay, in a file that reads back; a shape
    // that was given a number to be animated keeps it, so its effect is still on it.
    void animationEditSavesOnlyEffectsOfShapesThatAreThere()
    {
        auto d = editDeck(2);
        Page &slide = *d->pages[0];
        // A shape new to the file has no number: animating it gives it one, and it is written with it.
        auto fresh = std::make_shared<ShapeItem>();
        fresh->name = QStringLiteral("Fresh");
        fresh->rect = QRectF(400, 100, 80, 40);
        slide.items.push_back(fresh);
        QCOMPARE(fresh->sourceId, 0);
        const int number = deck::shapeNumber(slide, fresh->id);
        QVERIFY(number > 11);   // past the numbers the boxes have
        QCOMPARE(deck::shapeNumber(slide, fresh->id), number);   // the same again
        deck::AnimSequence seq;
        seq.add(deck::makeAnim(QStringLiteral("entr.fade"), 10));
        seq.add(deck::makeAnim(QStringLiteral("entr.fly"), number));
        seq.add(deck::makeAnim(QStringLiteral("exit.fade"), 11));
        deck::setAnimations(*d, slide, seq);
        QString err;
        QStringList skipped;
        QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        deck::AnimSequence read = deck::readAnimations(*back->pages[0]);
        QCOMPARE(read.effects.size(), 3);
        QVERIFY(!read.effects[0].custom() && !read.effects[1].custom() && !read.effects[2].custom());
        // Each still aims at the shape it did, by name.
        auto nameOf = [&](const Document &doc, int shape) {
            QString name;
            walkItems(doc.pages[0]->items, [&](const ItemPtr &it) {
                if (it->sourceId == shape) name = it->name;
            });
            return name;
        };
        QCOMPARE(nameOf(*back, read.effects[0].shape), QStringLiteral("Box 1"));
        QCOMPARE(nameOf(*back, read.effects[1].shape), QStringLiteral("Fresh"));
        QCOMPARE(nameOf(*back, read.effects[2].shape), QStringLiteral("Box 2"));
        // Delete "Box 1": its effect is not written, the others are, and the file says so.
        for (auto it = slide.items.begin(); it != slide.items.end(); ++it)
            if ((*it)->name == QLatin1String("Box 1")) {
                slide.items.erase(it);
                break;
            }
        skipped.clear();
        bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        read = deck::readAnimations(*back->pages[0]);
        QCOMPARE(read.effects.size(), 2);
        QCOMPARE(nameOf(*back, read.effects[0].shape), QStringLiteral("Fresh"));
        QCOMPARE(nameOf(*back, read.effects[1].shape), QStringLiteral("Box 2"));
        QVERIFY(!read.effects[0].custom() && read.effects[0].kind == QLatin1String("entr.fly"));
        bool noted = false;
        for (const QString &s : std::as_const(skipped)) noted |= s.startsWith(QLatin1String("Slide 1")) && s.contains(QStringLiteral("animation"));
        QVERIFY2(noted, qPrintable(skipped.join('\n')));
    }

    // The Animations tab: between Transitions and Slide Show, with its gallery, Effect Options, Add Animation, the pane, the
    // painter, and the timing controls; the commands are steps that undo, and the pane lists the effects.
    void windowAnimationsTab()
    {
        DeckWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        Ribbon *r = w.ribbon();
        int transitions = -1, animations = -1, show = -1;
        for (int i = 0; i < r->tabCount(); ++i) {
            if (r->tabName(i) == QLatin1String("Transitions")) transitions = i;
            if (r->tabName(i) == QLatin1String("Animations")) animations = i;
            if (r->tabName(i) == QLatin1String("Slide Show")) show = i;
        }
        QVERIFY(animations > 0);
        QCOMPARE(animations, transitions + 1);
        QCOMPARE(show, animations + 1);
        for (const char *id : {"anim.preview", "anim.pane", "anim.painter", "anim.earlier", "anim.later", "anim.none", "anim.options", "anim.timing", "anim.remove", "anim.set.entr.fade", "anim.add.exit.fly"})
            QVERIFY2(w.act(QString::fromLatin1(id)), id);
        QVERIFY(r->findChild<Gallery *>(QStringLiteral("animationGallery")));
        Editor *ed = w.editor();
        auto slidePage = [ed]() -> Page & { return *ed->doc()->pages[0]; };
        ed->doc()->storyDoc(static_cast<TextItem *>(slidePage().items[1].get())->storyId)->setPlainText(QStringLiteral("One\nTwo\nThree"));
        QVERIFY(!slidePage().props.contains(QStringLiteral("timingXml")));
        QVERIFY(!w.act("anim.preview")->isEnabled());
        // Nothing selected, nothing happens.
        w.act("anim.set.entr.fade")->trigger();
        QVERIFY(!slidePage().props.contains(QStringLiteral("timingXml")));
        // A shape selected: the effect is made, on a click, and undoes.
        ed->select(slidePage().items[0]->id);
        w.act("anim.set.entr.fade")->trigger();
        deck::AnimSequence seq = deck::readAnimations(slidePage());
        QCOMPARE(seq.effects.size(), 1);
        QCOMPARE(seq.effects[0].kind, QStringLiteral("entr.fade"));
        QCOMPARE(seq.effects[0].shape, slidePage().items[0]->sourceId);
        QVERIFY(slidePage().items[0]->sourceId > 0);
        QVERIFY(w.act("anim.preview")->isEnabled());
        auto *pane = w.animationPane();
        QVERIFY(pane);
        QCOMPARE(pane->rowCount(), 1);
        QVERIFY(pane->list()->item(0)->text().contains(QStringLiteral("Fade")));
        // Another effect on the same shape replaces the first; Add Animation adds one.
        w.act("anim.set.entr.fly")->trigger();
        seq = deck::readAnimations(slidePage());
        QCOMPARE(seq.effects.size(), 1);
        QCOMPARE(seq.effects[0].kind, QStringLiteral("entr.fly"));
        w.act("anim.add.emph.spin")->trigger();
        seq = deck::readAnimations(slidePage());
        QCOMPARE(seq.effects.size(), 2);
        QCOMPARE(seq.effects[1].kind, QStringLiteral("emph.spin"));
        QCOMPARE(pane->rowCount(), 2);
        // The timing controls follow the effect picked.
        auto *start = r->findChild<QComboBox *>(QStringLiteral("animStartBox"));
        auto *duration = r->findChild<QDoubleSpinBox *>(QStringLiteral("animDuration"));
        auto *delay = r->findChild<QDoubleSpinBox *>(QStringLiteral("animDelay"));
        QVERIFY(start && duration && delay);
        pane->list()->setCurrentRow(1, QItemSelectionModel::ClearAndSelect);
        QCOMPARE(int(ed->selection().size()), 1);   // the row's shape is selected on the slide
        QCOMPARE(duration->value(), 2.0);           // the spin takes 2 seconds
        QCOMPARE(start->currentData().toInt(), int(deck::EffectStart::OnClick));
        duration->setValue(1.25);
        delay->setValue(0.5);
        start->setCurrentIndex(start->findData(int(deck::EffectStart::AfterPrevious)));
        Q_EMIT start->activated(start->currentIndex());
        seq = deck::readAnimations(slidePage());
        QCOMPARE(seq.effects[1].duration, 1250);
        QCOMPARE(seq.effects[1].delay, 500);
        QCOMPARE(seq.effects[1].start, deck::EffectStart::AfterPrevious);
        QCOMPARE(seq.effects[0].duration, 500);   // the other is as it was
        // Each was a step.
        ed->undo();
        QCOMPARE(deck::readAnimations(slidePage()).effects[1].start, deck::EffectStart::OnClick);
        ed->undo();
        QCOMPARE(deck::readAnimations(slidePage()).effects[1].delay, 0);
        ed->undo();
        QCOMPARE(deck::readAnimations(slidePage()).effects[1].duration, 2000);
        // Re-Order from the pane: the picked effect moves earlier, and later.
        pane->list()->setCurrentRow(1, QItemSelectionModel::ClearAndSelect);
        Q_EMIT pane->reorderRequested(-1);
        seq = deck::readAnimations(slidePage());
        QCOMPARE(seq.effects[0].kind, QStringLiteral("emph.spin"));
        w.act("anim.later")->trigger();
        QCOMPARE(deck::readAnimations(slidePage()).effects[1].kind, QStringLiteral("emph.spin"));
        // Remove, and None.
        Q_EMIT pane->removeRequested({1});
        QCOMPARE(int(deck::readAnimations(slidePage()).effects.size()), 1);
        w.act("anim.none")->trigger();
        QVERIFY(!slidePage().props.contains(QStringLiteral("timingXml")));
    }

    // Effect Options lists what the effect picked can do: its directions, its amounts, and by paragraph for text; each choice is a step.
    void windowAnimationEffectOptions()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        auto slidePage = [ed]() -> Page & { return *ed->doc()->pages[0]; };
        ed->doc()->storyDoc(static_cast<TextItem *>(slidePage().items[1].get())->storyId)->setPlainText(QStringLiteral("One\nTwo\nThree"));
        QMenu *menu = nullptr;
        for (QToolButton *b : w.ribbon()->findChildren<QToolButton *>())
            if (b->defaultAction() && b->defaultAction()->objectName() == QLatin1String("ribbon.animEffectOptions") && b->menu()) menu = b->menu();
        QVERIFY(menu);
        auto texts = [&] {
            Q_EMIT menu->aboutToShow();
            QStringList out;
            for (QAction *a : menu->actions()) out << a->text();
            return out;
        };
        QCOMPARE(texts(), (QStringList{QStringLiteral("No Effect Options")}));
        ed->select(slidePage().items[1]->id);
        w.act("anim.set.entr.fly")->trigger();
        const QStringList fly = texts();
        QVERIFY(fly.contains(QStringLiteral("From Left")) && fly.contains(QStringLiteral("From Bottom-Right")) && fly.contains(QStringLiteral("By Paragraph")));
        auto choose = [&](const QString &text) {
            Q_EMIT menu->aboutToShow();
            for (QAction *a : menu->actions())
                if (a->text() == text) {
                    a->trigger();
                    return true;
                }
            return false;
        };
        QVERIFY(choose(QStringLiteral("From Left")));
        QCOMPARE(deck::readAnimations(slidePage()).effects[0].variant, QStringLiteral("left"));
        QVERIFY(choose(QStringLiteral("By Paragraph")));
        QCOMPARE(int(deck::readAnimations(slidePage()).effects.size()), 3);   // a paragraph each
        QCOMPARE(deck::readAnimations(slidePage()).effects[2].variant, QStringLiteral("left"));
        QVERIFY(choose(QStringLiteral("From Top")));
        for (const deck::Anim &a : deck::readAnimations(slidePage()).effects) QCOMPARE(a.variant, QStringLiteral("top"));   // the build changes together
        QVERIFY(choose(QStringLiteral("As One Object")));
        QCOMPARE(int(deck::readAnimations(slidePage()).effects.size()), 1);
        // Amounts: grow/shrink and spin.
        w.act("anim.set.emph.growshrink")->trigger();
        QVERIFY(texts().contains(QStringLiteral("Huge")));
        QVERIFY(choose(QStringLiteral("Huge")));
        QCOMPARE(deck::readAnimations(slidePage()).effects[0].amount, 400.0);
        QVERIFY(choose(QStringLiteral("Horizontal")));
        QCOMPARE(deck::readAnimations(slidePage()).effects[0].variant, QStringLiteral("horizontal"));
        w.act("anim.set.emph.spin")->trigger();
        QVERIFY(choose(QStringLiteral("Half Spin")));
        QCOMPARE(deck::readAnimations(slidePage()).effects[0].amount, 180.0);
        QVERIFY(choose(QStringLiteral("Counterclockwise")));
        QCOMPARE(deck::readAnimations(slidePage()).effects[0].variant, QStringLiteral("ccw"));
        // A motion path: its paths.
        w.act("anim.set.path.shapes")->trigger();
        QVERIFY(texts().contains(QStringLiteral("5 Point Star")));
        QVERIFY(choose(QStringLiteral("5 Point Star")));
        QCOMPARE(deck::readAnimations(slidePage()).effects[0].variant, QStringLiteral("p5"));
    }

    // The dialog for one effect shows its settings and gives them back.
    void animationOptionsDialogReadsAndGives()
    {
        deck::Anim a = deck::makeAnim(QStringLiteral("entr.fly"), 3);
        a.delay = 500;
        a.duration = 1500;
        a.smoothStart = true;
        a.repeat = 3;
        AnimationOptionsDialog dlg(a, true, AnimationOptionsDialog::TimingTab);
        QCOMPARE(dlg.findChild<QTabWidget *>()->currentIndex(), int(AnimationOptionsDialog::TimingTab));
        QCOMPARE(dlg.findChild<QDoubleSpinBox *>(QStringLiteral("animDelay"))->value(), 0.5);
        QCOMPARE(dlg.findChild<QDoubleSpinBox *>(QStringLiteral("animDuration"))->value(), 1.5);
        QVERIFY(dlg.findChild<QCheckBox *>(QStringLiteral("animSmoothStart"))->isChecked());
        QVERIFY(!dlg.findChild<QCheckBox *>(QStringLiteral("animSmoothEnd"))->isChecked());
        QCOMPARE(dlg.findChild<QComboBox *>(QStringLiteral("animRepeat"))->currentData().toInt(), 3);
        QVERIFY(!dlg.byParagraph());
        auto *variant = dlg.findChild<QComboBox *>(QStringLiteral("animVariant"));
        variant->setCurrentIndex(variant->findData(QStringLiteral("right")));
        dlg.findChild<QDoubleSpinBox *>(QStringLiteral("animDuration"))->setValue(2.25);
        dlg.findChild<QCheckBox *>(QStringLiteral("animRewind"))->setChecked(true);
        dlg.findChild<QRadioButton *>(QStringLiteral("animByParagraph"))->setChecked(true);
        const deck::Anim r = dlg.result();
        QCOMPARE(r.variant, QStringLiteral("right"));
        QCOMPARE(r.duration, 2250);
        QCOMPARE(r.delay, 500);
        QVERIFY(r.rewind && r.smoothStart);
        QVERIFY(dlg.byParagraph());
        // An appearing shape has no length to set, and an effect from another program offers only how it starts.
        AnimationOptionsDialog d2(deck::makeAnim(QStringLiteral("entr.appear"), 3), false, AnimationOptionsDialog::EffectTab);
        QVERIFY(!d2.findChild<QDoubleSpinBox *>(QStringLiteral("animDuration"))->isEnabled());
        QVERIFY(!d2.findChild<QRadioButton *>(QStringLiteral("animByParagraph"))->isEnabled());
        deck::Anim custom;
        custom.cls = deck::EffectClass::Entrance;
        custom.shape = 3;
        custom.start = deck::EffectStart::WithPrevious;
        AnimationOptionsDialog d3(custom, true, AnimationOptionsDialog::TimingTab);
        QVERIFY(!d3.findChild<QDoubleSpinBox *>(QStringLiteral("animDelay"))->isEnabled());
        QCOMPARE(d3.result().start, deck::EffectStart::WithPrevious);
    }

    // Preview plays the slide's effects over the slide and hides itself when they are over; the numbers sit by the shapes while
    // the tab or the pane shows, and not otherwise.
    void windowAnimationPreviewAndNumbers()
    {
        DeckWindow w;
        w.resize(1300, 800);
        w.show();
        Editor *ed = w.editor();
        auto slidePage = [ed]() -> Page & { return *ed->doc()->pages[0]; };
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        ed->select(slidePage().items[0]->id);
        w.act("anim.set.entr.fade")->trigger();
        ed->select(slidePage().items[1]->id);
        w.act("anim.set.entr.fly")->trigger();
        QCOMPARE(int(w.canvas()->animationBadges().size()), 0);
        w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Animations")));
        QCOMPARE(int(w.canvas()->animationBadges().size()), 2);
        QCOMPARE(w.canvas()->animationBadges()[1].text, QStringLiteral("2"));
        w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Home")));
        QCOMPARE(int(w.canvas()->animationBadges().size()), 0);
        w.showAnimationPane(true);
        QVERIFY(w.act("anim.pane")->isChecked());
        QTRY_VERIFY(w.animationPane()->isVisible());
        QCOMPARE(int(w.canvas()->animationBadges().size()), 2);   // the pane shows them too
        w.showAnimationPane(false);
        QVERIFY(!w.act("anim.pane")->isChecked());
        QCOMPARE(int(w.canvas()->animationBadges().size()), 0);
        w.act("anim.preview")->trigger();
        auto *preview = w.canvas()->findChild<AnimationPreview *>(QStringLiteral("animationPreview"));
        QVERIFY(preview);
        QVERIFY(preview->isPlaying());
        QTRY_VERIFY2_WITH_TIMEOUT(!w.canvas()->findChild<AnimationPreview *>(QStringLiteral("animationPreview")), "the preview ends", 8000);
    }

    // What an effect does to its shape when it is over: hide it when the effect ends, or at the next click. It is written as the
    // presentation program writes it, reads back, and plays that way in the show.
    void animationEditHidesAfterAnEffect()
    {
        auto d = editDeck(2);
        Page &slide = *d->pages[0];
        deck::AnimSequence seq;
        deck::Anim a = deck::makeAnim(QStringLiteral("entr.fade"), 10);
        a.after = deck::AfterEffect::Hide;
        deck::Anim b = deck::makeAnim(QStringLiteral("emph.spin"), 11);
        b.after = deck::AfterEffect::HideOnNextClick;
        seq.add(a);
        seq.add(b);
        seq.add(deck::makeAnim(QStringLiteral("entr.appear"), 3));
        deck::setAnimations(*d, slide, seq);
        const QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
        // The first effect's time node is 5; its hiding ends with it.
        QVERIFY(xml.contains(QStringLiteral("<p:subTnLst><p:set><p:cBhvr override=\"childStyle\"><p:cTn dur=\"1\" fill=\"hold\" display=\"0\" masterRel=\"sameClick\" afterEffect=\"1\">"
                                            "<p:stCondLst><p:cond evt=\"end\" delay=\"0\"><p:tn val=\"5\"/></p:cond></p:stCondLst></p:cTn>")));
        QVERIFY(xml.contains(QStringLiteral("masterRel=\"nextClick\" afterEffect=\"1\"/>")));
        const deck::AnimSequence back = deck::readAnimations(slide);
        QCOMPARE(int(back.effects.size()), 3);
        QVERIFY(!back.effects[0].custom() && !back.effects[1].custom());
        QCOMPARE(back.effects[0].after, deck::AfterEffect::Hide);
        QCOMPARE(back.effects[1].after, deck::AfterEffect::HideOnNextClick);
        QCOMPARE(back.effects[2].after, deck::AfterEffect::None);
        // Played: gone after its effect, and the second at the click after it.
        const deck::Timeline tl = deck::playableTimeline(slide);
        const deck::Geometry g = deck::geometryOf(*d, slide);
        auto visible = [&](int done, double ms, int shape) { return deck::statesAt(tl, g, done, ms).value(deck::Target{shape}).visible; };
        QVERIFY(visible(0, 250, 10));     // while its fade plays, it is there
        QVERIFY(!visible(1, -1, 10));     // and hidden when it ends
        QVERIFY(visible(2, -1, 11));      // the spin is over, and the shape stays...
        QVERIFY(!visible(3, -1, 11));     // ...until the next click: the step after it hides it
        QVERIFY(visible(3, -1, 3));       // as the third step shows its own shape
        QCOMPARE(int(tl.steps.size()), 3);
        // Changing the kind of an effect keeps what it does when it is over, unless the kind is an exit.
        deck::Anim c = a;
        deck::changeKind(c, *deck::effectKind(QStringLiteral("entr.wipe")));
        QCOMPARE(c.after, deck::AfterEffect::Hide);
        deck::changeKind(c, *deck::effectKind(QStringLiteral("exit.wipe")));
        QCOMPARE(c.after, deck::AfterEffect::None);
        // The dialog gives it back.
        AnimationOptionsDialog dlg(a, false, AnimationOptionsDialog::EffectTab);
        auto *after = dlg.findChild<QComboBox *>(QStringLiteral("animAfter"));
        QVERIFY(after && after->isEnabled());
        QCOMPARE(after->currentData().toInt(), int(deck::AfterEffect::Hide));
        after->setCurrentIndex(after->findData(int(deck::AfterEffect::HideOnNextClick)));
        QCOMPARE(dlg.result().after, deck::AfterEffect::HideOnNextClick);
        AnimationOptionsDialog exit(deck::makeAnim(QStringLiteral("exit.fade"), 3), false, AnimationOptionsDialog::EffectTab);
        QVERIFY(!exit.findChild<QComboBox *>(QStringLiteral("animAfter"))->isEnabled());
    }

    // A timing a file makes hostile is read within limits and never rewritten: one with more effects than a slide could have, and
    // one that nests its effects deeper than any program does, are shown as far as they can be and not changed.
    void animationEditReadsHostileTiming()
    {
        auto d = editDeck();
        Page &slide = *d->pages[0];
        QStringList many;
        for (int i = 0; i < 2600; ++i) many << animFadeIn(3, 100);
        slide.props[QStringLiteral("timingXml")] = animTiming(animSteps(many));
        deck::AnimSequence seq;
        const qint64 readBefore = deck::animationEffectsReadForTest();
        seq = deck::readAnimations(slide);
        // Counted: the effects are read as far as a sequence holds (2,000), not all 2,600, and once.
        QCOMPARE(deck::animationEffectsReadForTest() - readBefore, qint64(2000));
        QVERIFY(!seq.editable);
        QCOMPARE(int(seq.effects.size()), 2000);
        // Nested thirty deep, the effect is not found by the editing reader, which then does not rewrite the timing.
        QString nested = animFadeIn(3, 100);
        for (int i = 0; i < 30; ++i) nested = animGroup(0, {nested});
        slide.props[QStringLiteral("timingXml")] = animTiming(animStep({nested}));
        seq = deck::readAnimations(slide);
        QVERIFY(!seq.editable);
        // Garbage is nothing, and a timing with no sequence is none.
        slide.props[QStringLiteral("timingXml")] = QStringLiteral("<p:timing xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:tnLst/></p:timing>");
        QVERIFY(deck::readAnimations(slide).effects.isEmpty());
        QVERIFY(deck::readAnimations(slide).editable);
        slide.props[QStringLiteral("timingXml")] = QStringLiteral("<p:timing");
        QVERIFY(deck::readAnimations(slide).effects.isEmpty());
        QVERIFY(!deck::readAnimations(slide).editable);   // never an empty sequence that an edit would write over it
    }

    static const QString animMainNs() { return QStringLiteral("http://schemas.openxmlformats.org/presentationml/2006/main"); }

    // A timing of one click step holding the effects (written as XML text), the way a file lays it out; the ids come last in each time node, which a file may do.
    static QString animBareTiming(const QString &effects, const QString &around = QString(), const QString &before = QString(), const QString &after = QString())
    {
        return animXml("<p:timing xmlns:p='http://schemas.openxmlformats.org/presentationml/2006/main'><p:tnLst><p:par><p:cTn dur='indefinite' restart='never' nodeType='tmRoot' id='1'><p:childTnLst>")
            + before + animXml("<p:seq concurrent='1' nextAc='seek'><p:cTn dur='indefinite' nodeType='mainSeq' id='2'><p:childTnLst><p:par><p:cTn fill='hold' id='3'><p:stCondLst><p:cond delay='indefinite'/></p:stCondLst>"
                               "<p:childTnLst><p:par><p:cTn fill='hold' id='4'><p:stCondLst><p:cond delay='0'/></p:stCondLst><p:childTnLst>")
            + effects + animXml("</p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:seq>") + after
            + animXml("</p:childTnLst></p:cTn></p:par></p:tnLst>") + around + animXml("</p:timing>");
    }

    // An effect JeffDeck has no kind for (preset 77), on shape `spid`, its time nodes numbered from `id`, the id the last attribute of each.
    static QString animUnknownEffect(int spid, int id)
    {
        return animXml("<p:par><p:cTn presetID='77' presetClass='entr' presetSubtype='0' fill='hold' grpId='0' nodeType='clickEffect' id='%1'><p:stCondLst><p:cond delay='0'/></p:stCondLst><p:childTnLst>"
                       "<p:set><p:cBhvr><p:cTn dur='1' fill='hold' id='%2'><p:stCondLst><p:cond delay='0'/></p:stCondLst></p:cTn><p:tgtEl><p:spTgt spid='%4'/></p:tgtEl><p:attrNameLst><p:attrName>style.visibility</p:attrName></p:attrNameLst></p:cBhvr><p:to><p:strVal val='visible'/></p:to></p:set>"
                       "<p:animEffect transition='in' filter='wheel(1)'><p:cBhvr><p:cTn dur='2000' id='%3'/><p:tgtEl><p:spTgt spid='%4'/></p:tgtEl></p:cBhvr></p:animEffect>"
                       "</p:childTnLst></p:cTn></p:par>").arg(id).arg(id + 1).arg(id + 2).arg(spid);
    }

    // Writing a slide's animations back gives each time node an id of its own, a kept effect's too, whatever order a file wrote its attributes in.
    void animationEditNumbersTimeNodesOnce()
    {
        auto d = editDeck(2);
        Page &slide = *d->pages[0];
        slide.props[QStringLiteral("timingXml")] = animBareTiming(animUnknownEffect(10, 5));
        QVERIFY2(animIdProblem(slide.props.value(QStringLiteral("timingXml")).toString()).isEmpty(), "the timing a file has is sound");
        deck::AnimSequence seq = deck::readAnimations(slide);
        QCOMPARE(int(seq.effects.size()), 1);
        QVERIFY(seq.effects[0].custom());
        // Written again as it is, and with an effect added to it: the ids are all different.
        deck::setAnimations(*d, slide, seq);
        QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY2(animIdProblem(xml).isEmpty(), qPrintable(animIdProblem(xml)));
        QVERIFY(xml.contains(QStringLiteral("presetID=\"77\"")) && xml.contains(QStringLiteral("filter=\"wheel(1)\"")));
        seq.add(deck::makeAnim(QStringLiteral("entr.fade"), 11));
        seq.add(deck::makeAnim(QStringLiteral("entr.fly"), 3));
        deck::setAnimations(*d, slide, seq);
        xml = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY2(animIdProblem(xml).isEmpty(), qPrintable(animIdProblem(xml)));
        QCOMPARE(int(deck::readAnimations(slide).effects.size()), 3);
        // Two kept effects, one after the other, and a start that refers to a time node of its own.
        QString second = animUnknownEffect(11, 5).replace(QStringLiteral("<p:cond delay=\"0\"/></p:stCondLst><p:childTnLst><p:set>"),
                                                          QStringLiteral("<p:cond evt=\"onBegin\" delay=\"0\"><p:tn val=\"6\"/></p:cond></p:stCondLst><p:childTnLst><p:set>"));
        slide.props[QStringLiteral("timingXml")] = animBareTiming(animUnknownEffect(10, 5) + second.replace(QStringLiteral("id=\"5\""), QStringLiteral("id=\"8\"")).replace(QStringLiteral("id=\"6\""), QStringLiteral("id=\"9\""))
                                                                  .replace(QStringLiteral("id=\"7\""), QStringLiteral("id=\"10\"")).replace(QStringLiteral("<p:tn val=\"6\"/>"), QStringLiteral("<p:tn val=\"9\"/>")));
        QVERIFY2(animIdProblem(slide.props.value(QStringLiteral("timingXml")).toString()).isEmpty(), qPrintable(animIdProblem(slide.props.value(QStringLiteral("timingXml")).toString())));
        seq = deck::readAnimations(slide);
        QCOMPARE(int(seq.effects.size()), 2);
        deck::setAnimations(*d, slide, seq);
        xml = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY2(animIdProblem(xml).isEmpty(), qPrintable(animIdProblem(xml)));   // the reference follows its node to the new number
    }

    // What a timing holds beside its main sequence (a movie's or sound's node, a sequence that waits for a click) is written back
    // just as it was, in its place, with its own ids; the main sequence takes the numbers it leaves.
    void animationEditKeepsMediaNodesAsWritten()
    {
        auto d = editDeck(2);
        Page &slide = *d->pages[0];
        const QString video = QStringLiteral("<p:video><p:cMediaNode vol=\"80000\" numSld=\"1\" showWhenStopped=\"0\"><p:cTn id=\"9\" fill=\"hold\" display=\"0\"><p:stCondLst><p:cond delay=\"indefinite\"/>"
                                             "<p:cond evt=\"onBegin\" delay=\"0\"><p:tn val=\"2\"/></p:cond></p:stCondLst><p:endCondLst><p:cond evt=\"onStopAudio\" delay=\"0\"><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:endCondLst></p:cTn>"
                                             "<p:tgtEl><p:spTgt spid=\"10\"/></p:tgtEl></p:cMediaNode></p:video>");
        const QString audio = QString::fromUtf8("<p:audio isNarration=\"1\" descr=\"caf\u00E9 \u2603 &amp; \U0001F600 &#x41;\"><p:cMediaNode vol=\"80000\" numSld=\"3\" showWhenStopped=\"0\"><p:cTn id=\"6\" repeatCount=\"indefinite\" fill=\"hold\" display=\"0\">"
                                                "<p:stCondLst><p:cond delay=\"indefinite\"/></p:stCondLst></p:cTn><p:tgtEl><p:spTgt spid=\"11\"/></p:tgtEl></p:cMediaNode></p:audio>");
        slide.props[QStringLiteral("timingXml")] = animBareTiming(animFadeIn(3, 500), QString(), video, audio);
        deck::AnimSequence seq = deck::readAnimations(slide);
        QVERIFY(seq.editable);
        QCOMPARE(int(seq.effects.size()), 1);
        QCOMPARE(int(seq.kept.size()), 2);
        seq.add(deck::makeAnim(QStringLiteral("entr.fly"), 10));
        deck::setAnimations(*d, slide, seq);
        const QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY(xml.contains(video));   // byte for byte
        QVERIFY(xml.contains(audio));
        const qsizetype v = xml.indexOf(video), m = xml.indexOf(QStringLiteral("nodeType=\"mainSeq\"")), a = xml.indexOf(audio);
        QVERIFY(v > 0 && m > v && a > m);   // in the places they had
        QVERIFY2(animIdProblem(xml).isEmpty(), qPrintable(animIdProblem(xml)));
        QCOMPARE(int(deck::readAnimations(slide).effects.size()), 2);
        // When the file is saved with some effects dropped for shapes that are gone, they stay too.
        int dropped = 0;
        const QString saved = deck::timingForSave(*d, slide, [](int shape) { return shape != 3; }, &dropped);
        QCOMPARE(dropped, 1);
        QVERIFY(saved.contains(video) && saved.contains(audio));
        QVERIFY2(animIdProblem(saved).isEmpty(), qPrintable(animIdProblem(saved)));
    }

    // A frame the slide has but JeffDeck has no object for (a diagram kept as written) and a table are shapes an effect can be on; an
    // edit keeps their effects, and a chart or a table read from a file carries the number the file gave it.
    void animationEditKeepsEffectsOnFramesAndTables()
    {
        auto d = editDeck(1);
        Page &slide = *d->pages[0];
        auto table = std::make_shared<TableItem>();
        table->sourceId = 14;
        table->rect = QRectF(300, 100, 200, 80);
        slide.items.push_back(table);
        QJsonArray unread;
        unread.append(QJsonObject{{"xml", QStringLiteral("<p:graphicFrame xmlns:p=\"%1\"><p:nvGraphicFramePr><p:cNvPr id=\"8\" name=\"Diagram\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr></p:graphicFrame>").arg(animMainNs())}});
        slide.props[QStringLiteral("unread")] = unread;
        deck::AnimSequence seq;
        seq.add(deck::makeAnim(QStringLiteral("entr.fade"), 14));
        seq.add(deck::makeAnim(QStringLiteral("entr.wipe"), 8));
        seq.add(deck::makeAnim(QStringLiteral("entr.fly"), 10));
        deck::setAnimations(*d, slide, seq);
        QCOMPARE(int(deck::readAnimations(slide).effects.size()), 3);
        // Another edit keeps them.
        seq = deck::readAnimations(slide);
        seq.add(deck::makeAnim(QStringLiteral("emph.spin"), 14));
        deck::setAnimations(*d, slide, seq);
        QCOMPARE(int(deck::readAnimations(slide).effects.size()), 4);
        // A number no frame, table, or other shape has is a shape that is gone.
        deck::AnimSequence gone = deck::readAnimations(slide);
        gone.add(deck::makeAnim(QStringLiteral("entr.fade"), 99));
        deck::setAnimations(*d, slide, gone);
        QCOMPARE(int(deck::readAnimations(slide).effects.size()), 4);
    }

    // A table and a chart read from a file are given the number the file gave their frame, so that an effect on them finds them.
    void importGivesTablesAndChartsTheirFrameNumbers()
    {
        QString err;
        auto d = deck::importPresentation(pack(chartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const auto *pic = chartPicture(*d, *d->pages[0]);
        QVERIFY(pic);
        QCOMPARE(pic->sourceId, 9);
        const QString table =
            QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"8\" name=\"Grid\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                           "<p:xfrm><a:off x=\"914400\" y=\"914400\"/><a:ext cx=\"914400\" cy=\"254000\"/></p:xfrm>"
                           "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/table\"><a:tbl><a:tblPr/><a:tblGrid><a:gridCol w=\"914400\"/></a:tblGrid>"
                           "<a:tr h=\"254000\"><a:tc><a:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\"/><a:t>x</a:t></a:r></a:p></a:txBody><a:tcPr/></a:tc></a:tr>"
                           "</a:tbl></a:graphicData></a:graphic></p:graphicFrame>");
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        xml.replace(unknownFrame(8), table);
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        auto withTable = deck::importPresentation(pack(p), &err);
        QVERIFY2(withTable, qPrintable(err));
        const TableItem *t = nullptr;
        for (const auto &it : withTable->pages[0]->items)
            if (it->type() == ItemType::Table) t = static_cast<const TableItem *>(it.get());
        QVERIFY(t);
        QCOMPARE(t->sourceId, 8);
    }

    // Effects on a chart and on a frame kept as written are saved with the file, and read back from it.
    void animationEditSavesEffectsOnChartsAndFrames()
    {
        QString err;
        auto d = deck::importPresentation(pack(chartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        Page &slide = *d->pages[0];
        deck::AnimSequence seq;
        seq.add(deck::makeAnim(QStringLiteral("entr.fade"), 9));    // the chart
        seq.add(deck::makeAnim(QStringLiteral("entr.wipe"), 8));    // the frame kept as written
        deck::setAnimations(*d, slide, seq);
        QCOMPARE(int(deck::readAnimations(slide).effects.size()), 2);
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const deck::AnimSequence read = deck::readAnimations(*back->pages[0]);
        QCOMPARE(int(read.effects.size()), 2);
        QVERIFY(!read.effects[0].custom() && !read.effects[1].custom());
    }

    // What an effect does to its shape afterwards takes no part in the effect's own time: hide after a fade that plays three times is
    // at the end of the third, and a spin that goes back when it ends still hides its shape.
    void animationEditHidesAfterTheEffectEnds()
    {
        auto d = editDeck(2);
        Page &slide = *d->pages[0];
        deck::AnimSequence seq;
        deck::Anim fade = deck::makeAnim(QStringLiteral("entr.fade"), 10);
        fade.repeat = 3;
        fade.after = deck::AfterEffect::Hide;
        seq.add(fade);
        deck::setAnimations(*d, slide, seq);
        deck::Timeline tl = deck::playableTimeline(slide);
        const deck::Geometry g = deck::geometryOf(*d, slide);
        auto visible = [&](int done, double ms, int shape) { return deck::statesAt(tl, g, done, ms).value(deck::Target{shape}).visible; };
        QVERIFY(std::abs(tl.steps[0].duration() - 1500) < 5);
        QVERIFY(visible(0, 1400, 10));
        QVERIFY(!visible(0, 1600, 10));
        QVERIFY(!visible(1, -1, 10));
        // A spin that rewinds, and hides.
        deck::AnimSequence spin;
        deck::Anim a = deck::makeAnim(QStringLiteral("emph.spin"), 11);
        a.duration = 1000;
        a.rewind = true;
        a.after = deck::AfterEffect::Hide;
        spin.add(a);
        deck::setAnimations(*d, slide, spin);
        tl = deck::playableTimeline(slide);
        QVERIFY(visible(0, 500, 11));
        QVERIFY(!visible(1, -1, 11));
        QCOMPARE(deck::statesAt(tl, g, 1, -1).value(deck::Target{11}).rotation, 0.0);   // and it is as it was
    }

    // A repeat count a file makes huge does not make a step that long: an effect plays its repeats for at most twenty seconds.
    void animationEditLimitsHugeRepeats()
    {
        auto d = editDeck(1);
        Page &slide = *d->pages[0];
        deck::AnimSequence seq;
        deck::Anim a = deck::makeAnim(QStringLiteral("emph.spin"), 10);
        a.duration = 500;
        a.repeat = 9999;
        seq.add(a);
        deck::setAnimations(*d, slide, seq);
        QCOMPARE(deck::readAnimations(slide).effects[0].repeat, 9999.0);   // the file keeps what it says
        const deck::Timeline tl = deck::playableTimeline(slide);
        QVERIFY2(tl.steps[0].duration() <= 20500, qPrintable(QString::number(tl.steps[0].duration())));
        QVERIFY(tl.steps[0].duration() >= 19000);
        // "Until the end" plays once.
        QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
        xml.replace(QRegularExpression(QStringLiteral("repeatCount=\"\\d+\"")), QStringLiteral("repeatCount=\"indefinite\""));
        slide.props[QStringLiteral("timingXml")] = xml;
        QVERIFY(deck::playableTimeline(slide).steps[0].duration() < 600);
    }

    // A new shape does not take the number of one that was deleted, or of any shape the timing names, in its effects, its kept
    // sequences, or its builds.
    void animationEditNumbersNewShapesPastGoneOnes()
    {
        auto d = editDeck(1);
        Page &slide = *d->pages[0];
        slide.props[QStringLiteral("timingXml")] = animBareTiming(animFadeIn(10, 300) + animFadeIn(20, 300), animXml("<p:bldLst><p:bldP spid='23' grpId='0'/></p:bldLst>"));
        auto fresh = std::make_shared<ShapeItem>();
        fresh->rect = QRectF(400, 100, 80, 40);
        slide.items.push_back(fresh);
        const int number = deck::shapeNumber(slide, fresh->id);
        QVERIFY2(number > 23, qPrintable(QString::number(number)));
    }

    // A shape that is deleted has no effects in the pane, or in the show, and the next change to the slide's animations leaves them
    // out; undoing the deletion brings them back.
    void windowAnimationsFollowDeletedShapes()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        auto page = [ed]() -> Page & { return *ed->doc()->pages[0]; };
        ed->select(page().items[0]->id);
        w.act("anim.set.entr.fade")->trigger();
        ed->select(page().items[1]->id);
        w.act("anim.set.entr.fly")->trigger();
        QCOMPARE(w.animationPane()->rowCount(), 2);
        const QString bodyName = page().items[1]->name;
        ed->select(page().items[1]->id);
        ed->deleteSelection();
        QCOMPARE(w.animationPane()->rowCount(), 1);
        QCOMPARE(int(deck::readAnimations(page(), ed->doc()).effects.size()), 1);
        QCOMPARE(int(deck::playableTimeline(*ed->doc(), page()).steps.size()), 1);   // no click for the shape that is gone
        QCOMPARE(int(deck::playableTimeline(page()).steps.size()), 2);
        ed->undo();
        QCOMPARE(w.animationPane()->rowCount(), 2);
        // Deleted again, a change to the animations takes the stale effect out of the file.
        ed->select(page().items[1]->id);
        ed->deleteSelection();
        ed->select(page().items[0]->id);
        w.act("anim.add.emph.spin")->trigger();
        QCOMPARE(int(deck::readAnimations(page()).effects.size()), 2);
        QCOMPARE(w.animationPane()->rowCount(), 2);
    }

    // The rows picked in the animation pane are effects, not places in the list: when an effect above them goes, the same effects stay picked, and
    // a pick is not carried over to another slide's list.
    void windowAnimationPaneKeepsTheSameEffectsPickedWhenRowsMove()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        auto page = [ed]() -> Page & { return *ed->doc()->pages[0]; };
        ed->select(page().items[0]->id);
        w.act("anim.set.entr.fade")->trigger();
        ed->select(page().items[1]->id);
        w.act("anim.set.entr.fly")->trigger();
        ed->select(page().items[0]->id);
        w.act("anim.add.emph.spin")->trigger();
        ed->select(page().items[1]->id);
        w.act("anim.add.emph.spin")->trigger();
        ed->clearSelection();
        auto *pane = w.animationPane();
        QCOMPARE(pane->rowCount(), 4);
        pane->list()->setCurrentRow(2, QItemSelectionModel::ClearAndSelect);
        QCOMPARE(pane->selectedRows(), QVector<int>({2}));
        // The first effect goes (as an undo elsewhere would take it): the third is the second row now, and still the one picked.
        deck::AnimSequence seq = deck::readAnimations(page(), ed->doc());
        const int pickedShape = seq.effects[2].shape, otherShape = seq.effects[3].shape;
        QVERIFY(pickedShape != otherShape);
        seq.effects.removeAt(0);
        deck::setAnimations(*ed->doc(), page(), seq);
        pane->reload();
        QCOMPARE(pane->rowCount(), 3);
        QCOMPARE(pane->selectedRows(), QVector<int>({1}));
        // The commands mean it too: With Previous changes the effect picked, not the one that took its place in the list.
        w.act("anim.start.with")->trigger();
        const deck::AnimSequence after = deck::readAnimations(page(), ed->doc());
        QCOMPARE(int(after.effects.size()), 3);
        QCOMPARE(after.effects[1].shape, pickedShape);
        QVERIFY2(after.effects[1].start == deck::EffectStart::WithPrevious, qPrintable(QStringLiteral("%1 %2 %3").arg(int(after.effects[0].start)).arg(int(after.effects[1].start)).arg(int(after.effects[2].start))));
        QCOMPARE(after.effects[2].shape, otherShape);
        QVERIFY(after.effects[2].start != deck::EffectStart::WithPrevious);
    }

    // A build of what a kept sequence aims at stays with it.
    void animationEditKeepsBuildsOfKeptSequences()
    {
        auto d = editDeck(2);
        Page &slide = *d->pages[0];
        deck::AnimSequence seq;
        seq.add(deck::makeAnim(QStringLiteral("entr.fade"), 10));
        deck::setAnimations(*d, slide, seq);
        QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
        const QString waits = animXml("<p:seq concurrent='1' nextAc='seek'><p:cTn id='30' restart='whenNotActive' fill='hold' evtFilter='cancelBubble' nodeType='interactiveSeq'><p:stCondLst><p:cond evt='onClick' delay='0'><p:tgtEl><p:spTgt spid='11'/></p:tgtEl></p:cond></p:stCondLst>"
                                      "<p:endSync evt='end' delay='0'><p:rtn val='all'/></p:endSync><p:childTnLst/></p:cTn><p:nextCondLst><p:cond evt='onClick' delay='0'><p:tgtEl><p:spTgt spid='11'/></p:tgtEl></p:cond></p:nextCondLst></p:seq>");
        const QString tail = QStringLiteral("</p:childTnLst></p:cTn></p:par></p:tnLst>");
        QVERIFY(xml.contains(tail));
        xml.replace(tail, waits + tail);
        xml.replace(QStringLiteral("</p:bldLst>"), QStringLiteral("<p:bldP spid=\"11\" grpId=\"0\" animBg=\"1\"/></p:bldLst>"));
        slide.props[QStringLiteral("timingXml")] = xml;
        seq = deck::readAnimations(slide);
        QVERIFY(seq.editable);
        QCOMPARE(int(seq.kept.size()), 1);
        seq.add(deck::makeAnim(QStringLiteral("entr.fly"), 3));
        deck::setAnimations(*d, slide, seq);
        const QString written = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY(written.contains(waits));
        QVERIFY2(written.contains(QStringLiteral("<p:bldP spid=\"11\"")), qPrintable(written.right(500)));
        QVERIFY2(animIdProblem(written).isEmpty(), qPrintable(animIdProblem(written)));
    }

    // A paragraph that is gone has no effect: not in the pane, not written, and not a click in the show.
    void animationEditDropsParagraphsThatAreGone()
    {
        auto d = editDeck();
        Page &slide = *d->pages[0];
        deck::AnimSequence seq;
        seq.add(deck::makeAnim(QStringLiteral("entr.fade"), 3));
        deck::setByParagraph(seq, 0, true, *d, slide);
        deck::setAnimations(*d, slide, seq);
        QCOMPARE(int(deck::playableTimeline(*d, slide).steps.size()), 3);
        QTextDocument *story = d->storyDoc(static_cast<TextItem *>(slide.items[1].get())->storyId);
        story->setPlainText(QStringLiteral("First paragraph\nSecond paragraph"));
        QCOMPARE(int(deck::readAnimations(slide).effects.size()), 3);
        QCOMPARE(int(deck::readAnimations(slide, d.get()).effects.size()), 2);
        QCOMPARE(int(deck::playableTimeline(*d, slide).steps.size()), 2);
        // The next write leaves it out.
        deck::AnimSequence again = deck::readAnimations(slide);
        again.add(deck::makeAnim(QStringLiteral("entr.fly"), 2));
        deck::setAnimations(*d, slide, again);
        QCOMPARE(int(deck::readAnimations(slide).effects.size()), 3);
    }

    // A kept effect nested deeper than a hundred levels is written back whole.
    void animationEditKeepsDeepEffects()
    {
        auto d = editDeck(1);
        Page &slide = *d->pages[0];
        QString inner = animXml("<p:set><p:cBhvr><p:cTn dur='1' fill='hold' id='900'/><p:tgtEl><p:spTgt spid='10'/></p:tgtEl><p:attrNameLst><p:attrName>style.visibility</p:attrName></p:attrNameLst></p:cBhvr><p:to><p:strVal val='visible'/></p:to></p:set>");
        for (int i = 0; i < 40; ++i) inner = animXml("<p:par><p:cTn fill='hold' id='%1'><p:childTnLst>").arg(100 + i) + inner + animXml("</p:childTnLst></p:cTn></p:par>");
        const QString effect = animXml("<p:par><p:cTn presetID='77' presetClass='entr' presetSubtype='0' fill='hold' nodeType='clickEffect' id='5'><p:stCondLst><p:cond delay='0'/></p:stCondLst><p:childTnLst>")
                               + inner + animXml("</p:childTnLst></p:cTn></p:par>");
        slide.props[QStringLiteral("timingXml")] = animBareTiming(effect);
        deck::AnimSequence seq = deck::readAnimations(slide);
        QCOMPARE(int(seq.effects.size()), 1);
        QVERIFY(seq.effects[0].custom());
        seq.add(deck::makeAnim(QStringLiteral("entr.fade"), 3));
        deck::setAnimations(*d, slide, seq);
        const QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
        QCOMPARE(xml.count(QStringLiteral("<p:par>")), effect.count(QStringLiteral("<p:par>")) + 5);   // the nested pars, and each click's two and the new effect's one
        QVERIFY(xml.contains(QStringLiteral("spid=\"10\"")));
        QVERIFY2(animIdProblem(xml).isEmpty(), qPrintable(animIdProblem(xml)));
    }

    // The Repeat box keeps a repeat that is not a whole number.
    void animationOptionsDialogKeepsFractionalRepeats()
    {
        deck::Anim a = deck::makeAnim(QStringLiteral("entr.fade"), 3);
        a.repeat = 2.5;
        AnimationOptionsDialog dlg(a, false, AnimationOptionsDialog::TimingTab);
        QCOMPARE(dlg.result().repeat, 2.5);
        a.repeat = 3;
        AnimationOptionsDialog whole(a, false, AnimationOptionsDialog::TimingTab);
        QCOMPARE(whole.result().repeat, 3.0);
    }

    // The gallery's pictures say what each effect is called, under the picture.
    void animationGalleryPicturesAreNamed()
    {
        const deck::EffectKind &kind = *deck::effectKind(QStringLiteral("entr.fly"));
        const QSize size(64, 60);
        const QImage plain = effectIcon(kind).pixmap(size).toImage(), named = effectIcon(kind, QString(), true).pixmap(size).toImage();
        QVERIFY(plain != named);
        // The name is in the bottom of the picture, and the slide above it.
        int ink = 0;
        for (int y = size.height() - 12; y < size.height(); ++y)
            for (int x = 8; x < size.width() - 8; ++x)
                if (qAlpha(named.pixel(x, y)) > 160) ++ink;
        QVERIFY2(ink > 15, qPrintable(QString::number(ink)));
        QCOMPARE(qAlpha(named.pixel(4, size.height() - 3)), 0);
        QVERIFY(qAlpha(plain.pixel(4, size.height() - 3)) > 0);
    }

    // The windows of the animation feature, as pictures, when JD_ANIM_SHOTS names a folder (the other times it only checks that they draw).
    void animationScreenshots()
    {
        const QString folder = qEnvironmentVariable("JD_ANIM_SHOTS");
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Editor *ed = w.editor();
        w.newSlide(QStringLiteral("obj"));
        auto slidePage = [ed]() -> Page & { return *ed->doc()->pages[ed->currentPage()]; };
        ed->doc()->storyDoc(static_cast<TextItem *>(slidePage().items[0].get())->storyId)->setPlainText(QStringLiteral("Quarterly review"));
        ed->doc()->storyDoc(static_cast<TextItem *>(slidePage().items[1].get())->storyId)->setPlainText(QStringLiteral("Sales grew in every region\nThree new offices opened\nCosts stayed flat"));
        auto box = std::make_shared<ShapeItem>();
        box->name = QStringLiteral("Growth chart");
        box->rect = QRectF(560, 330, 300, 150);
        box->fill = Fill::solid(ColorRef::rgb(QColor(0x3A, 0x6E, 0xA5)));
        slidePage().items.push_back(box);
        auto pick = [&](const QString &id, const char *action) {
            ed->select(id);
            w.act(action)->trigger();
        };
        pick(slidePage().items[0]->id, "anim.set.entr.fade");
        pick(slidePage().items[1]->id, "anim.set.entr.fly");
        {
            const deck::AnimSequence seq = deck::readAnimations(slidePage());
            deck::AnimSequence two = seq;
            two.effects[0].start = deck::EffectStart::OnClick;
            deck::setByParagraph(two, 1, true, *ed->doc(), slidePage());
            deck::setAnimations(*ed->doc(), slidePage(), two);
        }
        pick(box->id, "anim.set.emph.spin");
        w.act("anim.add.path.shapes")->trigger();
        w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Animations")));
        w.showAnimationPane(true);
        w.animationPane()->list()->setCurrentRow(3, QItemSelectionModel::ClearAndSelect);
        QTRY_VERIFY(w.animationPane()->isVisible());
        QApplication::processEvents();
        QCOMPARE(w.animationPane()->rowCount(), 6);
        auto save = [&](const QImage &img, const char *name) {
            QVERIFY(!img.isNull());
            if (!folder.isEmpty()) QVERIFY(img.save(QDir(folder).filePath(QString::fromLatin1(name))));
        };
        save(w.grab().toImage(), "tab-and-pane.png");
        // The gallery, all open.
        auto *gallery = w.ribbon()->findChild<Gallery *>(QStringLiteral("animationGallery"));
        QVERIFY(gallery);
        for (QToolButton *b : gallery->findChildren<QToolButton *>())
            if (b->accessibleName() == QLatin1String("Show all")) b->click();
        QTRY_VERIFY(QApplication::activePopupWidget());
        save(QApplication::activePopupWidget()->grab().toImage(), "gallery.png");
        QApplication::activePopupWidget()->close();
        // Effect Options of the fly in.
        w.animationPane()->list()->setCurrentRow(1, QItemSelectionModel::ClearAndSelect);
        QMenu *menu = nullptr;
        for (QToolButton *b : w.ribbon()->findChildren<QToolButton *>())
            if (b->defaultAction() && b->defaultAction()->objectName() == QLatin1String("ribbon.animEffectOptions") && b->menu()) menu = b->menu();
        QVERIFY(menu);
        menu->popup(QPoint(300, 200));
        QTRY_VERIFY(menu->isVisible());
        save(menu->grab().toImage(), "effect-options-menu.png");
        menu->close();
        // The row menu.
        // The dialogs.
        deck::Anim a = deck::makeAnim(QStringLiteral("entr.fly"), 3);
        a.variant = QStringLiteral("left");
        a.duration = 1500;
        a.delay = 250;
        a.smoothEnd = true;
        for (auto tab : {AnimationOptionsDialog::EffectTab, AnimationOptionsDialog::TimingTab, AnimationOptionsDialog::TextTab}) {
            AnimationOptionsDialog dlg(a, true, tab, &w);
            dlg.show();
            QVERIFY(QTest::qWaitForWindowExposed(&dlg));
            save(dlg.grab().toImage(), tab == AnimationOptionsDialog::EffectTab ? "dialog-effect.png" : tab == AnimationOptionsDialog::TimingTab ? "dialog-timing.png" : "dialog-text.png");
        }
    }

    // A click plays the slide's next step; the slide changes only when the
    // steps are played. Going back goes back a step, to its end state.
    void showPlaysClickStepsBeforeMovingOn()
    {
        auto d = animatedDeck(100);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(640, 360);
        QCOMPARE(show.stepCount(), 2);
        QCOMPARE(show.step(), 0);
        const QImage start = show.grab().toImage();
        show.next();
        QCOMPARE(show.currentSlide(), 0);
        QVERIFY(show.isPlayingStep());
        QTRY_VERIFY(!show.isPlayingStep());
        QCOMPARE(show.step(), 1);
        QCOMPARE(show.currentSlide(), 0);
        const QImage first = show.grab().toImage();
        QVERIFY(first != start);
        show.next();                       // the body appears at once
        QCOMPARE(show.step(), 2);
        QVERIFY(!show.isPlayingStep());
        const QImage second = show.grab().toImage();
        QVERIFY(second != first);
        show.next();
        QCOMPARE(show.currentSlide(), 1);
        QCOMPARE(show.stepCount(), 0);
        // Back: to the slide as it was left, then a step at a time.
        show.previous();
        QCOMPARE(show.currentSlide(), 0);
        QCOMPARE(show.step(), 2);
        QCOMPARE(show.grab().toImage(), second);
        show.previous();
        QCOMPARE(show.step(), 1);
        QCOMPARE(show.grab().toImage(), first);
        show.previous();
        QCOMPARE(show.step(), 0);
        QCOMPARE(show.grab().toImage(), start);
        show.previous();                   // nothing before the first slide's start
        QCOMPARE(show.currentSlide(), 0);
        QCOMPARE(show.step(), 0);
        // A jump shows the slide at its start.
        show.next();
        QTRY_COMPARE(show.step(), 1);
        show.goTo(2);
        show.goTo(0);
        QCOMPARE(show.step(), 0);
        QCOMPARE(show.grab().toImage(), start);
        // Home does too.
        show.next();
        QTRY_COMPARE(show.step(), 1);
        QTest::keyClick(&show, Qt::Key_Home);
        QCOMPARE(show.step(), 0);
    }

    // A click while an effect plays finishes it at once, and nothing more; a
    // step undone by Back can be played again.
    void showClickDuringAnEffectFinishesIt()
    {
        auto d = animatedDeck(60000);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(640, 360);
        show.next();
        QVERIFY(show.isPlayingStep());
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
        QVERIFY(!show.isPlayingStep());
        QCOMPARE(show.step(), 1);
        QCOMPARE(show.currentSlide(), 0);
        // Back in the middle of an effect stops it and takes the step back.
        show.previous();
        QCOMPARE(show.step(), 0);
        show.next();
        QVERIFY(show.isPlayingStep());
        show.previous();
        QVERIFY(!show.isPlayingStep());
        QCOMPARE(show.step(), 0);
        QCOMPARE(show.currentSlide(), 0);
        show.next();
        QVERIFY(show.isPlayingStep());
        // The key does what the click does.
        QTest::keyClick(&show, Qt::Key_Space);
        QVERIFY(!show.isPlayingStep());
        QCOMPARE(show.step(), 1);
    }

    // A slide that does not move on at a click still plays its steps at one.
    void showClickPlaysStepsOnASlideThatIgnoresClicks()
    {
        auto d = animatedDeck(30);
        deck::Transition t;
        t.advanceOnClick = false;
        deck::setTransition(*d->pages[0], t);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(640, 360);
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
        QTRY_COMPARE(show.step(), 1);
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
        QCOMPARE(show.step(), 2);
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));   // all played: the slide ignores it
        QCOMPARE(show.currentSlide(), 0);
        QTest::keyClick(&show, Qt::Key_Right);
        QCOMPARE(show.currentSlide(), 1);
    }

    // A step that starts after or with the previous one with nothing before it
    // starts by itself once the slide, and its transition, is shown.
    void showStartsAutomaticStepsWhenTheSlideIsShown()
    {
        auto d = showDeck(3);
        numberShowShapes(*d->pages[0]);
        numberShowShapes(*d->pages[1]);
        d->pages[0]->props[QStringLiteral("timingXml")] = animTiming(animStep({animGroup(0, {animFadeIn(2, 60, "afterEffect")})}, true) + animStep({animGroup(0, {animAppear(3)})}));
        d->pages[1]->props[QStringLiteral("timingXml")] = animTiming(animStep({animGroup(0, {animFadeIn(2, 60, "withEffect")})}, true));
        deck::setTransition(*d->pages[1], showTransition("fade", 0.25));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(640, 360);
        QCOMPARE(show.stepCount(), 2);
        QCOMPARE(show.step(), 0);
        QVERIFY(!show.isPlayingStep());       // not before the show is shown
        show.show();
        QTRY_COMPARE(show.step(), 1);
        QVERIFY(!show.isPlayingStep());
        show.next();                          // the click step, not the automatic one
        QCOMPARE(show.step(), 2);
        // The next slide's steps wait for its transition.
        show.next();
        QVERIFY(show.isAnimating());
        QCOMPARE(show.targetSlide(), 1);
        QVERIFY(!show.isPlayingStep());
        QTRY_VERIFY(!show.isAnimating());
        QTRY_COMPARE(show.step(), 1);
        QCOMPARE(show.currentSlide(), 1);
        // A jump starts them too; going back to a slide shows it with its steps played.
        show.goTo(0);
        QTRY_COMPARE(show.step(), 1);
        show.goTo(1);
        QTRY_COMPARE(show.step(), 1);
        show.goTo(0);
        show.next();
        show.next();
        QCOMPARE(show.step(), 2);
        show.next();
        QTRY_COMPARE(show.currentSlide(), 1);
        QTRY_COMPARE(show.step(), 1);
        show.previous();                      // back a step on the slide, then to the one before
        QCOMPARE(show.currentSlide(), 1);
        QCOMPARE(show.step(), 0);
        show.previous();
        QCOMPARE(show.currentSlide(), 0);
        QCOMPARE(show.step(), 2);
        QVERIFY(!show.isPlayingStep());
    }

    // A slide's seconds count from the end of its automatic steps; clicks left
    // unplayed when they are up are skipped.
    void showAdvanceAfterCountsFromTheLastEffect()
    {
        auto d = showDeck(3);
        numberShowShapes(*d->pages[0]);
        d->pages[0]->props[QStringLiteral("timingXml")] = animTiming(animStep({animGroup(0, {animFadeIn(2, 400, "afterEffect")})}, true) + animStep({animGroup(0, {animAppear(3)})}));
        deck::Transition t = showTransition("none", 0.7);
        t.advanceAfter = 0.05;
        deck::setTransition(*d->pages[0], t);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(640, 360);
        QElapsedTimer sinceShown;
        sinceShown.start();
        show.show();
        QVERIFY(show.isPlayingStep());
        QVERIFY(!show.autoAdvanceActive());    // the effect is playing
        QTRY_VERIFY(show.autoAdvanceActive() || show.currentSlide() == 1);
        QTRY_COMPARE(show.currentSlide(), 1);
        QVERIFY(sinceShown.elapsed() >= 400);
        QCOMPARE(show.step(), 0);              // the second step was skipped
    }

    // A step with only a media call takes no click.
    void showSkipsStepsThatShowNothing()
    {
        auto d = showDeck(2);
        numberShowShapes(*d->pages[0]);
        d->pages[0]->props[QStringLiteral("timingXml")] = animTiming(animSteps({animMediaCall(2)}));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        QCOMPARE(show.stepCount(), 0);
        show.next();
        QCOMPARE(show.currentSlide(), 1);
    }

    // Presenter View's Next shows what the next click gives: the slide with its
    // next step played, then the next slide.
    void presenterViewNextShowsTheNextStep()
    {
        auto d = animatedDeck(30);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        PresenterView pv(&show);
        QCOMPARE(pv.counterText(), QStringLiteral("Slide 1 of 3"));
        QCOMPARE(pv.nextSlide(), 0);            // the same slide, a click on
        const QImage afterFirst = pv.nextImage();
        QVERIFY(!afterFirst.isNull());
        show.next();
        QTRY_VERIFY(!show.isPlayingStep());
        QCOMPARE(pv.nextSlide(), 0);
        const QImage afterSecond = pv.nextImage();
        QVERIFY(afterSecond != afterFirst);
        show.next();
        QCOMPARE(pv.nextSlide(), 1);            // all played: the next slide
        QVERIFY(pv.nextImage() != afterSecond);
        QCOMPARE(pv.counterText(), QStringLiteral("Slide 1 of 3"));
        show.previous();
        QCOMPARE(show.step(), 1);
        QCOMPARE(pv.nextSlide(), 0);
        QCOMPARE(pv.nextImage(), afterSecond);
        show.previous();
        QCOMPARE(show.step(), 0);
        QCOMPARE(pv.nextImage(), afterFirst);
        // The buttons play steps too.
        pv.findChild<QAbstractButton *>("nextButton")->click();
        QTRY_COMPARE(show.step(), 1);
        pv.grab();
    }

    // The ribbon's preview leaves the slide before as the show leaves it, with
    // its steps played (here the title exits).
    void transitionPreviewLeavesTheSlideBeforeWithItsStepsPlayed()
    {
        auto grabOf = [&](bool animated) {
            auto d = showDeck(2);
            numberShowShapes(*d->pages[0]);
            if (animated) d->pages[0]->props[QStringLiteral("timingXml")] = animTiming(animSteps({animFadeOut(2, 200)}));
            else d->pages[0]->items.erase(d->pages[0]->items.begin());   // the title is gone
            deck::setTransition(*d->pages[1], showTransition("fade", 600));
            QWidget parent;
            parent.resize(480, 300);
            LayoutCache cache;
            TransitionPreview preview(d.get(), &cache, &parent);
            preview.play(1);
            return preview.grab().toImage();
        };
        QCOMPARE(grabOf(true), grabOf(false));
    }

    // A step's frames are drawn from what lies beneath and above the shapes it
    // moves, kept; they come out as drawing the whole slide does.
    void showStepFramesAreTheWholeSlideDrawn()
    {
        auto d = showDeck(2);
        Page &slide = *d->pages[0];
        numberShowShapes(slide);
        d->storyDoc(static_cast<TextItem *>(slide.items[1].get())->storyId)->setPlainText(QStringLiteral("The body of the slide"));
        auto under = std::make_shared<ShapeItem>();
        under->rect = QRectF(40, 60, 500, 300);
        under->fill = Fill::solid(ColorRef::rgb(QColor(220, 230, 250)));
        under->sourceId = 20;
        auto over = std::make_shared<ShapeItem>();
        over->shape = QStringLiteral("ellipse");
        over->rect = QRectF(300, 200, 400, 250);
        over->fill = Fill::solid(ColorRef::rgb(QColor(250, 200, 80)), 0.4);
        over->sourceId = 21;
        slide.items.insert(slide.items.begin(), under);
        slide.items.push_back(over);
        slide.props[QStringLiteral("timingXml")] = animTiming(animSteps({animFlyIn(20, 200), animFadeIn(3, 200), animWipe(21, "wipe(left)", true, 200)}));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        const QSize size(640, 360);
        const deck::Timeline tl = deck::playableTimeline(slide);
        const deck::Geometry geometry = deck::geometryOf(*d, slide);
        QCOMPARE(int(tl.steps.size()), 3);
        for (int step = 0; step < 3; ++step)
            for (const double ms : {0.0, 60.0, 150.0, 199.0}) {
                const QImage frame = show.renderView(SlideShow::View{0, step}, ms, size, 1.0);
                const deck::States states = deck::statesAt(tl, geometry, step, ms);
                const QImage whole = renderSlideImage(d.get(), &cache, 0, size, 1.0, Qt::black, &states);
                QCOMPARE(frame.size(), whole.size());
                int worst = 0;
                for (int y = 0; y < frame.height(); ++y)
                    for (int x = 0; x < frame.width(); ++x) {
                        const QRgb a = frame.pixel(x, y), b = whole.pixel(x, y);
                        worst = std::max({worst, std::abs(qRed(a) - qRed(b)), std::abs(qGreen(a) - qGreen(b)), std::abs(qBlue(a) - qBlue(b))});
                    }
                QVERIFY2(worst <= 2, qPrintable(QStringLiteral("step %1 at %2 ms differs by %3").arg(step).arg(ms).arg(worst)));
            }
        // It moves: the frames are not all one picture.
        QVERIFY(show.renderView(SlideShow::View{0, 0}, 0, size, 1.0) != show.renderView(SlideShow::View{0, 0}, 150, size, 1.0));
    }

    // Prints what a frame of an effect costs on a busy slide, against drawing the slide whole.
    void showStepFrameTimes()
    {
        auto d = showDeck(2);
        Page &slide = *d->pages[0];
        QStringList lines;
        for (int k = 0; k < 8; ++k) lines << QStringLiteral("Point %1 of the slide, with enough words to fill a line of the body text").arg(k + 1);
        d->storyDoc(static_cast<TextItem *>(slide.items[1].get())->storyId)->setPlainText(lines.join('\n'));
        for (int k = 0; k < 5; ++k) {
            auto shape = std::make_shared<ShapeItem>();
            shape->shape = k % 2 ? QStringLiteral("ellipse") : QStringLiteral("roundRect");
            shape->rect = QRectF(40 + k * 170, 330, 160, 150);
            shape->fill = Fill::gradient(ColorRef::rgb(QColor(40 + k * 50, 90, 180)), ColorRef::rgb(QColor(240, 200 - k * 30, 60)), 45 + k * 30);
            shape->fx.shadow.on = k == 0;
            shape->sourceId = 10 + k;
            slide.items.push_back(shape);
        }
        slide.props[QStringLiteral("timingXml")] = animTiming(animSteps({animFadeIn(14, 500), animFlyIn(10, 500)}));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        const QSize size(1920, 1080);
        const deck::Timeline tl = deck::playableTimeline(slide);
        const deck::Geometry geometry = deck::geometryOf(*d, slide);
        auto millis = [](const std::function<void()> &f) {
            QElapsedTimer t;
            t.start();
            f();
            return t.nsecsElapsed() / 1e6;
        };
        QStringList line;
        for (int step = 0; step < 2; ++step) {
            const double first = millis([&] { show.renderView(SlideShow::View{0, step}, 0, size, 1.0); });
            QVector<double> frames, wholes;
            for (int i = 1; i <= 7; ++i) {
                frames << millis([&] { show.renderView(SlideShow::View{0, step}, i * 60.0, size, 1.0); });
                const deck::States states = deck::statesAt(tl, geometry, step, i * 60.0);
                wholes << millis([&] { renderSlideImage(d.get(), &cache, 0, size, 1.0, Qt::black, &states); });
            }
            std::sort(frames.begin(), frames.end());
            std::sort(wholes.begin(), wholes.end());
            line << QStringLiteral("step %1: kept layers %2 ms, a frame %3 ms, the whole slide %4 ms").arg(step + 1).arg(first, 0, 'f', 1).arg(frames[3], 0, 'f', 1).arg(wholes[3], 0, 'f', 1);
        }
        qInfo().noquote() << QStringLiteral("SHOWTIMES 1920x1080 effects:") << line.join(QStringLiteral(", "));
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
};

JP_TEST_CLASS(DeckAnimationTests)
#include "tst_deck_animation.moc"
