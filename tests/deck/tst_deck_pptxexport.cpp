// JeffDeck's tests of writing a presentation file.
#include "deckhelpers.h"

class DeckPptxExportTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

private:

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
        // The theme's font is the standard font a new presentation is drawn to (Calibri).
        QCOMPARE(deck::themeOf(*back).headingFont, QStringLiteral("Calibri"));
        QCOMPARE(deck::themeOf(*back).bodyFont, QStringLiteral("Calibri"));
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
        // The transition stays, without its sound.
        QVERIFY(slide.contains(QStringLiteral("<p:fade/>")) && !slide.contains(QStringLiteral("sndAc")));
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
};

JP_TEST_CLASS(DeckPptxExportTests)
#include "tst_deck_pptxexport.moc"
