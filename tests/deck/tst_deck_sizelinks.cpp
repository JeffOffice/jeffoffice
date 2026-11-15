// JeffDeck's tests of slide sizes, links and action buttons, and gradient and picture backgrounds.
#include "deckhelpers.h"
#include "../withdialog.h"
#include "core/linkareas.h"
#include "core/remotepath.h"
#include "app/slidesorter.h"
#include "core/shapetypes.h"
#include "io/presentationfile.h"
#include "office/hyperlink.h"

#include <QDesktopServices>
#include <QMessageBox>

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

    // The notes page has an orientation of its own, kept in .pptx (notesSz) and .odp (the notes page's layout).
    void notesOrientationRoundTrips()
    {
        auto d = deck::newPresentation();
        QVERIFY(deck::notesArePortrait(*d));
        deck::setNotesPortrait(*d, false);
        QVERIFY(!deck::notesArePortrait(*d));
        QString err;
        auto viaPptx = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
        QVERIFY2(viaPptx, qPrintable(err));
        QVERIFY(!deck::notesArePortrait(*viaPptx));
        auto viaOdp = deck::importOdp(deck::exportOdp(*d, false, &err), &err);
        QVERIFY2(viaOdp, qPrintable(err));
        QVERIFY(!deck::notesArePortrait(*viaOdp));
        deck::setNotesPortrait(*d, true);
        QVERIFY(deck::notesArePortrait(*d));
    }

    // The Custom Slide Size box has the notes' orientation; a change of it alone is one step that undoes.
    void customSlideSizeBoxSetsNotesOrientation()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        withDialog([&] { w.act("design.sizeCustom")->trigger(); }, QStringLiteral("slideSizeDialog"), [&](QDialog *dlg) {
            auto *landscape = dlg->findChild<QRadioButton *>(QStringLiteral("notesLandscape"));
            auto *portrait = dlg->findChild<QRadioButton *>(QStringLiteral("notesPortrait"));
            QVERIFY(landscape && portrait && portrait->isChecked());
            landscape->setChecked(true);
        });
        QTRY_VERIFY(!deck::notesArePortrait(*ed->doc()));
        QCOMPARE(ed->doc()->pageSize(), QSizeF(960, 540));
        ed->undo();
        QVERIFY(deck::notesArePortrait(*ed->doc()));
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

    // A file that is opened does nothing with its links: shown, pointed at, and drawn, no address is opened, none is asked about, and no file is looked at.
    void openingADeckFollowsNoLink()
    {
        auto d = showDeck(2);
        linkShape(*d->pages[0], QRectF(10, 300, 90, 60), QStringLiteral("https://example.com/a"));
        linkShape(*d->pages[0], QRectF(110, 300, 90, 60), QStringLiteral("\\\\server\\share\\b.pdf"));
        linkShape(*d->pages[0], QRectF(210, 300, 90, 60), QStringLiteral("/usr/bin/env"));
        linkShape(*d->pages[0], QRectF(310, 300, 90, 60), QStringLiteral("#end"));
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
        const int looked = office::linkFileLooks();
        int opened = 0, asked = 0, said = 0;
        LayoutCache cache;
        SlideShow show(back.get(), &cache, 0);
        show.setLinkOpener([&](const QUrl &) { ++opened; });
        show.setLinkPrompt([&](const QString &, bool) {
            ++asked;
            return false;
        });
        show.setLinkNotice([&](const QString &) { ++said; });
        show.resize(960, 540);
        show.show();
        for (int i = 0; i < 4; ++i) QTest::mouseMove(&show, QPoint(10 + 100 * i + 50, 330));
        QVERIFY(!show.grab().isNull());
        QCOMPARE(opened, 0);
        QCOMPARE(asked, 0);
        QCOMPARE(said, 0);
        QCOMPARE(office::linkFileLooks(), looked);
        QVERIFY(catcher.urls.isEmpty());
        QCOMPARE(show.currentSlide(), 0);
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
        const QVector<deck::LinkArea> areas = deck::linkAreas(*d, cache, 0, RenderOptions());
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
        const int looked = office::linkFileLooks();
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(60, 330));
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(160, 330));
        QCOMPARE(office::linkFileLooks(), looked);   // (the file on another computer was not looked at, nor was an address)
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(260, 330));
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

    // ---------------- links to the last slide viewed, a custom show, and another presentation ----------------

    // The strings that stand for these links, and the files that hold them: saved as the file has them, and read back as they were.
    void newKindsOfLinkAreParsedAndRoundTrip()
    {
        QCOMPARE(office::parseLink(QStringLiteral("#lastviewed")).kind, office::LinkKind::LastViewed);
        const office::LinkTarget show = office::parseLink(office::customShowLink(7, false));
        QVERIFY(show.kind == office::LinkKind::CustomShow && show.slide == 7 && !show.back);
        QVERIFY(office::parseLink(office::customShowLink(7, true)).back);
        const office::LinkTarget other = office::parseLink(office::presentationLink(QStringLiteral("/home/user/Other Deck.pptx")));
        QVERIFY(other.kind == office::LinkKind::Deck && other.address == QStringLiteral("/home/user/Other Deck.pptx"));
        QCOMPARE(office::parseLink(QStringLiteral("#show:x")).kind, office::LinkKind::None);
        QCOMPARE(office::parseLink(QStringLiteral("#deck:")).kind, office::LinkKind::None);
        auto d = showDeck(3);
        const QStringList links = {QStringLiteral("#lastviewed"), office::customShowLink(4, true), office::customShowLink(5, false), office::presentationLink(QStringLiteral("/home/user/Other.pptx"))};
        for (int i = 0; i < links.size(); ++i) {
            auto s = linkShape(*d->pages[0], QRectF(10 + 100 * i, 400, 90, 60), links[i]);
            s->hoverLink = links[(i + 1) % links.size()];
        }
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        const QString xml = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(xml.contains(QStringLiteral("ppaction://hlinkshowjump?jump=lastslideviewed")));
        QVERIFY(xml.contains(QStringLiteral("ppaction://customshowjump?id=4&amp;return=true")));
        QVERIFY(xml.contains(QStringLiteral("ppaction://customshowjump?id=5\"")));
        QVERIFY(xml.contains(QStringLiteral("ppaction://hlinkpres")));
        QVERIFY(xml.contains(QStringLiteral("a:hlinkHover")));
        for (int format = 0; format < 2; ++format) {
            auto back = format == 0 ? deck::importPresentation(bytes, &err) : deck::importOdp(deck::exportOdp(*d, false, &err), &err);
            QVERIFY2(back, qPrintable(err));
            const QString what = format == 0 ? QStringLiteral("pptx") : QStringLiteral("odp");
            const ItemList &items = back->pages[0]->items;
            QVector<const Item *> shapes;
            for (const ItemPtr &it : items)
                if (it->type() == ItemType::Shape && !it->hyperlink.isEmpty()) shapes << it.get();
            QVERIFY2(shapes.size() == 4, qPrintable(what));
            for (int i = 0; i < 4; ++i) {
                QVERIFY2(shapes[i]->hyperlink == links[i], qPrintable(what + QStringLiteral(" ") + shapes[i]->hyperlink));
                QVERIFY2(shapes[i]->hoverLink == links[(i + 1) % 4], qPrintable(what + QStringLiteral(" %1: [%2]").arg(i).arg(shapes[i]->hoverLink)));
            }
        }
    }

    // A link to the last slide viewed goes back to it, and a second click goes forward again.
    void showGoesToTheLastSlideViewed()
    {
        auto d = showDeck(5);
        linkShape(*d->pages[3], QRectF(10, 400, 100, 100), QStringLiteral("#lastviewed"));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        show.goTo(1);
        show.goTo(3);
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(60, 450));
        QCOMPARE(show.currentSlide(), 1);
        show.goTo(3);
        show.goTo(4);
        show.goTo(3);
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(60, 450));
        QCOMPARE(show.currentSlide(), 4);
    }

    // A link to a custom show plays that show's slides; with "and return" the show comes back to the slide it left when the custom show ends, else the show ends there.
    void showPlaysACustomShowAndReturns()
    {
        for (const bool back : {false, true}) {
            auto d = showDeck(6);
            deck::CustomShow cs;
            cs.id = 7;
            cs.name = QStringLiteral("Extra");
            cs.slides = {d->pages[4]->id, d->pages[5]->id};
            deck::setCustomShows(*d, {cs});
            linkShape(*d->pages[1], QRectF(10, 400, 100, 100), office::customShowLink(7, back));
            LayoutCache cache;
            SlideShow show(d.get(), &cache, 0);
            show.resize(960, 540);
            show.show();
            show.goTo(1);
            QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(60, 450));
            QCOMPARE(show.currentSlide(), 4);
            show.next();
            QCOMPARE(show.currentSlide(), 5);
            show.previous();
            QCOMPARE(show.currentSlide(), 4);
            show.next();
            show.next();
            if (back) {
                QCOMPARE(show.currentSlide(), 1);
                show.next();
                QCOMPARE(show.currentSlide(), 2);   // (the show goes on from there)
            } else {
                QCOMPARE(show.currentSlide(), -1);   // the end of the custom show is the end
            }
        }
        // A show that is not there is no jump.
        auto d = showDeck(3);
        linkShape(*d->pages[0], QRectF(10, 400, 100, 100), office::customShowLink(99, true));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(60, 450));
        QCOMPARE(show.currentSlide(), 0);
    }

    // A link to another presentation asks first (as a file does), plays it as a show inside this one, and comes back at its end. A program is never run, and a file on another
    // computer is asked about as one.
    void showPlaysAnotherPresentationAsANestedShow()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        auto other = showDeck(2);
        const QString path = dir.filePath(QStringLiteral("other.pptx"));
        QString err;
        QVERIFY2(deck::saveAnyPresentationFile(*other, path, &err), qPrintable(err));
        auto d = showDeck(3);
        linkShape(*d->pages[0], QRectF(10, 400, 100, 100), office::presentationLink(path));
        linkShape(*d->pages[0], QRectF(120, 400, 100, 100), office::presentationLink(dir.filePath(QStringLiteral("run.exe"))));
        linkShape(*d->pages[0], QRectF(230, 400, 100, 100), office::presentationLink(QStringLiteral("\\\\server\\share\\far.pptx")));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QStringList asked, said;
        QList<bool> remote;
        bool yes = false;
        show.setLinkPrompt([&](const QString &address, bool isRemote) {
            asked << address;
            remote << isRemote;
            return yes;
        });
        show.setLinkNotice([&](const QString &text) { said << text; });
        // No: nothing opens.
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(60, 450));
        QCOMPARE(asked, QStringList{path});
        QVERIFY(!show.nestedShow());
        // A program is refused with a message and never asked about.
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(170, 450));
        QCOMPARE(asked.size(), 1);
        QCOMPARE(said.size(), 1);
        // A file on another computer is asked about as one, never looked for until the answer is yes (and here it does not exist: nothing opens).
        const int looked = office::linkFileLooks();
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(280, 450));
        QCOMPARE(remote.last(), true);
        QCOMPARE(office::linkFileLooks(), looked);
        QVERIFY(!show.nestedShow());
        // Yes: it plays, its own slides, and at its end the show is back.
        yes = true;
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(60, 450));
        QPointer<SlideShow> nested = show.nestedShow();
        QVERIFY(nested);
        QCOMPARE(int(nested->document()->pages.size()), 2);
        QCOMPARE(nested->currentSlide(), 0);
        nested->next();
        QCOMPARE(nested->currentSlide(), 1);
        nested->next();   // the end screen
        nested->next();   // closes
        QTRY_VERIFY(!show.nestedShow());
        QCOMPARE(show.currentSlide(), 0);
    }

    // A mouse-over action is followed when the pointer comes onto the object, once, and not again until it has left.
    void showFollowsAMouseOverAction()
    {
        auto d = showDeck(4);
        auto s = linkShape(*d->pages[0], QRectF(10, 400, 100, 100), QString());
        s->hoverLink = QStringLiteral("#next");
        auto w = linkShape(*d->pages[1], QRectF(10, 400, 100, 100), QString());
        w->hoverLink = QStringLiteral("https://example.com/over");
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QStringList asked;
        show.setLinkPrompt([&](const QString &address, bool) {
            asked << address;
            return false;
        });
        QTest::mouseMove(&show, QPoint(500, 100));
        QCOMPARE(show.currentSlide(), 0);
        QTest::mouseMove(&show, QPoint(60, 450));
        QCOMPARE(show.currentSlide(), 1);
        QVERIFY(asked.isEmpty());
        // On slide 2 a web address asks, once for a visit.
        QTest::mouseMove(&show, QPoint(500, 100));
        QTest::mouseMove(&show, QPoint(60, 450));
        QTest::mouseMove(&show, QPoint(70, 460));
        QCOMPARE(asked, QStringList{QStringLiteral("https://example.com/over")});
        QTest::mouseMove(&show, QPoint(500, 100));
        QTest::mouseMove(&show, QPoint(60, 450));
        QCOMPARE(asked.size(), 2);
    }

    // Tab and Shift+Tab move through the links of the slide, bottom object first, and Enter follows the one chosen; with none chosen Enter goes on, as it did.
    void keysMoveThroughLinksAndEnterFollowsOne()
    {
        auto d = showDeck(5);
        linkShape(*d->pages[1], QRectF(10, 400, 100, 100), QStringLiteral("#page5"));
        linkShape(*d->pages[1], QRectF(120, 400, 100, 100), QStringLiteral("#first"));
        linkShape(*d->pages[1], QRectF(230, 400, 100, 100), QStringLiteral("#last"));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        show.goTo(1);
        QCOMPARE(show.focusedLink(), QString());
        QTest::keyClick(&show, Qt::Key_Tab);
        QCOMPARE(show.focusedLink(), QStringLiteral("#page5"));
        QTest::keyClick(&show, Qt::Key_Tab);
        QCOMPARE(show.focusedLink(), QStringLiteral("#first"));
        QTest::keyClick(&show, Qt::Key_Tab, Qt::ShiftModifier);
        QCOMPARE(show.focusedLink(), QStringLiteral("#page5"));
        QTest::keyClick(&show, Qt::Key_Tab, Qt::ShiftModifier);   // (round to the last)
        QCOMPARE(show.focusedLink(), QStringLiteral("#last"));
        QTest::keyClick(&show, Qt::Key_Return);
        QCOMPARE(show.currentSlide(), 4);
        // A new slide starts with none chosen, and Enter goes on.
        QCOMPARE(show.focusedLink(), QString());
        show.goTo(1);
        QTest::keyClick(&show, Qt::Key_Return);
        QCOMPARE(show.currentSlide(), 2);
        // Tab on a slide with no links changes nothing.
        QTest::keyClick(&show, Qt::Key_Tab);
        QCOMPARE(show.focusedLink(), QString());
        QCOMPARE(show.currentSlide(), 2);
    }

    // The keyboard follows a web link only after asking, as a click does, and a link on words is among them.
    void keyboardLinksAskToo()
    {
        auto d = showDeck(2);
        linkShape(*d->pages[0], QRectF(10, 400, 100, 100), QStringLiteral("https://example.com/k"));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QStringList asked;
        show.setLinkPrompt([&](const QString &address, bool) {
            asked << address;
            return false;
        });
        QTest::keyClick(&show, Qt::Key_Tab);
        QTest::keyClick(&show, Qt::Key_Return);
        QCOMPARE(asked, QStringList{QStringLiteral("https://example.com/k")});
        QCOMPARE(show.currentSlide(), 0);
    }

    // Insert > Link offers the last slide viewed, each custom show (with or without coming back), and another presentation.
    void insertLinkOffersTheNewKinds()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        deck::CustomShow cs;
        cs.id = 7;
        cs.name = QStringLiteral("Extra");
        cs.slides = {ed->doc()->pages[0]->id};
        deck::setCustomShows(*ed->doc(), {cs});
        const QString id = linkShape(*ed->doc()->pages[0], QRectF(100, 300, 100, 60), QString())->id;
        ed->select(id);
        auto choose = [&](const QString &data) {
            withDialog([&] { w.act("insert.link")->trigger(); }, QStringLiteral("linkDialog"), [&](QDialog *dlg) {
                dlg->findChild<QComboBox *>(QStringLiteral("linkKind"))->setCurrentIndex(2);
                auto *place = dlg->findChild<QComboBox *>(QStringLiteral("linkPlace"));
                QVERIFY(place->findData(data) >= 0);
                place->setCurrentIndex(place->findData(data));
            });
            return ed->doc()->item(id)->hyperlink;
        };
        QCOMPARE(choose(QStringLiteral("#lastviewed")), QStringLiteral("#lastviewed"));
        QCOMPARE(choose(office::customShowLink(7, true)), office::customShowLink(7, true));
        QCOMPARE(choose(office::customShowLink(7, false)), office::customShowLink(7, false));
        withDialog([&] { w.act("insert.link")->trigger(); }, QStringLiteral("linkDialog"), [&](QDialog *dlg) {
            dlg->findChild<QComboBox *>(QStringLiteral("linkKind"))->setCurrentIndex(4);
            dlg->findChild<QLineEdit *>(QStringLiteral("linkAddress"))->setText(QStringLiteral("/home/user/other.pptx"));
        });
        QCOMPARE(ed->doc()->item(id)->hyperlink, office::presentationLink(QStringLiteral("/home/user/other.pptx")));
    }

    // ---------------- Insert > Slide Number ----------------

    static const Item *numberPlaceholder(const ItemList &items)
    {
        for (const ItemPtr &it : items)
            if (deck::placeholderType(*it) == QLatin1String("sldNum")) return it.get();
        return nullptr;
    }

    // The slide number's place moves the placeholder of the master, every layout, and every slide, and it is saved in both formats and read back as the same place.
    void slideNumberPlaceMovesEveryPlaceholderAndRoundTrips()
    {
        auto d = showDeck(3);
        deck::FooterOptions o;
        o.slideNumber = true;
        deck::applyFooters(*d, o);
        QVERIFY(numberPlaceholder(d->pages[1]->items));
        QCOMPARE(deck::slideNumberPlace(*d).across, deck::NumberAcross::Right);
        QCOMPARE(deck::slideNumberPlace(*d).down, deck::NumberDown::Bottom);
        const double width = numberPlaceholder(d->pages[1]->items)->rect.width();
        deck::setSlideNumberPlace(*d, deck::SlideNumberPlace{deck::NumberAcross::Center, deck::NumberDown::Top});
        const QSizeF page = d->pageSize();
        for (const auto &m : d->masters)
            if (const Item *n = numberPlaceholder(m->items)) {
                QVERIFY(std::abs(n->rect.center().x() - page.width() / 2) < 0.5 && n->rect.top() < page.height() / 4);
                QCOMPARE(n->rect.width(), width);
            }
        for (const auto &p : d->pages) {
            const Item *n = numberPlaceholder(p->items);
            QVERIFY(n && std::abs(n->rect.center().x() - page.width() / 2) < 0.5 && n->rect.top() < page.height() / 4);
        }
        QCOMPARE(deck::slideNumberPlace(*d).across, deck::NumberAcross::Center);
        QCOMPARE(deck::slideNumberPlace(*d).down, deck::NumberDown::Top);
        for (int format = 0; format < 2; ++format) {
            QString err;
            auto back = format == 0 ? deck::importPresentation(deck::exportPresentation(*d, &err), &err) : deck::importOdp(deck::exportOdp(*d, false, &err), &err);
            QVERIFY2(back, qPrintable(err));
            QVERIFY2(deck::slideNumberPlace(*back) == (deck::SlideNumberPlace{deck::NumberAcross::Center, deck::NumberDown::Top}), format == 0 ? "pptx" : "odp");
            QVERIFY(numberPlaceholder(back->pages[1]->items));
        }
        deck::setSlideNumberPlace(*d, deck::SlideNumberPlace{deck::NumberAcross::Left, deck::NumberDown::Bottom});
        QCOMPARE(deck::slideNumberPlace(*d).across, deck::NumberAcross::Left);
        QVERIFY(numberPlaceholder(d->pages[0]->items)->rect.left() < page.width() / 4 && numberPlaceholder(d->pages[0]->items)->rect.top() > page.height() / 2);
    }

    // The Slide Number menu turns the number on and off, puts it at a place, and keeps it off the title slide; each is one step that undoes.
    void slideNumberMenuIsOneStepEach()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        Document *d = ed->doc();
        QVERIFY(!numberPlaceholder(d->pages[1]->items));
        w.act("insert.slideNumberOn")->trigger();
        QVERIFY(numberPlaceholder(d->pages[0]->items) && numberPlaceholder(d->pages[1]->items) && numberPlaceholder(d->pages[2]->items));
        QVERIFY(w.act("insert.slideNumberOn")->isChecked());
        ed->undo();
        QVERIFY(!numberPlaceholder(d->pages[1]->items));
        ed->redo();
        QVERIFY(numberPlaceholder(d->pages[1]->items));
        // A place.
        QVERIFY(w.act("insert.slideNumberPlace.bottomRight")->isChecked());
        w.act("insert.slideNumberPlace.topLeft")->trigger();
        QCOMPARE(deck::slideNumberPlace(*d).across, deck::NumberAcross::Left);
        QCOMPARE(deck::slideNumberPlace(*d).down, deck::NumberDown::Top);
        QVERIFY(w.act("insert.slideNumberPlace.topLeft")->isChecked() && !w.act("insert.slideNumberPlace.bottomRight")->isChecked());
        ed->undo();
        QCOMPARE(deck::slideNumberPlace(*d).across, deck::NumberAcross::Right);
        QCOMPARE(deck::slideNumberPlace(*d).down, deck::NumberDown::Bottom);
        // Not on the title slide.
        w.act("insert.slideNumberSkipTitle")->trigger();
        QVERIFY(!numberPlaceholder(d->pages[0]->items));
        QVERIFY(numberPlaceholder(d->pages[1]->items));
        QVERIFY(w.act("insert.slideNumberSkipTitle")->isChecked());
        ed->undo();
        QVERIFY(numberPlaceholder(d->pages[0]->items));
        // Off.
        w.act("insert.slideNumberOn")->trigger();
        QVERIFY(!numberPlaceholder(d->pages[1]->items));
    }

    void hoverShowsTheAddress()
    {
        auto d = showDeck(2);
        linkShape(*d->pages[0], QRectF(10, 300, 90, 60), QStringLiteral("https://example.com/hover"));
        linkShape(*d->pages[0], QRectF(110, 300, 90, 60), QStringLiteral("#next"));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QTest::mouseMove(&show, QPoint(50, 330));
        QCOMPARE(show.cursor().shape(), Qt::PointingHandCursor);
        QCOMPARE(show.tipAt(QPointF(50, 330)), QStringLiteral("https://example.com/hover"));
        QCOMPARE(show.tipAt(QPointF(150, 330)), QString());   // (a jump has no address to show)
        QCOMPARE(show.tipAt(QPointF(500, 100)), QString());
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
        const int looked = office::linkFileLooks();
        withDialog([&] { w.act("insert.link")->trigger(); }, QStringLiteral("linkDialog"), [](QDialog *dlg) {
            dlg->findChild<QComboBox *>(QStringLiteral("linkKind"))->setCurrentIndex(3);
            dlg->findChild<QLineEdit *>(QStringLiteral("linkAddress"))->setText(QStringLiteral("\\\\no-such-server\\share\\a.pdf"));
        });
        QCOMPARE(shape->hyperlink, QStringLiteral("\\\\no-such-server\\share\\a.pdf"));
        QCOMPARE(office::linkFileLooks(), looked);   // nothing was looked at
        // (and a program file's name is judged by its name alone)
        QVERIFY(office::isProgramFile(QStringLiteral("\\\\no-such-server\\share\\run.exe")));
        QCOMPARE(office::linkFileLooks(), looked);
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

    // A background nobody changed is saved as the file had it: a reference to the theme's styles stays one, a theme color keeps its changes, and a master with none has none. Only one
    // that was edited is written new, as a plain color.
    void anUntouchedBackgroundIsSavedAsItWas()
    {
        const QString bgRef = QStringLiteral("<p:bg><p:bgRef idx=\"1001\"><a:schemeClr val=\"bg2\"><a:lumMod val=\"95000\"/></a:schemeClr></p:bgRef></p:bg>");
        const QString bgPr = QStringLiteral("<p:bg><p:bgPr><a:solidFill><a:schemeClr val=\"accent2\"><a:lumMod val=\"40000\"/><a:lumOff val=\"60000\"/></a:schemeClr></a:solidFill><a:effectLst/></p:bgPr></p:bg>");
        Parts p = samplePptx();
        auto put = [&](const QString &part, const QString &bg) {
            QString xml = QString::fromUtf8(p.data[part].xml);
            xml.replace(QStringLiteral("<p:cSld>"), QStringLiteral("<p:cSld>") + bg);
            p.data[part].xml = xml.toUtf8();
        };
        put(QStringLiteral("/ppt/slides/slide1.xml"), bgRef);
        put(QStringLiteral("/ppt/slides/slide2.xml"), bgPr);
        QString master = QString::fromUtf8(p.data["/ppt/slideMasters/slideMaster1.xml"].xml);
        master.remove(QRegularExpression(QStringLiteral("<p:bg>.*?</p:bg>")));
        p.data["/ppt/slideMasters/slideMaster1.xml"].xml = master.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        auto backgrounds = [&](const Document &doc, QStringList *masterOnes = nullptr) -> QStringList {
            Exported e;
            QString why;
            if (!unpack(deck::exportPresentation(doc, &why), &e, &why)) return QStringList{why};
            auto in = [&](const QString &name) {
                QStringList out;
                const QString xml = part(e, name);
                for (auto it = QRegularExpression(QStringLiteral("<p:bg>.*?</p:bg>")).globalMatch(xml); it.hasNext();) out << it.next().captured(0);
                return out;
            };
            if (masterOnes) *masterOnes = in(QStringLiteral("/ppt/slideMasters/slideMaster1.xml"));
            return QStringList{in(QStringLiteral("/ppt/slides/slide1.xml")).join(QString()), in(QStringLiteral("/ppt/slides/slide2.xml")).join(QString())};
        };
        QStringList masterBg;
        const QStringList saved = backgrounds(*d, &masterBg);
        QCOMPARE(saved[0], bgRef);
        QCOMPARE(saved[1], bgPr);
        QVERIFY2(masterBg.isEmpty(), "a master with no background has none");
        // Saved and opened again, the same (the kept form is kept through the second save).
        auto second = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
        QVERIFY2(second, qPrintable(err));
        QStringList masterBg2;
        QCOMPARE(backgrounds(*second, &masterBg2), saved);
        QVERIFY(masterBg2.isEmpty());
        // One that is changed is written new, as the new color; the other stays.
        deck::setSlideBackground(*d->pages[0], QColor(0x11, 0x22, 0x33));
        const QStringList changed = backgrounds(*d);
        QVERIFY2(changed[0].contains(QStringLiteral("srgbClr val=\"112233\"")) && changed[0].contains(QStringLiteral("bgPr")), qPrintable(changed[0]));
        QCOMPARE(changed[1], bgPr);
    }

    // A program or a macro that a file has on a shape or on words is never followed, and is saved as it was: the action, what it names, and what is in it.
    void programAndMacroActionsAreKeptOnSave()
    {
        Parts p = samplePptx();
        int base = 0;
        for (const Rel &r : p.rels) base += r.source == QLatin1String("/ppt/slides/slide1.xml");
        const QString programClick = QStringLiteral("<a:hlinkClick r:id=\"rId%1\" action=\"ppaction://program\" tooltip=\"Runs it\"><a:snd r:embed=\"\" name=\"x\"/></a:hlinkClick>").arg(base + 1);
        const QString macroClick = QStringLiteral("<a:hlinkClick r:id=\"\" action=\"ppaction://macro?name=Run\"/>");
        const QString wordsClick = QStringLiteral("<a:hlinkClick r:id=\"rId%1\" action=\"ppaction://program\"/>").arg(base + 1);
        auto shape = [](int id, const QString &name, const QString &click, const QString &runLink = QString()) {
            return QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"%2\">%3</p:cNvPr><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr>").arg(id).arg(name, click) + xfrm(914400 * (id - 49), 914400, 600000, 600000)
                   + QStringLiteral("<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></p:spPr><p:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\">%1</a:rPr><a:t>words</a:t></a:r></a:p></p:txBody></p:sp>").arg(runLink);
        };
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        xml.replace(QStringLiteral("</p:spTree>"), shape(50, QStringLiteral("program"), programClick) + shape(51, QStringLiteral("macro"), macroClick) + shape(52, QStringLiteral("words"), QString(), wordsClick) + QStringLiteral("</p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        p.rels << Rel{QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("hyperlink"), QStringLiteral("C:\\tools\\run.exe"), true};
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        for (const auto &it : d->pages[0]->items) QVERIFY2(it->hyperlink.isEmpty(), "never a link");
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        const QString out = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QCOMPARE(out.count(QStringLiteral("ppaction://program")), 2);
        QVERIFY(out.contains(QStringLiteral("tooltip=\"Runs it\"")));
        QVERIFY(out.contains(QStringLiteral("<a:snd ")) && out.contains(QStringLiteral("name=\"x\"")) && !out.contains(QStringLiteral("<n1:")));
        QVERIFY(out.contains(QStringLiteral("ppaction://macro?name=Run")));
        // The address the program click names is a relationship of the saved part, and the click refers to it.
        const QVector<office::Relationship> rels = e.pkg.relationships(QStringLiteral("/ppt/slides/slide1.xml"));
        QString relId;
        for (const office::Relationship &r : rels)
            if (r.external && r.target == QLatin1String("C:\\tools\\run.exe")) relId = r.id;
        QVERIFY(!relId.isEmpty());
        QVERIFY(out.contains(QStringLiteral("<a:hlinkClick r:id=\"%1\" action=\"ppaction://program\"").arg(relId)) || out.contains(QStringLiteral("<a:hlinkClick r:id=\"%1\" tooltip=\"Runs it\" action=\"ppaction://program\"").arg(relId))
                || out.contains(QStringLiteral("r:id=\"%1\"").arg(relId)));
        // Opened again, still no link anywhere, and a second save still has them.
        auto back = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
        QVERIFY2(back, qPrintable(err));
        for (const auto &it : back->pages[0]->items) QVERIFY(it->hyperlink.isEmpty());
        Exported twice;
        QVERIFY2(unpack(deck::exportPresentation(*back, &err), &twice, &err), qPrintable(err));
        QCOMPARE(part(twice, QStringLiteral("/ppt/slides/slide1.xml")).count(QStringLiteral("ppaction://program")), 2);
    }

    // ---------------- what a link may be, and what is refused ----------------

    // Only a plain link, a jump, a file link, and a link to another presentation are links. A program, a macro, and any other action stay text; a relationship that is no hyperlink (a
    // picture, an embedded object) is no link either.
    void programsAndMacrosAreNotLinks()
    {
        Parts p = samplePptx();
        int base = 0;
        for (const Rel &r : p.rels) base += r.source == QLatin1String("/ppt/slides/slide1.xml");
        auto shape = [](int id, const QString &name, const QString &click, const QString &runLink = QString()) {
            return QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"%2\">%3</p:cNvPr><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr>").arg(id).arg(name, click) + xfrm(914400 * (id - 49), 914400, 600000, 600000)
                   + QStringLiteral("<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></p:spPr><p:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\">%1</a:rPr><a:t>words</a:t></a:r></a:p></p:txBody></p:sp>").arg(runLink);
        };
        auto click = [&](int rel, const QString &action) { return QStringLiteral("<a:hlinkClick r:id=\"rId%1\"%2/>").arg(rel).arg(action.isEmpty() ? QString() : QStringLiteral(" action=\"%1\"").arg(action)); };
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        QString shapes = shape(50, QStringLiteral("plain"), click(base + 1, QString())) + shape(51, QStringLiteral("program"), click(base + 2, QStringLiteral("ppaction://program")))
                         + shape(52, QStringLiteral("macro"), QStringLiteral("<a:hlinkClick r:id=\"\" action=\"ppaction://macro?name=Run\"/>")) + shape(53, QStringLiteral("image"), click(base + 3, QString()))
                         + shape(54, QStringLiteral("file"), click(base + 4, QStringLiteral("ppaction://hlinkfile"))) + shape(55, QStringLiteral("other"), click(base + 5, QStringLiteral("ppaction://hlinkpres")))
                         + shape(56, QStringLiteral("words"), QString(), click(base + 2, QStringLiteral("ppaction://program")));
        xml.replace(QStringLiteral("</p:spTree>"), shapes + QStringLiteral("</p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        p.rels << Rel{QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("hyperlink"), QStringLiteral("https://ok.example/"), true}
               << Rel{QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("hyperlink"), QStringLiteral("C:\\tools\\run.exe"), true}
               << Rel{QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("image"), QStringLiteral("http://x.example/y.png"), true}
               << Rel{QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("hyperlink"), QStringLiteral("C:\\docs\\a.pdf"), true}
               << Rel{QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("hyperlink"), QStringLiteral("other.pptx"), true};
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QHash<QString, QString> links;
        for (const auto &it : d->pages[0]->items) links.insert(it->name, it->hyperlink);
        QCOMPARE(links.value(QStringLiteral("plain")), QStringLiteral("https://ok.example/"));
        QCOMPARE(links.value(QStringLiteral("file")), QStringLiteral("C:\\docs\\a.pdf"));
        QCOMPARE(links.value(QStringLiteral("other")), QStringLiteral("#deck:other.pptx"));
        for (const char *none : {"program", "macro", "image"}) QVERIFY2(links.value(QString::fromLatin1(none)).isEmpty(), none);
        for (const auto &it : d->pages[0]->items)
            if (it->name == QLatin1String("words")) {
                const QString story = it->type() == ItemType::Text ? static_cast<TextItem *>(it.get())->storyId : it->type() == ItemType::Shape ? static_cast<ShapeItem *>(it.get())->storyId : QString();
                const QTextDocument *td = d->storyDoc(story);
                QVERIFY2(td, "the words are there");
                for (QTextBlock b = td->begin(); b.isValid(); b = b.next())
                    for (auto f = b.begin(); !f.atEnd(); ++f) QVERIFY(!f.fragment().charFormat().isAnchor());
            }
    }

    // A link to a program or a script is refused, with a message, and nothing is asked or opened; a file marked as one that runs is too; a plain file is asked about.
    void showRefusesLinksToPrograms()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString tool = dir.filePath(QStringLiteral("tool")), doc = dir.filePath(QStringLiteral("doc.pdf"));
        for (const QString &path : {tool, doc}) {
            QFile f(path);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("x");
        }
        QVERIFY(QFile::setPermissions(tool, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        QVERIFY(QFile::setPermissions(doc, QFile::ReadOwner | QFile::WriteOwner));
        QStringList programs = {QStringLiteral("C:\\tools\\Setup.EXE"), QStringLiteral("/home/u/run.sh"), QStringLiteral("\\\\server\\share\\x.bat"), QStringLiteral("file:///tmp/a.ps1"),
                                QStringLiteral("C:\\x\\a.JAR"), QStringLiteral("/a/b.command"), QStringLiteral("C:\\a\\b.exe.")};
#ifndef Q_OS_WIN
        programs << tool;   // (no ending: it is marked as one that runs)
#endif
        const QStringList all = programs + QStringList{doc};
        auto d = showDeck(2);
        for (int i = 0; i < all.size(); ++i) linkShape(*d->pages[0], QRectF(10 + 100 * (i % 9), 300 + 100 * (i / 9), 90, 60), all[i]);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        QStringList notices, asked, opened;
        show.setLinkNotice([&](const QString &text) { notices << text; });
        show.setLinkPrompt([&](const QString &address, bool) {
            asked << address;
            return true;
        });
        show.setLinkOpener([&](const QUrl &url) { opened << url.toString(); });
        for (int i = 0; i < all.size(); ++i) QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(10 + 100 * (i % 9) + 50, 300 + 100 * (i / 9) + 30));
        QCOMPARE(notices.size(), int(programs.size()));
        QVERIFY(notices.first().startsWith(QStringLiteral("This link opens a program")));
        QCOMPARE(asked, QStringList{doc});
        QCOMPARE(opened.size(), 1);
        QVERIFY(opened.first().endsWith(QStringLiteral("doc.pdf")));
        QVERIFY(office::isProgramFile(QStringLiteral("./x.desktop")));
        QVERIFY(!office::isProgramFile(QStringLiteral("https://example.com/setup.exe")));   // (an address is not a file)
        QVERIFY(!office::isProgramFile(QStringLiteral("C:\\docs\\a.pdf")));
    }

    // One check for a file on another computer, for every place that follows a link (a slide's, a workbook's): the forms that a path can take.
    void isOnAnotherComputerKnowsTheForms()
    {
        for (const char *path : {"\\\\host\\share\\x.xlsx", "//host/share/x.xlsx", "file://host/share/x.xlsx", "file://///host/share/x.xlsx", "\\\\?\\UNC\\host\\share\\x.xlsx", "\\\\host@SSL\\share\\x.xlsx",
                                 "file:///\\\\host\\s\\x.xlsx", "file:///%5C%5Chost/s/x.xlsx", "/\\host\\s\\x.xlsx", "\\/host/s/x.xlsx", "file://///host/s/x.xlsx", "FILE://Host/s/x.xlsx", "file:\\\\host\\s"})
            QVERIFY2(jp::isOnAnotherComputer(QString::fromLatin1(path)), path);
        for (const char *path : {"file:///tmp/x%20y.xlsx", "file://localhost/tmp/x.xlsx", "C:\\docs\\a.pdf", "/home/a/b.pdf", "x.xlsx", "https://example.com/a", "mailto:a@example.com", "#next", "", "www.example.com"})
            QVERIFY2(!jp::isOnAnotherComputer(QString::fromLatin1(path)), path);
        // The slide's own check is that one.
        QVERIFY(office::isRemoteFile(QStringLiteral("file:///%5C%5Chost/s/x.pdf")));
    }

    // Jumps and file links come back from an .odp; so does a link written without a scheme.
    void odpKeepsJumpAndFileLinks()
    {
        auto d = showDeck(3);
        const QStringList links = {QStringLiteral("#next"), QStringLiteral("#previous"), QStringLiteral("#first"), QStringLiteral("#last"), QStringLiteral("#end"), QStringLiteral("#page2"),
                                   QStringLiteral("C:\\docs\\a.pdf"), QStringLiteral("/home/a/b.pdf"), QStringLiteral("https://example.com/a"), QStringLiteral("www.example.com")};
        for (int i = 0; i < links.size(); ++i) linkShape(*d->pages[0], QRectF(10 + 90 * (i % 10), 300, 80, 60), links[i]);
        auto *title = static_cast<TextItem *>(d->pages[0]->items[0].get());
        QTextCursor c(d->storyDoc(title->storyId));
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setAnchor(true);
        f.setAnchorHref(QStringLiteral("#page3"));
        c.mergeCharFormat(f);
        QString err;
        auto back = deck::importOdp(deck::exportOdp(*d, false, &err), &err);
        QVERIFY2(back, qPrintable(err));
        QStringList got;
        for (const auto &it : back->pages[0]->items)
            if (!it->hyperlink.isEmpty()) got << it->hyperlink;
        QCOMPARE(got, links);
        auto *backTitle = static_cast<TextItem *>(back->pages[0]->items[0].get());
        QTextCursor bc(back->storyDoc(backTitle->storyId));
        bc.setPosition(1);
        QCOMPARE(bc.charFormat().anchorHref(), QStringLiteral("#page3"));
    }

    // An address typed without a scheme stays a web link, written as an address with no file action, as it always was.
    void anAddressWithoutASchemeIsNotAFile()
    {
        QCOMPARE(office::parseLink(QStringLiteral("www.example.com")).kind, office::LinkKind::Web);
        QCOMPARE(office::parseLink(QStringLiteral("example.com/a")).kind, office::LinkKind::File);   // (it has a slash: a path)
        auto d = showDeck(2);
        linkShape(*d->pages[0], QRectF(10, 300, 90, 60), QStringLiteral("www.example.com"));
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        const QString xml = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(xml.contains(QStringLiteral("hlinkClick")));
        QVERIFY2(!xml.contains(QStringLiteral("hlinkfile")), "a scheme-less address is no file");
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->pages[0]->items.back()->hyperlink, QStringLiteral("www.example.com"));
    }

    // Insert > Link gives the words the look of a link at once (the theme's link color, underlined), and Remove Link takes both away.
    void insertLinkGivesTheLinkItsLook()
    {
        DeckWindow w;
        QTextDocument *story = editTitle(w, QStringLiteral("Visit our site"), 6, 9);
        const QColor linkColor = w.editor()->doc()->colors.slot(Hyperlink);
        withDialog([&] { w.act("insert.link")->trigger(); }, QStringLiteral("linkDialog"), [](QDialog *dlg) { dlg->findChild<QLineEdit *>(QStringLiteral("linkAddress"))->setText(QStringLiteral("example.com")); });
        QTextCursor c(story);
        c.setPosition(7);
        QVERIFY(c.charFormat().isAnchor());
        QVERIFY(c.charFormat().fontUnderline());
        QCOMPARE(c.charFormat().foreground().color(), linkColor);
        QCOMPARE(c.charFormat().stringProperty(tp::ColorRefP), ColorRef::rgb(linkColor).toString());
        c.setPosition(2);
        QVERIFY(!c.charFormat().fontUnderline());
        // Remove Link, with the words selected again.
        w.editor()->beginTextEdit(static_cast<TextItem *>(w.editor()->doc()->pages[0]->items[0].get())->id, 6);
        QTextCursor sel = w.editor()->cursor();
        sel.setPosition(6);
        sel.setPosition(9, QTextCursor::KeepAnchor);
        w.editor()->setCursor(sel);
        withDialog([&] { w.act("insert.link")->trigger(); }, QStringLiteral("linkDialog"), [](QDialog *dlg) {
            for (QPushButton *b : dlg->findChildren<QPushButton *>())
                if (b->text() == QLatin1String("Remove Link")) b->click();
        });
        QTextCursor after(story);
        after.setPosition(7);
        QVERIFY(!after.charFormat().isAnchor());
        QVERIFY(!after.charFormat().fontUnderline());
        QVERIFY(!after.charFormat().hasProperty(QTextFormat::ForegroundBrush));
        QVERIFY(after.charFormat().stringProperty(tp::ColorRefP).isEmpty());
    }

    // A jump to a slide that has been deleted is not dropped without a word: the save says so.
    void aJumpToADeletedSlideIsSaidAtSave()
    {
        auto d = showDeck(3);
        linkShape(*d->pages[0], QRectF(10, 300, 90, 60), QStringLiteral("#page3"));
        linkShape(*d->pages[0], QRectF(110, 300, 90, 60), QStringLiteral("#page2"));
        QString err;
        QStringList skipped;
        QVERIFY(!deck::exportPresentation(*d, &err, &skipped).isEmpty());
        QVERIFY2(skipped.filter(QStringLiteral("no longer")).isEmpty(), "both slides are there");
        d->pages.erase(d->pages.begin() + 2);
        skipped.clear();
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY(!bytes.isEmpty());
        QStringList said = skipped.filter(QStringLiteral("no longer"));
        QCOMPARE(said.size(), 1);
        QVERIFY2(said.first().contains(QStringLiteral("no longer")), qPrintable(said.first()));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->pages[0]->items.back()->hyperlink, QStringLiteral("#page2"));
    }

    // Pointing at the slide makes no layout: the areas of the slide on the screen are worked out once, from the layout that the drawing made.
    void hoverDoesNoLayout()
    {
        auto d = showDeck(2);
        auto *title = static_cast<TextItem *>(d->pages[0]->items[0].get());
        QTextCursor c(d->storyDoc(title->storyId));
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setAnchor(true);
        f.setAnchorHref(QStringLiteral("#next"));
        c.mergeCharFormat(f);
        linkShape(*d->pages[0], QRectF(10, 300, 90, 60), QStringLiteral("https://example.com/a"));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QVERIFY(!show.grab().isNull());   // (the slide is drawn)
        const qint64 before = LayoutCache::layoutsMadeForTest(), runs = deck::linkAreaRunsForTest();
        for (int i = 0; i < 30; ++i) QTest::mouseMove(&show, QPoint(20 + 20 * i, 100 + (i % 3) * 5));
        QTest::mouseMove(&show, QPoint(50, 330));
        QCOMPARE(show.cursor().shape(), Qt::PointingHandCursor);
        QCOMPARE(LayoutCache::layoutsMadeForTest(), before);
        QVERIFY2(deck::linkAreaRunsForTest() - runs <= 1, "the areas of the slide are worked out once");
    }

    // The eleven action buttons have codes in a publication too, so that saving one there keeps its picture.
    void actionButtonsHaveShapeCodes()
    {
        const QStringList names = {"Home", "Help", "Information", "ForwardNext", "BackPrevious", "End", "Beginning", "Return", "Document", "Sound", "Movie"};
        QSet<int> codes;
        for (const QString &name : names) {
            const QString preset = QStringLiteral("actionButton") + name;
            const int code = drawingShapeType(preset);
            QVERIFY2(code >= 190 && code <= 200, qPrintable(preset));
            QCOMPARE(presetForDrawingShapeType(code), preset);
            codes << code;
        }
        QCOMPARE(int(codes.size()), 11);
        QCOMPARE(drawingShapeType(QStringLiteral("actionButtonBlank")), 189);
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
    // Reset Background takes a slide's own background away, so it shows its layout's again; it is one step that undoes.
    void resetBackgroundIsOneStep()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        w.setBackground(gradientFill(Fill::Linear), true);
        ed->doc()->pages[1]->background = gradientFill(Fill::Radial);
        ed->doc()->pages[2]->background = gradientFill(Fill::Radial);
        const int at = ed->currentPage();
        w.resetBackground(false);
        QVERIFY(ed->doc()->pages[at]->background.isNone());
        ed->undo();
        QCOMPARE(ed->doc()->pages[at]->background.gradType, Fill::Radial);
        w.resetBackground(true);
        for (auto &p : ed->doc()->pages) QVERIFY(p->background.isNone());
        QCOMPARE(ed->doc()->masters[0]->background.type, Fill::Gradient);
        ed->undo();
        QCOMPARE(ed->doc()->pages[1]->background.type, Fill::Gradient);
        QCOMPARE(ed->doc()->pages[2]->background.type, Fill::Gradient);
    }

    // The Format Background box has Reset Background.
    void formatBackgroundBoxResets()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        ed->doc()->pages[0]->background = gradientFill(Fill::Radial);
        withDialog([&] { w.act("design.formatBackground")->trigger(); }, QStringLiteral("backgroundDialog"), [](QDialog *dlg) {
            auto *reset = dlg->findChild<QPushButton *>(QStringLiteral("resetBackground"));
            QVERIFY(reset);
            reset->click();
        });
        QVERIFY(ed->doc()->pages[0]->background.isNone());
        ed->undo();
        QCOMPARE(ed->doc()->pages[0]->background.type, Fill::Gradient);
    }

    // With slides selected in the slide sorter, a background is set on each of them and no other.
    void backgroundGoesToTheSelectedSlides()
    {
        DeckWindow w;
        w.resize(1300, 850);
        w.show();
        w.newSlide();
        w.newSlide();
        w.newSlide();
        w.setView(DeckWindow::View::Sorter);
        w.slideSorter()->select({1, 3}, 3);
        w.setBackground(gradientFill(Fill::Linear), false);
        Document *d = w.editor()->doc();
        QVERIFY(d->pages[0]->background.isNone() && d->pages[2]->background.isNone());
        QCOMPARE(d->pages[1]->background.type, Fill::Gradient);
        QCOMPARE(d->pages[3]->background.type, Fill::Gradient);
        w.editor()->undo();
        QVERIFY(d->pages[1]->background.isNone() && d->pages[3]->background.isNone());
    }

    // Every part of a gradient (rectangular and path types, a stop's transparency) and of a picture fill (tiled, scaled, clear) is saved and read back the same.
    void everyBackgroundSettingRoundTrips()
    {
        QImage png(8, 8, QImage::Format_RGB32);
        png.fill(QColor(0x40, 0x80, 0xC0));
        QByteArray bytes;
        QBuffer buf(&bytes);
        buf.open(QIODevice::WriteOnly);
        png.save(&buf, "PNG");
        for (int format = 0; format < 2; ++format) {
            const QString what = format == 0 ? QStringLiteral("pptx") : QStringLiteral("odp");
            auto d = deck::newPresentation();
            for (int i = 0; i < 3; ++i) deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
            Fill rect = gradientFill(Fill::Rectangular);
            rect.stops[1].transparency = 0.4;
            deck::setSlideBackground(*d->pages[0], rect);
            deck::setSlideBackground(*d->pages[1], gradientFill(Fill::PathGrad));
            Fill tiled;
            tiled.type = Fill::Picture;
            tiled.imageId = d->addImage(bytes, QStringLiteral("png"), QString());
            tiled.tile = true;
            tiled.tileScale = 2.0;
            tiled.transparency = 0.25;
            deck::setSlideBackground(*d->pages[2], tiled);
            QString err;
            std::unique_ptr<Document> back;
            if (format == 0) back = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
            else back = deck::importOdp(deck::exportOdp(*d, false, &err), &err);
            QVERIFY2(back, qPrintable(err));
            const Fill r = back->pages[0]->background;
            QVERIFY2(r.type == Fill::Gradient && r.gradType == Fill::Rectangular, qPrintable(what));
            // (A file of .odp fades from the first stop's opacity to the last's, and keeps a stop's own only in a linear gradient.)
            QVERIFY2(format == 1 || (r.stops.size() == 3 && std::abs(r.stops[1].transparency - 0.4) < 0.02), qPrintable(what));
            QVERIFY2(back->pages[1]->background.type == Fill::Gradient && (format == 1 || back->pages[1]->background.gradType == Fill::PathGrad), qPrintable(what));
            const Fill t = back->pages[2]->background;
            QVERIFY2(t.type == Fill::Picture || t.type == Fill::Texture, qPrintable(what));
            QVERIFY2(t.tile && std::abs(t.tileScale - 2.0) < 0.05 && std::abs(t.transparency - 0.25) < 0.02, qPrintable(what));
        }
    }
};

JP_TEST_CLASS(DeckSizeLinkTests)
#include "tst_deck_sizelinks.moc"
