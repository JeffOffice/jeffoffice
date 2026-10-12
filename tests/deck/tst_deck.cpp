// JeffDeck's tests: its presentation model on the shared page model, and
// its window.

#include "../../apps/pub/app/pubinfo.h"   // only its inline pubAppInfo(): the other app's colors, to compare
#include "app/deckinfo.h"
#include "app/deckwindow.h"
#include "app/editor.h"
#include "app/icons.h"
#include "app/notespane.h"
#include "app/ribbon.h"
#include "app/slideshow.h"
#include "app/slidespane.h"
#include "core/deck.h"
#include "core/fonts.h"
#include "render/renderer.h"
#include "text/textprops.h"

#include <QAbstractButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
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

    // A new slide size scales everything on every slide, layout, and the
    // slide master to fit, centered (Ensure Fit): 16:9 to 4:3 shrinks it by
    // the width's ratio, and going wider moves it to the middle at its size.
    void slideSizeScalesEverything()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        const auto *title = static_cast<const TextItem *>(d->pages[1]->items[0].get());
        const MasterPage *layout = deck::layoutOfType(*d, QStringLiteral("obj"));
        const QRectF slideBefore = title->rect, layoutBefore = layout->items[0]->rect, masterBefore = d->masters[0]->items[0]->rect;
        auto size = [&](const QString &storyId) { return QTextCursor(d->storyDoc(storyId)).charFormat().fontPointSize(); };
        const double textBefore = size(title->storyId);
        QVERIFY(textBefore > 0);

        deck::setSlideSize(*d, QSizeF(deck::kStandardWidth, deck::kStandardHeight));
        QCOMPARE(d->pageSize(), QSizeF(720, 540));
        QCOMPARE(d->setup.sizeName, QStringLiteral("Standard"));
        auto fitted = [](const QRectF &r) { return QRectF(r.x() * 0.75, r.y() * 0.75 + 67.5, r.width() * 0.75, r.height() * 0.75); };
        QCOMPARE(title->rect, fitted(slideBefore));
        QCOMPARE(layout->items[0]->rect, fitted(layoutBefore));
        QCOMPARE(d->masters[0]->items[0]->rect, fitted(masterBefore));
        QCOMPARE(size(title->storyId), textBefore * 0.75);
        // A slide still draws, at its new size.
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        QCOMPARE(Renderer::renderToImage(ctx, 0, 0.5).size(), QSize(360, 270));

        // Wider again: nothing grows, it moves to the middle.
        deck::setSlideSize(*d, QSizeF(deck::kSlideWidth, deck::kSlideHeight));
        QCOMPARE(d->setup.sizeName, QStringLiteral("Widescreen"));
        QCOMPARE(title->rect, fitted(slideBefore).translated(120, 0));
        QCOMPARE(size(title->storyId), textBefore * 0.75);
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
