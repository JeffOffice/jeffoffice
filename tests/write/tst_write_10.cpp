#include "tst_write.h"


// A group of drawn objects that is not set at a place is in the line, as tall and as wide as it says: an empty object holds its room, and the shapes of a picture type that
// name no picture have neither line nor fill, while a picture whose shape says it has a line is drawn with it.
void WriteTests::aGroupInTheLineHoldsItsRoomAndAPictureTypeHasNoLine()
{
    const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
                          "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";
    using jp::office::wml::Shape;
    const QVector<Shape> shapes = jp::office::wml::readShapes("<w:pict " + ns + "><v:group id='g' style='width:468pt;height:235pt' coordorigin='0,0' coordsize='7200,3615'>"
        "<v:shape id='bg' type='#_x0000_t75' style='position:absolute;left:0;top:0;width:7200;height:3615'/>"
        "<v:shape id='pic' type='#_x0000_t75' style='position:absolute;left:0;top:0;width:3600;height:2702' stroked='t'><v:imagedata r:id='rId1'/></v:shape></v:group></w:pict>");
    QCOMPARE(int(shapes.size()), 3);
    QVERIFY(shapes[0].room && !shapes[0].floating);
    QCOMPARE(shapes[0].width, 468.0);
    QCOMPARE(shapes[0].height, 235.0);
    QVERIFY(!shapes[1].room && !shapes[1].stroked && !shapes[1].filled);
    QCOMPARE(shapes[2].kind, Shape::Picture);
    QVERIFY(shapes[2].borderWeight > 0 && shapes[2].borderSides[0] > 0 && shapes[2].borderSides[3] > 0);
    const QVector<Shape> placed = jp::office::wml::readShapes("<w:pict " + ns + "><v:group id='g' style='position:absolute;width:468pt;height:235pt' coordorigin='0,0' coordsize='7200,3615'>"
        "<v:shape id='bg' type='#_x0000_t75' style='position:absolute;left:0;top:0;width:7200;height:3615'/></v:group></w:pict>");
    QCOMPARE(int(placed.size()), 1);
    QVERIFY(!placed[0].room);
}


// A group set at a place whose own wrap keeps the text off it has an empty object of its size, set where it is and with that wrap; one with no wrap has none.
void WriteTests::aGroupSetAtAPlaceKeepsTheTextOffItsRoom()
{
    const QByteArray ns = "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" "
                          "xmlns:w10=\"urn:schemas-microsoft-com:office:word\"";
    using jp::office::wml::Shape;
    auto group = [&](const QByteArray &wrap) {
        return jp::office::wml::readShapes("<w:pict " + ns + "><v:group id='g' style='position:absolute;margin-left:2pt;margin-top:3pt;width:468pt;height:200pt;z-index:5' coordorigin='0,0' coordsize='9360,4000'>"
            "<v:rect id='r' style='position:absolute;left:0;top:0;width:100;height:100' fillcolor='red'/>" + wrap + "</v:group></w:pict>");
    };
    QVector<Shape> shapes = group("<w10:wrap type='topAndBottom'/>");
    QCOMPARE(int(shapes.size()), 2);
    QVERIFY(shapes[0].floating && shapes[0].wrap == Shape::TopBottom && !shapes[0].filled && !shapes[0].stroked);
    QCOMPARE(shapes[0].width, 468.0);
    QCOMPARE(shapes[0].height, 200.0);
    QCOMPARE(shapes[0].left, 2.0);
    QCOMPARE(shapes[0].top, 3.0);
    QVERIFY(shapes[1].wrap == Shape::WrapNone);
    QCOMPARE(int(group(QByteArray()).size()), 1);
}


// Columns that the file gives widths for (w:equalWidth off) have them: each column is as wide as it says, at its place after the ones before and the space after them.
void WriteTests::columnsOfUnequalWidthsAreLaidOutAsTheFileSaysThem()
{
    const QByteArray cols = "<w:cols w:num=\"2\" w:space=\"720\" w:equalWidth=\"0\"><w:col w:w=\"2400\" w:space=\"720\"/><w:col w:w=\"5520\"/></w:cols>";
    const QByteArray last = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1800\" w:bottom=\"1440\" w:left=\"1800\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>" + cols + "</w:sectPr>";
    const QByteArray text = wordsOf("Long", 60);
    auto doc = open(makeDocxWithParts("<w:p><w:r><w:br w:type=\"column\"/><w:t>" + text + "</w:t></w:r></w:p>", last, {}));
    QVERIFY(doc);
    const flow::Section &sec = doc->flow().sections[0];
    QCOMPARE(sec.columnRect(0).width(), 120.0);
    QCOMPARE(sec.columnRect(1).left(), 90.0 + 120.0 + 36.0);
    QCOMPARE(sec.columnRect(1).width(), 276.0);
    const auto lines = doc->pager().lines(0);
    double widest = 0;
    for (const auto &ln : lines)
        if (ln.frame == 1) widest = std::max(widest, ln.rect.width());
    QVERIFY2(widest > 150 && widest <= 276.01, qPrintable(QString::number(widest)));
}


// A paragraph that an object the text goes round is set from stays in its column as the columns of a section above a continuous one are made even: the page is not cut before it.
void WriteTests::aWallsParagraphStaysInItsColumnWhenColumnsAreMadeEven()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:cols w:num=\"2\" w:space=\"720\"/></w:sectPr>";
    const QByteArray logo = "<w:r><w:pict xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\">"
                            "<v:rect style=\"position:absolute;margin-left:0;margin-top:0;width:85pt;height:58pt;mso-position-horizontal-relative:text;mso-position-vertical-relative:text\" fillcolor=\"red\">"
                            "<w10:wrap type=\"topAndBottom\"/></v:rect></w:pict></w:r>";
    const QByteArray body = "<w:p/><w:p/><w:p>" + logo + "</w:p><w:p><w:pPr><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:cols w:num=\"2\" w:space=\"720\"/></w:sectPr></w:pPr></w:p>"
                            "<w:p><w:r><w:t>Marker</w:t></w:r></w:p>";
    auto doc = open(makeDocxWithParts(body, "<w:sectPr><w:type w:val=\"continuous\"/>" + sect.mid(10), {}));
    QVERIFY(doc);
    QVERIFY(doc->pager().pageCount() == 1);
    QVERIFY2(doc->pager().lines(0).size() >= 5, qPrintable(QString::number(doc->pager().lines(0).size())));
}


// Text in Courier New that exactly fills its line, 65 letters of 12 points in 468 points, does not fit it as the original's letters are a hair wider than their stand-in's: the
// last word goes on to the next line.
void WriteTests::aLineOfCourierNewThatExactlyFillsItsRoomBreaksBeforeItsLastWord()
{
    const QByteArray run = "<w:r><w:rPr><w:rFonts w:ascii=\"Courier New\" w:hAnsi=\"Courier New\"/><w:sz w:val=\"24\"/></w:rPr><w:t>Instead of calling an electrician\xe2\x80\x99s mate to replace a blown fuses an MS3 does it</w:t></w:r>";
    const QByteArray last = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts("<w:p>" + run + "</w:p>", last, {}));
    QVERIFY(doc);
    const auto lines = doc->pager().lines(0);
    QVERIFY(lines.size() >= 2);
    const QString text = doc->story()->toPlainText();
    const int firstEnd = lines[0].end - lines[0].pos;
    QVERIFY2(!text.left(lines[0].end).trimmed().endsWith(QLatin1String("fuses")), qPrintable(QString::number(firstEnd)));
}


// A footer's text after a tab whose stop is past the right edge stays on its line, off to the right, and does not wrap to more lines: the footer is as tall as one with no such tab.
void WriteTests::aFootersTextAfterATabPastTheRightEdgeDoesNotWrap()
{
    const QByteArray sect = "<w:sectPr><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"200\" w:gutter=\"0\"/></w:sectPr>";
    auto bodyHeight = [&](const QByteArray &runs) {
        const QByteArray footer = headerPart("ftr", {runs});
        auto doc = open(makeDocxWithParts(paragraphs(3), sect, {{QStringLiteral("/word/footer1.xml"), footer}}));
        return doc ? doc->pager().page(0).content.height() : -1.0;
    };
    QByteArray tabs;
    for (int i = 0; i < 16; ++i) tabs += "<w:r><w:tab/></w:r>";
    const QByteArray stops = "<w:pPr><w:tabs><w:tab w:val=\"center\" w:pos=\"4320\"/><w:tab w:val=\"right\" w:pos=\"8640\"/></w:tabs></w:pPr>";
    const double plain = bodyHeight(stops + "<w:r><w:t>Left</w:t></w:r>"), run = bodyHeight(stops + "<w:r><w:t>Left</w:t></w:r>" + tabs + "<w:r><w:t>May 2001</w:t></w:r>");
    QVERIFY(plain > 0);
    QCOMPARE(run, plain);
}


// ---- frames

QByteArray WriteTests::framedP(const QByteArray &text, const QByteArray &framePr)
{
    return "<w:p><w:pPr>" + framePr + "</w:pPr><w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r></w:p>";
}


// The paragraphs of a frame in the body (w:framePr) are one frame, set apart from the text: no lines and no room there, and a text box where the frame asks, at the paragraph
// after them, with their text; and the file comes back as it was.
void WriteTests::framePrParagraphsAreAFrameSetWhereItAsks()
{
    const QByteArray pr = "<w:framePr w:w=\"2400\" w:h=\"1000\" w:hRule=\"atLeast\" w:hSpace=\"180\" w:vSpace=\"60\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"margin\" w:xAlign=\"right\" w:y=\"200\"/>";
    const QByteArray body = plainP(wordsOf("Before", 30)) + framedP("FRAMEWORDS ONE", pr) + framedP("FRAMEWORDS TWO", pr) + plainP(wordsOf("After the frame", 60)) + plainP(wordsOf("Last", 40));
    auto doc = open(makeDocx(body, kStyles));
    QVERIFY(doc);
    const flow::FlowDocument &fd = doc->flow();
    QCOMPARE(int(fd.frames.size()), 1);
    const flow::FloatingObject &o = fd.objects[fd.frames[0] - 1];
    QCOMPARE(o.frame, 1);
    QCOMPARE(o.frameWidth, 120.0);
    QCOMPARE(o.frameHeight, 50.0);
    QVERIFY(!o.frameExactHeight);
    QVERIFY(o.hRel == flow::FloatingObject::Rel::Margin && o.vRel == flow::FloatingObject::Rel::Paragraph);
    QVERIFY(o.hAlign == flow::FloatingObject::Align::End);
    QCOMPARE(o.dy, 10.0);
    QVERIFY(o.wrap == flow::FloatingObject::Wrap::Square);
    QCOMPARE(o.gap.left(), 9.0);
    QCOMPARE(o.gap.top(), 3.0);
    int inFrame = 0;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
        if (b.blockFormat().intProperty(flow::fp::Frame) == 1) {
            ++inFrame;
            QVERIFY(b.blockFormat().boolProperty(tp::OutOfLayout));
        }
    QCOMPARE(inFrame, 2);
    // On the page: no lines of the frame's paragraphs, a text box at the right margin with their text, and the text of the paragraph after goes round it.
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    const TextItem *box = nullptr;
    for (const auto &it : page->doc->pages[0]->items)
        if (const auto *t = dynamic_cast<const TextItem *>(it.get()))
            if (const Story *st = page->doc->story(t->storyId))
                if (st->doc->toPlainText().contains(QStringLiteral("FRAMEWORDS ONE")) && st->doc->toPlainText().contains(QStringLiteral("FRAMEWORDS TWO")) && t->id.startsWith(QLatin1String("flow-object-"))) box = t;
    QVERIFY(box);
    const flow::Section &sec = fd.section(0);
    QVERIFY2(std::abs(box->rect.right() - (sec.pageSize().width() - sec.margins.right())) < 0.01 && std::abs(box->rect.width() - 120) < 0.01, qPrintable(audit::rectText(box->rect)));
    QVERIFY(box->rect.height() >= 50 - 0.01);
    QVERIFY2(audit::problems(doc->pager(), doc->story()).isEmpty(), qPrintable(audit::problems(doc->pager(), doc->story())));
    for (const auto &l : doc->pager().lines(0)) QVERIFY(doc->story()->findBlock(l.pos).blockFormat().intProperty(flow::fp::Frame) == 0);
    // The file comes back as it was: the frame's paragraphs, with their properties, where they were.
    const QByteArray saved = doc->toDocx();
    const QByteArray x = documentXml(saved);
    QVERIFY2(x.count("w:framePr") == 2 && x.contains("FRAMEWORDS ONE") && x.indexOf("FRAMEWORDS ONE") < x.indexOf("FRAMEWORDS TWO") && x.indexOf("FRAMEWORDS TWO") < x.indexOf("After the frame"), x.left(1500).constData());
    QVERIFY2(rt::strictProblems(saved).isEmpty(), qPrintable(rt::strictProblems(saved).join(QLatin1Char('\n'))));
    auto back = open(saved);
    QVERIFY(back);
    QCOMPARE(int(back->flow().frames.size()), 1);
    QCOMPARE(documentXml(back->toDocx()), x);
}


// The frame properties of a paragraph style are the frame of its paragraphs, with what a paragraph says itself over them, attribute by attribute: a paragraph that says only
// how the text goes round (and one that says nothing) is in the frame the style sets, and the paragraphs next to one another with the same properties are one.
void WriteTests::aParagraphStylesFrameIsItsParagraphsFrameWithTheirOwnOverIt()
{
    const QByteArray styles = kStyles + "<w:style w:type=\"paragraph\" w:styleId=\"Boxed\"><w:name w:val=\"Boxed\"/><w:basedOn w:val=\"Normal\"/><w:pPr>"
                              "<w:framePr w:w=\"3000\" w:h=\"1440\" w:wrap=\"around\" w:vAnchor=\"page\" w:hAnchor=\"margin\" w:xAlign=\"center\" w:y=\"889\"/></w:pPr></w:style>"
                              "<w:style w:type=\"paragraph\" w:styleId=\"BoxedToo\"><w:name w:val=\"Boxed too\"/><w:basedOn w:val=\"Boxed\"/><w:pPr><w:framePr w:w=\"2000\"/></w:pPr></w:style>";
    auto framed = [&](const QByteArray &style, const QByteArray &direct) {
        return "<w:p><w:pPr><w:pStyle w:val=\"" + style + "\"/>" + direct + "</w:pPr><w:r><w:t>words</w:t></w:r></w:p>";
    };
    auto frameOf = [&](const QByteArray &body, int *frames) {
        auto doc = open(makeDocx(plainP("Before") + body + plainP("After"), styles));
        *frames = doc ? int(doc->flow().frames.size()) : -1;
        return doc && doc->flow().frames.size() >= 1 ? doc->flow().objects[doc->flow().frames[0] - 1] : flow::FloatingObject();
    };
    using FO = flow::FloatingObject;
    int frames = 0;
    // Said only by the style.
    FO o = frameOf(framed("Boxed", QByteArray()), &frames);
    QCOMPARE(frames, 1);
    QCOMPARE(o.frameWidth, 150.0);
    QCOMPARE(o.dy, 889 / 20.0);
    QVERIFY(o.vRel == FO::Rel::Page && o.hRel == FO::Rel::Margin && o.hAlign == FO::Align::Center);
    // The paragraph's own attributes win; the rest are the style's.
    o = frameOf(framed("Boxed", "<w:framePr w:wrap=\"notBeside\" w:anchorLock=\"1\"/>"), &frames);
    QCOMPARE(frames, 1);
    QCOMPARE(o.frameWidth, 150.0);
    QCOMPARE(o.dy, 889 / 20.0);
    QVERIFY(o.wrap == FO::Wrap::TopBottom);
    // A style based on another says over it, attribute by attribute.
    o = frameOf(framed("BoxedToo", QByteArray()), &frames);
    QCOMPARE(o.frameWidth, 100.0);
    QCOMPARE(o.dy, 889 / 20.0);
    // Paragraphs with the same properties (their style's, said or not) are one frame; another property makes another.
    frameOf(framed("Boxed", QByteArray()) + framed("Boxed", "<w:framePr w:wrap=\"around\"/>") + framed("Boxed", "<w:framePr w:wrap=\"around\" w:w=\"3000\"/>"), &frames);
    QCOMPARE(frames, 1);
    frameOf(framed("Boxed", QByteArray()) + framed("BoxedToo", QByteArray()), &frames);
    QCOMPARE(frames, 2);
}


// The text of a frame goes round none of the other frames: two frames set at one place (the same properties, with a paragraph between them) are drawn one over the other, as
// the lines of each fill its own box (the text of the first is not put out of its box by the room that the second takes).
void WriteTests::aFramesTextIsNotPutOutOfItsBoxByAnotherFrameAtThePlace()
{
    const QByteArray pr = "<w:framePr w:w=\"6000\" w:h=\"2400\" w:wrap=\"notBeside\" w:vAnchor=\"page\" w:hAnchor=\"margin\" w:xAlign=\"center\" w:y=\"720\"/>";
    auto big = [&](const char *text) {
        return "<w:p><w:pPr>" + pr + "<w:spacing w:before=\"0\" w:after=\"0\"/></w:pPr><w:r><w:rPr><w:b/><w:sz w:val=\"72\"/></w:rPr><w:t>" + QByteArray(text) + "</w:t></w:r></w:p>";
    };
    const QByteArray body = big("HHHH") + big("HHHH") + big("HHHH") + plainP("Between the frames") + big(" ") + plainP("After");
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocx(body, kStyles, QByteArray(), sect));
    QVERIFY(doc);
    QCOMPARE(int(doc->flow().frames.size()), 2);
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    const QImage img = view.pageImage(0, 816);
    const double perPoint = 816 / 612.0;
    // The third line of the first frame (the frame's top is 36 points down; its lines are 36-point text, over 40 points each): ink between 36 + 85 and 36 + 115 points.
    int ink = 0;
    for (int y = int((36 + 85) * perPoint); y < int((36 + 115) * perPoint); ++y)
        for (int x = int(150 * perPoint); x < int(460 * perPoint); ++x) ink += qGray(img.pixel(x, y)) < 128;
    QVERIFY2(ink > 200, qPrintable(QStringLiteral("%1 dark pixels where the third line of the frame is").arg(ink)));
}


// What the properties of a frame say: its place (from the text, the margin, or the page, by an offset or at the start, middle, or end of it), its height (at least what is
// given, exactly that, or the text's), the room round it, and how the text goes round it; a drop cap is a frame at the start of its paragraph.
void WriteTests::aFramesPropertiesAreRead()
{
    auto frameOf = [&](const QByteArray &pr) {
        auto doc = open(makeDocx(plainP("Before") + framedP("frame", "<w:framePr " + pr + "/>") + plainP("After"), kStyles));
        return doc && doc->flow().frames.size() == 1 ? doc->flow().objects[doc->flow().frames[0] - 1] : flow::FloatingObject();
    };
    using FO = flow::FloatingObject;
    FO o = frameOf("w:w=\"1000\" w:h=\"800\" w:hRule=\"exact\" w:hAnchor=\"page\" w:vAnchor=\"page\" w:x=\"1440\" w:y=\"-720\"");
    QCOMPARE(o.frameWidth, 50.0);
    QCOMPARE(o.frameHeight, 40.0);
    QVERIFY(o.frameExactHeight);
    QVERIFY(o.hRel == FO::Rel::Page && o.vRel == FO::Rel::Page);
    QCOMPARE(o.dx, 72.0);
    QCOMPARE(o.dy, -36.0);
    QVERIFY(o.hAlign == FO::Align::None && o.vAlign == FO::Align::None);
    o = frameOf("w:w=\"1000\" w:h=\"800\" w:hRule=\"auto\" w:hAnchor=\"margin\" w:vAnchor=\"margin\" w:xAlign=\"center\" w:yAlign=\"bottom\" w:wrap=\"notBeside\"");
    QCOMPARE(o.frameHeight, 0.0);
    QVERIFY(o.hRel == FO::Rel::Margin && o.vRel == FO::Rel::Margin && o.hAlign == FO::Align::Center && o.vAlign == FO::Align::End);
    QVERIFY(o.wrap == FO::Wrap::TopBottom);
    o = frameOf("w:wrap=\"none\" w:xAlign=\"inside\" w:yAlign=\"top\"");
    QVERIFY(o.wrap == FO::Wrap::None && o.hAlign == FO::Align::Inside && o.vAlign == FO::Align::Start);   // (inside, across, is the side of the binding: the page's number says which)
    QCOMPARE(o.frameWidth, 0.0);   // (as wide as its text)
    o = frameOf("w:xAlign=\"outside\" w:yAlign=\"inside\"");
    QVERIFY(o.hAlign == FO::Align::Outside && o.vAlign == FO::Align::Start);   // (down, inside is the top)
    o = frameOf("w:wrap=\"tight\" w:hSpace=\"240\"");
    QVERIFY(o.wrap == FO::Wrap::Square);
    QCOMPARE(o.gap.right(), 12.0);
    o = frameOf("w:dropCap=\"drop\" w:lines=\"3\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"text\"");
    QVERIFY(o.hRel == FO::Rel::Column && o.vRel == FO::Rel::Paragraph && o.hAlign == FO::Align::Start && o.wrap == FO::Wrap::Square);
    QVERIFY(o.frameShrinks);   // (a drop cap is as wide as its letter; any other frame with no width is as wide as the column)
    QVERIFY(!frameOf("w:wrap=\"tight\" w:hSpace=\"240\"").frameShrinks);
    // Paragraphs with other properties are other frames; the same properties with a paragraph that is not in the frame between them are another frame.
    const QByteArray one = "<w:framePr w:w=\"1000\" w:hAnchor=\"page\" w:x=\"100\"/>", two = "<w:framePr w:w=\"1000\" w:hAnchor=\"page\" w:x=\"200\"/>";
    auto doc = open(makeDocx(plainP("Before") + framedP("a", one) + framedP("b", one) + framedP("c", two) + plainP("Between") + framedP("d", one) + plainP("After"), kStyles));
    QVERIFY(doc);
    QCOMPARE(int(doc->flow().frames.size()), 3);
    QVector<int> keys;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) keys << b.blockFormat().intProperty(flow::fp::Frame);
    QCOMPARE(keys, (QVector<int>{0, 1, 1, 2, 0, 3, 0}));
    // A paragraph with framePr in a cell of a table is no frame; nor is one in a header (which sets it as before).
    const QByteArray cell = cellX(framedP("in a cell", one));
    auto inCell = open(makeDocx(plainP("Before") + tableX(rowX(cell), 1) + plainP("After"), kStyles));
    QVERIFY(inCell);
    QCOMPARE(int(inCell->flow().frames.size()), 0);
}


// ---- editing next to frames

// The paragraphs of a document in a line each: the start of its text, with F for a paragraph of a frame (and H when it is set out of the text).
QStringList WriteTests::kindsOf(WriteDoc &doc)
{
    QStringList out;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        out << QStringLiteral("%1[%2%3]").arg(b.text().left(14)).arg(b.blockFormat().intProperty(flow::fp::Frame) ? QStringLiteral("F") : QString()).arg(b.blockFormat().boolProperty(tp::OutOfLayout) ? QStringLiteral("H") : QString());
    return out;
}

QByteArray WriteTests::aFramePr() { return "<w:framePr w:w=\"2400\" w:hSpace=\"180\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"margin\" w:xAlign=\"right\"/>"; }

QTextBlock WriteTests::blockNumber(WriteDoc &doc, int n) { return doc.story()->findBlockByNumber(n); }


// A frame's text is set apart from the text round it, so no edit joins the two: Backspace at the start of the paragraph after a frame and Delete at the end of the one before
// it do nothing (as at the edge of a cell), and so do they at the frame's own edges; inside the frame they join its paragraphs.
void WriteTests::backspaceAndDeleteNeverJoinAFramesTextAndTheTextRoundIt()
{
    const QByteArray body = plainP(wordsOf("Before", 30)) + framedP("FRAMEWORDS ONE", aFramePr()) + framedP("FRAMEWORDS TWO", aFramePr()) + plainP(wordsOf("After the frame", 60)) + plainP(wordsOf("Last", 40));
    auto fresh = [&]() { auto d = open(makeDocx(body, kStyles)); return d; };
    {
        auto doc = fresh();
        QVERIFY(doc);
        const QStringList was = kindsOf(*doc);
        QCOMPARE(int(was.size()), 5);
        QTextCursor c(doc->story());
        c.setPosition(blockNumber(*doc, 3).position());   // the start of the paragraph after the frame
        deleteBack(*doc, c, false);
        QCOMPARE(kindsOf(*doc), was);
        const QByteArray x = documentXml(doc->toDocx());
        QCOMPARE(int(x.count("w:framePr")), 2);
    }
    {
        auto doc = fresh();
        const QStringList was = kindsOf(*doc);
        QTextCursor c(doc->story());
        const QTextBlock b = blockNumber(*doc, 0);
        c.setPosition(b.position() + b.length() - 1);   // the end of the paragraph before the frame
        deleteForward(*doc, c, false);
        QCOMPARE(kindsOf(*doc), was);
    }
    {
        auto doc = fresh();
        const QStringList was = kindsOf(*doc);
        QTextCursor c(doc->story());
        c.setPosition(blockNumber(*doc, 1).position());   // the start of the frame
        deleteBack(*doc, c, false);
        QCOMPARE(kindsOf(*doc), was);
        const QTextBlock last = blockNumber(*doc, 2);
        c.setPosition(last.position() + last.length() - 1);   // the end of the frame
        deleteForward(*doc, c, false);
        QCOMPARE(kindsOf(*doc), was);
    }
    {
        // Inside the frame: its two paragraphs become one, still the frame's.
        auto doc = fresh();
        QTextCursor c(doc->story());
        c.setPosition(blockNumber(*doc, 2).position());
        deleteBack(*doc, c, false);
        QCOMPARE(int(kindsOf(*doc).size()), 4);
        QCOMPARE(blockNumber(*doc, 1).text(), QStringLiteral("FRAMEWORDS ONEFRAMEWORDS TWO"));
        QVERIFY(blockNumber(*doc, 1).blockFormat().intProperty(flow::fp::Frame) > 0 && blockNumber(*doc, 1).blockFormat().boolProperty(tp::OutOfLayout));
        QVERIFY(blockNumber(*doc, 2).blockFormat().intProperty(flow::fp::Frame) == 0);
        QCOMPARE(int(documentXml(doc->toDocx()).count("w:framePr")), 1);
        QVERIFY2(audit::floatProblems(doc->pager(), doc->flow()).isEmpty(), qPrintable(audit::floatProblems(doc->pager(), doc->flow())));
    }
    {
        // A selection from the text into the frame cuts what it holds of each and joins neither to the other.
        auto doc = fresh();
        const QTextBlock b0 = blockNumber(*doc, 0);
        QTextCursor c(doc->story());
        c.setPosition(b0.position() + b0.length() - 1 - 5);
        c.setPosition(blockNumber(*doc, 1).position() + 3, QTextCursor::KeepAnchor);
        deleteBack(*doc, c, false);
        QCOMPARE(int(kindsOf(*doc).size()), 5);
        QVERIFY(blockNumber(*doc, 0).blockFormat().intProperty(flow::fp::Frame) == 0);
        QCOMPARE(blockNumber(*doc, 1).text(), QStringLiteral("MEWORDS ONE"));
        QVERIFY(blockNumber(*doc, 1).blockFormat().boolProperty(tp::OutOfLayout));
        QVERIFY(!blockNumber(*doc, 0).text().contains(QLatin1String("MEWORDS")));
    }
}


// A paragraph style gives a paragraph its look, not its place: a frame's paragraph that is made a heading is still the frame's (and the file still says so).
void WriteTests::aStyleAppliedToAFramesParagraphLeavesItInTheFrame()
{
    const QByteArray body = plainP(wordsOf("Before", 30)) + framedP("FRAMEWORDS ONE", aFramePr()) + plainP(wordsOf("After the frame", 60)) + plainP(wordsOf("Last", 40));
    auto doc = open(makeDocx(body, kStyles));
    QVERIFY(doc);
    const int key = blockNumber(*doc, 1).blockFormat().intProperty(flow::fp::Frame);
    QVERIFY(key > 0);
    QTextCursor c(doc->story());
    c.setPosition(blockNumber(*doc, 1).position() + 2);
    applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
    const QTextBlockFormat bf = blockNumber(*doc, 1).blockFormat();
    QCOMPARE(bf.stringProperty(tp::StyleName), QStringLiteral("Heading1"));
    QCOMPARE(bf.intProperty(flow::fp::Frame), key);
    QVERIFY(bf.boolProperty(tp::OutOfLayout));
    // Still no line of the text, and a box with its words.
    for (int k = 0; k < doc->pager().pageCount(); ++k)
        for (const auto &l : doc->pager().lines(k)) QVERIFY(doc->story()->findBlock(l.pos).blockFormat().intProperty(flow::fp::Frame) == 0);
    QVERIFY2(audit::floatProblems(doc->pager(), doc->flow()).isEmpty(), qPrintable(audit::floatProblems(doc->pager(), doc->flow())));
    QVERIFY2(audit::coverageProblems(doc->pager(), doc->story()).isEmpty(), qPrintable(audit::coverageProblems(doc->pager(), doc->story())));
    const QByteArray x = documentXml(doc->toDocx());
    QCOMPARE(int(x.count("w:framePr")), 1);
    QVERIFY(x.indexOf("Heading1") > 0 && x.indexOf("Heading1") < x.indexOf("FRAMEWORDS ONE") && x.indexOf("w:framePr") < x.indexOf("FRAMEWORDS ONE"));
    // And so with a style that is put on the paragraph again, and one taken off.
    applyParagraphStyle(*doc, c, doc->defaultParagraphStyle());
    QCOMPARE(blockNumber(*doc, 1).blockFormat().intProperty(flow::fp::Frame), key);
    QVERIFY(blockNumber(*doc, 1).blockFormat().boolProperty(tp::OutOfLayout));
}


// Enter in a frame's paragraph makes another paragraph of the frame (even at its end, where the style says what follows it); Enter at the end of the paragraph before the
// frame makes one of the text, and what is typed in it is text.
void WriteTests::enterNextToAFrameMakesParagraphsOfTheRightPlace()
{
    const QByteArray body = plainP(wordsOf("Before", 30)) + framedP("FRAMEWORDS ONE", aFramePr()) + plainP(wordsOf("After the frame", 60)) + plainP(wordsOf("Last", 40));
    {
        auto doc = open(makeDocx(body, kStyles));
        const int key = blockNumber(*doc, 1).blockFormat().intProperty(flow::fp::Frame);
        QTextCursor c(doc->story());
        const QTextBlock f = blockNumber(*doc, 1);
        c.setPosition(f.position() + f.length() - 1);
        insertParagraph(*doc, c);
        typeText(*doc, c, QStringLiteral("NEWFRAMEPARA"));
        QCOMPARE(int(kindsOf(*doc).size()), 5);
        QCOMPARE(blockNumber(*doc, 2).text(), QStringLiteral("NEWFRAMEPARA"));
        QCOMPARE(blockNumber(*doc, 2).blockFormat().intProperty(flow::fp::Frame), key);
        QVERIFY(blockNumber(*doc, 2).blockFormat().boolProperty(tp::OutOfLayout));
        QCOMPARE(int(documentXml(doc->toDocx()).count("w:framePr")), 2);
    }
    {
        // A heading in the frame, and Enter at its end: the next paragraph is of the heading's next style, and still the frame's.
        auto doc = open(makeDocx(body, kStyles));
        const int key = blockNumber(*doc, 1).blockFormat().intProperty(flow::fp::Frame);
        QTextCursor c(doc->story());
        c.setPosition(blockNumber(*doc, 1).position() + 2);
        applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
        const QTextBlock f = blockNumber(*doc, 1);
        c.setPosition(f.position() + f.length() - 1);
        insertParagraph(*doc, c);
        typeText(*doc, c, QStringLiteral("NEXTSTYLE"));
        QCOMPARE(blockNumber(*doc, 2).text(), QStringLiteral("NEXTSTYLE"));
        QCOMPARE(blockNumber(*doc, 2).blockFormat().intProperty(flow::fp::Frame), key);
        QVERIFY(blockNumber(*doc, 2).blockFormat().boolProperty(tp::OutOfLayout));
        QVERIFY(blockNumber(*doc, 2).blockFormat().stringProperty(tp::StyleName) != QStringLiteral("Heading1"));
    }
    {
        auto doc = open(makeDocx(body, kStyles));
        QTextCursor c(doc->story());
        const QTextBlock b = blockNumber(*doc, 0);
        c.setPosition(b.position() + b.length() - 1);
        insertParagraph(*doc, c);
        typeText(*doc, c, QStringLiteral("TYPEDINTEXT"));
        QCOMPARE(int(kindsOf(*doc).size()), 5);
        QCOMPARE(blockNumber(*doc, 1).text(), QStringLiteral("TYPEDINTEXT"));
        QVERIFY(blockNumber(*doc, 1).blockFormat().intProperty(flow::fp::Frame) == 0 && !blockNumber(*doc, 1).blockFormat().boolProperty(tp::OutOfLayout));
        QVERIFY(blockNumber(*doc, 2).blockFormat().boolProperty(tp::OutOfLayout));
        bool shown = false;
        for (const auto &l : doc->pager().lines(0)) shown |= doc->story()->findBlock(l.pos).text() == QStringLiteral("TYPEDINTEXT");
        QVERIFY(shown);
    }
}


// Text copied from a frame is paragraphs of the text where it is pasted (a frame's key is its document's, and the paragraphs would be hidden in another's); pasted into a
// frame's paragraph it is the frame's.
void WriteTests::pastedFrameTextIsTextOrIsThatOfTheFrameItIsPastedInto()
{
    const QByteArray body = plainP(wordsOf("Before", 30)) + framedP("FRAMEWORDS ONE", aFramePr()) + plainP(wordsOf("After the frame", 60)) + plainP(wordsOf("Last", 40));
    auto a = open(makeDocx(body, kStyles));
    auto b = open(makeDocx(plainP(wordsOf("Other first", 20)) + plainP(wordsOf("Other second", 20)), kStyles));
    QVERIFY(a && b);
    QTextCursor sel(a->story());
    sel.setPosition(blockNumber(*a, 1).position());
    sel.setPosition(blockNumber(*a, 2).position() + 5, QTextCursor::KeepAnchor);
    const Clip clip = copySelection(*a, sel);
    QTextCursor at(b->story());
    at.setPosition(blockNumber(*b, 1).position() + 3);
    pasteClip(*b, at, clip);
    bool found = false;
    for (QTextBlock x = b->story()->begin(); x.isValid(); x = x.next()) {
        QVERIFY2(x.blockFormat().intProperty(flow::fp::Frame) == 0 && !x.blockFormat().boolProperty(tp::OutOfLayout), qPrintable(x.text().left(30)));
        found |= x.text().contains(QLatin1String("FRAMEWORDS"));
    }
    QVERIFY(found);
    QVERIFY2(audit::coverageProblems(b->pager(), b->story()).isEmpty(), qPrintable(audit::coverageProblems(b->pager(), b->story())));
    QCOMPARE(int(documentXml(b->toDocx()).count("w:framePr")), 0);
    // Into the frame's own paragraph: the frame's.
    QTextCursor inFrame(a->story());
    inFrame.setPosition(blockNumber(*a, 1).position() + 4);
    const int key = blockNumber(*a, 1).blockFormat().intProperty(flow::fp::Frame);
    QTextCursor from(b->story());
    from.setPosition(blockNumber(*b, 0).position());
    from.setPosition(blockNumber(*b, 2).position() + 4, QTextCursor::KeepAnchor);
    pasteClip(*a, inFrame, copySelection(*b, from));
    int framed = 0, others = 0;
    for (QTextBlock x = a->story()->begin(); x.isValid(); x = x.next()) {
        if (x.blockFormat().intProperty(flow::fp::Frame) == key && x.blockFormat().boolProperty(tp::OutOfLayout)) ++framed;
        else if (x.blockFormat().intProperty(flow::fp::Frame) != 0 || x.blockFormat().boolProperty(tp::OutOfLayout)) ++others;
    }
    QVERIFY2(framed >= 2 && others == 0, qPrintable(QStringLiteral("%1 framed, %2 other").arg(framed).arg(others)));
    QVERIFY2(audit::floatProblems(a->pager(), a->flow()).isEmpty(), qPrintable(audit::floatProblems(a->pager(), a->flow())));
}


// The caret is in a frame's text where the frame's box draws it, on the page that holds the frame (the page of the paragraph the frame is set from: here the second, with the
// frame's paragraph before the break), a click in the box puts the caret in the frame's text, and Home, End, and Up and Down go by the box's lines.
void WriteTests::theCaretAndTheMouseWorkInAFramesText()
{
    const QByteArray body = plainP("First page text") + framedP("FRAMEWORDS ONE and more words to make a second line in the box of the frame", aFramePr()) + plainP(wordsOf("Second page", 30), QByteArray(), "<w:pageBreakBefore/>") + plainP("Tail");
    auto doc = open(makeDocx(body, kStyles));
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 700);
    view.setDocument(doc.get());
    view.show();
    view.paginateAll();
    QCOMPARE(view.pageCount(), 2);
    const QTextBlock f = blockNumber(*doc, 1);
    QVERIFY(f.blockFormat().boolProperty(tp::OutOfLayout));
    QCOMPARE(doc->pager().pageOfPosition(f.position() + 3), 1);
    const auto page = doc->pager().materialize(1);
    QVERIFY(page);
    const TextItem *box = nullptr;
    for (const auto &it : page->doc->pages[0]->items)
        if (const auto *t = dynamic_cast<const TextItem *>(it.get()))
            if (const Story *st = page->doc->story(t->storyId))
                if (st->doc->toPlainText().contains(QStringLiteral("FRAMEWORDS")) && t->id.startsWith(QLatin1String("flow-object-"))) box = t;
    QVERIFY(box);
    auto caretAt = [&](int offset) {
        view.cursor().setPosition(f.position() + offset);
        view.setCursor(view.cursor());
        QRect r;
        return view.caretRect(&r) ? r : QRect();
    };
    const QRect a = caretAt(3);
    QVERIFY(a.isValid());
    QCOMPARE(view.currentPage(), 1);
    const QRect pr = view.pageRect(1);   // (where the view has scrolled to)
    const double sc = pr.width() / page->size.width();
    const QRectF boxDev(pr.left() + box->rect.left() * sc, pr.top() + box->rect.top() * sc, box->rect.width() * sc, box->rect.height() * sc);
    QVERIFY2(boxDev.adjusted(-3, -3, 3, 3).contains(a.center()), qPrintable(QStringLiteral("caret %1,%2 and the box %3,%4 %5x%6").arg(a.x()).arg(a.y()).arg(boxDev.x()).arg(boxDev.y()).arg(boxDev.width()).arg(boxDev.height())));
    const QRect b = caretAt(8);
    QVERIFY(b.isValid() && b.x() > a.x() + 10 && b.y() == a.y());
    // Home and End by the box's first line; Down goes to its second.
    view.cursor().setPosition(f.position() + 8);
    view.setCursor(view.cursor());
    view.typeKey(Qt::Key_Home, Qt::NoModifier);
    QCOMPARE(view.cursor().position(), f.position());
    view.typeKey(Qt::Key_End, Qt::NoModifier);
    QVERIFY2(view.cursor().position() > f.position() + 10 && view.cursor().position() < f.position() + f.length() - 1, qPrintable(QString::number(view.cursor().position() - f.position())));
    view.cursor().setPosition(f.position() + 8);
    view.setCursor(view.cursor());
    view.typeKey(Qt::Key_Down, Qt::NoModifier);
    QVERIFY2(view.cursor().position() > f.position() + 17 && view.cursor().position() < f.position() + f.length(), qPrintable(QString::number(view.cursor().position() - f.position())));
    // A click in the box's first line: the frame's text, near the place.
    const QRect again = caretAt(8);   // (the view may have scrolled)
    view.cursor().setPosition(f.position());   // (on the same page: the view stays)
    view.setCursor(view.cursor());
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(again.x() + 1, again.y() + again.height() / 2));
    QVERIFY2(std::abs(view.cursor().position() - (f.position() + 8)) <= 1, qPrintable(QString::number(view.cursor().position() - f.position())));
    // Typing goes in the frame; the first page's text and the second page's are as they were.
    typeText(*doc, view.cursor(), QStringLiteral("TYPED"));
    QVERIFY(f.text().contains(QLatin1String("TYPED")) || blockNumber(*doc, 1).text().contains(QLatin1String("TYPED")));
    QVERIFY(blockNumber(*doc, 1).blockFormat().boolProperty(tp::OutOfLayout));
    QVERIFY(!blockNumber(*doc, 2).text().contains(QLatin1String("TYPED")));
}


// ---- tables with the notes, headers and footers, and sections of a file (the two waves of JeffWrite together)

QByteArray WriteTests::endRef(int id) { return "<w:r><w:rPr><w:rStyle w:val=\"EndnoteReference\"/></w:rPr><w:endnoteReference w:id=\"" + QByteArray::number(id) + "\"/></w:r>"; }

// An endnotes part of the notes given (an Id and a text each), with its separator.
QByteArray WriteTests::endnotesOf(const QList<std::pair<int, QByteArray>> &notes)
{
    QByteArray out = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:endnote>";
    for (const auto &n : notes)
        out += "<w:endnote w:id=\"" + QByteArray::number(n.first) + "\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> " + n.second + "</w:t></w:r></w:p></w:endnote>";
    return out + "</w:endnotes>";
}

QByteArray WriteTests::refTo(int id) { return "<w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteReference w:id=\"" + QByteArray::number(id) + "\"/></w:r>"; }

QByteArray WriteTests::plainP(const QByteArray &text, const QByteArray &after, const QByteArray &pPr)
{
    return "<w:p>" + (pPr.isEmpty() ? QByteArray() : "<w:pPr>" + pPr + "</w:pPr>") + "<w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r>" + after + "</w:p>";
}

QByteArray WriteTests::wordsOf(const QByteArray &first, int count)
{
    QByteArray out = first;
    for (int i = 0; i < count; ++i) out += " word" + QByteArray::number(i);
    return out;
}

// A footnotes part of the notes given (an Id and a text each).
QByteArray WriteTests::notesPart(const QList<std::pair<int, QByteArray>> &notes)
{
    QByteArray out = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>";
    for (const auto &n : notes)
        out += "<w:footnote w:id=\"" + QByteArray::number(n.first) + "\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> " + n.second + "</w:t></w:r></w:p></w:footnote>";
    return out + "</w:footnotes>";
}

// A section's properties: a page of `w` by `h` twenty-ths of a point with `margin` on every side (header and footer 18 points from the edge), the references and break
// type that come before the page's size, and the elements that come after its margins (columns, a first page of its own).
QByteArray WriteTests::sectionXml(int w, int h, int margin, const QByteArray &refs, const QByteArray &type, const QByteArray &after)
{
    const QByteArray m = QByteArray::number(margin);
    return "<w:sectPr>" + refs + type + "<w:pgSz w:w=\"" + QByteArray::number(w) + "\" w:h=\"" + QByteArray::number(h) + "\"/><w:pgMar w:top=\"" + m + "\" w:right=\"" + m + "\" w:bottom=\"" + m + "\" w:left=\"" + m +
           "\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/>" + after + "</w:sectPr>";
}

QByteArray WriteTests::tableOfRows(int rows, int columns, int width, const std::function<QByteArray(int, int)> &cell)
{
    QByteArray out;
    for (int r = 0; r < rows; ++r) {
        QByteArray cells;
        for (int c = 0; c < columns; ++c) cells += cellX(cell(r, c), QByteArray(), width);
        out += rowX(cells);
    }
    return tableX(out, columns, width);
}

// How many rows the tables of the made pages show in all (a row shown in parts on two pages counts once).
int WriteTests::rowsShown(flow::Paginator &pg)
{
    std::set<std::pair<const void *, int>> rows;
    for (int k = 0; k < pg.pageCount(); ++k)
        if (const auto m = pg.materialize(k))
            for (const auto &t : m->tables)
                for (int r = t->firstRow; r < t->endRow; ++r) rows.insert({t->layout.get(), r});
    return int(rows.size());
}


// A note cited in the paragraph before a table, one cited in a cell of it, and one in the text after it, in a file (each alone, then the three together): each is on the
// page whose lines cite it, below the rows, at the number its reference has.
void WriteTests::aFilesNotesAroundATableAreOnThePagesThatCiteThem_data()
{
    QTest::addColumn<bool>("before");
    QTest::addColumn<bool>("cell");
    QTest::addColumn<bool>("after");
    QTest::newRow("in the paragraph before the table") << true << false << false;
    QTest::newRow("in a cell") << false << true << false;
    QTest::newRow("in the text after the table") << false << false << true;
    QTest::newRow("all three") << true << true << true;
}


void WriteTests::aFilesNotesAroundATableAreOnThePagesThatCiteThem()
{
    QFETCH(bool, before);
    QFETCH(bool, cell);
    QFETCH(bool, after);
    const QByteArray body = plainP(wordsOf("Opening", 40)) + plainP(wordsOf("Second", 30)) + plainP("The paragraph before the table", (before ? refTo(2) : QByteArray()) + "<w:r><w:t> and its end.</w:t></w:r>") +
                            tableOfRows(60, 2, 1500, [&](int r, int c) { return cell && c == 1 && r == 33 ? plainP("row 33 cites", refTo(3)) : cellP("row " + QByteArray::number(r) + " col " + QByteArray::number(c)); }) +
                            plainP("The paragraph after the table", (after ? refTo(4) : QByteArray()) + "<w:r><w:t> and its end.</w:t></w:r>") + plainP(wordsOf("Last", 20));
    const QByteArray notes = notesPart({{2, "ALPHA note of the paragraph before the table"}, {3, wordsOf("BETA", 25)}, {4, wordsOf("GAMMA", 8)}});
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    flow::Paginator &pg = doc->pager();
    const int pages = pg.pageCount();
    QVERIFY(pages >= 4);
    QMap<QString, int> pageOf;
    for (int k = 0; k < pages; ++k)
        for (const QString &id : audit::citedNotes(pg, doc->story(), k)) pageOf[id] = k;
    QCOMPARE(int(pageOf.size()), int(before) + int(cell) + int(after));
    if (before && cell && after)
        QVERIFY2(pageOf["fn2"] < pageOf["fn3"] && pageOf["fn3"] < pageOf["fn4"], qPrintable(QStringLiteral("the notes are on the pages %1 %2 %3").arg(pageOf["fn2"]).arg(pageOf["fn3"]).arg(pageOf["fn4"])));
    const QString bad = audit::problems(pg, doc->story(), {{"fn2", "ALPHA"}, {"fn3", "BETA"}, {"fn4", "GAMMA"}});
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    QCOMPARE(rowsShown(pg), 60);
}


// The first page, the even pages, and the odd ones have headers and footers of other heights, over a table that runs across many pages: its rows stay in the text area
// that they leave on each, none over the footer, and a page holds as many rows as its own area has room for.
void WriteTests::aFilesTableRowsStayInTheBodyThatItsHeadersAndFootersLeave()
{
    const QByteArray refs = "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:headerReference w:type=\"first\" r:id=\"rIdheader2\"/><w:headerReference w:type=\"even\" r:id=\"rIdheader3\"/>"
                            "<w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:footerReference w:type=\"first\" r:id=\"rIdfooter2\"/><w:footerReference w:type=\"even\" r:id=\"rIdfooter3\"/>";
    auto lines = [](const char *name, int n) {
        QList<QByteArray> out;
        for (int i = 1; i <= n; ++i) out << "<w:r><w:t>" + QByteArray(name) + " " + QByteArray::number(i) + "</w:t></w:r>";
        return out;
    };
    const QMap<QString, QByteArray> parts{{QStringLiteral("/word/header1.xml"), headerPart("hdr", lines("ODD HEAD", 2))}, {QStringLiteral("/word/header2.xml"), headerPart("hdr", lines("FIRST HEAD", 1))},
                                          {QStringLiteral("/word/header3.xml"), headerPart("hdr", lines("EVEN HEAD", 5))}, {QStringLiteral("/word/footer1.xml"), headerPart("ftr", lines("ODD FOOT", 1))},
                                          {QStringLiteral("/word/footer2.xml"), headerPart("ftr", lines("FIRST FOOT", 3))}, {QStringLiteral("/word/footer3.xml"), headerPart("ftr", lines("EVEN FOOT", 4))}};
    const QByteArray body = plainP("Before the table") + tableOfRows(90, 2, 1500, [](int r, int c) { return r % 7 == 3 ? cellP("row " + QByteArray::number(r) + " col " + QByteArray::number(c)) + cellP("a second line") : cellP("row " + QByteArray::number(r) + " col " + QByteArray::number(c)); }) +
                            plainP("After the table");
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720, refs, QByteArray(), "<w:titlePg/>"), parts, "<w:evenAndOddHeaders/>"));
    QVERIFY(doc);
    flow::Paginator &pg = doc->pager();
    const int pages = pg.pageCount();
    QVERIFY(pages >= 6);
    QSet<int> heights;
    for (int k = 0; k < pages; ++k) heights << int(std::lround(pg.page(k).content.height() * 10));
    QVERIFY2(heights.size() >= 3, "the pages' text areas are of three heights (the first page's, the odd pages', the even pages')");
    const QString bad = audit::problems(pg, doc->story());
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    for (int k = 0; k < pages; ++k) {
        const auto page = pg.materialize(k);
        QVERIFY(page);
        const QString want = k == 0 ? QStringLiteral("FIRST") : (k + 1) % 2 == 0 ? QStringLiteral("EVEN") : QStringLiteral("ODD");
        const auto heads = audit::itemsWith(*page, QStringLiteral("flow-header-box")), foots = audit::itemsWith(*page, QStringLiteral("flow-footer-box"));
        QVERIFY2(heads.size() == 1 && foots.size() == 1, qPrintable(QStringLiteral("page %1 has %2 headers and %3 footers").arg(k).arg(heads.size()).arg(foots.size())));
        QVERIFY2(audit::storyOf(*page, heads[0]).startsWith(want + QStringLiteral(" HEAD")), qPrintable(QStringLiteral("page %1: %2").arg(k).arg(audit::storyOf(*page, heads[0]))));
        QVERIFY2(audit::storyOf(*page, foots[0]).startsWith(want + QStringLiteral(" FOOT")), qPrintable(QStringLiteral("page %1: %2").arg(k).arg(audit::storyOf(*page, foots[0]))));
        // The rows end above the footer, and the page is as full as its room lets it be.
        if (!page->tables.isEmpty()) {
            const auto &last = page->tables.last();
            QVERIFY2(last->rect().bottom() <= foots[0]->rect.top() + 0.05, qPrintable(QStringLiteral("page %1: rows end at %2, the footer begins at %3").arg(k).arg(last->rect().bottom()).arg(foots[0]->rect.top())));
            const auto next = k + 1 < pages ? pg.materialize(k + 1) : nullptr;
            if (next && !next->tables.isEmpty() && next->tables.first()->firstRow == last->endRow)
                QVERIFY2(last->rect().bottom() + next->tables.first()->heightOf(last->endRow) > pg.page(k).content.bottom() + 0.05, qPrintable(QStringLiteral("page %1 leaves room for the next row").arg(k)));
        }
    }
    QCOMPARE(rowsShown(pg), 90);
}


// A row taken away beside a content control that holds rows (the table is itself in a content control) leaves the table and the controls valid: the control that held only
// that row stays, empty; the control before the row taken stays closed where it was.
void WriteTests::deletingARowBesideAContentControlKeepsTheTableAndTheControlsValid()
{
    auto row = [](const char *text) { return QByteArray("<w:tr><w:tc><w:p><w:r><w:t>") + text + "</w:t></w:r></w:p></w:tc></w:tr>"; };
    const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid>" + row("r0") +
                             "<w:sdt><w:sdtPr><w:id w:val=\"2\"/></w:sdtPr><w:sdtContent>" + row("r1") + "</w:sdtContent></w:sdt>" + row("r2") + row("r3") + "</w:tbl>";
    const QByteArray body = plainP("before") + "<w:sdt><w:sdtPr><w:id w:val=\"1\"/></w:sdtPr><w:sdtContent>" + table + "</w:sdtContent></w:sdt>" + plainP("after");
    // The saved file is well formed, and every row is inside a table.
    auto problems = [](const QByteArray &xmlText) {
        QXmlStreamReader in(xmlText);
        QStringList stack;
        while (!in.atEnd()) {
            in.readNext();
            if (in.isStartElement()) {
                if (in.name() == QLatin1String("tr")) {
                    int k = int(stack.size()) - 1;
                    while (k >= 0 && (stack[k] == QLatin1String("sdt") || stack[k] == QLatin1String("sdtContent"))) --k;
                    if (k < 0 || stack[k] != QLatin1String("tbl")) return QStringLiteral("a row is in a %1").arg(k < 0 ? QStringLiteral("nothing") : stack[k]);
                }
                stack << in.name().toString();
            } else if (in.isEndElement() && !stack.isEmpty()) {
                stack.removeLast();
            }
        }
        return in.hasError() ? in.errorString() : QString();
    };
    for (int which = 0; which < 2; ++which) {
        auto doc = open(makeDocxWithParts(body, sectionXml(12240, 15840, 1440), {}));
        QVERIFY(doc);
        const QByteArray was = documentXml(doc->toDocx());
        QVERIFY(was.contains(table));
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->toPlainText().indexOf(which == 0 ? QStringLiteral("r1") : QStringLiteral("r2")));
        deleteTableRows(*doc, c);
        const QByteArray out = documentXml(doc->toDocx());
        const QString label = which == 0 ? QStringLiteral("the row in the control") : QStringLiteral("the row after it");
        QVERIFY2(problems(out).isEmpty(), qPrintable(label + QStringLiteral(": ") + problems(out) + QLatin1String(" in ") + QString::fromUtf8(out)));
        QCOMPARE(out.count("<w:tbl>"), 1);
        QCOMPARE(out.count("<w:tr>"), 3);
        QCOMPARE(out.count("<w:sdt>"), 2);
        QCOMPARE(out.count("</w:sdt>"), 2);
        QVERIFY2(!out.contains(which == 0 ? "r1" : "r2"), qPrintable(label));
        // (And the file opens as the table it is: three rows, the same controls.)
        auto again = open(doc->toDocx());
        QVERIFY(again);
        const QByteArray second = documentXml(again->toDocx());
        QCOMPARE(second.count("<w:tr>"), 3);
        QCOMPARE(second.count("<w:sdt>"), 2);
        QVERIFY(problems(second).isEmpty());
        if (which == 1) QVERIFY2(out.contains("<w:sdtContent><w:tr>") && out.contains("r1</w:t></w:r></w:p></w:tc></w:tr></w:sdtContent></w:sdt>"), qPrintable(QString::fromUtf8(out)));
        // Taken away and brought back, as it was.
        doc->story()->undo();
        QCOMPARE(documentXml(doc->toDocx()), was);
    }
}


// Rows and cells that content controls hold are shown, and a file that is saved after one of their cells is edited has the controls as they were.
void WriteTests::aTableWhoseRowsAndCellsAreInContentControlsShowsThemAndKeepsThem()
{
        const QByteArray controlRow = R"(<w:sdt><w:sdtPr><w:id w:val="1"/></w:sdtPr><w:sdtContent><w:sdt><w:sdtPr><w:id w:val="2"/></w:sdtPr><w:sdtContent>)"
                                      R"(<w:tr><w:sdt><w:sdtPr><w:id w:val="3"/></w:sdtPr><w:sdtContent><w:tc><w:p><w:r><w:t>Choose an item.</w:t></w:r></w:p></w:tc></w:sdtContent></w:sdt>)"
                                      R"(<w:tc><w:p><w:r><w:t>Here is a sample</w:t></w:r></w:p></w:tc></w:tr></w:sdtContent></w:sdt></w:sdtContent></w:sdt>)";
        const QByteArray table = R"(<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/></w:tblPr><w:tblGrid><w:gridCol w:w="2000"/><w:gridCol w:w="2000"/></w:tblGrid>)"
                                 R"(<w:tr><w:tc><w:p><w:r><w:t>Note</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>Details</w:t></w:r></w:p></w:tc></w:tr>)" + controlRow +
                                 R"(<w:tr><w:tc><w:p><w:r><w:t>last</w:t></w:r></w:p></w:tc><w:tc><w:p/></w:tc></w:tr></w:tbl>)";
        auto doc = open(makeDocxWithParts(plainP("Sample 1:") + table + plainP("after"), sectionXml(12240, 15840, 1440), {}));
        QVERIFY(doc);
        const QString text = doc->story()->toPlainText();
        QVERIFY2(text.contains(QStringLiteral("Choose an item.")) && text.contains(QStringLiteral("Here is a sample")), qPrintable(text));
        QVERIFY(documentXml(doc->toDocx()).contains(table));
        // The sample cell is edited: the controls are written where they were.
        QTextCursor c(doc->story());
        c.setPosition(text.indexOf(QStringLiteral("Here is a sample")));
        c.setPosition(c.position() + 4, QTextCursor::KeepAnchor);
        c.insertText(QStringLiteral("There"));
        const QByteArray out = documentXml(doc->toDocx());
        QVERIFY2(out.contains("There is a sample"), out.constData());
        QVERIFY(out.contains(R"(<w:sdtContent><w:sdt><w:sdtPr><w:id w:val="2"/></w:sdtPr><w:sdtContent><w:tr><w:sdt><w:sdtPr><w:id w:val="3"/></w:sdtPr><w:sdtContent><w:tc>)"));
        QVERIFY(out.contains(R"(</w:tc></w:sdtContent></w:sdt><w:tc>)"));
        QVERIFY(out.contains(R"(</w:tr></w:sdtContent></w:sdt></w:sdtContent></w:sdt><w:tr>)"));
    }


// A section's line numbers (w:lnNumType) are drawn in the margin, left of the text by the distance the file says, in the "line number" character style, from 1 at each page, and are
// written back as they were.
void WriteTests::aSectionsLineNumbersAreDrawnAndKept()
{
    const QByteArray styles = kStyles + QByteArray("<w:style w:type=\"character\" w:styleId=\"LineNumber\"><w:name w:val=\"line number\"/><w:rPr><w:sz w:val=\"14\"/></w:rPr></w:style>");
    QByteArray body;
    for (int i = 0; i < 40; ++i) body += plainP(wordsOf("line", 6));
    const QByteArray sect = sectionXml(6000, 6000, 1440, QByteArray(), QByteArray(), "<w:lnNumType w:countBy=\"1\" w:distance=\"144\"/>");
    auto doc = open(makeDocxWithParts(body, sect, {}, QByteArray(), styles));
    QVERIFY(doc);
    flow::Paginator &pg = doc->pager();
    QVERIFY(pg.pageCount() >= 3);
    for (int k = 0; k < pg.pageCount(); ++k) {
        const auto page = pg.materialize(k);
        QVERIFY(page);
        QStringList numbers;
        double right = 0;
        for (const auto &item : page->doc->pages[0]->items)
            if (item->id.startsWith(QLatin1String("flow-lineno-box"))) {
                numbers << page->doc->stories.value(dynamic_cast<const TextItem *>(item.get())->storyId)->doc->toPlainText();
                right = item->rect.right();
            }
        QCOMPARE(int(numbers.size()), int(pg.lines(k).size()));
        QCOMPARE(numbers.value(0), QStringLiteral("1"));
        QCOMPARE(numbers.last(), QString::number(numbers.size()));
        QVERIFY2(std::abs(right - (pg.page(k).content.left() - 7.2)) < 0.01, qPrintable(QString::number(right)));
    }
    QVERIFY(documentXml(doc->toDocx()).contains("<w:lnNumType w:countBy=\"1\" w:distance=\"144\"/>"));
}


// A section's line numbers skip as many numbers as the file's start says (3: the first line is 4), then go on from page to page when the file says they run on (w:restart
// "continuous") and begin again at each page when it says nothing.
void WriteTests::aSectionsLineNumbersSkipTheStartAndRunOnWhenTheFileSays()
{
    QByteArray body;
    for (int i = 0; i < 40; ++i) body += plainP(wordsOf("line", 6));
    for (const bool runOn : {false, true}) {
        const QByteArray numbering = runOn ? "<w:lnNumType w:countBy=\"1\" w:start=\"3\" w:restart=\"continuous\"/>" : "<w:lnNumType w:countBy=\"1\" w:start=\"3\"/>";
        auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 1440, QByteArray(), QByteArray(), numbering), {}, QByteArray(), kStyles));
        QVERIFY(doc);
        flow::Paginator &pg = doc->pager();
        QVERIFY(pg.pageCount() >= 3);
        int before = 0;
        for (int k = 0; k < pg.pageCount(); ++k) {
            const auto page = pg.materialize(k);
            QVERIFY(page);
            QStringList numbers;
            for (const auto &item : page->doc->pages[0]->items)
                if (item->id.startsWith(QLatin1String("flow-lineno-box"))) numbers << page->doc->stories.value(dynamic_cast<const TextItem *>(item.get())->storyId)->doc->toPlainText();
            const int lines = int(pg.lines(k).size());
            QCOMPARE(int(numbers.size()), lines);
            QCOMPARE(numbers.value(0), QString::number(4 + (runOn ? before : 0)));
            before += lines;
        }
    }
}


// Ruby text in a header shows as it does in the text: the base, and the ruby after it in brackets.
void WriteTests::rubyInAHeaderIsShown()
{
    const QByteArray ruby = "<w:r><w:ruby><w:rt><w:r><w:t>ruby</w:t></w:r></w:rt><w:rubyBase><w:r><w:t>base</w:t></w:r></w:rubyBase></w:ruby></w:r>";
    const QMap<QString, QByteArray> parts{{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t xml:space=\"preserve\">Head </w:t></w:r>" + ruby})}};
    auto doc = open(makeDocxWithParts(plainP("body"), sectionXml(6000, 6000, 720, "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>"), parts));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    const auto heads = audit::itemsWith(*page, QStringLiteral("flow-header-box"));
    QVERIFY(heads.size() == 1);
    QCOMPARE(audit::storyOf(*page, heads[0]), QStringLiteral("Head base(ruby)"));
}


// An equation is drawn: the line of text that is its characters in order, on the baseline, in the size and face of its first run, in the text, in a cell of a table, in a text box,
// and in a header; saved, it is what the file had.
void WriteTests::anEquationIsDrawnAsALineOfText()
{
    const QByteArray ns = "xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"";
    const QByteArray eq = "<m:oMath " + ns + "><m:r><w:rPr><w:sz w:val=\"40\"/></w:rPr><m:t>y=</m:t></m:r><m:f><m:num><m:r><m:t>a</m:t></m:r></m:num><m:den><m:r><m:t>b+1</m:t></m:r></m:den></m:f></m:oMath>";
    const QString line = QStringLiteral("y = a/(b + 1)");
    // In the text: a line with the words round it, the atom is set as the run says and its letters are italic.
    auto doc = open(makeDocxWithParts("<w:p><w:r><w:t xml:space=\"preserve\">Solve </w:t></w:r>" + eq + "<w:r><w:t xml:space=\"preserve\"> now</w:t></w:r></w:p>", sectionXml(6000, 6000, 720), {}));
    QVERIFY(doc);
    QCOMPARE(boxLines(*doc), QStringList{QStringLiteral("Solve y = a/(b + 1) now")});
    const QList<QTextCharFormat> atoms = atomFormats(*doc, QStringLiteral("oMath"));
    QCOMPARE(int(atoms.size()), 1);
    QCOMPARE(atoms[0].stringProperty(tp::Field), QStringLiteral("raw:") + line);
    QVERIFY2(std::abs(atoms[0].fontPointSize() - 20) < 0.01, qPrintable(QString::number(atoms[0].fontPointSize())));
    QCOMPARE(atoms[0].stringProperty(tp::DisplayStyle), QStringLiteral("i"));
    QVERIFY(atoms[0].fontFamilies().toStringList().contains(QStringLiteral("Cambria Math")));
    const QByteArray saved = documentXml(doc->toDocx());
    QVERIFY(saved.contains(eq));
    QVERIFY(!saved.contains("DisplayStyle"));
    // In a cell of a table.
    auto table = open(makeDocxWithParts("<w:tbl><w:tblPr/><w:tblGrid><w:gridCol w:w=\"3000\"/></w:tblGrid><w:tr><w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr><w:p>" + eq +
                                        "</w:p></w:tc></w:tr></w:tbl><w:p><w:r><w:t>after</w:t></w:r></w:p>", sectionXml(6000, 6000, 720), {}));
    QVERIFY(table);
    QCOMPARE(int(atomFormats(*table, QStringLiteral("oMath")).size()), 1);
    QCOMPARE(atomFormats(*table, QStringLiteral("oMath"))[0].stringProperty(tp::Field), QStringLiteral("raw:") + line);
    QVERIFY(documentXml(table->toDocx()).contains(eq));
    // In a text box of the text, and in a header.
    const QByteArray box = "<w:p><w:r><w:t>text</w:t></w:r><w:r><w:pict " + QByteArray(kVml) + "><v:shape style=\"position:absolute;margin-left:10pt;margin-top:60pt;width:150pt;height:40pt;z-index:3\">"
                           "<v:textbox><w:txbxContent><w:p>" + eq + "</w:p></w:txbxContent></v:textbox><w10:wrap type=\"square\"/></v:shape></w:pict></w:r></w:p>";
    auto boxed = open(makeDocxWithParts(box, sectionXml(6000, 6000, 720), {}));
    QVERIFY(boxed);
    const auto boxedPage = boxed->pager().materialize(0);
    QVERIFY(boxedPage);
    QVERIFY2(boxWithText(*boxedPage, line), "the text box shows the equation");
    const QMap<QString, QByteArray> parts{{QStringLiteral("/word/header1.xml"), headerPart("hdr", {eq})}};
    auto headed = open(makeDocxWithParts(plainP("body"), sectionXml(6000, 6000, 720, "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>"), parts));
    QVERIFY(headed);
    const auto headedPage = headed->pager().materialize(0);
    QVERIFY(headedPage);
    const auto heads = audit::itemsWith(*headedPage, QStringLiteral("flow-header-box"));
    QVERIFY(heads.size() == 1);
    QCOMPARE(audit::storyOf(*headedPage, heads[0]), line);
}


// An equation that is a paragraph of its own (m:oMathPara) is a line of its own, centered on the page: the line it makes is in the middle of the text area, and the paragraphs round it
// begin at the left.
void WriteTests::anEquationOfItsOwnParagraphIsCentered()
{
    const QByteArray ns = "xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"";
    auto doc = open(makeDocxWithParts("<w:p><w:r><w:t>Before</w:t></w:r></w:p><w:p><m:oMathPara " + ns + "><m:oMath><m:r><m:t>a=b</m:t></m:r></m:oMath></m:oMathPara></w:p><w:p><w:r><w:t>After</w:t></w:r></w:p>",
                                      sectionXml(6000, 6000, 720), {}));
    QVERIFY(doc);
    QCOMPARE(boxLines(*doc), (QStringList{QStringLiteral("Before"), QStringLiteral("a = b"), QStringLiteral("After")}));
    const auto page = doc->pager().materialize(0);
    QVERIFY(page && !page->boxes.isEmpty());
    const auto *box = dynamic_cast<const TextItem *>(page->doc->item(page->boxes[0]));
    QVERIFY(box);
    FrameSpec fs;
    fs.size = box->rect.size();
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.hyphenate = false;
    StoryLayout layout;
    layout.build(page->doc->story(box->storyId)->doc.get(), {fs}, doc->flow().env);
    auto rectOfBlock = [&](int number) {
        const QTextBlock b = page->doc->story(box->storyId)->doc->findBlockByNumber(number);
        const QVector<QRectF> rects = layout.rangeRects(0, b.position(), b.position() + std::max(1, b.length() - 1));
        return rects.isEmpty() ? QRectF() : rects.first();
    };
    const QRectF equation = rectOfBlock(1), before = rectOfBlock(0);
    QVERIFY2(equation.width() > 0 && std::abs((equation.left() + equation.right()) / 2 - fs.size.width() / 2) < 3, qPrintable(QStringLiteral("the equation is from %1 to %2 in %3").arg(equation.left()).arg(equation.right()).arg(fs.size.width())));
    QVERIFY2(before.left() < 3, qPrintable(QStringLiteral("the paragraph before begins at %1").arg(before.left())));
}


// Opened and saved with no edit, a paragraph that is an equation of its own is as the file had it: the centering the pages give it is not written (no w:jc is added to it).
void WriteTests::anEquationOfItsOwnParagraphIsWrittenBackAsItWas()
{
    const QByteArray ns = "xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"";
    const QByteArray equation = "<w:p><m:oMathPara " + ns + "><m:oMath><m:r><m:t>a=b</m:t></m:r></m:oMath></m:oMathPara></w:p>";
    auto doc = open(makeDocxWithParts("<w:p><w:r><w:t>Before</w:t></w:r></w:p>" + equation + "<w:p><w:r><w:t>After</w:t></w:r></w:p>", sectionXml(6000, 6000, 720), {}));
    QVERIFY(doc);
    const QByteArray saved = documentXml(doc->toDocx());
    QVERIFY2(saved.contains(equation), saved.constData());
    QVERIFY(!saved.contains("<w:jc"));
    // (And a second save, after the file is opened again, is the same.)
    auto again = open(doc->toDocx());
    QVERIFY(again);
    QCOMPARE(documentXml(again->toDocx()), saved);
}


// A letter of the mathematical alphanumeric block in the text (a run in a math face has it) is drawn as its letter, in the style it stands for (italic here), where no face of the
// machine has the block; a double-struck one is drawn as it is, in a bundled face that has it; the file's characters are written back as they were.
void WriteTests::aStyledLetterInTextIsDrawnAsItsLetter()
{
    const QByteArray text = QString::fromUtf8("<w:p><w:r><w:rPr><w:rFonts w:ascii=\"Cambria Math\" w:hAnsi=\"Cambria Math\"/></w:rPr><w:t>\xf0\x9d\x91\x86</w:t></w:r>"
                                              "<w:r><w:t xml:space=\"preserve\"> and </w:t></w:r><w:r><w:t>\xf0\x9d\x94\xb8</w:t></w:r><w:r><w:t xml:space=\"preserve\"> and </w:t></w:r><w:r><w:t>\xf0\x9d\x90\x81x</w:t></w:r></w:p>").toUtf8();
    auto doc = open(makeDocxWithParts(text, sectionXml(6000, 6000, 720), {}));
    QVERIFY(doc);
    QCOMPARE(boxLines(*doc), QStringList{QString::fromUtf8("S and \xf0\x9d\x94\xb8 and Bx")});
    const QList<QTextCharFormat> atoms = atomFormats(*doc, QStringLiteral("t"));
    QCOMPARE(int(atoms.size()), 3);
    QCOMPARE(atoms[0].stringProperty(tp::Field), QStringLiteral("raw:S"));
    QCOMPARE(atoms[0].stringProperty(tp::DisplayStyle), QStringLiteral("i"));
    QCOMPARE(atoms[1].stringProperty(tp::Field), QString::fromUtf8("raw:\xf0\x9d\x94\xb8"));
    QCOMPARE(atoms[1].stringProperty(tp::DisplayFamily), QStringLiteral("DejaVu Sans"));
    QCOMPARE(atoms[2].stringProperty(tp::DisplayStyle), QStringLiteral("b"));
    const QByteArray saved = documentXml(doc->toDocx());
    QVERIFY(saved.contains("\xf0\x9d\x91\x86") && saved.contains("\xf0\x9d\x94\xb8") && saved.contains("\xf0\x9d\x90\x81"));
    QVERIFY(!saved.contains("DisplayStyle") && !saved.contains("DisplayFamily"));
    // In a text box too.
    const QByteArray box = QString::fromUtf8("<w:p><w:r><w:t>text</w:t></w:r><w:r><w:pict ").toUtf8() + kVml + "><v:shape style=\"position:absolute;margin-left:10pt;margin-top:60pt;width:150pt;height:40pt;z-index:3\">"
                           "<v:textbox><w:txbxContent><w:p><w:r><w:t>\xf0\x9d\x91\x86 plus</w:t></w:r></w:p></w:txbxContent></v:textbox><w10:wrap type=\"square\"/></v:shape></w:pict></w:r></w:p>";
    auto boxed = open(makeDocxWithParts(box, sectionXml(6000, 6000, 720), {}));
    QVERIFY(boxed);
    const auto page = boxed->pager().materialize(0);
    QVERIFY(page);
    QVERIFY2(boxWithText(*page, QStringLiteral("S plus")), "the text box shows the letter");
}


// A legacy form field is drawn as what it shows: a check box its box (checked or not), a text field its result, or its default text where it has none, a drop-down the entry it has
// chosen; in the text, in a text box, and in a header; and the fields are written back as they were.
void WriteTests::aFormFieldIsDrawnAsWhatItShows()
{
    auto field = [](const QByteArray &data, const QByteArray &code, const QByteArray &result = QByteArray()) {
        return "<w:r><w:fldChar w:fldCharType=\"begin\"><w:ffData><w:name w:val=\"F\"/><w:enabled/>" + data + "</w:ffData></w:fldChar></w:r><w:r><w:instrText xml:space=\"preserve\"> " + code +
               " </w:instrText></w:r>" + (result.isNull() ? QByteArray() : "<w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" + result) + "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
    };
    auto line = [](const QByteArray &label, const QByteArray &content) { return "<w:p><w:r><w:t xml:space=\"preserve\">" + label + " </w:t></w:r>" + content + "</w:p>"; };
    const QByteArray off = "<w:checkBox><w:sizeAuto/><w:default w:val=\"0\"/></w:checkBox>", on = "<w:checkBox><w:sizeAuto/><w:default w:val=\"1\"/></w:checkBox>";
    const QByteArray ticked = "<w:checkBox><w:size w:val=\"24\"/><w:default w:val=\"0\"/><w:checked/></w:checkBox>", unticked = "<w:checkBox><w:sizeAuto/><w:default w:val=\"1\"/><w:checked w:val=\"0\"/></w:checkBox>";
    const QByteArray text = "<w:textInput><w:default w:val=\"Jane &amp; Co\"/></w:textInput>";
    const QByteArray drop = "<w:ddList><w:result w:val=\"1\"/><w:listEntry w:val=\"Alpha\"/><w:listEntry w:val=\"Beta\"/><w:listEntry w:val=\"Gamma\"/></w:ddList>";
    const QByteArray blank = "<w:r><w:t> </w:t></w:r>";
    const QList<QByteArray> bodies = {line("A", field(off, "FORMCHECKBOX")), line("B", field(on, "FORMCHECKBOX")), line("C", field(ticked, "FORMCHECKBOX")), line("D", field(unticked, "FORMCHECKBOX")),
                                      line("E", field(text, "FORMTEXT", blank + blank)), line("F", field(text, "FORMTEXT", "<w:r><w:t>Alice</w:t></w:r>")), line("G", field(drop, "FORMDROPDOWN"))};
    const QStringList shown = {QString::fromUtf8("A ☐"), QString::fromUtf8("B ☒"), QString::fromUtf8("C ☒"), QString::fromUtf8("D ☐"), QStringLiteral("E Jane & Co"), QStringLiteral("F Alice"),
                               QStringLiteral("G Beta")};
    QByteArray body;
    for (const QByteArray &b : bodies) body += b;
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 9000, 720), {}));
    QVERIFY(doc);
    QCOMPARE(boxLines(*doc), shown);
    const QByteArray saved = documentXml(doc->toDocx());
    for (const QByteArray &data : {off, on, ticked, unticked, text, drop}) QVERIFY2(saved.contains(data), data.constData());
    QVERIFY(!saved.contains("DisplayFamily") && !saved.contains("<w:sz "));
    // In a text box, and in a header: the same.
    QByteArray inBox;
    for (const QByteArray &b : bodies) inBox += b;
    const QByteArray box = "<w:p><w:r><w:t>text</w:t></w:r><w:r><w:pict " + QByteArray(kVml) + "><v:shape style=\"position:absolute;margin-left:10pt;margin-top:60pt;width:150pt;height:200pt;z-index:3\">"
                           "<v:textbox><w:txbxContent>" + inBox + "</w:txbxContent></v:textbox><w10:wrap type=\"square\"/></v:shape></w:pict></w:r></w:p>";
    auto boxed = open(makeDocxWithParts(box, sectionXml(6000, 9000, 720), {}));
    QVERIFY(boxed);
    const auto page = boxed->pager().materialize(0);
    QVERIFY(page);
    const TextItem *boxItem = boxWithText(*page, QStringLiteral("G "));
    QVERIFY2(boxItem, "the text box is there");
    QCOMPARE(page->doc->story(boxItem->storyId)->doc->toPlainText().split(QLatin1Char('\n')), shown);
    QList<QByteArray> headerParagraphs;
    for (const QByteArray &b : bodies) headerParagraphs << b.mid(5, b.size() - 11);
    const QMap<QString, QByteArray> parts{{QStringLiteral("/word/header1.xml"), headerPart("hdr", headerParagraphs)}};
    auto headed = open(makeDocxWithParts(plainP("body"), sectionXml(6000, 9000, 720, "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>"), parts));
    QVERIFY(headed);
    const auto headedPage = headed->pager().materialize(0);
    QVERIFY(headedPage);
    const auto heads = audit::itemsWith(*headedPage, QStringLiteral("flow-header-box"));
    QVERIFY(heads.size() == 1);
    QCOMPARE(audit::storyOf(*headedPage, heads[0]).split(QLatin1Char('\n')), shown);
}


// The texts a form field shows have their character references decoded, the numeric ones too (&#233; and &#xE9;); a drop-down's entries are the entries of the list, not an element
// whose name only begins as one.
void WriteTests::aFormFieldsTextsHaveTheirCharacterReferencesDecoded()
{
    auto field = [](const QByteArray &data, const QByteArray &code) {
        return "<w:p><w:r><w:fldChar w:fldCharType=\"begin\"><w:ffData><w:name w:val=\"F\"/>" + data + "</w:ffData></w:fldChar></w:r><w:r><w:instrText xml:space=\"preserve\"> " + code +
               " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r></w:p>";
    };
    const QByteArray text = "<w:textInput><w:default w:val=\"Caf&#233; &#xE9;t&#xE9; &amp; Co &#x1F600;\"/></w:textInput>";
    const QByteArray drop = "<w:ddList><w:result w:val=\"1\"/><w:listEntryNote w:val=\"Not an entry\"/><w:listEntry w:val=\"Alpha\"/><w:listEntry w:val=\"Ren&#233;e &#x263A;\"/></w:ddList>";
    auto doc = open(makeDocxWithParts(field(text, "FORMTEXT") + field(drop, "FORMDROPDOWN"), sectionXml(6000, 6000, 720), {}));
    QVERIFY(doc);
    QCOMPARE(boxLines(*doc), (QStringList{QString::fromUtf8("Caf\xc3\xa9 \xc3\xa9t\xc3\xa9 & Co \xf0\x9f\x98\x80"), QString::fromUtf8("Ren\xc3\xa9""e \xe2\x98\xba")}));
    QVERIFY(documentXml(doc->toDocx()).contains(text) && documentXml(doc->toDocx()).contains(drop));
}


// A frame that is aligned to the outside or the inside of the margin (w:xAlign) is set by the page's number: a paragraph of a header or a footer so aligned is marked to be (it is
// not at one side for all pages), and the frame of the text is an object aligned to the inside or the outside.
void WriteTests::aFrameAlignedInsideOrOutsideFollowsThePageNumber()
{
    for (const char *side : {"outside", "inside"}) {
        const QByteArray frame = QByteArray("<w:framePr w:w=\"1000\" w:h=\"400\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"margin\" w:xAlign=\"") + side + "\" w:y=\"1\"/>";
        const QMap<QString, QByteArray> parts{{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:pPr>" + frame + "</w:pPr><w:r><w:t>5</w:t></w:r>"})}};
        auto doc = open(makeDocxWithParts("<w:p><w:pPr>" + frame + "</w:pPr><w:r><w:t>FRAME</w:t></w:r></w:p><w:p><w:r><w:t>body</w:t></w:r></w:p>",
                                          sectionXml(6000, 6000, 720, "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>"), parts));
        QVERIFY(doc);
        const auto page = doc->pager().materialize(0);
        QVERIFY(page);
        const auto heads = audit::itemsWith(*page, QStringLiteral("flow-header-box"));
        QVERIFY(heads.size() == 1);
        const auto *box = dynamic_cast<const TextItem *>(heads[0]);
        QVERIFY(box);
        const QTextBlockFormat bf = page->doc->story(box->storyId)->doc->begin().blockFormat();
        QCOMPARE(bf.intProperty(tp::PageSideAlign), std::string_view(side) == "outside" ? 1 : 2);
        QVERIFY2(!bf.hasProperty(QTextFormat::BlockAlignment) || bf.alignment() == Qt::AlignLeft, side);
        bool aligned = false;
        for (const flow::FloatingObject &o : doc->flow().objects)
            if (o.frame > 0) aligned = aligned || o.hAlign == (std::string_view(side) == "outside" ? flow::FloatingObject::Align::Outside : flow::FloatingObject::Align::Inside);
        QVERIFY2(aligned, side);
    }
}


// A content control shows its content wherever it stands (round runs, round paragraphs, inside a cell, round a cell, round a row), its placeholder text where it shows one, and is
// written back as it was.
void WriteTests::contentControlsShowTheirContentInEveryPlace()
{
        auto control = [](const QByteArray &props, const QByteArray &content) { return "<w:sdt><w:sdtPr>" + props + "</w:sdtPr><w:sdtContent>" + content + "</w:sdtContent></w:sdt>"; };
        auto para = [](const QByteArray &text) { return "<w:p><w:r><w:t>" + text + "</w:t></w:r></w:p>"; };
        const QByteArray cellPr = "<w:tcPr><w:tcW w:w=\"2000\" w:type=\"dxa\"/></w:tcPr>";
        const QByteArray table = "<w:tbl><w:tblPr/><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid>"
                                 "<w:tr>" + control("<w:alias w:val=\"around a cell\"/>", "<w:tc>" + cellPr + para("AROUNDCELL") + "</w:tc>") +
                                 "<w:tc>" + cellPr + control("<w:id w:val=\"5\"/>", para("INSIDECELL")) + "</w:tc></w:tr>" +
                                 control("<w:id w:val=\"6\"/>", "<w:tr><w:tc>" + cellPr + para("AROUNDROW") + "</w:tc><w:tc>" + cellPr + "<w:p><w:r><w:t>ROWEND</w:t></w:r></w:p></w:tc></w:tr>") + "</w:tbl>";
        const QByteArray body = "<w:p><w:r><w:t xml:space=\"preserve\">Name: </w:t></w:r>" + control("<w:showingPlcHdr/><w:text/>", "<w:r><w:rPr><w:rStyle w:val=\"PlaceholderText\"/></w:rPr><w:t>PLACEHOLDER</w:t></w:r>") +
                                "<w:r><w:t xml:space=\"preserve\"> end</w:t></w:r></w:p>" + control("<w:id w:val=\"7\"/>", para("AROUNDPARA")) + table + "<w:p><w:r><w:t>last</w:t></w:r></w:p>";
        auto doc = open(makeDocxWithParts(body, sectionXml(9000, 9000, 720), {}));
        QVERIFY(doc);
        QString story = doc->story()->toPlainText();
        story.remove(QChar::ObjectReplacementCharacter);   // (a control's start and end are atoms in the text)
        QVERIFY2(story.contains(QLatin1String("Name: PLACEHOLDER end")), qPrintable(story));
        for (const char *word : {"AROUNDPARA", "AROUNDCELL", "INSIDECELL", "AROUNDROW", "ROWEND"}) QVERIFY2(story.contains(QLatin1String(word)), word);
        const QByteArray saved = documentXml(doc->toDocx());
        QCOMPARE(saved.count("<w:sdt>"), 5);
        for (const char *tag : {"<w:showingPlcHdr/>", "<w:alias w:val=\"around a cell\"/>", "<w:id w:val=\"5\"/>", "<w:id w:val=\"6\"/>", "<w:id w:val=\"7\"/>"}) QVERIFY2(saved.contains(tag), tag);
        // (The same in a text box and a header.)
        const QByteArray box = "<w:p><w:r><w:t>text</w:t></w:r><w:r><w:pict " + QByteArray(kVml) + "><v:shape style=\"position:absolute;margin-left:10pt;margin-top:60pt;width:150pt;height:200pt;z-index:3\">"
                               "<v:textbox><w:txbxContent>" + control("<w:id w:val=\"7\"/>", para("AROUNDPARA")) + "<w:p><w:r><w:t xml:space=\"preserve\">Name: </w:t></w:r>" +
                               control("<w:showingPlcHdr/>", "<w:r><w:t>PLACEHOLDER</w:t></w:r>") + "</w:p></w:txbxContent></v:textbox><w10:wrap type=\"square\"/></v:shape></w:pict></w:r></w:p>";
        auto boxed = open(makeDocxWithParts(box, sectionXml(6000, 9000, 720), {}));
        QVERIFY(boxed);
        const auto page = boxed->pager().materialize(0);
        QVERIFY(page);
        QVERIFY2(boxWithText(*page, QStringLiteral("Name: PLACEHOLDER")), "the control's placeholder in the text box");
        QVERIFY2(boxWithText(*page, QStringLiteral("AROUNDPARA")), "the control round a paragraph in the text box");
    }


// A file with facing pages (w:mirrorMargins in its settings) has the margins of its even pages swapped: the section's left margin is its inside, so the text of page two begins at
// the right margin's width; a file without has every page alike.
void WriteTests::facingPagesMirrorTheMarginsOfEvenPages()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"800\" w:bottom=\"1440\" w:left=\"2000\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray body = "<w:p><w:r><w:t>One</w:t></w:r></w:p><w:p><w:r><w:br w:type=\"page\"/></w:r></w:p><w:p><w:r><w:t>Two</w:t></w:r></w:p>";
    for (const bool mirrored : {false, true}) {
        auto doc = open(makeDocxWithParts(body, sect, {}, mirrored ? QByteArray("<w:mirrorMargins/>") : QByteArray()));
        QVERIFY(doc);
        QCOMPARE(doc->pager().pageCount(), 2);
        for (int page = 0; page < 2; ++page) {
            const auto m = doc->pager().materialize(page);
            QVERIFY(m && !m->boxes.isEmpty());
            const auto *box = dynamic_cast<const TextItem *>(m->doc->item(m->boxes[0]));
            QVERIFY(box);
            const double left = mirrored && page == 1 ? 40.0 : 100.0;
            QVERIFY2(std::abs(box->rect.left() - left) < 0.01 && std::abs(box->rect.right() - (612 - (mirrored && page == 1 ? 100.0 : 40.0))) < 0.01,
                     qPrintable(QStringLiteral("%1 page %2: the text is from %3 to %4").arg(mirrored ? "mirrored" : "plain").arg(page + 1).arg(box->rect.left()).arg(box->rect.right())));
        }
        QVERIFY(documentXml(doc->toDocx()).contains("w:left=\"2000\""));
    }
}


// A run's display style makes its letters italic and bold where the layout is a word processor's, and is nothing in any other.
void WriteTests::aDisplayStyleSetsTheFaceOfAWordRun()
{
    QTextCharFormat f;
    f.setFontFamilies({QStringLiteral("Arimo")});
    f.setFontPointSize(10);
    auto resolved = [&](const QString &style, bool word) {
        QTextCharFormat g = f;
        g.setProperty(tp::DisplayStyle, style);
        LayoutEnv env;
        env.wordLines = word;
        return resolveCharFormat(g, env);
    };
    QVERIFY(resolved(QStringLiteral("i"), true).fontItalic());
    QVERIFY(!resolved(QStringLiteral("i"), true).fontWeight() || resolved(QStringLiteral("i"), true).fontWeight() < QFont::DemiBold);
    QVERIFY(resolved(QStringLiteral("b"), true).fontWeight() >= QFont::DemiBold);
    QVERIFY(!resolved(QStringLiteral("b"), true).fontItalic());
    QVERIFY(resolved(QStringLiteral("ib"), true).fontItalic() && resolved(QStringLiteral("ib"), true).fontWeight() >= QFont::DemiBold);
    QVERIFY(!resolved(QStringLiteral("ib"), false).fontItalic() && resolved(QStringLiteral("ib"), false).fontWeight() < QFont::DemiBold);
}


// An absolute position tab (w:ptab) in a footer takes the text after it to the margin's right edge whatever stops the paragraph has (W179's footer: its name, then "Page 14 of 14"
// at the margin, which ran together without a tab); one in the text is drawn the same and is written back as it was.
void WriteTests::anAbsolutePositionTabGoesToTheMarginAndIsKept()
{
    const QByteArray ptab = "<w:r><w:ptab w:relativeTo=\"margin\" w:alignment=\"right\" w:leader=\"none\"/></w:r>";
    const QByteArray stops = "<w:pPr><w:tabs><w:tab w:val=\"center\" w:pos=\"1500\"/><w:tab w:val=\"right\" w:pos=\"3000\"/></w:tabs></w:pPr>";
    const QMap<QString, QByteArray> parts{{QStringLiteral("/word/footer1.xml"), headerPart("ftr", {stops + "<w:r><w:t>Left side</w:t></w:r>" + ptab + "<w:r><w:t>Page 3</w:t></w:r>"})}};
    auto doc = open(makeDocxWithParts("<w:p><w:r><w:t>body</w:t></w:r>" + ptab + "<w:r><w:t>end</w:t></w:r></w:p>",
                                      sectionXml(6000, 6000, 720, "<w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/>"), parts));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    const auto feet = audit::itemsWith(*page, QStringLiteral("flow-footer-box"));
    QVERIFY(feet.size() == 1);
    const QTextDocument *footer = page->doc->story(dynamic_cast<const TextItem *>(feet[0])->storyId)->doc.get();
    const QString text = footer->toPlainText();
    QCOMPARE(text, QStringLiteral("Left side\tPage 3"));
    FrameSpec fs;
    fs.size = QSizeF(228, 200);   // (the text area: 6000 twentieths of a point less the margins)
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.hyphenate = false;
    StoryLayout layout;
    layout.build(footer, {fs}, doc->flow().env);
    const QVector<QRectF> shown = layout.rangeRects(0, int(text.indexOf(QLatin1String("Page"))), int(text.size()));
    QVERIFY(!shown.isEmpty());
    QVERIFY2(std::abs(shown.last().right() - 228) < 0.5, qPrintable(QStringLiteral("the footer's text ends at %1, the margin is at 228").arg(shown.last().right())));
    QVERIFY(documentXml(doc->toDocx()).contains("<w:ptab w:relativeTo=\"margin\" w:alignment=\"right\" w:leader=\"none\"/>"));
}


// The endnotes are set after the document's last text, the file's separator for them first and then the notes its references name, each with its number: they are paragraphs of
// the pages' story that are not the document's own (no edit reaches them, saving leaves them out, and the file's part is kept as it was).
void WriteTests::theEndnotesAreSetAfterTheLastTextAndAreNotWritten()
{
    const QByteArray body = "<w:p><w:r><w:t>First</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p><w:p><w:r><w:t>Second</w:t></w:r><w:r><w:endnoteReference w:id=\"3\"/></w:r></w:p>";
    const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:t>Sources</w:t></w:r></w:p></w:endnote>"
                             "<w:endnote w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:t>Continued</w:t></w:r></w:p></w:endnote>"
                             "<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Alpha note</w:t></w:r></w:p></w:endnote>"
                             "<w:endnote w:id=\"3\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Beta note</w:t></w:r></w:p></w:endnote></w:endnotes>";
    const QByteArray zip = makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>");
    auto doc = open(zip);
    QVERIFY(doc);
    QCOMPARE(int(doc->story()->blockCount()), 5);   // (two paragraphs, the separator, and two notes)
    // What the first page shows, line by line.
    const auto page = doc->pager().materialize(0);
    QVERIFY(page && !page->boxes.isEmpty());
    const auto *box = dynamic_cast<const TextItem *>(page->doc->item(page->boxes[0]));
    QVERIFY(box);
    FrameSpec fs;
    fs.size = box->rect.size();
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.hyphenate = false;
    StoryLayout layout;
    layout.build(page->doc->story(box->storyId)->doc.get(), {fs}, doc->flow().env);
    QStringList lines;
    for (const auto &l : layout.lineInfo(0)) lines << l.text.trimmed();
    QCOMPARE(lines, (QStringList{QStringLiteral("First1"), QStringLiteral("Second2"), QStringLiteral("Sources"), QStringLiteral("1 Alpha note"), QStringLiteral("2 Beta note")}));
    // Delete at the end of the text does not join the first of the notes to it; the end of the story is the end of the last note, and what is typed there is that note's.
    const QString before = doc->story()->toPlainText();
    QTextCursor c(doc->story());
    c.setPosition(doc->story()->findBlockByNumber(1).position() + doc->story()->findBlockByNumber(1).length() - 1);
    deleteForward(*doc, c, false);
    QCOMPARE(doc->story()->toPlainText(), before);
    c.movePosition(QTextCursor::End);
    insertText(*doc, c, QStringLiteral("x"));
    QCOMPARE(doc->story()->toPlainText(), before + QStringLiteral("x"));
    QCOMPARE(savedBody(*doc), QStringLiteral("First|Second"));
    QVERIFY(notesPartOf(doc->toDocx()).contains("Beta notex"));
    doc->undo();
    QCOMPARE(doc->story()->toPlainText(), before);
    // Saved, the document has its two paragraphs, and the endnotes part is as the file had it; read again, it shows the same.
    const QByteArray saved = doc->toDocx();
    const QByteArray written = documentXml(saved);
    QVERIFY(written.contains("First") && written.contains("Second"));
    QVERIFY(!written.contains("Sources") && !written.contains("Alpha note"));
    office::Package p;
    p.read(saved);
    QVERIFY(p.part(QStringLiteral("/word/endnotes.xml")).contains("Alpha note"));
    auto again = open(saved);
    QVERIFY(again);
    QCOMPARE(again->story()->toPlainText(), before);
}


// A long run of endnotes is all set (more than a header's or a footer's paragraphs are), and no more than the most the program sets of a hostile file.
void WriteTests::aLongRunOfEndnotesIsSetInFull()
{
    QByteArray body = "<w:p><w:r><w:t>Text</w:t></w:r>", notes = "<w:endnotes " + QByteArray(kNs) + ">";
    for (int i = 1; i <= 450; ++i) {
        body += "<w:r><w:endnoteReference w:id=\"" + QByteArray::number(i) + "\"/></w:r>";
        notes += "<w:endnote w:id=\"" + QByteArray::number(i) + "\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Note " + QByteArray::number(i) + "</w:t></w:r></w:p></w:endnote>";
    }
    body += "</w:p>";
    notes += "</w:endnotes>";
    auto doc = open(makeDocxWithParts(body, QByteArray(), {{QStringLiteral("/word/endnotes.xml"), notes}}));
    QVERIFY(doc);
    QCOMPARE(int(doc->story()->blockCount()), 451);
    QVERIFY(doc->story()->toPlainText().contains(QLatin1String("Note 450")));
}


// A section whose page numbers carry the chapter's (w:chapStyle of its w:pgNumType) shows "chapter-page": the number of the last heading of that level on the page (else the
// nearest before it), the separator the file says (a hyphen when it says none), and the page's own number.
void WriteTests::aPageNumberCanCarryTheChaptersNumber()
{
    auto heading = [](const char *text, bool newPage) {
        return QByteArray("<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/>") + (newPage ? "<w:pageBreakBefore/>" : "") + "<w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>" + text + "</w:t></w:r></w:p>";
    };
    const QByteArray body = heading("One", false) + plainP("alpha") + heading("Two", true) + plainP("beta") + heading("Three", false) + "<w:p><w:r><w:t>gamma</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r></w:p>" + plainP("tail");
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/></w:lvl></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
    const QMap<QString, QByteArray> parts{{QStringLiteral("/word/footer1.xml"), headerPart("ftr", {"<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>"})}};
    for (const auto &[pgNum, want] : {std::pair<const char *, QStringList>{"<w:pgNumType w:chapStyle=\"1\"/>", {QStringLiteral("1-1"), QStringLiteral("3-2"), QStringLiteral("3-3")}},
                                      std::pair<const char *, QStringList>{"<w:pgNumType w:chapStyle=\"1\" w:chapSep=\"colon\"/>", {QStringLiteral("1:1"), QStringLiteral("3:2"), QStringLiteral("3:3")}},
                                      std::pair<const char *, QStringList>{"", {QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3")}}}) {
        auto doc = open(makeDocxWithParts(body, sectionXml(6000, 4500, 720, "<w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/>", QByteArray(), pgNum), parts, QByteArray(), kStyles, numbering));
        QVERIFY(doc);
        flow::Paginator &pg = doc->pager();
        QCOMPARE(pg.pageCount(), 3);
        for (int k = 0; k < 3; ++k) {
            const auto page = pg.materialize(k);
            QVERIFY(page);
            const auto feet = audit::itemsWith(*page, QStringLiteral("flow-footer-box"));
            QVERIFY(feet.size() == 1);
            FrameSpec fs;
            fs.size = QSizeF(228, 40);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            fs.ctx.doc = page->doc.get();
            fs.ctx.pageNumber = 1;
            StoryLayout layout;
            layout.build(page->doc->story(dynamic_cast<const TextItem *>(feet[0])->storyId)->doc.get(), {fs}, doc->flow().env);
            const auto lines = layout.lineInfo(0);
            QVERIFY(!lines.isEmpty());
            QCOMPARE(lines[0].text.trimmed(), want[k]);
        }
    }
}


// A column break (w:br w:type="column") sends what follows it to the next column of the section, or the next page where there is one column, and is written back as it was.
void WriteTests::aColumnBreakIsAColumnBreakAndIsKept()
{
    const QByteArray body = plainP("one") + "<w:p><w:r><w:t>two</w:t></w:r><w:r><w:br w:type=\"column\"/></w:r><w:r><w:t>three</w:t></w:r></w:p>" + plainP("four");
    auto columns = open(makeDocxWithParts(body, sectionXml(12240, 15840, 1440, QByteArray(), QByteArray(), "<w:cols w:num=\"2\" w:space=\"720\"/>"), {}));
    QVERIFY(columns);
    QCOMPARE(columns->pager().pageCount(), 1);
    const auto lines = columns->pager().lines(0);
    QCOMPARE(int(lines.size()), 4);
    QVERIFY2(lines[0].frame == lines[1].frame && lines[2].frame == lines[1].frame + 1 && lines[3].frame == lines[2].frame,
             qPrintable(QStringLiteral("the lines are in the columns %1, %2, %3, %4").arg(lines[0].frame).arg(lines[1].frame).arg(lines[2].frame).arg(lines[3].frame)));
    QVERIFY(documentXml(columns->toDocx()).contains("<w:br w:type=\"column\"/>"));
    auto single = open(makeDocxWithParts(body, sectionXml(12240, 15840, 1440), {}));
    QVERIFY(single);
    QCOMPARE(single->pager().pageCount(), 2);
}


// The text of a box a file draws is set in the box less its insets, whatever the width of the box's line: the line does not take room from the text (a box 27 points high with
// insets of 3.6 holds two lines of 8-point text, as it does where the file was made; the program took half the line's width from each side, and cut off the second line).
void WriteTests::aBoxesLineDoesNotTakeRoomFromItsText()
{
    // A box whose text is a few lines of 8-point text, `height` points high, with a 6-point line and insets of 3.6 above and below; the item of it on the page, and the page.
    auto boxed = [&](double height, std::unique_ptr<WriteDoc> *keep, std::unique_ptr<flow::MaterializedPage> *page) -> const TextItem * {
        const QByteArray box = "<w:p><w:r><w:pict " + QByteArray(kVml) + "><v:rect id='b' style='position:absolute;margin-left:72pt;margin-top:72pt;width:100pt;height:" + QByteArray::number(height) +
                               "pt' strokeweight='6pt' filled='f'><v:textbox inset='7.2pt,3.6pt,7.2pt,3.6pt'><w:txbxContent><w:p><w:r><w:rPr><w:sz w:val=\"16\"/></w:rPr>"
                               "<w:t>Some text of the box and more of it</w:t></w:r></w:p></w:txbxContent></v:textbox></v:rect></w:pict></w:r></w:p>";
        *keep = open(makeDocxWithParts(box, sectionXml(12240, 15840, 1440), {}));
        if (!*keep) return nullptr;
        *page = (*keep)->pager().materialize(0);
        if (!*page) return nullptr;
        for (const auto &item : (*page)->doc->pages[0]->items)
            if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
                if (const auto st = (*page)->doc->story(t->storyId); st && st->doc->toPlainText().contains(QLatin1String("Some text"))) return t;
        return nullptr;
    };
    std::unique_ptr<WriteDoc> tallDoc, fitDoc;
    std::unique_ptr<flow::MaterializedPage> tall, fit;
    const TextItem *big = boxed(200, &tallDoc, &tall);
    QVERIFY(big);
    QVERIFY2(big->stroke.width >= 5, "the box has its line");
    // The height its lines need, with the insets: a box of that height and a line of 6 points holds them (the line was half of it each side from the insets, and one line fell out).
    StoryLayout need;
    need.build(tall->doc->story(big->storyId)->doc.get(), {Renderer::frameSpec(*tall->doc, *big, 1, RenderOptions())}, tallDoc->flow().env);
    const auto wanted = need.lineInfo(0);
    QVERIFY(wanted.size() >= 2);
    const double height = wanted.last().rect.bottom() + big->insets.top() + big->insets.bottom() + 0.5;
    const TextItem *found = boxed(height, &fitDoc, &fit);
    QVERIFY(found);
    const FrameSpec spec = Renderer::frameSpec(*fit->doc, *found, 1, RenderOptions());
    QCOMPARE(spec.insets, found->insets);
    StoryLayout lay;
    lay.build(fit->doc->story(found->storyId)->doc.get(), {spec}, fitDoc->flow().env);
    QCOMPARE(int(lay.lineBriefs().size()), int(wanted.size()));
    for (const auto &l : lay.lineBriefs()) QVERIFY2(l.frame == 0, "every line is in the box");
}


// A word processor's document has the line of a drawn box and of a drawn shape over its edge, taking no room from the text; a publication's keeps half the line's width on each side
// (the text of a box and of a shape both).
void WriteTests::aBoxesAndAShapesLineTakeRoomFromTextOnlyInAPublication()
{
    auto d = Document::blank(QSizeF(612, 792), QString(), 1);
    auto shape = std::make_shared<ShapeItem>();
    shape->rect = QRectF(100, 100, 100, 60);
    shape->insets = QMarginsF(0, 0, 0, 0);
    shape->stroke = Stroke::line(ColorRef::rgb(Qt::black), 10);
    shape->storyId = d->createStory();
    d->storyDoc(shape->storyId)->setPlainText(QStringLiteral("alpha beta"));
    d->pages[0]->items.push_back(shape);
    auto box = std::make_shared<TextItem>();
    box->rect = QRectF(300, 100, 100, 60);
    box->insets = QMarginsF(2, 3, 4, 5);
    box->stroke = Stroke::line(ColorRef::rgb(Qt::black), 10);
    box->storyId = d->createStory();
    d->storyDoc(box->storyId)->setPlainText(QStringLiteral("alpha beta"));
    d->pages[0]->items.push_back(box);
    LayoutCache cache;
    PaintContext ctx;
    ctx.doc = d.get();
    ctx.cache = &cache;
    for (const bool word : {true, false}) {
        d->wordText = word;
        cache.clear();
        const double shift = word ? 0.0 : 5.0;   // (half of the line's 10 points)
        QPointF origin;
        const StoryLayout *shown = Renderer::shapeTextLayout(ctx, *shape, &origin);
        QVERIFY(shown);
        QVERIFY2(std::abs(shown->lineInfo(0).first().rect.left() - shift) < 0.5, qPrintable(QStringLiteral("a shape's text begins at %1, %2 word processor").arg(shown->lineInfo(0).first().rect.left()).arg(word ? "a" : "no")));
        const FrameSpec spec = Renderer::frameSpec(*d, *box, 1, RenderOptions());
        QCOMPARE(spec.insets, box->insets + QMarginsF(shift, shift, shift, shift));
    }
}


// A file with more endnotes than the program sets has the first of them set and the rest left out (the references show their numbers): a hostile file's cost is bounded.
void WriteTests::anEndlessRunOfEndnotesIsBounded()
{
    QByteArray body = "<w:p><w:r><w:t>Text</w:t></w:r>", notes = "<w:endnotes " + QByteArray(kNs) + ">";
    for (int i = 1; i <= 5100; ++i) {
        body += "<w:r><w:endnoteReference w:id=\"" + QByteArray::number(i) + "\"/></w:r>";
        notes += "<w:endnote w:id=\"" + QByteArray::number(i) + "\"><w:p><w:r><w:t>N" + QByteArray::number(i) + "</w:t></w:r></w:p></w:endnote>";
    }
    body += "</w:p>";
    notes += "</w:endnotes>";
    auto doc = open(makeDocxWithParts(body, QByteArray(), {{QStringLiteral("/word/endnotes.xml"), notes}}));
    QVERIFY(doc);
    QCOMPARE(int(doc->story()->blockCount()), 5001);
    QVERIFY(doc->story()->toPlainText().contains(QLatin1String("N5000")) && !doc->story()->toPlainText().contains(QLatin1String("N5001")));
}


// ---- the endnotes the pages set after the text are outside what is edited
// A document of two paragraphs, the second citing two endnotes, and their part (a separator "Sources"; the notes "Alpha body note brwon" and "Beta body note"): its package.
QByteArray WriteTests::notedDocx()
{
    const QByteArray body = "<w:p><w:r><w:t>First body paragraph</w:t></w:r></w:p><w:p><w:r><w:t>Second body paragraph</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r><w:r><w:endnoteReference w:id=\"3\"/></w:r></w:p>";
    const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:t>Sources</w:t></w:r></w:p></w:endnote>"
                             "<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Alpha body note brwon</w:t></w:r></w:p></w:endnote>"
                             "<w:endnote w:id=\"3\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Beta body note</w:t></w:r></w:p></w:endnote></w:endnotes>";
    return makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>");
}

// The paragraphs of the document as it is saved, their text joined by "|".
QString WriteTests::savedBody(WriteDoc &doc)
{
    const QString x = QString::fromUtf8(documentXml(doc.toDocx()));
    static const QRegularExpression para(QStringLiteral("<w:p[ >].*?</w:p>|<w:p/>")), text(QStringLiteral("<w:t[^>]*>([^<]*)</w:t>"));
    QStringList out;
    for (auto m = para.globalMatch(x); m.hasNext();) {
        QString t;
        for (auto n = text.globalMatch(m.next().captured()); n.hasNext();) t += n.next().captured(1);
        out << t;
    }
    return out.join(QLatin1Char('|'));
}

QByteArray WriteTests::notesPartOf(const QByteArray &zip)
{
    office::Package p;
    p.read(zip);
    return p.part(QStringLiteral("/word/endnotes.xml"));
}

// How many paragraphs of the story are the endnotes', and whether they are all the last.
int WriteTests::tailOf(WriteDoc &doc, bool *allLast)
{
    int marked = 0;
    bool afterTail = false, last = true;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) {
        const bool is = b.blockFormat().hasProperty(prop::EndnoteBlock);
        marked += is;
        if (is) afterTail = true;
        else if (afterTail) last = false;
    }
    if (allLast) *allLast = last;
    return marked;
}


// Select All and typing (or Delete), and a selection from a paragraph's start to the end of the story, change the text the person has and not the endnotes: the selection ends
// where the text does. What is typed goes in the text, the text is saved whole, and the endnotes part is as it was.
void WriteTests::selectingAllAndTypingChangesOnlyTheText()
{
    const QByteArray zip = notedDocx();
    auto doc = open(zip);
    QVERIFY(doc);
    QCOMPARE(tailOf(*doc), 3);
    QTextCursor c(doc->story());
    c.select(QTextCursor::Document);
    insertText(*doc, c, QStringLiteral("X"));
    QCOMPARE(savedBody(*doc), QStringLiteral("X"));
    insertText(*doc, c, QStringLiteral("Y"));
    QCOMPARE(savedBody(*doc), QStringLiteral("XY"));   // (typing goes on)
    // (The selection ended where the text does, and the references that were in it went with it: their notes are gone from the pages and the file, and an undo brings them back.)
    QCOMPARE(tailOf(*doc), 0);
    QVERIFY(!notesPartOf(doc->toDocx()).contains("Alpha"));
    while (doc->canUndo()) doc->undo();
    QCOMPARE(tailOf(*doc), 3);
    QCOMPARE(savedBody(*doc), QStringLiteral("First body paragraph|Second body paragraph"));
    QCOMPARE(notesPartOf(doc->toDocx()), notesPartOf(zip));
    // From the start of the last paragraph to the end of the story.
    auto again = open(zip);
    QVERIFY(again);
    QTextCursor d(again->story());
    d.setPosition(again->story()->findBlockByNumber(1).position());
    d.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    removeSelection(*again, d);
    QCOMPARE(savedBody(*again), QStringLiteral("First body paragraph|"));
    insertText(*again, d, QStringLiteral("Q"));
    QCOMPARE(savedBody(*again), QStringLiteral("First body paragraph|Q"));
    QCOMPARE(tailOf(*again), 0);
}


// A selection from the middle of the last paragraph into the middle of an endnote, deleted, takes the text's part of it away and not the rest of the endnote's, which is not joined to the
// text and saved there.
void WriteTests::aSelectionIntoAnEndnoteIsDeletedAsFarAsTheTextGoes()
{
    const QByteArray body = "<w:p><w:r><w:t>First</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p><w:p><w:r><w:t>Second paragraph</w:t></w:r></w:p>";
    const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:t>Sources</w:t></w:r></w:p></w:endnote>"
                             "<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Alpha body note brwon</w:t></w:r></w:p></w:endnote></w:endnotes>";
    const QByteArray zip = makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>");
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    QCOMPARE(tailOf(*doc), 2);
    QTextCursor c(story);
    c.setPosition(story->findBlockByNumber(1).position() + 6);   // (after "Second")
    c.setPosition(story->findBlockByNumber(3).position() + 8, QTextCursor::KeepAnchor);
    removeSelection(*doc, c);
    QCOMPARE(savedBody(*doc), QStringLiteral("First|Second"));
    QCOMPARE(tailOf(*doc), 2);
    QVERIFY2(story->toPlainText().contains(QLatin1String(" Alpha body note brwon")), "the note is whole");
    QCOMPARE(notesPartOf(doc->toDocx()), notesPartOf(zip));
}


// Typing, deleting, and making a paragraph where the caret is in the separator of the endnotes does nothing: the story and what is saved are as they were.
void WriteTests::typingInTheSeparatorOfTheEndnotesDoesNothing()
{
    const QByteArray zip = notedDocx();
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    const QString before = story->toPlainText(), body = savedBody(*doc);
    const QTextBlock separator = story->findBlockByNumber(2);
    QTextCursor at(story);
    at.setPosition(separator.position() + 3);
    insertText(*doc, at, QStringLiteral("ZZZ"));
    insertParagraph(*doc, at);
    deleteBack(*doc, at, false);
    deleteForward(*doc, at, false);
    QTextCursor word(story);   // (a word of it selected)
    word.setPosition(separator.position() + 1);
    word.setPosition(separator.position() + 4, QTextCursor::KeepAnchor);
    insertText(*doc, word, QStringLiteral("ZZZ"));
    QCOMPARE(story->toPlainText(), before);
    QCOMPARE(savedBody(*doc), body);
    QCOMPARE(notesPartOf(doc->toDocx()), notesPartOf(zip));
    QVERIFY(!doc->canUndo());
}


// The caret and the selection in the page view are held where the text ends: Ctrl+End, Select All, and a click or drag past it do not reach the endnotes, so what is typed there is typed
// at the end of the text, and Enter makes a paragraph there.
void WriteTests::theViewsCaretStaysInTheText()
{
    const QByteArray zip = notedDocx();
    auto doc = open(zip);
    QVERIFY(doc);
    PageView view;
    view.resize(800, 600);
    view.setDocument(doc.get());
    view.show();
    const int end = editableEnd(doc->story());
    QVERIFY(end > 0 && end < doc->story()->characterCount() - 1);
    view.typeKey(Qt::Key_End, Qt::ControlModifier);
    QCOMPARE(view.cursor().position(), end);
    view.insertText(QStringLiteral("end"));
    QCOMPARE(savedBody(*doc), QStringLiteral("First body paragraph|Second body paragraphend"));
    view.typeKey(Qt::Key_Return, Qt::NoModifier);
    view.insertText(QStringLiteral("n"));
    QCOMPARE(savedBody(*doc), QStringLiteral("First body paragraph|Second body paragraphend|n"));
    bool last = false;
    QCOMPARE(tailOf(*doc, &last), 3);
    QVERIFY(last);
    view.selectAll();
    QVERIFY(view.cursor().selectionEnd() <= editableEnd(doc->story()));
    QTextCursor beyond(doc->story());
    beyond.setPosition(0);
    beyond.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    view.setCursor(beyond);
    QVERIFY(view.cursor().position() <= editableEnd(doc->story()) && view.cursor().anchor() <= editableEnd(doc->story()));
    QCOMPARE(notesPartOf(doc->toDocx()), notesPartOf(zip));
}


// Where the text ends is where it ends after an edit, an undo, and another edit (the story's revision goes back with an undo, and is the same again with another edit: it does not say).
void WriteTests::whereTheTextEndsFollowsTheStoryThroughUndo()
{
    auto doc = open(notedDocx());
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    auto end = [&] { return story->findBlockByNumber(1).position() + story->findBlockByNumber(1).length() - 1; };   // (the end of the last paragraph of the text)
    const int first = end();
    QCOMPARE(editableEnd(story), first);
    QTextCursor c(story);
    c.setPosition(first);
    insertText(*doc, c, QStringLiteral("abc"));
    QCOMPARE(editableEnd(story), first + 3);
    story->undo(&c);
    QCOMPARE(editableEnd(story), first);
    c.setPosition(first);
    insertText(*doc, c, QStringLiteral("Z"));
    QCOMPARE(editableEnd(story), first + 1);
    QCOMPARE(savedBody(*doc), QStringLiteral("First body paragraph|Second body paragraphZ"));
}


// A copy that takes in the endnotes has only the text, a paste of a paragraph that was an endnote's is a paragraph of the text and not a marked one in the middle of it, and the
// endnotes stay the last paragraphs.
void WriteTests::aCopyLeavesTheEndnotesOutAndAPasteMakesNoneInTheText()
{
    const QByteArray zip = notedDocx();
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    QTextCursor c(story);
    c.select(QTextCursor::Document);
    const Clip clip = copySelection(*doc, c);
    const QString copied = plainTextOf(clip.fragment);
    QVERIFY2(copied.contains(QLatin1String("Second body paragraph")) && !copied.contains(QLatin1String("Alpha")) && !copied.contains(QLatin1String("Sources")), qPrintable(copied));
    QVERIFY(!plainTextOf(selectionFragment(c)).contains(QLatin1String("Beta")));
    QTextCursor at(story);
    at.setPosition(story->findBlockByNumber(1).position());
    pasteClip(*doc, at, clip);
    bool last = false;
    QCOMPARE(tailOf(*doc, &last), 5);   // (the pasted references cite copies of their notes: the separator, the two notes, and their copies)
    QVERIFY(last);
    QVERIFY(!savedBody(*doc).contains(QLatin1String("Alpha")) && !savedBody(*doc).contains(QLatin1String("Sources")));
    // A fragment of the endnotes themselves, pasted: its paragraphs are the text's.
    QTextCursor notes(story);
    notes.setPosition(story->findBlockByNumber(story->blockCount() - 3).position());
    notes.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    const QTextDocumentFragment endnotes(notes);
    QTextCursor into(story);
    into.setPosition(story->findBlockByNumber(1).position());
    pasteFragment(*doc, into, endnotes);
    QCOMPARE(tailOf(*doc, &last), 5);
    QVERIFY(last);
    QVERIFY(savedBody(*doc).contains(QLatin1String("Alpha body note")));   // (what was pasted is text, and is saved as such)
}


// Replace All and spelling see the text the person has: a word that is in an endnote too is changed in the text alone, and a misspelled word of an endnote is neither flagged nor replaced.
void WriteTests::findAndSpellingLeaveTheEndnotesAlone()
{
    const QByteArray zip = notedDocx();
    WriteWindow w;
    w.setDocument(open(zip), QString());
    w.show();
    WriteDoc &doc = *w.document();
    w.showFindPane(true);
    WriteFindPane *pane = w.findPane();
    pane->findBox()->setText(QStringLiteral("body"));
    pane->replaceBox()->setText(QStringLiteral("text"));
    pane->replaceAll();
    QCOMPARE(pane->status(), QStringLiteral("JeffWrite made 2 replacements."));
    QCOMPARE(savedBody(doc), QStringLiteral("First text paragraph|Second text paragraph"));
    QVERIFY2(doc.story()->toPlainText().contains(QLatin1String(" Alpha body note brwon")), "the endnote is as it was");
    QCOMPARE(notesPartOf(doc.toDocx()), notesPartOf(zip));
    // Spelling: the endnote's "brwon" is not flagged, and a replacement of it does nothing.
    SpellCheck sc(doc.story());
    QVERIFY(flaggedWords(sc, doc).isEmpty());
    const QString whole = doc.story()->toPlainText();
    const int at = int(whole.indexOf(QLatin1String("brwon")));
    QVERIFY(at > 0);
    replaceWord(doc.story(), at, at + 5, QStringLiteral("brown"));
    QCOMPARE(doc.story()->toPlainText(), whole);
}


// ---- endnotes at the end of each section
// A document of two sections (the first ends in "Second body"), a note cited in each, and the notes' part (a separator "Sources"; "Alpha body note brwon" and "Beta body note"): the
// sections' properties and the settings' say where the endnotes go (`first`, `last`, and `settings` are the XML of an endnotePr, or empty).
QByteArray WriteTests::sectionNotedDocx(const QByteArray &first, const QByteArray &last, const QByteArray &settings)
{
    const QByteArray body = "<w:p><w:r><w:t>First body</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>"
                            "<w:p><w:pPr>" + sectionXml(6000, 6000, 720, first) + "</w:pPr><w:r><w:t>Second body</w:t></w:r></w:p>"
                            "<w:p><w:r><w:t>Third body</w:t></w:r><w:r><w:endnoteReference w:id=\"3\"/></w:r></w:p>";
    const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:t>Sources</w:t></w:r></w:p></w:endnote>"
                             "<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Alpha body note brwon</w:t></w:r></w:p></w:endnote>"
                             "<w:endnote w:id=\"3\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Beta body note</w:t></w:r></w:p></w:endnote></w:endnotes>";
    return makeDocxWithParts(body, sectionXml(6000, 6000, 720, last), {{QStringLiteral("/word/endnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/>" + settings + "</w:endnotePr>");
}

// The text of each paragraph of the story, without the pieces kept as XML, trimmed.
QStringList WriteTests::storyParagraphs(WriteDoc &doc)
{
    QStringList out;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) out << b.text().remove(QChar::ObjectReplacementCharacter).trimmed();
    return out;
}


// Where a section's properties say its endnotes go (w:pos "sectEnd"), the notes its references name are set at the end of that section, each section's after its own text and the file's
// separator first, and not at the end of the document. They are paragraphs of the pages' story that the file does not have: the section's break is after the last of them in the story,
// and is written back on the paragraph of the file's that came before.
void WriteTests::endnotesAreSetAtTheEndOfEachSectionThatSaysSo()
{
    const QByteArray pr = "<w:endnotePr><w:pos w:val=\"sectEnd\"/></w:endnotePr>";
    const QByteArray zip = sectionNotedDocx(pr, pr);
    auto doc = open(zip);
    QVERIFY(doc);
    QCOMPARE(storyParagraphs(*doc), (QStringList{QStringLiteral("First body"), QStringLiteral("Second body"), QStringLiteral("Sources"), QStringLiteral("Alpha body note brwon"), QStringLiteral("Third body"),
                                                 QStringLiteral("Sources"), QStringLiteral("Beta body note")}));
    QList<bool> breaks, marked;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) {
        breaks << b.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
        marked << b.blockFormat().hasProperty(prop::EndnoteBlock);
    }
    QCOMPARE(breaks, (QList<bool>{false, false, false, true, false, false, false}));
    QCOMPARE(marked, (QList<bool>{false, false, true, true, false, true, true}));
    QCOMPARE(doc->sectionCount(), 2);
    // Each section's page ends with its own notes.
    QCOMPARE(boxLines(*doc, 0), (QStringList{QStringLiteral("First body1"), QStringLiteral("Second body"), QStringLiteral("Sources"), QStringLiteral("1 Alpha body note brwon")}));
    QCOMPARE(boxLines(*doc, 1), (QStringList{QStringLiteral("Third body2"), QStringLiteral("Sources"), QStringLiteral("2 Beta body note")}));
    // Saved: the paragraphs of the file, the first section's break on its last, and the notes' part as it was.
    QCOMPARE(savedBody(*doc), QStringLiteral("First body|Second body|Third body"));
    const QByteArray written = documentXml(doc->toDocx());
    QCOMPARE(written.count("<w:sectPr"), 2);
    const qsizetype second = written.indexOf("Second body"), at = written.indexOf("<w:sectPr");
    QVERIFY2(at > 0 && at < second, "the first section's break is in the paragraph of its last text");
    QCOMPARE(notesPartOf(doc->toDocx()), notesPartOf(zip));
    auto again = open(doc->toDocx());
    QVERIFY(again);
    QCOMPARE(storyParagraphs(*again), storyParagraphs(*doc));
    QCOMPARE(documentXml(again->toDocx()), written);
}


// The settings' w:pos is the sections' where they say none; a section that says the end of the document (docEnd) has its notes after the document's last text, with the other
// sections' that are there, in the order of their references; and a file that says nothing has all of them at the end, as before.
void WriteTests::whereTheEndnotesGoFollowsTheSettingsAndTheSection()
{
    const QByteArray sectEnd = "<w:pos w:val=\"sectEnd\"/>";
    auto paragraphsOf = [&](const QByteArray &first, const QByteArray &last, const QByteArray &settings) {
        auto doc = open(sectionNotedDocx(first, last, settings));
        return doc ? storyParagraphs(*doc) : QStringList{QStringLiteral("NO DOCUMENT")};
    };
    const QStringList atSections = {QStringLiteral("First body"), QStringLiteral("Second body"), QStringLiteral("Sources"), QStringLiteral("Alpha body note brwon"), QStringLiteral("Third body"),
                                    QStringLiteral("Sources"), QStringLiteral("Beta body note")};
    const QStringList atEnd = {QStringLiteral("First body"), QStringLiteral("Second body"), QStringLiteral("Third body"), QStringLiteral("Sources"), QStringLiteral("Alpha body note brwon"),
                               QStringLiteral("Beta body note")};
    QCOMPARE(paragraphsOf(QByteArray(), QByteArray(), sectEnd), atSections);   // (the settings')
    QCOMPARE(paragraphsOf(QByteArray(), QByteArray(), QByteArray()), atEnd);
    QCOMPARE(paragraphsOf(QByteArray(), QByteArray(), "<w:pos w:val=\"docEnd\"/>"), atEnd);
    QCOMPARE(paragraphsOf("<w:endnotePr><w:pos w:val=\"docEnd\"/></w:endnotePr>", QByteArray(), sectEnd), atEnd);   // (the first section's own says the end of the document)
    // (A document of one section has them at its end either way.)
    auto one = open(makeDocxWithParts("<w:p><w:r><w:t>Only</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>", sectionXml(6000, 6000, 720, "<w:endnotePr><w:pos w:val=\"sectEnd\"/></w:endnotePr>"),
                                      {{QStringLiteral("/word/endnotes.xml"), "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t> Note</w:t></w:r></w:p></w:endnote></w:endnotes>"}}));
    QVERIFY(one);
    QCOMPARE(storyParagraphs(*one), (QStringList{QStringLiteral("Only"), QStringLiteral("Note")}));
}


// Nothing types into, deletes, or joins the endnotes set at a section's end, and a selection that goes over them changes the text round them and not them: what is deleted is the
// text's alone (the paragraphs before and after them are not joined), and it is one step of undo.
void WriteTests::editingNextToTheEndnotesOfASectionLeavesThem()
{
    const QByteArray pr = "<w:endnotePr><w:pos w:val=\"sectEnd\"/></w:endnotePr>";
    const QByteArray zip = sectionNotedDocx(pr, pr);
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    const QString before = story->toPlainText();
    // In the separators: nothing happens (a caret in them, a word of one selected). The words of a note are the note's to edit (theWordsOfAnEndnoteAreEditedWhereTheyStand).
    for (int number : {2, 5}) {
        const QTextBlock b = story->findBlockByNumber(number);
        QTextCursor at(story);
        at.setPosition(b.position() + std::min(2, b.length() - 1));
        insertText(*doc, at, QStringLiteral("ZZZ"));
        insertParagraph(*doc, at);
        deleteBack(*doc, at, false);
        deleteForward(*doc, at, false);
        QCOMPARE(story->toPlainText(), before);
    }
    QTextCursor word(story);
    word.setPosition(story->findBlockByNumber(2).position() + 1);
    word.setPosition(story->findBlockByNumber(2).position() + 5, QTextCursor::KeepAnchor);
    insertText(*doc, word, QStringLiteral("ZZZ"));
    QCOMPARE(story->toPlainText(), before);
    // At the edges Delete and Backspace join the paragraphs round the notes and end the section's break (deleteAndBackspaceAtTheEndnotesOfASectionEndTheBreak).
    QTextCursor edge(story);
    // Typing at the end of the section's last text is typing in the text.
    edge.setPosition(story->findBlockByNumber(1).position() + story->findBlockByNumber(1).length() - 1);
    insertText(*doc, edge, QStringLiteral("!"));
    QCOMPARE(storyParagraphs(*doc).value(1), QStringLiteral("Second body!"));
    QCOMPARE(doc->sectionCount(), 2);
    story->undo();
    QCOMPARE(story->toPlainText(), before);
    // A selection from inside the first paragraph to inside the last: the text between goes, the notes stay where they are, and what is left on either side of them is not joined.
    QTextCursor over(story);
    over.setPosition(story->findBlockByNumber(1).position() + story->findBlockByNumber(1).length() - 1);   // (after the first section's last reference)
    over.setPosition(story->findBlockByNumber(4).position() + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, over);
    QCOMPARE(storyParagraphs(*doc), (QStringList{QStringLiteral("First body"), QStringLiteral("Second body"), QStringLiteral("Sources"), QStringLiteral("Alpha body note brwon"), QStringLiteral("hird body"),
                                                 QStringLiteral("Sources"), QStringLiteral("Beta body note")}));
    QCOMPARE(tailOf(*doc), 4);
    QCOMPARE(doc->sectionCount(), 2);
    QCOMPARE(savedBody(*doc), QStringLiteral("First body|Second body|hird body"));
    QCOMPARE(documentXml(doc->toDocx()).count("<w:sectPr"), 2);
    QCOMPARE(notesPartOf(doc->toDocx()), notesPartOf(zip));
    story->undo();
    QCOMPARE(story->toPlainText(), before);
}


// A caret and a selection are held out of the endnotes set at a section's end, a copy has only the text, Replace All and spelling see the text alone, and Select All with typing changes the
// text and not the notes.
void WriteTests::copyFindAndSpellingLeaveTheEndnotesOfASectionAlone()
{
    const QByteArray pr = "<w:endnotePr><w:pos w:val=\"sectEnd\"/></w:endnotePr>";
    const QByteArray zip = sectionNotedDocx(pr, pr);
    WriteWindow w;
    w.setDocument(open(zip), QString());
    w.show();
    WriteDoc &doc = *w.document();
    QTextDocument *story = doc.story();
    // The caret: a position in a note is held at the end of the paragraph before it.
    QTextCursor c(story);
    c.setPosition(story->findBlockByNumber(3).position() + 3);
    holdInBody(c);
    QCOMPARE(c.position(), story->findBlockByNumber(1).position() + story->findBlockByNumber(1).length() - 1);
    c.setPosition(0);
    c.setPosition(story->findBlockByNumber(3).position() + 3, QTextCursor::KeepAnchor);
    holdInBody(c);
    QCOMPARE(c.selectionEnd(), story->findBlockByNumber(1).position() + story->findBlockByNumber(1).length() - 1);
    // The copy of the whole story.
    c.select(QTextCursor::Document);
    const QString copied = plainTextOf(copySelection(doc, c).fragment);
    QVERIFY2(copied.contains(QLatin1String("First body")) && copied.contains(QLatin1String("Third body")) && !copied.contains(QLatin1String("Alpha")) && !copied.contains(QLatin1String("Sources"))
                 && !copied.contains(QLatin1String("Beta")), qPrintable(copied));
    QVERIFY(!plainTextOf(selectionFragment(c)).contains(QLatin1String("Alpha")));
    // Replace All.
    w.showFindPane(true);
    WriteFindPane *pane = w.findPane();
    pane->findBox()->setText(QStringLiteral("body"));
    pane->replaceBox()->setText(QStringLiteral("text"));
    pane->replaceAll();
    QCOMPARE(pane->status(), QStringLiteral("JeffWrite made 3 replacements."));
    QCOMPARE(savedBody(doc), QStringLiteral("First text|Second text|Third text"));
    QVERIFY2(story->toPlainText().contains(QLatin1String(" Alpha body note brwon")) && story->toPlainText().contains(QLatin1String(" Beta body note")), "the notes are as they were");
    QCOMPARE(notesPartOf(doc.toDocx()), notesPartOf(zip));
    // Spelling: the notes' "brwon" is neither flagged nor replaced.
    SpellCheck sc(story);
    QVERIFY(flaggedWords(sc, doc).isEmpty());
    const QString whole = story->toPlainText();
    const int at = int(whole.indexOf(QLatin1String("brwon")));
    QVERIFY(at > 0);
    replaceWord(story, at, at + 5, QStringLiteral("brown"));
    QCOMPARE(story->toPlainText(), whole);
    // Select All and typing: the text is replaced by what is typed, a paragraph to each stretch of text, and the notes and the saved part of them are as they were.
    QTextCursor everything(story);
    everything.select(QTextCursor::Document);
    write::insertText(doc, everything, QStringLiteral("Z"));
    QVERIFY(story->findBlockByNumber(0).text().startsWith(QLatin1Char('Z')));
    // (The references went with the text they were in, and so did their notes; an undo brings both back.)
    QCOMPARE(tailOf(doc), 0);
    QCOMPARE(doc.sectionCount(), 2);
    QVERIFY(!notesPartOf(doc.toDocx()).contains("Alpha"));
    doc.undo();
    QCOMPARE(tailOf(doc), 4);
    QVERIFY(story->toPlainText().contains(QLatin1String(" Alpha body note brwon")) && story->toPlainText().contains(QLatin1String(" Beta body note")));
    QCOMPARE(notesPartOf(doc.toDocx()), notesPartOf(zip));
}


// The caret passes the endnotes set at a section's end: from the last place of text of a section, Right, Down, Ctrl+Right, and Ctrl+Down go to the first place of the next section's
// text; from that first place, Left, Up, Ctrl+Left, and Ctrl+Up go to the last place of the section before; with Shift the selection is extended the same way; and a click in the
// notes puts the caret at the nearest place the person may type, the notes' first lines being nearer the text above them and their last the text below.
void WriteTests::theCaretPassesTheEndnotesOfASection()
{
    const QByteArray pr = "<w:endnotePr><w:pos w:val=\"sectEnd\"/></w:endnotePr>";
    auto doc = open(sectionNotedDocx(pr, pr));
    QVERIFY(doc);
    PageView view;
    view.resize(800, 700);
    view.setDocument(doc.get());
    view.show();
    view.paginateAll();
    QTextDocument *story = doc->story();
    const QTextBlock second = story->findBlockByNumber(1), third = story->findBlockByNumber(4);
    const int lastReal = second.position() + second.length() - 1, nextFirst = third.position();
    QVERIFY(story->findBlockByNumber(2).blockFormat().hasProperty(prop::EndnoteBlock) && story->findBlockByNumber(3).blockFormat().hasProperty(prop::EndnoteBlock));
    struct Key {
        int key;
        Qt::KeyboardModifiers modifiers;
        const char *name;
    };
    const Key forward[] = {{Qt::Key_Right, Qt::NoModifier, "Right"}, {Qt::Key_Down, Qt::NoModifier, "Down"}, {Qt::Key_Right, Qt::ControlModifier, "Ctrl+Right"}, {Qt::Key_Down, Qt::ControlModifier, "Ctrl+Down"}};
    const Key backward[] = {{Qt::Key_Left, Qt::NoModifier, "Left"}, {Qt::Key_Up, Qt::NoModifier, "Up"}, {Qt::Key_Left, Qt::ControlModifier, "Ctrl+Left"}, {Qt::Key_Up, Qt::ControlModifier, "Ctrl+Up"}};
    auto press = [&](int at, const Key &k, bool shift) {
        QTextCursor c(story);
        c.setPosition(at);
        view.setCursor(c);
        view.typeKey(k.key, k.modifiers | (shift ? Qt::ShiftModifier : Qt::NoModifier));
        return QPair<int, int>(view.cursor().anchor(), view.cursor().position());
    };
    auto expect = [&](int from, const Key &k, bool shift, QPair<int, int> want) {
        const QPair<int, int> got = press(from, k, shift);
        QVERIFY2(got == want, qPrintable(QStringLiteral("%1%2 from %3: anchor %4 and position %5, not %6 and %7").arg(shift ? "Shift+" : "", k.name).arg(from).arg(got.first).arg(got.second).arg(want.first).arg(want.second)));
    };
    for (const Key &k : forward) {
        expect(lastReal, k, false, {nextFirst, nextFirst});
        expect(lastReal, k, true, {lastReal, nextFirst});
    }
    for (const Key &k : backward) {
        expect(nextFirst, k, false, {lastReal, lastReal});
        expect(nextFirst, k, true, {nextFirst, lastReal});
    }
    // A click in the notes: the lines of the first page are the first section's two paragraphs, the separator, and the note.
    const auto page = doc->pager().materialize(0);
    QVERIFY(page && !page->boxes.isEmpty());
    const auto *item = dynamic_cast<const TextItem *>(page->doc->item(page->boxes[0]));
    QVERIFY(item);
    FrameSpec fs;
    fs.size = item->rect.size();
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.hyphenate = false;
    StoryLayout layout;
    layout.build(page->doc->story(item->storyId)->doc.get(), {fs}, doc->flow().env);
    const auto lines = layout.lineInfo(0);
    QVERIFY(lines.size() == 4);
    auto click = [&](int line) {
        const QRect pageRect = view.pageRect(0);
        const double scale = pageRect.width() / 300.0;
        const QRectF r = lines[line].rect.translated(item->rect.topLeft());
        QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(int(pageRect.left() + (r.left() + 20) * scale), int(pageRect.top() + r.center().y() * scale)));
        return view.cursor().position();
    };
    QCOMPARE(click(2), lastReal);   // (the separator: the first of the notes)
    // (A click in a note's words puts the caret in them: the notes' words are the person's to edit.)
    const int inNote = click(3);
    const EndnoteSpan span = endnoteSpanAt(story, inNote);
    QVERIFY2(span.valid && span.editable && inNote >= span.textFrom && inNote <= span.textTo, qPrintable(QString::number(inNote)));
}


// What is done to the characters of a selection that goes over the endnotes of a section reaches the text and not the notes: bold, italic, underline, a size, a color, a highlight, a
// face, super and subscript, a character style, and clearing the formatting; and a second Bold, over text that is all bold, turns it off (the notes' own looks are not counted).
void WriteTests::formattingASelectionLeavesTheEndnotesOfASectionAlone()
{
    const QByteArray pr = "<w:endnotePr><w:pos w:val=\"sectEnd\"/></w:endnotePr>";
    auto doc = open(sectionNotedDocx(pr, pr));
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    auto notes = [&] {
        QList<QTextCharFormat> out;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            if (b.blockFormat().hasProperty(prop::EndnoteBlock))
                for (auto it = b.begin(); !it.atEnd(); ++it) out << it.fragment().charFormat();
        return out;
    };
    const QList<QTextCharFormat> before = notes();
    QVERIFY(!before.isEmpty());
    auto select = [&] {
        QTextCursor c(story);
        c.setPosition(0);
        c.setPosition(story->findBlockByNumber(4).position() + story->findBlockByNumber(4).length() - 1, QTextCursor::KeepAnchor);
        return c;
    };
    struct Change {
        const char *name;
        std::function<void(QTextCursor &)> make;
    };
    const Change changes[] = {
        {"bold", [&](QTextCursor &c) { toggleBold(*doc, c); }},
        {"italic", [&](QTextCursor &c) { toggleItalic(*doc, c); }},
        {"underline", [&](QTextCursor &c) { toggleUnderline(*doc, c); }},
        {"strike", [&](QTextCursor &c) { toggleStrike(*doc, c); }},
        {"subscript", [&](QTextCursor &c) { toggleSubscript(*doc, c); }},
        {"superscript", [&](QTextCursor &c) { toggleSuperscript(*doc, c); }},
        {"size", [&](QTextCursor &c) { setFontSize(*doc, c, 30); }},
        {"grow", [&](QTextCursor &c) { changeFontSize(*doc, c, 1); }},
        {"face", [&](QTextCursor &c) { setFontFamily(*doc, c, QStringLiteral("Arimo")); }},
        {"color", [&](QTextCursor &c) { setTextColor(*doc, c, QColor(200, 0, 0)); }},
        {"highlight", [&](QTextCursor &c) { setHighlight(*doc, c, QStringLiteral("yellow")); }},
        {"character style", [&](QTextCursor &c) { applyCharacterStyle(*doc, c, QStringLiteral("Strong")); }},
        {"clear", [&](QTextCursor &c) { clearFormatting(*doc, c); }},
    };
    for (const Change &change : changes) {
        QTextCursor c = select();
        change.make(c);
        QVERIFY2(notes() == before, change.name);
    }
    // The text took the formats: Bold on the whole is bold, and Bold again is not.
    auto bold = [&] {
        QTextCursor c = select();
        return allSelected(c, [](const QTextCharFormat &f) { return f.fontWeight() >= QFont::DemiBold; });
    };
    QTextCursor c = select();
    clearFormatting(*doc, c);
    QVERIFY(!bold());
    c = select();
    toggleBold(*doc, c);
    QVERIFY(bold());
    QCOMPARE(notes(), before);
    c = select();
    toggleBold(*doc, c);
    QVERIFY(!bold());
    QCOMPARE(notes(), before);
}
