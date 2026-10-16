// JeffDeck's tests of the window: ribbon, help, status bar, headers and footers, backgrounds, the slide menu, and find.
#include "deckhelpers.h"

class DeckWindowTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

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
    // JeffDeck's ribbon labels are listed for translators (tools/ribbon_strings.py), as JeffPub's are.
    void ribbonStringsListed()
    {
        QFile json(QStringLiteral(":/deck/ribbon.json"));
        QVERIFY(json.open(QIODevice::ReadOnly));
        QFile listed(QStringLiteral(JD_SOURCE_DIR "/apps/deck/app/ribbon_strings.cpp"));
        QVERIFY(listed.open(QIODevice::ReadOnly | QIODevice::Text));
        const QString have = QString::fromUtf8(listed.readAll());
        QStringList missing;
        const QStringList keys = {"name", "text", "launcherTip", "tip", "label", "section", "submenu", "context"};
        std::function<void(const QJsonValue &)> walk = [&](const QJsonValue &v) {
            if (v.isObject()) {
                const QJsonObject o = v.toObject();
                for (auto it = o.begin(); it != o.end(); ++it) {
                    if (keys.contains(it.key()) && it.value().isString() && !it.value().toString().isEmpty()) {
                        const QString lit = QStringLiteral("QT_TRANSLATE_NOOP(\"Ribbon\", \"%1\")").arg(it.value().toString().replace(QLatin1Char('"'), QStringLiteral("\\\"")));
                        if (!have.contains(lit)) missing << it.value().toString();
                    }
                    walk(it.value());
                }
            } else if (v.isArray()) {
                for (const QJsonValue &x : v.toArray()) walk(x);
            }
        };
        walk(QJsonDocument::fromJson(json.readAll()).object());
        QVERIFY2(missing.isEmpty(), qPrintable(QStringLiteral("not in ribbon_strings.cpp (run tools/ribbon_strings.py): ") + missing.join(QStringLiteral(", "))));
    }

    // JeffDeck has the suite's translations built in, and speaks German when German is chosen.
    void translationsAreBuiltIn()
    {
        const QStringList have = availableUiLanguages();
        for (const char *lang : {"de", "es", "fr", "it", "nl", "pt_BR", "pt_PT"}) QVERIFY2(have.contains(QLatin1String(lang)), qPrintable(have.join(QLatin1Char(' '))));
        QTranslator de;
        QVERIFY(de.load(QStringLiteral(":/i18n/jeffoffice_de.qm")));
        // A string of the shared libraries, translated.
        QVERIFY(!de.translate("Ribbon", "File").isEmpty());
    }

    // JeffDeck's Help lists its own topics (resources/help/en/deck) and none of
    // JeffPub's: a topic only JeffPub has isn't found, and one both have (the
    // contents, the keyboard) is JeffDeck's.
    void helpListsDeckTopicsOnly()
    {
        QCOMPARE(appInfo().helpFolder, QStringLiteral("deck"));
        const QStringList ids = help::topicIds();
        QVERIFY2(ids.size() >= 20, qPrintable(ids.join(QLatin1Char(' '))));
        for (const char *id : {"index", "start", "window", "slides", "show", "printing", "keyboard", "tab-home", "tab-slide-show"})
            QVERIFY2(ids.contains(QLatin1String(id)), id);
        QCOMPARE(help::topic(QStringLiteral("index")).title, QStringLiteral("JeffDeck Help"));
        for (const QString &file : QDir(QStringLiteral(":/help/en")).entryList({QStringLiteral("*.md")}, QDir::Files)) {
            const QString id = file.chopped(3);
            QFile pub(QStringLiteral(":/help/en/") + file);
            QVERIFY(pub.open(QIODevice::ReadOnly));
            if (ids.contains(id)) QVERIFY2(help::topic(id).markdown.toUtf8() != pub.readAll(), qPrintable(id));
            else QVERIFY2(help::topic(id).isNull(), qPrintable(id));
        }
        // The search finds them.
        QVERIFY(help::searchTopics(QStringLiteral("how do I add a transition")).contains(QStringLiteral("transitions")));
        QVERIFY(help::searchTopics(QStringLiteral("zzqxv")).isEmpty());
    }

    // Every topic has a title and search words, every link and F1 context leads
    // to a topic, the contents lists them all, the keyboard topic's table comes
    // from JeffDeck's commands, and none names another company's products.
    void helpTopicsAreComplete()
    {
        const QStringList ids = help::topicIds();
        static const QRegularExpression link(QStringLiteral("\\]\\(([^)]+)\\)"));
        static const QRegularExpression banned(QStringLiteral("\\b(Microsoft|Publisher|Office|PowerPoint|Excel|WordArt)\\b|(?<!Each )\\bWord\\b(?! Count)"));
        const help::Topic contents = help::topic(QStringLiteral("index"));
        QStringList problems;
        for (const QString &id : ids) {
            const help::Topic t = help::topic(id);
            if (t.title.isEmpty()) problems << id + QStringLiteral(": no title");
            if (t.keywords.isEmpty()) problems << id + QStringLiteral(": no keywords");
            if (const auto m = banned.match(t.markdown); m.hasMatch()) problems << id + QStringLiteral(": says ") + m.captured(0);
            for (auto m = link.globalMatch(t.markdown); m.hasNext();) {
                const QString target = m.next().captured(1);
                if (!target.startsWith(QLatin1String("https://")) && !ids.contains(target)) problems << id + QStringLiteral(": link to ") + target;
            }
            if (id != QLatin1String("index") && !contents.markdown.contains(QStringLiteral("](%1)").arg(id))) problems << id + QStringLiteral(": not in the contents");
            QTextDocument doc;
            doc.setMarkdown(help::displayMarkdown(t, nullptr), QTextDocument::MarkdownDialectGitHub);
            if (doc.toPlainText().trimmed().size() < 100) problems << id + QStringLiteral(": nearly empty");
        }
        QFile cf(QStringLiteral(":/help/en/deck/context.json"));
        QVERIFY(cf.open(QIODevice::ReadOnly));
        const QJsonObject context = QJsonDocument::fromJson(cf.readAll()).object();
        QVERIFY(!context.isEmpty());
        for (auto it = context.begin(); it != context.end(); ++it)
            if (!ids.contains(it.value().toString())) problems << QStringLiteral("context %1: no topic %2").arg(it.key(), it.value().toString());
        DeckWindow w;
        for (int i = 0; i < w.ribbon()->tabCount(); ++i)
            if (help::contextTopic(QStringLiteral("tab:") + w.ribbon()->tabName(i)).isEmpty()) problems << QStringLiteral("no topic for the tab ") + w.ribbon()->tabName(i);
        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QStringLiteral("; "))));
        const QString keys = help::displayMarkdown(help::topic(QStringLiteral("keyboard")), &w);
        QVERIFY(!keys.contains(QLatin1String("<!--")));
        QVERIFY2(keys.contains(QStringLiteral("| New Slide | %1 |").arg(QKeySequence(Qt::CTRL | Qt::Key_M).toString(QKeySequence::NativeText))), qPrintable(keys.right(3000)));
        QVERIFY(keys.contains(QStringLiteral("| From Beginning | %1 |").arg(QKeySequence(Qt::Key_F5).toString(QKeySequence::NativeText))));
        QVERIFY(keys.contains(QStringLiteral("| Help | %1 |").arg(QKeySequence(QKeySequence::HelpContents).toString(QKeySequence::NativeText))));
    }

    // F1 and the "?" beside the ribbon's collapse arrow open JeffDeck's Help: the
    // contents from the slide, a tab's topic from the ribbon, and a window of its
    // own over the File screen.
    void helpOpensFromF1AndTheHelpButton()
    {
        DeckWindow w;
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        QVERIFY(!w.helpView());
        w.canvas()->setFocus();
        QTest::keyClick(w.canvas(), Qt::Key_F1);
        HelpView *v = w.helpView();
        QVERIFY(v && v->isVisible());
        QCOMPARE(v->currentTopic(), QStringLiteral("index"));
        QVERIFY(v->browser()->toPlainText().contains(QLatin1String("JeffDeck")));
        QVERIFY(!v->browser()->toPlainText().contains(QLatin1String("publication")));
        // From the ribbon: the tab's topic.
        Ribbon *r = w.ribbon();
        r->showTab(r->tab(QStringLiteral("Transitions")));
        r->focusCurrentTab();
        QTest::keyClick(QApplication::focusWidget(), Qt::Key_F1);
        QCOMPARE(v->currentTopic(), QStringLiteral("tab-transitions"));
        // The "?" opens it again once it is put away.
        auto *dock = w.findChild<QDockWidget *>(QStringLiteral("helpDock"));
        QVERIFY(dock && dock->isVisible());
        dock->hide();
        auto *qmark = r->findChild<QAbstractButton *>(QStringLiteral("jpRibbonHelp"));
        QVERIFY(qmark && qmark->isVisible());
        qmark->click();
        QVERIFY(dock->isVisible());
        QCOMPARE(w.helpView(), v);
        // Over the File screen, which covers the pane: a window, on that page's topic.
        w.showFileScreen(QStringLiteral("print"));
        QTest::keyClick(QApplication::focusWidget(), Qt::Key_F1);
        auto *hw = w.findChild<HelpWindow *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(hw && hw->isVisible());
        QCOMPARE(hw->view()->currentTopic(), QStringLiteral("printing"));
        hw->close();
        QTRY_VERIFY(!w.findChild<HelpWindow *>());   // closing deletes it
    }

    // A translated topic of JeffDeck's Help is one of the English ones, with a
    // title and search words, the same links as the English, and no other
    // company's product names.
    void translatedDeckHelpIsComplete()
    {
        const QDir help(QStringLiteral(JD_SOURCE_DIR "/resources/help"));
        static const QRegularExpression link(QStringLiteral("\\]\\(([^)\\s]+)\\)"));
        static const QRegularExpression banned(QStringLiteral("\\b(Microsoft|Publisher|Office|PowerPoint|Excel|WordArt)\\b|(?<!Each )\\bWord\\b(?! Count)"));
        auto read = [](const QString &path) {
            QFile f(path);
            return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()) : QString();
        };
        auto links = [](const QString &md) {
            QStringList out;
            for (auto m = link.globalMatch(md); m.hasNext();) out << m.next().captured(1);
            out.sort();
            return out;
        };
        QStringList problems;
        for (const QString &lang : help.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            if (lang == QLatin1String("en")) continue;
            const QDir folder(help.filePath(lang + QStringLiteral("/deck")));
            for (const QString &file : folder.entryList({QStringLiteral("*.md")}, QDir::Files)) {
                const QString en = read(help.filePath(QStringLiteral("en/deck/") + file)), md = read(folder.filePath(file));
                const QString where = lang + QStringLiteral("/deck/") + file;
                if (en.isEmpty()) {
                    problems << where + QStringLiteral(": no English topic");
                    continue;
                }
                if (!md.startsWith(QStringLiteral("# "))) problems << where + QStringLiteral(": no title");
                if (en.contains(QStringLiteral("<!-- keywords:")) && !md.contains(QStringLiteral("<!-- keywords:"))) problems << where + QStringLiteral(": no keywords");
                if (links(md) != links(en)) problems << where + QStringLiteral(": links differ from the English");
                if (const auto m = banned.match(md); m.hasMatch()) problems << where + QStringLiteral(": says ") + m.captured(0);
            }
        }
        QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QStringLiteral("; "))));
    }

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
    // A slide's text box shows text that doesn't fit, running out of it as
    // the presentation program draws it: down from text at the top, both ways
    // from centered text, up from text at the bottom. A publication's box
    // keeps it for a next box instead. Text that fits is set as before.
    void slideTextRunsOutOfItsBox()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat cf;
        cf.setFontPointSize(20);
        c.insertText(QStringLiteral("One"), cf);
        c.insertBlock();
        c.insertText(QStringLiteral("Two"), cf);
        c.insertBlock();
        c.insertText(QStringLiteral("Three"), cf);
        FrameSpec fs;
        fs.size = QSizeF(200, 30);
        fs.insets = QMarginsF(0, 0, 0, 0);
        StoryLayout held;
        held.build(&doc, {fs}, LayoutEnv());
        QVERIFY(held.overflow());
        QVERIFY(held.lineInfo(0).size() < 3);
        fs.runsOver = true;
        for (const VAlign v : {VAlign::Top, VAlign::Middle, VAlign::Bottom}) {
            fs.valign = v;
            StoryLayout lay;
            lay.build(&doc, {fs}, LayoutEnv());
            QVERIFY(!lay.overflow());
            const auto lines = lay.lineInfo(0);
            QCOMPARE(int(lines.size()), 3);
            const double top = lines.first().rect.top(), bottom = lines.last().rect.bottom() - lines.last().below;
            QVERIFY(bottom - top > 30);
            if (v == VAlign::Top) QVERIFY2(std::abs(top) < 0.5, qPrintable(QString::number(top)));
            if (v == VAlign::Middle) QVERIFY2(std::abs((top + bottom) / 2 - 15) < 0.5, qPrintable(QString::number((top + bottom) / 2)));
            if (v == VAlign::Bottom) QVERIFY2(std::abs(bottom - 30) < 0.5, qPrintable(QString::number(bottom)));
        }
        fs.size = QSizeF(200, 300);
        fs.valign = VAlign::Middle;
        FrameSpec plain = fs;
        plain.runsOver = false;
        StoryLayout a, b;
        a.build(&doc, {plain}, LayoutEnv());
        b.build(&doc, {fs}, LayoutEnv());
        QCOMPARE(int(a.lineInfo(0).size()), 3);
        for (int i = 0; i < 3; ++i) QCOMPARE(b.lineInfo(0)[i].rect, a.lineInfo(0)[i].rect);
        // Presentations' boxes run over, publications' don't.
        QVERIFY(deck::newPresentation()->slideText);
        QVERIFY(!Document().slideText);
        QString err;
        auto opened = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(opened && opened->slideText, qPrintable(err));
    }

    // A slide's empty placeholder shows the fill its layout gives it (a design's sample picture), whose picture is named by the
    // layout's relationships, not the slide's.
    void emptyPlaceholderShowsItsLayoutsPictureFill()
    {
        Parts p = samplePptx();
        QImage green(4, 4, QImage::Format_RGB32);
        green.fill(QColor(0x10, 0xC0, 0x30));
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        green.save(&buf, "PNG");
        p.data["/ppt/media/image2.png"] = {QStringLiteral("image/png"), png};
        // The layout's rId2 is the green picture; the slide's rId2 is the orange one.
        p.rels << Rel{"/ppt/slideLayouts/slideLayout2.xml", "image", "/ppt/media/image2.png"};
        QString layout = QString::fromUtf8(p.data["/ppt/slideLayouts/slideLayout2.xml"].xml);
        layout.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"9\" name=\"Picture Placeholder 8\"/><p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr><p:nvPr><p:ph type=\"pic\" idx=\"13\"/></p:nvPr></p:nvSpPr><p:spPr>")
                                                          + xfrm(6400800, 1825625, 4572000, 3429000)
                                                          + QStringLiteral("<a:blipFill><a:blip r:embed=\"rId2\"/><a:stretch><a:fillRect/></a:stretch></a:blipFill></p:spPr></p:sp></p:spTree>"));
        p.data["/ppt/slideLayouts/slideLayout2.xml"].xml = layout.toUtf8();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        xml.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"31\" name=\"Picture Placeholder 30\"/><p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr><p:nvPr><p:ph type=\"pic\" idx=\"13\"/></p:nvPr></p:nvSpPr><p:spPr/></p:sp></p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const PictureItem *slot = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->name == QLatin1String("Picture Placeholder 30")) slot = static_cast<const PictureItem *>(it.get());
        QVERIFY(slot && slot->imageId.isEmpty());
        QCOMPARE(int(slot->fill.type), int(Fill::Picture));
        const QImage shown = d->image(slot->fill.imageId);
        QVERIFY(!shown.isNull());
        QCOMPARE(QColor(shown.pixel(1, 1)), QColor(0x10, 0xC0, 0x30));
    }

    // A shape filled with the slide's background (p:sp useBgFill) has the background's fill: the slide's own, else the one its
    // layout or master shows.
    void shapesFilledWithTheBackground()
    {
        const QString panel = QStringLiteral("<p:sp useBgFill=\"1\"><p:nvSpPr><p:cNvPr id=\"40\" name=\"Panel\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr>")
                              + xfrm(0, 0, 6096000, 6858000)
                              + QStringLiteral("<a:ln><a:noFill/></a:ln></p:spPr><p:style><a:lnRef idx=\"2\"><a:schemeClr val=\"accent1\"/></a:lnRef><a:fillRef idx=\"1\"><a:schemeClr val=\"accent1\"/></a:fillRef>"
                                               "<a:effectRef idx=\"0\"><a:schemeClr val=\"accent1\"/></a:effectRef><a:fontRef idx=\"minor\"><a:schemeClr val=\"lt1\"/></a:fontRef></p:style></p:sp>");
        auto panelOf = [](const Document &d, int slide) -> const Item * {
            for (const auto &it : d.pages[slide]->items)
                if (it->name == QLatin1String("Panel")) return it.get();
            return nullptr;
        };
        Parts p = samplePptx();
        // Slide 2 has a background of its own (112233); slide 1 shows its master's.
        for (const char *slide : {"/ppt/slides/slide1.xml", "/ppt/slides/slide2.xml"}) {
            QString xml = QString::fromUtf8(p.data[QString::fromLatin1(slide)].xml);
            xml.replace(QStringLiteral("</p:spTree>"), panel + QStringLiteral("</p:spTree>"));
            p.data[QString::fromLatin1(slide)].xml = xml.toUtf8();
        }
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const Item *own = panelOf(*d, 1), *inherited = panelOf(*d, 0);
        QVERIFY(own && inherited);
        QCOMPARE(int(own->fill.type), int(Fill::Solid));
        QCOMPARE(own->fill.color.resolve(d->colors), QColor(0x11, 0x22, 0x33));
        const MasterPage *layout = d->masterFor(*d->pages[0]);
        Fill shown = layout ? layout->background : Fill();
        for (const MasterPage *m : d->masterChain(layout))
            if (!m->background.isNone()) shown = m->background;
        if (layout && !layout->background.isNone()) shown = layout->background;
        QCOMPARE(int(inherited->fill.type), int(shown.type));
        QVERIFY(!(inherited->fill.type == Fill::Solid && inherited->fill.color.resolve(d->colors) == d->colors.c[Accent1]));
    }

    // Text art (an old file's) is saved as the presentation program keeps the older kind: text warped into its box and stretched
    // to fill it, with the art's letters' look; read again, it is the same art.
    void textArtIsSavedAndReadBack()
    {
        auto d = deck::newPresentation();
        auto art = std::make_shared<TextArtItem>();
        art->name = QStringLiteral("Art");
        art->rect = QRectF(100, 100, 400, 150);
        art->text = QStringLiteral("Hello\nWorld");
        art->font = QStringLiteral("Arial");
        art->bold = true;
        art->size = 40;
        art->transform_ = QStringLiteral("archUp");
        art->align = 0;
        art->fill = Fill::solid(ColorRef::rgb(QColor(0xCC, 0x11, 0x22)));
        art->stroke = Stroke::line(ColorRef::rgb(QColor(0x11, 0x22, 0xCC)), 1.5);
        d->pages[0]->items.push_back(art);
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.filter(QStringLiteral("text art")).isEmpty(), qPrintable(skipped.join('\n')));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(slide.contains(QStringLiteral("<a:prstTxWarp prst=\"textArchUp\">")));
        QVERIFY(slide.contains(QStringLiteral("fromWordArt=\"1\"")));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const TextArtItem *read = nullptr;
        for (const auto &it : back->pages[0]->items)
            if (it->name == QLatin1String("Art") && it->type() == ItemType::TextArt) read = static_cast<const TextArtItem *>(it.get());
        QVERIFY(read);
        QCOMPARE(read->text, art->text);
        QCOMPARE(read->transform_, QStringLiteral("archUp"));
        QCOMPARE(read->font, QStringLiteral("Arial"));
        QVERIFY(read->bold);
        QCOMPARE(read->size, 40.0);
        QCOMPARE(read->align, 0);
        QCOMPARE(read->fill.color.resolve(back->colors), QColor(0xCC, 0x11, 0x22));
        QCOMPARE(read->stroke.color.resolve(back->colors), QColor(0x11, 0x22, 0xCC));
        QVERIFY(std::abs(read->rect.width() - 400) < 0.5 && std::abs(read->rect.height() - 150) < 0.5);
    }

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

    // Ctrl+F1 collapses the ribbon and brings it back, as its button says.
    void ctrlF1CollapsesTheRibbon()
    {
        DeckWindow w;
        QAction *a = w.act(QStringLiteral("view.collapseRibbon"));
        QVERIFY(a);
        QCOMPARE(a->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_F1));
        const bool was = w.ribbon()->isMinimized();
        a->trigger();
        QCOMPARE(w.ribbon()->isMinimized(), !was);
        a->trigger();
        QCOMPARE(w.ribbon()->isMinimized(), was);
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

    // The dialog gives back what it doesn't show as it was: a fixed date's text, the date's field, and that the date or the number is the text typed in
    // the master, so that Apply to All with only the footer's text changed leaves them.
    void headerFooterDialogKeepsWhatItDoesNotShow()
    {
        deck::FooterOptions o;
        o.date = true;
        o.slideNumber = true;
        o.footer = true;
        o.footerText = QStringLiteral("Quarterly");
        o.dateText = QStringLiteral("May 19, 2003");
        o.dateCode = QStringLiteral("datetime:M/d/yyyy");
        o.dateFromMaster = true;
        o.numberFromMaster = true;
        HeaderFooterDialog dlg(o);
        dlg.findChild<QLineEdit *>(QStringLiteral("footerText"))->setText(QStringLiteral("Q3 review"));
        const deck::FooterOptions got = dlg.options();
        QCOMPARE(got.footerText, QStringLiteral("Q3 review"));
        QVERIFY(got.date && got.slideNumber && got.footer);
        QCOMPARE(got.dateText, o.dateText);
        QCOMPARE(got.dateCode, o.dateCode);
        QVERIFY(got.dateFromMaster && got.numberFromMaster);
        // What it does show it changes: the date turned off.
        dlg.findChild<QCheckBox *>(QStringLiteral("date"))->setChecked(false);
        QVERIFY(!dlg.options().date);
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

private:
    // What a menu offers, by id: each command by its action's id (a spelling suggestion by its words), each submenu as [its title],
    // and what is in it with the title and a colon before.
    static QStringList menuIds(const QMenu *menu, const QString &prefix = QString())
    {
        QStringList out;
        for (const QAction *a : menu->actions()) {
            if (a->isSeparator()) continue;
            if (a->menu()) {
                out << prefix + QStringLiteral("[%1]").arg(a->text());
                out << menuIds(a->menu(), a->text() + QLatin1Char(':'));
            } else {
                out << prefix + (a->objectName().isEmpty() ? a->text() : a->objectName());
            }
        }
        return out;
    }
    // The ids of the slide's menu; with JP_SHOT_DIR set it is also opened and a picture of it saved under `shotName`.
    static QStringList slideMenu(DeckWindow &w, const QString &shotName = QString())
    {
        std::unique_ptr<QMenu> menu(w.slideContextMenu());
        if (!shotName.isEmpty() && !qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            menu->popup(w.canvas()->mapToGlobal(QPoint(380, 260)));
            QApplication::processEvents();
            shot(menu.get(), shotName);
            menu->close();
        }
        return menuIds(menu.get());
    }
    static TextItem *placeholderOfSlide(const Document &d, int i, int n) { return static_cast<TextItem *>(d.pages[i]->items[n].get()); }
    // A deck with its words in the places Find has to look: slide 1 (a title slide) has "Year review" and a subtitle;
    // slide 2 a body with "review" twice and notes with it once; slide 3 "Reviews and previews", a shape saying "Peer review", and a table
    // whose first cell says "Review".
    static void fillFindDeck(DeckWindow &w)
    {
        Editor *ed = w.editor();
        fillSlides(w, {QStringLiteral("Year review"), QStringLiteral("Profit"), QStringLiteral("Plans")},
                   {{QStringLiteral("Sub")}, {QStringLiteral("We review profit and Review again")}, {QStringLiteral("Reviews and previews")}});
        Document *d = ed->doc();
        QTextCursor(d->storyDoc(d->pages[1]->notesStoryId)).insertText(QStringLiteral("Do not review the numbers"));
        auto shape = std::make_shared<ShapeItem>();
        shape->shape = QStringLiteral("rect");
        shape->rect = QRectF(100, 380, 200, 80);
        shape->storyId = d->createStory(QStringLiteral("Peer review"));
        d->pages[2]->items.push_back(shape);
        ed->setCurrentPage(2);
        w.insertTable(1, 2);
        auto *table = static_cast<TableItem *>(d->pages[2]->items.back().get());
        QTextCursor(d->storyDoc(table->cells[0].storyId)).insertText(QStringLiteral("Review"));
        QTextCursor(d->storyDoc(table->cells[1].storyId)).insertText(QStringLiteral("none"));
        ed->clearSelection();
        ed->setCurrentPage(0);
        ed->notifyLive();
        ed->undoStack()->clear();
    }
    // Where Find Next has put the match: "slide:text@start" in a box, or the same "in the notes".
    static QString matchShown(DeckWindow &w)
    {
        Editor *ed = w.editor();
        if (ed->isEditingText())
            return QStringLiteral("%1:%2@%3").arg(ed->currentPage() + 1).arg(ed->cursor().selectedText()).arg(ed->cursor().selectionStart());
        const QTextCursor c = w.notesPane()->textCursor();
        return QStringLiteral("%1:%2@%3 in the notes").arg(ed->currentPage() + 1).arg(c.selectedText()).arg(c.selectionStart());
    }
    static QStringList findAll(DeckWindow &w, int steps)
    {
        QStringList seen;
        for (int i = 0; i < steps; ++i) {
            w.findPane()->findNext();
            seen << matchShown(w);
        }
        return seen;
    }
    static QString storyText(DeckWindow &w, const QString &storyId) { return w.editor()->doc()->storyDoc(storyId)->toPlainText(); }

private Q_SLOTS:
    // Right-click menu of the slide: nothing selected offers the slide's commands, an object offers its own, text being typed in
    // offers the text's.
    void slideMenuOffersWhatTheSelectionCanDo()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        Editor *ed = w.editor();
        QStringList ids = slideMenu(w, QStringLiteral("menu-empty-slide"));
        for (const char *id : {"edit.paste", "slide.reset", "design.formatBackground", "view.rulers", "comment.new", "[Layout]", "[Grid and Guides]",
                               "Grid and Guides:view.gridlines", "Grid and Guides:view.guides"})
            QVERIFY2(ids.contains(QLatin1String(id)), qPrintable(QLatin1String(id) + QLatin1String(" in ") + ids.join(QLatin1Char(' '))));
        for (const char *id : {"edit.cut", "edit.copy", "edit.text", "arr.group", "insert.link", "fmt.fontDialog"}) QVERIFY2(!ids.contains(QLatin1String(id)), id);

        // A placeholder: clipboard, delete, edit text, order, align text, link, comment; no slide commands.
        const QString titleId = placeholderOfSlide(*ed->doc(), 0, 0)->id;
        ed->select(titleId);
        ids = slideMenu(w, QStringLiteral("menu-object"));
        for (const char *id : {"edit.cut", "edit.copy", "edit.paste", "edit.delete", "edit.text", "[Order]", "Order:arr.front", "Order:arr.forward", "Order:arr.backward",
                               "Order:arr.back", "[Alignment]", "Alignment:fmt.alignLeft", "Alignment:fmt.alignCenter", "Alignment:fmt.alignRight", "Alignment:fmt.justify",
                               "insert.link", "comment.new"})
            QVERIFY2(ids.contains(QLatin1String(id)), qPrintable(QLatin1String(id) + QLatin1String(" in ") + ids.join(QLatin1Char(' '))));
        for (const char *id : {"arr.group", "arr.ungroup", "[Layout]", "slide.reset", "design.formatBackground"}) QVERIFY2(!ids.contains(QLatin1String(id)), id);

        // Two objects can be grouped; the group can be taken apart.
        ed->select(QStringList{titleId, placeholderOfSlide(*ed->doc(), 0, 1)->id});
        ids = slideMenu(w);
        QVERIFY(ids.contains(QStringLiteral("arr.group")) && !ids.contains(QStringLiteral("arr.ungroup")));
        w.act(QStringLiteral("arr.group"))->trigger();
        QCOMPARE(ed->selectionKind(), QStringLiteral("group"));
        ids = slideMenu(w);
        QVERIFY(ids.contains(QStringLiteral("arr.ungroup")) && !ids.contains(QStringLiteral("arr.group")));
        ed->undo();

        // A picture has no words to edit or align.
        QTemporaryDir dir;
        QImage img(40, 30, QImage::Format_ARGB32);
        img.fill(Qt::red);
        QVERIFY(img.save(dir.filePath(QStringLiteral("red.png"))));
        QVERIFY(w.insertPicture(dir.filePath(QStringLiteral("red.png"))));
        QCOMPARE(ed->selectionKind(), QStringLiteral("picture"));
        ids = slideMenu(w);
        QVERIFY(ids.contains(QStringLiteral("edit.cut")) && ids.contains(QStringLiteral("[Order]")) && ids.contains(QStringLiteral("insert.link")));
        QVERIFY(!ids.contains(QStringLiteral("edit.text")) && !ids.contains(QStringLiteral("[Alignment]")));

        // A table can be typed in.
        w.insertTable(2, 2);
        QCOMPARE(ed->selectionKind(), QStringLiteral("table"));
        QVERIFY(slideMenu(w).contains(QStringLiteral("edit.text")));

        // Text being typed in: the text's commands, not the object's.
        ed->beginTextEdit(titleId);
        ids = slideMenu(w, QStringLiteral("menu-text"));
        for (const char *id : {"edit.cut", "edit.copy", "edit.paste", "fmt.fontDialog", "para.dialog", "para.bullets", "para.numbers", "insert.link", "review.thesaurus"})
            QVERIFY2(ids.contains(QLatin1String(id)), qPrintable(QLatin1String(id) + QLatin1String(" in ") + ids.join(QLatin1Char(' '))));
        for (const char *id : {"edit.delete", "edit.text", "[Order]", "[Layout]"}) QVERIFY2(!ids.contains(QLatin1String(id)), id);
    }

    // Every command in the menu is the very action the ribbon has (so it is enabled and checked as the ribbon's is, and counts the
    // same); Reset Slide is the ribbon's Reset under a clearer name, and calls it.
    void slideMenuCommandsAreTheRibbonsActions()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        std::function<void(const QMenu *)> check = [&](const QMenu *menu) {
            for (QAction *a : menu->actions()) {
                if (a->menu()) {
                    check(a->menu());
                } else if (!a->isSeparator() && !a->objectName().isEmpty() && a->objectName() != QLatin1String("slide.reset")) {
                    QVERIFY2(w.act(a->objectName()), qPrintable(a->objectName()));
                    QCOMPARE(a, w.act(a->objectName()));
                }
            }
        };
        auto checkMenu = [&] {
            std::unique_ptr<QMenu> menu(w.slideContextMenu());
            QVERIFY(menu->actions().size() > 3);
            check(menu.get());
        };
        checkMenu();
        ed->select(placeholderOfSlide(*ed->doc(), 0, 0)->id);
        checkMenu();
        ed->beginTextEdit(placeholderOfSlide(*ed->doc(), 0, 0)->id);
        checkMenu();
        // Reset Slide puts the placeholders back where the layout has them.
        ed->endTextEdit();
        ed->clearSelection();
        const QRectF home = placeholderOfSlide(*ed->doc(), 0, 0)->rect;
        ed->change(QStringLiteral("Move"), [&] { placeholderOfSlide(*ed->doc(), 0, 0)->rect.translate(40, 30); });
        QVERIFY(placeholderOfSlide(*ed->doc(), 0, 0)->rect != home);
        std::unique_ptr<QMenu> menu(w.slideContextMenu());
        QAction *reset = nullptr;
        for (QAction *a : menu->actions())
            if (a->text() == QLatin1String("Reset Slide")) reset = a;
        QVERIFY(reset);
        QCOMPARE(reset->objectName(), QStringLiteral("slide.reset"));
        reset->trigger();
        QCOMPARE(placeholderOfSlide(*ed->doc(), 0, 0)->rect, home);
        // The menu's Edit Text is the action that types in the selected box.
        ed->select(placeholderOfSlide(*ed->doc(), 0, 1)->id);
        std::unique_ptr<QMenu> objectMenu(w.slideContextMenu());
        QVERIFY(objectMenu->actions().contains(w.act(QStringLiteral("edit.text"))));
        QVERIFY(!ed->isEditingText());
        w.act(QStringLiteral("edit.text"))->trigger();
        QVERIFY(ed->isEditingText());
        QCOMPARE(ed->textTarget().itemId, placeholderOfSlide(*ed->doc(), 0, 1)->id);
    }

    // A right click on the slide opens the menu for what is there; a click in the layout gallery of its Layout submenu changes the layout and closes
    // every menu.
    void rightClickingTheSlideOpensItsMenu()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        QStringList ids;
        rightClick(w.canvas()->viewport(), QPoint(300, 200), [&](QMenu *m) { ids = menuIds(m); });
        QVERIFY2(ids.contains(QStringLiteral("slide.reset")), qPrintable(ids.join(QLatin1Char(' '))));
        ed->select(placeholderOfSlide(*ed->doc(), 0, 0)->id);
        ids.clear();
        rightClick(w.canvas()->viewport(), QPoint(300, 200), [&](QMenu *m) { ids = menuIds(m); });
        QVERIFY2(ids.contains(QStringLiteral("edit.cut")), qPrintable(ids.join(QLatin1Char(' '))));
        // The same menu is not opened over itself.
        ed->clearSelection();
        bool again = false;
        QTimer::singleShot(0, [&] {
            untilDone([&] {
                auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget());
                if (!m) return false;
                w.showSlideContextMenu(QPoint(10, 10));   // returns at once
                again = true;
                m->close();
                return true;
            });
        });
        w.showSlideContextMenu(w.canvas()->mapToGlobal(QPoint(300, 200)));
        QVERIFY(again);
        // Layout: choosing one changes the slide and closes the menu.
        QCOMPARE(deck::layoutType(*ed->doc()->masterFor(*ed->doc()->pages[0])), QStringLiteral("title"));
        bool chose = false;
        QTimer::singleShot(0, [&] {
            untilDone([&] {
                auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget());
                if (!m) return false;
                for (QAction *a : m->actions())
                    if (a->menu() && a->text() == QLatin1String("Layout"))
                        if (auto *wa = qobject_cast<QWidgetAction *>(a->menu()->actions().value(0)))
                            if (auto *gallery = qobject_cast<LayoutGallery *>(wa->defaultWidget())) {
                                gallery->reload();
                                Q_EMIT gallery->chosen(QStringLiteral("titleOnly"));
                                chose = true;
                            }
                if (!chose) m->close();
                return true;
            });
        });
        w.showSlideContextMenu(w.canvas()->mapToGlobal(QPoint(300, 200)));   // comes back only once every menu is closed
        QVERIFY(chose);
        QCOMPARE(deck::layoutType(*ed->doc()->masterFor(*ed->doc()->pages[0])), QStringLiteral("titleOnly"));
    }

    // Shift+F10 and the Menu key ask for the menu where the selection is: under the text cursor, else in the middle of the selected object,
    // else in the middle of the view.
    void shiftF10AndTheMenuKeyOpenTheMenuAtTheSelection()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        Canvas *cv = w.canvas();
        cv->setFocus();
        QObject::disconnect(cv, &Canvas::contextMenuWanted, &w, nullptr);   // only to see where the menu would open
        QSignalSpy asked(cv, &Canvas::contextMenuWanted);
        auto where = [&](Qt::Key key, Qt::KeyboardModifiers mods) {
            asked.clear();
            QTest::keyClick(cv, key, mods);
            return asked.count() == 1 ? asked.takeFirst().at(0).toPoint() : QPoint(-9999, -9999);
        };
        TextItem *title = placeholderOfSlide(*ed->doc(), 0, 0);
        ed->select(title->id);
        for (const auto &[key, mods] : {std::pair{Qt::Key_F10, Qt::KeyboardModifiers(Qt::ShiftModifier)}, std::pair{Qt::Key_Menu, Qt::KeyboardModifiers()}}) {
            const QPoint at = where(key, mods);
            QVERIFY2(cv->screenRect(title->bounds()).contains(at), qPrintable(QStringLiteral("%1,%2").arg(at.x()).arg(at.y())));
        }
        // The platform's own request for the menu from the keyboard comes to the same place.
        asked.clear();
        QContextMenuEvent keyboard(QContextMenuEvent::Keyboard, QPoint(1, 1), cv->mapToGlobal(QPoint(1, 1)));
        QApplication::sendEvent(cv->viewport(), &keyboard);
        QCOMPARE(asked.count(), 1);
        QVERIFY(cv->screenRect(title->bounds()).contains(asked.first().at(0).toPoint()));
        // A mouse request stays where the mouse is.
        asked.clear();
        QContextMenuEvent mouse(QContextMenuEvent::Mouse, QPoint(7, 8), cv->viewport()->mapToGlobal(QPoint(7, 8)));
        QApplication::sendEvent(cv->viewport(), &mouse);
        QCOMPARE(asked.first().at(0).toPoint(), cv->viewport()->mapToGlobal(QPoint(7, 8)));
        // Typing in the box: just under the caret.
        ed->beginTextEdit(title->id);
        const QPoint under = where(Qt::Key_F10, Qt::ShiftModifier);
        const QRect caret = cv->caretScreenRect(ed->cursor().position());
        QVERIFY2(under.y() >= caret.bottom() && under.y() <= caret.bottom() + 8 && under.x() >= caret.left() - 2 && under.x() <= caret.right() + 2,
                 qPrintable(QStringLiteral("%1,%2 for %3,%4 %5x%6").arg(under.x()).arg(under.y()).arg(caret.x()).arg(caret.y()).arg(caret.width()).arg(caret.height())));
        // Nothing selected: somewhere in the view.
        ed->endTextEdit();
        ed->clearSelection();
        const QPoint mid = where(Qt::Key_Menu, Qt::NoModifier);
        QVERIFY(cv->viewport()->rect().contains(cv->viewport()->mapFromGlobal(mid)));
        // The menu itself: a key press opens it, and it comes back when it is closed.
        QObject::connect(cv, &Canvas::contextMenuWanted, &w, &DeckWindow::showSlideContextMenu);
        bool opened = false;
        QTimer::singleShot(0, [&] {
            untilDone([&] {
                auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget());
                if (!m) return false;
                opened = true;
                m->close();
                return true;
            });
        });
        QTest::keyClick(cv, Qt::Key_F10, Qt::ShiftModifier);
        QVERIFY(opened);
    }

    // Right-clicking a misspelled word being typed offers the dictionary's suggestions first; choosing one replaces the word.
    void slideMenuOffersSpellingSuggestions()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        Editor *ed = w.editor();
        const QString titleId = placeholderOfSlide(*ed->doc(), 0, 0)->id, storyId = placeholderOfSlide(*ed->doc(), 0, 0)->storyId;
        QTextCursor(ed->doc()->storyDoc(storyId)).insertText(QStringLiteral("This is teh test"));
        ed->beginTextEdit(titleId, 10);
        slideMenu(w, QStringLiteral("menu-spelling"));
        std::unique_ptr<QMenu> menu(w.slideContextMenu());
        const QList<QAction *> actions = menu->actions();
        QVERIFY(actions.size() > 6);
        const QStringList suggestions = spellingSuggestions(QStringLiteral("teh"));
        QVERIFY(suggestions.contains(actions.first()->text()));
        QVERIFY(actions.first()->font().bold());
        QStringList texts;
        for (const QAction *a : actions) texts << a->text();
        QVERIFY(texts.contains(QStringLiteral("Ignore All")) && texts.contains(QStringLiteral("Add to Dictionary")));
        QVERIFY(texts.indexOf(QStringLiteral("Ignore All")) < texts.indexOf(QStringLiteral("Cut")));
        const QString chosen = actions.first()->text();
        actions.first()->trigger();
        QCOMPARE(storyText(w, storyId), QStringLiteral("This is %1 test").arg(chosen));
        // Off a misspelled word there are none.
        ed->beginTextEdit(titleId, 2);
        std::unique_ptr<QMenu> plain(w.slideContextMenu());
        QCOMPARE(plain->actions().first()->objectName(), QStringLiteral("edit.cut"));
    }

    // Home > Editing has Find, Replace, and Select, with Ctrl+F and Ctrl+H; Select offers Select All and Select Objects.
    void homeTabHasFindReplaceAndSelect()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QCOMPARE(w.act(QStringLiteral("edit.find"))->shortcut(), QKeySequence(QKeySequence::Find));
        QCOMPARE(w.act(QStringLiteral("edit.replace"))->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_H));
        QToolButton *selectButton = nullptr;
        int buttons = 0;
        for (QToolButton *b : w.ribbon()->findChildren<QToolButton *>()) {
            if (b->defaultAction() == w.act(QStringLiteral("edit.find")) || b->defaultAction() == w.act(QStringLiteral("edit.replace"))) ++buttons;
            if (b->text() == QLatin1String("Select") && b->menu()) selectButton = b;
        }
        QCOMPARE(buttons, 2);
        QVERIFY(selectButton);
        QVERIFY(selectButton->menu()->actions().contains(w.act(QStringLiteral("edit.selectAll"))));
        QVERIFY(selectButton->menu()->actions().contains(w.act(QStringLiteral("edit.selectObjects"))));
        // The commands open the pane, with the cursor in the box they are for.
        QVERIFY(!w.findPane());
        w.show();
        w.act(QStringLiteral("edit.find"))->trigger();
        QVERIFY(w.findPane() && w.findPane()->isVisible());
        QVERIFY(w.findPane()->speakerNotes()->isChecked());
        QVERIFY(!w.findPane()->matchCase()->isChecked() && !w.findPane()->wholeWords()->isChecked());
        w.act(QStringLiteral("edit.replace"))->trigger();
        QVERIFY(w.findPane()->isVisible());
        // Select Objects is the pointer again, with no text being typed in.
        Editor *ed = w.editor();
        ed->beginTextEdit(placeholderOfSlide(*ed->doc(), 0, 0)->id);
        w.act(QStringLiteral("edit.selectObjects"))->trigger();
        QVERIFY(!ed->isEditingText());
        QCOMPARE(ed->tool(), Tool::Select);
    }

    // Find Next goes through the text of every slide in order from the current one (titles, bodies, shapes, tables, and the notes) and round
    // to the start; each match is shown on its slide, selected.
    void findGoesThroughEverySlideNotesTablesAndShapes()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        fillFindDeck(w);
        Editor *ed = w.editor();
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("review"));
        const QStringList all = {QStringLiteral("1:review@5"), QStringLiteral("2:review@3"), QStringLiteral("2:Review@21"), QStringLiteral("2:review@7 in the notes"),
                                 QStringLiteral("3:Review@0"), QStringLiteral("3:review@13"), QStringLiteral("3:review@5"), QStringLiteral("3:Review@0")};
        QCOMPARE(findAll(w, 9), all + QStringList{QStringLiteral("1:review@5")});
        QVERIFY(pane->status().isEmpty());
        // The match is on its slide, in the notes pane when it is a note.
        ed->setCurrentPage(0);
        ed->endTextEdit();
        QStringList slides;
        for (int i = 0; i < 4; ++i) {
            pane->findNext();
            slides << QString::number(ed->currentPage() + 1);
        }
        QCOMPARE(slides, (QStringList{QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("2"), QStringLiteral("2")}));
        QVERIFY(w.notesPane()->isVisible());
        QCOMPARE(w.notesPane()->textCursor().selectedText(), QStringLiteral("review"));
        QVERIFY(!ed->isEditingText());
        QApplication::processEvents();
        shot(&w, QStringLiteral("find-notes-match"));
        pane->findNext();
        pane->findNext();
        QApplication::processEvents();
        shot(&w, QStringLiteral("find-slide-match"));
        // Not in the notes when they are turned off.
        pane->speakerNotes()->setChecked(false);
        ed->setCurrentPage(0);
        ed->endTextEdit();
        QCOMPARE(findAll(w, 8), (QStringList{QStringLiteral("1:review@5"), QStringLiteral("2:review@3"), QStringLiteral("2:Review@21"), QStringLiteral("3:Review@0"),
                                             QStringLiteral("3:review@13"), QStringLiteral("3:review@5"), QStringLiteral("3:Review@0"), QStringLiteral("1:review@5")}));
        // Match case, and whole words only.
        pane->speakerNotes()->setChecked(true);
        pane->matchCase()->setChecked(true);
        ed->setCurrentPage(0);
        ed->endTextEdit();
        QCOMPARE(findAll(w, 5), (QStringList{QStringLiteral("1:review@5"), QStringLiteral("2:review@3"), QStringLiteral("2:review@7 in the notes"), QStringLiteral("3:review@13"),
                                             QStringLiteral("3:review@5")}));
        pane->matchCase()->setChecked(false);
        pane->wholeWords()->setChecked(true);
        ed->setCurrentPage(0);
        ed->endTextEdit();
        QCOMPARE(findAll(w, 7), (QStringList{QStringLiteral("1:review@5"), QStringLiteral("2:review@3"), QStringLiteral("2:Review@21"), QStringLiteral("2:review@7 in the notes"),
                                             QStringLiteral("3:review@5"), QStringLiteral("3:Review@0"), QStringLiteral("1:review@5")}));
        // Nothing found says so and leaves everything as it was.
        pane->wholeWords()->setChecked(false);
        pane->findBox()->setText(QStringLiteral("nothing like this"));
        const QString before = matchShown(w);
        QVERIFY(!pane->findNext());
        QVERIFY2(pane->status().contains(QStringLiteral("\"nothing like this\" was not found")), qPrintable(pane->status()));
        QCOMPARE(matchShown(w), before);
    }

    // The search starts at the slide being edited, and at the text cursor when text is being typed in.
    void findStartsWhereYouAre()
    {
        DeckWindow w;
        w.show();
        fillFindDeck(w);
        Editor *ed = w.editor();
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("review"));
        ed->setCurrentPage(2);
        QCOMPARE(findAll(w, 3), (QStringList{QStringLiteral("3:Review@0"), QStringLiteral("3:review@13"), QStringLiteral("3:review@5")}));
        ed->setCurrentPage(1);
        ed->endTextEdit();
        ed->clearSelection();
        QCOMPARE(findAll(w, 2), (QStringList{QStringLiteral("2:review@3"), QStringLiteral("2:Review@21")}));
        // With the caret after the first match in the body, the next is the second, not the first again.
        ed->setCurrentPage(1);
        ed->beginTextEdit(placeholderOfSlide(*ed->doc(), 1, 1)->id, 10);
        QCOMPARE(findAll(w, 1), (QStringList{QStringLiteral("2:Review@21")}));
        // A search from the end of the last slide's body goes on to its shape.
        ed->setCurrentPage(2);
        ed->beginTextEdit(placeholderOfSlide(*ed->doc(), 2, 1)->id, 100);
        QVERIFY(ed->isEditingText());
        QCOMPARE(findAll(w, 1), (QStringList{QStringLiteral("3:review@5")}));
    }

    // Replace All swaps every match, notes included, as one step that undoes, and says how many.
    void replaceAllIsOneStepAndSaysHowMany()
    {
        DeckWindow w;
        w.show();
        fillFindDeck(w);
        Editor *ed = w.editor();
        Document *d = ed->doc();
        w.showFindPane(true);
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("review"));
        pane->replaceBox()->setText(QStringLiteral("audit"));
        const QString title = storyText(w, placeholderOfSlide(*d, 0, 0)->storyId);
        const int steps = ed->undoStack()->count();
        pane->replaceAll();
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QCOMPARE(pane->status(), QStringLiteral("JeffDeck made 8 replacements."));
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 0, 0)->storyId), QStringLiteral("Year audit"));
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 1, 1)->storyId), QStringLiteral("We audit profit and audit again"));
        QCOMPARE(deck::notesText(*d, *d->pages[1]), QStringLiteral("Do not audit the numbers"));
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 2, 1)->storyId), QStringLiteral("audits and paudits"));
        QCOMPARE(storyText(w, static_cast<ShapeItem *>(d->pages[2]->items[2].get())->storyId), QStringLiteral("Peer audit"));
        QCOMPARE(storyText(w, static_cast<TableItem *>(d->pages[2]->items[3].get())->cells[0].storyId), QStringLiteral("audit"));
        // The notes pane shows the new words when its slide is the one being edited.
        ed->setCurrentPage(1);
        QCOMPARE(w.notesPane()->toPlainText(), QStringLiteral("Do not audit the numbers"));
        // One Undo puts every word back.
        ed->undo();
        d = ed->doc();
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 0, 0)->storyId), title);
        QCOMPARE(deck::notesText(*d, *d->pages[1]), QStringLiteral("Do not review the numbers"));
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 2, 1)->storyId), QStringLiteral("Reviews and previews"));
        QCOMPARE(storyText(w, static_cast<TableItem *>(d->pages[2]->items[3].get())->cells[0].storyId), QStringLiteral("Review"));
        QCOMPARE(ed->undoStack()->index(), steps);
        // Without the notes, and with case and whole words; one match says "1 replacement", none says none and adds no step.
        pane->speakerNotes()->setChecked(false);
        pane->matchCase()->setChecked(true);
        pane->wholeWords()->setChecked(true);
        pane->replaceAll();
        QCOMPARE(pane->status(), QStringLiteral("JeffDeck made 3 replacements."));
        QCOMPARE(deck::notesText(*d, *d->pages[1]), QStringLiteral("Do not review the numbers"));
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 0, 0)->storyId), QStringLiteral("Year audit"));
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 2, 1)->storyId), QStringLiteral("Reviews and previews"));
        ed->undo();
        pane->findBox()->setText(QStringLiteral("Profit"));
        pane->replaceAll();
        QCOMPARE(pane->status(), QStringLiteral("JeffDeck made 1 replacement."));
        ed->undo();
        const int stepsNow = ed->undoStack()->count();
        pane->findBox()->setText(QStringLiteral("zzz"));
        pane->replaceAll();
        QCOMPARE(ed->undoStack()->count(), stepsNow);
        QVERIFY2(pane->status().contains(QStringLiteral("no replacements")), qPrintable(pane->status()));
    }

    // Replace swaps the match that is shown, in a box or in the notes, and goes on to the next; each is a step of its own.
    void replaceSwapsTheMatchShownAndMovesOn()
    {
        DeckWindow w;
        w.show();
        fillFindDeck(w);
        Editor *ed = w.editor();
        Document *d = ed->doc();
        w.showFindPane(true);
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("review"));
        pane->replaceBox()->setText(QStringLiteral("audit"));
        const int steps = ed->undoStack()->count();
        // Nothing is shown yet: Replace only finds the first.
        pane->replaceOne();
        QCOMPARE(matchShown(w), QStringLiteral("1:review@5"));
        QCOMPARE(ed->undoStack()->count(), steps);
        pane->replaceOne();
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 0, 0)->storyId), QStringLiteral("Year audit"));
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QCOMPARE(matchShown(w), QStringLiteral("2:review@3"));
        pane->replaceOne();
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 1, 1)->storyId), QStringLiteral("We audit profit and Review again"));
        QCOMPARE(matchShown(w), QStringLiteral("2:Review@20"));
        pane->replaceOne();
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 1, 1)->storyId), QStringLiteral("We audit profit and audit again"));
        QCOMPARE(matchShown(w), QStringLiteral("2:review@7 in the notes"));
        // In the notes.
        pane->replaceOne();
        QCOMPARE(deck::notesText(*d, *d->pages[1]), QStringLiteral("Do not audit the numbers"));
        QCOMPARE(ed->undoStack()->count(), steps + 4);
        QCOMPARE(matchShown(w), QStringLiteral("3:Review@0"));
        ed->undo();
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[1]), QStringLiteral("Do not review the numbers"));
    }

    // The pane's boxes are named for screen readers.
    void findPaneControlsAreNamed()
    {
        DeckWindow w;
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        for (QLineEdit *e : pane->findChildren<QLineEdit *>()) {
            QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(e);
            QVERIFY(iface);
            QVERIFY2(!iface->text(QAccessible::Name).trimmed().isEmpty(), qPrintable(e->objectName()));
        }
        for (QAbstractButton *b : pane->findChildren<QAbstractButton *>()) QVERIFY(!b->text().isEmpty());
    }

private:
    // A real mouse click: pressed and let go on the slide at a point of the page, through the canvas.
    static void clickSlide(DeckWindow &w, const QPointF &page, Qt::MouseButton button)
    {
        Canvas *cv = w.canvas();
        const QPointF v = cv->pageToView(page);
        QMouseEvent press(QEvent::MouseButtonPress, v, cv->viewport()->mapToGlobal(v.toPoint()), button, button, Qt::NoModifier);
        QApplication::sendEvent(cv->viewport(), &press);
        QMouseEvent release(QEvent::MouseButtonRelease, v, cv->viewport()->mapToGlobal(v.toPoint()), button, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(cv->viewport(), &release);
    }
    static QString longWords()
    {
        QString out;
        for (int i = 0; i < 40; ++i) out += QStringLiteral("longer words ");
        return out;
    }

private Q_SLOTS:
    // Replace All fits what the new words need, as typing them would: a title a program had shrunk is fitted here from now on, a box that
    // grows to fit grows, a table's rows grow; one Undo gives all of it back.
    void replaceAllClearsAStoredShrinkAndRefitsBoxesAndTables()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        TextItem *title = placeholderOfSlide(*d, 0, 0);
        QTextCursor(d->storyDoc(title->storyId)).insertText(QStringLiteral("review"));
        title->autofit = TextItem::ShrinkOnOverflow;
        title->fitAsStored = true;
        title->fitScale = 0.8;
        auto grow = std::make_shared<TextItem>();
        grow->rect = QRectF(100, 300, 200, 30);
        grow->storyId = d->createStory(QStringLiteral("review"));
        grow->autofit = TextItem::GrowBox;
        d->pages[0]->items.push_back(grow);
        w.insertTable(1, 2);
        auto *table = static_cast<TableItem *>(d->pages[0]->items.back().get());
        QTextCursor(d->storyDoc(table->cells[0].storyId)).insertText(QStringLiteral("Review"));
        ed->clearSelection();
        ed->notifyLive();
        ed->undoStack()->clear();
        const double growHeight = grow->rect.height(), rowHeight = table->rowH[0];
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("review"));
        pane->replaceBox()->setText(longWords());
        pane->replaceAll();
        QVERIFY(!title->fitAsStored);
        QVERIFY2(grow->rect.height() > growHeight + 50, qPrintable(QString::number(grow->rect.height())));
        QVERIFY2(table->rowH[0] > rowHeight + 50, qPrintable(QString::number(table->rowH[0])));
        const double h = grow->rect.height(), r = table->rowH[0];
        ed->autoGrowText(grow.get());
        ed->fitTableRows(table);
        QCOMPARE(grow->rect.height(), h);
        QCOMPARE(table->rowH[0], r);
        ed->undo();
        d = ed->doc();
        QVERIFY(placeholderOfSlide(*d, 0, 0)->fitAsStored);
        QCOMPARE(d->pages[0]->items[2]->rect.height(), growHeight);
        QCOMPARE(static_cast<TableItem *>(d->pages[0]->items[3].get())->rowH[0], rowHeight);
    }

    // Find reaches text in groups and on hidden slides, and Text Art words (shown by selecting the Text Art); the layouts and the
    // slide master are the presentation's design, not its words, and are left alone.
    void findReachesGroupsHiddenSlidesAndTextArtButNotLayouts()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        Document *d = ed->doc();
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 0, 0)->storyId)).insertText(QStringLiteral("Plain review"));
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 1, 0)->storyId)).insertText(QStringLiteral("Hidden review"));
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 2, 0)->storyId)).insertText(QStringLiteral("Group review one"));
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 2, 1)->storyId)).insertText(QStringLiteral("Group review two"));
        d->pages[1]->hidden = true;
        auto art = std::make_shared<TextArtItem>();
        art->rect = QRectF(100, 400, 300, 80);
        art->text = QStringLiteral("Art review words");
        d->pages[2]->items.push_back(art);
        auto art2 = std::make_shared<TextArtItem>();
        art2->rect = QRectF(100, 300, 300, 80);
        art2->text = QStringLiteral("Second review art");
        d->pages[2]->items.push_back(art2);
        const QString artId = art->id, art2Id = art2->id;
        // A word in a layout's prompt: not the presentation's.
        QString layoutStory;
        for (const auto &m : d->masters)
            for (const auto &it : m->items)
                if (it->type() == ItemType::Text && layoutStory.isEmpty()) layoutStory = static_cast<TextItem *>(it.get())->storyId;
        QVERIFY(!layoutStory.isEmpty());
        QTextCursor(d->storyDoc(layoutStory)).insertText(QStringLiteral(" review"));
        ed->setCurrentPage(2);
        ed->select(QStringList{placeholderOfSlide(*d, 2, 0)->id, placeholderOfSlide(*d, 2, 1)->id});
        slideMenu(w);   // the commands are enabled as the selection allows
        w.act(QStringLiteral("arr.group"))->trigger();
        QCOMPARE(ed->selectionKind(), QStringLiteral("group"));
        ed->clearSelection();
        ed->setCurrentPage(0);
        ed->notifyLive();
        ed->undoStack()->clear();
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("review"));
        QCOMPARE(findAll(w, 4), (QStringList{QStringLiteral("1:review@6"), QStringLiteral("2:review@7"), QStringLiteral("3:review@6"), QStringLiteral("3:review@6")}));
        QCOMPARE(ed->currentPage(), 2);
        QVERIFY(ed->isEditingText());
        // The Text Art is shown by selecting it, and Replace swaps its words; the next search goes on from there.
        QVERIFY(pane->findNext());
        QCOMPARE(ed->currentPage(), 2);
        QVERIFY(!ed->isEditingText());
        QCOMPARE(ed->selection(), QStringList{artId});
        pane->replaceBox()->setText(QStringLiteral("audit"));
        pane->replaceOne();
        QCOMPARE(static_cast<TextArtItem *>(ed->doc()->item(artId))->text, QStringLiteral("Art audit words"));
        QCOMPARE(ed->selection(), QStringList{art2Id});   // on to the second
        // Replace All: the group's, the hidden slide's, the first slide's, and the other Text Art's, but not the layout's.
        pane->replaceAll();
        QCOMPARE(pane->status(), QStringLiteral("JeffDeck made 5 replacements."));
        d = ed->doc();
        QVERIFY(storyText(w, layoutStory).contains(QStringLiteral(" review")));
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 1, 0)->storyId), QStringLiteral("Hidden audit"));
        QCOMPARE(static_cast<TextArtItem *>(d->item(art2Id))->text, QStringLiteral("Second audit art"));
        ed->undo();
        QCOMPARE(static_cast<TextArtItem *>(ed->doc()->item(art2Id))->text, QStringLiteral("Second review art"));
        QCOMPARE(static_cast<TextArtItem *>(ed->doc()->item(artId))->text, QStringLiteral("Art audit words"));
        pane->findBox()->setText(QStringLiteral("review words"));
        QVERIFY(!pane->findNext());
        pane->findBox()->setText(QStringLiteral("audit words"));
        QVERIFY(pane->findNext());
        QCOMPARE(ed->selection(), QStringList{artId});
    }

    // The words that replace a match are not searched for again: after Replace the next match is the next one, and Replace All makes one pass.
    void aReplacementThatHoldsTheSearchTextIsNotFoundAgain()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 0, 0)->storyId)).insertText(QStringLiteral("cat and cat"));
        QTextCursor(d->storyDoc(d->pages[0]->notesStoryId)).insertText(QStringLiteral("cat"));
        ed->notifyLive();
        ed->undoStack()->clear();
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("cat"));
        pane->replaceBox()->setText(QStringLiteral("concatenate"));
        pane->replaceOne();
        QCOMPARE(matchShown(w), QStringLiteral("1:cat@0"));
        pane->replaceOne();
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 0, 0)->storyId), QStringLiteral("concatenate and cat"));
        QCOMPARE(matchShown(w), QStringLiteral("1:cat@16"));
        pane->replaceOne();
        QCOMPARE(storyText(w, placeholderOfSlide(*d, 0, 0)->storyId), QStringLiteral("concatenate and concatenate"));
        QCOMPARE(matchShown(w), QStringLiteral("1:cat@0 in the notes"));
        pane->replaceOne();
        QCOMPARE(deck::notesText(*d, *d->pages[0]), QStringLiteral("concatenate"));
        ed->undo();
        ed->undo();
        ed->undo();
        pane->replaceAll();
        QCOMPARE(pane->status(), QStringLiteral("JeffDeck made 3 replacements."));
        QCOMPARE(storyText(w, placeholderOfSlide(*ed->doc(), 0, 0)->storyId), QStringLiteral("concatenate and concatenate"));
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QStringLiteral("concatenate"));
    }

    // Two matches in one notes box are both found, in order, and Replace on the first leaves the second to be found where it moved to.
    void twoMatchesInOneNotesBoxAreBothFound()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        QTextCursor(d->storyDoc(d->pages[0]->notesStoryId)).insertText(QStringLiteral("review one and review two"));
        ed->notifyLive();
        ed->undoStack()->clear();
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        pane->findBox()->setText(QStringLiteral("review"));
        QCOMPARE(findAll(w, 3), (QStringList{QStringLiteral("1:review@0 in the notes"), QStringLiteral("1:review@15 in the notes"), QStringLiteral("1:review@0 in the notes")}));
        pane->replaceBox()->setText(QStringLiteral("audit"));
        pane->replaceOne();
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QStringLiteral("audit one and review two"));
        QCOMPARE(matchShown(w), QStringLiteral("1:review@14 in the notes"));
        pane->replaceOne();
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QStringLiteral("audit one and audit two"));
        QVERIFY(!pane->findNext());
        QVERIFY2(pane->status().contains(QStringLiteral("was not found")), qPrintable(pane->status()));
    }

    // A real right click (pressed and let go on the slide, not a menu request made up in the test) selects the object it is on, keeps the
    // objects selected together when it is on one of them, and clears the selection on the bare slide.
    void aRightClickSelectsTheObjectUnderIt()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 0, 0)->storyId)).insertText(QStringLiteral("Title words"));
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 0, 1)->storyId)).insertText(QStringLiteral("Subtitle words"));
        ed->notifyLive();
        QApplication::processEvents();
        const QString titleId = placeholderOfSlide(*d, 0, 0)->id, subId = placeholderOfSlide(*d, 0, 1)->id;
        const QPointF titleAt = placeholderOfSlide(*d, 0, 0)->bounds().center(), subAt = placeholderOfSlide(*d, 0, 1)->bounds().center();
        ed->select(titleId);
        clickSlide(w, subAt, Qt::RightButton);
        QCOMPARE(ed->selection(), QStringList{subId});
        QVERIFY(slideMenu(w).contains(QStringLiteral("edit.cut")));
        // On one of two selected objects, both stay selected.
        ed->select(QStringList{titleId, subId});
        clickSlide(w, titleAt, Qt::RightButton);
        QCOMPARE(ed->selection().size(), 2);
        QVERIFY(slideMenu(w).contains(QStringLiteral("arr.group")));
        // On the bare slide, nothing is selected.
        clickSlide(w, QPointF(5, 5), Qt::RightButton);
        QVERIFY(ed->selection().isEmpty());
        QVERIFY(slideMenu(w).contains(QStringLiteral("slide.reset")));
    }

    // A right click on the bare slide while typing ends the typing, so the menu is the slide's; on the text being typed in, it stays.
    void aRightClickOnTheBareSlideWhileTypingEndsTheTyping()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        QTextCursor(d->storyDoc(placeholderOfSlide(*d, 0, 0)->storyId)).insertText(QStringLiteral("Title words"));
        ed->notifyLive();
        QApplication::processEvents();
        const QString titleId = placeholderOfSlide(*d, 0, 0)->id;
        const QPointF titleAt = placeholderOfSlide(*d, 0, 0)->bounds().center();
        ed->beginTextEdit(titleId);
        clickSlide(w, titleAt, Qt::RightButton);
        QVERIFY(ed->isEditingText());
        QVERIFY(slideMenu(w).contains(QStringLiteral("fmt.fontDialog")));
        clickSlide(w, QPointF(5, 5), Qt::RightButton);
        QVERIFY(!ed->isEditingText());
        QVERIFY(ed->selection().isEmpty());
        const QStringList ids = slideMenu(w);
        QVERIFY2(ids.contains(QStringLiteral("slide.reset")) && !ids.contains(QStringLiteral("fmt.fontDialog")), qPrintable(ids.join(QLatin1Char(' '))));
    }

    // The pane's message is plain text, and a long search text can't stretch the pane: it is cut short and may break anywhere.
    void findPaneMessageIsPlainTextAndNarrow()
    {
        DeckWindow w;
        w.resize(1440, 900);
        w.show();
        w.showFindPane();
        DeckFindPane *pane = w.findPane();
        QLabel *status = pane->findChild<QLabel *>(QStringLiteral("findStatus"));
        QVERIFY(status);
        pane->findBox()->setText(QStringLiteral("<b>x"));
        pane->findNext();
        QCOMPARE(status->textFormat(), Qt::PlainText);
        QVERIFY2(pane->status().contains(QStringLiteral("\"<b>x\" was not found")), qPrintable(pane->status()));
        pane->findBox()->setText(QString(120, QLatin1Char('x')));
        pane->findNext();
        QVERIFY2(pane->status().size() < 200, qPrintable(QString::number(pane->status().size())));
        QVERIFY2(status->minimumSizeHint().width() < 300, qPrintable(QString::number(status->minimumSizeHint().width())));
        pane->replaceBox()->setText(QStringLiteral("y"));
        pane->replaceAll();
        QVERIFY2(pane->status().size() < 200, qPrintable(QString::number(pane->status().size())));
        QVERIFY2(status->minimumSizeHint().width() < 300, qPrintable(QString::number(status->minimumSizeHint().width())));
    }
};

JP_TEST_CLASS(DeckWindowTests)
#include "tst_deck_window.moc"
