// JeffDeck's tests of the Shape Format tab's styles: the Quick Styles gallery drawn from the theme (and how a style is saved), Change Shape, and
// the Text Art styles of the text in a shape. Each command is one step that undoes, and comes back from a .pptx and from an .odp.
#include "deckhelpers.h"

#include "app/formatops.h"
#include "core/shapestyles.h"
#include "core/themes.h"
#include "render/shapes.h"

#include <QMenu>
#include <QToolButton>
#include <QWidgetAction>

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

    // The drop-down menu of a command of the tab, by its action's name, and the choices in it (made again as it opens).
    QMenu *menuOf(const char *id) const
    {
        for (QToolButton *b : w.ribbon()->findChildren<QToolButton *>())
            if (b->defaultAction() && b->defaultAction()->objectName() == QLatin1String(id) && b->menu()) return b->menu();
        qWarning("no menu %s", id);
        return nullptr;
    }
    static QAction *choose(QMenu *m, const QString &text)
    {
        Q_EMIT m->aboutToShow();
        for (QAction *a : m->actions())
            if (a->text() == text) return a;
        return nullptr;
    }
    static QAction *sub(QMenu *m, const QString &title, const QString &text)
    {
        Q_EMIT m->aboutToShow();
        for (QAction *a : m->actions())
            if (a->menu() && a->text() == title)
                for (QAction *b : a->menu()->actions())
                    if (b->text() == text) return b;
        return nullptr;
    }
    // A color of the menu's grid, by the name it has there ("Accent 2").
    static QToolButton *swatch(QMenu *m, const QString &tip)
    {
        Q_EMIT m->aboutToShow();
        for (QAction *a : m->actions())
            if (auto *wa = qobject_cast<QWidgetAction *>(a))
                for (auto *b : wa->defaultWidget()->findChildren<QToolButton *>(QStringLiteral("swatch")))
                    if (b->toolTip() == tip) return b;
        return nullptr;
    }
    // The format of the letter at a place in a story.
    QTextCharFormat letter(const QString &storyId, int at) const
    {
        QTextCursor c(ed()->doc()->storyDoc(storyId));
        c.setPosition(at + 1);
        return c.charFormat();
    }
    static TextEffects fxOf(const QTextCharFormat &cf) { return TextEffects::fromJson(cf.stringProperty(tp::TextFx)); }

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

// A shape of a slide, with its style and what its properties say, as a file writes it.
QString styledShape(int id, const QString &name, const QString &properties, const QString &style, const QString &words = QStringLiteral("Words"))
{
    return QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"%2\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr><p:spPr>").arg(id).arg(name) + xfrm(914400 + id * 100000, 914400, 1828800, 914400)
        + QStringLiteral("<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>") + properties + QStringLiteral("</p:spPr>") + style
        + QStringLiteral("<p:txBody><a:bodyPr/><a:lstStyle/>") + para(words) + QStringLiteral("</p:txBody></p:sp>");
}

// A p:style: the line, fill, and effect style numbers, in one color each, and a text color.
QString styleOf(int line, int fill, int effect, const QString &color, const QString &text)
{
    return QStringLiteral("<p:style><a:lnRef idx=\"%1\">%5</a:lnRef><a:fillRef idx=\"%2\">%5</a:fillRef><a:effectRef idx=\"%3\">%5</a:effectRef>"
                          "<a:fontRef idx=\"minor\">%4</a:fontRef></p:style>")
        .arg(line).arg(fill).arg(effect).arg(text, color);
}

// The sample presentation's theme with a shadow for its second effect style.
QByteArray themeWithShadow(const QByteArray &theme)
{
    QString xml = QString::fromUtf8(theme);
    const QString none = QStringLiteral("<a:effectStyle><a:effectLst/></a:effectStyle>");
    xml.replace(none + none, none + QStringLiteral("<a:effectStyle><a:effectLst><a:outerShdw blurRad=\"50800\" dist=\"38100\" dir=\"5400000\" algn=\"ctr\" rotWithShape=\"0\">"
                                                   "<a:srgbClr val=\"000000\"><a:alpha val=\"40000\"/></a:srgbClr></a:outerShdw></a:effectLst></a:effectStyle>"));
    return xml.toUtf8();
}

// What is wrong with the colors of a p:style, if anything: a theme color must be named as the standard names them, an RGB one must be six hex
// digits, and a change must be one the standard has, with a number.
QString styleProblem(const QDomElement &style)
{
    static const QStringList schemes = {"dk1", "lt1", "dk2", "lt2", "accent1", "accent2", "accent3", "accent4", "accent5", "accent6", "hlink", "folHlink", "tx1", "tx2", "bg1", "bg2"};
    static const QStringList changes = {"alpha", "alphaMod", "alphaOff", "hue", "hueMod", "hueOff", "sat", "satMod", "satOff", "lum", "lumMod", "lumOff", "red", "redMod", "redOff",
                                        "green", "greenMod", "greenOff", "blue", "blueMod", "blueOff", "shade", "tint"};
    static const QRegularExpression hex(QStringLiteral("^[0-9A-Fa-f]{6}$")), number(QStringLiteral("^-?[0-9]{1,9}$"));
    for (QDomElement ref = style.firstChildElement(); !ref.isNull(); ref = ref.nextSiblingElement()) {
        const QString local = ref.localName();
        if (local == QLatin1String("fontRef")) {
            if (!QStringList{"major", "minor", "none"}.contains(ref.attribute(QStringLiteral("idx")))) return QStringLiteral("fontRef idx ") + ref.attribute(QStringLiteral("idx"));
        } else if (!number.match(ref.attribute(QStringLiteral("idx"))).hasMatch()) {
            return local + QStringLiteral(" idx ") + ref.attribute(QStringLiteral("idx"));
        }
        for (QDomElement c = ref.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
            const QString kind = c.localName(), val = c.attribute(QStringLiteral("val"));
            if (kind == QLatin1String("schemeClr") ? !schemes.contains(val) : kind == QLatin1String("srgbClr") ? !hex.match(val).hasMatch() : true) return kind + QStringLiteral(" val ") + val;
            for (QDomElement m = c.firstChildElement(); !m.isNull(); m = m.nextSiblingElement())
                if (!changes.contains(m.localName()) || !number.match(m.attribute(QStringLiteral("val"))).hasMatch()) return QStringLiteral("change ") + m.localName() + QLatin1Char(' ') + m.attribute(QStringLiteral("val"));
        }
    }
    return QString();
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
        // Drawn by the renderer in the theme's colors: the inside of a moderate style is its accent (a gradient from a lighter to a darker
        // shade of it), and of an outline alone, the slide (the line is at the edge).
        for (int accent = 0; accent < 6; ++accent) {
            const QImage moderate = items[12 + accent].icon.pixmap(QSize(64, 48)).toImage();
            const QColor middle = moderate.pixelColor(12, moderate.height() / 2);   // (beside the word in the tile, whatever font draws it)
            QVERIFY2(apart(middle, theme.colors[4 + accent]) < 70, qPrintable(QStringLiteral("accent %1: %2 not near %3").arg(accent + 1).arg(middle.name(), theme.colors[4 + accent].name())));
            const QImage outline = items[accent].icon.pixmap(QSize(64, 48)).toImage();
            QVERIFY(apart(outline.pixelColor(12, outline.height() / 2), theme.colors[4 + accent]) > 40);   // (its inside is not filled)
        }
        QVERIFY(apart(items[12].icon.pixmap(QSize(64, 48)).toImage().pixelColor(12, 24), items[13].icon.pixmap(QSize(64, 48)).toImage().pixelColor(12, 24)) > 20);
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
        QJsonObject entry{{"idx", 99999999999.0}, {"name", QStringLiteral("accent\"><x/>")}, {"mods", mods}};
        QJsonObject o{{"ln", entry}, {"fill", entry}, {"fx", entry}, {"font", QStringLiteral("minor\"><x/>")}, {"fontColor", QJsonObject{{"name", QStringLiteral("lt1")}, {"mods", mods}}}};
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
        // Saved as a .pptx, it is a sound package, and reads again; and what is written, not what our own reader puts up with, is what the standard allows.
        Exported e;
        QVERIFY(unpack(deck::exportPresentation(*d, &err), &e, &err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        QVERIFY(deck::importPresentation(deck::exportPresentation(*d, &err), &err));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(!slide.contains(QStringLiteral("evil")) && !slide.contains(QStringLiteral("<x/>")));
        QDomDocument dom;
        const QDomElement sp = shapeNamed(slide, QStringLiteral("Styled"), &dom);
        QVERIFY(!sp.isNull());
        QVERIFY2(styleProblem(childByTag(sp, "p:style")).isEmpty(), qPrintable(styleProblem(childByTag(sp, "p:style"))));
    }


    // ---- Text Art Styles

    // Text Fill, Text Outline, and Text Effects change all the text of the shapes selected (each choice one step): a color, a gradient, or none for
    // the letters; a color, a weight, or none for their outline; a shadow, reflection, or glow.
    void textMenusChangeAllTheTextOfTheShapesSelected()
    {
        Rig r;
        const QString a = r.addShape(QStringLiteral("A"), QStringLiteral("Hello world"), QRectF(40, 40, 200, 80))->id;
        const QString b = r.addShape(QStringLiteral("B"), QStringLiteral("Second shape"), QRectF(300, 40, 200, 80))->id;
        r.ed()->select(QStringList{a, b});
        const QString sa = r.shape(a)->storyId, sb = r.shape(b)->storyId;
        QMenu *fill = r.menuOf("ribbon.textFill"), *outline = r.menuOf("ribbon.textOutline"), *effects = r.menuOf("ribbon.textEffects");
        QVERIFY(fill && outline && effects);
        int steps = r.steps();
        auto oneStep = [&](QAction *action, const char *what) {
            QVERIFY2(action, what);
            action->trigger();
            QVERIFY2(r.steps() == steps + 1, what);
            ++steps;
        };
        // Fill: a color of the grid, in every letter of both.
        QToolButton *accent2 = Rig::swatch(fill, QStringLiteral("Accent 2"));
        QVERIFY(accent2);
        accent2->click();
        QCOMPARE(r.steps(), ++steps);
        for (const QString &story : {sa, sb})
            for (int at : {0, 4, 9}) QCOMPARE(r.letter(story, at).stringProperty(tp::ColorRefP), ColorRef::scheme(Accent2).toString());
        // A gradient is in the letters, and its first color is their color.
        oneStep(Rig::sub(fill, QStringLiteral("Gradient"), QStringLiteral("Linear Right")), "gradient");
        {
            const QTextCharFormat cf = r.letter(sa, 3);
            const Fill g = Fill::fromJson(QJsonDocument::fromJson(cf.stringProperty(tp::TextFill).toUtf8()).object());
            QCOMPARE(g.type, Fill::Gradient);
            QCOMPARE(g.angle, 0.0);
            QCOMPARE(g.stops.size(), 2);
            QCOMPARE(cf.stringProperty(tp::ColorRefP), g.stops.first().color.toString());
        }
        // A color after it replaces the gradient; none is clear letters.
        Rig::swatch(fill, QStringLiteral("Accent 3"))->click();
        ++steps;
        QVERIFY(r.letter(sa, 3).stringProperty(tp::TextFill).isEmpty());
        QCOMPARE(r.steps(), steps);
        oneStep(Rig::choose(fill, QStringLiteral("No Fill")), "no fill");
        QCOMPARE(r.letter(sb, 2).foreground().color().alpha(), 0);
        QCOMPARE(ColorRef::fromString(r.letter(sb, 2).stringProperty(tp::ColorRefP)).resolve(r.ed()->doc()->colors).alpha(), 0);
        // Outline: a color (a point wide to start with), a weight, none.
        QVERIFY(r.letter(sa, 1).stringProperty(tp::OutlineRef).isEmpty());
        Rig::swatch(outline, QStringLiteral("Accent 4"))->click();
        ++steps;
        QCOMPARE(r.steps(), steps);
        for (const QString &story : {sa, sb}) {
            QCOMPARE(r.letter(story, 1).stringProperty(tp::OutlineRef), ColorRef::scheme(Accent4).toString());
            QCOMPARE(r.letter(story, 1).property(tp::OutlineWidth).toDouble(), 1.0);
        }
        oneStep(Rig::sub(outline, QStringLiteral("Weight"), QStringLiteral("3 pt")), "weight");
        QCOMPARE(r.letter(sb, 5).property(tp::OutlineWidth).toDouble(), 3.0);
        oneStep(Rig::choose(outline, QStringLiteral("No Outline")), "no outline");
        QVERIFY(r.letter(sa, 1).stringProperty(tp::OutlineRef).isEmpty());
        // Effects: a shadow, a reflection, a glow, each beside the others; and a shadow off leaves the rest.
        oneStep(Rig::sub(effects, QStringLiteral("Shadow"), QStringLiteral("Offset Bottom Right")), "shadow");
        {
            const TextEffects fx = Rig::fxOf(r.letter(sa, 2));
            QVERIFY(fx.outer.on);
            QCOMPARE(fx.outer.distance, 3.0);
            QCOMPARE(fx.outer.angle, 45.0);
            QVERIFY(fx.outer.color.alpha() > 0 && fx.outer.color.alpha() < 255);
        }
        oneStep(Rig::sub(effects, QStringLiteral("Reflection"), QStringLiteral("Half Reflection")), "reflection");
        oneStep(Rig::sub(effects, QStringLiteral("Glow"), QStringLiteral("10 Point Glow")), "glow");
        {
            const TextEffects fx = Rig::fxOf(r.letter(sb, 4));
            QVERIFY(fx.outer.on && fx.reflection.on && fx.glow.on);
            QCOMPARE(fx.glow.radius, 10.0);
            QCOMPARE(fx.reflection.sy, -1.0);
            QVERIFY(near(fx.reflection.endPos, 0.55, 0.001));
            QVERIFY(fx.reflection.startAlpha > fx.reflection.endAlpha);
        }
        oneStep(Rig::sub(effects, QStringLiteral("Shadow"), QStringLiteral("No Shadow")), "no shadow");
        {
            const TextEffects fx = Rig::fxOf(r.letter(sa, 2));
            QVERIFY(!fx.outer.on && fx.reflection.on && fx.glow.on);
        }
        oneStep(Rig::sub(effects, QStringLiteral("Glow"), QStringLiteral("No Glow")), "no glow");
        oneStep(Rig::sub(effects, QStringLiteral("Reflection"), QStringLiteral("No Reflection")), "no reflection");
        QVERIFY(!Rig::fxOf(r.letter(sa, 2)).any());
        // The words are the words they were, and each change undoes.
        QCOMPARE(r.ed()->doc()->storyDoc(sa)->toPlainText(), QStringLiteral("Hello world"));
        r.ed()->undo();
        QVERIFY(Rig::fxOf(r.letter(sa, 2)).reflection.on);
        r.ed()->undo();
        QVERIFY(Rig::fxOf(r.letter(sa, 2)).glow.on);
    }

    // While the words are being edited, the menus change the words selected and nothing else.
    void textMenusChangeTheSelectedWordsWhileEditing()
    {
        Rig r;
        const QString a = r.addShape(QStringLiteral("A"), QStringLiteral("Hello world today"))->id;
        r.ed()->select(a);
        r.ed()->beginTextEdit(a, 6);
        QTextCursor c = r.ed()->cursor();
        c.setPosition(6);
        c.setPosition(11, QTextCursor::KeepAnchor);
        r.ed()->setCursor(c);
        const QString story = r.shape(a)->storyId;
        const int base = r.steps();
        Rig::swatch(r.menuOf("ribbon.textFill"), QStringLiteral("Accent 1"))->click();
        Rig::sub(r.menuOf("ribbon.textEffects"), QStringLiteral("Glow"), QStringLiteral("5 Point Glow"))->trigger();
        QCOMPARE(r.steps(), base + 2);
        QVERIFY(r.letter(story, 2).stringProperty(tp::ColorRefP).isEmpty());   // "Hello "
        QCOMPARE(r.letter(story, 7).stringProperty(tp::ColorRefP), ColorRef::scheme(Accent1).toString());   // "world"
        QVERIFY(Rig::fxOf(r.letter(story, 8)).glow.on);
        QVERIFY(!Rig::fxOf(r.letter(story, 3)).any());
        QVERIFY(!Rig::fxOf(r.letter(story, 13)).any());   // " today"
        QVERIFY(r.letter(story, 13).stringProperty(tp::ColorRefP).isEmpty());
        r.ed()->endTextEdit();
    }

    // Text Art is letters in itself: the menus change its fill, outline, and effects, and Transform its warp; for the words of a shape, Transform is
    // off (those are not warped).
    void textMenusChangeTextArtAndTransformIsForIt()
    {
        Rig r;
        auto art = std::make_shared<TextArtItem>();
        art->text = QStringLiteral("Art");
        art->name = QStringLiteral("Art");
        art->rect = QRectF(300, 200, 240, 60);
        r.ed()->addItem(art, false);
        const QString shape = r.addShape(QStringLiteral("Shape"), QStringLiteral("Words"))->id;
        const QString id = art->id;
        r.ed()->select(id);
        QMenu *fill = r.menuOf("ribbon.textFill"), *outline = r.menuOf("ribbon.textOutline"), *effects = r.menuOf("ribbon.textEffects");
        const int base = r.steps();
        Rig::swatch(fill, QStringLiteral("Accent 3"))->click();
        QCOMPARE(r.steps(), base + 1);
        QCOMPARE(r.item(id)->fill.color, ColorRef::scheme(Accent3));
        Rig::swatch(outline, QStringLiteral("Accent 5"))->click();
        QCOMPARE(r.item(id)->stroke.color, ColorRef::scheme(Accent5));
        Rig::sub(outline, QStringLiteral("Weight"), QStringLiteral("4.5 pt"))->trigger();
        QCOMPARE(r.item(id)->stroke.width, 4.5);
        Rig::sub(effects, QStringLiteral("Shadow"), QStringLiteral("Offset Top Left"))->trigger();
        QVERIFY(r.item(id)->fx.shadow.on);
        QCOMPARE(r.item(id)->fx.shadow.angle, 225.0);
        QAction *archUp = Rig::sub(effects, QStringLiteral("Transform"), QStringLiteral("Arch Up"));
        QVERIFY(archUp && archUp->isEnabled());
        const int before = r.steps();
        archUp->trigger();
        QCOMPARE(r.steps(), before + 1);
        QCOMPARE(static_cast<TextArtItem *>(r.item(id))->transform_, QStringLiteral("archUp"));
        Rig::sub(effects, QStringLiteral("Transform"), QStringLiteral("Plain Text"))->trigger();
        QCOMPARE(static_cast<TextArtItem *>(r.item(id))->transform_, QStringLiteral("plain"));
        Rig::choose(outline, QStringLiteral("No Outline"))->trigger();
        QVERIFY(r.item(id)->stroke.isNone());
        QCOMPARE(static_cast<TextArtItem *>(r.item(id))->text, QStringLiteral("Art"));
        // The words of a shape are not warped: with only that selected, Transform is off.
        r.ed()->select(shape);
        QAction *off = Rig::sub(effects, QStringLiteral("Transform"), QStringLiteral("Arch Up"));
        QVERIFY(off && !off->isEnabled());
        // With both selected the Text Art takes it and the shape's words are as they were.
        r.ed()->select(QStringList{id, shape});
        QAction *both = Rig::sub(effects, QStringLiteral("Transform"), QStringLiteral("Wave 1"));
        QVERIFY(both->isEnabled());
        both->trigger();
        QCOMPARE(static_cast<TextArtItem *>(r.item(id))->transform_, QStringLiteral("waveUp"));
        QVERIFY(Rig::fxOf(r.letter(r.shape(shape)->storyId, 1)).outer.on == false);
    }

    // The gallery is Insert > Text Art's styles: one on the words of a shape gives them the style's fill, outline, and shadow (not its font), and on
    // Text Art the style whole but for its words; each is one step.
    void textArtStyleGalleryGivesTheStylesOfTextArt()
    {
        Rig r;
        const QString a = r.addShape(QStringLiteral("A"), QStringLiteral("Hello world"))->id;
        auto art = std::make_shared<TextArtItem>();
        art->text = QStringLiteral("Art");
        art->rect = QRectF(300, 200, 240, 60);
        r.ed()->addItem(art, false);
        const QString id = art->id;
        auto *gallery = r.widget<Gallery>("textArtStyleGallery");
        QVERIFY(gallery);
        QCOMPARE(r.w.textArtStyleItems().size(), int(textArtStyles().size()));
        for (int i = 0; i < r.w.textArtStyleItems().size(); ++i) QCOMPARE(r.w.textArtStyleItems()[i].id, textArtStyles()[i].id);
        r.ed()->clearSelection();
        QVERIFY(!gallery->isEnabled());
        r.ed()->select(a);
        QVERIFY(gallery->isEnabled());
        const QString story = r.shape(a)->storyId;
        const QString font = r.letter(story, 2).stringProperty(QTextFormat::FontFamily);
        const int base = r.steps();
        QVERIFY(r.w.applyTextStyle(QStringLiteral("grad-gold")));
        QCOMPARE(r.steps(), base + 1);
        {
            const QTextCharFormat cf = r.letter(story, 2);
            const Fill g = Fill::fromJson(QJsonDocument::fromJson(cf.stringProperty(tp::TextFill).toUtf8()).object());
            QCOMPARE(g.type, Fill::Gradient);
            QCOMPARE(g.stops.size(), 2);
            QCOMPARE(g.stops.first().color, ColorRef::rgb(QColor("#F7D774")));
            QCOMPARE(cf.stringProperty(tp::OutlineRef), ColorRef::rgb(QColor("#7A5210")).toString());
            QCOMPARE(cf.property(tp::OutlineWidth).toDouble(), 1.0);
            QVERIFY(Rig::fxOf(cf).outer.on);
            QCOMPARE(cf.stringProperty(QTextFormat::FontFamily), font);
        }
        QVERIFY(r.w.applyTextStyle(QStringLiteral("rainbow")));
        {
            const Fill g = Fill::fromJson(QJsonDocument::fromJson(r.letter(story, 2).stringProperty(tp::TextFill).toUtf8()).object());
            QCOMPARE(g.stops.size(), 6);
            QVERIFY(r.letter(story, 2).stringProperty(tp::OutlineRef).isEmpty());   // (the style has none)
            QVERIFY(!Rig::fxOf(r.letter(story, 2)).outer.on);
        }
        // A solid color style takes the gradient off.
        QVERIFY(r.w.applyTextStyle(QStringLiteral("fill-a1")));
        QVERIFY(r.letter(story, 2).stringProperty(tp::TextFill).isEmpty());
        QCOMPARE(r.letter(story, 2).stringProperty(tp::ColorRefP), ColorRef::scheme(Accent1).toString());
        // Text Art: the whole style but for its words and its place.
        r.ed()->select(id);
        QVERIFY(r.w.applyTextStyle(QStringLiteral("arch-a1")));
        auto *got = static_cast<TextArtItem *>(r.item(id));
        QCOMPARE(got->styleId, QStringLiteral("arch-a1"));
        QCOMPARE(got->transform_, QStringLiteral("archUp"));
        QCOMPARE(got->fill.color, ColorRef::scheme(Accent1));
        QCOMPARE(got->text, QStringLiteral("Art"));
        QCOMPARE(got->rect, QRectF(300, 200, 240, 60));
        QVERIFY(!r.w.applyTextStyle(QStringLiteral("no such style")));
        // Nothing that holds words selected: nothing to do.
        auto line = std::make_shared<LineItem>();
        line->p1 = QPointF(40, 400);
        line->p2 = QPointF(300, 400);
        line->syncRect();
        r.ed()->addItem(line, false);
        r.ed()->select(line->id);
        QVERIFY(!gallery->isEnabled());
        const int idle = r.steps();
        QVERIFY(!r.w.applyTextStyle(QStringLiteral("fill-a1")));
        QCOMPARE(r.steps(), idle);
    }

    // The letters' fill (a color, a gradient, or none), outline, and effects come back from a .pptx and from an .odp.
    void letterStylesComeBackFromBothFiles()
    {
        Rig r;
        const QString id = r.addShape(QStringLiteral("Styled"), QStringLiteral("Lettered words"), QRectF(100, 100, 300, 100))->id;
        r.ed()->select(id);
        const QString story = r.shape(id)->storyId;
        QMenu *fill = r.menuOf("ribbon.textFill"), *outline = r.menuOf("ribbon.textOutline"), *effects = r.menuOf("ribbon.textEffects");
        Rig::sub(fill, QStringLiteral("Gradient"), QStringLiteral("Linear Down"))->trigger();
        Rig::swatch(outline, QStringLiteral("Accent 2"))->click();
        Rig::sub(outline, QStringLiteral("Weight"), QStringLiteral("2.25 pt"))->trigger();
        Rig::sub(effects, QStringLiteral("Shadow"), QStringLiteral("Offset Bottom Left"))->trigger();
        Rig::sub(effects, QStringLiteral("Reflection"), QStringLiteral("Tight Reflection"))->trigger();
        Rig::sub(effects, QStringLiteral("Glow"), QStringLiteral("18 Point Glow"))->trigger();
        const QTextCharFormat want = r.letter(story, 3);
        const Fill wantFill = Fill::fromJson(QJsonDocument::fromJson(want.stringProperty(tp::TextFill).toUtf8()).object());
        QVERIFY(wantFill.type == Fill::Gradient);
        const TextEffects wantFx = Rig::fxOf(want);
        for (int how = 0; how < 2; ++how) {
            const QString which = how ? QStringLiteral("odp") : QStringLiteral("pptx");
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY2(d, qPrintable(which));
            Item *it = Rig::named(*d, QStringLiteral("Styled"));
            QVERIFY2(it, qPrintable(which));
            const QString back = it->type() == ItemType::Shape ? static_cast<ShapeItem *>(it)->storyId : static_cast<TextItem *>(it)->storyId;
            QTextCursor c(d->storyDoc(back));
            c.setPosition(4);
            const QTextCharFormat cf = c.charFormat();
            const Fill g = Fill::fromJson(QJsonDocument::fromJson(cf.stringProperty(tp::TextFill).toUtf8()).object());
            QVERIFY2(g.type == Fill::Gradient && g.stops.size() == wantFill.stops.size(), qPrintable(which + QStringLiteral(": gradient")));
            QVERIFY2(near(g.angle, wantFill.angle, 1), qPrintable(which));
            QVERIFY2(apart(g.stops.first().color.resolve(d->colors), wantFill.stops.first().color.resolve(r.ed()->doc()->colors)) <= 2, qPrintable(which));
            QVERIFY2(apart(g.stops.last().color.resolve(d->colors), wantFill.stops.last().color.resolve(r.ed()->doc()->colors)) <= 2, qPrintable(which));
            QVERIFY2(apart(ColorRef::fromString(cf.stringProperty(tp::OutlineRef)).resolve(d->colors), ColorRef::fromString(want.stringProperty(tp::OutlineRef)).resolve(r.ed()->doc()->colors)) <= 2,
                     qPrintable(which + QStringLiteral(": outline color")));
            QVERIFY2(near(cf.property(tp::OutlineWidth).toDouble(), 2.25, 0.02), qPrintable(which + QStringLiteral(": outline width ") + QString::number(cf.property(tp::OutlineWidth).toDouble())));
            const TextEffects fx = Rig::fxOf(cf);
            QVERIFY2(fx.outer.on && fx.reflection.on && fx.glow.on, qPrintable(which + QStringLiteral(": effects")));
            QVERIFY2(near(fx.outer.distance, wantFx.outer.distance, 0.05) && near(fx.outer.angle, wantFx.outer.angle, 0.5) && near(fx.outer.blur, wantFx.outer.blur, 0.05), qPrintable(which));
            QVERIFY2(near(fx.glow.radius, 18, 0.05), qPrintable(which));
            QVERIFY2(near(fx.reflection.endPos, wantFx.reflection.endPos, 0.01) && near(fx.reflection.sy, -1, 0.001), qPrintable(which));
        }
        // Clear letters (no fill) and an outline alone come back as they were.
        Rig::choose(fill, QStringLiteral("No Fill"))->trigger();
        for (int how = 0; how < 2; ++how) {
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            Item *it = Rig::named(*d, QStringLiteral("Styled"));
            QVERIFY(it);
            const QString back = it->type() == ItemType::Shape ? static_cast<ShapeItem *>(it)->storyId : static_cast<TextItem *>(it)->storyId;
            QTextCursor c(d->storyDoc(back));
            c.setPosition(4);
            const QTextCharFormat cf = c.charFormat();
            const QString which = how ? QStringLiteral("odp") : QStringLiteral("pptx");
            QVERIFY2(ColorRef::fromString(cf.stringProperty(tp::ColorRefP)).resolve(d->colors).alpha() == 0, qPrintable(which + QStringLiteral(": ") + cf.stringProperty(tp::ColorRefP)));
            QVERIFY2(cf.stringProperty(tp::TextFill).isEmpty(), qPrintable(which));
            QVERIFY2(!cf.stringProperty(tp::OutlineRef).isEmpty(), qPrintable(which));
        }
    }

    // A fill, a line, or a shadow taken off a shape that has a style stays off: the file says so beside the style, which would give it back.
    void aRemovedFillLineOrShadowStaysRemovedThroughSaving()
    {
        Rig r;
        const QString id = r.addShape(QStringLiteral("Styled"), QStringLiteral("Words"), QRectF(100, 100, 220, 110))->id;
        r.ed()->select(id);
        QVERIFY(r.w.applyShapeStyle(QStringLiteral("intense:accent2")));
        QVERIFY(r.item(id)->fx.shadow.on && !r.item(id)->fill.isNone() && !r.item(id)->stroke.isNone());
        Rig::sub(r.menuOf("ribbon.shapeEffects"), QStringLiteral("Shadow"), QStringLiteral("No Shadow"))->trigger();
        Rig::choose(r.menuOf("ribbon.shapeFill"), QStringLiteral("No Fill"))->trigger();
        Rig::choose(r.menuOf("ribbon.shapeOutline"), QStringLiteral("No Outline"))->trigger();
        QVERIFY(!r.item(id)->fx.shadow.on && r.item(id)->fill.isNone() && r.item(id)->stroke.isNone());
        QString err;
        Exported e;
        QVERIFY(unpack(deck::exportPresentation(*r.ed()->doc(), &err), &e, &err));
        QDomDocument dom;
        const QDomElement sp = shapeNamed(part(e, QStringLiteral("/ppt/slides/slide1.xml")), QStringLiteral("Styled"), &dom);
        QVERIFY(!sp.isNull());
        QVERIFY(!childByTag(sp, "p:style").isNull());
        const QDomElement spPr = childByTag(sp, "p:spPr");
        QVERIFY(!childByTag(spPr, "a:noFill").isNull());
        QVERIFY(!childByTag(childByTag(spPr, "a:ln"), "a:noFill").isNull());
        const QDomElement list = childByTag(spPr, "a:effectLst");
        QVERIFY(!list.isNull() && list.firstChildElement().isNull());
        std::unique_ptr<Document> back = r.viaPptx();
        QVERIFY(back);
        const Item *again = Rig::named(*back, QStringLiteral("Styled"));
        QVERIFY(again);
        QVERIFY(again->fill.isNone());
        QVERIFY(again->stroke.isNone());
        QVERIFY(!again->fx.shadow.on);
        QVERIFY(again->styleRef.on);
    }

    // A file that says none beside its style (no fill, no line, an empty effect list) keeps saying it; the style's own is still the style's.
    void aFilesExplicitNoneBesideItsStyleIsKept()
    {
        Parts p = samplePptx();
        p.data["/ppt/theme/theme1.xml"].xml = themeWithShadow(p.data["/ppt/theme/theme1.xml"].xml);
        const QString style = styleOf(2, 1, 2, QStringLiteral("<a:schemeClr val=\"accent2\"/>"), QStringLiteral("<a:schemeClr val=\"lt1\"/>"));
        const QString shapes = styledShape(20, QStringLiteral("From style"), QString(), style)
                               + styledShape(21, QStringLiteral("Says none"), QStringLiteral("<a:noFill/><a:ln><a:noFill/></a:ln><a:effectLst/>"), style);
        QByteArray &slide = p.data["/ppt/slides/slide1.xml"].xml;
        slide = QString::fromUtf8(slide).replace(QStringLiteral("</p:spTree>"), shapes + QStringLiteral("</p:spTree>")).toUtf8();
        QString err;
        std::unique_ptr<Document> d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const Item *given = Rig::named(*d, QStringLiteral("From style")), *none = Rig::named(*d, QStringLiteral("Says none"));
        QVERIFY(given && none);
        QVERIFY(given->fx.shadow.on && !given->fill.isNone() && !given->stroke.isNone());
        QVERIFY(!none->fx.shadow.on && none->fill.isNone() && none->stroke.isNone());
        Exported e;
        QVERIFY(unpack(deck::exportPresentation(*d, &err), &e, &err));
        QDomDocument dom;
        const QDomElement sp = shapeNamed(part(e, QStringLiteral("/ppt/slides/slide1.xml")), QStringLiteral("Says none"), &dom);
        QVERIFY(!sp.isNull());
        const QDomElement spPr = childByTag(sp, "p:spPr");
        QVERIFY(!childByTag(spPr, "a:effectLst").isNull());
        QVERIFY(!childByTag(spPr, "a:noFill").isNull());
        QVERIFY(!childByTag(childByTag(spPr, "a:ln"), "a:noFill").isNull());
        std::unique_ptr<Document> back = deck::importPresentation(deck::exportPresentation(*d, &err), &err);
        QVERIFY(back);
        const Item *again = Rig::named(*back, QStringLiteral("Says none")), *againGiven = Rig::named(*back, QStringLiteral("From style"));
        QVERIFY(again && againGiven);
        QVERIFY(!again->fx.shadow.on && again->fill.isNone() && again->stroke.isNone());
        QVERIFY(againGiven->fx.shadow.on);
        QCOMPARE(againGiven->fill.toJson(), given->fill.toJson());
        // (and the one that took them from the style writes nothing beside it)
        const QDomElement plain = shapeNamed(part(e, QStringLiteral("/ppt/slides/slide1.xml")), QStringLiteral("From style"), &dom);
        const QDomElement plainProps = childByTag(plain, "p:spPr");
        for (const char *tag : {"a:solidFill", "a:noFill", "a:ln", "a:effectLst"}) QVERIFY2(childByTag(plainProps, tag).isNull(), tag);
    }

    // Names a p:style may not have do not reach the file: a loaded reference keeps only theme colors by the standard's names, RGB colors of six
    // hex digits, and changes the standard has.
    void hostileStyleColorsAreNotWritten()
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
        auto ref = [](const QString &name, QJsonArray mods = {}) { return QJsonObject{{"idx", 2}, {"name", name}, {"mods", mods}}; };
        const QJsonArray odd{QJsonArray{QStringLiteral("zzz"), 5}, QJsonArray{QStringLiteral("lumMod"), 50000}, QJsonArray{QStringLiteral("shade"), 99999999999.0}};
        QJsonObject o{{"ln", ref(QStringLiteral("banana"), odd)}, {"fill", ref(QStringLiteral("#ZZ"))}, {"fx", ref(QStringLiteral("#12345"))},
                      {"font", QStringLiteral("minor")}, {"fontColor", ref(QStringLiteral("lt1"), odd)}};
        QString json = QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact));
        json.replace(QLatin1Char('"'), QStringLiteral("&quot;"));
        content.replace(from, to - from, json);
        ZipWriter w;
        w.add(QStringLiteral("mimetype"), zip.value(QStringLiteral("mimetype")));
        for (auto it = zip.begin(); it != zip.end(); ++it)
            if (it.key() != QLatin1String("mimetype") && it.key() != QLatin1String("content.xml")) w.add(it.key(), it.value(), true);
        w.add(QStringLiteral("content.xml"), content.toUtf8(), true);
        std::unique_ptr<Document> d = deck::importOdp(w.finish(), &err);
        QVERIFY2(d, qPrintable(err));
        Item *it = Rig::named(*d, QStringLiteral("Styled"));
        QVERIFY(it && it->styleRef.on);
        QVERIFY(it->styleRef.line.color.name.isEmpty() && it->styleRef.fill.color.name.isEmpty() && it->styleRef.effect.color.name.isEmpty());
        QCOMPARE(it->styleRef.fontColor.name, QStringLiteral("lt1"));
        QCOMPARE(it->styleRef.fontColor.mods.size(), 2);   // (lumMod, and shade with the number it can have)
        QCOMPARE(it->styleRef.fontColor.mods.first().first, QStringLiteral("lumMod"));
        // What is written is what the standard allows, whatever our own reader would put up with.
        Exported e;
        QVERIFY(unpack(deck::exportPresentation(*d, &err), &e, &err));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        for (const char *bad : {"banana", "zzz", "\"ZZ\"", "12345"}) QVERIFY2(!slide.contains(QLatin1String(bad)), bad);
        QDomDocument dom;
        const QDomElement sp = shapeNamed(slide, QStringLiteral("Styled"), &dom);
        QVERIFY(!sp.isNull());
        const QDomElement style = childByTag(sp, "p:style");
        QVERIFY(!style.isNull());
        QVERIFY2(styleProblem(style).isEmpty(), qPrintable(styleProblem(style)));
        // The same for a style a file carries: a color it can't name is no style, and the look the file's reader made stays.
        Parts parts = samplePptx();
        const QString shape = styledShape(20, QStringLiteral("Odd"), QString(), styleOf(2, 1, 0, QStringLiteral("<a:schemeClr val=\"banana\"/>"), QStringLiteral("<a:schemeClr val=\"lt1\"/>")));
        QByteArray &xml = parts.data["/ppt/slides/slide1.xml"].xml;
        xml = QString::fromUtf8(xml).replace(QStringLiteral("</p:spTree>"), shape + QStringLiteral("</p:spTree>")).toUtf8();
        std::unique_ptr<Document> odd2 = deck::importPresentation(pack(parts), &err);
        QVERIFY2(odd2, qPrintable(err));
        QVERIFY(Rig::named(*odd2, QStringLiteral("Odd")));
        QVERIFY(!Rig::named(*odd2, QStringLiteral("Odd"))->styleRef.on);
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
