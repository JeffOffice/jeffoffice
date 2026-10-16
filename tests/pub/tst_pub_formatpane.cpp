// The tests of the format pane's frame and property editors (libs/shell): the parts every app's Format pane is made of.
#include "pubhelpers.h"

namespace {

// What an editor said it changed, in order (a function can't ride in a QSignalSpy).
template <class Editor, class Change> struct Recorder {
    Recorder(Editor *e, void (Editor::*signal)(const Change &))
    {
        QObject::connect(e, signal, e, [this](const Change &c) { list << c; });
    }
    int count() const { return int(list.size()); }
    Change last() const { return list.last(); }
    QVector<Change> list;
};

} // namespace

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

    // A number reports each edit and says when it is let go of: a value set in the box, or typed, is one edit that is finished at once; a
    // slider dragged, or an arrow key held, is many edits and one finish. The pane's own setValue says nothing.
    void numberFieldReportsEditsAndFinishesWhenLetGo()
    {
        NumberField f;
        f.setRange(0, 100);
        f.setDecimals(0);
        f.setSlider(true);
        f.setSuffix(QStringLiteral("%"));
        QSignalSpy edited(&f, &NumberField::edited), done(&f, &NumberField::finished);
        f.box()->setValue(30);
        QCOMPARE(edited.count(), 1);
        QCOMPARE(done.count(), 1);
        QCOMPARE(edited.last().first().toDouble(), 30.0);
        QCOMPARE(f.slider()->value(), 30);
        f.setValue(40);   // from the objects: no signal
        QCOMPARE(edited.count(), 1);
        QCOMPARE(f.value(), 40.0);
        QCOMPARE(f.slider()->value(), 40);
        // The slider dragged: an edit at each value, one finish at the end.
        f.slider()->setSliderDown(true);
        QVERIFY(f.isBusy());
        f.setValue(5);   // the objects change under the drag: the field keeps what the person is doing
        QCOMPARE(f.value(), 40.0);
        for (int v = 41; v <= 60; ++v) f.slider()->setValue(v);
        QCOMPARE(edited.count(), 21);
        QCOMPARE(done.count(), 1);
        f.slider()->setSliderDown(false);
        QCOMPARE(done.count(), 2);
        QVERIFY(!f.isBusy());
        QCOMPARE(f.box()->value(), 60.0);
        QCOMPARE(edited.last().first().toDouble(), 60.0);
        // An arrow key held down: edits while it is, a finish when it is let go.
        QTest::keyPress(f.box(), Qt::Key_Up);
        QTest::keyPress(f.box(), Qt::Key_Up);
        QCOMPARE(edited.count(), 23);
        QCOMPARE(done.count(), 2);
        QTest::keyRelease(f.box(), Qt::Key_Up);
        QCOMPARE(done.count(), 3);
        QCOMPARE(f.box()->value(), 62.0);
    }

    // Values the objects disagree on show as a blank box. Looking at the box changes nothing; typing in it does, and ends the blank.
    void numberFieldShowsMixedValuesAsBlank()
    {
        NumberField f;
        f.setRange(0, 100);
        f.setDecimals(0);
        f.resize(160, 30);
        f.show();
        QVERIFY(QTest::qWaitForWindowExposed(&f));
        f.setValue(25);
        f.setMixed(true);
        QVERIFY(f.isMixed());
        QVERIFY(f.box()->cleanText().isEmpty());
        QSignalSpy edited(&f, &NumberField::edited), done(&f, &NumberField::finished);
        // Gone into and out of: still blank, nothing said.
        f.box()->setFocus(Qt::TabFocusReason);
        QTRY_VERIFY(f.box()->hasFocus());
        f.box()->clearFocus();
        QTRY_VERIFY(!f.box()->hasFocus());
        QCOMPARE(edited.count(), 0);
        QVERIFY(f.isMixed());
        QVERIFY(f.box()->cleanText().isEmpty());
        // Typed in.
        f.box()->setFocus(Qt::TabFocusReason);
        QTRY_VERIFY(f.box()->hasFocus());
        QTest::keyClicks(f.box(), QStringLiteral("7"));
        QTest::keyClick(f.box(), Qt::Key_Return);
        QCOMPARE(edited.count(), 1);
        QCOMPARE(edited.last().first().toDouble(), 7.0);
        QCOMPARE(done.count(), 1);
        QVERIFY(!f.isMixed());
        // A value from the objects ends the blank only when the pane says so.
        f.box()->clearFocus();
        f.setMixed(true);
        f.setValue(9);
        f.setMixed(false);
        QCOMPARE(f.box()->cleanText(), QStringLiteral("9"));
    }

    // A measure is in points and shows in the unit of Options; a value typed with a unit is read.
    void numberFieldMeasuresInTheUnitOfOptions()
    {
        const Unit was = Settings::get().unit();
        Settings::get().setUnit(Unit::Inch);
        auto restore = qScopeGuard([&] { Settings::get().setUnit(was); });
        NumberField f;
        f.setMeasure(true);
        f.setRange(0, 1584);
        f.setValue(72);
        QCOMPARE(f.box()->text(), Settings::get().format(72, 3));
        QSignalSpy edited(&f, &NumberField::edited);
        f.box()->findChild<QLineEdit *>()->setText(QStringLiteral("2 in"));
        f.box()->interpretText();
        QCOMPARE(edited.count(), 1);
        QVERIFY(std::abs(edited.last().first().toDouble() - 144) < 0.01);
        Settings::get().setUnit(Unit::Point);
        f.setValue(36);
        QVERIFY(f.box()->text().startsWith(QStringLiteral("36")));
    }

    // The stops of a gradient: a click chooses one, a drag moves it between its neighbors and is one change, the keys do the same, a double
    // click adds one in the color the bar has there, and Delete takes one away (never below two).
    void gradientBarChoosesMovesAddsAndRemovesStops()
    {
        GradientBar bar;
        bar.resize(240, 44);
        bar.show();
        QVERIFY(QTest::qWaitForWindowExposed(&bar));
        bar.setStops({{0, ColorRef::rgb(Qt::red), 0}, {1, ColorRef::rgb(Qt::blue), 0}});
        QCOMPARE(bar.current(), 0);
        QSignalSpy edited(&bar, &GradientBar::stopsEdited), done(&bar, &GradientBar::finished), chosen(&bar, &GradientBar::currentChanged);
        QTest::mouseClick(&bar, Qt::LeftButton, {}, bar.markerRect(1).center().toPoint());
        QCOMPARE(bar.current(), 1);
        QCOMPARE(chosen.count(), 1);
        QCOMPARE(edited.count(), 0);
        // Dragged to the middle: edits on the way, one finish.
        const QPointF from = bar.markerRect(1).center();
        const double middle = bar.barRect().left() + bar.barRect().width() / 2;
        QTest::mousePress(&bar, Qt::LeftButton, {}, from.toPoint());
        QTest::mouseMove(&bar, QPoint(int(from.x() - 20), int(from.y())));
        QTest::mouseMove(&bar, QPoint(int(middle), int(from.y())));
        QVERIFY(edited.count() >= 1);
        QCOMPARE(done.count(), 0);
        QTest::mouseRelease(&bar, Qt::LeftButton, {}, QPoint(int(middle), int(from.y())));
        QCOMPARE(done.count(), 1);
        QVERIFY(std::abs(bar.stops()[1].pos - 0.5) < 0.03);
        // It can't pass the stop before it.
        QTest::mousePress(&bar, Qt::LeftButton, {}, bar.markerRect(1).center().toPoint());
        QTest::mouseMove(&bar, QPoint(int(bar.barRect().left()) - 10, int(from.y())));
        QTest::mouseRelease(&bar, Qt::LeftButton, {}, QPoint(int(bar.barRect().left()) - 10, int(from.y())));
        QVERIFY(bar.stops()[1].pos >= bar.stops()[0].pos);
        QCOMPARE(bar.stops().size(), 2);
        bar.setStops({{0, ColorRef::rgb(Qt::red), 0}, {1, ColorRef::rgb(Qt::blue), 0}});
        // The keys: Right moves a stop by a hundredth and finishes when let go of; Down chooses the next stop.
        bar.setCurrent(0);
        bar.setFocus(Qt::TabFocusReason);
        QTRY_VERIFY(bar.hasFocus());
        const int finishes = done.count();
        QTest::keyPress(&bar, Qt::Key_Right);
        QTest::keyRelease(&bar, Qt::Key_Right);
        QVERIFY(std::abs(bar.stops()[0].pos - 0.01) < 1e-9);
        QCOMPARE(done.count(), finishes + 1);
        QTest::keyClick(&bar, Qt::Key_Down);
        QCOMPARE(bar.current(), 1);
        // A double click on the bar adds a stop there, between the colors; the new one is chosen.
        const QPoint at(int(bar.barRect().left() + bar.barRect().width() * 0.5), int(bar.barRect().center().y()));
        QTest::mouseDClick(&bar, Qt::LeftButton, {}, at);
        QCOMPARE(bar.stops().size(), 3);
        QCOMPARE(bar.current(), 1);
        QVERIFY(std::abs(bar.stops()[1].pos - 0.5) < 0.03);
        const QColor mid = bar.stops()[1].color.rgbValue();
        QVERIFY(mid.red() > 60 && mid.red() < 200 && mid.blue() > 60 && mid.blue() < 200);
        // Delete takes the chosen one away; two stay.
        QTest::keyClick(&bar, Qt::Key_Delete);
        QCOMPARE(bar.stops().size(), 2);
        QTest::keyClick(&bar, Qt::Key_Delete);
        QCOMPARE(bar.stops().size(), 2);
    }

    // A fill editor shows the fills of the objects: the type they share (none, when they differ), and says what is changed as a function that
    // changes a fill, so each object keeps the rest of its own.
    void fillEditorShowsTypesAndSaysChangesToApply()
    {
        FillEditor e;
        e.resize(300, 600);
        e.show();
        QVERIFY(QTest::qWaitForWindowExposed(&e));
        auto radio = [&](const char *name) { return e.findChild<QRadioButton *>(QLatin1String(name)); };
        for (const char *n : {"fillNone", "fillSolid", "fillGradient", "fillPicture", "fillPattern"}) QVERIFY2(radio(n), n);
        e.setFills({Fill::solid(ColorRef::rgb(Qt::red), 0.2)});
        QVERIFY(radio("fillSolid")->isChecked());
        e.setFills({Fill::solid(ColorRef::rgb(Qt::red)), Fill::gradient(ColorRef::rgb(Qt::red), ColorRef::rgb(Qt::blue))});
        for (const char *n : {"fillNone", "fillSolid", "fillGradient", "fillPicture", "fillPattern"}) QVERIFY2(!radio(n)->isChecked(), n);
        e.setFills({Fill()});
        QVERIFY(radio("fillNone")->isChecked());
        e.setFills({});
        QVERIFY(!e.isEnabled());
        QVERIFY(e.isEnabled() == false);

        // Solid to gradient: two stops from the color, and the transparency the fill had.
        Recorder<FillEditor, FillChange> changed(&e, &FillEditor::changed);
        QSignalSpy done(&e, &FillEditor::finished), asked(&e, &FillEditor::pictureRequested);
        e.setFills({Fill::solid(ColorRef::rgb(Qt::red), 0.2)});
        QVERIFY(e.isEnabled());
        radio("fillGradient")->click();
        QCOMPARE(changed.count(), 1);
        QCOMPARE(done.count(), 1);
        Fill f = Fill::solid(ColorRef::rgb(Qt::red), 0.2);
        changed.last()(f);
        QCOMPARE(f.type, Fill::Gradient);
        QCOMPARE(f.stops.size(), 2);
        QCOMPARE(f.stops.first().color, ColorRef::rgb(Qt::red));
        QCOMPARE(f.stops.first().pos, 0.0);
        QCOMPARE(f.stops.last().pos, 1.0);
        // No fill.
        radio("fillNone")->click();
        f = Fill::solid(ColorRef::rgb(Qt::red));
        changed.last()(f);
        QCOMPARE(f.type, Fill::NoFill);
        // A picture fill with no picture asks for one, and changes nothing yet.
        const int before = changed.count();
        radio("fillPicture")->click();
        QCOMPARE(asked.count(), 1);
        QCOMPARE(changed.count(), before);
        // A pattern: the button's pattern, in the colors the fill had.
        e.setFills({Fill::solid(ColorRef::rgb(Qt::red))});
        auto *p5 = e.findChild<QToolButton *>(QStringLiteral("pattern5"));
        QVERIFY(p5);
        radio("fillPattern")->click();
        f = Fill::solid(ColorRef::rgb(Qt::red));
        changed.last()(f);
        QCOMPARE(f.type, Fill::Pattern);
        QCOMPARE(f.color, ColorRef::rgb(Qt::red));
    }

    // Two fills that differ in color but are both solid: the transparency is shown as blank when they differ, and a change of it leaves each
    // color alone.
    void fillEditorChangesOnePropertyAndKeepsTheRest()
    {
        FillEditor e;
        e.resize(300, 600);
        e.show();
        QVERIFY(QTest::qWaitForWindowExposed(&e));
        Fill a = Fill::solid(ColorRef::rgb(Qt::red), 0.2), b = Fill::solid(ColorRef::rgb(Qt::blue), 0.5);
        e.setFills({a, b});
        auto *t = e.findChild<NumberField *>(QStringLiteral("fillTransparency"));
        auto *c = e.findChild<ColorButton *>(QStringLiteral("fillColor"));
        QVERIFY(t && c);
        QVERIFY(t->isMixed());
        QVERIFY(t->box()->cleanText().isEmpty());
        Recorder<FillEditor, FillChange> changed(&e, &FillEditor::changed);
        QSignalSpy done(&e, &FillEditor::finished);
        t->box()->setValue(40);
        QCOMPARE(changed.count(), 1);
        QCOMPARE(done.count(), 1);
        const FillChange change = changed.last();
        change(a);
        change(b);
        QCOMPARE(a.transparency, 0.4);
        QCOMPARE(b.transparency, 0.4);
        QCOMPARE(a.color, ColorRef::rgb(Qt::red));
        QCOMPARE(b.color, ColorRef::rgb(Qt::blue));
        // Same transparency, same color: both shown.
        e.setFills({Fill::solid(ColorRef::rgb(Qt::red), 0.25), Fill::solid(ColorRef::rgb(Qt::red), 0.25)});
        QVERIFY(!t->isMixed());
        QCOMPARE(t->value(), 25.0);
        QCOMPARE(c->current(), ColorRef::rgb(Qt::red));
    }

    // A gradient's pieces: the kind and angle, a stop's color, place, and transparency, adding and taking away stops.
    void gradientEditorChangesTheGradientAndItsStops()
    {
        GradientEditor g;
        g.resize(300, 400);
        g.show();
        QVERIFY(QTest::qWaitForWindowExposed(&g));
        Fill base;
        base.type = Fill::Gradient;
        base.stops = {{0, ColorRef::rgb(Qt::red), 0}, {0.5, ColorRef::rgb(Qt::green), 0.1}, {1, ColorRef::rgb(Qt::blue), 0}};
        base.angle = 90;
        g.setGradients({base});
        Recorder<GradientEditor, FillChange> changed(&g, &GradientEditor::changed);
        QSignalSpy done(&g, &GradientEditor::finished);
        auto apply = [&](Fill f) {
            changed.last()(f);
            return f;
        };
        auto *kind = g.findChild<QComboBox *>(QStringLiteral("gradientType"));
        auto *angle = g.findChild<NumberField *>(QStringLiteral("gradientAngle"));
        auto *pos = g.findChild<NumberField *>(QStringLiteral("stopPosition"));
        auto *tr = g.findChild<NumberField *>(QStringLiteral("stopTransparency"));
        auto *add = g.findChild<QToolButton *>(QStringLiteral("gradientAdd"));
        auto *remove = g.findChild<QToolButton *>(QStringLiteral("gradientRemove"));
        QVERIFY(kind && angle && pos && tr && add && remove);
        QVERIFY(kind->count() == 4);
        QCOMPARE(kind->currentIndex(), int(Fill::Linear));
        QCOMPARE(angle->value(), 90.0);
        QVERIFY(angle->isVisibleTo(&g));
        kind->setCurrentIndex(int(Fill::Radial));
        QCOMPARE(apply(base).gradType, Fill::Radial);
        QVERIFY(!angle->isVisibleTo(&g));   // only a linear gradient has an angle
        kind->setCurrentIndex(int(Fill::Linear));
        angle->box()->setValue(45);
        QCOMPARE(apply(base).angle, 45.0);
        // The stop chosen: the middle one.
        g.bar()->setCurrent(1);
        QCOMPARE(pos->value(), 50.0);
        QCOMPARE(tr->value(), 10.0);
        pos->box()->setValue(70);
        Fill moved = apply(base);
        QCOMPARE(moved.stops.size(), 3);
        QCOMPARE(moved.stops[1].pos, 0.7);
        QCOMPARE(moved.stops[1].color, ColorRef::rgb(Qt::green));
        tr->box()->setValue(60);
        QCOMPARE(apply(base).stops[1].transparency, 0.6);
        // Add: a stop between the chosen one and the next; Remove: the chosen one, never below two.
        add->click();
        Fill added = apply(base);
        QCOMPARE(added.stops.size(), 4);
        remove->click();
        QCOMPARE(apply(base).stops.size(), 3);
        remove->click();
        QCOMPARE(apply(base).stops.size(), 2);
        QVERIFY(!remove->isEnabled());   // two stay
        QVERIFY(done.count() >= 5);
    }

    // A line editor: none, solid, or gradient; the color, width, compound type, dash type, cap, join, and arrows of the lines, each change
    // leaving the rest of every line alone.
    void lineEditorShowsLinesAndSaysChangesToApply()
    {
        LineEditor e;
        e.resize(300, 800);
        e.show();
        QVERIFY(QTest::qWaitForWindowExposed(&e));
        auto radio = [&](const char *name) { return e.findChild<QRadioButton *>(QLatin1String(name)); };
        auto field = [&](const char *name) { return e.findChild<NumberField *>(QLatin1String(name)); };
        auto combo = [&](const char *name) { return e.findChild<QComboBox *>(QLatin1String(name)); };
        for (const char *n : {"lineNone", "lineSolid", "lineGradient"}) QVERIFY2(radio(n), n);
        Stroke thin = Stroke::line(ColorRef::rgb(Qt::red), 0.75), thick = Stroke::line(ColorRef::rgb(Qt::red), 4);
        thick.dash = Stroke::DashLine;
        e.setStrokes({thin, thick});
        QVERIFY(radio("lineSolid")->isChecked());
        QVERIFY(field("lineWidth")->isMixed());
        QVERIFY(field("lineWidth")->box()->cleanText().isEmpty());
        QCOMPARE(combo("lineDash")->currentIndex(), -1);   // they differ: blank
        QCOMPARE(combo("lineCap")->currentIndex(), 0);   // they agree: flat
        Recorder<LineEditor, StrokeChange> changed(&e, &LineEditor::changed);
        QSignalSpy done(&e, &LineEditor::finished);
        auto apply = [&](Stroke s) {
            changed.last()(s);
            return s;
        };
        field("lineWidth")->box()->setValue(2.5);
        QCOMPARE(changed.count(), 1);
        QCOMPARE(done.count(), 1);
        Stroke s = apply(thick);
        QCOMPARE(s.width, 2.5);
        QCOMPARE(s.dash, Stroke::DashLine);   // the rest stays
        s = apply(thin);
        QCOMPARE(s.dash, Stroke::SolidLine);
        combo("lineDash")->setCurrentIndex(int(Stroke::LongDash));
        QCOMPARE(apply(thin).dash, Stroke::LongDash);
        combo("lineCompound")->setCurrentIndex(int(Stroke::ThickThin));
        QCOMPARE(apply(thin).compound, Stroke::ThickThin);
        combo("lineCap")->setCurrentIndex(1);
        QCOMPARE(apply(thin).cap, Qt::RoundCap);
        combo("lineJoin")->setCurrentIndex(0);
        QCOMPARE(apply(thin).join, Qt::RoundJoin);
        combo("lineBeginType")->setCurrentIndex(int(Arrow::Triangle));
        QCOMPARE(apply(thin).startArrow, Arrow::Triangle);
        combo("lineEndSize")->setCurrentIndex(2);
        QCOMPARE(apply(thin).endSize, 2);
        // No line: the color goes and the rest stays; Solid brings back a color (the old one if it had one, else the theme's).
        radio("lineNone")->click();
        s = apply(thick);
        QVERIFY(s.isNone());
        QCOMPARE(s.dash, Stroke::DashLine);
        e.setStrokes({s});
        QVERIFY(radio("lineNone")->isChecked());
        radio("lineSolid")->click();
        s = apply(s);
        QVERIFY(!s.isNone());
        QVERIFY(s.width > 0);
        // A gradient line has the gradient's colors and keeps the first stop as its color.
        radio("lineGradient")->click();
        s = apply(Stroke::line(ColorRef::rgb(Qt::red), 3));
        QVERIFY(s.hasGradient());
        QCOMPARE(s.gradient.stops.size(), 2);
        QCOMPARE(s.color, s.gradient.stops.first().color);
        QCOMPARE(s.width, 3.0);
        e.setStrokes({s});
        QVERIFY(radio("lineGradient")->isChecked());
        radio("lineSolid")->click();
        s = apply(s);
        QVERIFY(!s.hasGradient());
        QVERIFY(!s.isNone());
        // A shape's outline has no ends.
        e.setArrowsShown(false);
        QVERIFY(!combo("lineBeginType")->isVisibleTo(&e));
        e.setArrowsShown(true);
        QVERIFY(combo("lineBeginType")->isVisibleTo(&e));
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
