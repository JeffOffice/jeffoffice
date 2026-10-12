// JeffDeck's tests: its presentation model on the shared page model, and
// its window.

#include "app/deckinfo.h"
#include "app/deckwindow.h"
#include "app/editor.h"
#include "app/notespane.h"
#include "app/slideshow.h"
#include "app/slidespane.h"
#include "core/deck.h"
#include "core/fonts.h"
#include "io/pptximport.h"
#include "office/drawingml.h"
#include "office/package.h"
#include "office/xml.h"
#include "render/renderer.h"

#include <QApplication>
#include <QBuffer>
#include <QJsonArray>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>
#include <QTextDocument>
#include <QTimer>

using namespace jp;

class DeckTests : public QObject {
    Q_OBJECT
    QTemporaryDir m_settingsDir;

private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(deck);
        setAppInfo(deckAppInfo());
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, m_settingsDir.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settingsDir.path());
        initCore();
    }

    // A new presentation: widescreen, one slide master with the nine
    // standard layouts over it, the default theme, and a Title Slide whose
    // placeholders are empty and prompt for text.
    void newPresentationHasMasterLayoutsAndTitleSlide()
    {
        auto d = deck::newPresentation();
        QCOMPARE(d->pageSize(), QSizeF(960, 540));
        QCOMPARE(int(d->masters.size()), 10);
        QVERIFY(deck::isSlideMaster(*d->masters[0]));
        const QVector<MasterPage *> layouts = deck::layouts(*d);
        QCOMPARE(int(layouts.size()), 9);
        for (MasterPage *l : layouts) QCOMPARE(l->baseId, d->masters[0]->id);
        QStringList types;
        for (MasterPage *l : layouts) types << deck::layoutType(*l);
        QCOMPARE(types, (QStringList{"title", "obj", "secHead", "twoObj", "twoTxTwoObj", "titleOnly", "blank", "objTx", "picTx"}));
        QCOMPARE(deck::themeOf(*d).name, QStringLiteral("JeffOffice"));
        QCOMPARE(int(d->pages.size()), 1);
        const Page &slide = *d->pages[0];
        QCOMPARE(deck::layoutType(*d->masterFor(slide)), QStringLiteral("title"));
        QStringList placeholders;
        for (const auto &it : slide.items) {
            placeholders << deck::placeholderType(*it);
            const auto *t = static_cast<const TextItem *>(it.get());
            QVERIFY(d->storyDoc(t->storyId)->isEmpty());
            QVERIFY(!t->prompt.isEmpty());
        }
        QCOMPARE(placeholders, (QStringList{"ctrTitle", "subTitle"}));
        QVERIFY(d->storyDoc(slide.notesStoryId));
        // Empty placeholders and the layout's prompts print nothing.
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        const QImage img = Renderer::renderToImage(ctx, 0, 0.25);
        for (int y = 0; y < img.height(); y += 7)
            for (int x = 0; x < img.width(); x += 7) QCOMPARE(img.pixelColor(x, y), QColor(Qt::white));
    }

    // Slides copy their layout's placeholders; the date, footer, and slide
    // number only when the presentation shows them.
    void addSlideCopiesPlaceholders()
    {
        auto d = deck::newPresentation();
        const MasterPage *two = deck::layoutOfType(*d, QStringLiteral("twoObj"));
        const int i = deck::addSlide(*d, *two);
        QCOMPARE(i, 1);
        QStringList types;
        for (const auto &it : d->pages[i]->items) types << deck::placeholderType(*it);
        QCOMPARE(types, (QStringList{"title", "obj", "obj"}));
        // Each its own story and object, not the layout's.
        const auto *layoutTitle = static_cast<const TextItem *>(two->items[0].get());
        const auto *slideTitle = static_cast<const TextItem *>(d->pages[i]->items[0].get());
        QVERIFY(layoutTitle->id != slideTitle->id);
        QVERIFY(layoutTitle->storyId != slideTitle->storyId);
        QCOMPARE(slideTitle->rect, layoutTitle->rect);
        d->appData[QStringLiteral("footers")] = true;
        const int j = deck::addSlide(*d, *two, 0);
        QCOMPARE(j, 0);
        types.clear();
        for (const auto &it : d->pages[j]->items) types << deck::placeholderType(*it);
        QCOMPARE(types, (QStringList{"title", "obj", "obj", "dt", "ftr", "sldNum"}));
    }

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

    // The window: the ribbon builds from its file with nothing missing, and
    // the slide commands are steps that undo.
    void windowSlideCommands()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        Editor *ed = w.editor();
        QCOMPARE(w.slidesPane()->count(), 1);
        w.newSlide();
        QCOMPARE(int(ed->doc()->pages.size()), 2);
        QCOMPARE(ed->currentPage(), 1);
        QCOMPARE(deck::layoutType(*ed->doc()->masterFor(*ed->doc()->pages[1])), QStringLiteral("obj"));   // content after a title slide
        QTRY_COMPARE(w.slidesPane()->count(), 2);
        w.duplicateSlide();
        QCOMPARE(int(ed->doc()->pages.size()), 3);
        w.toggleHidden();
        QVERIFY(ed->doc()->pages[2]->hidden);
        w.setLayout(QStringLiteral("titleOnly"));
        QCOMPARE(deck::layoutType(*ed->doc()->masterFor(*ed->doc()->pages[2])), QStringLiteral("titleOnly"));
        ed->undo();
        ed->undo();
        ed->undo();
        QCOMPARE(int(ed->doc()->pages.size()), 2);
        w.deleteSlide();
        QCOMPARE(int(ed->doc()->pages.size()), 1);
        w.deleteSlide();   // the last slide stays
        QCOMPARE(int(ed->doc()->pages.size()), 1);
        // Notes typed in the pane reach the slide as an undo step.
        QTest::keyClicks(w.notesPane(), QStringLiteral("Remember the demo"));
        QVERIFY(w.notesPane()->hasTyping());
        w.notesPane()->commit();
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QStringLiteral("Remember the demo"));
        ed->undo();
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QString());
        QCOMPARE(w.notesPane()->toPlainText(), QString());   // the pane shows the undone notes
        w.newSlide();   // and moving on doesn't write them back
        QCOMPARE(deck::notesText(*ed->doc(), *ed->doc()->pages[0]), QString());
        ed->markSaved();
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

private:
    struct Rel {
        QString source, type, target;
    };
    struct Part {
        QString type;
        QByteArray xml;
    };
    // A presentation file as parts and relationships, so a test can change
    // one before it is packed.
    struct Parts {
        QMap<QString, Part> data;
        QVector<Rel> rels;
    };

    static QString nsDecl()
    {
        return QStringLiteral(
            "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
            "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" "
            "xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"");
    }
    static const QString &relBase()
    {
        static const QString base = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
        return base;
    }
    static QByteArray tinyPng()
    {
        QImage img(4, 4, QImage::Format_RGB32);
        img.fill(QColor(0xB6, 0x42, 0x0F));
        QByteArray bytes;
        QBuffer buf(&bytes);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "PNG");
        return bytes;
    }
    // A shape tree's start, and its end.
    static QString treeStart() { return QStringLiteral("<p:cSld><p:spTree><p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>"); }
    static QString treeEnd() { return QStringLiteral("</p:spTree></p:cSld>"); }
    static QString placeholderShape(int id, const QString &name, const QString &ph, const QString &xfrm, const QString &body)
    {
        return QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"%2\"/><p:cNvSpPr><a:spLocks noGrp=\"1\"/></p:cNvSpPr><p:nvPr><p:ph %3/></p:nvPr></p:nvSpPr>"
                              "<p:spPr>%4</p:spPr><p:txBody><a:bodyPr/><a:lstStyle/>%5</p:txBody></p:sp>")
            .arg(id).arg(name, ph, xfrm, body);
    }
    static QString xfrm(qint64 x, qint64 y, qint64 cx, qint64 cy)
    {
        return QStringLiteral("<a:xfrm><a:off x=\"%1\" y=\"%2\"/><a:ext cx=\"%3\" cy=\"%4\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>").arg(x).arg(y).arg(cx).arg(cy);
    }
    static QString para(const QString &text)
    {
        return text.isEmpty() ? QStringLiteral("<a:p><a:endParaRPr lang=\"en-US\"/></a:p>") : QStringLiteral("<a:p><a:r><a:rPr lang=\"en-US\"/><a:t>%1</a:t></a:r></a:p>").arg(text);
    }
    static QByteArray themeXml()
    {
        QString colors;
        const char *const names[12] = {"dk1", "lt1", "dk2", "lt2", "accent1", "accent2", "accent3", "accent4", "accent5", "accent6", "hlink", "folHlink"};
        const char *const values[12] = {"111111", "FEFEFE", "224466", "EEEEDD", "B6420F", "13773A", "1D55C4", "9E1F63", "0A6C8C", "5E2CA5", "0000EE", "551A8B"};
        for (int i = 0; i < 12; ++i) colors += QStringLiteral("<a:%1><a:srgbClr val=\"%2\"/></a:%1>").arg(QLatin1String(names[i]), QLatin1String(values[i]));
        const QString fill = QStringLiteral("<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>");
        const QString line = QStringLiteral("<a:ln w=\"9525\">") + fill + QStringLiteral("</a:ln>");
        return QStringLiteral(
                   "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" name=\"Test Theme\">"
                   "<a:themeElements><a:clrScheme name=\"Test Colors\">%1</a:clrScheme>"
                   "<a:fontScheme name=\"Test Fonts\"><a:majorFont><a:latin typeface=\"Georgia\"/><a:ea typeface=\"\"/><a:cs typeface=\"\"/></a:majorFont>"
                   "<a:minorFont><a:latin typeface=\"Verdana\"/><a:ea typeface=\"\"/><a:cs typeface=\"\"/></a:minorFont></a:fontScheme>"
                   "<a:fmtScheme name=\"Test Formats\"><a:fillStyleLst>%2%2%2</a:fillStyleLst><a:lnStyleLst>%3%3%3</a:lnStyleLst>"
                   "<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>"
                   "<a:bgFillStyleLst>%2%2%2</a:bgFillStyleLst></a:fmtScheme></a:themeElements></a:theme>")
            .arg(colors, fill, line)
            .toUtf8();
    }
    static QString masterXml()
    {
        return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:sldMaster %1>").arg(nsDecl())
            + QStringLiteral("<p:cSld name=\"Test Master\"><p:bg><p:bgPr><a:solidFill><a:srgbClr val=\"FFFFFF\"/></a:solidFill><a:effectLst/></p:bgPr></p:bg><p:spTree>"
                             "<p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>")
            + placeholderShape(2, QStringLiteral("Title Placeholder 1"), QStringLiteral("type=\"title\""), xfrm(838200, 365125, 10515600, 1325563), para(QStringLiteral("Click to edit Master title style")))
            + placeholderShape(3, QStringLiteral("Text Placeholder 2"), QStringLiteral("type=\"body\" idx=\"1\""), xfrm(838200, 1825625, 10515600, 4351338), para(QStringLiteral("Click to edit Master text styles")))
            + placeholderShape(4, QStringLiteral("Date Placeholder 3"), QStringLiteral("type=\"dt\" sz=\"half\" idx=\"2\""), xfrm(838200, 6356350, 2743200, 365125), para(QStringLiteral("10/11/2026")))
            + QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"5\" name=\"Master Band\"/><p:cNvSpPr/><p:nvPr userDrawn=\"1\"/></p:nvSpPr><p:spPr>") + xfrm(0, 0, 12192000, 182880)
            + QStringLiteral("<a:solidFill><a:schemeClr val=\"accent1\"/></a:solidFill></p:spPr></p:sp>")
            + QStringLiteral("</p:spTree></p:cSld>"
                             "<p:clrMap bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/>"
                             "<p:sldLayoutIdLst><p:sldLayoutId id=\"2147483649\" r:id=\"rId1\"/><p:sldLayoutId id=\"2147483650\" r:id=\"rId2\"/></p:sldLayoutIdLst>"
                             "<p:txStyles><p:titleStyle><a:lvl1pPr algn=\"l\"><a:defRPr sz=\"4400\"><a:latin typeface=\"+mj-lt\"/></a:defRPr></a:lvl1pPr></p:titleStyle>"
                             "<p:bodyStyle><a:lvl1pPr marL=\"228600\" indent=\"-228600\"><a:buChar char=\"&#8226;\"/><a:defRPr sz=\"2800\"/></a:lvl1pPr></p:bodyStyle>"
                             "<p:otherStyle><a:lvl1pPr><a:defRPr sz=\"1800\"/></a:lvl1pPr></p:otherStyle></p:txStyles></p:sldMaster>");
    }
    static QString layoutXml(const QString &type, const QString &name, const QString &attrs, const QString &shapes)
    {
        return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:sldLayout %1 type=\"%2\" %3>").arg(nsDecl(), type, attrs)
            + QStringLiteral("<p:cSld name=\"%1\"><p:spTree><p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>").arg(name) + shapes
            + QStringLiteral("</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr></p:sldLayout>");
    }
    static QString slideXml(const QString &attrs, const QString &background, const QString &shapes, const QString &after = QString())
    {
        return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:sld %1 %2>").arg(nsDecl(), attrs)
            + QStringLiteral("<p:cSld>") + background + QStringLiteral("<p:spTree><p:nvGrpSpPr><p:cNvPr id=\"1\" name=\"\"/><p:cNvGrpSpPr/><p:nvPr/></p:nvGrpSpPr><p:grpSpPr/>") + shapes
            + QStringLiteral("</p:spTree></p:cSld><p:clrMapOvr><a:masterClrMapping/></p:clrMapOvr>") + after + QStringLiteral("</p:sld>");
    }
    static QString pictureShape(int id, qint64 x, qint64 y)
    {
        return QStringLiteral("<p:pic><p:nvPicPr><p:cNvPr id=\"%1\" name=\"Picture %1\" descr=\"A small square\"/><p:cNvPicPr><a:picLocks noChangeAspect=\"1\"/></p:cNvPicPr><p:nvPr/></p:nvPicPr>"
                              "<p:blipFill><a:blip r:embed=\"rId2\"/><a:stretch><a:fillRect/></a:stretch></p:blipFill><p:spPr>%2</p:spPr></p:pic>")
            .arg(id).arg(xfrm(x, y, 914400, 914400));
    }
    static QString tableFrame(int id)
    {
        return QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"%1\" name=\"Table %1\"/><p:cNvGraphicFramePr><a:graphicFrameLocks noGrp=\"1\"/></p:cNvGraphicFramePr><p:nvPr/></p:nvGraphicFramePr>"
                              "<p:xfrm><a:off x=\"914400\" y=\"1828800\"/><a:ext cx=\"3657600\" cy=\"1371600\"/></p:xfrm>"
                              "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/table\"><a:tbl><a:tblPr/><a:tblGrid><a:gridCol w=\"3657600\"/></a:tblGrid>"
                              "<a:tr h=\"370840\"><a:tc><a:txBody><a:bodyPr/><a:lstStyle/><a:p><a:r><a:t>Cell &amp; &lt;1&gt;</a:t></a:r></a:p></a:txBody><a:tcPr/></a:tc></a:tr></a:tbl></a:graphicData></a:graphic></p:graphicFrame>")
            .arg(id);
    }
    // A chart the reader doesn't know, standing in a pair of alternatives
    // with a picture of it.
    static QString chartWithPicture(int id)
    {
        return QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\">"
                              "<mc:Choice xmlns:cx1=\"urn:test:chart\" Requires=\"cx1\"><p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"%1\" name=\"Chart %1\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                              "<p:xfrm><a:off x=\"1219200\" y=\"1219200\"/><a:ext cx=\"6096000\" cy=\"3048000\"/></p:xfrm><a:graphic><a:graphicData uri=\"urn:test:chart\"><cx1:chart r:id=\"rId9\"/></a:graphicData></a:graphic></p:graphicFrame></mc:Choice>"
                              "<mc:Fallback>%2</mc:Fallback></mc:AlternateContent>")
            .arg(id)
            .arg(pictureShape(id + 1, 1219200, 1219200));
    }

    // An embedded object with a picture of itself, in a pair of alternatives for programs that
    // know the other kind of drawing.
    static QString objectWithPicture(int id)
    {
        return QStringLiteral("<p:graphicFrame><p:nvGraphicFramePr><p:cNvPr id=\"%1\" name=\"Object %1\"/><p:cNvGraphicFramePr/><p:nvPr/></p:nvGraphicFramePr>"
                              "<p:xfrm><a:off x=\"7315200\" y=\"1219200\"/><a:ext cx=\"1828800\" cy=\"1828800\"/></p:xfrm><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/presentationml/2006/ole\">"
                              "<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\"><mc:Choice xmlns:v=\"urn:test:drawing\" Requires=\"v\">"
                              "<p:oleObj spid=\"_x0000_s1040\" name=\"Object\" r:id=\"rId7\"><p:embed/></p:oleObj></mc:Choice>"
                              "<mc:Fallback><p:oleObj name=\"Object\" r:id=\"rId7\"><p:embed/>%2</p:oleObj></mc:Fallback></mc:AlternateContent></a:graphicData></a:graphic></p:graphicFrame>")
            .arg(id)
            .arg(pictureShape(id + 1, 7315200, 1219200));
    }

    // The sample presentation: 16:9, one master over two layouts (Title
    // Slide, Title and Content), and two slides.
    static Parts samplePptx()
    {
        const QString pres = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml");
        Parts p;
        p.data["/ppt/presentation.xml"] = {pres,
            (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:presentation %1>").arg(nsDecl())
             + QStringLiteral("<p:sldMasterIdLst><p:sldMasterId id=\"2147483648\" r:id=\"rId1\"/></p:sldMasterIdLst>"
                              "<p:notesMasterIdLst><p:notesMasterId r:id=\"rId4\"/></p:notesMasterIdLst>"
                              "<p:sldIdLst><p:sldId id=\"256\" r:id=\"rId2\"/><p:sldId id=\"257\" r:id=\"rId3\"/></p:sldIdLst>"
                              "<p:sldSz cx=\"12192000\" cy=\"6858000\"/><p:notesSz cx=\"6858000\" cy=\"9144000\"/>"
                              "<p:defaultTextStyle><a:defPPr><a:defRPr lang=\"en-US\"/></a:defPPr><a:lvl1pPr marL=\"0\" algn=\"l\"><a:defRPr sz=\"1800\"><a:solidFill><a:schemeClr val=\"tx1\"/></a:solidFill>"
                              "<a:latin typeface=\"+mn-lt\"/></a:defRPr></a:lvl1pPr></p:defaultTextStyle></p:presentation>")).toUtf8()};
        p.rels << Rel{"/ppt/presentation.xml", "slideMaster", "/ppt/slideMasters/slideMaster1.xml"}
               << Rel{"/ppt/presentation.xml", "slide", "/ppt/slides/slide1.xml"}
               << Rel{"/ppt/presentation.xml", "slide", "/ppt/slides/slide2.xml"}
               << Rel{"/ppt/presentation.xml", "notesMaster", "/ppt/notesMasters/notesMaster1.xml"};

        p.data["/ppt/theme/theme1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml"), themeXml()};
        p.data["/ppt/slideMasters/slideMaster1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideMaster+xml"), masterXml().toUtf8()};
        p.rels << Rel{"/ppt/slideMasters/slideMaster1.xml", "slideLayout", "/ppt/slideLayouts/slideLayout1.xml"}
               << Rel{"/ppt/slideMasters/slideMaster1.xml", "slideLayout", "/ppt/slideLayouts/slideLayout2.xml"}
               << Rel{"/ppt/slideMasters/slideMaster1.xml", "theme", "/ppt/theme/theme1.xml"};

        const QString layoutType = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slideLayout+xml");
        p.data["/ppt/slideLayouts/slideLayout1.xml"] = {layoutType,
            layoutXml(QStringLiteral("title"), QStringLiteral("Title Slide"), QStringLiteral("showMasterSp=\"0\""),
                      placeholderShape(2, QStringLiteral("Title 1"), QStringLiteral("type=\"ctrTitle\""), xfrm(1524000, 1122363, 9144000, 2387600), para(QStringLiteral("Click to edit Master title style")))
                          + placeholderShape(3, QStringLiteral("Subtitle 2"), QStringLiteral("type=\"subTitle\" idx=\"1\""), xfrm(1524000, 3602038, 9144000, 1655762), para(QStringLiteral("Click to edit Master subtitle style"))))
                .toUtf8()};
        p.data["/ppt/slideLayouts/slideLayout2.xml"] = {layoutType,
            layoutXml(QStringLiteral("obj"), QStringLiteral("Title and Content"), QString(),
                      placeholderShape(2, QStringLiteral("Title 1"), QStringLiteral("type=\"title\""), QString(), para(QStringLiteral("Click to edit Master title style")))
                          + placeholderShape(3, QStringLiteral("Content Placeholder 2"), QStringLiteral("idx=\"1\""), xfrm(838200, 1825625, 5181600, 4351338), para(QStringLiteral("Click to edit Master text styles")))
                          + placeholderShape(4, QStringLiteral("Date Placeholder 3"), QStringLiteral("type=\"dt\" sz=\"half\" idx=\"10\""), QString(), para(QStringLiteral("10/11/2026"))))
                .toUtf8()};
        p.rels << Rel{"/ppt/slideLayouts/slideLayout1.xml", "slideMaster", "/ppt/slideMasters/slideMaster1.xml"}
               << Rel{"/ppt/slideLayouts/slideLayout2.xml", "slideMaster", "/ppt/slideMasters/slideMaster1.xml"};

        const QString slideType = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.slide+xml");
        const QString dateField = QStringLiteral("<a:p><a:fld id=\"{B5D1E7E0-0000-4000-8000-000000000001}\" type=\"datetime1\"><a:rPr lang=\"en-US\"/><a:t>10/11/2026</a:t></a:fld></a:p>");
        p.data["/ppt/slides/slide1.xml"] = {slideType,
            slideXml(QString(), QString(),
                     placeholderShape(2, QStringLiteral("Title 1"), QStringLiteral("type=\"title\""), QString(), para(QStringLiteral("Quarterly results")))
                         + placeholderShape(3, QStringLiteral("Content Placeholder 2"), QStringLiteral("idx=\"1\""), QString(), para(QString()))
                         + placeholderShape(4, QStringLiteral("Date Placeholder 3"), QStringLiteral("type=\"dt\" sz=\"half\" idx=\"10\""), QString(), dateField)
                         + QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"5\" name=\"TextBox 4\"/><p:cNvSpPr txBox=\"1\"/><p:nvPr/></p:nvSpPr><p:spPr>") + xfrm(914400, 5029200, 3048000, 457200)
                         + QStringLiteral("<a:noFill/></p:spPr><p:txBody><a:bodyPr wrap=\"square\"/><a:lstStyle/>") + para(QStringLiteral("Plain text")) + QStringLiteral("</p:txBody></p:sp>")
                         + QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"6\" name=\"Rectangle 5\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr>") + xfrm(5486400, 5029200, 1828800, 914400)
                         + QStringLiteral("<a:solidFill><a:schemeClr val=\"accent1\"/></a:solidFill></p:spPr></p:sp>")
                         + pictureShape(7, 8229600, 5029200) + tableFrame(8),
                     QStringLiteral("<p:transition spd=\"med\"><p:fade/></p:transition><p:timing><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\"/></p:par></p:tnLst></p:timing>"))
                .toUtf8()};
        p.data["/ppt/slides/slide2.xml"] = {slideType,
            slideXml(QStringLiteral("show=\"0\" showMasterSp=\"0\""),
                     QStringLiteral("<p:bg><p:bgPr><a:solidFill><a:srgbClr val=\"112233\"/></a:solidFill><a:effectLst/></p:bgPr></p:bg>"),
                     placeholderShape(2, QStringLiteral("Title 1"), QStringLiteral("type=\"ctrTitle\""), QString(), para(QStringLiteral("Second")))
                         + placeholderShape(3, QStringLiteral("Subtitle 2"), QStringLiteral("type=\"subTitle\" idx=\"1\""), QString(), para(QString()))
                         + chartWithPicture(4) + objectWithPicture(6))
                .toUtf8()};
        p.rels << Rel{"/ppt/slides/slide1.xml", "slideLayout", "/ppt/slideLayouts/slideLayout2.xml"}
               << Rel{"/ppt/slides/slide1.xml", "image", "/ppt/media/image1.png"}
               << Rel{"/ppt/slides/slide1.xml", "notesSlide", "/ppt/notesSlides/notesSlide1.xml"}
               << Rel{"/ppt/slides/slide2.xml", "slideLayout", "/ppt/slideLayouts/slideLayout1.xml"}
               << Rel{"/ppt/slides/slide2.xml", "image", "/ppt/media/image1.png"};
        p.data["/ppt/media/image1.png"] = {QStringLiteral("image/png"), tinyPng()};

        p.data["/ppt/notesSlides/notesSlide1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.notesSlide+xml"),
            (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:notes %1>").arg(nsDecl()) + treeStart()
             + QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"2\" name=\"Slide Image Placeholder 1\"/><p:cNvSpPr/><p:nvPr><p:ph type=\"sldImg\"/></p:nvPr></p:nvSpPr><p:spPr/></p:sp>")
             + placeholderShape(3, QStringLiteral("Notes Placeholder 2"), QStringLiteral("type=\"body\" idx=\"1\""), QString(),
                                para(QStringLiteral("Say hello first")) + para(QStringLiteral("Then the numbers")))
             + treeEnd() + QStringLiteral("</p:notes>")).toUtf8()};
        p.rels << Rel{"/ppt/notesSlides/notesSlide1.xml", "notesMaster", "/ppt/notesMasters/notesMaster1.xml"}
               << Rel{"/ppt/notesSlides/notesSlide1.xml", "slide", "/ppt/slides/slide1.xml"};
        p.data["/ppt/notesMasters/notesMaster1.xml"] = {QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.notesMaster+xml"),
            (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:notesMaster %1>").arg(nsDecl()) + treeStart() + treeEnd()
             + QStringLiteral("<p:clrMap bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" accent5=\"accent5\" accent6=\"accent6\" hlink=\"hlink\" folHlink=\"folHlink\"/>"
                              "<p:notesStyle><a:lvl1pPr marL=\"0\"><a:defRPr sz=\"1200\"/></a:lvl1pPr></p:notesStyle></p:notesMaster>")).toUtf8()};
        p.rels << Rel{"/ppt/notesMasters/notesMaster1.xml", "theme", "/ppt/theme/theme1.xml"};
        return p;
    }
    static QByteArray pack(const Parts &p)
    {
        office::PackageWriter w;
        w.addDefault(QStringLiteral("png"), QStringLiteral("image/png"));
        for (auto it = p.data.cbegin(); it != p.data.cend(); ++it) w.addPart(it.key(), it.value().xml, it.value().type);
        w.addRelationship(QStringLiteral("/"), relBase() + QStringLiteral("officeDocument"), QStringLiteral("/ppt/presentation.xml"));
        for (const Rel &r : p.rels) w.addRelationship(r.source, relBase() + r.type, r.target);
        return w.finish();
    }
    // Whether the shared reader reads pictures yet (a picture is the one
    // shape these tests need that its first version leaves out).
    static bool readerReadsPictures()
    {
        QDomDocument dom;
        if (!office::parseXml(("<p:pic " + nsDecl() + "><p:nvPicPr><p:cNvPr id=\"1\" name=\"P\"/><p:cNvPicPr/><p:nvPr/></p:nvPicPr><p:blipFill><a:blip/></p:blipFill><p:spPr>" + xfrm(0, 0, 1270, 1270) + "</p:spPr></p:pic>").toUtf8(), &dom))
            return false;
        return office::readShape(dom.documentElement(), office::DrawingContext()) != nullptr;
    }
    static QString textOf(const Document &d, const Item &it)
    {
        const auto *t = static_cast<const TextItem *>(&it);
        const QTextDocument *sd = d.storyDoc(t->storyId);
        return sd ? sd->toPlainText() : QString();
    }

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

    // What can't be read yet is kept to write back, with its place.
    void importKeepsUnreadFrames()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        const QJsonArray unread = d->pages[0]->props.value(QStringLiteral("unread")).toArray();
        QCOMPARE(int(unread.size()), 1);
        const QJsonObject frame = unread[0].toObject();
        QCOMPARE(frame.value(QStringLiteral("x")).toDouble(), 72.0);
        QCOMPARE(frame.value(QStringLiteral("y")).toDouble(), 144.0);
        QCOMPARE(frame.value(QStringLiteral("w")).toDouble(), 288.0);
        QCOMPARE(frame.value(QStringLiteral("h")).toDouble(), 108.0);
        // The XML stands alone: it parses by itself, with the prefixes it uses declared.
        QDomDocument dom;
        QString why;
        QVERIFY2(office::parseXml(frame.value(QStringLiteral("xml")).toString().toUtf8(), &dom, &why), qPrintable(why));
        QVERIFY(office::isElement(dom.documentElement(), office::ns::P, QStringLiteral("graphicFrame")));
        QVERIFY(frame.value(QStringLiteral("xml")).toString().contains(QStringLiteral("Cell &amp; &lt;1&gt;")));
        QCOMPARE(dom.documentElement().elementsByTagNameNS(QString::fromLatin1(office::ns::A.data()), QStringLiteral("t")).at(0).toElement().text(), QStringLiteral("Cell & <1>"));
        // The chart and the object on the second slide have pictures of themselves to read in their
        // place; where the reader can't read pictures yet, they are kept like the table.
        const QJsonArray second = d->pages[1]->props.value(QStringLiteral("unread")).toArray();
        QCOMPARE(int(second.size()), readerReadsPictures() ? 0 : 2);
        if (!second.isEmpty()) {
            const QString xml = second[0].toObject().value(QStringLiteral("xml")).toString();
            QVERIFY(xml.contains(QStringLiteral("AlternateContent")));
            QVERIFY(office::parseXml(xml.toUtf8(), &dom, &why));
            QCOMPARE(second[0].toObject().value(QStringLiteral("x")).toDouble(), 96.0);
            QCOMPARE(second[0].toObject().value(QStringLiteral("w")).toDouble(), 480.0);
            const QString object = second[1].toObject().value(QStringLiteral("xml")).toString();
            QVERIFY(object.contains(QStringLiteral("oleObj")));
            QVERIFY(office::parseXml(object.toUtf8(), &dom, &why));
            QCOMPARE(second[1].toObject().value(QStringLiteral("x")).toDouble(), 576.0);
        }
        // Nothing else is added for them.
        QCOMPARE(int(d->pages[0]->items.size()), readerReadsPictures() ? 6 : 5);
    }

    void importReadsNotes()
    {
        QString err;
        auto d = deck::importPresentation(pack(samplePptx()), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(deck::notesText(*d, *d->pages[0]), QStringLiteral("Say hello first\nThen the numbers"));
        // A slide without notes has an empty story to type in.
        QVERIFY(!d->pages[1]->notesStoryId.isEmpty());
        QVERIFY(d->storyDoc(d->pages[1]->notesStoryId));
        QVERIFY(deck::notesText(*d, *d->pages[1]).isEmpty());
        QVERIFY(d->pages[0]->notesStoryId != d->pages[1]->notesStoryId);
    }

    // The theme's colors and fonts are the presentation's, and its part is kept to write back.
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
        QCOMPARE(static_cast<const TextItem *>(slide.items[3].get())->prompt, deck::promptFor(QStringLiteral("pic")));
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

    // At most 10,000 slides and 1,000 layouts are read, however many a file lists.
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
            QCOMPARE(int(d->pages.size()), 10000);
            QVERIFY(!err.isEmpty());
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

QTEST_MAIN(DeckTests)
#include "tst_deck.moc"
