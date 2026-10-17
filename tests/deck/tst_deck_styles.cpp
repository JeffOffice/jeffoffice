// JeffDeck's tests of the Shape Format tab's styles: the Quick Styles gallery drawn from the theme (and how a style is saved), Change Shape, and
// the WordArt styles of the text in a shape. Each command is one step that undoes, and comes back from a .pptx and from an .odp.
#include "deckhelpers.h"

#include "app/formatops.h"
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

} // namespace

class DeckStylesTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

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
