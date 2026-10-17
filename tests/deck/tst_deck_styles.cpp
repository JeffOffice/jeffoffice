// JeffDeck's tests of the Shape Format tab's styles: the Quick Styles gallery drawn from the theme (and how a style is saved), Change Shape, and
// the WordArt styles of the text in a shape. Each command is one step that undoes, and comes back from a .pptx and from an .odp.
#include "deckhelpers.h"

#include "app/formatops.h"
#include "core/shapestyles.h"
#include "core/themes.h"
#include "render/shapes.h"

#include <QMenu>
#include <QToolButton>

namespace {

// A window over a slide with nothing on it, and the shapes the tests add.
struct Rig {
    Rig()
    {
        w.editor()->surfaceItems().clear();
    }
    Editor *ed() const { return w.editor(); }
    Item *item(const QString &id) const { return ed()->doc()->item(id); }
    ShapeItem *shape(const QString &id) const { return static_cast<ShapeItem *>(item(id)); }
    // A shape with words in it, a fill, a line, and a shadow of its own, on a slide.
    ShapeItem *addShape(const QString &name, const QString &words = QString(), const QRectF &rect = QRectF(100, 100, 200, 100))
    {
        auto s = std::make_shared<ShapeItem>();
        s->name = name;
        s->rect = rect;
        s->rotation = 20;
        s->flipH = true;
        s->fill = Fill::solid(ColorRef::rgb(QColor(200, 30, 30)), 0.25);
        s->stroke = Stroke::line(ColorRef::rgb(QColor(20, 20, 160)), 3);
        s->fx.shadow.on = true;
        s->fx.shadow.distance = 7;
        if (!words.isEmpty()) s->storyId = ed()->doc()->createStory(words);
        ed()->addItem(s, false);
        ids << s->id;
        return s.get();
    }
    int steps() const { return ed()->undoStack()->index(); }
    std::unique_ptr<Document> viaPptx() const
    {
        QString err;
        const QByteArray bytes = deck::exportPresentation(*ed()->doc(), &err);
        return deck::importPresentation(bytes, &err);
    }
    std::unique_ptr<Document> viaOdp() const
    {
        QString err;
        const QByteArray bytes = deck::exportOdp(*ed()->doc(), false, &err);
        return deck::importOdp(bytes, &err);
    }
    static Item *named(Document &d, const QString &name)
    {
        Item *found = nullptr;
        if (d.pages.isEmpty()) return nullptr;
        walkItems(d.pages[0]->items, [&](const ItemPtr &it) {
            if (it->name == name && !found) found = it.get();
        });
        return found;
    }
    // The button of a command of the tab (a gallery or a menu), by the name its widget or action has.
    template <class T> T *widget(const char *name) const
    {
        T *found = w.ribbon()->findChild<T *>(QLatin1String(name));
        if (!found) qWarning("no %s on the ribbon", name);
        return found;
    }

    mutable DeckWindow w;
    QStringList ids;
};

bool near(double a, double b, double by = 0.01) { return std::abs(a - b) <= by; }

// How far apart two colors are, by the largest difference of a channel.
int apart(const QColor &a, const QColor &b) { return std::max({std::abs(a.red() - b.red()), std::abs(a.green() - b.green()), std::abs(a.blue() - b.blue())}); }

// The p:sp or p:cxnSp of a slide's XML that has this name, as an element of `dom`.
QDomElement shapeNamed(const QString &xml, const QString &name, QDomDocument *dom)
{
    QString err;
    if (!office::parseXml(xml.toUtf8(), dom, &err)) return QDomElement();
    std::function<QDomElement(const QDomElement &)> find = [&](const QDomElement &e) -> QDomElement {
        for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
            if (c.localName() == QLatin1String("cNvPr") && c.attribute(QStringLiteral("name")) == name) return e.parentNode().toElement();
            if (const QDomElement found = find(c); !found.isNull()) return found;
        }
        return QDomElement();
    };
    return find(dom->documentElement());
}

// The first child with this name ("p:spPr": the prefix is not looked at).
QDomElement childByTag(const QDomElement &e, const char *tag)
{
    const QString local = QString::fromLatin1(tag).section(QLatin1Char(':'), -1);
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == local) return c;
    return QDomElement();
}

} // namespace

class DeckStylesTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // ---- Shape Styles

    // The gallery is four rows (an outline alone, then a subtle, a moderate, and an intense fill, line, and shadow) of the theme's six accents,
    // each drawn as the shape it gives, and named as the original names them. A style in another accent is another color.
    void shapeStylesGalleryIsDrawnFromTheThemesAccents()
    {
        Rig r;
        const QVector<GalleryItem> items = r.w.shapeStyleItems();
        const QVector<deck::ShapeStyle> styles = deck::shapeStyles();
        QCOMPARE(items.size(), 24);
        QCOMPARE(styles.size(), 24);
        QStringList groups;
        const deck::DeckTheme theme = deck::themeOf(*r.ed()->doc());
        for (int i = 0; i < items.size(); ++i) {
            QCOMPARE(items[i].id, styles[i].id);
            QCOMPARE(items[i].tip, styles[i].name);
            QVERIFY(items[i].tip.endsWith(QStringLiteral(" - Accent %1").arg(i % 6 + 1)));
            QVERIFY(!items[i].icon.isNull());
            if (!groups.contains(items[i].group)) groups << items[i].group;
            QCOMPARE(items[i].group, styles[i % 6 + i / 6 * 6].group);
        }
        QCOMPARE(groups, (QStringList{QStringLiteral("Outline Only"), QStringLiteral("Subtle Effect"), QStringLiteral("Moderate Effect"), QStringLiteral("Intense Effect")}));
        // Drawn by the renderer in the theme's colors: the middle of a moderate style is its accent (a gradient from a lighter to a darker
        // shade of it), and of an outline alone, the slide (the line is at the edge).
        for (int accent = 0; accent < 6; ++accent) {
            const QImage moderate = items[12 + accent].icon.pixmap(QSize(64, 48)).toImage();
            const QColor middle = moderate.pixelColor(moderate.width() / 2, moderate.height() / 3);
            QVERIFY2(apart(middle, theme.colors[4 + accent]) < 70, qPrintable(QStringLiteral("accent %1: %2 not near %3").arg(accent + 1).arg(middle.name(), theme.colors[4 + accent].name())));
            const QImage outline = items[accent].icon.pixmap(QSize(64, 48)).toImage();
            QVERIFY(apart(outline.pixelColor(10, 6), theme.colors[4 + accent]) > 40);   // (its inside is not filled)
        }
        QVERIFY(apart(items[12].icon.pixmap(QSize(64, 48)).toImage().pixelColor(32, 16), items[13].icon.pixmap(QSize(64, 48)).toImage().pixelColor(32, 16)) > 20);
    }

    // A style gives the shapes it is clicked on the theme's fill, line, and shadow for its kind and accent, as one step; its text takes the style's
    // color where it had none of its own; a line keeps its arrowheads and takes no fill. Undo takes it all back.
    void applyingAStyleGivesFillLineAndShadowFromTheTheme()
    {
        Rig r;
        const QString a = r.addShape(QStringLiteral("A"), QStringLiteral("Words"), QRectF(40, 40, 140, 80))->id;
        const QString b = r.addShape(QStringLiteral("B"), QString(), QRectF(220, 40, 140, 80))->id;
        auto line = std::make_shared<LineItem>();
        line->name = QStringLiteral("L");
        line->p1 = QPointF(40, 300);
        line->p2 = QPointF(300, 300);
        line->syncRect();
        line->stroke = Stroke::line(ColorRef::rgb(Qt::black), 5);
        line->stroke.endArrow = Arrow::Triangle;
        r.ed()->addItem(line, false);
        const QString l = line->id;
        const deck::DeckTheme theme = deck::themeOf(*r.ed()->doc());
        r.ed()->select(QStringList{a, b, l});
        const int base = r.steps();
        QVERIFY(r.w.applyShapeStyle(QStringLiteral("intense:accent2")));
        QCOMPARE(r.steps(), base + 1);
        const QColor accent = theme.colors[5];
        for (const QString &id : {a, b}) {
            const Item *it = r.item(id);
            QVERIFY(it->styleRef.on);
            QCOMPARE(it->styleRef.line.idx, 3);
            QCOMPARE(it->styleRef.fill.idx, 3);
            QCOMPARE(it->styleRef.effect.idx, 3);
            QCOMPARE(it->styleRef.fill.color.name, QStringLiteral("accent2"));
            QCOMPARE(it->styleRef.line.color.mods, (QVector<QPair<QString, int>>{{QStringLiteral("shade"), 50000}}));
            QCOMPARE(it->styleRef.fontColor.name, QStringLiteral("lt1"));
            // The theme's third fill is a gradient from a lighter shade of the color to a darker, and its third line is 1.5 points.
            QCOMPARE(it->fill.type, Fill::Gradient);
            QCOMPARE(it->fill.stops.size(), 2);
            const QColor first = it->fill.stops.first().color.resolve(r.ed()->doc()->colors), last = it->fill.stops.last().color.resolve(r.ed()->doc()->colors);
            QVERIFY(first.lightness() > accent.lightness());
            QVERIFY(last.lightness() < accent.lightness());
            QVERIFY2(std::abs(first.hue() - accent.hue()) < 15 && std::abs(last.hue() - accent.hue()) < 15, qPrintable(QStringLiteral("%1 %2 %3").arg(first.hue()).arg(accent.hue()).arg(last.hue())));
            QCOMPARE(it->stroke.width, 1.5);
            const QColor edge = it->stroke.color.resolve(r.ed()->doc()->colors);
            QVERIFY(edge.lightness() < accent.lightness());
            QVERIFY(it->fx.shadow.on);
            QVERIFY(!it->fx.shadow.inner);
        }
        // The words had no color; they take the style's light 1.
        QTextDocument *words = r.ed()->doc()->storyDoc(r.shape(a)->storyId);
        QTextCursor c(words);
        c.setPosition(1);
        QCOMPARE(ColorRef::fromString(c.charFormat().stringProperty(tp::ColorRefP)).resolve(r.ed()->doc()->colors), theme.colors[1]);
        // The line: its own color and weight from the style, its arrow kept, and no fill.
        const Item *ln = r.item(l);
        QCOMPARE(ln->stroke.width, 1.5);
        QCOMPARE(ln->stroke.endArrow, Arrow::Triangle);
        QVERIFY(ln->fill.isNone());
        QVERIFY(ln->styleRef.on);
        // Another style over it: the words follow it (they were the old style's color), and an outline alone has no fill and a line of the accent.
        QVERIFY(r.w.applyShapeStyle(QStringLiteral("outline:accent5")));
        QCOMPARE(r.steps(), base + 2);
        const Item *outline = r.item(a);
        QVERIFY(outline->fill.isNone());
        QCOMPARE(outline->stroke.width, 1.0);
        QCOMPARE(outline->stroke.color.resolve(r.ed()->doc()->colors), theme.colors[8]);
        QVERIFY(!outline->fx.shadow.on);
        QCOMPARE(outline->styleRef.fontColor.name, QStringLiteral("tx1"));
        QTextCursor again(r.ed()->doc()->storyDoc(r.shape(a)->storyId));
        again.setPosition(1);
        QCOMPARE(ColorRef::fromString(again.charFormat().stringProperty(tp::ColorRefP)).resolve(r.ed()->doc()->colors), theme.colors[0]);
        // Undo goes back one step at a time, to the shapes as they were.
        r.ed()->undo();
        QCOMPARE(r.item(a)->styleRef.line.idx, 3);
        r.ed()->undo();
        for (const QString &id : {a, b}) {
            const Item *it = r.item(id);
            QVERIFY(!it->styleRef.on);
            QCOMPARE(it->fill.color, ColorRef::rgb(QColor(200, 30, 30)));
            QCOMPARE(it->fill.transparency, 0.25);
            QCOMPARE(it->stroke.width, 3.0);
            QCOMPARE(it->fx.shadow.distance, 7.0);
        }
        QVERIFY(!r.item(l)->styleRef.on);
        QCOMPARE(r.item(l)->stroke.width, 5.0);
        QTextCursor undone(r.ed()->doc()->storyDoc(r.shape(a)->storyId));
        undone.setPosition(1);
        QVERIFY(undone.charFormat().stringProperty(tp::ColorRefP).isEmpty());
    }

    // Words with a color of their own keep it under a style, and only the gallery's own kinds take a style: not a picture or a table.
    void aStyleLeavesColoredWordsAndOtherKindsAlone()
    {
        Rig r;
        ShapeItem *s = r.addShape(QStringLiteral("Red words"), QStringLiteral("Red words"));
        const QString id = s->id;
        {
            QTextCursor c(r.ed()->doc()->storyDoc(s->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat cf;
            cf.setProperty(tp::ColorRefP, ColorRef::rgb(Qt::red).toString());
            cf.setForeground(QBrush(Qt::red));
            c.mergeCharFormat(cf);
        }
        auto table = r.ed()->newTable(QRectF(40, 300, 200, 80), 2, 2);
        r.ed()->addItem(table, false);
        auto *gallery = r.widget<Gallery>("shapeStyleGallery");
        QVERIFY(gallery);
        r.ed()->clearSelection();
        QVERIFY(!gallery->isEnabled());
        r.ed()->select(table->id);
        QVERIFY(!gallery->isEnabled());
        const int base = r.steps();
        QVERIFY(!r.w.applyShapeStyle(QStringLiteral("moderate:accent1")));
        QCOMPARE(r.steps(), base);
        r.ed()->select(QStringList{id, table->id});
        QVERIFY(gallery->isEnabled());
        QVERIFY(!r.w.applyShapeStyle(QStringLiteral("no such style")));
        QVERIFY(r.w.applyShapeStyle(QStringLiteral("moderate:accent1")));
        QTextCursor c(r.ed()->doc()->storyDoc(r.shape(id)->storyId));
        c.setPosition(1);
        QCOMPARE(ColorRef::fromString(c.charFormat().stringProperty(tp::ColorRefP)).resolve(r.ed()->doc()->colors), QColor(Qt::red));
        QVERIFY(!r.ed()->doc()->item(table->id)->styleRef.on);
    }

    // A style is saved as the presentation program saves it: the shape's style reference (lnRef, fillRef, effectRef, and fontRef) with only the
    // properties that differ from what it gives; one changed after is written beside the reference, and the file reads back the same.
    void styleIsSavedAsTheReferenceWithOnlyWhatDiffers()
    {
        Rig r;
        const QString id = r.addShape(QStringLiteral("Styled"), QStringLiteral("Words in it"), QRectF(100, 100, 220, 110))->id;
        r.ed()->select(id);
        QVERIFY(r.w.applyShapeStyle(QStringLiteral("intense:accent4")));
        const Fill fill = r.item(id)->fill;
        const Stroke stroke = r.item(id)->stroke;
        const QJsonObject fx = r.item(id)->fx.toJson();
        QString err;
        Exported e;
        QVERIFY(unpack(deck::exportPresentation(*r.ed()->doc(), &err), &e, &err));
        // The theme made here has the format styles the style refers to: a shadow among the effect styles.
        QVERIFY(part(e, QStringLiteral("/ppt/theme/theme1.xml")).contains(QStringLiteral("<a:outerShdw")));
        QDomDocument dom;
        const QDomElement sp = shapeNamed(part(e, QStringLiteral("/ppt/slides/slide1.xml")), QStringLiteral("Styled"), &dom);
        QVERIFY(!sp.isNull());
        const QDomElement spPr = childByTag(sp, "p:spPr"), style = childByTag(sp, "p:style");
        QVERIFY(!style.isNull());
        // The properties say the shape, and the style says its look: nothing of the fill, line, or effects is written beside it.
        for (const char *tag : {"a:solidFill", "a:gradFill", "a:noFill", "a:ln", "a:effectLst"}) QVERIFY2(childByTag(spPr, tag).isNull(), tag);
        const QDomElement ln = childByTag(style, "a:lnRef"), fl = childByTag(style, "a:fillRef"), fxr = childByTag(style, "a:effectRef"), font = childByTag(style, "a:fontRef");
        QCOMPARE(ln.attribute(QStringLiteral("idx")), QStringLiteral("3"));
        QCOMPARE(fl.attribute(QStringLiteral("idx")), QStringLiteral("3"));
        QCOMPARE(fxr.attribute(QStringLiteral("idx")), QStringLiteral("3"));
        QCOMPARE(font.attribute(QStringLiteral("idx")), QStringLiteral("minor"));
        QCOMPARE(childByTag(ln, "a:schemeClr").attribute(QStringLiteral("val")), QStringLiteral("accent4"));
        QCOMPARE(childByTag(childByTag(ln, "a:schemeClr"), "a:shade").attribute(QStringLiteral("val")), QStringLiteral("50000"));
        QCOMPARE(childByTag(fl, "a:schemeClr").attribute(QStringLiteral("val")), QStringLiteral("accent4"));
        QCOMPARE(childByTag(font, "a:schemeClr").attribute(QStringLiteral("val")), QStringLiteral("lt1"));
        // It comes back the same: the look, and the reference.
        std::unique_ptr<Document> back = r.viaPptx();
        QVERIFY(back);
        Item *again = Rig::named(*back, QStringLiteral("Styled"));
        QVERIFY(again);
        QCOMPARE(again->styleRef, r.item(id)->styleRef);
        QCOMPARE(again->fill.toJson(), fill.toJson());
        QCOMPARE(again->stroke.toJson(), stroke.toJson());
        QCOMPARE(again->fx.toJson(), fx);
        // A fill chosen after is written beside the reference; the line and effects are still the style's.
        r.w.formatPane()->session()->apply(QStringLiteral("Shape Fill"), [](Item &it) { formatops::setSolid(it.fill, ColorRef::rgb(QColor(10, 200, 30))); });
        QVERIFY(unpack(deck::exportPresentation(*r.ed()->doc(), &err), &e, &err));
        const QDomElement sp2 = shapeNamed(part(e, QStringLiteral("/ppt/slides/slide1.xml")), QStringLiteral("Styled"), &dom);
        const QDomElement spPr2 = childByTag(sp2, "p:spPr");
        QVERIFY(!childByTag(sp2, "p:style").isNull());
        QCOMPARE(childByTag(childByTag(spPr2, "a:solidFill"), "a:srgbClr").attribute(QStringLiteral("val")), QStringLiteral("0AC81E"));
        QVERIFY(childByTag(spPr2, "a:ln").isNull() && childByTag(spPr2, "a:effectLst").isNull());
        std::unique_ptr<Document> changed = r.viaPptx();
        Item *edited = Rig::named(*changed, QStringLiteral("Styled"));
        QVERIFY(edited);
        QCOMPARE(edited->fill.color.resolve(changed->colors), QColor(10, 200, 30));
        QCOMPARE(edited->stroke.toJson(), stroke.toJson());
        QCOMPARE(edited->styleRef, r.item(id)->styleRef);
    }

    // A shape of a file that refers to a style keeps the reference through opening and saving, and what it gives is the file's theme's.
    void aFilesStyleReferenceIsKeptThroughSaving()
    {
        Parts p = samplePptx();
        const QString shape = QStringLiteral(
            "<p:sp><p:nvSpPr><p:cNvPr id=\"20\" name=\"From a file\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr>") + xfrm(914400, 914400, 1828800, 914400)
            + QStringLiteral("<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></p:spPr><p:style><a:lnRef idx=\"2\"><a:schemeClr val=\"accent2\"><a:shade val=\"50000\"/></a:schemeClr></a:lnRef>"
                             "<a:fillRef idx=\"1\"><a:schemeClr val=\"accent2\"/></a:fillRef><a:effectRef idx=\"0\"><a:schemeClr val=\"accent2\"/></a:effectRef>"
                             "<a:fontRef idx=\"minor\"><a:schemeClr val=\"lt1\"/></a:fontRef></p:style><p:txBody><a:bodyPr/><a:lstStyle/>")
            + para(QStringLiteral("White words")) + QStringLiteral("</p:txBody></p:sp>");
        QByteArray &slide = p.data["/ppt/slides/slide1.xml"].xml;
        QString xml = QString::fromUtf8(slide);
        xml.replace(QStringLiteral("</p:spTree>"), shape + QStringLiteral("</p:spTree>"));
        slide = xml.toUtf8();
        QString err;
        std::unique_ptr<Document> d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        Item *it = Rig::named(*d, QStringLiteral("From a file"));
        QVERIFY(it);
        // The theme of the sample has flat fills, a 0.75 point line, and accent 2 of 13773A.
        QVERIFY(it->styleRef.on);
        QCOMPARE(it->styleRef.line.idx, 2);
        QCOMPARE(it->styleRef.fill.color.name, QStringLiteral("accent2"));
        QCOMPARE(it->styleRef.font, QStringLiteral("minor"));
        QCOMPARE(it->styleRef.fontColor.name, QStringLiteral("lt1"));
        QCOMPARE(it->fill.color.resolve(d->colors), QColor(0x13, 0x77, 0x3A));
        QCOMPARE(it->stroke.width, 0.75);
        // Saved again, the style is written and the shape's fill, line, and effects are the style's alone.
        Exported e;
        QVERIFY(unpack(deck::exportPresentation(*d, &err), &e, &err));
        QDomDocument dom;
        const QDomElement sp = shapeNamed(part(e, QStringLiteral("/ppt/slides/slide1.xml")), QStringLiteral("From a file"), &dom);
        QVERIFY(!sp.isNull());
        QVERIFY(!childByTag(sp, "p:style").isNull());
        const QDomElement spPr = childByTag(sp, "p:spPr");
        for (const char *tag : {"a:solidFill", "a:noFill", "a:ln", "a:effectLst"}) QVERIFY2(childByTag(spPr, tag).isNull(), tag);
        QCOMPARE(childByTag(childByTag(sp, "p:style"), "a:lnRef").attribute(QStringLiteral("idx")), QStringLiteral("2"));
        std::unique_ptr<Document> back = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
        QVERIFY(back);
        Item *again = Rig::named(*back, QStringLiteral("From a file"));
        QVERIFY(again);
        QCOMPARE(again->fill.toJson(), it->fill.toJson());
        QCOMPARE(again->stroke.toJson(), it->stroke.toJson());
        QCOMPARE(again->styleRef, it->styleRef);
    }

    // The .odp keeps the look (the fill, line, and shadow of the style) and, in an attribute of ours, the reference.
    void styleComesBackFromAnOdp()
    {
        Rig r;
        const QString id = r.addShape(QStringLiteral("Styled"), QStringLiteral("Words"), QRectF(100, 100, 220, 110))->id;
        r.ed()->select(id);
        QVERIFY(r.w.applyShapeStyle(QStringLiteral("moderate:accent3")));
        std::unique_ptr<Document> back = r.viaOdp();
        QVERIFY(back);
        Item *again = Rig::named(*back, QStringLiteral("Styled"));
        QVERIFY(again);
        QCOMPARE(again->styleRef, r.item(id)->styleRef);
        const Item *was = r.item(id);
        QCOMPARE(again->fill.type, Fill::Gradient);
        QVERIFY(apart(again->fill.stops.first().color.resolve(back->colors), was->fill.stops.first().color.resolve(r.ed()->doc()->colors)) <= 2);
        QVERIFY(apart(again->fill.stops.last().color.resolve(back->colors), was->fill.stops.last().color.resolve(r.ed()->doc()->colors)) <= 2);
        QVERIFY(near(again->stroke.width, was->stroke.width, 0.02));
        QVERIFY(apart(again->stroke.color.resolve(back->colors), was->stroke.color.resolve(r.ed()->doc()->colors)) <= 2);
        QVERIFY(again->fx.shadow.on);
        QVERIFY(near(again->fx.shadow.distance, was->fx.shadow.distance, 0.1));
    }

    // A style reference in an .odp that is not what we write (names that are not names, any number of changes, an index past the theme's) is held to
    // what can be kept: the file saved after it is sound.
    void hostileStyleReferenceInAnOdpIsHeld()
    {
        Rig r;
        const QString id = r.addShape(QStringLiteral("Styled"), QStringLiteral("Words"))->id;
        r.ed()->select(id);
        QVERIFY(r.w.applyShapeStyle(QStringLiteral("moderate:accent1")));
        QString err;
        QMap<QString, QByteArray> zip;
        QVERIFY(readZip(deck::exportOdp(*r.ed()->doc(), false, &err), zip, &err));
        QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        const int from = content.indexOf(QStringLiteral("jo:style-ref=\"")) + QStringLiteral("jo:style-ref=\"").size(), to = content.indexOf(QLatin1Char('"'), from);
        QVERIFY(from > 20 && to > from);
        QJsonArray mods;
        for (int i = 0; i < 5000; ++i) mods.append(QJsonArray{i % 2 ? QStringLiteral("shade") : QStringLiteral("a b><evil/"), 1000000000});
        QJsonObject part{{"idx", 99999999999.0}, {"name", QStringLiteral("accent\"><x/>")}, {"mods", mods}};
        QJsonObject o{{"ln", part}, {"fill", part}, {"fx", part}, {"font", QStringLiteral("minor\"><x/>")}, {"fontColor", QJsonObject{{"name", QStringLiteral("lt1")}, {"mods", mods}}}};
        QString json = QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact));
        json.replace(QLatin1Char('"'), QStringLiteral("&quot;")).replace(QLatin1Char('<'), QStringLiteral("&lt;")).replace(QLatin1Char('>'), QStringLiteral("&gt;"));
        content.replace(from, to - from, json);
        ZipWriter w;
        w.add(QStringLiteral("mimetype"), zip.value(QStringLiteral("mimetype")));
        for (auto it = zip.begin(); it != zip.end(); ++it)
            if (it.key() != QLatin1String("mimetype") && it.key() != QLatin1String("content.xml")) w.add(it.key(), it.value(), true);
        w.add(QStringLiteral("content.xml"), content.toUtf8(), true);
        std::unique_ptr<Document> d = deck::importOdp(w.finish(), &err);
        QVERIFY2(d, qPrintable(err));
        Item *it = Rig::named(*d, QStringLiteral("Styled"));
        QVERIFY(it);
        QVERIFY(it->styleRef.on);
        QVERIFY(it->styleRef.fill.idx <= 1000000);
        QVERIFY(it->styleRef.fill.color.mods.size() <= 16);
        QVERIFY(it->styleRef.font.isEmpty());
        for (const auto &m : it->styleRef.fill.color.mods) QVERIFY(m.first == QLatin1String("shade"));
        // Saved as a .pptx, it is a sound package, and reads again.
        Exported e;
        QVERIFY(unpack(deck::exportPresentation(*d, &err), &e, &err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QVERIFY(deck::importPresentation(deck::exportPresentation(*d, &err), &err));
    }


    // ---- Change Shape

    // Another geometry from the library goes onto a shape and nothing else of it changes: its words, fill, line, shadow, place, size, turn, and
    // flip stay, the handles' values and any edited points go, and it is one step that undoes.
    void changeShapeSwapsTheGeometryAndKeepsTheRest()
    {
        Rig r;
        ShapeItem *s = r.addShape(QStringLiteral("Swap"), QStringLiteral("Words in the shape"));
        s->shape = QStringLiteral("roundRect");
        s->adj = {0.4};
        const QString story = s->storyId, id = s->id;
        r.ed()->select(id);
        const QRectF rect = s->rect;
        const int base = r.steps();
        QVERIFY(r.w.changeShape(QStringLiteral("rightArrow")));
        QCOMPARE(r.steps(), base + 1);
        s = r.shape(id);
        QCOMPARE(s->shape, QStringLiteral("rightArrow"));
        QVERIFY(s->adj.isEmpty());
        QCOMPARE(s->storyId, story);
        QCOMPARE(r.ed()->doc()->storyDoc(story)->toPlainText(), QStringLiteral("Words in the shape"));
        QCOMPARE(s->rect, rect);
        QCOMPARE(s->rotation, 20.0);
        QVERIFY(s->flipH);
        QCOMPARE(s->fill.color, ColorRef::rgb(QColor(200, 30, 30)));
        QCOMPARE(s->fill.transparency, 0.25);
        QCOMPARE(s->stroke.color, ColorRef::rgb(QColor(20, 20, 160)));
        QCOMPARE(s->stroke.width, 3.0);
        QVERIFY(s->fx.shadow.on);
        QCOMPARE(s->fx.shadow.distance, 7.0);
        // Undo and redo go from one geometry to the other.
        r.ed()->undo();
        QCOMPARE(r.shape(id)->shape, QStringLiteral("roundRect"));
        QCOMPARE(r.shape(id)->adj, QVector<double>{0.4});
        r.ed()->redo();
        QCOMPARE(r.shape(id)->shape, QStringLiteral("rightArrow"));
        // Edited points are replaced by the geometry chosen.
        r.shape(id)->customPath = QPainterPath();
        r.shape(id)->customPath.addEllipse(QRectF(0, 0, 50, 50));
        r.ed()->select(id);
        QVERIFY(r.w.changeShape(QStringLiteral("star5")));
        QVERIFY(r.shape(id)->customPath.isEmpty());
        QCOMPARE(r.shape(id)->shape, QStringLiteral("star5"));
    }

    // It is for shapes: a text box, a line, and a picture are left alone (the command is off for them), and a shape the library doesn't have
    // changes nothing. Every shape selected takes the geometry, in one step.
    void changeShapeIsForShapesAndTakesEveryOneSelected()
    {
        Rig r;
        const QString a = r.addShape(QStringLiteral("A"), QStringLiteral("A words"), QRectF(40, 40, 120, 80))->id;
        const QString b = r.addShape(QStringLiteral("B"), QString(), QRectF(200, 40, 120, 80))->id;
        auto box = std::static_pointer_cast<TextItem>(r.ed()->newTextBox(QRectF(40, 200, 200, 60), QStringLiteral("Text box")));
        r.ed()->addItem(box, false);
        auto *gallery = r.widget<GalleryButton>("changeShape");
        QVERIFY(gallery);
        r.ed()->select(box->id);
        QVERIFY(!gallery->isEnabled());
        const int base = r.steps();
        QVERIFY(!r.w.changeShape(QStringLiteral("star5")));
        QCOMPARE(r.steps(), base);
        QCOMPARE(r.ed()->doc()->item(box->id)->type(), ItemType::Text);
        r.ed()->select(QStringList{a, b, box->id});
        QVERIFY(gallery->isEnabled());
        QVERIFY(!r.w.changeShape(QStringLiteral("no such shape")));
        QCOMPARE(r.steps(), base);
        QVERIFY(r.w.changeShape(QStringLiteral("diamond")));
        QCOMPARE(r.steps(), base + 1);
        QCOMPARE(r.shape(a)->shape, QStringLiteral("diamond"));
        QCOMPARE(r.shape(b)->shape, QStringLiteral("diamond"));
        QCOMPARE(r.ed()->doc()->item(box->id)->type(), ItemType::Text);
        // The same shape again changes nothing, so it takes no step.
        QVERIFY(r.w.changeShape(QStringLiteral("diamond")));
        QCOMPARE(r.steps(), base + 1);
        r.ed()->undo();
        QCOMPARE(r.shape(a)->shape, QStringLiteral("rect"));
        QCOMPARE(r.shape(b)->shape, QStringLiteral("rect"));
    }

    // The gallery on the tab offers every shape of Insert > Shapes (and not its line tools), each by the name it has there, and a choice in it
    // changes the selected shapes.
    void changeShapeGalleryOffersTheShapesOfInsertShapes()
    {
        Rig r;
        const QString id = r.addShape(QStringLiteral("Gallery"))->id;
        r.ed()->select(id);
        auto *gallery = r.widget<GalleryButton>("changeShape");
        QVERIFY(gallery);
        const QVector<GalleryItem> items = gallery->items();
        QCOMPARE(items.size(), int(shapeLibrary().size()));
        QStringList seen;
        for (const GalleryItem &i : items) {
            const ShapeDef *def = shapeDef(i.id);
            QVERIFY2(def, qPrintable(i.id));
            QCOMPARE(i.tip, def->name);
            QCOMPARE(i.group, def->category);
            QVERIFY(!i.icon.isNull());
            seen << i.id;
        }
        QVERIFY(!seen.contains(QStringLiteral("tool:line")));
        const int base = r.steps();
        Q_EMIT gallery->activated(QStringLiteral("rightArrow"));
        QCOMPARE(r.shape(id)->shape, QStringLiteral("rightArrow"));
        QCOMPARE(r.steps(), base + 1);
    }

    // Any shape of the library, put on a shape with text, a fill, and a line, comes back from a .pptx and an .odp with its words and look: as
    // the geometry the shape has when the file can name it, else as the path it draws (the same outline).
    void changedShapeComesBackFromBothFiles()
    {
        Rig r;
        const QString id = r.addShape(QStringLiteral("Round trip"), QStringLiteral("Kept words"), QRectF(100, 100, 160, 120))->id;
        r.ed()->select(id);
        int tried = 0;
        for (const ShapeDef &def : shapeLibrary()) {
            QVERIFY2(r.w.changeShape(def.id), qPrintable(def.id));
            QCOMPARE(r.shape(id)->shape, def.id);
            for (int how = 0; how < 2; ++how) {
                std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
                QVERIFY2(d, qPrintable(def.id));
                Item *found = Rig::named(*d, QStringLiteral("Round trip"));
                QVERIFY2(found, qPrintable(def.id));
                const QString which = def.id + (how ? QStringLiteral(" (odp)") : QStringLiteral(" (pptx)"));
                // (A plain rectangle with words in it is what a file calls a text box, and comes back as one; a shape of several
                // faces, such as an arc, comes back as a group of them with the words on the first.)
                const Item *face = found;
                if (found->type() == ItemType::Group) face = static_cast<GroupItem *>(found)->children.front().get();
                QString storyId, shapeName;
                if (face->type() == ItemType::Text) {
                    QVERIFY2(def.id == QLatin1String("rect"), qPrintable(which));
                    storyId = static_cast<const TextItem *>(face)->storyId;
                } else {
                    QVERIFY2(face->type() == ItemType::Shape, qPrintable(which));
                    storyId = static_cast<const ShapeItem *>(face)->storyId;
                    shapeName = static_cast<const ShapeItem *>(face)->shape;
                }
                QVERIFY2(d->storyDoc(storyId), qPrintable(which + QStringLiteral(": no text")));
                QCOMPARE(d->storyDoc(storyId)->toPlainText(), QStringLiteral("Kept words"));
                QVERIFY2(face->fill.isNone() || face->fill.color.resolve(d->colors) == QColor(200, 30, 30), qPrintable(which));
                QVERIFY2(face->stroke.isNone() || near(face->stroke.width, 3.0, 0.02), qPrintable(which));
                // A shape the file names comes back as that shape.
                if (how == 0 && !def.open && !shapeName.isEmpty() && !office::presetForShape(def.id).isEmpty() && office::shapeForPreset(office::presetForShape(def.id)) == def.id)
                    QVERIFY2(shapeName == def.id, qPrintable(which + QStringLiteral(" came back as ") + shapeName));
            }
            ++tried;
        }
        QVERIFY(tried > 40);
    }
};

JP_TEST_CLASS(DeckStylesTests)
#include "tst_deck_styles.moc"
