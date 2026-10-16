// The tests of the format pane's frame and property editors (libs/shell): the parts every app's Format pane is made of.
#include "pubhelpers.h"

class PubFormatPaneTests : public QObject {
    Q_OBJECT

private:
    // A frame with three pages, as the Format pane has.
    static void fill(PaneFrame &f)
    {
        f.setTitle(QStringLiteral("Format Shape"));
        f.addPage(QStringLiteral("fill"), icon(QStringLiteral("paint-bucket")), QStringLiteral("Fill & Line"))->addSection(QStringLiteral("Fill"));
        f.addPage(QStringLiteral("effects"), icon(QStringLiteral("sparkles")), QStringLiteral("Effects"))->addSection(QStringLiteral("Shadow"));
        f.addPage(QStringLiteral("size"), icon(QStringLiteral("scaling")), QStringLiteral("Size & Properties"))->addSection(QStringLiteral("Size"));
    }

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // A pane has a title and a close button, and icon tabs across the top with a name for the tool tip and for screen readers.
    void frameHasTitleCloseButtonAndIconTabs()
    {
        PaneFrame f;
        fill(f);
        QCOMPARE(f.title(), QStringLiteral("Format Shape"));
        QCOMPARE(f.pageCount(), 3);
        QCOMPARE(f.currentPage(), 0);
        QVERIFY(f.closeButton());
        QCOMPARE(f.closeButton()->accessibleName(), QStringLiteral("Close"));
        const QStringList names = {QStringLiteral("Fill & Line"), QStringLiteral("Effects"), QStringLiteral("Size & Properties")};
        for (int i = 0; i < 3; ++i) {
            QToolButton *tab = f.tabButton(i);
            QVERIFY(tab);
            QVERIFY(tab->text().isEmpty());   // only the icon shows
            QVERIFY(!tab->icon().isNull());
            QCOMPARE(tab->toolTip(), names[i]);
            QCOMPARE(tab->accessibleName(), names[i]);
        }
        QSignalSpy closed(&f, &PaneFrame::closeRequested);
        f.closeButton()->click();
        QCOMPARE(closed.count(), 1);
    }

    // A click on a tab opens its page; the arrow keys move from tab to tab, and the page follows.
    void tabsSwitchPagesByMouseAndArrowKeys()
    {
        PaneFrame f;
        fill(f);
        f.resize(320, 480);
        f.show();
        QVERIFY(QTest::qWaitForWindowActive(&f));
        QSignalSpy changed(&f, &PaneFrame::currentPageChanged);
        f.tabButton(1)->click();
        QCOMPARE(f.currentPage(), 1);
        QCOMPARE(changed.count(), 1);
        QVERIFY(f.page(1)->isVisibleTo(&f));
        QVERIFY(!f.page(0)->isVisibleTo(&f));
        // From the keyboard: the tab with the focus, then Right and Left.
        f.focusTabs();
        QTRY_VERIFY(f.tabButton(1)->hasFocus());
        QTest::keyClick(f.tabButton(1), Qt::Key_Right);
        QTRY_COMPARE(f.currentPage(), 2);
        QTest::keyClick(f.focusWidget(), Qt::Key_Left);
        QTRY_COMPARE(f.currentPage(), 1);
        QTest::keyClick(f.focusWidget(), Qt::Key_Left);
        QTRY_COMPARE(f.currentPage(), 0);
    }

    // The keyboard reaches every part of the pane in order: the close button, the tabs (the open one), then the headings of the sections.
    void keyboardReachesTheCloseButtonTheTabAndTheSections()
    {
        PaneFrame f;
        fill(f);
        f.resize(320, 480);
        f.show();
        QVERIFY(QTest::qWaitForWindowActive(&f));
        f.closeButton()->setFocus(Qt::TabFocusReason);
        QTRY_VERIFY(f.closeButton()->hasFocus());
        QTest::keyClick(f.focusWidget(), Qt::Key_Tab);
        QTRY_VERIFY2(f.tabButton(0)->hasFocus(), qPrintable(f.focusWidget() ? f.focusWidget()->objectName() : QStringLiteral("none")));
        QTest::keyClick(f.focusWidget(), Qt::Key_Tab);
        QTRY_VERIFY(f.page(0)->section(QStringLiteral("Fill"))->header()->hasFocus());
    }

    // A section folds up and opens again with Space or Enter on its heading, tells screen readers which it is, and says so when it changes.
    void sectionsFoldWithTheKeyboard()
    {
        PaneFrame f;
        fill(f);
        f.resize(320, 480);
        f.show();
        QVERIFY(QTest::qWaitForWindowActive(&f));
        PaneSection *s = f.page(0)->section(QStringLiteral("Fill"));
        auto *label = new QLabel(QStringLiteral("inside"));
        s->body()->addWidget(label);
        label->show();
        QVERIFY(s->isExpanded());
        QVERIFY(label->isVisibleTo(&f));
        const QString open = s->header()->accessibleDescription();
        QSignalSpy toggled(s, &PaneSection::toggled);
        s->header()->setFocus(Qt::TabFocusReason);
        QTRY_VERIFY(s->header()->hasFocus());
        QTest::keyClick(s->header(), Qt::Key_Space);
        QVERIFY(!s->isExpanded());
        QVERIFY(!label->isVisibleTo(&f));
        QVERIFY(s->header()->accessibleDescription() != open);
        QTest::keyClick(s->header(), Qt::Key_Return);
        QVERIFY(s->isExpanded());
        QVERIFY(label->isVisibleTo(&f));
        QCOMPARE(s->header()->accessibleDescription(), open);
        QCOMPARE(toggled.count(), 2);
        s->header()->click();
        QVERIFY(!s->isExpanded());
        s->setExpanded(true);
        QVERIFY(label->isVisibleTo(&f));
    }

    // A tab can be taken away; the pane goes to a page that is left, and comes back to none when all are gone.
    void aTabCanBeHidden()
    {
        PaneFrame f;
        fill(f);
        f.setCurrentPage(2);
        f.setPageVisible(2, false);
        QVERIFY(!f.isPageVisible(2));
        QCOMPARE(f.currentPage(), 0);
        f.setCurrentPage(2);   // a tab that is hidden can't be opened
        QCOMPARE(f.currentPage(), 0);
        f.setPageVisible(2, true);
        QVERIFY(f.isPageVisible(2));
        f.setCurrentPage(2);
        QCOMPARE(f.currentPage(), 2);
    }

    // With nothing to format the pane says so in place of its tabs, and has them back with the next selection.
    void aMessageTakesThePlaceOfTheTabs()
    {
        PaneFrame f;
        fill(f);
        f.resize(320, 480);
        f.show();
        QVERIFY(QTest::qWaitForWindowActive(&f));
        QVERIFY(f.tabButton(0)->isVisibleTo(&f));
        f.setMessage(QStringLiteral("Select an object."));
        QCOMPARE(f.message(), QStringLiteral("Select an object."));
        QVERIFY(!f.tabButton(0)->isVisibleTo(&f));
        QVERIFY(!f.page(0)->isVisibleTo(&f));
        f.setMessage(QString());
        QVERIFY(f.tabButton(0)->isVisibleTo(&f));
        QVERIFY(f.page(0)->isVisibleTo(&f));
    }

    // Esc in the pane gives the keyboard back to the page.
    void escapeLeavesThePane()
    {
        PaneFrame f;
        fill(f);
        f.resize(320, 480);
        f.show();
        QVERIFY(QTest::qWaitForWindowActive(&f));
        QSignalSpy escaped(&f, &PaneFrame::escaped);
        f.focusTabs();
        QTRY_VERIFY(f.tabButton(0)->hasFocus());
        QTest::keyClick(f.focusWidget(), Qt::Key_Escape);
        QCOMPARE(escaped.count(), 1);
    }

    // The pane follows the selection: it says what to do while nothing is selected, is named for what is, and shows the Picture tab for
    // pictures alone, the Fill & Line tab for anything but a table alone.
    void paneFollowsTheSelection()
    {
        Editor ed;
        ed.setDocument(Document::blank(QSizeF(612, 792)));
        auto shape = std::make_shared<ShapeItem>();
        shape->rect = QRectF(72, 72, 200, 100);
        auto pic = std::make_shared<PictureItem>();
        pic->rect = QRectF(300, 72, 100, 100);
        ed.addItem(shape, false);
        ed.addItem(pic, false);
        ed.addItem(ed.newTable(QRectF(72, 300, 200, 100), 2, 2), false);
        ed.clearSelection();
        FormatPane pane(&ed);
        pane.resize(320, 600);
        pane.show();
        QVERIFY(QTest::qWaitForWindowExposed(&pane));
        QVERIFY(!pane.hasSelection());
        QVERIFY(!pane.frame()->message().isEmpty());
        ed.select(shape->id);
        QVERIFY(pane.frame()->message().isEmpty());
        QCOMPARE(pane.title(), QStringLiteral("Format Shape"));
        QVERIFY(pane.isTabShown(FormatPane::Tab::FillLine));
        QVERIFY(pane.isTabShown(FormatPane::Tab::Effects));
        QVERIFY(pane.isTabShown(FormatPane::Tab::SizeProperties));
        QVERIFY(!pane.isTabShown(FormatPane::Tab::Picture));
        ed.select(pic->id);
        QCOMPARE(pane.title(), QStringLiteral("Format Picture"));
        QVERIFY(pane.isTabShown(FormatPane::Tab::Picture));
        pane.showTab(FormatPane::Tab::Picture);
        QCOMPARE(pane.currentTab(), FormatPane::Tab::Picture);
        // A picture with a shape: the Picture tab goes, and the pane leaves it for a page that shows.
        ed.select(QStringList{pic->id, shape->id});
        QCOMPARE(pane.title(), QStringLiteral("Format Shape"));
        QVERIFY(!pane.isTabShown(FormatPane::Tab::Picture));
        QVERIFY(pane.currentTab() != FormatPane::Tab::Picture);
        // A table has no fill or line of its own.
        ed.select(ed.surfaceItems().back()->id);
        QVERIFY(!pane.isTabShown(FormatPane::Tab::FillLine));
        QVERIFY(pane.isTabShown(FormatPane::Tab::SizeProperties));
        ed.clearSelection();
        QVERIFY(!pane.frame()->message().isEmpty());
        // A pane that is closed stays out of the way, and is up to date when it opens.
        pane.hide();
        ed.select(shape->id);
        QVERIFY(!pane.frame()->message().isEmpty());
        pane.show();
        QVERIFY(pane.frame()->message().isEmpty());
    }

    // Each change is one step that undoes: a control let go of at once is one, and so is one dragged through many values, taken when it is
    // let go of. The step is named for the change, undoes to what the objects were, and a drag that gets no further than where it began
    // is no step at all.
    void everyChangeIsOneStepThatUndoes()
    {
        Editor ed;
        ed.setDocument(Document::blank(QSizeF(612, 792)));
        auto a = std::make_shared<ShapeItem>();
        a->rect = QRectF(72, 72, 200, 100);
        auto b = std::make_shared<ShapeItem>();
        b->rect = QRectF(72, 300, 100, 50);
        ed.addItem(a, false);
        ed.addItem(b, false);
        ed.select(QStringList{a->id, b->id});
        FormatPane pane(&ed);
        FormatSession *s = pane.session();
        QUndoStack *undo = ed.undoStack();
        const int base = undo->count();
        const Fill firstFill = a->fill;
        s->apply(QStringLiteral("Fill"), [](Item &it) { it.fill = Fill::solid(ColorRef::rgb(Qt::red)); });
        QCOMPARE(undo->count(), base + 1);
        QCOMPARE(undo->undoText(), QStringLiteral("Fill"));
        QVERIFY(!s->isOpen());
        // Dragged: many changes, one step, which comes when it is let go of.
        for (int i = 1; i <= 20; ++i) s->apply(QStringLiteral("Rotation"), [i](Item &it) { it.rotation = i; }, FormatSession::Scope::Selected, false);
        QVERIFY(s->isOpen());
        QCOMPARE(undo->count(), base + 1);
        QCOMPARE(a->rotation, 20.0);
        QCOMPARE(b->rotation, 20.0);
        s->finish();
        QVERIFY(!s->isOpen());
        QCOMPARE(undo->count(), base + 2);
        QCOMPARE(undo->undoText(), QStringLiteral("Rotation"));
        ed.undo();
        QCOMPARE(ed.doc()->item(a->id)->rotation, 0.0);
        QCOMPARE(ed.doc()->item(b->id)->rotation, 0.0);
        QCOMPARE(ed.doc()->item(a->id)->fill, Fill::solid(ColorRef::rgb(Qt::red)));   // the step before stays
        ed.undo();
        QCOMPARE(ed.doc()->item(a->id)->fill, firstFill);
        // A drag back to where it began changes nothing, and is no step.
        const int steps = undo->index();
        s->apply(QStringLiteral("Rotation"), [](Item &it) { it.rotation = 30; }, FormatSession::Scope::Selected, false);
        s->apply(QStringLiteral("Rotation"), [](Item &it) { it.rotation = 0; }, FormatSession::Scope::Selected, false);
        s->finish();
        QCOMPARE(undo->index(), steps);
        // A step left open when the selection moves ends with it.
        s->apply(QStringLiteral("Rotation"), [](Item &it) { it.rotation = 45; }, FormatSession::Scope::Selected, false);
        QVERIFY(s->isOpen());
        ed.select(b->id);
        QVERIFY(!s->isOpen());
        QCOMPARE(undo->index(), steps + 1);
    }

    // A group's fill and line are those of the objects in it; its size and place are its own. Locked objects are left out when the
    // change asks, and everything else is changed, once.
    void groupsAreOpenedAndLockedObjectsCanBeLeftOut()
    {
        Editor ed;
        ed.setDocument(Document::blank(QSizeF(612, 792)));
        auto a = std::make_shared<ShapeItem>();
        a->rect = QRectF(72, 72, 100, 50);
        auto b = std::make_shared<ShapeItem>();
        b->rect = QRectF(200, 72, 100, 50);
        b->locked = true;
        auto g = std::make_shared<GroupItem>();
        g->children = {a, b};
        g->syncRect();
        ed.addItem(g);
        FormatPane pane(&ed);
        FormatSession *s = pane.session();
        QCOMPARE(s->items().size(), 1);
        QCOMPARE(s->leaves().size(), 2);
        s->apply(QStringLiteral("Fill"), [](Item &it) { it.fill = Fill::solid(ColorRef::rgb(Qt::blue)); }, FormatSession::Scope::Leaves);
        QCOMPARE(a->fill, Fill::solid(ColorRef::rgb(Qt::blue)));
        QCOMPARE(b->fill, Fill::solid(ColorRef::rgb(Qt::blue)));
        s->apply(QStringLiteral("Size"), [](Item &it) { it.rect.setWidth(10); }, FormatSession::Scope::Leaves, true, false);
        QCOMPARE(a->rect.width(), 10.0);
        QCOMPARE(b->rect.width(), 100.0);
    }
};

JP_TEST_CLASS(PubFormatPaneTests)
#include "tst_pub_formatpane.moc"
