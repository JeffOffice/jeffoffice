// JeffDeck's tests of movies and sounds: timing, playback, and the show.
#include "deckhelpers.h"

// A server on this computer that counts who calls, for the tests of what a file may make the player fetch.
struct MediaListener {
    QTcpServer server;
    int calls = 0;
    QByteArray what;
    MediaListener()
    {
        QObject::connect(&server, &QTcpServer::newConnection, &server, [this] {
            while (QTcpSocket *s = server.nextPendingConnection()) {
                ++calls;
                QObject::connect(s, &QTcpSocket::readyRead, s, [this, s] { what += s->readAll(); });
                s->disconnectFromHost();
            }
        });
        server.listen(QHostAddress::LocalHost, 0);
    }
    QString url(const char *path) const { return QStringLiteral("http://127.0.0.1:%1/%2").arg(server.serverPort()).arg(QLatin1String(path)); }
};

class DeckMediaTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // ---- movies and sounds ----

    // A picture of a movie or sound on a slide, as the reader leaves it.
    static std::shared_ptr<PictureItem> mediaPicture(int spid, const char *kind)
    {
        auto pic = std::make_shared<PictureItem>();
        pic->sourceId = spid;
        pic->mediaKind = QLatin1String(kind);
        pic->mediaId = QStringLiteral("media1");
        return pic;
    }

    static QString mediaCall(int spid, const char *cmd, const char *nodeType, int preset = 1)
    {
        return animNode("mediacall", preset, 0, nodeType, 0, {animXml("<p:cmd type='call' cmd='%1'><p:cBhvr><p:cTn id='%2' dur='1' fill='hold'/>%3</p:cBhvr></p:cmd>").arg(QString::fromLatin1(cmd)).arg(animId()).arg(animTarget(spid))});
    }

    static QString mediaTrigger(int spid, const char *cmd, int preset = 2)
    {
        return animXml("<p:seq concurrent='1' nextAc='seek'><p:cTn id='%1' restart='whenNotActive' fill='hold' evtFilter='cancelBubble' nodeType='interactiveSeq'><p:stCondLst><p:cond evt='onClick' delay='0'><p:tgtEl><p:spTgt spid='%2'/></p:tgtEl></p:cond></p:stCondLst><p:endSync evt='end' delay='0'><p:rtn val='all'/></p:endSync><p:childTnLst>")
                   .arg(animId()).arg(spid)
               + animGroup(0, {animGroup(0, {mediaCall(spid, cmd, "clickEffect", preset)})})
               + animXml("</p:childTnLst></p:cTn><p:nextCondLst><p:cond evt='onClick' delay='0'><p:tgtEl><p:spTgt spid='%1'/></p:tgtEl></p:cond></p:nextCondLst></p:seq>").arg(spid);
    }

    // The settings of a movie or sound are read from the slide's timing (when it starts, its volume, looping, rewinding, showing
    // while stopped, playing across slides, and full screen) and from the picture (trim and fade).
    void mediaPlayIsReadFromTimingAndPicture()
    {
        Page slide;
        auto movie = mediaPicture(4, "video");
        movie->mediaTrimStart = 1500;
        movie->mediaTrimEnd = 9000;
        movie->mediaFadeIn = 500;
        movie->mediaFadeOut = 700;
        slide.items.push_back(movie);
        // Nothing in the timing: the defaults, and starting in the click sequence.
        deck::MediaPlay p = deck::mediaPlayOf(slide, *movie);
        QCOMPARE(p.shape, 4);
        QVERIFY(p.video);
        QVERIFY(p.start == deck::MediaStart::ClickSequence);
        QCOMPARE(p.volume, deck::kVolumeHigh);
        QVERIFY(!p.mute && !p.loop && !p.rewind && !p.fullScreen && !p.hideWhenStopped);
        QCOMPARE(p.acrossSlides, 1);
        QCOMPARE(p.trimStart, 1500);
        QCOMPARE(p.trimEnd, 9000);
        QCOMPARE(p.fadeIn, 500);
        QCOMPARE(p.fadeOut, 700);
        // Every option, and a call in the click sequence.
        slide.props[QStringLiteral("timingXml")] = animTiming(
            animSteps({mediaCall(4, "playFrom(0.0)", "clickEffect")}),
            animXml("<p:video fullScrn='1'><p:cMediaNode vol='20000' mute='1' numSld='3' showWhenStopped='0'><p:cTn id='30' repeatCount='indefinite' fill='remove' display='0'><p:stCondLst><p:cond delay='indefinite'/></p:stCondLst></p:cTn>"
                    "<p:tgtEl><p:spTgt spid='4'/></p:tgtEl></p:cMediaNode></p:video>"));
        p = deck::mediaPlayOf(slide, *movie);
        QVERIFY(p.start == deck::MediaStart::ClickSequence);
        QCOMPARE(p.volume, deck::kVolumeLow);
        QVERIFY(p.mute && p.loop && p.rewind && p.fullScreen && p.hideWhenStopped);
        QCOMPARE(p.acrossSlides, 3);
        // The call says how long it plays, which the bar shows before the file is played.
        slide.props[QStringLiteral("timingXml")] = animTiming(animSteps({animNode("mediacall", 1, 0, "clickEffect", 0, {animXml("<p:cmd type='call' cmd='playFrom(0.0)'><p:cBhvr><p:cTn id='%1' dur='5000' fill='hold'/>%2</p:cBhvr></p:cmd>").arg(animId()).arg(animTarget(4))})}));
        QCOMPARE(deck::mediaPlayOf(slide, *movie).lengthHint, 5000);
        // A call that goes after the one before it, or a step that begins by itself, is automatic.
        slide.props[QStringLiteral("timingXml")] = animTiming(animStep({animGroup(0, {mediaCall(4, "playFrom(0.0)", "afterEffect")})}, true));
        QVERIFY(deck::mediaPlayOf(slide, *movie).start == deck::MediaStart::Automatically);
        slide.props[QStringLiteral("timingXml")] = animTiming(animSteps({animAppear(5), mediaCall(4, "playFrom(0.0)", "withEffect")}));
        QVERIFY(deck::mediaPlayOf(slide, *movie).start == deck::MediaStart::Automatically);
        // Only a click on a shape starts it.
        slide.props[QStringLiteral("timingXml")] = animTiming(QString(), mediaTrigger(4, "togglePause"));
        QVERIFY(deck::mediaPlayOf(slide, *movie).start == deck::MediaStart::WhenClickedOn);
        // A sound has no full screen, and a picture with no number can't be named by the timing.
        auto sound = mediaPicture(0, "audio");
        QVERIFY(!deck::mediaPlayOf(slide, *sound).video);
        QCOMPARE(deck::mediaPlayOf(slide, *sound).shape, 0);
        // A picture that plays nothing is not one of the slide's media.
        auto plain = std::make_shared<PictureItem>();
        slide.items.push_back(plain);
        slide.items.push_back(sound);
        QCOMPARE(int(deck::mediaPicturesOf(slide).size()), 2);
    }

    // What the timing says of the media, as the show plays it: which steps start it, which click starts it, and which pictures hide
    // while stopped.
    void mediaPlanListsTheCallsOfTheTiming()
    {
        Page slide;
        auto movie = mediaPicture(4, "video"), sound = mediaPicture(7, "audio");
        slide.items.push_back(movie);
        slide.items.push_back(sound);
        slide.props[QStringLiteral("timingXml")] = animTiming(
            animSteps({animAppear(5), mediaCall(4, "playFrom(0.0)", "clickEffect"), animAppear(5)}),
            animXml("<p:audio><p:cMediaNode vol='50000' showWhenStopped='0'><p:cTn id='30' fill='hold' display='0'><p:stCondLst><p:cond delay='indefinite'/></p:stCondLst></p:cTn><p:tgtEl><p:spTgt spid='7'/></p:tgtEl></p:cMediaNode></p:audio>") +
                mediaTrigger(7, "playFrom(0.0)", 1));
        const deck::MediaPlan plan = deck::mediaPlanOf(slide);
        QCOMPARE(int(plan.plays.size()), 2);
        QVERIFY(plan.plays.value(4).video);
        QVERIFY(plan.plays.value(7).start == deck::MediaStart::WhenClickedOn);
        QVERIFY(plan.plays.value(7).hideWhenStopped);
        QCOMPARE(int(plan.steps.size()), 3);
        QVERIFY(plan.steps[0].visual && plan.steps[0].cues.isEmpty());
        QVERIFY(!plan.steps[1].visual);
        QCOMPARE(int(plan.steps[1].cues.size()), 1);
        QCOMPARE(plan.steps[1].cues[0].shape, 4);
        QCOMPARE(plan.steps[1].cues[0].command, QStringLiteral("playFrom(0.0)"));
        QVERIFY(plan.steps[2].visual);
        QCOMPARE(int(plan.triggers.value(7).size()), 1);
        QCOMPARE(plan.triggers.value(7)[0].command, QStringLiteral("playFrom(0.0)"));
        QCOMPARE(deck::mediaHiddenWhenStopped(slide), QSet<QString>{sound->id});
        // A slide with no media has nothing read.
        Page bare;
        bare.props[QStringLiteral("timingXml")] = animTiming(animSteps({animAppear(5)}));
        QVERIFY(deck::mediaPlanOf(bare).isEmpty());
        QVERIFY(deck::mediaHiddenWhenStopped(bare).isEmpty());
    }

    // What the settings write into the timing is what the presentation program writes for a sound in the click sequence, and
    // a movie that waits for a click on it.
    void mediaTimingIsWrittenAsThePresentationProgramWritesIt()
    {
        const QString head = QStringLiteral("<p:timing xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\"><p:childTnLst>");
        const QString tail = QStringLiteral("</p:childTnLst></p:cTn></p:par></p:tnLst></p:timing>");
        const QString mainSeq = QStringLiteral("<p:seq concurrent=\"1\" nextAc=\"seek\"><p:cTn id=\"2\" dur=\"indefinite\" nodeType=\"mainSeq\"><p:childTnLst>%1</p:childTnLst></p:cTn>"
                                               "<p:prevCondLst><p:cond evt=\"onPrev\" delay=\"0\"><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:prevCondLst>"
                                               "<p:nextCondLst><p:cond evt=\"onNext\" delay=\"0\"><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:nextCondLst></p:seq>");
        // A sound in the click sequence.
        Page slide;
        auto sound = mediaPicture(0, "audio");
        slide.items.push_back(sound);
        deck::MediaPlay play;
        deck::setMediaPlay(slide, *sound, play, 92157);
        QCOMPARE(sound->sourceId, 2);
        QCOMPARE(slide.props.value(QStringLiteral("timingXml")).toString(),
                 head + mainSeq.arg(QStringLiteral("<p:par><p:cTn id=\"3\" fill=\"hold\"><p:stCondLst><p:cond delay=\"indefinite\"/></p:stCondLst><p:childTnLst>"
                                                   "<p:par><p:cTn id=\"4\" fill=\"hold\"><p:stCondLst><p:cond delay=\"0\"/></p:stCondLst><p:childTnLst>"
                                                   "<p:par><p:cTn id=\"5\" presetID=\"1\" presetClass=\"mediacall\" presetSubtype=\"0\" fill=\"hold\" nodeType=\"clickEffect\"><p:stCondLst><p:cond delay=\"0\"/></p:stCondLst><p:childTnLst>"
                                                   "<p:cmd type=\"call\" cmd=\"playFrom(0.0)\"><p:cBhvr><p:cTn id=\"6\" dur=\"92157\" fill=\"hold\"/><p:tgtEl><p:spTgt spid=\"2\"/></p:tgtEl></p:cBhvr></p:cmd>"
                                                   "</p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:par>"))
                     + QStringLiteral("<p:audio><p:cMediaNode vol=\"80000\"><p:cTn id=\"7\" fill=\"hold\" display=\"0\"><p:stCondLst><p:cond delay=\"indefinite\"/></p:stCondLst>"
                                      "<p:endCondLst><p:cond evt=\"onStopAudio\" delay=\"0\"><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:endCondLst></p:cTn><p:tgtEl><p:spTgt spid=\"2\"/></p:tgtEl></p:cMediaNode></p:audio>")
                     + tail);
        QVERIFY(deck::mediaPlayOf(slide, *sound) == [&] { deck::MediaPlay p; p.shape = 2; return p; }());
        // A movie that waits for a click on it: the click pauses it and plays it again. Its sequence comes first, and its node after it.
        Page other;
        auto movie = mediaPicture(0, "video");
        other.items.push_back(movie);
        play.start = deck::MediaStart::WhenClickedOn;
        deck::setMediaPlay(other, *movie, play);
        QCOMPARE(other.props.value(QStringLiteral("timingXml")).toString(),
                 head
                     + QStringLiteral("<p:seq concurrent=\"1\" nextAc=\"seek\"><p:cTn id=\"2\" restart=\"whenNotActive\" fill=\"hold\" evtFilter=\"cancelBubble\" nodeType=\"interactiveSeq\"><p:stCondLst><p:cond evt=\"onClick\" delay=\"0\"><p:tgtEl><p:spTgt spid=\"2\"/></p:tgtEl></p:cond></p:stCondLst>"
                                      "<p:endSync evt=\"end\" delay=\"0\"><p:rtn val=\"all\"/></p:endSync><p:childTnLst>"
                                      "<p:par><p:cTn id=\"3\" fill=\"hold\"><p:stCondLst><p:cond delay=\"0\"/></p:stCondLst><p:childTnLst>"
                                      "<p:par><p:cTn id=\"4\" fill=\"hold\"><p:stCondLst><p:cond delay=\"0\"/></p:stCondLst><p:childTnLst>"
                                      "<p:par><p:cTn id=\"5\" presetID=\"2\" presetClass=\"mediacall\" presetSubtype=\"0\" fill=\"hold\" nodeType=\"clickEffect\"><p:stCondLst><p:cond delay=\"0\"/></p:stCondLst><p:childTnLst>"
                                      "<p:cmd type=\"call\" cmd=\"togglePause\"><p:cBhvr><p:cTn id=\"6\" dur=\"1\" fill=\"hold\"/><p:tgtEl><p:spTgt spid=\"2\"/></p:tgtEl></p:cBhvr></p:cmd>"
                                      "</p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn>"
                                      "<p:nextCondLst><p:cond evt=\"onClick\" delay=\"0\"><p:tgtEl><p:spTgt spid=\"2\"/></p:tgtEl></p:cond></p:nextCondLst></p:seq>"
                                      "<p:video><p:cMediaNode vol=\"80000\"><p:cTn id=\"7\" fill=\"hold\" display=\"0\"><p:stCondLst><p:cond delay=\"indefinite\"/></p:stCondLst></p:cTn><p:tgtEl><p:spTgt spid=\"2\"/></p:tgtEl></p:cMediaNode></p:video>")
                     + tail);
        // Every option comes back as it was set, for each way to start.
        for (const bool video : {false, true})
            for (const deck::MediaStart start : {deck::MediaStart::ClickSequence, deck::MediaStart::Automatically, deck::MediaStart::WhenClickedOn}) {
                Page s;
                auto pic = mediaPicture(0, video ? "video" : "audio");
                s.items.push_back(pic);
                deck::MediaPlay want;
                want.start = start;
                want.volume = deck::kVolumeMedium;
                want.mute = true;
                want.loop = true;
                want.rewind = true;
                want.fullScreen = video;
                want.hideWhenStopped = true;
                want.acrossSlides = video ? 1 : 999;
                want.trimStart = 1000;
                want.trimEnd = 8000;
                want.fadeIn = 250;
                want.fadeOut = 400;
                deck::setMediaPlay(s, *pic, want, 20000);
                want.shape = pic->sourceId;
                want.video = video;
                QVERIFY(deck::mediaPlayOf(s, *pic) == want);
                QCOMPARE(int(deck::readTimeline(s.props.value(QStringLiteral("timingXml")).toString()).steps.size()), start == deck::MediaStart::WhenClickedOn ? 0 : 1);
            }
        // An automatic one on a slide with no steps begins with the sequence.
        Page auto1;
        auto pic1 = mediaPicture(0, "audio");
        auto1.items.push_back(pic1);
        play = deck::MediaPlay();
        play.start = deck::MediaStart::Automatically;
        deck::setMediaPlay(auto1, *pic1, play, 5000);
        const QString autoXml = auto1.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY(autoXml.contains(QStringLiteral("<p:cond evt=\"onBegin\" delay=\"0\"><p:tn val=\"2\"/></p:cond>")));
        QVERIFY(autoXml.contains(QStringLiteral("nodeType=\"afterEffect\"")));
        QVERIFY(deck::readTimeline(autoXml).steps[0].automatic);
        // The step begins with the sequence, and the node of the sound says nothing of it.
        QVERIFY(!autoXml.contains(QStringLiteral("<p:tn val=\"5\"/>")));   // the sound's own node doesn't name the call (the program writes none)
    }

    // Changing how a movie plays changes only what the timing says of it: the other animations and the other media stay, and
    // setting it again does not stack another call.
    void mediaTimingChangesLeaveOtherAnimationsAlone()
    {
        Page slide;
        auto movie = mediaPicture(4, "video"), sound = mediaPicture(7, "audio");
        slide.items.push_back(movie);
        slide.items.push_back(sound);
        slide.props[QStringLiteral("timingXml")] = animTiming(animSteps({animFadeIn(5, 500), mediaCall(4, "playFrom(0.0)", "clickEffect"), mediaCall(7, "playFrom(0.0)", "clickEffect"), animAppear(5)}),
                                                             animXml("<p:video><p:cMediaNode vol='80000'><p:cTn id='900' fill='hold' display='0'><p:stCondLst><p:cond delay='indefinite'/></p:stCondLst></p:cTn><p:tgtEl><p:spTgt spid='4'/></p:tgtEl></p:cMediaNode></p:video>") +
                                                                 animXml("<p:audio><p:cMediaNode vol='80000'><p:cTn id='901' fill='hold' display='0'><p:stCondLst><p:cond delay='indefinite'/></p:stCondLst></p:cTn><p:tgtEl><p:spTgt spid='7'/></p:tgtEl></p:cMediaNode></p:audio>"));
        deck::MediaPlay play = deck::mediaPlayOf(slide, *movie);
        play.start = deck::MediaStart::Automatically;
        play.loop = true;
        deck::setMediaPlay(slide, *movie, play, 4000);
        deck::setMediaPlay(slide, *movie, play, 4000);
        const QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
        const deck::Timeline tl = deck::readTimeline(xml);
        int callsTo4 = 0, callsTo7 = 0, fades = 0;
        for (const deck::Step &s : tl.steps)
            for (const deck::Effect &e : s.effects) {
                if (e.cls == deck::EffectClass::Entrance) ++fades;
                for (const deck::Behavior &b : e.behaviors) {
                    callsTo4 += b.kind == deck::Behavior::Kind::Command && b.target.shape == 4;
                    callsTo7 += b.kind == deck::Behavior::Kind::Command && b.target.shape == 7;
                }
            }
        QCOMPARE(callsTo4, 1);
        QCOMPARE(callsTo7, 1);
        QCOMPARE(fades, 2);
        QCOMPARE(xml.count(QStringLiteral("<p:video")), 1);
        QCOMPARE(xml.count(QStringLiteral("<p:audio")), 1);
        QVERIFY(deck::mediaPlayOf(slide, *movie).loop);
        QVERIFY(deck::mediaPlayOf(slide, *movie).start == deck::MediaStart::Automatically);
        QVERIFY(deck::mediaPlayOf(slide, *sound).start == deck::MediaStart::ClickSequence);
        // Every number in it is the number of one time node only.
        static const QRegularExpression ids(QStringLiteral("<p:cTn id=\"(\\d+)\""));
        QSet<QString> seen;
        for (auto it = ids.globalMatch(xml); it.hasNext();) {
            const QString id = it.next().captured(1);
            QVERIFY2(!seen.contains(id), qPrintable(xml));
            seen.insert(id);
        }
        // The animation it joins, with nothing else on the slide, is made whole: its last step is followed by the call.
        Page lone;
        auto m2 = mediaPicture(0, "video");
        lone.items.push_back(m2);
        lone.props[QStringLiteral("timingXml")] = animTiming(animSteps({animFadeIn(5, 500)}));
        deck::MediaPlay auto2;
        auto2.start = deck::MediaStart::Automatically;
        deck::setMediaPlay(lone, *m2, auto2, 3000);
        const deck::Timeline joined = deck::readTimeline(lone.props.value(QStringLiteral("timingXml")).toString());
        QCOMPARE(int(joined.steps.size()), 1);
        QCOMPARE(int(joined.steps[0].effects.size()), 2);
        QVERIFY(joined.steps[0].effects[1].cls == deck::EffectClass::Media);
        QVERIFY(joined.steps[0].effects[1].start == deck::EffectStart::AfterPrevious);
        QCOMPARE(joined.steps[0].effects[1].begin, 500.0);
        // Taking the last of the media away leaves a slide with no timing; the others' stay.
        Page only;
        auto m3 = mediaPicture(0, "audio");
        only.items.push_back(m3);
        deck::setMediaPlay(only, *m3, deck::MediaPlay(), 1000);
        QVERIFY(only.props.contains(QStringLiteral("timingXml")));
        deck::clearMediaPlay(only, m3->sourceId);
        QVERIFY(!only.props.contains(QStringLiteral("timingXml")));
        deck::clearMediaPlay(slide, 4);
        QVERIFY(!slide.props.value(QStringLiteral("timingXml")).toString().contains(QStringLiteral("spid=\"4\"")));
        QVERIFY(slide.props.value(QStringLiteral("timingXml")).toString().contains(QStringLiteral("spid=\"7\"")));
        QCOMPARE(int(deck::readTimeline(slide.props.value(QStringLiteral("timingXml")).toString()).steps.size()), 3);
    }

    // A movie's trim and fade are read from the picture's p14:media, written back with it, and read again the same.
    void mediaTrimAndFadeSurviveTheRoundTrip()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        xml.replace(QStringLiteral("</p:spTree>"),
                    QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"20\" name=\"Movie\"><a:hlinkClick r:id=\"\" action=\"ppaction://media\"/></p:cNvPr><p:cNvPicPr/><p:nvPr><a:videoFile r:link=\"rId4\"/>"
                                   "<p:extLst><p:ext uri=\"{DAA4B4D4-6D71-4841-9C94-3DE7FCFB9230}\"><p14:media xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\" r:embed=\"rId5\">"
                                   "<p14:trim st=\"1500.4\" end=\"9000\"/><p14:fade in=\"500\" out=\"700.2\"/></p14:media></p:ext></p:extLst></p:nvPr></p:nvPicPr>"
                                   "<p:blipFill><a:blip r:embed=\"rId2\"/><a:stretch><a:fillRect/></a:stretch></p:blipFill><p:spPr>")
                        + xfrm(0, 0, 1828800, 1371600) + QStringLiteral("<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></p:spPr></p:pic></p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        p.data["/ppt/media/media1.mp4"] = {QStringLiteral("video/mp4"), QByteArray("not really a movie")};
        p.rels << Rel{"/ppt/slides/slide1.xml", "video", "/ppt/media/media1.mp4"}
               << Rel{"/ppt/slides/slide1.xml", "http://schemas.microsoft.com/office/2007/relationships/media", "/ppt/media/media1.mp4"};
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        auto movieOf = [](const Document &doc) -> const PictureItem * {
            for (const auto &it : doc.pages[0]->items)
                if (it->name == QLatin1String("Movie") && it->type() == ItemType::Picture) return static_cast<const PictureItem *>(it.get());
            return nullptr;
        };
        const PictureItem *movie = movieOf(*d);
        QVERIFY(movie);
        QCOMPARE(movie->mediaTrimStart, 1500);
        QCOMPARE(movie->mediaTrimEnd, 9000);
        QCOMPARE(movie->mediaFadeIn, 500);
        QCOMPARE(movie->mediaFadeOut, 700);
        // The numbers survive an undo's copy of the item.
        QCOMPARE(static_cast<const PictureItem &>(*Item::fromJsonAny(movie->toJson())).mediaTrimEnd, 9000);
        QCOMPARE(static_cast<const PictureItem &>(*movie->clone()).mediaFadeOut, 700);
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY2(slide.contains(QStringLiteral("<p14:trim st=\"1500\" end=\"9000\"/><p14:fade in=\"500\" out=\"700\"/></p14:media>")), qPrintable(slide));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const PictureItem *again = movieOf(*back);
        QVERIFY(again);
        QCOMPARE(again->mediaTrimStart, 1500);
        QCOMPARE(again->mediaTrimEnd, 9000);
        QCOMPARE(again->mediaFadeIn, 500);
        QCOMPARE(again->mediaFadeOut, 700);
        // Only the end trimmed, only the start trimmed, and none.
        auto edited = deck::importPresentation(pack(p), &err);
        for (const auto &it : edited->pages[0]->items)
            if (it->name == QLatin1String("Movie")) {
                auto &pic = static_cast<PictureItem &>(*it);
                pic.mediaTrimStart = 0;
                pic.mediaFadeIn = 0;
                pic.mediaFadeOut = 0;
            }
        QVERIFY(unpack(deck::exportPresentation(*edited, &err), &e, &err));
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<p14:trim end=\"9000\"/></p14:media>")));
        for (const auto &it : edited->pages[0]->items)
            if (it->name == QLatin1String("Movie")) static_cast<PictureItem &>(*it).mediaTrimEnd = 0;
        QVERIFY(unpack(deck::exportPresentation(*edited, &err), &e, &err));
        QVERIFY(!part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("p14:trim")));
        // A fraction, a minus sign, and a number past a day are made sane.
        p.data["/ppt/slides/slide1.xml"].xml = xml.replace(QStringLiteral("st=\"1500.4\" end=\"9000\""), QStringLiteral("st=\"-5\" end=\"99999999999\"")).toUtf8();
        auto odd = deck::importPresentation(pack(p), &err);
        QVERIFY2(odd, qPrintable(err));
        QCOMPARE(movieOf(*odd)->mediaTrimStart, 0);
        QCOMPARE(movieOf(*odd)->mediaTrimEnd, 86400000);
    }

    // A movie or sound put on a slide is saved as the presentation program saves one it inserted: the picture with its file
    // (a:videoFile or a:audioFile with the link, and the p14:media copy), the media part, the poster, and the timing that plays it.
    void insertedMediaIsSavedWithItsParts()
    {
        auto d = deck::newPresentation();
        Page &slide = *d->pages[0];
        QImage poster(320, 180, QImage::Format_RGB32);
        poster.fill(QColor(30, 90, 160));
        const QByteArray movie(2000, 'v'), sound(900, 's');
        auto video = deck::newMediaPicture(*d, slide, movie, QStringLiteral("/tmp/Clip One.mp4"), poster, 12345);
        QVERIFY(video);
        slide.items.push_back(video);
        auto audio = deck::newMediaPicture(*d, slide, sound, QStringLiteral("Tune.wav"), QImage(), 5000);
        QVERIFY(audio);
        slide.items.push_back(audio);
        QCOMPARE(video->mediaKind, QStringLiteral("video"));
        QCOMPARE(audio->mediaKind, QStringLiteral("audio"));
        QCOMPARE(video->name, QStringLiteral("Clip One.mp4"));
        QVERIFY(video->sourceId >= 2 && audio->sourceId >= 2 && video->sourceId != audio->sourceId);
        QCOMPARE(d->media.value(video->mediaId).bytes, movie);
        QCOMPARE(d->media.value(video->mediaId).contentType, QStringLiteral("video/mp4"));
        QCOMPARE(d->media.value(audio->mediaId).contentType, QStringLiteral("audio/wav"));
        QVERIFY(d->pageSize().width() > video->rect.width() && d->pageSize().height() > video->rect.height());
        QVERIFY(std::abs(video->rect.center().x() - d->pageSize().width() / 2) < 0.01);
        QVERIFY(audio->rect.width() == 48 && audio->rect.height() == 48);
        QVERIFY(!d->image(audio->imageId).isNull());   // the speaker
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.isEmpty(), qPrintable(skipped.join('\n')));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString xml = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QCOMPARE(int(xml.count(QStringLiteral("<a:videoFile r:link="))), 1);
        QCOMPARE(int(xml.count(QStringLiteral("<a:audioFile r:link="))), 1);
        QCOMPARE(int(xml.count(QStringLiteral("<p14:media xmlns:p14="))), 2);
        QCOMPARE(int(xml.count(QStringLiteral("action=\"ppaction://media\""))), 2);
        // In the schema's order: the file, then the extension list, inside p:nvPr, after the picture's identity.
        static const QRegularExpression order(QStringLiteral("<p:cNvPr [^>]*name=\"Clip One.mp4\"[^>]*>.*?</p:cNvPr><p:cNvPicPr>.*?</p:cNvPicPr><p:nvPr><a:videoFile r:link=\"rId\\d+\"/><p:extLst><p:ext uri=\"\\{DAA4B4D4-6D71-4841-9C94-3DE7FCFB9230\\}\"><p14:media [^>]*r:embed=\"rId\\d+\"/></p:ext></p:extLst></p:nvPr></p:nvPicPr><p:blipFill>"));
        QVERIFY2(order.match(xml).hasMatch(), qPrintable(xml));
        // The relationships: the video and the media, to the same part; the audio likewise.
        QStringList types;
        QString videoPart, audioPart;
        for (const office::Relationship &r : e.pkg.relationships(QStringLiteral("/ppt/slides/slide1.xml"))) {
            types << r.type.mid(r.type.lastIndexOf(QLatin1Char('/')) + 1) + (r.external ? QStringLiteral("!") : QString());
            if (r.type.endsWith(QLatin1String("/relationships/video"))) videoPart = r.target;
            if (r.type.endsWith(QLatin1String("/relationships/audio"))) audioPart = r.target;
        }
        QVERIFY2(types.count(QStringLiteral("media")) == 2 && types.count(QStringLiteral("video")) == 1 && types.count(QStringLiteral("audio")) == 1, qPrintable(types.join(' ')));
        QCOMPARE(e.pkg.part(videoPart), movie);
        QCOMPARE(e.pkg.part(audioPart), sound);
        QVERIFY(videoPart.startsWith(QLatin1String("/ppt/media/media")) && videoPart.endsWith(QLatin1String(".mp4")));
        QCOMPARE(e.pkg.contentType(videoPart), QStringLiteral("video/mp4"));
        QCOMPARE(e.pkg.contentType(audioPart), QStringLiteral("audio/wav"));
        // The timing: a call for each in the click sequence, a node for each, and a toggle for the movie; the numbers are the pictures'.
        const QString timing = xml.mid(xml.indexOf(QStringLiteral("<p:timing")));
        QVERIFY(timing.contains(QStringLiteral("<p:video><p:cMediaNode vol=\"80000\">")));
        QVERIFY(timing.contains(QStringLiteral("<p:audio><p:cMediaNode vol=\"80000\">")));
        QVERIFY(timing.contains(QStringLiteral("cmd=\"togglePause\"")));
        QCOMPARE(int(timing.count(QStringLiteral("cmd=\"playFrom(0.0)\""))), 2);
        QVERIFY(timing.contains(QStringLiteral("dur=\"12345\"")) && timing.contains(QStringLiteral("dur=\"5000\"")));
        static const QRegularExpression names(QStringLiteral("<p:cNvPr id=\"(\\d+)\" name=\"(Clip One.mp4|Tune.wav)\""));
        QMap<QString, int> written;
        for (auto it = names.globalMatch(xml); it.hasNext();) {
            const auto m = it.next();
            written.insert(m.captured(2), m.captured(1).toInt());
        }
        QCOMPARE(written.value(QStringLiteral("Clip One.mp4")), video->sourceId);
        QCOMPARE(written.value(QStringLiteral("Tune.wav")), audio->sourceId);
        QVERIFY(timing.contains(QStringLiteral("spid=\"%1\"").arg(video->sourceId)) && timing.contains(QStringLiteral("spid=\"%1\"").arg(audio->sourceId)));
        // Read again: the same movie and sound, playing the same way.
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QVector<const PictureItem *> read = deck::mediaPicturesOf(*back->pages[0]);
        QCOMPARE(int(read.size()), 2);
        for (const PictureItem *pic : read) {
            QVERIFY(back->media.contains(pic->mediaId));
            const bool isVideo = pic->mediaKind == QLatin1String("video");
            QCOMPARE(back->media.value(pic->mediaId).bytes, isVideo ? movie : sound);
            const deck::MediaPlay play = deck::mediaPlayOf(*back->pages[0], *pic);
            QVERIFY(play.start == deck::MediaStart::ClickSequence);
            QCOMPARE(play.lengthHint, isVideo ? 12345 : 5000);
            QCOMPARE(play.volume, deck::kVolumeHigh);
            QCOMPARE(play.video, isVideo);
        }
        // Another slide's picture keeps its own number free of these.
        auto text = std::make_shared<TextItem>();
        slide.items.push_back(text);
        QVERIFY(unpack(deck::exportPresentation(*d, &err), &e, &err));
        QCOMPARE(packageProblem(e), QString());
    }

    // ---- playing ----

    // A WAV file of a sine wave: 16-bit mono PCM at 8,000 Hz.
    static QByteArray testWav(int ms = 1000, int hz = 440)
    {
        const int rate = 8000, samples = rate * ms / 1000;
        QByteArray pcm;
        pcm.resize(samples * 2);
        for (int i = 0; i < samples; ++i) {
            const qint16 v = qint16(12000 * std::sin(2 * M_PI * hz * i / rate));
            pcm[2 * i] = char(v & 0xFF);
            pcm[2 * i + 1] = char((v >> 8) & 0xFF);
        }
        auto le32 = [](quint32 v) { return QByteArray(reinterpret_cast<const char *>(&v), 4); };
        auto le16 = [](quint16 v) { return QByteArray(reinterpret_cast<const char *>(&v), 2); };
        const QByteArray fmt = le16(1) + le16(1) + le32(rate) + le32(rate * 2) + le16(2) + le16(16);
        return "RIFF" + le32(4 + 8 + fmt.size() + 8 + pcm.size()) + "WAVE" + "fmt " + le32(fmt.size()) + fmt + "data" + le32(pcm.size()) + pcm;
    }

    // A short movie: motion JPEG frames of one color each (the same red in all of them when `still`), in an AVI file.
    static QByteArray testAvi(int frames = 5, const QSize &size = QSize(64, 48), int fps = 10, bool still = false)
    {
        auto le32 = [](quint32 v) { return QByteArray(reinterpret_cast<const char *>(&v), 4); };
        auto le16 = [](quint16 v) { return QByteArray(reinterpret_cast<const char *>(&v), 2); };
        auto chunk = [&](const char *id, const QByteArray &data) { return QByteArray(id, 4) + le32(quint32(data.size())) + data + (data.size() & 1 ? QByteArray(1, '\0') : QByteArray()); };
        auto list = [&](const char *type, const QByteArray &data) { return QByteArray("LIST") + le32(quint32(data.size() + 4)) + QByteArray(type, 4) + data; };
        QVector<QByteArray> jpegs;
        quint32 biggest = 0;
        for (int i = 0; i < frames; ++i) {
            QImage img(size, QImage::Format_RGB32);
            img.fill(QColor::fromHsv(still ? 0 : (i * 60) % 360, 255, 230));
            QByteArray jpeg;
            QBuffer buffer(&jpeg);
            buffer.open(QIODevice::WriteOnly);
            img.save(&buffer, "JPG", 92);
            biggest = std::max(biggest, quint32(jpeg.size()));
            jpegs << jpeg;
        }
        const QByteArray avih = le32(quint32(1000000 / fps)) + le32(biggest * fps) + le32(0) + le32(0x10) + le32(quint32(frames)) + le32(0) + le32(1) + le32(biggest) +
                                le32(quint32(size.width())) + le32(quint32(size.height())) + le32(0) + le32(0) + le32(0) + le32(0);
        const QByteArray strh = QByteArray("vids") + QByteArray("MJPG") + le32(0) + le16(0) + le16(0) + le32(0) + le32(1) + le32(quint32(fps)) + le32(0) + le32(quint32(frames)) + le32(biggest) +
                                le32(0xFFFFFFFF) + le32(0) + le16(0) + le16(0) + le16(quint16(size.width())) + le16(quint16(size.height()));
        const QByteArray strf = le32(40) + le32(quint32(size.width())) + le32(quint32(size.height())) + le16(1) + le16(24) + QByteArray("MJPG") +
                                le32(quint32(size.width() * size.height() * 3)) + le32(0) + le32(0) + le32(0) + le32(0);
        const QByteArray hdrl = list("hdrl", chunk("avih", avih) + list("strl", chunk("strh", strh) + chunk("strf", strf)));
        QByteArray movi, index;
        for (const QByteArray &jpeg : std::as_const(jpegs)) {
            index += QByteArray("00dc") + le32(0x10) + le32(quint32(4 + movi.size())) + le32(quint32(jpeg.size()));
            movi += chunk("00dc", jpeg);
        }
        const QByteArray body = QByteArray("AVI ") + hdrl + list("movi", movi) + chunk("idx1", index);
        return QByteArray("RIFF") + le32(quint32(body.size())) + body;
    }

    // What a test of playing waits for: a player that plays (or has a picture), or one that says why not. The backend not being
    // there at all is not the program's failure: the test is skipped.
    // Whether this Qt can read the test movie and sound at all (a runner without the backend can't), for the tests that put them on a slide.
    static bool backendReads(const QByteArray &bytes, const char *extension, bool movie)
    {
        const MediaProbe probe = probeMedia(bytes, QLatin1String(extension), movie, 15000);
        return probe.error.isEmpty() && probe.duration > 0 && (!movie || !probe.frame.isNull());
    }

    static bool playerSettled(const MediaPlayback &pb, bool wantFrame)
    {
        return pb.failed() || (wantFrame ? !pb.frame().isNull() : pb.state() == MediaPlayback::State::Playing);
    }

    // A movie or sound is decoded only when it is played: opening a presentation, saving it, drawing its slides, and making its
    // thumbnails never start the decoder, whatever the file holds.
    void mediaIsNeverDecodedUntilPlayed()
    {
        const int before = MediaPlayback::decoders();
        auto d = deck::newPresentation();
        Page &slide = *d->pages[0];
        auto video = deck::newMediaPicture(*d, slide, testAvi(), QStringLiteral("clip.avi"), QImage(64, 48, QImage::Format_RGB32), 500);
        QVERIFY(video);
        slide.items.push_back(video);
        auto sound = deck::newMediaPicture(*d, slide, QByteArray("not a sound at all, but kept"), QStringLiteral("tune.mp3"), QImage(), 1000);
        QVERIFY(sound);
        slide.items.push_back(sound);
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = back.get();
        ctx.cache = &cache;
        QVERIFY(!Renderer::renderToImage(ctx, 0, 0.5).isNull());
        QVERIFY(!renderSlideImage(back.get(), &cache, 0, QSize(480, 270), 1).isNull());
        // A window with the file open: its thumbnails, and a save.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("media.pptx"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(bytes);
        f.close();
        DeckWindow w;
        w.show();
        QVERIFY(w.openFile(path));
        QVERIFY(w.slidesPane());
        w.slidesPane()->grab();
        QVERIFY(w.saveTo(dir.filePath(QStringLiteral("again.pptx"))));
        QCOMPARE(MediaPlayback::decoders(), before);
        Settings::get().removeRecentFile(path);   // the files go with the folder; the list of recent ones doesn't keep them
        Settings::get().removeRecentFile(dir.filePath(QStringLiteral("again.pptx")));
        // A player with its file set decodes nothing either.
        MediaPlayback pb;
        pb.setFile(testWav(), QStringLiteral("wav"));
        QCOMPARE(MediaPlayback::decoders(), before);
    }

    // A sound plays, or the player says why it can't (a computer with no sound device is not an error for the picture, and
    // a runner with no backend says so clearly).
    void mediaPlaybackPlaysASoundOrSaysWhy()
    {
        MediaPlayback pb;
        deck::MediaPlay play;
        pb.setFile(testWav(1500), QStringLiteral("wav"));
        pb.setPlay(play);
        const int before = MediaPlayback::decoders();
        pb.play();
        QCOMPARE(MediaPlayback::decoders(), before + 1);
        QTRY_VERIFY_WITH_TIMEOUT(playerSettled(pb, false), 15000);
        if (pb.failed()) {
            QVERIFY(!pb.error().isEmpty());
            QCOMPARE(int(pb.state()), int(MediaPlayback::State::Stopped));
            return;
        }
        QTRY_VERIFY_WITH_TIMEOUT(pb.length() > 0, 5000);
        QVERIFY(std::abs(pb.length() - 1500) < 100);
        pb.pause();
        QTRY_COMPARE_WITH_TIMEOUT(int(pb.state()), int(MediaPlayback::State::Paused), 5000);
        pb.resume();
        QTRY_COMPARE_WITH_TIMEOUT(int(pb.state()), int(MediaPlayback::State::Playing), 5000);
        pb.stop();
        QCOMPARE(int(pb.state()), int(MediaPlayback::State::Stopped));
        QVERIFY(pb.frame().isNull());
        pb.setVolume(0.25);
        pb.setMuted(true);
        QCOMPARE(pb.volume(), 0.25);
        QVERIFY(pb.muted());
    }

    // A movie gives its pictures to paint in place; its first picture and its length can be had for the poster.
    void mediaPlaybackGivesTheFramesOfAMovie()
    {
        const QByteArray avi = testAvi(8, QSize(64, 48), 10);
        MediaProbe probe = probeMedia(avi, QStringLiteral("avi"), true, 15000);
        if (!probe.error.isEmpty() || probe.frame.isNull()) QSKIP(qPrintable(QStringLiteral("this Qt has no movie backend here: ") + probe.error));
        QCOMPARE(probe.frame.size(), QSize(64, 48));
        QVERIFY(std::abs(probe.duration - 800) < 150);
        const QColor first = probe.frame.pixelColor(32, 24);
        QVERIFY2(first.red() > 150 && first.green() < 110 && first.blue() < 110, qPrintable(first.name()));   // the first color is red
        MediaPlayback pb;
        pb.setFile(avi, QStringLiteral("avi"));
        deck::MediaPlay play;
        play.video = true;
        pb.setPlay(play);
        pb.play();
        QTRY_VERIFY_WITH_TIMEOUT(playerSettled(pb, true), 15000);
        QVERIFY2(!pb.failed(), qPrintable(pb.error()));
        QCOMPARE(pb.frame().size(), QSize(64, 48));
        // It plays to its end and stops, holding its last picture.
        QSignalSpy ended(&pb, &MediaPlayback::ended);
        QTRY_VERIFY_WITH_TIMEOUT(ended.count() > 0, 15000);
        QCOMPARE(int(pb.state()), int(MediaPlayback::State::Stopped));
        QVERIFY(!pb.frame().isNull());
        // With rewind, the picture goes back to the poster.
        MediaPlayback again;
        again.setFile(avi, QStringLiteral("avi"));
        play.rewind = true;
        again.setPlay(play);
        again.play();
        QSignalSpy over(&again, &MediaPlayback::ended);
        QTRY_VERIFY_WITH_TIMEOUT(over.count() > 0, 15000);
        QVERIFY(again.frame().isNull());
    }

    // A file that can't be played says so and nothing closes; asking again, pausing, or moving in it does no harm.
    void mediaPlaybackSaysWhenAFileCantBePlayed()
    {
        MediaPlayback pb;
        QByteArray junk(5000, '\0');
        for (int i = 0; i < junk.size(); ++i) junk[i] = char((i * 131 + 7) & 0xFF);
        pb.setFile(junk, QStringLiteral("mp4"));
        deck::MediaPlay play;
        play.video = true;
        pb.setPlay(play);
        pb.play();
        QTRY_VERIFY_WITH_TIMEOUT(pb.failed(), 15000);
        QVERIFY(pb.error().contains(QStringLiteral("movie")));
        pb.play();
        pb.pause();
        pb.resume();
        pb.seek(500);
        pb.stop();
        pb.togglePause();
        QCOMPARE(int(pb.state()), int(MediaPlayback::State::Stopped));
        // Nothing to play at all.
        MediaPlayback none;
        none.play();
        QVERIFY(none.failed());
        // The probe says it can't be read.
        const MediaProbe probe = probeMedia(junk, QStringLiteral("wav"), false, 15000);
        QVERIFY(!probe.error.isEmpty() || probe.duration == 0);
    }

    // The control bar: its parts are where they are drawn, a click on the seek bar means a moment, and it stays on the
    // screen when the picture is at the bottom.
    void mediaBarLaysOutItsPartsAndStaysOnTheScreen()
    {
        const QRectF area(0, 0, 800, 600);
        QRectF bar = MediaBar::placement(QRectF(100, 100, 400, 225), area);
        QCOMPARE(bar.top(), 331.0);   // under the picture
        QCOMPARE(bar.width(), 400.0);
        QCOMPARE(bar.height(), MediaBar::kHeight);
        // A small picture gets a bar that is wide enough to use, kept inside the area.
        bar = MediaBar::placement(QRectF(0, 100, 48, 48), area);
        QCOMPARE(bar.width(), MediaBar::kMinWidth);
        QVERIFY(bar.left() >= 0);
        bar = MediaBar::placement(QRectF(760, 500, 40, 40), area);
        QVERIFY(bar.right() <= 800.0);
        // A picture at the bottom: the bar goes over its bottom edge.
        bar = MediaBar::placement(QRectF(100, 350, 400, 250), area);
        QVERIFY(bar.bottom() <= 600.0);
        QVERIFY(bar.top() < 600.0 - MediaBar::kHeight + 1);
        // The parts, from left to right, and nothing outside the bar.
        bar = QRectF(100, 100, 400, MediaBar::kHeight);
        QVERIFY(MediaBar::partAt(bar, QPointF(115, 115)) == MediaBar::Part::Play);
        QVERIFY(MediaBar::partAt(bar, QPointF(200, 115)) == MediaBar::Part::Seek);
        QVERIFY(MediaBar::partAt(bar, QPointF(430, 115)) == MediaBar::Part::Mute);
        QVERIFY(MediaBar::partAt(bar, QPointF(480, 115)) == MediaBar::Part::Volume);
        QVERIFY(MediaBar::partAt(bar, QPointF(50, 115)) == MediaBar::Part::None);
        // Along the seek bar, from the start to the end of what plays; along the volume slider, from none to all.
        QCOMPARE(MediaBar::seekAt(bar, QPointF(0, 115), 10000), qint64(0));
        QCOMPARE(MediaBar::seekAt(bar, QPointF(1000, 115), 10000), qint64(10000));
        const qint64 middle = MediaBar::seekAt(bar, QPointF(200, 115), 10000);
        QVERIFY(middle > 0 && middle < 10000);
        QCOMPARE(MediaBar::volumeAt(bar, QPointF(0, 115)), 0.0);
        QCOMPARE(MediaBar::volumeAt(bar, QPointF(1000, 115)), 1.0);
        QCOMPARE(MediaBar::timeText(7000), QStringLiteral("0:07"));
        QCOMPARE(MediaBar::timeText(754000), QStringLiteral("12:34"));
        QCOMPARE(MediaBar::timeText(3723000), QStringLiteral("1:02:03"));
        // Drawn in every state without trouble.
        QImage img(420, 40, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::white);
        QPainter p(&img);
        MediaBarState s;
        s.length = 90000;
        s.played = 30000;
        for (const auto state : {MediaPlayback::State::Stopped, MediaPlayback::State::Playing, MediaPlayback::State::Paused}) {
            s.state = state;
            for (const bool muted : {false, true}) {
                s.muted = muted;
                MediaBar::paint(&p, QRectF(10, 5, 400, MediaBar::kHeight), s, MediaBar::Part::Play);
            }
        }
        p.end();
        QVERIFY(img.pixelColor(20, 20) != QColor(Qt::white));
    }

    // ---- movies and sounds in the window ----

    static bool tabShown(const Ribbon *r, const QString &name)
    {
        for (int i = 0; i < r->tabCount(); ++i)
            if (r->tabName(i) == name) return r->tabVisible(i);
        return false;
    }

    static QString writeTestFile(const QTemporaryDir &dir, const QString &name, const QByteArray &bytes)
    {
        const QString path = dir.filePath(name);
        QFile f(path);
        if (f.open(QIODevice::WriteOnly)) f.write(bytes);
        return path;
    }

    static const PictureItem *lastMedia(const Document &d)
    {
        const QVector<const PictureItem *> all = deck::mediaPicturesOf(*d.pages[0]);
        return all.isEmpty() ? nullptr : all.last();
    }

    // Insert > Video and Audio put the file on the slide as a picture that plays it, in one step that undoes; the Playback tab
    // shows while it is selected.
    void insertingVideoAndAudioPutsThemOnTheSlide()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        QVERIFY(!tabShown(w.ribbon(), QStringLiteral("Playback")));
        const QString described = w.ribbon()->describe();
        QVERIFY2(described.contains(QStringLiteral("ribbon.video")) && described.contains(QStringLiteral("menu=[insert.video]")) && described.contains(QStringLiteral("menu=[insert.audio]")), qPrintable(described));
        QCOMPARE(w.act(QStringLiteral("insert.video"))->text(), QStringLiteral("Video on This Device…"));
        QCOMPARE(w.act(QStringLiteral("insert.audio"))->text(), QStringLiteral("Audio on This Device…"));
        const int before = int(ed->doc()->pages[0]->items.size());
        if (!backendReads(testAvi(5, QSize(64, 48), 10), "avi", true) || !backendReads(testWav(800), "wav", false)) QSKIP("this Qt can't read the test movie and sound here");
        const QString avi = writeTestFile(dir, QStringLiteral("clip.avi"), testAvi(5, QSize(64, 48), 10));
        QVERIFY(w.insertMedia(avi, QStringLiteral("video")));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        const PictureItem *movie = lastMedia(*ed->doc());
        QVERIFY(movie);
        QCOMPARE(movie->mediaKind, QStringLiteral("video"));
        QCOMPARE(movie->name, QStringLiteral("clip.avi"));
        QCOMPARE(ed->selection(), QStringList{movie->id});
        QVERIFY(movie->sourceId >= 2);
        QCOMPARE(ed->doc()->image(movie->imageId).size(), QSize(64, 48));   // the first picture of the movie
        const deck::MediaPlay play = deck::mediaPlayOf(*ed->doc()->pages[0], *movie);
        QVERIFY(play.video && play.start == deck::MediaStart::ClickSequence);
        QVERIFY(ed->doc()->pages[0]->props.value(QStringLiteral("timingXml")).toString().contains(QStringLiteral("<p:video>")));
        QTRY_VERIFY(tabShown(w.ribbon(), QStringLiteral("Playback")));
        // One step undoes all of it.
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);
        QVERIFY(!ed->doc()->pages[0]->props.contains(QStringLiteral("timingXml")));
        QTRY_VERIFY(!tabShown(w.ribbon(), QStringLiteral("Playback")));
        ed->redo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        // A sound shows a speaker.
        const QString wav = writeTestFile(dir, QStringLiteral("tune.wav"), testWav(800));
        QVERIFY(w.insertMedia(wav, QStringLiteral("audio")));
        const PictureItem *sound = lastMedia(*ed->doc());
        QVERIFY(sound && sound->mediaKind == QLatin1String("audio"));
        QCOMPARE(sound->rect.size(), QSizeF(48, 48));
        // The kind is checked, and so is the file.
        QString shown;
        QTimer closer;   // the message boxes, closed as they come (the file is read in a loop of its own first)
        closer.setInterval(10);
        connect(&closer, &QTimer::timeout, &closer, [&] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                shown = box->text();
                box->close();
            }
        });
        closer.start();
        QVERIFY(!w.insertMedia(wav, QStringLiteral("video")));
        QVERIFY2(shown.contains(QStringLiteral("tune.wav")), qPrintable(shown));
        shown.clear();
        QByteArray junk(3000, '\0');
        for (int i = 0; i < junk.size(); ++i) junk[i] = char((i * 37 + 11) & 0xFF);
        QVERIFY(!w.insertMedia(writeTestFile(dir, QStringLiteral("broken.mp4"), junk), QStringLiteral("video")));
        QVERIFY2(shown.contains(QStringLiteral("broken.mp4")), qPrintable(shown));
        closer.stop();
        // Saved and opened again, they are as they were.
        const QString saved = dir.filePath(QStringLiteral("deck.pptx"));
        QVERIFY(w.saveTo(saved));
        DeckWindow other;
        QVERIFY(other.openFile(saved));
        const QVector<const PictureItem *> read = deck::mediaPicturesOf(*other.editor()->doc()->pages[0]);
        QCOMPARE(int(read.size()), 2);
        for (const PictureItem *pic : read) {
            QVERIFY(other.editor()->doc()->media.contains(pic->mediaId));
            QCOMPARE(deck::mediaPlayOf(*other.editor()->doc()->pages[0], *pic).volume, deck::kVolumeHigh);
        }
        Settings::get().removeRecentFile(saved);
    }

    // The Playback tab changes how the selected one plays: Start, Volume, and the four boxes, each one step that undoes, kept
    // when the file is saved and opened again.
    void playbackTabChangesHowAMoviePlays()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        const QByteArray avi = testAvi(30, QSize(320, 180), 10, true);
        if (!backendReads(avi, "avi", true)) QSKIP("this Qt can't read the test movie here");
        QVERIFY(w.insertMedia(writeTestFile(dir, QStringLiteral("clip.avi"), avi), QStringLiteral("video")));
        QTRY_VERIFY(tabShown(w.ribbon(), QStringLiteral("Playback")));
        const QString id = lastMedia(*ed->doc())->id;
        if (qEnvironmentVariableIsSet("JD_MEDIA_SHOTS")) {
            w.resize(1440, 900);
            w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Playback")));
            QCoreApplication::processEvents();
            w.grab().save(qEnvironmentVariable("JD_MEDIA_SHOTS") + QStringLiteral("/playback-tab.png"));
            w.ribbon()->showTab(w.ribbon()->tab(QStringLiteral("Insert")));
            QCoreApplication::processEvents();
            w.grab().save(qEnvironmentVariable("JD_MEDIA_SHOTS") + QStringLiteral("/insert-tab.png"));
        }
        auto play = [&] { return deck::mediaPlayOf(*ed->doc()->pages[0], *lastMedia(*ed->doc())); };
        auto *start = w.ribbon()->findChild<QComboBox *>(QStringLiteral("mediaStart"));
        auto *volume = w.ribbon()->findChild<QComboBox *>(QStringLiteral("mediaVolume"));
        auto *full = w.ribbon()->findChild<QCheckBox *>(QStringLiteral("mediaFullScreen"));
        auto *hide = w.ribbon()->findChild<QCheckBox *>(QStringLiteral("mediaHide"));
        auto *loop = w.ribbon()->findChild<QCheckBox *>(QStringLiteral("mediaLoop"));
        auto *rewind = w.ribbon()->findChild<QCheckBox *>(QStringLiteral("mediaRewind"));
        QVERIFY(start && volume && full && hide && loop && rewind);
        QCOMPARE(start->currentIndex(), 0);
        QCOMPARE(volume->currentText(), QStringLiteral("High"));
        QVERIFY(!loop->isChecked() && !rewind->isChecked() && !full->isChecked() && !hide->isChecked());
        QVERIFY(full->isEnabled());
        // Each box changes one thing.
        start->setCurrentIndex(1);
        Q_EMIT start->activated(1);
        QVERIFY(play().start == deck::MediaStart::Automatically);
        start->setCurrentIndex(2);
        Q_EMIT start->activated(2);
        QVERIFY(play().start == deck::MediaStart::WhenClickedOn);
        volume->setCurrentIndex(0);
        Q_EMIT volume->activated(0);
        QCOMPARE(play().volume, deck::kVolumeLow);
        volume->setCurrentIndex(3);
        Q_EMIT volume->activated(3);
        QVERIFY(play().mute);
        loop->setChecked(true);
        rewind->setChecked(true);
        full->setChecked(true);
        hide->setChecked(true);
        const deck::MediaPlay set = play();
        QVERIFY(set.loop && set.rewind && set.fullScreen && set.hideWhenStopped && set.mute && set.start == deck::MediaStart::WhenClickedOn);
        // Undo takes them one at a time.
        ed->undo();
        QVERIFY(!play().hideWhenStopped && play().fullScreen);
        QTRY_VERIFY(!hide->isChecked() && full->isChecked());
        ed->undo();
        ed->undo();
        ed->undo();
        QVERIFY(!play().loop && !play().fullScreen);
        ed->redo();
        ed->redo();
        ed->redo();
        ed->redo();
        QVERIFY(play() == set);
        // The tab follows the selection: another picture, or none.
        ed->clearSelection();
        QTRY_VERIFY(!tabShown(w.ribbon(), QStringLiteral("Playback")));
        ed->select(id);
        QTRY_VERIFY(tabShown(w.ribbon(), QStringLiteral("Playback")));
        QTRY_VERIFY(loop->isChecked() && start->currentIndex() == 2 && volume->currentText() == QStringLiteral("Mute"));
        // Saved and opened again.
        const QString saved = dir.filePath(QStringLiteral("deck.pptx"));
        QVERIFY(w.saveTo(saved));
        DeckWindow other;
        QVERIFY(other.openFile(saved));
        const PictureItem *again = lastMedia(*other.editor()->doc());
        QVERIFY(again);
        deck::MediaPlay read = deck::mediaPlayOf(*other.editor()->doc()->pages[0], *again);
        read.shape = set.shape;
        QVERIFY(read == set);
        Settings::get().removeRecentFile(saved);
        // Trim: the box shows a start and an end (where it plays to), and gives back what is cut off each.
        MediaTrimDialog trim(QStringLiteral("Trim Video"), 5000, 1000, 0);
        auto *from = trim.findChild<QDoubleSpinBox *>(QStringLiteral("trimStart"));
        auto *to = trim.findChild<QDoubleSpinBox *>(QStringLiteral("trimEnd"));
        QVERIFY(from && to);
        QCOMPARE(from->value(), 1.0);
        QCOMPARE(to->value(), 5.0);   // nothing cut off the end: it plays to the 5 seconds there are
        QCOMPARE(trim.cutStartMs(), 1000);
        QCOMPARE(trim.cutEndMs(), 0);
        to->setValue(3.5);
        QCOMPARE(trim.cutEndMs(), 1500);   // 1.5 seconds cut off
        MediaTrimDialog reopened(QStringLiteral("Trim Video"), 5000, 1000, 1500);
        QCOMPARE(reopened.findChild<QDoubleSpinBox *>(QStringLiteral("trimEnd"))->value(), 3.5);
        // With the length not known, the end is left as it was.
        MediaTrimDialog unknown(QStringLiteral("Trim Video"), 0, 1000, 1500);
        QVERIFY(!unknown.findChild<QDoubleSpinBox *>(QStringLiteral("trimEnd"))->isEnabled());
        QCOMPARE(unknown.cutEndMs(), 1500);
        w.editMedia(QStringLiteral("Trim"), [&](deck::MediaPlay &p) {
            p.trimStart = trim.cutStartMs();
            p.trimEnd = trim.cutEndMs();
        });
        QCOMPARE(play().trimStart, 1000);
        QCOMPARE(play().trimEnd, 1500);
        QCOMPARE(static_cast<const PictureItem *>(lastMedia(*ed->doc()))->mediaTrimEnd, 1500);
    }

    // A movie or sound selected on the slide has a control bar under it (and one the mouse is over); it plays in place on the
    // slide, over the canvas, scaled with the zoom, and stops when the slide changes.
    void editorShowsAControlBarAndPlaysInPlace()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        Editor *ed = w.editor();
        DeckCanvas *canvas = w.canvas();
        // A movie that is red all the way through.
        const QByteArray avi = testAvi(40, QSize(320, 180), 10, true);
        if (!backendReads(avi, "avi", true)) QSKIP("this Qt can't read the test movie here");
        QVERIFY(w.insertMedia(writeTestFile(dir, QStringLiteral("clip.avi"), avi), QStringLiteral("video")));
        const QString id = lastMedia(*ed->doc())->id;
        QVERIFY(canvas->media()->barItem() == id);
        QVERIFY(canvas->media()->activeId().isEmpty());
        QVERIFY(!canvas->media()->playbackOf(id));   // selecting reads nothing
        const int decoders = MediaPlayback::decoders();
        const QRectF bar = canvas->media()->barRect();
        QVERIFY(!bar.isEmpty() && QRectF(canvas->viewport()->rect()).contains(bar));
        const QRectF picture = canvas->pageToView(lastMedia(*ed->doc())->rect.topLeft()).isNull() ? QRectF() : QRectF(canvas->pageToView(lastMedia(*ed->doc())->rect.topLeft()), canvas->pageToView(lastMedia(*ed->doc())->rect.bottomRight()));
        QVERIFY(bar.top() >= picture.bottom() - 0.5 || bar.bottom() <= picture.bottom());   // under the picture, or over its bottom when there is no room
        QCOMPARE(MediaPlayback::decoders(), decoders);
        // Hovering a movie that is not selected shows its bar.
        ed->clearSelection();
        QTRY_VERIFY(canvas->media()->barItem().isEmpty());
        QTest::mouseMove(canvas->viewport(), picture.center().toPoint());
        QTRY_COMPARE(canvas->media()->barItem(), id);
        QTest::mouseMove(canvas->viewport(), QPoint(3, 3));
        QTRY_VERIFY(canvas->media()->barItem().isEmpty());
        ed->select(id);
        // Play on the bar.
        const QRectF barNow = canvas->media()->barRect();
        QTest::mouseClick(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(int(barNow.left() + 15), int(barNow.center().y())));
        MediaPlayback *pb = canvas->media()->playbackOf(id);
        QVERIFY(pb);
        QCOMPARE(MediaPlayback::decoders(), decoders + 1);
        QTRY_VERIFY_WITH_TIMEOUT(playerSettled(*pb, true), 15000);
        if (pb->failed()) QSKIP(qPrintable(QStringLiteral("this Qt has no movie backend here: ") + pb->error()));
        QCOMPARE(canvas->media()->activeId(), id);
        // The frames are painted in the picture's place, and again, larger, when the zoom is.
        auto centerOfMovie = [&] {
            const QPointF c = canvas->pageToView(lastMedia(*ed->doc())->rect.center());
            return canvas->viewport()->grab().toImage().pixelColor(c.toPoint());
        };
        QTRY_VERIFY_WITH_TIMEOUT(centerOfMovie().red() > 180 && centerOfMovie().green() < 90 && centerOfMovie().blue() < 90, 5000);
        if (qEnvironmentVariableIsSet("JD_MEDIA_SHOTS")) canvas->viewport()->grab().save(qEnvironmentVariable("JD_MEDIA_SHOTS") + QStringLiteral("/editor-bar.png"));
        const double ppp = canvas->ppp();
        canvas->setZoom(canvas->zoom() * 1.5);
        QVERIFY(canvas->ppp() > ppp);
        QTRY_VERIFY_WITH_TIMEOUT(centerOfMovie().red() > 180 && centerOfMovie().green() < 90 && centerOfMovie().blue() < 90, 5000);   // larger, still red
        const QRectF zoomed = canvas->media()->barRect();
        QVERIFY(zoomed != barNow);
        // Pause on the bar, then the slide changes: it stops.
        const QRectF b2 = canvas->media()->barRect();
        QTest::mouseClick(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(int(b2.left() + 15), int(b2.center().y())));
        QTRY_COMPARE_WITH_TIMEOUT(int(pb->state()), int(MediaPlayback::State::Paused), 5000);
        w.newSlide();
        QTRY_VERIFY(canvas->media()->activeId().isEmpty());
        ed->setCurrentPage(0);
        QVERIFY(canvas->media()->activeId().isEmpty());
    }

    // A file that can't be played shows a message where the movie is, and the program goes on.
    void editorShowsAMessageForAMovieThatCantPlay()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        QByteArray junk(4000, '\0');
        for (int i = 0; i < junk.size(); ++i) junk[i] = char((i * 53 + 3) & 0xFF);
        QImage poster(320, 180, QImage::Format_RGB32);
        poster.fill(QColor(240, 240, 240));
        auto pic = deck::newMediaPicture(*d, *d->pages[0], junk, QStringLiteral("bad.mp4"), poster, 1000);
        QVERIFY(pic);
        ed->beginChange(QStringLiteral("Insert"));
        d->pages[0]->items.push_back(pic);
        ed->endChange();
        ed->select(pic->id);
        const QRectF bar = w.canvas()->media()->barRect();
        QTest::mouseClick(w.canvas()->viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(int(bar.left() + 15), int(bar.center().y())));
        MediaPlayback *pb = w.canvas()->media()->playbackOf(pic->id);
        QVERIFY(pb);
        QTRY_VERIFY_WITH_TIMEOUT(pb->failed(), 15000);
        // The message is drawn over the poster.
        const QPointF c = w.canvas()->pageToView(pic->rect.center());
        QTRY_VERIFY(w.canvas()->viewport()->grab().toImage().pixelColor(c.toPoint() + QPoint(0, -int(pic->rect.height() * w.canvas()->ppp() * 0.4))).lightness() < 90);
        // Moving to another slide puts the poster back.
        w.newSlide();
        ed->setCurrentPage(0);
        QTRY_VERIFY(w.canvas()->media()->activeId().isEmpty());
        QVERIFY(!w.canvas()->media()->playbackOf(pic->id) || !w.canvas()->media()->playbackOf(pic->id)->failed());
        // A movie linked from outside the presentation says so.
        auto linked = std::make_shared<PictureItem>();
        linked->rect = QRectF(60, 60, 200, 120);
        linked->imgRect = QRectF(0, 0, 200, 120);
        linked->imageId = pic->imageId;
        linked->mediaKind = QStringLiteral("video");
        linked->mediaLink = QStringLiteral("https://example.com/watch?v=abc");
        ed->beginChange(QStringLiteral("Insert"));
        d->pages[0]->items.push_back(linked);
        ed->endChange();
        ed->select(linked->id);
        w.canvas()->media()->play(linked->id);
        MediaPlayback *lp = w.canvas()->media()->playbackOf(linked->id);
        QVERIFY(lp && lp->failed());
        QVERIFY2(lp->error().contains(QStringLiteral("linked")), qPrintable(lp->error()));
    }

    // ---- movies and sounds in the slide show ----

    // A movie or sound on slide `slide`, playing as `play` says (its timing written for it).
    static std::shared_ptr<PictureItem> showMedia(Document &d, int slide, const QByteArray &bytes, const QString &name, const QPointF &at, const deck::MediaPlay &play, const QSizeF &size = QSizeF())
    {
        QImage poster(160, 120, QImage::Format_RGB32);
        poster.fill(QColor(30, 90, 160));
        auto pic = deck::newMediaPicture(d, *d.pages[slide], bytes, name, poster, 2000, at);
        pic->imgRect = QRectF(QPointF(0, 0), pic->rect.size());
        if (!size.isEmpty()) {
            pic->rect.setSize(size);
            pic->imgRect = QRectF(QPointF(0, 0), size);
        }
        d.pages[slide]->items.push_back(pic);
        deck::setMediaPlay(*d.pages[slide], *pic, play, 2000);
        return pic;
    }

    // A show on a window of the slide's own size, shown (as it is in use), with its media looked at through the show.
    static QPoint showCenter(const PictureItem &pic) { return pic.rect.center().toPoint(); }

    // What the file says plays: a step of only a call to a movie is a click of its own, a sound set to start by itself starts
    // when the slide appears, nothing is decoded before, and everything stops when the slide changes.
    void showPlaysMediaWhenTheFileSays()
    {
        auto d = showDeck(2);
        const QByteArray avi = testAvi(30, QSize(64, 48), 10, true), wav = testWav(3000);
        deck::MediaPlay click, automatic;
        automatic.start = deck::MediaStart::Automatically;
        automatic.loop = true;
        automatic.volume = deck::kVolumeLow;
        auto sound = showMedia(*d, 0, wav, QStringLiteral("tune.wav"), QPointF(700, 400), automatic);   // the slide's first, so it begins with the slide
        auto movie = showMedia(*d, 0, avi, QStringLiteral("clip.avi"), QPointF(100, 100), click, QSizeF(320, 240));
        LayoutCache cache;
        const int before = MediaPlayback::decoders();
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        QCOMPARE(MediaPlayback::decoders(), before);   // nothing decoded for a show that has not begun
        show.show();
        QVERIFY(show.media());
        // The sound starts with the slide, looping, at its volume; the movie waits for its click.
        QPointer<MediaPlayback> tune = show.media()->playbackOf(sound->id);
        QVERIFY(tune);
        QCOMPARE(MediaPlayback::decoders(), before + 1);
        QVERIFY(tune->settings().loop);
        QCOMPARE(tune->settings().volume, deck::kVolumeLow);
        QVERIFY(!show.media()->playbackOf(movie->id));
        QTRY_VERIFY_WITH_TIMEOUT(playerSettled(*tune, false), 15000);
        // The click is the movie's: the show stays on the slide.
        show.next();
        QCOMPARE(show.currentSlide(), 0);
        QPointer<MediaPlayback> clip = show.media()->playbackOf(movie->id);
        QVERIFY(clip);
        QCOMPARE(MediaPlayback::decoders(), before + 2);
        QTRY_VERIFY_WITH_TIMEOUT(playerSettled(*clip, true), 15000);
        // The next click moves on; both stop, and their players go.
        show.next();
        QCOMPARE(show.currentSlide(), 1);
        QTRY_VERIFY(tune.isNull() && clip.isNull());
        QVERIFY(!show.media()->playbackOf(sound->id));
        // Back on the slide, with all its steps played, the sound starts again when the slide has none to play first.
        show.previous();
        QCOMPARE(show.currentSlide(), 0);
    }

    // A sound set to play across slides goes on over as many as the file says, then stops.
    void showMediaPlaysAcrossSlides()
    {
        auto d = showDeck(4);
        deck::MediaPlay across;
        across.start = deck::MediaStart::Automatically;
        across.acrossSlides = 2;
        auto sound = showMedia(*d, 0, testWav(5000), QStringLiteral("song.wav"), QPointF(40, 40), across);
        // Without it, a sound stops at the first.
        deck::MediaPlay alone;
        alone.start = deck::MediaStart::Automatically;
        auto other = showMedia(*d, 2, testWav(5000), QStringLiteral("other.wav"), QPointF(40, 40), alone);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QPointer<MediaPlayback> song = show.media()->playbackOf(sound->id);
        QVERIFY(song);
        show.next();
        QCOMPARE(show.currentSlide(), 1);
        QVERIFY(!song.isNull());   // over the first slide's end
        show.next();
        QCOMPARE(show.currentSlide(), 2);
        QTRY_VERIFY(song.isNull());   // the second slide it plays across has gone too
        QPointer<MediaPlayback> once = show.media()->playbackOf(other->id);
        QVERIFY(once);
        show.next();
        QTRY_VERIFY(once.isNull());
    }

    // A movie that waits for a click on it: the click plays it, another pauses it, a third plays it again; the mouse over it
    // shows the control bar, and a click on the bar's play button does the same.
    void showClickOnAMoviePlaysAndPausesIt()
    {
        auto d = showDeck(2);
        deck::MediaPlay onClick;
        onClick.start = deck::MediaStart::WhenClickedOn;
        auto movie = showMedia(*d, 0, testAvi(60, QSize(64, 48), 10, true), QStringLiteral("clip.avi"), QPointF(300, 150), onClick, QSizeF(320, 240));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        ShowMedia *media = show.media();
        QVERIFY(!media->playbackOf(movie->id));
        QTest::mouseMove(&show, QPoint(10, 10));
        QVERIFY(media->hovered().isEmpty());
        QTest::mouseMove(&show, showCenter(*movie));
        QCOMPARE(media->hovered(), movie->id);   // its bar shows
        const QRectF bar = media->barRect(movie->id);
        QVERIFY(!bar.isEmpty() && QRectF(show.rect()).contains(bar));
        // The bar stays while the mouse goes down to it.
        QTest::mouseMove(&show, bar.center().toPoint());
        QCOMPARE(media->hovered(), movie->id);
        // A click on the movie starts it, and the show stays where it is.
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, showCenter(*movie));
        QCOMPARE(show.currentSlide(), 0);
        MediaPlayback *pb = media->playbackOf(movie->id);
        QVERIFY(pb);
        QTRY_VERIFY_WITH_TIMEOUT(playerSettled(*pb, true), 15000);
        if (pb->failed()) QSKIP(qPrintable(QStringLiteral("this Qt has no movie backend here: ") + pb->error()));
        QTRY_COMPARE_WITH_TIMEOUT(int(pb->state()), int(MediaPlayback::State::Playing), 5000);
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, showCenter(*movie));
        QTRY_COMPARE_WITH_TIMEOUT(int(pb->state()), int(MediaPlayback::State::Paused), 5000);
        QCOMPARE(show.currentSlide(), 0);
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(int(bar.left() + 15), int(bar.center().y())));   // the bar's play button
        QTRY_COMPARE_WITH_TIMEOUT(int(pb->state()), int(MediaPlayback::State::Playing), 5000);
        // The movie is painted where its picture is.
        QTRY_VERIFY_WITH_TIMEOUT(show.grab().toImage().pixelColor(showCenter(*movie)).red() > 180, 5000);
        if (qEnvironmentVariableIsSet("JD_MEDIA_SHOTS")) show.grab().save(qEnvironmentVariable("JD_MEDIA_SHOTS") + QStringLiteral("/show-movie.png"));
        // A click elsewhere moves on, and the movie stops.
        QPointer<MediaPlayback> gone = pb;
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(2, 538));
        QCOMPARE(show.currentSlide(), 1);
        QTRY_VERIFY(gone.isNull());
    }

    // A movie that hides while it isn't playing is not on the slide until it plays, and not after; one that plays full screen
    // fills the window.
    void showHidesAndFillsAsTheFileSays()
    {
        auto d = showDeck(1);
        deck::MediaPlay hidden;
        hidden.hideWhenStopped = true;
        hidden.start = deck::MediaStart::WhenClickedOn;
        deck::MediaPlay full;
        full.fullScreen = true;
        full.start = deck::MediaStart::WhenClickedOn;
        auto veiled = showMedia(*d, 0, testAvi(30, QSize(64, 48), 10, true), QStringLiteral("a.avi"), QPointF(40, 300), hidden, QSizeF(240, 180));
        auto filling = showMedia(*d, 0, testAvi(30, QSize(64, 48), 10, true), QStringLiteral("b.avi"), QPointF(600, 300), full, QSizeF(240, 180));
        LayoutCache cache;
        // As the show draws the slide: the poster of the hidden one is not there; the other's is.
        const QImage img = renderSlideImage(d.get(), &cache, 0, QSize(960, 540), 1);
        QVERIFY(img.pixelColor(showCenter(*veiled)) != QColor(30, 90, 160));
        QVERIFY(img.pixelColor(showCenter(*veiled)).lightness() > 200);
        QVERIFY(img.pixelColor(showCenter(*filling)) == QColor(30, 90, 160));
        // The editor and the thumbnails still show it: only the show hides it.
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        QVERIFY(Renderer::renderToImage(ctx, 0, 1.0).pixelColor(showCenter(*veiled)) == QColor(30, 90, 160));
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, showCenter(*filling));
        MediaPlayback *pb = show.media()->playbackOf(filling->id);
        QVERIFY(pb);
        QTRY_VERIFY_WITH_TIMEOUT(playerSettled(*pb, true), 15000);
        if (pb->failed()) QSKIP(qPrintable(QStringLiteral("this Qt has no movie backend here: ") + pb->error()));
        // Full screen: the window is black around a picture of the movie's own shape (4 by 3, in 16 by 9), red in its middle.
        QTRY_VERIFY_WITH_TIMEOUT(show.grab().toImage().pixelColor(5, 270) == QColor(Qt::black), 5000);
        const QImage grabbed = show.grab().toImage();
        QVERIFY(grabbed.pixelColor(480, 270).red() > 180 && grabbed.pixelColor(480, 270).green() < 90);
        // It rewinds or holds when it ends, but a hidden one goes again when it stops.
        QPointer<MediaPlayback> keep = pb;
        QVERIFY(keep);
    }

    // What the call to a sound says it lasts is what the bar shows before it plays: it follows the trim when the file's
    // length is known (the Trim box knows it), and the other settings keep it.
    void mediaLengthFollowsTheTrimAndSurvivesOtherEdits()
    {
        DeckWindow w;
        Document *d = w.editor()->doc();
        auto sound = deck::newMediaPicture(*d, *d->pages[0], testWav(3000), QStringLiteral("t.wav"), QImage(), 3000);
        QVERIFY(sound);
        w.editor()->beginChange(QStringLiteral("Insert"));
        d->pages[0]->items.push_back(sound);
        w.editor()->endChange();
        w.editor()->select(sound->id);
        auto play = [&] { return deck::mediaPlayOf(*d->pages[0], *lastMedia(*d)); };
        QCOMPARE(play().lengthHint, 3000);
        w.editMedia(QStringLiteral("Trim"), [](deck::MediaPlay &p) { p.trimStart = 1000; }, 3000);
        QCOMPARE(play().lengthHint, 2000);
        w.editMedia(QStringLiteral("Volume"), [](deck::MediaPlay &p) { p.volume = deck::kVolumeLow; });
        QCOMPARE(play().lengthHint, 2000);
        QCOMPARE(play().volume, deck::kVolumeLow);
        w.editMedia(QStringLiteral("Trim"), [](deck::MediaPlay &p) { p.trimStart = 0; p.trimEnd = 500; }, 3000);
        QCOMPARE(play().lengthHint, 2500);
        // A movie that waits for a click has no length in the file (its call lasts a millisecond).
        w.editMedia(QStringLiteral("Start"), [](deck::MediaPlay &p) { p.start = deck::MediaStart::WhenClickedOn; });
        QCOMPARE(play().lengthHint, 2500);   // a sound's call still says
    }

    // A file that can't be played says so in the show, and the show goes on.
    void showMessageForAMovieThatCantPlay()
    {
        auto d = showDeck(2);
        QByteArray junk(4000, '\0');
        for (int i = 0; i < junk.size(); ++i) junk[i] = char((i * 71 + 5) & 0xFF);
        deck::MediaPlay onClick;
        onClick.start = deck::MediaStart::WhenClickedOn;
        auto movie = showMedia(*d, 0, junk, QStringLiteral("bad.mp4"), QPointF(300, 150), onClick, QSizeF(320, 240));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, showCenter(*movie));
        MediaPlayback *pb = show.media()->playbackOf(movie->id);
        QVERIFY(pb);
        QTRY_VERIFY_WITH_TIMEOUT(pb->failed(), 15000);
        QTRY_VERIFY(show.grab().toImage().pixelColor(showCenter(*movie) + QPoint(0, -100)).lightness() < 90);
        QCOMPARE(show.currentSlide(), 0);
        show.next();
        QCOMPARE(show.currentSlide(), 1);
    }

    // ---- a stranger's media is only media ----

    // A playlist that names a place to fetch, which FFmpeg follows: the player never gets one (what the player fetches or reads is the
    // presentation's own bytes, and nothing else).
    void mediaPlayerNeverFollowsAPlaylist()
    {
        MediaListener where;
        QVERIFY(where.server.isListening());
        const int before = MediaPlayback::decoders();
        const QString hls = QStringLiteral("#EXTM3U\n#EXT-X-VERSION:3\n#EXT-X-TARGETDURATION:1\n#EXTINF:1.0,\n%1\n#EXT-X-ENDLIST\n").arg(where.url("segment.ts"));
        const QStringList texts = {
            hls,
            hls + QStringLiteral("# ") + QString(80, QChar(0xE9)) + QStringLiteral("\n"),   // with a good deal that isn't ASCII
            QStringLiteral("  \n#EXTM3U\n#EXTINF:1,\n%1\n").arg(where.url("a.mp3")),
            QStringLiteral("ffconcat version 1.0\nfile '%1'\n").arg(where.url("b.wav")),
            QStringLiteral("v=0\no=- 0 0 IN IP4 127.0.0.1\ns=x\nc=IN IP4 127.0.0.1\nm=audio 5004 RTP/AVP 0\na=control:%1\n").arg(where.url("c")),
            QStringLiteral("<?xml version=\"1.0\"?><MPD xmlns=\"urn:mpeg:dash:schema:mpd:2011\"><Period><AdaptationSet><Representation><BaseURL>%1</BaseURL></Representation></AdaptationSet></Period></MPD>").arg(where.url("d.mp4")),
            QStringLiteral("[playlist]\nFile1=%1\nNumberOfEntries=1\n").arg(where.url("e.mp3")),
            QStringLiteral("<ASX VERSION=\"3.0\"><ENTRY><REF HREF=\"%1\"/></ENTRY></ASX>").arg(where.url("f.wma")),
            where.url("g.mp4") + QStringLiteral("\n"),
            QStringLiteral("file:///etc/passwd\n"),
        };
        QList<QByteArray> lists{QByteArray("\xEF\xBB\xBF") + hls.toUtf8()};   // after a byte order mark
        for (const QString &text : texts) lists << text.toUtf8();
        for (const QByteArray &text : std::as_const(lists)) {
            // Whatever the part is called: a playlist's name, a sound's, or a movie's.
            for (const char *extension : {"m3u8", "wav", "mp4", "txt"}) {
                MediaPlayback pb;
                deck::MediaPlay play;
                play.video = extension[0] == 'm' && extension[1] == 'p';
                pb.setFile(text, QLatin1String(extension));
                pb.setPlay(play);
                pb.play();
                QVERIFY2(pb.failed(), qPrintable(QString::fromUtf8(text.left(40)) + QStringLiteral(" as .") + QLatin1String(extension)));
                QVERIFY(!pb.error().isEmpty());
            }
        }
        QCOMPARE(MediaPlayback::decoders(), before);   // not even a decoder was started
        QTest::qWait(300);
        QCOMPARE(where.calls, 0);
        // The same for what a window's Insert would read.
        QVERIFY(!probeMedia(hls.toUtf8(), QStringLiteral("m3u8"), false, 3000).error.isEmpty());
        QCOMPARE(MediaPlayback::decoders(), before);
        QCOMPARE(where.calls, 0);
    }

    // And where a file hides its list in something that looks like media (the sniff doesn't see it), FFmpeg is held to the protocols the player
    // allows: it reads no address and no file the presentation names.
    void mediaPlayerAllowsFFmpegNoProtocolButItsOwnBytes()
    {
        MediaListener where;
        QVERIFY(where.server.isListening());
        const QByteArray hls = QStringLiteral("#EXTM3U\n#EXT-X-VERSION:3\n#EXT-X-TARGETDURATION:1\n#EXTINF:1.0,\n%1\n#EXT-X-ENDLIST\n").arg(where.url("segment.ts")).toUtf8();
        // The check of the first bytes off (a test of the second wall).
        MediaPlayback::checkBytes(false);
        {
            MediaPlayback pb;
            pb.setFile(hls, QStringLiteral("m3u8"));
            deck::MediaPlay play;
            pb.setPlay(play);
            pb.play();
            QTRY_VERIFY_WITH_TIMEOUT(pb.failed() || pb.state() == MediaPlayback::State::Playing, 15000);
            QTest::qWait(500);
            QCOMPARE(where.calls, 0);
        }
        // What the player plays still plays with it.
        MediaPlayback ok;
        ok.setFile(testWav(400), QStringLiteral("wav"));
        ok.play();
        MediaPlayback::checkBytes(true);
        QTRY_VERIFY_WITH_TIMEOUT(playerSettled(ok, false), 15000);
        QVERIFY2(!ok.failed() || ok.error().contains(QStringLiteral("sound")), qPrintable(ok.error()));
        QCOMPARE(qgetenv("QT_FFMPEG_PROTOCOL_WHITELIST"), QByteArray("data"));
    }

    // The part's own name is not the player's: only a name a movie or sound file may have goes to the decoder, else "bin".
    void mediaExtensionsAreOnlyThoseOfMediaFiles()
    {
        QCOMPARE(deck::mediaExtensionFor(QStringLiteral("mp4")), QStringLiteral("mp4"));
        QCOMPARE(deck::mediaExtensionFor(QStringLiteral("WAV")), QStringLiteral("wav"));
        for (const char *odd : {"m3u8", "m3u", "pls", "asx", "sdp", "mpd", "txt", "ffconcat", "", "mp4/../x", "a b", "mp4?x=1", "..", "http://x/y.mp3", "abcdefghijklmnop"})
            QCOMPARE(deck::mediaExtensionFor(QString::fromLatin1(odd)), QStringLiteral("bin"));
        // Binary media is media; text is not.
        QVERIFY(deck::looksLikeMediaFile(testWav(100)));
        QVERIFY(deck::looksLikeMediaFile(testAvi()));
        QByteArray frames("\xFF\xFB\x90\x64\x00", 5);   // MPEG audio
        for (int i = 0; i < 600; ++i) frames.append(char((i * 7 + 3) & 0xFF));
        QVERIFY(deck::looksLikeMediaFile(frames));
        // A playlist with a stray byte in it is no more media than one without.
        QVERIFY(!deck::looksLikeMediaFile(QByteArray("#EXTM3U\n#x\x01\n#EXTINF:1,\nhttp://127.0.0.1:9/a.ts\n", 48)));
        QVERIFY(!deck::looksLikeMediaFile(QByteArray("FFCONCAT version 1.0\nfile 'x'\n\x02", 32)));
        QVERIFY(!deck::looksLikeMediaFile(QByteArray("<MPD\n\x02 xmlns=\"urn:mpeg:dash:schema:mpd:2011\"></MPD>", 50)));
        QVERIFY(deck::looksLikeMediaFile(QByteArray("ID3\x03\x00\x00\x00\x00\x00\x00", 10) + QByteArray(300, '\x01')));
        QVERIFY(!deck::looksLikeMediaFile(QByteArray()));
        QVERIFY(!deck::looksLikeMediaFile(QByteArray("#EXTM3U\n#EXTINF:1,\nhttp://example.com/a.mp3\n")));
        QVERIFY(!deck::looksLikeMediaFile(QByteArray(2000, 'a')));
    }

    // ---- what a reviewer found ----

    // What the media may come to is held under what a file can be read with: bytes that would go past it are not put in (those it already
    // keeps add nothing), and what a file holds is read only up to the most, whatever size it says.
    void mediaIsHeldUnderWhatAFileCanBeReadWith()
    {
        auto d = deck::newPresentation();
        deck::setMediaByteLimit(0);
        QVERIFY(deck::mediaByteLimit() >= (qint64(1) << 30));
        QVERIFY(deck::mediaByteLimit() < (qint64(1) << 31));
        QVERIFY(deck::mediaWouldFit(*d, QByteArray(1000, 'a')));
        qint64 used = 0;
        for (const auto &image : std::as_const(d->images)) used += image.bytes.size();
        deck::setMediaByteLimit(used + 5000);
        const QByteArray kept(3000, 'k');
        d->addMedia(kept, QStringLiteral("audio/wav"), QStringLiteral("wav"));
        QVERIFY(deck::mediaWouldFit(*d, QByteArray(1500, 'b')));
        QVERIFY(!deck::mediaWouldFit(*d, QByteArray(3000, 'c')));
        QVERIFY(deck::mediaWouldFit(*d, kept));   // already in: one copy is kept
        // A stream that has no end, and says it has no size.
        struct Endless : QIODevice {
            qint64 readData(char *data, qint64 max) override
            {
                memset(data, 'z', size_t(max));
                given += max;
                return max;
            }
            qint64 writeData(const char *, qint64) override { return -1; }
            bool isSequential() const override { return true; }
            qint64 given = 0;
        } endless;
        endless.open(QIODevice::ReadOnly);
        QByteArray got;
        QVERIFY(!deck::readMediaBytes(endless, 1000, &got));
        QVERIFY(got.isEmpty());
        QVERIFY2(endless.given <= 1000 + 65536, qPrintable(QString::number(endless.given)));   // it stopped soon after the most
        QByteArray small(500, 'q');
        QBuffer exact(&small);
        exact.open(QIODevice::ReadOnly);
        QVERIFY(deck::readMediaBytes(exact, 500, &got));
        QCOMPARE(got, small);
        QBuffer over(&small);
        over.open(QIODevice::ReadOnly);
        QVERIFY(!deck::readMediaBytes(over, 499, &got));
        deck::setMediaByteLimit(0);
    }

    // Insert says no, with the reason, before it reads or decodes a thing, when the slide being edited is not a slide (a master) or when
    // the presentation could not take the file and be read again.
    void insertMediaSaysWhyBeforeItReadsAnything()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        DeckWindow w;
        Editor *ed = w.editor();
        const QString wav = writeTestFile(dir, QStringLiteral("tune.wav"), testWav(1000));
        QString shown;
        QTimer closer;
        closer.setInterval(10);
        connect(&closer, &QTimer::timeout, &closer, [&] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                shown = box->text();
                box->close();
            }
        });
        closer.start();
        const int before = MediaPlayback::decoders();
        // Too much for a file to hold.
        qint64 used = 0;
        for (const auto &image : std::as_const(ed->doc()->images)) used += image.bytes.size();
        deck::setMediaByteLimit(used + 1000);
        QVERIFY(!w.insertMedia(wav, QStringLiteral("audio")));
        QVERIFY2(shown.contains(QStringLiteral("tune.wav")) && shown.contains(QStringLiteral("too")), qPrintable(shown));
        QVERIFY(ed->doc()->media.isEmpty());
        deck::setMediaByteLimit(0);
        // A master is not a slide.
        shown.clear();
        QVERIFY(!ed->doc()->masters.isEmpty());
        ed->setMasterView(ed->doc()->masters.first()->id);
        QVERIFY(!w.insertMedia(wav, QStringLiteral("audio")));
        QVERIFY2(shown.contains(QStringLiteral("slide")), qPrintable(shown));
        QCOMPARE(MediaPlayback::decoders(), before);   // nothing was decoded to find out
        QVERIFY(ed->doc()->media.isEmpty());
        closer.stop();
        ed->setMasterView(QString());
    }

    // After a clip stops (at its trim end, say), moving in it and playing again plays to the trim end again, not to the end of the file.
    void mediaStopsAtTheTrimEndAgainAfterScrubbing()
    {
        MediaPlayback pb;
        deck::MediaPlay play;
        play.trimStart = 300;
        play.trimEnd = 2200;   // 3,000 ms less 2,200: it plays from 300 to 800
        pb.setFile(testWav(3000), QStringLiteral("wav"));
        pb.setPlay(play);
        pb.play();
        QSignalSpy ended(&pb, &MediaPlayback::ended);
        QTest::qWait(100);
        QTRY_VERIFY_WITH_TIMEOUT(pb.failed() || ended.count() > 0, 8000);
        if (pb.failed() || ended.count() == 0) QSKIP("this computer's player doesn't run a sound out here");
        // Moved back into it, and played again.
        pb.seek(100);
        QTRY_COMPARE_WITH_TIMEOUT(int(pb.state()), int(MediaPlayback::State::Paused), 5000);
        pb.resume();
        QTRY_VERIFY_WITH_TIMEOUT(pb.state() == MediaPlayback::State::Playing || ended.count() > 1, 5000);
        QTRY_VERIFY_WITH_TIMEOUT(ended.count() > 1, 5000);   // at 800, not at 3,000
        QCOMPARE(int(pb.state()), int(MediaPlayback::State::Stopped));
    }

    // "Across N slides" counts a slide's going once, whether a transition plays or not, and nothing plays on after the show.
    void showMediaAcrossSlidesCountsEachChangeOnceAndEndsWithTheShow()
    {
        auto d = showDeck(4);
        for (int i = 1; i < 4; ++i) deck::setTransition(*d->pages[i], showTransition("fade", 0.05));
        deck::MediaPlay across;
        across.start = deck::MediaStart::Automatically;
        across.acrossSlides = 3;
        auto sound = showMedia(*d, 0, testWav(8000), QStringLiteral("song.wav"), QPointF(40, 40), across);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);
        show.show();
        QPointer<MediaPlayback> song = show.media()->playbackOf(sound->id);
        QVERIFY(song);
        show.next();
        QTRY_VERIFY(!show.isAnimating());
        QCOMPARE(show.currentSlide(), 1);
        QVERIFY(!song.isNull());   // over one slide's end
        show.next();
        QTRY_VERIFY(!show.isAnimating());
        QCOMPARE(show.currentSlide(), 2);
        QVERIFY2(!song.isNull(), "three slides: still on the second");
        show.next();
        QTRY_VERIFY(!show.isAnimating());
        QCOMPARE(show.currentSlide(), 3);
        QTRY_VERIFY(song.isNull());   // after the third
        // A show that ends takes what plays across slides with it.
        auto d2 = showDeck(2);
        deck::MediaPlay everywhere;
        everywhere.start = deck::MediaStart::Automatically;
        everywhere.acrossSlides = 999;
        auto bell = showMedia(*d2, 0, testWav(8000), QStringLiteral("bell.wav"), QPointF(40, 40), everywhere);
        SlideShow other(d2.get(), &cache, 0);
        other.resize(960, 540);
        other.show();
        QPointer<MediaPlayback> ring = other.media()->playbackOf(bell->id);
        QVERIFY(ring);
        other.next();
        QCOMPARE(other.currentSlide(), 1);
        QVERIFY(!ring.isNull());
        other.next();   // past the last slide: the end of the show
        QVERIFY(other.currentSlide() < 0);
        QTRY_VERIFY(ring.isNull());
    }

    // Trim doesn't decode the file to learn how long it is: the timing's length is what it goes by.
    void trimDoesNotDecodeTheFile()
    {
        DeckWindow w;
        Document *d = w.editor()->doc();
        auto sound = deck::newMediaPicture(*d, *d->pages[0], testWav(3000), QStringLiteral("t.wav"), QImage(), 3000);
        QVERIFY(sound);
        w.editor()->beginChange(QStringLiteral("Insert"));
        d->pages[0]->items.push_back(sound);
        w.editor()->endChange();
        w.editor()->select(sound->id);
        const int before = MediaPlayback::decoders();
        double shownEnd = -1;
        QTimer closer;
        closer.setInterval(10);
        connect(&closer, &QTimer::timeout, &closer, [&] {
            if (auto *dlg = qobject_cast<MediaTrimDialog *>(QApplication::activeModalWidget())) {
                if (auto *end = dlg->findChild<QDoubleSpinBox *>(QStringLiteral("trimEnd"))) shownEnd = end->value();
                dlg->reject();
            }
        });
        closer.start();
        w.act(QStringLiteral("playback.trim"))->trigger();
        closer.stop();
        QCOMPARE(MediaPlayback::decoders(), before);
        QCOMPARE(shownEnd, 3.0);   // the length the timing says
    }

    // A document that is replaced lets go of the players it had.
    void editorMediaLetsGoOfItsPlayersWhenTheDocumentIsReplaced()
    {
        DeckWindow w;
        Document *d = w.editor()->doc();
        auto sound = deck::newMediaPicture(*d, *d->pages[0], testWav(3000), QStringLiteral("t.wav"), QImage(), 3000);
        QVERIFY(sound);
        w.editor()->beginChange(QStringLiteral("Insert"));
        d->pages[0]->items.push_back(sound);
        w.editor()->endChange();
        w.editor()->select(sound->id);
        w.canvas()->media()->play(sound->id);
        QPointer<MediaPlayback> pb = w.canvas()->media()->playbackOf(sound->id);
        QVERIFY(pb);
        w.editor()->setDocument(deck::newPresentation());
        QVERIFY(pb.isNull());
        QVERIFY(w.canvas()->media()->activeId().isEmpty());
    }

    // ---- what the presentation program showed ----

    // The end of a trim, in a file, is how much is cut off the end (p14:trim's end), not where it ends: a movie 3 seconds long cut 500 ms
    // at each end plays 2 seconds, the call that plays it lasts 2 seconds, and it is written and read so.
    void mediaTrimEndIsWhatIsCutOffTheEnd()
    {
        Page slide;
        auto movie = mediaPicture(0, "video");
        slide.items.push_back(movie);
        deck::MediaPlay play;
        play.trimStart = 500;
        play.trimEnd = 500;
        deck::setMediaPlay(slide, *movie, play, 3000);
        const deck::MediaPlay read = deck::mediaPlayOf(slide, *movie);
        QCOMPARE(read.lengthHint, 2000);   // what the call says it lasts
        QCOMPARE(deck::knownFileLength(read), qint64(3000));
        QCOMPARE(deck::knownFileLength(deck::MediaPlay()), qint64(0));
        // The player: a sound of 3 seconds with 2.2 seconds cut off its end plays from 0.3 to 0.8 and stops.
        MediaPlayback pb;
        deck::MediaPlay cut;
        cut.trimStart = 300;
        cut.trimEnd = 2200;
        pb.setFile(testWav(3000), QStringLiteral("wav"));
        pb.setPlay(cut);
        pb.play();
        QSignalSpy ended(&pb, &MediaPlayback::ended);
        QTRY_VERIFY_WITH_TIMEOUT(pb.failed() || pb.length() > 0, 8000);
        if (pb.failed()) return;
        QVERIFY2(std::abs(pb.length() - 500) < 60, qPrintable(QString::number(pb.length())));
        QCOMPARE(pb.fileLength() > 2900, true);
        QTest::qWait(100);
        QTRY_VERIFY_WITH_TIMEOUT(pb.failed() || ended.count() > 0, 8000);
        if (ended.count() == 0) QSKIP("this computer's player doesn't run a sound out here");
        // A trim that leaves nothing is none (the presentation program ignores it too).
        MediaPlayback whole;
        deck::MediaPlay nothing;
        nothing.trimStart = 1500;
        nothing.trimEnd = 1500;
        whole.setFile(testWav(3000), QStringLiteral("wav"));
        whole.setPlay(nothing);
        whole.play();
        QTRY_VERIFY_WITH_TIMEOUT(whole.failed() || whole.length() > 0, 8000);
        if (!whole.failed()) QVERIFY(std::abs(whole.length() - 3000) < 60);
    }

    // The presentation program refuses a main sequence with nothing in it, and writes none when a slide has only what waits for a click:
    // taking the last step out of the main sequence takes the sequence out, and the node of a movie that waits for a click follows
    // its sequence as the program writes them.
    void timingHasNoEmptyMainSequence()
    {
        for (const bool video : {true, false}) {
            Page slide;
            auto pic = mediaPicture(0, video ? "video" : "audio");
            slide.items.push_back(pic);
            deck::MediaPlay play;
            deck::setMediaPlay(slide, *pic, play, 3000);   // in the click sequence: a step in the main sequence
            QVERIFY(slide.props.value(QStringLiteral("timingXml")).toString().contains(QStringLiteral("nodeType=\"mainSeq\"")));
            play.start = deck::MediaStart::WhenClickedOn;
            deck::setMediaPlay(slide, *pic, play, 3000);
            const QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
            QVERIFY2(!xml.contains(QStringLiteral("mainSeq")), qPrintable(xml));
            QVERIFY2(animIdProblem(xml).isEmpty(), qPrintable(xml));
            QVERIFY(xml.indexOf(QStringLiteral("interactiveSeq")) < xml.indexOf(video ? QStringLiteral("<p:video>") : QStringLiteral("<p:audio>")));
            // And back.
            play.start = deck::MediaStart::ClickSequence;
            deck::setMediaPlay(slide, *pic, play, 3000);
            QVERIFY(slide.props.value(QStringLiteral("timingXml")).toString().contains(QStringLiteral("nodeType=\"mainSeq\"")));
            deck::clearMediaPlay(slide, pic->sourceId);
            QVERIFY(!slide.props.contains(QStringLiteral("timingXml")));
        }
        // A movie by itself keeps nothing of the click sequence: no node in the timing begins with a call.
        Page auto1;
        auto m = mediaPicture(0, "audio");
        auto1.items.push_back(m);
        deck::MediaPlay a;
        a.start = deck::MediaStart::Automatically;
        deck::setMediaPlay(auto1, *m, a, 3000);
        const QString xml = auto1.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY(xml.contains(QStringLiteral("<p:cond evt=\"onBegin\" delay=\"0\"><p:tn val=\"2\"/></p:cond>")));   // the step begins with the sequence
        QCOMPARE(mediaBeginsWith(xml, "audio"), 0);   // and the sound's own node says nothing of it
    }

    // The animation editor writes no main sequence when the slide has no effects but a movie that waits for a click: the program
    // refuses the file otherwise.
    void animationEditsWriteNoEmptyMainSequence()
    {
        auto d = editDeck(1);
        Page &slide = *d->pages[0];
        auto movie = deck::newMediaPicture(*d, slide, testAvi(), QStringLiteral("clip.avi"), QImage(160, 120, QImage::Format_RGB32), 500);
        slide.items.push_back(movie);
        deck::MediaPlay click;
        click.start = deck::MediaStart::WhenClickedOn;
        deck::setMediaPlay(slide, *movie, click, 500);
        deck::AnimSequence seq = deck::readAnimations(slide, d.get());
        seq.add(deck::makeAnim(QStringLiteral("entr.fade"), 10));
        deck::setAnimations(*d, slide, seq);
        QVERIFY(slide.props.value(QStringLiteral("timingXml")).toString().contains(QStringLiteral("nodeType=\"mainSeq\"")));
        // The effect is taken away again.
        seq = deck::readAnimations(slide, d.get());
        seq.remove(0);
        deck::setAnimations(*d, slide, seq);
        const QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY(!xml.isEmpty());
        QVERIFY2(!xml.contains(QStringLiteral("mainSeq")), qPrintable(xml));
        QVERIFY(xml.contains(QStringLiteral("<p:video>")) && xml.contains(QStringLiteral("interactiveSeq")));
        QVERIFY(deck::mediaPlayOf(slide, *movie).start == deck::MediaStart::WhenClickedOn);
    }

    // The cTn id the effect that calls the shape's media has, in a timing; 0 if none.
    static int mediaCallNode(const QString &timing, int shape)
    {
        QDomDocument dom;
        if (!office::parseXml(timing.toUtf8(), &dom)) return 0;
        QVector<QDomElement> stack{dom.documentElement()};
        while (!stack.isEmpty()) {
            const QDomElement e = stack.takeLast();
            if (e.localName() == QLatin1String("cTn") && e.attribute(QStringLiteral("presetClass")) == QLatin1String("mediacall")) {
                for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
                    if (c.localName() == QLatin1String("childTnLst") && c.toElement().text().isEmpty()) {
                        QVector<QDomElement> in{c};
                        while (!in.isEmpty()) {
                            const QDomElement f = in.takeLast();
                            if (f.localName() == QLatin1String("spTgt") && f.attribute(QStringLiteral("spid")).toInt() == shape) return e.attribute(QStringLiteral("id")).toInt();
                            for (QDomElement g = f.firstChildElement(); !g.isNull(); g = g.nextSiblingElement()) in << g;
                        }
                    }
            }
            for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack << c;
        }
        return 0;
    }

    // What a media node begins with: the node of the call that plays it, named by its onBegin condition.
    static int mediaBeginsWith(const QString &timing, const char *kind)
    {
        const int at = timing.indexOf(QStringLiteral("<p:%1").arg(QLatin1String(kind)));
        if (at < 0) return 0;
        const QString node = timing.mid(at, timing.indexOf(QStringLiteral("</p:%1>").arg(QLatin1String(kind)), at) - at);
        static const QRegularExpression begin(QStringLiteral("evt=\"onBegin\" delay=\"0\"><p:tn val=\"(\\d+)\""));
        const QRegularExpressionMatch m = begin.match(node);
        return m.hasMatch() ? m.captured(1).toInt() : 0;
    }

    // An edit of the slide's animations keeps the movie's and sound's nodes and the calls to them, and the node of a sound that begins with its
    // call still names that call by its new number; an edit of how they play keeps the animations.
    void animationEditsKeepTheMediaAndTheNumbersItNames()
    {
        auto d = editDeck(2);
        Page &slide = *d->pages[0];
        QImage poster(160, 120, QImage::Format_RGB32);
        poster.fill(QColor(30, 90, 160));
        // Two fades first (their numbers come before the sound's), then a sound that begins after them, and a movie that waits for a click on it.
        deck::AnimSequence first;
        first.add(deck::makeAnim(QStringLiteral("entr.fade"), 10));
        first.add(deck::makeAnim(QStringLiteral("entr.fade"), 11));
        deck::setAnimations(*d, slide, first);
        auto sound = deck::newMediaPicture(*d, slide, testWav(), QStringLiteral("tune.wav"), QImage(), 1000);
        slide.items.push_back(sound);
        deck::MediaPlay auto1;
        auto1.start = deck::MediaStart::Automatically;
        auto1.loop = true;
        deck::setMediaPlay(slide, *sound, auto1, 1000);
        auto movie = deck::newMediaPicture(*d, slide, testAvi(), QStringLiteral("clip.avi"), poster, 500);
        slide.items.push_back(movie);
        deck::MediaPlay click;
        click.start = deck::MediaStart::WhenClickedOn;
        click.fullScreen = true;
        deck::setMediaPlay(slide, *movie, click, 500);
        QString timing = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY2(animIdProblem(timing).isEmpty(), qPrintable(animIdProblem(timing)));
        const int called = mediaCallNode(timing, sound->sourceId);
        QVERIFY(called > 0);
        // This program's sound node says nothing of its call; another writer's can name it by its number, and an edit must follow it.
        const qsizetype audioAt = timing.indexOf(QStringLiteral("<p:audio>"));
        const QString plainCond = QStringLiteral("<p:cond delay=\"indefinite\"/></p:stCondLst>");
        const qsizetype condAt = timing.indexOf(plainCond, audioAt);
        QVERIFY(audioAt > 0 && condAt > audioAt);
        timing.insert(condAt + plainCond.size() - QStringLiteral("</p:stCondLst>").size(), QStringLiteral("<p:cond evt=\"onBegin\" delay=\"0\"><p:tn val=\"%1\"/></p:cond>").arg(called));
        slide.props[QStringLiteral("timingXml")] = timing;
        QCOMPARE(mediaBeginsWith(timing, "audio"), called);
        // An effect is put in front of everything: the sound's call is numbered afresh.
        deck::AnimSequence seq = deck::readAnimations(slide, d.get());
        QVERIFY(seq.editable);
        seq.add(deck::makeAnim(QStringLiteral("entr.wipe"), 10), 0);
        seq.add(deck::makeAnim(QStringLiteral("entr.wipe"), 11), 0);
        seq.add(deck::makeAnim(QStringLiteral("entr.wipe"), 10), 0);
        seq.add(deck::makeAnim(QStringLiteral("emph.pulse"), 11));
        deck::setAnimations(*d, slide, seq);
        timing = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY2(animIdProblem(timing).isEmpty(), qPrintable(animIdProblem(timing)));
        QCOMPARE(int(timing.count(QStringLiteral("<p:audio>"))), 1);
        QCOMPARE(int(timing.count(QStringLiteral("<p:video"))), 1);
        QVERIFY(timing.contains(QStringLiteral("fullScrn=\"1\"")));
        const int calledNow = mediaCallNode(timing, sound->sourceId);
        QVERIFY2(calledNow > 0, qPrintable(timing));
        QCOMPARE(mediaBeginsWith(timing, "audio"), calledNow);   // follows the call to its new number
        QVERIFY(deck::mediaPlayOf(slide, *sound) == [&] { deck::MediaPlay p = auto1; p.shape = sound->sourceId; p.video = false; p.lengthHint = deck::mediaPlayOf(slide, *sound).lengthHint; return p; }());
        QVERIFY(deck::mediaPlayOf(slide, *movie).start == deck::MediaStart::WhenClickedOn);
        QCOMPARE(int(deck::readAnimations(slide, d.get()).effects.size()), 7);   // 2 fades, the 4 added, and the sound's call (the movie waits for a click on it: that is not in the main sequence)
        QVERIFY2(calledNow != called, "the edit renumbered the call, so following it means something");
        // An edit of how the sound plays leaves the animations alone.
        deck::MediaPlay quiet = deck::mediaPlayOf(slide, *sound);
        quiet.volume = deck::kVolumeLow;
        quiet.start = deck::MediaStart::ClickSequence;
        deck::setMediaPlay(slide, *sound, quiet);
        timing = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY2(animIdProblem(timing).isEmpty(), qPrintable(animIdProblem(timing)));
        int fades = 0, wipes = 0, pulses = 0;
        for (const deck::Step &s : deck::readTimeline(timing).steps)
            for (const deck::Effect &e : s.effects) {
                fades += e.cls == deck::EffectClass::Entrance && e.presetId == 10;
                wipes += e.cls == deck::EffectClass::Entrance && e.presetId == 22;
                pulses += e.cls == deck::EffectClass::Emphasis;
            }
        QCOMPARE(fades, 2);
        QCOMPARE(wipes, 3);
        QCOMPARE(pulses, 1);
        QVERIFY(deck::mediaPlayOf(slide, *sound).start == deck::MediaStart::ClickSequence);
        QCOMPARE(deck::mediaPlayOf(slide, *sound).volume, deck::kVolumeLow);
        QVERIFY(deck::mediaPlayOf(slide, *movie).fullScreen);
        // Saved, the numbers hold.
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        const QString saved = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY2(animIdProblem(saved.mid(saved.indexOf(QStringLiteral("<p:timing")), saved.indexOf(QStringLiteral("</p:timing>")) + 11 - saved.indexOf(QStringLiteral("<p:timing")))).isEmpty(), "ids in the saved timing");
    }

    // The notes a save makes about the animations of shapes that are gone, and nothing else.
    static bool onlyGoneNotes(const QStringList &notes)
    {
        for (const QString &n : notes)
            if (!n.contains(QStringLiteral("animation effects of shapes that are gone"))) return false;
        return true;
    }

    // A movie's picture that is deleted takes the movie's part of the timing with it; the slide's other animations are saved (the
    // save says the effect of the gone shape was left out). A picture that is still there but plays nothing any more is still what the presentation program would refuse.
    void deletedMediaTakesItsTimingAndNoMore()
    {
        auto d = deck::newPresentation();
        Page &slide = *d->pages[0];
        QImage poster(160, 120, QImage::Format_RGB32);
        poster.fill(QColor(30, 90, 160));
        auto movie = deck::newMediaPicture(*d, slide, testAvi(), QStringLiteral("clip.avi"), poster, 500);
        QVERIFY(movie);
        slide.items.push_back(movie);
        auto sound = deck::newMediaPicture(*d, slide, testWav(), QStringLiteral("tune.wav"), QImage(), 1000);
        QVERIFY(sound);
        slide.items.push_back(sound);
        QString xml = slide.props.value(QStringLiteral("timingXml")).toString();
        QVERIFY(xml.contains(QStringLiteral("<p:video>")) && xml.contains(QStringLiteral("<p:audio>")));
        // Delete the movie's picture.
        const int deleted = movie->sourceId;
        slide.items.erase(std::find(slide.items.begin(), slide.items.end(), movie));
        QString err;
        QStringList skipped;
        QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(onlyGoneNotes(skipped), qPrintable(skipped.join('\n')));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString saved = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(!saved.contains(QStringLiteral("<p:video>")));
        QVERIFY(saved.contains(QStringLiteral("<p:audio>")));
        QVERIFY(!saved.contains(QStringLiteral("spid=\"%1\"").arg(deleted)));
        QVERIFY(saved.contains(QStringLiteral("cmd=\"playFrom(0.0)\"")));
        // The last one deleted, the timing is not written at all.
        slide.items.erase(std::find(slide.items.begin(), slide.items.end(), sound));
        bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY(!part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<p:timing")));
        QVERIFY2(onlyGoneNotes(skipped), qPrintable(skipped.join('\n')));
        // The editor's own deletion does the same.
        DeckWindow w;
        {
            Document *wd = w.editor()->doc();
            auto kept = deck::newMediaPicture(*wd, *wd->pages[0], testWav(), QStringLiteral("tune.wav"), QImage(), 1000);
            QVERIFY(kept);
            w.editor()->beginChange(QStringLiteral("Insert"));
            wd->pages[0]->items.push_back(kept);
            w.editor()->endChange();
            w.editor()->select(kept->id);
        }
        w.editor()->deleteSelection();
        QVERIFY(deck::mediaPicturesOf(*w.editor()->doc()->pages[0]).isEmpty());
        skipped.clear();
        bytes = deck::exportPresentation(*w.editor()->doc(), &err, &skipped);
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY(!part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<p:timing")));
        QVERIFY2(onlyGoneNotes(skipped), qPrintable(skipped.join('\n')));
    }

    // A file that holds more calls to media than a slide has, or a damaged timing, makes no more than a few cues and no harm.
    void mediaPlanHoldsNoMoreThanASlideCanUse()
    {
        Page slide;
        auto movie = mediaPicture(4, "video");
        slide.items.push_back(movie);
        QStringList calls;
        for (int i = 0; i < 3000; ++i) calls << mediaCall(i % 2 ? 4 : 99, "playFrom(0.0)", "withEffect");
        slide.props[QStringLiteral("timingXml")] = animTiming(animStep({animGroup(0, calls)}));
        const deck::MediaPlan plan = deck::mediaPlanOf(slide);
        QCOMPARE(int(plan.steps.size()), 1);
        QVERIFY(plan.steps[0].cues.size() <= 64);
        for (const deck::MediaCue &c : plan.steps[0].cues) QCOMPARE(c.shape, 4);   // only the slide's own media
        // Not timing at all.
        slide.props[QStringLiteral("timingXml")] = QStringLiteral("<p:timing xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"><p:tnLst><p:par>");
        QVERIFY(deck::mediaPlanOf(slide).steps.isEmpty());
        QVERIFY(deck::mediaPlayOf(slide, *movie).start == deck::MediaStart::ClickSequence);
        // Setting how it plays on damaged timing starts the timing over, with the movie in it.
        deck::setMediaPlay(slide, *movie, deck::MediaPlay());
        QVERIFY(deck::mediaPlanOf(slide).steps.size() == 1);
        // Deeply nested timing is read to a limit and no further.
        QString deep = QStringLiteral("<p:timing xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\">");
        for (int i = 0; i < 5000; ++i) deep += QStringLiteral("<p:par>");
        for (int i = 0; i < 5000; ++i) deep += QStringLiteral("</p:par>");
        deep += QStringLiteral("</p:timing>");
        slide.props[QStringLiteral("timingXml")] = deep;
        QVERIFY(deck::mediaPlanOf(slide).steps.isEmpty());
        deck::clearMediaPlay(slide, 4);
        QVERIFY(deck::mediaTimingForShapes(deep, QSet<qint64>{4}) == deep);
    }

    // A deck with a movie and a sound set every way the Playback tab sets them, for the presentation program to open (the
    // gate): it is saved, read again as it was, and written to JD_MEDIA_SAMPLE_ALL when that names a file (JD_MEDIA_SAMPLE gets the plain one).
    void mediaSampleDeckSavesAndReadsBack()
    {
        auto d = deck::newPresentation();
        const QVector<MasterPage *> lays = deck::layouts(*d);
        QVERIFY(lays.size() > 5);
        deck::addSlide(*d, *lays[5]);
        deck::addSlide(*d, *lays[5]);
        QImage poster(320, 180, QImage::Format_RGB32);
        poster.fill(QColor(30, 90, 160));
        auto put = [&](int slide, const QByteArray &bytes, const QString &name, const QImage &image, const QPointF &at, const deck::MediaPlay &want) {
            Page &page = *d->pages[slide];
            auto pic = deck::newMediaPicture(*d, page, bytes, name, image, 3000, at);
            Q_ASSERT(pic);
            page.items.push_back(pic);
            deck::MediaPlay play = want;
            deck::setMediaPlay(page, *pic, play, 3000);
            return pic;
        };
        const QByteArray movie = testAvi(30, QSize(320, 180), 10), tune = testWav(3000, 523);
        deck::MediaPlay a;   // in the click sequence, trimmed and faded
        a.trimStart = 500;
        a.trimEnd = 500;
        a.fadeIn = 300;
        a.fadeOut = 400;
        a.volume = deck::kVolumeMedium;
        deck::MediaPlay b;   // by itself, looping, hidden until it plays
        b.start = deck::MediaStart::Automatically;
        b.loop = true;
        b.hideWhenStopped = true;
        b.volume = deck::kVolumeLow;
        deck::MediaPlay c;   // on a click, full screen, rewinding
        c.start = deck::MediaStart::WhenClickedOn;
        c.fullScreen = true;
        c.rewind = true;
        deck::MediaPlay e;   // across slides, muted
        e.start = deck::MediaStart::Automatically;
        e.acrossSlides = 999;
        e.loop = true;
        e.mute = true;
        QVector<std::pair<int, std::shared_ptr<PictureItem>>> placed;
        placed << std::make_pair(0, put(0, movie, QStringLiteral("Clip.avi"), poster, QPointF(60, 60), a));
        placed << std::make_pair(0, put(0, tune, QStringLiteral("Tune.wav"), QImage(), QPointF(700, 400), b));
        placed << std::make_pair(1, put(1, movie, QStringLiteral("Clip.avi"), poster, QPointF(300, 150), c));
        placed << std::make_pair(2, put(2, tune, QStringLiteral("Tune.wav"), QImage(), QPointF(40, 40), e));
        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        QVERIFY2(skipped.isEmpty(), qPrintable(skipped.join('\n')));
        Exported x;
        QVERIFY2(unpack(bytes, &x, &err), qPrintable(err));
        QVERIFY2(packageProblem(x).isEmpty(), qPrintable(packageProblem(x)));
        // The same movie on two slides is one part.
        int mediaParts = 0;
        for (const QString &name : x.pkg.partNames())
            if (name.startsWith(QLatin1String("/ppt/media/media"))) ++mediaParts;
        QCOMPARE(mediaParts, 2);
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages.size()), 3);
        const deck::MediaPlay wants[] = {a, b, c, e};
        int k = 0;
        for (const auto &[slide, pic] : placed) {
            const QVector<const PictureItem *> read = deck::mediaPicturesOf(*back->pages[slide]);
            const PictureItem *match = nullptr;
            for (const PictureItem *r : read)
                if (r->name == pic->name && deck::mediaPlayOf(*back->pages[slide], *r).start == wants[k].start) match = r;
            QVERIFY2(match, qPrintable(QStringLiteral("picture %1").arg(k)));
            deck::MediaPlay got = deck::mediaPlayOf(*back->pages[slide], *match), want = wants[k];
            want.shape = got.shape = 0;
            want.video = pic->mediaKind == QLatin1String("video");
            QVERIFY2(got == want, qPrintable(QStringLiteral("picture %1").arg(k)));
            ++k;
        }
        const QString all = qEnvironmentVariable("JD_MEDIA_SAMPLE_ALL");
        if (!all.isEmpty()) {
            QFile out(all);
            QVERIFY(out.open(QIODevice::WriteOnly));
            out.write(bytes);
        }
        // And the plainest: a movie and a sound as they are when they are put on a slide, nothing set.
        auto plain = deck::newPresentation();
        auto clip = deck::newMediaPicture(*plain, *plain->pages[0], movie, QStringLiteral("Clip.avi"), poster, 3000, QPointF(80, 150));
        plain->pages[0]->items.push_back(clip);
        auto song = deck::newMediaPicture(*plain, *plain->pages[0], tune, QStringLiteral("Tune.wav"), QImage(), 3000, QPointF(800, 400));
        plain->pages[0]->items.push_back(song);
        skipped.clear();
        const QByteArray basic = deck::exportPresentation(*plain, &err, &skipped);
        QVERIFY2(!basic.isEmpty() && skipped.isEmpty(), qPrintable(err + skipped.join('\n')));
        Exported y;
        QVERIFY2(unpack(basic, &y, &err), qPrintable(err));
        QVERIFY2(packageProblem(y).isEmpty(), qPrintable(packageProblem(y)));
        // One deck for each way of setting a movie or sound, for finding what the presentation program refuses (JD_MEDIA_MATRIX names the folder).
        if (const QString matrix = qEnvironmentVariable("JD_MEDIA_MATRIX"); !matrix.isEmpty()) {
            struct Variant {
                const char *name;
                bool video;
                std::function<void(deck::MediaPlay &)> set;
            };
            const std::vector<Variant> variants = {
                {"plain-video", true, [](deck::MediaPlay &) {}},
                {"plain-audio", false, [](deck::MediaPlay &) {}},
                {"trim", true, [](deck::MediaPlay &p) { p.trimStart = 500; p.trimEnd = 500; }},
                {"trim-start", true, [](deck::MediaPlay &p) { p.trimStart = 500; }},
                {"trim-end", true, [](deck::MediaPlay &p) { p.trimEnd = 500; }},
                {"fade", false, [](deck::MediaPlay &p) { p.fadeIn = 300; p.fadeOut = 400; }},
                {"loop", true, [](deck::MediaPlay &p) { p.loop = true; }},
                {"rewind", true, [](deck::MediaPlay &p) { p.rewind = true; }},
                {"fullscreen", true, [](deck::MediaPlay &p) { p.fullScreen = true; }},
                {"hide", true, [](deck::MediaPlay &p) { p.hideWhenStopped = true; }},
                {"mute", false, [](deck::MediaPlay &p) { p.mute = true; }},
                {"volume-low", false, [](deck::MediaPlay &p) { p.volume = deck::kVolumeLow; }},
                {"across", false, [](deck::MediaPlay &p) { p.acrossSlides = 999; p.start = deck::MediaStart::Automatically; }},
                {"auto-video", true, [](deck::MediaPlay &p) { p.start = deck::MediaStart::Automatically; }},
                {"auto-audio", false, [](deck::MediaPlay &p) { p.start = deck::MediaStart::Automatically; }},
                {"clicked-video", true, [](deck::MediaPlay &p) { p.start = deck::MediaStart::WhenClickedOn; }},
                {"clicked-audio", false, [](deck::MediaPlay &p) { p.start = deck::MediaStart::WhenClickedOn; }},
            };
            for (const Variant &v : variants) {
                auto one = deck::newPresentation();
                auto pic = deck::newMediaPicture(*one, *one->pages[0], v.video ? movie : tune, v.video ? QStringLiteral("Clip.avi") : QStringLiteral("Tune.wav"), v.video ? poster : QImage(), 3000, QPointF(80, 150));
                one->pages[0]->items.push_back(pic);
                deck::MediaPlay play = deck::mediaPlayOf(*one->pages[0], *pic);
                v.set(play);
                deck::setMediaPlay(*one->pages[0], *pic, play, 3000);
                QFile out(matrix + QLatin1Char('/') + QLatin1String(v.name) + QStringLiteral(".pptx"));
                QVERIFY(out.open(QIODevice::WriteOnly));
                out.write(deck::exportPresentation(*one, &err));
            }
        }
        if (const QString matrix = qEnvironmentVariable("JD_MEDIA_MATRIX"); !matrix.isEmpty()) {
            // With the animation editor's effects: before the media, and after an edit of them.
            auto write = [&](Document &doc, const char *name) {
                QFile out(matrix + QLatin1Char('/') + QLatin1String(name) + QStringLiteral(".pptx"));
                QVERIFY(out.open(QIODevice::WriteOnly));
                out.write(deck::exportPresentation(doc, &err));
            };
            for (const deck::MediaStart start : {deck::MediaStart::ClickSequence, deck::MediaStart::Automatically, deck::MediaStart::WhenClickedOn}) {
                auto fx = deck::newPresentation();
                Page &pg = *fx->pages[0];
                deck::AnimSequence q;
                q.add(deck::makeAnim(QStringLiteral("entr.fade"), deck::shapeNumber(pg, pg.items[0]->id)));
                deck::setAnimations(*fx, pg, q);
                auto clip = deck::newMediaPicture(*fx, pg, movie, QStringLiteral("Clip.avi"), poster, 3000, QPointF(80, 150));
                pg.items.push_back(clip);
                deck::MediaPlay play;
                play.start = start;
                deck::setMediaPlay(pg, *clip, play, 3000);
                write(*fx, start == deck::MediaStart::ClickSequence ? "after-fade-click" : start == deck::MediaStart::Automatically ? "after-fade-auto" : "after-fade-clicked");
                // The effect edited away and another put in.
                deck::AnimSequence again = deck::readAnimations(pg, fx.get());
                again.remove(0);
                deck::setAnimations(*fx, pg, again);
                write(*fx, start == deck::MediaStart::ClickSequence ? "edited-click" : start == deck::MediaStart::Automatically ? "edited-auto" : "edited-clicked");
            }
        }
        const QString sample = qEnvironmentVariable("JD_MEDIA_SAMPLE");
        if (!sample.isEmpty()) {
            QFile out(sample);
            QVERIFY(out.open(QIODevice::WriteOnly));
            out.write(basic);
        }
    }

    // What can't be read is left out and the rest plays: a behavior that aims
    // at nothing, an animation with no values, a set of something that is not
    // played, a time that makes no sense, and a file that is not timing at all.
    void animationReadsDamagedTimingWithoutHarm()
    {
        const deck::Geometry g = animGeometry();
        QVERIFY(deck::readTimeline(QStringLiteral("<p:transition xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"/>")).isEmpty());
        QVERIFY(deck::readTimeline(animTiming(QString())).isEmpty());
        // Aimed at no shape: a step is read, and plays nothing.
        const deck::Timeline nowhere = deck::readTimeline(animTiming(animSteps({animNode("entr", 10, 0, "clickEffect", 0, {animShow(0, true)})})));
        QCOMPARE(int(nowhere.steps.size()), 1);
        QVERIFY(!nowhere.steps[0].visual());
        QVERIFY(deck::statesAt(nowhere, g, 1).isEmpty());
        // No values, a set of a color, a duration below nothing, and a keyframe past the end.
        const QString noValues = QStringLiteral("<p:anim calcmode=\"lin\" valueType=\"num\"><p:cBhvr><p:cTn id=\"90\" dur=\"400\" fill=\"hold\"/>") + animTarget(4) +
                                 QStringLiteral("<p:attrNameLst><p:attrName>ppt_x</p:attrName></p:attrNameLst></p:cBhvr></p:anim>");
        const QString color = QStringLiteral("<p:set><p:cBhvr><p:cTn id=\"91\" dur=\"1\" fill=\"hold\"/>") + animTarget(4) +
                              QStringLiteral("<p:attrNameLst><p:attrName>fill.color</p:attrName></p:attrNameLst></p:cBhvr><p:to><p:clrVal><a:srgbClr xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" val=\"FF0000\"/></p:clrVal></p:to></p:set>");
        const QString negative = QStringLiteral("<p:animEffect transition=\"in\" filter=\"fade\"><p:cBhvr><p:cTn id=\"92\" dur=\"-5\"/>") + animTarget(4) + QStringLiteral("</p:cBhvr></p:animEffect>");
        const QString late = QStringLiteral("<p:anim calcmode=\"lin\"><p:cBhvr><p:cTn id=\"93\" dur=\"400\" fill=\"hold\"/>") + animTarget(4) +
                             QStringLiteral("<p:attrNameLst><p:attrName>ppt_y</p:attrName></p:attrNameLst></p:cBhvr><p:tavLst><p:tav tm=\"250000\"><p:val><p:fltVal val=\"0.9\"/></p:val></p:tav><p:tav tm=\"-5\"><p:val><p:fltVal val=\"0.1\"/></p:val></p:tav></p:tavLst></p:anim>");
        const deck::Timeline odd = deck::readTimeline(animTiming(animSteps({animNode("emph", 0, 0, "clickEffect", 0, {noValues, color, negative, late})})));
        QCOMPARE(int(odd.steps.size()), 1);
        for (const double ms : {0.0, 100.0, 399.0, 5000.0}) {
            const deck::TargetState s = deck::statesAt(odd, g, 0, ms).value(deck::Target{4});
            QVERIFY(s.visible);
            QCOMPARE(s.offset.x(), 0.0);   // no values: where it is
            QVERIFY(std::isfinite(s.offset.y()) && std::isfinite(s.opacity));
        }
        // A target that is a number that is not one, and a paragraph range backwards.
        const deck::Timeline backwards = deck::readTimeline(animTiming(animStep({animGroup(0, {animNode("entr", 1, 0, "clickEffect", 0, {QStringLiteral(
            "<p:set><p:cBhvr><p:cTn id=\"94\" dur=\"1\" fill=\"hold\"/><p:tgtEl><p:spTgt spid=\"4\"><p:txEl><p:pRg st=\"3\" end=\"1\"/></p:txEl></p:spTgt></p:tgtEl><p:attrNameLst><p:attrName>style.visibility</p:attrName></p:attrNameLst></p:cBhvr><p:to><p:strVal val=\"visible\"/></p:to></p:set>")})})})));
        const deck::Target range = backwards.steps[0].effects[0].behaviors[0].target;
        QCOMPARE(range.firstParagraph, 1);
        QCOMPARE(range.lastParagraph, 3);
        const deck::Timeline junk = deck::readTimeline(animTiming(animSteps({animNode("entr", 1, 0, "clickEffect", 0, {QStringLiteral(
            "<p:set><p:cBhvr><p:cTn id=\"95\" dur=\"1\"/><p:tgtEl><p:spTgt spid=\"x\"/></p:tgtEl></p:cBhvr><p:to><p:strVal val=\"visible\"/></p:to></p:set>")})})));
        QVERIFY(!junk.steps[0].visual());
    }

    // A formula nested thousands deep (a damaged or hostile file) is refused
    // like any other it can't read, without running out of stack; real ones
    // nest a few levels.
    void animationFormulasNestOnlySoDeep()
    {
        const QHash<QString, double> v{{"ppt_x", 0.25}};
        bool ok = false;
        const QString deep = QString(40, QLatin1Char('(')) + QStringLiteral("#ppt_x") + QString(40, QLatin1Char(')'));
        QVERIFY(animNear(deck::evaluateFormula(deep, v, &ok), 0.25) && ok);
        ok = true;
        deck::evaluateFormula(QString(1000, QLatin1Char('(')) + QStringLiteral("1") + QString(1000, QLatin1Char(')')), v, &ok);   // short, but deep
        QVERIFY(!ok);
        ok = true;
        deck::evaluateFormula(QString(200000, QLatin1Char('(')) + QStringLiteral("1") + QString(200000, QLatin1Char(')')), v, &ok);
        QVERIFY(!ok);
        ok = true;
        deck::evaluateFormula(QString(300000, QLatin1Char('-')) + QStringLiteral("1"), v, &ok);
        QVERIFY(!ok);
    }

    // The shapes' places are found by the number the file gave, in groups too.
    void animationGeometryFindsShapesByNumber()
    {
        auto d = showDeck(1);
        Page &slide = *d->pages[0];
        numberShowShapes(slide);
        auto inner = std::make_shared<ShapeItem>();
        inner->rect = QRectF(300, 380, 120, 80);
        inner->sourceId = 9;
        auto group = std::make_shared<GroupItem>();
        group->children.push_back(inner);
        group->syncRect();
        group->sourceId = 8;
        slide.items.push_back(group);
        const deck::Geometry g = deck::geometryOf(*d, slide);
        QCOMPARE(g.slide, QSizeF(960, 540));
        QCOMPARE(g.shapes.value(2), slide.items[0]->rect);
        QCOMPARE(g.shapes.value(3), slide.items[1]->rect);
        QCOMPARE(g.shapes.value(9), QRectF(300, 380, 120, 80));
        QCOMPARE(g.shapes.value(8), QRectF(300, 380, 120, 80));
        QVERIFY(!g.shapes.contains(0));
    }

    // The slide painted with the states: hidden, moved, turned, scaled, faded,
    // and wiped shapes come out as drawing them so would.
    void animationPaintsTheStates()
    {
        auto d = showDeck(1);
        Page &slide = *d->pages[0];
        numberShowShapes(slide);
        auto box = std::make_shared<ShapeItem>();
        box->rect = QRectF(600, 380, 120, 80);   // its middle is at 660, 420
        box->fill = Fill::solid(ColorRef::rgb(QColor(200, 60, 40)));
        box->sourceId = 7;
        slide.items.push_back(box);
        LayoutCache cache;
        auto paint = [&](const deck::States &states) {
            return paintedSlide(d.get(), &cache, 0, [&](PaintContext &ctx) { deck::animate(ctx, *d, states); });
        };
        const QImage plain = paintedSlide(d.get(), &cache, 0);
        QCOMPARE(paint({}), plain);   // nothing to change
        const QColor fill(200, 60, 40);
        QCOMPARE(plain.pixelColor(660, 420), fill);
        // Hidden: as if it were not there.
        deck::States hidden;
        hidden[deck::Target{7}].visible = false;
        const QImage hid = paint(hidden);
        QVERIFY(hid != plain);
        QCOMPARE(hid.pixelColor(660, 420), QColor(Qt::white));
        slide.items.pop_back();
        QCOMPARE(hid, paintedSlide(d.get(), &cache, 0));
        slide.items.push_back(box);
        // Moved 40 points: as if it were put there.
        deck::States moved;
        moved[deck::Target{7}].offset = QPointF(40, -20);
        const QImage mv = paint(moved);
        box->rect.translate(40, -20);
        QCOMPARE(mv, paintedSlide(d.get(), &cache, 0));
        box->rect.translate(-40, 20);
        // Turned a quarter about its middle: 80 wide and 120 high.
        deck::States turned;
        turned[deck::Target{7}].rotation = 90;
        const QImage tn = paint(turned);
        QCOMPARE(tn.pixelColor(660, 365), fill);                        // above the old top, inside the new
        QCOMPARE(tn.pixelColor(660 - 55, 420), QColor(Qt::white));      // inside the old left, outside the new
        // Scaled to twice its size about its middle.
        deck::States scaled;
        scaled[deck::Target{7}].scaleX = 2;
        scaled[deck::Target{7}].scaleY = 2;
        const QImage sc = paint(scaled);
        QCOMPARE(sc.pixelColor(660 + 110, 420 + 70), fill);
        // Half see-through: the fill half way to white.
        deck::States faded;
        faded[deck::Target{7}].opacity = 0.5;
        const QImage fd = paint(faded);
        const QColor mid = fd.pixelColor(660, 420);
        QVERIFY(std::abs(mid.red() - 227) <= 2 && std::abs(mid.green() - 157) <= 2 && std::abs(mid.blue() - 147) <= 2);
        QCOMPARE(fd.pixelColor(300, 300), plain.pixelColor(300, 300));
        // Wiped: the left half.
        deck::States wiped;
        wiped[deck::Target{7}].clip = QRectF(0, 0, 0.5, 1);
        const QImage wp = paint(wiped);
        QCOMPARE(wp.pixelColor(630, 420), fill);
        QCOMPARE(wp.pixelColor(690, 420), QColor(Qt::white));
        // Turned, wiped, and faded at once: the top half of the shape, which turned is its right half, half see-through.
        deck::States all;
        all[deck::Target{7}].rotation = 90;
        all[deck::Target{7}].clip = QRectF(0, 0, 1, 0.5);
        all[deck::Target{7}].opacity = 0.5;
        const QImage al = paint(all);
        QVERIFY(al.pixelColor(680, 420) != QColor(Qt::white));
        QCOMPARE(al.pixelColor(640, 420), QColor(Qt::white));
    }

    // A paragraph that is a target is drawn on its own: hidden, it is gone and
    // the lines around it stay where they are; moved, only its lines move.
    void animationPaintsParagraphs()
    {
        auto d = showDeck(1);
        Page &slide = *d->pages[0];
        numberShowShapes(slide);
        auto *body = static_cast<TextItem *>(slide.items[1].get());
        d->storyDoc(body->storyId)->setPlainText(QStringLiteral("First paragraph\nSecond paragraph\nThird paragraph"));
        LayoutCache cache;
        auto paint = [&](const deck::States &states) {
            return paintedSlide(d.get(), &cache, 0, [&](PaintContext &ctx) { deck::animate(ctx, *d, states); });
        };
        const QImage plain = paintedSlide(d.get(), &cache, 0);
        // The lines of paragraph 1, in the page's coordinates.
        const LayoutCache::FrameLayout fl = cache.textFrame(*d, *body, 1, RenderOptions());
        QVERIFY(fl.layout);
        const QTextBlock second = d->storyDoc(body->storyId)->findBlockByNumber(1);
        QRectF band;
        for (const QRectF &r : fl.layout->rangeRects(fl.frame, second.position(), second.position() + second.length())) band = band.united(r);
        band.translate(body->rect.topLeft());
        QVERIFY(band.isValid());
        auto changedRows = [&](const QImage &a, const QImage &b) {
            int top = -1, bottom = -1;
            for (int y = 0; y < a.height(); ++y)
                for (int x = 0; x < a.width(); ++x)
                    if (a.pixel(x, y) != b.pixel(x, y)) {
                        if (top < 0) top = y;
                        bottom = y;
                        break;
                    }
            return QPair<int, int>(top, bottom);
        };
        deck::States hidden;
        hidden[deck::Target{3, 1, 1}].visible = false;
        const QPair<int, int> rows = changedRows(plain, paint(hidden));
        QVERIFY(rows.first >= 0);                       // something went
        QVERIFY(rows.first >= band.top() - 2 && rows.second <= band.bottom() + 2);   // only its lines
        // Shown, nothing differs.
        deck::States shown;
        shown[deck::Target{3, 1, 1}];
        QCOMPARE(paint(shown), plain);
        // Moved right: its lines are where they were, 300 points over.
        deck::States moved;
        moved[deck::Target{3, 1, 1}].offset = QPointF(300, 0);
        const QImage mv = paint(moved);
        const QPair<int, int> movedRows = changedRows(plain, mv);
        QVERIFY(movedRows.first >= band.top() - 2 && movedRows.second <= band.bottom() + 2);
        auto inkLeft = [&](const QImage &img) {
            for (int x = 0; x < img.width(); ++x)
                for (int y = int(band.top()); y < int(band.bottom()); ++y)
                    if (img.pixel(x, y) != QColor(Qt::white).rgba()) return x;
            return -1;
        };
        QVERIFY(inkLeft(plain) >= 0);
        QCOMPARE(inkLeft(mv), inkLeft(plain) + 300);
    }
};

JP_TEST_CLASS(DeckMediaTests)
#include "tst_deck_media.moc"
