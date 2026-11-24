// JeffDeck's test that a presentation is saved as the same bytes in every process.
#include "../stablesaves.h"
#include "deckhelpers.h"

class DeckStableSavesTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // Not a test of its own: the work of a process that the next function starts. A presentation with what is kept from a file as markup (a shadow with four attributes, a time
    // line, a table style, a transition), read and written again.
    void savesTheRichPresentation()
    {
        QString out;
        if (!stable::childOutput(&out)) return;
        const QString shadow = QStringLiteral("<a:effectLst><a:outerShdw blurRad=\"38100\" dist=\"38100\" dir=\"2700000\" algn=\"tl\" rotWithShape=\"0\"><a:srgbClr val=\"000000\"/></a:outerShdw></a:effectLst>");
        const QString id = QStringLiteral("{11111111-2222-3333-4444-555555555555}");
        Parts p = samplePptx();
        p.data["/ppt/tableStyles.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.tableStyles+xml"),
            QStringLiteral("<a:tblStyleLst xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" def=\"%1\"><a:tblStyle styleId=\"%1\" styleName=\"Test\"><a:wholeTbl>"
                           "<a:tcTxStyle b=\"on\" i=\"off\"><a:fontRef idx=\"minor\"/><a:srgbClr val=\"202020\"/></a:tcTxStyle><a:tcStyle><a:tcBdr/></a:tcStyle></a:wholeTbl>"
                           "<a:firstRow><a:tcTxStyle i=\"off\" b=\"on\"><a:fontRef idx=\"minor\"/><a:srgbClr val=\"FFFFFF\"/></a:tcTxStyle><a:tcStyle><a:tcBdr/></a:tcStyle></a:firstRow></a:tblStyle></a:tblStyleLst>")
                .arg(id).toUtf8()};
        p.rels << Rel{"/ppt/presentation.xml", "tableStyles", "/ppt/tableStyles.xml"};
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        xml.replace(QStringLiteral("</p:spTree>"), QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"30\" name=\"Effects\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>") + xfrm(0, 0, 4572000, 914400)
                                                       + QStringLiteral("</p:spPr><p:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\">") + shadow
                                                       + QStringLiteral("</a:rPr><a:t>Shadowed</a:t></a:r></a:p></p:txBody></p:sp></p:spTree>"));
        xml.replace(QStringLiteral("</p:sld>"), QStringLiteral("<p:timing><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\"><p:childTnLst>"
                                                               "<p:seq concurrent=\"1\" nextAc=\"seek\"><p:cTn id=\"2\" dur=\"indefinite\" nodeType=\"mainSeq\"><p:childTnLst>"
                                                               "<p:par><p:cTn id=\"3\" fill=\"hold\"><p:stCondLst><p:cond delay=\"indefinite\"/></p:stCondLst><p:childTnLst>"
                                                               "<p:par><p:cTn id=\"4\" fill=\"hold\"><p:stCondLst><p:cond delay=\"0\"/></p:stCondLst><p:childTnLst>"
                                                               "<p:par><p:cTn id=\"5\" presetID=\"1\" presetClass=\"entr\" presetSubtype=\"0\" fill=\"hold\" grpId=\"0\" nodeType=\"clickEffect\">"
                                                               "<p:stCondLst><p:cond delay=\"0\"/></p:stCondLst><p:childTnLst><p:set><p:cBhvr><p:cTn id=\"6\" dur=\"1\" fill=\"hold\"/>"
                                                               "<p:tgtEl><p:spTgt spid=\"30\"/></p:tgtEl><p:attrNameLst><p:attrName>style.visibility</p:attrName></p:attrNameLst></p:cBhvr>"
                                                               "<p:to><p:strVal val=\"visible\"/></p:to></p:set></p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:par></p:childTnLst></p:cTn></p:par>"
                                                               "</p:childTnLst></p:cTn><p:prevCondLst><p:cond evt=\"onPrev\" delay=\"0\"><p:tgtEl><p:sldTgt/></p:tgtEl></p:cond></p:prevCondLst></p:seq>"
                                                               "</p:childTnLst></p:cTn></p:par></p:tnLst></p:timing></p:sld>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(d->pages[0]->props.value(QStringLiteral("timingXml")).toString().contains(QStringLiteral("mainSeq")));
        const QByteArray saved = deck::exportPresentation(*d, &err);
        QVERIFY2(!saved.isEmpty(), qPrintable(err));
        QFile f(out);
        QVERIFY(f.open(QIODevice::WriteOnly));
        QCOMPARE(f.write(saved), saved.size());
    }

    // Qt picks its hash seed anew for each process, and the DOM keeps attributes in a hash: what is kept as markup is saved in a fixed order, or two saves differ.
    void aPresentationIsSavedAsTheSameBytesInEachProcess() { JP_VERIFY_SAME_SAVES("savesTheRichPresentation"); }

    // A presentation opened and saved twice through the window, with nothing edited between: the two files are the same bytes in every part (only the modified time may differ), and a
    // third saved after opening the second is the same again. `ending` picks the format.
    static QString differenceOfTwoSaves(const QString &ending, bool reopened = true)
    {
        QTemporaryDir dir;
        if (!dir.isValid()) return QStringLiteral("no folder");
        const QString src = dir.filePath(QStringLiteral("src.pptx"));
        QFile f(src);
        if (!f.open(QIODevice::WriteOnly) || f.write(pack(samplePptx())) < 0) return QStringLiteral("no source");
        f.close();
        setLossyStandIn([](const LossyAsk &, bool *) { return LossyChoice::SaveAnyway; });
        const auto reset = qScopeGuard([] { setLossyStandIn(nullptr); });
        DeckWindow w;
        if (!w.openFile(src)) return QStringLiteral("not opened");
        const QString one = dir.filePath(QStringLiteral("one") + ending), two = dir.filePath(QStringLiteral("two") + ending), three = dir.filePath(QStringLiteral("three") + ending);
        if (!w.saveTo(one) || !w.saveTo(two)) return QStringLiteral("not saved");
        auto read = [](const QString &path) {
            QFile r(path);
            return r.open(QIODevice::ReadOnly) ? r.readAll() : QByteArray();
        };
        QString d = stable::differenceBetweenFiles(read(one), read(two));
        if (!d.isEmpty()) return QStringLiteral("first against second: ") + d;
        if (!reopened) return QString();
        DeckWindow again;
        if (!again.openFile(two)) return QStringLiteral("not reopened");
        if (!again.saveTo(three)) return QStringLiteral("not saved again");
        d = stable::differenceBetweenFiles(read(two), read(three));
        return d.isEmpty() ? QString() : QStringLiteral("second against reopened: ") + d;
    }
    void aPptxSavedTwiceIsTheSameBytes() { const QString d = differenceOfTwoSaves(QStringLiteral(".pptx")); QVERIFY2(d.isEmpty(), qPrintable(d)); }
    void aPotxSavedTwiceIsTheSameBytes() { const QString d = differenceOfTwoSaves(QStringLiteral(".potx")); QVERIFY2(d.isEmpty(), qPrintable(d)); }
    void aPpsxSavedTwiceIsTheSameBytes() { const QString d = differenceOfTwoSaves(QStringLiteral(".ppsx")); QVERIFY2(d.isEmpty(), qPrintable(d)); }
    void anOdpSavedTwiceIsTheSameBytes() { const QString d = differenceOfTwoSaves(QStringLiteral(".odp"), false); QVERIFY2(d.isEmpty(), qPrintable(d)); }

    // An open document presentation keeps the length of a transition as it is (not the speed word nearest to it) and the format of the date its slides show.
    void anOpenDocumentPresentationKeepsATransitionLengthAndADateFormat()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        deck::Transition t;
        t.type = QStringLiteral("fade");
        t.duration = 0.75;
        deck::setTransition(*d->pages[0], t);
        const QByteArray bytes = deck::exportOdp(*d, false, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importOdp(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(deck::transitionOf(*back->pages[0]).duration, 0.75);
        // (the slide is drawn the same: its date shows in the format it had, not the file's default)
        QVERIFY(slideImage(*back) == slideImage(*d));
    }
};

JP_TEST_CLASS(DeckStableSavesTests)
#include "tst_deck_stablesaves.moc"
