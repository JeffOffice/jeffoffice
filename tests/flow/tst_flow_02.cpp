#include "tst_flow.h"


// A paragraph that is one run, with a page break every few words (or every few spaces, which have no letter to settle small capitals by), over hundreds of pages: each page's lines
// are laid out from the part of the paragraph it holds, and the paragraph's text is read (copied or looked through) a few times in all, not once or more for each page (a paragraph
// of 8,000 pages of one run took more than two minutes).
void FlowTests::aParagraphOfOneRunOverHundredsOfPagesIsReadInProportionToItsText()
{
    for (const bool letters : {true, false}) {
        FlowDocument fd;
        fd.env.wordLines = true;
        const int pages = 400;
        QString text;
        for (int i = 0; i < pages; ++i) text += (letters ? QStringLiteral("word %1 some more text here to fill ").arg(i) : QString(36, QLatin1Char(' '))) + flow::kPageBreak;
        QTextCursor c(fd.story());
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("Arimo")});
        cf.setFontPointSize(11);
        c.insertText(text, cf);
        const qint64 before = StoryLayout::paragraphCharsReadForTest();
        Paginator pg(&fd);
        QVERIFY(pg.pageCount() >= pages);
        const qint64 read = StoryLayout::paragraphCharsReadForTest() - before;
        QVERIFY2(read <= 12 * text.size(), qPrintable(QStringLiteral("%1 characters read for a text of %2 (%3 pages, %4)").arg(read).arg(text.size()).arg(pg.pageCount()).arg(letters ? "words" : "spaces")));
    }
}


// The lines of a paragraph that goes past the end of its frames ask, for each line, whether the paragraph's text is nothing but white space: it is found once, not by copying the
// paragraph's text at each line (a note of thousands of characters under a page's text, which has room for none of it, took more than a minute and a half).
void FlowTests::theLinesPastTheEndOfAFrameAskOnceWhetherTheTextIsBlank()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat cf;
    cf.setFontFamilies({QStringLiteral("Arimo")});
    cf.setFontPointSize(11);
    QString text;
    for (int i = 0; i < 4000; ++i) text += QStringLiteral("word ");
    c.insertText(text, cf);
    LayoutEnv word;
    word.wordLines = true;
    const qint64 before = StoryLayout::paragraphCharsReadForTest();
    StoryLayout layout;
    layout.build(&doc, {plainFrame(300, 40)}, word);
    QVERIFY(layout.lineInfo(0).size() < 10);
    const qint64 read = StoryLayout::paragraphCharsReadForTest() - before;
    QVERIFY2(read <= 12 * text.size(), qPrintable(QStringLiteral("%1 characters read for a text of %2").arg(read).arg(text.size())));
}


// Runs that have the format of the run before them, and the pieces that show no letters between them (a file's bookmarks, which cut a stretch of text into runs of one letter), are
// no ranges of their own for the layout: Qt makes a font for each range each time it looks at it, and a paragraph of 300,000 such letters took each of its pages three quarters of a
// second.
void FlowTests::runsThatShowNothingBetweenThemAreOneRangeOfFormat()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat text, mark;
    text.setFontFamilies({QStringLiteral("Arimo")});
    text.setFontPointSize(11);
    mark = text;
    mark.setProperty(tp::Field, QStringLiteral("raw:"));   // (a piece kept as XML, as a bookmark's mark is: it shows nothing)
    const int letters = 1500;
    for (int i = 0; i < letters; ++i) {
        c.insertText(QString(QChar::ObjectReplacementCharacter), mark);
        c.insertText(QStringLiteral("x"), text);
        c.insertText(QString(QChar::ObjectReplacementCharacter), mark);
    }
    LayoutEnv word;
    word.wordLines = true;
    const qint64 before = StoryLayout::formatRangesMadeForTest();
    StoryLayout layout;
    layout.build(&doc, {plainFrame(300, 4000)}, word);
    const qint64 made = StoryLayout::formatRangesMadeForTest() - before;
    QVERIFY2(made <= 4, qPrintable(QStringLiteral("%1 ranges for %2 letters").arg(made).arg(letters)));
    QVERIFY(layout.lineInfo(0).size() >= 2);
}


// A paragraph of letters with no place to break a line (a file that cuts one long word into runs of a letter) is laid out with its lines allowed to break anywhere, and gets the same
// lines: Qt reads on to the end of such a word for each line, at a font lookup for each run, and a paragraph of 500,000 such runs took each of its pages a second. The lines are
// compared with those of the same text with a space at its end (a place to break, so the paragraph is laid out as any is) in each of the program's own families (every system has
// them), at three widths, and of letters, capitals, and digits: the lines break at the same letters and stand in the same places.
void FlowTests::aParagraphWithNoPlaceToBreakIsLaidOutAsBreakingAnywhereWithTheSameLines()
{
    auto build = [](const QString &family, const QString &letters, const QString &end, qint64 *anywhere, double width) {
        QTextDocument doc;
        QTextCursor c(&doc);
        c.beginEditBlock();   // (the cursor does not find its x for each run)
        for (int i = 0; i < 900; ++i) {
            QTextCharFormat cf;
            cf.setFontFamilies({family});
            cf.setFontPointSize(10 + i % 3);
            cf.setFontWeight(i % 2 ? QFont::Bold : QFont::Normal);
            c.insertText(QString(letters[(i * 7 + i / 5) % letters.size()]), cf);
        }
        if (!end.isEmpty()) {
            QTextCharFormat cf;
            cf.setFontFamilies({family});
            cf.setFontPointSize(11);
            c.insertText(end, cf);
        }
        c.endEditBlock();
        LayoutEnv word;
        word.wordLines = true;
        const qint64 before = StoryLayout::unbreakableParagraphsForTest();
        StoryLayout layout;
        layout.build(&doc, {plainFrame(width, 8000)}, word);
        *anywhere = StoryLayout::unbreakableParagraphsForTest() - before;
        return layout.lineInfo(0);
    };
    const QStringList alphabets = {QStringLiteral("abcdefg"), QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), QStringLiteral("0123456789abcXYZ")};
    for (const QString &family : {QStringLiteral("Arimo"), QStringLiteral("Tinos"), QStringLiteral("Cousine")}) {   // (the program's own: no system font is looked for, nothing is skipped)
        QVERIFY2(QFontDatabase::hasFamily(family), qPrintable(family));
        for (const double width : {137.0, 300.0, 481.5}) {
            for (const QString &letters : alphabets) {
                const QString what = QStringLiteral("%1, %2 wide, %3").arg(family).arg(width).arg(letters.left(4));
                qint64 broken = 0, withSpace = 0;
                const auto lines = build(family, letters, QString(), &broken, width);
                const auto reference = build(family, letters, QStringLiteral(" "), &withSpace, width);
                QVERIFY2(withSpace == 0, qPrintable(what));
                QVERIFY2(broken >= 1, qPrintable(what + ": the paragraph of letters with no place to break was not set to break anywhere"));
                QVERIFY2(lines.size() >= 5, qPrintable(what));
                QCOMPARE(lines.size(), reference.size());
                for (int i = 0; i + 1 < lines.size(); ++i) {   // (the last line of the reference has the space at its end)
                    QVERIFY2(lines[i].docPos == reference[i].docPos && lines[i].docEnd == reference[i].docEnd, qPrintable(QStringLiteral("%1: line %2").arg(what).arg(i)));
                    QVERIFY2(std::abs(lines[i].rect.left() - reference[i].rect.left()) < 1e-6 && std::abs(lines[i].rect.top() - reference[i].rect.top()) < 1e-6 &&
                                 std::abs(lines[i].rect.width() - reference[i].rect.width()) < 1e-6,
                             qPrintable(QStringLiteral("%1: line %2 stands elsewhere").arg(what).arg(i)));
                }
            }
        }
    }
}


// A tab that starts a hair before a default stop goes to that stop, as the list of a line's stops puts it: Qt's own default stops pass over a stop that a tab starts a fraction of a point
// before. (A text of 162 letters i of 11 points, in a paragraph with a stop of its own at 20 points, has its tab start a tenth of a point before the stop at 396; the tab went on to
// 432, and a text after it was a stop to the right.) The size of the letters moves the tab's start across the stop.
void FlowTests::aTabThatStartsJustBeforeADefaultStopGoesToItAsTheListOfStopsPutsIt()
{
    for (int k = -30; k <= 30; ++k) {
        QList<TabPara> paras(1);
        paras[0].stops = {QTextOption::Tab(20, QTextOption::LeftTab)};
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("Arimo")});
        cf.setFontPointSize(11 + k * 0.001);
        paras[0].runs << std::make_pair(QString(162, QLatin1Char('i')) + QStringLiteral("\tzzzzzz"), cf);
        const TabPlaces left = tabPlaces(paras, 36, 450, false), listed = tabPlaces(paras, 36, 450, true);
        QVERIFY(left.lines.size() >= 1);
        const QString why = tabPlacesDiffer(left, listed);
        QVERIFY2(why.isEmpty(), qPrintable(QStringLiteral("size %1: %2").arg(11 + k * 0.001).arg(why)));
    }
}


// A word processor's tab goes where the list of the paragraph's own stops and the default stops past them would put it, in stories of paragraphs with stops of every kind, indents,
// and hanging lines: the lines break at the same letters, and every letter is at the same place.
void FlowTests::tabsOfStoriesWithTheirOwnStopsGoWhereTheListOfDefaultStopsPutsThem()
{
    const QString why = firstTabStoryThatDiffers(36, 150);
    QVERIFY2(why.isEmpty(), qPrintable(why));
}


// The same with default stops 709 twips apart (35.45 points, and 28.35): Qt rounds a distance of stops to 64ths of a pixel, so the stops it goes to drift from the list's.
void FlowTests::tabsToDefaultStopsOfAnUnevenDistanceGoWhereTheListPutsThem()
{
    for (const double step : {35.45, 28.35}) {
        const QString why = firstTabStoryThatDiffers(step, 150);
        QVERIFY2(why.isEmpty(), qPrintable(why));
    }
}


// A paragraph whose text begins with a letter that reads right to left (Hebrew, Arabic) is laid out right to left by Qt whatever the direction it is set to, and Qt turns the left stops of its
// list into right stops: its tabs go where the list of its stops, the default ones too as left stops, puts them. (So do those of a paragraph that begins with a Latin word.)
void FlowTests::tabsOfParagraphsThatReadRightToLeftGoWhereTheListOfDefaultStopsPutsThem()
{
    QVERIFY(QFontDatabase::hasFamily(QStringLiteral("DejaVu Sans")));
    for (const int script : {1, 2, 3})
        for (const double step : {36.0, 35.45}) {
            const QString why = firstTabStoryThatDiffers(step, 60, script);
            QVERIFY2(why.isEmpty(), qPrintable(why));
        }
}


// A paragraph set right to left has Qt's own tabs, as it always had: no default stops are made for its lines, whatever stops of its own it has.
void FlowTests::aParagraphSetRightToLeftIsGivenNoDefaultStops()
{
    for (const int script : {1, 2})
        for (const double step : {36.0, 35.45})
            for (unsigned seed = 0; seed < 30; ++seed) {
                const QList<TabPara> paras = randomTabParas(seed, script, true);
                const qint64 before = StoryLayout::defaultTabStopsMadeForTest();
                const TabPlaces places = tabPlaces(paras, step, 468, false);
                const qint64 made = StoryLayout::defaultTabStopsMadeForTest() - before;
                QVERIFY(places.lines.size() >= 1);
                QVERIFY2(made == 0, qPrintable(QStringLiteral("script %1, story %2, stops %3 apart: %4 default stops were made").arg(script).arg(seed).arg(step).arg(made)));
            }
}


// The default stops past a paragraph's own are not listed in the form that makes Qt read the rest of the paragraph for every tab it answers (a left stop does: a paragraph of 100,000
// absolute position tabs took each page a tenth of a second), whether the lines start on the stops' grid or off it (an indent, a hanging line).
void FlowTests::defaultStopsPastAParagraphsOwnAreNotListedAsStopsThatMakeQtReadTheParagraph()
{
    for (const double left : {0.0, 10.0}) {
        QList<TabPara> paras(1);
        paras[0].stops = {QTextOption::Tab(100, QTextOption::LeftTab)};
        paras[0].left = left;
        for (int i = 0; i < 300; ++i) {
            QTextCharFormat cf;
            cf.setFontFamilies({QStringLiteral("Arimo")});
            cf.setFontPointSize(10 + i % 2);
            paras[0].runs << std::make_pair(QStringLiteral("x\t"), cf);
        }
        const qint64 before = StoryLayout::defaultTabStopsListedForTest();
        const TabPlaces places = tabPlaces(paras, 36, 300, false);
        const qint64 listed = StoryLayout::defaultTabStopsListedForTest() - before;
        QVERIFY2(listed == 0, qPrintable(QStringLiteral("with the lines starting %1 from the edge, %2 default stops were listed as left stops").arg(left).arg(listed)));
        QVERIFY(places.lines.size() >= 25);
        const QString why = tabPlacesDiffer(places, tabPlaces(paras, 36, 300, true));
        QVERIFY2(why.isEmpty(), qPrintable(QStringLiteral("with the lines starting %1 from the edge: %2").arg(left).arg(why)));
    }
}


// A paragraph of many runs over many pages is not passed run by run to the place each page begins at, to the place its text is taken from, nor to where its tabs are looked for: the
// runs before the place were passed by the page before it, and the new page goes on from there (a paragraph of 500,000 runs took each page more than the one before it).
void FlowTests::thePagesOfAParagraphOfManyRunsPassTheRunsBeforeThemOnceInAll()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    const int runs = 30000;
    QTextCursor c(fd.story());
    c.beginEditBlock();   // (the cursor does not find its x for each run)
    for (int i = 0; i < runs; ++i) {
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("Arimo")});
        cf.setFontPointSize(10 + i % 2);
        c.insertText(i % 5 == 4 ? QStringLiteral(" ") : QStringLiteral("w"), cf);
    }
    c.endEditBlock();
    const qint64 before = StoryLayout::runsPassedForTest();
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 8);
    const qint64 passed = StoryLayout::runsPassedForTest() - before;
    QVERIFY2(passed <= 8 * runs, qPrintable(QStringLiteral("%1 runs passed for a paragraph of %2 runs over %3 pages").arg(passed).arg(runs).arg(pg.pageCount())));
}


// The notes before a page that begins inside a paragraph are counted from where the count for the page before it left off, not from the paragraph's start: a paragraph of thousands
// of runs over many pages reads each run's format a few times in all.
void FlowTests::thePagesOfAParagraphOfThousandsOfRunsCountItsNotesFromWhereTheLastLeftOff()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    const int runs = 4000;
    QTextCursor c(fd.story());
    QTextCharFormat plain, bold;
    plain.setFontFamilies({QStringLiteral("Arimo")});
    plain.setFontPointSize(11);
    bold = plain;
    bold.setFontWeight(QFont::Bold);
    for (int i = 0; i < runs; ++i) {
        c.insertText(QStringLiteral("a"), i % 2 ? bold : plain);
        if (i % 330 == 329) c.insertText(QString(flow::kPageBreak), i % 2 ? bold : plain);
    }
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 12);
    QVERIFY2(pg.noteFormatsReadForTest() <= 3 * runs, qPrintable(QStringLiteral("%1 formats read for %2 runs over %3 pages").arg(pg.noteFormatsReadForTest()).arg(runs).arg(pg.pageCount())));
}


// The font database is scanned for the East Asian faces once for the program, not for each character looked up: laying out two documents of hundreds of different ideographs, in a
// face that no machine has, scans it no more than once for each writing system (four, in all).
void FlowTests::theFontDatabaseIsScannedForEastAsianFacesOnce()
{
    const QString family = QStringLiteral("Test Mincho That No Machine Has");
    const qint64 before = eastAsianFontScansForTest();
    for (int round = 0; round < 2; ++round) {
        QString text;
        for (int i = 0; i < 300; ++i) text += QChar(0x4E00 + round * 400 + i * 3);
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat cf;
        cf.setFontFamilies({family});
        cf.setFontPointSize(12);
        c.insertText(text, cf);
        FrameSpec fs;
        fs.size = QSizeF(600, 400);
        fs.insets = QMarginsF(0, 0, 0, 0);
        fs.hyphenate = false;
        LayoutEnv word;
        word.wordLines = true;
        StoryLayout layout;
        layout.build(&doc, {fs}, word);
    }
    const qint64 scans = eastAsianFontScansForTest() - before;
    QVERIFY2(scans <= 4, qPrintable(QStringLiteral("the font database was scanned %1 times").arg(scans)));
}


// Only East Asian characters go looking for a face: an emoji and a letter of the mathematical alphanumeric block do not, and an ideograph of the supplementary planes (U+20000 to
// U+3FFFF) does; Latin text gains no family.
void FlowTests::onlyEastAsianCharactersTakeTheirFaceFromTheMachine()
{
    QTextCharFormat cf;
    cf.setFontFamilies({QStringLiteral("Test Mincho That No Machine Has")});
    const qint64 start = eastAsianLookupsForTest();
    QString noise;
    for (int i = 0; i < 100; ++i) {
        const char32_t emoji = 0x1F600 + i, letter = 0x1D400 + i;
        noise += QString::fromUcs4(&emoji, 1) + QString::fromUcs4(&letter, 1);
    }
    const QTextCharFormat kept = withEastAsianFaces(cf, noise + QStringLiteral(" Latin text, and 12345."));
    QCOMPARE(eastAsianLookupsForTest() - start, qint64(0));
    QCOMPARE(kept.fontFamilies().toStringList(), QStringList{QStringLiteral("Test Mincho That No Machine Has")});
    // (An ideograph of the supplementary planes takes the way, whether or not a face has it.)
    const char32_t extB = 0x20000;
    withEastAsianFaces(cf, QString::fromUcs4(&extB, 1));
    QVERIFY(eastAsianLookupsForTest() - start >= 1);
}


// The family choice, tested directly (so that it fails without the feature on every machine, where Qt's own fallback would draw the characters anyway): a run in a face the machine
// lacks gains, after its own, the family of the machine's that has each of its East Asian characters, the first of the candidates that has it.
void FlowTests::aRunInAMissingFontGainsTheMachinesFamilyThatHasItsEastAsianCharacters()
{
    if (installedFaceFor(0x6771).isEmpty() || installedFaceFor(0x3068).isEmpty()) QSKIP("this machine has no face for the Japanese characters of the test");
    QTextCharFormat cf;
    cf.setFontFamilies({QStringLiteral("Test Mincho That No Machine Has")});
    const QTextCharFormat gained = withEastAsianFaces(cf, QString::fromUtf8("\xe6\x9d\xb1\xe4\xba\xac\xe3\x81\xa8"));
    const QStringList families = gained.fontFamilies().toStringList();
    QVERIFY2(families.size() >= 2 && families.first() == QStringLiteral("Test Mincho That No Machine Has"), qPrintable(families.join(QLatin1Char(','))));
    QVERIFY2(families.contains(installedFaceFor(0x6771)) && families.contains(installedFaceFor(0x3068)), qPrintable(families.join(QLatin1Char(','))));
}


// The line of Courier New is Courier New's wherever it is measured, though the text is drawn in TeX Gyre Cursor (whose own line is 1.201 em, to Courier New's 1.133):
// the number of a list item is a run of its own on the first line, and the paragraph after a keep-with-next one is measured for room.
void FlowTests::aNumberedCourierNewItemKeepsCourierNewsLine()
{
    if (QFontDatabase::hasFamily(QStringLiteral("Courier New"))) QSKIP("Courier New is installed");
    auto doc = std::make_unique<QTextDocument>();
    QTextCursor c(doc.get());
    QTextListFormat lf;
    lf.setStyle(QTextListFormat::ListDecimal);
    lf.setNumberSuffix(QStringLiteral("."));
    QTextCharFormat cf;
    cf.setFontFamilies({QStringLiteral("Courier New")});
    cf.setFontPointSize(20);
    c.setBlockCharFormat(cf);
    c.insertText(QStringLiteral("Text of the item"), cf);
    c.createList(lf);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(doc.get(), {plainFrame(400, 700)}, env);
    const auto lines = lay.lineInfo(0);
    QVERIFY(!lines.isEmpty());
    const double courierNew = (1705 + 615) / 2048.0 * 20;
    QVERIFY2(std::abs(lines[0].rect.height() - courierNew) < 0.01, qPrintable(QStringLiteral("the first line is %1 tall, not %2").arg(lines[0].rect.height()).arg(courierNew)));
    QVERIFY2(std::abs(lines[0].baseline - 1705 / 2048.0 * 20) < 0.02, qPrintable(QString::number(lines[0].baseline)));
}


void FlowTests::keepWithNextMeasuresTheNextParagraphInCourierNewsLine()
{
    if (QFontDatabase::hasFamily(QStringLiteral("Courier New"))) QSKIP("Courier New is installed");
    auto doc = std::make_unique<QTextDocument>();
    QTextCursor c(doc.get());
    QTextCharFormat times;
    times.setFontFamilies({QStringLiteral("Times New Roman")});
    times.setFontPointSize(12);
    c.setBlockCharFormat(times);
    c.insertText(QStringLiteral("A line before"), times);
    QTextBlockFormat keep;
    keep.setProperty(tp::KeepWithNext, true);
    c.insertBlock(keep, times);
    c.insertText(QStringLiteral("Heading"), times);
    QTextCharFormat courier;
    courier.setFontFamilies({QStringLiteral("Courier New")});
    courier.setFontPointSize(20);
    c.insertBlock(QTextBlockFormat(), courier);
    c.insertText(QStringLiteral("The line after it"), courier);
    // Room under the heading for Courier New's line (22.66) but not for the stand-in's own (24.02 for TeX Gyre Cursor, measured, halfway between): the heading stays where it is.
    QFont stand(QStringLiteral("TeX Gyre Cursor"));
    stand.setPixelSize(1000);
    const double own = QFontMetricsF(stand).height() / 1000 * 20, courierNew = (1705 + 615) / 2048.0 * 20;
    if (own < courierNew + 0.5) QSKIP("the stand-in's own line is no taller than Courier New's");
    const double line = (1825 + 443 + 87) / 2048.0 * 12, room = (own + courierNew) / 2;
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(doc.get(), {plainFrame(400, 2 * line + room), plainFrame(400, 700)}, env);
    QCOMPARE(lay.lineInfo(0).size(), 3);
    QCOMPARE(lay.lineInfo(1).size(), 0);
}


// A run set in a stand-in that is drawn narrower than its face (Agency FB, in Archivo Narrow at 85 percent) keeps that to itself: the runs after it in the paragraph are
// drawn at their own widths, not at the first run's.
void FlowTests::aStretchedStandInsStretchStaysWithItsRun()
{
    if (QFontDatabase::hasFamily(QStringLiteral("Agency FB"))) QSKIP("Agency FB is installed");
    auto widthOf = [](const QVector<std::pair<QString, QString>> &runs) {
        auto doc = std::make_unique<QTextDocument>();
        QTextCursor c(doc.get());
        for (const auto &run : runs) {
            QTextCharFormat cf;
            cf.setFontFamilies({run.first});
            cf.setFontPointSize(12);
            c.insertText(run.second, cf);
        }
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(doc.get(), {plainFrame(600, 700)}, env);
        int frame = -1;
        QRectF at;
        return lay.caretRect(doc->characterCount() - 1, &frame, &at) ? at.left() : -1.0;
    };
    const QString narrow = QStringLiteral("Agency FB"), times = QStringLiteral("Times New Roman"), text = QStringLiteral("WWWWWWWWWW");
    const double first = widthOf({{narrow, QStringLiteral("xx")}}), after = widthOf({{times, text}}), both = widthOf({{narrow, QStringLiteral("xx")}, {times, text}});
    QVERIFY2(after > 50, qPrintable(QString::number(after)));
    QVERIFY2(std::abs(both - (first + after)) < 0.05, qPrintable(QStringLiteral("the two runs are %1 wide, the runs alone %2 and %3").arg(both).arg(first).arg(after)));
}


// Lines of whole points follow every kind of line spacing: a multiple is a multiple of the whole-point line (Times New Roman 11, 12.65 high, is 13), an at-least
// height more than that is a descent above its bottom, and one less is the whole-point line itself; an exact height keeps the parts in proportion.
void FlowTests::wholePointLinesFollowMultipleAtLeastAndExactSpacing()
{
    const QString tnr = QStringLiteral("Times New Roman");
    const double descent = 443 / 2048.0 * 11, whole = 13;
    auto one = [&](QTextBlockFormat::LineHeightTypes type, double value) {
        return wordLinesOf({{tnr, 11}}, [&](int, QTextBlockFormat &bf) { bf.setLineHeight(value, type); }, true, [](LayoutEnv &env) { env.wordWholePointLines = true; }).first();
    };
    auto twice = one(QTextBlockFormat::ProportionalHeight, 200);
    QVERIFY2(std::abs(twice.rect.height() - 2 * whole) < 0.01, qPrintable(QString::number(twice.rect.height())));
    QVERIFY2(std::abs(twice.baseline - (whole - descent)) < 0.02, qPrintable(QString::number(twice.baseline)));
    auto most = one(QTextBlockFormat::MinimumHeight, 20);
    QVERIFY(std::abs(most.rect.height() - 20) < 0.01);
    QVERIFY2(std::abs(most.baseline - (20 - descent)) < 0.02, qPrintable(QString::number(most.baseline)));
    auto least = one(QTextBlockFormat::MinimumHeight, 10);
    QVERIFY(std::abs(least.rect.height() - whole) < 0.01);
    QVERIFY2(std::abs(least.baseline - (whole - descent)) < 0.02, qPrintable(QString::number(least.baseline)));
    auto exact = one(QTextBlockFormat::FixedHeight, 26);
    QVERIFY(std::abs(exact.rect.height() - 26) < 0.01);
    QVERIFY2(std::abs(exact.baseline - (whole - descent) * 26 / whole) < 0.02, qPrintable(QString::number(exact.baseline)));
}


// A font the file's font table gives an alternate for is set in that alternate when the word processor has it, whether or not this machine has it (a
// bundled look-alike stands in for it): an unknown serif face with Bookman Old Style as its alternate is not set in Calibri.
void FlowTests::aFontsAlternateIsKeptWhereTheWordProcessorHasIt()
{
    LayoutEnv env;
    env.wordLines = true;
    env.standIns.insert(QStringLiteral("itc bookman light"), QStringLiteral("Bookman Old Style"));
    env.standIns.insert(QStringLiteral("futura md bt"), QStringLiteral("Century Gothic"));
    env.standIns.insert(QStringLiteral("no such face"), QStringLiteral("No Such Alternate"));
    auto familyOf = [&](const QString &family) {
        QTextCharFormat cf;
        cf.setFontFamilies({family});
        cf.setFontPointSize(10);
        return resolveCharFormat(cf, env).fontFamilies().toStringList().value(0);
    };
    QCOMPARE(familyOf(QStringLiteral("ITC Bookman Light")), QStringLiteral("Bookman Old Style"));
    QCOMPARE(familyOf(QStringLiteral("Futura Md BT")), QStringLiteral("Century Gothic"));
    QCOMPARE(familyOf(QStringLiteral("No Such Face")), QStringLiteral("Calibri"));
}


// A word processor kerns the letters of a run only where the file asks for it (tp::KernAbove, from w:kern): a run that doesn't ask isn't kerned at any
// size, where a publisher's text is kerned from 14 points. Kerned text of letter pairs that kern (AV, VA) is narrower, so more of it fits on a line.
void FlowTests::wordTextKernsOnlyWhereTheFileAsks()
{
    auto charsOnFirstLine = [&](bool word, double size, std::optional<double> kernAbove) {
        auto doc = std::make_unique<QTextDocument>();
        QTextCursor c(doc.get());
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("Arial")});
        cf.setFontPointSize(size);
        if (kernAbove) cf.setProperty(tp::KernAbove, *kernAbove);
        c.setCharFormat(cf);
        QString text;
        for (int i = 0; i < 60; ++i) text += QStringLiteral("AVAVAVAV ");
        c.insertText(text, cf);
        LayoutEnv env;
        env.wordLines = word;
        StoryLayout lay;
        lay.build(doc.get(), {plainFrame(300, 700)}, env);
        return lay.lineInfo(0).value(0).text.size();
    };
    const auto plain = charsOnFirstLine(true, 20, std::nullopt);       // asks nothing: not kerned, though 20 points is past 14
    const auto never = charsOnFirstLine(true, 20, 200.0);              // asks from 200 points: not kerned
    const auto asked = charsOnFirstLine(true, 20, 10.0);               // asks from 10 points: kerned
    QCOMPARE(plain, never);
    QVERIFY2(asked > plain, qPrintable(QStringLiteral("kerned %1 characters, plain %2").arg(asked).arg(plain)));
    // (A publisher's text keeps its own rule: kerned from 14 points whether it says so or not.)
    QVERIFY(charsOnFirstLine(false, 20, std::nullopt) > charsOnFirstLine(false, 20, 200.0));
}


// The spaces and tabs that end a word processor's line take no part in its height, and a line that is only such blanks is as tall as the paragraph
// mark's font makes it; blanks among the words count, but a lone space between runs does not.
void FlowTests::blanksAtTheEndOfAWordLineTakeNoPartInItsHeight()
{
    const QString tnr = QStringLiteral("Times New Roman");
    auto single = [&](double size) { return (1825 + 443 + 87) / 2048.0 * size; };
    // One paragraph of runs (text, size in points), its mark `mark` points; the height of its first line.
    auto height = [&](const QVector<std::pair<QString, double>> &runs, double mark) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat markFormat;
        markFormat.setFontFamilies({tnr});
        markFormat.setFontPointSize(mark);
        c.setBlockCharFormat(markFormat);
        for (const auto &r : runs) {
            QTextCharFormat cf;
            cf.setFontFamilies({tnr});
            cf.setFontPointSize(r.second);
            c.insertText(r.first, cf);
        }
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(400, 700)}, env);
        const auto lines = lay.lineInfo(0);
        return lines.isEmpty() ? -1.0 : lines[0].rect.height();
    };
    const QString text = QStringLiteral("some text");
    QVERIFY(std::abs(height({{text, 10}, {QStringLiteral("   "), 24}}, 10) - single(10)) < 0.01);
    QVERIFY(std::abs(height({{text, 10}, {QStringLiteral("\t\t"), 24}}, 10) - single(10)) < 0.01);
    QVERIFY(std::abs(height({{text, 10}, {QStringLiteral(" \t "), 24}}, 10) - single(10)) < 0.01);
    // Only blanks: the mark's font.
    QVERIFY(std::abs(height({{QStringLiteral("\t\t\t"), 24}}, 9) - single(9)) < 0.01);
    QVERIFY(std::abs(height({{QStringLiteral("    "), 24}}, 14) - single(14)) < 0.01);
    // Blanks among the words are part of the line, but one space in a run of its own is not (a reference page of the original program: a space in the paragraph's 12 point font between
    // two runs of 10 point text leaves the line a 10 point line's).
    QVERIFY(std::abs(height({{QStringLiteral("a"), 10}, {QStringLiteral("   "), 24}, {QStringLiteral("b"), 10}}, 10) - single(24)) < 0.01);
    QVERIFY(std::abs(height({{QStringLiteral("a"), 10}, {QStringLiteral(" "), 24}, {QStringLiteral("b"), 10}}, 10) - single(10)) < 0.01);
    QVERIFY(std::abs(height({{QStringLiteral("a b"), 24}}, 10) - single(24)) < 0.01);
    // And the mark doesn't make a line of text taller.
    QVERIFY(std::abs(height({{text, 10}}, 24) - single(10)) < 0.01);
}


// A tab is no part of a word processor's line height, where it stands: a paragraph that begins with a tab in a larger font than its text (the tab that
// indents a first line, in the size the paragraph's last edit left it) is as tall as its text, not as its tab (a reference page of the original program, 16 points for the tab and 12 for the
// words: the lines are 13.8 apart, a 12 point line's).
void FlowTests::aTabTakesNoPartInALineHeight()
{
    const QString tnr = QStringLiteral("Times New Roman");
    auto lineOf = [&](const QVector<std::pair<QString, double>> &runs) {
        QTextDocument doc;
        QTextCursor c(&doc);
        for (const auto &r : runs) {
            QTextCharFormat cf;
            cf.setFontFamilies({tnr});
            cf.setFontPointSize(r.second);
            c.insertText(r.first, cf);
        }
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(400, 700)}, env);
        return lay.lineInfo(0).value(0);
    };
    const double single12 = (1825 + 443 + 87) / 2048.0 * 12, ascent12 = 1825 / 2048.0 * 12;
    const auto led = lineOf({{QStringLiteral("\t"), 16}, {QStringLiteral("Thank you very much"), 12}});
    QVERIFY2(std::abs(led.rect.height() - single12) < 0.01, qPrintable(QString::number(led.rect.height())));
    QVERIFY2(std::abs(led.baseline - ascent12) < 0.02, qPrintable(QString::number(led.baseline)));
    // (Nor does a lone space, in a run of its own; a longer run of spaces is as tall as its font.)
    const auto spaced = lineOf({{QStringLiteral(" "), 16}, {QStringLiteral("Thank you very much"), 12}});
    QVERIFY(std::abs(spaced.rect.height() - single12) < 0.01);
    const auto indented = lineOf({{QStringLiteral("   "), 16}, {QStringLiteral("Thank you very much"), 12}});
    QVERIFY(indented.rect.height() > single12 + 1);
}


// A file that asks for the spacing of an older word processor (w:suppressTopSpacingWP, LayoutEnv::wordWholePointLines) has lines of whole points: a font's single
// line, taken up to the next whole point (10 points of Times New Roman, 11.5 points high, are 12; 11 points, 12.65 high, are 13; 12, 13.8, are 14), with the
// baseline a descent above the bottom (the leading and what is added are above the text); one that doesn't ask has the font's own. Another that asks for no leading (w:noLeading, LayoutEnv::wordNoLeading) has the line of the
// font's Windows ascent and descent alone: 1.1074 em for Times New Roman.
void FlowTests::aFileCanAskForWholePointLinesOrNoLeading()
{
    auto lines = [&](const std::function<void(LayoutEnv &)> &setup) {
        return wordLinesOf({{QStringLiteral("Times New Roman"), 10}, {QStringLiteral("Times New Roman"), 11}, {QStringLiteral("Arial"), 12}, {QStringLiteral("Courier New"), 12}}, {}, true, setup);
    };
    const double descents[] = {443 / 2048.0 * 10, 443 / 2048.0 * 11, 434 / 2048.0 * 12, 615 / 2048.0 * 12};
    const auto whole = lines([](LayoutEnv &env) { env.wordWholePointLines = true; });
    QCOMPARE(whole.size(), 4);
    const double wholeHeights[] = {12, 13, 14, 14};   // (13.59 of Courier New 12 up to 14)
    double top = 0;
    for (int i = 0; i < 4; ++i) {
        QVERIFY2(std::abs(whole[i].rect.height() - wholeHeights[i]) < 0.01, qPrintable(QStringLiteral("line %1 is %2 tall").arg(i).arg(whole[i].rect.height())));
        QVERIFY2(std::abs(whole[i].baseline - (top + wholeHeights[i] - descents[i])) < 0.02, qPrintable(QStringLiteral("line %1's baseline is at %2").arg(i).arg(whole[i].baseline)));
        top += wholeHeights[i];
    }
    const auto plain = lines({});
    QVERIFY(std::abs(plain[0].rect.height() - (1825 + 443 + 87) / 2048.0 * 10) < 0.01);
    const auto none = lines([](LayoutEnv &env) { env.wordNoLeading = true; });
    const double noLeading[] = {(1825 + 443) / 2048.0 * 10, (1825 + 443) / 2048.0 * 11, (1854 + 434) / 2048.0 * 12, (1705 + 615) / 2048.0 * 12};
    for (int i = 0; i < 4; ++i) QVERIFY2(std::abs(none[i].rect.height() - noLeading[i]) < 0.01, qPrintable(QStringLiteral("line %1 is %2 tall").arg(i).arg(none[i].rect.height())));
}


// A file that asks for trailing spaces to wrap (w:wrapTrailSpaces, LayoutEnv::wordWrapTrailSpaces) sets the spaces that end a paragraph as text: where they do not fit the
// line the paragraph's last words are on, they go on to the lines after it. Without it they hang past the margin and make no line. A few that fit make none either way.
void FlowTests::aFileCanAskForTrailingSpacesToWrap()
{
    auto count = [&](const QString &text, bool wrap) {
        auto doc = std::make_unique<QTextDocument>();
        QTextCursor c(doc.get());
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("Times New Roman")});
        cf.setFontPointSize(12);
        c.setBlockCharFormat(cf);
        c.insertText(text, cf);
        LayoutEnv env;
        env.wordLines = true;
        env.wordWrapTrailSpaces = wrap;
        StoryLayout lay;
        lay.build(doc.get(), {plainFrame(400, 700)}, env);
        return int(lay.lineInfo(0).size());
    };
    const QString words = QStringLiteral("Released: December 20, 1999");
    QCOMPARE(count(words + QString(2000, QLatin1Char(' ')), false), 1);
    QVERIFY(count(words + QString(2000, QLatin1Char(' ')), true) >= 3);
    QCOMPARE(count(words + QStringLiteral("  "), true), 1);
    QCOMPARE(count(words, true), 1);
    // (A word that all but fills the line, and spaces after it, put them on a line of their own.)
    QCOMPARE(count(QString(30, QLatin1Char('M')) + QString(40, QLatin1Char(' ')), false), 1);
    QVERIFY(count(QString(30, QLatin1Char('M')) + QString(40, QLatin1Char(' ')), true) >= 2);
}


// A word processor's line does not break after a slash in the text a link's field shows (its address), whatever the room left on the line before it; in the text another field
// shows (a date's slashes), a line does break there, so the rule is the link's (the control: without it a line does end at a slash), and it adds no width, in the face a missing one
// stands in for as in any other.
void FlowTests::aLineDoesNotBreakAfterASlashInALinksAddress()
{
    auto addressEnd = [&](int words, bool link, const QString &family, double *width) {
        auto doc = std::make_unique<QTextDocument>();
        QTextCursor c(doc.get());
        QTextCharFormat cf;
        cf.setFontFamilies({family});
        cf.setFontPointSize(12);
        c.setBlockCharFormat(cf);
        c.insertText(QStringLiteral("ab ").repeated(words), cf);
        QTextCharFormat field = cf;
        field.setProperty(tp::Field, QStringLiteral("raw:www.example.gov/aaaaaaaaaa/bbbbbbbbbb/cc.htm"));
        if (link) field.setProperty(tp::LinkResult, true);
        c.insertText(QString(QChar::ObjectReplacementCharacter), field);
        c.insertText(QStringLiteral("."), cf);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(doc.get(), {plainFrame(8000, 700)}, env);
        {
            int frame = -1;
            QRectF r;
            lay.caretRect(int(QString(QStringLiteral("ab ")).size()) * words + 1, &frame, &r);
            *width = r.x();
        }
    };
    auto endsAtSlash = [&](int words, bool link) {
        auto doc = std::make_unique<QTextDocument>();
        QTextCursor c(doc.get());
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("Times New Roman")});
        cf.setFontPointSize(12);
        c.setBlockCharFormat(cf);
        c.insertText(QStringLiteral("ab ").repeated(words), cf);
        QTextCharFormat field = cf;
        field.setProperty(tp::Field, QStringLiteral("raw:www.example.gov/aaaaaaaaaa/bbbbbbbbbb/cc.htm"));
        if (link) field.setProperty(tp::LinkResult, true);
        c.insertText(QString(QChar::ObjectReplacementCharacter), field);
        c.insertText(QStringLiteral("."), cf);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(doc.get(), {plainFrame(300, 700)}, env);
        for (const auto &line : lay.lineInfo(0)) {
            QString shown = line.text;
            shown.remove(QChar(0x2060));
            if (shown.endsWith(QLatin1Char('/'))) return true;
        }
        return false;
    };
    bool broke = false;
    for (int words = 14; words < 48; ++words) {
        QVERIFY2(!endsAtSlash(words, true), qPrintable(QStringLiteral("%1 words: a line of a link's address ends at a slash").arg(words)));
        broke |= endsAtSlash(words, false);
    }
    QVERIFY2(broke, "control: where the field is no link's, some line ends at a slash");
    for (const QString &family : {QStringLiteral("Times New Roman"), QStringLiteral("Arial"), QStringLiteral("Calibri"), QStringLiteral("Courier New")}) {
        double with = 0, without = 0;
        addressEnd(3, true, family, &with);
        addressEnd(3, false, family, &without);
        QVERIFY2(without > 0 && std::abs(with - without) < 0.01, qPrintable(QStringLiteral("%1: the address is %2 wide with the rule, %3 without").arg(family).arg(with).arg(without)));
    }
}


// The stand-in a file's font table names for a family (the environment's stand-ins, LayoutEnv::standIns) is the line a missing family is set in.
void FlowTests::missingFamilyStandsInAsTheFileSays()
{
    auto height = [&](const QString &family, const QHash<QString, QString> &standIns) {
        auto doc = std::make_unique<QTextDocument>();
        QTextCursor c(doc.get());
        QTextCharFormat cf;
        cf.setFontFamilies({family});
        cf.setFontPointSize(12);
        c.setBlockCharFormat(cf);
        c.insertText(QStringLiteral("Line"), cf);
        LayoutEnv env;
        env.wordLines = true;
        env.standIns = standIns;
        StoryLayout lay;
        lay.build(doc.get(), {plainFrame(400, 700)}, env);
        return lay.lineInfo(0).first().rect.height();
    };
    const QString missing = QStringLiteral("No Such Typeface Anywhere");
    const double calibri = (1950 + 550) / 2048.0 * 12, times = (1825 + 443 + 87) / 2048.0 * 12, arial = (1854 + 434 + 67) / 2048.0 * 12, courier = (1705 + 615) / 2048.0 * 12;
    // Each stand-in the table names is the line it is set in (none of them is the Calibri that a family with no entry gets).
    QVERIFY(std::abs(height(missing, {{missing.toLower(), QStringLiteral("Times New Roman")}}) - times) < 0.01);
    QVERIFY(std::abs(height(missing, {{missing.toLower(), QStringLiteral("Arial")}}) - arial) < 0.01);
    QVERIFY(std::abs(height(missing, {{missing.toLower(), QStringLiteral("Courier New")}}) - courier) < 0.01);
    QVERIFY(std::abs(times - calibri) > 0.5 && std::abs(arial - calibri) > 0.1 && std::abs(courier - calibri) > 0.5);
    // A family with no entry, or an entry that names another family that isn't there, is set in Calibri.
    QVERIFY(std::abs(height(missing, {}) - calibri) < 0.01);
    QVERIFY(std::abs(height(missing, {{missing.toLower(), QStringLiteral("Another Missing Typeface")}}) - calibri) < 0.01);
}


// A section's page numbers in dashes are the number between two hyphens, a space each side.
void FlowTests::pageNumbersInDashes()
{
    struct Dashed : TestNotes {
        QString pageNumberFormat() const override { return QStringLiteral("dash"); }
    } source;
    FieldContext ctx;
    ctx.doc = &source;
    ctx.pageNumber = 3;
    QCOMPARE(ctx.resolve(QStringLiteral("page")), QStringLiteral("- 3 -"));
}


// A number that isn't hanging is followed by a tab: the text begins at the next stop past the number, the paragraph's own or the default ones.
void FlowTests::numberIsFollowedByATab()
{
    auto textStart = [&](double indent, double left, const QList<QTextOption::Tab> &tabs) {
        auto doc = std::make_unique<QTextDocument>();
        QTextCursor c(doc.get());
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDecimal);
        lf.setNumberSuffix(QStringLiteral("."));
        QTextBlockFormat bf;
        bf.setTextIndent(indent);
        bf.setLeftMargin(left);
        if (!tabs.isEmpty()) bf.setTabPositions(tabs);
        c.setBlockFormat(bf);
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("Times New Roman")});
        cf.setFontPointSize(12);
        c.setBlockCharFormat(cf);
        c.insertText(QStringLiteral("Text of the item"), cf);
        c.createList(lf);
        LayoutEnv env;
        env.wordLines = true;
        env.defaultTab = 36;
        StoryLayout lay;
        lay.build(doc.get(), {plainFrame(400, 700)}, env);
        return lay.lineInfo(0).first().rect.left();
    };
    // A number set in at 36 points ends before the stop at 72; one that ends past a stop goes to the next.
    QVERIFY2(std::abs(textStart(36, 0, {}) - 72) < 0.01, qPrintable(QString::number(textStart(36, 0, {}))));
    QVERIFY2(std::abs(textStart(60, 0, {}) - 72) < 0.01, qPrintable(QString::number(textStart(60, 0, {}))));
    // A stop of the paragraph's own is the one it goes to, and the default stops begin past the last of them.
    QVERIFY2(std::abs(textStart(36, 0, {QTextOption::Tab(100, QTextOption::LeftTab)}) - 100) < 0.01, qPrintable(QString::number(textStart(36, 0, {QTextOption::Tab(100, QTextOption::LeftTab)}))));
    QVERIFY2(std::abs(textStart(36, 0, {QTextOption::Tab(40, QTextOption::LeftTab)}) - 72) < 0.01, "a stop before the number's end is passed");
    // A hanging number leaves the text at the paragraph's indent.
    QVERIFY2(std::abs(textStart(-36, 36, {}) - 36) < 0.01, qPrintable(QString::number(textStart(-36, 36, {}))));
}


// A multiple of single spacing makes the line that many times as tall with the text at its top; an exact height sets the baseline as far below the top
// as the font's ascent is of its single line, scaled to the exact height (the text's top is cut off where there is less room than the font's ascent; the reference
// pages: Times New Roman 12 in exactly 24 points has its baseline 2.8 points higher than a descent above the bottom); an at-least height sets it a
// descent above its bottom when it is more than a single line, and is a single line when it is less.
void FlowTests::wordLineHeightsFollowTheirRules()
{
    const QString tnr = QStringLiteral("Times New Roman");
    const double single = (1825 + 443 + 87) / 2048.0 * 12, ascent = 1825 / 2048.0 * 12, descent = 443 / 2048.0 * 12;
    auto one = [&](QTextBlockFormat::LineHeightTypes type, double value) {
        return wordLinesOf({{tnr, 12}}, [&](int, QTextBlockFormat &bf) { bf.setLineHeight(value, type); }).first();
    };
    auto twice = one(QTextBlockFormat::ProportionalHeight, 200);
    QVERIFY(std::abs(twice.rect.height() - 2 * single) < 0.01);
    QVERIFY(std::abs(twice.baseline - ascent) < 0.02);
    auto exact = one(QTextBlockFormat::FixedHeight, 20);
    QVERIFY(std::abs(exact.rect.height() - 20) < 0.01);
    QVERIFY2(std::abs(exact.baseline - ascent * 20 / single) < 0.02, qPrintable(QString::number(exact.baseline)));
    auto tight = one(QTextBlockFormat::FixedHeight, 9);
    QVERIFY(std::abs(tight.rect.height() - 9) < 0.01);
    QVERIFY2(std::abs(tight.baseline - ascent * 9 / single) < 0.02, qPrintable(QString::number(tight.baseline)));
    auto most = one(QTextBlockFormat::MinimumHeight, 24);
    QVERIFY(std::abs(most.rect.height() - 24) < 0.01);
    QVERIFY(std::abs(most.baseline - (24 - descent)) < 0.02);
    auto least = one(QTextBlockFormat::MinimumHeight, 10);
    QVERIFY(std::abs(least.rect.height() - single) < 0.01);
    QVERIFY(std::abs(least.baseline - ascent) < 0.02);
}


// A mark that shows nothing in a line (a tag's or a bookmark's, kept as a character that shows no text) is no run of it: it doesn't make the line taller.
void FlowTests::aMarkThatShowsNothingIsNoRunOfTheLine()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat plainRun;
    plainRun.setFontFamilies({QStringLiteral("Times New Roman")});
    plainRun.setFontPointSize(12);
    QTextCharFormat mark = plainRun;
    mark.setFontPointSize(40);
    mark.setProperty(tp::Field, QStringLiteral("raw:"));
    c.insertText(QStringLiteral("some words "), plainRun);
    c.insertText(QString(QChar::ObjectReplacementCharacter), mark);
    c.insertText(QStringLiteral(" and more"), plainRun);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(400, 700)}, env);
    const auto lines = lay.lineInfo(0);
    QCOMPARE(lines.size(), 1);
    QVERIFY2(std::abs(lines[0].rect.height() - (1825 + 443 + 87) / 2048.0 * 12) < 0.01, qPrintable(QString::number(lines[0].rect.height())));
}


// A list item's number or bullet is a run of its own font on the item's first line, which has room for it above the baseline: a Symbol bullet's line is Symbol's
// ascent over the text's descent.
void FlowTests::aBulletInItsOwnFontHasRoomInTheFirstLine()
{
    auto first = [](const QString &bulletFont) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        c.setBlockCharFormat(run);   // (the paragraph's mark: a word processor's marker is set in it)
        c.insertText(QStringLiteral("an item of the list that is long enough to take two lines of the frame when it is set in it, at twelve points"), run);
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        lf.setIndent(1);
        if (!bulletFont.isEmpty()) lf.setProperty(tp::BulletFont, bulletFont);
        c.createList(lf);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(300, 700)}, env);
        return lay.lineInfo(0);
    };
    const auto plain = first(QString()), symbol = first(QStringLiteral("Symbol"));
    QVERIFY(plain.size() >= 2 && symbol.size() >= 2);
    const double single = (1825 + 443 + 87) / 2048.0 * 12, symbolLine = (2059 + 443) / 2048.0 * 12;
    QVERIFY2(std::abs(plain[0].rect.height() - single) < 0.01, qPrintable(QString::number(plain[0].rect.height())));
    QVERIFY2(std::abs(symbol[0].rect.height() - symbolLine) < 0.01, qPrintable(QString::number(symbol[0].rect.height())));
    QVERIFY(std::abs(symbol[1].rect.height() - single) < 0.01);   // (the lines after it are the text's)
    // A bullet whose own line is shorter than the text's (Courier's) makes it no taller.
    const auto courier = first(QStringLiteral("Courier New"));
    QVERIFY2(std::abs(courier[0].rect.height() - single) < 0.01, qPrintable(QString::number(courier[0].rect.height())));
}


// A level whose number is followed by a space or by nothing (w:suff, tp::NumberSuffixKind) has the text of its item begin right after the number, and one that is followed by a tab
// (the usual) at the stop the indents and the stops give.
void FlowTests::aNumberFollowedByASpaceOrNothingIsFollowedByTheText()
{
    auto textStart = [](int kind) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextBlockFormat bf;
        c.setBlockFormat(bf);
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("Times New Roman")});
        cf.setFontPointSize(12);
        c.setBlockCharFormat(cf);
        c.insertText(QStringLiteral("Introduction"), cf);
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDecimal);
        lf.setNumberSuffix(QStringLiteral("."));
        if (kind > 0) lf.setProperty(tp::NumberSuffixKind, kind);
        c.createList(lf);
        LayoutEnv env;
        env.wordLines = true;
        env.defaultTab = 36;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(300, 200)}, env);
        const auto lines = lay.lineInfo(0);
        return lines.isEmpty() ? -1.0 : lines[0].rect.left();
    };
    const double tab = textStart(0), space = textStart(1), nothing = textStart(2);
    QVERIFY2(nothing > 0 && space > nothing && tab > space * 1.25, qPrintable(QStringLiteral("text starts at %1 after a tab, %2 after a space, %3 after nothing").arg(tab).arg(space).arg(nothing)));
    QVERIFY2((space - nothing) > 12 * 0.1 && (space - nothing) < 12 * 0.5, qPrintable(QStringLiteral("a space is %1 wide").arg(space - nothing)));   // (about a quarter of the size, a tenth to a half of it whatever the face)
}


// A number set in less room than it needs (the text after it is held to the line's end) stays at its indent: the number is not pushed back by as much as the text was held.
void FlowTests::aNumberInARoomTooNarrowStaysAtItsIndent()
{
    for (const bool option : {false, true}) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextBlockFormat bf;
        bf.setLeftMargin(36);
        bf.setTextIndent(-18);
        c.setBlockFormat(bf);
        QTextCharFormat cf;
        cf.setFontFamilies({QStringLiteral("Tinos")});
        cf.setFontPointSize(12);
        c.setBlockCharFormat(cf);
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDecimal);
        lf.setNumberSuffix(QStringLiteral("."));
        c.createList(lf);   // (an item of nothing: a number in a cell as narrow as 26 points)
        LayoutEnv env;
        env.wordLines = true;
        env.wordNoNumberIndentStop = option;
        env.defaultTab = 36;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(26, 200)}, env);
        const auto lines = lay.lineInfo(0);
        QVERIFY(!lines.isEmpty());
        QVERIFY2(std::abs(lines[0].markerX - 18) < 0.01, qPrintable(QString::number(lines[0].markerX) + (option ? " (the option)" : QString())));
    }
}


// A paragraph's borders take room: the top one and its distance from the text above the first line, the bottom one below the last, and none between
// paragraphs with the same borders but the line between them; the side borders take none. The room is added to the space before and after.
void FlowTests::paragraphBordersTakeRoom()
{
    const QString tnr = QStringLiteral("Times New Roman");
    tp::ParaBox box;
    box.top = {QStringLiteral("single"), 8, 3, QColor()};
    box.bottom = {QStringLiteral("double"), 4, 2, QColor()};
    box.left = {QStringLiteral("single"), 4, 4, QColor()};
    const double single = (1825 + 443 + 87) / 2048.0 * 12;
    auto lines = [&](const std::function<void(int, QTextBlockFormat &)> &shape) { return wordLinesOf({{tnr, 12}, {tnr, 12}, {tnr, 12}, {tnr, 12}}, shape); };
    // The second paragraph alone has the borders.
    const auto one = lines([&](int i, QTextBlockFormat &bf) {
        bf.setTopMargin(2);
        bf.setBottomMargin(2);
        if (i == 1) bf.setProperty(tp::Borders, box.spec());
    });
    const auto none = lines([&](int, QTextBlockFormat &bf) {
        bf.setTopMargin(2);
        bf.setBottomMargin(2);
    });
    const double topRoom = 1 + 3, bottomRoom = 4 / 8.0 * 3 + 2;   // (a double border of half point lines is three times that wide)
    QVERIFY(std::abs(one[1].rect.top() - (none[1].rect.top() + topRoom)) < 0.01);
    QVERIFY(std::abs(one[2].rect.top() - (none[2].rect.top() + topRoom + bottomRoom)) < 0.01);
    QVERIFY(std::abs(one[0].rect.top() - none[0].rect.top()) < 0.01);
    QVERIFY(std::abs(one[1].rect.height() - single) < 0.01);
    // Two paragraphs with the same borders are one box: the top is above the first, the bottom below the second, and nothing between them
    // but their space (the larger of the first's after and the second's before).
    const auto pair = lines([&](int i, QTextBlockFormat &bf) {
        bf.setTopMargin(2);
        bf.setBottomMargin(2);
        if (i == 1 || i == 2) bf.setProperty(tp::Borders, box.spec());
    });
    QVERIFY(std::abs(pair[1].rect.top() - (none[1].rect.top() + topRoom)) < 0.01);
    QVERIFY(std::abs((pair[2].rect.top() - pair[1].rect.bottom()) - 2) < 0.01);
    QVERIFY(std::abs(pair[3].rect.top() - (none[3].rect.top() + topRoom + bottomRoom)) < 0.01);
    // (A line between them takes the room its width and distance ask.)
    tp::ParaBox ruled = box;
    ruled.between = {QStringLiteral("single"), 8, 1, QColor()};
    const auto between = lines([&](int i, QTextBlockFormat &bf) {
        bf.setTopMargin(2);
        bf.setBottomMargin(2);
        if (i == 1 || i == 2) bf.setProperty(tp::Borders, ruled.spec());
    });
    QVERIFY(std::abs((between[2].rect.top() - between[1].rect.bottom()) - (2 + 2)) < 0.01);
    // The spec reads back as it was written.
    QVERIFY(tp::ParaBox::parse(ruled.spec()) == ruled);
}


// The first line of a frame goes there, however tall it is, also when its paragraph's top border and the distance of it take room above it: a text box that is shorter than
// the border's room and the line together shows the line (a masthead in a box of its own is not left out).
void FlowTests::theFirstLineOfABorderedParagraphGoesInAFrameThatIsTooShort()
{
    auto layout = [&](bool bordered, double height) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        if (bordered) {
            tp::ParaBox box;
            box.top = {QStringLiteral("single"), 2, 11, QColor(Qt::white)};
            box.bottom = {QStringLiteral("single"), 2, 9, QColor(Qt::white)};
            bf.setProperty(tp::Borders, box.spec());
        }
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("NEWS"), run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(200, height)}, env);
        return lay.lineInfo(0);
    };
    QCOMPARE(int(layout(false, 20).size()), 1);   // (the line alone fits a frame of twenty points)
    const auto bordered = layout(true, 20);
    QVERIFY2(bordered.size() == 1, qPrintable(QStringLiteral("%1 lines of the bordered paragraph in a frame of twenty points").arg(bordered.size())));
    QVERIFY2(bordered[0].rect.top() > 11, qPrintable(QString::number(bordered[0].rect.top())));   // (under the border's room)
}


// The borders are drawn at the paragraph's edges, a distance from the text and outside its indents, and the shading fills the box.
void FlowTests::paragraphBordersAndShadingAreDrawn()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    QTextBlockFormat bf;
    bf.setLeftMargin(30);
    bf.setRightMargin(30);
    bf.setTopMargin(20);
    tp::ParaBox box;
    box.top = {QStringLiteral("single"), 16, 3, QColor(Qt::black)};
    box.bottom = {QStringLiteral("single"), 16, 3, QColor(Qt::black)};
    box.left = {QStringLiteral("single"), 16, 4, QColor(Qt::black)};
    box.right = {QStringLiteral("single"), 16, 4, QColor(Qt::black)};
    box.fill = QColor(200, 200, 255);
    bf.setProperty(tp::Borders, box.spec());
    c.setBlockFormat(bf);
    c.insertText(QStringLiteral("boxed"), run);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(200, 100)}, env);
    const auto lines = lay.lineInfo(0);
    QCOMPARE(lines.size(), 1);
    const double scale = 8;
    QImage img(int(200 * scale), int(100 * scale), QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.scale(scale, scale);
    lay.paint(&p, 0, PaintOptions());
    p.end();
    auto at = [&](double x, double y) { return QColor(img.pixel(int(x * scale), int(y * scale))); };
    const QRectF text = lines[0].rect;
    const double top = text.top(), bottom = text.bottom();
    // The box's edges: the borders 2 points wide, 3 and 4 points from the text, outside the indents (30 points).
    QVERIFY(at(100, top - 3 - 1).lightness() < 40);                    // the top line, 3 points above the text
    QVERIFY(at(100, bottom + 3 + 1).lightness() < 40);                  // the bottom line
    QVERIFY(at(30 - 4 - 1, (top + bottom) / 2).lightness() < 40);      // the left line, 4 points left of the indent
    QVERIFY(at(200 - 30 + 4 + 1, (top + bottom) / 2).lightness() < 40);   // the right line
    QCOMPARE(at(100, top - 3 - 2 - 6).lightness(), 255);               // outside it, nothing
    QCOMPARE(at(15, (top + bottom) / 2).lightness(), 255);
    const QColor inside = at(31 + 100, bottom + 1);                     // between the text and the bottom line, filled
    QVERIFY2(std::abs(inside.red() - 200) < 4 && std::abs(inside.blue() - 255) < 4, qPrintable(inside.name()));
    const QColor gap = at(30 - 2, (top + bottom) / 2);                 // between the left line and the indent, filled
    QVERIFY2(std::abs(gap.red() - 200) < 4, qPrintable(gap.name()));
}


// A word processor's underscore leader is one line from where its tab starts to the stop (W124's "NAME:" and the "COUNTY" after the right tab), not underscores on a grid with a
// gap of two fifths of one at each end.
void FlowTests::anUnderscoreLeaderIsOneLineFromTheTabToItsStop()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Tinos")});
    run.setFontPointSize(12);
    QTextBlockFormat bf;
    bf.setTabPositions({QTextOption::Tab(150, QTextOption::RightTab)});
    bf.setProperty(tp::TabLeaders, QStringLiteral("_"));
    c.setBlockFormat(bf);
    c.insertText(QStringLiteral("Mark:\tCounty"), run);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(200, 100)}, env);
    const QRectF tab = lay.rangeRects(0, 5, 6).first();
    const double scale = 8;
    QImage img(int(200 * scale), int(100 * scale), QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.scale(scale, scale);
    lay.paint(&p, 0, PaintOptions());
    p.end();
    // The row of the underscores: the one with the most dark pixels between the tab's two ends and the text after it.
    int best = 0, row = -1, first = -1, last = -1;
    for (int y = 0; y < img.height(); ++y) {
        int count = 0, a = -1, b = -1;
        for (int x = int(tab.left() * scale) - 20; x < int((tab.right() + 4) * scale); ++x)
            if (x >= 0 && qGray(img.pixel(x, y)) < 128) {
                ++count;
                if (a < 0) a = x;
                b = x;
            }
        if (count > best) { best = count; row = y; first = a; last = b; }
    }
    QVERIFY(row >= 0);
    QVERIFY2(std::abs(first / scale - tab.left()) < 0.4, qPrintable(QStringLiteral("the line starts at %1, the tab at %2").arg(first / scale).arg(tab.left())));
    QVERIFY2(std::abs((last + 1) / scale - tab.right()) < 0.4, qPrintable(QStringLiteral("the line ends at %1, the stop at %2").arg((last + 1) / scale).arg(tab.right())));
}


// A word processor's absolute position tab (a tab that says where it goes) takes the text after it to the margin's right edge, its middle, or its left, whatever stops the
// paragraph has: a footer style's stops at the middle and the right do not catch it (W179's "Azeem_HW13", then "Page 14 of 14" at the margin).
void FlowTests::anAbsolutePositionTabGoesToItsPlaceWhateverTheParagraphsStops()
{
    auto laid = [&](const QString &spec, bool stops, QString *shown) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Tinos")});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        if (stops) bf.setTabPositions({QTextOption::Tab(100, QTextOption::CenterTab), QTextOption::Tab(250, QTextOption::RightTab)});
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("Left"), run);
        QTextCharFormat tab = run;
        tab.setProperty(tp::AlignTab, spec);
        c.insertText(QStringLiteral("\t"), tab);
        c.insertText(QStringLiteral("Page"), run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(300, 100)}, env);
        if (shown) *shown = doc.toPlainText();
        return lay.rangeRects(0, 5, 9).first();
    };
    QString text;
    const QRectF right = laid(QStringLiteral("right:margin:none"), true, &text);
    QVERIFY2(std::abs(right.right() - 300) < 0.5, qPrintable(QStringLiteral("the text ends at %1, the margin is at 300").arg(right.right())));
    const QRectF centered = laid(QStringLiteral("center:margin:none"), true, nullptr);
    QVERIFY2(std::abs((centered.left() + centered.right()) / 2 - 150) < 0.5, qPrintable(QStringLiteral("the text is centered at %1, the middle is 150").arg((centered.left() + centered.right()) / 2)));
    // Without stops of the paragraph's own, as with them.
    const QRectF alone = laid(QStringLiteral("right:margin:none"), false, nullptr);
    QVERIFY2(std::abs(alone.right() - 300) < 0.5, qPrintable(QString::number(alone.right())));
}


// A paragraph that has ordinary tabs and an absolute position tab keeps its own stops for the ordinary ones: "A", a tab to the paragraph's stop at 144, "B", the absolute
// position tab to the margin's right edge, and "C" ending at the margin (the stops of the paragraph were all replaced by the absolute tab's, and "B" was at the margin).
void FlowTests::anAbsolutePositionTabLeavesTheParagraphsStopsToItsOrdinaryTabs()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Tinos")});
    run.setFontPointSize(12);
    QTextBlockFormat bf;
    bf.setTabPositions({QTextOption::Tab(144, QTextOption::LeftTab)});
    c.setBlockFormat(bf);
    c.insertText(QStringLiteral("A\tB"), run);
    QTextCharFormat ptab = run;
    ptab.setProperty(tp::AlignTab, QStringLiteral("right:margin:none"));
    c.insertText(QStringLiteral("\t"), ptab);
    c.insertText(QStringLiteral("C"), run);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(300, 100)}, env);
    QCOMPARE(int(lay.lineBriefs().size()), 1);
    const QRectF b = lay.rangeRects(0, 2, 3).first(), last = lay.rangeRects(0, 4, 5).first();
    QVERIFY2(std::abs(b.left() - 144) < 1, qPrintable(QStringLiteral("B begins at %1, the paragraph's stop is at 144").arg(b.left())));
    QVERIFY2(std::abs(last.right() - 300) < 1, qPrintable(QStringLiteral("C ends at %1, the margin is at 300").arg(last.right())));
}


// A justified line of a script that has no spaces between its words (Han, Kana, Thai) still has its characters spread over the width, as the reference does it: only the
// line of a script with spaces and no space in it is left as it is.
void FlowTests::aJustifiedLineOfAScriptWithoutSpacesStillSpreadsItsCharacters()
{
    const QString text = QString::fromUtf8("\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E").repeated(40);   // (the characters of "Japanese", over and over: more than a line, whatever the width of the machine's)
    auto edgeOf = [&](Qt::Alignment al, double width) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextBlockFormat bf;
        bf.setAlignment(al);
        c.setBlockFormat(bf);
        QTextCharFormat run;
        run.setFontPointSize(10);
        c.insertText(text, run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(width, 200)}, env);
        const auto lines = lay.lineBriefs();
        return lay.rangeRects(0, lines.first().end - 1, lines.first().end).first().right();
    };
    int checked = 0;
    for (double width = 60; width <= 70; width += 0.37) {
        const double natural = edgeOf(Qt::AlignLeft, width);
        if (width - natural < 0.8) continue;   // (a width the characters fill: nothing to spread)
        ++checked;
        QVERIFY2(edgeOf(Qt::AlignJustify, width) > width - 0.6, qPrintable(QStringLiteral("the line ends at %1 of %2 (%3 unjustified)").arg(edgeOf(Qt::AlignJustify, width)).arg(width).arg(natural)));
    }
    QVERIFY(checked > 0);
}


// The leaders of JeffPub's and JeffDeck's text are as they were: an underscore goes in only where it fits between the grid's start and the stop less its gap (two fifths of
// one), not where it is a hundredth of a point over that. The widths and places are the layout's own, measured: the stop is set 0.005 point short of where the first
// underscore would fit.
void FlowTests::anotherProgramsLeaderIsNotGivenAHundredthOfAPointOver()
{
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Tinos")});
    run.setFontPointSize(12);
    auto laidOut = [&](const QString &text, double stop, StoryLayout *lay, QTextDocument *doc) {
        QTextCursor c(doc);
        QTextBlockFormat bf;
        if (stop > 0) {
            bf.setTabPositions({QTextOption::Tab(stop, QTextOption::LeftTab)});
            bf.setProperty(tp::TabLeaders, QStringLiteral("_"));
        }
        c.setBlockFormat(bf);
        c.insertText(text, run);
        LayoutEnv env;   // (not a word processor's)
        lay->build(doc, {plainFrame(200, 100)}, env);
    };
    double x0 = 0, w = 0;
    {
        QTextDocument doc;
        StoryLayout lay;
        laidOut(QStringLiteral("00__________"), 0, &lay, &doc);
        const QRectF digits = lay.rangeRects(0, 0, 2).first(), bars = lay.rangeRects(0, 2, 12).first();
        x0 = digits.right();
        w = bars.width() / 10;
    }
    const double gap = 0.4 * w, start = std::ceil((x0 + gap) / w) * w, stop = start + w + gap - 0.005;
    QTextDocument doc;
    StoryLayout lay;
    laidOut(QStringLiteral("00\tZ"), stop, &lay, &doc);
    const QRectF tab = lay.rangeRects(0, 2, 3).first();
    if (std::abs(tab.right() - stop) > 0.001 || std::abs(tab.left() - x0) > 0.001) QSKIP("the layout puts the stop or the tab elsewhere than it was set");
    const double scale = 8;
    QImage img(int(200 * scale), int(100 * scale), QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.scale(scale, scale);
    lay.paint(&p, 0, PaintOptions());
    p.end();
    int dark = 0;
    for (int y = 0; y < img.height(); ++y)
        for (int x = int((start + 0.5) * scale); x < int((start + w - 0.5) * scale); ++x)
            if (qGray(img.pixel(x, y)) < 128) ++dark;
    QVERIFY2(dark == 0, qPrintable(QStringLiteral("%1 dark pixels where the underscore would go (from %2, %3 wide)").arg(dark).arg(start).arg(w)));
}


// The shading of a paragraph with a hanging indent begins at the hanging line's edge (the least of its indents), and it is one band with the shaded paragraph after it that
// begins there too, though their left indents differ: nothing white between them, nor at the left of the first.
void FlowTests::theShadingOfAHangingParagraphJoinsTheShadedParagraphAfterIt()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    tp::ParaBox box;
    box.fill = QColor(200, 200, 200);
    QTextBlockFormat first;
    first.setLeftMargin(36);
    first.setTextIndent(-36);
    first.setProperty(tp::Borders, box.spec());
    c.setBlockFormat(first);
    c.insertText(QStringLiteral("one"), run);
    QTextBlockFormat second;
    second.setTopMargin(10);
    second.setProperty(tp::Borders, box.spec());
    c.insertBlock(second);
    c.insertText(QStringLiteral("two"), run);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(200, 100)}, env);
    const auto lines = lay.lineInfo(0);
    QCOMPARE(lines.size(), 2);
    const double scale = 8;
    QImage img(int(200 * scale), int(100 * scale), QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.scale(scale, scale);
    lay.paint(&p, 0, PaintOptions());
    p.end();
    auto at = [&](double x, double y) { return QColor(img.pixel(int(x * scale), int(y * scale))); };
    const double middle = (lines[0].rect.bottom() + lines[1].rect.top()) / 2;   // (in the space before the second paragraph)
    QVERIFY2(at(100, middle).lightness() < 210, qPrintable(QStringLiteral("between the paragraphs: %1").arg(at(100, middle).name())));
    QVERIFY2(at(10, (lines[0].rect.top() + lines[0].rect.bottom()) / 2).lightness() < 210, qPrintable(QStringLiteral("left of the first line, in the hang: %1").arg(at(10, lines[0].rect.center().y()).name())));
    QVERIFY2(at(100, lines[1].rect.center().y()).lightness() < 210, "the second paragraph is shaded");
}


// A border round a run of text (a form field's box) is drawn round the run: at its characters' edges, above and below its text, and the run next to it with the same border is one box.
// (The runs are spaces, which have no ink of their own: every dark point on the line is the border, whatever the font and its antialiasing.)
void FlowTests::aBorderRoundARunIsDrawnRoundIt()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat plain;
    plain.setFontFamilies({QStringLiteral("Times New Roman")});
    plain.setFontPointSize(12);
    QTextCharFormat boxed = plain;
    boxed.setProperty(tp::RunBorder, QStringLiteral("single;4;0;000000"));
    c.insertBlock();   // (an empty line above, to have room over the box)
    c.insertText(QStringLiteral("  "), plain);
    c.insertText(QStringLiteral("  "), boxed);
    c.insertText(QStringLiteral("  "), boxed);   // (the same border: one box)
    c.insertText(QStringLiteral("  "), plain);
    const int start = doc.findBlockByNumber(1).position();
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(300, 100)}, env);
    const auto lines = lay.lineInfo(0);
    QCOMPARE(lines.size(), 2);
    const double scale = 8;
    QImage img(int(300 * scale), int(100 * scale), QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.scale(scale, scale);
    lay.paint(&p, 0, PaintOptions());
    p.end();
    // (Only points that are in the picture count: a point outside it is not drawn on.)
    auto dark = [&](double x, double y) { return x >= 0 && y >= 0 && x < 300 && y < 100 && QColor(img.pixel(int(x * scale), int(y * scale))).lightness() < 60; };
    const QRectF first = lay.rangeRects(0, start + 2, start + 4).first(), second = lay.rangeRects(0, start + 4, start + 6).first();
    const double left = first.left(), right = second.right();
    const double top = lines[1].rect.top(), bottom = lines[1].rect.bottom();
    // The dark points of the line: the box round both runs, half a point wide, outside the spaces.
    double x0 = 1e9, x1 = -1, y0 = 1e9, y1 = -1;
    for (double y = top - 3; y < bottom + 3; y += 0.125)
        for (double x = left - 3; x < right + 3; x += 0.125)
            if (dark(x, y)) {
                x0 = std::min(x0, x);
                x1 = std::max(x1, x);
                y0 = std::min(y0, y);
                y1 = std::max(y1, y);
            }
    QVERIFY2(x1 > 0, "a border is drawn round the run");
    QVERIFY2(std::abs(x0 - (left - 0.5)) < 0.3 && std::abs(x1 - (right + 0.5)) < 0.3, qPrintable(QStringLiteral("the box is across %1 to %2, the run is %3 to %4").arg(x0).arg(x1).arg(left).arg(right)));
    QVERIFY2(y0 < top + 0.3 && y1 > bottom - 3 && y1 < bottom + 1, qPrintable(QStringLiteral("the box is down %1 to %2, the line %3 to %4").arg(y0).arg(y1).arg(top).arg(bottom)));
    // One box, not two: nothing is drawn between the runs but the lines above and below (a column of points between them, from the lines to the middle, is white).
    const double join = first.right();
    bool between = false;
    for (double dx = -0.6; dx <= 0.6; dx += 0.1)
        for (double y = y0 + 1.2; y < y1 - 1.2; y += 0.25) between |= dark(join + dx, y);
    QVERIFY2(!between, "the runs next to one another with the same border are one box");
    // And none round the text that has no border (the spaces before it, and the line above).
    QVERIFY(!dark(lay.rangeRects(0, start, start + 2).first().center().x(), (top + bottom) / 2));
}


// The shading of a paragraph with a hanging indent begins at the same place on each page the paragraph is on: the piece that goes on on the next page has no first line (and no
// first-line indent of its own), but the box is the paragraph's, from the edge its first line hangs to.
void FlowTests::theShadingOfAParagraphThatGoesOnBeginsWhereItDidOnThePageBefore()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fd.sections[0].paper = QSizeF(300, 200);
    fd.sections[0].margins = QMarginsF(40, 40, 40, 40);
    QTextCursor c(fd.story());
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    tp::ParaBox box;
    box.fill = QColor(200, 200, 200);
    QTextBlockFormat bf;
    bf.setLeftMargin(36);
    bf.setTextIndent(-36);
    bf.setProperty(tp::Borders, box.spec());
    c.setBlockFormat(bf);
    QString words;
    for (int i = 0; i < 200; ++i) words += QStringLiteral("word ");
    c.insertText(words, run);
    Paginator pg(&fd);
    QVERIFY(pg.pageCount() >= 2);
    auto filledAtTheHang = [&](int k) {
        const auto page = pg.materialize(k);
        const TextItem *box = page ? boxHolding(*page, QStringLiteral("word")) : nullptr;
        if (!box) return false;
        LayoutCache cache;
        const LayoutCache::FrameLayout fl = cache.textFrame(*page->doc, *box, 1, RenderOptions());
        if (!fl.layout || fl.layout->lineInfo(fl.frame).isEmpty()) return false;
        const double scale = 4;
        QImage img(int(box->rect.width() * scale), int((box->rect.height() + 20) * scale), QImage::Format_RGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        p.scale(scale, scale);
        fl.layout->paint(&p, fl.frame, PaintOptions());
        p.end();
        const QRectF line = fl.layout->lineInfo(fl.frame).first().rect;
        return QColor(img.pixel(int(10 * scale), int((line.top() + line.height() / 2) * scale))).lightness() < 230;   // (ten points in: left of the indent, right of the hang)
    };
    QVERIFY2(filledAtTheHang(0), "the shading begins at the hanging line's edge on the first page");
    QVERIFY2(filledAtTheHang(1), "and where it did on the page the paragraph goes on on");
}


// The box round a run is as high as the run's text (the letters' ascent and descent, a point round them), not the line it is on: a line with room above and below the text, as
// double spacing leaves, has a box that is the text's.
void FlowTests::theBorderRoundARunIsAsHighAsItsTextNotItsLine()
{
    const BoxedSpaces b = boxedSpaces(QStringLiteral("single;4;0;000000"), 200);
    QVERIFY(b.run.width() > 0);
    double y0 = 1e9, y1 = -1;
    for (double y = b.line.top() - 6; y < b.line.bottom() + 6; y += 0.125)
        for (double x = b.run.left() - 2; x < b.run.right() + 2; x += 0.125)
            if (b.dark(x, y)) {
                y0 = std::min(y0, y);
                y1 = std::max(y1, y);
            }
    QVERIFY2(y1 > 0, "a box is drawn");
    // (Twelve point type is about fourteen points high; the double spaced line, twenty-eight.)
    QVERIFY2(y1 - y0 > 11 && y1 - y0 < 18, qPrintable(QStringLiteral("the box is %1 points high on a line of %2").arg(y1 - y0).arg(b.line.height())));
}


// A double border round a run is two lines with a gap between (as a paragraph's is), and a dashed one is a line with gaps in it.
void FlowTests::aDoubleAndADashedBorderRoundARunAreDrawnAsSuch()
{
    const BoxedSpaces two = boxedSpaces(QStringLiteral("double;4;0;000000"));
    QVERIFY(two.run.width() > 0);
    // Across the left edge, at the middle of the line: the outer line, a gap, the inner line (half a point each, three halves of a point in all, outside the spaces).
    const double mid = (two.line.top() + two.line.bottom()) / 2;
    QVERIFY2(two.dark(two.run.left() - 0.25, mid) && !two.dark(two.run.left() - 0.75, mid) && two.dark(two.run.left() - 1.25, mid), "two lines with a gap between them");
    const BoxedSpaces dashed = boxedSpaces(QStringLiteral("dashed;8;0;000000"));
    QVERIFY(dashed.run.width() > 0);
    // Along the top edge: dark and white points both.
    int darkPoints = 0, whitePoints = 0;
    double top = 1e9;
    for (double y = dashed.line.top() - 3; y < dashed.line.top() + 4 && top > 1e8; y += 0.125)
        if (dashed.dark(dashed.run.center().x(), y)) top = y;
    for (double x = dashed.run.left(); x < dashed.run.right(); x += 0.25) (dashed.dark(x, top) ? darkPoints : whitePoints)++;
    QVERIFY2(top < 1e8 && darkPoints > 4 && whitePoints > 4, qPrintable(QStringLiteral("%1 dark and %2 white points along the top edge (at %3)").arg(darkPoints).arg(whitePoints).arg(top)));
}


// A word processor's right tab stop past the text area takes the text after its tab only where the text area has margin beside it: in a column or a text box it does not (the text
// would be drawn over the next column or past the box). The page layer, which draws the text, is given the same room the paginator laid the lines with.
void FlowTests::aRightTabPastAColumnTakesNoTextOverTheNextColumnAsDrawn()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fd.sections[0].columns = 2;
    fd.sections[0].columnGap = 36;   // (columns of 216 points on 468)
    QTextCursor c(fd.story());
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    QTextBlockFormat bf;
    bf.setTabPositions({QTextOption::Tab(350, QTextOption::RightTab)});
    c.setBlockFormat(bf);
    c.insertText(QStringLiteral("a\tSTOP"), run);
    Paginator pg(&fd);
    const auto page = pg.materialize(0);
    QVERIFY(page);
    const TextItem *box = boxHolding(*page, QStringLiteral("STOP"));
    QVERIFY(box);
    QCOMPARE(box->columns, 1);
    // The paginator's frames have no room beyond the column; so does the page layer's frame for the box.
    const FrameSpec spec = Renderer::frameSpec(*page->doc, *box, 1, RenderOptions());
    QVERIFY2(spec.rightRoom == 0, qPrintable(QStringLiteral("the box of a column of %1 points is given %2 points of room beyond it").arg(box->rect.width()).arg(spec.rightRoom)));
    // The text as drawn does not go past the column.
    LayoutCache cache;
    const LayoutCache::FrameLayout fl = cache.textFrame(*page->doc, *box, 1, RenderOptions());
    QVERIFY(fl.layout);
    double right = 0;
    for (const QRectF &r : fl.layout->rangeRects(fl.frame, 2, 6)) right = std::max(right, r.right());
    QVERIFY2(right > 0 && right <= box->rect.width() + 0.5, qPrintable(QStringLiteral("the text of a column %1 points wide is drawn to %2").arg(box->rect.width()).arg(right)));
}


// A single column has the margin beside it for a right tab (the room the paginator gives it), and a text box the page layer is given has none.
void FlowTests::aTextBoxIsGivenNoRoomForARightTabBeyondItAndASingleColumnTheMargin()
{
    FlowDocument fd;
    fd.env.wordLines = true;
    fillStory(fd.story(), 3, 7);
    Paginator pg(&fd);
    const auto page = pg.materialize(0);
    QVERIFY(page && !page->boxes.isEmpty());
    const TextItem *column = nullptr;
    for (const auto &item : page->doc->pages[0]->items)
        if (const auto *t = dynamic_cast<const TextItem *>(item.get()))
            if (t->id == page->boxes.first()) column = t;
    QVERIFY(column);
    QCOMPARE(Renderer::frameSpec(*page->doc, *column, 1, RenderOptions()).rightRoom, 72.0);   // (the margin of the page: its text area is as the paginator's frame is)
    TextItem box;
    box.rect = QRectF(100, 100, 200, 100);
    QCOMPARE(Renderer::frameSpec(*page->doc, box, 1, RenderOptions()).rightRoom, 0.0);
}


// A line of text that is as wide as the room it has, to what rounding leaves, fits on one line.
void FlowTests::aLineAsWideAsItsRoomFits()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    c.insertText(QString(36, QLatin1Char('_')), run);   // (half an em each: 216 points)
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(216, 200)}, env);
    QCOMPARE(lay.lineInfo(0).size(), 1);
    StoryLayout narrower;
    narrower.build(&doc, {plainFrame(215, 200)}, env);
    QCOMPARE(narrower.lineInfo(0).size(), 2);
}


// A line fits its room by one rule whether punctuation may hang at its end or not: text of two words that is a tenth of a point over the room it is set in goes to a
// second line in both, and one a tenth under it, or a fifth, goes on one. (Digits, whose ink is inside their advances in any text engine: a letter that reaches past its
// advance, an underscore among them, is counted by the engine of each machine its own way, and the room is a hair more than the text is wide, whatever the machine.)
void FlowTests::aLineFitsAsMuchWithPunctuationThatHangsAsWithoutIt()
{
    auto lines = [&](bool hang, double room) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Tinos")});
        run.setFontPointSize(12);
        c.insertText(QStringLiteral("000000000000000000000000000000000000 0000"), run);   // (36 and 4 half-em digits: 216 points, a space, and 24)
        LayoutEnv env;
        env.wordLines = true;
        env.wordNoHang = !hang;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(room, 200)}, env);
        return int(lay.lineInfo(0).size());
    };
    for (const bool hang : {false, true}) {
        QCOMPARE(lines(hang, 216 + 3 + 24 + 0.2), 1);
        QCOMPARE(lines(hang, 216 + 3 + 24 + 0.1), 1);
        QCOMPARE(lines(hang, 216 + 3 + 24 - 0.1), 2);
        QCOMPARE(lines(hang, 216 + 3 + 24 - 0.2), 2);
    }
}


// A mark of punctuation hangs past the margin, and the text before it may fill the room exactly (the room of two words of 243 points and a period of 3: the period is past the
// margin and the line is one), where a line that ends in a word stops a pixel short of its room. A line that is a tenth of a point over its room is two lines, and so is one
// with no hang.
void FlowTests::aMarkThatHangsLeavesTheTextBeforeItTheRoomExactly()
{
    auto lines = [&](bool hang, double room) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Tinos")});
        run.setFontPointSize(12);
        c.insertText(QStringLiteral("____________________________________ ____."), run);   // (216 points, a space of 3, 24, and the period's 3)
        LayoutEnv env;
        env.wordLines = true;
        env.wordNoHang = !hang;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(room, 200)}, env);
        return int(lay.lineInfo(0).size());
    };
    QCOMPARE(lines(true, 243.0), 1);
    QCOMPARE(lines(true, 243.1), 1);
    QCOMPARE(lines(true, 242.9), 2);
    QCOMPARE(lines(false, 243.0), 2);
    QCOMPARE(lines(false, 246.2), 1);
}


// A closing bracket does not hang past the margin as a period does (V055's "recorded as triggerSetup[0]" line ends before the word, which with its bracket hung would be 1.9
// points over the room): the text before it may not fill the room with the bracket outside, and two words of 243 points and a bracket of 4 are two lines in a room of 243.
void FlowTests::aClosingBracketDoesNotHangPastTheMargin()
{
    auto lines = [&](const QString &mark, double room) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Tinos")});
        run.setFontPointSize(12);
        c.insertText(QString(36, QLatin1Char('0')) + QLatin1Char(' ') + QString(4, QLatin1Char('0')) + mark, run);   // (216 points, a space of 3, 24, and the mark)
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(room, 200)}, env);
        return int(lay.lineInfo(0).size());
    };
    QCOMPARE(lines(QStringLiteral("."), 243.0), 1);   // (a period hangs)
    for (const QString &bracket : {QStringLiteral(")"), QStringLiteral("]"), QStringLiteral("}")})
        QCOMPARE(lines(bracket, 243.0), 2);
    QCOMPARE(lines(QStringLiteral("]"), 250.0), 1);
}


// A word too wide for the line is broken where it must be, and a row of underscores as wide as the room is one line (the lines of a form): the room is given back what the
// engine of the machine counts of an underscore past its advance, measured on the machine and not assumed (an eighth of a point on Linux, less on macOS). A row a fifth of a
// point wider than the room is two.
void FlowTests::aRowOfUnderscoresAsWideAsTheRoomIsOneLine()
{
    const QString row(40, QLatin1Char('_'));
    auto lines = [&](double room) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Tinos")});
        run.setFontPointSize(12);
        c.insertText(row, run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(room, 400)}, env);
        return int(lay.lineInfo(0).size());
    };
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Tinos")});
    run.setFontPointSize(12);
    c.insertText(row, run);
    StoryLayout wide;
    LayoutEnv env;
    env.wordLines = true;
    wide.build(&doc, {plainFrame(1000, 400)}, env);
    int frame = -1;
    QRectF end;
    QVERIFY(wide.caretRect(row.size(), &frame, &end));
    QCOMPARE(lines(end.left()), 1);
    QCOMPARE(lines(end.left() - 0.2), 2);
}


// A word processor breaks a line at spaces and after hyphens and dashes, and in a word that is wider than the line wherever it must: not between two parenthesized parts that
// touch ("(c)(5)(iv)", which V147's pages keep whole), nor after a question mark in an address (V035's "subindex.asp?maintarget="). Digits are 6 points, a bracket 4, a question
// mark 5.3, in a room of 100.
void FlowTests::aLineBreaksNeitherBetweenBracketsThatTouchNorAfterAQuestionMark()
{
    auto lines = [&](const QString &text) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Tinos")});
        run.setFontPointSize(12);
        c.insertText(text, run);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(100, 400)}, env);
        QStringList out;
        for (const auto &line : lay.lineInfo(0)) out << QString(line.text).remove(QChar(0x2060)).remove(QChar(0x200B));   // (the marks that join and divide are not shown)
        return out;
    };
    const QString twelve(12, QLatin1Char('0')), fourteen(14, QLatin1Char('0'));
    // Twelve digits and "(00)" are 92 points, the "(" 96, and one more digit would be 102: the word is broken where it must be, after the second "(", not between the brackets.
    QCOMPARE(lines(twelve + QStringLiteral("(00)(00)")), (QStringList{twelve + QStringLiteral("(00)("), QStringLiteral("00)")}));
    // Fourteen digits and the mark are 89 points, one digit more 95, and two 101: after the first of them, not after the mark.
    QCOMPARE(lines(fourteen + QStringLiteral("?") + twelve), (QStringList{fourteen + QStringLiteral("?0"), QString(11, QLatin1Char('0'))}));
    // A hyphen is where a line breaks (the first part is 88 points).
    QCOMPARE(lines(fourteen + QStringLiteral("-") + twelve).value(0), fourteen + QStringLiteral("-"));
}


// East Asian text breaks after a question mark as it does after a letter, so the rule that keeps an address whole leaves it alone (an ideograph is a letter to QChar), and one
// that goes on in another run of the paragraph is kept as one that goes on in the run: "a?" and "b" in two runs are as "a?b" in one.
void FlowTests::theRuleAfterAQuestionMarkLeavesEastAsianTextAndSpansRuns()
{
    auto firstLine = [&](const QStringList &runs, double room) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat plain, bold;
        plain.setFontFamilies({QStringLiteral("Tinos")});
        plain.setFontPointSize(12);
        bold = plain;
        bold.setFontWeight(QFont::Bold);
        for (int i = 0; i < runs.size(); ++i) c.insertText(runs[i], i % 2 ? bold : plain);
        LayoutEnv env;
        env.wordLines = true;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(room, 400)}, env);
        return QString(lay.lineInfo(0).value(0).text).remove(QChar(0x2060)).remove(QChar(0x200B));
    };
    const QString fourteen(14, QLatin1Char('0'));
    // After the question mark an ideograph begins, where a line may break: ten digits, a space, five digits, and the mark are 98 points, which the 100 hold, the ideographs go on
    // the next line (were the mark joined to the first of them, the line would end at the space).
    const QString han = QString(QChar(0x4E2D)) + QString(QChar(0x6587)) + QString(QChar(0x5B57));
    const QString head = QString(10, QLatin1Char('0')) + QLatin1Char(' ') + QString(5, QLatin1Char('0')) + QStringLiteral("?");
    QCOMPARE(firstLine({head + han}, 100), head);
    // The same as one run, and as two (the second begins where the first ends): the digits wrap at the room, not after the mark.
    const QString expected = fourteen + QStringLiteral("?0");
    QCOMPARE(firstLine({fourteen + QStringLiteral("?") + QString(12, QLatin1Char('0'))}, 100), expected);
    QCOMPARE(firstLine({fourteen + QStringLiteral("?"), QString(12, QLatin1Char('0'))}, 100), expected);
    // The same for brackets that touch.
    QCOMPARE(firstLine({QString(12, QLatin1Char('0')) + QStringLiteral("(00)"), QStringLiteral("(00)")}, 100), QString(12, QLatin1Char('0')) + QStringLiteral("(00)("));
}


// The rules that keep a line from breaking after a question mark and between brackets leave a break where the stretch since the last place a line may break is a hundred and
// twenty letters: a file of "a?a?..." (the stretch of a hostile file is as long as it likes) is laid out in pieces, not as one word that Qt sets a line at a time over all of
// it (80,000 letters took 1.6 seconds, 60 times what the same with spaces took).
void FlowTests::theRulesAgainstABreakAreBoundedByTheLengthOfTheStretch()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Tinos")});
    run.setFontPointSize(12);
    QString text;
    for (int i = 0; i < 40000; ++i) text += QStringLiteral("a?");
    c.insertText(text, run);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(300, 400)}, env);
    QVERIFY2(StoryLayout::longestJoinedStretchForTest() >= 20, qPrintable(QString::number(StoryLayout::longestJoinedStretchForTest())));   // (the rule is at work)
    QVERIFY2(StoryLayout::longestJoinedStretchForTest() <= 120, qPrintable(QStringLiteral("a stretch of %1 letters is kept whole").arg(StoryLayout::longestJoinedStretchForTest())));
}


// A line that ends where a break may be (after a dash) is fitted as the others are: the room is not given back, as it is for a word wider than the line. What the first
// line needs to take the em dash, over the width of the text to its end (the dash's advance and what its machine's engine counts past it: found by halving the interval, not
// assumed), is the same where the line breaks before the dash as where it breaks at a space before it, which is fitted as any line is. Were the room given back where a line
// breaks at the dash, the first would need an eighth of a point less.
void FlowTests::aLineThatEndsAtADashIsFittedLikeTheOthers()
{
    // The least room in which the first line takes the dash, less the text's width to the dash's end.
    auto needs = [&](const QString &text) {
        auto build = [&](double room, QTextDocument *doc, StoryLayout *lay) {
            QTextCursor c(doc);
            QTextCharFormat run;
            run.setFontFamilies({QStringLiteral("Tinos")});
            run.setFontPointSize(12);
            c.insertText(text, run);
            LayoutEnv env;
            env.wordLines = true;
            env.wordNoHang = true;
            lay->build(doc, {plainFrame(room, 200)}, env);
        };
        auto takesTheDash = [&](double room) {
            QTextDocument doc;
            StoryLayout lay;
            build(room, &doc, &lay);
            const auto lines = lay.lineInfo(0);
            return !lines.isEmpty() && lines[0].text.contains(QChar(0x2014));
        };
        double width = 0;
        {
            QTextDocument doc;
            StoryLayout lay;
            build(1000, &doc, &lay);
            int frame = -1;
            QRectF at;
            if (!lay.caretRect(int(text.indexOf(QChar(0x2014))) + 1, &frame, &at)) return -1000.0;
            width = at.left();
        }
        double lo = 100, hi = 300;   // (the text is about 216 points)
        if (takesTheDash(lo) || !takesTheDash(hi)) return -1000.0;
        while (hi - lo > 0.0005) {
            const double mid = (lo + hi) / 2;
            (takesTheDash(mid) ? hi : lo) = mid;
        }
        return hi - width;
    };
    const QString zeros(34, QLatin1Char('0')), later(4, QLatin1Char('0'));
    const double atSpace = needs(QString(33, QLatin1Char('0')) + QLatin1Char(' ') + QChar(0x2014) + later);   // (breaks at the space)
    const double atDash = needs(zeros + QChar(0x2014) + later);                                                  // (breaks before the dash)
    QVERIFY2(atSpace > -100 && atDash > -100, qPrintable(QStringLiteral("at a space %1, at the dash %2").arg(atSpace).arg(atDash)));
    QVERIFY2(std::abs(atDash - atSpace) < 0.01, qPrintable(QStringLiteral("the dash needs %1 more than its width where the line breaks before it, %2 where it breaks at a space").arg(atDash).arg(atSpace)));
}


// Hidden text takes no room and shows nothing: a line's text is what is left, and a paragraph of nothing else is an empty line.
void FlowTests::hiddenTextIsLeftOut()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    QTextCharFormat hidden = run;
    hidden.setProperty(tp::HiddenText, true);
    c.insertText(QStringLiteral("seen "), run);
    c.insertText(QStringLiteral("<@33><i>hidden codes "), hidden);
    c.insertText(QStringLiteral("and seen again"), run);
    c.insertBlock();
    c.insertText(QStringLiteral("only hidden"), hidden);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(400, 700)}, env);
    const auto lines = lay.lineInfo(0);
    QCOMPARE(lines.size(), 2);
    QCOMPARE(lines[0].text, QStringLiteral("seen and seen again"));
    QCOMPARE(lines[1].text, QString());
    // (As before, for a text that isn't a word processor's: the characters are shown.)
    StoryLayout plain;
    plain.build(&doc, {plainFrame(400, 700)}, LayoutEnv());
    QVERIFY(plain.lineInfo(0)[0].text.contains(QStringLiteral("hidden codes")));
}


// Automatic space before and after (a paragraph from a web page) is 14 points, whatever the margins say.
void FlowTests::automaticSpaceIsFourteenPoints()
{
    const QString tnr = QStringLiteral("Times New Roman");
    const auto lines = wordLinesOf({{tnr, 12}, {tnr, 12}, {tnr, 12}}, [](int i, QTextBlockFormat &bf) {
        bf.setTopMargin(5);
        bf.setBottomMargin(5);
        if (i == 1) {
            bf.setProperty(tp::SpaceBeforeAuto, true);
            bf.setProperty(tp::SpaceAfterAuto, true);
        }
    });
    // The first's space after (5) and the second's automatic before (14): the larger; then its automatic after and the third's before (5).
    QVERIFY(std::abs((lines[1].rect.top() - lines[0].rect.bottom()) - 14) < 0.01);
    QVERIFY(std::abs((lines[2].rect.top() - lines[1].rect.bottom()) - 14) < 0.01);
    // Between two items of a list, none; before the first and after the last, the automatic space.
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({tnr});
    run.setFontPointSize(12);
    QTextBlockFormat bf;
    bf.setProperty(tp::SpaceBeforeAuto, true);
    bf.setProperty(tp::SpaceAfterAuto, true);
    c.setBlockFormat(bf);
    c.insertText(QStringLiteral("intro"), run);
    QTextListFormat lf;
    lf.setStyle(QTextListFormat::ListDisc);
    QTextList *list = nullptr;
    for (int i = 0; i < 3; ++i) {
        c.insertBlock(bf);
        c.insertText(QStringLiteral("item"), run);
        if (i == 0) list = c.createList(lf); else list->add(c.block());
    }
    c.insertBlock(bf);
    c.insertText(QStringLiteral("outro"), run);
    LayoutEnv env;
    env.wordLines = true;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(400, 700)}, env);
    const auto all = lay.lineInfo(0);
    QCOMPARE(all.size(), 5);
    QVERIFY(std::abs((all[1].rect.top() - all[0].rect.bottom()) - 14) < 0.01);
    QVERIFY(std::abs((all[2].rect.top() - all[1].rect.bottom())) < 0.01);
    QVERIFY(std::abs((all[3].rect.top() - all[2].rect.bottom())) < 0.01);
    QVERIFY(std::abs((all[4].rect.top() - all[3].rect.bottom()) - 14) < 0.01);
}


// A word processor's tab stops are measured from the text area's edge, not from the paragraph's indent, and the default stops go on after the last
// of the paragraph's own.
void FlowTests::tabStopsAreFromTheEdgeAndTheDefaultOnesFollowTheLast()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    QTextBlockFormat bf;
    bf.setLeftMargin(20);
    QTextOption::Tab stop;
    stop.position = 100;
    stop.type = QTextOption::LeftTab;
    bf.setTabPositions({stop});
    c.setBlockFormat(bf);
    c.insertText(QStringLiteral("ab\tcd\tef"), run);
    LayoutEnv env;
    env.wordLines = true;
    env.defaultTab = 36;
    StoryLayout lay;
    lay.build(&doc, {plainFrame(400, 100)}, env);
    auto xOf = [&](int from) { return lay.rangeRects(0, from, from + 1).first().left(); };
    QVERIFY2(std::abs(xOf(3) - 100) < 0.05, qPrintable(QString::number(xOf(3))));   // "cd": at the stop, from the edge
    QVERIFY2(std::abs(xOf(6) - 144) < 0.05, qPrintable(QString::number(xOf(6))));   // "ef": the next default stop past "cd" (4 times 36)
    QVERIFY(xOf(0) > 19.9 && xOf(0) < 20.1);                                         // (the text begins at the indent)
}


// The hanging indent is a tab stop for the first line (a tab goes to it), unless the file says its hanging indents are not (LayoutEnv::wordNoHangStop):
// then the default stops go on from the edge, and two tabs after a short word end at the second of them.
void FlowTests::aHangingIndentIsATabStopUnlessTheFileSaysNot()
{
    auto textAt = [&](bool noStop) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        bf.setLeftMargin(72);
        bf.setTextIndent(-72);
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("!\t\ttext"), run);
        LayoutEnv env;
        env.wordLines = true;
        env.defaultTab = 36;
        env.wordNoHangStop = noStop;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(400, 100)}, env);
        return lay.rangeRects(0, 3, 4).first().left();
    };
    QVERIFY2(std::abs(textAt(false) - 108) < 0.05, qPrintable(QString::number(textAt(false))));   // the first tab to the indent (72), the second to the stop after it
    QVERIFY2(std::abs(textAt(true) - 72) < 0.05, qPrintable(QString::number(textAt(true))));      // the first to 36, the second to 72
    LayoutEnv a, b;
    b.wordNoHangStop = true;
    QVERIFY(a.key() != b.key());
}


// The hanging indent is a tab stop for the first line though the paragraph has stops of its own, past it: "a.<tab>text" has its text at the indent (V016's sub-items, indent
// 40 points and stops from 103), and a stop of the paragraph's own before the indent is the nearer one.
void FlowTests::aHangingIndentIsATabStopBeforeTheParagraphsOwnStops()
{
    auto textAt = [&](double firstStop) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Times New Roman")});
        run.setFontPointSize(12);
        QTextBlockFormat bf;
        bf.setLeftMargin(40);
        bf.setTextIndent(-26.65);
        bf.setTabPositions({QTextOption::Tab(firstStop, QTextOption::LeftTab), QTextOption::Tab(firstStop + 30, QTextOption::LeftTab)});
        c.setBlockFormat(bf);
        c.insertText(QStringLiteral("a.\ttext"), run);
        LayoutEnv env;
        env.wordLines = true;
        env.defaultTab = 36;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(400, 100)}, env);
        return lay.rangeRects(0, 3, 4).first().left();
    };
    QVERIFY2(std::abs(textAt(103) - 40) < 0.05, qPrintable(QString::number(textAt(103))));
    QVERIFY2(std::abs(textAt(30) - 30) < 0.05, qPrintable(QString::number(textAt(30))));
}


// A justified line of a word processor's text reaches the margin with its last letter, not a space short of it (the space that ends the line is not
// part of the width the words are spread over); the paragraph's last line is as long as its words make it.
void FlowTests::justifiedWordLinesReachTheMargin()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat run;
    run.setFontFamilies({QStringLiteral("Times New Roman")});
    run.setFontPointSize(12);
    QTextBlockFormat bf;
    bf.setAlignment(Qt::AlignJustify);
    c.setBlockFormat(bf);
    c.insertText(QStringLiteral("Alvin developed from a poorly-defined tropical wave that crossed the Atlantic Ocean and the Caribbean Sea during the second half of May and then moved "
                                "westward across the eastern North Pacific with little associated thunderstorm activity until the wave continued westward across the "
                                "eastern North Pacific waters."), run);
    for (bool noHang : {false, true}) {
        LayoutEnv env;
        env.wordLines = true;
        env.wordNoHang = noHang;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(468, 300)}, env);
        const auto lines = lay.lineInfo(0);
        QVERIFY(lines.size() >= 3);
        for (int i = 0; i + 1 < lines.size(); ++i) {
            const auto rects = lay.rangeRects(0, lines[i].docEnd - 2, lines[i].docEnd - 1);
            QVERIFY(!rects.isEmpty());
            QVERIFY2(std::abs(rects.last().right() - 468) < 0.3, qPrintable(QStringLiteral("line %1 ends at %2 (hang %3)").arg(i).arg(rects.last().right()).arg(!noHang)));
        }
        QVERIFY(lay.rangeRects(0, lines.last().docEnd - 2, lines.last().docEnd - 1).last().right() < 400);
    }
}


// A word processor's text has no ligatures: the letters of "ti" in Carlito (a face the program carries, with a ligature for them) take what they take
// apart, where a publication's text sets them as one glyph.
void FlowTests::wordTextHasNoLigatures()
{
    QVERIFY2(QFontDatabase::hasFamily(QStringLiteral("Carlito")), "the face the program carries is not registered");
    auto width = [&](const QString &text, bool word) {
        QTextDocument doc;
        QTextCursor c(&doc);
        QTextCharFormat run;
        run.setFontFamilies({QStringLiteral("Carlito")});
        run.setFontPointSize(24);
        c.insertText(text, run);
        LayoutEnv env;
        env.wordLines = word;
        StoryLayout lay;
        lay.build(&doc, {plainFrame(1000, 100)}, env);
        const auto rects = lay.rangeRects(0, 0, text.size());
        return rects.isEmpty() ? -1.0 : rects.last().right() - rects.first().left();
    };
    const double apart = width(QStringLiteral("t"), true) + width(QStringLiteral("i"), true);
    QVERIFY2(std::abs(width(QStringLiteral("ti"), true) - apart) < 0.01, qPrintable(QStringLiteral("%1 against %2").arg(width(QStringLiteral("ti"), true)).arg(apart)));
    QVERIFY2(std::abs(width(QStringLiteral("ti"), false) - apart) > 0.05, "the face has no ligature for ti");   // (a publication's text keeps it)
}
