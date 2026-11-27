// JeffDeck's tests of the slide show: settings, custom shows, ink, and rehearsal.
#include "app/storyedit.h"
#include "app/listmarkers.h"
#include "app/notespageview.h"
#include <QScrollBar>
#include <QTextEdit>
#include <QScopeGuard>
#include "deckhelpers.h"
#include "../withdialog.h"
#include "app/theme.h"

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
        const auto closeShow = qScopeGuard([&] { delete show.data(); });   // (a failed check must not leave the show running past the deck it reads)
        deck::ShowSettings s;
        s.type = deck::ShowSettings::Type::Kiosk;
        show->setSettings(s);
        show->setLinkPrompt([](const QString &, bool) { return true; });   // (a link asks at a kiosk too)
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
        const auto closeShow = qScopeGuard([&] { delete show.data(); });   // (a failed check must not leave the show running past the deck it reads)
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

    // Arrow Options: Automatic hides the arrow on a full screen and shows it in a window, Visible shows it always, Hidden never (Ctrl+U and Ctrl+H, and the menu under Pointer Options); the
    // rehearsal bar is clicked with the arrow, so it shows then whatever was chosen.
    void arrowOptionsChooseWhetherTheArrowShows()
    {
        const auto reset = qScopeGuard([] { SlideShow::forceArrowKeyModifier(Qt::NoModifier); });
        auto d = showDeck(3);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        const auto sessionBack = qScopeGuard([] { SlideShow::setSessionArrowShown(SlideShow::ArrowShown::Automatic); });   // (the choice lasts for the session)
        show.resize(960, 540);
        QCOMPARE(int(show.arrowShown()), int(SlideShow::ArrowShown::Automatic));
        QCOMPARE(show.cursor().shape(), Qt::BlankCursor);
        auto checked = [&](const char *id) {
            std::unique_ptr<QMenu> menu(show.pointerMenu(&show));
            QMenu *arrow = menu->findChild<QMenu *>(QStringLiteral("arrowOptionsMenu"));
            QVERIFY2(arrow, "no Arrow Options menu");
            QAction *a = arrow->findChild<QAction *>(QString::fromLatin1(id));
            QVERIFY2(a && a->isCheckable(), id);
            QVERIFY2(a->isChecked(), id);
            for (QAction *other : arrow->actions()) QVERIFY2(other == a || !other->isChecked(), id);
        };
        checked("arrowAutomatic");
        {
            std::unique_ptr<QMenu> menu(show.pointerMenu(&show));
            QAction *visible = menu->findChild<QAction *>(QStringLiteral("arrowVisible"));
            QVERIFY(visible);
            visible->trigger();
        }
        QCOMPARE(int(show.arrowShown()), int(SlideShow::ArrowShown::Visible));
        QCOMPARE(show.cursor().shape(), Qt::ArrowCursor);
        checked("arrowVisible");
        QTest::keyClick(&show, Qt::Key_H, SlideShow::arrowKeyModifier());
        QCOMPARE(int(show.arrowShown()), int(SlideShow::ArrowShown::Hidden));
        QCOMPARE(show.cursor().shape(), Qt::BlankCursor);
        checked("arrowHidden");
        QCOMPARE(show.currentSlide(), 0);   // (the keys changed nothing else)
        // In a window Automatic shows the arrow; Hidden still hides it, except while the times are recorded.
        deck::ShowSettings windowed;
        windowed.type = deck::ShowSettings::Type::Individual;
        show.setSettings(windowed);
        QTest::keyClick(&show, Qt::Key_U, SlideShow::arrowKeyModifier());
        QCOMPARE(int(show.arrowShown()), int(SlideShow::ArrowShown::Automatic));
        QCOMPARE(show.cursor().shape(), Qt::ArrowCursor);
        QTest::keyClick(&show, Qt::Key_H, SlideShow::arrowKeyModifier());
        QCOMPARE(show.cursor().shape(), Qt::BlankCursor);
        show.setRecording(true);
        QCOMPARE(show.cursor().shape(), Qt::ArrowCursor);
        show.setRecording(false);
        QCOMPARE(show.cursor().shape(), Qt::BlankCursor);
    }

    // Where Qt's Control is the Command key (a Mac), the arrow's keys are the key marked Control, which Qt calls Meta: Command+H is the system's Hide. Run as the other platform has it, the key that is not
    // the choice does nothing, and the menu shows the key that works.
    void arrowKeysUseTheKeyMarkedControlWhereCommandIsTheSystems()
    {
        const auto reset = qScopeGuard([] { SlideShow::forceArrowKeyModifier(Qt::NoModifier); });
        auto d = showDeck(3);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        const auto sessionBack = qScopeGuard([] { SlideShow::setSessionArrowShown(SlideShow::ArrowShown::Automatic); });
        show.resize(960, 540);
#ifdef Q_OS_MACOS
        QCOMPARE(SlideShow::arrowKeyModifier(), Qt::MetaModifier);
#else
        QCOMPARE(SlideShow::arrowKeyModifier(), Qt::ControlModifier);
#endif
        for (const Qt::KeyboardModifier mine : {Qt::MetaModifier, Qt::ControlModifier}) {
            const Qt::KeyboardModifier other = mine == Qt::MetaModifier ? Qt::ControlModifier : Qt::MetaModifier;
            SlideShow::forceArrowKeyModifier(mine);
            show.setArrowShown(SlideShow::ArrowShown::Automatic);
            QTest::keyClick(&show, Qt::Key_H, other);
            QCOMPARE(int(show.arrowShown()), int(SlideShow::ArrowShown::Automatic));   // not the arrow's key here
            QTest::keyClick(&show, Qt::Key_H, mine);
            QCOMPARE(int(show.arrowShown()), int(SlideShow::ArrowShown::Hidden));
            QTest::keyClick(&show, Qt::Key_U, other);
            QCOMPARE(int(show.arrowShown()), int(SlideShow::ArrowShown::Hidden));
            QTest::keyClick(&show, Qt::Key_U, mine);
            QCOMPARE(int(show.arrowShown()), int(SlideShow::ArrowShown::Automatic));
            std::unique_ptr<QMenu> menu(show.pointerMenu(&show));
            QAction *hidden = menu->findChild<QAction *>(QStringLiteral("arrowHidden"));
            QVERIFY(hidden);
            QVERIFY2(hidden->text().endsWith(QKeySequence(mine | Qt::Key_H).toString(QKeySequence::NativeText)), qPrintable(hidden->text()));
        }
    }

    // The choice of the arrow lasts for the session: the next show starts with it.
    void theArrowChoiceIsKeptForTheSessionsLaterShows()
    {
        auto d = showDeck(2);
        LayoutCache cache;
        std::unique_ptr<SlideShow> first(new SlideShow(d.get(), &cache, 0));
        const auto sessionBack = qScopeGuard([] { SlideShow::setSessionArrowShown(SlideShow::ArrowShown::Automatic); });
        QCOMPARE(int(first->arrowShown()), int(SlideShow::ArrowShown::Automatic));
        first->setArrowShown(SlideShow::ArrowShown::Hidden);
        first.reset();
        SlideShow second(d.get(), &cache, 0);
        QCOMPARE(int(second.arrowShown()), int(SlideShow::ArrowShown::Hidden));
        second.setArrowShown(SlideShow::ArrowShown::Visible);
        SlideShow third(d.get(), &cache, 0);
        QCOMPARE(int(third.arrowShown()), int(SlideShow::ArrowShown::Visible));
        third.setArrowShown(SlideShow::ArrowShown::Automatic);
        SlideShow fourth(d.get(), &cache, 0);
        QCOMPARE(int(fourth.arrowShown()), int(SlideShow::ArrowShown::Automatic));
    }

    // The Menu key opens the menu on the window the presenter is looking at (Presenter View when it is open); Esc closes the menu and leaves the show running, and a second Esc ends it.
    void theMenuKeyOpensOnPresenterViewAndEscClosesOnlyTheMenu()
    {
        auto d = showDeck(2);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        const ScreenInfo a{QStringLiteral("A"), QRect(0, 0, 1920, 1080), QRect(0, 0, 1920, 1040), true};
        const ScreenInfo b{QStringLiteral("B"), QRect(1920, 0, 1280, 720), QRect(1920, 0, 1280, 720), false};
        show.placeOn(a, b);
        PresenterView *pv = show.presenterView();
        QVERIFY(pv);
        QTRY_VERIFY(pv->isVisible());
        QTest::keyClick(&show, Qt::Key_Menu);
        QPointer<QMenu> menu = pv->findChild<QMenu *>(QStringLiteral("showMenu"));
        QVERIFY2(menu && menu->isVisible(), "the menu is not on Presenter View");
        QVERIFY(!show.findChild<QMenu *>(QStringLiteral("showMenu")));
        // Esc is the menu's: it closes, and the show goes on.
        QWidget *popup = QApplication::activePopupWidget() ? QApplication::activePopupWidget() : menu.data();
        QTest::keyClick(popup, Qt::Key_Escape);
        QTRY_VERIFY(menu.isNull() || !menu->isVisible());
        QVERIFY(show.isVisible());
        QCOMPARE(show.currentSlide(), 0);
        // The next Esc (the menu is gone) ends the show.
        QTest::keyClick(&show, Qt::Key_Escape);
        QTRY_VERIFY(!show.isVisible());
    }

    // Last Viewed in the show's menu goes back to the slide that was on the screen before this one (off while there is none); the Menu key and Shift+F10 open the menu without a mouse.
    void lastViewedAndTheMenuKeyAreInTheShowMenu()
    {
        auto d = showDeck(3);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        QPointer<QMenu> menu;
        auto open = [&](int key, Qt::KeyboardModifiers mods) {
            menu = nullptr;
            QTest::keyClick(&show, Qt::Key(key), mods);
            menu = show.findChild<QMenu *>(QStringLiteral("showMenu"));
            return !menu.isNull() && menu->isVisible();
        };
        QVERIFY(open(Qt::Key_Menu, Qt::NoModifier));
        QAction *last = menu->findChild<QAction *>(QStringLiteral("menuLastViewed"));
        QVERIFY(last);
        QVERIFY(!last->isEnabled());   // nothing before the first slide
        menu->close();
        QTRY_VERIFY(menu.isNull());
        QCOMPARE(show.currentSlide(), 0);
        QVERIFY(!show.goToLastViewed());
        show.goToPosition(2);
        QCOMPARE(show.currentSlide(), 2);
        QVERIFY(open(Qt::Key_F10, Qt::ShiftModifier));
        last = menu->findChild<QAction *>(QStringLiteral("menuLastViewed"));
        QVERIFY(last && last->isEnabled());
        QCOMPARE(show.currentSlide(), 2);   // (the keys that opened the menu moved nothing)
        last->trigger();
        QCOMPARE(show.currentSlide(), 0);
        // And back again: the slide it left is now the last one viewed.
        QVERIFY(show.goToLastViewed());
        QCOMPARE(show.currentSlide(), 2);
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

    // Which screen shows the slides and which holds Presenter View: Automatic keeps the old way (the audience on the main screen, or the
    // first other one when the window is on it, Presenter View where the window is); a chosen screen shows the slides; one that is gone is Automatic.
    void chooseScreensPicksTheAudienceAndPresenterScreens()
    {
        auto screen = [](const QString &name, int x, bool primary) { return ScreenInfo{name, QRect(x, 0, 1920, 1080), QRect(x, 0, 1920, 1040), primary}; };
        const QVector<ScreenInfo> one{screen(QStringLiteral("A"), 0, true)};
        const QVector<ScreenInfo> two{screen(QStringLiteral("A"), 0, true), screen(QStringLiteral("B"), 1920, false)};
        const QVector<ScreenInfo> three{screen(QStringLiteral("A"), 0, true), screen(QStringLiteral("B"), 1920, false), screen(QStringLiteral("C"), 3840, false)};
        auto is = [](const ScreenChoice &c, int audience, int presenter) { return c.audience == audience && c.presenter == presenter; };
        QVERIFY(is(chooseScreens({}, QString(), 0), -1, -1));
        // One screen: the show fills it and there is no other for Presenter View, whatever was chosen.
        QVERIFY(is(chooseScreens(one, QString(), 0), 0, -1));
        QVERIFY(is(chooseScreens(one, QStringLiteral("A"), 0), 0, -1));
        QVERIFY(is(chooseScreens(one, QStringLiteral("Z"), 0), 0, -1));
        // Two screens, Automatic: the window on the main screen sends the show to the other; the window on the other, the show to the main.
        QVERIFY(is(chooseScreens(two, QString(), 0), 1, 0));
        QVERIFY(is(chooseScreens(two, QString(), 1), 0, 1));
        // Chosen: that screen shows the slides, Presenter View is where the window is, or on another screen when the window is on the chosen one.
        QVERIFY(is(chooseScreens(two, QStringLiteral("A"), 0), 0, 1));
        QVERIFY(is(chooseScreens(two, QStringLiteral("B"), 1), 1, 0));
        QVERIFY(is(chooseScreens(two, QStringLiteral("B"), 0), 1, 0));
        // Three screens.
        QVERIFY(is(chooseScreens(three, QString(), 0), 1, 0));
        QVERIFY(is(chooseScreens(three, QString(), 2), 0, 2));
        QVERIFY(is(chooseScreens(three, QStringLiteral("C"), 1), 2, 1));
        QVERIFY(is(chooseScreens(three, QStringLiteral("C"), 2), 2, 0));   // the window is on it: Presenter View takes the main screen
        QVERIFY(is(chooseScreens(three, QStringLiteral("A"), 0), 0, 1));
        // A saved screen that is gone is Automatic; so is a window on no screen (the main one).
        QVERIFY(is(chooseScreens(two, QStringLiteral("Gone"), 0), 1, 0));
        QVERIFY(is(chooseScreens(three, QStringLiteral("Gone"), 2), 0, 2));
        QVERIFY(is(chooseScreens(two, QString(), -1), 1, 0));
        QVERIFY(is(chooseScreens(two, QString(), 9), 1, 0));
        // The label of a list entry: the name and the size.
        QCOMPARE(two.at(1).label(), QStringLiteral("B (1920 x 1080)"));
        // A screen with no name is "Screen N" by its place in the list.
        QCOMPARE(screenName(QString(), 2), QStringLiteral("Screen 2"));
        QCOMPARE(screenName(QStringLiteral("DP-1"), 2), QStringLiteral("DP-1"));
        QCOMPARE((ScreenInfo{screenName(QString(), 1), QRect(0, 0, 800, 800), QRect(), true}).label(), QStringLiteral("Screen 1 (800 x 800)"));
    }

    // The choice is kept for the computer (a presentation file has no place for it), and the Slide Show tab lists Automatic and each screen.
    void windowShowPresentationOnList()
    {
        SlideShow::setShowOn(QString());
        DeckWindow w;
        w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Slide Show")));
        auto *box = w.ribbon()->findChild<QComboBox *>(QStringLiteral("showOn"));
        QVERIFY(box);
        QCOMPARE(box->itemText(0), QStringLiteral("Automatic"));
        const QVector<ScreenInfo> now = screensNow();
        QVERIFY(!now.isEmpty());
        QCOMPARE(box->count(), int(now.size()) + 1);
        QCOMPARE(box->itemText(1), now.at(0).label());
        QVERIFY(SlideShow::showOn().isEmpty());
        QCOMPARE(box->currentIndex(), 0);
        box->setCurrentIndex(1);
        Q_EMIT box->activated(1);
        QCOMPARE(SlideShow::showOn(), now.at(0).name);
        // A saved screen that is not there shows as Automatic, and is kept until another is chosen.
        SlideShow::setShowOn(QStringLiteral("No Such Screen"));
        DeckWindow w2;
        w2.ribbon()->showTab(w2.ribbon()->tab(QStringLiteral("Slide Show")));
        QCOMPARE(w2.ribbon()->findChild<QComboBox *>(QStringLiteral("showOn"))->currentIndex(), 0);
        QCOMPARE(SlideShow::showOn(), QStringLiteral("No Such Screen"));
        box->setCurrentIndex(0);
        Q_EMIT box->activated(0);
        QVERIFY(SlideShow::showOn().isEmpty());
        SlideShow::setShowOn(QString());
    }

    // Swap in Presenter View trades the two screens: the show goes where Presenter View was, and Presenter View where the show was.
    void presenterViewSwapTradesTheScreens()
    {
        auto d = showDeck(2);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        QVERIFY(!show.canSwapScreens());   // no second screen yet
        QVERIFY(!show.presenterView());
        const ScreenInfo a{QStringLiteral("A"), QRect(0, 0, 1920, 1080), QRect(0, 0, 1920, 1040), true};
        const ScreenInfo b{QStringLiteral("B"), QRect(1920, 0, 1280, 720), QRect(1920, 0, 1280, 720), false};
        show.placeOn(a, b);
        PresenterView &pv = *show.presenterView();
        auto *swap = pv.findChild<QAbstractButton *>(QStringLiteral("swapScreens"));
        QVERIFY(swap);
        QCOMPARE(show.audienceScreen().name, QStringLiteral("A"));
        QCOMPARE(show.presenterScreen().name, QStringLiteral("B"));
        QVERIFY(show.canSwapScreens());
        QTRY_VERIFY(swap->isEnabled());
        swap->click();
        QCOMPARE(show.audienceScreen().name, QStringLiteral("B"));
        QCOMPARE(show.presenterScreen().name, QStringLiteral("A"));
        swap->click();   // and back
        QCOMPARE(show.audienceScreen().name, QStringLiteral("A"));
        QCOMPARE(show.presenterScreen().name, QStringLiteral("B"));
    }

    // A screen unplugged during the show: the windows that were on it go to the screens that are left by the same rule, and with one screen left
    // there is nothing to swap. (The screens here are not the computer's, so no window is moved: the decision is what is tested.)
    void showFollowsScreensComingAndGoing()
    {
        auto d = showDeck(2);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        auto screen = [](const QString &name, int x, bool primary) { return ScreenInfo{name, QRect(x, 0, 1920, 1080), QRect(x, 0, 1920, 1040), primary}; };
        const ScreenInfo a = screen(QStringLiteral("A"), 0, true), b = screen(QStringLiteral("B"), 1920, false), c = screen(QStringLiteral("C"), 3840, false);
        show.placeOn(a, b);
        PresenterView &pv = *show.presenterView();
        auto *swap = pv.findChild<QAbstractButton *>(QStringLiteral("swapScreens"));
        QTRY_VERIFY(swap->isEnabled());
        show.reconcileScreens({a, b, c});   // one came: nothing moves
        QCOMPARE(show.audienceScreen().name, QStringLiteral("A"));
        QCOMPARE(show.presenterScreen().name, QStringLiteral("B"));
        show.reconcileScreens({a, c});   // Presenter View's screen went: it takes the other one
        QCOMPARE(show.audienceScreen().name, QStringLiteral("A"));
        QCOMPARE(show.presenterScreen().name, QStringLiteral("C"));
        show.reconcileScreens({c, screen(QStringLiteral("D"), 0, true)});   // the show's went too, and the one Presenter View was on stays its place
        QCOMPARE(show.presenterScreen().name, QStringLiteral("C"));
        QCOMPARE(show.audienceScreen().name, QStringLiteral("D"));
        show.reconcileScreens({c});   // one left
        QCOMPARE(show.audienceScreen().name, QStringLiteral("C"));
        QVERIFY(show.presenterScreen().geometry.isEmpty());
        QVERIFY(!show.canSwapScreens());
        QVERIFY(!swap->isEnabled());
        show.swapScreens();   // nothing to trade
        QCOMPARE(show.audienceScreen().name, QStringLiteral("C"));
        show.reconcileScreens({});   // no screens known: nothing is done
        QCOMPARE(show.audienceScreen().name, QStringLiteral("C"));
    }

    // The notes in Presenter View are typed in the same kind of box as the notes pane: each edit is one step of the window's Undo, the notes stay
    // when the show ends and in the saved file, and A+ and A- still change their size.
    void presenterViewNotesAreEdited()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        ed->setCurrentPage(0);
        QTest::keyClicks(w.notesPane(), QStringLiteral("First notes"));
        w.notesPane()->commit();
        auto notes = [&](int i) { return deck::notesText(*ed->doc(), *ed->doc()->pages[i]); };
        QCOMPARE(notes(0), QStringLiteral("First notes"));
        w.act("show.presenterView")->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show && show->presenterView());
        PresenterView *pv = show->presenterView();
        QTextEdit *edit = pv->findChild<QTextEdit *>(QStringLiteral("notesText"));
        QVERIFY(edit && !edit->isReadOnly());
        QCOMPARE(edit->toPlainText(), QStringLiteral("First notes"));
        // Typing is not a key of the show: the slide stays.
        edit->setFocus();
        edit->moveCursor(QTextCursor::End);
        QTest::keyClicks(edit, QStringLiteral(" and more"));
        QCOMPARE(show->currentSlide(), 0);
        pv->commitNotes();
        QCOMPARE(notes(0), QStringLiteral("First notes and more"));
        ed->undo();   // one step
        QCOMPARE(notes(0), QStringLiteral("First notes"));
        QTRY_COMPARE(edit->toPlainText(), QStringLiteral("First notes"));
        ed->redo();
        QCOMPARE(notes(0), QStringLiteral("First notes and more"));
        QTRY_COMPARE(edit->toPlainText(), QStringLiteral("First notes and more"));
        // A+ and A- still work.
        const int size = pv->notesPointSize();
        pv->findChild<QAbstractButton *>(QStringLiteral("notesBigger"))->click();
        QCOMPARE(pv->notesPointSize(), size + 2);
        pv->findChild<QAbstractButton *>(QStringLiteral("notesSmaller"))->click();
        QCOMPARE(pv->notesPointSize(), size);
        // Typed and not yet saved when the show ends: kept.
        edit->moveCursor(QTextCursor::End);
        QTest::keyClicks(edit, QStringLiteral("!"));
        show->close();
        QTRY_VERIFY(!w.slideShow());
        QCOMPARE(notes(0), QStringLiteral("First notes and more!"));
        QCOMPARE(w.notesPane()->toPlainText(), QStringLiteral("First notes and more!"));
        // And in the saved file.
        QString err;
        auto back = deck::importPresentation(deck::exportPresentation(*ed->doc(), &err), &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(deck::notesText(*back, *back->pages[0]), QStringLiteral("First notes and more!"));
        ed->undo();
        QCOMPARE(notes(0), QStringLiteral("First notes and more"));
    }

    // While Presenter View is up its box is the one view of the notes' story: the pane under the slide gives the story up (and has it again when the show ends), so two views never
    // hold one document. Words typed in the box are the story's at once and are one step of Undo, as in the pane.
    void presenterViewHoldsTheNotesAloneWhileItIsUp()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        ed->setCurrentPage(0);
        auto story = [&] { return ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId); };
        auto notes = [&] { return deck::notesText(*ed->doc(), *ed->doc()->pages[0]); };
        QTest::keyClicks(w.notesPane(), QStringLiteral("one"));
        QVERIFY(w.notesPane()->document() == story());
        w.act("show.presenterView")->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show && show->presenterView());
        PresenterView *pv = show->presenterView();
        QTextEdit *edit = pv->findChild<QTextEdit *>(QStringLiteral("notesText"));
        QVERIFY(edit);
        QVERIFY(edit->document() == story());
        QVERIFY2(w.notesPane()->document() != story() && w.notesPageView()->notesBox()->document() != story(), "the window's views let the story go");
        QTRY_COMPARE(edit->toPlainText(), QStringLiteral("one"));
        edit->setFocus();
        edit->moveCursor(QTextCursor::End);
        QTest::keyClicks(edit, QStringLiteral(" two"));
        QCOMPARE(notes(), QStringLiteral("One two"));   // (the words are the story's at once)
        pv->commitNotes();
        ed->undo();
        QCOMPARE(notes(), QStringLiteral("one"));
        ed->redo();
        QCOMPARE(notes(), QStringLiteral("One two"));
        QTRY_COMPARE(edit->toPlainText(), QStringLiteral("One two"));
        show->close();
        QTRY_VERIFY(!w.slideShow());
        QVERIFY(w.notesPane()->document() == story());
        QCOMPARE(w.notesPane()->toPlainText(), QStringLiteral("One two"));
        QVERIFY(w.notesPageView()->notesBox()->document() != story());   // (the page's box is out of sight)
    }

    // The words of a presenter's notes, with a mark around each run that is bold (B), italic (I), or underlined (U): "alpha [B]beta[/B] gamma".
    static QString markedNotes(const QTextDocument *story)
    {
        QString out;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            if (b != story->begin()) out += QLatin1Char('|');
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextCharFormat cf = it.fragment().charFormat();
                QString m;
                if (cf.fontWeight() > QFont::Normal) m += QLatin1Char('B');
                if (cf.fontItalic()) m += QLatin1Char('I');
                if (cf.fontUnderline()) m += QLatin1Char('U');
                out += m.isEmpty() ? it.fragment().text() : QStringLiteral("[%1]%2[/%1]").arg(m, it.fragment().text());
            }
        }
        return out;
    }
    // A window with a slide whose notes have a bold word, and Presenter View open on it.
    static PresenterView *presenterOnFormattedNote(DeckWindow &w, SlideShow **show)
    {
        Editor *ed = w.editor();
        w.resize(1280, 800);
        w.show();
        w.newSlide();
        ed->setCurrentPage(0);
        QTextDocument *story = ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId);
        QTextCursor c(story);
        QTextCharFormat bold;
        bold.setFontWeight(QFont::Bold);
        c.insertText(QStringLiteral("alpha "), QTextCharFormat());
        c.insertText(QStringLiteral("beta"), bold);
        c.insertText(QStringLiteral(" gamma"), QTextCharFormat());
        w.notesPane()->reload();
        ed->undoStack()->clear();
        w.act("show.presenterView")->trigger();
        *show = w.slideShow();
        return *show ? (*show)->presenterView() : nullptr;
    }

    // Presenter View shows the note formatted and edits it through the same widget as the notes under the slide: an edit at the start and one at the end with no pause between keep
    // the formatting between them, Ctrl+B bolds the selection, and the Undo history is the presentation's.
    void presenterViewShowsAndEditsTheNoteFormatted()
    {
        DeckWindow w;
        SlideShow *show = nullptr;
        PresenterView *pv = presenterOnFormattedNote(w, &show);
        QVERIFY(show && pv);
        Editor *ed = w.editor();
        QTextEdit *edit = pv->findChild<QTextEdit *>(QStringLiteral("notesText"));
        QVERIFY(edit);
        QTextDocument *story = ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId);
        QTRY_VERIFY(edit->document() == story);
        QCOMPARE(markedNotes(edit->document()), QStringLiteral("alpha [B]beta[/B] gamma"));   // (shown formatted: it is the story's own document)
        edit->setFocus();
        edit->moveCursor(QTextCursor::Start);
        QTest::keyClicks(edit, QStringLiteral(">"));
        edit->moveCursor(QTextCursor::End);   // (the caret jumps; no pause)
        QTest::keyClicks(edit, QStringLiteral("<"));
        QCOMPARE(markedNotes(story), QStringLiteral(">alpha [B]beta[/B] gamma<"));
        // Ctrl+B on the selected word.
        QTextCursor sel(edit->document());
        sel.setPosition(1);
        sel.setPosition(6, QTextCursor::KeepAnchor);
        edit->setTextCursor(sel);
        QTest::keyClick(edit, Qt::Key_B, Qt::ControlModifier);
        QCOMPARE(markedNotes(story), QStringLiteral(">[B]alpha[/B] [B]beta[/B] gamma<"));
        pv->commitNotes();
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QStringLiteral(">alpha beta gamma<"));
        ed->undo();   // the bold
        QCOMPARE(markedNotes(ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId)), QStringLiteral(">alpha [B]beta[/B] gamma<"));
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // Pictures for the review (only when RN9_SHOT_PREFIX names a path prefix): the widget as it is on screen.
    static void rn9Shot(QWidget *w, const QString &name)
    {
        const QString prefix = qEnvironmentVariable("RN9_SHOT_PREFIX");
        if (!prefix.isEmpty()) w->grab().save(prefix + name + QStringLiteral(".png"));
    }
    // The pixels of a notes view's viewport that differ from its background, in a rectangle of the viewport.
    static int viewportInk(QTextEdit *e, const QRect &r)
    {
        const QImage img = e->viewport()->grab().toImage();
        const QColor bg = img.pixelColor(img.width() - 2, 2);
        int n = 0;
        for (int y = std::max(0, r.top()); y <= r.bottom() && y < img.height(); ++y)
            for (int x = std::max(0, r.left()); x <= r.right() && x < img.width(); ++x) n += img.pixelColor(x, y) != bg;
        return n;
    }

    // Formatting the first word of a note in Presenter View (select it with the keys, press Ctrl+B) leaves the first paragraph where it was: every paragraph keeps its height, and the first
    // line is still drawn, at each text size A+ and A- give (the whole first paragraph vanished from the screen, though the note kept its words).
    void formattingTheFirstWordInPresenterViewKeepsTheFirstParagraphOnScreen()
    {
        const QVariant sizeSetting = Settings::get().value(QStringLiteral("show/notesSize"));   // (A+ and A- are remembered: the setting is put back, for the tests after)
        const auto putBack = qScopeGuard([sizeSetting] {
            if (sizeSetting.isValid()) Settings::get().setValue(QStringLiteral("show/notesSize"), sizeSetting);
            else Settings::get().remove(QStringLiteral("show/notesSize"));
        });
        for (int steps : {0, 3, -4, -9}) {
            DeckWindow w;
            Editor *ed = w.editor();
            w.resize(1280, 800);
            w.show();
            w.newSlide();
            ed->setCurrentPage(0);
            {
                QTextCursor c(ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId));
                c.insertText(QStringLiteral("alpha beta"), QTextCharFormat());
                c.insertBlock();
                c.insertText(QStringLiteral("gamma delta"), QTextCharFormat());
            }
            w.notesPane()->reload();
            ed->undoStack()->clear();
            w.act("show.presenterView")->trigger();
            SlideShow *show = w.slideShow();
            QVERIFY(show && show->presenterView());
            PresenterView *pv = show->presenterView();
            pv->resize(1400, 900);
            for (int i = 0; i < std::abs(steps); ++i) pv->findChild<QAbstractButton *>(steps > 0 ? QStringLiteral("notesBigger") : QStringLiteral("notesSmaller"))->click();
            auto *edit = qobject_cast<StoryEdit *>(pv->findChild<QTextEdit *>(QStringLiteral("notesText")));
            QVERIFY(edit);
            QTextDocument *story = ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId);
            QTRY_VERIFY(edit->document() == story);
            auto firstLine = [&] {
                QTextCursor c(edit->document());
                c.setPosition(0);
                return edit->cursorRect(c);
            };
            QTRY_VERIFY(firstLine().height() > 2);
            const QRect line = firstLine().adjusted(0, 0, 200, 0);
            const QString at = QStringLiteral("text size step %1").arg(steps);
            QTRY_VERIFY2(viewportInk(edit, line) > 20, qPrintable(at + QStringLiteral(": the first line is drawn at the start")));
            rn9Shot(pv, QStringLiteral("item1-%1-0start").arg(steps));
            QTextCursor c0(edit->document());
            c0.setPosition(0);
            const QRect r0 = edit->cursorRect(c0);
            QTest::mouseClick(edit->viewport(), Qt::LeftButton, {}, QPoint(r0.x() + 1, r0.center().y()));
            for (int i = 0; i < 5; ++i) QTest::keyClick(edit, Qt::Key_Right, Qt::ShiftModifier);
            QCOMPARE(edit->textCursor().selectedText(), QStringLiteral("alpha"));
            QTest::keyClick(edit, Qt::Key_B, Qt::ControlModifier);
            QTRY_VERIFY2(markedNotes(story).startsWith(QStringLiteral("[B]alpha[/B]")), qPrintable(markedNotes(story)));
            for (int i = 0; i < 6; ++i) QCoreApplication::processEvents();
            rn9Shot(pv, QStringLiteral("item1-%1-1bold").arg(steps));
            QAbstractTextDocumentLayout *lay = edit->document()->documentLayout();
            QVERIFY(lay);
            int blocks = 0;
            for (QTextBlock b = edit->document()->begin(); b.isValid(); b = b.next(), ++blocks)
                QVERIFY2(lay->blockBoundingRect(b).height() > 2, qPrintable(at + QStringLiteral(": paragraph %1 has no height").arg(blocks)));
            QCOMPARE(blocks, 2);
            QVERIFY2(viewportInk(edit, firstLine().adjusted(0, 0, 200, 0)) > 20, qPrintable(at + QStringLiteral(": the first line is still drawn")));
            QVERIFY2(viewportInk(edit, firstLine().adjusted(0, 0, 200, 0)) >= viewportInk(edit, line) * 3 / 4, qPrintable(at + QStringLiteral(": the bold word is as inked as the plain one")));
            show->close();
            QTRY_VERIFY(!w.slideShow());
        }
    }

    // The editing keys work in Presenter View's notes, a window with no shortcuts of its own (they did nothing there, and the menu's Undo, Redo, Cut, Copy, and Paste were gray): Undo and
    // Redo move through the presentation's one history, Copy and Cut put the selected words on the clipboard, Paste puts them in, and each of the five is in the right-click menu, enabled
    // when it can act. Every key is a real key press, and the menu is opened by a real context event.
    void editingKeysAndMenuWorkInPresenterViewNotes()
    {
        DeckWindow w;
        SlideShow *show = nullptr;
        PresenterView *pv = presenterOnFormattedNote(w, &show);
        QVERIFY(show && pv);
        Editor *ed = w.editor();
        auto *edit = qobject_cast<StoryEdit *>(pv->findChild<QTextEdit *>(QStringLiteral("notesText")));
        QVERIFY(edit);
        pv->resize(1400, 900);
        QTextDocument *story = ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId);
        QTRY_VERIFY(edit->document() == story);
        QApplication::clipboard()->clear();
        const auto cleanUp = qScopeGuard([] { QApplication::clipboard()->clear(); });
        auto press = [&](QKeySequence::StandardKey k) {
            const QKeyCombination c = QKeySequence(k)[0];
            QTest::keyClick(edit, c.key(), c.keyboardModifiers());
        };
        auto marked = [&] { return markedNotes(ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId)); };   // (an undo puts a new document in: the note is looked up each time)
        auto selectFirst = [&](int n) {   // (click at the start of the note, then Shift+Right)
            QTextCursor c0(edit->document());
            c0.setPosition(0);
            const QRect r0 = edit->cursorRect(c0);
            QTest::mouseClick(edit->viewport(), Qt::LeftButton, {}, QPoint(r0.x() + 1, r0.center().y()));
            for (int i = 0; i < n; ++i) QTest::keyClick(edit, Qt::Key_Right, Qt::ShiftModifier);
        };
        const QString plain = QStringLiteral("alpha [B]beta[/B] gamma");
        QCOMPARE(marked(), plain);
        // (the menu as it is first opened, on a selected word: a picture of it for the review)
        selectFirst(2);
        {
            const QRect r = edit->cursorRect(edit->textCursor());
            rightClick(edit->viewport(), QPoint(r.x() - 4, r.center().y()), [&](QMenu *menu) { rn9Shot(menu, QStringLiteral("item2-menu-first")); });
        }
        // Typing, then Undo and Redo by their keys.
        selectFirst(0);
        QTest::keyClicks(edit, QStringLiteral("XY"));
        QCOMPARE(marked(), QStringLiteral("XYalpha [B]beta[/B] gamma"));
        press(QKeySequence::Undo);
        QTRY_COMPARE(marked(), plain);
        QVERIFY(ed->undoStack()->canRedo());   // (the step undone is in the presentation's one history, to redo)
        QCOMPARE(ed->undoStack()->index(), 0);
        press(QKeySequence::Redo);
        QTRY_COMPARE(marked(), QStringLiteral("XYalpha [B]beta[/B] gamma"));
        // Copy puts the selection on the clipboard and changes nothing; Paste puts it back in at the caret.
        selectFirst(2);
        QCOMPARE(edit->textCursor().selectedText(), QStringLiteral("XY"));
        press(QKeySequence::Copy);
        QTRY_COMPARE(QApplication::clipboard()->text(), QStringLiteral("XY"));
        QCOMPARE(marked(), QStringLiteral("XYalpha [B]beta[/B] gamma"));
        QTest::keyClick(edit, Qt::Key_End);
        press(QKeySequence::Paste);
        QTRY_COMPARE(marked(), QStringLiteral("XYalpha [B]beta[/B] gammaXY"));
        // Cut takes the selection out and keeps it on the clipboard; Undo brings it back.
        QApplication::clipboard()->clear();
        selectFirst(2);
        press(QKeySequence::Cut);
        QTRY_COMPARE(marked(), QStringLiteral("alpha [B]beta[/B] gammaXY"));
        QCOMPARE(QApplication::clipboard()->text(), QStringLiteral("XY"));
        press(QKeySequence::Undo);
        QTRY_COMPARE(marked(), QStringLiteral("XYalpha [B]beta[/B] gammaXY"));
        // The right-click menu: each of the five is there, enabled when it can act, and does what its key does.
        auto itemOf = [](QMenu *menu, const QString &text) -> QAction * {
            for (QAction *a : menu->actions())
                if (a->text().remove(QLatin1Char('&')) == text) return a;
            return nullptr;
        };
        auto menuDo = [&](const QString &item, bool expectEnabled, int select = 2) {
            selectFirst(select);
            const QRect r = edit->cursorRect(edit->textCursor());
            bool found = false, enabled = false;
            rightClick(edit->viewport(), QPoint(r.x() - 4, r.center().y()), [&](QMenu *menu) {
                rn9Shot(menu, QStringLiteral("item2-menu-") + item);
                QAction *a = itemOf(menu, item);
                found = a != nullptr;
                enabled = a && a->isEnabled();
                if (a && a->isEnabled()) a->trigger();
            });
            QTRY_VERIFY2(found, qPrintable(item + QStringLiteral(" is in the menu")));
            QCOMPARE(enabled, expectEnabled);
        };
        QApplication::clipboard()->clear();
        menuDo(QStringLiteral("Copy"), true);
        QTRY_COMPARE(QApplication::clipboard()->text(), QStringLiteral("XY"));
        menuDo(QStringLiteral("Cut"), true);
        QTRY_COMPARE(marked(), QStringLiteral("alpha [B]beta[/B] gammaXY"));
        menuDo(QStringLiteral("Undo"), true);
        QTRY_COMPARE(marked(), QStringLiteral("XYalpha [B]beta[/B] gammaXY"));
        menuDo(QStringLiteral("Redo"), true);
        QTRY_COMPARE(marked(), QStringLiteral("alpha [B]beta[/B] gammaXY"));
        menuDo(QStringLiteral("Paste"), true, 0);
        QTRY_COMPARE(marked(), QStringLiteral("XYalpha [B]beta[/B] gammaXY"));
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // A note of one very long word (a file can hold one) opens in Presenter View with work counted by the letters of a line, not by the square of the word: the view wraps it anywhere (laying
    // out a word longer than a line looked over all of it for each line: 20,000 letters took 15 seconds and 50,000 over 100). The letters looked at to decide are bounded by the limit, the
    // same at 20,000 and 100,000 letters, every letter is laid out, a word that is only long by typing changes the wrapping once it reaches the limit, and a note of ordinary words is wrapped
    // at its words as before.
    void aNoteOfOneVeryLongWordOpensInPresenterViewWithBoundedWork()
    {
        int looked[2] = {};
        for (int k = 0; k < 2; ++k) {
            const int n = k ? 100000 : 20000;
            DeckWindow w;
            Editor *ed = w.editor();
            w.resize(1280, 800);
            w.show();
            w.newSlide();
            ed->setCurrentPage(0);
            {
                QTextCursor c(ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId));
                c.insertText(QString(n, QLatin1Char('W')), QTextCharFormat());
            }
            w.notesPane()->reload();
            for (int i = 0; i < 4; ++i) QCoreApplication::processEvents();
            const int before = StoryEdit::runLettersLooked();
            w.act("show.presenterView")->trigger();
            SlideShow *show = w.slideShow();
            QVERIFY(show && show->presenterView());
            auto *edit = qobject_cast<StoryEdit *>(show->presenterView()->findChild<QTextEdit *>(QStringLiteral("notesText")));
            QVERIFY(edit);
            QTextDocument *story = ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId);
            QTRY_VERIFY(edit->document() == story);
            looked[k] = StoryEdit::runLettersLooked() - before;
            rn9Shot(show->presenterView(), QStringLiteral("item4-%1").arg(n));
            QVERIFY2(edit->wordWrapMode() == QTextOption::WrapAnywhere, qPrintable(QStringLiteral("%1 letters: wrapped anywhere").arg(n)));
            QVERIFY2(looked[k] > 0 && looked[k] <= 8 * StoryEdit::kLongRun, qPrintable(QStringLiteral("%1 letters: %2 letters looked at").arg(n).arg(looked[k])));
            // every letter is laid out, in lines of the view's width
            const QTextBlock b = story->begin();
            QTRY_VERIFY(b.layout() && b.layout()->lineCount() > 20);
            int laid = 0, widest = 0;
            for (int i = 0; i < b.layout()->lineCount(); ++i) {
                laid += b.layout()->lineAt(i).textLength();
                widest = std::max(widest, int(b.layout()->lineAt(i).naturalTextWidth()));
            }
            QCOMPARE(laid, n);
            QVERIFY2(widest <= edit->viewport()->width() + 2, qPrintable(QStringLiteral("a line is %1 wide in a view %2 wide").arg(widest).arg(edit->viewport()->width())));
            // a key typed at the start looks at no more letters than the limit
            edit->setFocus();
            QTest::mouseClick(edit->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(2, 6));
            const int beforeKey = StoryEdit::runLettersLooked();
            QTest::keyClicks(edit, QStringLiteral("x"));
            QVERIFY2(StoryEdit::runLettersLooked() - beforeKey <= StoryEdit::kLongRun + 2, qPrintable(QStringLiteral("a key looked at %1 letters").arg(StoryEdit::runLettersLooked() - beforeKey)));
            QCOMPARE(story->characterCount() - 1, n + 1);
            show->close();
            QTRY_VERIFY(!w.slideShow());
        }
        QCOMPARE(looked[1], looked[0]);   // (the letters looked at do not grow with the word)
        // A word that grows long by typing: wrapped at the word's edge until it reaches the limit, then anywhere; ordinary words in a long paragraph are wrapped at the words.
        {
            DeckWindow w;
            Editor *ed = w.editor();
            w.resize(1280, 800);
            w.show();
            w.newSlide();
            ed->setCurrentPage(0);
            {
                QTextCursor c(ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId));
                c.insertText(QStringLiteral("start ") + QString(StoryEdit::kLongRun - 20, QLatin1Char('W')), QTextCharFormat());
            }
            auto *pane = w.notesPane();
            pane->reload();
            QTRY_VERIFY(pane->document() == ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId));
            QVERIFY(pane->wordWrapMode() != QTextOption::WrapAnywhere);
            pane->setFocus();
            pane->moveCursor(QTextCursor::End);
            int worst = 0;
            for (int i = 0; i < 40; ++i) {
                const int one = StoryEdit::runLettersLooked();
                QTest::keyClicks(pane, QStringLiteral("W"));
                worst = std::max(worst, StoryEdit::runLettersLooked() - one);
            }
            QTRY_VERIFY(pane->wordWrapMode() == QTextOption::WrapAnywhere);
            QVERIFY2(worst <= StoryEdit::kLongRun + 2, qPrintable(QStringLiteral("a key looked at %1 letters").arg(worst)));
        }
        {
            DeckWindow w;
            Editor *ed = w.editor();
            w.resize(1280, 800);
            w.show();
            w.newSlide();
            ed->setCurrentPage(0);
            {
                QTextCursor c(ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId));
                QString words;
                while (words.size() < 40000) words += QStringLiteral("ordinary ");
                c.insertText(words, QTextCharFormat());
            }
            auto *pane = w.notesPane();
            pane->reload();
            QTRY_VERIFY(pane->document() == ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId));
            QCOMPARE(pane->wordWrapMode(), QTextOption::WrapAtWordBoundaryOrAnywhere);
        }
    }

    // A long note opens at its top in Presenter View, on every slide shown (it opened scrolled to its end), and the notes pane under the slide shows the top again after the show: the first
    // line a person sees is the note's first, the caret is at its start, the box is not scrolled, and the note is longer than the box (or the test shows nothing).
    void presenterViewOpensEveryLongNoteAtItsTop()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.resize(1280, 800);
        w.show();
        w.newSlide();
        w.newSlide();
        for (int slide = 0; slide < 2; ++slide) {
            QTextCursor c(ed->doc()->storyDoc(ed->doc()->pages[slide]->notesStoryId));
            for (int i = 0; i < 41; ++i) {
                if (i) c.insertBlock();
                c.insertText(QStringLiteral("Slide %1 paragraph %2 of a long note").arg(slide).arg(i), QTextCharFormat());
            }
        }
        ed->setCurrentPage(0);
        w.notesPane()->reload();
        ed->undoStack()->clear();
        w.act("show.presenterView")->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show);
        PresenterView *pv = show->presenterView();
        QVERIFY(pv);
        auto *edit = pv->findChild<QTextEdit *>(QStringLiteral("notesText"));
        QVERIFY(edit);
        for (int slide = 0; slide < 2; ++slide) {
            if (slide) show->goTo(slide);
            QTRY_VERIFY(edit->document() == ed->doc()->storyDoc(ed->doc()->pages[slide]->notesStoryId));
            const QString tag = QStringLiteral("slide %1").arg(slide);
            QVERIFY2(edit->verticalScrollBar()->maximum() > 0, qPrintable(tag + QStringLiteral(": the note is longer than the box")));
            QCOMPARE(edit->verticalScrollBar()->value(), 0);
            QVERIFY2(edit->cursorForPosition(QPoint(4, 2)).blockNumber() == 0, qPrintable(tag + QStringLiteral(": the first line shown is not the note's first")));
            QCOMPARE(edit->textCursor().position(), 0);
        }
        show->close();
        QTRY_VERIFY(!w.slideShow());
        // after the show the pane under the slide shows the top of the note, not its end
        NotesPane *pane = w.notesPane();
        QTRY_VERIFY(pane->document() == ed->doc()->storyDoc(ed->doc()->pages[ed->currentPage()]->notesStoryId));
        QCOMPARE(pane->verticalScrollBar()->value(), 0);
        QCOMPARE(pane->cursorForPosition(QPoint(4, 2)).blockNumber(), 0);
    }

    // The person types at the END of a long note in one view; every other view then opens that note at its TOP (it opened at the end after the Notes Page was edited): each pair of
    // views, in both directions: pane to Notes Page box, box to pane, pane to Presenter View, the pane after the show, box to Presenter View. (The show's Speaker Notes box is a fresh copy of the note for each slide: it is at the top by its making.)
    // The view typed in keeps its caret where it was typed.
    void everyViewOpensTheNoteAtItsTopAfterItWasEditedInAnother()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.resize(1280, 800);
        w.show();
        w.newSlide();
        {
            QTextCursor c(ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId));
            for (int i = 0; i < 300; ++i) {
                if (i) c.insertBlock();
                c.insertText(QStringLiteral("Paragraph %1 of a long note").arg(i), QTextCharFormat());
            }
        }
        ed->setCurrentPage(0);
        w.notesPane()->reload();
        ed->undoStack()->clear();
        auto story = [&] { return ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId); };
        auto top = [&](QTextEdit *e, const QString &tag) {
            QTRY_VERIFY2(e->isVisible() && e->document() == story(), qPrintable(tag + QStringLiteral(": not shown")));
            QVERIFY2(e->verticalScrollBar()->maximum() > 0, qPrintable(tag + QStringLiteral(": the note is not longer than the view")));
            QVERIFY2(e->verticalScrollBar()->value() == 0, qPrintable(tag + QStringLiteral(": scrolled to ") + QString::number(e->verticalScrollBar()->value())));
            QVERIFY2(e->cursorForPosition(QPoint(4, 2)).blockNumber() == 0, qPrintable(tag + QStringLiteral(": the first line shown is not the note's first")));
        };
        auto typeAtEnd = [&](NotesPane *view, const QString &tag) {
            view->setFocus();
            QVERIFY2(view->takeTarget(), qPrintable(tag));
            QTest::keyClick(view, Qt::Key_End, Qt::ControlModifier);
            QTest::keyClick(view, Qt::Key_Z);
            QVERIFY2(view->verticalScrollBar()->value() > 0, qPrintable(tag + QStringLiteral(": the view typed in follows its caret")));
        };
        // pane -> Notes Page box
        top(w.notesPane(), QStringLiteral("pane at first"));
        typeAtEnd(w.notesPane(), QStringLiteral("pane typed"));
        w.act(QStringLiteral("view.notesPage"))->trigger();
        NotesPane *box = w.notesPageView()->notesBox();
        top(box, QStringLiteral("box after the pane was typed in"));
        // box -> pane
        typeAtEnd(box, QStringLiteral("box typed"));
        w.act(QStringLiteral("view.normal"))->trigger();
        top(w.notesPane(), QStringLiteral("pane after the box was typed in"));
        // pane -> Presenter View
        typeAtEnd(w.notesPane(), QStringLiteral("pane typed again"));
        w.act("show.presenterView")->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show && show->presenterView());
        top(show->presenterView()->findChild<QTextEdit *>(QStringLiteral("notesText")), QStringLiteral("Presenter View after the pane was typed in"));
        show->close();
        QTRY_VERIFY(!w.slideShow());
        top(w.notesPane(), QStringLiteral("pane after the show"));
        typeAtEnd(w.notesPane(), QStringLiteral("pane typed before the box"));
        w.act(QStringLiteral("view.notesPage"))->trigger();
        top(box, QStringLiteral("box (after the show)"));
        // box -> Presenter View
        typeAtEnd(box, QStringLiteral("box typed before the show"));
        w.act("show.presenterView")->trigger();
        show = w.slideShow();
        QVERIFY(show && show->presenterView());
        top(show->presenterView()->findChild<QTextEdit *>(QStringLiteral("notesText")), QStringLiteral("Presenter View after the box was typed in"));
        show->close();
        QTRY_VERIFY(!w.slideShow());
        w.act(QStringLiteral("view.normal"))->trigger();
        top(w.notesPane(), QStringLiteral("pane after the show from the Notes Page"));
    }

    // A+ and A- change how big the notes are shown and nothing in the note: its words, formats, default font, and revision stay; the shown size follows.
    void presenterViewTextSizeChangesOnlyTheDisplay()
    {
        DeckWindow w;
        SlideShow *show = nullptr;
        PresenterView *pv = presenterOnFormattedNote(w, &show);
        QVERIFY(show && pv);
        Editor *ed = w.editor();
        QTextEdit *edit = pv->findChild<QTextEdit *>(QStringLiteral("notesText"));
        QTextDocument *story = ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId);
        QTRY_VERIFY(edit->document() == story);
        const QString html = story->toHtml();
        const QFont font = story->defaultFont();
        const int revision = story->revision(), steps = ed->undoStack()->count();
        const int size = pv->notesPointSize();
        auto height = [&] { return edit->document()->size().height(); };
        QTRY_VERIFY(height() > 1);
        const double before = height();
        for (int i = 0; i < 4; ++i) pv->findChild<QAbstractButton *>(QStringLiteral("notesBigger"))->click();
        QCOMPARE(pv->notesPointSize(), size + 8);
        QTRY_VERIFY2(height() > before * 1.2, qPrintable(QStringLiteral("%1 -> %2").arg(before).arg(height())));
        for (int i = 0; i < 8; ++i) pv->findChild<QAbstractButton *>(QStringLiteral("notesSmaller"))->click();
        QTRY_VERIFY(height() < before * 0.9);
        QCOMPARE(story->toHtml(), html);
        QCOMPARE(story->defaultFont(), font);
        QCOMPARE(story->revision(), revision);
        QCOMPARE(ed->undoStack()->count(), steps);
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // The note's bullets and numbers show in Presenter View, inside the notes box and left of the words, with the printed page's texts, and they are bigger at a bigger text size (A+).
    void presenterViewDrawsTheMarkersAtEveryTextSize()
    {
        DeckWindow w;
        SlideShow *show = nullptr;
        PresenterView *pv = presenterOnFormattedNote(w, &show);
        QVERIFY(show && pv);
        Editor *ed = w.editor();
        QTextDocument *story = ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId);
        fillMixedListNote(story);
        auto *edit = qobject_cast<StoryEdit *>(pv->findChild<QTextEdit *>(QStringLiteral("notesText")));
        QVERIFY(edit);
        QTRY_VERIFY(edit->document() == story);
        const QStringList printed = printedMarkerTexts(ed);
        QCOMPARE(printed.size(), 10);
        QVector<double> heights;
        for (int step = 0; step < 2; ++step) {
            if (step) pv->findChild<QAbstractButton *>(QStringLiteral("notesBigger"))->click();
            if (step) pv->findChild<QAbstractButton *>(QStringLiteral("notesBigger"))->click();
            QTRY_VERIFY(edit->listMarkers()->visibleMarkers().size() >= 2);
            QTRY_VERIFY([&] {
                for (const ListMarkers::Marker &m : edit->listMarkers()->visibleMarkers())
                    if (m.rect.left() < 0 || m.rect.right() > edit->width()) return false;
                return true;
            }());
            const QVector<ListMarkers::Marker> shown = edit->listMarkers()->visibleMarkers();
            QStringList texts;
            for (const ListMarkers::Marker &m : shown) texts << m.text;
            QCOMPARE(texts, printed.mid(0, texts.size()));
            const QImage img = edit->grab().toImage();
            const QColor ground = edit->viewport()->palette().color(edit->viewport()->backgroundRole());
            for (const ListMarkers::Marker &m : shown) {
                QVERIFY2(m.rect.left() >= 0 && m.rect.right() <= edit->width(), qPrintable(QStringLiteral("size step %1: %2").arg(step).arg(m.text)));
                QTextCursor c(edit->document());
                c.setPosition(m.blockPos);
                QVERIFY2(m.rect.right() < edit->viewport()->x() + edit->cursorRect(c).left(), qPrintable(m.text));
                int ink = 0;
                for (int y = int(m.rect.top()); y < int(m.rect.bottom()); ++y)
                    for (int x = int(m.rect.left()); x < int(m.rect.right()); ++x) {
                        const QColor px = img.pixelColor(x, y);
                        ink += qAbs(px.red() - ground.red()) + qAbs(px.green() - ground.green()) + qAbs(px.blue() - ground.blue()) > 150;
                    }
                QVERIFY2(ink >= 2, qPrintable(m.text));
            }
            heights << shown.first().rect.height();
        }
        QVERIFY2(heights[1] > heights[0] * 1.1, qPrintable(QStringLiteral("%1 then %2").arg(heights[0]).arg(heights[1])));
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // The words of a link are drawn in the link color (blue, as the printed page has them) and underlined, and the bullets and numbers of a list are drawn, in Presenter View's notes and
    // in the show's Speaker Notes box: counted as pixels of the link's color along the link's words, none beside them. With JP_SHOT_DIR set, pictures of both in both window themes.
    static int linkInk(QTextEdit *e, int from, int to, const QColor &want)
    {
        QTextCursor a(e->document()), b(e->document());
        a.setPosition(from);
        b.setPosition(to);
        const QRect ra = e->cursorRect(a), rb = e->cursorRect(b);
        const QImage img = e->viewport()->grab().toImage();
        int n = 0;
        for (int y = ra.top(); y <= ra.bottom() + 3; ++y)
            for (int x = ra.left(); x < rb.left() && x < img.width(); ++x) {
                const QColor c = img.pixelColor(x, y);
                n += qAbs(c.red() - want.red()) + qAbs(c.green() - want.green()) + qAbs(c.blue() - want.blue()) < 90;
            }
        return n;
    }
    void linksAndMarkersShowInPresenterViewAndTheShowBox()
    {
        const QString dir = qEnvironmentVariable("JP_SHOT_DIR");
        for (int theme : {1, 2}) {
            if (!dir.isEmpty()) jp::applyUiTheme(theme);
            else if (theme == 2) break;
            const QString tag = theme == 1 ? QStringLiteral("light") : QStringLiteral("dark");
            DeckWindow w;
            Editor *ed = w.editor();
            w.resize(1280, 800);
            w.show();
            w.newSlide();
            ed->setCurrentPage(0);
            QTextDocument *story = ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId);
            fillMixedListNote(story);
            QTextCursor c(story);
            c.movePosition(QTextCursor::End);
            c.insertBlock();
            c.insertText(QStringLiteral("See "), QTextCharFormat());
            QTextCharFormat link;
            link.setAnchor(true);
            link.setAnchorHref(QStringLiteral("https://example.com/"));
            const int linkFrom = c.position();
            c.insertText(QStringLiteral("the linked words"), link);
            const int linkTo = c.position();
            c.insertText(QStringLiteral(" and more"), QTextCharFormat());
            w.notesPane()->reload();
            ed->undoStack()->clear();
            const QColor want = ed->doc()->colors.slot(Hyperlink);
            w.act("show.presenterView")->trigger();
            SlideShow *show = w.slideShow();
            QVERIFY(show && show->presenterView());
            QTextEdit *pv = show->presenterView()->findChild<QTextEdit *>(QStringLiteral("notesText"));
            QVERIFY(pv);
            show->presenterView()->resize(1400, 900);
            QTRY_VERIFY(pv->document()->size().height() > 100);
            pv->verticalScrollBar()->setValue(pv->verticalScrollBar()->maximum());   // (the link is in the last paragraph)
            QTRY_VERIFY2(linkInk(pv, linkFrom, linkTo, want) > 60, qPrintable(tag + QStringLiteral(": Presenter View draws the link in the link color")));
            QCOMPARE(linkInk(pv, linkFrom - 4, linkFrom, want), 0);   // (the words beside it are not)
            auto *marks = pv->findChild<ListMarkers *>();
            QVERIFY(marks);
            pv->verticalScrollBar()->setValue(0);
            QTRY_VERIFY2(marks->visibleMarkers().size() >= 8, qPrintable(tag + QStringLiteral(": Presenter View draws the markers")));
            for (const ListMarkers::Marker &m : marks->visibleMarkers()) QVERIFY2(m.rect.width() > 0 && m.rect.height() > 0, qPrintable(m.text));
            if (!dir.isEmpty()) show->presenterView()->grab().save(dir + QStringLiteral("/presenter-") + tag + QStringLiteral(".png"));
            show->close();
            QTRY_VERIFY(!w.slideShow());
        }
        // the Speaker Notes box
        for (int theme : {1, 2}) {
            if (!dir.isEmpty()) jp::applyUiTheme(theme);
            else if (theme == 2) break;
            const QString tag = theme == 1 ? QStringLiteral("light") : QStringLiteral("dark");
            auto d = showDeck(1);
            QTextDocument *story = d->storyDoc(d->pages[0]->notesStoryId);
            fillMixedListNote(story);
            QTextCursor c(story);
            c.movePosition(QTextCursor::End);
            c.insertBlock();
            c.insertText(QStringLiteral("See "), QTextCharFormat());
            QTextCharFormat link;
            link.setAnchor(true);
            link.setAnchorHref(QStringLiteral("https://example.com/"));
            const int linkFrom = c.position();
            c.insertText(QStringLiteral("the linked words"), link);
            const int linkTo = c.position();
            LayoutCache cache;
            SlideShow show(d.get(), &cache, 0);
            show.resize(960, 540);
            show.show();
            show.showSpeakerNotes();
            QTextEdit *box = show.speakerNotesBox();
            QVERIFY(box && box->isVisible());
            box->resize(box->width(), 700);
            auto *marks = box->findChild<ListMarkers *>();
            QVERIFY(marks);
            QTRY_VERIFY2(marks->visibleMarkers().size() >= 8, qPrintable(tag + QStringLiteral(": the box draws the markers")));
            QTRY_VERIFY2(linkInk(box, linkFrom, linkTo, d->colors.slot(Hyperlink)) > 60, qPrintable(tag + QStringLiteral(": the show's box draws the link in the link color")));
            QCOMPARE(linkInk(box, linkFrom - 4, linkFrom, d->colors.slot(Hyperlink)), 0);
            if (!dir.isEmpty()) box->grab().save(dir + QStringLiteral("/showbox-") + tag + QStringLiteral(".png"));
        }
        if (!dir.isEmpty()) jp::applyUiTheme(1);
    }

    // A picture of Presenter View with the mixed note, for looking at, when JP_SHOT_DIR names a folder.
    void pictureOfTheMarkersInPresenterView()
    {
        if (qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) QSKIP("JP_SHOT_DIR names the folder for the pictures");
        jp::applyUiTheme(1);
        DeckWindow w;
        SlideShow *show = nullptr;
        PresenterView *pv = presenterOnFormattedNote(w, &show);
        QVERIFY(show && pv);
        fillMixedListNote(w.editor()->doc()->storyDoc(w.editor()->doc()->pages[0]->notesStoryId));
        auto *edit = qobject_cast<StoryEdit *>(pv->findChild<QTextEdit *>(QStringLiteral("notesText")));
        QTRY_VERIFY(edit && edit->listMarkers()->visibleMarkers().size() >= 2);
        QCoreApplication::processEvents();
        pv->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + QStringLiteral("/presenter-view.png"));
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // The sequence of the notes everywhere: the caret in each notes view (under the slide, on the Notes Page, in Presenter View), the presentation becomes read only, a key and a format
    // command are refused, and nothing of the edit is left (a stale cursor crashed the next command).
    void aReadOnlyPresentationRefusesEditsInPresenterViewNotes()
    {
        DeckWindow w;
        SlideShow *show = nullptr;
        PresenterView *pv = presenterOnFormattedNote(w, &show);
        QVERIFY(show && pv);
        Editor *ed = w.editor();
        QTextEdit *edit = pv->findChild<QTextEdit *>(QStringLiteral("notesText"));
        QTextDocument *story = ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId);
        QTRY_VERIFY(edit->document() == story);
        QTest::mouseClick(edit->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(3, 3));
        QVERIFY(ed->editingStoryOnly());
        const QString before = markedNotes(story);
        w.setReadOnly(true);
        QVERIFY(!ed->isEditingText());
        QTest::keyClicks(edit, QStringLiteral("zz"));
        QTest::keyClick(edit, Qt::Key_B, Qt::ControlModifier);
        ed->typeText(QStringLiteral("zz"));
        ed->toggleBold();
        QCOMPARE(markedNotes(story), before);
        QVERIFY(!ed->isEditingText());
        w.setReadOnly(false);
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // Keys typed in Presenter View's notes are the notes' own, and never move the show: Return and Space would go to the next slide, Backspace and P to the one before, and N to the next,
    // whether the edit is made (an editable presentation) or refused (one open for reading only). Real key presses.
    void keysTypedInPresenterViewNotesNeverMoveTheShow_data()
    {
        QTest::addColumn<bool>("readOnly");
        QTest::newRow("editable") << false;
        QTest::newRow("open for reading only") << true;
    }
    void keysTypedInPresenterViewNotesNeverMoveTheShow()
    {
        QFETCH(bool, readOnly);
        DeckWindow w;
        Editor *ed = w.editor();
        w.resize(1280, 800);
        w.show();
        for (int i = 0; i < 3; ++i) w.newSlide();
        ed->setCurrentPage(1);
        QTextDocument *story = ed->doc()->storyDoc(ed->doc()->pages[1]->notesStoryId);
        QTextCursor(story).insertText(QStringLiteral("alpha beta"), QTextCharFormat());
        w.notesPane()->reload();
        ed->undoStack()->clear();
        w.act("show.presenterView")->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show && show->presenterView());
        QCOMPARE(show->currentSlide(), 1);
        auto *edit = show->presenterView()->findChild<QTextEdit *>(QStringLiteral("notesText"));
        QVERIFY(edit);
        QTRY_VERIFY(edit->document() == story);
        QTest::mouseClick(edit->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(3, 3));
        QVERIFY(ed->editingStoryOnly());
        if (readOnly) w.setReadOnly(true);
        const QString before = story->toPlainText();
        for (const int key : {Qt::Key_Return, Qt::Key_Backspace, Qt::Key_Delete, Qt::Key_Space, Qt::Key_Tab, Qt::Key_N, Qt::Key_P, Qt::Key_Return, Qt::Key_Backspace}) {
            QTest::keyClick(edit, Qt::Key(key));
            QVERIFY2(show->currentSlide() == 1, qPrintable(QStringLiteral("key %1 moved the show to slide %2").arg(key).arg(show->currentSlide())));
        }
        QTest::keyClicks(edit, QStringLiteral("n p "));
        QCOMPARE(show->currentSlide(), 1);
        // (an editable note took the keys as words; a read-only one is as it was)
        if (readOnly) QCOMPARE(story->toPlainText(), before);
        else QVERIFY(story->toPlainText() != before);
        // The keys that are the show's own still are: Page Down in the notes goes to the next slide.
        QTest::keyClick(edit, Qt::Key_PageDown);
        QTRY_COMPARE(show->currentSlide(), 2);
        if (readOnly) w.setReadOnly(false);
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // In a presentation open for reading only, a key that is a menu's access key (Alt and a letter) or another shortcut (Ctrl+Enter, Ctrl+Tab) is not an attempt to edit: the notes do
    // not say "open for reading only" for it, and do not take it, while the letter alone is refused with that message. Real key events sent to the notes' view.
    void shortcutKeysInPresenterViewNotesAreNotRefusedAsEdits()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.resize(1280, 800);
        w.show();
        for (int i = 0; i < 2; ++i) w.newSlide();
        ed->setCurrentPage(0);
        QTextDocument *story = ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId);
        QTextCursor(story).insertText(QStringLiteral("alpha beta"), QTextCharFormat());
        w.notesPane()->reload();
        ed->undoStack()->clear();
        w.act("show.presenterView")->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show && show->presenterView());
        auto *edit = show->presenterView()->findChild<StoryEdit *>(QStringLiteral("notesText"));
        QVERIFY(edit);
        QTRY_VERIFY(edit->document() == story);
        QTest::mouseClick(edit->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(3, 3));
        w.setReadOnly(true);
        QSignalSpy refused(edit, &StoryEdit::readOnlyRefused);
        auto press = [&](int key, Qt::KeyboardModifiers mods, const QString &text) {
            QKeyEvent ev(QEvent::KeyPress, key, mods, text);
            QApplication::sendEvent(edit, &ev);
        };
#ifdef Q_OS_MACOS
        const Qt::KeyboardModifiers menuKey = Qt::MetaModifier;   // (Option types other letters there)
#else
        const Qt::KeyboardModifiers menuKey = Qt::AltModifier;
#endif
        const QString before = story->toPlainText();
        press(Qt::Key_F, menuKey, QStringLiteral("f"));
        press(Qt::Key_Return, Qt::ControlModifier, QStringLiteral("\r"));
        press(Qt::Key_Tab, Qt::ControlModifier, QStringLiteral("\t"));
        QCOMPARE(refused.count(), 0);
        // The letter alone is refused, as before.
        press(Qt::Key_F, Qt::NoModifier, QStringLiteral("f"));
        QCOMPARE(refused.count(), 1);
        QCOMPARE(story->toPlainText(), before);
        w.setReadOnly(false);
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // Tab at the start of a list item moves it a level in and Shift+Tab out in Presenter View's notes too, and a Tab inside the item's words types a tab (real key presses).
    void tabAtTheStartOfAListItemInPresenterViewNotesChangesItsLevel()
    {
        DeckWindow w;
        SlideShow *show = nullptr;
        PresenterView *pv = presenterOnFormattedNote(w, &show);
        QVERIFY(show && pv);
        pv->resize(1400, 900);
        Editor *ed = w.editor();
        fillMixedListNote(ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId));
        auto *edit = qobject_cast<StoryEdit *>(pv->findChild<QTextEdit *>(QStringLiteral("notesText")));
        QVERIFY(edit);
        w.notesPane()->reload();
        pv->setNotesPointSize(pv->notesPointSize());
        QTRY_VERIFY(edit->document() == ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId));
        auto levelOf = [&](int number) {
            const QTextBlock b = ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId)->findBlockByNumber(number);
            return b.textList() ? b.textList()->format().intProperty(tp::ListLevel) : -1;
        };
        auto textOf = [&](int number) { return ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId)->findBlockByNumber(number).text(); };
        auto clickIn = [&](int number, int chars) {
            const QTextBlock b = edit->document()->findBlockByNumber(number);
            QTextCursor c(edit->document());
            c.setPosition(b.position() + chars);
            const QRect r = edit->cursorRect(c);
            QTextCursor c2(edit->document());
            c2.setPosition(b.position() + chars + 1);
            const QRect r2 = edit->cursorRect(c2);
            QTest::mouseClick(edit->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(r.x() + std::max(1, (r2.x() - r.x()) / 3), r.center().y()));
        };
        QCOMPARE(levelOf(1), 0);
        clickIn(1, 0);
        QTest::keyClick(edit, Qt::Key_Tab);
        QTRY_COMPARE(levelOf(1), 1);
        QCOMPARE(textOf(1), QStringLiteral("first point"));
        rn9Shot(pv, QStringLiteral("item7-presenter-tab"));
        clickIn(1, 0);
        QTest::keyClick(edit, Qt::Key_Backtab, Qt::ShiftModifier);
        QTRY_COMPARE(levelOf(1), 0);
        clickIn(1, 3);
        QTest::keyClick(edit, Qt::Key_Tab);
        QTRY_COMPARE(textOf(1), QStringLiteral("fir\tst point"));
        QCOMPARE(levelOf(1), 0);
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // A refused edit of a presentation open for reading only says so in Presenter View, which has no status bar: a line under the notes, in the words the other windows use, for a few seconds;
    // moving the caret says nothing.
    void aReadOnlyPresentationSaysSoInPresenterViewNotes()
    {
        DeckWindow w;
        SlideShow *show = nullptr;
        PresenterView *pv = presenterOnFormattedNote(w, &show);
        QVERIFY(show && pv);
        pv->resize(1400, 900);
        Editor *ed = w.editor();
        auto *edit = qobject_cast<StoryEdit *>(pv->findChild<QTextEdit *>(QStringLiteral("notesText")));
        QVERIFY(edit);
        QTRY_VERIFY(edit->document() == ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId));
        QLabel *line = pv->findChild<QLabel *>(QStringLiteral("readOnlyNote"));
        QVERIFY(line);
        QVERIFY(!line->isVisible());
        w.setReadOnly(true);
        QTest::mouseClick(edit->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(3, 3));
        QTest::keyClick(edit, Qt::Key_Right);
        QVERIFY(!line->isVisible());
        QTest::keyClicks(edit, QStringLiteral("z"));
        QVERIFY(line->isVisible());
        QCOMPARE(line->text(), QCoreApplication::translate("AppWindow", "This file is open for reading only. Use Save a Copy, or Save As, to make a copy you can change."));
        rn9Shot(pv, QStringLiteral("item8-presenter"));
        w.setReadOnly(false);
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // No command of a notes view reaches a slide object selected on the slide: Ctrl+B in Presenter View's notes formats the words selected there only.
    void aSlideObjectNeverReceivesABoldKeyFromPresenterViewNotes()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        ed->setCurrentPage(0);
        auto text = std::static_pointer_cast<TextItem>(ed->newTextBox(QRectF(72, 72, 400, 100), QStringLiteral("slide words")));
        ed->addItem(text);
        ed->select(text->id);
        const QByteArray item = QJsonDocument(text->toJson()).toJson(QJsonDocument::Compact);
        const QString slideWords = markedNotes(ed->doc()->storyDoc(text->storyId));
        SlideShow *show = nullptr;
        PresenterView *pv = presenterOnFormattedNote(w, &show);
        QVERIFY(show && pv);
        QTextEdit *edit = pv->findChild<QTextEdit *>(QStringLiteral("notesText"));
        QTRY_VERIFY(edit->document() == ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId));
        edit->setFocus();
        QTextCursor sel(edit->document());
        sel.setPosition(0);
        sel.setPosition(5, QTextCursor::KeepAnchor);
        edit->setTextCursor(sel);
        QTest::keyClick(edit, Qt::Key_I, Qt::ControlModifier);
        QCOMPARE(markedNotes(ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId)), QStringLiteral("[I]alpha[/I] [B]beta[/B] gamma"));
        QCOMPARE(QJsonDocument(text->toJson()).toJson(QJsonDocument::Compact), item);
        QCOMPARE(markedNotes(ed->doc()->storyDoc(text->storyId)), slideWords);
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // The window goes while the caret is in Presenter View's notes: the show and Presenter View go with it, and nothing is saved into the closing window.
    void aWindowGoingWithTheCaretInPresenterViewNotesLeavesNothing()
    {
        auto *w = new DeckWindow;
        SlideShow *show = nullptr;
        PresenterView *pv = presenterOnFormattedNote(*w, &show);
        QVERIFY(show && pv);
        QPointer<SlideShow> showGuard(show);
        QPointer<PresenterView> pvGuard(pv);
        QTextEdit *edit = pv->findChild<QTextEdit *>(QStringLiteral("notesText"));
        QTRY_VERIFY(edit->document() == w->editor()->doc()->storyDoc(w->editor()->doc()->pages[0]->notesStoryId));
        QTest::mouseClick(edit->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(3, 3));
        QTest::keyClicks(edit, QStringLiteral("zz"));
        delete w;
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QVERIFY(showGuard.isNull() && pvGuard.isNull());
    }

    // The Speaker Notes box shows the note formatted, follows the show, keeps the cut at 5,000 characters and its sentence (the cut text keeps its formatting, the sentence has none),
    // and, while Presenter View is up (it shows the notes: a box would be on the audience's screen), is not offered.
    void speakerNotesBoxShowsTheNoteFormatted()
    {
        auto d = showDeck(3);
        {
            QTextDocument *story = d->storyDoc(d->pages[0]->notesStoryId);
            story->clear();
            QTextCursor c(story);
            QTextCharFormat bold;
            bold.setFontWeight(QFont::Bold);
            c.insertText(QStringLiteral("plain "), QTextCharFormat());
            c.insertText(QStringLiteral("bold"), bold);
            c.insertBlock();
            c.insertText(QStringLiteral("second paragraph"), QTextCharFormat());
            QTextDocument *longer = d->storyDoc(d->pages[2]->notesStoryId);
            longer->clear();
            QTextCursor lc(longer);
            lc.insertText(QString(SlideShow::kSpeakerNotesLimit + 500, QLatin1Char('x')), bold);
        }
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        show.showSpeakerNotes();
        QTextEdit *box = show.speakerNotesBox();
        QVERIFY(box && box->isVisible());
        QCOMPARE(box->toPlainText(), QStringLiteral("plain bold\nsecond paragraph"));
        QCOMPARE(markedNotes(box->document()), QStringLiteral("plain [B]bold[/B]|second paragraph"));
        QVERIFY(!box->isModal() && box->isReadOnly() && box->focusPolicy() == Qt::NoFocus);
        // (the note itself is as it was)
        QCOMPARE(markedNotes(d->storyDoc(d->pages[0]->notesStoryId)), QStringLiteral("plain [B]bold[/B]|second paragraph"));
        show.goTo(2);
        const QString cutNote = QStringLiteral("\n\nThese notes were cut short because they are very long.");
        QVERIFY(box->toPlainText().endsWith(cutNote));
        QCOMPARE(box->toPlainText().size(), SlideShow::kSpeakerNotesLimit + cutNote.size());
        const QString shown = markedNotes(box->document());
        QVERIFY2(shown.startsWith(QStringLiteral("[B]xxx")), qPrintable(shown.left(20)));
        QVERIFY2(shown.contains(QStringLiteral("x[/B]|")) && shown.endsWith(QStringLiteral("very long.")), qPrintable(shown.right(80)));
        show.goTo(0);
        QCOMPARE(markedNotes(box->document()), QStringLiteral("plain [B]bold[/B]|second paragraph"));
    }

    // The Speaker Notes box draws the bullets and numbers of the note (its copy keeps the lists) left of the words, inside the box, with the texts the printed page shows.
    void speakerNotesBoxDrawsTheMarkers()
    {
        auto d = showDeck(1);
        fillMixedListNote(d->storyDoc(d->pages[0]->notesStoryId));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        show.showSpeakerNotes();
        QTextEdit *box = show.speakerNotesBox();
        QVERIFY(box && box->isVisible());
        auto *marks = box->findChild<ListMarkers *>();
        QVERIFY(marks);
        box->resize(box->width(), 700);
        QStringList texts;
        QTRY_VERIFY([&] {
            texts.clear();
            for (const ListMarkers::Marker &m : marks->visibleMarkers()) {
                texts << m.text;
                if (m.rect.left() < 0 || m.rect.right() > box->width()) return false;
            }
            return texts.size() == 10;
        }());
        QCOMPARE(texts, (QStringList{"•", "•", "◦", "1.", "2.", "3.", "a.", "4.", "1.", "2."}));
        QCOMPARE(marks->gutterLeft(), 0);   // (the markers hang in the indent the story gives its items: no gutter)
        for (const ListMarkers::Marker &m : marks->visibleMarkers()) {
            QTextCursor c(box->document());
            c.setPosition(m.blockPos);
            QVERIFY2(m.rect.right() < box->viewport()->x() + box->cursorRect(c).left(), qPrintable(m.text));
        }
    }

    // The markers of the Speaker Notes box are the size print gives them: the size of the item's own words (numbers and letters were drawn larger than the words, bullets were not), for notes
    // whose words have no size of their own and for notes whose words and paragraph marks name one.
    void speakerNotesBoxMarkersAreTheSizeOfTheirWords()
    {
        for (int named = 0; named < 2; ++named) {
            auto d = showDeck(1);
            QTextDocument *story = d->storyDoc(d->pages[0]->notesStoryId);
            fillMixedListNote(story);
            if (named) {
                QTextCursor c(story);
                c.beginEditBlock();
                for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
                    QTextCharFormat sized;
                    sized.setFontPointSize(11);
                    c.setPosition(b.position());
                    c.setPosition(b.position() + std::max(0, b.length() - 1), QTextCursor::KeepAnchor);
                    c.mergeCharFormat(sized);
                    c.setPosition(b.position());
                    c.mergeBlockCharFormat(sized);
                }
                c.endEditBlock();
            }
            LayoutCache cache;
            SlideShow show(d.get(), &cache, 0);
            show.resize(960, 540);
            show.show();
            show.showSpeakerNotes();
            QTextEdit *box = show.speakerNotesBox();
            QVERIFY(box && box->isVisible());
            auto *marks = box->findChild<ListMarkers *>();
            QVERIFY(marks);
            box->resize(box->width(), 700);
            QTRY_COMPARE(marks->visibleMarkers().size(), 10);
            rn9Shot(box, QStringLiteral("item5-named%1").arg(named));
            for (const ListMarkers::Marker &m : marks->visibleMarkers()) {
                const QTextBlock b = box->document()->findBlock(m.blockPos);
                QVERIFY(b.layout() && b.layout()->lineCount() > 0);
                const double line = b.layout()->lineAt(0).height();
                QVERIFY2(std::abs(m.rect.height() - line) <= 2.0, qPrintable(QStringLiteral("%1 (words named %2): the marker is %3 high, the line %4").arg(m.text).arg(named).arg(m.rect.height()).arg(line)));
            }
        }
    }

    // The Speaker Notes box lays a list as the printed page does: a paragraph that is no list item starts at the left edge (there is no gutter), a list item's first line starts where its other
    // lines do, and its marker hangs left of them by the story's hang (it started plain paragraphs at the markers' gutter, and a wrapped item's first line left of the rest).
    void speakerNotesBoxLaysAListAsThePrintedPageDoes()
    {
        auto d = showDeck(1);
        QTextDocument *story = d->storyDoc(d->pages[0]->notesStoryId);
        fillMixedListNote(story);
        {
            QTextCursor c(story->findBlockByNumber(1));
            c.movePosition(QTextCursor::EndOfBlock);
            c.insertText(QStringLiteral(" and so many more words that this item goes on over several lines of the box and wraps again and again"));
            c.insertText(QString(QStringLiteral(" more words to wrap")).repeated(20));
        }
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        show.showSpeakerNotes();
        QTextEdit *box = show.speakerNotesBox();
        QVERIFY(box && box->isVisible());
        box->resize(box->width(), 900);
        auto *marks = box->findChild<ListMarkers *>();
        QVERIFY(marks);
        QTRY_VERIFY(marks->visibleMarkers().size() >= 8);
        QCOMPARE(marks->gutterLeft(), 0);
        QTextDocument *doc = box->document();
        QTextBlock plain = doc->findBlockByNumber(0), item = doc->findBlockByNumber(1);
        QTRY_VERIFY((doc->size(), item.layout()->lineCount() >= 3));
        QCOMPARE(plain.layout()->lineAt(0).x() + plain.layout()->position().x(), 0.0);
        const double second = item.layout()->lineAt(1).x();
        QVERIFY2(qAbs(item.layout()->lineAt(0).x() - second) < 0.5, qPrintable(QStringLiteral("first line at %1, second at %2").arg(item.layout()->lineAt(0).x()).arg(second)));
        const ListMarkers::Marker m = marks->visibleMarkers()[0];
        QCOMPARE(m.blockPos, item.position());
        const double vx = box->viewport()->x();   // (the marker is in the view's own coordinates: the words' are the viewport's)
        QVERIFY2(m.rect.left() - vx >= -0.5 && m.rect.left() - vx < second - 4 && m.rect.right() - vx <= second + 1, qPrintable(QStringLiteral("marker %1..%2, text at %3").arg(m.rect.left() - vx).arg(m.rect.right() - vx).arg(second)));
        // a level in is as far in as the story has it: its marker is where the first level's text is
        const ListMarkers::Marker deeper = marks->visibleMarkers()[2];
        QVERIFY2(qAbs(deeper.rect.left() - vx - second) < 3, qPrintable(QStringLiteral("level 2 marker at %1, level 1 text at %2").arg(deeper.rect.left() - vx).arg(second)));
    }

    void speakerNotesBoxIsNotOfferedWhilePresenterViewIsUp()
    {
        DeckWindow w;
        SlideShow *show = nullptr;
        PresenterView *pv = presenterOnFormattedNote(w, &show);
        QVERIFY(show && pv && pv->isVisible());
        show->showSpeakerNotes();
        QVERIFY(!show->speakerNotesBox());
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // Keys typed in Presenter View's notes do not make the show draw its slides again or look through the pages (that work is of the presentation's size): the show is told of a change
    // when typing is over, so an undo of it still brings the notes back into the box.
    void typingInPresenterViewNotesDoesNotRedrawTheShowForEachKey()
    {
        DeckWindow w;
        SlideShow *show = nullptr;
        PresenterView *pv = presenterOnFormattedNote(w, &show);
        QVERIFY(show && pv);
        Editor *ed = w.editor();
        QTextEdit *edit = pv->findChild<QTextEdit *>(QStringLiteral("notesText"));
        QTRY_VERIFY(edit->document() == ed->doc()->storyDoc(ed->doc()->pages[0]->notesStoryId));
        QTest::mouseClick(edit->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(3, 3));
        QVERIFY(ed->editingStoryOnly());
        QSignalSpy told(show, &SlideShow::slidesChanged);
        for (int i = 0; i < 30; ++i) QTest::keyClick(edit, Qt::Key_X);
        QVERIFY2(told.count() <= 1, qPrintable(QString::number(told.count())));
        pv->commitNotes();
        ed->undo();
        QTRY_VERIFY(told.count() >= 1);
        QTRY_COMPARE(edit->toPlainText(), QStringLiteral("alpha beta gamma"));
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // Words typed in the notes pane and not yet closed into a step are closed when the show starts: they are one step of Undo of their own, the pane holds no typing, and
    // the show's notes hold the words.
    void startingTheShowClosesTheWordsTypedInThePane()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        ed->setCurrentPage(0);
        QTest::keyClicks(w.notesPane(), QStringLiteral("Spoken words"));
        QVERIFY(w.notesPane()->hasTyping());
        const int steps = ed->undoStack()->count();
        w.act("show.presenterView")->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show && show->presenterView());
        QVERIFY(!w.notesPane()->hasTyping());
        QCOMPARE(ed->undoStack()->count(), steps + 1);
        QTextEdit *edit = show->presenterView()->findChild<QTextEdit *>(QStringLiteral("notesText"));
        QVERIFY(edit);
        QTRY_COMPARE(edit->toPlainText(), QStringLiteral("Spoken words"));
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QStringLiteral("Spoken words"));
        show->close();
        QTRY_VERIFY(!w.slideShow());
        ed->undo();
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QString());
    }

    // With the notes box holding the keyboard, Page Down and Page Up still move the show; letters and arrows stay in the text; a click on Next
    // gives the keyboard back to the show.
    void presenterViewNotesKeepTheShowsPageKeys()
    {
        DeckWindow w;
        w.newSlide();
        w.newSlide();
        w.editor()->setCurrentPage(0);
        w.act("show.presenterView")->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show && show->presenterView());
        PresenterView *pv = show->presenterView();
        pv->activateWindow();
        QTextEdit *edit = pv->findChild<QTextEdit *>(QStringLiteral("notesText"));
        edit->setFocus();
        QTest::keyClicks(edit, QStringLiteral("abc"));
        QCOMPARE(show->currentSlide(), 0);   // letters are the text's
        QTest::keyClick(edit, Qt::Key_Left);
        QCOMPARE(show->currentSlide(), 0);   // so are the arrows
        QTest::keyClick(edit, Qt::Key_PageDown);
        QCOMPARE(show->currentSlide(), 1);
        QTest::keyClick(edit, Qt::Key_PageUp);
        QCOMPARE(show->currentSlide(), 0);
        QCOMPARE(edit->toPlainText(), QStringLiteral("abc"));
        // Clicking a button of Presenter View leaves the box (what was typed is saved).
        QTRY_VERIFY(edit->hasFocus());
        QTest::mousePress(pv->findChild<QAbstractButton *>(QStringLiteral("nextButton")), Qt::LeftButton);
        QVERIFY(!edit->hasFocus());
        edit->setFocus();
        QTRY_VERIFY(edit->hasFocus());
        QTest::mousePress(pv->findChild<QAbstractButton *>(QStringLiteral("endButton")), Qt::LeftButton);   // (pressed only: the show goes on)
        QVERIFY(!edit->hasFocus());
        edit->setFocus();
        QTRY_VERIFY(edit->hasFocus());
        QTest::mouseClick(pv->findChild<QWidget *>(QStringLiteral("currentSlide")), Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
        QVERIFY(!edit->hasFocus());
        show->close();
        QTRY_VERIFY(!w.slideShow());
    }

    // A presentation open for reading only: Presenter View's notes are read only, so nothing typed is dropped silently.
    void presenterViewNotesAreReadOnlyInAReadOnlyPresentation()
    {
        DeckWindow w;
        w.newSlide();
        w.editor()->setReadOnly(true);
        w.act("show.presenterView")->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show && show->presenterView());
        QVERIFY(show->notesReadOnly());
        QTextEdit *box = show->presenterView()->findChild<QTextEdit *>(QStringLiteral("notesText"));
        QVERIFY(box->isReadOnly());
        // And it follows the file becoming writable and read only again during the show.
        w.editor()->setReadOnly(false);
        show->presenterView()->refresh();
        QVERIFY(!box->isReadOnly());
        w.editor()->setReadOnly(true);
        box->setFocus();   // (the box has the keyboard again)
        QTRY_VERIFY(box->isReadOnly());
        show->close();
        QTRY_VERIFY(!w.slideShow());
        w.editor()->setReadOnly(false);
    }

    // With JO_SHOT_DIR set: pictures of Presenter View with notes being typed, and of the Slide Show tab's list.
    void presenterViewShots()
    {
        if (qEnvironmentVariableIsEmpty("JO_SHOT_DIR")) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        DeckWindow w;
        w.resize(1400, 900);
        Editor *ed = w.editor();
        ed->setDocument(showDeck(3));
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        QTest::keyClicks(w.notesPane(), QStringLiteral("Welcome everyone. Start with the numbers."));
        w.notesPane()->commit();
        w.act("show.presenterView")->trigger();
        SlideShow *show = w.slideShow();
        QVERIFY(show && show->presenterView());
        PresenterView *pv = show->presenterView();
        pv->resize(1200, 760);
        QTextEdit *edit = pv->findChild<QTextEdit *>(QStringLiteral("notesText"));
        edit->setFocus();
        edit->moveCursor(QTextCursor::End);
        QTest::keyClicks(edit, QStringLiteral(" Then pause for questions."));
        QTRY_VERIFY(pv->isVisible() && edit->toPlainText().endsWith(QStringLiteral("questions.")));
        shotOf(pv, QStringLiteral("show-presenter-view-notes"));
        show->close();
        QTRY_VERIFY(!w.slideShow());
        w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Slide Show")));
        auto *box = w.ribbon()->findChild<QComboBox *>(QStringLiteral("showOn"));
        QVERIFY(box);
        QTRY_VERIFY(box->isVisible());
        shotOf(&w, QStringLiteral("show-ribbon"));
        box->showPopup();
        QVERIFY(box->view());
        QTRY_VERIFY(box->view()->isVisible());
        shotOf(box->view()->window(), QStringLiteral("show-ribbon-list"));
        box->hidePopup();
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

    // The slide a rehearsal is on is deleted under it: the time goes to the slide that is now on the screen, and none is kept for the one that is gone; with every slide gone, the clock stops.
    void rehearsalTimesTheSlideThatTakesThePlaceOfADeletedOne()
    {
        auto d = showDeck(3);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        qint64 now = 1000;
        show.setClock([&now] { return now; });
        show.setRecording(true);
        show.show();
        show.next();
        QCOMPARE(show.currentSlide(), 1);
        now += 2000;
        d->pages.erase(d->pages.begin() + 1);
        show.documentChanged();
        QCOMPARE(show.currentSlide(), 1);
        QCOMPARE(show.slideClock(), qint64(0));            // (counted from when the new slide came on)
        now += 4000;
        show.next();
        QCOMPARE(show.currentSlide(), -1);
        QCOMPARE(show.recordedSeconds().value(1), 4.0);
        // every slide gone
        SlideShow other(d.get(), &cache, 0);
        other.setClock([&now] { return now; });
        other.setRecording(true);
        other.show();
        now += 3000;
        d->pages.clear();
        other.documentChanged();
        now += 5000;
        QCOMPARE(other.slideClock(), qint64(0));
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

    // Reading View: the show plays inside the window (the title bar and status bar stay, the ribbon and panes go), and Esc returns to the view it was started from.
    void readingViewPlaysTheShowInTheWindowAndEscGoesBack()
    {
        DeckWindow w;
        w.resize(1100, 700);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.newSlide();
        w.newSlide();
        w.editor()->setCurrentPage(0);
        w.act("view.outline")->trigger();
        QCOMPARE(int(w.view()), int(DeckWindow::View::Outline));
        QVERIFY(!w.inReadingView());
        w.act("view.reading")->trigger();
        QVERIFY(w.inReadingView());
        QPointer<SlideShow> show = w.readingShow();
        QVERIFY(show->isEmbedded());
        QVERIFY(show->isVisible());
        QVERIFY(!show->isWindow());
        QVERIFY(w.isAncestorOf(show));
        QVERIFY(w.statusBar()->isVisible());
        QVERIFY(!w.ribbon()->isVisible());
        QVERIFY(w.act("view.reading")->isChecked());
        QVERIFY(w.findChild<QToolButton *>(QStringLiteral("readingNext")));
        QCOMPARE(w.findChild<QLabel *>(QStringLiteral("readingPlace"))->text(), QStringLiteral("Slide 1 of 3"));
        // A key goes to the show, not to the window's shortcuts.
        QTest::keyClick(show, Qt::Key_Right);
        QCOMPARE(show->currentSlide(), 1);
        QCOMPARE(w.findChild<QLabel *>(QStringLiteral("readingPlace"))->text(), QStringLiteral("Slide 2 of 3"));
        // Ctrl+P is the pen here, not Print.
        QTest::keyClick(show, Qt::Key_P, Qt::ControlModifier);
        QCOMPARE(int(show->pointer()), int(SlideShow::Pointer::Pen));
        QTest::keyClick(show, Qt::Key_Escape);   // the arrow first
        QVERIFY(w.inReadingView());
        QTest::keyClick(show, Qt::Key_Escape);
        QVERIFY(!w.inReadingView());
        QVERIFY(w.ribbon()->isVisible());
        QCOMPARE(int(w.view()), int(DeckWindow::View::Outline));
        QVERIFY(w.act("view.outline")->isChecked());
        QCOMPARE(w.editor()->currentPage(), 1);   // the window is on the slide the show ended on
        QTRY_VERIFY(show.isNull());
        // Another view ends it too.
        w.act("view.reading")->trigger();
        QVERIFY(w.inReadingView());
        QPointer<SlideShow> again = w.readingShow();
        w.act("view.normal")->trigger();
        QVERIFY(!w.inReadingView());
        QCOMPARE(int(w.view()), int(DeckWindow::View::Normal));
        QTRY_VERIFY(again.isNull());
    }

    // The status bar's Previous, Next, and Menu work the show; the menu is the show's own, and End Show in it ends Reading View.
    void readingViewStatusBarControlsAndMenu()
    {
        DeckWindow w;
        w.resize(1100, 700);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.newSlide();
        w.newSlide();
        w.editor()->setCurrentPage(0);
        w.act("view.reading")->trigger();
        QVERIFY(w.inReadingView());
        SlideShow *show = w.readingShow();
        auto *next = w.findChild<QToolButton *>(QStringLiteral("readingNext"));
        auto *previous = w.findChild<QToolButton *>(QStringLiteral("readingPrevious"));
        auto *menuButton = w.findChild<QToolButton *>(QStringLiteral("readingMenu"));
        QVERIFY(next && previous && menuButton);
        next->click();
        next->click();
        QCOMPARE(show->currentSlide(), 2);
        previous->click();
        QCOMPARE(show->currentSlide(), 1);
        menuButton->click();
        QMenu *menu = nullptr;
        for (int i = 0; i < 100 && !menu; ++i) {
            for (QWidget *top : QApplication::topLevelWidgets())
                if (top->objectName() == QLatin1String("showMenu") && top->isVisible()) menu = qobject_cast<QMenu *>(top);
            if (!menu) QCoreApplication::processEvents();
        }
        QVERIFY(menu);
        QPointer<QMenu> guarded = menu;
        QVERIFY(menu->findChild<QAction *>(QStringLiteral("menuLastViewed")));
        QAction *end = menu->findChild<QAction *>(QStringLiteral("menuEnd"));
        QVERIFY(end);
        end->trigger();
        menu->close();
        QVERIFY(!w.inReadingView());
        QVERIFY(w.ribbon()->isVisible());
        QTRY_VERIFY(guarded.isNull());
        QTRY_VERIFY(!w.findChild<QToolButton *>(QStringLiteral("readingNext")));
    }

    // Ending Reading View in the middle of an effect stops the frame clock: no frame is drawn afterward and the show goes (nothing it queued runs on a freed show).
    void endingReadingViewMidAnimationStopsEverything()
    {
        DeckWindow w;
        w.resize(1100, 700);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Page *page = w.editor()->doc()->pages[0].get();
        QVERIFY(page->items.size() >= 2);
        numberShowShapes(*page);
        page->props[QStringLiteral("timingXml")] = animTiming(animSteps({animFadeIn(2, 600000), animAppear(3)}));
        w.act("view.reading")->trigger();
        QVERIFY(w.inReadingView());
        QPointer<SlideShow> show = w.readingShow();
        int frames = 0;
        connect(show.data(), &SlideShow::frameChanged, &w, [&frames] { ++frames; });
        show->next();
        QVERIFY(show->isPlayingStep());
        QTRY_VERIFY(frames > 0);
        QVERIFY(show->frameClockRunning());
        QTest::keyClick(show, Qt::Key_Escape);   // mid-animation: the first Esc ends the view (the pointer is the arrow)
        QVERIFY(!w.inReadingView());
        QVERIFY(show->isHidden());
        QVERIFY(!show->frameClockRunning());
        const int atEnd = frames;
        for (int i = 0; i < 30; ++i) QCoreApplication::processEvents();
        QCOMPARE(frames, atEnd);
        QTRY_VERIFY(show.isNull());
        for (int i = 0; i < 30; ++i) QCoreApplication::processEvents();
        QCOMPARE(frames, atEnd);
        // A show that never played a step, ended in the same way, leaves nothing either.
        w.act("view.reading")->trigger();
        QPointer<SlideShow> second = w.readingShow();
        QVERIFY(second);
        w.act("view.normal")->trigger();
        QTRY_VERIFY(second.isNull());
    }

    // Reading View plays triggers too (Tab to the object, Enter), and ending the view in the middle of the effect a trigger started stops the frame clock and leaves nothing running.
    void readingViewPlaysTriggersAndEndsMidTrigger()
    {
        DeckWindow w;
        w.resize(1100, 700);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        Page *page = w.editor()->doc()->pages[0].get();
        QVERIFY(page->items.size() >= 2);
        numberShowShapes(*page);
        page->props[QStringLiteral("timingXml")] = animTriggerTiming(animSteps({}), {{3, animFadeIn(2, 600000)}});
        w.act("view.reading")->trigger();
        QVERIFY(w.inReadingView());
        QPointer<SlideShow> show = w.readingShow();
        QVERIFY(show);
        int frames = 0;
        connect(show.data(), &SlideShow::frameChanged, &w, [&frames] { ++frames; });
        QCOMPARE(show->stepCount(), 0);
        QTest::keyClick(show, Qt::Key_Tab);
        QTest::keyClick(show, Qt::Key_Return);
        QCOMPARE(show->triggerRuns(), 1);
        QVERIFY(show->isPlayingStep());
        QTRY_VERIFY(frames > 0);
        QVERIFY(show->frameClockRunning());
        QTest::keyClick(show, Qt::Key_Escape);
        QVERIFY(!w.inReadingView());
        QVERIFY(!show->frameClockRunning());
        const int atEnd = frames;
        for (int i = 0; i < 30; ++i) QCoreApplication::processEvents();
        QCOMPARE(frames, atEnd);
        QTRY_VERIFY(show.isNull());
    }

    // Pictures of a show with a trigger object, before and after its click, when JO_SHOT_DIR names a folder (the pointer is not in a grabbed picture).
    void triggerShowPictures()
    {
        if (qEnvironmentVariableIsEmpty("JO_SHOT_DIR")) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        auto d = editDeck(2);
        d->pages[0]->items[2]->name = QStringLiteral("Click me");
        d->pages[0]->props[QStringLiteral("timingXml")] = animTriggerTiming(animSteps({}), {{11, animAppear(3)}});
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        QVERIFY(show.grab().save(dir + QStringLiteral("/trigger-show-before.png")));
        QTest::mouseMove(&show, d->pages[0]->items[3]->bounds().center().toPoint());
        QCOMPARE(int(show.cursor().shape()), int(Qt::PointingHandCursor));
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, d->pages[0]->items[3]->bounds().center().toPoint());
        QVERIFY(show.grab().save(dir + QStringLiteral("/trigger-show-after.png")));
    }

    // A click on an object that has effects waiting for it plays them, at once, apart from the main sequence; a click anywhere else still takes the next step of the main sequence; the object can be clicked again; the pointer is a hand over it.
    void aClickOnATriggerObjectPlaysItsEffects()
    {
        auto d = editDeck(2);
        d->pages[0]->props[QStringLiteral("timingXml")] = animTriggerTiming(animSteps({animAppear(10)}), {{11, animAppear(3)}});
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QCOMPARE(show.stepCount(), 1);
        const QPoint box = d->pages[0]->items[3]->bounds().center().toPoint();
        QTest::mouseMove(&show, box);
        QCOMPARE(int(show.cursor().shape()), int(Qt::PointingHandCursor));
        QTest::mouseMove(&show, QPoint(5, 530));
        QVERIFY(show.cursor().shape() != Qt::PointingHandCursor);
        // The object's click plays its effect and leaves the main sequence's step to the next ordinary click.
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, box);
        QCOMPARE(show.triggerRuns(), 1);
        QCOMPARE(show.step(), 1);
        QCOMPARE(show.stepCount(), 2);
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(5, 530));
        QCOMPARE(show.step(), 2);
        QCOMPARE(show.currentSlide(), 0);
        // Again, and again: each click plays it once more.
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, box);
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, box);
        QCOMPARE(show.triggerRuns(), 3);
        QCOMPARE(show.step(), 4);
        // Going back takes the object's run back, and leaves the main sequence's steps as they were.
        show.previous();
        show.previous();
        show.previous();
        QCOMPARE(show.step(), 1);
        QCOMPARE(show.stepCount(), 2);
        QCOMPARE(show.currentSlide(), 0);
        // A slide shown again starts clean.
        show.goTo(0);
        QCOMPARE(show.stepCount(), 1);
        QCOMPARE(show.step(), 0);
    }

    // Tab reaches an object with a trigger, the same way it reaches a link, and Enter plays it.
    void aTriggerObjectIsReachedWithTabAndEnter()
    {
        auto d = editDeck(2);
        d->pages[0]->props[QStringLiteral("timingXml")] = animTriggerTiming(animSteps({animAppear(10)}), {{11, animAppear(3)}});
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        QTest::keyClick(&show, Qt::Key_Tab);
        QCOMPARE(show.triggerRuns(), 0);
        QTest::keyClick(&show, Qt::Key_Return);
        QCOMPARE(show.triggerRuns(), 1);
        QCOMPARE(show.step(), 1);
        QCOMPARE(show.stepCount(), 2);
    }

    // The color of a box's middle in the show as it is drawn: whether the box is there (a box's fill is red, the slide's ground is not).
    static bool boxShown(SlideShow &show, const Item &box)
    {
        const QColor c = show.grab().toImage().pixelColor(box.bounds().center().toPoint());
        return c.red() > 150 && c.green() < 100;
    }

    // Two slides, the first with `boxes` boxes numbered from 10 (as editDeck makes them).
    static std::unique_ptr<Document> twoSlideEditDeck(int boxes)
    {
        auto d = showDeck(2);
        Page &slide = *d->pages[0];
        numberShowShapes(slide);
        for (int i = 0; i < boxes; ++i) {
            auto box = std::make_shared<ShapeItem>();
            box->rect = QRectF(20 + 90 * (i % 8), 400 + 40 * (i / 8), 80, 30);
            box->fill = Fill::solid(ColorRef::rgb(QColor(200, 60, 40)));
            box->sourceId = 10 + i;
            slide.items.push_back(box);
        }
        return d;
    }

    // A file that names a trigger object the slide does not have, triggers by the hundred, or an effect that aims at its own trigger object cannot make the show loop, grow without end, or take a click.
    void hostileTriggersAreHeldToLimits()
    {
        auto d = editDeck(2);
        QVector<QPair<int, QString>> waits;
        waits.push_back({99, animAppear(3)});                  // an object that is not on the slide
        waits.push_back({10, animAppear(77)});                 // an effect on an object that is not there
        waits.push_back({10, animFadeIn(10, 1)});              // on itself
        for (int i = 0; i < 300; ++i) waits.push_back({11, animAppear(3)});   // many times over, on one object
        const QString xml = animTriggerTiming(animSteps({animAppear(10)}), waits);
        // The sequences of one object are one trigger, each effect kept up to the limit.
        const deck::Timeline read = deck::readTimeline(xml);
        QCOMPARE(int(read.triggers.size()), 3);
        QCOMPARE(int(read.triggers[2].effects.size()), deck::kMaxTriggerEffects);
        d->pages[0]->props[QStringLiteral("timingXml")] = xml;
        const deck::Timeline playable = deck::playableTimeline(*d, *d->pages[0]);
        QVERIFY(!std::any_of(playable.triggers.cbegin(), playable.triggers.cend(), [](const deck::Trigger &t) { return t.shape == 99; }));
        QCOMPARE(int(std::count_if(playable.triggers.cbegin(), playable.triggers.cend(), [](const deck::Trigger &t) { return t.shape == 10; })), 1);   // (only the one that is aimed at the slide's own objects)
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        const QPoint box = d->pages[0]->items[3]->bounds().center().toPoint();
        const QPoint gone = d->pages[0]->items[2]->bounds().center().toPoint();   // (box 10: not there until the main sequence's step brings it in)
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, gone);
        QCOMPARE(show.triggerRuns(), 0);
        QCOMPARE(show.step(), 1);   // an ordinary click: the main sequence's step
        for (int i = 0; i < 500; ++i) QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, box);
        QCOMPARE(show.triggerRuns(), 500);
        QCOMPARE(show.stepCount(), 501);
        QCOMPARE(show.currentSlide(), 0);
        // Past that a click on the object does nothing: it is not a click on the slide either.
        const int step = show.step();
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, box);
        QCOMPARE(show.triggerRuns(), 500);
        QCOMPARE(show.currentSlide(), 0);
        QCOMPARE(show.step(), step);
        QVERIFY(!show.isPlayingStep());
    }

    // Every interactive sequence a file gives one object plays at its click, in the file's order.
    void everySequenceOfAnObjectPlaysAtItsClick()
    {
        auto d = editDeck(4);
        d->pages[0]->props[QStringLiteral("timingXml")] = animTriggerTiming(animSteps({}), {{11, animAppear(10)}, {11, animAppear(12)}, {11, animAppear(13)}});
        const deck::Timeline tl = deck::playableTimeline(*d, *d->pages[0]);
        QCOMPARE(int(tl.triggers.size()), 1);
        QCOMPARE(int(tl.triggers[0].effects.size()), 3);
        QCOMPARE(tl.triggers[0].effects[0].behaviors[0].target.shape, 10);
        QCOMPARE(tl.triggers[0].effects[2].behaviors[0].target.shape, 13);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        const ItemList &items = d->pages[0]->items;
        QVERIFY(!boxShown(show, *items[2]) && !boxShown(show, *items[4]) && !boxShown(show, *items[5]));
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, items[3]->bounds().center().toPoint());
        QCOMPARE(show.triggerRuns(), 1);
        QTRY_VERIFY(!show.isPlayingStep());
        QVERIFY(boxShown(show, *items[2]) && boxShown(show, *items[4]) && boxShown(show, *items[5]));
    }

    // A trigger that can never be clicked (its object hidden, off the slide, with no area, brought in only by itself, or by one that waits for it) does not keep what it brings in away
    // for the whole show; one that can be clicked still does.
    void aTriggerNobodyCanClickLeavesItsTargetShown()
    {
        auto d = editDeck(11);
        const ItemList &items = d->pages[0]->items;
        auto box = [&](int id) -> Item & { return *items[2 + id - 10]; };
        box(10).hidden = true;                                  // 10, hidden, waits to bring in 12
        box(11).rect = QRectF(5000, 5000, 80, 30);              // 11, off the slide, brings in 13
        box(14).rect = QRectF(20, 20, 0, 0);                    // 14, with no area, brings in 15
        QVector<QPair<int, QString>> waits = {{10, animAppear(12)}, {11, animAppear(13)}, {14, animAppear(15)}, {16, animAppear(16)}, {17, animAppear(18)}, {18, animAppear(17)}, {19, animAppear(20)}};
        d->pages[0]->props[QStringLiteral("timingXml")] = animTriggerTiming(animSteps({}), waits);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        for (const int id : {12, 13, 15, 16, 17, 18}) QVERIFY2(boxShown(show, box(id)), qPrintable(QString::number(id)));
        // The one that can be clicked: its target is not there until the click.
        QVERIFY(!boxShown(show, box(20)));
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, box(19).bounds().center().toPoint());
        QTRY_VERIFY(!show.isPlayingStep());
        QVERIFY(boxShown(show, box(20)));
        // The click the object cannot have is not taken from the slide.
        QCOMPARE(show.triggerRuns(), 1);
    }

    // After the last run, what was still to come in is shown, and a click on the object that waits does nothing.
    void whenTheRunsAreUsedUpEverythingIsShown()
    {
        auto d = editDeck(5);
        const ItemList &items = d->pages[0]->items;
        d->pages[0]->props[QStringLiteral("timingXml")] = animTriggerTiming(animSteps({}), {{11, animAppear(12)}, {13, animAppear(14)}});
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QVERIFY(!boxShown(show, *items[6]));   // (box 14 waits for a click on box 13)
        const QPoint first = items[3]->bounds().center().toPoint();
        for (int i = 0; i < 500; ++i) QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, first);
        QCOMPARE(show.triggerRuns(), 500);
        QTRY_VERIFY(!show.isPlayingStep());
        QVERIFY(boxShown(show, *items[6]));
        const int step = show.step();
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, items[5]->bounds().center().toPoint());
        QCOMPARE(show.triggerRuns(), 500);
        QCOMPARE(show.step(), step);
        QCOMPARE(show.currentSlide(), 0);
        // The slide shown again waits for its clicks again.
        show.goTo(0);
        QCOMPARE(show.triggerRuns(), 0);
        QVERIFY(!boxShown(show, *items[6]));
    }

    // A click on an object that waits for a click, while a transition plays, does not move the show on.
    void aTriggerClickDuringATransitionDoesNothing()
    {
        auto d = twoSlideEditDeck(2);
        d->pages[0]->props[QStringLiteral("timingXml")] = animTriggerTiming(animSteps({}), {{11, animAppear(10)}});
        deck::setTransition(*d->pages[1], showTransition("fade", 30));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        show.next();
        QVERIFY(show.isAnimating());
        const int target = show.targetSlide();
        QVERIFY(!show.fireTrigger(11));
        show.followLink(deck::triggerLink(11));
        QVERIFY(show.isAnimating());
        QCOMPARE(show.targetSlide(), target);
        QCOMPARE(show.triggerRuns(), 0);
        show.finishTransition();
        QCOMPARE(show.currentSlide(), 1);
    }

    // An address of "jdtrigger:11" in a file is an ordinary address that goes nowhere (also as a mouse-over action); it never plays an object's effects.
    void anAddressThatReadsLikeATriggerIsNotOne()
    {
        auto d = editDeck(4);
        const ItemList &items = d->pages[0]->items;
        items[4]->hyperlink = QStringLiteral("jdtrigger:11");
        items[5]->hoverLink = QStringLiteral("jdtrigger:11");
        d->pages[0]->props[QStringLiteral("timingXml")] = animTriggerTiming(animSteps({}), {{11, animAppear(10)}});
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QCOMPARE(deck::triggerOfLink(QStringLiteral("x-jdtrigger:11")), 0);
        QTest::mouseMove(&show, items[5]->bounds().center().toPoint());
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, items[4]->bounds().center().toPoint());
        QCOMPARE(show.triggerRuns(), 0);
        QCOMPARE(show.currentSlide(), 0);
        QVERIFY(!boxShown(show, *items[2]));
        // (the real object still plays)
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, items[3]->bounds().center().toPoint());
        QCOMPARE(show.triggerRuns(), 1);
    }

    // A trigger object inside a group plays its effects at a click on it.
    void aTriggerObjectInAGroupPlaysItsEffects()
    {
        auto d = editDeck(3);
        ItemList &items = d->pages[0]->items;
        auto group = std::make_shared<GroupItem>();
        group->children.push_back(items[3]);
        group->syncRect();
        items.erase(items.begin() + 3);
        items.push_back(group);
        d->pages[0]->props[QStringLiteral("timingXml")] = animTriggerTiming(animSteps({}), {{11, animAppear(12)}});
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QVERIFY(!boxShown(show, *items[3]));   // (box 12, now at 3)
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, group->children[0]->bounds().center().toPoint());
        QCOMPARE(show.triggerRuns(), 1);
        QTRY_VERIFY(!show.isPlayingStep());
        QVERIFY(boxShown(show, *items[3]));
    }

    // A click in Presenter View's slide is a click of the show's steps: it never plays an object's effects, and the steps a trigger added are not taken for the main sequence's.
    void aClickInPresenterViewTakesTheMainSteps()
    {
        auto d = editDeck(3);
        d->pages[0]->props[QStringLiteral("timingXml")] = animTriggerTiming(animSteps({animAppear(10)}), {{11, animAppear(12)}});
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        PresenterView pv(&show);
        pv.resize(1000, 700);
        pv.show();
        auto *current = pv.findChild<QWidget *>(QStringLiteral("currentSlide"));
        QVERIFY(current);
        const QPoint on = QPoint(current->width() / 2, current->height() / 2);
        QTest::mouseClick(current, Qt::LeftButton, Qt::NoModifier, on);
        QCOMPARE(show.triggerRuns(), 0);
        QCOMPARE(show.step(), 1);
        QTRY_VERIFY(!show.isPlayingStep());
        // A trigger played in the show, then a click in Presenter View: the main sequence is over, so the click moves on.
        QVERIFY(show.fireTrigger(11));
        QTRY_VERIFY(!show.isPlayingStep());
        QCOMPARE(show.stepCount(), 2);
        QVERIFY(!pv.nextImage().isNull());
        QTest::mouseClick(current, Qt::LeftButton, Qt::NoModifier, on);
        QCOMPARE(show.currentSlide(), -1);   // (the only slide is over)
    }

    // The slide a trigger is playing on goes from the deck while the effect plays: the show goes on to the slide in its place, with nothing left running.
    void aSlideTakenAwayWhileATriggerPlaysLeavesNothingRunning()
    {
        auto d = twoSlideEditDeck(2);
        d->pages[0]->props[QStringLiteral("timingXml")] = animTriggerTiming(animSteps({}), {{11, animFadeIn(10, 600000)}});
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QVERIFY(show.fireTrigger(11));
        QVERIFY(show.isPlayingStep());
        QTRY_VERIFY(show.frameClockRunning());
        d->pages.erase(d->pages.begin());
        show.documentChanged();
        QCOMPARE(show.currentSlide(), 0);
        QVERIFY(!show.isPlayingStep());
        QVERIFY(!show.frameClockRunning());
        QCOMPARE(show.step(), 0);
        QVERIFY(!show.fireTrigger(11));   // (the slide that has it is gone)
        for (int i = 0; i < 30; ++i) QCoreApplication::processEvents();
        QCOMPARE(show.currentSlide(), 0);
    }

    // The show's menu lists the presentation's custom shows (a choice plays that show from its first slide) and has Help, which lists the keys; a presentation with no custom show has no such menu.
    void showMenuListsCustomShowsAndHelp()
    {
        auto d = showDeck(5);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        {
            std::unique_ptr<QMenu> menu(show.buildMenu(&show));
            QVERIFY(!menu->findChild<QMenu *>(QStringLiteral("customShowMenu")));
            QVERIFY(menu->findChild<QAction *>(QStringLiteral("menuHelp")));
        }
        deck::CustomShow a, b;
        a.id = 4;
        a.name = QStringLiteral("Tom & Jerry");
        a.slides = {d->pages[3]->id, d->pages[1]->id};
        b.id = 5;
        b.name = QStringLiteral("Closing");
        b.slides = {d->pages[4]->id};
        deck::setCustomShows(*d, {a, b});
        std::unique_ptr<QMenu> menu(show.buildMenu(&show));
        QMenu *custom = menu->findChild<QMenu *>(QStringLiteral("customShowMenu"));
        QVERIFY(custom);
        QCOMPARE(custom->actions().size(), 2);
        QCOMPARE(custom->actions().at(0)->text(), QStringLiteral("Tom && Jerry"));   // (an ampersand in a name is not a key)
        QCOMPARE(show.currentSlide(), 0);
        custom->actions().at(0)->trigger();
        QCOMPARE(show.currentSlide(), 3);
        show.next();
        QCOMPARE(show.currentSlide(), 1);
        custom->actions().at(1)->trigger();
        QCOMPARE(show.currentSlide(), 4);
        const QString help = SlideShow::helpText();
        for (const char *word : {"Page Down", "Home", "Enter", "Esc", "Shift+F10"}) QVERIFY2(help.contains(QLatin1String(word)), word);
    }

    // The show's menu opens Presenter View (once: a second choice brings the one that is open forward, and Reading View, which has none, can't); with it open the
    // Speaker Notes entry is not offered (Presenter View shows the notes, and a box would be on the audience's screen).
    void showMenuHasPresenterViewAndSpeakerNotes()
    {
        auto d = showDeck(3);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        std::unique_ptr<QMenu> menu(show.buildMenu(&show));
        QAction *pv = menu->findChild<QAction *>(QStringLiteral("menuPresenterView"));
        QAction *notes = menu->findChild<QAction *>(QStringLiteral("menuSpeakerNotes"));
        QVERIFY(pv && notes);
        QVERIFY(pv->isEnabled() && notes->isEnabled());
        QVERIFY(!show.presenterView());
        notes->trigger();
        QVERIFY(show.speakerNotesBox());
        pv->trigger();
        QVERIFY(show.presenterView());
        QVERIFY(show.presenterView()->isVisible());
        QVERIFY(!show.speakerNotesBox());   // Presenter View has the notes
        QCOMPARE(show.presenterView()->counterText(), QStringLiteral("Slide 1 of 3"));
        QPointer<PresenterView> first = show.presenterView();
        pv->trigger();
        QCOMPARE(show.presenterView(), first.data());
        int open = 0;
        for (QWidget *w : QApplication::allWidgets()) open += qobject_cast<PresenterView *>(w) != nullptr;
        QCOMPARE(open, 1);
        std::unique_ptr<QMenu> later(show.buildMenu(&show));
        QVERIFY(!later->findChild<QAction *>(QStringLiteral("menuSpeakerNotes"))->isEnabled());
        show.showSpeakerNotes();
        QVERIFY(!show.speakerNotesBox());
        // Reading View has no Presenter View to open.
        QWidget host;
        host.resize(960, 540);
        SlideShow inside(d.get(), &cache, 0);
        inside.embedIn(&host);
        std::unique_ptr<QMenu> insideMenu(inside.buildMenu(&inside));
        QVERIFY(!insideMenu->findChild<QAction *>(QStringLiteral("menuPresenterView"))->isEnabled());
    }

    // The notes box does not block the show: it is not modal, takes no focus, and the show's keys and clicks still move it on; it shows the notes of the slide that is on the screen
    // after every move (and says when there are none); Esc closes the box first, and then ends the show.
    void speakerNotesBoxFollowsTheShowAndTakesNoKeys()
    {
        auto d = showDeck(4);
        d->storyDoc(d->pages[1]->notesStoryId)->setPlainText(QString());
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QVERIFY(QTest::qWaitForWindowExposed(&show));
        show.activateWindow();
        std::unique_ptr<QMenu> menu(show.buildMenu(&show));
        menu->findChild<QAction *>(QStringLiteral("menuSpeakerNotes"))->trigger();
        QTextEdit *box = show.speakerNotesBox();
        QVERIFY(box);
        QVERIFY(box->isVisible());
        QCOMPARE(box->toPlainText(), QStringLiteral("Notes 1"));
        QVERIFY(!box->isModal());
        QVERIFY(!QApplication::activeModalWidget());
        QCOMPARE(box->focusPolicy(), Qt::NoFocus);
        QVERIFY(!box->hasFocus());
        // A key and a click of the show move it on, with the box up, and the box follows.
        QTest::keyClick(&show, Qt::Key_Right);
        QCOMPARE(show.currentSlide(), 1);
        QCOMPARE(box->toPlainText(), QStringLiteral("This slide has no notes."));
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(480, 270));
        QCOMPARE(show.currentSlide(), 2);
        QCOMPARE(box->toPlainText(), QStringLiteral("Notes 3"));
        show.previous();
        show.goTo(0);
        QCOMPARE(box->toPlainText(), QStringLiteral("Notes 1"));
        // Choosing the entry again keeps the one box.
        menu->findChild<QAction *>(QStringLiteral("menuSpeakerNotes"))->trigger();
        QCOMPARE(show.speakerNotesBox(), box);
        // Esc closes the box and not the show; the next one ends the show.
        QPointer<QTextEdit> gone = box;
        QTest::keyClick(&show, Qt::Key_Escape);
        QVERIFY(!show.speakerNotesBox());
        QVERIFY(show.isVisible());
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QVERIFY(gone.isNull());
        QTest::keyClick(&show, Qt::Key_Escape);
        QVERIFY(!show.isVisible());
    }

    // A long note is cut at the limit between whole characters: not inside a pair (an emoji), and not between a letter and its marks; and the box says it was cut.
    void speakerNotesAreCutBetweenWholeCharacters()
    {
        const int limit = SlideShow::kSpeakerNotesLimit;
        auto d = showDeck(3);
        const QString emoji = QString::fromUcs4(U"\U0001F600");
        d->storyDoc(d->pages[0]->notesStoryId)->setPlainText(QStringLiteral("a") + emoji.repeated(limit + 1000));
        d->storyDoc(d->pages[1]->notesStoryId)->setPlainText(QString(limit - 1, QLatin1Char('x')) + QStringLiteral("e") + QString(QChar(0x0301)).repeated(10) + QString(200, QLatin1Char('y')));
        d->storyDoc(d->pages[2]->notesStoryId)->setPlainText(QString(limit * 3, QLatin1Char('x')));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        show.showSpeakerNotes();
        QTextEdit *box = show.speakerNotesBox();
        QVERIFY(box);
        const QString cutNote = QStringLiteral("\n\nThese notes were cut short because they are very long.");
        for (int slide = 0; slide < 3; ++slide) {
            show.goTo(slide);
            const QString text = box->toPlainText();
            QVERIFY(text.endsWith(cutNote));
            const QString body = text.left(text.size() - cutNote.size());
            QVERIFY(body.size() <= limit);
            QVERIFY(body.size() > limit - 4);   // (nothing but the last whole character is lost)
            for (int i = 0; i < body.size(); ++i) {
                if (body[i].isHighSurrogate()) QVERIFY2(i + 1 < body.size() && body[i + 1].isLowSurrogate(), "a pair was split");
                if (body[i].isLowSurrogate()) QVERIFY2(i > 0 && body[i - 1].isHighSurrogate(), "a lone second half");
            }
            QVERIFY(!body.endsWith(QChar(0x0301)));
            if (slide == 1) QVERIFY(!body.endsWith(QLatin1Char('e')));   // the letter and its marks go together
        }
        QCOMPARE(box->toPlainText().size(), limit + cutNote.size());   // (the plain long note: cut exactly at the limit)
    }

    // Zoom from a menu that Presenter View opened (or any menu that was not a right click on the show) is around the middle, not the point of an earlier right click.
    void zoomFromPresenterViewIsAroundTheMiddle()
    {
        auto d = showDeck(2);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QVERIFY(QTest::qWaitForWindowExposed(&show));
        // A right click on the show at one corner, and its menu put away.
        QTest::mouseClick(&show, Qt::RightButton, Qt::NoModifier, QPoint(100, 60));
        if (QWidget *popup = QApplication::activePopupWidget()) QTest::keyClick(popup, Qt::Key_Escape);
        QVERIFY(!QApplication::activePopupWidget());
        show.showPresenterView();
        PresenterView *pv = show.presenterView();
        QVERIFY(pv);
        std::unique_ptr<QMenu> menu(show.buildMenu(pv));
        menu->findChild<QAction *>(QStringLiteral("menuZoom"))->trigger();
        QVERIFY(show.isZoomed());
        QCOMPARE(show.zoomOrigin(), QPointF(480, 270));
        show.endZoom();
        // A right click on the show is around its own point.
        QTest::mouseClick(&show, Qt::RightButton, Qt::NoModifier, QPoint(100, 60));
        QMenu *popup = qobject_cast<QMenu *>(QApplication::activePopupWidget());
        QVERIFY(popup);
        popup->findChild<QAction *>(QStringLiteral("menuZoom"))->trigger();
        QVERIFY(show.isZoomed());
        QCOMPARE(show.zoomOrigin(), QPointF(100, 60));
        if (QWidget *p = QApplication::activePopupWidget()) QTest::keyClick(p, Qt::Key_Escape);
    }

    // With a pen, a highlighter, an eraser, or the laser pointer chosen, Zoom is not offered (the ink would vanish and the pen go dead).
    void zoomIsNotOfferedWithAPen()
    {
        auto d = showDeck(2);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        for (SlideShow::Pointer p : {SlideShow::Pointer::Pen, SlideShow::Pointer::Highlighter, SlideShow::Pointer::Eraser, SlideShow::Pointer::Laser}) {
            show.setPointer(p);
            std::unique_ptr<QMenu> menu(show.buildMenu(&show));
            QVERIFY(!menu->findChild<QAction *>(QStringLiteral("menuZoom"))->isEnabled());
            show.zoomInto(QPointF(100, 100));
            QVERIFY(!show.isZoomed());
            QCOMPARE(show.pointer(), p);   // (the pen is still the pen)
        }
        show.setPointer(SlideShow::Pointer::Arrow);
        std::unique_ptr<QMenu> menu(show.buildMenu(&show));
        QVERIFY(menu->findChild<QAction *>(QStringLiteral("menuZoom"))->isEnabled());
    }

    // The larger picture has a pixel limit: a big window at a high pixel ratio draws it at a lower scale, and it still shows the slide twice as large.
    void zoomPictureHasAPixelCap()
    {
        const double cap = SlideShow::kZoomMaxPixels;
        auto pixels = [](const QSize &s, qreal r) { return double(s.width()) * SlideShow::kZoomFactor * r * (double(s.height()) * SlideShow::kZoomFactor * r); };
        QCOMPARE(SlideShow::zoomRatio(QSize(960, 540), 1.0), 1.0);
        QVERIFY(pixels(QSize(1920, 1080), 2.0) > cap);
        QVERIFY(pixels(QSize(1920, 1080), SlideShow::zoomRatio(QSize(1920, 1080), 2.0)) <= cap * 1.0001);
        QVERIFY(pixels(QSize(7680, 4320), SlideShow::zoomRatio(QSize(7680, 4320), 2.0)) <= cap * 1.0001);
        // A real picture of a large window.
        auto d = showDeck(1);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(3840, 2160);
        show.show();
        QVERIFY(QTest::qWaitForWindowExposed(&show));
        QVERIFY(show.size().width() >= 3000);
        show.zoomInto(QPointF(100, 100));
        QVERIFY(show.isZoomed());
        const QSize px = show.zoomImageSize();
        QVERIFY(double(px.width()) * px.height() <= cap * 1.01);
        QVERIFY(std::abs(show.zoomImage().deviceIndependentSize().width() - show.width() * 2) <= 2);
    }

    // A step of the slide's effects that is about to play ends the zoom too (the larger picture would not show it), and the slide stays where it is.
    void zoomEndsWhenAStepPlays()
    {
        auto d = editDeck(2);
        d->pages[0]->props[QStringLiteral("timingXml")] = animTriggerTiming(animSteps({animAppear(10)}), {{11, animAppear(12)}});
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QVERIFY(QTest::qWaitForWindowExposed(&show));
        show.zoomInto(QPointF(100, 100));
        QVERIFY(show.isZoomed());
        show.next();
        QVERIFY(!show.isZoomed());
        QCOMPARE(show.currentSlide(), 0);
        QCOMPARE(show.step(), 1);
        QTRY_VERIFY(!show.isPlayingStep());
    }

    // Zoom into the slide: the picture is twice as large, a drag moves it (and stops at the slide's edges), a click does not move the show on, and Esc, the next slide, a
    // second choice of the menu entry, or a new window size end the zoom.
    void zoomIntoTheSlideDragsAndEnds()
    {
        auto d = showDeck(3);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QVERIFY(QTest::qWaitForWindowExposed(&show));
        QVERIFY(!show.isZoomed());
        std::unique_ptr<QMenu> menu(show.buildMenu(&show));
        QAction *zoom = menu->findChild<QAction *>(QStringLiteral("menuZoom"));
        QVERIFY(zoom && zoom->isCheckable() && !zoom->isChecked());
        show.zoomInto(QPointF(480, 270));
        QVERIFY(show.isZoomed());
        QCOMPARE(show.zoomOrigin(), QPointF(480, 270));   // the middle stays where it was
        QVERIFY(!show.grab().toImage().isNull());
        // A drag moves the picture the other way, a click moves nothing on, and the drag stops at the edge.
        QTest::mousePress(&show, Qt::LeftButton, Qt::NoModifier, QPoint(500, 300));
        QTest::mouseMove(&show, QPoint(460, 280));
        QCOMPARE(show.zoomOrigin(), QPointF(520, 290));
        QTest::mouseMove(&show, QPoint(-5000, -5000));
        QTest::mouseRelease(&show, Qt::LeftButton, Qt::NoModifier, QPoint(-5000, -5000));
        QCOMPARE(show.zoomOrigin(), QPointF(960, 540));
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(300, 300));
        QCOMPARE(show.currentSlide(), 0);
        QVERIFY(show.isZoomed());
        std::unique_ptr<QMenu> again(show.buildMenu(&show));
        QAction *off = again->findChild<QAction *>(QStringLiteral("menuZoom"));
        QVERIFY(off->isChecked());
        off->trigger();   // the entry again puts the slide back
        QVERIFY(!show.isZoomed());
        // Esc leaves the zoom and nothing else.
        show.zoomInto(QPointF(100, 100));
        QTest::keyClick(&show, Qt::Key_Escape);
        QVERIFY(!show.isZoomed());
        QVERIFY(show.isVisible());
        // The next slide, and a new size, end it too.
        show.zoomInto(QPointF(100, 100));
        show.next();
        QVERIFY(!show.isZoomed());
        QCOMPARE(show.currentSlide(), 1);
        show.zoomInto(QPointF(100, 100));
        show.resize(800, 450);
        QVERIFY(!show.isZoomed());
        // No zoom on a black screen.
        show.toggleBlank(Qt::black);
        show.zoomInto(QPointF(100, 100));
        QVERIFY(!show.isZoomed());
    }

    // Lets the event loop run a counted number of turns, with the deletions that are waiting.
    static void settle(int turns = 50)
    {
        for (int i = 0; i < turns; ++i) {
            QCoreApplication::processEvents();
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        }
    }

    static int liveShows()
    {
        int n = 0;
        for (QWidget *w : QApplication::allWidgets())
            if (qobject_cast<SlideShow *>(w)) ++n;
        return n;
    }

    // The show ends while a box it put up is open (the Help box, here, in a show of its own and in Reading View): the box goes with the show and nothing is freed twice. The show is ended
    // from a timer that runs while the box is up (inside the box's own loop, when it has one).
    void endingTheShowWhileItsHelpBoxIsOpenIsSafe_data()
    {
        QTest::addColumn<bool>("embedded");
        QTest::newRow("in a show of its own") << false;
        QTest::newRow("in Reading View") << true;
    }

    void endingTheShowWhileItsHelpBoxIsOpenIsSafe()
    {
        QFETCH(bool, embedded);
        DeckWindow w;
        w.resize(1100, 700);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.newSlide();
        if (embedded) w.act("view.reading")->trigger();
        else w.startShow(0);
        QPointer<SlideShow> show = embedded ? w.readingShow() : w.slideShow();
        QVERIFY(show);
        QMenu *menu = show->buildMenu(&w);
        QAction *help = menu->findChild<QAction *>(QStringLiteral("menuHelp"));
        QVERIFY(help);
        bool boxWasUp = false;
        QTimer::singleShot(0, show.data(), [&] {
            boxWasUp = qobject_cast<QMessageBox *>(QApplication::activeModalWidget()) != nullptr;
            show->close();
            settle(20);
        });
        help->trigger();
        for (int i = 0; i < 100 && (!boxWasUp || show); ++i) settle(1);
        QVERIFY(boxWasUp);
        QVERIFY(show.isNull());
        settle();
        QVERIFY(!QApplication::activeModalWidget());
        QVERIFY(embedded ? !w.inReadingView() : !w.slideShow());
        delete menu;
    }

    // Every way to start a show first ends Reading View (as Esc does, with its question about the ink), so that there are never two shows on one presentation.
    void everyRouteToAShowEndsReadingViewFirst()
    {
        DeckWindow w;
        w.resize(1100, 700);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.newSlide();
        w.newSlide();
        deck::CustomShow cs;
        cs.id = 7;
        cs.name = QStringLiteral("Two");
        cs.slides = {w.editor()->doc()->pages[1]->id};
        deck::setCustomShows(*w.editor()->doc(), {cs});
        int asked = 0;
        bool showUpWhenAsked = false;
        w.setShowQuestion([&](const QString &) {
            ++asked;
            showUpWhenAsked = w.slideShow() != nullptr;
            return false;
        });
        QToolButton *statusButton = nullptr;
        for (QToolButton *b : w.statusBar()->findChildren<QToolButton *>())
            if (b->defaultAction() == w.act("view.slideShow")) statusButton = b;
        QVERIFY(statusButton);
        struct Route {
            const char *name;
            std::function<void()> go;
        };
        const QVector<Route> routes{
            {"status bar button", [&] { statusButton->click(); }},
            {"From Beginning (F5)", [&] { w.act("show.fromStart")->trigger(); }},
            {"From Current Slide (Shift+F5)", [&] { w.act("show.fromCurrent")->trigger(); }},
            {"Presenter View", [&] { w.act("show.presenterView")->trigger(); }},
            {"Rehearse Timings", [&] { w.rehearseTimings(); }},
            {"a custom show", [&] { w.startCustomShow(7); }},
        };
        bool first = true;
        for (const Route &route : routes) {
            w.act("view.reading")->trigger();
            QVERIFY2(w.inReadingView(), route.name);
            if (first) {
                SlideShow *reading = w.readingShow();
                reading->setPointer(SlideShow::Pointer::Pen);
                reading->pressAt(QPointF(100, 100));
                reading->moveTo(QPointF(200, 120));
                reading->releasePointer();
            }
            route.go();
            QVERIFY2(!w.inReadingView(), route.name);
            QVERIFY2(w.slideShow(), route.name);
            settle();
            QVERIFY2(liveShows() == 1, route.name);
            if (first) {
                QCOMPARE(asked, 1);
                QVERIFY(!showUpWhenAsked);   // (the question comes before the new show starts)
                first = false;
            }
            w.slideShow()->close();
            settle();
            QVERIFY2(liveShows() == 0, route.name);
        }
        w.setShowQuestion(nullptr);
    }

    // Closing the window with Reading View up ends it too, and leaves nothing behind.
    void closingTheWindowInReadingViewLeavesNoShow()
    {
        QPointer<SlideShow> show;
        {
            auto *w = new DeckWindow;
            w->setAttribute(Qt::WA_DeleteOnClose, false);
            w->resize(1100, 700);
            w->show();
            QVERIFY(QTest::qWaitForWindowExposed(w));
            w->act("view.reading")->trigger();
            show = w->readingShow();
            QVERIFY(show);
            QVERIFY(w->close());
            QVERIFY(!w->inReadingView());
            delete w;
        }
        settle();
        QVERIFY(show.isNull());
        QCOMPARE(liveShows(), 0);
    }

    // Reading View is for reading: the commands that change the slides are refused while it is up (named here, and Undo and Redo among them), the editor takes back a change that gets past them,
    // and a drag onto the page is not accepted. The views, Save, Print, Open, New, and Close stay.
    void readingViewRefusesChangesToTheSlides()
    {
        DeckWindow w;
        w.resize(1100, 700);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.newSlide();
        w.newSlide();
        w.editor()->setCurrentPage(0);
        w.act("edit.selectAll")->trigger();
        QApplication::clipboard()->setText(QStringLiteral("pasted"));
        w.act("view.reading")->trigger();
        QVERIFY(w.inReadingView());
        const QJsonObject before = w.editor()->doc()->toJson();
        int refused = 0;
        for (const char *id : {"slide.new", "edit.paste", "edit.cut", "edit.delete", "edit.undo", "edit.redo", "edit.pasteText"}) {
            QAction *a = w.act(QString::fromLatin1(id));
            QVERIFY2(a, id);
            w.statusBar()->clearMessage();
            a->trigger();
            QVERIFY2(w.inReadingView(), id);
            QVERIFY2(w.editor()->doc()->toJson() == before, id);
            QCOMPARE(int(w.editor()->doc()->pages.size()), 3);
            if (!a->isEnabled()) continue;   // (a command that is off asks for nothing)
            QVERIFY2(w.statusBar()->currentMessage().contains(QLatin1String("Reading View is for reading")), id);
            ++refused;
        }
        QVERIFY(refused >= 4);
        // A change that gets past the commands is taken back, and a drop is not accepted.
        w.editor()->beginChange(QStringLiteral("Test"));
        w.editor()->doc()->pages.erase(w.editor()->doc()->pages.begin() + 2);
        w.editor()->endChange();
        QCOMPARE(int(w.editor()->doc()->pages.size()), 3);
        QMimeData mime;
        mime.setText(QStringLiteral("dropped"));
        QDragEnterEvent enter(QPoint(50, 50), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(w.canvas(), &enter);
        QVERIFY(!enter.isAccepted());
        for (const char *id : {"file.save", "file.saveAs", "file.saveCopy", "file.print", "file.open", "file.new", "file.close", "help.show", "view.normal", "view.sorter"})
            QVERIFY2(AppWindow::commandAllowedReadOnly(QString::fromLatin1(id)), id);
        // Another view ends Reading View, and the slides can be changed again.
        w.act("view.sorter")->trigger();
        QVERIFY(!w.inReadingView());
        QVERIFY(!w.editor()->readOnly());
        w.act("slide.new")->trigger();
        QCOMPARE(int(w.editor()->doc()->pages.size()), 4);
    }

    // Leaving Reading View gives the keyboard back to what had it when the view began (once the show is deleted).
    void leavingReadingViewGivesTheKeyboardBack_data()
    {
        QTest::addColumn<QString>("view");
        QTest::newRow("the slide") << QStringLiteral("view.normal");
        QTest::newRow("the sorter") << QStringLiteral("view.sorter");
        QTest::newRow("the outline") << QStringLiteral("view.outline");
        QTest::newRow("the notes") << QStringLiteral("notes");
    }

    void leavingReadingViewGivesTheKeyboardBack()
    {
        QFETCH(QString, view);
        DeckWindow w;
        w.resize(1100, 700);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.activateWindow();
        w.newSlide();
        if (view == QLatin1String("notes")) {
            w.act("view.notes")->setChecked(true);
            QTRY_VERIFY(w.notesPane()->isVisible());
            w.notesPane()->setFocus(Qt::OtherFocusReason);
        } else {
            w.act(view)->trigger();
            if (view == QLatin1String("view.normal")) w.canvas()->setFocus(Qt::OtherFocusReason);
        }
        QWidget *had = w.focusWidget();
        QVERIFY(had);
        w.act("view.reading")->trigger();
        QVERIFY(w.inReadingView());
        QPointer<SlideShow> show = w.readingShow();
        QVERIFY(w.focusWidget() != had);
        QTest::keyClick(show, Qt::Key_Escape);
        QVERIFY(!w.inReadingView());
        settle();
        QVERIFY(show.isNull());
        QCOMPARE(w.focusWidget(), had);
    }

    // Tab reaches the status bar's controls from a slide with no link (and goes on round to the others), and the controls have names a screen reader can say.
    void readingViewControlsAreReachableByTabAndNamed()
    {
        DeckWindow w;
        w.resize(1100, 700);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.activateWindow();
        w.newSlide();
        w.act("view.reading")->trigger();
        QVERIFY(w.inReadingView());
        SlideShow *show = w.readingShow();
        show->setFocus(Qt::OtherFocusReason);
        QCOMPARE(w.focusWidget(), static_cast<QWidget *>(show));
        QSet<QString> reached;
        for (int i = 0; i < 12 && reached.size() < 3; ++i) {
            QTest::keyClick(w.focusWidget(), Qt::Key_Tab);
            if (auto *b = qobject_cast<QToolButton *>(w.focusWidget())) reached << b->objectName();
        }
        QVERIFY(reached.contains(QStringLiteral("readingPrevious")));
        QVERIFY(reached.contains(QStringLiteral("readingNext")));
        QVERIFY(reached.contains(QStringLiteral("readingMenu")));
        QCOMPARE(w.findChild<QToolButton *>(QStringLiteral("readingMenu"))->text(), QStringLiteral("Show Menu"));
        for (const char *name : {"readingPrevious", "readingNext", "readingMenu"}) {
            auto *b = w.findChild<QToolButton *>(QString::fromLatin1(name));
            QVERIFY2(!b->accessibleName().isEmpty(), name);
            QCOMPARE(b->accessibleName(), b->text());
        }
    }

    // The keys the help lists: Esc's two steps are told in one line, and the eraser (Ctrl and E) and erasing all the ink (E alone) are in lines of their own.
    void showHelpKeepsTheEraserAndEraseAllApart()
    {
        const QStringList lines = SlideShow::helpText().split(QLatin1Char('\n'));
        const QString ctrlE = QKeySequence(Qt::ControlModifier | Qt::Key_E).toString(QKeySequence::NativeText);
        int eraser = 0, eraseAll = 0, esc = 0;
        for (const QString &line : lines) {
            if (line.contains(ctrlE)) {
                ++eraser;
                QVERIFY(!line.contains(QLatin1String("all ink")));
            }
            if (line.contains(QLatin1String("all ink"))) {
                ++eraseAll;
                QVERIFY(!line.contains(ctrlE));
                QVERIFY(line.contains(QLatin1String("without Ctrl")));
            }
            if (line == QLatin1String("Esc: put the pointer back to an arrow; press again to end the show.")) ++esc;
        }
        QCOMPARE(eraser, 1);
        QCOMPARE(eraseAll, 1);
        QCOMPARE(esc, 1);
    }

    // Pictures of Reading View and the show's menu, when JP_SHOT_DIR names a folder.
    void readingViewPictures()
    {
        if (qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) QSKIP("JP_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        const QString dir = qEnvironmentVariable("JP_SHOT_DIR");
        DeckWindow w;
        w.resize(1200, 760);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        w.newSlide();
        w.newSlide();
        deck::CustomShow cs;
        cs.id = 3;
        cs.name = QStringLiteral("Short version");
        cs.slides = {w.editor()->doc()->pages[1]->id};
        deck::setCustomShows(*w.editor()->doc(), {cs});
        auto *title = static_cast<TextItem *>(w.editor()->doc()->pages[0]->items[0].get());
        w.editor()->doc()->storyDoc(title->storyId)->setPlainText(QStringLiteral("Quarterly review"));
        w.editor()->setCurrentPage(0);
        w.act("view.reading")->trigger();
        QVERIFY(w.inReadingView());
        QTest::keyClick(w.readingShow(), Qt::Key_Right);
        QTest::keyClick(w.readingShow(), Qt::Key_Left);
        QVERIFY(w.grab().save(dir + QStringLiteral("/reading-view.png")));
        auto *menuButton = w.findChild<QToolButton *>(QStringLiteral("readingMenu"));
        menuButton->click();
        QMenu *menu = nullptr;
        for (int i = 0; i < 100 && !menu; ++i) {
            for (QWidget *top : QApplication::topLevelWidgets())
                if (top->objectName() == QLatin1String("showMenu") && top->isVisible()) menu = qobject_cast<QMenu *>(top);
            if (!menu) QCoreApplication::processEvents();
        }
        QVERIFY(menu);
        QVERIFY(menu->grab().save(dir + QStringLiteral("/show-menu.png")));
        menu->close();
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
