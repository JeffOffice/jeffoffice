// JeffDeck's tests: its presentation model on the shared page model, and
// its window.

#include "../../apps/pub/app/pubinfo.h"   // only its inline pubAppInfo(): the other app's colors, to compare
#include "app/deckcanvas.h"
#include "app/deckdialogs.h"
#include "app/deckinfo.h"
#include "app/deckwidgets.h"
#include "app/deckwindow.h"
#include "app/editor.h"
#include "app/filescreen.h"
#include "app/icons.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/presenterview.h"
#include "app/recovery.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "app/slideshow.h"
#include "app/slidesorter.h"
#include "app/slidespane.h"
#include "core/animation.h"
#include "core/deck.h"
#include "core/fonts.h"
#include "core/transition.h"
#include "io/cfb.h"
#include "io/deckoutput.h"
#include "io/pptimport.h"
#include "io/pptxexport.h"
#include "io/pptximport.h"
#include "io/presentationfile.h"
#include "io/zip.h"
#include "office/chart.h"
#include "office/drawingml.h"
#include "office/drawingml_write.h"
#include "office/package.h"
#include "office/xml.h"
#include "render/renderer.h"
#include "text/textprops.h"

#include <QAbstractButton>
#include <QApplication>
#include <QBuffer>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QDeadlineTimer>
#include <QDesktopServices>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QFrame>
#include <QFileInfo>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPointer>
#include <QPrinter>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QSignalSpy>
#include <QSlider>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QXmlStreamWriter>
#include <cmath>
#include <functional>

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
    // Slides for the Slide Sorter and Outline tests: a Title Slide, then
    // Title and Content slides, `titles` for their titles and `bodies` for
    // the lines of their bodies (the Title Slide's subtitle), typed in the
    // way that keeps the placeholders' formatting.
    static void fillSlides(DeckWindow &w, const QStringList &titles, const QList<QStringList> &bodies = {})
    {
        Editor *ed = w.editor();
        for (int i = 1; i < titles.size(); ++i) w.newSlide();
        Document *d = ed->doc();
        auto type = [](const QString &text, QTextDocument *story) {
            QTextCursor c(story);
            const QStringList lines = text.split(QLatin1Char('\n'));
            for (int k = 0; k < lines.size(); ++k) {
                if (k) c.insertBlock();
                c.insertText(lines[k]);
            }
        };
        for (int i = 0; i < titles.size(); ++i)
            for (const auto &it : d->pages[i]->items) {
                const QString kind = deck::placeholderType(*it);
                QTextDocument *story = d->storyDoc(static_cast<TextItem *>(it.get())->storyId);
                if (kind == QLatin1String("title") || kind == QLatin1String("ctrTitle")) type(titles[i], story);
                else if ((kind == QLatin1String("obj") || kind == QLatin1String("subTitle")) && !bodies.value(i).isEmpty()) type(bodies[i].join(QLatin1Char('\n')), story);
            }
        ed->setCurrentPage(0);
        ed->notifyLive();
        ed->undoStack()->clear();
    }
    // The title of slide `i`, as its title placeholder holds it.
    static QString titleOfSlide(const Document &d, int i)
    {
        for (const auto &it : d.pages[i]->items) {
            const QString kind = deck::placeholderType(*it);
            if (kind == QLatin1String("title") || kind == QLatin1String("ctrTitle")) return d.storyDoc(static_cast<const TextItem *>(it.get())->storyId)->toPlainText();
        }
        return QString();
    }
    static QStringList titlesOf(const Document &d)
    {
        QStringList out;
        for (int i = 0; i < d.pages.size(); ++i) out << titleOfSlide(d, i);
        return out;
    }
    // The story of slide `i`'s first body placeholder (a subtitle counts).
    static QTextDocument *bodyOfSlide(const Document &d, int i)
    {
        for (const auto &it : d.pages[i]->items) {
            const QString kind = deck::placeholderType(*it);
            if (kind == QLatin1String("obj") || kind == QLatin1String("body") || kind == QLatin1String("subTitle")) return d.storyDoc(static_cast<const TextItem *>(it.get())->storyId);
        }
        return nullptr;
    }
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

    // ---- Old binary presentations ([MS-PPT]), built record by record ----
private:
    static QByteArray le(quint64 v, int n)
    {
        QByteArray b;
        for (int i = 0; i < n; ++i) b.append(char((v >> (8 * i)) & 0xFF));
        return b;
    }
    // A record: its header (version, instance, type, length), then its data.
    static QByteArray pptRec(quint16 type, const QByteArray &data, quint16 inst = 0, quint8 ver = 0)
    {
        return le(quint16(ver | (inst << 4)), 2) + le(type, 2) + le(quint32(data.size()), 4) + data;
    }
    static QByteArray pptBox(quint16 type, const QByteArray &kids, quint16 inst = 0) { return pptRec(type, kids, inst, 0xF); }
    static QByteArray utf16(const QString &s)
    {
        QByteArray b;
        for (const QChar c : s) b += le(c.unicode(), 2);
        return b;
    }
    // A color as the format stores it: red, green, blue, and a scheme index (0xFE: the RGB values are the color).
    static QByteArray raw(std::initializer_list<int> values)
    {
        QByteArray b;
        for (const int v : values) b.append(char(v));
        return b;
    }
    static QByteArray pptColor(int r, int g, int b, int index) { return raw({r, g, b, index}); }
    static QByteArray fontEntity(const QString &name)
    {
        QByteArray face = utf16(name);
        face.append(QByteArray(64 - face.size(), '\0'));
        return pptRec(4023, face + QByteArray(4, '\0'));
    }
    // The stream's name is fixed by [MS-PPT] section 2.1.2: the program's name and "Document".
    static QString pptDocumentStream() { return QStringLiteral("Power") + QStringLiteral("Point Document"); }

    // One text of a slide: a TextHeaderAtom with its type, the characters
    // (8-bit unless `wide`), and the style runs (a StyleTextPropAtom).
    static QByteArray pptText(int type, const QString &s, const QByteArray &styles = QByteArray(), bool wide = false)
    {
        QByteArray out = pptRec(3999, le(type, 4));
        out += wide ? pptRec(4000, utf16(s)) : pptRec(4008, s.toLatin1());
        if (!styles.isEmpty()) out += pptRec(4001, styles);
        return out;
    }
    static QByteArray pfRun(int count, int level, quint32 masks, const QByteArray &data = QByteArray()) { return le(count, 4) + le(level, 2) + le(masks, 4) + data; }
    static QByteArray cfRun(int count, quint32 masks, const QByteArray &data = QByteArray()) { return le(count, 4) + le(masks, 4) + data; }

    static QByteArray slidePersist(quint32 persistId, int texts, quint32 slideId) { return pptRec(1011, le(persistId, 4) + le(4, 4) + le(texts, 4) + le(slideId, 4) + le(0, 4)); }
    // A slide's or master's SlideAtom: the layout geometry, the placeholders it names, the master and notes it refers to, and its flags.
    static QByteArray slideAtom(int geom, const QByteArray &placeholders, quint32 master, quint32 notes, int flags)
    {
        QByteArray ph = placeholders;
        ph.append(QByteArray(8 - ph.size(), '\0'));
        return pptRec(1007, le(geom, 4) + ph + le(master, 4) + le(notes, 4) + le(flags, 2) + le(0, 2), 0, 2);
    }
    static QByteArray colorScheme(int instance)
    {
        // background, text, shadow, title text, fills, accent, accent and hyperlink, accent and followed hyperlink
        return pptRec(2032, pptColor(0xFA, 0xFA, 0xFA, 0) + pptColor(0x11, 0x22, 0x33, 0) + pptColor(0xDD, 0xDD, 0xCC, 0) + pptColor(0x44, 0x55, 0x66, 0)
                                 + pptColor(0x77, 0x88, 0x99, 0) + pptColor(0xAA, 0xBB, 0xCC, 0) + pptColor(1, 2, 3, 0) + pptColor(4, 5, 6, 0), instance);
    }
    // A drawing: a group shape and the given shapes.
    static QByteArray pptDrawing(const QByteArray &shapes = QByteArray())
    {
        const QByteArray group = pptBox(0xF004, pptRec(0xF009, QByteArray(16, '\0'), 0, 1) + pptRec(0xF00A, le(0x400, 4) + le(5, 4), 0, 2));
        return pptBox(1036, pptBox(0xF002, pptRec(0xF008, le(2, 4) + le(0x402, 4)) + pptBox(0xF003, group + shapes)));
    }
    // A placeholder shape: a rectangle at `top, left, right, bottom` (master units), the placeholder it is, and what it holds.
    static QByteArray pptShape(quint32 id, int placeholder, const QByteArray &textbox, int top = 100, int left = 200, int right = 5000, int bottom = 900)
    {
        const QByteArray anchor = le(top, 2) + le(left, 2) + le(right, 2) + le(bottom, 2);
        const QByteArray data = pptBox(0xF011, pptRec(3011, le(0xFFFFFFFF, 4) + raw({placeholder, 0, 0, 0})));
        return pptBox(0xF004, pptRec(0xF00A, le(id, 4) + le(0xA00, 4), 1, 2) + pptRec(0xF010, anchor) + data + (textbox.isEmpty() ? QByteArray() : pptBox(0xF00D, textbox)));
    }
    static QByteArray outlineRef(int index) { return pptRec(3998, le(index, 4)); }

    // A UserEditAtom, and a PersistDirectoryAtom mapping the given persist ids (each next to the last) to offsets.
    static QByteArray userEdit(quint32 lastEdit, quint32 directory, quint32 document, int extra = 0)
    {
        QByteArray d = le(0x100, 4) + le(0, 2) + raw({0, 3}) + le(lastEdit, 4) + le(directory, 4) + le(document, 4) + le(0x20, 4) + le(1, 2) + le(0, 2);
        if (extra) d += le(extra, 4);
        return pptRec(4085, d);
    }
    static QByteArray persistDirectory(const QVector<QPair<quint32, quint32>> &entries)
    {
        QByteArray d;
        for (const auto &e : entries) d += le(e.first | (1u << 20), 4) + le(e.second, 4);
        return pptRec(6002, d);
    }

    // A presentation of two slides on one master, in the order First (persist
    // id 5, with notes) and Second (4, hidden by the newer of two edits); with
    // `titleMaster`, a slide on a title master; with `links`, a slide whose text
    // has a field, a link, a language, and a ruler, and that fades in. `cycle`
    // makes the older edit point back at the newer. `notesByAtom` leaves the
    // first slide without the id of its notes, which then name the slide.
    // `bodyGroup` stands for the first slide's body text.
    static QByteArray buildPpt(bool titleMaster = false, bool cycle = false, bool links = false, bool notesByAtom = false, const QByteArray &bodyGroup = QByteArray())
    {
        constexpr quint32 kMaster = 0x80000001, kTitleMaster = 0x80000002;
        const QByteArray fonts = pptBox(2005, fontEntity(QStringLiteral("Alpha Sans")) + fontEntity(QStringLiteral("Beta Serif")));
        // The default body text: bullets, 28 and 24 points, the first font, the text color.
        const QByteArray bodyStyle = le(2, 2)
            + le(0x593, 4) + le(3, 2) + le(0x2022, 2) + le(0, 2) + le(360, 2) + le(0, 2)
            + le(0x70000, 4) + le(0, 2) + le(28, 2) + pptColor(0, 0, 0, 1)
            + le(0x581, 4) + le(1, 2) + le(0x2013, 2) + le(720, 2) + le(360, 2)
            + le(0x20000, 4) + le(24, 2);
        const QByteArray environment = pptBox(1010, fonts + pptRec(4003, bodyStyle, 1));
        const QByteArray document = pptBox(1000, pptRec(1001, le(5760, 4) + le(4320, 4) + le(4320, 4) + le(5760, 4) + le(5, 4) + le(10, 4) + le(0, 4) + le(0, 4) + le(1, 2) + le(0, 2) + le(0, 4))
            + environment
            + (links ? pptBox(1033, pptRec(1034, le(1, 4)) + pptBox(4055, pptRec(4051, le(7, 4)) + pptRec(4026, utf16(QStringLiteral("the site")), 0) + pptRec(4026, utf16(QStringLiteral("https://example.org/site")), 1)))
                     : QByteArray())
            + pptBox(4080, slidePersist(3, 0, kMaster) + (titleMaster ? slidePersist(7, 0, kTitleMaster) : QByteArray()), 1)
            + pptBox(4080,
                     slidePersist(5, 2, 0x100) + pptText(0, QStringLiteral("Quarterly Results"), pfRun(18, 0, 0) + cfRun(18, 0), true)
                         + (!bodyGroup.isEmpty() ? bodyGroup : pptText(1, QStringLiteral("First point\rSecond point\rThird point"), pfRun(12, 0, 0) + pfRun(13, 1, 0) + pfRun(12, 1, 0) + cfRun(12, 0) + cfRun(6, 1, le(1, 2)) + cfRun(19, 0)))
                         + slidePersist(4, 2, 0x101) + pptText(0, QStringLiteral("Plan"), QByteArray(), true)
                         // The first paragraph turns its bullet off: the bullet flags name only hasBullet, and it is clear.
                         + pptText(1, QStringLiteral("Ship\vit"), pfRun(8, 0, 1, le(0, 2)) + cfRun(8, 0))
                         + (titleMaster ? slidePersist(8, 2, 0x102) + pptText(6, QStringLiteral("Welcome"), QByteArray(), true) + pptText(5, QStringLiteral("To the talk")) : QByteArray())
                         + (links ? slidePersist(9, 2, 0x103) + pptText(0, QStringLiteral("Links"), QByteArray(), true)
                                + pptText(1, QStringLiteral("Go to the site. Slide *"), pfRun(24, 0, 0) + cfRun(24, 0))
                                // The ruler sets the first level's margin and indent; the language is French for the first 15
                                // characters; the slide number is the last; the link covers "the site".
                                + pptRec(4006, le(0x108, 4) + le(720, 2) + le(288, 2)) + pptRec(4010, le(15, 4) + le(2, 4) + le(0x040C, 2) + le(9, 4) + le(0, 4))
                                + pptRec(4056, le(22, 4)) + pptBox(4082, pptRec(4083, le(0, 4) + le(7, 4) + raw({4, 0, 0, 0, 8, 0, 0, 0})))
                                + pptRec(4063, le(6, 4) + le(14, 4))
                                  : QByteArray()),
                     0)
            + pptBox(4080, slidePersist(6, 0, 0x200), 2)
            + pptRec(1002, QByteArray()));

        const QByteArray mainMaster = pptBox(1016, slideAtom(1, QByteArray("\x01\x02\x07\x08\x09", 5), 0, 0, 3) + colorScheme(1)
            // The title text: centered, the second font, 44 points, the title text color.
            + pptRec(4003, le(1, 2) + le(0x800, 4) + le(1, 2) + le(0x70000, 4) + le(1, 2) + le(44, 2) + pptColor(0, 0, 0, 3), 0)
            + pptDrawing(pptShape(0x402, 1, QByteArray()) + pptShape(0x403, 2, QByteArray())));
        const QByteArray titleMasterRec = pptBox(1016, slideAtom(3, QByteArray("\x03\x04\x07\x08\x09", 5), 0, 0, 1) + colorScheme(1)
            // The centered title and the subtitle, with only their sizes (the levels of these two start with their number).
            + pptRec(4003, le(1, 2) + le(0, 2) + le(0x800, 4) + le(1, 2) + le(0x20000, 4) + le(54, 2), 6)
            + pptRec(4003, le(1, 2) + le(0, 2) + le(0, 4) + le(0x20000, 4) + le(30, 2), 5)
            + pptDrawing());

        auto slide = [&](int geom, const QByteArray &placeholders, quint32 master, quint32 notes, bool hidden, const QByteArray &shapes, const QByteArray &transition = QByteArray()) {
            QByteArray info = le(0, 4) + le(0, 4) + raw({2, 0}) + le(hidden ? 0x4 : 0x400, 2) + raw({2, 0, 0, 0});
            return pptBox(1006, slideAtom(geom, placeholders, master, notes, 7) + pptRec(1017, transition.isEmpty() ? info : transition) + pptDrawing(shapes));
        };
        const QByteArray titleOnBody = pptShape(0x402, 0xD, outlineRef(0)) + pptShape(0x403, 0xE, outlineRef(1), 1000, 200, 5000, 3800);
        const QByteArray first = slide(1, QByteArray("\x0D\x0E", 2), kMaster, notesByAtom ? 0 : 0x200, false, titleOnBody);
        const QByteArray secondOld = slide(1, QByteArray("\x0D\x0E", 2), kMaster, 0, false, titleOnBody);
        const QByteArray second = slide(1, QByteArray("\x0D\x0E", 2), kMaster, 0, true, titleOnBody);
        const QByteArray third = slide(0, QByteArray("\x0F\x10", 2), kTitleMaster, 0, false, pptShape(0x402, 0xF, outlineRef(0)) + pptShape(0x403, 0x10, outlineRef(1)));
        // Three seconds, then the next slide; effect 6, direction 1, speed 2.
        const QByteArray linksSlide = slide(1, QByteArray("\x0D\x0E", 2), kMaster, 0, false, titleOnBody, le(3000, 4) + le(0, 4) + raw({1, 6}) + le(0x501, 2) + raw({2, 0, 0, 0}));
        const QByteArray notes = pptBox(1008, pptRec(1009, le(0x100, 4) + le(0, 4), 0, 1)
            + pptDrawing(pptShape(0x402, 0xC, pptRec(3999, le(2, 4)) + pptRec(4000, utf16(QStringLiteral("Remember the demo"))))));

        QByteArray s = document;
        QHash<quint32, quint32> at;
        auto place = [&](quint32 persistId, const QByteArray &rec) {
            at[persistId] = quint32(s.size());
            s += rec;
        };
        at[1] = 0;
        place(3, mainMaster);
        if (titleMaster) {
            place(7, titleMasterRec);
            place(8, third);
        }
        place(4, secondOld);
        place(5, first);
        place(6, notes);
        if (links) place(9, linksSlide);
        QVector<QPair<quint32, quint32>> older{{1, 0}, {3, at[3]}, {4, at[4]}, {5, at[5]}, {6, at[6]}};
        if (titleMaster) older << qMakePair(quint32(7), at[7]) << qMakePair(quint32(8), at[8]);
        if (links) older << qMakePair(quint32(9), at[9]);
        std::sort(older.begin(), older.end());
        const quint32 olderDirectory = quint32(s.size());
        // Each entry names one id (a run of one), so the ids need not be next to each other.
        s += persistDirectory(older);
        const quint32 olderEdit = quint32(s.size());
        s += userEdit(0, olderDirectory, 1);
        place(4, second);
        const quint32 newerDirectory = quint32(s.size());
        s += persistDirectory({{4, at[4]}});
        const quint32 newerEdit = quint32(s.size());
        s += userEdit(olderEdit, newerDirectory, 1);
        if (cycle) s.replace(olderEdit + 8 + 8, 4, le(newerEdit, 4));

        cfb::File f;
        cfb::Entry root;
        root.type = cfb::Entry::Root;
        root.name = QStringLiteral("Root Entry");
        f.entries << root;
        auto stream = [&](const QString &name, const QByteArray &data) {
            cfb::Entry e;
            e.type = cfb::Entry::Stream;
            e.name = name;
            e.data = data;
            f.entries[0].children << int(f.entries.size());
            f.entries << e;
        };
        const QByteArray user = QByteArray("Tester");
        stream(QStringLiteral("Current User"), pptRec(4086, le(20, 4) + le(0xE391C05F, 4) + le(newerEdit, 4) + le(user.size(), 2) + le(0x3F4, 2) + raw({3, 0, 0, 0}) + user + le(8, 4)));
        stream(pptDocumentStream(), s);
        stream(QStringLiteral("Pictures"), QByteArray());
        return cfb::write(f);
    }
    static QByteArray withStream(const QByteArray &file, const QString &name, const QByteArray &data)
    {
        cfb::File f;
        cfb::read(file, &f);
        f.setStream(name, data);
        return cfb::write(f);
    }

    // The text of a slide's placeholders by role.
    static QString placeholderText(const Document &d, const Page &slide, const QString &role)
    {
        for (const auto &it : slide.items)
            if (it->role == role && it->type() == ItemType::Text) return d.storyDoc(static_cast<const TextItem *>(it.get())->storyId)->toPlainText();
        return QStringLiteral("<none>");
    }
    static QTextDocument *placeholderStory(const Document &d, const Page &slide, const QString &role)
    {
        for (const auto &it : slide.items)
            if (it->role == role && it->type() == ItemType::Text) return d.storyDoc(static_cast<const TextItem *>(it.get())->storyId);
        return nullptr;
    }

private Q_SLOTS:
    // An old binary presentation opens as slides over a master and layouts:
    // the slide size, the slides in the order the file lists them, the newest
    // edit of each (the second slide is hidden by the later edit), the notes
    // of the one that has them, and the colors of the master's scheme.
    void binaryPresentationOpensWithMastersAndLayouts()
    {
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(d->pageSize(), QSizeF(720, 540));
        QCOMPARE(int(d->pages.size()), 2);
        QCOMPARE(placeholderText(*d, *d->pages[0], QStringLiteral("ph:title")), QStringLiteral("Quarterly Results"));
        QCOMPARE(placeholderText(*d, *d->pages[1], QStringLiteral("ph:title")), QStringLiteral("Plan"));
        QCOMPARE(d->pages[1]->title, QStringLiteral("Plan"));
        QVERIFY(d->pages[1]->hidden);
        QVERIFY(!d->pages[0]->hidden);
        QCOMPARE(deck::notesText(*d, *d->pages[0]), QStringLiteral("Remember the demo"));
        QVERIFY(deck::notesText(*d, *d->pages[1]).isEmpty());
        // Notes also find their slide by the slide they name.
        auto byAtom = deck::importBinaryPresentation(buildPpt(false, false, false, true), &err);
        QVERIFY2(byAtom, qPrintable(err));
        QCOMPARE(deck::notesText(*byAtom, *byAtom->pages[0]), QStringLiteral("Remember the demo"));
        QVERIFY(deck::notesText(*byAtom, *byAtom->pages[1]).isEmpty());

        int masters = 0;
        for (const auto &m : d->masters) masters += deck::isSlideMaster(*m);
        QCOMPARE(masters, 1);
        const MasterPage *layout = deck::layoutOfType(*d, QStringLiteral("obj"));
        QVERIFY(layout);
        QVERIFY(d->masterFor(*d->pages[0]) == layout);
        QVERIFY(d->masterFor(*d->pages[1]) == layout);
        QVERIFY(deck::isSlideMaster(*d->master(layout->baseId)));
        QStringList placeholders;
        for (const auto &it : layout->items) placeholders << deck::placeholderType(*it);
        QVERIFY(placeholders.contains(QStringLiteral("title")));
        QVERIFY(placeholders.contains(QStringLiteral("obj")));
        // Slides can also be added in the layouts every presentation has.
        for (const char *type : {"title", "titleOnly", "blank"}) QVERIFY2(deck::layoutOfType(*d, QString::fromLatin1(type)), type);

        // The scheme's eight colors: the presentation program turns them into a theme's dark 1 (text), light 1 (background),
        // dark 2 (title text), light 2 (shadow), accent 1 (fills), accent 2 (accent), hyperlink, and followed hyperlink.
        const deck::DeckTheme theme = deck::themeOf(*d);
        QCOMPARE(theme.colors[0], QColor(0x11, 0x22, 0x33));
        QCOMPARE(theme.colors[1], QColor(0xFA, 0xFA, 0xFA));
        QCOMPARE(theme.colors[2], QColor(0x44, 0x55, 0x66));
        QCOMPARE(theme.colors[3], QColor(0xDD, 0xDD, 0xCC));
        QCOMPARE(theme.colors[4], QColor(0x77, 0x88, 0x99));
        QCOMPARE(theme.colors[5], QColor(0xAA, 0xBB, 0xCC));
        QCOMPARE(theme.colors[10], QColor(1, 2, 3));
        QCOMPARE(theme.colors[11], QColor(4, 5, 6));
        // The fonts of the default title and body styles.
        QCOMPARE(theme.headingFont, QStringLiteral("Beta Serif"));
        QCOMPARE(theme.bodyFont, QStringLiteral("Alpha Sans"));
        QVERIFY(d->fromFile);
        // Tabs go to every inch when the styles don't say otherwise.
        QCOMPARE(d->defaultTabStop, 72.0);
    }

    // Text takes its look from the default styles of its kind and level, then
    // from its own paragraph and character runs.
    void binaryPresentationTextFormats()
    {
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(), &err);
        QVERIFY2(d, qPrintable(err));
        const Page &first = *d->pages[0];

        QTextDocument *title = placeholderStory(*d, first, QStringLiteral("ph:title"));
        QVERIFY(title);
        const QTextCharFormat tf = title->begin().begin().fragment().charFormat();
        QCOMPARE(tf.fontFamilies().toStringList(), QStringList{"Beta Serif"});
        QCOMPARE(tf.fontPointSize(), 44.0);
        QCOMPARE(ColorRef::fromString(tf.stringProperty(tp::ColorRefP)).rgbValue(), QColor(0x44, 0x55, 0x66));   // the scheme's title text
        QVERIFY((title->begin().blockFormat().alignment() & Qt::AlignHorizontal_Mask) == Qt::AlignHCenter);
        QVERIFY(!title->begin().textList());

        QTextDocument *body = placeholderStory(*d, first, QStringLiteral("ph:body:1"));
        QVERIFY(body);
        QCOMPARE(body->toPlainText(), QStringLiteral("First point\nSecond point\nThird point"));
        const QTextBlock one = body->begin(), two = one.next(), three = two.next();
        QVERIFY(one.isValid() && two.isValid() && three.isValid() && !three.next().isValid());
        // The first level: a round bullet hanging 45 points, text of 28 points in the first font and the text color.
        QVERIFY(one.textList());
        QCOMPARE(one.textList()->format().style(), QTextListFormat::ListDisc);
        QCOMPARE(one.textList()->format().intProperty(tp::ListLevel), 0);
        QCOMPARE(one.blockFormat().leftMargin(), 45.0);
        QCOMPARE(one.blockFormat().textIndent(), -45.0);
        const QTextCharFormat c1 = one.begin().fragment().charFormat();
        QCOMPARE(c1.fontFamilies().toStringList(), QStringList{"Alpha Sans"});
        QCOMPARE(c1.fontPointSize(), 28.0);
        QVERIFY(c1.fontWeight() != QFont::Bold);
        QCOMPARE(ColorRef::fromString(c1.stringProperty(tp::ColorRefP)).rgbValue(), QColor(0x11, 0x22, 0x33));
        // The second level: a dash, 24 points, 90 points in, and a bold first word.
        QVERIFY(two.textList());
        QCOMPARE(two.textList()->format().intProperty(tp::ListLevel), 1);
        QCOMPARE(two.textList()->format().stringProperty(tp::BulletChar), QString(QChar(0x2013)));
        QCOMPARE(two.blockFormat().leftMargin(), 90.0);
        QCOMPARE(two.blockFormat().textIndent(), -45.0);
        QStringList runs;
        for (auto f = two.begin(); !f.atEnd(); ++f) {
            const QTextCharFormat cf = f.fragment().charFormat();
            runs << QStringLiteral("%1|%2|%3").arg(f.fragment().text(), cf.fontWeight() >= QFont::Bold ? QStringLiteral("bold") : QStringLiteral("plain")).arg(cf.fontPointSize());
        }
        QCOMPARE(runs, (QStringList{"Second|bold|24", " point|plain|24"}));
        // Paragraphs of one level and mark go on in the one list; another level starts its own.
        QVERIFY(three.textList() == two.textList());
        QVERIFY(one.textList() != two.textList());

        // The second slide's body: a line break within a paragraph that has no bullet.
        QTextDocument *plan = placeholderStory(*d, *d->pages[1], QStringLiteral("ph:body:1"));
        QVERIFY(plan);
        QCOMPARE(plan->toPlainText(), QStringLiteral("Ship\nit"));
        QCOMPARE(plan->blockCount(), 1);
        QVERIFY(plan->toRawText().contains(QChar(QChar::LineSeparator)));
        QVERIFY(!plan->begin().textList());
    }

    // A title master is the base of the title layout's own objects; the
    // centered title and the subtitle are placeholders of that layout.
    void binaryPresentationTitleMasterMakesTheTitleLayout()
    {
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(true), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 3);
        int masters = 0;
        for (const auto &m : d->masters) masters += deck::isSlideMaster(*m);
        QCOMPARE(masters, 1);
        const Page &welcome = *d->pages[2];
        const MasterPage *layout = d->masterFor(welcome);
        QVERIFY(layout);
        QCOMPARE(deck::layoutType(*layout), QStringLiteral("title"));
        QVERIFY(deck::isSlideMaster(*d->master(layout->baseId)));
        QStringList types;
        for (const auto &it : layout->items) types << deck::placeholderType(*it);
        QVERIFY(types.contains(QStringLiteral("ctrTitle")));
        QVERIFY(types.contains(QStringLiteral("subTitle")));
        QCOMPARE(placeholderText(*d, welcome, QStringLiteral("ph:ctrTitle")), QStringLiteral("Welcome"));
        QCOMPARE(placeholderText(*d, welcome, QStringLiteral("ph:subTitle:1")), QStringLiteral("To the talk"));
        // The title master's own size for the centered title (54 points), centered.
        QTextDocument *ctr = placeholderStory(*d, welcome, QStringLiteral("ph:ctrTitle"));
        QVERIFY(ctr);
        QCOMPARE(ctr->begin().begin().fragment().charFormat().fontPointSize(), 54.0);
        QVERIFY((ctr->begin().blockFormat().alignment() & Qt::AlignHorizontal_Mask) == Qt::AlignHCenter);
    }

    // A text's fields, links, languages, and ruler come through, and so does how a
    // slide moves on (kept as the file has it).
    void binaryPresentationFieldsLinksRulerAndTransition()
    {
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, true), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 3);
        const Page &slide = *d->pages[2];
        QTextDocument *body = placeholderStory(*d, slide, QStringLiteral("ph:body:1"));
        QVERIFY(body);
        QCOMPARE(body->toPlainText(), QStringLiteral("Go to the site. Slide %1").arg(QChar(QChar::ObjectReplacementCharacter)));
        QStringList runs;
        for (auto f = body->begin().begin(); !f.atEnd(); ++f) {
            const QTextCharFormat cf = f.fragment().charFormat();
            runs << QStringLiteral("%1|%2|%3|%4").arg(f.fragment().text(), cf.isAnchor() ? cf.anchorHref() : QString(), cf.stringProperty(tp::Language), cf.stringProperty(tp::Field));
        }
        QCOMPARE(runs, (QStringList{QStringLiteral("Go to ||fr-FR|"), QStringLiteral("the site|https://example.org/site|fr-FR|"), QStringLiteral(".||fr-FR|"),
                                    QStringLiteral(" Slide |||"), QString(QChar(QChar::ObjectReplacementCharacter)) + QStringLiteral("|||page")}));
        // The ruler's first level: text at 90 points, its first line (and bullet) at 36.
        QCOMPARE(body->begin().blockFormat().leftMargin(), 90.0);
        QCOMPARE(body->begin().blockFormat().textIndent(), -54.0);

        // Effect 6 (a fade), medium speed, moving on by itself after 3 seconds and on a click.
        const deck::Transition fade = deck::transitionOf(slide);
        QCOMPARE(fade.type, QStringLiteral("fade"));
        QCOMPARE(fade.duration, 0.5);
        QVERIFY(fade.advanceAfter.has_value());
        QCOMPARE(*fade.advanceAfter, 3.0);
        QVERIFY(fade.advanceOnClick);
        // What the file said is kept too.
        const QJsonObject raw = slide.props.value(QStringLiteral("binaryTransition")).toObject();
        QCOMPARE(raw.value(QStringLiteral("effect")).toInt(), 6);
        QCOMPARE(raw.value(QStringLiteral("direction")).toInt(), 1);
        QCOMPARE(raw.value(QStringLiteral("speed")).toInt(), 2);
        QCOMPARE(raw.value(QStringLiteral("advanceMs")).toInt(), 3000);
        QVERIFY(!d->pages[0]->props.contains(QStringLiteral("binaryTransition")));
        QVERIFY(!d->pages[0]->props.contains(QStringLiteral("transition")));
    }

    // Files that can't be read say why: protected with a password, cut short,
    // edits that go round in a circle, no record of who edited last.
    void binaryPresentationRefusesDamagedFiles()
    {
        const QByteArray good = buildPpt();
        QString err;
        QVERIFY(deck::importBinaryPresentation(good, &err));

        cfb::File f;
        QVERIFY(cfb::read(good, &f));
        // A password: the header token of the current-user record says the presentation is encrypted.
        QByteArray user = f.stream(QStringLiteral("Current User"));
        user.replace(12, 4, le(0xF3D1C4DF, 4));
        QVERIFY(!deck::importBinaryPresentation(withStream(good, QStringLiteral("Current User"), user), &err));
        QVERIFY2(err.contains(QStringLiteral("password")), qPrintable(err));

        err.clear();
        const QByteArray stream = f.stream(pptDocumentStream());
        QVERIFY(!deck::importBinaryPresentation(withStream(good, pptDocumentStream(), stream.left(stream.size() / 2)), &err));
        QVERIFY(!err.isEmpty());

        err.clear();
        QVERIFY(!deck::importBinaryPresentation(buildPpt(false, true), &err));
        QVERIFY2(err.contains(QStringLiteral("circle")), qPrintable(err));

        err.clear();
        cfb::File without;
        QVERIFY(cfb::read(good, &without));
        without.entries[0].children.removeAll(without.find(QStringLiteral("Current User")));
        QVERIFY(!deck::importBinaryPresentation(cfb::write(without), &err));
        QVERIFY(!err.isEmpty());

        err.clear();
        QVERIFY(!deck::importBinaryPresentation(QByteArray("not a compound file at all"), &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!deck::importBinaryPresentation(QByteArray(), &err));
        QVERIFY(!err.isEmpty());
        // A compound file that holds no presentation.
        cfb::File other;
        QVERIFY(cfb::read(good, &other));
        other.entries[0].children.removeAll(other.find(pptDocumentStream()));
        err.clear();
        QVERIFY(!deck::importBinaryPresentation(cfb::write(other), &err));
        QVERIFY(!err.isEmpty());
    }

    // A text of thousands of paragraphs, a bullet on each, is read to a limit and
    // quickly (a list takes its items at a cost that grows with its length).
    void binaryPresentationLimitsHugeTexts()
    {
        QString text;
        for (int i = 0; i < 30000; ++i) text += QStringLiteral("a\r");
        const QByteArray big = pptRec(3999, le(1, 4)) + pptRec(4000, utf16(text)) + pptRec(4001, pfRun(text.size() + 1, 0, 0) + cfRun(text.size() + 1, 0));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, big), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(err.contains(QStringLiteral("paragraphs")), qPrintable(err));
        const QTextDocument *body = placeholderStory(*d, *d->pages[0], QStringLiteral("ph:body:1"));
        QVERIFY(body);
        QCOMPARE(body->blockCount(), 10000);
        QVERIFY(body->begin().textList());
    }

    // Damage to the bytes of the streams never crashes or hangs the reader: a
    // file is a presentation, or says it isn't.
    void binaryPresentationSurvivesRandomDamage()
    {
        const QByteArray good = buildPpt(true);
        cfb::File f;
        QVERIFY(cfb::read(good, &f));
        const QByteArray stream = f.stream(pptDocumentStream());
        quint32 seed = 12345;
        auto next = [&] { return seed = seed * 1664525u + 1013904223u; };
        int opened = 0;
        for (int round = 0; round < 300; ++round) {
            QByteArray bad = stream;
            const int hits = 1 + int(next() % 6);
            for (int i = 0; i < hits; ++i) bad[int((next() >> 8) % quint32(bad.size()))] = char(next() >> 16);
            if (round % 7 == 0) bad.truncate(int((next() >> 8) % quint32(bad.size())));
            QString err;
            auto d = deck::importBinaryPresentation(withStream(good, pptDocumentStream(), bad), &err);
            if (d) {
                ++opened;
                QVERIFY(d->pages.size() <= 3);
            } else {
                QVERIFY(!err.isEmpty());
            }
        }
        QVERIFY(opened > 0);
    }

    // Presentation files open through one function that looks at the file, not
    // its name: the old binary format by its compound file signature.
    void presentationFilesOpenByTheirFormat()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("talk.ppt"));
        QFile out(path);
        QVERIFY(out.open(QIODevice::WriteOnly));
        out.write(buildPpt());
        out.close();
        QString err;
        auto d = deck::importAnyPresentationFile(path, &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 2);

        // The name doesn't decide it.
        const QString renamed = dir.filePath(QStringLiteral("talk.pptx"));
        QVERIFY(QFile::copy(path, renamed));
        auto again = deck::importAnyPresentationFile(renamed, &err);
        QVERIFY2(again, qPrintable(err));
        QCOMPARE(int(again->pages.size()), 2);

        // A newer presentation goes to the reader for its format; other files say they aren't presentations.
        err.clear();
        QFile junk(dir.filePath(QStringLiteral("junk.pptx")));
        QVERIFY(junk.open(QIODevice::WriteOnly));
        junk.write("PK nothing");
        junk.close();
        QVERIFY(!deck::importAnyPresentationFile(junk.fileName(), &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!deck::importAnyPresentationFile(dir.filePath(QStringLiteral("missing.ppt")), &err));
        QVERIFY(!err.isEmpty());
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

    // Each app names its color and badge for the shared window: the File
    // button and every accent in it follow, and the dark shade reads on the
    // dark ribbon.
    void appInfoColorsPerApp()
    {
        auto contrast = [](const QColor &a, const QColor &b) {
            auto lum = [](const QColor &c) {
                auto f = [](double v) { return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
                return 0.2126 * f(c.redF()) + 0.7152 * f(c.greenF()) + 0.0722 * f(c.blueF());
            };
            const double la = lum(a), lb = lum(b);
            return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
        };
        const AppInfo pub = pubAppInfo(), deckInfo = deckAppInfo();
        QCOMPARE(pub.accent, QColor(0x9E, 0x1F, 0x63));
        QCOMPARE(pub.accentDark, QColor(0xE0, 0x6A, 0xA5));
        QCOMPARE(pub.badge, QStringLiteral("JP"));
        QCOMPARE(deckInfo.accent, QColor(0xB6, 0x42, 0x0F));
        QCOMPARE(deckInfo.badge, QStringLiteral("JD"));
        QVERIFY(deckInfo.accentDark.isValid());
        QVERIFY(contrast(deckInfo.accentDark, QColor(0x26, 0x2B, 0x33)) >= 4.5);   // on the dark ribbon
        QVERIFY(contrast(deckInfo.accent, QColor(0xFB, 0xFB, 0xFC)) >= 4.5);       // on the light one
        QVERIFY(contrast(deckInfo.highlight, Qt::white) >= 4.5);                   // selected text on the dark selection
        // The shared parts read the running app's.
        setUiDark(false);
        QCOMPARE(uiAccent(), deckInfo.accent);
        {
            DeckWindow w;
            auto *file = w.ribbon()->findChild<QAbstractButton *>(QStringLiteral("jpRibbonFile"));
            QVERIFY(file);
            file->resize(file->sizeHint());
            // The badge is a circle of the accent at the button's left.
            const QColor badge = file->grab().toImage().pixelColor(7, file->height() / 2);
            QVERIFY2(std::abs(badge.red() - deckInfo.accent.red()) < 12 && std::abs(badge.green() - deckInfo.accent.green()) < 12
                         && std::abs(badge.blue() - deckInfo.accent.blue()) < 12,
                     qPrintable(badge.name()));
        }
        setUiDark(true);
        QCOMPARE(uiAccent(), deckInfo.accentDark);
        setUiDark(false);
    }

    // The slide thumbnails: a row for each slide, the current row following
    // the editor and the editor following Up and Down; the current slide
    // outlined in the accent without a fill over it; a hidden slide marked.
    void thumbnailsFollowTheEditor()
    {
        setUiDark(false);
        DeckWindow w;
        Editor *ed = w.editor();
        SlidesPane *pane = w.slidesPane();
        QCOMPARE(pane->count(), 1);
        QCOMPARE(pane->currentRow(), 0);
        w.newSlide();
        w.newSlide();
        QTRY_COMPARE(pane->count(), 3);
        QCOMPARE(pane->currentRow(), 2);
        ed->setCurrentPage(0);
        QCOMPARE(pane->currentRow(), 0);
        QCOMPARE(pane->item(1)->text(), QStringLiteral("2"));   // the number, which the row draws at the thumbnail's left
        QTest::keyClick(pane, Qt::Key_Down);
        QCOMPARE(ed->currentPage(), 1);
        QTest::keyClick(pane, Qt::Key_Down);
        QCOMPARE(ed->currentPage(), 2);
        QTest::keyClick(pane, Qt::Key_Down);   // the last stays
        QCOMPARE(ed->currentPage(), 2);
        QTest::keyClick(pane, Qt::Key_Up);
        QCOMPARE(ed->currentPage(), 1);
        QCOMPARE(pane->accessibleName(), QStringLiteral("Slides"));
        QCOMPARE(pane->item(1)->data(Qt::AccessibleTextRole).toString(), QStringLiteral("Slide 2"));
        w.toggleHidden();
        QTRY_VERIFY(pane->item(1)->data(SlidesPane::HiddenRole).toBool());
        QCOMPARE(pane->item(1)->data(Qt::AccessibleTextRole).toString(), QStringLiteral("Slide 2, hidden"));
        QVERIFY(!pane->item(0)->data(SlidesPane::HiddenRole).toBool());

        // Drawn: the current thumbnail has the accent around it and its own white inside; the others a gray edge.
        ed->setCurrentPage(2);
        pane->resize(260, 420);
        pane->show();
        const QImage img = pane->viewport()->grab().toImage();
        const QRect cur = pane->thumbnailRect(2), other = pane->thumbnailRect(0), hid = pane->thumbnailRect(1);
        QCOMPARE(img.pixelColor(cur.left() - 2, cur.center().y()), deckAppInfo().accent);
        QCOMPARE(img.pixelColor(cur.center().x(), cur.top() - 2), deckAppInfo().accent);
        QCOMPARE(img.pixelColor(cur.left() + 4, cur.top() + 4), QColor(Qt::white));   // no fill over the slide
        QVERIFY(img.pixelColor(other.left() - 2, other.center().y()) != deckAppInfo().accent);
        QCOMPARE(img.pixelColor(other.left() + 4, other.top() + 4), QColor(Qt::white));
        // Hidden: faded, so its white isn't white any more.
        QVERIFY(img.pixelColor(hid.left() + 4, hid.top() + 4) != QColor(Qt::white));
        pane->hide();
    }

    // A new slide size fits everything on every slide, layout, and the slide
    // master to it (Ensure Fit): the layouts and placeholders take the new
    // shape, text shrinks by the width's ratio, and the other objects on a
    // slide keep their shape, smaller and centered.
    void slideSizeScalesEverything()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        auto shape = std::make_shared<ShapeItem>();
        shape->rect = QRectF(100, 100, 200, 100);
        d->pages[1]->items.push_back(shape);
        const auto *title = static_cast<const TextItem *>(d->pages[1]->items[0].get());
        const MasterPage *layout = deck::layoutOfType(*d, QStringLiteral("obj"));
        const QRectF slideBefore = title->rect, layoutBefore = layout->items[0]->rect, masterBefore = d->masters[0]->items[0]->rect;
        auto size = [&](const QString &storyId) { return QTextCursor(d->storyDoc(storyId)).charFormat().fontPointSize(); };
        const double textBefore = size(title->storyId);
        QVERIFY(textBefore > 0);

        deck::setSlideSize(*d, QSizeF(deck::kStandardWidth, deck::kStandardHeight));
        QCOMPARE(d->pageSize(), QSizeF(720, 540));
        QCOMPARE(d->setup.sizeName, QStringLiteral("Standard"));
        auto narrower = [](const QRectF &r) { return QRectF(r.x() * 0.75, r.y(), r.width() * 0.75, r.height()); };
        QCOMPARE(title->rect, narrower(slideBefore));
        QCOMPARE(layout->items[0]->rect, narrower(layoutBefore));
        QCOMPARE(d->masters[0]->items[0]->rect, narrower(masterBefore));
        QCOMPARE(size(title->storyId), textBefore * 0.75);
        QCOMPARE(shape->rect, QRectF(75, 100 * 0.75 + 67.5, 150, 75));   // in shape, smaller, and in the middle
        // A slide still draws, at its new size.
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        QCOMPARE(Renderer::renderToImage(ctx, 0, 0.5).size(), QSize(360, 270));

        // Wider again: the placeholders and layouts are where they were, nothing else grows.
        deck::setSlideSize(*d, QSizeF(deck::kSlideWidth, deck::kSlideHeight));
        QCOMPARE(d->setup.sizeName, QStringLiteral("Widescreen"));
        QCOMPARE(title->rect, slideBefore);
        QCOMPARE(layout->items[0]->rect, layoutBefore);
        QCOMPARE(size(title->storyId), textBefore * 0.75);
        QCOMPARE(shape->rect, QRectF(75 + 120, 100 * 0.75 + 67.5, 150, 75));
    }

    // Header & Footer puts the slide number, the date, and a line of text in
    // each slide's own placeholders for them, and takes them away again.
    void headerAndFooterPlaceholders()
    {
        auto d = deck::newPresentation();   // a title slide
        for (int i = 0; i < 2; ++i) deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        auto typesOf = [&](int i) {
            QStringList t;
            for (const auto &it : d->pages[i]->items) t << deck::placeholderType(*it);
            return t;
        };
        deck::FooterOptions o;
        o.slideNumber = true;
        o.skipTitleSlide = true;
        deck::applyFooters(*d, o);
        QVERIFY(d->appData.value(QStringLiteral("footers")).toBool());
        QCOMPARE(typesOf(0), (QStringList{"ctrTitle", "subTitle"}));   // not on the title slide
        QCOMPARE(typesOf(1), (QStringList{"title", "obj", "sldNum"}));
        QCOMPARE(typesOf(2), (QStringList{"title", "obj", "sldNum"}));
        // The number is the page number field, in the layout's place and look.
        const auto *num = static_cast<const TextItem *>(d->pages[1]->items.back().get());
        QCOMPARE(num->rect, deck::layoutOfType(*d, QStringLiteral("obj"))->items.back()->rect);
        const QTextDocument *sd = d->storyDoc(num->storyId);
        QCOMPARE(sd->toPlainText(), QString(QChar(QChar::ObjectReplacementCharacter)));
        QCOMPARE(QTextCursor(const_cast<QTextDocument *>(sd)).charFormat().stringProperty(tp::Field), QStringLiteral("page"));
        // Apply to one slide: its own, and what new slides get stays.
        o.date = o.footer = true;
        o.footerText = QStringLiteral("Quarterly review");
        o.skipTitleSlide = false;
        deck::applyFooters(*d, o, d->pages[0].get());
        QCOMPARE(typesOf(0), (QStringList{"ctrTitle", "subTitle", "dt", "ftr", "sldNum"}));
        QCOMPARE(d->storyDoc(static_cast<const TextItem *>(d->pages[0]->items[3].get())->storyId)->toPlainText(), QStringLiteral("Quarterly review"));
        QCOMPARE(typesOf(1), (QStringList{"title", "obj", "sldNum"}));
        const int added = deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("titleOnly")));
        QCOMPARE(typesOf(added), (QStringList{"title", "sldNum"}));
        // Apply to all with nothing chosen takes them all away.
        deck::applyFooters(*d, deck::FooterOptions());
        for (int i = 0; i < d->pages.size(); ++i) QVERIFY2(!typesOf(i).contains(QStringLiteral("sldNum")) && !typesOf(i).contains(QStringLiteral("ftr")), qPrintable(typesOf(i).join(',')));
        QVERIFY(!d->appData.value(QStringLiteral("footers")).toBool());
    }

    // A slide's own background, or one for all of them through the master.
    void backgroundsSetAndClear()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        deck::setSlideBackground(*d->pages[1], QColor(0x33, 0x66, 0x99));
        QCOMPARE(d->pages[1]->background, Fill::solid(ColorRef::rgb(QColor(0x33, 0x66, 0x99))));
        QVERIFY(d->pages[0]->background.type == Fill::NoFill);
        deck::setAllBackgrounds(*d, QColor(0x11, 0x22, 0x33));
        QVERIFY(d->pages[1]->background.type == Fill::NoFill);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        for (int i = 0; i < 2; ++i) QCOMPARE(Renderer::renderToImage(ctx, i, 0.1).pixelColor(3, 3), QColor(0x11, 0x22, 0x33));
    }

    // The ribbon builds with nothing missing and has the commands of the
    // Home, Insert, Design, and View tabs; each of those tabs gives every
    // control its own KeyTip.
    void ribbonHasTheTabsCommands()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        for (const char *id : {"fmt.bold", "fmt.shadow", "fmt.grow", "fmt.shrink", "fmt.clear", "case.2", "spacing.Tight", "edit.formatPainter", "para.bullets", "bullet.check",
                               "para.numbers", "number.3", "para.indentDec", "para.indentInc", "ls.1.5", "fmt.alignCenter", "tb.dirVertical", "valign.1", "cols.2", "arr.front",
                               "arr.backward", "arr.group", "arr.flipH", "slide.new", "insert.picture", "insert.table", "insert.textBox", "insert.headerFooter",
                               "design.sizeStandard", "design.sizeWidescreen", "design.formatBackground", "view.rulers", "view.gridlines", "view.guides", "view.notes",
                               "view.fit", "view.normal", "view.sorter", "view.slideShow", "zoom.in", "zoom.out", "zoom.100"})
            QVERIFY2(w.act(QString::fromLatin1(id)), id);
        QStringList tabs;
        for (int i = 0; i < w.ribbon()->tabCount(); ++i) tabs << w.ribbon()->tabName(i);
        QVERIFY(tabs.indexOf(QStringLiteral("Insert")) < tabs.indexOf(QStringLiteral("Design")));
        QVERIFY(tabs.indexOf(QStringLiteral("Design")) < tabs.indexOf(QStringLiteral("Slide Show")));
        for (const QString &name : {QStringLiteral("Home"), QStringLiteral("Insert"), QStringLiteral("Design"), QStringLiteral("View")}) {
            QVERIFY2(w.ribbon()->tab(name), qPrintable(name));
            QStringList seen;
            for (const QWidget *c : w.ribbon()->tab(name)->findChildren<QWidget *>()) {
                const QString k = keytip(c);
                if (k.isEmpty()) continue;
                QVERIFY2(k.size() <= 2 && k.indexOf(QRegularExpression(QStringLiteral("[^A-Z0-9]"))) < 0, qPrintable(name + ": " + k));
                QVERIFY2(!seen.contains(k), qPrintable(name + ": two controls have the KeyTip " + k));
                seen << k;
            }
            QVERIFY2(seen.size() > 3, qPrintable(name));
        }
    }

    // A presentation shows no rulers until asked, and no outline around every
    // object; the View tab's check boxes turn the ruler, gridlines, and guides on.
    void normalViewDefaults()
    {
        DeckWindow w;
        QVERIFY(!w.editor()->view.rulers);
        QVERIFY(!w.editor()->view.boundaries);
        QVERIFY(w.canvas()->hRuler()->isHidden() && w.canvas()->vRuler()->isHidden());
        QVERIFY(!w.act("view.rulers")->isChecked() && !w.act("view.gridlines")->isChecked() && !w.act("view.guides")->isChecked());
        w.act("view.rulers")->trigger();
        QVERIFY(w.editor()->view.rulers);
        QVERIFY(!w.canvas()->hRuler()->isHidden() && !w.canvas()->vRuler()->isHidden());
        QVERIFY(w.act("view.rulers")->isChecked());
        w.act("view.rulers")->trigger();
        QVERIFY(w.canvas()->hRuler()->isHidden());
        w.act("view.gridlines")->trigger();
        w.act("view.guides")->trigger();
        QVERIFY(w.canvas()->gridlines() && w.canvas()->guides());
        // The boxes are the commands'.
        QCheckBox *ruler = nullptr;
        for (QCheckBox *b : w.ribbon()->findChildren<QCheckBox *>())
            if (b->text() == QStringLiteral("Ruler")) ruler = b;
        QVERIFY(ruler);
        ruler->click();
        QVERIFY(w.editor()->view.rulers);
        QVERIFY(ruler->isChecked());
    }

    // A placeholder is outlined (dotted) while it is empty or selected, and a
    // slide's objects aren't outlined the way every object is in the other apps.
    void placeholdersAreOutlinedOnlyWhileEmptyOrSelected()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        Editor *ed = w.editor();
        auto outlined = [&] {
            const QImage img = w.canvas()->viewport()->grab().toImage();
            const auto *title = static_cast<const TextItem *>(ed->doc()->pages[0]->items[0].get());
            const QPoint at = w.canvas()->pageToView(QPointF(title->rect.center().x(), title->rect.top())).toPoint();
            int marks = 0;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -20; dx < 20; ++dx) {
                    const QColor c = img.pixelColor(at.x() + dx, at.y() + dy);
                    marks += c.lightness() < 235;
                }
            return marks;
        };
        QVERIFY(outlined() > 3);   // empty
        auto *title = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get());
        ed->doc()->storyDoc(title->storyId)->setPlainText(QStringLiteral("Hi"));
        ed->notifyLive();
        QCOMPARE(outlined(), 0);   // text in it, not selected
        ed->select(title->id);
        QVERIFY(outlined() > 3);   // selected
        ed->clearSelection();
        QCOMPARE(outlined(), 0);
    }

    // The status bar: the slide's place, Notes, the views, and a zoom slider
    // with its percent and a fit button.
    void statusBarShowsSlideAndZoom()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        auto *place = w.findChild<QLabel *>(QStringLiteral("slideNumber"));
        QVERIFY(place);
        QCOMPARE(place->text(), QStringLiteral("Slide 1 of 1"));
        w.newSlide();
        QTRY_COMPARE(place->text(), QStringLiteral("Slide 2 of 2"));
        QVERIFY(w.act("view.sorter")->isEnabled());
        QVERIFY(w.act("view.normal")->isChecked());
        // Notes shows and hides the notes below the slide.
        QVERIFY(!w.notesPane()->isHidden());
        QTRY_VERIFY(w.act("view.notes")->isChecked());
        w.act("view.notes")->trigger();
        QVERIFY(w.notesPane()->isHidden());
        QTRY_VERIFY(!w.act("view.notes")->isChecked());
        w.act("view.notes")->trigger();
        QVERIFY(!w.notesPane()->isHidden());
        // Zoom: the slider zooms, the label follows the canvas, and Fit comes back to the window.
        auto *slider = w.findChild<QSlider *>(QStringLiteral("zoomSlider"));
        auto *percent = w.findChild<QLabel *>(QStringLiteral("zoomPercent"));
        QVERIFY(slider && percent);
        slider->setValue(0);
        QCOMPARE(w.canvas()->zoom(), 1.0);
        QCOMPARE(percent->text(), QStringLiteral("100%"));
        w.canvas()->setZoom(0.5);
        QCOMPARE(percent->text(), QStringLiteral("50%"));
        w.act("view.fit")->trigger();
        QVERIFY(w.canvas()->fitMode() == Canvas::Fit::WholePage);
        w.act("zoom.200")->trigger();
        QCOMPARE(w.canvas()->zoom(), 2.0);
    }

    // The slide size from the window is one step that undoes, and the
    // thumbnails take the slides' new shape.
    void slideSizeFromTheWindow()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        const QRectF before = ed->doc()->pages[0]->items[0]->rect;
        w.setSlideSize(QSizeF(deck::kStandardWidth, deck::kStandardHeight));
        QCOMPARE(ed->doc()->pageSize(), QSizeF(720, 540));
        QVERIFY(ed->doc()->pages[0]->items[0]->rect != before);
        QTRY_COMPARE(w.slidesPane()->iconSize(), QSize(168, 126));
        QTRY_VERIFY(w.act("design.sizeStandard")->isChecked());
        QVERIFY(!w.act("design.sizeWidescreen")->isChecked());
        ed->undo();
        QCOMPARE(ed->doc()->pageSize(), QSizeF(960, 540));
        QCOMPARE(ed->doc()->pages[0]->items[0]->rect, before);
        QTRY_VERIFY(w.act("design.sizeWidescreen")->isChecked());
    }

    // Format Background and Header & Footer from the window are steps that undo.
    void backgroundAndFooterFromTheWindow()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        w.setBackground(QColor(0x33, 0x66, 0x99), false);
        QCOMPARE(ed->doc()->pages[1]->background, Fill::solid(ColorRef::rgb(QColor(0x33, 0x66, 0x99))));
        QVERIFY(ed->doc()->pages[0]->background.type == Fill::NoFill);
        ed->undo();
        QVERIFY(ed->doc()->pages[1]->background.type == Fill::NoFill);
        w.setBackground(QColor(0x11, 0x22, 0x33), true);
        QCOMPARE(ed->doc()->masters[0]->background, Fill::solid(ColorRef::rgb(QColor(0x11, 0x22, 0x33))));
        ed->undo();
        QCOMPARE(ed->doc()->masters[0]->background, Fill::solid(ColorRef::rgb(Qt::white)));

        deck::FooterOptions o;
        o.slideNumber = true;
        o.skipTitleSlide = true;
        w.applyFooters(o, true);
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), 2);
        QCOMPARE(int(ed->doc()->pages[1]->items.size()), 3);
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[1]->items.size()), 2);
        QVERIFY(!ed->doc()->appData.value(QStringLiteral("footers")).toBool());
    }

    // The Header & Footer dialog hands back what was chosen and which button.
    void headerFooterDialogReturnsTheChoice()
    {
        deck::FooterOptions o;
        o.slideNumber = true;
        o.footerText = QStringLiteral("Quarterly");
        HeaderFooterDialog dlg(o);
        QVERIFY(dlg.options().slideNumber && !dlg.options().date && !dlg.options().footer);
        QVERIFY(!dlg.findChild<QLineEdit *>(QStringLiteral("footerText"))->isEnabled());   // until Footer is checked
        dlg.findChild<QCheckBox *>(QStringLiteral("footer"))->setChecked(true);
        dlg.findChild<QCheckBox *>(QStringLiteral("date"))->setChecked(true);
        dlg.findChild<QCheckBox *>(QStringLiteral("skipTitle"))->setChecked(true);
        dlg.findChild<QLineEdit *>(QStringLiteral("footerText"))->setText(QStringLiteral("Q3 review"));
        const deck::FooterOptions got = dlg.options();
        QVERIFY(got.date && got.slideNumber && got.footer && got.skipTitleSlide);
        QCOMPARE(got.footerText, QStringLiteral("Q3 review"));
        dlg.findChild<QPushButton *>(QStringLiteral("applyToAll"))->click();
        QCOMPARE(dlg.result(), int(QDialog::Accepted));
        QVERIFY(dlg.applyToAll());
        HeaderFooterDialog one(o);
        one.findChild<QPushButton *>(QStringLiteral("apply"))->click();
        QCOMPARE(one.result(), int(QDialog::Accepted));
        QVERIFY(!one.applyToAll());
    }

    // New Slide's gallery shows each layout as a picture and says which was chosen.
    void layoutGalleryShowsEachLayout()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        LayoutGallery g(ed);
        QCOMPARE(g.count(), 9);
        QCOMPARE(g.typeAt(0), QStringLiteral("title"));
        QCOMPARE(g.typeAt(8), QStringLiteral("picTx"));
        QCOMPARE(layoutPicture(*ed->doc(), *deck::layoutOfType(*ed->doc(), QStringLiteral("obj")), 112).size(), QSize(112, 63));
        auto busy = [&](const QString &type) {
            const QImage img = layoutPicture(*ed->doc(), *deck::layoutOfType(*ed->doc(), type), 112);
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (img.pixelColor(x, y) != QColor(Qt::white)) return true;
            return false;
        };
        QVERIFY(busy(QStringLiteral("obj")));
        QVERIFY(busy(QStringLiteral("twoObj")));
        QVERIFY(!busy(QStringLiteral("blank")));
        QSignalSpy chosen(&g, &LayoutGallery::chosen);
        QTest::keyClick(&g, Qt::Key_Right);
        QTest::keyClick(&g, Qt::Key_Right);
        QTest::keyClick(&g, Qt::Key_Down);
        QTest::keyClick(&g, Qt::Key_Return);
        QCOMPARE(chosen.count(), 1);
        QCOMPARE(chosen.first().first().toString(), g.typeAt(4));   // the middle of the second row
    }

    // The layouts gallery in the New Slide menu makes a slide with the layout
    // chosen, and the one in the Layout menu changes the slide's.
    void layoutMenusUseTheGallery()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        // In the order the ribbon built them: Home's New Slide and Layout, then Insert's New Slide.
        QList<LayoutGallery *> galleries;
        for (QMenu *m : w.findChildren<QMenu *>())
            if (auto *g = m->findChild<LayoutGallery *>()) galleries << g;
        QCOMPARE(galleries.size(), 3);
        LayoutGallery *newSlide = galleries.at(0), *layout = galleries.at(1);
        for (int i = 0; i < 6; ++i) QTest::keyClick(newSlide, Qt::Key_Right);
        QTest::keyClick(newSlide, Qt::Key_Return);   // Title Only
        QCOMPARE(int(ed->doc()->pages.size()), 2);
        QCOMPARE(deck::layoutType(*ed->doc()->masterFor(*ed->doc()->pages[1])), QStringLiteral("titleOnly"));
        QCOMPARE(ed->currentPage(), 1);
        QTest::keyClick(layout, Qt::Key_Home);
        QTest::keyClick(layout, Qt::Key_Return);   // Title Slide
        QCOMPARE(deck::layoutType(*ed->doc()->masterFor(*ed->doc()->pages[1])), QStringLiteral("title"));
        QCOMPARE(int(ed->doc()->pages.size()), 2);
    }

    // Insert > Pictures centers the picture on the slide at its natural size, one
    // step that undoes; Insert > Table puts a table there too.
    void insertPictureAndTable()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QImage image(200, 100, QImage::Format_RGB32);
        image.fill(Qt::red);
        QVERIFY(image.save(dir.filePath(QStringLiteral("red.png"))));
        QFile text(dir.filePath(QStringLiteral("notes.txt")));
        QVERIFY(text.open(QIODevice::WriteOnly));
        text.write("not a picture");
        text.close();
        DeckWindow w;
        Editor *ed = w.editor();
        const int before = int(ed->doc()->pages[0]->items.size());
        QVERIFY(!w.insertPicture(dir.filePath(QStringLiteral("notes.txt"))));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);
        QVERIFY(w.insertPicture(dir.filePath(QStringLiteral("red.png"))));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        const auto *pic = dynamic_cast<const PictureItem *>(ed->doc()->pages[0]->items.back().get());
        QVERIFY(pic);
        QCOMPARE(pic->rect, QRectF(480 - 75, 270 - 37.5, 150, 75));
        QCOMPARE(ed->selection(), QStringList{pic->id});
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);
        // Dropped on the slide, it goes where it was dropped.
        Q_EMIT w.canvas()->insertFilesWanted({dir.filePath(QStringLiteral("red.png"))}, QPointF(100, 50));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        QCOMPARE(ed->doc()->pages[0]->items.back()->rect.topLeft(), QPointF(100, 50));
        ed->undo();

        w.insertTable(3, 4);
        const auto *table = dynamic_cast<const TableItem *>(ed->doc()->pages[0]->items.back().get());
        QVERIFY(table);
        QCOMPARE(table->rows, 3);
        QCOMPARE(table->cols, 4);
        QCOMPARE(table->rect.center(), QPointF(480, 270));
        QCOMPARE(QTextCursor(ed->doc()->storyDoc(table->cells[0].storyId)).charFormat().fontPointSize(), 18.0);
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

    // Effect Options lists the ways the slide's transition goes.
    void windowEffectOptionsMenu()
    {
        DeckWindow w;
        QAction *command = w.findChild<QAction *>(QStringLiteral("ribbon.effectOptions"));
        QVERIFY(command);
        QMenu *menu = nullptr;
        for (QObject *o : command->associatedObjects())
            if (auto *button = qobject_cast<QToolButton *>(o)) menu = button->menu();
        QVERIFY(menu);
        auto listed = [&] {
            Q_EMIT menu->aboutToShow();
            return menu->actions();
        };
        QCOMPARE(listed().size(), 1);   // nothing to choose for no transition
        QVERIFY(!listed().at(0)->isEnabled());
        w.act("transition.push")->trigger();
        QCOMPARE(listed(), (QList<QAction *>{w.act("transition.dir.l"), w.act("transition.dir.r"), w.act("transition.dir.u"), w.act("transition.dir.d")}));
        QVERIFY(w.act("transition.dir.l")->isChecked());
        QVERIFY(!w.act("transition.dir.u")->isChecked());
        w.act("transition.zoom")->trigger();
        QCOMPARE(listed(), (QList<QAction *>{w.act("transition.dir.out"), w.act("transition.dir.in")}));
        QVERIFY(w.act("transition.dir.in")->isChecked());
        QVERIFY(!w.act("transition.dir.l")->isEnabled());   // a direction means nothing to zoom
        w.act("transition.fade")->trigger();
        QCOMPARE(listed().size(), 1);
    }

    // Preview plays the slide's transition over the slide area and puts
    // itself away when it is done.
    void windowTransitionPreview()
    {
        DeckWindow w;
        w.newSlide();
        QVERIFY(!w.act("transition.preview")->isEnabled());   // no transition to see
        w.act("transition.wipe")->trigger();
        QVERIFY(w.act("transition.preview")->isEnabled());
        w.ribbon()->findChild<QDoubleSpinBox *>("transitionDuration")->setValue(0.2);
        w.act("transition.preview")->trigger();
        auto *preview = w.findChild<TransitionPreview *>(QStringLiteral("transitionPreview"));
        QVERIFY(preview);
        QVERIFY(preview->isPlaying());
        QVERIFY(!preview->grab().isNull());
        QTRY_VERIFY(!preview->isPlaying());
        QVERIFY(preview->isHidden());
    }

    // The Slide Show tab's Use Presenter View is on until turned off, and is
    // remembered.
    void windowUsePresenterViewCheckBox()
    {
        DeckWindow w;
        QAction *use = w.act("show.usePresenterView");
        QVERIFY(use);
        QVERIFY(use->isCheckable());
        QVERIFY(use->isChecked());
        QVERIFY(SlideShow::usePresenterView());
        use->trigger();
        QVERIFY(!use->isChecked());
        QVERIFY(!SlideShow::usePresenterView());
        auto *box = w.ribbon()->findChild<QCheckBox *>("usePresenterView");
        QVERIFY(box);
        QVERIFY(!box->isChecked());
        box->click();
        QVERIFY(use->isChecked());
        QVERIFY(SlideShow::usePresenterView());
    }

    // Slides that have a transition are marked in the thumbnails.
    void slidesPaneMarksTransitions()
    {
        DeckWindow w;
        w.newSlide();
        Editor *ed = w.editor();
        auto starPixels = [&] {
            const QImage img = w.slidesPane()->viewport()->grab().toImage();
            int n = 0;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (img.pixelColor(x, y) == QColor(0xB6, 0x42, 0x0F)) ++n;
            return n;
        };
        w.slidesPane()->resize(260, 400);
        w.slidesPane()->refresh();
        const int plain = starPixels();
        w.act("transition.fade")->trigger();
        w.slidesPane()->refresh();
        QVERIFY(starPixels() > plain);
        ed->undo();
        w.slidesPane()->refresh();
        QCOMPARE(starPixels(), plain);
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

    // A slide's page props "timingXml" and the shapes it animates need a
    // number each (the file's): the slide's title is 2 and its body 3.
    static void numberShowShapes(Page &slide)
    {
        slide.items[0]->sourceId = 2;
        slide.items[1]->sourceId = 3;
    }

    // A slide painted with the page's hooks, or without: a picture the size of the slide.
    static QImage paintedSlide(Document *d, LayoutCache *cache, int slide, const std::function<void(PaintContext &)> &hook = nullptr)
    {
        QImage img(QSize(960, 540), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        PaintContext ctx;
        ctx.doc = d;
        ctx.cache = cache;
        if (hook) hook(ctx);
        Renderer::paintPage(&p, ctx, slide);
        p.end();
        return img;
    }

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

    // ---- animations: slide timings as the presentation program writes them ----

    // XML written with ' where an attribute's quotes go, so that it reads without escapes.
    static QString animXml(const char *xml) { return QString::fromLatin1(xml).replace(QLatin1Char('\''), QLatin1Char('"')); }

    static int animId() { static int n = 10; return ++n; }

    static QString animTarget(int spid, int paragraph = -1)
    {
        if (paragraph < 0) return animXml("<p:tgtEl><p:spTgt spid='%1'/></p:tgtEl>").arg(spid);
        return animXml("<p:tgtEl><p:spTgt spid='%1'><p:txEl><p:pRg st='%2' end='%2'/></p:txEl></p:spTgt></p:tgtEl>").arg(spid).arg(paragraph);
    }

    // What an entrance starts with and an exit ends with: the shape is shown or hidden at `delay` ms.
    static QString animShow(int spid, bool visible, int delay = 0, int paragraph = -1)
    {
        return animXml("<p:set><p:cBhvr><p:cTn id='%1' dur='1' fill='hold'><p:stCondLst><p:cond delay='%2'/></p:stCondLst></p:cTn>%3<p:attrNameLst><p:attrName>style.visibility</p:attrName></p:attrNameLst></p:cBhvr><p:to><p:strVal val='%4'/></p:to></p:set>")
            .arg(animId()).arg(delay).arg(animTarget(spid, paragraph), visible ? QStringLiteral("visible") : QStringLiteral("hidden"));
    }

    static QString animFilter(int spid, const char *filter, bool entering, int ms, int paragraph = -1)
    {
        return animXml("<p:animEffect transition='%1' filter='%2'><p:cBhvr><p:cTn id='%3' dur='%4'/>%5</p:cBhvr></p:animEffect>")
            .arg(entering ? QStringLiteral("in") : QStringLiteral("out"), QString::fromLatin1(filter)).arg(animId()).arg(ms).arg(animTarget(spid, paragraph));
    }

    // One attribute animated from a value or formula to another.
    static QString animValues(int spid, const char *attribute, const char *from, const char *to, int ms)
    {
        return animXml("<p:anim calcmode='lin' valueType='num'><p:cBhvr additive='base'><p:cTn id='%1' dur='%2' fill='hold'/>%3<p:attrNameLst><p:attrName>%4</p:attrName></p:attrNameLst></p:cBhvr><p:tavLst><p:tav tm='0'><p:val><p:strVal val='%5'/></p:val></p:tav><p:tav tm='100000'><p:val><p:strVal val='%6'/></p:val></p:tav></p:tavLst></p:anim>")
            .arg(animId()).arg(ms).arg(animTarget(spid), QString::fromLatin1(attribute), QString::fromLatin1(from), QString::fromLatin1(to));
    }

    // An effect: a node of the step's tree with a preset and its behaviors.
    static QString animNode(const char *cls, int preset, int subtype, const char *nodeType, int delay, const QStringList &behaviors)
    {
        return animXml("<p:par><p:cTn id='%1' presetID='%2' presetClass='%3' presetSubtype='%4' fill='hold' nodeType='%5'><p:stCondLst><p:cond delay='%6'/></p:stCondLst><p:childTnLst>%7</p:childTnLst></p:cTn></p:par>")
            .arg(animId()).arg(preset).arg(QString::fromLatin1(cls)).arg(subtype).arg(QString::fromLatin1(nodeType)).arg(delay).arg(behaviors.join(QString()));
    }

    static QString animFadeIn(int spid, int ms, const char *nodeType = "clickEffect", int delay = 0, int paragraph = -1)
    {
        return animNode("entr", 10, 0, nodeType, delay, {animShow(spid, true, 0, paragraph), animFilter(spid, "fade", true, ms, paragraph)});
    }

    static QString animAppear(int spid, const char *nodeType = "clickEffect", int delay = 0)
    {
        return animNode("entr", 1, 0, nodeType, delay, {animShow(spid, true)});
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

    static QString animGroup(int delay, const QStringList &effects)
    {
        return animXml("<p:par><p:cTn id='%1' fill='hold'><p:stCondLst><p:cond delay='%2'/></p:stCondLst><p:childTnLst>%3</p:childTnLst></p:cTn></p:par>")
            .arg(animId()).arg(delay).arg(effects.join(QString()));
    }

    // A click step; one that starts by itself when the slide is shown has the main sequence's start as a second condition.
    static QString animStep(const QStringList &groups, bool automatic = false)
    {
        return animXml("<p:par><p:cTn id='%1' fill='hold'><p:stCondLst><p:cond delay='indefinite'/>%2</p:stCondLst><p:childTnLst>%3</p:childTnLst></p:cTn></p:par>")
            .arg(animId())
            .arg(automatic ? animXml("<p:cond evt='onBegin' delay='0'><p:tn val='2'/></p:cond>") : QString(), groups.join(QString()));
    }

    // Each effect in a step of its own.
    static QString animSteps(const QStringList &effects)
    {
        QString out;
        for (const QString &e : effects) out += animStep({animGroup(0, {e})});
        return out;
    }

    static QString animTiming(const QString &steps, const QString &after = QString())
    {
        return animXml("<p:timing xmlns:p='http://schemas.openxmlformats.org/presentationml/2006/main'><p:tnLst><p:par><p:cTn id='1' dur='indefinite' restart='never' nodeType='tmRoot'><p:childTnLst><p:seq concurrent='1' nextAc='seek'><p:cTn id='2' dur='indefinite' nodeType='mainSeq'><p:childTnLst>")
            + steps
            + animXml("</p:childTnLst></p:cTn><p:prevCondLst><p:cond evt='onPrev' delay='0'><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:prevCondLst><p:nextCondLst><p:cond evt='onNext' delay='0'><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:nextCondLst></p:seq>")
            + after + QStringLiteral("</p:childTnLst></p:cTn></p:par></p:tnLst></p:timing>");
    }

    // Where the shapes the samples aim at are, on a 16:9 slide.
    static deck::Geometry animGeometry()
    {
        deck::Geometry g;
        g.slide = QSizeF(960, 540);
        g.shapes.insert(4, QRectF(100, 200, 200, 100));   // its middle is at 200, 250
        g.shapes.insert(5, QRectF(500, 100, 100, 100));
        g.shapes.insert(6, QRectF(700, 300, 100, 100));
        return g;
    }

    static bool animNear(double a, double b) { return std::abs(a - b) < 1e-6; }

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
        const deck::Timeline wheel = deck::readTimeline(animTiming(animSteps({animNode("entr", 21, 1, "clickEffect", 0, {animShow(4, true), animFilter(4, "wheel(1)", true, 1000)})})));
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

    // The shapes' places are found by the number the file gave, in groups too.
    void animationGeometryFindsShapesByNumber()
    {
        auto d = showDeck(1);
        Page &slide = *d->pages[0];
        numberShowShapes(slide);
        auto inner = std::make_shared<ShapeItem>();
        inner->rect = QRectF(300, 380, 120, 80);
        inner->sourceId = 9;
        auto group = std::make_shared<GroupItem>();
        group->children.push_back(inner);
        group->syncRect();
        group->sourceId = 8;
        slide.items.push_back(group);
        const deck::Geometry g = deck::geometryOf(*d, slide);
        QCOMPARE(g.slide, QSizeF(960, 540));
        QCOMPARE(g.shapes.value(2), slide.items[0]->rect);
        QCOMPARE(g.shapes.value(3), slide.items[1]->rect);
        QCOMPARE(g.shapes.value(9), QRectF(300, 380, 120, 80));
        QCOMPARE(g.shapes.value(8), QRectF(300, 380, 120, 80));
        QVERIFY(!g.shapes.contains(0));
    }

    // The slide painted with the states: hidden, moved, turned, scaled, faded,
    // and wiped shapes come out as drawing them so would.
    void animationPaintsTheStates()
    {
        auto d = showDeck(1);
        Page &slide = *d->pages[0];
        numberShowShapes(slide);
        auto box = std::make_shared<ShapeItem>();
        box->rect = QRectF(600, 380, 120, 80);   // its middle is at 660, 420
        box->fill = Fill::solid(ColorRef::rgb(QColor(200, 60, 40)));
        box->sourceId = 7;
        slide.items.push_back(box);
        LayoutCache cache;
        auto paint = [&](const deck::States &states) {
            return paintedSlide(d.get(), &cache, 0, [&](PaintContext &ctx) { deck::animate(ctx, *d, states); });
        };
        const QImage plain = paintedSlide(d.get(), &cache, 0);
        QCOMPARE(paint({}), plain);   // nothing to change
        const QColor fill(200, 60, 40);
        QCOMPARE(plain.pixelColor(660, 420), fill);
        // Hidden: as if it were not there.
        deck::States hidden;
        hidden[deck::Target{7}].visible = false;
        const QImage hid = paint(hidden);
        QVERIFY(hid != plain);
        QCOMPARE(hid.pixelColor(660, 420), QColor(Qt::white));
        slide.items.pop_back();
        QCOMPARE(hid, paintedSlide(d.get(), &cache, 0));
        slide.items.push_back(box);
        // Moved 40 points: as if it were put there.
        deck::States moved;
        moved[deck::Target{7}].offset = QPointF(40, -20);
        const QImage mv = paint(moved);
        box->rect.translate(40, -20);
        QCOMPARE(mv, paintedSlide(d.get(), &cache, 0));
        box->rect.translate(-40, 20);
        // Turned a quarter about its middle: 80 wide and 120 high.
        deck::States turned;
        turned[deck::Target{7}].rotation = 90;
        const QImage tn = paint(turned);
        QCOMPARE(tn.pixelColor(660, 365), fill);                        // above the old top, inside the new
        QCOMPARE(tn.pixelColor(660 - 55, 420), QColor(Qt::white));      // inside the old left, outside the new
        // Scaled to twice its size about its middle.
        deck::States scaled;
        scaled[deck::Target{7}].scaleX = 2;
        scaled[deck::Target{7}].scaleY = 2;
        const QImage sc = paint(scaled);
        QCOMPARE(sc.pixelColor(660 + 110, 420 + 70), fill);
        // Half see-through: the fill half way to white.
        deck::States faded;
        faded[deck::Target{7}].opacity = 0.5;
        const QImage fd = paint(faded);
        const QColor mid = fd.pixelColor(660, 420);
        QVERIFY(std::abs(mid.red() - 227) <= 2 && std::abs(mid.green() - 157) <= 2 && std::abs(mid.blue() - 147) <= 2);
        QCOMPARE(fd.pixelColor(300, 300), plain.pixelColor(300, 300));
        // Wiped: the left half.
        deck::States wiped;
        wiped[deck::Target{7}].clip = QRectF(0, 0, 0.5, 1);
        const QImage wp = paint(wiped);
        QCOMPARE(wp.pixelColor(630, 420), fill);
        QCOMPARE(wp.pixelColor(690, 420), QColor(Qt::white));
        // Turned, wiped, and faded at once: the top half of the shape, which turned is its right half, half see-through.
        deck::States all;
        all[deck::Target{7}].rotation = 90;
        all[deck::Target{7}].clip = QRectF(0, 0, 1, 0.5);
        all[deck::Target{7}].opacity = 0.5;
        const QImage al = paint(all);
        QVERIFY(al.pixelColor(680, 420) != QColor(Qt::white));
        QCOMPARE(al.pixelColor(640, 420), QColor(Qt::white));
    }

    // A paragraph that is a target is drawn on its own: hidden, it is gone and
    // the lines around it stay where they are; moved, only its lines move.
    void animationPaintsParagraphs()
    {
        auto d = showDeck(1);
        Page &slide = *d->pages[0];
        numberShowShapes(slide);
        auto *body = static_cast<TextItem *>(slide.items[1].get());
        d->storyDoc(body->storyId)->setPlainText(QStringLiteral("First paragraph\nSecond paragraph\nThird paragraph"));
        LayoutCache cache;
        auto paint = [&](const deck::States &states) {
            return paintedSlide(d.get(), &cache, 0, [&](PaintContext &ctx) { deck::animate(ctx, *d, states); });
        };
        const QImage plain = paintedSlide(d.get(), &cache, 0);
        // The lines of paragraph 1, in the page's coordinates.
        const LayoutCache::FrameLayout fl = cache.textFrame(*d, *body, 1, RenderOptions());
        QVERIFY(fl.layout);
        const QTextBlock second = d->storyDoc(body->storyId)->findBlockByNumber(1);
        QRectF band;
        for (const QRectF &r : fl.layout->rangeRects(fl.frame, second.position(), second.position() + second.length())) band = band.united(r);
        band.translate(body->rect.topLeft());
        QVERIFY(band.isValid());
        auto changedRows = [&](const QImage &a, const QImage &b) {
            int top = -1, bottom = -1;
            for (int y = 0; y < a.height(); ++y)
                for (int x = 0; x < a.width(); ++x)
                    if (a.pixel(x, y) != b.pixel(x, y)) {
                        if (top < 0) top = y;
                        bottom = y;
                        break;
                    }
            return QPair<int, int>(top, bottom);
        };
        deck::States hidden;
        hidden[deck::Target{3, 1, 1}].visible = false;
        const QPair<int, int> rows = changedRows(plain, paint(hidden));
        QVERIFY(rows.first >= 0);                       // something went
        QVERIFY(rows.first >= band.top() - 2 && rows.second <= band.bottom() + 2);   // only its lines
        // Shown, nothing differs.
        deck::States shown;
        shown[deck::Target{3, 1, 1}];
        QCOMPARE(paint(shown), plain);
        // Moved right: its lines are where they were, 300 points over.
        deck::States moved;
        moved[deck::Target{3, 1, 1}].offset = QPointF(300, 0);
        const QImage mv = paint(moved);
        const QPair<int, int> movedRows = changedRows(plain, mv);
        QVERIFY(movedRows.first >= band.top() - 2 && movedRows.second <= band.bottom() + 2);
        auto inkLeft = [&](const QImage &img) {
            for (int x = 0; x < img.width(); ++x)
                for (int y = int(band.top()); y < int(band.bottom()); ++y)
                    if (img.pixel(x, y) != QColor(Qt::white).rgba()) return x;
            return -1;
        };
        QVERIFY(inkLeft(plain) >= 0);
        QCOMPARE(inkLeft(mv), inkLeft(plain) + 300);
    }

    // ---- the show plays them ----

    // Slide 1: the title fades in at the first click, the body appears at the second.
    static std::unique_ptr<Document> animatedDeck(int fadeMs)
    {
        auto d = showDeck(3);
        numberShowShapes(*d->pages[0]);
        d->storyDoc(static_cast<TextItem *>(d->pages[0]->items[1].get())->storyId)->setPlainText(QStringLiteral("The body of the slide"));
        d->pages[0]->props[QStringLiteral("timingXml")] = animTiming(animSteps({animFadeIn(2, fadeMs), animAppear(3)}));
        return d;
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

    // The button on the File screen with this text, if it is showing.
    static QAbstractButton *screenButton(QWidget *screen, const QString &text)
    {
        for (QAbstractButton *b : screen->findChildren<QAbstractButton *>())
            if (b->isVisibleTo(screen) && b->text() == text) return b;
        return nullptr;
    }

    // The File button opens the File screen over the window, with the
    // commands in its sidebar; Escape and the back button return to the slides.
    void windowFileScreenOpens()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        auto *screen = w.findChild<FileScreen *>();
        QVERIFY(screen);
        QVERIFY(!screen->isVisible());
        static_cast<QAbstractButton *>(w.ribbon()->fileButton())->click();
        QVERIFY(screen->isVisible());
        QCOMPARE(screen->geometry(), w.rect());
        auto *side = screen->findChild<QFrame *>(QStringLiteral("jpSide"));
        QVERIFY(side);
        const QList<QAbstractButton *> buttons = side->findChildren<QAbstractButton *>(QString(), Qt::FindDirectChildrenOnly);
        QStringList entries;
        for (QAbstractButton *b : buttons) entries << b->text();
        const QStringList want = {QStringLiteral("Back to presentation"), QStringLiteral("New"), QStringLiteral("Open"), QStringLiteral("Save"), QStringLiteral("Save As"),
                                  QStringLiteral("Print"), QStringLiteral("Export"), QStringLiteral("Close"), QStringLiteral("Options"), QStringLiteral("About")};
        QCOMPARE(entries, want);
        QTest::keyClick(&w, Qt::Key_Escape);
        QVERIFY(!screen->isVisible());
        w.showFileScreen(QStringLiteral("about"));
        QVERIFY(screen->isVisible());
        QCOMPARE(screen->currentPage(), QStringLiteral("about"));
        buttons.constFirst()->click();   // back
        QVERIFY(!screen->isVisible());
    }

    // Alt and F, the KeyTips' File letter, open it too.
    void windowFileScreenOpensWithAltF()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        auto *screen = w.findChild<FileScreen *>();
        QVERIFY(screen && !screen->isVisible());
        QTest::keyPress(&w, Qt::Key_Alt);
        QTest::keyClick(&w, Qt::Key_F, Qt::AltModifier);
        QTest::keyRelease(&w, Qt::Key_Alt);
        QVERIFY(screen->isVisible());
        QTest::keyClick(&w, Qt::Key_Escape);
        QVERIFY(!screen->isVisible());
    }

    // New: the blank presentation takes the place of one nobody has touched,
    // and opens in a window of its own beside one with work in it.
    void windowFileScreenNew()
    {
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        Editor *ed = w.editor();
        const int windows = int(AppWindow::windows().size());
        QSignalSpy replaced(ed, &Editor::documentReplaced);
        w.showFileScreen(QStringLiteral("new"));
        auto *screen = w.findChild<FileScreen *>();
        QVERIFY(screen);
        QAbstractButton *tile = screenButton(screen, QStringLiteral("Blank Presentation"));
        QVERIFY(tile);
        tile->click();
        QCOMPARE(replaced.count(), 1);
        QCOMPARE(int(AppWindow::windows().size()), windows);
        QVERIFY(!screen->isVisible());
        w.newSlide();
        QVERIFY(ed->isModified());
        w.showFileScreen(QStringLiteral("new"));
        QVERIFY(screenButton(screen, QStringLiteral("Blank Presentation")));
        screenButton(screen, QStringLiteral("Blank Presentation"))->click();
        QCOMPARE(replaced.count(), 1);
        QCOMPARE(int(AppWindow::windows().size()), windows + 1);
        QCOMPARE(int(ed->doc()->pages.size()), 2);   // this one is as it was
        QVERIFY(!screen->isVisible());
        for (AppWindow *other : AppWindow::windows())
            if (other != &w) other->close();
        QTRY_COMPARE(int(AppWindow::windows().size()), windows);
        ed->markSaved();
    }

    // Open lists the recent presentations, the pinned ones first and not
    // again below; choosing one opens it, and Browse is there for the rest.
    void windowFileScreenOpenListsRecentFiles()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString recent = dir.filePath(QStringLiteral("quarterly.pptx")), pinned = dir.filePath(QStringLiteral("pinned.pptx"));
        for (const QString &path : {recent, pinned}) QVERIFY(deck::savePresentationFile(*deck::newPresentation(), path));
        Settings::get().addRecentFile(recent);
        Settings::get().addRecentFile(pinned);
        Settings::get().setPinned(pinned, true);
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        w.showFileScreen(QStringLiteral("open"));
        auto *screen = w.findChild<FileScreen *>();
        QVERIFY(screen && screen->isVisible());
        QCOMPARE(screen->currentPage(), QStringLiteral("open"));
        QVERIFY(screenButton(screen, QStringLiteral("Browse")));
        QList<QListWidget *> lists;
        for (QListWidget *l : screen->findChildren<QListWidget *>())
            if (l->isVisibleTo(screen)) lists << l;
        std::sort(lists.begin(), lists.end(), [](QListWidget *a, QListWidget *b) { return a->mapTo(a->window(), QPoint()).y() < b->mapTo(b->window(), QPoint()).y(); });
        QCOMPARE(lists.size(), 2);
        QCOMPARE(lists[0]->count(), 1);
        QCOMPARE(lists[0]->item(0)->data(Qt::UserRole).toString(), pinned);
        QVERIFY(lists[0]->item(0)->text().startsWith(QLatin1String("pinned.pptx")));
        QCOMPARE(lists[1]->count(), 1);
        QCOMPARE(lists[1]->item(0)->data(Qt::UserRole).toString(), recent);
        QVERIFY(lists[1]->item(0)->text().startsWith(QLatin1String("quarterly.pptx")));
        // This window holds nothing, so the presentation opens in it.
        Q_EMIT lists[1]->itemActivated(lists[1]->item(0));
        QCOMPARE(w.editor()->filePath(), recent);
        QVERIFY(!screen->isVisible());
        QCOMPARE(Settings::get().recentFiles().value(0), recent);
        Settings::get().setPinned(pinned, false);
        Settings::get().removeRecentFile(recent);
        Settings::get().removeRecentFile(pinned);
    }

    // Save and Save As are commands in the sidebar: they save, and the screen
    // goes away once the file is written.
    void windowFileScreenSaves()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        const QString path = dir.filePath(QStringLiteral("saved.pptx"));
        int asked = 0;
        auto *screen = w.findChild<FileScreen *>();
        w.showFileScreen(QStringLiteral("open"));
        // No name chosen: nothing is written and the screen stays.
        w.setSavePathChooser([&](const QString &, const QString &) {
            ++asked;
            return QString();
        });
        screenButton(screen, QStringLiteral("Save As"))->click();
        QCOMPARE(asked, 1);
        QVERIFY(screen->isVisible());
        QVERIFY(!QFileInfo::exists(path));
        w.setSavePathChooser([&](const QString &, const QString &) {
            ++asked;
            return path;
        });
        screenButton(screen, QStringLiteral("Save"))->click();   // never saved: asks where
        QCOMPARE(asked, 2);
        QVERIFY(QFileInfo::exists(path));
        QVERIFY(!screen->isVisible());
        QCOMPARE(w.editor()->filePath(), path);
        // Close leaves the screen and closes the window.
        w.showFileScreen(QStringLiteral("open"));
        screenButton(screen, QStringLiteral("Close"))->click();
        QVERIFY(!w.isVisible());
        QVERIFY(!screen->isVisible());
    }

    // Print: the options beside a preview of the pages that will print, with
    // arrows to page through them; the count follows the options.
    void windowFilePrintPagePreviewsTheOutput()
    {
        DeckWindow w;
        w.resize(1300, 850);
        w.show();
        w.editor()->setDocument(showDeck(7));
        w.showFileScreen(QStringLiteral("print"));
        auto *screen = w.findChild<FileScreen *>();
        QVERIFY(screen && screen->isVisible());
        QCOMPARE(screen->currentPage(), QStringLiteral("print"));
        auto *what = screen->findChild<QComboBox *>(QStringLiteral("printWhat"));
        auto *pages = screen->findChild<QLabel *>(QStringLiteral("printPage"));
        auto *preview = screen->findChild<QLabel *>(QStringLiteral("printPreview"));
        QVERIFY(what && pages && preview);
        QVERIFY(screenButton(screen, QStringLiteral("Print")));
        QCOMPARE(pages->text(), QStringLiteral("Page 1 of 7"));
        QTRY_VERIFY(!preview->pixmap().isNull());
        what->setCurrentIndex(what->findData(int(deck::PrintLayout::Handout6)));
        QCOMPARE(pages->text(), QStringLiteral("Page 1 of 2"));
        auto *next = screen->findChild<QAbstractButton *>(QStringLiteral("printNext"));
        auto *previous = screen->findChild<QAbstractButton *>(QStringLiteral("printPrevious"));
        QVERIFY(next && previous);
        QVERIFY(!previous->isEnabled());
        next->click();
        QCOMPARE(pages->text(), QStringLiteral("Page 2 of 2"));
        QVERIFY(!next->isEnabled());
        previous->click();
        QCOMPARE(pages->text(), QStringLiteral("Page 1 of 2"));
        // The slides asked for are the pages.
        what->setCurrentIndex(what->findData(int(deck::PrintLayout::Slides)));
        screen->findChild<QRadioButton *>(QStringLiteral("printRangeSlides"))->setChecked(true);
        screen->findChild<QLineEdit *>(QStringLiteral("printRangeText"))->setText(QStringLiteral("2-4"));
        QCOMPARE(pages->text(), QStringLiteral("Page 1 of 3"));
        w.editor()->markSaved();
    }

    // Export: a button for each way out, each running the command the window
    // has, and the screen goes away once the file is written.
    void windowFileScreenExports()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        w.newSlide();
        const QString png = dir.filePath(QStringLiteral("slide.png")), pdf = dir.filePath(QStringLiteral("talk.pdf"));
        QString path = png;
        w.setSavePathChooser([&](const QString &, const QString &) { return path; });
        w.showFileScreen(QStringLiteral("export"));
        auto *screen = w.findChild<FileScreen *>();
        QVERIFY(screen && screen->isVisible());
        QCOMPARE(screen->currentPage(), QStringLiteral("export"));
        QVERIFY(screenButton(screen, QStringLiteral("Export to PDF")));
        QVERIFY(screenButton(screen, QStringLiteral("Export Slide as Picture")));
        screenButton(screen, QStringLiteral("Export Slide as Picture"))->click();
        QCOMPARE(QImage(png).size(), QSize(1920, 1080));
        QVERIFY(!screen->isVisible());
        w.showFileScreen(QStringLiteral("export"));
        path = pdf;
        answerOutputDialog();
        screenButton(screen, QStringLiteral("Export to PDF"))->click();
        QCOMPARE(pdfPageCount(fileBytes(pdf)), 2);
        QVERIFY(!screen->isVisible());
        // Turned away at the first step, the screen stays.
        QVERIFY(QFile::remove(pdf));
        w.showFileScreen(QStringLiteral("export"));
        answerOutputDialog(nullptr, QDialogButtonBox::Cancel);
        screenButton(screen, QStringLiteral("Export to PDF"))->click();
        QVERIFY(!QFileInfo::exists(pdf));
        QVERIFY(screen->isVisible());
        w.editor()->markSaved();
    }

    // About: the suite's apps with where each stands, this one's version, its
    // links, and its license.
    void windowFileScreenAbout()
    {
        DeckWindow w;
        w.resize(1200, 900);
        w.show();
        w.showFileScreen(QStringLiteral("about"));
        auto *screen = w.findChild<FileScreen *>();
        QVERIFY(screen && screen->isVisible());
        const QStringList rows = {QStringLiteral("JeffPub, Publications, Available"), QStringLiteral("JeffDeck, Presentations, You're using it"),
                                  QStringLiteral("JeffWrite, Documents, Coming soon"), QStringLiteral("JeffCrunch, Spreadsheets, Coming soon"),
                                  QStringLiteral("JeffMail, Email and calendar, Coming soon"), QStringLiteral("JeffBase, Databases, Coming soon")};
        QStringList named;
        int comingSoon = 0;
        bool version = false, links = false, license = false;
        for (QWidget *c : screen->findChildren<QWidget *>()) {
            if (!c->isVisibleTo(screen)) continue;
            if (rows.contains(c->accessibleName())) named << c->accessibleName();
            if (auto *l = qobject_cast<QLabel *>(c)) {
                if (l->text() == QLatin1String("Coming soon")) ++comingSoon;
                if (l->text().startsWith(QLatin1String("Version ")) && l->text().contains(QLatin1String(JP_VERSION)) && l->text().contains(QLatin1String("Qt"))) version = true;
                if (l->text().contains(QLatin1String("github.com/JeffOffice/jeffoffice/releases"))) links = true;
                if (l->text().contains(QLatin1String("GNU General Public License"))) license = true;
            }
        }
        named.sort();
        QStringList sorted = rows;
        sorted.sort();
        QCOMPARE(named, sorted);
        QCOMPARE(comingSoon, 4);
        QVERIFY(version);
        QVERIFY(links);
        QVERIFY(license);
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

    // The Home tab's Font and Paragraph groups open the shared dialogs from
    // their launchers, and the Font dialog changes the text selected in a
    // placeholder.
    void fontAndParagraphLaunchers()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        auto launcher = [&](const QString &group) -> QToolButton * {
            for (RibbonGroup *g : w.ribbon()->tab(QStringLiteral("Home"))->groups())
                if (g->title() == group) return g->launcher();
            return nullptr;
        };
        QToolButton *font = launcher(QStringLiteral("Font")), *paragraph = launcher(QStringLiteral("Paragraph"));
        QVERIFY(font && paragraph);
        QVERIFY(!keytip(font).isEmpty() && !keytip(paragraph).isEmpty());
        QString title;
        QTimer::singleShot(0, [&] {
            if (auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget())) {
                title = d->windowTitle();
                d->reject();
            }
        });
        font->click();
        QCOMPARE(title, QStringLiteral("Font"));
        title.clear();
        QTimer::singleShot(0, [&] {
            if (auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget())) {
                title = d->windowTitle();
                d->reject();
            }
        });
        paragraph->click();
        QCOMPARE(title, QStringLiteral("Paragraph"));
        // Font, on the selected word: its size.
        QTextDocument *story = editTitle(w, QStringLiteral("Hello world"), 0, 5);
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            d->findChild<QDoubleSpinBox *>()->setValue(40);
            d->accept();
        });
        font->click();
        QTextCursor c(story);
        c.setPosition(2);
        QCOMPARE(c.charFormat().fontPointSize(), 40.0);
        c.setPosition(9);
        QVERIFY(c.charFormat().fontPointSize() != 40.0);
    }

    // Review > Spelling (F7) goes through the slides to a misspelled word and
    // selects it; Thesaurus is Shift+F7.
    void spellingChecksTheSlides()
    {
        DeckWindow w;
        QCOMPARE(w.act(QStringLiteral("review.spelling"))->shortcut(), QKeySequence(Qt::Key_F7));
        QCOMPARE(w.act(QStringLiteral("review.thesaurus"))->shortcut(), QKeySequence(Qt::SHIFT | Qt::Key_F7));
        Editor *ed = w.editor();
        auto *title = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get());
        QTextCursor(ed->doc()->storyDoc(title->storyId)).insertText(QStringLiteral("A mispelled word"));
        QString shown, selected;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            for (QLabel *l : d->findChildren<QLabel *>())
                if (l->text().startsWith(QLatin1String("<b>"))) shown = l->text();
            selected = ed->cursor().selectedText();
            d->reject();
        });
        w.act(QStringLiteral("review.spelling"))->trigger();
        QVERIFY2(shown.contains(QStringLiteral("mispelled")), qPrintable(shown));
        QCOMPARE(selected, QStringLiteral("mispelled"));
        QVERIFY(ed->isEditingText());
    }

    // Word Count counts the words on the slides, not the prompts of the layouts.
    void wordCountCountsTheSlidesWords()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        QTextCursor(ed->doc()->storyDoc(static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get())->storyId)).insertText(QStringLiteral("One two three"));
        QTextCursor(ed->doc()->storyDoc(static_cast<TextItem *>(ed->doc()->pages[0]->items[1].get())->storyId)).insertText(QStringLiteral("Four five"));
        QString shown;
        QTimer::singleShot(0, [&] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                shown = box->text();
                box->close();
            }
        });
        w.act(QStringLiteral("review.wordCount"))->trigger();
        QVERIFY2(shown.contains(QStringLiteral("Words: 5")), qPrintable(shown));
    }

    // Insert > Symbol puts the character chosen in the dialog into the
    // placeholder being typed in; Date & Time puts in the format chosen.
    void symbolAndDateGoIntoThePlaceholderBeingEdited()
    {
        DeckWindow w;
        QTextDocument *story = editTitle(w, QStringLiteral("ab"), 1, 1);
        QString title;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            title = d->windowTitle();
            QTableWidget *grid = nullptr;
            for (QTableWidget *t : d->findChildren<QTableWidget *>())
                if (t->rowCount() > 1) grid = t;
            QPushButton *insert = nullptr;
            for (QPushButton *b : d->findChildren<QPushButton *>())
                if (b->text() == QLatin1String("Insert")) insert = b;
            if (grid && insert) {
                grid->setCurrentCell(2, 1);   // Basic Latin from U+0020: 33 on is "A"
                insert->click();
            }
            d->accept();
        });
        w.act(QStringLiteral("insert.symbol"))->trigger();
        QCOMPARE(title, QStringLiteral("Symbol"));
        QCOMPARE(story->toPlainText(), QStringLiteral("aAb"));
        // Date & Time: the first format in the list.
        QString shown;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            if (auto *list = d->findChild<QListWidget *>()) shown = list->currentItem()->text();
            d->accept();
        });
        w.act(QStringLiteral("insert.dateTime"))->trigger();
        QVERIFY(!shown.isEmpty());
        QCOMPARE(story->toPlainText(), QStringLiteral("aA") + shown + QStringLiteral("b"));
    }

    // Insert > Link (Ctrl+K) makes the selected text a link to the address typed.
    void hyperlinkLinksTheSelectedText()
    {
        DeckWindow w;
        QCOMPARE(w.act(QStringLiteral("insert.link"))->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_K));
        QTextDocument *story = editTitle(w, QStringLiteral("Visit our site"), 6, 9);
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            for (QLabel *l : d->findChildren<QLabel *>())
                if (l->text() == QLatin1String("Address:") && l->buddy()) static_cast<QLineEdit *>(l->buddy())->setText(QStringLiteral("example.com"));
            d->accept();
        });
        w.act(QStringLiteral("insert.link"))->trigger();
        QTextCursor c(story);
        c.setPosition(7);
        QCOMPARE(c.charFormat().anchorHref(), QStringLiteral("https://example.com"));
        c.setPosition(2);
        QVERIFY(c.charFormat().anchorHref().isEmpty());
    }

    // The ribbon has a Review tab after Slide Show, with the proofing commands,
    // and the Insert tab has the symbol, link, and date commands, each KeyTip its own.
    void reviewTabAndInsertDialogs()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QStringList tabs;
        for (int i = 0; i < w.ribbon()->tabCount(); ++i) tabs << w.ribbon()->tabName(i);
        QCOMPARE(tabs.indexOf(QStringLiteral("Review")), tabs.indexOf(QStringLiteral("Slide Show")) + 1);
        for (const char *id : {"review.spelling", "review.thesaurus", "review.wordCount", "insert.symbol", "insert.link", "insert.dateTime", "fmt.fontDialog", "para.dialog", "fmt.spacingDialog",
                               "para.bulletsDialog", "para.numberingDialog", "file.options"})
            QVERIFY2(w.act(QString::fromLatin1(id)), id);
        for (const QString &name : {QStringLiteral("Home"), QStringLiteral("Insert"), QStringLiteral("Review")}) {
            QVERIFY2(w.ribbon()->tab(name), qPrintable(name));
            QStringList seen;
            for (const QWidget *c : w.ribbon()->tab(name)->findChildren<QWidget *>()) {
                const QString k = keytip(c);
                if (k.isEmpty()) continue;
                QVERIFY2(!seen.contains(k), qPrintable(name + ": two controls have the KeyTip " + k));
                seen << k;
            }
        }
        // Each is in its tab.
        auto has = [&](const QString &tab, const QString &id) {
            for (const QToolButton *b : w.ribbon()->tab(tab)->findChildren<QToolButton *>())
                if (b->defaultAction() == w.act(id)) return true;
            return false;
        };
        QVERIFY(has(QStringLiteral("Review"), QStringLiteral("review.spelling")));
        QVERIFY(has(QStringLiteral("Review"), QStringLiteral("review.thesaurus")));
        QVERIFY(has(QStringLiteral("Review"), QStringLiteral("review.wordCount")));
        QVERIFY(has(QStringLiteral("Insert"), QStringLiteral("insert.symbol")));
        QVERIFY(has(QStringLiteral("Insert"), QStringLiteral("insert.link")));
        QVERIFY(has(QStringLiteral("Insert"), QStringLiteral("insert.dateTime")));
    }

    // File > Options is the shared dialog: its General tab has the user name
    // and the app's own name in the update check, Proofing follows, and
    // JeffDeck's View page sets the ruler and snapping for every open window.
    void optionsHasTheGeneralTabAndTheViewPage()
    {
        DeckWindow w;
        QVERIFY(w.act(QStringLiteral("file.options")));
        QStringList tabNames;
        QString title, shown, userName, updates;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            title = d->windowTitle();
            if (auto *tabs = d->findChild<QTabWidget *>()) {
                shown = tabs->tabText(tabs->currentIndex());
                for (int i = 0; i < tabs->count(); ++i) tabNames << tabs->tabText(i);
            }
            for (QLabel *l : d->findChildren<QLabel *>())
                if (l->text() == QLatin1String("User name:") && l->buddy()) userName = static_cast<QLineEdit *>(l->buddy())->text();
            if (auto *box = d->findChild<QCheckBox *>(QStringLiteral("updates"))) updates = box->text();
            d->reject();
        });
        w.act(QStringLiteral("file.options"))->trigger();
        QCOMPARE(title, QStringLiteral("JeffDeck Options"));
        QCOMPARE(tabNames, (QStringList{"General", "Proofing", "View"}));
        QCOMPARE(shown, QStringLiteral("General"));
        QCOMPARE(userName, Settings::get().userName());
        QCOMPARE(updates, QStringLiteral("Check for updates when JeffDeck starts"));
        // The View page: the ruler and snapping, in this window now and in new ones.
        QVERIFY(!w.editor()->view.rulers);
        QVERIFY(w.editor()->view.snapObjects);
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            if (auto *box = d->findChild<QCheckBox *>(QStringLiteral("showRuler"))) box->setChecked(true);
            if (auto *box = d->findChild<QCheckBox *>(QStringLiteral("snapObjects"))) box->setChecked(false);
            d->accept();
        });
        w.act(QStringLiteral("file.options"))->trigger();
        QVERIFY(w.editor()->view.rulers);
        QVERIFY(!w.canvas()->hRuler()->isHidden());
        QVERIFY(!w.editor()->view.snapObjects);
        {
            DeckWindow another;
            QVERIFY(another.editor()->view.rulers);
            QVERIFY(!another.editor()->view.snapObjects);
        }
        Settings::get().setValue(QStringLiteral("view/rulers"), false);
        Settings::get().setValue(QStringLiteral("view/snapObjects"), true);
    }

private:
    // Types `text` in the first slide's title placeholder and leaves it being
    // typed in, with from..to selected when that is a range. Returns the story.
    static QTextDocument *editTitle(DeckWindow &w, const QString &text, int from, int to)
    {
        Editor *ed = w.editor();
        auto *title = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get());
        QTextCursor(ed->doc()->storyDoc(title->storyId)).insertText(text);
        ed->select(title->id);
        ed->beginTextEdit(title->id, from);
        if (to > from) {
            QTextCursor c = ed->cursor();
            c.setPosition(from);
            c.setPosition(to, QTextCursor::KeepAnchor);
            ed->setCursor(c);
        }
        return ed->doc()->storyDoc(title->storyId);
    }

    struct Rel {
        QString source, type, target;   // type: the last word of one of the standard's, or a whole URI
        bool external = false;
    };
    struct Part {
        QString type;
        QByteArray xml;
    };
    // A presentation file as parts and relationships, so a test can change
    // one before it is packed.
    struct Parts {
        QMap<QString, Part> data;
        QVector<Rel> rels;
    };

    static QString nsDecl()
    {
        return QStringLiteral(
            "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
            "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
            "xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"");
    }
    static const QString &relBase()
    {
        static const QString base = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        return base;
    }
    static QByteArray tinyPng()
    {
        QImage img(4, 4, QImage::Format_RGB32);
        img.fill(QColor(0xB6, 0x42, 0x0F));
        QByteArray bytes;
        QBuffer buf(&bytes);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "PNG");
        return bytes;
    }
    // A shape tree's start, and its end.
    static QString treeStart() { return QStringLiteral("<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>"); }
    static QString treeEnd() { return QStringLiteral("</p:spTree></p:cSld>"); }
    static QString placeholderShape(int id, const QString &name, const QString &ph, const QString &xfrm, const QString &body)
    {
        return QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"%2\"/><p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr><p:nvPr><p:ph %3/></p:nvPr></p:nvSpPr>"
                              "<p:spPr>%4</p:spPr><p:txBody><a:bodyPr/><a:lstStyle/>%5</p:txBody></p:sp>")
            .arg(id).arg(name, ph, xfrm, body);
    }
    static QString xfrm(qint64 x, qint64 y, qint64 cx, qint64 cy)
    {
        return QStringLiteral("<a:xfrm><a:off x=\"%1\" y=\"%2\"/><a:ext cx=\"%3\" cy=\"%4\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>").arg(x).arg(y).arg(cx).arg(cy);
    }
    static QString para(const QString &text)
    {
        return text.isEmpty() ? QStringLiteral("<a:p><a:endParaRPr lang=\"en-US\"/></a:p>") : QStringLiteral("<a:p><a:r><a:rPr lang=\"en-US\"/><a:t>%1</a:t></a:r></a:p>").arg(text);
    }
    static QByteArray themeXml()
    {
        QString colors;
        const char *const names[12] = {"dk1", "lt1", "dk2", "lt2", "accent1", "accent2", "accent3", "accent4", "accent5", "accent6", "hlink", "folHlink"};
        const char *const values[12] = {"111111", "FEFEFE", "224466", "EEEEDD", "B6420F", "13773A", "1D55C4", "9E1F63", "0A6C8C", "5E2CA5", "0000EE", "551A8B"};
        for (int i = 0; i < 12; ++i) colors += QStringLiteral("<a:%1><a:srgbClr val=\"%2\"/></a:%1>").arg(QLatin1String(names[i]), QLatin1String(values[i]));
        const QString fill = QStringLiteral("<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>");
        const QString line = QStringLiteral("<a:ln w=\"9525\">") + fill + QStringLiteral("</a:ln>");
        return QStringLiteral(
                   "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" name=\"Test Theme\">"
                   "<a:themeElements><a:clrScheme name=\"Test Colors\">%1</a:clrScheme>"
                   "<a:fontScheme name=\"Test Fonts\"><a:majorFont><a:latin typeface=\"Georgia\"/><a:ea typeface=\"\"/><a:cs typeface=\"\"/></a:majorFont>"
                   "<a:minorFont><a:latin typeface=\"Verdana\"/><a:ea typeface=\"\"/><a:cs typeface=\"\"/></a:minorFont></a:fontScheme>"
                   "<a:fmtScheme name=\"Test Formats\"><a:fillStyleLst>%2%2%2</a:fillStyleLst><a:lnStyleLst>%3%3%3</a:lnStyleLst>"
                   "<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>"
                   "<a:bgFillStyleLst>%2%2%2</a:bgFillStyleLst></a:fmtScheme></a:themeElements></a:theme>")
            .arg(colors, fill, line)
            .toUtf8();
    }
    static QString masterXml()
    {
        return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:sldMaster %1>").arg(nsDecl())
            + QStringLiteral("<p:cSld name=\"Test Master\"><p:bg><p:bgPr><a:solidFill><a:srgbClr val=\"FFFFFF\"/></a:solidFill><a:effectLst/></p:bgPr></p:bg><p:spTree>"
                             "<p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>")
            + placeholderShape(2, QStringLiteral("Title Placeholder 1"), QStringLiteral("type=\"title\""), xfrm(838200, 365125, 10515600, 1325563), para(QStringLiteral("Click to edit Master title style")))
            + placeholderShape(3, QStringLiteral("Text Placeholder 2"), QStringLiteral("type=\"body\" idx=\"1\""), xfrm(838200, 1825625, 10515600, 4351338), para(QStringLiteral("Click to edit Master text styles")))
            + placeholderShape(4, QStringLiteral("Date Placeholder 3"), QStringLiteral("type=\"dt\" sz=\"half\" idx=\"2\""), xfrm(838200, 6356350, 2743200, 365125), para(QStringLiteral("10/11/2026")))
            + QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"5\" name=\"Master Band\"/><p:cNvSpPr/><p:nvPr userDrawn=\"1\"/></p:nvSpPr><p:spPr>") + xfrm(0, 0, 12192000, 182880)
            + QStringLiteral("<a:solidFill><a:schemeClr val=\"accent1\"/></a:solidFill></p:spPr></p:sp>")
            + QStringLiteral("</p:spTree></p:cSld>"
                             "<p:clrMap bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/>"
                             "<p:sldLayoutIdLst><p:sldLayoutId id=\"2147483649\" r:id=\"rId1\"/><p:sldLayoutId id=\"2147483650\" r:id=\"rId2\"/></p:sldLayoutIdLst>"
                             "<p:txStyles><p:titleStyle><a:lvl1pPr algn=\"l\"><a:defRPr sz=\"4400\"><a:latin typeface=\"+mj-lt\"/></a:defRPr></a:lvl1pPr></p:titleStyle>"
                             "<p:bodyStyle><a:lvl1pPr marL=\"228600\" indent=\"-228600\"><a:buChar char=\"&#8226;\"/><a:defRPr sz=\"2800\"/></a:lvl1pPr></p:bodyStyle>"
                             "<p:otherStyle><a:lvl1pPr><a:defRPr sz=\"1800\"/></a:lvl1pPr></p:otherStyle></p:txStyles></p:sldMaster>");
    }
    static QString layoutXml(const QString &type, const QString &name, const QString &attrs, const QString &shapes)
    {
        return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:sldLayout %1 type=\"%2\" %3>").arg(nsDecl(), type, attrs)
            + QStringLiteral("<p:cSld name=\"%1\"><p:spTree><p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>").arg(name) + shapes
            + QStringLiteral("</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sldLayout>");
    }
    static QString slideXml(const QString &attrs, const QString &background, const QString &shapes, const QString &after = QString())
    {
        return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:sld %1 %2>").arg(nsDecl(), attrs)
            + QStringLiteral("<p:cSld>") + background + QStringLiteral("<p:spTree><p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>") + shapes
            + QStringLiteral("</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr>") + after + QStringLiteral("</p:sld>");
    }
    static QString pictureShape(int id, qint64 x, qint64 y)
    {
        return QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"%1\" name=\"Picture %1\" descr=\"A small square\"/><p:cNvPicPr><a:picLocks noChangeAspect=\"1\"/></p:cNvPicPr><p:nvPr/></p:nvPicPr>"
                              "<p:blipFill><a:blip r:embed=\"rId2\"/><a:stretch><a:fillRect/></a:stretch></p:blipFill><p:spPr>%2</p:spPr></p:pic>")
            .arg(id).arg(xfrm(x, y, 914400, 914400));
    }
    // A frame of a kind the reader doesn't know (a table by another name, as the reader reads tables), which is kept as it was.
    static QString unknownFrame(int id)
    {
        return QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"%1\" name=\"Table %1\"/><p:cNvGraphicFramePr><a:graphicFrameLocks noGrp=\"1\"/></p:cNvGraphicFramePr><p:nvPr/></p:nvGraphicFramePr>"
                              "<p:xfrm><a:off x=\"914400\" y=\"1828800\"/><a:ext cx=\"3657600\" cy=\"1371600\"/></p:xfrm>"
                              "<a:graphic><a:graphicData uri=\"urn:test:unknown-kind\"><a:tbl><a:tblPr/><a:tblGrid><a:gridCol w=\"3657600\"/></a:tblGrid>"
                              "<a:tr h=\"370840\"><a:tc><a:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:t>Cell &amp; &lt;1&gt;</a:t></a:r></a:p></a:txBody><a:tcPr/></a:tc></a:tr></a:tbl></a:graphicData></a:graphic></p:graphicFrame>")
            .arg(id);
    }
    // A chart the reader doesn't know, standing in a pair of alternatives
    // with a picture of it.
    static QString chartWithPicture(int id)
    {
        return QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\">"
                              "<mc:Choice xmlns:cx1=\"urn:test:chart\" Requires=\"cx1\"><p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"%1\" name=\"Chart %1\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                              "<p:xfrm><a:off x=\"1219200\" y=\"1219200\"/><a:ext cx=\"6096000\" cy=\"3048000\"/></p:xfrm><a:graphic><a:graphicData uri=\"urn:test:chart\"><cx1:chart r:id=\"rId9\"/></a:graphicData></a:graphic></p:graphicFrame></mc:Choice>"
                              "<mc:Fallback>%2</mc:Fallback></mc:AlternateContent>")
            .arg(id)
            .arg(pictureShape(id + 1, 1219200, 1219200));
    }

    // An embedded object with a picture of itself, in a pair of alternatives for programs that
    // know the other kind of drawing.
    static QString objectWithPicture(int id)
    {
        return QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"%1\" name=\"Object %1\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                              "<p:xfrm><a:off x=\"7315200\" y=\"1219200\"/><a:ext cx=\"1828800\" cy=\"1828800\"/></p:xfrm><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/presentationml/2006/ole\">"
                              "<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice xmlns:v=\"urn:test:drawing\" Requires=\"v\">"
                              "<p:oleObj spid=\"_x0000_s1040\" name=\"Object\" r:id=\"rId7\"><p:embed/></p:oleObj></mc:Choice>"
                              "<mc:Fallback><p:oleObj name=\"Object\" r:id=\"rId7\"><p:embed/>%2</p:oleObj></mc:Fallback></mc:AlternateContent></a:graphicData></a:graphic></p:graphicFrame>")
            .arg(id)
            .arg(pictureShape(id + 1, 7315200, 1219200));
    }

    // The sample presentation: 16:9, one master over two layouts (Title
    // Slide, Title and Content), and two slides.
    static Parts samplePptx()
    {
        const QString pres = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml");
        Parts p;
        p.data["/ppt/presentation.xml"] = {pres,
            (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:presentation %1>").arg(nsDecl())
             + QStringLiteral("<p:sldMasterIdLst><p:sldMasterId id=\"2147483648\" r:id=\"rId1\"/></p:sldMasterIdLst>"
                              "<p:notesMasterIdLst><p:notesMasterId r:id=\"rId4\"/></p:notesMasterIdLst>"
                              "<p:sldIdLst><p:sldId id=\"256\" r:id=\"rId2\"/><p:sldId id=\"257\" r:id=\"rId3\"/></p:sldIdLst>"
                              "<p:sldSz cx=\"12192000\" cy=\"6858000\"/><p:notesSz cx=\"6858000\" cy=\"9144000\"/>"
                              "<p:defaultTextStyle><a:defPPr><a:defRPr lang=\"en-US\"/></a:defPPr><a:lvl1pPr marL=\"0\" algn=\"l\"><a:defRPr sz=\"1800\"><a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill>"
                              "<a:latin typeface=\"+mn-lt\"/></a:defRPr></a:lvl1pPr></p:defaultTextStyle></p:presentation>")).toUtf8()};
        p.rels << Rel{"/ppt/presentation.xml", "slideMaster", "/ppt/slideMasters/slideMaster1.xml"}
               << Rel{"/ppt/presentation.xml", "slide", "/ppt/slides/slide1.xml"}
               << Rel{"/ppt/presentation.xml", "slide", "/ppt/slides/slide2.xml"}
               << Rel{"/ppt/presentation.xml", "notesMaster", "/ppt/notesMasters/notesMaster1.xml"};

        p.data["/ppt/theme/theme1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"), themeXml()};
        p.data["/ppt/slideMasters/slideMaster1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"), masterXml().toUtf8()};
        p.rels << Rel{"/ppt/slideMasters/slideMaster1.xml", "slideLayout", "/ppt/slideLayouts/slideLayout1.xml"}
               << Rel{"/ppt/slideMasters/slideMaster1.xml", "slideLayout", "/ppt/slideLayouts/slideLayout2.xml"}
               << Rel{"/ppt/slideMasters/slideMaster1.xml", "theme", "/ppt/theme/theme1.xml"};

        const QString layoutType = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml");
        p.data["/ppt/slideLayouts/slideLayout1.xml"] = {layoutType,
            layoutXml(QStringLiteral("title"), QStringLiteral("Title Slide"), QStringLiteral("showMasterSp=\"0\""),
                      placeholderShape(2, QStringLiteral("Title 1"), QStringLiteral("type=\"ctrTitle\""), xfrm(1524000, 1122363, 9144000, 2387600), para(QStringLiteral("Click to edit Master title style")))
                          + placeholderShape(3, QStringLiteral("Subtitle 2"), QStringLiteral("type=\"subTitle\" idx=\"1\""), xfrm(1524000, 3602038, 9144000, 1655762), para(QStringLiteral("Click to edit Master subtitle style"))))
                .toUtf8()};
        p.data["/ppt/slideLayouts/slideLayout2.xml"] = {layoutType,
            layoutXml(QStringLiteral("obj"), QStringLiteral("Title and Content"), QString(),
                      placeholderShape(2, QStringLiteral("Title 1"), QStringLiteral("type=\"title\""), QString(), para(QStringLiteral("Click to edit Master title style")))
                          + placeholderShape(3, QStringLiteral("Content Placeholder 2"), QStringLiteral("idx=\"1\""), xfrm(838200, 1825625, 5181600, 4351338), para(QStringLiteral("Click to edit Master text styles")))
                          + placeholderShape(4, QStringLiteral("Date Placeholder 3"), QStringLiteral("type=\"dt\" sz=\"half\" idx=\"10\""), QString(), para(QStringLiteral("10/11/2026"))))
                .toUtf8()};
        p.rels << Rel{"/ppt/slideLayouts/slideLayout1.xml", "slideMaster", "/ppt/slideMasters/slideMaster1.xml"}
               << Rel{"/ppt/slideLayouts/slideLayout2.xml", "slideMaster", "/ppt/slideMasters/slideMaster1.xml"};

        const QString slideType = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide+xml");
        const QString dateField = QStringLiteral("<a:p><a:fld id=\"{B5D1E7E0-0000-4000-8000-000000000001}\" type=\"datetime1\"><a:rPr lang=\"en-US\"/><a:t>10/11/2026</a:t></a:fld></a:p>");
        p.data["/ppt/slides/slide1.xml"] = {slideType,
            slideXml(QString(), QString(),
                     placeholderShape(2, QStringLiteral("Title 1"), QStringLiteral("type=\"title\""), QString(), para(QStringLiteral("Quarterly results")))
                         + placeholderShape(3, QStringLiteral("Content Placeholder 2"), QStringLiteral("idx=\"1\""), QString(), para(QString()))
                         + placeholderShape(4, QStringLiteral("Date Placeholder 3"), QStringLiteral("type=\"dt\" sz=\"half\" idx=\"10\""), QString(), dateField)
                         + QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"5\" name=\"TextBox 4\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>") + xfrm(914400, 5029200, 3048000, 457200)
                         + QStringLiteral("<a:noFill/></p:spPr><p:txBody><a:bodyPr wrap=\"square\"/><a:lstStyle/>") + para(QStringLiteral("Plain text")) + QStringLiteral("</p:txBody></p:sp>")
                         + QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"6\" name=\"Rectangle 5\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr>") + xfrm(5486400, 5029200, 1828800, 914400)
                         + QStringLiteral("<a:solidFill><a:schemeClr val=\"accent1\"/></a:solidFill></p:spPr></p:sp>")
                         + pictureShape(7, 8229600, 5029200) + unknownFrame(8),
                     QStringLiteral("<p:transition spd=\"med\"><p:fade/></p:transition><p:timing><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\"/></p:par></p:tnLst></p:timing>"))
                .toUtf8()};
        p.data["/ppt/slides/slide2.xml"] = {slideType,
            slideXml(QStringLiteral("show=\"0\" showMasterSp=\"0\""),
                     QStringLiteral("<p:bg><p:bgPr><a:solidFill><a:srgbClr val=\"112233\"/></a:solidFill><a:effectLst/></p:bgPr></p:bg>"),
                     placeholderShape(2, QStringLiteral("Title 1"), QStringLiteral("type=\"ctrTitle\""), QString(), para(QStringLiteral("Second")))
                         + placeholderShape(3, QStringLiteral("Subtitle 2"), QStringLiteral("type=\"subTitle\" idx=\"1\""), QString(), para(QString()))
                         + chartWithPicture(4) + objectWithPicture(6))
                .toUtf8()};
        p.rels << Rel{"/ppt/slides/slide1.xml", "slideLayout", "/ppt/slideLayouts/slideLayout2.xml"}
               << Rel{"/ppt/slides/slide1.xml", "image", "/ppt/media/image1.png"}
               << Rel{"/ppt/slides/slide1.xml", "notesSlide", "/ppt/notesSlides/notesSlide1.xml"}
               << Rel{"/ppt/slides/slide2.xml", "slideLayout", "/ppt/slideLayouts/slideLayout1.xml"}
               << Rel{"/ppt/slides/slide2.xml", "image", "/ppt/media/image1.png"};
        p.data["/ppt/media/image1.png"] = {QStringLiteral("image/png"), tinyPng()};

        p.data["/ppt/notesSlides/notesSlide1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.notesSlide+xml"),
            (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:notes %1>").arg(nsDecl()) + treeStart()
             + QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"2\" name=\"Slide Image Placeholder 1\"/><p:cNvSpPr/><p:nvPr><p:ph type=\"sldImg\"/></p:nvPr></p:nvSpPr><p:spPr/></p:sp>")
             + placeholderShape(3, QStringLiteral("Notes Placeholder 2"), QStringLiteral("type=\"body\" idx=\"1\""), QString(),
                                para(QStringLiteral("Say hello first")) + para(QStringLiteral("Then the numbers")))
             + treeEnd() + QStringLiteral("</p:notes>")).toUtf8()};
        p.rels << Rel{"/ppt/notesSlides/notesSlide1.xml", "notesMaster", "/ppt/notesMasters/notesMaster1.xml"}
               << Rel{"/ppt/notesSlides/notesSlide1.xml", "slide", "/ppt/slides/slide1.xml"};
        p.data["/ppt/notesMasters/notesMaster1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.notesMaster+xml"),
            (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:notesMaster %1>").arg(nsDecl()) + treeStart() + treeEnd()
             + QStringLiteral("<p:clrMap bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/>"
                              "<p:notesStyle><a:lvl1pPr marL=\"0\"><a:defRPr sz=\"1200\"/></a:lvl1pPr></p:notesStyle></p:notesMaster>")).toUtf8()};
        p.rels << Rel{"/ppt/notesMasters/notesMaster1.xml", "theme", "/ppt/theme/theme1.xml"};
        return p;
    }
    static QByteArray pack(const Parts &p)
    {
        office::PackageWriter w;
        w.addDefault(QStringLiteral("png"), QStringLiteral("image/png"));
        for (auto it = p.data.cbegin(); it != p.data.cend(); ++it) w.addPart(it.key(), it.value().xml, it.value().type);
        w.addRelationship(QStringLiteral("/"), relBase() + QStringLiteral("officeDocument"), QStringLiteral("/ppt/presentation.xml"));
        for (const Rel &r : p.rels) w.addRelationship(r.source, r.type.contains(QLatin1String("://")) ? r.type : relBase() + r.type, r.target, r.external);
        return w.finish();
    }
    // Whether the shared reader reads pictures yet (a picture is the one
    // shape these tests need that its first version leaves out).
    static bool readerReadsPictures()
    {
        QDomDocument dom;
        if (!office::parseXml(("<p:pic " + nsDecl() + "><p:nvPicPr><p:cNvPr id=\"1\" name=\"P\"/><p:cNvPicPr/><p:nvPr/></p:nvPicPr><p:blipFill><a:blip/></p:blipFill><p:spPr>" + xfrm(0, 0, 1270, 1270) + "</p:spPr></p:pic>").toUtf8(), &dom))
            return false;
        return office::readShape(dom.documentElement(), office::DrawingContext()) != nullptr;
    }
    static QString textOf(const Document &d, const Item &it)
    {
        const auto *t = static_cast<const TextItem *>(&it);
        const QTextDocument *sd = d.storyDoc(t->storyId);
        return sd ? sd->toPlainText() : QString();
    }

private Q_SLOTS:
    // The page size, and the slide master with its layouts over it.
    void importReadsSizeMastersAndLayouts()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        QCOMPARE(d->pageSize(), QSizeF(960, 540));
        QCOMPARE(d->setup.sheet, QSizeF(960, 540));
        QCOMPARE(d->setup.sizeName, QStringLiteral("Widescreen"));
        QCOMPARE(int(d->masters.size()), 3);
        const MasterPage &master = *d->masters[0];
        QVERIFY(deck::isSlideMaster(master));
        QCOMPARE(master.id, QStringLiteral("SM1"));
        QCOMPARE(master.name, QStringLiteral("Test Master"));
        QVERIFY(master.baseId.isEmpty());
        QCOMPARE(master.background, Fill::solid(ColorRef::rgb(QColor(0xFF, 0xFF, 0xFF))));
        QCOMPARE(int(master.items.size()), 4);   // the title, text, and date prompts, and a band
        QStringList roles;
        for (const auto &it : master.items) roles << it->role;
        QCOMPARE(roles, (QStringList{"ph:title", "ph:body:1", "ph:dt:2", QString()}));
        QCOMPARE(master.items[0]->rect, QRectF(66, 28.75, 828, 1325563 / 12700.0));
        QCOMPARE(textOf(*d, *master.items[0]), QStringLiteral("Click to edit Master title style"));

        const QVector<MasterPage *> layouts = deck::layouts(*d);
        QCOMPARE(int(layouts.size()), 2);
        QCOMPARE(layouts[0]->id, QStringLiteral("L1"));
        QCOMPARE(layouts[0]->name, QStringLiteral("Title Slide"));
        QCOMPARE(deck::layoutType(*layouts[0]), QStringLiteral("title"));
        QCOMPARE(layouts[0]->baseId, QStringLiteral("SM1"));
        QVERIFY(!layouts[0]->showMasterItems);   // showMasterSp="0"
        QCOMPARE(layouts[1]->id, QStringLiteral("L2"));
        QCOMPARE(layouts[1]->name, QStringLiteral("Title and Content"));
        QCOMPARE(deck::layoutType(*layouts[1]), QStringLiteral("obj"));
        QCOMPARE(layouts[1]->baseId, QStringLiteral("SM1"));
        QVERIFY(layouts[1]->showMasterItems);
        QCOMPARE(d->masterChain(layouts[1]), (QVector<MasterPage *>{d->masters[0].get(), layouts[1]}));
        // The layout's placeholders sample their text, and stand where the
        // layout puts them, else where the master does (by type).
        QStringList layoutRoles;
        for (const auto &it : layouts[1]->items) layoutRoles << it->role;
        QCOMPARE(layoutRoles, (QStringList{"ph:title", "ph:obj:1", "ph:dt:10"}));
        QCOMPARE(layouts[1]->items[0]->rect, master.items[0]->rect);
        QCOMPARE(layouts[1]->items[1]->rect, QRectF(66, 143.75, 408, 4351338 / 12700.0));
        QCOMPARE(layouts[1]->items[2]->rect, master.items[2]->rect);
        QCOMPARE(textOf(*d, *layouts[1]->items[0]), QStringLiteral("Click to edit Master title style"));
        QCOMPARE(layouts[0]->items[0]->rect, QRectF(120, 1122363 / 12700.0, 720, 2387600 / 12700.0));
        QCOMPARE(layouts[0]->items[0]->role, QStringLiteral("ph:ctrTitle"));
        QCOMPARE(layouts[0]->items[1]->role, QStringLiteral("ph:subTitle:1"));
    }

    // Slides: their order and layouts, hidden slides, backgrounds, transitions.
    void importReadsSlides()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 2);
        const Page &first = *d->pages[0], &second = *d->pages[1];
        QCOMPARE(first.masterId, QStringLiteral("L2"));
        QCOMPARE(second.masterId, QStringLiteral("L1"));
        QCOMPARE(deck::layoutType(*d->masterFor(first)), QStringLiteral("obj"));
        QCOMPARE(deck::layoutType(*d->masterFor(second)), QStringLiteral("title"));
        QVERIFY(!first.hidden);
        QVERIFY(second.hidden);
        QVERIFY(first.showMasterItems);
        QVERIFY(!second.showMasterItems);
        QVERIFY(first.background.isNone());   // the layout's and master's show
        QCOMPARE(second.background, Fill::solid(ColorRef::rgb(QColor(0x11, 0x22, 0x33))));
        QCOMPARE(first.title, QStringLiteral("Quarterly results"));
        // The transition and timing are kept as written.
        const QString transition = first.props.value(QStringLiteral("transitionXml")).toString();
        QVERIFY(transition.startsWith(QStringLiteral("<p:transition")));
        QVERIFY2(transition.contains(QStringLiteral("<p:fade/>")), qPrintable(transition));
        QVERIFY(first.props.value(QStringLiteral("timingXml")).toString().contains(QStringLiteral("tmRoot")));
        QVERIFY(!second.props.contains(QStringLiteral("transitionXml")));
        // Footers show on the slides, so new slides get them too.
        QVERIFY(d->appData.value(QStringLiteral("footers")).toBool());
        // The presentation draws.
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        QVERIFY(!Renderer::renderToImage(ctx, 0, 0.5).isNull());
        QCOMPARE(Renderer::renderToImage(ctx, 1, 0.5).pixelColor(10, 10), QColor(0x11, 0x22, 0x33));
    }

    // Placeholders: roles from the file, positions from the layout, else the
    // master; a prompt on the empty one only; shapes that aren't placeholders.
    void importPlaceholdersInheritFromLayoutAndMaster()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const Page &first = *d->pages[0];
        QStringList roles;
        for (const auto &it : first.items) roles << it->role;
        QVERIFY2(roles.mid(0, 3) == (QStringList{"ph:title", "ph:obj:1", "ph:dt:10"}), qPrintable(roles.join(' ')));
        // The title says nothing of its place: the layout's says nothing
        // either, so the master's gives it.
        QCOMPARE(first.items[0]->rect, d->masters[0]->items[0]->rect);
        // The empty content placeholder takes the layout's.
        QCOMPARE(first.items[1]->rect, QRectF(66, 143.75, 408, 4351338 / 12700.0));
        QCOMPARE(first.items[2]->rect, d->masters[0]->items[2]->rect);
        QCOMPARE(textOf(*d, *first.items[0]), QStringLiteral("Quarterly results"));
        QVERIFY(textOf(*d, *first.items[1]).isEmpty());
        for (int i = 0; i < 3; ++i) QCOMPARE(first.items[i]->type(), ItemType::Text);
        QVERIFY(static_cast<const TextItem *>(first.items[0].get())->prompt.isEmpty());
        QCOMPARE(static_cast<const TextItem *>(first.items[1].get())->prompt, deck::promptFor(QStringLiteral("obj")));
        QVERIFY(!static_cast<const TextItem *>(first.items[1].get())->prompt.isEmpty());
        // The date has its field's text, so no prompt.
        QVERIFY(!textOf(*d, *first.items[2]).isEmpty());
        QVERIFY(static_cast<const TextItem *>(first.items[2].get())->prompt.isEmpty());
        // The title slide's: a centered title and a subtitle, by their layout.
        const Page &second = *d->pages[1];
        QCOMPARE(second.items[0]->role, QStringLiteral("ph:ctrTitle"));
        QCOMPARE(second.items[0]->rect, QRectF(120, 1122363 / 12700.0, 720, 2387600 / 12700.0));
        QCOMPARE(second.items[1]->role, QStringLiteral("ph:subTitle:1"));
        QCOMPARE(second.items[1]->rect, QRectF(120, 3602038 / 12700.0, 720, 1655762 / 12700.0));
        QCOMPARE(static_cast<const TextItem *>(second.items[1].get())->prompt, deck::promptFor(QStringLiteral("subTitle")));
        QVERIFY(static_cast<const TextItem *>(second.items[0].get())->prompt.isEmpty());
        // A text box and a rectangle are not placeholders, and stay where they are.
        QCOMPARE(first.items[3]->type(), ItemType::Text);
        QVERIFY(first.items[3]->role.isEmpty());
        QCOMPARE(first.items[3]->rect, QRectF(72, 396, 240, 36));
        QCOMPARE(textOf(*d, *first.items[3]), QStringLiteral("Plain text"));
        QVERIFY(static_cast<const TextItem *>(first.items[3].get())->prompt.isEmpty());
        QCOMPARE(first.items[4]->type(), ItemType::Shape);
        QCOMPARE(first.items[4]->rect, QRectF(432, 396, 144, 72));
        QVERIFY(first.items[4]->role.isEmpty());
    }

    // A picture's file is stored once, and the picture points at it.
    void importStoresPictures()
    {
        if (!readerReadsPictures()) QSKIP("the shared reader doesn't read pictures yet");
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const Page &first = *d->pages[0];
        QCOMPARE(int(first.items.size()), 6);
        const Item &pic = *first.items[5];
        QCOMPARE(pic.type(), ItemType::Picture);
        QCOMPARE(pic.rect, QRectF(648, 396, 72, 72));
        const QString id = static_cast<const PictureItem &>(pic).imageId;
        QVERIFY(d->images.contains(id));
        QCOMPARE(d->images.value(id).format, QStringLiteral("png"));
        QCOMPARE(d->images.value(id).pixelSize, QSize(4, 4));
        // The fallback picture of the chart on the second slide, and the preview of the object, are the same file.
        const Page &second = *d->pages[1];
        QCOMPARE(int(second.items.size()), 4);
        QCOMPARE(second.items[2]->type(), ItemType::Picture);
        QCOMPARE(second.items[2]->rect, QRectF(96, 96, 72, 72));
        QCOMPARE(static_cast<const PictureItem *>(second.items[2].get())->imageId, id);
        QCOMPARE(second.items[3]->type(), ItemType::Picture);
        QCOMPARE(second.items[3]->rect, QRectF(576, 96, 72, 72));
        QCOMPARE(static_cast<const PictureItem *>(second.items[3].get())->imageId, id);
        QCOMPARE(int(d->images.size()), 1);
    }

    // What can't be read yet is kept to write back, with its place.
    void importKeepsUnreadFrames()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const QJsonArray unread = d->pages[0]->props.value(QStringLiteral("unread")).toArray();
        QCOMPARE(int(unread.size()), 1);
        const QJsonObject frame = unread[0].toObject();
        QCOMPARE(frame.value(QStringLiteral("x")).toDouble(), 72.0);
        QCOMPARE(frame.value(QStringLiteral("y")).toDouble(), 144.0);
        QCOMPARE(frame.value(QStringLiteral("w")).toDouble(), 288.0);
        QCOMPARE(frame.value(QStringLiteral("h")).toDouble(), 108.0);
        // The XML stands alone: it parses by itself, with the prefixes it uses declared.
        QDomDocument dom;
        QString why;
        QVERIFY2(office::parseXml(frame.value(QStringLiteral("xml")).toString().toUtf8(), &dom, &why), qPrintable(why));
        QVERIFY(office::isElement(dom.documentElement(), office::ns::P, QStringLiteral("graphicFrame")));
        QVERIFY(frame.value(QStringLiteral("xml")).toString().contains(QStringLiteral("Cell &amp; &lt;1&gt;")));
        QCOMPARE(dom.documentElement().elementsByTagNameNS(QString::fromLatin1(office::ns::A.data()), QStringLiteral("t")).at(0).toElement().text(), QStringLiteral("Cell & <1>"));
        // The chart and the object on the second slide have pictures of themselves to read in their
        // place; where the reader can't read pictures yet, they are kept like that frame.
        const QJsonArray second = d->pages[1]->props.value(QStringLiteral("unread")).toArray();
        QCOMPARE(int(second.size()), readerReadsPictures() ? 0 : 2);
        if (!second.isEmpty()) {
            const QString xml = second[0].toObject().value(QStringLiteral("xml")).toString();
            QVERIFY(xml.contains(QStringLiteral("AlternateContent")));
            QVERIFY(office::parseXml(xml.toUtf8(), &dom, &why));
            QCOMPARE(second[0].toObject().value(QStringLiteral("x")).toDouble(), 96.0);
            QCOMPARE(second[0].toObject().value(QStringLiteral("w")).toDouble(), 480.0);
            const QString object = second[1].toObject().value(QStringLiteral("xml")).toString();
            QVERIFY(object.contains(QStringLiteral("oleObj")));
            QVERIFY(office::parseXml(object.toUtf8(), &dom, &why));
            QCOMPARE(second[1].toObject().value(QStringLiteral("x")).toDouble(), 576.0);
        }
        // Nothing else is added for them.
        QCOMPARE(int(d->pages[0]->items.size()), readerReadsPictures() ? 6 : 5);
    }

    void importReadsNotes()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(deck::notesText(*d, *d->pages[0]), QStringLiteral("Say hello first\nThen the numbers"));
        // A slide without notes has an empty story to type in.
        QVERIFY(!d->pages[1]->notesStoryId.isEmpty());
        QVERIFY(d->storyDoc(d->pages[1]->notesStoryId));
        QVERIFY(deck::notesText(*d, *d->pages[1]).isEmpty());
        QVERIFY(d->pages[0]->notesStoryId != d->pages[1]->notesStoryId);
    }

    // A chart on a slide is a picture of it (an SVG drawn from the values the file stores), in the frame's place, and its parts
    // (the chart, the workbook behind it, and what they link to) are kept with the slide to be written back.
    void importReadsChartsAsPictures()
    {
        QString err;
        auto d = deck::importPresentation(pack(chartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const Page &page = *d->pages[0];
        const auto *pic = chartPicture(*d, page);
        QVERIFY(pic);
        QCOMPARE(pic->name, QStringLiteral("Chart 8"));
        QCOMPARE(pic->altText, QStringLiteral("Sales by region"));
        QCOMPARE(pic->rect, QRectF(72, 72, 288, 216));
        QCOMPARE(pic->imgRect, QRectF(0, 0, 288, 216));
        QVERIFY(d->images.contains(pic->imageId));
        QCOMPARE(d->images.value(pic->imageId).format, QStringLiteral("svg"));
        QVERIFY(d->images.value(pic->imageId).bytes.contains("<text"));
        QVERIFY(d->images.value(pic->imageId).bytes.contains(">East<"));
        QVERIFY(!d->images.value(pic->imageId).image().isNull());
        // The frame the unknown kind of frame was kept as is still the only one left unread.
        QCOMPARE(int(page.props.value(QStringLiteral("unread")).toArray().size()), 1);
        // What is kept for the picture.
        const QJsonArray charts = page.props.value(QStringLiteral("charts")).toArray();
        QCOMPARE(int(charts.size()), 1);
        const QJsonObject kept = charts[0].toObject();
        QCOMPARE(kept.value(QStringLiteral("item")).toString(), pic->id);
        QCOMPARE(kept.value(QStringLiteral("main")).toString(), QStringLiteral("/ppt/charts/chart1.xml"));
        QDomDocument dom;
        QString why;
        QVERIFY2(office::parseXml(kept.value(QStringLiteral("frameXml")).toString().toUtf8(), &dom, &why), qPrintable(why));
        QVERIFY(office::isElement(dom.documentElement(), office::ns::P, QStringLiteral("graphicFrame")));
        QVERIFY(office::isChartFrame(dom.documentElement()));
        const QJsonObject parts = kept.value(QStringLiteral("parts")).toObject();
        QCOMPARE(parts.keys(), (QStringList{"/ppt/charts/chart1.xml", "/ppt/drawings/drawing1.xml", "/ppt/embeddings/Sheet1.xlsx", "/ppt/media/image1.png", "/ppt/theme/themeOverride1.xml"}));
        const Parts source = chartPptx();
        for (auto it = source.data.cbegin(); it != source.data.cend(); ++it) {
            if (!parts.contains(it.key())) continue;
            const QJsonObject one = parts.value(it.key()).toObject();
            QCOMPARE(QByteArray::fromBase64(one.value(QStringLiteral("data")).toString().toLatin1()), it.value().xml);
            QCOMPARE(one.value(QStringLiteral("type")).toString(), it.value().type);
        }
        const QJsonArray rels = parts.value(QStringLiteral("/ppt/charts/chart1.xml")).toObject().value(QStringLiteral("rels")).toArray();
        QCOMPARE(int(rels.size()), 3);
        QCOMPARE(rels[0].toObject().value(QStringLiteral("id")).toString(), QStringLiteral("rId1"));
        QVERIFY(rels[0].toObject().value(QStringLiteral("type")).toString().endsWith(QStringLiteral("/package")));
        QCOMPARE(rels[0].toObject().value(QStringLiteral("target")).toString(), QStringLiteral("/ppt/embeddings/Sheet1.xlsx"));
    }

    // A chart that can't be read is kept as the frame it is, like any other frame the reader can't read.
    void importLeavesACorruptChartUnread()
    {
        Parts p = chartPptx();
        p.data["/ppt/charts/chart1.xml"].xml = "<c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"/>";
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(!chartPicture(*d, *d->pages[0]));
        QCOMPARE(int(d->pages[0]->props.value(QStringLiteral("unread")).toArray().size()), 2);
        QVERIFY(!d->pages[0]->props.contains(QStringLiteral("charts")));
    }

    // Saved, the chart is a frame again, with its parts and links in the package: the chart, its workbook, and what they relate to.
    void exportWritesTheKeptChartParts()
    {
        QString err;
        auto d = deck::importPresentation(pack(chartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QVERIFY(!skipped.contains(chartPicture(*d, *d->pages[0])->id));   // the chart is written, not skipped
        const QString slidePart = QStringLiteral("/ppt/slides/slide1.xml");
        const QStringList chartParts = relationshipTargets(e, slidePart, QStringLiteral("/chart"));
        QCOMPARE(int(chartParts.size()), 1);
        // The frame's r:id is that link.
        QDomDocument dom;
        const QDomElement root = rootOf(e.zip.value(slidePart.mid(1)), &dom);
        QDomElement chart;
        for (QDomElement frame : office::children(office::child(office::child(root, office::ns::P, QStringLiteral("cSld")), office::ns::P, QStringLiteral("spTree")), office::ns::P, QStringLiteral("graphicFrame")))
            if (office::isChartFrame(frame)) chart = frame;
        QVERIFY(!chart.isNull());
        const office::Relationship link = e.pkg.relationship(slidePart, office::chartFrameRelId(chart));
        QCOMPARE(link.target, chartParts[0]);
        QCOMPARE(e.pkg.contentType(chartParts[0]), QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"));
        // The chart as it was, its workbook and drawing and theme beside it, each reached from the chart as before.
        const Parts source = chartPptx();
        QCOMPARE(e.pkg.part(chartParts[0]), source.data.value(QStringLiteral("/ppt/charts/chart1.xml")).xml);
        const QStringList workbook = relationshipTargets(e, chartParts[0], QStringLiteral("/package"));
        QCOMPARE(int(workbook.size()), 1);
        QCOMPARE(e.pkg.part(workbook[0]), source.data.value(QStringLiteral("/ppt/embeddings/Sheet1.xlsx")).xml);
        QCOMPARE(e.pkg.contentType(workbook[0]), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
        const QStringList shapes = relationshipTargets(e, chartParts[0], QStringLiteral("/chartUserShapes"));
        QCOMPARE(int(shapes.size()), 1);
        QCOMPARE(e.pkg.part(shapes[0]), source.data.value(QStringLiteral("/ppt/drawings/drawing1.xml")).xml);
        const QStringList pictures = relationshipTargets(e, shapes[0], QStringLiteral("/image"));
        QCOMPARE(int(pictures.size()), 1);
        QCOMPARE(e.pkg.part(pictures[0]), tinyPng());
        QCOMPARE(int(relationshipTargets(e, chartParts[0], QStringLiteral("/themeOverride")).size()), 1);
        // The chart's own links keep the ids its XML names (externalData r:id="rId1", userShapes r:id="rId2").
        QCOMPARE(e.pkg.relationship(chartParts[0], QStringLiteral("rId1")).target, workbook[0]);
        QCOMPARE(e.pkg.relationship(chartParts[0], QStringLiteral("rId2")).target, shapes[0]);
        // Its place: the frame keeps its shape number, and no other shape on the slide takes it. It stands where the picture does among the shapes.
        QCOMPARE(chart.firstChildElement().firstChildElement().attribute(QStringLiteral("id")), QStringLiteral("9"));
        QSet<QString> ids;
        QStringList all;
        QVector<QDomElement> stack{root};
        while (!stack.isEmpty()) {
            const QDomElement el = stack.takeLast();
            if (el.localName() == QLatin1String("cNvPr")) {
                QVERIFY2(!ids.contains(el.attribute(QStringLiteral("id"))), qPrintable(el.attribute(QStringLiteral("id"))));
                ids << el.attribute(QStringLiteral("id"));
            }
            for (QDomElement c = el.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack << c;
        }
        const QString slide = part(e, slidePart);
        QVERIFY(slide.indexOf(QStringLiteral("name=\"Rectangle 5\"")) < slide.indexOf(QStringLiteral("name=\"Chart 8\"")));
        // Every part of the package is reached, and the first slide's other frames are as before.
        QVERIFY(slide.contains(QStringLiteral("Cell &amp; &lt;1&gt;")));
    }

    // A chart that was moved, resized, or deleted: its frame goes where the picture is, or is not written at all.
    // A duplicated slide keeps its chart (the copy of the chart's picture is
    // written as the chart, once more) and the animations that aim at its
    // shapes by number.
    void duplicatedSlideKeepsItsChartAndAnimations()
    {
        QString err;
        auto d = deck::importPresentation(pack(chartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const Item *chart = chartPicture(*d, *d->pages[0]);
        QVERIFY(chart);
        // The chart's frame keeps its number from the file; an animation aims at that.
        const QString frame = d->pages[0]->props.value(QStringLiteral("charts")).toArray().at(0).toObject().value(QStringLiteral("frameXml")).toString();
        const int number = QRegularExpression(QStringLiteral("cNvPr\\b[^>]*\\bid=\"(\\d+)\"")).match(frame).captured(1).toInt();
        QVERIFY(number >= 2);
        d->pages[0]->props[QStringLiteral("timingXml")] =
            QStringLiteral("<p:timing xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" nodeType=\"tmRoot\"><p:childTnLst>"
                           "<p:set><p:cBhvr><p:cTn id=\"2\"/><p:tgtEl><p:spTgt spid=\"%1\"/></p:tgtEl></p:cBhvr></p:set></p:childTnLst></p:cTn></p:par></p:tnLst></p:timing>").arg(number);
        QCOMPARE(deck::duplicateSlide(*d, 0), 1);
        const Item *copy = chartPicture(*d, *d->pages[1]);
        QVERIFY(copy && copy->id != chart->id);
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        for (const QString &slide : {QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/ppt/slides/slide2.xml")}) {
            QCOMPARE(int(relationshipTargets(e, slide, QStringLiteral("/chart")).size()), 1);
            QVERIFY2(part(e, slide).contains(QStringLiteral("<p:spTgt spid=\"%1\"/>").arg(number)), qPrintable(slide));
        }
        // Two charts, each with parts of its own.
        QVERIFY(relationshipTargets(e, QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/chart"))
                != relationshipTargets(e, QStringLiteral("/ppt/slides/slide2.xml"), QStringLiteral("/chart")));
        QVERIFY2(skipped.filter(QStringLiteral("Slide 2")).isEmpty(), qPrintable(skipped.join('\n')));
    }

    void exportPlacesAndDropsCharts()
    {
        QString err;
        auto d = deck::importPresentation(pack(chartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        auto *pic = const_cast<PictureItem *>(chartPicture(*d, *d->pages[0]));
        QVERIFY(pic);
        pic->rect = QRectF(100, 50, 200, 150);
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY2(slide.contains(QStringLiteral("<a:off x=\"1270000\" y=\"635000\"/><a:ext cx=\"2540000\" cy=\"1905000\"/>")), qPrintable(slide.mid(slide.indexOf(QStringLiteral("Chart 8")), 600)));
        QCOMPARE(int(relationshipTargets(e, QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/chart")).size()), 1);
        // Taken off the slide, its kept parts are not written, and nothing points at them.
        auto &items = d->pages[0]->items;
        for (size_t i = 0; i < items.size(); ++i)
            if (items[i].get() == pic) {
                items.erase(items.begin() + i);
                break;
            }
        Exported gone;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &gone, &err), qPrintable(err));
        QVERIFY2(packageProblem(gone).isEmpty(), qPrintable(packageProblem(gone)));
        QVERIFY(!part(gone, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("drawingml/2006/chart")));
        for (const QString &name : gone.pkg.partNames()) {
            QVERIFY2(!name.contains(QStringLiteral("/charts/")), qPrintable(name));
            QVERIFY2(!name.contains(QStringLiteral("/embeddings/")), qPrintable(name));
        }
    }

    // What was saved reads again as it did: the picture of the chart, and its parts, for the next save.
    void chartsSurviveAnotherRoundTrip()
    {
        QString err;
        auto d = deck::importPresentation(pack(chartPptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const auto *pic = chartPicture(*back, *back->pages[0]);
        QVERIFY(pic);
        QCOMPARE(pic->rect, QRectF(72, 72, 288, 216));
        QCOMPARE(back->images.value(pic->imageId).format, QStringLiteral("svg"));
        const QJsonObject was = d->pages[0]->props.value(QStringLiteral("charts")).toArray()[0].toObject().value(QStringLiteral("parts")).toObject();
        const QJsonObject now = back->pages[0]->props.value(QStringLiteral("charts")).toArray()[0].toObject().value(QStringLiteral("parts")).toObject();
        QCOMPARE(int(now.size()), int(was.size()));
        // The workbook's bytes come back whole.
        QByteArray workbook;
        for (auto it = now.begin(); it != now.end(); ++it)
            if (it.key().endsWith(QStringLiteral(".xlsx"))) workbook = QByteArray::fromBase64(it.value().toObject().value(QStringLiteral("data")).toString().toLatin1());
        QCOMPARE(workbook, chartPptx().data.value(QStringLiteral("/ppt/embeddings/Sheet1.xlsx")).xml);
        // And once more.
        const QByteArray again = deck::exportPresentation(*back, &err);
        QVERIFY2(!again.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(again, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
    }

    // The parts a chart keeps are written under names no other part has, and its links keep working whatever their ids were.
    void exportRenamesChartPartsAndKeepsLinksWorking()
    {
        // The chart's links are called R9 and x7 in the file; and a second frame on the slide shows the same chart (its parts are kept for each).
        QMap<QString, QByteArray> zip;
        QString err;
        QVERIFY2(readZip(pack(chartPptx()), zip, &err), qPrintable(err));
        auto swap = [&](const QString &name, const QString &from, const QString &to) {
            QString text = QString::fromUtf8(zip.value(name));
            QVERIFY2(text.contains(from), qPrintable(name + " " + from));
            zip[name] = text.replace(from, to).toUtf8();
        };
        swap(QStringLiteral("ppt/charts/chart1.xml"), QStringLiteral("r:id=\"rId1\""), QStringLiteral("r:id=\"R9\""));
        swap(QStringLiteral("ppt/charts/chart1.xml"), QStringLiteral("r:id=\"rId2\""), QStringLiteral("r:id=\"x7\""));
        QString rels = QString::fromUtf8(zip.value(QStringLiteral("ppt/charts/_rels/chart1.xml.rels")));
        rels.replace(QStringLiteral("Id=\"rId1\""), QStringLiteral("Id=\"R9\"")).replace(QStringLiteral("Id=\"rId2\""), QStringLiteral("Id=\"x7\"")).replace(QStringLiteral("Id=\"rId3\""), QStringLiteral("Id=\"a1\""));
        zip[QStringLiteral("ppt/charts/_rels/chart1.xml.rels")] = rels.toUtf8();
        QString slide = QString::fromUtf8(zip.value(QStringLiteral("ppt/slides/slide1.xml")));
        const int at = int(slide.indexOf(QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"9\"")));
        const int end = int(slide.indexOf(QStringLiteral("</p:graphicFrame>"), at)) + 17;
        slide.insert(end, QString(slide.mid(at, end - at)).replace(QStringLiteral("id=\"9\""), QStringLiteral("id=\"10\"")).replace(QStringLiteral("Chart 8"), QStringLiteral("Chart 9")));
        zip[QStringLiteral("ppt/slides/slide1.xml")] = slide.toUtf8();
        ZipWriter z;
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) z.add(it.key(), it.value(), true);
        auto d = deck::importPresentation(z.finish(), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages[0]->props.value(QStringLiteral("charts")).toArray().size()), 2);

        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QStringList charts = relationshipTargets(e, QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/chart"));
        QCOMPARE(int(charts.size()), 2);
        QVERIFY(charts[0].compare(charts[1], Qt::CaseInsensitive) != 0);
        for (const QString &chart : charts) {
            // Its XML names its workbook and its drawing by the ids it has now.
            const QString xml = QString::fromUtf8(e.pkg.part(chart));
            const QRegularExpression ext(QStringLiteral("<c:externalData r:id=\"([^\"]*)\""));
            const QRegularExpression shapes(QStringLiteral("<c:userShapes r:id=\"([^\"]*)\""));
            QVERIFY2(ext.match(xml).hasMatch(), qPrintable(xml));
            QVERIFY(shapes.match(xml).hasMatch());
            const office::Relationship workbook = e.pkg.relationship(chart, ext.match(xml).captured(1)), drawing = e.pkg.relationship(chart, shapes.match(xml).captured(1));
            QVERIFY(workbook.type.endsWith(QStringLiteral("/package")));
            QVERIFY(drawing.type.endsWith(QStringLiteral("/chartUserShapes")));
            QCOMPARE(e.pkg.part(workbook.target), chartPptx().data.value(QStringLiteral("/ppt/embeddings/Sheet1.xlsx")).xml);
        }
        // Each frame has its own link, and every part is written once under its own name.
        QStringList names = e.pkg.partNames();
        for (QString &n : names) n = n.toLower();
        QCOMPARE(int(QSet<QString>(names.begin(), names.end()).size()), int(names.size()));
    }

private:
    // The sample presentation with a chart on its first slide: a column chart of two series, the workbook it takes its numbers from, a
    // drawing of shapes laid over it (with a picture of its own), and a theme of its own.
    static Parts chartPptx()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        const QString frame = QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"9\" name=\"Chart 8\" descr=\"Sales by region\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                                             "<p:xfrm><a:off x=\"914400\" y=\"914400\"/><a:ext cx=\"3657600\" cy=\"2743200\"/></p:xfrm><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/chart\">"
                                             "<c:chart xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" r:id=\"rId4\"/></a:graphicData></a:graphic></p:graphicFrame>");
        xml.replace(QStringLiteral("</p:spTree>"), frame + QStringLiteral("</p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        p.rels << Rel{"/ppt/slides/slide1.xml", "chart", "/ppt/charts/chart1.xml"};
        auto series = [](int idx, const QString &name, const QString &values) {
            return QStringLiteral("<c:ser><c:idx val=\"%1\"/><c:order val=\"%1\"/><c:tx><c:strRef><c:f>Sheet1!$B$1</c:f><c:strCache><c:ptCount val=\"1\"/><c:pt idx=\"0\"><c:v>%2</c:v></c:pt></c:strCache></c:strRef></c:tx>"
                                  "<c:cat><c:strRef><c:f>Sheet1!$A$2:$A$4</c:f><c:strCache><c:ptCount val=\"3\"/><c:pt idx=\"0\"><c:v>East</c:v></c:pt><c:pt idx=\"1\"><c:v>West</c:v></c:pt><c:pt idx=\"2\"><c:v>North</c:v></c:pt></c:strCache></c:strRef></c:cat>"
                                  "<c:val><c:numRef><c:f>Sheet1!$B$2:$B$4</c:f><c:numCache><c:formatCode>General</c:formatCode><c:ptCount val=\"3\"/>%3</c:numCache></c:numRef></c:val></c:ser>")
                .arg(idx).arg(name, values);
        };
        const QString chart = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" "
                                             "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
                                             "<c:chart><c:autoTitleDeleted val=\"1\"/><c:plotArea><c:layout/><c:barChart><c:barDir val=\"col\"/><c:grouping val=\"clustered\"/>")
                              + series(0, QStringLiteral("Q1"), QStringLiteral("<c:pt idx=\"0\"><c:v>10</c:v></c:pt><c:pt idx=\"1\"><c:v>20</c:v></c:pt><c:pt idx=\"2\"><c:v>40</c:v></c:pt>"))
                              + series(1, QStringLiteral("Q2"), QStringLiteral("<c:pt idx=\"0\"><c:v>5</c:v></c:pt><c:pt idx=\"1\"><c:v>15</c:v></c:pt><c:pt idx=\"2\"><c:v>25</c:v></c:pt>"))
                              + QStringLiteral("<c:axId val=\"1\"/><c:axId val=\"2\"/></c:barChart>"
                                               "<c:catAx><c:axId val=\"1\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"b\"/><c:crossAx val=\"2\"/></c:catAx>"
                                               "<c:valAx><c:axId val=\"2\"/><c:scaling><c:orientation val=\"minMax\"/></c:scaling><c:delete val=\"0\"/><c:axPos val=\"l\"/><c:majorGridlines/><c:crossAx val=\"1\"/></c:valAx>"
                                               "</c:plotArea><c:legend><c:legendPos val=\"b\"/></c:legend><c:plotVisOnly val=\"1\"/></c:chart>"
                                               "<c:externalData r:id=\"rId1\"><c:autoUpdate val=\"0\"/></c:externalData><c:userShapes r:id=\"rId2\"/></c:chartSpace>");
        p.data["/ppt/charts/chart1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chart+xml"), chart.toUtf8()};
        p.rels << Rel{"/ppt/charts/chart1.xml", "package", "/ppt/embeddings/Sheet1.xlsx"}
               << Rel{"/ppt/charts/chart1.xml", "chartUserShapes", "/ppt/drawings/drawing1.xml"}
               << Rel{"/ppt/charts/chart1.xml", "themeOverride", "/ppt/theme/themeOverride1.xml"};
        p.data["/ppt/embeddings/Sheet1.xlsx"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"), QByteArray("PK\x03\x04 a workbook, as far as this test goes")};
        p.data["/ppt/drawings/drawing1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.drawingml.chartshapes+xml"),
            QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><c:userShapes xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"/>")};
        p.rels << Rel{"/ppt/drawings/drawing1.xml", "image", "/ppt/media/image1.png"};
        p.data["/ppt/theme/themeOverride1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.themeOverride+xml"), themeXml()};
        return p;
    }
    // The picture a chart is on the slide: the one the slide's kept charts name.
    static const PictureItem *chartPicture(const Document &d, const Page &page)
    {
        const QJsonArray charts = page.props.value(QStringLiteral("charts")).toArray();
        for (const QJsonValue &v : charts) {
            const Document::Loc at = d.find(v.toObject().value(QStringLiteral("item")).toString());
            if (at.item && at.item->type() == ItemType::Picture) return static_cast<const PictureItem *>(at.item);
        }
        // A chart that was not kept shows as no picture at all: pictures with an SVG of their own and the name the frame had.
        return nullptr;
    }

private Q_SLOTS:
    // The theme's colors and fonts are the presentation's, and its part is kept to write back.
    // A transition the file wraps for newer readers names a prefix in
    // Requires; the kept copy declares that prefix where Requires is, or the
    // presentation program refuses the saved file.
    void importKeepsCompatibilityPrefixes()
    {
        Parts p = samplePptx();
        const QString mc = QStringLiteral("http://schemas.openxmlformats.org/markup-compatibility/2006");
        p.data["/ppt/slides/slide1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide+xml"),
            slideXml(QStringLiteral("xmlns:mc=\"%1\" xmlns:x14=\"urn:example:x14\"").arg(mc), QString(), QString(),
                     QStringLiteral("<mc:AlternateContent><mc:Choice Requires=\"x14\"><p:transition spd=\"slow\" x14:dur=\"2000\"/></mc:Choice>"
                                    "<mc:Fallback><p:transition spd=\"slow\"/></mc:Fallback></mc:AlternateContent>"))
                .toUtf8()};
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const QString kept = d->pages[0]->props.value(QStringLiteral("transitionXml")).toString();
        QVERIFY2(QRegularExpression(QStringLiteral("<mc:Choice[^>]*xmlns:x14=\"urn:example:x14\"[^>]*Requires=\"x14\"|<mc:Choice[^>]*Requires=\"x14\"[^>]*xmlns:x14=\"urn:example:x14\""))
                     .match(kept).hasMatch(), qPrintable(kept));
    }

    // A table on a slide is a table (not a frame kept as it was): in its place, with its cells' text and look, and
    // the style of the table styles part put into them. Saved, it is a graphic frame holding a table again.
    void importReadsTablesAndExportWritesThem()
    {
        const QString id = QStringLiteral("{11111111-2222-3333-4444-555555555555}");
        Parts p = samplePptx();
        p.data["/ppt/tableStyles.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.tableStyles+xml"),
            QStringLiteral("<a:tblStyleLst xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" def=\"%1\"><a:tblStyle styleId=\"%1\" styleName=\"Test\">"
                           "<a:wholeTbl><a:tcTxStyle><a:fontRef idx=\"minor\"/><a:srgbClr val=\"202020\"/></a:tcTxStyle><a:tcStyle><a:tcBdr/><a:fill><a:solidFill><a:srgbClr val=\"EEEEEE\"/></a:solidFill></a:fill></a:tcStyle></a:wholeTbl>"
                           "<a:firstRow><a:tcTxStyle b=\"on\"><a:fontRef idx=\"minor\"/><a:srgbClr val=\"FFFFFF\"/></a:tcTxStyle><a:tcStyle><a:tcBdr/><a:fill><a:solidFill><a:srgbClr val=\"1F3864\"/></a:solidFill></a:fill></a:tcStyle></a:firstRow>"
                           "</a:tblStyle></a:tblStyleLst>").arg(id).toUtf8()};
        p.rels << Rel{"/ppt/presentation.xml", "tableStyles", "/ppt/tableStyles.xml"};
        const auto cell = [](const QString &text, const QString &pr = QString()) {
            return QStringLiteral("<a:tc><a:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\"/><a:t>%1</a:t></a:r></a:p></a:txBody><a:tcPr>%2</a:tcPr></a:tc>").arg(text, pr);
        };
        const QString table =
            QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"8\" name=\"Results\"/><p:cNvGraphicFramePr><a:graphicFrameLocks noGrp=\"1\"/></p:cNvGraphicFramePr><p:nvPr/></p:nvGraphicFramePr>"
                           "<p:xfrm><a:off x=\"914400\" y=\"1828800\"/><a:ext cx=\"3657600\" cy=\"741680\"/></p:xfrm>"
                           "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/table\"><a:tbl>"
                           "<a:tblPr firstRow=\"1\"><a:tableStyleId>%1</a:tableStyleId></a:tblPr><a:tblGrid><a:gridCol w=\"1828800\"/><a:gridCol w=\"1828800\"/></a:tblGrid>"
                           "<a:tr h=\"370840\">%2%3</a:tr><a:tr h=\"370840\">%4%5</a:tr></a:tbl></a:graphicData></a:graphic></p:graphicFrame>")
                .arg(id, cell(QStringLiteral("Quarter")), cell(QStringLiteral("Revenue")), cell(QStringLiteral("Q1")),
                     cell(QStringLiteral("12"), QStringLiteral("<a:solidFill><a:srgbClr val=\"FFC000\"/></a:solidFill>")));
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        QVERIFY(xml.contains(unknownFrame(8)));
        xml.replace(unknownFrame(8), table);
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();

        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        // Nothing is kept unread on the slide: the table is an object on it.
        QVERIFY(!d->pages[0]->props.contains(QStringLiteral("unread")));
        const Page &slide = *d->pages[0];
        const TableItem *t = nullptr;
        for (const auto &it : slide.items)
            if (it->type() == ItemType::Table) t = static_cast<const TableItem *>(it.get());
        QVERIFY(t);
        QCOMPARE(t->rows, 2);
        QCOMPARE(t->cols, 2);
        QCOMPARE(t->name, QStringLiteral("Results"));
        QCOMPARE(t->rect, QRectF(72, 144, 288, 2 * 370840 / 12700.0));
        QCOMPARE(t->colW, (QVector<double>{144, 144}));
        QCOMPARE(t->format, id);
        QVERIFY(t->header);
        const auto text = [&](int r, int c) { return d->storyDoc(t->cell(r, c).storyId)->toPlainText(); };
        QCOMPARE(text(0, 0), QStringLiteral("Quarter"));
        QCOMPARE(text(1, 1), QStringLiteral("12"));
        // The style's fills and header text, and the cell's own fill over the style's.
        QCOMPARE(t->cell(0, 1).fill.color.rgbValue(), QColor(0x1F, 0x38, 0x64));
        QCOMPARE(t->cell(1, 0).fill.color.rgbValue(), QColor(0xEE, 0xEE, 0xEE));
        QCOMPARE(t->cell(1, 1).fill.color.rgbValue(), QColor(0xFF, 0xC0, 0x00));
        const QTextCharFormat head = d->storyDoc(t->cell(0, 0).storyId)->begin().begin().fragment().charFormat();
        QCOMPARE(head.fontWeight(), int(QFont::Bold));
        QCOMPARE(head.foreground().color(), QColor(0xFF, 0xFF, 0xFF));
        QVERIFY(d->storyDoc(t->cell(1, 0).storyId)->begin().begin().fragment().charFormat().fontWeight() < int(QFont::Bold));

        // Saved: a graphic frame with a table, in the order of the slide's objects, with its own shape number.
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString saved = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY2(saved.contains(QStringLiteral("<p:graphicFrame>")) && saved.contains(QStringLiteral("<a:tbl>")), qPrintable(saved));
        QVERIFY(saved.contains(QStringLiteral("uri=\"http://schemas.openxmlformats.org/drawingml/2006/table\"")));
        QVERIFY(saved.contains(QStringLiteral("<a:tableStyleId>%1</a:tableStyleId>").arg(id)));
        QCOMPARE(saved.count(QStringLiteral("<a:tr ")), 2);
        QCOMPARE(saved.count(QStringLiteral("<a:tc>")), 4);
        for (const QString &s : std::as_const(skipped)) QVERIFY2(!s.contains(QStringLiteral("Table")), qPrintable(s));
        QDomDocument dom;
        QVERIFY(office::parseXml(saved.toUtf8(), &dom));
        QStringList ids;
        QVector<QDomElement> stack{dom.documentElement()};
        while (!stack.isEmpty()) {
            const QDomElement el = stack.takeLast();
            if (el.localName() == QLatin1String("cNvPr")) ids << el.attribute(QStringLiteral("id"));
            for (QDomElement c = el.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack << c;
        }
        QCOMPARE(int(QSet<QString>(ids.cbegin(), ids.cend()).size()), int(ids.size()));

        // And read again, it is the same table.
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY2(!back->pages[0]->props.contains(QStringLiteral("unread")), qPrintable(err));
        const TableItem *u = nullptr;
        for (const auto &it : back->pages[0]->items)
            if (it->type() == ItemType::Table) u = static_cast<const TableItem *>(it.get());
        QVERIFY(u);
        QCOMPARE(u->rows, 2);
        QCOMPARE(u->cols, 2);
        QCOMPARE(u->colW, t->colW);
        QCOMPARE(u->rowH, t->rowH);
        QCOMPARE(u->rect, t->rect);
        QCOMPARE(u->format, id);
        for (int r = 0; r < 2; ++r)
            for (int c = 0; c < 2; ++c) QVERIFY(u->cell(r, c).fill == t->cell(r, c).fill);
        QCOMPARE(back->storyDoc(u->cell(0, 1).storyId)->toPlainText(), QStringLiteral("Revenue"));
    }

    // A table in a pair of alternatives is read from the choice, and its fallback picture is left alone.
    void importReadsATableInAlternatives()
    {
        Parts p = samplePptx();
        const QString table = QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"8\" name=\"Chosen\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                                             "<p:xfrm><a:off x=\"914400\" y=\"914400\"/><a:ext cx=\"1828800\" cy=\"370840\"/></p:xfrm>"
                                             "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/table\"><a:tbl><a:tblPr/><a:tblGrid><a:gridCol w=\"1828800\"/></a:tblGrid>"
                                             "<a:tr h=\"370840\"><a:tc><a:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:t>Alternative</a:t></a:r></a:p></a:txBody><a:tcPr/></a:tc></a:tr>"
                                             "</a:tbl></a:graphicData></a:graphic></p:graphicFrame>");
        const QString alternatives = QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\">"
                                                    "<mc:Choice xmlns:a14=\"urn:test:a14\" Requires=\"a14\">%1</mc:Choice><mc:Fallback>%2</mc:Fallback></mc:AlternateContent>")
                                         .arg(table, pictureShape(9, 914400, 914400));
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        xml.replace(unknownFrame(8), alternatives);
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        int tables = 0, pictures = 0;
        for (const auto &it : d->pages[0]->items) {
            tables += it->type() == ItemType::Table;
            pictures += it->type() == ItemType::Picture;
        }
        QCOMPARE(tables, 1);
        QCOMPARE(pictures, 1);   // the slide's own, not the fallback's
        QVERIFY(!d->pages[0]->props.contains(QStringLiteral("unread")));
    }

    // The height a table's row has in the file is the least it can be: the row grows to hold its text, as in the
    // presentation program, and the table with it.
    void importGrowsTableRowsToTheirText()
    {
        const auto cell = [](const QString &text) {
            return QStringLiteral("<a:tc><a:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\" sz=\"1400\"/><a:t>%1</a:t></a:r></a:p></a:txBody><a:tcPr/></a:tc>").arg(text);
        };
        // One narrow column (1 inch): a row of 10 points that has a sentence to hold, one of 200 that has a word, and one empty.
        const QString table =
            QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"8\" name=\"Grows\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                           "<p:xfrm><a:off x=\"914400\" y=\"914400\"/><a:ext cx=\"914400\" cy=\"2794000\"/></p:xfrm>"
                           "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/table\"><a:tbl><a:tblPr/><a:tblGrid><a:gridCol w=\"914400\"/></a:tblGrid>"
                           "<a:tr h=\"127000\">%1</a:tr><a:tr h=\"2540000\">%2</a:tr><a:tr h=\"127000\"><a:tc><a:txBody><a:bodyPr/><a:lstStyle/><a:p/></a:txBody><a:tcPr/></a:tc></a:tr>"
                           "</a:tbl></a:graphicData></a:graphic></p:graphicFrame>")
                .arg(cell(QStringLiteral("one two three four five six seven eight nine ten")), cell(QStringLiteral("word")));
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        xml.replace(unknownFrame(8), table);
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const TableItem *t = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Table) t = static_cast<const TableItem *>(it.get());
        QVERIFY(t);
        QCOMPARE(t->rows, 3);
        QVERIFY2(t->rowH[0] > 40, qPrintable(QString::number(t->rowH[0])));    // five or so lines of 14 points
        QCOMPARE(t->rowH[1], 200.0);                                           // room enough: as the file says
        QVERIFY2(t->rowH[2] > 10 && t->rowH[2] < 40, qPrintable(QString::number(t->rowH[2])));   // a line of the usual 18 points
        QCOMPARE(t->rect.height(), t->rowH[0] + t->rowH[1] + t->rowH[2]);
        QCOMPARE(t->rect.width(), 72.0);
    }

    void importKeepsTheme()
    {
        const Parts p = samplePptx();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const deck::DeckTheme theme = deck::themeOf(*d);
        QCOMPARE(theme.name, QStringLiteral("Test Theme"));
        QCOMPARE(theme.colors[0], QColor(0x11, 0x11, 0x11));
        QCOMPARE(theme.colors[1], QColor(0xFE, 0xFE, 0xFE));
        QCOMPARE(theme.colors[4], QColor(0xB6, 0x42, 0x0F));
        QCOMPARE(theme.colors[11], QColor(0x55, 0x1A, 0x8B));
        QCOMPARE(theme.headingFont, QStringLiteral("Georgia"));
        QCOMPARE(theme.bodyFont, QStringLiteral("Verdana"));
        QCOMPARE(d->fonts.heading, QStringLiteral("Georgia"));
        QCOMPARE(d->fonts.body, QStringLiteral("Verdana"));
        QCOMPARE(QByteArray::fromBase64(d->appData.value(QStringLiteral("themeXml")).toString().toLatin1()), p.data["/ppt/theme/theme1.xml"].xml);
        // The notes page size.
        QCOMPARE(d->appData.value(QStringLiteral("notesSize")).toObject().value(QStringLiteral("w")).toDouble(), 540.0);
        QCOMPARE(d->appData.value(QStringLiteral("notesSize")).toObject().value(QStringLiteral("h")).toDouble(), 720.0);
    }

    // A slide's placeholder takes its layout's by index, then by type; a layout's takes its master's by
    // type, and any kind of content takes the master's text placeholder.
    void importMatchesPlaceholdersByIndexThenType()
    {
        Parts p = samplePptx();
        const QString layoutType = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml");
        p.data["/ppt/slideLayouts/slideLayout2.xml"] = {layoutType,
            layoutXml(QStringLiteral("twoObj"), QStringLiteral("Two Content"), QString(),
                      placeholderShape(2, QStringLiteral("Title 1"), QStringLiteral("type=\"title\""), QString(), para(QStringLiteral("t")))
                          + placeholderShape(3, QStringLiteral("Left"), QStringLiteral("idx=\"1\""), xfrm(838200, 1825625, 5181600, 4351338), para(QStringLiteral("l")))
                          + placeholderShape(4, QStringLiteral("Right"), QStringLiteral("idx=\"2\""), xfrm(6172200, 1825625, 5181600, 4351338), para(QStringLiteral("r")))
                          + placeholderShape(5, QStringLiteral("Picture"), QStringLiteral("type=\"pic\" idx=\"13\""), QString(), para(QString())))
                .toUtf8()};
        p.data["/ppt/slides/slide1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide+xml"),
            slideXml(QString(), QString(),
                     placeholderShape(2, QStringLiteral("Right"), QStringLiteral("type=\"body\" idx=\"2\""), QString(), para(QStringLiteral("right")))
                         + placeholderShape(3, QStringLiteral("Left"), QStringLiteral("idx=\"1\""), QString(), para(QStringLiteral("left")))
                         + placeholderShape(4, QStringLiteral("Centered title"), QStringLiteral("type=\"ctrTitle\""), QString(), para(QStringLiteral("title")))
                         + placeholderShape(5, QStringLiteral("Picture"), QStringLiteral("type=\"pic\" idx=\"13\""), QString(), para(QString()))
                         + placeholderShape(6, QStringLiteral("Own place"), QStringLiteral("idx=\"1\""), xfrm(0, 0, 1270000, 635000), para(QStringLiteral("own"))))
                .toUtf8()};
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const Page &slide = *d->pages[0];
        QCOMPARE(int(slide.items.size()), 5);
        QCOMPARE(slide.items[0]->role, QStringLiteral("ph:body:2"));
        QCOMPARE(slide.items[0]->rect, QRectF(486, 143.75, 408, 4351338 / 12700.0));    // the layout's second content place
        QCOMPARE(slide.items[1]->rect, QRectF(66, 143.75, 408, 4351338 / 12700.0));     // the first
        QCOMPARE(slide.items[2]->rect, d->masters[0]->items[0]->rect);                  // title, by kind, and the master's
        QCOMPARE(slide.items[3]->rect, d->masters[0]->items[1]->rect);                  // a picture place: the master's text place
        QCOMPARE(slide.items[4]->rect, QRectF(0, 0, 100, 50));                          // its own, whatever the layout says
        QCOMPARE(d->masters[2]->items[3]->rect, d->masters[0]->items[1]->rect);
        QCOMPARE(slide.items[3]->role, QStringLiteral("ph:pic:13"));
        // An empty picture place: a picture frame waiting for its picture, or a text box prompting for one.
        if (slide.items[3]->type() == ItemType::Picture) QVERIFY(static_cast<const PictureItem *>(slide.items[3].get())->imageId.isEmpty());
        else QCOMPARE(static_cast<const TextItem *>(slide.items[3].get())->prompt, deck::promptFor(QStringLiteral("pic")));
    }

    // The size a file names, in the names the presentation program gives them.
    void importReadsSlideSizes()
    {
        struct Case { const char *attrs; QSizeF size; const char *name; };
        const Case cases[] = {
            {"cx=\"9144000\" cy=\"6858000\" type=\"screen4x3\"", QSizeF(720, 540), "On-screen Show (4:3)"},
            {"cx=\"12192000\" cy=\"6858000\"", QSizeF(960, 540), "Widescreen"},
            {"cx=\"7772400\" cy=\"10058400\" type=\"letter\"", QSizeF(612, 792), "Letter Paper"},
            {"cx=\"5000000\" cy=\"3000000\"", QSizeF(5000000 / 12700.0, 3000000 / 12700.0), "Custom"},
        };
        for (const Case &c : cases) {
            Parts p = samplePptx();
            QString xml = QString::fromUtf8(p.data["/ppt/presentation.xml"].xml);
            xml.replace(QStringLiteral("cx=\"12192000\" cy=\"6858000\""), QString::fromLatin1(c.attrs));
            p.data["/ppt/presentation.xml"].xml = xml.toUtf8();
            QString err;
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY2(d, qPrintable(err));
            QCOMPARE(d->pageSize(), c.size);
            QCOMPARE(d->setup.sizeName, QString::fromLatin1(c.name));
        }
        // A size no slide can have is not taken.
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/presentation.xml"].xml);
        xml.replace(QStringLiteral("cx=\"12192000\" cy=\"6858000\""), QStringLiteral("cx=\"0\" cy=\"-5\""));
        p.data["/ppt/presentation.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY(d);
        QCOMPARE(d->pageSize(), QSizeF(deck::kSlideWidth, deck::kSlideHeight));
        QVERIFY(!err.isEmpty());
    }

    // The standard's other namespaces (strict) read the same.
    void importReadsStrictFiles()
    {
        Parts p = samplePptx();
        for (auto it = p.data.begin(); it != p.data.end(); ++it) {
            if (!it.key().endsWith(QLatin1String(".xml"))) continue;
            QString xml = QString::fromUtf8(it.value().xml);
            xml.replace(QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/main"), QStringLiteral("http://purl.oclc.org/ooxml/drawingml/main"));
            xml.replace(QStringLiteral("http://schemas.openxmlformats.org/presentationml/2006/main"), QStringLiteral("http://purl.oclc.org/ooxml/presentationml/main"));
            xml.replace(QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships"), QStringLiteral("http://purl.oclc.org/ooxml/officeDocument/relationships"));
            it.value().xml = xml.toUtf8();
        }
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 2);
        QCOMPARE(int(deck::layouts(*d).size()), 2);
        QCOMPARE(d->pages[0]->items[1]->rect, QRectF(66, 143.75, 408, 4351338 / 12700.0));
        QCOMPARE(deck::notesText(*d, *d->pages[0]), QStringLiteral("Say hello first\nThen the numbers"));
        QCOMPARE(deck::themeOf(*d).colors[4], QColor(0xB6, 0x42, 0x0F));
    }

    // Damaged and hostile files are refused, or read as far as they go, and say why.
    void importSurvivesHostileFiles()
    {
        QString err;
        QVERIFY(!deck::importPresentation(QByteArray("this is not a package"), &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!deck::importPresentation(QByteArray(), &err));
        QVERIFY(!err.isEmpty());
        // An older binary presentation is told apart from damage.
        err.clear();
        QVERIFY(!deck::importPresentation(QByteArray::fromHex("D0CF11E0A1B11AE1") + QByteArray(512, '\0'), &err));
        QVERIFY(err.contains(QStringLiteral("older")));
        // A package with no presentation in it.
        {
            office::PackageWriter w;
            w.addPart(QStringLiteral("/word/document.xml"), "<w:document xmlns:w=\"urn:test\"/>", QStringLiteral("application/xml"));
            w.addRelationship(QStringLiteral("/"), relBase() + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
            err.clear();
            QVERIFY(!deck::importPresentation(w.finish(), &err));
            QVERIFY(!err.isEmpty());
            office::PackageWriter none;
            none.addPart(QStringLiteral("/a.xml"), "<a/>", QStringLiteral("application/xml"));
            err.clear();
            QVERIFY(!deck::importPresentation(none.finish(), &err));
            QVERIFY(!err.isEmpty());
        }
        // A presentation part that isn't XML.
        {
            Parts p = samplePptx();
            p.data["/ppt/presentation.xml"].xml = "<<< not xml";
            err.clear();
            QVERIFY(!deck::importPresentation(pack(p), &err));
            QVERIFY(!err.isEmpty());
        }
        // A slide part that isn't XML is left out, with the others read.
        {
            Parts p = samplePptx();
            p.data["/ppt/slides/slide2.xml"].xml = "<<< not xml";
            err.clear();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(d->pages.size()), 1);
            QVERIFY2(err.contains(QStringLiteral("Slide 2")), qPrintable(err));
            QCOMPARE(textOf(*d, *d->pages[0]->items[0]), QStringLiteral("Quarterly results"));
        }
        // A relationship to a slide that isn't in the package.
        {
            Parts p = samplePptx();
            p.data.remove(QStringLiteral("/ppt/slides/slide1.xml"));
            err.clear();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(d->pages.size()), 1);
            QCOMPARE(d->pages[0]->masterId, QStringLiteral("L1"));
            QVERIFY2(err.contains(QStringLiteral("Slide 1")), qPrintable(err));
        }
        // A slide whose layout is gone stands on the slide master; one whose picture is gone has none.
        {
            Parts p = samplePptx();
            p.data.remove(QStringLiteral("/ppt/slideLayouts/slideLayout2.xml"));
            p.data.remove(QStringLiteral("/ppt/media/image1.png"));
            err.clear();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(d->pages.size()), 2);
            QCOMPARE(int(deck::layouts(*d).size()), 1);
            QCOMPARE(d->pages[0]->masterId, QStringLiteral("SM1"));
            QVERIFY(!err.isEmpty());
            LayoutCache cache;
            PaintContext ctx;
            ctx.doc = d.get();
            ctx.cache = &cache;
            QVERIFY(!Renderer::renderToImage(ctx, 0, 0.25).isNull());
        }
        // A presentation whose only slide master can't be read has nothing to stand on.
        {
            Parts p = samplePptx();
            p.data["/ppt/slideMasters/slideMaster1.xml"].xml = "<<< not xml";
            err.clear();
            QVERIFY(!deck::importPresentation(pack(p), &err));
            QVERIFY(!err.isEmpty());
        }
        // A theme that isn't one: the presentation's own colors stand in.
        {
            Parts p = samplePptx();
            p.data["/ppt/theme/theme1.xml"].xml = "<a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"/>";
            err.clear();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(d->pages.size()), 2);
            QVERIFY(!err.isEmpty());
        }
        // Parts that name each other in a loop, or themselves.
        {
            Parts p = samplePptx();
            p.rels << Rel{"/ppt/slideLayouts/slideLayout1.xml", "slideLayout", "/ppt/slideLayouts/slideLayout1.xml"}
                   << Rel{"/ppt/slideMasters/slideMaster1.xml", "slideMaster", "/ppt/slideMasters/slideMaster1.xml"}
                   << Rel{"/ppt/slides/slide1.xml", "slide", "/ppt/slides/slide1.xml"};
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(d->pages.size()), 2);
        }
    }

    // At most 10,000 slides and 1,000 layouts are read, however many a file lists.
    void importLimitsSlidesAndLayouts()
    {
        {
            Parts p = samplePptx();
            QString xml = QString::fromUtf8(p.data["/ppt/presentation.xml"].xml);
            QString ids;
            for (int i = 0; i < 10050; ++i) ids += QStringLiteral("<p:sldId id=\"%1\" r:id=\"rId5\"/>").arg(256 + i);
            xml.replace(QRegularExpression(QStringLiteral("<p:sldIdLst>.*</p:sldIdLst>")), QStringLiteral("<p:sldIdLst>") + ids + QStringLiteral("</p:sldIdLst>"));
            p.data["/ppt/presentation.xml"].xml = xml.toUtf8();
            // The one slide they all name has a title and nothing else.
            p.data["/ppt/slides/small.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide+xml"),
                slideXml(QString(), QString(), placeholderShape(2, QStringLiteral("Title 1"), QStringLiteral("type=\"title\""), QString(), para(QStringLiteral("Again")))).toUtf8()};
            p.rels << Rel{"/ppt/presentation.xml", "slide", "/ppt/slides/small.xml"}
                   << Rel{"/ppt/slides/small.xml", "slideLayout", "/ppt/slideLayouts/slideLayout2.xml"};
            QString err;
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(d->pages.size()), 10000);
            QVERIFY(!err.isEmpty());
        }
        {
            Parts p = samplePptx();
            QString xml = QString::fromUtf8(p.data["/ppt/slideMasters/slideMaster1.xml"].xml);
            // The first two layouts, then 1,100 more.
            QString ids = QStringLiteral("<p:sldLayoutId id=\"2147483649\" r:id=\"rId1\"/><p:sldLayoutId id=\"2147483650\" r:id=\"rId2\"/>");
            const int count = 1100;
            QVector<Rel> rels;
            for (const Rel &r : p.rels)
                if (r.source != QLatin1String("/ppt/slideMasters/slideMaster1.xml")) rels << r;
            // The master's links: rId1 and rId2 the first layouts, rId3 the theme, the others follow.
            rels << Rel{"/ppt/slideMasters/slideMaster1.xml", "slideLayout", "/ppt/slideLayouts/slideLayout1.xml"}
                 << Rel{"/ppt/slideMasters/slideMaster1.xml", "slideLayout", "/ppt/slideLayouts/slideLayout2.xml"}
                 << Rel{"/ppt/slideMasters/slideMaster1.xml", "theme", "/ppt/theme/theme1.xml"};
            for (int i = 0; i < count; ++i) {
                const QString name = QStringLiteral("/ppt/slideLayouts/layout%1.xml").arg(i + 3);
                ids += QStringLiteral("<p:sldLayoutId id=\"%1\" r:id=\"rId%2\"/>").arg(2147483651LL + i).arg(i + 4);
                p.data[name] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"),
                                layoutXml(QStringLiteral("blank"), QStringLiteral("Blank %1").arg(i), QString(), QString()).toUtf8()};
                rels << Rel{"/ppt/slideMasters/slideMaster1.xml", "slideLayout", name} << Rel{name, "slideMaster", "/ppt/slideMasters/slideMaster1.xml"};
            }
            p.rels = rels;
            xml.replace(QRegularExpression(QStringLiteral("<p:sldLayoutIdLst>.*</p:sldLayoutIdLst>")), QStringLiteral("<p:sldLayoutIdLst>") + ids + QStringLiteral("</p:sldLayoutIdLst>"));
            p.data["/ppt/slideMasters/slideMaster1.xml"].xml = xml.toUtf8();
            QString err;
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(deck::layouts(*d).size()), 1000);
            QVERIFY(!err.isEmpty());
            QCOMPARE(int(d->pages.size()), 2);
            QCOMPARE(d->pages[0]->masterId, QStringLiteral("L2"));   // the slides' layouts are among the first
        }
    }

    // File > Open: the window takes the presentation in place of an untouched new one.
    void windowOpensFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("quarterly.pptx"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(pack(samplePptx()));
        f.close();
        const int windows = int(AppWindow::windows().size());
        DeckWindow w;
        QVERIFY(w.openFile(path));
        QCOMPARE(int(AppWindow::windows().size()), windows + 1);   // this one, no more
        Editor *ed = w.editor();
        QCOMPARE(int(ed->doc()->pages.size()), 2);
        QCOMPARE(ed->currentPage(), 0);
        QCOMPARE(ed->filePath(), path);
        QVERIFY(!ed->isModified());
        QTRY_COMPARE(w.slidesPane()->count(), 2);
        QCOMPARE(w.windowTitle(), QStringLiteral("quarterly - JeffDeck"));
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QStringLiteral("Say hello first\nThen the numbers"));
        // Moves on to another slide, and opening it again starts over at the first.
        ed->setCurrentPage(1);
        // The window now holds a file, so another one opens in a window of its own.
        QVERIFY(w.openFile(path));
        QTRY_COMPARE(int(AppWindow::windows().size()), windows + 2);
        QCOMPARE(ed->currentPage(), 1);
        const QList<AppWindow *> all = AppWindow::windows();
        for (AppWindow *other : all)
            if (other != &w && other->objectName() == QLatin1String("deckWindow")) {
                auto *opened = static_cast<DeckWindow *>(other);
                QCOMPARE(int(opened->editor()->doc()->pages.size()), 2);
                QCOMPARE(opened->editor()->currentPage(), 0);
                opened->setAttribute(Qt::WA_DeleteOnClose);
                opened->close();
            }
        // A file that can't be read says so, and changes nothing.
        const QString bad = dir.filePath(QStringLiteral("bad.pptx"));
        QFile b(bad);
        QVERIFY(b.open(QIODevice::WriteOnly));
        b.write("not a presentation");
        b.close();
        QString shown;
        QTimer::singleShot(0, [&] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                shown = box->text();
                box->close();
            }
        });
        QVERIFY(!w.openFile(bad));
        QVERIFY2(shown.contains(QStringLiteral("bad.pptx")), qPrintable(shown));
        QCOMPARE(int(ed->doc()->pages.size()), 2);
        ed->markSaved();
    }

private:
    // An exported file as the tests below look at it.
    struct Exported {
        QMap<QString, QByteArray> zip;   // by entry name, no leading slash
        office::Package pkg;
    };
    static bool unpack(const QByteArray &bytes, Exported *out, QString *why)
    {
        return readZip(bytes, out->zip, why) && out->pkg.read(bytes, why);
    }
    static QString part(const Exported &e, const QString &name) { return QString::fromUtf8(e.zip.value(name.mid(1))); }

    // The first thing wrong with a package, as a reader that stops at the first
    // damage would find it; empty when it is sound: every part has a content
    // type and every type names a part, every XML part parses, every
    // relationship and every r:id in a part resolves, and every part is
    // reached from the package's relationships.
    static QString packageProblem(const Exported &e)
    {
        QDomDocument types;
        QString why;
        if (!e.zip.contains(QStringLiteral("[Content_Types].xml"))) return QStringLiteral("no [Content_Types].xml");
        if (!office::parseXml(e.zip.value(QStringLiteral("[Content_Types].xml")), &types, &why)) return QStringLiteral("[Content_Types].xml: ") + why;
        QSet<QString> defaults, overrides;
        for (const QDomElement &d : office::children(types.documentElement(), office::ns::ContentTypes, QStringLiteral("Default"))) defaults << d.attribute(QStringLiteral("Extension")).toLower();
        for (const QDomElement &o : office::children(types.documentElement(), office::ns::ContentTypes, QStringLiteral("Override"))) {
            const QString name = o.attribute(QStringLiteral("PartName"));
            if (!e.zip.contains(name.mid(1))) return QStringLiteral("the content types list a part that is missing: ") + name;
            overrides << name.toLower();
        }
        for (auto it = e.zip.cbegin(); it != e.zip.cend(); ++it) {
            if (it.key() == QLatin1String("[Content_Types].xml")) continue;
            if (!defaults.contains(QFileInfo(it.key()).suffix().toLower()) && !overrides.contains(QLatin1Char('/') + it.key().toLower()))
                return QStringLiteral("no content type for ") + it.key();
            if (!it.key().endsWith(QLatin1String(".xml")) && !it.key().endsWith(QLatin1String(".rels"))) continue;
            QDomDocument dom;
            if (!office::parseXml(it.value(), &dom, &why)) return it.key() + QStringLiteral(" doesn't parse: ") + why;
            if (!it.value().startsWith("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>")) return it.key() + QStringLiteral(" has no XML declaration");
            // Every r:id (and r:embed...) in a part is one of its relationships; an empty one
            // points nowhere (a picture's click that plays its movie, as the presentation program writes it).
            if (it.key().endsWith(QLatin1String(".rels"))) continue;
            QVector<QDomElement> stack{dom.documentElement()};
            while (!stack.isEmpty()) {
                const QDomElement el = stack.takeLast();
                const QDomNamedNodeMap attrs = el.attributes();
                for (int i = 0; i < attrs.length(); ++i) {
                    const QDomAttr a = attrs.item(i).toAttr();
                    if (!office::sameNamespace(a.namespaceURI(), office::ns::R) || a.value().isEmpty()) continue;
                    if (e.pkg.relationship(QLatin1Char('/') + it.key(), a.value()).id.isEmpty()) return it.key() + QStringLiteral(" uses ") + a.name() + QStringLiteral("=") + a.value() + QStringLiteral(", which isn't a relationship");
                }
                for (QDomElement c = el.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack << c;
            }
        }
        QStringList sources = e.pkg.partNames();
        sources << QStringLiteral("/");
        for (const QString &s : std::as_const(sources))
            for (const office::Relationship &r : e.pkg.relationships(s)) {
                if (!r.external && !e.pkg.hasPart(r.target)) return s + QStringLiteral(" links to a part that is missing: ") + r.target;
                if (!r.external && r.type.isEmpty()) return s + QStringLiteral(" has a link without a type");
            }
        QSet<QString> reached;
        QStringList queue{QStringLiteral("/")};
        while (!queue.isEmpty()) {
            const QString s = queue.takeLast();
            for (const office::Relationship &r : e.pkg.relationships(s))
                if (!r.external && !reached.contains(r.target.toLower())) {
                    reached << r.target.toLower();
                    queue << r.target;
                }
        }
        for (const QString &name : e.pkg.partNames())
            if (!reached.contains(name.toLower())) return name + QStringLiteral(" is not linked from anywhere");
        return QString();
    }

    // The local names of an element's children, in order.
    static QStringList childNames(const QByteArray &xml)
    {
        QDomDocument dom;
        QString why;
        if (!office::parseXml(xml, &dom, &why)) return {QStringLiteral("unreadable: ") + why};
        QStringList out;
        for (const QDomElement &c : office::children(dom.documentElement())) out << c.localName();
        return out;
    }
    // Whether names come in the order of the schema's sequence (each at or after the one before).
    static bool inSequence(const QStringList &names, const QStringList &sequence)
    {
        int at = 0;
        for (const QString &n : names) {
            const int i = int(sequence.indexOf(n));
            if (i < at) return false;
            at = i;
        }
        return true;
    }
    static QDomElement rootOf(const QByteArray &xml, QDomDocument *dom)
    {
        office::parseXml(xml, dom);
        return dom->documentElement();
    }
    static QStringList relationshipTargets(const Exported &e, const QString &source, const QString &typeSuffix)
    {
        QStringList out;
        for (const office::Relationship &r : e.pkg.relationshipsOfType(source, typeSuffix)) out << r.target;
        return out;
    }

    // Answers the modal dialogs that open next, one for each answer: a button of a
    // message box, or (NoButton) closing the dialog. The first label clicked is kept in `label`.
    static void answerDialogs(QList<QMessageBox::StandardButton> answers, QString *label = nullptr, QDeadlineTimer until = QDeadlineTimer(10000))
    {
        if (answers.isEmpty() || until.hasExpired()) return;
        QTimer::singleShot(0, [answers, label, until]() mutable {
            QWidget *dialog = QApplication::activeModalWidget();
            if (!dialog) {
                answerDialogs(answers, label, until);
                return;
            }
            const QMessageBox::StandardButton which = answers.takeFirst();
            auto *box = qobject_cast<QMessageBox *>(dialog);
            QAbstractButton *button = box && which != QMessageBox::NoButton ? box->button(which) : nullptr;
            if (button && label && label->isEmpty()) *label = button->text();
            if (button) button->click();
            else dialog->close();
            answerDialogs(answers, label, until);
        });
    }
    static QByteArray fileBytes(const QString &path)
    {
        QFile f(path);
        return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
    }

private Q_SLOTS:
    // A new presentation is written as a whole package: every part has its
    // type, every link leads to a part, and the main part, its master, the
    // layouts, the slides, and the notes are chained as the presentation
    // program chains them.
    void exportWritesACompletePackage()
    {
        auto d = deck::newPresentation();
        d->props.title = QStringLiteral("Quarterly results");
        d->storyDoc(d->pages[0]->notesStoryId)->setPlainText(QStringLiteral("Say hello first"));
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("twoObj")));
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        d->pages[2]->hidden = true;
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QString why;
        QVERIFY2(unpack(bytes, &e, &why), qPrintable(why));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));

        QCOMPARE(e.pkg.mainPart(), QStringLiteral("/ppt/presentation.xml"));
        QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/presentation.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml"));
        const QStringList names = e.pkg.partNames();
        for (const char *n : {"/ppt/presProps.xml", "/ppt/viewProps.xml", "/ppt/tableStyles.xml", "/ppt/theme/theme1.xml", "/ppt/slideMasters/slideMaster1.xml",
                              "/ppt/notesMasters/notesMaster1.xml", "/ppt/slides/slide3.xml", "/ppt/slideLayouts/slideLayout9.xml", "/ppt/notesSlides/notesSlide1.xml",
                              "/docProps/core.xml", "/docProps/app.xml", "/docProps/thumbnail.jpeg"})
            QVERIFY2(names.contains(QLatin1String(n)), n);
        QCOMPARE(int(names.filter(QRegularExpression(QStringLiteral("^/ppt/slides/slide\\d+\\.xml$"))).size()), 3);
        QCOMPARE(int(names.filter(QRegularExpression(QStringLiteral("^/ppt/slideLayouts/slideLayout\\d+\\.xml$"))).size()), 9);
        QCOMPARE(int(names.filter(QRegularExpression(QStringLiteral("^/ppt/notesSlides/"))).size()), 1);   // only the first slide has notes
        QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/slides/slide1.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide+xml"));
        QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/slideLayouts/slideLayout1.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml"));
        QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/slideMasters/slideMaster1.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"));
        QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/theme/theme1.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"));
        QCOMPARE(e.pkg.contentType(QStringLiteral("/docProps/thumbnail.jpeg")), QStringLiteral("image/jpeg"));

        // The chains: presentation to master, slides in order, notes master; master to layouts and theme.
        const QString pres = QStringLiteral("/ppt/presentation.xml");
        QCOMPARE(relationshipTargets(e, pres, QStringLiteral("/slideMaster")), QStringList{QStringLiteral("/ppt/slideMasters/slideMaster1.xml")});
        QCOMPARE(relationshipTargets(e, pres, QStringLiteral("/slide")),
                 (QStringList{QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/ppt/slides/slide2.xml"), QStringLiteral("/ppt/slides/slide3.xml")}));
        QCOMPARE(relationshipTargets(e, pres, QStringLiteral("/notesMaster")), QStringList{QStringLiteral("/ppt/notesMasters/notesMaster1.xml")});
        for (const char *type : {"/presProps", "/viewProps", "/theme", "/tableStyles"}) QCOMPARE(int(e.pkg.relationshipsOfType(pres, QLatin1String(type)).size()), 1);
        const QString master = QStringLiteral("/ppt/slideMasters/slideMaster1.xml");
        const QStringList layoutParts = relationshipTargets(e, master, QStringLiteral("/slideLayout"));
        QCOMPARE(int(layoutParts.size()), 9);
        QCOMPARE(int(e.pkg.relationshipsOfType(master, QStringLiteral("/theme")).size()), 1);
        for (const QString &l : layoutParts) QCOMPARE(relationshipTargets(e, l, QStringLiteral("/slideMaster")), QStringList{master});
        // Each slide stands on the layout of its type, in the order the master lists them.
        const QVector<MasterPage *> layouts = deck::layouts(*d);
        for (int i = 0; i < 3; ++i) {
            const QStringList on = relationshipTargets(e, QStringLiteral("/ppt/slides/slide%1.xml").arg(i + 1), QStringLiteral("/slideLayout"));
            QCOMPARE(int(on.size()), 1);
            QCOMPARE(on[0], layoutParts.value(int(layouts.indexOf(d->masterFor(*d->pages[i])))));
        }
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/notesSlide")), QStringList{QStringLiteral("/ppt/notesSlides/notesSlide1.xml")});
        QVERIFY(relationshipTargets(e, QStringLiteral("/ppt/slides/slide2.xml"), QStringLiteral("/notesSlide")).isEmpty());
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/notesSlides/notesSlide1.xml"), QStringLiteral("/notesMaster")), QStringList{QStringLiteral("/ppt/notesMasters/notesMaster1.xml")});
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/notesSlides/notesSlide1.xml"), QStringLiteral("/slide")), QStringList{QStringLiteral("/ppt/slides/slide1.xml")});
        QCOMPARE(int(e.pkg.relationshipsOfType(QStringLiteral("/ppt/notesMasters/notesMaster1.xml"), QStringLiteral("/theme")).size()), 1);
        // Master and notes master do not share a theme part.
        QVERIFY(relationshipTargets(e, QStringLiteral("/ppt/notesMasters/notesMaster1.xml"), QStringLiteral("/theme")) != relationshipTargets(e, master, QStringLiteral("/theme")));
        // The package's own links.
        QCOMPARE(int(e.pkg.relationshipsOfType(QStringLiteral("/"), QStringLiteral("/officeDocument")).size()), 1);
        QCOMPARE(int(e.pkg.relationshipsOfType(QStringLiteral("/"), QStringLiteral("/metadata/core-properties")).size()), 1);
        QCOMPARE(int(e.pkg.relationshipsOfType(QStringLiteral("/"), QStringLiteral("/extended-properties")).size()), 1);
        QCOMPARE(int(e.pkg.relationshipsOfType(QStringLiteral("/"), QStringLiteral("/metadata/thumbnail")).size()), 1);

        // What the properties say.
        const QString core = part(e, QStringLiteral("/docProps/core.xml")), app = part(e, QStringLiteral("/docProps/app.xml"));
        QVERIFY(core.contains(QStringLiteral("<dc:title>Quarterly results</dc:title>")));
        QVERIFY2(core.contains(QRegularExpression(QStringLiteral("<dcterms:created xsi:type=\"dcterms:W3CDTF\">\\d{4}-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\dZ</dcterms:created>"))), qPrintable(core));
        QVERIFY(core.contains(QStringLiteral("<dcterms:modified xsi:type=\"dcterms:W3CDTF\">")));
        QVERIFY(app.contains(QStringLiteral("<Application>JeffOffice</Application>")));
        QVERIFY(app.contains(QStringLiteral("<Slides>3</Slides>")));
        QVERIFY(app.contains(QStringLiteral("<Notes>1</Notes>")));
        QVERIFY(app.contains(QStringLiteral("<HiddenSlides>1</HiddenSlides>")));
        // The thumbnail is the first slide, 256 pixels across.
        const QImage thumb = QImage::fromData(e.zip.value(QStringLiteral("docProps/thumbnail.jpeg")));
        QCOMPARE(thumb.width(), 256);
        QVERIFY(std::abs(thumb.height() - 144) <= 1);
    }

    // Each part's elements follow the schema's sequence, the ids that must be
    // unique are, and the slide size says what it is.
    void exportWritesElementsInSchemaOrder()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));

        const QByteArray pres = e.zip.value(QStringLiteral("ppt/presentation.xml"));
        QStringList names = childNames(pres);
        QVERIFY2(inSequence(names, {"sldMasterIdLst", "notesMasterIdLst", "handoutMasterIdLst", "sldIdLst", "sldSz", "notesSz", "smartTags", "embeddedFontLst", "custShowLst",
                                    "photoAlbum", "custDataLst", "kinsoku", "defaultTextStyle", "modifyVerifier", "extLst"}), qPrintable(names.join(' ')));
        for (const char *required : {"sldMasterIdLst", "sldIdLst", "sldSz", "notesSz", "defaultTextStyle"}) QVERIFY2(names.contains(QLatin1String(required)), required);
        QVERIFY2(inSequence(childNames(e.zip.value(QStringLiteral("ppt/slideMasters/slideMaster1.xml"))), {"cSld", "clrMap", "sldLayoutIdLst", "transition", "timing", "hf", "txStyles", "extLst"}), "master");
        QCOMPARE(childNames(e.zip.value(QStringLiteral("ppt/slideMasters/slideMaster1.xml"))).mid(0, 2), (QStringList{"cSld", "clrMap"}));
        for (const QString &l : {QStringLiteral("ppt/slideLayouts/slideLayout1.xml"), QStringLiteral("ppt/slideLayouts/slideLayout2.xml")}) {
            names = childNames(e.zip.value(l));
            QVERIFY2(inSequence(names, {"cSld", "clrMapOvr", "transition", "timing", "hf", "extLst"}), qPrintable(l + ": " + names.join(' ')));
            QCOMPARE(names, (QStringList{"cSld", "clrMapOvr"}));
        }
        for (const QString &s : {QStringLiteral("ppt/slides/slide1.xml"), QStringLiteral("ppt/slides/slide2.xml")}) {
            names = childNames(e.zip.value(s));
            QVERIFY2(inSequence(names, {"cSld", "clrMapOvr", "transition", "timing", "extLst"}), qPrintable(s + ": " + names.join(' ')));
        }
        // The first slide's transition and timing follow its color mapping.
        QCOMPARE(childNames(e.zip.value(QStringLiteral("ppt/slides/slide1.xml"))), (QStringList{"cSld", "clrMapOvr", "transition", "timing"}));
        QCOMPARE(childNames(e.zip.value(QStringLiteral("ppt/slides/slide2.xml"))), (QStringList{"cSld", "clrMapOvr"}));
        QVERIFY(inSequence(childNames(e.zip.value(QStringLiteral("ppt/notesSlides/notesSlide1.xml"))), {"cSld", "clrMapOvr", "extLst"}));
        QVERIFY(inSequence(childNames(e.zip.value(QStringLiteral("ppt/notesMasters/notesMaster1.xml"))), {"cSld", "clrMap", "hf", "notesStyle", "extLst"}));
        // A shape tree starts with its group's properties, after the background.
        for (const QString &s : {QStringLiteral("ppt/slides/slide1.xml"), QStringLiteral("ppt/slides/slide2.xml"), QStringLiteral("ppt/slideMasters/slideMaster1.xml"), QStringLiteral("ppt/slideLayouts/slideLayout1.xml")}) {
            QDomDocument dom;
            const QDomElement cSld = office::child(rootOf(e.zip.value(s), &dom), office::ns::P, QStringLiteral("cSld"));
            QStringList inside;
            for (const QDomElement &c : office::children(cSld)) inside << c.localName();
            QVERIFY2(inSequence(inside, {"bg", "spTree", "custDataLst", "controls", "extLst"}) && inside.contains(QStringLiteral("spTree")), qPrintable(s + ": " + inside.join(' ')));
            const QDomElement tree = office::child(cSld, office::ns::P, QStringLiteral("spTree"));
            const QVector<QDomElement> kids = office::children(tree);
            QVERIFY(kids.size() >= 2);
            QCOMPARE(kids[0].localName(), QStringLiteral("nvGrpSpPr"));
            QCOMPARE(kids[1].localName(), QStringLiteral("grpSpPr"));
        }

        // The ids: masters and layouts share one range from 2147483648 and none repeats; slides count from 256.
        QDomDocument dom;
        const QDomElement root = rootOf(pres, &dom);
        QSet<qint64> ids;
        for (const QDomElement &m : office::children(office::child(root, office::ns::P, QStringLiteral("sldMasterIdLst")))) ids << office::attr(m, QStringLiteral("id")).toLongLong();
        QDomDocument masterDom;
        const QDomElement master = rootOf(e.zip.value(QStringLiteral("ppt/slideMasters/slideMaster1.xml")), &masterDom);
        const QVector<QDomElement> layoutIds = office::children(office::child(master, office::ns::P, QStringLiteral("sldLayoutIdLst")));
        QCOMPARE(int(layoutIds.size()), 2);
        for (const QDomElement &l : layoutIds) ids << office::attr(l, QStringLiteral("id")).toLongLong();
        QCOMPARE(int(ids.size()), 3);
        for (qint64 id : std::as_const(ids)) QVERIFY(id >= 2147483648LL && id <= 4294967295LL);
        QStringList slideIds;
        for (const QDomElement &s : office::children(office::child(root, office::ns::P, QStringLiteral("sldIdLst")))) slideIds << office::attr(s, QStringLiteral("id"));
        QCOMPARE(slideIds, (QStringList{"256", "257"}));
        // The master's layouts are the ones the layout parts are, in order.
        QCOMPARE(layoutIds[0].attributeNS(QString::fromLatin1(office::ns::R.data()), QStringLiteral("id")), e.pkg.relationshipsOfType(QStringLiteral("/ppt/slideMasters/slideMaster1.xml"), QStringLiteral("/slideLayout"))[0].id);

        // The size: widescreen says no type; the standard 4:3 slide says screen4x3.
        QDomElement sz = office::child(root, office::ns::P, QStringLiteral("sldSz"));
        QCOMPARE(sz.attribute(QStringLiteral("cx")), QStringLiteral("12192000"));
        QCOMPARE(sz.attribute(QStringLiteral("cy")), QStringLiteral("6858000"));
        QVERIFY(!sz.hasAttribute(QStringLiteral("type")));
        const QDomElement notesSz = office::child(root, office::ns::P, QStringLiteral("notesSz"));
        QCOMPARE(notesSz.attribute(QStringLiteral("cx")), QStringLiteral("6858000"));
        QCOMPARE(notesSz.attribute(QStringLiteral("cy")), QStringLiteral("9144000"));
        deck::setSlideSize(*d, QSizeF(deck::kStandardWidth, deck::kStandardHeight));
        Exported small;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &small, &err), qPrintable(err));
        QDomDocument smallDom;
        sz = office::child(rootOf(small.zip.value(QStringLiteral("ppt/presentation.xml")), &smallDom), office::ns::P, QStringLiteral("sldSz"));
        QCOMPARE(sz.attribute(QStringLiteral("cx")), QStringLiteral("9144000"));
        QCOMPARE(sz.attribute(QStringLiteral("cy")), QStringLiteral("6858000"));
        QCOMPARE(sz.attribute(QStringLiteral("type")), QStringLiteral("screen4x3"));
        QVERIFY2(packageProblem(small).isEmpty(), qPrintable(packageProblem(small)));
    }

    // Exported and read back, the slides are the same slides: their order,
    // layouts, hidden flags, and backgrounds, the notes, and what the reader
    // keeps (transition, timing, frames) as it was, and the shapes and the text in them.
    void exportRoundTripKeepsTheSlides()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));

        QCOMPARE(back->pageSize(), d->pageSize());
        QCOMPARE(back->setup.sizeName, d->setup.sizeName);
        QCOMPARE(int(back->pages.size()), 2);
        QCOMPARE(deck::layouts(*back).size(), deck::layouts(*d).size());
        for (int i = 0; i < 2; ++i) {
            QCOMPARE(back->masterFor(*back->pages[i])->name, d->masterFor(*d->pages[i])->name);
            QCOMPARE(deck::layoutType(*back->masterFor(*back->pages[i])), deck::layoutType(*d->masterFor(*d->pages[i])));
            QCOMPARE(back->pages[i]->hidden, d->pages[i]->hidden);
            QCOMPARE(back->pages[i]->showMasterItems, d->pages[i]->showMasterItems);
        }
        QVERIFY(!back->pages[0]->hidden);
        QVERIFY(back->pages[1]->hidden);
        QVERIFY(!back->pages[1]->showMasterItems);
        QCOMPARE(back->pages[1]->background.type, Fill::Solid);
        QCOMPARE(back->pages[1]->background.color.rgbValue(), QColor(0x11, 0x22, 0x33));
        QCOMPARE(back->pages[0]->background.type, Fill::NoFill);
        // The layouts keep their names, and the Title Slide layout hides the master's graphics.
        QCOMPARE(back->masters[1]->name, QStringLiteral("Title Slide"));
        QVERIFY(!back->masters[1]->showMasterItems);
        QCOMPARE(back->masters[2]->name, QStringLiteral("Title and Content"));
        QVERIFY(back->masters[2]->showMasterItems);
        QCOMPARE(back->masters[0]->name, d->masters[0]->name);

        // What the reader keeps comes back as it was kept.
        QCOMPARE(back->pages[0]->props.value(QStringLiteral("transitionXml")).toString(), d->pages[0]->props.value(QStringLiteral("transitionXml")).toString());
        QVERIFY(!back->pages[0]->props.value(QStringLiteral("transitionXml")).toString().isEmpty());
        QCOMPARE(back->pages[0]->props.value(QStringLiteral("timingXml")).toString(), d->pages[0]->props.value(QStringLiteral("timingXml")).toString());
        QVERIFY(!back->pages[0]->props.value(QStringLiteral("timingXml")).toString().isEmpty());
        QVERIFY(!back->pages[1]->props.contains(QStringLiteral("transitionXml")));
        const QJsonArray was = d->pages[0]->props.value(QStringLiteral("unread")).toArray(), now = back->pages[0]->props.value(QStringLiteral("unread")).toArray();
        QCOMPARE(int(was.size()), 1);
        QCOMPARE(now, was);
        QCOMPARE(back->appData.value(QStringLiteral("themeXml")), d->appData.value(QStringLiteral("themeXml")));
        QCOMPARE(back->appData.value(QStringLiteral("notesSize")), d->appData.value(QStringLiteral("notesSize")));
        QCOMPARE(deck::themeOf(*back).colors[4], deck::themeOf(*d).colors[4]);

        // The notes: a page for the first slide, none for the second.
        QCOMPARE(int(back->pages[0]->notesStoryId.isEmpty()), 0);
        QCOMPARE(deck::notesText(*back, *back->pages[0]), QStringLiteral("Say hello first\nThen the numbers"));
        QVERIFY(deck::notesText(*back, *back->pages[1]).isEmpty());
        // The shapes: the placeholders with their roles, the text, the picture.
        QCOMPARE(int(back->pages[0]->items.size()), int(d->pages[0]->items.size()));
        {
            QStringList roles, backRoles;
            for (const auto &it : d->pages[0]->items) roles << it->role;
            for (const auto &it : back->pages[0]->items) backRoles << it->role;
            QCOMPARE(backRoles, roles);
        }
        QCOMPARE(back->pages[0]->title, QStringLiteral("Quarterly results"));
    }

    // A new presentation, saved and opened again, is the presentation it was.
    void exportRoundTripOfANewPresentation()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("secHead")));
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("picTx")));
        d->pages[1]->hidden = true;
        deck::setSlideBackground(*d->pages[2], QColor(0x20, 0x40, 0x60));
        d->storyDoc(d->pages[2]->notesStoryId)->setPlainText(QStringLiteral("Third slide"));
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        QCOMPARE(back->pageSize(), QSizeF(960, 540));
        QCOMPARE(back->setup.sizeName, QStringLiteral("Widescreen"));
        QCOMPARE(int(back->pages.size()), 3);
        QStringList types, backTypes;
        for (int i = 0; i < 3; ++i) {
            types << deck::layoutType(*d->masterFor(*d->pages[i]));
            backTypes << deck::layoutType(*back->masterFor(*back->pages[i]));
        }
        QCOMPARE(backTypes, (QStringList{"title", "secHead", "picTx"}));
        QCOMPARE(backTypes, types);
        QCOMPARE(back->pages[1]->hidden, true);
        QCOMPARE(back->pages[2]->background.color.rgbValue(), QColor(0x20, 0x40, 0x60));
        // The nine layouts, over the one master, in their order, with the theme.
        QCOMPARE(int(deck::layouts(*back).size()), 9);
        QStringList layoutTypes, backLayoutTypes;
        for (MasterPage *l : deck::layouts(*d)) layoutTypes << deck::layoutType(*l);
        for (MasterPage *l : deck::layouts(*back)) backLayoutTypes << deck::layoutType(*l);
        QCOMPARE(backLayoutTypes, layoutTypes);
        QCOMPARE(deck::themeOf(*back).name, QStringLiteral("JeffOffice"));
        for (int i = 0; i < 12; ++i) QCOMPARE(deck::themeOf(*back).colors[i], deck::themeOf(*d).colors[i]);
        QCOMPARE(deck::themeOf(*back).headingFont, QStringLiteral("Carlito"));
        QCOMPARE(deck::themeOf(*back).bodyFont, QStringLiteral("Carlito"));
        QCOMPARE(int(back->pages[0]->items.size()), 2);
        QCOMPARE(deck::notesText(*back, *back->pages[2]), QStringLiteral("Third slide"));
    }

    // What the reader kept that points at parts it did not keep, or at shapes
    // whose numbers it did not keep, is left out with a note, never written
    // as a link that leads nowhere.
    void exportLeavesOutWhatWouldPointNowhere()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        const QString chart = QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"9\" name=\"Chart 8\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                                             "<p:xfrm><a:off x=\"914400\" y=\"914400\"/><a:ext cx=\"3657600\" cy=\"1371600\"/></p:xfrm><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/chart\">"
                                             "<c:chart xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\" r:id=\"rId9\"/></a:graphicData></a:graphic></p:graphicFrame>");
        xml.replace(QStringLiteral("</p:spTree>"), chart + QStringLiteral("</p:spTree>"));
        // An animation that aims at a shape that is not there, and a transition with a sound.
        xml.replace(QRegularExpression(QStringLiteral("<p:timing>.*</p:timing>")),
                    QStringLiteral("<p:timing><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\"><p:childLst><p:set><p:cBhvr><p:cTn id=\"2\"/><p:tgtEl><p:spTgt spid=\"99\"/></p:tgtEl></p:cBhvr></p:set></p:childLst></p:cTn></p:par></p:tnLst></p:timing>"));
        xml.replace(QStringLiteral("<p:transition spd=\"med\"><p:fade/></p:transition>"),
                    QStringLiteral("<p:transition spd=\"med\"><p:fade/><p:sndAc><p:stSnd><p:snd r:embed=\"rId8\" name=\"chime.wav\"/></p:stSnd></p:sndAc></p:transition>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages[0]->props.value(QStringLiteral("unread")).toArray().size()), 2);   // the unknown frame, and the chart

        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(slide.contains(QStringLiteral("Cell &amp; &lt;1&gt;")));   // the frame has nothing to point at, and stays
        QVERIFY(!slide.contains(QStringLiteral("c:chart")));
        QVERIFY(!slide.contains(QStringLiteral("rId9")));
        QVERIFY(!slide.contains(QStringLiteral("<p:timing")));
        QVERIFY(!slide.contains(QStringLiteral("chime.wav")));
        QVERIFY(!slide.contains(QStringLiteral("<p:transition")));
        int notes = 0;
        for (const QString &s : std::as_const(skipped))
            if (s.startsWith(QLatin1String("Slide 1"))) ++notes;
        QVERIFY2(notes >= 3, qPrintable(skipped.join('\n')));   // the chart, the animation, the transition
        // The shape number of the frame is not given to anything else on the slide.
        QDomDocument dom;
        QVERIFY(office::parseXml(slide.toUtf8(), &dom));
        QStringList ids;
        QVector<QDomElement> stack{dom.documentElement()};
        while (!stack.isEmpty()) {
            const QDomElement el = stack.takeLast();
            if (el.localName() == QLatin1String("cNvPr")) ids << el.attribute(QStringLiteral("id"));
            for (QDomElement c = el.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack << c;
        }
        QVERIFY(ids.contains(QStringLiteral("8")));
        QCOMPARE(int(QSet<QString>(ids.cbegin(), ids.cend()).size()), int(ids.size()));

        // An animation that aims at a frame that is kept, by the number the frame keeps, stays.
        QJsonObject props = d->pages[0]->props;
        props[QStringLiteral("timingXml")] = QStringLiteral("<p:timing xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" nodeType=\"tmRoot\"><p:childTnLst>"
                                                           "<p:set><p:cBhvr><p:cTn id=\"2\"/><p:tgtEl><p:spTgt spid=\"8\"/></p:tgtEl></p:cBhvr></p:set></p:childTnLst></p:cTn></p:par></p:tnLst></p:timing>");
        props.remove(QStringLiteral("transitionXml"));
        d->pages[0]->props = props;
        const QByteArray kept = deck::exportPresentation(*d, &err);
        QVERIFY2(!kept.isEmpty(), qPrintable(err));
        Exported k;
        QVERIFY2(unpack(kept, &k, &err), qPrintable(err));
        QVERIFY(part(k, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<p:spTgt spid=\"8\"/>")));
    }

    // A shape read from a file is written with the number it had, so an animation
    // that aims at it stays; one that is gone, and the new and copied ones, take
    // no number an animation could still aim at.
    void exportKeepsAnimationsOnShapesThatKeptTheirNumbers()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        QMap<QString, int> numbers;
        for (const auto &it : d->pages[0]->items) numbers.insert(it->name, it->sourceId);
        QCOMPARE(numbers.value(QStringLiteral("TextBox 4")), 5);
        QCOMPARE(numbers.value(QStringLiteral("Rectangle 5")), 6);
        auto timingAt = [](const QStringList &targets) {
            QString sets;
            for (const QString &t : targets)
                sets += QStringLiteral("<p:set><p:cBhvr><p:cTn id=\"%1\"/><p:tgtEl><p:spTgt spid=\"%1\"/></p:tgtEl></p:cBhvr></p:set>").arg(t);
            return QStringLiteral("<p:timing xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" nodeType=\"tmRoot\"><p:childTnLst>")
                + sets + QStringLiteral("</p:childTnLst></p:cTn></p:par></p:tnLst></p:timing>");
        };
        auto numbersIn = [](const QString &slide) {
            QMap<QString, QString> byName;
            QDomDocument dom;
            if (!office::parseXml(slide.toUtf8(), &dom)) return byName;
            QVector<QDomElement> stack{dom.documentElement()};
            while (!stack.isEmpty()) {
                const QDomElement el = stack.takeLast();
                if (el.localName() == QLatin1String("cNvPr")) byName.insert(el.attribute(QStringLiteral("name")), el.attribute(QStringLiteral("id")));
                for (QDomElement c = el.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack << c;
            }
            return byName;
        };
        d->pages[0]->props[QStringLiteral("timingXml")] = timingAt({QStringLiteral("5"), QStringLiteral("6")});
        d->pages[0]->props.remove(QStringLiteral("transitionXml"));
        QStringList skipped;
        QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(slide.contains(QStringLiteral("<p:spTgt spid=\"5\"/>")) && slide.contains(QStringLiteral("<p:spTgt spid=\"6\"/>")));
        QCOMPARE(numbersIn(slide).value(QStringLiteral("TextBox 4")), QStringLiteral("5"));
        QCOMPARE(numbersIn(slide).value(QStringLiteral("Rectangle 5")), QStringLiteral("6"));
        // And once read again they still have them.
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QMap<QString, int> backNumbers;
        for (const auto &it : back->pages[0]->items) backNumbers.insert(it->name, it->sourceId);
        QCOMPARE(backNumbers.value(QStringLiteral("TextBox 4")), 5);
        QCOMPARE(backNumbers.value(QStringLiteral("Rectangle 5")), 6);

        // A shape added, and a copy of one, are written with numbers of their own.
        auto copy = d->pages[0]->items[3]->clone();
        copy->name = QStringLiteral("Copy");
        auto added = std::make_shared<ShapeItem>();
        added->name = QStringLiteral("Added");
        added->rect = QRectF(10, 10, 20, 20);
        d->pages[0]->items.push_back(copy);
        d->pages[0]->items.push_back(added);
        QVERIFY2(!(bytes = deck::exportPresentation(*d, &err)).isEmpty(), qPrintable(err));
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(slide.contains(QStringLiteral("<p:spTgt spid=\"5\"/>")));
        const QMap<QString, QString> ids = numbersIn(slide);
        QVERIFY(ids.value(QStringLiteral("Copy")) != QStringLiteral("5") && ids.value(QStringLiteral("Copy")) != QStringLiteral("6"));
        QVERIFY(!ids.value(QStringLiteral("Added")).isEmpty());
        QCOMPARE(int(QSet<QString>(ids.cbegin(), ids.cend()).size()), int(ids.size()));

        // With the shape an animation aims at gone, it goes with a note.
        for (auto it = d->pages[0]->items.begin(); it != d->pages[0]->items.end(); ++it)
            if ((*it)->name == QLatin1String("Rectangle 5")) {
                d->pages[0]->items.erase(it);
                break;
            }
        QVERIFY2(!(bytes = deck::exportPresentation(*d, &err, &skipped)).isEmpty(), qPrintable(err));
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY(!part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<p:timing")));
        bool noted = false;
        for (const QString &s : std::as_const(skipped)) noted |= s.startsWith(QLatin1String("Slide 1")) && s.contains(QStringLiteral("animation"));
        QVERIFY2(noted, qPrintable(skipped.join('\n')));
    }

    // A movie and a sound a slide plays are kept: the copy the file holds is
    // written back byte for byte with its relationships, one outside stays a
    // link, and the timing that plays them stays with them. Timing that plays
    // a movie or sound on a shape that has none is left out with a note: the
    // presentation program refuses such a file.
    void exportKeepsMoviesAndSounds()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        auto media = [](int id, const QString &name, const QString &nvPr) {
            return QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"%1\" name=\"%2\"><a:hlinkClick r:id=\"\" action=\"ppaction://media\"/></p:cNvPr><p:cNvPicPr/><p:nvPr>%3</p:nvPr></p:nvPicPr>"
                                  "<p:blipFill><a:blip r:embed=\"rId2\"/><a:stretch><a:fillRect/></a:stretch></p:blipFill><p:spPr>").arg(id).arg(name, nvPr)
                + xfrm(0, 0, 1828800, 1371600) + QStringLiteral("<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></p:spPr></p:pic>");
        };
        xml.replace(QStringLiteral("</p:spTree>"),
                    media(20, QStringLiteral("Movie"), QStringLiteral("<a:videoFile r:link=\"rId4\"/><p:extLst><p:ext uri=\"{DAA4B4D4-6D71-4841-9C94-3DE7FCFB9230}\">"
                                                                      "<p14:media xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\" r:embed=\"rId5\"/></p:ext></p:extLst>"))
                        + media(21, QStringLiteral("Sound"), QStringLiteral("<a:audioFile r:link=\"rId6\"/>")) + QStringLiteral("</p:spTree>"));
        auto plays = [](const char *kind, int id, int spid) {
            return QStringLiteral("<p:%1><p:cMediaNode vol=\"80000\"><p:cTn id=\"%2\" fill=\"hold\" display=\"0\"><p:stCondLst><p:cond delay=\"indefinite\"/></p:stCondLst></p:cTn>"
                                  "<p:tgtEl><p:spTgt spid=\"%3\"/></p:tgtEl></p:cMediaNode></p:%1>").arg(QLatin1String(kind)).arg(id).arg(spid);
        };
        xml.replace(QRegularExpression(QStringLiteral("<p:timing>.*</p:timing>")),
                    QStringLiteral("<p:timing><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\"><p:childTnLst>")
                        + plays("video", 2, 20) + plays("audio", 3, 21) + QStringLiteral("</p:childTnLst></p:cTn></p:par></p:tnLst></p:timing>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        const QByteArray movie("not really a movie, but kept byte for byte");
        p.data["/ppt/media/media1.mp4"] = {QStringLiteral("video/mp4"), movie};
        p.rels << Rel{"/ppt/slides/slide1.xml", "video", "/ppt/media/media1.mp4"}
               << Rel{"/ppt/slides/slide1.xml", "http://schemas.microsoft.com/office/2007/relationships/media", "/ppt/media/media1.mp4"}
               << Rel{"/ppt/slides/slide1.xml", "audio", "file:///C:/Sounds/applause.wav", true};

        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        auto named = [](const Document &doc, const QString &name) -> const PictureItem * {
            for (const auto &it : doc.pages[0]->items)
                if (it->name == name && it->type() == ItemType::Picture) return static_cast<const PictureItem *>(it.get());
            return nullptr;
        };
        const PictureItem *mv = named(*d, QStringLiteral("Movie")), *snd = named(*d, QStringLiteral("Sound"));
        QVERIFY(mv && snd);
        QCOMPARE(mv->mediaKind, QStringLiteral("video"));
        QVERIFY(d->media.contains(mv->mediaId));
        QCOMPARE(d->media.value(mv->mediaId).bytes, movie);
        QCOMPARE(d->media.value(mv->mediaId).contentType, QStringLiteral("video/mp4"));
        QCOMPARE(d->media.value(mv->mediaId).extension, QStringLiteral("mp4"));
        QCOMPARE(snd->mediaKind, QStringLiteral("audio"));
        QVERIFY(snd->mediaId.isEmpty());
        QCOMPARE(snd->mediaLink, QStringLiteral("file:///C:/Sounds/applause.wav"));

        QStringList skipped;
        QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(slide.contains(QStringLiteral("<a:videoFile r:link=")));
        QVERIFY(slide.contains(QStringLiteral("<p14:media xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\" r:embed=")));
        QVERIFY(slide.contains(QStringLiteral("<a:audioFile r:link=")));
        QCOMPARE(int(slide.count(QStringLiteral("action=\"ppaction://media\""))), 2);
        QVERIFY(slide.contains(QStringLiteral("<p:video>")) && slide.contains(QStringLiteral("<p:spTgt spid=\"20\"/>")));
        QVERIFY(slide.contains(QStringLiteral("<p:audio>")) && slide.contains(QStringLiteral("<p:spTgt spid=\"21\"/>")));
        QString moviePart;
        bool linked = false, copied = false;
        for (const office::Relationship &r : e.pkg.relationships(QStringLiteral("/ppt/slides/slide1.xml"))) {
            if (r.type.endsWith(QLatin1String("/relationships/video")) && !r.external) moviePart = r.target;
            if (r.type == QLatin1String("http://schemas.microsoft.com/office/2007/relationships/media")) copied = !r.external;
            if (r.type.endsWith(QLatin1String("/relationships/audio"))) linked = r.external && r.target == QLatin1String("file:///C:/Sounds/applause.wav");
        }
        QVERIFY(copied && linked);
        QVERIFY(moviePart.endsWith(QLatin1String(".mp4")));
        QCOMPARE(e.pkg.part(moviePart), movie);
        QCOMPARE(e.pkg.contentType(moviePart), QStringLiteral("video/mp4"));

        // Read again, it is the same movie.
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const PictureItem *again = named(*back, QStringLiteral("Movie"));
        QVERIFY(again && back->media.contains(again->mediaId));
        QCOMPARE(back->media.value(again->mediaId).bytes, movie);

        // A movie's picture that plays nothing any more takes the timing that plays it with it.
        for (const auto &it : d->pages[0]->items)
            if (it->name == QLatin1String("Movie")) static_cast<PictureItem &>(*it).mediaKind.clear();
        skipped.clear();
        QVERIFY2(!(bytes = deck::exportPresentation(*d, &err, &skipped)).isEmpty(), qPrintable(err));
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(!slide.contains(QStringLiteral("<p:timing")));
        QVERIFY(!slide.contains(QStringLiteral("<a:videoFile")));
        bool noted = false;
        for (const QString &n : std::as_const(skipped)) noted |= n.startsWith(QLatin1String("Slide 1")) && n.contains(QStringLiteral("movie or sound"));
        QVERIFY2(noted, qPrintable(skipped.join('\n')));
    }

    // What a person makes in the editor is written as it looks: scheme colors as
    // the presentation's own, shapes as the standard's presets, a picture used
    // twice once, lines with their ends and what they are attached to, and
    // groups with what is in them.
    void exportWritesWhatTheEditorMakes()
    {
        auto d = deck::newPresentation();
        d->colors.c[jp::Accent1] = QColor(0x12, 0x34, 0x56);
        const int at = deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("titleOnly")));
        Page &slide = *d->pages[at];
        const size_t placeholders = slide.items.size();
        auto box = std::make_shared<TextItem>();
        box->rect = QRectF(50, 150, 300, 60);
        box->storyId = d->createStory(QStringLiteral("Hello\nWorld"));
        {
            QTextCursor c(d->storyDoc(box->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat cf;
            cf.setFontPointSize(20);
            cf.setProperty(tp::ColorRefP, ColorRef::scheme(jp::Accent1).toString());
            c.mergeCharFormat(cf);
        }
        auto arrow = std::make_shared<ShapeItem>();
        arrow->rect = QRectF(400, 150, 200, 80);
        arrow->shape = QStringLiteral("rightArrow");
        arrow->adj = {0.4, 0.6};
        arrow->fill = Fill::solid(ColorRef::scheme(jp::Accent1));
        arrow->stroke = Stroke::none();
        auto circle = std::make_shared<ShapeItem>();
        circle->rect = QRectF(550, 300, 60, 60);
        circle->shape = QStringLiteral("ellipse");
        auto star = std::make_shared<ShapeItem>();
        star->rect = QRectF(400, 300, 100, 100);
        star->shape = QStringLiteral("star5");
        QImage image(4, 4, QImage::Format_RGB32);
        image.fill(Qt::green);
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        const QString imageId = d->addImage(png, QStringLiteral("png"), QString());
        auto first = std::make_shared<PictureItem>(), second = std::make_shared<PictureItem>();
        for (const auto &p : {first, second}) {
            p->imageId = imageId;
            p->imgRect = QRectF(0, 0, 100, 50);
            p->rect = QRectF(50, 300, 100, 50);
        }
        second->rect.moveTo(200, 300);
        auto line = std::make_shared<LineItem>();
        line->p1 = QPointF(350, 180);
        line->p2 = QPointF(400, 190);
        line->start = LineItem::Glue{box->id, 3};
        line->end = LineItem::Glue{circle->id, 2};
        line->stroke.endArrow = Arrow::Triangle;
        line->syncRect();
        auto group = std::make_shared<GroupItem>();
        group->children = {star, first};
        group->syncRect();
        auto table = std::make_shared<jp::TableItem>();
        table->rect = QRectF(50, 400, 200, 60);
        table->rows = 2;
        table->cols = 2;
        table->colW = {100, 100};
        table->rowH = {30, 30};
        table->cells.resize(4);
        for (jp::TableCell &cell : table->cells) {
            cell.storyId = d->createStory(QStringLiteral("cell"));
            cell.fill = Fill::solid(ColorRef::scheme(jp::Accent1, 60));
            cell.border.top = cell.border.bottom = cell.border.left = cell.border.right = Stroke::line(ColorRef::scheme(jp::Main), 1);
        }
        slide.items.push_back(table);
        slide.items.push_back(box);
        slide.items.push_back(arrow);
        slide.items.push_back(circle);
        slide.items.push_back(second);
        slide.items.push_back(line);
        slide.items.push_back(group);

        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.isEmpty(), qPrintable(skipped.join('\n')));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        int media = 0;
        for (const QString &name : e.pkg.partNames()) media += name.startsWith(QLatin1String("/ppt/media/"));
        QCOMPARE(media, 1);

        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const Page &page = *back->pages[at];
        QCOMPARE(page.items.size(), placeholders + 7);
        // The table comes back as a table, first, as it was written.
        QCOMPARE(page.items[placeholders]->type(), ItemType::Table);
        QVERIFY(page.props.value(QStringLiteral("unread")).toArray().isEmpty());
        const auto *text = static_cast<TextItem *>(page.items[placeholders + 1].get());
        QCOMPARE(text->storyId.isEmpty(), false);
        QCOMPARE(back->storyDoc(text->storyId)->toPlainText(), QStringLiteral("Hello\nWorld"));
        QCOMPARE(back->storyDoc(text->storyId)->begin().begin().fragment().charFormat().stringProperty(tp::ColorRefP), QStringLiteral("#123456"));
        const auto *shape = static_cast<ShapeItem *>(page.items[placeholders + 2].get());
        QCOMPARE(shape->shape, QStringLiteral("rightArrow"));
        QCOMPARE(shape->fill.color.rgbValue(), QColor(0x12, 0x34, 0x56));
        QVERIFY(shape->stroke.isNone());
        QCOMPARE(shape->adj.size(), 2);
        QVERIFY(std::abs(shape->adj[0] - 0.4) < 1e-4 && std::abs(shape->adj[1] - 0.6) < 1e-4);
        QCOMPARE(page.items[placeholders + 4]->type(), ItemType::Picture);
        const auto *connector = static_cast<LineItem *>(page.items[placeholders + 5].get());
        QVERIFY(connector->stroke.endArrow == Arrow::Triangle);
        // The connector names the numbers of the two shapes it is attached to (the importer reads each shape alone, so what it joins is not read back).
        QDomDocument dom;
        QVERIFY(office::parseXml(part(e, QStringLiteral("/ppt/slides/slide%1.xml").arg(at + 1)).toUtf8(), &dom));
        QStringList numbers;
        for (QDomElement el = office::child(office::child(office::child(dom.documentElement(), office::ns::P, QStringLiteral("cSld")), office::ns::P, QStringLiteral("spTree")),
                                            office::ns::P, QStringLiteral("sp"));
             !el.isNull(); el = el.nextSiblingElement(QStringLiteral("sp")))
            numbers << office::child(office::child(el, office::ns::P, QStringLiteral("nvSpPr")), office::ns::P, QStringLiteral("cNvPr")).attribute(QStringLiteral("id"));
        const QDomElement cxn = office::child(office::child(office::child(office::child(dom.documentElement(), office::ns::P, QStringLiteral("cSld")), office::ns::P, QStringLiteral("spTree")),
                                                            office::ns::P, QStringLiteral("cxnSp")), office::ns::P, QStringLiteral("nvCxnSpPr"));
        const QDomElement props = office::child(cxn, office::ns::P, QStringLiteral("cNvCxnSpPr"));
        const QDomElement start = office::child(props, office::ns::A, QStringLiteral("stCxn")), end = office::child(props, office::ns::A, QStringLiteral("endCxn"));
        QCOMPARE(int(numbers.size()), int(placeholders) + 3);
        QCOMPARE(start.attribute(QStringLiteral("id")), numbers[placeholders]);
        QCOMPARE(start.attribute(QStringLiteral("idx")), QStringLiteral("3"));
        QCOMPARE(end.attribute(QStringLiteral("id")), numbers[placeholders + 2]);
        QCOMPARE(end.attribute(QStringLiteral("idx")), QStringLiteral("4"));   // the bottom of an ellipse is its fifth point
        const auto *grouped = static_cast<GroupItem *>(page.items[placeholders + 6].get());
        QCOMPARE(grouped->children.size(), size_t(2));
        QCOMPARE(static_cast<ShapeItem *>(grouped->children[0].get())->shape, QStringLiteral("star5"));
        QCOMPARE(static_cast<PictureItem *>(grouped->children[1].get())->imageId, static_cast<PictureItem *>(page.items[placeholders + 4].get())->imageId);
    }

    // Slide masters beyond the first keep their own themes, and every master and
    // layout has an id of its own.
    void exportWritesEachMastersTheme()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        auto second = std::make_shared<MasterPage>(*d->masters[0]);
        second->id = QStringLiteral("SM2");
        second->items.clear();
        second->props[QStringLiteral("themeXml")] = QString::fromLatin1(QByteArray(themeXml()).replace("Test Theme", "Second Theme").toBase64());
        auto layout = std::make_shared<MasterPage>();
        layout->id = QStringLiteral("L3");
        layout->name = QStringLiteral("Only Title");
        layout->baseId = second->id;
        layout->props[QStringLiteral("kind")] = QStringLiteral("layout");
        layout->props[QStringLiteral("type")] = QStringLiteral("cust");
        d->masters << second << layout;
        auto slide = std::make_shared<Page>();
        slide->masterId = layout->id;
        slide->notesStoryId = d->createStory();
        d->pages << slide;

        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QCOMPARE(e.pkg.relationshipsOfType(QStringLiteral("/ppt/presentation.xml"), QStringLiteral("/slideMaster")).size(), 2);
        // The first master's theme is the kept part as it was; the second's is its own; the notes master has a third.
        QCOMPARE(e.zip.value(QStringLiteral("ppt/theme/theme1.xml")), themeXml());
        QVERIFY(part(e, QStringLiteral("/ppt/theme/theme2.xml")).contains(QStringLiteral("Second Theme")));
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/slideMasters/slideMaster2.xml"), QStringLiteral("/theme")), QStringList{QStringLiteral("/ppt/theme/theme2.xml")});
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/slideMasters/slideMaster1.xml"), QStringLiteral("/theme")), QStringList{QStringLiteral("/ppt/theme/theme1.xml")});
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/notesMasters/notesMaster1.xml"), QStringLiteral("/theme")), QStringList{QStringLiteral("/ppt/theme/theme3.xml")});
        // The layouts: two on the first master, one on the second, numbered across.
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/slideMasters/slideMaster2.xml"), QStringLiteral("/slideLayout")), QStringList{QStringLiteral("/ppt/slideLayouts/slideLayout3.xml")});
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/slides/slide3.xml"), QStringLiteral("/slideLayout")), QStringList{QStringLiteral("/ppt/slideLayouts/slideLayout3.xml")});
        // A custom layout says no type; the master's ids and the layouts' ids do not repeat.
        QDomDocument layoutDom;
        QVERIFY(!rootOf(e.zip.value(QStringLiteral("ppt/slideLayouts/slideLayout3.xml")), &layoutDom).hasAttribute(QStringLiteral("type")));
        QDomDocument presDom;
        const QDomElement pres = rootOf(e.zip.value(QStringLiteral("ppt/presentation.xml")), &presDom);
        QSet<QString> ids;
        int count = 0;
        for (const QDomElement &m : office::children(office::child(pres, office::ns::P, QStringLiteral("sldMasterIdLst")))) ids << office::attr(m, QStringLiteral("id")), ++count;
        for (const QString &m : {QStringLiteral("ppt/slideMasters/slideMaster1.xml"), QStringLiteral("ppt/slideMasters/slideMaster2.xml")}) {
            QDomDocument md;
            for (const QDomElement &l : office::children(office::child(rootOf(e.zip.value(m), &md), office::ns::P, QStringLiteral("sldLayoutIdLst")))) ids << office::attr(l, QStringLiteral("id")), ++count;
        }
        QCOMPARE(count, 5);
        QCOMPARE(int(ids.size()), 5);
        // Read back: two masters, the second with its theme.
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), 3);
        int masters = 0;
        for (const auto &m : back->masters) masters += deck::isSlideMaster(*m);
        QCOMPARE(masters, 2);
        QCOMPARE(back->masterFor(*back->pages[2])->name, QStringLiteral("Only Title"));
        QVERIFY(back->masters[3]->props.contains(QStringLiteral("themeXml")));
    }

    // The master's text styles say what its title and text placeholders look
    // like: sizes, the theme's fonts, bullets, and indents, level by level.
    void exportWritesTheMastersTextStyles()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QDomDocument dom;
        const QDomElement styles = office::child(rootOf(e.zip.value(QStringLiteral("ppt/slideMasters/slideMaster1.xml")), &dom), office::ns::P, QStringLiteral("txStyles"));
        QStringList kinds;
        for (const QDomElement &c : office::children(styles)) kinds << c.localName();
        QCOMPARE(kinds, (QStringList{"titleStyle", "bodyStyle", "otherStyle"}));
        const QDomElement title = office::child(office::child(styles, office::ns::P, QStringLiteral("titleStyle")), office::ns::A, QStringLiteral("lvl1pPr"));
        QCOMPARE(office::attr(title, QStringLiteral("algn")), QStringLiteral("l"));
        const QDomElement titleRun = office::child(title, office::ns::A, QStringLiteral("defRPr"));
        QCOMPARE(office::attr(titleRun, QStringLiteral("sz")), QStringLiteral("4400"));
        QCOMPARE(office::attr(office::child(titleRun, office::ns::A, QStringLiteral("latin")), QStringLiteral("typeface")), QStringLiteral("+mj-lt"));
        QVERIFY(!office::child(title, office::ns::A, QStringLiteral("buNone")).isNull());
        // The text style has nine levels, none larger than the one before; the first is bulleted as the master's text is.
        const QDomElement body = office::child(styles, office::ns::P, QStringLiteral("bodyStyle"));
        QCOMPARE(int(office::children(body).size()), 9);
        const QDomElement first = office::child(body, office::ns::A, QStringLiteral("lvl1pPr"));
        QCOMPARE(office::attr(first, QStringLiteral("marL")), QStringLiteral("228600"));
        QCOMPARE(office::attr(first, QStringLiteral("indent")), QStringLiteral("-228600"));
        QCOMPARE(office::attr(office::child(first, office::ns::A, QStringLiteral("buChar")), QStringLiteral("char")), QStringLiteral("\u2022"));
        QCOMPARE(office::attr(office::child(first, office::ns::A, QStringLiteral("defRPr")), QStringLiteral("sz")), QStringLiteral("2800"));
        QStringList inLevel;
        for (const QDomElement &c : office::children(first)) inLevel << c.localName();
        QVERIFY2(inSequence(inLevel, {"lnSpc", "spcBef", "spcAft", "buClrTx", "buClr", "buSzTx", "buSzPct", "buSzPts", "buFontTx", "buFont", "buNone", "buAutoNum", "buChar", "buBlip", "tabLst", "defRPr", "extLst"}),
                 qPrintable(inLevel.join(' ')));
        int previous = 100000;
        for (const QDomElement &level : office::children(body)) {
            const int size = office::attr(office::child(level, office::ns::A, QStringLiteral("defRPr")), QStringLiteral("sz")).toInt();
            QVERIFY(size > 0 && size <= previous);
            previous = size;
        }
        // A presentation made here has no bullets in its text, and its master says so.
        auto fresh = deck::newPresentation();
        Exported n;
        QVERIFY2(unpack(deck::exportPresentation(*fresh, &err), &n, &err), qPrintable(err));
        QDomDocument freshDom;
        const QDomElement freshStyles = office::child(rootOf(n.zip.value(QStringLiteral("ppt/slideMasters/slideMaster1.xml")), &freshDom), office::ns::P, QStringLiteral("txStyles"));
        for (const QDomElement &level : office::children(office::child(freshStyles, office::ns::P, QStringLiteral("bodyStyle")))) {
            QVERIFY(!office::child(level, office::ns::A, QStringLiteral("buNone")).isNull());
            QVERIFY(office::child(level, office::ns::A, QStringLiteral("buChar")).isNull());
        }
    }

    // The theme: kept as the file had it, made from the presentation's colors
    // and fonts when it came from none, and as the presentation says when the
    // colors changed after the file was read.
    void exportWritesTheTheme()
    {
        QString err;
        auto d = deck::newPresentation();
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        office::Theme theme;
        QVERIFY2(office::readTheme(e.zip.value(QStringLiteral("ppt/theme/theme1.xml")), &theme, &err), qPrintable(err));
        const deck::DeckTheme was = deck::themeOf(*d);
        for (int i = 0; i < 12; ++i) QCOMPARE(theme.colors[i], was.colors[i]);
        QCOMPARE(theme.majorLatin, was.headingFont);
        QCOMPARE(theme.minorLatin, was.bodyFont);
        QCOMPARE(theme.name, was.name);

        auto imported = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(imported, qPrintable(err));
        Exported kept;
        QVERIFY2(unpack(deck::exportPresentation(*imported, &err), &kept, &err), qPrintable(err));
        QCOMPARE(kept.zip.value(QStringLiteral("ppt/theme/theme1.xml")), themeXml());
        // A color changed since the file was read is written, not the part kept from before.
        deck::DeckTheme changed = deck::themeOf(*imported);
        changed.colors[4] = QColor(0x00, 0x99, 0x66);
        changed.bodyFont = QStringLiteral("Calibri");
        deck::setTheme(*imported, changed);
        Exported fresh;
        QVERIFY2(unpack(deck::exportPresentation(*imported, &err), &fresh, &err), qPrintable(err));
        QVERIFY2(office::readTheme(fresh.zip.value(QStringLiteral("ppt/theme/theme1.xml")), &theme, &err), qPrintable(err));
        QCOMPARE(theme.colors[4], QColor(0x00, 0x99, 0x66));
        QCOMPARE(theme.minorLatin, QStringLiteral("Calibri"));
        QCOMPARE(theme.majorLatin, QStringLiteral("Georgia"));
        QVERIFY2(packageProblem(fresh).isEmpty(), qPrintable(packageProblem(fresh)));
    }

    // A theme's picture fills (its textures) point at pictures the reader did not keep:
    // written as they were, they would point nowhere, so they become plain fills.
    void exportLeavesOutThePicturesOfATheme()
    {
        Parts p = samplePptx();
        QByteArray theme = p.data["/ppt/theme/theme1.xml"].xml;
        const QByteArray texture =
            "<a:blipFill dpi=\"0\" rotWithShape=\"1\"><a:blip xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" r:embed=\"rId1\">"
            "<a:duotone><a:schemeClr val=\"phClr\"/></a:duotone></a:blip><a:srcRect/><a:tile tx=\"0\" ty=\"0\" sx=\"100000\" sy=\"100000\" flip=\"none\" algn=\"tl\"/></a:blipFill>";
        theme.replace("<a:bgFillStyleLst>", "<a:bgFillStyleLst>" + texture);
        p.data["/ppt/theme/theme1.xml"].xml = theme;
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        // The rest of the part is as it was.
        QCOMPARE(e.zip.value(QStringLiteral("ppt/theme/theme1.xml")), QByteArray(theme).replace(texture, "<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>"));
        office::Theme read;
        QVERIFY2(office::readTheme(e.zip.value(QStringLiteral("ppt/theme/theme1.xml")), &read, &err), qPrintable(err));
        QCOMPARE(read.colors[4], QColor(0xB6, 0x42, 0x0F));
        // The same for the theme of a second slide master, which is kept in its master's properties.
        auto second = std::make_shared<MasterPage>(*d->masters[0]);
        second->id = QStringLiteral("SM2");
        second->items.clear();
        second->props[QStringLiteral("themeXml")] = QString::fromLatin1(theme.toBase64());
        d->masters << second;
        Exported two;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &two, &err), qPrintable(err));
        QVERIFY2(packageProblem(two).isEmpty(), qPrintable(packageProblem(two)));
        QVERIFY(!part(two, QStringLiteral("/ppt/theme/theme2.xml")).contains(QStringLiteral("embed")));
        // A theme that points at something else that isn't kept is made again from its colors and fonts.
        QByteArray odd = QByteArray(theme).replace("<a:objectDefaults/>", "");
        odd.replace("</a:themeElements>", "</a:themeElements><a:objectDefaults><a:spDef><a:spPr/><a:bodyPr/><a:lstStyle/><a:style><a:lnRef idx=\"1\"><a:schemeClr val=\"accent1\"/></a:lnRef>"
                                          "<a:fillRef idx=\"1\"><a:schemeClr val=\"accent1\"/></a:fillRef><a:effectRef idx=\"0\"><a:schemeClr val=\"accent1\"/></a:effectRef>"
                                          "<a:fontRef idx=\"minor\"><a:hlinkClick xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" r:id=\"rId5\"/></a:fontRef></a:style></a:spDef></a:objectDefaults>");
        second->props[QStringLiteral("themeXml")] = QString::fromLatin1(odd.toBase64());
        Exported three;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &three, &err), qPrintable(err));
        QVERIFY2(packageProblem(three).isEmpty(), qPrintable(packageProblem(three)));
        QVERIFY2(office::readTheme(three.zip.value(QStringLiteral("ppt/theme/theme2.xml")), &read, &err), qPrintable(err));
        QCOMPARE(read.colors[4], QColor(0xB6, 0x42, 0x0F));
    }

    // The same presentation as a template or a show differs in one content type.
    void exportKindsDifferInTheMainPartsType()
    {
        auto d = deck::newPresentation();
        QString err;
        const QString base = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.");
        const struct { deck::PresentationKind kind; const char *type; } kinds[] = {
            {deck::PresentationKind::Presentation, "presentation.main+xml"}, {deck::PresentationKind::Template, "template.main+xml"}, {deck::PresentationKind::Show, "slideshow.main+xml"}};
        for (const auto &k : kinds) {
            Exported e;
            QVERIFY2(unpack(deck::exportPresentation(*d, &err, nullptr, k.kind), &e, &err), qPrintable(err));
            QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/presentation.xml")), base + QLatin1String(k.type));
            QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        }
        QCOMPARE(deck::kindOfFileName(QStringLiteral("/tmp/a.pptx")), deck::PresentationKind::Presentation);
        QCOMPARE(deck::kindOfFileName(QStringLiteral("/tmp/a.POTX")), deck::PresentationKind::Template);
        QCOMPARE(deck::kindOfFileName(QStringLiteral("/tmp/a.ppsx")), deck::PresentationKind::Show);
        QCOMPARE(deck::kindOfFileName(QStringLiteral("/tmp/a")), deck::PresentationKind::Presentation);
    }

    // Saving a file replaces the old one only when the new one is whole: a
    // path that is no path, a folder that isn't there, or one that can't be
    // written leave the old file as it was.
    void saveKeepsTheOldFileWhenItCantWriteTheNewOne()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto d = deck::newPresentation();
        const QString path = dir.filePath(QStringLiteral("talk.pptx"));
        QString err;
        QVERIFY2(deck::savePresentationFile(*d, path, &err), qPrintable(err));
        const QByteArray first = fileBytes(path);
        QVERIFY(first.startsWith("PK"));
        auto read = deck::importPresentationFile(path, &err);
        QVERIFY2(read, qPrintable(err));
        QCOMPARE(int(read->pages.size()), 1);

        // Another save of a changed presentation replaces it, and no stray temporary file stays.
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        QVERIFY2(deck::savePresentationFile(*d, path, &err), qPrintable(err));
        QVERIFY(fileBytes(path) != first);
        read = deck::importPresentationFile(path, &err);
        QVERIFY2(read, qPrintable(err));
        QCOMPARE(int(read->pages.size()), 2);
        QCOMPARE(QDir(dir.path()).entryList(QDir::Files).size(), 1);

        // A template and a show are written under their endings.
        QVERIFY2(deck::savePresentationFile(*d, dir.filePath(QStringLiteral("look.potx")), &err), qPrintable(err));
        QVERIFY2(deck::savePresentationFile(*d, dir.filePath(QStringLiteral("show.ppsx")), &err), qPrintable(err));
        Exported t, s;
        QVERIFY2(unpack(fileBytes(dir.filePath(QStringLiteral("look.potx"))), &t, &err), qPrintable(err));
        QVERIFY2(unpack(fileBytes(dir.filePath(QStringLiteral("show.ppsx"))), &s, &err), qPrintable(err));
        QVERIFY(t.pkg.contentType(QStringLiteral("/ppt/presentation.xml")).endsWith(QLatin1String("template.main+xml")));
        QVERIFY(s.pkg.contentType(QStringLiteral("/ppt/presentation.xml")).endsWith(QLatin1String("slideshow.main+xml")));

        const QByteArray second = fileBytes(path);
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        err.clear();
        QVERIFY(!deck::savePresentationFile(*d, dir.filePath(QStringLiteral("missing/talk.pptx")), &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!deck::savePresentationFile(*d, QString(), &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!deck::savePresentationFile(*d, dir.path(), &err));   // a folder is not a file
        QVERIFY(!err.isEmpty());
        QCOMPARE(fileBytes(path), second);
        // A folder that can't be written (a user who may write anywhere can't be kept out, and skips this).
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("locked")));
        const QString locked = dir.filePath(QStringLiteral("locked/talk.pptx"));
        QVERIFY2(deck::savePresentationFile(*d, locked, &err), qPrintable(err));
        const QByteArray lockedBytes = fileBytes(locked);
        QVERIFY(QFile::setPermissions(dir.filePath(QStringLiteral("locked")), QFileDevice::ReadOwner | QFileDevice::ExeOwner));
        if (!QFileInfo(dir.filePath(QStringLiteral("locked"))).isWritable()) {
            err.clear();
            deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
            QVERIFY(!deck::savePresentationFile(*d, locked, &err));
            QVERIFY(!err.isEmpty());
            QCOMPARE(fileBytes(locked), lockedBytes);
        }
        QFile::setPermissions(dir.filePath(QStringLiteral("locked")), QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    }

    // File > Save and Save As: the window writes the file, marks itself saved,
    // and takes its name from the file; a save that fails says so and leaves
    // the presentation unsaved.
    void windowSavesThePresentation()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        Editor *ed = w.editor();
        QVERIFY(w.act(QStringLiteral("file.save")));
        QVERIFY(w.act(QStringLiteral("file.saveAs")));
        QCOMPARE(w.act(QStringLiteral("file.save"))->shortcut(), QKeySequence(QKeySequence::Save));
        QCOMPARE(w.act(QStringLiteral("file.saveAs"))->shortcut(), QKeySequence(Qt::Key_F12));
        w.newSlide();
        QVERIFY(ed->isModified());
        const QString path = dir.filePath(QStringLiteral("talk.pptx"));
        QVERIFY(w.saveTo(path));
        QVERIFY(!ed->isModified());
        QCOMPARE(ed->filePath(), path);
        QCOMPARE(w.windowTitle(), QStringLiteral("talk - JeffDeck"));
        QString err;
        auto back = deck::importPresentationFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), 2);

        // Save, with a name already, writes to it without asking.
        w.newSlide();
        QVERIFY(ed->isModified());
        QVERIFY(w.windowTitle().contains(QLatin1Char('*')));
        w.act(QStringLiteral("file.save"))->trigger();
        QVERIFY(!ed->isModified());
        QVERIFY(!w.windowTitle().contains(QLatin1Char('*')));
        back = deck::importPresentationFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), 3);
        // Notes typed and not yet in the slide are saved too.
        QTest::keyClicks(w.notesPane(), QStringLiteral("Last words"));
        QVERIFY(w.save());
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[ed->currentPage()]), QStringLiteral("Last words"));
        QVERIFY(!ed->isModified());

        // A save that can't be done says so, and the presentation is still unsaved, under the same name.
        w.newSlide();
        const QByteArray before = fileBytes(path);
        answerDialogs({QMessageBox::Ok});
        QVERIFY(!w.saveTo(dir.filePath(QStringLiteral("missing/talk.pptx"))));
        QVERIFY(ed->isModified());
        QCOMPARE(ed->filePath(), path);
        QCOMPARE(fileBytes(path), before);

        // Save As to another name moves the presentation there, even when nothing changed since the last save.
        QVERIFY(w.saveTo(path));
        const QString other = dir.filePath(QStringLiteral("other.pptx"));
        QVERIFY(w.saveTo(other));
        QCOMPARE(ed->filePath(), other);
        QCOMPARE(w.windowTitle(), QStringLiteral("other - JeffDeck"));
        QVERIFY(!ed->isModified());
        ed->markSaved();
    }

    // A presentation opened from a template saves under a new name, never over the template.
    void windowSavesATemplateAsANewPresentation()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString templatePath = dir.filePath(QStringLiteral("look.potx"));
        QString err;
        auto d = deck::newPresentation();
        QVERIFY2(deck::savePresentationFile(*d, templatePath, &err), qPrintable(err));
        const QByteArray before = fileBytes(templatePath);
        DeckWindow w;
        QVERIFY(w.openFile(templatePath));
        w.newSlide();
        // Save asks where, since the template is not to be written over, and suggests a new name; cancelled, nothing is written.
        QStringList asked;
        w.setSavePathChooser([&](const QString &suggested, const QString &) {
            asked << suggested;
            return QString();
        });
        QVERIFY(!w.save());
        QCOMPARE(asked, QStringList{dir.filePath(QStringLiteral("look.pptx"))});
        QCOMPARE(fileBytes(templatePath), before);
        QVERIFY(w.editor()->isModified());
        // Saved to a name, that is the presentation's file from then on.
        const QString path = dir.filePath(QStringLiteral("deck.pptx"));
        QVERIFY(w.saveTo(path));
        QCOMPARE(w.editor()->filePath(), path);
        QCOMPARE(fileBytes(templatePath), before);
        // A template the window itself wrote is saved over when saved again.
        const QString mine = dir.filePath(QStringLiteral("mine.potx"));
        QVERIFY(w.saveTo(mine));
        w.newSlide();
        QVERIFY(w.save());
        QVERIFY(!w.editor()->isModified());
        auto back = deck::importPresentationFile(mine, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), 3);
        w.editor()->markSaved();
    }

    // Closing a changed presentation asks Save, Don't Save, or Cancel.
    void windowAsksToSaveBeforeClosing()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.show();
        w.newSlide();
        QString label;
        answerDialogs({QMessageBox::Cancel});
        QVERIFY(!w.close());
        QVERIFY(w.isVisible());
        QVERIFY(w.editor()->isModified());
        // Save, with nowhere to save to yet, asks where; a window left from there stays open.
        w.setSavePathChooser([](const QString &, const QString &) { return QString(); });
        answerDialogs({QMessageBox::Save});
        QVERIFY(!w.close());
        QVERIFY(w.isVisible());
        // Save, with a name given, writes the file and closes.
        const QString path = dir.filePath(QStringLiteral("talk.pptx"));
        w.setSavePathChooser([&](const QString &, const QString &) { return path; });
        answerDialogs({QMessageBox::Save});
        QVERIFY(w.close());
        QVERIFY(!w.isVisible());
        auto back = deck::importPresentationFile(path);
        QVERIFY(back);
        QCOMPARE(int(back->pages.size()), 2);
        // Don't Save closes and leaves the file alone.
        DeckWindow v;
        v.show();
        QVERIFY(v.openFile(path));
        v.newSlide();
        const QByteArray before = fileBytes(path);
        answerDialogs({QMessageBox::Discard}, &label);
        QVERIFY(v.close());
        QCOMPARE(label, QStringLiteral("Don't Save"));
        QCOMPARE(fileBytes(path), before);
        // Nothing changed: it closes without asking.
        DeckWindow u;
        u.show();
        QVERIFY(u.close());
    }

    // AutoRecover: a changed presentation keeps a .pptx copy at the interval Options sets,
    // the copy goes when the work is saved, and a copy a crash left opens as unsaved work.
    void windowKeepsAutoRecoverCopies()
    {
        QTemporaryDir root, dir;
        QVERIFY(root.isValid() && dir.isValid());
        recovery::setRoot(root.path());
        recovery::setCopySuffix(QStringLiteral(".pptx"));
        {
            DeckWindow w;
            QCOMPARE(w.autoRecoverInterval(), std::max(1, Settings::get().autoRecoverMinutes()) * 60 * 1000);
            w.autoRecover();   // nothing changed: nothing to keep
            QVERIFY(w.recoveryCopy().isEmpty());
            w.newSlide();
            w.autoRecover();
            const QString copy = w.recoveryCopy();
            QVERIFY(copy.endsWith(QLatin1String(".pptx")));
            QVERIFY(QFileInfo::exists(copy));
            QVERIFY(copy.startsWith(root.path()));
            QString err;
            auto kept = deck::importPresentationFile(copy, &err);
            QVERIFY2(kept, qPrintable(err));
            QCOMPARE(int(kept->pages.size()), 2);
            // Saved, the work needs no copy.
            QVERIFY(w.saveTo(dir.filePath(QStringLiteral("a.pptx"))));
            QVERIFY(!QFileInfo::exists(copy));
            QVERIFY(w.recoveryCopy().isEmpty());

            // A copy of work never saved comes back as work never saved.
            DeckWindow lost;
            lost.newSlide();
            lost.newSlide();
            lost.autoRecover();
            const QString lostCopy = lost.recoveryCopy();
            QVERIFY(QFileInfo::exists(lostCopy));
            recovery::Recovered r;
            r.file = lostCopy;
            r.title = QStringLiteral("Lost talk");
            r.saved = QDateTime::currentDateTime();
            DeckWindow host;   // untouched, so it takes the copy itself
            DeckWindow *opened = host.openRecovered(r, &err);
            QVERIFY2(opened, qPrintable(err));
            QCOMPARE(opened, &host);
            QCOMPARE(int(host.editor()->doc()->pages.size()), 3);
            QVERIFY(host.editor()->isModified());
            QVERIFY(host.editor()->filePath().isEmpty());
            QCOMPARE(host.editor()->displayName(), QStringLiteral("Lost talk"));
            QVERIFY(!QFileInfo::exists(lostCopy));   // the old copy is replaced by this window's own
            QVERIFY(!host.recoveryCopy().isEmpty());
            host.editor()->markSaved();
            lost.editor()->markSaved();
            w.editor()->markSaved();
            // With no copies left, asking says so.
            answerDialogs({QMessageBox::Ok});
            DeckWindow asker;
            asker.offerRecovery(true);
        }
        recovery::setRoot(QString());
    }

private:
    // The pages a PDF's page tree holds (a "/Type /Pages" node is not one).
    static int pdfPageCount(const QByteArray &pdf)
    {
        static const QRegularExpression page(QStringLiteral("/Type\\s*/Page(?![A-Za-z])"));
        int n = 0;
        for (auto it = page.globalMatch(QString::fromLatin1(pdf)); it.hasNext(); it.next()) ++n;
        return n;
    }
    // The size of every page of a PDF, in points.
    static QVector<QSizeF> pdfPageSizes(const QByteArray &pdf)
    {
        static const QRegularExpression box(QStringLiteral("/MediaBox \\[0 0 ([0-9.]+) ([0-9.]+)\\]"));
        QVector<QSizeF> sizes;
        for (auto it = box.globalMatch(QString::fromLatin1(pdf)); it.hasNext();) {
            const QRegularExpressionMatch m = it.next();
            sizes << QSizeF(m.captured(1).toDouble(), m.captured(2).toDouble());
        }
        return sizes;
    }
    // What the pages of a PDF draw: its compressed streams, one after another.
    static QByteArray pdfContent(const QByteArray &pdf)
    {
        QByteArray all;
        for (qsizetype at = pdf.indexOf("stream\n"); at >= 0; at = pdf.indexOf("stream\n", at + 1)) {
            if (at >= 3 && pdf.mid(at - 3, 3) == "end") continue;
            const qsizetype from = at + 7, to = pdf.indexOf("endstream", from);
            if (to < 0) break;
            // The compressed streams (they start as zlib's do), without the length qCompress puts before one.
            if (pdf[from] == 'x') all += qUncompress(QByteArray(4, '\0') + pdf.mid(from, to - from));
        }
        return all;
    }
    // Sets up the dialog that asks what to print, which opens next, and clicks its `button`.
    static void answerOutputDialog(const std::function<void(QDialog *)> &set = nullptr, QDialogButtonBox::StandardButton button = QDialogButtonBox::Ok,
                                   QDeadlineTimer until = QDeadlineTimer(10000))
    {
        QTimer::singleShot(0, [set, button, until] {
            auto *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!dialog || dialog->objectName() != QLatin1String("outputDialog")) {
                if (!until.hasExpired()) answerOutputDialog(set, button, until);
                return;
            }
            if (set) set(dialog);
            dialog->findChild<QDialogButtonBox *>()->button(button)->click();
        });
    }
    // Five slides, titled and with notes, the third hidden.
    static std::unique_ptr<Document> outputDeck(int n = 5)
    {
        auto d = showDeck(n);
        if (n > 2) d->pages[2]->hidden = true;
        return d;
    }
    // Pixels darker than light gray in `area`.
    static int inkIn(const QImage &img, const QRect &area)
    {
        int n = 0;
        for (int y = std::max(0, area.top()); y <= std::min(img.height() - 1, area.bottom()); ++y)
            for (int x = std::max(0, area.left()); x <= std::min(img.width() - 1, area.right()); ++x)
                if (qGray(img.pixel(x, y)) < 200) ++n;
        return n;
    }
    // A page of the output on white, `scale` pixels to the point.
    static QImage outputImage(const Document &d, LayoutCache *cache, const deck::OutputOptions &o, int page, const QSizeF &paper, double scale = 1)
    {
        QImage img((paper * scale).toSize(), QImage::Format_RGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        p.scale(scale, scale);
        deck::paintOutputPage(&p, d, cache, o, page, paper);
        p.end();
        return img;
    }
    // The separate regions of `img` painted `color` (a few levels off is the same), each with its box; those under `least` pixels are specks of text.
    static QVector<QRect> colorRegions(const QImage &img, const QColor &color, int least = 400)
    {
        auto near = [&](int x, int y) {
            const QColor c = img.pixelColor(x, y);
            return std::abs(c.red() - color.red()) < 12 && std::abs(c.green() - color.green()) < 12 && std::abs(c.blue() - color.blue()) < 12;
        };
        QVector<bool> seen(img.width() * img.height(), false);
        QVector<QRect> boxes;
        for (int y0 = 0; y0 < img.height(); ++y0)
            for (int x0 = 0; x0 < img.width(); ++x0) {
                if (seen[y0 * img.width() + x0] || !near(x0, y0)) continue;
                QRect box(x0, y0, 1, 1);
                int size = 0;
                QVector<QPoint> todo{QPoint(x0, y0)};
                seen[y0 * img.width() + x0] = true;
                while (!todo.isEmpty()) {
                    const QPoint at = todo.takeLast();
                    ++size;
                    box |= QRect(at, QSize(1, 1));
                    for (const QPoint &to : {at + QPoint(1, 0), at + QPoint(-1, 0), at + QPoint(0, 1), at + QPoint(0, -1)})
                        if (to.x() >= 0 && to.y() >= 0 && to.x() < img.width() && to.y() < img.height() && !seen[to.y() * img.width() + to.x()] && near(to.x(), to.y())) {
                            seen[to.y() * img.width() + to.x()] = true;
                            todo << to;
                        }
                }
                if (size >= least) boxes << box;
            }
        return boxes;
    }
    static bool hasColor(const QImage &img)
    {
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) {
                const QRgb c = img.pixel(x, y);
                if (qRed(c) != qGreen(c) || qGreen(c) != qBlue(c)) return true;
            }
        return false;
    }

private Q_SLOTS:
    // How many pages each layout makes: a slide each, a notes page each,
    // the handouts' slides to the page. Hidden slides are out unless asked for,
    // and a range names the slides to print.
    void outputPageCountsPerLayout()
    {
        auto d = outputDeck();   // five slides, the third hidden
        deck::OutputOptions o;
        auto pages = [&](deck::PrintLayout layout) {
            o.layout = layout;
            return deck::outputPageCount(*d, o);
        };
        QCOMPARE(pages(deck::PrintLayout::Slides), 4);
        QCOMPARE(pages(deck::PrintLayout::NotesPages), 4);
        QCOMPARE(pages(deck::PrintLayout::Handout1), 4);
        QCOMPARE(pages(deck::PrintLayout::Handout2), 2);
        QCOMPARE(pages(deck::PrintLayout::Handout3), 2);
        QCOMPARE(pages(deck::PrintLayout::Handout4), 1);
        QCOMPARE(pages(deck::PrintLayout::Handout6), 1);
        QCOMPARE(pages(deck::PrintLayout::Handout9), 1);
        QVERIFY(pages(deck::PrintLayout::Outline) >= 1);
        o.hiddenSlides = true;
        QCOMPARE(pages(deck::PrintLayout::Slides), 5);
        QCOMPARE(pages(deck::PrintLayout::NotesPages), 5);
        QCOMPARE(pages(deck::PrintLayout::Handout4), 2);
        QCOMPARE(pages(deck::PrintLayout::Handout6), 1);
        QCOMPARE(pages(deck::PrintLayout::Handout3), 2);
        QCOMPARE(pages(deck::PrintLayout::Handout2), 3);
        o.hiddenSlides = false;
        // A range: those slides, in the order named, without the hidden one or any the presentation lacks.
        o.slides = {4, 0, 2, 7, -1};
        QCOMPARE(deck::outputSlides(*d, o), (QVector<int>{4, 0}));
        QCOMPARE(pages(deck::PrintLayout::Slides), 2);
        o.slides = {2};
        QCOMPARE(pages(deck::PrintLayout::Slides), 0);
        QCOMPARE(pages(deck::PrintLayout::Outline), 0);
        o.hiddenSlides = true;
        QCOMPARE(pages(deck::PrintLayout::Slides), 1);
        QCOMPARE(deck::slidesPerPage(deck::PrintLayout::Handout9), 9);
        QCOMPARE(deck::slidesPerPage(deck::PrintLayout::Slides), 0);
        // Many slides on the outline take several pages; more paper takes fewer.
        auto many = showDeck(60);
        o = deck::OutputOptions();
        o.layout = deck::PrintLayout::Outline;
        const int letter = deck::outputPageCount(*many, o, QSizeF(612, 792)), tall = deck::outputPageCount(*many, o, QSizeF(612, 3000));
        QVERIFY(letter > 1);
        QVERIFY(tall < letter);
    }

    // A slide range as typed: numbers and spans, counted from 1, made 0-based;
    // anything else is no range.
    void slideRangesAreParsed()
    {
        using R = QVector<int>;
        QCOMPARE(deck::parseSlideRange(QStringLiteral("1-3, 5"), 8), (R{0, 1, 2, 4}));
        QCOMPARE(deck::parseSlideRange(QStringLiteral(" 2 ;7-"), 8), (R{1, 6, 7}));
        QCOMPARE(deck::parseSlideRange(QStringLiteral("-2"), 8), (R{0, 1}));
        QCOMPARE(deck::parseSlideRange(QStringLiteral("6-99"), 8), (R{5, 6, 7}));
        QCOMPARE(deck::parseSlideRange(QStringLiteral("5,1"), 8), (R{4, 0}));
        QCOMPARE(deck::parseSlideRange(QStringLiteral("3"), 3), (R{2}));
        for (const char *bad : {"", "  ", "9", "0", "4-2", "1-a", "x", "1,,", "1-2-3", "-"}) QCOMPARE(deck::parseSlideRange(QString::fromLatin1(bad), 8), R());
        QCOMPARE(deck::parseSlideRange(QStringLiteral("1"), 0), R());
    }

    // Printed slides are as big as the slide; the rest are on the locale's paper.
    void outputPageSizes()
    {
        auto d = outputDeck();
        deck::OutputOptions o;
        QCOMPARE(deck::outputPageSize(*d, o), QSizeF(960, 540));
        const QSizeF paper = deck::localePaper();
        QVERIFY(paper == QSizeF(612, 792) || qAbs(paper.width() - 595.28) < 0.5);
        QVERIFY(paper.height() > paper.width());
        for (deck::PrintLayout layout : {deck::PrintLayout::NotesPages, deck::PrintLayout::Handout6, deck::PrintLayout::Outline}) {
            o.layout = layout;
            QCOMPARE(deck::outputPageSize(*d, o), paper);
            QCOMPARE(deck::outputPageSize(*d, o, QSizeF(500, 700)), QSizeF(500, 700));
        }
    }

    // The slides PDF has a page for each slide, each as big as the slide,
    // vector text with its fonts in the file, and the presentation's title.
    void exportPdfWritesOnePagePerSlide()
    {
        auto d = outputDeck();
        d->props.title = QStringLiteral("Quarterly talk");
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        deck::OutputOptions o;
        QString err;
        const QString path = dir.filePath(QStringLiteral("slides.pdf"));
        QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
        QVERIFY(err.isEmpty());
        const QByteArray pdf = fileBytes(path);
        QVERIFY(pdf.startsWith("%PDF-"));
        QCOMPARE(pdfPageCount(pdf), 4);
        QCOMPARE(pdfPageSizes(pdf), (QVector<QSizeF>(4, QSizeF(960, 540))));   // each page is the slide's size
        QVERIFY2(pdf.contains("/FontFile"), "the fonts are in the file");
        QVERIFY(pdf.contains("/Title"));
        QVERIFY(pdf.contains("/Creator"));
        // The only file left in the folder is the PDF (the temporary file was renamed).
        QCOMPARE(QDir(dir.path()).entryList(QDir::Files), QStringList{QStringLiteral("slides.pdf")});
        o.hiddenSlides = true;
        QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
        QCOMPARE(pdfPageCount(fileBytes(path)), 5);
        // A deck without a title is named for the file.
        d->props.title.clear();
        QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
        QVERIFY(fileBytes(path).contains("/Title"));
    }

    // Notes pages and handouts are on the locale's paper, upright.
    void exportPdfNotesAndHandoutsAreOnPaper()
    {
        auto d = outputDeck();
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QSizeF paper = deck::localePaper();
        struct Case { deck::PrintLayout layout; int pages; };
        for (const Case &c : {Case{deck::PrintLayout::NotesPages, 4}, Case{deck::PrintLayout::Handout6, 1}, Case{deck::PrintLayout::Handout3, 2},
                              Case{deck::PrintLayout::Outline, deck::outputPageCount(*d, [] { deck::OutputOptions o; o.layout = deck::PrintLayout::Outline; return o; }())}}) {
            deck::OutputOptions o;
            o.layout = c.layout;
            QString err;
            const QString path = dir.filePath(QStringLiteral("out%1.pdf").arg(int(c.layout)));
            QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
            const QByteArray pdf = fileBytes(path);
            QCOMPARE(pdfPageCount(pdf), c.pages);
            const QVector<QSizeF> sizes = pdfPageSizes(pdf);
            QCOMPARE(int(sizes.size()), c.pages);
            for (const QSizeF &size : sizes) QVERIFY(qAbs(size.width() - paper.width()) < 0.01 && qAbs(size.height() - paper.height()) < 0.01);
            QVERIFY(pdf.contains("/FontFile"));
        }
    }

    // Nothing to print, or nowhere to write, is an error that says so and
    // leaves what was there.
    void exportPdfFailsCleanly()
    {
        auto d = outputDeck();
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        deck::OutputOptions o;
        QString err;
        QVERIFY(!deck::exportPdf(*d, dir.filePath(QStringLiteral("missing/folder/a.pdf")), o, &err));
        QVERIFY(!err.isEmpty());
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("missing"))));
        QVERIFY(QDir(dir.path()).isEmpty());
        // A path that is a folder.
        QVERIFY(QDir(dir.path()).mkdir(QStringLiteral("folder.pdf")));
        err.clear();
        QVERIFY(!deck::exportPdf(*d, dir.filePath(QStringLiteral("folder.pdf")), o, &err));
        QVERIFY(!err.isEmpty());
        QVERIFY(QFileInfo(dir.filePath(QStringLiteral("folder.pdf"))).isDir());
        QCOMPARE(QDir(dir.path()).entryList(QDir::NoDotAndDotDot | QDir::AllEntries), QStringList{QStringLiteral("folder.pdf")});
        // No slides to print: the old PDF stays whole.
        const QString kept = dir.filePath(QStringLiteral("kept.pdf"));
        QVERIFY(deck::exportPdf(*d, kept, o, &err));
        const QByteArray before = fileBytes(kept);
        QVERIFY(!before.isEmpty());
        o.slides = {2};   // the hidden slide
        err.clear();
        QVERIFY(!deck::exportPdf(*d, kept, o, &err));
        QVERIFY(!err.isEmpty());
        QCOMPARE(fileBytes(kept), before);
        QVERIFY(!deck::exportPdf(*d, kept, o, nullptr));   // no place for the reason is fine
    }

    // A notes page has the slide framed in its upper half and the speaker
    // notes under it; a page without notes has nothing there.
    void notesPagePaintsTheSlideAboveItsNotes()
    {
        auto d = outputDeck();
        deck::setSlideBackground(*d->pages[0], QColor(0xC0, 0x20, 0x20));
        d->storyDoc(d->pages[1]->notesStoryId)->clear();
        LayoutCache cache;
        deck::OutputOptions o;
        o.layout = deck::PrintLayout::NotesPages;
        const QSizeF paper = deck::localePaper();
        const int w = int(paper.width()), h = int(paper.height());
        const QImage with = outputImage(*d, &cache, o, 0, paper, 2);
        // The slide: one red region in the upper half, framed, in the margins.
        const QVector<QRect> slide = colorRegions(with, QColor(0xC0, 0x20, 0x20));
        QCOMPARE(int(slide.size()), 1);
        QVERIFY(slide[0].top() >= 20);   // (pixels, 2 to the point)
        QVERIFY2(slide[0].bottom() < h, "the slide is above the middle");
        QVERIFY(slide[0].left() > 20 && slide[0].right() < 2 * w - 20);
        QVERIFY2(slide[0].width() * 9 / 16 - 4 <= slide[0].height() && slide[0].height() <= slide[0].width() * 9 / 16 + 4, "the slide keeps its shape");
        QVERIFY2(inkIn(with, QRect(slide[0].left() - 1, slide[0].center().y(), 1, 1)) == 1, "the slide is framed");
        // The notes are under the slide, and no lower than the page number.
        const QRect lower(40 * 2, slide[0].bottom() + 12, (w - 80) * 2, 2 * h - 160 - slide[0].bottom());
        QVERIFY2(inkIn(with, lower) > 20, "the notes");
        QCOMPARE(inkIn(with, QRect(0, 0, 2 * w, 30)), 0);
        // No notes: nothing there, but the page number at the foot.
        const QImage without = outputImage(*d, &cache, o, 1, paper, 2);
        QCOMPARE(inkIn(without, lower), 0);
        QVERIFY(inkIn(without, QRect(2 * w - 200, 2 * h - 100, 200, 100)) > 0);
        QVERIFY(inkIn(without, QRect(0, 2 * h - 100, w, 100)) == 0);
    }

    // A handout of six has six slides in a grid, each framed; the last page has the rest.
    void handoutOfSixFramesSixSlides()
    {
        auto d = showDeck(7);
        for (auto &page : d->pages) deck::setSlideBackground(*page, QColor(0x1F, 0x5F, 0xBF));
        LayoutCache cache;
        deck::OutputOptions o;
        o.layout = deck::PrintLayout::Handout6;
        QCOMPARE(deck::outputPageCount(*d, o), 2);
        const QSizeF paper = deck::localePaper();
        const QImage first = outputImage(*d, &cache, o, 0, paper, 2);
        const QVector<QRect> six = colorRegions(first, QColor(0x1F, 0x5F, 0xBF));
        QCOMPARE(int(six.size()), 6);
        for (const QRect &slide : six) {
            QVERIFY2(slide.width() > 100 && slide.width() * 9 / 16 - 4 <= slide.height() && slide.height() <= slide.width() * 9 / 16 + 4, "each keeps its shape");
            // The frame is dark on every side.
            for (const QPoint &outside : {QPoint(slide.left() - 1, slide.center().y()), QPoint(slide.right() + 1, slide.center().y()),
                                          QPoint(slide.center().x(), slide.top() - 1), QPoint(slide.center().x(), slide.bottom() + 1)})
                QVERIFY2(qGray(first.pixel(outside)) < 120, qPrintable(QStringLiteral("frame at %1,%2").arg(outside.x()).arg(outside.y())));
        }
        // Two columns of three rows.
        QSet<int> xs, ys;
        for (const QRect &slide : six) {
            xs << slide.left() / 20;
            ys << slide.top() / 20;
        }
        QCOMPARE(int(xs.size()), 2);
        QCOMPARE(int(ys.size()), 3);
        const QImage last = outputImage(*d, &cache, o, 1, paper, 2);
        QCOMPARE(int(colorRegions(last, QColor(0x1F, 0x5F, 0xBF)).size()), 1);
        // Across, then down puts the second slide beside the first; down, then across puts it under.
        auto firstTwo = [&](bool horizontal) {
            o.horizontalOrder = horizontal;
            for (int i = 0; i < 6; ++i) d->pages[i]->background = Fill();
            deck::setSlideBackground(*d->pages[0], QColor(0xC0, 0x20, 0x20));
            deck::setSlideBackground(*d->pages[1], QColor(0x20, 0xA0, 0x20));
            const QImage img = outputImage(*d, &cache, o, 0, paper, 1);
            return QPair<QVector<QRect>, QVector<QRect>>(colorRegions(img, QColor(0xC0, 0x20, 0x20), 100), colorRegions(img, QColor(0x20, 0xA0, 0x20), 100));
        };
        auto across = firstTwo(true);
        QCOMPARE(int(across.first.size()), 1);
        QCOMPARE(int(across.second.size()), 1);
        QVERIFY(across.second[0].left() > across.first[0].right() && across.second[0].top() == across.first[0].top());
        auto down = firstTwo(false);
        QCOMPARE(int(down.first.size()), 1);
        QCOMPARE(int(down.second.size()), 1);
        QVERIFY(down.second[0].top() > down.first[0].bottom() && down.second[0].left() == down.first[0].left());
    }

    // Three to a page leave lines for notes to the right of the slides.
    void handoutOfThreeHasLinesBesideTheSlides()
    {
        auto d = showDeck(3);
        for (auto &page : d->pages) deck::setSlideBackground(*page, QColor(0x1F, 0x5F, 0xBF));
        LayoutCache cache;
        deck::OutputOptions o;
        o.layout = deck::PrintLayout::Handout3;
        const QSizeF paper = deck::localePaper();
        const QImage img = outputImage(*d, &cache, o, 0, paper);
        const QVector<QRect> three = colorRegions(img, QColor(0x1F, 0x5F, 0xBF));
        QCOMPARE(int(three.size()), 3);
        int right = 0;
        for (const QRect &slide : three) right = std::max(right, slide.right());
        QVERIFY2(right < paper.width() * 0.55, "the slides are to the left");
        // Lines: dark runs in the right part, none of which is a slide.
        const int from = int(paper.width() * 0.62);
        QVERIFY(inkIn(img, QRect(from, 80, int(paper.width()) - from - 20, int(paper.height()) - 160)) > 300);
        o.layout = deck::PrintLayout::Handout4;
        QCOMPARE(int(colorRegions(outputImage(*d, &cache, o, 0, paper), QColor(0x1F, 0x5F, 0xBF)).size()), 3);
    }

    // Gray output has no color in it, on any layout; the same page in color does.
    void grayscaleOutputHasNoColor()
    {
        auto d = showDeck(3);
        deck::setSlideBackground(*d->pages[0], QColor(0xC0, 0x20, 0x20));
        deck::setSlideBackground(*d->pages[1], QColor(0x20, 0xA0, 0x20));
        LayoutCache cache;
        deck::OutputOptions o;
        for (deck::PrintLayout layout : {deck::PrintLayout::Slides, deck::PrintLayout::NotesPages, deck::PrintLayout::Handout6}) {
            o.layout = layout;
            const QSizeF paper = deck::outputPageSize(*d, o);
            o.grayscale = false;
            const QImage color = outputImage(*d, &cache, o, 0, paper, 0.5);
            QVERIFY2(hasColor(color), "the slide is red in color");
            o.grayscale = true;
            const QImage gray = outputImage(*d, &cache, o, 0, paper, 0.5);
            QVERIFY2(!hasColor(gray), qPrintable(QStringLiteral("layout %1").arg(int(layout))));
            QVERIFY(inkIn(gray, QRect(QPoint(0, 0), gray.size())) > 100);   // still drawn
        }
        // The PDF is made in gray.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString err;
        o.layout = deck::PrintLayout::Slides;
        const QString path = dir.filePath(QStringLiteral("gray.pdf"));
        QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
        QCOMPARE(pdfPageCount(fileBytes(path)), 3);
        // Colors in the pages are set with three numbers, grays with one.
        static const QRegularExpression rgb(QStringLiteral("[0-9.]+ [0-9.]+ [0-9.]+ scn"));
        const QString gray = QString::fromLatin1(pdfContent(fileBytes(path)));
        QVERIFY(gray.contains(QStringLiteral(" scn")));
        QVERIFY2(!gray.contains(rgb), "a color in the gray PDF");
        o.grayscale = false;
        QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
        QVERIFY2(QString::fromLatin1(pdfContent(fileBytes(path))).contains(rgb), "the PDF in color has colors");
    }

    // Printed slides are framed only when asked for.
    void printedSlidesAreFramedOnRequest()
    {
        auto d = showDeck(2);
        deck::setSlideBackground(*d->pages[0], QColor(0xE0, 0xE0, 0x20));
        LayoutCache cache;
        deck::OutputOptions o;
        const QSizeF paper(960, 540);
        QImage plain = outputImage(*d, &cache, o, 0, paper);
        QCOMPARE(plain.pixelColor(0, 270), QColor(0xE0, 0xE0, 0x20));
        QCOMPARE(plain.pixelColor(959, 270), QColor(0xE0, 0xE0, 0x20));
        o.frameSlides = true;
        QImage framed = outputImage(*d, &cache, o, 0, paper);
        QVERIFY(qGray(framed.pixel(0, 270)) < 120);
        QVERIFY(qGray(framed.pixel(959, 270)) < 120);
        QVERIFY(qGray(framed.pixel(480, 0)) < 120);
        QVERIFY(qGray(framed.pixel(480, 539)) < 120);
        QCOMPARE(framed.pixelColor(20, 270), QColor(0xE0, 0xE0, 0x20));
        // On bigger paper the slide is as large as fits, centered.
        const QImage onPaper = outputImage(*d, &cache, o, 0, QSizeF(1200, 900));
        const QVector<QRect> slide = colorRegions(onPaper, QColor(0xE0, 0xE0, 0x20));
        QCOMPARE(int(slide.size()), 1);
        QVERIFY(qAbs(slide[0].width() - 1200) <= 6 && qAbs(slide[0].center().y() - 450) <= 3);
    }

    // The outline is the slides' titles with their body text by level.
    void outlineListsTitlesAndBodyByLevel()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        auto story = [&](const Page &slide, const QString &type) -> QTextDocument * {
            for (const auto &it : slide.items)
                if (deck::placeholderType(*it) == type && it->type() == ItemType::Text) return d->storyDoc(static_cast<TextItem *>(it.get())->storyId);
            return nullptr;
        };
        story(*d->pages[0], QStringLiteral("ctrTitle"))->setPlainText(QStringLiteral("Quarterly results"));
        story(*d->pages[0], QStringLiteral("subTitle"))->setPlainText(QStringLiteral("North region"));
        story(*d->pages[1], QStringLiteral("title"))->setPlainText(QStringLiteral("Highlights"));
        QTextDocument *body = story(*d->pages[1], QStringLiteral("body"));
        if (!body) body = story(*d->pages[1], QStringLiteral("obj"));
        QVERIFY(body);
        body->setPlainText(QStringLiteral("Sales grew\nUp 12 percent\nCosts fell\n\nHiring"));
        QTextCursor c(body->findBlockByNumber(1));
        QTextBlockFormat deeper;
        deeper.setProperty(tp::ListLevel, 1);
        c.mergeBlockFormat(deeper);
        // The third slide has no title and no text.
        deck::OutputOptions o;
        o.layout = deck::PrintLayout::Outline;
        QCOMPARE(deck::outputOutlineText(*d, o),
                 QStringLiteral("1  Quarterly results\n  North region\n2  Highlights\n  Sales grew\n    Up 12 percent\n  Costs fell\n  Hiring\n3\n"));
        // A range keeps the numbers of the slides.
        o.slides = {1};
        QCOMPARE(deck::outputOutlineText(*d, o), QStringLiteral("2  Highlights\n  Sales grew\n    Up 12 percent\n  Costs fell\n  Hiring\n"));
        // The page has them in ink.
        o.slides.clear();
        LayoutCache cache;
        const QSizeF paper = deck::localePaper();
        const QImage page = outputImage(*d, &cache, o, 0, paper);
        QVERIFY(inkIn(page, QRect(40, 40, int(paper.width()) - 80, 200)) > 200);
        QCOMPARE(inkIn(page, QRect(40, int(paper.height()) / 2, int(paper.width()) - 80, int(paper.height()) / 2 - 40)), 0);
    }

    // A slide as a picture: 1920 pixels across, the slide's shape, on its own background.
    void slideImageIs1920PixelsWide()
    {
        auto d = showDeck(2);
        deck::setSlideBackground(*d->pages[1], QColor(0x20, 0x80, 0x40));
        LayoutCache cache;
        const QImage img = deck::slideImage(*d, &cache, 1);
        QCOMPARE(img.size(), QSize(1920, 1080));
        QCOMPARE(img.pixelColor(5, 5), QColor(0x20, 0x80, 0x40));
        QCOMPARE(deck::slideImage(*d, &cache, 0, 640).size(), QSize(640, 360));
        QVERIFY(deck::slideImage(*d, &cache, 9).isNull());
        // A standard-shaped slide keeps its shape.
        deck::setSlideSize(*d, QSizeF(deck::kStandardWidth, deck::kStandardHeight));
        QCOMPARE(deck::slideImage(*d, &cache, 0, 800).size(), QSize(800, 600));
    }

    // Printing paints each page on the printer's sheet, in gray (as pictures) when the printer or the options ask.
    void printerOutputPaintsEveryPage()
    {
        auto d = outputDeck();
        deck::setSlideBackground(*d->pages[0], QColor(0xC0, 0x20, 0x20));
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        LayoutCache cache;
        for (bool gray : {false, true}) {
            const QString path = dir.filePath(gray ? QStringLiteral("gray.pdf") : QStringLiteral("color.pdf"));
            {
                QPrinter printer(QPrinter::HighResolution);
                printer.setOutputFormat(QPrinter::PdfFormat);
                printer.setOutputFileName(path);
                printer.setPageSize(QPageSize(deck::localePaper(), QPageSize::Point, QString(), QPageSize::FuzzyMatch));
                deck::OutputOptions o;
                o.layout = deck::PrintLayout::Handout3;
                if (gray) printer.setColorMode(QPrinter::GrayScale);
                o.grayscale = gray;
                QPainter p(&printer);
                QVERIFY(p.isActive());
                const QSizeF sheet = printer.pageRect(QPrinter::Point).size();
                const int pages = deck::outputPageCount(*d, o, sheet);
                QCOMPARE(pages, 2);
                for (int i = 0; i < pages; ++i) {
                    if (i) printer.newPage();
                    p.save();
                    p.scale(printer.resolution() / 72.0, printer.resolution() / 72.0);
                    deck::paintOutputPage(&p, *d, &cache, o, i, sheet);
                    p.restore();
                }
            }
            const QByteArray pdf = fileBytes(path);
            QCOMPARE(pdfPageCount(pdf), 2);
            static const QRegularExpression rgb(QStringLiteral("[0-9.]+ [0-9.]+ [0-9.]+ scn"));
            const QString content = QString::fromLatin1(pdfContent(pdf));
            QCOMPARE(content.contains(rgb), !gray);
            QCOMPARE(pdf.contains("/Subtype /Image"), gray);
        }
    }

    // Print and the exports are in the File menu's commands, and Ctrl+P prints.
    void outputCommandsExist()
    {
        DeckWindow w;
        for (const char *id : {"file.exportPdf", "file.print", "file.exportPicture"}) {
            QVERIFY2(w.act(id), id);
            QVERIFY2(!w.act(id)->text().isEmpty(), id);
        }
        QCOMPARE(w.act("file.print")->shortcut(), QKeySequence(QKeySequence::Print));
        QVERIFY(w.act("file.exportPdf")->text().endsWith(QChar(0x2026)));
    }

    // Export to PDF asks what to print, then where, and writes the file; a
    // refusal at either step writes nothing, and a file that can't be written is said so.
    void windowExportsPdf()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.newSlide();
        w.newSlide();
        const QString path = dir.filePath(QStringLiteral("talk.pdf"));
        QStringList asked;
        w.setSavePathChooser([&](const QString &suggested, const QString &filter) {
            asked << suggested << filter;
            return path;
        });
        auto choose = [](deck::PrintLayout layout) {
            return [layout](QDialog *dialog) {
                auto *what = dialog->findChild<QComboBox *>(QStringLiteral("outputWhat"));
                QVERIFY(what);
                const int at = what->findData(int(layout));
                QVERIFY(at >= 0);
                what->setCurrentIndex(at);
            };
        };
        // Cancelled at the first step.
        answerOutputDialog(nullptr, QDialogButtonBox::Cancel);
        w.act("file.exportPdf")->trigger();
        QVERIFY(asked.isEmpty());
        QVERIFY(!QFileInfo::exists(path));
        // Slides, as the dialog starts: a page for each.
        answerOutputDialog();
        w.act("file.exportPdf")->trigger();
        QCOMPARE(asked.size(), 2);
        QVERIFY2(asked[0].endsWith(QLatin1String("Presentation1.pdf")), qPrintable(asked[0]));
        QVERIFY(asked[1].contains(QLatin1String("*.pdf")));
        QCOMPARE(pdfPageCount(fileBytes(path)), 3);
        QCOMPARE(pdfPageSizes(fileBytes(path)).value(0), QSizeF(960, 540));
        QVERIFY(w.statusBar()->currentMessage().contains(QLatin1String("talk.pdf")));
        // Notes pages, only the slides 2 to 3.
        answerOutputDialog([&](QDialog *dialog) {
            choose(deck::PrintLayout::NotesPages)(dialog);
            dialog->findChild<QRadioButton *>(QStringLiteral("outputRangeSlides"))->setChecked(true);
            dialog->findChild<QLineEdit *>(QStringLiteral("outputRangeText"))->setText(QStringLiteral("2-3"));
        });
        w.act("file.exportPdf")->trigger();
        QCOMPARE(pdfPageCount(fileBytes(path)), 2);
        QCOMPARE(pdfPageSizes(fileBytes(path)).value(0), deck::localePaper());
        // The slide being edited, hidden slides left out.
        w.editor()->setCurrentPage(1);
        w.toggleHidden();
        QVERIFY(QFile::remove(path));
        answerOutputDialog(
            [&](QDialog *dialog) {
                dialog->findChild<QRadioButton *>(QStringLiteral("outputRangeCurrent"))->setChecked(true);
                QVERIFY2(!dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->isEnabled(), "the only slide chosen is hidden: nothing to write");
            },
            QDialogButtonBox::Cancel);
        w.act("file.exportPdf")->trigger();
        QVERIFY(!QFileInfo::exists(path));
        answerOutputDialog([&](QDialog *dialog) { dialog->findChild<QCheckBox *>(QStringLiteral("outputHidden"))->setChecked(true); });
        w.act("file.exportPdf")->trigger();
        QCOMPARE(pdfPageCount(fileBytes(path)), 3);
        // No name chosen writes nothing.
        QVERIFY(QFile::remove(path));
        w.setSavePathChooser([](const QString &, const QString &) { return QString(); });
        answerOutputDialog();
        w.act("file.exportPdf")->trigger();
        QVERIFY(!QFileInfo::exists(path));
        // A place that can't be written is an error the person sees.
        QString label;
        w.setSavePathChooser([&](const QString &, const QString &) {
            answerDialogs({QMessageBox::Ok}, &label);
            return dir.filePath(QStringLiteral("missing/talk.pdf"));
        });
        answerOutputDialog();
        w.act("file.exportPdf")->trigger();
        QCOMPARE(label, QStringLiteral("OK"));
        QVERIFY(!QFileInfo::exists(dir.filePath(QStringLiteral("missing"))));
    }

    // Export slide as picture saves the slide being edited, 1920 pixels across,
    // as the kind of picture the name asks for.
    void windowExportsSlidePicture()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.newSlide();
        deck::setSlideBackground(*w.editor()->doc()->pages[1], QColor(0x20, 0x80, 0x40));
        w.editor()->setCurrentPage(1);
        QString suggested, path = dir.filePath(QStringLiteral("slide.png"));
        w.setSavePathChooser([&](const QString &name, const QString &) {
            suggested = name;
            return path;
        });
        w.act("file.exportPicture")->trigger();
        QVERIFY2(suggested.endsWith(QLatin1String(".png")) && suggested.contains(QLatin1String("2")), qPrintable(suggested));
        QImage png(path);
        QCOMPARE(png.size(), QSize(1920, 1080));
        QCOMPARE(png.pixelColor(4, 4), QColor(0x20, 0x80, 0x40));
        path = dir.filePath(QStringLiteral("slide.jpg"));
        w.act("file.exportPicture")->trigger();
        QImage jpeg(path);
        QCOMPARE(jpeg.size(), QSize(1920, 1080));
        QVERIFY(qAbs(jpeg.pixelColor(4, 4).green() - 0x80) < 8);
        QCOMPARE(fileBytes(path).left(2), QByteArray("\xff\xd8"));
        // No ending takes the kind the dialog showed.
        path = dir.filePath(QStringLiteral("bare"));
        w.act("file.exportPicture")->trigger();
        QVERIFY(QFileInfo::exists(path + QLatin1String(".png")));
        // No name, no file; a place that can't be written is said.
        const int before = int(QDir(dir.path()).entryList(QDir::Files).size());
        w.setSavePathChooser([](const QString &, const QString &) { return QString(); });
        w.act("file.exportPicture")->trigger();
        QCOMPARE(int(QDir(dir.path()).entryList(QDir::Files).size()), before);
        QString label;
        w.setSavePathChooser([&](const QString &, const QString &) {
            answerDialogs({QMessageBox::Ok}, &label);
            return dir.filePath(QStringLiteral("missing/slide.png"));
        });
        w.act("file.exportPicture")->trigger();
        QCOMPARE(label, QStringLiteral("OK"));
    }
};

QTEST_MAIN(DeckTests)
#include "tst_deck.moc"
