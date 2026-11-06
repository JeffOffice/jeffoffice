#include "tst_write.h"


// Typing, deleting, selecting, the clipboard, and character and paragraph formatting work in a header as in the text; a tab goes to the style's center and right stops.
void WriteTests::editingInAHeaderWorksAsInTheText()
{
    auto doc = open(makeDocxWithParts(paragraphs(3, 20), QByteArray(), {}));
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 900);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    const QString bodyBefore = doc->story()->toPlainText();
    QVERIFY(view.enterHeaderFooter(0, false));
    QTextDocument *story = doc->headerFooterStory(0, false, 0);
    QCOMPARE(view.cursor().document(), story);
    QTest::keyClicks(&view, QStringLiteral("Left"));
    QTest::keyClick(&view, Qt::Key_Tab);
    QTest::keyClicks(&view, QStringLiteral("Middle"));
    QTest::keyClick(&view, Qt::Key_Tab);
    QTest::keyClicks(&view, QStringLiteral("Right"));
    QCOMPARE(story->toPlainText(), QStringLiteral("Left\tMiddle\tRight"));
    // The tab stops of the Header style put the three at the left, the middle, and the right of the line.
    {
        const auto page = doc->pager().materialize(0);
        QVERIFY(page);
        const TextItem *box = boxWithText(*page, QStringLiteral("Middle"));
        QVERIFY(box);
        LayoutCache cache;
        const LayoutCache::FrameLayout fl = cache.textFrame(*page->doc, *box, 1, RenderOptions());
        QVERIFY(fl.layout);
        int frame = 0;
        QRectF left, middle, right;
        QVERIFY(fl.layout->caretRect(story->toPlainText().indexOf(QLatin1Char('M')), &frame, &middle));
        QVERIFY(fl.layout->caretRect(story->toPlainText().indexOf(QLatin1Char('R')), &frame, &right));
        QVERIFY(fl.layout->caretRect(0, &frame, &left));
        const double text = box->rect.width();
        QVERIFY2(std::abs(middle.x() - text / 2) < text * 0.12, qPrintable(QStringLiteral("middle at %1 of %2").arg(middle.x()).arg(text)));   // (the word starts left of the middle: it is centered on it)
        QVERIFY2(right.x() > text * 0.8, qPrintable(QStringLiteral("right at %1 of %2").arg(right.x()).arg(text)));
    }
    // Backspace, Delete, Home and End, and a selection by the keys.
    QTest::keyClick(&view, Qt::Key_Backspace);
    QCOMPARE(story->toPlainText(), QStringLiteral("Left\tMiddle\tRigh"));
    QTest::keyClick(&view, Qt::Key_Home);
    QCOMPARE(view.cursor().position(), 0);
    QTest::keyClick(&view, Qt::Key_Delete);
    QCOMPARE(story->toPlainText(), QStringLiteral("eft\tMiddle\tRigh"));
    QTest::keyClick(&view, Qt::Key_Right, Qt::ShiftModifier, 0);
    QTest::keyClick(&view, Qt::Key_Right, Qt::ShiftModifier, 0);
    QCOMPARE(view.cursor().selectedText(), QStringLiteral("ef"));
    // Bold on the selection, copy, and paste at the end.
    toggleBold(*doc, view.cursor());
    QVERIFY(allSelected(view.cursor(), [](const QTextCharFormat &f) { return f.fontWeight() >= QFont::DemiBold; }));
    view.copy();
    QTest::keyClick(&view, Qt::Key_End);
    view.paste();
    QCOMPARE(story->toPlainText(), QStringLiteral("eft\tMiddle\tRighef"));
    // Paragraph formatting (a second paragraph, centered).
    QTest::keyClick(&view, Qt::Key_Return);
    QTest::keyClicks(&view, QStringLiteral("Second"));
    setAlignment(*doc, view.cursor(), Qt::AlignHCenter);
    QCOMPARE(story->blockCount(), 2);
    QVERIFY(story->lastBlock().blockFormat().alignment() & Qt::AlignHCenter);
    // The text of the document was not touched, and what was done is written.
    QCOMPARE(doc->story()->toPlainText(), bodyBefore);
    const QByteArray out = partOf(doc->toDocx(), QStringLiteral("/word/header1.xml"));
    QVERIFY2(out.contains("Second") && out.contains("<w:jc w:val=\"center\"/>") && out.contains("<w:b/>"), out.constData());
    // Select all and the clipboard: cut takes everything.
    view.selectAll();
    view.cut();
    QCOMPARE(story->toPlainText(), QString());
    view.undo();
    QVERIFY(story->toPlainText().startsWith(QLatin1String("eft\t")));
    QVERIFY(view.inHeaderFooter());
}


// A taller header pushes the text down on every page that uses it, and the pages are laid out again: more pages for the same text.
void WriteTests::aTallerHeaderPushesTheTextDownOnEveryPageThatUsesIt()
{
    const QByteArray sect = sectionNaming({{"default", "rIdheader1"}, {"first", "rIdheader2"}}, "<w:titlePg/>");
    auto doc = open(makeDocxWithParts(paragraphs(60, 40), sect,
                                      {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>one line</w:t></w:r>"})}, {QStringLiteral("/word/header2.xml"), headerPart("hdr", {"<w:r><w:t>first</w:t></w:r>"})}}));
    QVERIFY(doc);
    auto bodyTop = [&](int page) {
        const auto p = doc->pager().materialize(page);
        return p ? p->doc->item(p->boxes.first())->rect.top() : -1.0;
    };
    const int pagesBefore = doc->pager().pageCount();
    const double topBefore = bodyTop(1), topLater = bodyTop(2), topFirst = bodyTop(0);
    QTextDocument *header = doc->editableHeaderFooter(0, false, 0);
    QVERIFY(header);
    QTextCursor c(header);
    c.movePosition(QTextCursor::End);
    for (int i = 0; i < 12; ++i) {
        insertParagraph(*doc, c);
        typeText(*doc, c, QStringLiteral("more header text"));
    }
    // (The lines of the header are taller than the margin leaves room for: the text starts below them.)
    const auto page = doc->pager().materialize(1);
    QVERIFY(page);
    const TextItem *box = boxWithText(*page, QStringLiteral("more header text"));
    QVERIFY(box);
    QVERIFY2(bodyTop(1) >= box->rect.bottom() - 0.5, qPrintable(QStringLiteral("text at %1, header ends at %2").arg(bodyTop(1)).arg(box->rect.bottom())));
    // The text's first line moved down by what the header's growth took of the margin: from where it was to the foot of the header.
    const double growth = box->rect.bottom() - topBefore;
    QVERIFY2(growth > 60, qPrintable(QString::number(growth)));
    QVERIFY2(std::abs((bodyTop(1) - topBefore) - growth) < 0.5, qPrintable(QStringLiteral("%1 before, %2 after, the header grew to %3").arg(topBefore).arg(bodyTop(1)).arg(box->rect.bottom())));
    QVERIFY2(std::abs((bodyTop(2) - topLater) - growth) < 0.5, qPrintable(QStringLiteral("%1 before, %2 after").arg(topLater).arg(bodyTop(2))));
    QCOMPARE(bodyTop(0), topFirst);   // (the first page has a header of its own)
    // Shorter again: back where it was.
    doc->undo();
    doc->undo();
    while (doc->canUndo()) doc->undo();
    QCOMPARE(bodyTop(1), topBefore);
    QCOMPARE(doc->pager().pageCount(), pagesBefore);
}


// A header's paragraph that has no style, in a file that defines no Header style, is set as the built-in header paragraph is, which has no size of its own: the line of an
// empty one is the document's default size (10 points) and not the Normal style's. A file that defines the Header style, and a footer where it defines no Footer, differ only
// by that style. (Where the file defines the style, the paragraph is of Normal as it was.)
void WriteTests::anEmptyHeaderParagraphWithNoStyleHasTheDefaultSizeWhereTheFileHasNoHeaderStyle()
{
    const QByteArray normal = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:rPr><w:sz w:val=\"24\"/></w:rPr></w:style>";
    const QByteArray header = "<w:style w:type=\"paragraph\" w:styleId=\"Header\"><w:name w:val=\"header\"/><w:basedOn w:val=\"Normal\"/></w:style>";
    auto bodyTop = [&](const QByteArray &styles) {
        const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                                "<w:pgMar w:top=\"720\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(paragraphs(2, 10), sect, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"", "", ""})}}, QByteArray(), styles));
        if (!doc) return -1.0;
        const auto p = doc->pager().materialize(0);
        return p ? p->doc->item(p->boxes.first())->rect.top() : -1.0;
    };
    const double without = bodyTop(normal) - 36, with = bodyTop(normal + header) - 36;
    QVERIFY(without > 0 && with > 0);
    QVERIFY2(std::abs(with / without - 1.2) < 0.03, qPrintable(QStringLiteral("three empty lines are %1 high with no Header style and %2 with it").arg(without).arg(with)));
}


// The mark size given to an unstyled header paragraph is the layout's alone: a file that is opened and saved, with its header opened for editing or not, has no size written
// into the paragraphs that had none.
void WriteTests::theHeaderMarkSizeIsNotWrittenBack()
{
    const QByteArray normal = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:rPr><w:sz w:val=\"24\"/></w:rPr></w:style>";
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"720\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    for (const bool edit : {false, true}) {
        auto doc = open(makeDocxWithParts(paragraphs(2, 10), sect, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"", "<w:r><w:t>text</w:t></w:r>", ""})}}, QByteArray(), normal));
        QVERIFY(doc);
        if (edit) {
            QTextDocument *header = doc->editableHeaderFooter(0, false, 0);
            QVERIFY(header);
            QTextCursor c(header);
            c.movePosition(QTextCursor::End);
            c.insertText(QStringLiteral("!"));
            c.deletePreviousChar();   // (the header is in its edited form and has changed, with the text it had)
        }
        const QByteArray out = partOf(doc->toDocx(), QStringLiteral("/word/header1.xml"));
        QVERIFY(!out.isEmpty());
        QVERIFY2(!out.contains("w:sz "), qPrintable(QString(edit ? "edited: " : "unedited: ") + QString::fromUtf8(out)));
    }
}


// Only a link's field has the mark that keeps a line from breaking after a slash in its result (tp::LinkResult): the atom of a HYPERLINK field has it, and a DATE field, whose
// result has slashes too, has not.
void WriteTests::onlyALinksFieldResultCarriesTheNoBreakMark()
{
    auto field = [](const QByteArray &instruction, const QByteArray &result) {
        return "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> " + instruction + " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
               "<w:r><w:t>" + result + "</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
    };
    const QByteArray body = "<w:p>" + field("HYPERLINK \"http://www.example.gov/a/b.htm\"", "www.example.gov/a/b.htm") + "<w:r><w:t xml:space=\"preserve\"> and </w:t></w:r>" +
                            field("DATE \\@ \"M/d/yyyy\"", "10/31/2026") + "</w:p>";
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("fields.docx"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(makeDocxWithParts(body, "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>", {}));
    file.close();
    QByteArray bytes;
    {
        QFile in(path);
        QVERIFY(in.open(QIODevice::ReadOnly));
        bytes = in.readAll();
    }
    auto doc = open(bytes);
    QVERIFY(doc);
    int links = 0, others = 0;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat f = it.fragment().charFormat();
            if (f.stringProperty(tp::Field).isEmpty()) continue;
            if (f.boolProperty(tp::LinkResult)) ++links;
            else ++others;
        }
    QVERIFY2(links >= 1, "the link's field has the mark");
    QVERIFY2(others >= 1, "the date's field is there and has no mark");
}


// The view follows an undo or a redo into the header or footer the step was in, and out of it for a step in the text; a burst of typing in a header is one step.
void WriteTests::theViewFollowsTheHistoryIntoAndOutOfAHeader()
{
    auto doc = open(makeDocxWithParts(paragraphs(3, 20), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>H</w:t></w:r>"})}}));
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 900);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    const QString body = doc->story()->toPlainText();
    // Typing in the text, then in the header (three characters: one step), then the text again.
    view.cursor().setPosition(1);
    view.insertText(QStringLiteral("x"));
    QVERIFY(view.enterHeaderFooter(0, false));
    view.cursor().movePosition(QTextCursor::End);
    for (const QChar ch : QStringLiteral("abc")) view.insertText(QString(ch));
    QCOMPARE(doc->headerFooterStory(0, false, 0)->toPlainText(), QStringLiteral("Habc"));
    view.leaveHeaderFooter();
    view.insertText(QStringLiteral("y"));
    QVERIFY(doc->story()->toPlainText() != body);
    const QString typedBody = doc->story()->toPlainText();
    view.undo();   // the text's "y"
    QVERIFY(!view.inHeaderFooter());
    QVERIFY(doc->story()->toPlainText() != typedBody);
    view.undo();   // the header's burst, as one step: the header is shown
    QVERIFY(view.inHeaderFooter());
    QCOMPARE(doc->headerFooterStory(0, false, 0)->toPlainText(), QStringLiteral("H"));
    QCOMPARE(view.cursor().document(), doc->headerFooterStory(0, false, 0));
    view.undo();   // the text's "x": the view is back in the text
    QVERIFY(!view.inHeaderFooter());
    QCOMPARE(doc->story()->toPlainText(), body);
    view.redo();
    QCOMPARE(doc->story()->toPlainText().size(), body.size() + 1);
    view.redo();
    QVERIFY(view.inHeaderFooter());
    QCOMPARE(doc->headerFooterStory(0, false, 0)->toPlainText(), QStringLiteral("Habc"));
    view.redo();
    QVERIFY(!view.inHeaderFooter());
    QCOMPARE(doc->story()->toPlainText(), typedBody);
}


// Misspelled words show a red wave while a header is edited, in the header; the dimmed text of the page has none of its own marks.
void WriteTests::spellingMarksShowInAHeaderWhileItIsEdited()
{
    auto doc = open(makeDocxWithParts(paragraphs(3, 20), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>plain words</w:t></w:r>"})}}));
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 700);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    // The red pixels in the header's area (the checking of the words that are drawn is done in idle time: it is waited for).
    auto headerArea = [&] {
        const QRect page = view.pageRect(0);
        return QRect(page.left(), page.top(), page.width(), int(72 * 96 / 72 * view.zoom()));
    };
    auto redNow = [&] {
        int n = 0;
        redBounds(view.grab().toImage(), headerArea(), &n);
        return n;
    };
    QTRY_VERIFY((view.grab(), !view.editedSpelling()->pending()));
    QCOMPARE(redNow(), 0);
    QVERIFY(view.enterHeaderFooter(0, false));
    QTRY_VERIFY((view.grab(), !view.editedSpelling()->pending()));
    QCOMPARE(redNow(), 0);
    view.cursor().movePosition(QTextCursor::End);
    for (const QChar ch : QStringLiteral(" brwon ")) view.insertText(QString(ch));
    QVERIFY(view.editedSpelling());
    QCOMPARE(view.editedSpelling()->story(), doc->headerFooterStory(0, false, 0));
    QCOMPARE(view.spelling()->story(), doc->story());   // (the text's check is the text's still)
    QVERIFY(!view.editedSpelling()->misspelledIn(0, 40).isEmpty());
    int count = 0;
    QTRY_VERIFY((redBounds(view.grab().toImage(), headerArea(), &count), count > 8));
    // The right-click menu offers the header's word, and a suggestion takes the place of that word, not of anything in the text.
    {
        const QString shown = doc->headerFooterStory(0, false, 0)->toPlainText();
        view.cursor().setPosition(shown.indexOf(QLatin1String("brwon")) + 2);
        QMenu menu;
        QVERIFY(addSpellingSuggestions(menu, &view));
        QAction *first = menu.actions().value(0);
        QVERIFY(first && first->isEnabled());
        const QString body = doc->story()->toPlainText();
        first->trigger();
        QVERIFY2(!doc->headerFooterStory(0, false, 0)->toPlainText().contains(QLatin1String("brwon")), qPrintable(doc->headerFooterStory(0, false, 0)->toPlainText()));
        QCOMPARE(doc->story()->toPlainText(), body);
        QVERIFY(view.inHeaderFooter());
    }
    // Left, the header is as any: no marks.
    view.leaveHeaderFooter();
    QTRY_VERIFY((view.grab(), !view.editedSpelling()->pending()));
    QCOMPARE(redNow(), 0);
}


// Automatic recovery writes the headers and footers with the rest: a copy of work that is not saved has the edited header.
void WriteTests::anEditedHeaderIsInTheRecoveryCopy()
{
    QTemporaryDir dir;
    recovery::setRoot(dir.path());
    recovery::setCopySuffix(QStringLiteral(".docx"));
    WriteWindow w;
    w.show();
    QVERIFY(w.view()->enterHeaderFooterAtCaret(false));
    typeText(*w.document(), w.view()->cursor(), QStringLiteral("Only in the header"));
    QVERIFY(w.isModified());
    w.autoRecover();
    QVERIFY(!w.recoveryCopy().isEmpty());
    QFile f(w.recoveryCopy());
    QVERIFY(f.open(QIODevice::ReadOnly));
    auto back = WriteDoc::fromDocx(f.readAll());
    QVERIFY(back);
    QCOMPARE(back->story()->toPlainText(), QString());
    QCOMPARE(headerOfPage(*back, 0), QStringLiteral("Only in the header"));
}


// Help has a topic for headers and footers, which the Insert and Header & Footer tabs open, and which says nothing it should not.
void WriteTests::helpHasATopicForHeadersAndFooters()
{
    const jp::help::Topic topic = jp::help::topic(QStringLiteral("headers-footers"));
    QVERIFY(!topic.markdown.isEmpty());
    QVERIFY(topic.markdown.startsWith(QLatin1String("# Headers and footers")));
    QCOMPARE(jp::help::contextTopic(QStringLiteral("tab:Header & Footer")), QStringLiteral("headers-footers"));
    for (const char *banned : {"Microsoft", "Word", "Office 2021", "Windows", "virtual machine", " VM ", "Excel"})
        QVERIFY2(!topic.markdown.contains(QLatin1String(banned)), banned);
    for (const char *needed : {"Esc", "Insert > Header", "Link to Previous", "Different First Page", "Different Odd & Even Pages", "Page Number", "Undo"}) QVERIFY2(topic.markdown.contains(QLatin1String(needed)), needed);
    // The welcome topic says headers and footers can be edited, and links to the topic.
    const QString start = jp::help::topic(QStringLiteral("start")).markdown;
    QVERIFY(start.contains(QLatin1String("(headers-footers)")));
    QVERIFY(!start.contains(QLatin1String("Headers, footers, footnotes, and endnotes can't be edited")));
}


void WriteTests::helpHasATopicForFootnotesAndEndnotes()
{
    const jp::help::Topic topic = jp::help::topic(QStringLiteral("footnotes-endnotes"));
    QVERIFY(!topic.markdown.isEmpty());
    QVERIFY(topic.markdown.startsWith(QLatin1String("# Footnotes and endnotes")));
    QCOMPARE(jp::help::contextTopic(QStringLiteral("tab:References")), QStringLiteral("footnotes-endnotes"));
    for (const char *banned : {"Microsoft", "Word", "Office 2021", "Windows", "virtual machine", " VM ", "Excel"})
        QVERIFY2(!topic.markdown.contains(QLatin1String(banned)), banned);
    for (const char *needed : {"Ctrl+Alt+F", "Ctrl+Alt+D", "Insert Footnote", "Insert Endnote", "Esc", "Undo", "number", "reference"}) QVERIFY2(topic.markdown.contains(QLatin1String(needed)), needed);
    // The welcome topic says notes can be edited, and links to the topic.
    const QString start = jp::help::topic(QStringLiteral("start")).markdown;
    QVERIFY(start.contains(QLatin1String("(footnotes-endnotes)")));
    QVERIFY(!start.contains(QLatin1String("Footnotes and endnotes can't be edited")));
}


// The window with a header being edited, as it looks (a picture of it is saved where JW_HEADER_SHOT says, for the documentation; without it, the picture is only made).
void WriteTests::aHeaderBeingEditedIsShownInTheWindow()
{
    const QByteArray tabs = "<w:pPr><w:pStyle w:val=\"Header\"/><w:tabs><w:tab w:val=\"center\" w:pos=\"4680\"/><w:tab w:val=\"right\" w:pos=\"9360\"/></w:tabs></w:pPr>";
    const QByteArray field = "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
                             "<w:r><w:t>1</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
    const QByteArray header = tabs + "<w:r><w:t>Quarterly Report</w:t></w:r><w:r><w:tab/></w:r><w:r><w:t>Confidential</w:t></w:r><w:r><w:tab/></w:r><w:r><w:t xml:space=\"preserve\">Page </w:t></w:r>" + field;
    const QByteArray footer = "<w:pPr><w:pStyle w:val=\"Footer\"/><w:jc w:val=\"center\"/></w:pPr><w:r><w:t>Prepared by the finance team</w:t></w:r>";
    QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Results for the third quarter</w:t></w:r></w:p>";
    body += paragraphs(6, 45);
    WriteWindow w;
    w.resize(1440, 1270);
    w.show();
    w.setDocument(open(makeDocxWithParts(body, sectionNaming({{"default", "rIdheader1"}, {"f-default", "rIdfooter1"}}),
                                         {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {header})}, {QStringLiteral("/word/footer1.xml"), headerPart("ftr", {footer})}})),
                  QString());
    w.view()->setZoom(1.0);
    w.view()->paginateAll();
    QVERIFY(w.view()->enterHeaderFooter(0, false));
    // The caret after "Confidential", and the words before it selected: the picture shows the caret's place and a selection.
    QTextCursor &c = w.view()->cursor();
    c.setPosition(0);
    c.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
    c.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
    w.view()->setCaretWithoutFocus(true);
    QTRY_VERIFY(w.act("hf.close")->isEnabled());
    QCoreApplication::processEvents();
    const QImage shot = w.grab().toImage();
    QVERIFY(!shot.isNull());
    QVERIFY(w.view()->inHeaderFooter());
    if (qEnvironmentVariableIsSet("JW_HEADER_SHOT")) QVERIFY(shot.save(qEnvironmentVariable("JW_HEADER_SHOT")));
}


// A header that is edited shows the result its file kept for a field that shows a heading's text (so that a place in the page's copy is a place in the story); the pages
// that are printed show the page's own.
void WriteTests::aHeaderBeingEditedKeepsTheResultOfAFieldThatShowsAHeading()
{
    auto styleRef = [](const QByteArray &result) {
        return "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> STYLEREF \"heading 1\" </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
               "<w:r><w:t>" + result + "</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
    };
    const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Chapter One</w:t></w:r></w:p>" + paragraphs(2, 20);
    auto doc = open(makeDocxWithParts(body, sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t xml:space=\"preserve\">Title: </w:t></w:r>" + styleRef("old result")})}}));
    QVERIFY(doc);
    auto shown = [&] { return headerOfPage(*doc, 0); };
    QCOMPARE(shown(), QStringLiteral("Title: Chapter One"));
    QTextDocument *story = doc->editableHeaderFooter(0, false, 0);
    QVERIFY(story);
    QCOMPARE(shown(), QStringLiteral("Title: Chapter One"));   // (made ready to edit: the page is as it was)
    doc->pager().setEditedStory(story);
    QCOMPARE(int(shown().count(QChar::ObjectReplacementCharacter)), 1);   // (the field is a character of the story, in its place)
    QVERIFY(shown().startsWith(QLatin1String("Title: ")));
    doc->pager().setEditedStory(nullptr);
    QCOMPARE(shown(), QStringLiteral("Title: Chapter One"));
}


// A page says which text boxes show its header and footer, the story each shows, where its text area is, and whether it begins its section.
void WriteTests::aPageSaysWhereItsHeaderAndFooterAreAndWhichStoryTheyShow()
{
    const QByteArray refs = sectionNaming({{"default", "rIdheader1"}, {"f-default", "rIdfooter1"}});
    const QByteArray body = paragraphs(40, 40) + "<w:p><w:pPr>" + refs + "</w:pPr><w:r><w:t>end of one</w:t></w:r></w:p>" + paragraphs(2, 20);
    auto doc = open(makeDocxWithParts(body, refs, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>HEAD</w:t></w:r>"})}, {QStringLiteral("/word/footer1.xml"), headerPart("ftr", {"<w:r><w:t>FOOT</w:t></w:r>"})}}));
    QVERIFY(doc);
    QCOMPARE(doc->sectionCount(), 2);
    QTextDocument *header = doc->headerFooterStory(0, false, 0), *footer = doc->headerFooterStory(0, true, 0);
    QVERIFY(header && footer);
    int secondFirst = -1;
    for (int k = 0; k < doc->pager().pageCount() && secondFirst < 0; ++k)
        if (doc->pager().page(k).start.section == 1) secondFirst = k;
    QVERIFY(secondFirst > 1);
    for (int k : {0, 1, secondFirst}) {
        const auto page = doc->pager().materialize(k);
        QVERIFY(page);
        QCOMPARE(int(page->furniture.size()), 2);
        QCOMPARE(page->furniture[0].footer, false);
        QCOMPARE(page->furniture[0].source, header);
        QCOMPARE(page->furniture[1].footer, true);
        QCOMPARE(page->furniture[1].source, footer);
        QCOMPARE(page->furniture[0].from, 0);
        QCOMPARE(page->firstOfSection, k == 0 || k == secondFirst);
        const Item *h = page->doc->item(page->furniture[0].itemId), *f = page->doc->item(page->furniture[1].itemId);
        QVERIFY(h && f);
        // The text area is between them.
        QVERIFY(page->bodyArea.top() >= h->rect.bottom() - 0.01);
        QVERIFY(page->bodyArea.bottom() <= f->rect.top() + 0.01);
        QVERIFY(page->bodyArea.top() >= 72 - 0.01);
    }
    QCOMPARE(doc->flow().section(0).slotFor(true, 1), 0);
    // (A page with no header or footer has no boxes.)
    auto none = open(makeDocxWithParts(paragraphs(3, 20), QByteArray(), {}));
    QVERIFY(none);
    QCOMPARE(int(none->pager().materialize(0)->furniture.size()), 0);
}


// A table in a header keeps its place; the text in its cells is edited, and the caret is in it on the page.
void WriteTests::aTableInAHeaderKeepsItsPlaceAndItsCellTextIsEdited()
{
    const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"5000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"5000\"/></w:tblGrid><w:tr><w:tc><w:tcPr><w:tcW w:w=\"5000\" w:type=\"dxa\"/></w:tcPr>"
                             "<w:p><w:r><w:t>cell text</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
    const QByteArray header = "<w:hdr " + QByteArray(kNs) + "><w:p><w:r><w:t>Before</w:t></w:r></w:p>" + table + "<w:p><w:r><w:t>After</w:t></w:r></w:p></w:hdr>";
    auto doc = open(makeDocxWithParts(paragraphs(3, 20), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), header}}));
    QVERIFY(doc);
    QTextDocument *story = doc->editableHeaderFooter(0, false, 0);
    QVERIFY(story);
    QCOMPARE(story->blockCount(), 3);
    const int before = story->toPlainText().indexOf(QLatin1String("cell text"));
    QVERIFY(before > 0);
    {
        const auto page = doc->pager().materialize(0);
        QVERIFY(page);
        QCOMPARE(int(page->furnitureTables.size()), 1);
        QCOMPARE(page->furnitureTableSources[0], story);
        QVERIFY(page->furniture.size() >= 2);   // (the text before the table and the text after it)
        QCOMPARE(page->furniture[0].from, 0);
        QVERIFY(page->furniture[1].from > before);
    }
    // The caret in the cell's text is on the page; the text typed there is the cell's.
    PageView view;
    view.resize(1000, 900);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    QVERIFY(view.enterHeaderFooter(0, false));
    view.cursor().setPosition(before + 4);
    QRect r;
    QVERIFY(view.caretRect(&r));
    view.cursor().setPosition(story->characterCount() - 1);
    QVERIFY(view.caretRect(&r));   // (the paragraph after the table)
    view.cursor().setPosition(before + 9);   // (the end of "cell text")
    for (const QChar ch : QStringLiteral(" more")) view.insertText(QString(ch));
    QVERIFY(view.cursor().document() == story);
    QCOMPARE(story->toPlainText().count(QLatin1String("cell text more")), 1);
    // Written: the table is whole, in its place, with the cell's new text.
    const QByteArray out = partOf(doc->toDocx(), QStringLiteral("/word/header1.xml"));
    QVERIFY2(out.contains("<w:tbl>") && out.contains("</w:tbl>") && out.contains("cell text more") && out.indexOf("Before") < out.indexOf("<w:tbl>") && out.indexOf("</w:tbl>") < out.indexOf("After"), out.constData());
    QCOMPARE(int(out.count("<w:tc>")), 1);
    auto again = open(doc->toDocx());
    QVERIFY(again);
    QTextDocument *back = again->editableHeaderFooter(0, false, 0);
    QVERIFY(back);
    QCOMPARE(back->toPlainText(), story->toPlainText());
}


// ---- review fixes

// A multi-level list and a page header, for the tests of the history.
std::unique_ptr<WriteDoc> WriteTests::listAndHeaderDoc()
{
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:multiLevelType w:val=\"multilevel\"/><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/>"
                                 "<w:lvlJc w:val=\"left\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl><w:lvl w:ilvl=\"1\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.%2\"/>"
                                 "<w:lvlJc w:val=\"left\"/><w:pPr><w:ind w:left=\"1440\" w:hanging=\"360\"/></w:pPr></w:lvl></w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
    QByteArray body = itemOf(1, 0, "top");
    for (int i = 1; i <= 5; ++i) body += itemOf(1, 1, ("sub " + QByteArray::number(i)).constData());
    body += itemOf(1, 0, "top2") + "<w:p><w:r><w:t>plain text after the list</w:t></w:r></w:p>";
    return open(makeDocxWithParts(body, sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>HEAD</w:t></w:r>"})}}, QByteArray(), kStyles, numbering));
}


// An edit that renumbers a list is one step of the history, in its place: typing in the text, then in the header, then pressing Enter in the middle of a sub-item, is three steps
// that undo in the order they were made.
void WriteTests::anEditThatRenumbersAListIsOneStepOfTheHistory()
{
    auto doc = listAndHeaderDoc();
    QVERIFY(doc);
    QTextDocument *body = doc->story(), *header = doc->editableHeaderFooter(0, false, 0);
    QVERIFY(header);
    const QString original = body->toPlainText();
    QTextCursor bc(body);
    bc.movePosition(QTextCursor::End);
    insertText(*doc, bc, QStringLiteral("X"));
    typeAtEnd(*doc, header, QStringLiteral("h"));
    QTextCursor ec(body);
    ec.setPosition(body->toPlainText().indexOf(QLatin1String("sub 2")) + 3);
    const QStringList markers = markersOf(*doc);
    insertParagraph(*doc, ec);
    QVERIFY(markersOf(*doc) != markers);   // (the items were numbered again by the edit)
    QCOMPARE(doc->undo(), body);
    QVERIFY2(body->toPlainText().endsWith(QLatin1Char('X')), qPrintable(body->toPlainText().right(30)));
    QCOMPARE(markersOf(*doc), markers);
    QTextDocument *second = doc->undo();
    QVERIFY2(second == header, second == body ? "the second undo took the text's X" : "the second undo was in no story");
    QCOMPARE(header->toPlainText(), QStringLiteral("HEAD"));
    QCOMPARE(doc->undo(), body);
    QCOMPARE(body->toPlainText(), original);
    QVERIFY(!doc->canUndo());
    QVERIFY(!doc->isModified());
    // And again, in the order they were made.
    QCOMPARE(doc->redo(), body);
    QCOMPARE(doc->redo(), header);
    QCOMPARE(doc->redo(), body);
    QVERIFY(!doc->canRedo());
}


// Random edits of a list and a header, among them those that number the list again and the steps back and forward through all of it: each step back lands on a state that was
// there before, in the reverse order, back to the file as it was; and every step forward goes back to the last.
void WriteTests::randomEditsThatRenumberKeepTheOrderOfTheHistory()
{
    QStringList failures;
    for (unsigned seed = 1; seed <= 12; ++seed) {
        auto doc = listAndHeaderDoc();
        QVERIFY(doc);
        QTextDocument *body = doc->story(), *header = doc->editableHeaderFooter(0, false, 0);
        std::mt19937 rng(seed * 7919u);
        auto pick = [&](int n) { return int(rng() % unsigned(std::max(1, n))); };
        // (What the text says, with the number each item of the list shows, and the header's text.)
        auto state = [&] {
            QString out;
            for (QTextBlock b = body->begin(); b.isValid(); b = b.next()) out += b.blockFormat().stringProperty(tp::BulletChar) + QLatin1Char('#') + QString::number(b.blockFormat().intProperty(prop::NumLevel)) + QLatin1Char(':') + b.text() + QLatin1Char('\n');
            return out + QLatin1Char('\u0001') + header->toPlainText();
        };
        QStringList snaps{state()};
        for (int step = 0; step < 60; ++step) {
            QTextCursor c(body), h(header);
            c.setPosition(pick(body->characterCount() - 1));
            h.setPosition(pick(header->characterCount() - 1));
            switch (pick(9)) {
            case 0: case 1: insertText(*doc, c, QString(QChar('a' + pick(26)))); break;
            case 2: case 3: insertParagraph(*doc, c); break;   // (in the list: the items are numbered again)
            case 4: deleteBack(*doc, c, false); break;
            case 5: toggleList(*doc, c, pick(2)); break;
            case 6: insertText(*doc, h, QString(QChar('A' + pick(26)))); break;
            case 7: insertParagraph(*doc, h); break;
            default: if (pick(2)) doc->undo(); else doc->redo(); break;
            }
            if (state() != snaps.last()) snaps << state();
        }
        // (The steps that were left to go forward again are gone through, so that the last state is the one all of them come to.)
        int guard = 0;
        while (doc->canRedo() && guard++ < 500) {
            doc->redo();
            if (state() != snaps.last()) snaps << state();
        }
        QStringList seen{state()};
        guard = 0;
        while (doc->canUndo() && guard++ < 500) {
            if (!doc->undo()) break;
            seen << state();
        }
        int at = int(snaps.size()) - 1, bad = 0;
        for (int i = 1; i < seen.size(); ++i) {
            if (seen[i] == seen[i - 1]) continue;   // (a step that changes nothing that shows: a list made and unmade, a property)
            int found = -1;
            for (int j = at - 1; j >= 0 && found < 0; --j)
                if (snaps[j] == seen[i]) found = j;
            if (found < 0) ++bad;
            else at = found;
        }
        if (bad > 0 || state() != snaps.first() || doc->canUndo())
            failures << QStringLiteral("seed %1: %2 steps back landed on no earlier state; back to the first state: %3; can still undo: %4").arg(seed).arg(bad).arg(state() == snaps.first()).arg(doc->canUndo());
        guard = 0;
        while (doc->canRedo() && guard++ < 500) doc->redo();
        if (state() != snaps.last()) failures << QStringLiteral("seed %1: redo did not come back to the last state").arg(seed);
    }
    QVERIFY2(failures.isEmpty(), qPrintable(failures.join(QLatin1Char('\n'))));
}


// ---- the Find pane and the Spelling pane beside an open header

// Find Next with a header open takes the window to the text: the header is left, the match is selected, and what is typed goes into the text.
void WriteTests::findNextWithAHeaderOpenLeavesTheHeader()
{
    WriteWindow w;
    w.resize(1100, 800);
    w.show();
    w.setDocument(open(makeDocxWithParts(paragraphs(2, 10) + "<w:p><w:r><w:t>find the needle here</w:t></w:r></w:p>", sectionNaming({{"default", "rIdheader1"}}),
                                         {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>HEAD</w:t></w:r>"})}})), QString());
    w.showFindPane(false);
    WriteFindPane *pane = w.findPane();
    QVERIFY(pane);
    pane->findBox()->setText(QStringLiteral("needle"));
    QVERIFY(w.view()->enterHeaderFooter(0, false));
    QTRY_VERIFY(w.act("hf.close")->isEnabled());
    QVERIFY(pane->findNext());
    QVERIFY2(!w.view()->inHeaderFooter(), "the view stayed in the header");
    QCOMPARE(w.view()->cursor().document(), w.document()->story());
    QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("needle"));
    QTRY_VERIFY(!w.act("hf.close")->isEnabled());
    QTest::keyClicks(w.view(), QStringLiteral("Z"));
    QCOMPARE(w.document()->headerFooterStory(0, false, 0)->toPlainText(), QStringLiteral("HEAD"));
    QVERIFY(w.document()->story()->toPlainText().contains(QLatin1String("find the Z here")));
}


// A cursor in the text, given to the view while a header is open, ends the header's editing (the view never has the caret in one story and the tab of another).
void WriteTests::aCursorInTheTextGivenToTheViewLeavesTheHeader()
{
    auto doc = open(makeDocxWithParts(paragraphs(3, 20), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>HEAD</w:t></w:r>"})}}));
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 900);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    for (const bool show : {false, true}) {
        QVERIFY(view.enterHeaderFooter(0, false));
        QTextCursor c(doc->story());
        c.setPosition(5);
        if (show) view.showCursor(c);
        else view.setCursor(c);
        QVERIFY2(!view.inHeaderFooter(), show ? "showCursor" : "setCursor");
        QCOMPARE(view.cursor().document(), doc->story());
        QCOMPARE(view.cursor().position(), 5);
    }
    // A cursor in the header that is open is its own.
    QVERIFY(view.enterHeaderFooter(0, false));
    QTextCursor own(doc->headerFooterStory(0, false, 0));
    own.setPosition(2);
    view.setCursor(own);
    QVERIFY(view.inHeaderFooter());
    QCOMPARE(view.cursor().position(), 2);
}


// The Spelling pane, when it goes on to the next word with a header open, leaves the header: the word is shown in the text.
void WriteTests::theSpellingPaneWithAHeaderOpenLeavesTheHeader()
{
    const SettingGuard asType(QStringLiteral("proof/asYouType"), true, true);
    WriteWindow w;
    w.setDocument(open(makeDocxWithParts("<w:p><w:r><w:t>Zzalpha is first.</w:t></w:r></w:p><w:p><w:r><w:t>Then zzbeta follows.</w:t></w:r></w:p>", sectionNaming({{"default", "rIdheader1"}}),
                                         {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>HEAD</w:t></w:r>"})}})), QString());
    w.show();
    w.act("review.spelling")->trigger();
    auto *pane = w.findChild<SpellPane *>();
    QVERIFY(pane);
    QTRY_COMPARE(pane->word(), QStringLiteral("Zzalpha"));
    QVERIFY(w.view()->enterHeaderFooter(0, false));
    pane->findChild<QPushButton *>(QStringLiteral("spellIgnore"))->click();
    QTRY_COMPARE(pane->word(), QStringLiteral("zzbeta"));
    QVERIFY2(!w.view()->inHeaderFooter(), "the view stayed in the header");
    QCOMPARE(w.view()->cursor().document(), w.document()->story());
    QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("zzbeta"));
}


// ---- what a header takes of the text's commands

// A section break, a page break, and a note's reference are not made in a header or footer: the commands do nothing there, and what is pasted into one loses them.
void WriteTests::aHeaderTakesNoSectionBreakPageBreakOrNote()
{
    const QByteArray body = "<w:p><w:pPr>" + sectionNaming({}) + "</w:pPr><w:r><w:t>first section</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p><w:p><w:r><w:t>second</w:t></w:r></w:p>";
    auto doc = open(makeDocxWithParts(body, sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>HEAD</w:t></w:r>"})},
                                                                                          {QStringLiteral("/word/footnotes.xml"), notesPart({{2, "A note"}})}}));
    QVERIFY(doc);
    QCOMPARE(doc->sectionCount(), 2);
    QTextDocument *header = doc->editableHeaderFooter(1, false, 0);
    QVERIFY(header);
    QTextCursor c(header);
    c.movePosition(QTextCursor::End);
    // The commands do nothing.
    insertSectionBreak(*doc, c);
    insertPageBreak(*doc, c);
    QCOMPARE(header->toPlainText(), QStringLiteral("HEAD"));
    QCOMPARE(header->blockCount(), 1);
    QCOMPARE(doc->sectionCount(), 2);
    // The text's own, with a note and a section break in it, is pasted: the words come, and not those.
    QTextCursor all(doc->story());
    all.select(QTextCursor::Document);
    const Clip clip = copySelection(*doc, all);
    QVERIFY(!clip.isEmpty());
    pasteClip(*doc, c, clip);
    QVERIFY(header->toPlainText().contains(QLatin1String("first section")));
    QVERIFY(header->toPlainText().contains(QLatin1String("second")));
    pasteFragment(*doc, c, clip.fragment);
    QCOMPARE(doc->sectionCount(), 2);
    const QByteArray out = partOf(doc->toDocx(), QStringLiteral("/word/header1.xml"));
    QVERIFY2(!out.contains("sectPr"), out.constData());
    QVERIFY2(!out.contains("footnoteReference"), out.constData());
    QVERIFY2(!out.contains("w:br"), out.constData());
    QVERIFY(out.contains("first section"));
}


// The ribbon commands for those are grayed while a header is edited, and the keys for them do nothing there.
void WriteTests::theBreakCommandsAreGrayedWhileAHeaderIsEdited()
{
    WriteWindow w;
    w.resize(1100, 800);
    w.show();
    w.setDocument(open(makeDocxWithParts(paragraphs(3, 20), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>HEAD</w:t></w:r>"})}})), QString());
    QTRY_VERIFY(w.act("insert.pageBreak")->isEnabled());
    QVERIFY(w.act("layout.sectionBreak")->isEnabled());
    QVERIFY(w.view()->enterHeaderFooter(0, false));
    QTRY_VERIFY(!w.act("insert.pageBreak")->isEnabled());
    QVERIFY(!w.act("layout.sectionBreak")->isEnabled());
    const int sections = w.document()->sectionCount();
    w.view()->setFocus();
    QTest::keyClick(w.view(), Qt::Key_Return, Qt::ControlModifier);
    QCOMPARE(w.document()->headerFooterStory(0, false, 0)->toPlainText(), QStringLiteral("HEAD"));
    QCOMPARE(w.document()->sectionCount(), sections);
    w.view()->leaveHeaderFooter();
    QTRY_VERIFY(w.act("insert.pageBreak")->isEnabled());
    QVERIFY(w.act("layout.sectionBreak")->isEnabled());
}


// ---- copying

// What is copied from a header as text has its fields as they are shown (the page's number, the count of pages, the date as it is), and a copy within the program
// keeps them fields.
void WriteTests::copyingHeaderTextKeepsWhatItsFieldsShow()
{
    auto field = [](const QByteArray &code, const QByteArray &result) {
        return "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> " + code + " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
               "<w:r><w:t>" + result + "</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
    };
    const QByteArray text = "<w:r><w:t>AB</w:t></w:r>" + field("PAGE", "7") + "<w:r><w:t>CD</w:t></w:r>" + field("NUMPAGES", "9") + "<w:r><w:t>EF</w:t></w:r>" + field("DATE \\@ \"MMMM d, yyyy\"", "June 5, 2024") + "<w:r><w:t>GH</w:t></w:r>";
    auto doc = open(makeDocxWithParts(paragraphs(60, 40), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {text})}}));
    QVERIFY(doc);
    const int pages = doc->pager().pageCount();
    QVERIFY(pages >= 3);
    PageView view;
    view.resize(1000, 900);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    QVERIFY(view.enterHeaderFooter(0, false));
    view.selectAll();
    view.copy();
    QCOMPARE(QApplication::clipboard()->text(), QStringLiteral("AB1CD%1EFJune 5, 2024GH").arg(pages));
    // Pasted in the program, the fields come as fields.
    QTextDocument *story = doc->headerFooterStory(0, false, 0);
    QCOMPARE(int(story->toPlainText().count(QChar::ObjectReplacementCharacter)), 3);
    view.cursor().movePosition(QTextCursor::End);
    view.paste();
    QCOMPARE(int(story->toPlainText().count(QChar::ObjectReplacementCharacter)), 6);
    // Copied on the third page, the page's number is that page's.
    view.leaveHeaderFooter();
    QVERIFY(view.enterHeaderFooter(2, false));
    view.selectAll();
    view.copy();
    QVERIFY2(QApplication::clipboard()->text().startsWith(QLatin1String("AB3CD")), qPrintable(QApplication::clipboard()->text()));
}


// ---- entering the edit form shows what the display showed

// A paragraph whose mark and text a tracked change deleted takes no room in the header, and entering the edit form for the header leaves the page as it was (the paragraph is kept and
// written back).
void WriteTests::enteringAHeaderForEditingLeavesThePageAsItWas()
{
    const QByteArray deleted = "<w:p><w:pPr><w:rPr><w:del w:id=\"1\" w:author=\"A\" w:date=\"2020-01-01T00:00:00Z\"/></w:rPr></w:pPr><w:del w:id=\"2\" w:author=\"A\" w:date=\"2020-01-01T00:00:00Z\">"
                               "<w:r><w:delText>gone words</w:delText></w:r></w:del></w:p>";
    const QByteArray header = "<w:hdr " + QByteArray(kNs) + "><w:p><w:r><w:t>One</w:t></w:r></w:p>" + deleted + "<w:p><w:r><w:t>Three</w:t></w:r></w:p></w:hdr>";
    auto doc = open(makeDocxWithParts(paragraphs(6, 30), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), header}}));
    QVERIFY(doc);
    auto look = [&] {
        const auto page = doc->pager().materialize(0);
        QString s;
        for (const auto &item : page->doc->pages[0]->items) s += QStringLiteral("%1:%2,%3,%4,%5 ").arg(item->id).arg(item->rect.left()).arg(item->rect.top()).arg(item->rect.width()).arg(item->rect.height());
        return s + QStringLiteral("body %1 %2 | ").arg(page->bodyArea.top()).arg(page->bodyArea.bottom()) + shownHeader(*page);
    };
    const QString before = look();
    QTextDocument *story = doc->editableHeaderFooter(0, false, 0);
    QVERIFY(story);
    QCOMPARE(look(), before);
    QVERIFY(!story->toPlainText().contains(QLatin1String("gone")));
    typeAtEnd(*doc, story, QStringLiteral("!"));
    const QByteArray out = partOf(doc->toDocx(), QStringLiteral("/word/header1.xml"));
    QVERIFY2(out.contains("gone words") && out.contains("<w:del w:id=\"1\"") && out.indexOf("One") < out.indexOf("gone words") && out.indexOf("gone words") < out.indexOf("Three!"), out.constData());
}


// A header with a table of thousands of rows is bounded when it is entered as it is when it is shown: what is past the bound is not made into the story, and is kept and written back
// as it was.
void WriteTests::aHugeTableInAHeaderIsBoundedWhenItIsEntered()
{
    constexpr int rows = 3000;
    QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"6000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/><w:gridCol w:w=\"3000\"/></w:tblGrid>";
    for (int r = 0; r < rows; ++r)
        table += "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr><w:p><w:r><w:t>r" + QByteArray::number(r) + "a</w:t></w:r></w:p></w:tc><w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr>"
                 "<w:p><w:r><w:t>r" + QByteArray::number(r) + "b</w:t></w:r></w:p></w:tc></w:tr>";
    table += "</w:tbl>";
    const QByteArray header = "<w:hdr " + QByteArray(kNs) + "><w:p><w:r><w:t>Before</w:t></w:r></w:p>" + table + "<w:p><w:r><w:t>After</w:t></w:r></w:p></w:hdr>";
    auto doc = open(makeDocxWithParts(paragraphs(3, 20), sectionNaming({{"default", "rIdheader1"}}), {{QStringLiteral("/word/header1.xml"), header}}));
    QVERIFY(doc);
    const int cellsShown = doc->flow().tables.count();
    QVERIFY2(cellsShown <= 400, qPrintable(QString::number(cellsShown)));
    auto tablesOnPage = [&] { return int(doc->pager().materialize(0)->furnitureTables.size()); };
    const int tablesBefore = tablesOnPage();
    QTextDocument *story = doc->editableHeaderFooter(0, false, 0);
    QVERIFY(story);
    // (The work of entering: the cells made, and the paragraphs of the story, are what the display had.)
    QVERIFY2(doc->flow().tables.count() - cellsShown <= 400, qPrintable(QString::number(doc->flow().tables.count() - cellsShown)));
    QVERIFY2(story->blockCount() <= 410, qPrintable(QString::number(story->blockCount())));
    QCOMPARE(tablesOnPage(), tablesBefore);
    QTextCursor c(story);
    insertText(*doc, c, QStringLiteral(">"));
    const QByteArray out = partOf(doc->toDocx(), QStringLiteral("/word/header1.xml"));
    QCOMPARE(int(out.count("<w:tr>")), rows);
    QVERIFY(out.contains("&gt;Before<"));
    QVERIFY(out.indexOf("Before") < out.indexOf("<w:tbl>") && out.indexOf("</w:tbl>") < out.indexOf("After"));
    QVERIFY(out.contains("r2999b"));
}


// ---- footnotes: editing them

// The styles of the sample documents with the two a note has: its text, and the raised number of its mark and its reference.
const QByteArray &WriteTests::noteStyles()
{
    static const QByteArray s = kStyles +
        "<w:style w:type=\"paragraph\" w:styleId=\"FootnoteText\"><w:name w:val=\"footnote text\"/><w:basedOn w:val=\"Normal\"/><w:pPr><w:spacing w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr><w:rPr><w:sz w:val=\"20\"/></w:rPr></w:style>"
        "<w:style w:type=\"character\" w:styleId=\"FootnoteReference\"><w:name w:val=\"footnote reference\"/><w:rPr><w:vertAlign w:val=\"superscript\"/></w:rPr></w:style>";
    return s;
}


// A footnotes part with the separators and the notes given: each a w:id and what follows the mark in its first paragraph (the XML of runs).
QByteArray WriteTests::footnotesPart(const QList<std::pair<int, QByteArray>> &notes)
{
    QByteArray out = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                     "<w:footnote w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></w:footnote>";
    for (const auto &n : notes)
        out += "<w:footnote w:id=\"" + QByteArray::number(n.first) + "\"><w:p><w:pPr><w:pStyle w:val=\"FootnoteText\"/></w:pPr><w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteRef/></w:r>" +
               n.second + "</w:p></w:footnote>";
    return out + "</w:footnotes>";
}


QByteArray WriteTests::plainRun(const QByteArray &text) { return "<w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r>"; }


// A paragraph of words with a reference to note `id` after them.
QByteArray WriteTests::citing(const QByteArray &text, int id)
{
    return "<w:p><w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r><w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteReference w:id=\"" + QByteArray::number(id) +
           "\"/></w:r><w:r><w:t xml:space=\"preserve\"> and the words that follow it.</w:t></w:r></w:p>";
}


QByteArray WriteTests::pageSection() { return "<w:sectPr>" + QByteArray(kPage) + "</w:sectPr>"; }


QByteArray WriteTests::notesDocx(const QByteArray &body, const QByteArray &notes, const QByteArray &settings)
{
    return makeDocxWithParts(body, pageSection(), {{QStringLiteral("/word/footnotes.xml"), notes}}, settings, noteStyles());
}


// Three paragraphs, each citing a note of its own: 2, 3, and 5 (the Ids of a file need not follow one another).
QByteArray WriteTests::threeNotesBody()
{
    return citing("First paragraph", 2) + citing("Second paragraph", 3) + citing("Third paragraph", 5);
}


QList<std::pair<int, QByteArray>> WriteTests::threeNotes()
{
    return {{2, plainRun(" note two text")}, {3, plainRun(" note three text")}, {5, plainRun(" note five text")}};
}


QByteArray WriteTests::threeNotesDocx() { return notesDocx(threeNotesBody(), footnotesPart(threeNotes())); }


// The notes a page shows: each note's Id and the number its mark shows, in the order of the page's boxes.
QList<std::pair<int, int>> WriteTests::notesOnPage(WriteDoc &doc, int index)
{
    QList<std::pair<int, int>> out;
    const auto page = doc.pager().materialize(index);
    if (!page) return out;
    for (const auto &nb : page->noteBoxes) {
        int id = 0;
        if (!doc.footnoteIdOf(nb.source, &id)) continue;
        int number = 0;
        if (const Item *item = page->doc->item(nb.itemId))
            if (const Story *s = page->doc->story(static_cast<const TextItem *>(item)->storyId))
                for (QTextBlock b = s->doc->begin(); b.isValid() && !number; b = b.next())
                    for (auto it = b.begin(); !it.atEnd(); ++it) {
                        const QString field = it.fragment().charFormat().stringProperty(tp::Field);
                        if (field.startsWith(QLatin1String("raw:")) && field.size() > 4) {
                            number = field.mid(4).toInt();
                            break;
                        }
                    }
        out << std::make_pair(id, number);
    }
    return out;
}


QString WriteTests::noteMark() { return QString(QChar::ObjectReplacementCharacter); }


// A footnote of the file is edited in a story of its own, and the one history holds its steps with the text's.
void WriteTests::aFootnoteIsEditedInItsOwnStoryAndTheHistoryHoldsItsSteps()
{
    auto doc = open(threeNotesDocx());
    QVERIFY(doc);
    QTextDocument *note = doc->editableFootnote(3);
    QVERIFY(note);
    int id = 0;
    QVERIFY(doc->footnoteIdOf(note, &id));
    QCOMPARE(id, 3);
    QVERIFY(!doc->footnoteIdOf(doc->story()));
    QCOMPARE(doc->editableFootnote(3), note);   // (made once)
    QVERIFY(!doc->editableFootnote(99));
    QCOMPARE(note->toPlainText(), noteMark() + QStringLiteral(" note three text"));
    QVERIFY(!doc->isModified());
    QVERIFY(!doc->canUndo());
    typeAtEnd(*doc, note, QStringLiteral(" typed"));
    QCOMPARE(note->toPlainText().mid(1), QStringLiteral(" note three text typed"));
    QVERIFY(doc->isModified());
    QTextCursor body(doc->story());
    body.setPosition(0);
    insertText(*doc, body, QStringLiteral("B"));
    typeAtEnd(*doc, note, QStringLiteral("!"));
    // (The steps come back in the order they were made, each in the story it was made in.)
    QCOMPARE(doc->undo(), note);
    QCOMPARE(note->toPlainText().mid(1), QStringLiteral(" note three text typed"));
    QCOMPARE(doc->undo(), doc->story());
    QVERIFY(!plain(*doc).startsWith(QLatin1Char('B')));
    QCOMPARE(doc->undo(), note);
    QCOMPARE(note->toPlainText().mid(1), QStringLiteral(" note three text"));
    QVERIFY(!doc->canUndo());
    QVERIFY(!doc->isModified());
    QCOMPARE(doc->redo(), note);
    QCOMPARE(doc->redo(), doc->story());
    QCOMPARE(doc->redo(), note);
    QCOMPARE(note->toPlainText().mid(1), QStringLiteral(" note three text typed!"));
}


// The number that begins a note's text is one character that no edit takes: typing before it goes after it, and Backspace, a selection, and a cut leave it.
void WriteTests::theNumberMarkOfANoteIsOneCharacterThatNoEditTakes()
{
    auto doc = open(threeNotesDocx());
    QVERIFY(doc);
    QTextDocument *note = doc->editableFootnote(2);
    QVERIFY(note);
    QCOMPARE(noteTextStart(note), 1);
    QCOMPARE(noteTextStart(doc->story()), 0);
    const QString mark = noteMark();
    QTextCursor c(note);
    c.setPosition(1);
    deleteBack(*doc, c, false);
    deleteBack(*doc, c, true);
    QCOMPARE(note->toPlainText(), mark + QStringLiteral(" note two text"));
    QVERIFY(!doc->canUndo());
    c.setPosition(0);
    insertText(*doc, c, QStringLiteral("X"));
    QCOMPARE(note->toPlainText(), mark + QStringLiteral("X note two text"));
    c.setPosition(0);
    c.setPosition(2, QTextCursor::KeepAnchor);
    removeSelection(*doc, c);
    QCOMPARE(note->toPlainText(), mark + QStringLiteral(" note two text"));
    // Everything selected: deleted, the mark stays; a paste at the start goes after it; a cut takes the words only.
    c.select(QTextCursor::Document);
    removeSelection(*doc, c);
    QCOMPARE(note->toPlainText(), mark);
    c.setPosition(0);
    pasteText(*doc, c, QStringLiteral("pasted"));
    QCOMPARE(note->toPlainText(), mark + QStringLiteral("pasted"));
    c.select(QTextCursor::Document);
    const QTextDocumentFragment taken = selectionFragment(c);
    QCOMPARE(plainTextOf(taken), QStringLiteral("pasted"));
    removeSelection(*doc, c);
    QCOMPARE(note->toPlainText(), mark);
    // (Typing over everything selected replaces the words.)
    typeAtEnd(*doc, note, QStringLiteral("new words"));
    c.select(QTextCursor::Document);
    insertText(*doc, c, QStringLiteral("again"));
    QCOMPARE(note->toPlainText(), mark + QStringLiteral("again"));
    // What is written has the mark as the file had it, in each note.
    const QByteArray part = partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml"));
    QCOMPARE(int(part.count("<w:footnoteRef/>")), 3);
    QVERIFY(part.contains("again"));
}


// A note is edited as the text is: characters and paragraphs get their looks, Enter makes a paragraph, the clipboard works; and all of it is written, and all of it undone.
void WriteTests::aFootnoteIsEditedAsTheTextIs()
{
    auto doc = open(threeNotesDocx());
    QVERIFY(doc);
    const QByteArray was = partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml"));
    QTextDocument *note = doc->editableFootnote(3);
    QVERIFY(note);
    QTextCursor c(note);
    c.setPosition(1);
    c.setPosition(note->characterCount() - 1, QTextCursor::KeepAnchor);
    toggleBold(*doc, c);
    QVERIFY(allSelected(c, [](const QTextCharFormat &f) { return f.fontWeight() >= QFont::DemiBold; }));
    setAlignment(*doc, c, Qt::AlignRight);
    QCOMPARE(int(note->begin().blockFormat().alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignRight));
    c.movePosition(QTextCursor::End);
    insertParagraph(*doc, c);
    typeText(*doc, c, QStringLiteral("second"));
    QCOMPARE(note->blockCount(), 2);
    c.setPosition(note->lastBlock().position());
    deleteBack(*doc, c, false);
    QCOMPARE(note->blockCount(), 1);
    // The clipboard: some words copied, and pasted at the end.
    c.setPosition(2);
    c.setPosition(6, QTextCursor::KeepAnchor);
    const QString words = c.selectedText();
    const QTextDocumentFragment held = selectionFragment(c);
    c.movePosition(QTextCursor::End);
    pasteFragment(*doc, c, held);
    QVERIFY(note->toPlainText().endsWith(words));
    const QByteArray part = partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml"));
    QVERIFY2(part.contains("<w:jc w:val=\"right\"/>"), part.constData());
    QVERIFY(part.contains("<w:b/>"));
    QVERIFY(part.contains(words.toUtf8()));
    QVERIFY(part.contains("<w:footnoteRef/>"));
    // Taken back to the end, the part is the bytes it was.
    while (doc->canUndo()) doc->undo();
    QCOMPARE(note->toPlainText(), noteMark() + QStringLiteral(" note three text"));
    QCOMPARE(partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")), was);
}


// The pages say which story each of their notes shows, and where in it the box's text begins: a place in a box is a place in the note's story.
void WriteTests::thePagesSayWhichStoryEachOfTheirNotesShows()
{
    auto doc = open(threeNotesDocx());
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    QCOMPARE(int(page->noteBoxes.size()), 3);
    QVector<int> ids;
    for (const auto &nb : page->noteBoxes) {
        int id = 0;
        QVERIFY(nb.source && doc->footnoteIdOf(nb.source, &id));
        ids << id;
        QCOMPARE(nb.from, 0);
        const Item *item = page->doc->item(nb.itemId);
        QVERIFY(item && item->type() == ItemType::Text);
        // (The box shows the note's text, from the place the page says.)
        const Story *shown = page->doc->story(static_cast<const TextItem *>(item)->storyId);
        QVERIFY(shown);
        QCOMPARE(shown->doc->toPlainText(), nb.source->toPlainText().mid(nb.from));
    }
    QCOMPARE(ids, (QVector<int>{2, 3, 5}));
    // A note that goes on to the next page: the next page's box begins where the part before it ended.
    QByteArray longNote;
    for (int i = 0; i < 160; ++i) longNote += plainRun(" a long note of many words that goes over the page and goes on to the next one " + QByteArray::number(i));
    auto big = open(notesDocx(citing("A paragraph", 7) + paragraphs(4, 30), footnotesPart({{7, longNote}})));
    QVERIFY(big);
    QVERIFY(big->pager().pageCount() >= 2);
    int found = 0;
    for (int p = 0; p < big->pager().pageCount() && p < 4; ++p) {
        const auto pg = big->pager().materialize(p);
        for (const auto &nb : pg->noteBoxes) {
            const Item *item = pg->doc->item(nb.itemId);
            const Story *shown = pg->doc->story(static_cast<const TextItem *>(item)->storyId);
            QVERIFY(shown);
            if (p > 0) QVERIFY2(nb.from > 0, qPrintable(QString::number(nb.from)));
            QCOMPARE(shown->doc->toPlainText().left(60), nb.source->toPlainText().mid(nb.from, 60));
            ++found;
        }
    }
    QVERIFY(found >= 2);
}


// What a note's edit does to the pages is what a new paginator makes of the document: the notes at the foot of the page move the text, the pages after follow, and an undo gives them back.
void WriteTests::editingAFootnoteLaysThePagesOutAsANewPaginatorDoes()
{
    QByteArray body;
    QList<std::pair<int, QByteArray>> notes;
    for (int i = 0; i < 14; ++i) {
        body += citing("Paragraph " + QByteArray::number(i) + " of the text, with enough words in it to fill some lines of the page, and some more words to run on for a while", 10 + i);
        notes << std::make_pair(10 + i, plainRun(" note " + QByteArray::number(i) + " words words words words"));
    }
    auto doc = open(notesDocx(body + paragraphs(24, 40), footnotesPart(notes)));
    QVERIFY(doc);
    const int pagesBefore = doc->pager().pageCount();
    QVERIFY(pagesBefore >= 3);
    QVERIFY(layoutDiffersFromFresh(*doc).isEmpty());
    QTextDocument *note = doc->editableFootnote(12);
    QVERIFY(note);
    QString longText;
    for (int i = 0; i < 30; ++i) longText += QStringLiteral(" typed words that make the note taller %1").arg(i);
    typeAtEnd(*doc, note, longText);
    const QString differs = layoutDiffersFromFresh(*doc);
    QVERIFY2(differs.isEmpty(), qPrintable(differs));
    QVERIFY(doc->pager().pageCount() >= pagesBefore);
    // The page that cites the note shows the typed words in its note's box.
    const int cited = doc->pager().pageOfPosition(doc->footnoteReferences(12).first());
    bool shown = false;
    for (int p = cited; p <= cited + 1 && !shown; ++p) shown = boxWithText(*doc->pager().materialize(p), QStringLiteral("typed words that make the note taller 0")) != nullptr;
    QVERIFY(shown);
    // Undone step by step, the pages are a new paginator's at each, and the document is what it was.
    int steps = 0;
    while (doc->canUndo()) {
        doc->undo();
        if (++steps % 7 == 0) {
            const QString d = layoutDiffersFromFresh(*doc);
            QVERIFY2(d.isEmpty(), qPrintable(d));
        }
    }
    QVERIFY(layoutDiffersFromFresh(*doc).isEmpty());
    QCOMPARE(doc->pager().pageCount(), pagesBefore);
    QCOMPARE(note->toPlainText(), noteMark() + QStringLiteral(" note 2 words words words words"));
}


// Insert > Footnote puts a reference in the text and a new note with its number, and the caret goes into the note.
void WriteTests::insertFootnoteMakesAReferenceAndANoteAndMovesTheCaretIntoIt()
{
    auto doc = open(threeNotesDocx());
    QVERIFY(doc);
    QTextCursor c(doc->story());
    const QTextBlock second = doc->story()->findBlockByNumber(1);
    const int at = second.position() + 3;
    c.setPosition(at);
    QTextDocument *note = insertFootnote(*doc, c);
    QVERIFY(note);
    QCOMPARE(c.document(), note);
    QCOMPARE(c.position(), note->characterCount() - 1);
    QCOMPARE(note->toPlainText(), noteMark() + QStringLiteral(" "));
    int id = 0;
    QVERIFY(doc->footnoteIdOf(note, &id));
    QCOMPARE(id, 6);   // (one more than the highest the file has)
    QCOMPARE(doc->footnoteReferences(6), (QVector<int>{at}));
    QCOMPARE(doc->story()->characterAt(at), QChar(QChar::ObjectReplacementCharacter));
    QTextCursor probe(doc->story());
    probe.setPosition(at + 1);
    QCOMPARE(probe.charFormat().stringProperty(tp::Field), QStringLiteral("footnote:fn6"));
    // The note is at the foot of the page, and the notes are numbered by where their references are: 2, then the new one (before the third's reference), then 3 and 5.
    QCOMPARE(notesOnPage(*doc, 0), (QList<std::pair<int, int>>{{2, 1}, {6, 2}, {3, 3}, {5, 4}}));
    // What is typed in the note is the note's, in the one history with the reference: an undo takes the typing, then the reference and the note with it.
    typeAtEnd(*doc, note, QStringLiteral("A new note."));
    QCOMPARE(note->toPlainText(), noteMark() + QStringLiteral(" A new note."));
    QCOMPARE(doc->undo(), note);
    QCOMPARE(note->toPlainText(), noteMark() + QStringLiteral(" "));
    QCOMPARE(doc->undo(), doc->story());
    QVERIFY(doc->footnoteReferences(6).isEmpty());
    QCOMPARE(notesOnPage(*doc, 0), (QList<std::pair<int, int>>{{2, 1}, {3, 2}, {5, 3}}));
    QVERIFY(!doc->canUndo());
    QVERIFY(!doc->isModified());
    // And done again, both come back with the words.
    QCOMPARE(doc->redo(), doc->story());
    QCOMPARE(doc->redo(), note);
    QCOMPARE(notesOnPage(*doc, 0), (QList<std::pair<int, int>>{{2, 1}, {6, 2}, {3, 3}, {5, 4}}));
    QByteArray saved = doc->toDocx();
    const QByteArray part = partOf(saved, QStringLiteral("/word/footnotes.xml"));
    QVERIFY2(part.contains("<w:footnote w:id=\"6\">") && part.contains("A new note."), part.constData());
    QCOMPARE(int(documentXml(saved).count("<w:footnoteReference w:id=\"6\"/>")), 1);
}


// The notes that come after a new one are renumbered, and so are all of them when one goes: the number of a note is its place among the references.
void WriteTests::laterNotesAreRenumberedAsReferencesComeAndGo()
{
    auto doc = open(threeNotesDocx());
    QVERIFY(doc);
    QTextCursor c(doc->story());
    c.setPosition(0);
    QTextDocument *first = insertFootnote(*doc, c);
    QVERIFY(first);
    QCOMPARE(notesOnPage(*doc, 0), (QList<std::pair<int, int>>{{6, 1}, {2, 2}, {3, 3}, {5, 4}}));
    QTextCursor end(doc->story());
    end.movePosition(QTextCursor::End);
    QTextDocument *last = insertFootnote(*doc, end);
    QVERIFY(last);
    QCOMPARE(notesOnPage(*doc, 0), (QList<std::pair<int, int>>{{6, 1}, {2, 2}, {3, 3}, {5, 4}, {7, 5}}));
    // A reference taken out: the numbers close up.
    QTextCursor gone(doc->story());
    const int at = doc->footnoteReferences(2).first();
    gone.setPosition(at);
    gone.setPosition(at + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, gone);
    QCOMPARE(notesOnPage(*doc, 0), (QList<std::pair<int, int>>{{6, 1}, {3, 2}, {5, 3}, {7, 4}}));
}


// A reference taken out takes its note with it, in the text and in the file; an undo brings both back, and so does a redo take them away again.
void WriteTests::deletingAReferenceDeletesItsNoteAndUndoBringsBothBack()
{
    auto doc = open(threeNotesDocx());
    QVERIFY(doc);
    const QByteArray original = partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml"));
    QVERIFY(original.contains("note three text"));
    const QVector<int> refs = doc->footnoteReferences(3);
    QCOMPARE(int(refs.size()), 1);
    QTextCursor c(doc->story());
    c.setPosition(refs[0]);
    c.setPosition(refs[0] + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, c);
    QVERIFY(doc->footnoteReferences(3).isEmpty());
    QCOMPARE(notesOnPage(*doc, 0), (QList<std::pair<int, int>>{{2, 1}, {5, 2}}));
    QByteArray saved = doc->toDocx();
    QByteArray part = partOf(saved, QStringLiteral("/word/footnotes.xml"));
    QVERIFY2(!part.contains("w:id=\"3\"") && !part.contains("note three text"), part.constData());
    QVERIFY(part.contains("w:id=\"2\"") && part.contains("w:id=\"5\"") && part.contains("w:type=\"separator\""));
    QVERIFY(!documentXml(saved).contains("footnoteReference w:id=\"3\""));
    doc->undo();
    QCOMPARE(notesOnPage(*doc, 0), (QList<std::pair<int, int>>{{2, 1}, {3, 2}, {5, 3}}));
    QCOMPARE(partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")), original);
    doc->redo();
    QVERIFY(doc->footnoteReferences(3).isEmpty());
    QVERIFY(!partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")).contains("note three text"));
    doc->undo();
    // Backspace after a reference, and Delete before one, are the same.
    QTextCursor key(doc->story());
    key.setPosition(doc->footnoteReferences(5).first() + 1);
    deleteBack(*doc, key, false);
    QVERIFY(doc->footnoteReferences(5).isEmpty());
    QVERIFY(!partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")).contains("note five text"));
    doc->undo();
    QCOMPARE(int(doc->footnoteReferences(5).size()), 1);
    key.setPosition(doc->footnoteReferences(2).first());
    deleteForward(*doc, key, false);
    QVERIFY(doc->footnoteReferences(2).isEmpty());
    QVERIFY(!partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")).contains("note two text"));
    // The whole text selected and deleted: no notes. Undone: all three.
    doc->undo();
    key.select(QTextCursor::Document);
    removeSelection(*doc, key);
    QCOMPARE(notesOnPage(*doc, 0), (QList<std::pair<int, int>>{}));
    QVERIFY(!partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")).contains("note two text"));
    doc->undo();
    QCOMPARE(partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")), original);
}


// A note cited twice stays until its last reference goes: taking one reference out leaves the note, and the second takes it.
void WriteTests::aNoteCitedTwiceStaysUntilItsLastReferenceGoes()
{
    const QByteArray body = citing("First", 2) + citing("Second", 2) + citing("Third", 5);
    auto doc = open(notesDocx(body, footnotesPart({{2, plainRun(" shared note")}, {5, plainRun(" other note")}})));
    QVERIFY(doc);
    QCOMPARE(int(doc->footnoteReferences(2).size()), 2);
    auto remove = [&](int at) {
        QTextCursor c(doc->story());
        c.setPosition(at);
        c.setPosition(at + 1, QTextCursor::KeepAnchor);
        removeSelection(*doc, c);
    };
    remove(doc->footnoteReferences(2).first());
    QCOMPARE(int(doc->footnoteReferences(2).size()), 1);
    QVERIFY(partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")).contains("shared note"));
    remove(doc->footnoteReferences(2).first());
    QVERIFY(!partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")).contains("shared note"));
    QVERIFY(partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")).contains("other note"));
    doc->undo();
    doc->undo();
    QCOMPARE(int(doc->footnoteReferences(2).size()), 2);
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:footnoteReference w:id=\"2\"/>")), 2);
}


// Cutting a reference and pasting it moves its note (and what had been written in it); copying one and pasting it makes a note of its own with the same words.
void WriteTests::cuttingAndPastingAReferenceMovesItsNote()
{
    auto doc = open(threeNotesDocx());
    QVERIFY(doc);
    QTextDocument *three = doc->editableFootnote(3);
    QVERIFY(three);
    typeAtEnd(*doc, three, QStringLiteral(" edited"));
    QTextCursor c(doc->story());
    QVERIFY(!doc->footnoteReferences(3).isEmpty());
    const int at = doc->footnoteReferences(3).first();
    c.setPosition(at);
    c.setPosition(at + 1, QTextCursor::KeepAnchor);
    const QTextDocumentFragment held = selectionFragment(c);
    removeSelection(*doc, c);
    QVERIFY(doc->footnoteReferences(3).isEmpty());
    QCOMPARE(notesOnPage(*doc, 0), (QList<std::pair<int, int>>{{2, 1}, {5, 2}}));
    c.setPosition(0);
    pasteFragment(*doc, c, held);
    QCOMPARE(doc->footnoteReferences(3), (QVector<int>{0}));
    // (It is the first reference now: its note is the first, with the words it had.)
    QCOMPARE(notesOnPage(*doc, 0), (QList<std::pair<int, int>>{{3, 1}, {2, 2}, {5, 3}}));
    QVERIFY(three->toPlainText().endsWith(QStringLiteral("note three text edited")));
    QByteArray part = partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml"));
    QCOMPARE(int(part.count("w:id=\"3\"")), 1);
    QVERIFY(part.contains("note three text edited"));
    // A copy: a new reference with a new note that says the same.
    QTextCursor copy(doc->story());
    copy.setPosition(0);
    copy.setPosition(1, QTextCursor::KeepAnchor);
    const QTextDocumentFragment again = selectionFragment(copy);
    copy.movePosition(QTextCursor::End);
    pasteFragment(*doc, copy, again);
    QCOMPARE(int(doc->footnoteReferences(3).size()), 1);
    QCOMPARE(int(doc->footnoteReferences(6).size()), 1);
    QTextDocument *six = doc->editableFootnote(6);
    QVERIFY(six && six->toPlainText().endsWith(QStringLiteral("note three text edited")));
    QCOMPARE(notesOnPage(*doc, 0), (QList<std::pair<int, int>>{{3, 1}, {2, 2}, {5, 3}, {6, 4}}));
    part = partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml"));
    QCOMPARE(int(part.count("note three text edited")), 2);
    // (The file's references say which: the copy's is to the new note.)
    QVERIFY(documentXml(doc->toDocx()).contains("<w:footnoteReference w:id=\"6\"/>"));
    QCOMPARE(int(documentXml(doc->toDocx()).count("<w:footnoteReference w:id=\"3\"/>")), 1);
    // Undone, the copy is gone and the first note is as it was.
    doc->undo();
    QVERIFY(doc->footnoteReferences(6).isEmpty());
    QCOMPARE(int(partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")).count("note three text edited")), 1);
}


// No note is made in a header or in a note.
void WriteTests::aFootnoteIsNotInsertedInAHeaderOrInANote()
{
    auto doc = open(makeDocxWithParts(threeNotesBody(), sectionNaming({{"default", "rIdheader1"}}),
                                      {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>Head</w:t></w:r>"})}, {QStringLiteral("/word/footnotes.xml"), footnotesPart(threeNotes())}},
                                      QByteArray(), noteStyles()));
    QVERIFY(doc);
    QTextDocument *header = doc->editableHeaderFooter(0, false, WriteDoc::kDefaultSlot);
    QVERIFY(header);
    QTextCursor h(header);
    h.movePosition(QTextCursor::End);
    QVERIFY(!insertFootnote(*doc, h));
    QCOMPARE(h.document(), header);
    QCOMPARE(header->toPlainText(), QStringLiteral("Head"));
    QTextDocument *note = doc->editableFootnote(2);
    QVERIFY(note);
    QTextCursor n(note);
    n.movePosition(QTextCursor::End);
    QVERIFY(!insertFootnote(*doc, n));
    QCOMPARE(n.document(), note);
    QCOMPARE(note->toPlainText(), noteMark() + QStringLiteral(" note two text"));
    QVERIFY(!doc->canUndo());
    QVERIFY(!doc->isModified());
}


// ---- footnotes: writing them

// A footnotes part nobody edited is written back byte for byte, though its notes were made ready to edit and shown, and though an edit of one was taken back.
void WriteTests::anUneditedFootnotesPartIsWrittenByteForByte()
{
    // (Bytes a rewrite would change: a comment, odd spacing, an attribute order, an unknown element, a declared prefix nobody uses.)
    const QByteArray odd = "<w:footnotes " + QByteArray(kNs) + " xmlns:x=\"urn:x\"><!-- kept --><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>\n"
                           "<w:footnote w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></w:footnote>"
                           "<w:footnote  w:id=\"2\" x:odd=\"1\"><w:p  w:rsidR=\"00A1\"><w:pPr><w:pStyle w:val=\"FootnoteText\"/><x:odd a=\"1\"/></w:pPr><w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteRef/></w:r>"
                           "<w:r ><w:t>ODD  bytes</w:t></w:r></w:p>\n</w:footnote><x:extra/>"
                           "<w:footnote w:id=\"3\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>three</w:t></w:r></w:p></w:footnote><w:footnote w:id=\"5\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>five</w:t></w:r></w:p></w:footnote></w:footnotes>";
    auto doc = open(notesDocx(threeNotesBody(), odd));
    QVERIFY(doc);
    // Shown on the pages and made ready to edit, not edited.
    QCOMPARE(int(notesOnPage(*doc, 0).size()), 3);
    QVERIFY(doc->editableFootnote(2));
    QTextDocument *three = doc->editableFootnote(3);
    QVERIFY(three);
    QCOMPARE(partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")), xml(odd));
    // An edit that is taken back is no edit.
    typeAtEnd(*doc, three, QStringLiteral("!"));
    QVERIFY(partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")).contains("three!"));
    doc->undo();
    QCOMPARE(partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")), xml(odd));
    // One edited: the others (and the separators, the comment, the extra element) are as they were.
    typeAtEnd(*doc, three, QStringLiteral("!"));
    const QByteArray out = partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml"));
    QVERIFY(out.contains("<w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"));
    QVERIFY(out.contains("<w:footnote  w:id=\"2\" x:odd=\"1\"><w:p  w:rsidR=\"00A1\">"));
    QVERIFY(out.contains("<x:extra/>"));
    QVERIFY(out.contains("<w:footnote w:id=\"5\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>five</w:t></w:r></w:p></w:footnote>"));
    QVERIFY(out.contains("three!"));
    QVERIFY(out.contains(" xmlns:x=\"urn:x\""));
}


// An edited note keeps what the program cannot edit where it was (a bookmark, a field, a picture's run), its own tag and attributes, and the others' bytes; and it reads back as the note that was edited.
void WriteTests::anEditedFootnoteKeepsItsAtomsAndWritesBackAsTheNoteThatWasEdited()
{
    const QByteArray atoms = "<w:r><w:t xml:space=\"preserve\"> before </w:t></w:r><w:bookmarkStart w:id=\"7\" w:name=\"inNote\"/><w:r><w:t>marked</w:t></w:r><w:bookmarkEnd w:id=\"7\"/>"
                             "<w:r><w:tab/></w:r><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> AUTHOR </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>"
                             "<w:r><w:t>Ann</w:t></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r><w:r><w:rPr><w:b/></w:rPr><w:t xml:space=\"preserve\"> bold </w:t></w:r><w:r><w:sym w:font=\"Symbol\" w:char=\"F0B7\"/></w:r>";
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                             "<w:footnote w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>two</w:t></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"3\" w:rsid=\"0012\"><w:p w:rsidR=\"00A2\"><w:pPr><w:pStyle w:val=\"FootnoteText\"/></w:pPr><w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteRef/></w:r>" + atoms +
                             "</w:p><w:p><w:r><w:t>second paragraph</w:t></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"5\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>five</w:t></w:r></w:p></w:footnote></w:footnotes>";
    const QByteArray zip = notesDocx(threeNotesBody(), notes);
    auto doc = open(zip);
    QVERIFY(doc);
    QTextDocument *note = doc->editableFootnote(3);
    QVERIFY(note);
    QVERIFY(note->toPlainText().contains(QStringLiteral("before")) && note->toPlainText().contains(QStringLiteral("bold")));
    // The words are typed in the middle, and a paragraph's end gets some.
    QTextCursor c(note);
    c.setPosition(2);
    insertText(*doc, c, QStringLiteral("NEW"));
    typeAtEnd(*doc, note, QStringLiteral("!"));
    const QByteArray saved = doc->toDocx();
    const QByteArray out = partOf(saved, QStringLiteral("/word/footnotes.xml"));
    QVERIFY2(out.contains("<w:footnote w:id=\"3\" w:rsid=\"0012\">"), out.constData());
    QVERIFY(out.contains("<w:bookmarkStart w:id=\"7\" w:name=\"inNote\"/>") && out.contains("<w:bookmarkEnd w:id=\"7\"/>"));
    QVERIFY(out.contains("<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r>"));
    QVERIFY(out.contains("<w:instrText xml:space=\"preserve\"> AUTHOR </w:instrText>") && out.contains("Ann"));
    QVERIFY(out.contains("<w:sym w:font=\"Symbol\" w:char=\"F0B7\"/>"));
    QVERIFY(out.contains("<w:tab/>"));
    QVERIFY(out.contains("<w:rPr><w:b/></w:rPr>"));
    QVERIFY(out.contains("NEW") && out.contains("second paragraph!"));
    QCOMPARE(int(out.count("<w:footnoteRef/>")), 3);
    QVERIFY(out.contains("<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>two</w:t></w:r></w:p></w:footnote>"));
    QVERIFY(out.contains("<w:footnote w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></w:footnote>"));
    QVERIFY(out.contains("<w:footnote w:id=\"5\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>five</w:t></w:r></w:p></w:footnote>"));
    QVERIFY(rt::strictProblems(saved, zip).isEmpty());
    // It reads back as the note that was edited, and saves the bytes again.
    auto back = open(saved);
    QVERIFY(back);
    QTextDocument *again = back->editableFootnote(3);
    QVERIFY(again);
    QCOMPARE(again->toPlainText(), note->toPlainText());
    QCOMPARE(partOf(back->toDocx(), QStringLiteral("/word/footnotes.xml")), out);
    QCOMPARE(documentXml(back->toDocx()), documentXml(saved));
}


// A document with no notes gets a footnotes part when a footnote is inserted: the part with its separators, its content type and relationship, the settings that name them, and the styles; and it reads back.
void WriteTests::aNewFootnotesPartIsValidAndReadsBack()
{
    const QByteArray original = makeDocxWithParts(paragraphs(3, 20), pageSection(), {}, QByteArray(), kStyles);
    auto doc = open(original);
    QVERIFY(doc);
    QTextCursor c(doc->story());
    c.setPosition(doc->story()->findBlockByNumber(1).position() + 4);
    QTextDocument *note = insertFootnote(*doc, c);
    QVERIFY(note);
    typeAtEnd(*doc, note, QStringLiteral("A new note."));
    const QByteArray saved = doc->toDocx();
    QStringList problems = rt::strictProblems(saved, original);
    QVERIFY2(problems.isEmpty(), qPrintable(problems.join(QLatin1Char('\n'))));
    QVERIFY(rt::danglingReferences(saved).isEmpty());
    office::Package pkg;
    QVERIFY(pkg.read(saved));
    QVERIFY(pkg.hasPart(QStringLiteral("/word/footnotes.xml")));
    QCOMPARE(pkg.contentType(QStringLiteral("/word/footnotes.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.footnotes+xml"));
    QCOMPARE(int(pkg.relationshipsOfType(pkg.mainPart(), QStringLiteral("/footnotes")).size()), 1);
    QVERIFY(pkg.relationshipsOfType(pkg.mainPart(), QStringLiteral("/footnotes")).first().target.endsWith(QLatin1String("footnotes.xml")));
    QVector<wml::NotePart> notes;
    QVERIFY(wml::readNotes(pkg.part(QStringLiteral("/word/footnotes.xml")), &notes));
    QCOMPARE(int(notes.size()), 3);
    QCOMPARE(notes[0].type, QStringLiteral("separator"));
    QCOMPARE(notes[1].type, QStringLiteral("continuationSeparator"));
    QCOMPARE(notes[2].id, 1);
    // The settings name the separators (a part made for it, with its relationship and type), and the styles that the note and its reference use are there.
    QCOMPARE(int(pkg.relationshipsOfType(pkg.mainPart(), QStringLiteral("/settings")).size()), 1);
    const QString settingsName = pkg.relationshipsOfType(pkg.mainPart(), QStringLiteral("/settings")).first().target;
    QVERIFY(pkg.part(settingsName).contains("<w:footnotePr><w:footnote w:id=\"-1\"/><w:footnote w:id=\"0\"/></w:footnotePr>"));
    QCOMPARE(pkg.contentType(settingsName), QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"));
    const QByteArray styles = pkg.part(QStringLiteral("/word/styles.xml"));
    QVERIFY(styles.contains("w:styleId=\"FootnoteText\"") && styles.contains("w:styleId=\"FootnoteReference\""));
    const QByteArray main = documentXml(saved);
    QVERIFY2(main.contains("<w:footnoteReference w:id=\"1\"/>") && main.contains("<w:rStyle w:val=\"FootnoteReference\"/>"), main.constData());
    // It reads back: the reference, the note and its words, on the page; saved again, the same bytes.
    auto back = open(saved);
    QVERIFY(back);
    QCOMPARE(int(back->footnoteReferences(1).size()), 1);
    QTextDocument *readBack = back->editableFootnote(1);
    QVERIFY(readBack && readBack->toPlainText().endsWith(QStringLiteral("A new note.")));
    QCOMPARE(int(notesOnPage(*back, 0).size()), 1);
    QCOMPARE(partOf(back->toDocx(), QStringLiteral("/word/footnotes.xml")), pkg.part(QStringLiteral("/word/footnotes.xml")));
    QCOMPARE(documentXml(back->toDocx()), main);
    // A document that had a footnotes part with only its separators gets the note in that part, and its settings are as they were.
    const QByteArray settings = "<w:footnotePr><w:footnote w:id=\"-1\"/><w:footnote w:id=\"0\"/></w:footnotePr>";
    const QByteArray bare = notesDocx(paragraphs(2, 20), footnotesPart({}), settings);
    auto doc2 = open(bare);
    QVERIFY(doc2);
    QTextCursor c2(doc2->story());
    c2.setPosition(3);
    QVERIFY(insertFootnote(*doc2, c2));
    office::Package pkg2;
    QVERIFY(pkg2.read(doc2->toDocx()));
    office::Package was;
    QVERIFY(was.read(bare));
    QCOMPARE(pkg2.part(QStringLiteral("/word/settings.xml")), was.part(QStringLiteral("/word/settings.xml")));
    QCOMPARE(int(pkg2.relationshipsOfType(pkg2.mainPart(), QStringLiteral("/footnotes")).size()), 1);
    QVERIFY(pkg2.part(QStringLiteral("/word/footnotes.xml")).contains("w:id=\"1\""));
}


// A footnote put in and taken back leaves the document as it was: no part, no relationship, no styles, not modified.
void WriteTests::aFootnoteInsertedAndTakenBackLeavesTheFileAsItWas()
{
    const QByteArray original = makeDocxWithParts(paragraphs(3, 20), pageSection(), {}, QByteArray(), kStyles);
    auto doc = open(original);
    QVERIFY(doc);
    const QByteArray before = doc->toDocx();
    QTextCursor c(doc->story());
    c.setPosition(4);
    QTextDocument *note = insertFootnote(*doc, c);
    QVERIFY(note);
    typeAtEnd(*doc, note, QStringLiteral("typed"));
    while (doc->canUndo()) doc->undo();
    QVERIFY(!doc->isModified());
    const QByteArray after = doc->toDocx();
    office::Package a, b;
    QVERIFY(a.read(before));
    QVERIFY(b.read(after));
    QCOMPARE(b.partNames(), a.partNames());
    QCOMPARE(b.part(QStringLiteral("/word/styles.xml")), a.part(QStringLiteral("/word/styles.xml")));
    QCOMPARE(documentXml(after), documentXml(before));
    QVERIFY(!b.hasPart(QStringLiteral("/word/footnotes.xml")));
    QCOMPARE(int(b.relationships(b.mainPart()).size()), int(a.relationships(a.mainPart()).size()));
}


// ---- footnotes: in the window

// A point on a page, in points from its top left corner, as a place in the view's window.
QPoint WriteTests::pagePoint(PageView &view, int page, double x, double y)
{
    const QRect r = view.pageRect(page);
    return QPoint(r.left() + int(x * 96 / 72 * view.zoom()), r.top() + int(y * 96 / 72 * view.zoom()));
}


// The box of a note on a page (in page coordinates): where the page sets the note with that Id.
bool WriteTests::noteBoxOf(WriteDoc &doc, int page, int id, QRectF *rect)
{
    const auto pg = doc.pager().materialize(page);
    if (!pg) return false;
    for (const auto &nb : pg->noteBoxes) {
        int found = 0;
        if (!doc.footnoteIdOf(nb.source, &found) || found != id) continue;
        if (const Item *item = pg->doc->item(nb.itemId)) {
            *rect = item->rect;
            return true;
        }
    }
    return false;
}


// Clicking in a note's words at the foot of a page puts the caret there: typing, deleting, selecting, and the clipboard are the note's, and the text of the page is as it was.
void WriteTests::clickingInAFootnoteOnThePagePutsTheCaretThereAndEditsAreTheNotes()
{
    auto doc = open(threeNotesDocx());
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 1200);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    QSignalSpy mode(&view, &PageView::noteModeChanged);
    QRectF box;
    QVERIFY(noteBoxOf(*doc, 0, 3, &box));
    QTextDocument *note = doc->editableFootnote(3);
    QVERIFY(note);
    const QString body = plain(*doc);
    QVERIFY(!view.inNote());
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(view, 0, box.left() + 60, box.center().y()));
    QVERIFY(view.inNote());
    QCOMPARE(mode.count(), 1);
    QCOMPARE(view.noteEdit().id, 3);
    QCOMPARE(view.noteEdit().page, 0);
    QCOMPARE(view.cursor().document(), note);
    QVERIFY(view.cursor().position() >= noteTextStart(note));
    // The caret is drawn in the note's box.
    QRect caret;
    QVERIFY(view.caretRect(&caret));
    const QRect inView(pagePoint(view, 0, box.left(), box.top()), pagePoint(view, 0, box.right(), box.bottom()));
    QVERIFY2(inView.adjusted(-3, -3, 3, 3).contains(caret), qPrintable(QStringLiteral("caret %1,%2 box %3,%4 %5x%6").arg(caret.x()).arg(caret.y()).arg(inView.x()).arg(inView.y()).arg(inView.width()).arg(inView.height())));
    // Typing and deleting.
    QTest::keyClicks(&view, "xyz");
    QVERIFY(note->toPlainText().contains(QLatin1String("xyz")));
    QTest::keyClick(&view, Qt::Key_Backspace);
    QVERIFY(note->toPlainText().contains(QLatin1String("xy")) && !note->toPlainText().contains(QLatin1String("xyz")));
    const int length = int(note->toPlainText().size());
    QTest::keyClick(&view, Qt::Key_Delete);
    QCOMPARE(int(note->toPlainText().size()), length - 1);
    // Selecting with the keys, and the clipboard: a copy of the words, pasted at the end.
    QTest::keyClick(&view, Qt::Key_Left, Qt::ShiftModifier);
    QTest::keyClick(&view, Qt::Key_Left, Qt::ShiftModifier);
    QVERIFY(view.cursor().hasSelection());
    QCOMPARE(view.cursor().document(), note);
    const QString picked = view.cursor().selectedText();
    view.copy();
    QTest::keyClick(&view, Qt::Key_End, Qt::ControlModifier);
    view.paste();
    QVERIFY(note->toPlainText().endsWith(picked));
    // Everything in the note selected and cut: the words go, the number stays; and an undo gives them back.
    view.selectAll();
    QCOMPARE(view.cursor().selectionStart(), noteTextStart(note));
    view.cut();
    QCOMPARE(note->toPlainText(), noteMark());
    view.undo();
    QVERIFY(note->toPlainText().size() > 10);
    // The text of the page is as it was, and the note is the document's the whole time.
    QCOMPARE(plain(*doc), body);
    QVERIFY(doc->isModified());
    QVERIFY(layoutDiffersFromFresh(*doc).isEmpty());
}


// The commands that change how text looks work in a note as they do in the text.
void WriteTests::theLooksOfTextAreGivenToWordsInAFootnote()
{
    WriteWindow w;
    w.resize(1300, 1000);
    w.show();
    w.setDocument(open(threeNotesDocx()), QString());
    WriteDoc *doc = w.document();
    w.view()->paginateAll();
    QVERIFY(w.view()->enterFootnote(3));
    QTextDocument *note = doc->editableFootnote(3);
    QVERIFY(note);
    QTextCursor &c = w.view()->cursor();
    QCOMPARE(c.document(), note);
    c.setPosition(noteTextStart(note));
    c.setPosition(note->characterCount() - 1, QTextCursor::KeepAnchor);
    w.act("fmt.bold")->trigger();
    w.act("fmt.alignRight")->trigger();
    QVERIFY(allSelected(c, [](const QTextCharFormat &f) { return f.fontWeight() >= QFont::DemiBold; }));
    QCOMPARE(int(note->begin().blockFormat().alignment() & Qt::AlignHorizontal_Mask), int(Qt::AlignRight));
    // (The commands for a page's breaks and for a note are gray in a note.)
    QTRY_VERIFY(!w.act("insert.pageBreak")->isEnabled());
    QVERIFY(!w.act("layout.sectionBreak")->isEnabled());
    QVERIFY(!w.act("refs.footnote")->isEnabled());
    const QByteArray part = partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml"));
    QVERIFY2(part.contains("<w:b/>") && part.contains("<w:jc w:val=\"right\"/>"), part.constData());
    // Out of the note, and they are enabled again.
    QTest::keyClick(w.view(), Qt::Key_Escape);
    QVERIFY(!w.view()->inNote());
    QTRY_VERIFY(w.act("insert.pageBreak")->isEnabled());
    QVERIFY(w.act("refs.footnote")->isEnabled());
}


// Neither the number that begins a note nor the line above the notes can be edited: a click on the number puts the caret after it, and a click on the line is in neither note.
void WriteTests::aClickOnTheNumberMarkOrTheSeparatorEditsNothing()
{
    auto doc = open(threeNotesDocx());
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 1200);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    QRectF box;
    QVERIFY(noteBoxOf(*doc, 0, 2, &box));
    QTextDocument *note = doc->editableFootnote(2);
    QVERIFY(note);
    const QString was = note->toPlainText();
    // A click on the number (the left edge of the box): the caret is after it; Home keeps it there; Backspace and a select-all with Delete leave it.
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(view, 0, box.left() + 0.5, box.center().y()));
    QVERIFY(view.inNote());
    QVERIFY(view.cursor().position() >= noteTextStart(note));
    QTest::keyClick(&view, Qt::Key_Home);
    QCOMPARE(view.cursor().position(), noteTextStart(note));
    QTest::keyClick(&view, Qt::Key_Backspace);
    QCOMPARE(note->toPlainText(), was);
    view.selectAll();
    QTest::keyClick(&view, Qt::Key_Delete);
    QCOMPARE(note->toPlainText(), noteMark());
    QTest::keyClick(&view, Qt::Key_Backspace);
    QCOMPARE(note->toPlainText(), noteMark());
    view.undo();
    QCOMPARE(note->toPlainText(), was);
    // The line above the notes: a click on it enters no note (the text is where the caret goes).
    view.leaveNote();
    const auto page = doc->pager().materialize(0);
    const LineItem *rule = nullptr;
    for (const auto &item : page->doc->pages[0]->items)
        if (const auto *line = dynamic_cast<const LineItem *>(item.get())) rule = line;
    QVERIFY(rule);
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(view, 0, (rule->p1.x() + rule->p2.x()) / 2, rule->p1.y()));
    QVERIFY(!view.inNote());
    QCOMPARE(view.cursor().document(), doc->story());
    // And the separator is written as it was whatever was done.
    const QByteArray part = partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml"));
    QVERIFY(part.contains("<w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"));
    QVERIFY(part.contains("<w:footnote w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:continuationSeparator/></w:r></w:p></w:footnote>"));
}


// Up from a note's first line and Down from its last leave it, with the caret in the text just after the note's reference; so do Esc and a click in the text. Inside the note the keys move by its lines.
void WriteTests::upAndDownAndAClickInTheTextLeaveTheNote()
{
    QByteArray longNote;
    for (int i = 0; i < 40; ++i) longNote += plainRun(" words that fill the lines of this note, " + QByteArray::number(i));
    auto doc = open(notesDocx(citing("First paragraph", 2) + paragraphs(3, 30) + citing("Second", 3), footnotesPart({{2, plainRun(" short note")}, {3, longNote}})));
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 1200);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    QSignalSpy mode(&view, &PageView::noteModeChanged);
    QRectF box;
    QVERIFY(noteBoxOf(*doc, 0, 2, &box));
    QTextDocument *shortNote = doc->editableFootnote(2);
    QVERIFY(shortNote);
    const int after = doc->footnoteReferences(2).first() + 1;
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(view, 0, box.left() + 40, box.center().y()));
    QVERIFY(view.inNote());
    QTest::keyClick(&view, Qt::Key_Up);
    QVERIFY(!view.inNote());
    QCOMPARE(view.cursor().document(), doc->story());
    QCOMPARE(view.cursor().position(), after);
    QCOMPARE(mode.count(), 2);
    QVERIFY(view.enterFootnote(2));
    QTest::keyClick(&view, Qt::Key_Down);
    QVERIFY(!view.inNote());
    QCOMPARE(view.cursor().position(), after);
    QVERIFY(view.enterFootnote(2));
    QTest::keyClick(&view, Qt::Key_Escape);
    QVERIFY(!view.inNote());
    QCOMPARE(view.cursor().position(), after);
    // A click in the text of the page leaves the note, and the caret is where the click was.
    QVERIFY(view.enterFootnote(2));
    QTest::mouseClick(view.viewport(), Qt::LeftButton, Qt::NoModifier, pagePoint(view, 0, 200, 120));
    QVERIFY(!view.inNote());
    QCOMPARE(view.cursor().document(), doc->story());
    QVERIFY(view.cursor().position() != after);
    // A note of many lines: the keys move by its lines first. From the last line, Up goes up the note, and only from the first does it leave.
    QRectF big;
    const int page = doc->pager().pageOfPosition(doc->footnoteReferences(3).first());
    QVERIFY(noteBoxOf(*doc, page, 3, &big));
    QVERIFY(view.enterFootnote(3));
    QTextDocument *longStory = doc->editableFootnote(3);
    view.cursor().setPosition(longStory->characterCount() - 1);
    const int last = view.cursor().position();
    QTest::keyClick(&view, Qt::Key_Up);
    QVERIFY(view.inNote());
    QVERIFY(view.cursor().position() < last);
    for (int i = 0; i < 60 && view.inNote(); ++i) QTest::keyClick(&view, Qt::Key_Up);
    QVERIFY(!view.inNote());
    QCOMPARE(view.cursor().position(), doc->footnoteReferences(3).first() + 1);
}


// The one history holds the text's steps, a header's, and a note's, in the order they were made; the view follows an undo into the note or the header it was made in.
void WriteTests::oneHistoryInterleavesTheTextAHeaderAndANote()
{
    auto doc = open(makeDocxWithParts(threeNotesBody(), sectionNaming({{"default", "rIdheader1"}}),
                                      {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {"<w:r><w:t>Head</w:t></w:r>"})}, {QStringLiteral("/word/footnotes.xml"), footnotesPart(threeNotes())}},
                                      QByteArray(), noteStyles()));
    QVERIFY(doc);
    PageView view;
    view.resize(1000, 1200);
    view.show();
    view.setDocument(doc.get());
    view.paginateAll();
    QTextDocument *header = doc->editableHeaderFooter(0, false, WriteDoc::kDefaultSlot);
    QTextDocument *note = doc->editableFootnote(5);
    QVERIFY(header && note);
    QTextCursor body(doc->story());
    body.setPosition(0);
    insertText(*doc, body, QStringLiteral("B"));
    typeAtEnd(*doc, header, QStringLiteral("H"));
    typeAtEnd(*doc, note, QStringLiteral("N"));
    insertText(*doc, body, QStringLiteral("b"));
    QCOMPARE(header->toPlainText(), QStringLiteral("HeadH"));
    // Undone one step at a time, the view goes where the step was.
    view.undo();   // the text's second step
    QVERIFY(!view.inNote() && !view.inHeaderFooter());
    QVERIFY(plain(*doc).startsWith(QStringLiteral("B")) && !plain(*doc).startsWith(QStringLiteral("Bb")));
    view.undo();   // the note's
    QVERIFY(view.inNote());
    QCOMPARE(view.noteEdit().id, 5);
    QCOMPARE(view.cursor().document(), note);
    QVERIFY(!note->toPlainText().endsWith(QLatin1Char('N')));
    view.undo();   // the header's: the view leaves the note for it
    QVERIFY(view.inHeaderFooter());
    QVERIFY(!view.inNote());
    QCOMPARE(header->toPlainText(), QStringLiteral("Head"));
    view.undo();   // the text's first
    QVERIFY(!view.inHeaderFooter() && !view.inNote());
    QVERIFY(!plain(*doc).startsWith(QLatin1Char('B')));
    QVERIFY(!doc->canUndo());
    // Done again, in the same order.
    view.redo();
    view.redo();
    QVERIFY(view.inHeaderFooter());
    view.redo();
    QVERIFY(view.inNote());
    QVERIFY(note->toPlainText().endsWith(QLatin1Char('N')));
    view.redo();
    QVERIFY(!view.inNote() && !view.inHeaderFooter());
    QVERIFY(plain(*doc).startsWith(QStringLiteral("Bb")));
    QVERIFY(!doc->canRedo());
    QVERIFY(layoutDiffersFromFresh(*doc).isEmpty());
}


// Insert Footnote on the References tab, and Ctrl+Alt+F: the window makes the note and the caret is in it, at the foot of the page, for typing.
void WriteTests::insertFootnoteIsOnTheReferencesTabAndMovesTheCaretIntoTheNote()
{
    WriteWindow w;
    w.resize(1300, 1000);
    w.show();
    QVERIFY(QTest::qWaitForWindowActive(&w));
    QStringList tabs;
    for (int i = 0; i < w.ribbon()->tabCount(); ++i) tabs << w.ribbon()->tabName(i);
    QVERIFY2(tabs.contains(QStringLiteral("References")), qPrintable(tabs.join(QLatin1Char(','))));
    QVERIFY(tabs.indexOf(QStringLiteral("Layout")) < tabs.indexOf(QStringLiteral("References")) && tabs.indexOf(QStringLiteral("References")) < tabs.indexOf(QStringLiteral("Review")));
    QVERIFY(w.act("refs.footnote"));
    QCOMPARE(w.act("refs.footnote")->shortcut(), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F));
    w.setDocument(open(makeDocxWithParts(paragraphs(4, 40), pageSection(), {}, QByteArray(), kStyles)), QString());
    WriteDoc *doc = w.document();
    w.view()->paginateAll();
    w.view()->cursor().setPosition(doc->story()->findBlockByNumber(1).position() + 5);
    QVERIFY(w.act("refs.footnote")->isEnabled());
    w.act("refs.footnote")->trigger();
    QVERIFY(w.view()->inNote());
    QCOMPARE(w.view()->noteEdit().id, 1);
    QCOMPARE(w.view()->cursor().document(), doc->editableFootnote(1));
    QCOMPARE(w.view()->cursor().position(), doc->editableFootnote(1)->characterCount() - 1);
    // (The caret is drawn in the note, on the page.)
    QRect caret;
    QVERIFY(w.view()->caretRect(&caret));
    QRectF box;
    QVERIFY(noteBoxOf(*doc, 0, 1, &box));
    QTest::keyClicks(w.view(), "A note typed in the window");
    QVERIFY(doc->editableFootnote(1)->toPlainText().endsWith(QStringLiteral("A note typed in the window")));
    QVERIFY(partOf(doc->toDocx(), QStringLiteral("/word/footnotes.xml")).contains("A note typed in the window"));
    // The key combination does it too, from the text: a second note, numbered by its place.
    QTest::keyClick(w.view(), Qt::Key_Escape);
    QVERIFY(!w.view()->inNote());
    w.view()->cursor().setPosition(doc->story()->findBlockByNumber(0).position() + 3);
    QTest::keyClick(w.view(), Qt::Key_F, Qt::ControlModifier | Qt::AltModifier);
    QVERIFY(w.view()->inNote());
    QCOMPARE(w.view()->noteEdit().id, 2);
    QCOMPARE(notesOnPage(*doc, 0), (QList<std::pair<int, int>>{{2, 1}, {1, 2}}));
    // Not in a header: the command is gray and does nothing.
    QTest::keyClick(w.view(), Qt::Key_Escape);
    QVERIFY(w.view()->enterHeaderFooter(0, false, true));
    QTRY_VERIFY(!w.act("refs.footnote")->isEnabled());
    const int notes = int(notesOnPage(*doc, 0).size());
    w.act("refs.footnote")->trigger();
    QCOMPARE(int(notesOnPage(*doc, 0).size()), notes);
    QVERIFY(w.view()->inHeaderFooter());
}


// A footnote being edited, as the window shows it (a picture of it is saved where JW_NOTE_SHOT says, for the documentation; without it, the picture is only made).
void WriteTests::aFootnoteBeingEditedIsShownInTheWindow()
{
    QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>Results for the third quarter</w:t></w:r></w:p>";
    body += citing("Sales rose in every region, led by the northern offices", 2);
    body += paragraphs(3, 45);
    body += citing("The cost of materials fell for the second quarter in a row", 3);
    const QByteArray notes = footnotesPart({{2, plainRun(" Figures are in thousands, and the quarter ended on September 30.")},
                                            {3, plainRun(" Prices come from the suppliers' own lists, which are published every month.")}});
    WriteWindow w;
    w.resize(1440, 1270);
    w.show();
    w.setDocument(open(notesDocx(body, notes)), QString());
    w.view()->setZoom(1.0);
    w.view()->paginateAll();
    QVERIFY(w.view()->enterFootnote(3));
    // The caret after "Prices come from", and the words after it to the end of the line selected: the picture shows the caret's place and a selection.
    QTextCursor &c = w.view()->cursor();
    QTextDocument *note = w.document()->editableFootnote(3);
    c.setPosition(noteTextStart(note) + 1);
    c.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
    c.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
    w.view()->setCaretWithoutFocus(true);
    // The ribbon follows the caret a moment later (it refreshes on a timer): its font and size boxes show the note's own look, not the heading's they showed before.
    const QTextCharFormat shown = c.charFormat();
    QVERIFY(shown.fontPointSize() > 0 && shown.fontPointSize() < 17);
    const QList<FontCombo *> fonts = w.findChildren<FontCombo *>();
    const QList<SizeCombo *> sizes = w.findChildren<SizeCombo *>();
    QVERIFY(!fonts.isEmpty() && !sizes.isEmpty());
    const QString family = shown.hasProperty(QTextFormat::FontFamilies) && !shown.fontFamilies().toStringList().isEmpty() ? shown.fontFamilies().toStringList().first() : w.document()->flow().env.fonts.body;
    QTRY_COMPARE(sizes.first()->currentText(), QString::number(shown.fontPointSize()));
    QTRY_COMPARE(fonts.first()->currentText(), family);
    QCoreApplication::processEvents();
    const QImage shot = w.grab().toImage();
    QVERIFY(!shot.isNull());
    QCOMPARE(shot.size(), w.size() * int(shot.devicePixelRatio()));
    QVERIFY(w.view()->inNote());
    // The picture has the note in it: text is drawn in the box of the note, over the white of the page.
    QRectF box;
    QVERIFY(noteBoxOf(*w.document(), 0, 3, &box));
    const QRect inWindow(w.view()->viewport()->mapTo(&w, pagePoint(*w.view(), 0, box.left(), box.top())), w.view()->viewport()->mapTo(&w, pagePoint(*w.view(), 0, box.right(), box.bottom())));
    int ink = 0;
    for (int y = inWindow.top(); y < inWindow.bottom(); y += 2)
        for (int x = inWindow.left(); x < inWindow.right(); x += 2) ink += qGray(shot.pixel(QPoint(x, y) * int(shot.devicePixelRatio()))) < 128;
    QVERIFY2(ink > 50, qPrintable(QString::number(ink)));
    if (qEnvironmentVariableIsSet("JW_NOTE_SHOT")) QVERIFY(shot.save(qEnvironmentVariable("JW_NOTE_SHOT")));
}


// A note that goes on from page to page (it is cited among tables and objects, and the pages that hold the rest of it are not the pages right after the one that cites it) is
// edited anywhere in its words, and the pages after are as a new paginator makes them: those that hold the rest of it are laid out again too.
void WriteTests::aNoteThatGoesOnFromPageToPageIsEditedAnywhereAndThePagesFollow()
{
    // (The note is made long enough to be cut over three pages or more, whatever the fonts the machine has: its length doubles until it is.)
    std::unique_ptr<WriteDoc> doc;
    QTextDocument *note = nullptr;
    int carriers = 0;
    for (int words = 220; carriers < 2 && words <= 7040; words *= 2) {
        doc = open(richMixedDocx(words));
        QVERIFY(doc);
        note = doc->editableFootnote(4);
        QVERIFY(note);
        carriers = 0;
        for (int k = 0; k < doc->pager().pageCount(); ++k)
            for (const QString &c : doc->pager().page(k).start.carried) carriers += c.startsWith(QLatin1String("fn4|"));
    }
    QVERIFY2(carriers >= 2, qPrintable(QString::number(carriers)));
    QVERIFY(layoutDiffersFromFresh(*doc).isEmpty());
    const int total = note->characterCount();
    for (int kind = 0; kind < 4; ++kind)
        for (int at = 1; at < total - 1; at += 151) {
            QTextCursor nc(note);
            nc.setPosition(at);
            nc.setPosition(std::min(at + 25, total - 1), QTextCursor::KeepAnchor);
            switch (kind) {
            case 0: removeSelection(*doc, nc); break;
            case 1: toggleBold(*doc, nc); break;
            case 2: deleteBack(*doc, nc, false); break;
            default: insertParagraph(*doc, nc); break;
            }
            const QString d = layoutDiffersFromFresh(*doc);
            QVERIFY2(d.isEmpty(), qPrintable(QStringLiteral("kind %1 at %2: %3").arg(kind).arg(at).arg(d.left(400))));
            while (doc->canUndo()) doc->undo();
            const QString back = layoutDiffersFromFresh(*doc);
            QVERIFY2(back.isEmpty(), qPrintable(QStringLiteral("after undoing kind %1 at %2: %3").arg(kind).arg(at).arg(back.left(400))));
        }
}


// ---- endnotes: editing them where they stand

// The paragraphs of the story that are one endnote's (the first copy of it), in order.
QVector<QTextBlock> WriteTests::endnoteParagraphs(WriteDoc &doc, int id)
{
    QVector<QTextBlock> out;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) {
        int found = 0;
        if (doc.endnoteIdOf(b, &found) && found == id && b.blockFormat().intProperty(prop::EndnoteNote) > 0) out << b;
    }
    return out;
}


// The notes the pages set, in order, each as its Id and the number its mark shows ("2:1"); and the references of the text, each as the Id it names and the number it shows.
QStringList WriteTests::endnoteMarks(WriteDoc &doc)
{
    QStringList out;
    int last = 0;
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) {
        int id = 0;
        const int serial = b.blockFormat().intProperty(prop::EndnoteNote);
        if (!doc.endnoteIdOf(b, &id) || serial == last) continue;
        last = serial;
        out << QStringLiteral("%1:%2").arg(id).arg(b.begin().fragment().charFormat().stringProperty(tp::Field).mid(4));
    }
    return out;
}
