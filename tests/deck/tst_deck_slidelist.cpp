// JeffDeck's tests of the slide sorter, sections, and the outline view.
#include "deckhelpers.h"

class DeckSlideListTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

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

    // A file with two slide masters offers the current slide's master's
    // layouts; placeholder numbers past an int still match; a new
    // presentation is "Presentation1".
    void layoutsPerMasterAndLargePlaceholderNumbers()
    {
        auto d = deck::newPresentation();
        auto second = std::make_shared<MasterPage>(*d->masters[0]);
        second->id = QStringLiteral("SM2");
        second->items.clear();
        d->masters.push_back(second);
        auto layout2 = std::make_shared<MasterPage>();
        layout2->id = QStringLiteral("L2-1");
        layout2->baseId = second->id;
        layout2->props[QStringLiteral("kind")] = QStringLiteral("layout");
        layout2->props[QStringLiteral("type")] = QStringLiteral("obj");
        auto body = std::make_shared<TextItem>();
        body->role = deck::placeholderRole(QStringLiteral("body"), 0) + QStringLiteral(":4294967295");
        body->rect = QRectF(10, 20, 300, 200);
        body->storyId = d->createStory();
        layout2->items.push_back(body);
        d->masters.push_back(layout2);
        QCOMPARE(deck::layouts(*d).size(), 10);
        QCOMPARE(deck::layouts(*d, QStringLiteral("SM2")).size(), 1);
        QCOMPARE(deck::slideMasterOf(*d, 0), QStringLiteral("SM1"));
        QCOMPARE(deck::layoutOfType(*d, QStringLiteral("obj"), QStringLiteral("SM2")), layout2.get());
        // A slide's placeholder with the same big number takes the layout's place.
        const int i = deck::addSlide(*d, *layout2);
        Page &slide = *d->pages[i];
        QCOMPARE(slide.items.size(), size_t(1));
        slide.items[0]->rect = QRectF(0, 0, 1, 1);
        deck::resetSlide(*d, slide);
        QCOMPARE(slide.items[0]->rect, QRectF(10, 20, 300, 200));

        DeckWindow w;
        QCOMPARE(w.editor()->displayName(), QStringLiteral("Presentation1"));
    }

private:
    static QPoint centerOf(SlideSorter *s, int slide) { return s->thumbnailRect(slide).center(); }

private Q_SLOTS:
    // Slide Sorter: every slide is a thumbnail in a grid that wraps with the
    // window; the Slide Sorter command switches to it and back.
    void slideSorterShowsEverySlideAsAThumbnail()
    {
        setUiDark(false);
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"One", "Two", "Three", "Four", "Five"});
        QVERIFY(w.slideSorter()->isHidden());
        QVERIFY(w.act("view.normal")->isChecked());
        QVERIFY(w.act("view.sorter")->isEnabled());
        w.act("view.sorter")->trigger();
        SlideSorter *s = w.slideSorter();
        QVERIFY(s->isVisible());
        QVERIFY(!w.canvas()->isVisible());
        QVERIFY(w.act("view.sorter")->isChecked() && !w.act("view.normal")->isChecked());
        QCOMPARE(w.view(), DeckWindow::View::Sorter);
        QCOMPARE(s->slideCount(), 5);
        QTRY_COMPARE(s->renderedCount(), 5);
        // A grid that wraps: left to right, then the next row; fewer to a row when the window narrows.
        w.resize(760, 800);
        QTRY_VERIFY(s->columns() >= 2 && s->columns() < 5);
        const int cols = s->columns();
        QCOMPARE(s->thumbnailRect(cols).left(), s->thumbnailRect(0).left());
        QVERIFY(s->thumbnailRect(cols).top() > s->thumbnailRect(0).bottom());
        QVERIFY(s->thumbnailRect(1).left() > s->thumbnailRect(0).right());
        w.resize(520, 800);
        QTRY_VERIFY(s->columns() < cols);
        QCOMPARE(s->slideAt(centerOf(s, 3)), 3);
        w.resize(1280, 800);
        QTRY_VERIFY(s->columns() >= 5);   // all in one row again
        QCOMPARE(s->thumbnailRect(4).top(), s->thumbnailRect(0).top());
        QTRY_COMPARE(s->renderedCount(), 5);
        w.act("view.normal")->trigger();
        QVERIFY(s->isHidden() && w.canvas()->isVisible());
        QVERIFY(w.act("view.normal")->isChecked());
    }

    // The current slide is outlined in the accent, a hidden one is faded, and
    // a slide with a transition has a star beside its number.
    void slideSorterMarksTheCurrentHiddenAndAnimatedSlides()
    {
        setUiDark(false);
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"One", "Two", "Three"});
        w.act("view.sorter")->trigger();
        SlideSorter *s = w.slideSorter();
        QTRY_COMPARE(s->renderedCount(), 3);
        w.editor()->setCurrentPage(2);
        QCOMPARE(s->selection(), QList<int>({2}));
        w.editor()->setCurrentPage(1);
        w.toggleHidden();
        w.editor()->setCurrentPage(2);
        QTRY_COMPARE(s->renderedCount(), 3);
        QImage img = s->viewport()->grab().toImage();
        const QRect cur = s->thumbnailRect(2), hid = s->thumbnailRect(1), plain = s->thumbnailRect(0);
        QCOMPARE(img.pixelColor(cur.left() - 2, cur.center().y()), deckAppInfo().accent);
        QCOMPARE(img.pixelColor(cur.center().x(), cur.top() - 2), deckAppInfo().accent);
        QCOMPARE(img.pixelColor(cur.left() + 4, cur.top() + 4), QColor(Qt::white));   // no fill over the slide
        QCOMPARE(img.pixelColor(plain.left() + 4, plain.top() + 4), QColor(Qt::white));
        QVERIFY(img.pixelColor(plain.left() - 2, plain.center().y()) != deckAppInfo().accent);
        QVERIFY(img.pixelColor(hid.left() + 4, hid.top() + 4) != QColor(Qt::white));   // faded
        // The strip under a thumbnail, where the number and the star are.
        auto strip = [&](const QRect &t) { return s->viewport()->grab(QRect(t.left(), t.bottom() + 4, t.width(), 24)).toImage(); };
        const QImage before = strip(plain);
        w.editor()->setCurrentPage(0);
        w.act("transition.fade")->trigger();
        QTRY_VERIFY(strip(plain) != before);
    }

    // Click, Shift+click, Ctrl+click, and Ctrl+A select slides; Delete removes
    // the selected ones as one step that undoes.
    void slideSorterSelectsSeveralSlidesAndDeletesThemAsOneStep()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"One", "Two", "Three", "Four", "Five"});
        Editor *ed = w.editor();
        w.act("view.sorter")->trigger();
        SlideSorter *s = w.slideSorter();
        QWidget *vp = s->viewport();
        QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, centerOf(s, 1));
        QCOMPARE(s->selection(), QList<int>({1}));
        QCOMPARE(ed->currentPage(), 1);
        QTest::mouseClick(vp, Qt::LeftButton, Qt::ShiftModifier, centerOf(s, 3));
        QCOMPARE(s->selection(), QList<int>({1, 2, 3}));
        QCOMPARE(ed->currentPage(), 3);
        QTest::mouseClick(vp, Qt::LeftButton, Qt::ControlModifier, centerOf(s, 0));
        QCOMPARE(s->selection(), QList<int>({0, 1, 2, 3}));
        QTest::mouseClick(vp, Qt::LeftButton, Qt::ControlModifier, centerOf(s, 0));
        QCOMPARE(s->selection(), QList<int>({1, 2, 3}));
        QTest::keyClick(s, Qt::Key_A, Qt::ControlModifier);
        QCOMPARE(s->selection(), QList<int>({0, 1, 2, 3, 4}));
        QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, centerOf(s, 1));
        QTest::mouseClick(vp, Qt::LeftButton, Qt::ShiftModifier, centerOf(s, 3));
        const int steps = ed->undoStack()->count();
        QTest::keyClick(s, Qt::Key_Delete);
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"One", "Five"}));
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QTRY_COMPARE(s->slideCount(), 2);
        QCOMPARE(s->selection(), QList<int>({1}));   // the slide that took their place
        ed->undo();
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"One", "Two", "Three", "Four", "Five"}));
        QTRY_COMPARE(s->slideCount(), 5);
        ed->redo();
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"One", "Five"}));
        // Everything selected: one slide stays.
        QTest::keyClick(s, Qt::Key_A, Qt::ControlModifier);
        QTest::keyClick(s, Qt::Key_Delete);
        QCOMPARE(int(ed->doc()->pages.size()), 1);
    }

    // Duplicate (Ctrl+D) puts each copy after its slide; Hide hides them all;
    // New Slide goes after the selection; each is one step that undoes.
    void slideSorterDuplicatesHidesAndAddsSlides()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"One", "Two", "Three", "Four"});
        Editor *ed = w.editor();
        Document *d = ed->doc();
        w.act("view.sorter")->trigger();
        SlideSorter *s = w.slideSorter();
        s->select({1, 2}, 2);
        QTest::keyClick(s, Qt::Key_D, Qt::ControlModifier);
        QCOMPARE(titlesOf(*d), (QStringList{"One", "Two", "Two", "Three", "Three", "Four"}));
        QCOMPARE(s->selection(), QList<int>({2, 4}));
        QCOMPARE(ed->undoStack()->count(), 1);
        ed->undo();
        QCOMPARE(titlesOf(*d), (QStringList{"One", "Two", "Three", "Four"}));
        s->select({0, 3}, 3);
        s->toggleHiddenSelected();
        QVERIFY(d->pages[0]->hidden && d->pages[3]->hidden && !d->pages[1]->hidden);
        QCOMPARE(ed->undoStack()->count(), 1);   // the copies, undone, are gone from the list
        s->toggleHiddenSelected();   // all hidden: shows them
        QVERIFY(!d->pages[0]->hidden && !d->pages[3]->hidden);
        QCOMPARE(ed->undoStack()->count(), 2);
        s->select({0, 1}, 1);
        s->newSlideAfterSelection();
        QCOMPARE(int(d->pages.size()), 5);
        QCOMPARE(titleOfSlide(*d, 2), QString());
        QCOMPARE(deck::layoutType(*d->masterFor(*d->pages[2])), QStringLiteral("obj"));
        QCOMPARE(ed->currentPage(), 2);
        QTRY_COMPARE(s->selection(), QList<int>({2}));
        QTRY_COMPARE(s->slideCount(), 5);
    }

    // Moving slides (what a drag does) puts them before another slide, in
    // their order, as one step that undoes; the current slide stays current.
    void slideSorterMovesSlidesAsOneStep()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"One", "Two", "Three", "Four", "Five"});
        Editor *ed = w.editor();
        w.act("view.sorter")->trigger();
        SlideSorter *s = w.slideSorter();
        ed->setCurrentPage(1);
        s->moveSlides({0, 1}, 4);
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"Three", "Four", "One", "Two", "Five"}));
        QCOMPARE(ed->undoStack()->count(), 1);
        QCOMPARE(ed->currentPage(), 3);   // "Two" is still the current slide
        QCOMPARE(s->selection(), QList<int>({2, 3}));
        ed->undo();
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"One", "Two", "Three", "Four", "Five"}));
        QCOMPARE(ed->currentPage(), 1);
        ed->redo();
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"Three", "Four", "One", "Two", "Five"}));
        ed->undo();
        // Where a slide already is, nothing happens; to the end is the count.
        s->moveSlides({2}, 2);
        s->moveSlides({2}, 3);
        QCOMPARE(ed->undoStack()->count(), 1);
        s->moveSlides({0}, 5);
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"Two", "Three", "Four", "Five", "One"}));
        s->moveSlides({4, 0}, 2);   // out of order: they keep the order they have
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"Three", "Two", "One", "Four", "Five"}));
        ed->undo();
        ed->undo();
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"One", "Two", "Three", "Four", "Five"}));
        ed->undoStack()->clear();
        // The drag: press on a slide, move to the left of the first one, let go.
        QWidget *vp = s->viewport();
        QTRY_COMPARE(s->renderedCount(), 5);
        const QPoint from = centerOf(s, 4), to(s->thumbnailRect(0).left() + 3, s->thumbnailRect(0).center().y());
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(vp, from + QPoint(-12, 0));
        QTest::mouseMove(vp, from + QPoint(-40, 0));
        QTest::mouseMove(vp, to);
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, to);
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"Five", "One", "Two", "Three", "Four"}));
        QCOMPARE(ed->undoStack()->count(), 1);
        QCOMPARE(ed->currentPage(), 0);
        // A click that stays put moves nothing, and Escape lets go of a drag.
        QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, centerOf(s, 2));
        QCOMPARE(ed->undoStack()->count(), 1);
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, centerOf(s, 0));
        QTest::mouseMove(vp, centerOf(s, 0) + QPoint(30, 0));
        QTest::mouseMove(vp, centerOf(s, 3));
        QTest::keyClick(s, Qt::Key_Escape);
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, centerOf(s, 3));
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"Five", "One", "Two", "Three", "Four"}));
        QCOMPARE(ed->undoStack()->count(), 1);
    }

    // Double-clicking a slide, or Enter on it, opens it in Normal view; the
    // arrow keys move through the grid.
    void slideSorterOpensASlideInNormalView()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"One", "Two", "Three", "Four", "Five", "Six", "Seven"});
        Editor *ed = w.editor();
        w.act("view.sorter")->trigger();
        SlideSorter *s = w.slideSorter();
        QTRY_COMPARE(s->renderedCount(), 7);
        QTest::mouseDClick(s->viewport(), Qt::LeftButton, Qt::NoModifier, centerOf(s, 2));
        QCOMPARE(w.view(), DeckWindow::View::Normal);
        QCOMPARE(ed->currentPage(), 2);
        QVERIFY(w.canvas()->isVisible() && s->isHidden());
        QVERIFY(w.act("view.normal")->isChecked());
        w.act("view.sorter")->trigger();
        const int cols = s->columns();
        QVERIFY(cols >= 3);
        QTest::keyClick(s, Qt::Key_Right);
        QCOMPARE(ed->currentPage(), 3);
        QCOMPARE(s->selection(), QList<int>({3}));
        QTest::keyClick(s, Qt::Key_Left);
        QTest::keyClick(s, Qt::Key_Left);
        QCOMPARE(ed->currentPage(), 1);
        QTest::keyClick(s, Qt::Key_Down);   // to the slide below, or the last one when none is right below
        const int below = ed->currentPage();
        QCOMPARE(below, std::min(1 + cols, 6));
        QTest::keyClick(s, Qt::Key_Up);
        QCOMPARE(ed->currentPage(), below - cols);
        QTest::keyClick(s, Qt::Key_End);
        QCOMPARE(ed->currentPage(), 6);
        QTest::keyClick(s, Qt::Key_Home, Qt::ShiftModifier);   // Shift extends the selection from where it began
        QCOMPARE(s->selection().size(), 7);
        QTest::keyClick(s, Qt::Key_Home);
        QTest::keyClick(s, Qt::Key_Right);
        QTest::keyClick(s, Qt::Key_Return);
        QCOMPARE(w.view(), DeckWindow::View::Normal);
        QCOMPARE(ed->currentPage(), 1);
    }

    // The zoom slider and Ctrl+wheel change the thumbnails' size, and Normal
    // view gets its own zoom back.
    void slideSorterZoomsItsThumbnails()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"One", "Two"});
        w.canvas()->zoomToFit(Canvas::Fit::WholePage);
        const double canvasZoom = w.canvas()->zoom();
        w.act("view.sorter")->trigger();
        SlideSorter *s = w.slideSorter();
        const int width = s->thumbnailRect(0).width();
        auto *slider = w.findChild<QSlider *>(QStringLiteral("zoomSlider"));
        auto *percent = w.findChild<QLabel *>(QStringLiteral("zoomPercent"));
        QVERIFY(slider && percent);
        QCOMPARE(percent->text(), QStringLiteral("%1%").arg(qRound(s->zoom() * 100)));
        slider->setValue(slider->value() + 6);
        QTRY_VERIFY(s->thumbnailRect(0).width() > width);
        QCOMPARE(percent->text(), QStringLiteral("%1%").arg(qRound(s->zoom() * 100)));
        const int bigger = s->thumbnailRect(0).width();
        QWheelEvent wheel(QPointF(40, 40), QPointF(40, 40), QPoint(), QPoint(0, -240), Qt::NoButton, Qt::ControlModifier, Qt::NoScrollPhase, false);
        QApplication::sendEvent(s->viewport(), &wheel);
        QVERIFY(s->thumbnailRect(0).width() < bigger);
        QCOMPARE(percent->text(), QStringLiteral("%1%").arg(qRound(s->zoom() * 100)));
        // While its thumbnail is drawn again at the new size, the old one stands in.
        QTRY_COMPARE(s->renderedCount(), 2);
        s->setZoom(s->zoom() * 1.3);
        QCOMPARE(s->renderedCount(), 0);
        const QImage standIn = s->viewport()->grab(s->thumbnailRect(0)).toImage();
        bool text = false;
        for (int y = 0; y < standIn.height() && !text; ++y)
            for (int x = 0; x < standIn.width() && !text; ++x) text = standIn.pixelColor(x, y).lightness() < 140;
        QVERIFY(text);
        // Far out and far in stop at the smallest and the largest.
        s->setZoom(0.001);
        QVERIFY(s->thumbnailRect(0).width() >= 40);
        s->setZoom(1000);
        QVERIFY(s->thumbnailRect(0).width() < 1000);
        w.act("view.normal")->trigger();
        QCOMPARE(w.canvas()->zoom(), canvasZoom);
        QVERIFY(w.canvas()->fitMode() == Canvas::Fit::WholePage);
    }
    // Sections are named runs of slides kept as counts: the first one made in
    // the middle of a deck puts the slides before it in a Default Section,
    // removing a section gives its slides to the one before (the next, for the
    // first), and a section moves with its slides.
    void sectionsAreAddedRenamedMovedAndRemoved()
    {
        auto d = lettered(6);
        QVERIFY(!deck::hasSections(*d));
        QVERIFY(deck::sectionsOf(*d).isEmpty());
        QCOMPARE(deck::sectionOfSlide(*d, 2), -1);
        QCOMPARE(deck::addSection(*d, 2), 1);
        QCOMPARE(sectionNames(*d), (QStringList{"Default Section", "Untitled Section"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 4}));
        const QRegularExpression guid(QStringLiteral("^\\{[0-9A-F]{8}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{12}\\}$"));
        const QVector<deck::Section> two = deck::sectionsOf(*d);
        QVERIFY(guid.match(two[0].id).hasMatch() && guid.match(two[1].id).hasMatch());
        QVERIFY(two[0].id != two[1].id);
        QCOMPARE(deck::sectionOfSlide(*d, 1), 0);
        QCOMPARE(deck::sectionOfSlide(*d, 2), 1);
        QCOMPARE(deck::sectionOfSlide(*d, 6), -1);
        QCOMPARE(deck::firstSlideOf(*d, 1), 2);
        QCOMPARE(deck::slideCountOf(*d, 1), 4);
        // A section made inside another takes the rest of it.
        QCOMPARE(deck::addSection(*d, 4, QStringLiteral("Budget")), 2);
        QCOMPARE(sectionNames(*d), (QStringList{"Default Section", "Untitled Section", "Budget"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 2, 2}));
        // At a section's first slide it takes that section's slides and leaves it empty.
        QCOMPARE(deck::addSection(*d, 2, QStringLiteral("Plan")), 2);
        QCOMPARE(sectionNames(*d), (QStringList{"Default Section", "Untitled Section", "Plan", "Budget"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 0, 2, 2}));
        QCOMPARE(deck::sectionOfSlide(*d, 2), 2);
        QCOMPARE(deck::firstSlideOf(*d, 1), 2);
        QCOMPARE(deck::slideCountOf(*d, 1), 0);
        deck::renameSection(*d, 1, QStringLiteral("Intro"));
        QCOMPARE(sectionNames(*d), (QStringList{"Default Section", "Intro", "Plan", "Budget"}));
        deck::renameSection(*d, 9, QStringLiteral("Nowhere"));
        QCOMPARE(sectionNames(*d).size(), 4);

        // Removing: the slides go up to the section before.
        deck::removeSection(*d, 2);
        QCOMPARE(sectionNames(*d), (QStringList{"Default Section", "Intro", "Budget"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 2, 2}));
        deck::removeSection(*d, 0);   // the first gives its slides to the next
        QCOMPARE(sectionNames(*d), (QStringList{"Intro", "Budget"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{4, 2}));

        // Moving a section takes its slides along.
        QCOMPARE(letters(*d), QStringLiteral("ABCDEF"));
        QCOMPARE(deck::moveSection(*d, 0, 1), 1);
        QCOMPARE(sectionNames(*d), (QStringList{"Budget", "Intro"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 4}));
        QCOMPARE(letters(*d), QStringLiteral("EFABCD"));
        QCOMPARE(deck::moveSection(*d, 0, -1), -1);   // nothing above the first
        QCOMPARE(deck::moveSection(*d, 1, 1), -1);    // nor below the last
        QCOMPARE(letters(*d), QStringLiteral("EFABCD"));
        QCOMPARE(deck::moveSection(*d, 1, -1), 0);
        QCOMPARE(letters(*d), QStringLiteral("ABCDEF"));

        // A section and its slides go together, but never all the slides.
        QVERIFY(deck::removeSectionAndSlides(*d, 1));
        QCOMPARE(letters(*d), QStringLiteral("ABCD"));
        QCOMPARE(sectionNames(*d), (QStringList{"Intro"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{4}));
        QVERIFY(!deck::removeSectionAndSlides(*d, 0));
        QCOMPARE(letters(*d), QStringLiteral("ABCD"));
        // The only section's removal leaves none, and so does Remove All.
        deck::removeSection(*d, 0);
        QVERIFY(!deck::hasSections(*d));
        QVERIFY(!d->appData.contains(QStringLiteral("sections")));
        QCOMPARE(deck::addSection(*d, 0), 0);   // at the first slide there is nothing for a Default Section
        QCOMPARE(sectionNames(*d), (QStringList{"Untitled Section"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{4}));
        deck::addSection(*d, 2);
        deck::removeAllSections(*d);
        QVERIFY(!deck::hasSections(*d));
    }

    // The counts follow the slide list: a new slide joins the section of the
    // slide before it, a slide taken away leaves its section, and slides put
    // in another order join the section where they land. What a caller misses
    // is made good on reading.
    void sectionCountsFollowTheSlideList()
    {
        auto d = lettered(6);
        deck::addSection(*d, 2, QStringLiteral("Plan"));    // Default Section 2, Plan 4
        deck::addSection(*d, 4, QStringLiteral("Budget"));  // 2, 2, 2
        const MasterPage *layout = deck::layoutOfType(*d, QStringLiteral("obj"));
        // After the last slide of a section it joins that section; at the very start, the first.
        deck::addSlide(*d, *layout, 2);
        QCOMPARE(sectionCounts(*d), (QList<int>{3, 2, 2}));
        deck::addSlide(*d, *layout, 0);
        QCOMPARE(sectionCounts(*d), (QList<int>{4, 2, 2}));
        deck::addSlide(*d, *layout);
        QCOMPARE(sectionCounts(*d), (QList<int>{4, 2, 3}));
        QCOMPARE(int(d->pages.size()), 9);
        // A copy stands right after its slide, in its section.
        deck::duplicateSlide(*d, 3);
        QCOMPARE(sectionCounts(*d), (QList<int>{5, 2, 3}));
        deck::duplicateSlide(*d, 5);
        QCOMPARE(sectionCounts(*d), (QList<int>{5, 3, 3}));
        // Slides taken away.
        d->pages.removeAt(9);
        deck::slideRemoved(*d, 9);
        QCOMPARE(sectionCounts(*d), (QList<int>{5, 3, 2}));
        d->pages.removeAt(0);
        deck::slideRemoved(*d, 0);
        QCOMPARE(sectionCounts(*d), (QList<int>{4, 3, 2}));

        // Moves: a slide dropped between two sections' slides takes the one before it, or the section it is dropped into.
        auto fresh = lettered(6);
        deck::addSection(*fresh, 2, QStringLiteral("Plan"));    // AB | CDEF
        deck::addSection(*fresh, 4, QStringLiteral("Budget"));  // AB | CD | EF
        auto move = [&](const QList<int> &slides, int before, int into = -1) {
            const QList<int> order = deck::orderAfterMoving(int(fresh->pages.size()), slides, before);
            const QVector<std::shared_ptr<Page>> old = fresh->pages;
            for (int k = 0; k < order.size(); ++k) fresh->pages[k] = old[order[k]];
            deck::slidesReordered(*fresh, order, slides, into);
        };
        move({0}, 3);   // A between C and D: Plan
        QCOMPARE(letters(*fresh), QStringLiteral("BCADEF"));
        QCOMPARE(sectionCounts(*fresh), (QList<int>{1, 3, 2}));
        move({2}, 4, 2);   // A to the start of Budget (before E), by the section it is dropped into
        QCOMPARE(letters(*fresh), QStringLiteral("BCDAEF"));
        QCOMPARE(sectionCounts(*fresh), (QList<int>{1, 2, 3}));
        move({5}, 0);   // F to the very start: the first section
        QCOMPARE(letters(*fresh), QStringLiteral("FBCDAE"));
        QCOMPARE(sectionCounts(*fresh), (QList<int>{2, 2, 2}));
        move({0, 1}, 6, 0);   // two to the end, asked into the first section: only where they could be, after the last slide's
        QCOMPARE(letters(*fresh), QStringLiteral("CDAEFB"));
        QCOMPARE(sectionCounts(*fresh), (QList<int>{0, 2, 4}));
        QCOMPARE(deck::orderAfterMoving(5, {3, 1, 3, 9}, 0), (QList<int>{1, 3, 0, 2, 4}));

        // A list made again out of the slides it had (the outline does): those that keep their order keep their
        // sections, a moved one and a new one join the section of the slide before them, and a slide left out is gone.
        auto again = lettered(6);
        deck::addSection(*again, 2, QStringLiteral("Plan"));
        deck::addSection(*again, 4, QStringLiteral("Budget"));   // AB | CD | EF
        const deck::SectionMemory memory = deck::rememberSections(*again);
        QVERIFY(!memory.isEmpty());
        auto added = std::make_shared<Page>();
        added->title = QStringLiteral("N");
        const QVector<std::shared_ptr<Page>> was = again->pages;
        again->pages = QVector<std::shared_ptr<Page>>{was[2], was[0], was[1], was[3], added, was[5]};
        deck::restoreSections(*again, memory);
        QCOMPARE(letters(*again), QStringLiteral("CABDNF"));
        QCOMPARE(sectionCounts(*again), (QList<int>{3, 2, 1}));
        QVERIFY(deck::rememberSections(*lettered(3)).isEmpty());   // none to keep without sections
        QCOMPARE(deck::orderAfterMoving(5, {0}, 5), (QList<int>{1, 2, 3, 4, 0}));

        // Counts that don't add up are made to: slides they miss join the last section, counts past the end are cut.
        auto odd = lettered(5);
        odd->appData[QStringLiteral("sections")] = QJsonArray{QJsonObject{{"id", "{A}"}, {"name", "One"}, {"count", 2}}, QJsonObject{{"id", "{B}"}, {"name", "Two"}, {"count", 1}}};
        QCOMPARE(sectionCounts(*odd), (QList<int>{2, 3}));
        odd->appData[QStringLiteral("sections")] = QJsonArray{QJsonObject{{"id", "{A}"}, {"name", "One"}, {"count", 4}}, QJsonObject{{"id", "{B}"}, {"name", "Two"}, {"count", 4}}};
        QCOMPARE(sectionCounts(*odd), (QList<int>{4, 1}));
        deck::setSections(*odd, {});
        QVERIFY(!deck::hasSections(*odd));
    }

private:
    // The names of the actions a menu offers, in order (separators left out).
    static QStringList menuTexts(const QMenu *menu)
    {
        QStringList out;
        for (const QAction *a : menu->actions())
            if (!a->isSeparator()) out << a->text();
        return out;
    }
    // Answers the Rename Section box: what the name shows when it opens, then the name to give (cancel when empty).
    static void answerRename(QString *shown, QString *selected, const QString &name)
    {
        QTimer::singleShot(0, [shown, selected, name] {
            untilDone([shown, selected, name] {
                auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!dlg) return false;
                auto *edit = dlg->findChild<QLineEdit *>();
                *shown = edit->text();
                *selected = edit->selectedText();
                if (name.isEmpty()) {
                    dlg->reject();
                } else {
                    edit->setText(name);
                    dlg->accept();
                }
                return true;
            });
        });
    }

private Q_SLOTS:
    // Every section command is one undo step; Rename asks for the name with it selected; Add Section is at
    // the slide being edited, and the others are on its section.
    void sectionCommandsAreOneUndoStepEach()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"A", "B", "C", "D", "E", "F"});
        Editor *ed = w.editor();
        Document *d = ed->doc();
        for (const char *id : {"section.rename", "section.remove", "section.removeAll", "section.collapseAll", "section.expandAll"}) QVERIFY(!w.act(id)->isEnabled());
        QVERIFY(w.act("section.add")->isEnabled());
        ed->setCurrentPage(2);
        w.act("section.add")->trigger();
        QCOMPARE(sectionNames(*d), (QStringList{"Default Section", "Untitled Section"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 4}));
        QCOMPARE(ed->undoStack()->count(), 1);
        QCOMPARE(ed->currentPage(), 2);
        for (const char *id : {"section.rename", "section.remove", "section.removeAll", "section.collapseAll", "section.expandAll"}) QTRY_VERIFY(w.act(id)->isEnabled());
        ed->undo();
        QVERIFY(!deck::hasSections(*d));
        ed->redo();
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 4}));

        // Rename: the box opens with the name selected; Cancel changes nothing.
        QString shown, selected;
        answerRename(&shown, &selected, QString());
        w.act("section.rename")->trigger();
        QCOMPARE(shown, QStringLiteral("Untitled Section"));
        QCOMPARE(selected, shown);
        QCOMPARE(ed->undoStack()->count(), 1);
        answerRename(&shown, &selected, QStringLiteral("Budget"));
        w.act("section.rename")->trigger();
        QCOMPARE(sectionNames(*d), (QStringList{"Default Section", "Budget"}));
        QCOMPARE(ed->undoStack()->count(), 2);
        ed->setCurrentPage(0);
        answerRename(&shown, &selected, QStringLiteral("Intro"));
        w.act("section.rename")->trigger();
        QCOMPARE(shown, QStringLiteral("Default Section"));
        QCOMPARE(sectionNames(*d), (QStringList{"Intro", "Budget"}));
        ed->undo();
        QCOMPARE(sectionNames(*d), (QStringList{"Default Section", "Budget"}));
        ed->redo();

        // Moving a section takes its slides; the current slide stays the one being edited.
        ed->setCurrentPage(3);
        w.moveSection(1, -1);
        QCOMPARE(sectionNames(*d), (QStringList{"Budget", "Intro"}));
        QCOMPARE(titlesOf(*d), (QStringList{"C", "D", "E", "F", "A", "B"}));
        QCOMPARE(ed->currentPage(), 1);
        QCOMPARE(ed->undoStack()->count(), 4);
        ed->undo();
        QCOMPARE(titlesOf(*d), (QStringList{"A", "B", "C", "D", "E", "F"}));
        QCOMPARE(ed->currentPage(), 3);
        w.moveSection(0, 1);
        QCOMPARE(titlesOf(*d), (QStringList{"C", "D", "E", "F", "A", "B"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{4, 2}));
        w.moveSection(0, -1);   // the first can't go up: no step
        QCOMPARE(ed->undoStack()->count(), 4);
        ed->undo();
        QCOMPARE(sectionNames(*d), (QStringList{"Intro", "Budget"}));
        QCOMPARE(titlesOf(*d), (QStringList{"A", "B", "C", "D", "E", "F"}));
        ed->undoStack()->clear();

        // Remove Section is on the section of the slide being edited; its slides join the one before.
        ed->setCurrentPage(4);
        w.act("section.remove")->trigger();
        QCOMPARE(sectionNames(*d), (QStringList{"Intro"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{6}));
        QCOMPARE(ed->undoStack()->count(), 1);
        ed->undo();
        QCOMPARE(sectionNames(*d), (QStringList{"Intro", "Budget"}));
        // Remove Section & Slides takes the slides too, and the editor lands on a slide that is there.
        ed->setCurrentPage(5);
        w.removeSectionAndSlides(1);
        QCOMPARE(titlesOf(*d), (QStringList{"A", "B"}));
        QCOMPARE(sectionNames(*d), (QStringList{"Intro"}));
        QCOMPARE(ed->currentPage(), 1);
        QCOMPARE(ed->undoStack()->count(), 1);
        ed->undo();
        QCOMPARE(titlesOf(*d), (QStringList{"A", "B", "C", "D", "E", "F"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 4}));
        // The last section holding every slide can't take them all with it.
        ed->undoStack()->clear();
        w.removeSectionAndSlides(0);
        QCOMPARE(titlesOf(*d), (QStringList{"C", "D", "E", "F"}));
        QCOMPARE(sectionNames(*d), (QStringList{"Budget"}));
        w.removeSectionAndSlides(0);
        QCOMPARE(int(d->pages.size()), 4);
        QCOMPARE(ed->undoStack()->count(), 1);
        w.act("section.removeAll")->trigger();
        QVERIFY(!deck::hasSections(*d));
        QCOMPARE(ed->undoStack()->count(), 2);
        ed->undo();
        QCOMPARE(sectionCounts(*d), (QList<int>{4}));
    }

    // New, duplicated, deleted, and moved slides keep the sections' counts right, whichever view
    // does it, each as the one step it was.
    void sectionCountsFollowSlidesChangedInTheWindow()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"A", "B", "C", "D", "E", "F"});
        Editor *ed = w.editor();
        Document *d = ed->doc();
        ed->setCurrentPage(2);
        w.act("section.add")->trigger();   // AB | CDEF
        ed->setCurrentPage(4);
        w.act("section.add")->trigger();   // AB | CD | EF
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 2, 2}));
        ed->undoStack()->clear();
        // A new slide after the last slide of a section is in that section.
        ed->setCurrentPage(3);
        w.newSlide();
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 3, 2}));
        QCOMPARE(ed->currentPage(), 4);
        ed->undo();
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 2, 2}));
        ed->redo();
        w.duplicateSlide();
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 4, 2}));
        QCOMPARE(ed->undoStack()->count(), 2);
        w.deleteSlide();
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 3, 2}));
        QCOMPARE(ed->undoStack()->count(), 3);
        ed->undo();
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 4, 2}));
        ed->undo();
        ed->undo();
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 2, 2}));
        QCOMPARE(titlesOf(*d), (QStringList{"A", "B", "C", "D", "E", "F"}));
        ed->undoStack()->clear();

        // The slide sorter: deleting, duplicating, and moving (the last slide dropped under a header joins its section).
        w.act("view.sorter")->trigger();
        SlideSorter *s = w.slideSorter();
        s->select({3, 4}, 4);
        s->deleteSelected();
        QCOMPARE(titlesOf(*d), (QStringList{"A", "B", "C", "F"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 1, 1}));
        QCOMPARE(ed->undoStack()->count(), 1);
        s->select({0}, 0);
        s->duplicateSelected();
        QCOMPARE(sectionCounts(*d), (QList<int>{3, 1, 1}));
        s->moveSlides({4}, 1);   // F to between A and its copy: the first section
        QCOMPARE(titlesOf(*d), (QStringList{"A", "F", "A", "B", "C"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{4, 1, 0}));
        s->moveSlides({0}, 5, 2);   // A to the end, asked into the last section
        QCOMPARE(titlesOf(*d), (QStringList{"F", "A", "B", "C", "A"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{3, 1, 1}));
        QCOMPARE(ed->undoStack()->count(), 4);
        ed->undo();
        QCOMPARE(sectionCounts(*d), (QList<int>{4, 1, 0}));
        w.act("view.normal")->trigger();
    }

    // Enter on a title makes a slide in its section, and a slide folded into
    // the one before leaves its section.
    void sectionCountsFollowTheOutline()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"A", "B", "C", "D"});
        Editor *ed = w.editor();
        Document *d = ed->doc();
        ed->setCurrentPage(2);
        w.act("section.add")->trigger();   // AB | CD
        ed->undoStack()->clear();
        w.act("view.outline")->trigger();
        OutlineView *o = w.outlineView();
        auto lineAt = [&](int block) {
            QTextCursor c(o->document()->findBlockByNumber(block));
            c.movePosition(QTextCursor::EndOfBlock);
            o->setTextCursor(c);
        };
        lineAt(1);
        QTest::keyClick(o, Qt::Key_Return);
        QCOMPARE(titlesOf(*d), (QStringList{"A", "B", "", "C", "D"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{3, 2}));
        QCOMPARE(ed->undoStack()->count(), 1);
        ed->undo();
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 2}));
        lineAt(2);   // "C", the first of its section
        QTest::keyClick(o, Qt::Key_Tab);
        QCOMPARE(titlesOf(*d), (QStringList{"A", "B", "D"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 1}));
        ed->undo();
        QCOMPARE(titlesOf(*d), (QStringList{"A", "B", "C", "D"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 2}));
        lineAt(3);
        QTest::keyClick(o, Qt::Key_Return);
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 3}));
    }

    // The slides pane puts a heading row before each section's slides; a click on it folds the section
    // away, and its menu has the section's commands.
    void slidesPaneGroupsSlidesUnderSectionHeaders()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"One", "Two", "Three", "Four", "Five"});
        Editor *ed = w.editor();
        SlidesPane *pane = w.slidesPane();
        QCOMPARE(pane->count(), 5);   // no sections: one row a slide
        QVERIFY(!pane->isHeaderRow(0));
        QCOMPARE(pane->rowOfSlide(3), 3);
        QCOMPARE(pane->slideOfRow(3), 3);
        ed->setCurrentPage(2);
        w.act("section.add")->trigger();   // One Two | Three Four Five
        QString shown, selected;
        answerRename(&shown, &selected, QStringLiteral("Budget"));
        w.act("section.rename")->trigger();
        QTRY_COMPARE(pane->count(), 7);
        QVERIFY(pane->isHeaderRow(0) && pane->isHeaderRow(3));
        QCOMPARE(pane->headerRow(0), 0);
        QCOMPARE(pane->headerRow(1), 3);
        QCOMPARE(pane->item(0)->text(), QStringLiteral("Default Section"));
        QCOMPARE(pane->item(3)->text(), QStringLiteral("Budget"));
        QCOMPARE(pane->item(0)->data(SlidesPane::CountRole).toInt(), 2);
        QCOMPARE(pane->item(3)->data(SlidesPane::CountRole).toInt(), 3);
        QCOMPARE(pane->rowOfSlide(0), 1);
        QCOMPARE(pane->rowOfSlide(1), 2);
        QCOMPARE(pane->rowOfSlide(2), 4);
        QCOMPARE(pane->slideOfRow(4), 2);
        QCOMPARE(pane->slideOfRow(3), -1);
        QCOMPARE(pane->item(pane->rowOfSlide(2))->text(), QStringLiteral("3"));   // numbers run on through the sections
        QCOMPARE(pane->currentRow(), 4);
        // Clicking a thumbnail goes to its slide; the editor's slide is the one marked.
        QTest::mouseClick(pane->viewport(), Qt::LeftButton, Qt::NoModifier, pane->thumbnailRect(1).center());
        QCOMPARE(ed->currentPage(), 1);
        QCOMPARE(pane->currentRow(), pane->rowOfSlide(1));
        ed->setCurrentPage(3);
        QCOMPARE(pane->currentRow(), pane->rowOfSlide(3));
        // Clicking a heading folds its section away, and again brings it back; the slide being edited stays.
        QVERIFY(!pane->isCollapsed(0));
        QTest::mouseClick(pane->viewport(), Qt::LeftButton, Qt::NoModifier, pane->headerRect(0).center());
        QVERIFY(pane->isCollapsed(0));
        QVERIFY(pane->isRowHidden(pane->rowOfSlide(0)) && pane->isRowHidden(pane->rowOfSlide(1)));
        QVERIFY(pane->thumbnailRect(0).isNull());
        QVERIFY(!pane->thumbnailRect(2).isNull());
        QCOMPARE(ed->currentPage(), 3);
        QCOMPARE(ed->undoStack()->count(), 2);   // folding is not a change to the presentation
        QVERIFY(w.slideSorter()->isCollapsed(0));   // the Slide Sorter folds the same way
        QTest::mouseClick(pane->viewport(), Qt::LeftButton, Qt::NoModifier, pane->headerRect(0).center());
        QVERIFY(!pane->isCollapsed(0) && !pane->isRowHidden(pane->rowOfSlide(0)));
        QVERIFY(!w.slideSorter()->isCollapsed(0));
        w.act("section.collapseAll")->trigger();
        QVERIFY(pane->isCollapsed(0) && pane->isCollapsed(1));
        QVERIFY(w.slideSorter()->isCollapsed(0) && w.slideSorter()->isCollapsed(1));
        // A slide chosen from elsewhere opens its section.
        ed->setCurrentPage(0);
        QVERIFY(!pane->isCollapsed(0) && pane->isCollapsed(1));
        QVERIFY(!pane->thumbnailRect(0).isNull());
        w.act("section.expandAll")->trigger();
        QVERIFY(!pane->isCollapsed(0) && !pane->isCollapsed(1));
        QCOMPARE(ed->undoStack()->count(), 2);

        // The menu of a heading, and of a slide.
        QStringList texts;
        QList<bool> enabled;
        rightClick(pane->viewport(), pane->headerRect(1).center(), [&](QMenu *m) {
            texts = menuTexts(m);
            for (const QAction *a : m->actions())
                if (!a->isSeparator()) enabled << a->isEnabled();
        });
        QCOMPARE(texts, (QStringList{"Rename Section", "Remove Section", "Remove Section && Slides", "Move Section Up", "Move Section Down", "Collapse All", "Expand All"}));
        QCOMPARE(enabled, (QList<bool>{true, true, true, true, false, true, true}));   // the last section can't go down
        rightClick(pane->viewport(), pane->headerRect(0).center(), [&](QMenu *m) {
            enabled.clear();
            for (const QAction *a : m->actions())
                if (!a->isSeparator()) enabled << a->isEnabled();
        });
        QCOMPARE(enabled, (QList<bool>{true, true, true, false, true, true, true}));
        rightClick(pane->viewport(), pane->thumbnailRect(1).center(), [&](QMenu *m) { texts = menuTexts(m); });
        QVERIFY(texts.contains(QStringLiteral("Add Section")) && texts.contains(QStringLiteral("Duplicate Slide")));
        // Move Section Up, from the menu, takes the section's slides along.
        rightClick(pane->viewport(), pane->headerRect(1).center(), [&](QMenu *m) {
            for (QAction *a : m->actions())
                if (a->text() == QLatin1String("Move Section Up")) a->trigger();
        });
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"Three", "Four", "Five", "One", "Two"}));
        QCOMPARE(sectionNames(*ed->doc()), (QStringList{"Budget", "Default Section"}));
        QCOMPARE(ed->undoStack()->count(), 3);
        QTRY_COMPARE(pane->item(0)->text(), QStringLiteral("Budget"));
        // Remove Section from the menu, and Remove Section & Slides.
        rightClick(pane->viewport(), pane->headerRect(0).center(), [&](QMenu *m) {
            for (QAction *a : m->actions())
                if (a->text() == QLatin1String("Remove Section && Slides")) a->trigger();
        });
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"One", "Two"}));
        QCOMPARE(sectionNames(*ed->doc()), (QStringList{"Default Section"}));
        QTRY_COMPARE(pane->count(), 3);
        ed->undo();
        QTRY_COMPARE(pane->count(), 7);
        // Dragging a thumbnail to another section's place (what a drop does): it joins the section it lands in.
        pane->moveSlide(0, 3, 1);
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"Four", "Five", "One", "Three", "Two"}));
        QCOMPARE(sectionCounts(*ed->doc()), (QList<int>{2, 3}));
        QCOMPARE(ed->undoStack()->count(), 4);
        ed->undo();
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"Three", "Four", "Five", "One", "Two"}));
        QCOMPARE(sectionCounts(*ed->doc()), (QList<int>{3, 2}));
        // Put under the next heading without changing places (the last of a section to the first of the next), it changes section.
        pane->moveSlide(2, 3, 1);
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"Three", "Four", "One", "Five", "Two"}));
        ed->undo();
        pane->moveSlide(2, 2, 1);
        QCOMPARE(titlesOf(*ed->doc()), (QStringList{"Three", "Four", "Five", "One", "Two"}));
        QCOMPARE(sectionCounts(*ed->doc()), (QList<int>{2, 3}));
        pane->moveSlide(2, 2, 1);   // where it is: nothing
        QCOMPARE(ed->undoStack()->count(), 4);
        ed->undo();
        // Up and Down in the pane go from slide to slide, past the slides of a section folded away.
        pane->setFocus();
        ed->setCurrentPage(2);   // the last of the first section
        pane->setCollapsed(1, true);
        QTest::keyClick(pane, Qt::Key_Down);
        QCOMPARE(ed->currentPage(), 2);
        pane->setCollapsed(1, false);
        QTest::keyClick(pane, Qt::Key_Down);
        QCOMPARE(ed->currentPage(), 3);
        QTest::keyClick(pane, Qt::Key_Home);
        QCOMPARE(ed->currentPage(), 0);
        QTest::keyClick(pane, Qt::Key_End);
        QCOMPARE(ed->currentPage(), 4);
    }

    // The slide sorter starts each section on a row of its own under its heading; a heading folds
    // its section, and a slide dropped among a section's slides is in the section.
    void slideSorterGroupsThumbnailsUnderSectionHeaders()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"One", "Two", "Three", "Four", "Five", "Six"});
        Editor *ed = w.editor();
        Document *d = ed->doc();
        ed->setCurrentPage(2);
        w.act("section.add")->trigger();
        ed->setCurrentPage(5);
        w.act("section.add")->trigger();   // One Two | Three Four Five | Six
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 3, 1}));
        w.act("view.sorter")->trigger();
        SlideSorter *s = w.slideSorter();
        QTRY_COMPARE(s->renderedCount(), 6);
        auto row = [&](int slide) { return s->thumbnailRect(slide).top(); };
        // A heading above each section's first row; a section's slides in rows from there.
        for (int section = 0; section < 3; ++section) {
            QVERIFY(!s->sectionHeaderRect(section).isNull());
            QCOMPARE(s->headerAt(s->sectionHeaderRect(section).center()), section);
            QCOMPARE(s->slideAt(s->sectionHeaderRect(section).center()), -1);
        }
        QVERIFY(s->sectionHeaderRect(0).bottom() < row(0));
        QCOMPARE(row(1), row(0));
        QVERIFY(row(2) > row(1) + s->thumbnailRect(1).height());
        QVERIFY(s->sectionHeaderRect(1).top() >= s->thumbnailRect(1).bottom());
        QVERIFY(s->sectionHeaderRect(1).bottom() < row(2));
        QCOMPARE(row(3), row(2));
        QCOMPARE(row(4), row(2));
        QVERIFY(row(5) > row(4) + s->thumbnailRect(4).height());
        QVERIFY(s->sectionHeaderRect(2).bottom() < row(5));
        QCOMPARE(s->slideAt(centerOf(s, 4)), 4);
        // Folding: the slides go and the rest comes up.
        const int before = row(5);
        QTest::mouseClick(s->viewport(), Qt::LeftButton, Qt::NoModifier, s->sectionHeaderRect(1).center());
        QVERIFY(s->isCollapsed(1));
        QVERIFY(s->thumbnailRect(3).isNull());
        QCOMPARE(s->slideAt(QPoint(5, 5)), -1);
        QVERIFY(row(5) < before);
        QVERIFY(w.slidesPane()->isCollapsed(1));
        s->setCollapsed(1, false);
        QCOMPARE(row(5), before);
        QVERIFY(!w.slidesPane()->isCollapsed(1));
        // Dragging "Six" to the left of the first slide puts it first in the first section.
        QTRY_COMPARE(s->renderedCount(), 6);
        QWidget *vp = s->viewport();
        const QPoint from = centerOf(s, 5), to(s->thumbnailRect(0).left() + 3, s->thumbnailRect(0).center().y());
        ed->undoStack()->clear();
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(vp, from + QPoint(-12, 0));
        QTest::mouseMove(vp, from + QPoint(-40, 0));
        QTest::mouseMove(vp, to);
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, to);
        QCOMPARE(titlesOf(*d), (QStringList{"Six", "One", "Two", "Three", "Four", "Five"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{3, 3, 0}));
        QCOMPARE(ed->undoStack()->count(), 1);
        ed->undo();
        QCOMPARE(sectionCounts(*d), (QList<int>{2, 3, 1}));
        // Dropped at the right of a section's last slide, it is that section's (not the next one's first).
        QTRY_COMPARE(s->renderedCount(), 6);
        const QRect last = s->thumbnailRect(4);
        const QPoint far(last.right() + 8, last.center().y());
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, centerOf(s, 0));
        QTest::mouseMove(vp, centerOf(s, 0) + QPoint(14, 14));
        QTest::mouseMove(vp, centerOf(s, 0) + QPoint(60, 40));
        QTest::mouseMove(vp, far);
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, far);
        QCOMPARE(titlesOf(*d), (QStringList{"Two", "Three", "Four", "Five", "One", "Six"}));
        QCOMPARE(sectionCounts(*d), (QList<int>{1, 4, 1}));
        ed->undo();
        // A heading's menu is the section's, as in the slides pane.
        QStringList texts;
        rightClick(vp, s->sectionHeaderRect(2).center(), [&](QMenu *m) { texts = menuTexts(m); });
        QCOMPARE(texts, (QStringList{"Rename Section", "Remove Section", "Remove Section && Slides", "Move Section Up", "Move Section Down", "Collapse All", "Expand All"}));
        rightClick(vp, s->thumbnailRect(2).center(), [&](QMenu *m) { texts = menuTexts(m); });
        QVERIFY(texts.contains(QStringLiteral("Add Section")));
        // Left and Right go from slide to slide across the sections; Down to the row below.
        ed->setCurrentPage(1);
        QTest::keyClick(s, Qt::Key_Right);
        QCOMPARE(ed->currentPage(), 2);
        QTest::keyClick(s, Qt::Key_Left);
        QCOMPARE(ed->currentPage(), 1);
        QTest::keyClick(s, Qt::Key_Down);
        QCOMPARE(ed->currentPage(), 3);   // the nearest slide of the section below
        QTest::keyClick(s, Qt::Key_Down);
        QCOMPARE(ed->currentPage(), 5);
        QTest::keyClick(s, Qt::Key_Up);
        QCOMPARE(ed->currentPage(), 2);   // the one across from "Six", at the left
    }

    // The Home tab's Section button: Add, Rename, Remove, Remove All, Collapse All, and Expand All.
    void ribbonSectionMenu()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QAction *command = w.findChild<QAction *>(QStringLiteral("ribbon.section"));
        QVERIFY(command);
        QMenu *menu = nullptr;
        for (QObject *o : command->associatedObjects())
            if (auto *button = qobject_cast<QToolButton *>(o)) menu = button->menu();
        QVERIFY(menu);
        QCOMPARE(menu->actions(), (QList<QAction *>{w.act("section.add"), w.act("section.rename"), w.act("section.remove"), w.act("section.removeAll"), menu->actions().at(4),
                                                    w.act("section.collapseAll"), w.act("section.expandAll")}));
        QVERIFY(menu->actions().at(4)->isSeparator());
        QCOMPARE(w.act("section.add")->text(), QStringLiteral("Add Section"));
        QCOMPARE(w.act("section.rename")->text(), QStringLiteral("Rename Section"));
        QCOMPARE(w.act("section.remove")->text(), QStringLiteral("Remove Section"));
        QCOMPARE(w.act("section.removeAll")->text(), QStringLiteral("Remove All Sections"));
        QCOMPARE(w.act("section.collapseAll")->text(), QStringLiteral("Collapse All"));
        QCOMPARE(w.act("section.expandAll")->text(), QStringLiteral("Expand All"));
        // In the Slides group, after Reset.
        RibbonGroup *slides = nullptr;
        for (RibbonGroup *g : w.ribbon()->tab(QStringLiteral("Home"))->groups())
            if (g->title() == QLatin1String("Slides")) slides = g;
        QVERIFY(slides);
        QList<QToolButton *> buttons = slides->findChildren<QToolButton *>();
        int reset = -1, section = -1;
        for (int i = 0; i < buttons.size(); ++i) {
            if (buttons[i]->defaultAction() == w.act("slide.reset")) reset = i;
            if (buttons[i]->defaultAction() == command) section = i;
        }
        QVERIFY(reset >= 0 && section > reset);
    }

    // Outline view: the presentation's text, each slide a numbered heading
    // with its title in bold and its body lines under it, by level.
    void outlineShowsTitlesAndBodyLines()
    {
        setUiDark(false);
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"Quarterly Review", "Plan", "Budget"}, {{"Subtitle text"}, {"Goals", "Risks"}, {"Spend"}});
        QVERIFY(w.outlineView()->isHidden());
        QVERIFY(w.act("view.outline")->isEnabled());
        QVERIFY(!w.act("view.notesPage")->isEnabled() && w.act("view.notesPage")->toolTip().contains(QStringLiteral("coming")));
        QVERIFY(!w.act("view.reading")->isEnabled() && w.act("view.reading")->toolTip().contains(QStringLiteral("coming")));
        w.act("view.outline")->trigger();
        OutlineView *o = w.outlineView();
        QCOMPARE(w.view(), DeckWindow::View::Outline);
        QVERIFY(o->isVisible() && w.canvas()->isVisible() && w.slidesPane()->isHidden());
        QVERIFY(w.act("view.outline")->isChecked());
        QCOMPARE(o->toPlainText(), QStringLiteral("Quarterly Review\nSubtitle text\nPlan\nGoals\nRisks\nBudget\nSpend"));
        // Titles bold at the left; body lines indented under them.
        QTextDocument *doc = o->document();
        auto weight = [&](int line) { return doc->findBlockByNumber(line).begin().fragment().charFormat().fontWeight(); };
        QVERIFY(weight(0) >= QFont::DemiBold && weight(2) >= QFont::DemiBold && weight(5) >= QFont::DemiBold);
        QVERIFY(weight(1) < QFont::DemiBold && weight(3) < QFont::DemiBold);
        QVERIFY(doc->findBlockByNumber(3).blockFormat().leftMargin() > doc->findBlockByNumber(2).blockFormat().leftMargin());
        // The text follows a change made elsewhere, and the cursor tells the current slide.
        Editor *ed = w.editor();
        auto *plan = static_cast<TextItem *>(ed->doc()->pages[1]->items[0].get());
        ed->doc()->storyDoc(plan->storyId)->clear();
        QTextCursor typed(ed->doc()->storyDoc(plan->storyId));
        typed.insertText(QStringLiteral("Roadmap"));
        ed->notifyLive();
        QTRY_COMPARE(o->toPlainText(), QStringLiteral("Quarterly Review\nSubtitle text\nRoadmap\nGoals\nRisks\nBudget\nSpend"));
        o->goToSlide(2);
        QCOMPARE(o->slideAtCursor(), 2);
        QCOMPARE(ed->currentPage(), 2);
        QTextCursor c = o->textCursor();
        c.setPosition(doc->findBlockByNumber(4).position());   // "Risks"
        o->setTextCursor(c);
        QCOMPARE(ed->currentPage(), 1);
        ed->setCurrentPage(2);   // the slide chosen elsewhere takes the cursor
        QCOMPARE(o->slideAtCursor(), 2);
    }

    // Typing in the outline edits the slide's placeholder as one undo step
    // once typing pauses, keeping the text's formatting.
    void typingInTheOutlineEditsTheSlide()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"Quarterly Review", "Plan", "Budget"}, {{"Subtitle text"}, {"Goals", "Risks"}, {"Spend"}});
        Editor *ed = w.editor();
        w.act("view.outline")->trigger();
        OutlineView *o = w.outlineView();
        const QString before = o->toPlainText();
        QTextCursor c(o->document()->findBlockByNumber(2));
        c.movePosition(QTextCursor::EndOfBlock);
        o->setTextCursor(c);
        QTest::keyClicks(o, " B");
        QVERIFY(o->hasTyping());
        QCOMPARE(titleOfSlide(*ed->doc(), 1), QStringLiteral("Plan"));   // waits for a pause
        o->commit();
        QVERIFY(!o->hasTyping());
        QCOMPARE(titleOfSlide(*ed->doc(), 1), QStringLiteral("Plan B"));
        QCOMPARE(ed->undoStack()->count(), 1);
        const auto *title = static_cast<TextItem *>(ed->doc()->pages[1]->items[0].get());
        QCOMPARE(ed->doc()->storyDoc(title->storyId)->begin().begin().fragment().charFormat().fontPointSize(), 44.0);
        ed->undo();
        QCOMPARE(titleOfSlide(*ed->doc(), 1), QStringLiteral("Plan"));
        QCOMPARE(o->toPlainText(), before);
        ed->redo();
        QCOMPARE(titleOfSlide(*ed->doc(), 1), QStringLiteral("Plan B"));
        QCOMPARE(o->toPlainText(), QStringLiteral("Quarterly Review\nSubtitle text\nPlan B\nGoals\nRisks\nBudget\nSpend"));
        // A body line, and a pause saves it without asking.
        c = QTextCursor(o->document()->findBlockByNumber(4));
        c.movePosition(QTextCursor::EndOfBlock);
        o->setTextCursor(c);
        QTest::keyClicks(o, "!");
        QTRY_VERIFY(!o->hasTyping());
        QCOMPARE(bodyOfSlide(*ed->doc(), 1)->toPlainText(), QStringLiteral("Goals\nRisks!"));
        QCOMPARE(ed->undoStack()->count(), 2);
        // Ctrl+Z in the outline undoes the saved step; the line goes back.
        QTest::keyClick(o, Qt::Key_Z, Qt::ControlModifier);
        QCOMPARE(bodyOfSlide(*ed->doc(), 1)->toPlainText(), QStringLiteral("Goals\nRisks"));
        QCOMPARE(o->toPlainText(), QStringLiteral("Quarterly Review\nSubtitle text\nPlan B\nGoals\nRisks\nBudget\nSpend"));
        // Clicking anywhere else, as for a command, saves what is waiting first.
        QTest::keyClicks(o, "?");
        QVERIFY(o->hasTyping());
        QTest::mouseClick(w.canvas()->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(2, 2));
        QVERIFY(!o->hasTyping());
        QCOMPARE(bodyOfSlide(*ed->doc(), 1)->toPlainText(), QStringLiteral("Goals\nRisks?"));
    }

    // Enter on a title line makes a Title and Content slide after that slide
    // (after its body lines), with the cursor on its title.
    void enterOnATitleMakesASlide()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"Quarterly Review", "Plan", "Budget"}, {{"Subtitle text"}, {"Goals", "Risks"}, {"Spend"}});
        Editor *ed = w.editor();
        Document *d = ed->doc();
        w.act("view.outline")->trigger();
        OutlineView *o = w.outlineView();
        QTextCursor c(o->document()->findBlockByNumber(2));
        c.movePosition(QTextCursor::EndOfBlock);
        o->setTextCursor(c);
        QTest::keyClick(o, Qt::Key_Return);
        QCOMPARE(int(d->pages.size()), 4);
        QCOMPARE(titlesOf(*d), (QStringList{"Quarterly Review", "Plan", "", "Budget"}));
        QCOMPARE(deck::layoutType(*d->masterFor(*d->pages[2])), QStringLiteral("obj"));
        QCOMPARE(bodyOfSlide(*d, 1)->toPlainText(), QStringLiteral("Goals\nRisks"));   // still Plan's
        QCOMPARE(ed->currentPage(), 2);
        QCOMPARE(o->slideAtCursor(), 2);
        QCOMPARE(o->toPlainText(), QStringLiteral("Quarterly Review\nSubtitle text\nPlan\nGoals\nRisks\n\nBudget\nSpend"));
        QCOMPARE(ed->undoStack()->count(), 1);
        QTest::keyClicks(o, "Risks");
        o->commit();
        QCOMPARE(titleOfSlide(*d, 2), QStringLiteral("Risks"));
        QCOMPARE(int(d->pages.size()), 4);
        const auto *made = static_cast<TextItem *>(d->pages[2]->items[0].get());   // typed in the way the title place looks
        QCOMPARE(d->storyDoc(made->storyId)->begin().begin().fragment().charFormat().fontPointSize(), 44.0);
        // In the middle of a title it splits: the rest goes to the new slide.
        c = QTextCursor(o->document()->findBlockByNumber(0));
        c.setPosition(c.block().position() + 9);
        o->setTextCursor(c);
        QTest::keyClick(o, Qt::Key_Return);
        QCOMPARE(titlesOf(*d), (QStringList{"Quarterly", " Review", "Plan", "Risks", "Budget"}));
        QCOMPARE(ed->currentPage(), 1);
        QCOMPARE(o->slideAtCursor(), 1);
        ed->undo();
        QCOMPARE(titlesOf(*d), (QStringList{"Quarterly Review", "Plan", "Risks", "Budget"}));
        ed->undo();
        QCOMPARE(titlesOf(*d), (QStringList{"Quarterly Review", "Plan", "", "Budget"}));
        ed->undo();
        QCOMPARE(int(d->pages.size()), 3);
        QCOMPARE(o->toPlainText(), QStringLiteral("Quarterly Review\nSubtitle text\nPlan\nGoals\nRisks\nBudget\nSpend"));
    }

    // Tab on a body line moves it a level in, Shift+Tab a level out (and at
    // the first level makes it a slide of its own); Tab on a title makes it a
    // line of the slide before.
    void tabChangesALinesLevel()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"Quarterly Review", "Plan", "Budget"}, {{"Subtitle text"}, {"Goals", "Risks"}, {"Spend"}});
        Editor *ed = w.editor();
        Document *d = ed->doc();
        w.act("view.outline")->trigger();
        OutlineView *o = w.outlineView();
        auto levelOf = [&](int slide, int line) { return bodyOfSlide(*d, slide)->findBlockByNumber(line).blockFormat().property(tp::ListLevel).toInt(); };
        auto lineAt = [&](int block) {
            QTextCursor c(o->document()->findBlockByNumber(block));
            c.movePosition(QTextCursor::EndOfBlock);
            o->setTextCursor(c);
        };
        lineAt(4);   // "Risks"
        const qreal flush = o->document()->findBlockByNumber(4).blockFormat().leftMargin();
        QTest::keyClick(o, Qt::Key_Tab);
        QCOMPARE(levelOf(1, 1), 1);
        QCOMPARE(levelOf(1, 0), 0);
        QCOMPARE(ed->undoStack()->count(), 1);
        QVERIFY(o->document()->findBlockByNumber(4).blockFormat().leftMargin() > flush);
        QCOMPARE(bodyOfSlide(*d, 1)->toPlainText(), QStringLiteral("Goals\nRisks"));
        QTest::keyClick(o, Qt::Key_Tab);
        QCOMPARE(levelOf(1, 1), 2);
        QTest::keyClick(o, Qt::Key_Backtab);
        QCOMPARE(levelOf(1, 1), 1);
        QTest::keyClick(o, Qt::Key_Backtab);
        QCOMPARE(levelOf(1, 1), 0);
        QCOMPARE(ed->undoStack()->count(), 4);
        for (int k = 0; k < 4; ++k) ed->undo();
        QCOMPARE(levelOf(1, 1), 0);
        ed->undoStack()->clear();
        // Shift+Tab at the first level: the line becomes a slide, after its own.
        lineAt(4);
        QTest::keyClick(o, Qt::Key_Backtab);
        QCOMPARE(titlesOf(*d), (QStringList{"Quarterly Review", "Plan", "Risks", "Budget"}));
        QCOMPARE(bodyOfSlide(*d, 1)->toPlainText(), QStringLiteral("Goals"));
        QCOMPARE(ed->currentPage(), 2);
        QCOMPARE(ed->undoStack()->count(), 1);
        ed->undo();
        QCOMPARE(titlesOf(*d), (QStringList{"Quarterly Review", "Plan", "Budget"}));
        QCOMPARE(bodyOfSlide(*d, 1)->toPlainText(), QStringLiteral("Goals\nRisks"));
        QCOMPARE(o->toPlainText(), QStringLiteral("Quarterly Review\nSubtitle text\nPlan\nGoals\nRisks\nBudget\nSpend"));
        // Tab on a title: it becomes a line of the slide before, its own lines a level deeper.
        lineAt(5);   // "Budget"
        QTest::keyClick(o, Qt::Key_Tab);
        QCOMPARE(int(d->pages.size()), 2);
        QCOMPARE(bodyOfSlide(*d, 1)->toPlainText(), QStringLiteral("Goals\nRisks\nBudget\nSpend"));
        QCOMPARE(levelOf(1, 2), 0);
        QCOMPARE(levelOf(1, 3), 1);
        QCOMPARE(ed->currentPage(), 1);
        ed->undo();
        QCOMPARE(titlesOf(*d), (QStringList{"Quarterly Review", "Plan", "Budget"}));
        // The first slide can't be one.
        lineAt(0);
        QTest::keyClick(o, Qt::Key_Tab);
        QCOMPARE(int(d->pages.size()), 3);
    }

    // A line typed under a slide that has no place for one gives it a layout
    // that has, as does a title typed on a slide with no place for a title.
    void outlineGivesASlideThePlaceholdersItNeeds()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"Quarterly Review", "Plan", "x"});
        Editor *ed = w.editor();
        Document *d = ed->doc();
        d->pages[1]->items.resize(1);   // a title only
        d->pages[1]->masterId = deck::layoutOfType(*d, QStringLiteral("titleOnly"))->id;
        d->pages[2]->items.clear();     // nothing at all
        d->pages[2]->masterId = deck::layoutOfType(*d, QStringLiteral("blank"))->id;
        ed->notifyLive();
        w.act("view.outline")->trigger();
        OutlineView *o = w.outlineView();
        QCOMPARE(o->toPlainText(), QStringLiteral("Quarterly Review\nPlan\n"));
        QTextCursor c(o->document()->findBlockByNumber(2));
        o->setTextCursor(c);
        QTest::keyClicks(o, "Budget");
        o->commit();
        QCOMPARE(titleOfSlide(*d, 2), QStringLiteral("Budget"));
        QCOMPARE(deck::layoutType(*d->masterFor(*d->pages[2])), QStringLiteral("titleOnly"));
        // Under Plan: its title only, so Tab on the next title (a line of Plan now) gives it a body.
        c = QTextCursor(o->document()->findBlockByNumber(2));
        o->setTextCursor(c);
        QTest::keyClick(o, Qt::Key_Tab);
        QCOMPARE(int(d->pages.size()), 2);
        QCOMPARE(deck::layoutType(*d->masterFor(*d->pages[1])), QStringLiteral("obj"));
        QVERIFY(bodyOfSlide(*d, 1));
        QCOMPARE(bodyOfSlide(*d, 1)->toPlainText(), QStringLiteral("Budget"));
        QCOMPARE(titleOfSlide(*d, 1), QStringLiteral("Plan"));
    }

    // Backspace at the start of a title joins it to the line before, which
    // takes the slide's lines with it; pasting lines makes lines.
    void joiningAndPastingLinesInTheOutline()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"Quarterly Review", "Plan", "Budget"}, {{"Subtitle text"}, {"Goals", "Risks"}, {"Spend"}});
        Editor *ed = w.editor();
        Document *d = ed->doc();
        w.act("view.outline")->trigger();
        OutlineView *o = w.outlineView();
        QTextCursor c(o->document()->findBlockByNumber(5));   // "Budget"
        o->setTextCursor(c);
        QTest::keyClick(o, Qt::Key_Backspace);
        QCOMPARE(titlesOf(*d), (QStringList{"Quarterly Review", "Plan"}));
        QCOMPARE(bodyOfSlide(*d, 1)->toPlainText(), QStringLiteral("Goals\nRisksBudget\nSpend"));
        QCOMPARE(ed->undoStack()->count(), 1);
        ed->undo();
        QCOMPARE(int(d->pages.size()), 3);
        QCOMPARE(o->toPlainText(), QStringLiteral("Quarterly Review\nSubtitle text\nPlan\nGoals\nRisks\nBudget\nSpend"));
        // Two lines pasted at the end of a body line: the first joins it, the second is a line of its own.
        QApplication::clipboard()->setText(QStringLiteral("x\ny"));
        c = QTextCursor(o->document()->findBlockByNumber(3));
        c.movePosition(QTextCursor::EndOfBlock);
        o->setTextCursor(c);
        QTest::keyClick(o, Qt::Key_V, Qt::ControlModifier);
        QCOMPARE(bodyOfSlide(*d, 1)->toPlainText(), QStringLiteral("Goalsx\ny\nRisks"));
        QCOMPARE(o->toPlainText(), QStringLiteral("Quarterly Review\nSubtitle text\nPlan\nGoalsx\ny\nRisks\nBudget\nSpend"));
        QCOMPARE(ed->undoStack()->count(), 1);   // in place of the undone join
        ed->undo();
        QCOMPARE(bodyOfSlide(*d, 1)->toPlainText(), QStringLiteral("Goals\nRisks"));
    }

    // Switching views keeps the current slide: the outline puts its cursor on
    // it, the sorter selects it, and what is chosen there is current in Normal.
    void switchingViewsKeepsTheCurrentSlide()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        fillSlides(w, {"One", "Two", "Three", "Four"}, {{}, {"a"}, {"b"}, {"c"}});
        Editor *ed = w.editor();
        ed->setCurrentPage(2);
        w.act("view.outline")->trigger();
        QCOMPARE(ed->currentPage(), 2);
        QCOMPARE(w.outlineView()->slideAtCursor(), 2);
        w.act("view.sorter")->trigger();
        QCOMPARE(ed->currentPage(), 2);
        QCOMPARE(w.slideSorter()->selection(), QList<int>({2}));
        QTest::mouseClick(w.slideSorter()->viewport(), Qt::LeftButton, Qt::NoModifier, centerOf(w.slideSorter(), 3));
        w.act("view.outline")->trigger();
        QCOMPARE(ed->currentPage(), 3);
        QCOMPARE(w.outlineView()->slideAtCursor(), 3);
        w.outlineView()->goToSlide(1);
        QCOMPARE(ed->currentPage(), 1);
        w.act("view.normal")->trigger();
        QCOMPARE(ed->currentPage(), 1);
        QVERIFY(w.slidesPane()->isVisible() && w.outlineView()->isHidden() && w.slideSorter()->isHidden());
        QCOMPARE(w.slidesPane()->currentRow(), 1);
        // The status bar's buttons are the same commands; only one view is checked.
        w.act("view.sorter")->trigger();
        int checked = 0;
        for (const char *id : {"view.normal", "view.outline", "view.sorter", "view.notesPage", "view.reading"}) checked += w.act(QString::fromLatin1(id))->isChecked();
        QCOMPARE(checked, 1);
        // Slides added in one view show in the others.
        w.act("view.normal")->trigger();
        w.newSlide();
        w.act("view.sorter")->trigger();
        QCOMPARE(w.slideSorter()->slideCount(), 5);
        w.act("view.outline")->trigger();
        QCOMPARE(w.outlineView()->toPlainText().split(QLatin1Char('\n')).size(), 8);
        w.act("view.normal")->trigger();
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
};

JP_TEST_CLASS(DeckSlideListTests)
#include "tst_deck_slidelist.moc"
