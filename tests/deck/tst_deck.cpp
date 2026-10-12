// JeffDeck's tests: its presentation model on the shared page model, and
// its window.

#include "../../apps/pub/app/pubinfo.h"   // only its inline pubAppInfo(): the other app's colors, to compare
#include "app/deckcanvas.h"
#include "app/deckdialogs.h"
#include "app/deckinfo.h"
#include "app/deckwidgets.h"
#include "app/deckwindow.h"
#include "app/editor.h"
#include "app/icons.h"
#include "app/notespane.h"
#include "app/presenterview.h"
#include "app/recovery.h"
#include "app/ribbon.h"
#include "app/settings.h"
#include "app/slideshow.h"
#include "app/slidespane.h"
#include "core/deck.h"
#include "core/fonts.h"
#include "core/transition.h"
#include "io/cfb.h"
#include "io/pptimport.h"
#include "io/pptxexport.h"
#include "io/pptximport.h"
#include "io/presentationfile.h"
#include "io/zip.h"
#include "office/drawingml.h"
#include "office/drawingml_write.h"
#include "office/package.h"
#include "office/xml.h"
#include "render/renderer.h"
#include "text/textprops.h"

#include <QAbstractButton>
#include <QApplication>
#include <QBuffer>
#include <QCheckBox>
#include <QDeadlineTimer>
#include <QDesktopServices>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QSignalSpy>
#include <QSlider>
#include <QTemporaryDir>
#include <QTest>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QXmlStreamWriter>

using namespace jp;

// Takes the addresses the slide show opens in place of a browser.
class LinkCatcher : public QObject {
    Q_OBJECT
public:
    QStringList urls;
public Q_SLOTS:
    void open(const QUrl &url) { urls << url.toString(); }
};

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

    // A file with two slide masters offers the current slide's master's
    // layouts; placeholder numbers past an int still match; a new
    // presentation is "Presentation1".
    void layoutsPerMasterAndLargePlaceholderNumbers()
    {
        auto d = deck::newPresentation();
        auto second = std::make_shared<MasterPage>(*d->masters[0]);
        second->id = QStringLiteral("SM2");
        second->items.clear();
        d->masters.push_back(second);
        auto layout2 = std::make_shared<MasterPage>();
        layout2->id = QStringLiteral("L2-1");
        layout2->baseId = second->id;
        layout2->props[QStringLiteral("kind")] = QStringLiteral("layout");
        layout2->props[QStringLiteral("type")] = QStringLiteral("obj");
        auto body = std::make_shared<TextItem>();
        body->role = deck::placeholderRole(QStringLiteral("body"), 0) + QStringLiteral(":4294967295");
        body->rect = QRectF(10, 20, 300, 200);
        body->storyId = d->createStory();
        layout2->items.push_back(body);
        d->masters.push_back(layout2);
        QCOMPARE(deck::layouts(*d).size(), 10);
        QCOMPARE(deck::layouts(*d, QStringLiteral("SM2")).size(), 1);
        QCOMPARE(deck::slideMasterOf(*d, 0), QStringLiteral("SM1"));
        QCOMPARE(deck::layoutOfType(*d, QStringLiteral("obj"), QStringLiteral("SM2")), layout2.get());
        // A slide's placeholder with the same big number takes the layout's place.
        const int i = deck::addSlide(*d, *layout2);
        Page &slide = *d->pages[i];
        QCOMPARE(slide.items.size(), size_t(1));
        slide.items[0]->rect = QRectF(0, 0, 1, 1);
        deck::resetSlide(*d, slide);
        QCOMPARE(slide.items[0]->rect, QRectF(10, 20, 300, 200));

        DeckWindow w;
        QCOMPARE(w.editor()->displayName(), QStringLiteral("Presentation1"));
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

    // ---- Old binary presentations ([MS-PPT]), built record by record ----
private:
    static QByteArray le(quint64 v, int n)
    {
        QByteArray b;
        for (int i = 0; i < n; ++i) b.append(char((v >> (8 * i)) & 0xFF));
        return b;
    }
    // A record: its header (version, instance, type, length), then its data.
    static QByteArray pptRec(quint16 type, const QByteArray &data, quint16 inst = 0, quint8 ver = 0)
    {
        return le(quint16(ver | (inst << 4)), 2) + le(type, 2) + le(quint32(data.size()), 4) + data;
    }
    static QByteArray pptBox(quint16 type, const QByteArray &kids, quint16 inst = 0) { return pptRec(type, kids, inst, 0xF); }
    static QByteArray utf16(const QString &s)
    {
        QByteArray b;
        for (const QChar c : s) b += le(c.unicode(), 2);
        return b;
    }
    // A color as the format stores it: red, green, blue, and a scheme index (0xFE: the RGB values are the color).
    static QByteArray raw(std::initializer_list<int> values)
    {
        QByteArray b;
        for (const int v : values) b.append(char(v));
        return b;
    }
    static QByteArray pptColor(int r, int g, int b, int index) { return raw({r, g, b, index}); }
    static QByteArray fontEntity(const QString &name)
    {
        QByteArray face = utf16(name);
        face.append(QByteArray(64 - face.size(), '\0'));
        return pptRec(4023, face + QByteArray(4, '\0'));
    }
    // The stream's name is fixed by [MS-PPT] section 2.1.2: the program's name and "Document".
    static QString pptDocumentStream() { return QStringLiteral("Power") + QStringLiteral("Point Document"); }

    // One text of a slide: a TextHeaderAtom with its type, the characters
    // (8-bit unless `wide`), and the style runs (a StyleTextPropAtom).
    static QByteArray pptText(int type, const QString &s, const QByteArray &styles = QByteArray(), bool wide = false)
    {
        QByteArray out = pptRec(3999, le(type, 4));
        out += wide ? pptRec(4000, utf16(s)) : pptRec(4008, s.toLatin1());
        if (!styles.isEmpty()) out += pptRec(4001, styles);
        return out;
    }
    static QByteArray pfRun(int count, int level, quint32 masks, const QByteArray &data = QByteArray()) { return le(count, 4) + le(level, 2) + le(masks, 4) + data; }
    static QByteArray cfRun(int count, quint32 masks, const QByteArray &data = QByteArray()) { return le(count, 4) + le(masks, 4) + data; }

    static QByteArray slidePersist(quint32 persistId, int texts, quint32 slideId) { return pptRec(1011, le(persistId, 4) + le(4, 4) + le(texts, 4) + le(slideId, 4) + le(0, 4)); }
    // A slide's or master's SlideAtom: the layout geometry, the placeholders it names, the master and notes it refers to, and its flags.
    static QByteArray slideAtom(int geom, const QByteArray &placeholders, quint32 master, quint32 notes, int flags)
    {
        QByteArray ph = placeholders;
        ph.append(QByteArray(8 - ph.size(), '\0'));
        return pptRec(1007, le(geom, 4) + ph + le(master, 4) + le(notes, 4) + le(flags, 2) + le(0, 2), 0, 2);
    }
    static QByteArray colorScheme(int instance)
    {
        // background, text, shadow, title text, fills, accent, accent and hyperlink, accent and followed hyperlink
        return pptRec(2032, pptColor(0xFA, 0xFA, 0xFA, 0) + pptColor(0x11, 0x22, 0x33, 0) + pptColor(0xDD, 0xDD, 0xCC, 0) + pptColor(0x44, 0x55, 0x66, 0)
                                 + pptColor(0x77, 0x88, 0x99, 0) + pptColor(0xAA, 0xBB, 0xCC, 0) + pptColor(1, 2, 3, 0) + pptColor(4, 5, 6, 0), instance);
    }
    // A drawing: a group shape and the given shapes.
    static QByteArray pptDrawing(const QByteArray &shapes = QByteArray())
    {
        const QByteArray group = pptBox(0xF004, pptRec(0xF009, QByteArray(16, '\0'), 0, 1) + pptRec(0xF00A, le(0x400, 4) + le(5, 4), 0, 2));
        return pptBox(1036, pptBox(0xF002, pptRec(0xF008, le(2, 4) + le(0x402, 4)) + pptBox(0xF003, group + shapes)));
    }
    // A placeholder shape: a rectangle at `top, left, right, bottom` (master units), the placeholder it is, and what it holds (and its property record).
    static QByteArray pptShape(quint32 id, int placeholder, const QByteArray &textbox, int top = 100, int left = 200, int right = 5000, int bottom = 900, const QByteArray &props = QByteArray())
    {
        const QByteArray anchor = le(top, 2) + le(left, 2) + le(right, 2) + le(bottom, 2);
        const QByteArray data = pptBox(0xF011, pptRec(3011, le(0xFFFFFFFF, 4) + raw({placeholder, 0, 0, 0})));
        return pptBox(0xF004, pptRec(0xF00A, le(id, 4) + le(0xA00, 4), 1, 2) + props + pptRec(0xF010, anchor) + data + (textbox.isEmpty() ? QByteArray() : pptBox(0xF00D, textbox)));
    }
    static QByteArray outlineRef(int index) { return pptRec(3998, le(index, 4)); }

    // A UserEditAtom, and a PersistDirectoryAtom mapping the given persist ids (each next to the last) to offsets.
    static QByteArray userEdit(quint32 lastEdit, quint32 directory, quint32 document, int extra = 0)
    {
        QByteArray d = le(0x100, 4) + le(0, 2) + raw({0, 3}) + le(lastEdit, 4) + le(directory, 4) + le(document, 4) + le(0x20, 4) + le(1, 2) + le(0, 2);
        if (extra) d += le(extra, 4);
        return pptRec(4085, d);
    }
    static QByteArray persistDirectory(const QVector<QPair<quint32, quint32>> &entries)
    {
        QByteArray d;
        for (const auto &e : entries) d += le(e.first | (1u << 20), 4) + le(e.second, 4);
        return pptRec(6002, d);
    }

    // A presentation of two slides on one master, in the order First (persist
    // id 5, with notes) and Second (4, hidden by the newer of two edits); with
    // `titleMaster`, a slide on a title master; with `links`, a slide whose text
    // has a field, a link, a language, and a ruler, and that fades in. `cycle`
    // makes the older edit point back at the newer. `notesByAtom` leaves the
    // first slide without the id of its notes, which then name the slide.
    // `bodyGroup` stands for the first slide's body text; `drawingGroup` is the document's drawing group, `firstSlideShapes` more
    // shapes for the first slide's drawing, `masterTitleProps` a property record for the main master's title shape, and `masterBackground`
    // a shape for its drawing.
    static QByteArray buildPpt(bool titleMaster = false, bool cycle = false, bool links = false, bool notesByAtom = false, const QByteArray &bodyGroup = QByteArray(),
                               const QByteArray &drawingGroup = QByteArray(), const QByteArray &firstSlideShapes = QByteArray(), const QByteArray &masterTitleProps = QByteArray(), const QByteArray &masterBackground = QByteArray())
    {
        constexpr quint32 kMaster = 0x80000001, kTitleMaster = 0x80000002;
        const QByteArray fonts = pptBox(2005, fontEntity(QStringLiteral("Alpha Sans")) + fontEntity(QStringLiteral("Beta Serif")));
        // The default body text: bullets, 28 and 24 points, the first font, the text color.
        const QByteArray bodyStyle = le(2, 2)
            + le(0x593, 4) + le(3, 2) + le(0x2022, 2) + le(0, 2) + le(360, 2) + le(0, 2)
            + le(0x70000, 4) + le(0, 2) + le(28, 2) + pptColor(0, 0, 0, 1)
            + le(0x581, 4) + le(1, 2) + le(0x2013, 2) + le(720, 2) + le(360, 2)
            + le(0x20000, 4) + le(24, 2);
        const QByteArray environment = pptBox(1010, fonts + pptRec(4003, bodyStyle, 1));
        const QByteArray document = pptBox(1000, pptRec(1001, le(5760, 4) + le(4320, 4) + le(4320, 4) + le(5760, 4) + le(5, 4) + le(10, 4) + le(0, 4) + le(0, 4) + le(1, 2) + le(0, 2) + le(0, 4))
            + environment
            + drawingGroup
            + (links ? pptBox(1033, pptRec(1034, le(1, 4)) + pptBox(4055, pptRec(4051, le(7, 4)) + pptRec(4026, utf16(QStringLiteral("the site")), 0) + pptRec(4026, utf16(QStringLiteral("https://example.org/site")), 1)))
                     : QByteArray())
            + pptBox(4080, slidePersist(3, 0, kMaster) + (titleMaster ? slidePersist(7, 0, kTitleMaster) : QByteArray()), 1)
            + pptBox(4080,
                     slidePersist(5, 2, 0x100) + pptText(0, QStringLiteral("Quarterly Results"), pfRun(18, 0, 0) + cfRun(18, 0), true)
                         + (!bodyGroup.isEmpty() ? bodyGroup : pptText(1, QStringLiteral("First point\rSecond point\rThird point"), pfRun(12, 0, 0) + pfRun(13, 1, 0) + pfRun(12, 1, 0) + cfRun(12, 0) + cfRun(6, 1, le(1, 2)) + cfRun(19, 0)))
                         + slidePersist(4, 2, 0x101) + pptText(0, QStringLiteral("Plan"), QByteArray(), true)
                         // The first paragraph turns its bullet off: the bullet flags name only hasBullet, and it is clear.
                         + pptText(1, QStringLiteral("Ship\vit"), pfRun(8, 0, 1, le(0, 2)) + cfRun(8, 0))
                         + (titleMaster ? slidePersist(8, 2, 0x102) + pptText(6, QStringLiteral("Welcome"), QByteArray(), true) + pptText(5, QStringLiteral("To the talk")) : QByteArray())
                         + (links ? slidePersist(9, 2, 0x103) + pptText(0, QStringLiteral("Links"), QByteArray(), true)
                                + pptText(1, QStringLiteral("Go to the site. Slide *"), pfRun(24, 0, 0) + cfRun(24, 0))
                                // The ruler sets the first level's margin and indent; the language is French for the first 15
                                // characters; the slide number is the last; the link covers "the site".
                                + pptRec(4006, le(0x108, 4) + le(720, 2) + le(288, 2)) + pptRec(4010, le(15, 4) + le(2, 4) + le(0x040C, 2) + le(9, 4) + le(0, 4))
                                + pptRec(4056, le(22, 4)) + pptBox(4082, pptRec(4083, le(0, 4) + le(7, 4) + raw({4, 0, 0, 0, 8, 0, 0, 0})))
                                + pptRec(4063, le(6, 4) + le(14, 4))
                                  : QByteArray()),
                     0)
            + pptBox(4080, slidePersist(6, 0, 0x200), 2)
            + pptRec(1002, QByteArray()));

        const QByteArray mainMaster = pptBox(1016, slideAtom(1, QByteArray("\x01\x02\x07\x08\x09", 5), 0, 0, 3) + colorScheme(1)
            // The title text: centered, the second font, 44 points, the title text color.
            + pptRec(4003, le(1, 2) + le(0x800, 4) + le(1, 2) + le(0x70000, 4) + le(1, 2) + le(44, 2) + pptColor(0, 0, 0, 3), 0)
            + pptDrawing(pptShape(0x402, 1, QByteArray(), 100, 200, 5000, 900, masterTitleProps) + pptShape(0x403, 2, QByteArray()) + masterBackground));
        const QByteArray titleMasterRec = pptBox(1016, slideAtom(3, QByteArray("\x03\x04\x07\x08\x09", 5), 0, 0, 1) + colorScheme(1)
            // The centered title and the subtitle, with only their sizes (the levels of these two start with their number).
            + pptRec(4003, le(1, 2) + le(0, 2) + le(0x800, 4) + le(1, 2) + le(0x20000, 4) + le(54, 2), 6)
            + pptRec(4003, le(1, 2) + le(0, 2) + le(0, 4) + le(0x20000, 4) + le(30, 2), 5)
            + pptDrawing());

        auto slide = [&](int geom, const QByteArray &placeholders, quint32 master, quint32 notes, bool hidden, const QByteArray &shapes, const QByteArray &transition = QByteArray()) {
            QByteArray info = le(0, 4) + le(0, 4) + raw({2, 0}) + le(hidden ? 0x4 : 0x400, 2) + raw({2, 0, 0, 0});
            return pptBox(1006, slideAtom(geom, placeholders, master, notes, 7) + pptRec(1017, transition.isEmpty() ? info : transition) + pptDrawing(shapes));
        };
        const QByteArray titleOnBody = pptShape(0x402, 0xD, outlineRef(0)) + pptShape(0x403, 0xE, outlineRef(1), 1000, 200, 5000, 3800);
        const QByteArray first = slide(1, QByteArray("\x0D\x0E", 2), kMaster, notesByAtom ? 0 : 0x200, false, titleOnBody + firstSlideShapes);
        const QByteArray secondOld = slide(1, QByteArray("\x0D\x0E", 2), kMaster, 0, false, titleOnBody);
        const QByteArray second = slide(1, QByteArray("\x0D\x0E", 2), kMaster, 0, true, titleOnBody);
        const QByteArray third = slide(0, QByteArray("\x0F\x10", 2), kTitleMaster, 0, false, pptShape(0x402, 0xF, outlineRef(0)) + pptShape(0x403, 0x10, outlineRef(1)));
        // Three seconds, then the next slide; effect 6, direction 1, speed 2.
        const QByteArray linksSlide = slide(1, QByteArray("\x0D\x0E", 2), kMaster, 0, false, titleOnBody, le(3000, 4) + le(0, 4) + raw({1, 6}) + le(0x501, 2) + raw({2, 0, 0, 0}));
        const QByteArray notes = pptBox(1008, pptRec(1009, le(0x100, 4) + le(0, 4), 0, 1)
            + pptDrawing(pptShape(0x402, 0xC, pptRec(3999, le(2, 4)) + pptRec(4000, utf16(QStringLiteral("Remember the demo"))))));

        QByteArray s = document;
        QHash<quint32, quint32> at;
        auto place = [&](quint32 persistId, const QByteArray &rec) {
            at[persistId] = quint32(s.size());
            s += rec;
        };
        at[1] = 0;
        place(3, mainMaster);
        if (titleMaster) {
            place(7, titleMasterRec);
            place(8, third);
        }
        place(4, secondOld);
        place(5, first);
        place(6, notes);
        if (links) place(9, linksSlide);
        QVector<QPair<quint32, quint32>> older{{1, 0}, {3, at[3]}, {4, at[4]}, {5, at[5]}, {6, at[6]}};
        if (titleMaster) older << qMakePair(quint32(7), at[7]) << qMakePair(quint32(8), at[8]);
        if (links) older << qMakePair(quint32(9), at[9]);
        std::sort(older.begin(), older.end());
        const quint32 olderDirectory = quint32(s.size());
        // Each entry names one id (a run of one), so the ids need not be next to each other.
        s += persistDirectory(older);
        const quint32 olderEdit = quint32(s.size());
        s += userEdit(0, olderDirectory, 1);
        place(4, second);
        const quint32 newerDirectory = quint32(s.size());
        s += persistDirectory({{4, at[4]}});
        const quint32 newerEdit = quint32(s.size());
        s += userEdit(olderEdit, newerDirectory, 1);
        if (cycle) s.replace(olderEdit + 8 + 8, 4, le(newerEdit, 4));

        cfb::File f;
        cfb::Entry root;
        root.type = cfb::Entry::Root;
        root.name = QStringLiteral("Root Entry");
        f.entries << root;
        auto stream = [&](const QString &name, const QByteArray &data) {
            cfb::Entry e;
            e.type = cfb::Entry::Stream;
            e.name = name;
            e.data = data;
            f.entries[0].children << int(f.entries.size());
            f.entries << e;
        };
        const QByteArray user = QByteArray("Tester");
        stream(QStringLiteral("Current User"), pptRec(4086, le(20, 4) + le(0xE391C05F, 4) + le(newerEdit, 4) + le(user.size(), 2) + le(0x3F4, 2) + raw({3, 0, 0, 0}) + user + le(8, 4)));
        stream(pptDocumentStream(), s);
        stream(QStringLiteral("Pictures"), QByteArray());
        return cfb::write(f);
    }
    static QByteArray withStream(const QByteArray &file, const QString &name, const QByteArray &data)
    {
        cfb::File f;
        cfb::read(file, &f);
        f.setStream(name, data);
        return cfb::write(f);
    }

    // The text of a slide's placeholders by role.
    static QString placeholderText(const Document &d, const Page &slide, const QString &role)
    {
        for (const auto &it : slide.items)
            if (it->role == role && it->type() == ItemType::Text) return d.storyDoc(static_cast<const TextItem *>(it.get())->storyId)->toPlainText();
        return QStringLiteral("<none>");
    }
    static QTextDocument *placeholderStory(const Document &d, const Page &slide, const QString &role)
    {
        for (const auto &it : slide.items)
            if (it->role == role && it->type() == ItemType::Text) return d.storyDoc(static_cast<const TextItem *>(it.get())->storyId);
        return nullptr;
    }

private Q_SLOTS:
    // An old binary presentation opens as slides over a master and layouts:
    // the slide size, the slides in the order the file lists them, the newest
    // edit of each (the second slide is hidden by the later edit), the notes
    // of the one that has them, and the colors of the master's scheme.
    void binaryPresentationOpensWithMastersAndLayouts()
    {
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(d->pageSize(), QSizeF(720, 540));
        QCOMPARE(int(d->pages.size()), 2);
        QCOMPARE(placeholderText(*d, *d->pages[0], QStringLiteral("ph:title")), QStringLiteral("Quarterly Results"));
        QCOMPARE(placeholderText(*d, *d->pages[1], QStringLiteral("ph:title")), QStringLiteral("Plan"));
        QCOMPARE(d->pages[1]->title, QStringLiteral("Plan"));
        QVERIFY(d->pages[1]->hidden);
        QVERIFY(!d->pages[0]->hidden);
        QCOMPARE(deck::notesText(*d, *d->pages[0]), QStringLiteral("Remember the demo"));
        QVERIFY(deck::notesText(*d, *d->pages[1]).isEmpty());
        // Notes also find their slide by the slide they name.
        auto byAtom = deck::importBinaryPresentation(buildPpt(false, false, false, true), &err);
        QVERIFY2(byAtom, qPrintable(err));
        QCOMPARE(deck::notesText(*byAtom, *byAtom->pages[0]), QStringLiteral("Remember the demo"));
        QVERIFY(deck::notesText(*byAtom, *byAtom->pages[1]).isEmpty());

        int masters = 0;
        for (const auto &m : d->masters) masters += deck::isSlideMaster(*m);
        QCOMPARE(masters, 1);
        const MasterPage *layout = deck::layoutOfType(*d, QStringLiteral("obj"));
        QVERIFY(layout);
        QVERIFY(d->masterFor(*d->pages[0]) == layout);
        QVERIFY(d->masterFor(*d->pages[1]) == layout);
        QVERIFY(deck::isSlideMaster(*d->master(layout->baseId)));
        QStringList placeholders;
        for (const auto &it : layout->items) placeholders << deck::placeholderType(*it);
        QVERIFY(placeholders.contains(QStringLiteral("title")));
        QVERIFY(placeholders.contains(QStringLiteral("obj")));
        // Slides can also be added in the layouts every presentation has.
        for (const char *type : {"title", "titleOnly", "blank"}) QVERIFY2(deck::layoutOfType(*d, QString::fromLatin1(type)), type);

        // The scheme's eight colors: the presentation program turns them into a theme's dark 1 (text), light 1 (background),
        // dark 2 (title text), light 2 (shadow), accent 1 (fills), accent 2 (accent), hyperlink, and followed hyperlink.
        const deck::DeckTheme theme = deck::themeOf(*d);
        QCOMPARE(theme.colors[0], QColor(0x11, 0x22, 0x33));
        QCOMPARE(theme.colors[1], QColor(0xFA, 0xFA, 0xFA));
        QCOMPARE(theme.colors[2], QColor(0x44, 0x55, 0x66));
        QCOMPARE(theme.colors[3], QColor(0xDD, 0xDD, 0xCC));
        QCOMPARE(theme.colors[4], QColor(0x77, 0x88, 0x99));
        QCOMPARE(theme.colors[5], QColor(0xAA, 0xBB, 0xCC));
        QCOMPARE(theme.colors[10], QColor(1, 2, 3));
        QCOMPARE(theme.colors[11], QColor(4, 5, 6));
        // The fonts of the default title and body styles.
        QCOMPARE(theme.headingFont, QStringLiteral("Beta Serif"));
        QCOMPARE(theme.bodyFont, QStringLiteral("Alpha Sans"));
        QVERIFY(d->fromFile);
        // Tabs go to every inch when the styles don't say otherwise.
        QCOMPARE(d->defaultTabStop, 72.0);
    }

    // Text takes its look from the default styles of its kind and level, then
    // from its own paragraph and character runs.
    void binaryPresentationTextFormats()
    {
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(), &err);
        QVERIFY2(d, qPrintable(err));
        const Page &first = *d->pages[0];

        QTextDocument *title = placeholderStory(*d, first, QStringLiteral("ph:title"));
        QVERIFY(title);
        const QTextCharFormat tf = title->begin().begin().fragment().charFormat();
        QCOMPARE(tf.fontFamilies().toStringList(), QStringList{"Beta Serif"});
        QCOMPARE(tf.fontPointSize(), 44.0);
        QCOMPARE(ColorRef::fromString(tf.stringProperty(tp::ColorRefP)).rgbValue(), QColor(0x44, 0x55, 0x66));   // the scheme's title text
        QVERIFY((title->begin().blockFormat().alignment() & Qt::AlignHorizontal_Mask) == Qt::AlignHCenter);
        QVERIFY(!title->begin().textList());

        QTextDocument *body = placeholderStory(*d, first, QStringLiteral("ph:body:1"));
        QVERIFY(body);
        QCOMPARE(body->toPlainText(), QStringLiteral("First point\nSecond point\nThird point"));
        const QTextBlock one = body->begin(), two = one.next(), three = two.next();
        QVERIFY(one.isValid() && two.isValid() && three.isValid() && !three.next().isValid());
        // The first level: a round bullet hanging 45 points, text of 28 points in the first font and the text color.
        QVERIFY(one.textList());
        QCOMPARE(one.textList()->format().style(), QTextListFormat::ListDisc);
        QCOMPARE(one.textList()->format().intProperty(tp::ListLevel), 0);
        QCOMPARE(one.blockFormat().leftMargin(), 45.0);
        QCOMPARE(one.blockFormat().textIndent(), -45.0);
        const QTextCharFormat c1 = one.begin().fragment().charFormat();
        QCOMPARE(c1.fontFamilies().toStringList(), QStringList{"Alpha Sans"});
        QCOMPARE(c1.fontPointSize(), 28.0);
        QVERIFY(c1.fontWeight() != QFont::Bold);
        QCOMPARE(ColorRef::fromString(c1.stringProperty(tp::ColorRefP)).rgbValue(), QColor(0x11, 0x22, 0x33));
        // The second level: a dash, 24 points, 90 points in, and a bold first word.
        QVERIFY(two.textList());
        QCOMPARE(two.textList()->format().intProperty(tp::ListLevel), 1);
        QCOMPARE(two.textList()->format().stringProperty(tp::BulletChar), QString(QChar(0x2013)));
        QCOMPARE(two.blockFormat().leftMargin(), 90.0);
        QCOMPARE(two.blockFormat().textIndent(), -45.0);
        QStringList runs;
        for (auto f = two.begin(); !f.atEnd(); ++f) {
            const QTextCharFormat cf = f.fragment().charFormat();
            runs << QStringLiteral("%1|%2|%3").arg(f.fragment().text(), cf.fontWeight() >= QFont::Bold ? QStringLiteral("bold") : QStringLiteral("plain")).arg(cf.fontPointSize());
        }
        QCOMPARE(runs, (QStringList{"Second|bold|24", " point|plain|24"}));
        // Paragraphs of one level and mark go on in the one list; another level starts its own.
        QVERIFY(three.textList() == two.textList());
        QVERIFY(one.textList() != two.textList());

        // The second slide's body: a line break within a paragraph that has no bullet.
        QTextDocument *plan = placeholderStory(*d, *d->pages[1], QStringLiteral("ph:body:1"));
        QVERIFY(plan);
        QCOMPARE(plan->toPlainText(), QStringLiteral("Ship\nit"));
        QCOMPARE(plan->blockCount(), 1);
        QVERIFY(plan->toRawText().contains(QChar(QChar::LineSeparator)));
        QVERIFY(!plan->begin().textList());
    }

    // A title master is the base of the title layout's own objects; the
    // centered title and the subtitle are placeholders of that layout.
    void binaryPresentationTitleMasterMakesTheTitleLayout()
    {
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(true), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 3);
        int masters = 0;
        for (const auto &m : d->masters) masters += deck::isSlideMaster(*m);
        QCOMPARE(masters, 1);
        const Page &welcome = *d->pages[2];
        const MasterPage *layout = d->masterFor(welcome);
        QVERIFY(layout);
        QCOMPARE(deck::layoutType(*layout), QStringLiteral("title"));
        QVERIFY(deck::isSlideMaster(*d->master(layout->baseId)));
        QStringList types;
        for (const auto &it : layout->items) types << deck::placeholderType(*it);
        QVERIFY(types.contains(QStringLiteral("ctrTitle")));
        QVERIFY(types.contains(QStringLiteral("subTitle")));
        QCOMPARE(placeholderText(*d, welcome, QStringLiteral("ph:ctrTitle")), QStringLiteral("Welcome"));
        QCOMPARE(placeholderText(*d, welcome, QStringLiteral("ph:subTitle:1")), QStringLiteral("To the talk"));
        // The title master's own size for the centered title (54 points), centered.
        QTextDocument *ctr = placeholderStory(*d, welcome, QStringLiteral("ph:ctrTitle"));
        QVERIFY(ctr);
        QCOMPARE(ctr->begin().begin().fragment().charFormat().fontPointSize(), 54.0);
        QVERIFY((ctr->begin().blockFormat().alignment() & Qt::AlignHorizontal_Mask) == Qt::AlignHCenter);
    }

    // A text's fields, links, languages, and ruler come through, and so does how a
    // slide moves on (kept as the file has it).
    void binaryPresentationFieldsLinksRulerAndTransition()
    {
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, true), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 3);
        const Page &slide = *d->pages[2];
        QTextDocument *body = placeholderStory(*d, slide, QStringLiteral("ph:body:1"));
        QVERIFY(body);
        QCOMPARE(body->toPlainText(), QStringLiteral("Go to the site. Slide %1").arg(QChar(QChar::ObjectReplacementCharacter)));
        QStringList runs;
        for (auto f = body->begin().begin(); !f.atEnd(); ++f) {
            const QTextCharFormat cf = f.fragment().charFormat();
            runs << QStringLiteral("%1|%2|%3|%4").arg(f.fragment().text(), cf.isAnchor() ? cf.anchorHref() : QString(), cf.stringProperty(tp::Language), cf.stringProperty(tp::Field));
        }
        QCOMPARE(runs, (QStringList{QStringLiteral("Go to ||fr-FR|"), QStringLiteral("the site|https://example.org/site|fr-FR|"), QStringLiteral(".||fr-FR|"),
                                    QStringLiteral(" Slide |||"), QString(QChar(QChar::ObjectReplacementCharacter)) + QStringLiteral("|||page")}));
        // The ruler's first level: text at 90 points, its first line (and bullet) at 36.
        QCOMPARE(body->begin().blockFormat().leftMargin(), 90.0);
        QCOMPARE(body->begin().blockFormat().textIndent(), -54.0);

        // Effect 6 (a fade), medium speed, moving on by itself after 3 seconds and on a click.
        const deck::Transition fade = deck::transitionOf(slide);
        QCOMPARE(fade.type, QStringLiteral("fade"));
        QCOMPARE(fade.duration, 0.5);
        QVERIFY(fade.advanceAfter.has_value());
        QCOMPARE(*fade.advanceAfter, 3.0);
        QVERIFY(fade.advanceOnClick);
        // What the file said is kept too.
        const QJsonObject raw = slide.props.value(QStringLiteral("binaryTransition")).toObject();
        QCOMPARE(raw.value(QStringLiteral("effect")).toInt(), 6);
        QCOMPARE(raw.value(QStringLiteral("direction")).toInt(), 1);
        QCOMPARE(raw.value(QStringLiteral("speed")).toInt(), 2);
        QCOMPARE(raw.value(QStringLiteral("advanceMs")).toInt(), 3000);
        QVERIFY(!d->pages[0]->props.contains(QStringLiteral("binaryTransition")));
        QVERIFY(!d->pages[0]->props.contains(QStringLiteral("transition")));
    }

    // Files that can't be read say why: protected with a password, cut short,
    // edits that go round in a circle, no record of who edited last.
    void binaryPresentationRefusesDamagedFiles()
    {
        const QByteArray good = buildPpt();
        QString err;
        QVERIFY(deck::importBinaryPresentation(good, &err));

        cfb::File f;
        QVERIFY(cfb::read(good, &f));
        // A password: the header token of the current-user record says the presentation is encrypted.
        QByteArray user = f.stream(QStringLiteral("Current User"));
        user.replace(12, 4, le(0xF3D1C4DF, 4));
        QVERIFY(!deck::importBinaryPresentation(withStream(good, QStringLiteral("Current User"), user), &err));
        QVERIFY2(err.contains(QStringLiteral("password")), qPrintable(err));

        err.clear();
        const QByteArray stream = f.stream(pptDocumentStream());
        QVERIFY(!deck::importBinaryPresentation(withStream(good, pptDocumentStream(), stream.left(stream.size() / 2)), &err));
        QVERIFY(!err.isEmpty());

        err.clear();
        QVERIFY(!deck::importBinaryPresentation(buildPpt(false, true), &err));
        QVERIFY2(err.contains(QStringLiteral("circle")), qPrintable(err));

        err.clear();
        cfb::File without;
        QVERIFY(cfb::read(good, &without));
        without.entries[0].children.removeAll(without.find(QStringLiteral("Current User")));
        QVERIFY(!deck::importBinaryPresentation(cfb::write(without), &err));
        QVERIFY(!err.isEmpty());

        err.clear();
        QVERIFY(!deck::importBinaryPresentation(QByteArray("not a compound file at all"), &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!deck::importBinaryPresentation(QByteArray(), &err));
        QVERIFY(!err.isEmpty());
        // A compound file that holds no presentation.
        cfb::File other;
        QVERIFY(cfb::read(good, &other));
        other.entries[0].children.removeAll(other.find(pptDocumentStream()));
        err.clear();
        QVERIFY(!deck::importBinaryPresentation(cfb::write(other), &err));
        QVERIFY(!err.isEmpty());
    }

    // A text of thousands of paragraphs, a bullet on each, is read to a limit and
    // quickly (a list takes its items at a cost that grows with its length).
    void binaryPresentationLimitsHugeTexts()
    {
        QString text;
        for (int i = 0; i < 30000; ++i) text += QStringLiteral("a\r");
        const QByteArray big = pptRec(3999, le(1, 4)) + pptRec(4000, utf16(text)) + pptRec(4001, pfRun(text.size() + 1, 0, 0) + cfRun(text.size() + 1, 0));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, big), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(err.contains(QStringLiteral("paragraphs")), qPrintable(err));
        const QTextDocument *body = placeholderStory(*d, *d->pages[0], QStringLiteral("ph:body:1"));
        QVERIFY(body);
        QCOMPARE(body->blockCount(), 10000);
        QVERIFY(body->begin().textList());
    }

    // Damage to the bytes of the streams never crashes or hangs the reader: a
    // file is a presentation, or says it isn't.
    void binaryPresentationSurvivesRandomDamage()
    {
        const QByteArray good = buildPpt(true);
        cfb::File f;
        QVERIFY(cfb::read(good, &f));
        const QByteArray stream = f.stream(pptDocumentStream());
        quint32 seed = 12345;
        auto next = [&] { return seed = seed * 1664525u + 1013904223u; };
        int opened = 0;
        for (int round = 0; round < 300; ++round) {
            QByteArray bad = stream;
            const int hits = 1 + int(next() % 6);
            for (int i = 0; i < hits; ++i) bad[int((next() >> 8) % quint32(bad.size()))] = char(next() >> 16);
            if (round % 7 == 0) bad.truncate(int((next() >> 8) % quint32(bad.size())));
            QString err;
            auto d = deck::importBinaryPresentation(withStream(good, pptDocumentStream(), bad), &err);
            if (d) {
                ++opened;
                QVERIFY(d->pages.size() <= 3);
            } else {
                QVERIFY(!err.isEmpty());
            }
        }
        QVERIFY(opened > 0);
    }

    // Presentation files open through one function that looks at the file, not
    // its name: the old binary format by its compound file signature.
    void presentationFilesOpenByTheirFormat()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("talk.ppt"));
        QFile out(path);
        QVERIFY(out.open(QIODevice::WriteOnly));
        out.write(buildPpt());
        out.close();
        QString err;
        auto d = deck::importAnyPresentationFile(path, &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 2);

        // The name doesn't decide it.
        const QString renamed = dir.filePath(QStringLiteral("talk.pptx"));
        QVERIFY(QFile::copy(path, renamed));
        auto again = deck::importAnyPresentationFile(renamed, &err);
        QVERIFY2(again, qPrintable(err));
        QCOMPARE(int(again->pages.size()), 2);

        // A newer presentation goes to the reader for its format; other files say they aren't presentations.
        err.clear();
        QFile junk(dir.filePath(QStringLiteral("junk.pptx")));
        QVERIFY(junk.open(QIODevice::WriteOnly));
        junk.write("PK nothing");
        junk.close();
        QVERIFY(!deck::importAnyPresentationFile(junk.fileName(), &err));
        QVERIFY(!err.isEmpty());
        err.clear();
        QVERIFY(!deck::importAnyPresentationFile(dir.filePath(QStringLiteral("missing.ppt")), &err));
        QVERIFY(!err.isEmpty());
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

    // Each app names its color and badge for the shared window: the File
    // button and every accent in it follow, and the dark shade reads on the
    // dark ribbon.
    void appInfoColorsPerApp()
    {
        auto contrast = [](const QColor &a, const QColor &b) {
            auto lum = [](const QColor &c) {
                auto f = [](double v) { return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
                return 0.2126 * f(c.redF()) + 0.7152 * f(c.greenF()) + 0.0722 * f(c.blueF());
            };
            const double la = lum(a), lb = lum(b);
            return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
        };
        const AppInfo pub = pubAppInfo(), deckInfo = deckAppInfo();
        QCOMPARE(pub.accent, QColor(0x9E, 0x1F, 0x63));
        QCOMPARE(pub.accentDark, QColor(0xE0, 0x6A, 0xA5));
        QCOMPARE(pub.badge, QStringLiteral("JP"));
        QCOMPARE(deckInfo.accent, QColor(0xB6, 0x42, 0x0F));
        QCOMPARE(deckInfo.badge, QStringLiteral("JD"));
        QVERIFY(deckInfo.accentDark.isValid());
        QVERIFY(contrast(deckInfo.accentDark, QColor(0x26, 0x2B, 0x33)) >= 4.5);   // on the dark ribbon
        QVERIFY(contrast(deckInfo.accent, QColor(0xFB, 0xFB, 0xFC)) >= 4.5);       // on the light one
        QVERIFY(contrast(deckInfo.highlight, Qt::white) >= 4.5);                   // selected text on the dark selection
        // The shared parts read the running app's.
        setUiDark(false);
        QCOMPARE(uiAccent(), deckInfo.accent);
        {
            DeckWindow w;
            auto *file = w.ribbon()->findChild<QAbstractButton *>(QStringLiteral("jpRibbonFile"));
            QVERIFY(file);
            file->resize(file->sizeHint());
            // The badge is a circle of the accent at the button's left.
            const QColor badge = file->grab().toImage().pixelColor(7, file->height() / 2);
            QVERIFY2(std::abs(badge.red() - deckInfo.accent.red()) < 12 && std::abs(badge.green() - deckInfo.accent.green()) < 12
                         && std::abs(badge.blue() - deckInfo.accent.blue()) < 12,
                     qPrintable(badge.name()));
        }
        setUiDark(true);
        QCOMPARE(uiAccent(), deckInfo.accentDark);
        setUiDark(false);
    }

    // The slide thumbnails: a row for each slide, the current row following
    // the editor and the editor following Up and Down; the current slide
    // outlined in the accent without a fill over it; a hidden slide marked.
    void thumbnailsFollowTheEditor()
    {
        setUiDark(false);
        DeckWindow w;
        Editor *ed = w.editor();
        SlidesPane *pane = w.slidesPane();
        QCOMPARE(pane->count(), 1);
        QCOMPARE(pane->currentRow(), 0);
        w.newSlide();
        w.newSlide();
        QTRY_COMPARE(pane->count(), 3);
        QCOMPARE(pane->currentRow(), 2);
        ed->setCurrentPage(0);
        QCOMPARE(pane->currentRow(), 0);
        QCOMPARE(pane->item(1)->text(), QStringLiteral("2"));   // the number, which the row draws at the thumbnail's left
        QTest::keyClick(pane, Qt::Key_Down);
        QCOMPARE(ed->currentPage(), 1);
        QTest::keyClick(pane, Qt::Key_Down);
        QCOMPARE(ed->currentPage(), 2);
        QTest::keyClick(pane, Qt::Key_Down);   // the last stays
        QCOMPARE(ed->currentPage(), 2);
        QTest::keyClick(pane, Qt::Key_Up);
        QCOMPARE(ed->currentPage(), 1);
        QCOMPARE(pane->accessibleName(), QStringLiteral("Slides"));
        QCOMPARE(pane->item(1)->data(Qt::AccessibleTextRole).toString(), QStringLiteral("Slide 2"));
        w.toggleHidden();
        QTRY_VERIFY(pane->item(1)->data(SlidesPane::HiddenRole).toBool());
        QCOMPARE(pane->item(1)->data(Qt::AccessibleTextRole).toString(), QStringLiteral("Slide 2, hidden"));
        QVERIFY(!pane->item(0)->data(SlidesPane::HiddenRole).toBool());

        // Drawn: the current thumbnail has the accent around it and its own white inside; the others a gray edge.
        ed->setCurrentPage(2);
        pane->resize(260, 420);
        pane->show();
        const QImage img = pane->viewport()->grab().toImage();
        const QRect cur = pane->thumbnailRect(2), other = pane->thumbnailRect(0), hid = pane->thumbnailRect(1);
        QCOMPARE(img.pixelColor(cur.left() - 2, cur.center().y()), deckAppInfo().accent);
        QCOMPARE(img.pixelColor(cur.center().x(), cur.top() - 2), deckAppInfo().accent);
        QCOMPARE(img.pixelColor(cur.left() + 4, cur.top() + 4), QColor(Qt::white));   // no fill over the slide
        QVERIFY(img.pixelColor(other.left() - 2, other.center().y()) != deckAppInfo().accent);
        QCOMPARE(img.pixelColor(other.left() + 4, other.top() + 4), QColor(Qt::white));
        // Hidden: faded, so its white isn't white any more.
        QVERIFY(img.pixelColor(hid.left() + 4, hid.top() + 4) != QColor(Qt::white));
        pane->hide();
    }

    // A new slide size fits everything on every slide, layout, and the slide
    // master to it (Ensure Fit): the layouts and placeholders take the new
    // shape, text shrinks by the width's ratio, and the other objects on a
    // slide keep their shape, smaller and centered.
    void slideSizeScalesEverything()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        auto shape = std::make_shared<ShapeItem>();
        shape->rect = QRectF(100, 100, 200, 100);
        d->pages[1]->items.push_back(shape);
        const auto *title = static_cast<const TextItem *>(d->pages[1]->items[0].get());
        const MasterPage *layout = deck::layoutOfType(*d, QStringLiteral("obj"));
        const QRectF slideBefore = title->rect, layoutBefore = layout->items[0]->rect, masterBefore = d->masters[0]->items[0]->rect;
        auto size = [&](const QString &storyId) { return QTextCursor(d->storyDoc(storyId)).charFormat().fontPointSize(); };
        const double textBefore = size(title->storyId);
        QVERIFY(textBefore > 0);

        deck::setSlideSize(*d, QSizeF(deck::kStandardWidth, deck::kStandardHeight));
        QCOMPARE(d->pageSize(), QSizeF(720, 540));
        QCOMPARE(d->setup.sizeName, QStringLiteral("Standard"));
        auto narrower = [](const QRectF &r) { return QRectF(r.x() * 0.75, r.y(), r.width() * 0.75, r.height()); };
        QCOMPARE(title->rect, narrower(slideBefore));
        QCOMPARE(layout->items[0]->rect, narrower(layoutBefore));
        QCOMPARE(d->masters[0]->items[0]->rect, narrower(masterBefore));
        QCOMPARE(size(title->storyId), textBefore * 0.75);
        QCOMPARE(shape->rect, QRectF(75, 100 * 0.75 + 67.5, 150, 75));   // in shape, smaller, and in the middle
        // A slide still draws, at its new size.
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        QCOMPARE(Renderer::renderToImage(ctx, 0, 0.5).size(), QSize(360, 270));

        // Wider again: the placeholders and layouts are where they were, nothing else grows.
        deck::setSlideSize(*d, QSizeF(deck::kSlideWidth, deck::kSlideHeight));
        QCOMPARE(d->setup.sizeName, QStringLiteral("Widescreen"));
        QCOMPARE(title->rect, slideBefore);
        QCOMPARE(layout->items[0]->rect, layoutBefore);
        QCOMPARE(size(title->storyId), textBefore * 0.75);
        QCOMPARE(shape->rect, QRectF(75 + 120, 100 * 0.75 + 67.5, 150, 75));
    }

    // Header & Footer puts the slide number, the date, and a line of text in
    // each slide's own placeholders for them, and takes them away again.
    void headerAndFooterPlaceholders()
    {
        auto d = deck::newPresentation();   // a title slide
        for (int i = 0; i < 2; ++i) deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        auto typesOf = [&](int i) {
            QStringList t;
            for (const auto &it : d->pages[i]->items) t << deck::placeholderType(*it);
            return t;
        };
        deck::FooterOptions o;
        o.slideNumber = true;
        o.skipTitleSlide = true;
        deck::applyFooters(*d, o);
        QVERIFY(d->appData.value(QStringLiteral("footers")).toBool());
        QCOMPARE(typesOf(0), (QStringList{"ctrTitle", "subTitle"}));   // not on the title slide
        QCOMPARE(typesOf(1), (QStringList{"title", "obj", "sldNum"}));
        QCOMPARE(typesOf(2), (QStringList{"title", "obj", "sldNum"}));
        // The number is the page number field, in the layout's place and look.
        const auto *num = static_cast<const TextItem *>(d->pages[1]->items.back().get());
        QCOMPARE(num->rect, deck::layoutOfType(*d, QStringLiteral("obj"))->items.back()->rect);
        const QTextDocument *sd = d->storyDoc(num->storyId);
        QCOMPARE(sd->toPlainText(), QString(QChar(QChar::ObjectReplacementCharacter)));
        QCOMPARE(QTextCursor(const_cast<QTextDocument *>(sd)).charFormat().stringProperty(tp::Field), QStringLiteral("page"));
        // Apply to one slide: its own, and what new slides get stays.
        o.date = o.footer = true;
        o.footerText = QStringLiteral("Quarterly review");
        o.skipTitleSlide = false;
        deck::applyFooters(*d, o, d->pages[0].get());
        QCOMPARE(typesOf(0), (QStringList{"ctrTitle", "subTitle", "dt", "ftr", "sldNum"}));
        QCOMPARE(d->storyDoc(static_cast<const TextItem *>(d->pages[0]->items[3].get())->storyId)->toPlainText(), QStringLiteral("Quarterly review"));
        QCOMPARE(typesOf(1), (QStringList{"title", "obj", "sldNum"}));
        const int added = deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("titleOnly")));
        QCOMPARE(typesOf(added), (QStringList{"title", "sldNum"}));
        // Apply to all with nothing chosen takes them all away.
        deck::applyFooters(*d, deck::FooterOptions());
        for (int i = 0; i < d->pages.size(); ++i) QVERIFY2(!typesOf(i).contains(QStringLiteral("sldNum")) && !typesOf(i).contains(QStringLiteral("ftr")), qPrintable(typesOf(i).join(',')));
        QVERIFY(!d->appData.value(QStringLiteral("footers")).toBool());
    }

    // A slide's own background, or one for all of them through the master.
    void backgroundsSetAndClear()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        deck::setSlideBackground(*d->pages[1], QColor(0x33, 0x66, 0x99));
        QCOMPARE(d->pages[1]->background, Fill::solid(ColorRef::rgb(QColor(0x33, 0x66, 0x99))));
        QVERIFY(d->pages[0]->background.type == Fill::NoFill);
        deck::setAllBackgrounds(*d, QColor(0x11, 0x22, 0x33));
        QVERIFY(d->pages[1]->background.type == Fill::NoFill);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        for (int i = 0; i < 2; ++i) QCOMPARE(Renderer::renderToImage(ctx, i, 0.1).pixelColor(3, 3), QColor(0x11, 0x22, 0x33));
    }

    // The ribbon builds with nothing missing and has the commands of the
    // Home, Insert, Design, and View tabs; each of those tabs gives every
    // control its own KeyTip.
    void ribbonHasTheTabsCommands()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        for (const char *id : {"fmt.bold", "fmt.shadow", "fmt.grow", "fmt.shrink", "fmt.clear", "case.2", "spacing.Tight", "edit.formatPainter", "para.bullets", "bullet.check",
                               "para.numbers", "number.3", "para.indentDec", "para.indentInc", "ls.1.5", "fmt.alignCenter", "tb.dirVertical", "valign.1", "cols.2", "arr.front",
                               "arr.backward", "arr.group", "arr.flipH", "slide.new", "insert.picture", "insert.table", "insert.textBox", "insert.headerFooter",
                               "design.sizeStandard", "design.sizeWidescreen", "design.formatBackground", "view.rulers", "view.gridlines", "view.guides", "view.notes",
                               "view.fit", "view.normal", "view.sorter", "view.slideShow", "zoom.in", "zoom.out", "zoom.100"})
            QVERIFY2(w.act(QString::fromLatin1(id)), id);
        QStringList tabs;
        for (int i = 0; i < w.ribbon()->tabCount(); ++i) tabs << w.ribbon()->tabName(i);
        QVERIFY(tabs.indexOf(QStringLiteral("Insert")) < tabs.indexOf(QStringLiteral("Design")));
        QVERIFY(tabs.indexOf(QStringLiteral("Design")) < tabs.indexOf(QStringLiteral("Slide Show")));
        for (const QString &name : {QStringLiteral("Home"), QStringLiteral("Insert"), QStringLiteral("Design"), QStringLiteral("View")}) {
            QVERIFY2(w.ribbon()->tab(name), qPrintable(name));
            QStringList seen;
            for (const QWidget *c : w.ribbon()->tab(name)->findChildren<QWidget *>()) {
                const QString k = keytip(c);
                if (k.isEmpty()) continue;
                QVERIFY2(k.size() <= 2 && k.indexOf(QRegularExpression(QStringLiteral("[^A-Z0-9]"))) < 0, qPrintable(name + ": " + k));
                QVERIFY2(!seen.contains(k), qPrintable(name + ": two controls have the KeyTip " + k));
                seen << k;
            }
            QVERIFY2(seen.size() > 3, qPrintable(name));
        }
    }

    // A presentation shows no rulers until asked, and no outline around every
    // object; the View tab's check boxes turn the ruler, gridlines, and guides on.
    void normalViewDefaults()
    {
        DeckWindow w;
        QVERIFY(!w.editor()->view.rulers);
        QVERIFY(!w.editor()->view.boundaries);
        QVERIFY(w.canvas()->hRuler()->isHidden() && w.canvas()->vRuler()->isHidden());
        QVERIFY(!w.act("view.rulers")->isChecked() && !w.act("view.gridlines")->isChecked() && !w.act("view.guides")->isChecked());
        w.act("view.rulers")->trigger();
        QVERIFY(w.editor()->view.rulers);
        QVERIFY(!w.canvas()->hRuler()->isHidden() && !w.canvas()->vRuler()->isHidden());
        QVERIFY(w.act("view.rulers")->isChecked());
        w.act("view.rulers")->trigger();
        QVERIFY(w.canvas()->hRuler()->isHidden());
        w.act("view.gridlines")->trigger();
        w.act("view.guides")->trigger();
        QVERIFY(w.canvas()->gridlines() && w.canvas()->guides());
        // The boxes are the commands'.
        QCheckBox *ruler = nullptr;
        for (QCheckBox *b : w.ribbon()->findChildren<QCheckBox *>())
            if (b->text() == QStringLiteral("Ruler")) ruler = b;
        QVERIFY(ruler);
        ruler->click();
        QVERIFY(w.editor()->view.rulers);
        QVERIFY(ruler->isChecked());
    }

    // A placeholder is outlined (dotted) while it is empty or selected, and a
    // slide's objects aren't outlined the way every object is in the other apps.
    void placeholdersAreOutlinedOnlyWhileEmptyOrSelected()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        Editor *ed = w.editor();
        auto outlined = [&] {
            const QImage img = w.canvas()->viewport()->grab().toImage();
            const auto *title = static_cast<const TextItem *>(ed->doc()->pages[0]->items[0].get());
            const QPoint at = w.canvas()->pageToView(QPointF(title->rect.center().x(), title->rect.top())).toPoint();
            int marks = 0;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -20; dx < 20; ++dx) {
                    const QColor c = img.pixelColor(at.x() + dx, at.y() + dy);
                    marks += c.lightness() < 235;
                }
            return marks;
        };
        QVERIFY(outlined() > 3);   // empty
        auto *title = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get());
        ed->doc()->storyDoc(title->storyId)->setPlainText(QStringLiteral("Hi"));
        ed->notifyLive();
        QCOMPARE(outlined(), 0);   // text in it, not selected
        ed->select(title->id);
        QVERIFY(outlined() > 3);   // selected
        ed->clearSelection();
        QCOMPARE(outlined(), 0);
    }

    // The status bar: the slide's place, Notes, the views (Slide Sorter
    // waiting), and a zoom slider with its percent and a fit button.
    void statusBarShowsSlideAndZoom()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        auto *place = w.findChild<QLabel *>(QStringLiteral("slideNumber"));
        QVERIFY(place);
        QCOMPARE(place->text(), QStringLiteral("Slide 1 of 1"));
        w.newSlide();
        QTRY_COMPARE(place->text(), QStringLiteral("Slide 2 of 2"));
        QVERIFY(!w.act("view.sorter")->isEnabled());
        QVERIFY(w.act("view.sorter")->toolTip().contains(QStringLiteral("coming")));
        QVERIFY(w.act("view.normal")->isChecked());
        // Notes shows and hides the notes below the slide.
        QVERIFY(!w.notesPane()->isHidden());
        QTRY_VERIFY(w.act("view.notes")->isChecked());
        w.act("view.notes")->trigger();
        QVERIFY(w.notesPane()->isHidden());
        QTRY_VERIFY(!w.act("view.notes")->isChecked());
        w.act("view.notes")->trigger();
        QVERIFY(!w.notesPane()->isHidden());
        // Zoom: the slider zooms, the label follows the canvas, and Fit comes back to the window.
        auto *slider = w.findChild<QSlider *>(QStringLiteral("zoomSlider"));
        auto *percent = w.findChild<QLabel *>(QStringLiteral("zoomPercent"));
        QVERIFY(slider && percent);
        slider->setValue(0);
        QCOMPARE(w.canvas()->zoom(), 1.0);
        QCOMPARE(percent->text(), QStringLiteral("100%"));
        w.canvas()->setZoom(0.5);
        QCOMPARE(percent->text(), QStringLiteral("50%"));
        w.act("view.fit")->trigger();
        QVERIFY(w.canvas()->fitMode() == Canvas::Fit::WholePage);
        w.act("zoom.200")->trigger();
        QCOMPARE(w.canvas()->zoom(), 2.0);
    }

    // The slide size from the window is one step that undoes, and the
    // thumbnails take the slides' new shape.
    void slideSizeFromTheWindow()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        const QRectF before = ed->doc()->pages[0]->items[0]->rect;
        w.setSlideSize(QSizeF(deck::kStandardWidth, deck::kStandardHeight));
        QCOMPARE(ed->doc()->pageSize(), QSizeF(720, 540));
        QVERIFY(ed->doc()->pages[0]->items[0]->rect != before);
        QTRY_COMPARE(w.slidesPane()->iconSize(), QSize(168, 126));
        QTRY_VERIFY(w.act("design.sizeStandard")->isChecked());
        QVERIFY(!w.act("design.sizeWidescreen")->isChecked());
        ed->undo();
        QCOMPARE(ed->doc()->pageSize(), QSizeF(960, 540));
        QCOMPARE(ed->doc()->pages[0]->items[0]->rect, before);
        QTRY_VERIFY(w.act("design.sizeWidescreen")->isChecked());
    }

    // Format Background and Header & Footer from the window are steps that undo.
    void backgroundAndFooterFromTheWindow()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        w.setBackground(QColor(0x33, 0x66, 0x99), false);
        QCOMPARE(ed->doc()->pages[1]->background, Fill::solid(ColorRef::rgb(QColor(0x33, 0x66, 0x99))));
        QVERIFY(ed->doc()->pages[0]->background.type == Fill::NoFill);
        ed->undo();
        QVERIFY(ed->doc()->pages[1]->background.type == Fill::NoFill);
        w.setBackground(QColor(0x11, 0x22, 0x33), true);
        QCOMPARE(ed->doc()->masters[0]->background, Fill::solid(ColorRef::rgb(QColor(0x11, 0x22, 0x33))));
        ed->undo();
        QCOMPARE(ed->doc()->masters[0]->background, Fill::solid(ColorRef::rgb(Qt::white)));

        deck::FooterOptions o;
        o.slideNumber = true;
        o.skipTitleSlide = true;
        w.applyFooters(o, true);
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), 2);
        QCOMPARE(int(ed->doc()->pages[1]->items.size()), 3);
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[1]->items.size()), 2);
        QVERIFY(!ed->doc()->appData.value(QStringLiteral("footers")).toBool());
    }

    // The Header & Footer dialog hands back what was chosen and which button.
    void headerFooterDialogReturnsTheChoice()
    {
        deck::FooterOptions o;
        o.slideNumber = true;
        o.footerText = QStringLiteral("Quarterly");
        HeaderFooterDialog dlg(o);
        QVERIFY(dlg.options().slideNumber && !dlg.options().date && !dlg.options().footer);
        QVERIFY(!dlg.findChild<QLineEdit *>(QStringLiteral("footerText"))->isEnabled());   // until Footer is checked
        dlg.findChild<QCheckBox *>(QStringLiteral("footer"))->setChecked(true);
        dlg.findChild<QCheckBox *>(QStringLiteral("date"))->setChecked(true);
        dlg.findChild<QCheckBox *>(QStringLiteral("skipTitle"))->setChecked(true);
        dlg.findChild<QLineEdit *>(QStringLiteral("footerText"))->setText(QStringLiteral("Q3 review"));
        const deck::FooterOptions got = dlg.options();
        QVERIFY(got.date && got.slideNumber && got.footer && got.skipTitleSlide);
        QCOMPARE(got.footerText, QStringLiteral("Q3 review"));
        dlg.findChild<QPushButton *>(QStringLiteral("applyToAll"))->click();
        QCOMPARE(dlg.result(), int(QDialog::Accepted));
        QVERIFY(dlg.applyToAll());
        HeaderFooterDialog one(o);
        one.findChild<QPushButton *>(QStringLiteral("apply"))->click();
        QCOMPARE(one.result(), int(QDialog::Accepted));
        QVERIFY(!one.applyToAll());
    }

    // New Slide's gallery shows each layout as a picture and says which was chosen.
    void layoutGalleryShowsEachLayout()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        LayoutGallery g(ed);
        QCOMPARE(g.count(), 9);
        QCOMPARE(g.typeAt(0), QStringLiteral("title"));
        QCOMPARE(g.typeAt(8), QStringLiteral("picTx"));
        QCOMPARE(layoutPicture(*ed->doc(), *deck::layoutOfType(*ed->doc(), QStringLiteral("obj")), 112).size(), QSize(112, 63));
        auto busy = [&](const QString &type) {
            const QImage img = layoutPicture(*ed->doc(), *deck::layoutOfType(*ed->doc(), type), 112);
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (img.pixelColor(x, y) != QColor(Qt::white)) return true;
            return false;
        };
        QVERIFY(busy(QStringLiteral("obj")));
        QVERIFY(busy(QStringLiteral("twoObj")));
        QVERIFY(!busy(QStringLiteral("blank")));
        QSignalSpy chosen(&g, &LayoutGallery::chosen);
        QTest::keyClick(&g, Qt::Key_Right);
        QTest::keyClick(&g, Qt::Key_Right);
        QTest::keyClick(&g, Qt::Key_Down);
        QTest::keyClick(&g, Qt::Key_Return);
        QCOMPARE(chosen.count(), 1);
        QCOMPARE(chosen.first().first().toString(), g.typeAt(4));   // the middle of the second row
    }

    // The layouts gallery in the New Slide menu makes a slide with the layout
    // chosen, and the one in the Layout menu changes the slide's.
    void layoutMenusUseTheGallery()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        // In the order the ribbon built them: Home's New Slide and Layout, then Insert's New Slide.
        QList<LayoutGallery *> galleries;
        for (QMenu *m : w.findChildren<QMenu *>())
            if (auto *g = m->findChild<LayoutGallery *>()) galleries << g;
        QCOMPARE(galleries.size(), 3);
        LayoutGallery *newSlide = galleries.at(0), *layout = galleries.at(1);
        for (int i = 0; i < 6; ++i) QTest::keyClick(newSlide, Qt::Key_Right);
        QTest::keyClick(newSlide, Qt::Key_Return);   // Title Only
        QCOMPARE(int(ed->doc()->pages.size()), 2);
        QCOMPARE(deck::layoutType(*ed->doc()->masterFor(*ed->doc()->pages[1])), QStringLiteral("titleOnly"));
        QCOMPARE(ed->currentPage(), 1);
        QTest::keyClick(layout, Qt::Key_Home);
        QTest::keyClick(layout, Qt::Key_Return);   // Title Slide
        QCOMPARE(deck::layoutType(*ed->doc()->masterFor(*ed->doc()->pages[1])), QStringLiteral("title"));
        QCOMPARE(int(ed->doc()->pages.size()), 2);
    }

    // Insert > Pictures centers the picture on the slide at its natural size, one
    // step that undoes; Insert > Table puts a table there too.
    void insertPictureAndTable()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QImage image(200, 100, QImage::Format_RGB32);
        image.fill(Qt::red);
        QVERIFY(image.save(dir.filePath(QStringLiteral("red.png"))));
        QFile text(dir.filePath(QStringLiteral("notes.txt")));
        QVERIFY(text.open(QIODevice::WriteOnly));
        text.write("not a picture");
        text.close();
        DeckWindow w;
        Editor *ed = w.editor();
        const int before = int(ed->doc()->pages[0]->items.size());
        QVERIFY(!w.insertPicture(dir.filePath(QStringLiteral("notes.txt"))));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);
        QVERIFY(w.insertPicture(dir.filePath(QStringLiteral("red.png"))));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        const auto *pic = dynamic_cast<const PictureItem *>(ed->doc()->pages[0]->items.back().get());
        QVERIFY(pic);
        QCOMPARE(pic->rect, QRectF(480 - 75, 270 - 37.5, 150, 75));
        QCOMPARE(ed->selection(), QStringList{pic->id});
        ed->undo();
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before);
        // Dropped on the slide, it goes where it was dropped.
        Q_EMIT w.canvas()->insertFilesWanted({dir.filePath(QStringLiteral("red.png"))}, QPointF(100, 50));
        QCOMPARE(int(ed->doc()->pages[0]->items.size()), before + 1);
        QCOMPARE(ed->doc()->pages[0]->items.back()->rect.topLeft(), QPointF(100, 50));
        ed->undo();

        w.insertTable(3, 4);
        const auto *table = dynamic_cast<const TableItem *>(ed->doc()->pages[0]->items.back().get());
        QVERIFY(table);
        QCOMPARE(table->rows, 3);
        QCOMPARE(table->cols, 4);
        QCOMPARE(table->rect.center(), QPointF(480, 270));
        QCOMPARE(QTextCursor(ed->doc()->storyDoc(table->cells[0].storyId)).charFormat().fontPointSize(), 18.0);
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

    // `n` content slides, each titled and with notes of its own, so that
    // each draws differently.
    static std::unique_ptr<Document> showDeck(int n)
    {
        auto d = deck::newPresentation();
        while (int(d->pages.size()) < n) deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        for (int i = 0; i < n; ++i) {
            Page &s = *d->pages[i];
            d->storyDoc(static_cast<TextItem *>(s.items[0].get())->storyId)->setPlainText(QStringLiteral("Slide %1 title").arg(i + 1));
            d->storyDoc(s.notesStoryId)->setPlainText(QStringLiteral("Notes %1").arg(i + 1));
        }
        return d;
    }

    static deck::Transition showTransition(const char *type, double seconds)
    {
        deck::Transition t;
        t.type = QString::fromLatin1(type);
        t.duration = seconds;
        return t;
    }

    // A transition lives in the slide's props and comes back whole; what is
    // missing or makes no sense takes its default.
    void transitionJsonRoundTrip()
    {
        deck::Transition t;
        QCOMPARE(t.type, QStringLiteral("none"));
        QCOMPARE(t.dir, QStringLiteral("l"));
        QCOMPARE(t.duration, 0.7);
        QVERIFY(!t.advanceAfter);
        QVERIFY(t.advanceOnClick);
        QVERIFY(deck::Transition::fromJson(QJsonObject()) == t);
        QVERIFY(!t.toJson().contains(QStringLiteral("advanceAfter")));   // absent, not zero
        t.type = QStringLiteral("push");
        t.dir = QStringLiteral("u");
        t.duration = 1.5;
        t.advanceAfter = 4.0;
        t.advanceOnClick = false;
        const QJsonObject o = t.toJson();
        QCOMPARE(o.value("type").toString(), QStringLiteral("push"));
        QCOMPARE(o.value("dir").toString(), QStringLiteral("u"));
        QCOMPARE(o.value("duration").toDouble(), 1.5);
        QCOMPARE(o.value("advanceAfter").toDouble(), 4.0);
        QCOMPARE(o.value("advanceOnClick").toBool(true), false);
        QVERIFY(deck::Transition::fromJson(o) == t);
        // Split and zoom go in or out, the rest in a direction.
        QCOMPARE(deck::Transition::fromJson({{"type", "split"}}).dir, QStringLiteral("out"));
        QCOMPARE(deck::Transition::fromJson({{"type", "zoom"}}).dir, QStringLiteral("in"));
        QCOMPARE(deck::Transition::fromJson({{"type", "zoom"}, {"dir", "in"}}).dir, QStringLiteral("in"));
        QCOMPARE(deck::Transition::fromJson({{"type", "split"}, {"dir", "in"}}).dir, QStringLiteral("in"));
        QCOMPARE(deck::Transition::fromJson({{"type", "wipe"}, {"dir", "out"}}).dir, QStringLiteral("l"));
        QCOMPARE(deck::Transition::fromJson({{"type", "wipe"}, {"dir", "d"}}).dir, QStringLiteral("d"));
        QCOMPARE(deck::Transition::fromJson({{"type", "zoom"}, {"dir", "u"}}).dir, QStringLiteral("in"));
        // An unknown type is none; a duration stays within what a person can sit through.
        QCOMPARE(deck::Transition::fromJson({{"type", "spin"}}).type, QStringLiteral("none"));
        QCOMPARE(deck::Transition::fromJson({{"type", "fade"}, {"duration", 500}}).duration, 59.0);
        QCOMPARE(deck::Transition::fromJson({{"type", "fade"}, {"duration", 0}}).duration, 0.01);
        QVERIFY(!deck::Transition::fromJson({{"advanceAfter", -3}}).advanceAfter);
        QCOMPARE(*deck::Transition::fromJson({{"advanceAfter", 0}}).advanceAfter, 0.0);

        // On a slide: kept in its props beside whatever else is there, and
        // gone from them when it is the default again.
        Page slide;
        slide.props[QStringLiteral("kind")] = QStringLiteral("other");
        QVERIFY(deck::transitionOf(slide) == deck::Transition());
        deck::setTransition(slide, t);
        QVERIFY(slide.props.value("transition").isObject());
        QVERIFY(deck::transitionOf(slide) == t);
        deck::setTransition(slide, deck::Transition());
        QVERIFY(!slide.props.contains(QStringLiteral("transition")));
        QCOMPARE(slide.props.value("kind").toString(), QStringLiteral("other"));
        QCOMPARE(deck::transitionTypes(),
                 (QStringList{"none", "fade", "push", "wipe", "split", "reveal", "cover", "uncover", "zoom", "cut"}));
    }

    // Going forward onto a slide with a transition keeps the old slide on the
    // screen while it plays and lands on the new one when it ends; going
    // back, or jumping, shows the slide at once.
    void showTransitionPlaysOnNext()
    {
        auto d = showDeck(3);
        deck::setTransition(*d->pages[1], showTransition("fade", 0.15));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        QSignalSpy changes(&show, &SlideShow::changed);
        show.next();
        QCOMPARE(show.currentSlide(), 0);   // still the old slide on the screen
        QCOMPARE(show.targetSlide(), 1);
        QVERIFY(show.isAnimating());
        QVERIFY(changes.count() >= 1);      // Presenter View already knows
        QTRY_VERIFY(!show.isAnimating());
        QCOMPARE(show.currentSlide(), 1);
        QCOMPARE(show.targetSlide(), 1);
        // Going back has no transition, though the slide left has one.
        show.previous();
        QVERIFY(!show.isAnimating());
        QCOMPARE(show.currentSlide(), 0);
        // Finishing it by hand lands on the new slide too.
        deck::setTransition(*d->pages[1], showTransition("push", 30));
        show.next();
        QVERIFY(show.isAnimating());
        show.finishTransition();
        QVERIFY(!show.isAnimating());
        QCOMPARE(show.currentSlide(), 1);
        // Next in the middle of a transition finishes it first, then plays the next one.
        show.goTo(0);
        QVERIFY(!show.isAnimating());   // a jump shows the slide at once
        show.next();
        QVERIFY(show.isAnimating());
        deck::setTransition(*d->pages[2], showTransition("wipe", 30));
        show.next();
        QCOMPARE(show.currentSlide(), 1);
        QCOMPARE(show.targetSlide(), 2);
        QVERIFY(show.isAnimating());
        show.finishTransition();
        QCOMPARE(show.currentSlide(), 2);
        // Every type plays and ends.
        for (const QString &type : deck::transitionTypes()) {
            deck::setTransition(*d->pages[1], showTransition(type.toLatin1().constData(), 30));
            show.goTo(0);
            show.next();
            QCOMPARE(show.isAnimating(), type != QLatin1String("none"));
            show.finishTransition();
            QCOMPARE(show.currentSlide(), 1);
        }
    }

    // After a slide's seconds the show moves on by itself, counting from when
    // the slide's transition has finished.
    void showAdvancesAfterItsTime()
    {
        auto d = showDeck(3);
        deck::Transition t = showTransition("none", 0.7);
        t.advanceAfter = 0.05;
        deck::setTransition(*d->pages[0], t);
        t = showTransition("fade", 0.8);
        t.advanceAfter = 0.3;
        deck::setTransition(*d->pages[1], t);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        QVERIFY(!show.autoAdvanceActive());   // not before it is shown
        show.show();
        QTRY_COMPARE(show.targetSlide(), 1);
        QElapsedTimer sinceFade;
        sinceFade.start();
        // The fade plays out first; the clock starts after it.
        QVERIFY(show.isAnimating());
        QVERIFY(!show.autoAdvanceActive());
        QTRY_COMPARE(show.currentSlide(), 2);
        QVERIFY(sinceFade.elapsed() >= 700);   // not 0.3 seconds into the fade
        QVERIFY(!show.autoAdvanceActive());    // the third slide has no time: the show waits there
        show.next();
        QCOMPARE(show.currentSlide(), -1);
    }

    // A slide that doesn't advance on a click ignores clicks but not keys.
    void showAdvanceOnClickOff()
    {
        auto d = showDeck(3);
        deck::Transition t;
        t.advanceOnClick = false;
        deck::setTransition(*d->pages[0], t);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(640, 360);
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
        QCOMPARE(show.currentSlide(), 0);
        QTest::keyClick(&show, Qt::Key_Right);
        QCOMPARE(show.currentSlide(), 1);
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
        QCOMPARE(show.currentSlide(), 2);
    }

    // Esc in the middle of a transition ends the show.
    void showEscapeDuringTransition()
    {
        auto d = showDeck(3);
        deck::setTransition(*d->pages[1], showTransition("zoom", 30));
        LayoutCache cache;
        QPointer<SlideShow> show = new SlideShow(d.get(), &cache, 0);
        QSignalSpy ended(show.data(), &SlideShow::ended);
        show->show();
        show->next();
        QVERIFY(show->isAnimating());
        QTest::keyClick(show.data(), Qt::Key_Escape);
        QCOMPARE(ended.count(), 1);
        QCOMPARE(ended.at(0).at(0).toInt(), 1);   // the slide it was moving to
        QTRY_VERIFY(show.isNull());
    }

    // The end screen, black and white screens, number and Enter, and hidden
    // slides work with transitions on.
    void showKeysWithTransitions()
    {
        auto d = showDeck(4);
        d->pages[2]->hidden = true;
        for (int i = 1; i < 4; ++i) deck::setTransition(*d->pages[i], showTransition("fade", 0.05));
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        QTest::keyClick(&show, Qt::Key_B);
        QVERIFY(show.isBlank());
        QTest::keyClick(&show, Qt::Key_B);
        QVERIFY(!show.isBlank());
        QTest::keyClick(&show, Qt::Key_W);
        QVERIFY(show.isBlank());
        QTest::keyClick(&show, Qt::Key_Space);   // moving on shows the slide again
        QVERIFY(!show.isBlank());
        QTRY_COMPARE(show.currentSlide(), 1);
        QTest::keyClick(&show, Qt::Key_Right);
        QTRY_COMPARE(show.currentSlide(), 3);   // past the hidden slide
        QTest::keyClicks(&show, "1");
        QTest::keyClick(&show, Qt::Key_Return);
        QCOMPARE(show.currentSlide(), 0);
        QTest::keyClick(&show, Qt::Key_End);
        QCOMPARE(show.currentSlide(), 3);
        QTest::keyClick(&show, Qt::Key_Right);
        QTRY_COMPARE(show.currentSlide(), -1);   // the end screen
        QVERIFY(!show.isAnimating());
        QTest::keyClick(&show, Qt::Key_Left);
        QCOMPARE(show.currentSlide(), 3);
    }

    // A click on an item with a link opens the link and stays on the slide.
    void showClickOnLinkOpensIt()
    {
        auto d = showDeck(2);
        Item *title = d->pages[0]->items[0].get();
        title->hyperlink = QStringLiteral("https://example.com/talk");
        LinkCatcher catcher;
        QDesktopServices::setUrlHandler(QStringLiteral("https"), &catcher, "open");
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        show.resize(960, 540);   // the slide's own size: slide points are pixels
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, title->bounds().center().toPoint());
        QCOMPARE(catcher.urls, (QStringList{"https://example.com/talk"}));
        QCOMPARE(show.currentSlide(), 0);
        // Elsewhere a click moves on.
        QTest::mouseClick(&show, Qt::LeftButton, Qt::NoModifier, QPoint(2, 538));
        QCOMPARE(show.currentSlide(), 1);
        QCOMPARE(catcher.urls.size(), 1);
        QDesktopServices::unsetUrlHandler(QStringLiteral("https"));
    }

    // Presenter View: where the show is, the next slide, the notes, the timer;
    // its buttons drive the show.
    void presenterViewFollowsTheShow()
    {
        auto d = showDeck(4);
        d->pages[2]->hidden = true;
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        PresenterView pv(&show);
        QCOMPARE(pv.counterText(), QStringLiteral("Slide 1 of 4"));
        QCOMPARE(pv.notesText(), QStringLiteral("Notes 1"));
        QCOMPARE(pv.nextSlide(), 1);
        const QImage firstNext = pv.nextImage();
        QVERIFY(!firstNext.isNull());
        show.next();
        QCOMPARE(pv.counterText(), QStringLiteral("Slide 2 of 4"));
        QCOMPARE(pv.notesText(), QStringLiteral("Notes 2"));
        QCOMPARE(pv.nextSlide(), 3);   // the hidden slide is skipped
        QVERIFY(pv.nextImage() != firstNext);
        // The buttons call the show.
        pv.findChild<QAbstractButton *>("previousButton")->click();
        QCOMPARE(show.currentSlide(), 0);
        QCOMPARE(pv.counterText(), QStringLiteral("Slide 1 of 4"));
        pv.findChild<QAbstractButton *>("nextButton")->click();
        QCOMPARE(show.currentSlide(), 1);
        pv.findChild<QAbstractButton *>("blackButton")->click();
        QVERIFY(show.isBlank());
        pv.findChild<QAbstractButton *>("blackButton")->click();
        QVERIFY(!show.isBlank());
        // Keys work in Presenter View too.
        QTest::keyClick(&pv, Qt::Key_Right);
        QCOMPARE(show.currentSlide(), 3);
        QCOMPARE(pv.nextSlide(), -1);
        QTest::keyClick(&pv, Qt::Key_Right);   // the end
        QCOMPARE(pv.counterText(), QStringLiteral("End of slide show"));
        QTest::keyClick(&pv, Qt::Key_Left);
        QCOMPARE(pv.counterText(), QStringLiteral("Slide 4 of 4"));
        // The notes grow and shrink.
        const int size = pv.notesPointSize();
        pv.findChild<QAbstractButton *>("notesBigger")->click();
        QVERIFY(pv.notesPointSize() > size);
        pv.findChild<QAbstractButton *>("notesSmaller")->click();
        pv.findChild<QAbstractButton *>("notesSmaller")->click();
        QVERIFY(pv.notesPointSize() < size);
        // The slide grid jumps to a slide.
        pv.findChild<QAbstractButton *>("allSlidesButton")->click();
        QVERIFY(pv.isShowingAllSlides());
        pv.jumpTo(0);
        QCOMPARE(show.currentSlide(), 0);
        QVERIFY(!pv.isShowingAllSlides());
        // It draws: the slides and the controls, not a blank window.
        pv.resize(1100, 700);
        const QImage shot = pv.grab().toImage();
        QCOMPARE(shot.size(), QSize(1100, 700));
        QSet<QRgb> colors;
        for (int y = 0; y < shot.height(); y += 9)
            for (int x = 0; x < shot.width(); x += 9) colors.insert(shot.pixel(x, y));
        QVERIFY(colors.size() >= 4);
        // End Slide Show closes the show, and Presenter View with it.
        QPointer<SlideShow> alive = new SlideShow(d.get(), &cache, 0);
        QPointer<PresenterView> pv2 = new PresenterView(alive.data());
        pv2->findChild<QAbstractButton *>("endButton")->click();
        QTRY_VERIFY(alive.isNull());
        QTRY_VERIFY(pv2.isNull());
    }

    // The timer counts from the start of the show, pauses and resumes, and resets.
    void presenterViewTimer()
    {
        auto d = showDeck(2);
        LayoutCache cache;
        SlideShow show(d.get(), &cache, 0);
        PresenterView pv(&show);
        QCOMPARE(pv.timerText(), QStringLiteral("00:00"));
        QVERIFY(!pv.isTimerPaused());
        QAbstractButton *pause = pv.findChild<QAbstractButton *>("pauseButton");
        QVERIFY(pause);
        const QString runningLabel = pause->text();
        pause->click();
        QVERIFY(pv.isTimerPaused());
        QVERIFY(pause->text() != runningLabel);   // it says Resume
        pv.setTimerElapsed(65 * 1000);
        QCOMPARE(pv.timerText(), QStringLiteral("01:05"));
        pause->click();
        QVERIFY(!pv.isTimerPaused());
        QCOMPARE(pause->text(), runningLabel);
        pv.setTimerElapsed(3725 * 1000);
        QCOMPARE(pv.timerText(), QStringLiteral("1:02:05"));
        pv.findChild<QAbstractButton *>("resetButton")->click();
        QVERIFY(pv.timerElapsed() < 1000);
        QCOMPARE(pv.timerText(), QStringLiteral("00:00"));
    }

    // Alt+F5 on one screen rehearses: the show in a window and Presenter View
    // in another; Esc in either ends both.
    void windowPresenterViewRehearsal()
    {
        DeckWindow w;
        w.newSlide();
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QVERIFY(w.act("show.presenterView"));
        QCOMPARE(w.act("show.presenterView")->shortcut(), QKeySequence(Qt::ALT | Qt::Key_F5));
        w.act("show.presenterView")->trigger();
        QPointer<SlideShow> show = w.slideShow();
        QVERIFY(show);
        QPointer<PresenterView> pv = show->presenterView();
        QVERIFY(pv);
        QCOMPARE(pv->counterText(), QStringLiteral("Slide 2 of 2"));   // from the current slide
        QTest::keyClick(pv.data(), Qt::Key_Left);
        QCOMPARE(show->currentSlide(), 0);
        QTest::keyClick(show.data(), Qt::Key_Right);
        QCOMPARE(pv->counterText(), QStringLiteral("Slide 2 of 2"));
        QTest::keyClick(pv.data(), Qt::Key_Escape);
        QTRY_VERIFY(show.isNull());
        QTRY_VERIFY(pv.isNull());
        // One screen and F5: the show alone.
        w.startShow(0);
        QVERIFY(w.slideShow());
        QVERIFY(!w.slideShow()->presenterView());
        QTest::keyClick(w.slideShow(), Qt::Key_Escape);
        QTRY_VERIFY(!w.slideShow());
    }

    // The Transitions tab: each command is a step that undoes; Apply To All
    // gives every slide the transition of the one being edited.
    void windowTransitionsTab()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QVERIFY(w.ribbon()->tab(QStringLiteral("Transitions")));
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        QCOMPARE(ed->currentPage(), 2);
        auto current = [&] { return deck::transitionOf(*ed->doc()->pages[ed->currentPage()]); };
        QCOMPARE(current().type, QStringLiteral("none"));
        for (const QString &type : deck::transitionTypes()) QVERIFY2(w.act("transition." + type), qPrintable(type));
        w.act("transition.push")->trigger();
        QCOMPARE(current().type, QStringLiteral("push"));
        QCOMPARE(deck::transitionOf(*ed->doc()->pages[1]).type, QStringLiteral("none"));   // only this slide
        QVERIFY(w.act("transition.push")->isChecked());
        w.act("transition.dir.u")->trigger();
        QCOMPARE(current().dir, QStringLiteral("u"));
        w.act("transition.zoom")->trigger();   // a type that goes in or out takes its default
        QCOMPARE(current().dir, QStringLiteral("in"));
        w.act("transition.dir.out")->trigger();
        QCOMPARE(current().dir, QStringLiteral("out"));
        w.act("transition.push")->trigger();
        QCOMPARE(current().dir, QStringLiteral("l"));   // and back to one that goes in a direction
        // Timing.
        auto *duration = w.ribbon()->findChild<QDoubleSpinBox *>("transitionDuration");
        auto *onClick = w.ribbon()->findChild<QCheckBox *>("advanceOnClick");
        auto *after = w.ribbon()->findChild<QCheckBox *>("advanceAfter");
        auto *seconds = w.ribbon()->findChild<QDoubleSpinBox *>("advanceAfterSeconds");
        QVERIFY(duration && onClick && after && seconds);
        QCOMPARE(duration->value(), 0.7);
        QVERIFY(onClick->isChecked());
        QVERIFY(!after->isChecked());
        QVERIFY(!seconds->isEnabled());
        duration->setValue(1.5);
        QCOMPARE(current().duration, 1.5);
        onClick->setChecked(false);
        QVERIFY(!current().advanceOnClick);
        after->setChecked(true);
        QVERIFY(seconds->isEnabled());
        QVERIFY(current().advanceAfter);
        seconds->setValue(4);
        QCOMPARE(*current().advanceAfter, 4.0);
        // Another slide shows its own settings; coming back shows these.
        ed->setCurrentPage(0);
        QTRY_COMPARE(duration->value(), 0.7);
        QVERIFY(onClick->isChecked());
        QVERIFY(!after->isChecked());
        ed->setCurrentPage(2);
        QTRY_COMPARE(duration->value(), 1.5);
        QVERIFY(!onClick->isChecked());
        QVERIFY(after->isChecked());
        QCOMPARE(seconds->value(), 4.0);
        // Apply To All is one step.
        const deck::Transition mine = current();
        QVERIFY(mine.type == QLatin1String("push"));
        w.act("transition.applyToAll")->trigger();
        for (int i = 0; i < 3; ++i) QVERIFY(deck::transitionOf(*ed->doc()->pages[i]) == mine);
        ed->undo();
        QVERIFY(deck::transitionOf(*ed->doc()->pages[0]) == deck::Transition());
        QVERIFY(deck::transitionOf(*ed->doc()->pages[1]) == deck::Transition());
        QVERIFY(deck::transitionOf(*ed->doc()->pages[2]) == mine);
        // Each of the others was a step too.
        ed->undo();   // seconds
        QCOMPARE(current().advanceAfter.value_or(-1), 5.0);
        ed->undo();   // After on
        QVERIFY(!current().advanceAfter);
        ed->undo();   // On Mouse Click off
        QVERIFY(current().advanceOnClick);
        ed->undo();   // duration
        QCOMPARE(current().duration, 0.7);
        ed->undo();   // push again
        QCOMPARE(current().type, QStringLiteral("zoom"));
        QCOMPARE(current().dir, QStringLiteral("out"));
        // None takes the transition off and the star with it.
        w.act("transition.none")->trigger();
        QVERIFY(deck::transitionOf(*ed->doc()->pages[2]) == deck::Transition());
        QVERIFY(!ed->doc()->pages[2]->props.contains(QStringLiteral("transition")));
    }

    // Painting only the changed part of a frame over the last one gives the
    // next frame, for every type and direction.
    void transitionChangeRegionIsComplete()
    {
        for (const qreal dpr : {1.0, 2.0}) {
            const QSize size(160, 90);
            const QSize px(qRound(size.width() * dpr), qRound(size.height() * dpr));
            QImage a(px, QImage::Format_RGB32), b(px, QImage::Format_RGB32);
            a.setDevicePixelRatio(dpr);
            b.setDevicePixelRatio(dpr);
            for (int y = 0; y < px.height(); ++y)
                for (int x = 0; x < px.width(); ++x) {
                    a.setPixel(x, y, qRgb((x * 7) % 256, (y * 5) % 256, 40));
                    b.setPixel(x, y, qRgb(200, (x * 3) % 256, (y * 11) % 256));
                }
            auto frameAt = [&](const deck::Transition &t, double progress, const QRegion &only, QImage *over) {
                QImage frame = over ? *over : QImage(px, QImage::Format_RGB32);
                frame.setDevicePixelRatio(dpr);
                QPainter p(&frame);
                if (!only.isEmpty()) p.setClipRegion(only);
                paintTransition(&p, t, a, b, progress);
                return frame;
            };
            for (const QString &type : deck::transitionTypes()) {
                deck::Transition t;
                t.type = type;
                QStringList dirs{"l", "r", "u", "d"};
                if (t.hasInOut()) dirs = QStringList{"in", "out"};
                for (const QString &dir : dirs) {
                    t.dir = dir;
                    for (const auto &[before, after] : {std::pair(0.0, 0.1), std::pair(0.1, 0.13), std::pair(0.34, 0.36), std::pair(0.64, 0.66), std::pair(0.9, 1.0)}) {
                        QImage last = frameAt(t, before, QRegion(), nullptr);
                        const QRegion changed = transitionChange(t, QSizeF(size), before, after);
                        QImage next = changed.isEmpty() ? last : frameAt(t, after, changed, &last);
                        const QImage expected = frameAt(t, after, QRegion(), nullptr);
                        QVERIFY2(next == expected, qPrintable(QStringLiteral("%1 %2 from %3 to %4 at %5x").arg(type, dir).arg(before).arg(after).arg(dpr)));
                    }
                }
            }
        }
        // A wipe changes a strip, not the screen; a cut changes twice.
        deck::Transition wipe;
        wipe.type = QStringLiteral("wipe");
        QVERIFY(transitionChange(wipe, QSizeF(1920, 1080), 0.5, 0.52).boundingRect().width() < 80);
        deck::Transition cut;
        cut.type = QStringLiteral("cut");
        QVERIFY(transitionChange(cut, QSizeF(1920, 1080), 0.4, 0.5).isEmpty());
        QVERIFY(!transitionChange(cut, QSizeF(1920, 1080), 0.3, 0.4).isEmpty());
    }

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

    // The presentation's drawing group keeps the properties a shape starts from: a shape of a slide that names no colors has the
    // fills color and the text color of the scheme (the master's), not white and black.
    void importBinaryShapesTakeTheDrawingGroupsDefaults()
    {
        auto table = [&](const QVector<QPair<int, quint32>> &props) {
            QByteArray t;
            for (const auto &p : props) t += le(quint16(p.first), 2) + le(p.second, 4);
            return pptRec(0xF00B, t, quint16(props.size()), 3);
        };
        // Filled in scheme color 4 (the fills color), a line in scheme color 1 (the text color).
        const QByteArray group = pptBox(1035, pptBox(0xF000, pptRec(0xF006, QByteArray(16, '\0')) + table({{0x0181, 0x08000004}, {0x01BF, 0x00100010}, {0x01C0, 0x08000001}, {0x01FF, 0x00080008}})));
        // A rectangle that says nothing of its look, and one that has its own fill.
        auto rectangle = [&](quint32 id, int top, const QByteArray &props) {
            return pptBox(0xF004, pptRec(0xF00A, le(id, 4) + le(0xA00, 4), 1, 2) + props + pptRec(0xF010, le(top, 2) + le(100, 2) + le(2000, 2) + le(top + 500, 2)));
        };
        const QByteArray shapes = rectangle(0x500, 100, QByteArray()) + rectangle(0x501, 800, table({{0x0181, 0x0000FF}}));
        auto look = [&](const QByteArray &file) {
            QString err;
            auto d = deck::importBinaryPresentation(file, &err);
            QList<QPair<QColor, QColor>> out;   // the fill and the line color of the page's plain shapes
            if (!d) qWarning("%s", qPrintable(err));
            if (!d) return out;
            for (const auto &it : d->pages[0]->items)
                if (it->role.isEmpty() && it->type() == ItemType::Shape)
                    out << qMakePair(it->fill.color.kind() == ColorRef::Rgb ? it->fill.color.rgbValue() : QColor(), it->stroke.color.kind() == ColorRef::Rgb ? it->stroke.color.rgbValue() : QColor());
            return out;
        };
        const auto withGroup = look(buildPpt(false, false, false, false, QByteArray(), group, shapes));
        QCOMPARE(int(withGroup.size()), 2);
        QCOMPARE(withGroup[0].first, QColor(0x77, 0x88, 0x99));
        QCOMPARE(withGroup[0].second, QColor(0x11, 0x22, 0x33));
        QCOMPARE(withGroup[1].first, QColor(255, 0, 0));
        // With no drawing group, the format's own: white, and black.
        const auto without = look(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes));
        QCOMPARE(int(without.size()), 2);
        QCOMPARE(without[0].first, QColor(Qt::white));
        QCOMPARE(without[0].second, QColor(Qt::black));
    }

    // A slide's placeholder takes what it leaves out of its look from the master's shape of its kind, and then from the drawing group's
    // defaults: the master's title says what the slides' titles look like.
    void importBinaryPlaceholdersTakeAfterTheirMastersShapes()
    {
        auto table = [&](const QVector<QPair<int, quint32>> &props) {
            QByteArray t;
            for (const auto &p : props) t += le(quint16(p.first), 2) + le(p.second, 4);
            return pptRec(0xF00B, t, quint16(props.size()), 3);
        };
        // The defaults: filled in the fills color, with a line in the text color.
        const QByteArray group = pptBox(1035, pptBox(0xF000, pptRec(0xF006, QByteArray(16, '\0')) + table({{0x0181, 0x08000004}, {0x01BF, 0x00100010}, {0x01C0, 0x08000001}, {0x01FF, 0x00080008}})));
        // The master's title: a green fill and no line. Its body says nothing.
        const QByteArray title = table({{0x0181, 0x00FF00}, {0x01BF, 0x00100010}, {0x01FF, 0x00080000}});
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), group, QByteArray(), title), &err);
        QVERIFY2(d, qPrintable(err));
        const Item *titleItem = nullptr, *bodyItem = nullptr;
        for (const auto &it : d->pages[0]->items) {
            if (it->role == QLatin1String("ph:title")) titleItem = it.get();
            if (it->role.startsWith(QLatin1String("ph:body"))) bodyItem = it.get();
        }
        QVERIFY(titleItem && bodyItem);
        QVERIFY(titleItem->fill.type == Fill::Solid);
        QCOMPARE(titleItem->fill.color.rgbValue(), QColor(0, 255, 0));
        QVERIFY(titleItem->stroke.isNone());
        // The master's body says nothing: a text frame is neither filled nor lined of itself, whatever the defaults are.
        QVERIFY(bodyItem->fill.isNone());
        QVERIFY(bodyItem->stroke.isNone());
    }

    // A texture with a brightness and a contrast (a background's, washed out) has them in its pixels, where a fill has no other place
    // for them.
    void importBinaryTexturesTakeTheirBrightnessAndContrast()
    {
        QImage tile(2, 1, QImage::Format_ARGB32);
        tile.setPixelColor(0, 0, QColor(255, 0, 0));
        tile.setPixelColor(1, 0, QColor(0, 0, 128));
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        tile.save(&buffer, "PNG");
        // The drawing group's picture store: one picture, kept in its own record.
        const QByteArray blip = pptRec(0xF01E, QByteArray(16, 'u') + QByteArray(1, '\xFF') + png, 0x6E0);
        const QByteArray entry = pptRec(0xF007, QByteArray(1, '\x06') + QByteArray(1, '\0') + QByteArray(16, 's') + le(0xFF, 2) + le(quint32(blip.size()), 4) + le(1, 4) + le(0xFFFFFFFF, 4) + le(0, 4) + blip, 6, 2);
        const QByteArray group = pptBox(1035, pptBox(0xF000, pptRec(0xF006, QByteArray(16, '\0')) + pptBox(0xF001, entry, 1)));
        auto table = [&](const QVector<QPair<int, quint32>> &props) {
            QByteArray t;
            for (const auto &p : props) t += le(quint16(p.first), 2) + le(p.second, 4);
            return pptRec(0xF00B, t, quint16(props.size()), 3);
        };
        auto pixels = [&](const QVector<QPair<int, quint32>> &more) {
            QVector<QPair<int, quint32>> props = {{0x0180, 2}, {0x4186, 1}, {0x01BF, 0x00100010}};
            props += more;
            const QByteArray background = pptBox(0xF004, pptRec(0xF00A, le(0x1000, 4) + le(0xC00, 4), 1, 2) + table(props));
            QString err;
            auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), group, QByteArray(), QByteArray(), background), &err);
            QList<QColor> out;
            if (!d) qWarning("%s", qPrintable(err));
            if (!d || d->masters.isEmpty() || d->masters.first()->background.type != Fill::Texture) return out;
            const QImage im = d->image(d->masters.first()->background.imageId);
            for (int x = 0; x < im.width(); ++x) out << im.pixelColor(x, 0);
            return out;
        };
        // As it is, with none set.
        QList<QColor> plain = pixels({});
        QCOMPARE(int(plain.size()), 2);
        QCOMPARE(plain[0], QColor(255, 0, 0));
        QCOMPARE(plain[1], QColor(0, 0, 128));
        // Half brighter (0x4000 of 0x8000): half of the range more in each color, and no more than the most.
        QList<QColor> bright = pixels({{0x0109, 0x4000}});
        QCOMPARE(int(bright.size()), 2);
        QCOMPARE(bright[0], QColor(255, 128, 128));
        QCOMPARE(bright[1], QColor(128, 128, 255));
        // A contrast of 0.5 (0x8000 of 0x10000) halves the distance of each color from the middle.
        QList<QColor> flat = pixels({{0x0108, 0x8000}});
        QCOMPARE(int(flat.size()), 2);
        QCOMPARE(flat[0], QColor(191, 64, 64));
        QCOMPARE(flat[1], QColor(64, 64, 128));
        // Both, as the washed-out look is stored (0x5C29 and 0xD70A): the colors come to the top of the range, from about three quarters.
        QList<QColor> washed = pixels({{0x0109, 0x5C29}, {0x0108, 0xD70A}});
        QCOMPARE(int(washed.size()), 2);
        QVERIFY2(washed[0].green() > 185 && washed[0].green() < 195, qPrintable(washed[0].name()));
        QCOMPARE(washed[0].red(), 255);
    }

    // A shape that shows a tiled background has the tiles where the page has them, not from the shape's own corner: the picture is moved
    // by the shape's place.
    void importBinaryShapesKeepTheBackgroundsTilesInPlace()
    {
        QImage tile(4, 1, QImage::Format_ARGB32);
        const QColor colors[4] = {QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255), QColor(255, 255, 0)};
        for (int i = 0; i < 4; ++i) tile.setPixelColor(i, 0, colors[i]);
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        tile.save(&buffer, "PNG");
        const QByteArray blip = pptRec(0xF01E, QByteArray(16, 'u') + QByteArray(1, '\xFF') + png, 0x6E0);
        const QByteArray entry = pptRec(0xF007, QByteArray(1, '\x06') + QByteArray(1, '\0') + QByteArray(16, 's') + le(0xFF, 2) + le(quint32(blip.size()), 4) + le(1, 4) + le(0xFFFFFFFF, 4) + le(0, 4) + blip, 6, 2);
        const QByteArray group = pptBox(1035, pptBox(0xF000, pptRec(0xF006, QByteArray(16, '\0')) + pptBox(0xF001, entry, 1)));
        auto table = [&](const QVector<QPair<int, quint32>> &props) {
            QByteArray t;
            for (const auto &p : props) t += le(quint16(p.first), 2) + le(p.second, 4);
            return pptRec(0xF00B, t, quint16(props.size()), 3);
        };
        const QByteArray background = pptBox(0xF004, pptRec(0xF00A, le(0x1000, 4) + le(0xC00, 4), 1, 2) + table({{0x0180, 2}, {0x4186, 1}, {0x01BF, 0x00100010}}));
        // Rectangles that show the background, at 6 master units (0.75 points, one pixel of a picture) from the left, and at the page's edge.
        auto window = [&](quint32 id, int left) {
            return pptBox(0xF004, pptRec(0xF00A, le(id, 4) + le(0xA00, 4), 1, 2) + table({{0x0180, 9}, {0x01BF, 0x00100010}}) + pptRec(0xF010, le(100, 2) + le(left, 2) + le(2000, 2) + le(600, 2)));
        };
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), group, window(0x500, 6) + window(0x501, 0), QByteArray(), background), &err);
        QVERIFY2(d, qPrintable(err));
        QVector<const Item *> windows;
        for (const auto &it : d->pages[0]->items)
            if (it->role.isEmpty() && it->type() == ItemType::Shape) windows << it.get();
        QCOMPARE(int(windows.size()), 2);
        auto pixels = [&](const Item *it) {
            QList<QColor> out;
            if (it->fill.type != Fill::Texture) return out;
            const QImage im = d->image(it->fill.imageId);
            for (int x = 0; x < im.width(); ++x) out << im.pixelColor(x, 0);
            return out;
        };
        // Moved by a pixel: what the page has a pixel along is at the shape's corner.
        const QList<QColor> moved = pixels(windows[0]);
        QCOMPARE(int(moved.size()), 4);
        for (int i = 0; i < 4; ++i) QCOMPARE(moved[i], colors[(i + 1) % 4]);
        // At the edge, as it is, and the same picture the page shows.
        QCOMPARE(windows[1]->fill.imageId, d->masters.first()->background.imageId);
    }

    // A fill of the background kind (the master's title as a window onto the background) shows what the page has behind it:
    // the master's own background, which the slides on it show too.
    void importBinaryShapesCanFillWithTheBackground()
    {
        auto table = [&](const QVector<QPair<int, quint32>> &props) {
            QByteArray t;
            for (const auto &p : props) t += le(quint16(p.first), 2) + le(p.second, 4);
            return pptRec(0xF00B, t, quint16(props.size()), 3);
        };
        // The master's background shape is solid (a color of its own), and its title's fill is the background's.
        const QByteArray background = pptBox(0xF004, pptRec(0xF00A, le(0x1000, 4) + le(0xC00, 4), 1, 2) + table({{0x0181, 0xCCBBAA}, {0x01BF, 0x00100010}}));
        const QByteArray title = table({{0x0180, 9}, {0x01BF, 0x00100010}, {0x01FF, 0x00080000}});
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), title, background), &err);
        QVERIFY2(d, qPrintable(err));
        const Item *titleItem = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->role == QLatin1String("ph:title")) titleItem = it.get();
        QVERIFY(titleItem);
        QVERIFY(titleItem->fill.type == Fill::Solid);
        QCOMPARE(titleItem->fill.color.rgbValue(), QColor(0xAA, 0xBB, 0xCC));
        // Without a background shape the master has the scheme's background color, and that is what the title shows.
        d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), title), &err);
        QVERIFY2(d, qPrintable(err));
        titleItem = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->role == QLatin1String("ph:title")) titleItem = it.get();
        QVERIFY(titleItem);
        QCOMPARE(titleItem->fill.color.rgbValue(), QColor(0xFA, 0xFA, 0xFA));
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

private:
    // An exported file as the tests below look at it.
    struct Exported {
        QMap<QString, QByteArray> zip;   // by entry name, no leading slash
        office::Package pkg;
    };
    static bool unpack(const QByteArray &bytes, Exported *out, QString *why)
    {
        return readZip(bytes, out->zip, why) && out->pkg.read(bytes, why);
    }
    static QString part(const Exported &e, const QString &name) { return QString::fromUtf8(e.zip.value(name.mid(1))); }

    // Whether a drawing writer that writes shapes is in place (the first version writes none).
    static bool writerWritesShapes()
    {
        ShapeItem shape;
        shape.rect = QRectF(0, 0, 10, 10);
        office::PackageWriter package;
        office::DrawingWriteContext ctx;
        ctx.package = &package;
        ctx.part = QStringLiteral("/ppt/slides/slide1.xml");
        ctx.storyDoc = [](const QString &) -> const QTextDocument * { return nullptr; };
        QByteArray out;
        QXmlStreamWriter w(&out);
        office::writeShape(w, shape, ctx);
        return !out.isEmpty();
    }

    // The first thing wrong with a package, as a reader that stops at the first
    // damage would find it; empty when it is sound: every part has a content
    // type and every type names a part, every XML part parses, every
    // relationship and every r:id in a part resolves, and every part is
    // reached from the package's relationships.
    static QString packageProblem(const Exported &e)
    {
        QDomDocument types;
        QString why;
        if (!e.zip.contains(QStringLiteral("[Content_Types].xml"))) return QStringLiteral("no [Content_Types].xml");
        if (!office::parseXml(e.zip.value(QStringLiteral("[Content_Types].xml")), &types, &why)) return QStringLiteral("[Content_Types].xml: ") + why;
        QSet<QString> defaults, overrides;
        for (const QDomElement &d : office::children(types.documentElement(), office::ns::ContentTypes, QStringLiteral("Default"))) defaults << d.attribute(QStringLiteral("Extension")).toLower();
        for (const QDomElement &o : office::children(types.documentElement(), office::ns::ContentTypes, QStringLiteral("Override"))) {
            const QString name = o.attribute(QStringLiteral("PartName"));
            if (!e.zip.contains(name.mid(1))) return QStringLiteral("the content types list a part that is missing: ") + name;
            overrides << name.toLower();
        }
        for (auto it = e.zip.cbegin(); it != e.zip.cend(); ++it) {
            if (it.key() == QLatin1String("[Content_Types].xml")) continue;
            if (!defaults.contains(QFileInfo(it.key()).suffix().toLower()) && !overrides.contains(QLatin1Char('/') + it.key().toLower()))
                return QStringLiteral("no content type for ") + it.key();
            if (!it.key().endsWith(QLatin1String(".xml")) && !it.key().endsWith(QLatin1String(".rels"))) continue;
            QDomDocument dom;
            if (!office::parseXml(it.value(), &dom, &why)) return it.key() + QStringLiteral(" doesn't parse: ") + why;
            if (!it.value().startsWith("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>")) return it.key() + QStringLiteral(" has no XML declaration");
            // Every r:id (and r:embed...) in a part is one of its relationships.
            if (it.key().endsWith(QLatin1String(".rels"))) continue;
            QVector<QDomElement> stack{dom.documentElement()};
            while (!stack.isEmpty()) {
                const QDomElement el = stack.takeLast();
                const QDomNamedNodeMap attrs = el.attributes();
                for (int i = 0; i < attrs.length(); ++i) {
                    const QDomAttr a = attrs.item(i).toAttr();
                    if (!office::sameNamespace(a.namespaceURI(), office::ns::R)) continue;
                    if (e.pkg.relationship(QLatin1Char('/') + it.key(), a.value()).id.isEmpty()) return it.key() + QStringLiteral(" uses ") + a.name() + QStringLiteral("=") + a.value() + QStringLiteral(", which isn't a relationship");
                }
                for (QDomElement c = el.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack << c;
            }
        }
        QStringList sources = e.pkg.partNames();
        sources << QStringLiteral("/");
        for (const QString &s : std::as_const(sources))
            for (const office::Relationship &r : e.pkg.relationships(s)) {
                if (!r.external && !e.pkg.hasPart(r.target)) return s + QStringLiteral(" links to a part that is missing: ") + r.target;
                if (!r.external && r.type.isEmpty()) return s + QStringLiteral(" has a link without a type");
            }
        QSet<QString> reached;
        QStringList queue{QStringLiteral("/")};
        while (!queue.isEmpty()) {
            const QString s = queue.takeLast();
            for (const office::Relationship &r : e.pkg.relationships(s))
                if (!r.external && !reached.contains(r.target.toLower())) {
                    reached << r.target.toLower();
                    queue << r.target;
                }
        }
        for (const QString &name : e.pkg.partNames())
            if (!reached.contains(name.toLower())) return name + QStringLiteral(" is not linked from anywhere");
        return QString();
    }

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
    static QDomElement rootOf(const QByteArray &xml, QDomDocument *dom)
    {
        office::parseXml(xml, dom);
        return dom->documentElement();
    }
    static QStringList relationshipTargets(const Exported &e, const QString &source, const QString &typeSuffix)
    {
        QStringList out;
        for (const office::Relationship &r : e.pkg.relationshipsOfType(source, typeSuffix)) out << r.target;
        return out;
    }

    // Answers the modal dialogs that open next, one for each answer: a button of a
    // message box, or (NoButton) closing the dialog. The first label clicked is kept in `label`.
    static void answerDialogs(QList<QMessageBox::StandardButton> answers, QString *label = nullptr, QDeadlineTimer until = QDeadlineTimer(10000))
    {
        if (answers.isEmpty() || until.hasExpired()) return;
        QTimer::singleShot(0, [answers, label, until]() mutable {
            QWidget *dialog = QApplication::activeModalWidget();
            if (!dialog) {
                answerDialogs(answers, label, until);
                return;
            }
            const QMessageBox::StandardButton which = answers.takeFirst();
            auto *box = qobject_cast<QMessageBox *>(dialog);
            QAbstractButton *button = box && which != QMessageBox::NoButton ? box->button(which) : nullptr;
            if (button && label && label->isEmpty()) *label = button->text();
            if (button) button->click();
            else dialog->close();
            answerDialogs(answers, label, until);
        });
    }
    static QByteArray fileBytes(const QString &path)
    {
        QFile f(path);
        return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
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
    // keeps (transition, timing, frames) as it was. The shapes and the text in
    // them come back once the drawing writer writes them.
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
        if (!writerWritesShapes()) QEXPECT_FAIL("", "drawing writer pending", Continue);
        QCOMPARE(deck::notesText(*back, *back->pages[0]), QStringLiteral("Say hello first\nThen the numbers"));
        QVERIFY(deck::notesText(*back, *back->pages[1]).isEmpty());
        // The shapes: the placeholders with their roles, the text, the picture.
        if (!writerWritesShapes()) QEXPECT_FAIL("", "drawing writer pending", Continue);
        QCOMPARE(int(back->pages[0]->items.size()), int(d->pages[0]->items.size()));
        if (!writerWritesShapes()) QEXPECT_FAIL("", "drawing writer pending", Continue);
        {
            QStringList roles, backRoles;
            for (const auto &it : d->pages[0]->items) roles << it->role;
            for (const auto &it : back->pages[0]->items) backRoles << it->role;
            QCOMPARE(backRoles, roles);
        }
        if (!writerWritesShapes()) QEXPECT_FAIL("", "drawing writer pending", Continue);
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
        QCOMPARE(deck::themeOf(*back).headingFont, QStringLiteral("Carlito"));
        QCOMPARE(deck::themeOf(*back).bodyFont, QStringLiteral("Carlito"));
        if (!writerWritesShapes()) QEXPECT_FAIL("", "drawing writer pending", Continue);
        QCOMPARE(int(back->pages[0]->items.size()), 2);
        if (!writerWritesShapes()) QEXPECT_FAIL("", "drawing writer pending", Continue);
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
        // An animation that aims at a shape by its number, and a transition with a sound.
        xml.replace(QRegularExpression(QStringLiteral("<p:timing>.*</p:timing>")),
                    QStringLiteral("<p:timing><p:tnLst><p:par><p:cTn id=\"1\" dur=\"indefinite\" restart=\"never\" nodeType=\"tmRoot\"><p:childLst><p:set><p:cBhvr><p:cTn id=\"2\"/><p:tgtEl><p:spTgt spid=\"5\"/></p:tgtEl></p:cBhvr></p:set></p:childLst></p:cTn></p:par></p:tnLst></p:timing>"));
        xml.replace(QStringLiteral("<p:transition spd=\"med\"><p:fade/></p:transition>"),
                    QStringLiteral("<p:transition spd=\"med\"><p:fade/><p:sndAc><p:stSnd><p:snd r:embed=\"rId8\" name=\"chime.wav\"/></p:stSnd></p:sndAc></p:transition>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages[0]->props.value(QStringLiteral("unread")).toArray().size()), 2);   // the table, and the chart

        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(slide.contains(QStringLiteral("Cell &amp; &lt;1&gt;")));   // the table has nothing to point at, and stays
        QVERIFY(!slide.contains(QStringLiteral("c:chart")));
        QVERIFY(!slide.contains(QStringLiteral("rId9")));
        QVERIFY(!slide.contains(QStringLiteral("<p:timing")));
        QVERIFY(!slide.contains(QStringLiteral("chime.wav")));
        QVERIFY(!slide.contains(QStringLiteral("<p:transition")));
        int notes = 0;
        for (const QString &s : std::as_const(skipped))
            if (s.startsWith(QLatin1String("Slide 1"))) ++notes;
        QVERIFY2(notes >= 3, qPrintable(skipped.join('\n')));   // the chart, the animation, the transition
        // The shape number of the table is not given to anything else on the slide.
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

    // A theme's picture fills (its textures) point at pictures the reader did not keep:
    // written as they were, they would point nowhere, so they become plain fills.
    void exportLeavesOutThePicturesOfATheme()
    {
        Parts p = samplePptx();
        QByteArray theme = p.data["/ppt/theme/theme1.xml"].xml;
        const QByteArray texture =
            "<a:blipFill dpi=\"0\" rotWithShape=\"1\"><a:blip xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" r:embed=\"rId1\">"
            "<a:duotone><a:schemeClr val=\"phClr\"/></a:duotone></a:blip><a:srcRect/><a:tile tx=\"0\" ty=\"0\" sx=\"100000\" sy=\"100000\" flip=\"none\" algn=\"tl\"/></a:blipFill>";
        theme.replace("<a:bgFillStyleLst>", "<a:bgFillStyleLst>" + texture);
        p.data["/ppt/theme/theme1.xml"].xml = theme;
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        // The rest of the part is as it was.
        QCOMPARE(e.zip.value(QStringLiteral("ppt/theme/theme1.xml")), QByteArray(theme).replace(texture, "<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>"));
        office::Theme read;
        QVERIFY2(office::readTheme(e.zip.value(QStringLiteral("ppt/theme/theme1.xml")), &read, &err), qPrintable(err));
        QCOMPARE(read.colors[4], QColor(0xB6, 0x42, 0x0F));
        // The same for the theme of a second slide master, which is kept in its master's properties.
        auto second = std::make_shared<MasterPage>(*d->masters[0]);
        second->id = QStringLiteral("SM2");
        second->items.clear();
        second->props[QStringLiteral("themeXml")] = QString::fromLatin1(theme.toBase64());
        d->masters << second;
        Exported two;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &two, &err), qPrintable(err));
        QVERIFY2(packageProblem(two).isEmpty(), qPrintable(packageProblem(two)));
        QVERIFY(!part(two, QStringLiteral("/ppt/theme/theme2.xml")).contains(QStringLiteral("embed")));
        // A theme that points at something else that isn't kept is made again from its colors and fonts.
        QByteArray odd = QByteArray(theme).replace("<a:objectDefaults/>", "");
        odd.replace("</a:themeElements>", "</a:themeElements><a:objectDefaults><a:spDef><a:spPr/><a:bodyPr/><a:lstStyle/><a:style><a:lnRef idx=\"1\"><a:schemeClr val=\"accent1\"/></a:lnRef>"
                                          "<a:fillRef idx=\"1\"><a:schemeClr val=\"accent1\"/></a:fillRef><a:effectRef idx=\"0\"><a:schemeClr val=\"accent1\"/></a:effectRef>"
                                          "<a:fontRef idx=\"minor\"><a:hlinkClick xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" r:id=\"rId5\"/></a:fontRef></a:style></a:spDef></a:objectDefaults>");
        second->props[QStringLiteral("themeXml")] = QString::fromLatin1(odd.toBase64());
        Exported three;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &three, &err), qPrintable(err));
        QVERIFY2(packageProblem(three).isEmpty(), qPrintable(packageProblem(three)));
        QVERIFY2(office::readTheme(three.zip.value(QStringLiteral("ppt/theme/theme2.xml")), &read, &err), qPrintable(err));
        QCOMPARE(read.colors[4], QColor(0xB6, 0x42, 0x0F));
    }

    // The same presentation as a template or a show differs in one content type.
    void exportKindsDifferInTheMainPartsType()
    {
        auto d = deck::newPresentation();
        QString err;
        const QString base = QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.");
        const struct { deck::PresentationKind kind; const char *type; } kinds[] = {
            {deck::PresentationKind::Presentation, "presentation.main+xml"}, {deck::PresentationKind::Template, "template.main+xml"}, {deck::PresentationKind::Show, "slideshow.main+xml"}};
        for (const auto &k : kinds) {
            Exported e;
            QVERIFY2(unpack(deck::exportPresentation(*d, &err, nullptr, k.kind), &e, &err), qPrintable(err));
            QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/presentation.xml")), base + QLatin1String(k.type));
            QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        }
        QCOMPARE(deck::kindOfFileName(QStringLiteral("/tmp/a.pptx")), deck::PresentationKind::Presentation);
        QCOMPARE(deck::kindOfFileName(QStringLiteral("/tmp/a.POTX")), deck::PresentationKind::Template);
        QCOMPARE(deck::kindOfFileName(QStringLiteral("/tmp/a.ppsx")), deck::PresentationKind::Show);
        QCOMPARE(deck::kindOfFileName(QStringLiteral("/tmp/a")), deck::PresentationKind::Presentation);
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

QTEST_MAIN(DeckTests)
#include "tst_deck.moc"
