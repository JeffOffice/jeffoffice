#include "tst_write.h"


// A run of paragraphs with deleted marks is cut into paragraphs of 32 of them (counted by the paragraphs' numbers in the story), whatever the run's length: the longest one made is 32.
void WriteTests::aLongRunOfJoinedParagraphsIsCutIntoParagraphsOfThirtyTwo()
{
    const QByteArray deleted = "<w:rPr><w:del w:id=\"7\" w:author=\"a\"/></w:rPr>";
    QByteArray body;
    for (int i = 0; i < 100; ++i) body += "<w:p><w:pPr>" + (i + 1 < 100 ? deleted : QByteArray()) + "</w:pPr><w:r><w:t xml:space=\"preserve\">paragraph " + QByteArray::number(i) + " </w:t></w:r></w:p>";
    StoryLayout::resetLongestJoinedParagraphsForTest();
    auto doc = open(makeDocx(body, kStyles));
    QVERIFY(doc);
    QVERIFY(doc->pager().pageCount() >= 1);
    QCOMPARE(StoryLayout::longestJoinedParagraphsForTest(), qint64(StoryLayout::kMostJoinedParagraphs));
}


// The runs are cut at the same places wherever a layout begins: one that begins in the paragraph after a cut, or in the rest of a paragraph, has the lines the layout of the whole
// story has (the same paragraphs, every word once), and the items of a list are numbered as they are there (a run is one item).
void WriteTests::aJoinedRunIsCutAtTheSamePlacesWhereverALayoutBegins()
{
    const QByteArray deleted = "<w:rPr><w:del w:id=\"7\" w:author=\"a\"/></w:rPr>";
    const QByteArray level = "<w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl>";
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\">" + level + "</w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
    const QByteArray item = "<w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"6000\" w:h=\"4800\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>";
    const int count = 70;
    QByteArray body;
    for (int i = 0; i < count; ++i)
        body += "<w:p><w:pPr>" + item + (i + 1 < count ? deleted : QByteArray()) + "</w:pPr><w:r><w:t xml:space=\"preserve\">paragraph " + QByteArray::number(i) + " of the run goes on a while. </w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles, numbering, sect));
    QVERIFY(doc);
    const QTextDocument *story = doc->story();
    const QString whole = story->toPlainText();
    auto at = [&](int number) { return story->findBlockByNumber(number).position(); };
    FrameSpec fs;
    fs.size = QSizeF(300, 100000);
    fs.insets = QMarginsF(0, 0, 0, 0);
    fs.hyphenate = false;
    auto markers = [&](const StoryLayout &layout) {
        QMap<int, QString> out;
        for (const auto &l : layout.lineInfo(0))
            if (!l.marker.isEmpty()) out[l.docStart] = l.marker;
        return out;
    };
    StoryLayout topDown;
    topDown.build(story, {fs}, doc->flow().env);
    const auto all = topDown.lineBriefs();
    // (Runs of 32 paragraphs by their numbers: 0 to 31, 32 to 63, and 64 to 69, each an item: 1., 2., and 3.)
    const auto top = markers(topDown);
    QCOMPARE(top.size(), 3);
    QCOMPARE(top.value(at(0)), QStringLiteral("1."));
    QCOMPARE(top.value(at(32)), QStringLiteral("2."));
    QCOMPARE(top.value(at(64)), QStringLiteral("3."));
    // A layout that begins in paragraph 33, laid out whole, is that of the run from paragraph 32 on.
    {
        StoryLayout::Span span;
        span.from = at(33);
        StoryLayout later;
        later.build(story, {fs}, doc->flow().env, span);
        QVector<StoryLayout::LineBrief> wanted;
        for (const auto &l : all)
            if (l.block >= at(32)) wanted << l;
        const auto got = later.lineBriefs();
        QCOMPARE(got.size(), wanted.size());
        for (int i = 0; i < got.size(); ++i)
            QVERIFY2(got[i].block == wanted[i].block && got[i].line == wanted[i].line && got[i].pos == wanted[i].pos && got[i].end == wanted[i].end,
                     qPrintable(QStringLiteral("line %1 is %2 to %3 of the paragraph at %4, the whole story's %5 to %6 of the one at %7").arg(i).arg(got[i].pos).arg(got[i].end).arg(got[i].block)
                                    .arg(wanted[i].pos).arg(wanted[i].end).arg(wanted[i].block)));
        QMap<int, QString> expected = top;
        expected.remove(at(0));
        QCOMPARE(markers(later), expected);
    }
    // One that begins in the rest of a paragraph in the middle of the second run shows every word of the rest once.
    {
        StoryLayout::Span span;
        span.from = at(40) + 3;
        span.continued = true;
        StoryLayout rest;
        rest.build(story, {fs}, doc->flow().env, span);
        QString shown;
        int last = span.from;
        for (const auto &l : rest.lineBriefs()) {
            QVERIFY2(l.pos >= last, qPrintable(QStringLiteral("a line at %1 begins before the one before it ends, at %2").arg(l.pos).arg(last)));
            shown += whole.mid(l.pos, l.end - l.pos);
            last = l.end;
        }
        QCOMPARE(shown.remove(QLatin1Char('\n')), whole.mid(span.from).remove(QLatin1Char('\n')));
    }
    // And the pages, which begin where the lines run out of room (in the middle of a run, and at the cut), show every word once.
    {
        const int pages = doc->pager().pageCount();
        QVERIFY(pages >= 3);
        QString shown;
        int last = 0;
        for (int i = 0; i < pages; ++i)
            for (const auto &l : doc->pager().lines(i)) {
                QVERIFY2(l.pos >= last, qPrintable(QStringLiteral("page %1: a line at %2 begins before the one before it ends, at %3").arg(i + 1).arg(l.pos).arg(last)));
                shown += whole.mid(l.pos, l.end - l.pos);
                last = l.end;
            }
        QCOMPARE(shown.remove(QLatin1Char('\n')), QString(whole).remove(QLatin1Char('\n')));
    }
}


// The copy of the text that a page is drawn from holds the page's own text of a run of joined paragraphs, not the whole of the run's paragraphs: the characters copied for the pages
// of thirty-two paragraphs of thousands of characters, each page a few lines of them, are about the characters there are, not that many for every page. (Counted.)
void WriteTests::thePagesCopiesOfAJoinedRunHoldOnlyTheirOwnText()
{
    const QByteArray deleted = "<w:rPr><w:del w:id=\"7\" w:author=\"a\"/></w:rPr>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"6000\" w:h=\"4800\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>";
    QByteArray body;
    for (int i = 0; i < 32; ++i) {
        QByteArray text;
        for (int w = 0; w < 200; ++w) text += "word" + QByteArray::number(w % 10) + " ";
        body += "<w:p><w:pPr>" + (i < 31 ? deleted : QByteArray()) + "</w:pPr><w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r></w:p>";
    }
    body += "<w:p><w:r><w:t>After.</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles, QByteArray(), sect));
    QVERIFY(doc);
    const int pages = doc->pager().pageCount();
    QVERIFY2(pages >= 40, qPrintable(QString::number(pages)));
    const qint64 before = doc->pager().charsCopiedForTest();
    for (int i = 0; i < pages; ++i) QVERIFY(doc->pager().materialize(i));
    const qint64 copied = doc->pager().charsCopiedForTest() - before, length = doc->story()->characterCount();
    QVERIFY2(copied <= 2 * length, qPrintable(QStringLiteral("%1 characters copied for the %2 pages of a story of %3").arg(copied).arg(pages).arg(length)));
}


// The copy of a page's text is made in one edit: a paragraph of thousands of runs of one character each, set small, is a page of thousands of pieces, and the copy of that page takes
// a moment, not the time of laying out the paragraph for every piece put in. (A ceiling many times what the page takes alone.)
void WriteTests::aPageOfThousandsOfRunsIsCopiedInOneEdit()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>";
    QByteArray runs;
    for (int i = 0; i < 6000; ++i) runs += QByteArray(i % 2 ? "<w:r><w:rPr><w:b/><w:sz w:val=\"16\"/></w:rPr><w:t>b</w:t></w:r>" : "<w:r><w:rPr><w:sz w:val=\"16\"/></w:rPr><w:t>a</w:t></w:r>");
    auto doc = open(makeDocx("<w:p>" + runs + "</w:p>", kStyles, QByteArray(), sect));
    QVERIFY(doc);
    QVERIFY(doc->pager().pageCount() >= 1);
    const qint64 best = bestNanoseconds([&] { doc->pager().materialize(0); }, 3);
    QVERIFY2(best < 600000000LL, qPrintable(QStringLiteral("%1 ms to copy one page of %2 runs").arg(best / 1000000).arg(6000)));
}


// A line reads the format ranges that reach it, not every range of its paragraph: a paragraph of thousands of runs of two looks, laid out, has the ranges the scans of its lines read
// counted at some tens of times the ranges there are, not at the ranges times the lines. (Counted.)
void WriteTests::theLinesOfAParagraphOfThousandsOfRunsReadOnlyTheirOwnRanges()
{
    QByteArray runs;
    for (int i = 0; i < 8000; ++i) runs += QByteArray(i % 2 ? "<w:r><w:rPr><w:b/></w:rPr><w:t>b </w:t></w:r>" : "<w:r><w:t>a </w:t></w:r>");
    const qint64 before = jp::StoryLayout::rangesScannedForTest();
    auto doc = open(makeDocx("<w:p>" + runs + "</w:p>", kStyles));
    QVERIFY(doc);
    QVERIFY(doc->pager().pageCount() >= 2);
    QVERIFY(doc->pager().materialize(0));
    const qint64 scanned = jp::StoryLayout::rangesScannedForTest() - before;
    QVERIFY2(scanned <= 8000 * 60, qPrintable(QStringLiteral("%1 ranges read for a paragraph of 8000").arg(scanned)));
}


// The copy of a page far down a paragraph of a hundred thousand runs does not pass the runs before the page, nor the runs of the paragraph for each line of the page: it takes a moment.
// (A ceiling many times what the page takes alone.)
void WriteTests::aPageFarDownAParagraphOfAHundredThousandRunsIsCopiedQuickly()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>";
    QByteArray runs;
    for (int i = 0; i < 100000; ++i) runs += QByteArray(i % 2 ? "<w:r><w:rPr><w:b/><w:sz w:val=\"16\"/></w:rPr><w:t>b</w:t></w:r>" : "<w:r><w:rPr><w:sz w:val=\"16\"/></w:rPr><w:t>a</w:t></w:r>");
    auto doc = open(makeDocx("<w:p>" + runs + "</w:p>", kStyles, QByteArray(), sect));
    QVERIFY(doc);
    const int pages = doc->pager().pageCount();
    QVERIFY2(pages >= 8, qPrintable(QString::number(pages)));
    const qint64 best = bestNanoseconds([&] { doc->pager().materialize(pages - 2); }, 3);
#if defined(__SANITIZE_ADDRESS__)
    const qint64 ceiling = 30000000000LL;   // (a build that checks every access takes many times as long)
#else
    const qint64 ceiling = 1000000000LL;
#endif
    QVERIFY2(best < ceiling, qPrintable(QStringLiteral("%1 ms to copy page %2 of %3").arg(best / 1000000).arg(pages - 1).arg(pages)));
}


// A page break that ends the last line of a run of joined paragraphs, with no room on the page for the empty line after it, goes on at the paragraph after the run: nothing of the run
// (its empty last paragraph, here) is on the next page again.
void WriteTests::aPageBreakEndingAJoinedRunGoesOnAfterTheRun()
{
    const QByteArray deleted = "<w:rPr><w:del w:id=\"7\" w:author=\"a\"/></w:rPr>";
    // (A page that holds one line.)
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"6000\" w:h=\"1840\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>";
    auto linesOfPages = [&](const QByteArray &runEnd) {
        const QByteArray body = "<w:p><w:pPr>" + deleted + "</w:pPr><w:r><w:t>alpha</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r></w:p>" + runEnd + "<w:p><w:r><w:t>After</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body, kStyles, QByteArray(), sect));
        QStringList out;
        if (!doc) return out;
        const QString whole = doc->story()->toPlainText();
        const int pages = doc->pager().pageCount();
        for (int i = 0; i < pages; ++i) {
            QStringList texts;
            for (const auto &l : doc->pager().lines(i)) texts << whole.mid(l.pos, l.end - l.pos).remove(QLatin1Char('\n')).remove(QChar(0x000C));
            out << texts.join(QLatin1Char('|'));
        }
        return out;
    };
    // The run ends with an empty paragraph: the page after the break is the paragraph after it, as when the run is one paragraph.
    const QStringList joined = linesOfPages("<w:p/>");
    QStringList alone;   // (the same as one paragraph that is the break, and the paragraph after it)
    {
        const QByteArray body = "<w:p><w:r><w:t>alpha</w:t></w:r><w:r><w:br w:type=\"page\"/></w:r></w:p><w:p><w:r><w:t>After</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body, kStyles, QByteArray(), sect));
        QVERIFY(doc);
        const QString whole = doc->story()->toPlainText();
        for (int i = 0; i < doc->pager().pageCount(); ++i) {
            QStringList texts;
            for (const auto &l : doc->pager().lines(i)) texts << whole.mid(l.pos, l.end - l.pos).remove(QLatin1Char('\n')).remove(QChar(0x000C));
            alone << texts.join(QLatin1Char('|'));
        }
    }
    QVERIFY2(joined.size() >= 2 && joined.last().contains(QStringLiteral("After")), qPrintable(joined.join(QStringLiteral(" // "))));
    QCOMPARE(joined.size(), alone.size());
}


// What changes a run of joined paragraphs as a whole (the properties of its last paragraph, which the lines of all of them have) lays the pages of it out again from its first paragraph:
// the pages after such an edit are those of a document that had the change from the start.
void WriteTests::anEditOfTheLastParagraphOfAJoinedRunLaysOutAgainFromTheFirst()
{
    const QByteArray deleted = "<w:rPr><w:del w:id=\"7\" w:author=\"a\"/></w:rPr>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"6000\" w:h=\"4800\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>";
    auto words = [](int count, const char *word) {
        QByteArray out;
        for (int i = 0; i < count; ++i) out += QByteArray(word) + QByteArray::number(i % 10) + " ";
        return out;
    };
    auto bodyWith = [&](const QByteArray &lastProps) {
        return "<w:p><w:pPr>" + deleted + "</w:pPr><w:r><w:t xml:space=\"preserve\">" + words(300, "alpha") + "</w:t></w:r></w:p>"
               "<w:p><w:pPr>" + deleted + "</w:pPr><w:r><w:t xml:space=\"preserve\">" + words(60, "beta") + "</w:t></w:r></w:p>"
               "<w:p><w:pPr>" + lastProps + "</w:pPr><w:r><w:t xml:space=\"preserve\">" + words(80, "gamma") + "</w:t></w:r></w:p><w:p><w:r><w:t>After</w:t></w:r></w:p>";
    };
    auto edited = open(makeDocx(bodyWith(QByteArray()), kStyles, QByteArray(), sect));
    auto fresh = open(makeDocx(bodyWith("<w:ind w:left=\"1200\"/>"), kStyles, QByteArray(), sect));
    QVERIFY(edited && fresh);
    QVERIFY(edited->pager().pageCount() >= 4);
    QTextCursor c(edited->story());
    c.setPosition(edited->story()->findBlockByNumber(2).position() + 1);
    QTextBlockFormat bf = c.blockFormat();
    bf.setLeftMargin(60);
    c.setBlockFormat(bf);
    const int pages = edited->pager().pageCount();
    QCOMPARE(pages, fresh->pager().pageCount());
    for (int i = 0; i < pages; ++i) {
        const auto a = edited->pager().lines(i), b = fresh->pager().lines(i);
        QVERIFY2(a.size() == b.size(), qPrintable(QStringLiteral("page %1 has %2 lines, the new document's %3").arg(i + 1).arg(a.size()).arg(b.size())));
        for (int k = 0; k < a.size(); ++k) QVERIFY2(a[k].pos == b[k].pos && a[k].end == b[k].end && a[k].rect == b[k].rect, qPrintable(QStringLiteral("page %1 line %2 differs").arg(i + 1).arg(k)));
    }
}


// The bullet or number of a run of joined paragraphs is set from the properties of the run's last, as the lines are: it is where it is for that paragraph alone.
void WriteTests::theNumberOfAJoinedRunIsSetFromItsLastParagraph()
{
    const QByteArray deleted = "<w:rPr><w:del w:id=\"7\" w:author=\"a\"/></w:rPr>";
    const QByteArray level = "<w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/><w:pPr><w:ind w:left=\"720\" w:hanging=\"360\"/></w:pPr></w:lvl>";
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\">" + level + "</w:abstractNum><w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>";
    const QByteArray item = "<w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr>";
    const QByteArray own = "<w:ind w:left=\"2160\" w:hanging=\"720\"/>";
    auto markerOf = [&](const QByteArray &body) {
        auto doc = open(makeDocx(body, kStyles, numbering));
        if (!doc) return -1000.0;
        FrameSpec fs;
        fs.size = QSizeF(400, 10000);
        fs.insets = QMarginsF(0, 0, 0, 0);
        fs.hyphenate = false;
        StoryLayout layout;
        layout.build(doc->story(), {fs}, doc->flow().env);
        for (const auto &l : layout.lineInfo(0))
            if (!l.marker.isEmpty()) return l.markerX;
        return -1001.0;
    };
    const double joined = markerOf("<w:p><w:pPr>" + item + deleted + "</w:pPr><w:r><w:t>first </w:t></w:r></w:p><w:p><w:pPr>" + item + own + "</w:pPr><w:r><w:t>second</w:t></w:r></w:p>");
    const double alone = markerOf("<w:p><w:pPr>" + item + own + "</w:pPr><w:r><w:t>first second</w:t></w:r></w:p>");
    QVERIFY2(joined > -1000 && alone > -1000, qPrintable(QStringLiteral("%1 and %2").arg(joined).arg(alone)));
    QVERIFY2(std::abs(joined - alone) < 0.5, qPrintable(QStringLiteral("the number is at %1 where the last paragraph alone has it at %2").arg(joined).arg(alone)));
}


// Backspace and Delete at the junction of a run of joined paragraphs, at the end of the first and at the start of the next, keep every character of the text and leave a layout that
// shows it: nothing is lost, and nothing is laid out twice.
void WriteTests::backspaceAndDeleteAtTheJunctionOfAJoinedRunKeepEveryCharacter()
{
    const QByteArray deleted = "<w:rPr><w:del w:id=\"7\" w:author=\"a\"/></w:rPr>";
    const QByteArray body = "<w:p><w:pPr>" + deleted + "</w:pPr><w:r><w:t>alpha</w:t></w:r></w:p><w:p><w:pPr>" + deleted + "</w:pPr><w:r><w:t>beta</w:t></w:r></w:p><w:p><w:r><w:t>gamma</w:t></w:r></w:p>";
    auto letters = [](WriteDoc &doc) { return doc.story()->toPlainText().remove(QLatin1Char('\n')); };
    for (int variant = 0; variant < 4; ++variant) {
        auto doc = open(makeDocx(body, kStyles));
        QVERIFY(doc);
        QTextCursor c(doc->story());
        const int junction = doc->story()->findBlockByNumber(variant < 2 ? 1 : 2).position();
        c.setPosition(variant % 2 ? junction - 1 : junction);
        if (variant % 2) deleteForward(*doc, c, false);
        else deleteBack(*doc, c, false);
        QCOMPARE(letters(*doc), QStringLiteral("alphabetagamma"));
        const QString whole = doc->story()->toPlainText();
        QString shown;
        int last = 0;
        for (int i = 0; i < doc->pager().pageCount(); ++i)
            for (const auto &l : doc->pager().lines(i)) {
                QVERIFY2(l.pos >= last, qPrintable(QStringLiteral("variant %1: a line at %2 begins before the one before it ends, at %3").arg(variant).arg(l.pos).arg(last)));
                shown += whole.mid(l.pos, l.end - l.pos);
                last = l.end;
            }
        QCOMPARE(shown.remove(QLatin1Char('\n')), QStringLiteral("alphabetagamma"));
        const QByteArray x = documentXml(doc->toDocx());
        QVERIFY2(x.contains("alpha") && x.contains("beta") && x.contains("gamma"), x.constData());
    }
}


// A deleted mark never joins the paragraphs of two table cells, nor a cell's last paragraph and the one after the table: the text of each cell is in its own cell, once.
void WriteTests::aDeletedMarkNeverJoinsTheParagraphsOfTwoCells()
{
    const QByteArray deleted = "<w:rPr><w:del w:id=\"7\" w:author=\"a\"/></w:rPr>";
    auto cell = [&](const char *text, bool markDeleted) {
        return "<w:tc><w:tcPr><w:tcW w:w=\"3000\" w:type=\"dxa\"/></w:tcPr><w:p><w:pPr>" + QByteArray(markDeleted ? deleted : QByteArray()) + "</w:pPr><w:r><w:t>" + text + "</w:t></w:r></w:p></w:tc>";
    };
    const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"6000\" w:type=\"dxa\"/><w:tblLayout w:type=\"fixed\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"3000\"/><w:gridCol w:w=\"3000\"/></w:tblGrid>"
                             "<w:tr>" + cell("CELLONE", true) + cell("CELLTWO", true) + "</w:tr></w:tbl>";
    auto doc = open(makeDocx("<w:p><w:r><w:t>Before</w:t></w:r></w:p>" + table + "<w:p><w:r><w:t>After</w:t></w:r></w:p>", kStyles));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    QCOMPARE(page->tables.size(), 1);
    const QString whole = doc->story()->toPlainText();
    QStringList texts;
    page->tables.first()->forLines([&](const StoryLayout::LineBrief &line, const QRectF &) { texts << whole.mid(line.pos, line.end - line.pos).remove(QLatin1Char('\n')); });
    texts.sort();
    QCOMPARE(texts, (QStringList{QStringLiteral("CELLONE"), QStringLiteral("CELLTWO")}));
    // The paragraph after the table is its own line, and the one before.
    QStringList outside;
    for (const auto &l : doc->pager().lines(0)) outside << whole.mid(l.pos, l.end - l.pos).remove(QLatin1Char('\n'));
    QVERIFY2(outside.contains(QStringLiteral("Before")) && outside.contains(QStringLiteral("After")), qPrintable(outside.join(QLatin1Char('|'))));
}


// What a paste from another document brings that points into the first's notes and objects points at nothing of the second's: a reference to a note is
// left out (its text isn't there), and an object's place on the page isn't carried (the object isn't there); the rest of what was copied is.
void WriteTests::aPasteFromAnotherDocumentLeavesNoDanglingNoteOrObject()
{
    const QByteArray ns = "xmlns:v=\"urn:schemas-microsoft-com:vml\"";
    const QByteArray body = "<w:p " + ns + "><w:r><w:t xml:space=\"preserve\">Cited</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r><w:r><w:pict><v:rect id=\"r\" style=\"position:absolute;margin-left:10pt;margin-top:5pt;width:50pt;height:20pt\" fillcolor=\"red\"/></w:pict></w:r><w:r><w:t xml:space=\"preserve\"> end.</w:t></w:r></w:p>";
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t>The note.</w:t></w:r></w:p></w:footnote></w:footnotes>";
    auto source = open(makeDocxWithParts(body, QByteArray(), {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(source);
    auto has = [](const QTextDocument *d, auto test) {
        for (QTextBlock b = d->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it)
                if (test(it.fragment().charFormat())) return true;
        return false;
    };
    QVERIFY(has(source->story(), [](const QTextCharFormat &f) { return f.stringProperty(tp::Field).startsWith(QLatin1String("footnote:")); }));
    QVERIFY(has(source->story(), [](const QTextCharFormat &f) { return f.hasProperty(flow::fp::AnchorId); }));
    QTextCursor sel(source->story());
    sel.select(QTextCursor::Document);
    const Clip clip = copySelection(*source, sel);
    auto target = open(makeDocx("<w:p><w:r><w:t>Target.</w:t></w:r></w:p>", kStyles));
    QVERIFY(target);
    QTextCursor t(target->story());
    t.movePosition(QTextCursor::End);
    pasteClip(*target, t, clip);
    QVERIFY2(!has(target->story(), [](const QTextCharFormat &f) { return f.stringProperty(tp::Field).startsWith(QLatin1String("footnote:")); }), "a reference to a note of the other document");
    QVERIFY2(!has(target->story(), [](const QTextCharFormat &f) { return f.hasProperty(flow::fp::AnchorId); }), "an object's place from the other document");
    const QByteArray saved = target->toDocx();
    office::Package p;
    QVERIFY(p.read(saved));
    const QByteArray x = p.part(p.mainPart());
    QVERIFY2(!x.contains("footnoteReference"), x.constData());
    QVERIFY(x.contains("Cited") && x.contains(" end."));
    QVERIFY2(rt::danglingReferences(saved).isEmpty(), qPrintable(rt::danglingReferences(saved).join(QLatin1Char(' '))));
}


// A symbol of a font the machine lacks is set in a stand-in on the page only: what is saved of a file nobody touched has the run as it was.
void WriteTests::aSymbolOfAMissingFontIsSavedAsItWas()
{
    const QByteArray body = "<w:p><w:r><w:t xml:space=\"preserve\">Before </w:t></w:r><w:r><w:sym w:font=\"No Such Face Anywhere\" w:char=\"0047\"/></w:r><w:r><w:t xml:space=\"preserve\"> after</w:t></w:r></w:p>";
    // (A file whose styles say what font the text is in, as files do.)
    const QByteArray styles = "<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii=\"Times New Roman\" w:hAnsi=\"Times New Roman\"/><w:sz w:val=\"24\"/></w:rPr></w:rPrDefault></w:docDefaults>"
                              "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>";
    const QByteArray original = makeDocx(body, styles);
    auto doc = open(original);
    QVERIFY(doc);
    const QByteArray saved = doc->toDocx();
    const QStringList loss = rt::lossProblems(original, saved);
    QVERIFY2(loss.isEmpty(), qPrintable(loss.join(QLatin1String("; "))));
    office::Package pkg;
    QVERIFY(pkg.read(saved));
    const QString main = QString::fromUtf8(pkg.part(pkg.mainPart()));
    QVERIFY2(!main.contains(QLatin1String("rFonts")), qPrintable(main.mid(main.indexOf(QLatin1String("<w:body")), 600)));
}


// A symbol (w:sym) of a symbol font is set in that font on the page, whatever font the run is in: the width of its picture is that font's, the run's own font being
// kept as the file has it (what the page sets it in is not written).
void WriteTests::aSymbolOfASymbolFontIsSetInThatFont()
{
    auto family = [&](const char *font, QString *saved = nullptr) {
        const QByteArray body = "<w:p><w:r><w:rPr><w:rFonts w:ascii=\"Times New Roman\" w:hAnsi=\"Times New Roman\"/></w:rPr><w:t xml:space=\"preserve\">Before </w:t></w:r><w:r><w:rPr><w:rFonts w:ascii=\"Times New Roman\" w:hAnsi=\"Times New Roman\"/></w:rPr>"
                                "<w:sym w:font=\"" + QByteArray(font) + "\" w:char=\"F0FC\"/></w:r></w:p>";
        auto doc = open(makeDocx(body, kStyles));
        if (!doc) return QString::fromLatin1("no document");
        if (saved) *saved = QString::fromUtf8(documentXml(doc->toDocx()));
        for (QTextBlock::iterator it = doc->story()->begin().begin(); !it.atEnd(); ++it)
            if (it.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("raw:"))) return it.fragment().charFormat().stringProperty(tp::DisplayFamily);
        return QString::fromLatin1("no symbol");
    };
    QString saved;
    QCOMPARE(family("Wingdings", &saved), QStringLiteral("Wingdings"));
    QVERIFY2(saved.contains(QLatin1String("w:sym")) && !saved.contains(QLatin1String("DisplayFamily")), qPrintable(saved));
    QCOMPARE(family("Symbol"), QStringLiteral("Symbol"));
    QCOMPARE(family("Webdings"), QStringLiteral("Webdings"));
}


// The picture a symbol of a symbol font stands for is drawn, with room of its own before the text that follows (the font's width for it, not none): a file's diamond bullet.
void WriteTests::aSymbolOfASymbolFontIsDrawnWithRoomOfItsOwn()
{
    const auto with = inkAlongFirstLine("<w:r><w:sym w:font=\"Symbol\" w:char=\"F0A8\"/></w:r>"), without = inkAlongFirstLine(QByteArray());
    QVERIFY(with.size() >= 2 && without.size() >= 1);
    // The symbol's picture is drawn (a stretch of ink of its own, at the margin), and the text begins after it, further in than it does with no symbol.
    QVERIFY2(with[0].first < 76 && with[0].second - with[0].first >= 2, qPrintable(QStringLiteral("the first ink is %1 to %2 points").arg(with[0].first).arg(with[0].second)));
    QVERIFY2(with[1].first > without[0].first + 3, qPrintable(QStringLiteral("the text begins at %1 points with the symbol, at %2 without").arg(with[1].first).arg(without[0].first)));
}


// A symbol takes as much of the line as its own font makes its picture (a round bullet of Wingdings, a diamond of Symbol, set in 11.5 points): the text after it begins
// that far on.
void WriteTests::aSymbolTakesItsAdvanceFromItsFont()
{
    struct Case { const char *font, *code; char16_t picture; };
    for (const Case &c : {Case{"Wingdings", "F06C", 0x25CF}, Case{"Symbol", "F0A8", 0x2666}}) {
        const QString font = QString::fromLatin1(c.font);
        if (QFontDatabase::hasFamily(font)) continue;   // (installed: its own glyphs are drawn)
        const QHash<char16_t, double> *widths = symbolFontLetterWidths(font);
        QVERIFY2(widths && widths->contains(c.picture), c.font);
        const auto with = inkAlongFirstLine("<w:r><w:sym w:font=\"" + QByteArray(c.font) + "\" w:char=\"" + QByteArray(c.code) + "\"/></w:r>"), without = inkAlongFirstLine(QByteArray());
        QVERIFY2(with.size() >= 2 && without.size() >= 1, c.font);
        const double advance = with[1].first - without[0].first, wanted = widths->value(c.picture) * 11.5;
        QVERIFY2(std::abs(advance - wanted) < 0.8, qPrintable(QStringLiteral("%1: the symbol takes %2 points, the font's picture is %3").arg(font).arg(advance).arg(wanted)));
    }
}


// The result of a field that is nothing but blanks (a reference to a form field with no text in it) is shown as the file has it, but takes no part in its line's height, whatever size
// its run is in; blanks that are no field's result count as they did.
void WriteTests::aFieldResultOfBlanksTakesNoPartInTheLineHeight()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray none = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr>";
    auto props = [](int halfPoints) { return "<w:rPr><w:rFonts w:ascii=\"Arial\" w:hAnsi=\"Arial\"/><w:sz w:val=\"" + QByteArray::number(halfPoints) + "\"/></w:rPr>"; };
    // The height of the line of a small text with four blanks in it, as a field's result in a run of `blanks` half points, or as plain text.
    auto lineHeight = [&](int blanks, bool inField) {
        QByteArray body = "<w:p>" + none + "<w:r>" + props(14) + "<w:t xml:space=\"preserve\">at </w:t></w:r>";
        if (inField)
            body += "<w:r>" + props(14) + "<w:fldChar w:fldCharType=\"begin\"/></w:r><w:r>" + props(14) + "<w:instrText xml:space=\"preserve\"> REF dropdown23 \\* MERGEFORMAT </w:instrText></w:r><w:r>" +
                    props(14) + "<w:fldChar w:fldCharType=\"separate\"/></w:r><w:r>" + props(blanks) + "<w:t xml:space=\"preserve\">    </w:t></w:r><w:r>" + props(14) + "<w:fldChar w:fldCharType=\"end\"/></w:r>";
        else
            body += "<w:r>" + props(blanks) + "<w:t xml:space=\"preserve\">    </w:t></w:r>";
        body += "<w:r>" + props(14) + "<w:t xml:space=\"preserve\"> Plant Location</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body, kStyles, QByteArray(), sect));
        if (!doc) return -1.0;
        const auto lines = doc->pager().lines(0);
        return lines.isEmpty() ? -1.0 : lines.first().rect.height();
    };
    const double small = lineHeight(14, true), big = lineHeight(40, true);
    QVERIFY(small > 0 && big > 0);
    QVERIFY2(std::abs(big - small) < 0.01, qPrintable(QStringLiteral("the line is %1 points with the blanks of a field in 20 point type, %2 in 7 point").arg(big).arg(small)));
    // Not in a field, the same blanks make the line as tall as their size does.
    QVERIFY(lineHeight(40, false) > small + 5);
}


// Blanks typed right after the blanks that are a reference's result are the typist's own: they count in the line's height as blanks typed anywhere do. (They took the result's mark of
// taking no part in it, and a line of spaces typed there stayed as short as the field's.) The saved file has them as plain text, and the mark is no part of it.
void WriteTests::blanksTypedAfterABlankFieldResultCountInTheLineHeight()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray none = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr>";
    auto props = [](int halfPoints) { return "<w:rPr><w:rFonts w:ascii=\"Arial\" w:hAnsi=\"Arial\"/><w:sz w:val=\"" + QByteArray::number(halfPoints) + "\"/></w:rPr>"; };
    const QByteArray body = "<w:p>" + none + "<w:r>" + props(14) + "<w:t xml:space=\"preserve\">at </w:t></w:r><w:r>" + props(14) + "<w:fldChar w:fldCharType=\"begin\"/></w:r><w:r>" + props(14) +
                            "<w:instrText xml:space=\"preserve\"> REF dropdown23 \\* MERGEFORMAT </w:instrText></w:r><w:r>" + props(14) + "<w:fldChar w:fldCharType=\"separate\"/></w:r><w:r>" +
                            props(40) + "<w:t xml:space=\"preserve\">    </w:t></w:r><w:r>" + props(14) + "<w:fldChar w:fldCharType=\"end\"/></w:r><w:r>" + props(14) +
                            "<w:t xml:space=\"preserve\"> Plant Location</w:t></w:r></w:p>";
    auto height = [](WriteDoc &doc) {
        const auto lines = doc.pager().lines(0);
        return lines.isEmpty() ? -1.0 : lines.first().rect.height();
    };
    auto doc = open(makeDocx(body, kStyles, QByteArray(), sect));
    QVERIFY(doc);
    const double small = height(*doc);
    QVERIFY(small > 0);
    QTextCursor c(doc->story());
    c.setPosition(int(doc->story()->toPlainText().indexOf(QStringLiteral(" Plant"))));
    typeText(*doc, c, QStringLiteral("    "));
    const double typed = height(*doc);
    QVERIFY2(typed > small + 5, qPrintable(QStringLiteral("the line is %1 points with four blanks typed after the field's, %2 without").arg(typed).arg(small)));
    // The file has the blanks as text, and when it is opened again they are as tall.
    const QByteArray saved = doc->toDocx();
    auto again = open(saved);
    QVERIFY(again);
    QVERIFY2(std::abs(height(*again) - typed) < 0.01, qPrintable(QStringLiteral("%1 points when opened again, %2 before").arg(height(*again)).arg(typed)));
}


// A style that has no styleId is the one its name names, as a paragraph that asks for it by that name finds it (a file whose style sheet was written by a program that left the id out:
// the reference sets such a paragraph in the 1 point type the style gives it).
void WriteTests::aStyleWithNoIdIsFoundByItsName()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray styles = kStyles + "<w:style><w:name w:val=\"TinyCellLayout\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:sz w:val=\"2\"/></w:rPr></w:style>";
    auto lineHeight = [&](const QByteArray &style) {
        const QByteArray body = "<w:p><w:pPr>" + style + "<w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr></w:p>";
        auto doc = open(makeDocx(body, styles, QByteArray(), sect));
        if (!doc) return -1.0;
        const auto lines = doc->pager().lines(0);
        return lines.isEmpty() ? -1.0 : lines.first().rect.height();
    };
    const double tiny = lineHeight("<w:pStyle w:val=\"TinyCellLayout\"/>"), plain = lineHeight(QByteArray());
    QVERIFY(tiny > 0 && plain > 0);
    QVERIFY2(tiny < plain / 3, qPrintable(QStringLiteral("the line is %1 points in the style the name names, %2 in no style").arg(tiny).arg(plain)));
    // Saved, the style is as the file had it, with no id written for it.
    auto doc = open(makeDocx("<w:p><w:pPr><w:pStyle w:val=\"TinyCellLayout\"/></w:pPr></w:p>", styles, QByteArray(), sect));
    QVERIFY(doc);
    office::Package saved;
    QVERIFY(saved.read(doc->toDocx()));
    const QByteArray out = saved.part(QStringLiteral("/word/styles.xml"));
    QVERIFY(out.contains("<w:style><w:name w:val=\"TinyCellLayout\"/>"));
    QVERIFY(!out.contains("w:styleId=\"TinyCellLayout\""));
}


// A symbol sets the line as its own font does: a line with a big diamond of Symbol is as tall as Symbol makes it, not as the run's font does.
void WriteTests::aSymbolSetsTheLineOfItsFont()
{
    // The point from the page's top where the second of two paragraphs, each of one line, has its first ink.
    auto secondBegins = [&](const QByteArray &start) {
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        const QByteArray none = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr>";
        const QByteArray body = "<w:p>" + none + start + "<w:r><w:t>First</w:t></w:r></w:p><w:p>" + none + "<w:r><w:t>Second</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body, kStyles, QByteArray(), sect));
        if (!doc) return -1.0;
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        const QImage img = view.pageImage(0, 816);
        const double perPoint = 816 / 612.0;
        bool inFirst = false;
        int gapRows = 0;
        for (int y = int(70 * perPoint); y < int(200 * perPoint); ++y) {
            bool ink = false;
            for (int x = int(60 * perPoint); x < int(200 * perPoint) && !ink; ++x) ink = qGray(img.pixel(x, y)) < 160;
            if (ink && gapRows > 0 && inFirst) return y / perPoint;
            if (ink) { inFirst = true; gapRows = 0; } else if (inFirst) ++gapRows;
        }
        return -1.0;
    };
    const double with = secondBegins("<w:r><w:rPr><w:sz w:val=\"96\"/></w:rPr><w:sym w:font=\"Symbol\" w:char=\"F0A8\"/></w:r>"), plain = secondBegins("<w:r><w:rPr><w:sz w:val=\"96\"/></w:rPr><w:t>x</w:t></w:r>");
    QVERIFY(with > 0 && plain > 0);
    QVERIFY2(with > plain + 1.5, qPrintable(QStringLiteral("the second paragraph begins at %1 points after a symbol, at %2 after a letter").arg(with).arg(plain)));
}


// The entries of a table of contents are in the color of their text: the link style that a file puts on their runs shows in a link of the text, not in an entry of the
// table (where the page's own style, toc 1 and the like, is what shows).
void WriteTests::aLinkInATableOfContentsEntryIsNotMarkedAsALink()
{
    const QByteArray styles = kStyles +
        "<w:style w:type=\"paragraph\" w:styleId=\"TOC1\"><w:name w:val=\"toc 1\"/><w:basedOn w:val=\"Normal\"/></w:style>"
        "<w:style w:type=\"character\" w:styleId=\"Hyperlink\"><w:name w:val=\"Hyperlink\"/><w:rPr><w:color w:val=\"0000FF\"/><w:u w:val=\"single\"/></w:rPr></w:style>";
    auto marked = [&](const char *paragraphStyle) {
        const QByteArray body = "<w:p><w:pPr><w:pStyle w:val=\"" + QByteArray(paragraphStyle) + "\"/></w:pPr><w:hyperlink w:anchor=\"x\"><w:r><w:rPr><w:rStyle w:val=\"Hyperlink\"/></w:rPr><w:t>Entry</w:t></w:r></w:hyperlink></w:p>";
        auto doc = open(makeDocx(body, styles));
        if (!doc) return QStringLiteral("no document");
        for (QTextBlock::iterator it = doc->story()->begin().begin(); !it.atEnd(); ++it)
            if (it.fragment().text().startsWith(QLatin1String("Entry"))) {
                const QTextCharFormat f = it.fragment().charFormat();
                return QStringLiteral("%1 %2").arg(f.foreground().color().name()).arg(f.fontUnderline() ? QStringLiteral("underlined") : QStringLiteral("plain"));
            }
        return QStringLiteral("no entry");
    };
    QCOMPARE(marked("Normal"), QStringLiteral("#0000ff underlined"));   // (a link in the text is marked)
    const QString entry = marked("TOC1");
    QVERIFY2(!entry.contains(QLatin1String("underlined")) && !entry.contains(QLatin1String("#0000ff")), qPrintable(entry));
}


// The page number's own element (w:pgNum, in a run) is the page's number, in a paragraph that has other fields as well as in one that has none.
void WriteTests::aPageNumberRunIsTheNumberOfThePage()
{
    auto pageFields = [&](const QByteArray &paragraph) {
        auto doc = open(makeDocx("<w:p>" + paragraph + "</w:p>"));
        int n = 0;
        if (doc)
            for (QTextBlock::iterator it = doc->story()->begin().begin(); !it.atEnd(); ++it)
                n += it.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("page"));
        return n;
    };
    QCOMPARE(pageFields("<w:r><w:t xml:space=\"preserve\">Page </w:t></w:r><w:r><w:pgNum/></w:r>"), 1);
    QCOMPARE(pageFields("<w:r><w:t xml:space=\"preserve\">Page </w:t></w:r><w:r><w:pgNum/></w:r><w:r><w:t xml:space=\"preserve\"> of </w:t></w:r>"
                        "<w:fldSimple w:instr=\" NUMPAGES \"><w:r><w:t>25</w:t></w:r></w:fldSimple>"), 1);
}


// The round-trip check itself: a run that gains a property the file never had is a difference, though what it adds is one the program understands.
void WriteTests::theRoundTripCheckSeesAGainedRunProperty()
{
    auto withRun = [&](const QByteArray &props) { return makeDocx("<w:p><w:r>" + props + "<w:t>text</w:t></w:r></w:p>"); };
    const QByteArray plain = withRun(QByteArray()), gained = withRun("<w:rPr><w:rFonts w:ascii=\"Foo\"/></w:rPr>");
    QVERIFY(rt::lossProblems(plain, plain).isEmpty());
    QVERIFY(rt::lossProblems(gained, plain).isEmpty());   // (one left out is normalizing)
    const QStringList added = rt::lossProblems(plain, gained);
    QVERIFY2(!added.isEmpty() && added.first().contains(QLatin1String("gains run properties")), qPrintable(added.join(QLatin1String("; "))));
}


// The round-trip check reads a property as a reader does: an on/off property said "false", "0", or "off" is off, and one said "true", "1", "on", or with no value is on,
// whichever the file spelled; hidden text that is not hidden is nothing; an underline of none that names a color, and a font said for the face alone, say what the
// underline and the fonts said. What the file never said is still a gain, and another font is another font.
void WriteTests::theRoundTripCheckReadsPropertiesAsAReaderDoes()
{
    auto withRun = [&](const QByteArray &props) { return makeDocx("<w:p><w:r><w:rPr>" + props + "</w:rPr><w:t>text</w:t></w:r></w:p>"); };
    auto same = [&](const QByteArray &was, const QByteArray &now) { return rt::lossProblems(withRun(was), withRun(now)).isEmpty(); };
    QVERIFY(same("<w:b w:val=\"false\"/>", "<w:b w:val=\"0\"/>"));
    QVERIFY(same("<w:caps w:val=\"off\"/><w:smallCaps w:val=\"0\"/>", "<w:caps w:val=\"false\"/><w:smallCaps w:val=\"false\"/>"));
    QVERIFY(same("<w:i w:val=\"true\"/><w:strike w:val=\"on\"/>", "<w:i/><w:strike w:val=\"1\"/>"));
    QVERIFY(same("<w:vanish w:val=\"false\"/>", QByteArray()));
    QVERIFY(same("<w:u w:val=\"none\" w:color=\"112233\"/>", "<w:u w:val=\"none\"/>"));
    QVERIFY(same("<w:rFonts w:ascii=\"Face\" w:hAnsi=\"Face\" w:cs=\"Face\"/>", "<w:rFonts w:cs=\"Face\"/>"));
    QVERIFY(!same(QByteArray(), "<w:b w:val=\"0\"/>"));
    QVERIFY(!same("<w:b/>", "<w:b w:val=\"0\"/>"));
    QVERIFY(!same("<w:rFonts w:ascii=\"Face\" w:cs=\"Face\"/>", "<w:rFonts w:cs=\"Other\"/>"));
    QVERIFY(!same("<w:u w:val=\"none\"/>", "<w:u w:val=\"single\"/>"));
    QVERIFY(!same("<w:vanish/>", QByteArray()));
    QVERIFY(!same(QByteArray(), "<w:vanish/>"));
}


// Text holds one kind of capitals and one kind of strike-through: a run in capitals over a style that says small capitals is in capitals, and one in a double strike
// over a style's single strike is double. What the run says of the other kind is not "off" for the format has no room for it: it is not given an off that the file never
// said. A person who turns the capitals off over a style of small capitals still saves the look they see (no small capitals).
void WriteTests::capitalsAndDoubleStrikeOverTheStylesOtherKindAreSavedAsTheFileSaidThem()
{
    const QByteArray styles = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                              "<w:style w:type=\"paragraph\" w:styleId=\"Small\"><w:name w:val=\"Small\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:smallCaps/><w:strike/></w:rPr></w:style>";
    const QByteArray original = makeDocx("<w:p><w:pPr><w:pStyle w:val=\"Small\"/></w:pPr><w:r><w:rPr><w:caps/><w:dstrike/></w:rPr><w:t>Both</w:t></w:r></w:p>", styles);
    auto doc = open(original);
    QVERIFY(doc);
    const QByteArray saved = doc->toDocx();
    const QByteArray x = documentXml(saved);
    QVERIFY2(x.contains("<w:caps/>") && x.contains("<w:dstrike/>"), x.constData());
    QVERIFY2(!x.contains("smallCaps") && !x.contains("<w:strike"), x.constData());
    const QStringList loss = rt::lossProblems(original, saved);
    QVERIFY2(loss.isEmpty(), qPrintable(loss.join(QLatin1String("; "))));
    // (The person turns the capitals off: the text is not in small capitals on the screen, so the file says so.)
    QTextCursor c(doc->story());
    c.setPosition(0);
    c.setPosition(4, QTextCursor::KeepAnchor);
    changeCharFormat(*doc, c, [](QTextCharFormat &f) { f.setFontCapitalization(QFont::MixedCase); });
    const QByteArray after = documentXml(doc->toDocx());
    QVERIFY2(after.contains("<w:smallCaps w:val=\"0\"/>") && !after.contains("<w:caps"), after.constData());
}


// Bold in a character style over a paragraph style that is bold turns it off, as it does in the original program; a run that says bold itself over both is bold, and is saved
// saying so: the file's own bold is not left out as if it were the style's.
void WriteTests::aToggleOfACharacterStyleFlipsTheParagraphStylesAndADirectOneStaysSaid()
{
    const QByteArray styles = "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\"><w:name w:val=\"Normal\"/></w:style>"
                              "<w:style w:type=\"paragraph\" w:styleId=\"Head\"><w:name w:val=\"Head\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:b/><w:i/></w:rPr></w:style>"
                              "<w:style w:type=\"character\" w:styleId=\"Strong\"><w:name w:val=\"Strong\"/><w:rPr><w:b/></w:rPr></w:style>";
    const QByteArray original = makeDocx("<w:p><w:pPr><w:pStyle w:val=\"Head\"/></w:pPr><w:r><w:rPr><w:rStyle w:val=\"Strong\"/></w:rPr><w:t>flipped</w:t></w:r>"
                                         "<w:r><w:rPr><w:rStyle w:val=\"Strong\"/><w:b/></w:rPr><w:t>said</w:t></w:r></w:p>",
                                         styles);
    auto looks = [](WriteDoc &doc) {
        QTextCursor c(doc.story());
        c.setPosition(3);
        const QTextCharFormat first = c.charFormat();
        c.setPosition(doc.story()->toPlainText().indexOf(QLatin1String("said")) + 2);
        return QVector<QTextCharFormat>{first, c.charFormat()};
    };
    auto doc = open(original);
    QVERIFY(doc);
    QVector<QTextCharFormat> f = looks(*doc);
    QVERIFY2(f[0].fontWeight() < QFont::DemiBold && f[0].fontItalic(), "the character style's bold flips the heading's");
    QVERIFY2(f[1].fontWeight() >= QFont::DemiBold && f[1].fontItalic(), "bold said by the run itself");
    const QByteArray saved = doc->toDocx();
    const QByteArray x = documentXml(saved);
    QVERIFY2(x.contains("<w:rStyle w:val=\"Strong\"/><w:b/></w:rPr><w:t xml:space=\"preserve\">said"), x.constData());
    QVERIFY2(x.contains("<w:rPr><w:rStyle w:val=\"Strong\"/></w:rPr><w:t xml:space=\"preserve\">flipped"), x.constData());
    const QStringList loss = rt::lossProblems(original, saved);
    QVERIFY2(loss.isEmpty(), qPrintable(loss.join(QLatin1String("; "))));
    auto back = open(saved);
    QVERIFY(back);
    f = looks(*back);
    QVERIFY(f[0].fontWeight() < QFont::DemiBold && f[1].fontWeight() >= QFont::DemiBold);
}


// A file that flags none of its paragraph styles as the default has paragraphs that say "Normal": they are saved saying it (the program takes Normal for the default of such
// a file, and another reader may not, so leaving it out could change the paragraph's look). A file that flags Normal as its default has it left out of the paragraphs, as before.
void WriteTests::aParagraphStyleIsSavedSaidWhenTheFileFlagsNoDefault()
{
    auto saved = [&](const QByteArray &normalAttrs) {
        const QByteArray styles = "<w:style w:type=\"paragraph\"" + normalAttrs + " w:styleId=\"Normal\"><w:name w:val=\"Normal\"/><w:rPr><w:b/></w:rPr></w:style>";
        const QByteArray original = makeDocx("<w:p><w:pPr><w:pStyle w:val=\"Normal\"/></w:pPr><w:r><w:t>said</w:t></w:r></w:p>", styles);
        auto doc = open(original);
        return doc ? documentXml(doc->toDocx()) : QByteArray();
    };
    const QByteArray unflagged = saved(QByteArray());
    QVERIFY2(unflagged.contains("<w:pStyle w:val=\"Normal\"/>"), unflagged.constData());
    const QByteArray flagged = saved(" w:default=\"1\"");
    QVERIFY2(!flagged.contains("<w:pStyle"), flagged.constData());
    // (A file with no styles has none to say.)
    auto bare = open(makeDocx("<w:p><w:r><w:t>none</w:t></w:r></w:p>"));
    QVERIFY(bare);
    QVERIFY(!documentXml(bare->toDocx()).contains("<w:pStyle"));
}


// An empty paragraph that ends a section, when the next section is continuous, takes no room; before a section on a page of its own it does.
void WriteTests::anEmptyBreakParagraphBeforeAContinuousSectionTakesNoRoom()
{
    auto linesWith = [&](const QByteArray &type) {
        const QByteArray first = "<w:p><w:r><w:t>First</w:t></w:r></w:p><w:p><w:pPr><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr></w:pPr></w:p>";
        const QByteArray second = "<w:p><w:r><w:t>Second</w:t></w:r></w:p>";
        const QByteArray last = "<w:sectPr>" + type + "<w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(first + second, last, {}));
        return doc ? doc->pager().lines(0) : QVector<StoryLayout::LineBrief>();
    };
    const auto joined = linesWith("<w:type w:val=\"continuous\"/>");
    QVERIFY(joined.size() >= 3);
    // (The second paragraph is where it would be with no break paragraph between.)
    const QByteArray plain = "<w:p><w:r><w:t>First</w:t></w:r></w:p><w:p><w:r><w:t>Second</w:t></w:r></w:p>";
    auto alone = open(makeDocxWithParts(plain, "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>", {}));
    QVERIFY(alone);
    const auto direct = alone->pager().lines(0);
    QVERIFY(direct.size() >= 2);
    QVERIFY2(std::abs(joined[2].rect.top() - direct[1].rect.top()) < 0.01, qPrintable(QString::number(joined[2].rect.top() - direct[1].rect.top())));
    // (Before a section on a page of its own the empty paragraph keeps its line: the section after starts on the next page.)
    const auto apart = linesWith(QByteArray());
    QCOMPARE(int(apart.size()), 2);
}


// The empty paragraph that ends a section keeps its line when it is the first paragraph of the document (nothing is above it for its break to be the end of): the text after it is
// where it is below an empty paragraph of its own, whatever the next section's type.
void WriteTests::anEmptyBreakParagraphThatBeginsTheDocumentKeepsItsLine()
{
    auto tops = [&](const QByteArray &firstParagraph, const QByteArray &type) {
        const QByteArray second = "<w:p><w:r><w:t>Second</w:t></w:r></w:p>";
        const QByteArray last = "<w:sectPr>" + type + "<w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
        auto doc = open(makeDocxWithParts(firstParagraph + second, last, {}));
        const auto lines = doc ? doc->pager().lines(0) : QVector<StoryLayout::LineBrief>();
        return lines.isEmpty() ? -1.0 : lines.last().rect.top();
    };
    const QByteArray breaking = "<w:p><w:pPr><w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr></w:pPr></w:p>";
    const QByteArray ordinary = "<w:p/>";
    const double continuous = tops(breaking, "<w:type w:val=\"continuous\"/>"), below = tops(ordinary, QByteArray());
    QVERIFY(continuous >= 0 && below > 0);
    QVERIFY2(std::abs(continuous - below) < 0.01, qPrintable(QStringLiteral("the text is at %1 after the break paragraph, %2 after an empty paragraph").arg(continuous).arg(below)));
}


// The columns of a continuous section that has more of them than the section before it begin where that section's text ends: the empty paragraph that ends the section before
// takes no room there either (they begin as they do when the section's last paragraph has the break in it).
void WriteTests::theColumnsAfterAnEmptyBreakParagraphBeginWhereTheTextAboveEnds()
{
    const QByteArray one = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray three = "<w:sectPr><w:type w:val=\"continuous\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"720\" w:right=\"720\" w:bottom=\"720\" w:left=\"720\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/><w:cols w:num=\"3\" w:space=\"360\"/></w:sectPr>";
    auto columnsTop = [&](const QByteArray &first) {
        auto doc = open(makeDocxWithParts(first + "<w:p><w:r><w:t>COLUMNTEXT</w:t></w:r></w:p>", three, {}));
        if (!doc) return -1.0;
        const auto page = doc->pager().materialize(0);
        if (!page) return -1.0;
        // (The first column of the three: the page's text boxes are the first section's, then the columns of the second.)
        for (const auto &item : page->doc->pages[0]->items)
            if (item->id.endsWith(QLatin1String("-0-1")) && item->id.contains(QLatin1String("flow-box"))) return item->rect.top();
        return -1.0;
    };
    // (The break in the last paragraph, with text; and in an empty paragraph after it.)
    const double inText = columnsTop("<w:p><w:pPr>" + one + "</w:pPr><w:r><w:t>Above</w:t></w:r></w:p>");
    const double afterIt = columnsTop("<w:p><w:r><w:t>Above</w:t></w:r></w:p><w:p><w:pPr>" + one + "</w:pPr></w:p>");
    QVERIFY(inText > 0 && afterIt > 0);
    QVERIFY2(std::abs(inText - afterIt) < 0.01, qPrintable(QStringLiteral("the columns begin at %1 with the break in the text above, at %2 after an empty paragraph with it").arg(inText).arg(afterIt)));
}


// The empty paragraph that ends a section stays on the page the text before it ends on, even when the page has no room left for its line (a continuous section's takes
// none; and before a section on a page of its own it is only where the section breaks): the section after, which needs a page of its own (for its other paper, or by its
// type), is the second page, not the third.
void WriteTests::anEmptyBreakParagraphAtTheFootOfAFullPageStaysThere()
{
    auto line = [](const char *text) {
        return "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr><w:r><w:t>" + QByteArray(text) + "</w:t></w:r></w:p>";
    };
    QByteArray body;
    for (int i = 0; i < 50; ++i) body += line("Filling");   // (600 points of the 648 of the text area)
    const QByteArray margins = "<w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";
    // (The break paragraph's own line is 60 points: more than the 48 that are left.)
    body += "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"1200\" w:lineRule=\"exact\"/><w:sectPr>" + margins + "</w:sectPr></w:pPr></w:p>" + line("Second");
    auto pagesWith = [&](const QByteArray &type, const QByteArray &paper, int top) {
        const QByteArray last = "<w:sectPr>" + type + paper + "<w:pgMar w:top=\"" + QByteArray::number(top) + "\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        return open(makeDocxWithParts(body, last, {}));
    };
    const QByteArray landscape = "<w:pgSz w:w=\"15840\" w:h=\"12240\" w:orient=\"landscape\"/>", portrait = "<w:pgSz w:w=\"12240\" w:h=\"15840\"/>";
    const QByteArray continuous = "<w:type w:val=\"continuous\"/>";
    // (A continuous section of other paper, a section on a page of its own by its type, and a continuous one with other top margin that shares the page the text before it
    // fills: its text goes on at the next page, with nothing lost.)
    struct Case { QByteArray type, paper; int top; };
    for (const Case &c : {Case{continuous, landscape, 1440}, Case{QByteArray(), portrait, 1440}, Case{continuous, portrait, 2160}}) {
        const QByteArray &type = c.type;
        auto doc = pagesWith(type, c.paper, c.top);
        QVERIFY(doc);
        QVERIFY2(doc->pager().pageCount() == 2, qPrintable(QStringLiteral("%1 pages for the section type '%2'").arg(doc->pager().pageCount()).arg(QString::fromLatin1(type))));
        const auto second = doc->pager().lines(1);
        QVERIFY(!second.isEmpty());
        QVERIFY2(std::abs(second[0].rect.top()) < 0.01, qPrintable(QString::number(second[0].rect.top())));   // (the section after begins at the top of its page)
    }
    // A section of odd pages after it: the page it would begin is the second, an even one, so a blank page is left and the section's text is on the third.
    auto odd = pagesWith("<w:type w:val=\"oddPage\"/>", portrait, 1440);
    QVERIFY(odd);
    QCOMPARE(odd->pager().pageCount(), 3);
    QVERIFY(odd->pager().lines(1).isEmpty());
    QVERIFY(!odd->pager().lines(2).isEmpty());
}


// A paragraph that begins with a page break and is kept with the next puts the break at the foot of the page the text before it ends on: the text after the break
// is on the next page, so the keeping is not of that page's room (and no page holds only the break).
void WriteTests::aKeptParagraphThatBeginsWithAPageBreakStaysOnTheFullPage()
{
    // (A line of 596 of the 648 points of the text area; the break's paragraph is of 40-point text, which takes about 46 of the 52 that are left, and what follows it a line of
    // 12-point text, which does not fit after it: kept with that, the paragraph would leave the page.)
    const QByteArray filler = "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"11920\" w:lineRule=\"exact\"/></w:pPr><w:r><w:t>Filling</w:t></w:r></w:p>";
    const QByteArray big = "<w:p><w:pPr><w:keepNext/><w:spacing w:before=\"0\" w:after=\"0\"/></w:pPr><w:r><w:rPr><w:sz w:val=\"80\"/></w:rPr><w:br w:type=\"page\"/></w:r><w:r><w:rPr><w:sz w:val=\"80\"/></w:rPr><w:t>Chapter</w:t></w:r></w:p>";
    const QByteArray after = "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\"/></w:pPr><w:r><w:t>After.</w:t></w:r></w:p>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(filler + big + after, sect, {}));
    QVERIFY(doc);
    QCOMPARE(doc->pager().pageCount(), 2);
    QCOMPARE(int(doc->pager().lines(0).size()), 2);   // (the filler's line and the break's)
    const auto second = doc->pager().lines(1);
    QVERIFY(second.size() >= 2);
    QVERIFY2(std::abs(second[0].rect.top()) < 0.01, qPrintable(QString::number(second[0].rect.top())));
}


// The page borders of a section (w:pgBorders) are read: where they are measured from, on which pages, and each side's line (its style, width in eighths of a point, space in
// points, and color); a pattern of pictures is a line of its own width here. The file comes back as it was.
void WriteTests::aSectionsPageBordersAreRead()
{
    const QByteArray borders = "<w:pgBorders w:offsetFrom=\"page\" w:display=\"firstPage\"><w:top w:val=\"double\" w:sz=\"4\" w:space=\"24\" w:color=\"auto\"/>"
                               "<w:left w:val=\"single\" w:sz=\"12\" w:space=\"20\" w:color=\"000080\" w:shadow=\"1\"/><w:bottom w:val=\"dotted\" w:sz=\"6\" w:space=\"4\" w:color=\"FF0000\"/>"
                               "<w:right w:val=\"palmsBlack\" w:sz=\"8\" w:space=\"1\" w:color=\"auto\"/></w:pgBorders>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>" +
                            borders + "</w:sectPr>";
    auto doc = open(makeDocx(plainP("text"), kStyles, QByteArray(), sect));
    QVERIFY(doc);
    const flow::PageBorders &b = doc->flow().section(0).pageBorders;
    using Line = flow::PageBorders::Side::Line;
    QVERIFY(b.any() && b.fromPage && b.show == flow::PageBorders::Show::FirstPage);
    QVERIFY(b.top.present && b.top.line == Line::Double && b.top.width == 0.5 && b.top.space == 24 && b.top.color == QColor(Qt::black) && !b.top.art);
    QVERIFY(b.left.present && b.left.line == Line::Single && b.left.width == 1.5 && b.left.space == 20 && b.left.color == QColor(0, 0, 0x80));
    QVERIFY(b.bottom.present && b.bottom.line == Line::Dotted && b.bottom.width == 0.75 && b.bottom.space == 4 && b.bottom.color == QColor(0xff, 0, 0));
    QVERIFY(b.right.present && b.right.art && b.right.line == Line::Single && b.right.width == 1 && b.right.space == 1);
    QVERIFY2(documentXml(doc->toDocx()).contains(borders), documentXml(doc->toDocx()).constData());
    // From the text, on every page, by default; a side the file leaves out has no line, and a side that says none or nil has none.
    const QByteArray text = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgBorders><w:top w:val=\"single\" w:sz=\"8\" w:space=\"6\" w:color=\"auto\"/><w:left w:val=\"nil\"/><w:bottom w:val=\"none\" w:sz=\"8\"/></w:pgBorders></w:sectPr>";
    auto other = open(makeDocx(plainP("text"), kStyles, QByteArray(), text));
    QVERIFY(other);
    const flow::PageBorders &t = other->flow().section(0).pageBorders;
    QVERIFY(!t.fromPage && t.show == flow::PageBorders::Show::All);
    QVERIFY(t.top.present && !t.left.present && !t.bottom.present && !t.right.present);
    auto none = open(makeDocx(plainP("text"), kStyles));
    QVERIFY(none);
    QVERIFY(!none->flow().section(0).pageBorders.any());
}


// A field that sets the text after it at a place (ADVANCE with \\y: so many points from the page's top) asks its paragraph to be no higher than that: the paragraph's first line
// is there when it would be higher (the other directions of the field move nothing here), and the file comes back as it was.
void WriteTests::anAdvanceFieldSetsItsParagraphAtThePlaceFromThePagesTop()
{
    auto withField = [&](const QByteArray &instruction) {
        const QByteArray field = "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> " + instruction + " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        return "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\"/></w:pPr><w:r><w:t>one</w:t></w:r></w:p><w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\"/></w:pPr>" + field +
               "<w:r><w:t>two</w:t></w:r></w:p><w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\"/></w:pPr><w:r><w:t>three</w:t></w:r></w:p>";
    };
    auto tops = [&](const QByteArray &instruction, bool *kept = nullptr) {
        const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"720\" w:right=\"1440\" w:bottom=\"720\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        auto doc = open(makeDocx(withField(instruction), kStyles, QByteArray(), sect));
        QVector<double> out;
        if (!doc) return out;
        const double areaTop = doc->pager().page(0).content.top();
        for (const auto &l : doc->pager().lines(0)) out << areaTop + l.rect.top();
        if (kept) *kept = documentXml(doc->toDocx()).contains("w:instrText") && documentXml(doc->toDocx()).contains(instruction);
        return out;
    };
    const QVector<double> plain = tops("PAGE");   // (a field that sets nothing)
    QCOMPARE(int(plain.size()), 3);
    bool kept = false;
    const QVector<double> moved = tops("ADVANCE \\y 200", &kept);
    QCOMPARE(int(moved.size()), 3);
    QVERIFY2(std::abs(moved[0] - plain[0]) < 0.01 && std::abs(moved[1] - 200) < 0.01 && std::abs((moved[2] - moved[1]) - (plain[2] - plain[1])) < 0.01,
             qPrintable(QStringLiteral("lines at %1, %2, %3").arg(moved[0]).arg(moved[1]).arg(moved[2])));
    QVERIFY(kept);
    // A place the paragraph is lower than already, a different direction, and a field that is no advance move nothing.
    const QVector<double> higher = tops("ADVANCE \\y 10");
    QVERIFY(higher.size() == 3 && std::abs(higher[1] - plain[1]) < 0.01);
    const QVector<double> down = tops("ADVANCE \\d 100");
    QVERIFY(down.size() == 3 && std::abs(down[1] - plain[1]) < 0.01);
}


// A place that no line of the paragraph could be under (the foot of the page, or past it) leaves the paragraph where it is: no page loses its lines, and the document
// has all of its text whatever the place says.
void WriteTests::anAdvanceFieldPastTheFootOfThePageLeavesNoTextOut()
{
    auto totalLines = [&](const QByteArray &instruction, int *pages = nullptr) {
        const QByteArray field = "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> " + instruction + " </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
        const QByteArray first = "<w:p>" + field + "<w:r><w:t>first paragraph words</w:t></w:r></w:p>";
        auto doc = open(makeDocx(first + paragraphs(29, 40), kStyles));
        if (!doc) return -1;
        const int count = doc->pager().pageCount();
        if (pages) *pages = count;
        int lines = 0;
        for (int k = 0; k < count; ++k) lines += int(doc->pager().lines(k).size());
        return lines;
    };
    int plainPages = 0;
    const int plain = totalLines("PAGE", &plainPages);
    QVERIFY(plain > 100 && plainPages > 2);
    for (const char *place : {"600", "720", "740", "756", "792", "100000"}) {
        int pages = 0;
        const int lines = totalLines(QByteArray("ADVANCE \\y ") + place, &pages);
        QVERIFY2(lines == plain && pages >= plainPages, qPrintable(QStringLiteral("a place of %1: %2 lines on %3 pages, where %4 lines on %5 pages are").arg(QLatin1String(place)).arg(lines).arg(pages).arg(plain).arg(plainPages)));
    }
}


// What the program makes of a style's frame properties does not cost a regular expression for every paragraph: a long chain of styles with many properties each, and many
// paragraphs of the last, open in step with the paragraphs. (Compared with a chain of one, not against a clock.)
void WriteTests::aLongChainOfStylesCostsNothingForEachParagraph()
{
    auto build = [&](int depth) {
        QByteArray styles = kStyles;
        for (int i = 0; i < depth; ++i) {
            styles += "<w:style w:type=\"paragraph\" w:styleId=\"Deep" + QByteArray::number(i) + "\"><w:name w:val=\"Deep " + QByteArray::number(i) + "\"/>" +
                      (i + 1 < depth ? "<w:basedOn w:val=\"Deep" + QByteArray::number(i + 1) + "\"/>" : QByteArray("<w:basedOn w:val=\"Normal\"/>")) +
                      "<w:pPr><w:suppressAutoHyphens/><w:kinsoku/><w:wordWrap/><w:snapToGrid/><w:textAlignment w:val=\"auto\"/><w:autoSpaceDE/><w:autoSpaceDN/><w:adjustRightInd/>"
                      "<w:suppressLineNumbers/><w:topLinePunct/><w:mirrorIndents/><w:suppressOverlap/></w:pPr></w:style>";
        }
        QByteArray body;
        for (int i = 0; i < 1500; ++i) body += "<w:p><w:pPr><w:pStyle w:val=\"Deep0\"/></w:pPr><w:r><w:t>paragraph " + QByteArray::number(i) + "</w:t></w:r></w:p>";
        return makeDocx(body, styles);
    };
    auto expressions = [&](const QByteArray &zip) {
        const qint64 before = frameExpressionRunsForTest();
        auto doc = open(zip);
        return doc ? frameExpressionRunsForTest() - before : qint64(-1);
    };
    const QByteArray shallow = build(1), deep = build(32);
    const qint64 base = expressions(shallow), chain = expressions(deep);
    // Counted: not one regular expression is run for the style chain's properties of any of the 1,500 paragraphs (a property that names no frame is passed by a plain look).
    QVERIFY2(base >= 0 && chain >= 0 && chain <= base + 32, qPrintable(QStringLiteral("a chain of 32 styles %1 expressions, of one %2").arg(chain).arg(base)));
}


// A section break made inside a section of odd pages (or of any other kind) is a break of the kind asked for: the section the break makes begins as the command says, not as the
// section it was made in did, and the section before it begins as it did.
void WriteTests::aSectionBreakMadeInAnOddPageSectionBeginsOnTheNextPage()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray odd = "<w:sectPr><w:type w:val=\"oddPage\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocx("<w:p><w:pPr>" + sect + "</w:pPr><w:r><w:t>first section</w:t></w:r></w:p>" + plainP("an odd page section, first paragraph") + plainP("second paragraph"), kStyles, QByteArray(), odd));
    QVERIFY(doc);
    QCOMPARE(doc->sectionCount(), 2);
    QVERIFY(doc->flow().sections[1].startParity == flow::Section::Parity::Odd);
    QTextCursor c(doc->story());
    c.setPosition(doc->story()->findBlockByNumber(1).position() + 3);
    insertSectionBreak(*doc, c);
    QCOMPARE(doc->sectionCount(), 3);
    QVERIFY2(doc->flow().sections[1].startParity == flow::Section::Parity::Odd, "the section before the break begins as it did");
    QVERIFY2(doc->flow().sections[2].startParity == flow::Section::Parity::Any && !doc->flow().sections[2].continuous, "the section the break makes is on the next page");
    QCOMPARE(doc->sectionProps(2).type.isEmpty() || doc->sectionProps(2).type == QLatin1String("nextPage"), true);
    // Each kind of break makes a section that begins as it says (and the file says so: nothing for the next page).
    struct Kind { const char *type; bool continuous; flow::Section::Parity parity; };
    for (const Kind &k : {Kind{"continuous", true, flow::Section::Parity::Any}, Kind{"oddPage", false, flow::Section::Parity::Odd}, Kind{"evenPage", false, flow::Section::Parity::Even}, Kind{"nextPage", false, flow::Section::Parity::Any}}) {
        auto plain = open(makeDocx(plainP("one") + plainP("two"), kStyles));
        QVERIFY(plain);
        QTextCursor at(plain->story());
        at.setPosition(plain->story()->findBlockByNumber(0).position() + 2);
        insertSectionBreak(*plain, at, QString::fromLatin1(k.type));
        QCOMPARE(plain->sectionCount(), 2);
        QVERIFY2(plain->flow().sections[1].continuous == k.continuous && plain->flow().sections[1].startParity == k.parity, k.type);
        QVERIFY2(plain->flow().sections[0].startParity == flow::Section::Parity::Any && !plain->flow().sections[0].continuous, k.type);
        const QByteArray saved = documentXml(plain->toDocx());
        QVERIFY2(strcmp(k.type, "nextPage") == 0 ? !saved.contains("<w:type") : saved.contains(QByteArray("<w:type w:val=\"") + k.type + "\"/>"), k.type);
    }
}


// Three paragraphs, the second with an ADVANCE field that asks for 200 points; the place a block asks for (-1: none).
QByteArray WriteTests::askingParagraphs()
{
    const QByteArray field = "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\"> ADVANCE \\y 200 </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
    return plainP("one") + "<w:p>" + field + "<w:r><w:t>two words</w:t></w:r></w:p>" + plainP("three");
}

double WriteTests::askedPlace(WriteDoc &doc, int block)
{
    const QTextBlockFormat f = doc.story()->findBlockByNumber(block).blockFormat();
    return f.hasProperty(flow::fp::AdvanceTop) ? f.doubleProperty(flow::fp::AdvanceTop) : -1.0;
}


// A paragraph that asks for a place (its ADVANCE field) keeps the ask as the person edits it. A style applied to it keeps the ask.
void WriteTests::aStyleAppliedToAParagraphThatAsksForAPlaceKeepsTheAsk()
{
    auto doc = open(makeDocx(askingParagraphs(), kStyles));
    QVERIFY(doc && askedPlace(*doc, 1) == 200.0);
    QTextCursor c(doc->story());
    c.setPosition(doc->story()->findBlockByNumber(1).position() + 4);
    applyParagraphStyle(*doc, c, QStringLiteral("Heading1"));
    QVERIFY2(askedPlace(*doc, 1) == 200.0, "the style took the ask away");
}


// Enter in it leaves the ask with the first half only (and the field).
void WriteTests::enterInAParagraphThatAsksForAPlaceLeavesTheAskWithTheFirstHalf()
{
    auto doc = open(makeDocx(askingParagraphs(), kStyles));
    QVERIFY(doc);
    QTextCursor c(doc->story());
    c.setPosition(doc->story()->findBlockByNumber(1).position() + 5);   // (after the three characters of the field, and "tw")
    insertParagraph(*doc, c);
    QCOMPARE(doc->story()->blockCount(), 4);
    QVERIFY2(askedPlace(*doc, 1) == 200.0, "the first half lost the ask");
    QVERIFY2(askedPlace(*doc, 2) < 0, "the second half has the ask too");
}


// Taking its field out takes the ask away (other text taken out leaves it), and one undo brings the field and the ask back.
void WriteTests::takingOutAnAdvanceFieldTakesTheAskAway()
{
    auto doc = open(makeDocx(askingParagraphs(), kStyles));
    QVERIFY(doc);
    const int at = doc->story()->findBlockByNumber(1).position() + 1;   // (the field's instruction is the second of its three characters)
    QCOMPARE(doc->story()->characterAt(at), QChar::ObjectReplacementCharacter);
    QTextCursor t(doc->story());
    t.setPosition(doc->story()->findBlockByNumber(1).position() + 3);
    t.setPosition(doc->story()->findBlockByNumber(1).position() + 5, QTextCursor::KeepAnchor);
    removeSelection(*doc, t);
    QVERIFY2(askedPlace(*doc, 1) == 200.0, "other text taken out took the ask away");
    QTextCursor c(doc->story());
    c.setPosition(at);
    c.setPosition(at + 1, QTextCursor::KeepAnchor);
    removeSelection(*doc, c);
    QVERIFY2(askedPlace(*doc, 1) < 0, "the ask is still there with its field gone");
    doc->story()->undo();
    QVERIFY2(askedPlace(*doc, 1) == 200.0 && doc->story()->characterAt(at) == QChar::ObjectReplacementCharacter, "one undo did not bring the field and the ask back");
}


// The page color of a file (w:background) is the pages' color, as the page is drawn, and the file comes back with it.
void WriteTests::aFilesPageColorFillsItsPages()
{
    auto doc = open(makeDocxWithBackground("<w:background w:color=\"00B8FF\"/>"));
    QVERIFY(doc);
    QCOMPARE(doc->flow().pageColor, QColor(0x00, 0xb8, 0xff));
    QVERIFY(documentXml(doc->toDocx()).contains("<w:background w:color=\"00B8FF\"/>"));
    auto drawnAt = [&](WriteDoc &d, int x, int y) {
        PageView view;
        view.resize(1000, 700);
        view.setDocument(&d);
        return view.pageImage(0, 816).pixelColor(x, y);
    };
    // The page as drawn is that color at its corners and between its lines, and white where the file has no page color.
    QCOMPARE(drawnAt(*doc, 4, 4), QColor(0x00, 0xb8, 0xff));
    QCOMPARE(drawnAt(*doc, 810, 1050), QColor(0x00, 0xb8, 0xff));
    auto none = open(makeDocxWithBackground(QByteArray()));
    QVERIFY(none);
    QVERIFY(!none->flow().pageColor.isValid());
    QCOMPARE(drawnAt(*none, 4, 4), QColor(Qt::white));
    auto automatic = open(makeDocxWithBackground("<w:background w:color=\"auto\"/>"));
    QVERIFY(automatic);
    QVERIFY(!automatic->flow().pageColor.isValid());
}


// The page color is in the PDF export (a reference PDF of such a file has it) and is not printed on paper.
void WriteTests::thePageColorIsInThePdfExportAndNotOnAPrinter()
{
    QTemporaryDir dir;
    WriteWindow w;
    w.setDocument(open(makeDocxWithBackground("<w:background w:color=\"00B8FF\"/>")), QString());
    w.view()->paginateAll();
    QVector<bool> exported, printed;
    QVector<bool> *into = &exported;
    const ObserverGuard observer;
    write::setPaintObserver([&](const flow::MaterializedPage &page, const write::PageDrawing &) {
        bool has = false;
        for (const auto &item : page.doc->pages[0]->items) has |= item->id.startsWith(QLatin1String("flow-page-color-"));
        *into << has;
    });
    QString next = dir.filePath(QStringLiteral("export.pdf"));
    w.setSavePathChooser([&](const QString &, const QString &) { return next; });
    QVERIFY(w.exportPdf());
    into = &printed;
    next = dir.filePath(QStringLiteral("print.pdf"));
    WriteOutputOptions o;
    o.toFile = true;
    QVERIFY(w.printOutput(o));
    write::setPaintObserver(nullptr);
    QVERIFY2(exported == QVector<bool>{true}, "the PDF export has no page color");
    QVERIFY2(printed == QVector<bool>{false}, "the printer is sent the page color");
}


// A symbol of a font that isn't there is its character in the stand-in (Calibri): the line it is on is Calibri's, taller than Times New Roman's.
void WriteTests::aSymbolOfAMissingFontMakesItsLineTheStandIns()
{
    auto run = [](const QByteArray &inner) { return "<w:r><w:rPr><w:rFonts w:ascii=\"Times New Roman\" w:hAnsi=\"Times New Roman\"/><w:sz w:val=\"24\"/></w:rPr>" + inner + "</w:r>"; };
    auto para = [&](const QByteArray &inner) { return "<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"auto\"/></w:pPr>" + run(inner) + "</w:p>"; };
    const QByteArray body = para("<w:t>Plain words</w:t>") + para("<w:t>Words </w:t></w:r><w:r><w:rPr><w:rFonts w:ascii=\"Times New Roman\" w:hAnsi=\"Times New Roman\"/><w:sz w:val=\"24\"/></w:rPr><w:sym w:font=\"No Such Face Anywhere\" w:char=\"0047\"/>");
    auto doc = open(makeDocx(body));
    QVERIFY(doc);
    const auto lines = doc->pager().lines(0);
    QVERIFY(lines.size() >= 2);
    QVERIFY2(std::abs(lines[0].rect.height() - 12 * (1825 + 443 + 87) / 2048.0) < 0.02, qPrintable(QString::number(lines[0].rect.height())));
    QVERIFY2(std::abs(lines[1].rect.height() - 12 * (1950 + 550) / 2048.0) < 0.02, qPrintable(QString::number(lines[1].rect.height())));
}


// A multiple of the line that is written as a negative number (a file converted from an older format has them) is the same multiple as a positive
// one: the line is as tall as it would be for its size, not as one with nothing in it.
void WriteTests::aNegativeMultipleOfTheLineIsReadAsItsSize()
{
    auto lineOf = [](const char *line) {
        const QByteArray body = QByteArray("<w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"") + line + "\" w:lineRule=\"auto\"/></w:pPr><w:r><w:t>Words</w:t></w:r></w:p>";
        auto doc = open(makeDocx(body));
        return doc->pager().lines(0).first().rect.height();
    };
    const double single = lineOf("240");
    QVERIFY(single > 5);
    QVERIFY2(std::abs(lineOf("-480") - 2 * single) < 0.02, qPrintable(QString::number(lineOf("-480"))));
    QVERIFY2(std::abs(lineOf("-120") - lineOf("120")) < 0.02, qPrintable(QString::number(lineOf("-120"))));
}


// A number in a list is the size its level gives it (half-points, as a bullet's is), not the text's: a ten-point number before twelve-point text.
void WriteTests::aListNumberHasTheSizeItsLevelGives()
{
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/>"
                                 "<w:pPr><w:ind w:left=\"360\" w:hanging=\"360\"/></w:pPr><w:rPr><w:sz w:val=\"20\"/></w:rPr></w:lvl></w:abstractNum>"
                                 "<w:abstractNum w:abstractNumId=\"1\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/><w:lvlText w:val=\"%1.\"/></w:lvl></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num><w:num w:numId=\"2\"><w:abstractNumId w:val=\"1\"/></w:num>";
    const QByteArray body = "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"1\"/></w:numPr></w:pPr><w:r><w:rPr><w:sz w:val=\"24\"/></w:rPr><w:t>Sized</w:t></w:r></w:p>"
                            "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"2\"/></w:numPr></w:pPr><w:r><w:rPr><w:sz w:val=\"24\"/></w:rPr><w:t>Plain</w:t></w:r></w:p>";
    auto doc = open(makeDocx(body, kStyles, numbering));
    QVERIFY(doc);
    const QTextBlock sized = doc->story()->begin(), plain = sized.next();
    QVERIFY(sized.textList() && plain.textList());
    QCOMPARE(sized.textList()->format().property(tp::BulletSize).toDouble(), 10.0);
    QVERIFY(!plain.textList()->format().hasProperty(tp::BulletSize));
}


// A page number in a frame (set in the middle or at the side, anchored to the text) takes no room: a footer of it and an empty paragraph is
// one line tall, and the number is set as the frame says.
void WriteTests::aFrameParagraphInAFooterTakesNoRoom()
{
    const QByteArray sect = "<w:sectPr><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"1080\" w:right=\"1440\" w:bottom=\"1080\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto footerWith = [](const QByteArray &frame) {
        return headerPart("ftr", {"<w:pPr>" + frame + "</w:pPr><w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText> PAGE </w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"end\"/></w:r>", QByteArray()});
    };
    auto bottomOf = [&](const QByteArray &frame) {
        auto doc = open(makeDocxWithParts(paragraphs(60, 40), sect, {{QStringLiteral("/word/footer1.xml"), footerWith(frame)}}));
        return std::make_pair(doc->pager().page(0).content.bottom(), doc->pager().materialize(0)->doc->pages[0]->items.size());
    };
    const auto framed = bottomOf("<w:framePr w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"margin\" w:xAlign=\"center\" w:y=\"1\"/>");
    const auto plain = bottomOf(QByteArray());
    // As one line (the empty paragraph's), the footer leaves the text its bottom margin; as two it takes more.
    QVERIFY2(std::abs(framed.first - (792 - 54)) < 0.01, qPrintable(QString::number(framed.first)));
    QVERIFY2(plain.first < framed.first - 5, qPrintable(QString::number(plain.first)));
}


// A shape in the header that the text goes round (a banner in a text box, set from the page's top) holds the page's text off: the text begins under it, where the margin
// alone would let it go behind.
void WriteTests::aHeadersWrappedShapeMovesTheTextBelowIt()
{
    auto top = [&](const QByteArray &wrap) {
        const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                                "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
        const QByteArray shape = "<w:r><w:pict><v:shapetype id=\"t202\" coordsize=\"21600,21600\" o:spt=\"202\" path=\"m,l,21600r21600,l21600,xe\"/>"
                                 "<v:shape id=\"s1\" type=\"#t202\" style=\"position:absolute;margin-left:-20pt;margin-top:0;width:400pt;height:150pt;mso-position-vertical-relative:page;z-index:1\" filled=\"f\" stroked=\"f\">"
                                 "<v:textbox><w:txbxContent><w:p><w:r><w:t>banner</w:t></w:r></w:p></w:txbxContent></v:textbox>" + wrap + "</v:shape></w:pict></w:r>";
        QByteArray header = headerPart("hdr", {shape});
        header.replace("<w:hdr ", "<w:hdr xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\" ");
        auto doc = open(makeDocxWithParts(paragraphs(10, 5), sect, {{QStringLiteral("/word/header1.xml"), header}}));
        return doc ? doc->pager().page(0).content.top() : -1.0;
    };
    const double round = top("<w10:wrap type=\"square\"/>"), none = top("<w10:wrap type=\"none\"/>");
    QVERIFY2(std::abs(none - 72) < 12, qPrintable(QString::number(none)));   // (not wrapped: the margin, and the header's own line)
    QVERIFY2(round > 149.9 && round < 170, qPrintable(QString::number(round)));
}


// A paragraph of a header that is set at a place on the page (a frame whose position is from the page) is a text box there and takes no room in the header, whatever its
// offset says: one that asks to be above the page's top is at the top. The paragraph after it (its mark hidden does not join them) keeps its line.
void WriteTests::aHeaderFrameSetAtAPlaceOnThePageIsATextBoxThere()
{
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"360\" w:right=\"1440\" w:bottom=\"720\" w:left=\"1440\" w:header=\"240\" w:footer=\"240\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray frame = "<w:pPr><w:framePr w:w=\"9360\" w:h=\"280\" w:hRule=\"exact\" w:wrap=\"notBeside\" w:vAnchor=\"page\" w:hAnchor=\"text\" w:y=\"-314\"/><w:jc w:val=\"right\"/><w:rPr><w:vanish/></w:rPr></w:pPr>"
                             "<w:r><w:t xml:space=\"preserve\">FRAMED page </w:t></w:r><w:fldSimple w:instr=\" PAGE \"><w:r><w:t>1</w:t></w:r></w:fldSimple>";
    const QByteArray footFrame = "<w:pPr><w:framePr w:w=\"9360\" w:h=\"280\" w:hRule=\"exact\" w:wrap=\"notBeside\" w:vAnchor=\"page\" w:hAnchor=\"text\" w:y=\"10000\"/></w:pPr><w:r><w:t>FOOTFRAME</w:t></w:r>";
    const QByteArray first = "<w:r><w:t>First line</w:t></w:r>", foot = "<w:r><w:t>Foot line</w:t></w:r>";
    auto open1 = [&](const QList<QByteArray> &inHeader, const QList<QByteArray> &inFooter) {
        return open(makeDocxWithParts(paragraphs(10, 5), sect, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", inHeader)}, {QStringLiteral("/word/footer1.xml"), headerPart("ftr", inFooter)}}));
    };
    // The text box of a frame whose text is `text`: not the header's or the footer's own box, which has the frame's paragraph in its story too, with no line of it.
    auto boxOf = [&](const flow::MaterializedPage &page, const char *text) {
        const TextItem *box = nullptr;
        for (const auto &item : page.doc->pages[0]->items)
            if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
                if (!t->id.contains(QLatin1String("header-box")) && !t->id.contains(QLatin1String("footer-box")) && page.doc->story(t->storyId) &&
                    page.doc->story(t->storyId)->doc->toPlainText().contains(QLatin1String(text))) box = t;
        return box;
    };
    auto doc = open1({first, frame, QByteArray()}, {foot, footFrame});
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    const TextItem *box = boxOf(*page, "FRAMED");
    QVERIFY2(box, "the header's frame is a text box of its own");
    QVERIFY2(box->rect.top() >= 0 && box->rect.top() < 1, qPrintable(QString::number(box->rect.top())));
    // Its text has the page's number in it, live: the box is drawn on each page with that page's.
    bool live = false;
    for (QTextBlock b = page->doc->story(box->storyId)->doc->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) live |= it.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("page"));
    QVERIFY2(live, "the page number of the frame is a field");
    // The footer's frame is a text box there, at the place the page's top is not.
    const TextItem *foot_ = boxOf(*page, "FOOTFRAME");
    QVERIFY2(foot_, "the footer's frame is a text box of its own");
    QVERIFY2(std::abs(foot_->rect.top() - 500) < 0.01, qPrintable(QString::number(foot_->rect.top())));
    // Neither takes room in its story, and the empty paragraph after the header's frame keeps its line.
    const double with = doc->pager().page(0).content.top();
    const double without = open1({first, QByteArray()}, {foot})->pager().page(0).content.top(), alone = open1({first}, {foot})->pager().page(0).content.top();
    QVERIFY2(std::abs(with - without) < 0.1, qPrintable(QStringLiteral("%1 with the frame, %2 without").arg(with).arg(without)));
    QVERIFY2(without > alone + 5, qPrintable(QStringLiteral("%1 with its empty paragraph, %2 without").arg(without).arg(alone)));
    QVERIFY2(std::abs(doc->pager().page(0).content.bottom() - open1({first, QByteArray()}, {foot})->pager().page(0).content.bottom()) < 0.1, "the footer's frame takes no room under the text");
}


// A frame of a header of more paragraphs than a frame has is text, as the body's is: the header keeps its paragraphs and the page has no stack of boxes made of it.
void WriteTests::aHeaderFrameOfTooManyParagraphsIsTextOfTheHeader()
{
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray frame = "<w:pPr><w:framePr w:w=\"9360\" w:h=\"280\" w:hRule=\"exact\" w:wrap=\"notBeside\" w:vAnchor=\"page\" w:hAnchor=\"text\" w:y=\"100\"/></w:pPr><w:r><w:t>FRAMEPARA</w:t></w:r>";
    QList<QByteArray> paragraphsOfHeader;
    for (int i = 0; i < 320; ++i) paragraphsOfHeader << frame;
    auto doc = open(makeDocxWithParts(paragraphs(10, 5), sect, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", paragraphsOfHeader)}}));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    int boxes = 0;
    for (const auto &item : page->doc->pages[0]->items)
        if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
            if (!t->id.contains(QLatin1String("header-box")) && page->doc->story(t->storyId) && page->doc->story(t->storyId)->doc->toPlainText().contains(QLatin1String("FRAMEPARA"))) ++boxes;
    QVERIFY2(boxes == 0, qPrintable(QStringLiteral("%1 boxes were made of a frame of 320 paragraphs").arg(boxes)));
    const Story *header = nullptr;
    for (const auto &item : page->doc->pages[0]->items)
        if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
            if (t->id.contains(QLatin1String("header-box"))) header = page->doc->story(t->storyId);
    QVERIFY(header);
    QVERIFY2(!header->doc->begin().blockFormat().boolProperty(tp::OutOfLayout), "the header's first paragraph has lines");
}


// The boxes of a header's frames are made once for the header, not for each page: however many pages are drawn, they are made a time or two (counted, not timed).
void WriteTests::theFramesOfAHeaderCostNothingForEachPage()
{
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray frame = "<w:pPr><w:framePr w:w=\"9360\" w:h=\"280\" w:hRule=\"exact\" w:wrap=\"notBeside\" w:vAnchor=\"page\" w:hAnchor=\"text\" w:y=\"100\"/></w:pPr><w:r><w:t>Framed paragraph of the header, with some words</w:t></w:r>";
    QList<QByteArray> inHeader;
    for (int i = 0; i < 250; ++i) inHeader << frame;
    auto doc = open(makeDocxWithParts(paragraphs(400, 40), sect, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", inHeader)}}));
    QVERIFY(doc);
    const int pages = doc->pager().pageCount();
    QVERIFY2(pages >= 20, qPrintable(QString::number(pages)));
    for (int k = 0; k < pages; ++k) QVERIFY(doc->pager().materialize(k));
    QVERIFY2(doc->pager().headerFramesMadeForTest() <= 2, qPrintable(QStringLiteral("the header's frames were made %1 times for %2 pages").arg(doc->pager().headerFramesMadeForTest()).arg(pages)));
}


// How far down the page a header's wrapped shapes reach is worked out once for the header, not for each page: however many pages are laid out, it is worked out a time or two.
void WriteTests::theWrappedShapesOfAHeaderCostNothingForEachPage()
{
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray shape = "<w:r><w:pict><v:shapetype id=\"t202\" coordsize=\"21600,21600\" o:spt=\"202\" path=\"m,l,21600r21600,l21600,xe\"/>"
                             "<v:shape id=\"s1\" type=\"#t202\" style=\"position:absolute;margin-left:-20pt;margin-top:0;width:100pt;height:20pt;mso-position-vertical-relative:page;z-index:1\" filled=\"f\" stroked=\"f\">"
                             "<v:textbox><w:txbxContent><w:p><w:r><w:t>banner</w:t></w:r></w:p></w:txbxContent></v:textbox><w10:wrap type=\"square\"/></v:shape></w:pict></w:r>";
    QList<QByteArray> inHeader{shape};
    for (int i = 0; i < 400; ++i) inHeader << "<w:pPr><w:framePr w:w=\"2000\" w:xAlign=\"right\" w:wrap=\"around\" w:vAnchor=\"text\" w:hAnchor=\"margin\" w:y=\"1\"/></w:pPr><w:r><w:t>side</w:t></w:r>";
    QByteArray header = headerPart("hdr", inHeader);
    header.replace("<w:hdr ", "<w:hdr xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\" ");
    auto doc = open(makeDocxWithParts(paragraphs(600, 40), sect, {{QStringLiteral("/word/header1.xml"), header}}));
    QVERIFY(doc);
    const int pages = doc->pager().pageCount();
    QVERIFY2(pages >= 50, qPrintable(QString::number(pages)));
    // (All the pages laid out again: each has the header's places worked out for it, or all have them once.)
    doc->pager().invalidate();
    const qint64 before = doc->pager().headerReachesWorkedOutForTest();
    QCOMPARE(doc->pager().pageCount(), pages);
    const qint64 made = doc->pager().headerReachesWorkedOutForTest() - before;
    QVERIFY2(made <= 2, qPrintable(QStringLiteral("how far the header's shapes reach was worked out %1 times for %2 pages").arg(made).arg(pages)));
}


// A right tab stop that is past the right indent, and past the text area too (into the margin, as a letterhead's address block is set), still takes the text after its
// tab: the line is as wide as the stop, and the text is one line ending there, as with a stop inside the text area.
void WriteTests::aRightTabIntoTheMarginTakesTheTextAfterItsTab()
{
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"720\" w:right=\"1800\" w:bottom=\"1440\" w:left=\"1800\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto contentTop = [&](const QByteArray &entry) {
        const QByteArray para = "<w:pPr><w:tabs><w:tab w:val=\"right\" w:pos=\"9900\"/></w:tabs><w:ind w:right=\"-540\"/><w:rPr><w:b/><w:sz w:val=\"16\"/></w:rPr></w:pPr>" + entry;
        auto doc = open(makeDocxWithParts(paragraphs(10, 5), sect, {{QStringLiteral("/word/header1.xml"), headerPart("hdr", {para})}}));
        return doc ? doc->pager().page(0).content.top() : -1.0;
    };
    const QByteArray tabbed = "<w:r><w:rPr><w:b/><w:sz w:val=\"16\"/></w:rPr><w:tab/><w:t>Fax-On-Demand   202 / 418-2830 and a good deal more to say</w:t></w:r>";
    const QByteArray plain = "<w:r><w:rPr><w:b/><w:sz w:val=\"16\"/></w:rPr><w:t>Fax</w:t></w:r>";
    const double one = contentTop(plain), tab = contentTop(tabbed);
    QVERIFY(one > 0 && tab > 0);
    QVERIFY2(std::abs(tab - one) < 0.1, qPrintable(QStringLiteral("the header with a right tab into the margin makes the text start at %1, one line at %2").arg(tab).arg(one)));
}


// Tabs that end a paragraph, after its last visible character, take no width and cause no wrap: with stops at 144, 288, and 468 points (the text area's edge) the fourth of four
// trailing tabs is past the margin, and the paragraph is still one line. Tabs that something follows are as they were (a tab past the margin takes the text after it to the next line).
void WriteTests::tabsThatEndAParagraphCauseNoWrap()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray stops = "<w:pPr><w:tabs><w:tab w:val=\"left\" w:pos=\"2880\"/><w:tab w:val=\"left\" w:pos=\"5760\"/><w:tab w:val=\"left\" w:pos=\"9360\"/></w:tabs></w:pPr>";
    auto linesOf = [&](const QByteArray &runs) {
        auto doc = open(makeDocxWithParts("<w:p>" + stops + runs + "</w:p>", sect, {}));
        return doc ? int(doc->pager().lines(0).size()) : -1;
    };
    const QByteArray tab = "<w:r><w:tab/></w:r>";
    const QByteArray total = "<w:r><w:t>Total</w:t></w:r>";
    QCOMPARE(linesOf(total + tab + tab + tab), 1);
    QCOMPARE(linesOf(total + tab + tab + tab + tab), 1);
    QCOMPARE(linesOf(total + tab + tab + tab + tab + tab + tab), 1);
    QCOMPARE(linesOf(total + tab + total + tab + tab + tab + tab), 1);
    QCOMPARE(linesOf(total + tab + tab + tab + tab + total), 2);
}


// Trailing tabs that draw something keep their width: a signature line made of underlined tabs after the words is drawn to the margin, where tabs that show nothing take none.
void WriteTests::trailingTabsThatAreUnderlinedAreDrawn()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray stops = "<w:pPr><w:tabs><w:tab w:val=\"left\" w:pos=\"2880\"/><w:tab w:val=\"left\" w:pos=\"5760\"/><w:tab w:val=\"left\" w:pos=\"9360\"/></w:tabs></w:pPr>";
    auto inkBeyondTheWords = [&](const QByteArray &tabRunProps) {
        QByteArray tabs;
        for (int i = 0; i < 6; ++i) tabs += "<w:r>" + tabRunProps + "<w:tab/></w:r>";
        auto doc = open(makeDocxWithParts("<w:p>" + stops + "<w:r><w:t>Signature:</w:t></w:r>" + tabs + "</w:p>", sect, {}));
        if (!doc) return -1;
        PageView view;
        view.resize(1000, 700);
        view.setDocument(doc.get());
        const QImage img = view.pageImage(0, 816);
        const double perPoint = img.width() / 612.0;
        int dark = 0;   // (in the lines of the paragraph, right of where its words end)
        for (int y = int(72 * perPoint); y < int(100 * perPoint); ++y)
            for (int x = int(200 * perPoint); x < int(460 * perPoint); ++x) dark += qGray(img.pixel(x, y)) < 128;
        return dark;
    };
    const int underlined = inkBeyondTheWords("<w:rPr><w:u w:val=\"single\"/></w:rPr>"), plain = inkBeyondTheWords(QByteArray());
    QCOMPARE(plain, 0);
    QVERIFY2(underlined > 100, qPrintable(QStringLiteral("%1 dark points of the underline of the tabs").arg(underlined)));
}


// The border a file puts round a run (w:bdr) is the run's, drawn round its text; a drop-down field of a form, which shows the entry it has chosen, shows its first entry when the
// file kept no result for it (the box round it is that entry's width).
void WriteTests::aRunBorderAndTheFirstEntryOfADropDownAreShown()
{
    const QByteArray bdr = "<w:rPr><w:bdr w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/></w:rPr>";
    const QByteArray body = "<w:p><w:r><w:t xml:space=\"preserve\">before </w:t></w:r>"
                            "<w:r>" + bdr + "<w:fldChar w:fldCharType=\"begin\"><w:ffData><w:name w:val=\"dd\"/><w:enabled/><w:ddList><w:listEntry w:val=\"CHOSEN\"/><w:listEntry w:val=\" X \"/></w:ddList></w:ffData></w:fldChar></w:r>"
                            "<w:r>" + bdr + "<w:instrText xml:space=\"preserve\"> FORMDROPDOWN </w:instrText></w:r><w:r>" + bdr + "<w:fldChar w:fldCharType=\"end\"/></w:r>"
                            "<w:r><w:t xml:space=\"preserve\"> after</w:t></w:r></w:p>"
                            "<w:p><w:r>" + bdr + "<w:t>plainbox</w:t></w:r></w:p>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(body, sect, {}));
    QVERIFY(doc);
    QString spec;
    bool plain = false, chosen = false;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const QString border = f.charFormat().stringProperty(tp::RunBorder);
            if (f.text() == QLatin1String("plainbox")) plain = !border.isEmpty();
            if (f.charFormat().stringProperty(tp::Field).contains(QLatin1String("CHOSEN"))) {
                chosen = true;
                spec = border;
            }
        }
    QVERIFY2(plain, "the run with a border has it");
    QVERIFY2(chosen, "the drop-down field shows its first entry");
    QVERIFY2(spec == QLatin1String("single;4;0;"), qPrintable(spec));
}


// The drop-down fields of forms are found in one pass over a paragraph's elements: a paragraph of thousands of fields that are never ended (a damaged file's) does not cost the
// square of them to open. (Counted: the elements the scan looked at.)
void WriteTests::aParagraphOfThousandsOfUnendedDropDownsIsScannedOnce()
{
    const int fields = 3000;
    QByteArray body = "<w:p>";
    for (int i = 0; i < fields; ++i)
        body += "<w:r><w:fldChar w:fldCharType=\"begin\"><w:ffData><w:ddList><w:listEntry w:val=\"a\"/></w:ddList></w:ffData></w:fldChar></w:r>";
    body += "<w:r><w:t>end</w:t></w:r></w:p>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(body, sect, {}));
    QVERIFY(doc);
    QVERIFY2(doc->dropDownStepsForTest() <= 4 * fields, qPrintable(QStringLiteral("%1 elements looked at for %2 fields").arg(doc->dropDownStepsForTest()).arg(fields)));
}


// The selection is the body's: the tables of a header or a footer are drawn without it, whichever positions of the body it covers (they are positions of the header's own story).
void WriteTests::theBodySelectionIsNotPaintedInAHeadersTable()
{
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray header = "<w:hdr " + QByteArray(kNs) + "><w:tbl><w:tblPr><w:tblW w:w=\"8000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"8000\"/></w:tblGrid><w:tr><w:tc><w:tcPr><w:tcW w:w=\"8000\" w:type=\"dxa\"/></w:tcPr>"
                              "<w:p><w:r><w:t>HEADERCELL with some words in it</w:t></w:r></w:p></w:tc></w:tr></w:tbl><w:p/></w:hdr>";
    auto doc = open(makeDocxWithParts(paragraphs(10, 30), sect, {{QStringLiteral("/word/header1.xml"), header}}));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    QVERIFY(page && page->furnitureTables.size() == 1);
    auto paint = [&](int from, int to) {
        QImage img(1224, 1584, QImage::Format_RGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.scale(2, 2);
        flow::TablePaint options;
        options.selFrom = from;
        options.selTo = to;
        flow::paintTables(&p, *page, options);
        p.end();
        const QRectF r = page->furnitureTables[0]->rect();
        return img.copy(QRect(int(r.left() * 2), int(r.top() * 2), int(r.width() * 2) + 2, int(r.height() * 2) + 2));
    };
    const QImage none = paint(-1, -1), selected = paint(0, 30);
    QVERIFY2(none == selected, "the body's selection (its first thirty characters) is painted in the header's table");
}


// The border of a run is as wide as a file can say (twelve points at most, thirty-one away), and it is also what the run's character style or its paragraph's style gives (the
// run's own, if it says one, over them); a border of none in the run takes it off.
void WriteTests::aRunBorderIsClampedAndReadFromStyles()
{
    const QByteArray styles = kStyles + QByteArray("<w:style w:type=\"character\" w:styleId=\"Boxed\"><w:name w:val=\"Boxed\"/><w:rPr><w:bdr w:val=\"dashed\" w:sz=\"6\" w:space=\"1\" w:color=\"FF0000\"/></w:rPr></w:style>"
                                                   "<w:style w:type=\"paragraph\" w:styleId=\"Framed\"><w:name w:val=\"Framed\"/><w:basedOn w:val=\"Normal\"/><w:rPr><w:bdr w:val=\"double\" w:sz=\"8\" w:space=\"0\"/></w:rPr></w:style>");
    auto run = [](const QByteArray &props, const char *text) { return "<w:r>" + (props.isEmpty() ? QByteArray() : "<w:rPr>" + props + "</w:rPr>") + "<w:t>" + QByteArray(text) + "</w:t></w:r>"; };
    const QByteArray body = "<w:p>" + run("<w:bdr w:val=\"single\" w:sz=\"999\" w:space=\"999\" w:color=\"auto\"/>", "wide") + "</w:p>"
                            "<w:p>" + run("<w:rStyle w:val=\"Boxed\"/>", "styled") + "</w:p>"
                            "<w:p><w:pPr><w:pStyle w:val=\"Framed\"/></w:pPr>" + run(QByteArray(), "inherited") + run("<w:bdr w:val=\"nil\"/>", "taken") + "</w:p>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(body, sect, {}, QByteArray(), styles));
    QVERIFY(doc);
    QHash<QString, QString> specs;
    for (QTextBlock b = doc->story()->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) specs.insert(it.fragment().text(), it.fragment().charFormat().stringProperty(tp::RunBorder));
    QCOMPARE(specs.value(QStringLiteral("wide")), QStringLiteral("single;96;31;"));
    QCOMPARE(specs.value(QStringLiteral("styled")), QStringLiteral("dashed;6;1;FF0000"));
    QCOMPARE(specs.value(QStringLiteral("inherited")), QStringLiteral("double;8;0;"));
    QCOMPARE(specs.value(QStringLiteral("taken")), QString());
}


// The tables of a header or a footer are laid out once for the header where nothing in them is a page's own (no page number or count): not for each page, which would cost each
// page the whole table and keep a copy of its layout for each. Where a field is, they are laid out for the pages that show it, and the copies kept are bounded by their cells.
void WriteTests::theTablesOfAHeaderAreLaidOutOnceUnlessFieldsMakeThemDiffer()
{
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>";
    auto table = [](int rows, int columns, const QByteArray &last) {
        QByteArray t = "<w:tbl><w:tblPr><w:tblW w:w=\"9000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid>";
        for (int c = 0; c < columns; ++c) t += "<w:gridCol w:w=\"" + QByteArray::number(9000 / columns) + "\"/>";
        t += "</w:tblGrid>";
        for (int r = 0; r < rows; ++r) {
            t += "<w:tr>";
            for (int c = 0; c < columns; ++c)
                t += "<w:tc><w:tcPr><w:tcW w:w=\"" + QByteArray::number(9000 / columns) + "\" w:type=\"dxa\"/></w:tcPr><w:p>" + (r == rows - 1 && c == columns - 1 ? last : QByteArray()) + "</w:p></w:tc>";
            t += "</w:tr>";
        }
        return t + "</w:tbl>";
    };
    const QByteArray pageField = "<w:r><w:t xml:space=\"preserve\">Page </w:t></w:r><w:fldSimple w:instr=\" PAGE \"><w:r><w:t>1</w:t></w:r></w:fldSimple>";
    // (A header of thirty cells with nothing of the page's in it; a footer of two hundred and forty with the page's number in one.)
    const QByteArray header = "<w:hdr " + QByteArray(kNs) + ">" + table(6, 5, "<w:r><w:t>cell</w:t></w:r>") + "<w:p/></w:hdr>";
    const QByteArray footer = "<w:ftr " + QByteArray(kNs) + ">" + table(6, 40, pageField) + "<w:p/></w:ftr>";
    auto doc = open(makeDocxWithParts(paragraphs(300, 30), sect, {{QStringLiteral("/word/header1.xml"), header}, {QStringLiteral("/word/footer1.xml"), footer}}));
    QVERIFY(doc);
    const int pages = doc->pager().pageCount();
    QVERIFY2(pages >= 20 && pages <= 200, qPrintable(QString::number(pages)));
    for (int k = 0; k < pages; ++k) QVERIFY(doc->pager().materialize(k));
    // The header's table is laid out once; the footer's, which shows the page's number, once for each page (as the page is laid out and as it is drawn it is the same).
    const qint64 made = doc->pager().headerTablesLaidOutForTest();
    QVERIFY2(made <= pages + 2, qPrintable(QStringLiteral("%1 layouts of the tables for %2 pages").arg(made).arg(pages)));
    QVERIFY2(made >= pages, qPrintable(QStringLiteral("%1 layouts for %2 pages: the footer's is laid out for each page's number").arg(made).arg(pages)));
    // The copies kept are bounded by their cells (twenty pages and more of two hundred and forty cells are not all held).
    QVERIFY2(doc->pager().headerTableCellsHeldForTest() <= 2000, qPrintable(QStringLiteral("%1 cells held").arg(doc->pager().headerTableCellsHeldForTest())));
}


// The objects a header's paragraphs anchor are drawn also where the header has a table, and a shape the text goes round holds the page's text below it as it does without one.
void WriteTests::anObjectInAHeaderWithATableIsDrawnAndHoldsTheTextBelowIt()
{
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"720\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"360\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray shape = "<w:r><w:pict><v:shapetype id=\"t202\" coordsize=\"21600,21600\" o:spt=\"202\" path=\"m,l,21600r21600,l21600,xe\"/>"
                             "<v:shape id=\"s1\" type=\"#t202\" style=\"position:absolute;margin-left:-20pt;margin-top:0;width:100pt;height:150pt;mso-position-vertical-relative:page;z-index:1\" fillcolor=\"red\" stroked=\"f\">"
                             "<v:textbox><w:txbxContent><w:p><w:r><w:t>SIDE</w:t></w:r></w:p></w:txbxContent></v:textbox><w10:wrap type=\"square\"/></v:shape></w:pict></w:r>";
    const QByteArray table = "<w:tbl><w:tblPr><w:tblW w:w=\"8000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"8000\"/></w:tblGrid><w:tr><w:tc><w:tcPr><w:tcW w:w=\"8000\" w:type=\"dxa\"/></w:tcPr>"
                             "<w:p><w:r><w:t>TABLECELL</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
    auto drawnTop = [&](bool withTable, bool *drawn) {
        QByteArray header = "<w:hdr " + QByteArray(kNs) + ">" + (withTable ? table : QByteArray()) + "<w:p>" + shape + "<w:r><w:t>after</w:t></w:r></w:p></w:hdr>";
        header.replace("<w:hdr ", "<w:hdr xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\" ");
        auto doc = open(makeDocxWithParts(paragraphs(10, 30), sect, {{QStringLiteral("/word/header1.xml"), header}}));
        if (!doc) return -1.0;
        const auto page = doc->pager().materialize(0);
        *drawn = false;
        if (page)
            for (const auto &item : page->doc->pages[0]->items) *drawn |= item->id.contains(QLatin1String("flow-object"));
        return doc->pager().page(0).content.top();
    };
    bool plainDrawn = false, tabledDrawn = false;
    const double plain = drawnTop(false, &plainDrawn), tabled = drawnTop(true, &tabledDrawn);
    QVERIFY2(plainDrawn, "the shape of a header without a table is drawn");
    QVERIFY2(tabledDrawn, "and so is the shape of a header with one");
    // (The shape reaches a hundred and fifty points down the page; the text is held below it whether or not the header has a table.)
    QVERIFY2(plain >= 150 && tabled >= 150, qPrintable(QStringLiteral("the text begins at %1 without a table, at %2 with one").arg(plain).arg(tabled)));
}


// SECTIONPAGES in a cell of a header's table is the page count of the section the page is in, not the document's.
void WriteTests::theSectionPagesOfAHeadersTableAreTheSectionsOwn()
{
    const QByteArray one = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray two = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray header = "<w:hdr " + QByteArray(kNs) + "><w:tbl><w:tblPr><w:tblW w:w=\"8000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"8000\"/></w:tblGrid><w:tr><w:tc><w:tcPr><w:tcW w:w=\"8000\" w:type=\"dxa\"/></w:tcPr>"
                              "<w:p><w:r><w:t xml:space=\"preserve\">In section: </w:t></w:r><w:fldSimple w:instr=\" SECTIONPAGES \"><w:r><w:t>1</w:t></w:r></w:fldSimple></w:p></w:tc></w:tr></w:tbl><w:p/></w:hdr>";
    const QByteArray body = paragraphs(120, 30) + "<w:p><w:pPr>" + one + "</w:pPr></w:p>" + paragraphs(40, 30);
    auto doc = open(makeDocxWithParts(body, two, {{QStringLiteral("/word/header1.xml"), header}}));
    QVERIFY(doc);
    const int pages = doc->pager().pageCount();
    int inFirst = 0;
    for (int k = 0; k < pages; ++k) inFirst += doc->pager().page(k).start.section == 0;
    QVERIFY2(inFirst >= 2 && pages > inFirst + 1, qPrintable(QStringLiteral("%1 pages, %2 of them in the first section").arg(pages).arg(inFirst)));
    auto shown = [&](int k) {
        const auto page = doc->pager().materialize(k);
        QString text;
        if (page)
            for (const auto &t : page->furnitureTables) t->forTexts([&](const StoryLayout &layout) { for (const auto &l : layout.lineInfo(0)) text += l.text; });
        return text;
    };
    QVERIFY2(shown(0).contains(QStringLiteral("In section: %1").arg(inFirst)), qPrintable(shown(0) + QStringLiteral(" (the first section has %1 pages)").arg(inFirst)));
    const int last = pages - 1, inSecond = pages - inFirst;
    QVERIFY2(shown(last).contains(QStringLiteral("In section: %1").arg(inSecond)), qPrintable(shown(last) + QStringLiteral(" (the second section has %1 pages)").arg(inSecond)));
}


// A table in a header or a footer is laid out and drawn: its cells' text (a page's number in one of them is the page's own), and the room it takes at the top of the page (or the
// foot of it), under the header's text and over the footer's.
void WriteTests::aTableInAHeaderOrAFooterIsDrawnAndTakesRoom()
{
    const QByteArray sect = "<w:sectPr><w:headerReference w:type=\"default\" r:id=\"rIdheader1\"/><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"720\" w:right=\"1440\" w:bottom=\"720\" w:left=\"1440\" w:header=\"360\" w:footer=\"360\" w:gutter=\"0\"/></w:sectPr>";
    auto cell = [](const QByteArray &content) { return "<w:tc><w:tcPr><w:tcW w:w=\"4000\" w:type=\"dxa\"/></w:tcPr><w:p>" + content + "</w:p></w:tc>"; };
    auto table = [&](const QByteArray &left, const QByteArray &right) {
        return "<w:tbl><w:tblPr><w:tblW w:w=\"8000\" w:type=\"dxa\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"4000\"/><w:gridCol w:w=\"4000\"/></w:tblGrid><w:tr>" + cell(left) + cell(right) + "</w:tr></w:tbl>";
    };
    const QByteArray field = "<w:r><w:t xml:space=\"preserve\">Page </w:t></w:r><w:fldSimple w:instr=\" PAGE \"><w:r><w:t>1</w:t></w:r></w:fldSimple><w:r><w:t xml:space=\"preserve\"> of </w:t></w:r>"
                             "<w:fldSimple w:instr=\" NUMPAGES \"><w:r><w:t>1</w:t></w:r></w:fldSimple>";
    // (A header is a table and an empty paragraph after it, as a file's is; a footer a table only.)
    const QByteArray header = "<w:hdr " + QByteArray(kNs) + ">" + table("<w:r><w:t>LEFTCELL</w:t></w:r>", field) + "<w:p/></w:hdr>";
    const QByteArray footer = "<w:ftr " + QByteArray(kNs) + ">" + table("<w:r><w:t>FOOTCELL</w:t></w:r>", "<w:r><w:t>right</w:t></w:r>") + "<w:p/></w:ftr>";
    auto doc = open(makeDocxWithParts(paragraphs(300, 30), sect, {{QStringLiteral("/word/header1.xml"), header}, {QStringLiteral("/word/footer1.xml"), footer}}));
    QVERIFY(doc);
    QVERIFY(doc->pager().pageCount() >= 3);
    const auto page = doc->pager().materialize(1);
    QVERIFY(page);
    QCOMPARE(int(page->furnitureTables.size()), 2);
    auto textOf = [](const flow::TableFragment &fragment) {
        QStringList lines;
        fragment.forTexts([&](const StoryLayout &text) { for (const auto &l : text.lineInfo(0)) lines << l.text; });
        return lines.join(QLatin1Char('|'));
    };
    const flow::TableFragment *top = nullptr, *foot = nullptr;
    for (const auto &t : page->furnitureTables) (textOf(*t).contains(QLatin1String("LEFTCELL")) ? top : foot) = t.get();
    QVERIFY2(top && foot, "a table in the header and one in the footer");
    // The header's table is at the header's distance from the top, with the page's own number in a cell (this is the second page's).
    QVERIFY2(std::abs(top->rect().top() - 18) < 0.01, qPrintable(QString::number(top->rect().top())));
    QVERIFY2(textOf(*top).contains(QStringLiteral("Page 2 of %1").arg(doc->pager().pageCount())), qPrintable(textOf(*top) + QStringLiteral(" (the document has %1 pages)").arg(doc->pager().pageCount())));
    QVERIFY2(textOf(*foot).contains(QLatin1String("FOOTCELL")), qPrintable(textOf(*foot)));
    // The text begins under the header (a line of the empty paragraph after the table too) and ends over the footer's table.
    const QRectF content = doc->pager().page(1).content;
    QVERIFY2(content.top() >= top->rect().bottom() + 10, qPrintable(QStringLiteral("the text begins at %1, the table ends at %2").arg(content.top()).arg(top->rect().bottom())));
    QVERIFY2(content.bottom() <= foot->rect().top() + 0.01, qPrintable(QStringLiteral("the text ends at %1, the footer's table begins at %2").arg(content.bottom()).arg(foot->rect().top())));
}


// A paragraph of a footer whose mark a tracked change deleted, with nothing else of its own, is gone from the footer as the text stands: it takes no room
// (twenty points a line here), so the text keeps what the footer leaves it.
void WriteTests::aDeletedParagraphOfAFooterTakesNoRoom()
{
    const QByteArray sect = "<w:sectPr><w:footerReference w:type=\"default\" r:id=\"rIdfooter1\"/><w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
                            "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"720\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray spacing = "<w:spacing w:before=\"0\" w:after=\"0\" w:line=\"400\" w:lineRule=\"exact\"/>";
    const QByteArray gone = "<w:pPr>" + spacing + "<w:rPr><w:del w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"/></w:rPr></w:pPr>"
                            "<w:del w:id=\"8\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"><w:r><w:delText>gone</w:delText></w:r></w:del>";
    const QByteArray footer = headerPart("ftr", {"<w:pPr>" + spacing + "</w:pPr><w:r><w:t>ONE</w:t></w:r>", gone, "<w:pPr>" + spacing + "</w:pPr><w:r><w:t>TWO</w:t></w:r>"});
    auto doc = open(makeDocxWithParts(paragraphs(60, 40), sect, {{QStringLiteral("/word/footer1.xml"), footer}}));
    QVERIFY(doc);
    // Two lines of twenty points under the footer distance of thirty-six: the text ends at 792 - 36 - 40.
    QVERIFY2(std::abs(doc->pager().page(0).content.bottom() - (792 - 36 - 40)) < 0.01, qPrintable(QString::number(doc->pager().page(0).content.bottom())));
}


// A note that goes on past its page has the file's continuation notice under its first part, and the part on the next page has the file's own separator for it above it
// (its text, and its rule as wide as its character makes it: the column's for a continuation separator, two inches for the plain separator).
void WriteTests::aNoteThatGoesOnHasTheFilesNoticeAndSeparatorSet()
{
    QByteArray words;
    for (int i = 0; i < 1500; ++i) words += "wordy ";
    const QByteArray body = "<w:p><w:r><w:t>Text with a long note</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r><w:r><w:t xml:space=\"preserve\"> and more words.</w:t></w:r></w:p>";
    for (const char *character : {"continuationSeparator", "separator"}) {
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                                 "<w:footnote w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:" + character + "/></w:r></w:p><w:p><w:r><w:t>(Resumed from before)</w:t></w:r></w:p></w:footnote>"
                                 "<w:footnote w:type=\"continuationNotice\" w:id=\"1\"><w:p><w:r><w:t>(goes on below)</w:t></w:r></w:p></w:footnote>"
                                 "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> " + words + "</w:t></w:r></w:p></w:footnote></w:footnotes>";
        auto doc = open(makeDocxWithParts(body, QByteArray(), {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(doc);
        QVERIFY(doc->pager().pageCount() >= 2);
        const auto first = doc->pager().materialize(0), second = doc->pager().materialize(1);
        QVERIFY(first && second);
        QVERIFY2(boxWithText(*first, QStringLiteral("(goes on below)")), "the notice is under the first part of the note");
        QVERIFY2(!boxWithText(*first, QStringLiteral("(Resumed from before)")), "the first part has no separator for a part that goes on");
        QVERIFY2(boxWithText(*second, QStringLiteral("(Resumed from before)")), "the part on the next page has the file's separator above it");
        double width = -1;
        for (const auto &item : second->doc->pages[0]->items)
            if (const auto *line = dynamic_cast<const LineItem *>(item.get())) width = std::max(width, line->rect.width());
        const double column = 612 - 2 * 72;
        if (QByteArray(character) == "separator") QVERIFY2(std::abs(width - 144) < 0.5, qPrintable(QStringLiteral("the rule is %1 wide, not two inches").arg(width)));
        else QVERIFY2(std::abs(width - column) < 0.5, qPrintable(QStringLiteral("the rule is %1 wide, not the column's %2").arg(width).arg(column)));
    }
}


// The pages' notice and separator for a note that goes on are set in the room the layout kept for them (a quarter of the page's text area at most), however many
// paragraphs the file's are, with the rule where the file's character is (the last of the forty paragraphs), kept inside that room, in a page that has a table too.
void WriteTests::aLongNoticeAndSeparatorAreSetInTheRoomThatWasKept()
{
    QByteArray words;
    for (int i = 0; i < 1500; ++i) words += "wordy ";
    auto paragraphsOf = [](const char *label, int n, const QByteArray &first = QByteArray()) {
        QByteArray out = first;
        for (int i = 0; i < n; ++i) out += "<w:p><w:r><w:t>(" + QByteArray(label) + " " + QByteArray::number(i) + ")</w:t></w:r></w:p>";
        return out;
    };
    for (const bool table : {false, true}) {
        const QByteArray cell = "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr><w:tblGrid><w:gridCol w:w=\"4000\"/></w:tblGrid><w:tr><w:tc><w:p><w:r><w:t>cell</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";
        const QByteArray body = (table ? cell : QByteArray()) + "<w:p><w:r><w:t>Text with a long note</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r><w:r><w:t xml:space=\"preserve\"> and more words.</w:t></w:r></w:p>";
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                                 "<w:footnote w:type=\"continuationSeparator\" w:id=\"0\">" + paragraphsOf("Resumed", 40) + "<w:p><w:r><w:separator/></w:r></w:p>" + "</w:footnote>"
                                 "<w:footnote w:type=\"continuationNotice\" w:id=\"1\">" + paragraphsOf("Goes", 40) + "</w:footnote>"
                                 "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> " + words + "</w:t></w:r></w:p></w:footnote></w:footnotes>";
        auto doc = open(makeDocxWithParts(body, QByteArray(), {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(doc);
        QVERIFY(doc->pager().pageCount() >= 2);
        const auto first = doc->pager().materialize(0), second = doc->pager().materialize(1);
        QVERIFY(first && second);
        const double top = 72, bottom = 792 - 72, limit = (bottom - top) / 4;
        const TextItem *notice = boxWithText(*first, QStringLiteral("(Goes 0)"));
        QVERIFY(notice);
        QVERIFY2(notice->rect.bottom() <= bottom + 0.5 && notice->rect.height() <= limit + 0.5,
                 qPrintable(QStringLiteral("the notice is %1 tall and ends at %2 (the text area ends at %3, and keeps %4 for it)").arg(notice->rect.height()).arg(notice->rect.bottom()).arg(bottom).arg(limit)));
        const TextItem *separator = boxWithText(*second, QStringLiteral("(Resumed 0)"));
        QVERIFY(separator);
        QVERIFY2(separator->rect.top() >= top - 0.01 && separator->rect.height() <= limit + 0.5,
                 qPrintable(QStringLiteral("the separator is %1 tall and begins at %2 (the text area begins at %3, and keeps %4 for it)").arg(separator->rect.height()).arg(separator->rect.top()).arg(top).arg(limit)));
        bool ruled = false;
        for (const auto &item : second->doc->pages[0]->items)
            if (const auto *line = dynamic_cast<const LineItem *>(item.get())) {
                ruled = true;
                QVERIFY2(line->rect.top() >= separator->rect.top() - 0.01 && line->rect.top() <= separator->rect.bottom() + 0.01,
                         qPrintable(QStringLiteral("the rule is at %1, outside the separator's room (%2 to %3)").arg(line->rect.top()).arg(separator->rect.top()).arg(separator->rect.bottom())));
            }
        QVERIFY(ruled);
    }
}


// The rule over a note's part that goes on begins after the words that stand before the separator's character in its line, where the layout has them (by the word processor's
// widths), and never reaches out of the column.
void WriteTests::theRuleOverAContinuedNoteBeginsAfterTheWordsBeforeItsCharacter()
{
    QByteArray words;
    for (int i = 0; i < 1500; ++i) words += "wordy ";
    const QByteArray body = "<w:p><w:r><w:t>Text with a long note</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
    for (const QByteArray &before : {QByteArray("(Resumed from before) "), QByteArray("(Resumed from before and on and on and on and on and on and on and on and on and on and on and on and on and on and on and on and on and on) ")}) {
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                                 "<w:footnote w:type=\"continuationSeparator\" w:id=\"0\"><w:p><w:r><w:t xml:space=\"preserve\">" + before + "</w:t></w:r><w:r><w:separator/></w:r></w:p></w:footnote>"
                                 "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> " + words + "</w:t></w:r></w:p></w:footnote></w:footnotes>";
        auto doc = open(makeDocxWithParts(body, QByteArray(), {{QStringLiteral("/word/footnotes.xml"), notes}}));
        QVERIFY(doc);
        QVERIFY(doc->pager().pageCount() >= 2);
        const auto second = doc->pager().materialize(1);
        QVERIFY(second);
        const TextItem *box = boxWithText(*second, QStringLiteral("(Resumed from before"));
        QVERIFY(box);
        const LineItem *rule = nullptr;
        for (const auto &item : second->doc->pages[0]->items)
            if (const auto *line = dynamic_cast<const LineItem *>(item.get())) rule = line;
        QVERIFY(rule);
        const double left = 72, right = 612 - 72;
        QVERIFY2(rule->rect.width() >= 0 && rule->rect.left() >= left - 0.01 && rule->rect.right() <= right + 0.01,
                 qPrintable(QStringLiteral("the rule is from %1 to %2, outside the column (%3 to %4)").arg(rule->rect.left()).arg(rule->rect.right()).arg(left).arg(right)));
        // Where the layout ends the words before the character in one line, in the story the page sets.
        FrameSpec fs;
        fs.size = QSizeF(1e5, 10000);
        fs.insets = QMarginsF(0, 0, 0, 0);
        fs.hyphenate = false;
        StoryLayout layout;
        layout.build(second->doc->story(box->storyId)->doc.get(), {fs}, doc->flow().env);
        int frame = -1;
        QRectF at;
        QVERIFY(layout.caretRect(before.size(), &frame, &at));
        const double expected = std::min(left + at.x() + 3, right);
        QVERIFY2(std::abs(rule->rect.left() - expected) < 0.3, qPrintable(QStringLiteral("the rule begins at %1, where the words end in the layout is %2").arg(rule->rect.left()).arg(expected)));
    }
}


// A file's notice for a note that goes on is bounded when it is read: a notice of thousands of paragraphs is the first few of them (the pages set a quarter of a page of it at
// most), so that each page's copy of it costs the same.
void WriteTests::aHugeNoticeIsBoundedWhenItIsRead()
{
    QByteArray notice;
    for (int i = 0; i < 20000; ++i) notice += "<w:p><w:r><w:t>(goes on " + QByteArray::number(i) + ")</w:t></w:r></w:p>";
    QByteArray words;
    for (int i = 0; i < 1500; ++i) words += "wordy ";
    const QByteArray body = "<w:p><w:r><w:t>Text with a long note</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                             "<w:footnote w:type=\"continuationNotice\" w:id=\"1\">" + notice + "</w:footnote>"
                             "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> " + words + "</w:t></w:r></w:p></w:footnote></w:footnotes>";
    auto doc = open(makeDocxWithParts(body, QByteArray(), {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    const QTextDocument *kept = doc->flow().fields ? doc->flow().fields->noteNoticeStory() : nullptr;
    QVERIFY(kept);
    QVERIFY2(kept->blockCount() <= 64, qPrintable(QStringLiteral("the notice has %1 paragraphs").arg(kept->blockCount())));
    QVERIFY(doc->pager().pageCount() >= 2);
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    const TextItem *box = boxWithText(*page, QStringLiteral("(goes on 0)"));
    QVERIFY(box);
    const int blocks = page->doc->story(box->storyId)->doc->blockCount();
    QVERIFY2(blocks <= 64, qPrintable(QStringLiteral("each page copies %1 paragraphs of the notice").arg(blocks)));
}


// A symbol of a symbol font in a note that is not a blank (a check mark) is shown, in that font, as one in the text is.
void WriteTests::aVisibleSymbolInANoteIsKept()
{
    const QByteArray body = "<w:p><w:r><w:t>Text with a note</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:sym w:font=\"Wingdings\" w:char=\"F0FC\"/></w:r><w:r><w:t xml:space=\"preserve\"> NOTEWORDS here.</w:t></w:r></w:p></w:footnote></w:footnotes>";
    auto doc = open(makeDocxWithParts(body, QByteArray(), {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    const TextItem *note = page ? boxWithText(*page, QStringLiteral("NOTEWORDS")) : nullptr;
    QVERIFY(note);
    bool kept = false;
    for (QTextBlock b = page->doc->story(note->storyId)->doc->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it)
            kept |= it.fragment().charFormat().stringProperty(tp::DisplayFamily) == QLatin1String("Wingdings");
    QVERIFY2(kept, "the symbol of the symbol font is in the note");
}


// A note whose text begins with a raised blank of the symbol font (the mark an older program wrote) is as tall as that symbol makes its first line: a little under a
// point more for 10-point text.
void WriteTests::aNoteThatBeginsWithARaisedSymbolIsTallerByWhatItReaches()
{
    const QByteArray body = "<w:p><w:r><w:t>Text with a note</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r><w:r><w:t xml:space=\"preserve\"> and more words.</w:t></w:r></w:p>";
    const QByteArray font = "<w:rFonts w:ascii=\"Times New Roman\" w:hAnsi=\"Times New Roman\"/><w:sz w:val=\"20\"/>";
    auto height = [&](const QByteArray &mark) {
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                                 "<w:footnote w:id=\"2\"><w:p>" + mark + "<w:r><w:rPr>" + font + "</w:rPr><w:t xml:space=\"preserve\"> NOTEWORDS here.</w:t></w:r></w:p></w:footnote></w:footnotes>";
        auto doc = open(makeDocxWithParts(body, QByteArray(), {{QStringLiteral("/word/footnotes.xml"), notes}}));
        if (!doc) return -1.0;
        const auto page = doc->pager().materialize(0);
        const TextItem *note = page ? boxWithText(*page, QStringLiteral("NOTEWORDS")) : nullptr;
        return note ? note->rect.height() : -1.0;
    };
    const double plain = height("<w:r><w:rPr>" + font + "</w:rPr><w:t></w:t></w:r>");
    const double symbol = height("<w:r><w:rPr>" + font + "<w:vertAlign w:val=\"superscript\"/></w:rPr><w:sym w:font=\"Symbol\" w:char=\"F020\"/></w:r>");
    QVERIFY(plain > 8);
    QVERIFY2(symbol - plain > 0.5 && symbol - plain < 1.1, qPrintable(QStringLiteral("a raised symbol made the note %1 points taller, not about 0.8").arg(symbol - plain)));
}


// A file's footnotes are at the foot of the pages that cite them: the note in a box at the bottom of the text area with a rule above it, its mark
// the note's number, and the reference in the text showing the same number; saving keeps the part and the reference as they were.
void WriteTests::aFilesFootnotesAreAtTheFootOfItsPages()
{
    const QByteArray body = "<w:p><w:r><w:t>First paragraph with a note</w:t></w:r><w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteReference w:id=\"2\"/></w:r><w:r><w:t xml:space=\"preserve\"> and more words after it.</w:t></w:r></w:p>" + paragraphs(40, 40);
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>"
                             "<w:footnote w:id=\"2\"><w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> FOOTNOTE TWO says something.</w:t></w:r></w:p></w:footnote></w:footnotes>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    const QByteArray zip = makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}});
    auto doc = open(zip);
    QVERIFY(doc);
    QVERIFY(doc->pager().pageCount() >= 2);
    const auto page = doc->pager().materialize(0);
    const TextItem *note = boxWithText(*page, QStringLiteral("FOOTNOTE TWO"));
    QVERIFY(note);
    const double bottom = 792 - 72;
    QVERIFY2(std::abs(note->rect.bottom() - bottom) < 0.1, qPrintable(QString::number(note->rect.bottom())));
    // The mark that begins the note and the reference in the text show the number.
    auto showsOne = [&](const QString &text) {
        const TextItem *box = boxWithText(*page, text);
        if (!box) return false;
        for (QTextBlock b = page->doc->story(box->storyId)->doc->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it)
                if (it.fragment().charFormat().stringProperty(tp::Field) == QLatin1String("raw:1")) return true;
        return false;
    };
    QVERIFY(showsOne(QStringLiteral("FOOTNOTE TWO")));
    QVERIFY(showsOne(QStringLiteral("First paragraph")));
    bool rule = false;
    for (const auto &item : page->doc->pages[0]->items) rule |= dynamic_cast<const LineItem *>(item.get()) != nullptr;
    QVERIFY(rule);
    // Kept as it was: the part, and the reference in the paragraph.
    const QByteArray saved = doc->toDocx();
    office::Package before, after;
    before.read(zip);
    after.read(saved);
    QCOMPARE(after.part(QStringLiteral("/word/footnotes.xml")), before.part(QStringLiteral("/word/footnotes.xml")));
    QCOMPARE(int(documentXml(saved).count("footnoteReference")), 1);
    // A page without the reference has no note.
    const auto next = doc->pager().materialize(1);
    QVERIFY(!boxWithText(*next, QStringLiteral("FOOTNOTE TWO")));
}


// The empty paragraphs at the start of a note whose marks a tracked change deleted are gone from it as the text stands: the note is as tall as its own text, and the
// notes' room at the foot of the page is that.
void WriteTests::deletedEmptyParagraphsAtTheStartOfANoteTakeNoRoom()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto noteTop = [&](int deleted) {
        const QByteArray gone = "<w:p><w:pPr><w:rPr><w:del w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"/></w:rPr></w:pPr></w:p>";
        QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote><w:footnote w:id=\"2\">";
        for (int i = 0; i < deleted; ++i) notes += gone;
        notes += "<w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> FOOTNOTE TWO says something.</w:t></w:r></w:p></w:footnote></w:footnotes>";
        const QByteArray body = "<w:p><w:r><w:t>First paragraph with a note</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>" + paragraphs(40, 40);
        auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
        if (!doc) return -1.0;
        const auto page = doc->pager().materialize(0);
        // Where the last line of the page's text ends: the notes' room is what is left under it.
        const auto lines = doc->pager().lines(0);
        return page && !lines.isEmpty() ? lines.last().rect.bottom() : -1.0;
    };
    const double plain = noteTop(0), withGone = noteTop(3);
    QVERIFY(plain > 0 && withGone > 0);
    QVERIFY2(std::abs(plain - withGone) < 0.01, qPrintable(QStringLiteral("the text ends at %1 with the note alone, at %2 after three deleted paragraphs").arg(plain).arg(withGone)));
}


// The note as the page draws it has the first paragraph's mark as the file has it: one that a tracked change deleted shows nothing, and the note is drawn from its text
// (the copy of a story loses the mark of an empty first paragraph unless it is put back).
void WriteTests::aNoteOnThePageKeepsTheDeletedMarkOfItsFirstParagraph()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote><w:footnote w:id=\"2\">"
                       "<w:p><w:pPr><w:rPr><w:del w:id=\"7\" w:author=\"a\" w:date=\"2020-01-01T00:00:00Z\"/></w:rPr></w:pPr></w:p>"
                       "<w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> FOOTNOTE TWO says something.</w:t></w:r></w:p></w:footnote></w:footnotes>";
    const QByteArray body = "<w:p><w:r><w:t>First paragraph with a note</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>" + paragraphs(40, 40);
    auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    const auto page = doc->pager().materialize(0);
    QVERIFY(page);
    const TextItem *note = boxWithText(*page, QStringLiteral("FOOTNOTE TWO"));
    QVERIFY(note);
    const auto story = page->doc->story(note->storyId);
    QVERIFY(story && story->doc);
    QVERIFY2(story->doc->begin().charFormat().boolProperty(tp::HiddenText), "the first paragraph's mark is hidden in the note as drawn");
}


// Only a header's or a footer's frame set on the page is a text box there: a paragraph of a note that has the same frame properties is a paragraph of the note, which keeps its
// text (a note has no page to set a box on).
void WriteTests::aFramePropertyOfAPageInANoteChangesNothing()
{
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto noteBox = [&](bool framed) -> std::pair<bool, double> {
        const QByteArray first = framed ? "<w:p><w:pPr><w:framePr w:w=\"9360\" w:h=\"280\" w:hRule=\"exact\" w:wrap=\"notBeside\" w:vAnchor=\"page\" w:hAnchor=\"text\" w:y=\"100\"/></w:pPr><w:r><w:t>FRAMED NOTE</w:t></w:r></w:p>"
                                        : "<w:p><w:r><w:t>FRAMED NOTE</w:t></w:r></w:p>";
        const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:r><w:separator/></w:r></w:p></w:footnote><w:footnote w:id=\"2\">" + first +
                                 "<w:p><w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> the rest of the note.</w:t></w:r></w:p></w:footnote></w:footnotes>";
        const QByteArray body = "<w:p><w:r><w:t>First paragraph with a note</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>" + paragraphs(40, 40);
        auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
        if (!doc) return {false, 0};
        const auto page = doc->pager().materialize(0);
        const TextItem *box = page ? boxWithText(*page, QStringLiteral("FRAMED NOTE")) : nullptr;
        const Story *story = box ? page->doc->story(box->storyId) : nullptr;
        return {story && !story->doc->begin().blockFormat().boolProperty(tp::OutOfLayout), box ? box->rect.height() : 0.0};
    };
    const auto framed = noteBox(true), plain = noteBox(false);
    QVERIFY2(plain.first && plain.second > 0, "the plain note is on the page");
    QVERIFY2(framed.first, "the note's first paragraph is set apart from its text");
    QVERIFY2(std::abs(framed.second - plain.second) < 0.01, qPrintable(QStringLiteral("a note of %1 points, %2 without the frame properties").arg(framed.second).arg(plain.second)));
}


// The room between the text and the notes is the file's own separator paragraph, not a line of the Normal style: a separator thirty points tall
// (exactly) leaves the text fifty lines of twelve points above a note one line tall, where the Normal style's line would leave it one more.
void WriteTests::theNoteSeparatorIsTheFilesOwnParagraph()
{
    const QByteArray exact = "<w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr>";
    QByteArray body = "<w:p>" + exact + "<w:r><w:t>Cited</w:t></w:r><w:r><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
    for (int i = 0; i < 120; ++i) body += "<w:p>" + exact + "<w:r><w:t>x</w:t></w:r></w:p>";
    const QByteArray notes = "<w:footnotes " + QByteArray(kNs) + "><w:footnote w:type=\"separator\" w:id=\"-1\"><w:p><w:pPr><w:spacing w:before=\"0\" w:after=\"0\" w:line=\"600\" w:lineRule=\"exact\"/></w:pPr>"
                             "<w:r><w:separator/></w:r></w:p></w:footnote><w:footnote w:id=\"2\"><w:p>" + exact + "<w:r><w:footnoteRef/></w:r><w:r><w:t xml:space=\"preserve\"> Note.</w:t></w:r></w:p></w:footnote></w:footnotes>";
    const QByteArray sect = "<w:sectPr><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>";
    auto doc = open(makeDocxWithParts(body, sect, {{QStringLiteral("/word/footnotes.xml"), notes}}));
    QVERIFY(doc);
    const auto lines = doc->pager().lines(0);
    QVERIFY(lines.size() > 10);
    QVERIFY2(std::abs(lines.last().rect.bottom() - lines.first().rect.top() - 50 * 12) < 0.01, qPrintable(QString::number(lines.last().rect.bottom() - lines.first().rect.top())));
}


// Automatic spacing (the 14 points of a web page's paragraphs) is none between the items of one list, of whatever level, and 14 where a list follows another.
void WriteTests::autoSpacingIsNoneInAListAndFourteenBetweenLists()
{
    const QByteArray numbering = "<w:abstractNum w:abstractNumId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"o\"/></w:lvl>"
                                 "<w:lvl w:ilvl=\"1\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/><w:lvlText w:val=\"o\"/></w:lvl></w:abstractNum>"
                                 "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num><w:num w:numId=\"2\"><w:abstractNumId w:val=\"0\"/></w:num>";
    auto item = [](int num, int level, const char *text) {
        return "<w:p><w:pPr><w:numPr><w:ilvl w:val=\"" + QByteArray::number(level) + "\"/><w:numId w:val=\"" + QByteArray::number(num) + "\"/></w:numPr>"
               "<w:spacing w:before=\"100\" w:beforeAutospacing=\"1\" w:after=\"100\" w:afterAutospacing=\"1\" w:line=\"240\" w:lineRule=\"exact\"/></w:pPr><w:r><w:t>" + QByteArray(text) + "</w:t></w:r></w:p>";
    };
    auto doc = open(makeDocx(item(1, 0, "a") + item(1, 1, "b") + item(1, 0, "c") + item(2, 0, "d"), kStyles, numbering));
    QVERIFY(doc);
    const auto lines = doc->pager().lines(0);
    QCOMPARE(int(lines.size()), 4);
    // (Lines of twelve points exactly: the items of one list follow one another, and the next list has the fourteen points between.)
    QVERIFY2(std::abs(lines[1].rect.top() - lines[0].rect.top() - 12) < 0.01, qPrintable(QString::number(lines[1].rect.top() - lines[0].rect.top())));
    QVERIFY2(std::abs(lines[2].rect.top() - lines[1].rect.top() - 12) < 0.01, qPrintable(QString::number(lines[2].rect.top() - lines[1].rect.top())));
    QVERIFY2(std::abs(lines[3].rect.top() - lines[2].rect.top() - 26) < 0.01, qPrintable(QString::number(lines[3].rect.top() - lines[2].rect.top())));
}


// A line of a word processor's text is not broken after a slash ("and/or", an address): it breaks at spaces and hyphens, and a word longer than the line anywhere.
void WriteTests::aLineIsNotBrokenAfterASlash()
{
    const char *words[] = {"alpha/beta/gamma", "delta/epsilon", "zeta/eta/theta/iota", "kappa", "lambda/mu"};
    QByteArray text;
    for (int i = 0; i < 90; ++i) text += QByteArray(words[(i * 7) % 5]) + " ";
    auto doc = open(makeDocx("<w:p><w:r><w:t xml:space=\"preserve\">" + text + "</w:t></w:r></w:p>", kStyles));
    QVERIFY(doc);
    const auto lines = doc->pager().lines(0);
    QVERIFY(lines.size() > 8);
    const QString plain = doc->story()->toPlainText();
    for (const auto &l : lines) {
        const QString shown = plain.mid(l.pos, l.end - l.pos).trimmed();
        QVERIFY2(!shown.endsWith(QLatin1Char('/')), qPrintable(shown));
    }
}
