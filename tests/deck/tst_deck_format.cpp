// JeffDeck's tests of the Format pane: what each page changes, that every change undoes as one step, and that it comes back from a .pptx and
// from an .odp. The pane itself is libs/page's (the pieces it is made of are tested with JeffPub's, in tst_pub_formatpane.cpp).
#include "deckhelpers.h"

#include <QStandardItemModel>

namespace {

// A slide with a few objects on it and the Format pane over its editor.
struct Rig {
    explicit Rig(int shapes = 2, int lines = 0)
    {
        ed.setDocument(deck::newPresentation());
        ed.surfaceItems().clear();
        for (int i = 0; i < shapes; ++i) {
            auto s = std::make_shared<ShapeItem>();
            s->name = QStringLiteral("Shape %1").arg(i + 1);
            s->rect = QRectF(40 + i * 150, 60, 120, 80);
            ed.addItem(s, false);
            ids << s->id;
        }
        for (int i = 0; i < lines; ++i) {
            auto l = std::make_shared<LineItem>();
            l->name = QStringLiteral("Line %1").arg(i + 1);
            l->p1 = QPointF(40, 200 + i * 30);
            l->p2 = QPointF(300, 200 + i * 30);
            l->syncRect();
            l->stroke = Stroke::line(ColorRef::rgb(Qt::black), 1);
            ed.addItem(l, false);
            ids << l->id;
        }
        ed.clearSelection();
        pane = std::make_unique<FormatPane>(&ed);
        pane->resize(340, 1400);
        pane->show();
    }
    Item *item(int i) const { return ed.doc()->item(ids[i]); }
    ShapeItem *shape(int i) const { return static_cast<ShapeItem *>(item(i)); }
    LineItem *line(int i) const { return static_cast<LineItem *>(item(i)); }
    void select(std::initializer_list<int> which)
    {
        QStringList s;
        for (int i : which) s << ids[i];
        ed.select(s);
    }
    template <class T> T *find(const char *name) const
    {
        T *w = pane->findChild<T *>(QLatin1String(name));
        if (!w) qWarning("no %s in the pane", name);
        return w;
    }
    int steps() const { return ed.undoStack()->index(); }
    QColor color(const ColorRef &c) const { return c.resolve(ed.doc()->colors); }
    // The presentation as a file of each kind brings it back.
    std::unique_ptr<Document> viaPptx() const
    {
        QString err;
        const QByteArray bytes = deck::exportPresentation(*ed.doc(), &err);
        return deck::importPresentation(bytes, &err);
    }
    std::unique_ptr<Document> viaOdp() const
    {
        QString err;
        const QByteArray bytes = deck::exportOdp(*ed.doc(), false, &err);
        return deck::importOdp(bytes, &err);
    }
    static Item *named(Document &d, const QString &name)
    {
        Item *found = nullptr;
        if (d.pages.isEmpty()) return nullptr;
        walkItems(d.pages[0]->items, [&](const ItemPtr &it) {
            if (it->name == name) found = it.get();
        });
        return found;
    }

    mutable Editor ed;
    std::unique_ptr<FormatPane> pane;
    QStringList ids;
};

bool near(double a, double b, double by = 0.01) { return std::abs(a - b) <= by; }

} // namespace

class DeckFormatTests : public QObject {
    Q_OBJECT

private:
    // The pane is open or shut as the last window left it, in the settings; the tests leave it as they found it.
    struct PaneGuard {
        PaneGuard() : was(Settings::get().value(QStringLiteral("view/formatPane"), false)) { Settings::get().setValue(QStringLiteral("view/formatPane"), false); }
        ~PaneGuard() { Settings::get().setValue(QStringLiteral("view/formatPane"), was); }
        QVariant was;
    };

    // Opens a page of the pane (on the one it is on already, it does nothing).
    static void pane_page(Rig &r, FormatPane::Tab tab) { r.pane->showTab(tab); }


private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    // ---- Fill & Line

    // A solid fill's transparency and color change every shape selected and leave what they don't share alone: shapes with different colors
    // show a blank transparency and keep their colors when it changes. Each change is one step, and the fill comes back from both files.
    void solidFillChangesEveryShapeAndKeepsWhatTheyDontShare()
    {
        Rig r(2);
        r.shape(0)->fill = Fill::solid(ColorRef::rgb(Qt::red), 0.2);
        r.shape(1)->fill = Fill::solid(ColorRef::rgb(Qt::blue), 0.5);
        r.select({0, 1});
        QVERIFY(r.find<QRadioButton>("fillSolid")->isChecked());
        auto *transparency = r.find<NumberField>("fillTransparency");
        auto *color = r.find<ColorButton>("fillColor");
        QVERIFY(transparency && color);
        QVERIFY(transparency->isMixed());
        QVERIFY(transparency->box()->cleanText().isEmpty());
        QVERIFY(color->isMixed());
        const int base = r.steps();
        transparency->box()->setValue(40);
        QCOMPARE(r.steps(), base + 1);
        QCOMPARE(r.shape(0)->fill.transparency, 0.4);
        QCOMPARE(r.shape(1)->fill.transparency, 0.4);
        QCOMPARE(r.shape(0)->fill.color, ColorRef::rgb(Qt::red));
        QCOMPARE(r.shape(1)->fill.color, ColorRef::rgb(Qt::blue));
        QVERIFY(!transparency->isMixed());
        QCOMPARE(transparency->value(), 40.0);
        // A color picked is every shape's.
        Q_EMIT color->colorPicked(ColorRef::rgb(Qt::green));
        QCOMPARE(r.steps(), base + 2);
        for (int i : {0, 1}) {
            QCOMPARE(r.shape(i)->fill.type, Fill::Solid);
            QCOMPARE(r.shape(i)->fill.color, ColorRef::rgb(Qt::green));
            QCOMPARE(r.shape(i)->fill.transparency, 0.4);
        }
        QVERIFY(!color->isMixed());
        // Both files bring it back.
        for (int how = 0; how < 2; ++how) {
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            for (const char *name : {"Shape 1", "Shape 2"}) {
                Item *it = Rig::named(*d, QLatin1String(name));
                QVERIFY2(it, name);
                QCOMPARE(it->fill.type, Fill::Solid);
                QCOMPARE(it->fill.color.resolve(d->colors), QColor(Qt::green));
                QVERIFY2(near(it->fill.transparency, 0.4, 0.01), how ? "odp" : "pptx");
            }
        }
        // One undo takes back one change.
        r.ed.undo();
        QCOMPARE(r.shape(0)->fill.color, ColorRef::rgb(Qt::red));
        QCOMPARE(r.shape(1)->fill.color, ColorRef::rgb(Qt::blue));
        QCOMPARE(r.shape(0)->fill.transparency, 0.4);
        r.ed.undo();
        QCOMPARE(r.shape(0)->fill.transparency, 0.2);
        QCOMPARE(r.shape(1)->fill.transparency, 0.5);
        r.ed.redo();
        r.ed.redo();
        QCOMPARE(r.shape(1)->fill.color, ColorRef::rgb(Qt::green));
    }

    // A slider dragged through many values is one step, taken when it is let go of; the shapes follow it on the way.
    void aSliderDraggedIsOneStep()
    {
        Rig r(1);
        r.shape(0)->fill = Fill::solid(ColorRef::rgb(Qt::red));
        r.select({0});
        auto *transparency = r.find<NumberField>("fillTransparency");
        QVERIFY(transparency && transparency->slider());
        const int base = r.steps();
        transparency->slider()->setSliderDown(true);
        for (int v = 5; v <= 65; v += 5) {
            transparency->slider()->setValue(v);
            QVERIFY(near(r.shape(0)->fill.transparency, v / 100.0));
        }
        QCOMPARE(r.steps(), base);
        QVERIFY(r.ed.isModified());   // the shape has changed, though the step is not taken yet
        transparency->slider()->setSliderDown(false);
        QCOMPARE(r.steps(), base + 1);
        QVERIFY(near(r.shape(0)->fill.transparency, 0.65));
        r.ed.undo();
        QCOMPARE(r.shape(0)->fill.transparency, 0.0);
        QCOMPARE(r.steps(), base);
    }

    // No fill, and solid to gradient and back: the color goes where the fill goes.
    void fillTypesChangeIntoOneAnother()
    {
        Rig r(2);
        r.shape(0)->fill = Fill::solid(ColorRef::rgb(Qt::red), 0.3);
        r.shape(1)->fill = Fill();
        r.select({0, 1});
        for (const char *n : {"fillNone", "fillSolid", "fillGradient", "fillPicture", "fillPattern"}) QVERIFY2(!r.find<QRadioButton>(n)->isChecked(), n);   // they differ
        r.find<QRadioButton>("fillSolid")->click();
        QCOMPARE(r.shape(0)->fill.color, ColorRef::rgb(Qt::red));   // the one that was solid keeps its color
        QCOMPARE(r.shape(1)->fill.type, Fill::Solid);   // the one with none gets the theme's first accent
        QCOMPARE(r.shape(1)->fill.color, ColorRef::scheme(Accent1));
        r.find<QRadioButton>("fillNone")->click();
        QVERIFY(r.shape(0)->fill.isNone());
        QVERIFY(r.shape(1)->fill.isNone());
        QVERIFY(r.find<QRadioButton>("fillNone")->isChecked());
        r.ed.undo();
        QCOMPARE(r.shape(0)->fill.color, ColorRef::rgb(Qt::red));
        QVERIFY(r.find<QRadioButton>("fillSolid")->isChecked());
    }

    // A gradient fill: its kind, angle, and stops (each with a color, a place, and a transparency; stops can be added, taken away, and moved)
    // are changed from the pane, are one step each, and come back from both files.
    void gradientFillHasStopsToAddRemoveAndMove()
    {
        Rig r(1);
        r.shape(0)->fill = Fill::solid(ColorRef::rgb(Qt::red));
        r.select({0});
        r.find<QRadioButton>("fillGradient")->click();
        QCOMPARE(r.shape(0)->fill.type, Fill::Gradient);
        QCOMPARE(r.shape(0)->fill.stops.size(), 2);
        QCOMPARE(r.shape(0)->fill.stops.first().color, ColorRef::rgb(Qt::red));
        auto *editor = r.find<GradientEditor>("fillGradientEditor");
        QVERIFY(editor);
        auto *kind = editor->findChild<QComboBox *>(QStringLiteral("gradientType"));
        auto *angle = editor->findChild<NumberField *>(QStringLiteral("gradientAngle"));
        auto *position = editor->findChild<NumberField *>(QStringLiteral("stopPosition"));
        auto *transparency = editor->findChild<NumberField *>(QStringLiteral("stopTransparency"));
        auto *stopColor = editor->findChild<ColorButton *>(QStringLiteral("stopColor"));
        QVERIFY(kind && angle && position && transparency && stopColor);
        angle->box()->setValue(45);
        QCOMPARE(r.shape(0)->fill.angle, 45.0);
        // The first stop made blue and half clear; a third stop added after it, in the middle, green; the last moved in.
        editor->bar()->setCurrent(0);
        Q_EMIT stopColor->colorPicked(ColorRef::rgb(Qt::blue));
        transparency->box()->setValue(50);
        editor->findChild<QToolButton *>(QStringLiteral("gradientAdd"))->click();
        QCOMPARE(r.shape(0)->fill.stops.size(), 3);
        QCOMPARE(editor->bar()->current(), 1);
        QVERIFY(near(r.shape(0)->fill.stops[1].pos, 0.5));
        Q_EMIT stopColor->colorPicked(ColorRef::rgb(Qt::green));
        editor->bar()->setCurrent(2);
        position->box()->setValue(80);
        const int base = r.steps();
        const Fill made = r.shape(0)->fill;
        QCOMPARE(made.stops.size(), 3);
        QVERIFY(near(made.stops[0].transparency, 0.5));
        QCOMPARE(made.stops[0].color, ColorRef::rgb(Qt::blue));
        QCOMPARE(made.stops[1].color, ColorRef::rgb(Qt::green));
        QVERIFY(near(made.stops[2].pos, 0.8));
        // Kept in a file of each kind.
        for (int how = 0; how < 2; ++how) {
            const QByteArray label = how ? "odp" : "pptx";
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            Item *it = Rig::named(*d, QStringLiteral("Shape 1"));
            QVERIFY2(it, label);
            const Fill &f = it->fill;
            QVERIFY2(f.type == Fill::Gradient, label);
            QVERIFY2(f.stops.size() == 3, label);
            QVERIFY2(near(f.angle, 45, 1.5), label);
            for (int i = 0; i < 3; ++i) {
                QVERIFY2(near(f.stops[i].pos, made.stops[i].pos, 0.011), label);
                QVERIFY2(f.stops[i].color.resolve(d->colors) == made.stops[i].color.resolve(r.ed.doc()->colors), label);
                QVERIFY2(near(f.stops[i].transparency, made.stops[i].transparency, 0.011), label);
            }
        }
        // A stop taken away, and the kind changed: one step each.
        editor->bar()->setCurrent(1);
        editor->findChild<QToolButton *>(QStringLiteral("gradientRemove"))->click();
        QCOMPARE(r.shape(0)->fill.stops.size(), 2);
        QCOMPARE(r.steps(), base + 1);
        kind->setCurrentIndex(int(Fill::Radial));
        QCOMPARE(r.shape(0)->fill.gradType, Fill::Radial);
        QCOMPARE(r.steps(), base + 2);
        for (int how = 0; how < 2; ++how) {
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            QCOMPARE(Rig::named(*d, QStringLiteral("Shape 1"))->fill.gradType, Fill::Radial);
        }
        r.ed.undo();
        QCOMPARE(r.shape(0)->fill.gradType, Fill::Linear);
        QCOMPARE(r.shape(0)->fill.stops.size(), 2);
        r.ed.undo();
        QCOMPARE(r.shape(0)->fill.stops.size(), 3);
    }

    // A picture fill from a file, stretched or tiled, with its transparency, kept in both files. Cancelling the file dialog changes nothing.
    void pictureFillIsStretchedOrTiledAndKept()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QImage image(40, 20, QImage::Format_RGB32);
        image.fill(QColor(200, 30, 30));
        const QString path = dir.filePath(QStringLiteral("red.png"));
        QVERIFY(image.save(path));
        Rig r(2);
        r.select({0, 1});
        QString answer;
        r.pane->setPictureChooser([&] { return answer; });
        const int base = r.steps();
        r.find<QRadioButton>("fillPicture")->click();   // cancelled: no file
        QCOMPARE(r.steps(), base);
        QVERIFY(r.shape(0)->fill.type != Fill::Picture);
        answer = path;
        r.find<QRadioButton>("fillPicture")->click();
        QCOMPARE(r.steps(), base + 1);
        for (int i : {0, 1}) {
            QCOMPARE(r.shape(i)->fill.type, Fill::Picture);
            QVERIFY(!r.shape(i)->fill.imageId.isEmpty());
            QVERIFY(!r.shape(i)->fill.tile);
        }
        QCOMPARE(r.shape(0)->fill.imageId, r.shape(1)->fill.imageId);
        QVERIFY(r.find<QRadioButton>("fillPicture")->isChecked());
        // Tiled, at half its size, a quarter clear.
        r.find<QCheckBox>("pictureTile")->click();
        QVERIFY(r.shape(0)->fill.tile);
        auto *scale = r.find<NumberField>("pictureScale");
        QVERIFY(scale->isEnabled());
        scale->box()->setValue(50);
        QVERIFY(near(r.shape(1)->fill.tileScale, 0.5));
        r.find<NumberField>("pictureTransparency")->box()->setValue(25);
        QVERIFY(near(r.shape(1)->fill.transparency, 0.25));
        for (int how = 0; how < 2; ++how) {
            const QByteArray label = how ? "odp" : "pptx";
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            for (const char *name : {"Shape 1", "Shape 2"}) {
                Item *it = Rig::named(*d, QLatin1String(name));
                QVERIFY2(it, label);
                QVERIFY2(it->fill.type == Fill::Picture || it->fill.type == Fill::Texture, label);
                QVERIFY2(!it->fill.imageId.isEmpty() && d->image(it->fill.imageId).size() == QSize(40, 20), label);
                QVERIFY2(it->fill.tile || it->fill.type == Fill::Texture, label);
                QVERIFY2(near(it->fill.tileScale, 0.5, 0.02), label);
                QVERIFY2(near(it->fill.transparency, 0.25, 0.02), label);
            }
        }
        // Stretched again.
        r.find<QCheckBox>("pictureTile")->click();
        QVERIFY(!r.shape(0)->fill.tile);
        std::unique_ptr<Document> d = r.viaPptx();
        QVERIFY(!Rig::named(*d, QStringLiteral("Shape 1"))->fill.tile);
        // Undo takes the picture away with the fill.
        while (r.steps() > base) r.ed.undo();
        QVERIFY(r.shape(0)->fill.type != Fill::Picture);
    }

    // A pattern fill: one of the 13 patterns, and its foreground and background colors.
    void patternFillHasForegroundAndBackground()
    {
        Rig r(1);
        r.select({0});
        r.find<QRadioButton>("fillPattern")->click();
        QCOMPARE(r.shape(0)->fill.type, Fill::Pattern);
        auto *button = r.find<QToolButton>("pattern8");   // vertical lines
        QVERIFY(button);
        button->click();
        QCOMPARE(r.shape(0)->fill.pattern, 8);
        QVERIFY(r.find<QToolButton>("pattern8")->isChecked());
        Q_EMIT r.find<ColorButton>("patternFore")->colorPicked(ColorRef::rgb(Qt::blue));
        Q_EMIT r.find<ColorButton>("patternBack")->colorPicked(ColorRef::rgb(Qt::yellow));
        QCOMPARE(r.shape(0)->fill.color, ColorRef::rgb(Qt::blue));
        QCOMPARE(r.shape(0)->fill.color2, ColorRef::rgb(Qt::yellow));
        for (int how = 0; how < 2; ++how) {
            const QByteArray label = how ? "odp" : "pptx";
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            const Fill &f = Rig::named(*d, QStringLiteral("Shape 1"))->fill;
            QVERIFY2(f.type == Fill::Pattern, label);
            QVERIFY2(f.pattern == 8, label);
            QVERIFY2(f.color.resolve(d->colors) == QColor(Qt::blue), label);
            QVERIFY2(f.color2.resolve(d->colors) == QColor(Qt::yellow), label);
        }
    }

    // A line: its color, transparency, width, compound type, dash type, cap, join, and the arrows at its ends, each from the pane, each one
    // step, and kept by both files.
    void lineHasColorWidthCompoundDashCapJoinAndArrows()
    {
        Rig r(0, 1);
        r.select({0});
        auto *lineColor = r.find<ColorButton>("lineColor");
        QVERIFY(lineColor && r.find<QRadioButton>("lineSolid")->isChecked());
        const int base = r.steps();
        Q_EMIT lineColor->colorPicked(ColorRef::rgb(Qt::red));
        r.find<NumberField>("lineWidth")->box()->setValue(4.5);
        r.find<NumberField>("lineTransparency")->box()->setValue(20);
        r.find<QComboBox>("lineDash")->setCurrentIndex(int(Stroke::LongDash));
        r.find<QComboBox>("lineCap")->setCurrentIndex(1);
        r.find<QComboBox>("lineJoin")->setCurrentIndex(1);
        r.find<QComboBox>("lineCompound")->setCurrentIndex(int(Stroke::ThickThin));
        r.find<QComboBox>("lineBeginType")->setCurrentIndex(int(Arrow::Triangle));
        r.find<QComboBox>("lineBeginSize")->setCurrentIndex(2);
        r.find<QComboBox>("lineEndType")->setCurrentIndex(int(Arrow::Oval));
        r.find<QComboBox>("lineEndSize")->setCurrentIndex(0);
        QCOMPARE(r.steps(), base + 11);   // each its own step
        const Stroke made = r.line(0)->stroke;
        QCOMPARE(made.color, ColorRef::rgb(Qt::red));
        QCOMPARE(made.width, 4.5);
        QVERIFY(near(made.transparency, 0.2));
        QCOMPARE(made.dash, Stroke::LongDash);
        QCOMPARE(made.cap, Qt::RoundCap);
        QCOMPARE(made.join, Qt::BevelJoin);
        QCOMPARE(made.compound, Stroke::ThickThin);
        QCOMPARE(made.startArrow, Arrow::Triangle);
        QCOMPARE(made.startSize, 2);
        QCOMPARE(made.endArrow, Arrow::Oval);
        QCOMPARE(made.endSize, 0);
        for (int how = 0; how < 2; ++how) {
            const QByteArray label = how ? "odp" : "pptx";
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            Item *it = Rig::named(*d, QStringLiteral("Line 1"));
            QVERIFY2(it && it->type() == ItemType::Line, label);
            const Stroke &s = it->stroke;
            QVERIFY2(s.color.resolve(d->colors) == QColor(Qt::red), label);
            QVERIFY2(near(s.width, 4.5), label);
            QVERIFY2(near(s.transparency, 0.2, 0.011), label);
            QVERIFY2(s.dash == Stroke::LongDash, label);
            QVERIFY2(s.cap == Qt::RoundCap, label);
            QVERIFY2(s.join == Qt::BevelJoin, label);
            QVERIFY2(s.compound == Stroke::ThickThin, label);
            QVERIFY2(s.startArrow == Arrow::Triangle && s.startSize == 2, label);
            QVERIFY2(s.endArrow == Arrow::Oval && s.endSize == 0, label);
        }
        // No line, and a line again.
        r.find<QRadioButton>("lineNone")->click();
        QVERIFY(r.line(0)->stroke.isNone());
        r.find<QRadioButton>("lineSolid")->click();
        QVERIFY(!r.line(0)->stroke.isNone());
        r.ed.undo();
        r.ed.undo();
        QCOMPARE(r.line(0)->stroke.color, ColorRef::rgb(Qt::red));
    }

    // A gradient line: its colors along it, from the pane; it is drawn in them, and both files keep it as it is made.
    void gradientLineIsDrawnInItsColorsAndKept()
    {
        Rig r(0, 1);
        r.line(0)->stroke = Stroke::line(ColorRef::rgb(Qt::red), 12);
        r.select({0});
        r.find<QRadioButton>("lineGradient")->click();
        QVERIFY(r.line(0)->stroke.hasGradient());
        auto *editor = r.find<GradientEditor>("lineGradientEditor");
        QVERIFY(editor);
        QCOMPARE(r.line(0)->stroke.gradient.stops.size(), 2);
        editor->bar()->setCurrent(1);
        Q_EMIT editor->findChild<ColorButton *>(QStringLiteral("stopColor"))->colorPicked(ColorRef::rgb(Qt::blue));
        QCOMPARE(r.line(0)->stroke.color, ColorRef::rgb(Qt::red));   // the first stop
        QCOMPARE(r.line(0)->stroke.gradient.stops.last().color, ColorRef::rgb(Qt::blue));
        // Drawn: the left end is red and the right end blue.
        QImage slide = slideImage(*r.ed.doc());
        QVERIFY(!slide.isNull());
        const double sx = slide.width() / r.ed.doc()->pageSize().width();
        const double y = (r.line(0)->p1.y()) * sx;
        const QColor left = slide.pixelColor(int((r.line(0)->p1.x() + 20) * sx), int(y)), right = slide.pixelColor(int((r.line(0)->p2.x() - 20) * sx), int(y));
        QVERIFY2(left.red() > left.blue() + 60, qPrintable(left.name()));
        QVERIFY2(right.blue() > right.red() + 60, qPrintable(right.name()));
        // The pptx keeps the gradient; the odp, which has no gradient lines, keeps the first color.
        std::unique_ptr<Document> p = r.viaPptx();
        QVERIFY(p);
        const Stroke &ps = Rig::named(*p, QStringLiteral("Line 1"))->stroke;
        QVERIFY(ps.hasGradient());
        QCOMPARE(ps.gradient.stops.size(), 2);
        QCOMPARE(ps.gradient.stops.last().color.resolve(p->colors), QColor(Qt::blue));
        std::unique_ptr<Document> o = r.viaOdp();
        QVERIFY(o);
        const Stroke &os = Rig::named(*o, QStringLiteral("Line 1"))->stroke;
        QVERIFY(os.hasGradient());
        QCOMPARE(os.gradient.stops.last().color.resolve(o->colors), QColor(Qt::blue));
        // Solid again takes the gradient away.
        r.find<QRadioButton>("lineSolid")->click();
        QVERIFY(!r.line(0)->stroke.hasGradient());
        QCOMPARE(r.line(0)->stroke.color, ColorRef::rgb(Qt::red));
    }

    // Lines that differ show blank where they differ; the rest of each is kept when one thing changes.
    void mixedLinesShowBlankAndKeepTheirOwn()
    {
        Rig r(2);
        r.shape(0)->stroke = Stroke::line(ColorRef::rgb(Qt::red), 1);
        r.shape(1)->stroke = Stroke::line(ColorRef::rgb(Qt::red), 6);
        r.shape(1)->stroke.dash = Stroke::DashLine;
        r.select({0, 1});
        QVERIFY(r.find<QRadioButton>("lineSolid")->isChecked());
        QVERIFY(r.find<NumberField>("lineWidth")->isMixed());
        QVERIFY(r.find<NumberField>("lineWidth")->box()->cleanText().isEmpty());
        QCOMPARE(r.find<QComboBox>("lineDash")->currentIndex(), -1);
        QVERIFY(!r.find<ColorButton>("lineColor")->isMixed());
        QCOMPARE(r.find<QComboBox>("lineCap")->currentIndex(), 0);
        r.find<QComboBox>("lineCap")->setCurrentIndex(2);
        QCOMPARE(r.shape(0)->stroke.cap, Qt::SquareCap);
        QCOMPARE(r.shape(1)->stroke.cap, Qt::SquareCap);
        QCOMPARE(r.shape(0)->stroke.width, 1.0);
        QCOMPARE(r.shape(1)->stroke.width, 6.0);
        QCOMPARE(r.shape(1)->stroke.dash, Stroke::DashLine);
        // Shapes have no ends, so no arrows to choose; a line has.
        QVERIFY(!r.find<QComboBox>("lineBeginType")->isVisibleTo(r.pane.get()));
    }

    // A group takes the fill and line of what is in it, and the pane says nothing of a line's fill.
    void groupsTakeTheFillAndLineOfWhatIsInThem()
    {
        Rig r(2, 1);
        auto g = std::make_shared<GroupItem>();
        g->name = QStringLiteral("Group 1");
        g->children = {r.ed.doc()->pages[0]->items[0], r.ed.doc()->pages[0]->items[1]};
        r.ed.surfaceItems().erase(r.ed.surfaceItems().begin(), r.ed.surfaceItems().begin() + 2);
        g->syncRect();
        r.ed.surfaceItems().insert(r.ed.surfaceItems().begin(), g);
        r.ed.select(g->id);
        r.find<QRadioButton>("fillSolid")->click();
        Q_EMIT r.find<ColorButton>("fillColor")->colorPicked(ColorRef::rgb(Qt::magenta));
        for (int i : {0, 1}) QCOMPARE(r.shape(i)->fill.color, ColorRef::rgb(Qt::magenta));
        // A line alone: no fill section.
        r.select({2});
        QVERIFY(!r.pane->frame()->page(0)->section(QStringLiteral("Fill"))->isVisibleTo(r.pane.get()));
        QVERIFY(r.pane->frame()->page(0)->section(QStringLiteral("Line"))->isVisibleTo(r.pane.get()));
        QVERIFY(r.find<QComboBox>("lineBeginType")->isVisibleTo(r.pane.get()));
    }

    // ---- Effects

    // A shadow: switched on with a preset, then its color, transparency, size, blur, angle, and distance, each one step; shadows that differ show
    // blank and keep their own; both files keep it.
    void shadowHasPresetsAndEveryProperty()
    {
        Rig r(2);
        r.select({0, 1});
        pane_page(r, FormatPane::Tab::Effects);
        auto *on = r.find<QCheckBox>("shadowOn");
        QVERIFY(on && on->checkState() == Qt::Unchecked);
        QVERIFY(!r.find<NumberField>("shadowBlur")->isEnabled());   // nothing to change while there is no shadow
        const int base = r.steps();
        auto *preset = r.find<QComboBox>("shadowPreset");
        QVERIFY(preset && preset->count() == 9);
        preset->setCurrentIndex(2);   // bottom left
        QCOMPARE(r.steps(), base + 1);
        QCOMPARE(preset->currentIndex(), -1);   // a preset is an action, and the box goes back to saying "Presets"
        for (int i : {0, 1}) {
            QVERIFY(r.shape(i)->fx.shadow.on);
            QCOMPARE(r.shape(i)->fx.shadow.angle, 135.0);
        }
        QCOMPARE(on->checkState(), Qt::Checked);
        QVERIFY(r.find<NumberField>("shadowBlur")->isEnabled());
        Q_EMIT r.find<ColorButton>("shadowColor")->colorPicked(ColorRef::rgb(Qt::darkBlue));
        r.find<NumberField>("shadowTransparency")->box()->setValue(30);
        r.find<NumberField>("shadowSize")->box()->setValue(120);
        r.find<NumberField>("shadowBlur")->box()->setValue(9);
        r.find<NumberField>("shadowAngle")->box()->setValue(300);
        r.find<NumberField>("shadowDistance")->box()->setValue(12.5);
        QCOMPARE(r.steps(), base + 7);
        for (int i : {0, 1}) {
            const ShadowFx &sh = r.shape(i)->fx.shadow;
            QCOMPARE(sh.color, ColorRef::rgb(Qt::darkBlue));
            QVERIFY(near(sh.transparency, 0.3));
            QCOMPARE(sh.size, 120.0);
            QCOMPARE(sh.blur, 9.0);
            QCOMPARE(sh.angle, 300.0);
            QCOMPARE(sh.distance, 12.5);
        }
        for (int how = 0; how < 2; ++how) {
            const QByteArray label = how ? "odp" : "pptx";
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            for (const char *name : {"Shape 1", "Shape 2"}) {
                const ShadowFx &sh = Rig::named(*d, QLatin1String(name))->fx.shadow;
                QVERIFY2(sh.on, label);
                QVERIFY2(sh.color.resolve(d->colors) == QColor(Qt::darkBlue), label);
                QVERIFY2(near(sh.transparency, 0.3, 0.011), label);
                QVERIFY2(near(sh.size, 120, 0.5), label);
                QVERIFY2(near(sh.blur, 9, 0.05), label);
                QVERIFY2(near(sh.angle, 300, 0.5), label);
                QVERIFY2(near(sh.distance, 12.5, 0.05), label);
            }
        }
        // Different blurs: blank, and a change of the angle alone leaves each blur.
        r.shape(1)->fx.shadow.blur = 2;
        r.ed.notifyLive();
        QVERIFY(r.find<NumberField>("shadowBlur")->isMixed());
        r.find<NumberField>("shadowAngle")->box()->setValue(20);
        QCOMPARE(r.shape(0)->fx.shadow.blur, 9.0);
        QCOMPARE(r.shape(1)->fx.shadow.blur, 2.0);
        QCOMPARE(r.shape(1)->fx.shadow.angle, 20.0);
        // Off, and one undo brings it back with all it had.
        on->click();
        QVERIFY(!r.shape(0)->fx.shadow.on && !r.shape(1)->fx.shadow.on);
        r.ed.undo();
        QVERIFY(r.shape(0)->fx.shadow.on && r.shape(1)->fx.shadow.on);
        QCOMPARE(r.shape(0)->fx.shadow.blur, 9.0);
    }

    // A shadow of another size is drawn larger or smaller about the middle of the object; at 100 it is as it was.
    void shadowSizeIsDrawn()
    {
        Rig r(1);
        ShapeItem *s = r.shape(0);
        s->rect = QRectF(300, 200, 120, 80);
        s->fill = Fill::solid(ColorRef::rgb(Qt::white));
        s->stroke = Stroke::none();
        s->fx.shadow.on = true;
        s->fx.shadow.color = ColorRef::rgb(Qt::black);
        s->fx.shadow.transparency = 0;
        s->fx.shadow.blur = 0;
        s->fx.shadow.distance = 0;
        const double scale = 1.0;
        auto darkAt = [&](double x, double y) {
            QImage img = slideImage(*r.ed.doc());
            const double k = img.width() / r.ed.doc()->pageSize().width() * scale;
            return img.pixelColor(int(x * k), int(y * k)).lightness() < 100;
        };
        // 10 points left of the shape, on its middle.
        QVERIFY(!darkAt(290, 240));   // size 100: hidden behind the shape
        s->fx.shadow.size = 130;
        QVERIFY(darkAt(290, 240));    // 130 percent of 120 points is 18 more, 9 on each side
        QVERIFY(!darkAt(280, 240));
        s->fx.shadow.size = 50;
        QVERIFY(!darkAt(290, 240));
    }

    // A reflection: presets, then transparency, size, distance, and blur; drawn under the object; kept by the .pptx and the .odp.
    void reflectionHasPresetsAndEveryProperty()
    {
        Rig r(1);
        r.select({0});
        pane_page(r, FormatPane::Tab::Effects);
        const int base = r.steps();
        r.find<QComboBox>("reflectionPreset")->setCurrentIndex(1);   // half
        QCOMPARE(r.steps(), base + 1);
        QVERIFY(r.shape(0)->fx.reflection.on);
        QVERIFY(near(r.shape(0)->fx.reflection.size, 0.55));
        r.find<NumberField>("reflectionTransparency")->box()->setValue(40);
        r.find<NumberField>("reflectionSize")->box()->setValue(70);
        r.find<NumberField>("reflectionDistance")->box()->setValue(6);
        r.find<NumberField>("reflectionBlur")->box()->setValue(2);
        const ReflectionFx &x = r.shape(0)->fx.reflection;
        QVERIFY(near(x.transparency, 0.4) && near(x.size, 0.7) && x.distance == 6 && x.blur == 2);
        for (int how = 0; how < 2; ++how) {
            const QByteArray label = how ? "odp" : "pptx";
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            const ReflectionFx &got = Rig::named(*d, QStringLiteral("Shape 1"))->fx.reflection;
            QVERIFY2(got.on, label);
            QVERIFY2(near(got.transparency, 0.4, 0.011) && near(got.size, 0.7, 0.011) && near(got.distance, 6, 0.05) && near(got.blur, 2, 0.05), label);
        }
        // It shows under the object.
        ShapeItem *s = r.shape(0);
        s->rect = QRectF(300, 100, 120, 80);
        s->fill = Fill::solid(ColorRef::rgb(Qt::black));
        s->fx.reflection.transparency = 0;
        s->fx.reflection.size = 1;
        s->fx.reflection.blur = 0;
        s->fx.reflection.distance = 0;
        QImage img = slideImage(*r.ed.doc());
        const double k = img.width() / r.ed.doc()->pageSize().width();
        QVERIFY(img.pixelColor(int(360 * k), int(190 * k)).lightness() < 140);   // just under the shape
        s->fx.reflection.on = false;
        img = slideImage(*r.ed.doc());
        QVERIFY(img.pixelColor(int(360 * k), int(190 * k)).lightness() > 200);
    }

    // A glow has a color, a size, and a transparency; soft edges a size.
    void glowAndSoftEdgesHaveTheirProperties()
    {
        Rig r(1);
        r.select({0});
        pane_page(r, FormatPane::Tab::Effects);
        r.find<QComboBox>("glowPreset")->setCurrentIndex(1);   // 10 points
        QVERIFY(r.shape(0)->fx.glow.on);
        QCOMPARE(r.shape(0)->fx.glow.size, 10.0);
        Q_EMIT r.find<ColorButton>("glowColor")->colorPicked(ColorRef::rgb(Qt::yellow));
        r.find<NumberField>("glowSize")->box()->setValue(14);
        r.find<NumberField>("glowTransparency")->box()->setValue(35);
        r.find<QComboBox>("softEdgePreset")->setCurrentIndex(2);   // 5 points
        QCOMPARE(r.shape(0)->fx.softEdge, 5.0);
        r.find<NumberField>("softEdgeSize")->box()->setValue(7.5);
        for (int how = 0; how < 2; ++how) {
            const QByteArray label = how ? "odp" : "pptx";
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            const Effects &fx = Rig::named(*d, QStringLiteral("Shape 1"))->fx;
            QVERIFY2(fx.glow.on, label);
            QVERIFY2(fx.glow.color.resolve(d->colors) == QColor(Qt::yellow), label);
            QVERIFY2(near(fx.glow.size, 14, 0.1) && near(fx.glow.transparency, 0.35, 0.011), label);
            QVERIFY2(near(fx.softEdge, 7.5, 0.1), label);
        }
        r.find<QCheckBox>("glowOn")->click();
        QVERIFY(!r.shape(0)->fx.glow.on);
        r.find<NumberField>("softEdgeSize")->box()->setValue(0);
        QCOMPARE(r.shape(0)->fx.softEdge, 0.0);
    }

    // The effects of a group are its children's; a line has a shadow and no other effect.
    void effectsOfGroupsAndLines()
    {
        Rig r(2, 1);
        r.select({2});
        pane_page(r, FormatPane::Tab::Effects);
        QVERIFY(r.pane->frame()->page(1)->section(QStringLiteral("Shadow"))->isVisibleTo(r.pane.get()));
        QVERIFY(!r.pane->frame()->page(1)->section(QStringLiteral("Glow"))->isVisibleTo(r.pane.get()));
        r.find<QComboBox>("shadowPreset")->setCurrentIndex(0);
        QVERIFY(r.line(2)->fx.shadow.on);
        r.select({0, 1});
        QVERIFY(r.pane->frame()->page(1)->section(QStringLiteral("Glow"))->isVisibleTo(r.pane.get()));
        r.find<QComboBox>("glowPreset")->setCurrentIndex(0);
        QVERIFY(r.shape(0)->fx.glow.on && r.shape(1)->fx.glow.on);
    }

    // ---- Size & Properties

    // Height, width, rotation, and place (from the top left corner) are typed in the boxes; each is one step. What the objects don't share is
    // blank, and the objects locked in place keep theirs. Both files keep them.
    void sizeRotationAndPlaceAreTypedInTheBoxes()
    {
        Rig r(2);
        r.shape(1)->rect = QRectF(250, 60, 90, 80);
        r.select({0, 1});
        pane_page(r, FormatPane::Tab::SizeProperties);
        auto *height = r.find<NumberField>("sizeHeight");
        auto *width = r.find<NumberField>("sizeWidth");
        QVERIFY(height && width);
        QVERIFY(!height->isMixed());   // 80 and 80
        QCOMPARE(height->value(), 80.0);
        QVERIFY(width->isMixed());      // 120 and 90
        QVERIFY(width->box()->cleanText().isEmpty());
        const int base = r.steps();
        width->box()->setValue(200);
        height->box()->setValue(50);
        r.find<NumberField>("sizeRotation")->box()->setValue(30);
        r.find<NumberField>("positionX")->box()->setValue(20);
        r.find<NumberField>("positionY")->box()->setValue(35.5);
        QCOMPARE(r.steps(), base + 5);
        for (int i : {0, 1}) {
            QCOMPARE(r.shape(i)->rect, QRectF(20, 35.5, 200, 50));
            QCOMPARE(r.shape(i)->rotation, 30.0);
        }
        QVERIFY(!width->isMixed());
        for (int how = 0; how < 2; ++how) {
            const QByteArray label = how ? "odp" : "pptx";
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            for (const char *name : {"Shape 1", "Shape 2"}) {
                Item *it = Rig::named(*d, QLatin1String(name));
                QVERIFY2(it, label);
                QVERIFY2(near(it->rect.x(), 20, 0.05) && near(it->rect.y(), 35.5, 0.05) && near(it->rect.width(), 200, 0.05) && near(it->rect.height(), 50, 0.05), label);
                QVERIFY2(near(it->rotation, 30, 0.05), label);
            }
        }
        // One locked in place keeps its size and its place.
        r.shape(1)->locked = true;
        r.ed.notifyLive();
        width->box()->setValue(60);
        QCOMPARE(r.shape(0)->rect.width(), 60.0);
        QCOMPARE(r.shape(1)->rect.width(), 200.0);
        r.ed.undo();
        QCOMPARE(r.shape(0)->rect.width(), 200.0);
    }

    // Lock aspect ratio: with it on, the width follows the height and the height the width; it is kept in both files; a picture has it on from
    // the start, and a corner dragged on the canvas keeps proportions for exactly the objects that have it.
    void lockAspectRatioKeepsTheProportions()
    {
        Rig r(1);
        r.select({0});
        pane_page(r, FormatPane::Tab::SizeProperties);
        auto *lock = r.find<QCheckBox>("lockAspect");
        QVERIFY(lock && lock->checkState() == Qt::Unchecked);
        r.find<NumberField>("sizeWidth")->box()->setValue(240);
        QCOMPARE(r.shape(0)->rect.size(), QSizeF(240, 80));   // free
        lock->click();
        QVERIFY(r.shape(0)->lockAspect);
        r.find<NumberField>("sizeWidth")->box()->setValue(120);
        QCOMPARE(r.shape(0)->rect.size(), QSizeF(120, 40));   // 240 by 80 is three to one
        r.find<NumberField>("sizeHeight")->box()->setValue(80);
        QCOMPARE(r.shape(0)->rect.size(), QSizeF(240, 80));
        for (int how = 0; how < 2; ++how) {
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            QVERIFY2(Rig::named(*d, QStringLiteral("Shape 1"))->lockAspect, how ? "odp" : "pptx");
        }
        lock->click();
        QVERIFY(!r.shape(0)->lockAspect);
        for (int how = 0; how < 2; ++how) {
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY2(!Rig::named(*d, QStringLiteral("Shape 1"))->lockAspect, how ? "odp" : "pptx");
        }
        // A picture starts with it on and keeps a choice not to have it.
        auto pic = std::make_shared<PictureItem>();
        pic->name = QStringLiteral("Picture 1");
        pic->rect = QRectF(300, 100, 100, 50);
        pic->imgRect = QRectF(0, 0, 100, 50);
        QVERIFY(pic->lockAspect);
        QJsonObject json = pic->toJson();
        QVERIFY(!json.contains(QStringLiteral("lockAspect")));   // as it was for every picture saved before
        pic->lockAspect = false;
        json = pic->toJson();
        QVERIFY(json.contains(QStringLiteral("lockAspect")));
        QVERIFY(!static_cast<PictureItem *>(Item::fromJsonAny(json).get())->lockAspect);
        QVERIFY(static_cast<PictureItem *>(Item::fromJsonAny(PictureItem().toJson()).get())->lockAspect);
    }

    // A picture is scaled against the size of its file; the scale boxes show how much, and typing one changes the frame and the picture in it
    // together. Anything else is scaled against the size it has.
    void scaleIsAgainstThePicturesOwnSize()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QImage image(80, 40, QImage::Format_RGB32);   // 60 by 30 points
        image.fill(Qt::red);
        const QString path = dir.filePath(QStringLiteral("red.png"));
        QVERIFY(image.save(path));
        Rig r(1);
        auto pic = std::make_shared<PictureItem>();
        pic->name = QStringLiteral("Picture 1");
        QByteArray bytes = readPictureBytes(path);
        pic->imageId = r.ed.doc()->addImage(bytes, QStringLiteral("png"), path);
        pic->rect = QRectF(300, 100, 120, 60);
        pic->imgRect = QRectF(0, 0, 120, 60);
        r.ed.addItem(pic, false);
        r.ids << pic->id;
        r.select({1});
        pane_page(r, FormatPane::Tab::SizeProperties);
        QCOMPARE(r.find<NumberField>("scaleWidth")->value(), 200.0);
        QCOMPARE(r.find<NumberField>("scaleHeight")->value(), 200.0);
        r.find<NumberField>("scaleWidth")->box()->setValue(100);   // locked: the height follows
        QCOMPARE(r.item(1)->rect.size(), QSizeF(60, 30));
        QCOMPARE(static_cast<PictureItem *>(r.item(1))->imgRect.size(), QSizeF(60, 30));
        QCOMPARE(r.find<NumberField>("scaleHeight")->value(), 100.0);
        // A shape: 100 until it is changed, and then the size changes by that much.
        r.select({0});
        QCOMPARE(r.find<NumberField>("scaleWidth")->value(), 100.0);
        r.find<NumberField>("scaleWidth")->box()->setValue(150);
        QCOMPARE(r.shape(0)->rect.width(), 180.0);
        QCOMPARE(r.shape(0)->rect.height(), 80.0);
        QCOMPARE(r.find<NumberField>("scaleWidth")->value(), 100.0);
    }

    // The options of the text in a text box: where it sits, which way it runs, whether the box or the text gives way, the four margins, whether it
    // wraps, and columns; each one step, and kept by both files.
    void textBoxOptionsAreChangedAndKept()
    {
        Rig r(0);
        auto t = std::make_shared<TextItem>();
        t->name = QStringLiteral("Text 1");
        t->rect = QRectF(40, 60, 240, 120);
        t->storyId = r.ed.doc()->createStory(QStringLiteral("One two three four five six seven eight nine ten eleven twelve thirteen fourteen fifteen sixteen"));
        r.ed.addItem(t, false);
        r.ids << t->id;
        r.select({0});
        pane_page(r, FormatPane::Tab::SizeProperties);
        auto text = [&]() { return static_cast<TextItem *>(r.item(0)); };
        QVERIFY(r.pane->frame()->page(2)->section(QStringLiteral("Text Box"))->isVisibleTo(r.pane.get()));
        const int base = r.steps();
        r.find<QComboBox>("textAlign")->setCurrentIndex(3);   // middle, centered
        QVERIFY(text()->valign == VAlign::Middle && text()->centerBlock);
        r.find<QComboBox>("textDirection")->setCurrentIndex(1);
        QVERIFY(text()->vertical);
        r.find<NumberField>("marginLeft")->box()->setValue(10);
        r.find<NumberField>("marginRight")->box()->setValue(11);
        r.find<NumberField>("marginTop")->box()->setValue(12);
        r.find<NumberField>("marginBottom")->box()->setValue(13);
        r.find<QCheckBox>("textWrap")->click();
        QVERIFY(text()->noWrap);
        r.find<QComboBox>("textDirection")->setCurrentIndex(0);
        r.find<QCheckBox>("textWrap")->click();
        QVERIFY(!text()->noWrap);
        r.find<NumberField>("textColumns")->box()->setValue(3);
        QCOMPARE(text()->columns, 3);
        QVERIFY(r.find<NumberField>("columnSpacing")->isEnabled());
        r.find<NumberField>("columnSpacing")->box()->setValue(20);
        QCOMPARE(r.steps(), base + 11);
        QCOMPARE(text()->insets, QMarginsF(10, 12, 11, 13));
        QCOMPARE(text()->columnGap, 20.0);
        for (int how = 0; how < 2; ++how) {
            const QByteArray label = how ? "odp" : "pptx";
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            Item *it = Rig::named(*d, QStringLiteral("Text 1"));
            QVERIFY2(it && it->type() == ItemType::Text, label);
            const auto &got = *static_cast<TextItem *>(it);
            QVERIFY2(got.valign == VAlign::Middle && got.centerBlock, label);
            QVERIFY2(!got.vertical && !got.noWrap, label);
            QVERIFY2(near(got.insets.left(), 10, 0.05) && near(got.insets.top(), 12, 0.05) && near(got.insets.right(), 11, 0.05) && near(got.insets.bottom(), 13, 0.05), label);
            QVERIFY2(got.columns == 3 && near(got.columnGap, 20, 0.05), label);
        }
        // Autofit: shrink on overflow; resize shape to fit text (the box grows to the text, here a few lines); none.
        r.find<QComboBox>("textAutofit")->setCurrentIndex(1);
        QCOMPARE(text()->autofit, TextItem::ShrinkOnOverflow);
        r.find<QComboBox>("textAutofit")->setCurrentIndex(2);
        QCOMPARE(text()->autofit, TextItem::GrowBox);
        r.find<NumberField>("textColumns")->box()->setValue(1);
        r.find<QComboBox>("textAutofit")->setCurrentIndex(0);
        QCOMPARE(text()->autofit, TextItem::NoAutofit);
        r.find<QComboBox>("textAutofit")->setCurrentIndex(2);
        QVERIFY2(text()->rect.height() < 120, "the box is fitted to the text");
        for (int how = 0; how < 2; ++how) {
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            QCOMPARE(static_cast<TextItem *>(Rig::named(*d, QStringLiteral("Text 1")))->autofit, TextItem::GrowBox);
        }
    }

    // A shape has the margins, wrap, vertical alignment, and columns too; its text can't shrink or grow it yet, and says so by offering no
    // choice but none.
    void shapeTextOptionsAreKeptAndShrinkingIsNotOffered()
    {
        Rig r(1);
        r.select({0});
        pane_page(r, FormatPane::Tab::SizeProperties);
        r.find<QComboBox>("textAlign")->setCurrentIndex(4);   // bottom
        r.find<NumberField>("marginLeft")->box()->setValue(20);
        r.find<NumberField>("textColumns")->box()->setValue(2);
        r.find<NumberField>("columnSpacing")->box()->setValue(15);
        r.find<QCheckBox>("textWrap")->click();
        QVERIFY(r.shape(0)->valign == VAlign::Bottom && !r.shape(0)->centerBlock);
        QVERIFY(r.shape(0)->noWrap && r.shape(0)->columns == 2 && r.shape(0)->columnGap == 15);
        auto *autofit = r.find<QComboBox>("textAutofit");
        auto *model = qobject_cast<QStandardItemModel *>(autofit->model());
        QVERIFY(model);
        QVERIFY(model->item(0)->isEnabled() && !model->item(1)->isEnabled() && !model->item(2)->isEnabled());
        QVERIFY(!r.find<QComboBox>("textDirection")->isEnabled());
        for (int how = 0; how < 2; ++how) {
            const QByteArray label = how ? "odp" : "pptx";
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            Item *it = Rig::named(*d, QStringLiteral("Shape 1"));
            QVERIFY2(it && it->type() == ItemType::Shape, label);
            const auto &got = *static_cast<ShapeItem *>(it);
            QVERIFY2(got.valign == VAlign::Bottom && got.noWrap, label);
            QVERIFY2(near(got.insets.left(), 20, 0.05) && got.columns == 2 && near(got.columnGap, 15, 0.05), label);
        }
        // Text in columns is laid out in them.
        r.shape(0)->storyId = r.ed.doc()->createStory(QStringLiteral("alpha beta gamma delta epsilon zeta eta theta iota kappa lambda mu"));
        r.shape(0)->noWrap = false;
        r.shape(0)->rect = QRectF(40, 60, 400, 200);
        PaintContext ctx;
        ctx.doc = r.ed.doc();
        ctx.cache = &r.ed.cache();
        ctx.pageNumber = 1;
        QPointF origin;
        const StoryLayout *lay = Renderer::shapeTextLayout(ctx, *r.shape(0), &origin);
        QVERIFY(lay);
        const double inColumns = lay->usedHeight(0);
        r.shape(0)->columns = 1;
        r.ed.cache().clear();
        const StoryLayout *one = Renderer::shapeTextLayout(ctx, *r.shape(0), &origin);
        QVERIFY(one);
        // The same words in two columns take less height than in one.
        QVERIFY2(inColumns < one->usedHeight(0) - 1, qPrintable(QStringLiteral("%1 and %2").arg(inColumns).arg(one->usedHeight(0))));
    }

    // Alt text: a title and a description for screen readers, and a mark that the object is only decoration; the description and title are
    // blank where the objects differ, and decoration turns the boxes off. Both files keep all three.
    void altTextHasTitleDescriptionAndDecorative()
    {
        Rig r(2);
        r.shape(0)->altText = QStringLiteral("A red box");
        r.select({0, 1});
        pane_page(r, FormatPane::Tab::SizeProperties);
        auto *title = r.find<TextField>("altTitle");
        auto *description = r.find<TextField>("altDescription");
        auto *decorative = r.find<QCheckBox>("altDecorative");
        QVERIFY(title && description && decorative);
        QVERIFY(description->text().isEmpty() && description->isMixed());   // they differ
        r.select({0});
        QCOMPARE(description->text(), QStringLiteral("A red box"));
        const int base = r.steps();
        title->setText(QStringLiteral("Red box"));
        title->moveCursor(QTextCursor::End);
        QTest::keyClicks(title, QStringLiteral("!"));   // typing changes nothing until the person leaves the box
        QCOMPARE(r.steps(), base);
        title->commit();
        QCOMPARE(r.steps(), base + 1);
        QCOMPARE(r.shape(0)->altTitle, QStringLiteral("Red box!"));
        description->moveCursor(QTextCursor::End);
        QTest::keyClicks(description, QStringLiteral(" and more"));
        QCOMPARE(r.steps(), base + 1);
        description->commit();
        QCOMPARE(r.steps(), base + 2);
        QCOMPARE(r.shape(0)->altText, QStringLiteral("A red box and more"));
        for (int how = 0; how < 2; ++how) {
            const QByteArray label = how ? "odp" : "pptx";
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            Item *it = Rig::named(*d, QStringLiteral("Shape 1"));
            QVERIFY2(it, label);
            QVERIFY2(it->altTitle == QStringLiteral("Red box!") && it->altText == QStringLiteral("A red box and more"), label);
        }
        decorative->click();
        QVERIFY(r.shape(0)->decorative);
        QVERIFY(!title->isEnabled() && !description->isEnabled());
        for (int how = 0; how < 2; ++how) {
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY2(Rig::named(*d, QStringLiteral("Shape 1"))->decorative, how ? "odp" : "pptx");
            QVERIFY2(!Rig::named(*d, QStringLiteral("Shape 2"))->decorative, how ? "odp" : "pptx");
        }
        decorative->click();
        QVERIFY(!r.shape(0)->decorative && title->isEnabled());
        r.ed.undo();
        QVERIFY(r.shape(0)->decorative);
        // A title and decoration kept in the JSON of a document too.
        QVERIFY(r.shape(0)->toJson().contains(QStringLiteral("decorative")));
        QCOMPARE(Item::fromJsonAny(r.shape(0)->toJson())->altTitle, QStringLiteral("Red box!"));
    }

    // ---- Picture

    // A picture's corrections (brightness and contrast), color (recolor, its level, and transparency), and crop (the whole picture's size and
    // place in its frame): each one step; kept by both files; the tab is there for pictures alone.
    void pictureCorrectionsColorAndCropAreChangedAndKept()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QImage image(80, 40, QImage::Format_RGB32);
        image.fill(QColor(200, 60, 30));
        const QString path = dir.filePath(QStringLiteral("red.png"));
        QVERIFY(image.save(path));
        Rig r(1);
        auto pic = std::make_shared<PictureItem>();
        pic->name = QStringLiteral("Picture 1");
        pic->imageId = r.ed.doc()->addImage(readPictureBytes(path), QStringLiteral("png"), path);
        pic->rect = QRectF(300, 100, 120, 60);
        pic->imgRect = QRectF(0, 0, 120, 60);
        r.ed.addItem(pic, false);
        r.ids << pic->id;
        r.select({0});
        QVERIFY(!r.pane->isTabShown(FormatPane::Tab::Picture));   // a shape has none
        r.select({1});
        QVERIFY(r.pane->isTabShown(FormatPane::Tab::Picture));
        QCOMPARE(r.pane->title(), QStringLiteral("Format Picture"));
        pane_page(r, FormatPane::Tab::Picture);
        auto picture = [&]() { return static_cast<PictureItem *>(r.item(1)); };
        const int base = r.steps();
        r.find<NumberField>("imageBrightness")->box()->setValue(25);
        r.find<NumberField>("imageContrast")->box()->setValue(-40);
        r.find<QComboBox>("imageRecolor")->setCurrentIndex(2);   // black and white
        QVERIFY(r.find<NumberField>("imageThreshold")->isEnabled());
        r.find<NumberField>("imageThreshold")->box()->setValue(60);
        r.find<NumberField>("imageTransparency")->box()->setValue(30);
        r.find<NumberField>("cropWidth")->box()->setValue(240);
        r.find<NumberField>("cropHeight")->box()->setValue(120);
        r.find<NumberField>("cropOffsetX")->box()->setValue(-20);
        r.find<NumberField>("cropOffsetY")->box()->setValue(10);
        QCOMPARE(r.steps(), base + 9);
        QCOMPARE(picture()->brightness, 25.0);
        QCOMPARE(picture()->contrast, -40.0);
        QCOMPARE(picture()->recolor, PictureItem::BlackWhite);
        QVERIFY(near(picture()->blackWhiteLevel, 0.6) && near(picture()->transparency, 0.3));
        QVERIFY(near(picture()->imgRect.width(), 240) && near(picture()->imgRect.height(), 120));
        QVERIFY(near(picture()->imgRect.center().x(), 60 - 20) && near(picture()->imgRect.center().y(), 30 + 10));
        for (int how = 0; how < 2; ++how) {
            const QByteArray label = how ? "odp" : "pptx";
            std::unique_ptr<Document> d = how ? r.viaOdp() : r.viaPptx();
            QVERIFY(d);
            Item *it = Rig::named(*d, QStringLiteral("Picture 1"));
            QVERIFY2(it && it->type() == ItemType::Picture, label);
            const auto &got = *static_cast<PictureItem *>(it);
            QVERIFY2(near(got.brightness, 25, 1) && near(got.contrast, -40, 1), label);
            QVERIFY2(got.recolor == PictureItem::BlackWhite, label);
            if (!how) QVERIFY2(near(got.blackWhiteLevel, 0.6, 0.011), label);   // (a .odp has no level of its own)
            QVERIFY2(near(got.transparency, 0.3, 0.011), label);
            QVERIFY2(near(got.imgRect.width(), 240, 1.5) && near(got.imgRect.height(), 120, 1.5), label);
            QVERIFY2(near(got.imgRect.center().x(), 40, 1.5) && near(got.imgRect.center().y(), 40, 1.5), label);
        }
        r.find<QComboBox>("imageRecolor")->setCurrentIndex(1);
        QCOMPARE(picture()->recolor, PictureItem::Grayscale);
        QVERIFY(!r.find<NumberField>("imageThreshold")->isEnabled());
        std::unique_ptr<Document> d = r.viaPptx();
        QCOMPARE(static_cast<PictureItem *>(Rig::named(*d, QStringLiteral("Picture 1")))->recolor, PictureItem::Grayscale);
        // The picture drawn is changed: black and white is only black and white.
        picture()->recolor = PictureItem::BlackWhite;
        picture()->transparency = 0;
        picture()->imgRect = QRectF(0, 0, 120, 60);
        QImage slide = slideImage(*r.ed.doc());
        const double k = slide.width() / r.ed.doc()->pageSize().width();
        const QColor c = slide.pixelColor(int(360 * k), int(130 * k));
        QVERIFY2((c.red() == c.green() && c.green() == c.blue()) && (c.red() < 30 || c.red() > 225), qPrintable(c.name()));
    }

    // ---- the window

    // The Shape Format tab shows while a shape, a text box, a line, or a group of them is selected, and not for a picture, a table, or nothing;
    // its Height and Width boxes show the size of the object and change it (and every other selected) as one step, keeping proportions if locked.
    void shapeFormatTabFollowsTheSelectionAndSizesIt()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        PaneGuard guard;
        DeckWindow w;
        Editor *ed = w.editor();
        ed->surfaceItems().clear();
        auto shown = [&] {
            for (int i = 0; i < w.ribbon()->tabCount(); ++i)
                if (w.ribbon()->tabName(i) == QLatin1String("Shape Format")) return w.ribbon()->tabVisible(i);
            return false;
        };
        QVERIFY(!shown());
        auto shape = std::make_shared<ShapeItem>();
        shape->rect = QRectF(100, 100, 200, 100);
        auto box = std::static_pointer_cast<TextItem>(ed->newTextBox(QRectF(100, 250, 240, 60), QStringLiteral("Words")));
        auto line = std::make_shared<LineItem>();
        line->p1 = QPointF(400, 100);
        line->p2 = QPointF(600, 100);
        line->syncRect();
        ed->addItem(shape, false);
        ed->addItem(box, false);
        ed->addItem(line, false);
        for (const QString &id : {shape->id, box->id, line->id}) {
            ed->select(id);
            QVERIFY2(shown(), qPrintable(id));
        }
        ed->select(QStringList{shape->id, box->id});
        QVERIFY(shown());
        ed->clearSelection();
        QVERIFY(!shown());
        ed->addItem(ed->newTable(QRectF(100, 400, 200, 80), 2, 2), true);
        QVERIFY(!shown());
        // A picture does not show it (it has the Picture Format tab); neither does a shape with a picture among what is selected.
        QImage image(20, 10, QImage::Format_RGB32);
        image.fill(Qt::red);
        const QString path = dir.filePath(QStringLiteral("red.png"));
        QVERIFY(image.save(path));
        QVERIFY(w.insertPicture(path));
        QVERIFY(!shown());
        ed->select(QStringList{ed->selection().first(), shape->id});
        QVERIFY(!shown());
        // The boxes.
        ed->select(shape->id);
        auto *height = w.ribbon()->findChild<MeasureSpin *>(QStringLiteral("shapeHeight"));
        auto *width = w.ribbon()->findChild<MeasureSpin *>(QStringLiteral("shapeWidth"));
        QVERIFY(height && width);
        QVERIFY(height->isEnabled());
        QCOMPARE(height->value(), 100.0);
        QCOMPARE(width->value(), 200.0);
        const int base = ed->undoStack()->index();
        width->setValue(300);
        QCOMPARE(ed->undoStack()->index(), base + 1);
        QCOMPARE(ed->doc()->item(shape->id)->rect.size(), QSizeF(300, 100));
        ed->doc()->item(shape->id)->lockAspect = true;
        height->setValue(50);
        QCOMPARE(ed->doc()->item(shape->id)->rect.size(), QSizeF(150, 50));
        ed->select(QStringList{shape->id, box->id});
        height->setValue(40);   // every one selected takes it
        QCOMPARE(ed->doc()->item(box->id)->rect.height(), 40.0);
        ed->undo();
        QCOMPARE(ed->doc()->item(box->id)->rect.height(), 60.0);
        ed->clearSelection();
        QVERIFY(!height->isEnabled());
    }

    // The three menus of the tab reach the same settings as the pane, each choice one step: a color, none, a gradient, or a picture for the fill; a
    // color, none, a weight, a dash, or arrows for the outline; a shadow, reflection, glow, or soft edges. The last choice of each opens the pane.
    void shapeFormatMenusReachThePanesSettings()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QImage image(20, 10, QImage::Format_RGB32);
        image.fill(Qt::red);
        const QString path = dir.filePath(QStringLiteral("red.png"));
        QVERIFY(image.save(path));
        PaneGuard guard;
        DeckWindow w;
        Editor *ed = w.editor();
        ed->surfaceItems().clear();
        auto shape = std::make_shared<ShapeItem>();
        shape->rect = QRectF(100, 100, 200, 100);
        auto line = std::make_shared<LineItem>();
        line->p1 = QPointF(400, 100);
        line->p2 = QPointF(600, 100);
        line->syncRect();
        ed->addItem(shape, false);
        ed->addItem(line, false);
        ed->select(QStringList{shape->id, line->id});
        w.formatPane()->setPictureChooser([&] { return path; });
        auto menuOf = [&](const char *id) -> QMenu * {
            for (QToolButton *b : w.ribbon()->findChildren<QToolButton *>())
                if (b->defaultAction() && b->defaultAction()->objectName() == QLatin1String(id) && b->menu()) return b->menu();
            return nullptr;
        };
        auto choose = [](QMenu *m, const QString &text) -> QAction * {
            Q_EMIT m->aboutToShow();
            for (QAction *a : m->actions())
                if (a->text() == text) return a;
            return nullptr;
        };
        auto sub = [&](QMenu *m, const QString &title, const QString &text) -> QAction * {
            Q_EMIT m->aboutToShow();
            for (QAction *a : m->actions())
                if (a->menu() && a->text() == title)
                    for (QAction *b : a->menu()->actions())
                        if (b->text() == text) return b;
            return nullptr;
        };
        QMenu *fill = menuOf("ribbon.shapeFill"), *outline = menuOf("ribbon.shapeOutline"), *effects = menuOf("ribbon.shapeEffects");
        QVERIFY(fill && outline && effects);
        int steps = ed->undoStack()->index();
        auto oneStep = [&](QAction *a, const char *what) {
            QVERIFY2(a, what);
            a->trigger();
            QVERIFY2(ed->undoStack()->index() == steps + 1, what);
            ++steps;
        };
        // Fill: the shape's, and the line takes none.
        QAction *noFill = choose(fill, QStringLiteral("No Fill"));
        oneStep(noFill, "no fill");
        QVERIFY(ed->doc()->item(shape->id)->fill.isNone());
        oneStep(sub(fill, QStringLiteral("Gradient"), QStringLiteral("Linear Right")), "gradient");
        QCOMPARE(ed->doc()->item(shape->id)->fill.type, Fill::Gradient);
        QCOMPARE(ed->doc()->item(shape->id)->fill.angle, 0.0);
        oneStep(choose(fill, QStringLiteral("Picture…")), "picture");
        QCOMPARE(ed->doc()->item(shape->id)->fill.type, Fill::Picture);
        // The grid's swatches are in the menu as one widget; a click on one is a solid fill, and the menu is closed after.
        Q_EMIT fill->aboutToShow();
        QWidget *grid = nullptr;
        for (QAction *a : fill->actions())
            if (auto *wa = qobject_cast<QWidgetAction *>(a)) grid = wa->defaultWidget();
        QVERIFY(grid);
        auto swatches = grid->findChildren<QToolButton *>(QStringLiteral("swatch"));
        QVERIFY(swatches.size() > 40);
        swatches.first()->click();
        QCOMPARE(ed->doc()->item(shape->id)->fill.type, Fill::Solid);
        QCOMPARE(ed->doc()->item(shape->id)->fill.color, ColorRef::scheme(Main));
        ++steps;
        QCOMPARE(ed->undoStack()->index(), steps);
        // Outline: weight, dash, arrows (on the line), none.
        oneStep(sub(outline, QStringLiteral("Weight"), QStringLiteral("3 pt")), "weight");
        QCOMPARE(ed->doc()->item(shape->id)->stroke.width, 3.0);
        QCOMPARE(ed->doc()->item(line->id)->stroke.width, 3.0);
        oneStep(sub(outline, QStringLiteral("Dashes"), dashName(Stroke::DashDot)), "dash");
        QCOMPARE(ed->doc()->item(line->id)->stroke.dash, Stroke::DashDot);
        oneStep(sub(outline, QStringLiteral("Arrows"), QStringLiteral("Double Arrow")), "arrows");
        QCOMPARE(ed->doc()->item(line->id)->stroke.startArrow, Arrow::Triangle);
        QCOMPARE(ed->doc()->item(line->id)->stroke.endArrow, Arrow::Triangle);
        oneStep(choose(outline, QStringLiteral("No Outline")), "no outline");
        QVERIFY(ed->doc()->item(shape->id)->stroke.isNone() && ed->doc()->item(line->id)->stroke.isNone());
        // Effects.
        oneStep(sub(effects, QStringLiteral("Shadow"), QStringLiteral("Offset Top Left")), "shadow");
        QVERIFY(ed->doc()->item(shape->id)->fx.shadow.on && ed->doc()->item(line->id)->fx.shadow.on);
        QCOMPARE(ed->doc()->item(shape->id)->fx.shadow.angle, 225.0);
        oneStep(sub(effects, QStringLiteral("Reflection"), QStringLiteral("Half Reflection")), "reflection");
        QVERIFY(ed->doc()->item(shape->id)->fx.reflection.on);
        oneStep(sub(effects, QStringLiteral("Glow"), QStringLiteral("10 Point Glow")), "glow");
        QCOMPARE(ed->doc()->item(shape->id)->fx.glow.size, 10.0);
        oneStep(sub(effects, QStringLiteral("Soft Edges"), QStringLiteral("2.5 Point")), "soft edges");
        QCOMPARE(ed->doc()->item(shape->id)->fx.softEdge, 2.5);
        oneStep(sub(effects, QStringLiteral("Shadow"), QStringLiteral("No Shadow")), "no shadow");
        QVERIFY(!ed->doc()->item(shape->id)->fx.shadow.on);
        // The last choice opens the pane on the page.
        QVERIFY(!w.formatPane()->isVisibleTo(&w));
        choose(effects, QStringLiteral("More Effects Options…"))->trigger();
        QVERIFY(!w.formatDock()->isHidden());
        QCOMPARE(w.formatPane()->currentTab(), FormatPane::Tab::Effects);
        choose(fill, QStringLiteral("More Fill Options…"))->trigger();
        QCOMPARE(w.formatPane()->currentTab(), FormatPane::Tab::FillLine);
    }

    // Format Shape is on the right-click menu of a shape, and Format Picture on a picture's; both open the pane on a page, and so do Ctrl+1,
    // the Format Pane button (which toggles it), the launchers, and Alt Text (the alt text page, with the description ready to type in).
    void formatShapeOpensThePaneFromEveryWayIn()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QImage image(20, 10, QImage::Format_RGB32);
        image.fill(Qt::red);
        const QString path = dir.filePath(QStringLiteral("red.png"));
        QVERIFY(image.save(path));
        PaneGuard guard;
        DeckWindow w;
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        Editor *ed = w.editor();
        ed->surfaceItems().clear();
        auto shape = std::make_shared<ShapeItem>();
        shape->rect = QRectF(100, 100, 200, 100);
        ed->addItem(shape, true);
        QVERIFY(w.formatDock()->isHidden());
        auto menuTexts = [&] {
            std::unique_ptr<QMenu> menu(w.slideContextMenu());
            QStringList out;
            for (QAction *a : menu->actions()) out << a->text();
            return out;
        };
        QVERIFY(menuTexts().contains(QStringLiteral("Format Shape…")));
        {
            std::unique_ptr<QMenu> menu(w.slideContextMenu());
            QAction *format = nullptr;
            for (QAction *a : menu->actions())
                if (a->text() == QStringLiteral("Format Shape…")) format = a;
            QVERIFY(format);
            format->trigger();
        }
        QVERIFY(!w.formatDock()->isHidden());
        QCOMPARE(w.formatPane()->currentTab(), FormatPane::Tab::FillLine);
        QCOMPARE(w.formatPane()->title(), QStringLiteral("Format Shape"));
        QVERIFY(w.act(QStringLiteral("shape.formatPane"))->isChecked());
        // The keyboard is on the pane's tabs, and Esc gives it back to the slide.
        QTRY_VERIFY(w.formatPane()->frame()->tabButton(0)->hasFocus());
        QTest::keyClick(w.formatPane()->frame()->tabButton(0), Qt::Key_Escape);
        QTRY_VERIFY(w.canvas()->hasFocus());
        // The button toggles it.
        w.act(QStringLiteral("shape.formatPane"))->trigger();
        QVERIFY(w.formatDock()->isHidden());
        w.act(QStringLiteral("shape.formatPane"))->trigger();
        QVERIFY(!w.formatDock()->isHidden());
        // The pane's own close button.
        w.formatPane()->frame()->closeButton()->click();
        QVERIFY(w.formatDock()->isHidden());
        QVERIFY(!w.act(QStringLiteral("shape.formatPane"))->isChecked());
        // Alt Text.
        w.act(QStringLiteral("shape.altText"))->trigger();
        QVERIFY(!w.formatDock()->isHidden());
        QCOMPARE(w.formatPane()->currentTab(), FormatPane::Tab::SizeProperties);
        QVERIFY(w.formatPane()->frame()->page(2)->section(QStringLiteral("Alt Text"))->isExpanded());
        QTRY_VERIFY(w.formatPane()->findChild<TextField *>(QStringLiteral("altDescription"))->hasFocus());
        // A picture: Format Picture, on the Picture page.
        QVERIFY(w.insertPicture(path));
        QVERIFY(menuTexts().contains(QStringLiteral("Format Picture…")));
        w.act(QStringLiteral("shape.format"))->trigger();
        QCOMPARE(w.formatPane()->currentTab(), FormatPane::Tab::Picture);
        // The tab's launchers.
        ed->select(shape->id);
        for (QToolButton *b : w.ribbon()->findChildren<QToolButton *>())
            if (b->objectName() == QLatin1String("launcher") && b->toolTip() == QLatin1String("Size and Position")) b->click();
        QCOMPARE(w.formatPane()->currentTab(), FormatPane::Tab::SizeProperties);
    }

    // The pane is open or shut as it was left: a new window opens it if the last change was to open it, and not if it was to close it.
    void paneRemembersWhetherItIsOpen()
    {
        PaneGuard guard;
        {
            DeckWindow w;
            QVERIFY(w.formatDock()->isHidden());
            w.showFormatPane(true);
        }
        {
            DeckWindow w;
            QVERIFY(!w.formatDock()->isHidden());
            QVERIFY(w.act(QStringLiteral("shape.formatPane"))->isChecked());
            w.formatPane()->frame()->closeButton()->click();
        }
        {
            DeckWindow w;
            QVERIFY(w.formatDock()->isHidden());
            QVERIFY(!w.act(QStringLiteral("shape.formatPane"))->isChecked());
        }
    }

    // F1 in the pane, and on the Shape Format tab, opens the topic about them; the topic names the keys the commands have.
    void helpTellsAboutThePaneAndTheTab()
    {
        QCOMPARE(help::contextTopic(QStringLiteral("tab:Shape Format")), QStringLiteral("format-shape"));
        QCOMPARE(help::contextTopic(QStringLiteral("pane:format")), QStringLiteral("format-shape"));
        const help::Topic topic = help::topic(QStringLiteral("format-shape"));
        QVERIFY(!topic.title.isEmpty() && topic.markdown.contains(QStringLiteral("**Ctrl+1**")));
        PaneGuard guard;
        DeckWindow w;
        QCOMPARE(w.act(QStringLiteral("shape.format"))->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_1));
        w.resize(1400, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowActive(&w));
        w.editor()->surfaceItems().clear();
        auto shape = std::make_shared<ShapeItem>();
        shape->rect = QRectF(100, 100, 200, 100);
        w.editor()->addItem(shape, true);
        w.formatSelection(FormatPane::Tab::FillLine);
        QTRY_VERIFY(w.formatPane()->frame()->tabButton(0)->hasFocus());
        w.act(QStringLiteral("help.show"))->trigger();   // F1: the topic for where the keyboard is
        QVERIFY(w.helpView());
        QTRY_COMPARE(w.helpView()->currentTopic(), QStringLiteral("format-shape"));
    }
};

JP_TEST_CLASS(DeckFormatTests)
#include "tst_deck_format.moc"
