// JeffPub's tests of objects on the page: wrapping, inline objects, shapes, connectors, footnotes, and editing text.
#include "pubhelpers.h"

class PubObjectsTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }

    // Text Art's Word Justify widens the spaces and Letter Justify the gaps
    // between letters (Word Justify was left-aligned, and Letter Justify
    // stretched the letters, which is Stretch Justify).
    void textArtJustify()
    {
        jp::TextArtItem w;
        w.text = QStringLiteral("ab cd\nabcdefghijklmnopqrstuv");
        w.font = QStringLiteral("DejaVu Sans");
        // The top line's ink: how much there is, and how far it reaches.
        auto topLine = [&](int align) {
            w.align = align;
            const QPainterPath path = jp::textArtPath(w, QSizeF(600, 200));
            QImage img(600, 100, QImage::Format_Grayscale8);
            img.fill(255);
            QPainter p(&img);
            p.setRenderHint(QPainter::Antialiasing);
            p.fillPath(path, Qt::black);
            p.end();
            int ink = 0, right = 0;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (img.pixelColor(x, y).value() < 128) { ++ink; right = std::max(right, x); }
            return std::make_pair(ink, right);
        };
        const auto left = topLine(0), word = topLine(3), letter = topLine(4), stretch = topLine(5);
        QVERIFY2(left.second < 300, qPrintable(QString::number(left.second)));
        for (const auto &j : {word, letter, stretch}) QVERIFY2(j.second > 540, qPrintable(QString::number(j.second)));
        // Moved, not widened: as much ink as left-aligned; stretched letters have far more.
        QVERIFY2(std::abs(word.first - left.first) < left.first / 20, qPrintable(QStringLiteral("%1 %2").arg(word.first).arg(left.first)));
        QVERIFY2(std::abs(letter.first - left.first) < left.first / 20, qPrintable(QStringLiteral("%1 %2").arg(letter.first).arg(left.first)));
        QVERIFY2(stretch.first > left.first * 1.5, qPrintable(QStringLiteral("%1 %2").arg(stretch.first).arg(left.first)));
    }

    // A line with nothing in it but objects set in the text is as tall as
    // the tallest of them inside its wrap distances: no text above or below
    // it, so the paragraph's font size and line spacing (1.0 or more) don't
    // change it. Publisher (Oct 11), a box 100 pt tall alone in a paragraph
    // of 12, 36, or 72 pt Arial, single, 1.5, or double spaced: the next
    // paragraph 2.88 + 100 + 2.88 lower; a box 20 pt tall gave 25.76, less
    // than the text's own line. With text in the line the descent stays.
    void inlineObjectAloneInLine()
    {
        using namespace jp;
        for (double size : {12.0, 36.0})
            for (double spacing : {100.0, 150.0})
                for (double h : {20.0, 100.0}) {
                    auto doc = Document::blank(QSizeF(612, 792));
                    auto t = std::make_shared<TextItem>();
                    t->rect = QRectF(72, 72, 200, 500);
                    t->insets = QMarginsF(0, 0, 0, 0);
                    t->storyId = doc->createStory(QStringLiteral("HH\n\nHH"));
                    QTextCursor c(doc->storyDoc(t->storyId));
                    c.select(QTextCursor::Document);
                    QTextCharFormat cf;
                    cf.setFontFamilies({QStringLiteral("Arial")});
                    cf.setFontPointSize(size);
                    c.mergeCharFormat(cf);
                    QTextBlockFormat bf;
                    bf.setTopMargin(0);
                    bf.setBottomMargin(0);
                    bf.setLineHeight(spacing, QTextBlockFormat::ProportionalHeight);
                    c.mergeBlockFormat(bf);
                    ShapeItem box;
                    box.rect = QRectF(400, 600, 24, h);
                    c.setPosition(3);   // the empty second paragraph
                    QTextCharFormat of = c.charFormat();
                    of.setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(box.toJson()).toJson(QJsonDocument::Compact)));
                    c.insertText(QString(QChar::ObjectReplacementCharacter), of);
                    doc->pages[0]->items = {t};
                    LayoutCache cache;
                    const auto fl = cache.textFrame(*doc, *t, 1, RenderOptions());
                    const auto info = fl.layout->lineInfo(0);
                    const QString what = QStringLiteral("size %1 spacing %2 h %3").arg(size).arg(spacing).arg(h);
                    QCOMPARE(int(info.size()), 3);
                    const double line = 2.88 + h + 2.88;
                    QVERIFY2(std::abs(info[2].rect.top() - info[1].rect.top() - line) < 0.05, qPrintable(what + QStringLiteral(": the next line starts %1 below").arg(info[2].rect.top() - info[1].rect.top())));
                    QVERIFY2(std::abs(info[1].baseline - (info[1].rect.top() + line)) < 0.05, qPrintable(what + QStringLiteral(": baseline %1").arg(info[1].baseline - info[1].rect.top())));
                    const auto objects = fl.layout->inlineObjects();
                    QCOMPARE(int(objects.size()), 1);
                    QVERIFY2(std::abs(objects[0].rect.top() - (info[1].rect.top() + 2.88)) < 0.05, qPrintable(what));
                }
    }

    // A line of weight 0 is no line in Publisher: its picture and its PDF
    // (Oct 11, a text box, a rectangle, and a straight line of weight 0,
    // 0.25, and 0.75) show nothing for the 0, and the text sits where a box
    // with no line has it. JeffPub read it as a hairline, so a box whose line
    // was 0 had a border it never had, and saved it as a visible 0.25.
    void aLineOfWeightZeroIsNoLine()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(300, 200));
        auto box = std::make_shared<TextItem>();
        box->rect = QRectF(20, 20, 100, 60);
        box->insets = QMarginsF(3, 3, 3, 3);
        box->storyId = doc->createStory(QStringLiteral("Hi"));
        box->stroke = Stroke::line(ColorRef::rgb(Qt::black), 0.00001);   // written as 0 EMU
        auto rect = std::make_shared<ShapeItem>();
        rect->shape = QStringLiteral("rect");
        rect->rect = QRectF(150, 20, 100, 60);
        rect->fill = Fill::none();
        rect->stroke = Stroke::line(ColorRef::rgb(Qt::black), 0.00001);
        doc->pages[0]->items.push_back(box);
        doc->pages[0]->items.push_back(rect);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("zero-line.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        int seen = 0;
        std::function<void(const Item &)> visit = [&](const Item &it) {
            if (it.type() == ItemType::Group) {
                for (const auto &c : static_cast<const GroupItem &>(it).children) visit(*c);
                return;
            }
            if (it.type() == ItemType::Text) {
                ++seen;
                const auto &t = static_cast<const TextItem &>(it);
                QVERIFY2(t.stroke.isNone(), qPrintable(QStringLiteral("text box line %1").arg(t.stroke.width)));
            } else if (it.type() == ItemType::Shape) {
                ++seen;
                const auto &sh = static_cast<const ShapeItem &>(it);
                QVERIFY2(sh.stroke.isNone(), qPrintable(QStringLiteral("shape line %1").arg(sh.stroke.width)));
            }
        };
        for (const auto &it : back->pages[0]->items) visit(*it);
        QVERIFY(seen >= 1);   // the text box (the rectangle, with no fill and no line, draws nothing)
    }

    // Publisher refuses ("cannot open the file") a publication with a shape
    // holding text whose shape chunk carries the text direction field (34) at
    // 0: every build saved one so since shapes could hold text (Oct 5), and
    // the saves of a rectangle, a star, and a callout with text all failed
    // alone (Oct 11). Without the field the file opens and the text sits
    // where JeffPub puts it. A text box leaves the field out unless its text
    // runs top to bottom, and so does a shape holding text.
    void aShapeHoldingTextHasNoTextDirectionField()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(300, 200));
        auto sh = std::make_shared<ShapeItem>();
        sh->shape = QStringLiteral("rect");
        sh->rect = QRectF(20, 20, 200, 100);
        sh->fill = Fill::solid(ColorRef::rgb(QColor(250, 220, 120)));
        sh->stroke = Stroke::line(ColorRef::rgb(Qt::black), 1);
        sh->storyId = doc->createStory(QStringLiteral("Text in a rectangle"));
        doc->pages[0]->items.push_back(sh);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("shape-text.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        const QByteArray c = cfb::readStream(path, QStringLiteral("Contents"));
        // The shape chunk's text id block (27:20) with the field after it (34:20).
        int ids = 0, withDirection = 0;
        for (qsizetype at = c.indexOf(QByteArray::fromHex("2720")); at >= 0; at = c.indexOf(QByteArray::fromHex("2720"), at + 1)) {
            if (c.size() < at + 8) break;
            ++ids;
            if (c.mid(at + 6, 2) == QByteArray::fromHex("3420")) ++withDirection;
        }
        QVERIFY2(ids >= 1, "the shape's text id");
        QVERIFY2(withDirection == 0, qPrintable(QStringLiteral("%1 of %2 text ids are followed by a 34:20 field").arg(withDirection).arg(ids)));
        // A box whose text runs top to bottom keeps it.
        auto doc2 = Document::blank(QSizeF(300, 200));
        auto tb = std::make_shared<TextItem>();
        tb->rect = QRectF(20, 20, 40, 150);
        tb->storyId = doc2->createStory(QStringLiteral("Spine"));
        tb->vertical = true;
        doc2->pages[0]->items.push_back(tb);
        const QString path2 = dir.filePath(QStringLiteral("spine.pub"));
        QVERIFY2(exportPublisher(*doc2, path2, &err), qPrintable(err));
        const QByteArray c2 = cfb::readStream(path2, QStringLiteral("Contents"));
        QVERIFY(c2.contains(QByteArray::fromHex("342002000000")));
    }

    // Publisher's freeforms hold no text, and
    // it refuses a file with a freeform that has a text id, as JeffPub saved
    // a star, a callout, or any shape other than a rectangle that held text
    // (Oct 11: "cannot open the file"). Such a shape is saved as the shape and
    // a text box over it, set where JeffPub sets the shape's text.
    void aShapeOtherThanARectangleHoldingTextIsSavedWithATextBox()
    {
        using namespace jp;
        for (const QString kind : {QStringLiteral("star5"), QStringLiteral("wedgeRoundRectCallout"), QStringLiteral("ellipse")}) {
            for (double angle : {0.0, 30.0}) {
                auto doc = Document::blank(QSizeF(400, 300));
                auto sh = std::make_shared<ShapeItem>();
                sh->shape = kind;
                sh->rect = QRectF(60, 40, 250, 150);
                sh->rotation = angle;
                sh->fill = Fill::solid(ColorRef::rgb(QColor(250, 220, 120)));
                sh->stroke = Stroke::line(ColorRef::rgb(Qt::black), 2);
                sh->valign = VAlign::Top;
                sh->storyId = doc->createStory(QStringLiteral("Words in the shape"));
                doc->pages[0]->items.push_back(sh);
                // Where JeffPub sets the first line: its left edge's page position.
                LayoutCache cache;
                PaintContext ctx;
                ctx.doc = doc.get();
                ctx.cache = &cache;
                QPointF origin;
                const auto lay = Renderer::shapeTextLayout(ctx, *sh, &origin);
                QVERIFY(lay);
                const QPointF want = sh->rect.topLeft() + origin + lay->lineInfo(0)[0].rect.topLeft();   // origin is in the shape's frame
                QTemporaryDir dir;
                const QString path = dir.filePath(QStringLiteral("shape-text.pub"));
                QString err;
                QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
                auto back = importPublisherFile(path, &err);
                QVERIFY2(back, qPrintable(err));
                int shapes = 0, boxes = 0;
                const TextItem *box = nullptr;
                walkItems(back->pages[0]->items, [&](const ItemPtr &it) {
                    if (it->type() == ItemType::Shape) {
                        ++shapes;
                        QVERIFY2(static_cast<const ShapeItem *>(it.get())->storyId.isEmpty(), "the shape holds no text");
                    }
                    if (it->type() == ItemType::Text) {
                        ++boxes;
                        box = static_cast<const TextItem *>(it.get());
                    }
                });
                QVERIFY2(shapes == 1 && boxes == 1, qPrintable(QStringLiteral("%1 angle %2: %3 shapes, %4 text boxes").arg(kind).arg(angle).arg(shapes).arg(boxes)));
                QCOMPARE(back->storyDoc(box->storyId)->toPlainText(), QStringLiteral("Words in the shape"));
                QVERIFY2(std::abs(box->rotation - angle) < 0.5, qPrintable(QString::number(box->rotation)));
                if (angle == 0) {
                    LayoutCache cache2;
                    const auto frame = cache2.textFrame(*back, *box, 1, RenderOptions());
                    const QPointF got = box->rect.topLeft() + frame.layout->lineInfo(0)[0].rect.topLeft();
                    QVERIFY2(std::abs(got.x() - want.x()) < 0.3 && std::abs(got.y() - want.y()) < 0.3,
                             qPrintable(QStringLiteral("%1: first line at %2,%3, JeffPub sets it at %4,%5").arg(kind).arg(got.x()).arg(got.y()).arg(want.x()).arg(want.y())));
                }
            }
        }
    }

    // A text box whose frame reaches into another's text area pushes that
    // text aside, even by less than a point: in Publisher's picture of a
    // newsletter's columns, a box 0.72 points into the next one had the
    // lines beside it start at its edge plus its wrap distance. Half a point
    // pushed nothing.
    void textBoxReachingIntoTheTextAreaWraps()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        auto box = [&](const QRectF &r, const QString &text) {
            auto t = std::make_shared<TextItem>();
            t->rect = r;
            t->insets = QMarginsF(2.88, 2.88, 2.88, 2.88);
            t->storyId = doc->createStory(text);
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontPointSize(8);
            c.mergeCharFormat(f);
            doc->pages[0]->items.push_back(t);
            return t;
        };
        auto column = box(QRectF(100, 100, 200, 100), QStringLiteral("The quick brown fox jumps over the lazy dog."));
        auto beside = box(QRectF(40, 100, 63.6, 100), QStringLiteral("Beside"));   // its edge at 103.6, 0.72 into the text area
        LayoutCache cache;
        RenderOptions opt;
        auto firstX = [&] { return cache.textFrame(*doc, *column, 1, opt).layout->lineInfo(0).value(0).rect.x(); };
        // The box's edge, 103.6, and its wrap distance, 2.88, from the column's left edge: 6.48.
        QVERIFY2(std::abs(firstX() - 6.48) < 0.01, qPrintable(QString::number(firstX())));
        beside->rect.setWidth(63.38);   // 0.5 in
        QVERIFY2(std::abs(firstX() - 2.88) < 0.01, qPrintable(QString::number(firstX())));
    }

    // The margins and vertical alignment set for a shape in Format Shape are kept through a save, with no text in it: they were written only
    // for a shape that held text, so an empty shape came back with a new shape's.
    void emptyShapeKeepsItsMarginsAndAnchorWhenSaved()
    {
        Editor ed;
        ed.setDocument(Document::blank(QSizeF(612, 792)));
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(72, 72, 200, 100);
        ed.addItem(s);
        QVERIFY(s->storyId.isEmpty());
        // As the dialog sets them.
        ed.change(QStringLiteral("Format Shape"), [&] {
            s->insets = QMarginsF(1, 2, 3, 4);
            s->valign = VAlign::Bottom;
        });
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("shape.jpub"));
        QString err;
        QVERIFY2(savePublication(*ed.doc(), path, QImage(), &err), qPrintable(err));
        auto back = loadPublication(path, &err);
        QVERIFY2(back, qPrintable(err));
        const ShapeItem *again = nullptr;
        for (const auto &it : back->pages[0]->items)
            if (it->type() == ItemType::Shape) again = static_cast<const ShapeItem *>(it.get());
        QVERIFY(again);
        QVERIFY(again->storyId.isEmpty());
        QCOMPARE(again->insets, QMarginsF(1, 2, 3, 4));
        QCOMPARE(int(again->valign), int(VAlign::Bottom));
    }

    // Insert > Text > Text Art lists every style, each drawn as "Aa". Chosen, a style makes a Text Art in that style, in the middle of the
    // page a third of the way down, and asks for its words with the old ones selected.
    void textArtFromTheRibbonIsPlacedAndAsksForItsWords()
    {
        jp::MainWindow w;
        jp::GalleryButton *gallery = nullptr;
        for (auto *g : w.findChildren<jp::GalleryButton *>())
            if (g->text() == QStringLiteral("Text Art")) gallery = g;
        QVERIFY(gallery);
        const auto &styles = jp::textArtStyles();
        const QVector<jp::GalleryItem> items = gallery->items();
        QCOMPARE(items.size(), styles.size());
        QCOMPARE(items[3].id, styles[3].id);
        QCOMPARE(items[3].tip, styles[3].name);
        QVERIFY(!items[3].icon.pixmap(QSize(64, 40)).toImage().isNull());
        QString asked, selected;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            asked = d->windowTitle();
            if (auto *edit = d->findChild<QPlainTextEdit *>()) selected = edit->textCursor().selectedText();
            d->reject();
        });
        Q_EMIT gallery->activated(styles[3].id);
        QCOMPARE(asked, QStringLiteral("Edit Text Art Text"));
        auto *art = dynamic_cast<jp::TextArtItem *>(w.editor()->single());
        QVERIFY(art);
        QCOMPARE(art->styleId, styles[3].id);
        QCOMPARE(art->font, styles[3].font);
        const QSizeF ps = w.editor()->doc()->pageSize();
        QCOMPARE(art->rect, QRectF(ps.width() / 2 - 144, ps.height() / 3, 288, 72));
        QCOMPARE(selected, art->text);
        QVERIFY(!art->text.isEmpty());
        w.editor()->undo();
        QVERIFY(!w.editor()->doc()->item(art->id));
    }

    // The Text Art gallery every app with pages shares: an item for each style, drawn as "Aa" in the document's colors, and a new
    // Text Art in the style an item names (the plain style for a name no style has).
    void textArtGalleryItemsAndNewTextArt()
    {
        jp::Editor ed;
        ed.setDocument(jp::Document::blank(QSizeF(612, 792)));
        const auto &styles = jp::textArtStyles();
        const QVector<jp::GalleryItem> items = jp::textArtGalleryItems(&ed);
        QCOMPARE(items.size(), styles.size());
        QSet<QString> seen;
        for (int i = 0; i < items.size(); ++i) {
            QCOMPARE(items[i].id, styles[i].id);
            QCOMPARE(items[i].tip, styles[i].name);
            QVERIFY(!seen.contains(items[i].id));
            seen.insert(items[i].id);
            QImage shown = items[i].icon.pixmap(QSize(64, 40)).toImage().convertToFormat(QImage::Format_ARGB32);
            bool drawn = false;
            for (int y = 0; y < shown.height() && !drawn; ++y)
                for (int x = 0; x < shown.width() && !drawn; ++x) drawn = qAlpha(shown.pixel(x, y)) > 0;
            QVERIFY2(drawn, qPrintable(items[i].id));
        }
        for (const auto &style : styles) {
            const std::shared_ptr<jp::TextArtItem> art = jp::newTextArt(style.id);
            QVERIFY(art);
            QCOMPARE(art->styleId, style.id);
            QCOMPARE(art->font, style.font);
            QCOMPARE(art->bold, style.bold);
            QCOMPARE(art->transform_, style.transform);
            QCOMPARE(int(art->fill.type), int(style.fill.type));
        }
        const std::shared_ptr<jp::TextArtItem> plain = jp::newTextArt(QStringLiteral("no such style"));
        QVERIFY(plain);
        QVERIFY(plain->styleId.isEmpty());
        QCOMPARE(plain->transform_, QStringLiteral("plain"));
        QVERIFY(!plain->text.isEmpty());
    }

    // Typing: AutoCorrect fixes a word when it ends, capitalizes sentences,
    // and smart quotes curl; each can be turned off in Options.
    void typingAutoCorrect()
    {
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        jp::Editor *ed = w.editor();
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 400, 200)));
        ed->addItem(box);
        ed->beginTextEdit(box->id);
        auto type = [&](const QString &s) { for (QChar c : s) ed->typeText(QString(c)); };
        type(QStringLiteral("teh cat said \"hi\" and i didnt mind. it's THursday (c) "));
        const QString got = ed->doc()->storyDoc(box->storyId)->toPlainText();
        QCOMPARE(got, QStringLiteral("The cat said “hi” and I didn’t mind. It’s Thursday © "));
        ed->endTextEdit();
        jp::Settings::get().setValue("proof/autocorrect", false);
        jp::Settings::get().setValue("proof/smartQuotes", false);
        auto plain = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 300, 400, 200)));
        ed->addItem(plain);
        ed->beginTextEdit(plain->id);
        type(QStringLiteral("teh \"x\" "));
        QCOMPARE(ed->doc()->storyDoc(plain->storyId)->toPlainText(), QStringLiteral("teh \"x\" "));
        ed->endTextEdit();
        jp::Settings::get().setValue("proof/autocorrect", true);
        jp::Settings::get().setValue("proof/smartQuotes", true);
        // New text boxes follow "hyphenate automatically in new text boxes".
        jp::Settings::get().setValue("edit/hyphenate", false);
        QVERIFY(!std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(0, 0, 50, 50)))->hyphenate);
        jp::Settings::get().setValue("edit/hyphenate", true);
        QVERIFY(std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(0, 0, 50, 50)))->hyphenate);
    }

    // Dragging across words selects whole words; dragging selected text
    // moves it (Options > Advanced).
    void textMouseSelectAndDrag()
    {
        jp::MainWindow w;
        w.resize(1200, 900);
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        jp::Editor *ed = w.editor();
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 400, 100), QStringLiteral("alpha beta gamma delta")));
        box->hyphenate = false;
        ed->addItem(box);
        ed->beginTextEdit(box->id);
        jp::Canvas *cv = w.canvas();
        QTest::qWait(50);
        auto at = [&](int pos) {
            QTextCursor c = ed->cursor();
            c.setPosition(pos);
            ed->setCursor(c);
            return cv->caretViewRect().center().toPoint();
        };
        auto drag = [&](const QPoint &a, const QPoint &b) {
            QTest::mousePress(cv->viewport(), Qt::LeftButton, Qt::NoModifier, a);
            QMouseEvent mv(QEvent::MouseMove, QPointF(b), cv->viewport()->mapToGlobal(QPointF(b)), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(cv->viewport(), &mv);
            QTest::mouseRelease(cv->viewport(), Qt::LeftButton, Qt::NoModifier, b);
        };
        // Press in "beta", drag into "gamma": both words, whole.
        const QPoint from = at(8), to = at(13);
        drag(from, to);
        QCOMPARE(ed->cursor().selectedText(), QStringLiteral("beta gamma"));
        // Drag the selection to the end: it moves there.
        QTest::qWait(QApplication::doubleClickInterval() + 100);   // not a double click
        const QPoint grab = at(9), drop = at(22);
        QTextCursor c = ed->cursor();
        c.setPosition(6);
        c.setPosition(16, QTextCursor::KeepAnchor);
        ed->setCursor(c);
        drag(grab, drop);
        QCOMPARE(ed->doc()->storyDoc(box->storyId)->toPlainText(), QStringLiteral("alpha  deltabeta gamma"));
        QVERIFY(ed->undoStack()->canUndo());
    }

    // Autoflow: a long story continues on new pages in linked boxes.
    void autoflowNewPages()
    {
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        jp::Editor *ed = w.editor();
        QString text;
        for (int i = 0; i < 120; ++i) text += QStringLiteral("This is paragraph %1 of a long story that will not fit on one page at all.\n").arg(i + 1);
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 468, 300), text));
        ed->addItem(box);
        const int added = jp::autoflowText(ed, box->id);
        QVERIFY(added >= 2);
        QCOMPARE(int(ed->doc()->pages.size()), 1 + added);
        const auto chain = ed->doc()->chainOf(box->id);
        QCOMPARE(int(chain.size()), 1 + added);
        for (int i = 1; i < chain.size(); ++i) {
            QCOMPARE(ed->doc()->find(chain[i]->id).page, i);
            QCOMPARE(chain[i]->rect, box->rect);
        }
        jp::LayoutCache cache;
        jp::RenderOptions opt;
        QVERIFY(!cache.textFrame(*ed->doc(), *chain.last(), int(chain.size()), opt).layout->overflow());
        QCOMPARE(jp::autoflowText(ed, box->id), 0);   // nothing more to do
    }

    // A story through many linked boxes keeps its lines where they belong in
    // the last boxes too (line positions once overflowed Qt's fixed-point
    // range past the 33rd box).
    void manyLinkedFrames()
    {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat cf;
        cf.setFontFamilies(QStringList{QStringLiteral("Arial")});
        cf.setFontPointSize(11);
        for (int i = 0; i < 40; ++i) {
            if (i) c.insertBlock();
            c.insertText(QStringLiteral("Box %1").arg(i + 1), cf);
        }
        QVector<jp::FrameSpec> frames;
        for (int i = 0; i < 40; ++i) {
            jp::FrameSpec fs;
            fs.size = QSizeF(200, 20);
            fs.insets = QMarginsF(0, 0, 0, 0);
            frames << fs;
        }
        jp::StoryLayout lay;
        lay.build(&doc, frames, jp::LayoutEnv());
        QVERIFY(!lay.overflow());
        for (int f : {0, 33, 39}) {
            const auto lines = lay.lineInfo(f);
            QCOMPARE(lines.size(), 1);
            QVERIFY2(lines[0].baseline > 5 && lines[0].baseline < 20, qPrintable(QStringLiteral("box %1: %2").arg(f).arg(lines[0].baseline)));
            QCOMPARE(lines[0].text, QStringLiteral("Box %1").arg(f + 1));
        }
    }

    // Circle Text Art follows Publisher's path: clockwise from just above
    // 9 o'clock, the whole text (trailing spaces too) filling it, letters
    // straddling the frame's ellipse.
    void textArtCircle()
    {
        jp::TextArtItem w;
        w.transform_ = QStringLiteral("circle");
        w.font = QStringLiteral("Arimo");
        w.text = QStringLiteral("A") + QString(30, QLatin1Char(' '));
        const QPainterPath p = jp::textArtPath(w, QSizeF(200, 200));
        QVERIFY(!p.isEmpty());
        const QPointF c = p.boundingRect().center() - QPointF(100, 100);
        const double ang = std::fmod(qRadiansToDegrees(std::atan2(c.y(), c.x())) + 360, 360);   // clockwise from 3 o'clock
        QVERIFY2(ang > 181 && ang < 215, qPrintable(QString::number(ang)));
        const double r = std::hypot(c.x(), c.y());
        QVERIFY2(r > 96 && r < 112, qPrintable(QString::number(r)));
        // A long text goes all the way round.
        w.text = QStringLiteral("UNITED STATES SENTENCING COMMISSION");
        const QRectF all = jp::textArtPath(w, QSizeF(200, 200)).boundingRect();
        QVERIFY2(all.left() < 0 && all.top() < 0 && all.right() > 200, qPrintable(QStringLiteral("%1 %2 %3").arg(all.left()).arg(all.top()).arg(all.right())));
        // A missing AG_Futura's wide spaces leave its letters less of the circle.
        if (!QFontDatabase::hasFamily(QStringLiteral("AG_Futura"))) {
            auto span = [&](const QString &font) {
                w.font = font;
                w.text = QStringLiteral("AB") + QString(10, QLatin1Char(' '));
                const QPainterPath q = jp::textArtPath(w, QSizeF(200, 200));
                double lo = 360, hi = 0;
                for (const QPolygonF &poly : q.toSubpathPolygons())
                    for (const QPointF &pt : poly) {
                        const double a = std::fmod(qRadiansToDegrees(std::atan2(pt.y() - 100, pt.x() - 100)) + 360 - 170, 360);
                        lo = std::min(lo, a);
                        hi = std::max(hi, a);
                    }
                return hi - lo;
            };
            QVERIFY(span(QStringLiteral("AG_Futura")) < span(QStringLiteral("Jost")) * 0.85);
        }
    }

    // AutoFormat as you type: dashes, fractions, ordinals and lists.
    void typingAutoFormat()
    {
        jp::MainWindow w;
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        jp::Editor *ed = w.editor();
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 400, 300)));
        ed->addItem(box);
        ed->beginTextEdit(box->id);
        auto type = [&](const QString &s) { for (QChar c : s) ed->typeText(QString(c)); };
        type(QStringLiteral("It was 1/2 done--almost the 1st try - really "));
        QTextDocument *doc = ed->doc()->storyDoc(box->storyId);
        QCOMPARE(doc->toPlainText(), QString::fromUtf8("It was \u00BD done\u2014almost the 1st try \u2013 really "));
        // The ordinal's letters are raised.
        const int st = int(doc->toPlainText().indexOf(QStringLiteral("1st"))) + 1;
        QTextCursor c(doc);
        c.setPosition(st + 1);
        QCOMPARE(c.charFormat().verticalAlignment(), QTextCharFormat::AlignSuperScript);
        // Lists.
        auto list = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 400, 400, 200)));
        ed->endTextEdit();
        ed->addItem(list);
        ed->beginTextEdit(list->id);
        type(QStringLiteral("* apples "));
        QTextDocument *ld = ed->doc()->storyDoc(list->storyId);
        QVERIFY(ld->begin().textList());
        QVERIFY(jp::isBulletList(ld->begin().textList()->format().style()));
        QCOMPARE(ld->begin().text(), QStringLiteral("Apples "));   // and AutoCorrect capitalizes the item
        ed->endTextEdit();
        auto nums = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 600, 400, 100)));
        ed->addItem(nums);
        ed->beginTextEdit(nums->id);
        type(QStringLiteral("3) third"));
        QTextBlock nb = ed->doc()->storyDoc(nums->storyId)->begin();
        QVERIFY(nb.textList());
        QVERIFY(!jp::isBulletList(nb.textList()->format().style()));
        QCOMPARE(nb.textList()->format().numberSuffix(), QStringLiteral(")"));
        QCOMPARE(nb.textList()->format().start(), 3);
    }

    // A long publication keeps all its text: .pub text past 64 KB of the
    // text stream used to wrap libmspub's 16-bit run positions, so every
    // story after the first ~32,000 characters opened empty.
    void longTextSurvivesPubRoundTrip()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        QStringList expected;
        for (int i = 0; i < 4; ++i) {
            if (i) doc->pages.push_back(std::make_shared<Page>());
            auto t = std::make_shared<TextItem>();
            t->rect = QRectF(36, 36, 540, 720);
            QString text;
            while (text.size() < 12000) text += QStringLiteral("Story %1 keeps every word of its text. ").arg(i + 1);
            text += QStringLiteral("End of story %1.").arg(i + 1);
            t->storyId = doc->createStory(text);
            expected << text;
            doc->pages[i]->items.push_back(t);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("long.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(back->pages.size(), 4);
        for (int i = 0; i < 4; ++i) {
            auto *t = dynamic_cast<TextItem *>(back->pages[i]->items.empty() ? nullptr : back->pages[i]->items[0].get());
            QVERIFY(t);
            const QString got = back->storyDoc(t->storyId)->toPlainText().remove(QChar(0x00AD));
            QVERIFY2(got.endsWith(QStringLiteral("End of story %1.").arg(i + 1)), qPrintable(QStringLiteral("story %1: %2 characters").arg(i + 1).arg(got.size())));
            QCOMPARE(got.size(), expected[i].size());
        }
    }

    // A vertical text box (a book's spine) saves as one: text flow 1 (top
    // to bottom), as Publisher writes it, not letters stacked in a tall box.
    void verticalTextBoxSavesToPub()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        auto spine = std::make_shared<TextItem>();
        spine->rect = QRectF(290, 36, 32, 700);
        spine->vertical = true;
        spine->storyId = doc->createStory(QStringLiteral("UNITED STATES BANKRUPTCY CODE"));
        doc->pages[0]->items.push_back(spine);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("spine.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        auto *t = dynamic_cast<TextItem *>(back->pages[0]->items[0].get());
        QVERIFY(t);
        QVERIFY(t->vertical);
        QCOMPARE(t->rotation, 0.0);
        QVERIFY(std::abs(t->rect.width() - 32) < 0.5 && std::abs(t->rect.height() - 700) < 0.5);
        // Its frame layout record is turned: width 700 points, height 32.
        const QByteArray q = cfb::readStream(path, QStringLiteral("Quill/QuillSub/CONTENTS"));
        auto field = [](int id, quint32 v) {
            QByteArray b;
            b.append(char(id)).append(char(0x22));
            for (int k = 0; k < 4; ++k) b.append(char((v >> (8 * k)) & 0xff));
            return b;
        };
        QVERIFY(q.contains(field(0x04, 700 * 12700) + field(0x05, 32 * 12700)));
        // And its shape chunk says so (34 = 2), or Publisher stacks the letters.
        QVERIFY(cfb::readStream(path, QStringLiteral("Contents")).contains(QByteArray("\x34\x20\x02\x00\x00\x00", 6)));
    }

    // Connectors: a line's ends attach to connection sites (the middle of
    // each side: 0 top, 1 left, 2 bottom, 3 right) and follow the objects;
    // an elbow runs level or upright from each end as the site faces.
    void connectorsFollowObjects()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        auto a = std::make_shared<ShapeItem>(), b = std::make_shared<ShapeItem>();
        a->rect = QRectF(72, 72, 100, 60);
        b->rect = QRectF(300, 250, 100, 60);
        auto l = std::make_shared<LineItem>();
        l->route = LineItem::Elbow;
        l->start = LineItem::Glue{a->id, 3};
        l->end = LineItem::Glue{b->id, 0};
        doc->pages[0]->items = {a, b, l};
        QVERIFY(doc->routeConnectors());
        QCOMPARE(l->p1, QPointF(172, 102));
        QCOMPARE(l->p2, QPointF(350, 250));
        QVERIFY(!l->startVertical && l->endVertical);
        // Right side to top: out level, one corner, down into the top.
        QCOMPARE(l->routePoints(), (QVector<QPointF>{{172, 102}, {350, 102}, {350, 250}}));
        // The objects move and turn; the ends go with them.
        b->moveBy(40, 100);
        a->rotation = 90;
        QVERIFY(doc->routeConnectors());
        QCOMPARE(l->p2, QPointF(390, 350));
        QVERIFY(std::abs(l->p1.x() - 122) < 1e-9 && std::abs(l->p1.y() - 152) < 1e-9);
        QVERIFY(l->startVertical);   // the right side now faces down
        QVERIFY(!doc->routeConnectors());
        // Saved and opened again.
        LineItem back;
        back.fromJson(l->toJson());
        QCOMPARE(back.route, LineItem::Elbow);
        QVERIFY(back.start == l->start && back.end == l->end);
        // Copies of both objects and the line stay attached to each other; a
        // copy of the line alone comes loose.
        const ItemList copies = doc->cloneItems(doc->pages[0]->items);
        auto *cl = static_cast<LineItem *>(copies[2].get());
        QCOMPARE(cl->start.id, copies[0]->id);
        QCOMPARE(cl->end.id, copies[1]->id);
        QVERIFY(static_cast<LineItem *>(doc->cloneItem(*l).get())->start.id.isEmpty());
        // An object deleted: its end comes loose and stays where it was.
        doc->pages[0]->items = {a, l};
        QVERIFY(doc->routeConnectors());
        QVERIFY(l->end.id.isEmpty());
        QCOMPARE(l->p2, QPointF(390, 350));
    }

    // Drawing a connector from one shape's side to another's attaches it;
    // moving a shape takes the connector's end along; dragging the connector
    // away on its own detaches it.
    void connectorDrawnBetweenShapes()
    {
        jp::MainWindow w;
        w.resize(1200, 900);
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        jp::Editor *ed = w.editor();
        auto a = std::make_shared<jp::ShapeItem>(), b = std::make_shared<jp::ShapeItem>();
        a->rect = QRectF(72, 72, 100, 60);
        b->rect = QRectF(300, 250, 100, 60);
        ed->addItem(a);
        ed->addItem(b);
        jp::Canvas *c = w.canvas();
        QWidget *vp = c->viewport();
        auto view = [&](QPointF page) { return c->pageToView(page).toPoint(); };
        auto move = [&](QPointF page) {
            const QPointF v = view(page);
            QMouseEvent mv(QEvent::MouseMove, v, vp->mapToGlobal(v), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
            QApplication::sendEvent(vp, &mv);
        };
        auto drag = [&](QPointF from, QPointF to) {
            QTest::qWait(QApplication::doubleClickInterval() + 50);
            QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, view(from));
            move((from + to) / 2);
            move(to);
            QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, view(to));
        };
        // Drawn from near A's right side to near B's left: the ends snap there.
        ed->setTool(jp::Tool::Arrow, QStringLiteral("elbow"));
        drag(QPointF(174, 104), QPointF(298, 278));
        const auto &items = ed->doc()->pages[0]->items;
        QCOMPARE(int(items.size()), 3);
        auto line = std::dynamic_pointer_cast<jp::LineItem>(items[2]);
        QVERIFY(line);
        QCOMPARE(line->route, jp::LineItem::Elbow);
        QCOMPARE(line->start, (jp::LineItem::Glue{a->id, 3}));
        QCOMPARE(line->end, (jp::LineItem::Glue{b->id, 1}));
        QCOMPARE(line->p1, QPointF(172, 102));
        QCOMPARE(line->p2, QPointF(300, 280));
        QCOMPARE(line->stroke.endArrow, jp::Arrow::Triangle);
        // B dragged down: the line's end goes with it.
        ed->setTool(jp::Tool::Select);
        drag(QPointF(370, 290), QPointF(370, 326));
        jp::Item *bNow = ed->doc()->item(b->id);
        QVERIFY(bNow && bNow->rect.top() > 270);
        line = std::dynamic_pointer_cast<jp::LineItem>(ed->doc()->itemPtr(line->id));
        QCOMPARE(line->p2, jp::connectionSite(*bNow, 1));
        QCOMPARE(line->p1, QPointF(172, 102));
        // The line dragged by its middle on its own: it comes loose.
        const QVector<QPointF> rp = line->routePoints();
        const QPointF mid = (rp[1] + rp[2]) / 2 + QPointF(0, 10);
        drag(mid, mid + QPointF(30, 0));
        line = std::dynamic_pointer_cast<jp::LineItem>(ed->doc()->itemPtr(line->id));
        QVERIFY(line->start.id.isEmpty() && line->end.id.isEmpty());
        QVERIFY(std::abs(line->p1.x() - 202) < 1.5 && std::abs(line->p1.y() - 102) < 1e-9);
        // Undo puts it back, attached.
        ed->undo();
        line = std::dynamic_pointer_cast<jp::LineItem>(ed->doc()->itemPtr(line->id));
        QCOMPARE(line->start.id, a->id);
    }

    // Regroup puts the objects of the group last ungrouped back together.
    // Deleting an object keeps the stories text refers to: a footnote's (the
    // field "footnote:<story>") and the text of a text box set in text.
    // Both counted as unused and were dropped with any deletion.
    void deleteKeepsStoriesTextRefersTo()
    {
        using namespace jp;
        Editor ed;
        ed.setDocument(Document::blank(QSizeF(612, 792)));
        Document *d = ed.doc();
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(72, 72, 300, 200);
        t->storyId = d->createStory(QStringLiteral("Text"));
        const QString note = d->createStory(QStringLiteral("A note."));
        const QString inner = d->createStory(QStringLiteral("Inner."));
        QTextCursor c(d->storyDoc(t->storyId));
        c.movePosition(QTextCursor::End);
        QTextCharFormat ff;
        ff.setProperty(tp::Field, QStringLiteral("footnote:") + note);
        c.insertText(QString(QChar::ObjectReplacementCharacter), ff);
        TextItem box;
        box.rect = QRectF(0, 0, 50, 20);
        box.storyId = inner;
        QTextCharFormat of;
        of.setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(box.toJson()).toJson(QJsonDocument::Compact)));
        c.insertText(QString(QChar::ObjectReplacementCharacter), of);
        ed.addItem(t, false);
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(400, 400, 50, 50);
        ed.addItem(s);
        ed.deleteItems({s->id});
        QVERIFY(d->storyDoc(t->storyId));
        QVERIFY2(d->storyDoc(note), "the footnote's text was dropped");
        QVERIFY2(d->storyDoc(inner), "the text of the box set in text was dropped");
    }

    // Wrap Text > In Line with Text: a shape over a text box goes into its
    // text where its top left is (the start, here). Selected in the text,
    // Square brings it back onto the page where the text showed it. Both
    // undo.
    void moveObjectIntoAndOutOfText()
    {
        using namespace jp;
        Editor ed;
        ed.setDocument(Document::blank(QSizeF(612, 792)));
        ItemPtr t = ed.newTextBox(QRectF(72, 72, 300, 200), QStringLiteral("Hello world"));
        ed.addItem(t, false);
        const QString sid = static_cast<TextItem *>(t.get())->storyId;
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(70, 70, 30, 20);
        ed.addItem(s);
        QVERIFY(ed.canMoveIntoText());
        QVERIFY(ed.moveIntoText());
        QCOMPARE(int(ed.doc()->pages[0]->items.size()), 1);
        const QString inText = QString(QChar::ObjectReplacementCharacter) + QStringLiteral("Hello world");
        QCOMPARE(ed.doc()->storyDoc(sid)->toPlainText(), inText);
        ed.beginTextEdit(t->id, 0);
        QTextCursor c = ed.cursor();
        c.setPosition(0);
        c.setPosition(1, QTextCursor::KeepAnchor);
        ed.setCursor(c);
        QVERIFY(ed.selectionIsInlineObject());
        QVERIFY(ed.moveOutOfText(Wrap::Square));
        QCOMPARE(int(ed.doc()->pages[0]->items.size()), 2);
        QCOMPARE(ed.doc()->storyDoc(sid)->toPlainText(), QStringLiteral("Hello world"));
        const Item *out = ed.doc()->pages[0]->items[1].get();
        QVERIFY(out->type() == ItemType::Shape && out->wrap.mode == Wrap::Square);
        QVERIFY(std::abs(out->rect.width() - 30) < 0.01 && std::abs(out->rect.height() - 20) < 0.01);
        // At the start of the first line: the box's inset, then the object's left wrap distance.
        QVERIFY2(std::abs(out->rect.left() - (72 + 2.88 + 2.88)) < 0.5, qPrintable(QString::number(out->rect.left())));
        ed.undo();
        QCOMPARE(int(ed.doc()->pages[0]->items.size()), 1);
        QCOMPARE(ed.doc()->storyDoc(sid)->toPlainText(), inText);
        ed.undo();
        QCOMPARE(int(ed.doc()->pages[0]->items.size()), 2);
        QCOMPARE(ed.doc()->storyDoc(sid)->toPlainText(), QStringLiteral("Hello world"));
    }

    void regroupAfterUngroup()
    {
        using namespace jp;
        Editor ed;
        ed.setDocument(Document::blank(QSizeF(612, 792)));
        QStringList ids;
        for (int i = 0; i < 3; ++i) {
            auto s = std::make_shared<ShapeItem>();
            s->rect = QRectF(72 + 100 * i, 72, 72, 72);
            ed.addItem(s);
            ids << s->id;
        }
        QVERIFY(!ed.canRegroup());
        ed.select(ids);
        ed.groupSelection();
        QCOMPARE(int(ed.doc()->pages[0]->items.size()), 1);
        ed.ungroupSelection();
        QCOMPARE(int(ed.doc()->pages[0]->items.size()), 3);
        ed.clearSelection();
        QVERIFY(ed.canRegroup());
        ed.regroup();
        QCOMPARE(int(ed.doc()->pages[0]->items.size()), 1);
        auto *g = dynamic_cast<GroupItem *>(ed.doc()->pages[0]->items[0].get());
        QVERIFY(g && g->children.size() == 3);
        QCOMPARE(ed.selection(), QStringList{g->id});
        ed.undo();
        QCOMPARE(int(ed.doc()->pages[0]->items.size()), 3);
    }

    // Shapes > Lines: Curve (clicks, a smooth line through them, double-click
    // to end), Freeform (clicks joined straight; clicking the first point
    // closes it) and Scribble (one stroke by hand).
    void freehandDrawingTools()
    {
        jp::MainWindow w;
        w.resize(1200, 900);
        w.editor()->setDocument(jp::Document::blank(QSizeF(612, 792)));
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        jp::Editor *ed = w.editor();
        jp::Canvas *c = w.canvas();
        QWidget *vp = c->viewport();
        auto view = [&](QPointF page) { return c->pageToView(page).toPoint(); };
        auto click = [&](QPointF page) {
            QTest::qWait(QApplication::doubleClickInterval() + 50);
            QTest::mouseClick(vp, Qt::LeftButton, Qt::NoModifier, view(page));
        };
        auto move = [&](QPointF page, Qt::MouseButtons held) {
            const QPointF v = view(page);
            QMouseEvent mv(QEvent::MouseMove, v, vp->mapToGlobal(v), Qt::NoButton, held, Qt::NoModifier);
            QApplication::sendEvent(vp, &mv);
        };
        const auto &items = ed->doc()->pages[0]->items;
        auto last = [&] { return std::dynamic_pointer_cast<jp::ShapeItem>(items.back()); };
        auto passesNear = [](const QPainterPath &path, QPointF pt) {
            for (double t = 0; t <= 1.0; t += 0.002)
                if (QLineF(path.pointAtPercent(t), pt).length() < 1.5) return true;
            return false;
        };
        // A curve through three points, ended with a double click on the third.
        ed->setTool(jp::Tool::Freeform, QStringLiteral("curve"));
        click(QPointF(100, 100));
        click(QPointF(200, 160));
        click(QPointF(300, 100));
        QTest::mouseDClick(vp, Qt::LeftButton, Qt::NoModifier, view(QPointF(300, 100)));
        QCOMPARE(int(items.size()), 1);
        auto curve = last();
        QVERIFY(curve && curve->fill.isNone());
        QCOMPARE(ed->tool(), jp::Tool::Select);
        const QPainterPath cp = curve->customPath.translated(curve->rect.topLeft());
        QVERIFY(cp.elementAt(1).type == QPainterPath::CurveToElement);
        for (QPointF pt : {QPointF(100, 100), QPointF(200, 160), QPointF(300, 100)}) QVERIFY(passesNear(cp, pt));
        // A freeform triangle closed on its first point: filled.
        ed->setTool(jp::Tool::Freeform, QStringLiteral("freeform"));
        click(QPointF(100, 300));
        click(QPointF(250, 300));
        click(QPointF(175, 400));
        click(QPointF(100.5, 300.5));
        QCOMPARE(int(items.size()), 2);
        auto tri = last();
        QVERIFY(!tri->fill.isNone());
        QVERIFY(std::abs(tri->rect.width() - 150) < 1.5 && std::abs(tri->rect.height() - 100) < 1.5);
        // A scribble: one stroke.
        ed->setTool(jp::Tool::Freeform, QStringLiteral("scribble"));
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, view(QPointF(100, 500)));
        for (int i = 1; i <= 20; ++i) move(QPointF(100 + i * 10, 500 + (i % 2 ? 15 : -15)), Qt::LeftButton);
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, view(QPointF(300, 485)));
        QCOMPARE(int(items.size()), 3);
        auto scribble = last();
        QVERIFY(scribble->fill.isNone());
        QVERIFY(scribble->customPath.elementCount() >= 15);
        // Saved to .pub and opened again: the open lines stay open and unfilled.
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("free.pub"));
        QString err;
        QVERIFY2(jp::exportPublisher(*ed->doc(), path, &err), qPrintable(err));
        auto back = jp::importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages[0]->items.size()), 3);
        auto *bc = dynamic_cast<jp::ShapeItem *>(back->pages[0]->items[0].get());
        QVERIFY(bc && bc->fill.isNone());
        QVERIFY(passesNear(bc->customPath.translated(bc->rect.topLeft()), QPointF(200, 160)));
        QVERIFY(!static_cast<jp::ShapeItem *>(back->pages[0]->items[1].get())->fill.isNone());
    }

    // Connectors in .pub: elbow and curved routes are Publisher's connector
    // shapes, turned and flipped so they start at the line's start, and the
    // drawing's connector rules keep them attached (checked by opening the
    // file in Publisher and moving a shape: the connectors followed it).
    void connectorsSaveToPub()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        auto a = std::make_shared<ShapeItem>(), b = std::make_shared<ShapeItem>();
        a->rect = QRectF(72, 72, 100, 60);
        b->rect = QRectF(300, 250, 100, 60);
        doc->pages[0]->items = {a, b};
        const struct { LineItem::Route route; int from, to; } specs[] = {
            {LineItem::Elbow, 3, 1}, {LineItem::Elbow, 2, 0}, {LineItem::Curved, 3, 1}, {LineItem::Curved, 2, 1}, {LineItem::Straight, 3, 1}};
        for (const auto &sp : specs) {
            auto l = std::make_shared<LineItem>();
            l->route = sp.route;
            l->start = LineItem::Glue{a->id, sp.from};
            l->end = LineItem::Glue{b->id, sp.to};
            l->stroke.endArrow = Arrow::Triangle;
            doc->pages[0]->items.push_back(l);
        }
        static_cast<LineItem *>(doc->pages[0]->items[2].get())->bend = 0.25;
        doc->routeConnectors();
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("conn.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        const QByteArray escher = cfb::readStream(path, QStringLiteral("Escher/EscherStm"));
        // Five rules in a solver container; the bend as 5400 of 21600.
        QVERIFY(escher.contains(QByteArray("\x5f\x00\x05\xf0", 4)));
        QVERIFY(escher.contains(QByteArray("\x47\x01\x18\x15\x00\x00", 6)));
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QVector<LineItem *> lines;
        QString aId, bId;
        for (const auto &it : back->pages[0]->items) {
            if (auto *l = dynamic_cast<LineItem *>(it.get())) lines << l;
            else if (it->rect.left() < 100) aId = it->id;
            else bId = it->id;
        }
        QCOMPARE(lines.size(), 5);
        for (int i = 0; i < 5; ++i) {
            QCOMPARE(lines[i]->route, specs[i].route);
            QCOMPARE(lines[i]->start, (LineItem::Glue{aId, specs[i].from}));
            QCOMPARE(lines[i]->end, (LineItem::Glue{bId, specs[i].to}));
            QCOMPARE(lines[i]->stroke.endArrow, Arrow::Triangle);
        }
        QCOMPARE(lines[0]->bend, 0.25);
        QVERIFY(lines[1]->startVertical && lines[1]->endVertical);
        QVERIFY(lines[3]->startVertical && !lines[3]->endVertical);
    }

    // Text wrapping in .pub: the shape record's field 04 low byte (0 none, 2
    // tight, 3 through, 4 top and bottom; left out for square), and the
    // distances in the drawing. Agrees with the wrapping Publisher reports for
    // 1,330 of the 1,342 objects in 284 publications.
    void pubWrapSetting()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        const Wrap::Mode modes[] = {Wrap::None, Wrap::Square, Wrap::Tight, Wrap::Through, Wrap::TopBottom};
        for (int i = 0; i < 5; ++i) {
            auto s = std::make_shared<ShapeItem>();
            s->rect = QRectF(72 + 90 * i, 72, 72, 72);
            s->wrap.mode = modes[i];
            s->wrap.left = 9 + i;
            doc->pages[0]->items.push_back(s);
        }
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("wrap.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages[0]->items.size()), 5);
        for (int i = 0; i < 5; ++i) {
            const Item *it = back->pages[0]->items[i].get();
            QCOMPARE(int(it->wrap.mode), int(modes[i]));
            QVERIFY(std::abs(it->wrap.left - (9 + i)) < 0.01);
            QVERIFY(std::abs(it->wrap.top - 2.88) < 0.01);
        }
    }

    // Wrapping counts a line's text (ascent and descent), not the spacing
    // below it: in Publisher a 48-point line keeps its full width while an
    // object starts below the bottom of its text, even inside the line's
    // spacing. And centered text wraps where it ends up: a line centered
    // below a picture stays whole.
    void wrapAroundObjectsLikePublisher()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(72, 72, 400, 300);
        t->insets = QMarginsF(0, 0, 0, 0);
        t->storyId = doc->createStory(QStringLiteral("Wide words here and more words follow on and on and on."));
        QTextCursor c(doc->storyDoc(t->storyId));
        c.select(QTextCursor::Document);
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("Tinos")});
        cf.setFontPointSize(48);
        c.mergeCharFormat(cf);
        QTextBlockFormat bf;
        bf.setLineHeight(130, QTextBlockFormat::ProportionalHeight);
        c.mergeBlockFormat(bf);
        auto pic = std::make_shared<ShapeItem>();
        doc->pages[0]->items = {t, pic};
        auto firstLineWidth = [&](double objectTop) {
            pic->rect = QRectF(300, objectTop + 2.88, 100, 40);
            LayoutCache cache;
            const auto fl = cache.textFrame(*doc, *t, 1, RenderOptions());
            return fl.layout->lineInfo(0).value(0).rect.width();
        };
        const double whole = firstLineWidth(400);
        // The text's bottom is about 53 points (1.107 em) below the top; the
        // line, with its spacing, about 62.
        QVERIFY(firstLineWidth(72 + 56) >= whole - 0.5);
        QVERIFY(firstLineWidth(72 + 40) < whole - 50);
        // Centered text below an object it would meet at the top of the box.
        t->valign = VAlign::Middle;
        doc->storyDoc(t->storyId)->setPlainText(QStringLiteral("CENTERED"));
        QTextCursor c2(doc->storyDoc(t->storyId));
        c2.select(QTextCursor::Document);
        cf.setFontPointSize(20);
        c2.mergeCharFormat(cf);
        pic->rect = QRectF(150, 72, 100, 60);
        LayoutCache cache;
        const auto fl = cache.textFrame(*doc, *t, 1, RenderOptions());
        const auto info = fl.layout->lineInfo(0);
        QCOMPARE(int(info.size()), 1);
        QVERIFY2(info[0].rect.left() < 10, qPrintable(QString::number(info[0].rect.left())));   // not pushed right of the object
    }

    // An object set in the text sits on the baseline inside its wrap
    // distances (2.88 pt), and one taller than the text's ascent lowers its
    // line by the difference; the next line keeps its usual distance.
    // Publisher (Oct 7), 24 pt wide boxes 6, 18 and 72 pt tall in 12 pt Times
    // New Roman, single spaced: line 2's baseline 2.64 + 2.88 + h + 2.88
    // below line 1's, the box 2.88 above it, line 3 12.72 below.
    void inlineObjectInLine()
    {
        using namespace jp;
        for (double h : {6.0, 18.0, 72.0}) {
            auto doc = Document::blank(QSizeF(612, 792));
            auto t = std::make_shared<TextItem>();
            t->rect = QRectF(72, 72, 120, 220);
            t->insets = QMarginsF(0, 0, 0, 0);
            t->storyId = doc->createStory(QStringLiteral("HxH\nHxH\nHxH"));
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("Times New Roman")});
            cf.setFontPointSize(12);
            c.mergeCharFormat(cf);
            QTextBlockFormat bf;
            bf.setTopMargin(0);
            bf.setBottomMargin(0);
            bf.setLineHeight(100, QTextBlockFormat::ProportionalHeight);
            c.mergeBlockFormat(bf);
            ShapeItem box;
            box.rect = QRectF(400, 600, 24, h);
            c.setPosition(5);   // after line 2's "H"
            QTextCharFormat of = c.charFormat();
            of.setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(box.toJson()).toJson(QJsonDocument::Compact)));
            c.insertText(QString(QChar::ObjectReplacementCharacter), of);
            doc->pages[0]->items = {t};
            LayoutCache cache;
            const auto fl = cache.textFrame(*doc, *t, 1, RenderOptions());
            const auto info = fl.layout->lineInfo(0);
            QCOMPARE(int(info.size()), 3);
            const auto objects = fl.layout->inlineObjects();
            QCOMPARE(int(objects.size()), 1);
            const QRectF r = objects[0].rect;
            const double b1 = info[0].baseline, b2 = info[1].baseline, b3 = info[2].baseline;
            const double normal = b3 - b2, descent = normal - b1;
            QVERIFY2(std::abs(r.height() - h) < 0.01 && std::abs(r.width() - 24) < 0.01, qPrintable(QStringLiteral("%1x%2").arg(r.width()).arg(r.height())));
            QVERIFY2(std::abs(r.bottom() - (b2 - 2.88)) < 0.05, qPrintable(QStringLiteral("h %1: bottom %2 baseline %3").arg(h).arg(r.bottom()).arg(b2)));
            QVERIFY2(std::abs((b2 - b1) - (descent + 2.88 + h + 2.88)) < 0.05,
                     qPrintable(QStringLiteral("h %1: b1 %2 b2 %3 b3 %4").arg(h).arg(b1).arg(b2).arg(b3)));
            QVERIFY(normal > 10 && normal < 16);
            // Across: after line 2's "H" (document position 4), inside the
            // side distances.
            const QVector<QRectF> hRects = fl.layout->rangeRects(0, 4, 5);
            QVERIFY(!hRects.isEmpty());
            QVERIFY2(std::abs(r.left() - (hRects.first().right() + 2.88)) < 0.05, qPrintable(QStringLiteral("%1 after %2").arg(r.left()).arg(hRects.first().right())));
            QCOMPARE(objects[0].docPos, 5);
            // The display text keeps one character for it, which isn't drawn.
            QCOMPARE(info[1].text.size(), 4);
        }
    }

    // An object set in text, saved as Publisher saves one: its U+FFFC's run
    // has 00 = 2, the story's EOBJ section gives its position and number,
    // the object sits on the last special page with that number (0f), and
    // an index (0x70, field 05 of the text index) ties number, story and
    // object together. Publisher showed such a copy exactly as its own file
    // (Oct 7); the object comes back with its size and wrap distances.
    void pubInlineObjectRoundTrip()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(72, 72, 200, 200);
        t->storyId = doc->createStory(QStringLiteral("Logo: here"));
        ShapeItem box;
        box.rect = QRectF(300, 500, 36, 24);
        box.fill = Fill::solid(ColorRef::rgb(Qt::red));
        box.wrap.left = 5;
        QTextCursor c(doc->storyDoc(t->storyId));
        c.setPosition(6);
        QTextCharFormat of = c.charFormat();
        of.setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(box.toJson()).toJson(QJsonDocument::Compact)));
        c.insertText(QString(QChar::ObjectReplacementCharacter), of);
        doc->pages[0]->items = {t};
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("inline.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        const QByteArray q = cfb::readStream(path, QStringLiteral("Quill/QuillSub/CONTENTS"));
        // EOBJ 0 (the first story): one object at 6, the story 12 long (with
        // its paragraph mark), number 1.
        QByteArray eobj;
        for (quint32 v : {1u, 4u, 0xff00u, 6u, 12u, 1u}) eobj.append(reinterpret_cast<const char *>(&v), 4);
        QVERIFY(q.contains(eobj));
        // The object's run: 00 (type 0x12) = 2.
        QVERIFY(q.contains(QByteArray::fromHex("001202000c22")));
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(back->pages[0]->items.size()), 1);   // the object isn't on the page
        auto *bt = dynamic_cast<TextItem *>(back->pages[0]->items[0].get());
        QVERIFY(bt);
        QTextDocument *sd = back->storyDoc(bt->storyId);
        QCOMPARE(sd->toPlainText(), QStringLiteral("Logo: ") + QChar(QChar::ObjectReplacementCharacter) + QStringLiteral("here"));
        QTextCursor bc(sd);
        bc.setPosition(7);
        const ItemPtr obj = Item::fromJsonAny(QJsonDocument::fromJson(bc.charFormat().stringProperty(tp::InlineObject).toUtf8()).object());
        QVERIFY(obj && obj->type() == ItemType::Shape);
        QVERIFY(std::abs(obj->rect.width() - 36) < 0.01 && std::abs(obj->rect.height() - 24) < 0.01);
        QVERIFY(std::abs(obj->wrap.left - 5) < 0.01);
        QCOMPARE(static_cast<const ShapeItem *>(obj.get())->fill.color.resolve(back->colors), QColor(Qt::red));
        // JeffPub's own files keep it too.
        const QString jpath = dir.filePath(QStringLiteral("inline.jpub"));
        QVERIFY2(savePublication(*doc, jpath, QImage(), &err), qPrintable(err));
        auto again = loadPublication(jpath, &err);
        QVERIFY2(again, qPrintable(err));
        auto *jt = dynamic_cast<TextItem *>(again->pages[0]->items[0].get());
        QVERIFY(jt);
        QTextCursor jc(again->storyDoc(jt->storyId));
        jc.setPosition(7);
        QCOMPARE(jc.charFormat().stringProperty(tp::InlineObject), of.stringProperty(tp::InlineObject));

        // A damaged EOBJ section (its index entry and count claim more than
        // the stream holds) loses the object, never the publication's text.
        QFile in(path);
        QVERIFY(in.open(QIODevice::ReadOnly));
        cfb::File file;
        QVERIFY(cfb::read(in.readAll(), &file));
        in.close();
        QByteArray quill = file.stream(QStringLiteral("Quill/QuillSub/CONTENTS"));
        const qsizetype entry = quill.indexOf("EOBJ") - 2;   // the section's index entry
        QVERIFY(entry > 0 && entry < 512);
        const quint32 eobjAt = qFromLittleEndian<quint32>(quill.constData() + entry + 16);
        qToLittleEndian<quint32>(0x00ffffff, quill.data() + entry + 20);   // its length
        qToLittleEndian<quint32>(100000, quill.data() + eobjAt);            // its count
        QVERIFY(file.setStream(QStringLiteral("Quill/QuillSub/CONTENTS"), quill));
        const QString damaged = dir.filePath(QStringLiteral("damaged.pub"));
        QFile out(damaged);
        QVERIFY(out.open(QIODevice::WriteOnly));
        out.write(cfb::write(file));
        out.close();
        auto rescued = importPublisherFile(damaged, &err);
        QVERIFY2(rescued, qPrintable(err));
        QVERIFY(!rescued->pages[0]->items.empty());
        auto *rt = dynamic_cast<TextItem *>(rescued->pages[0]->items.front().get());
        QVERIFY(rt);
        QCOMPARE(rescued->storyDoc(rt->storyId)->toPlainText(), QStringLiteral("Logo: here"));
    }

    // A text box set in text shows its own text (it's on no page, so it had
    // no chain of boxes, and its text was never laid out).
    void textBoxInTextShowsItsText()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        auto host = std::make_shared<TextItem>();
        host->rect = QRectF(72, 72, 300, 300);
        host->storyId = doc->createStory(QStringLiteral("Before "));
        TextItem inner;
        inner.rect = QRectF(0, 0, 150, 40);
        inner.storyId = doc->createStory(QStringLiteral("Inner words"));
        QTextCursor c(doc->storyDoc(host->storyId));
        c.movePosition(QTextCursor::End);
        QTextCharFormat of;
        of.setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(inner.toJson()).toJson(QJsonDocument::Compact)));
        c.insertText(QString(QChar::ObjectReplacementCharacter), of);
        doc->pages[0]->items = {host};
        LayoutCache cache;
        const auto fl = cache.textFrame(*doc, inner, 1, RenderOptions());
        QVERIFY(fl.layout && fl.frame == 0);
        const auto lines = fl.layout->lineInfo(0);
        QVERIFY(!lines.isEmpty());
        QCOMPARE(QString(lines.first().text).remove(QChar(0x00AD)).trimmed(), QStringLiteral("Inner words"));
    }

    // Objects in text from a damaged or crafted file: a text box set in its
    // own story (drawing it drew it again, without end, and saving it kept
    // adding it), and sizes that aren't numbers or are absurd (they reached
    // Qt's fixed-point line positions).
    void inlineObjectsFromBadFilesStaySafe()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(72, 72, 300, 300);
        t->storyId = doc->createStory(QStringLiteral("Self: "));
        // First in its story, with no insets or wrap distances, so the box
        // fits in itself at every level.
        TextItem self;
        self.rect = QRectF(0, 0, 100, 40);
        self.insets = QMarginsF();
        self.wrap.left = self.wrap.top = self.wrap.right = self.wrap.bottom = 0;
        self.storyId = t->storyId;
        QTextCursor c(doc->storyDoc(t->storyId));
        QTextCharFormat of;
        of.setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(self.toJson()).toJson(QJsonDocument::Compact)));
        c.insertText(QString(QChar::ObjectReplacementCharacter), of);
        c.movePosition(QTextCursor::End);
        // Absurd and not-a-number sizes.
        for (const char *json : {"{\"type\":\"shape\",\"rect\":[0,0,1e300,1e300]}", "{\"type\":\"shape\",\"rect\":[0,0,-50,-50]}",
                                 "{\"type\":\"shape\",\"rect\":[0,0,100,100],\"wrap\":{\"t\":1e300,\"b\":-1e300,\"l\":1e300,\"r\":0}}"}) {
            QTextCharFormat bad;
            bad.setProperty(tp::InlineObject, QString::fromLatin1(json));
            c.insertText(QString(QChar::ObjectReplacementCharacter), bad);
        }
        doc->pages[0]->items = {t};
        LayoutCache cache;
        const auto fl = cache.textFrame(*doc, *t, 1, RenderOptions());
        QVERIFY(fl.layout);
        for (const auto &ob : fl.layout->inlineObjects()) {
            QVERIFY(std::isfinite(ob.rect.left()) && std::isfinite(ob.rect.top()) && std::isfinite(ob.rect.width()) && std::isfinite(ob.rect.height()));
            QVERIFY(std::abs(ob.rect.top()) < 1e6 && ob.rect.width() >= 0 && ob.rect.height() >= 0);
        }
        for (const auto &li : fl.layout->lineInfo(0)) QVERIFY(std::isfinite(li.baseline) && std::abs(li.baseline) < 1e6);
        // Drawing ends (the box in its own text is drawn, but not inside itself).
        QImage img(612, 792, QImage::Format_ARGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        PaintContext ctx;
        ctx.doc = doc.get();
        ctx.cache = &cache;
        Renderer::paintItem(&p, ctx, *t);
        p.end();
        // Saving ends too.
        QTemporaryDir dir;
        QString err;
        QVERIFY2(exportPublisher(*doc, dir.filePath(QStringLiteral("self.pub")), &err), qPrintable(err));
    }

    // A .pub story marks an object set in its text with U+FFFC. Publisher
    // shows nothing there when the object doesn't come through; JeffPub
    // drew the font's "OBJ" box.
    void pubInlineObjectMarks()
    {
        using namespace jp;
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->rect = QRectF(72, 72, 300, 100);
        t->storyId = doc->createStory(QStringLiteral("Before") + QChar(QChar::ObjectReplacementCharacter) + QStringLiteral("after"));
        doc->pages[0]->items.push_back(t);
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("obj.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        auto back = importPublisherFile(path, &err);
        QVERIFY2(back, qPrintable(err));
        auto *bt = dynamic_cast<TextItem *>(back->pages[0]->items[0].get());
        QVERIFY(bt);
        QCOMPARE(back->storyDoc(bt->storyId)->toPlainText(), QStringLiteral("Beforeafter"));
    }

    // Footnotes at the bottom of the column their reference lands in (under
    // a rule, numbered in order, the text kept above them); a line whose
    // note won't fit moves on with it; endnotes under "Notes" after the
    // story; editing, undo, numbers outside layout, and saving.
    void footnotesAndEndnotes()
    {
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto a = std::make_shared<jp::TextItem>();
        a->rect = QRectF(72, 72, 300, 200);
        a->storyId = doc->createStory();
        auto b2 = std::make_shared<jp::TextItem>();
        b2->rect = QRectF(72, 400, 300, 300);
        b2->storyId = a->storyId;
        a->nextId = b2->id;
        doc->pages[0]->items.push_back(a);
        doc->pages[0]->items.push_back(b2);
        QTextDocument *sd = doc->storyDoc(a->storyId);
        {
            QTextCursor c(sd);
            for (int i = 0; i < 12; ++i) {
                if (i) c.insertBlock();
                c.insertText(QStringLiteral("Paragraph %1 with enough words to fill a line of the box.").arg(i + 1));
            }
        }
        w.editor()->setDocument(std::move(doc));
        jp::Editor *ed = w.editor();
        auto at = [&](int block, bool end) {
            ed->beginTextEdit(a->id);
            QTextCursor c(sd->findBlockByNumber(block));
            c.movePosition(QTextCursor::EndOfBlock);
            ed->setCursor(c);
            return end;
        };
        at(0, false);
        const QString n1 = jp::addNote(ed, false, QStringLiteral("The first note."));
        at(1, false);
        const QString n2 = jp::addNote(ed, false, QStringLiteral("The second note, long enough to take two lines in a box of this width."));
        at(11, true);
        const QString e1 = jp::addNote(ed, true, QStringLiteral("An endnote."));
        QVERIFY(!n1.isEmpty() && !n2.isEmpty() && !e1.isEmpty());
        // The reference is the number, superscript.
        QCOMPARE(sd->findBlockByNumber(0).text().right(1), QString(QChar::ObjectReplacementCharacter));
        jp::LayoutCache cache;
        jp::RenderOptions opt;
        auto lay = [&] { return cache.textFrame(*ed->doc(), *a, 1, opt); };
        auto fl = lay();
        QVERIFY(fl.layout);
        const auto &notes = fl.layout->notes();
        QCOMPARE(notes.size(), 3);
        QCOMPARE(notes[0].number, 1);
        QCOMPARE(notes[1].number, 2);
        QVERIFY(!notes[0].endnote && notes[2].endnote && notes[2].number == 1);
        // Both footnotes at the bottom of the first box, in order; no line
        // of text runs into them.
        QCOMPARE(notes[0].frame, 0);
        QCOMPARE(notes[1].frame, 0);
        QVERIFY(notes[0].rect.bottom() <= notes[1].rect.top() + 0.01);
        QVERIFY2(std::abs(notes[1].rect.bottom() + 2 - (200 - a->insets.bottom())) < 1.5, qPrintable(QString::number(notes[1].rect.bottom())));
        for (const QRectF &r : fl.layout->lineRects(0)) QVERIFY(r.bottom() <= notes[0].rect.top());
        QVERIFY(fl.layout->lineInfo(0).first().text.contains(QLatin1Char('1')));
        // The endnote after the story, in the second box.
        QCOMPARE(notes[2].frame, 1);
        double lastLine = 0;
        for (const QRectF &r : fl.layout->lineRects(1)) lastLine = std::max(lastLine, r.bottom());
        QVERIFY(notes[2].rect.top() > lastLine);
        // Numbers outside layout (exports).
        jp::FieldContext ctx;
        ctx.doc = ed->doc();
        QCOMPARE(ctx.resolve(QStringLiteral("footnote:") + n2), QStringLiteral("2"));
        QCOMPARE(ctx.resolve(QStringLiteral("endnote:") + e1), QStringLiteral("1"));
        // A long note that can't fit with its line: both move to the next box.
        at(5, false);
        const QString big = jp::addNote(ed, false, QStringLiteral("A very long note. ").repeated(30));
        fl = lay();
        int bigIdx = -1;
        for (int i = 0; i < fl.layout->notes().size(); ++i)
            if (fl.layout->notes()[i].storyId == big) bigIdx = i;
        QVERIFY(bigIdx >= 0);
        QCOMPARE(fl.layout->notes()[bigIdx].frame, 1);

        // The line with its number went too (the paragraph's earlier lines stay).
        bool refInSecond = false;
        const int para6 = sd->findBlockByNumber(5).position();
        for (const auto &li : fl.layout->lineInfo(1)) refInSecond |= li.docStart == para6 && li.text.contains(QLatin1Char('3'));
        QVERIFY(refInSecond);
        // Editing: at the reference, the note's own text.
        at(0, false);
        bool isEnd = true;
        QCOMPARE(jp::noteAtCursor(ed, &isEnd), n1);
        QVERIFY(!isEnd);
        jp::setNoteText(ed, n1, QStringLiteral("Changed."));
        QCOMPARE(ed->doc()->storyDoc(n1)->toPlainText(), QStringLiteral("Changed."));
        ed->undo();
        QCOMPARE(ed->doc()->storyDoc(n1)->toPlainText(), QStringLiteral("The first note."));
        // Saved and opened again: the same notes.
        QString err;
        auto back = jp::publicationFromBytes(jp::publicationBytes(*ed->doc(), QImage()), &err);
        QVERIFY2(back, qPrintable(err));
        jp::TextItem *ba = nullptr;
        for (const auto &it : back->pages[0]->items)
            if (it->id == a->id) ba = static_cast<jp::TextItem *>(it.get());
        QVERIFY(ba);
        jp::LayoutCache cache2;
        const auto fl2 = cache2.textFrame(*back, *ba, 1, opt);
        QCOMPARE(fl2.layout->notes().size(), 4);
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) {
            ed->endTextEdit();
            w.resize(1200, 900);
            w.show();
            QTest::qWait(100);
            w.grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/notes.png");
        }
    }

    // A footnote too tall for a whole column is cut where its column ends and
    // goes on at the bottom of the next column, above that column's own
    // notes, under a rule as wide as the column. The line with the number
    // stays where the note starts, no line of the note is hidden, and they
    // keep their order.
    void footnoteTallerThanColumnGoesOnInNextColumn()
    {
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto a = std::make_shared<jp::TextItem>();
        a->rect = QRectF(72, 72, 300, 200);
        a->columns = 2;
        a->storyId = doc->createStory();
        doc->pages[0]->items.push_back(a);
        QTextDocument *sd = doc->storyDoc(a->storyId);
        {
            QTextCursor c(sd);
            for (int i = 0; i < 4; ++i) {
                if (i) c.insertBlock();
                c.insertText(QStringLiteral("Paragraph %1.").arg(i + 1));
            }
        }
        w.editor()->setDocument(std::move(doc));
        jp::Editor *ed = w.editor();
        auto at = [&](int block) {
            ed->beginTextEdit(a->id);
            QTextCursor c(sd->findBlockByNumber(block));
            c.movePosition(QTextCursor::EndOfBlock);
            ed->setCursor(c);
        };
        auto words = [](int n) {
            QStringList l;
            for (int i = 1; i <= n; ++i) l << QStringLiteral("word%1").arg(i);
            return l.join(QLatin1Char(' '));
        };
        at(0);
        const QString big = jp::addNote(ed, false, words(10));
        at(3);
        const QString small = jp::addNote(ed, false, QStringLiteral("The second note."));
        QVERIFY(!big.isEmpty() && !small.isEmpty());
        jp::LayoutCache cache;
        jp::RenderOptions opt;
        auto lay = [&] {
            cache.clear();
            return cache.textFrame(*ed->doc(), *a, 1, opt);
        };
        // Words are added to the first note until it no longer fits a column
        // with its line (whatever the fonts), then a few lines more.
        auto fl = lay();
        int count = 10;
        while (fl.layout->notes()[0].more.isEmpty() && count < 1000) {
            count += 5;
            jp::setNoteText(ed, big, words(count));
            fl = lay();
        }
        QVERIFY(count < 1000);
        count += 15;
        jp::setNoteText(ed, big, words(count));
        fl = lay();
        const auto &notes = fl.layout->notes();
        QCOMPARE(notes.size(), 2);
        const auto &n = notes[0];
        QCOMPARE(n.frame, 0);
        QCOMPARE(n.column, 0);
        QCOMPARE(n.more.size(), 1);
        QCOMPARE(n.more[0].frame, 0);
        QCOMPARE(n.more[0].column, 1);
        QVERIFY(!fl.layout->overflow());
        // Each part is at the bottom of its own column; the second note
        // follows the rest of the first in the second column.
        const double colW = (300 - a->insets.left() - a->insets.right() - a->columnGap) / 2;
        const double col1Left = a->insets.left() + colW + a->columnGap;
        const double bottom = 200 - a->insets.bottom();
        QVERIFY(std::abs(n.rect.left() - a->insets.left()) < 0.01);
        QVERIFY(std::abs(n.more[0].rect.left() - col1Left) < 0.01);
        QVERIFY2(std::abs(n.rect.bottom() + 2 - bottom) < 1.5, qPrintable(QString::number(n.rect.bottom())));
        QCOMPARE(notes[1].frame, 0);
        QCOMPARE(notes[1].column, 1);
        QVERIFY(n.more[0].rect.bottom() <= notes[1].rect.top() + 0.01);
        QVERIFY(std::abs(notes[1].rect.bottom() + 2 - bottom) < 1.5);
        QVERIFY(n.rect.top() >= a->insets.top());
        // The line with the number is the first in the first column, above its note;
        // no line of text runs into a note.
        const auto info = fl.layout->lineInfo(0);
        QVERIFY(info.first().rect.left() < col1Left - 1);
        QVERIFY(info.first().text.contains(QStringLiteral(".1")));
        for (const QRectF &r : fl.layout->lineRects(0))
            QVERIFY(r.bottom() <= (r.left() < col1Left - 1 ? n.rect.top() : n.more[0].rect.top()) + 0.01);
        // The note's lines, in order, none hidden.
        auto shown = [](const jp::StoryLayout *l) {
            QString s;
            for (const auto &li : l->lineInfo(0)) s += li.text;
            return s;
        };
        QVERIFY(!shown(n.layout.get()).isEmpty());
        QVERIFY(!shown(n.more[0].layout.get()).isEmpty());
        QString all = shown(n.layout.get()) + shown(n.more[0].layout.get());
        all.remove(QChar(0x00AD));
        QCOMPARE(all.simplified(), words(count));
        // A short rule over the first column's notes, one as wide as the column over the second's.
        QVector<QLineF> rules;
        for (const auto &r : fl.layout->noteRules())
            if (r.frame == 0) rules << r.line;
        QCOMPARE(rules.size(), 2);
        std::sort(rules.begin(), rules.end(), [](const QLineF &x, const QLineF &y) { return x.x1() < y.x1(); });
        QVERIFY(std::abs(rules[0].length() - colW / 3) < 0.01);
        QVERIFY(std::abs(rules[1].x1() - col1Left) < 0.01);
        QVERIFY2(std::abs(rules[1].length() - colW) < 0.01, qPrintable(QString::number(rules[1].length())));
        QVERIFY(rules[0].y1() < n.rect.top() && rules[1].y1() < n.more[0].rect.top());
        // Drawn: ink where the rest of the note is, none above the box.
        const QRectF firstRect = n.rect, restRect = n.more[0].rect;
        jp::PaintContext pc;
        pc.doc = ed->doc();
        pc.cache = &cache;
        pc.opt.output = true;
        const QImage img = jp::Renderer::renderToImage(pc, 0, 1.0);
        auto ink = [&](const QRectF &r) {
            int dark = 0;
            for (int y = std::max(0, int(r.top())); y < std::min(img.height(), int(std::ceil(r.bottom()))); ++y)
                for (int x = std::max(0, int(r.left())); x < std::min(img.width(), int(std::ceil(r.right()))); ++x) dark += qGray(img.pixel(x, y)) < 128;
            return dark;
        };
        QVERIFY(ink(restRect.translated(a->rect.topLeft())) > 20);
        QVERIFY(ink(firstRect.translated(a->rect.topLeft())) > 20);
        QCOMPARE(ink(QRectF(0, 0, 612, a->rect.top())), 0);
    }

    // The rest of a footnote goes on in the next linked text box, on whatever
    // page it is and however wide its columns are.
    void footnoteTallerThanBoxGoesOnInNextBox()
    {
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto a = std::make_shared<jp::TextItem>();
        a->rect = QRectF(72, 72, 300, 200);
        a->storyId = doc->createStory();
        auto b2 = std::make_shared<jp::TextItem>();
        b2->rect = QRectF(100, 72, 240, 200);
        b2->storyId = a->storyId;
        a->nextId = b2->id;
        doc->pages[0]->items.push_back(a);
        doc->addPage()->items.push_back(b2);
        QTextDocument *sd = doc->storyDoc(a->storyId);
        {
            QTextCursor c(sd);
            for (int i = 0; i < 6; ++i) {
                if (i) c.insertBlock();
                c.insertText(QStringLiteral("Paragraph %1.").arg(i + 1));
            }
        }
        w.editor()->setDocument(std::move(doc));
        jp::Editor *ed = w.editor();
        ed->beginTextEdit(a->id);
        QTextCursor cur(sd->findBlockByNumber(0));
        cur.movePosition(QTextCursor::EndOfBlock);
        ed->setCursor(cur);
        auto words = [](int n) {
            QStringList l;
            for (int i = 1; i <= n; ++i) l << QStringLiteral("word%1").arg(i);
            return l.join(QLatin1Char(' '));
        };
        const QString big = jp::addNote(ed, false, words(10));
        QVERIFY(!big.isEmpty());
        jp::LayoutCache cache;
        jp::RenderOptions opt;
        auto lay = [&] {
            cache.clear();
            return cache.textFrame(*ed->doc(), *a, 1, opt);
        };
        auto fl = lay();
        int count = 10;
        while (fl.layout->notes()[0].more.isEmpty() && count < 1000) {
            count += 5;
            jp::setNoteText(ed, big, words(count));
            fl = lay();
        }
        QVERIFY(count < 1000);
        count += 25;
        jp::setNoteText(ed, big, words(count));
        fl = lay();
        QCOMPARE(fl.layout->frameCount(), 2);
        const auto &n = fl.layout->notes()[0];
        QCOMPARE(n.frame, 0);
        QCOMPARE(n.more.size(), 1);
        QCOMPARE(n.more[0].frame, 1);
        QCOMPARE(n.more[0].column, 0);
        QVERIFY(!fl.layout->overflow());
        // The rest is laid out for the second box's column and sits at its bottom.
        const double colW2 = 240 - b2->insets.left() - b2->insets.right();
        QVERIFY(std::abs(n.more[0].rect.width() - colW2) < 0.01);
        QVERIFY(std::abs(n.more[0].rect.left() - b2->insets.left()) < 0.01);
        QVERIFY2(std::abs(n.more[0].rect.bottom() + 2 - (200 - b2->insets.bottom())) < 1.5, qPrintable(QString::number(n.more[0].rect.bottom())));
        // The text of the story goes on above it in the second box.
        const auto second = fl.layout->lineInfo(1);
        QVERIFY(!second.isEmpty());
        for (const QRectF &r : fl.layout->lineRects(1)) QVERIFY(r.bottom() <= n.more[0].rect.top() + 0.01);
        for (const QRectF &r : fl.layout->lineRects(0)) QVERIFY(r.bottom() <= n.rect.top() + 0.01);
        QVERIFY(fl.layout->lineInfo(0).first().text.contains(QStringLiteral(".1")));
        // Every line of the note, in order.
        auto shown = [](const jp::StoryLayout *l) {
            QString s;
            for (const auto &li : l->lineInfo(0)) s += li.text;
            return s;
        };
        QString all = shown(n.layout.get()) + shown(n.more[0].layout.get());
        all.remove(QChar(0x00AD));
        QCOMPARE(all.simplified(), words(count));
        // A short rule in the first box, one as wide as the column in the second.
        QLineF first, rest;
        for (const auto &r : fl.layout->noteRules()) (r.frame == 0 ? first : rest) = r.line;
        QVERIFY(std::abs(first.length() - (300 - a->insets.left() - a->insets.right()) / 3) < 0.01);
        QVERIFY2(std::abs(rest.length() - colW2) < 0.01, qPrintable(QString::number(rest.length())));
        // Drawn on the second box's page.
        const QRectF restRect = n.more[0].rect;
        jp::PaintContext pc;
        pc.doc = ed->doc();
        pc.cache = &cache;
        pc.opt.output = true;
        const QImage img = jp::Renderer::renderToImage(pc, 1, 1.0);
        int dark = 0;
        const QRectF r = restRect.translated(b2->rect.topLeft());
        for (int y = int(r.top()); y < int(std::ceil(r.bottom())); ++y)
            for (int x = int(r.left()); x < int(std::ceil(r.right())); ++x) dark += qGray(img.pixel(x, y)) < 128;
        QVERIFY(dark > 20);
    }

    // A very long footnote (of several paragraphs) goes on through as many
    // text boxes as it needs, a whole box where it takes one; with no box
    // left, the story overflows as for any other text.
    void footnoteGoesOnThroughManyBoxes()
    {
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        std::shared_ptr<jp::TextItem> boxes[3];
        for (int i = 0; i < 3; ++i) {
            boxes[i] = std::make_shared<jp::TextItem>();
            boxes[i]->rect = QRectF(72, 72 + 230 * i, 300, 200);
            doc->pages[0]->items.push_back(boxes[i]);
        }
        auto a = boxes[0];
        a->storyId = doc->createStory();
        for (int i = 1; i < 3; ++i) {
            boxes[i]->storyId = a->storyId;
            boxes[i - 1]->nextId = boxes[i]->id;
        }
        QTextDocument *sd = doc->storyDoc(a->storyId);
        QTextCursor(sd).insertText(QStringLiteral("Paragraph 1."));
        w.editor()->setDocument(std::move(doc));
        jp::Editor *ed = w.editor();
        ed->beginTextEdit(a->id);
        QTextCursor cur(sd->findBlockByNumber(0));
        cur.movePosition(QTextCursor::EndOfBlock);
        ed->setCursor(cur);
        // Paragraphs of twelve words each, the last shorter.
        auto words = [](int n) {
            QStringList paragraphs, l;
            for (int i = 1; i <= n; ++i) {
                l << QStringLiteral("word%1").arg(i);
                if (l.size() == 12 || i == n) {
                    paragraphs << l.join(QLatin1Char(' '));
                    l.clear();
                }
            }
            return paragraphs.join(QLatin1Char('\n'));
        };
        const QString big = jp::addNote(ed, false, words(10));
        QVERIFY(!big.isEmpty());
        jp::LayoutCache cache;
        jp::RenderOptions opt;
        auto lay = [&] {
            cache.clear();
            return cache.textFrame(*ed->doc(), *a, 1, opt);
        };
        auto fl = lay();
        int count = 10;
        while (fl.layout->notes()[0].more.size() < 2 && count < 2000) {
            count += 10;
            jp::setNoteText(ed, big, words(count));
            fl = lay();
        }
        QVERIFY(count < 2000);
        const auto &n = fl.layout->notes()[0];
        QCOMPARE(n.more.size(), 2);
        QVERIFY(!fl.layout->overflow());
        QCOMPARE(n.frame, 0);
        QCOMPARE(n.more[0].frame, 1);
        QCOMPARE(n.more[1].frame, 2);
        // The second box holds nothing but the note, from its rule to its
        // bottom, though the text ended in the first.
        QVERIFY(fl.layout->lineRects(1).isEmpty());
        QVERIFY(n.more[0].rect.top() < 40);
        QVERIFY(std::abs(n.more[0].rect.bottom() + 2 - (200 - a->insets.bottom())) < 1.5);
        QVERIFY(std::abs(n.more[1].rect.bottom() + 2 - (200 - a->insets.bottom())) < 1.5);
        QVector<QLineF> rules(3);
        for (const auto &r : fl.layout->noteRules()) rules[r.frame] = r.line;
        QVERIFY(std::abs(rules[0].length() - (300 - a->insets.left() - a->insets.right()) / 3) < 0.01);
        QVERIFY(std::abs(rules[1].length() - (300 - a->insets.left() - a->insets.right())) < 0.01);
        QVERIFY(std::abs(rules[2].length() - (300 - a->insets.left() - a->insets.right())) < 0.01);
        auto shown = [](const jp::StoryLayout *l) {
            QString s;
            for (const auto &li : l->lineInfo(0)) s += li.text + QLatin1Char(' ');
            return s;
        };
        QString all = shown(n.layout.get()) + shown(n.more[0].layout.get()) + shown(n.more[1].layout.get());
        all.remove(QChar(0x00AD));
        QCOMPARE(all.simplified(), words(count).simplified());
        // The last box taken away: the rest of the note has nowhere to go.
        boxes[1]->nextId.clear();
        fl = lay();
        QVERIFY(fl.layout->overflow());
        QCOMPARE(fl.layout->notes()[0].more.size(), 1);
        QCOMPARE(fl.layout->notes()[0].more[0].frame, 1);
    }

    // A note that fits a column is never cut: at the bottom of the column
    // under the short rule, or, when it doesn't fit under its line, on with
    // the line in the next column.
    void shortFootnoteIsNotCut()
    {
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto a = std::make_shared<jp::TextItem>();
        a->rect = QRectF(72, 72, 300, 200);
        a->columns = 2;
        a->storyId = doc->createStory();
        doc->pages[0]->items.push_back(a);
        QTextDocument *sd = doc->storyDoc(a->storyId);
        {
            QTextCursor c(sd);
            for (int i = 0; i < 9; ++i) {
                if (i) c.insertBlock();
                c.insertText(QStringLiteral("Paragraph %1.").arg(i + 1));
            }
        }
        w.editor()->setDocument(std::move(doc));
        jp::Editor *ed = w.editor();
        auto at = [&](int block) {
            ed->beginTextEdit(a->id);
            QTextCursor c(sd->findBlockByNumber(block));
            c.movePosition(QTextCursor::EndOfBlock);
            ed->setCursor(c);
        };
        at(0);
        QVERIFY(!jp::addNote(ed, false, QStringLiteral("The first note.")).isEmpty());
        jp::LayoutCache cache;
        jp::RenderOptions opt;
        auto fl = cache.textFrame(*ed->doc(), *a, 1, opt);
        const double colW = (300 - a->insets.left() - a->insets.right() - a->columnGap) / 2;
        const double col1Left = a->insets.left() + colW + a->columnGap;
        auto n = fl.layout->notes()[0];
        QCOMPARE(n.frame, 0);
        QCOMPARE(n.column, 0);
        QVERIFY(n.more.isEmpty());
        QVERIFY(std::abs(n.rect.bottom() + 2 - (200 - a->insets.bottom())) < 1.5);
        QVERIFY(std::abs(n.rect.width() - colW) < 0.01);
        QCOMPARE(fl.layout->noteRules().size(), 1);
        QVERIFY(std::abs(fl.layout->noteRules()[0].line.length() - colW / 3) < 0.01);
        QVERIFY(!fl.layout->overflow());
        for (const QRectF &r : fl.layout->lineRects(0)) QVERIFY(r.left() > col1Left - 1 || r.bottom() <= n.rect.top() + 0.01);
        // A note that fits an empty column but not the room left under its
        // line: lines are added to it until it no longer fits the first
        // column; both go to the next column, the note whole.
        at(6);
        const QString mid = jp::addNote(ed, false, QStringLiteral("Note line 1"));
        QVERIFY(!mid.isEmpty());
        QStringList lines{QStringLiteral("Note line 1")};
        cache.clear();
        fl = cache.textFrame(*ed->doc(), *a, 1, opt);
        QCOMPARE(fl.layout->notes().size(), 2);
        QCOMPARE(fl.layout->notes()[1].column, 0);
        while (fl.layout->notes()[1].column == 0 && lines.size() < 40) {
            lines << QStringLiteral("Note line %1").arg(lines.size() + 1);
            jp::setNoteText(ed, mid, lines.join(QLatin1Char('\n')));
            cache.clear();
            fl = cache.textFrame(*ed->doc(), *a, 1, opt);
        }
        n = fl.layout->notes()[1];
        QCOMPARE(n.frame, 0);
        QCOMPARE(n.column, 1);
        QVERIFY(n.more.isEmpty());
        QVERIFY(!fl.layout->overflow());
        bool refInSecond = false;
        for (const auto &li : fl.layout->lineInfo(0)) refInSecond |= li.rect.left() > col1Left - 1 && li.text.contains(QStringLiteral("7.2"));
        QVERIFY(refInSecond);
    }

    // Inserting a symbol puts it in the text and at the front of the recently used list.
    void recentSymbols()
    {
        const QVariant before = jp::Settings::get().value(QStringLiteral("symbols/recent"));
        jp::Settings::get().setValue(QStringLiteral("symbols/recent"), QStringList());
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 300, 100);
        t->storyId = doc->createStory(QStringLiteral("x"));
        doc->pages[0]->items.push_back(t);
        w.editor()->setDocument(std::move(doc));
        jp::Editor *ed = w.editor();
        const QString id = ed->doc()->pages[0]->items[0]->id;
        ed->select(id);
        ed->beginTextEdit(id, 1);
        QVERIFY(jp::insertSymbol(&w, ed, QString(QChar(0x00A7)), QString()));
        QVERIFY(jp::insertSymbol(&w, ed, QString(QChar(0x2122)), QStringLiteral("Symbol")));
        const QString sid = static_cast<const jp::TextItem *>(ed->doc()->pages[0]->items[0].get())->storyId;
        QCOMPARE(ed->doc()->storyDoc(sid)->toPlainText(), QString(QChar('x')) + QChar(0x00A7) + QChar(0x2122));
        const QStringList r = jp::recentSymbols();
        QCOMPARE(r.size(), 2);
        QCOMPARE(r[0], QString(QChar(0x2122)) + QStringLiteral("\tSymbol"));
        QCOMPARE(r[1], QString(QChar(0x00A7)));
        jp::Settings::get().setValue(QStringLiteral("symbols/recent"), before);
    }

    // An outline set inside its shape (Publisher's text box borders and many
    // design frames, the inset-pen flag in a .pub) draws nothing outside the
    // shape's edge, and keeps the setting through a .pub.
    void insetOutlineStaysInside()
    {
        auto doc = Document::blank(QSizeF(200, 200));
        auto box = std::make_shared<ShapeItem>();
        box->shape = QStringLiteral("rect");
        box->rect = QRectF(40, 40, 120, 120);
        box->fill = Fill::none();
        box->stroke = Stroke::line(ColorRef::rgb(Qt::black), 16);
        box->stroke.inset = true;
        doc->pages[0]->items.push_back(box);
        auto draw = [&](Document &d) {
            PaintContext ctx;
            ctx.doc = &d;
            ctx.opt.output = true;
            return Renderer::renderToImage(ctx, 0, 1.0).convertToFormat(QImage::Format_RGB32);
        };
        auto dark = [](const QImage &img, int x, int y) { return qGray(img.pixel(x, y)) < 128; };
        QImage img = draw(*doc);
        QVERIFY(!dark(img, 36, 100) && !dark(img, 100, 36));   // nothing outside the edge
        QVERIFY(dark(img, 42, 100) && dark(img, 54, 100));      // the full width inside it
        QVERIFY(!dark(img, 60, 100));
        // Through a .pub: Publisher's inset-pen flag.
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("inset.pub"));
        QString err;
        QVERIFY2(exportPublisher(*doc, path, &err), qPrintable(err));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        auto back = importPublisher(f.readAll(), nullptr);
        QVERIFY(back);
        const ShapeItem *got = nullptr;
        for (const auto &it : back->pages[0]->items)
            if (auto *s = dynamic_cast<const ShapeItem *>(it.get()); s && !s->stroke.isNone()) got = s;
        QVERIFY(got);
        QVERIFY(got->stroke.inset);
        QVERIFY(!dark(draw(*back), 36, 100));
    }

    // A text box that only touches another pushes none of its text aside
    // (Publisher's designs butt boxes together, their wrap distances
    // reaching past each other's insets): a name in a short box lost its
    // only line. One that overlaps still does.
    void touchingTextBoxesDontWrap()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        auto box = [&](const QRectF &r, const QString &text) {
            auto t = std::make_shared<TextItem>();
            t->rect = r;
            t->insets = QMarginsF(2.85, 2.85, 2.85, 2.85);
            t->storyId = doc->createStory(text);
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontPointSize(8);
            c.mergeCharFormat(f);
            doc->pages[0]->items.push_back(t);
            return t;
        };
        auto name = box(QRectF(72, 100, 150, 12.7), QStringLiteral("Jeff"));
        auto above = box(QRectF(72, 80, 150, 20), QStringLiteral("Title"));   // touches the name's top
        LayoutCache cache;
        RenderOptions opt;
        QCOMPARE(cache.textFrame(*doc, *name, 1, opt).layout->lineInfo(0).size(), 1);
        above->rect.moveTop(84);   // now 4 points into the name's text area
        QCOMPARE(cache.textFrame(*doc, *name, 1, opt).layout->lineInfo(0).size(), 0);
    }

    // An object over a whole text box never pushes its text aside, whatever
    // its wrap or fill (Publisher's pictures of a box under rectangles with
    // each wrap); one over part of it does. Sign and newsletter designs
    // frame their text boxes with an empty rectangle set to wrap Through,
    // and JeffPub emptied the boxes.
    void coveringObjectDoesntWrap()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<TextItem>();
        t->id = QStringLiteral("box");
        t->rect = QRectF(92, 92, 140, 150);
        t->storyId = doc->createStory(QStringLiteral("The quick brown fox jumps over the lazy dog again and again."));
        doc->pages[0]->items.push_back(t);
        auto frame = std::make_shared<ShapeItem>();
        frame->id = QStringLiteral("frame");
        frame->shape = QStringLiteral("rect");
        frame->fill = Fill::none();
        frame->stroke.color = ColorRef::rgb(Qt::red);
        doc->pages[0]->items.push_back(frame);
        // Its text then sits at the top, whatever its vertical alignment
        // (Publisher's pictures of middle-aligned boxes under each wrap).
        t->valign = VAlign::Middle;
        RenderOptions opt;
        for (Wrap::Mode mode : {Wrap::Square, Wrap::Tight, Wrap::Through, Wrap::TopBottom}) {
            frame->wrap.mode = mode;
            frame->rect = QRectF(72, 72, 180, 190);   // all around the box
            QVERIFY2(jp::Renderer::wrapObstacles(*doc, *t).isEmpty(), qPrintable(QString::number(int(mode))));
            QCOMPARE(jp::Renderer::frameSpec(*doc, *t, 1, opt).valign, VAlign::Top);
            frame->rect = QRectF(162, 82, 90, 170);   // over its right half
            QCOMPARE(jp::Renderer::wrapObstacles(*doc, *t).size(), 1);
            QCOMPARE(jp::Renderer::frameSpec(*doc, *t, 1, opt).valign, VAlign::Middle);
        }
        frame->wrap.mode = Wrap::None;
        frame->rect = QRectF(72, 72, 180, 190);
        QCOMPARE(jp::Renderer::frameSpec(*doc, *t, 1, opt).valign, VAlign::Middle);
    }

    void editWrapPoints()
    {
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 400, 300);
        t->storyId = doc->createStory(QString("Text flows around the shape. ").repeated(60));
        doc->pages[0]->items.push_back(t);
        auto sh = std::make_shared<jp::ShapeItem>();
        sh->shape = QStringLiteral("ellipse");
        sh->rect = QRectF(150, 150, 100, 100);
        sh->fill = jp::Fill::solid(jp::ColorRef::rgb(Qt::red));
        doc->pages[0]->items.push_back(sh);
        w.editor()->setDocument(std::move(doc));
        jp::Document *d = w.editor()->doc();
        auto *tb = static_cast<jp::TextItem *>(d->pages[0]->items[0].get());
        jp::Item *s = d->pages[0]->items[1].get();
        w.editor()->select(s->id);
        QTest::qWait(50);
        w.act(QStringLiteral("wrap.edit"))->trigger();
        QCOMPARE(w.editor()->wrapItem, s->id);
        QCOMPARE(int(s->wrap.mode), int(jp::Wrap::Tight));
        const int n = s->wrap.points.size();
        QVERIFY2(n >= 8 && n <= 24, qPrintable(QString::number(n)));
        // The starting outline follows the ellipse.
        const QRectF wb = s->transform().map(s->wrap.points).boundingRect();
        QVERIFY(std::abs(wb.left() - 150) < 2 && std::abs(wb.right() - 250) < 2);
        jp::Canvas *c = w.canvas();
        QWidget *vp = c->viewport();
        auto view = [&](QPointF page) { return c->pageToView(page).toPoint(); };
        // Drag the rightmost point 100 pt further right.
        int ri = 0;
        for (int i = 1; i < n; ++i)
            if (s->wrap.points[i].x() > s->wrap.points[ri].x()) ri = i;
        const QPointF from = s->transform().map(s->wrap.points[ri]), to = from + QPointF(100, 0);
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, view(from));
        QTest::mouseMove(vp, view(from + QPointF(50, 0)));
        QTest::mouseMove(vp, view(to));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, view(to));
        QCOMPARE(s->wrap.points.size(), n);
        QVERIFY(std::abs(s->transform().map(s->wrap.points[ri]).x() - to.x()) < 2);
        double right = 0;
        for (const QPolygonF &o : jp::Renderer::wrapObstacles(*d, *tb)) right = std::max(right, o.boundingRect().right());
        // Obstacles are in the text box's own coordinates.
        QVERIFY2(right > to.x() - tb->rect.left() - 1, qPrintable(QString::number(right)));
        if (!qEnvironmentVariableIsEmpty("JP_SHOT_DIR")) { QTest::qWait(100); c->grab().save(qEnvironmentVariable("JP_SHOT_DIR") + "/wrappoints.png"); }
        // Dragging the middle of an edge adds a point.
        const QPointF a = s->transform().map(s->wrap.points[0]), b = s->transform().map(s->wrap.points[1]);
        const QPointF mid = (a + b) / 2;
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, view(mid));
        QTest::mouseMove(vp, view(mid + QPointF(0, -10)));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, view(mid + QPointF(0, -10)));
        QCOMPARE(s->wrap.points.size(), n + 1);
        // Ctrl+click deletes it again.
        QTest::mouseClick(vp, Qt::LeftButton, Qt::ControlModifier, view(mid + QPointF(0, -10)));
        QCOMPARE(s->wrap.points.size(), n);
        // Undo walks back to the starting outline.
        w.editor()->undo();
        w.editor()->undo();
        w.editor()->undo();
        jp::Item *s2 = w.editor()->doc()->pages[0]->items[1].get();
        QCOMPARE(s2->wrap.points.size(), n);
        QVERIFY(std::abs(s2->transform().map(s2->wrap.points).boundingRect().right() - 250) < 2);
        // Esc finishes.
        w.editor()->select(s2->id);
        w.editor()->setWrapItem(s2->id);
        QTest::keyClick(vp, Qt::Key_Escape);
        QVERIFY(w.editor()->wrapItem.isEmpty());
    }

    // Edit Points: drag a corner, Ctrl+click a point to delete it, Ctrl+click an
    // edge to add one, all with real mouse events on the canvas.
    void editPoints()
    {
        jp::MainWindow w;
        w.resize(1200, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto sh = std::make_shared<jp::ShapeItem>();
        sh->shape = QStringLiteral("rect");
        sh->rect = QRectF(200, 200, 100, 100);
        sh->fill = jp::Fill::solid(jp::ColorRef::rgb(Qt::red));
        doc->pages[0]->items.push_back(sh);
        w.editor()->setDocument(std::move(doc));
        jp::ShapeItem *s = static_cast<jp::ShapeItem *>(w.editor()->doc()->pages[0]->items[0].get());
        w.editor()->select(s->id);
        QTest::qWait(50);
        w.act(QStringLiteral("shape.editPoints"))->trigger();
        QCOMPARE(w.editor()->pointsItem, s->id);
        QVERIFY(!s->customPath.isEmpty());
        jp::Canvas *c = w.canvas();
        QWidget *vp = c->viewport();
        auto view = [&](QPointF page) { return c->pageToView(page).toPoint(); };
        // Drag the bottom-right corner (300,300) to (340,330).
        QTest::mousePress(vp, Qt::LeftButton, Qt::NoModifier, view(QPointF(300, 300)));
        QTest::mouseMove(vp, view(QPointF(320, 315)));
        QTest::mouseMove(vp, view(QPointF(340, 330)));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::NoModifier, view(QPointF(340, 330)));
        QRectF b = s->transform().map(s->customPath).boundingRect();
        QVERIFY2(std::abs(b.right() - 340) < 2 && std::abs(b.bottom() - 330) < 2, qPrintable(QStringLiteral("bounds %1,%2 %3x%4").arg(b.x()).arg(b.y()).arg(b.width()).arg(b.height())));
        // The frame follows the outline.
        QVERIFY(std::abs(s->rect.right() - 340) < 2);
        const int before = s->customPath.elementCount();
        // Ctrl+click the middle of the top edge adds a point.
        QTest::mousePress(vp, Qt::LeftButton, Qt::ControlModifier, view(QPointF(250, 200)));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::ControlModifier, view(QPointF(250, 200)));
        QCOMPARE(s->customPath.elementCount(), before + 1);
        // Ctrl+click that new point deletes it again.
        QTest::mousePress(vp, Qt::LeftButton, Qt::ControlModifier, view(QPointF(250, 200)));
        QTest::mouseRelease(vp, Qt::LeftButton, Qt::ControlModifier, view(QPointF(250, 200)));
        QCOMPARE(s->customPath.elementCount(), before);
        // Undo restores the drag's starting outline step by step.
        w.editor()->undo();
        w.editor()->undo();
        w.editor()->undo();
        b = w.editor()->doc()->pages[0]->items[0]->transform().map(static_cast<jp::ShapeItem *>(w.editor()->doc()->pages[0]->items[0].get())->customPath).boundingRect();
        QVERIFY2(std::abs(b.right() - 300) < 1, "undo did not restore the corner");
    }

    // A run that doesn't say it's bold isn't, whatever the paragraph's first
    // run is: after a bold lead-in, the rest of a .pub paragraph (its runs
    // state only what they turn on) came out bold and underlined.
    void runsDontInheritFirstRun()
    {
        auto doc = Document::blank(QSizeF(612, 792));
        const QString text = QStringLiteral("and the rest of this paragraph is plain text that wraps across several lines of a narrow box");
        auto box = [&](bool explicitPlain, double y) {
            auto t = std::make_shared<TextItem>();
            t->rect = QRectF(72, y, 220, 200);
            t->storyId = doc->createStory();
            QTextCursor c(doc->storyDoc(t->storyId));
            QTextCharFormat lead;
            lead.setFontFamilies({QStringLiteral("Carlito")});
            lead.setFontPointSize(12);
            lead.setFontWeight(QFont::Bold);
            lead.setFontUnderline(true);
            c.insertText(QStringLiteral("NOTE: "), lead);
            QTextCharFormat rest;
            rest.setFontFamilies({QStringLiteral("Carlito")});
            rest.setFontPointSize(12);
            if (explicitPlain) {
                rest.setFontWeight(QFont::Normal);
                rest.setFontUnderline(false);
            }
            c.insertText(text, rest);
            doc->pages[0]->items.push_back(t);
            return t;
        };
        auto unsaid = box(false, 72), said = box(true, 400);
        LayoutCache cache;
        RenderOptions opt;
        auto lines = [&](const std::shared_ptr<TextItem> &t) {
            QStringList out;
            for (const auto &li : cache.textFrame(*doc, *t, 1, opt).layout->lineInfo(0)) out << li.text;
            return out;
        };
        const QStringList a = lines(unsaid), b = lines(said);
        QVERIFY(b.size() >= 3);
        QCOMPARE(a, b);
    }
};

JP_TEST_CLASS(PubObjectsTests)
#include "tst_pub_objects.moc"
