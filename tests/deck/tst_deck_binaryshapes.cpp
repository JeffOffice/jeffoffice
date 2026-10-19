// JeffDeck's tests of old binary presentations: shapes, text, footers, pictures, and metafiles as they are drawn.
#include "deckhelpers.h"
#include "../timing.h"

class DeckBinaryShapesTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

private:

    // The headers and footers record of a document (instance 3) or of a slide: the format of the date and the flags of what is shown, with the
    // user's own date (the string of instance 0) and the footer's text (instance 2) when there are any.
    static QByteArray footersBox(int formatId, int flags, const QString &userDate = QString(), const QString &footer = QString())
    {
        QByteArray body = pptRec(4058, le(formatId, 2) + le(flags, 2));
        if (!userDate.isEmpty()) body += pptRec(4026, utf16(userDate), 0);
        if (!footer.isEmpty()) body += pptRec(4026, utf16(footer), 2);
        return pptBox(4057, body, 3);
    }
    // The master's places for the date (7), the footer (9), and the slide number (8); each holds the text it is given, with the field of its kind (the
    // generic date's, the slide number's) at the position given in the text, when there is one.
    static QByteArray masterFooterPlaces(const QString &footerText = QString(), const QString &dateText = QString(), const QString &numberText = QString(), int numberFieldAt = -1,
                                         int dateFieldAt = -1)
    {
        auto text = [](const QString &s, int fieldType = 0, int fieldAt = -1) {
            if (s.isEmpty()) return QByteArray();
            return pptRec(3999, le(4, 4)) + pptRec(4000, utf16(s)) + (fieldAt >= 0 ? pptRec(fieldType, le(fieldAt, 4)) : QByteArray());
        };
        return pptShape(0x410, 7, text(dateText, 4088, dateFieldAt), 4000, 200, 2000, 4300) + pptShape(0x411, 9, text(footerText), 4000, 2100, 3900, 4300)
               + pptShape(0x412, 8, text(numberText, 4056, numberFieldAt), 4000, 4000, 5500, 4300);
    }
    // How many placeholders of that type ("dt", "ftr", "sldNum") a slide has.
    static int placeholdersOf(const Page &p, const QString &type)
    {
        int n = 0;
        for (const auto &it : p.items) n += deck::placeholderType(*it) == type;
        return n;
    }
    // The code of the first field in a story (empty for a story with none).
    static QString storyFieldCode(const QTextDocument *story)
    {
        if (!story) return QString();
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it)
                if (const QString code = it.fragment().charFormat().stringProperty(tp::Field); !code.isEmpty()) return code;
        return QString();
    }
    // A presentation saved as .pptx or as .odp and opened again (null when it can't be).
    static std::unique_ptr<Document> viaPptx(const Document &d, QString *err)
    {
        const QByteArray bytes = deck::exportPresentation(d, err);
        return bytes.isEmpty() ? nullptr : deck::importPresentation(bytes, err);
    }
    static std::unique_ptr<Document> viaOdp(const Document &d, QString *err)
    {
        const QByteArray bytes = deck::exportOdp(d, false, err);
        return bytes.isEmpty() ? nullptr : deck::importOdp(bytes, err);
    }
    // The story on a slide that begins with the text (empty when none does).
    static const QTextDocument *storyBeginning(const Document &d, const Page &slide, const QString &text)
    {
        for (const auto &it : slide.items)
            if (it->type() == ItemType::Text)
                if (const QTextDocument *s = d.storyDoc(static_cast<const TextItem *>(it.get())->storyId); s && s->toPlainText().startsWith(text)) return s;
        return nullptr;
    }
    // The picture, 200 by 150 points, that such a metafile draws: the window is set to 400 by 300 units and a gray brush chosen, then the given
    // records come (the scaling), and then a box from the window's top left corner, 200 by 150 units.
    static QImage drawMetafileWith(const QByteArray &records)
    {
        const QByteArray brush = wmfWords({0}) + wmfDword(0x00646464) + wmfWords({0}), pen = wmfWords({5, 0, 0}) + wmfDword(0);
        QByteArray body = wmfRecord(0x020B, wmfWords({0, 0})) + wmfRecord(0x020C, wmfWords({300, 400})) + wmfRecord(0x02FC, brush) + wmfRecord(0x02FA, pen) + records;
        body += wmfRecord(0x012D, wmfWords({1})) + wmfRecord(0x012D, wmfWords({0})) + wmfRecord(0x041B, wmfWords({150, 200, 0, 0})) + wmfRecord(0x0000, QByteArray());
        QByteArray head = wmfDword(0x9AC6CDD7u) + wmfWords({0, 0, 0, 400, 300, 72}) + wmfDword(0);
        quint16 sum = 0;
        for (int i = 0; i < 20; i += 2) sum ^= quint16(quint8(head[i]) | (quint8(head[i + 1]) << 8));
        head += wmfWords({int(sum)});
        const QByteArray standard = wmfWords({1, 9, 0x300}) + wmfDword(quint32((18 + body.size()) / 2)) + wmfWords({2}) + wmfDword(60) + wmfWords({0});
        Document doc;
        doc.slidePictures = true;
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = doc.addImage(head + standard + body, QStringLiteral("wmf"));
        if (pic->imageId.isEmpty()) return QImage();
        pic->rect = QRectF(0, 0, 200, 150);
        pic->imgRect = pic->rect;
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = &doc;
        ctx.cache = &cache;
        const ItemList one{pic};
        return Renderer::renderItemsToImage(ctx, one, 1.0);
    }
    // Whether the point (as shares of the picture's width and height) is the box's gray; how many points are.
    static bool paintedGrayAt(const QImage &img, double fx, double fy)
    {
        const QColor c = img.pixelColor(int(img.width() * fx), int(img.height() * fy));
        return c.alpha() == 255 && qAbs(c.red() - 100) <= 2;
    }
    static int paintedGray(const QImage &img)
    {
        int n = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) {
                const QColor c = img.pixelColor(x, y);
                n += c.alpha() == 255 && qAbs(c.red() - 100) <= 2;
            }
        return n;
    }

private Q_SLOTS:

    // The presentation's drawing group holds the properties of a new shape in the program that wrote the file, and the program that draws
    // the file never reads them: a shape of a slide that names no colors is white with a black line, as the format has it, and text sits at
    // the top, however the group's table says the fills color is the scheme's, the line the text color, and text is centered in its shape.
    void importBinaryShapesIgnoreTheDrawingGroupsDefaults()
    {
        auto table = [&](const QVector<QPair<int, quint32>> &props) {
            QByteArray t;
            for (const auto &p : props) t += le(quint16(p.first), 2) + le(p.second, 4);
            return pptRec(0xF00B, t, quint16(props.size()), 3);
        };
        // Filled in scheme color 4 (the fills color), a line in scheme color 1 (the text color), text anchored in the middle.
        const QByteArray group = pptBox(1035, pptBox(0xF000, pptRec(0xF006, QByteArray(16, '\0'))
                                                                 + table({{0x0087, 1}, {0x0181, 0x08000004}, {0x01BF, 0x00100010}, {0x01C0, 0x08000001}, {0x01FF, 0x00080008}})));
        // A rectangle that says nothing of its look, one that has its own fill, and a text box that says nothing of its text.
        auto rectangle = [&](quint32 id, int top, const QByteArray &props, const QByteArray &textbox = QByteArray()) {
            return pptBox(0xF004, pptRec(0xF00A, le(id, 4) + le(0xA00, 4), 1, 2) + props + pptRec(0xF010, le(top, 2) + le(100, 2) + le(2000, 2) + le(top + 500, 2))
                                      + (textbox.isEmpty() ? QByteArray() : pptBox(0xF00D, textbox)));
        };
        const QByteArray shapes = rectangle(0x500, 100, QByteArray()) + rectangle(0x501, 800, table({{0x0181, 0x0000FF}}))
                                  + rectangle(0x502, 1500, table({{0x01BF, 0x00100000}, {0x01FF, 0x00080000}}), pptText(4, QStringLiteral("Words")));
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
        auto anchorOfTheTextBox = [&](const QByteArray &file) {
            QString err;
            auto d = deck::importBinaryPresentation(file, &err);
            if (!d) qWarning("%s", qPrintable(err));
            if (d)
                for (const auto &it : d->pages[0]->items)
                    if (it->role.isEmpty() && it->type() == ItemType::Text) return int(static_cast<const TextItem *>(it.get())->valign);
            return -1;
        };
        const auto withGroup = look(buildPpt(false, false, false, false, QByteArray(), group, shapes));
        QCOMPARE(int(withGroup.size()), 2);
        QCOMPARE(withGroup[0].first, QColor(Qt::white));
        QCOMPARE(withGroup[0].second, QColor(Qt::black));
        QCOMPARE(withGroup[1].first, QColor(255, 0, 0));
        QCOMPARE(anchorOfTheTextBox(buildPpt(false, false, false, false, QByteArray(), group, shapes)), int(VAlign::Top));
        // With no drawing group, the same.
        const auto without = look(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes));
        QCOMPARE(int(without.size()), 2);
        QCOMPARE(without[0].first, QColor(Qt::white));
        QCOMPARE(without[0].second, QColor(Qt::black));
        QCOMPARE(anchorOfTheTextBox(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes)), int(VAlign::Top));
    }

    // A slide's placeholder takes what it leaves out of its look from the master's shape of its kind, and then from the format's own (the
    // drawing group's default tables are not read): the master's title says what the slides' titles look like.
    void importBinaryPlaceholdersTakeAfterTheirMastersShapes()
    {
        auto table = [&](const QVector<QPair<int, quint32>> &props) {
            QByteArray t;
            for (const auto &p : props) t += le(quint16(p.first), 2) + le(p.second, 4);
            return pptRec(0xF00B, t, quint16(props.size()), 3);
        };
        // The group's defaults, which are not read: filled in the fills color, with a line in the text color.
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

    // A frame whose text is longer than it keeps its place and size from the file, and shows all of it as the program draws it,
    // running out of the frame: down from the top, both ways from the middle, up from the bottom.
    void importBinaryFramesShowTheirOverflowingText()
    {
        auto textBox = [&](quint32 id, int anchorText, int top, int height, const QString &text, const QVector<QPair<int, quint32>> &more = {}) {
            QByteArray props = le(0x0087, 2) + le(quint32(anchorText), 4);
            int count = 1;
            for (const auto &p : more) {
                props += le(quint16(p.first), 2) + le(p.second, 4);
                ++count;
            }
            return pptBox(0xF004, pptRec(0xF00A, le(id, 4) + le(0xA00, 4), 1, 2) + pptRec(0xF00B, props, quint16(count), 3) + pptRec(0xF010, le(top, 2) + le(200, 2) + le(2600, 2) + le(top + height, 2))
                                      + pptBox(0xF00D, pptText(4, text)));
        };
        const QString four = QStringLiteral("One\rTwo\rThree\rFour"), one = QStringLiteral("One");
        // No fill and no line: unfilled (0x01BF, 0x01FF with their use bits).
        const QVector<QPair<int, quint32>> plain = {{0x01BF, 0x00100000}, {0x01FF, 0x00080000}};
        const QByteArray shapes = textBox(0x500, 0, 2000, 200, four, plain) + textBox(0x501, 1, 2400, 200, four, plain) + textBox(0x502, 2, 2800, 200, four, plain)
                                  + textBox(0x503, 1, 3200, 400, one, plain) + textBox(0x504, 1, 3800, 200, four, {{0x0181, 0x0000FF}, {0x01BF, 0x00100010}});
        QByteArray manyPoints;
        for (int i = 0; i < 14; ++i) manyPoints += QByteArray(i ? "\r" : "") + "Point " + QByteArray::number(i);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, pptText(1, QString::fromLatin1(manyPoints)), QByteArray(), shapes), &err);
        QVERIFY2(d, qPrintable(err));
        QVector<const TextItem *> boxes;
        const TextItem *body = nullptr;
        for (const auto &it : d->pages[0]->items) {
            if (it->type() != ItemType::Text) continue;
            if (it->role.startsWith(QLatin1String("ph:body"))) body = static_cast<const TextItem *>(it.get());
            else if (it->role.isEmpty()) boxes << static_cast<const TextItem *>(it.get());
        }
        QVERIFY(body);
        QCOMPARE(int(boxes.size()), 5);
        QVERIFY(d->slideText);
        LayoutCache cache;
        const RenderOptions opt;
        auto linesOf = [&](const TextItem &t) {
            const auto fl = cache.textFrame(*d, t, 1, opt);
            return fl.layout ? fl.layout->lineInfo(0) : QVector<StoryLayout::LineInfo>();
        };
        // The body placeholder: 350 points tall from 125 as the file has it, and all 14 lines of 28 points drawn.
        QCOMPARE(body->rect.top(), 125.0);
        QCOMPARE(body->rect.height(), 350.0);
        QCOMPARE(int(body->autofit), int(TextItem::NoAutofit));
        QCOMPARE(int(linesOf(*body).size()), 14);
        // 25 points tall boxes of four lines: from the top, from the middle, from the bottom.
        for (int i : {0, 1, 2}) {
            QCOMPARE(boxes[i]->rect.height(), 25.0);
            const auto lines = linesOf(*boxes[i]);
            QCOMPARE(int(lines.size()), 4);
            const double top = lines.first().rect.top(), bottom = lines.last().rect.bottom() - lines.last().below;
            const double inTop = boxes[i]->insets.top(), inBottom = 25 - boxes[i]->insets.bottom();
            QVERIFY(bottom - top > inBottom - inTop);
            if (i == 0) QVERIFY2(std::abs(top - inTop) < 0.5, qPrintable(QString::number(top)));
            if (i == 1) QVERIFY2(std::abs((top + bottom) / 2 - (inTop + inBottom) / 2) < 0.5, qPrintable(QString::number((top + bottom) / 2)));
            if (i == 2) QVERIFY2(std::abs(bottom - inBottom) < 0.5, qPrintable(QString::number(bottom)));
        }
        // One that holds its text, and one with a fill, as they are.
        QCOMPARE(boxes[3]->rect.height(), 50.0);
        QCOMPARE(boxes[4]->rect.height(), 25.0);
    }

    // A text box that doesn't wrap its text (wrapText 0x0085 = 2, the format's "none") keeps each paragraph on one line, however long it is,
    // and sets the line against the box as its alignment says: a left one starts at the left inset and runs out past the right edge, a
    // centered one is centered on the box and runs out of both sides, a right one ends at the right inset and runs out of the left. The
    // same box that says nothing, or "square" (0), wraps, and a drawing group's table that says "none" is not a box that says it.
    void importBinaryTextWithNoWrapIsSetOnOneLine()
    {
        const QString words = QStringLiteral("one two three four five six seven eight nine ten");
        const int length = int(words.size());
        // A box 100 points wide (800 master units), the paragraph's alignment (0 left, 1 center, 2 right) and the wrap property.
        auto textBox = [&](quint32 id, int top, int align, const QVector<QPair<int, quint32>> &more) {
            QByteArray props;
            int count = 0;
            for (const auto &p : more) {
                props += le(quint16(p.first), 2) + le(p.second, 4);
                ++count;
            }
            const QByteArray styles = pfRun(length + 1, 0, 0x800, le(align, 2)) + cfRun(length + 1, 0);
            return pptBox(0xF004, pptRec(0xF00A, le(id, 4) + le(0xA00, 4), 202, 2) + pptRec(0xF00B, props, quint16(count), 3)
                                      + pptRec(0xF010, le(top, 2) + le(2000, 2) + le(2800, 2) + le(top + 400, 2)) + pptBox(0xF00D, pptText(4, words, styles)));
        };
        const QVector<QPair<int, quint32>> none = {{0x0085, 2}}, square = {{0x0085, 0}}, rest = {{0x01BF, 0x00100000}, {0x01FF, 0x00080000}};
        auto both = [&](const QVector<QPair<int, quint32>> &a) { return a + rest; };
        const QByteArray shapes = textBox(0x500, 1000, 0, both(none)) + textBox(0x501, 1500, 1, both(none)) + textBox(0x502, 2000, 2, both(none))
                                  + textBox(0x503, 2500, 0, both(square)) + textBox(0x504, 3000, 0, rest);
        // A drawing group that says "none" for every shape: it is not read.
        auto table = [&](const QVector<QPair<int, quint32>> &props) {
            QByteArray t;
            for (const auto &p : props) t += le(quint16(p.first), 2) + le(p.second, 4);
            return pptRec(0xF00B, t, quint16(props.size()), 3);
        };
        const QByteArray group = pptBox(1035, pptBox(0xF000, pptRec(0xF006, QByteArray(16, '\0')) + table({{0x0085, 2}})));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), group, shapes), &err);
        QVERIFY2(d, qPrintable(err));
        QVector<const TextItem *> boxes;
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Text && it->role.isEmpty()) boxes << static_cast<const TextItem *>(it.get());
        QCOMPARE(int(boxes.size()), 5);
        QVERIFY(d->slideText);
        LayoutCache cache;
        const RenderOptions opt;
        // The lines of a box, and the span the characters of the whole text cover from the box's left edge.
        auto lineCount = [&](const TextItem &t) {
            const auto fl = cache.textFrame(*d, t, 1, opt);
            return fl.layout ? int(fl.layout->lineInfo(0).size()) : -1;
        };
        auto span = [&](const TextItem &t) {
            const auto fl = cache.textFrame(*d, t, 1, opt);
            QRectF all;
            if (fl.layout)
                for (const QRectF &r : fl.layout->rangeRects(0, 0, d->storyDoc(t.storyId)->characterCount() - 1)) all = all.united(r);
            return all;
        };
        for (int i : {0, 1, 2}) {
            QVERIFY2(boxes[i]->noWrap, qPrintable(QString::number(i)));
            QCOMPARE(boxes[i]->rect.width(), 100.0);
            QCOMPARE(lineCount(*boxes[i]), 1);
            QVERIFY2(span(*boxes[i]).width() > 200, qPrintable(QString::number(span(*boxes[i]).width())));
        }
        const double inLeft = boxes[0]->insets.left(), inRight = boxes[0]->insets.right();
        QVERIFY2(std::abs(span(*boxes[0]).left() - inLeft) < 1.0, qPrintable(QString::number(span(*boxes[0]).left())));
        QVERIFY2(std::abs(span(*boxes[1]).center().x() - 50.0) < 1.0, qPrintable(QString::number(span(*boxes[1]).center().x())));
        QVERIFY2(std::abs(span(*boxes[2]).right() - (100.0 - inRight)) < 1.0, qPrintable(QString::number(span(*boxes[2]).right())));
        // Square, and nothing said: wrapped in the box.
        for (int i : {3, 4}) {
            QVERIFY2(!boxes[i]->noWrap, qPrintable(QString::number(i)));
            QVERIFY2(lineCount(*boxes[i]) > 3, qPrintable(QString::number(lineCount(*boxes[i]))));
            QVERIFY(span(*boxes[i]).right() <= 100.0);
        }
    }

    // The text of a shape on a slide is all shown, running out of the shape that is too small for it (the presentation program doesn't cut it at the
    // shape's edge, as a text box's): a shape 30 points tall lays out as many lines as one 400 tall.
    void shapeTextOnASlideRunsOutOfAShapeThatIsTooSmall()
    {
        auto d = deck::newPresentation();
        QVERIFY(d->slideText);
        auto shape = [&](double height) {
            auto s = std::make_shared<ShapeItem>();
            s->shape = QStringLiteral("roundRect");
            s->rect = QRectF(100, 100, 150, height);
            s->storyId = d->createStory();
            d->storyDoc(s->storyId)->setPlainText(QStringLiteral("one two three four five six seven eight nine ten eleven twelve ").repeated(4));
            d->pages[0]->items.push_back(s);
            return s;
        };
        const auto tall = shape(400), small = shape(30);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        QPointF origin;
        const StoryLayout *all = Renderer::shapeTextLayout(ctx, *tall, &origin);
        const StoryLayout *cut = Renderer::shapeTextLayout(ctx, *small, &origin);
        QVERIFY(all && cut);
        QVERIFY(all->lineInfo(0).size() > 3);
        QCOMPARE(cut->lineInfo(0).size(), all->lineInfo(0).size());
    }

    // The text of a shape on a slide is not hyphenated (the presentation program breaks a line at a space), where a publication's shapes hyphenate: a long
    // word in a narrow shape is broken across lines without a hyphen.
    void shapeTextOnASlideIsNotHyphenated()
    {
        auto d = deck::newPresentation();
        QVERIFY(d->slideText);
        auto s = std::make_shared<ShapeItem>();
        s->shape = QStringLiteral("roundRect");
        s->rect = QRectF(100, 100, 110, 400);
        s->storyId = d->createStory();
        QTextDocument *story = d->storyDoc(s->storyId);
        story->setPlainText(QStringLiteral("internationalization internationalization internationalization"));
        // (The language is said, so that the dictionary is a known one.)
        QTextCursor language(story);
        language.select(QTextCursor::Document);
        QTextCharFormat english;
        english.setProperty(tp::Language, QStringLiteral("en-US"));
        language.mergeCharFormat(english);
        d->pages[0]->items.push_back(s);
        auto linesOf = [&] {
            LayoutCache cache;
            PaintContext ctx;
            ctx.doc = d.get();
            ctx.cache = &cache;
            QPointF origin;
            QStringList out;
            for (const auto &line : Renderer::shapeTextLayout(ctx, *s, &origin)->lineInfo(0)) out << line.text;
            return out;
        };
        const QStringList slide = linesOf();
        QVERIFY(slide.size() > 3);
        for (const QString &line : slide) QVERIFY2(!line.contains(QLatin1Char('-')) && !line.contains(QChar::SoftHyphen), qPrintable(slide.join('|')));
        d->slideText = false;
        const QStringList publication = linesOf();
        bool hyphenated = false;
        for (const QString &line : publication) hyphenated = hyphenated || line.contains(QLatin1Char('-')) || line.contains(QChar::SoftHyphen);
        QVERIFY2(hyphenated, qPrintable(publication.join('|')));
    }

    // A shape that says its text shrinks to fit (a:normAutofit, here with the scale 62.5%) shows it at that scale: the size of its letters is the
    // scale of the size the text has (24 points, drawn at 15), where a shape that doesn't say so keeps the size, and the scale is kept in the file.
    void shapeTextOnASlideShrinksAsItsAutofitSays()
    {
        auto d = deck::newPresentation();
        auto shape = [&](const char *name, bool shrinks) {
            auto s = std::make_shared<ShapeItem>();
            s->name = QString::fromLatin1(name);
            s->shape = QStringLiteral("roundRect");
            s->rect = QRectF(100, 100, 150, 60);
            s->storyId = d->createStory();
            QTextDocument *story = d->storyDoc(s->storyId);
            story->setPlainText(QStringLiteral("alpha beta gamma delta epsilon zeta eta theta"));
            QTextCursor c(story);
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontPointSize(24);
            c.mergeCharFormat(f);
            s->shrinks = shrinks;
            s->fitScale = 0.625;
            s->fitLineCut = 10;
            d->pages[0]->items.push_back(s);
            return s;
        };
        const auto plain = shape("Plain", false), shrunk = shape("Shrunk", true);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        QPointF origin;
        const auto big = Renderer::shapeTextLayout(ctx, *plain, &origin)->lineInfo(0), small = Renderer::shapeTextLayout(ctx, *shrunk, &origin)->lineInfo(0);
        QVERIFY(!big.isEmpty() && !small.isEmpty());
        QVERIFY2(qAbs(small[0].pointSize / big[0].pointSize - 0.625) < 0.01, qPrintable(QStringLiteral("%1 and %2").arg(small[0].pointSize).arg(big[0].pointSize)));
        QVERIFY(small.size() < big.size());
        // The scale is kept in the item's data, and in a .pptx (as a:normAutofit's fontScale and lnSpcReduction).
        ShapeItem again;
        again.fromJson(shrunk->toJson());
        QVERIFY(again.shrinks && qAbs(again.fitScale - 0.625) < 1e-9 && qAbs(again.fitLineCut - 10) < 1e-9);
        ShapeItem plainAgain;
        plainAgain.fromJson(plain->toJson());
        QVERIFY(!plainAgain.shrinks);
        QString err;
        auto back = viaPptx(*d, &err);
        QVERIFY2(back, qPrintable(err));
        int found = 0;
        for (const auto &it : back->pages[0]->items) {
            if (it->type() != ItemType::Shape) continue;
            const auto *sh = static_cast<const ShapeItem *>(it.get());
            if (it->name == QLatin1String("Shrunk")) {
                ++found;
                QVERIFY(sh->shrinks);
                QVERIFY(qAbs(sh->fitScale - 0.625) < 0.01 && qAbs(sh->fitLineCut - 10) < 0.5);
            } else if (it->name == QLatin1String("Plain")) {
                ++found;
                QVERIFY(!sh->shrinks);
            }
        }
        QCOMPARE(found, 2);
    }

    // The same, in a shape that holds text: its frame has the flag too, and its text is set unwrapped.
    void importBinaryShapesWithNoWrapSetTheirTextOnOneLine()
    {
        const QString words = QStringLiteral("one two three four five six seven eight nine ten");
        auto shape = [&](quint32 id, int top, quint32 wrap) {
            QByteArray props = le(0x0085, 2) + le(wrap, 4);
            props += le(0x0087, 2) + le(1, 4);
            const QByteArray styles = pfRun(int(words.size()) + 1, 0, 0) + cfRun(int(words.size()) + 1, 0);
            // A rounded rectangle (spt 2) with its text.
            return pptBox(0xF004, pptRec(0xF00A, le(id, 4) + le(0xA00, 4), 2, 2) + pptRec(0xF00B, props, 2, 3)
                                      + pptRec(0xF010, le(top, 2) + le(2000, 2) + le(2800, 2) + le(top + 1600, 2)) + pptBox(0xF00D, pptText(4, words, styles)));
        };
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shape(0x500, 1000, 2) + shape(0x501, 3000, 0)), &err);
        QVERIFY2(d, qPrintable(err));
        QVector<const ShapeItem *> shapes;
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Shape && it->role.isEmpty()) shapes << static_cast<const ShapeItem *>(it.get());
        QCOMPARE(int(shapes.size()), 2);
        QVERIFY(shapes[0]->noWrap);
        QVERIFY(!shapes[1]->noWrap);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        QPointF origin;
        const StoryLayout *one = Renderer::shapeTextLayout(ctx, *shapes[0], &origin);
        QVERIFY(one);
        QCOMPARE(int(one->lineInfo(0).size()), 1);
        const StoryLayout *many = Renderer::shapeTextLayout(ctx, *shapes[1], &origin);
        QVERIFY(many);
        QVERIFY(many->lineInfo(0).size() > 3);
    }

    // A text frame that doesn't wrap is the file's too, in the formats that have it: a:bodyPr wrap="none" in a .pptx (read from the frame
    // or the placeholder it takes after, written back, and "square" for the others), fo:wrap-option="no-wrap" in an .odp, and the item's
    // own data (undo, copies) keep it.
    void textFramesThatDoNotWrapAreKeptByTheFormats()
    {
        Parts p = samplePptx();
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        auto box = [&](int id, const QString &name, const QString &bodyPr, bool textBox, int y) {
            return QStringLiteral("<p:sp><p:nvSpPr><p:cNvPr id=\"%1\" name=\"%2\"/><p:cNvSpPr%3/><p:nvPr/></p:nvSpPr><p:spPr>").arg(id).arg(name, textBox ? QStringLiteral(" txBox=\"1\"") : QString())
                   + xfrm(0, y, 1828800, 457200) + (textBox ? QStringLiteral("<a:noFill/>") : QStringLiteral("<a:prstGeom prst=\"roundRect\"><a:avLst/></a:prstGeom>"))
                   + QStringLiteral("</p:spPr><p:txBody>") + bodyPr + QStringLiteral("<a:lstStyle/><a:p><a:r><a:rPr lang=\"en-US\"/><a:t>one two three four five six seven eight nine ten</a:t></a:r></a:p></p:txBody></p:sp>");
        };
        xml.replace(QStringLiteral("</p:spTree>"),
                    box(30, QStringLiteral("NoWrapBox"), QStringLiteral("<a:bodyPr wrap=\"none\"><a:spAutoFit/></a:bodyPr>"), true, 0)
                        + box(31, QStringLiteral("NoWrapShape"), QStringLiteral("<a:bodyPr wrap=\"none\" rtlCol=\"0\"/>"), false, 600000)
                        + box(32, QStringLiteral("SquareBox"), QStringLiteral("<a:bodyPr wrap=\"square\"/>"), true, 1200000)
                        + box(33, QStringLiteral("PlainBox"), QStringLiteral("<a:bodyPr/>"), true, 1800000) + QStringLiteral("</p:spTree>"));
        p.data["/ppt/slides/slide1.xml"].xml = xml.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        auto flag = [](const Document &doc, const QString &name, bool *found) {
            *found = false;
            for (const auto &it : doc.pages[0]->items) {
                if (it->name != name) continue;
                *found = true;
                if (it->type() == ItemType::Text) return static_cast<const TextItem *>(it.get())->noWrap;
                if (it->type() == ItemType::Shape) return static_cast<const ShapeItem *>(it.get())->noWrap;
            }
            return false;
        };
        bool found = false;
        QVERIFY(flag(*d, QStringLiteral("NoWrapBox"), &found) && found);
        QVERIFY(flag(*d, QStringLiteral("NoWrapShape"), &found) && found);
        QVERIFY(!flag(*d, QStringLiteral("SquareBox"), &found) && found);
        QVERIFY(!flag(*d, QStringLiteral("PlainBox"), &found) && found);
        // Written back: wrap="none" for the two, "square" for the others.
        Exported e;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        auto wrapOf = [](const QString &slide, const QString &name) {
            const int at = slide.indexOf(QStringLiteral("name=\"") + name + QStringLiteral("\""));
            const int body = slide.indexOf(QStringLiteral("<a:bodyPr"), at);
            return slide.mid(body, slide.indexOf(QLatin1Char('>'), body) - body);
        };
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY2(wrapOf(slide, QStringLiteral("NoWrapBox")).contains(QStringLiteral("wrap=\"none\"")), qPrintable(wrapOf(slide, QStringLiteral("NoWrapBox"))));
        QVERIFY2(wrapOf(slide, QStringLiteral("NoWrapShape")).contains(QStringLiteral("wrap=\"none\"")), qPrintable(wrapOf(slide, QStringLiteral("NoWrapShape"))));
        QVERIFY2(wrapOf(slide, QStringLiteral("SquareBox")).contains(QStringLiteral("wrap=\"square\"")), qPrintable(wrapOf(slide, QStringLiteral("SquareBox"))));
        QVERIFY2(wrapOf(slide, QStringLiteral("PlainBox")).contains(QStringLiteral("wrap=\"square\"")), qPrintable(wrapOf(slide, QStringLiteral("PlainBox"))));
        // Read again, the same; and through an .odp.
        auto again = deck::importPresentation(bytes, &err);
        QVERIFY2(again, qPrintable(err));
        QVERIFY(flag(*again, QStringLiteral("NoWrapBox"), &found) && found);
        QVERIFY(flag(*again, QStringLiteral("NoWrapShape"), &found) && found);
        QVERIFY(!flag(*again, QStringLiteral("SquareBox"), &found) && found);
        const QByteArray odp = deck::exportOdp(*d, false, &err);
        QVERIFY2(!odp.isEmpty(), qPrintable(err));
        QMap<QString, QByteArray> zip;
        QVERIFY2(readZip(odp, zip, &err), qPrintable(err));
        const QString content = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        QVERIFY(content.contains(QStringLiteral("fo:wrap-option=\"no-wrap\"")));
        auto fromOdp = deck::importOdp(odp, &err);
        QVERIFY2(fromOdp, qPrintable(err));
        QVERIFY(flag(*fromOdp, QStringLiteral("NoWrapBox"), &found) && found);
        QVERIFY(flag(*fromOdp, QStringLiteral("NoWrapShape"), &found) && found);
        QVERIFY(!flag(*fromOdp, QStringLiteral("SquareBox"), &found) && found);
        QVERIFY(!flag(*fromOdp, QStringLiteral("PlainBox"), &found) && found);
    }

    // A placeholder the drawing gives as a plain rectangle, with no text of its own in the drawing, is a text box in place of it, and keeps
    // what the rectangle said of its text: a title and a body that say wrapText 2 don't wrap.
    void importBinaryRectanglePlaceholdersKeepNoWrap()
    {
        const QByteArray none = pptRec(0xF00B, le(0x0085, 2) + le(2, 4), 1, 3);
        // A title (0xD) and a body (0xE) of their own, below the slide's, each a rectangle with no text in its record.
        const QByteArray shapes = pptShape(0x600, 0xD, QByteArray(), 2000, 200, 5000, 2400, none) + pptShape(0x601, 0xE, QByteArray(), 2600, 200, 5000, 3800, none);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes), &err);
        QVERIFY2(d, qPrintable(err));
        const TextItem *title = nullptr, *body = nullptr;
        for (const auto &it : d->pages[0]->items) {
            if (it->type() != ItemType::Text) continue;
            if (std::abs(it->rect.top() - 250) < 0.5) title = static_cast<const TextItem *>(it.get());
            if (std::abs(it->rect.top() - 325) < 0.5) body = static_cast<const TextItem *>(it.get());
        }
        QVERIFY(title && body);
        QVERIFY(title->role.startsWith(QLatin1String("ph:title")));
        QVERIFY(body->role.startsWith(QLatin1String("ph:body")));
        QVERIFY(title->noWrap);
        QVERIFY(body->noWrap);
        // The slide's own placeholders, which say nothing of it, wrap.
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Text && it->role.startsWith(QLatin1String("ph:")) && it.get() != title && it.get() != body)
                QVERIFY2(!static_cast<const TextItem *>(it.get())->noWrap, qPrintable(it->role));
    }

    // A frame that doesn't wrap passes that to what takes after it: a slide's placeholder takes it from its layout's (in a .pptx, where the
    // placeholder's own a:bodyPr says nothing), a slide moved to a layout takes it from the layout's placeholder, a placeholder made on a
    // slide from the layout copies it, and a layout's frame is written into the master page's styles of an .odp.
    void textFramesTakeNoWrapFromTheirLayouts()
    {
        auto isFooter = [](const Item &it) {
            const QString type = deck::placeholderType(it);
            return type == QLatin1String("dt") || type == QLatin1String("ftr") || type == QLatin1String("sldNum") || type == QLatin1String("hdr");
        };
        // The .pptx: the layout's content placeholder says wrap="none"; the slide's, which is on that layout, says nothing.
        Parts p = samplePptx();
        QString layout = QString::fromUtf8(p.data["/ppt/slideLayouts/slideLayout2.xml"].xml);
        const int at = layout.indexOf(QStringLiteral("name=\"Content Placeholder 2\""));
        QVERIFY(at >= 0);
        const int body = layout.indexOf(QStringLiteral("<a:bodyPr/>"), at);
        QVERIFY(body > at);
        layout.replace(body, int(QStringLiteral("<a:bodyPr/>").size()), QStringLiteral("<a:bodyPr wrap=\"none\"/>"));
        p.data["/ppt/slideLayouts/slideLayout2.xml"].xml = layout.toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        auto flagOf = [](const Document &doc, const Page &page, const QString &name, bool *found) {
            *found = false;
            for (const auto &it : page.items)
                if (it->name == name && it->type() == ItemType::Text) {
                    *found = true;
                    return static_cast<const TextItem *>(it.get())->noWrap;
                }
            Q_UNUSED(doc);
            return false;
        };
        bool found = false;
        QVERIFY(flagOf(*d, *d->pages[0], QStringLiteral("Content Placeholder 2"), &found) && found);
        QVERIFY(!flagOf(*d, *d->pages[0], QStringLiteral("Title 1"), &found) && found);
        // And written back with the frame (the slide's own file says it now).
        Exported e;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        auto again = deck::importPresentation(bytes, &err);
        QVERIFY2(again, qPrintable(err));
        QVERIFY(flagOf(*again, *again->pages[0], QStringLiteral("Content Placeholder 2"), &found) && found);

        // A new presentation: its layout "Two Content" has a title with no wrapping and a content place with none.
        auto fresh = deck::newPresentation();
        const MasterPage *two = deck::layoutOfType(*fresh, QStringLiteral("twoObj"));
        QVERIFY(two && two->items.size() >= 3);
        for (const auto &it : two->items)
            if (it->type() == ItemType::Text && !isFooter(*it)) static_cast<TextItem *>(it.get())->noWrap = true;
        Page &slide = *fresh->pages[0];
        auto *title = static_cast<TextItem *>(slide.items[0].get());
        QVERIFY(!title->noWrap);
        deck::applyLayout(*fresh, slide, *two);
        QVERIFY(title->noWrap);
        // The places of the layout the slide's own fill and the one made for the place it had none for have the frame too.
        int texts = 0;
        for (const auto &it : slide.items)
            if (it->type() == ItemType::Text && !deck::placeholderType(*it).isEmpty() && !isFooter(*it)) {
                ++texts;
                QVERIFY2(static_cast<const TextItem *>(it.get())->noWrap, qPrintable(it->role));
            }
        QVERIFY(texts >= 3);
        // The .odp: the master pages' styles say no-wrap for the placeholders that say it, and wrap for the others.
        auto wrapStyles = [&](const Document &doc, int *noWrap, int *wrap) {
            QString why;
            const QByteArray odp = deck::exportOdp(doc, false, &why);
            QMap<QString, QByteArray> zip;
            if (odp.isEmpty() || !readZip(odp, zip, &why)) return false;
            const QString styles = QString::fromUtf8(zip.value(QStringLiteral("styles.xml")));
            *noWrap = int(styles.count(QStringLiteral("fo:wrap-option=\"no-wrap\"")));
            *wrap = int(styles.count(QStringLiteral("fo:wrap-option=\"wrap\"")));
            return true;
        };
        auto framed = deck::newPresentation();
        for (const auto &m : framed->masters)
            for (const auto &it : m->items)
                if (it->type() == ItemType::Text && !isFooter(*it)) static_cast<TextItem *>(it.get())->noWrap = true;
        int noWrap = 0, wrap = 0;
        QVERIFY(wrapStyles(*framed, &noWrap, &wrap));
        QVERIFY2(noWrap >= 1, qPrintable(QString::number(noWrap)));
        QVERIFY(wrap >= 1);   // (the footers' frames wrap)
        auto plainDeck = deck::newPresentation();
        int none = 0, all = 0;
        QVERIFY(wrapStyles(*plainDeck, &none, &all));
        QCOMPARE(none, 0);
        QVERIFY(all >= 1);
    }

    // Spacing as the file has it, set as the presentation program sets it (the text engine's slide lines): a line of a share (100 for a
    // single line) of 1.2 times the text size whatever the font, its extra spacing above it, and no space before the paragraph at the
    // top of the box.
    void importBinarySpacingFollowsTheProgram()
    {
        // Three paragraphs of the body's 28 points: 100 percent with 20 before, 80 percent with 20 before, and 500 master units exactly.
        const QByteArray styles = pfRun(4, 0, 0x3000, le(100, 2) + le(20, 2)) + pfRun(4, 0, 0x3000, le(80, 2) + le(20, 2)) + pfRun(6, 0, 0x3000, le(quint16(-500), 2) + le(20, 2));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, pptText(1, QStringLiteral("One\rTwo\rThree"), styles)), &err);
        QVERIFY2(d, qPrintable(err));
        QTextDocument *story = placeholderStory(*d, *d->pages[0], QStringLiteral("ph:body:1"));
        QVERIFY(story && story->blockCount() == 3);
        const QTextBlock one = story->begin(), two = one.next(), three = two.next();
        // 20 percent of a line of 1.2 times 28 points before each, the first too.
        QVERIFY(qAbs(one.blockFormat().topMargin() - 0.2 * 28 * 1.2) < 1e-9);
        QVERIFY(qAbs(two.blockFormat().topMargin() - 0.2 * 28 * 1.2) < 1e-9);
        QVERIFY(one.blockFormat().lineHeightType() != QTextBlockFormat::ProportionalHeight || one.blockFormat().lineHeight() == 100);
        QVERIFY(two.blockFormat().lineHeightType() == QTextBlockFormat::ProportionalHeight);
        QCOMPARE(two.blockFormat().lineHeight(), 80.0);
        // A line of so many master units is that many points.
        QVERIFY(three.blockFormat().lineHeightType() == QTextBlockFormat::FixedHeight);
        QCOMPARE(three.blockFormat().lineHeight(), 500.0 / 8);
        // Drawn: the first line at the top of the box; the second below it by the space before and its baseline in its own 80 percent line.
        const TextItem *body = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->role == QLatin1String("ph:body:1")) body = static_cast<const TextItem *>(it.get());
        QVERIFY(body);
        LayoutCache cache;
        const auto lines = cache.textFrame(*d, *body, 1, RenderOptions()).layout->lineInfo(0);
        QCOMPARE(int(lines.size()), 3);
        QVERIFY2(qAbs(lines[0].rect.top() - body->insets.top()) < 0.01, qPrintable(QString::number(lines[0].rect.top())));
        // (The single line has its baseline a font's descent above its bottom; the 80 percent line keeps that descent but no higher than 3/4 of
        // the way down, as probed. Which of the two wins depends on the font's descent, which differs where the machine stands in another font.)
        const double descent = lines[0].rect.bottom() - lines[0].baseline, shorter = 0.8 * 1.2 * 28;
        const double wanted = descent + 0.2 * 1.2 * 28 + std::max(0.75 * shorter, shorter - descent);
        QVERIFY2(qAbs(lines[1].baseline - lines[0].baseline - wanted) < 0.05, qPrintable(QString::number(lines[1].baseline - lines[0].baseline) + QStringLiteral(" vs ") + QString::number(wanted)));
    }

    // Objects on a slide, on its master, and in a group don't wrap text around them: text and objects overlap there.
    void importBinaryObjectsDoNotWrapText()
    {
        const QByteArray shape = pptBox(0xF004, pptRec(0xF00A, le(0x500, 4) + le(0xA00, 4), 1, 2) + pptRec(0xF010, le(100, 2) + le(200, 2) + le(2000, 2) + le(600, 2)));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shape, QByteArray(), shape), &err);
        QVERIFY2(d, qPrintable(err));
        int seen = 0;
        for (const auto &it : d->pages[0]->items) {
            QVERIFY2(it->wrap.mode == Wrap::None, qPrintable(it->role));
            ++seen;
        }
        QVERIFY(seen >= 3);
        for (const auto &m : d->masters)
            for (const auto &it : m->items) QVERIFY2(it->wrap.mode == Wrap::None, qPrintable(it->role));
    }

    // The XML a presentation's writer kept for a shape beside its old properties says what the shape looks like where they can't: its
    // fill and line, and colors by the theme's names (the theme the writer kept with the master). What it doesn't say stays.
    void importBinaryShapesTakeTheirLookFromTheWritersXml()
    {
        auto xmlPackage = [&](const QString &properties) {
            office::PackageWriter w;
            w.addPart(QStringLiteral("/drs/shapexml.xml"),
                      QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:sp xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" "
                                     "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"><p:nvSpPr><p:cNvPr id=\"2\" name=\"R\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr>"
                                     "<p:spPr>%1</p:spPr></p:sp>").arg(properties).toUtf8(),
                      QStringLiteral("application/xml"));
            return w.finish();
        };
        // An old shape: filled blue, with a red line, and the writer's XML as property 0x03A9, whose data follows the table.
        auto rectangle = [&](quint32 id, int top, const QByteArray &package) {
            QByteArray table = le(0x0181, 2) + le(0xFF0000, 4) + le(0x01BF, 2) + le(0x00100010, 4) + le(0x01C0, 2) + le(0x0000FF, 4) + le(0x01FF, 2) + le(0x00080008, 4)
                               + le(0x03A9 | 0x8000, 2) + le(quint32(package.size()), 4);
            return pptBox(0xF004, pptRec(0xF00A, le(id, 4) + le(0xA00, 4), 1, 2) + pptRec(0xF00B, table + package, 5, 3) + pptRec(0xF010, le(top, 2) + le(100, 2) + le(2000, 2) + le(top + 500, 2)));
        };
        const QByteArray shapes = rectangle(0x500, 100, xmlPackage(QStringLiteral("<a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill><a:ln w=\"12700\"><a:noFill/></a:ln>")))
                                  + rectangle(0x501, 800, xmlPackage(QStringLiteral("<a:solidFill><a:schemeClr val=\"accent1\"><a:alpha val=\"50000\"/></a:schemeClr></a:solidFill>")))
                                  + rectangle(0x502, 1500, xmlPackage(QStringLiteral("<a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"10\" cy=\"10\"/></a:xfrm>")))
                                  + rectangle(0x503, 2200, QByteArray("not a package"));
        // The master's theme (as a package) and color mapping, as the writer kept them: accent 1 is magenta.
        office::PackageWriter t;
        t.addPart(QStringLiteral("/theme/theme/theme1.xml"),
                  QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><a:theme xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" name=\"T\"><a:themeElements>"
                             "<a:clrScheme name=\"C\"><a:dk1><a:srgbClr val=\"000000\"/></a:dk1><a:lt1><a:srgbClr val=\"FFFFFF\"/></a:lt1><a:dk2><a:srgbClr val=\"111111\"/></a:dk2>"
                             "<a:lt2><a:srgbClr val=\"EEEEEE\"/></a:lt2><a:accent1><a:srgbClr val=\"FF00FF\"/></a:accent1><a:accent2><a:srgbClr val=\"00FFFF\"/></a:accent2>"
                             "<a:accent3><a:srgbClr val=\"333333\"/></a:accent3><a:accent4><a:srgbClr val=\"444444\"/></a:accent4><a:accent5><a:srgbClr val=\"555555\"/></a:accent5>"
                             "<a:accent6><a:srgbClr val=\"666666\"/></a:accent6><a:hlink><a:srgbClr val=\"0000FF\"/></a:hlink><a:folHlink><a:srgbClr val=\"800080\"/></a:folHlink></a:clrScheme>"
                             "<a:fontScheme name=\"F\"><a:majorFont><a:latin typeface=\"Alpha Sans\"/></a:majorFont><a:minorFont><a:latin typeface=\"Alpha Sans\"/></a:minorFont></a:fontScheme>"
                             "<a:fmtScheme name=\"M\"/></a:themeElements></a:theme>"),
                  QStringLiteral("application/xml"));
        const QByteArray masterExtra = pptRec(1038, t.finish())
                                       + pptRec(1039, QByteArray("<?xml version=\"1.0\"?><p:clrMap xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" bg1=\"lt1\" tx1=\"dk1\" bg2=\"lt2\" tx2=\"dk2\" "
                                                                 "accent1=\"accent1\" accent2=\"accent2\" accent3=\"accent3\" accent4=\"accent4\" accent5=\"accent5\" accent6=\"accent6\" "
                                                                 "hlink=\"hlink\" folHlink=\"folHlink\"/>"));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes, QByteArray(), QByteArray(), masterExtra), &err);
        QVERIFY2(d, qPrintable(err));
        QVector<const Item *> plain;
        for (const auto &it : d->pages[0]->items)
            if (it->role.isEmpty() && it->type() == ItemType::Shape) plain << it.get();
        QCOMPARE(int(plain.size()), 4);
        // Its own color and no line, where the old properties say blue and red.
        QVERIFY(plain[0]->fill.type == Fill::Solid);
        QCOMPARE(plain[0]->fill.color.rgbValue(), QColor(0, 255, 0));
        QVERIFY(plain[0]->stroke.isNone());
        // A color by name is the master's theme's, with its transparency; the line it says nothing of stays.
        QVERIFY(plain[1]->fill.type == Fill::Solid);
        QCOMPARE(plain[1]->fill.color.rgbValue(), QColor(255, 0, 255));
        QVERIFY(qAbs(plain[1]->fill.transparency - 0.5) < 0.01);
        QCOMPARE(plain[1]->stroke.color.rgbValue(), QColor(255, 0, 0));
        // A shape the XML says nothing of the look of, and one whose XML can't be read, stay as the old properties have them.
        QCOMPARE(plain[2]->fill.color.rgbValue(), QColor(0, 0, 255));
        QCOMPARE(plain[2]->stroke.color.rgbValue(), QColor(255, 0, 0));
        QCOMPARE(plain[3]->fill.color.rgbValue(), QColor(0, 0, 255));
        // A theme and a color mapping that can't be read: the colors the XML spells out still show, and the named ones are the defaults'.
        d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes, QByteArray(), QByteArray(),
                                                    pptRec(1038, QByteArray("not a package")) + pptRec(1039, QByteArray("<not xml"))), &err);
        QVERIFY2(d, qPrintable(err));
        plain.clear();
        for (const auto &it : d->pages[0]->items)
            if (it->role.isEmpty() && it->type() == ItemType::Shape) plain << it.get();
        QCOMPARE(int(plain.size()), 4);
        QCOMPARE(plain[0]->fill.color.rgbValue(), QColor(0, 255, 0));
        QVERIFY(plain[1]->fill.type == Fill::Solid);
    }

    // The writer's XML for a shape is a few kilobytes. One that inflates past
    // a megabyte (a hostile file's, a few bytes packed) is not read, and the
    // shape keeps the look its old properties give; nor is more than 64 MB of
    // it read in all.
    void importBinaryWritersXmlIsReadOnlySoLarge()
    {
        auto xmlPackage = [&](qsizetype padding) {
            office::PackageWriter w;
            w.addPart(QStringLiteral("/drs/shapexml.xml"),
                      QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:sp xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\" "
                                 "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"><!--") + QByteArray(padding, ' ')
                          + QByteArray("--><p:nvSpPr><p:cNvPr id=\"2\" name=\"R\"/><p:cNvSpPr/><p:nvPr/></p:nvSpPr>"
                                       "<p:spPr><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></p:spPr></p:sp>"),
                      QStringLiteral("application/xml"));
            return w.finish();
        };
        auto rectangle = [&](quint32 id, int top, const QByteArray &package) {
            QByteArray table = le(0x0181, 2) + le(0xFF0000, 4) + le(0x01BF, 2) + le(0x00100010, 4) + le(0x03A9 | 0x8000, 2) + le(quint32(package.size()), 4);
            return pptBox(0xF004, pptRec(0xF00A, le(id, 4) + le(0xA00, 4), 1, 2) + pptRec(0xF00B, table + package, 3, 3) + pptRec(0xF010, le(top, 2) + le(100, 2) + le(2000, 2) + le(top + 50, 2)));
        };
        auto greens = [](const Document &d) {
            int n = 0;
            for (const auto &it : d.pages[0]->items)
                if (it->role.isEmpty() && it->type() == ItemType::Shape && it->fill.color.rgbValue() == QColor(0, 255, 0).rgb()) ++n;
            return n;
        };
        QString err;
        // One that inflates to 2 MB stays blue; a small one beside it takes its green.
        QByteArray shapes = rectangle(0x500, 100, xmlPackage(2 << 20)) + rectangle(0x501, 300, xmlPackage(10));
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(greens(*d), 1);
        // 120 of 600 KB each: the first 64 MB are read, the rest are not.
        shapes.clear();
        const QByteArray big = xmlPackage(600 << 10);
        for (int i = 0; i < 120; ++i) shapes += rectangle(0x600 + i, 100 + i * 20, big);
        const qint64 ms = bestNanoseconds([&] {
            d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes), &err);
        }, 3, [&] { d.reset(); }) / 1000000;
        QVERIFY2(d, qPrintable(err));
        const int read = greens(*d);
        QVERIFY2(read > 90 && read < 120, qPrintable(QString::number(read)));
        QVERIFY(ms < 30000);
    }

    // The shadow flag of an old file's words is a shadow in the words' effect list, as the presentation program draws it for them (light
    // and near); saved as a .pptx, it is written as one.
    void importBinaryShadowedWordsHaveAShadow()
    {
        const QByteArray body = pptText(1, QStringLiteral("Plain Shaded"), pfRun(13, 0, 0) + cfRun(6, 0) + cfRun(7, 0x10, le(0x10, 2)));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, body), &err);
        QVERIFY2(d, qPrintable(err));
        const QTextDocument *story = placeholderStory(*d, *d->pages[0], QStringLiteral("ph:body:1"));
        QVERIFY(story);
        QString plain, shaded;
        for (QTextBlock::iterator it = story->begin().begin(); !it.atEnd(); ++it) {
            const QString list = it.fragment().charFormat().stringProperty(tp::TextFx);
            if (it.fragment().text().contains(QStringLiteral("Plain"))) plain = list;
            if (it.fragment().text().contains(QStringLiteral("Shaded"))) shaded = list;
        }
        QVERIFY(plain.isEmpty());
        QVERIFY(!shaded.isEmpty());
        const TextEffects fx = TextEffects::fromJson(shaded);
        QVERIFY(fx.outer.on && !fx.inner.on && !fx.glow.on);
        QCOMPARE(fx.outer.angle, 45.0);
        QVERIFY(fx.outer.distance > 0.5 && fx.outer.distance < 3);
        QVERIFY(fx.outer.color.alpha() > 40 && fx.outer.color.alpha() < 130);
        QCOMPARE(fx.outer.color.rgb(), QColor(Qt::black).rgb());
        Exported e;
        QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(unpack(bytes, &e, &err), qPrintable(err));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY(slide.contains(QStringLiteral("<a:outerShdw")));
        QVERIFY(slide.contains(QStringLiteral("dir=\"2700000\"")));
    }

    // A long text's runs of characters each carry a running count in bits 10 and up of the style and of the mask. Past 15 the count spills into
    // bits 14 and 15, which the format leaves unnamed, and the style is still there: the size after it stays the size of every run that follows.
    void importBinaryRunsKeepTheirSizeWhenTheStyleCountSpillsPastFourBits()
    {
        QString text;
        QByteArray characters;
        for (int n = 1; n <= 40; ++n) {
            text += QStringLiteral("ab ");
            characters += cfRun(n == 40 ? 4 : 3, 0x20000 | (n << 10), le(0x10 | (n << 10), 2) + le(18, 2));
        }
        const QByteArray body = pptText(1, text, pfRun(int(text.size()) + 1, 0, 0) + characters);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, body), &err);
        QVERIFY2(d, qPrintable(err));
        const QTextDocument *story = placeholderStory(*d, *d->pages[0], QStringLiteral("ph:body:1"));
        QVERIFY(story);
        int fragments = 0;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it, ++fragments)
                QVERIFY2(it.fragment().charFormat().fontPointSize() == 18, qPrintable(QString::number(it.fragment().charFormat().fontPointSize())));
        QVERIFY(fragments > 0);
    }

    // A blank paragraph keeps its bullet as the file has it (text typed in it shows one), and shows none, as in the program.
    void importBinaryBlankParagraphsShowNoBullets()
    {
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, pptText(1, QStringLiteral("One\r\rTwo\r"))), &err);
        QVERIFY2(d, qPrintable(err));
        QTextDocument *story = placeholderStory(*d, *d->pages[0], QStringLiteral("ph:body:1"));
        QVERIFY(story);
        QCOMPARE(story->blockCount(), 4);
        const QTextBlock one = story->begin(), blank = one.next(), two = blank.next();
        QVERIFY(one.textList() && blank.textList() && two.textList());
        QCOMPARE(blank.blockFormat().leftMargin(), one.blockFormat().leftMargin());
        const TextItem *body = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->role == QLatin1String("ph:body:1")) body = static_cast<const TextItem *>(it.get());
        QVERIFY(body);
        LayoutCache cache;
        const auto fl = cache.textFrame(*d, *body, 1, RenderOptions());
        const auto lines = fl.layout->lineInfo(0);
        QVERIFY(lines.size() >= 3);
        QImage img((body->rect.size() * 4).toSize(), QImage::Format_ARGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.scale(4, 4);
        fl.layout->paint(&p, 0, PaintOptions());
        p.end();
        // Ink where the first line's bullet is, beside the first line and the blank one.
        auto ink = [&](const QRectF &r) {
            int n = 0;
            for (int y = int(r.top() * 4); y < int(r.bottom() * 4) && y < img.height(); ++y)
                for (int x = std::max(0, int(r.left() * 4)); x < int(r.right() * 4) && x < img.width(); ++x)
                    n += qGray(img.pixel(x, y)) < 128;
            return n;
        };
        const double bulletRight = lines[0].rect.left();
        QVERIFY(ink(QRectF(0, lines[0].rect.top(), bulletRight, lines[0].rect.height())) > 0);
        QCOMPARE(ink(QRectF(0, lines[1].rect.top(), bulletRight, lines[1].rect.height())), 0);
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

    // A pattern fill (fillType 1) is a bitmap of black and white that the shape's own colors are put in, as the presentation program draws
    // it: the bitmap's white bits in the fill color, its black ones in the back color (a master's striped background has three rows of black
    // to one of white, and is drawn with three rows of the back color to one of the fill color), each with its opacity. A bitmap of
    // more colors is a picture and keeps them.
    void importBinaryPatternsTakeTheShapesColors()
    {
        // An 8 by 8 bitmap of one bit a pixel: white on its left half, black on its right (a DIB's palette is white, then black).
        QByteArray dib = le(40, 4) + le(8, 4) + le(8, 4) + le(1, 2) + le(1, 2) + le(0, 4) + le(32, 4) + le(2834, 4) + le(2834, 4) + le(2, 4) + le(2, 4)
                         + raw({0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0});
        for (int row = 0; row < 8; ++row) dib += raw({0x0F, 0, 0, 0});
        QImage photo(2, 1, QImage::Format_ARGB32);
        photo.setPixelColor(0, 0, QColor(255, 0, 0));
        photo.setPixelColor(1, 0, QColor(0, 0, 128));
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        photo.save(&buffer, "PNG");
        const QByteArray bitmap = pptRec(0xF01F, QByteArray(16, 'u') + QByteArray(1, '\xFF') + dib, 0x7A0), picture = pptRec(0xF01E, QByteArray(16, 'v') + QByteArray(1, '\xFF') + png, 0x6E0);
        auto entry = [&](const QByteArray &blip, char uid) {
            return pptRec(0xF007, QByteArray(1, '\x06') + QByteArray(1, '\0') + QByteArray(16, uid) + le(0xFF, 2) + le(quint32(blip.size()), 4) + le(1, 4) + le(0xFFFFFFFF, 4) + le(0, 4) + blip, 6, 2);
        };
        const QByteArray group = pptBox(1035, pptBox(0xF000, pptRec(0xF006, QByteArray(16, '\0')) + pptBox(0xF001, entry(bitmap, 's') + entry(picture, 't'), 2)));
        auto table = [&](const QVector<QPair<int, quint32>> &props) {
            QByteArray t;
            for (const auto &p : props) t += le(quint16(p.first), 2) + le(p.second, 4);
            return pptRec(0xF00B, t, quint16(props.size()), 3);
        };
        // The background's pixels in its first row: red where the fill color shows (the fill color is red, the back color blue).
        double transparency = -1;
        auto pixels = [&](int blip, const QVector<QPair<int, quint32>> &more) {
            QVector<QPair<int, quint32>> props = {{0x0180, 1}, {0x4186, quint32(blip)}, {0x0181, 0x0000FF}, {0x0183, 0xFF0000}, {0x01BF, 0x00100010}};
            props += more;
            const QByteArray background = pptBox(0xF004, pptRec(0xF00A, le(0x1000, 4) + le(0xC00, 4), 1, 2) + table(props));
            QString err;
            auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), group, QByteArray(), QByteArray(), background), &err);
            QList<QColor> out;
            if (!d) qWarning("%s", qPrintable(err));
            if (!d || d->masters.isEmpty() || d->masters.first()->background.type != Fill::Texture) return out;
            transparency = d->masters.first()->background.transparency;
            const QImage im = d->image(d->masters.first()->background.imageId);
            for (int x = 0; x < im.width(); ++x) out << im.pixelColor(x, 0);
            return out;
        };
        const QList<QColor> plain = pixels(1, {});
        QCOMPARE(transparency, 0.0);   // (the colors hold the opacity)
        QCOMPARE(int(plain.size()), 8);
        for (int x = 0; x < 8; ++x) QCOMPARE(plain[x], x < 4 ? QColor(255, 0, 0) : QColor(0, 0, 255));
        // Half as opaque a fill color, and a back color half as opaque.
        const QList<QColor> half = pixels(1, {{0x0182, 0x8000}, {0x0184, 0x4000}});
        QCOMPARE(int(half.size()), 8);
        QCOMPARE(half[0].rgb(), QColor(255, 0, 0).rgb());
        QVERIFY2(std::abs(half[0].alpha() - 128) <= 1, qPrintable(QString::number(half[0].alpha())));
        QCOMPARE(half[7].rgb(), QColor(0, 0, 255).rgb());
        QVERIFY2(std::abs(half[7].alpha() - 64) <= 1, qPrintable(QString::number(half[7].alpha())));
        // A picture of other colors than black and white is drawn as it is.
        const QList<QColor> kept = pixels(2, {});
        QCOMPARE(int(kept.size()), 2);
        QCOMPARE(kept[0], QColor(255, 0, 0));
        QCOMPARE(kept[1], QColor(0, 0, 128));
    }

    // A file with a pattern fill for each of hundreds of colors makes a picture for no more than 200 of them: the others keep the bitmap, so
    // no file can make the reader do the work again and again. A bitmap bigger than a pattern (257 by 256, 65,792 pixels, past the 65,536 a
    // pattern may have) is a picture, and keeps its pixels.
    void importBinaryPatternsInManyColorsAreBounded()
    {
        QByteArray dib = le(40, 4) + le(8, 4) + le(8, 4) + le(1, 2) + le(1, 2) + le(0, 4) + le(32, 4) + le(2834, 4) + le(2834, 4) + le(2, 4) + le(2, 4)
                         + raw({0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0});
        for (int row = 0; row < 8; ++row) dib += raw({0x0F, 0, 0, 0});
        const QByteArray blip = pptRec(0xF01F, QByteArray(16, 'u') + QByteArray(1, '\xFF') + dib, 0x7A0);
        // The big one: 257 pixels a row (36 bytes with the padding), 256 rows, all of its bits set (black).
        QByteArray bigDib = le(40, 4) + le(257, 4) + le(256, 4) + le(1, 2) + le(1, 2) + le(0, 4) + le(0, 4) + le(2834, 4) + le(2834, 4) + le(2, 4) + le(2, 4)
                            + raw({0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0});
        bigDib += QByteArray(36 * 256, '\xFF');
        const QByteArray bigBlip = pptRec(0xF01F, QByteArray(16, 'v') + QByteArray(1, '\xFF') + bigDib, 0x7A0);
        auto storeEntry = [&](const QByteArray &record, char uid) {
            return pptRec(0xF007, QByteArray(1, '\x06') + QByteArray(1, '\0') + QByteArray(16, uid) + le(0xFF, 2) + le(quint32(record.size()), 4) + le(1, 4) + le(0xFFFFFFFF, 4) + le(0, 4) + record, 6, 2);
        };
        const QByteArray group = pptBox(1035, pptBox(0xF000, pptRec(0xF006, QByteArray(16, '\0')) + pptBox(0xF001, storeEntry(blip, 's') + storeEntry(bigBlip, 't'), 2)));
        QByteArray shapes;
        for (int i = 0; i < 400; ++i) {
            const QByteArray props = le(0x0180, 2) + le(1, 4) + le(0x4186, 2) + le(1, 4) + le(0x0181, 2) + le(quint32(0x010000 + i * 0x300), 4) + le(0x01BF, 2) + le(0x00100010, 4);
            shapes += pptBox(0xF004, pptRec(0xF00A, le(0x500 + i, 4) + le(0xA00, 4), 1, 2) + pptRec(0xF00B, props, 4, 3) + pptRec(0xF010, le(100, 2) + le(100, 2) + le(300, 2) + le(300, 2)));
        }
        {
            // The last shape's pattern is the big bitmap, in a color of its own.
            const QByteArray props = le(0x0180, 2) + le(1, 4) + le(0x4186, 2) + le(2, 4) + le(0x0181, 2) + le(0x0000FF, 4) + le(0x0183, 2) + le(0xFF0000, 4) + le(0x01BF, 2) + le(0x00100010, 4);
            shapes += pptBox(0xF004, pptRec(0xF00A, le(0x900, 4) + le(0xA00, 4), 1, 2) + pptRec(0xF00B, props, 5, 3) + pptRec(0xF010, le(400, 2) + le(100, 2) + le(600, 2) + le(300, 2)));
        }
        QString err;
        std::unique_ptr<Document> d;
        const qint64 ms = bestNanoseconds([&] {
            d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), group, shapes), &err);
        }, 3, [&] { d.reset(); }) / 1000000;
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(ms < 30000, qPrintable(QString::number(ms)));
        QVERIFY2(d->images.size() <= 203, qPrintable(QString::number(d->images.size())));
        QVERIFY(d->images.size() >= 100);
        int tiles = 0;
        for (const auto &it : d->pages[0]->items) tiles += it->fill.type == Fill::Texture;
        QCOMPARE(tiles, 401);
        // The big bitmap is as the file has it: its size, and black (not the shape's red or blue).
        const Item *big = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->fill.type == Fill::Texture) big = it.get();
        QVERIFY(big);
        const QImage bitmap = d->image(big->fill.imageId);
        QCOMPARE(bitmap.size(), QSize(257, 256));
        QCOMPARE(bitmap.pixelColor(0, 0), QColor(Qt::black));
        QCOMPARE(bitmap.pixelColor(256, 255), QColor(Qt::black));
    }

    // Text in a symbol font shows the font's own pictures: a run that names a font for its symbols (character run, the fourth font reference)
    // keeps them at 0xF020 to 0xF0FF, and a run set in the font itself has the plain codes. A Greek letter, an arrow, or a number in a circle
    // is the Unicode character that looks like it, in the text's own font (so it shows wherever the symbol font isn't); one that has no
    // look-alike stays in a piece set in the symbol font, where the font is. Text of a font that isn't a symbol font is as it was.
    void importBinarySymbolFontTextBecomesItsPictures()
    {
        // The fonts of the buildPpt document: 0 Alpha Sans, 1 Beta Serif, 2 Symbol, 3 Wingdings 2.
        const QChar alpha(0x3B1), arrow(0x2192), circled(0x2776), none(0xF0FF);
        const QString text = QStringLiteral("x") + QChar(0xF061) + QStringLiteral("y") + none + QChar(0xF0AE) + QStringLiteral("z") + QChar(0xF075) + QStringLiteral("a") + QStringLiteral("b");
        // x; alpha (Symbol, as a symbol run); y; the character Symbol lacks; an arrow; z; a number in a circle (Wingdings 2); then "a" set
        // in Symbol itself; and "b" in the text's own font. The last run has the paragraph's end too.
        const quint32 sym = 0x800000, own = 0x10000;
        const QByteArray styles = pfRun(int(text.size()) + 1, 0, 0)
            + cfRun(1, 0) + cfRun(1, sym, le(2, 2)) + cfRun(1, 0) + cfRun(1, sym, le(2, 2)) + cfRun(1, sym, le(2, 2)) + cfRun(1, 0) + cfRun(1, sym, le(3, 2))
            + cfRun(1, own, le(2, 2)) + cfRun(2, own, le(0, 2));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, pptText(1, text, styles, true)), &err);
        QVERIFY2(d, qPrintable(err));
        const QTextDocument *body = placeholderStory(*d, *d->pages[0], QStringLiteral("ph:body:1"));
        QVERIFY(body);
        QString shown;
        QMap<QChar, QStringList> families;   // each character -> the fonts of its piece
        for (QTextBlock b = body->begin(); b.isValid(); b = b.next())
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                shown += it.fragment().text();
                for (const QChar ch : it.fragment().text()) families[ch] = it.fragment().charFormat().fontFamilies().toStringList();
            }
        QCOMPARE(shown, QStringLiteral("x") + alpha + QStringLiteral("y") + none + arrow + QStringLiteral("z") + circled + alpha + QStringLiteral("b"));
        // The character with no look-alike is a piece of its own in Symbol; the others are not in it: the letter alpha of a run set in Symbol
        // is in the text's font too, and "b" is in the font its run names.
        QCOMPARE(families.value(none), QStringList{QStringLiteral("Symbol")});
        for (const QChar ch : {QChar('x'), alpha, QChar('y'), arrow, QChar('z'), circled}) QVERIFY2(!families.value(ch).contains(QStringLiteral("Symbol")), qPrintable(QString(ch)));
        QCOMPARE(families.value(QChar('b')), QStringList{QStringLiteral("Alpha Sans")});
    }

    // The split of text in a symbol font adds at most 256 pieces to a paragraph: a run of 50,000 characters that alternate between a
    // letter the font has a look-alike for and one it hasn't would be 50,000 pieces, each laid out by itself, and takes far longer than
    // the same text in a font of letters. The rest of the run after them is one piece, as it was before the split.
    void importBinarySymbolRunsAreCutIntoFewPieces()
    {
        QString text;
        for (int i = 0; i < 25000; ++i) text += QStringLiteral("a") + QChar(0x100);
        const quint32 own = 0x10000;
        auto deckOf = [&](int font) {
            const QByteArray styles = pfRun(int(text.size()) + 1, 0, 0) + cfRun(int(text.size()) + 1, own, le(font, 2));
            QString err;
            auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, pptText(1, text, styles, true)), &err);
            if (!d) qWarning("%s", qPrintable(err));
            return d;
        };
        auto fragments = [&](const Document &d) {
            int n = 0;
            const QTextDocument *body = placeholderStory(d, *d.pages[0], QStringLiteral("ph:body:1"));
            if (body)
                for (QTextBlock b = body->begin(); b.isValid(); b = b.next())
                    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) ++n;
            return n;
        };
        // The font Symbol (2), and a font of letters (0).
        auto symbols = deckOf(2), letters = deckOf(0);
        QVERIFY(symbols && letters);
        QCOMPARE(fragments(*letters), 1);
        QVERIFY2(fragments(*symbols) <= 256 + 8, qPrintable(QString::number(fragments(*symbols))));
        QVERIFY(fragments(*symbols) > 2);   // (the first of them are cut as before)
        // Laid out, it costs about what the same text in another font does.
        auto layoutMs = [&](Document &d) {
            const TextItem *body = nullptr;
            for (const auto &it : d.pages[0]->items)
                if (it->type() == ItemType::Text && it->role == QLatin1String("ph:body:1")) body = static_cast<const TextItem *>(it.get());
            if (!body) return qint64(-1);
            const RenderOptions opt;
            std::unique_ptr<LayoutCache> cache;
            bool laid = false;
            const qint64 ns = bestNanoseconds([&] { laid = bool(cache->textFrame(d, *body, 1, opt).layout); }, 5, [&] {
                cache = std::make_unique<LayoutCache>();   // a cache with nothing laid out, for each run
            });
            return laid ? ns / 1000000 : qint64(-1);
        };
        const qint64 plain = layoutMs(*letters), cut = layoutMs(*symbols);
        QVERIFY2(plain >= 0 && cut >= 0, "the text was laid out");
        QVERIFY2(cut <= 5 * std::max<qint64>(plain, 100), qPrintable(QStringLiteral("cut %1 ms, plain %2 ms").arg(cut).arg(plain)));
    }

    // In 8-bit text the codes 0x80 to 0x9F read as Windows-1252's characters, which a symbol font doesn't have: a run set in the font gets
    // its picture from the code itself (0x8C is the number 1 in a dark circle in Wingdings, 0x9F a bullet), where the same codes in
    // another font are those characters. The pictures that are emoji are followed by U+FE0E, for the plain glyph.
    void importBinarySymbolFontBytesAreReadByTheirCodes()
    {
        // 0x8C, 0x9F, then 'C' (a thumb up in Wingdings) and 'F' (a hand that is not an emoji), as bytes; the fonts are 4 Wingdings, 0 Alpha Sans.
        const QByteArray raw8 = QByteArray("\x8C\x9F" "CF");
        auto textIn = [&](int font) {
            const QByteArray styles = pfRun(int(raw8.size()) + 1, 0, 0) + cfRun(int(raw8.size()) + 1, 0x10000, le(font, 2));
            QByteArray text = pptRec(3999, le(1, 4)) + pptRec(4008, raw8) + pptRec(4001, styles);
            QString err;
            auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, text), &err);
            if (!d) qWarning("%s", qPrintable(err));
            const QTextDocument *body = d ? placeholderStory(*d, *d->pages[0], QStringLiteral("ph:body:1")) : nullptr;
            return body ? body->toPlainText() : QStringLiteral("<none>");
        };
        const QChar plain(0xFE0E);
        QCOMPARE(textIn(4), QString(QChar(0x2776)) + QChar(0x2022) + QString::fromUcs4(U"\U0001F44D") + plain + QChar(0x261E));
        // In a font of letters, the Windows-1252 characters they were.
        QCOMPARE(textIn(0), QString(QChar(0x0152)) + QChar(0x0178) + QStringLiteral("CF"));
    }

    // The pictures that only the word processor knows (the arrows of Wingdings E7, E9, and EA) are not read as pictures here: a run in Wingdings with those codes keeps the
    // characters the codes were, as in a font of letters, and a presentation saved from it says the same.
    void importBinaryWingdingsArrowsOfTheWordProcessorStayAsTheyWere()
    {
        const QByteArray raw8 = QByteArray("\xE7\xE9\xEA");
        auto textIn = [&](int font) {
            const QByteArray styles = pfRun(int(raw8.size()) + 1, 0, 0) + cfRun(int(raw8.size()) + 1, 0x10000, le(font, 2));
            QByteArray text = pptRec(3999, le(1, 4)) + pptRec(4008, raw8) + pptRec(4001, styles);
            QString err;
            auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, text), &err);
            if (!d) qWarning("%s", qPrintable(err));
            const QTextDocument *body = d ? placeholderStory(*d, *d->pages[0], QStringLiteral("ph:body:1")) : nullptr;
            return body ? body->toPlainText() : QStringLiteral("<none>");
        };
        QCOMPARE(textIn(0), QString(QChar(0x00E7)) + QChar(0x00E9) + QChar(0x00EA));
        QCOMPARE(textIn(4), textIn(0));
    }

    // A metafile whose writer sets a smaller window and scales the viewport down by the same ratio (the program's own clip art does, to
    // keep a drawing's size) is drawn at the size it had: here a drawing 400 by 300 units that sets a window of 200 by 150 and halves
    // the viewport, then fills the whole of that window, covers the top left quarter of its picture, not all of it.
    void renderMetafileKeepsTheScaleOfAViewportScaledWithItsWindow()
    {
        auto le16 = [](QByteArray &b, int v) { b.append(char(v & 0xFF)); b.append(char((v >> 8) & 0xFF)); };
        auto le32 = [](QByteArray &b, quint32 v) { for (int i = 0; i < 4; ++i) b.append(char((v >> (8 * i)) & 0xFF)); };
        auto words = [&](std::initializer_list<int> v) { QByteArray b; for (int w : v) le16(b, w); return b; };
        auto record = [&](int fn, const QByteArray &params) {
            QByteArray b;
            le32(b, quint32((6 + params.size() + (params.size() & 1)) / 2));
            le16(b, fn);
            b += params;
            if (params.size() & 1) b.append('\0');
            return b;
        };
        QByteArray brush = words({0}), pen = words({5, 0, 0});
        le32(brush, 0x00646464);
        le16(brush, 0);
        le32(pen, 0);
        // The parameters of a record come last first: the viewport's scale is the height's denominator and numerator, then the width's.
        QByteArray body = record(0x020B, words({0, 0})) + record(0x020C, words({300, 400})) + record(0x02FC, brush) + record(0x02FA, pen);
        body += record(0x0412, words({300, 150, 400, 200})) + record(0x020C, words({150, 200}));
        body += record(0x012D, words({1})) + record(0x012D, words({0})) + record(0x041B, words({150, 200, 0, 0})) + record(0x0000, QByteArray());
        QByteArray head, standard;
        le32(head, 0x9AC6CDD7u);
        le16(head, 0);
        for (int v : {0, 0, 400, 300, 72}) le16(head, v);
        le32(head, 0);
        quint16 sum = 0;
        for (int i = 0; i < 20; i += 2) sum ^= quint16(quint8(head[i]) | (quint8(head[i + 1]) << 8));
        le16(head, sum);
        for (int v : {1, 9, 0x300}) le16(standard, v);
        le32(standard, quint32((18 + body.size()) / 2));
        le16(standard, 2);
        le32(standard, 60);
        le16(standard, 0);
        const QByteArray wmf = head + standard + body;
        Document doc;
        doc.slidePictures = true;
        auto pic = std::make_shared<PictureItem>();
        pic->imageId = doc.addImage(wmf, QStringLiteral("wmf"));
        QVERIFY(!pic->imageId.isEmpty());
        pic->rect = QRectF(0, 0, 200, 150);
        pic->imgRect = pic->rect;
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = &doc;
        ctx.cache = &cache;
        const ItemList one{pic};
        const QImage img = Renderer::renderItemsToImage(ctx, one, 1.0);
        QVERIFY(!img.isNull());
        // A point in the top left quarter is the box's gray; one in the bottom right is not (nothing is drawn there).
        const QColor inside = img.pixelColor(img.width() / 4, img.height() / 4), outside = img.pixelColor(img.width() * 3 / 4, img.height() * 3 / 4);
        QVERIFY2(qAbs(inside.red() - 100) <= 2 && inside.alpha() == 255, qPrintable(inside.name(QColor::HexArgb)));
        QVERIFY2(outside.alpha() == 0 || outside.red() > 200, qPrintable(outside.name(QColor::HexArgb)));
    }

    // A presentation whose document says (in its headers and footers record) that the slides show a slide number and a footer text shows
    // them on the slides that lack them, wherever the slide's master has the place for them; a slide that has a place for one of them of its own
    // keeps it as it is, and gets the others. A document that says nothing of them shows none. The footers are kept when it is saved as a
    // .pptx or as an .odp and opened again.
    void importBinaryShowsTheFootersTheDocumentSetsOnSlidesWithoutThem()
    {
        // The flags: slide number (8) and footer (0x20); the footer's text is the string of instance 2.
        const QByteArray footers = pptBox(4057, pptRec(4058, le(0, 2) + le(0x28, 2)) + pptRec(4026, utf16(QStringLiteral("Quarterly review")), 2), 3);
        // The master's places for the footer (9) and the slide number (8), and the first slide's own number (8), holding 7.
        const QByteArray masterPlaces = pptShape(0x410, 9, QByteArray(), 4000, 200, 3000, 4300) + pptShape(0x411, 8, QByteArray(), 4000, 4000, 5500, 4300);
        const QByteArray ownNumber = pptShape(0x420, 8, pptRec(3999, le(4, 4)) + pptRec(4000, utf16(QStringLiteral("7"))), 4000, 4000, 5500, 4300);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), ownNumber, QByteArray(), masterPlaces, QByteArray(), 1016, footers), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 2);
        auto count = [](const Page &p, const QString &type) {
            int n = 0;
            for (const auto &it : p.items) n += deck::placeholderType(*it) == type;
            return n;
        };
        // The second slide has both, the footer with the text and the number as the field of the slide's number.
        QCOMPARE(count(*d->pages[1], QStringLiteral("ftr")), 1);
        QCOMPARE(count(*d->pages[1], QStringLiteral("sldNum")), 1);
        QCOMPARE(count(*d->pages[1], QStringLiteral("dt")), 0);
        QCOMPARE(placeholderText(*d, *d->pages[1], QStringLiteral("ph:ftr")), QStringLiteral("Quarterly review"));
        const QTextDocument *number = placeholderStory(*d, *d->pages[1], QStringLiteral("ph:sldNum"));
        QVERIFY(number);
        QCOMPARE(number->toPlainText(), QString(QChar::ObjectReplacementCharacter));
        // The first slide keeps its own number, and gets the footer it lacked.
        QCOMPARE(count(*d->pages[0], QStringLiteral("sldNum")), 1);
        QCOMPARE(count(*d->pages[0], QStringLiteral("ftr")), 1);
        QCOMPARE(placeholderText(*d, *d->pages[0], QStringLiteral("ph:sldNum")), QStringLiteral("7"));
        QCOMPARE(placeholderText(*d, *d->pages[0], QStringLiteral("ph:ftr")), QStringLiteral("Quarterly review"));
        // Saved as a .pptx or an .odp and opened again, the second slide has the footer's text and the number's field.
        for (const bool odp : {false, true}) {
            auto again = odp ? viaOdp(*d, &err) : viaPptx(*d, &err);
            QVERIFY2(again, qPrintable(err));
            QCOMPARE(int(again->pages.size()), 2);
            QCOMPARE(count(*again->pages[1], QStringLiteral("ftr")), 1);
            QCOMPARE(placeholderText(*again, *again->pages[1], QStringLiteral("ph:ftr")), QStringLiteral("Quarterly review"));
            QCOMPARE(count(*again->pages[1], QStringLiteral("sldNum")), 1);
            const QTextDocument *again2 = placeholderStory(*again, *again->pages[1], QStringLiteral("ph:sldNum"));
            QVERIFY(again2);
            QCOMPARE(again2->toPlainText(), QString(QChar::ObjectReplacementCharacter));
            QCOMPARE(count(*again->pages[1], QStringLiteral("dt")), 0);
            QCOMPARE(count(*again->pages[0], QStringLiteral("sldNum")), 1);
        }
        // With no record of them, no slide shows them.
        auto plain = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), masterPlaces), &err);
        QVERIFY2(plain, qPrintable(err));
        for (const auto &p : plain->pages) QCOMPARE(count(*p, QStringLiteral("ftr")) + count(*p, QStringLiteral("sldNum")), 0);
    }

    // The extension that comes with the outline's text (a slide's text, by its slide and its kind) can number the paragraphs of a run of the
    // text: the first run here is numbered the usual way ("1."), the second counts from 5 with a parenthesis ("5)", "6)"), the third has no
    // numbering and keeps the master's bullet. The entries go with the runs of paragraphs in order.
    void importBinaryNumbersTheParagraphsTheTextExtensionNames()
    {
        auto entry = [&](quint32 masks, const QByteArray &fields) { return le(masks, 4) + fields + le(0, 4) + le(0, 4); };
        // bulletHasScheme (0x02000000), and with bulletScheme (0x01000000): the number's scheme (2 is "1)") and where it starts.
        const QByteArray atom = entry(0x02000000, le(1, 2)) + entry(0x03000000, le(1, 2) + le(2, 2) + le(5, 2)) + entry(0, QByteArray());
        const QByteArray outline = pptBox(4014, pptRec(4015, le(0x100, 4) + le(1, 4), 1) + pptRec(4012, atom));
        const QByteArray tags = pptBox(5000, pptBox(5002, pptRec(4026, utf16(QStringLiteral("___PPT9"))) + pptRec(5003, outline)));
        // Three paragraphs, each a run of its own (12, 13, and 12 characters; the master's bullets are on for both levels).
        const QByteArray body = pptText(1, QStringLiteral("First point\rSecond point\rThird point"), pfRun(12, 0, 0) + pfRun(13, 1, 0) + pfRun(12, 1, 0) + cfRun(37, 0));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, body, QByteArray(), QByteArray(), QByteArray(), QByteArray(), QByteArray(), 1016, tags), &err);
        QVERIFY2(d, qPrintable(err));
        const QTextDocument *story = placeholderStory(*d, *d->pages[0], QStringLiteral("ph:body:1"));
        QVERIFY(story);
        QCOMPARE(story->blockCount(), 3);
        const QTextBlock first = story->firstBlock(), second = first.next(), third = second.next();
        QVERIFY(first.textList() && second.textList() && third.textList());
        const QTextListFormat one = first.textList()->format(), two = second.textList()->format(), none = third.textList()->format();
        QCOMPARE(one.style(), QTextListFormat::ListDecimal);
        QCOMPARE(one.numberSuffix(), QStringLiteral("."));
        QCOMPARE(one.start(), 1);
        QCOMPARE(two.style(), QTextListFormat::ListDecimal);
        QCOMPARE(two.numberSuffix(), QStringLiteral(")"));
        QCOMPARE(two.start(), 5);
        QCOMPARE(none.style(), QTextListFormat::ListDisc);
        // Without the extension the same text is bulleted.
        auto plain = deck::importBinaryPresentation(buildPpt(false, false, false, false, body), &err);
        QVERIFY2(plain, qPrintable(err));
        const QTextDocument *plainStory = placeholderStory(*plain, *plain->pages[0], QStringLiteral("ph:body:1"));
        QVERIFY(plainStory && plainStory->firstBlock().textList());
        QCOMPARE(plainStory->firstBlock().textList()->format().style(), QTextListFormat::ListDisc);
        // An empty paragraph between two numbered ones is no item of the list: it has no number, and the numbering goes on past it.
        const QByteArray gapOutline = pptBox(4014, pptRec(4015, le(0x100, 4) + le(1, 4), 1) + pptRec(4012, entry(0x02000000, le(1, 2))));
        const QByteArray gapTags = pptBox(5000, pptBox(5002, pptRec(4026, utf16(QStringLiteral("___PPT9"))) + pptRec(5003, gapOutline)));
        const QByteArray gapBody = pptText(1, QStringLiteral("Alpha\r\rBeta"), pfRun(12, 0, 0) + cfRun(12, 0));
        auto gap = deck::importBinaryPresentation(buildPpt(false, false, false, false, gapBody, QByteArray(), QByteArray(), QByteArray(), QByteArray(), QByteArray(), 1016, gapTags), &err);
        QVERIFY2(gap, qPrintable(err));
        const QTextDocument *gapStory = placeholderStory(*gap, *gap->pages[0], QStringLiteral("ph:body:1"));
        QVERIFY(gapStory);
        QCOMPARE(gapStory->blockCount(), 3);
        const QTextBlock alpha = gapStory->firstBlock(), empty = alpha.next(), beta = empty.next();
        QVERIFY(alpha.textList() && beta.textList());
        QVERIFY(!empty.textList());
        QVERIFY(alpha.textList() == beta.textList());
        QCOMPARE(beta.textList()->itemNumber(beta), 1);
        // The numbering is kept when the presentation is saved as a .pptx or an .odp and opened again: "1.", then "5)", then the bullet.
        for (const bool odp : {false, true}) {
            auto again = odp ? viaOdp(*d, &err) : viaPptx(*d, &err);
            QVERIFY2(again, qPrintable(err));
            const QTextDocument *s = storyBeginning(*again, *again->pages[0], QStringLiteral("First point"));
            QVERIFY(s);
            QCOMPARE(s->blockCount(), 3);
            const QTextBlock a = s->firstBlock(), b = a.next(), c = b.next();
            QVERIFY(a.textList() && b.textList() && c.textList());
            QCOMPARE(a.textList()->format().style(), QTextListFormat::ListDecimal);
            QCOMPARE(a.textList()->format().numberSuffix(), QStringLiteral("."));
            QCOMPARE(b.textList()->format().style(), QTextListFormat::ListDecimal);
            QCOMPARE(b.textList()->format().numberSuffix(), QStringLiteral(")"));
            QCOMPARE(b.textList()->format().start(), 5);
            QCOMPARE(c.textList()->format().style(), QTextListFormat::ListDisc);
        }
    }

    // A text box keeps the numbering of its own text in its client data (the same extension, in the shape's tag): the paragraphs of the second run
    // are numbered "1.", "2." and the first run (a heading) has none and no bullet.
    void importBinaryNumbersTheParagraphsOfAShapesOwnText()
    {
        auto entry = [&](quint32 masks, const QByteArray &fields) { return le(masks, 4) + fields + le(0, 4) + le(0, 4); };
        const QByteArray atom = entry(0, QByteArray()) + entry(0x02000000, le(1, 2));
        const QByteArray tags = pptBox(5000, pptBox(5002, pptRec(4026, utf16(QStringLiteral("___PPT9"))) + pptRec(5003, pptRec(4012, atom))));
        // "References" (11 with its paragraph mark) with no bullet, then two paragraphs with bullets on (the flag 1) the master's text would show as bullets.
        const QByteArray text = pptRec(3999, le(4, 4)) + pptRec(4000, utf16(QStringLiteral("References\rFirst item\rSecond item")))
                                + pptRec(4001, pfRun(11, 0, 0) + pfRun(23, 0, 0x1, le(1, 2)) + cfRun(34, 0));
        QString err;
        auto build = [&](const QByteArray &clientExtra) {
            return deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), pptShape(0x470, 0, text, 1000, 200, 5000, 3800, QByteArray(), 0xA00, 1, clientExtra)), &err);
        };
        auto d = build(tags);
        QVERIFY2(d, qPrintable(err));
        const QTextDocument *story = storyBeginning(*d, *d->pages[0], QStringLiteral("References"));
        QVERIFY(story);
        QCOMPARE(story->blockCount(), 3);
        const QTextBlock heading = story->firstBlock(), first = heading.next(), second = first.next();
        QVERIFY(!heading.textList());
        QVERIFY(first.textList() && second.textList());
        QVERIFY(first.textList() == second.textList());
        QCOMPARE(first.textList()->format().style(), QTextListFormat::ListDecimal);
        QCOMPARE(first.textList()->format().numberSuffix(), QStringLiteral("."));
        QCOMPARE(first.textList()->itemNumber(first), 0);
        QCOMPARE(second.textList()->itemNumber(second), 1);
        // Without the extension the same paragraphs are bulleted.
        auto plain = build(QByteArray());
        QVERIFY2(plain, qPrintable(err));
        const QTextDocument *plainStory = storyBeginning(*plain, *plain->pages[0], QStringLiteral("References"));
        QVERIFY(plainStory && plainStory->firstBlock().next().textList());
        QCOMPARE(plainStory->firstBlock().next().textList()->format().style(), QTextListFormat::ListDisc);
    }

    // A shape that shows the slide's text (an outline reference) can keep a text ruler beside the reference: the margins it sets are those
    // of the text in that shape (here the first level's, 720 master units, which is 90 points), where another shape that shows the same
    // text with no ruler of its own has the master's.
    void importBinaryTakesTheRulerOfAShapeThatShowsTheSlidesText()
    {
        const QByteArray ruled = pptShape(0x430, 0, outlineRef(1) + pptRec(4006, le(0x8, 4) + le(720, 2)), 1000, 200, 5000, 3800);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), ruled), &err);
        QVERIFY2(d, qPrintable(err));
        QList<double> margins;
        for (const auto &it : d->pages[0]->items) {
            if (it->type() != ItemType::Text) continue;
            const QTextDocument *story = d->storyDoc(static_cast<const TextItem *>(it.get())->storyId);
            if (!story || story->toPlainText().isEmpty()) continue;
            if (it->role == QLatin1String("ph:body:1") || it->role.isEmpty()) margins << story->firstBlock().blockFormat().leftMargin();
        }
        QCOMPARE(int(margins.size()), 2);
        std::sort(margins.begin(), margins.end());
        QVERIFY2(qAbs(margins.last() - 90.0) < 0.01, qPrintable(QString::number(margins.last())));
        QVERIFY2(qAbs(margins.first() - 90.0) > 1.0, qPrintable(QString::number(margins.first())));
        // The two margins are kept in a .pptx and an .odp (the text of both shapes begins "First point").
        for (const bool odp : {false, true}) {
            auto again = odp ? viaOdp(*d, &err) : viaPptx(*d, &err);
            QVERIFY2(again, qPrintable(err));
            QList<double> kept;
            for (const auto &it : again->pages[0]->items) {
                if (it->type() != ItemType::Text) continue;
                const QTextDocument *story = again->storyDoc(static_cast<const TextItem *>(it.get())->storyId);
                if (story && story->toPlainText().startsWith(QLatin1String("First point"))) kept << story->firstBlock().blockFormat().leftMargin();
            }
            QCOMPARE(int(kept.size()), 2);
            std::sort(kept.begin(), kept.end());
            QVERIFY2(qAbs(kept.last() - 90.0) < 0.5 && qAbs(kept.first() - margins.first()) < 0.5, qPrintable(QStringLiteral("%1, %2").arg(kept.first()).arg(kept.last())));
        }
    }

    // A fill that is a picture and is set to be recolored as a picture, with the color to recolor it in (a master's background, in the clouds
    // of one deck), is drawn in two colors, as the presentation program draws it: each pixel as light as the picture's gray, from that color (for
    // black) to the back color (for white). A picture fill that isn't set so, or that has no such color (another deck's textured background
    // is set so and keeps its colors), is as it is.
    void importBinaryRecolorsAPictureFillAsTwoColors()
    {
        QImage photo(2, 1, QImage::Format_ARGB32);
        photo.setPixelColor(0, 0, QColor(255, 0, 0));
        photo.setPixelColor(1, 0, QColor(0, 0, 128));
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        photo.save(&buffer, "PNG");
        const QByteArray picture = pptRec(0xF01E, QByteArray(16, 'v') + QByteArray(1, '\xFF') + png, 0x6E0);
        const QByteArray entry = pptRec(0xF007, QByteArray(1, '\x06') + QByteArray(1, '\0') + QByteArray(16, 't') + le(0xFF, 2) + le(quint32(picture.size()), 4) + le(1, 4) + le(0xFFFFFFFF, 4) + le(0, 4) + picture, 6, 2);
        const QByteArray group = pptBox(1035, pptBox(0xF000, pptRec(0xF006, QByteArray(16, '\0')) + pptBox(0xF001, entry, 1)));
        std::unique_ptr<Document> kept;
        auto pixels = [&](quint32 styleFlags, bool recolorTo, bool keep = false) {
            QByteArray t;
            // A picture fill of the first picture, the fill color green, the color to recolor it in red (when it has one), and the back color blue
            // (a color is stored blue, green, red).
            QVector<QPair<int, quint32>> props{{0x0180, 3}, {0x4186, 1}, {0x0181, 0x00FF00}, {0x0183, 0xFF0000}, {0x01BF, styleFlags}};
            if (recolorTo) props.insert(1, {0x011A, 0x0000FF});
            for (const auto &p : props) t += le(quint16(p.first), 2) + le(p.second, 4);
            const QByteArray background = pptBox(0xF004, pptRec(0xF00A, le(0x1000, 4) + le(0xC00, 4), 1, 2) + pptRec(0xF00B, t, quint16(props.size()), 3));
            QString err;
            auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), group, QByteArray(), QByteArray(), background), &err);
            QList<QColor> out;
            if (!d) qWarning("%s", qPrintable(err));
            if (!d || d->masters.isEmpty() || d->masters.first()->background.type != Fill::Picture) return out;
            const QImage im = d->image(d->masters.first()->background.imageId);
            for (int x = 0; x < im.width(); ++x) out << im.pixelColor(x, 0);
            if (keep) kept = std::move(d);
            return out;
        };
        // Set to recolor: red's gray is 0.2126 of the way from the fill color to the back color, and dark blue's 0.0362.
        const QList<QColor> two = pixels(0x00500050, true, true);
        QCOMPARE(int(two.size()), 2);
        QVERIFY2(colorsNear(two[0], QColor(201, 0, 54), 2), qPrintable(two[0].name()));
        QVERIFY2(colorsNear(two[1], QColor(246, 0, 9), 2), qPrintable(two[1].name()));
        // The two colors stay when the presentation is saved as a .pptx or an .odp and opened again.
        QVERIFY(kept);
        for (const bool odp : {false, true}) {
            QString err;
            auto again = odp ? viaOdp(*kept, &err) : viaPptx(*kept, &err);
            QVERIFY2(again, qPrintable(err));
            QList<QColor> colors;
            for (const auto &m : again->masters)
                if (m->background.type == Fill::Picture && colors.isEmpty()) {
                    const QImage im = again->image(m->background.imageId);
                    for (int x = 0; x < im.width(); ++x) colors << im.pixelColor(x, 0);
                }
            QCOMPARE(int(colors.size()), 2);
            QVERIFY2(colorsNear(colors[0], two[0], 3) && colorsNear(colors[1], two[1], 3), qPrintable(colors[0].name() + QLatin1Char(' ') + colors[1].name()));
        }
        // Not set to, or set to with no color to recolor it in: as it is.
        for (const QList<QColor> &kept : {pixels(0x00100010, true), pixels(0x00500050, false)}) {
            QCOMPARE(int(kept.size()), 2);
            QCOMPARE(kept[0], QColor(255, 0, 0));
            QCOMPARE(kept[1], QColor(0, 0, 128));
        }
    }

    // The pentagon arrow and the chevron of an old presentation point at a quarter of their width unless the shape says (its first value, in
    // 21600ths of the width, is where the point starts: 16200 by default), where the preset's own measure is a share of the shorter side. A
    // box 350 by 187.5 points: the point is 87.5 points, 0.4667 of the shorter side; set to 10800 it is 175, 0.9333.
    void importBinaryChevronsPointAtAQuarterOfTheirWidth()
    {
        auto adjust = [&](quint32 id, int type, const QByteArray &props) {
            return pptShape(id, 0, QByteArray(), 1000, 200, 3000, 2500, props, 0xA00, quint16(type));
        };
        const QByteArray set = pptRec(0xF00B, le(0x0147, 2) + le(10800, 4), 1, 3);
        const QByteArray shapes = adjust(0x450, 55, QByteArray()) + adjust(0x451, 55, set) + adjust(0x452, 15, QByteArray());
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes), &err);
        QVERIFY2(d, qPrintable(err));
        QVector<const ShapeItem *> found;
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Shape) found.push_back(static_cast<const ShapeItem *>(it.get()));
        // (The shapes come in the order they were written.)
        QCOMPARE(int(found.size()), 3);
        QCOMPARE(found[0]->shape, QStringLiteral("chevron"));
        QCOMPARE(found[2]->shape, QStringLiteral("homePlate"));
        QCOMPARE(int(found[0]->adj.size()), 1);
        QVERIFY2(qAbs(found[0]->adj[0] - 0.4667) < 0.001, qPrintable(QString::number(found[0]->adj[0])));
        QVERIFY2(qAbs(found[1]->adj[0] - 0.9333) < 0.001, qPrintable(QString::number(found[1]->adj[0])));
        QVERIFY2(qAbs(found[2]->adj[0] - 0.4667) < 0.001, qPrintable(QString::number(found[2]->adj[0])));
    }

    // A slide's line doesn't break after a slash, as the presentation program breaks at spaces and hyphens (and a word longer than the line
    // anywhere): in a long run of letters cut by slashes the lines don't all end at one. (Without the rule they all would.)
    void layoutSlideTextDoesNotBreakAfterASlash()
    {
        QString text;
        const int sizes[] = {2, 3, 5, 7, 4, 6, 8, 3};
        for (int i = 0; i < 80; ++i) {
            for (int k = 0; k < sizes[i % 8]; ++k) text += QChar('a' + (i * 7 + k) % 26);
            text += QLatin1Char('/');
        }
        const QByteArray body = pptText(1, text, pfRun(int(text.size()) + 1, 0, 0) + cfRun(int(text.size()) + 1, 0));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, body), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(d->slideText);
        const TextItem *box = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Text && it->role == QLatin1String("ph:body:1")) box = static_cast<const TextItem *>(it.get());
        QVERIFY(box);
        LayoutCache cache;
        const auto fl = cache.textFrame(*d, *box, 1, RenderOptions());
        QVERIFY(fl.layout);
        const auto lines = fl.layout->lineInfo(0);
        QVERIFY2(lines.size() > 3, qPrintable(QString::number(lines.size())));
        int atSlash = 0;
        for (int i = 0; i + 1 < int(lines.size()); ++i) atSlash += lines[i].text.trimmed().endsWith(QLatin1Char('/'));
        QVERIFY2(atSlash < int(lines.size()) - 1, qPrintable(QStringLiteral("%1 of %2").arg(atSlash).arg(lines.size() - 1)));
    }

    // The word joiner that keeps a slide's line from breaking after a slash is no letter: text with spacing between its letters is no wider for it
    // (three letters with 6 points between them are 18 points wider than with none, not 24).
    void layoutSlideTextSlashTakesNoLetterSpacing()
    {
        const QString text = QStringLiteral("a/b");
        const QByteArray body = pptText(1, text, pfRun(int(text.size()) + 1, 0, 0) + cfRun(int(text.size()) + 1, 0));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, body), &err);
        QVERIFY2(d, qPrintable(err));
        const TextItem *box = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Text && it->role == QLatin1String("ph:body:1")) box = static_cast<const TextItem *>(it.get());
        QVERIFY(box);
        QTextDocument *story = d->storyDoc(box->storyId);
        QVERIFY(story);
        auto widthWith = [&](double spacing) {
            QTextCursor c(story);
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setFontLetterSpacingType(QFont::AbsoluteSpacing);
            f.setFontLetterSpacing(spacing);
            c.mergeCharFormat(f);
            LayoutCache cache;
            const auto fl = cache.textFrame(*d, *box, 1, RenderOptions());
            QRectF all;
            if (fl.layout)
                for (const QRectF &r : fl.layout->rangeRects(0, 0, 3)) all = all.united(r);
            return all.width();
        };
        const double plain = widthWith(0), spaced = widthWith(6);
        QVERIFY2(std::abs((spaced - plain) - 18) < 0.8, qPrintable(QStringLiteral("%1 wider").arg(spaced - plain)));
    }

    // Text anchored "centered" (the kinds of anchor 3 to 5) sets the block of text in the middle of its box across: the widest line is as far
    // from one side as from the other, and the shorter lines keep their alignment in it (left aligned, here, at the block's left). Text
    // anchored in the middle only (1) stays against the left.
    void importBinaryCenteredAnchorSetsTheTextBlockInTheMiddle()
    {
        auto anchored = [&](quint32 id, const QString &text, int anchor) {
            const QByteArray props = pptRec(0xF00B, le(0x0087, 2) + le(quint32(anchor), 4), 1, 3);
            return pptShape(id, 0, pptRec(3999, le(4, 4)) + pptRec(4000, utf16(text)), 1000, 200, 5000, 1500, props);
        };
        const QByteArray shapes = anchored(0x460, QStringLiteral("Centered"), 4) + anchored(0x461, QStringLiteral("Plain"), 1)
                                  + anchored(0x462, QStringLiteral("A longer first line of words\rShort"), 3);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes), &err);
        QVERIFY2(d, qPrintable(err));
        LayoutCache cache;
        QHash<QString, QVector<QRectF>> rects, inks;   // by the text's start, the rectangles of its lines, and of each line's letters (its paragraph mark left out)
        double width = 0, inset = 0;
        for (const auto &it : d->pages[0]->items) {
            if (it->type() != ItemType::Text || !it->role.isEmpty()) continue;
            const auto *t = static_cast<const TextItem *>(it.get());
            const QString text = d->storyDoc(t->storyId)->toPlainText();
            const auto fl = cache.textFrame(*d, *t, 1, RenderOptions());
            QVERIFY(fl.layout);
            rects.insert(text.left(5), fl.layout->rangeRects(0, 0, d->storyDoc(t->storyId)->characterCount() - 1));
            QVector<QRectF> letters;
            for (const auto &li : fl.layout->lineInfo(0)) {
                int end = li.docEnd;
                if (end > li.docPos && d->storyDoc(t->storyId)->characterAt(end - 1) == QChar::ParagraphSeparator) --end;
                QRectF ink;
                for (const QRectF &r : fl.layout->rangeRects(0, li.docPos, end)) ink = ink.united(r);
                letters << ink;
            }
            inks.insert(text.left(5), letters);
            width = t->rect.width();
            inset = t->insets.left();
        }
        QCOMPARE(int(rects.size()), 3);
        QVERIFY(width > 100);
        auto whole = [&](const QString &key) {
            QRectF all;
            for (const QRectF &r : rects.value(key)) all = all.united(r);
            return all;
        };
        QVERIFY2(std::abs(whole(QStringLiteral("Cente")).center().x() - width / 2) < 1.5, qPrintable(QString::number(whole(QStringLiteral("Cente")).center().x())));
        QVERIFY2(std::abs(whole(QStringLiteral("Plain")).left() - inset) < 1.5, qPrintable(QString::number(whole(QStringLiteral("Plain")).left())));
        const QVector<QRectF> block = rects.value(QStringLiteral("A lon"));
        QVERIFY(block.size() >= 2);
        // The widest line's letters (a line's box takes in its paragraph mark, so it is the letters that are set in the middle) are as far from
        // one side as from the other, to the layout's own measure of a position.
        const QVector<QRectF> letters = inks.value(QStringLiteral("A lon"));
        QVERIFY(letters.size() >= 2);
        QRectF widest = letters[0];
        for (const QRectF &r : letters) if (r.width() > widest.width()) widest = r;
        QVERIFY2(std::abs(widest.center().x() - width / 2) < 0.5, qPrintable(QString::number(widest.center().x())));
        QVERIFY2(std::abs(block.first().left() - block.last().left()) < 1.0, qPrintable(QString::number(block.first().left() - block.last().left())));
        QVERIFY(block.first().left() > inset + 10);
        // The setting goes into a .pptx and comes back (the two boxes anchored centered).
        const QByteArray pptx = deck::exportPresentation(*d, &err);
        QVERIFY2(!pptx.isEmpty(), qPrintable(err));
        auto again = deck::importPresentation(pptx, &err);
        QVERIFY2(again, qPrintable(err));
        int centered = 0;
        for (const auto &it : again->pages[0]->items)
            if (it->type() == ItemType::Text && static_cast<const TextItem *>(it.get())->centerBlock) ++centered;
        QCOMPARE(centered, 2);
    }

    // The block of text anchored centered is as wide as its widest line's words: the spaces that hang at a line's end are not in it. A line of
    // centered text with spaces after it, in a box anchored centered, has its words in the middle of the box.
    void importBinaryCenteredBlockLeavesOutTheSpacesAtTheEndOfLines()
    {
        const QByteArray props = pptRec(0xF00B, le(0x0087, 2) + le(4, 4), 1, 3);
        const QString text = QStringLiteral("Spaced      ");
        // Centered (the paragraph format's alignment 1, mask 0x800).
        const QByteArray styles = pfRun(int(text.size()) + 1, 0, 0x800, le(1, 2)) + cfRun(int(text.size()) + 1, 0);
        const QByteArray shapes = pptShape(0x490, 0, pptRec(3999, le(4, 4)) + pptRec(4000, utf16(text)) + pptRec(4001, styles), 1000, 200, 5000, 1500, props);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes), &err);
        QVERIFY2(d, qPrintable(err));
        const TextItem *box = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Text && it->role.isEmpty()) box = static_cast<const TextItem *>(it.get());
        QVERIFY(box);
        LayoutCache cache;
        const auto fl = cache.textFrame(*d, *box, 1, RenderOptions());
        QVERIFY(fl.layout);
        QRectF words;
        for (const QRectF &r : fl.layout->rangeRects(0, 0, 6)) words = words.united(r);
        QVERIFY2(std::abs(words.center().x() - box->rect.width() / 2) < 1.5, qPrintable(QString::number(words.center().x() - box->rect.width() / 2)));
    }

    // A slide's justified lines have their words stretched to the edge: the spaces that hang at the end of a line are no part of the width the
    // words are spread over, so the last word of most lines (all but the paragraph's last) ends at the right edge of the text area, not a space short of it.
    void layoutSlideJustifiedLinesReachTheRightEdge()
    {
        QString text;
        // (Words with ligatures too: a line is cut after a space, never inside a word.)
        for (int i = 0; i < 12; ++i) text += QStringLiteral("The quick brown fox jumps over the lazy dog, the staff officers, difficult affiliate traffic ");
        text += QStringLiteral("end");
        // Justified: the alignment is 3 (the paragraph format's mask 0x800).
        const QByteArray body = pptText(1, text, pfRun(int(text.size()) + 1, 0, 0x800, le(3, 2)) + cfRun(int(text.size()) + 1, 0));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, body), &err);
        QVERIFY2(d, qPrintable(err));
        const TextItem *box = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Text && it->role == QLatin1String("ph:body:1")) box = static_cast<const TextItem *>(it.get());
        QVERIFY(box);
        LayoutCache cache;
        const auto fl = cache.textFrame(*d, *box, 1, RenderOptions());
        QVERIFY(fl.layout);
        const auto lines = fl.layout->lineInfo(0);
        QVERIFY2(lines.size() > 2, qPrintable(QString::number(lines.size())));
        const double edge = box->rect.width() - box->insets.right();
        QVector<bool> reaches;   // by line: whether its last letter is at the edge
        for (int i = 0; i + 1 < int(lines.size()); ++i) {
            QVERIFY2(d->storyDoc(box->storyId)->characterAt(lines[i].docEnd - 1).isSpace(), qPrintable(QStringLiteral("line %1 is cut inside a word").arg(i)));
            // The last character that isn't a space.
            int last = lines[i].docEnd - 1;
            while (last > lines[i].docPos && d->storyDoc(box->storyId)->characterAt(last).isSpace()) --last;
            QRectF r;
            for (const QRectF &c : fl.layout->rangeRects(0, last, last + 1)) r = r.united(c);
            reaches << (std::abs(r.right() - edge) < 0.75);
        }
        // A line that the spaces added to its width would take another word into is laid out as it was, a space short of the edge: all the others
        // reach it. Which lines those are the same text set to the left says (each line's right end as it is, and the width of the next line's first
        // word: the word comes in when it ends within the edge); the lines too near to tell by half a point are left out.
        QTextCursor all(d->storyDoc(box->storyId));
        all.select(QTextCursor::Document);
        QTextBlockFormat toLeft;
        toLeft.setAlignment(Qt::AlignLeft);
        all.mergeBlockFormat(toLeft);
        LayoutCache plainCache;
        const auto natural = plainCache.textFrame(*d, *box, 1, RenderOptions());
        QVERIFY(natural.layout);
        const auto plain = natural.layout->lineInfo(0);
        QCOMPARE(int(plain.size()), int(lines.size()));
        const QTextDocument *story = d->storyDoc(box->storyId);
        int checked = 0, reached = 0;
        for (int i = 0; i + 1 < int(plain.size()); ++i) {
            int last = plain[i].docEnd - 1;
            while (last > plain[i].docPos && story->characterAt(last).isSpace()) --last;
            QRectF end;
            for (const QRectF &c : natural.layout->rangeRects(0, last, last + 1)) end = end.united(c);
            int wordEnd = plain[i + 1].docPos;
            while (wordEnd < plain[i + 1].docEnd && !story->characterAt(wordEnd).isSpace()) ++wordEnd;
            QRectF word;
            for (const QRectF &c : natural.layout->rangeRects(0, plain[i + 1].docPos, wordEnd)) word = word.united(c);
            const double slack = edge - (end.right() + word.width());   // what is left of the line if the next word came on without a space
            if (std::abs(slack) < 0.5) continue;
            ++checked;
            QVERIFY2(reaches[i] == (slack < 0), qPrintable(QStringLiteral("line %1: slack %2, reaches the edge: %3").arg(i).arg(slack).arg(reaches[i])));
            reached += reaches[i];
        }
        QVERIFY2(checked * 2 >= int(lines.size()) - 1 && reached > 0, qPrintable(QStringLiteral("%1 lines checked of %2, %3 reach the edge").arg(checked).arg(lines.size() - 1).arg(reached)));
    }

    // The arrowhead that points left (code 0xD7 in Wingdings), the mirror of the one that points right (0xD8, which a theme sets at the end of a
    // line), has its own character in Unicode (the black leftwards equilateral arrowhead); a run in the font shows that and not a box.
    void importBinaryWingdingsLeftArrowheadIsAnArrowhead()
    {
        const QByteArray raw8 = QByteArray("\xD7\xD8");
        const QByteArray styles = pfRun(int(raw8.size()) + 1, 0, 0) + cfRun(int(raw8.size()) + 1, 0x10000, le(4, 2));   // font 4: Wingdings
        const QByteArray text = pptRec(3999, le(1, 4)) + pptRec(4008, raw8) + pptRec(4001, styles);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, text), &err);
        QVERIFY2(d, qPrintable(err));
        const QTextDocument *body = placeholderStory(*d, *d->pages[0], QStringLiteral("ph:body:1"));
        QVERIFY(body);
        QCOMPARE(body->toPlainText(), QString(QChar(0x2B9C)) + QChar(0x27A2));
    }

    // An arc of an old presentation is the part of an ellipse from the angle of its first adjust value to its second's (turning clockwise
    // from the right; a quarter from the top when they are left out), and the ellipse is the one its first two vertices give in the shape's
    // units when it has them: the frame holds the arc only. (It was drawn as lines through its vertices.) A frame 200 by 100 points.
    void importBinaryArcsArePartsOfTheirEllipse()
    {
        auto fixed = [&](int degrees) { return quint32(qint32(degrees) * 65536); };
        auto simple = [&](quint16 id, quint32 value) { return le(id, 2) + le(value, 4); };
        auto arc = [&](quint32 id, const QByteArray &entries, int count, const QByteArray &data) {
            return pptShape(id, 0, QByteArray(), 1000, 200, 1800, 1800, pptRec(0xF00B, entries + data, quint16(count), 3), 0xA00, 19);
        };
        // The vertices: an array of two points of two 32-bit numbers, the corners of an ellipse twice as wide and tall as the frame, its center
        // the frame's bottom left corner.
        QByteArray vertices = le(2, 2) + le(2, 2) + le(8, 2);
        for (qint32 v : {-21600, 0, 21600, 43200}) vertices += le(quint32(v), 4);
        const QByteArray shapes = arc(0x470, QByteArray(), 0, QByteArray())
                                  + arc(0x471, simple(0x0148, fixed(90)), 1, QByteArray())
                                  + arc(0x472, simple(0x8145, quint32(vertices.size())), 1, vertices);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes), &err);
        QVERIFY2(d, qPrintable(err));
        QVector<const ShapeItem *> arcs;
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Shape) arcs.push_back(static_cast<const ShapeItem *>(it.get()));
        QCOMPARE(int(arcs.size()), 3);
        auto near = [&](const QPointF &a, const QPointF &b) { return std::abs(a.x() - b.x()) < 1.5 && std::abs(a.y() - b.y()) < 1.5; };
        auto text = [&](const QPointF &p) { return QStringLiteral("%1, %2").arg(p.x()).arg(p.y()); };
        // The quarter from the top to the right, in the frame.
        QVERIFY(!arcs[0]->customPath.isEmpty());
        QVERIFY2(near(arcs[0]->customPath.pointAtPercent(0), QPointF(100, 0)), qPrintable(text(arcs[0]->customPath.pointAtPercent(0))));
        QVERIFY2(near(arcs[0]->customPath.pointAtPercent(1), QPointF(200, 50)), qPrintable(text(arcs[0]->customPath.pointAtPercent(1))));
        // To 90 degrees: the half, down to the bottom.
        QVERIFY2(near(arcs[1]->customPath.pointAtPercent(1), QPointF(100, 100)), qPrintable(text(arcs[1]->customPath.pointAtPercent(1))));
        // Of the larger ellipse: from its top (the frame's top left corner) to its right (the frame's bottom right corner).
        QVERIFY2(near(arcs[2]->customPath.pointAtPercent(0), QPointF(0, 0)), qPrintable(text(arcs[2]->customPath.pointAtPercent(0))));
        QVERIFY2(near(arcs[2]->customPath.pointAtPercent(1), QPointF(200, 100)), qPrintable(text(arcs[2]->customPath.pointAtPercent(1))));
    }

    // An action button of an old presentation is a plain box, with its picture on it where it has one: the arrow that points back (shape
    // 194) is a group of the box and a triangle in a darker shade of the box's color, inside it; the blank button (189) is the box alone, not
    // the library's box with a frame in it.
    void importBinaryActionButtonsAreBoxesWithTheirPictures()
    {
        const QByteArray fill = pptRec(0xF00B, le(0x0181, 2) + le(0xC0C0C0, 4) + le(0x01BF, 2) + le(0x00110011, 4), 2, 3);
        auto button = [&](quint32 id, int type) { return pptShape(id, 0, QByteArray(), 1000, 200, 1000, 1500, fill, 0xA00, quint16(type)); };
        // A button 100 points wide and 62.5 high.
        const QByteArray shapes = button(0x480, 194) + button(0x481, 189);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes), &err);
        QVERIFY2(d, qPrintable(err));
        const GroupItem *back = nullptr;
        const ShapeItem *blank = nullptr;
        for (const auto &it : d->pages[0]->items) {
            if (it->type() == ItemType::Group) back = static_cast<const GroupItem *>(it.get());
            if (it->type() == ItemType::Shape) blank = static_cast<const ShapeItem *>(it.get());
        }
        QVERIFY(back && blank);
        QCOMPARE(int(back->children.size()), 2);
        QCOMPARE(back->children[0]->type(), ItemType::Shape);
        QCOMPARE(back->children[1]->type(), ItemType::Shape);
        const auto *box = static_cast<const ShapeItem *>(back->children[0].get());
        const auto *mark = static_cast<const ShapeItem *>(back->children[1].get());
        QCOMPARE(box->shape, QStringLiteral("rect"));
        QVERIFY(box->customPath.isEmpty());
        QVERIFY(!mark->customPath.isEmpty());
        // The triangle is inside the box and darker than it.
        const QRectF inside = mark->customPath.boundingRect();
        QVERIFY2(QRectF(QPointF(0, 0), mark->rect.size()).contains(inside), qPrintable(QStringLiteral("%1,%2 %3x%4").arg(inside.x()).arg(inside.y()).arg(inside.width()).arg(inside.height())));
        QCOMPARE(box->fill.color.rgbValue(), QColor(0xC0, 0xC0, 0xC0));
        QVERIFY(mark->fill.color.rgbValue().red() < 0xC0 && mark->fill.color.rgbValue().red() > 0x40);
        QCOMPARE(blank->shape, QStringLiteral("rect"));
        // The button (its box and the picture on it) and the blank one are kept in a .pptx and an .odp.
        for (const bool odp : {false, true}) {
            auto again = odp ? viaOdp(*d, &err) : viaPptx(*d, &err);
            QVERIFY2(again, qPrintable(err));
            const GroupItem *group = nullptr;
            const ShapeItem *plain = nullptr;
            for (const auto &it : again->pages[0]->items) {
                if (it->type() == ItemType::Group) group = static_cast<const GroupItem *>(it.get());
                if (it->type() == ItemType::Shape) plain = static_cast<const ShapeItem *>(it.get());
            }
            QVERIFY(group && plain);
            QCOMPARE(int(group->children.size()), 2);
            QVERIFY(group->children[0]->type() == ItemType::Shape && group->children[1]->type() == ItemType::Shape);
            const auto *keptBox = static_cast<const ShapeItem *>(group->children[0].get());
            const auto *keptMark = static_cast<const ShapeItem *>(group->children[1].get());
            QCOMPARE(keptBox->fill.color.rgbValue(), QColor(0xC0, 0xC0, 0xC0));
            QVERIFY(keptMark->fill.color.rgbValue().red() < 0xC0 && keptMark->fill.color.rgbValue().red() > 0x40);
            QVERIFY(qAbs(group->rect.width() - back->rect.width()) < 1.0 && qAbs(group->rect.height() - back->rect.height()) < 1.0);
        }
    }

    // The empty paragraph that ends a slide's text counts in the height the text is centered by, as the presentation program counts it, when
    // the text with it is taller than the box (text that runs over is centered both ways): in a box 60 points high, anchored in the middle,
    // the lines A and B and the empty one after them are 64.8 points (21.6 each) in a place of 52.8, and begin 6 points above where the
    // box's text begins; without the empty one they would be 43.2 and begin below it.
    void layoutSlideTextCountsTheEmptyParagraphThatEndsIt()
    {
        const QByteArray props = pptRec(0xF00B, le(0x0087, 2) + le(1, 4), 1, 3);
        const QByteArray shapes = pptShape(0x4A0, 0, pptRec(3999, le(4, 4)) + pptRec(4000, utf16(QStringLiteral("A\rB\r"))), 1000, 200, 3000, 1480, props);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes), &err);
        QVERIFY2(d, qPrintable(err));
        const TextItem *box = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Text && it->role.isEmpty()) box = static_cast<const TextItem *>(it.get());
        QVERIFY(box);
        QVERIFY2(std::abs(box->rect.height() - 60.0) < 0.01, qPrintable(QString::number(box->rect.height())));
        LayoutCache cache;
        const auto fl = cache.textFrame(*d, *box, 1, RenderOptions());
        QVERIFY(fl.layout);
        // (The empty paragraph is no line of the box, though it moves the text: it is past the end of it, and no overflow.)
        QVERIFY(!fl.layout->overflow());
        const auto lines = fl.layout->lineInfo(0);
        QCOMPARE(int(lines.size()), 2);
        // The first line is above the inset (-2.4 with 3.6 of it and the 6 points).
        QVERIFY2(lines[0].rect.top() < box->insets.top() - 4, qPrintable(QString::number(lines[0].rect.top())));
    }

    // A metafile's records that scale the window and the viewport multiply the extents each has by their ratios, as the device context does: a
    // window made half the size draws its contents twice the size, a viewport made half the size draws them half the size, and both together (the
    // program's own clip art does it) change nothing. The window here is 400 by 300 units and the box 200 by 150: a quarter of the picture.
    void renderMetafileScalesTheWindowAndTheViewportByTheirRatios()
    {
        const QByteArray half = wmfWords({2, 1, 2, 1}), twice = wmfWords({1, 2, 1, 2});   // (the parameters come last first)
        const QImage none = drawMetafileWith(QByteArray());
        QVERIFY(!none.isNull());
        QVERIFY(paintedGrayAt(none, 0.25, 0.25) && paintedGrayAt(none, 0.45, 0.45) && !paintedGrayAt(none, 0.75, 0.75));
        // The viewport alone, halved: the box is a quarter of the picture's width.
        const QImage viewport = drawMetafileWith(wmfRecord(0x0412, half));
        QVERIFY(paintedGrayAt(viewport, 0.125, 0.125) && !paintedGrayAt(viewport, 0.375, 0.375));
        // The window alone, halved: the box fills it. Doubled: the box is a quarter of its width.
        const QImage window = drawMetafileWith(wmfRecord(0x0410, half));
        QVERIFY(paintedGrayAt(window, 0.75, 0.75) && paintedGrayAt(window, 0.95, 0.95));
        const QImage doubled = drawMetafileWith(wmfRecord(0x0410, twice));
        QVERIFY(paintedGrayAt(doubled, 0.125, 0.125) && !paintedGrayAt(doubled, 0.375, 0.375));
        // Both halved: as if neither were.
        const QImage pair = drawMetafileWith(wmfRecord(0x0410, half) + wmfRecord(0x0412, half));
        QVERIFY(paintedGrayAt(pair, 0.25, 0.25) && paintedGrayAt(pair, 0.45, 0.45) && !paintedGrayAt(pair, 0.75, 0.75));
        QVERIFY2(qAbs(paintedGray(pair) - paintedGray(none)) * 50 < paintedGray(none), qPrintable(QStringLiteral("%1 against %2").arg(paintedGray(pair)).arg(paintedGray(none))));
        // The ratios of a hostile file are held to a bounded size (each is a 16-bit pair, repeated a hundred times: 30000 to the hundredth, a
        // denominator of zero, and a negative ratio), and the picture is still drawn in a moment.
        QByteArray hostile;
        for (int i = 0; i < 100; ++i)
            hostile += wmfRecord(0x0410, wmfWords({1, 30000, 1, 30000})) + wmfRecord(0x0412, wmfWords({1, 30000, 1, 30000})) + wmfRecord(0x0410, wmfWords({1, -1, 1, -1}));
        hostile += wmfRecord(0x0410, wmfWords({0, 1, 0, 1})) + wmfRecord(0x0412, wmfWords({0, 1, 0, 1}));
        QImage bad;
        const qint64 ms = bestNanoseconds([&] { bad = drawMetafileWith(hostile); }, 3) / 1000000;
        QVERIFY(!bad.isNull());
        QVERIFY2(ms < 10000, qPrintable(QString::number(ms)));
    }

    // A text box with a quarter turn that is flipped reads upright, turned the way its box is turned on the page: the box turns and then is flipped
    // (the item flips and then turns), so a box flipped one way and turned a quarter has its text turned the other quarter, and one flipped top to
    // bottom has it a half turn more. Flipped both ways it is turned a half turn.
    void importBinaryFlippedTextBoxKeepsItsQuarterTurn()
    {
        auto turn = [&](int degrees) { return pptRec(0xF00B, le(0x0004, 2) + le(quint32(degrees) << 16, 4), 1, 3); };
        auto box = [&](quint32 id, const QString &text, quint32 flags, const QByteArray &props) {
            return pptShape(id, 0, pptRec(3999, le(4, 4)) + pptRec(4000, utf16(text)), 1000, 200, 5000, 1500, props, flags);
        };
        // Flags: a text box (0xA00) with flip left to right (0x40) and/or top to bottom (0x80).
        const QByteArray shapes = box(0x440, QStringLiteral("Over"), 0xA80, turn(180)) + box(0x441, QStringLiteral("Side"), 0xA40, QByteArray()) + box(0x442, QStringLiteral("Up"), 0xA80, QByteArray())
                                  + box(0x443, QStringLiteral("SideQuarter"), 0xA40, turn(90)) + box(0x444, QStringLiteral("UpQuarter"), 0xA80, turn(90))
                                  + box(0x445, QStringLiteral("SideBack"), 0xA40, turn(-90)) + box(0x446, QStringLiteral("BothQuarter"), 0xAC0, turn(90));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes), &err);
        QVERIFY2(d, qPrintable(err));
        QHash<QString, const TextItem *> byText;
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Text) {
                const auto *t = static_cast<const TextItem *>(it.get());
                byText.insert(d->storyDoc(t->storyId)->toPlainText(), t);
            }
        const QVector<QPair<QString, double>> expected{{"Over", 0}, {"Side", 0}, {"Up", 180}, {"SideQuarter", 270}, {"UpQuarter", 90}, {"SideBack", 90}, {"BothQuarter", 270}};
        for (const auto &e : expected) {
            QVERIFY2(byText.contains(e.first), qPrintable(e.first));
            const TextItem *t = byText.value(e.first);
            QVERIFY2(!t->flipH && !t->flipV, qPrintable(e.first));
            QVERIFY2(qAbs(t->rotation - e.second) < 0.01, qPrintable(QStringLiteral("%1 is turned %2, not %3").arg(e.first).arg(t->rotation).arg(e.second)));
        }
    }

    // A fixed date that a presentation's document sets (its headers and footers record names the date as the user's own and holds its text), and each
    // of the places the document says to show that a slide lacks: a slide with a place of its own for the number keeps it, and still gets the date.
    void importBinaryShowsAFixedDateAndEachFooterThatASlideLacks()
    {
        // The flags: a date (1) that is the user's own (4), and the slide number (8).
        const QByteArray footers = footersBox(0, 0x1 | 0x4 | 0x8, QStringLiteral("May 19, 2003"));
        const QByteArray ownNumber = pptShape(0x420, 8, pptRec(3999, le(4, 4)) + pptRec(4000, utf16(QStringLiteral("7"))), 4000, 4000, 5500, 4300);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), ownNumber, QByteArray(), masterFooterPlaces(), QByteArray(), 1016, footers), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 2);
        const QString dt = QStringLiteral("dt"), number = QStringLiteral("sldNum");
        // The second slide has no place of its own: the date as the user typed it (text, not a field), and the number as a field.
        QCOMPARE(placeholdersOf(*d->pages[1], dt), 1);
        QCOMPARE(placeholderText(*d, *d->pages[1], QStringLiteral("ph:dt")), QStringLiteral("May 19, 2003"));
        QVERIFY(storyFieldCode(placeholderStory(*d, *d->pages[1], QStringLiteral("ph:dt"))).isEmpty());
        QCOMPARE(placeholdersOf(*d->pages[1], number), 1);
        // The first has its number (holding 7), kept as it is, and the date it lacked.
        QCOMPARE(placeholdersOf(*d->pages[0], number), 1);
        QCOMPARE(placeholderText(*d, *d->pages[0], QStringLiteral("ph:sldNum")), QStringLiteral("7"));
        QCOMPARE(placeholdersOf(*d->pages[0], dt), 1);
        QCOMPARE(placeholderText(*d, *d->pages[0], QStringLiteral("ph:dt")), QStringLiteral("May 19, 2003"));
    }

    // An automatic date shows in the format the record names (the format id of its headers and footers atom, an index into the program's list of
    // date and time formats): 0 is the short date, 1 the long one with the day's name, 3 the month's name with the day and year, 7 the date and
    // the time, 11 and 12 the time with its AM or PM.
    void importBinaryAutomaticDateTakesTheFormatTheRecordNames()
    {
        const QVector<QPair<int, QString>> formats{{0, "M/d/yyyy"}, {1, "dddd, MMMM d, yyyy"}, {2, "d MMMM yyyy"}, {3, "MMMM d, yyyy"}, {4, "d-MMM-yy"}, {5, "MMMM yy"},
                                                   {6, "MMM-yy"}, {7, "M/d/yyyy h:mm AP"}, {8, "M/d/yyyy h:mm:ss AP"}, {11, "h:mm AP"}, {12, "h:mm:ss AP"}};
        QString err;
        for (const auto &f : formats) {
            // The flags: a date (1) that is today's (2).
            auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), masterFooterPlaces(), QByteArray(), 1016, footersBox(f.first, 0x3)), &err);
            QVERIFY2(d, qPrintable(err));
            QCOMPARE(placeholdersOf(*d->pages[1], QStringLiteral("dt")), 1);
            QCOMPARE(storyFieldCode(placeholderStory(*d, *d->pages[1], QStringLiteral("ph:dt"))), QStringLiteral("datetime:") + f.second);
            if (f.first != 0) continue;
            // The short date, on the day the original shows it: 10/15/2026.
            FieldContext ctx;
            ctx.now = QDateTime(QDate(2026, 10, 15), QTime(19, 31, 36));
            QCOMPARE(ctx.resolve(storyFieldCode(placeholderStory(*d, *d->pages[1], QStringLiteral("ph:dt")))), QStringLiteral("10/15/2026"));
        }
    }

    // A footer whose text the document's record leaves out but the master's footer place holds is shown with that text on the slides that show their
    // footer; the text the record names wins over it, a slide whose own record shows no footer has none, and a document that says nothing of footers
    // shows none (the master holds the text, but no slide shows it).
    void importBinaryShowsTheFooterTextOfTheMastersFooterPlace()
    {
        const QString line = QStringLiteral("Seminar, Berlin, May 19, 2003"), ftr = QStringLiteral("ftr");
        const QByteArray places = masterFooterPlaces(line);
        QString err;
        // The flag of the footer (0x20) and no text.
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), places, QByteArray(), 1016, footersBox(0, 0x20)), &err);
        QVERIFY2(d, qPrintable(err));
        for (const auto &p : d->pages) {
            QCOMPARE(placeholdersOf(*p, ftr), 1);
            QCOMPARE(placeholderText(*d, *p, QStringLiteral("ph:ftr")), line);
        }
        // The first slide has a record of its own that shows only the number (8): no footer there, the master's line on the second.
        auto hidden = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), places, QByteArray(), 1016, footersBox(0, 0x20), footersBox(0, 0x08)), &err);
        QVERIFY2(hidden, qPrintable(err));
        QCOMPARE(placeholdersOf(*hidden->pages[0], ftr), 0);
        QCOMPARE(placeholderText(*hidden, *hidden->pages[1], QStringLiteral("ph:ftr")), line);
        // The record's own text wins.
        auto named = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), places, QByteArray(), 1016, footersBox(0, 0x20, QString(), QStringLiteral("Annual review"))), &err);
        QVERIFY2(named, qPrintable(err));
        QCOMPARE(placeholderText(*named, *named->pages[1], QStringLiteral("ph:ftr")), QStringLiteral("Annual review"));
        // No record, no footer.
        auto silent = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), places), &err);
        QVERIFY2(silent, qPrintable(err));
        for (const auto &p : silent->pages) QCOMPARE(placeholdersOf(*p, ftr), 0);
    }

    // A date typed in the master's date place (a whole line, not the program's date field) is what each slide shows that the record says shows a
    // date without saying which: not today's, and none of its own text. A record that names the text, or says it is today's, wins over it; a
    // slide whose own record hides the date has none, and a document with no record shows none.
    void importBinaryShowsTheDateTextOfTheMastersDatePlace()
    {
        const QString line = QStringLiteral("Seminar, Berlin, July 2nd 2008"), dt = QStringLiteral("dt"), ph = QStringLiteral("ph:dt");
        const QByteArray places = masterFooterPlaces(QString(), line);
        auto open = [&](const QByteArray &document, const QByteArray &own = QByteArray(), const QByteArray &masterPlaces = QByteArray()) {
            QString err;
            auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), masterPlaces.isEmpty() ? places : masterPlaces, QByteArray(), 1016,
                                                             document, own), &err);
            if (!d) qWarning("%s", qPrintable(err));
            return d;
        };
        // The flag of the date (1) and nothing more: the master's line, as text, on both slides.
        auto shown = open(footersBox(0, 0x1));
        QVERIFY(shown);
        QCOMPARE(int(shown->pages.size()), 2);
        for (const auto &p : shown->pages) {
            QCOMPARE(placeholdersOf(*p, dt), 1);
            QCOMPARE(placeholderText(*shown, *p, ph), line);
            QVERIFY(storyFieldCode(placeholderStory(*shown, *p, ph)).isEmpty());
        }
        // The user's own date (4) without its text says the same.
        auto own = open(footersBox(0, 0x1 | 0x4));
        QVERIFY(own);
        for (const auto &p : own->pages) QCOMPARE(placeholderText(*own, *p, ph), line);
        // New slides will show it too: the options kept for them say the date is the one typed in the slide's own master.
        QVERIFY(deck::footerOptions(*shown).date && deck::footerOptions(*shown).dateFromMaster);
        QVERIFY(deck::footerOptions(*shown).dateText.isEmpty());
        // The first slide has a record of its own that shows only the number (8): no date there, the line on the second.
        auto hidden = open(footersBox(0, 0x1), footersBox(0, 0x8));
        QVERIFY(hidden);
        QCOMPARE(placeholdersOf(*hidden->pages[0], dt), 0);
        QCOMPARE(placeholderText(*hidden, *hidden->pages[1], ph), line);
        // A document that shows no date (the number only) has none, though the master holds the line; one with no record says nothing at all.
        auto numbers = open(footersBox(0, 0x8));
        QVERIFY(numbers);
        for (const auto &p : numbers->pages) QCOMPARE(placeholdersOf(*p, dt), 0);
        QString err;
        auto silent = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), places), &err);
        QVERIFY2(silent, qPrintable(err));
        for (const auto &p : silent->pages) QCOMPARE(placeholdersOf(*p, dt), 0);
        // The text the record holds wins, and so does today's date (the flag 2, a field that shows the day).
        auto named = open(footersBox(0, 0x1 | 0x4, QStringLiteral("May 19, 2003")));
        QVERIFY(named);
        QCOMPARE(placeholderText(*named, *named->pages[1], ph), QStringLiteral("May 19, 2003"));
        auto today = open(footersBox(0, 0x3));
        QVERIFY(today);
        QVERIFY(storyFieldCode(placeholderStory(*today, *today->pages[1], ph)).startsWith(QLatin1String("datetime:")));
        QCOMPARE(placeholderText(*today, *today->pages[1], ph), QString(QChar::ObjectReplacementCharacter));
        // A master whose date place holds nothing typed (here no text; a place holding only the date's field is the same) has nothing to show.
        auto empty = open(footersBox(0, 0x1), QByteArray(), masterFooterPlaces());
        QVERIFY(empty);
        for (const auto &p : empty->pages) QCOMPARE(placeholdersOf(*p, dt), 0);
        auto fieldOnly = open(footersBox(0, 0x1), QByteArray(), masterFooterPlaces(QString(), QStringLiteral("*"), QString(), -1, 0));
        QVERIFY(fieldOnly);
        for (const auto &p : fieldOnly->pages) QCOMPARE(placeholdersOf(*p, dt), 0);
        // Typed text with the date's field in it: the text and the field, in the place.
        auto mixed = open(footersBox(0, 0x1), QByteArray(), masterFooterPlaces(QString(), QStringLiteral("Seminar, *"), QString(), -1, 9));
        QVERIFY(mixed);
        QCOMPARE(placeholderText(*mixed, *mixed->pages[1], ph), QStringLiteral("Seminar, ") + QChar(QChar::ObjectReplacementCharacter));
        QCOMPARE(storyFieldCode(placeholderStory(*mixed, *mixed->pages[1], ph)), QStringLiteral("date"));
    }

    // The slide number's place of the master holds the number's field and perhaps text around it ("Page 4 of 12"): a slide that shows its number
    // without a place of its own has that text with its own number in it. A place that holds only the field is the number alone.
    void importBinaryKeepsTheTextAroundTheFieldOfTheMastersNumberPlace()
    {
        const QString number = QStringLiteral("sldNum"), ph = QStringLiteral("ph:sldNum"), field{QChar(QChar::ObjectReplacementCharacter)};
        auto open = [&](const QByteArray &masterPlaces) {
            QString err;
            auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), masterPlaces, QByteArray(), 1016, footersBox(0, 0x8)), &err);
            if (!d) qWarning("%s", qPrintable(err));
            return d;
        };
        auto around = open(masterFooterPlaces(QString(), QString(), QStringLiteral("Page * of 12"), 5));
        QVERIFY(around);
        for (const auto &p : around->pages) {
            QCOMPARE(placeholdersOf(*p, number), 1);
            QCOMPARE(placeholderText(*around, *p, ph), QStringLiteral("Page ") + field + QStringLiteral(" of 12"));
            QCOMPARE(storyFieldCode(placeholderStory(*around, *p, ph)), QStringLiteral("page"));
        }
        // Text with no field in it is what the slide shows.
        auto typed = open(masterFooterPlaces(QString(), QString(), QStringLiteral("Draft")));
        QVERIFY(typed);
        QCOMPARE(placeholderText(*typed, *typed->pages[1], ph), QStringLiteral("Draft"));
        // The field alone is the number alone.
        auto alone = open(masterFooterPlaces(QString(), QString(), QStringLiteral("*"), 0));
        QVERIFY(alone);
        QCOMPARE(placeholderText(*alone, *alone->pages[1], ph), field);
        QCOMPARE(storyFieldCode(placeholderStory(*alone, *alone->pages[1], ph)), QStringLiteral("page"));
    }

    // The text a master's number place holds around its field belongs to that master: slides on the main master and on the title master each show
    // their own, and a slide added on either afterward shows the one its own layout has (not the first slide's master's).
    void importBinaryNumberTextBelongsToEachMaster()
    {
        const QString number = QStringLiteral("sldNum"), ph = QStringLiteral("ph:sldNum"), field{QChar(QChar::ObjectReplacementCharacter)};
        const QByteArray mainPlaces = masterFooterPlaces(QString(), QString(), QStringLiteral("Page * of 12"), 5);
        const QByteArray titlePlaces = masterFooterPlaces(QString(), QString(), QStringLiteral("Slide *"), 6);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(true, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), mainPlaces, QByteArray(), 1016, footersBox(0, 0x8), QByteArray(), titlePlaces), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(d->pages.size()), 3);
        const QString onMain = QStringLiteral("Page ") + field + QStringLiteral(" of 12"), onTitle = QStringLiteral("Slide ") + field;
        QCOMPARE(placeholderText(*d, *d->pages[0], ph), onMain);
        QCOMPARE(placeholderText(*d, *d->pages[2], ph), onTitle);
        // Slides added now, one on each master's layout.
        const MasterPage *mainLayout = d->masterFor(*d->pages[0]), *titleLayout = d->masterFor(*d->pages[2]);
        QVERIFY(mainLayout && titleLayout && mainLayout != titleLayout);
        const int onTitleLayout = deck::addSlide(*d, *titleLayout), onMainLayout = deck::addSlide(*d, *mainLayout);
        QCOMPARE(placeholderText(*d, *d->pages[onTitleLayout], ph), onTitle);
        QCOMPARE(placeholderText(*d, *d->pages[onMainLayout], ph), onMain);
        // And the footers applied again (the dialog's Apply to All) keep each as it is.
        deck::applyFooters(*d, deck::footerOptions(*d));
        QCOMPARE(placeholderText(*d, *d->pages[0], ph), onMain);
        QCOMPARE(placeholderText(*d, *d->pages[2], ph), onTitle);
        Q_UNUSED(number);
    }

    // A slide can set its own footers in a record of its own: its date (the user's, here), its footer text, and no number, where the next slide
    // follows the document's.
    void importBinaryEachSlideCanSetItsOwnFooters()
    {
        const QByteArray document = footersBox(0, 0x8 | 0x20, QString(), QStringLiteral("Quarterly review"));
        const QByteArray own = footersBox(0, 0x1 | 0x4 | 0x20, QStringLiteral("Slide one date"), QStringLiteral("Only here"));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), masterFooterPlaces(), QByteArray(), 1016, document, own), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(placeholderText(*d, *d->pages[0], QStringLiteral("ph:dt")), QStringLiteral("Slide one date"));
        QCOMPARE(placeholderText(*d, *d->pages[0], QStringLiteral("ph:ftr")), QStringLiteral("Only here"));
        QCOMPARE(placeholdersOf(*d->pages[0], QStringLiteral("sldNum")), 0);
        QCOMPARE(placeholderText(*d, *d->pages[1], QStringLiteral("ph:ftr")), QStringLiteral("Quarterly review"));
        QCOMPARE(placeholdersOf(*d->pages[1], QStringLiteral("sldNum")), 1);
        QCOMPARE(placeholdersOf(*d->pages[1], QStringLiteral("dt")), 0);
    }

    // The footer text and the user's date of a record, and the footer text the master's footer place holds, are cut at a thousand characters
    // (kMaxFooterChars), so a file can't make every slide carry text of any length in them.
    void importBinaryFooterTextIsCutAtTheLimit()
    {
        const QByteArray footers = footersBox(0, 0x1 | 0x4 | 0x20, QString(2500, QLatin1Char('d')), QString(3000, QLatin1Char('f')));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), masterFooterPlaces(), QByteArray(), 1016, footers), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(placeholderText(*d, *d->pages[1], QStringLiteral("ph:ftr")), QString(1000, QLatin1Char('f')));
        QCOMPARE(placeholderText(*d, *d->pages[1], QStringLiteral("ph:dt")), QString(1000, QLatin1Char('d')));
        // The master's footer place holds 3000 letters, and the record shows the footer without a text of its own.
        auto fromMaster = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), masterFooterPlaces(QString(3000, QLatin1Char('m'))), QByteArray(), 1016, footersBox(0, 0x20)), &err);
        QVERIFY2(fromMaster, qPrintable(err));
        QCOMPARE(placeholderText(*fromMaster, *fromMaster->pages[1], QStringLiteral("ph:ftr")), QString(1000, QLatin1Char('m')));
    }

    // A text extension with a great many entries (the limit is kMaxPp9Entries, which the runs of a text can't reach, so this holds the file to its
    // memory and not what it shows) is read in a moment and numbers the first paragraphs by their entries.
    void importBinaryTextExtensionWithManyEntriesIsRead()
    {
        auto entry = [&](quint32 masks, const QByteArray &fields) { return le(masks, 4) + fields + le(0, 4) + le(0, 4); };
        // The first numbered (scheme 3, "1.") from 7; sixty thousand more with nothing set.
        QByteArray atom = entry(0x03000000, le(1, 2) + le(3, 2) + le(7, 2));
        for (int i = 0; i < 60000; ++i) atom += entry(0, QByteArray());
        const QByteArray outline = pptBox(4014, pptRec(4015, le(0x100, 4) + le(1, 4), 1) + pptRec(4012, atom));
        const QByteArray tags = pptBox(5000, pptBox(5002, pptRec(4026, utf16(QStringLiteral("___PPT9"))) + pptRec(5003, outline)));
        const QByteArray body = pptText(1, QStringLiteral("First point\rSecond point\rThird point"), pfRun(12, 0, 0) + pfRun(13, 1, 0) + pfRun(12, 1, 0) + cfRun(37, 0));
        QString err;
        std::unique_ptr<Document> d;
        const qint64 ms = bestNanoseconds([&] {
            d = deck::importBinaryPresentation(buildPpt(false, false, false, false, body, QByteArray(), QByteArray(), QByteArray(), QByteArray(), QByteArray(), 1016, tags), &err);
        }, 3, [&] { d.reset(); }) / 1000000;
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(ms < 20000, qPrintable(QString::number(ms)));
        const QTextDocument *story = placeholderStory(*d, *d->pages[0], QStringLiteral("ph:body:1"));
        QVERIFY(story && story->firstBlock().textList());
        QCOMPARE(story->firstBlock().textList()->format().style(), QTextListFormat::ListDecimal);
        QCOMPARE(story->firstBlock().textList()->format().start(), 7);
        QCOMPARE(story->firstBlock().next().textList()->format().style(), QTextListFormat::ListDisc);
    }

    // The centered text block of a text box or a shape is kept in an .odp (as the area of the text centered across the shape) and is read back; a
    // box that isn't centered stays as it was.
    void importBinaryCenteredTextBlockSurvivesAnOdpRoundTrip()
    {
        auto anchored = [&](quint32 id, const QString &text, int anchor, bool filled) {
            QByteArray props = le(0x0087, 2) + le(quint32(anchor), 4);
            if (filled) props += le(0x0181, 2) + le(0xC0C0C0, 4) + le(0x01BF, 2) + le(0x00110011, 4);
            return pptShape(id, 0, pptRec(3999, le(4, 4)) + pptRec(4000, utf16(text)), 1000, 200, 5000, 1500, pptRec(0xF00B, props, filled ? 3 : 1, 3));
        };
        const QByteArray shapes = anchored(0x460, QStringLiteral("Centered"), 4, false) + anchored(0x461, QStringLiteral("Plain"), 1, false) + anchored(0x462, QStringLiteral("Filled"), 3, true)
                                  + anchored(0x463, QStringLiteral("Filled plain"), 1, true);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes), &err);
        QVERIFY2(d, qPrintable(err));
        // Which of the slide's boxes and shapes are centered, by the text they hold.
        auto centered = [](const Document &doc) {
            QStringList out;
            for (const auto &it : doc.pages[0]->items) {
                const QString text = it->type() == ItemType::Text ? doc.storyDoc(static_cast<const TextItem &>(*it).storyId)->toPlainText()
                                     : it->type() == ItemType::Shape ? doc.storyDoc(static_cast<const ShapeItem &>(*it).storyId)->toPlainText() : QString();
                const bool on = it->type() == ItemType::Text ? static_cast<const TextItem &>(*it).centerBlock : it->type() == ItemType::Shape && static_cast<const ShapeItem &>(*it).centerBlock;
                if (on && (text == QLatin1String("Centered") || text == QLatin1String("Filled"))) out << text;
                else if (on && !text.isEmpty()) out << QStringLiteral("wrongly ") + text;
            }
            out.sort();
            return out;
        };
        QCOMPARE(centered(*d), (QStringList{"Centered", "Filled"}));
        auto again = viaOdp(*d, &err);
        QVERIFY2(again, qPrintable(err));
        QCOMPARE(centered(*again), (QStringList{"Centered", "Filled"}));
    }

    // A fixed date is kept as the text it is in an .odp (the date's declaration is a fixed one with its text), where an automatic one stays a field
    // that shows the day it is opened on.
    void importBinaryFixedDateSurvivesAnOdpRoundTrip()
    {
        QString err;
        const QString dt = QStringLiteral("ph:dt");
        auto fixed = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), masterFooterPlaces(), QByteArray(), 1016, footersBox(0, 0x1 | 0x4, QStringLiteral("May 19, 2003"))), &err);
        QVERIFY2(fixed, qPrintable(err));
        QCOMPARE(placeholderText(*fixed, *fixed->pages[1], dt), QStringLiteral("May 19, 2003"));
        auto again = viaOdp(*fixed, &err);
        QVERIFY2(again, qPrintable(err));
        QCOMPARE(int(again->pages.size()), 2);
        for (const auto &p : again->pages) {
            QCOMPARE(placeholdersOf(*p, QStringLiteral("dt")), 1);
            QCOMPARE(placeholderText(*again, *p, dt), QStringLiteral("May 19, 2003"));
            QVERIFY(storyFieldCode(placeholderStory(*again, *p, dt)).isEmpty());
        }
        auto automatic = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), masterFooterPlaces(), QByteArray(), 1016, footersBox(0, 0x3)), &err);
        QVERIFY2(automatic, qPrintable(err));
        auto autoAgain = viaOdp(*automatic, &err);
        QVERIFY2(autoAgain, qPrintable(err));
        QVERIFY(storyFieldCode(placeholderStory(*autoAgain, *autoAgain->pages[1], dt)).startsWith(QLatin1String("date")));
        QCOMPARE(placeholderText(*autoAgain, *autoAgain->pages[1], dt), QString(QChar::ObjectReplacementCharacter));
    }

    // The date that JP_NOW names (in ISO 8601, with its offset) is the time the tool draws automatic dates and times at; without it they show today. The
    // app itself never reads it. A presentation whose slides show the date in its short form: JP_NOW of 15 October 2026 draws 10/15/2026, one of
    // 2 March 2027 draws 3/2/2027, and two renders at the same moment are the same picture.
    void toolRenderDrawsTheDateJPNowNames()
    {
        QString tool = QCoreApplication::applicationDirPath() + QStringLiteral("/jdecktool");
#ifdef Q_OS_WIN
        tool += QStringLiteral(".exe");
#endif
        if (!QFileInfo::exists(tool)) QSKIP("jdecktool is not built beside the tests");
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString file = dir.filePath(QStringLiteral("dated.ppt"));
        {
            QFile f(file);
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(buildPpt(false, false, false, false, QByteArray(), QByteArray(), QByteArray(), QByteArray(), masterFooterPlaces(), QByteArray(), 1016, footersBox(0, 0x3)));
        }
        auto run = [&](const QStringList &args, const QString &now) {
            QProcess p;
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
            env.insert(QStringLiteral("JP_NO_TELEMETRY"), QStringLiteral("1"));
            env.remove(QStringLiteral("JP_NOW"));
            if (!now.isEmpty()) env.insert(QStringLiteral("JP_NOW"), now);
            p.setProcessEnvironment(env);
            p.start(tool, args);
            if (!p.waitForFinished(120000)) {
                p.kill();
                return QStringLiteral("<timed out>");
            }
            return QString::fromUtf8(p.readAllStandardOutput());
        };
        const QString first = QStringLiteral("2026-10-15T19:31:36-04:00"), second = QStringLiteral("2027-03-02T08:00:00-05:00");
        const QString linesFirst = run({QStringLiteral("lines"), file, QStringLiteral("1")}, first), linesSecond = run({QStringLiteral("lines"), file, QStringLiteral("1")}, second);
        QVERIFY2(linesFirst.contains(QLatin1String("10/15/2026")), qPrintable(linesFirst));
        QVERIFY2(linesSecond.contains(QLatin1String("3/2/2027")), qPrintable(linesSecond));
        auto render = [&](const QString &now, const QString &name) {
            const QString out = dir.filePath(name);
            run({QStringLiteral("render"), file, out, QStringLiteral("640")}, now);
            return QImage(QDir(out).filePath(QStringLiteral("slide-001.png")));
        };
        const QImage a = render(first, QStringLiteral("a")), b = render(first, QStringLiteral("b")), c = render(second, QStringLiteral("c"));
        QVERIFY(!a.isNull() && !b.isNull() && !c.isNull());
        QVERIFY(a == b);
        QVERIFY(a != c);
    }

    // A slide's line doesn't break after a slash when the text goes on in another format: a slash that ends a run in regular type, followed by a
    // word in bold, is no more a place to break than one inside a run (every slash of this text ends a run, so without the rule every line would end
    // at one).
    void layoutSlideTextDoesNotBreakAfterASlashAtAFormatChange()
    {
        QString text;
        QByteArray characters;
        const int sizes[] = {2, 3, 5, 7, 4, 6, 8, 3};
        for (int i = 0; i < 80; ++i) {
            QString unit;
            for (int k = 0; k < sizes[i % 8]; ++k) unit += QChar('a' + (i * 7 + k) % 26);
            unit += QLatin1Char('/');
            text += unit;
            // Every other unit in bold (the first mask bit, with the flag 1).
            characters += cfRun(int(unit.size()) + (i == 79 ? 1 : 0), 1, le(i & 1, 2));
        }
        const QByteArray body = pptText(1, text, pfRun(int(text.size()) + 1, 0, 0) + characters);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, body), &err);
        QVERIFY2(d, qPrintable(err));
        const TextItem *box = nullptr;
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Text && it->role == QLatin1String("ph:body:1")) box = static_cast<const TextItem *>(it.get());
        QVERIFY(box);
        LayoutCache cache;
        const auto fl = cache.textFrame(*d, *box, 1, RenderOptions());
        QVERIFY(fl.layout);
        const auto lines = fl.layout->lineInfo(0);
        QVERIFY2(lines.size() > 3, qPrintable(QString::number(lines.size())));
        int atSlash = 0;
        for (int i = 0; i + 1 < int(lines.size()); ++i) atSlash += lines[i].text.trimmed().endsWith(QLatin1Char('/'));
        QVERIFY2(atSlash < int(lines.size()) - 1, qPrintable(QStringLiteral("%1 of %2").arg(atSlash).arg(lines.size() - 1)));
    }

    // Numbering goes on past the cut a long list is made in (a list is cut into lists of 400 for its cost): the 401st numbered paragraph is 401,
    // not 1, as in a presentation the other format holds.
    void importBinaryNumberingGoesOnPastTheCutOfALongList()
    {
        auto entry = [&](quint32 masks, const QByteArray &fields) { return le(masks, 4) + fields + le(0, 4) + le(0, 4); };
        const QByteArray outline = pptBox(4014, pptRec(4015, le(0x100, 4) + le(1, 4), 1) + pptRec(4012, entry(0x02000000, le(1, 2))));
        const QByteArray tags = pptBox(5000, pptBox(5002, pptRec(4026, utf16(QStringLiteral("___PPT9"))) + pptRec(5003, outline)));
        QString text;
        for (int i = 0; i < 450; ++i) text += QLatin1String(i < 449 ? "a\r" : "a");
        // One run of paragraphs, so one entry numbers them all.
        const QByteArray body = pptText(1, text, pfRun(int(text.size()) + 1, 0, 0) + cfRun(int(text.size()) + 1, 0));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, body, QByteArray(), QByteArray(), QByteArray(), QByteArray(), QByteArray(), 1016, tags), &err);
        QVERIFY2(d, qPrintable(err));
        const QTextDocument *story = placeholderStory(*d, *d->pages[0], QStringLiteral("ph:body:1"));
        QVERIFY(story);
        QCOMPARE(story->blockCount(), 450);
        QTextBlock b = story->firstBlock();
        for (int i = 0; i < 450; ++i, b = b.next()) {
            QVERIFY2(b.textList(), qPrintable(QString::number(i)));
            QCOMPARE(b.textList()->format().style(), QTextListFormat::ListDecimal);
            const int shown = b.textList()->format().start() + b.textList()->itemNumber(b);
            if (i == 0 || i == 398 || i == 399 || i == 400 || i == 401 || i == 449) QVERIFY2(shown == i + 1, qPrintable(QStringLiteral("paragraph %1 is numbered %2").arg(i + 1).arg(shown)));
        }
    }

    // A slide's text box that says its text isn't centered across the box (anchored in the middle only) keeps that when its master's is centered: a
    // title the master centers across its box is centered on the slide, and a second title shape on the slide that sets its anchor itself is not.
    void importBinaryASlideCanTurnOffTheCenteredBlockItsMasterSets()
    {
        // The master's title anchored centered (4); the slide's own shape anchored in the middle only (1).
        const QByteArray masterTitle = pptRec(0xF00B, le(0x0087, 2) + le(4, 4), 1, 3);
        const QByteArray own = pptShape(0x430, 0xD, outlineRef(0), 1000, 200, 5000, 1500, pptRec(0xF00B, le(0x0087, 2) + le(1, 4), 1, 3));
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), own, masterTitle), &err);
        QVERIFY2(d, qPrintable(err));
        int centered = 0, plain = 0;
        for (const auto &it : d->pages[0]->items)
            if (it->type() == ItemType::Text && it->role == QLatin1String("ph:title")) (static_cast<const TextItem *>(it.get())->centerBlock ? centered : plain)++;
        QCOMPARE(centered, 1);
        QCOMPARE(plain, 1);
    }

    // Blank paragraphs that end a slide's text count in the place the text is centered at, but not as text that runs over: the text of two lines in
    // a box that holds them, with the empty paragraph after them that it doesn't, is not too tall for it (nothing is shrunk or marked for it), and
    // the same text in a .pptx.
    void layoutSlideTextEmptyParagraphAtTheEndIsNoOverflow()
    {
        const QByteArray props = pptRec(0xF00B, le(0x0087, 2) + le(1, 4), 1, 3);
        const QByteArray shapes = pptShape(0x4A0, 0, pptRec(3999, le(4, 4)) + pptRec(4000, utf16(QStringLiteral("A\rB\r"))), 1000, 200, 3000, 1480, props);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), shapes), &err);
        QVERIFY2(d, qPrintable(err));
        // Whether the text of the box fits it, as the renderer's fitting and the .odp export decide (a flag of overflow, and the lines' height).
        auto fits = [](const Document &doc, const Page &page) {
            for (const auto &it : page.items) {
                if (it->type() != ItemType::Text) continue;
                const auto &t = static_cast<const TextItem &>(*it);
                const QTextDocument *story = doc.storyDoc(t.storyId);
                if (!story || !story->toPlainText().startsWith(QLatin1String("A"))) continue;
                LayoutCache cache;
                const auto fl = cache.textFrame(doc, t, 1, RenderOptions());
                if (!fl.layout) return false;
                if (fl.layout->overflow()) return false;
                const auto lines = fl.layout->lineInfo(0);
                if (lines.size() != 2) return false;
                double top = lines.first().rect.top(), bottom = lines.first().rect.bottom() - lines.first().below;
                for (const auto &li : lines) {
                    top = std::min(top, li.rect.top());
                    bottom = std::max(bottom, li.rect.bottom() - li.below);
                }
                return bottom - top <= t.rect.height() - t.insets.top() - t.insets.bottom() + 0.5;
            }
            return false;
        };
        QVERIFY(fits(*d, *d->pages[0]));
        auto again = viaPptx(*d, &err);
        QVERIFY2(again, qPrintable(err));
        QVERIFY(fits(*again, *again->pages[0]));
    }

    // A slide's own date place that holds the generic date field (the one the slide's date place has in a file, which takes the format of the headers
    // and footers record rather than saying one) shows the date in the format the slide's record names, or the document's: the first slide's own
    // record names format 1 (the long date with the day's name), the second slide follows the document's, format 0.
    void importBinaryGenericDateFieldTakesTheFormatOfTheRecord()
    {
        // The first slide's own date place (7): its text is the field's place ("*") and the generic date field's atom (position 0).
        const QByteArray ownDate = pptShape(0x421, 7, pptRec(3999, le(4, 4)) + pptRec(4000, utf16(QStringLiteral("*"))) + pptRec(4088, le(0, 4)), 4000, 200, 2000, 4300);
        QString err;
        auto d = deck::importBinaryPresentation(buildPpt(false, false, false, false, QByteArray(), QByteArray(), ownDate, QByteArray(), masterFooterPlaces(), QByteArray(), 1016, footersBox(0, 0x3), footersBox(1, 0x3)), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(placeholdersOf(*d->pages[0], QStringLiteral("dt")), 1);
        QCOMPARE(storyFieldCode(placeholderStory(*d, *d->pages[0], QStringLiteral("ph:dt"))), QStringLiteral("datetime:dddd, MMMM d, yyyy"));
        QCOMPARE(storyFieldCode(placeholderStory(*d, *d->pages[1], QStringLiteral("ph:dt"))), QStringLiteral("datetime:M/d/yyyy"));
    }

    // A slide's picture with a brightness and a contrast (a:lum) is drawn as the presentation program draws it: the contrast times 1 / (1 - it)
    // the distance from the middle and the brightness moved by the mean of one and that (measured on reference pictures: a photo of a
    // collar at 21% and 52% came darker than the program's by more than 40 gray levels with Publisher's), and the picture's bytes, and what
    // the file says of it, are written back as they were. A publication's picture keeps Publisher's own.
    void importPicturesAreDrawnWithTheirBrightnessAndContrast()
    {
        QString err;
        auto d = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\"><a:lum bright=\"21000\" contrast=\"52000\"/></a:blip>"), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY(d->slidePictures);
        QVERIFY2(colorsNear(colorOfSamplePicture(*d), QColor(255, 82, 0)), qPrintable(colorOfSamplePicture(*d).name()));
        // Brightness alone adds its share of the range to every color.
        auto bright = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\"><a:lum bright=\"14000\"/></a:blip>"), &err);
        QVERIFY2(bright, qPrintable(err));
        QVERIFY2(colorsNear(colorOfSamplePicture(*bright), QColor(218, 102, 51)), qPrintable(colorOfSamplePicture(*bright).name()));
        // The picture is as the file has it, and is written as it was, with what is said of it.
        QCOMPARE(d->images.size(), 1);
        QCOMPARE(d->images.begin()->bytes, tinyPng());
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<a:lum bright=\"21000\" contrast=\"52000\"/>")), qPrintable(part(e, QStringLiteral("/ppt/slides/slide1.xml"))));
        int media = 0;
        for (auto it = e.zip.cbegin(); it != e.zip.cend(); ++it)
            if (it.key().startsWith(QStringLiteral("ppt/media/"))) {
                QCOMPARE(it.value(), tinyPng());
                ++media;
            }
        QCOMPARE(media, 1);
        // The same picture in a publication (a document that isn't a presentation) is drawn as Publisher draws it.
        const auto firstPicture = [](Document &doc) {
            PictureItem *found = nullptr;
            for (const auto &it : doc.pages[0]->items)
                if (it->type() == ItemType::Picture && !found) found = static_cast<PictureItem *>(it.get());
            return found;
        };
        PictureItem *pic = firstPicture(*d);
        QVERIFY(pic);
        QVERIFY(qAbs(pic->brightness - 21) < 1e-6 && qAbs(pic->contrast - 52) < 1e-6);
        d->slidePictures = false;
        const QImage publisher = Renderer::processedImage(*d, *pic, QSizeF());
        QCOMPARE(publisher.pixelColor(0, 0), QColor(255, 38, 0));
        d->slidePictures = true;
        QCOMPARE(Renderer::processedImage(*d, *pic, QSizeF()).pixelColor(0, 0), QColor(255, 82, 0));
        // What the program writes of its newer corrections (a14:imgProps) has the changes in the picture's own pixels already: the picture
        // is drawn as it is, not changed twice.
        auto newer = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\"><a:extLst><a:ext uri=\"{BEBA8EAE-BF5A-486C-A8C5-ECC9F3942E4B}\">"
                                                        "<a14:imgProps xmlns:a14=\"http://schemas.microsoft.com/office/drawing/2010/main\"><a14:imgLayer r:embed=\"rId2\"><a14:imgEffect>"
                                                        "<a14:brightnessContrast bright=\"15000\"/></a14:imgEffect></a14:imgLayer></a14:imgProps></a:ext></a:extLst></a:blip>"),
                                         &err);
        QVERIFY2(newer, qPrintable(err));
        QVERIFY2(colorsNear(colorOfSamplePicture(*newer), QColor(0xB6, 0x42, 0x0F)), qPrintable(colorOfSamplePicture(*newer).name()));
    }

    // The other changes a picture's blip lists (a duotone, a color replaced or changed, a hue turned, a tint) are drawn in their order after the
    // settings, and written back as listed, with the picture's bytes as they were; a gray or black and white picture keeps its threshold.
    void importPicturesAreDrawnWithTheirOtherEffects()
    {
        QString err;
        const QString duotone = QStringLiteral("<a:duotone><a:srgbClr val=\"000080\"/><a:srgbClr val=\"FFFF00\"/></a:duotone>");
        auto d = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\">%1</a:blip>").arg(duotone), &err);
        QVERIFY2(d, qPrintable(err));
        // 182, 66, 15 is 97 in gray, 38% of the way from the first color to the second.
        QVERIFY2(colorsNear(colorOfSamplePicture(*d), QColor(97, 97, 79)), qPrintable(colorOfSamplePicture(*d).name()));
        auto replaced = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\"><a:clrRepl><a:srgbClr val=\"00FF00\"/></a:clrRepl></a:blip>"), &err);
        QVERIFY2(replaced, qPrintable(err));
        QVERIFY2(colorsNear(colorOfSamplePicture(*replaced), QColor(0, 255, 0)), qPrintable(colorOfSamplePicture(*replaced).name()));
        auto changed = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\"><a:clrChange><a:clrFrom><a:srgbClr val=\"B6420F\"/></a:clrFrom><a:clrTo><a:srgbClr val=\"0000FF\"/></a:clrTo></a:clrChange></a:blip>"), &err);
        QVERIFY2(changed, qPrintable(err));
        QVERIFY2(colorsNear(colorOfSamplePicture(*changed), QColor(0, 0, 255)), qPrintable(colorOfSamplePicture(*changed).name()));
        // Black and white from a quarter of the range: 97 in gray is white; from half, it is black.
        auto quarter = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\"><a:biLevel thresh=\"25000\"/></a:blip>"), &err);
        auto half = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\"><a:biLevel thresh=\"50000\"/></a:blip>"), &err);
        QVERIFY2(quarter && half, qPrintable(err));
        QCOMPARE(colorOfSamplePicture(*quarter), QColor(255, 255, 255));
        QCOMPARE(colorOfSamplePicture(*half), QColor(0, 0, 0));
        // Written as listed, each with its colors and numbers, and read again as the same.
        auto both = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\"><a:biLevel thresh=\"25000\"/>%1<a:hsl hue=\"5400000\" sat=\"-20000\" lum=\"10000\"/>"
                                                       "<a:clrRepl><a:srgbClr val=\"FF0000\"/></a:clrRepl><a:tint hue=\"10800000\" amt=\"30000\"/></a:blip>")
                                            .arg(duotone),
                                        &err);
        QVERIFY2(both, qPrintable(err));
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*both, &err), &e, &err), qPrintable(err));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY2(slide.contains(QStringLiteral("<a:biLevel thresh=\"25000\"/>")), qPrintable(slide));
        const QString listed = QStringLiteral("%1<a:hsl hue=\"5400000\" sat=\"-20000\" lum=\"10000\"/><a:clrRepl><a:srgbClr val=\"FF0000\"/></a:clrRepl><a:tint hue=\"10800000\" amt=\"30000\"/>").arg(duotone);
        QVERIFY2(slide.contains(listed), qPrintable(slide));
        auto back = deck::importPresentation(deck::exportPresentation(*both, &err), &err);
        QVERIFY2(back, qPrintable(err));
        const auto picture = [](Document &doc) {
            for (const auto &it : doc.pages[0]->items)
                if (it->type() == ItemType::Picture) return std::static_pointer_cast<PictureItem>(it);
            return std::shared_ptr<PictureItem>();
        };
        const auto before = picture(*both), after = picture(*back);
        QVERIFY(before && after);
        // The threshold is the picture's setting, and the four others are listed, in the order the file had them.
        for (const PictureItem *pic : {before.get(), after.get()}) {
            QCOMPARE(pic->blipEffects.size(), 4);
            QVERIFY(pic->blipEffects[0].kind == BlipEffect::Duotone && pic->blipEffects[0].color == QColor(0, 0, 0x80) && pic->blipEffects[0].color2 == QColor(255, 255, 0));
            QVERIFY(pic->blipEffects[1].kind == BlipEffect::Hsl && qAbs(pic->blipEffects[1].hue - 90) < 1e-6 && qAbs(pic->blipEffects[1].saturation + 0.2) < 1e-6 &&
                    qAbs(pic->blipEffects[1].lightness - 0.1) < 1e-6);
            QVERIFY(pic->blipEffects[2].kind == BlipEffect::ColorReplace && pic->blipEffects[2].color == QColor(255, 0, 0));
            QVERIFY(pic->blipEffects[3].kind == BlipEffect::Tint && qAbs(pic->blipEffects[3].hue - 180) < 1e-6 && qAbs(pic->blipEffects[3].amount - 0.3) < 1e-6);
        }
        QVERIFY(qAbs(after->blackWhiteLevel - 0.25) < 1e-6 && after->recolor == PictureItem::BlackWhite);
        QCOMPARE(colorOfSamplePicture(*back), colorOfSamplePicture(*both));
        // A background that is a picture with a brightness and contrast is drawn with them too.
        Parts p = samplePptx();
        p.data["/ppt/slides/slide2.xml"].xml.replace("<p:bg><p:bgPr><a:solidFill><a:srgbClr val=\"112233\"/></a:solidFill><a:effectLst/></p:bgPr></p:bg>",
                                                     "<p:bg><p:bgPr><a:blipFill><a:blip r:embed=\"rId2\"><a:lum bright=\"14000\"/></a:blip><a:stretch><a:fillRect/></a:stretch></a:blipFill><a:effectLst/></p:bgPr></p:bg>");
        auto behind = deck::importPresentation(pack(p), &err);
        QVERIFY2(behind, qPrintable(err));
        QCOMPARE(behind->pages[1]->background.blipEffects.size(), 1);
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = behind.get();
        ctx.cache = &cache;
        QVERIFY2(colorsNear(Renderer::renderToImage(ctx, 1, 0.5).pixelColor(10, 10), QColor(218, 102, 51)), qPrintable(Renderer::renderToImage(ctx, 1, 0.5).pixelColor(10, 10).name()));
    }

    // The color effects a blip lists come out of the file as the file lists them, in any order, however many of a kind: drawn in that order
    // (a duotone before the brightness is not the brightness before it), none lost (a second color changed, a clear color after another change),
    // and written back as listed, with their own words (a color change that doesn't count the clearness).
    void importPictureEffectsInAnyOrderAreDrawnAndSavedAsListed()
    {
        QString err;
        const QString duotone = QStringLiteral("<a:duotone><a:srgbClr val=\"000080\"/><a:srgbClr val=\"FFFF00\"/></a:duotone>");
        const QString lum = QStringLiteral("<a:lum bright=\"30000\"/>");
        const auto change = [](const QString &from, const QString &to, const QString &toAlpha = QString(), const QString &attrs = QString()) {
            const QString target = toAlpha.isEmpty() ? QStringLiteral("<a:srgbClr val=\"%1\"/>").arg(to) : QStringLiteral("<a:srgbClr val=\"%1\">%2</a:srgbClr>").arg(to, toAlpha);
            return QStringLiteral("<a:clrChange%1><a:clrFrom><a:srgbClr val=\"%2\"/></a:clrFrom><a:clrTo>%3</a:clrTo></a:clrChange>").arg(attrs, from, target);
        };
        const QString clear = QStringLiteral("<a:alpha val=\"0\"/>");
        // The color of the sample's picture (182, 66, 15) after the same effects, made by hand in the order the file lists them.
        const auto byHand = [](const QVector<BlipEffect> &effects) {
            QImage one(1, 1, QImage::Format_ARGB32);
            one.setPixelColor(0, 0, QColor(0xB6, 0x42, 0x0F));
            return applyBlipEffects(one, effects).pixelColor(0, 0);
        };
        BlipEffect lumEffect, duotoneEffect;
        lumEffect.kind = BlipEffect::Luminance;
        lumEffect.brightness = 0.3;
        duotoneEffect.kind = BlipEffect::Duotone;
        duotoneEffect.color = QColor(0, 0, 0x80);
        duotoneEffect.color2 = QColor(255, 255, 0);
        const QColor duotoneThenLum = byHand({duotoneEffect, lumEffect}), lumThenDuotone = byHand({lumEffect, duotoneEffect});
        QVERIFY2(qAbs(duotoneThenLum.blue() - lumThenDuotone.blue()) > 20, qPrintable(duotoneThenLum.name() + " " + lumThenDuotone.name()));   // the order shows
        const auto exportedBlip = [&](Document &doc) {
            Exported e;
            if (!unpack(deck::exportPresentation(doc, &err), &e, &err)) return QString();
            const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
            const int at = int(slide.indexOf(QStringLiteral("<a:blip ")));
            const int end = int(slide.indexOf(QStringLiteral("</a:blip>"), at));
            return at < 0 || end < 0 ? QString() : slide.mid(slide.indexOf(QLatin1Char('>'), at) + 1, end - slide.indexOf(QLatin1Char('>'), at) - 1);
        };
        // A duotone before the brightness is drawn first, and written first.
        auto d = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\">%1%2</a:blip>").arg(duotone, lum), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(colorsNear(colorOfSamplePicture(*d), duotoneThenLum), qPrintable(colorOfSamplePicture(*d).name() + " not " + duotoneThenLum.name()));
        QCOMPARE(exportedBlip(*d), duotone + lum);
        auto other = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\">%1%2</a:blip>").arg(lum, duotone), &err);
        QVERIFY2(other, qPrintable(err));
        QVERIFY2(colorsNear(colorOfSamplePicture(*other), lumThenDuotone), qPrintable(colorOfSamplePicture(*other).name() + " not " + lumThenDuotone.name()));
        QCOMPARE(exportedBlip(*other), lum + duotone);
        // Two changes of a color: the sample's color becomes blue, and blue is then made clear, so the picture is not there (the slide shows).
        const QString both = change(QStringLiteral("B6420F"), QStringLiteral("0000FF")) + change(QStringLiteral("0000FF"), QStringLiteral("0000FF"), clear);
        auto two = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\">%1</a:blip>").arg(both), &err);
        QVERIFY2(two, qPrintable(err));
        QVERIFY2(colorsNear(colorOfSamplePicture(*two), QColor(255, 255, 255)), qPrintable(colorOfSamplePicture(*two).name()));
        QCOMPARE(exportedBlip(*two), both);
        // In the other order the clear one finds nothing, and the picture is blue.
        const QString reversed = change(QStringLiteral("0000FF"), QStringLiteral("0000FF"), clear) + change(QStringLiteral("B6420F"), QStringLiteral("0000FF"));
        auto blue = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\">%1</a:blip>").arg(reversed), &err);
        QVERIFY2(blue, qPrintable(err));
        QVERIFY2(colorsNear(colorOfSamplePicture(*blue), QColor(0, 0, 255)), qPrintable(colorOfSamplePicture(*blue).name()));
        QCOMPARE(exportedBlip(*blue), reversed);
        // A color change that doesn't count the clearness (useA) is written with it; one made clear is the picture's transparent color, and the
        // brightness after it is a setting, written as before.
        const QString noAlpha = change(QStringLiteral("B6420F"), QStringLiteral("B6420F"), clear, QStringLiteral(" useA=\"0\""));
        auto kept = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\">%1</a:blip>").arg(noAlpha), &err);
        QVERIFY2(kept, qPrintable(err));
        QCOMPARE(exportedBlip(*kept), noAlpha);
        QVERIFY2(colorsNear(colorOfSamplePicture(*kept), QColor(255, 255, 255)), qPrintable(colorOfSamplePicture(*kept).name()));
        const QString ordinary = change(QStringLiteral("B6420F"), QStringLiteral("B6420F"), clear) + lum + QStringLiteral("<a:grayscl/>");
        auto settings = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\">%1</a:blip>").arg(ordinary), &err);
        QVERIFY2(settings, qPrintable(err));
        QCOMPARE(exportedBlip(*settings), ordinary);
    }

    // The contrast at its most (100 percent) cuts each color at the middle, end to end: read, drawn, and written as the file says.
    void importAContrastOfAllIsDrawnAndSaved()
    {
        QString err;
        auto d = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\"><a:lum contrast=\"100000\"/></a:blip>"), &err);
        QVERIFY2(d, qPrintable(err));
        // 182, 66, 15: the red is above the middle, the others below.
        QVERIFY2(colorsNear(colorOfSamplePicture(*d), QColor(255, 0, 0)), qPrintable(colorOfSamplePicture(*d).name()));
        Exported e;
        QVERIFY2(unpack(deck::exportPresentation(*d, &err), &e, &err), qPrintable(err));
        QVERIFY2(part(e, QStringLiteral("/ppt/slides/slide1.xml")).contains(QStringLiteral("<a:lum contrast=\"100000\"/>")), qPrintable(part(e, QStringLiteral("/ppt/slides/slide1.xml"))));
        // Below the least (past -100 percent, which the standard does not have) is the least: a flat gray, not a crash.
        auto flat = deckWithPictureBlip(QStringLiteral("<a:blip r:embed=\"rId2\"><a:lum contrast=\"-100000\"/></a:blip>"), &err);
        QVERIFY2(flat, qPrintable(err));
        QVERIFY2(colorsNear(colorOfSamplePicture(*flat), QColor(128, 128, 128)), qPrintable(colorOfSamplePicture(*flat).name()));
    }

    // The picture an equation falls back to has the changes of its fill (a shape filled with the picture, and a duotone on the fill: it is
    // made a picture, which has its own place for them).
    void importAnEquationsPictureKeepsItsEffects()
    {
        Parts p = equationPptx(QStringLiteral("shape"));
        QString xml = QString::fromUtf8(p.data["/ppt/slides/slide1.xml"].xml);
        const int fallback = int(xml.indexOf(QStringLiteral("<mc:Fallback>")));
        QVERIFY(fallback > 0);
        const QRegularExpression blip(QStringLiteral("<a:blip r:embed=\"(rId\\d+)\"/>"));
        QString tail = xml.mid(fallback);
        tail.replace(blip, QStringLiteral("<a:blip r:embed=\"\\1\"><a:lum bright=\"21000\" contrast=\"52000\"/><a:duotone><a:srgbClr val=\"000080\"/><a:srgbClr val=\"FFFF00\"/></a:duotone></a:blip>"));
        QVERIFY(tail != xml.mid(fallback));
        p.data["/ppt/slides/slide1.xml"].xml = (xml.left(fallback) + tail).toUtf8();
        QString err;
        auto d = deck::importPresentation(pack(p), &err);
        QVERIFY2(d, qPrintable(err));
        const PictureItem *pic = keptPicture(*d, *d->pages[0], QStringLiteral("equation"));
        QVERIFY(pic);
        QCOMPARE(d->images.value(pic->imageId).bytes, equationPicture());   // the picture is as the file has it
        QCOMPARE(pic->blipEffects.size(), 2);
        QVERIFY(pic->blipEffects[0].kind == BlipEffect::Luminance && qAbs(pic->blipEffects[0].brightness - 0.21) < 1e-6 && qAbs(pic->blipEffects[0].contrast - 0.52) < 1e-6);
        QVERIFY(pic->blipEffects[1].kind == BlipEffect::Duotone && pic->blipEffects[1].color == QColor(0, 0, 0x80) && pic->blipEffects[1].color2 == QColor(255, 255, 0));
        // It is drawn with them: the brightness and contrast take 192, 48, 48 to 255, 44, 44, and the duotone (97 of 255 in gray, 45 between)
        // to 116, 116, 70.
        LayoutCache cache;
        PaintContext ctx;
        ctx.doc = d.get();
        ctx.cache = &cache;
        const QPointF at = pic->rect.center();
        const QColor drawn = Renderer::renderToImage(ctx, 0, 1.0).pixelColor(int(at.x()), int(at.y()));
        QVERIFY2(colorsNear(drawn, QColor(116, 116, 70), 2), qPrintable(drawn.name()));
    }

    // A picture of a presentation drawn at one size and another is changed once (not for each size, as a zoom or a thumbnail asks), and one a
    // file asks too much of (thirty-two turns of the hue) is drawn as it is, at once, at every size.
    void renderChangesAPictureOnceAndRefusesWhatCostsTooMuch()
    {
        QImage source(2000, 1500, QImage::Format_RGB32);
        source.fill(QColor(100, 150, 200));
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        source.save(&buffer, "PNG");
        auto make = [&](Document &doc, const QVector<BlipEffect> &effects) {
            auto pic = std::make_shared<PictureItem>();
            pic->imageId = doc.addImage(png, QStringLiteral("png"));
            pic->rect = QRectF(0, 0, 400, 300);
            pic->imgRect = pic->rect;
            pic->blipEffects = effects;
            return pic;
        };
        BlipEffect turn;
        turn.kind = BlipEffect::Hsl;
        turn.hue = 120;
        Document doc;
        doc.slidePictures = true;
        auto once = make(doc, {turn});
        const int before = blipAdjustedMade();
        const QImage big = Renderer::processedImage(doc, *once, QSizeF(800, 600), QSizeF());
        const QImage small = Renderer::processedImage(doc, *once, QSizeF(100, 75), QSizeF());
        const QImage exact = Renderer::processedImage(doc, *once, QSizeF(), QSizeF(333, 250));
        QCOMPARE(blipAdjustedMade(), before + 1);
        QVERIFY(big.width() != small.width());
        for (const QImage &img : {big, small, exact}) {
            const QColor c = img.pixelColor(img.width() / 2, img.height() / 2);
            QVERIFY2(qAbs(c.red() - 200) <= 2 && qAbs(c.green() - 100) <= 2 && qAbs(c.blue() - 150) <= 2, qPrintable(c.name()));
        }
        // Thirty-two of them: 3 million pixels each; the picture is drawn as it is, and at once.
        auto crowd = make(doc, QVector<BlipEffect>(32, turn));
        const int made = blipAdjustedMade();
        const qint64 ms = bestNanoseconds([&] {
            for (const QSizeF &size : {QSizeF(800, 600), QSizeF(100, 75), QSizeF(333, 250)}) {
                const QImage img = Renderer::processedImage(doc, *crowd, size, QSizeF());
                const QColor c = img.pixelColor(img.width() / 2, img.height() / 2);
                QVERIFY2(qAbs(c.red() - 100) <= 2 && qAbs(c.green() - 150) <= 2 && qAbs(c.blue() - 200) <= 2, qPrintable(c.name()));
            }
        }) / 1000000;
        QVERIFY2(ms < 2000, qPrintable(QString::number(ms)));
        QCOMPARE(blipAdjustedMade(), made);
    }

    // A metafile (clip art) drawn in black and white takes the picture's own threshold, not half: the same box of gray 100 is black from half
    // the range and white from a quarter.
    void renderBlackAndWhiteOfAMetafileFromThePicturesThreshold()
    {
        auto le16 = [](QByteArray &b, int v) { b.append(char(v & 0xFF)); b.append(char((v >> 8) & 0xFF)); };
        auto le32 = [](QByteArray &b, quint32 v) { for (int i = 0; i < 4; ++i) b.append(char((v >> (8 * i)) & 0xFF)); };
        auto words = [&](std::initializer_list<int> v) { QByteArray b; for (int w : v) le16(b, w); return b; };
        auto record = [&](int fn, const QByteArray &params) {
            QByteArray b;
            le32(b, quint32((6 + params.size() + (params.size() & 1)) / 2));
            le16(b, fn);
            b += params;
            if (params.size() & 1) b.append('\0');
            return b;
        };
        // A 400 by 300 drawing of one box over all of it, in a gray of 100 and with no outline.
        QByteArray brush = words({0}), pen = words({5, 0, 0});
        le32(brush, 0x00646464);
        le16(brush, 0);
        le32(pen, 0);
        QByteArray body = record(0x020B, words({0, 0})) + record(0x020C, words({300, 400})) + record(0x02FC, brush) + record(0x02FA, pen);
        body += record(0x012D, words({1})) + record(0x012D, words({0})) + record(0x041B, words({300, 400, 0, 0})) + record(0x0000, QByteArray());
        QByteArray head, standard;
        le32(head, 0x9AC6CDD7u);
        le16(head, 0);
        for (int v : {0, 0, 400, 300, 72}) le16(head, v);
        le32(head, 0);
        quint16 sum = 0;
        for (int i = 0; i < 20; i += 2) sum ^= quint16(quint8(head[i]) | (quint8(head[i + 1]) << 8));
        le16(head, sum);
        for (int v : {1, 9, 0x300}) le16(standard, v);
        le32(standard, quint32((18 + body.size()) / 2));
        le16(standard, 2);
        le32(standard, 60);
        le16(standard, 0);
        const QByteArray wmf = head + standard + body;
        for (const auto &want : {std::pair<double, QColor>{0.5, QColor(0, 0, 0)}, std::pair<double, QColor>{0.25, QColor(255, 255, 255)}}) {
            Document doc;
            doc.slidePictures = true;
            auto pic = std::make_shared<PictureItem>();
            pic->imageId = doc.addImage(wmf, QStringLiteral("wmf"));
            QVERIFY(!pic->imageId.isEmpty());
            pic->rect = QRectF(0, 0, 200, 150);
            pic->imgRect = pic->rect;
            pic->recolor = PictureItem::BlackWhite;
            pic->blackWhiteLevel = want.first;
            LayoutCache cache;
            PaintContext ctx;
            ctx.doc = &doc;
            ctx.cache = &cache;
            const ItemList one{pic};
            const QImage img = Renderer::renderItemsToImage(ctx, one, 1.0);
            QVERIFY(!img.isNull());
            const QColor c = img.pixelColor(img.width() / 2, img.height() / 2);
            QVERIFY2(c == want.second, qPrintable(QString::number(want.first) + ": " + c.name()));
        }
    }

    // A slide's PNG picture that has a color profile of its own (a screen's, most often) is drawn in the colors of that profile as screen
    // colors, as the presentation program draws it, not with its numbers as they are. A file that only names its gamma and primaries has
    // no profile to apply (the program leaves those out), and a publication's pictures are drawn as they always were.
    void renderPictureWithAColorProfileInScreenColors()
    {
        const QColor raw(40, 160, 200);
        QImage src(8, 8, QImage::Format_RGB32);
        src.fill(raw);
        QByteArray plain;
        QBuffer plainBuffer(&plain);
        QVERIFY(plainBuffer.open(QIODevice::WriteOnly) && src.save(&plainBuffer, "PNG"));
        src.setColorSpace(QColorSpace(QColorSpace::DisplayP3));
        QByteArray profiled;
        QBuffer profiledBuffer(&profiled);
        QVERIFY(profiledBuffer.open(QIODevice::WriteOnly) && src.save(&profiledBuffer, "PNG"));
        const QColor want = src.convertedToColorSpace(QColorSpace(QColorSpace::SRgb)).pixelColor(4, 4);
        QVERIFY2(!colorsNear(want, raw, 6), qPrintable(want.name()));
        // The plain file with a gamma and the primaries of a wide screen after its header (a chunk: length, name, data, checksum).
        auto chunk = [](const char *name, const QByteArray &data) {
            QByteArray body = QByteArray(name, 4) + data, out(4, '\0');
            qToBigEndian<quint32>(quint32(data.size()), out.data());
            out += body;
            QByteArray sum(4, '\0');
            qToBigEndian<quint32>(jp::crc32(body), sum.data());
            return out + sum;
        };
        auto be32 = [](quint32 v) { QByteArray b(4, '\0'); qToBigEndian<quint32>(v, b.data()); return b; };
        const QByteArray primaries = be32(31270) + be32(32900) + be32(68000) + be32(32000) + be32(26500) + be32(69000) + be32(15000) + be32(6000);
        const QByteArray named = plain.left(33) + chunk("gAMA", be32(45455)) + chunk("cHRM", primaries) + plain.mid(33);
        QVERIFY(QImage::fromData(named).colorSpace().isValid());   // Qt takes a color space from them
        struct Case { QByteArray file; bool slide; QColor expected; };
        for (const Case &c : {Case{profiled, true, want}, Case{named, true, raw}, Case{profiled, false, raw}}) {
            Document doc;
            doc.slidePictures = c.slide;
            auto pic = std::make_shared<PictureItem>();
            pic->imageId = doc.addImage(c.file, QStringLiteral("png"));
            QVERIFY(!pic->imageId.isEmpty());
            pic->rect = QRectF(0, 0, 80, 80);
            pic->imgRect = pic->rect;
            LayoutCache cache;
            PaintContext ctx;
            ctx.doc = &doc;
            ctx.cache = &cache;
            const ItemList one{pic};
            const QImage img = Renderer::renderItemsToImage(ctx, one, 1.0);
            QVERIFY(!img.isNull());
            const QColor got = img.pixelColor(img.width() / 2, img.height() / 2);
            QVERIFY2(colorsNear(got, c.expected, 3), qPrintable(QStringLiteral("slide %1, %2 bytes: %3 instead of %4").arg(c.slide).arg(c.file.size()).arg(got.name(), c.expected.name())));
        }
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
};

JP_TEST_CLASS(DeckBinaryShapesTests)
#include "tst_deck_binaryshapes.moc"
