// JeffDeck's tests of the slide show: settings, custom shows, ink, and rehearsal.
#include "deckhelpers.h"

class DeckSlideShowTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

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

    // The slide show's settings are the presentation's own: the defaults leave no key, what a person sets comes back whole, and a value that
    // makes no sense takes its default.
    void showSettingsAreKept()
    {
        auto d = lettered(3);
        QVERIFY(!d->appData.contains(QStringLiteral("showSettings")));
        QVERIFY(deck::showSettingsOf(*d) == deck::ShowSettings());
        deck::ShowSettings s;
        s.type = deck::ShowSettings::Type::Kiosk;
        s.slides = deck::ShowSettings::Slides::Range;
        s.from = 2;
        s.to = 3;
        s.loop = true;
        s.animation = false;
        s.useTimings = false;
        s.penColor = QColor(QStringLiteral("#2060c0"));
        s.laserColor = QColor(QStringLiteral("#00ff00"));
        deck::setShowSettings(*d, s);
        QVERIFY(deck::showSettingsOf(*d) == s);
        QVERIFY(s.looping());
        QVERIFY(s.usesTimings());   // a kiosk has nothing else to move it on
        deck::setShowSettings(*d, deck::ShowSettings());
        QVERIFY(!d->appData.contains(QStringLiteral("showSettings")));
        QVERIFY(!deck::ShowSettings().looping());
        // Nonsense takes the defaults: no show is "custom" without one, a color that isn't one is red.
        const deck::ShowSettings odd = deck::ShowSettings::fromJson({{"type", "spin"}, {"slides", "custom"}, {"from", -4}, {"to", "x"}, {"pen", "nope"}, {"loop", 3}});
        QVERIFY(odd == deck::ShowSettings());
    }

    // A custom show lists slides by their pages, in any order, a slide twice if it likes. A slide taken from the presentation leaves
    // its shows, and a show left with none is gone.
    void customShowsFollowTheSlides()
    {
        auto d = lettered(5);
        deck::CustomShow a, b, c;
        a.id = 7;
        a.name = QStringLiteral("Short");
        a.slides = {d->pages[3]->id, d->pages[0]->id, d->pages[3]->id};
        b.id = 7;   // an id twice: the second gets a new one
        b.name = QStringLiteral("Other");
        b.slides = {d->pages[1]->id};
        c.id = 2;
        c.name = QStringLiteral("Stray");
        c.slides = {QStringLiteral("not a page")};
        deck::setCustomShows(*d, {a, b, c});
        QVector<deck::CustomShow> shows = deck::customShowsOf(*d);
        QCOMPARE(shows.size(), 2);   // the one with no slide that exists is not a show
        QCOMPARE(shows[0].id, qint64(7));
        QVERIFY(shows[1].id != 7);
        QCOMPARE(deck::customShowSlides(*d, shows[0]), (QList<int>{3, 0, 3}));
        QVERIFY(deck::customShowById(shows, shows[1].id));
        QVERIFY(!deck::customShowById(shows, 99));
        QCOMPARE(deck::newCustomShowName(shows), QStringLiteral("Custom Show 1"));
        QVERIFY(deck::newCustomShowId(shows) > shows[1].id);
        // Plans: a custom show plays the slides it lists, hidden ones too; the others leave hidden slides to the show.
        d->pages[3]->hidden = true;
        const deck::ShowPlan plan = deck::planOfCustomShow(*d, shows[0]);
        QCOMPARE(plan.slides, (QList<int>{3, 0, 3}));
        QVERIFY(plan.hidden);
        deck::ShowSettings s;
        QVERIFY(deck::planOf(*d, s).slides.isEmpty());   // every slide
        s.slides = deck::ShowSettings::Slides::Range;
        s.from = 2;
        s.to = 4;
        QCOMPARE(deck::planOf(*d, s).slides, (QList<int>{1, 2, 3}));
        QVERIFY(!deck::planOf(*d, s).hidden);
        QVERIFY(deck::planOf(*d, s, false).slides.isEmpty());   // From Current Slide plays every slide
        s.to = 99;   // past the last slide is the last slide
        QCOMPARE(deck::planOf(*d, s).slides, (QList<int>{1, 2, 3, 4}));
        s.from = 4;
        s.to = 2;   // nothing between: every slide
        QVERIFY(deck::planOf(*d, s).slides.isEmpty());
        s.slides = deck::ShowSettings::Slides::Custom;
        s.customShow = 7;
        QCOMPARE(deck::planOf(*d, s).slides, (QList<int>{3, 0, 3}));
        // Slide D goes: the show keeps the others, in their order.
        d->pages.removeAt(3);
        shows = deck::customShowsOf(*d);
        QCOMPARE(shows.size(), 2);
        QCOMPARE(deck::customShowSlides(*d, shows[0]), (QList<int>{0}));
        QCOMPARE(shows[0].slides.size(), 1);
        QCOMPARE(deck::planOf(*d, s).slides, (QList<int>{0}));
        // Slide A goes too: Short has nothing left and is gone, and a presentation set to play it plays every slide.
        d->pages.removeAt(0);
        shows = deck::customShowsOf(*d);
        QCOMPARE(shows.size(), 1);
        QCOMPARE(shows[0].name, QStringLiteral("Other"));
        QVERIFY(deck::planOf(*d, s).slides.isEmpty());
    }

    // The settings are p:showPr in the presentation properties, in the order the schema has: the kind of show, which slides, the pen's
    // color, then the extensions with the laser pointer's color. They come back as set, and a show with defaults writes none.
    void showSettingsAreWrittenAndReadBack()
    {
        auto d = lettered(4);
        deck::ShowSettings s;
        s.type = deck::ShowSettings::Type::Kiosk;
        s.slides = deck::ShowSettings::Slides::Range;
        s.from = 2;
        s.to = 3;
        s.loop = true;
        s.animation = false;
        s.useTimings = false;
        s.penColor = QColor(QStringLiteral("#2060c0"));
        s.laserColor = QColor(QStringLiteral("#00ff00"));
        s.kioskRestart = 60000;
        deck::setShowSettings(*d, s);
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QDomDocument dom;
        const QDomElement root = rootOf(e.zip.value(QStringLiteral("ppt/presProps.xml")), &dom);
        QCOMPARE(root.localName(), QStringLiteral("presentationPr"));
        const QDomElement show = office::child(root, office::ns::P, QStringLiteral("showPr"));
        QVERIFY(!show.isNull());
        QCOMPARE(show.attribute(QStringLiteral("loop")), QStringLiteral("1"));
        QCOMPARE(show.attribute(QStringLiteral("showAnimation")), QStringLiteral("0"));
        QCOMPARE(show.attribute(QStringLiteral("useTimings")), QStringLiteral("0"));
        QVERIFY(!show.hasAttribute(QStringLiteral("showNarration")));   // the default is not written
        QStringList order;
        for (const QDomElement &c : office::children(show)) order << c.localName();
        QCOMPARE(order, (QStringList{"kiosk", "sldRg", "penClr", "extLst"}));
        QCOMPARE(office::child(show, office::ns::P, QStringLiteral("kiosk")).attribute(QStringLiteral("restart")), QStringLiteral("60000"));
        const QDomElement range = office::child(show, office::ns::P, QStringLiteral("sldRg"));
        QCOMPARE(range.attribute(QStringLiteral("st")), QStringLiteral("2"));
        QCOMPARE(range.attribute(QStringLiteral("end")), QStringLiteral("3"));
        QCOMPARE(office::child(office::child(show, office::ns::P, QStringLiteral("penClr")), office::ns::A, QStringLiteral("srgbClr")).attribute(QStringLiteral("val")),
                 QStringLiteral("2060C0"));
        const QString p14 = QStringLiteral("http://schemas.microsoft.com/office/powerpoint/2010/main");
        const QDomElement ext = office::child(office::child(show, office::ns::P, QStringLiteral("extLst")), office::ns::P, QStringLiteral("ext"));
        QCOMPARE(ext.attribute(QStringLiteral("uri")), QStringLiteral("{EC167BDD-8182-4AB7-AECC-EB403E3ABB37}"));
        QCOMPARE(office::child(office::child(ext, p14, QStringLiteral("laserClr")), office::ns::A, QStringLiteral("srgbClr")).attribute(QStringLiteral("val")),
                 QStringLiteral("00FF00"));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(deck::showSettingsOf(*back) == s);
        // Saved again, the file is the same.
        Exported again;
        QVERIFY2(unpack(deck::exportPresentation(*back, &err), &again, &err), qPrintable(err));
        QCOMPARE(again.zip.value(QStringLiteral("ppt/presProps.xml")), e.zip.value(QStringLiteral("ppt/presProps.xml")));
        // A speaker's show, and a show in a window; narration, the scroll bar, and a show that plays a custom show.
        auto other = lettered(3);
        deck::CustomShow c;
        c.id = 4;
        c.name = QStringLiteral("Quick");
        c.slides = {other->pages[2]->id};
        deck::setCustomShows(*other, {c});
        deck::ShowSettings t;
        t.type = deck::ShowSettings::Type::Individual;
        t.scrollbar = false;
        t.narration = true;
        t.slides = deck::ShowSettings::Slides::Custom;
        t.customShow = 4;
        deck::setShowSettings(*other, t);
        Exported second;
        QVERIFY2(unpack(deck::exportPresentation(*other, &err), &second, &err), qPrintable(err));
        const QDomElement browsed = office::child(rootOf(second.zip.value(QStringLiteral("ppt/presProps.xml")), &dom), office::ns::P, QStringLiteral("showPr"));
        QCOMPARE(browsed.attribute(QStringLiteral("showNarration")), QStringLiteral("1"));
        QCOMPARE(office::child(browsed, office::ns::P, QStringLiteral("browse")).attribute(QStringLiteral("showScrollbar")), QStringLiteral("0"));
        QCOMPARE(office::child(browsed, office::ns::P, QStringLiteral("custShow")).attribute(QStringLiteral("id")), QStringLiteral("4"));
        auto otherBack = deck::importPresentation(deck::exportPresentation(*other, &err), &err);
        QVERIFY2(otherBack, qPrintable(err));
        QVERIFY(deck::showSettingsOf(*otherBack) == t);
        // Defaults write no show properties.
        auto plain = lettered(2);
        Exported none;
        QVERIFY2(unpack(deck::exportPresentation(*plain, &err), &none, &err), qPrintable(err));
        QVERIFY(!part(none, QStringLiteral("/ppt/presProps.xml")).contains(QStringLiteral("showPr")));
        QVERIFY(!part(none, QStringLiteral("/ppt/presentation.xml")).contains(QStringLiteral("custShow")));
    }

    // The custom shows are p:custShowLst in the presentation, after the notes size and before the default text style, each show's
    // slides named by the relationship the presentation has to them; they come back with their slides in order, a slide twice too.
    void customShowsAreWrittenAndReadBack()
    {
        auto d = lettered(4);
        deck::CustomShow a, b;
        a.id = 0;
        a.name = QStringLiteral("Q & \"A\"");
        a.slides = {d->pages[2]->id, d->pages[0]->id, d->pages[2]->id};
        b.id = 5;
        b.name = QStringLiteral("Closing");
        b.slides = {d->pages[3]->id};
        deck::setCustomShows(*d, {a, b});
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QDomDocument dom;
        const QDomElement root = rootOf(e.zip.value(QStringLiteral("ppt/presentation.xml")), &dom);
        QStringList order;
        for (const QDomElement &c : office::children(root)) order << c.localName();
        const int sizes = int(order.indexOf(QStringLiteral("notesSz")));
        QVERIFY(sizes >= 0);
        QCOMPARE(order.mid(sizes), (QStringList{"notesSz", "custShowLst", "defaultTextStyle"}));
        QStringList rels;
        for (const QDomElement &s : office::children(office::child(root, office::ns::P, QStringLiteral("sldIdLst")), office::ns::P, QStringLiteral("sldId")))
            rels << office::attrNs(s, office::ns::R, QStringLiteral("id"));
        QCOMPARE(int(rels.size()), 4);
        const QVector<QDomElement> written = office::children(office::child(root, office::ns::P, QStringLiteral("custShowLst")), office::ns::P, QStringLiteral("custShow"));
        QCOMPARE(int(written.size()), 2);
        QCOMPARE(written[0].attribute(QStringLiteral("name")), a.name);
        QCOMPARE(written[0].attribute(QStringLiteral("id")), QStringLiteral("0"));
        QCOMPARE(written[1].attribute(QStringLiteral("id")), QStringLiteral("5"));
        QStringList listed;
        for (const QDomElement &s : office::children(office::child(written[0], office::ns::P, QStringLiteral("sldLst")), office::ns::P, QStringLiteral("sld")))
            listed << office::attrNs(s, office::ns::R, QStringLiteral("id"));
        QCOMPARE(listed, (QStringList{rels[2], rels[0], rels[2]}));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const QVector<deck::CustomShow> shows = deck::customShowsOf(*back);
        QCOMPARE(shows.size(), 2);
        QCOMPARE(shows[0].name, a.name);
        QCOMPARE(shows[0].id, qint64(0));
        QCOMPARE(shows[1].id, qint64(5));
        QCOMPARE(deck::customShowSlides(*back, shows[0]), (QList<int>{2, 0, 2}));
        QCOMPARE(deck::customShowSlides(*back, shows[1]), (QList<int>{3}));
        Exported again;
        QVERIFY2(unpack(deck::exportPresentation(*back, &err), &again, &err), qPrintable(err));
        QCOMPARE(again.zip.value(QStringLiteral("ppt/presentation.xml")), e.zip.value(QStringLiteral("ppt/presentation.xml")));
    }

    // A file the presentation program wrote: show properties with its extensions, custom shows, and a show set to play one read in;
    // what JeffDeck has no form for (the recent colors, the media controls, the image settings) is written back as it was.
    void showSettingsFromTheProgramAreRead()
    {
        const QString p14 = QStringLiteral("http://schemas.microsoft.com/office/powerpoint/2010/main");
        auto d = lettered(4);
        QString err;
        const QByteArray plain = deck::exportPresentation(*d, &err);
        Exported base;
        QVERIFY2(unpack(plain, &base, &err), qPrintable(err));
        QDomDocument dom;
        QStringList rels;
        for (const QDomElement &s : office::children(office::child(rootOf(base.zip.value(QStringLiteral("ppt/presentation.xml")), &dom), office::ns::P, QStringLiteral("sldIdLst")), office::ns::P,
                                                     QStringLiteral("sldId")))
            rels << office::attrNs(s, office::ns::R, QStringLiteral("id"));
        QCOMPARE(int(rels.size()), 4);
        const QString namespaces = QStringLiteral("xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
                                                  "xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"");
        auto written = [&](const QString &props, const QString &shows) {
            QMap<QString, QByteArray> zip = base.zip;
            zip[QStringLiteral("ppt/presProps.xml")] = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<p:presentationPr %1>%2</p:presentationPr>").arg(namespaces, props).toUtf8();
            QString xml = QString::fromUtf8(zip.value(QStringLiteral("ppt/presentation.xml")));
            const qsizetype at = xml.indexOf(QStringLiteral("<p:defaultTextStyle>"));
            xml.insert(at, shows);
            zip[QStringLiteral("ppt/presentation.xml")] = xml.toUtf8();
            ZipWriter z;
            for (auto it = zip.cbegin(); it != zip.cend(); ++it) z.add(it.key(), it.value(), true);
            return z.finish();
        };
        const QString shows = QStringLiteral("<p:custShowLst><p:custShow name=\"Tour\" id=\"2\"><p:sldLst><p:sld r:id=\"%1\"/><p:sld r:id=\"%2\"/></p:sldLst></p:custShow>"
                                             "<p:custShow name=\"Lost\" id=\"3\"><p:sldLst><p:sld r:id=\"rId999\"/></p:sldLst></p:custShow></p:custShowLst>")
                                  .arg(rels[3], rels[1]);
        const QString props = QStringLiteral("<p:showPr loop=\"1\" showNarration=\"1\" showAnimation=\"0\" useTimings=\"0\"><p:browse showScrollbar=\"0\"/><p:custShow id=\"2\"/>"
                                             "<p:penClr><a:srgbClr val=\"0070C0\"/></p:penClr><p:extLst>"
                                             "<p:ext uri=\"{EC167BDD-8182-4AB7-AECC-EB403E3ABB37}\"><p14:laserClr xmlns:p14=\"%1\"><a:srgbClr val=\"00FF00\"/></p14:laserClr></p:ext>"
                                             "<p:ext uri=\"{2FDB2607-1784-4EEB-B798-7EB5836EED8A}\"><p14:showMediaCtrls xmlns:p14=\"%1\" val=\"1\"/></p:ext></p:extLst></p:showPr>"
                                             "<p:clrMru><a:srgbClr val=\"FF0000\"/></p:clrMru>"
                                             "<p:extLst><p:ext uri=\"{E76CE94A-603C-4142-B9EB-6D1370010A27}\"><p14:discardImageEditData xmlns:p14=\"%1\" val=\"0\"/></p:ext>"
                                             "<p:ext uri=\"{D31A062A-798A-4329-ABDD-BBA856620510}\"><p14:defaultImageDpi xmlns:p14=\"%1\" val=\"220\"/></p:ext></p:extLst>")
                                  .arg(p14);
        const QByteArray file = written(props, shows);
        auto d2 = deck::importPresentation(file, &err);
        QVERIFY2(d2, qPrintable(err));
        const deck::ShowSettings s = deck::showSettingsOf(*d2);
        QCOMPARE(int(s.type), int(deck::ShowSettings::Type::Individual));
        QVERIFY(!s.scrollbar);
        QVERIFY(s.loop);
        QVERIFY(s.narration);
        QVERIFY(!s.animation);
        QVERIFY(!s.useTimings);
        QCOMPARE(int(s.slides), int(deck::ShowSettings::Slides::Custom));
        QCOMPARE(s.customShow, qint64(2));
        QCOMPARE(s.penColor, QColor(0x00, 0x70, 0xC0));
        QCOMPARE(s.laserColor, QColor(0x00, 0xFF, 0x00));
        const QVector<deck::CustomShow> custom = deck::customShowsOf(*d2);
        QCOMPARE(custom.size(), 1);   // the one that names a slide the file doesn't have is left out
        QCOMPARE(custom[0].name, QStringLiteral("Tour"));
        QCOMPARE(deck::customShowSlides(*d2, custom[0]), (QList<int>{3, 1}));
        QCOMPARE(deck::planOf(*d2, s).slides, (QList<int>{3, 1}));
        // Saved again, what was kept is there, in the schema's order.
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d2, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QDomElement root = rootOf(e.zip.value(QStringLiteral("ppt/presProps.xml")), &dom);
        QStringList order;
        for (const QDomElement &c : office::children(root)) order << c.localName();
        QCOMPARE(order, (QStringList{"showPr", "clrMru", "extLst"}));
        const QDomElement show = office::child(root, office::ns::P, QStringLiteral("showPr"));
        QStringList inShow;
        for (const QDomElement &c : office::children(show)) inShow << c.localName();
        QCOMPARE(inShow, (QStringList{"browse", "custShow", "penClr", "extLst"}));
        QStringList exts;
        for (const QDomElement &x : office::children(office::child(show, office::ns::P, QStringLiteral("extLst")))) exts << x.attribute(QStringLiteral("uri"));
        QCOMPARE(exts, (QStringList{"{EC167BDD-8182-4AB7-AECC-EB403E3ABB37}", "{2FDB2607-1784-4EEB-B798-7EB5836EED8A}"}));
        const QString text = part(e, QStringLiteral("/ppt/presProps.xml"));
        QVERIFY(text.contains(QStringLiteral("showMediaCtrls")));
        QVERIFY(text.contains(QStringLiteral("defaultImageDpi")));
        QVERIFY(text.contains(QStringLiteral("discardImageEditData")));
        QVERIFY(text.contains(QStringLiteral("<p:clrMru")));
        auto back = deck::importPresentation(deck::exportPresentation(*d2, &err), &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(deck::showSettingsOf(*back) == s);
        // A speaker's range, and a pen the program names by a preset color, read as such.
        auto ranged = deck::importPresentation(written(QStringLiteral("<p:showPr><p:present/><p:sldRg st=\"2\" end=\"3\"/><p:penClr><a:prstClr val=\"red\"/></p:penClr></p:showPr>"), QString()), &err);
        QVERIFY2(ranged, qPrintable(err));
        const deck::ShowSettings r = deck::showSettingsOf(*ranged);
        QCOMPARE(int(r.type), int(deck::ShowSettings::Type::Speaker));
        QCOMPARE(int(r.slides), int(deck::ShowSettings::Slides::Range));
        QCOMPARE(r.from, 2);
        QCOMPARE(r.to, 3);
        QCOMPARE(r.penColor, QColor(255, 0, 0));
        QVERIFY(r.animation && r.useTimings && !r.loop);
        // A kiosk.
        auto kiosk = deck::importPresentation(written(QStringLiteral("<p:showPr loop=\"1\" showNarration=\"1\"><p:kiosk/><p:sldAll/></p:showPr>"), QString()), &err);
        QVERIFY2(kiosk, qPrintable(err));
        QCOMPARE(int(deck::showSettingsOf(*kiosk).type), int(deck::ShowSettings::Type::Kiosk));
        QCOMPARE(deck::showSettingsOf(*kiosk).kioskRestart, qint64(300000));
        // A show set to play a custom show the file doesn't have plays every slide, and the file says so.
        auto missing = deck::importPresentation(written(QStringLiteral("<p:showPr><p:present/><p:custShow id=\"41\"/></p:showPr>"), QString()), &err);
        QVERIFY2(missing, qPrintable(err));
        QCOMPARE(int(deck::showSettingsOf(*missing).slides), int(deck::ShowSettings::Slides::All));
        QVERIFY2(err.contains(QStringLiteral("custom slide show")), qPrintable(err));
    }

    // A file may list only so many custom shows and slides in them; the rest are left out, with a note, and what is kept reads as it
    // would anywhere. Properties that can't be read leave the defaults.
    void customShowsInHostileFilesAreLimited()
    {
        auto d = lettered(2);
        QString err;
        Exported base;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &base, &err), qPrintable(err));
        QDomDocument dom;
        QStringList rels;
        for (const QDomElement &s : office::children(office::child(rootOf(base.zip.value(QStringLiteral("ppt/presentation.xml")), &dom), office::ns::P, QStringLiteral("sldIdLst")), office::ns::P,
                                                     QStringLiteral("sldId")))
            rels << office::attrNs(s, office::ns::R, QStringLiteral("id"));
        auto with = [&](const QString &shows, const QByteArray &props = QByteArray()) {
            QMap<QString, QByteArray> zip = base.zip;
            QString xml = QString::fromUtf8(zip.value(QStringLiteral("ppt/presentation.xml")));
            xml.insert(xml.indexOf(QStringLiteral("<p:defaultTextStyle>")), shows);
            zip[QStringLiteral("ppt/presentation.xml")] = xml.toUtf8();
            if (!props.isEmpty()) zip[QStringLiteral("ppt/presProps.xml")] = props;
            ZipWriter z;
            for (auto it = zip.cbegin(); it != zip.cend(); ++it) z.add(it.key(), it.value(), true);
            return z.finish();
        };
        // 1,500 shows of one slide: 1,000 are read.
        QString many = QStringLiteral("<p:custShowLst>");
        for (int i = 0; i < 1500; ++i) many += QStringLiteral("<p:custShow name=\"Show %1\" id=\"%1\"><p:sldLst><p:sld r:id=\"%2\"/></p:sldLst></p:custShow>").arg(i).arg(rels[0]);
        many += QStringLiteral("</p:custShowLst>");
        auto d1 = deck::importPresentation(with(many), &err);
        QVERIFY2(d1, qPrintable(err));
        QCOMPARE(int(deck::customShowsOf(*d1).size()), 1000);
        QVERIFY2(err.contains(QStringLiteral("1000")), qPrintable(err));
        // One show that lists a slide 12,000 times: 10,000 are read.
        QString slides;
        for (int i = 0; i < 12000; ++i) slides += QStringLiteral("<p:sld r:id=\"%1\"/>").arg(rels[i % 2]);
        auto d2 = deck::importPresentation(with(QStringLiteral("<p:custShowLst><p:custShow name=\"Long\" id=\"1\"><p:sldLst>%1</p:sldLst></p:custShow></p:custShowLst>").arg(slides)), &err);
        QVERIFY2(d2, qPrintable(err));
        const QVector<deck::CustomShow> longShow = deck::customShowsOf(*d2);
        QCOMPARE(int(longShow.size()), 1);
        QCOMPARE(int(longShow[0].slides.size()), 10000);
        // A name past 255 characters is cut, an id twice is made new, a show with no name is given one.
        auto d3 = deck::importPresentation(with(QStringLiteral("<p:custShowLst><p:custShow name=\"%1\" id=\"4\"><p:sldLst><p:sld r:id=\"%2\"/></p:sldLst></p:custShow>"
                                                               "<p:custShow name=\"\" id=\"4\"><p:sldLst><p:sld r:id=\"%2\"/></p:sldLst></p:custShow></p:custShowLst>")
                                                    .arg(QString(400, QLatin1Char('x')), rels[0])),
                                           &err);
        QVERIFY2(d3, qPrintable(err));
        const QVector<deck::CustomShow> odd = deck::customShowsOf(*d3);
        QCOMPARE(int(odd.size()), 2);
        QCOMPARE(int(odd[0].name.size()), 255);
        QVERIFY(odd[1].id != 4);
        QVERIFY(!odd[1].name.isEmpty());
        // Presentation properties that are not XML leave the defaults, with a note.
        auto d4 = deck::importPresentation(with(QString(), QByteArray("<p:presentationPr")), &err);
        QVERIFY2(d4, qPrintable(err));
        QVERIFY(deck::showSettingsOf(*d4) == deck::ShowSettings());
        // Kept properties are held to a size too.
        QString huge;
        for (int i = 0; i < 3000; ++i) huge += QStringLiteral("<p:ext uri=\"{%1}\"/>").arg(i);
        const QByteArray props = QStringLiteral("<p:presentationPr xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:extLst>%1</p:extLst></p:presentationPr>").arg(huge).toUtf8();
        auto d5 = deck::importPresentation(with(QString(), props), &err);
        QVERIFY2(d5, qPrintable(err));
        QVERIFY(d5->appData.value(QStringLiteral("presPropsKept")).toObject().isEmpty() || QJsonDocument(d5->appData.value(QStringLiteral("presPropsKept")).toObject()).toJson().size() < 200000);
    }

    // A file that repeats the show properties, or lists many shows for them to name, makes the program do the work once: only the first
    // properties are read, and the shows are looked through once.
    void showPropertiesInHostileFilesAreReadOnce()
    {
        auto d = lettered(2);
        QString err;
        Exported base;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &base, &err), qPrintable(err));
        QDomDocument dom;
        QStringList rels;
        for (const QDomElement &s : office::children(office::child(rootOf(base.zip.value(QStringLiteral("ppt/presentation.xml")), &dom), office::ns::P, QStringLiteral("sldIdLst")), office::ns::P,
                                                     QStringLiteral("sldId")))
            rels << office::attrNs(s, office::ns::R, QStringLiteral("id"));
        // 1,000 shows of 90 slides, and 20,000 show properties that each name one of them.
        QString shows = QStringLiteral("<p:custShowLst>");
        for (int i = 0; i < 1000; ++i) {
            shows += QStringLiteral("<p:custShow name=\"Show %1\" id=\"%1\"><p:sldLst>").arg(i);
            for (int k = 0; k < 90; ++k) shows += QStringLiteral("<p:sld r:id=\"%1\"/>").arg(rels[k % 2]);
            shows += QStringLiteral("</p:sldLst></p:custShow>");
        }
        shows += QStringLiteral("</p:custShowLst>");
        QString props = QStringLiteral("<p:presentationPr xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\">");
        props += QStringLiteral("<p:showPr loop=\"1\"><p:kiosk/><p:custShow id=\"7\"/></p:showPr>");
        for (int i = 0; i < 20000; ++i) props += QStringLiteral("<p:showPr><p:present/><p:custShow id=\"%1\"/></p:showPr>").arg(i % 1000);
        props += QStringLiteral("</p:presentationPr>");
        QMap<QString, QByteArray> zip = base.zip;
        QString xml = QString::fromUtf8(zip.value(QStringLiteral("ppt/presentation.xml")));
        xml.insert(xml.indexOf(QStringLiteral("<p:defaultTextStyle>")), shows);
        zip[QStringLiteral("ppt/presentation.xml")] = xml.toUtf8();
        zip[QStringLiteral("ppt/presProps.xml")] = props.toUtf8();
        ZipWriter z;
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) z.add(it.key(), it.value(), true);
        const deck::ShowImportWork readBefore = deck::showImportWorkForTest();
        auto back = deck::importPresentation(z.finish(), &err);
        QVERIFY2(back, qPrintable(err));
        // Counted: of the 20,000 show properties only the first is read in full, and the shows' slides are kept up to the limit on their entries.
        QCOMPARE(deck::showImportWorkForTest().showPropertiesRead - readBefore.showPropertiesRead, qint64(1));
        QVERIFY2(deck::showImportWorkForTest().customShowEntriesKept - readBefore.customShowEntriesKept <= 1000 * 90, qPrintable(QString::number(deck::showImportWorkForTest().customShowEntriesKept - readBefore.customShowEntriesKept)));
        const deck::ShowSettings s = deck::showSettingsOf(*back);
        QCOMPARE(int(s.type), int(deck::ShowSettings::Type::Kiosk));   // the first one's
        QCOMPARE(s.customShow, qint64(7));
        QVERIFY(s.loop);
        QCOMPARE(int(deck::customShowsOf(*back).size()), 1000);
        QCOMPARE(int(deck::customShowsOf(*back)[0].slides.size()), 90);
        // Saved again, the shows and the properties are written once, in a moment.
        Exported again;
        QVERIFY2(unpack(deck::exportPresentation(*back, &err), &again, &err), qPrintable(err));
        QCOMPARE(int(part(again, QStringLiteral("/ppt/presProps.xml")).count(QStringLiteral("<p:showPr"))), 1);
        QCOMPARE(int(part(again, QStringLiteral("/ppt/presentation.xml")).count(QStringLiteral("<p:custShow "))), 1000);
    }

    // Kept ink is a freeform line for each stroke above the slide's objects: the pen opaque and thin, the highlighter wide and
    // see-through, both as lines the file holds and the program shows.
    void inkBecomesFreeformShapes()
    {
        auto d = lettered(2);
        const int before = int(d->pages[0]->items.size());
        deck::InkStroke pen, marker, dot;
        for (int i = 0; i <= 200; ++i) pen.points << QPointF(100 + i * 2, 100 + 30 * std::sin(i / 10.0));
        pen.color = QColor(255, 0, 0);
        pen.width = deck::penWeightFor(d->pageSize().width());
        marker.points = {QPointF(100, 300), QPointF(400, 300)};
        marker.color = QColor(255, 255, 0);
        marker.width = deck::highlighterWeightFor(d->pageSize().width());
        marker.highlighter = true;
        dot.points = {QPointF(50, 50)};
        dot.color = QColor(0, 0, 255);
        dot.width = pen.width;
        deck::addInk(*d->pages[0], {pen, marker, dot});
        QCOMPARE(int(d->pages[0]->items.size()), before + 3);
        QCOMPARE(int(d->pages[1]->items.size()), before);
        const ShapeItem *p = static_cast<const ShapeItem *>(d->pages[0]->items[before].get());
        const ShapeItem *m = static_cast<const ShapeItem *>(d->pages[0]->items[before + 1].get());
        const ShapeItem *dotShape = static_cast<const ShapeItem *>(d->pages[0]->items[before + 2].get());
        QVERIFY(p->type() == ItemType::Shape && m->type() == ItemType::Shape);
        QVERIFY(!p->customPath.isEmpty());
        QVERIFY(p->fill.isNone());
        QVERIFY(p->customPath.elementCount() < 120);   // thinned to the points that matter, not all 201
        QVERIFY(p->rect.contains(QPointF(300, 100)));
        QCOMPARE(p->stroke.width, 2.5 * d->pageSize().width() / 960);
        QCOMPARE(p->stroke.transparency, 0.0);   // opaque
        QCOMPARE(m->stroke.transparency, 0.5);   // see-through
        QVERIFY(m->stroke.width > 5 * p->stroke.width);
        QVERIFY(p->stroke.width < 4);
        QVERIFY(p->stroke.cap == Qt::RoundCap);
        QVERIFY(dotShape->rect.width() >= 1 && dotShape->rect.height() >= 1);
        QVERIFY(m->customPath.elementCount() == 2);
        // Saved, each is a freeform with its own line, and the file reads back with them.
        QString err;
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QCOMPARE(int(slide.count(QStringLiteral("<a:custGeom>"))), 3);
        QVERIFY(slide.contains(QStringLiteral("<a:ln w=\"%1\"").arg(qRound64(p->stroke.width * 12700))));
        QVERIFY(slide.contains(QStringLiteral("<a:alpha val=\"50000\"/>")));
        auto back = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages[0]->items.size()), before + 3);
        // A stroke with no point makes nothing.
        QVERIFY(!deck::inkShape(deck::InkStroke()));
        // The eraser reaches a stroke by its line and half its weight.
        QVERIFY(deck::inkTouches(marker, QPointF(250, 305), 1));
        QVERIFY(!deck::inkTouches(marker, QPointF(250, 330), 1));
        QVERIFY(deck::inkTouches(dot, QPointF(52, 50), 1));
        QVERIFY(!deck::inkTouches(dot, QPointF(60, 50), 1));
    }

    // A show set to loop starts again at the first slide after the last, with the transition into it; one that does not ends with
    // the black screen. Back from the first slide stays on it.
    void showLoopsWhenSetTo()
    {
        auto d = showDeck(3);
        LayoutCache cache;
        SlideShow plain(d.get(), &cache, 0);
        plain.goTo(2);
        plain.next();
        QCOMPARE(plain.currentSlide(), -1);
        SlideShow loop(d.get(), &cache, 0);
        deck::ShowSettings s;
        s.loop = true;
        loop.setSettings(s);
        loop.goTo(2);
        loop.next();
        QCOMPARE(loop.currentSlide(), 0);
        loop.previous();
        QCOMPARE(loop.currentSlide(), 0);
        deck::setTransition(*d->pages[0], showTransition("fade", 30));
        loop.goTo(2);
        loop.next();
        QVERIFY(loop.isAnimating());
        QCOMPARE(loop.targetSlide(), 0);
        loop.finishTransition();
        QCOMPARE(loop.currentSlide(), 0);
        // A hidden first slide is passed over on the way round.
        d->pages[0]->hidden = true;
        loop.goTo(2);
        loop.next();
        QCOMPARE(loop.currentSlide(), 1);
    }

    // At a kiosk the keys and clicks do nothing but Esc ends the show and a link opens; the slides' times move it on, and it
    // starts again after the last.
    void showAtAKioskMovesOnOnlyByTimings()
    {
        auto d = showDeck(3);
        for (int i = 0; i < 3; ++i) {
            deck::Transition t;
            t.advanceAfter = 0.05;
            deck::setTransition(*d->pages[i], t);
        }
        d->pages[1]->items[0]->hyperlink = QStringLiteral("https://example.com/kiosk");
        LinkCatcher catcher;
        QDesktopServices::setUrlHandler(QStringLiteral("https"), &catcher, "open");
        LayoutCache cache;
        QPointer<SlideShow> show = new SlideShow(d.get(), &cache, 0);
        deck::ShowSettings s;
        s.type = deck::ShowSettings::Type::Kiosk;
        show->setSettings(s);
        QVERIFY(show->isKiosk());
        QVERIFY(!show->isWindowed());
        QSignalSpy ended(show.data(), &SlideShow::ended);
        show->resize(960, 540);
        QTest::keyClick(show.data(), Qt::Key_Right);
        QTest::keyClick(show.data(), Qt::Key_Space);
        QTest::mouseClick(show.data(), Qt::LeftButton, Qt::NoModifier, QPoint(5, 530));
        QTest::mouseClick(show.data(), Qt::RightButton, Qt::NoModifier, QPoint(5, 530));
        QTest::keyClick(show.data(), Qt::Key_P, Qt::ControlModifier);
        QCOMPARE(show->currentSlide(), 0);
        QVERIFY(!show->findChild<QMenu *>(QStringLiteral("showMenu")));
        QCOMPARE(int(show->pointer()), int(SlideShow::Pointer::Arrow));
        QCOMPARE(ended.count(), 0);
        // A link opens, and the show stays where it is.
        show->goTo(1);
        QTest::mouseClick(show.data(), Qt::LeftButton, Qt::NoModifier, d->pages[1]->items[0]->bounds().center().toPoint());
        QCOMPARE(catcher.urls, (QStringList{"https://example.com/kiosk"}));
        QCOMPARE(show->currentSlide(), 1);
        QDesktopServices::unsetUrlHandler(QStringLiteral("https"));
        // The times move it on, round and round.
        QList<int> seen;
        connect(show.data(), &SlideShow::changed, this, [&] { seen << show->currentSlide(); });
        show->goTo(0);
        seen.clear();
        show->show();
        QTRY_VERIFY(seen.size() >= 4);
        QCOMPARE(seen.mid(0, 4), (QList<int>{1, 2, 0, 1}));
        QTest::keyClick(show.data(), Qt::Key_Escape);
        QCOMPARE(ended.count(), 1);
        QTRY_VERIFY(show.isNull());
    }

    // Moving on by the slides' times is a setting: off, they do nothing.
    void showFollowsTimingsOnlyWhenSetTo()
    {
        auto d = showDeck(2);
        deck::Transition t;
        t.advanceAfter = 30;
        deck::setTransition(*d->pages[0], t);
        LayoutCache cache;
        SlideShow timed(d.get(), &cache, 0);
        timed.show();
        QVERIFY(timed.autoAdvanceActive());
        SlideShow manual(d.get(), &cache, 0);
        deck::ShowSettings s;
        s.useTimings = false;
        manual.setSettings(s);
        manual.show();
        QVERIFY(!manual.autoAdvanceActive());
        manual.next();   // by hand it still goes
        QCOMPARE(manual.currentSlide(), 1);
    }

    // A show without animation has no steps to click through, and shows each slide whole.
    void showWithoutAnimationShowsSlidesWhole()
    {
        auto d = animatedDeck(100);
        LayoutCache cache;
        SlideShow animated(d.get(), &cache, 0);
        QCOMPARE(animated.stepCount(), 2);
        SlideShow still(d.get(), &cache, 0);
        deck::ShowSettings s;
        s.animation = false;
        still.setSettings(s);
        QCOMPARE(still.stepCount(), 0);
        const QSize size(640, 360);
        const QImage start = animated.renderView(SlideShow::View{0, 0}, -1, size, 1.0);
        const QImage end = animated.renderView(SlideShow::View{0, 2}, -1, size, 1.0);
        QVERIFY(start != end);
        QCOMPARE(still.renderView(SlideShow::View{0, 0}, -1, size, 1.0), end);
        still.next();   // the first click moves on
        QCOMPARE(still.currentSlide(), 1);
    }

    // A show plays the slides it is given in the order it is given them, a slide twice if it lists it twice, and hidden slides if it
    // is a custom show; back goes the way it came; a number goes to the first place a slide has.
    void showPlaysTheSlidesItIsGiven()
    {
        auto d = showDeck(5);
        d->pages[3]->hidden = true;
        LayoutCache cache;
        SlideShow ranged(d.get(), &cache, 0);
        ranged.setSlides({1, 2, 3}, false);
        QCOMPARE(ranged.currentSlide(), 1);
        QCOMPARE(ranged.playedSlides(), (QList<int>{1, 2}));
        ranged.next();
        QCOMPARE(ranged.currentSlide(), 2);
        ranged.next();
        QCOMPARE(ranged.currentSlide(), -1);   // the hidden slide is passed over
        SlideShow custom(d.get(), &cache, 0);
        custom.setSlides({3, 0, 3, 4}, true);
        QCOMPARE(custom.currentSlide(), 3);
        QCOMPARE(custom.playedSlides(), (QList<int>{3, 0, 3, 4}));
        PresenterView pv(&custom);
        QCOMPARE(pv.nextSlide(), 0);
        QCOMPARE(pv.counterText(), QStringLiteral("Slide 4 of 5"));
        custom.next();
        QCOMPARE(custom.currentSlide(), 0);
        custom.next();
        QCOMPARE(custom.currentSlide(), 3);
        QCOMPARE(pv.nextSlide(), 4);
        custom.previous();
        QCOMPARE(custom.currentSlide(), 0);   // not the first 3: back is the place before
        custom.next();
        custom.next();
        QCOMPARE(custom.currentSlide(), 4);
        custom.next();
        QCOMPARE(custom.currentSlide(), -1);
        custom.previous();
        QCOMPARE(custom.currentSlide(), 4);
        custom.goTo(3);
        QCOMPARE(custom.currentSlide(), 3);
        custom.next();
        QCOMPARE(custom.currentSlide(), 0);   // the first place of 3 came before 0
        custom.goTo(2);                        // not in the show
        QCOMPARE(custom.currentSlide(), 0);
        QTest::keyClick(&custom, Qt::Key_End);
        QCOMPARE(custom.currentSlide(), 4);
        QTest::keyClick(&custom, Qt::Key_Home);
        QCOMPARE(custom.currentSlide(), 3);
        // A show of every slide starts where it is asked to.
        SlideShow all(d.get(), &cache, 2);
        all.setSlides({}, false);
        QCOMPARE(all.currentSlide(), 2);
    }

    // Ctrl+P, Ctrl+I, Ctrl+L, and Ctrl+E choose the pen, the highlighter, the laser pointer, and the eraser; Ctrl+A and Esc bring the
    // arrow back (Esc ends the show only from the arrow); P alone still goes back.
    void showKeysChooseThePointer()
    {
        auto d = showDeck(3);
        LayoutCache cache;
        QPointer<SlideShow> show = new SlideShow(d.get(), &cache, 1);
        QSignalSpy ended(show.data(), &SlideShow::ended);
        QSignalSpy changed(show.data(), &SlideShow::overlayChanged);
        QCOMPARE(int(show->pointer()), int(SlideShow::Pointer::Arrow));
        auto press = [&](int key, SlideShow::Pointer expect) {
            QTest::keyClick(show.data(), key, Qt::ControlModifier);
            QCOMPARE(int(show->pointer()), int(expect));
        };
        press(Qt::Key_P, SlideShow::Pointer::Pen);
        QVERIFY(changed.count() > 0);
        press(Qt::Key_I, SlideShow::Pointer::Highlighter);
        press(Qt::Key_L, SlideShow::Pointer::Laser);
        press(Qt::Key_E, SlideShow::Pointer::Eraser);
        press(Qt::Key_A, SlideShow::Pointer::Arrow);
        press(Qt::Key_P, SlideShow::Pointer::Pen);
        QTest::keyClick(show.data(), Qt::Key_Escape);   // the arrow first
        QCOMPARE(int(show->pointer()), int(SlideShow::Pointer::Arrow));
        QCOMPARE(ended.count(), 0);
        QCOMPARE(show->currentSlide(), 1);
        QTest::keyClick(show.data(), Qt::Key_P);        // plain P: back
        QCOMPARE(show->currentSlide(), 0);
        QCOMPARE(int(show->pointer()), int(SlideShow::Pointer::Arrow));
        QTest::keyClick(show.data(), Qt::Key_Escape);
        QCOMPARE(ended.count(), 1);
        QTRY_VERIFY(show.isNull());
    }

    // The pen and the highlighter draw on the slide where the pointer drags, a click doesn't move the show on, the ink stays on its slide
    // while the show runs and is on the screen, the eraser takes a stroke it touches, and E takes all the slide's.
    void showDrawsInkOnTheSlide()
    {
        auto d = showDeck(3);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);   // slide points are pixels
        auto mouse = [&](QEvent::Type type, QPoint p, Qt::MouseButton button, Qt::KeyboardModifiers mods = Qt::NoModifier) {
            QMouseEvent e(type, QPointF(p), QPointF(show.mapToGlobal(p)), type == QEvent::MouseMove ? Qt::NoButton : button,
                          type == QEvent::MouseButtonRelease ? Qt::NoButton : button, mods);
            QApplication::sendEvent(&show, &e);
        };
        auto stroke = [&](QPoint from, QPoint to) {
            mouse(QEvent::MouseButtonPress, from, Qt::LeftButton);
            mouse(QEvent::MouseMove, QPoint((from.x() + to.x()) / 2, (from.y() + to.y()) / 2 + 10), Qt::LeftButton);
            mouse(QEvent::MouseMove, to, Qt::LeftButton);
            mouse(QEvent::MouseButtonRelease, to, Qt::LeftButton);
        };
        // The arrow draws nothing.
        stroke(QPoint(100, 100), QPoint(300, 100));
        QCOMPARE(show.inkCount(), 0);
        QCOMPARE(show.currentSlide(), 1);   // (a click moved on)
        show.goTo(0);
        QTest::keyClick(&show, Qt::Key_P, Qt::ControlModifier);
        stroke(QPoint(100, 100), QPoint(300, 100));
        QCOMPARE(show.currentSlide(), 0);   // a pen's click doesn't
        QCOMPARE(show.inkCount(), 1);
        QCOMPARE(int(show.ink(0).size()), 1);
        const deck::InkStroke &pen = show.ink(0)[0];
        QVERIFY(pen.points.size() >= 3);
        QVERIFY(!pen.highlighter);
        QCOMPARE(pen.color, QColor(255, 0, 0));
        QCOMPARE(pen.width, 2.5);
        QCOMPARE(pen.points.first(), QPointF(100, 100));
        QVERIFY(!show.isDrawing());
        // It is on the screen: red where the line goes.
        auto redNear = [&](QPoint at) {
            const QImage shot = show.grab().toImage();
            for (int y = at.y() - 3; y <= at.y() + 3; ++y)
                for (int x = at.x() - 3; x <= at.x() + 3; ++x)
                    if (shot.pixelColor(x, y).red() > 200 && shot.pixelColor(x, y).green() < 60) return true;
            return false;
        };
        QVERIFY(redNear(QPoint(300, 100)));
        // Another color, and the highlighter: wide and yellow.
        show.setInkColor(QColor(0, 112, 192));
        QCOMPARE(show.inkColor(), QColor(0, 112, 192));
        QTest::keyClick(&show, Qt::Key_I, Qt::ControlModifier);
        QCOMPARE(show.inkColor(), QColor(255, 255, 0));
        stroke(QPoint(100, 300), QPoint(400, 300));
        QCOMPARE(int(show.ink(0).size()), 2);
        QVERIFY(show.ink(0)[1].highlighter);
        QCOMPARE(show.ink(0)[1].width, 18.0);
        QCOMPARE(show.ink(0)[1].color, QColor(255, 255, 0));
        // The pen kept its color.
        QTest::keyClick(&show, Qt::Key_P, Qt::ControlModifier);
        QCOMPARE(show.inkColor(), QColor(0, 112, 192));
        // Ink stays on its slide: gone from the screen on another, back when the show returns.
        show.next();
        QCOMPARE(show.currentSlide(), 1);
        QCOMPARE(show.ink(1).size(), 0);
        QVERIFY(!redNear(QPoint(300, 100)));
        show.previous();
        QCOMPARE(int(show.ink(0).size()), 2);
        QCOMPARE(show.allInk().size(), 1);
        // The eraser takes the stroke it touches, the rest stay.
        QTest::keyClick(&show, Qt::Key_E, Qt::ControlModifier);
        mouse(QEvent::MouseButtonPress, QPoint(250, 300), Qt::LeftButton);
        mouse(QEvent::MouseButtonRelease, QPoint(250, 300), Qt::LeftButton);
        QCOMPARE(int(show.ink(0).size()), 1);
        QVERIFY(!show.ink(0)[0].highlighter);
        // A press away from every stroke takes none.
        mouse(QEvent::MouseButtonPress, QPoint(700, 450), Qt::LeftButton);
        mouse(QEvent::MouseButtonRelease, QPoint(700, 450), Qt::LeftButton);
        QCOMPARE(int(show.ink(0).size()), 1);
        // E alone takes all the ink of the slide.
        QTest::keyClick(&show, Qt::Key_P, Qt::ControlModifier);
        stroke(QPoint(500, 200), QPoint(600, 250));
        QCOMPARE(int(show.ink(0).size()), 2);
        show.goTo(1);
        stroke(QPoint(500, 200), QPoint(600, 250));
        QCOMPARE(show.inkCount(), 3);
        QTest::keyClick(&show, Qt::Key_E);
        QCOMPARE(int(show.ink(1).size()), 0);
        QCOMPARE(int(show.ink(0).size()), 2);   // only the slide on the screen
        QCOMPARE(show.inkCount(), 2);
        // A click without a drag is a dot.
        mouse(QEvent::MouseButtonPress, QPoint(50, 400), Qt::LeftButton);
        mouse(QEvent::MouseButtonRelease, QPoint(50, 400), Qt::LeftButton);
        QCOMPARE(int(show.ink(1).size()), 1);
        QCOMPARE(int(show.ink(1)[0].points.size()), 1);
    }

    // The laser pointer is a dot on the slide that follows the mouse and leaves nothing behind; Ctrl and the button give it for as long as
    // they are held, from the arrow.
    void showLaserPointerFollowsTheMouse()
    {
        auto d = showDeck(2);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        auto mouse = [&](QEvent::Type type, QPoint p, Qt::MouseButton button, Qt::KeyboardModifiers mods = Qt::NoModifier) {
            QMouseEvent e(type, QPointF(p), QPointF(show.mapToGlobal(p)), type == QEvent::MouseMove ? Qt::NoButton : button,
                          type == QEvent::MouseButtonRelease ? Qt::NoButton : button, mods);
            QApplication::sendEvent(&show, &e);
        };
        auto laserNear = [&](QPoint at) {
            const QImage shot = show.grab().toImage();
            return shot.pixelColor(at.x(), at.y()).red() > 200 && shot.pixelColor(at.x(), at.y()).green() < 80 && shot.pixelColor(at.x(), at.y()).blue() < 80;
        };
        QTest::keyClick(&show, Qt::Key_L, Qt::ControlModifier);
        QVERIFY(!laserNear(QPoint(700, 400)));
        mouse(QEvent::MouseMove, QPoint(700, 400), Qt::NoButton);
        QVERIFY(laserNear(QPoint(700, 400)));
        mouse(QEvent::MouseMove, QPoint(200, 400), Qt::NoButton);
        QVERIFY(laserNear(QPoint(200, 400)));
        QVERIFY(!laserNear(QPoint(700, 400)));
        QCOMPARE(show.inkCount(), 0);
        show.pointerLeft();
        QVERIFY(!laserNear(QPoint(200, 400)));
        // The color is the setting's.
        deck::ShowSettings s;
        s.laserColor = QColor(0, 255, 0);
        show.setSettings(s);
        QCOMPARE(show.laserColor(), QColor(0, 255, 0));
        mouse(QEvent::MouseMove, QPoint(300, 300), Qt::NoButton);
        QCOMPARE(show.grab().toImage().pixelColor(300, 300), QColor(0, 255, 0));
        // From the arrow, Ctrl and the button.
        QTest::keyClick(&show, Qt::Key_A, Qt::ControlModifier);
        mouse(QEvent::MouseMove, QPoint(500, 300), Qt::NoButton);
        QVERIFY(show.grab().toImage().pixelColor(500, 300) != QColor(0, 255, 0));   // no dot
        mouse(QEvent::MouseButtonPress, QPoint(500, 300), Qt::LeftButton, Qt::ControlModifier);
        QCOMPARE(show.currentSlide(), 0);   // it did not click
        QCOMPARE(show.grab().toImage().pixelColor(500, 300), QColor(0, 255, 0));
        mouse(QEvent::MouseButtonRelease, QPoint(500, 300), Qt::LeftButton, Qt::ControlModifier);
        QCOMPARE(int(show.pointer()), int(SlideShow::Pointer::Arrow));
        QVERIFY(show.grab().toImage().pixelColor(500, 300) != QColor(0, 255, 0));
        QCOMPARE(show.inkCount(), 0);
    }

    // A right click opens a menu of the show: next and previous, go to a slide, the pointer's tools and ink colors, the screens, and
    // the end; its pointer part is the same in Presenter View.
    void showRightClickOpensTheMenu()
    {
        auto d = showDeck(3);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 1);
        show.resize(960, 540);
        QTest::mouseClick(&show, Qt::RightButton, Qt::NoModifier, QPoint(10, 10));
        QCOMPARE(show.currentSlide(), 1);   // a right click no longer goes back: the menu has Previous
        QMenu *menu = show.findChild<QMenu *>(QStringLiteral("showMenu"));
        QVERIFY(menu);
        QVERIFY(menu->isVisible());
        QVERIFY(menu->findChild<QAction *>(QStringLiteral("menuNext")));
        QVERIFY(menu->findChild<QAction *>(QStringLiteral("menuEnd")));
        QMenu *go = menu->findChild<QMenu *>(QStringLiteral("goToSlideMenu"));
        QVERIFY(go);
        QCOMPARE(int(go->actions().size()), 3);
        QVERIFY(go->actions()[1]->isChecked());
        QVERIFY(go->actions()[0]->text().contains(QStringLiteral("Slide 1 title")));
        go->actions()[2]->trigger();
        QCOMPARE(show.currentSlide(), 2);
        menu->findChild<QAction *>(QStringLiteral("menuPrevious"))->trigger();
        QCOMPARE(show.currentSlide(), 1);
        // The pointer's tools: each chooses it; the checked one is the pointer.
        QAction *pen = menu->findChild<QAction *>(QStringLiteral("pointerPen"));
        QVERIFY(pen);
        QVERIFY(pen->isCheckable());
        QVERIFY(!pen->isChecked());
        QVERIFY(pen->text().contains(QKeySequence(Qt::CTRL | Qt::Key_P).toString(QKeySequence::NativeText)));
        pen->trigger();
        QCOMPARE(int(show.pointer()), int(SlideShow::Pointer::Pen));
        for (const auto &pair : {QPair<const char *, SlideShow::Pointer>{"pointerHighlighter", SlideShow::Pointer::Highlighter},
                                 {"pointerEraser", SlideShow::Pointer::Eraser}, {"pointerLaser", SlideShow::Pointer::Laser}, {"pointerArrow", SlideShow::Pointer::Arrow}}) {
            QAction *a = menu->findChild<QAction *>(QString::fromLatin1(pair.first));
            QVERIFY(a);
            a->trigger();
            QCOMPARE(int(show.pointer()), int(pair.second));
        }
        // Ink color: from the list, for the tool in hand.
        show.setPointer(SlideShow::Pointer::Pen);
        QMenu *colors = menu->findChild<QMenu *>(QStringLiteral("inkColorMenu"));
        QVERIFY(colors);
        QAction *blue = nullptr;
        for (QAction *a : colors->actions())
            if (a->data().value<QColor>() == QColor(0, 112, 192)) blue = a;
        QVERIFY(blue);
        blue->trigger();
        QCOMPARE(show.inkColor(), QColor(0, 112, 192));
        // Erase All Ink on Slide is there when the slide has ink.
        QMenu *fresh = show.pointerMenu(&show);
        QVERIFY(!fresh->findChild<QAction *>(QStringLiteral("eraseAllInk"))->isEnabled());
        delete fresh;
        QTest::keyClick(&show, Qt::Key_P, Qt::ControlModifier);
        QTest::mousePress(&show, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
        QTest::mouseRelease(&show, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
        QCOMPARE(show.inkCount(), 1);
        fresh = show.pointerMenu(&show);
        QAction *erase = fresh->findChild<QAction *>(QStringLiteral("eraseAllInk"));
        QVERIFY(erase->isEnabled());
        erase->trigger();
        QCOMPARE(show.inkCount(), 0);
        delete fresh;
        // End Show ends it.
        QSignalSpy ended(&show, &SlideShow::ended);
        menu->findChild<QAction *>(QStringLiteral("menuEnd"))->trigger();
        QCOMPARE(ended.count(), 1);
        delete menu;
    }

    // Presenter View draws with the pen on its large slide, and the audience's screen follows; its button opens the pointer menu.
    void presenterViewDrawsInk()
    {
        auto d = showDeck(2);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        PresenterView pv(&show);
        pv.resize(1100, 700);
        pv.show();
        QWidget *large = pv.findChild<QWidget *>(QStringLiteral("currentSlide"));
        QVERIFY(large);
        QVERIFY(pv.findChild<QAbstractButton *>(QStringLiteral("pointerButton")));
        QSignalSpy overlay(&show, &SlideShow::overlayChanged);
        auto mouse = [&](QEvent::Type type, QPoint p) {
            QMouseEvent e(type, QPointF(p), QPointF(large->mapToGlobal(p)), type == QEvent::MouseMove ? Qt::NoButton : Qt::LeftButton,
                          type == QEvent::MouseButtonRelease ? Qt::NoButton : Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(large, &e);
        };
        // The arrow: a click moves on.
        mouse(QEvent::MouseButtonPress, QPoint(10, 10));
        mouse(QEvent::MouseButtonRelease, QPoint(10, 10));
        QCOMPARE(show.currentSlide(), 1);
        show.goTo(0);
        QTest::keyClick(&pv, Qt::Key_P, Qt::ControlModifier);   // keys reach the show from this window too
        QCOMPARE(int(show.pointer()), int(SlideShow::Pointer::Pen));
        QVERIFY(pv.findChild<QAbstractButton *>(QStringLiteral("pointerButton"))->isChecked());
        const QRectF at = slideRectIn(d->pageSize(), large->size());
        const QPoint a(int(at.left() + at.width() * 0.25), int(at.top() + at.height() * 0.5)), b(int(at.left() + at.width() * 0.75), int(at.top() + at.height() * 0.5));
        mouse(QEvent::MouseButtonPress, a);
        mouse(QEvent::MouseMove, QPoint((a.x() + b.x()) / 2, a.y() + 8));
        mouse(QEvent::MouseMove, b);
        mouse(QEvent::MouseButtonRelease, b);
        QCOMPARE(show.currentSlide(), 0);
        QCOMPARE(show.inkCount(), 1);
        QVERIFY(overlay.count() >= 3);
        // The stroke is in the slide's own points: a quarter of the way across to three quarters.
        const QPointF first = show.ink(0)[0].points.first(), last = show.ink(0)[0].points.last();
        QVERIFY(std::abs(first.x() - 0.25 * d->pageSize().width()) < 4);
        QVERIFY(std::abs(last.x() - 0.75 * d->pageSize().width()) < 4);
        // It shows in Presenter View.
        const QImage shot = large->grab().toImage();
        bool red = false;
        for (int x = a.x(); x < b.x() && !red; ++x)
            for (int y = a.y() - 12; y < a.y() + 12 && !red; ++y) red = shot.pixelColor(x, y).red() > 200 && shot.pixelColor(x, y).green() < 60;
        QVERIFY(red);
        // A right click on the large slide opens the show's menu.
        {
            QMouseEvent press(QEvent::MouseButtonPress, QPointF(a), QPointF(large->mapToGlobal(a)), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
            QApplication::sendEvent(large, &press);
            QMenu *shown = pv.findChild<QMenu *>(QStringLiteral("showMenu"));
            QVERIFY(shown);
            QVERIFY(shown->findChild<QAction *>(QStringLiteral("menuNext")));
            QCOMPARE(show.currentSlide(), 0);   // it did not move on
            delete shown;
        }
        // The button opens the menu of the pointer.
        pv.findChild<QAbstractButton *>(QStringLiteral("pointerButton"))->click();
        QMenu *menu = nullptr;
        for (QMenu *m : pv.findChildren<QMenu *>(QStringLiteral("pointerMenu"))) menu = m;
        QVERIFY(menu);
        QVERIFY(menu->findChild<QAction *>(QStringLiteral("pointerLaser")));
        delete menu;
    }

    // Rehearse Timings: the show times each slide from when it comes on the screen to when it goes, with the bar's next, pause, repeat,
    // and the two clocks; it moves on only when told, and has no pen and no menu.
    void showRecordsTheTimeOfEachSlide()
    {
        auto d = showDeck(3);
        deck::Transition t;
        t.advanceAfter = 0.05;
        deck::setTransition(*d->pages[0], t);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        qint64 now = 1000;
        show.setClock([&now] { return now; });
        show.setRecording(true);
        QVERIFY(show.isRecording());
        RehearseBar *bar = show.rehearseBar();
        QVERIFY(bar);
        QAbstractButton *next = bar->findChild<QAbstractButton *>(QStringLiteral("rehearseNext"));
        QAbstractButton *pause = bar->findChild<QAbstractButton *>(QStringLiteral("rehearsePause"));
        QAbstractButton *repeat = bar->findChild<QAbstractButton *>(QStringLiteral("rehearseRepeat"));
        QVERIFY(next && pause && repeat);
        show.show();   // the slides' own times do not move it
        QVERIFY(!show.autoAdvanceActive());
        now = 1000 + 3000;
        QCOMPARE(show.slideClock(), qint64(3000));
        QCOMPARE(show.totalClock(), qint64(3000));
        bar->refresh();
        QCOMPARE(bar->slideText(), QStringLiteral("0:00:03"));
        QCOMPARE(bar->totalText(), QStringLiteral("0:00:03"));
        next->click();   // the bar's Next is the click
        QCOMPARE(show.currentSlide(), 1);
        QCOMPARE(show.slideClock(), qint64(0));
        now += 5000;
        QCOMPARE(show.slideClock(), qint64(5000));
        QCOMPARE(show.totalClock(), qint64(8000));
        // Paused, the clocks hold; resumed, they go on.
        pause->click();
        QVERIFY(show.isRecordingPaused());
        now += 9999;
        QCOMPARE(show.slideClock(), qint64(5000));
        QCOMPARE(show.totalClock(), qint64(8000));
        pause->click();
        QVERIFY(!show.isRecordingPaused());
        now += 1000;
        QCOMPARE(show.slideClock(), qint64(6000));
        // Repeat starts the slide's time from nothing, and the total goes back by it.
        repeat->click();
        QCOMPARE(show.slideClock(), qint64(0));
        QCOMPARE(show.totalClock(), qint64(3000));
        now += 2000;
        show.next();
        QCOMPARE(show.currentSlide(), 2);
        now += 700;
        show.next();   // past the last slide: the end
        QCOMPARE(show.currentSlide(), -1);
        now += 5000;   // the black screen is no slide's time
        const QMap<int, double> seconds = show.recordedSeconds();
        QCOMPARE(seconds.keys(), (QList<int>{0, 1, 2}));
        QCOMPARE(seconds.value(0), 3.0);
        QCOMPARE(seconds.value(1), 2.0);
        QCOMPARE(seconds.value(2), 0.7);
        QCOMPARE(show.recordedTotal(), 5.7);
        // No pen, no menu; a right click goes back as before.
        QTest::keyClick(&show, Qt::Key_P, Qt::ControlModifier);
        QCOMPARE(int(show.pointer()), int(SlideShow::Pointer::Arrow));
        show.previous();
        QCOMPARE(show.currentSlide(), 2);
        show.resize(960, 540);
        QTest::mouseClick(&show, Qt::RightButton, Qt::NoModifier, QPoint(300, 300));
        QCOMPARE(show.currentSlide(), 1);
        QVERIFY(!show.findChild<QMenu *>(QStringLiteral("showMenu")));
        // Going back to a slide times it again.
        now += 4000;
        show.next();
        QCOMPARE(show.recordedSeconds().value(1), 4.0);
    }

    // The Slide Show tab: Custom Slide Show lists the custom shows by name above Custom Shows..., and plays one; Set Up has Set Up Slide
    // Show, Rehearse Timings, and the Use Timings check box.
    void windowSlideShowTabHasSetUpAndCustomShows()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        for (const char *id : {"show.setUp", "show.rehearse", "show.customShows", "show.useTimings"}) QVERIFY2(w.act(QString::fromLatin1(id)), id);
        QVERIFY(w.act("show.useTimings")->isCheckable());
        QVERIFY(w.act("show.useTimings")->isChecked());
        auto *box = w.ribbon()->findChild<QCheckBox *>(QStringLiteral("useTimings"));
        QVERIFY(box);
        QVERIFY(box->isChecked());
        QStringList groups;
        for (RibbonGroup *g : w.ribbon()->tab(QStringLiteral("Slide Show"))->groups()) groups << g->title();
        QCOMPARE(groups, (QStringList{"Start Slide Show", "Set Up", "Monitors"}));
        QAction *command = w.findChild<QAction *>(QStringLiteral("ribbon.customShow"));
        QVERIFY(command);
        QMenu *menu = nullptr;
        for (QObject *o : command->associatedObjects())
            if (auto *button = qobject_cast<QToolButton *>(o)) menu = button->menu();
        QVERIFY(menu);
        auto listed = [&] {
            Q_EMIT menu->aboutToShow();
            QStringList names;
            for (QAction *a : menu->actions())
                if (!a->isSeparator()) names << a->text();
            return names;
        };
        QCOMPARE(listed(), (QStringList{w.act("show.customShows")->text()}));   // none yet: only the way to make some
        deck::CustomShow a, b;
        a.id = 1;
        a.name = QStringLiteral("Overview");
        a.slides = {ed->doc()->pages[2]->id, ed->doc()->pages[0]->id};
        b.id = 2;
        b.name = QStringLiteral("Details");
        b.slides = {ed->doc()->pages[1]->id};
        ed->beginChange(QStringLiteral("test"));
        deck::setCustomShows(*ed->doc(), {a, b});
        ed->endChange();
        QCOMPARE(listed(), (QStringList{"Overview", "Details", w.act("show.customShows")->text()}));
        // Choosing one plays it, from its first slide, in its order.
        for (QAction *action : menu->actions())
            if (action->text() == QLatin1String("Overview")) action->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show);
        QCOMPARE(show->playedSlides(), (QList<int>{2, 0}));
        QCOMPARE(show->currentSlide(), 2);
        show->close();
        QTRY_VERIFY(!w.slideShow());
        // A show set to play it, and the slide taken out of the presentation leaves it.
        ed->setCurrentPage(2);
        w.deleteSlide();
        QCOMPARE(deck::customShowsOf(*ed->doc()).size(), 2);
        QCOMPARE(int(deck::customShowsOf(*ed->doc())[0].slides.size()), 1);
        QCOMPARE(listed().first(), QStringLiteral("Overview"));
    }

    // The Use Timings check box is the setting: one step that undoes, and the box and the command follow.
    void windowUseTimingsCheckBox()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        auto *box = w.ribbon()->findChild<QCheckBox *>(QStringLiteral("useTimings"));
        QVERIFY(box);
        QVERIFY(!ed->isModified());
        box->click();
        QVERIFY(!deck::showSettingsOf(*ed->doc()).useTimings);
        QVERIFY(!w.act("show.useTimings")->isChecked());
        QVERIFY(ed->isModified());
        ed->undo();
        QVERIFY(deck::showSettingsOf(*ed->doc()).useTimings);
        QVERIFY(box->isChecked());
        QVERIFY(w.act("show.useTimings")->isChecked());
        ed->redo();
        QVERIFY(!box->isChecked());
        QVERIFY(!deck::showSettingsOf(*ed->doc()).useTimings);
        // A show started now does not move on by the slides' times.
        deck::Transition t;
        t.advanceAfter = 30;
        ed->beginChange(QStringLiteral("test"));
        deck::setTransition(*ed->doc()->pages[0], t);
        ed->endChange();
        w.startShow(0);
        QVERIFY(w.slideShow());
        QVERIFY(!w.slideShow()->autoAdvanceActive());
        w.slideShow()->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // Set Up Slide Show: the dialog shows what the presentation has, and OK is one step that undoes; a kiosk loops and moves on by the
    // times; the show follows what was set.
    void windowSetUpSlideShowDialog()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        w.newSlide();
        deck::CustomShow c;
        c.id = 3;
        c.name = QStringLiteral("Two and four");
        c.slides = {ed->doc()->pages[1]->id, ed->doc()->pages[3]->id};
        ed->beginChange(QStringLiteral("test"));
        deck::setCustomShows(*ed->doc(), {c});
        ed->endChange();
        QVERIFY(w.act("show.setUp")->shortcut().isEmpty());
        // Shown with the defaults.
        bool opened = false;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            opened = true;
            QCOMPARE(d->objectName(), QStringLiteral("setUpShowDialog"));
            QVERIFY(d->findChild<QRadioButton *>(QStringLiteral("typeSpeaker"))->isChecked());
            QVERIFY(d->findChild<QRadioButton *>(QStringLiteral("slidesAll"))->isChecked());
            QVERIFY(d->findChild<QRadioButton *>(QStringLiteral("advanceTimings"))->isChecked());
            QVERIFY(!d->findChild<QCheckBox *>(QStringLiteral("loop"))->isChecked());
            QVERIFY(!d->findChild<QCheckBox *>(QStringLiteral("noAnimation"))->isChecked());
            QVERIFY(d->findChild<QSpinBox *>(QStringLiteral("rangeFrom")));
            QVERIFY(!d->findChild<QSpinBox *>(QStringLiteral("rangeFrom"))->isEnabled());   // only for From...To
            QVERIFY(!d->findChild<QComboBox *>(QStringLiteral("customShow"))->isEnabled());   // only for a custom show
            d->findChild<QRadioButton *>(QStringLiteral("slidesRange"))->setChecked(true);
            QVERIFY(d->findChild<QSpinBox *>(QStringLiteral("rangeFrom"))->isEnabled());
            QVERIFY(d->findChild<QSpinBox *>(QStringLiteral("rangeTo"))->isEnabled());
            d->findChild<QRadioButton *>(QStringLiteral("slidesAll"))->setChecked(true);
            QCOMPARE(d->findChild<QSpinBox *>(QStringLiteral("rangeTo"))->maximum(), 4);
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("customShow"))->count(), 1);
            d->reject();
        });
        w.act("show.setUp")->trigger();
        QVERIFY(opened);
        QVERIFY(!ed->doc()->appData.contains(QStringLiteral("showSettings")));   // Cancel changed nothing
        // A kiosk with slides 2 to 3 and no animation.
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            d->findChild<QRadioButton *>(QStringLiteral("typeKiosk"))->setChecked(true);
            QCheckBox *loop = d->findChild<QCheckBox *>(QStringLiteral("loop"));
            QVERIFY(loop->isChecked());     // a kiosk loops
            QVERIFY(!loop->isEnabled());
            QVERIFY(d->findChild<QRadioButton *>(QStringLiteral("advanceTimings"))->isChecked());
            QVERIFY(!d->findChild<QRadioButton *>(QStringLiteral("advanceManual"))->isEnabled());
            d->findChild<QCheckBox *>(QStringLiteral("noAnimation"))->setChecked(true);
            d->findChild<QRadioButton *>(QStringLiteral("slidesRange"))->setChecked(true);
            d->findChild<QSpinBox *>(QStringLiteral("rangeFrom"))->setValue(2);
            d->findChild<QSpinBox *>(QStringLiteral("rangeTo"))->setValue(3);
            d->findChild<QComboBox *>(QStringLiteral("laserColor"))->setCurrentIndex(1);   // green
            d->accept();
        });
        w.act("show.setUp")->trigger();
        deck::ShowSettings s = deck::showSettingsOf(*ed->doc());
        QCOMPARE(int(s.type), int(deck::ShowSettings::Type::Kiosk));
        QCOMPARE(int(s.slides), int(deck::ShowSettings::Slides::Range));
        QCOMPARE(s.from, 2);
        QCOMPARE(s.to, 3);
        QVERIFY(!s.animation);
        QVERIFY(s.loop);
        QVERIFY(s.useTimings);
        QCOMPARE(s.laserColor.green(), 255);
        QCOMPARE(s.laserColor.red(), 0);
        QVERIFY(ed->isModified());
        ed->undo();
        QVERIFY(!ed->doc()->appData.contains(QStringLiteral("showSettings")));
        ed->redo();
        QCOMPARE(int(deck::showSettingsOf(*ed->doc()).type), int(deck::ShowSettings::Type::Kiosk));
        // The dialog opens on what is set; a custom show and the pen's color.
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            QVERIFY(d->findChild<QRadioButton *>(QStringLiteral("typeKiosk"))->isChecked());
            QVERIFY(d->findChild<QRadioButton *>(QStringLiteral("slidesRange"))->isChecked());
            QCOMPARE(d->findChild<QSpinBox *>(QStringLiteral("rangeFrom"))->value(), 2);
            QCOMPARE(d->findChild<QSpinBox *>(QStringLiteral("rangeTo"))->value(), 3);
            QVERIFY(d->findChild<QCheckBox *>(QStringLiteral("noAnimation"))->isChecked());
            QCOMPARE(d->findChild<QComboBox *>(QStringLiteral("laserColor"))->currentIndex(), 1);
            d->findChild<QRadioButton *>(QStringLiteral("typeSpeaker"))->setChecked(true);
            QVERIFY(d->findChild<QCheckBox *>(QStringLiteral("loop"))->isEnabled());
            d->findChild<QRadioButton *>(QStringLiteral("slidesCustom"))->setChecked(true);
            d->findChild<QRadioButton *>(QStringLiteral("advanceManual"))->setChecked(true);
            d->findChild<QCheckBox *>(QStringLiteral("loop"))->setChecked(true);
            d->accept();
        });
        w.act("show.setUp")->trigger();
        s = deck::showSettingsOf(*ed->doc());
        QCOMPARE(int(s.type), int(deck::ShowSettings::Type::Speaker));
        QCOMPARE(int(s.slides), int(deck::ShowSettings::Slides::Custom));
        QCOMPARE(s.customShow, qint64(3));
        QVERIFY(!s.useTimings);
        QVERIFY(s.loop);
        QVERIFY(w.act("show.useTimings")->isChecked() == false);
        // From Beginning follows the set up (the custom show, looping); From Current Slide plays every slide from there.
        w.act("show.fromStart")->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show);
        QCOMPARE(show->playedSlides(), (QList<int>{1, 3}));
        QCOMPARE(show->currentSlide(), 1);
        QVERIFY(show->settings().looping());
        show->close();
        QTRY_VERIFY(!w.slideShow());
        ed->setCurrentPage(2);
        w.act("show.fromCurrent")->trigger();
        show = w.slideShow();
        QVERIFY(show);
        QCOMPARE(show->currentSlide(), 2);
        QCOMPARE(show->playedSlides(), (QList<int>{0, 1, 2, 3}));
        QVERIFY(show->settings().looping());   // the rest of the set up still applies
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // A show by an individual is a window; the others are full screen.
    void windowShowTypesAreWindowOrFullScreen()
    {
        DeckWindow w;
        deck::ShowSettings s;
        s.type = deck::ShowSettings::Type::Individual;
        w.editor()->beginChange(QStringLiteral("test"));
        deck::setShowSettings(*w.editor()->doc(), s);
        w.editor()->endChange();
        w.startShow(0);
        SlideShow *show = w.slideShow();
        QVERIFY(show);
        QVERIFY(show->isWindowed());
        QVERIFY(!show->isFullScreen());
        QVERIFY(!(show->windowFlags() & Qt::FramelessWindowHint));
        QVERIFY(!show->presenterView());
        show->close();
        QTRY_VERIFY(!w.slideShow());
        s.type = deck::ShowSettings::Type::Kiosk;
        w.editor()->beginChange(QStringLiteral("test"));
        deck::setShowSettings(*w.editor()->doc(), s);
        w.editor()->endChange();
        w.startShow(0);
        show = w.slideShow();
        QVERIFY(show);
        QVERIFY(show->isKiosk());
        QVERIFY(show->isFullScreen());
        show->close();
        QTRY_VERIFY(!w.slideShow());
        s.type = deck::ShowSettings::Type::Speaker;
        w.editor()->beginChange(QStringLiteral("test"));
        deck::setShowSettings(*w.editor()->doc(), s);
        w.editor()->endChange();
        w.startShow(0);
        show = w.slideShow();
        QVERIFY(show && show->isFullScreen() && !show->isWindowed());
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // Custom Shows: the dialog lists the shows, and New, Edit, Copy, and Remove change them; Show plays one. Making one asks for a name
    // and picks slides from the presentation in any order.
    void customShowsDialogMakesEditsCopiesAndDeletes()
    {
        auto d = lettered(4);
        deck::CustomShow existing;
        existing.id = 0;
        existing.name = QStringLiteral("First");
        existing.slides = {d->pages[0]->id, d->pages[1]->id};
        // The editor of one show.
        {
            CustomShowEditDialog edit(*d, QVector<deck::CustomShow>{existing}, nullptr);
            QCOMPARE(edit.findChild<QLineEdit *>(QStringLiteral("showName"))->text(), QStringLiteral("Custom Show 1"));
            QListWidget *all = edit.findChild<QListWidget *>(QStringLiteral("slidesInPresentation"));
            QListWidget *mine = edit.findChild<QListWidget *>(QStringLiteral("slidesInShow"));
            QVERIFY(all && mine);
            QCOMPARE(all->count(), 4);
            QCOMPARE(mine->count(), 0);
            QAbstractButton *ok = edit.findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok);
            QVERIFY(!ok->isEnabled());   // no slide yet
            QAbstractButton *add = edit.findChild<QAbstractButton *>(QStringLiteral("addSlide"));
            QAbstractButton *remove = edit.findChild<QAbstractButton *>(QStringLiteral("removeSlide"));
            QAbstractButton *up = edit.findChild<QAbstractButton *>(QStringLiteral("moveUp"));
            QAbstractButton *down = edit.findChild<QAbstractButton *>(QStringLiteral("moveDown"));
            QVERIFY(add && remove && up && down);
            QVERIFY(!remove->isEnabled());
            all->item(3)->setSelected(true);
            all->item(0)->setSelected(true);
            add->click();   // the selected ones, in the order the presentation has them
            QCOMPARE(mine->count(), 2);
            all->clearSelection();
            all->item(0)->setSelected(true);
            add->click();   // a slide twice
            QCOMPARE(mine->count(), 3);
            QVERIFY(ok->isEnabled());
            auto order = [&] {
                QStringList out;
                for (int i = 0; i < mine->count(); ++i) out << mine->item(i)->data(Qt::UserRole).toString();
                return out;
            };
            QCOMPARE(order(), (QStringList{d->pages[0]->id, d->pages[3]->id, d->pages[0]->id}));
            mine->setCurrentRow(2);
            up->click();
            QCOMPARE(order(), (QStringList{d->pages[0]->id, d->pages[0]->id, d->pages[3]->id}));
            mine->setCurrentRow(0);
            down->click();
            down->click();
            QCOMPARE(order(), (QStringList{d->pages[0]->id, d->pages[3]->id, d->pages[0]->id}));
            mine->setCurrentRow(1);
            remove->click();
            QCOMPARE(order(), (QStringList{d->pages[0]->id, d->pages[0]->id}));
            // The name must be there and not another show's.
            QLineEdit *name = edit.findChild<QLineEdit *>(QStringLiteral("showName"));
            name->setText(QStringLiteral("  "));
            QVERIFY(!ok->isEnabled());
            name->setText(QStringLiteral("FIRST"));
            QVERIFY(!ok->isEnabled());   // the same as another's, in other letters
            name->setText(QStringLiteral("Mine"));
            QVERIFY(ok->isEnabled());
            ok->click();
            QCOMPARE(edit.result(), int(QDialog::Accepted));
            const deck::CustomShow made = edit.show();
            QCOMPARE(made.name, QStringLiteral("Mine"));
            QCOMPARE(made.slides, (QStringList{d->pages[0]->id, d->pages[0]->id}));
        }
        // Editing one starts from it, and the name it has is its own to keep.
        {
            CustomShowEditDialog edit(*d, QVector<deck::CustomShow>{existing}, &existing);
            QCOMPARE(edit.findChild<QLineEdit *>(QStringLiteral("showName"))->text(), QStringLiteral("First"));
            QCOMPARE(edit.findChild<QListWidget *>(QStringLiteral("slidesInShow"))->count(), 2);
            QVERIFY(edit.findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->isEnabled());
            QCOMPARE(edit.show().id, existing.id);
        }
        // The list.
        CustomShowsDialog dlg(*d, QVector<deck::CustomShow>{existing});
        QListWidget *list = dlg.findChild<QListWidget *>(QStringLiteral("customShowList"));
        QVERIFY(list);
        QCOMPARE(list->count(), 1);
        QAbstractButton *edit = dlg.findChild<QAbstractButton *>(QStringLiteral("editShow"));
        QAbstractButton *copy = dlg.findChild<QAbstractButton *>(QStringLiteral("copyShow"));
        QAbstractButton *remove = dlg.findChild<QAbstractButton *>(QStringLiteral("removeShow"));
        QAbstractButton *play = dlg.findChild<QAbstractButton *>(QStringLiteral("playShow"));
        QAbstractButton *create = dlg.findChild<QAbstractButton *>(QStringLiteral("newShow"));
        QVERIFY(edit && copy && remove && play && create);
        list->setCurrentRow(0);
        // Copy: the same slides under a new name and id.
        copy->click();
        QCOMPARE(list->count(), 2);
        QCOMPARE(dlg.shows().size(), 2);
        QCOMPARE(dlg.shows()[1].slides, existing.slides);
        QVERIFY(dlg.shows()[1].id != existing.id);
        QVERIFY(dlg.shows()[1].name != existing.name);
        QVERIFY2(dlg.shows()[1].name.contains(QStringLiteral("First")), qPrintable(dlg.shows()[1].name));
        // New: the editor opens, and what it makes is added.
        QTimer::singleShot(0, [&] {
            auto *e = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!e) return;
            QCOMPARE(e->objectName(), QStringLiteral("customShowEditDialog"));
            e->findChild<QLineEdit *>(QStringLiteral("showName"))->setText(QStringLiteral("Fresh"));
            e->findChild<QListWidget *>(QStringLiteral("slidesInPresentation"))->item(2)->setSelected(true);
            e->findChild<QAbstractButton *>(QStringLiteral("addSlide"))->click();
            e->accept();
        });
        create->click();
        QCOMPARE(dlg.shows().size(), 3);
        QCOMPARE(dlg.shows()[2].name, QStringLiteral("Fresh"));
        QCOMPARE(dlg.shows()[2].slides, (QStringList{d->pages[2]->id}));
        // Edit: changes the one selected.
        list->setCurrentRow(2);
        QTimer::singleShot(0, [&] {
            auto *e = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!e) return;
            QCOMPARE(e->findChild<QLineEdit *>(QStringLiteral("showName"))->text(), QStringLiteral("Fresh"));
            e->findChild<QLineEdit *>(QStringLiteral("showName"))->setText(QStringLiteral("Fresher"));
            e->accept();
        });
        const qint64 freshId = dlg.shows()[2].id;
        edit->click();
        QCOMPARE(dlg.shows()[2].name, QStringLiteral("Fresher"));
        QCOMPARE(dlg.shows()[2].id, freshId);
        // Cancelling the editor changes nothing.
        QTimer::singleShot(0, [&] {
            if (auto *e = qobject_cast<QDialog *>(QApplication::activeModalWidget())) e->reject();
        });
        create->click();
        QCOMPARE(dlg.shows().size(), 3);
        // Remove.
        list->setCurrentRow(1);
        remove->click();
        QCOMPARE(dlg.shows().size(), 2);
        QCOMPARE(dlg.shows()[1].name, QStringLiteral("Fresher"));
        // Show: plays the selected one.
        QCOMPARE(dlg.playId(), qint64(-1));
        list->setCurrentRow(1);
        play->click();
        QCOMPARE(dlg.result(), int(QDialog::Accepted));
        QCOMPARE(dlg.playId(), dlg.shows()[1].id);
    }

    // The window applies the dialog: one step that undoes, and what is made is saved and read back.
    void windowCustomShowsDialogIsOneStep()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            QCOMPARE(d->objectName(), QStringLiteral("customShowsDialog"));
            QTimer::singleShot(0, [&] {
                auto *e = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!e) return;
                e->findChild<QLineEdit *>(QStringLiteral("showName"))->setText(QStringLiteral("Tour"));
                QListWidget *all = e->findChild<QListWidget *>(QStringLiteral("slidesInPresentation"));
                all->item(2)->setSelected(true);
                all->item(1)->setSelected(true);
                e->findChild<QAbstractButton *>(QStringLiteral("addSlide"))->click();
                e->accept();
            });
            d->findChild<QAbstractButton *>(QStringLiteral("newShow"))->click();
            d->accept();   // Close
        });
        w.act("show.customShows")->trigger();
        QVector<deck::CustomShow> shows = deck::customShowsOf(*ed->doc());
        QCOMPARE(shows.size(), 1);
        QCOMPARE(shows[0].name, QStringLiteral("Tour"));
        QCOMPARE(deck::customShowSlides(*ed->doc(), shows[0]), (QList<int>{1, 2}));
        QVERIFY(ed->isModified());
        ed->undo();
        QVERIFY(deck::customShowsOf(*ed->doc()).isEmpty());
        ed->redo();
        QCOMPARE(deck::customShowsOf(*ed->doc()).size(), 1);
        // Close with nothing changed is no step.
        ed->flushTyping();
        ed->undo();   // the step made is gone, and Close with nothing changed makes none
        QVERIFY(deck::customShowsOf(*ed->doc()).isEmpty());
        QTimer::singleShot(0, [&] {
            if (auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget())) d->accept();
        });
        w.act("show.customShows")->trigger();
        QVERIFY(deck::customShowsOf(*ed->doc()).isEmpty());
        ed->redo();
        QCOMPARE(deck::customShowsOf(*ed->doc()).size(), 1);
        // Show plays the one selected, and the dialog closes.
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            d->findChild<QListWidget *>(QStringLiteral("customShowList"))->setCurrentRow(0);
            d->findChild<QAbstractButton *>(QStringLiteral("playShow"))->click();
        });
        w.act("show.customShows")->trigger();
        QVERIFY(w.slideShow());
        QCOMPARE(w.slideShow()->playedSlides(), (QList<int>{1, 2}));
        w.slideShow()->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // When a show ends with ink drawn, the window asks whether to keep it. Keep puts one freeform line on its slide for each stroke, as one
    // step that undoes; Discard leaves the slides as they were.
    void windowAsksWhetherToKeepInk()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        QStringList asked;
        bool keep = true;
        w.setShowQuestion([&](const QString &q) {
            asked << q;
            return keep;
        });
        const int before0 = int(ed->doc()->pages[0]->items.size()), before1 = int(ed->doc()->pages[1]->items.size());
        auto draw = [&](int slide, QPointF from, QPointF to, bool highlight) {
            SlideShow *show = w.slideShow();
            show->goTo(slide);
            show->setPointer(highlight ? SlideShow::Pointer::Highlighter : SlideShow::Pointer::Pen);
            show->pressAt(from);
            show->moveTo((from + to) / 2 + QPointF(0, 10));
            show->moveTo(to);
            show->releasePointer();
        };
        // A show with no ink asks nothing.
        w.startShow(0);
        w.slideShow()->close();
        QTRY_VERIFY(!w.slideShow());
        QCoreApplication::processEvents();
        QCoreApplication::processEvents();
        QVERIFY(asked.isEmpty());
        // Kept.
        w.startShow(0);
        draw(0, QPointF(100, 100), QPointF(300, 150), false);
        draw(0, QPointF(100, 300), QPointF(400, 300), true);
        draw(1, QPointF(50, 50), QPointF(200, 80), false);
        QCOMPARE(w.slideShow()->inkCount(), 3);
        w.slideShow()->close();
        QTRY_COMPARE(asked.size(), 1);
        QVERIFY2(asked[0].contains(QStringLiteral("ink annotations")), qPrintable(asked[0]));
        QTRY_COMPARE(int(ed->doc()->pages[0]->items.size()), before0 + 2);
        QCOMPARE(int(ed->doc()->pages[1]->items.size()), before1 + 1);
        const ShapeItem *pen = static_cast<const ShapeItem *>(ed->doc()->pages[0]->items[before0].get());
        const ShapeItem *marker = static_cast<const ShapeItem *>(ed->doc()->pages[0]->items[before0 + 1].get());
        QVERIFY(pen->type() == ItemType::Shape && !pen->customPath.isEmpty());
        QCOMPARE(pen->stroke.transparency, 0.0);
        QCOMPARE(marker->stroke.transparency, 0.5);
        QVERIFY(marker->stroke.width > pen->stroke.width * 5);
        QVERIFY(ed->isModified());
        // One step takes all of it away.
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before0);
        QCOMPARE(int(ed->doc()->pages[1]->items.size()), before1);
        ed->redo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before0 + 2);
        ed->undo();
        // Discarded.
        keep = false;
        asked.clear();
        w.startShow(0);
        draw(0, QPointF(100, 100), QPointF(300, 150), false);
        w.slideShow()->close();
        QTRY_COMPARE(asked.size(), 1);
        QCoreApplication::processEvents();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before0);
        // The slides can be saved with the kept ink and the file reads back with it.
        keep = true;
        w.startShow(0);
        draw(0, QPointF(100, 100), QPointF(300, 150), false);
        w.slideShow()->close();
        QTRY_COMPARE(int(ed->doc()->pages[0]->items.size()), before0 + 1);
        QString err;
        auto back = deck::importPresentation(deck::exportPresentation(*ed->doc(), &err), &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages[0]->items.size()), before0 + 1);
    }

    // Rehearse Timings plays the show from the first slide with the toolbar; at the end it asks whether to keep the times. Kept, each
    // slide's After is its time, Use Timings turns on, and it is one step that undoes.
    void windowRehearseTimingsKeepsTheTimes()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        QVERIFY(w.act("show.rehearse"));
        // The show's own setting is off, and a slide already has a time.
        deck::ShowSettings s;
        s.useTimings = false;
        ed->beginChange(QStringLiteral("test"));
        deck::setShowSettings(*ed->doc(), s);
        deck::Transition t;
        t.advanceAfter = 99;
        deck::setTransition(*ed->doc()->pages[0], t);
        ed->endChange();
        QStringList asked;
        bool keep = true;
        w.setShowQuestion([&](const QString &q) {
            asked << q;
            return keep;
        });
        ed->setCurrentPage(2);   // it starts at the first slide anyway
        w.act("show.rehearse")->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show);
        QVERIFY(show->isRecording());
        QCOMPARE(show->currentSlide(), 0);
        QVERIFY(show->rehearseBar());
        qint64 now = 0;
        show->setClock([&now] { return now; });
        now += 2500;
        show->next();
        now += 4000;
        show->next();
        now += 1040;
        show->next();
        QCOMPARE(show->currentSlide(), -1);
        show->next();   // the end screen is clicked away
        QTRY_COMPARE(asked.size(), 1);
        QVERIFY2(asked[0].contains(QStringLiteral("0:00:07")), qPrintable(asked[0]));
        QTRY_VERIFY(deck::showSettingsOf(*ed->doc()).useTimings);
        auto after = [&](int i) { return deck::transitionOf(*ed->doc()->pages[i]).advanceAfter; };
        QVERIFY(after(0) && after(1) && after(2));
        QCOMPARE(*after(0), 2.5);
        QCOMPARE(*after(1), 4.0);
        QCOMPARE(*after(2), 1.0);
        QVERIFY(ed->isModified());
        QTRY_VERIFY(!w.slideShow());
        // The Transitions tab's After box shows the time.
        ed->setCurrentPage(1);
        auto *box = w.ribbon()->findChild<QCheckBox *>(QStringLiteral("advanceAfter"));
        auto *seconds = w.ribbon()->findChild<QDoubleSpinBox *>(QStringLiteral("advanceAfterSeconds"));
        QTRY_VERIFY(box->isChecked());
        QCOMPARE(seconds->value(), 4.0);
        // One step undoes the times and the setting.
        ed->undo();
        QVERIFY(!deck::showSettingsOf(*ed->doc()).useTimings);
        QCOMPARE(*after(0), 99.0);
        QVERIFY(!after(1));
        // Not kept: nothing changes.
        keep = false;
        asked.clear();
        w.act("show.rehearse")->trigger();
        show = w.slideShow();
        QVERIFY(show);
        show->setClock([&now] { return now; });
        now += 3000;
        show->close();   // Esc part way: asks too
        QTRY_COMPARE(asked.size(), 1);
        QCoreApplication::processEvents();
        QVERIFY(!after(1));
        QVERIFY(!deck::showSettingsOf(*ed->doc()).useTimings);
        QCOMPARE(*after(0), 99.0);
        // A time under a tenth of a second is a tenth, and the slides not reached keep theirs.
        keep = true;
        asked.clear();
        w.act("show.rehearse")->trigger();
        show = w.slideShow();
        show->setClock([&now] { return now; });
        now += 20;
        show->next();
        show->close();
        QTRY_COMPARE(asked.size(), 1);
        QTRY_VERIFY(deck::showSettingsOf(*ed->doc()).useTimings);
        QCOMPARE(*after(0), 0.1);
        QVERIFY(!after(2));
    }

    // F1 in each of the show's dialogs opens the Help topic of the show, in a window over the dialog; the Help finds them by their titles.
    void showDialogsOpenHelpOnF1()
    {
        DeckWindow w;
        auto d = lettered(2);
        SetUpShowDialog setUp(deck::ShowSettings(), QVector<deck::CustomShow>(), 2, w.editor()->doc()->colors, &w);
        CustomShowsDialog shows(*d, QVector<deck::CustomShow>(), &w);
        CustomShowEditDialog edit(*d, QVector<deck::CustomShow>(), nullptr, &w);
        for (QDialog *dlg : {static_cast<QDialog *>(&setUp), static_cast<QDialog *>(&shows), static_cast<QDialog *>(&edit)}) {
            QCOMPARE(help::contextTopic(QStringLiteral("dialog:") + dlg->windowTitle()), QStringLiteral("show"));
            dlg->show();
            QTest::keyClick(dlg, Qt::Key_F1);
            auto *hw = dlg->findChild<HelpWindow *>(QString(), Qt::FindDirectChildrenOnly);
            QVERIFY2(hw, qPrintable(dlg->windowTitle()));
            QCOMPARE(hw->view()->currentTopic(), QStringLiteral("show"));
            hw->close();
            QTRY_VERIFY(!dlg->findChild<HelpWindow *>());
        }
        // Esc in the list closes it, keeping what it holds.
        shows.show();
        QTest::keyClick(&shows, Qt::Key_Escape);
        QCOMPARE(shows.result(), int(QDialog::Accepted));
    }

    // A show in a window leaves the deck editable, so the show follows the slides by their pages: a slide taken away, one before it
    // gone, and slides moved or put back leave it on the slide it was on (or the next, when that one is gone), and nothing reads a
    // slide that isn't there.
    void windowedShowFollowsTheDecksPages()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        w.newSlide();   // four slides
        deck::ShowSettings s;
        s.type = deck::ShowSettings::Type::Individual;
        ed->beginChange(QStringLiteral("test"));
        deck::setShowSettings(*ed->doc(), s);
        ed->endChange();
        w.setShowQuestion([](const QString &) { return true; });
        // The slide the show is on is taken away: it lands on the one that took its place, and moves on from there.
        w.startShow(3);
        SlideShow *show = w.slideShow();
        QVERIFY(show && show->isWindowed());
        QCOMPARE(show->currentSlide(), 3);
        ed->deletePage(3);
        QCOMPARE(show->currentSlide(), 2);
        QCOMPARE(show->playedSlides(), (QList<int>{0, 1, 2}));
        show->resize(960, 540);
        QTest::mouseClick(show, Qt::LeftButton, Qt::NoModifier, QPoint(5, 530));
        QCOMPARE(show->currentSlide(), -1);   // the end: nothing past the last slide was read
        QTest::keyClick(show, Qt::Key_Left);
        QCOMPARE(show->currentSlide(), 2);
        show->close();
        QTRY_VERIFY(!w.slideShow());
        // A slide before it goes, one is moved, and the change is undone: it stays on its own slide.
        QCOMPARE(int(ed->doc()->pages.size()), 3);
        ed->undo();
        QCOMPARE(int(ed->doc()->pages.size()), 4);
        const QString third = ed->doc()->pages[2]->id;
        w.startShow(2);
        show = w.slideShow();
        QVERIFY(show);
        show->setPointer(SlideShow::Pointer::Pen);
        show->pressAt(QPointF(100, 100));
        show->moveTo(QPointF(200, 120));
        show->releasePointer();
        ed->deletePage(0);
        QCOMPARE(ed->doc()->pages[1]->id, third);
        QCOMPARE(show->currentSlide(), 1);
        QCOMPARE(int(show->ink(1).size()), 1);   // the ink went with its slide
        QCOMPARE(int(show->ink(2).size()), 0);
        ed->movePage(1, 2);
        QCOMPARE(ed->doc()->pages[2]->id, third);
        QCOMPARE(show->currentSlide(), 2);
        QCOMPARE(int(show->ink(2).size()), 1);
        ed->undo();   // moved back
        QCOMPARE(show->currentSlide(), 1);
        ed->undo();   // the deleted slide is back before it
        QCOMPARE(int(ed->doc()->pages.size()), 4);
        QCOMPARE(show->currentSlide(), 2);
        QCOMPARE(show->playedSlides(), (QList<int>{0, 1, 2, 3}));
        show->close();
        QTRY_VERIFY(!w.slideShow());
        // A custom show: the slides it lists are followed too, and a place that goes is passed to the next.
        deck::CustomShow c;
        c.id = 5;
        c.name = QStringLiteral("Around");
        c.slides = {ed->doc()->pages[3]->id, ed->doc()->pages[0]->id, ed->doc()->pages[3]->id};
        ed->beginChange(QStringLiteral("test"));
        deck::setCustomShows(*ed->doc(), {c});
        ed->endChange();
        const QString first = ed->doc()->pages[0]->id;
        w.startCustomShow(5);
        show = w.slideShow();
        QVERIFY(show);
        show->next();
        QCOMPARE(show->currentSlide(), 0);   // the middle place: slide 1
        ed->deletePage(2);                  // a slide the show does not list
        QCOMPARE(show->currentSlide(), 0);
        QCOMPARE(show->playedSlides(), (QList<int>{2, 0, 2}));
        ed->deletePage(0);                  // the slide it is on: the next place, which is the last slide again
        QCOMPARE(show->playedSlides(), (QList<int>{1, 1}));
        QCOMPARE(show->currentSlide(), 1);
        show->next();
        QCOMPARE(show->currentSlide(), -1);
        QVERIFY(ed->doc()->pages[0]->id != first);
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // The rehearsal window (Alt+F5) and Presenter View beside it follow the deck's pages too.
    void rehearsalWindowFollowsTheDecksPages()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        ed->setCurrentPage(2);
        w.act("show.presenterView")->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show && show->presenterView());
        QCOMPARE(show->currentSlide(), 2);
        ed->deletePage(2);
        QCOMPARE(show->currentSlide(), 1);
        QCOMPARE(show->presenterView()->counterText(), QStringLiteral("Slide 2 of 2"));
        show->resize(640, 360);
        QTest::mouseClick(show, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
        QCOMPARE(show->currentSlide(), -1);
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // A new presentation put in the window while a show is open ends the show, with nothing read from the old one.
    void windowedShowEndsWhenTheDocumentIsReplaced()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        deck::ShowSettings s;
        s.type = deck::ShowSettings::Type::Individual;
        ed->beginChange(QStringLiteral("test"));
        deck::setShowSettings(*ed->doc(), s);
        ed->endChange();
        QStringList asked;
        w.setShowQuestion([&](const QString &q) {
            asked << q;
            return true;
        });
        w.startShow(1);
        QPointer<SlideShow> show = w.slideShow();
        QVERIFY(show);
        show->setPointer(SlideShow::Pointer::Pen);
        show->pressAt(QPointF(100, 100));
        show->moveTo(QPointF(200, 120));
        show->releasePointer();
        ed->setDocument(deck::newPresentation());
        QTRY_VERIFY(show.isNull() || !show->isVisible());
        QTRY_VERIFY(!w.slideShow());
        QCOMPARE(ed->currentPage(), 0);
        QCoreApplication::processEvents();
        QVERIFY(asked.isEmpty());   // the ink was on slides this presentation does not have
    }

    // Ink is kept on the slide it was drawn on, whatever happened to the slides' places, and not asked about when its slide is gone.
    void keptInkLandsOnItsOwnSlide()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        deck::ShowSettings s;
        s.type = deck::ShowSettings::Type::Individual;
        ed->beginChange(QStringLiteral("test"));
        deck::setShowSettings(*ed->doc(), s);
        ed->endChange();
        QStringList asked;
        w.setShowQuestion([&](const QString &q) {
            asked << q;
            return true;
        });
        auto draw = [&](SlideShow *show) {
            show->setPointer(SlideShow::Pointer::Pen);
            show->pressAt(QPointF(100, 100));
            show->moveTo(QPointF(200, 120));
            show->releasePointer();
        };
        // Ink on the third slide, and the first goes: it lands on the page it was drawn on.
        const QString third = ed->doc()->pages[2]->id;
        const int before = int(ed->doc()->pages[2]->items.size());
        w.startShow(2);
        draw(w.slideShow());
        ed->deletePage(0);
        w.slideShow()->close();
        QTRY_COMPARE(asked.size(), 1);
        QTRY_COMPARE(int(ed->doc()->pages[1]->items.size()), before + 1);
        QCOMPARE(ed->doc()->pages[1]->id, third);
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);
        // Ink on a slide that is then deleted is nothing to keep, and nothing is asked.
        asked.clear();
        w.startShow(1);
        draw(w.slideShow());
        ed->deletePage(1);
        w.slideShow()->close();
        QTRY_VERIFY(!w.slideShow());
        QCoreApplication::processEvents();
        QCoreApplication::processEvents();
        QVERIFY(asked.isEmpty());
        QCOMPARE(int(ed->doc()->pages.size()), 1);
    }

    // The mouse pointer shows while the times are recorded: the bar's buttons are clicked with it.
    void rehearsalShowsTheArrow()
    {
        auto d = showDeck(2);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        deck::ShowSettings s;
        show.setSettings(s);
        QCOMPARE(int(show.cursor().shape()), int(Qt::BlankCursor));
        show.setRecording(true);
        QVERIFY(show.cursor().shape() != Qt::BlankCursor);
        QVERIFY(show.rehearseBar()->cursor().shape() != Qt::BlankCursor);
        QVERIFY(show.rehearseBar()->findChild<QAbstractButton *>(QStringLiteral("rehearseNext"))->cursor().shape() != Qt::BlankCursor);
        show.resize(960, 540);
        QMouseEvent move(QEvent::MouseMove, QPointF(300, 300), QPointF(show.mapToGlobal(QPoint(300, 300))), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(&show, &move);
        QVERIFY(show.cursor().shape() != Qt::BlankCursor);
        show.setSettings(s);   // however the settings came
        QVERIFY(show.cursor().shape() != Qt::BlankCursor);
        show.setRecording(false);
        QCOMPARE(int(show.cursor().shape()), int(Qt::BlankCursor));
    }

    // A file's unnamed custom shows are named in one pass: a thousand take a moment, not half a minute.
    void unnamedCustomShowsAreNamedQuickly()
    {
        auto d = lettered(2);
        QString err;
        Exported base;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &base, &err), qPrintable(err));
        QDomDocument dom;
        QStringList rels;
        for (const QDomElement &s : office::children(office::child(rootOf(base.zip.value(QStringLiteral("ppt/presentation.xml")), &dom), office::ns::P, QStringLiteral("sldIdLst")), office::ns::P,
                                                     QStringLiteral("sldId")))
            rels << office::attrNs(s, office::ns::R, QStringLiteral("id"));
        QString shows = QStringLiteral("<p:custShowLst>");
        for (int i = 0; i < 1000; ++i) shows += QStringLiteral("<p:custShow name=\"\" id=\"%1\"><p:sldLst><p:sld r:id=\"%2\"/></p:sldLst></p:custShow>").arg(i).arg(rels[0]);
        shows += QStringLiteral("</p:custShowLst>");
        QMap<QString, QByteArray> zip = base.zip;
        QString xml = QString::fromUtf8(zip.value(QStringLiteral("ppt/presentation.xml")));
        xml.insert(xml.indexOf(QStringLiteral("<p:defaultTextStyle>")), shows);
        zip[QStringLiteral("ppt/presentation.xml")] = xml.toUtf8();
        ZipWriter z;
        for (auto it = zip.cbegin(); it != zip.cend(); ++it) z.add(it.key(), it.value(), true);
        const deck::ShowImportWork readBefore = deck::showImportWorkForTest();
        auto back = deck::importPresentation(z.finish(), &err);
        QVERIFY2(back, qPrintable(err));
        // Counted: the 1,000 shows keep their one slide each, once.
        QCOMPARE(deck::showImportWorkForTest().customShowEntriesKept - readBefore.customShowEntriesKept, qint64(1000));
        const QVector<deck::CustomShow> got = deck::customShowsOf(*back);
        QCOMPARE(int(got.size()), 1000);
        QSet<QString> names;
        for (const deck::CustomShow &s : got) names.insert(s.name.toLower());
        QCOMPARE(int(names.size()), 1000);   // each its own
        QCOMPARE(got[0].name, QStringLiteral("Custom Show 1"));
        QCOMPARE(got[999].name, QStringLiteral("Custom Show 1000"));
    }

    // A stroke being drawn stays on the slide it began on when the show moves on with the button held; so does an erasing.
    void strokeStaysOnItsSlideWhenTheShowMovesOn()
    {
        auto d = showDeck(3);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 1);
        show.setPointer(SlideShow::Pointer::Pen);
        show.pressAt(QPointF(100, 100));
        show.moveTo(QPointF(200, 100));
        show.releasePointer();   // a stroke on slide 2
        QCOMPARE(int(show.ink(1)[0].points.size()), 2);
        show.goTo(0);
        show.pressAt(QPointF(100, 300));
        show.moveTo(QPointF(150, 300));
        QTest::keyClick(&show, Qt::Key_Right);   // on with the button held
        QCOMPARE(show.currentSlide(), 1);
        QVERIFY(!show.isDrawing());
        show.moveTo(QPointF(300, 300));
        show.moveTo(QPointF(400, 300));
        show.releasePointer();
        QCOMPARE(int(show.ink(1).size()), 1);
        QCOMPARE(int(show.ink(1)[0].points.size()), 2);   // the other slide's stroke did not grow
        QCOMPARE(int(show.ink(0).size()), 1);
        QCOMPARE(int(show.ink(0)[0].points.size()), 2);
        // The same for the eraser, held down over a slide change.
        show.setPointer(SlideShow::Pointer::Eraser);
        show.pressAt(QPointF(5, 5));
        show.goTo(2);
        show.next();
        QVERIFY(show.currentSlide() == -1 || show.currentSlide() == 2);
        show.goTo(1);
        show.moveTo(QPointF(150, 100));   // over the stroke on slide 2
        show.releasePointer();
        QCOMPARE(int(show.ink(1).size()), 1);
        // And with a transition playing into a slide that has a stroke of its own.
        deck::setTransition(*d->pages[2], showTransition("fade", 30));
        show.setPointer(SlideShow::Pointer::Pen);
        show.goTo(2);
        show.pressAt(QPointF(100, 400));
        show.moveTo(QPointF(200, 400));
        show.releasePointer();
        show.goTo(1);
        show.pressAt(QPointF(100, 300));
        show.next();   // onto slide 3, with its transition
        QVERIFY(show.isAnimating());
        QVERIFY(!show.isDrawing());
        show.finishTransition();
        QCOMPARE(show.currentSlide(), 2);
        show.moveTo(QPointF(250, 250));
        show.moveTo(QPointF(300, 250));
        show.releasePointer();
        QCOMPARE(int(show.ink(2).size()), 1);
        QCOMPARE(int(show.ink(2)[0].points.size()), 2);
    }

    // A custom show that lists a slide twice: End, a number typed, and Go to Slide go to a place of it, not the first one.
    void customShowPlacesAreNotTheirSlides()
    {
        auto d = showDeck(5);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.setSlides({3, 0, 3}, true);
        QCOMPARE(show.currentSlide(), 3);
        // End is the last place: back from it is the place before.
        QTest::keyClick(&show, Qt::Key_End);
        QCOMPARE(show.currentSlide(), 3);
        QTest::keyClick(&show, Qt::Key_Left);
        QCOMPARE(show.currentSlide(), 0);
        // Home is the first place: forward from it is the second.
        QTest::keyClick(&show, Qt::Key_Home);
        QTest::keyClick(&show, Qt::Key_Right);
        QCOMPARE(show.currentSlide(), 0);
        // Go to Slide lists the places, and each goes to its own.
        QMenu *menu = show.buildMenu(&show);
        QMenu *go = menu->findChild<QMenu *>(QStringLiteral("goToSlideMenu"));
        QVERIFY(go);
        QCOMPARE(int(go->actions().size()), 3);
        QVERIFY(go->actions()[1]->isChecked());   // the place the show is at
        QVERIFY(!go->actions()[0]->isChecked() && !go->actions()[2]->isChecked());
        go->actions()[2]->trigger();
        QCOMPARE(show.currentSlide(), 3);
        QTest::keyClick(&show, Qt::Key_Left);
        QCOMPARE(show.currentSlide(), 0);   // that was the last place
        go->actions()[0]->trigger();
        QTest::keyClick(&show, Qt::Key_Right);
        QCOMPARE(show.currentSlide(), 0);   // that was the first
        delete menu;
        // A number goes to the next place that slide has, going round: typing it again goes to the other.
        QTest::keyClick(&show, Qt::Key_Home);
        QTest::keyClick(&show, Qt::Key_Right);   // the middle place, slide 1
        QTest::keyClicks(&show, "4");
        QTest::keyClick(&show, Qt::Key_Return);
        QCOMPARE(show.currentSlide(), 3);
        QTest::keyClick(&show, Qt::Key_Left);
        QCOMPARE(show.currentSlide(), 0);   // it was the last place
        QTest::keyClick(&show, Qt::Key_Right);   // the last place
        QTest::keyClicks(&show, "4");
        QTest::keyClick(&show, Qt::Key_Return);
        QCOMPARE(show.currentSlide(), 3);        // round to the first place
        QTest::keyClick(&show, Qt::Key_Right);
        QCOMPARE(show.currentSlide(), 0);        // so the second is next
    }

    // A custom show's id is never used again: a show taken out and one made after it are two, and what was set to play the first plays
    // every slide, not the second. The settings never name a show that is not there.
    void customShowIdsAreNotReused()
    {
        auto d = lettered(4);
        deck::CustomShow a, b, c;
        a.id = 0;
        a.name = QStringLiteral("A");
        a.slides = {d->pages[0]->id};
        b.id = 1;
        b.name = QStringLiteral("B");
        b.slides = {d->pages[1]->id};
        c.id = 2;
        c.name = QStringLiteral("C");
        c.slides = {d->pages[2]->id};
        deck::setCustomShows(*d, {a, b, c});
        deck::ShowSettings s;
        s.slides = deck::ShowSettings::Slides::Custom;
        s.customShow = 2;
        deck::setShowSettings(*d, s);
        QCOMPARE(deck::showSettingsOf(*d).customShow, qint64(2));
        // C goes; the next show made is not number 2.
        deck::setCustomShows(*d, {a, b});
        QVERIFY(deck::newCustomShowId(*d, {a, b}) > 2);
        deck::CustomShow n;
        n.id = deck::newCustomShowId(*d, {a, b});
        n.name = QStringLiteral("New");
        n.slides = {d->pages[3]->id};
        deck::setCustomShows(*d, {a, b, n});
        QVERIFY(n.id != 2);
        // The show set to play C is a show that is not there: every slide plays, and the dialog and the file say so.
        QCOMPARE(int(deck::showSettingsOf(*d).slides), int(deck::ShowSettings::Slides::All));
        QVERIFY(deck::planOf(*d, deck::showSettingsOf(*d)).slides.isEmpty());
        QVERIFY(!deck::planOf(*d, deck::showSettingsOf(*d)).none);
        // Everything out and one made: still a new number.
        deck::setCustomShows(*d, {});
        QVERIFY(deck::newCustomShowId(*d, {}) > n.id);
        // A show whose slides are all deleted is gone, and what was set to play it plays every slide.
        deck::setCustomShows(*d, {a, b, n});
        s.customShow = n.id;
        deck::setShowSettings(*d, s);
        QCOMPARE(deck::showSettingsOf(*d).customShow, n.id);
        d->pages.removeAt(3);
        QCOMPARE(int(deck::showSettingsOf(*d).slides), int(deck::ShowSettings::Slides::All));
        QCOMPARE(deck::showSettingsOf(*d).customShow, qint64(-1));
        // And the ids stay as a file keeps them: read back, the next is past the largest.
        QString err;
        auto back = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY(deck::newCustomShowId(*back, deck::customShowsOf(*back)) > b.id);
    }

    // The window gives a new show an id no show had, and the setting that named a removed show is reset.
    void windowCustomShowRemovedAndMadeAgainIsAnotherShow()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        deck::CustomShow a, b;
        a.id = 0;
        a.name = QStringLiteral("A");
        a.slides = {ed->doc()->pages[0]->id};
        b.id = 1;
        b.name = QStringLiteral("B");
        b.slides = {ed->doc()->pages[1]->id};
        deck::ShowSettings s;
        s.slides = deck::ShowSettings::Slides::Custom;
        s.customShow = 1;
        ed->beginChange(QStringLiteral("test"));
        deck::setCustomShows(*ed->doc(), {a, b});
        deck::setShowSettings(*ed->doc(), s);
        ed->endChange();
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            QListWidget *list = d->findChild<QListWidget *>(QStringLiteral("customShowList"));
            list->setCurrentRow(1);
            d->findChild<QAbstractButton *>(QStringLiteral("removeShow"))->click();
            QTimer::singleShot(0, [&] {
                auto *e = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (!e) return;
                e->findChild<QLineEdit *>(QStringLiteral("showName"))->setText(QStringLiteral("B again"));
                e->findChild<QListWidget *>(QStringLiteral("slidesInPresentation"))->item(2)->setSelected(true);
                e->findChild<QAbstractButton *>(QStringLiteral("addSlide"))->click();
                e->accept();
            });
            d->findChild<QAbstractButton *>(QStringLiteral("newShow"))->click();
            d->accept();
        });
        w.act("show.customShows")->trigger();
        const QVector<deck::CustomShow> shows = deck::customShowsOf(*ed->doc());
        QCOMPARE(int(shows.size()), 2);
        QCOMPARE(shows[1].name, QStringLiteral("B again"));
        QVERIFY(shows[1].id != 1);
        QCOMPARE(int(deck::showSettingsOf(*ed->doc()).slides), int(deck::ShowSettings::Slides::All));
        QVERIFY(!ed->doc()->appData.value(QStringLiteral("showSettings")).toObject().value(QStringLiteral("slides")).toString().contains(QStringLiteral("custom")));
    }

    // Ink is drawn again from a picture of the strokes that are finished: a stroke of a hundred thousand points does not make every
    // repaint take over a second, and the stroke being drawn is added on top.
    void inkRepaintsFromAPictureOfFinishedStrokes()
    {
        auto d = showDeck(1);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.setPointer(SlideShow::Pointer::Pen);
        show.pressAt(QPointF(480, 270));
        for (int i = 1; i < 100000; ++i) {
            const double r = 20 + i * 0.0025, a = i * 0.012;
            show.moveTo(QPointF(480 + r * std::cos(a), 270 + r * 0.8 * std::sin(a)));
        }
        show.releasePointer();
        QVERIFY(show.ink(0)[0].points.size() > 50000);
        auto alike = [](const QImage &a, const QImage &b) {
            if (a.size() != b.size()) return false;
            int worst = 0;
            for (int y = 0; y < a.height(); ++y)
                for (int x = 0; x < a.width(); ++x) {
                    const QColor p = a.pixelColor(x, y), q = b.pixelColor(x, y);
                    worst = std::max({worst, std::abs(p.red() - q.red()), std::abs(p.green() - q.green()), std::abs(p.blue() - q.blue())});
                }
            return worst <= 4;
        };
        const QImage first = show.grab().toImage();   // the picture is made
        // Counted: a repaint draws none of the 100,000 points again (the finished stroke comes from its picture).
        const qint64 drawn = SlideShow::inkPointsDrawnForTest();
        const QImage again = show.grab().toImage();
        QCOMPARE(SlideShow::inkPointsDrawnForTest(), drawn);
        QCOMPARE(again, first);
        // A stroke being drawn over it: each repaint is the picture and that stroke.
        show.pressAt(QPointF(50, 50));
        for (int i = 1; i < 200; ++i) show.moveTo(QPointF(50 + i, 50 + (i % 7)));
        const qint64 drawnLive = SlideShow::inkPointsDrawnForTest();
        const QImage live = show.grab().toImage();
        // (Only the stroke being drawn, its 200 points, is drawn on top of the picture.)
        QVERIFY2(SlideShow::inkPointsDrawnForTest() - drawnLive <= 200, qPrintable(QString::number(SlideShow::inkPointsDrawnForTest() - drawnLive)));
        QVERIFY(live != first);
        show.releasePointer();
        // The same picture as drawing every stroke: once finished, the stroke is in the picture as it was on top of it.
        QVERIFY(alike(show.grab().toImage(), live));
        // An erased stroke is gone from it.
        show.setPointer(SlideShow::Pointer::Eraser);
        show.pressAt(QPointF(100, 50 + 3));
        show.releasePointer();
        QCOMPARE(int(show.ink(0).size()), 1);
        QVERIFY(alike(show.grab().toImage(), first));
    }

    // At the last slide of a looping show the slide Next shows is the first, as a click goes there.
    void presenterViewNextWrapsInALoopingShow()
    {
        auto d = showDeck(3);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        deck::ShowSettings s;
        s.loop = true;
        show.setSettings(s);
        PresenterView pv(&show);
        show.goTo(2);
        QCOMPARE(show.slideAfter(), 0);
        QCOMPARE(pv.nextSlide(), 0);
        show.next();
        QCOMPARE(show.currentSlide(), 0);
        SlideShow plain(d.get(), &cache, 0);
        plain.goTo(2);
        QCOMPARE(plain.slideAfter(), -1);
    }

    // A slide title with an ampersand is not read as a menu key.
    void goToSlideMenuKeepsAmpersands()
    {
        auto d = showDeck(2);
        d->storyDoc(static_cast<TextItem *>(d->pages[1]->items[0].get())->storyId)->setPlainText(QStringLiteral("Q&A"));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        QMenu *menu = show.buildMenu(&show);
        QMenu *go = menu->findChild<QMenu *>(QStringLiteral("goToSlideMenu"));
        QVERIFY(go);
        QCOMPARE(int(go->actions().size()), 2);
        QVERIFY2(go->actions()[1]->text().contains(QStringLiteral("Q&&A")), qPrintable(go->actions()[1]->text()));
        delete menu;
    }

    // A range past the end of the deck is clamped to it; wholly past, it plays nothing, and the window says so; it does not play every slide.
    void rangePastTheDeckPlaysNothing()
    {
        auto d = lettered(5);
        deck::ShowSettings s;
        s.slides = deck::ShowSettings::Slides::Range;
        s.from = 4;
        s.to = 9;
        QCOMPARE(deck::planOf(*d, s).slides, (QList<int>{3, 4}));
        QVERIFY(!deck::planOf(*d, s).none);
        s.from = 7;
        s.to = 9;
        const deck::ShowPlan none = deck::planOf(*d, s);
        QVERIFY(none.none);
        QVERIFY(none.slides.isEmpty());
        // The window plays nothing and says so.
        DeckWindow w;
        w.newSlide();
        QStringList told;
        w.setShowQuestion([&](const QString &text) {
            told << text;
            return true;
        });
        w.editor()->beginChange(QStringLiteral("test"));
        deck::setShowSettings(*w.editor()->doc(), s);
        w.editor()->endChange();
        w.act("show.fromStart")->trigger();
        QVERIFY(!w.slideShow());
        QCOMPARE(told.size(), 1);
        QVERIFY2(told[0].contains(QStringLiteral("7")), qPrintable(told[0]));
        // From Current Slide plays every slide as it always does.
        w.act("show.fromCurrent")->trigger();
        QVERIFY(w.slideShow());
        w.slideShow()->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // Set Up opens on "All" when the show it names is not there; the pen and the laser pointer's colors are shown by name, and a color
    // the lists lack by its code.
    void setUpDialogShowsMissingShowsAndColorsByName()
    {
        DeckWindow w;
        deck::CustomShow c;
        c.id = 3;
        c.name = QStringLiteral("Only");
        c.slides = {w.editor()->doc()->pages[0]->id};
        deck::ShowSettings s;
        s.slides = deck::ShowSettings::Slides::Custom;
        s.customShow = 99;
        SetUpShowDialog dlg(s, QVector<deck::CustomShow>{c}, 1, w.editor()->doc()->colors, &w);
        QVERIFY(dlg.findChild<QRadioButton *>(QStringLiteral("slidesAll"))->isChecked());
        QVERIFY(!dlg.findChild<QRadioButton *>(QStringLiteral("slidesCustom"))->isChecked());
        QCOMPARE(int(dlg.settings().slides), int(deck::ShowSettings::Slides::All));
        QCOMPARE(dlg.settings().customShow, qint64(-1));
        // Colors by name.
        auto *pen = dlg.findChild<QComboBox *>(QStringLiteral("penColor"));
        auto *laser = dlg.findChild<QComboBox *>(QStringLiteral("laserColor"));
        QVERIFY(pen && laser);
        QCOMPARE(pen->currentText(), QStringLiteral("Red"));
        QCOMPARE(laser->currentText(), QStringLiteral("Red"));
        int blue = -1;
        for (int i = 0; i < pen->count(); ++i)
            if (pen->itemText(i) == QLatin1String("Blue")) blue = i;
        QVERIFY(blue >= 0);
        pen->setCurrentIndex(blue);
        QCOMPARE(dlg.settings().penColor, QColor(0, 112, 192));
        // A color the lists lack is shown by its code, and kept.
        deck::ShowSettings odd;
        odd.penColor = QColor(0x12, 0x34, 0x56);
        odd.laserColor = QColor(0xAB, 0xCD, 0xEF);
        SetUpShowDialog other(odd, QVector<deck::CustomShow>(), 1, w.editor()->doc()->colors, &w);
        QCOMPARE(other.findChild<QComboBox *>(QStringLiteral("penColor"))->currentText(), QStringLiteral("#123456"));
        QCOMPARE(other.findChild<QComboBox *>(QStringLiteral("laserColor"))->currentText(), QStringLiteral("#ABCDEF"));
        QCOMPARE(other.settings().penColor, QColor(0x12, 0x34, 0x56));
        QCOMPARE(other.settings().laserColor, QColor(0xAB, 0xCD, 0xEF));
        // A pen color the program names (a theme's blue as an RGB) is the name, and what a name stands for.
        deck::ShowSettings blueSettings;
        blueSettings.penColor = QColor(0, 112, 192);
        SetUpShowDialog named(blueSettings, QVector<deck::CustomShow>(), 1, w.editor()->doc()->colors, &w);
        QCOMPARE(named.findChild<QComboBox *>(QStringLiteral("penColor"))->currentText(), QStringLiteral("Blue"));
    }

    // The questions that end a show are answered by Esc with the safe answer, since Esc has just ended the show: ink is discarded and
    // the new times are not kept.
    void escapeAnswersTheShowsQuestionsWithTheSafeOne()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        const int before = int(ed->doc()->pages[0]->items.size());
        QString escape;
        QTimer poll;
        poll.setInterval(10);
        QObject::connect(&poll, &QTimer::timeout, &poll, [&] {
            auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            if (!box || !box->escapeButton()) return;
            escape = box->escapeButton()->text();
            poll.stop();
            QTest::keyClick(box, Qt::Key_Escape);
        });
        // Ink.
        w.startShow(0);
        w.slideShow()->setPointer(SlideShow::Pointer::Pen);
        w.slideShow()->pressAt(QPointF(100, 100));
        w.slideShow()->moveTo(QPointF(200, 120));
        w.slideShow()->releasePointer();
        poll.start();
        w.slideShow()->close();
        QTRY_VERIFY(!escape.isEmpty());
        QCOMPARE(escape, QStringLiteral("Discard"));
        QTRY_VERIFY(!QApplication::activeModalWidget());
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);
        // Times.
        escape.clear();
        w.act("show.rehearse")->trigger();
        SlideShow *show = w.slideShow();
        qint64 now = 0;
        show->setClock([&now] { return now; });
        now += 3000;
        poll.start();
        show->close();
        QTRY_VERIFY(!escape.isEmpty());
        QCOMPARE(escape, QStringLiteral("No"));
        QTRY_VERIFY(!QApplication::activeModalWidget());
        QVERIFY(!deck::transitionOf(*ed->doc()->pages[0]).advanceAfter);
    }

    // A stroke of a hundred thousand points is thinned in a moment, however it winds.
    void inkThinningIsQuickOnSpirals()
    {
        deck::InkStroke spiral;
        for (int i = 0; i < 100000; ++i) {
            const double r = 5 + i * 0.004, a = i * 0.05;
            spiral.points << QPointF(480 + r * std::cos(a), 270 + r * std::sin(a));
        }
        spiral.color = QColor(255, 0, 0);
        spiral.width = 2.5;
        const qint64 looked = deck::inkPointsLookedAtForTest();
        const ItemPtr shape = deck::inkShape(spiral);
        QVERIFY(shape);
        // Counted: each point is looked at about twice, however the stroke winds (it would be the square, 10 billion, if each run began again at its first point).
        QVERIFY2(deck::inkPointsLookedAtForTest() - looked <= 3 * 100000, qPrintable(QString::number(deck::inkPointsLookedAtForTest() - looked)));
        const auto *line = static_cast<const ShapeItem *>(shape.get());
        QVERIFY(line->customPath.elementCount() > 100);
        QVERIFY(line->customPath.elementCount() < 100000);
        // It is still the line: the ends stay, and the points stay within the thinning's reach of the stroke.
        QCOMPARE(line->customPath.elementAt(0).x + line->rect.left(), spiral.points.first().x());
        QCOMPARE(line->customPath.elementAt(line->customPath.elementCount() - 1).y + line->rect.top(), spiral.points.last().y());
    }

    // The old "Rehearse with Presenter View" window plays as the presentation is set up: without animation, with its looping, and its colors.
    void rehearsalWindowFollowsTheSetUp()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        deck::ShowSettings s;
        s.animation = false;
        s.loop = true;
        s.type = deck::ShowSettings::Type::Kiosk;   // a window for rehearsing is never a kiosk
        s.laserColor = QColor(0, 255, 0);
        s.penColor = QColor(0, 112, 192);
        ed->beginChange(QStringLiteral("test"));
        deck::setShowSettings(*ed->doc(), s);
        ed->endChange();
        w.act("show.presenterView")->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show && show->presenterView());
        QVERIFY(!show->settings().animation);
        QVERIFY(show->settings().loop);
        QCOMPARE(show->laserColor(), QColor(0, 255, 0));
        QVERIFY(!show->isKiosk());
        QVERIFY(show->isWindowed());
        QTest::keyClick(show, Qt::Key_P, Qt::ControlModifier);   // a rehearsal window has a pen
        QCOMPARE(show->inkColor(), QColor(0, 112, 192));
        show->close();
        QTRY_VERIFY(!w.slideShow());
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

JP_TEST_CLASS(DeckSlideShowTests)
#include "tst_deck_slideshow.moc"
