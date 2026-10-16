// JeffDeck's tests of the slide master view.
#include "deckhelpers.h"

class DeckMasterTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

private:
    // The text placeholder of that type on a slide, slide master, or layout.
    static TextItem *holderOn(const PageBase &page, const QString &type)
    {
        for (const auto &it : page.items)
            if (it->type() == ItemType::Text && deck::placeholderType(*it) == type) return static_cast<TextItem *>(it.get());
        return nullptr;
    }
    // The font family the first letter of a placeholder's text is set in, and its size.
    static QTextCharFormat firstLetterOf(const Document &d, const TextItem &t)
    {
        const QTextDocument *sd = d.storyDoc(t.storyId);
        if (!sd) return QTextCharFormat();
        const QTextBlock b = sd->begin();
        return b.begin().atEnd() ? b.charFormat() : b.begin().fragment().charFormat();
    }
    static QString familyOfHolder(const Document &d, const TextItem &t) { return firstLetterOf(d, t).fontFamilies().toStringList().value(0); }
    // The tab of the ribbon with that name, or -1.
    static int tabNumber(const Ribbon &r, const QString &name)
    {
        for (int i = 0; i < r.tabCount(); ++i)
            if (r.tabName(i) == name) return i;
        return -1;
    }
    static std::unique_ptr<Document> throughPptx(const Document &d, QMap<QString, QByteArray> *zip = nullptr)
    {
        QString err;
        const QByteArray bytes = deck::exportPresentation(d, &err);
        if (zip) readZip(bytes, *zip, &err);
        return deck::importPresentation(bytes, &err);
    }

private Q_SLOTS:
    // View > Slide Master opens Slide Master view on the layout of the slide being edited: the thumbnails pane lists each slide master with its
    // layouts indented under it, the Slide Master tab comes first and is the one shown, and Close Master View goes back to the slide.
    void slideMasterViewOpensAndCloses()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two"});
        w.editor()->setCurrentPage(1);
        QAction *open = w.act(QStringLiteral("view.master"));
        QVERIFY(open && w.act(QStringLiteral("master.close")));
        const int tab = tabNumber(*w.ribbon(), QStringLiteral("Slide Master"));
        QVERIFY(tab >= 0);
        QVERIFY(!w.ribbon()->tabVisible(tab));
        open->trigger();
        QCOMPARE(w.view(), DeckWindow::View::Master);
        Document *d = w.editor()->doc();
        const QString layout = d->pages[1]->masterId;
        QCOMPARE(w.editor()->masterView(), layout);
        QVERIFY(w.ribbon()->tabVisible(tab));
        QCOMPARE(w.ribbon()->current(), w.ribbon()->tab(QStringLiteral("Slide Master")));
        QVERIFY(w.slidesPane()->isHidden() && !w.masterPane()->isHidden() && w.notesPane()->isHidden());
        MasterPane *pane = w.masterPane();
        QCOMPARE(pane->ids().size(), 10);
        QCOMPARE(pane->ids().first(), QStringLiteral("SM1"));
        QCOMPARE(pane->depthOf(QStringLiteral("SM1")), 0);
        QCOMPARE(pane->depthOf(layout), 1);
        QCOMPARE(pane->currentId(), layout);
        // The tabs that work on slides wait.
        QVERIFY(!w.ribbon()->tabVisible(tabNumber(*w.ribbon(), QStringLiteral("Transitions"))));
        QVERIFY(!w.act(QStringLiteral("slide.new"))->isEnabled());
        // Another row in the pane is edited on the canvas.
        pane->selectPage(QStringLiteral("SM1"));
        QCOMPARE(w.editor()->masterView(), QStringLiteral("SM1"));
        QCOMPARE(w.editor()->surface(), d->master(QStringLiteral("SM1")));
        w.act(QStringLiteral("master.close"))->trigger();
        QCOMPARE(w.view(), DeckWindow::View::Normal);
        QVERIFY(w.editor()->masterView().isEmpty());
        QCOMPARE(w.editor()->currentPage(), 1);
        QVERIFY(!w.slidesPane()->isHidden() && w.masterPane()->isHidden() && !w.notesPane()->isHidden());
        QVERIFY(!w.ribbon()->tabVisible(tab));
        QVERIFY(w.ribbon()->tabVisible(tabNumber(*w.ribbon(), QStringLiteral("Transitions"))));
        QVERIFY(w.act(QStringLiteral("slide.new"))->isEnabled());
    }

    // The title font of the slide master, changed in Slide Master view, is the title font of every layout and slide whose title looked as the
    // master's did; a slide whose title was set in a font by hand keeps it. It is one step that undoes.
    void masterTitleFontReachesTheSlidesThatFollowIt()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two", "Three"});
        Document *d = w.editor()->doc();
        const QString heading = deck::themeOf(*d).headingFont;
        {
            const TextItem *own = holderOn(*d->pages[2], QStringLiteral("title"));
            QVERIFY(own);
            QTextCursor c(d->storyDoc(own->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontFamilies({QStringLiteral("Courier New")});
            c.mergeCharFormat(f);
        }
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->pages[1], QStringLiteral("title"))), heading);
        const int steps = w.editor()->undoStack()->count();
        w.editMasters(QStringLiteral("SM1"));
        w.editor()->select(holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("title"))->id);
        w.editor()->setFontFamily(QStringLiteral("Georgia"));
        QCOMPARE(w.editor()->undoStack()->count(), steps + 1);
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("title"))), QStringLiteral("Georgia"));
        for (const MasterPage *l : deck::layouts(*d, QStringLiteral("SM1"))) {
            const TextItem *title = holderOn(*l, QStringLiteral("title")) ? holderOn(*l, QStringLiteral("title")) : holderOn(*l, QStringLiteral("ctrTitle"));
            if (title) QCOMPARE(familyOfHolder(*d, *title), QStringLiteral("Georgia"));
        }
        // The Title Slide's title, and the slide whose title follows the layout.
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->pages[0], QStringLiteral("ctrTitle"))), QStringLiteral("Georgia"));
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->pages[1], QStringLiteral("title"))), QStringLiteral("Georgia"));
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->pages[2], QStringLiteral("title"))), QStringLiteral("Courier New"));
        // What the text says is still what it said.
        QCOMPARE(titlesOf(*d), QStringList({"One", "Two", "Three"}));
        w.editor()->undo();
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("title"))), heading);
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->pages[1], QStringLiteral("title"))), heading);
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->pages[0], QStringLiteral("ctrTitle"))), heading);
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->pages[2], QStringLiteral("title"))), QStringLiteral("Courier New"));
        w.editor()->redo();
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->pages[1], QStringLiteral("title"))), QStringLiteral("Georgia"));
    }

    // The size of the slide master's text is the size of the text of the layouts and slides that had its size, and not of the ones that had
    // their own (Content with Caption sets its text at 32).
    void masterBodySizeReachesTheSlidesThatFollowIt()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two"}, {{"sub"}, {"a", "b"}});
        Document *d = w.editor()->doc();
        auto sizeOf = [&](const PageBase &page, const QString &type) { return firstLetterOf(*d, *holderOn(page, type)).fontPointSize(); };
        QCOMPARE(sizeOf(*d->pages[1], QStringLiteral("obj")), 28.0);
        w.editMasters(QStringLiteral("SM1"));
        w.editor()->select(holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("body"))->id);
        w.editor()->setFontSize(40);
        QCOMPARE(sizeOf(*d->master(QStringLiteral("SM1")), QStringLiteral("body")), 40.0);
        QCOMPARE(sizeOf(*d->pages[1], QStringLiteral("obj")), 40.0);
        QCOMPARE(sizeOf(*deck::layoutOfType(*d, QStringLiteral("twoObj")), QStringLiteral("obj")), 40.0);
        QCOMPARE(sizeOf(*deck::layoutOfType(*d, QStringLiteral("objTx")), QStringLiteral("obj")), 32.0);
        // Every paragraph of the slide's text, not just the first.
        QTextDocument *story = bodyOfSlide(*d, 1);
        QVERIFY(story && story->blockCount() == 2);
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) QCOMPARE(b.begin().fragment().charFormat().fontPointSize(), 40.0);
    }

    // A change to a layout shows on every slide that stands on it at once: a placeholder where the layout had it moves with it (one a slide
    // moved by hand stays), and what the layout draws is drawn under the slides. Slides on other layouts don't change.
    void layoutEditShowsOnItsSlidesAtOnce()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two", "Three"});
        Document *d = w.editor()->doc();
        const QString layout = d->pages[1]->masterId;
        QCOMPARE(d->pages[2]->masterId, layout);
        const QRectF was = holderOn(*d->pages[1], QStringLiteral("title"))->rect;
        QCOMPARE(holderOn(*d->master(layout), QStringLiteral("title"))->rect, was);
        holderOn(*d->pages[2], QStringLiteral("title"))->rect = QRectF(10, 10, 300, 40);
        const QRectF titleSlide = holderOn(*d->pages[0], QStringLiteral("ctrTitle"))->rect;
        w.editMasters(layout);
        const QRectF moved(66, 60, 600, 80);
        w.editor()->change(QStringLiteral("Move"), [&] { holderOn(*d->master(layout), QStringLiteral("title"))->rect = moved; });
        QCOMPARE(holderOn(*d->pages[1], QStringLiteral("title"))->rect, moved);
        QCOMPARE(holderOn(*d->pages[2], QStringLiteral("title"))->rect, QRectF(10, 10, 300, 40));
        QCOMPARE(holderOn(*d->pages[0], QStringLiteral("ctrTitle"))->rect, titleSlide);
        // A band across the foot of the layout.
        auto band = std::make_shared<ShapeItem>();
        band->shape = QStringLiteral("rect");
        band->rect = QRectF(0, 480, 960, 60);
        band->fill = Fill::solid(ColorRef::rgb(QColor(0x12, 0x34, 0x56)));
        band->stroke = Stroke();
        w.editor()->addItem(band, false);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d;
        ctx.cache = &cache;
        auto bandAt = [&](int slide) { return Renderer::renderToImage(ctx, slide, 1).pixelColor(480, 510) == QColor(0x12, 0x34, 0x56); };
        QVERIFY(bandAt(1) && bandAt(2));
        QVERIFY(!bandAt(0));
        w.editor()->undo();
        QVERIFY(!bandAt(1));
    }

    // Insert Layout adds a layout to the slide master (a title and the footers, as the slide master has them) after the one being edited, and
    // opens it; Delete takes away a layout nothing stands on, and says why it won't take away one that slides stand on. Each is one step.
    void layoutsAreInsertedAndDeleted()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two"});
        Document *d = w.editor()->doc();
        const QString used = d->pages[1]->masterId;
        const int pages = int(d->masters.size());
        w.editMasters(used);
        const QString made = w.insertLayout();
        QVERIFY(!made.isEmpty());
        const MasterPage *layout = d->master(made);
        QVERIFY(layout && deck::isLayout(*layout));
        QCOMPARE(layout->baseId, QStringLiteral("SM1"));
        QCOMPARE(layout->name, QStringLiteral("Custom Layout"));
        QCOMPARE(int(d->masters.size()), pages + 1);
        QCOMPARE(w.editor()->masterView(), made);
        // Right after the layout that was open.
        int at = -1, after = -1;
        for (int i = 0; i < d->masters.size(); ++i) {
            if (d->masters[i]->id == used) at = i;
            if (d->masters[i]->id == made) after = i;
        }
        QCOMPARE(after, at + 1);
        QVERIFY(holderOn(*layout, QStringLiteral("title")) && holderOn(*layout, QStringLiteral("dt")) && holderOn(*layout, QStringLiteral("ftr")) && holderOn(*layout, QStringLiteral("sldNum")));
        QCOMPARE(holderOn(*layout, QStringLiteral("title"))->rect, holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("title"))->rect);
        QCOMPARE(w.masterPane()->ids().size(), 11);
        QCOMPARE(w.masterPane()->depthOf(made), 1);
        // A second one is named another way, and a slide can stand on it.
        const QString second = w.insertLayout();
        QCOMPARE(d->master(second)->name, QStringLiteral("Custom Layout 2"));
        w.editor()->setCurrentPage(1);
        w.newSlide(QStringLiteral("id:") + second);
        QCOMPARE(d->pages[2]->masterId, second);
        QVERIFY(holderOn(*d->pages[2], QStringLiteral("title")));
        // What a slide stands on stays, and the reason is given.
        w.editMasters(made);
        QString why;
        QVERIFY(!w.deleteMasterPage(used, &why));
        QVERIFY2(why.contains(QLatin1String("slide")), qPrintable(why));
        QVERIFY(d->master(used));
        QVERIFY(!w.deleteMasterPage(second, &why));
        QVERIFY(w.deleteMasterPage(made, &why));
        QVERIFY2(why.isEmpty(), qPrintable(why));
        QVERIFY(!d->master(made) && d->master(second));
        // The page being edited was taken away: the one before it is shown.
        QCOMPARE(w.editor()->masterView(), used);
        // Its text went with it.
        w.editor()->setCurrentPage(2);
        w.deleteSlide();
        const int storiesNow = int(d->stories.size());
        QVERIFY(w.deleteMasterPage(second, &why));
        QCOMPARE(int(d->masters.size()), pages);
        QCOMPARE(int(d->stories.size()), storiesNow - 4);   // its title, date, footer, and slide number
        w.editor()->undo();
        QVERIFY(d->master(second));
        w.editor()->undo();   // the slide
        w.editor()->undo();
        QVERIFY(d->master(made));
    }

    // Insert Slide Master adds a slide master with the nine layouts after the one being edited, and opens it; a slide master no slide stands on
    // can be deleted (with its layouts), the last one cannot, and neither can one with slides on it.
    void slideMastersAreInsertedAndDeleted()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two"});
        Document *d = w.editor()->doc();
        QString why;
        QVERIFY(!w.deleteMasterPage(QStringLiteral("SM1"), &why));
        QVERIFY2(!why.isEmpty(), "the only slide master stays");
        w.editMasters(d->pages[0]->masterId);
        const QString made = w.insertSlideMaster();
        const MasterPage *master = d->master(made);
        QVERIFY(master && deck::isSlideMaster(*master));
        QCOMPARE(master->name, QStringLiteral("Slide Master 2"));
        QCOMPARE(deck::layouts(*d, made).size(), 9);
        QCOMPARE(int(d->masters.size()), 20);
        QCOMPARE(w.editor()->masterView(), made);
        QCOMPARE(w.masterPane()->ids().size(), 20);
        QCOMPARE(w.masterPane()->depthOf(made), 0);
        // Its pages come right after the first master's, and the ids are all different.
        QSet<QString> ids;
        for (const auto &m : d->masters) ids.insert(m->id);
        QCOMPARE(ids.size(), 20);
        QCOMPARE(d->masters[10]->id, made);
        // A slide on one of its layouts keeps it from being deleted.
        const MasterPage *layout = deck::layoutOfType(*d, QStringLiteral("obj"), made);
        QVERIFY(layout);
        w.editor()->setCurrentPage(1);
        w.setLayout(QStringLiteral("id:") + layout->id);
        QCOMPARE(d->pages[1]->masterId, layout->id);
        QCOMPARE(deck::slideMasterOf(*d, 1), made);
        QVERIFY(!w.deleteMasterPage(made, &why));
        QVERIFY2(why.contains(QLatin1String("slide")), qPrintable(why));
        w.setLayout(QStringLiteral("id:") + deck::layoutOfType(*d, QStringLiteral("obj"), QStringLiteral("SM1"))->id);
        QCOMPARE(deck::slideMasterOf(*d, 1), QStringLiteral("SM1"));
        const int stories = int(d->stories.size());
        QVERIFY(w.deleteMasterPage(made, &why));
        QCOMPARE(int(d->masters.size()), 10);
        QVERIFY(d->stories.size() < stories);
        QVERIFY(w.editor()->masterView().isEmpty() || d->master(w.editor()->masterView()));
        w.editor()->undo();
        QCOMPARE(int(d->masters.size()), 20);
        QVERIFY(d->master(made));
    }

    // Deleting the first slide master hands the presentation's theme to the next one.
    void deletingTheFirstSlideMasterKeepsTheTheme()
    {
        DeckWindow w;
        Document *d = w.editor()->doc();
        w.editMasters(QStringLiteral("SM1"));
        const QString made = w.insertSlideMaster();
        w.closeMasters();
        const QString theme = deck::themeOf(*d).name;
        w.setLayout(QStringLiteral("id:") + deck::layoutOfType(*d, QStringLiteral("obj"), made)->id);
        w.setLayout(QStringLiteral("id:") + deck::layoutOfType(*d, QStringLiteral("title"), made)->id);
        QString why;
        QVERIFY2(w.deleteMasterPage(QStringLiteral("SM1"), &why), qPrintable(why));
        QCOMPARE(int(d->masters.size()), 10);
        QCOMPARE(deck::themeOf(*d).name, theme);
        QCOMPARE(deck::slideMasterOf(*d, 0), made);
        QVERIFY(!deck::exportPresentation(*d).isEmpty());
    }

    // The slide master that has the presentation's theme can be deleted once no slide stands on it: the next slide master's theme becomes the
    // presentation's.
    void deletingTheFirstSlideMasterHandsOnTheThemeOfTheNext()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two"});
        Document *d = w.editor()->doc();
        const QString first = deck::themeOf(*d).name;
        // Both slides go to a slide master of a theme of their own (the second time reuses the first's).
        w.editor()->setCurrentPage(0);
        w.applyTheme(1, false);
        w.editor()->setCurrentPage(1);
        w.applyTheme(1, false);
        QCOMPARE(deck::themeOf(*d).name, first);
        QString second;
        for (const auto &m : d->masters)
            if (deck::isSlideMaster(*m) && m->id != QLatin1String("SM1")) second = m->id;
        QVERIFY(!second.isEmpty());
        QCOMPARE(deck::slideMasterOf(*d, 0), second);
        QCOMPARE(deck::slideMasterOf(*d, 1), second);
        QString why;
        QVERIFY2(w.deleteMasterPage(QStringLiteral("SM1"), &why), qPrintable(why));
        QCOMPARE(deck::themeOf(*d).name, deck::builtinThemeName(1));
        QCOMPARE(deck::themeOf(*d).headingFont, deck::builtinThemeFonts(1).heading);
        QCOMPARE(d->fonts.heading, deck::builtinThemeFonts(1).heading);
        // Saved and read again, the presentation has that theme.
        auto back = throughPptx(*d);
        QVERIFY(back);
        QCOMPARE(deck::themeOf(*back).headingFont, deck::builtinThemeFonts(1).heading);
        QCOMPARE(int(deck::layouts(*back).size()), 9);
    }

    // The commands that work on a slide wait while a master or layout is open, and come back as they were (Delete Slide stays off in a
    // presentation of one slide).
    void slideCommandsComeBackWithTheirOwnState()
    {
        DeckWindow w;
        QVERIFY(!w.act(QStringLiteral("slide.delete"))->isEnabled());
        w.editMasters();
        for (const char *id : {"slide.new", "slide.duplicate", "slide.reset", "insert.headerFooter", "insert.video", "section.add", "comment.new"})
            QVERIFY2(!w.act(QString::fromLatin1(id))->isEnabled(), id);
        w.closeMasters();
        for (const char *id : {"slide.new", "slide.duplicate", "slide.reset", "insert.headerFooter", "insert.video", "section.add", "comment.new"})
            QVERIFY2(w.act(QString::fromLatin1(id))->isEnabled(), id);
        QVERIFY(!w.act(QStringLiteral("slide.delete"))->isEnabled());
        w.newSlide();
        w.editMasters();
        QVERIFY(!w.act(QStringLiteral("slide.delete"))->isEnabled());
        w.closeMasters();
        QVERIFY(w.act(QStringLiteral("slide.delete"))->isEnabled());
    }

    // Master Layout's dialog has a box for each place of the slide master, and says which are checked.
    void masterLayoutDialogHasABoxForEachPlace()
    {
        deck::MasterParts parts;
        parts.title = parts.date = true;
        MasterLayoutDialog dlg(parts);
        QCOMPARE(dlg.findChildren<QCheckBox *>().size(), 5);
        QCOMPARE(dlg.parts(), parts);
        dlg.findChild<QCheckBox *>(QStringLiteral("footer"))->setChecked(true);
        dlg.findChild<QCheckBox *>(QStringLiteral("title"))->setChecked(false);
        parts.footer = true;
        parts.title = false;
        QCOMPARE(dlg.parts(), parts);
    }

    // Rename changes the name of a slide master or layout, as one step; nothing is renamed to nothing.
    void masterPagesAreRenamed()
    {
        DeckWindow w;
        Document *d = w.editor()->doc();
        const QString layout = d->pages[0]->masterId;
        w.editMasters(layout);
        const int steps = w.editor()->undoStack()->count();
        w.renameMasterPage(layout, QStringLiteral("Opening"));
        w.renameMasterPage(QStringLiteral("SM1"), QStringLiteral("House Style"));
        QCOMPARE(d->master(layout)->name, QStringLiteral("Opening"));
        QCOMPARE(d->master(QStringLiteral("SM1"))->name, QStringLiteral("House Style"));
        QCOMPARE(w.editor()->undoStack()->count(), steps + 2);
        w.renameMasterPage(layout, QStringLiteral("   "));
        QCOMPARE(d->master(layout)->name, QStringLiteral("Opening"));
        QCOMPARE(w.editor()->undoStack()->count(), steps + 2);
        QCOMPARE(w.masterPane()->nameOf(layout), QStringLiteral("Opening"));
        w.editor()->undo();
        QCOMPARE(d->master(QStringLiteral("SM1"))->name, QStringLiteral("Slide Master"));
        // The names are what a saved file calls them: the new one of the layout (and not the old), and the master's own.
        w.renameMasterPage(QStringLiteral("SM1"), QStringLiteral("House Style"));
        QMap<QString, QByteArray> zip;
        auto back = throughPptx(*d, &zip);
        QVERIFY(back);
        QStringList masters, layouts;
        for (const auto &m : back->masters) (deck::isSlideMaster(*m) ? masters : layouts) << m->name;
        QCOMPARE(masters, QStringList{"House Style"});
        QVERIFY(layouts.contains(QStringLiteral("Opening")));
        QVERIFY(!layouts.contains(QStringLiteral("Title Slide")));
        QCOMPARE(layouts.size(), 9);
        QVERIFY(zip.value(QStringLiteral("ppt/slideMasters/slideMaster1.xml")).contains("<p:cSld name=\"House Style\">"));
        QVERIFY(zip.value(QStringLiteral("ppt/slideLayouts/slideLayout1.xml")).contains("<p:cSld name=\"Opening\">"));
    }

    // Insert Placeholder puts a placeholder of the kind on the layout, selected and ready to place, with the next number its placeholders
    // have; a slide put on the layout has it, empty. Each is one step.
    void placeholdersAreInsertedOnALayout()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two"});
        Document *d = w.editor()->doc();
        const MasterPage *layout = deck::layoutOfType(*d, QStringLiteral("titleOnly"));
        QVERIFY(layout);
        const QString id = layout->id;
        w.editMasters(id);
        const int items = int(layout->items.size());
        struct Kind { deck::PlaceholderKind kind; const char *role; };
        int n = 0;
        for (const Kind &k : {Kind{deck::PlaceholderKind::Content, "ph:obj:1"}, Kind{deck::PlaceholderKind::Text, "ph:body:2"}, Kind{deck::PlaceholderKind::Picture, "ph:pic:3"},
                              Kind{deck::PlaceholderKind::Chart, "ph:chart:4"}, Kind{deck::PlaceholderKind::Table, "ph:tbl:5"}, Kind{deck::PlaceholderKind::Media, "ph:media:6"},
                              Kind{deck::PlaceholderKind::OnlineImage, "ph:clipArt:7"}}) {
            const int steps = w.editor()->undoStack()->count();
            const QString made = w.insertPlaceholder(k.kind);
            QVERIFY2(!made.isEmpty(), k.role);
            const Item *it = d->item(made);
            QVERIFY(it);
            QCOMPARE(it->role, QString::fromLatin1(k.role));
            QCOMPARE(w.editor()->selection(), QStringList{made});
            QVERIFY(it->rect.width() > 50 && it->rect.height() > 30 && QRectF(0, 0, 960, 540).contains(it->rect));
            QCOMPARE(w.editor()->undoStack()->count(), steps + 1);
            QCOMPARE(int(d->master(id)->items.size()), items + ++n);
        }
        // A second one of a kind doesn't sit exactly on the first.
        const QString again = w.insertPlaceholder(deck::PlaceholderKind::Content);
        QCOMPARE(d->item(again)->role, QStringLiteral("ph:obj:8"));
        for (const auto &it : d->master(id)->items)
            if (it->id != again && it->role == QLatin1String("ph:obj:1")) QVERIFY(it->rect != d->item(again)->rect);
        // A slide on the layout has the places, with nothing in them.
        w.editor()->setCurrentPage(1);
        w.setLayout(QStringLiteral("id:") + id);
        QVERIFY(holderOn(*d->pages[1], QStringLiteral("chart")) && holderOn(*d->pages[1], QStringLiteral("pic")) && holderOn(*d->pages[1], QStringLiteral("media")));
        QVERIFY(d->storyDoc(holderOn(*d->pages[1], QStringLiteral("tbl"))->storyId)->isEmpty());
        // On a layout only: a slide master has none of these.
        w.editMasters(QStringLiteral("SM1"));
        QVERIFY(w.insertPlaceholder(deck::PlaceholderKind::Chart).isEmpty());
        QVERIFY(!w.act(QStringLiteral("master.placeholder.chart"))->isEnabled());
    }

    // The Title and Footers boxes of a layout give it a title and the date, footer, and slide number, as the slide master has them, and
    // take them away.
    void layoutTitleAndFootersAreBoxes()
    {
        DeckWindow w;
        Document *d = w.editor()->doc();
        const MasterPage *blank = deck::layoutOfType(*d, QStringLiteral("blank"));
        QVERIFY(blank);
        const QString id = blank->id;
        w.editMasters(id);
        QVERIFY(!w.act(QStringLiteral("master.title"))->isChecked() && w.act(QStringLiteral("master.footers"))->isChecked());
        QVERIFY(!deck::layoutHasTitle(*blank) && deck::layoutHasFooters(*blank));
        w.setLayoutTitle(true);
        QVERIFY(deck::layoutHasTitle(*d->master(id)));
        QCOMPARE(holderOn(*d->master(id), QStringLiteral("title"))->rect, holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("title"))->rect);
        QVERIFY(w.act(QStringLiteral("master.title"))->isChecked());
        w.setLayoutFooters(false);
        QVERIFY(!deck::layoutHasFooters(*d->master(id)));
        QVERIFY(!holderOn(*d->master(id), QStringLiteral("dt")) && !holderOn(*d->master(id), QStringLiteral("ftr")) && !holderOn(*d->master(id), QStringLiteral("sldNum")));
        QVERIFY(!w.act(QStringLiteral("master.footers"))->isChecked());
        w.setLayoutFooters(true);
        QVERIFY(holderOn(*d->master(id), QStringLiteral("dt")) && holderOn(*d->master(id), QStringLiteral("ftr")) && holderOn(*d->master(id), QStringLiteral("sldNum")));
        w.setLayoutTitle(false);
        QVERIFY(!deck::layoutHasTitle(*d->master(id)));
        w.editor()->undo();
        QVERIFY(deck::layoutHasTitle(*d->master(id)));
        // Not on a slide master, which has its own boxes.
        w.editMasters(QStringLiteral("SM1"));
        QVERIFY(!w.act(QStringLiteral("master.title"))->isEnabled() && !w.act(QStringLiteral("master.footers"))->isEnabled());
    }

    // Master Layout says which places the slide master has: the title, text, date, slide number, and footer.
    void masterLayoutChoosesThePlacesOfTheMaster()
    {
        DeckWindow w;
        Document *d = w.editor()->doc();
        w.editMasters(QStringLiteral("SM1"));
        const MasterPage *master = d->master(QStringLiteral("SM1"));
        deck::MasterParts parts = deck::masterParts(*master);
        QVERIFY(parts.title && parts.text && parts.date && parts.number && parts.footer);
        const int steps = w.editor()->undoStack()->count();
        parts.date = parts.footer = false;
        w.setMasterParts(parts);
        QCOMPARE(w.editor()->undoStack()->count(), steps + 1);
        QVERIFY(!holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("dt")) && !holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("ftr")));
        QVERIFY(holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("sldNum")));
        // The layouts keep theirs.
        QVERIFY(holderOn(*deck::layoutOfType(*d, QStringLiteral("obj")), QStringLiteral("dt")));
        parts.date = true;
        parts.text = false;
        w.setMasterParts(parts);
        QCOMPARE(holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("dt"))->rect, QRectF(66, 500.5, 216, 28.75));
        QVERIFY(!holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("body")));
        QCOMPARE(deck::masterParts(*d->master(QStringLiteral("SM1"))), parts);
        // Saved without them, and read back without them.
        auto back = throughPptx(*d);
        QVERIFY(back);
        QCOMPARE(deck::masterParts(*back->masters[0]), parts);
        // Only on a slide master.
        w.editMasters(d->pages[0]->masterId);
        QVERIFY(!w.act(QStringLiteral("master.layout"))->isEnabled());
    }

    // Hide Background Graphics on a layout stops the slide master's graphics showing on its slides, and is written to the file.
    void hidingBackgroundGraphicsOnALayout()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two"});
        Document *d = w.editor()->doc();
        w.editMasters(QStringLiteral("SM1"));
        auto band = std::make_shared<ShapeItem>();
        band->shape = QStringLiteral("rect");
        band->rect = QRectF(0, 480, 960, 60);
        band->fill = Fill::solid(ColorRef::rgb(QColor(0x65, 0x43, 0x21)));
        band->stroke = Stroke();
        w.editor()->addItem(band, false);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d;
        ctx.cache = &cache;
        auto bandAt = [&](int slide) { return Renderer::renderToImage(ctx, slide, 1).pixelColor(480, 510) == QColor(0x65, 0x43, 0x21); };
        QVERIFY(bandAt(1));
        const QString layout = d->pages[1]->masterId;
        w.editMasters(layout);
        QVERIFY(!w.act(QStringLiteral("master.hideBackground"))->isChecked());
        w.hideBackgroundGraphics(true);
        QVERIFY(!d->master(layout)->showMasterItems);
        QVERIFY(w.act(QStringLiteral("master.hideBackground"))->isChecked());
        QVERIFY(!bandAt(1) && bandAt(0));
        QMap<QString, QByteArray> zip;
        auto back = throughPptx(*d, &zip);
        QVERIFY(back);
        QStringList hidden;
        for (const auto &m : back->masters)
            if (deck::isLayout(*m) && !m->showMasterItems) hidden << m->name;
        QCOMPARE(hidden, QStringList{"Title and Content"});
        w.editor()->undo();
        QVERIFY(bandAt(1));
    }

    // Hide Background Graphics on a slide (the Format Background box) does the same for that slide alone, and is saved with it.
    void hidingBackgroundGraphicsOnASlide()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two"});
        Document *d = w.editor()->doc();
        w.editMasters(QStringLiteral("SM1"));
        auto band = std::make_shared<ShapeItem>();
        band->shape = QStringLiteral("rect");
        band->rect = QRectF(0, 480, 960, 60);
        band->fill = Fill::solid(ColorRef::rgb(QColor(0x65, 0x43, 0x21)));
        band->stroke = Stroke();
        w.editor()->addItem(band, false);
        w.closeMasters();
        w.editor()->setCurrentPage(1);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d;
        ctx.cache = &cache;
        auto bandAt = [&](int slide) { return Renderer::renderToImage(ctx, slide, 1).pixelColor(480, 510) == QColor(0x65, 0x43, 0x21); };
        QVERIFY(!w.act(QStringLiteral("master.hideBackground"))->isEnabled());
        QVERIFY(bandAt(0) && bandAt(1));
        w.hideBackgroundGraphics(true);
        QVERIFY(!d->pages[1]->showMasterItems && d->pages[0]->showMasterItems);
        QVERIFY(bandAt(0) && !bandAt(1));
        QMap<QString, QByteArray> zip;
        auto back = throughPptx(*d, &zip);
        QVERIFY(back);
        QVERIFY(back->pages[0]->showMasterItems && !back->pages[1]->showMasterItems);
        QVERIFY(zip.value(QStringLiteral("ppt/slides/slide2.xml")).contains("showMasterSp=\"0\""));
        w.editor()->undo();
        QVERIFY(d->pages[1]->showMasterItems && bandAt(1));
        // The Format Background box says what the slide has, and sets it.
        BackgroundDialog dlg(Qt::white, d->colors);
        dlg.setHideGraphics(true);
        QVERIFY(dlg.hideGraphics());
        QVERIFY(dlg.findChild<QCheckBox *>(QStringLiteral("hideGraphics")));
    }

    // Colors, fonts, and background styles chosen while a slide master is open change the presentation as they do in Normal view: text that
    // lies on a picture stays as the readable color it was kept in, and is not carried to the layout's new color as an edit of the layout.
    void designChangesInMasterViewAreTheSameAsInNormalView()
    {
        auto run = [&](bool inMaster) {
            DeckWindow w;
            fillSlides(w, {"One", "Two"}, {{"sub"}, {"a", "b"}});
            Document *d = w.editor()->doc();
            // A white picture over the whole of slide 2, under its words.
            auto picture = std::make_shared<PictureItem>();
            picture->imageId = d->addImage(odptest::png(40, 30, Qt::white), QStringLiteral("png"));
            picture->rect = QRectF(0, 0, 960, 540);
            picture->imgRect = QRectF(0, 0, 960, 540);
            d->pages[1]->items.insert(d->pages[1]->items.begin(), picture);
            if (inMaster) w.editMasters(d->pages[1]->masterId);
            w.chooseFonts(deck::fontSets().value(2));
            w.chooseColors(deck::colorSets().value(3));
            w.chooseBackgroundStyle(11);   // Dark 1
            const TextItem *title = holderOn(*d->pages[1], QStringLiteral("title"));
            const TextItem *body = holderOn(*d->pages[1], QStringLiteral("obj"));
            const TextItem *first = holderOn(*d->pages[0], QStringLiteral("ctrTitle"));
            const TextItem *master = holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("title"));
            auto color = [&](const TextItem *t) { return firstLetterOf(*d, *t).foreground().color().name(); };
            return QStringList{familyOfHolder(*d, *title), color(title), color(body), color(first), familyOfHolder(*d, *master), color(master),
                               d->master(QStringLiteral("SM1"))->background.color.resolve(d->colors).name(), QString::number(w.editor()->undoStack()->count())};
        };
        const QStringList normal = run(false);
        QCOMPARE(run(true), normal);
        // (The words on the picture did stay a color that reads on it, and the others turned light on the dark background.)
        QVERIFY(normal[1] != normal[3]);
    }

    // Preserve keeps a slide master or layout in the file whether or not slides stand on it; it is on unless it was turned off, and the file
    // says so either way.
    void preserveIsKeptInTheFile()
    {
        DeckWindow w;
        Document *d = w.editor()->doc();
        w.editMasters(QStringLiteral("SM1"));
        QVERIFY(w.act(QStringLiteral("master.preserve"))->isChecked());
        const QString layout = deck::layoutOfType(*d, QStringLiteral("blank"))->id;
        QMap<QString, QByteArray> zip;
        auto same = throughPptx(*d, &zip);
        QVERIFY(same && zip.value(QStringLiteral("ppt/slideMasters/slideMaster1.xml")).contains("preserve=\"1\""));
        w.setPreserve(layout, false);
        QVERIFY(!deck::isPreserved(*d->master(layout)));
        w.editMasters(layout);
        QVERIFY(!w.act(QStringLiteral("master.preserve"))->isChecked());
        auto back = throughPptx(*d, &zip);
        QVERIFY(back);
        int off = 0, on = 0;
        for (const auto &m : back->masters) {
            if (!deck::isLayout(*m)) continue;
            if (deck::isPreserved(*m)) ++on;
            else {
                ++off;
                QCOMPARE(m->name, QStringLiteral("Blank"));
            }
        }
        QCOMPARE(off, 1);
        QCOMPARE(on, 8);
        QVERIFY(zip.value(QStringLiteral("ppt/slideLayouts/slideLayout7.xml")).contains("preserve=\"0\""));
        w.setPreserve(layout, true);
        QVERIFY(deck::isPreserved(*d->master(layout)));
    }

    // An edited slide master, a second slide master, and a new layout with its own graphic and places are saved to a .pptx as the other program
    // writes them (the parts, their links, the layouts' ids, the places' types and numbers) and read back as they were.
    void editedMastersRoundTripThroughPptx()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two"});
        Document *d = w.editor()->doc();
        w.editMasters(QStringLiteral("SM1"));
        w.editor()->select(holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("title"))->id);
        w.editor()->setFontFamily(QStringLiteral("Georgia"));
        const QString second = w.insertSlideMaster();
        w.renameMasterPage(second, QStringLiteral("Annual Report"));
        const QString custom = w.insertLayout();
        w.renameMasterPage(custom, QStringLiteral("Big Chart"));
        w.insertPlaceholder(deck::PlaceholderKind::Chart);
        w.insertPlaceholder(deck::PlaceholderKind::Text);
        auto bar = std::make_shared<ShapeItem>();
        bar->shape = QStringLiteral("rect");
        bar->name = QStringLiteral("Bar");
        bar->rect = QRectF(0, 0, 960, 20);
        bar->fill = Fill::solid(ColorRef::rgb(QColor(0xAA, 0x22, 0x22)));
        w.editor()->addItem(bar, false);
        w.hideBackgroundGraphics(true);
        w.setPreserve(second, false);
        w.editor()->setCurrentPage(1);
        w.setLayout(QStringLiteral("id:") + custom);
        w.closeMasters();

        QMap<QString, QByteArray> zip;
        auto back = throughPptx(*d, &zip);
        QVERIFY(back);
        // The parts: two slide masters, nineteen layouts, each layout linked from its master with an id of its own.
        QStringList masterParts, layoutParts;
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) {
            if (it.key().startsWith(QLatin1String("ppt/slideMasters/slideMaster")) && it.key().endsWith(QLatin1String(".xml"))) masterParts << it.key();
            if (it.key().startsWith(QLatin1String("ppt/slideLayouts/slideLayout")) && it.key().endsWith(QLatin1String(".xml"))) layoutParts << it.key();
        }
        QCOMPARE(masterParts.size(), 2);
        QCOMPARE(layoutParts.size(), 19);
        QSet<QString> ids;
        int listed = 0;
        for (const QString &part : masterParts) {
            const QString xml = QString::fromUtf8(zip.value(part));
            QVERIFY(xml.contains(QLatin1String("<p:sldLayoutIdLst>")));
            QRegularExpressionMatchIterator m = QRegularExpression(QStringLiteral("<p:sldLayoutId id=\"(\\d+)\" r:id=\"(rId\\d+)\"/>")).globalMatch(xml);
            while (m.hasNext()) {
                const auto match = m.next();
                QVERIFY2(!ids.contains(match.captured(1)), "the layout ids are all different");
                ids.insert(match.captured(1));
                QVERIFY(match.captured(1).toLongLong() >= 2147483648LL && match.captured(1).toLongLong() <= 4294967295LL);
                ++listed;
                const QString rels = QString::fromUtf8(zip.value(part.section('/', 0, -2) + QStringLiteral("/_rels/") + part.section('/', -1) + QStringLiteral(".rels")));
                QVERIFY2(rels.contains(QLatin1String("Id=\"") + match.captured(2) + QLatin1String("\"")), qPrintable(part));
            }
        }
        QCOMPARE(listed, 19);
        // The custom layout: not one of the standard types, drawn by the user, with its places named by type and number.
        QString customXml;
        for (const QString &part : layoutParts) {
            const QString xml = QString::fromUtf8(zip.value(part));
            if (xml.contains(QLatin1String("name=\"Big Chart\""))) customXml = xml;
        }
        QVERIFY(!customXml.isEmpty());
        QVERIFY(customXml.contains(QLatin1String("userDrawn=\"1\"")) && !customXml.contains(QLatin1String(" type=\"cust\"")));
        QVERIFY(customXml.contains(QLatin1String("showMasterSp=\"0\"")));
        QVERIFY(customXml.contains(QRegularExpression(QStringLiteral("<p:ph type=\"chart\" idx=\"1\"/>"))));
        QVERIFY(customXml.contains(QRegularExpression(QStringLiteral("<p:ph type=\"body\" idx=\"2\"/>"))));
        QVERIFY(customXml.contains(QLatin1String("<p:ph type=\"title\"/>")));
        // Read back.
        QStringList masters, layouts;
        for (const auto &m : back->masters) (deck::isSlideMaster(*m) ? masters : layouts) << m->name;
        QCOMPARE(masters, QStringList({"Slide Master", "Annual Report"}));
        QCOMPARE(layouts.size(), 19);
        QVERIFY(layouts.contains(QStringLiteral("Big Chart")));
        const MasterPage *again = nullptr;
        for (const auto &m : back->masters)
            if (m->name == QLatin1String("Big Chart")) again = m.get();
        QVERIFY(again);
        QVERIFY(!again->showMasterItems);
        QVERIFY(holderOn(*again, QStringLiteral("chart")) && holderOn(*again, QStringLiteral("body")) && holderOn(*again, QStringLiteral("title")));
        QCOMPARE(holderOn(*again, QStringLiteral("chart"))->rect, holderOn(*d->master(custom), QStringLiteral("chart"))->rect);
        QCOMPARE(holderOn(*again, QStringLiteral("body"))->role, QStringLiteral("ph:body:2"));
        bool bar2 = false;
        for (const auto &it : again->items) bar2 = bar2 || it->name == QLatin1String("Bar");
        QVERIFY(bar2);
        QCOMPARE(back->pages[1]->masterId, again->id);
        QCOMPARE(back->masterFor(*back->pages[1])->name, QStringLiteral("Big Chart"));
        QCOMPARE(familyOfHolder(*back, *holderOn(*back->master(back->masters[0]->id), QStringLiteral("title"))), QStringLiteral("Georgia"));
        QCOMPARE(deck::slideMasterOf(*back, 1), back->masters[10]->id);
        // Preserve, as set.
        int notPreserved = 0;
        for (const auto &m : back->masters) notPreserved += deck::isPreserved(*m) ? 0 : 1;
        QCOMPARE(notPreserved, 1);
        QVERIFY(!deck::isPreserved(*back->masters[10]));
        // Saved again, the same.
        QMap<QString, QByteArray> zip2;
        auto twice = throughPptx(*back, &zip2);
        QVERIFY(twice);
        QCOMPARE(twice->masters.size(), back->masters.size());
        for (int i = 0; i < back->masters.size(); ++i) {
            QCOMPARE(twice->masters[i]->name, back->masters[i]->name);
            QCOMPARE(twice->masters[i]->baseId == back->masters[i]->baseId, true);
            QCOMPARE(deck::isPreserved(*twice->masters[i]), deck::isPreserved(*back->masters[i]));
            QCOMPARE(twice->masters[i]->showMasterItems, back->masters[i]->showMasterItems);
            QCOMPARE(twice->masters[i]->items.size(), back->masters[i]->items.size());
        }
        QCOMPARE(layoutParts.size(), 19);
    }

    // The same through a .odp: the slide master's edited title, and a layout with a graphic of its own that a slide stands on.
    void editedMastersRoundTripThroughOdp()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two"});
        Document *d = w.editor()->doc();
        w.editMasters(QStringLiteral("SM1"));
        w.editor()->select(holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("title"))->id);
        w.editor()->setFontFamily(QStringLiteral("Georgia"));
        w.editor()->setFontSize(36);
        const QString custom = w.insertLayout();
        w.renameMasterPage(custom, QStringLiteral("Banner"));
        auto bar = std::make_shared<ShapeItem>();
        bar->shape = QStringLiteral("rect");
        bar->name = QStringLiteral("Bar");
        bar->rect = QRectF(0, 0, 960, 20);
        bar->fill = Fill::solid(ColorRef::rgb(QColor(0xAA, 0x22, 0x22)));
        w.editor()->addItem(bar, false);
        w.editor()->setCurrentPage(1);
        w.setLayout(QStringLiteral("id:") + custom);
        w.closeMasters();
        QString err;
        QStringList skipped;
        const QByteArray odp = deck::exportOdp(*d, false, &err, &skipped);
        QVERIFY2(!odp.isEmpty(), qPrintable(err));
        auto back = deck::importOdp(odp, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->pages.size(), 2);
        const MasterPage *master = nullptr;
        for (const auto &m : back->masters)
            if (deck::isSlideMaster(*m)) master = m.get();
        QVERIFY(master);
        const TextItem *title = holderOn(*master, QStringLiteral("title"));
        QVERIFY(title);
        QCOMPARE(familyOfHolder(*back, *title), QStringLiteral("Georgia"));
        QCOMPARE(firstLetterOf(*back, *title).fontPointSize(), 36.0);
        // The slide on the layout with the bar has the bar under it, as before.
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = back.get();
        ctx.cache = &cache;
        const QImage first = Renderer::renderToImage(ctx, 1, 1);
        QCOMPARE(first.pixelColor(480, 10), QColor(0xAA, 0x22, 0x22));
        QVERIFY(Renderer::renderToImage(ctx, 0, 1).pixelColor(480, 10) != QColor(0xAA, 0x22, 0x22));
        // The first slide of the original is the same.
        QCOMPARE(titlesOf(*back), QStringList({"One", "Two"}));
    }

    // Every edit of Slide Master view is one step: a command of the tab, a change of the canvas, and nothing for opening and closing.
    void masterViewEditsAreOneStepEach()
    {
        DeckWindow w;
        Document *d = w.editor()->doc();
        const int steps = w.editor()->undoStack()->count();
        w.editMasters(QStringLiteral("SM1"));
        w.closeMasters();
        QCOMPARE(w.editor()->undoStack()->count(), steps);
        w.editMasters(QStringLiteral("SM1"));
        const QString layout = w.insertLayout();
        w.insertPlaceholder(deck::PlaceholderKind::Picture);
        w.setLayoutFooters(false);
        w.hideBackgroundGraphics(true);
        w.renameMasterPage(layout, QStringLiteral("X"));
        QCOMPARE(w.editor()->undoStack()->count(), steps + 5);
        for (int i = 0; i < 5; ++i) w.editor()->undo();
        QVERIFY(!d->master(layout));
        QCOMPARE(w.editor()->undoStack()->count(), steps + 5);
        QCOMPARE(w.editor()->undoStack()->index(), steps);
        for (int i = 0; i < 5; ++i) w.editor()->redo();
        QCOMPARE(d->master(layout)->name, QStringLiteral("X"));
        w.closeMasters();
        QCOMPARE(w.view(), DeckWindow::View::Normal);
        w.editor()->undo();
        QCOMPARE(d->master(layout)->name, QStringLiteral("Custom Layout"));
        // The page that edit was made on is shown again.
        QCOMPARE(w.view(), DeckWindow::View::Master);
        QCOMPARE(w.editor()->masterView(), layout);
    }

    // The presentations a person checks in the other program (saved into the folder JP_SHOT_DIR names, when it is set): an edited slide master, a
    // second slide master with a custom layout, every kind of placeholder, a slide master without its places, one with the first slide master
    // deleted, and a slide that hides the background graphics. Each is saved, read again, and found as it was.
    void savedMasterCasesForTheOtherProgram()
    {
        const QString dir = qEnvironmentVariable("JP_SHOT_DIR");
        auto save = [&](Document &d, const QString &name) {
            QString err;
            const QByteArray bytes = deck::exportPresentation(d, &err);
            QVERIFY2(!bytes.isEmpty(), qPrintable(err));
            if (dir.isEmpty()) return;
            QDir().mkpath(dir);
            QFile f(dir + QLatin1Char('/') + name + QStringLiteral(".pptx"));
            if (f.open(QIODevice::WriteOnly)) f.write(bytes);
        };
        auto band = [](const QRectF &r, const QColor &c) {
            auto b = std::make_shared<ShapeItem>();
            b->shape = QStringLiteral("rect");
            b->name = QStringLiteral("Band");
            b->rect = r;
            b->fill = Fill::solid(ColorRef::rgb(c));
            b->stroke = Stroke();
            return b;
        };
        {   // The slide master's title in another font and size, and a slide of its own.
            DeckWindow w;
            fillSlides(w, {"Opening", "Following the master", "Setting its own"}, {{"sub"}, {"Body text"}, {"Body text"}});
            Document *d = w.editor()->doc();
            QTextCursor c(d->storyDoc(holderOn(*d->pages[2], QStringLiteral("title"))->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat own;
            own.setFontFamilies({QStringLiteral("Courier New")});
            c.mergeCharFormat(own);
            w.editMasters(QStringLiteral("SM1"));
            w.editor()->select(holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("title"))->id);
            w.editor()->setFontFamily(QStringLiteral("Georgia"));
            w.editor()->setFontSize(36);
            w.editor()->toggleBold();
            w.closeMasters();
            save(*d, QStringLiteral("master-edited-title"));
            auto back = throughPptx(*d);
            QVERIFY(back);
            QCOMPARE(familyOfHolder(*back, *holderOn(*back->master(back->masters[0]->id), QStringLiteral("title"))), QStringLiteral("Georgia"));
            QCOMPARE(familyOfHolder(*back, *holderOn(*back->pages[1], QStringLiteral("title"))), QStringLiteral("Georgia"));
            QCOMPARE(familyOfHolder(*back, *holderOn(*back->pages[2], QStringLiteral("title"))), QStringLiteral("Courier New"));
        }
        {   // A second slide master with a custom layout: a chart place, a text place, a bar, no master graphics; not preserved.
            DeckWindow w;
            fillSlides(w, {"One", "Two"});
            Document *d = w.editor()->doc();
            w.editMasters(QStringLiteral("SM1"));
            w.editor()->addItem(band(QRectF(0, 0, 960, 12), QColor(0x2B, 0x7A, 0x78)), false);
            const QString second = w.insertSlideMaster();
            w.renameMasterPage(second, QStringLiteral("Annual Report"));
            const QString custom = w.insertLayout();
            w.renameMasterPage(custom, QStringLiteral("Big Chart"));
            w.insertPlaceholder(deck::PlaceholderKind::Chart);
            w.insertPlaceholder(deck::PlaceholderKind::Text);
            w.editor()->addItem(band(QRectF(0, 520, 960, 20), QColor(0xAA, 0x22, 0x22)), false);
            w.hideBackgroundGraphics(true);
            w.setPreserve(second, false);
            w.closeMasters();
            w.editor()->setCurrentPage(1);
            w.setLayout(QStringLiteral("id:") + custom);
            save(*d, QStringLiteral("master-second-master-custom-layout"));
        }
        {   // A layout with a place of every kind, and a slide on it.
            DeckWindow w;
            fillSlides(w, {"One", "Two"});
            Document *d = w.editor()->doc();
            w.editMasters(deck::layoutOfType(*d, QStringLiteral("titleOnly"))->id);
            for (auto kind : {deck::PlaceholderKind::Content, deck::PlaceholderKind::Text, deck::PlaceholderKind::Picture, deck::PlaceholderKind::Chart, deck::PlaceholderKind::Table,
                              deck::PlaceholderKind::Media, deck::PlaceholderKind::OnlineImage}) {
                const QString id = w.insertPlaceholder(kind);
                const double n = d->item(id)->role.section(':', 2).toDouble();
                d->item(id)->rect = QRectF(40 + 90 * (n - 1), 200, 80, 120);
            }
            const QString layout = w.editor()->masterView();
            w.closeMasters();
            w.editor()->setCurrentPage(1);
            w.setLayout(QStringLiteral("id:") + layout);
            save(*d, QStringLiteral("master-every-kind-of-place"));
            auto back = throughPptx(*d);
            QVERIFY(back);
            int kinds = 0;
            for (const char *type : {"obj", "body", "pic", "chart", "tbl", "media", "clipArt"})
                if (holderOn(*back->masterFor(*back->pages[1]), QString::fromLatin1(type))) ++kinds;
            QCOMPARE(kinds, 6);   // (a content place reads back as a text place of the same number: both are a place with no type)
        }
        {   // A slide master without its text, date, and footer places.
            DeckWindow w;
            fillSlides(w, {"One", "Two"});
            Document *d = w.editor()->doc();
            w.editMasters(QStringLiteral("SM1"));
            deck::MasterParts parts = deck::masterParts(*d->master(QStringLiteral("SM1")));
            parts.text = parts.date = parts.footer = false;
            w.setMasterParts(parts);
            w.closeMasters();
            save(*d, QStringLiteral("master-without-places"));
        }
        {   // The first slide master deleted, its theme handed on.
            DeckWindow w;
            fillSlides(w, {"One", "Two"});
            Document *d = w.editor()->doc();
            w.editor()->setCurrentPage(0);
            w.applyTheme(1, false);
            w.editor()->setCurrentPage(1);
            w.applyTheme(1, false);
            QString why;
            QVERIFY2(w.deleteMasterPage(QStringLiteral("SM1"), &why), qPrintable(why));
            save(*d, QStringLiteral("master-first-deleted"));
        }
        {   // A slide that hides the background graphics.
            DeckWindow w;
            fillSlides(w, {"One", "Two"});
            Document *d = w.editor()->doc();
            w.editMasters(QStringLiteral("SM1"));
            w.editor()->addItem(band(QRectF(0, 480, 960, 60), QColor(0x65, 0x43, 0x21)), false);
            w.closeMasters();
            w.editor()->setCurrentPage(1);
            w.hideBackgroundGraphics(true);
            save(*d, QStringLiteral("slide-hides-background-graphics"));
        }
    }

    // A slide whose text place says "text" over a layout whose place says "content" (what an old binary file gives, and some .pptx) follows
    // the slide master's edits as any other does: its text's size, and then its place.
    void masterEditsReachATextPlaceOverAContentPlace()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two"}, {{"sub"}, {"a", "b"}});
        Document *d = w.editor()->doc();
        const QString layout = d->pages[1]->masterId;
        holderOn(*d->pages[1], QStringLiteral("obj"))->role = QStringLiteral("ph:body:1");
        QVERIFY(holderOn(*d->master(layout), QStringLiteral("obj")) && !holderOn(*d->master(layout), QStringLiteral("body")));
        w.editMasters(QStringLiteral("SM1"));
        w.editor()->select(holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("body"))->id);
        w.editor()->setFontSize(40);
        QCOMPARE(firstLetterOf(*d, *holderOn(*d->pages[1], QStringLiteral("body"))).fontPointSize(), 40.0);
        const QRectF moved(100, 200, 700, 250);
        w.editor()->change(QStringLiteral("Move"), [&] { holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("body"))->rect = moved; });
        QCOMPARE(holderOn(*d->master(layout), QStringLiteral("obj"))->rect, moved);
        QCOMPARE(holderOn(*d->pages[1], QStringLiteral("body"))->rect, moved);
    }

    // The same on a file that is read: a .pptx made in code whose slide says "body" over the layout's content place.
    void masterEditsReachTheSlidesOfAReadFile()
    {
        auto src = deck::newPresentation();
        deck::addSlide(*src, *deck::layoutOfType(*src, QStringLiteral("obj")));
        QString err;
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(deck::exportPresentation(*src), zip, &err));
        QByteArray &slide = zip[QStringLiteral("ppt/slides/slide2.xml")];
        QVERIFY(slide.contains("<p:ph idx=\"1\"/>"));
        slide.replace("<p:ph idx=\"1\"/>", "<p:ph type=\"body\" idx=\"1\"/>");
        ZipWriter z;
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) z.add(it.key(), it.value(), true);
        auto doc = deck::importPresentation(z.finish(), &err);
        QVERIFY2(doc, qPrintable(err));
        QCOMPARE(doc->pages.size(), 2);
        QVERIFY(holderOn(*doc->pages[1], QStringLiteral("body")) && !holderOn(*doc->pages[1], QStringLiteral("obj")));
        DeckWindow w;
        w.editor()->setDocument(std::move(doc));
        Document *d = w.editor()->doc();
        const QString master = deck::slideMasterOf(*d, 1);
        const QString layout = d->pages[1]->masterId;
        const TextItem *was = holderOn(*d->pages[1], QStringLiteral("body"));
        const QRectF where = was->rect;
        const double size = firstLetterOf(*d, *was).fontPointSize();
        w.editMasters(master);
        TextItem *place = holderOn(*d->master(master), QStringLiteral("body"));
        QVERIFY(place);
        w.editor()->select(place->id);
        w.editor()->setFontSize(size + 6);
        QCOMPARE(firstLetterOf(*d, *holderOn(*d->pages[1], QStringLiteral("body"))).fontPointSize(), size + 6);
        const QRectF moved = where.translated(0, 12);
        w.editor()->change(QStringLiteral("Move"), [&] { place->rect = moved; });
        QVERIFY(holderOn(*d->master(layout), QStringLiteral("obj")) || holderOn(*d->master(layout), QStringLiteral("body")));
        QCOMPARE(holderOn(*d->pages[1], QStringLiteral("body"))->rect, moved);
    }

    // A slide master (or its layouts) changed in Slide Master view is the user's from then on: the next change of theme for some slides does not
    // take it away, and Preserve says so. One that was only looked at is still the theme's.
    void aSlideMasterChangedInMasterViewIsKept()
    {
        auto run = [&](const std::function<void(DeckWindow &, const QString &)> &edit, bool kept) {
            DeckWindow w;
            fillSlides(w, {"One", "Two"});
            Document *d = w.editor()->doc();
            // A theme for the first slide only: it stands on a slide master of its own.
            w.editor()->setCurrentPage(0);
            w.applyTheme(1, false);
            const QString own = deck::slideMasterOf(*d, 0);
            QVERIFY(!own.isEmpty() && own != QLatin1String("SM1"));
            const int layouts = int(deck::layouts(*d, own).size());
            w.editMasters(d->pages[0]->masterId);
            QVERIFY(!w.act(QStringLiteral("master.preserve"))->isChecked());
            edit(w, own);
            QCOMPARE(w.act(QStringLiteral("master.preserve"))->isChecked(), kept);
            w.closeMasters();
            w.editor()->setCurrentPage(0);
            w.applyTheme(2, false);
            QCOMPARE(d->master(own) != nullptr, kept);
            if (kept) QVERIFY(int(deck::layouts(*d, own).size()) >= layouts);
        };
        run([](DeckWindow &, const QString &) {}, false);
        run([](DeckWindow &w, const QString &) { QVERIFY(!w.insertLayout().isEmpty()); }, true);
        run([](DeckWindow &w, const QString &own) { w.renameMasterPage(own, QStringLiteral("Mine")); }, true);
        run([](DeckWindow &w, const QString &) { w.insertPlaceholder(deck::PlaceholderKind::Picture); w.setLayoutFooters(false); }, true);
        run([](DeckWindow &w, const QString &own) { w.setPreserve(own, true); }, true);
    }

    // Every command that works on a slide is off while a master or layout is open, in the ribbon and in the slide's menu; each is on again
    // when the view closes.
    void everyCommandThatActsOnASlideIsOffInMasterView()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two"});
        w.editor()->setCurrentPage(1);
        auto command = [&](const QString &id) -> QAction * {
            QAction *a = w.act(id);
            return a ? a : w.findChild<QAction *>(id);
        };
        const QStringList ids = {"slide.new", "slide.duplicate", "slide.delete", "slide.hide", "slide.reset", "slide.reuse", "ribbon.layout", "ribbon.section", "section.add",
                                 "comment.new", "insert.headerFooter", "insert.video", "insert.audio", "ribbon.video", "ribbon.audio"};
        for (const QString &id : ids) {
            QVERIFY2(command(id), qPrintable(id));
            QTRY_VERIFY2(command(id)->isEnabled(), qPrintable(id + " off in Normal view"));
        }
        w.editMasters();
        for (const QString &id : ids) QVERIFY2(!command(id)->isEnabled(), qPrintable(id + " on in master view"));
        // Their buttons too.
        int buttons = 0;
        for (const QString &tab : {QStringLiteral("Home"), QStringLiteral("Insert")})
            for (const QToolButton *b : w.ribbon()->tab(tab)->findChildren<QToolButton *>())
                if (b->defaultAction() && ids.contains(b->defaultAction()->objectName())) {
                    ++buttons;
                    QVERIFY2(!b->isEnabled(), qPrintable(b->defaultAction()->objectName() + " button"));
                }
        QVERIFY(buttons >= 8);
        // The menu of an empty place on the canvas has no Layout or Reset Slide.
        w.editor()->clearSelection();
        std::unique_ptr<QMenu> menu(w.slideContextMenu(&w));
        for (const QAction *a : menu->actions()) {
            QVERIFY2(a->text() != QLatin1String("Layout") && a->text() != QLatin1String("Reset Slide"), qPrintable(a->text()));
        }
        w.closeMasters();
        for (const QString &id : ids)
            if (id != QLatin1String("section.add")) QVERIFY2(command(id)->isEnabled(), qPrintable(id + " off after"));
        w.editor()->clearSelection();
        menu.reset(w.slideContextMenu(&w));
        QStringList texts;
        for (const QAction *a : menu->actions()) texts << a->text();
        QVERIFY(texts.contains(QStringLiteral("Layout")) && texts.contains(QStringLiteral("Reset Slide")));
    }

    // Insert Placeholder takes a number no other place on the layout has, the smallest from 1 up, however large the others are (a file may
    // number a place 4294967295); none gets the title's 0.
    void insertedPlaceholdersGetFreeNumbers()
    {
        DeckWindow w;
        Document *d = w.editor()->doc();
        const MasterPage *layout = deck::layoutOfType(*d, QStringLiteral("titleOnly"));
        const QString id = layout->id;
        holderOn(*layout, QStringLiteral("ftr"))->role = QStringLiteral("ph:ftr:4294967295");
        w.editMasters(id);
        QStringList made;
        for (auto kind : {deck::PlaceholderKind::Content, deck::PlaceholderKind::Picture, deck::PlaceholderKind::Chart}) made << w.insertPlaceholder(kind);
        QSet<qint64> numbers;
        for (const auto &it : d->master(id)->items) {
            const QString n = it->role.section(':', 2);
            const qint64 number = n.isEmpty() ? 0 : n.toLongLong();
            if (number == 0) QVERIFY2(deck::placeholderType(*it) == QLatin1String("title") || deck::placeholderType(*it) == QLatin1String("dt") || deck::placeholderType(*it) == QLatin1String("sldNum"), qPrintable(it->role));
            else QVERIFY2(!numbers.contains(number), qPrintable(it->role));
            numbers.insert(number);
        }
        QVERIFY(numbers.contains(4294967295LL));
        QStringList roles;
        for (const QString &m : made) roles << d->item(m)->role;
        QCOMPARE(roles, QStringList({"ph:obj:1", "ph:pic:2", "ph:chart:3"}));
    }

    // Master Layout: a place unchecked and checked again comes back in the slide master's own look for it (a Georgia title of 60 points stays so),
    // as the slide master's own text style, not the stock one.
    void aMasterPlaceComesBackInTheMastersOwnStyle()
    {
        DeckWindow w;
        Document *d = w.editor()->doc();
        w.editMasters(QStringLiteral("SM1"));
        w.editor()->select(holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("title"))->id);
        w.editor()->setFontFamily(QStringLiteral("Georgia"));
        w.editor()->setFontSize(60);
        deck::MasterParts parts = deck::masterParts(*d->master(QStringLiteral("SM1")));
        parts.title = false;
        w.setMasterParts(parts);
        QVERIFY(!holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("title")));
        parts.title = true;
        w.setMasterParts(parts);
        const TextItem *title = holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("title"));
        QVERIFY(title);
        QCOMPARE(familyOfHolder(*d, *title), QStringLiteral("Georgia"));
        QCOMPARE(firstLetterOf(*d, *title).fontPointSize(), 60.0);
        // The same for the text place, and where no look was kept it is the stock one.
        w.editor()->select(holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("body"))->id);
        w.editor()->setFontSize(31);
        parts.text = false;
        w.setMasterParts(parts);
        parts.text = true;
        w.setMasterParts(parts);
        QCOMPARE(firstLetterOf(*d, *holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("body"))).fontPointSize(), 31.0);
        QVERIFY(holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("title"))->rect == QRectF(66, 28.75, 828, 104.4));
        // Undone, the title is the one before.
        w.editor()->undo();
        w.editor()->undo();
        QCOMPARE(firstLetterOf(*d, *holderOn(*d->master(QStringLiteral("SM1")), QStringLiteral("body"))).fontPointSize(), 31.0);
    }

    // An edit that touches no placeholder (a band drawn on a layout, Preserve) leaves every slide as it was: its text is not set again, so the
    // lists in it are not regrouped.
    void anEditThatTouchesNoPlaceholderLeavesTheSlidesAlone()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two", "Three"}, {{"sub"}, {"a", "b", "c"}, {"d", "e"}});
        Document *d = w.editor()->doc();
        const QString layout = d->pages[1]->masterId;
        // The layout's text place and the slides' bodies are bulleted lists.
        auto bulleted = [&](QTextDocument *story) {
            QTextCursor c(story);
            c.select(QTextCursor::Document);
            QTextListFormat lf;
            lf.setStyle(QTextListFormat::ListDisc);
            lf.setIndent(1);
            c.createList(lf);
        };
        bulleted(d->storyDoc(holderOn(*d->master(layout), QStringLiteral("obj"))->storyId));
        for (int i : {1, 2}) bulleted(d->storyDoc(holderOn(*d->pages[i], QStringLiteral("obj"))->storyId));
        QSignalSpy spy(d->storyDoc(holderOn(*d->pages[1], QStringLiteral("obj"))->storyId), &QTextDocument::contentsChanged);
        auto pagesJson = [&] { return QJsonDocument(QJsonObject{{"pages", d->toJson().value(QStringLiteral("pages"))}, {"stories", d->toJson().value(QStringLiteral("stories"))}}).toJson(QJsonDocument::Compact); };
        const QByteArray before = pagesJson();
        w.editMasters(layout);
        auto band = std::make_shared<ShapeItem>();
        band->shape = QStringLiteral("rect");
        band->rect = QRectF(0, 0, 36, 540);
        band->fill = Fill::solid(ColorRef::rgb(QColor(0x2B, 0x7A, 0x78)));
        band->stroke = Stroke();
        w.editor()->addItem(band, false);
        w.setPreserve(layout, false);
        w.setPreserve(layout, true);
        QCOMPARE(pagesJson(), before);
        QCOMPARE(spy.count(), 0);
        // A change to the place itself still reaches them.
        w.editor()->select(holderOn(*d->master(layout), QStringLiteral("obj"))->id);
        w.editor()->setFontSize(33);
        QCOMPARE(firstLetterOf(*d, *holderOn(*d->pages[1], QStringLiteral("obj"))).fontPointSize(), 33.0);
    }

    // A slide that goes to a layout of another slide master takes that slide master's theme (its fonts and colors), as a change of theme from the
    // Design tab gives it, so that deleting the first slide master afterwards changes nothing that shows.
    void aSlideMovedToAnotherMastersLayoutTakesItsTheme()
    {
        DeckWindow w;
        fillSlides(w, {"One", "Two"}, {{"sub"}, {"a", "b"}});
        Document *d = w.editor()->doc();
        const deck::DeckTheme first = deck::themeOf(*d);
        // A theme with other fonts, for the first slide only: it stands on a slide master of its own.
        int other = -1;
        for (int i = 0; i < deck::builtinThemeCount() && other < 0; ++i)
            if (deck::builtinThemeFonts(i).heading != first.headingFont && deck::builtinThemeFonts(i).body != first.bodyFont) other = i;
        QVERIFY(other >= 0);
        const deck::FontSet fonts = deck::builtinThemeFonts(other);
        w.editor()->setCurrentPage(0);
        w.applyTheme(other, false);
        const QString second = deck::slideMasterOf(*d, 0);
        QVERIFY(second != QLatin1String("SM1"));
        w.editor()->setCurrentPage(1);
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->pages[1], QStringLiteral("title"))), first.headingFont);
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->pages[1], QStringLiteral("obj"))), first.bodyFont);
        w.setLayout(QStringLiteral("id:") + deck::layoutOfType(*d, QStringLiteral("obj"), second)->id);
        QCOMPARE(deck::slideMasterOf(*d, 1), second);
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->pages[1], QStringLiteral("title"))), fonts.heading);
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->pages[1], QStringLiteral("obj"))), fonts.body);
        QCOMPARE(titleOfSlide(*d, 1), QStringLiteral("Two"));
        // One step; and deleting the first slide master now changes nothing that is drawn.
        w.editor()->undo();
        QCOMPARE(deck::slideMasterOf(*d, 1), QStringLiteral("SM1"));
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->pages[1], QStringLiteral("title"))), first.headingFont);
        w.editor()->redo();
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d;
        ctx.cache = &cache;
        const QImage a = Renderer::renderToImage(ctx, 0, 0.5), b = Renderer::renderToImage(ctx, 1, 0.5);
        QString why;
        QVERIFY2(w.deleteMasterPage(QStringLiteral("SM1"), &why), qPrintable(why));
        QCOMPARE(Renderer::renderToImage(ctx, 0, 0.5), a);
        QCOMPARE(Renderer::renderToImage(ctx, 1, 0.5), b);
    }

    // Slide Master view as it looks, saved as pictures in the folder JP_SHOT_DIR names (for looking at it): the master with its layouts, a layout
    // being edited, and a slide that shows the change.
    void slideMasterViewLooksRight()
    {
        setUiDark(false);
        DeckWindow w;
        // The window has settled (the ribbon has laid itself out) when the picture is taken.
        auto look = [&w](const QString &name) {
            if (qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) return;
            QTest::qWait(150);   // only when looking: the layout and the timers have run
            shot(&w, name);
        };
        w.resize(1440, 900);
        w.show();
        fillSlides(w, {"Quarterly results", "Revenue by region", "Next steps"}, {{"Prepared for the board"}, {"North: up 8%", "South: flat", "West: up 12%"}, {"Hire two analysts", "Launch in March"}});
        Document *d = w.editor()->doc();
        w.editor()->setCurrentPage(1);
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        // The tabs that have the commands that open it and Hide Background Graphics.
        w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("View")));
        look(QStringLiteral("master-view-view-tab"));
        w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Design")));
        look(QStringLiteral("master-view-design-tab"));
        w.editMasters(QStringLiteral("SM1"));
        QTRY_COMPARE(w.masterPane()->ids().size(), 10);
        look(QStringLiteral("master-view-master"));
        const QString layout = d->pages[1]->masterId;
        w.editMasters(layout);
        // A band down the left side, and a bold title in another font.
        auto band = std::make_shared<ShapeItem>();
        band->shape = QStringLiteral("rect");
        band->name = QStringLiteral("Band");
        band->rect = QRectF(0, 0, 36, 540);
        band->fill = Fill::solid(ColorRef::rgb(QColor(0x2B, 0x7A, 0x78)));
        band->stroke = Stroke();
        w.editor()->addItem(band, false);
        w.editor()->select(holderOn(*d->master(layout), QStringLiteral("title"))->id);
        w.editor()->setFontFamily(QStringLiteral("Georgia"));
        w.editor()->toggleBold();
        w.insertPlaceholder(deck::PlaceholderKind::Picture);
        w.renameMasterPage(layout, QStringLiteral("Title, Text, and Picture"));
        w.editor()->clearSelection();
        QTRY_VERIFY(w.masterPane()->thumbnailRect(layout).isValid());
        look(QStringLiteral("master-view-edited-layout"));
        w.closeMasters();
        QTRY_VERIFY(!w.slidesPane()->isHidden());
        w.slidesPane()->refresh();
        look(QStringLiteral("master-view-slide"));
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d;
        ctx.cache = &cache;
        // The presentation as it is saved, for looking at in the other programs.
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            const QString dir = qEnvironmentVariable("JP_SHOT_DIR");
            QFile pptx(dir + QStringLiteral("/master-view.pptx")), odp(dir + QStringLiteral("/master-view.odp"));
            if (pptx.open(QIODevice::WriteOnly)) pptx.write(deck::exportPresentation(*d));
            if (odp.open(QIODevice::WriteOnly)) odp.write(deck::exportOdp(*d));
        }
        QCOMPARE(Renderer::renderToImage(ctx, 1, 1).pixelColor(10, 270), QColor(0x2B, 0x7A, 0x78));
        QCOMPARE(familyOfHolder(*d, *holderOn(*d->pages[1], QStringLiteral("title"))), QStringLiteral("Georgia"));
        QVERIFY(firstLetterOf(*d, *holderOn(*d->pages[1], QStringLiteral("title"))).fontWeight() >= QFont::DemiBold);
    }
};

JP_TEST_CLASS(DeckMasterTests)
#include "tst_deck_master.moc"
