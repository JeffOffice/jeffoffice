#include "tst_flow.h"


// Sections that count their lines in different ways, one after the other: each line has the number the rules give it from the line counted before it, whatever pages the
// sections are on, and a section that goes on does not take its own start.
void FlowTests::sectionsOfDifferentLineNumberingGoOnFromOneAnother()
{
    using Restart = Section::LineNumbers::Restart;
    FlowDocument fd;
    numberedStory(fd, 30, 1, Restart::NewPage);
    const int breaks[2] = {8, 17};   // (the paragraphs that end the first and the second section)
    for (int endsSection : breaks) {
        QTextCursor c(fd.story()->findBlockByNumber(endsSection));
        QTextBlockFormat bf = c.blockFormat();
        bf.setProperty(fp::SectionBreakAfter, true);
        c.setBlockFormat(bf);
    }
    const int secondStart = fd.story()->findBlockByNumber(breaks[0] + 1).position(), thirdStart = fd.story()->findBlockByNumber(breaks[1] + 1).position();
    Section second = fd.sections[0], third = fd.sections[0];
    second.continuous = true;
    second.lineNumbers.restart = Restart::Continuous;
    second.lineNumbers.start = 7;
    third.continuous = true;
    third.lineNumbers.restart = Restart::NewSection;
    third.lineNumbers.start = 3;
    fd.sections << second << third;
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 4);
    int last = -1, lastSection = -1, lastPage = -1, again = 0;
    for (int k = 0; k < pg.pageCount(); ++k) {
        const QStringList shown = lineNumbersOnPage(pg, k);
        const auto briefs = pg.lines(k);
        QVERIFY2(shown.size() == briefs.size(), qPrintable(QStringLiteral("page %1 has %2 numbers for %3 lines").arg(k).arg(shown.size()).arg(briefs.size())));
        for (int i = 0; i < briefs.size(); ++i) {
            const int section = briefs[i].pos >= thirdStart ? 2 : briefs[i].pos >= secondStart ? 1 : 0;
            const Section::LineNumbers &numbers = fd.sections[section].lineNumbers;
            const bool begins = last < 0 || (numbers.restart == Restart::NewPage && lastPage != k) || (numbers.restart == Restart::NewSection && section != lastSection);
            last = begins ? numbers.start : last + 1;
            again += begins;
            lastSection = section;
            lastPage = k;
            QVERIFY2(shown[i] == QString::number(last), qPrintable(QStringLiteral("page %1, line %2: %3, not %4").arg(k).arg(i).arg(shown[i]).arg(last)));
        }
    }
    QVERIFY(again > 3 && lastSection == 2);
}


// A continuous section of two columns that holds a table: it begins below the text of the section before, the table is inside a column and flows into the next, and
// the page's note (cited in the text above the table, in the first column) and furniture keep to their places.
void FlowTests::aTableInAContinuousTwoColumnSectionIsInsideItsColumnBelowTheTextAbove()
{
        TestNotes notes;
        addNote(&notes, QStringLiteral("n"), 20, QStringLiteral("NOTECOL"));
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.fields = &notes;
        fillStory(fd.story(), 6, 7, [](int i, QTextBlockFormat &bf) {
            bf.setBottomMargin(6);
            if (i == 2) bf.setProperty(fp::SectionBreakAfter, true);
        }, 8, 12);
        TestTable spec;
        spec.rows = 90;
        spec.cols = 2;
        spec.columnWidth = 100;
        const TestTable table = addTable(fd, spec);
        appendParagraph(fd, QStringLiteral("After the table, in the second section."));
        addReference(fd.story(), fd.story()->findBlockByNumber(4).position() + 5, QStringLiteral("n"));   // (in the second section, in the text before the table)
        Section second = fd.sections[0];
        second.continuous = true;
        second.columns = 2;
        second.columnGap = 20;
        fd.sections << second;
        fd.sections[0].header.odd = furnitureStory({QStringLiteral("HEADER")});
        fd.sections[0].footer.odd = furnitureStory({QStringLiteral("FOOTER A"), QStringLiteral("FOOTER B")});
        fd.sections[1].header.odd = fd.sections[0].header.odd;
        fd.sections[1].footer.odd = fd.sections[0].footer.odd;
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 2);
        const Section &sec = fd.section(1);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        // The first section's lines end where the second begins (the lines of its frame), and the table's first rows are in its first column, below them.
        const auto lines = pg.lines(0);
        double split = 0;
        for (const auto &l : lines)
            if (l.frame == 0) split = std::max(split, l.rect.bottom());
        QVERIFY(split > 0);
        QVERIFY(!page->tables.isEmpty());
        const double top = pg.page(0).content.top();
        int tablesChecked = 0;
        for (int k = 0; k < pages; ++k) {
            const auto m = pg.materialize(k);
            QVERIFY(m);
            for (const auto &t : m->tables) {
                const QRectF r = t->rect();
                bool inAColumn = false;
                for (int col = 0; col < sec.columnCount(); ++col) {
                    const QRectF c = sec.columnRect(col);
                    inAColumn |= r.left() >= c.left() - 0.1 && r.right() <= c.right() + 0.1;
                }
                QVERIFY2(inAColumn, qPrintable(QStringLiteral("page %1: the table at %2 is not inside a column of the section").arg(k).arg(audit::rectText(r))));
                if (k == 0) QVERIFY2(r.top() >= top + split - 0.1, qPrintable(QStringLiteral("page 0: the table begins at %1, the section before it ends at %2").arg(r.top()).arg(top + split)));
                ++tablesChecked;
            }
        }
        QVERIFY(tablesChecked >= 2);
        // The table goes from the first column to the second, which begins where the section's columns do (below the text of the section before).
        const auto &fragments = page->tables;
        QVERIFY2(fragments.size() == 2, qPrintable(QString::number(fragments.size())));
        QVERIFY(fragments[1]->rect().left() > fragments[0]->rect().right() - 0.1);
        QVERIFY2(std::abs(fragments[1]->rect().top() - (top + split)) < 0.1, qPrintable(QStringLiteral("the table's first part is at %1, its second at %2, the columns begin at %3")
                                                                                         .arg(audit::rectText(fragments[0]->rect()), audit::rectText(fragments[1]->rect())).arg(top + split)));
        QVector<int> seen, wanted;
        for (const RowRange &r : rowRangesOf(fd)) wanted << r.from;   // (the references put into cells moved the rows on)
        for (int k = 0; k < pages; ++k)
            for (const auto &r : rowLines(pg, fd, k)) seen << r.pos;
        QVERIFY2(seen == wanted, qPrintable(QStringLiteral("the rows on the pages begin at [%1], the table's rows at [%2]").arg(intsText(seen), intsText(wanted))));
        const QString bad = audit::problems(pg, fd.story(), {{"n", "NOTECOL"}});
        QVERIFY2(bad.isEmpty(), qPrintable(bad));
    }


// A note longer than the pages, cited from a cell of a table: it goes on over pages of its own, which are made and drawn, and the end of it is on the last of them.
void FlowTests::aNoteLongerThanThePageCitedFromACellHasPagesOfItsOwn()
{
    TestNotes notes;
    addNote(&notes, QStringLiteral("long"), 1500, QStringLiteral("LONGNOTE"));
    addNote(&notes, QStringLiteral("short"), 10, QStringLiteral("SHORTNOTE"));
    FlowDocument fd;
    fd.env.wordLines = true;
    fd.fields = &notes;
    fillStory(fd.story(), 2, 4, {}, 20, 30);
    TestTable spec;
    spec.rows = 8;
    spec.cols = 2;
    spec.columnWidth = 150;
    const TestTable table = addTable(fd, spec);
    for (int i = 0; i < 5; ++i) appendParagraph(fd, QStringLiteral("A paragraph after the table, number %1, with a few words in it.").arg(i));
    addReference(fd.story(), fd.story()->findBlockByNumber(fd.story()->blockCount() - 2).position() + 10, QStringLiteral("short"));
    addReference(fd.story(), table.rowStarts[3] + 2, QStringLiteral("long"));
    Paginator pg(&fd);
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
    const QString lost = audit::lostNotes(pg, audit::fullNoteHeight(notes.docs["long"].get(), pg.page(0).content.width(), fd.env) + audit::fullNoteHeight(notes.docs["short"].get(), pg.page(0).content.width(), fd.env));
    QVERIFY2(lost.isEmpty(), qPrintable(lost));
    QVERIFY2(drawn.contains(QStringLiteral("SHORTNOTE")), "the short note is drawn");
    QVector<int> seen, wanted;
    for (const RowRange &r : rowRangesOf(fd)) wanted << r.from;   // (the references put into cells moved the rows on)
    for (int k = 0; k < pages; ++k)
        for (const auto &r : rowLines(pg, fd, k)) seen << r.pos;
    QCOMPARE(seen, wanted);
    const QString bad = audit::problems(pg, fd.story(), {{"long", "LONGNOTE"}, {"short", "SHORTNOTE"}});
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
}


// Under a word processor's line heights (several kinds of spacing and line height before the table), the stand-in for the table in the page's text, a paragraph of a
// fixed height, does not move the table or the text after it: the table begins where the text before it ends (and its space after), and the page's text has every line
// where the paginator put it.
void FlowTests::aTableIsNotShiftedByItsPlaceholderUnderProcessorLineHeights()
{
    struct Case { const char *name; QTextBlockFormat::LineHeightTypes type; double height, after, before; bool bordered = false; };
    const Case cases[] = {{"single, 8 after", QTextBlockFormat::SingleHeight, 0, 8, 0},
                          {"115 percent, 10 after", QTextBlockFormat::ProportionalHeight, 115, 10, 0},
                          {"exactly 20, none after", QTextBlockFormat::FixedHeight, 20, 0, 0},
                          {"150 percent, 4 after, 12 before", QTextBlockFormat::ProportionalHeight, 150, 4, 12},
                          {"at least 30, 6 after", QTextBlockFormat::MinimumHeight, 30, 6, 0},
                          {"a border below, 6 after", QTextBlockFormat::SingleHeight, 0, 6, 0, true}};
    tp::ParaBox box;
    box.top = {QStringLiteral("single"), 4, 2, QColor()};
    box.bottom = {QStringLiteral("single"), 8, 3, QColor()};
    for (const bool withTable : {false, true})
        for (const Case &cs : cases) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), 3, 9, [&](int i, QTextBlockFormat &bf) {
                bf.setBottomMargin(i == 2 ? cs.after : 6);
                if (i == 2) {
                    bf.setTopMargin(cs.before);
                    if (cs.type != QTextBlockFormat::SingleHeight) bf.setLineHeight(cs.height, cs.type);
                    if (cs.bordered) bf.setProperty(tp::Borders, box.spec());
                }
            }, 8, 12);
            TestTable spec;
            spec.rows = 3;
            spec.cols = 2;
            spec.columnWidth = 150;
            if (withTable) addTable(fd, spec);
            appendParagraph(fd, QStringLiteral("After the table."), [](QTextBlockFormat &bf) { bf.setLineHeight(130, QTextBlockFormat::ProportionalHeight); });
            appendParagraph(fd, QStringLiteral("And one more paragraph, the last."));
            Paginator pg(&fd);
            const QString what = QStringLiteral("%1, %2").arg(QLatin1String(cs.name), withTable ? "with the table" : "without it");
            QCOMPARE(pg.pageCount(), 1);
            const auto lines = pg.lines(0);
            if (withTable) {
                const auto rows = rowLines(pg, fd, 0);
                QCOMPARE(rows.size(), 3);
                int firstRow = 0;
                while (lines[firstRow].pos != rows[0].pos) ++firstRow;
                // The text before the table ends, with the space after its last paragraph, where the table begins.
                // (A paragraph with a border below has the room of the border's lower edge under its last line, before the space after it.)
                const double after = formatAt(fd, lines[firstRow - 1].pos).bottomMargin() + (cs.bordered ? box.bottom.reserve() : 0.0);
                QVERIFY2(std::abs(lines[firstRow - 1].rect.bottom() + std::max(0.0, after) - rows[0].rect.top()) < 0.05, qPrintable(what + ": the table's top and the end of the text before it"));
                const auto page = pg.materialize(0);
                QVERIFY(page && page->tables.size() == 1);
                QVERIFY2(std::abs(page->tables[0]->rect().top() - pg.page(0).content.top() - rows[0].rect.top()) < 0.05, qPrintable(what + ": the table is drawn where its rows are"));
                QVERIFY2(std::abs(page->tables[0]->height - (rows.last().rect.bottom() - rows[0].rect.top())) < 0.05, qPrintable(what + ": the table is as tall as its rows"));
            }
            const QString bad = audit::placementProblems(pg, fd, 0);
            QVERIFY2(bad.isEmpty(), qPrintable(what + QStringLiteral(": ") + bad));
            if (cs.before > 0 && !withTable) {
                // (The check is able to tell: the lines laid out as the page layer would with other settings are not where the paginator put them.)
                LayoutEnv other = fd.env;
                other.wordLines = false;
                QVERIFY2(!audit::placementProblems(pg, fd, 0, &other).isEmpty(), qPrintable(what + ": the check sees no difference"));
            }
        }
}


// A footer with the page's number and the document's page count, on every page of a document whose table spans pages, shows them right, and the pages made after an
// edit that adds rows show the new count.
void FlowTests::pageFieldsInAFooterKeepRightForATableThatSpansPages()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 2, 6, {}, 10, 14);
    TestTable spec;
    spec.rows = 140;
    spec.cols = 2;
    spec.columnWidth = 120;
    spec.setRow = [](int r, RowData &row) { row.header = r == 0; };
    addTable(fd, spec);
    appendParagraph(fd, QStringLiteral("After the table."));
    fd.sections[0].footer.odd = footerWithPageFields({QStringLiteral("A second line of the footer")});
    Paginator pg(&fd);
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
            if (ctx.resolve(QStringLiteral("pages")) != QString::number(pages)) return QStringLiteral("%1: page %2 shows %3 pages, the document has %4").arg(when).arg(k).arg(ctx.resolve(QStringLiteral("pages"))).arg(pages);
            if (!page->showsTotal || page->pageTotal != pages) return QStringLiteral("%1: page %2 is made with the total %3 (it shows it: %4), the document has %5").arg(when).arg(k).arg(page->pageTotal).arg(page->showsTotal).arg(pages);
        }
        return audit::problems(pg, fd.story());
    };
    const int before = pg.pageCount();
    QVERIFY(before >= 4);
    QString bad = checkPages(QStringLiteral("as made"));
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    // Rows put in near the top of the table: it has more pages, and every page that was made before shows the count it had.
    const auto old = pg.materialize(before - 1);
    QVERIFY(old);
    for (int i = 0; i < 25; ++i) QVERIFY(copyRowOf(fd, rowRangesOf(fd)[3]));
    QVERIFY(pg.pageCount() > before);
    bad = checkPages(QStringLiteral("after the rows"));
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    Paginator fresh(&fd);
    bad = audit::differenceFromFresh(pg, fresh);
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
}


// A row that goes over page breaks, under headers and footers that take room from the text: every paragraph of its cells is found on the page that shows the part of the
// row it is in.
void FlowTests::positionsInARowThatGoesOverPageBreaksAreOnThePagesThatShowThem()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 2, 5, {}, 10, 20);
    TestTable spec;
    spec.rows = 4;
    spec.cols = 2;
    spec.columnWidth = 200;
    spec.text = [](int r, int c) {
        QStringList lines;
        for (int i = 0; i < (r == 1 && c == 1 ? 140 : 3); ++i) lines << QStringLiteral("line %1 of cell %2 and %3").arg(i).arg(r).arg(c);
        return lines.join(QLatin1Char('\n'));
    };
    addTable(fd, spec);
    appendParagraph(fd, QStringLiteral("After the table."));
    fd.sections[0].header.odd = furnitureStory({QStringLiteral("HEAD 1"), QStringLiteral("HEAD 2"), QStringLiteral("HEAD 3"), QStringLiteral("HEAD 4"), QStringLiteral("HEAD 5")});
    fd.sections[0].footer.odd = furnitureStory({QStringLiteral("FOOT 1"), QStringLiteral("FOOT 2"), QStringLiteral("FOOT 3"), QStringLiteral("FOOT 4")});
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 3);
    QVERIFY2(fd.section(0).content().height() - pg.page(1).content.height() > 20, "the headers and the footer take room from the text");
    const QString bad = audit::splitRowProblems(pg, fd.story());
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    const QString rest = audit::problems(pg, fd.story());
    QVERIFY2(rest.isEmpty(), qPrintable(rest));
}


// Margins that leave a page no text area (and a header and footer beside them) give it the least text area every page has, not a negative one.
void FlowTests::marginsLargerThanThePageLeaveTheLeastTextArea()
{
        FlowDocument fd;
        fillStory(fd.story(), 30, 3);
        Section &s = fd.sections[0];
        s.margins = QMarginsF(72, 5000, 72, 5000);
        s.header.odd = furnitureStory({QStringLiteral("HEADER")});
        s.footer.odd = furnitureStory({QStringLiteral("FOOTER")});
        Paginator pg(&fd);
        const auto page = pg.page(0);
        const QRectF plain = s.content();
        QVERIFY2(page.content.height() >= 35.9, qPrintable(QString::number(page.content.height())));
        QVERIFY2(std::abs(page.content.height() - plain.height()) < 0.01, qPrintable(QStringLiteral("%1 vs %2").arg(page.content.height()).arg(plain.height())));
        QVERIFY(pg.pageCount() >= 1 && pg.pageCount() < 100000);
    }


// A header or footer taller than the margin leaves room for moves the text away from it, so the page holds fewer lines.
void FlowTests::aTallHeaderAndFooterTakeRoomFromTheText()
{
    auto lines = [](bool tall) {
        FlowDocument fd;
        fillStory(fd.story(), 80, 9);
        Section &s = fd.sections[0];
        s.headerDistance = 36;
        s.footerDistance = 36;
        if (tall) {
            s.header.odd = furnitureStory({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c"), QStringLiteral("d"), QStringLiteral("e"), QStringLiteral("f"), QStringLiteral("g")});
            s.footer.odd = furnitureStory({QStringLiteral("x"), QStringLiteral("y"), QStringLiteral("z"), QStringLiteral("w"), QStringLiteral("v"), QStringLiteral("u")});
        }
        Paginator pg(&fd);
        const auto first = pg.lines(0);
        return std::make_tuple(int(first.size()), first.last().rect.bottom(), pg.page(0).content, pg.pageCount());
    };
    const auto bare = lines(false), tall = lines(true);
    QVERIFY(std::get<0>(tall) < std::get<0>(bare));
    QVERIFY(std::get<3>(tall) > std::get<3>(bare));
    // The text area starts below the header and ends above the footer, each at its distance from the edge plus its own height.
    FlowDocument probe;
    const double width = probe.sections[0].content().width();
    const double headerH = storyHeightOf(furnitureStory({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c"), QStringLiteral("d"), QStringLiteral("e"), QStringLiteral("f"), QStringLiteral("g")}).get(), width, probe);
    const double footerH = storyHeightOf(furnitureStory({QStringLiteral("x"), QStringLiteral("y"), QStringLiteral("z"), QStringLiteral("w"), QStringLiteral("v"), QStringLiteral("u")}).get(), width, probe);
    const QRectF body = std::get<2>(tall);
    QVERIFY(std::abs(body.top() - (36 + headerH)) < 0.02);
    QVERIFY(std::abs(body.bottom() - (792 - 36 - footerH)) < 0.02);
    QVERIFY(headerH + 36 > 72 && footerH + 36 > 72);
    QVERIFY(std::get<1>(tall) <= body.height() + 0.01);
}


// The first page of a section, the even pages, and the rest each have their own story, as the section asks.
void FlowTests::firstAndEvenPagesUseTheirOwnHeaders()
{
    FlowDocument fd;
    fillStory(fd.story(), 120, 6, [](int i, QTextBlockFormat &bf) {
        bf.setBottomMargin(10);
        if (i == 59) bf.setProperty(flow::fp::SectionBreakAfter, true);
    });
    Section &s = fd.sections[0];
    s.differentFirstPage = true;
    s.differentOddEven = true;
    s.header.first = furnitureStory({QStringLiteral("first")});
    s.header.odd = furnitureStory({QStringLiteral("odd")});
    s.header.even = furnitureStory({QStringLiteral("even")});
    Section second = s;
    second.header.first = furnitureStory({QStringLiteral("second first")});
    second.differentOddEven = false;
    fd.sections << second;
    Paginator pg(&fd);
    const int pages = pg.pageCount();
    QVERIFY(pages >= 6);
    QStringList seen;
    int firstOfSecond = -1;
    for (int k = 0; k < pages; ++k) {
        const auto page = pg.materialize(k);
        QString header;
        const auto boxes = boxesByText(*page);
        for (auto it = boxes.cbegin(); it != boxes.cend(); ++it)
            if (std::abs(it.value()->rect.top() - 36) < 0.01) header = it.key();
        seen << header;
        if (page->section == 1 && firstOfSecond < 0) firstOfSecond = k;
    }
    QVERIFY(firstOfSecond >= 4 && pages >= firstOfSecond + 3);
    QCOMPARE(seen[0], QStringLiteral("first"));
    QCOMPARE(seen[1], QStringLiteral("even"));
    QCOMPARE(seen[2], QStringLiteral("odd"));
    QCOMPARE(seen[3], QStringLiteral("even"));
    QCOMPARE(seen[firstOfSecond], QStringLiteral("second first"));
    QCOMPARE(seen[firstOfSecond + 1], QStringLiteral("odd"));   // (the second section asks for no even pages' headers: its ordinary one)
    QCOMPARE(seen[firstOfSecond + 2], QStringLiteral("odd"));
}


// NUMPAGES and SECTIONPAGES show how many pages the document and the page's section have, which the page's fields read.
void FlowTests::pageFieldsKnowTheTotals()
{
    FlowDocument fd;
    fillStory(fd.story(), 70, 6, [](int i, QTextBlockFormat &bf) {
        bf.setBottomMargin(10);
        if (i == 39) bf.setProperty(flow::fp::SectionBreakAfter, true);
    });
    fd.sections << fd.sections[0];
    Paginator pg(&fd);
    const int pages = pg.pageCount();
    int inFirst = 0;
    for (int k = 0; k < pages; ++k) inFirst += pg.page(k).start.section == 0;
    QVERIFY(inFirst > 1 && inFirst < pages - 1);
    for (int k : {0, pages - 1}) {
        const auto page = pg.materialize(k);
        FieldContext ctx;
        ctx.doc = page->doc.get();
        QCOMPARE(ctx.resolve(QStringLiteral("pages")), QString::number(pages));
        QCOMPARE(ctx.resolve(QStringLiteral("sectionpages")), QString::number(k == 0 ? inFirst : pages - inFirst));
    }
    // (Without a flowing document's counts, a context's own page count is shown.)
    FieldContext own;
    own.pageCount = 7;
    QCOMPARE(own.resolve(QStringLiteral("pages")), QStringLiteral("7"));
    QCOMPARE(own.resolve(QStringLiteral("sectionpages")), QStringLiteral("7"));
}


// A note longer than the pages the body has goes on over pages of its own, which have no text of the body: every page is made, and the end of the note is
// drawn on the last of them.
void FlowTests::aNoteLongerThanTheBodyHasPagesOfItsOwn()
{
    TestNotes notes;
    addNote(&notes, QStringLiteral("long"), 2000, QStringLiteral("LONGNOTE"));
    FlowDocument fd;
    fd.env.wordLines = true;
    fd.fields = &notes;
    fillStory(fd.story(), 20, 4, {}, 30, 40);
    addReference(fd.story(), fd.story()->findBlockByNumber(1).position() + 5, QStringLiteral("long"));
    Paginator pg(&fd);
    const int pages = pg.pageCount();
    int noteOnly = 0;
    QString drawn;
    for (int k = 0; k < pages; ++k) {
        const auto page = pg.materialize(k);
        QVERIFY2(page, qPrintable(QStringLiteral("page %1 of %2 isn't made").arg(k).arg(pages)));
        if (pg.lines(k).isEmpty()) {
            ++noteOnly;
            QVERIFY2(!page->doc->pages[0]->items.empty(), "a page of notes alone has them drawn");
        }
        for (const auto &item : page->doc->pages[0]->items)
            if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
                if (const Story *st = page->doc->story(t->storyId)) drawn += st->doc->toPlainText() + QLatin1Char('\n');
    }
    QVERIFY2(noteOnly > 0, "the note is longer than the body's pages");
    QVERIFY(drawn.contains(QStringLiteral("word1999")));
}


// References packed in one paragraph cost in step with their number, not its square: the page lays out only what it holds of the paragraph's notes.
void FlowTests::denseReferencesTakeTimeInStepWithTheirNumber()
{
    auto work = [&](int count) {
        return workToPaginate([count](FlowDocument &fd, TestNotes &notes) {
            QTextCursor c(fd.story());
            QTextCharFormat cf;
            cf.setFontPointSize(11);
            for (int i = 0; i < count; ++i) {
                const QString id = QStringLiteral("n%1").arg(i);
                addNote(&notes, id, 3, QStringLiteral("N%1").arg(i));
                QTextCharFormat ref = cf;
                ref.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
                ref.setProperty(tp::Field, QStringLiteral("footnote:") + id);
                c.insertText(QString(QChar(0xFFFC)), ref);
            }
        }, 12);
    };
    const PaginateWork small = work(400), large = work(1600);
    // (Four times the references: four times the work when it is in step, sixteen when it is the square; it was twenty before.)
    verifyInStep(small, large, 8, true);
}


// Note numbers that restart in each section restart where a continuous section begins in the middle of a page: on the page, in its references and in
// its notes, and in the numbers the pages after it go on from.
void FlowTests::noteNumbersRestartAtAContinuousSectionBreak()
{
    auto numbersOf = [](Paginator &pg, int k, QStringList *references, QStringList *notes) {
        const auto page = pg.materialize(k);
        QVERIFY(page);
        QStringList ids = page->doc->stories.keys();
        ids.sort();
        for (const QString &id : ids)
            for (QTextBlock b = page->doc->story(id)->doc->begin(); b.isValid(); b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it) {
                    const QString code = it.fragment().charFormat().stringProperty(tp::Field);
                    if (code.startsWith(QLatin1String("raw:"))) (id.startsWith(QLatin1String("flow-note-")) ? notes : references)->push_back(code.mid(4));
                }
    };
    for (bool restart : {false, true}) {
        TestNotes notes;
        for (const char *id : {"a", "b", "c", "d"}) addNote(&notes, QString::fromLatin1(id), 3, QString::fromLatin1(id).toUpper());
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.fields = &notes;
        fd.restartNotesInSections = restart;
        fillStory(fd.story(), 4, 3, [](int i, QTextBlockFormat &bf) {
            bf.setBottomMargin(6);
            if (i == 1) bf.setProperty(flow::fp::SectionBreakAfter, true);
        }, 8, 12);
        Section second = fd.sections[0];
        second.continuous = true;
        fd.sections << second;
        const char *ids[] = {"a", "b", "c", "d"};
        for (int i = 3; i >= 0; --i) addReference(fd.story(), fd.story()->findBlockByNumber(i).position() + 3, QString::fromLatin1(ids[i]));
        Paginator pg(&fd);
        QCOMPARE(pg.pageCount(), 1);
        QStringList references, shown;
        numbersOf(pg, 0, &references, &shown);
        const QStringList want = restart ? QStringList{"1", "2", "1", "2"} : QStringList{"1", "2", "3", "4"};
        QCOMPARE(references, want);
        QCOMPARE(shown, want);
    }
}


// A footnote is at the foot of the page that cites it, in a text box of its own at the bottom of the text area with a rule above it, and its
// number is where the reference is counted: the references go on through the pages, and the page's text has less room for the notes.
void FlowTests::footnotesAreAtTheFootOfThePageThatCitesThem()
{
    TestNotes notes;
    addNote(&notes, QStringLiteral("a"), 12, QStringLiteral("NOTEA"));
    addNote(&notes, QStringLiteral("b"), 12, QStringLiteral("NOTEB"));
    auto build = [&](bool withNotes) {
        auto fd = std::make_unique<FlowDocument>();
        fd->env.wordLines = true;
        fillStory(fd->story(), 70, 12);
        if (withNotes) {
            fd->fields = &notes;
            addReference(fd->story(), fd->story()->findBlockByNumber(2).position() + 5, QStringLiteral("a"));
            addReference(fd->story(), fd->story()->findBlockByNumber(30).position() + 5, QStringLiteral("b"));
        }
        return fd;
    };
    auto with = build(true), without = build(false);
    Paginator pg(with.get()), plain(without.get());
    QVERIFY(pg.pageCount() >= 3);
    // The first page: the note, its rule, and fewer lines than the page without notes holds.
    const auto first = pg.materialize(0);
    QString noteStory;
    const TextItem *note = boxHolding(*first, QStringLiteral("NOTEA"), &noteStory);
    QVERIFY(note);
    QVERIFY(!boxHolding(*first, QStringLiteral("NOTEB")));
    const double bottom = with->sections[0].content().bottom();
    QVERIFY2(std::abs(note->rect.bottom() - bottom) < 0.05, qPrintable(QString::number(note->rect.bottom())));
    QVERIFY(noteStory.contains(QStringLiteral("raw")) == false);   // (the mark is its number)
    QVERIFY(pg.lines(0).size() < plain.lines(0).size());
    const LineItem *rule = nullptr;
    for (const auto &item : first->doc->pages[0]->items)
        if (const auto *l = dynamic_cast<const LineItem *>(item.get())) rule = l;
    QVERIFY(rule);
    QVERIFY(std::abs((rule->p2.x() - rule->p1.x()) - 144) < 0.01);
    QVERIFY(rule->p1.y() < note->rect.top() && rule->p1.y() > note->rect.top() - 20);
    // The reference in the text shows its number; the number goes on through the pages (the second note is 2 wherever it is).
    const TextItem *body = boxHolding(*first, QStringLiteral("court"));
    QVERIFY(body);
    const Story *bodyStory = first->doc->story(body->storyId);
    bool numbered = false;
    for (QTextBlock b = bodyStory->doc->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) numbered |= it.fragment().charFormat().stringProperty(tp::Field) == QLatin1String("raw:1");
    QVERIFY(numbered);
    int notePage = -1;
    for (int k = 1; k < pg.pageCount() && notePage < 0; ++k)
        if (boxHolding(*pg.materialize(k), QStringLiteral("NOTEB"))) notePage = k;
    QVERIFY(notePage > 0);
    const auto later = pg.materialize(notePage);
    const TextItem *bNote = boxHolding(*later, QStringLiteral("NOTEB"));
    bool two = false;
    for (QTextBlock b = later->doc->story(bNote->storyId)->doc->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) two |= it.fragment().charFormat().stringProperty(tp::Field) == QLatin1String("raw:2");
    QVERIFY(two);
    QCOMPARE(pg.page(notePage).start.foot, 1);   // (one note came before the page's first paragraph)
    QVERIFY(std::abs(bNote->rect.bottom() - bottom) < 0.05);
    // A page that cites none has no note and no rule.
    const auto bare = pg.materialize(1 == notePage ? 2 : 1);
    QVERIFY(!boxHolding(*bare, QStringLiteral("NOTE")));
    for (const auto &item : bare->doc->pages[0]->items) QVERIFY(!dynamic_cast<const LineItem *>(item.get()));
}


// A footnote too long for the room at the foot of the page is cut there, and the rest of it begins the notes of the next page.
void FlowTests::aLongFootnoteGoesOnOnTheNextPage()
{
    TestNotes notes;
    addNote(&notes, QStringLiteral("long"), 900, QStringLiteral("LONGNOTE"));
    FlowDocument fd;
    fd.env.wordLines = true;
    fd.fields = &notes;
    fillStory(fd.story(), 40, 14);
    addReference(fd.story(), fd.story()->findBlockByNumber(1).position() + 5, QStringLiteral("long"));
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 3);
    QVERIFY(!pg.page(1).start.carried.isEmpty());
    const auto first = pg.materialize(0), second = pg.materialize(1);
    QVERIFY(boxHolding(*first, QStringLiteral("LONGNOTE")));
    QString rest;
    const TextItem *tail = boxHolding(*second, QStringLiteral("word"), &rest);
    QVERIFY(tail);
    QVERIFY(!rest.contains(QStringLiteral("LONGNOTE")));   // (the second part begins in the middle of the note's text)
    // The note's text is all on the pages it is on: every word once.
    QSet<QString> seen;
    int count = 0;
    for (int k = 0; k < pg.pageCount(); ++k) {
        const auto page = pg.materialize(k);
        for (const auto &item : page->doc->pages[0]->items)
            if (const auto *t = dynamic_cast<const TextItem *>(item.get())) {
                const Story *st = page->doc->story(t->storyId);
                if (!st->doc->toPlainText().contains(QStringLiteral("word"))) continue;
                if (st->doc->toPlainText().contains(QStringLiteral("court"))) continue;   // (the text, not a note)
                LayoutCache cache;
                const auto fl = cache.textFrame(*page->doc, *t, 1, RenderOptions());
                if (!fl.layout) continue;
                for (const auto &li : fl.layout->lineInfo(fl.frame))
                    for (const QString &w : li.text.split(QLatin1Char(' '), Qt::SkipEmptyParts))
                        if (w.startsWith(QStringLiteral("word"))) { seen.insert(w); ++count; }
            }
    }
    QCOMPARE(int(seen.size()), 900);
    QCOMPARE(count, 900);
}


// An object set at a place goes where its anchor's page, paragraph, or line puts it: from the margin or the page, in the middle or at the end of one,
// behind the text or over it; a rule's ends are from the place too; a picture's data is in the page's document.
void FlowTests::objectsAreSetWherePageParagraphAndLinePutThem()
{
    FlowDocument fd;
    fillStory(fd.story(), 60, 12);
    const QTextDocument *story = fd.story();
    const int third = story->findBlockByNumber(3).position() + 4;
    auto box = [](double w, double h) {
        auto item = std::make_shared<ShapeItem>();
        item->rect = QRectF(0, 0, w, h);
        return item;
    };
    using Rel = FloatingObject::Rel;
    // From the margin across and the paragraph down.
    const int a = addObject(&fd, box(100, 20), third, Rel::Margin, Rel::Paragraph, 10, 5);
    // In the middle across the page, at the end of the margin down, behind the text.
    const int b = addObject(&fd, box(50, 30), third, Rel::Page, Rel::Margin);
    fd.objects[b - 1].hAlign = FloatingObject::Align::Center;
    fd.objects[b - 1].vAlign = FloatingObject::Align::End;
    fd.objects[b - 1].behind = true;
    // A rule from the column's edge, a line down.
    auto line = std::make_shared<LineItem>();
    line->p1 = QPointF(0, 0);
    line->p2 = QPointF(120, 0);
    const int c = addObject(&fd, line, third, Rel::Column, Rel::Line, 3, 2);
    // A picture from the page's corner.
    auto picture = std::make_shared<PictureItem>();
    picture->rect = QRectF(0, 0, 40, 20);
    fd.images.push_back({tinyPng(8, 4, Qt::red), QStringLiteral("png")});
    const int p = addObject(&fd, picture, third, Rel::Page, Rel::Page, 20, 30);
    fd.objects[p - 1].image = 0;
    Q_UNUSED(a); Q_UNUSED(c);
    Paginator pg(&fd);
    const auto page = pg.materialize(0);
    QVERIFY(page);
    const auto &items = page->doc->pages[0]->items;
    // The paragraph and the line that hold the anchor, as the page laid them out.
    const auto lines = pg.lines(0);
    double paragraphTop = -1, lineTop = -1;
    const int blockStart = story->findBlockByNumber(3).position();
    for (const auto &l : lines)
        if (l.block == blockStart) {
            if (paragraphTop < 0) paragraphTop = l.rect.top();
            if (third >= l.pos && third < l.end) lineTop = l.rect.top();
        }
    QVERIFY(paragraphTop > 0 && lineTop >= paragraphTop);
    const double top = fd.sections[0].margins.top(), left = fd.sections[0].margins.left();
    const ShapeItem *first = nullptr, *second = nullptr;
    const LineItem *rule = nullptr;
    const PictureItem *pic = nullptr;
    for (const auto &it : items) {
        if (it->type() == ItemType::Line) rule = static_cast<const LineItem *>(it.get());
        if (it->type() == ItemType::Picture) pic = static_cast<const PictureItem *>(it.get());
        if (it->type() == ItemType::Shape) {
            const auto *sh = static_cast<const ShapeItem *>(it.get());
            if (sh->rect.width() == 100) first = sh;
            else second = sh;
        }
    }
    QVERIFY(first && second && rule && pic);
    QVERIFY2(std::abs(first->rect.left() - (left + 10)) < 0.01 && std::abs(first->rect.top() - (top + paragraphTop + 5)) < 0.01, qPrintable(QStringLiteral("%1 %2 (paragraph at %3)").arg(first->rect.left()).arg(first->rect.top()).arg(paragraphTop)));
    QVERIFY(std::abs(second->rect.left() - (612 - 50) / 2.0) < 0.01);
    QVERIFY(std::abs(second->rect.bottom() - (792 - fd.sections[0].margins.bottom())) < 0.01);
    QVERIFY(items.front().get() == second);   // (behind the text: before it)
    QVERIFY(std::abs(rule->p1.x() - (left + 3)) < 0.01 && std::abs(rule->p2.x() - (left + 3 + 120)) < 0.01);
    QVERIFY(std::abs(rule->p1.y() - (top + lineTop + 2)) < 0.01 && std::abs(rule->p1.y() - rule->p2.y()) < 0.01);
    QVERIFY(std::abs(pic->rect.left() - 20) < 0.01 && std::abs(pic->rect.top() - 30) < 0.01);
    QVERIFY(!pic->imageId.isEmpty());
    QVERIFY(page->doc->images.contains(pic->imageId));
    // A page whose lines hold no anchor has none of them.
    const auto other = pg.materialize(pg.pageCount() - 1);
    for (const auto &it : other->doc->pages[0]->items) QVERIFY(it->type() == ItemType::Text);
}


// The objects set in a section's header are on every page of it, from the header's own lines; an inline picture in a story has the Id its picture
// has in the page's document.
void FlowTests::headerObjectsAndInlinePicturesAreOnTheirPages()
{
        FlowDocument fd;
        fillStory(fd.story(), 80, 12);
        auto header = furnitureStory({QStringLiteral("HEADER"), QStringLiteral("second line")});
        {
            // A rule anchored to the header's second paragraph, from the margin across and the paragraph down.
            auto line = std::make_shared<LineItem>();
            line->p1 = QPointF(0, 0);
            line->p2 = QPointF(200, 0);
            FloatingObject o;
            o.item = line;
            o.hRel = FloatingObject::Rel::Margin;
            o.vRel = FloatingObject::Rel::Paragraph;
            o.dy = 1;
            fd.objects.push_back(o);
            QTextCursor c(header.get());
            c.setPosition(header->findBlockByNumber(1).position());
            QTextCharFormat anchor;
            anchor.setProperty(fp::AnchorId, 1);
            c.insertText(QString(QChar(0xFFFC)), anchor);
        }
        fd.sections[0].header.odd = header;
        fd.sections[0].headerDistance = 30;
        // An inline picture in the body.
        fd.images.push_back({tinyPng(6, 6, Qt::blue), QStringLiteral("png")});
        {
            auto item = std::make_shared<PictureItem>();
            item->rect = QRectF(0, 0, 30, 20);
            item->imageId = QStringLiteral("flow-img:0");
            QTextCursor c(fd.story());
            c.setPosition(fd.story()->findBlockByNumber(1).position() + 3);
            QTextCharFormat inl;
            inl.setFontPointSize(11);
            inl.setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(item->toJson()).toJson(QJsonDocument::Compact)));
            c.insertText(QString(QChar(0xFFFC)), inl);
        }
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 2);
        for (int k : {0, 1}) {
            const auto page = pg.materialize(k);
            const LineItem *rule = nullptr;
            for (const auto &it : page->doc->pages[0]->items)
                if (it->type() == ItemType::Line) rule = static_cast<const LineItem *>(it.get());
            QVERIFY(rule);
            // The header's second line: below its first by a line, from the header's top.
            StoryLayout own;
            own.build(header.get(), {plainFrame(fd.sections[0].content().width(), 100000)}, fd.env);
            const double second = own.lineInfo(0).at(1).rect.top();
            QVERIFY2(std::abs(rule->p1.y() - (30 + second + 1)) < 0.01, qPrintable(QString::number(rule->p1.y())));
            QVERIFY(std::abs(rule->p1.x() - fd.sections[0].margins.left()) < 0.01);
        }
        const auto first = pg.materialize(0);
        const TextItem *body = boxHolding(*first, QStringLiteral("court"));
        QVERIFY(body);
        QString id;
        for (QTextBlock b = first->doc->story(body->storyId)->doc->begin(); b.isValid() && id.isEmpty(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QString json = it.fragment().charFormat().stringProperty(tp::InlineObject);
                if (!json.isEmpty()) id = QJsonDocument::fromJson(json.toUtf8()).object().value(QStringLiteral("image")).toString();
            }
        QVERIFY(!id.isEmpty() && !id.startsWith(QStringLiteral("flow-img:")));
        QVERIFY(first->doc->images.contains(id));
    }


// ---- frames

// A word processor's text does not lay out a paragraph that is set apart from it: it has no lines and no room, and the text around it is as if it were not there; the other
// texts (a publication's) take no notice of the property.
void FlowTests::aParagraphOutOfLayoutHasNoLinesAndNoRoom()
{
    auto lay = [](bool outOfLayout, bool word) {
        auto story = makeStory(5, 9, {}, 14, 25);
        if (outOfLayout) {
            QTextCursor c(story.get());
            c.setPosition(story->findBlockByNumber(2).position());
            QTextBlockFormat bf = c.blockFormat();
            bf.setProperty(tp::OutOfLayout, true);
            c.setBlockFormat(bf);
        }
        LayoutEnv env;
        env.wordLines = word;
        StoryLayout l;
        l.build(story.get(), {plainFrame(300, 600)}, env);
        QStringList out;
        const int second = story->findBlockByNumber(2).position();
        for (const auto &b : l.lineBriefs()) out << QStringLiteral("%1 %2%3").arg(b.block == second ? QStringLiteral("hidden") : QStringLiteral("body"), audit::rectText(b.rect), b.block == second ? QStringLiteral("!") : QString());
        return out;
    };
    const QStringList word = lay(true, true), plain = lay(false, true), other = lay(true, false), otherPlain = lay(false, false);
    for (const QString &line : word) QVERIFY2(!line.endsWith(QLatin1Char('!')), qPrintable(line));
    QCOMPARE(word.size() + int(std::count_if(plain.begin(), plain.end(), [](const QString &l) { return l.endsWith(QLatin1Char('!')); })), int(plain.size()));
    // The lines of the others are where they are without the paragraph in the text at all (the same story with the paragraph taken out).
    {
        auto story = makeStory(5, 9, {}, 14, 25);
        QTextCursor c(story.get());
        const QTextBlock b = story->findBlockByNumber(2);
        c.setPosition(b.position());
        c.setPosition(b.position() + b.length(), QTextCursor::KeepAnchor);
        c.removeSelectedText();
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout l;
        l.build(story.get(), {plainFrame(300, 600)}, env);
        QStringList without;
        for (const auto &x : l.lineBriefs()) without << QStringLiteral("body %1").arg(audit::rectText(x.rect));
        QCOMPARE(word, without);
    }
    // (In a publication's text it is a paragraph like any other.)
    QCOMPARE(other, otherPlain);
    // A picture in the paragraph (a frame with only a logo in it) is not laid out either: no line, no room, nothing drawn in the text.
    {
        auto story = makeStory(3, 9, {}, 14, 25);
        auto item = std::make_shared<PictureItem>();
        item->rect = QRectF(0, 0, 40, 30);
        item->imageId = QStringLiteral("flow-img:0");
        QTextCursor c(story.get());
        c.setPosition(story->findBlockByNumber(1).position());
        QTextBlockFormat bf = c.blockFormat();
        bf.setProperty(tp::OutOfLayout, true);
        c.setBlockFormat(bf);
        QTextCharFormat inl;
        inl.setFontPointSize(11);
        inl.setProperty(tp::InlineObject, QString::fromUtf8(QJsonDocument(item->toJson()).toJson(QJsonDocument::Compact)));
        c.insertText(QString(QChar(0xFFFC)), inl);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout l;
        l.build(story.get(), {plainFrame(300, 600)}, env);
        QVERIFY(l.inlineObjects().isEmpty());
        const int second = story->findBlockByNumber(1).position();
        for (const auto &b : l.lineBriefs()) QVERIFY(b.block != second);
    }
}


// A stretch of text that stops at a frame (a page's, laid out to see what goes past it) does not stop at a paragraph with no lines, which has put nothing in the frame: the
// text after it is laid out, and is what lands in the frame.
void FlowTests::aStretchIsNotEndedByAParagraphWithNoLines()
{
    auto story = makeStory(4, 9, {}, 14, 25);
    QTextCursor c(story.get());
    c.setPosition(0);
    QTextBlockFormat bf = c.blockFormat();
    bf.setProperty(tp::OutOfLayout, true);
    c.setBlockFormat(bf);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    StoryLayout::Span span;
    span.stopAtFrame = 0;   // (the only frame is the one that runs over)
    lay.build(story.get(), {plainFrame(300, 100000)}, env, span);
    const int second = story->findBlockByNumber(1).position();
    QVERIFY(!lay.lineBriefs().isEmpty());
    QCOMPARE(lay.lineBriefs().first().block, second);
    // (All of the paragraph that lands there is laid out, and the stretch ends after it.)
    int lastBlock = -1;
    for (const auto &l : lay.lineBriefs()) lastBlock = l.block;
    QCOMPARE(lastBlock, second);
}


// A frame is set where the paragraph after its paragraphs is: its paragraphs have no lines and no room, it is a text box the page draws at its place, with the text of its
// paragraphs and as wide as it says, and the text goes round it.
void FlowTests::aFrameIsSetAtTheParagraphAfterItAndTheTextGoesRoundIt()
{
    auto make = [](bool withFrame) {
        auto fd = std::make_unique<FlowDocument>();
        fd->env.wordLines = true;
        fillStory(fd->story(), 24, 41, {}, 25, 40);
        if (withFrame) {
            FloatingObject proto;
            proto.hRel = FloatingObject::Rel::Column;
            proto.vRel = FloatingObject::Rel::Paragraph;
            proto.hAlign = FloatingObject::Align::End;
            proto.dy = 6;
            proto.wrap = FloatingObject::Wrap::Square;
            proto.gap = QMarginsF(9, 0, 9, 0);
            addFrame(*fd, 6, {QStringLiteral("First paragraph of the frame"), QStringLiteral("and a second paragraph of it, which is longer, so that it takes two lines of the frame")}, proto, 130);
        }
        return fd;
    };
    auto plain = make(false), framed = make(true);
    Paginator pp(plain.get()), pg(framed.get());
    const QSet<int> frameBlocks = frameBlocksOf(*framed);
    QCOMPARE(int(frameBlocks.size()), 2);
    const int anchorBlock = framed->story()->findBlockByNumber(8).position();   // (the paragraph that was block 6)
    QVERIFY(!frameBlocks.contains(anchorBlock));
    for (const auto &l : pg.lines(0)) QVERIFY2(!frameBlocks.contains(l.block), "a line of a frame's paragraph");
    // The paragraphs above the frame are as they are without it, and so is the paragraph it is anchored to (its top).
    const auto a = pageLines(pg, *framed, 0), b = pageLines(pp, *plain, 0);
    int above = 0;
    for (const PageLine &l : a) {
        if (l.block >= anchorBlock) break;
        QVERIFY2(above < b.size() && std::abs(b[above].rect.top() - l.rect.top()) < 0.01 && std::abs(b[above].rect.width() - l.rect.width()) < 0.01, qPrintable(audit::rectText(l.rect)));
        ++above;
    }
    QVERIFY(above > 10);
    const double top = paragraphTopOn(pg, *framed, 0, anchorBlock);
    QVERIFY(top > 0 && std::abs(top - paragraphTopOn(pp, *plain, 0, plain->story()->findBlockByNumber(6).position())) < 0.01);
    const auto page = pg.materialize(0);
    QVERIFY(page);
    const TextItem *box = frameBoxWith(*page, QStringLiteral("First paragraph of the frame"));
    QVERIFY(box);
    const QRectF column = framed->sections[0].columnRect(0);
    QVERIFY2(std::abs(box->rect.width() - 130) < 0.01 && std::abs(box->rect.right() - column.right()) < 0.01 && std::abs(box->rect.top() - (top + 6)) < 0.02, qPrintable(audit::rectText(box->rect)));
    const Story *story = page->doc->story(box->storyId);
    QVERIFY(story && story->doc->toPlainText().contains(QStringLiteral("two lines of the frame")));
    // (Its paragraphs are laid out in it: not set out of the layout there.)
    for (QTextBlock x = story->doc->begin(); x.isValid(); x = x.next()) QVERIFY(!x.blockFormat().boolProperty(tp::OutOfLayout) && x.blockFormat().intProperty(fp::Frame) == 0);
    // As tall as its text is in a box 130 wide.
    StoryLayout own;
    own.build(story->doc.get(), {plainFrame(130, 100000)}, framed->env);
    QVERIFY2(std::abs(box->rect.height() - own.usedHeight(0)) < 0.02 && own.lineInfo(0).size() >= 4, qPrintable(QStringLiteral("%1 high; its text %2 in a box 130 wide").arg(box->rect.height()).arg(own.usedHeight(0))));
    // The text goes round it, as the page layer draws it; and the page is whole.
    const QRectF wall = box->rect.adjusted(-9, 0, 9, 0);
    int beside = 0;
    for (const PageLine &l : a)
        if (l.rect.top() < wall.bottom() - 0.05 && l.rect.bottom() > wall.top() + 0.05) {
            ++beside;
            QVERIFY2(l.rect.right() <= wall.left() + 0.05, qPrintable(audit::rectText(l.rect)));
        }
    QVERIFY(beside >= 4);
    QVERIFY2(audit::problems(pg, framed->story()).isEmpty(), qPrintable(audit::problems(pg, framed->story())));
    for (int k = 0; k < pg.pageCount(); ++k) QVERIFY2(audit::placementProblems(pg, *framed, k).isEmpty(), qPrintable(audit::placementProblems(pg, *framed, k)));
}


// How tall a frame is: as its text is when it asks nothing, at least what it asks, or exactly that (then the text is cut by it).
void FlowTests::aFramesHeightIsItsTextsOrWhatItAsks()
{
    auto heightOf = [](double asked, bool exact) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 10, 42, {}, 25, 40);
        FloatingObject proto;
        proto.hRel = FloatingObject::Rel::Column;
        proto.vRel = FloatingObject::Rel::Paragraph;
        proto.wrap = FloatingObject::Wrap::Square;
        addFrame(fd, 3, {QStringLiteral("One line of the frame"), QStringLiteral("a second paragraph of the frame, which is long enough to take a second line")}, proto, 140, asked, exact);
        Paginator pg(&fd);
        const auto page = pg.materialize(0);
        const TextItem *box = page ? frameBoxWith(*page, QStringLiteral("One line of the frame")) : nullptr;
        return box ? box->rect.height() : -1.0;
    };
    const double natural = heightOf(0, false);
    QVERIFY(natural > 30 && natural < 100);
    QVERIFY(std::abs(heightOf(natural / 2, false) - natural) < 0.01);    // (at least: a text taller than that is as tall as it is)
    QVERIFY(std::abs(heightOf(natural + 50, false) - (natural + 50)) < 0.01);
    QVERIFY(std::abs(heightOf(20, true) - 20) < 0.01);                   // (exactly)
    QVERIFY(std::abs(heightOf(natural + 50, true) - (natural + 50)) < 0.01);
}


// A frame with no width of its own is as wide as the column; one that shrinks (a drop cap) is as wide as its longest line: no narrower (a narrower box wraps a line) and no
// wider than needed (in it no line is wrapped, and a few points of room at most), and no wider than the column.
void FlowTests::aFrameWithNoWidthIsAsWideAsTheColumnOrAsItsLongestLineIfItShrinks()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 10, 43, {}, 25, 40);
    FloatingObject proto;
    proto.hRel = FloatingObject::Rel::Column;
    proto.vRel = FloatingObject::Rel::Paragraph;
    proto.wrap = FloatingObject::Wrap::Square;
    const int a = addFrame(fd, 3, {QStringLiteral("Some words of a frame"), QStringLiteral("and more of them")}, proto, 0);
    fd.objects[size_t(fd.frames[a - 1]) - 1].frameShrinks = true;
    addFrame(fd, 6, {QStringLiteral("A frame that does not shrink, with a few words")}, proto, 0);
    addFrame(fd, 9, {QStringLiteral("a very long line of a frame that shrinks, longer than the column of the page it is on, so that it has to wrap there and then, at the column's width, "
                                    "which is what the column has")}, proto, 0);
    fd.objects[size_t(fd.frames[2]) - 1].frameShrinks = true;
    Paginator pg(&fd);
    const auto page = pg.materialize(0);
    QVERIFY(page);
    const TextItem *narrow = frameBoxWith(*page, QStringLiteral("Some words of a frame")), *plain = frameBoxWith(*page, QStringLiteral("does not shrink")),
                   *wide = frameBoxWith(*page, QStringLiteral("a very long line"));
    QVERIFY(narrow && plain && wide);
    const double column = fd.sections[0].columnWidth();
    QVERIFY2(narrow->rect.width() > 40 && narrow->rect.width() < column - 100, qPrintable(QString::number(narrow->rect.width())));
    QVERIFY2(std::abs(plain->rect.width() - column) < 0.01, qPrintable(QString::number(plain->rect.width())));
    QVERIFY2(std::abs(wide->rect.width() - column) < 0.01, qPrintable(QString::number(wide->rect.width())));
    const Story *story = page->doc->story(narrow->storyId);
    QVERIFY(story);
    auto lines = [&](double width) {
        StoryLayout lay;
        lay.build(story->doc.get(), {plainFrame(width, 100000)}, fd.env);
        return int(lay.lineInfo(0).size());
    };
    QCOMPARE(lines(narrow->rect.width()), 2);          // (a line to a paragraph: nothing is wrapped in it)
    QVERIFY(lines(narrow->rect.width() - 8) > 2);      // (and it is no wider than it must be: a few points of room at most)
}


// A frame belongs to the page of the paragraph after it: where that paragraph begins a page, the frame is on that page and not the one before.
void FlowTests::aFrameIsOnThePageOfItsParagraph()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 20, 44, [](int i, QTextBlockFormat &bf) { if (i == 7) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 25, 40);
    FloatingObject proto;
    proto.hRel = FloatingObject::Rel::Column;
    proto.vRel = FloatingObject::Rel::Paragraph;
    proto.wrap = FloatingObject::Wrap::Square;
    proto.hAlign = FloatingObject::Align::End;
    addFrame(fd, 7, {QStringLiteral("FRAMEWORDS of the frame")}, proto, 120);
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 2);
    const auto first = pg.materialize(0), second = pg.materialize(1);
    QVERIFY(first && second);
    QVERIFY(!frameBoxWith(*first, QStringLiteral("FRAMEWORDS")));
    QVERIFY(frameBoxWith(*second, QStringLiteral("FRAMEWORDS")));
    QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
}


// Edits in a frame's paragraphs, and before and after them, lay out the page of the paragraph after them: the pages are those a new paginator makes, and the frame shows
// what was typed.
void FlowTests::editsInAFrameLayOutThePageItIsOn()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 120, 45, {}, 25, 45);
    FloatingObject proto;
    proto.hRel = FloatingObject::Rel::Column;
    proto.vRel = FloatingObject::Rel::Paragraph;
    proto.wrap = FloatingObject::Wrap::Square;
    proto.hAlign = FloatingObject::Align::End;
    addFrame(fd, 50, {QStringLiteral("FRAMEWORDS in the middle")}, proto, 120);
    Paginator pg(&fd);
    const int pages = pg.pageCount();
    QVERIFY(pages >= 8);
    const QSet<int> frame = frameBlocksOf(fd);
    QCOMPARE(int(frame.size()), 1);
    QTextCursor c(fd.story());
    c.setPosition(*frame.begin() + 5);
    c.insertText(QStringLiteral(" typed into the frame's own paragraph, which takes more of the frame's height, "));
    Paginator fresh0(&fd);
    QString bad = audit::differenceFromFresh(pg, fresh0);
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    int shown = 0;
    for (int k = 0; k < pg.pageCount(); ++k)
        if (const auto m = pg.materialize(k))
            if (const TextItem *box = frameBoxWith(*m, QStringLiteral("typed into the frame"))) {
                ++shown;
                QVERIFY(box->rect.height() > 40);
            }
    QCOMPARE(shown, 1);
    // Taking the paragraph after it away (text before the frame too): the same.
    c.setPosition(fd.story()->findBlockByNumber(40).position());
    c.insertText(QStringLiteral("more words before the frame, enough to move it to the next page or not, as the text goes. "));
    Paginator fresh(&fd);
    bad = audit::differenceFromFresh(pg, fresh);
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
}


// A frame at the start of a continuous section with other columns (the page is laid out in two parts there) loses no text, whether the page has room for the section's first
// lines or not, with a table in the section or without: every paragraph of the section is on the pages.
void FlowTests::aFrameAtTheStartOfAContinuousSectionLosesNoText()
{
    for (const bool table : {false, true})
        for (int before = 3; before <= 14; ++before) {
            FlowDocument fd;
            fd.env.wordLines = true;
            fillStory(fd.story(), before + 12, 48, [&](int i, QTextBlockFormat &bf) { if (i == before) bf.setProperty(fp::SectionBreakAfter, true); }, 25, 40);
            Section second = fd.sections[0];
            second.continuous = true;
            second.columns = 2;
            second.columnGap = 20;
            fd.sections << second;
            FloatingObject proto;
            proto.hRel = FloatingObject::Rel::Column;
            proto.vRel = FloatingObject::Rel::Paragraph;
            proto.wrap = FloatingObject::Wrap::Square;
            addFrame(fd, before + 1, {QStringLiteral("FRAMEWORDS of the second section")}, proto, 100);
            if (table) {
                TestTable spec;
                spec.rows = 4;
                spec.cols = 2;
                spec.columnWidth = 60;
                addTableAfter(fd, before + 3, spec);
            }
            Paginator pg(&fd);
            const QString what = QStringLiteral("%1 paragraphs in the first section, table %2: ").arg(before).arg(table);
            QVERIFY2(pg.pageCount() >= 1, qPrintable(what));
            const QSet<int> frame = frameBlocksOf(fd);
            QSet<int> seen;
            for (int k = 0; k < pg.pageCount(); ++k)
                for (const auto &l : pg.lines(k)) seen.insert(l.block);
            // Every paragraph of the story (but a frame's, and the cells' of a table, which are lines as rows) has a line on a page.
            for (QTextBlock b = fd.story()->begin(); b.isValid(); b = b.next()) {
                if (frame.contains(b.position()) || b.blockFormat().intProperty(fp::Cell)) continue;
                QVERIFY2(seen.contains(b.position()), qPrintable(what + QStringLiteral("the paragraph at %1 (\"%2\") has no line on any page").arg(b.position()).arg(b.text().left(20))));
            }
        }
}


// A frame is set from the paragraph after it, and that may be the empty paragraph that ends a section before one on a page of its own: where the page is full that paragraph has no
// room, and the page keeps it all the same, and the frame is drawn from it, whatever height the page is.
void FlowTests::aFrameBeforeTheEmptyParagraphThatEndsASectionIsDrawnWhateverThePageFoot()
{
    int tested = 0;
    for (double height = 150; height <= 230; height += 1.5) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fd.sections[0].paper = QSizeF(300, height);
        fd.sections[0].margins = QMarginsF(30, 30, 30, 30);
        fillStory(fd.story(), 4, 2, {}, 10, 16);
        appendParagraph(fd, QString(), [](QTextBlockFormat &bf) { bf.setProperty(fp::SectionBreakAfter, true); });
        appendParagraph(fd, QStringLiteral("The next section, on a page of its own."));
        Section second = fd.sections[0];
        fd.sections << second;
        FloatingObject proto;
        proto.hRel = FloatingObject::Rel::Column;
        proto.vRel = FloatingObject::Rel::Paragraph;
        proto.wrap = FloatingObject::Wrap::Square;
        addFrame(fd, 4, {QStringLiteral("FRAMEWORDS of the first section")}, proto, 100);
        Paginator pg(&fd);
        const QString bad = audit::floatProblems(pg, fd);
        QVERIFY2(bad.isEmpty(), qPrintable(QStringLiteral("page %1 high: %2").arg(height).arg(bad)));
        ++tested;
    }
    QVERIFY(tested > 40);
}


// The paginator keeps the frames it has made for a few hundred edits, then lets them go; the page being laid out holds the ones it uses (a pointer to a frame that was let
// go is a use of freed memory, which the address sanitizer's build of this test finds). Typing past the cache's size with frames on the page, the pages stay right.
void FlowTests::framesInUseSurviveTheirCacheTurningOver()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 30, 31, {}, 30, 60);
    FloatingObject proto;
    proto.hRel = FloatingObject::Rel::Column;
    proto.vRel = FloatingObject::Rel::Paragraph;
    proto.hAlign = FloatingObject::Align::End;
    proto.wrap = FloatingObject::Wrap::Square;
    proto.gap = QMarginsF(9, 0, 9, 0);
    addFrame(fd, 3, {QStringLiteral("frame one text")}, proto, 100);
    addFrame(fd, 6, {QStringLiteral("frame two text")}, proto, 100);
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 2);
    QTextCursor cur(fd.story());
    for (int i = 0; i < 260; ++i) {
        cur.setPosition(fd.story()->findBlockByNumber(20).position() + 3);
        cur.insertText(QStringLiteral("x"));
        QVERIFY(pg.pageCount() >= 2);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        if (i % 100 == 0) {
            QVERIFY(frameBoxWith(*page, QStringLiteral("frame one text")));
            QVERIFY(frameBoxWith(*page, QStringLiteral("frame two text")));
        }
    }
    Paginator fresh(&fd);
    const QString bad = audit::differenceFromFresh(pg, fresh);
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
}


// A file can have a frame of thousands of paragraphs: one of no more than a frame has (kMostFrameParagraphs) is a box of its own, and one of more is paragraphs in the text, all
// of them, as they would be without the frame's properties.
void FlowTests::aFrameOfThousandsOfParagraphsIsInTheText()
{
    for (const int count : {kMostFrameParagraphs, kMostFrameParagraphs + 1, 3000}) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 12, 55, {}, 25, 40);
        FloatingObject proto;
        proto.hRel = FloatingObject::Rel::Column;
        proto.vRel = FloatingObject::Rel::Paragraph;
        proto.wrap = FloatingObject::Wrap::Square;
        QStringList paragraphs;
        for (int i = 0; i < count; ++i) paragraphs << QStringLiteral("frame paragraph %1").arg(i);
        addFrame(fd, 4, paragraphs, proto, 120);
        const EngineWork before = EngineWork::now();
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        QVERIFY(pages >= 1);
        const EngineWork did = EngineWork::now().since(before);
        // (A page looks at a frame's paragraph a few times: about 25 characters read for each paragraph, where the square of 3,000 is nine million.)
        QVERIFY2(did.chars < 60 * count + 50000 && did.ranges < 4 * count + 3000, qPrintable(QStringLiteral("%1 frame paragraphs: %2").arg(count).arg(did.text())));
        const auto page = pg.materialize(0);
        QVERIFY(page);
        const TextItem *box = frameBoxWith(*page, QStringLiteral("frame paragraph 0"));
        if (count <= kMostFrameParagraphs) {
            QVERIFY(box);
            QVERIFY(page->doc->story(box->storyId)->doc->blockCount() == count);
        } else {
            QVERIFY2(!box, "a frame of more paragraphs than a frame has is in the text");
        }
        QVERIFY2(audit::coverageProblems(pg, fd.story()).isEmpty(), qPrintable(audit::coverageProblems(pg, fd.story())));
        QVERIFY2(audit::floatProblems(pg, fd).isEmpty(), qPrintable(audit::floatProblems(pg, fd)));
        QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
        // A frame that is paragraphs of the text is no box, and no boxes of its pieces are stacked (a box as tall as the frame's pieces together is taller than a page).
        if (count > kMostFrameParagraphs)
            for (int k = 0; k < pg.pageCount(); ++k) {
                const auto m = pg.materialize(k);
                QVERIFY(m);
                for (const auto &it : m->doc->pages[0]->items) QVERIFY2(it->rect.height() <= m->size.height() + 0.01, qPrintable(audit::rectText(it->rect)));
            }
    }
}


// Frames side by side before a paragraph, and one at the end of the story (nothing after it to be set from, so it is in the text): all are made and drawn where they can
// be, and nothing is lost or stuck.
void FlowTests::framesTogetherAndAtTheEndOfTheStory()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 12, 46, {}, 25, 40);
    FloatingObject proto;
    proto.hRel = FloatingObject::Rel::Column;
    proto.vRel = FloatingObject::Rel::Paragraph;
    proto.wrap = FloatingObject::Wrap::Square;
    FloatingObject left = proto, right = proto;
    left.hAlign = FloatingObject::Align::Start;
    right.hAlign = FloatingObject::Align::End;
    addFrame(fd, 4, {QStringLiteral("LEFTFRAME text")}, left, 90);
    addFrame(fd, 5, {QStringLiteral("RIGHTFRAME text")}, right, 90);   // (after the first frame's paragraph: the two are one after the other before the paragraph that follows)
    // A frame at the very end.
    {
        QTextCursor c(fd.story());
        c.movePosition(QTextCursor::End);
        FloatingObject o = proto;
        o.item = std::make_shared<TextItem>();
        o.frameWidth = 80;
        fd.objects.push_back(o);
        fd.frames.push_back(int(fd.objects.size()));
        fd.objects.last().frame = int(fd.frames.size());
        QTextBlockFormat bf;
        bf.setProperty(fp::Frame, int(fd.frames.size()));
        bf.setProperty(tp::OutOfLayout, true);
        QTextCharFormat cf;
        cf.setFontPointSize(11);
        c.insertBlock(bf, cf);
        c.insertText(QStringLiteral("ENDFRAME text"), cf);
    }
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 1);
    const auto page = pg.materialize(0);
    QVERIFY(page);
    QVERIFY(frameBoxWith(*page, QStringLiteral("LEFTFRAME")));
    QVERIFY(frameBoxWith(*page, QStringLiteral("RIGHTFRAME")));
    QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
    for (int k = 0; k < pg.pageCount(); ++k) QVERIFY(pg.materialize(k));
    // The frame at the very end has no paragraph after it to be set from: its text is lines of the text, with the rest of it.
    QVERIFY2(audit::floatProblems(pg, fd).isEmpty(), qPrintable(audit::floatProblems(pg, fd)));
    QVERIFY2(audit::coverageProblems(pg, fd.story()).isEmpty(), qPrintable(audit::coverageProblems(pg, fd.story())));
    bool endLine = false;
    for (int k = 0; k < pg.pageCount(); ++k)
        for (const auto &l : pg.lines(k)) endLine |= fd.story()->findBlock(l.pos).text().contains(QStringLiteral("ENDFRAME"));
    QVERIFY(endLine);
}


// A paragraph put in between the paragraphs of a frame cuts it in two: the paragraph that the frame was set from now has the second half only (a shorter box, and a wall of
// another height); one put in after the frame is the paragraph it is set from, and the paragraph that it was set from has no frame: the pages from there on are laid out
// again (they are those of a new paginator).
void FlowTests::aParagraphPutInsideAFrameLaysOutThePageOfTheParagraphItWasSetFrom()
{
    QStringList failures;
    for (unsigned seed = 1; seed <= 24; ++seed) {
        const bool after = seed > 12;   // (the paragraph put in after the frame, which it is set from now, and not the paragraph that followed the frame)
        FlowDocument fd;
        fd.env.wordLines = true;
        // (The paragraph the frame is set from begins the second page, so that the page before ends where the edit's end is.)
        fillStory(fd.story(), 40, seed, [](int i, QTextBlockFormat &bf) { if (i == 10) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 30, 60);
        FloatingObject proto;
        proto.hRel = FloatingObject::Rel::Column;
        proto.vRel = FloatingObject::Rel::Paragraph;
        proto.hAlign = FloatingObject::Align::End;
        proto.wrap = FloatingObject::Wrap::Square;
        proto.gap = QMarginsF(8, 2, 8, 2);
        addFrame(fd, 10, {QStringLiteral("first paragraph of the frame, a few words that fill lines of the frame"), QStringLiteral("second paragraph of the frame with more words, so that its box is a good deal taller than one line of the text")}, proto, 110);
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 3);
        const QSet<int> frame = frameBlocksOf(fd);
        QCOMPARE(int(frame.size()), 2);
        const int first = after ? *std::max_element(frame.begin(), frame.end()) : *std::min_element(frame.begin(), frame.end());
        const QTextBlock firstBlock = fd.story()->findBlock(first);
        QTextCursor c(fd.story());
        c.setPosition(firstBlock.position() + firstBlock.length() - 1);
        QTextBlockFormat plain;
        plain.setBottomMargin(6);
        QTextCharFormat cf;
        cf.setFontPointSize(11);
        c.insertBlock(plain, cf);
        c.insertText(QStringLiteral("a paragraph of the text between the two halves of the frame, with a few words."), cf);
        Paginator fresh(&fd);
        const QString bad = audit::differenceFromFresh(pg, fresh);
        if (!bad.isEmpty()) failures << QStringLiteral("seed %1: %2").arg(seed).arg(bad.left(160));
        const QString floats = audit::floatProblems(pg, fd);
        if (!floats.isEmpty()) failures << QStringLiteral("seed %1: %2").arg(seed).arg(floats.left(160));
    }
    QVERIFY2(failures.isEmpty(), qPrintable(QStringLiteral("%1:\n%2").arg(failures.size()).arg(failures.mid(0, 4).join(QLatin1Char('\n')))));
}


// A frame comes right before a table that is in the text: it is set from the table, at its first row, and is drawn.
void FlowTests::aFrameRightBeforeATableIsDrawnAtTheTable()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 8, 41, {}, 25, 40);
    TestTable spec;
    spec.rows = 3;
    spec.cols = 2;
    spec.columnWidth = 70;
    addTableAfter(fd, 4, spec);
    FloatingObject proto;
    proto.hRel = FloatingObject::Rel::Column;
    proto.vRel = FloatingObject::Rel::Paragraph;
    proto.wrap = FloatingObject::Wrap::None;
    proto.hAlign = FloatingObject::Align::End;
    addFrame(fd, 5, {QStringLiteral("FRAMEBEFORETABLE text")}, proto, 90);
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 1);
    const auto page = pg.materialize(0);
    QVERIFY(page);
    const TextItem *box = frameBoxWith(*page, QStringLiteral("FRAMEBEFORETABLE"));
    QVERIFY(box);
    QVERIFY(!page->tables.isEmpty());
    QVERIFY2(std::abs(box->rect.top() - page->tables[0]->rect().top()) < 0.5, qPrintable(audit::rectText(box->rect) + QStringLiteral(" and the table at ") + audit::rectText(page->tables[0]->rect())));
    QVERIFY2(audit::floatProblems(pg, fd).isEmpty(), qPrintable(audit::floatProblems(pg, fd)));
    QVERIFY2(audit::coverageProblems(pg, fd.story()).isEmpty(), qPrintable(audit::coverageProblems(pg, fd.story())));
    QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
}


// A table set at a place and a frame next to one another, in either order, before a paragraph: both are drawn, and every paragraph of the text has its lines (the text
// after them is not lost for the frame's being between the table and it).
void FlowTests::aFloatingTableAndAFrameNextToOneAnotherAreBothDrawn()
{
    for (const bool tableFirst : {true, false}) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 8, 41, {}, 25, 40);
        TestTable spec;
        spec.rows = 3;
        spec.cols = 2;
        spec.columnWidth = 70;
        spec.setTable = [](TableData &t) {
            TableData::Place p;
            p.across = TableData::Place::From::Margin;
            p.acrossSpec = TableData::Place::Spec::End;
            p.room = QMarginsF(9, 0, 9, 0);
            t.place = p;
        };
        addTableAfter(fd, 4, spec);
        FloatingObject proto;
        proto.hRel = FloatingObject::Rel::Column;
        proto.vRel = FloatingObject::Rel::Paragraph;
        proto.wrap = FloatingObject::Wrap::Square;
        proto.hAlign = FloatingObject::Align::Start;
        if (tableFirst) {
            int n = 0, after = -1;
            for (QTextBlock b = fd.story()->begin(); b.isValid() && after < 0; b = b.next(), ++n)
                if (n > 4 && !b.blockFormat().intProperty(fp::Cell)) after = n;
            addFrame(fd, after, {QStringLiteral("FRAMEAFTERTABLE text")}, proto, 90);   // (after the table's last cell, before the paragraph that follows it)
        } else {
            addFrame(fd, 5, {QStringLiteral("FRAMEBEFORETABLE text")}, proto, 90);   // (after block 4, before the table)
        }
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= 1);
        QString where = tableFirst ? QStringLiteral("table, then frame") : QStringLiteral("frame, then table");
        const auto page = pg.materialize(0);
        QVERIFY(page);
        QVERIFY2(frameBoxWith(*page, QStringLiteral("FRAME")), qPrintable(where));
        QVERIFY2(!page->tables.isEmpty() && page->tables[0]->floating, qPrintable(where));
        QVERIFY2(audit::floatProblems(pg, fd).isEmpty(), qPrintable(where + QStringLiteral(": ") + audit::floatProblems(pg, fd)));
        QVERIFY2(audit::coverageProblems(pg, fd.story()).isEmpty(), qPrintable(where + QStringLiteral(": ") + audit::coverageProblems(pg, fd.story())));
        QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(where + QStringLiteral(": ") + audit::problems(pg, fd.story())));
    }
}


// An object anchored in a frame's text (a picture in a frame's paragraph) is set where the frame is, and is drawn.
void FlowTests::anObjectAnchoredInAFramesTextIsDrawn()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 10, 61, {}, 25, 40);
    FloatingObject proto;
    proto.hRel = FloatingObject::Rel::Column;
    proto.vRel = FloatingObject::Rel::Paragraph;
    proto.wrap = FloatingObject::Wrap::Square;
    proto.hAlign = FloatingObject::Align::End;
    addFrame(fd, 4, {QStringLiteral("FRAMEWORDS text")}, proto, 100);
    const QSet<int> frame = frameBlocksOf(fd);
    QCOMPARE(int(frame.size()), 1);
    auto item = std::make_shared<ShapeItem>();
    item->rect = QRectF(0, 0, 33, 22);
    addObject(&fd, item, *frame.begin() + 11, FloatingObject::Rel::Column, FloatingObject::Rel::Paragraph, 0, 0);   // (after "FRAMEWORDS ")
    Paginator pg(&fd);
    const auto page = pg.materialize(0);
    QVERIFY(page);
    QVERIFY(frameBoxWith(*page, QStringLiteral("FRAMEWORDS")));
    QVERIFY2(objectOfSize(*page, 33, 22), "the picture anchored in the frame's text is on the page");
    int count = 0;
    for (const auto &it : page->doc->pages[0]->items) count += it->type() == ItemType::Shape && std::abs(it->rect.width() - 33) < 0.01 && std::abs(it->rect.height() - 22) < 0.01;
    QCOMPARE(count, 1);
}


// Thousands of frames one after another, each its own: the nearest few of them are boxes at the paragraph after them, and the rest are paragraphs in the text, so that no
// word is lost and the pages are as many as the text takes.
void FlowTests::thousandsOfFramesInARowLoseNoText()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 12, 55, {}, 25, 40);
    FloatingObject proto;
    proto.hRel = FloatingObject::Rel::Column;
    proto.vRel = FloatingObject::Rel::Paragraph;
    proto.wrap = FloatingObject::Wrap::Square;
    const int count = 3000;
    for (int i = 0; i < count; ++i) addFrame(fd, 4 + i, {QStringLiteral("FRAMETEXT %1").arg(i)}, proto, 50);
    const EngineWork before = EngineWork::now();
    Paginator pg(&fd);
    const int pages = pg.pageCount();
    const EngineWork did = EngineWork::now().since(before);
    QVERIFY2(did.chars < 60 * count + 50000 && did.ranges < 4 * count + 3000, qPrintable(QStringLiteral("%1 frames: %2").arg(count).arg(did.text())));
    // (Each frame paragraph in the text is a line of its own, 3000 lines and more than the text's own: dozens of pages.)
    QVERIFY2(pages >= 40, qPrintable(QString::number(pages)));
    QVERIFY2(audit::coverageProblems(pg, fd.story()).isEmpty(), qPrintable(audit::coverageProblems(pg, fd.story())));
    QVERIFY2(audit::floatProblems(pg, fd).isEmpty(), qPrintable(audit::floatProblems(pg, fd)));
    int boxes = 0;
    for (int k = 0; k < pages; ++k) {
        const auto page = pg.materialize(k);
        QVERIFY(page);
        int here = 0;
        for (const auto &it : page->doc->pages[0]->items) here += it->type() == ItemType::Text && it->id.startsWith(QLatin1String("flow-object-"));
        QVERIFY2(here <= kMostFloatsAtAParagraph, qPrintable(QStringLiteral("page %1 has %2 frame boxes").arg(k).arg(here)));
        boxes += here;
    }
    QVERIFY(boxes >= 1 && boxes <= kMostFloatsAtAParagraph);
}


// The paragraphs of a frame, and the cells of a table set at a place, have no lines of their own: they are on the page of the paragraph they are set from, which may be the
// page after the one the text before them ends on.
void FlowTests::positionsInAFrameOrAFloatingTableAreOnThePageOfTheirParagraph()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 80, 62, [](int i, QTextBlockFormat &bf) { if (i == 30 || i == 60) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 25, 45);
    // A frame right before the paragraph that begins a page, and a table set at a place right before the one that begins another.
    TestTable spec;
    spec.rows = 2;
    spec.cols = 2;
    spec.columnWidth = 60;
    spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; p.room = QMarginsF(8, 0, 8, 0); t.place = p; };
    FloatingObject proto;
    proto.hRel = FloatingObject::Rel::Column;
    proto.vRel = FloatingObject::Rel::Paragraph;
    proto.wrap = FloatingObject::Wrap::Square;
    proto.hAlign = FloatingObject::Align::End;
    addFrame(fd, 30, {QStringLiteral("FRAMEWORDS first paragraph"), QStringLiteral("and a second of them")}, proto, 100);
    const TestTable table = addTableAfter(fd, 61, spec);   // (before the paragraph that was block 60, with its break: the frame's two paragraphs are before it now)
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 4);
    QSet<int> frame = frameBlocksOf(fd);
    QCOMPARE(int(frame.size()), 2);
    int frameFirst = *std::min_element(frame.begin(), frame.end());
    int frameLast = *std::max_element(frame.begin(), frame.end());
    const QTextBlock afterFrame = fd.story()->findBlock(frameLast).next();
    QVERIFY(afterFrame.isValid());
    const int home = pg.pageOfPosition(afterFrame.position());
    QVERIFY(home >= 1);
    QCOMPARE(pg.pageOfPosition(frameFirst), home);
    QCOMPARE(pg.pageOfPosition(frameFirst + 4), home);
    QCOMPARE(pg.pageOfPosition(frameLast + 3), home);
    QVERIFY(pg.pageOfPosition(fd.story()->findBlockByNumber(28).position() + 5) < home);   // (the text before the frame is on the page before)
    // The table's cells.
    const QTextBlock paragraphAfterTable = fd.story()->findBlock(table.rowStarts.last()).next();
    QTextBlock past = fd.story()->findBlock(table.rowStarts.first());
    while (past.isValid() && past.blockFormat().intProperty(fp::Cell)) past = past.next();
    QVERIFY(past.isValid());
    Q_UNUSED(paragraphAfterTable);
    const int tableHome = pg.pageOfPosition(past.position());
    QVERIFY(tableHome > home);
    for (const int rowStart : table.rowStarts) QCOMPARE(pg.pageOfPosition(rowStart + 2), tableHome);
    // And the page does draw them (so that the caret has a box to be in).
    const auto page = pg.materialize(home);
    QVERIFY(page);
    QVERIFY(frameBoxWith(*page, QStringLiteral("FRAMEWORDS")));
    QVERIFY(!page->frames.isEmpty() && page->frames[0].from == frameFirst);
    const auto tablePage = pg.materialize(tableHome);
    QVERIFY(tablePage && !tablePage->tables.isEmpty() && tablePage->tables[0]->floating);
}


// ---- floating tables

// A table that is set at a place has no lines among the text: the text goes round it, and the page draws its rows where it is set, at the paragraph after the table
// (the place is from that paragraph's top down, and from the margin's right edge across).
void FlowTests::aTableSetAtAPlaceIsDrawnThereAndTheTextGoesRoundIt()
{
    auto make = [](bool withTable) {
        auto fd = std::make_unique<FlowDocument>();
        fd->env.wordLines = true;
        fillStory(fd->story(), 24, 51, {}, 25, 40);
        if (withTable) {
            TestTable spec;
            spec.rows = 3;
            spec.cols = 2;
            spec.columnWidth = 70;
            spec.setTable = [](TableData &t) {
                TableData::Place p;
                p.across = TableData::Place::From::Margin;
                p.acrossSpec = TableData::Place::Spec::End;
                p.y = 8;
                p.room = QMarginsF(9, 0, 9, 0);
                t.place = p;
            };
            addTableAfter(*fd, 5, spec);
        }
        return fd;
    };
    auto plain = make(false), floating = make(true);
    Paginator pp(plain.get()), pg(floating.get());
    QVERIFY(rowLines(pg, *floating, 0).isEmpty());
    // The first paragraph after the table.
    int anchorBlock = -1;
    for (QTextBlock b = floating->story()->findBlockByNumber(6); b.isValid() && anchorBlock < 0; b = b.next())
        if (!b.blockFormat().intProperty(fp::Cell)) anchorBlock = b.position();
    const double top = paragraphTopOn(pg, *floating, 0, anchorBlock);
    QVERIFY(top > 0);
    QVERIFY2(std::abs(top - paragraphTopOn(pp, *plain, 0, plain->story()->findBlockByNumber(6).position())) < 0.01, "the paragraph after the table is where it is without the table");
    const auto a = pageLines(pg, *floating, 0), b = pageLines(pp, *plain, 0);
    int above = 0;
    for (const PageLine &l : a) {
        if (l.block >= anchorBlock) break;
        QVERIFY2(above < b.size() && std::abs(b[above].rect.top() - l.rect.top()) < 0.01, qPrintable(audit::rectText(l.rect)));
        ++above;
    }
    QVERIFY(above > 10);
    const auto page = pg.materialize(0);
    QVERIFY(page);
    QCOMPARE(int(page->tables.size()), 1);
    const TableFragment &t = *page->tables[0];
    QVERIFY(t.floating);
    const flow::Section &sec = floating->sections[0];
    QVERIFY2(std::abs(t.rect().right() - (sec.pageSize().width() - sec.margins.right())) < 0.01 && std::abs(t.rect().width() - 140) < 0.01 && std::abs(t.rect().top() - (top + 8)) < 0.02,
             qPrintable(audit::rectText(t.rect())));
    QVERIFY(t.height > 30 && t.endRow - t.firstRow == 3);
    // The text beside it stands clear of its place and the room (9 points at the sides): left of it.
    const QRectF wall = t.rect().adjusted(-9, 0, 9, 0);
    int beside = 0;
    for (const PageLine &l : a)
        if (l.rect.top() < wall.bottom() - 0.05 && l.rect.bottom() > wall.top() + 0.05) {
            ++beside;
            QVERIFY2(l.rect.right() <= wall.left() + 0.05, qPrintable(audit::rectText(l.rect)));
        }
    QVERIFY2(beside >= 2, qPrintable(QString::number(beside)));
    QVERIFY2(audit::problems(pg, floating->story()).isEmpty(), qPrintable(audit::problems(pg, floating->story())));
    for (int k = 0; k < pg.pageCount(); ++k) QVERIFY2(audit::placementProblems(pg, *floating, k).isEmpty(), qPrintable(audit::placementProblems(pg, *floating, k)));
    // Every paragraph that is not in the table has a line on a page.
    QSet<int> seen;
    for (int k = 0; k < pg.pageCount(); ++k)
        for (const auto &l : pg.lines(k)) seen.insert(l.block);
    for (QTextBlock x = floating->story()->begin(); x.isValid(); x = x.next())
        if (!x.blockFormat().intProperty(fp::Cell)) QVERIFY2(seen.contains(x.position()), qPrintable(QString::number(x.position())));
}


// A table that says it is set at a place but is too tall for one (more rows than a page's text area holds) is in the text, with its rows as lines.
void FlowTests::aTableSetAtAPlaceThatIsTooTallIsInTheText()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 6, 52, {}, 25, 40);
    TestTable spec;
    spec.rows = 80;
    spec.cols = 2;
    spec.columnWidth = 70;
    spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; t.place = p; };
    addTableAfter(fd, 2, spec);
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 2);
    int rows = 0;
    for (int k = 0; k < pg.pageCount(); ++k) {
        rows += int(rowLines(pg, fd, k).size());
        const auto page = pg.materialize(k);
        QVERIFY(page);
        for (const auto &t : page->tables) QVERIFY(!t->floating);
    }
    QVERIFY(rows >= 80);
    QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
}


// A floating table is on the page of the paragraph after it (the one that begins a page: the page it begins), and an edit in its cells lays that page out again: the pages
// are those a new paginator makes, and the table is as tall as its cells need.
void FlowTests::aFloatingTableIsOnThePageOfItsParagraphAndEditsInItLayThatPageOut()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 120, 53, [](int i, QTextBlockFormat &bf) { if (i == 60) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 25, 45);
    TestTable spec;
    spec.rows = 3;
    spec.cols = 2;
    spec.columnWidth = 70;
    spec.text = [](int r, int c) { return QStringLiteral("cell %1 %2").arg(r).arg(c); };
    spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; p.room = QMarginsF(8, 0, 8, 0); t.place = p; };
    const TestTable table = addTableAfter(fd, 59, spec);
    Paginator pg(&fd);
    const int pages = pg.pageCount();
    QVERIFY(pages >= 6);
    int onPage = -1;
    double height = 0;
    for (int k = 0; k < pages; ++k) {
        const auto m = pg.materialize(k);
        QVERIFY(m);
        if (!m->tables.isEmpty()) {
            QCOMPARE(onPage, -1);
            onPage = k;
            height = m->tables[0]->height;
        }
    }
    QVERIFY(onPage > 0);
    // (The paragraph after the table begins that page: a break comes before it.)
    QCOMPARE(pg.page(onPage).start.pos, fd.story()->findBlockByNumber(59 + 1 + 6).position());
    // Typing in a cell: the page is laid out again, the table is taller, and the pages are a new paginator's.
    QTextCursor c(fd.story());
    c.setPosition(table.rowStarts[1] + 3);
    c.insertText(QStringLiteral(" many more words typed into the cell so that it takes several lines of it and the row grows"));
    Paginator fresh(&fd);
    const QString bad = audit::differenceFromFresh(pg, fresh);
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    const auto m = pg.materialize(onPage);
    QVERIFY(m && !m->tables.isEmpty());
    QVERIFY2(m->tables[0]->height > height + 10, qPrintable(QStringLiteral("%1 then %2").arg(height).arg(m->tables[0]->height)));
}


// The same for the tables set at places: the objects made of them are let go from their cache while pages hold them.
void FlowTests::floatingTablesInUseSurviveTheirCacheTurningOver()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 30, 56, {}, 30, 60);
    for (const int after : {8, 4}) {   // (both on the first page: the second is made while the first is held)
        TestTable spec;
        spec.rows = 2;
        spec.cols = 2;
        spec.columnWidth = 70;
        spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; p.room = QMarginsF(8, 0, 8, 0); t.place = p; };
        addTableAfter(fd, after, spec);
    }
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 2);
    QTextCursor cur(fd.story());
    for (int i = 0; i < 260; ++i) {
        cur.setPosition(fd.story()->findBlockByNumber(26).position() + 3);
        cur.insertText(QStringLiteral("x"));
        QVERIFY(pg.pageCount() >= 2);
        const auto page = pg.materialize(0);
        QVERIFY(page);
        if (i % 100 == 0) QVERIFY(!page->tables.isEmpty());
    }
    Paginator fresh(&fd);
    const QString bad = audit::differenceFromFresh(pg, fresh);
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
}


// Tables set at places one after another, before one paragraph, are all drawn, side by side where they ask; and one at the end of the story (no paragraph after it to be set
// from) is rows in the text.
void FlowTests::floatingTablesTogetherAndAtTheEndOfTheStory()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 14, 54, {}, 25, 40);
    auto add = [&](int after, TableData::Place::Spec side) {
        TestTable spec;
        spec.rows = 2;
        spec.cols = 1;
        spec.columnWidth = 60;
        spec.setTable = [side](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = side; p.room = QMarginsF(6, 0, 6, 0); t.place = p; };
        return addTableAfter(fd, after, spec);
    };
    add(5, TableData::Place::Spec::Start);
    // (After the first table's paragraphs: its last cell is a block before the paragraph that follows.)
    {
        QTextBlock last = fd.story()->findBlockByNumber(5);
        while (last.next().isValid() && last.next().blockFormat().intProperty(fp::Cell)) last = last.next();
        int n = 0;
        for (QTextBlock x = fd.story()->begin(); x != last; x = x.next()) ++n;
        add(n, TableData::Place::Spec::End);
    }
    TestTable end;
    end.rows = 1;
    end.cols = 1;
    end.columnWidth = 50;
    end.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; t.place = p; };
    addTable(fd, end);
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 1);
    const auto page = pg.materialize(0);
    QVERIFY(page);
    QVector<std::shared_ptr<TableFragment>> floating;
    for (const auto &t : page->tables)
        if (t->floating) floating << t;
    QCOMPARE(int(floating.size()), 2);   // (the one at the end of the story is rows in the text, on this page or a later one)
    QVERIFY(floating[0]->rect().left() < floating[1]->rect().left());
    QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
    for (int k = 0; k < pg.pageCount(); ++k) QVERIFY(pg.materialize(k));
    // The one at the end of the story has no paragraph to be set from: it is rows in the text, among the lines, where it is.
    QVERIFY2(audit::floatProblems(pg, fd).isEmpty(), qPrintable(audit::floatProblems(pg, fd)));
    bool inText = false;
    for (int k = 0; k < pg.pageCount(); ++k) {
        const auto made = pg.materialize(k);
        QVERIFY(made);
        for (const auto &t : made->tables) inText |= !t->floating;
    }
    QVERIFY(inText);
}


// A file stores a paragraph property by its number: the numbers of the properties that files already have are not moved by a new one (it goes at the end).
void FlowTests::theNumbersOfTheStoredPropertiesAreNotMovedByNewOnes()
{
    QCOMPARE(int(tp::NoHangPunct), int(tp::BorderBottomOff) + 1);
    QVERIFY2(int(tp::OutOfLayout) > int(tp::NoHangPunct), "a property added after the files that store the others goes after them");
}


// Objects of every kind put among paragraphs of a few lines to a few pages, in one column or two: on every page the paginator's lines are where the page layer sets the text
// round the objects (to a hundredth of a point), and clear of the walls. (JP_FUZZ=n: n documents for a longer run.)
void FlowTests::randomWallsAreWhereThePageLayerPutsTheText()
{
    using Wr = FloatingObject::Wrap;
    using Rl = FloatingObject::Rel;
    const unsigned docs = qEnvironmentVariableIsSet("JP_FUZZ") ? std::max(150u, qEnvironmentVariable("JP_FUZZ").toUInt() * 10) : 150u;
    QStringList reports;
    for (unsigned seed = 1; seed <= docs; ++seed) {
        std::mt19937 rng(seed * 104729u);
        FlowDocument fd;
        fd.env.wordLines = true;
        const int paras = 20 + int(rng() % 60);
        fillStory(fd.story(), paras, seed, [&](int, QTextBlockFormat &bf) { bf.setBottomMargin(6); if (rng() % 17 == 0) bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore); }, 5, 90);
        if (rng() % 3 == 0) {
            Section b = fd.sections[0];
            b.columns = 2;
            b.columnGap = 20;
            fd.sections[0] = b;
        }
        for (unsigned i = 0, n = 3 + rng() % 40; i < n; ++i) {
            const Rl hs[] = {Rl::Column, Rl::Margin, Rl::Page, Rl::Character, Rl::LeftMargin, Rl::RightMargin};
            const Rl vs[] = {Rl::Paragraph, Rl::Line, Rl::Margin, Rl::Page, Rl::TopMargin, Rl::BottomMargin};
            const Wr ws[] = {Wr::Square, Wr::Square, Wr::TopBottom, Wr::Tight, Wr::None};
            const FloatingObject::Side sides[] = {FloatingObject::Side::Both, FloatingObject::Side::Left, FloatingObject::Side::Right, FloatingObject::Side::Largest};
            const double w = 10 + double(rng() % 500), h = 10 + double(rng() % (rng() % 4 == 0 ? 900 : 200));
            const int block = int(rng() % unsigned(paras));
            const int id = addWrapped(&fd, w, h, fd.story()->findBlockByNumber(block).position() + int(rng() % 4), ws[rng() % 5], double(int(rng() % 600) - 100), double(int(rng() % 700) - 350),
                                      QMarginsF(double(rng() % 20), double(rng() % 20), double(rng() % 20), double(rng() % 20)), sides[rng() % 4], hs[rng() % 6], vs[rng() % 6]);
            if (rng() % 4 == 0) fd.objects[size_t(id) - 1].hAlign = FloatingObject::Align(1 + rng() % 3);
            if (rng() % 8 == 0) fd.objects[size_t(id) - 1].behind = true;
        }
        Paginator pg(&fd);
        QString bad = audit::wallProblems(pg, fd.story());
        if (bad.isEmpty()) bad = audit::coverageProblems(pg, fd.story());
        if (!bad.isEmpty()) reports << QStringLiteral("seed %1: %2").arg(seed).arg(bad.left(300));
    }
    QVERIFY2(reports.isEmpty(), qPrintable(QStringLiteral("%1 of %2 documents:\n%3").arg(reports.size()).arg(docs).arg(reports.mid(0, 8).join(QLatin1Char('\n')))));
}


// ---- what the objects cost

// A paragraph of many runs (every other word in bold: 20,000 runs) costs no more with an object anchored in another paragraph than without: what a page looks for in the
// runs of the paragraphs it holds is found once for the paragraph, not for each of its lines.
void FlowTests::aParagraphOfManyRunsCostsNoMoreWithAnObjectOnThePage()
{
    EngineWork took[2];
    for (int variant = 0; variant < 2; ++variant) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 3, 5, {}, 30, 40);
        QTextCursor c(fd.story());
        c.movePosition(QTextCursor::End);
        c.insertBlock();
        for (int i = 0; i < 20000; ++i) {
            QTextCharFormat f;
            f.setFontPointSize(11);
            f.setFontWeight(i % 2 ? QFont::Bold : QFont::Normal);
            c.insertText(QStringLiteral("word "), f);
        }
        if (variant == 1) {
            auto item = std::make_shared<ShapeItem>();
            item->rect = QRectF(0, 0, 50, 40);
            const int id = addObject(&fd, item, fd.story()->findBlockByNumber(0).position() + 2, FloatingObject::Rel::Page, FloatingObject::Rel::Page, 500, 700);
            fd.objects[size_t(id) - 1].wrap = FloatingObject::Wrap::None;
        }
        const EngineWork before = EngineWork::now();
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() > 20);
        took[variant] = EngineWork::now().since(before);
    }
    qInfo("20,000 runs with an object on the page: %s; without: %s", qPrintable(took[1].text()), qPrintable(took[0].text()));
    QVERIFY2(took[1].runs < 2 * took[0].runs + 1000 && took[1].chars < 2 * took[0].chars + 1000 && took[1].ranges < 2 * took[0].ranges + 1000,
             qPrintable(QStringLiteral("with the object: %1; without: %2").arg(took[1].text(), took[0].text())));
}


// Ten thousand objects anchored in one paragraph cost little more than the paragraph does without them (anchor marks that anchor nothing): the anchors are cut at the most a
// page is given as they are found, character by character, not paragraph by paragraph, and each is found by its place in the paragraph, not by looking at all its lines.
void FlowTests::manyObjectsInOneParagraphAreCutAsTheyAreFound()
{
    EngineWork took[2];
    for (int variant = 0; variant < 2; ++variant) {
        FlowDocument fd;
        fd.env.wordLines = true;
        fillStory(fd.story(), 6, 5, {}, 80, 120);
        const int n = 10000;
        for (int i = 0; i < n; ++i) {
            auto item = std::make_shared<ShapeItem>();
            item->rect = QRectF(0, 0, 10, 10);
            if (variant == 1) {
                const int id = addObject(&fd, item, fd.story()->findBlockByNumber(2).position() + 2, FloatingObject::Rel::Column, FloatingObject::Rel::Paragraph, (i % 40) * 11, (i / 40) * 11);
                fd.objects[size_t(id) - 1].wrap = FloatingObject::Wrap::Square;
                fd.objects[size_t(id) - 1].gap = QMarginsF(2, 2, 2, 2);
            } else {
                // (The same marks in the text, and no object for any: the document has none.)
                QTextCursor c(fd.story());
                c.setPosition(fd.story()->findBlockByNumber(2).position() + 2);
                QTextCharFormat anchor;
                anchor.setFontPointSize(11);
                anchor.setProperty(fp::AnchorId, i + 1);
                c.insertText(QString(QChar(0xFFFC)), anchor);
            }
        }
        const EngineWork before = EngineWork::now();
        Paginator pg(&fd);
        const int pages = pg.pageCount();
        for (int k = 0; k < pages; ++k) QVERIFY(pg.materialize(k));
        took[variant] = EngineWork::now().since(before);
    }
    qInfo("10,000 anchor marks with the objects: %s; without: %s", qPrintable(took[1].text()), qPrintable(took[0].text()));
    QVERIFY2(took[1].runs < 2 * took[0].runs + 1000 && took[1].chars < 2 * took[0].chars + 1000 && took[1].ranges < 2 * took[0].ranges + 1000,
             qPrintable(QStringLiteral("with 10,000 objects: %1; with none: %2").arg(took[1].text(), took[0].text())));
}


// A page with walls is laid out again to find where they are, once; the walls it ended with are kept, and the page asked for again (its lines, the page made) is laid out
// once, not found again.
void FlowTests::aPageWithWallsIsLaidOutOnceWhenAskedAgain()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 40, 63, {}, 25, 45);
    const int id = addWrapped(&fd, 150, 90, fd.story()->findBlockByNumber(3).position() + 2, FloatingObject::Wrap::Square, 0, 0);
    fd.objects[size_t(id) - 1].hAlign = FloatingObject::Align::End;
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 3);
    QVERIFY(pg.materialize(0));   // (the first time the page is made: its walls are found)
    qint64 before = pg.pagePassesForTest();
    pg.lines(0);
    QCOMPARE(pg.pagePassesForTest() - before, qint64(1));
    before = pg.pagePassesForTest();
    QVERIFY(pg.materialize(0));
    QCOMPARE(pg.pagePassesForTest() - before, qint64(1));
    before = pg.pagePassesForTest();
    pg.lines(0);
    QVERIFY(pg.materialize(0));
    QCOMPARE(pg.pagePassesForTest() - before, qint64(2));
    // After an edit it is found again (and the pages are those of a new paginator).
    QTextCursor c(fd.story());
    c.setPosition(fd.story()->findBlockByNumber(1).position() + 4);
    c.insertText(QStringLiteral("some more words typed before the object, to move it a little. "));
    Paginator fresh(&fd);
    const QString bad = audit::differenceFromFresh(pg, fresh);
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    before = pg.pagePassesForTest();
    pg.lines(0);
    pg.lines(0);
    QVERIFY(pg.pagePassesForTest() - before <= 4);   // (the first after the edit finds them, the second does not)
}


// A table set at a place floats only if it fits the text area of the section it is in: one that is taller than a page of its own section's is in the text, though it would
// fit a page of another section's.
void FlowTests::aTableSetAtAPlaceFloatsOnlyIfItFitsItsOwnSection()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 30, 64, [](int i, QTextBlockFormat &bf) { if (i == 14) bf.setProperty(fp::SectionBreakAfter, true); }, 20, 35);
    Section small = fd.sections[0];
    small.paper = QSizeF(300, 230);
    small.margins = QMarginsF(36, 36, 36, 36);
    small.continuous = false;
    fd.sections << small;
    TestTable spec;
    spec.rows = 12;
    spec.cols = 2;
    spec.columnWidth = 50;
    spec.text = [](int r, int c) { return QStringLiteral("r%1c%2").arg(r).arg(c); };
    spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; p.room = QMarginsF(8, 0, 8, 0); t.place = p; };
    addTableAfter(fd, 22, spec);   // (in the second section, whose pages are 158 points of text tall: the table is taller than 0.9 of that)
    addTableAfter(fd, 6, spec);    // (in the first, whose pages are 648 tall)
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 3);
    int floating = 0, inText = 0, secondSectionFloating = 0;
    for (int k = 0; k < pg.pageCount(); ++k) {
        const auto page = pg.materialize(k);
        QVERIFY(page);
        for (const auto &t : page->tables) {
            if (t->floating) {
                ++floating;
                secondSectionFloating += page->section == 1;
            } else {
                ++inText;
            }
        }
    }
    QCOMPARE(floating, 1);
    QCOMPARE(secondSectionFloating, 0);
    QVERIFY(inText >= 1);
    QVERIFY2(audit::floatProblems(pg, fd).isEmpty(), qPrintable(audit::floatProblems(pg, fd)));
    QVERIFY2(audit::problems(pg, fd.story()).isEmpty(), qPrintable(audit::problems(pg, fd.story())));
}


// A table set at a place from its paragraph, near the foot of the text area, does not go over the foot (or the margin below it, to the page's edge): it is moved, with its
// paragraph, to the top of the next page.
void FlowTests::aFloatingTableThatWouldGoOverTheFootGoesToThePageAfter()
{
    auto make = [](int tableBefore) {
        auto fd = std::make_unique<FlowDocument>();
        fd->env.wordLines = true;
        fillStory(fd->story(), 150, 71, {}, 3, 6);
        if (tableBefore > 0) {
            TestTable spec;
            spec.rows = 6;
            spec.cols = 2;
            spec.columnWidth = 60;
            spec.text = [](int r, int c) { return QStringLiteral("r%1c%2").arg(r).arg(c); };
            spec.setTable = [](TableData &t) { TableData::Place p; p.across = TableData::Place::From::Margin; p.acrossSpec = TableData::Place::Spec::End; p.room = QMarginsF(8, 0, 8, 0); t.place = p; };
            addTableAfter(*fd, tableBefore - 1, spec);
        }
        return fd;
    };
    // A paragraph whose line is a little above the foot of the first page (less than a table's height above it).
    int paragraph = -1;
    {
        auto plain = make(0);
        Paginator pg(plain.get());
        const double bottom = pg.page(0).content.bottom() - pg.page(0).content.top();
        for (const auto &l : pg.lines(0))
            if (l.line == 0 && l.rect.top() > bottom - 70 && l.rect.top() < bottom - 25 && paragraph < 0) {
                int n = 0;
                for (QTextBlock b = plain->story()->begin(); b.isValid() && b.position() < l.block; b = b.next()) ++n;
                paragraph = n;
            }
    }
    QVERIFY(paragraph > 3);
    auto fd = make(paragraph);
    Paginator pg(fd.get());
    QVERIFY(pg.pageCount() >= 4);
    const QTextBlock after = [&]() { QTextBlock b = fd->story()->begin(); while (b.isValid() && !b.blockFormat().intProperty(fp::Cell)) b = b.next(); while (b.isValid() && b.blockFormat().intProperty(fp::Cell)) b = b.next(); return b; }();
    QVERIFY(after.isValid());
    // The paragraph the table is set from is the first on the second page; the table is on that page, whole inside the text area.
    QCOMPARE(pg.page(1).start.pos, after.position());
    QCOMPARE(pg.page(1).start.skip, 0);
    for (int k = 0; k < pg.pageCount(); ++k) {
        const auto page = pg.materialize(k);
        QVERIFY(page);
        const flow::Paginator::Page info = pg.page(k);
        for (const auto &t : page->tables) {
            if (!t->floating) continue;
            QCOMPARE(k, 1);
            QVERIFY2(t->rect().bottom() <= info.content.bottom() + 0.05, qPrintable(audit::rectText(t->rect()) + QStringLiteral(" in ") + audit::rectText(info.content)));
            QVERIFY(t->rect().top() >= info.content.top() - 0.05);
        }
    }
    QVERIFY2(audit::floatProblems(*&pg, *fd).isEmpty(), qPrintable(audit::floatProblems(pg, *fd)));
    QVERIFY2(audit::coverageProblems(pg, fd->story()).isEmpty(), qPrintable(audit::coverageProblems(pg, fd->story())));
    QVERIFY2(audit::problems(pg, fd->story()).isEmpty(), qPrintable(audit::problems(pg, fd->story())));
    // An edit before it: the pages are those of a new paginator (the move is found again on the page it was on, and the table stays with its paragraph).
    QTextCursor c(fd->story());
    c.setPosition(fd->story()->findBlockByNumber(2).position() + 2);
    c.insertText(QStringLiteral("a few words typed on the first page. "));
    Paginator fresh(fd.get());
    const QString bad = audit::differenceFromFresh(pg, fresh);
    QVERIFY2(bad.isEmpty(), qPrintable(bad));
    // The same table with room above it is on the first page, where it goes.
    auto roomy = make(8);
    Paginator pr(roomy.get());
    const auto first = pr.materialize(0);
    QVERIFY(first);
    int floating = 0;
    for (const auto &t : first->tables) floating += t->floating;
    QCOMPARE(floating, 1);
}
