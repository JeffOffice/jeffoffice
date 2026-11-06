#include "tst_write.h"


// At the edge of the endnotes set at a section's end, Delete at the end of the section's last paragraph and Backspace at the start of the next section's join the two paragraphs and
// so end the section break, as they do where the section has no notes; the notes of the one section that is left are in one tail at its end, in the order of their references. One
// undo restores the sections and the notes exactly, and a redo does it again; and what is saved opens the same.
void WriteTests::deleteAndBackspaceAtTheEndnotesOfASectionEndTheBreak()
{
    const QByteArray pr = "<w:endnotePr><w:pos w:val=\"sectEnd\"/></w:endnotePr>";
    for (const bool forward : {true, false}) {
        auto doc = open(sectionNotedDocx(pr, pr));
        QVERIFY(doc);
        QTextDocument *story = doc->story();
        const QString before = story->toPlainText();
        const QStringList paragraphsBefore = storyParagraphs(*doc);
        QTextCursor c(story);
        if (forward) c.setPosition(story->findBlockByNumber(1).position() + story->findBlockByNumber(1).length() - 1);
        else c.setPosition(story->findBlockByNumber(4).position());
        if (forward) deleteForward(*doc, c, false);
        else deleteBack(*doc, c, false);
        const QString which = forward ? QStringLiteral("Delete") : QStringLiteral("Backspace");
        const QStringList joined = {QStringLiteral("First body"), QStringLiteral("Second bodyThird body"), QStringLiteral("Sources"), QStringLiteral("Alpha body note brwon"), QStringLiteral("Beta body note")};
        QVERIFY2(storyParagraphs(*doc) == joined, qPrintable(which + QStringLiteral(": ") + storyParagraphs(*doc).join(QLatin1Char('|'))));
        QCOMPARE(doc->sectionCount(), 1);
        QCOMPARE(tailOf(*doc), 3);
        const QString after = story->toPlainText();
        const QByteArray written = documentXml(doc->toDocx());
        QCOMPARE(written.count("<w:sectPr"), 1);
        QCOMPARE(savedBody(*doc), QStringLiteral("First body|Second bodyThird body"));
        auto reopened = open(doc->toDocx());
        QVERIFY(reopened);
        QCOMPARE(storyParagraphs(*reopened), joined);
        story->undo();
        QCOMPARE(story->toPlainText(), before);
        QCOMPARE(storyParagraphs(*doc), paragraphsBefore);
        QCOMPARE(doc->sectionCount(), 2);
        QCOMPARE(documentXml(doc->toDocx()).count("<w:sectPr"), 2);
        story->redo();
        QCOMPARE(story->toPlainText(), after);
        QCOMPARE(doc->sectionCount(), 1);
    }
}


// A section break put inside a section that has its notes at its end sends each note to the end of the section its reference is in now (the first part's notes were after the second
// part's text until the file was opened again); one undo and one redo restore both exactly, and what is saved opens the same.
void WriteTests::aSectionBreakPutInASectionMovesItsNotesToTheEndOfTheRightPart()
{
    const QByteArray pr = "<w:endnotePr><w:pos w:val=\"sectEnd\"/></w:endnotePr>";
    const QByteArray body = "<w:p><w:r><w:t>First body</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p><w:p><w:r><w:t>Second body</w:t></w:r><w:r><w:endnoteReference w:id=\"3\"/></w:r></w:p>";
    const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:t>Sources</w:t></w:r></w:p></w:endnote>"
                             "<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Alpha body note brwon</w:t></w:r></w:p></w:endnote>"
                             "<w:endnote w:id=\"3\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Beta body note</w:t></w:r></w:p></w:endnote></w:endnotes>";
    const QByteArray zip = makeDocxWithParts(body, sectionXml(6000, 6000, 720, pr), {{QStringLiteral("/word/endnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>");
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *story = doc->story();
    const QStringList one = {QStringLiteral("First body"), QStringLiteral("Second body"), QStringLiteral("Sources"), QStringLiteral("Alpha body note brwon"), QStringLiteral("Beta body note")};
    QCOMPARE(storyParagraphs(*doc), one);
    const QString before = story->toPlainText();
    QTextCursor c(story);
    c.setPosition(story->findBlockByNumber(0).position() + story->findBlockByNumber(0).length() - 1);
    insertSectionBreak(*doc, c);
    const QStringList two = {QStringLiteral("First body"), QStringLiteral("Sources"), QStringLiteral("Alpha body note brwon"), QString(), QStringLiteral("Second body"), QStringLiteral("Sources"), QStringLiteral("Beta body note")};
    QVERIFY2(storyParagraphs(*doc) == two, qPrintable(storyParagraphs(*doc).join(QLatin1Char('|'))));
    QCOMPARE(doc->sectionCount(), 2);
    QList<bool> breaks;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) breaks << b.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
    QCOMPARE(breaks, (QList<bool>{false, false, true, false, false, false, false}));
    const QString after = story->toPlainText();
    const QByteArray written = documentXml(doc->toDocx());
    QCOMPARE(written.count("<w:sectPr"), 2);
    QCOMPARE(savedBody(*doc), QStringLiteral("First body||Second body"));
    QCOMPARE(notesPartOf(doc->toDocx()), notesPartOf(zip));
    auto reopened = open(doc->toDocx());
    QVERIFY(reopened);
    QCOMPARE(storyParagraphs(*reopened), two);
    story->undo();
    QCOMPARE(story->toPlainText(), before);
    QCOMPARE(storyParagraphs(*doc), one);
    QCOMPARE(doc->sectionCount(), 1);
    story->redo();
    QCOMPARE(story->toPlainText(), after);
    QCOMPARE(storyParagraphs(*doc), two);
    QCOMPARE(doc->sectionCount(), 2);
}


// The endnotes of a file of thousands of sections, each citing a note of many paragraphs, are made and set in the most the program sets in all (not that many for each section), and the
// sections and the text are all there.
void WriteTests::endnotesAtSectionEndsAreBoundedInAll()
{
    const int sections = 1500;
    QByteArray body, notes = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:id=\"1\">";
    const QByteArray pr = "<w:endnotePr><w:pos w:val=\"sectEnd\"/></w:endnotePr>";
    for (int i = 0; i < sections; ++i)
        body += "<w:p><w:pPr>" + sectionXml(6000, 6000, 720, pr, "<w:type w:val=\"continuous\"/>") + "</w:pPr><w:r><w:t>S" + QByteArray::number(i) + "</w:t></w:r><w:r><w:endnoteReference w:id=\"1\"/></w:r></w:p>";
    for (int i = 0; i < 40; ++i) notes += "<w:p><w:r><w:t>n" + QByteArray::number(i) + "</w:t></w:r></w:p>";
    notes += "</w:endnote></w:endnotes>";
    resetEndnoteParagraphsMadeForTest();
    auto doc = open(makeDocxWithParts(body + "<w:p><w:r><w:t>Last</w:t></w:r></w:p>", sectionXml(6000, 6000, 720, pr, "<w:type w:val=\"continuous\"/>"), {{QStringLiteral("/word/endnotes.xml"), notes}}));
    QVERIFY(doc);
    QCOMPARE(endnoteParagraphsMadeForTest(), qint64(kMostEndnoteParagraphs));
    QCOMPARE(tailOf(*doc), kMostEndnoteParagraphs);
    QCOMPARE(doc->sectionCount(), sections + 1);
    QVERIFY(savedBody(*doc).startsWith(QLatin1String("S0|S1|S2|")) && savedBody(*doc).endsWith(QLatin1String("|S1499|Last")));
}


// A file whose one note of thousands of paragraphs is cited thousands of times has the copies of it that are set made and no more: the paragraphs made are as many as are set
// (the most), not as many as the notes the references name times their paragraphs (10 million here).
void WriteTests::oneNoteOfThousandsOfParagraphsCitedThousandsOfTimesIsBounded()
{
    QByteArray body = "<w:p><w:r><w:t>Text</w:t></w:r>", notes = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:id=\"1\">";
    for (int i = 0; i < 5000; ++i) body += "<w:r><w:endnoteReference w:id=\"1\"/></w:r>";
    for (int i = 0; i < 2000; ++i) notes += "<w:p/>";
    body += "</w:p>";
    notes += "</w:endnote></w:endnotes>";
    resetEndnoteParagraphsMadeForTest();
    auto doc = open(makeDocxWithParts(body, QByteArray(), {{QStringLiteral("/word/endnotes.xml"), notes}}));
    QVERIFY(doc);
    QCOMPARE(endnoteParagraphsMadeForTest(), qint64(kMostEndnoteParagraphs));
    QCOMPARE(int(doc->story()->blockCount()), 1 + kMostEndnoteParagraphs);
}


// A column break where there is no next column to go on in (a table's cell, a text box) is a line break: the text before it and after it is shown, all of it.
void WriteTests::aColumnBreakInACellOrATextBoxLosesNoText()
{
    const QByteArray content = "<w:p><w:r><w:t>alpha</w:t></w:r><w:r><w:br w:type=\"column\"/></w:r><w:r><w:t>beta</w:t></w:r></w:p><w:p><w:r><w:t>gamma</w:t></w:r></w:p>";
    // In a cell.
    const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"3000\" w:type=\"dxa\"/><w:tblLayout w:type=\"fixed\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/></w:tblGrid>"
                             "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr>" + content + "</w:tc></w:tr></w:tbl>";
    auto inCell = open(makeDocx("<w:p><w:r><w:t>Before</w:t></w:r></w:p>" + table + "<w:p><w:r><w:t>After</w:t></w:r></w:p>", kStyles));
    QVERIFY(inCell);
    const auto page = inCell->pager().materialize(0);
    QVERIFY(page && page->tables.size() == 1);
    const QString whole = inCell->story()->toPlainText();
    QString shown;
    page->tables.first()->forLines([&](const StoryLayout::LineBrief &line, const QRectF &) { shown += whole.mid(line.pos, line.end - line.pos) + QLatin1Char('|'); });
    for (const char *word : {"alpha", "beta", "gamma"}) QVERIFY2(shown.contains(QLatin1String(word)), qPrintable(QStringLiteral("%1 is not shown in the cell: %2").arg(QLatin1String(word)).arg(shown)));
    // In a text box.
    const QByteArray box = "<w:p><w:r><w:pict " + QByteArray(kVml) + "><v:rect id='b' style='position:absolute;margin-left:72pt;margin-top:72pt;width:200pt;height:100pt' filled='f'>"
                           "<v:textbox>" + QByteArray("<w:txbxContent>") + content + "</w:txbxContent></v:textbox></v:rect></w:pict></w:r></w:p>";
    auto inBox = open(makeDocxWithParts(box, sectionXml(12240, 15840, 1440), {}));
    QVERIFY(inBox);
    const auto boxed = inBox->pager().materialize(0);
    QVERIFY(boxed);
    const TextItem *found = nullptr;
    for (const auto &item : boxed->doc->pages[0]->items)
        if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
            if (const auto st = boxed->doc->story(t->storyId); st && st->doc->toPlainText().contains(QLatin1String("alpha"))) found = t;
    QVERIFY(found);
    StoryLayout layout;
    layout.build(boxed->doc->story(found->storyId)->doc.get(), {Renderer::frameSpec(*boxed->doc, *found, 1, RenderOptions())}, inBox->flow().env);
    QString text;
    for (const auto &l : layout.lineInfo(0)) text += l.text.trimmed() + QLatin1Char('|');
    for (const auto &l : layout.lineBriefs()) QVERIFY2(l.frame == 0, "every line is in the box");
    for (const char *word : {"alpha", "beta", "gamma"}) QVERIFY2(text.contains(QLatin1String(word)), qPrintable(QStringLiteral("%1 is not shown in the box: %2").arg(QLatin1String(word)).arg(text)));
}


// Reading a file asks its tags their attributes with an expression made once for each name, not one for each tag (a file with hundreds of thousands of absolute position tabs took
// most of a minute to read).
void WriteTests::tagsAreAskedTheirAttributesWithAnExpressionMadeOncePerName()
{
    QByteArray runs;
    for (int i = 0; i < 3000; ++i) runs += "<w:r><w:ptab w:relativeTo=\"margin\" w:alignment=\"right\" w:leader=\"none\"/></w:r>";
    const qint64 before = attributeExpressionsMadeForTest();
    auto doc = open(makeDocx("<w:p>" + runs + "</w:p>", kStyles));
    QVERIFY(doc);
    const qint64 made = attributeExpressionsMadeForTest() - before;
    QVERIFY2(made <= 10, qPrintable(QString::number(made)));   // (a few names: the three of a tab's, and what the rest of reading asks; 9,000 when each tag made its own)
}


// The endnotes' own numbered paragraphs count on their own: the first numbered paragraph of an endnote is 1, whatever the text before it has counted of the list it is in.
void WriteTests::anEndnotesNumberedParagraphsCountOnTheirOwn()
{
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/></w:lvl></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
    const QByteArray body = itemOf(1, 0, "one") + itemOf(1, 0, "two") + "<w:p><w:r><w:t>cited</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>";
    const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:id=\"2\"><w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>noted</w:t></w:r></w:p></w:endnote></w:endnotes>";
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), notes}}, QByteArray(), kStyles, numbering));
    QVERIFY(doc);
    QCOMPARE(markersOf(*doc), (QStringList{QStringLiteral("1."), QStringLiteral("2."), QStringLiteral("1.")}));
}


// The file's separator for its endnotes (w:separator in a paragraph of nothing else) is a rule two inches long, as the footnote separator is: the paragraph's lower edge, over the first 144
// points of the column, not a blank line.
void WriteTests::theSeparatorOfTheEndnotesIsARule()
{
    const QByteArray body = "<w:p><w:r><w:t>cited</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>";
    const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:endnote>"
                             "<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> noted</w:t></w:r></w:p></w:endnote></w:endnotes>";
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), notes}}));
    QVERIFY(doc);
    QCOMPARE(int(doc->story()->blockCount()), 3);
    const QTextBlockFormat bf = doc->story()->findBlockByNumber(1).blockFormat();
    QVERIFY(bf.hasProperty(prop::EndnoteBlock));
    const tp::ParaBox box = tp::ParaBox::parse(bf.stringProperty(tp::Borders));
    QVERIFY2(box.bottom.visible() && !box.top.visible(), "the separator has its rule");
    QVERIFY2(std::abs(bf.rightMargin() - (228 - 144)) < 0.5, qPrintable(QString::number(bf.rightMargin())));   // (a column of 228 points, and a rule of 144)
    QVERIFY(!doc->story()->findBlockByNumber(2).blockFormat().hasProperty(tp::Borders));
}


// A reference to a bookmark (PAGEREF) shows the number of the page the bookmark is on, in the format of the page's section, where the pages are laid out; the file's own result stays in the
// story, and is what is saved. (A table of contents the file made before its pages moved: its numbers were all "2".) A reference to a bookmark that is not there shows the result the file had.
void WriteTests::aPageReferenceShowsThePageItsBookmarkIsOn()
{
    auto field = [](const char *name, const char *cached) {
        return "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> PAGEREF " + QByteArray(name) + " \\h </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
               "<w:r><w:t>" + QByteArray(cached) + "</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
    };
    auto body = [&](const char *name) {
        return "<w:p><w:r><w:t xml:space=\"preserve\">See page </w:t></w:r>" + field(name, "9") + "<w:r><w:br w:type=\"page\"/></w:r></w:p><w:p><w:r><w:t>Middle</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r></w:p>"
               "<w:p><w:bookmarkStart w:id=\"1\" w:name=\"target\"/><w:r><w:t>Target</w:t></w:r><w:bookmarkEnd w:id=\"1\"/></w:p>";
    };
    // The first line of the first page, as the page's own fields show it.
    auto shown = [&](const QByteArray &xml, const QByteArray &pgNum) {
        auto doc = open(makeDocxWithParts(xml, sectionXml(6000, 6000, 720, QByteArray(), QByteArray(), pgNum), {}));
        if (!doc || doc->pager().pageCount() != 3) return QStringLiteral("(%1 pages)").arg(doc ? doc->pager().pageCount() : -1);
        const auto page = doc->pager().materialize(0);
        if (!page || page->boxes.isEmpty()) return QStringLiteral("(no page)");
        const auto *box = dynamic_cast<const TextItem *>(page->doc->item(page->boxes[0]));
        FrameSpec fs = Renderer::frameSpec(*page->doc, *box, 1, RenderOptions());
        StoryLayout layout;
        layout.build(page->doc->story(box->storyId)->doc.get(), {fs}, doc->flow().env);
        const auto lines = layout.lineInfo(0);
        QString saved = savedBody(*doc);
        return (lines.isEmpty() ? QString() : lines.first().text.trimmed()) + QLatin1Char('|') + saved.left(saved.indexOf(QLatin1Char('|')));
    };
    QCOMPARE(shown(body("target"), QByteArray()), QStringLiteral("See page 3|See page 9"));
    QCOMPARE(shown(body("target"), "<w:pgNumType w:fmt=\"lowerRoman\"/>"), QStringLiteral("See page iii|See page 9"));
    QCOMPARE(shown(body("target"), "<w:pgNumType w:start=\"5\"/>"), QStringLiteral("See page 7|See page 9"));
    QCOMPARE(shown(body("nowhere"), QByteArray()), QStringLiteral("See page 9|See page 9"));
    // In a section that carries the chapter's number, the number is the chapter of the bookmark's place before it: Heading 1 "One" on page 1, "Two" on page 2 with the bookmark after it.
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/></w:lvl></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
    auto heading = [](const char *text) { return QByteArray("<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>") + text + "</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r></w:p>"; };
    const QByteArray chaptered = "<w:p><w:r><w:t xml:space=\"preserve\">See page </w:t></w:r>" + field("target", "9") + "<w:r><w:br w:type=\"page\"/></w:r></w:p>" + heading("One") +
                                 "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:bookmarkStart w:id=\"1\" w:name=\"target\"/><w:r><w:t>Two</w:t></w:r><w:bookmarkEnd w:id=\"1\"/></w:p>";
    auto doc = open(makeDocxWithParts(chaptered, sectionXml(6000, 6000, 720, QByteArray(), QByteArray(), "<w:pgNumType w:chapStyle=\"1\" w:chapSep=\"colon\"/>"), {}, QByteArray(), kStyles, numbering));
    QVERIFY(doc && doc->pager().pageCount() == 3);
    const auto first = doc->pager().materialize(0);
    QVERIFY(first && !first->boxes.isEmpty());
    const auto *box = dynamic_cast<const TextItem *>(first->doc->item(first->boxes[0]));
    StoryLayout layout;
    layout.build(first->doc->story(box->storyId)->doc.get(), {Renderer::frameSpec(*first->doc, *box, 1, RenderOptions())}, doc->flow().env);
    QVERIFY2(layout.lineInfo(0).first().text.contains(QLatin1String("See page 2:3")), qPrintable(layout.lineInfo(0).first().text));
}


// The references to bookmarks on a page that are looked at to find their pages are bounded: a hostile file of thousands of them has the most looked at (the rest show what the file had), so
// that a page costs no more.
void WriteTests::thePageReferencesOfAPageAreBounded()
{
    QByteArray body = "<w:p>";
    for (int i = 0; i < 5000; ++i)
        body += "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> PAGEREF mark" + QByteArray::number(i) + " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
                "<w:r><w:t>1</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
    body += "</w:p>";
    auto doc = open(makeDocx(body, kStyles));
    QVERIFY(doc);
    const qint64 before = flow::Paginator::pageRefsAskedForTest();
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    const qint64 asked = flow::Paginator::pageRefsAskedForTest() - before;
    QVERIFY2(asked > 0 && asked <= 2000, qPrintable(QString::number(asked)));
}


// A paragraph that says w:suppressLineNumbers (itself, or by its style) has no numbers, and the lines of the others are counted without it.
void WriteTests::aParagraphThatSuppressesLineNumbersIsNeitherNumberedNorCounted()
{
    const QByteArray styles = kStyles + QByteArray("<w:style w:type=\"character\" w:styleId=\"LineNumber\"><w:name w:val=\"line number\"/><w:rPr><w:sz w:val=\"14\"/></w:rPr></w:style>"
                                                      "<w:style w:type=\"paragraph\" w:styleId=\"Quiet\"><w:name w:val=\"Quiet\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:suppressLineNumbers/></w:pPr></w:style>");
    QByteArray body = plainP(wordsOf("first", 20)) + plainP(wordsOf("direct", 20), QByteArray(), "<w:suppressLineNumbers/>") + plainP(wordsOf("third", 20)) +
                      plainP(wordsOf("styled", 20), QByteArray(), "<w:pStyle w:val=\"Quiet\"/>") + plainP(wordsOf("fifth", 20)) + plainP(wordsOf("turned on again", 20), QByteArray(), "<w:suppressLineNumbers w:val=\"0\"/>");
    const QByteArray sect = sectionXml(12240, 15840, 1440, QByteArray(), QByteArray(), "<w:lnNumType w:countBy=\"1\"/>");
    auto doc = open(makeDocxWithParts(body, sect, {}, QByteArray(), styles));
    QVERIFY(doc);
    flow::Paginator &pg = doc->pager();
    QCOMPARE(pg.pageCount(), 1);
    const QTextDocument *story = doc->story();
    int counted = 0;
    for (const auto &l : pg.lines(0)) {
        const QString text = story->findBlock(l.block).text();
        counted += !text.startsWith(QLatin1String("direct")) && !text.startsWith(QLatin1String("styled"));
    }
    QVERIFY(counted > 4 && counted < int(pg.lines(0).size()));
    const auto page = pg.materialize(0);
    QVERIFY(page);
    QStringList numbers;
    for (const auto &item : page->doc->pages[0]->items)
        if (item->id.startsWith(QLatin1String("flow-lineno-box"))) numbers << page->doc->stories.value(dynamic_cast<const TextItem *>(item.get())->storyId)->doc->toPlainText();
    QStringList want;
    for (int i = 1; i <= counted; ++i) want << QString::number(i);
    QCOMPARE(numbers, want);
}


// A field of a header that shows a paragraph style's text (STYLEREF), as the file would have it: its instruction, and a result `cached`.
QByteArray WriteTests::styleRefField(const char *instruction, const char *cached)
{
    return "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> STYLEREF " + QByteArray(instruction) +
           " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r><w:r><w:t>" + cached + "</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
}


// What a header shows of a paragraph style (a STYLEREF field) is the page's own: the first paragraph of the style on the page (the last with \l, its number with \n), or the
// last before the page when it has none, or the first after it when there is none before (the pages before the first title show that one); the field's result in the file is
// what shows where the document has no paragraph of the style at all.
void WriteTests::aHeaderShowsTheStyledParagraphOfItsPage()
{
    const QMap<QString, QByteArray> parts{{QStringLiteral("/word/header1.xml"),
                                           headerPart("hdr", {"<w:r><w:t xml:space=\"preserve\">First: </w:t></w:r>" + styleRefField("\"heading 1\""), "<w:r><w:t xml:space=\"preserve\">Last: </w:t></w:r>" + styleRefField("\"heading 1\" \\l"),
                                                              "<w:r><w:t xml:space=\"preserve\">Number: </w:t></w:r>" + styleRefField("\"heading 1\" \\n"),
                                                              "<w:r><w:t xml:space=\"preserve\">None: </w:t></w:r>" + styleRefField("\"No Such Style\"")})}};
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/></w:lvl></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
    QByteArray body;
    for (int i = 0; i < 3; ++i) body += plainP(wordsOf("front", 150));   // (pages before the first title)
    for (int h = 0; h < 4; ++h) {
        body += "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>Chapter " + QByteArray::number(h + 1) + " title</w:t></w:r></w:p>";
        for (int i = 0; i < (h == 1 || h == 2 ? 1 : 3); ++i) body += plainP(wordsOf("text", h == 1 || h == 2 ? 8 : 150));   // (the second and third chapters share a page)
    }
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720, "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>"), parts, QByteArray(), kStyles, numbering));
    QVERIFY(doc);
    flow::Paginator &pg = doc->pager();
    const int pages = pg.pageCount();
    QVERIFY(pages >= 8);
    const QTextDocument *story = doc->story();
    int sawBefore = 0, sawTwo = 0, sawAfter = 0;
    for (int k = 0; k < pages; ++k) {
        const int from = pg.page(k).start.pos, to = k + 1 < pages ? pg.page(k + 1).start.pos : story->characterCount();
        QString first, last, before, after;
        int firstNumber = 0, lastNumber = 0, beforeNumber = 0, afterNumber = 0, number = 0;
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
            if (b.blockFormat().stringProperty(tp::StyleName) != QLatin1String("Heading1")) continue;
            ++number;
            if (b.position() + b.length() <= from) {
                before = b.text();
                beforeNumber = number;
            } else if (b.position() < to) {
                if (first.isEmpty()) {
                    first = b.text();
                    firstNumber = number;
                }
                last = b.text();
                lastNumber = number;
            } else if (after.isEmpty()) {
                after = b.text();
                afterNumber = number;
            }
        }
        // (On the page, else the last before it, else the first after it.)
        const QString shownFirst = !first.isEmpty() ? first : !before.isEmpty() ? before : after, shownLast = !last.isEmpty() ? last : !before.isEmpty() ? before : after;
        const int shownNumber = firstNumber ? firstNumber : beforeNumber ? beforeNumber : afterNumber;
        Q_UNUSED(lastNumber)
        sawBefore += first.isEmpty() && !before.isEmpty();
        sawTwo += first != last;
        sawAfter += first.isEmpty() && before.isEmpty();
        const auto page = pg.materialize(k);
        QVERIFY(page);
        const auto heads = audit::itemsWith(*page, QStringLiteral("flow-header-box"));
        QVERIFY(heads.size() == 1);
        QString shown = audit::storyOf(*page, heads[0]);
        const QString want = QStringLiteral("First: %1|Last: %2|Number: %3|None: ￼").arg(shownFirst, shownLast).arg(shownNumber);
        QVERIFY2(shown.replace(QLatin1Char('\n'), QLatin1Char('|')).replace(QChar::ParagraphSeparator, QLatin1Char('|')) == want, qPrintable(QStringLiteral("page %1: %2 for %3").arg(k).arg(shown, want)));
    }
    QVERIFY2(sawBefore > 0 && sawTwo > 0 && sawAfter >= 3, "the pages have some with no title on them, one with two, and three before the first title");
}


// The fields of a header are as many as the page gives text to (the rest show what the file had), and a title is found however far from the page it is.
void WriteTests::theFieldsOfAHeaderAreBoundedAndFindTitlesAnyDistanceOff()
{
    QByteArray fields;
    for (int i = 0; i < 70; ++i) fields += "<w:r><w:t xml:space=\"preserve\"> n" + QByteArray::number(i) + "=</w:t></w:r>" + styleRefField("\"heading 1\"");
    const QMap<QString, QByteArray> parts{{QStringLiteral("/word/header1.xml"), headerPart("hdr", {fields})}};
    QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>The only title</w:t></w:r></w:p>";
    for (int i = 0; i < 25000; ++i) body += plainP("x");   // (more paragraphs than a search can walk)
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720, "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>"), parts));
    QVERIFY(doc);
    flow::Paginator &pg = doc->pager();
    const int pages = pg.pageCount();
    QVERIFY(pages > 100);
    const auto page = pg.materialize(pages - 1);
    QVERIFY(page);
    const auto heads = audit::itemsWith(*page, QStringLiteral("flow-header-box"));
    QVERIFY(heads.size() == 1);
    const QString shown = audit::storyOf(*page, heads[0]);
    int givenText = 0;
    for (const QChar ch : shown) givenText += ch == QChar::ObjectReplacementCharacter;
    QCOMPARE(shown.count(QStringLiteral("The only title")), 64);   // (the title, in the first 64 fields, from 25,000 paragraphs off)
    QCOMPARE(givenText, 6);                                         // (and the other six show what the file had)
}


// A page's fields cost a search each and no walk over the story: for 600 pages and 64 fields of a style that is declared and not used, and of one that is, the paragraphs looked at are
// about those of the story, once, and a few to a page.
void WriteTests::theFieldsOfAHeaderDoNotWalkTheStoryForEachPage()
{
    const QByteArray styles = kStyles + QByteArray("<w:style w:type=\"paragraph\" w:styleId=\"Unused\"><w:name w:val=\"Never Used\"/><w:basedOn w:val=\"Normal\"/></w:style>");
    QByteArray fields;
    for (int i = 0; i < 64; ++i) fields += styleRefField(i % 2 ? "\"Never Used\"" : "\"heading 1\"");
    const QMap<QString, QByteArray> parts{{QStringLiteral("/word/header1.xml"), headerPart("hdr", {fields})}};
    QByteArray body;
    for (int i = 0; i < 6000; ++i) body += i % 500 == 0 ? QByteArray("<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Title ") + QByteArray::number(i) + "</w:t></w:r></w:p>" : plainP("line");
    auto doc = open(makeDocxWithParts(body, sectionXml(3000, 2400, 360, "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>"), parts, QByteArray(), styles));
    QVERIFY(doc);
    flow::Paginator &pg = doc->pager();
    const int pages = pg.pageCount();
    QVERIFY(pages >= 500);
    resetStyleRefParagraphsScannedForTest();
    for (int k = 0; k < pages; ++k) QVERIFY(pg.materialize(k));
    const qint64 looked = styleRefParagraphsScannedForTest();
    QVERIFY2(looked <= 2 * 6000 + qint64(pages) * 4, qPrintable(QStringLiteral("%1 paragraphs looked at for %2 pages of 64 fields").arg(looked).arg(pages)));
}


// A title that the page puts in a header's field and that is longer than the file's takes more lines: the box is as tall as the text, not a clip of the first line; and two fields
// next to one another that ask for the same are both given it.
void WriteTests::aLongTitleInAHeaderIsNotClippedAndTwoFieldsAreBothGivenIt()
{
    const QMap<QString, QByteArray> parts{{QStringLiteral("/word/header1.xml"), headerPart("hdr", {styleRefField("\"heading 1\"", "x") + styleRefField("\"heading 1\"", "x")})}};
    QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>short</w:t></w:r></w:p>" + plainP(wordsOf("page", 120));
    body += "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>" + wordsOf("A title that goes over several lines of the header because it is long", 30) + "</w:t></w:r></w:p>" + plainP(wordsOf("page", 120));
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720, "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>"), parts));
    QVERIFY(doc);
    flow::Paginator &pg = doc->pager();
    const int pages = pg.pageCount();
    QVERIFY(pages >= 3);
    double shortHeight = 0, longHeight = 0;
    for (int k = 0; k < pages; ++k) {
        const auto page = pg.materialize(k);
        QVERIFY(page);
        const auto heads = audit::itemsWith(*page, QStringLiteral("flow-header-box"));
        QVERIFY(heads.size() == 1);
        const QString shown = audit::storyOf(*page, heads[0]);
        QVERIFY2(!shown.contains(QChar::ObjectReplacementCharacter), qPrintable(shown));   // (both fields are replaced)
        if (shown.startsWith(QLatin1String("shortshort"))) shortHeight = heads[0]->rect.height();
        if (shown.startsWith(QLatin1String("A title"))) longHeight = std::max(longHeight, heads[0]->rect.height());
    }
    QVERIFY2(shortHeight > 0 && longHeight > 1.8 * shortHeight, qPrintable(QStringLiteral("a short title's header is %1 high, a long one's %2").arg(shortHeight).arg(longHeight)));
}


// A note cited in the section before a continuous section of two columns, on the page where that begins: the notes are at the foot of the page, under the columns, and the
// text of the first section is in the width of the page (the columns begin below it); the audit finds no overlap, nothing lost, and no empty page.
void WriteTests::aFilesNoteCitedBeforeAContinuousTwoColumnSectionIsAtThePageFoot()
{
    int checked = 0;
    for (const int noteWords : {5, 40, 120})
        for (const int cited : {0, 1}) {
            const QByteArray one = sectionXml(6000, 8000, 720);
            const QByteArray two = sectionXml(6000, 8000, 720, QByteArray(), "<w:type w:val=\"continuous\"/>", "<w:cols w:num=\"2\" w:space=\"360\"/>");
            QByteArray body = plainP(wordsOf("One", 12), cited == 0 ? refTo(2) : QByteArray()) + plainP(wordsOf("Second", 8), cited == 1 ? refTo(2) : QByteArray());
            body += plainP("end of the first section", QByteArray(), one);
            for (int i = 0; i < 14; ++i) body += plainP(wordsOf("Column", 14));
            auto doc = open(makeDocxWithParts(body, two, {{QStringLiteral("/word/footnotes.xml"), notesPart({{2, wordsOf("COLNOTE", noteWords)}})}}));
            QVERIFY(doc);
            flow::Paginator &pg = doc->pager();
            QCOMPARE(doc->sectionCount(), 2);
            const QString bad = audit::problems(pg, doc->story(), {{"fn2", "COLNOTE"}});
            QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("a note of %1 words, cited in paragraph %2: %3").arg(noteWords).arg(cited).arg(bad)));
            // The first section's lines are as wide as the page's text area, not as one of the columns.
            const double width = pg.page(0).content.width();
            for (const auto &l : pg.lines(0))
                if (l.pos < doc->story()->findBlock(0).length() + doc->story()->findBlockByNumber(1).length())
                    QVERIFY2(l.rect.width() > width - 1, qPrintable(QStringLiteral("a line of the first section is %1 wide in a text area %2 wide").arg(l.rect.width()).arg(width)));
            QVERIFY2(audit::lostNotes(pg, audit::fullNoteHeight(doc->flow().fields->storyDoc(QStringLiteral("fn2")), width, doc->flow().env)).isEmpty(), "the note is drawn whole");
            ++checked;
        }
    QCOMPARE(checked, 6);
}


// A continuous section of two columns with a table in it: it begins below the text of the section before, the table is inside a column and goes on in the next, and the
// note cited above it, in the first column, and the header and footer keep to their places.
void WriteTests::aFilesTableInAContinuousTwoColumnSectionIsInsideItsColumn()
{
    const QByteArray refs = "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/>";
    const QByteArray one = sectionXml(6000, 8000, 720, refs);
    const QByteArray two = sectionXml(6000, 8000, 720, QByteArray(), "<w:type w:val=\"continuous\"/>", "<w:cols w:num=\"2\" w:space=\"360\"/>");
    const QByteArray body = plainP(wordsOf("One", 20)) + plainP("end of the first section", QByteArray(), one) + plainP(wordsOf("Two", 5)) + plainP("The paragraph that cites a note", refTo(2)) +
                            tableOfRows(70, 2, 900, [](int r, int c) { return cellP("row " + QByteArray::number(r) + " col " + QByteArray::number(c)); }) + plainP("After the table");
    auto doc = open(makeDocxWithParts(body, two, {{QStringLiteral("/word/footnotes.xml"), notesPart({{2, wordsOf("COLNOTE", 12)}})}, {QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>HEADER</w:t></w:r>"})},
                                                   {QStringLiteral("/word/footer1.xml"), headerPart("ftr", {"<w:r><w:t>FOOTER A</w:t></w:r>", "<w:r><w:t>FOOTER B</w:t></w:r>"})}}));
    QVERIFY(doc);
    flow::Paginator &pg = doc->pager();
    const int pages = pg.pageCount();
    QVERIFY(pages >= 2);
    QCOMPARE(doc->sectionCount(), 2);
    const flow::Section &sec = doc->flow().section(1);
    QCOMPARE(sec.columnCount(), 2);
    double split = 0;
    for (const auto &l : pg.lines(0))
        if (l.frame == 0) split = std::max(split, l.rect.bottom());
    QVERIFY(split > 0);
    const double top = pg.page(0).content.top();
    int fragments = 0;
    for (int k = 0; k < pages; ++k) {
        const auto m = pg.materialize(k);
        QVERIFY(m);
        for (const auto &t : m->tables) {
            const QRectF r = t->rect();
            bool inAColumn = false;
            for (int col = 0; col < sec.columnCount(); ++col) {
                const QRectF c = sec.columnRect(col);
                inAColumn |= r.left() >= c.left() - 6 && r.right() <= c.right() + 0.1;   // (a table's edge is left of its text by the margin of its cells: 5.4 points)
            }
            QVERIFY2(inAColumn, qPrintable(QStringLiteral("page %1: the table at %2 is not inside a column of the section (%3 and %4)").arg(k).arg(audit::rectText(r), audit::rectText(sec.columnRect(0)), audit::rectText(sec.columnRect(1)))));
            if (k == 0) QVERIFY2(r.top() >= top + split - 0.1, qPrintable(QStringLiteral("page 0: the table begins at %1, the section before it ends at %2").arg(r.top()).arg(top + split)));
            ++fragments;
        }
    }
    QVERIFY(fragments >= 3);
    const auto first = pg.materialize(0);
    QVERIFY2(first->tables.size() == 2, qPrintable(QString::number(first->tables.size())));
    QVERIFY(first->tables[1]->rect().left() > first->tables[0]->rect().right() - 0.1);
    QVERIFY2(std::abs(first->tables[1]->rect().top() - (top + split)) < 0.1, qPrintable(QStringLiteral("the table's first part is at %1, its second at %2, the columns begin at %3")
                                                                                          .arg(audit::rectText(first->tables[0]->rect()), audit::rectText(first->tables[1]->rect())).arg(top + split)));
    QCOMPARE(rowsShown(pg), 70);
    const QString bad = audit::problems(pg, doc->story(), {{"fn2", "COLNOTE"}});
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
}


// A note longer than the pages, cited from a cell: it goes on over pages of its own, which are made and drawn, and the end of it is on the last of them.
void WriteTests::aFilesNoteLongerThanThePageCitedFromACellHasPagesOfItsOwn()
{
    const QByteArray body = plainP("Opening paragraph.") + tableOfRows(6, 2, 1500, [](int r, int c) { return c == 1 && r == 2 ? plainP("row 2 cites", refTo(2)) : cellP("row " + QByteArray::number(r) + " col " + QByteArray::number(c)); }) +
                            plainP("Tail one.") + plainP("Tail two.") + plainP("Tail three, which cites a short note.", refTo(3)) + plainP("Tail four.") + plainP("Tail five.");
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/footnotes.xml"), notesPart({{2, wordsOf("LONGNOTE", 700)}, {3, "SHORTNOTE says little"}})}}));
    QVERIFY(doc);
    flow::Paginator &pg = doc->pager();
    const int pages = pg.pageCount();
    int noteOnly = 0;
    QString drawn;
    for (int k = 0; k < pages; ++k) {
        const auto page = pg.materialize(k);
        QVERIFY2(page, qPrintable(QStringLiteral("page %1 of %2 isn't made").arg(k).arg(pages)));
        if (pg.lines(k).isEmpty()) {
            ++noteOnly;
            QVERIFY2(!page->doc->pages[0]->items.empty(), qPrintable(QStringLiteral("page %1 of notes alone has nothing drawn").arg(k)));
        }
        for (const auto &item : page->doc->pages[0]->items) drawn += audit::storyOf(*page, item.get()) + QLatin1Char('\n');
    }
    QVERIFY2(noteOnly > 0, qPrintable(QStringLiteral("%1 pages, none of notes alone").arg(pages)));
    // (A box's story holds the whole rest of its note, whatever the box shows of it: what is drawn is the height of the boxes.)
    const FieldSource *fields = doc->flow().fields;
    QVERIFY(fields);
    const double width = pg.page(0).content.width();
    const QString lost = audit::lostNotes(pg, audit::fullNoteHeight(fields->storyDoc(QStringLiteral("fn2")), width, doc->flow().env) + audit::fullNoteHeight(fields->storyDoc(QStringLiteral("fn3")), width, doc->flow().env));
    QVERIFY2(lost.isEmpty(), qPrintable(lost));
    QVERIFY2(drawn.contains(QStringLiteral("SHORTNOTE")), "the short note is drawn");
    QCOMPARE(rowsShown(pg), 6);
    const QString bad = audit::problems(pg, doc->story(), {{"fn2", "LONGNOTE"}, {"fn3", "SHORTNOTE"}});
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
}


// A row that goes over page breaks in a file whose headers and footers take room from the text: every paragraph of its cells is found on the page that shows the part of
// the row it is in.
void WriteTests::aFilesPositionsInARowThatGoesOverPageBreaksAreOnThePagesThatShowThem()
{
    const QByteArray refs = "<w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/>";
    QByteArray tall;
    for (int i = 0; i < 140; ++i) tall += cellP("line " + QByteArray::number(i) + " of the tall cell");
    const QByteArray body = plainP("Before the table") + tableX(rowX(cellX(cellP("first"), QByteArray(), 1500) + cellX(cellP("row"), QByteArray(), 1500)) + rowX(cellX(cellP("tall"), QByteArray(), 1500) + cellX(tall, QByteArray(), 1500)) +
                                                              rowX(cellX(cellP("last"), QByteArray(), 1500) + cellX(cellP("row"), QByteArray(), 1500)), 2, 1500) + plainP("After the table");
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720, refs), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>H1</w:t></w:r>", "<w:r><w:t>H2</w:t></w:r>", "<w:r><w:t>H3</w:t></w:r>", "<w:r><w:t>H4</w:t></w:r>"})},
                                                                            {QStringLiteral("/word/footer1.xml"), headerPart("ftr", {"<w:r><w:t>F1</w:t></w:r>", "<w:r><w:t>F2</w:t></w:r>", "<w:r><w:t>F3</w:t></w:r>"})}}));
    QVERIFY(doc);
    flow::Paginator &pg = doc->pager();
    QVERIFY(pg.pageCount() >= 3);
    QVERIFY2(doc->flow().section(0).content().height() - pg.page(1).content.height() > 10, "the header and the footer take room from the text");
    const QString bad = audit::splitRowProblems(pg, doc->story());
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    const QString rest = audit::problems(pg, doc->story());
    QVERIFY2(rest.isEmpty(), qPrintable(rest));
}


// A table under the line heights of a word processor, in a file of the 2003 programs (compatibility mode 11) and one of 2013's and after (15): the stand-in for the table in
// the page's text does not shift it or the text after it. The table begins where the text before it ends (and its space after), and the page's text has every line
// where the paginator put it. (The two modes differ in where a table's left edge is: a table's first cell has its text at the margin in the first.)
void WriteTests::aFilesTableIsNotShiftedByItsPlaceholderUnderProcessorLineHeights()
{
    struct Case { const char *name; QByteArray spacing; };
    const Case cases[] = {{"auto 1.15, 12 before, 8 after", "<w:spacing w:before=\"240\" w:after=\"160\" w:line=\"276\" w:lineRule=\"auto\"/>"},
                          {"exactly 20, none after", "<w:spacing w:before=\"0\" w:after=\"0\" w:line=\"400\" w:lineRule=\"exact\"/>"},
                          {"at least 30, 5 after", "<w:spacing w:after=\"100\" w:line=\"600\" w:lineRule=\"atLeast\"/>"},
                          {"single, 6 after", "<w:spacing w:after=\"120\" w:line=\"240\" w:lineRule=\"auto\"/>"},
                          {"a border below, 6 after", "<w:pBdr><w:bottom w:val=\"single\" w:sz=\"8\" w:space=\"3\" w:color=\"auto\"/></w:pBdr><w:spacing w:after=\"120\"/>"}};
    double edge[2] = {0, 0};
    for (const int mode : {11, 15})
        for (const Case &cs : cases) {
            const QByteArray settings = "<w:compat><w:compatSetting w:name=\"compatibilityMode\" w:uri=\"http://schemas.microsoft.com/office/word\" w:val=\"" + QByteArray::number(mode) + "\"/></w:compat>";
            const QByteArray body = plainP(wordsOf("First", 25)) + plainP(wordsOf("The last paragraph before the table", 12), QByteArray(), cs.spacing) +
                                    tableOfRows(4, 2, 2000, [](int r, int c) { return cellP("row " + QByteArray::number(r) + " col " + QByteArray::number(c)); }) +
                                    plainP("After the table", QByteArray(), "<w:spacing w:line=\"360\" w:lineRule=\"auto\"/>") + plainP(wordsOf("Last", 40));
            auto doc = open(makeDocxWithParts(body, sectionXml(7000, 9000, 1000), {}, settings));
            QVERIFY(doc);
            flow::Paginator &pg = doc->pager();
            const QString what = QStringLiteral("mode %1, %2").arg(mode).arg(QLatin1String(cs.name));
            QVERIFY2(pg.pageCount() >= 1, qPrintable(what));
            const auto lines = pg.lines(0);
            int firstRow = 0;
            while (firstRow < lines.size() && !cellKeyOf(doc->story()->findBlock(lines[firstRow].pos))) ++firstRow;
            QVERIFY2(firstRow > 0 && firstRow + 4 <= lines.size(), qPrintable(what));
            // (A paragraph with a border below has the room of the border's lower edge, its space and its width, under its last line before the space after it.)
            const double after = doc->story()->findBlock(lines[firstRow - 1].pos).blockFormat().bottomMargin() + (cs.spacing.contains("pBdr") ? 3.0 + 8 / 8.0 : 0.0);
            QVERIFY2(std::abs(lines[firstRow - 1].rect.bottom() + std::max(0.0, after) - lines[firstRow].rect.top()) < 0.05, qPrintable(what + ": the table's top and the end of the text before it"));
            const auto page = pg.materialize(0);
            QVERIFY(page && page->tables.size() == 1);
            QVERIFY2(std::abs(page->tables[0]->rect().top() - pg.page(0).content.top() - lines[firstRow].rect.top()) < 0.05, qPrintable(what + ": the table is drawn where its rows are"));
            const QString bad = audit::placementProblems(pg, doc->flow(), 0);
            QVERIFY2(bad.isEmpty(), qPrintable(what + QStringLiteral(": ") + bad));
            if (&cs == &cases[0]) edge[mode == 11 ? 0 : 1] = page->tables[0]->rect().left() - pg.page(0).content.left();
        }
    QVERIFY2(edge[0] < edge[1] - 1, qPrintable(QStringLiteral("the table's left edge is %1 from the margin in mode 11, %2 in mode 15").arg(edge[0]).arg(edge[1])));
}


// A footer with PAGE and NUMPAGES, in a file whose table spans pages: each page shows its number and the count, and the pages made after rows are put in show the new count.
void WriteTests::aFilesPageFieldsInAFooterAreRightForATableThatSpansPages()
{
    const QByteArray footer = headerPart("ftr", {"<w:r><w:t xml:space=\"preserve\">Page </w:t></w:r><w:fldSimple w:instr=\" PAGE \"><w:r><w:t>1</w:t></w:r></w:fldSimple><w:r><w:t xml:space=\"preserve\"> of </w:t></w:r>"
                                                   "<w:fldSimple w:instr=\" NUMPAGES \"><w:r><w:t>1</w:t></w:r></w:fldSimple>",
                                                   "<w:r><w:t>A second line of the footer</w:t></w:r>"});
    const QByteArray refs = "<w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/>";
    const QByteArray body = plainP("Before the table") + tableOfRows(100, 2, 1500, [](int r, int c) { return cellP("row " + QByteArray::number(r) + " col " + QByteArray::number(c)); }) + plainP("After the table");
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720, refs), {{QStringLiteral("/word/footer1.xml"), footer}}));
    QVERIFY(doc);
    flow::Paginator &pg = doc->pager();
    auto checkPages = [&](const QString &when) -> QString {
        const int pages = pg.pageCount();
        for (int k = 0; k < pages; ++k) {
            const auto page = pg.materialize(k);
            if (!page) return QStringLiteral("%1: page %2 is not made").arg(when).arg(k);
            const auto foots = audit::itemsWith(*page, QStringLiteral("flow-footer-box"));
            if (foots.size() != 1) return QStringLiteral("%1: page %2 has %3 footers").arg(when).arg(k).arg(foots.size());
            FieldContext ctx;
            ctx.doc = page->doc.get();
            if (ctx.resolve(QStringLiteral("page")) != QString::number(k + 1)) return QStringLiteral("%1: page %2 shows the number %3").arg(when).arg(k).arg(ctx.resolve(QStringLiteral("page")));
            if (ctx.resolve(QStringLiteral("pages")) != QString::number(pages)) return QStringLiteral("%1: page %2 shows %3 pages, the file has %4").arg(when).arg(k).arg(ctx.resolve(QStringLiteral("pages"))).arg(pages);
            if (!page->showsTotal || page->pageTotal != pages) return QStringLiteral("%1: page %2 is made with the total %3 (it shows it: %4), the file has %5").arg(when).arg(k).arg(page->pageTotal).arg(page->showsTotal).arg(pages);
        }
        return audit::problems(pg, doc->story());
    };
    const int before = pg.pageCount();
    QVERIFY(before >= 5);
    QString bad = checkPages(QStringLiteral("as opened"));
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    QTextCursor c(doc->story());
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
        if (cellKeyOf(b)) {
            c.setPosition(b.position());
            break;
        }
    for (int i = 0; i < 30; ++i) insertTableRow(*doc, c, true);
    QVERIFY(pg.pageCount() > before);
    bad = checkPages(QStringLiteral("after the rows"));
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    flow::Paginator fresh(&doc->flow());
    bad = audit::differenceFromFresh(pg, fresh);
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
}


void WriteTests::randomEditingKeepsTheDocumentWhole()
{
    // A document with lists, a section, pieces kept as XML, and a page break; hundreds of random commands on it, and after each few the
    // file it makes opens again with the same text, the same sections, and pages that can be made.
    const QByteArray numbering =
        "<w:abstractNum w:abstractNumId=\"0\"><w:multiLevelType w:val=\"hybridMultilevel\"/><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl>"
        "<w:lvl w:ilvl=\"1\"><w:start w:val=\"1\"/><w:numFmt w:val=\"lowerLetter\"/><w:lvlText w:val=\"%2)\"/><w:pPr><w:ind w:left=\"1440\" w:hanging=\"360\"/></w:pPr></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
    QByteArray body;
    for (int i = 0; i < 4; ++i) body += "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"" + QByteArray::number(i % 2) + "\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:t>item " + QByteArray::number(i) + "</w:t></w:r></w:p>";
    body += "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:bookmarkStart w:id=\"1\" w:name=\"a\"/><w:r><w:t>A heading</w:t></w:r><w:bookmarkEnd w:id=\"1\"/></w:p>";
    body += paragraphs(6, 40);
    // A table of cells joined across and down, with a header row, a cell with two paragraphs, a table in a cell, and a bookmark between the rows.
    body += "<w:tbl><w:tblPr><w:tblStyle w:val=\"Grid\"/><w:tblW w:w=\"6000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/><w:gridCol w:w=\"2000\"/></w:tblGrid>"
            "<w:tr><w:trPr><w:tblHeader/></w:trPr><w:tc><w:p><w:r><w:t>head a</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>head b</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>head c</w:t></w:r></w:p></w:tc></w:tr>"
            "<w:bookmarkStart w:id=\"2\" w:name=\"inTable\"/><w:bookmarkEnd w:id=\"2\"/>"
            "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"4000\" w:type=\"dxa\"/><w:gridSpan w:val=\"2\"/></w:tcPr><w:p><w:r><w:t>wide</w:t></w:r></w:p><w:p><w:r><w:t>second paragraph of the wide cell</w:t></w:r></w:p></w:tc>"
            "<w:tc><w:tcPr><w:vMerge w:val=\"restart\"/></w:tcPr><w:p><w:r><w:t>tall</w:t></w:r></w:p></w:tc></w:tr>"
            "<w:tr><w:tc><w:p><w:r><w:t>one</w:t></w:r></w:p></w:tc><w:tc><w:tbl><w:tblGrid><w:gridCol w:w=\"1000\"/><w:gridCol w:w=\"1000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>in</w:t></w:r></w:p></w:tc><w:tc><w:p><w:r><w:t>side</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p/></w:tc>"
            "<w:tc><w:tcPr><w:vMerge/></w:tcPr><w:p/></w:tc></w:tr></w:tbl>";
    body += "<w:p><w:r><w:t>before break </w:t></w:r><w:r><w:br w:type=\"page\"/></w:r><w:r><w:t>after break</w:t></w:r></w:p>";
    body += "<w:p><w:pPr><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr></w:pPr><w:r><w:t>end of one</w:t></w:r></w:p>";
    body += paragraphs(5, 30);
    const QByteArray gridStyle =
        "<w:style w:type=\"table\" w:styleId=\"Grid\"><w:name w:val=\"Grid\"/><w:pPr><w:spacing w:after=\"0\"/></w:pPr><w:tblPr><w:tblBorders><w:top w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
        "<w:insideH w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:insideV w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/></w:tblBorders></w:tblPr>"
        "<w:tblStylePr w:type=\"firstRow\"><w:rPr><w:b/></w:rPr><w:tcPr><w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"DDDDDD\"/></w:tcPr></w:tblStylePr></w:style>";
    const int seeds = qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") > 0 ? qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") : 8;   // (more, to look further)
    for (unsigned seed = std::max(1, qEnvironmentVariableIntValue("JW_FUZZ_FROM")); seed <= unsigned(seeds); ++seed) {
        auto doc = open(makeDocx(body, kStyles + gridStyle, numbering));
        QVERIFY(doc);
        std::mt19937 rng(seed);
        QTextCursor c(doc->story());
        auto pick = [&](int n) { return int(rng() % unsigned(std::max(1, n))); };
        auto place = [&] { c.setPosition(pick(doc->story()->characterCount() - 1)); };
        auto selectSome = [&] {
            const int a = pick(doc->story()->characterCount() - 1), len = pick(60);
            c.setPosition(a);
            c.setPosition(std::min(a + len, doc->story()->characterCount() - 1), QTextCursor::KeepAnchor);
        };
        QTextDocumentFragment held;
        QStringList log;
        for (int step = 0; step < 300; ++step) {
            const int op = pick(23);
            log << QStringLiteral("%1:op%2 pos=%3 anchor=%4 blocks=%5").arg(step).arg(op).arg(c.position()).arg(c.anchor()).arg(doc->story()->blockCount());
            switch (op) {
            case 0: place(); typeText(*doc, c, QStringLiteral("word ")); break;
            case 1: place(); insertParagraph(*doc, c); break;
            case 2: place(); deleteBack(*doc, c, pick(2)); break;
            case 3: place(); deleteForward(*doc, c, pick(2)); break;
            case 4: selectSome(); removeSelection(*doc, c); break;
            case 5: selectSome(); toggleBold(*doc, c); toggleItalic(*doc, c); setFontSize(*doc, c, 9 + pick(20)); break;
            case 6: selectSome(); toggleList(*doc, c, pick(2)); break;
            case 7: place(); applyParagraphStyle(*doc, c, pick(3) == 0 ? QStringLiteral("Heading1") : QStringLiteral("Normal")); break;
            case 8: place(); insertPageBreak(*doc, c); break;
            case 9: if (pick(4) == 0) { place(); insertSectionBreak(*doc, c); } break;
            case 10: doc->story()->undo(&c); break;
            case 11: doc->story()->redo(&c); break;
            case 12: selectSome(); changeIndent(*doc, c, pick(2) ? 36 : -36); break;
            case 13: selectSome(); setAlignment(*doc, c, pick(2) ? Qt::AlignRight : Qt::AlignJustify); break;
            case 14: selectSome(); held = selectionFragment(c); break;
            case 15: place(); pasteFragment(*doc, c, held); break;
            case 17: case 18: case 19: case 20: case 21: case 22: tableCommand(*doc, c, op - 17, pick); break;
            default: place(); moveCursor(c, pick(2) ? QTextCursor::NextCharacter : QTextCursor::PreviousCharacter, pick(2), 1 + pick(3)); break;
            }
            QVERIFY(doc->story()->blockCount() >= 1);
            // A paragraph that stands for a piece kept as XML holds that and nothing else.
            for (QTextBlock bk = doc->story()->begin(); bk.isValid(); bk = bk.next())
                if (isLocked(bk) && bk.length() != 2) QFAIL(qPrintable(QStringLiteral("seed %1: after %2 the stand-in holds %3 characters\nlast steps:\n%4").arg(seed).arg(log.last()).arg(bk.length() - 1).arg(log.mid(std::max<qsizetype>(0, log.size() - 5)).join(QLatin1Char('\n')))));
            if (step % 12 != 0) continue;
            // The document is whole: sections as the break paragraphs say, pages that can be made, a file that opens to the same.
            int breaks = 0;
            for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next()) breaks += b.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
            QCOMPARE(doc->sectionCount(), breaks + 1);
            const int pages = doc->pager().pageCount();
            QVERIFY(pages >= 1);
            QVERIFY(doc->pager().materialize(pick(pages)));
            QString err;
            const QByteArray zip = doc->toDocx(&err);
            QVERIFY2(!zip.isEmpty(), qPrintable(QStringLiteral("seed %1 step %2: %3").arg(seed).arg(step).arg(err)));
            QVERIFY2(rt::strictProblems(zip).isEmpty(), qPrintable(QStringLiteral("seed %1 step %2: %3").arg(seed).arg(step).arg(rt::strictProblems(zip).join(QLatin1Char(';')))));
            auto back = open(zip);
            QVERIFY2(back, qPrintable(QStringLiteral("seed %1 step %2").arg(seed).arg(step)));
            QString a = doc->story()->toPlainText(), b = back->story()->toPlainText();
            a.remove(QChar::ObjectReplacementCharacter);
            b.remove(QChar::ObjectReplacementCharacter);
            if (a != b) {
                int d = 0;
                while (d < a.size() && d < b.size() && a[d] == b[d]) ++d;
                const QByteArray x = documentXml(zip);
                QString dump;
                for (QTextBlock bk = doc->story()->begin(); bk.isValid(); bk = bk.next()) {
                    const QString t = bk.text();
                    dump += QStringLiteral("[%1 len %2 locked=%3 sect=%5] %6\n").arg(bk.blockNumber()).arg(bk.length()).arg(isLocked(bk)).arg(bk.blockFormat().boolProperty(flow::fp::SectionBreakAfter)).arg(t.left(30).replace(QChar::ObjectReplacementCharacter, QLatin1Char('#')));
                }
                qWarning("%s", qPrintable(dump));
                QFAIL(qPrintable(QStringLiteral("seed %1 step %2: the text differs at %3 (blocks %6 and %7):\n%4\n---\n%5\n=== xml:\n%8").arg(seed).arg(step).arg(d).arg(a.mid(std::max(0, d - 60), 120).toHtmlEscaped(), b.mid(std::max(0, d - 60), 120).toHtmlEscaped()).arg(doc->story()->blockCount()).arg(back->story()->blockCount()).arg(QString::fromUtf8(x.mid(std::max<qsizetype>(0, x.indexOf("judgment evidence court of.") - 1500), 3500)))));
            }
            QCOMPARE(back->sectionCount(), doc->sectionCount());
            // (And what it writes again is what it wrote: nothing drifts.)
            const QByteArray x1 = documentXml(zip), x2 = documentXml(back->toDocx());
            if (x1 != x2) {
                int d = 0;
                while (d < x1.size() && d < x2.size() && x1[d] == x2[d]) ++d;
                QFAIL(qPrintable(QStringLiteral("seed %1 step %2: drift at %3:\n%4\n---\n%5").arg(seed).arg(step).arg(d).arg(QString::fromUtf8(x1.mid(std::max(0, d - 200), 400)), QString::fromUtf8(x2.mid(std::max(0, d - 200), 400)))));
            }
        }
    }
}


void WriteTests::theBlankDocumentSavesAndOpensAndKeepsItsStyles()
{
    auto doc = WriteDoc::blank();
    QTextCursor c(doc->story());
    typeText(*doc, c, QStringLiteral("Title\nBody"));
    c.setPosition(0);
    applyParagraphStyle(*doc, c, QStringLiteral("Title"));
    const QByteArray zip = doc->toDocx();
    office::Package p;
    QVERIFY(p.read(zip));
    QVERIFY(p.hasPart(QStringLiteral("/word/styles.xml")));
    QVERIFY(p.part(QStringLiteral("/word/styles.xml")).contains("w:styleId=\"Title\""));
    QVERIFY(p.part(QStringLiteral("/word/styles.xml")).contains("w:styleId=\"Heading1\""));
    auto back = open(zip);
    QCOMPARE(back->story()->toPlainText(), QStringLiteral("Title\nBody"));
    QCOMPARE(back->story()->firstBlock().blockFormat().stringProperty(tp::StyleName), QStringLiteral("Title"));
    // A style that a file lacks is added when it is first used, and written.
    auto bare = open(makeDocx("<w:p><w:r><w:t>x</w:t></w:r></w:p>"));
    // (It is offered before that, with the look it will have, and offering it changes nothing in the file.)
    bool offered = false;
    for (const StyleEntry &e : bare->styleEntries()) offered = offered || (e.id == QLatin1String("Heading2") && e.quick);
    QVERIFY(offered);
    QVERIFY(bare->styleCharFormat(QStringLiteral("Heading2")).fontPointSize() > 11);
    QVERIFY(!bare->styleSheet().find(QStringLiteral("Heading2")));
    QTextCursor d(bare->story());
    applyParagraphStyle(*bare, d, QStringLiteral("Heading2"));
    QVERIFY(bare->styleSheet().find(QStringLiteral("Heading2")));
    office::Package q;
    QVERIFY(q.read(bare->toDocx()));
    QVERIFY(q.part(QStringLiteral("/word/styles.xml")).contains("w:styleId=\"Heading2\""));
}


// ---- notes: the style and look commands

QList<LookCommand> WriteTests::lookCommands()
{
    return {
        {"style Heading 1", [](WriteDoc &d, QTextCursor &c) { applyParagraphStyle(d, c, QStringLiteral("Heading1")); }},
        {"style Normal", [](WriteDoc &d, QTextCursor &c) { applyParagraphStyle(d, c, d.defaultParagraphStyle()); }},
        {"style Title", [](WriteDoc &d, QTextCursor &c) { applyParagraphStyle(d, c, QStringLiteral("Title")); }},
        {"clear formatting", [](WriteDoc &d, QTextCursor &c) { clearFormatting(d, c); }},
        {"center", [](WriteDoc &d, QTextCursor &c) { setAlignment(d, c, Qt::AlignHCenter); }},
        {"indent", [](WriteDoc &d, QTextCursor &c) { changeIndent(d, c, 36); }},
        {"indents", [](WriteDoc &d, QTextCursor &c) { setIndents(d, c, 20, 10, 5); }},
        {"spacing", [](WriteDoc &d, QTextCursor &c) { setSpacing(d, c, 6, 6); }},
        {"line spacing", [](WriteDoc &d, QTextCursor &c) { setLineSpacing(d, c, QTextBlockFormat::ProportionalHeight, 200); }},
        {"bullets", [](WriteDoc &d, QTextCursor &c) { toggleList(d, c, false); }},
        {"numbers", [](WriteDoc &d, QTextCursor &c) { toggleList(d, c, true); }},
        {"paragraph dialog", [](WriteDoc &d, QTextCursor &c) { changeBlockFormat(d, c, [](QTextBlockFormat &bf) { bf.setAlignment(Qt::AlignRight); bf.setTopMargin(4); }); }},
        {"bold", [](WriteDoc &d, QTextCursor &c) { toggleBold(d, c); }},
        {"character style", [](WriteDoc &d, QTextCursor &c) { applyCharacterStyle(d, c, QStringLiteral("Strong")); }},
        {"font size", [](WriteDoc &d, QTextCursor &c) { setFontSize(d, c, 14); }},
    };
}


// Which paragraphs of the story are tails (endnotes the pages set), and of which copy, in order.
QString WriteTests::tailShape(WriteDoc &doc)
{
    QString out;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next())
        out += b.blockFormat().hasProperty(prop::EndnoteBlock) ? QStringLiteral("E%1 ").arg(b.blockFormat().intProperty(prop::EndnoteNote)) : QStringLiteral("- ");
    return out;
}


// The words of the note with this Id in a saved notes part, its runs joined; the elements of the part by tag ("footnote" or "endnote").
QString WriteTests::notePartText(const QByteArray &part, const char *tag, int id)
{
    const QString x = QString::fromUtf8(part);
    const QRegularExpression note(QStringLiteral("<w:%1 w:id=\"%2\"[^>]*>(.*?)</w:%1>").arg(QLatin1String(tag)).arg(id), QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch m = note.match(x);
    if (!m.hasMatch()) return QString();
    QString out;
    static const QRegularExpression text(QStringLiteral("<w:t[^>]*>([^<]*)</w:t>"));
    for (auto n = text.globalMatch(m.captured(1)); n.hasNext();) out += n.next().captured(1);
    return out;
}


// Each command of the look, with the caret in an endnote's words or a selection in them: the paragraph stays one of its note (the text of the document is as it was, the file's text
// has none of the note's words, and the notes part has them), and an undo gives the file's own notes back.
void WriteTests::styleAndLookCommandsKeepAnEndnoteParagraphAnEndnote()
{
    const QByteArray zip = notedDocx();
    for (const LookCommand &command : lookCommands())
        for (int selected = 0; selected < 2; ++selected) {
            const QString what = QString::fromLatin1(command.name) + (selected ? QStringLiteral(" (selection)") : QStringLiteral(" (caret)"));
            auto doc = open(zip);
            QVERIFY(doc);
            const QString body = savedBody(*doc);
            const QString shape = tailShape(*doc);
            QTextCursor c(doc->story());
            const QTextBlock alpha = doc->story()->findBlockByNumber(3);
            c.setPosition(alpha.position() + 4);
            if (selected) c.setPosition(alpha.position() + 9, QTextCursor::KeepAnchor);
            command.run(*doc, c);
            QVERIFY2(tailShape(*doc) == shape, qPrintable(what + QStringLiteral(": the paragraphs of the notes are now ") + tailShape(*doc) + QStringLiteral(", were ") + shape));
            QVERIFY2(savedBody(*doc) == body, qPrintable(what + QStringLiteral(": the text of the document is now ") + savedBody(*doc)));
            const QByteArray saved = doc->toDocx();
            QVERIFY2(!documentXml(saved).contains("<w:endnoteRef/>"), qPrintable(what + QStringLiteral(": a note's mark is in the text")));
            QVERIFY2(!documentXml(saved).contains("Alpha"), qPrintable(what + QStringLiteral(": a note's words are in the text")));
            QVERIFY2(notePartText(notesPartOf(saved), "endnote", 2).contains(QLatin1String(" Alpha body note brwon")), qPrintable(what + QStringLiteral(": ") + notePartText(notesPartOf(saved), "endnote", 2)));
            QVERIFY2(rt::strictProblems(saved, zip).isEmpty(), qPrintable(what));
            while (doc->canUndo()) doc->undo();
            QVERIFY2(tailShape(*doc) == shape, qPrintable(what + QStringLiteral(" after undo")));
            QVERIFY2(notesPartOf(doc->toDocx()) == notesPartOf(zip), qPrintable(what + QStringLiteral(": the notes part after undo")));
        }
}


// The same with the caret in a footnote's words: the note keeps its mark and its place, the text of the page is as it was, and the notes part has the words.
void WriteTests::styleAndLookCommandsKeepAFootnoteWhole()
{
    const QByteArray zip = threeNotesDocx();
    for (const LookCommand &command : lookCommands())
        for (int selected = 0; selected < 2; ++selected) {
            const QString what = QString::fromLatin1(command.name) + (selected ? QStringLiteral(" (selection)") : QStringLiteral(" (caret)"));
            auto doc = open(zip);
            QVERIFY(doc);
            QTextDocument *note = doc->editableFootnote(3);
            QVERIFY(note);
            const QString body = plain(*doc);
            QTextCursor c(note);
            c.setPosition(noteTextStart(note) + 3);
            if (selected) c.setPosition(noteTextStart(note) + 8, QTextCursor::KeepAnchor);
            command.run(*doc, c);
            QVERIFY2(plain(*doc) == body, qPrintable(what + QStringLiteral(": the text changed")));
            QVERIFY2(note->toPlainText().startsWith(noteMark()), qPrintable(what + QStringLiteral(": the note lost its mark: ") + note->toPlainText()));
            const QByteArray saved = doc->toDocx();
            QVERIFY2(!documentXml(saved).contains("<w:footnoteRef/>"), qPrintable(what + QStringLiteral(": a note's mark is in the text")));
            QVERIFY2(!documentXml(saved).contains("note three"), qPrintable(what + QStringLiteral(": a note's words are in the text")));
            const QByteArray part = partOf(saved, QStringLiteral("/word/footnotes.xml"));
            QVERIFY2(notePartText(part, "footnote", 3).contains(QLatin1String(" note three text")), qPrintable(what + QStringLiteral(": ") + notePartText(part, "footnote", 3)));
            QCOMPARE(int(part.count("<w:footnoteRef/>")), 3);
            QVERIFY2(rt::strictProblems(saved, zip).isEmpty(), qPrintable(what));
            while (doc->canUndo()) doc->undo();
            QVERIFY2(partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")) == partOf(zip, QStringLiteral("/word/footnotes.xml")), qPrintable(what + QStringLiteral(": the notes part after undo")));
        }
}


// A style chosen in a note is the note's: the note's paragraph is written with it, the other notes as they were; Ctrl+Shift+N and Clear All Formatting in the window do the same.
void WriteTests::aStyleChosenInANoteIsTheNotesAndIsSavedInIt()
{
    {
        const QByteArray zip = notedDocx();
        auto doc = open(zip);
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->findBlockByNumber(3).position() + 4);
        applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
        const QByteArray part = notesPartOf(doc->toDocx());
        const int at = part.indexOf("<w:endnote w:id=\"2\"");
        QVERIFY(at >= 0);
        const QByteArray note = part.mid(at, part.indexOf("</w:endnote>", at) - at);
        QVERIFY2(note.contains("<w:pStyle w:val=\"Heading1\"/>"), note.constData());
        QVERIFY2(part.contains(betaNote()), part.constData());
        QCOMPARE(endnoteParagraphs(*doc, 2).size(), 1);
        QCOMPARE(endnoteParagraphs(*doc, 2).first().blockFormat().stringProperty(prop::EndnoteStyle), QStringLiteral("Heading1"));
    }
    {
        const QByteArray zip = notedDocx();
        WriteWindow w;
        w.resize(1000, 900);
        w.show();
        w.setDocument(open(zip), QString());
        WriteDoc *doc = w.document();
        QTextCursor c(doc->story());
        c.setPosition(doc->story()->findBlockByNumber(3).position() + 4);
        w.view()->setCursor(c);
        const QString shape = tailShape(*doc);
        w.act("style.heading2")->trigger();
        QCOMPARE(tailShape(*doc), shape);
        w.act("style.normal")->trigger();
        QCOMPARE(tailShape(*doc), shape);
        w.act("fmt.clear")->trigger();
        QCOMPARE(tailShape(*doc), shape);
        const QByteArray saved = doc->toDocx();
        QVERIFY(!documentXml(saved).contains("<w:endnoteRef/>"));
        QVERIFY(!documentXml(saved).contains("Alpha"));
        QVERIFY(notePartText(notesPartOf(saved), "endnote", 2).contains(QLatin1String(" Alpha body note brwon")));
    }
    {
        const QByteArray zip = threeNotesDocx();
        auto doc = open(zip);
        QVERIFY(doc);
        QTextDocument *note = doc->editableFootnote(3);
        QTextCursor c(note);
        c.setPosition(noteTextStart(note) + 3);
        applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
        const QByteArray part = partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml"));
        const int at = part.indexOf("<w:footnote w:id=\"3\"");
        QVERIFY(at >= 0);
        QVERIFY(part.mid(at, part.indexOf("</w:footnote>", at) - at).contains("<w:pStyle w:val=\"Heading1\"/>"));
    }
}


// ---- notes and headers: one mode at a time

// A document with notes, no header yet, for the tests of entering a header or a footer from a note.
QByteArray WriteTests::notesAndHeaderDocx() { return threeNotesDocx(); }


// Insert > Header and Insert > Footer while a note is edited: the note is left, the header is entered, and the two Escapes that follow end in the text with its own caret.
void WriteTests::insertingAHeaderWhileAFootnoteIsEditedLeavesTheNote()
{
    WriteWindow w;
    w.resize(1300, 1000);
    w.show();
    w.setDocument(open(notesAndHeaderDocx()), QString());
    PageView *v = w.view();
    v->paginateAll();
    QTextCursor at(w.document()->story());
    at.setPosition(3);
    v->setCursor(at);
    for (const char *command : {"insert.header", "insert.footer"}) {
        QVERIFY(v->enterFootnote(3));
        QVERIFY(v->inNote());
        w.act(command)->trigger();
        QVERIFY2(!(v->inNote() && v->inHeaderFooter()), command);
        QVERIFY2(v->inHeaderFooter(), command);
        QVERIFY(!v->inNote());
        QCOMPARE(v->cursor().document(), v->headerFooterEdit().story);
        QTest::keyClick(v, Qt::Key_Escape);
        QVERIFY(!v->inHeaderFooter());
        QVERIFY(!v->inNote());
        QCOMPARE(v->cursor().document(), w.document()->story());
        QCOMPARE(v->cursor().position(), 3);   // (the place in the text that was left for the note)
        QTest::keyClick(v, Qt::Key_Escape);
        QCOMPARE(v->cursor().document(), w.document()->story());
        QVERIFY(!v->inNote() && !v->inHeaderFooter());
    }
    // Typing after all that is typing in the text.
    const QString before = plain(*w.document());
    QTest::keyClicks(v, "Q");
    QCOMPARE(plain(*w.document()).size(), before.size() + 1);
}


// Entering a header or footer, or a note, leaves the other: by the commands, and by the calls the view's other paths make.
void WriteTests::enteringANoteOrAHeaderLeavesTheOtherMode()
{
    auto doc = open(notesAndHeaderDocx());
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 1200);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    QTextCursor at(doc->story());
    at.setPosition(5);
    view.setCursor(at);
    QVERIFY(view.enterFootnote(2));
    QVERIFY(view.inNote());
    QVERIFY(view.enterHeaderFooter(0, false, true));
    QVERIFY(view.inHeaderFooter());
    QVERIFY(!view.inNote());
    QVERIFY(view.enterFootnote(3));
    QVERIFY(view.inNote());
    QVERIFY(!view.inHeaderFooter());
    QVERIFY(view.enterFootnote(2));
    QVERIFY(view.enterHeaderFooterAtCaret(true));
    QVERIFY(view.inHeaderFooter() && !view.inNote());
    view.leaveHeaderFooter();
    QCOMPARE(view.cursor().document(), doc->story());
    QCOMPARE(view.cursor().position(), 5);
    QVERIFY(view.enterFootnote(2));
    view.leaveNote();
    QCOMPARE(view.cursor().document(), doc->story());
    view.leaveNote();
    view.leaveHeaderFooter();
    QCOMPARE(view.cursor().document(), doc->story());
    // From the caret in an endnote's words (a place in the text) a header is entered and left as from any place.
    auto ended = open(notedDocx());
    QVERIFY(ended);
    PageView other;
    other.resize(1000, 1200);
    other.show();
    other.setDocument(ended.get());
    other.paginateAll();
    QTextCursor inNote(ended->story());
    inNote.setPosition(ended->story()->findBlockByNumber(3).position() + 4);
    other.setCursor(inNote);
    QVERIFY(other.enterHeaderFooterAtCaret(false));
    QVERIFY(other.inHeaderFooter());
    other.leaveHeaderFooter();
    QCOMPARE(other.cursor().document(), ended->story());
    QCOMPARE(other.cursor().position(), inNote.position());
}


// Random keys, clicks, and commands in the window (the notes, the headers, the clipboard, and the history among them): after each, at most one of the note mode and the header mode is on,
// the caret is in the story of the mode it is in, and in a note the caret is drawn; and what is saved at the end is whole. JW_FUZZ_SEEDS says how many seeds.
void WriteTests::randomInputInTheWindowKeepsOneModeAtATime()
{
    const int seeds = qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") > 0 ? qEnvironmentVariableIntValue("JW_FUZZ_SEEDS") : 6;
    auto lorem = [](int n) {
        static const char *const w[] = {"alpha", "beta", "gamma", "delta"};
        QByteArray out;
        for (int i = 0; i < n; ++i) out += (i ? " " : "") + QByteArray(w[i % 4]) + QByteArray::number(i);
        return out;
    };
    const QByteArray body = citing("First", 2) + paragraphs(8, 40) + citing("Second", 3) + "<w:p><w:r><w:t>Endnote cite</w:t></w:r><w:r><w:endnoteReference w:id=\"1\"/></w:r></w:p>" + paragraphs(30, 40) + citing("Third", 4);
    const QByteArray notes = footnotesPart({{2, plainRun(" " + lorem(500))}, {3, plainRun(" short")}, {4, plainRun(" " + lorem(60))}});
    const QByteArray ends = "<w:endnotes " + QByteArray(kNs) + "><w:endnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:endnote>"
                            "<w:endnote w:id=\"1\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t> endnote words here</w:t></w:r></w:p><w:p><w:r><w:t>second endnote paragraph</w:t></w:r></w:p></w:endnote></w:endnotes>";
    const QByteArray header = headerPart("hdr", {"<w:r><w:t>Header</w:t></w:r>"});
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>" + QByteArray(kPage) + "</w:sectPr>";
    const QByteArray zip = makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}, {QStringLiteral("/word/endnotes.xml"), ends}, {QStringLiteral("/word/header1.xml"), header}},
                                             "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>", noteStyles());
    static const int keys[] = {Qt::Key_Left, Qt::Key_Right, Qt::Key_Up, Qt::Key_Down, Qt::Key_Home, Qt::Key_End, Qt::Key_PageUp, Qt::Key_PageDown, Qt::Key_Backspace, Qt::Key_Delete,
                               Qt::Key_Return, Qt::Key_Tab, Qt::Key_Escape};
    for (int seed = 1; seed <= seeds; ++seed) {
        auto doc = open(zip);
        QVERIFY(doc);
        WriteWindow w;
        w.resize(1300, 1000);
        w.show();
        w.setDocument(std::move(doc), QString());
        PageView *v = w.view();
        WriteDoc *d = w.document();
        v->paginateAll();
        std::mt19937 rng(seed);
        auto pick = [&](int n) { return int(rng() % unsigned(std::max(1, n))); };
        QStringList log;
        for (int step = 0; step < 120; ++step) {
            switch (pick(12)) {
            case 0:
            case 1:
            case 2: {
                const int k = keys[pick(int(sizeof(keys) / sizeof(keys[0])))];
                const Qt::KeyboardModifiers m = pick(3) == 0 ? Qt::ShiftModifier : pick(4) == 0 ? Qt::ControlModifier : Qt::NoModifier;
                QTest::keyClick(v, Qt::Key(k), m);
                log << QStringLiteral("key%1/%2").arg(k, 0, 16).arg(int(m));
                break;
            }
            case 3:
                QTest::keyClicks(v, QStringLiteral("ab c"));
                log << QStringLiteral("type");
                break;
            case 4: {
                const int pg = pick(std::max(1, d->pager().pageCount()));
                const auto page = d->pager().materialize(pg);
                if (!page) break;
                if (!page->noteBoxes.isEmpty() && pick(3)) {
                    const auto nb = page->noteBoxes[pick(int(page->noteBoxes.size()))];
                    if (const Item *it = page->doc->item(nb.itemId)) {
                        QTest::mouseClick(v->viewport(), Qt::LeftButton, pick(4) == 0 ? Qt::ShiftModifier : Qt::NoModifier,
                                          pagePoint(*v, pg, it->rect.left() + 10 + pick(300), it->rect.top() + 3 + pick(std::max(1, int(it->rect.height()) - 4))));
                        log << QStringLiteral("clickNote");
                    }
                } else {
                    QTest::mouseClick(v->viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(*v, pg, 72 + pick(450), 72 + pick(600)));
                    log << QStringLiteral("clickPage");
                }
                break;
            }
            case 5: {
                const QVector<int> ids = d->citedFootnotes();
                if (!ids.isEmpty()) {
                    v->enterFootnote(ids[pick(int(ids.size()))]);
                    log << QStringLiteral("enterFootnote");
                }
                break;
            }
            case 6:
                w.act("refs.footnote")->trigger();
                log << QStringLiteral("insFootnote");
                break;
            case 7:
                w.act("refs.endnote")->trigger();
                log << QStringLiteral("insEndnote");
                break;
            case 8:
                v->undo();
                log << QStringLiteral("undo");
                break;
            case 9:
                v->redo();
                log << QStringLiteral("redo");
                break;
            case 10: {
                const int what = pick(6);
                if (what == 0) v->copy();
                else if (what == 1) v->cut();
                else if (what == 2) v->paste();
                else if (what == 3) v->selectAll();
                else if (what == 4) w.act("fmt.bold")->trigger();
                else w.act("style.normal")->trigger();
                log << QStringLiteral("edit%1").arg(what);
                break;
            }
            default:
                if (pick(2)) {
                    w.act(pick(2) ? "insert.header" : "insert.footer")->trigger();
                    log << QStringLiteral("hdrButton");
                } else {
                    QTest::mouseDClick(v->viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(*v, 0, 300, 40));
                    log << QStringLiteral("dblclickHeader");
                }
                break;
            }
            QCoreApplication::processEvents();
            const QString trail = QStringLiteral("seed %1 step %2 after ").arg(seed).arg(step) + log.mid(std::max<qsizetype>(0, log.size() - 6)).join(QLatin1Char(' '));
            const QTextDocument *cd = v->cursor().document();
            QVERIFY2(v->sideModesExclusive(), qPrintable(trail + QStringLiteral(": both modes are on")));
            if (v->inNote()) QVERIFY2(cd == v->noteEdit().story, qPrintable(trail + QStringLiteral(": in a note, the caret is elsewhere")));
            else if (v->inHeaderFooter()) QVERIFY2(cd == v->headerFooterEdit().story, qPrintable(trail + QStringLiteral(": in a header, the caret is elsewhere")));
            else QVERIFY2(cd == d->story(), qPrintable(trail + QStringLiteral(": in no mode, the caret is not in the text")));
            if (v->inNote()) {
                QRect caret;
                QVERIFY2(v->caretRect(&caret), qPrintable(trail + QStringLiteral(": in a note, the caret has no place")));
            }
        }
        QString err;
        const QByteArray saved = d->toDocx(&err);
        QVERIFY2(!saved.isEmpty(), qPrintable(QStringLiteral("seed %1: %2").arg(seed).arg(err)));
        QVERIFY2(rt::strictProblems(saved, QByteArray()).isEmpty(), qPrintable(QStringLiteral("seed %1: ").arg(seed) + rt::strictProblems(saved, QByteArray()).join(QLatin1Char(' '))));
        QVERIFY2(rt::danglingReferences(saved).isEmpty(), qPrintable(QStringLiteral("seed %1: ").arg(seed) + rt::danglingReferences(saved).join(QLatin1Char(' '))));
    }
}


// ---- footnotes: the caret in a note that is cut over pages or cited twice

// The pages that hold a part of the note with this Id, and the box of each part, in order.
QList<std::pair<int, QRectF>> WriteTests::partsOfNote(WriteDoc &doc, int id)
{
    QList<std::pair<int, QRectF>> out;
    for (int pg = 0; pg < doc.pager().pageCount(); ++pg) {
        QRectF box;
        if (noteBoxOf(doc, pg, id, &box)) out.push_back({pg, box});
    }
    return out;
}


// Whether the caret is drawn in the box of the note on the page the view says the note is edited on.
bool WriteTests::caretIsInBoxOf(PageView &view, WriteDoc &doc, int id)
{
    QRect caret;
    QRectF box;
    if (!view.caretRect(&caret) || !noteBoxOf(doc, view.noteEdit().page, id, &box)) return false;
    const QRect inView(pagePoint(view, view.noteEdit().page, box.left(), box.top()), pagePoint(view, view.noteEdit().page, box.right(), box.bottom()));
    return inView.adjusted(-3, -3, 3, 3).contains(caret);
}


// A note that goes on from one page to the next: a click in its second part puts the caret there, typing leaves the page and the caret where they are, Ctrl+End and Ctrl+Home go to the
// last and the first parts (the view follows), and Down at the last line of a part goes on into the next.
void WriteTests::theCaretStaysInThePartOfANoteThatContinuesOnAnotherPage()
{
    QByteArray words;
    for (int i = 0; i < 2600; ++i) words += " word" + QByteArray::number(i);
    auto doc = open(notesDocx(citing("Start", 2) + paragraphs(4, 30), footnotesPart({{2, plainRun(words)}})));
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 1200);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    const auto parts = partsOfNote(*doc, 2);
    QVERIFY2(parts.size() >= 2, qPrintable(QString::number(parts.size())));
    const int page = parts.last().first;
    const QRectF box = parts.last().second;
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(view, page, box.left() + 30, box.top() + 4));
    QVERIFY(view.inNote());
    QCOMPARE(view.noteEdit().page, page);
    QTest::keyClicks(&view, "X");
    QVERIFY(view.inNote());
    QCOMPARE(view.noteEdit().page, page);
    QVERIFY2(caretIsInBoxOf(view, *doc, 2), "typing in the second part");
    QCOMPARE(view.currentPage(), page);
    const auto after = partsOfNote(*doc, 2);
    QTest::keyClick(&view, Qt::Key_Home, Qt::ControlModifier);
    QVERIFY(view.inNote());
    QCOMPARE(view.noteEdit().page, after.first().first);
    QVERIFY2(caretIsInBoxOf(view, *doc, 2), "Ctrl+Home");
    QTest::keyClick(&view, Qt::Key_End, Qt::ControlModifier);
    QVERIFY(view.inNote());
    QCOMPARE(view.noteEdit().page, after.last().first);
    QVERIFY2(caretIsInBoxOf(view, *doc, 2), "Ctrl+End");
    // Down from the first line to the last: every part of the note is visited, the caret is always drawn, and the note is not left until the last line is passed.
    QTest::keyClick(&view, Qt::Key_Home, Qt::ControlModifier);
    QSet<int> visited{view.noteEdit().page};
    for (int press = 0; press < 400 && view.inNote(); ++press) {
        QTest::keyClick(&view, Qt::Key_Down);
        if (!view.inNote()) break;
        visited.insert(view.noteEdit().page);
        QVERIFY2(caretIsInBoxOf(view, *doc, 2), qPrintable(QStringLiteral("Down, press %1").arg(press)));
    }
    QCOMPARE(int(visited.size()), int(after.size()));
    QVERIFY(!view.inNote());   // (the last line was passed: the caret is in the text, after the reference)
    // And Up from the last line to the first, the same way.
    QVERIFY(view.enterFootnote(2));
    QTest::keyClick(&view, Qt::Key_End, Qt::ControlModifier);
    visited = QSet<int>{view.noteEdit().page};
    for (int press = 0; press < 400 && view.inNote(); ++press) {
        QTest::keyClick(&view, Qt::Key_Up);
        if (!view.inNote()) break;
        visited.insert(view.noteEdit().page);
        QVERIFY2(caretIsInBoxOf(view, *doc, 2), qPrintable(QStringLiteral("Up, press %1").arg(press)));
    }
    QCOMPARE(int(visited.size()), int(after.size()));
    QVERIFY(!view.inNote());
}


// A note cited on two pages is edited on the copy that was clicked: typing and the keys leave the view on that copy, and the caret is drawn in it.
void WriteTests::theCaretStaysOnTheCopyOfANoteThatWasClicked()
{
    auto doc = open(notesDocx(citing("First citation", 2) + paragraphs(60, 40) + citing("Second citation of the same note", 2), footnotesPart({{2, plainRun(" the one note cited twice")}})));
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 1200);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    const auto copies = partsOfNote(*doc, 2);
    QVERIFY2(copies.size() >= 2, qPrintable(QString::number(copies.size())));
    const int page = copies.last().first;
    QVERIFY(page > copies.first().first);
    const QRectF box = copies.last().second;
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(view, page, box.left() + 60, box.top() + 4));
    QVERIFY(view.inNote());
    QCOMPARE(view.noteEdit().page, page);
    for (const char *typed : {"X", "Y"}) {
        QTest::keyClicks(&view, typed);
        QCOMPARE(view.noteEdit().page, page);
        QCOMPARE(view.currentPage(), page);
        QVERIFY2(caretIsInBoxOf(view, *doc, 2), typed);
        QVERIFY(view.viewport()->rect().adjusted(-50, -50, 50, 50).intersects([&] { QRect r; view.caretRect(&r); return r; }()));
    }
    QTest::keyClick(&view, Qt::Key_Home);
    QCOMPARE(view.noteEdit().page, page);
    QTest::keyClick(&view, Qt::Key_End, Qt::ControlModifier);
    QCOMPARE(view.noteEdit().page, page);
    QVERIFY(caretIsInBoxOf(view, *doc, 2));
    // An undo of what was typed there leaves the view where it was.
    view.undo();
    QCOMPARE(view.noteEdit().page, page);
    QVERIFY(caretIsInBoxOf(view, *doc, 2));
    // And Esc goes back to the reference on the page the note was edited on.
    QTest::keyClick(&view, Qt::Key_Escape);
    QVERIFY(!view.inNote());
    QCOMPARE(doc->pager().pageOfPosition(view.cursor().position()), page);
}


// ---- notes: the Ids of a part

// The separator and the continuation separator of a notes part, with the Ids given (producers number them as they like).
QByteArray WriteTests::separatorsAt(const char *tag, int separator, int continuation)
{
    const QByteArray t(tag);
    return "<w:" + t + " w:type=\"separator\" w:id=\"" + QByteArray::number(separator) + "\"><w:p><w:r><w:separator/></w:r></w:p></w:" + t + ">" +
           "<w:" + t + " w:type=\"continuationSeparator\" w:id=\"" + QByteArray::number(continuation) + "\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></w:" + t + ">";
}


// The elements of a notes part, each as its type and its Id ("separator:0", "normal:2").
QStringList WriteTests::noteElementsOf(const QByteArray &part, const char *tag)
{
    QStringList out;
    QXmlStreamReader xml(part);
    xml.setNamespaceProcessing(false);
    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement() || xml.qualifiedName() != QLatin1String("w:") + QLatin1String(tag)) continue;
        const QString type = xml.attributes().value(QStringLiteral("w:type")).toString();
        out << (type.isEmpty() ? QStringLiteral("normal") : type) + QLatin1Char(':') + xml.attributes().value(QStringLiteral("w:id")).toString();
    }
    return out;
}


// A part with separators at Ids 0 and 1 and no note: a note made in it takes an Id above them, the separators are as the file had them, and the note is edited again after the file is
// read again, with the words of the separators untouched.
void WriteTests::aNewNoteTakesAnIdAboveEveryIdOfItsPartTheSeparatorsToo()
{
    for (const bool endnotes : {false, true}) {
        const char *tag = endnotes ? "endnote" : "footnote";
        const QString name = endnotes ? QStringLiteral("/word/endnotes.xml") : QStringLiteral("/word/footnotes.xml");
        const QByteArray part = QByteArray("<w:") + (endnotes ? "endnotes " : "footnotes ") + kNs + ">" + separatorsAt(tag, 0, 1) + (endnotes ? "</w:endnotes>" : "</w:footnotes>");
        const QByteArray zip = makeDocxWithParts("<w:p><w:r><w:t>One paragraph of text</w:t></w:r></w:p>", pageSection(), {{name, part}}, endnotes ? QByteArray("<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>") : QByteArray(), noteStyles());
        auto doc = open(zip);
        QVERIFY(doc);
        QTextCursor c(doc->story());
        c.setPosition(5);
        int id = 0;
        if (endnotes) {
            QVERIFY(insertEndnote(*doc, c));
            QVERIFY(doc->endnoteIdOf(c.block(), &id));
            typeText(*doc, c, QStringLiteral("typed"));
        } else {
            QTextDocument *note = insertFootnote(*doc, c);
            QVERIFY(note);
            QVERIFY(doc->footnoteIdOf(note, &id));
            typeAtEnd(*doc, note, QStringLiteral("typed"));
        }
        QVERIFY2(id >= 2, qPrintable(QStringLiteral("%1: the new note is Id %2").arg(QLatin1String(tag)).arg(id)));
        const QByteArray saved = doc->toDocx();
        const QByteArray written = partOf(saved, name);
        QCOMPARE(noteElementsOf(written, tag), (QStringList{"separator:0", "continuationSeparator:1", QStringLiteral("normal:%1").arg(id)}));
        QVERIFY(written.contains(separatorsAt(tag, 0, 1)));
        // Read again, the note is edited and saved again: the separators keep their bytes and none of the words.
        auto again = open(saved);
        QVERIFY(again);
        if (endnotes) {
            int from = 0, to = 0;
            QVERIFY(again->endnoteWords(id, &from, &to));
            QTextCursor e(again->story());
            e.setPosition(to);
            typeText(*again, e, QStringLiteral("more"));
        } else {
            QTextDocument *note = again->editableFootnote(id);
            QVERIFY(note);
            QVERIFY(note->toPlainText().contains(QLatin1String("typed")));
            typeAtEnd(*again, note, QStringLiteral("more"));
        }
        const QByteArray second = partOf(again->toDocx(), name);
        QVERIFY(second.contains(separatorsAt(tag, 0, 1)));
        QCOMPARE(noteElementsOf(second, tag), noteElementsOf(written, tag));
        QVERIFY2(notePartText(second, tag, id).contains(QLatin1String("typedmore")), qPrintable(notePartText(second, tag, id)));
    }
}


// ---- notes: cited from other places

// A note that the text cites and a header, a text box, or another note cites too is still cited when the text's reference is taken out: it stays in the notes part (and the places
// that cite it still have their references); the note goes when the last place that cites it lets it go.
void WriteTests::aNoteStaysWhileAHeaderATextBoxOrAnotherNoteStillCitesIt()
{
    for (const bool endnotes : {false, true})
        for (int where = 0; where < 3; ++where) {
            if (endnotes && where == 2) continue;   // (the endnote's own note-in-a-note case is the footnote's)
            const QString name = endnotes ? QStringLiteral("endnote") : QStringLiteral("footnote");
            const QByteArray ref = endnotes ? "<w:r><w:endnoteReference w:id=\"2\"/></w:r>" : "<w:r><w:footnoteReference w:id=\"2\"/></w:r>";
            QByteArray body = endnotes ? "<w:p><w:r><w:t>Body one</w:t></w:r>" + ref + "</w:p><w:p><w:r><w:t>Body two</w:t></w:r><w:r><w:endnoteReference w:id=\"3\"/></w:r></w:p>"
                                       : citing("Body one", 2) + citing("Body two", 3);
            QByteArray sect = pageSection();
            QMap<QString, QByteArray> parts;
            if (where == 0) {
                parts[QStringLiteral("/word/header1.xml")] = headerPart("hdr", {"<w:r><w:t>Head</w:t></w:r>" + ref});
                sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>" + QByteArray(kPage) + "</w:sectPr>";
            } else if (where == 1) {
                body = QByteArray("<w:p><w:r><w:t>Body one</w:t></w:r>") + ref + "<w:r><w:pict><v:shape xmlns:v=\"urn:schemas-microsoft-com:vml\" id=\"s1\" style=\"width:100pt;height:50pt\"><v:textbox><w:txbxContent><w:p><w:r><w:t>in the box</w:t></w:r>" +
                       ref + "</w:p></w:txbxContent></v:textbox></v:shape></w:pict></w:r></w:p>" + (endnotes ? QByteArray("<w:p><w:r><w:t>Body two</w:t></w:r><w:r><w:endnoteReference w:id=\"3\"/></w:r></w:p>") : citing("Body two", 3));
            }
            const QByteArray notesXml = endnotes ? "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote() +
                                                       "<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t> note two</w:t></w:r></w:p></w:endnote>"
                                                       "<w:endnote w:id=\"3\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t> note three</w:t></w:r></w:p></w:endnote></w:endnotes>"
                                                 : footnotesPart({{2, plainRun(" note two")}, {3, plainRun(" note three") + (where == 2 ? ref : QByteArray())}});
            parts[endnotes ? QStringLiteral("/word/endnotes.xml") : QStringLiteral("/word/footnotes.xml")] = notesXml;
            const QByteArray zip = makeDocxWithParts(body, sect, parts, endnotes ? QByteArray("<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>") : QByteArray(), noteStyles());
            auto doc = open(zip);
            QVERIFY(doc);
            const QString what = name + QStringLiteral(" cited from ") + (where == 0 ? QStringLiteral("a header") : where == 1 ? QStringLiteral("a text box") : QStringLiteral("a note"));
            const QVector<int> refs = endnotes ? doc->endnoteReferences(2) : doc->footnoteReferences(2);
            QVERIFY2(!refs.isEmpty(), qPrintable(what));
            QTextCursor c(doc->story());
            c.setPosition(refs.first());
            c.setPosition(refs.first() + 1, QTextCursor::KeepAnchor);
            removeSelection(*doc, c);
            QVERIFY2((endnotes ? doc->endnoteReferences(2) : doc->footnoteReferences(2)).size() == refs.size() - 1, qPrintable(what + QStringLiteral(": the reference was not taken out")));
            const QString partName = endnotes ? QStringLiteral("/word/endnotes.xml") : QStringLiteral("/word/footnotes.xml");
            const QByteArray saved = doc->toDocx();
            const QStringList kept = noteElementsOf(partOf(saved, partName), endnotes ? "endnote" : "footnote");
            QVERIFY2(kept.contains(QStringLiteral("normal:2")), qPrintable(what + QStringLiteral(": the note went with the text's reference: ") + kept.join(QLatin1Char(' '))));
            QVERIFY2(kept.contains(QStringLiteral("normal:3")), qPrintable(what));
            QVERIFY2(rt::strictProblems(saved, zip).isEmpty(), qPrintable(what));
            // An undo gives the reference back, and the file is as it was.
            while (doc->canUndo()) doc->undo();
            QCOMPARE(partOf(doc->toDocx(), partName), partOf(zip, partName));
        }
}


// A note cited by a header only (its text's reference gone) goes when the header lets it go.
void WriteTests::aNoteGoesWhenTheLastPlaceThatCitesItLetsItGo()
{
    const QByteArray ref = "<w:r><w:footnoteReference w:id=\"2\"/></w:r>";
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>" + QByteArray(kPage) + "</w:sectPr>";
    const QByteArray zip = makeDocxWithParts(citing("Body one", 2) + citing("Body two", 3), sect,
                                             {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>Head</w:t></w:r>" + ref})}, {QStringLiteral("/word/footnotes.xml"), footnotesPart({{2, plainRun(" two")}, {3, plainRun(" three")}})}},
                                             QByteArray(), noteStyles());
    auto doc = open(zip);
    QVERIFY(doc);
    const QVector<int> refs = doc->footnoteReferences(2);
    QCOMPARE(int(refs.size()), 1);
    QTextCursor c(doc->story());
    c.setPosition(refs.first());
    c.setPosition(refs.first() + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, c);
    QTextDocument *header = doc->editableHeaderFooter(0, false, 0);
    QVERIFY(header);
    const int at = header->toPlainText().indexOf(QChar::ObjectReplacementCharacter);
    QVERIFY(at >= 0);
    QTextCursor h(header);
    h.setPosition(at);
    h.setPosition(at + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, h);
    const QByteArray saved = doc->toDocx();
    const QStringList kept = noteElementsOf(partOf(saved, QStringLiteral("/word/footnotes.xml")), "footnote");
    QVERIFY2(!kept.contains(QStringLiteral("normal:2")) && kept.contains(QStringLiteral("normal:3")), qPrintable(kept.join(QLatin1Char(' '))));
    QVERIFY(rt::strictProblems(saved, zip).isEmpty());
}


// ---- notes: the most the program holds

// Notes put in and taken back, over and over, use up nothing: after more cycles than the most notes, one more can still be put in, and what the document holds is a few.
void WriteTests::insertingAndUndoingNotesOverAndOverNeverFillsTheDocument()
{
    for (const bool endnotes : {false, true}) {
        auto doc = open(endnotes ? notedDocx() : threeNotesDocx());
        QVERIFY(doc);
        const QByteArray before = doc->toDocx();
        for (int cycle = 0; cycle < int(kMostNoteStories) + 100; ++cycle) {
            QTextCursor c(doc->story());
            c.setPosition(3);
            if (endnotes) QVERIFY2(insertEndnote(*doc, c), qPrintable(QStringLiteral("endnote cycle %1").arg(cycle)));
            else QVERIFY2(insertFootnote(*doc, c), qPrintable(QStringLiteral("footnote cycle %1").arg(cycle)));
            doc->undo();
        }
        QVERIFY(endnotes ? doc->roomForEndnote() : doc->roomForFootnote());
        QVERIFY2((endnotes ? doc->heldEndnotesForTest() : doc->heldFootnotesForTest()) <= 5, qPrintable(QString::number(endnotes ? doc->heldEndnotesForTest() : doc->heldFootnotesForTest())));
        QTextCursor c(doc->story());
        c.setPosition(3);
        QVERIFY(endnotes ? insertEndnote(*doc, c) : bool(insertFootnote(*doc, c)));
        while (doc->canUndo()) doc->undo();
        QCOMPARE(documentXml(doc->toDocx()), documentXml(before));
        QCOMPARE(partOf(doc->toDocx(), endnotes ? QStringLiteral("/word/endnotes.xml") : QStringLiteral("/word/footnotes.xml")),
                 partOf(before, endnotes ? QStringLiteral("/word/endnotes.xml") : QStringLiteral("/word/footnotes.xml")));
        // (Taken back and put in again by a redo: the note is the one that was.)
        QTextCursor d(doc->story());
        d.setPosition(3);
        QVERIFY(endnotes ? insertEndnote(*doc, d) : bool(insertFootnote(*doc, d)));
        const QByteArray inserted = doc->toDocx();
        doc->undo();
        doc->redo();
        QCOMPARE(documentXml(doc->toDocx()), documentXml(inserted));
        QCOMPARE(partOf(doc->toDocx(), endnotes ? QStringLiteral("/word/endnotes.xml") : QStringLiteral("/word/footnotes.xml")),
                 partOf(inserted, endnotes ? QStringLiteral("/word/endnotes.xml") : QStringLiteral("/word/footnotes.xml")));
    }
}


// A document with the most notes the program holds (each cited): no more can be made; the commands are off, and the status bar says why. A document with that many notes in its part
// and only a few cited has room: a note can still be made.
void WriteTests::aDocumentWithTheMostNotesTurnsTheCommandsOffAndSaysWhy()
{
    const int count = kMostNoteStories;
    QByteArray body;
    QList<std::pair<int, QByteArray>> notes;
    for (int i = 1; i <= count; ++i) {
        body += "<w:p><w:r><w:t>p" + QByteArray::number(i) + "</w:t></w:r><w:r><w:footnoteReference w:id=\"" + QByteArray::number(i) + "\"/></w:r></w:p>";
        notes << std::make_pair(i, plainRun(" note " + QByteArray::number(i)));
    }
    const QByteArray zip = notesDocx(body, footnotesPart(notes));
    {
        auto doc = open(zip);
        QVERIFY(doc);
        QVERIFY(!doc->roomForFootnote());
        QTextCursor c(doc->story());
        c.setPosition(2);
        QVERIFY(!insertFootnote(*doc, c));
        QVERIFY(c.document() == doc->story());
        QVERIFY(!doc->isModified());
    }
    WriteWindow w;
    w.resize(1000, 900);
    w.show();
    w.setDocument(open(zip), QString());
    QTRY_VERIFY(!w.act("refs.footnote")->isEnabled());
    QTRY_VERIFY2(w.statusBar()->currentMessage().contains(QLocale().toString(count)), qPrintable(w.statusBar()->currentMessage()));
    QVERIFY(w.act("refs.endnote")->isEnabled());   // (the document's endnotes are another count)
    // The same count of endnotes.
    QByteArray ends = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote();
    QByteArray endBody;
    for (int i = 1; i <= count; ++i) {
        ends += "<w:endnote w:id=\"" + QByteArray::number(i) + "\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t> e" + QByteArray::number(i) + "</w:t></w:r></w:p></w:endnote>";
        endBody += "<w:p><w:r><w:t>q" + QByteArray::number(i) + "</w:t></w:r><w:r><w:endnoteReference w:id=\"" + QByteArray::number(i) + "\"/></w:r></w:p>";
    }
    ends += "</w:endnotes>";
    const QByteArray endZip = makeDocxWithParts(endBody, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), ends}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>");
    auto ended = open(endZip);
    QVERIFY(ended);
    QVERIFY(!ended->roomForEndnote());
    QTextCursor c(ended->story());
    c.setPosition(2);
    QVERIFY(!insertEndnote(*ended, c));
    QVERIFY(!ended->isModified());
}


// ---- endnotes: what an edit costs

// The ids of the endnotes the references of the text name, and of those the pages set, each in order (they are the same list when the notes are set as the text asks).
QStringList WriteTests::endnoteIdsOfReferences(WriteDoc &doc)
{
    QStringList out;
    for (const QString &r : endnoteReferenceNumbers(doc)) out << r.section(QLatin1Char(':'), 0, 0);
    return out;
}

QStringList WriteTests::endnoteIdsOfTails(WriteDoc &doc)
{
    QStringList out;
    for (const QString &r : endnoteMarks(doc)) out << r.section(QLatin1Char(':'), 0, 0);
    return out;
}


// Typing, Enter, and Backspace in a long text with an endnotes part cost what the edit touched: the story is not walked again to see whether the notes the text names are those the
// pages set (the references are followed as the story changes); and the notes are still set again, as the text asks, when an edit takes a reference out, puts one in, or moves one.
void WriteTests::settlingTheEndnotesCostsWhatTheEditTouchedNotWhatTheStoryHolds()
{
    QByteArray body = paragraphs(3000, 8);
    body += "<w:p><w:r><w:t>First cite</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>" + paragraphs(5, 8) + "<w:p><w:r><w:t>Second cite</w:t></w:r><w:r><w:endnoteReference w:id=\"3\"/></w:r></w:p>";
    const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote() + "<w:endnote w:id=\"2\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Alpha note</w:t></w:r></w:p></w:endnote>" + betaNote() + "</w:endnotes>";
    auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>"));
    QVERIFY(doc);
    QCOMPARE(endnoteIdsOfReferences(*doc), endnoteIdsOfTails(*doc));
    QTextCursor c(doc->story());
    c.setPosition(doc->story()->findBlockByNumber(1500).position() + 3);
    insertParagraph(*doc, c);   // (the first edits that can change the notes: the references are looked for once)
    deleteBack(*doc, c, false);
    resetEndnoteSettleWorkForTest();
    for (int i = 0; i < 40; ++i) insertText(*doc, c, QStringLiteral("x"));
    for (int i = 0; i < 20; ++i) insertParagraph(*doc, c);
    for (int i = 0; i < 20; ++i) deleteBack(*doc, c, false);
    for (int i = 0; i < 10; ++i) deleteForward(*doc, c, false);
    QVERIFY2(endnoteSettleScansForTest() == 0, qPrintable(QStringLiteral("%1 walks over the story, %2 paragraphs looked at").arg(endnoteSettleScansForTest()).arg(endnoteSettleVisitedForTest())));
    QVERIFY2(endnoteSettleVisitedForTest() < 400, qPrintable(QString::number(endnoteSettleVisitedForTest())));
    QCOMPARE(endnoteIdsOfReferences(*doc), endnoteIdsOfTails(*doc));
    // Edits that change the references are followed: taking one out, putting one in, and moving one set the notes again.
    auto step = [&](const QString &what) {
        const bool same = endnoteIdsOfReferences(*doc) == endnoteIdsOfTails(*doc);
        if (!same)
            QTest::qFail(qPrintable(what + QStringLiteral(": the references name ") + endnoteIdsOfReferences(*doc).join(QLatin1Char(' ')) + QStringLiteral(", the pages set ") + endnoteIdsOfTails(*doc).join(QLatin1Char(' '))), __FILE__, __LINE__);
        const bool followed = doc->endnoteSequenceMatchesStoryForTest();
        if (!followed) QTest::qFail(qPrintable(what + QStringLiteral(": the references followed are not those the story has")), __FILE__, __LINE__);
        return same && followed;
    };
    QVERIFY(step(QStringLiteral("typing and Enter")));
    const QVector<int> refs = doc->endnoteReferences(2);
    QCOMPARE(int(refs.size()), 1);
    QTextCursor d(doc->story());
    d.setPosition(refs.first());
    d.setPosition(refs.first() + 1, QTextCursor::KeepAnchor);
    const Clip clip = copySelection(*doc, d);
    removeSelection(*doc, d);
    QVERIFY(step(QStringLiteral("a reference taken out")));
    QCOMPARE(endnoteIdsOfTails(*doc), (QStringList{"3"}));
    QTextCursor e(doc->story());
    e.setPosition(doc->story()->findBlockByNumber(10).position() + 2);
    pasteClip(*doc, e, clip);
    QVERIFY(step(QStringLiteral("a reference pasted")));
    QCOMPARE(int(endnoteIdsOfTails(*doc).size()), 2);
    QTextCursor f(doc->story());
    f.setPosition(doc->story()->findBlockByNumber(20).position() + 1);
    QVERIFY(insertEndnote(*doc, f));
    QVERIFY(step(QStringLiteral("a note inserted")));
    QCOMPARE(int(endnoteIdsOfTails(*doc).size()), 3);
    QVERIFY2(endnoteSettleScansForTest() >= 1, "the notes were not set again");
    while (doc->canUndo()) {
        doc->undo();
        QVERIFY(step(QStringLiteral("an undo")));
    }
    while (doc->canRedo()) {
        doc->redo();
        QVERIFY(step(QStringLiteral("a redo")));
    }
    QCOMPARE(int(endnoteIdsOfTails(*doc).size()), 3);
    // Random edits at and beside the references, and what undoes and does them again: the references followed are always those the story has, and the pages set the notes they name.
    std::mt19937 rng(11);
    for (int i = 0; i < 300; ++i) {
        QVector<int> places;
        for (QTextBlock b = doc->story()->begin(); b.isValid() && b.position() < editableEnd(doc->story()); b = b.next())
            if (b.text().contains(QChar::ObjectReplacementCharacter)) places << b.position() + int(b.text().indexOf(QChar::ObjectReplacementCharacter));
        auto somewhere = [&] {
            const int end = std::max(1, editableEnd(doc->story()) - 1);
            if (!places.isEmpty() && rng() % 3 != 0) return std::clamp(places[rng() % unsigned(places.size())] + int(rng() % 3) - 1, 0, end);
            return int(rng() % unsigned(end));
        };
        QTextCursor r(doc->story());
        r.setPosition(somewhere());
        QString what;
        switch (rng() % 9) {
        case 0: what = QStringLiteral("type"); insertText(*doc, r, QStringLiteral("x")); break;
        case 1: what = QStringLiteral("Enter"); insertParagraph(*doc, r); break;
        case 2: what = QStringLiteral("Backspace"); deleteBack(*doc, r, false); break;
        case 3: what = QStringLiteral("Delete"); deleteForward(*doc, r, false); break;
        case 4: what = QStringLiteral("delete three"); r.setPosition(std::min(r.position() + 3, editableEnd(doc->story()) - 1), QTextCursor::KeepAnchor); removeSelection(*doc, r); break;
        case 5: what = QStringLiteral("insert an endnote"); insertEndnote(*doc, r); break;
        case 6: {
            what = QStringLiteral("copy and paste a reference");
            if (places.isEmpty()) break;
            QTextCursor from(doc->story());
            from.setPosition(places[rng() % unsigned(places.size())]);
            from.setPosition(from.position() + 1, QTextCursor::KeepAnchor);
            const Clip piece = copySelection(*doc, from);
            pasteClip(*doc, r, piece);
            break;
        }
        case 7: what = QStringLiteral("undo"); doc->undo(); break;
        default: what = QStringLiteral("redo"); doc->redo(); break;
        }
        QVERIFY(step(QStringLiteral("random edit %1 (%2)").arg(i).arg(what)));
    }
}


// ---- notes: a paragraph that carries the properties of a section

// The section properties a producer wrote into a note's paragraph (a note has no sections, but a file may say so), as the program writes them.
QByteArray WriteTests::sectionAsWritten(const QByteArray &sectPr)
{
    auto doc = open(makeDocx("<w:p><w:pPr>" + sectPr + "</w:pPr><w:r><w:t>a</w:t></w:r></w:p><w:p><w:r><w:t>b</w:t></w:r></w:p>"));
    const QByteArray part = documentXml(doc->toDocx());
    const int at = part.indexOf("<w:sectPr");
    return part.mid(at, part.indexOf("</w:sectPr>", at) + 11 - at);
}

QByteArray WriteTests::firstSectionIn(const QByteArray &xml)
{
    const int at = xml.indexOf("<w:sectPr");
    return at < 0 ? QByteArray() : xml.mid(at, xml.indexOf("</w:sectPr>", at) + 11 - at);
}


// A note's paragraph that has a section's properties keeps them when the note is edited and saved, as they were read (in a document of two sections, whose own are not put in their place),
// and the file the note makes is the same when it is read and saved again.
void WriteTests::aSectionInANotesParagraphIsKeptWhenTheNoteIsEdited()
{
    const QByteArray landscape = "<w:sectPr><w:pgSz w:w=\"15840\" w:h=\"12240\" w:orient=\"landscape\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/><w:cols w:num=\"2\" w:space=\"360\"/></w:sectPr>";
    const QByteArray kept = sectionAsWritten(landscape);
    QVERIFY2(kept.contains("w:orient=\"landscape\""), kept.constData());
    const QByteArray firstSection = "<w:p><w:pPr><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr></w:pPr><w:r><w:t>One</w:t></w:r></w:p>";
    for (const bool endnotes : {false, true}) {
        const QString name = endnotes ? QStringLiteral("/word/endnotes.xml") : QStringLiteral("/word/footnotes.xml");
        const QByteArray first = endnotes ? "<w:endnote w:id=\"2\">" : "<w:footnote w:id=\"2\">";
        const QByteArray last = endnotes ? "</w:endnote>" : "</w:footnote>";
        const QByteArray paragraphs = "<w:p><w:r><w:" + QByteArray(endnotes ? "endnoteRef" : "footnoteRef") + "/></w:r><w:r><w:t xml:space=\"preserve\"> first</w:t></w:r></w:p>"
                                      "<w:p><w:pPr>" + kept + "</w:pPr><w:r><w:t>second</w:t></w:r></w:p>";
        const QByteArray part = endnotes ? "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote() + first + paragraphs + last + "</w:endnotes>"
                                         : "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>" + first + paragraphs + last + "</w:footnotes>";
        const QByteArray body = firstSection + (endnotes ? QByteArray("<w:p><w:r><w:t>Two</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>") : citing("Two", 2));
        const QByteArray zip = makeDocxWithParts(body, pageSection(), {{name, part}}, endnotes ? QByteArray("<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>") : QByteArray(), noteStyles());
        auto doc = open(zip);
        QVERIFY(doc);
        QTextCursor c(doc->story());
        if (endnotes) {
            int from = 0, to = 0;
            QVERIFY(doc->endnoteWords(2, &from, &to));
            c.setPosition(from + 2);
            insertText(*doc, c, QStringLiteral("EDIT"));
        } else {
            QTextDocument *note = doc->editableFootnote(2);
            QVERIFY(note);
            c = QTextCursor(note);
            c.setPosition(noteTextStart(note) + 2);
            insertText(*doc, c, QStringLiteral("EDIT"));
        }
        const QByteArray saved = doc->toDocx();
        const QByteArray written = partOf(saved, name);
        const QByteArray what = endnotes ? "endnote: " : "footnote: ";
        QVERIFY2(written.contains("EDIT"), what.constData());
        QVERIFY2(firstSectionIn(written.mid(written.indexOf("w:id=\"2\""))) == kept, qPrintable(QString::fromLatin1(what) + QString::fromUtf8(firstSectionIn(written.mid(written.indexOf("w:id=\"2\""))))));
        QCOMPARE(int(written.count("<w:sectPr")), 1);
        QVERIFY2(!documentXml(saved).contains("w:orient=\"landscape\""), "the note's section went into the text");
        QCOMPARE(int(documentXml(saved).count("<w:sectPr")), 2);   // (the first paragraph's, and the text's last)
        auto again = open(saved);
        QVERIFY(again);
        QCOMPARE(partOf(again->toDocx(), name), written);
        QVERIFY(rt::strictProblems(saved, zip).isEmpty());
    }
    // A header's paragraph is the same.
    const QByteArray header = "<w:hdr " + QByteArray(kNs) + "><w:p><w:r><w:t>Head</w:t></w:r></w:p><w:p><w:pPr>" + kept + "</w:pPr><w:r><w:t>second</w:t></w:r></w:p></w:hdr>";
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>" + QByteArray(kPage) + "</w:sectPr>";
    const QByteArray zip = makeDocxWithParts(firstSection + "<w:p><w:r><w:t>Two</w:t></w:r></w:p>", sect, {{QStringLiteral("/word/header1.xml"), header}});
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *story = doc->editableHeaderFooter(1, false, 0);   // (the header is the last section's)
    QVERIFY(story);
    QTextCursor h(story);
    h.setPosition(2);
    insertText(*doc, h, QStringLiteral("EDIT"));
    const QByteArray written = partOf(doc->toDocx(), QStringLiteral("/word/header1.xml"));
    QVERIFY(written.contains("EDIT"));
    QVERIFY2(firstSectionIn(written) == kept, written.constData());
}


// ---- notes: lists

// Turning a list off in a note gives its paragraph what its style gives: a note's own style (its paragraphs of the endnotes the pages set keep it apart from the document's styles),
// not the style the document has by default.
void WriteTests::turningAListOffInANoteGivesBackTheNotesOwnStyle()
{
    const QByteArray styles = kStyles +
        "<w:style w:type=\"paragraph\" w:styleId=\"EndnoteText\"><w:name w:val=\"endnote text\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:ind w:left=\"720\"/></w:pPr></w:style>"
        "<w:style w:type=\"paragraph\" w:styleId=\"FootnoteText\"><w:name w:val=\"footnote text\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:ind w:left=\"720\"/></w:pPr></w:style>"
        "<w:style w:type=\"character\" w:styleId=\"FootnoteReference\"><w:name w:val=\"footnote reference\"/><w:rPr><w:vertAlign w:val=\"superscript\"/></w:rPr></w:style>";
    {
        const QByteArray notes = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote() +
                                 "<w:endnote w:id=\"2\"><w:p><w:pPr><w:pStyle w:val=\"EndnoteText\"/></w:pPr><w:r><w:endnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Alpha note</w:t></w:r></w:p></w:endnote></w:endnotes>";
        const QByteArray body = "<w:p><w:r><w:t>Body</w:t></w:r><w:r><w:endnoteReference w:id=\"2\"/></w:r></w:p>";
        auto doc = open(makeDocxWithParts(body, sectionXml(6000, 6000, 720), {{QStringLiteral("/word/endnotes.xml"), notes}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>", styles));
        QVERIFY(doc);
        const QTextBlock note = doc->story()->findBlockByNumber(2);
        QCOMPARE(note.blockFormat().leftMargin(), 36.0);
        QTextCursor c(doc->story());
        c.setPosition(note.position() + 4);
        toggleList(*doc, c, false);
        QVERIFY(doc->story()->findBlockByNumber(2).blockFormat().hasProperty(prop::NumId));
        toggleList(*doc, c, false);
        const QTextBlock after = doc->story()->findBlockByNumber(2);
        QVERIFY(!after.blockFormat().hasProperty(prop::NumId));
        QCOMPARE(after.blockFormat().leftMargin(), 36.0);
        QCOMPARE(after.blockFormat().stringProperty(prop::EndnoteStyle), QStringLiteral("EndnoteText"));
    }
    {
        auto doc = open(makeDocxWithParts(citing("Body", 2), pageSection(), {{QStringLiteral("/word/footnotes.xml"), footnotesPart({{2, plainRun(" Alpha note")}})}}, QByteArray(), styles));
        QVERIFY(doc);
        QTextDocument *note = doc->editableFootnote(2);
        QVERIFY(note);
        QCOMPARE(note->firstBlock().blockFormat().leftMargin(), 36.0);
        QTextCursor c(note);
        c.setPosition(noteTextStart(note) + 3);
        toggleList(*doc, c, true);
        QVERIFY(note->firstBlock().blockFormat().hasProperty(prop::NumId));
        toggleList(*doc, c, true);
        QVERIFY(!note->firstBlock().blockFormat().hasProperty(prop::NumId));
        QCOMPARE(note->firstBlock().blockFormat().leftMargin(), 36.0);
    }
}


// ---- footnotes: copies of a note that share a page

// Every box that sets the note with this Id, on every page: the page, the box, and where in the note it begins.
QList<NoteBox> WriteTests::boxesOfNote(WriteDoc &doc, int id)
{
    QList<NoteBox> out;
    for (int pg = 0; pg < doc.pager().pageCount(); ++pg) {
        const auto page = doc.pager().materialize(pg);
        if (!page) continue;
        for (const auto &nb : page->noteBoxes) {
            int found = 0;
            if (!doc.footnoteIdOf(nb.source, &found) || found != id) continue;
            if (const Item *item = page->doc->item(nb.itemId)) out.push_back({pg, item->rect, nb.from});
        }
    }
    return out;
}


// A note cited twice whose first copy goes on over pages, the second citation put so that the second copy begins on a page that the first goes on to: each part is clicked, and the caret
// is in that part (its page, and its box) while the note is typed in and the keys move the caret.
void WriteTests::theCaretStaysInThePartClickedWhereOneCopyGoesOnAndAnotherBegins()
{
    std::unique_ptr<WriteDoc> doc;
    QList<NoteBox> boxes;
    bool shared = false;
    for (int attempt = 0; attempt < 80 && !shared; ++attempt) {
        const int filler = 1 + attempt % 40, wordCount = attempt < 40 ? 1400 : 800;
        QByteArray words;
        for (int i = 0; i < wordCount; ++i) words += " word" + QByteArray::number(i);
        doc = open(notesDocx(citing("First citation", 2) + paragraphs(filler, 40) + citing("Second citation", 2), footnotesPart({{2, plainRun(words)}})));
        QVERIFY(doc);
        PageView probe;
        probe.resize(1000, 1200);
        probe.setDocument(doc.get());
        probe.paginateAll();
        boxes = boxesOfNote(*doc, 2);
        QSet<int> seen;
        for (const NoteBox &b : boxes) {
            if (seen.contains(b.page)) shared = true;
            seen.insert(b.page);
        }
    }
    QVERIFY2(shared, "no layout was found in which two copies of the note share a page");
    PageView view;
    view.resize(1000, 1200);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    boxes = boxesOfNote(*doc, 2);
    QVERIFY(boxes.size() >= 3);
    auto inView = [&](const QRectF &box, int page) { return QRect(pagePoint(view, page, box.left(), box.top()), pagePoint(view, page, box.right(), box.bottom())).adjusted(-3, -3, 3, 3); };
    for (int index = 0; index < boxes.size(); ++index) {
        const NoteBox box = boxesOfNote(*doc, 2).value(index);
        QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(view, box.page, box.rect.left() + 50, box.rect.top() + 5));
        const QString where = QStringLiteral("the part %1 of %2 (page %3, from %4)").arg(index).arg(boxes.size()).arg(box.page).arg(box.from);
        QVERIFY2(view.inNote(), qPrintable(where));
        QCOMPARE(view.noteEdit().page, box.page);
        QRect caret;
        QVERIFY2(view.caretRect(&caret) && inView(box.rect, box.page).contains(caret), qPrintable(where + QStringLiteral(": after the click")));
        QTest::keyClicks(&view, "x");
        const NoteBox again = boxesOfNote(*doc, 2).value(index);
        QVERIFY2(view.caretRect(&caret) && inView(again.rect, again.page).contains(caret), qPrintable(where + QStringLiteral(": after typing")));
        QCOMPARE(view.noteEdit().page, box.page);
        QTest::keyClick(&view, Qt::Key_End);
        QVERIFY2(view.caretRect(&caret) && inView(boxesOfNote(*doc, 2).value(index).rect, box.page).contains(caret), qPrintable(where + QStringLiteral(": after End")));
        QTest::keyClick(&view, Qt::Key_Escape);
        QVERIFY(!view.inNote());
    }
    // From the start of each copy, Down goes through all of it, a line at a time (a part that goes on is followed by the next part of the same copy, though another copy's part may be on
    // its page), and leaves the note only after the note's last line.
    QTextDocument *note = doc->editableFootnote(2);
    QVERIFY(note);
    const int end = note->characterCount() - 1;
    for (int index = 0; index < boxes.size(); ++index) {
        const NoteBox box = boxesOfNote(*doc, 2).value(index);
        if (box.from != 0) continue;
        QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(view, box.page, box.rect.left() + 50, box.rect.top() + 5));
        QVERIFY(view.inNote());
        QTest::keyClick(&view, Qt::Key_Home, Qt::ControlModifier);
        int last = view.cursor().position();
        for (int press = 0; press < 700 && view.inNote(); ++press) {
            QTest::keyClick(&view, Qt::Key_Down);
            if (!view.inNote()) break;
            const int now = view.cursor().position();
            const QString where = QStringLiteral("the copy at page %1, press %2: from %3 to %4").arg(box.page).arg(press).arg(last).arg(now);
            QVERIFY2(now > last && now - last < 600, qPrintable(where));
            last = now;
        }
        QVERIFY2(!view.inNote(), "the note was never left");
        QVERIFY2(last >= end - 300, qPrintable(QStringLiteral("the copy at page %1 was left at %2 of %3").arg(box.page).arg(last).arg(end)));
        view.setCursor(QTextCursor(doc->story()));
    }
}


// ---- notes: what keeps a note in the file

// A header that was edited (it is in the form it is edited in) says what it cites now, not what the part it was read from said: the note it still cites stays when the text's reference goes,
// and the note whose reference was taken out of the header goes with the text's (the part as it was read still names both).
void WriteTests::aNoteStaysWhileAnEditedHeaderStillCitesIt()
{
    const QByteArray ref2 = "<w:r><w:footnoteReference w:id=\"2\"/></w:r>", ref3 = "<w:r><w:footnoteReference w:id=\"3\"/></w:r>";
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/>" + QByteArray(kPage) + "</w:sectPr>";
    const QByteArray zip = makeDocxWithParts(citing("Body one", 2) + citing("Body two", 3), sect,
                                             {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>Head</w:t></w:r>" + ref2 + ref3})}, {QStringLiteral("/word/footnotes.xml"), footnotesPart({{2, plainRun(" two")}, {3, plainRun(" three")}})}},
                                             QByteArray(), noteStyles());
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *header = doc->editableHeaderFooter(0, false, 0);
    QVERIFY(header);
    typeAtEnd(*doc, header, QStringLiteral(" edited"));
    // The header's reference to note 3 is taken out.
    const int at = header->toPlainText().lastIndexOf(QChar::ObjectReplacementCharacter);
    QVERIFY(at >= 0);
    QTextCursor h(header);
    h.setPosition(at);
    h.setPosition(at + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, h);
    for (const int id : {2, 3}) {
        const QVector<int> refs = doc->footnoteReferences(id);
        QCOMPARE(int(refs.size()), 1);
        QTextCursor c(doc->story());
        c.setPosition(refs.first());
        c.setPosition(refs.first() + 1, QTextCursor::KeepAnchor);
        removeSelection(*doc, c);
    }
    const QByteArray saved = doc->toDocx();
    const QStringList kept = noteElementsOf(partOf(saved, QStringLiteral("/word/footnotes.xml")), "footnote");
    QVERIFY2(kept.contains(QStringLiteral("normal:2")), qPrintable(kept.join(QLatin1Char(' ')) + QStringLiteral(": the note went while the edited header cites it")));
    QVERIFY2(!kept.contains(QStringLiteral("normal:3")), qPrintable(kept.join(QLatin1Char(' ')) + QStringLiteral(": the note stayed though the edited header no longer cites it")));
    QVERIFY(partOf(saved, QStringLiteral("/word/header1.xml")).contains("footnoteReference w:id=\"2\""));
    QVERIFY(!partOf(saved, QStringLiteral("/word/header1.xml")).contains("footnoteReference w:id=\"3\""));
    QVERIFY(partOf(saved, QStringLiteral("/word/header1.xml")).contains(" edited"));
    QVERIFY(rt::strictProblems(saved, zip).isEmpty());
}


// Notes that cite only one another are let go with the text's last reference to them: a note that only a note that is let go cites goes too (A cites B and B cites A; A cites B; a footnote
// and an endnote that cite one another), and one that a note that stays cites stays.
void WriteTests::notesThatCiteOnlyOneAnotherGoWithTheTextsLastReference()
{
    const QByteArray fnRef2 = "<w:r><w:footnoteReference w:id=\"2\"/></w:r>", fnRef3 = "<w:r><w:footnoteReference w:id=\"3\"/></w:r>", enRef1 = "<w:r><w:endnoteReference w:id=\"1\"/></w:r>";
    auto savedNotes = [](WriteDoc &doc, const QByteArray &zip, const QString &part, const char *tag) {
        const QByteArray saved = doc.toDocx();
        [&] { QVERIFY(rt::strictProblems(saved, zip).isEmpty()); }();
        return noteElementsOf(partOf(saved, part), tag);
    };
    auto dropBodyReference = [](WriteDoc &doc, int id) {
        const QVector<int> refs = doc.footnoteReferences(id);
        QCOMPARE(int(refs.size()), 1);
        QTextCursor c(doc.story());
        c.setPosition(refs.first());
        c.setPosition(refs.first() + 1, QTextCursor::KeepAnchor);
        removeSelection(doc, c);
    };
    {
        // A cites B and B cites A; the text cites A.
        const QByteArray zip = notesDocx(citing("Body", 2) + citing("Other", 4), footnotesPart({{2, plainRun(" two") + fnRef3}, {3, plainRun(" three") + fnRef2}, {4, plainRun(" four")}}));
        auto doc = open(zip);
        QVERIFY(doc);
        QCOMPARE(savedNotes(*doc, zip, QStringLiteral("/word/footnotes.xml"), "footnote"), (QStringList{"separator:-1", "continuationSeparator:0", "normal:2", "normal:3", "normal:4"}));
        dropBodyReference(*doc, 2);
        QCOMPARE(savedNotes(*doc, zip, QStringLiteral("/word/footnotes.xml"), "footnote"), (QStringList{"separator:-1", "continuationSeparator:0", "normal:4"}));
        while (doc->canUndo()) doc->undo();
        QCOMPARE(partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")), partOf(zip, QStringLiteral("/word/footnotes.xml")));
    }
    {
        // A cites B; the text cites A; B cites nothing.
        const QByteArray zip = notesDocx(citing("Body", 2), footnotesPart({{2, plainRun(" two") + fnRef3}, {3, plainRun(" three")}}));
        auto doc = open(zip);
        QVERIFY(doc);
        dropBodyReference(*doc, 2);
        QCOMPARE(savedNotes(*doc, zip, QStringLiteral("/word/footnotes.xml"), "footnote"), (QStringList{"separator:-1", "continuationSeparator:0"}));
    }
    {
        // The text cites B, and A and B cite one another: when the text lets A go, A stays, for B (which the text cites) cites it.
        const QByteArray zip = notesDocx(citing("Body", 2) + citing("Other", 3), footnotesPart({{2, plainRun(" two") + fnRef3}, {3, plainRun(" three") + fnRef2}}));
        auto doc = open(zip);
        QVERIFY(doc);
        dropBodyReference(*doc, 2);
        QCOMPARE(savedNotes(*doc, zip, QStringLiteral("/word/footnotes.xml"), "footnote"), (QStringList{"separator:-1", "continuationSeparator:0", "normal:2", "normal:3"}));
    }
    {
        // A footnote and an endnote that cite one another.
        const QByteArray body = citing("Body", 2);
        const QByteArray notes = footnotesPart({{2, plainRun(" two") + enRef1}});
        const QByteArray ends = "<w:endnotes " + QByteArray(kNs) + ">" + sourcesNote() + "<w:endnote w:id=\"1\"><w:p><w:r><w:endnoteRef/></w:r><w:r><w:t> one</w:t></w:r>" + fnRef2 + "</w:p></w:endnote></w:endnotes>";
        const QByteArray zip = makeDocxWithParts(body, pageSection(), {{QStringLiteral("/word/footnotes.xml"), notes}, {QStringLiteral("/word/endnotes.xml"), ends}}, "<w:endnotePr><w:numFmt w:val=\"decimal\"/></w:endnotePr>", noteStyles());
        auto doc = open(zip);
        QVERIFY(doc);
        dropBodyReference(*doc, 2);
        const QByteArray saved = doc->toDocx();
        QCOMPARE(noteElementsOf(partOf(saved, QStringLiteral("/word/footnotes.xml")), "footnote"), (QStringList{"separator:-1", "continuationSeparator:0"}));
        QCOMPARE(noteElementsOf(partOf(saved, QStringLiteral("/word/endnotes.xml")), "endnote"), (QStringList{"separator:-1"}));
        QVERIFY(rt::strictProblems(saved, zip).isEmpty());
    }
}
