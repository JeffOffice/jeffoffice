// JeffDeck's tests of the file screen, saving from the window, and reusing slides.
#include "deckhelpers.h"

class DeckFileTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

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
                                  QStringLiteral("JeffWrite, Documents, Preview"), QStringLiteral("JeffCrunch, Spreadsheets, Preview"),
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
        QCOMPARE(comingSoon, 2);   // JeffDeck is out, and JeffWrite and JeffCrunch are previews
        QVERIFY(version);
        QVERIFY(links);
        QVERIFY(license);
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

private:
    // A presentation to take slides from, built here: the chart sample (a chart with its workbook, a picture, notes,
    // a slide's transition and animation, a frame kept as it was) with a movie on its first slide, under a master
    // and layouts of its own whose theme has other fonts than a new presentation's.
    static QByteArray reuseMovie() { return QByteArray("not really a movie, but kept byte for byte"); }
    static std::unique_ptr<Document> reuseSource()
    {
        Parts p = chartPptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        xml.replace(QStringLiteral("</p:spTree>"),
                    QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"20\" name=\"Movie\"><a:hlinkClick r:id=\"\" action=\"ppaction://media\"/></p:cNvPr><p:cNvPicPr/><p:nvPr><a:videoFile r:link=\"rId5\"/>"
                                   "<p:extLst><p:ext uri=\"{DAA4B4D4-6D71-4841-9C94-3DE7FCFB9230}\"><p14:media xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\" r:embed=\"rId6\"/></p:ext></p:extLst>"
                                   "</p:nvPr></p:nvPicPr><p:blipFill><a:blip r:embed=\"rId2\"/><a:stretch><a:fillRect/></a:stretch></p:blipFill><p:spPr>")
                        + xfrm(0, 0, 1828800, 1371600) + QStringLiteral("<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></p:spPr></p:pic></p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        p.data["/ppt/media/media1.mp4"] = {QStringLiteral("video/mp4"), reuseMovie()};
        p.rels << Rel{"/ppt/slides/slide1.xml", "video", "/ppt/media/media1.mp4"} << Rel{"/ppt/slides/slide1.xml", "http://schemas.microsoft.com/office/2007/relationships/media", "/ppt/media/media1.mp4"};
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        if (!d) return nullptr;
        // The title in a size of its own, and a body with one bold word: what the layout gives and what the writer chose.
        for (const auto &it : d->pages[0]->items) {
            auto *text = it->type() == ItemType::Text ? static_cast<TextItem *>(it.get()) : nullptr;
            if (!text) continue;
            QTextDocument *story = d->storyDoc(text->storyId);
            if (deck::placeholderType(*it) == QLatin1String("title")) {
                QTextCursor c(story);
                c.select(QTextCursor::Document);
                QTextCharFormat f;
                f.setFontPointSize(31);
                c.mergeCharFormat(f);
            } else if (deck::placeholderType(*it) == QLatin1String("obj")) {
                QTextCursor c(story);
                c.insertText(QStringLiteral("Plain and "));
                QTextCharFormat bold;
                bold.setFontWeight(QFont::Bold);
                c.insertText(QStringLiteral("bold"), bold);
            }
        }
        return d;
    }
    static const Item *namedItem(const PageBase &page, const QString &name)
    {
        const Item *found = nullptr;
        walkItems(page.items, [&](const ItemPtr &it) {
            if (it->name == name) found = it.get();
        });
        return found;
    }
    // Every object's id in the presentation, with how many there are.
    static QSet<QString> allItemIds(const Document &d, int *count)
    {
        QSet<QString> ids;
        *count = 0;
        d.forEachItem([&](Item *it, int, const QString &) {
            ids.insert(it->id);
            ++*count;
        });
        return ids;
    }

private Q_SLOTS:
    // A slide taken from another presentation without its formatting stands on this one's layout of
    // its kind, its title and text in that layout's places and look (what the writer emphasized stays),
    // and brings what is its own: pictures, movies, notes, a chart, an animation and a transition, all with new ids.
    void reuseTakesTheDestinationsLayout()
    {
        auto src = reuseSource();
        QVERIFY(src);
        auto dest = deck::newPresentation();
        deck::addSlide(*dest, *deck::layoutOfType(*dest, QStringLiteral("obj")));
        const MasterPage *layout = deck::layoutOfType(*dest, QStringLiteral("obj"));
        const int masters = int(dest->masters.size());
        int destItemsBefore;
        allItemIds(*dest, &destItemsBefore);
        QCOMPARE(deck::insertSlideFrom(*dest, 1, *src, 0, false), 1);
        QCOMPARE(int(dest->pages.size()), 3);
        QCOMPARE(int(dest->masters.size()), masters);   // no layout or master comes with it
        const Page &page = *dest->pages[1];
        const Page &was = *src->pages[0];
        QCOMPARE(page.masterId, layout->id);
        QCOMPARE(int(src->pages.size()), 2);
        QCOMPARE(titleOfSlide(*src, 0), QStringLiteral("Quarterly results"));   // the source is as it was

        // The title and the text: in the layout's places, in the layout's look.
        const Item *title = placeholderOn(page, QStringLiteral("title"));
        QVERIFY(title);
        const Item *layoutTitle = nullptr;
        for (const auto &it : layout->items)
            if (deck::placeholderType(*it) == QLatin1String("title")) layoutTitle = it.get();
        QVERIFY(layoutTitle);
        QCOMPARE(title->rect, layoutTitle->rect);
        QTextDocument *titleStory = dest->storyDoc(static_cast<const TextItem *>(title)->storyId);
        QCOMPARE(titleStory->toPlainText(), QStringLiteral("Quarterly results"));
        QCOMPARE(titleStory->begin().begin().fragment().charFormat().fontPointSize(), 44.0);
        QCOMPARE(titleStory->begin().begin().fragment().charFormat().fontFamilies().toStringList().value(0), deck::themeOf(*dest).headingFont);
        QTextDocument *body = bodyOfSlide(*dest, 1);
        QVERIFY(body);
        QCOMPARE(body->toPlainText(), QStringLiteral("Plain and bold"));
        QTextBlock::iterator fragment = body->begin().begin();
        QCOMPARE(fragment.fragment().charFormat().fontPointSize(), 28.0);
        QVERIFY(fragment.fragment().charFormat().fontWeight() < QFont::DemiBold);
        ++fragment;
        QCOMPARE(fragment.fragment().text(), QStringLiteral("bold"));
        QVERIFY(fragment.fragment().charFormat().fontWeight() >= QFont::Bold);
        QCOMPARE(fragment.fragment().charFormat().fontPointSize(), 28.0);

        // What the slide brought: the picture, the movie, the chart, the notes, the animation and the transition.
        const auto *picture = static_cast<const PictureItem *>(namedItem(page, QStringLiteral("Picture 7")));
        QVERIFY(picture && picture->type() == ItemType::Picture);
        QVERIFY(dest->images.contains(picture->imageId));
        QCOMPARE(dest->images.value(picture->imageId).bytes, tinyPng());
        const auto *movie = static_cast<const PictureItem *>(namedItem(page, QStringLiteral("Movie")));
        QVERIFY(movie && movie->type() == ItemType::Picture);
        QCOMPARE(movie->mediaKind, QStringLiteral("video"));
        QVERIFY(dest->media.contains(movie->mediaId));
        QCOMPARE(dest->media.value(movie->mediaId).bytes, reuseMovie());
        QCOMPARE(dest->media.value(movie->mediaId).contentType, QStringLiteral("video/mp4"));
        const PictureItem *chart = chartPicture(*dest, page), *chartWas = chartPicture(*src, was);
        QVERIFY(chart && chartWas);
        QVERIFY(chart->id != chartWas->id);
        QVERIFY(dest->images.contains(chart->imageId));
        const QJsonArray charts = page.props.value(QStringLiteral("charts")).toArray();
        QCOMPARE(int(charts.size()), 1);
        QCOMPARE(charts[0].toObject().value(QStringLiteral("parts")), was.props.value(QStringLiteral("charts")).toArray()[0].toObject().value(QStringLiteral("parts")));
        QCOMPARE(deck::notesText(*dest, page), QStringLiteral("Say hello first\nThen the numbers"));
        QVERIFY(page.notesStoryId != was.notesStoryId);
        QVERIFY(!src->stories.contains(page.notesStoryId));
        QCOMPARE(page.props.value(QStringLiteral("timingXml")), was.props.value(QStringLiteral("timingXml")));
        QVERIFY(!page.props.value(QStringLiteral("timingXml")).toString().isEmpty());
        QCOMPARE(page.props.value(QStringLiteral("transitionXml")), was.props.value(QStringLiteral("transitionXml")));
        QVERIFY(!page.props.value(QStringLiteral("transitionXml")).toString().isEmpty());
        QCOMPARE(page.props.value(QStringLiteral("unread")), was.props.value(QStringLiteral("unread")));
        QVERIFY(!page.props.value(QStringLiteral("unread")).toArray().isEmpty());
        // The shapes keep their numbers in the file: what the animation aims at.
        QCOMPARE(namedItem(page, QStringLiteral("Movie"))->sourceId, namedItem(was, QStringLiteral("Movie"))->sourceId);

        // New ids all through: no object, story, or page shares one with the source, or with another in this presentation.
        int count;
        const QSet<QString> ids = allItemIds(*dest, &count);
        QCOMPARE(int(ids.size()), count);
        QCOMPARE(count, destItemsBefore + int(page.items.size()));
        int sourceCount;
        for (const QString &id : allItemIds(*src, &sourceCount)) QVERIFY2(!ids.contains(id), qPrintable(id));
        QVERIFY(page.id != was.id);
        for (const auto &it : page.items)
            if (it->type() == ItemType::Text) {
                const QString story = static_cast<const TextItem *>(it.get())->storyId;
                QVERIFY(dest->stories.contains(story) && !src->stories.contains(story));
            }
        // A slide at the end goes at the end; one past it too.
        QCOMPARE(deck::insertSlideFrom(*dest, 99, *src, 1, false), 3);
        QCOMPARE(int(dest->pages.size()), 4);
        QCOMPARE(titleOfSlide(*dest, 3), QStringLiteral("Second"));
        QCOMPARE(deck::insertSlideFrom(*dest, 0, *src, 5, false), -1);   // no such slide
        QCOMPARE(int(dest->pages.size()), 4);
        // The slide that has a background of its own and is hidden: hidden stays, the background is the layout's.
        QVERIFY(dest->pages[3]->hidden);
        QVERIFY(dest->pages[3]->background.isNone());
        QVERIFY(!src->pages[1]->background.isNone());
        // A slide on a title layout lands on the title layout.
        QCOMPARE(deck::layoutType(*dest->masterFor(*dest->pages[3])), QStringLiteral("title"));
    }

    // With its formatting kept, a slide brings its layout and slide master (named as they were, with their
    // theme), once for each: a second slide from the same master finds them here.
    void reuseKeepsTheSourceFormattingWithItsLayoutAndMaster()
    {
        auto src = reuseSource();
        QVERIFY(src);
        auto dest = deck::newPresentation();
        const int masters = int(dest->masters.size());
        QCOMPARE(deck::insertSlideFrom(*dest, 1, *src, 0, true), 1);
        QCOMPARE(int(dest->masters.size()), masters + 2);   // the master and the layout
        const Page &page = *dest->pages[1];
        const MasterPage *layout = dest->masterFor(page);
        QVERIFY(layout && deck::isLayout(*layout));
        QCOMPARE(layout->name, QStringLiteral("Title and Content"));
        QCOMPARE(deck::layoutType(*layout), QStringLiteral("obj"));
        const MasterPage *master = dest->master(layout->baseId);
        QVERIFY(master && deck::isSlideMaster(*master));
        QCOMPARE(master->name, QStringLiteral("Test Master"));
        QVERIFY(master != dest->masters[0].get());
        QCOMPARE(master->props.value(QStringLiteral("themeXml")).toString(), src->appData.value(QStringLiteral("themeXml")).toString());
        QVERIFY(!master->props.value(QStringLiteral("themeXml")).toString().isEmpty());
        // The master's own band stays, and the layout's places are the source's.
        QVERIFY(namedItem(*master, QStringLiteral("Master Band")));
        const Item *title = placeholderOn(page, QStringLiteral("title"));
        const Item *sourceTitle = placeholderOn(*src->pages[0], QStringLiteral("title"));
        QVERIFY(title && sourceTitle);
        QCOMPARE(title->rect, sourceTitle->rect);
        QTextDocument *titleStory = dest->storyDoc(static_cast<const TextItem *>(title)->storyId);
        QCOMPARE(titleStory->begin().begin().fragment().charFormat().fontPointSize(), 31.0);
        // The source's theme fonts are fonts of the source: Georgia, not what this presentation's headings are.
        QCOMPARE(titleStory->begin().begin().fragment().charFormat().fontFamilies().toStringList().value(0), QStringLiteral("Georgia"));
        QVERIFY(deck::themeOf(*dest).headingFont != QStringLiteral("Georgia"));
        // The master's pictures and the layout's objects came with new ids and stories of their own.
        int count;
        const QSet<QString> ids = allItemIds(*dest, &count);
        QCOMPARE(int(ids.size()), count);
        // A slide on another layout of that master: only that layout comes; the master was copied already.
        QCOMPARE(deck::insertSlideFrom(*dest, 2, *src, 1, true), 2);
        QCOMPARE(int(dest->masters.size()), masters + 3);
        const MasterPage *titleLayout = dest->masterFor(*dest->pages[2]);
        QVERIFY(titleLayout && deck::layoutType(*titleLayout) == QLatin1String("title"));
        QCOMPARE(titleLayout->baseId, master->id);
        // The same layout again finds the one that is here.
        QCOMPARE(deck::insertSlideFrom(*dest, 3, *src, 0, true), 3);
        QCOMPARE(deck::insertSlideFrom(*dest, 4, *src, 1, true), 4);
        QCOMPARE(int(dest->masters.size()), masters + 3);
        QCOMPARE(dest->pages[3]->masterId, page.masterId);
        // A slide's own background and its hiding come along; with the formatting kept, the background stays.
        QVERIFY(dest->pages[2]->hidden);
        QVERIFY(dest->pages[2]->background == src->pages[1]->background);
        QVERIFY(!dest->pages[2]->background.isNone());
        // A master that looks the same as one here is the one here: reusing a slide of this presentation's own kind adds nothing.
        auto same = deck::newPresentation();
        const int sameMasters = int(dest->masters.size());
        QCOMPARE(deck::insertSlideFrom(*dest, 5, *same, 0, true), 5);
        QCOMPARE(int(dest->masters.size()), sameMasters);
        QCOMPARE(dest->masterFor(*dest->pages[5])->id, deck::layoutOfType(*dest, QStringLiteral("title"), dest->masters[0]->id)->id);
        // What came with the slides is a sound file when saved: the charts' parts, the movie, the extra masters and their themes.
        {
            QString err;
            Exported e;
            const QByteArray bytes = deck::exportPresentation(*dest, &err);
            QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
            QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
            auto back = deck::importPresentation(bytes, &err);
            QVERIFY2(back, qPrintable(err));
            QCOMPARE(int(back->pages.size()), int(dest->pages.size()));
            QCOMPARE(titleOfSlide(*back, 1), QStringLiteral("Quarterly results"));
            QCOMPARE(back->masterFor(*back->pages[1])->name, QStringLiteral("Title and Content"));
            QCOMPARE(back->master(back->masterFor(*back->pages[1])->baseId)->name, QStringLiteral("Test Master"));
            QVERIFY(chartPicture(*back, *back->pages[1]));
            QVERIFY(namedItem(*back->pages[1], QStringLiteral("Movie")));
        }
        // The sections follow: the copy is in the section of the slide before it.
        deck::addSection(*dest, 3, QStringLiteral("Later"));
        const int before = int(dest->pages.size());
        deck::insertSlideFrom(*dest, 3, *src, 0, false);
        QCOMPARE(int(dest->pages.size()), before + 1);
        QCOMPARE(sectionCounts(*dest).value(0), 4);
    }

    // A slide from a presentation of another slide size is fitted to this one's, as Slide Size
    // does it: what it holds keeps its shape, scaled and centered, its layout and master stretch.
    void reuseFitsASlideToThisPresentationsSlideSize()
    {
        auto src = reuseSource();
        QVERIFY(src);
        QCOMPARE(src->pageSize(), QSizeF(960, 540));
        auto dest = deck::newPresentation();
        deck::setSlideSize(*dest, QSizeF(720, 540));
        const QRectF was = namedItem(*src->pages[0], QStringLiteral("Picture 7"))->rect;
        QCOMPARE(deck::insertSlideFrom(*dest, 1, *src, 0, false), 1);
        const QRectF page(QPointF(0, 0), dest->pageSize());
        const QRectF fitted = namedItem(*dest->pages[1], QStringLiteral("Picture 7"))->rect;
        QVERIFY2(page.contains(fitted), qPrintable(QStringLiteral("%1,%2 %3x%4").arg(fitted.x()).arg(fitted.y()).arg(fitted.width()).arg(fitted.height())));
        QVERIFY(std::abs(fitted.width() - was.width() * 0.75) < 0.01 && std::abs(fitted.height() - was.height() * 0.75) < 0.01);
        QVERIFY(std::abs(fitted.x() - (was.x() * 0.75)) < 0.01);   // 720/960 across, and the height ratio is 1: scaled by 0.75, centered
        for (const auto &it : dest->pages[1]->items) QVERIFY2(page.adjusted(-0.01, -0.01, 0.01, 0.01).contains(it->rect), qPrintable(it->name + " " + it->role));
        // With the formatting kept, the layout and the master come stretched to this slide size too.
        QCOMPARE(deck::insertSlideFrom(*dest, 2, *src, 0, true), 2);
        const MasterPage *layout = dest->masterFor(*dest->pages[2]);
        QVERIFY(layout);
        for (const MasterPage *m : QList<const MasterPage *>{layout, dest->master(layout->baseId)}) {
            QVERIFY(m);
            for (const auto &it : m->items) QVERIFY2(page.adjusted(-0.01, -0.01, 0.01, 0.01).contains(it->rect), qPrintable(m->name + " " + it->name + " " + it->role));
        }
        const Item *band = namedItem(*dest->master(layout->baseId), QStringLiteral("Master Band"));
        QVERIFY(band);
        QVERIFY(std::abs(band->rect.width() - 720) < 0.01);
        for (const auto &it : dest->pages[2]->items) QVERIFY2(page.adjusted(-0.01, -0.01, 0.01, 0.01).contains(it->rect), qPrintable(it->name + " " + it->role));
        // The same size changes nothing.
        auto same = deck::newPresentation();
        deck::insertSlideFrom(*same, 1, *src, 0, false);
        QCOMPARE(namedItem(*same->pages[1], QStringLiteral("Picture 7"))->rect, was);
    }

    // The Reuse Slides pane: Browse picks a presentation, its slides are listed as thumbnails with titles, and a
    // click puts a copy after the slide being edited, as one step that undoes. "Keep source formatting" is off
    // until it is turned on.
    void reuseSlidesPaneInsertsACopyAfterTheCurrentSlide()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("source.pptx"));
        {
            QString err;
            auto src = reuseSource();
            QVERIFY(src);
            QVERIFY2(deck::savePresentationFile(*src, path, &err), qPrintable(err));
        }
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        Editor *ed = w.editor();
        w.newSlide();
        ed->setCurrentPage(0);
        ed->undoStack()->clear();
        QVERIFY(w.act("slide.reuse"));
        QCOMPARE(w.act("slide.reuse")->text(), QStringLiteral("Reuse Slides…"));
        QVERIFY(!w.reusePane() || !w.reusePane()->isVisible());
        // The New Slide menu offers it.
        QMenu *menu = nullptr;
        for (QObject *o : w.act("slide.new")->associatedObjects())
            if (auto *button = qobject_cast<QToolButton *>(o)) menu = button->menu();
        QVERIFY(menu);
        QVERIFY(menu->actions().contains(w.act("slide.reuse")));
        w.act("slide.reuse")->trigger();
        ReusePane *pane = w.reusePane();
        QVERIFY(pane && pane->isVisible());
        QVERIFY(w.centralWidget()->geometry().right() <= pane->mapTo(&w, QPoint(0, 0)).x() + 2);   // on the right of the slides
        QVERIFY(!pane->keepFormatting());
        QCOMPARE(pane->slideCount(), 0);
        // A file that isn't a presentation says so and lists nothing.
        const QString bad = dir.filePath(QStringLiteral("bad.pptx"));
        QFile b(bad);
        QVERIFY(b.open(QIODevice::WriteOnly));
        b.write("not a presentation");
        b.close();
        QVERIFY(!pane->open(bad));
        QCOMPARE(pane->slideCount(), 0);
        QVERIFY(!pane->message().isEmpty());
        QVERIFY(pane->open(path));
        QCOMPARE(pane->slideCount(), 2);
        QTRY_COMPARE(pane->renderedCount(), 2);
        QCOMPARE(pane->list()->item(0)->text(), QStringLiteral("1  Quarterly results"));
        QCOMPARE(pane->list()->item(1)->text(), QStringLiteral("2  Second"));
        QVERIFY(!pane->list()->item(0)->icon().isNull());
        QVERIFY(pane->message().isEmpty());
        // A click inserts after the slide being edited.
        QListWidget *list = pane->list();
        QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualItemRect(list->item(0)).center());
        QCOMPARE(int(ed->doc()->pages.size()), 3);
        QCOMPARE(titleOfSlide(*ed->doc(), 1), QStringLiteral("Quarterly results"));
        QCOMPARE(ed->currentPage(), 1);
        QCOMPARE(ed->undoStack()->count(), 1);
        QCOMPARE(int(ed->doc()->masters.size()), 10);
        ed->undo();
        QCOMPARE(int(ed->doc()->pages.size()), 2);
        QCOMPARE(titleOfSlide(*ed->doc(), 1), QString());
        ed->redo();
        QCOMPARE(int(ed->doc()->pages.size()), 3);
        // With the check box on, the slide brings its layout and master; undone, they go.
        auto *keep = pane->findChild<QCheckBox *>();
        QVERIFY(keep);
        QCOMPARE(keep->text(), QStringLiteral("Keep source formatting"));
        keep->click();
        QVERIFY(pane->keepFormatting());
        QTest::mouseClick(list->viewport(), Qt::LeftButton, Qt::NoModifier, list->visualItemRect(list->item(1)).center());
        QCOMPARE(int(ed->doc()->pages.size()), 4);
        QCOMPARE(ed->currentPage(), 2);
        QCOMPARE(titleOfSlide(*ed->doc(), 2), QStringLiteral("Second"));
        QCOMPARE(int(ed->doc()->masters.size()), 12);
        QCOMPARE(ed->undoStack()->count(), 2);
        ed->undo();
        QCOMPARE(int(ed->doc()->pages.size()), 3);
        QCOMPARE(int(ed->doc()->masters.size()), 10);
        // The source file is untouched by all this, and the pane can be put away.
        QVERIFY(QFileInfo(path).size() > 0);
        w.reusePane()->setKeepFormatting(false);
        QVERIFY(!keep->isChecked());
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
};

JP_TEST_CLASS(DeckFileTests)
#include "tst_deck_file.moc"
