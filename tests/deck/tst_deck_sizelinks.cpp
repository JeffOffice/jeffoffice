// JeffDeck's tests of slide sizes, links and action buttons, and gradient and picture backgrounds.
#include "deckhelpers.h"
#include "../withdialog.h"
#include "core/linkareas.h"
#include "office/hyperlink.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QToolTip>

namespace {

std::shared_ptr<ShapeItem> linkShape(Page &slide, const QRectF &rect, const QString &link)
{
    auto shape = std::make_shared<ShapeItem>();
    shape->rect = rect;
    shape->hyperlink = link;
    slide.items.push_back(shape);
    return shape;
}

QStringList pdfLinkDictionaries(const QByteArray &pdf)
{
    QStringList out;
    const QString text = QString::fromLatin1(pdf);
    for (auto it = QRegularExpression(QStringLiteral("<< /Type /Annot /Subtype /Link[^\\n]*")).globalMatch(text); it.hasNext();) out << it.next().captured(0);
    return out;
}

} // namespace

class DeckSizeLinkTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // ---------------- slide sizes ----------------

    // Paper, screen, and banner sizes are named, and a size that is none of them is custom.
    void slideSizesHaveNames()
    {
        QCOMPARE(deck::slideSizeName(QSizeF(792, 612)), QStringLiteral("Letter Paper"));
        QCOMPARE(deck::slideSizeName(QSizeF(842, 595)), QStringLiteral("A4 Paper"));
        QCOMPARE(deck::slideSizeName(QSizeF(720, 450)), QStringLiteral("On-screen Show (16:10)"));
        QCOMPARE(deck::slideSizeName(QSizeF(576, 72)), QStringLiteral("Banner"));
        QCOMPARE(deck::slideSizeName(QSizeF(540, 720)), QStringLiteral("Custom"));   // portrait is a size of its own
        QCOMPARE(deck::slideSizeName(QSizeF(700, 333)), QStringLiteral("Custom"));
        QVERIFY(deck::slideSizePresets().size() >= 8);
    }

    // A size is kept when the objects are not scaled: nothing moves, the slides are the new size.
    void keepSizeMovesNothing()
    {
        auto d = deck::newPresentation();
        auto shape = linkShape(*d->pages[0], QRectF(100, 100, 200, 100), QString());
        const QRectF title = d->pages[0]->items[0]->rect;
        deck::setSlideSize(*d, QSizeF(540, 720), false);
        QCOMPARE(d->pageSize(), QSizeF(540, 720));
        QCOMPARE(shape->rect, QRectF(100, 100, 200, 100));
        QCOMPARE(d->pages[0]->items[0]->rect, title);
        // Scaled, the same change moves them.
        auto e = deck::newPresentation();
        auto s2 = linkShape(*e->pages[0], QRectF(100, 100, 200, 100), QString());
        deck::setSlideSize(*e, QSizeF(540, 720), true);
        QVERIFY(s2->rect != QRectF(100, 100, 200, 100));
    }

    // Slide Size from the menu: the new sizes are listed, and the box offers scaling or keeping the size; either is one step that undoes.
    void slideSizeMenuAsksAndIsOneStep()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        QVERIFY(w.act("design.size.a4"));
        QVERIFY(w.act("design.size.letter"));
        QVERIFY(w.act("design.size.banner"));
        QVERIFY(w.act("design.sizeCustom"));
        const QRectF title = ed->doc()->pages[0]->items[0]->rect;
        auto answer = [&](const char *button) {
            return [button](QDialog *dlg) {
                auto *box = qobject_cast<QMessageBox *>(dlg);
                QVERIFY(box);
                for (QAbstractButton *b : box->buttons())
                    if (b->text() == QLatin1String(button)) {
                        b->click();
                        return;
                    }
                QFAIL("no such button");
            };
        };
        withDialog([&] { w.act("design.size.a4")->trigger(); }, QStringLiteral("slideSizeAsk"), answer("Keep Size"));
        QCOMPARE(ed->doc()->setup.sizeName, QStringLiteral("A4 Paper"));
        QCOMPARE(ed->doc()->pages[0]->items[0]->rect, title);
        QTRY_VERIFY(w.act("design.size.a4")->isChecked());
        ed->undo();
        QCOMPARE(ed->doc()->pageSize(), QSizeF(960, 540));
        withDialog([&] { w.act("design.size.letter")->trigger(); }, QStringLiteral("slideSizeAsk"), answer("Scale to Fit"));
        QCOMPARE(ed->doc()->setup.sizeName, QStringLiteral("Letter Paper"));
        QVERIFY(ed->doc()->pages[0]->items[0]->rect != title);
        ed->undo();
        QCOMPARE(ed->doc()->pageSize(), QSizeF(960, 540));
        QCOMPARE(ed->doc()->pages[0]->items[0]->rect, title);
        // Cancel changes nothing.
        withDialog([&] { w.act("design.size.banner")->trigger(); }, QStringLiteral("slideSizeAsk"), [](QDialog *dlg) { dlg->reject(); }, false);
        QCOMPARE(ed->doc()->pageSize(), QSizeF(960, 540));
    }

    // The custom size box: width and height, and the orientation turns the size.
    void customSlideSizeBox()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        // The box that asks whether to scale comes up when the size box is answered: it is answered as it appears.
        QTimer ask;
        ask.setInterval(20);
        connect(&ask, &QTimer::timeout, [&] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                for (QAbstractButton *b : box->buttons())
                    if (b->text() == QLatin1String("Keep Size")) b->click();
            }
        });
        ask.start();
        withDialog([&] { w.act("design.sizeCustom")->trigger(); }, QStringLiteral("slideSizeDialog"), [&](QDialog *dlg) {
            auto *width = dlg->findChild<QDoubleSpinBox *>(QStringLiteral("slideWidth"));
            auto *height = dlg->findChild<QDoubleSpinBox *>(QStringLiteral("slideHeight"));
            auto *portrait = dlg->findChild<QRadioButton *>(QStringLiteral("slidePortrait"));
            QVERIFY(width && height && portrait);
            width->setValue(8);
            height->setValue(5);
            portrait->setChecked(true);
            QCOMPARE(width->value(), 5.0);   // portrait: the shorter side across
            QCOMPARE(height->value(), 8.0);
        });
        QTRY_COMPARE(ed->doc()->pageSize(), QSizeF(360, 576));
        QCOMPARE(ed->doc()->setup.sizeName, QStringLiteral("Custom"));
    }

    // Portrait, paper, and banner decks save in .pptx and .odp and open as they were, and their slides print, export, and draw in their own shape.
    void everySizeSavesAndDrawsInItsShape()
    {
        const QVector<QSizeF> sizes = {QSizeF(540, 720), QSizeF(792, 612), QSizeF(842, 595), QSizeF(720, 450), QSizeF(576, 72), QSizeF(612, 792), QSizeF(1000, 400)};
        for (const QSizeF &size : sizes) {
            const QString what = QStringLiteral("%1 x %2").arg(size.width()).arg(size.height());
            auto d = deck::newPresentation();
            deck::setSlideSize(*d, size);
            QString err;
            auto viaPptx = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
            QVERIFY2(viaPptx, qPrintable(err));
            QVERIFY2(std::abs(viaPptx->pageSize().width() - size.width()) < 0.1 && std::abs(viaPptx->pageSize().height() - size.height()) < 0.1, qPrintable(what));
            QCOMPARE(viaPptx->setup.sizeName, d->setup.sizeName);
            auto viaOdp = deck::importOdp(deck::exportOdp(*d, false, &err), &err);
            QVERIFY2(viaOdp, qPrintable(err));
            QVERIFY2(std::abs(viaOdp->pageSize().width() - size.width()) < 0.1 && std::abs(viaOdp->pageSize().height() - size.height()) < 0.1, qPrintable(what));
            // A slide's picture is the slide's shape.
            const QImage img = deck::slideImage(*d, nullptr, 0, 400);
            QCOMPARE(img.width(), 400);
            QVERIFY2(std::abs(img.height() - 400 * size.height() / size.width()) <= 1, qPrintable(what));
            // The PDF's pages are the slide's size.
            QTemporaryDir dir;
            QVERIFY(dir.isValid());
            const QString path = dir.filePath(QStringLiteral("size.pdf"));
            QVERIFY2(deck::exportPdf(*d, path, deck::OutputOptions(), &err), qPrintable(err));
            const QByteArray pdf = fileBytes(path);
            const QRegularExpressionMatch m = QRegularExpression(QStringLiteral("/MediaBox \\[0 0 ([0-9.]+) ([0-9.]+)\\]")).match(QString::fromLatin1(pdf));
            QVERIFY2(m.hasMatch(), qPrintable(what));
            QVERIFY2(std::abs(m.captured(1).toDouble() - size.width()) < 0.5 && std::abs(m.captured(2).toDouble() - size.height()) < 0.5, qPrintable(what));
            // The show draws it in its shape, in a window of another one.
            LayoutCache cache;
            SlideShow show(d.get(), &cache, 0);
            show.resize(800, 600);
            const QRectF at = slideRectIn(d->pageSize(), QSizeF(800, 600));
            QVERIFY2(std::abs(at.width() / at.height() - size.width() / size.height()) < 0.02, qPrintable(what));
            QVERIFY(QRectF(0, 0, 800, 600).contains(at.adjusted(1, 1, -1, -1)));
        }
    }

    // ---------------- links ----------------

    void linksAreParsed()
    {
        using office::LinkKind;
        QCOMPARE(office::parseLink(QStringLiteral("#next")).kind, LinkKind::Next);
        QCOMPARE(office::parseLink(QStringLiteral("#previous")).kind, LinkKind::Previous);
        QCOMPARE(office::parseLink(QStringLiteral("#first")).kind, LinkKind::First);
        QCOMPARE(office::parseLink(QStringLiteral("#last")).kind, LinkKind::Last);
        QCOMPARE(office::parseLink(QStringLiteral("#end")).kind, LinkKind::EndShow);
        QCOMPARE(office::parseLink(QStringLiteral("#page7")).slide, 7);
        QCOMPARE(office::parseLink(QStringLiteral("#page0")).kind, LinkKind::None);
        QCOMPARE(office::parseLink(QStringLiteral("https://example.com/")).kind, LinkKind::Web);
        QCOMPARE(office::parseLink(QStringLiteral("mailto:a@example.com")).kind, LinkKind::Mail);
        QCOMPARE(office::parseLink(QStringLiteral("C:\\docs\\a.pdf")).kind, LinkKind::File);
        QCOMPARE(office::parseLink(QStringLiteral("/home/a/b.pdf")).kind, LinkKind::File);
        QCOMPARE(office::parseLink(QStringLiteral("javascript:alert(1)")).kind, LinkKind::None);
    }

    // A file on another computer: two slashes or backslashes, or a file address with a host.
    void remoteFilesAreRecognized()
    {
        for (const char *link : {"\\\\server\\share\\a.pdf", "//server/share/a.pdf", "file://server/share/a.pdf", "file:////server/share/a.pdf", "FILE://Server/a.pdf", "file:\\\\server\\a.pdf"})
            QVERIFY2(office::isRemoteFile(QString::fromLatin1(link)), link);
        for (const char *link : {"file:///home/a/b.pdf", "file://localhost/home/a.pdf", "C:\\docs\\a.pdf", "/home/a/b.pdf", "https://example.com/", "#next", "doc.pdf"})
            QVERIFY2(!office::isRemoteFile(QString::fromLatin1(link)), link);
    }

    // Links on shapes and on words, of every kind, are saved in .pptx and read back as they were; jumps are written the way presentation files write them.
    void everyKindOfLinkRoundTripsInPptx()
    {
        auto d = showDeck(4);
        Page &slide = *d->pages[1];
        const QStringList links = {QStringLiteral("#next"), QStringLiteral("#previous"), QStringLiteral("#first"), QStringLiteral("#last"), QStringLiteral("#page3"), QStringLiteral("#end"),
                                   QStringLiteral("https://example.com/a"), QStringLiteral("C:\\docs\\a.pdf"), QStringLiteral("\\\\server\\share\\b.pdf")};
        for (int i = 0; i < links.size(); ++i) linkShape(slide, QRectF(10 + 100 * (i % 9), 300, 90, 60), links[i]);
        // Words with a link.
        auto *title = static_cast<TextItem *>(slide.items[0].get());
        QTextCursor c(d->storyDoc(title->storyId));
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setAnchor(true);
        f.setAnchorHref(QStringLiteral("#page4"));
        c.mergeCharFormat(f);
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        const QString xml = part(e, QStringLiteral("/ppt/slides/slide2.xml"));
        QVERIFY(xml.contains(QStringLiteral("ppaction://hlinkshowjump?jump=nextslide")));
        QVERIFY(xml.contains(QStringLiteral("ppaction://hlinkshowjump?jump=previousslide")));
        QVERIFY(xml.contains(QStringLiteral("ppaction://hlinkshowjump?jump=firstslide")));
        QVERIFY(xml.contains(QStringLiteral("ppaction://hlinkshowjump?jump=lastslide")));
        QVERIFY(xml.contains(QStringLiteral("ppaction://hlinkshowjump?jump=endshow")));
        QVERIFY(xml.contains(QStringLiteral("ppaction://hlinksldjump")));
        QVERIFY(xml.contains(QStringLiteral("ppaction://hlinkfile")));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QStringList got;
        for (const auto &it : back->pages[1]->items)
            if (!it->hyperlink.isEmpty()) got << it->hyperlink;
        QCOMPARE(got, links);
        auto *backTitle = static_cast<TextItem *>(back->pages[1]->items[0].get());
        QTextCursor bc(back->storyDoc(backTitle->storyId));
        bc.setPosition(1);
        QCOMPARE(bc.charFormat().anchorHref(), QStringLiteral("#page4"));
        // Saved again, the same.
        auto twice = deck::importPresentation(deck::exportPresentation(*back, &err), &err);
        QVERIFY2(twice, qPrintable(err));
        QStringList again;
        for (const auto &it : twice->pages[1]->items)
            if (!it->hyperlink.isEmpty()) again << it->hyperlink;
        QCOMPARE(again, links);
    }

    // A file that is opened does nothing with its links: no address is opened, no file looked for.
    void openingADeckFollowsNoLink()
    {
        auto d = showDeck(2);
        linkShape(*d->pages[0], QRectF(10, 300, 90, 60), QStringLiteral("https://example.com/a"));
        linkShape(*d->pages[0], QRectF(110, 300, 90, 60), QStringLiteral("\\\\server\\share\\b.pdf"));
        LinkCatcher catcher;
        QDesktopServices::setUrlHandler(QStringLiteral("https"), &catcher, "open");
        QDesktopServices::setUrlHandler(QStringLiteral("file"), &catcher, "open");
        const auto restore = qScopeGuard([] {
            QDesktopServices::unsetUrlHandler(QStringLiteral("https"));
            QDesktopServices::unsetUrlHandler(QStringLiteral("file"));
        });
        QString err;
        auto back = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
        QVERIFY2(back, qPrintable(err));
        LayoutCache cache;
        SlideShow show(back.get(), &cache, 0);
        show.resize(960, 540);
        QVERIFY(catcher.urls.isEmpty());
    }

    // In the show a click on a link goes to the slide it names, or ends the show; the pointer is a hand over it.
    void showFollowsJumps()
    {
        auto d = showDeck(5);
        Page &slide = *d->pages[2];
        const QStringList links = {QStringLiteral("#next"), QStringLiteral("#previous"), QStringLiteral("#first"), QStringLiteral("#last"), QStringLiteral("#page2"), QStringLiteral("#end")};
        for (int i = 0; i < links.size(); ++i) linkShape(slide, QRectF(10 + 110 * i, 400, 100, 100), links[i]);
        LayoutCache cache;
        QPointer<SlideShow> show = new SlideShow(d.get(), &cache, 0);
        show->resize(960, 540);
        show->show();
        QSignalSpy ended(show.data(), &SlideShow::ended);
        const int expected[] = {3, 1, 0, 4, 1};
        for (int i = 0; i < 5; ++i) {
            show->goTo(2);
            QCOMPARE(show->currentSlide(), 2);
            QTest::mouseMove(show.data(), QPoint(10 + 110 * i + 50, 450));
            QCOMPARE(show->cursor().shape(), Qt::PointingHandCursor);
            QTest::mouseClick(show.data(), Qt::LeftButton, Qt::NoModifier, QPoint(10 + 110 * i + 50, 450));
            QCOMPARE(show->currentSlide(), expected[i]);
        }
        show->goTo(2);
        QTest::mouseMove(show.data(), QPoint(300, 20));
        QVERIFY(show->cursor().shape() != Qt::PointingHandCursor);
        QTest::mouseClick(show.data(), Qt::LeftButton, Qt::NoModifier, QPoint(10 + 110 * 5 + 50, 450));
        QCOMPARE(ended.count(), 1);
        delete show.data();
    }

    // Words with a link are links in the show too.
    void showFollowsALinkOnWords()
    {
        auto d = showDeck(3);
        auto *title = static_cast<TextItem *>(d->pages[0]->items[0].get());
        QTextCursor c(d->storyDoc(title->storyId));
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setAnchor(true);
        f.setAnchorHref(QStringLiteral("#page3"));
        c.mergeCharFormat(f);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        const QVector<deck::LinkArea> areas = deck::linkAreas(*d, cache, 0);
        QVERIFY(!areas.isEmpty());
        const QPoint at = areas.first().rect.center().toPoint();
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, at);
        QCOMPARE(show.currentSlide(), 2);
    }

    // A web, mail, or file link asks first; nothing opens unless it is said yes to. A file on another computer is asked about as one.
    void showAsksBeforeOpeningAnything()
    {
        auto d = showDeck(2);
        linkShape(*d->pages[0], QRectF(10, 300, 90, 60), QStringLiteral("https://example.com/a"));
        linkShape(*d->pages[0], QRectF(110, 300, 90, 60), QStringLiteral("\\\\server\\share\\b.pdf"));
        linkShape(*d->pages[0], QRectF(210, 300, 90, 60), QStringLiteral("/home/user/a.pdf"));
        LinkCatcher catcher;
        QDesktopServices::setUrlHandler(QStringLiteral("https"), &catcher, "open");
        QDesktopServices::setUrlHandler(QStringLiteral("file"), &catcher, "open");
        const auto restore = qScopeGuard([] {
            QDesktopServices::unsetUrlHandler(QStringLiteral("https"));
            QDesktopServices::unsetUrlHandler(QStringLiteral("file"));
        });
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        QStringList asked;
        QList<bool> remote;
        bool yes = false;
        show.setLinkPrompt([&](const QString &address, bool isRemote) {
            asked << address;
            remote << isRemote;
            return yes;
        });
        for (int i = 0; i < 3; ++i) QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(10 + 100 * i + 50, 330));
        QCOMPARE(asked, (QStringList{QStringLiteral("https://example.com/a"), QStringLiteral("\\\\server\\share\\b.pdf"), QStringLiteral("/home/user/a.pdf")}));
        QCOMPARE(remote, (QList<bool>{false, true, false}));
        QVERIFY2(catcher.urls.isEmpty(), "nothing opens when the answer is no");
        yes = true;
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(60, 330));
        QCOMPARE(catcher.urls, QStringList{QStringLiteral("https://example.com/a")});
        // A path on this computer opens as a file; the other computer's too, only when it is said yes to.
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(260, 330));
        QCOMPARE(catcher.urls.size(), 2);
        QVERIFY(catcher.urls.last().startsWith(QStringLiteral("file:")));
    }

    // Hovering a web or file link shows its address.
    void hoverShowsTheAddress()
    {
        auto d = showDeck(2);
        linkShape(*d->pages[0], QRectF(10, 300, 90, 60), QStringLiteral("https://example.com/hover"));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QTest::mouseMove(&show, QPoint(50, 330));
        QCOMPARE(show.cursor().shape(), Qt::PointingHandCursor);
        QTRY_COMPARE(QToolTip::text(), QStringLiteral("https://example.com/hover"));
    }

    // An action button starts with the link it is for; the others have none.
    void actionButtonsStartWithTheirLinks()
    {
        const QHash<QString, QString> expected = {{"actionButtonHome", "#first"}, {"actionButtonForwardNext", "#next"}, {"actionButtonBackPrevious", "#previous"},
                                                  {"actionButtonEnd", "#last"}, {"actionButtonBeginning", "#first"}, {"actionButtonHelp", ""}};
        for (auto it = expected.begin(); it != expected.end(); ++it) {
            const ShapeDef *def = shapeDef(it.key());
            QVERIFY2(def, qPrintable(it.key()));
            QCOMPARE(def->link, it.value());
            QVERIFY(def->slideAction);
            QVERIFY(!shapePath(it.key(), QSizeF(100, 60)).isEmpty());
        }
        // The shapes gallery of the window offers them.
        DeckWindow w;
        bool found = false;
        for (GalleryButton *b : w.findChildren<GalleryButton *>())
            for (const GalleryItem &g : b->items()) found = found || g.id == QLatin1String("actionButtonForwardNext");
        QVERIFY(found);
    }

    // The button is saved as the shape it is, with its jump.
    void anActionButtonSurvivesPptx()
    {
        auto d = showDeck(3);
        auto b = std::make_shared<ShapeItem>();
        b->shape = QStringLiteral("actionButtonForwardNext");
        b->rect = QRectF(800, 450, 100, 60);
        b->hyperlink = QStringLiteral("#next");
        d->pages[0]->items.push_back(b);
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("prst=\"actionButtonForwardNext\"")));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        auto *shape = static_cast<ShapeItem *>(back->pages[0]->items.back().get());
        QCOMPARE(shape->shape, QStringLiteral("actionButtonForwardNext"));
        QCOMPARE(shape->hyperlink, QStringLiteral("#next"));
    }

    // Insert > Link (Ctrl+K) sets a link on the selected shape, as one step that undoes.
    void insertLinkSetsALinkOnAShape()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        const QString id = linkShape(*ed->doc()->pages[0], QRectF(100, 300, 100, 60), QString())->id;
        auto link = [&] { return ed->doc()->item(id)->hyperlink; };
        ed->select(id);
        QCOMPARE(w.act("insert.link")->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_K));
        withDialog([&] { w.act("insert.link")->trigger(); }, QStringLiteral("linkDialog"), [](QDialog *dlg) {
            auto *kind = dlg->findChild<QComboBox *>(QStringLiteral("linkKind"));
            auto *place = dlg->findChild<QComboBox *>(QStringLiteral("linkPlace"));
            QVERIFY(kind && place);
            kind->setCurrentIndex(2);
            place->setCurrentIndex(place->findData(QStringLiteral("#last")));
        });
        QCOMPARE(link(), QStringLiteral("#last"));
        ed->undo();
        QCOMPARE(link(), QString());
        // The box shows the link now there, and can remove it.
        ed->doc()->item(id)->hyperlink = QStringLiteral("https://example.com/x");
        ed->select(id);
        withDialog([&] { w.act("insert.link")->trigger(); }, QStringLiteral("linkDialog"), [](QDialog *dlg) {
            QCOMPARE(dlg->findChild<QLineEdit *>(QStringLiteral("linkAddress"))->text(), QStringLiteral("https://example.com/x"));
            for (QPushButton *b : dlg->findChildren<QPushButton *>())
                if (b->text() == QLatin1String("Remove Link")) b->click();
        });
        QCOMPARE(link(), QString());
    }

    // A file link is typed, and never looked for (a path on another computer is not touched by the box).
    void aFileLinkIsNotLookedFor()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        auto shape = linkShape(*ed->doc()->pages[0], QRectF(100, 300, 100, 60), QString());
        ed->select(shape->id);
        withDialog([&] { w.act("insert.link")->trigger(); }, QStringLiteral("linkDialog"), [](QDialog *dlg) {
            dlg->findChild<QComboBox *>(QStringLiteral("linkKind"))->setCurrentIndex(3);
            dlg->findChild<QLineEdit *>(QStringLiteral("linkAddress"))->setText(QStringLiteral("\\\\no-such-server\\share\\a.pdf"));
        });
        QCOMPARE(shape->hyperlink, QStringLiteral("\\\\no-such-server\\share\\a.pdf"));
    }

    // The slides' PDF has a clickable box for each link: the web ones go to the web, the jumps to the pages; a file and the end of the show have none.
    void pdfHasBoxesForLinks()
    {
        auto d = showDeck(4);
        Page &slide = *d->pages[1];
        linkShape(slide, QRectF(10, 300, 100, 60), QStringLiteral("https://example.com/pdf"));
        linkShape(slide, QRectF(120, 300, 100, 60), QStringLiteral("#next"));
        linkShape(slide, QRectF(230, 300, 100, 60), QStringLiteral("#previous"));
        linkShape(slide, QRectF(340, 300, 100, 60), QStringLiteral("#first"));
        linkShape(slide, QRectF(450, 300, 100, 60), QStringLiteral("#last"));
        linkShape(slide, QRectF(560, 300, 100, 60), QStringLiteral("#page3"));
        linkShape(slide, QRectF(670, 300, 100, 60), QStringLiteral("#end"));
        linkShape(slide, QRectF(780, 300, 100, 60), QStringLiteral("\\\\server\\share\\a.pdf"));
        linkShape(slide, QRectF(10, 400, 100, 60), QStringLiteral("C:\\docs\\a.pdf"));
        linkShape(slide, QRectF(120, 400, 100, 60), QStringLiteral("file://server/share/a.pdf"));
        for (bool archive : {false, true}) {
            QTemporaryDir dir;
            QVERIFY(dir.isValid());
            deck::OutputOptions o;
            o.archive = archive;
            QString err;
            const QString path = dir.filePath(QStringLiteral("links.pdf"));
            QVERIFY2(deck::exportPdf(*d, path, o, &err), qPrintable(err));
            const QByteArray pdf = fileBytes(path);
            const QStringList links = pdfLinkDictionaries(pdf);
            QCOMPARE(links.size(), 6);
            int uris = 0, dests = 0;
            for (const QString &l : links) {
                uris += l.contains(QStringLiteral("/URI (https://example.com/pdf)"));
                dests += l.contains(QStringLiteral("/Dest ["));
                QVERIFY2(!l.contains(QStringLiteral("server")) && !l.contains(QStringLiteral("docs")), qPrintable(l));
            }
            QCOMPARE(uris, 1);
            QCOMPARE(dests, 5);
            // The box is where the shape is: 10 to 110 points across, 300 to 360 down, from the bottom of a 540-point page.
            QVERIFY2(links.first().contains(QStringLiteral("/Rect [10.00 180.00 110.00 240.00]")), qPrintable(links.first()));
        }
        // A deck with no links has none.
        auto plain = showDeck(2);
        QTemporaryDir dir;
        QString err;
        QVERIFY2(deck::exportPdf(*plain, dir.filePath(QStringLiteral("p.pdf")), deck::OutputOptions(), &err), qPrintable(err));
        QCOMPARE(pdfLinkDictionaries(fileBytes(dir.filePath(QStringLiteral("p.pdf")))).size(), 0);
    }

    // Words with a link are boxes in the PDF too.
    void pdfHasBoxesForLinkedWords()
    {
        auto d = showDeck(3);
        auto *title = static_cast<TextItem *>(d->pages[0]->items[0].get());
        QTextCursor c(d->storyDoc(title->storyId));
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setAnchor(true);
        f.setAnchorHref(QStringLiteral("#page3"));
        c.mergeCharFormat(f);
        QTemporaryDir dir;
        QString err;
        const QString path = dir.filePath(QStringLiteral("words.pdf"));
        QVERIFY2(deck::exportPdf(*d, path, deck::OutputOptions(), &err), qPrintable(err));
        const QStringList links = pdfLinkDictionaries(fileBytes(path));
        QCOMPARE(links.size(), 1);
        QVERIFY(links.first().contains(QStringLiteral("/Dest [")));
    }

    // ---------------- backgrounds ----------------

    static Fill gradientFill(Fill::GradType type)
    {
        Fill f;
        f.type = Fill::Gradient;
        f.gradType = type;
        f.angle = 45;
        f.stops = {GradientStop{0.0, ColorRef::rgb(QColor(0xC0, 0x20, 0x20)), 0}, GradientStop{0.5, ColorRef::rgb(QColor(0x20, 0xA0, 0x20)), 0}, GradientStop{1.0, ColorRef::rgb(QColor(0x20, 0x20, 0xC0)), 0}};
        return f;
    }

    // A gradient, picture, or pattern background for a slide, for all, or for the layout and master, in one step that undoes.
    void backgroundsOfAnyKindAreOneStep()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        const Fill gradient = gradientFill(Fill::Linear);
        w.setBackground(gradient, false);
        QCOMPARE(ed->doc()->pages[1]->background.type, Fill::Gradient);
        QCOMPARE(ed->doc()->pages[1]->background.stops.size(), 3);
        QCOMPARE(ed->doc()->pages[1]->background.angle, 45.0);
        QVERIFY(ed->doc()->pages[0]->background.isNone());
        ed->undo();
        QVERIFY(ed->doc()->pages[1]->background.isNone());
        Fill radial = gradientFill(Fill::Radial);
        w.setBackground(radial, true);
        QCOMPARE(ed->doc()->masters[0]->background.gradType, Fill::Radial);
        QVERIFY(ed->doc()->pages[1]->background.isNone());
        ed->undo();
        QCOMPARE(ed->doc()->masters[0]->background.type, Fill::Solid);
        Fill pattern;
        pattern.type = Fill::Pattern;
        pattern.pattern = 5;
        pattern.color = ColorRef::rgb(Qt::red);
        pattern.color2 = ColorRef::rgb(Qt::white);
        w.setBackground(pattern, false);
        QCOMPARE(ed->doc()->pages[1]->background.type, Fill::Pattern);
        ed->undo();
        QVERIFY(ed->doc()->pages[1]->background.isNone());
    }

    // The Format Background box offers the fills, and a gradient chosen in it is the slide's background.
    void formatBackgroundBoxSetsAGradient()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        withDialog([&] { w.act("design.formatBackground")->trigger(); }, QStringLiteral("backgroundDialog"), [](QDialog *dlg) {
            auto *editor = dlg->findChild<FillEditor *>(QStringLiteral("backgroundFill"));
            QVERIFY(editor);
            // Gradient fill: the editor's second radio after "none" and "solid".
            const QList<QRadioButton *> radios = editor->findChildren<QRadioButton *>();
            QVERIFY(radios.size() >= 5);
            radios[2]->click();
            QCOMPARE(editor->fills().size(), 1);
        });
        QCOMPARE(ed->doc()->pages[0]->background.type, Fill::Gradient);
        QVERIFY(ed->doc()->pages[0]->background.stops.size() >= 2 || !ed->doc()->pages[0]->background.color2.isNone());
        ed->undo();
        QVERIFY(ed->doc()->pages[0]->background.type != Fill::Gradient);
    }

    // The fills are drawn: a gradient background has the colors of its stops at its ends.
    void gradientBackgroundIsDrawn()
    {
        auto d = deck::newPresentation();
        Fill f = gradientFill(Fill::Linear);
        f.angle = 0;   // left to right
        deck::setSlideBackground(*d->pages[0], f);
        const QImage img = deck::slideImage(*d, nullptr, 0, 480);
        const QColor left = img.pixelColor(2, 250), right = img.pixelColor(img.width() - 3, 250);
        QVERIFY(left.red() > 150 && left.blue() < 90);
        QVERIFY(right.blue() > 150 && right.red() < 90);
    }

    // Gradient, picture, and pattern backgrounds of a slide, a layout, and a master are saved in .pptx and .odp and read back as they were.
    void backgroundsRoundTrip()
    {
        QImage png(8, 8, QImage::Format_RGB32);
        png.fill(QColor(0x40, 0x80, 0xC0));
        QByteArray bytes;
        QBuffer buf(&bytes);
        buf.open(QIODevice::WriteOnly);
        png.save(&buf, "PNG");
        for (int format = 0; format < 2; ++format) {
            auto d = deck::newPresentation();
            for (int i = 0; i < 3; ++i) deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
            deck::setSlideBackground(*d->pages[0], gradientFill(Fill::Linear));
            Fill picture;
            picture.type = Fill::Picture;
            picture.imageId = d->addImage(bytes, QStringLiteral("png"), QString());
            deck::setSlideBackground(*d->pages[1], picture);
            Fill pattern;
            pattern.type = Fill::Pattern;
            pattern.pattern = int(patternBrushes().indexOf(Qt::CrossPattern));
            QVERIFY(pattern.pattern >= 0);
            pattern.color = ColorRef::rgb(QColor(0x80, 0x10, 0x10));
            pattern.color2 = ColorRef::rgb(QColor(0xF0, 0xF0, 0xA0));
            deck::setSlideBackground(*d->pages[2], pattern);
            deck::layoutOfType(*d, QStringLiteral("obj"))->background = gradientFill(Fill::Radial);
            d->masters[0]->background = gradientFill(Fill::Linear);
            d->masters[0]->background.angle = 90;
            QString err;
            std::unique_ptr<Document> back;
            if (format == 0) back = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
            else back = deck::importOdp(deck::exportOdp(*d, false, &err), &err);
            QVERIFY2(back, qPrintable(err));
            const QString what = format == 0 ? QStringLiteral("pptx") : QStringLiteral("odp");
            const Fill g = back->pages[0]->background;
            QVERIFY2(g.type == Fill::Gradient, qPrintable(what));
            QVERIFY2(g.gradType == Fill::Linear, qPrintable(what));
            QVERIFY2(g.stops.size() == 3, qPrintable(what));
            QVERIFY2(g.stops[1].color.rgbValue() == QColor(0x20, 0xA0, 0x20), qPrintable(what));
            QVERIFY2(std::abs(g.stops[1].pos - 0.5) < 0.02, qPrintable(what));
            QVERIFY2(std::abs(g.angle - 45) < 1.5, qPrintable(what));
            const Fill p = back->pages[1]->background;
            QVERIFY2(p.type == Fill::Picture && !p.imageId.isEmpty(), qPrintable(what));
            QVERIFY2(!back->image(p.imageId).isNull() && back->image(p.imageId).width() == 8, qPrintable(what));
            const Fill pt = back->pages[2]->background;
            QVERIFY2(pt.type == Fill::Pattern && pt.pattern == pattern.pattern, qPrintable(what));
            QVERIFY2(pt.color.rgbValue() == QColor(0x80, 0x10, 0x10), qPrintable(what));
            // The layout's and the master's own show on a slide that has none: the same colors in both files. (An .odp has no layouts of its own to keep it in: its slides name a
            // master page that has it.)
            if (format == 0) {
                const MasterPage *layout = deck::layoutOfType(*back, QStringLiteral("obj"));
                QVERIFY2(layout && layout->background.type == Fill::Gradient && layout->background.gradType == Fill::Radial, qPrintable(what));
                QVERIFY2(back->masters[0]->background.type == Fill::Gradient, qPrintable(what));
            }
            const QImage was = deck::slideImage(*d, nullptr, 3, 200), now = deck::slideImage(*back, nullptr, 3, 200);
            QVERIFY2(std::abs(now.height() - was.height()) <= 1 && now.width() == was.width(), qPrintable(what));
            for (const QPoint at : {QPoint(3, 3), QPoint(100, 56), QPoint(196, 106)}) {
                const QColor a = was.pixelColor(at), b = now.pixelColor(at);
                QVERIFY2(std::abs(a.red() - b.red()) < 40 && std::abs(a.green() - b.green()) < 40 && std::abs(a.blue() - b.blue()) < 40, qPrintable(what + QStringLiteral(" at %1,%2: %3 and %4").arg(at.x()).arg(at.y()).arg(a.name(), b.name())));
            }
        }
    }
};

JP_TEST_CLASS(DeckSizeLinkTests)
#include "tst_deck_sizelinks.moc"
