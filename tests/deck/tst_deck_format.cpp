// JeffDeck's tests of the Format pane: what each page changes, that every change undoes as one step, and that it comes back from a .pptx and
// from an .odp. The pane itself is libs/page's (the pieces it is made of are tested with JeffPub's, in tst_pub_formatpane.cpp).
#include "deckhelpers.h"

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
};

JP_TEST_CLASS(DeckFormatTests)
#include "tst_deck_format.moc"
