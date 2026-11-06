#include "tst_write.h"


// A rule drawn as a rectangle that is anchored in a paragraph of its own (or at the end of one) is drawn where the paragraph is; a shape that says it has no fill or no
// line has none (not the fill and line a new shape is given).
void WriteTests::aRuleAnchoredAtAParagraphsEndIsDrawnAsTheFileSaysIt()
{
    const QByteArray vml = "xmlns:v=\"urn:schemas-microsoft-com:vml\"";
    auto rect = [&](const QByteArray &attributes) {
        return "<w:r><w:pict " + vml + "><v:rect id=\"r\" style=\"position:absolute;margin-left:1.5pt;margin-top:-2.25pt;width:300pt;height:1.4pt\" " + attributes + "/></w:pict></w:r>";
    };
    auto shapeOf = [&](const QByteArray &attributes, QRectF *rectOut) -> std::shared_ptr<ShapeItem> {
        auto doc = open(makeDocx("<w:p><w:r><w:t>Heading</w:t></w:r></w:p><w:p>" + rect(attributes) + "</w:p><w:p><w:r><w:t>Below</w:t></w:r></w:p>", kStyles));
        if (!doc) return nullptr;
        const auto lines = doc->pager().lines(0);
        const auto page = doc->pager().materialize(0);
        for (const auto &item : page->doc->pages[0]->items)
            if (auto shape = std::dynamic_pointer_cast<ShapeItem>(item)) {
                *rectOut = QRectF(QPointF(shape->rect.left(), shape->rect.top() - doc->pager().page(0).content.top() - lines[1].rect.top()), shape->rect.size());
                return shape;
            }
        return nullptr;
    };
    QRectF at;
    auto black = shapeOf("fillcolor=\"black\" stroked=\"f\"", &at);
    QVERIFY(black);
    QVERIFY(black->fill.type == Fill::Solid && black->stroke.isNone());
    QVERIFY2(std::abs(at.top() - (-2.25)) < 0.01 && std::abs(at.width() - 300) < 0.01, qPrintable(QString::number(at.top())));
    auto outlined = shapeOf("filled=\"f\" strokecolor=\"#165189\" strokeweight=\"1.5pt\"", &at);
    QVERIFY(outlined);
    QVERIFY(outlined->fill.isNone());
    QVERIFY2(!outlined->stroke.isNone() && std::abs(outlined->stroke.width - 1.5) < 0.01, qPrintable(QString::number(outlined->stroke.width)));
}


// A picture that cuts its edges (VML's crop attributes) is drawn whole, outside its frame by what is cut, so that the frame shows the rest; the color it keys out
// is clear, and its border is drawn round the frame. A picture in the line, and one set at a place (the page layer keeps what the file's own frame says).
void WriteTests::aPictureIsCutKeyedAndBordered()
{
    const QByteArray shape = "<v:shape id=\"p\" type=\"#_x0000_t75\" style=\"STYLEwidth:100pt;height:50pt\"><v:imagedata r:id=\"rIdPic\" croptop=\"16384f\" cropbottom=\"32768f\" cropleft=\"0.1\" "
                             "cropright=\"0.125\" chromakey=\"black\"/><w10:bordertop type=\"single\" width=\"12\"/><w10:borderleft type=\"single\" width=\"12\"/><w10:borderbottom type=\"single\" width=\"12\"/>"
                             "<w10:borderright type=\"single\" width=\"12\"/></v:shape>";
    auto body = [&](const QByteArray &style) {
        QByteArray s = shape;
        s.replace("STYLE", style);
        return "<w:p><w:r><w:t xml:space=\"preserve\">Before </w:t></w:r><w:r><w:pict " + QByteArray(kVml) + ">" + s + "</w:pict></w:r></w:p>";
    };
    // (The whole picture's size is the frame's over what the cuts leave of it: a quarter of the height, and three quarters and an eighth less than a tenth of the width.)
    const double wholeW = 100 / (1 - 0.1 - 0.125), wholeH = 50 / (1 - 0.25 - 0.5);
    auto check = [&](const std::shared_ptr<PictureItem> &pic, const QString &where) {
        QVERIFY2(pic, qPrintable(where + ": no picture"));
        QVERIFY2(std::abs(pic->rect.width() - 100) < 0.01 && std::abs(pic->rect.height() - 50) < 0.01, qPrintable(where + ": frame"));
        QVERIFY2(std::abs(pic->imgRect.width() - wholeW) < 0.01 && std::abs(pic->imgRect.height() - wholeH) < 0.01, qPrintable(where + ": whole " + QString::number(pic->imgRect.width())));
        QVERIFY2(std::abs(pic->imgRect.left() + 0.1 * wholeW) < 0.01 && std::abs(pic->imgRect.top() + 0.25 * wholeH) < 0.01, qPrintable(where + ": place"));
        QVERIFY2(pic->hasTransparentColor && pic->transparentColor == QColor(Qt::black), qPrintable(where + ": clear color"));
        QVERIFY2(!pic->stroke.isNone() && std::abs(pic->stroke.width - 1.5) < 0.01, qPrintable(where + ": border"));
    };
    auto inlineDoc = open(makeDocxWithPicture(body("")));
    QVERIFY(inlineDoc);
    check(std::dynamic_pointer_cast<PictureItem>(inlineObjectOf(*inlineDoc)), QStringLiteral("inline"));
    auto floating = open(makeDocxWithPicture(body("position:absolute;margin-left:30pt;margin-top:10pt;z-index:5;")));
    QVERIFY(floating);
    std::shared_ptr<PictureItem> placed;
    const auto page = floating->pager().materialize(0);
    for (const auto &item : page->doc->pages[0]->items)
        if (auto pic = std::dynamic_pointer_cast<PictureItem>(item)) placed = pic;
    check(placed, QStringLiteral("floating"));
}


// Lettering on a path (a banner's text, the file's text on a path) is its text set to fill its frame, in the shape's colors, with the shadow it has.
void WriteTests::letteringOnAPathIsTextArtInTheShapesColors()
{
    const QByteArray shape = "<v:shape id=\"w\" type=\"#_x0000_t136\" style=\"position:absolute;margin-left:14.4pt;margin-top:0;width:121.4pt;height:32.4pt;z-index:5\" fillcolor=\"#336699\" strokeweight=\"1.5pt\">"
                             "<v:shadow on=\"t\" color=\"#969696\" opacity=\"52429f\" offset=\"1pt,1pt\"/><v:textpath style=\"font-family:&quot;Arial Black&quot;;font-style:italic;v-text-kern:t\" trim=\"t\" fitpath=\"t\" string=\"LTIMP\"/></v:shape>";
    auto doc = open(makeDocx("<w:p><w:r><w:t>Text</w:t></w:r><w:r><w:pict " + QByteArray(kVml) + ">" + shape + "</w:pict></w:r></w:p>", kStyles));
    QVERIFY(doc);
    std::shared_ptr<TextArtItem> art;
    const auto page = doc->pager().materialize(0);
    for (const auto &item : page->doc->pages[0]->items)
        if (auto a = std::dynamic_pointer_cast<TextArtItem>(item)) art = a;
    QVERIFY(art);
    QCOMPARE(art->text, QStringLiteral("LTIMP"));
    QCOMPARE(art->font, QStringLiteral("Arial Black"));
    QVERIFY(art->italic && !art->bold);
    QVERIFY(std::abs(art->rect.width() - 121.4) < 0.01 && std::abs(art->rect.height() - 32.4) < 0.01);
    QVERIFY(art->fill.type == Fill::Solid && art->fill.color.resolve(ColorScheme()) == QColor(0x33, 0x66, 0x99));
    QVERIFY(!art->stroke.isNone() && std::abs(art->stroke.width - 1.5) < 0.01);
    QVERIFY(art->fx.shadow.on);
    // (Lettering that is set in the line is an object of it, as wide as the frame.)
    auto inlineDoc = open(makeDocx("<w:p><w:r><w:t>Text</w:t></w:r><w:r><w:pict " + QByteArray(kVml) + ">" + QByteArray(shape).replace("position:absolute;margin-left:14.4pt;margin-top:0;", "").replace("z-index:5", "") + "</w:pict></w:r></w:p>", kStyles));
    QVERIFY(inlineDoc);
    auto set = std::dynamic_pointer_cast<TextArtItem>(inlineObjectOf(*inlineDoc));
    QVERIFY(set);
    QVERIFY(std::abs(set->rect.width() - 121.4) < 0.01);
}


// A horizontal rule is set in its line, as wide as its share of the text's width (all of it when it says none), as high as it says, in its fill, and set across the
// line as it is aligned.
void WriteTests::aHorizontalRuleIsAsWideAsItsShareOfTheText()
{
    auto rule = [&](const QByteArray &attributes) {
        return "<w:p><w:r><w:pict " + QByteArray(kVml) + "><v:rect id=\"r\" style=\"width:110.25pt;height:1pt\" o:hr=\"t\" o:hrstd=\"t\" " + attributes + "/></w:pict></w:r></w:p>";
    };
    // (No section is given: the page is a letter's, and its text is 468 points wide.)
    auto doc = open(makeDocx(rule("o:hrpct=\"960\" o:hralign=\"center\" fillcolor=\"black\" stroked=\"f\""), kStyles));
    QVERIFY(doc);
    auto shape = std::dynamic_pointer_cast<ShapeItem>(inlineObjectOf(*doc));
    QVERIFY(shape);
    QVERIFY2(std::abs(shape->rect.width() - 0.96 * 468) < 0.01 && std::abs(shape->rect.height() - 1) < 0.01, qPrintable(QString::number(shape->rect.width())));
    QVERIFY(shape->fill.type == Fill::Solid && shape->stroke.isNone());
    QVERIFY2(std::abs(shape->wrap.left - shape->wrap.right) < 0.01 && std::abs(shape->wrap.left - 0.02 * 468) < 0.05, qPrintable(QString::number(shape->wrap.left)));
    // All the width, when it says none; the width it gives when it gives no share.
    auto full = std::dynamic_pointer_cast<ShapeItem>(inlineObjectOf(*open(makeDocx(rule("style=\"width:0\" fillcolor=\"gray\" stroked=\"f\"").replace("style=\"width:110.25pt;height:1pt\"", "style=\"width:0;height:2.25pt\""), kStyles))));
    QVERIFY(full);
    QVERIFY2(std::abs(full->rect.width() - 468) < 0.01 && std::abs(full->rect.height() - 2.25) < 0.01, qPrintable(QString::number(full->rect.width())));
    auto fixed = std::dynamic_pointer_cast<ShapeItem>(inlineObjectOf(*open(makeDocx(rule("o:hrpct=\"0\" fillcolor=\"black\" stroked=\"f\""), kStyles))));
    QVERIFY(fixed);
    QVERIFY2(std::abs(fixed->rect.width() - 110.25) < 0.01, qPrintable(QString::number(fixed->rect.width())));
    // A paragraph that holds only a rule is as tall as a line of text, within the few points that another machine's fonts differ by (not the rule's own height).
    auto both = open(makeDocx(rule("o:hrpct=\"960\" o:hralign=\"center\" fillcolor=\"black\" stroked=\"f\"") + "<w:p><w:r><w:t>Text</w:t></w:r></w:p>", kStyles));
    QVERIFY(both);
    const auto lines = both->pager().lines(0);
    QVERIFY(lines.size() >= 2);
    QVERIFY2(std::abs(lines[0].rect.height() - lines[1].rect.height()) < 4, qPrintable(QString::number(lines[0].rect.height()) + " against " + QString::number(lines[1].rect.height())));
    // (The room that makes the line is above and below the rule, in the middle of it; a rule among text has none, for it wraps to a line of its own as high as it is.)
    QVERIFY2(std::abs(shape->wrap.top - shape->wrap.bottom) < 0.01 && shape->wrap.top > 3, qPrintable(QString::number(shape->wrap.top)));
    auto amongText = std::dynamic_pointer_cast<ShapeItem>(inlineObjectOf(*open(makeDocx("<w:p><w:r><w:t>Text before</w:t></w:r><w:r><w:pict " + QByteArray(kVml) +
        "><v:rect id=\"r\" style=\"width:0;height:1pt\" o:hr=\"t\" fillcolor=\"black\" stroked=\"f\"/></w:pict></w:r></w:p>", kStyles))));
    QVERIFY(amongText);
    QCOMPARE(amongText->wrap.top, 0.0);
    QCOMPARE(amongText->wrap.bottom, 0.0);
    // In a cell, its share is of the cell's text (a 100 point column less the usual 5.4 point margins at each side).
    const QByteArray cell = "<w:tbl><w:tblPr><w:tblW w:w=\"2000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr>"
                            + rule("o:hrpct=\"500\" fillcolor=\"black\" stroked=\"f\"") + "</w:tc></w:tr></w:tbl><w:p><w:r><w:t>after</w:t></w:r></w:p>";
    auto inCell = std::dynamic_pointer_cast<ShapeItem>(inlineObjectOf(*open(makeDocx(cell, kStyles))));
    QVERIFY(inCell);
    QVERIFY2(std::abs(inCell->rect.width() - 0.5 * (100 - 10.8)) < 0.01, qPrintable(QString::number(inCell->rect.width())));
}


// A shape of the drawing format's own kind (an arc, a bracket) is that shape, not a box, and has the fill that kind has (none for these) when the file says none.
void WriteTests::aShapeOfAKindIsThatShape()
{
    const QByteArray arc = "<v:shape id=\"a\" type=\"#_x0000_t19\" style=\"position:absolute;margin-left:10pt;margin-top:0;width:27pt;height:54pt;z-index:5\"/>";
    auto doc = open(makeDocx("<w:p><w:r><w:t>Text</w:t></w:r><w:r><w:pict " + QByteArray(kVml) + ">" + arc + "</w:pict></w:r></w:p>", kStyles));
    QVERIFY(doc);
    std::shared_ptr<ShapeItem> shape;
    const auto page = doc->pager().materialize(0);
    for (const auto &item : page->doc->pages[0]->items)
        if (auto s = std::dynamic_pointer_cast<ShapeItem>(item)) shape = s;
    QVERIFY(shape);
    QCOMPARE(shape->shape, QStringLiteral("arc"));
    QVERIFY(shape->fill.isNone());
    QVERIFY(!shape->stroke.isNone());
    // (The format's arc is a quarter of an ellipse twice the frame, from the frame's top left corner to its bottom right.)
    QVERIFY(!shape->customPath.isEmpty());
    QVERIFY(std::abs(shape->customPath.pointAtPercent(0).x()) < 0.01 && std::abs(shape->customPath.pointAtPercent(0).y()) < 0.01);
    QVERIFY(std::abs(shape->customPath.pointAtPercent(1).x() - 27) < 0.01 && std::abs(shape->customPath.pointAtPercent(1).y() - 54) < 0.01);
}


// A picture whose other form is a compressed picture of the kind the original program keeps for itself (r:pict, a .pcz) is shown as a picture that cannot be: the
// frame's outline on white with the mark of a missing picture at its corner (one small picture, of the document, drawn small), not the picture it has, and not a
// picture made for each size of frame (a file of a hundred such pictures took fifteen seconds to open).
void WriteTests::aPictureWithACompressedFormIsShownAsMissing()
{
    auto bodyOf = [&](int count) {
        QByteArray body;
        for (int i = 0; i < count; ++i)
            body += "<w:p><w:r><w:pict " + QByteArray(kVml) + "><v:shape id=\"p" + QByteArray::number(i) + "\" type=\"#_x0000_t75\" style=\"width:" + QByteArray::number(100 + i) +
                    "pt;height:" + QByteArray::number(50 + i / 2) + "pt\"><v:imagedata r:id=\"rIdPic\" r:pict=\"rIdPcz\"/></v:shape></w:pict></w:r></w:p>";
        return body;
    };
    auto doc = open(makeDocxWithPicture(bodyOf(1), true));
    QVERIFY(doc);
    auto pic = std::dynamic_pointer_cast<PictureItem>(inlineObjectOf(*doc));
    QVERIFY(pic);
    QVERIFY(std::abs(pic->rect.width() - 100) < 0.01 && std::abs(pic->rect.height() - 50) < 0.01);
    QCOMPARE(int(doc->flow().images.size()), 1);
    const QImage mark = QImage::fromData(doc->flow().images[0].bytes);
    QVERIFY(!mark.isNull() && mark.width() <= 128 && mark.height() <= 128);
    // (The frame is the picture's own: white with an outline; the mark is small, at the corner, and is not the picture's 20 by 10 black and white.)
    QVERIFY2(pic->fill.type == Fill::Solid && !pic->stroke.isNone(), "a white frame with an outline");
    QVERIFY2(pic->imgRect.width() < 30 && pic->imgRect.height() < 30 && pic->imgRect.left() >= 0 && pic->imgRect.top() >= 0, qPrintable(QString::number(pic->imgRect.width())));
    // A hundred of them, of a hundred sizes: one picture of the document, and no time to speak of.
    const qint64 decodes = Document::imageDecodesForTest();
    auto many = open(makeDocxWithPicture(bodyOf(100), true));
    QVERIFY(many);
    QVERIFY2(many->flow().images.size() == 1, qPrintable(QString::number(many->flow().images.size()) + " pictures made"));
    // Counted: the one picture is decoded no more than a couple of times for all hundred (not once for each).
    QVERIFY2(Document::imageDecodesForTest() - decodes <= 4, qPrintable(QString::number(Document::imageDecodesForTest() - decodes) + " decodes"));
}


// A row that is hidden (w:hidden in its properties) is not shown and takes no room, whatever height it says (the table is as tall as the other row alone), and a table
// whose rows are all hidden shows nothing of itself, not its lines either.
void WriteTests::aHiddenRowIsNotShownAndTakesNoRoom()
{
    const QByteArray line = "w:val=\"thickThinSmallGap\" w:sz=\"24\" w:space=\"0\" w:color=\"FF0000\"";
    auto body = [&](const QByteArray &firstRow, const QByteArray &secondRow) {
        auto row = [](const QByteArray &props, const QByteArray &text) {
            return "<w:tr><w:trPr>" + props + "</w:trPr><w:tc><w:tcPr><w:tcW w:w=\"5000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>" + text + "</w:t></w:r></w:p></w:tc></w:tr>";
        };
        return "<w:p><w:r><w:t>Before</w:t></w:r></w:p><w:tbl><w:tblPr><w:tblW w:w=\"5000\" w:type=\"dxa\"/><w:tblBorders><w:top " + line + "/><w:left " + line + "/><w:bottom " + line + "/><w:right " + line +
               "/><w:insideH " + line + "/></w:tblBorders></w:tblPr><w:tblGrid><w:gridCol w:w=\"5000\"/></w:tblGrid>" + (firstRow.isEmpty() ? QByteArray() : row(firstRow, "First row")) +
               (secondRow.isEmpty() ? QByteArray() : row(secondRow, "Second row")) + "</w:tbl><w:p><w:r><w:t>After</w:t></w:r></w:p>";
    };
    // The height of the table's piece on the page, and whether a line is drawn at its top and bottom edges (the table's own, or the first cell's).
    struct Shown {
        double height = -1;
        bool topLine = false, bottomLine = false;
    };
    auto shownOf = [&](const QByteArray &xml) {
        Shown out;
        auto doc = open(makeDocx(xml, kStyles));
        if (!doc) return out;
        const auto page = doc->pager().materialize(0);
        if (page->tables.empty()) return out;
        out.height = page->tables[0]->height;
        // (The top edge is the first cell's that does not say it has no line there, as a hidden row's cells do; the bottom edge is the last such cell's.)
        bool seen = false;
        for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
            if (const int key = cellKeyOf(b)) {
                const flow::CellRecord *record = doc->flow().tables.get(key);
                if (record->cell.top && !record->cell.top->visible()) continue;
                const auto edge = [&](const std::optional<flow::TableLine> &own, const flow::TableLine &table) { return own ? own->visible() : table.visible(); };
                if (!seen) out.topLine = edge(record->cell.top, record->table->top);
                seen = true;
                out.bottomLine = edge(record->cell.bottom, record->table->bottom);
            }
        return out;
    };
    const QByteArray exactHundred = "<w:trHeight w:val=\"2000\" w:hRule=\"exact\"/>", exactEighty = "<w:hidden/><w:trHeight w:val=\"1600\" w:hRule=\"exact\"/>";
    const Shown alone = shownOf(body(exactHundred, ""));
    QVERIFY2(alone.height >= 99 && alone.height < 125, qPrintable(QString::number(alone.height)));
    QVERIFY(alone.topLine && alone.bottomLine);
    // An eighty point row that is hidden beside it adds nothing (a table that showed it would be 80 points taller).
    const Shown beside = shownOf(body(exactEighty, exactHundred));
    QVERIFY2(std::abs(beside.height - alone.height) < 1, qPrintable(QString::number(beside.height) + " beside, " + QString::number(alone.height) + " alone"));
    QVERIFY(beside.topLine && beside.bottomLine);
    // Every row hidden: no room, and no line at any edge (the table with its row shown has both).
    const Shown none = shownOf(body(exactEighty, ""));
    QVERIFY2(none.height >= 0 && none.height < 1, qPrintable(QString::number(none.height)));
    QVERIFY(!none.topLine && !none.bottomLine);
}


// A row put beside a hidden row is a row like the file's (a row of a normal height, with its cells' lines and margins), not a copy of what the hidden row is made to be
// (no height, no lines).
void WriteTests::aRowInsertedBesideAHiddenRowTakesANormalHeight()
{
    const QByteArray line = "w:val=\"single\" w:sz=\"8\" w:space=\"0\" w:color=\"000000\"";
    const QByteArray body =
        "<w:p><w:r><w:t>Before</w:t></w:r></w:p><w:tbl><w:tblPr><w:tblW w:w=\"5000\" w:type=\"dxa\"/><w:tblBorders><w:top " + line + "/><w:left " + line + "/><w:bottom " + line + "/><w:right " + line + "/><w:insideH " + line +
        "/></w:tblBorders></w:tblPr><w:tblGrid><w:gridCol w:w=\"5000\"/></w:tblGrid>"
        "<w:tr><w:trPr><w:hidden/></w:trPr><w:tc><w:tcPr><w:tcW w:w=\"5000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>Hidden row</w:t></w:r></w:p></w:tc></w:tr>"
        "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"5000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>Shown row</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>After</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles));
    QVERIFY(doc);
    const double before = doc->pager().materialize(0)->tables[0]->height;
    QTextBlock hidden;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
        if (b.text() == QStringLiteral("Hidden row")) hidden = b;
    QVERIFY(hidden.isValid());
    QTextCursor cursor(doc->story());
    cursor.setPosition(hidden.position());
    insertTableRow(*doc, cursor, true);
    const auto page = doc->pager().materialize(0);
    QVERIFY(!page->tables.empty());
    const auto &table = page->tables[0];
    QCOMPARE(table->layout->rowCount(), 3);
    // (The new row is the middle one: as tall as a row of a line of text, which the hidden one is not.)
    QVERIFY2(table->heightOf(1) > 10, qPrintable(QString::number(table->heightOf(1))));
    QVERIFY2(table->height > before + 10, qPrintable(QString::number(table->height) + " from " + QString::number(before)));
    // The cells of the new row have the lines the file's rows do.
    const flow::TableLayout::Row &row = table->layout->row(1);
    const flow::CellRecord *record = table->layout->cell(row.cell0).rec;
    QVERIFY(!record->cell.top || record->cell.top->visible());
    QVERIFY(!record->cell.margins || record->cell.margins->top() >= 0);
}


// Text of the automatic color is drawn white on a dark fill, and nothing of it is saved: the file keeps the run as it was (no w:color), for the color is worked out as the
// page is laid out. Text typed into an empty dark paragraph, and a number or bullet, are drawn so as well.
void WriteTests::automaticTextIsDrawnWhiteOnADarkFillAndNothingIsSaved()
{
    const QByteArray shading = "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"000000\"/><w:spacing w:before=\"0\" w:after=\"0\"/>";
    const QRectF region(100, 77, 50, 7);   // (inside the first line's letters, which are 14 point)
    auto doc = open(makeDocx("<w:p><w:pPr>" + shading + "</w:pPr><w:r><w:rPr><w:sz w:val=\"28\"/></w:rPr><w:t>WWWWWWWW</w:t></w:r></w:p>", kStyles));
    QVERIFY(doc);
    const BackAndInk drawn = backAndInk(drawnPage(*doc), region);
    QVERIFY2(drawn.back < 60 && drawn.ink > 200, qPrintable(QString::number(drawn.back) + " on, " + QString::number(drawn.ink) + " ink"));
    const QByteArray saved = documentXml(doc->toDocx());
    QVERIFY2(!saved.contains("<w:color"), qPrintable(QString::fromUtf8(saved.mid(saved.indexOf("<w:body"), 700))));
    QVERIFY(saved.contains("w:fill=\"000000\""));
    // Typed into an empty dark paragraph (its mark's format is the text's).
    auto empty = open(makeDocx("<w:p><w:pPr>" + shading + "<w:rPr><w:sz w:val=\"28\"/></w:rPr></w:pPr></w:p>", kStyles));
    QVERIFY(empty);
    {
        PageView view;
        view.resize(1000, 700);
        view.setDocument(empty.get());
        view.cursor().setPosition(0);
        view.insertText(QStringLiteral("WWWWWWWW"));
        const BackAndInk typed = backAndInk(view.pageImage(0, 816), region);
        QVERIFY2(typed.back < 60 && typed.ink > 200, qPrintable(QString::number(typed.back) + " on, " + QString::number(typed.ink) + " ink typed"));
    }
    QVERIFY2(!documentXml(empty->toDocx()).contains("<w:color"), "typing saves no color");
    // A number: an empty numbered dark paragraph shows its number white.
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/><w:pPr><w:ind w:left=\"0\" w:firstLine=\"0\"/></w:pPr>"
                                 "<w:rPr><w:sz w:val=\"28\"/></w:rPr></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
    auto numbered = open(makeDocx("<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr>" + shading + "<w:rPr><w:sz w:val=\"28\"/></w:rPr></w:pPr></w:p>", kStyles, numbering));
    QVERIFY(numbered);
    const BackAndInk number = backAndInk(drawnPage(*numbered), QRectF(73, 77, 14, 7));
    QVERIFY2(number.back < 60 && number.ink > 200, qPrintable(QString::number(number.back) + " on, " + QString::number(number.ink) + " ink in the number"));
}


// The fill nearest the text decides the color of its automatic text: its run's highlight, then its paragraph's shading, then its cell's (a cell that has none is on its
// table's shading, and a table in a cell is on the cell's, so a cell with no fill in a dark cell is dark), and a light fill nearer than a dark one is the one.
void WriteTests::theFillNearestTheTextDecidesItsAutomaticColor()
{
    auto shd = [](const char *fill) { return "<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"" + QByteArray(fill) + "\"/>"; };
    auto paragraph = [&](const char *fill, const char *runProps) {
        return "<w:p><w:pPr>" + (fill ? shd(fill) : QByteArray()) + "<w:spacing w:before=\"0\" w:after=\"0\"/></w:pPr><w:r><w:rPr><w:sz w:val=\"28\"/>" + QByteArray(runProps) + "</w:rPr><w:t>WWWWWWWW</w:t></w:r></w:p>";
    };
    auto cell = [&](const char *fill, const QByteArray &content) {
        return "<w:tc><w:tcPr><w:tcW w:w=\"6000\" w:type=\"dxa\"/>" + (fill ? shd(fill) : QByteArray()) + "</w:tcPr>" + content + "</w:tc>";
    };
    auto table = [&](const char *tableFill, const QByteArray &cells) {
        return "<w:tbl><w:tblPr><w:tblW w:w=\"6000\" w:type=\"dxa\"/>" + (tableFill ? shd(tableFill) : QByteArray()) + "</w:tblPr><w:tblGrid><w:gridCol w:w=\"6000\"/></w:tblGrid><w:tr>" + cells + "</w:tr></w:tbl>";
    };
    struct Case {
        const char *name;
        QByteArray body;
        bool white;   // (the text is white on a dark fill, else dark on a light one)
    };
    const QVector<Case> cases = {
        {"plain paragraph", paragraph(nullptr, ""), false},
        {"dark paragraph, light highlight", paragraph("000000", "<w:highlight w:val=\"yellow\"/>"), false},
        {"dark paragraph, dark highlight", paragraph("FFFFFF", "<w:highlight w:val=\"black\"/>"), true},
        {"light paragraph in a dark cell", table(nullptr, cell("000000", paragraph("FFFFFF", ""))) + "<w:p/>", false},
        {"plain paragraph in a dark cell", table(nullptr, cell("000000", paragraph(nullptr, ""))) + "<w:p/>", true},
        {"cell with no fill in a dark cell", table(nullptr, cell("000000", table(nullptr, cell(nullptr, paragraph(nullptr, ""))) + "<w:p/>")) + "<w:p/>", true},
        {"light cell in a dark cell", table(nullptr, cell("000000", table(nullptr, cell("FFFFFF", paragraph(nullptr, ""))) + "<w:p/>")) + "<w:p/>", false},
        {"dark table shading, cell with none", table("000000", cell(nullptr, paragraph(nullptr, ""))) + "<w:p/>", true},
        {"dark table shading, light cell", table("000000", cell("D9D9D9", paragraph(nullptr, ""))) + "<w:p/>", false},
    };
    QStringList failures;
    for (const Case &c : cases) {
        auto doc = open(makeDocx(c.body, kStyles));
        if (!doc) {
            failures << QString::fromLatin1(c.name) + ": not opened";
            continue;
        }
        const BackAndInk look = backAndInk(drawnPage(*doc), QRectF(100, 77, 50, 7));
        const bool right = c.white ? look.back < 90 && look.ink > 190 : look.back > 160 && look.ink < 70;
        if (!right) failures << QStringLiteral("%1: %2 on, %3 ink").arg(QString::fromLatin1(c.name)).arg(look.back).arg(look.ink);
        if (documentXml(doc->toDocx()).contains("<w:color")) failures << QString::fromLatin1(c.name) + ": a color is saved";
    }
    QVERIFY2(failures.isEmpty(), qPrintable(failures.join(QLatin1String("; "))));
}


// A rule that is a share of the text's width is a share of its paragraph's line: its section's text (the section it is in, not the last), less the paragraph's indents.
void WriteTests::aRulesWidthFollowsItsSectionAndItsIndents()
{
    auto rule = [&](const QByteArray &paragraphProperties) {
        return "<w:p><w:pPr>" + paragraphProperties + "</w:pPr><w:r><w:pict " + QByteArray(kVml) + "><v:rect id=\"r\" style=\"width:0;height:1pt\" o:hr=\"t\" o:hrstd=\"t\" fillcolor=\"black\" stroked=\"f\"/></w:pict></w:r></w:p>";
    };
    const QByteArray threeInches = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"4320\" w:bottom=\"1440\" w:left=\"4320\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocx(rule(threeInches) + rule(QByteArray()), kStyles));   // (the first section ends with the first paragraph: its pages are 3 inches in from each side)
    QVERIFY(doc);
    const QVector<ItemPtr> rules = inlineObjectsOf(*doc);
    QCOMPARE(int(rules.size()), 2);
    QVERIFY2(std::abs(rules[0]->rect.width() - 180) < 0.01, qPrintable(QString::number(rules[0]->rect.width()) + " in the first section"));
    QVERIFY2(std::abs(rules[1]->rect.width() - 468) < 0.01, qPrintable(QString::number(rules[1]->rect.width()) + " in the last"));
    // A rule of a width of its own goes no further than its line.
    auto wide = open(makeDocx("<w:p><w:r><w:pict " + QByteArray(kVml) + "><v:rect id=\"r\" style=\"width:600pt;height:1pt\" o:hr=\"t\" o:hrpct=\"0\" fillcolor=\"black\" stroked=\"f\"/></w:pict></w:r></w:p>", kStyles));
    QVERIFY(wide);
    const QVector<ItemPtr> wides = inlineObjectsOf(*wide);
    QCOMPARE(int(wides.size()), 1);
    QVERIFY2(std::abs(wides[0]->rect.width() - 468) < 0.01, qPrintable(QString::number(wides[0]->rect.width()) + " for 600 points asked"));
    // Indented 72 points at the left and 36 at the right.
    auto indented = open(makeDocx(rule("<w:ind w:left=\"1440\" w:right=\"720\"/>"), kStyles));
    QVERIFY(indented);
    const QVector<ItemPtr> narrow = inlineObjectsOf(*indented);
    QCOMPARE(int(narrow.size()), 1);
    QVERIFY2(std::abs(narrow[0]->rect.width() - (468 - 72 - 36)) < 0.01, qPrintable(QString::number(narrow[0]->rect.width()) + " indented"));
}


// Lettering on a path is bounded: a document draws no more than a few lettering shapes and a few thousand characters of it, and no frame of it is larger than a few
// pages (each letter is outlined and stroked, and a file of a thousand shapes of five hundred letters took minutes).
void WriteTests::letteringIsBoundedInCountLettersAndSize()
{
    auto lettering = [&](int index, int letters, const char *size) {
        return "<w:r><w:pict " + QByteArray(kVml) + "><v:shape id=\"w" + QByteArray::number(index) + "\" type=\"#_x0000_t136\" style=\"position:absolute;margin-left:10pt;margin-top:" + QByteArray::number(index % 40) +
               "pt;" + size + ";z-index:5\" fillcolor=\"black\" stroked=\"f\"><v:textpath style=\"font-family:Arial\" string=\"" + QByteArray(letters, 'W') + "\"/></v:shape></w:pict></w:r>";
    };
    QByteArray runs;
    for (int i = 0; i < 300; ++i) runs += lettering(i, 50, "width:100pt;height:20pt");
    auto doc = open(makeDocx("<w:p><w:r><w:t>Text</w:t></w:r>" + runs + "</w:p>", kStyles));
    QVERIFY(doc);
    int shapes = 0, letters = 0;
    for (const flow::FloatingObject &o : doc->flow().objects)
        if (const auto art = std::dynamic_pointer_cast<TextArtItem>(o.item)) {
            ++shapes;
            letters += int(art->text.size());
        }
    QVERIFY2(shapes <= kMostLetteringShapes, qPrintable(QString::number(shapes) + " lettering shapes"));
    QVERIFY2(letters <= kMostLetteringCharacters, qPrintable(QString::number(letters) + " letters"));
    QVERIFY2(shapes > 10, "the first of them are drawn");
    // A frame is no larger than a few pages.
    auto big = open(makeDocx("<w:p><w:r><w:t>Text</w:t></w:r>" + lettering(1, 5, "width:90000pt;height:70000pt") + "</w:p>", kStyles));
    QVERIFY(big);
    for (const flow::FloatingObject &o : big->flow().objects)
        if (const auto art = std::dynamic_pointer_cast<TextArtItem>(o.item)) {
            QVERIFY2(art->rect.width() <= kMostLetteringSide && art->rect.height() <= kMostLetteringSide, qPrintable(QString::number(art->rect.width())));
            return;
        }
    QFAIL("a lettering shape is made");
}


// The outline of lettering is kept: drawn again at its size it costs a small part of the first drawing (a stroked outline of five hundred letters took three quarters of
// a second each time it was drawn).
void WriteTests::letteringDrawnAgainCostsLittle()
{
    auto art = std::make_shared<TextArtItem>();
    art->text = QString(400, QLatin1Char('W'));
    art->font = QStringLiteral("Arial");
    art->rect = QRectF(0, 0, 600, 100);
    art->fill = Fill::solid(ColorRef::rgb(Qt::blue));
    art->stroke = Stroke::line(ColorRef::rgb(Qt::black), 1.5);
    Document doc;
    LayoutCache cache;
    PaintContext ctx;
    ctx.doc = &doc;
    ctx.cache = &cache;
    auto paint = [&] {
        QImage img(700, 150, QImage::Format_ARGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.translate(10, 10);
        Renderer::paintItem(&p, ctx, *art);
    };
    // Counted: the outline of the lettering is made once, and so is the picture of it (the Text Art is long); drawn again, nothing is made.
    const qint64 outlines = Renderer::textArtOutlinesMadeForTest(), pictures = Renderer::textArtPicturesMadeForTest();
    paint();
    const qint64 outlinesFirst = Renderer::textArtOutlinesMadeForTest(), picturesFirst = Renderer::textArtPicturesMadeForTest();
    QVERIFY2(outlinesFirst - outlines == 1 && picturesFirst - pictures <= 1, qPrintable(QStringLiteral("%1 outlines and %2 pictures the first time").arg(outlinesFirst - outlines).arg(picturesFirst - pictures)));
    paint();
    QCOMPARE(Renderer::textArtOutlinesMadeForTest(), outlinesFirst);
    QCOMPARE(Renderer::textArtPicturesMadeForTest(), picturesFirst);
}


// A picture's border has the sides the file sets (a single w10:bordertop is one line, not a frame), in the color it says (or the picture's line color): all four sides are
// the picture's outline, fewer are lines at a picture that is set at a place, and a picture in the line has the outline only if all four are set.
void WriteTests::aPicturesBorderHasTheSidesTheFileSets()
{
    auto shape = [&](const QByteArray &style, const QByteArray &attributes, const QByteArray &sides) {
        return "<w:p><w:r><w:t xml:space=\"preserve\">Before </w:t></w:r><w:r><w:pict " + QByteArray(kVml) + "><v:shape id=\"p\" type=\"#_x0000_t75\" style=\"" + style + "width:100pt;height:50pt\" " + attributes +
               "><v:imagedata r:id=\"rIdPic\"/>" + sides + "</v:shape></w:pict></w:r></w:p>";
    };
    auto side = [](const char *name) { return "<w10:border" + QByteArray(name) + " type=\"single\" width=\"12\"/>"; };
    const QByteArray fixed = "position:absolute;margin-left:30pt;margin-top:10pt;z-index:5;";
    auto linesOf = [&](const QByteArray &xml, std::shared_ptr<PictureItem> *picture) {
        QVector<std::shared_ptr<LineItem>> lines;
        auto doc = open(makeDocxWithPicture(xml));
        if (!doc) return lines;
        const auto page = doc->pager().materialize(0);
        for (const auto &item : page->doc->pages[0]->items) {
            if (const auto l = std::dynamic_pointer_cast<LineItem>(item)) lines << l;
            if (const auto p = std::dynamic_pointer_cast<PictureItem>(item)) *picture = p;
        }
        return lines;
    };
    QByteArray xml = shape(fixed, "", side("top"));
    std::shared_ptr<PictureItem> pic;
    QVector<std::shared_ptr<LineItem>> lines = linesOf(xml, &pic);
    QVERIFY(pic);
    QVERIFY2(pic->stroke.isNone(), "one side is not an outline");
    QCOMPARE(int(lines.size()), 1);
    QVERIFY2(std::abs(lines[0]->p2.x() - lines[0]->p1.x() - 100) < 0.01 && std::abs(lines[0]->p2.y() - lines[0]->p1.y()) < 0.01 && std::abs(lines[0]->stroke.width - 1.5) < 0.01, "a line along the top, 1.5 points");
    // Left and bottom: two lines, down the left and along the bottom.
    xml = shape(fixed, "", side("left") + side("bottom"));
    lines = linesOf(xml, &pic);
    QCOMPARE(int(lines.size()), 2);
    // All four: the outline, no lines. In the color the file says.
    xml = shape(fixed, "", side("top") + side("left") + side("bottom") + side("right"));
    lines = linesOf(xml, &pic);
    QVERIFY(pic && !pic->stroke.isNone() && lines.isEmpty());
    xml = shape(fixed, "o:bordertopcolor=\"#FF0000\" o:borderleftcolor=\"#FF0000\" o:borderbottomcolor=\"#FF0000\" o:borderrightcolor=\"#FF0000\"", side("top") + side("left") + side("bottom") + side("right"));
    lines = linesOf(xml, &pic);
    QVERIFY(pic);
    QCOMPARE(pic->stroke.color.resolve(ColorScheme()), QColor(Qt::red));
    // "this" is the picture's line color (strokecolor).
    xml = shape(fixed, "strokecolor=\"#0000FF\" o:bordertopcolor=\"this\" o:borderleftcolor=\"this\" o:borderbottomcolor=\"this\" o:borderrightcolor=\"this\"", side("top") + side("left") + side("bottom") + side("right"));
    lines = linesOf(xml, &pic);
    QVERIFY(pic);
    QCOMPARE(pic->stroke.color.resolve(ColorScheme()), QColor(Qt::blue));
    // In the line, a single side is no outline (and no frame of four sides).
    auto inlineDoc = open(makeDocxWithPicture(shape("", "", side("top"))));
    QVERIFY(inlineDoc);
    auto set = std::dynamic_pointer_cast<PictureItem>(inlineObjectOf(*inlineDoc));
    QVERIFY(set);
    QVERIFY2(set->stroke.isNone(), "an inline picture with one side set has no outline");
}


// A text box that holds a table lays it out in the box: the rows are drawn under the paragraph before them, as wide as their columns, in the room the box has, and the paragraph that
// takes their place in the box's text is as tall as they are (so the text after them goes below). The fill a box says it has none of (v:fill opacity 0, however it is written) is not
// drawn, and one that is opaque is.
void WriteTests::aTableInATextBoxIsLaidOutInTheBox()
{
        const QByteArray cell = "<w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>CELLTEXT</w:t></w:r></w:p></w:tc>";
        const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"4000\" w:type=\"dxa\"/><w:tblLayout w:type=\"fixed\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid>"
                                 "<w:tr>" + cell + cell + "</w:tr><w:tr>" + cell + cell + "</w:tr></w:tbl>";
        const QByteArray box = "<w:r><w:pict " + QByteArray(kVml) + "><v:shape id=\"b\" style=\"position:absolute;margin-left:100pt;margin-top:200pt;width:300pt;height:250pt;z-index:3;"
                               "mso-position-horizontal-relative:page;mso-position-vertical-relative:page\" stroked=\"f\"><v:fill opacity=\"0\" color2=\"black\"/>"
                               "<v:textbox inset=\"10pt,0,0,0\"><w:txbxContent><w:p><w:r><w:t>BOXTITLE</w:t></w:r></w:p>" + table + "<w:p><w:r><w:t>BOXAFTER</w:t></w:r></w:p></w:txbxContent></v:textbox></v:shape></w:pict></w:r>";
        auto doc = open(makeDocx("<w:p><w:r><w:t>Body</w:t></w:r>" + box + "</w:p>", kStyles));
        QVERIFY(doc);
        const auto page = doc->pager().materialize(0);
        QVERIFY(page);
        std::shared_ptr<TextItem> item;
        QString text;
        for (const auto &candidate : page->doc->pages[0]->items)
            if (const auto textItem = std::dynamic_pointer_cast<TextItem>(candidate))
                if (page->doc->story(textItem->storyId) && page->doc->story(textItem->storyId)->doc->toPlainText().contains(QLatin1String("BOXTITLE"))) {
                    item = textItem;
                    text = page->doc->story(textItem->storyId)->doc->toPlainText();
                }
        QVERIFY2(item, "the text box is on the page");
        QVERIFY2(text.contains(QLatin1String("BOXAFTER")) && !text.contains(QLatin1String("CELLTEXT")), qPrintable(text));   // (the cells are drawn by their rows)
        QCOMPARE(page->furnitureTables.size(), 1);
        const auto &rows = page->furnitureTables.first();
        QCOMPARE(rows->layout->cellCount(), 4);
        const QRectF at = rows->rect();
        QVERIFY2(std::abs(at.width() - 200) < 0.5, qPrintable(QString::number(at.width())));   // (two columns of 100 points)
        // (The rows' edge is where the box's text begins, or left of it by the room a cell keeps at its side: the first cell's text is where the text of the paragraphs is.)
        QVERIFY2(at.left() > item->rect.left() + 10 - 6 && at.left() < item->rect.left() + 10 + 0.5, qPrintable(QStringLiteral("rows at %1, the box's text at %2").arg(at.left()).arg(item->rect.left() + 10)));
        QVERIFY2(at.top() > item->rect.top() + 1 && at.bottom() < item->rect.bottom(), qPrintable(QStringLiteral("rows from %1 to %2 in a box from %3 to %4").arg(at.top()).arg(at.bottom()).arg(item->rect.top()).arg(item->rect.bottom())));
        // The paragraph in their place is as tall as they are: the last line of the story holds nothing but the paragraph after.
        const QTextBlock place = page->doc->story(item->storyId)->doc->findBlockByNumber(1);
        QVERIFY2(place.blockFormat().lineHeightType() == QTextBlockFormat::FixedHeight && std::abs(place.blockFormat().lineHeight() - at.height()) < 0.01,
                 qPrintable(QStringLiteral("the place is %1 high, the rows %2").arg(place.blockFormat().lineHeight()).arg(at.height())));
        // The box has no fill where the file says it has none (an opacity of nothing, in any of its forms); an opaque one, or half of one, is drawn, and so is one that fades in
        // from nothing (a second opacity that is not nothing).
        QCOMPARE(item->fill.type, Fill::NoFill);
        auto fillWith = [&](const QByteArray &fillElement) {
            QByteArray b = box;
            b.replace("<v:fill opacity=\"0\" color2=\"black\"/>", fillElement);
            auto other = open(makeDocx("<w:p><w:r><w:t>Body</w:t></w:r>" + b + "</w:p>", kStyles));
            if (!other) return -2;
            const auto made = other->pager().materialize(0);
            if (!made) return -2;
            for (const auto &candidate : made->doc->pages[0]->items)
                if (const auto textItem = std::dynamic_pointer_cast<TextItem>(candidate))
                    if (made->doc->story(textItem->storyId) && made->doc->story(textItem->storyId)->doc->toPlainText().contains(QLatin1String("BOXTITLE"))) return int(textItem->fill.type);
            return -1;   // (the box is not on the page)
        };
        for (const char *opacity : {"0", "0%", "0f", "0.0"})
            QVERIFY2(fillWith(QByteArray("<v:fill opacity=\"") + opacity + "\"/>") == int(Fill::NoFill), opacity);
        for (const char *opacity : {"1", "50%", "32768f", ".5"})
            QVERIFY2(fillWith(QByteArray("<v:fill opacity=\"") + opacity + "\"/>") == int(Fill::Solid), opacity);
        QVERIFY2(fillWith("<v:fill opacity=\"0\" o:opacity2=\"1\" color2=\"black\" type=\"gradient\"/>") == int(Fill::Solid), "a fill that fades in from nothing");
    }


// A box filled with a picture that has no opacity is not drawn with it.
void WriteTests::aPictureFillOfNoOpacityIsNotDrawn()
{
    auto fillOf = [&](const QByteArray &fill) {
        auto doc = open(makeDocxWithPicture("<w:p><w:r><w:t>Text</w:t></w:r><w:r><w:pict " + QByteArray(kVml) + "><v:rect id=\"t\" style=\"position:absolute;margin-left:10pt;margin-top:5pt;width:120pt;height:40pt;z-index:3\" stroked=\"f\">" + fill +
                                             "<v:textbox inset=\"0,0,0,0\"><w:txbxContent><w:p><w:r><w:t>BOXTEXT</w:t></w:r></w:p></w:txbxContent></v:textbox></v:rect></w:pict></w:r></w:p>"));
        if (!doc) return -2;
        const auto page = doc->pager().materialize(0);
        if (!page) return -2;
        for (const auto &item : page->doc->pages[0]->items)
            if (const auto box = std::dynamic_pointer_cast<TextItem>(item))
                if (page->doc->story(box->storyId) && page->doc->story(box->storyId)->doc->toPlainText().contains(QLatin1String("BOXTEXT"))) return int(box->fill.type);
        return -1;
    };
    QCOMPARE(fillOf("<v:fill r:id=\"rIdPic\" type=\"frame\"/>"), int(Fill::Picture));
    QCOMPARE(fillOf("<v:fill r:id=\"rIdPic\" type=\"frame\" opacity=\"0\"/>"), int(Fill::NoFill));
}


// A text box of a header holds its table as one of the body does: the rows are drawn in the box, and the cells' paragraphs are not stacked in its text.
void WriteTests::aTableInATextBoxOfAHeaderIsLaidOutInTheBox()
{
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray cell = "<w:tc><w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>CELLTEXT</w:t></w:r></w:p></w:tc>";
    const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"4000\" w:type=\"dxa\"/><w:tblLayout w:type=\"fixed\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid>"
                             "<w:tr>" + cell + cell + "</w:tr></w:tbl>";
    const QByteArray shape = "<w:r><w:pict><v:shape id=\"s1\" style=\"position:absolute;margin-left:100pt;margin-top:30pt;width:300pt;height:100pt;z-index:1;mso-position-horizontal-relative:page;"
                             "mso-position-vertical-relative:page\" filled=\"f\" stroked=\"f\"><v:textbox><w:txbxContent><w:p><w:r><w:t>HEADBOX</w:t></w:r></w:p>" + table +
                             "<w:p/></w:txbxContent></v:textbox><w10:wrap type=\"none\"/></v:shape></w:pict></w:r>";
    QByteArray header = headerPart("hdr", {shape});
    header.replace("<w:hdr ", "<w:hdr xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\" ");
    auto doc = open(makeDocxWithParts(paragraphs(3, 5), sect, {{QStringLiteral("/word/header1.xml"), header}}));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    QCOMPARE(page->furnitureTables.size(), 1);
    QString text;
    QRectF box;
    for (const auto &candidate : page->doc->pages[0]->items)
        if (const auto textItem = std::dynamic_pointer_cast<TextItem>(candidate))
            if (page->doc->story(textItem->storyId) && page->doc->story(textItem->storyId)->doc->toPlainText().contains(QLatin1String("HEADBOX"))) {
                text = page->doc->story(textItem->storyId)->doc->toPlainText();
                box = textItem->rect;
            }
    QVERIFY2(text.contains(QLatin1String("HEADBOX")) && !text.contains(QLatin1String("CELLTEXT")), qPrintable(text));
    // The rows are in the box (a point's room for rounding), below the box's first paragraph.
    const QRectF rows = page->furnitureTables.first()->rect();
    QVERIFY2(box.adjusted(-1, -1, 1, 1).contains(rows) && rows.top() > box.top() + 1, qPrintable(QStringLiteral("rows %1,%2 %3x%4 in a box at %5,%6 %7x%8").arg(rows.left()).arg(rows.top()).arg(rows.width()).arg(rows.height()).arg(box.left()).arg(box.top()).arg(box.width()).arg(box.height())));
}


// The rows of a box's table are clipped to the box: a table taller than its box shows nothing below it. (Ink below the box is counted.)
void WriteTests::aTextBoxClipsItsTable()
{
    const QByteArray row = "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"4000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:rPr><w:b/><w:sz w:val=\"28\"/></w:rPr><w:t>ROWTEXT</w:t></w:r></w:p></w:tc></w:tr>";
    QByteArray rows;
    for (int i = 0; i < 12; ++i) rows += row;
    const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"4000\" w:type=\"dxa\"/><w:tblLayout w:type=\"fixed\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"4000\"/></w:tblGrid>" + rows + "</w:tbl>";
    const QByteArray box = "<w:r><w:pict " + QByteArray(kVml) + "><v:shape id=\"b\" style=\"position:absolute;margin-left:100pt;margin-top:300pt;width:300pt;height:60pt;z-index:3;"
                           "mso-position-horizontal-relative:page;mso-position-vertical-relative:page\" filled=\"f\" stroked=\"f\"><v:textbox inset=\"0,0,0,0\"><w:txbxContent><w:p><w:r><w:t>BOXTITLE</w:t></w:r></w:p>" +
                           table + "<w:p/></w:txbxContent></v:textbox></v:shape></w:pict></w:r>";
    auto doc = open(makeDocx("<w:p><w:r><w:t>Body</w:t></w:r>" + box + "</w:p>", kStyles));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    QCOMPARE(page->furnitureTables.size(), 1);
    QVERIFY2(page->furnitureTables.first()->rect().bottom() > 300 + 60 + 40, "the table is taller than the box");
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    const QImage img = view.pageImage(0, 816);
    const double perPoint = img.width() / 612.0;
    int below = 0, inside = 0;
    for (int y = int(300 * perPoint); y < int(520 * perPoint); ++y)
        for (int x = int(100 * perPoint); x < int(400 * perPoint); ++x) (y < int(360 * perPoint) ? inside : below) += qGray(img.pixel(x, y)) < 128;
    QVERIFY2(inside > 50, "the box shows its rows");
    QVERIFY2(below == 0, qPrintable(QStringLiteral("%1 dark points of the table below its box").arg(below)));
}


// The tables of text boxes in a header are laid out once for the pages, not again on each page: the boxes' tables are held apart from the header's own, and from one another's, within
// their own bound. (Counted: the tables laid out over twenty pages.)
void WriteTests::theTablesOfAHeadersTextBoxesAreNotLaidOutAgainOnEachPage()
{
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    QByteArray cells;
    for (int i = 0; i < 190; ++i) cells += "<w:tc><w:tcPr><w:tcW w:w=\"200\" w:type=\"dxa\"/></w:tcPr><w:p/></w:tc>";
    QByteArray grid;
    for (int i = 0; i < 190; ++i) grid += "<w:gridCol w:w=\"200\"/>";
    const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"38000\" w:type=\"dxa\"/><w:tblLayout w:type=\"fixed\"/></w:tblPr><w:tblGrid>" + grid + "</w:tblGrid><w:tr>" + cells + "</w:tr><w:tr>" + cells + "</w:tr></w:tbl>";
    QByteArray shapes;
    for (int i = 0; i < 5; ++i)
        shapes += "<w:r><w:pict><v:shape id=\"s" + QByteArray::number(i) + "\" style=\"position:absolute;margin-left:" + QByteArray::number(20 + i * 10) + "pt;margin-top:30pt;width:300pt;height:100pt;z-index:" +
                  QByteArray::number(i + 1) + ";mso-position-horizontal-relative:page;mso-position-vertical-relative:page\" filled=\"f\" stroked=\"f\"><v:textbox><w:txbxContent><w:p><w:r><w:t>HEADBOX</w:t></w:r></w:p>" + table +
                  "<w:p/></w:txbxContent></v:textbox><w10:wrap type=\"none\"/></v:shape></w:pict></w:r>";
    QByteArray header = QByteArray("<w:hdr ") + kNs + " xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\">" + table + "<w:p>" + shapes + "</w:p></w:hdr>";
    auto doc = open(makeDocxWithParts(paragraphs(200, 60), sect, {{QStringLiteral("/word/header1.xml"), header}}));
    QVERIFY(doc);
    const int pages = doc->pager().pageCount();
    QVERIFY2(pages >= 20, qPrintable(QString::number(pages)));
    const qint64 before = doc->pager().headerTablesLaidOutForTest();
    for (int i = 0; i < 20; ++i) QVERIFY(doc->pager().materialize(i));
    const qint64 made = doc->pager().headerTablesLaidOutForTest() - before;
    QVERIFY2(made <= 12, qPrintable(QStringLiteral("%1 layouts of tables for 20 pages of a header with 6 stories of tables").arg(made)));
}


// The tables of text boxes are bounded in all, whatever the number of boxes: a file of many boxes each with a table of a thousand cells has the tables of the first laid out in
// their boxes, and the rest kept as they were (their boxes are drawn without them).
void WriteTests::theTablesOfManyTextBoxesAreBoundedInAll()
{
    QByteArray grid, cells;
    for (int i = 0; i < 250; ++i) {
        grid += "<w:gridCol w:w=\"200\"/>";
        cells += "<w:tc><w:tcPr><w:tcW w:w=\"200\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>x</w:t></w:r></w:p></w:tc>";
    }
    const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"50000\" w:type=\"dxa\"/><w:tblLayout w:type=\"fixed\"/></w:tblPr><w:tblGrid>" + grid + "</w:tblGrid>"
                             "<w:tr>" + cells + "</w:tr><w:tr>" + cells + "</w:tr><w:tr>" + cells + "</w:tr><w:tr>" + cells + "</w:tr></w:tbl>";
    QByteArray body = "<w:p><w:r><w:t>Body</w:t></w:r>";
    const int boxes = 12;
    for (int i = 0; i < boxes; ++i)
        body += "<w:r><w:pict " + QByteArray(kVml) + "><v:shape id=\"b" + QByteArray::number(i) + "\" style=\"position:absolute;margin-left:100pt;margin-top:" + QByteArray::number(60 + i * 40) +
                "pt;width:300pt;height:30pt;z-index:3;mso-position-horizontal-relative:page;mso-position-vertical-relative:page\" stroked=\"f\"><v:textbox><w:txbxContent><w:p><w:r><w:t>BOX" +
                QByteArray::number(i) + "</w:t></w:r></w:p>" + table + "<w:p/></w:txbxContent></v:textbox></v:shape></w:pict></w:r>";
    body += "</w:p>";
    auto doc = open(makeDocx(body, kStyles));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    qint64 laid = 0;
    for (const auto &rows : page->furnitureTables) laid += rows->layout->cellCount();
    QVERIFY2(laid > 0, "the first box has its table");
    QVERIFY2(laid <= 2000, qPrintable(QStringLiteral("%1 cells of the boxes' tables are laid out").arg(laid)));
    int withText = 0;
    for (const auto &candidate : page->doc->pages[0]->items)
        if (const auto textItem = std::dynamic_pointer_cast<TextItem>(candidate))
            if (page->doc->story(textItem->storyId) && page->doc->story(textItem->storyId)->doc->toPlainText().contains(QLatin1String("BOX"))) ++withText;
    QCOMPARE(withText, boxes);   // (every box is on the page, with its own text)
}


// A box or a text box filled with a picture (a stretched fill) is still a box or a text box with the picture as its fill: the text box keeps its text, and the box its outline.
void WriteTests::aShapeFilledWithAPictureKeepsItsTextAndItsOutline()
{
    auto shapeOn = [&](const QByteArray &shape, const QByteArray &which, std::shared_ptr<Item> *found, QString *text) {
        auto doc = open(makeDocxWithPicture("<w:p><w:r><w:t>Text</w:t></w:r><w:r><w:pict " + QByteArray(kVml) + ">" + shape + "</w:pict></w:r></w:p>"));
        if (!doc) return false;
        const auto page = doc->pager().materialize(0);
        if (!page) return false;
        for (const auto &item : page->doc->pages[0]->items) {
            if (which == "text") {
                if (const auto box = std::dynamic_pointer_cast<TextItem>(item))
                    if (page->doc->story(box->storyId) && page->doc->story(box->storyId)->doc->toPlainText().contains(QLatin1String("BOXTEXT"))) {
                        *found = item;
                        *text = page->doc->story(box->storyId)->doc->toPlainText();
                    }
            } else if (const auto s = std::dynamic_pointer_cast<ShapeItem>(item)) {
                *found = s;
            }
        }
        return bool(*found);
    };
    std::shared_ptr<Item> item;
    QString text;
    QVERIFY2(shapeOn("<v:rect id=\"t\" style=\"position:absolute;margin-left:10pt;margin-top:5pt;width:120pt;height:40pt;z-index:3\" stroked=\"f\"><v:fill r:id=\"rIdPic\" type=\"frame\"/>"
                     "<v:textbox inset=\"0,0,0,0\"><w:txbxContent><w:p><w:r><w:t>BOXTEXT</w:t></w:r></w:p></w:txbxContent></v:textbox></v:rect>", "text", &item, &text), "the text box is on the page with its text");
    QCOMPARE(item->fill.type, Fill::Picture);
    QVERIFY2(!item->fill.imageId.isEmpty(), "the picture is its fill");
    item.reset();
    QVERIFY2(shapeOn("<v:rect id=\"r\" style=\"position:absolute;margin-left:10pt;margin-top:5pt;width:120pt;height:40pt;z-index:3\" strokecolor=\"#0000FF\" strokeweight=\"2pt\"><v:fill r:id=\"rIdPic\" type=\"frame\"/></v:rect>",
                     "shape", &item, &text), "the box is on the page");
    QCOMPARE(item->fill.type, Fill::Picture);
    QVERIFY2(!item->fill.imageId.isEmpty(), "the picture is its fill");
    QVERIFY2(!item->stroke.isNone() && std::abs(item->stroke.width - 2) < 0.01, "its outline is kept");
    QCOMPARE(item->stroke.color.resolve(ColorScheme()), QColor(Qt::blue));
}


// A picture too large for what it is drawn at is made smaller before its color is keyed out (a 7000 by 7000 picture was keyed at its 49 million pixels, 190 MB, when it is
// drawn without a size to go by, as a PDF is made).
void WriteTests::aBigKeyedPictureIsKeyedAtADrawnSize()
{
    QImage big(7000, 7000, QImage::Format_Indexed8);
    big.setColorCount(2);
    big.setColor(0, qRgb(0, 0, 0));
    big.setColor(1, qRgb(200, 30, 30));
    big.fill(0);
    for (int y = 0; y < 7000; y += 2)
        for (int x = 3000; x < 4000; ++x) big.setPixel(x, y, 1);
    QByteArray png;
    {
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        QVERIFY(big.save(&buffer, "PNG", 1));
    }
    Document doc;
    ImageData data;
    data.bytes = png;
    data.format = QStringLiteral("png");
    doc.images.insert(QStringLiteral("big"), data);
    PictureItem pic;
    pic.imageId = QStringLiteral("big");
    pic.rect = pic.imgRect = QRectF(0, 0, 300, 300);
    pic.hasTransparentColor = true;
    pic.transparentColor = QColor(Qt::black);
    const QImage keyed = Renderer::processedImage(doc, pic, QSizeF(), QSizeF());
    QVERIFY(!keyed.isNull());
    QVERIFY2(qint64(keyed.width()) * keyed.height() <= 16'000'000, qPrintable(QString::number(keyed.width()) + " by " + QString::number(keyed.height())));
    // (The color is keyed out: the corner is clear.)
    QCOMPARE(qAlpha(keyed.pixel(2, 2)), 0);
}


// An arc has the angles it says (VML's adj: where it starts and where it ends, in sixty-fifths of a thousandth of a degree, clockwise from the right), and the format's
// own (from the top to the right) where it says none or a number that is not one.
void WriteTests::anArcHasTheAnglesItSays()
{
    auto arcOf = [&](const QByteArray &adj) {
        auto doc = open(makeDocx("<w:p><w:r><w:t>Text</w:t></w:r><w:r><w:pict " + QByteArray(kVml) + "><v:shape id=\"a\" type=\"#_x0000_t19\" style=\"position:absolute;margin-left:10pt;margin-top:0;width:27pt;height:54pt;z-index:5\"" +
                                     (adj.isEmpty() ? QByteArray() : " adj=\"" + adj + "\"") + "/></w:pict></w:r></w:p>", kStyles));
        std::shared_ptr<ShapeItem> shape;
        if (!doc) return shape;
        const auto page = doc->pager().materialize(0);
        for (const auto &item : page->doc->pages[0]->items)
            if (auto s = std::dynamic_pointer_cast<ShapeItem>(item)) shape = s;
        return shape;
    };
    auto ends = [&](const QByteArray &adj) {
        const auto s = arcOf(adj);
        return s ? QPair<QPointF, QPointF>(s->customPath.pointAtPercent(0), s->customPath.pointAtPercent(1)) : QPair<QPointF, QPointF>(QPointF(-1, -1), QPointF(-1, -1));
    };
    auto near = [](const QPointF &a, double x, double y) { return std::abs(a.x() - x) < 0.05 && std::abs(a.y() - y) < 0.05; };
    // Without adj, and with the usual numbers: from the top (the left edge) to the right (the middle).
    QVERIFY(near(ends(QByteArray()).first, 0, 0) && near(ends(QByteArray()).second, 27, 54));
    QVERIFY(near(ends("-5898240,,,21600,21600").first, 0, 0) && near(ends("-5898240,,,21600,21600").second, 27, 54));
    // From 0 degrees (the right of the ellipse, at the middle of the frame's left edge: 27, 54) to 90 degrees (below it: 0, 108).
    const auto turned = ends("0,5898240");
    QVERIFY2(near(turned.first, 27, 54) && near(turned.second, 0, 108), qPrintable(QStringLiteral("%1,%2 to %3,%4").arg(turned.first.x()).arg(turned.first.y()).arg(turned.second.x()).arg(turned.second.y())));
    // A number that is not one is the format's own.
    QVERIFY(near(ends("nan,inf").first, 0, 0) && near(ends("nan,inf").second, 27, 54));
}


// What a file's drawn objects are survives opening it, saving it, and opening that: a picture's cuts, color key, and border, lettering on a path, a rule, and a hidden row are
// kept as the file wrote them, and are drawn the same.
void WriteTests::drawnObjectsSurviveASaveAndReopen()
{
        const QByteArray body =
            "<w:p><w:r><w:t xml:space=\"preserve\">Picture </w:t></w:r><w:r><w:pict " + QByteArray(kVml) + "><v:shape id=\"p\" type=\"#_x0000_t75\" style=\"width:100pt;height:50pt\" o:bordertopcolor=\"this\" o:borderleftcolor=\"this\" "
            "o:borderbottomcolor=\"this\" o:borderrightcolor=\"this\"><v:imagedata r:id=\"rIdPic\" croptop=\"16384f\" cropbottom=\"32768f\" chromakey=\"black\"/><w10:bordertop type=\"single\" width=\"12\"/><w10:borderleft type=\"single\" width=\"12\"/>"
            "<w10:borderbottom type=\"single\" width=\"12\"/><w10:borderright type=\"single\" width=\"12\"/></v:shape></w:pict></w:r></w:p>"
            "<w:p><w:r><w:pict " + QByteArray(kVml) + "><v:shape id=\"w\" type=\"#_x0000_t136\" style=\"width:121.4pt;height:32.4pt\" fillcolor=\"#336699\" strokeweight=\"1.5pt\"><v:textpath style=\"font-family:Arial;font-style:italic\" string=\"BANNER\"/></v:shape></w:pict></w:r></w:p>"
            "<w:p><w:r><w:pict " + QByteArray(kVml) + "><v:rect id=\"r\" style=\"width:0;height:2pt\" o:hr=\"t\" o:hrpct=\"500\" o:hralign=\"center\" fillcolor=\"gray\" stroked=\"f\"/></w:pict></w:r></w:p>"
            "<w:tbl><w:tblPr><w:tblW w:w=\"5000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"5000\"/></w:tblGrid><w:tr><w:trPr><w:hidden/></w:trPr><w:tc><w:tcPr><w:tcW w:w=\"5000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>Hidden</w:t></w:r></w:p></w:tc></w:tr>"
            "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"5000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>Shown</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>After</w:t></w:r></w:p>";
        auto first = open(makeDocxWithPicture(body));
        QVERIFY(first);
        const QByteArray saved = first->toDocx();
        const QByteArray xml = documentXml(saved);
        for (const char *kept : {"croptop=\"16384f\"", "cropbottom=\"32768f\"", "chromakey=\"black\"", "w10:bordertop", "w10:borderright", "v:textpath", "string=\"BANNER\"", "o:hr=\"t\"", "o:hrpct=\"500\"", "<w:hidden/>"})
            QVERIFY2(xml.contains(kept), kept);
        QVERIFY2(!xml.contains("<w:color"), "nothing is made of what the file did not say");
        auto second = open(saved);
        QVERIFY(second);
        auto describe = [](WriteDoc &doc) {
            QStringList out;
            for (const ItemPtr &item : inlineObjectsOf(doc)) {
                if (const auto pic = std::dynamic_pointer_cast<PictureItem>(item))
                    out << QStringLiteral("picture %1 %2 %3 %4 key %5 border %6").arg(pic->imgRect.left(), 0, 'f', 2).arg(pic->imgRect.top(), 0, 'f', 2).arg(pic->imgRect.width(), 0, 'f', 2).arg(pic->imgRect.height(), 0, 'f', 2)
                               .arg(pic->hasTransparentColor).arg(pic->stroke.width);
                else if (const auto art = std::dynamic_pointer_cast<TextArtItem>(item))
                    out << QStringLiteral("lettering %1 %2 %3").arg(art->text).arg(art->rect.width()).arg(art->italic);
                else if (const auto shape = std::dynamic_pointer_cast<ShapeItem>(item))
                    out << QStringLiteral("rule %1 %2").arg(shape->rect.width(), 0, 'f', 2).arg(shape->rect.height());
            }
            const auto page = doc.pager().materialize(0);
            out << QStringLiteral("table height %1").arg(page->tables.empty() ? -1.0 : page->tables[0]->height, 0, 'f', 2);
            return out;
        };
        const QStringList a = describe(*first), b = describe(*second);
        QCOMPARE(a.size(), 4);
        QVERIFY2(a == b, qPrintable(a.join(QLatin1String(" | ")) + "   against   " + b.join(QLatin1String(" | "))));
        QVERIFY2(a[0].contains("key 1") && a[0].contains("border 1.5") && a[1].startsWith("lettering BANNER") && a[2].startsWith("rule 23") && !a[3].contains("-1"), qPrintable(a.join(QLatin1String(" | "))));
    }


// A number or bullet counts in its line by its ascent over the text's descent: a thirty-six point bullet before twelve-point text makes the line its ascent and the text's
// descent, not its whole height (the room it has below the baseline is not the line's).
void WriteTests::aBulletRaisesItsLineByItsAscentOverTheTextsDescent()
{
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"\xEF\x82\xB7\"/>"
                                 "<w:rPr><w:rFonts w:ascii=\"Symbol\" w:hAnsi=\"Symbol\"/><w:sz w:val=\"72\"/></w:rPr></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
    const QByteArray body = "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr>"
                            "<w:r><w:rPr><w:rFonts w:ascii=\"Times New Roman\" w:hAnsi=\"Times New Roman\"/><w:sz w:val=\"24\"/></w:rPr><w:t>Item</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles, numbering));
    QVERIFY(doc);
    const auto lines = doc->pager().lines(0);
    QVERIFY(!lines.isEmpty());
    // (The Symbol line has the ascent 2059 and the Times New Roman the descent 443, of 2048 to the em.)
    const double want = 36 * 2059 / 2048.0 + 12 * 443 / 2048.0;
    QVERIFY2(std::abs(lines[0].rect.height() - want) < 0.05, qPrintable(QString::number(lines[0].rect.height()) + " want " + QString::number(want)));
}


// A footnote reference that says its mark follows it as text (w:customMarkFollows) shows no number and takes none: the notes after it count from where they were.
void WriteTests::aFootnoteWithACustomMarkTakesNoNumber()
{
    const QByteArray body = "<w:p><w:r><w:t>First</w:t></w:r><w:r><w:footnoteReference w:customMarkFollows=\"1\" w:id=\"2\"/><w:t>*</w:t></w:r><w:r><w:t xml:space=\"preserve\"> second</w:t></w:r>"
                            "<w:r><w:footnoteReference w:id=\"3\"/></w:r></w:p>";
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"2\"><w:p><w:r><w:t>* CUSTOM note</w:t></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"3\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> AUTO note</w:t></w:r></w:p></w:footnote></w:footnotes>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    auto marks = [&](const QString &text) {
        QStringList out;
        const TextItem *box = boxWithText(*page, text);
        if (!box) return QStringList{QStringLiteral("no box")};
        for (QTextBlock b = page->doc->story(box->storyId)->doc->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QString code = it.fragment().charFormat().stringProperty(tp::Field);
                if (code.startsWith(QLatin1String("raw:"))) out << code;
            }
        return out;
    };
    QCOMPARE(marks(QStringLiteral("second")), (QStringList{QStringLiteral("raw:"), QStringLiteral("raw:1")}));   // (the custom reference shows nothing; the other is the first number)
    QCOMPARE(marks(QStringLiteral("AUTO note")), (QStringList{QStringLiteral("raw:1")}));
}


// The notes of a page sit at its foot whatever follows the page: the first paragraph of the next section, which a section break takes to the next page, has a note of its own,
// and the room for it is not kept under this page's notes.
void WriteTests::aSectionBreakDoesNotKeepNoteRoomOnThePageBefore()
{
    const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
    const QByteArray margins = "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";
    const QByteArray body = "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/><w:sectPr>" + margins + "</w:sectPr></w:pPr><w:r><w:t>One</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>"
                            "<w:p>" + exact + "<w:r><w:t>Two</w:t></w:r><w:r><w:footnoteReference w:id=\"3\"/></w:r></w:p>";
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"2\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> FIRST note</w:t></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"3\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> SECOND note</w:t></w:r></w:p></w:footnote></w:footnotes>";
    auto doc = open(makeDocxWithParts(body, "<w:sectPr>" + margins + "</w:sectPr>", {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    QCOMPARE(doc->pager().pageCount(), 2);
    const auto page = doc->pager().materialize(0);
    const TextItem *note = boxWithText(*page, QStringLiteral("FIRST note"));
    QVERIFY(note);
    QVERIFY2(std::abs(note->rect.bottom() - (792 - 72)) < 0.1, qPrintable(QString::number(note->rect.bottom())));
}


// A file that gives the notes a continuation notice has a page keep room for it under its notes (a word processor does, whether or not a note goes on): the notes end the
// notice's height (twenty points, exactly) above the foot of the text area.
void WriteTests::aContinuationNoticeKeepsItsRoomUnderTheNotes()
{
    const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
    const QByteArray body = "<w:p>" + exact + "<w:r><w:t>Cited</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                             "<w:footnote w:type=\"continuationNotice\" w:id=\"1\"><w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"400\" w:lineRule=\"exact\"/></w:pPr><w:r><w:t>(continued)</w:t></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"2\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> THE note</w:t></w:r></w:p></w:footnote></w:footnotes>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    const TextItem *note = boxWithText(*page, QStringLiteral("THE note"));
    QVERIFY(note);
    QVERIFY2(std::abs(note->rect.bottom() - (792 - 72 - 20)) < 0.1, qPrintable(QString::number(note->rect.bottom())));
}


// A file that has its own separator for the part of a note that goes on from the page before (the continuation separator) has that part set under it: the rule is drawn in
// that paragraph (thirty points here), two fifths of it above the part, and not in the separator's.
void WriteTests::theContinuedPartOfANoteIsSetUnderTheFilesContinuationSeparator()
{
    const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
    QByteArray inNote;
    for (int i = 0; i < 14; ++i) {
        inNote += "<w:p>" + exact + "<w:r><w:t xml:space=\"preserve\">";
        for (int w = 0; w < 70; ++w) inNote += i == 13 && w == 0 ? "TAILWORD " : "word ";
        inNote += "</w:t></w:r></w:p>";
    }
    const QByteArray body = "<w:p>" + exact + "<w:r><w:t>Cited</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>" + paragraphs(40, 40);
    auto ruleToPart = [&](const QByteArray &continuation) {
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr><w:r><w:separator/></w:r></w:p></w:footnote>" +
                                 continuation + "<w:footnote w:id=\"2\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> First </w:t></w:r></w:p>" + inNote + "</w:footnote></w:footnotes>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
        if (!doc || doc->pager().pageCount() < 2) return -1000.0;
        const auto page = doc->pager().materialize(1);
        const TextItem *part = page ? boxWithText(*page, QStringLiteral("TAILWORD")) : nullptr;
        double rule = -1;
        if (page)
            for (const auto &item : page->doc->pages[0]->items)
                if (const auto *line = dynamic_cast<const LineItem *>(item.get())) rule = line->p1.y();
        return part && rule > 0 ? part->rect.top() - rule : -1000.0;
    };
    // (The separator is twelve points, the rule 0.6 of it down; the part is under the rest of it, 0.4.)
    QVERIFY2(std::abs(ruleToPart("") - 0.4 * 12) < 0.05, qPrintable(QString::number(ruleToPart(""))));
    const QByteArray own = "<w:footnote w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"600\" w:lineRule=\"exact\"/></w:pPr><w:r><w:t>(Continued)</w:t></w:r><w:r><w:separator/></w:r></w:p></w:footnote>";
    QVERIFY2(std::abs(ruleToPart(own) - 0.4 * 30) < 0.05, qPrintable(QString::number(ruleToPart(own))));
}


// The rule above the notes is drawn at three fifths of the separator paragraph (a line of thirty points here), not the fifth of it that a short rule's place was taken from.
void WriteTests::theNoteRuleIsThreeFifthsDownTheSeparator()
{
    const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
    const QByteArray body = "<w:p>" + exact + "<w:r><w:t>Cited</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"600\" w:lineRule=\"exact\"/></w:pPr>"
                             "<w:r><w:separator/></w:r></w:p></w:footnote><w:footnote w:id=\"2\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> RULED note</w:t></w:r></w:p></w:footnote></w:footnotes>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    const TextItem *note = boxWithText(*page, QStringLiteral("RULED note"));
    QVERIFY(note);
    double rule = -1;
    for (const auto &item : page->doc->pages[0]->items)
        if (const auto *line = dynamic_cast<const LineItem *>(item.get())) rule = line->p1.y();
    QVERIFY(rule > 0);
    // (The note begins where the separator ends: thirty points under the top of it; the rule is eighteen under the top, twelve above the note.)
    QVERIFY2(std::abs(note->rect.top() - rule - 12) < 0.01, qPrintable(QString::number(note->rect.top() - rule)));
}


// A separator that is taller than a quarter of the text area is cut to that quarter: one of two hundred paragraphs leaves a page its text (a hundred and sixty-two points less
// than the room above the note) and its note, and the document lays out.
void WriteTests::aTallNoteSeparatorIsCutToAQuarterOfThePage()
{
    const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
    QByteArray body = "<w:p>" + exact + "<w:r><w:t>Cited</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
    for (int i = 0; i < 120; ++i) body += "<w:p>" + exact + "<w:r><w:t>x</w:t></w:r></w:p>";
    QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\">";
    for (int i = 0; i < 200; ++i) notes += "<w:p>" + exact + "<w:r><w:t>.</w:t></w:r></w:p>";
    notes += "</w:footnote><w:footnote w:id=\"2\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> TALL note</w:t></w:r></w:p></w:footnote></w:footnotes>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    QVERIFY(doc->pager().pageCount() >= 3);
    const auto lines = doc->pager().lines(0);
    QVERIFY(lines.size() > 10);
    // (The text area is 648 points: a quarter is 162; the note is twelve, so the text ends 162 + 12 above the foot: thirty-nine lines of twelve.)
    QVERIFY2(std::abs(lines.last().rect.bottom() - lines.first().rect.top() - 39 * 12) < 0.01, qPrintable(QString::number(lines.last().rect.bottom() - lines.first().rect.top())));
    QVERIFY(boxWithText(*doc->pager().materialize(0), QStringLiteral("TALL note")));
    // A continuation notice of as many is cut to a quarter as well: the note, the separator, and the notice take 12 + 162 + 162, and the text, in lines of ten points, thirty-one.
    QByteArray notice = "<w:footnote w:type=\"continuationNotice\" w:id=\"1\">";
    for (int i = 0; i < 200; ++i) notice += "<w:p>" + exact + "<w:r><w:t>.</w:t></w:r></w:p>";
    notice += "</w:footnote>";
    QByteArray both = notes;
    both.replace("<w:footnote w:id=\"2\">", notice + "<w:footnote w:id=\"2\">");
    QByteArray tens = body;
    tens.replace("w:line=\"240\"", "w:line=\"200\"");
    auto second = open(makeDocxWithParts(tens, sect, {{QStringLiteral("/word/footnotes.xml"), both}}));
    QVERIFY(second);
    const auto more = second->pager().lines(0);
    QVERIFY(more.size() > 10);
    QVERIFY2(std::abs(more.last().rect.bottom() - more.first().rect.top() - 31 * 10) < 0.01, qPrintable(QString::number(more.last().rect.bottom() - more.first().rect.top())));
}


// With the compatibility option that a number's tab does not use the hanging indent (w:doNotUseIndentAsNumberingTabStop), the text after a number begins at the next tab
// stop past it, of the paragraph's own or the default ones, not at the indent; the lines after the first are at the indent.
void WriteTests::aNumbersTabSkipsTheHangingIndentWhenTheFileSaysSo()
{
    // Where the first line's text begins, in points from the text area's left edge: the left-most dark pixel of its row past the number, in the page as it is printed.
    double second = -1;   // where the second line of the last paragraph began
    auto startOf = [&](const QByteArray &settings, const QByteArray &levelTabs, const QByteArray &paragraphTabs) {
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1)\"/>"
                                     "<w:pPr>" + levelTabs + "<w:ind w:left=\"1440\" w:hanging=\"1440\"/></w:pPr></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
        QByteArray words;
        for (int i = 0; i < 40; ++i) words += "HHHH ";
        const QByteArray body = "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr>" + paragraphTabs + "<w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/>"
                                "<w:ind w:left=\"1440\" w:hanging=\"1440\"/></w:pPr><w:r><w:t xml:space=\"preserve\">" + words + "</w:t></w:r></w:p>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(body, sect, {}, settings, kStyles, numbering));
        const auto lines = doc->pager().lines(0);
        second = lines.size() > 1 ? lines[1].rect.left() : -1;
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        const QImage img = view.pageImage(0, 816);   // (96 to the inch: 4/3 pixel to the point; the first line is the 16 pixels under the top margin of 96)
        const double perPoint = 816 / 612.0;
        for (int x = int(std::ceil((72 + 18) * perPoint)); x < img.width(); ++x)   // (a number is under 18 points wide)
            for (int y = int(72 * perPoint); y < int(84 * perPoint); ++y)
                if (qGray(img.pixel(x, y)) < 128) return x / perPoint - 72;
        return -1.0;
    };
    const QByteArray flag = "<w:compat><w:doNotUseIndentAsNumberingTabStop/></w:compat>";
    // Without the option the text begins at the indent, 72 points.
    QVERIFY2(std::abs(startOf(QByteArray(), QByteArray(), QByteArray()) - 72) < 2, qPrintable(QString::number(startOf(QByteArray(), QByteArray(), QByteArray()))));
    // With it: the first stop after the number is the default one at 36 points (720 twentieths); the lines after the first are at the indent all the same.
    QVERIFY2(std::abs(startOf(flag, QByteArray(), QByteArray()) - 36) < 2, qPrintable(QString::number(startOf(flag, QByteArray(), QByteArray()))));
    QCOMPARE(second, 72.0);
    // The paragraph's own stop is taken (50 points: the default stops lie only past the last stop of the paragraph's own, as in the text), and so is the level's list tab
    // (54 points) when the paragraph has none of its own.
    QVERIFY2(std::abs(startOf(flag, QByteArray(), "<w:tabs><w:tab w:val=\"left\" w:pos=\"1000\"/></w:tabs>") - 50) < 2, "own stop");
    QVERIFY2(std::abs(startOf(flag, QByteArray(), "<w:tabs><w:tab w:val=\"left\" w:pos=\"500\"/></w:tabs>") - 25) < 2, "own stop before the default one");
    QVERIFY2(std::abs(startOf(flag, "<w:tabs><w:tab w:val=\"num\" w:pos=\"1080\"/></w:tabs>", QByteArray()) - 54) < 2, "list tab");
    // A stop of the level that the paragraph clears is gone.
    QVERIFY2(std::abs(startOf(flag, "<w:tabs><w:tab w:val=\"num\" w:pos=\"1080\"/></w:tabs>", "<w:tabs><w:tab w:val=\"clear\" w:pos=\"1080\"/></w:tabs>") - 36) < 2, "cleared stop");
}


// With that option and no list tab (the level has none, or the paragraph clears it or sets a stop of its own in its place), the text after a number goes to the nearest of the
// paragraph's stops, the hanging indent, and the default stops (which lie only past the last stop of the paragraph's own, as they do for a tab in the text). Reference pages:
// V004's bullets (the paragraph clears the list tab at 720; the indent is 18 points) have their text at 18, not at the default stop at 36; W124's items (indent 72, the list
// tab at 900 cleared) have theirs at 36; V094's items (the paragraph's own stop at 806, where the list tab is, and its own stops on to 8726) have theirs at 806, 40.3
// points, though the default stop at 605 lies before it.
void WriteTests::aNumberWithNoListTabGoesToTheNearestStopOrTheHangingIndent()
{
    auto startOf = [&](const QByteArray &levelPPr, const QByteArray &paragraphPPr, int defaultStop, double after) {
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1)\"/><w:pPr>" + levelPPr +
                                     "</w:pPr></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
        QByteArray words;
        for (int i = 0; i < 40; ++i) words += "HHHH ";
        const QByteArray body = "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr>" + paragraphPPr + "<w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/>" +
                                "</w:pPr><w:r><w:t xml:space=\"preserve\">" + words + "</w:t></w:r></w:p>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        const QByteArray settings = "<w:defaultTabStop w:val=\"" + QByteArray::number(defaultStop) + "\"/><w:compat><w:doNotUseIndentAsNumberingTabStop/></w:compat>";
        auto doc = open(makeDocxWithParts(body, sect, {}, settings, kStyles, numbering));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        const QImage img = view.pageImage(0, 816);
        const double perPoint = 816 / 612.0;
        for (int x = int(std::ceil((72 + after) * perPoint)); x < img.width(); ++x)   // (`after` is where the number's ink ends)
            for (int y = int(72 * perPoint); y < int(84 * perPoint); ++y)
                if (qGray(img.pixel(x, y)) < 128) return x / perPoint - 72;
        return -1.0;
    };
    const QByteArray levelTab = "<w:tabs><w:tab w:val=\"num\" w:pos=\"720\"/></w:tabs><w:ind w:left=\"720\" w:hanging=\"360\"/>";
    // The list tab cleared and the hanging indent at 18 points: the text is at the indent, before the default stop at 36.
    const double indent = startOf(levelTab, "<w:tabs><w:tab w:val=\"clear\" w:pos=\"720\"/></w:tabs><w:ind w:left=\"360\"/>", 720, 12);
    QVERIFY2(std::abs(indent - 18) < 2, qPrintable(QString::number(indent)));
    // The indent at 72 points: the default stop at 36 is nearer.
    const double nearer = startOf("<w:tabs><w:tab w:val=\"num\" w:pos=\"900\"/></w:tabs><w:ind w:left=\"1440\" w:hanging=\"1440\"/>", "<w:tabs><w:tab w:val=\"clear\" w:pos=\"900\"/></w:tabs>", 720, 12);
    QVERIFY2(std::abs(nearer - 36) < 2, qPrintable(QString::number(nearer)));
    // A stop of the paragraph's at the list tab's place (40 points) takes the list tab's place; the default stop at 30 points (605) lies before the paragraph's own
    // stops (up to the last of them, at 8726), so it is none.
    const double own = startOf("<w:tabs><w:tab w:val=\"num\" w:pos=\"806\"/></w:tabs><w:ind w:left=\"806\" w:hanging=\"532\"/>",
                               "<w:tabs><w:tab w:val=\"left\" w:pos=\"274\"/><w:tab w:val=\"left\" w:pos=\"806\"/><w:tab w:val=\"left\" w:pos=\"8726\"/></w:tabs>", 605, 26);
    QVERIFY2(std::abs(own - 40.3) < 2, qPrintable(QString::number(own)));
    // The list tab itself is still the stop where the paragraph leaves it (40 points, not the default one at 30).
    const double list = startOf("<w:tabs><w:tab w:val=\"num\" w:pos=\"806\"/></w:tabs><w:ind w:left=\"806\" w:hanging=\"532\"/>", QByteArray(), 605, 26);
    QVERIFY2(std::abs(list - 40.3) < 2, qPrintable(QString::number(list)));
}


// A paragraph's own list tab (w:tab w:val="num") is the list tab, and the level's is none then: V078's agenda items (the level's list tab at 108 points, the paragraph's at
// 18) have their text at 18, not at 108. A list tab that is not past the number (V004's, set to -337 points) is none: the text is at the hanging indent, 18 points.
void WriteTests::aParagraphsOwnListTabReplacesTheLevelsAndOneBeforeTheNumberIsNone()
{
    auto startOf = [&](const QByteArray &levelPPr, const QByteArray &paragraphPPr) {
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1)\"/><w:pPr>" + levelPPr +
                                     "</w:pPr></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
        QByteArray words;
        for (int i = 0; i < 40; ++i) words += "HHHH ";
        const QByteArray body = "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr>" + paragraphPPr + "<w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/>" +
                                "</w:pPr><w:r><w:t xml:space=\"preserve\">" + words + "</w:t></w:r></w:p>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        const QByteArray settings = "<w:defaultTabStop w:val=\"720\"/><w:compat><w:doNotUseIndentAsNumberingTabStop/></w:compat>";
        auto doc = open(makeDocxWithParts(body, sect, {}, settings, kStyles, numbering));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        const QImage img = view.pageImage(0, 816);
        const double perPoint = 816 / 612.0;
        for (int x = int(std::ceil((72 + 12) * perPoint)); x < img.width(); ++x)   // (the number's ink ends within 12 points)
            for (int y = int(72 * perPoint); y < int(84 * perPoint); ++y)
                if (qGray(img.pixel(x, y)) < 128) return x / perPoint - 72;
        return -1.0;
    };
    const double own = startOf("<w:tabs><w:tab w:val=\"num\" w:pos=\"2160\"/></w:tabs><w:ind w:left=\"2160\" w:hanging=\"360\"/>",
                               "<w:tabs><w:tab w:val=\"num\" w:pos=\"360\"/></w:tabs><w:ind w:hanging=\"2160\"/>");
    QVERIFY2(std::abs(own - 18) < 2, qPrintable(QStringLiteral("the paragraph's list tab: the text is at %1, not 18").arg(own)));
    const double before = startOf("<w:tabs><w:tab w:val=\"num\" w:pos=\"720\"/></w:tabs><w:ind w:left=\"720\" w:hanging=\"360\"/>",
                                  "<w:tabs><w:tab w:val=\"clear\" w:pos=\"720\"/><w:tab w:val=\"num\" w:pos=\"-6750\"/></w:tabs><w:ind w:left=\"360\"/>");
    QVERIFY2(std::abs(before - 18) < 2, qPrintable(QStringLiteral("a list tab before the number: the text is at %1, not 18").arg(before)));
}


// Numbering ids of one definition count on together (only a restart makes a list of its own), and a level written out in a numbering id (lvlOverride) is no restart.
void WriteTests::numberingIdsOfOneDefinitionCountOnTogether()
{
    const QByteArray level = "<w:lvl w:ilvl=\"0\"><w:start w:val=\"3\"/><w:numFmt w:val=\"lowerLetter\"/><w:lvlText w:val=\"%1.\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl>";
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\">" + level + "</w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>"
                                 "<w:num w:numId=\"2\"><w:abstractNumId w:val=\"0\"/><w:lvlOverride w:ilvl=\"0\">" + level + "</w:lvlOverride></w:num>"
                                 "<w:num w:numId=\"3\"><w:abstractNumId w:val=\"0\"/></w:num>"
                                 "<w:num w:numId=\"4\"><w:abstractNumId w:val=\"0\"/><w:lvlOverride w:ilvl=\"0\"><w:startOverride w:val=\"1\"/></w:lvlOverride></w:num>";
    auto item = [](int num) { return QByteArray("<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"") + QByteArray::number(num) + "\"/></w:numPr></w:pPr><w:r><w:t>item</w:t></w:r></w:p>"; };
    auto doc = open(makeDocx(item(1) + item(2) + item(2) + item(3) + item(4) + item(4), kStyles, numbering));
    QVERIFY(doc);
    QStringList markers;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
        if (b.textList()) markers << (b.blockFormat().stringProperty(tp::BulletChar).isEmpty() ? b.textList()->itemText(b) : b.blockFormat().stringProperty(tp::BulletChar));
    QCOMPARE(markers, (QStringList{QStringLiteral("c."), QStringLiteral("d."), QStringLiteral("e."), QStringLiteral("f."), QStringLiteral("a."), QStringLiteral("b.")}));   // (the restart is a list of its own)
}


// A list the older programs made (w:legacy) has its text begin at the indent, where the file's option that the hanging indent is no tab stop would send it to the next
// default stop.
void WriteTests::aLegacyNumbersTextBeginsAtTheIndentWhateverTheTabStopOption()
{
    auto startOf = [&](bool legacy, bool option) {
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1)\"/>" +
                                     (legacy ? QByteArray("<w:legacy w:legacy=\"1\" w:legacySpace=\"0\" w:legacyIndent=\"1440\"/>") : QByteArray()) +
                                     "<w:pPr><w:ind w:left=\"1440\" w:hanging=\"1440\"/></w:pPr></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
        QByteArray words;
        for (int i = 0; i < 40; ++i) words += "HHHH ";
        const QByteArray body = "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>"
                                "<w:r><w:t xml:space=\"preserve\">" + words + "</w:t></w:r></w:p>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        const QByteArray settings = option ? QByteArray("<w:compat><w:doNotUseIndentAsNumberingTabStop/></w:compat>") : QByteArray();
        auto doc = open(makeDocxWithParts(body, sect, {}, settings, kStyles, numbering));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        const QImage img = view.pageImage(0, 816);
        const double perPoint = 816 / 612.0;
        for (int x = int(std::ceil((72 + 18) * perPoint)); x < img.width(); ++x)   // (a number is under 18 points wide)
            for (int y = int(72 * perPoint); y < int(84 * perPoint); ++y)
                if (qGray(img.pixel(x, y)) < 128) return x / perPoint - 72;
        return -1.0;
    };
    QVERIFY2(std::abs(startOf(false, true) - 36) < 2, qPrintable(QString::number(startOf(false, true))));   // (the option: the next default stop)
    QVERIFY2(std::abs(startOf(true, true) - 72) < 2, qPrintable(QString::number(startOf(true, true))));    // (a legacy level: the indent)
    QVERIFY2(std::abs(startOf(true, false) - 72) < 2, qPrintable(QString::number(startOf(true, false))));
}


// Space shrinking is worked only where a line could take another word: a paragraph none of whose lines can (here words of 120 points, three to a line with the room left under
// the width of another) is not measured through or laid out again.
void WriteTests::spaceShrinkingIsWorkedOnlyWhereALineCouldTakeAnotherWord()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const qint64 passes = spaceShrinkWorkForTest(1), relaid = spaceShrinkWorkForTest(2), full = spaceShrinkWorkForTest(4);
    auto doc = open(makeDocxWithParts(justifiedParagraphs("xxxxxxxxxxxxxxxxxxxx", 150, 12), sect, {}, "<w:compat><w:wpJustification/></w:compat>", kStyles));
    QVERIFY(doc);
    QVERIFY(doc->pager().pageCount() > 3);
    QCOMPARE(spaceShrinkWorkForTest(1) - passes, qint64(0));
    QCOMPARE(spaceShrinkWorkForTest(2) - relaid, qint64(0));
    QCOMPARE(spaceShrinkWorkForTest(4) - full, qint64(0));
}


// The page that draws a paragraph is set from what the flow's layout of it came to: where lines could take another word, those paragraphs are worked on once, in laying out the
// pages, and drawing the pages works on none again (but the one that goes over from a page before, whose text begins in the middle of its paragraph).
void WriteTests::spaceShrinkingIsSharedWithThePageThatDrawsTheParagraph()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const qint64 passes = spaceShrinkWorkForTest(1), shared = spaceShrinkWorkForTest(3);
    auto doc = open(makeDocxWithParts(justifiedParagraphs(QByteArray(), 120, 90), sect, {}, "<w:compat><w:wpJustification/></w:compat>", kStyles));
    QVERIFY(doc);
    QVERIFY(doc->pager().pageCount() > 3);
    const qint64 worked = spaceShrinkWorkForTest(1) - passes;
    QVERIFY2(worked > 20, qPrintable(QString::number(worked)));
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    for (int k = 0; k < 3; ++k) view.pageImage(k, 400);
    const qint64 again = spaceShrinkWorkForTest(1) - passes - worked;
    QVERIFY2(again <= 3, qPrintable(QString::number(again) + " paragraphs worked on again of the 40 or so on three pages"));
    QVERIFY2(spaceShrinkWorkForTest(3) - shared > 20, qPrintable(QString::number(spaceShrinkWorkForTest(3) - shared)));
}


// A line made as wide as its room on purpose stays one line: a row of letters that exactly fills it (the room of 432 points holds 72 underscores of 12 points), text that is
// aligned at a right tab stop that is at the margin, and the text of a table's cell that is as wide as the cell is.
void WriteTests::aLineThatExactlyFillsItsRoomStaysOne()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1800\" w:bottom=\"1440\" w:left=\"1800\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto linesOf = [&](const QByteArray &body) {
        auto doc = open(makeDocxWithParts(body, sect, {}, QByteArray(), kStyles));
        return int(doc->pager().lines(0).size());
    };
    auto run = [](const QByteArray &text) { return QByteArray("<w:r><w:rPr><w:rFonts w:ascii=\"Tinos\" w:hAnsi=\"Tinos\"/><w:sz w:val=\"24\"/></w:rPr><w:t xml:space=\"preserve\">") + text + "</w:t></w:r>"; };
    // A row of letters: 72 of 6 points fill the 432.
    QCOMPARE(linesOf("<w:p>" + run(QByteArray(72, '_')) + "</w:p>"), 1);
    QCOMPARE(linesOf("<w:p>" + run(QByteArray(73, '_')) + "</w:p>"), 2);
    // A right tab stop at the margin: what follows the tab ends at it.
    const QByteArray right = "<w:pPr><w:tabs><w:tab w:val=\"right\" w:pos=\"8640\"/></w:tabs></w:pPr>";
    QCOMPARE(linesOf("<w:p>" + right + run("Federal Communications Commission") + "<w:r><w:tab/></w:r>" + run("DA 07-932") + "</w:p>"), 1);
    // A cell of 4536 twentieths (226.8 points) with the margins of 108 on each side holds 216 points: 36 underscores.
    auto cellRows = [&](int underscores) {
        const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"4536\" w:type=\"dxa\"/><w:tblLayout w:type=\"fixed\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"4536\"/></w:tblGrid>"
                                 "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"4536\" w:type=\"dxa\"/></w:tcPr><w:p>" + run(QByteArray(underscores, '_')) + "</w:p></w:tc></w:tr></w:tbl><w:p/>";
        auto doc = open(makeDocxWithParts(table, sect, {}, QByteArray(), kStyles));
        const auto lines = doc->pager().lines(0);
        return lines.isEmpty() ? -1.0 : lines[0].rect.height();
    };
    const double one = cellRows(35), full = cellRows(36);
    QVERIFY2(one > 0 && std::abs(full - one) < 0.5, qPrintable(QString::number(full) + " against " + QString::number(one)));
}


// With the older program's full justification (w:wpJustification) a justified line gives up part of its spaces to take in one more word: for some width of the text area its
// first line holds a word more than without the option, and for none fewer.
void WriteTests::fullJustificationTakesInAWordWhereTheSpacesCanGiveWay()
{
    auto firstLineWords = [&](bool option, int rightMargin) {
        QByteArray words;
        for (int i = 0; i < 90; ++i) words += "abcdefg ";
        const QByteArray body = "<w:p><w:pPr><w:jc w:val=\"both\"/></w:pPr><w:r><w:t xml:space=\"preserve\">" + words + "</w:t></w:r></w:p>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"" + QByteArray::number(rightMargin) +
                                "\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        const QByteArray settings = option ? QByteArray("<w:compat><w:wpJustification/></w:compat>") : QByteArray();
        auto doc = open(makeDocxWithParts(body, sect, {}, settings, kStyles));
        const auto lines = doc->pager().lines(0);
        if (lines.isEmpty()) return -1;
        return int(doc->story()->toPlainText().mid(lines[0].pos, lines[0].end - lines[0].pos).trimmed().count(QLatin1Char(' ')) + 1);
    };
    int more = 0;
    for (int margin = 1440; margin < 1440 + 20 * 60; margin += 20) {
        const int plain = firstLineWords(false, margin), given = firstLineWords(true, margin);
        QVERIFY2(plain > 0 && given >= plain, qPrintable(QString::number(margin) + ": " + QString::number(given) + " against " + QString::number(plain)));
        more += given > plain;
    }
    QVERIFY2(more > 3, qPrintable(QString::number(more)));
}


// A list that Qt counts as the file does is counted by Qt (so that it goes on counting as the person edits it): only what it cannot count is given a marker of its own.
// Here a definition has numbering ids that no paragraph uses, beside the one that is.
void WriteTests::aListQtCanCountIsNotGivenMarkersOfItsOwn()
{
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/></w:lvl></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num><w:num w:numId=\"2\"><w:abstractNumId w:val=\"0\"/></w:num>"
                                 "<w:num w:numId=\"3\"><w:abstractNumId w:val=\"0\"/></w:num>";
    auto doc = open(makeDocx(itemOf(1, 0) + itemOf(1, 0) + itemOf(1, 0), kStyles, numbering));
    QVERIFY(doc);
    int items = 0;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
        if (b.textList()) {
            ++items;
            QVERIFY2(!b.blockFormat().hasProperty(tp::BulletChar), "Qt's own count is the marker");
        }
    QCOMPARE(items, 3);
    QCOMPARE(markersOf(*doc), (QStringList{QStringLiteral("1."), QStringLiteral("2."), QStringLiteral("3.")}));
}


// The items of numbering ids that count together are numbered through editing: Enter in the middle of such a list, and deleting an item, number the rest again, and a
// single undo takes the Enter and the numbers back.
void WriteTests::editingASharedListNumbersTheRestAgain()
{
    const QByteArray level = "<w:lvl w:ilvl=\"0\"><w:start w:val=\"3\"/><w:numFmt w:val=\"lowerLetter\"/><w:lvlText w:val=\"%1.\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl>";
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\">" + level + "</w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num><w:num w:numId=\"2\"><w:abstractNumId w:val=\"0\"/></w:num>";
    auto open3 = [&] { return open(makeDocx(itemOf(1, 0, "first") + itemOf(2, 0, "second") + itemOf(2, 0, "third"), kStyles, numbering)); };
    const QStringList three{QStringLiteral("c."), QStringLiteral("d."), QStringLiteral("e.")};
    {
        auto doc = open3();
        QVERIFY(doc);
        QCOMPARE(markersOf(*doc), three);
        QTextCursor c(doc->story());
        const QTextBlock second = doc->story()->findBlockByNumber(1);
        c.setPosition(second.position() + second.length() - 1);
        insertParagraph(*doc, c);
        QCOMPARE(markersOf(*doc), (QStringList{QStringLiteral("c."), QStringLiteral("d."), QStringLiteral("e."), QStringLiteral("f.")}));
        doc->story()->undo();
        QCOMPARE(markersOf(*doc), three);   // (one step: the Enter and the numbers go together)
    }
    {
        auto doc = open3();
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->findBlockByNumber(1).position());
        c.setPosition(doc->story()->findBlockByNumber(2).position(), QTextCursor::KeepAnchor);
        removeSelection(*doc, c);
        QCOMPARE(markersOf(*doc), (QStringList{QStringLiteral("c."), QStringLiteral("d.")}));
    }
}


// A restart at one level restarts that level only: the others of the numbering id go on with the definition's count. The ids here share the definition; the second has the
// second level restart at 5.
void WriteTests::aRestartAtOneLevelRestartsThatLevelOnly()
{
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/></w:lvl>"
                                 "<w:lvl w:ilvl=\"1\"><w:start w:val=\"1\"/><w:numFmt w:val=\"lowerLetter\"/><w:lvlText w:val=\"%2)\"/></w:lvl></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>"
                                 "<w:num w:numId=\"2\"><w:abstractNumId w:val=\"0\"/><w:lvlOverride w:ilvl=\"1\"><w:startOverride w:val=\"5\"/></w:lvlOverride></w:num>";
    auto doc = open(makeDocx(itemOf(1, 0) + itemOf(1, 1) + itemOf(1, 0) + itemOf(2, 0) + itemOf(2, 1), kStyles, numbering));
    QVERIFY(doc);
    QCOMPARE(markersOf(*doc), (QStringList{QStringLiteral("1."), QStringLiteral("a)"), QStringLiteral("2."), QStringLiteral("3."), QStringLiteral("e)")}));
}


// Two definitions that are linked to one numbering style (w:numStyleLink) are the style's definition (the one with w:styleLink): their numbering ids count together.
void WriteTests::definitionsLinkedToOneStyleShareItsLevelsAndItsCount()
{
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:styleLink w:val=\"Outline\"/><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"lowerLetter\"/><w:lvlText w:val=\"%1.\"/></w:lvl></w:abstractNum>"
                                 "<w:abstractNum w:abstractNumId=\"1\"><w:numStyleLink w:val=\"Outline\"/></w:abstractNum><w:abstractNum w:abstractNumId=\"2\"><w:numStyleLink w:val=\"Outline\"/></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num><w:num w:numId=\"2\"><w:abstractNumId w:val=\"1\"/></w:num><w:num w:numId=\"3\"><w:abstractNumId w:val=\"2\"/></w:num>";
    auto doc = open(makeDocx(itemOf(1, 0) + itemOf(2, 0) + itemOf(3, 0), kStyles, numbering));
    QVERIFY(doc);
    QCOMPARE(markersOf(*doc), (QStringList{QStringLiteral("a."), QStringLiteral("b."), QStringLiteral("c.")}));
}


// A level that a numbering id writes out itself (w:lvl in its w:lvlOverride) is that id's own: its number's format, with the definition's count.
void WriteTests::aLevelWrittenOutInANumberingIdIsUsed()
{
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/></w:lvl></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>"
                                 "<w:num w:numId=\"2\"><w:abstractNumId w:val=\"0\"/><w:lvlOverride w:ilvl=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"upperRoman\"/>"
                                 "<w:lvlText w:val=\"%1)\"/></w:lvl></w:lvlOverride></w:num>";
    auto doc = open(makeDocx(itemOf(1, 0) + itemOf(2, 0) + itemOf(2, 0), kStyles, numbering));
    QVERIFY(doc);
    QCOMPARE(markersOf(*doc), (QStringList{QStringLiteral("1."), QStringLiteral("II)"), QStringLiteral("III)")}));
}


// An item at a deeper level than any the list has had counts the levels above it as begun, at their start: the third level's first item after items of the fourth level
// (numbered 1.1.1.1 and 1.1.1.2, the third level standing at 1) is 1.1.2, not 1.1.1 again (V117's "2.4.2 Cleanliness Check", numbered so in the reference pages).
void WriteTests::anItemAfterItsDeeperLevelsGoesOnFromTheLevelsStart()
{
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/></w:lvl>"
                                 "<w:lvl w:ilvl=\"1\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.%2\"/></w:lvl>"
                                 "<w:lvl w:ilvl=\"2\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.%2.%3\"/></w:lvl>"
                                 "<w:lvl w:ilvl=\"3\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.%2.%3.%4\"/></w:lvl></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
    auto doc = open(makeDocx(itemOf(1, 0) + itemOf(1, 1) + itemOf(1, 3) + itemOf(1, 3) + itemOf(1, 2) + itemOf(1, 3) + itemOf(1, 1) + itemOf(1, 2), kStyles, numbering));
    QVERIFY(doc);
    QCOMPARE(markersOf(*doc), (QStringList{QStringLiteral("1."), QStringLiteral("1.1"), QStringLiteral("1.1.1.1"), QStringLiteral("1.1.1.2"), QStringLiteral("1.1.2"), QStringLiteral("1.1.2.1"),
                                           QStringLiteral("1.2"), QStringLiteral("1.2.1")}));
}


// A number is set in its paragraph's mark, not in what the first words of the item happen to be in: here the mark is of 10 points and the first words of 40.
void WriteTests::aNumberIsInTheSizeOfItsParagraphMark()
{
    const NumberInk marked = numberInk(QByteArray(), "<w:sz w:val=\"20\"/>", "<w:sz w:val=\"80\"/>");   // (10 point digits are about 9 pixels tall; 40 points' would be 37)
    QVERIFY2(marked.dark > 0 && marked.box.height() < 20, qPrintable(QString::number(marked.box.height())));
}


// A level's typeface is its number's: a typewriter face is wider than the document's.
void WriteTests::aNumberIsInTheTypefaceOfItsLevel()
{
    // (A number of 60 points: "1." is 45 points wide in the document's face and 72 in a typewriter face, whatever the faces on the machine are.)
    const NumberInk plain = numberInk("<w:sz w:val=\"120\"/>", "<w:sz w:val=\"20\"/>", QByteArray());
    const NumberInk courier = numberInk("<w:rFonts w:ascii=\"Courier New\" w:hAnsi=\"Courier New\"/><w:sz w:val=\"120\"/>", "<w:sz w:val=\"20\"/>", QByteArray());
    QVERIFY2(courier.box.right() > plain.box.right() + 16, qPrintable(QString::number(courier.box.right()) + " against " + QString::number(plain.box.right())));
}


// A level that says not bold has a number that is not, where the paragraph's mark is bold.
void WriteTests::aLevelThatSaysNotBoldHasNoBoldNumber()
{
    const NumberInk boldMark = numberInk("<w:sz w:val=\"60\"/>", "<w:b/>", QByteArray()), lightLevel = numberInk("<w:b w:val=\"0\"/><w:sz w:val=\"60\"/>", "<w:b/>", QByteArray());
    QVERIFY2(boldMark.dark > lightLevel.dark * 11 / 10, qPrintable(QString::number(boldMark.dark) + " against " + QString::number(lightLevel.dark)));
}


// A color of the level is its number's.
void WriteTests::aNumberIsInTheColorOfItsLevel()
{
    QCOMPARE(numberInk("<w:sz w:val=\"60\"/>", "<w:sz w:val=\"20\"/>", QByteArray()).red, 0);
    const NumberInk red = numberInk("<w:color w:val=\"FF0000\"/><w:sz w:val=\"60\"/>", "<w:sz w:val=\"20\"/>", QByteArray());
    QVERIFY2(red.red > 20, qPrintable(QString::number(red.red)));
    // A paragraph's mark is of a color: its number is too, unless the level says automatic.
    const QByteArray redMark = "<w:color w:val=\"FF0000\"/><w:sz w:val=\"20\"/>";
    QVERIFY(numberInk("<w:sz w:val=\"60\"/>", redMark, QByteArray()).red > 20);
    QCOMPARE(numberInk("<w:color w:val=\"auto\"/><w:sz w:val=\"60\"/>", redMark, QByteArray()).red, 0);
}


// A bullet whose level asks for the outline effect (w:outline) is drawn hollow, as an outlined box is: the middle of a big black square is dark, and that of the same
// square outlined is not.
void WriteTests::theCheckBoxOfAnOutlinedBulletIsHollow()
{
    auto middleOf = [&](const QByteArray &effect) {
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"\xE2\x96\xA0\"/>"
                                     "<w:pPr><w:ind w:left=\"1440\" w:hanging=\"720\"/></w:pPr><w:rPr><w:rFonts w:ascii=\"Arial\" w:hAnsi=\"Arial\"/>" + effect + "<w:sz w:val=\"96\"/></w:rPr></w:lvl></w:abstractNum>"
                                     "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
        const QByteArray body = "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>Item</w:t></w:r></w:p>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(body, sect, {}, QByteArray(), kStyles, numbering));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        const QImage img = view.pageImage(0, 816);
        // The marker's dark pixels: left of the text, which begins at 72 + 72 points from the page's edge.
        const double perPoint = 816 / 612.0;
        QRect ink;
        for (int y = int(60 * perPoint); y < int(300 * perPoint); ++y)
            for (int x = int(72 * perPoint); x < int(140 * perPoint); ++x)
                if (qGray(img.pixel(x, y)) < 215) ink = ink.isNull() ? QRect(x, y, 1, 1) : ink.united(QRect(x, y, 1, 1));   // (a hairline is a pale gray)
        if (ink.width() < 8) return -1;
        return qGray(img.pixel(ink.center()));
    };
    const int solid = middleOf(QByteArray()), hollow = middleOf("<w:outline/>"), notOutlined = middleOf("<w:outline w:val=\"0\"/>");
    QVERIFY2(solid >= 0 && solid < 100, qPrintable(QString::number(solid)));
    QVERIFY2(notOutlined >= 0 && notOutlined < 100, qPrintable(QString::number(notOutlined)));
    QVERIFY2(hollow > 235, qPrintable(QString::number(hollow)));
}


// The notes of a page are laid out the same way when the document has a table anywhere (the page is then laid out beside its tables): a separator thirty points tall and a notice
// twenty tall, a note in a cell, and a section break after which the next section's first paragraph has a note of its own.
void WriteTests::theNotesOfAPageAreLaidOutTheSameWithATableInTheDocument()
{
    const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
    const QByteArray margins = "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";
    auto text = [](const char *t) { return QByteArray("<w:r><w:t xml:space=\"preserve\">") + t + "</w:t></w:r>"; };
    auto para = [&](const QByteArray &runs, const QByteArray &pPrExtra = QByteArray()) { return "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/>" + pPrExtra + "</w:pPr>" + runs + "</w:p>"; };
    auto table = [&](const QByteArray &cell) {
        return "<w:tbl><w:tblPr><w:tblW w:w=\"3000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/></w:tblGrid><w:tr><w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr>" + cell + "</w:tc></w:tr></w:tbl>";
    };
    auto note = [&](int id, const char *t) { return "<w:footnote w:id=\"" + QByteArray::number(id) + "\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r>" + text(t) + "</w:p></w:footnote>"; };
    auto notesPart = [&](const QByteArray &separator, const QByteArray &notice, const QByteArray &notes) {
        return "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\">" + separator + "</w:footnote>" + (notice.isEmpty() ? QByteArray() : "<w:footnote w:type=\"continuationNotice\" w:id=\"1\">" + notice + "</w:footnote>") + notes + "</w:footnotes>";
    };
    const QByteArray separator30 = "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"600\" w:lineRule=\"exact\"/></w:pPr><w:r><w:separator/></w:r></w:p>";
    const QByteArray notice20 = "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"400\" w:lineRule=\"exact\"/></w:pPr><w:r><w:t>(continued)</w:t></w:r></w:p>";
    const QByteArray sect = "<w:sectPr>" + margins + "</w:sectPr>";
    auto ruleOf = [](const flow::MaterializedPage &page) {
        double rule = -1;
        for (const auto &item : page.doc->pages[0]->items)
            if (const auto *line = dynamic_cast<const LineItem *>(item.get())) rule = line->p1.y();
        return rule;
    };
    {
        // A note in the text and one in a cell: the last ends the notice's height above the foot, the rule is 12 above the first (the separator's 30 less its 18).
        const QByteArray body = para(text("Cited") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>") + table(para(text("In the cell") + "<w:r><w:footnoteReference w:id=\"3\"/></w:r>")) + para(text("After"));
        auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notesPart(separator30, notice20, note(2, " ONE note") + note(3, " TWO note"))}}));
        QVERIFY(doc);
        const auto page = doc->pager().materialize(0);
        const TextItem *one = boxWithText(*page, QStringLiteral("ONE note")), *two = boxWithText(*page, QStringLiteral("TWO note"));
        QVERIFY(one && two);
        QVERIFY2(std::abs(two->rect.bottom() - (792 - 72 - 20)) < 0.1, qPrintable(QString::number(two->rect.bottom())));
        QVERIFY2(std::abs(one->rect.bottom() - two->rect.top()) < 0.1, qPrintable(QString::number(one->rect.bottom())));
        QVERIFY2(std::abs(one->rect.top() - ruleOf(*page) - 12) < 0.05, qPrintable(QString::number(one->rect.top() - ruleOf(*page))));
    }
    {
        // A table opens the first section, and the first paragraph of the next section has a note and is followed by one: the page before has its notes at the foot, all the same.
        const QByteArray body = table(para(text("Top cell"))) + para(text("One") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r>", "<w:sectPr>" + margins + "</w:sectPr>") +
                                para(text("Two") + "<w:r><w:footnoteReference w:id=\"3\"/></w:r>") + table(para(text("in a cell"))) + para(text("End"));
        auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notesPart(separator30, QByteArray(), note(2, " FIRST note") + note(3, " SECOND note"))}}));
        QVERIFY(doc);
        QCOMPARE(doc->pager().pageCount(), 2);
        // (The page owns its items: it is kept while the box is read, not made and dropped in the call that finds the box.)
        const auto firstPage = doc->pager().materialize(0);
        QVERIFY(firstPage);
        const TextItem *first = boxWithText(*firstPage, QStringLiteral("FIRST note"));
        QVERIFY(first);
        QVERIFY2(std::abs(first->rect.bottom() - (792 - 72)) < 0.1, qPrintable(QString::number(first->rect.bottom())));
    }
}


// A reference whose mark follows it takes no number among the notes of a page with a table either: a custom one, an automatic note, one in a cell, and one after the table are
// numbered 1, 2, and 3.
void WriteTests::aCustomMarkTakesNoNumberAmongTheNotesOfATablesPage()
{
    auto text = [](const char *t) { return QByteArray("<w:r><w:t xml:space=\"preserve\">") + t + "</w:t></w:r>"; };
    auto ref = [](int id) { return "<w:r><w:footnoteReference w:id=\"" + QByteArray::number(id) + "\"/></w:r>"; };
    const QByteArray custom = "<w:r><w:footnoteReference w:customMarkFollows=\"1\" w:id=\"2\"/><w:t>*</w:t></w:r>";
    const QByteArray cell = "<w:p>" + text("In the cell") + ref(4) + "</w:p>";
    const QByteArray body = "<w:p>" + text("First") + custom + text(" second") + ref(3) + "</w:p><w:tbl><w:tblPr><w:tblW w:w=\"3000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/></w:tblGrid>"
                            "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr>" + cell + "</w:tc></w:tr></w:tbl><w:p>" + text("After") + ref(5) + "</w:p>";
    auto note = [&](int id, const char *t) { return "<w:footnote w:id=\"" + QByteArray::number(id) + "\"><w:p><w:r><w:footnoteRef/></w:r>" + text(t) + "</w:p></w:footnote>"; };
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"2\"><w:p>" + text("* custom note") + "</w:p></w:footnote>" + note(3, " AUTOA note") + note(4, " AUTOB note") + note(5, " AUTOC note") + "</w:footnotes>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    auto marks = [&](const QString &needle) {
        QStringList out;
        const TextItem *box = boxWithText(*page, needle);
        if (!box) return QStringList{QStringLiteral("no box")};
        for (QTextBlock b = page->doc->story(box->storyId)->doc->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QString code = it.fragment().charFormat().stringProperty(tp::Field);
                if (code.startsWith(QLatin1String("raw:"))) out << code;
            }
        return out;
    };
    QCOMPARE(marks(QStringLiteral("AUTOA note")), (QStringList{QStringLiteral("raw:1")}));
    QCOMPARE(marks(QStringLiteral("AUTOB note")), (QStringList{QStringLiteral("raw:2")}));
    QCOMPARE(marks(QStringLiteral("AUTOC note")), (QStringList{QStringLiteral("raw:3")}));
}


// Clearing the formatting leaves a note reference's custom mark as it was: the next automatic note is still the first, and the file still says the mark follows.
void WriteTests::clearingTheFormattingKeepsACustomNoteMark()
{
    auto text = [](const char *t) { return QByteArray("<w:r><w:t xml:space=\"preserve\">") + t + "</w:t></w:r>"; };
    const QByteArray body = "<w:p>" + text("First") + "<w:r><w:footnoteReference w:customMarkFollows=\"1\" w:id=\"2\"/><w:t>*</w:t></w:r>" + text(" second") + "<w:r><w:footnoteReference w:id=\"3\"/></w:r></w:p>";
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"2\"><w:p>" + text("* custom note") + "</w:p></w:footnote><w:footnote w:id=\"3\"><w:p><w:r><w:footnoteRef/></w:r>" + text(" AUTO note") + "</w:p></w:footnote></w:footnotes>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    c.select(QTextCursor::Document);
    clearFormatting(*doc, c);
    bool kept = false;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) kept |= it.fragment().charFormat().boolProperty(tp::NoteMarkCustom);
    QVERIFY(kept);
    const auto page = doc->pager().materialize(0);
    const TextItem *box = boxWithText(*page, QStringLiteral("AUTO note"));
    QVERIFY(box);
    QString mark;
    for (QTextBlock b = page->doc->story(box->storyId)->doc->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it)
            if (it.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("raw:"))) mark = it.fragment().charFormat().stringProperty(tp::Field);
    QCOMPARE(mark, QStringLiteral("raw:1"));
    QVERIFY(documentXml(doc->toDocx()).contains("customMarkFollows"));
}


// A level's outline effect is looked for once for the level, not once for each item that is in the list: two hundred items of one level look at its run properties once.
void WriteTests::aLevelsOutlineEffectIsLookedForOncePerLevel()
{
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"o\"/><w:rPr><w:outline/></w:rPr></w:lvl></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
    QByteArray body;
    for (int i = 0; i < 200; ++i) body += "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>item</w:t></w:r></w:p>";
    const int before = levelsScannedForOutlineForTest();
    auto doc = open(makeDocx(body, kStyles, numbering));
    QVERIFY(doc);
    QVERIFY(doc->story()->begin().textList()->format().boolProperty(tp::BulletOutline));
    QCOMPARE(levelsScannedForOutlineForTest() - before, 1);
}


// The separator is cut to a quarter of the text area of the page it is on, not of the smallest page of the file: a section with a body of forty points does not shorten it on the
// full-size pages (a rule twelve above the note, the separator's thirty less its eighteen); on its own pages it is ten, and the rule four above the note.
void WriteTests::theNoteSeparatorIsCutPerSection()
{
    const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
    auto text = [](const char *t) { return QByteArray("<w:r><w:t xml:space=\"preserve\">") + t + "</w:t></w:r>"; };
    auto note = [&](int id, const char *t) { return "<w:footnote w:id=\"" + QByteArray::number(id) + "\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r>" + text(t) + "</w:p></w:footnote>"; };
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"600\" w:lineRule=\"exact\"/></w:pPr>"
                             "<w:r><w:separator/></w:r></w:p></w:footnote>" + note(2, " BIG note") + note(3, " SMALL note") + "</w:footnotes>";
    const QByteArray full = "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";
    const QByteArray small = "<w:pgSz w:w=\"12240\" w:h=\"2240\"/><w:pgMar w:top=\"720\" w:right=\"1440\" w:bottom=\"720\" w:left=\"1440\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/>";
    const QByteArray body = "<w:p>" + exact.left(exact.size() - 8) + "<w:sectPr>" + full + "</w:sectPr></w:pPr>" + text("Big") + "<w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>"
                            "<w:p>" + exact + text("Small") + "<w:r><w:footnoteReference w:id=\"3\"/></w:r></w:p>";
    auto doc = open(makeDocxWithParts(body, "<w:sectPr>" + small + "</w:sectPr>", {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    QCOMPARE(doc->pager().pageCount(), 2);
    auto gap = [&](int index, const QString &needle) {
        const auto page = doc->pager().materialize(index);
        const TextItem *n = boxWithText(*page, needle);
        double rule = -1;
        for (const auto &item : page->doc->pages[0]->items)
            if (const auto *line = dynamic_cast<const LineItem *>(item.get())) rule = line->p1.y();
        return n && rule > 0 ? n->rect.top() - rule : -1.0;
    };
    QVERIFY2(std::abs(gap(0, QStringLiteral("BIG note")) - 12) < 0.05, qPrintable(QString::number(gap(0, QStringLiteral("BIG note")))));
    QVERIFY2(std::abs(gap(1, QStringLiteral("SMALL note")) - 4) < 0.05, qPrintable(QString::number(gap(1, QStringLiteral("SMALL note")))));
}


// A note of a reference with a custom mark has no number to show for the reference mark its text begins with: it shows nothing (not a 0).
void WriteTests::aCustomNotesOwnReferenceMarkShowsNothing()
{
    auto text = [](const char *t) { return QByteArray("<w:r><w:t xml:space=\"preserve\">") + t + "</w:t></w:r>"; };
    const QByteArray body = "<w:p>" + text("Cited") + "<w:r><w:footnoteReference w:customMarkFollows=\"1\" w:id=\"2\"/><w:t>*</w:t></w:r></w:p>";
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r>" + text(" BODY text") + "</w:p></w:footnote></w:footnotes>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    const TextItem *box = boxWithText(*page, QStringLiteral("BODY text"));
    QVERIFY(box);
    QStringList marks;
    for (QTextBlock b = page->doc->story(box->storyId)->doc->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it)
            if (it.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("raw:"))) marks << it.fragment().charFormat().stringProperty(tp::Field);
    QCOMPARE(marks, (QStringList{QStringLiteral("raw:")}));
}


// An endnote reference whose mark follows it as text shows no number and takes none, as a footnote's does.
void WriteTests::anEndnoteWithACustomMarkTakesNoNumber()
{
    auto text = [](const char *t) { return QByteArray("<w:r><w:t xml:space=\"preserve\">") + t + "</w:t></w:r>"; };
    const QByteArray body = "<w:p>" + text("First") + "<w:r><w:endnoteReference w:customMarkFollows=\"1\" w:id=\"2\"/><w:t>*</w:t></w:r>" + text(" second") + "<w:r><w:endnoteReference w:id=\"3\"/></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    const TextItem *box = boxWithText(*page, QStringLiteral("second"));
    QVERIFY(box);
    QStringList marks;
    for (QTextBlock b = page->doc->story(box->storyId)->doc->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it)
            if (it.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("raw:"))) marks << it.fragment().charFormat().stringProperty(tp::Field);
    QCOMPARE(marks, (QStringList{QStringLiteral("raw:"), QStringLiteral("raw:i")}));
}


// Items of different lists are told apart by their numbering id whether or not the level they name is in the definition (an item moved to a level the list lacks).
void WriteTests::listsAreToldApartWhateverTheLevel()
{
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"o\"/></w:lvl></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num><w:num w:numId=\"2\"><w:abstractNumId w:val=\"0\"/></w:num>";
    auto doc = open(makeDocx("<w:p><w:r><w:t>x</w:t></w:r></w:p>", kStyles, numbering));
    QVERIFY(doc);
    QCOMPARE(doc->listFormat(1, 0).intProperty(tp::ListKey), 1);
    QCOMPARE(doc->listFormat(2, 0).intProperty(tp::ListKey), 2);
    QCOMPARE(doc->listFormat(1, 7).intProperty(tp::ListKey), 1);   // (a level the definition lacks)
    QCOMPARE(doc->listFormat(2, 7).intProperty(tp::ListKey), 2);
}


// A numbered level's outline effect draws its numbers hollow too.
void WriteTests::theOutlineEffectReachesNumbers()
{
    auto doc = open(makeDocx("<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>item</w:t></w:r></w:p>", kStyles,
                             "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/><w:rPr><w:outline/></w:rPr></w:lvl></w:abstractNum>"
                             "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>"));
    QVERIFY(doc);
    QVERIFY(doc->story()->begin().textList()->format().boolProperty(tp::BulletOutline));
}


// A bullet drawn to the box the reference pages measured (Wingdings' heavy arrow) is that box outlined when its level has the outline effect, not the font's own glyph at its own
// size: solid and outlined, it has the same box to a pixel or two.
void WriteTests::anOutlinedBulletOfAMeasuredFontIsItsBoxOutlined()
{
    auto boxOf = [&](const QByteArray &effect) {
        const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"\xEF\x83\xA8\"/>"
                                     "<w:pPr><w:ind w:left=\"1440\" w:hanging=\"720\"/></w:pPr><w:rPr><w:rFonts w:ascii=\"Wingdings\" w:hAnsi=\"Wingdings\"/>" + effect + "<w:sz w:val=\"96\"/></w:rPr></w:lvl></w:abstractNum>"
                                     "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
        const QByteArray body = "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>Item</w:t></w:r></w:p>";
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto d = open(makeDocxWithParts(body, sect, {}, QByteArray(), kStyles, numbering));
        PageView view;
        view.resize(1000, 700);
        view.setDocument(d.get());
        const QImage img = view.pageImage(0, 816);
        const double perPoint = 816 / 612.0;
        QRect ink;
        for (int y = int(60 * perPoint); y < int(300 * perPoint); ++y)
            for (int x = int(60 * perPoint); x < int(140 * perPoint); ++x)
                if (qGray(img.pixel(x, y)) < 215) ink = ink.isNull() ? QRect(x, y, 1, 1) : ink.united(QRect(x, y, 1, 1));
        return ink;
    };
    const QRect solid = boxOf(QByteArray()), hollow = boxOf("<w:outline/>");
    QVERIFY(solid.width() > 8 && hollow.width() > 8);
    QVERIFY2(std::abs(solid.left() - hollow.left()) <= 2 && std::abs(solid.top() - hollow.top()) <= 2 && std::abs(solid.width() - hollow.width()) <= 2 && std::abs(solid.height() - hollow.height()) <= 2,
             qPrintable(QStringLiteral("%1,%2 %3x%4 against %5,%6 %7x%8").arg(solid.left()).arg(solid.top()).arg(solid.width()).arg(solid.height()).arg(hollow.left()).arg(hollow.top()).arg(hollow.width()).arg(hollow.height())));
}


// A section break in the middle of a long document lays out the pages around it, not those before it again; and Enter in a document of twenty
// thousand sections makes no more than the sections it changed (counted, not timed).
void WriteTests::editingSectionsTouchesOnlyWhatChanged()
{
    auto doc = open(makeDocx(paragraphs(3000, 60), kStyles));
    const int pages = doc->pager().pageCount();
    QVERIFY(pages > 200);
    QTextCursor c(doc->story());
    c.setPosition(doc->story()->findBlockByNumber(1500).position() + 5);
    const qint64 laid = doc->pager().pagesLaidOut();
    insertSectionBreak(*doc, c);
    doc->pager().paginate(doc->pager().pageOfPosition(c.position()) + 2);
    QVERIFY2(doc->pager().pagesLaidOut() - laid < 12, qPrintable(QString::number(doc->pager().pagesLaidOut() - laid)));
    QCOMPARE(doc->sectionCount(), 2);
    QVERIFY(doc->pager().pageCount() >= pages);   // (and the rest settles, one page more for the section's own)
    // Twenty thousand sections.
    QByteArray body;
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
    for (int i = 0; i < 20000; ++i) body += "<w:p><w:pPr>" + sect + "</w:pPr><w:r><w:t>section " + QByteArray::number(i) + "</w:t></w:r></w:p>";
    auto many = open(makeDocx(body, kStyles));
    QCOMPARE(many->sectionCount(), 20001);
    const qint64 made = many->sectionsMadeForTest();
    QTextCursor m(many->story());
    m.setPosition(many->story()->findBlockByNumber(10000).position() + 5);
    insertParagraph(*many, m);
    QVERIFY2(many->sectionsMadeForTest() - made <= 3, qPrintable(QString::number(many->sectionsMadeForTest() - made)));
    QCOMPARE(many->sectionCount(), 20001);   // (the break stays with the second half: still 20,000 breaks and the last)
    insertText(*many, m, QStringLiteral("x"));
    QVERIFY(many->sectionsMadeForTest() - made <= 3);
    // A page setup change of one section makes one.
    wml::SectionProps p = many->sectionProps(5000);
    p.pageW = 15840;
    p.pageH = 12240;
    const qint64 before = many->sectionsMadeForTest();
    many->setSectionProps(5000, p);
    QVERIFY(many->sectionsMadeForTest() - before <= 2);
    QCOMPARE(many->sectionProps(5000).pageW, 15840);
    QCOMPARE(many->sectionProps(4999).pageW, 12240);
}


void WriteTests::aSectionThatEndsTheFileLeavesABlankLastPageThatIsDrawn()
{
    // The only paragraph ends a next-page section and nothing follows: the second section has no text, so its page has no lines.
    const QByteArray one = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocx("<w:p><w:pPr>" + one + "</w:pPr><w:r><w:t>the only words</w:t></w:r></w:p>", kStyles, QByteArray(), one));
    doc->pager().complete();
    QCOMPARE(doc->pager().pageCount(), 2);
    PageView view;
    view.resize(1000, 900);
    view.setDocument(doc.get());
    view.show();
    view.setZoomMode(PageView::ZoomMode::ManyPages, 2, 1);
    const QImage img = view.grab().toImage();
    const QRect blank = view.pageRect(1);
    QVERIFY(blank.width() > 100 && blank.right() < img.width());
    int ink = 0, white = 0;
    for (int y = blank.top() + 4; y < std::min(blank.bottom() - 4, img.height()); ++y)   // (inside the sheet, clear of its shadow)
        for (int x = blank.left() + 4; x < blank.right() - 4; ++x) (img.pixelColor(x, y).lightness() > 250 ? white : ink) += 1;
    QVERIFY2(ink == 0 && white > 0, "the last page is a blank sheet");
    // The caret ends the text, on the first page, and the keys that cross pages leave it there.
    QTextCursor c(doc->story());
    c.movePosition(QTextCursor::End);
    view.setCursor(c);
    QCOMPARE(view.currentPage(), 0);
    view.typeKey(Qt::Key_PageDown, Qt::NoModifier);
    view.typeKey(Qt::Key_Down, Qt::NoModifier);
    QVERIFY(view.cursor().position() <= doc->story()->characterCount() - 1);
    // Saving keeps the two sections.
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:sectPr")), 2);
}


void WriteTests::sectionsHaveTheirOwnPages()
{
    auto doc = WriteDoc::blank();
    QTextCursor c(doc->story());
    typeText(*doc, c, QStringLiteral("first section"));
    insertSectionBreak(*doc, c);
    typeText(*doc, c, QStringLiteral("second section"));
    QCOMPARE(doc->sectionCount(), 2);
    QCOMPARE(doc->pager().pageCount(), 2);
    // The second is landscape with two columns (as the Layout tab sets them), and it survives the file.
    wml::SectionProps s = doc->sectionProps(1);
    s.pageW = 15840;
    s.pageH = 12240;
    s.orient = QStringLiteral("landscape");
    s.cols = 2;
    doc->setSectionProps(1, s);
    QCOMPARE(doc->pager().pageCount(), 2);
    QCOMPARE(doc->pager().page(1).size, QSizeF(792, 612));
    QCOMPARE(doc->pager().page(1).columns, 2);
    QCOMPARE(doc->pager().page(0).size, doc->pager().page(0).size);
    const QByteArray x = documentXml(doc->toDocx());
    QCOMPARE(int(x.count("<w:sectPr")), 2);
    QVERIFY(x.contains("w:orient=\"landscape\""));
    QVERIFY(x.contains("<w:cols w:num=\"2\""));
    auto back = open(doc->toDocx());
    QCOMPARE(back->sectionCount(), 2);
    QCOMPARE(back->pager().pageCount(), 2);
    QCOMPARE(back->pager().page(1).size, QSizeF(792, 612));
    // Deleting the break takes the two sections into one.
    QTextCursor d(back->story());
    d.setPosition(back->story()->findBlockByNumber(1).position());
    deleteBack(*back, d, false);
    QCOMPARE(back->sectionCount(), 1);
    QCOMPARE(back->pager().pageCount(), 1);
    // And undo brings it back.
    back->story()->undo(&d);
    QCOMPARE(back->sectionCount(), 2);
    QCOMPARE(back->pager().pageCount(), 2);
}


void WriteTests::aSectionBreakInTheMiddleOfTextAlsoUndoes()
{
    auto doc = WriteDoc::blank();
    QTextCursor c(doc->story());
    typeText(*doc, c, QStringLiteral("a\nb\nc"));
    c.setPosition(doc->story()->findBlockByNumber(1).position());
    insertSectionBreak(*doc, c);
    QCOMPARE(doc->sectionCount(), 2);
    doc->story()->undo(&c);
    QCOMPARE(doc->sectionCount(), 1);
    doc->story()->redo(&c);
    QCOMPARE(doc->sectionCount(), 2);
}


void WriteTests::copyAndPasteKeepTheFormats()
{
    auto doc = open(makeDocx("<w:p><w:r><w:rPr><w:b/><w:color w:val=\"FF0000\"/></w:rPr><w:t>Bold red</w:t></w:r><w:r><w:t xml:space=\"preserve\"> and plain</w:t></w:r></w:p>", kStyles));
    PageView view;
    view.setDocument(doc.get());
    view.selectAll();
    view.copy();
    view.cursor().movePosition(QTextCursor::End);
    view.insertText(QStringLiteral(" | "));
    view.paste();
    QCOMPARE(doc->story()->toPlainText(), QStringLiteral("Bold red and plain | Bold red and plain"));
    QTextCursor t(doc->story());
    t.setPosition(26 + 2);
    QCOMPARE(t.charFormat().fontWeight(), int(QFont::Bold));
    QCOMPARE(t.charFormat().foreground().color(), QColor(0xFF, 0, 0));
    // Cut takes the text out, and undo puts it back.
    view.cursor().setPosition(0);
    view.cursor().setPosition(8, QTextCursor::KeepAnchor);
    view.cut();
    QVERIFY(doc->story()->toPlainText().startsWith(QLatin1String(" and plain")));
    view.undo();
    QVERIFY(doc->story()->toPlainText().startsWith(QLatin1String("Bold red")));
    // Plain text pastes as paragraphs.
    QApplication::clipboard()->setText(QStringLiteral("one\ntwo\r\nthree"));
    view.cursor().movePosition(QTextCursor::End);
    const int blocks = doc->story()->blockCount();
    view.paste();
    QCOMPARE(doc->story()->blockCount(), blocks + 2);
}


void WriteTests::piecesKeptAsXmlStayWhenTheTextAroundThemChanges()
{
    auto doc = open(makeDocx("<w:p><w:r><w:t>ab</w:t></w:r><w:bookmarkStart w:id=\"3\" w:name=\"here\"/><w:r><w:t>cd</w:t></w:r><w:bookmarkEnd w:id=\"3\"/><w:r><w:t>ef</w:t></w:r></w:p>", kStyles));
    QCOMPARE(doc->story()->toPlainText().size(), 8);   // (the bookmark's two ends are two characters)
    QTextCursor c(doc->story());
    // Right passes the bookmark on the way to the next letter; Left does the same the other way.
    c.setPosition(2);
    moveCursor(c, QTextCursor::NextCharacter, false);
    QCOMPARE(c.position(), 4);   // over the mark and the letter c
    moveCursor(c, QTextCursor::PreviousCharacter, false);
    QCOMPARE(c.position(), 3);
    moveCursor(c, QTextCursor::PreviousCharacter, false);
    QCOMPARE(c.position(), 1);   // over the mark and the letter b
    // Backspace after the bookmark's end deletes the letter before it, and the bookmark stays.
    c.setPosition(6);
    deleteBack(*doc, c, false);
    QByteArray x = documentXml(doc->toDocx());
    QVERIFY(x.contains("<w:bookmarkStart w:id=\"3\" w:name=\"here\"/>"));
    QVERIFY(x.contains("<w:bookmarkEnd w:id=\"3\"/>"));
    QVERIFY(x.contains(">c</w:t>"));
    QVERIFY(!x.contains(">cd</w:t>"));
    // Selecting across one end and deleting takes the pair out (the file never has half).
    QTextCursor all(doc->story());
    all.setPosition(0);
    all.setPosition(doc->story()->characterCount() - 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, all);
    x = documentXml(doc->toDocx());
    QVERIFY(!x.contains("bookmark"));
}


void WriteTests::theStyleOfAParagraphIsWhatTheFileSaysAndNothingMore()
{
    // The paragraph has the style's look, and its direct formatting is only what it adds: nothing is written twice.
    const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Plain heading</w:t></w:r></w:p>"
                            "<w:p><w:r><w:rPr><w:rStyle w:val=\"Strong\"/></w:rPr><w:t>strong</w:t></w:r><w:r><w:rPr><w:i/></w:rPr><w:t xml:space=\"preserve\"> italic</w:t></w:r></w:p>"
                            "<w:p><w:pPr><w:jc w:val=\"right\"/><w:spacing w:before=\"60\"/></w:pPr><w:r><w:t>right</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles));
    QCOMPARE(doc->story()->findBlockByNumber(0).blockFormat().stringProperty(tp::StyleName), QStringLiteral("Heading1"));
    QTextCursor t(doc->story());
    t.setPosition(3);
    QCOMPARE(t.charFormat().fontWeight(), int(QFont::Bold));   // (from the style)
    QCOMPARE(t.charFormat().fontPointSize(), 17.0);
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY2(x.contains("<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t xml:space=\"preserve\">Plain heading</w:t></w:r></w:p>"), x.constData());
    QVERIFY2(x.contains("<w:rPr><w:rStyle w:val=\"Strong\"/></w:rPr><w:t xml:space=\"preserve\">strong</w:t>"), x.constData());
    QVERIFY2(x.contains("<w:rPr><w:i/></w:rPr><w:t xml:space=\"preserve\"> italic</w:t>"), x.constData());
    QVERIFY2(x.contains("<w:pPr><w:spacing w:before=\"60\" w:after=\"120\" w:line=\"264\" w:lineRule=\"auto\"/><w:jc w:val=\"right\"/></w:pPr>") ||
                 x.contains("<w:pPr><w:spacing w:before=\"60\"/><w:jc w:val=\"right\"/></w:pPr>"), x.constData());
    // It comes out of a second round the same.
    QCOMPARE(documentXml(open(doc->toDocx())->toDocx()), x);
}


// ---------------------------------------------------------------- .docx
void WriteTests::aDocumentOpensAndPaginates()
{
    auto doc = open(makeDocx(paragraphs(300), kStyles, QByteArray(), "<w:sectPr><w:pgSz w:w=\"11906\" w:h=\"16838\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>"));
    QVERIFY(doc);
    QCOMPARE(doc->story()->blockCount(), 300);
    QVERIFY(doc->story()->toPlainText().startsWith(QLatin1String("The ")) || doc->story()->toPlainText().size() > 1000);
    QCOMPARE(doc->pager().page(0).size, QSizeF(595.3, 841.9));   // A4, as the file says
    const int pages = doc->pager().pageCount();
    QVERIFY2(pages > 8 && pages < 40, qPrintable(QString::number(pages)));
    const auto page = doc->pager().materialize(2);
    QVERIFY(page);
    QCOMPARE(page->number, 3);
}


void WriteTests::listsInTheFileAreLists()
{
    const QByteArray numbering =
        "<w:abstractNum w:abstractNumId=\"0\"><w:multiLevelType w:val=\"hybridMultilevel\"/>"
        "<w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"&#xF0B7;\"/><w:lvlJc w:val=\"left\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr><w:rPr><w:rFonts w:ascii=\"Symbol\" w:hAnsi=\"Symbol\"/></w:rPr></w:lvl></w:abstractNum>"
        "<w:abstractNum w:abstractNumId=\"1\"><w:multiLevelType w:val=\"multilevel\"/>"
        "<w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/><w:pPr><w:ind w:left=\"432\" w:hanging=\"432\"/></w:pPr></w:lvl>"
        "<w:lvl w:ilvl=\"1\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.%2\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"576\"/></w:pPr></w:lvl></w:abstractNum>"
        "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num><w:num w:numId=\"2\"><w:abstractNumId w:val=\"1\"/></w:num>";
    auto item = [](int num, int lvl, const char *t) { return QByteArray("<w:p><w:pPr><w:numPr><w:ilvl w:val=\"") + QByteArray::number(lvl) + "\"/><w:numId w:val=\"" + QByteArray::number(num) + "\"/></w:numPr></w:pPr><w:r><w:t>" + t + "</w:t></w:r></w:p>"; };
    auto doc = open(makeDocx(item(1, 0, "bullet") + item(2, 0, "one") + item(2, 1, "one point one") + item(2, 1, "one point two") + item(2, 0, "two") + item(2, 1, "two point one") + "<w:p><w:r><w:t>plain</w:t></w:r></w:p>" + item(2, 0, "three"), kStyles, numbering));
    QTextDocument *s = doc->story();
    auto marker = [&](int n) {
        const QTextBlock b = s->findBlockByNumber(n);
        return b.blockFormat().stringProperty(tp::BulletChar).isEmpty() ? (b.textList() ? b.textList()->itemText(b) : QString()) : b.blockFormat().stringProperty(tp::BulletChar);
    };
    QVERIFY(s->findBlockByNumber(0).textList());
    QCOMPARE(s->findBlockByNumber(0).blockFormat().leftMargin(), 36.0);      // the level's indents
    QCOMPARE(s->findBlockByNumber(0).blockFormat().textIndent(), -18.0);
    QCOMPARE(marker(1), QStringLiteral("1."));
    QCOMPARE(marker(2), QStringLiteral("1.1"));   // several levels in the marker: made as the file says
    QCOMPARE(marker(3), QStringLiteral("1.2"));
    QCOMPARE(marker(4), QStringLiteral("2."));
    QCOMPARE(marker(5), QStringLiteral("2.1"));   // (restarted under the new second item)
    QVERIFY(!s->findBlockByNumber(6).textList());
    QCOMPARE(marker(7), QStringLiteral("3."));    // (the list goes on after the plain paragraph)
    // The list comes back out as it was.
    const QByteArray x = documentXml(doc->toDocx());
    QCOMPARE(int(x.count("<w:numId w:val=\"2\"/>")), 6);
    QCOMPARE(int(x.count("<w:numId w:val=\"1\"/>")), 1);
    QVERIFY(x.contains("<w:ilvl w:val=\"1\"/>"));
}


void WriteTests::whatIsNotUnderstoodIsKeptWhereItWas()
{
    const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>cell</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
    const QByteArray body = "<w:p><w:r><w:t>before</w:t></w:r></w:p>" + table +
                            "<w:p><w:r><w:t>after </w:t></w:r><w:r><w:drawing><wp:inline xmlns:wp=\"urn:wp\"><wp:extent cx=\"1\" cy=\"1\"/></wp:inline></w:drawing></w:r>"
                            "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>7</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r></w:p>"
                            "<w:sdt><w:sdtPr><w:id w:val=\"9\"/></w:sdtPr><w:sdtContent><w:p><w:r><w:t>in a control</w:t></w:r></w:p></w:sdtContent></w:sdt>";
    auto doc = open(makeDocx(body, kStyles));
    // The table is its cell's paragraph in the story; the picture, the field, and the control have nothing that shows beyond their text.
    QTextDocument *s = doc->story();
    QVERIFY(!isLocked(s->findBlockByNumber(1)));
    QCOMPARE(s->findBlockByNumber(1).text(), QStringLiteral("cell"));
    QVERIFY(s->findBlockByNumber(1).blockFormat().intProperty(flow::fp::Cell) > 0);
    QVERIFY(s->toPlainText().contains(QLatin1String("in a control")));
    const QByteArray x = documentXml(doc->toDocx());
    QVERIFY(x.contains(table));   // (a table that was not changed is written as it was)
    QVERIFY(x.contains("<wp:inline xmlns:wp=\"urn:wp\">"));
    QVERIFY(x.contains("<w:instrText> PAGE </w:instrText>"));
    QVERIFY(x.contains("<w:sdt><w:sdtPr><w:id w:val=\"9\"/></w:sdtPr><w:sdtContent>"));
    QVERIFY(x.contains("</w:sdtContent></w:sdt>"));
    // Editing around them keeps them; typing in the cell changes the table's text and keeps the rest of it, and Enter in the cell makes another paragraph in it.
    QTextCursor c(s);
    c.setPosition(s->findBlockByNumber(1).position());
    insertText(*doc, c, QStringLiteral("zzz"));
    QCOMPARE(s->findBlockByNumber(1).text(), QStringLiteral("zzzcell"));
    insertParagraph(*doc, c);
    QCOMPARE(s->blockCount(), 5);
    QCOMPARE(s->findBlockByNumber(2).blockFormat().intProperty(flow::fp::Cell), s->findBlockByNumber(1).blockFormat().intProperty(flow::fp::Cell));
    const QByteArray y = documentXml(doc->toDocx());
    QVERIFY(!y.contains(table));
    QCOMPARE(int(y.count("<w:tbl>")), 1);
    QVERIFY(y.contains("<w:tblW w:w=\"0\" w:type=\"auto\"/>"));
    QVERIFY(y.contains("<w:gridCol w:w=\"2000\"/>"));
    QVERIFY(y.contains("zzz</w:t></w:r></w:p><w:p><w:r><w:t xml:space=\"preserve\">cell</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"));
}
