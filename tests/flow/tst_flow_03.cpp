#include "tst_flow.h"


// The ligatures of a word processor's text are the file's to ask for (tp::Ligatures on a run that has them: the standard ones, or none), and its letters of
// the scripts that are shaped with ligatures (Arabic) are not touched by the default of none: a paragraph that has one keeps its font's.
void FlowTests::wordLigaturesFollowTheRunAndTheScript()
{
    QVERIFY2(QFontDatabase::hasFamily(QStringLiteral("Carlito")), "the face the program carries is not registered");
    auto width = [&](const QString &text, bool word, int ligatures) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Carlito")});
        run.setFontPointSize(24);
        if (ligatures >= 0) run.setProperty(tp::Ligatures, ligatures);
        c.insertText(text, run);
        LayoutEnv env;
        env.wordLines = word;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(1000, 100)}, env);
        const auto rects = lay.rangeRects(0, 0, 2);
        return rects.isEmpty() ? -1.0 : rects.last().right() - rects.first().left();
    };
    const QString ti = QStringLiteral("ti");
    const double apart = width(ti, true, -1), joined = width(ti, false, -1);
    QVERIFY2(std::abs(apart - joined) > 0.05, "the face has no ligature for ti");
    QVERIFY(std::abs(width(ti, true, 0) - joined) < 0.01);      // (the file asks for the standard ligatures)
    QVERIFY(std::abs(width(ti, true, 1) - apart) < 0.01);       // (and for none)
    QVERIFY(std::abs(width(ti + QChar(0x0628), true, -1) - joined) < 0.01);   // (an Arabic letter in the paragraph: the font's own)
    QVERIFY(std::abs(width(ti + QStringLiteral(" \u0393\u0431"), true, -1) - apart) < 0.01);   // (Greek and Cyrillic are Latin's kind)
}


// A mark of punctuation at the end of a line may hang past the margin in a word processor's text: a word that fits but for its period is the last
// of the line, unless the file says no punctuation hangs; a word that is itself too long, or a mark that isn't one that hangs, is not let.
void FlowTests::punctuationAtTheEndOfALineHangsPastTheMargin()
{
    auto lineCount = [&](const QString &text, double room, bool noHang, bool word = true) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        c.insertText(text, run);
        LayoutEnv env;
        env.wordLines = word;
        env.wordNoHang = noHang;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(room, 300)}, env);
        return int(lay.lineInfo(0).size());
    };
    // The room: the width the text takes up to its last mark.
    const QString text = QStringLiteral("alpha beta gamma delta epsilon.");
    QTextDocument measure;
    QTextCursor mc(&measure);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    mc.insertText(text, run);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout wide;
    wide.build(&measure, {plainFrame(1000, 100)}, env);
    const double upToMark = wide.rangeRects(0, text.size() - 1, text.size()).first().left();
    const double room = upToMark + 0.5;   // (half a point past the word, not the room the period needs)
    QCOMPARE(lineCount(text, room, false), 1);                // it hangs
    QCOMPARE(lineCount(text, room, true), 2);                 // (not where the file says no punctuation hangs)
    QCOMPARE(lineCount(text, room, false, false), 2);         // (nor in a publication's text)
    QCOMPARE(lineCount(QStringLiteral("alpha beta gamma delta epsilon-"), room, false), 2);   // (a hyphen is no mark that hangs)
    QCOMPARE(lineCount(text, upToMark - 5, false), 2);        // (the word itself doesn't fit)
}


// A paragraph that does not let punctuation extend past the margin (tp::NoHangPunct, a word processor's overflowPunct turned off) has none hang, though the
// text does elsewhere.
void FlowTests::aParagraphThatTurnsHangingOffHasNone()
{
    auto lineCount = [&](double room, bool paragraphOff) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        if (paragraphOff) bf.setProperty(tp::NoHangPunct, true);
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("alpha beta gamma delta epsilon."), run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(room, 300)}, env);
        return int(lay.lineInfo(0).size());
    };
    QTextDocument measure;
    QTextCursor mc(&measure);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    const QString text = QStringLiteral("alpha beta gamma delta epsilon.");
    mc.insertText(text, run);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout wide;
    wide.build(&measure, {plainFrame(1000, 100)}, env);
    const double room = wide.rangeRects(0, text.size() - 1, text.size()).first().left() + 0.5;
    QCOMPARE(lineCount(room, false), 1);
    QCOMPARE(lineCount(room, true), 2);
}


// The text after a page break that begins a paragraph is its first line, with the paragraph's first-line indent (here a hanging one); after a break in
// the middle of a paragraph the text is a line that follows.
void FlowTests::textAfterABreakThatBeginsAParagraphHasItsFirstLineIndent()
{
    auto lefts = [&](const QString &text, bool paginated) {
        QVector<double> out;
        QTextBlockFormat bf;
        bf.setLeftMargin(40);
        bf.setTextIndent(-40);
        const QString words = QStringLiteral("alpha beta gamma delta epsilon zeta eta theta iota kappa lambda mu nu xi omicron pi rho sigma tau upsilon phi chi psi omega alpha beta gamma delta");
        if (paginated) {
            FlowDocument fd;
            fd.env.wordLines = true;
            QTextCursor c(fd.story());
            c.setBlockFormat(bf);
            c.insertText(text + words);
            Paginator pg(&fd);
            for (int k = 0; k < pg.pageCount(); ++k)
                for (const auto &l : pg.lines(k)) out << l.rect.left();
            return out;
        }
        QTextDocument doc;
        QTextCursor c(&doc);
        c.setBlockFormat(bf);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        c.insertText(text + words, run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(200, 300)}, env);
        for (const auto &l : lay.lineInfo(0)) out << l.rect.left();
        return out;
    };
    for (bool paginated : {false, true}) {
        const auto begin = lefts(QString(flow::kPageBreak), paginated), middle = lefts(QStringLiteral("some words ") + flow::kPageBreak, paginated);
        QVERIFY(begin.size() >= 3 && middle.size() >= 3);
        // The paragraph that begins with the break: its break's line, then the first line (at the margin), then the others (at the indent).
        QVERIFY2(std::abs(begin[1]) < 0.01 && std::abs(begin[2] - 40) < 0.01, qPrintable(QStringLiteral("%1 %2 %3").arg(begin[0]).arg(begin[1]).arg(begin[2])));
        // The break in the middle: the text after it is at the indent.
        QVERIFY2(std::abs(middle[1] - 40) < 0.01, qPrintable(QString::number(middle[1])));
    }
}


// A paragraph whose text and mark are both hidden takes no room (not a line, not its space); one whose mark is not hidden is an empty line.
void FlowTests::aParagraphThatIsAllHiddenTakesNoRoom()
{
    auto tops = [&](bool markHidden) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextCharFormat hidden = run;
        hidden.setProperty(tp::HiddenText, true);
        QTextBlockFormat bf;
        bf.setTopMargin(10);
        bf.setBottomMargin(10);
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("before"), run);
        c.insertBlock(bf, markHidden ? hidden : run);
        c.insertText(QStringLiteral("Top of Form"), hidden);
        c.insertBlock(bf, run);
        c.insertText(QStringLiteral("after"), run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(300, 300)}, env);
        const auto lines = lay.lineInfo(0);
        return std::make_pair(int(lines.size()), lines.last().rect.top() - lines.first().rect.bottom());
    };
    const auto gone = tops(true), line = tops(false);
    QCOMPARE(gone.first, 2);
    QVERIFY(std::abs(gone.second - 10) < 0.01);
    QCOMPARE(line.first, 3);
    QVERIFY(line.second > 20);
}


// The space between two paragraphs is the larger of the first's space after and the second's space before, not their sum; a paragraph that asks
// for contextual spacing has none between it and a paragraph of its own style.
void FlowTests::wordSpaceBetweenParagraphsIsTheLarger()
{
    const QString tnr = QStringLiteral("Times New Roman");
    const double single = (1825 + 443 + 87) / 2048.0 * 12;
    auto gap = [&](double after, double before, bool word = true) {
        const auto lines = wordLinesOf({{tnr, 12}, {tnr, 12}}, [&](int i, QTextBlockFormat &bf) {
            if (i == 0) bf.setBottomMargin(after);
            else bf.setTopMargin(before);
        }, word);
        return lines[1].rect.top() - lines[0].rect.bottom();
    };
    QVERIFY(std::abs(gap(6, 12) - 12) < 0.01);
    QVERIFY(std::abs(gap(12, 6) - 12) < 0.01);
    QVERIFY(std::abs(gap(6, 6) - 6) < 0.01);
    QVERIFY(std::abs(gap(0, 9) - 9) < 0.01);
    QVERIFY(std::abs(gap(6, 12, false) - 18) < 0.01);   // (as a publication's are added)
    auto contextual = [&](const QString &second) {
        const auto lines = wordLinesOf({{tnr, 12}, {tnr, 12}}, [&](int i, QTextBlockFormat &bf) {
            bf.setTopMargin(10);
            bf.setBottomMargin(10);
            bf.setProperty(tp::StyleName, i == 0 ? QStringLiteral("Body") : second);
            bf.setProperty(tp::ContextualSpacing, true);
        });
        return lines[1].rect.top() - lines[0].rect.bottom();
    };
    QVERIFY(std::abs(contextual(QStringLiteral("Body"))) < 0.01);
    QVERIFY(std::abs(contextual(QStringLiteral("Other")) - 10) < 0.01);
    Q_UNUSED(single);
}


// A file that turns the web page's way of spacing paragraphs off (LayoutEnv::wordSpaceSums) has the space after a paragraph and the space before
// the next both: their sum, as a publication's are.
void FlowTests::wordSpaceBetweenParagraphsAddsWhenTheFileAsksForIt()
{
    const QString tnr = QStringLiteral("Times New Roman");
    auto gap = [&](double after, double before, bool sums) {
        const auto lines = wordLinesOf({{tnr, 12}, {tnr, 12}}, [&](int i, QTextBlockFormat &bf) {
            if (i == 0) bf.setBottomMargin(after);
            else bf.setTopMargin(before);
        }, true, [&](LayoutEnv &env) { env.wordSpaceSums = sums; });
        return lines[1].rect.top() - lines[0].rect.bottom();
    };
    QVERIFY(std::abs(gap(12, 6, true) - 18) < 0.01);
    QVERIFY(std::abs(gap(6, 12, true) - 18) < 0.01);
    QVERIFY(std::abs(gap(6, 6, true) - 12) < 0.01);
    QVERIFY(std::abs(gap(12, 6, false) - 12) < 0.01);
    QVERIFY(std::abs(gap(6, 6, false) - 6) < 0.01);
    // (The layouts are kept apart: the same story is not found under the other's key.)
    LayoutEnv a, b;
    b.wordSpaceSums = true;
    QVERIFY(a.key() != b.key());
}


// A paragraph out of the flow (a frame's, in a word processor) is set where the next paragraph begins and takes none of the room.
void FlowTests::aParagraphOutOfTheFlowTakesNoRoom()
{
    const QString tnr = QStringLiteral("Times New Roman");
    auto lines = [&](bool out) {
        return wordLinesOf({{tnr, 12}, {tnr, 12}, {tnr, 12}}, [&](int i, QTextBlockFormat &bf) {
            bf.setBottomMargin(5);
            if (i == 1 && out) bf.setProperty(tp::OutOfFlow, true);
        });
    };
    const auto flowing = lines(false), floating = lines(true);
    QCOMPARE(floating.size(), 3);
    QVERIFY(std::abs(floating[1].rect.top() - floating[2].rect.top()) < 0.01);
    QVERIFY(std::abs(floating[2].rect.top() - flowing[1].rect.top()) < 0.01);
    QVERIFY(std::abs(floating[0].rect.top() - flowing[0].rect.top()) < 0.01);
    QVERIFY(floating[2].rect.top() < flowing[2].rect.top() - 10);
}


// A section's header and footer are drawn on its pages at the header and footer distances from the page's edge, the width of the text.
void FlowTests::pagesDrawTheirHeaderAndFooter()
{
    FlowDocument fd;
    fillStory(fd.story(), 60, 4);
    Section &s = fd.sections[0];
    s.headerDistance = 30;
    s.footerDistance = 20;
    const auto header = furnitureStory({QStringLiteral("HEADER ONE")}), footer = furnitureStory({QStringLiteral("FOOTER ONE"), QStringLiteral("FOOTER TWO")});
    s.header.odd = header;
    s.footer.odd = footer;
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() > 1);
    for (int k = 0; k < 2; ++k) {
        const auto page = pg.materialize(k);
        QVERIFY(page);
        const auto boxes = boxesByText(*page);
        QVERIFY(boxes.contains(QStringLiteral("HEADER ONE")));
        QVERIFY(boxes.contains(QStringLiteral("FOOTER ONE\nFOOTER TWO")));
        const TextItem *h = boxes[QStringLiteral("HEADER ONE")];
        QVERIFY(std::abs(h->rect.top() - 30) < 0.01);
        QVERIFY(std::abs(h->rect.left() - s.content().left()) < 0.01 && std::abs(h->rect.width() - s.content().width()) < 0.01);
        const double footerHeight = storyHeightOf(footer.get(), s.content().width(), fd);
        const TextItem *f = boxes[QStringLiteral("FOOTER ONE\nFOOTER TWO")];
        QVERIFY2(std::abs(f->rect.bottom() - (s.pageSize().height() - 20)) < 0.02, qPrintable(QString::number(f->rect.bottom())));
        QVERIFY2(std::abs(f->rect.height() - footerHeight) < 0.02, qPrintable(QStringLiteral("%1 vs %2").arg(f->rect.height()).arg(footerHeight)));
    }
    // (A page of a section with neither has no such boxes.)
    FlowDocument bare;
    fillStory(bare.story(), 5, 4);
    Paginator bp(&bare);
    const auto plain = bp.materialize(0);
    QCOMPARE(int(plain->doc->pages[0]->items.size()), 1);
}


// The space after a header's last paragraph is kept below it, where the paragraph has text or none (an empty one made to take room), but not where it is only line breaks.
void FlowTests::theSpaceAfterAHeadersLastParagraphOfLineBreaksIsNotKept()
{
    auto top = [](const QString &last) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 6, 4);
        auto header = std::make_shared<QTextDocument>();
        header->setDocumentMargin(0);
        {
            QTextCursor c(header.get());
            // (The program's own font, Arimo, in every line: a letter after the breaks is a line as tall as the empty one, on any machine.)
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("Arimo")});
            cf.setFontPointSize(60);
            c.insertText(QStringLiteral("Title"), cf);
            QTextBlockFormat after;
            after.setBottomMargin(40);
            c.insertBlock(after, cf);
            c.insertText(last, cf);
        }
        fd.sections[0].margins = QMarginsF(36, 18, 36, 36);
        fd.sections[0].headerDistance = 18;
        fd.sections[0].header.odd = header;
        Paginator pg(&fd);
        return pg.page(0).content.top();
    };
    const QString breaks(3, QChar::LineSeparator);
    const double withBreaks = top(breaks), withText = top(breaks + QLatin1Char('x')), empty = top(QString());
    QVERIFY2(std::abs(withText - withBreaks - 40) < 2, qPrintable(QStringLiteral("%1 with a letter after the line breaks, %2 with the breaks only").arg(withText).arg(withBreaks)));
    QVERIFY2(empty > 18 + 60, qPrintable(QString::number(empty)));
}


// A header's first paragraph, when it is empty, keeps its line height and the size of its mark on the page: the empty line is as tall as the paginator measured it.
void FlowTests::anEmptyFirstParagraphOfAHeaderKeepsItsMark()
{
    FlowDocument fd;
    fillStory(fd.story(), 20, 4);
    auto header = std::make_shared<QTextDocument>();
    header->setDocumentMargin(0);
    {
        QTextCursor c(header.get());
        QTextCharFormat big, small;
        big.setFontPointSize(30);
        small.setFontPointSize(10);
        QTextBlockFormat tight;
        tight.setLineHeight(80, QTextBlockFormat::ProportionalHeight);
        c.setBlockFormat(tight);
        c.setBlockCharFormat(big);
        c.insertBlock(QTextBlockFormat(), small);
        c.insertText(QStringLiteral("HEADER TEXT"), small);
    }
    fd.sections[0].header.odd = header;
    fd.sections[0].headerDistance = 10;
    Paginator pg(&fd);
    const auto page = pg.materialize(0);
    QVERIFY(page);
    const TextItem *box = nullptr;
    for (const auto &item : page->doc->pages[0]->items)
        if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
            if (const Story *st = page->doc->story(t->storyId))
                if (st->doc->toPlainText().contains(QStringLiteral("HEADER TEXT"))) {
                    box = t;
                    QCOMPARE(st->doc->begin().charFormat().fontPointSize(), 30.0);
                    QCOMPARE(st->doc->begin().blockFormat().lineHeight(), 80.0);
                }
    QVERIFY(box);
}


// With facing pages (mirrored margins) a section's left and right margins are its inside and its outside: the text, the header, and the footer of a page with an odd number are
// set as the section says, and those of a page with an even number have its right margin on the left and its left on the right, whatever the numbers of the pages begin at; the
// width of the text is the same on all, and a document without them has every page alike.
void FlowTests::mirroredMarginsSwapTheSidesOfEvenPages()
{
    for (const bool mirrored : {false, true}) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.mirrorMargins = mirrored;
        fillStory(fd.story(), 3, 7, [](int i, QTextBlockFormat &bf) { if (i > 0) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 10, 10);   // (a paragraph to a page)
        Section &s = fd.sections[0];
        s.margins = QMarginsF(100, 72, 40, 72);
        s.header.first = s.header.odd = s.header.even = furnitureStory({QStringLiteral("HEAD")});
        s.footer.first = s.footer.odd = s.footer.even = furnitureStory({QStringLiteral("FOOT")});
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 3);
        for (int page = 0; page < 3; ++page) {
            const auto m = pg.materialize(page);
            QVERIFY(m && !m->boxes.isEmpty());
            const double inside = 100, outside = 40, left = mirrored && page % 2 == 1 ? outside : inside, right = mirrored && page % 2 == 1 ? inside : outside;
            const auto *body = dynamic_cast<const TextItem *>(m->doc->item(m->boxes[0]));
            QVERIFY(body);
            const QString where = QStringLiteral("%1 page %2").arg(mirrored ? "mirrored" : "plain").arg(page + 1);
            QVERIFY2(std::abs(body->rect.left() - left) < 0.01, qPrintable(where + QStringLiteral(": the text begins at %1, not %2").arg(body->rect.left()).arg(left)));
            QVERIFY2(std::abs(body->rect.right() - (612 - right)) < 0.01, qPrintable(where + QStringLiteral(": the text ends at %1, not %2").arg(body->rect.right()).arg(612 - right)));
            for (const char *kind : {"flow-header-box", "flow-footer-box"}) {
                const auto boxes = audit::itemsWith(*m, QString::fromLatin1(kind));
                QVERIFY2(boxes.size() == 1, qPrintable(where + QStringLiteral(": %1 has %2 boxes").arg(kind).arg(boxes.size())));
                QVERIFY2(std::abs(boxes[0]->rect.left() - left) < 0.01 && std::abs(boxes[0]->rect.right() - (612 - right)) < 0.01,
                         qPrintable(where + QStringLiteral(": %1 is at %2 to %3").arg(kind).arg(boxes[0]->rect.left()).arg(boxes[0]->rect.right())));
            }
        }
    }
}


// The text of a page that is laid out in parts (a continuous section with other columns begins on it, under the text of the section before) is set between the mirrored margins on a
// page with an even number, in every part: the single column of the first and the columns of the second.
void FlowTests::mirroredMarginsHoldOnAPageLaidOutInParts()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fd.mirrorMargins = true;
    fillStory(fd.story(), 14, 7, [](int i, QTextBlockFormat &bf) {
        if (i == 1) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
        if (i == 1) bf.setProperty(fp::SectionBreakAfter, true);
    }, 40, 60);
    Section &one = fd.sections[0];
    one.margins = QMarginsF(100, 72, 40, 72);
    Section two = one;
    two.continuous = true;
    two.columns = 2;
    two.columnGap = 20;
    fd.sections << two;
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 2);
    const auto m = pg.materialize(1);   // (the page with the number 2: the text of the first section, and the columns of the second under it)
    QVERIFY(m);
    QStringList all;
    for (const auto &id : m->boxes)
        if (const auto *box = dynamic_cast<const TextItem *>(m->doc->item(id))) all << QStringLiteral("[%1 %2]").arg(box->rect.left()).arg(box->rect.right());
    QVERIFY2(m->boxes.size() >= 3, qPrintable(QStringLiteral("%1 pages, the boxes: %2").arg(pg.pageCount()).arg(all.join(QLatin1Char(' ')))));
    QVector<double> lefts, rights;
    for (const auto &id : m->boxes)
        if (const auto *box = dynamic_cast<const TextItem *>(m->doc->item(id))) {
            lefts << box->rect.left();
            rights << box->rect.right();
        }
    QVERIFY(lefts.size() >= 3);
    QVERIFY2(*std::min_element(lefts.begin(), lefts.end()) > 39.99 && *std::min_element(lefts.begin(), lefts.end()) < 40.01, qPrintable(QStringLiteral("the text begins at %1, not 40").arg(*std::min_element(lefts.begin(), lefts.end()))));
    QVERIFY2(*std::max_element(rights.begin(), rights.end()) > 511.99 && *std::max_element(rights.begin(), rights.end()) < 512.01, qPrintable(QStringLiteral("the text ends at %1, not 512").arg(*std::max_element(rights.begin(), rights.end()))));
    // (The columns are 226 wide, one from 40 and one from 286.)
    int columns = 0;
    for (const double left : lefts) columns += std::abs(left - 40) < 0.01 || std::abs(left - 286) < 0.01;
    QStringList begins;
    for (const double left : lefts) begins << QString::number(left);
    QVERIFY2(columns == lefts.size(), qPrintable(QStringLiteral("boxes begin at %1").arg(begins.join(QLatin1Char(',')))));
}


// Inside and outside are the sides of the binding by the page's number, not by whether the margins are mirrored (ECMA-376 part 1, 17.18: the left and the right on a page with an odd
// number, the right and the left on an even one): an object aligned to the inside of the margin, and one aligned to the outside, one set from the inside margin's strip, and one
// from the outside's, are on the left and the right of odd pages and the right and the left of even pages, whether or not the margins are mirrored.
void FlowTests::insideAndOutsideAreTheSidesOfTheBindingByThePageNumber()
{
    for (const bool mirrored : {false, true}) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.mirrorMargins = mirrored;
        fillStory(fd.story(), 3, 7, [](int i, QTextBlockFormat &bf) { if (i > 0) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 10, 10);
        fd.sections[0].margins = QMarginsF(100, 72, 40, 72);
        struct Kind {
            double width;
            FloatingObject::Rel rel;
            FloatingObject::Align align;
        };
        const Kind kinds[] = {{30, FloatingObject::Rel::Margin, FloatingObject::Align::Inside}, {31, FloatingObject::Rel::Margin, FloatingObject::Align::Outside},
                              {32, FloatingObject::Rel::InsideMargin, FloatingObject::Align::None}, {33, FloatingObject::Rel::OutsideMargin, FloatingObject::Align::None}};
        for (int page = 2; page >= 0; --page)
            for (const Kind &k : kinds) {
                auto item = std::make_shared<ShapeItem>();
                item->rect = QRectF(0, 0, k.width, 20);
                const int id = addObject(&fd, item, fd.story()->findBlockByNumber(page).position(), k.rel, FloatingObject::Rel::Paragraph, 0, 0);
                fd.objects[size_t(id) - 1].hAlign = k.align;
            }
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 3);
        for (int page = 0; page < 3; ++page) {
            const auto m = pg.materialize(page);
            QVERIFY(m);
            QHash<int, double> at;   // (by the object's width)
            for (const auto &it : m->doc->pages[0]->items)
                if (const auto *shape = dynamic_cast<const ShapeItem *>(it.get())) at.insert(int(std::lround(shape->rect.width())), shape->rect.left());
            const bool even = page % 2 == 1;
            const double leftMargin = mirrored && even ? 40 : 100, rightMargin = mirrored && even ? 100 : 40;   // (what the page's margins are)
            const QString where = QStringLiteral("%1 page %2").arg(mirrored ? "mirrored" : "plain").arg(page + 1);
            auto is = [&](int width, double expected, const char *what) {
                QVERIFY2(at.contains(width) && std::abs(at.value(width) - expected) < 0.01, qPrintable(where + QStringLiteral(": %1 is at %2, not %3").arg(QLatin1String(what)).arg(at.value(width, -1)).arg(expected)));
            };
            is(30, even ? 612 - rightMargin - 30 : leftMargin, "inside");
            is(31, even ? leftMargin : 612 - rightMargin - 31, "outside");
            is(32, even ? 612 - rightMargin : 0, "the inside strip");
            is(33, even ? 0 : 612 - rightMargin, "the outside strip");
        }
    }
}


// A paragraph set to the outside (or the inside) of the page is at the right (the left) of a page with an odd number and at the left (the right) of one with an even number.
void FlowTests::aParagraphSetOutsideOrInsideFollowsThePageNumber()
{
    for (const int side : {1, 2}) {   // (1: outside, 2: inside)
        for (const int number : {1, 2, 3, 4}) {
            QTextDocument doc;
            QTextCursor c(&doc);
            QTextBlockFormat bf;
            bf.setProperty(tp::PageSideAlign, side);
            c.setBlockFormat(bf);
            QTextCharFormat cf;
            cf.setFontPointSize(11);
            cf.setFontFamilies({QStringLiteral("Arimo")});
            c.insertText(QStringLiteral("Page"), cf);
            FrameSpec fs;
            fs.size = QSizeF(300, 100);
            fs.insets = QMarginsF(0, 0, 0, 0);
            fs.hyphenate = false;
            fs.ctx.pageNumber = number;
            fs.ctx.pageCount = 4;
            LayoutEnv env;
            env.wordLines = true;
            StoryLayout layout;
            layout.build(&doc, {fs}, env);
            const QVector<QRectF> rects = layout.rangeRects(0, 0, 4);
            QVERIFY(!rects.isEmpty());
            const bool right = (side == 1) == (number % 2 == 1);
            const QString what = QStringLiteral("%1 on page %2: the text is from %3 to %4").arg(side == 1 ? "outside" : "inside").arg(number).arg(rects.first().left()).arg(rects.last().right());
            if (right) QVERIFY2(rects.last().right() > 299 && rects.first().left() > 200, qPrintable(what));
            else QVERIFY2(rects.first().left() < 1 && rects.last().right() < 100, qPrintable(what));
        }
    }
}


// Pages that an edit moves down by a page change from odd to even, and where odd and even pages have headers of other heights they are laid out again:
// the pages after the edit are the ones a fresh paginator makes.
void FlowTests::evenAndOddHeadersOfOtherHeightsSettleAfterAnEdit()
{
    for (bool sameHeights : {false, true}) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 6 * 25, 7, [](int i, QTextBlockFormat &bf) { if (i % 25 == 0 && i > 0) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 40, 40);
        Section &s = fd.sections[0];
        s.differentOddEven = true;
        s.header.odd = furnitureStory({QStringLiteral("ODD")});
        s.header.even = sameHeights ? furnitureStory({QStringLiteral("EVEN")})
                                    : furnitureStory({QStringLiteral("EVEN 1"), QStringLiteral("EVEN 2"), QStringLiteral("EVEN 3"), QStringLiteral("EVEN 4"), QStringLiteral("EVEN 5"), QStringLiteral("EVEN 6")});
        Paginator pg(&fd);
        const int before = pg.pageCount();
        QTextCursor c(fd.story());
        c.setPosition(fd.story()->findBlockByNumber(2).position());
        QTextCharFormat cf;
        cf.setFontPointSize(11);
        for (int i = 0; i < 14; ++i) {
            c.insertText(QStringLiteral("An added paragraph that is long enough to take two or three lines of the page when it is set at eleven points in this frame. "), cf);
            c.insertBlock();
        }
        QVERIFY(pg.pageCount() > before);
        const QString bad = differenceFromFresh(fd, pg, false);
        QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("%1 headers: %2").arg(sameHeights ? "same-height" : "different-height", bad)));
    }
}


// Edits of every kind to a document with headers for the first, odd, and even pages of other heights, notes (one of them longer than a page), sections that
// are continuous and that are on pages of their own (with other columns), and page breaks: after each, the paginator that laid the document out before is
// the paginator a fresh one is. Half of the documents have tables as well, one in each kind of section, with notes cited in their cells and rows that go over
// page breaks; their edits also type in cells, put rows in and take them out, put section breaks in and take them out, replace headers and footers, and undo
// and redo. The two paginators have the same pages: the page count, and for each page its start, lines, table rows, notes, headers and footers, and boxes.
// (JP_FUZZ=n: seeds 1 to n, for a longer run; JP_FUZZ_TRACE=1: what each run did.)
void FlowTests::incrementalPaginationMatchesAFreshOneAfterRandomEdits()
{
    int compared = 0, runs = 0;
    QStringList failures;   // (a run that fails stops, and the next goes on: the report says which seeds fail)
    const unsigned seeds = qEnvironmentVariableIsSet("JP_FUZZ") ? qEnvironmentVariable("JP_FUZZ").toUInt() : 2u;
    for (int variant = 0; variant < 4; ++variant) {
        const bool restart = variant & 1, tables = variant & 2;
        for (unsigned seed = 1; seed <= seeds; ++seed) {
            std::mt19937 rng(seed * 7919u + (restart ? 1u : 0u) + (tables ? 1000003u : 0u));
            TestNotes notes;
            const int words[] = {3, 12, 60, 900, 4, 30};
            QStringList ids;
            for (int i = 0; i < 6; ++i) {
                ids << QStringLiteral("n%1").arg(i);
                addNote(&notes, ids.last(), words[i], QStringLiteral("NOTE%1").arg(i));
            }
            FlowDocument fd;
            fd.env.wordLines = true;
            fd.fields = &notes;
            fd.restartNotesInSections = restart;
            fillStory(fd.story(), 44, seed, [](int i, QTextBlockFormat &bf) {
                bf.setBottomMargin(6);
                if (i == 7 || i == 24 || i == 38) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
                if (i == 15 || i == 30) bf.setProperty(flow::fp::SectionBreakAfter, true);
                if (i % 3 == 0) bf.setProperty(tp::WidowControl, true);
            }, 20, 70);
            Section &a = fd.sections[0];
            a.differentFirstPage = a.differentOddEven = true;
            a.header.first = furnitureStory({QStringLiteral("FIRST")});
            a.header.odd = furnitureStory({QStringLiteral("ODD")});
            a.header.even = furnitureStory({QStringLiteral("EVEN 1"), QStringLiteral("EVEN 2"), QStringLiteral("EVEN 3"), QStringLiteral("EVEN 4")});
            a.footer.odd = furnitureStory({QStringLiteral("FOOT ODD")});
            a.footer.even = furnitureStory({QStringLiteral("FOOT EVEN 1"), QStringLiteral("FOOT EVEN 2")});
            Section b = a;
            b.continuous = true;
            b.columns = 2;
            b.columnGap = 20;
            Section c = a;
            c.columns = 1;
            c.differentFirstPage = false;
            c.margins = QMarginsF(90, 60, 54, 80);
            c.header.even = furnitureStory({QStringLiteral("C EVEN 1"), QStringLiteral("C EVEN 2"), QStringLiteral("C EVEN 3"), QStringLiteral("C EVEN 4"), QStringLiteral("C EVEN 5")});
            fd.sections << b << c;
            // (The third section begins on an odd page, or an even one, in two of the three kinds of document: the blank pages come and go with the edits before it.)
            fd.sections[2].startParity = (seed + unsigned(variant)) % 3 == 1 ? Section::Parity::Odd : (seed + unsigned(variant)) % 3 == 2 ? Section::Parity::Even : Section::Parity::Any;
            // (And in half of them the pages are not of two kinds: odd and even pages have the same headers, so that only the blank pages tell them apart.)
            if (seed % 2 == 0)
                for (Section &sec : fd.sections) sec.differentOddEven = false;
            for (const int block : {40, 22, 5, 33, 12}) addReference(fd.story(), fd.story()->findBlockByNumber(block).position() + 4, ids[rng() % 6]);
            // Objects the text goes round, in each section and of each kind: a picture against the column's edge, one with no text beside it, one with text on its left only,
            // one from the page with the text on its larger side, one from a line above it, and a wide one across a column (the story's first paragraphs hold them: edits that
            // take their anchors away, and move them across pages, change which pages have walls).
            {
                using Wr = FloatingObject::Wrap;
                using Sd = FloatingObject::Side;
                using Rl = FloatingObject::Rel;
                struct Wall { int block; double w, h; Wr wrap; Sd side; Rl h0, v0; double dx, dy; FloatingObject::Align align; };
                for (const Wall &w : {Wall{2, 120, 80, Wr::Square, Sd::Both, Rl::Column, Rl::Paragraph, 0, 0, FloatingObject::Align::End},
                                      Wall{9, 100, 40, Wr::TopBottom, Sd::Both, Rl::Column, Rl::Paragraph, 60, 0, FloatingObject::Align::None},
                                      Wall{14, 90, 70, Wr::Square, Sd::Left, Rl::Margin, Rl::Paragraph, 150, 10, FloatingObject::Align::None},
                                      Wall{20, 110, 100, Wr::Square, Sd::Largest, Rl::Page, Rl::Page, 300, 200, FloatingObject::Align::None},
                                      Wall{26, 80, 60, Wr::Square, Sd::Both, Rl::Column, Rl::Line, 0, -20, FloatingObject::Align::End},
                                      Wall{33, 70, 90, Wr::Square, Sd::Right, Rl::Column, Rl::Paragraph, 0, 5, FloatingObject::Align::Center},
                                      Wall{41, 300, 30, Wr::TopBottom, Sd::Both, Rl::Column, Rl::Paragraph, 20, 0, FloatingObject::Align::None}}) {
                    const int id = addWrapped(&fd, w.w, w.h, fd.story()->findBlockByNumber(w.block).position() + 1, w.wrap, w.dx, w.dy, QMarginsF(8, 4, 8, 4), w.side, w.h0, w.v0);
                    fd.objects[size_t(id) - 1].hAlign = w.align;
                }
            }
            if (tables) {
                // A table in each section (the last first, so that the blocks the others are after stay where they are), each with a note cited in a cell and a row taller than the rest.
                struct Put { int after, rows, cols; double width; const char *note; int row; };
                for (const Put &p : {Put{36, 18, 3, 140, "n2", 9}, Put{20, 7, 2, 100, "n0", 3}, Put{3, 6, 2, 110, "n1", 2}}) {
                    TestTable spec;
                    spec.rows = p.rows;
                    spec.cols = p.cols;
                    spec.columnWidth = p.width;
                    spec.setRow = [](int r, RowData &row) { row.header = r == 0; };
                    spec.text = [](int r, int col) { return r % 4 == 2 && col == 1 ? QStringLiteral("a taller\ncell of\nthree lines") : QStringLiteral("r%1c%2 words").arg(r).arg(col); };
                    const TestTable made = addTableAfter(fd, p.after, spec);
                    addReference(fd.story(), made.rowStarts[p.row] + 2, QString::fromLatin1(p.note));
                }
            }
            // Frames, in each section: set out of the text, at the paragraph after them (the first and last of the block numbers asked for that are not in a table: a frame
            // between the paragraphs of a cell would be no frame of the text's). Edits that go into them, cut them, or move their paragraph from one page to another change
            // which pages have them.
            {
                FloatingObject proto;
                proto.hRel = FloatingObject::Rel::Margin;
                proto.vRel = FloatingObject::Rel::Paragraph;
                proto.wrap = FloatingObject::Wrap::Square;
                proto.gap = QMarginsF(6, 3, 6, 3);
                int made = 0;
                for (const int wanted : {50, 30, 12, 4}) {
                    int block = std::min(wanted, fd.story()->blockCount() - 1);   // (a story of 44 paragraphs has no block 50: the frame would go in at the story's start)
                    while (block > 1 && (fd.story()->findBlockByNumber(block).blockFormat().intProperty(fp::Cell) || fd.story()->findBlockByNumber(block - 1).blockFormat().intProperty(fp::Cell) ||
                                         fd.story()->findBlockByNumber(block - 1).blockFormat().intProperty(fp::Frame)))
                        --block;
                    if (block <= 1) continue;
                    FloatingObject o = proto;
                    o.hAlign = made % 2 ? FloatingObject::Align::Start : FloatingObject::Align::End;
                    o.dy = 2 * made;
                    addFrame(fd, block, {QStringLiteral("FRAME %1 first words of it").arg(made), QStringLiteral("and a second paragraph of the frame, longer, so that it takes more than a line of it")}, o, made == 2 ? 0 : 110 + 10 * made, made == 3 ? 90 : 0, made == 1);
                    ++made;
                }
                QVERIFY(made >= 3);
            }
            if (tables) {
                // Tables set at places, with the text going round them: between ordinary paragraphs, one at the start of a column's text and one nearer the end. Edits in their
                // cells, and around them, change which pages draw them and how tall each is.
                int made = 0;
                auto plainBlock = [&](int i) { const QTextBlockFormat f = fd.story()->findBlockByNumber(i).blockFormat(); return !f.intProperty(fp::Cell) && !f.intProperty(fp::Frame); };
                QVector<int> candidates;
                for (int i = 1; i + 1 < fd.story()->blockCount(); ++i)
                    if (plainBlock(i - 1) && plainBlock(i) && plainBlock(i + 1)) candidates << i;
                QVERIFY(candidates.size() >= 6);
                // (From the last to the first, so that the numbers of the blocks before stay.)
                for (const int k : {3, 2, 1}) {
                    const int n = candidates[candidates.size() * k / 4];
                    TestTable spec;
                    spec.rows = 2 + made;
                    spec.cols = 2;
                    spec.columnWidth = 50 + 10 * made;
                    spec.setTable = [made](TableData &t) {
                        TableData::Place p;
                        p.across = made == 1 ? TableData::Place::From::Page : TableData::Place::From::Margin;
                        p.acrossSpec = made == 1 ? TableData::Place::Spec::None : (made == 0 ? TableData::Place::Spec::End : TableData::Place::Spec::Start);
                        p.x = made == 1 ? 120 : 0;
                        p.y = 3 * made;
                        p.room = QMarginsF(6, 2, 6, 2);
                        t.place = p;
                    };
                    addTableAfter(fd, n, spec);
                    ++made;
                }
                QVERIFY(made >= 2);
            }
            if (tables) fd.story()->setUndoRedoEnabled(true);   // (undo and redo are of the edits below)
            Paginator pg(&fd);
            QVERIFY(pg.pageCount() > 4);
            QString trace;
            std::vector<std::shared_ptr<QTextDocument>> kept;   // (replaced headers stay alive: a new story at the address of a freed one has the paginator's measures of the old)
            const int steps = tables ? 14 : 12;
            for (int step = 0; step < steps; ++step) {
                QTextCursor cur(fd.story());
                const QString was = startsText(pg);
                const int last = std::max(1, fd.story()->characterCount() - 1);
                const int at = int(rng() % unsigned(last));
                const QTextBlock blk = fd.story()->findBlock(at);
                const bool flagged = blk.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
                const bool inCell = blk.blockFormat().intProperty(fp::Cell) != 0;
                QTextCharFormat cf;
                cf.setFontPointSize(11);
                const unsigned what = rng() % (tables ? 13u : 6u);
                cur.setPosition(at);
                if (what == 0 || what == 1) {
                    QString text;
                    for (unsigned w = 0, n = 3 + rng() % 60; w < n; ++w) text += QStringLiteral("word%1 ").arg(rng() % 100);
                    cur.insertText(text, cf);
                    trace += QStringLiteral("text@%1 ").arg(at);
                } else if (what == 2) {
                    const int len = std::min<int>(int(rng() % 400), blk.position() + blk.length() - 1 - at);
                    if (len > 0) {
                        cur.setPosition(at + len, QTextCursor::KeepAnchor);
                        cur.removeSelectedText();
                    }
                    trace += QStringLiteral("cut@%1+%2 ").arg(at).arg(len);
                } else if (what == 3 && !flagged) {
                    cur.setPosition(blk.position() + blk.length() - 1);
                    QTextBlockFormat nf;
                    nf.setBottomMargin(6);
                    if (inCell) nf.setProperty(fp::Cell, blk.blockFormat().intProperty(fp::Cell));   // (Enter in a cell: another paragraph of the cell)
                    cur.insertBlock(nf, cf);
                    QString text;
                    for (unsigned w = 0, n = 10 + rng() % 70; w < n; ++w) text += QStringLiteral("para%1 ").arg(rng() % 100);
                    cur.insertText(text, cf);
                    trace += QStringLiteral("para@%1 ").arg(blk.position());
                } else if (what == 4 && blk.position() > 0 && !flagged && !inCell) {
                    QTextBlockFormat bf = blk.blockFormat();
                    bf.setPageBreakPolicy(bf.pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore ? QTextFormat::PageBreak_Auto : QTextFormat::PageBreak_AlwaysBefore);
                    cur.setPosition(blk.position());
                    cur.setBlockFormat(bf);
                    trace += QStringLiteral("break@%1 ").arg(blk.position());
                } else if (what == 6) {
                    // Typing in a cell.
                    QVector<QTextBlock> cells;
                    for (QTextBlock x = fd.story()->begin(); x.isValid(); x = x.next())
                        if (x.blockFormat().intProperty(fp::Cell)) cells << x;
                    if (!cells.isEmpty()) {
                        const QTextBlock cell = cells[int(rng() % unsigned(cells.size()))];
                        const int where = cell.position() + int(rng() % unsigned(cell.length()));
                        cur.setPosition(where);
                        cur.insertText(QStringLiteral(" typed in a cell with several words "), cf);
                        trace += QStringLiteral("cell@%1 ").arg(where);
                    }
                } else if (what == 7 || what == 8) {
                    // A row put in, or taken out (of a table that keeps two).
                    const QVector<RowRange> rows = rowRangesOf(fd);
                    if (!rows.isEmpty()) {
                        const RowRange r = rows[int(rng() % unsigned(rows.size()))];
                        int inTable = 0;
                        for (const RowRange &o : rows) inTable += o.table == r.table;
                        if (what == 7) {
                            copyRowOf(fd, r);
                            trace += QStringLiteral("row+@%1 ").arg(r.from);
                        } else if (inTable > 2) {
                            cur.setPosition(r.from - 1);
                            cur.setPosition(r.to - 1, QTextCursor::KeepAnchor);
                            cur.removeSelectedText();
                            trace += QStringLiteral("row-@%1 ").arg(r.from);
                        }
                    }
                } else if (what == 9) {
                    // A section break put in after a paragraph, or the one it has taken out; the sections are told as a word processor tells them.
                    if (!inCell && blk.next().isValid()) {
                        int s = 0;
                        for (QTextBlock x = fd.story()->begin(); x != blk; x = x.next()) s += x.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
                        QTextBlockFormat bf = blk.blockFormat();
                        bool changedSections = true;
                        if (flagged) {
                            bf.clearProperty(flow::fp::SectionBreakAfter);
                            cur.setPosition(blk.position());
                            cur.setBlockFormat(bf);
                            if (s + 1 < fd.sections.size()) fd.sections.remove(s + 1);
                            trace += QStringLiteral("unsection@%1 ").arg(blk.position());
                        } else if (fd.sections.size() < 6) {
                            bf.setProperty(flow::fp::SectionBreakAfter, true);
                            cur.setPosition(blk.position());
                            cur.setBlockFormat(bf);
                            Section made = fd.sections[std::min<int>(s, fd.sections.size() - 1)];
                            made.continuous = false;
                            made.startParity = rng() % 3 == 0 ? Section::Parity::Odd : rng() % 2 ? Section::Parity::Even : Section::Parity::Any;
                            if (rng() % 3 == 0) made.firstPageNumber = int(rng() % 4);   // (a restart of the numbers: odd or even by it, where the pages are of two kinds)
                            fd.sections.insert(s + 1, made);
                            trace += QStringLiteral("section@%1 ").arg(blk.position());
                        } else {
                            changedSections = false;
                        }
                        if (changedSections) {
                            fd.story()->clearUndoRedoStacks();   // (the sections are not in it)
                            pg.invalidateFrom(s);
                        }
                    }
                } else if (what == 10) {
                    // A header or a footer replaced.
                    const int s = int(rng() % unsigned(fd.sections.size())), slot = int(rng() % 6u);
                    QStringList lines;
                    for (unsigned i = 0, n = 1 + rng() % 6; i < n; ++i) lines << QStringLiteral("NEW %1 LINE %2").arg(slot).arg(i);
                    auto story = furnitureStory(lines);
                    HeaderFooter &hf = slot < 3 ? fd.sections[s].header : fd.sections[s].footer;
                    std::shared_ptr<QTextDocument> &target = slot % 3 == 0 ? hf.first : slot % 3 == 1 ? hf.odd : hf.even;
                    kept.push_back(target);
                    kept.push_back(story);
                    target = story;
                    pg.invalidateFrom(s);
                    trace += QStringLiteral("header%1/%2 ").arg(s).arg(slot);
                } else if (what == 11 || what == 12) {
                    if (what == 11 && fd.story()->isUndoAvailable()) {
                        fd.story()->undo();
                        trace += QStringLiteral("undo ");
                    } else if (what == 12 && fd.story()->isRedoAvailable()) {
                        fd.story()->redo();
                        trace += QStringLiteral("redo ");
                    }
                } else {
                    addReference(fd.story(), at, ids[rng() % 6]);
                    trace += QStringLiteral("note@%1 ").arg(at);
                }
                if (rng() % 3 == 0) pg.paginate(int(rng() % 4));   // (some of it, now)
                QString refsText;
                for (QTextBlock rb = fd.story()->begin(); rb.isValid(); rb = rb.next())
                    for (auto rit = rb.begin(); !rit.atEnd(); ++rit)
                        if (rit.fragment().charFormat().stringProperty(tp::Field).startsWith(QLatin1String("footnote:"))) refsText += QStringLiteral("%1:%2 ").arg(rit.fragment().position()).arg(rit.fragment().charFormat().stringProperty(tp::Field).mid(9));
                Paginator fresh(&fd);
                const QString bad0 = audit::differenceFromFresh(pg, fresh, step % 2 == 1 || step == steps - 1);   // (the pages made, every other edit: that costs more than their lines)
                const QString bad = bad0.isEmpty() ? bad0 : bad0 + QStringLiteral(" refs [") + refsText + QLatin1Char(']');
                ++compared;
                // The text goes round the objects as the page layer draws it, on every page that has them (every third edit: the pages are made for it).
                const QString walls = bad.isEmpty() && step % 3 == 2 ? audit::wallProblems(pg, fd.story()) : QString();
                // The text on the pages is the story's text, whole: the walls, frames, and floating tables take room from lines and move them, and keep none from the pages.
                if (const QString lost = bad.isEmpty() ? audit::coverageProblems(pg, fd.story()) : QString(); !lost.isEmpty()) {
                    failures << QStringLiteral("%1 restart %2, seed %3, step %4 (%5): text: %6").arg(tables ? "tables," : "plain,").arg(restart).arg(seed).arg(step).arg(trace, lost);
                    break;
                }
                if (!walls.isEmpty()) {
                    failures << QStringLiteral("%1 restart %2, seed %3, step %4 (%5): walls: %6").arg(tables ? "tables," : "plain,").arg(restart).arg(seed).arg(step).arg(trace, walls);
                    break;
                }
                // Every frame and every table set at a place is drawn once, or is in the text.
                if (const QString floats = bad.isEmpty() && step % 3 == 1 ? audit::floatProblems(pg, fd) : QString(); !floats.isEmpty()) {
                    failures << QStringLiteral("%1 restart %2, seed %3, step %4 (%5): floats: %6").arg(tables ? "tables," : "plain,").arg(restart).arg(seed).arg(step).arg(trace, floats);
                    break;
                }
                if (!bad.isEmpty()) {
                    failures << QStringLiteral("%1 restart %2, seed %3, step %4 (%5): %6 (before the last edit: %7)").arg(tables ? "tables," : "plain,").arg(restart).arg(seed).arg(step).arg(trace, bad, was.left(160));
                    break;   // (the next run)
                }
            }
            ++runs;
            if (qEnvironmentVariableIsSet("JP_FUZZ_TRACE")) qInfo("%s restart %d seed %u: %d pages, %d table rows; %s", tables ? "tables" : "plain", int(restart), seed, pg.pageCount(), int(rowRangesOf(fd).size()), qPrintable(trace));
        }
    }
    QVERIFY2(failures.isEmpty(), qPrintable(QStringLiteral("%1 of %2 runs differ from a fresh paginator:\n%3").arg(failures.size()).arg(runs).arg(failures.join(QLatin1Char('\n')))));
    QVERIFY(compared >= 8 * 12);
}


// ---- tables with the headers, footers, notes, and sections of a word processor's document (the two waves of JeffWrite together)

// A note cited in the paragraph before a table, one cited in a cell of it, and one in the text after it (each alone, then the three together): each is drawn on the page
// whose lines cite it, below the rows, at the number its reference has.
void FlowTests::notesCitedAroundATableAreOnThePagesThatCiteThem_data()
{
    QTest::addColumn<bool>("before");
    QTest::addColumn<bool>("cell");
    QTest::addColumn<bool>("after");
    QTest::newRow("in the paragraph before the table") << true << false << false;
    QTest::newRow("in a cell") << false << true << false;
    QTest::newRow("in the text after the table") << false << false << true;
    QTest::newRow("all three") << true << true << true;
}


void FlowTests::notesCitedAroundATableAreOnThePagesThatCiteThem()
{
        QFETCH(bool, before);
        QFETCH(bool, cell);
        QFETCH(bool, after);
        TestNotes notes;
        addNote(&notes, QStringLiteral("before"), 14, QStringLiteral("NOTEBEFORE"));
        addNote(&notes, QStringLiteral("cell"), 30, QStringLiteral("NOTECELL"));
        addNote(&notes, QStringLiteral("after"), 9, QStringLiteral("NOTEAFTER"));
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.fields = &notes;
        fillStory(fd.story(), 5, 3, {}, 30, 50);
        TestTable spec;
        spec.rows = 110;
        spec.cols = 2;
        spec.columnWidth = 150;
        const TestTable table = addTable(fd, spec);
        appendParagraph(fd, QStringLiteral("The paragraph after the table, which may cite a note."));
        appendParagraph(fd, QStringLiteral("The last paragraph."));
        if (after) addReference(fd.story(), fd.story()->findBlockByNumber(fd.story()->blockCount() - 2).position() + 20, QStringLiteral("after"));
        if (cell) addReference(fd.story(), table.rowStarts[45] + 2, QStringLiteral("cell"));
        if (before) addReference(fd.story(), fd.story()->findBlockByNumber(4).position() + 25, QStringLiteral("before"));
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 3);
        QMap<QString, int> pageOf;
        for (int k = 0; k < pages; ++k)
            for (const QString &id : audit::citedNotes(pg, fd.story(), k)) pageOf[id] = k;
        QCOMPARE(int(pageOf.size()), int(before) + int(cell) + int(after));
        // (All three are on three pages: the paragraph before the table on the first, the cell far down the table, the paragraph after it on the last.)
        if (before && cell && after)
            QVERIFY2(pageOf["before"] < pageOf["cell"] && pageOf["cell"] < pageOf["after"], qPrintable(QStringLiteral("the notes are on the pages %1 %2 %3").arg(pageOf["before"]).arg(pageOf["cell"]).arg(pageOf["after"])));
        const QMap<QString, QString> marks{{"before", "NOTEBEFORE"}, {"cell", "NOTECELL"}, {"after", "NOTEAFTER"}};
        const QString bad = audit::problems(pg, fd.story(), marks);
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }


// Headers and footers of the first, odd, and even pages are of other heights, and a table runs across several pages: its rows stay in the text area that the header and
// footer leave on each, none over the footer, and each page holds as many rows as its own area has room for.
void FlowTests::tableRowsStayInTheBodyThatHeadersAndFootersLeave()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 2, 5, {}, 10, 20);
    TestTable spec;
    spec.rows = 150;
    spec.cols = 2;
    spec.columnWidth = 140;
    spec.text = [](int r, int c) { return r % 9 == 4 ? QStringLiteral("r%1c%2\nand a second\nand a third line").arg(r).arg(c) : QStringLiteral("r%1c%2").arg(r).arg(c); };
    const TestTable table = addTable(fd, spec);
    appendParagraph(fd, QStringLiteral("After the table."));
    Section &s = fd.sections[0];
    s.differentFirstPage = s.differentOddEven = true;
    s.header.first = furnitureStory({QStringLiteral("FIRST HEAD")});
    s.header.odd = furnitureStory({QStringLiteral("ODD HEAD")});
    s.header.even = furnitureStory({QStringLiteral("EVEN HEAD 1"), QStringLiteral("EVEN HEAD 2"), QStringLiteral("EVEN HEAD 3"), QStringLiteral("EVEN HEAD 4"), QStringLiteral("EVEN HEAD 5"), QStringLiteral("EVEN HEAD 6")});
    s.footer.first = furnitureStory({QStringLiteral("FIRST FOOT 1"), QStringLiteral("FIRST FOOT 2"), QStringLiteral("FIRST FOOT 3")});
    s.footer.odd = furnitureStory({QStringLiteral("ODD FOOT")});
    s.footer.even = furnitureStory({QStringLiteral("EVEN FOOT 1"), QStringLiteral("EVEN FOOT 2"), QStringLiteral("EVEN FOOT 3"), QStringLiteral("EVEN FOOT 4"), QStringLiteral("EVEN FOOT 5")});
    Paginator pg(&fd);
    const int pages = pg.pageCount();
    QVERIFY(pages >= 6);
    QSet<int> heights;
    for (int k = 0; k < pages; ++k) heights << int(std::lround(pg.page(k).content.height() * 10));
    QVERIFY2(heights.size() >= 3, "the pages' text areas are of three heights (the first page's, the odd pages', the even pages')");
    const QString bad = audit::problems(pg, fd.story());
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    // Each page's furniture is the one its number and place say, and its rows end above the footer.
    QVector<int> seen;
    for (int k = 0; k < pages; ++k) {
        const auto page = pg.materialize(k);
        QVERIFY(page);
        const QString want = k == 0 ? QStringLiteral("FIRST") : (k + 1) % 2 == 0 ? QStringLiteral("EVEN") : QStringLiteral("ODD");
        const auto heads = audit::itemsWith(*page, QStringLiteral("flow-header-box")), foots = audit::itemsWith(*page, QStringLiteral("flow-footer-box"));
        QVERIFY2(heads.size() == 1 && foots.size() == 1, qPrintable(QString::number(k)));
        QVERIFY2(audit::storyOf(*page, heads[0]).startsWith(want + QStringLiteral(" HEAD")), qPrintable(QStringLiteral("page %1: %2").arg(k).arg(audit::storyOf(*page, heads[0]))));
        QVERIFY2(audit::storyOf(*page, foots[0]).startsWith(want + QStringLiteral(" FOOT")), qPrintable(QStringLiteral("page %1: %2").arg(k).arg(audit::storyOf(*page, foots[0]))));
        const double room = pg.page(k).content.height();
        const auto rows = rowLines(pg, fd, k);
        for (const auto &r : rows) seen << r.pos;
        if (!rows.isEmpty()) {
            QVERIFY2(rows.last().rect.bottom() <= room + 0.1, qPrintable(QStringLiteral("page %1: rows go down to %2 of %3").arg(k).arg(rows.last().rect.bottom()).arg(room)));
            const auto next = k + 1 < pages ? rowLines(pg, fd, k + 1) : QVector<StoryLayout::LineBrief>();
            if (!next.isEmpty() && next.first().pos == pg.lines(k + 1).first().pos)
                QVERIFY2(rows.last().rect.bottom() + next.first().rect.height() > room + 0.05, qPrintable(QStringLiteral("page %1 leaves room for the next row").arg(k)));
        }
    }
    QCOMPARE(seen, table.rowStarts);
}


// A numbered paragraph that begins with a page break has its number on the line after the break, which is the first of the next page, not on the break's own line at the foot of
// the page before (a chapter heading whose first run is a page break: its "1." was drawn nowhere).
void FlowTests::aNumberFollowsAPageBreakThatBeginsItsParagraph()
{
    auto fd = std::make_unique<FlowDocument>();
    fd->env.wordLines = true;
    fillStory(fd->story(), 2, 11, [&](int, QTextBlockFormat &bf) { bf.setBottomMargin(6); }, 8, 12);
    {
        QTextCursor c(fd->story());
        c.movePosition(QTextCursor::End);
        c.insertBlock();
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDecimal);
        lf.setNumberSuffix(QStringLiteral("."));
        lf.setIndent(1);
        c.createList(lf);
        QTextCharFormat body;
        body.setFontPointSize(11);
        c.insertText(QString(kPageBreak) + QStringLiteral("Heading text"), body);
    }
    Paginator pg(fd.get());
    QCOMPARE(pg.pageCount(), 2);
    auto markers = [&](int k) {
        QStringList out;
        const auto page = pg.materialize(k);
        if (!page || page->boxes.isEmpty()) return out;
        const auto *box = dynamic_cast<const TextItem *>(page->doc->item(page->boxes[0]));
        FrameSpec fs = plainFrame(box->rect.width(), 5000);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(page->doc->story(box->storyId)->doc.get(), {fs}, env);
        for (const auto &l : lay.lineInfo(0))
            if (!l.marker.isEmpty()) out << l.marker + QLatin1Char(' ') + l.text.trimmed();
        return out;
    };
    QCOMPARE(markers(0), QStringList());
    QCOMPARE(markers(1), (QStringList{QStringLiteral("1. Heading text")}));
}


// A word processor's column break ends the line it is on, and what follows it (the rest of the paragraph, and the paragraphs after) is in the next column, whatever room the column
// it is in has left: in two columns the text after the break is at the second column's left edge; in the last column it is past the frame (the next page's).
void FlowTests::aColumnBreakGoesOnInTheNextColumn()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Tinos")});
    run.setFontPointSize(11);
    c.insertText(QStringLiteral("Alpha") + QChar(0x000E) + QStringLiteral("Beta"), run);
    c.insertBlock();
    c.insertText(QStringLiteral("Gamma"), run);
    LayoutEnv env;
    env.wordLines = true;
    FrameSpec fs = plainFrame(300, 200);
    fs.columns = 2;
    fs.gap = 20;
    StoryLayout two;
    two.build(&doc, {fs}, env);
    const auto lines = two.lineBriefs();
    QCOMPARE(int(lines.size()), 3);
    const double second = (300 - 20) / 2.0 + 20;
    QVERIFY2(lines[0].rect.left() < 1, qPrintable(QString::number(lines[0].rect.left())));
    QVERIFY2(std::abs(lines[1].rect.left() - second) < 0.5, qPrintable(QStringLiteral("the text after the break is at %1, the second column is at %2").arg(lines[1].rect.left()).arg(second)));
    QVERIFY2(std::abs(lines[2].rect.left() - second) < 0.5, qPrintable(QStringLiteral("the next paragraph is at %1, the second column is at %2").arg(lines[2].rect.left()).arg(second)));
    QVERIFY(lines[2].rect.top() > lines[1].rect.top());
    // In a frame of one column, with none after it, the break is a line break: the text after it is where the lines go on, not lost (a table's cell, a text box).
    StoryLayout one;
    one.build(&doc, {plainFrame(300, 200)}, env);
    const auto single = one.lineBriefs();
    QCOMPARE(int(single.size()), 3);
    for (const auto &l : single) QCOMPARE(l.frame, 0);
    // With a frame after it (the next page's), it goes on there.
    StoryLayout pages;
    pages.build(&doc, {plainFrame(300, 200), plainFrame(300, 200)}, env);
    const auto after = pages.lineBriefs();
    QCOMPARE(int(after.size()), 3);
    QCOMPARE(after[0].frame, 0);
    QCOMPARE(after[1].frame, 1);
    QCOMPARE(after[2].frame, 1);
}


// A word processor's justified line that has no space before its last word (a long word alone in a narrow column or frame) is set as a left-aligned line is, not with its letters
// spread over the width (V107's "Percent" in a frame that was as wide as "1998-2008": "P e r c e n t").
void FlowTests::aJustifiedLineWithoutASpaceKeepsItsLettersTogether()
{
    auto placeOf = [&](Qt::Alignment al) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextBlockFormat bf;
        bf.setAlignment(al);
        c.setBlockFormat(bf);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Arimo")});
        run.setFontPointSize(10);
        run.setFontWeight(QFont::Bold);
        c.insertText(QStringLiteral("Percent Change 1998-2008"), run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(60, 200)}, env);
        int frame = -1;
        QRectF at6, at7;
        lay.caretRect(6, &frame, &at6);   // (before the "t" of "Percent")
        lay.caretRect(7, &frame, &at7);   // (after it)
        return QVector<double>{at6.x(), at7.x()};
    };
    const QVector<double> justified = placeOf(Qt::AlignJustify), left = placeOf(Qt::AlignLeft);
    QVERIFY2(left[1] < 55, "the word is narrower than the frame");
    for (int i = 0; i < 2; ++i) QVERIFY2(std::abs(justified[i] - left[i]) < 1.0, qPrintable(QStringLiteral("letter %1 is at %2 justified, %3 left-aligned").arg(i + 6).arg(justified[i]).arg(left[i])));   // (to a point: the spread was 23)
}


// A document whose first section begins on an even page numbers its first page 2 (the page before it is counted and nothing is drawn on it, as it is in a word processor's own
// pages), and its pages go on from there; one that begins on an odd page numbers it 1; a restart in the section is the number it says.
void FlowTests::aDocumentWhoseFirstSectionBeginsOnAnEvenPageNumbersItsFirstPageTwo()
{
    auto build = [&](Section::Parity parity, std::optional<int> restart) {
        auto fd = std::make_unique<FlowDocument>();
        fd->env.wordLines = true;
        fillStory(fd->story(), 3, 11, [&](int i, QTextBlockFormat &bf) {
            bf.setBottomMargin(6);
            if (i < 2) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);
        }, 8, 12);
        fd->sections[0].startParity = parity;
        fd->sections[0].firstPageNumber = restart;
        return fd;
    };
    auto numbers = [&](Paginator &pg) {
        QVector<int> out;
        for (int k = 0; k < pg.pageCount(); ++k) out << pg.page(k).number;
        return out;
    };
    for (const auto &[parity, restart, want] : {std::tuple{Section::Parity::Even, std::optional<int>(), QVector<int>{2, 3, 4}}, std::tuple{Section::Parity::Odd, std::optional<int>(), QVector<int>{1, 2, 3}},
                                                 std::tuple{Section::Parity::Any, std::optional<int>(), QVector<int>{1, 2, 3}}, std::tuple{Section::Parity::Even, std::optional<int>(7), QVector<int>{7, 8, 9}}}) {
        auto fd = build(parity, restart);
        Paginator pg(fd.get());
        QCOMPARE(pg.pageCount(), 3);
        QCOMPARE(numbers(pg), want);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        QCOMPARE(page->number, want[0]);
    }
}


// A section that begins on an odd (or even) page leaves a page of its own before it blank when the page after the break would be an even (or odd) one by its place in the
// document (a restart of the numbers in the new section is not counted); the blank page is a page of the section before, with no header or footer (as a word processor's own
// pages for such a break have none).
void FlowTests::aSectionOfOddOrEvenPagesLeavesABlankPageBeforeItWhereTheNumberIsWrong()
{
        // The first section has `pages` pages (a break after each paragraph but the last), the second one paragraph.
        auto build = [&](int pages, Section::Parity parity, bool restart) {
            auto fd = std::make_unique<FlowDocument>();
            fd->env.wordLines = true;
            fillStory(fd->story(), pages + 1, 11, [&](int i, QTextBlockFormat &bf) {
                bf.setBottomMargin(6);
                if (i < pages - 1) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);
                if (i == pages - 1) bf.setProperty(fp::SectionBreakAfter, true);
            }, 8, 12);
            Section second = fd->sections[0];
            second.startParity = parity;
            if (restart) second.firstPageNumber = 1;
            fd->sections << second;
            return fd;
        };
        auto numbers = [&](FlowDocument *fd, Paginator &pg) {
            QVector<int> out;
            for (int k = 0; k < pg.pageCount(); ++k) out << pg.page(k).number * 10 + pg.page(k).start.section;   // (the number, and the section: two digits)
            Q_UNUSED(fd);
            return out;
        };
        {   // One page, then a section on an odd page: the next number is 2, so a blank page (the section before's, numbered 2) and the section's page, which is 3.
            auto fd = build(1, Section::Parity::Odd, false);
            Paginator pg(fd.get());
            QCOMPARE(pg.pageCount(), 3);
            QCOMPARE(numbers(fd.get(), pg), (QVector<int>{10, 20, 31}));
            QVERIFY(pg.lines(1).isEmpty());
            const auto blank = pg.materialize(1);
            QVERIFY(blank);
            QVERIFY(blank->boxes.isEmpty());
            QVERIFY(!pg.lines(2).isEmpty());
            QVERIFY(pg.lines(2)[0].rect.top() < 1);
        }
        {   // The same with the numbers restarting at 1 in the new section: the blank page is still left (the numbers as they would go on), and the section's page is 1.
            auto fd = build(1, Section::Parity::Odd, true);
            Paginator pg(fd.get());
            QCOMPARE(pg.pageCount(), 3);
            QCOMPARE(numbers(fd.get(), pg), (QVector<int>{10, 20, 11}));
        }
        {   // One page, then an even page: the next number is 2, no blank page.
            auto fd = build(1, Section::Parity::Even, false);
            Paginator pg(fd.get());
            QCOMPARE(pg.pageCount(), 2);
            QCOMPARE(numbers(fd.get(), pg), (QVector<int>{10, 21}));
        }
        {   // Two pages, then an odd one: the next number is 3, none; then an even one: a blank page.
            auto odd = build(2, Section::Parity::Odd, false);
            Paginator po(odd.get());
            QCOMPARE(po.pageCount(), 3);
            auto even = build(2, Section::Parity::Even, false);
            Paginator pe(even.get());
            QCOMPARE(pe.pageCount(), 4);
            QCOMPARE(numbers(even.get(), pe), (QVector<int>{10, 20, 30, 41}));
        }
        {   // Where the pages are of odd and even kinds, a section that restarts the numbers at a number of the other kind than the page it begins (by its place in the
            // document) has a blank page before it too, with no header or footer: after one page, a restart at 1 (the page is the second); after two, a restart at 2.
            auto restartAt = [&](int pages, int number) {
                auto fd = build(pages, Section::Parity::Any, true);
                fd->sections[1].firstPageNumber = number;
                for (Section &sec : fd->sections) {
                    sec.differentOddEven = true;
                    sec.header.odd = furnitureStory({QStringLiteral("HEADER")});
                    sec.header.even = furnitureStory({QStringLiteral("HEADER")});
                }
                return fd;
            };
            auto one = restartAt(1, 1);
            Paginator p1(one.get());
            QCOMPARE(p1.pageCount(), 3);
            QCOMPARE(numbers(one.get(), p1), (QVector<int>{10, 20, 11}));
            const auto blank = p1.materialize(1);
            QVERIFY(blank);
            QVERIFY(blank->doc->pages[0]->items.empty());
            const auto before = p1.materialize(0), after = p1.materialize(2);
            QVERIFY(before && after);
            QVERIFY(!before->doc->pages[0]->items.empty() && !after->doc->pages[0]->items.empty());
            auto two = restartAt(2, 1);
            Paginator p2(two.get());
            QCOMPARE(p2.pageCount(), 3);   // (the third page is odd: a restart at 1 is the same kind)
            auto twoAt2 = restartAt(2, 2);
            Paginator p3(twoAt2.get());
            QCOMPARE(p3.pageCount(), 4);
            QCOMPARE(numbers(twoAt2.get(), p3), (QVector<int>{10, 20, 30, 21}));
            // (With the pages not of odd and even kinds, no blank page is left.)
            auto plain = restartAt(1, 1);
            for (Section &sec : plain->sections) sec.differentOddEven = false;
            Paginator pp(plain.get());
            QCOMPARE(pp.pageCount(), 2);
        }
        {   // A continuous section asks nothing of the page it is on.
            auto fd = build(1, Section::Parity::Odd, false);
            fd->sections[1].continuous = true;
            Paginator pg(fd.get());
            QCOMPARE(pg.pageCount(), 1);
        }
    }


// The pages are filled with the document's page color: a shape as large as the page, under everything else, on every page (a blank page too).
void FlowTests::thePageColorFillsEveryPageUnderTheText()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 3, 5, [](int i, QTextBlockFormat &bf) {
        bf.setBottomMargin(6);
        if (i == 0) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);
    }, 8, 12);
    fd.pageColor = QColor(0x00, 0xb8, 0xff);
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 2);
    for (int k = 0; k < pg.pageCount(); ++k) {
        const auto page = pg.materialize(k);
        QVERIFY(page);
        const auto &items = page->doc->pages[0]->items;
        QVERIFY(!items.empty());
        const auto *shape = dynamic_cast<const ShapeItem *>(items.front().get());
        QVERIFY2(shape, "the page's first item is not a shape");
        QVERIFY(shape->fill.type == Fill::Solid && shape->fill.color.resolve(page->doc->colors) == QColor(0x00, 0xb8, 0xff));
        QVERIFY(shape->stroke.isNone());
        QVERIFY2(std::abs(shape->rect.width() - page->size.width()) < 0.01 && std::abs(shape->rect.height() - page->size.height()) < 0.01 && shape->rect.topLeft() == QPointF(0, 0), qPrintable(audit::rectText(shape->rect)));
        QVERIFY(items.size() >= 2);   // (the text is over it)
    }
    // None by default.
    FlowDocument plain;
    plain.env.wordLines = true;
    fillStory(plain.story(), 1, 5, {}, 8, 12);
    Paginator pp(&plain);
    const auto page = pp.materialize(0);
    QVERIFY(page);
    for (const auto &it : page->doc->pages[0]->items) QVERIFY(!dynamic_cast<const ShapeItem *>(it.get()));
}


// The paragraphs of a frame in a header or footer (set out of the text, drawn where the next paragraph begins) take the room they are high together, and the story is as tall
// as the lowest of what it has: a frame of two lines beside a paragraph of one makes a header of two, and a frame alone makes one of its own height.
void FlowTests::aFramesLinesAreTheRoomItTakesInAHeaderAndAFooter()
{
    auto topWith = [&](const QStringList &lines, int frames, int after, bool footer = false) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 2, 3, {}, 8, 12);
        Section &sec = fd.sections[0];
        sec.headerDistance = sec.footerDistance = 60;
        auto story = furnitureStory(lines);
        int n = 0;
        for (QTextBlock b = story->begin(); b.isValid() && n < frames + after; b = b.next(), ++n) {
            if (n < frames) {
                QTextCursor c(b);
                QTextBlockFormat bf = b.blockFormat();
                bf.setProperty(tp::OutOfFlow, true);
                c.setBlockFormat(bf);
            }
        }
        (footer ? sec.footer.odd : sec.header.odd) = story;
        Paginator pg(&fd);
        const auto page = pg.page(0);
        return footer ? fd.section(0).pageSize().height() - page.content.bottom() : page.content.top();
    };
    const QStringList two = {QStringLiteral("one"), QStringLiteral("two")};
    for (const bool footer : {false, true}) {
        // The same two lines as ordinary paragraphs are the room two lines take; as a frame, with a third paragraph beside them, so do they.
        const double plain = topWith(two, 0, 0, footer);
        const double framed = topWith({QStringLiteral("one"), QStringLiteral("two"), QStringLiteral("three")}, 2, 0, footer);
        QVERIFY2(std::abs(framed - plain) < 0.01 && plain > 60, qPrintable(QStringLiteral("%1 against %2 (%3)").arg(framed).arg(plain).arg(footer ? "footer" : "header")));
        // A frame of one line alone: the room of one line.
        const double oneLine = topWith({QStringLiteral("one")}, 0, 0, footer);
        const double alone = topWith({QStringLiteral("one")}, 1, 0, footer);
        QVERIFY2(std::abs(alone - oneLine) < 0.01 && oneLine > 60, qPrintable(QStringLiteral("%1 against %2 (%3)").arg(alone).arg(oneLine).arg(footer ? "footer" : "header")));
    }
}


// The page borders of a section are lines round the page, under the text: from the page's edge or from the text's (with the header and the footer inside), each side
// as far out as its space says, the line's own width in from there; on every page, or the section's first, or the others.
void FlowTests::aSectionsPageBordersAreLinesRoundItsPages()
{
        auto makeBorders = [](bool fromPage, PageBorders::Show show) {
            PageBorders b;
            b.fromPage = fromPage;
            b.show = show;
            for (PageBorders::Side *side : {&b.top, &b.left, &b.bottom, &b.right}) {
                side->present = true;
                side->width = 2;
                side->space = 10;
                side->color = QColor(0, 0, 0x80);
            }
            b.right.space = 20;   // (each side has its own)
            return b;
        };
        auto linesOf = [&](const std::unique_ptr<MaterializedPage> &page) {
            QVector<const LineItem *> lines;
            for (const auto &it : page->doc->pages[0]->items)
                if (const auto *l = dynamic_cast<const LineItem *>(it.get()))
                    if (l->id.startsWith(QLatin1String("flow-page-border-"))) lines << l;
            return lines;
        };
        auto build = [&](bool fromPage, PageBorders::Show show, bool header) {
            auto fd = std::make_unique<FlowDocument>();
            fd->env.wordLines = true;
            fillStory(fd->story(), 4, 5, [](int i, QTextBlockFormat &bf) {
                bf.setBottomMargin(6);
                if (i == 0) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);
            }, 8, 12);
            Section &sec = fd->sections[0];
            sec.margins = QMarginsF(90, 80, 70, 60);
            sec.headerDistance = 30;
            sec.footerDistance = 20;
            if (header) {
                sec.header.odd = furnitureStory({QStringLiteral("HEADER")});
                sec.footer.odd = furnitureStory({QStringLiteral("FOOTER")});
            }
            sec.pageBorders = makeBorders(fromPage, show);
            return fd;
        };
        {   // From the page's edge: the lines' outer edges are `space` in, so the centers `space` and half the width in.
            auto fd = build(true, PageBorders::Show::All, false);
            Paginator pg(fd.get());
            QVERIFY(pg.pageCount() >= 2);
            const Section &sec = fd->section(0);
            const double w = sec.pageSize().width(), h = sec.pageSize().height();
            const auto page = pg.materialize(0), second = pg.materialize(1);
            QVERIFY(page && second);
            QCOMPARE(int(linesOf(page).size()), 4);
            QCOMPARE(int(linesOf(second).size()), 4);
            QVector<const LineItem *> l = linesOf(page);
            auto horizontalAt = [&](double y) { for (const LineItem *x : l) if (std::abs(x->p1.y() - y) < 0.01 && std::abs(x->p2.y() - y) < 0.01) return true; return false; };
            auto verticalAt = [&](double x) { for (const LineItem *i : l) if (std::abs(i->p1.x() - x) < 0.01 && std::abs(i->p2.x() - x) < 0.01) return true; return false; };
            QVERIFY(horizontalAt(10 + 1) && horizontalAt(h - 10 - 1));
            QVERIFY(verticalAt(10 + 1) && verticalAt(w - 20 - 1));
            for (const LineItem *x : l) QVERIFY2(x->stroke.width == 2 && x->stroke.color.resolve(page->doc->colors) == QColor(0, 0, 0x80), "a border's stroke");
            // The lines are under the text: before any text box of the page.
            const auto &items = page->doc->pages[0]->items;
            int lastLine = -1, firstText = int(items.size());
            for (int i = 0; i < int(items.size()); ++i) {
                if (items[size_t(i)]->type() == ItemType::Line) lastLine = i;
                else if (items[size_t(i)]->type() == ItemType::Text) firstText = std::min(firstText, i);
            }
            QVERIFY(lastLine >= 0 && lastLine < firstText);
        }
        {   // From the text: outside its edges (the header and the footer are inside, as high and as low as their distances from the page's edge).
            auto fd = build(false, PageBorders::Show::All, true);
            Paginator pg(fd.get());
            const Section &sec = fd->section(0);
            const double w = sec.pageSize().width(), h = sec.pageSize().height();
            const auto page = pg.materialize(0);
            QVERIFY(page);
            const QVector<const LineItem *> l = linesOf(page);
            QCOMPARE(int(l.size()), 4);
            auto horizontalAt = [&](double y) { for (const LineItem *x : l) if (std::abs(x->p1.y() - y) < 0.01 && std::abs(x->p2.y() - y) < 0.01) return true; return false; };
            auto verticalAt = [&](double x) { for (const LineItem *i : l) if (std::abs(i->p1.x() - x) < 0.01 && std::abs(i->p2.x() - x) < 0.01) return true; return false; };
            QVERIFY2(horizontalAt(30 - 10 - 1) && horizontalAt(h - 20 + 10 + 1), "the top and bottom lines");   // (the header and the footer are nearer the edge than the margins)
            QVERIFY2(verticalAt(90 - 10 - 1) && verticalAt(w - 70 + 20 + 1), "the left and right lines");
        }
        {   // The first page of the section only, and the pages but the first.
            auto first = build(true, PageBorders::Show::FirstPage, false);
            Paginator pf(first.get());
            QVERIFY(pf.pageCount() >= 2);
            QCOMPARE(int(linesOf(pf.materialize(0)).size()), 4);
            QCOMPARE(int(linesOf(pf.materialize(1)).size()), 0);
            auto rest = build(true, PageBorders::Show::NotFirstPage, false);
            Paginator pr(rest.get());
            QCOMPARE(int(linesOf(pr.materialize(0)).size()), 0);
            QCOMPARE(int(linesOf(pr.materialize(1)).size()), 4);
        }
    }


// The first page of a section is the page it begins at the top of: the page after the one a continuous section began in the middle of has the section's ordinary header, not
// its first page's.
void FlowTests::aSectionThatBeganInTheMiddleOfAPageIsNotOnItsFirstPageAfterIt()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 60, 4, [](int i, QTextBlockFormat &bf) {
        bf.setBottomMargin(6);
        if (i == 1) bf.setProperty(fp::SectionBreakAfter, true);
    }, 30, 40);
    Section &one = fd.sections[0];
    one.differentFirstPage = true;
    one.header.first = furnitureStory({QStringLiteral("HEADER FIRST")});
    one.header.odd = furnitureStory({QStringLiteral("HEADER ORDINARY")});
    Section two = one;
    two.continuous = true;
    two.margins.setLeft(one.margins.left() + 30);
    fd.sections << two;
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 3);
    auto headerText = [&](int k) {
        const auto page = pg.materialize(k);
        QString text;
        for (const auto &it : page->doc->pages[0]->items)
            if (const auto *t = dynamic_cast<const TextItem *>(it.get()))
                if (t->id.startsWith(QLatin1String("flow-header-box-"))) text = page->doc->story(t->storyId)->doc->toPlainText();
        return text;
    };
    QCOMPARE(headerText(0), QStringLiteral("HEADER FIRST"));
    QCOMPARE(headerText(1), QStringLiteral("HEADER ORDINARY"));   // (the second section began on the first page)
    QCOMPARE(headerText(2), QStringLiteral("HEADER ORDINARY"));
}


// A paragraph that asks to be no higher than so many points from the page's top (fp::AdvanceTop) has its first line there when it would be higher, with the text after it
// below; where it would be lower already, it stays where it is. The pages made for painting have the same lines.
void FlowTests::aParagraphAskedToBeLowerOnThePageBeginsAtThePlaceAsked()
{
    auto build = [&](double asked) {
        auto fd = std::make_unique<FlowDocument>();
        fd->env.wordLines = true;
        fillStory(fd->story(), 3, 8, [&](int i, QTextBlockFormat &bf) {
            bf.setBottomMargin(6);
            if (i == 1 && asked > 0) bf.setProperty(fp::AdvanceTop, asked);
        }, 8, 12);
        return fd;
    };
    auto lineTops = [&](double asked) {
        auto fd = build(asked);
        Paginator pg(fd.get());
        const double areaTop = pg.page(0).content.top();
        QVector<double> tops;
        for (const auto &l : pg.lines(0))
            if (l.line == 0) tops << areaTop + l.rect.top();
        return tops;
    };
    const QVector<double> plain = lineTops(0);
    QCOMPARE(int(plain.size()), 3);
    // Asked for a place lower than where the second paragraph is: its first line is there, and the third paragraph is as far below as it was.
    const double asked = plain[1] + 80;
    const QVector<double> moved = lineTops(asked);
    QCOMPARE(int(moved.size()), 3);
    QVERIFY2(std::abs(moved[0] - plain[0]) < 0.01, "the paragraph before is where it was");
    QVERIFY2(std::abs(moved[1] - asked) < 0.01, qPrintable(QStringLiteral("%1 for the place %2").arg(moved[1]).arg(asked)));
    QVERIFY2(std::abs((moved[2] - moved[1]) - (plain[2] - plain[1])) < 0.01, "the paragraph after is as far below as it was");
    // A place higher than where it is already moves nothing.
    const QVector<double> kept = lineTops(plain[1] - 5);
    QVERIFY2(std::abs(kept[1] - plain[1]) < 0.01 && std::abs(kept[2] - plain[2]) < 0.01, "a higher place moves nothing");
    // The page made for painting has the same lines.
    auto fd = build(asked);
    Paginator pg(fd.get());
    LayoutCache cache;
    const QString differs = materializedDiffers(*fd, pg, 0, cache);
    QVERIFY2(differs.isEmpty(), qPrintable(differs));
    // Two paragraphs that ask: each is at its own place (the first at no more than the second's), the second lower than the first by what the place says.
    FlowDocument two;
    two.env.wordLines = true;
    fillStory(two.story(), 3, 8, [&](int i, QTextBlockFormat &bf) {
        bf.setBottomMargin(6);
        if (i == 0) bf.setProperty(fp::AdvanceTop, plain[0] + 30);
        if (i == 1) bf.setProperty(fp::AdvanceTop, plain[1] + 90);
    }, 8, 12);
    Paginator pt(&two);
    const double areaTop = pt.page(0).content.top();
    QVector<double> placed;
    for (const auto &l : pt.lines(0))
        if (l.line == 0) placed << areaTop + l.rect.top();
    QCOMPARE(int(placed.size()), 3);
    QVERIFY2(std::abs(placed[0] - (plain[0] + 30)) < 0.01 && std::abs(placed[1] - (plain[1] + 90)) < 0.01,
             qPrintable(QStringLiteral("at %1 and %2, asked for %3 and %4").arg(placed[0]).arg(placed[1]).arg(plain[0] + 30).arg(plain[1] + 90)));
    QVERIFY2(std::abs((placed[2] - placed[1]) - (plain[2] - plain[1])) < 0.01, "the paragraph after is as far below as it was");
}


// The pages after an edit are as they are afresh where a section begins on an odd page: an edit that adds a page before it moves it to an even place (or back), and the blank
// page it needs comes (or goes), though the pages after it are where they were but one.
void FlowTests::aBlankPageForAnOddPageSectionFollowsEditsThatChangeThePagesBefore()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 3, 6, [](int i, QTextBlockFormat &bf) {
        bf.setBottomMargin(6);
        if (i < 2) bf.setProperty(fp::SectionBreakAfter, true);
    }, 8, 12);
    Section b = fd.sections[0], c = fd.sections[0];
    c.startParity = Section::Parity::Odd;
    fd.sections << b << c;
    Paginator pg(&fd);
    QCOMPARE(pg.pageCount(), 3);   // (the third section would begin the third page: an odd one)
    QTextCursor cur(fd.story());
    cur.setPosition(4);
    cur.insertText(QString(kPageBreak));   // (a page break typed into the first section: the third section would begin the fourth page)
    QString bad = differenceFromFresh(fd, pg);
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    QCOMPARE(pg.pageCount(), 5);   // (the first section's two pages, the second's, the blank page, the third's)
    cur.setPosition(4);
    cur.deleteChar();
    bad = differenceFromFresh(fd, pg);
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    QCOMPARE(pg.pageCount(), 3);
}


// The space before a paragraph is there at the top of the page that a break which begins the paragraph opens, and not after a break that has something before it on its
// paragraph's first line: of "break, a, break, b" the second page has it and the third has not.
void FlowTests::theSpaceBeforeIsAfterTheBreakThatBeginsTheParagraphOnly()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    QTextCursor c(fd.story());
    QTextBlockFormat bf;
    bf.setTopMargin(20);
    c.setBlockFormat(bf);
    c.insertText(QString(flow::kPageBreak) + QStringLiteral("a") + QString(flow::kPageBreak) + QStringLiteral("b"));
    c.insertBlock(bf);
    c.insertText(QStringLiteral("a paragraph after"));
    Paginator pg(&fd);
    QCOMPARE(pg.pageCount(), 3);
    const auto second = pg.lines(1), third = pg.lines(2);
    QVERIFY(!second.isEmpty() && !third.isEmpty());
    QVERIFY2(std::abs(second[0].rect.top() - 20) < 0.01, qPrintable(QStringLiteral("the second page's first line is at %1").arg(second[0].rect.top())));
    QVERIFY2(std::abs(third[0].rect.top()) < 0.01, qPrintable(QStringLiteral("the third page's first line is at %1").arg(third[0].rect.top())));
    // The pages made for painting have the same lines.
    LayoutCache cache;
    for (int k = 1; k < 3; ++k) {
        const QString differs = materializedDiffers(fd, pg, k, cache);
        QVERIFY2(differs.isEmpty(), qPrintable(differs));
    }
}


// The columns above a continuous section are as short as their lines let them be: the lowest of them reaches no further than the half of the height that the same text has in
// one column of their width, and a line or two more; and they are not shorter than that half less a line.
void FlowTests::theColumnsAboveAContinuousSectionAreAsShortAsTheirLinesLetThemBe()
{
    Section measure;
    measure.columns = 2;
    measure.columnGap = 20;
    const double columnWidth = measure.columnWidth();
    for (const int paragraphs : {2, 3, 5}) {
        // The text in one column of that width (the margins say how wide).
        FlowDocument single;
        single.env.wordLines = true;
        fillStory(single.story(), paragraphs, 11, [](int, QTextBlockFormat &bf) { bf.setBottomMargin(6); }, 30, 45);
        single.sections[0].margins.setRight(single.sections[0].pageSize().width() - single.sections[0].margins.left() - columnWidth);
        Paginator one(&single);
        QCOMPARE(one.pageCount(), 1);
        double whole = 0, tallest = 0;
        for (const auto &l : one.lines(0)) {
            whole = std::max(whole, l.rect.bottom());
            tallest = std::max(tallest, l.rect.height());
        }
        // The same text in two columns that a section of one column follows.
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), paragraphs + 1, 11, [&](int i, QTextBlockFormat &bf) {
            bf.setBottomMargin(6);
            if (i == paragraphs - 1) bf.setProperty(fp::SectionBreakAfter, true);
        }, 30, 45);
        fd.sections[0].columns = 2;
        fd.sections[0].columnGap = 20;
        Section after = fd.sections[0];
        after.continuous = true;
        after.columns = 1;
        fd.sections << after;
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 1);
        double reach = 0;
        for (const auto &l : pg.lines(0))
            if (l.frame < 2) reach = std::max(reach, l.rect.bottom());
        QVERIFY2(reach <= whole / 2 + 2 * tallest + 6, qPrintable(QStringLiteral("%1 paragraphs: the columns reach %2, the text in one column is %3 high").arg(paragraphs).arg(reach).arg(whole)));
        QVERIFY2(reach >= whole / 2 - tallest, qPrintable(QStringLiteral("%1 paragraphs: the columns reach %2, half of %3 less a line is more").arg(paragraphs).arg(reach).arg(whole)));
    }
}


// A continuous section break does not end the page when the sections' sheets are the same, whatever their margins: the text of the second begins below the text of the
// first, between the second's own side margins (its top and bottom margins are the page's of the page it begins at the top of).
void FlowTests::aContinuousSectionWithOtherMarginsBeginsOnThePageBelowTheTextAbove()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 4, 7, [](int i, QTextBlockFormat &bf) {
        bf.setBottomMargin(6);
        if (i == 1) bf.setProperty(fp::SectionBreakAfter, true);
    }, 8, 12);
    Section second = fd.sections[0];
    second.continuous = true;
    second.margins.setLeft(fd.sections[0].margins.left() + 90);
    second.margins.setRight(fd.sections[0].margins.right() + 40);
    second.margins.setTop(fd.sections[0].margins.top() + 30);
    fd.sections << second;
    Paginator pg(&fd);
    QCOMPARE(pg.pageCount(), 1);
    const auto page = pg.materialize(0);
    QVERIFY(page);
    QCOMPARE(int(page->boxes.size()), 2);
    const Section &a = fd.section(0), &b = fd.section(1);
    auto *first = dynamic_cast<TextItem *>(page->doc->item(page->boxes[0]));
    auto *next = dynamic_cast<TextItem *>(page->doc->item(page->boxes[1]));
    QVERIFY(first && next);
    QVERIFY2(std::abs(first->rect.left() - a.content().left()) < 0.01 && std::abs(first->rect.right() - a.content().right()) < 0.01, qPrintable(audit::rectText(first->rect)));
    QVERIFY2(std::abs(next->rect.left() - b.content().left()) < 0.01 && std::abs(next->rect.right() - b.content().right()) < 0.01, qPrintable(audit::rectText(next->rect)));
    // The text above ends where the text below begins (the boxes do not overlap), and the page's top and bottom margins are those of the section the page begins in, not the
    // second's own (it has more at the top).
    QVERIFY2(next->rect.top() >= first->rect.bottom() - 0.01, qPrintable(QStringLiteral("the boxes overlap: %1 and %2").arg(audit::rectText(first->rect), audit::rectText(next->rect))));
    QVERIFY2(std::abs(first->rect.top() - a.content().top()) < 0.01, "the first box is not at the page's top margin");
    QVERIFY2(next->rect.bottom() <= a.content().bottom() + 0.01, "the second box goes below the page's bottom margin");
    QCOMPARE(pg.page(0).content.top(), a.content().top());
    QCOMPARE(pg.page(0).content.bottom(), a.content().bottom());
    // (The sheets that are not alike do end the page.)
    FlowDocument other;
    other.env.wordLines = true;
    fillStory(other.story(), 4, 7, [](int i, QTextBlockFormat &bf) {
        bf.setBottomMargin(6);
        if (i == 1) bf.setProperty(fp::SectionBreakAfter, true);
    }, 8, 12);
    Section turned = other.sections[0];
    turned.continuous = true;
    turned.orientation = Section::Orientation::Landscape;
    other.sections << turned;
    Paginator po(&other);
    QCOMPARE(po.pageCount(), 2);
}


// A page that three sections begin on, one under another (one column, two, one), is laid out in a part for each: the section of two columns has its columns as even as its lines let
// them be, and the section after it begins below both, as wide as the page.
void FlowTests::aPageOfThreeSectionsHasAPartForEach()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 13, 9, [](int i, QTextBlockFormat &bf) {
        bf.setBottomMargin(4);
        if (i == 1 || i == 9) bf.setProperty(fp::SectionBreakAfter, true);
    }, 3, 4);
    fd.sections[0].paper = QSizeF(300, 400);
    fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
    Section two = fd.sections[0], three = fd.sections[0];
    two.continuous = true;
    two.columns = 2;
    two.columnGap = 20;
    three.continuous = true;
    fd.sections << two << three;
    Paginator pg(&fd);
    QCOMPARE(pg.pageCount(), 1);
    const auto page = pg.materialize(0);
    QVERIFY(page);
    QCOMPARE(int(page->boxes.size()), 4);   // (one box for the first section's column, one for each of the second's, and one for the third's)
    std::vector<const TextItem *> boxes;
    for (const QString &id : page->boxes) boxes.push_back(dynamic_cast<const TextItem *>(page->doc->item(id)));
    for (const TextItem *b : boxes) QVERIFY(b);
    // (A full line, as tall as the tallest the page has: the columns are as even as the lines let them be.)
    double line = 0;
    for (const auto &l : pg.lines(0)) line = std::max(line, l.rect.height());
    QVERIFY(line > 0);
    QVERIFY2(std::abs(boxes[1]->rect.height() - boxes[2]->rect.height()) <= line, qPrintable(QStringLiteral("the columns are %1 and %2 high").arg(boxes[1]->rect.height()).arg(boxes[2]->rect.height())));
    QVERIFY2(boxes[1]->rect.height() > 0 && boxes[2]->rect.height() > 0, "both columns have lines");
    QVERIFY(boxes[1]->rect.left() < boxes[2]->rect.left());
    QVERIFY(std::abs(boxes[1]->rect.width() - boxes[2]->rect.width()) < 0.5 && boxes[1]->rect.width() < boxes[0]->rect.width() * 0.6);
    QVERIFY2(boxes[3]->rect.top() >= std::max(boxes[1]->rect.bottom(), boxes[2]->rect.bottom()) - 0.5, qPrintable(QStringLiteral("the third section begins at %1, the columns end at %2 and %3")
                                                                                                                   .arg(boxes[3]->rect.top()).arg(boxes[1]->rect.bottom()).arg(boxes[2]->rect.bottom())));
    QVERIFY(std::abs(boxes[3]->rect.width() - boxes[0]->rect.width()) < 0.5);
    QVERIFY(boxes[3]->rect.top() >= boxes[0]->rect.bottom());
    // Every line of the text is on the page, once.
    QCOMPARE(int(pg.lines(0).size()) > 0, true);
    const QString bad = audit::problems(pg, fd.story());
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
}


// More sections begin on one page than the parts a page is laid out in (sixteen): the page is laid out in as many as it may, the text of the rest goes on in the last part's
// columns, and every paragraph of the story is on some page.
void FlowTests::aPageOfSeventeenSectionsIsBoundedAndShowsEveryParagraph()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    const int sections = 17;
    fillStory(fd.story(), sections * 2, 3, [&](int i, QTextBlockFormat &bf) {
        bf.setBottomMargin(2);
        if (i % 2 == 1 && i / 2 < sections - 1) bf.setProperty(fp::SectionBreakAfter, true);
    }, 3, 5);
    fd.sections[0].paper = QSizeF(300, 700);
    fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
    for (int k = 1; k < sections; ++k) {
        Section next = fd.sections[0];
        next.continuous = true;
        next.columns = k % 2 ? 2 : 1;
        next.columnGap = 12;
        fd.sections << next;
    }
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 1 && pg.pageCount() <= 4);
    QSet<int> shown;
    for (int k = 0; k < pg.pageCount(); ++k) {
        QVERIFY(pg.materialize(k));
        for (const auto &l : pg.lines(k)) shown.insert(l.block);
    }
    for (QTextBlock b = fd.story()->begin(); b.isValid(); b = b.next()) QVERIFY2(shown.contains(b.position()), qPrintable(QStringLiteral("the paragraph at %1 is on no page").arg(b.position())));
}


// A page is drawn from a copy of its text, whose paragraphs the copy numbers from its own first; a run of paragraphs joined by hidden marks is cut every kMostJoinedParagraphs by
// the document's numbers, and the copy cuts it where the document does: when a page begins inside a run of seventy, it shows the lines the paginator counted.
void FlowTests::aPagesCopyCutsARunOfJoinedParagraphsWhereTheDocumentDoes()
{
    int insideEver = 0;
    for (const double height : {200.0, 206.0, 212.0, 218.0, 224.0, 230.0, 236.0, 242.0}) {
        FlowDocument fd;
        fd.env.wordLines = true;
        {
            QTextCursor c(fd.story());
            QTextCharFormat run;
            run.setFontFamilies({QStringLiteral("Times New Roman")});
            run.setFontPointSize(11);
            QTextCharFormat mark = run;
            mark.setProperty(tp::HiddenText, true);
            for (int i = 0; i < 70; ++i) {
                if (i == 0) c.setBlockCharFormat(mark);
                else c.insertBlock(QTextBlockFormat(), mark);
                c.insertText(i == 63 ? QStringLiteral("end") : QStringLiteral("alpha beta gamma delta %1").arg(i), run);   // (the second chunk of the run ends in a short line)
            }
        }
        fd.sections[0].paper = QSizeF(200, height);
        fd.sections[0].margins = QMarginsF(30, 30, 30, 30);
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 4);
        // (Pages begin inside the second chunk of the run, its paragraphs 32 to 63: their copies begin there.)
        for (int k = 1; k < pages; ++k) {
            const int number = fd.story()->findBlock(pg.page(k).start.pos).blockNumber();
            insideEver += number >= 33 && number <= 63;
        }
        // The page layer sets each page's text, from its copy, where the paginator's lines are.
        for (int k = 0; k < pages; ++k) {
            const auto m = pg.materialize(k);
            QVERIFY(m);
            const QString bad = audit::pageLayerProblems(pg, fd.story(), k, *m);
            QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("page height %1: %2").arg(height).arg(bad)));
        }
    }
    QVERIFY(insideEver > 4);
}


// A section's line numbers (a number in the margin for each line, or each fifth, from its start, again at each page) are drawn left of the column, at the lines' own places,
// for the lines of the section's text: not for a table's rows.
void FlowTests::aSectionsLineNumbersAreDrawnInTheMarginAtEachLine()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 24, 4, [](int, QTextBlockFormat &bf) { bf.setBottomMargin(0); }, 6, 12);
    fd.sections[0].paper = QSizeF(240, 200);
    fd.sections[0].margins = QMarginsF(60, 30, 30, 30);
    fd.sections[0].lineNumbers.countBy = 1;
    fd.sections[0].lineNumbers.start = 1;
    fd.sections[0].lineNumbers.distance = 10;
    fd.sections[0].lineNumbers.format.setFontPointSize(7);
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 3);
    for (int k = 0; k < pg.pageCount(); ++k) {
        const auto page = pg.materialize(k);
        QVERIFY(page);
        QVector<const TextItem *> numbers;
        for (const auto &item : page->doc->pages[0]->items)
            if (item->id.startsWith(QLatin1String("flow-lineno-box"))) numbers.push_back(dynamic_cast<const TextItem *>(item.get()));
        const auto briefs = pg.lines(k);   // (kept: an element of a temporary list is gone when the statement ends)
        const int lines = int(briefs.size());
        QCOMPARE(int(numbers.size()), lines);
        for (int i = 0; i < numbers.size(); ++i) {
            QVERIFY(numbers[i]);
            QCOMPARE(page->doc->stories.value(numbers[i]->storyId)->doc->toPlainText(), QString::number(i + 1));   // (again from 1 at each page)
            QVERIFY2(numbers[i]->rect.right() <= 60 - 10 + 0.01, qPrintable(QString::number(numbers[i]->rect.right())));   // (the column's left, less the distance)
            const auto &line = briefs[i];
            QVERIFY2(numbers[i]->rect.top() < pg.page(k).content.top() + line.rect.bottom() && numbers[i]->rect.bottom() > pg.page(k).content.top() + line.rect.top(), "the number is at its line");
        }
    }
    // Every fifth line, when the section asks.
    fd.sections[0].lineNumbers.countBy = 5;
    Paginator five(&fd);
    const auto page = five.materialize(0);
    QVERIFY(page);
    QStringList shown;
    for (const auto &item : page->doc->pages[0]->items)
        if (item->id.startsWith(QLatin1String("flow-lineno-box"))) shown << page->doc->stories.value(dynamic_cast<const TextItem *>(item.get())->storyId)->doc->toPlainText();
    QVERIFY(shown.size() >= 2);
    for (int i = 0; i < shown.size(); ++i) QCOMPARE(shown[i], QString::number(5 * (i + 1)));
}

void FlowTests::aDocumentWithNoSectionsIsPaginatedWithoutNumbersForItsLines()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 24, 4, [](int, QTextBlockFormat &bf) { bf.setBottomMargin(0); }, 6, 12);
    fd.sections.clear();
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 1);
    const auto page = pg.materialize(0);
    QVERIFY(page);
    for (const auto &item : page->doc->pages[0]->items) QVERIFY(!item->id.startsWith(QLatin1String("flow-lineno-box")));
}


// Line numbers that run on (w:restart "continuous") go through the pages: a page's first line has the number after the last of the page before.
void FlowTests::continuousLineNumbersRunOnThroughThePages()
{
    FlowDocument fd;
    numberedStory(fd, 24, 1, Section::LineNumbers::Restart::Continuous);
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 3);
    int next = 1;
    for (int k = 0; k < pg.pageCount(); ++k) {
        const QStringList shown = lineNumbersOnPage(pg, k);
        const int lines = int(pg.lines(k).size());
        QVERIFY2(int(shown.size()) == lines, qPrintable(QStringLiteral("page %1 has %2 numbers for %3 lines").arg(k).arg(shown.size()).arg(lines)));
        for (int i = 0; i < shown.size(); ++i) QCOMPARE(shown[i], QString::number(next + i));
        next += lines;
    }
}


// Line numbers that begin again at each section (w:restart "newSection") do so at the section's first line, wherever it is on the page, and go on across the section's pages.
void FlowTests::newSectionLineNumbersBeginAgainAtEachSection()
{
    // (The paragraph that ends the first section is one of those that leave the second to begin in the middle of a page.)
    bool beganMidPage = false;
    for (int endsFirst = 9; endsFirst < 20 && !beganMidPage; ++endsFirst) {
        FlowDocument fd;
        numberedStory(fd, 24, 1, Section::LineNumbers::Restart::NewSection);
        QTextCursor c(fd.story()->findBlockByNumber(endsFirst));
        QTextBlockFormat bf = c.blockFormat();
        bf.setProperty(fp::SectionBreakAfter, true);
        c.setBlockFormat(bf);
        const int secondStart = fd.story()->findBlockByNumber(endsFirst + 1).position();
        Section second = fd.sections[0];
        second.continuous = true;
        fd.sections << second;
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 3);
        int count[2] = {0, 0};
        for (int k = 0; k < pg.pageCount(); ++k) {
            const QStringList shown = lineNumbersOnPage(pg, k);
            const auto briefs = pg.lines(k);
            QVERIFY2(shown.size() == briefs.size(), qPrintable(QStringLiteral("page %1 has %2 numbers for %3 lines").arg(k).arg(shown.size()).arg(briefs.size())));
            for (int i = 0; i < briefs.size(); ++i) {
                const int section = briefs[i].pos >= secondStart ? 1 : 0;
                QCOMPARE(shown[i], QString::number(++count[section]));
                beganMidPage |= section == 1 && count[1] == 1 && i > 0;
            }
        }
        QVERIFY(count[0] > 5 && count[1] > 5);
    }
    QVERIFY(beganMidPage);
}


// Every fifth line, counted from the first of the document, whatever page it falls on.
void FlowTests::lineNumbersByFiveGoOnAcrossAPage()
{
    FlowDocument fd;
    numberedStory(fd, 24, 5, Section::LineNumbers::Restart::Continuous);
    // (A page height at which the first page holds a number of lines that is no multiple of five, so that a page begins between two fifths.)
    std::unique_ptr<Paginator> held;
    for (double height = 185; height < 260 && !(held && int(held->lines(0).size()) % 5 != 0); height += 5) {
        fd.sections[0].paper = QSizeF(240, height);
        held = std::make_unique<Paginator>(&fd);
    }
    Paginator &pg = *held;
    QVERIFY(pg.pageCount() >= 3);
    QVERIFY(int(pg.lines(0).size()) % 5 != 0);
    int before = 0, midway = 0;
    for (int k = 0; k < pg.pageCount(); ++k) {
        const QStringList shown = lineNumbersOnPage(pg, k);
        const int lines = int(pg.lines(k).size());
        QStringList want;
        for (int line = before + 1; line <= before + lines; ++line)
            if (line % 5 == 0) want << QString::number(line);
        QVERIFY2(shown == want, qPrintable(QStringLiteral("page %1 shows %2, not %3").arg(k).arg(shown.join(',')).arg(want.join(','))));
        midway += before % 5 != 0;
        before += lines;
    }
    QVERIFY(midway > 0);   // (a page begins between two fifths)
}


// A paragraph that suppresses the numbers is neither numbered nor counted, where it runs over a page's foot as well.
void FlowTests::aSuppressedParagraphIsNeitherNumberedNorCountedAcrossAPage()
{
    FlowDocument fd;
    numberedStory(fd, 24, 1, Section::LineNumbers::Restart::Continuous);
    QVector<int> suppressed;
    {
        Paginator first(&fd);
        QVERIFY(first.pageCount() >= 3);
        const auto last = first.lines(0);
        const auto next = first.lines(1);
        QVERIFY(!last.isEmpty() && !next.isEmpty());
        suppressed << last.last().block << next.first().block;
    }
    for (int position : suppressed) {
        QTextCursor c(fd.story()->findBlock(position));
        QTextBlockFormat bf = c.blockFormat();
        bf.setProperty(fp::SuppressLineNumbers, true);
        c.setBlockFormat(bf);
    }
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 3);
    int next = 1, skipped = 0;
    for (int k = 0; k < pg.pageCount(); ++k) {
        const QStringList shown = lineNumbersOnPage(pg, k);
        const auto briefs = pg.lines(k);
        int counted = 0;
        for (const auto &line : briefs) {
            if (fd.story()->findBlock(line.block).blockFormat().boolProperty(fp::SuppressLineNumbers)) ++skipped;
            else ++counted;
        }
        QVERIFY2(shown.size() == counted, qPrintable(QStringLiteral("page %1 has %2 numbers for %3 counted lines").arg(k).arg(shown.size()).arg(counted)));
        for (int i = 0; i < shown.size(); ++i) QCOMPARE(shown[i], QString::number(next++));
    }
    QVERIFY(skipped >= 2);
}


// An edit that adds a tall line on the first page (fewer lines fit before the third page's) moves the numbers on the third, and they are the ones a paginator that never saw the
// edit gives.
void FlowTests::anEditOnTheFirstPageMovesTheLineNumbersOnTheThird()
{
    FlowDocument fd;
    numberedStory(fd, 24, 1, Section::LineNumbers::Restart::Continuous);
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 4);
    const QStringList before = lineNumbersOnPage(pg, 2);
    QVERIFY(!before.isEmpty());
    QTextCursor c(fd.story());
    c.setPosition(0);
    QTextCharFormat big;
    big.setFontPointSize(30);
    c.insertText(QStringLiteral("A heading\n"), big);
    pg.pageCount();
    const QStringList after = lineNumbersOnPage(pg, 2);
    int linesBefore = 0;
    for (int k = 0; k < 2; ++k) linesBefore += int(pg.lines(k).size());
    QVERIFY(!after.isEmpty());
    QCOMPARE(after.first(), QString::number(linesBefore + 1));
    QVERIFY2(after.first() != before.first(), qPrintable(before.first()));
    Paginator fresh(&fd);
    for (int k = 0; k < std::min(pg.pageCount(), fresh.pageCount()); ++k) {
        const QStringList kept = lineNumbersOnPage(pg, k), made = lineNumbersOnPage(fresh, k);
        QVERIFY2(kept == made, qPrintable(QStringLiteral("page %1: %2 after the edit, %3 in a new paginator").arg(k).arg(kept.value(0)).arg(made.value(0))));
    }
    QCOMPARE(pg.pageCount(), fresh.pageCount());
}


// The work of numbering the lines is a page's lines and no more: a long document's pages don't count the lines of all the pages before (the count goes on from the page before's).
void FlowTests::lineNumberingWorkIsLinearInTheLinesOfTheDocument()
{
    FlowDocument fd;
    numberedStory(fd, 600, 1, Section::LineNumbers::Restart::Continuous);
    Paginator pg(&fd);
    const int pages = pg.pageCount();
    QVERIFY(pages >= 60);
    qint64 lines = 0;
    for (int k = 0; k < pages; ++k) {
        QVERIFY(pg.materialize(k));
        lines += int(pg.lines(k).size());
    }
    const qint64 visits = pg.lineNumberVisitsForTest();
    QVERIFY2(visits >= lines && visits <= 8 * lines, qPrintable(QStringLiteral("%1 lines looked at to number %2 lines on %3 pages").arg(visits).arg(lines).arg(pages)));
    const QStringList last = lineNumbersOnPage(pg, pages - 1);
    QVERIFY(!last.isEmpty());
    QCOMPARE(last.last(), QString::number(lines));
}


// An edit that only moves the numbers (a line added inside the first page's text, where the pages after it begin where they did) lays out no page but the one it is in: the
// numbers of the pages after it are worked out when they are drawn, from the lines each page holds, and the number on page 150 is moved by the added lines.
void FlowTests::anEditThatOnlyMovesLineNumbersLaysOutNoLaterPage()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    QTextCharFormat face;
    face.setFontFamilies({QStringLiteral("Arimo")});
    face.setFontPointSize(11);
    {
        QTextCursor c(fd.story());
        for (int i = 0; i < 200; ++i) {
            if (i) c.insertBlock();
            c.insertText(QStringLiteral("The court held that section applies. Reasonable notice was given.") + QChar(kPageBreak), face);
        }
    }
    fd.sections[0].paper = QSizeF(240, 200);
    fd.sections[0].margins = QMarginsF(60, 30, 30, 30);
    fd.sections[0].lineNumbers.countBy = 1;
    fd.sections[0].lineNumbers.restart = Section::LineNumbers::Restart::Continuous;
    fd.sections[0].lineNumbers.format.setFontPointSize(7);
    Paginator pg(&fd);
    QCOMPARE(pg.pageCount(), 200);
    const QStringList before = lineNumbersOnPage(pg, 150);
    const int linesBefore = int(pg.lines(0).size());
    QVERIFY(!before.isEmpty());
    const qint64 laidOutBefore = pg.pagesLaidOut();
    QTextCursor c(fd.story());
    c.setPosition(0);
    c.insertText(QStringLiteral("Words that make the first paragraph wrap onto more lines, set before the rest. "), face);
    QCOMPARE(pg.pageCount(), 200);
    const qint64 laidOut = pg.pagesLaidOut() - laidOutBefore;
    const int added = int(pg.lines(0).size()) - linesBefore;
    QVERIFY2(added >= 1, qPrintable(QString::number(added)));
    QVERIFY2(laidOut <= 3, qPrintable(QStringLiteral("%1 pages laid out again").arg(laidOut)));
    const QStringList after = lineNumbersOnPage(pg, 150);
    QVERIFY(!after.isEmpty());
    QCOMPARE(after.first().toInt(), before.first().toInt() + added);
    Paginator fresh(&fd);
    QCOMPARE(lineNumbersOnPage(fresh, 150), after);
}
