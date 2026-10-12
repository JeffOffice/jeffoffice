// JeffDeck's tests: its presentation model on the shared page model, and
// its window.

#include "app/deckinfo.h"
#include "app/deckwindow.h"
#include "app/editor.h"
#include "app/notespane.h"
#include "app/slideshow.h"
#include "app/slidespane.h"
#include "core/deck.h"
#include "core/fonts.h"
#include "render/renderer.h"

#include <QSettings>
#include <QTemporaryDir>
#include <QTest>
#include <QTextDocument>

using namespace jp;

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
