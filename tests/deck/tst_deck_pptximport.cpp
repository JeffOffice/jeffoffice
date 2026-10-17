// JeffDeck's tests of reading a presentation file: slides, masters, pictures, and limits.
#include "deckhelpers.h"
#include "../timing.h"

class DeckPptxImportTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

private:
    // The online libraries answered from here (nothing goes online): a search finds one picture, "Red barn" by Pat Lee, whose file is
    // `picture`. The real fetch comes back when this goes.
    struct CannedLibraries {
        QStringList asked;
        std::function<void(const QUrl &, QObject *, const online::Done &, qint64)> kept = online::fetch;
        explicit CannedLibraries(const QByteArray &picture)
        {
            const QByteArray openverse = QJsonDocument(QJsonObject{{"results", QJsonArray{QJsonObject{
                {"id", "a1"}, {"title", "Red barn"}, {"creator", "Pat Lee"}, {"license", "by"}, {"license_version", "2.0"},
                {"license_url", "https://creativecommons.org/licenses/by/2.0/"}, {"foreign_landing_url", "https://example.org/page/a1"},
                {"url", "https://example.org/barn.png"}, {"thumbnail", "https://example.org/barn.thumb"}}}}}).toJson();
            online::fetch = [this, openverse, picture](const QUrl &u, QObject *, const online::Done &done, qint64) {
                asked << u.toString();
                done(u.host() == QLatin1String("api.openverse.org") ? openverse : picture, QString());
            };
        }
        ~CannedLibraries() { online::fetch = kept; }
    };

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

    // An SVG picture (an icon) is saved as the presentation program saves one: a PNG in the blip, for programs that don't draw SVG,
    // and the SVG itself in the blip's extension list (asvg:svgBlip), last in the blip, after the picture's other settings. The
    // pictures of one SVG share its two parts, and a picture of pixels has no extension.
    void svgPictureIsSavedWithAPngAndTheSvg()
    {
        auto d = deck::newPresentation();
        const QByteArray svg = iconSvg(QStringLiteral("heart"), QColor(0xCC, 0x11, 0x22));
        const QString image = d->addImage(svg, QStringLiteral("svg"));
        QVERIFY(!image.isEmpty());
        QImage red(8, 8, QImage::Format_RGB32);
        red.fill(Qt::red);
        const QString raster = d->addImage(imageBytes(red), QStringLiteral("png"));
        auto add = [&](const QString &name, const QString &id, double x, double transparency) {
            auto pic = std::make_shared<PictureItem>();
            pic->name = name;
            pic->rect = QRectF(x, 100, 72, 72);
            pic->imgRect = QRectF(0, 0, 72, 72);
            pic->imageId = id;
            pic->transparency = transparency;
            d->pages[0]->items.push_back(pic);
        };
        add(QStringLiteral("One"), image, 100, 0);
        add(QStringLiteral("Two"), image, 300, 0.5);
        add(QStringLiteral("Plain"), raster, 500, 0);
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.isEmpty(), qPrintable(skipped.join('\n')));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));

        const QString slidePart = QStringLiteral("/ppt/slides/slide1.xml");
        QDomDocument dom;
        QVERIFY2(office::parseXml(e.zip.value(slidePart.mid(1)), &dom, &err), qPrintable(err));
        const QString svgNs = QStringLiteral("http://schemas.microsoft.com/office/drawing/2016/SVG/main");
        // The blip of the picture named `name`.
        auto blipOf = [&](const QString &name) {
            const QDomNodeList pics = dom.elementsByTagNameNS(office::ns::P, QStringLiteral("pic"));
            for (int i = 0; i < pics.size(); ++i) {
                const QDomElement pic = pics.at(i).toElement();
                const QDomNodeList names = pic.elementsByTagNameNS(office::ns::P, QStringLiteral("cNvPr"));
                if (names.isEmpty() || names.at(0).toElement().attribute(QStringLiteral("name")) != name) continue;
                return pic.elementsByTagNameNS(office::ns::A, QStringLiteral("blip")).at(0).toElement();
            }
            return QDomElement();
        };
        auto embed = [](const QDomElement &e2) { return e2.attributeNS(office::ns::R, QStringLiteral("embed")); };
        auto localNames = [](const QDomElement &blip) {
            QStringList names;
            for (QDomElement c = blip.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) names << c.localName();
            return names;
        };
        const QDomElement one = blipOf(QStringLiteral("One")), two = blipOf(QStringLiteral("Two")), plain = blipOf(QStringLiteral("Plain"));
        QVERIFY(!one.isNull() && !two.isNull() && !plain.isNull());
        QCOMPARE(localNames(one), QStringList{QStringLiteral("extLst")});
        QCOMPARE(localNames(two), (QStringList{QStringLiteral("alphaModFix"), QStringLiteral("extLst")}));   // the extension is last
        QCOMPARE(localNames(plain), QStringList());
        // Each extension is the one the presentation program reads for an SVG, linking the SVG's own part.
        QStringList svgTargets;
        for (const QDomElement &blip : {one, two}) {
            const QDomElement list = blip.lastChildElement();
            const QDomElement ext = list.firstChildElement();
            QCOMPARE(list.childNodes().size(), 1);
            QCOMPARE(ext.localName(), QStringLiteral("ext"));
            QCOMPARE(ext.attribute(QStringLiteral("uri")), QStringLiteral("{96DAC541-7B7A-43D3-8B79-37D633B846F1}"));
            const QDomElement link = ext.firstChildElement();
            QCOMPARE(link.localName(), QStringLiteral("svgBlip"));
            QCOMPARE(link.namespaceURI(), svgNs);
            const office::Relationship r = e.pkg.relationship(slidePart, embed(link));
            QCOMPARE(r.type, QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/image"));
            QVERIFY2(r.target.endsWith(QLatin1String(".svg")), qPrintable(r.target));
            QCOMPARE(e.zip.value(r.target.mid(1)), svg);
            QCOMPARE(e.pkg.contentType(r.target), QStringLiteral("image/svg+xml"));
            svgTargets << r.target;
        }
        QCOMPARE(svgTargets[0], svgTargets[1]);
        // And its picture, as PNG, for the blip itself.
        QCOMPARE(embed(one), embed(two));
        const office::Relationship fallback = e.pkg.relationship(slidePart, embed(one));
        QVERIFY2(fallback.target.endsWith(QLatin1String(".png")), qPrintable(fallback.target));
        QVERIFY(e.zip.value(fallback.target.mid(1)).startsWith("\x89PNG"));
        QCOMPARE(e.pkg.contentType(fallback.target), QStringLiteral("image/png"));
        QVERIFY(e.pkg.relationship(slidePart, embed(plain)).target.endsWith(QLatin1String(".png")));
        // Two parts for the SVG's pictures and one for the picture of pixels.
        int svgParts = 0, pngParts = 0;
        for (const QString &name : e.zip.keys()) {
            if (!name.startsWith(QLatin1String("ppt/media/"))) continue;
            svgParts += name.endsWith(QLatin1String(".svg"));
            pngParts += name.endsWith(QLatin1String(".png"));
        }
        QCOMPARE(svgParts, 1);
        QCOMPARE(pngParts, 2);

        // Read again, the pictures are all there.
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        int pictures = 0;
        for (const auto &it : back->pages[0]->items) pictures += it->type() == ItemType::Picture && !static_cast<const PictureItem *>(it.get())->imageId.isEmpty();
        QCOMPARE(pictures, 3);
    }

    // The Insert tab has what JeffPub's has, in the groups the presentation program puts them in: Pictures is a split button (This Device,
    // Online Pictures), Icons is beside Shapes, and Text Art is a gallery in the Text group, between Header & Footer and Date & Time.
    void insertTabHasPicturesIconsAndTextArt()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        for (const char *id : {"insert.picture", "insert.pictureDevice", "insert.onlinePicture", "insert.icons"}) QVERIFY2(w.act(QString::fromLatin1(id)), id);
        QCOMPARE(w.act(QStringLiteral("insert.pictureDevice"))->text(), QStringLiteral("This Device…"));
        QCOMPARE(w.act(QStringLiteral("insert.onlinePicture"))->text(), QStringLiteral("Online Pictures…"));
        QCOMPARE(w.act(QStringLiteral("insert.icons"))->text(), QStringLiteral("Icons"));
        QCOMPARE(w.act(QStringLiteral("insert.picture"))->text(), QStringLiteral("Pictures"));   // the face, as it was
        for (const char *id : {"insert.pictureDevice", "insert.onlinePicture", "insert.icons"}) QVERIFY2(!w.act(QString::fromLatin1(id))->icon().isNull(), id);
        // The tab as built: from "tab Insert" to the next tab.
        const QStringList lines = w.ribbon()->describe().split(QLatin1Char('\n'));
        const qsizetype from = lines.indexOf(QStringLiteral("tab Insert"));
        QVERIFY(from >= 0);
        QStringList insert;
        for (qsizetype i = from + 1; i < lines.size() && lines[i].startsWith(QLatin1Char(' ')); ++i) insert << lines[i].trimmed();
        auto line = [&](const QString &needle) {
            for (qsizetype i = 0; i < insert.size(); ++i)
                if (insert[i].contains(needle)) return int(i);
            return -1;
        };
        const int images = line(QStringLiteral("group Images")), illustrations = line(QStringLiteral("group Illustrations")), text = line(QStringLiteral("group Text"));
        const int pictures = line(QStringLiteral("insert.picture "));
        QVERIFY(images >= 0 && illustrations > images && text > illustrations);
        QVERIFY2(pictures > images && pictures < illustrations, qPrintable(insert.join('\n')));
        QVERIFY2(insert[pictures].contains(QStringLiteral("popup=split menu=[insert.pictureDevice, insert.onlinePicture]")), qPrintable(insert[pictures]));
        const int icons = line(QStringLiteral("insert.icons"));
        QVERIFY(icons > illustrations && icons < text);
        QVERIFY(line(QStringLiteral("GalleryButton")) > illustrations);   // Shapes
        // Text: Text Box, Header & Footer, the Text Art gallery, then Date & Time.
        const int box = line(QStringLiteral("insert.textBox")), footer = line(QStringLiteral("insert.headerFooter")), date = line(QStringLiteral("insert.dateTime"));
        int art = -1;
        for (int i = int(insert.size()) - 1; i > text; --i)
            if (insert[i].startsWith(QStringLiteral("jp::GalleryButton"))) art = i;
        QVERIFY2(box > text && footer > box && art > footer && date > art, qPrintable(insert.join('\n')));
        // The gallery offers every Text Art style.
        QWidget *tab = w.ribbon()->tab(QStringLiteral("Insert"));
        GalleryButton *gallery = nullptr;
        for (GalleryButton *g : tab->findChildren<GalleryButton *>())
            if (g->text() == QStringLiteral("Text Art")) gallery = g;
        QVERIFY(gallery);
        QCOMPARE(int(gallery->items().size()), int(textArtStyles().size()));
        // Every control has a KeyTip of its own.
        QStringList seen;
        for (const QWidget *c : tab->findChildren<QWidget *>()) {
            const QString k = keytip(c);
            if (k.isEmpty()) continue;
            QVERIFY2(!seen.contains(k), qPrintable(k));
            seen << k;
        }
        QVERIFY(seen.contains(QStringLiteral("IC")) && seen.contains(QStringLiteral("WA")));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            w.resize(1440, 900);
            w.show();
            w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Insert")));
            QVERIFY(QTest::qWaitForWindowExposed(&w));
            shot(&w, QStringLiteral("insert-tab"));
            // The two controls that open: the Text Art gallery, and the Pictures menu.
            gallery->click();
            QTRY_VERIFY(QApplication::activePopupWidget());
            shot(QApplication::activePopupWidget(), QStringLiteral("text-art-gallery"));
            QApplication::activePopupWidget()->close();
            QTRY_VERIFY(!QApplication::activePopupWidget());
            QTimer::singleShot(0, [] {
                untilDone([] {
                    QWidget *popup = QApplication::activePopupWidget();
                    if (!popup) return false;
                    shot(popup, QStringLiteral("pictures-menu"));
                    popup->close();
                    return true;
                });
            });
            for (QToolButton *b : tab->findChildren<QToolButton *>())
                if (b->defaultAction() == w.act(QStringLiteral("insert.picture"))) b->showMenu();
        }
    }

    // Insert > Icons asks which (the picker JeffPub has), and puts each on the slide as an SVG picture an inch square, in the theme's text
    // color, side by side across the middle: one step that undoes, all of them selected, and saved with the SVG itself.
    void insertingIconsPutsSvgPicturesOnTheSlide()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        const int before = int(ed->doc()->pages[0]->items.size());
        w.insertIcons(QStringList{QStringLiteral("heart")});
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        const auto *pic = dynamic_cast<const PictureItem *>(ed->doc()->pages[0]->items.back().get());
        QVERIFY(pic);
        QCOMPARE(pic->rect, QRectF(480 - 36, 270 - 36, 72, 72));
        QCOMPARE(pic->imgRect, QRectF(0, 0, 72, 72));
        QCOMPARE(ed->doc()->images.value(pic->imageId).format, QStringLiteral("svg"));
        QCOMPARE(pic->altText, QStringLiteral("heart"));
        QCOMPARE(pic->name, QStringLiteral("heart icon"));
        QCOMPARE(ed->selection(), QStringList{pic->id});
        // In the theme's text color, not the color of the text around it.
        const QByteArray svg = ed->doc()->images.value(pic->imageId).bytes;
        QVERIFY(svg.contains(ed->doc()->colors.c[Main].name(QColor::HexRgb).toLatin1()));
        QVERIFY(!svg.contains("currentColor"));
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);

        // Several at once: side by side, centered, one step.
        w.insertIcons(QStringList{QStringLiteral("heart"), QStringLiteral("star"), QStringLiteral("no-such-icon"), QStringLiteral("tag")});
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 3);   // the one that is no icon is left out
        QCOMPARE(int(ed->selection().size()), 3);
        const QRectF a = ed->doc()->pages[0]->items[before]->rect, b = ed->doc()->pages[0]->items[before + 1]->rect, c = ed->doc()->pages[0]->items[before + 2]->rect;
        QCOMPARE(a.size(), QSizeF(72, 72));
        QCOMPARE(b.center().x(), 480.0);
        QCOMPARE(a.center().y(), 270.0);
        QCOMPARE(a.center().x() + c.center().x(), 960.0);
        QVERIFY(a.right() < b.left() && b.right() < c.left());
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);
        w.insertIcons(QStringList());
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);

        // The command asks which, in the picker.
        QString shown;
        QTimer::singleShot(0, [&shown] {
            untilDone([&shown] {
                auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!dlg) return false;
                shown = dlg->windowTitle();
                if (auto *search = dlg->findChild<QLineEdit *>()) search->setText(QStringLiteral("heart"));
                auto *list = dlg->findChild<QListWidget *>(QStringLiteral("iconList"));
                for (int i = 0; list && i < list->count(); ++i)
                    if (!list->item(i)->isHidden() && list->item(i)->data(Qt::UserRole).toString() == QLatin1String("heart")) list->item(i)->setSelected(true);
                if (auto *search = dlg->findChild<QLineEdit *>()) {
                    search->setText(QString());
                    shot(dlg, QStringLiteral("icon-picker"));
                    search->setText(QStringLiteral("heart"));
                }
                dlg->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
                return true;
            });
        });
        w.act(QStringLiteral("insert.icons"))->trigger();
        QCOMPARE(shown, QStringLiteral("Insert Icons"));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        QCOMPARE(ed->doc()->images.value(static_cast<const PictureItem *>(ed->doc()->pages[0]->items.back().get())->imageId).format, QStringLiteral("svg"));
        // And cancelling puts nothing.
        QTimer::singleShot(0, [] {
            untilDone([] {
                auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!dlg) return false;
                dlg->reject();
                return true;
            });
        });
        w.act(QStringLiteral("insert.icons"))->trigger();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);

        // Saved, the icon has its PNG and its SVG.
        QString err;
        const QByteArray bytes = deck::exportPresentation(*ed->doc(), &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<asvg:svgBlip ")));
    }

    // Insert > Text Art lists the styles and makes a Text Art in the one chosen, in the middle of the slide, and asks for its words with
    // the old ones selected, so that typing replaces them; a double click on it asks again. Cancelling leaves the art with its words.
    void insertingTextArtMakesStyledArtAndAsksForItsWords()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        const auto &styles = textArtStyles();
        QVERIFY(styles.size() > 3);
        const int before = int(ed->doc()->pages[0]->items.size());
        QString asked, selected;
        auto answer = [&](const QString &typed) {
            QTimer::singleShot(0, [&asked, &selected, typed] {
                untilDone([&asked, &selected, typed] {
                    auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                    if (!dlg) return false;
                    asked = dlg->windowTitle();
                    auto *edit = dlg->findChild<QPlainTextEdit *>();
                    selected = edit ? edit->textCursor().selectedText() : QString();
                    shot(dlg, QStringLiteral("text-art-words"));
                    if (typed.isEmpty()) {
                        dlg->reject();
                    } else {
                        QTest::keyClicks(edit, typed);   // typing replaces the selected words
                        dlg->accept();
                    }
                    return true;
                });
            });
        };
        answer(QString());
        w.insertTextArt(styles[3].id);
        QCOMPARE(asked, QStringLiteral("Edit Text Art Text"));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        auto *art = dynamic_cast<TextArtItem *>(ed->doc()->pages[0]->items.back().get());
        QVERIFY(art);
        QCOMPARE(art->styleId, styles[3].id);
        QCOMPARE(art->font, styles[3].font);
        QVERIFY(!art->text.isEmpty());
        QCOMPARE(selected, art->text);   // all of its words are selected
        QCOMPARE(art->rect.center(), QPointF(480, 270));
        QVERIFY(art->rect.width() >= 300 && art->rect.width() <= 960 * 0.8 && art->rect.height() >= 60);
        QCOMPARE(ed->selection(), QStringList{art->id});
        const QString words = art->text, artId = art->id;
        // Typing replaces the words, and is one step that undoes.
        answer(QStringLiteral("Big sale"));
        w.insertTextArt(styles[0].id);
        auto *second = dynamic_cast<TextArtItem *>(ed->doc()->pages[0]->items.back().get());
        QVERIFY(second && second->id != artId);
        QCOMPARE(second->text, QStringLiteral("Big sale"));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 2);
        ed->undo();   // the words
        QCOMPARE(dynamic_cast<TextArtItem *>(ed->doc()->pages[0]->items.back().get())->text, words);
        ed->undo();   // the art
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);

        // A double click on it, or Enter with it selected, asks for the words again.
        asked.clear();
        answer(QStringLiteral("New words"));
        Q_EMIT w.canvas()->editTextArtWanted(artId);
        QCOMPARE(asked, QStringLiteral("Edit Text Art Text"));
        QCOMPARE(static_cast<TextArtItem *>(ed->doc()->item(artId))->text, QStringLiteral("New words"));
        asked.clear();
        answer(QString());
        ed->select(artId);
        QTest::keyClick(w.canvas(), Qt::Key_Return);
        QCOMPARE(asked, QStringLiteral("Edit Text Art Text"));
        // The ribbon's gallery makes the same.
        GalleryButton *gallery = nullptr;
        for (GalleryButton *g : w.ribbon()->tab(QStringLiteral("Insert"))->findChildren<GalleryButton *>())
            if (g->text() == QStringLiteral("Text Art")) gallery = g;
        QVERIFY(gallery);
        answer(QString());
        Q_EMIT gallery->activated(styles[5].id);
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 2);
        QCOMPARE(static_cast<TextArtItem *>(ed->doc()->pages[0]->items.back().get())->styleId, styles[5].id);
        // Saved, it is Text Art still.
        QString err;
        const QByteArray bytes = deck::exportPresentation(*ed->doc(), &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        int arts = 0;
        for (const auto &it : back->pages[0]->items) arts += it->type() == ItemType::TextArt;
        QCOMPARE(arts, 2);
    }

    // Insert > Pictures > Online Pictures opens the pane on the right (the one JeffPub has) that says what it sends and where; searching and
    // inserting puts the picture in the middle of the slide, inside it, with its title as alt text and, as asked, the credit under it, as
    // one step that undoes. Offline: the libraries' answers are canned.
    void onlinePicturesPaneInsertsAPictureWithItsCredit()
    {
        // A picture much bigger than the slide.
        QImage big(2400, 1200, QImage::Format_RGB32);
        big.fill(Qt::red);
        CannedLibraries libraries(imageBytes(big));
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        QVERIFY(!w.onlinePictures());
        const int before = int(ed->doc()->pages[0]->items.size());
        w.act(QStringLiteral("insert.onlinePicture"))->trigger();
        OnlinePicturesPane *pane = w.onlinePictures();
        QVERIFY(pane);
        QVERIFY(pane->isVisible());
        QVERIFY(pane->window() == &w);
        // It says what it sends.
        QStringList words;
        for (const QLabel *l : pane->findChildren<QLabel *>()) words << l->text();
        QVERIFY2(words.join(' ').contains(QStringLiteral("Your search words go to the library you choose.")), qPrintable(words.join('\n')));
        // Asking again shows the same pane.
        w.act(QStringLiteral("insert.onlinePicture"))->trigger();
        QCOMPARE(w.onlinePictures(), pane);

        pane->search(QStringLiteral("barn"));
        QCOMPARE(pane->results()->count(), 1);
        QVERIFY(pane->creditBox()->isChecked());   // this license asks for the author to be named
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            w.resize(1440, 900);
            w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Insert")));
            QVERIFY(QTest::qWaitForWindowExposed(&w));
            shot(&w, QStringLiteral("online-pictures"));
        }
        pane->insertButton()->click();
        QVERIFY(libraries.asked.contains(QStringLiteral("https://example.org/barn.png")));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 2);   // the picture and its credit
        const auto *pic = dynamic_cast<const PictureItem *>(ed->single());
        QVERIFY(pic);
        QCOMPARE(pic->altText, QStringLiteral("Red barn"));
        QCOMPARE(ed->doc()->imageSize(pic->imageId), QSize(2400, 1200));
        // Inside the slide with room for the credit, in the middle, in proportion.
        const QRectF slide(0, 0, 960, 540);
        QVERIFY(slide.contains(pic->rect));
        QVERIFY(pic->rect.width() <= 960 * 0.8 + 0.01 && pic->rect.height() <= 540 * 0.8 + 0.01);
        QVERIFY(std::abs(pic->rect.width() / pic->rect.height() - 2.0) < 0.01);
        QVERIFY(std::abs(pic->rect.center().x() - 480) < 0.01);
        const auto *credit = dynamic_cast<const TextItem *>(ed->doc()->item(pic->caption));
        QVERIFY(credit);
        QCOMPARE(ed->doc()->storyDoc(credit->storyId)->toPlainText(), QStringLiteral("“Red barn” by Pat Lee, CC BY 2.0"));
        QVERIFY(slide.contains(credit->rect));
        QVERIFY(credit->rect.top() >= pic->rect.bottom());
        QVERIFY(credit->rect.top() - pic->rect.bottom() < 20);
        QCOMPARE(ed->selection(), QStringList{pic->id});
        // The picture and its credit go together.
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);
        ed->redo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 2);
        ed->undo();

        // Without the credit, only the picture.
        pane->creditBox()->setChecked(false);
        pane->insertButton()->click();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        const auto *alone = dynamic_cast<const PictureItem *>(ed->single());
        QVERIFY(alone);
        QCOMPARE(alone->altText, QStringLiteral("Red barn"));
        QVERIFY(alone->caption.isEmpty());
        // Saved, the picture has its alt text.
        QString err;
        const QByteArray bytes = deck::exportPresentation(*ed->doc(), &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("descr=\"Red barn\"")));
        // F1 in the pane opens the topic that explains it.
        QCOMPARE(help::contextTopic(QStringLiteral("pane:online")), QStringLiteral("objects"));
        QVERIFY(QTest::qWaitForWindowActive(&w));
        pane->findChild<QLineEdit *>()->setFocus();
        QTest::keyClick(QApplication::focusWidget(), Qt::Key_F1);
        QVERIFY(w.helpView());
        QCOMPARE(w.helpView()->currentTopic(), QStringLiteral("objects"));
    }

    // A slide with a picture from the web, an icon, and Text Art on it, each moved to a place of its own: saved and opened again, all three
    // are there, inside the slide.
    void onlinePictureIconAndTextArtSaveAndReopen()
    {
        QImage landscape(480, 320, QImage::Format_RGB32);
        landscape.fill(QColor(0xB0, 0x40, 0x30));
        CannedLibraries libraries(imageBytes(landscape));
        DeckWindow w;
        Editor *ed = w.editor();
        w.act(QStringLiteral("insert.onlinePicture"))->trigger();
        w.onlinePictures()->search(QStringLiteral("barn"));
        w.onlinePictures()->insertButton()->click();
        // As the user drags: the selected object to a new place, with its credit when it has one.
        auto place = [&](const QPointF &center) {
            ed->change(QStringLiteral("Move"), [&] {
                Item *it = ed->single();
                const QPointF by = center - it->rect.center();
                it->rect.translate(by);
                if (it->type() == ItemType::Picture)
                    if (Item *credit = ed->doc()->item(static_cast<PictureItem *>(it)->caption)) credit->rect.translate(by);
            });
        };
        place(QPointF(240, 220));
        w.insertIcons(QStringList{QStringLiteral("rocket")});
        place(QPointF(720, 160));
        QTimer::singleShot(0, [] {
            untilDone([] {
                auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!dlg) return false;
                dlg->reject();
                return true;
            });
        });
        w.insertTextArt(textArtStyles()[2].id);
        place(QPointF(700, 420));
        int pictures = 0, arts = 0, texts = 0;
        for (const auto &it : ed->doc()->pages[0]->items) {
            pictures += it->type() == ItemType::Picture;
            arts += it->type() == ItemType::TextArt;
            texts += it->type() == ItemType::Text && it->role.isEmpty();
        }
        QCOMPARE(pictures, 2);   // the picture from the web and the icon
        QCOMPARE(arts, 1);
        QCOMPARE(texts, 1);      // the credit
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            w.resize(1440, 900);
            w.show();
            w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Insert")));
            QVERIFY(QTest::qWaitForWindowExposed(&w));
            ed->select(QStringList());
            shot(&w, QStringLiteral("slide-with-each"));
        }
        QString err;
        const QByteArray bytes = deck::exportPresentation(*ed->doc(), &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        // Kept beside the screenshots, for opening in the presentation program.
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            QFile kept(QDir(qEnvironmentVariable("JP_SHOT_DIR")).filePath(QStringLiteral("inserted-objects.pptx")));
            if (kept.open(QIODevice::WriteOnly)) kept.write(bytes);
        }
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const QRectF slide(0, 0, 960, 540);
        int backPictures = 0, backArts = 0;
        QStringList alt;
        for (const auto &it : back->pages[0]->items) {
            if (it->type() != ItemType::Picture && it->type() != ItemType::TextArt && it->type() != ItemType::Text) continue;
            QVERIFY2(slide.contains(it->rect), qPrintable(it->name));
            backPictures += it->type() == ItemType::Picture;
            backArts += it->type() == ItemType::TextArt;
            if (it->type() == ItemType::Picture) alt << it->altText;
        }
        alt.sort();
        QCOMPARE(backPictures, 2);
        QCOMPARE(backArts, 1);
        QCOMPARE(alt, (QStringList{QStringLiteral("Red barn"), QStringLiteral("rocket")}));
    }

    // A picture added is not decoded: only its size is read. The first time it
    // is drawn it is decoded, and a decode the caller already has is kept.
    void addImageKeepsNoDecodeUntilDrawn()
    {
        Document d;
        QImage img(64, 32, QImage::Format_RGB32);
        img.fill(Qt::red);
        const QString id = d.addImage(imageBytes(img), QStringLiteral("png"));
        QVERIFY(!id.isEmpty());
        QCOMPARE(d.images.value(id).pixelSize, QSize(64, 32));
        QVERIFY(d.images.value(id).cache.isNull());
        QCOMPARE(d.image(id).size(), QSize(64, 32));
        QVERIFY(!d.images.value(id).cache.isNull());
        Document e;
        const QString kept = e.addImage(imageBytes(img), QStringLiteral("png"), QString(), img);
        QVERIFY(!e.images.value(kept).cache.isNull());
        // The size read is the size decoding gives: for a JPEG that says it is turned, with its sides swapped,
        // and for a metafile, which is drawn 1,600 pixels on its long side.
        QByteArray turned;
        QBuffer buffer(&turned);
        buffer.open(QIODevice::WriteOnly);
        QImageWriter writer(&buffer, "JPEG");
        writer.setTransformation(QImageIOHandler::TransformationRotate90);
        QVERIFY(writer.write(img));
        const QString jpeg = d.addImage(turned, QStringLiteral("jpg"));
        QVERIFY(d.images.value(jpeg).cache.isNull());
        const QSize jpegSize = d.images.value(jpeg).pixelSize;
        QCOMPARE(d.image(jpeg).size(), jpegSize);
        QCOMPARE(jpegSize, QSize(32, 64));
        const QString emf = d.addImage(emfBytes(), QStringLiteral("emf"));
        QVERIFY(!emf.isEmpty());
        QVERIFY(d.images.value(emf).cache.isNull());
        QCOMPARE(d.images.value(emf).pixelSize, QSize(1600, 800));
        QCOMPARE(d.image(emf).size(), QSize(1600, 800));
        // What isn't a picture is kept as it always was, with a size that is nothing.
        const QString junk = d.addImage(QByteArray(100, 'x'), QStringLiteral("png"));
        QVERIFY(!junk.isEmpty());
        QVERIFY(d.image(junk).isNull());
    }

    // A picture of more than 256 million pixels (a decode of a gigabyte), and a
    // metafile of more than 32 MB, are left out, and the document counts them
    // for the importer's notes. The reference decks' largest picture has 9 million
    // pixels, JeffPub's own files' 101 million, and the largest metafile is under 1 MB.
    void addImageLeavesOutPicturesPastTheLimits()
    {
        Document d;
        QImage atLimit(16384, 16384, QImage::Format_Mono);
        atLimit.fill(0);
        const QString kept = d.addImage(imageBytes(atLimit), QStringLiteral("png"));
        QVERIFY(!kept.isEmpty());
        QVERIFY(d.images.value(kept).cache.isNull());
        QCOMPARE(d.images.value(kept).pixelSize, QSize(16384, 16384));
        QCOMPARE(d.imagesLeftOut, 0);
        QImage past(20000, 16384, QImage::Format_Mono);
        past.fill(0);
        const QByteArray png = imageBytes(past);
        QString none;
        const qint64 ms = bestNanoseconds([&] { none = d.addImage(png, QStringLiteral("png")); }, 5, [&] {
            d.imagesLeftOut = 0;   // one picture left out, as for one call
        }) / 1000000;
        QVERIFY(none.isEmpty());
        QVERIFY2(ms < 3000, "reading the size of a picture must not decode it");
        QCOMPARE(d.imagesLeftOut, 1);
        QCOMPARE(int(d.images.size()), 1);
        // The same picture again is left out again (and counted again), not found.
        QVERIFY(d.addImage(png, QStringLiteral("png")).isEmpty());
        QCOMPARE(d.imagesLeftOut, 2);
        // A metafile of 33 MB.
        const QByteArray emf = emfBytes() + QByteArray(33 * 1024 * 1024, '\0');
        QVERIFY(d.addImage(emf, QStringLiteral("emf")).isEmpty());
        QCOMPARE(d.imagesLeftOut, 3);
        QVERIFY(!d.addImage(emfBytes(), QStringLiteral("emf")).isEmpty());
        QCOMPARE(d.imagesLeftOut, 3);
    }

    // The pictures decoded are kept up to a budget; past it, those used longest
    // ago are dropped (they are decoded again when drawn).
    void decodedPicturesStayWithinTheirBudget()
    {
        Document d;
        d.imageCacheBudget = 14 * 1024 * 1024;   // three and a half of the 4 MB decodes below
        QStringList ids;
        for (int i = 0; i < 12; ++i) ids << d.addImage(imageBytes(grayImage(1024, 1024, 20 * i)), QStringLiteral("png"));
        QCOMPARE(QSet<QString>(ids.cbegin(), ids.cend()).size(), 12);
        auto held = [&] {
            qint64 n = 0;
            for (const ImageData &image : d.images) n += image.cache.sizeInBytes();
            return n;
        };
        QCOMPARE(held(), qint64(0));
        for (int i = 0; i < 3; ++i) QVERIFY(!d.image(ids[i]).isNull());
        QVERIFY(held() <= d.imageCacheBudget);
        QVERIFY(!d.images.value(ids[1]).cache.isNull());
        d.image(ids[0]);                                   // used again: the one used longest ago is now ids[1]
        QVERIFY(!d.image(ids[3]).isNull());
        QVERIFY(d.images.value(ids[1]).cache.isNull());
        QVERIFY(!d.images.value(ids[0]).cache.isNull());
        QVERIFY(!d.images.value(ids[3]).cache.isNull());
        for (int i = 4; i < 12; ++i) {
            QVERIFY(!d.image(ids[i]).isNull());
            QVERIFY(held() <= d.imageCacheBudget);
        }
        const QImage again = d.image(ids[1]);
        QCOMPARE(again.size(), QSize(1024, 1024));
        QCOMPARE(qGray(again.pixel(0, 0)), 20);
        QVERIFY(held() <= d.imageCacheBudget);
        // A picture bigger than the budget is still drawn: it is the only one kept.
        d.imageCacheBudget = 1024;
        const QImage alone = d.image(ids[7]);
        QCOMPARE(alone.size(), QSize(1024, 1024));
        QCOMPARE(held(), alone.sizeInBytes());
    }

    // A picture added again is found by what its bytes hash to, wherever the
    // bytes are kept; entries put in some other way (a file read) are found too, and
    // an entry whose bytes were changed is never taken for the old bytes.
    void addImageFindsKnownPicturesByHash()
    {
        Document d;
        QVector<QByteArray> files;
        QStringList ids;
        for (int i = 0; i < 300; ++i) {
            QImage img(8, 8, QImage::Format_RGB32);
            img.fill(QColor(i % 256, i / 256, 7));
            files << imageBytes(img);
            ids << d.addImage(files[i], QStringLiteral("png"));
        }
        QCOMPARE(QSet<QString>(ids.cbegin(), ids.cend()).size(), 300);
        for (int i = 299; i >= 0; --i) QCOMPARE(d.addImage(QByteArray(files[i].constData(), files[i].size()), QStringLiteral("png")), ids[i]);
        QCOMPARE(int(d.images.size()), 300);
        ImageData loaded;
        loaded.bytes = grayPng(8, 8, 99);
        loaded.format = QStringLiteral("png");
        d.images.insert(QStringLiteral("zz-loaded"), loaded);
        QCOMPARE(d.addImage(QByteArray(loaded.bytes.constData(), loaded.bytes.size()), QStringLiteral("png")), QStringLiteral("zz-loaded"));
        d.images[ids[5]].bytes = grayPng(8, 8, 50);
        const QString fresh = d.addImage(QByteArray(files[5].constData(), files[5].size()), QStringLiteral("png"));
        QVERIFY(fresh != ids[5] && !fresh.isEmpty());
        QCOMPARE(d.addImage(QByteArray(files[6].constData(), files[6].size()), QStringLiteral("png")), ids[6]);
    }

    // The same picture used again and again (one image part listed by thousands of
    // pictures) is not compared with the stored copy each time.
    void addImageDoesNotCompareASharedPictureAgain()
    {
        QImage img(2048, 2048, QImage::Format_RGB32);
        img.fill(Qt::darkGreen);
        const QByteArray bmp = imageBytes(img, "BMP");
        QVERIFY(bmp.size() > 10 * 1024 * 1024);
        Document d;
        const QString id = d.addImage(bmp, QStringLiteral("bmp"));
        QVERIFY(!id.isEmpty());
        const qint64 ms = bestNanoseconds([&] {
            for (int i = 0; i < 4000; ++i)
                if (d.addImage(bmp, QStringLiteral("bmp")) != id) QFAIL("a picture used again is stored again");
        }) / 1000000;
        QVERIFY2(ms < 1000, qPrintable(QStringLiteral("took %1 ms").arg(ms)));
        QCOMPARE(int(d.images.size()), 1);
    }

    // An SVG picture that could read a file, run a script, or take minutes to
    // draw is left out; ordinary ones, with their links to their own parts and
    // their small pictures, are kept and drawn.
    void addImageLeavesOutHostileSvg()
    {
        Document d;
        int left = 0;
        auto leavesOut = [&](const char *what, const QByteArray &svg) {
            QString id;
            const qint64 ms = bestNanoseconds([&] { id = d.addImage(svg, QStringLiteral("svg")); }, 5, [&] {
                d.imagesLeftOut = left;   // each call leaves one out, counted from here
            }) / 1000000;
            QVERIFY2(id.isEmpty(), what);
            QVERIFY2(ms < 2000, what);
            QCOMPARE(d.imagesLeftOut, ++left);
        };
        auto keeps = [&](const char *what, const QByteArray &svg) {
            const QString id = d.addImage(svg, QStringLiteral("svg"));
            QVERIFY2(!id.isEmpty(), what);
            QVERIFY2(!d.image(id).isNull(), what);
        };
        const QString rect = QStringLiteral("<rect width=\"10\" height=\"10\" fill=\"red\"/>");
        leavesOut("script", svgOf(QStringLiteral("<script>alert(1)</script>") + rect));
        leavesOut("foreignObject", svgOf(QStringLiteral("<foreignObject width=\"10\" height=\"10\"><p xmlns=\"http://www.w3.org/1999/xhtml\">x</p></foreignObject>")));
        leavesOut("local file", svgOf(QStringLiteral("<image xlink:href=\"/etc/hostname\" width=\"10\" height=\"10\"/>")));
        leavesOut("file URL", svgOf(QStringLiteral("<image xlink:href=\"file:///etc/hostname\" width=\"10\" height=\"10\"/>")));
        leavesOut("server share", svgOf(QStringLiteral("<image xlink:href=\"//server/share/x.png\" width=\"10\" height=\"10\"/>")));
        leavesOut("relative path", svgOf(QStringLiteral("<image href=\"x.png\" width=\"10\" height=\"10\"/>")));
        leavesOut("web address", svgOf(QStringLiteral("<image href=\"https://example.com/x.png\" width=\"10\" height=\"10\"/>")));
        leavesOut("another file's part", svgOf(QStringLiteral("<use xlink:href=\"other.svg#a\"/>")));
        leavesOut("a style that names a file", svgOf(QStringLiteral("<rect width=\"10\" height=\"10\" style=\"fill:url(/tmp/x.png)\"/>")));
        leavesOut("an imported style sheet", svgOf(QStringLiteral("<style>@import url(x.css);</style>") + rect));
        leavesOut("doubling uses", svgOf(useBomb(26)));
        leavesOut("doubling uses, a few more", svgOf(useBomb(20)));
        leavesOut("a use of itself", svgOf(QStringLiteral("<g id=\"a\"><use xlink:href=\"#a\"/></g>")));
        leavesOut("a use of a parent", svgOf(QStringLiteral("<g id=\"a\"><g><use xlink:href=\"#a\"/></g></g>")));
        leavesOut("deep nesting", svgOf(QStringLiteral("<g>").repeated(200) + QStringLiteral("</g>").repeated(200)));
        leavesOut("a hundred thousand and one parts", svgOf(QStringLiteral("<g/>").repeated(100001)));
        leavesOut("not an svg", QByteArray("<html><body><svg xmlns=\"http://www.w3.org/2000/svg\"/></body></html>"));
        leavesOut("not well formed", QByteArray("<svg xmlns=\"http://www.w3.org/2000/svg\"><g></svg>"));
        // A picture inside one: a nested SVG, or one of more pixels than the limit.
        leavesOut("an svg inside", svgOf(QStringLiteral("<image width=\"10\" height=\"10\" xlink:href=\"data:image/svg+xml;base64,%1\"/>").arg(QString::fromLatin1(svgOf(rect).toBase64()))));
        QImage past(20000, 16384, QImage::Format_Mono);
        past.fill(0);
        leavesOut("a picture past the limit inside", svgOf(QStringLiteral("<image width=\"10\" height=\"10\" xlink:href=\"data:image/png;base64,%1\"/>").arg(QString::fromLatin1(imageBytes(past).toBase64()))));
        QVERIFY(d.images.isEmpty());
        keeps("plain", svgOf(rect));
        keeps("uses of a part", svgOf(QStringLiteral("<defs>%1</defs><use xlink:href=\"#a\"/><use href=\"#a\" x=\"20\"/>").arg(QStringLiteral("<g id=\"a\">%1</g>").arg(rect))));
        keeps("eight doublings", svgOf(useBomb(8)));
        keeps("a link", svgOf(QStringLiteral("<a xlink:href=\"https://example.com\">%1</a>").arg(rect)));
        keeps("a gradient", svgOf(QStringLiteral("<defs><linearGradient id=\"g\"><stop offset=\"0\" stop-color=\"red\"/><stop offset=\"1\" stop-color=\"blue\"/></linearGradient></defs>"
                                                 "<rect width=\"10\" height=\"10\" style=\"fill:url(#g)\"/><rect width=\"5\" height=\"5\" fill=\"url(#g)\"/>")));
        keeps("a small picture inside", svgOf(QStringLiteral("<image width=\"10\" height=\"10\" xlink:href=\"data:image/png;base64,%1\"/>").arg(QString::fromLatin1(tinyPng().toBase64()))));
        keeps("anchors that link to the part they are in", svgOf(QStringLiteral("<g id=\"state\"><a xlink:href=\"#state\">%1</a></g>").arg(rect)));
        keeps("a style sheet", svgOf(QStringLiteral("<style>.r { fill: red }</style><rect class=\"r\" width=\"10\" height=\"10\"/>")));
        QCOMPARE(d.imagesLeftOut, left);
    }

    // An SVG is drawn 1,200 pixels on its long side at the least, and, whatever
    // size it asks for, no more than 4,096 on a side or 16 million pixels in all.
    void svgPicturesAreDrawnWithinASizeCap()
    {
        Document d;
        const QString rect = QStringLiteral("<rect width=\"10\" height=\"10\" fill=\"red\"/>");
        const QString small = d.addImage(svgOf(rect, QStringLiteral("width=\"400\" height=\"300\" viewBox=\"0 0 400 300\"")), QStringLiteral("svg"));
        QCOMPARE(d.image(small).size(), QSize(1200, 900));
        QCOMPARE(d.images.value(small).pixelSize, QSize(1200, 900));
        const QString wide = d.addImage(svgOf(rect, QStringLiteral("width=\"20000\" height=\"2000\" viewBox=\"0 0 20000 2000\"")), QStringLiteral("svg"));
        QVERIFY(!wide.isEmpty());
        const QImage img = d.image(wide);
        QVERIFY(!img.isNull());
        QVERIFY(img.width() <= 4096 && img.height() <= 4096);
        QVERIFY(qint64(img.width()) * img.height() <= 16'000'000);
        QVERIFY(std::abs(double(img.width()) / img.height() - 10.0) < 0.1);
        QCOMPARE(d.images.value(wide).pixelSize, img.size());
        // A frame of a hundred thousand points by ten thousand (a chart, as an SVG) takes no more.
        const QString huge = d.addImage(svgOf(rect, QStringLiteral("width=\"100000\" height=\"10000\" viewBox=\"0 0 100000 10000\"")), QStringLiteral("svg"));
        QVERIFY(!huge.isEmpty());
        QCOMPARE(d.image(huge).size(), d.images.value(huge).pixelSize);
        QVERIFY(d.image(huge).width() <= 4096);
        // A thin one keeps at least a pixel.
        const QString thin = d.addImage(svgOf(rect, QStringLiteral("width=\"1000000\" height=\"85\" viewBox=\"0 0 1000000 85\"")), QStringLiteral("svg"));
        QVERIFY(!thin.isEmpty());
        QVERIFY(d.image(thin).width() <= 4096 && d.image(thin).height() >= 1);
    }

    // A slide's picture that is an SVG which would run for minutes, or one that
    // reads a file beside the presentation, is left out when the file opens.
    void importLeavesOutHostileSvgPictures()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QImage secret(4, 4, QImage::Format_RGB32);
        secret.fill(Qt::red);
        const QString secretFile = dir.filePath(QStringLiteral("secret.png"));
        QVERIFY(secret.save(secretFile));
        const QList<QByteArray> hostile = {svgOf(useBomb(26)),
                                           svgOf(QStringLiteral("<image xlink:href=\"%1\" width=\"10\" height=\"10\"/>").arg(secretFile))};
        for (const QByteArray &svg : hostile) {
            Parts p = samplePptx();
            p.data["/ppt/media/image1.png"] = {QStringLiteral("image/png"), svg};
            QString err;
            std::unique_ptr<Document> d;
            const qint64 ms = bestNanoseconds([&] { d = deck::importPresentation(pack(p), &err); }, 5, [&] {
                d.reset();
                err.clear();
            }) / 1000000;
            QVERIFY2(d, qPrintable(err));
            QVERIFY2(ms < 5000, qPrintable(QStringLiteral("took %1 ms").arg(ms)));
            QVERIFY(d->images.isEmpty());
            QVERIFY(d->imagesLeftOut >= 1);
            QVERIFY2(err.contains(QStringLiteral("picture")), qPrintable(err));
        }
    }

private:
    static QImage grayImage(int w, int h, int shade)
    {
        QImage img(w, h, QImage::Format_RGB32);
        img.fill(QColor(shade, shade, shade));
        return img;
    }
    static QByteArray grayPng(int w, int h, int shade) { return imageBytes(grayImage(w, h, shade)); }
    static QByteArray svgOf(const QString &body, const QString &attrs = QStringLiteral("width=\"100\" height=\"50\" viewBox=\"0 0 100 50\""))
    {
        return QStringLiteral("<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" %1>%2</svg>").arg(attrs, body).toUtf8();
    }
    // Levels of two uses of the level below, then a use of the top one: 2^levels parts to draw from a few lines.
    static QString useBomb(int levels)
    {
        QString s = QStringLiteral("<defs><g id=\"l0\"><rect width=\"1\" height=\"1\"/></g>");
        for (int i = 1; i <= levels; ++i)
            s += QStringLiteral("<g id=\"l%1\"><use xlink:href=\"#l%2\"/><use xlink:href=\"#l%2\"/></g>").arg(i).arg(i - 1);
        return s + QStringLiteral("</defs><use xlink:href=\"#l%1\"/>").arg(levels);
    }
    // A metafile of 150 by 75 points: the 88-byte header, then the end record.
    static QByteArray emfBytes()
    {
        QByteArray e(88, '\0');
        auto put = [&](int at, qint32 v) {
            for (int i = 0; i < 4; ++i) e[at + i] = char((quint32(v) >> (8 * i)) & 0xFF);
        };
        put(0, 1);
        put(4, 88);
        put(16, 199);
        put(20, 99);
        put(32, 5290);   // the frame, in hundredths of a millimeter
        put(36, 2646);
        put(40, 0x464D4520);
        put(72, 1920);   // the device, in pixels and in millimeters
        put(76, 1080);
        put(80, 508);
        put(84, 286);
        QByteArray end(20, '\0');
        end[0] = 14;
        end[4] = 20;
        end[12] = 16;
        end[16] = 20;
        return e + end;
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

    // At most 1,000 layouts are read, however many a file lists; and a slide it lists again and again is read once (a package holds too few
    // parts for 10,000 slides of their own, which is the most that are read).
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
            QCOMPARE(int(d->pages.size()), 1);
            QVERIFY2(err.contains(QStringLiteral("Slide 2")), qPrintable(err));
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

    // A part the file lists again is read once: a slide listed twice is one slide, and so is a master, however often it is named.
    void importReadsEachPartOnce()
    {
        const QString shape = QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"Box %1\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr>%2<a:solidFill><a:srgbClr val=\"336699\"/></a:solidFill></p:spPr></p:sp>");
        auto replaceList = [](Parts &p, const QString &list, const QString &ids) {
            QString xml = QString::fromUtf8(p.data["/ppt/presentation.xml"].xml);
            xml.replace(QRegularExpression(QStringLiteral("<p:%1>.*</p:%1>").arg(list)), QStringLiteral("<p:%1>%2</p:%1>").arg(list, ids));
            p.data["/ppt/presentation.xml"].xml = xml.toUtf8();
        };
        // The same two slides, named five times.
        {
            Parts p = samplePptx();
            replaceList(p, QStringLiteral("sldIdLst"), QStringLiteral("<p:sldId id=\"256\" r:id=\"rId2\"/><p:sldId id=\"257\" r:id=\"rId3\"/><p:sldId id=\"258\" r:id=\"rId2\"/>"
                                                                      "<p:sldId id=\"259\" r:id=\"rId3\"/><p:sldId id=\"260\" r:id=\"rId2\"/>"));
            QString err;
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(d->pages.size()), 2);
            QVERIFY2(err.contains(QStringLiteral("Slide 3")), qPrintable(err));
            QCOMPARE(textOf(*d, *d->pages[0]->items[0]), QStringLiteral("Quarterly results"));
        }
        // A slide of two thousand shapes named a hundred times is read once, not two hundred thousand.
        {
            Parts p = samplePptx();
            QString shapes;
            for (int i = 0; i < 2000; ++i) shapes += shape.arg(i + 2).arg(xfrm(i * 1000, 0, 5000, 5000));
            QString ids;
            for (int i = 0; i < 100; ++i) ids += QStringLiteral("<p:sldId id=\"%1\" r:id=\"rId5\"/>").arg(256 + i);
            replaceList(p, QStringLiteral("sldIdLst"), ids);
            p.data["/ppt/slides/big.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide+xml"), slideXml(QString(), QString(), shapes).toUtf8()};
            p.rels << Rel{"/ppt/presentation.xml", "slide", "/ppt/slides/big.xml"}
                   << Rel{"/ppt/slides/big.xml", "slideLayout", "/ppt/slideLayouts/slideLayout2.xml"};
            QString err;
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(d->pages.size()), 1);
            QCOMPARE(int(d->pages[0]->items.size()), 2000);
            QVERIFY2(err.contains(QStringLiteral("Slide 2")), qPrintable(err));
            QVERIFY2(err.contains(QStringLiteral("And 79 more")), qPrintable(err));   // 99 listings: 19 told, the rest counted
        }
        // A layout named three times by its master is one layout.
        {
            Parts p = samplePptx();
            QString xml = QString::fromUtf8(p.data["/ppt/slideMasters/slideMaster1.xml"].xml);
            const QString once = QStringLiteral("<p:sldLayoutId id=\"2147483649\" r:id=\"rId1\"/>");
            QVERIFY(xml.contains(once));
            xml.replace(once, once + once + once);
            p.data["/ppt/slideMasters/slideMaster1.xml"].xml = xml.toUtf8();
            QString err;
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(deck::layouts(*d).size()), 2);
            QCOMPARE(int(d->pages.size()), 2);
        }
        // A slide master named four times is one master, with its two layouts once.
        {
            Parts p = samplePptx();
            QString xml = QString::fromUtf8(p.data["/ppt/presentation.xml"].xml);
            const QString once = QStringLiteral("<p:sldMasterId id=\"2147483648\" r:id=\"rId1\"/>");
            QVERIFY(xml.contains(once));
            xml.replace(once, once + once + once + once);
            p.data["/ppt/presentation.xml"].xml = xml.toUtf8();
            QString err;
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(d->masters.size()), 3);   // the master, and its two layouts
            QCOMPARE(int(deck::layouts(*d).size()), 2);
            QVERIFY2(err.contains(QStringLiteral("Test Master")) || err.contains(QStringLiteral("slideMaster1")), qPrintable(err));
            QCOMPARE(int(d->pages.size()), 2);
        }
    }

    // A theme part is read once however many masters name it, and one too large to be a theme is not read at all.
    void importReadsEachThemeOnceAndHoldsItsSize()
    {
        auto masterWithoutLayouts = [] {
            QString xml = masterXml();
            xml.replace(QRegularExpression(QStringLiteral("<p:sldLayoutIdLst>.*</p:sldLayoutIdLst>")), QString());
            return xml;
        };
        const QString masterType = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml");
        // Three masters of their own, over one theme part: each keeps the theme, and the one theme is the presentation's.
        {
            Parts p = samplePptx();
            QString xml = QString::fromUtf8(p.data["/ppt/presentation.xml"].xml);
            QString ids = QStringLiteral("<p:sldMasterId id=\"2147483648\" r:id=\"rId1\"/>");
            for (int i = 2; i <= 3; ++i) {
                const QString name = QStringLiteral("/ppt/slideMasters/slideMaster%1.xml").arg(i);
                p.data[name] = {masterType, masterWithoutLayouts().toUtf8()};
                p.rels << Rel{"/ppt/presentation.xml", "slideMaster", name} << Rel{name, "theme", "/ppt/theme/theme1.xml"};
                ids += QStringLiteral("<p:sldMasterId id=\"%1\" r:id=\"rId%2\"/>").arg(2147483700LL + i).arg(3 + i);   // the presentation's links so far are four
            }
            xml.replace(QRegularExpression(QStringLiteral("<p:sldMasterIdLst>.*</p:sldMasterIdLst>")), QStringLiteral("<p:sldMasterIdLst>") + ids + QStringLiteral("</p:sldMasterIdLst>"));
            p.data["/ppt/presentation.xml"].xml = xml.toUtf8();
            QString err;
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QVERIFY2(err.isEmpty() || !err.contains(QStringLiteral("theme")), qPrintable(err));
            QVector<const MasterPage *> masters;
            for (const auto &m : d->masters)
                if (m->props.value(QStringLiteral("kind")).toString() == QLatin1String("master")) masters << m.get();
            QCOMPARE(int(masters.size()), 3);
            QVERIFY(!masters[1]->props.value(QStringLiteral("themeXml")).toString().isEmpty());
            QCOMPARE(masters[2]->props.value(QStringLiteral("themeXml")), masters[1]->props.value(QStringLiteral("themeXml")));
            QCOMPARE(QByteArray::fromBase64(masters[1]->props.value(QStringLiteral("themeXml")).toString().toLatin1()), themeXml());
        }
        // A theme of a megabyte (real ones are under ten kilobytes): the master takes the stand-in colors and fonts, and the rest of the file is read.
        {
            Parts p = samplePptx();
            QByteArray theme = themeXml();
            theme.replace("</a:themeElements>", "</a:themeElements><a:extLst><a:ext uri=\"{padding}\">" + QByteArray(1 << 20, 'x') + "</a:ext></a:extLst>");
            p.data["/ppt/theme/theme1.xml"].xml = theme;
            QString err;
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(d->pages.size()), 2);
            QVERIFY2(err.contains(QStringLiteral("theme")), qPrintable(err));
            QVERIFY(d->appData.value(QStringLiteral("themeXml")).toString().isEmpty());
        }
    }

    // At most 200 slide masters are read (real files have a few), with a note, and the slides and layouts of the first are all there.
    void importLimitsMasters()
    {
        auto masterWithoutLayouts = [] {
            QString xml = masterXml();
            xml.replace(QRegularExpression(QStringLiteral("<p:sldLayoutIdLst>.*</p:sldLayoutIdLst>")), QString());
            return xml;
        };
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/presentation.xml"].xml);
        QString ids = QStringLiteral("<p:sldMasterId id=\"2147483648\" r:id=\"rId1\"/>");
        for (int i = 2; i <= 230; ++i) {
            const QString name = QStringLiteral("/ppt/slideMasters/slideMaster%1.xml").arg(i);
            p.data[name] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"), masterWithoutLayouts().toUtf8()};
            p.rels << Rel{"/ppt/presentation.xml", "slideMaster", name} << Rel{name, "theme", "/ppt/theme/theme1.xml"};
            ids += QStringLiteral("<p:sldMasterId id=\"%1\" r:id=\"rId%2\"/>").arg(2147483700LL + i).arg(3 + i);   // the presentation's links so far are four
        }
        xml.replace(QRegularExpression(QStringLiteral("<p:sldMasterIdLst>.*</p:sldMasterIdLst>")), QStringLiteral("<p:sldMasterIdLst>") + ids + QStringLiteral("</p:sldMasterIdLst>"));
        p.data["/ppt/presentation.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY(d);
        int masters = 0;
        for (const auto &m : d->masters)
            if (m->props.value(QStringLiteral("kind")).toString() == QLatin1String("master")) ++masters;
        QCOMPARE(masters, 200);
        QVERIFY2(err.contains(QStringLiteral("200")), qPrintable(err));
        QCOMPARE(int(d->pages.size()), 2);
        QCOMPARE(d->pages[0]->masterId, QStringLiteral("L2"));
    }

    // Every page of an import draws from one budget of objects: three slides of 70,000 shapes keep 200,000 between them.
    void importHoldsAllPagesToOneObjectBudget()
    {
        Parts p = samplePptx();
        QString shapes;
        const QString shape = QStringLiteral("<p:sp/>");
        for (int i = 0; i < 5000; ++i) shapes += shape;
        QString ids;
        for (int s = 1; s <= 45; ++s) {
            const QString name = QStringLiteral("/ppt/slides/many%1.xml").arg(s);
            p.data[name] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide+xml"), slideXml(QString(), QString(), shapes).toUtf8()};
            p.rels << Rel{"/ppt/presentation.xml", "slide", name} << Rel{name, "slideLayout", "/ppt/slideLayouts/slideLayout2.xml"};
            ids += QStringLiteral("<p:sldId id=\"%1\" r:id=\"rId%2\"/>").arg(300 + s).arg(4 + s);   // the presentation's links so far are four
        }
        QString xml = QString::fromUtf8(p.data["/ppt/presentation.xml"].xml);
        xml.replace(QRegularExpression(QStringLiteral("<p:sldIdLst>.*</p:sldIdLst>")), QStringLiteral("<p:sldIdLst>") + ids + QStringLiteral("</p:sldIdLst>"));
        p.data["/ppt/presentation.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY(d);
        QCOMPARE(int(d->pages.size()), 45);
        // The layouts' and the master's objects are the file's too.
        qsizetype items = 0;
        for (const auto &page : d->pages) items += page->items.size();
        for (const auto &master : d->masters) items += master->items.size();
        QCOMPARE(int(items), 200000);
        QVERIFY2(err.contains(QStringLiteral("200000")), qPrintable(err));
    }

    // One slide is read to what a slide can show in a moment, whatever the
    // file has left: 5,000 objects, 1,000 shapes with text, and 100,000
    // letters (real slides hold under 50 objects and a few thousand letters).
    void importHoldsEachSlideToWhatOneCanShow()
    {
        Parts p = samplePptx();
        QString plain, texts, few;
        for (int i = 0; i < 6000; ++i) plain += QStringLiteral("<p:sp/>");
        for (int i = 0; i < 1200; ++i)
            texts += QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"t\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr/>"
                                    "<p:txBody><a:bodyPr/><a:p><a:r><a:t>x</a:t></a:r></a:p></p:txBody></p:sp>").arg(i + 10);
        for (int i = 0; i < 10; ++i) few += QStringLiteral("<p:sp/>");
        QString ids;
        int s = 0;
        for (const QString &tree : {plain, texts, few}) {
            ++s;
            const QString name = QStringLiteral("/ppt/slides/dense%1.xml").arg(s);
            p.data[name] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide+xml"), slideXml(QString(), QString(), tree).toUtf8()};
            p.rels << Rel{"/ppt/presentation.xml", "slide", name} << Rel{name, "slideLayout", "/ppt/slideLayouts/slideLayout2.xml"};
            ids += QStringLiteral("<p:sldId id=\"%1\" r:id=\"rId%2\"/>").arg(300 + s).arg(4 + s);
        }
        QString xml = QString::fromUtf8(p.data["/ppt/presentation.xml"].xml);
        xml.replace(QRegularExpression(QStringLiteral("<p:sldIdLst>.*</p:sldIdLst>")), QStringLiteral("<p:sldIdLst>") + ids + QStringLiteral("</p:sldIdLst>"));
        p.data["/ppt/presentation.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY(d);
        QCOMPARE(int(d->pages.size()), 3);
        QCOMPARE(int(d->pages[0]->items.size()), 5000);
        QCOMPARE(int(d->pages[1]->items.size()), 1000);
        QCOMPARE(int(d->pages[2]->items.size()), 10);
        QVERIFY2(err.contains(QStringLiteral("A slide is read to")), qPrintable(err));
    }

    // A comment's time zone is held to a day either side of UTC, and a huge one doesn't overflow the seconds it is added as.
    void importHoldsACommentsTimeZoneBiasToADay()
    {
        for (const char *bias : {"9223372036854775807", "-9223372036854775808"}) {
            Parts p = withLegacyComments();
            QString body = legacyComment(0, 1, QStringLiteral("2013-05-10T10:21:50.343"), 5760, 1152, QStringLiteral("Far away"), 0);
            body.replace(QStringLiteral("timeZoneBias=\"0\""), QStringLiteral("timeZoneBias=\"%1\"").arg(QLatin1String(bias)));
            p.data["/ppt/comments/comment1.xml"].xml = legacyCommentList(body).toUtf8();
            QString err;
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            const QVector<deck::Comment> list = deck::commentsOf(*d->pages[0]);
            QCOMPARE(int(list.size()), 1);
            const bool east = bias[0] != '-';
            const QDateTime expected(QDate(2013, 5, east ? 11 : 9), QTime(10, 21, 50, 343), QTimeZone::UTC);
            QCOMPARE(list[0].date, expected);
        }
    }

    // A notes part or a comments part is read for one slide: a second slide that names it has none of its own, and a slide that names it twice reads it once.
    void importReadsNotesAndCommentsOnce()
    {
        {
            Parts p = samplePptx();
            p.rels << Rel{"/ppt/slides/slide2.xml", "notesSlide", "/ppt/notesSlides/notesSlide1.xml"};
            QString err;
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(deck::notesText(*d, *d->pages[0]), QStringLiteral("Say hello first\nThen the numbers"));
            QVERIFY(deck::notesText(*d, *d->pages[1]).isEmpty());
            QVERIFY2(err.contains(QStringLiteral("slide2")), qPrintable(err));
        }
        {
            Parts p = withLegacyComments();
            p.rels << Rel{"/ppt/slides/slide1.xml", "comments", "/ppt/comments/comment1.xml"}
                   << Rel{"/ppt/slides/slide2.xml", "comments", "/ppt/comments/comment1.xml"}
                   << Rel{"/ppt/presentation.xml", "commentAuthors", "/ppt/commentAuthors.xml"};
            QString err;
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(deck::commentsOf(*d->pages[0]).size()), 2);   // not four
            QVERIFY(deck::commentsOf(*d->pages[1]).isEmpty());
            QVERIFY2(err.contains(QStringLiteral("comments")), qPrintable(err));
        }
    }

    // A slide's placeholders find their layout's by lookup, not by going over all of the layout's for each of the slide's
    // (at the most a page keeps: 1,000 shapes with text against 5,000 objects).
    void importMatchesPlaceholdersByLookup()
    {
        Parts p = samplePptx();
        QString layoutShapes, slideShapes;
        for (int i = 0; i < 5000; ++i)
            layoutShapes += QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"P\"/><p:cNvSpPr/><p:nvPr><p:ph type=\"pic\" idx=\"%1\"/></p:nvPr></p:nvSpPr><p:spPr/></p:sp>").arg(i + 10);
        for (int i = 0; i < 1000; ++i)
            slideShapes += QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"Q\"/><p:cNvSpPr/><p:nvPr><p:ph type=\"x%1\" idx=\"%2\"/></p:nvPr></p:nvSpPr><p:spPr/></p:sp>").arg(i + 2).arg(500000 + i);
        p.data["/ppt/slideLayouts/slideLayout2.xml"].xml = layoutXml(QStringLiteral("obj"), QStringLiteral("Wide"), QString(), layoutShapes).toUtf8();
        p.data["/ppt/slides/slide1.xml"].xml = slideXml(QString(), QString(), slideShapes).toUtf8();
        QString err;
        std::unique_ptr<Document> d;
        const qint64 ms = bestNanoseconds([&] { d = deck::importPresentation(pack(p), &err); }, 3, [&] {
            d.reset();
            err.clear();
        }) / 1000000;
        QVERIFY(d);
        QVERIFY2(!d->pages.empty() && d->pages[0]->items.size() == 1000, qPrintable(err));
        QVERIFY2(ms < 6000, qPrintable(QStringLiteral("took %1 ms").arg(ms)));
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
};

JP_TEST_CLASS(DeckPptxImportTests)
#include "tst_deck_pptximport.moc"
